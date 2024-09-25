#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// Function to parse command-line arguments (socket path and file path)
void parse_arguments(int argc, char *argv[], char **socket_path, char **file_path) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <socket_path> <file_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    *socket_path = argv[1];
    *file_path = argv[2];
}

// Function to create and connect the client socket
int create_and_connect_socket(const char *socket_path) {
    int sock;
    struct sockaddr_un addr;

    // Create a UNIX domain socket
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    // Connect to the server's socket
    if (connect(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        close(sock);
        exit(EXIT_FAILURE);
    }

    return sock;
}

// Function to send file path and receive file content
void send_file_path_and_receive_response(int sock, const char *file_path) {
    char buffer[BUFFER_SIZE];

    // Send file path to the server
    if (send(sock, file_path, strlen(file_path) + 1, 0) == -1) {
        perror("send");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Receive and print the file content from the server
    while (1) {
        ssize_t n = recv(sock, buffer, sizeof(buffer), 0);
        if (n == -1) {
            perror("recv");
            close(sock);
            exit(EXIT_FAILURE);
        } else if (n == 0) {
            break;  // Server closed connection
        }
        fwrite(buffer, 1, n, stdout);  // Print the received content
    }
}

int main(int argc, char *argv[]) {
    char *socket_path, *file_path;
    int client_sock;

    // Parse command-line arguments
    parse_arguments(argc, argv, &socket_path, &file_path);

    // Create and connect the client socket
    client_sock = create_and_connect_socket(socket_path);

    // Send file path to the server and receive file content
    send_file_path_and_receive_response(client_sock, file_path);

    // Close the client socket and clean up
    close(client_sock);
    return EXIT_SUCCESS;
}
