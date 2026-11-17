
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8084
#define BUFFER_SIZE 4096

int main(void) {
    int sock;
    struct sockaddr_in server = {0};
    char buffer[BUFFER_SIZE];

    // Avoid terminating on a broken pipe
    signal(SIGPIPE, SIG_IGN);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        perror("socket");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server,
                sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    printf("Connected to server.\n");

    // Start FFplay to decode and display the video
    FILE *player = popen(
        "ffplay -loglevel error -autoexit -i pipe:0",
        "w"
    );

    if (player == NULL) {
        perror("popen");
        close(sock);
        return 1;
    }

    ssize_t bytes_received;

    while ((bytes_received = recv(sock, buffer,
                                  BUFFER_SIZE, 0)) > 0) {

        if (fwrite(buffer, 1, bytes_received, player)
            != (size_t)bytes_received) {
            perror("fwrite");
            break;
        }
    }

    if (bytes_received < 0) {
        perror("recv");
    }

    pclose(player);
    close(sock);

    return 0;
}
