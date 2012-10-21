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

#include "../network/network.h"
#include "client.h"

#define TEST_ADDRESS "127.0.0.1"

int clientSocket;
bool clientIsRunning = true;

int main(int argc, char **args) {
    printf("Starting TAXOB Client...\n");
    if (createConnection(TEST_ADDRESS, TAXOB_PORT) == EXIT_FAILURE) {        
        return EXIT_FAILURE;
    }
    printf("TAXOB Client started!\n");
    clientLoop();
    return EXIT_FAILURE;
}

int createConnection(char *address, int port) {
    clientSocket = createSocket();
    if(clientSocket < 0 ) {
        perror("Unable to create socket!\n");
        return EXIT_FAILURE;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = SOCKET_FAMILY;
    serv_addr.sin_port = htons(port);
    
    if (getAddress(address, &serv_addr) == EXIT_FAILURE)
        return EXIT_FAILURE;
    
    if (connect(clientSocket, (struct sockaddr*)(&serv_addr), sizeof(struct sockaddr)) == -1) {
        perror ("Unable to connect to the server!\n");
        return EXIT_FAILURE;
    }
}

void clientLoop(void) {

    char outBuffer[512];
    char inBuffer[1024];

    while (clientIsRunning) {
        scanf("%512s", outBuffer);
        printf("Send\t'%s512'\n", outBuffer);
        if (sendMessage(outBuffer) == EXIT_SUCCESS) {
            clientIsRunning = false;
            break;
        }
        if (receiveMessage(inBuffer) == EXIT_SUCCESS) {
            clientIsRunning = false;
            break;
        }
        printf("Received\t'%s1024'\n", inBuffer);
    }
    
}

int sendMessage(char *msg) {
    int bytes = send(clientSocket, msg, strlen(msg), 0);
    if (bytes == -1) {
        perror("Error while sending data!\n");
        return EXIT_FAILURE;
    }
    else
        return EXIT_SUCCESS;
}

int receiveMessage(char *buffer) {
    int bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytes == -1) {
        perror("Error while receiving data!\n");
        return EXIT_FAILURE;
    }
    else
        return EXIT_SUCCESS;
}
