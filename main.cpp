#include <iostream>
#include <string>
#include <thread>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <json.hpp>

#include "walManager.hpp"
#include "commandParser.hpp"
#include "database.hpp"

using json = nlohmann::json;

void handleClient(int clientSocket, Database& db) {
    char buffer[1024];
    ssize_t bytesRead;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';  // Null-terminate input
        std::string input(buffer);

        std::string response = db.processInput(input);

        if (response == "__DISCONNECT__") break;

        if (!response.empty() && response.back() != '\n') {
            response += '\n';
        }

        send(clientSocket, response.c_str(), response.size(), 0);
    }

    std::cout << "Client disconnected\n";
    close(clientSocket);
}

int main() {
    const int PORT = 8080;
    int server_fd, clientSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    Database db;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "TCP server started on port " << PORT << "\n";

    while (true) {
        if ((clientSocket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            continue;
        }

        std::cout << "New client connected\n";
        std::thread(handleClient, clientSocket, std::ref(db)).detach();
    }

    return 0;
}
