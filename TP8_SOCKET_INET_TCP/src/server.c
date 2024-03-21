#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h> // Pour mkdir

#define PORT 3264
#define BUFFER_SIZE 1024

int server_socket;
char* root_directory;

void write_log(const char* client_ip, int window_size, const char* file_name) {
    struct stat st = {0};
    if (stat("logs", &st) == -1) {
        mkdir("logs", 0700);
    }

    FILE* log_file = fopen("logs/server.log", "a");
    if (log_file == NULL) {
        perror("Erreur lors de l'ouverture du fichier de log");
        return;
    }

    fprintf(log_file, "Client %s requested file %s with window size %d\n", client_ip, file_name, window_size);
    fclose(log_file);
}

void *handle_client(void *client_socket_ptr) {
    int client_socket = *((int*)client_socket_ptr);
    free(client_socket_ptr);

    char client_ip[INET_ADDRSTRLEN];
    int window_size;
    char file_name[256];
    char file_path[512]; // Chemin complet du fichier à ouvrir
    char buffer[BUFFER_SIZE];
    ssize_t read_bytes, sent_bytes;

    // Obtenir l'adresse IP du client
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    getpeername(client_socket, (struct sockaddr*)&addr, &addr_size);
    inet_ntop(AF_INET, &addr.sin_addr, client_ip, INET_ADDRSTRLEN);

    // Recevoir la taille de la fenêtre
    read(client_socket, &window_size, sizeof(window_size));
    window_size = (int)ntohl((unsigned int)window_size); // Convertir de l'ordre des octets réseau à l'ordre des octets hôte

    // Recevoir le nom du fichier
    read(client_socket, file_name, sizeof(file_name));

    // Construire le chemin complet du fichier à partir du répertoire racine et du nom du fichier reçu
    snprintf(file_path, sizeof(file_path), "%s/%s", root_directory, file_name);

    // Ouvrir le fichier
    int file_fd = open(file_path, O_RDONLY);
    if (file_fd < 0) {
        perror("Erreur lors de l'ouverture du fichier");
        close(client_socket);
        pthread_exit(NULL);
    }

    // Lire et envoyer le fichier par blocs
    while ((read_bytes = read(file_fd, buffer, (size_t)window_size)) > 0) {
        sent_bytes = send(client_socket, buffer, (size_t)read_bytes, 0);
        if (sent_bytes < 0) {
            perror("Erreur lors de l'envoi du fichier");
            break;
        }
    }

    // Envoyer un bloc final de 0 octet si nécessaire
    if (read_bytes == 0) {
        send(client_socket, buffer, 0, 0);
    }

    // Logging
    write_log(client_ip, window_size, file_name);

    close(file_fd);
    close(client_socket);
    pthread_exit(NULL);
}

void signal_handler() {
    printf("Fermeture du serveur\n");
    close(server_socket);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <dossier_des_fichiers>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    root_directory = argv[1];
    struct sockaddr_in server_addr;

    signal(SIGINT, signal_handler);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur de bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Erreur lors de l'écoute");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Serveur en écoute sur le port %d\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int *client_socket_ptr = malloc(sizeof(int));
        if (client_socket_ptr == NULL) {
            perror("Erreur d'allocation mémoire");
            continue;
        }

        *client_socket_ptr = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);

        if (*client_socket_ptr < 0) {
            perror("Erreur lors de l'acceptation de la connexion client");
            free(client_socket_ptr);
            continue;
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void*)client_socket_ptr) < 0) {
            perror("Erreur lors de la création du thread");
            free(client_socket_ptr);
        } else {
            // Détacher le thread pour libérer les ressources une fois terminé
            pthread_detach(thread_id);
        }
    }

    // Nettoyage (en théorie, ce code n'est jamais atteint en raison de la boucle infinie)
    close(server_socket);
    return 0;
}
