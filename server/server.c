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
            // CONVERT THE STRING TO AN INTEGER
            // TODO: Use a method that checqks if it is an valid number
            port = strtol(args[++i], (char **) NULL, 10);
            if (port == LONG_MIN || port == LONG_MAX) {
                printf("Port %s is an invalid number!\n", args[i]);
                return EXIT_FAILURE;
            }
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
    int result = bind(serverSocket, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr));
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
        // HANDLE THE CLIENT
        handleClient(clientSocket, &client);
    }

    stopServer(EXIT_SUCCESS);
}

#define OUT_BUFFER_SIZE 4096
#define IN_BUFFER_SIZE 4096

void handleClient(int clientSocket, struct sockaddr_in *client) {

    // TODO: ONLY ACCEPT A MAXIMUM
    clientSockets[connectedClients++] = clientSocket;
    // TODO: Create a thread to handle the connected client
    // BUFFER
    char outBuffer[OUT_BUFFER_SIZE];
    char inBuffer[IN_BUFFER_SIZE];
    // CLEAR BUFFER (WE DONT WANT TO SEND TRASH)
    memset(outBuffer, 0, OUT_BUFFER_SIZE);
    memset(inBuffer, 0, IN_BUFFER_SIZE);

    int bytes_read;
    int bytes_send;
    
    bytes_read = recv(clientSocket, inBuffer, IN_BUFFER_SIZE - 1, 0);
    
    if (bytes_read == -1) {
        perror("Something went wrong while receiving...\n");
    }  
    else {    
        printf("Received %d Bytes. %s", bytes_read, inBuffer);
        memcpy(outBuffer, inBuffer, bytes_read);
        bytes_send = send(clientSocket, outBuffer, bytes_read, 0);
        if (bytes_send == -1 ) {
            perror("Something went wrong while sending...\n");
        }
        else
            printf("Send %d Bytes.    %s", bytes_send, outBuffer);

    }
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
