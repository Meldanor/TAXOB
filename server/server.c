/*
 * Copyright (C) 2012 Kilian Gärtner
 * 
 * This file is part of TAXOB.
 * 
 * TAXOB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * 
 * TAXOB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with TAXOB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "server.h"
#include "../network/network.h"

int serverSocket;
bool serverIsRunning = true;

// CURRENT CONNECTED CLIENTS
int clientSockets[64];
int connectedClients;

int main(int argc, char **args) {

    printf("Starting TAXOB Server...\n");

    // REGISTERING THE STOP SIGNAL
    signal(SIGINT, stopServer);

    /* READ ARGUMENTS*/
    if (argc < 3) {
        printf("%s -p Port\n", args[0]);
        return EXIT_FAILURE;
    }
    int i;
    char *cur;
    long int port;
    for (i = 1 ; i < argc ; ++i) {
        cur = args[i];
        // READ PORT
        if (strcmp(cur, "-p") == 0) {
            // Todo: Ohne regex
            // Check if the user has written a number
            if (!isNumber(args[++i])) {
                printf("Port '%s' is an invalid number!\n", args[i]);
                return EXIT_FAILURE;
            }            
            port = strtol(args[i], (char **) NULL, 10);
            // DISALLOW KERNEL PORTS
            if (port <= 1024) {
                printf("Port %ld must be higher than 1024!\n", port);
                return EXIT_FAILURE;
            }
        }
        // UNKNOWN OPTION
        else {
            printf("Unknown option %s\n", cur);
            return EXIT_FAILURE;
        }
    }

    printf("Trying to listen to the port %ld...\n", port);
    // CREATE A SOCKET THE SERVER WILL LISTEN TO
    if (createConnection(port) == EXIT_FAILURE) {
        // SOMETHING FAILED
        return EXIT_FAILURE;
    }

    printf("TAXOB Server started!\n");

    // HANDLE ALL INCOMING CLIENTS
    serverLoop();

    return EXIT_SUCCESS;
}

int createConnection(int port) {

    // CREATE A SOCKET USING IP PROTOCOL AND TCP PROTOCOL
    serverSocket = createSocket();
    if (serverSocket < 0) {
        perror("Unable to create socket!\n");
        return EXIT_FAILURE;
    }
    
    // INIT PORT AND ACCEPT CONNECTIONS FROM ALL IPs
    struct sockaddr_in server_addr;
    // IP PROTOCOL
    server_addr.sin_family = SOCKET_FAMILY;
    // ACCEPT CONNECTIONS FROM EVERY IP
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // LISTEN TO PORT
    server_addr.sin_port = htons( port );
    // BIND THE SOCKET TO THE PARAMETER
    int result = bind(serverSocket, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr_in));
    if (result < 0 ) {
        perror("Unable to bind server to socket!\n");
        return EXIT_FAILURE;
    }

    // START LISTENING TO SOCKET
    listen(serverSocket, SERVER_QUEUE_SIZE);

    return EXIT_SUCCESS;
}

void serverLoop(void) {
    // INFORMATION ABOUT THE CLIENT
    struct sockaddr_in client;
    int clientSocket;
    socklen_t len = sizeof(client);

    // SERVER LOOP
    while (serverIsRunning) {
        // BLOCKS UNTIL A CONNECTION IS INSIDE THE QUEUE
        clientSocket = accept( serverSocket, (struct sockaddr*)(&client), &len);
        if (clientSocket < 0 ) {
            perror("Can't accept a new client!\n");
            continue;
        }
        puts("Client connected!");
        // HANDLE THE CLIENT
        handleClient(clientSocket, &client);
    }

    stopServer(EXIT_SUCCESS);
}

#define OUT_BUFFER_SIZE 4096
#define IN_BUFFER_SIZE 4096

void handleClient(int clientSocket, struct sockaddr_in *client) {

    // Write everything immidately to the console
    setbuf(stdout, NULL);

    clientSockets[connectedClients++] = clientSocket;
    // BUFFER
    char outBuffer[OUT_BUFFER_SIZE] = {0};
    char inBuffer[IN_BUFFER_SIZE] = {0};

    int bytes_read;
    int bytes_send;

    // Wait for information from the client
    while((bytes_read = read(clientSocket, inBuffer, sizeof(inBuffer)))) {

        if (bytes_read == -1) {
            perror("Something went wrong while receiving...\n");
            break;
        }  
        else {
            // Write information about the receiving process
            printf("Received %d Bytes: ", bytes_read);
            write(STDOUT_FILENO, inBuffer, bytes_read);

            // Copy incoming data to out going buffer
            memcpy(outBuffer, inBuffer, bytes_read);

            // Send data back to client
            bytes_send = write(clientSocket, outBuffer, bytes_read);
            if (bytes_send == -1 ) {
                perror("Something went wrong while sending...\n");
                break;
            }
            else {
                // Write information about the sending process
                printf("Sent %d Bytes: ", bytes_read);
                write(STDOUT_FILENO , outBuffer, bytes_send);
            }
        }
    }
    puts("Disconnect client...");
    // CLOSE CONNECTION
    close(clientSocket);
    clientSockets[--connectedClients] = 0;
}

void stopServer(int signal) {
    printf("Shutting down the server...\n");
    // FUNCTION TO CLEAN UP
    printf("Close %d client sockets...\n", connectedClients);
    // CLOSE CLIENT SOCKETS 
    int i;
    for (i = 0 ; i < connectedClients; ++i) {
        close(clientSockets[i]);
    } 
    printf("Close server socket...\n");
    // CLOSE SERVER SOCKET    
    close(serverSocket);
    
    exit(signal);
}
