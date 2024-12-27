#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <thread>
#pragma comment(lib, "ws2_32.lib")

#define PORT 42069
#define BUFFER_SIZE 1024

std::vector<int> clients;

void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    while (true) {
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            std::cerr << "Client disconnected or error\n";
            closesocket(client_fd);
            return;
        }
        buffer[bytes_received] = '\0';
        std::cout << "Received: " << buffer << "\n";
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Cannot create socket\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Cannot bind socket\n";
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    if (listen(server_fd, 5) == -1) {
        std::cerr << "Cannot listen on socket\n";
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    std::cout << "Waiting for connections...\n";

    while (true) {
        sockaddr_in client_addr;
        int client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            std::cerr << "Cannot accept connection\n";
            continue;
        }

        std::cout << "Client connected.\n";
        clients.push_back(client_fd);
        std::thread(handle_client, client_fd).detach();
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}