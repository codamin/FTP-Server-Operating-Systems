
#ifndef CLIENT_H
#define CLIENT_H

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
#include <signal.h>
#include <fcntl.h> 
#include <errno.h>

#define MAX_REQUEST_SIZE   100
#define MAX_OWNED_FILES    10
#define FILE_CHUNK_SIZE    512
#define FILE_DIR           "/home/amin/Desktop/p1-os/client/files/"
#define NOT_FOUND          "file not found"
#define MAX_HB_MSG_SIZE    4
#define MAX_IO             10
#define MAX_BC_MSG_SIZE    100
#define MAX_FILE_NAME_SIZE 30
#define TOTAL_PORTS        65535

int download(int sock, int file);
void upload(int sock, int file);

int init_heart_beat_listen_socket(int hb_port);
int read_hb_msg(int sock, struct sockaddr_in hb_addr, int* server_listen_port);
int client_hand_shake(int sock, char* buf);
int connect_to_server(struct sockaddr_in client_addr, struct sockaddr_in server_addr);
void send_request_to_server(int sock, char* buf, int buflen);
int create_broadcast_socket(int bc_port, struct sockaddr_in* bc_addr);
void broadcast_request(int bc_sock, struct sockaddr_in bc_addr, char* file_name, int ss_listen_port);
int create_socket_to_listen(struct sockaddr_in file_reciever_addr);
int create_socket_to_send_file(struct sockaddr_in file_sender_addr, struct sockaddr_in other_reciever_addr);

int generate_random_port();
char* itoa(int num, char* str, int base); 

int broadcast_request_flag;

#endif

