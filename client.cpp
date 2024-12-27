#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <cstring>
#include <thread>
#include <atomic>

#define SERVER_IP "127.0.0.1"
#define PORT 42069
#define BUFFER_SIZE 1024

std::atomic<bool> running(true);

void send_message(int client_fd) {
    char buffer[BUFFER_SIZE];
    while (running) {
        std::cout << "send message: ";
        std::cin.getline(buffer, BUFFER_SIZE);
        if (strcmp(buffer, "exit") == 0) {
            running = false;
            break;
        }
        int bytes_sent = send(client_fd, buffer, strlen(buffer), 0);
        if (bytes_sent == -1) {
            std::cerr << "Failed to send message\n";
            break;
        } else {
            std::cout << "Message sent: " << buffer << "\n";
        }
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        std::cerr << "Cannot create socket\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Cannot connect to server\n";
        closesocket(client_fd);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server.\n";

    std::thread send_thread(send_message, client_fd);
    if (send_thread.joinable()) {
        std::cout << "Thread started successfully.\n";
        send_thread.detach();
    } else {
        std::cerr << "Failed to start thread.\n";
    }

    // Keep the main thread alive to continue sending messages
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    closesocket(client_fd);
    WSACleanup();
    std::cout << "Client disconnected.\n";
    return 0;
}