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

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "../network/network.h"

int socketID;

int main(int argc, char **args) {
    printf("Starting TAXOB Server...\n");
    
    // CREATE A SOCKET THE SERVER WILL LISTEN TO
    if (createConnection() == EXIT_FAILURE) {
        // SOMETHING FAILED
        return EXIT_FAILURE;
    }

    printf("Server started!\n");
    return EXIT_SUCCESS;
}

int createConnection(int port) {

    // CREATE A SOCKET USING IP PROTOCOL AND TCP PROTOCOL
    socketID = createSocket();
    if (socketID < 0) {
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
    server_addr.sin_port = htons( TAXOB_PORT );
    // BIND THE SOCKET TO THE PARAMETER
    int result = bind(socketID, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr));
    if (result < 0 ) {
        perror("Unable to bind server to socket!\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
