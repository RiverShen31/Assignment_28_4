#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void get_time(char *buffer, const char *format) {
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, BUFFER_SIZE, format, timeinfo);
}

int main() {
    int listener, client, max_fd, i;
    struct sockaddr_in addr;
    fd_set read_fds, master;
    char buffer[BUFFER_SIZE];

    listener = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listener, (struct sockaddr *)&addr, sizeof(addr));

    listen(listener, MAX_CLIENTS);

    FD_ZERO(&master);
    FD_SET(listener, &master);
    max_fd = listener;

    while(1) {
        read_fds = master;
        select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        for(i = 0; i <= max_fd; i++) {
            if(FD_ISSET(i, &read_fds)) {
                if(i == listener) {
                    client = accept(listener, NULL, NULL);
                    FD_SET(client, &master);
                    if(client > max_fd) {
                        max_fd = client;
                    }
                } else {
                    memset(buffer, 0, BUFFER_SIZE);
                    if(recv(i, buffer, BUFFER_SIZE, 0) <= 0) {
                        close(i);
                        FD_CLR(i, &master);
                    } else {
                        char format[10];
                        if(sscanf(buffer, "GET_TIME %s", format) == 1) {
                            if(strcmp(format, "dd/mm/yyyy") == 0) {
                                get_time(buffer, "%d/%m/%Y");
                            } else if(strcmp(format, "dd/mm/yy") == 0) {
                                get_time(buffer, "%d/%m/%y");
                            } else if(strcmp(format, "mm/dd/yyyy") == 0) {
                                get_time(buffer, "%m/%d/%Y");
                            } else if(strcmp(format, "mm/dd/yy") == 0) {
                                get_time(buffer, "%m/%d/%y");
                            } else {
                                strcpy(buffer, "Invalid format");
                            }
                            send(i, buffer, strlen(buffer), 0);
                            send(i, "\n", 1, 0);
                        } else {
                            send(i, "Invalid command", 15, 0);
                            send(i, "\n", 1, 0);
                        }
                    }
                }
            }
        }
    }

    return 0;
}