
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
#define FILE_CHUNK_SIZE     512
#define FILE_DIR            "/home/amin/Desktop/p1-os/server/files/"

int clients[MAX_CLIENTS];
char* files[MAX_FILES];
bool IsHeartBeating;

int create_heart_beat_socket(int hb_port, struct sockaddr_in* hb_addr);
int server_hand_shake(int sock, char* full_file_name, int mode);
int create_new_connection(int listen_socket);
int download(int sock, int file);
void upload(int sock, int file);

#endif