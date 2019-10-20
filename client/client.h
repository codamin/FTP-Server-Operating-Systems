
#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <time.h>
#include "fcntl.h"

#define MAX_REQUEST_SIZE 100
#define MAX_OWNED_FILES  10
#define FILE_CHUNK_SIZE  256
#define FILE_DIR         "/home/amin/Desktop/p1-os/client/files/"
#define NOT_FOUND        "file not found"
#define MAX_HB_MSG_SIZE  4
#define MAX_IO           3
#define MAX_BC_MSG_SIZE  100 

#endif

