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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "network.h"

int createSocket(void) {
    return socket(SOCKET_FAMILY, SOCKET_TYPE, SOCKET_PROTOCOL);
}

int getAddress(char *address, struct sockaddr_in *sockAddr) {
    // IS THE ADDRESS IN THE FORMAT XXX.YYYY.ZZZZ.WWW LIKE 127.0.0.1?
    // IF RESULT IS ZERO, IT ISN'T
    if (inet_aton(address, &(sockAddr->sin_addr)) == 0) {
        // TRY TO RESOLVE THE DOMAIN
        struct hostent *host;
        // RESOLVE THE DOMAIN...
        host = gethostbyname(address);
        // UNABLE TO RESOLVE
        if (host == NULL) {
            printf("Unknown address %s!\n", address);
            return EXIT_FAILURE;
        }
        sockAddr->sin_addr = *(struct in_addr*)host->h_addr;
    }
    // FOUND AN ADDRESS
    return EXIT_SUCCESS;    
}
