#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

#define PORT 9000
#define MAX_CLIENTS 10
#define BUFFER_SIZE 256

void handle_connection(int client) {
    char buffer[BUFFER_SIZE];
    int ret = recv(client, buffer, BUFFER_SIZE, 0);
    if (ret <= 0) {
        perror("recv() failed");
        close(client);
        return;
    }
    buffer[ret] = '\0';
    
    // Kiểm tra và xử lý lệnh từ client
    if (strncmp(buffer, "GET_TIME", 8) == 0) {
        char *format = buffer + 9; // Định dạng thời gian bắt đầu từ vị trí thứ 9
        time_t now;
        struct tm *tm_info;
        time(&now);
        tm_info = localtime(&now);

        char response[BUFFER_SIZE];
        if (strcmp(format, "dd/mm/yyyy\n") == 0) {
            strftime(response, BUFFER_SIZE, "%d/%m/%Y\n", tm_info);
            send(client, response, strlen(response), 0);
        } else if (strcmp(format, "dd/mm/yy\n") == 0) {
            strftime(response, BUFFER_SIZE, "%d/%m/%y", tm_info);
            send(client, response, strlen(response), 0);
        } else if (strcmp(format, "mm/dd/yyyy\n") == 0) {
            strftime(response, BUFFER_SIZE, "%m/%d/%Y", tm_info);
            send(client, response, strlen(response), 0);
        } else if (strcmp(format, "mm/dd/yy\n") == 0) {
            strftime(response, BUFFER_SIZE, "%m/%d/%y", tm_info);
            send(client, response, strlen(response), 0);
        } else {
            char *msg = "Invalid format";
            send(client, msg, strlen(msg), 0);
        }
    } else {
        char *msg = "Invalid command";
        send(client, msg, strlen(msg), 0);
    }

    close(client);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int opt = 1;

    // Tạo socket cho kết nối
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    // Thiết lập các tùy chọn socket để tái sử dụng địa chỉ và cổng
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt() failed");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ máy chủ
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Ràng buộc socket với địa chỉ máy chủ
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    // Chuyển socket sang trạng thái lắng nghe
    if (listen(server_fd, MAX_CLIENTS) == -1) {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started, listening on port %d...\n", PORT);

    // Chấp nhận và xử lý các kết nối đến
    while (1) {
        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) == -1) {
            perror("accept() failed");
            continue;
        }
        printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Xử lý kết nối trong một tiến trình con
        if (fork() == 0) {
            close(server_fd);
            handle_connection(client_fd);
            exit(EXIT_SUCCESS);
        } else {
            close(client_fd);
        }
    }

    return 0;
}