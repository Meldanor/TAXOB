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

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "server.h"
#include "../network/network.h"

int serverSocket;
bool serverIsRunning = true;

int main(int argc, char **args) {
    printf("Starting TAXOB Server...\n");
    
    // CREATE A SOCKET THE SERVER WILL LISTEN TO
    if (createConnection(TAXOB_PORT) == EXIT_FAILURE) {
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

    stopServer();
}

void handleClient(int clientSocket, struct sockaddr_in *client) {

    // TODO: Create a thread to handle the connected client
    char outBuffer[512];
    char inBuffer[1024];
    memset(outBuffer, 0, 512);
    memset(inBuffer, 0, 1024);
    while(true) {
        if (receiveMessage(clientSocket, inBuffer) == EXIT_FAILURE)
            break;
        printf("Received '%s'\n", inBuffer);
        memcpy(outBuffer, inBuffer, 512);
        if (sendMessage(clientSocket, outBuffer) == EXIT_FAILURE)
            break;
        printf("Send     '%s'\n", outBuffer);
    }
    
}

int sendMessage(int socket, char *msg) {
    int bytes = send(socket, msg, strlen(msg), 0);
    if (bytes == -1) {
        perror("Error while sending data!\n");
        return EXIT_FAILURE;
    }
    else {
        return EXIT_SUCCESS;
    }
}

int receiveMessage(int socket, char *buffer) {
    int bytes = recv(socket, buffer, 1024, 0);
    if (bytes == -1) {
        perror("Error while receiving data!\n");
        return EXIT_FAILURE;
    }
    else {
        return EXIT_SUCCESS;
    }
}
void stopServer(void) {
    // FUNCTION TO CLEAN UP
    close(serverSocket);
}
