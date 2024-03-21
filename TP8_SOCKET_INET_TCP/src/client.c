#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h> 

#define SERVER_PORT 3264

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <adresse_ip_du_serveur> <taille_fenetre> <nom_fichier>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Création du dossier de réception 
    struct stat st = {0};
    if (stat("received", &st) == -1) {
        mkdir("received", 0700);
    }

    char *server_ip = argv[1];
    int window_size = atoi(argv[2]);
    char file_path[1024]; 
    snprintf(file_path, sizeof(file_path), "received/%s", argv[3]); 

    int sock;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        printf("Adresse IP du serveur invalide\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur de connexion au serveur");
        close(sock);
        exit(EXIT_FAILURE);
    }

    window_size = (int)htonl((unsigned int)window_size); 
    
    send(sock, &window_size, sizeof(window_size), 0);

    send(sock, argv[3], strlen(argv[3]) + 1, 0);

    FILE *file = fopen(file_path, "wb");
    if (!file) {
        perror("Impossible de créer le fichier");
        close(sock);
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    ssize_t received_bytes;
    while ((received_bytes = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, sizeof(char), (size_t)received_bytes, file);
    }

    printf("Fichier '%s' reçu avec succès.\n", argv[3]);

    fclose(file);
    close(sock);

    return 0;
}
