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
#define FILE_CHUNK_SIZE     512

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
        char buf[FILE_CHUNK_SIZE];
        int nbytes = read(file, buf, FILE_CHUNK_SIZE);
        if(nbytes > 0) {
            write(sock, buf, nbytes);
        }
        if (nbytes < FILE_CHUNK_SIZE)
            break;
    }
    write(1, "sending finished...\n", 20);
}

void reverse(char str[], int length) 
{ 
    int start = 0; 
    int end = length -1; 
    while (start < end) 
    { 
        // swap(*(str+start), *(str+end));
        int temp = *(str+start);
        *(str+start) = *(str+end);
        *(str+end) = temp;
        start++; 
        end--; 
    } 
} 
  
// Implementation of itoa() 
char* itoa(int num, char* str, int base) 
{ 
    int i = 0; 
    int isNegative = 0; 
  
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0) 
    { 
        str[i++] = '0'; 
        str[i] = '\0'; 
        return str; 
    } 
  
    // In standard itoa(), negative numbers are handled only with  
    // base 10. Otherwise numbers are considered unsigned. 
    if (num < 0 && base == 10) 
    { 
        isNegative = 1; 
        num = -num; 
    } 
  
    // Process individual digits 
    while (num != 0) 
    { 
        int rem = num % base; 
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0'; 
        num = num/base; 
    } 
  
    // If number is negative, append '-' 
    if (isNegative) 
        str[i++] = '-'; 
  
    str[i] = '\0'; // Append string terminator 
  
    // Reverse the string 
    reverse(str, i); 
  
    return str; 
} 