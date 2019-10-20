#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include "fcntl.h"

#define NOT_FOUND           "file not found" 
#define FILE_CHUNK_SIZE     1024

//char* find_full_name(char* file_name, char* file_dir) {
//
//    int full_size = strlen(file_dir) + strlen(file_name) + 1;
//    char* full_file_name = (char*)malloc(full_size);
//    for(int i = 0; i < strlen(file_dir); i++) {
//        full_file_name[i] = file_dir[i];
//    }
//    for(int i = 0; i < strlen(file_name); i++) {
//        full_file_name[i + strlen(file_dir)] = file_name[i];
//    }
//    full_file_name[full_size] = '\0';
//
//    return full_file_name;
//}

//void find_full_name(char* file_name, char* file_dir, char** full_file_name) {
//
//    int full_size = strlen(file_dir) + strlen(file_name) + 1;
//    *full_file_name = (char*)malloc(full_size);
//    for(int i = 0; i < strlen(file_dir); i++) {
//        (*full_file_name)[i] = file_dir[i];
//    }
//    for(int i = 0; i < strlen(file_name); i++) {
//        (*full_file_name)[i + strlen(file_dir)] = file_name[i];
//    }
//    (*full_file_name)[full_size - 1] = '\0';
//
//    // return full_file_name;
//}

int download(int sock, int file) {

    int nbytes;
    char recvBuf[FILE_CHUNK_SIZE];

    write(1, "downloading...\n", sizeof("downloading...\n"));
    while (1) {
        nbytes = read(sock, recvBuf, FILE_CHUNK_SIZE);
        if (nbytes > 0)
            write(file, recvBuf, nbytes);
        if (nbytes < FILE_CHUNK_SIZE) {
            break;
        }
    }
    close(file);
    write(1, "download finished\n", sizeof("download finished\n"));

}

void upload(int sock, int file) {
    
    write(1, "sending file...\n", 16);
    while (1) {
        char buf[FILE_CHUNK_SIZE] = {0};
        int nbytes = read(file, buf, FILE_CHUNK_SIZE);
        if(nbytes > 0) {
            write(sock, buf, nbytes);
        }
        if (nbytes < FILE_CHUNK_SIZE)
            break;
    }
    write(1, "sending finished...\n", 20);
}

void costum_itoa(int value, char** buf) {

    int copy_value = value;
    int digit_num = 0;
    while (copy_value >= 0) {
        copy_value /= 10;
        digit_num++;
    }
    *buf = (char*)malloc(digit_num * sizeof(char));
    for (int i = 0; i < digit_num; i++) {
        buf[i] = value%10 + '0';
        value /= 10;
    }
}