
#ifndef SERVER_H
#define SERVER_H

#include <string.h>   //strlen  
#include <stdlib.h>
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <signal.h>
#include <stdbool.h>
#include "fcntl.h"

#define MAX_CLIENTS         5
#define HEART_BEAT_TIMEOUT  0.1
#define MAX_MSG_LENGTH      50
#define MAX_FILES           50
#define NOT_FOUND           "file not found" 
#define FILE_CHUNK_SIZE     256
#define FILE_DIR            "/home/amin/Desktop/p1-os/server/files/"
#define LISTEN_PORT         8000

int clients[MAX_CLIENTS];
char* files[MAX_FILES];
bool IsHeartBeating;


#endif