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

#include "../network/network.h"
#include "client.h"

int clientSocket;
bool clientIsRunning = true;

int main(int argc, char **args) {
    printf("Starting TAXOB Client...\n");
    
    // REGISTERING THE STOP SIGNAL
    signal(SIGINT, stopClient);

    /* READ ARGUMENTS*/
    if (argc < 5) {
        printf("%s -p Port -h Host\n", args[0]);
        return EXIT_FAILURE;
    }
    int i;
    char *cur;

    char *address;
    long int port;
    for (i = 1 ; i < argc ; ++i) {
        cur = args[i];
        // READ PORT
        if (strcmp(cur, "-p") == 0) {
            port = strtol(args[++i], (char **) NULL, 10);
            if (port == LONG_MIN || port == LONG_MAX) {
                printf("Port %s is an invalid number!\n", args[i]);
                return EXIT_FAILURE;
            }
        }
        // READ HOST
        else if (strcmp(cur, "-h") == 0) {
            address = args[++i];
        }
        // UNKNOWN OPTION
        else {
            printf("Unknown option %s\n", cur);
            return EXIT_FAILURE;
        }
    }
    printf("Connect to %s on Port %ld...\n", address, port);

    if (createConnection(address, port) == EXIT_FAILURE) {        
        return EXIT_FAILURE;
    }

    printf("TAXOB Client started!\n");
    clientLoop();

    return EXIT_SUCCESS;
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
    memset(outBuffer, 0, 512);
    memset(inBuffer, 0, 1024);
    while (clientIsRunning) {
        fgets(outBuffer, 512, stdin);
        printf("Send     %s", outBuffer);
        if (sendMessage(outBuffer) == EXIT_FAILURE) {
            clientIsRunning = false;
            break;
        }
        if (receiveMessage(inBuffer) == EXIT_FAILURE) {
            clientIsRunning = false;
            break;
        }
        printf("Received %s", inBuffer);
    }
    
    stopClient(EXIT_SUCCESS);
}

int sendMessage(char *msg) {
    int bytes = send(clientSocket, msg, strlen(msg), 0);
    if (bytes == -1) {
        perror("Error while sending data!\n");
        return EXIT_FAILURE;
    }
    else {
        return EXIT_SUCCESS;
    }
}

int receiveMessage(char *buffer) {
    int bytes = recv(clientSocket, buffer, 1024 , 0);
    if (bytes == -1) {
        perror("Error while receiving data!\n");
        return EXIT_FAILURE;
    }
    else {
        return EXIT_SUCCESS;
    }
}

void stopClient(int signal) {
    printf("Shutting down the client...\n");
    printf("Close client socket...\n");
    close(clientSocket);
    
    exit(signal);
}
