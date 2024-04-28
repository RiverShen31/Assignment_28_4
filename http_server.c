#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define NUM_CHILDREN 4

void handle_connection(int client) {
    char buf[256];
    int ret = recv(client, buf, sizeof(buf), 0);
    buf[ret] = 0;
    puts(buf);

    char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
    send(client, msg, strlen(msg), 0);

    close(client);
}

int main() {
    // Create socket for connection
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    // Declare server address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    // Bind socket to address structure
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        return 1;
    }

    // Set socket to listening state
    if (listen(listener, 5)) {
        perror("listen() failed");
        return 1;
    }

    // Preforking
    for (int i = 0; i < NUM_CHILDREN; i++) {
        if (fork() == 0) {
            // Child process
            while (1) {
                int client = accept(listener, NULL, NULL);
                printf("New client connected: %d\n", client);
                handle_connection(client);
            }
            exit(0);
        }
    }

    // Parent process
    // Wait for input to terminate
    getchar();
    // Kill all child processes
    killpg(0, SIGKILL);

    return 0;
}