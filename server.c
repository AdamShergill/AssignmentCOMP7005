#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/unix_socket"

// Function to parse arguments (socket path)
void parse_arguments(int argc, char *argv[], char **socket_path) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <socket_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    *socket_path = argv[1];
}

// Function to create and bind socket
int create_and_bind_socket(const char *socket_path) {
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

    // Bind socket to the specified path
    unlink(socket_path);
    if (bind(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        close(sock);
        exit(EXIT_FAILURE);
    }

    return sock;
}

// Function to listen for and accept client connections
void accept_connections(int sock) {
    if (listen(sock, 5) == -1) {
        perror("listen");
        close(sock);
        exit(EXIT_FAILURE);
    }

    while (1) {
        int client_sock;
        char buffer[256];

        // Accept client connection
        if ((client_sock = accept(sock, NULL, NULL)) == -1) {
            perror("accept");
            continue;
        }

        // Receive file path from client
        memset(buffer, 0, sizeof(buffer));
        if (recv(client_sock, buffer, sizeof(buffer), 0) == -1) {
            perror("recv");
            close(client_sock);
            continue;
        }

        // Open the file and send its content back to the client
        FILE *file = fopen(buffer, "r");
        if (file == NULL) {
            perror("fopen");
            const char *error_msg = "Error: Unable to read file\n";
            send(client_sock, error_msg, strlen(error_msg), 0);
        } else {
            char file_content[1024];
            size_t n;

            // Read file and send it back to the client
            while ((n = fread(file_content, 1, sizeof(file_content), file)) > 0) {
                if (send(client_sock, file_content, n, 0) == -1) {
                    perror("send");
                    break;
                }
            }
            fclose(file);
        }

        close(client_sock);  // Close client connection
    }
}

int main(int argc, char *argv[]) {
    char *socket_path;
    int server_sock;

    // Parse command-line arguments
    parse_arguments(argc, argv, &socket_path);

    // Create and bind the server socket
    server_sock = create_and_bind_socket(socket_path);

    // Accept connections and handle clients
    accept_connections(server_sock);

    // Close the server socket and clean up
    close(server_sock);
    unlink(socket_path);
    return EXIT_SUCCESS;
}

