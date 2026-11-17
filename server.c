
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8084
#define BUFFER_SIZE 4096

int main(void) {
    int server_fd, client_fd;
    struct sockaddr_in address = {0};
    char buffer[BUFFER_SIZE];

    FILE *video = fopen("video.mp4", "rb");

    if (video == NULL) {
        perror("fopen");
        return 1;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        fclose(video);
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET,
               SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        return 1;
    }

    printf("Waiting for client on port %d...\n", PORT);

    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept");
        return 1;
    }

    printf("Client connected. Streaming video...\n");

    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, video)) > 0) {
        size_t sent = 0;

        while (sent < bytes_read) {
            ssize_t n = send(client_fd,
                             buffer + sent,
                             bytes_read - sent,
                             0);

            if (n <= 0) {
                perror("send");
                goto cleanup;
            }

            sent += n;
        }
    }

    printf("Streaming complete.\n");

cleanup:
    fclose(video);
    close(client_fd);
    close(server_fd);

    return 0;
}
