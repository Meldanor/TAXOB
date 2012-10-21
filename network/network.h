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

#define SOCKET_FAMILY AF_INET
#define SOCKET_TYPE SOCK_STREAM
#define SOCKET_PROTOCOL 0

#define TAXOB_PORT 1337

int createSocket(void);
