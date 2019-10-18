#include "client.h"

int main(int argc, char* argv[]) {
    char* owned_files[MAX_OWNED_FILES] = {0};
    struct sockaddr_in server_addr, client_addr;

    client_addr.sin_family = AF_INET; 
    client_addr.sin_addr.s_addr = INADDR_ANY; 
//    client_addr.sin_port = htons(argv[1]);
    client_addr.sin_port = htons(7000);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = 8080;

   
    char buf[MAX_REQUEST_SIZE];

    int sock = connect_to_server(client_addr, server_addr);

    int nbytes;
    while ((nbytes = read(0, buf, sizeof(buf))) > 0) {
        //if server is up
            //connect to server
            //send request to server

        buf[nbytes] = '\0';
        char* clean_buf = (char*)malloc(nbytes);
        for (int i = 0; i < nbytes - 1; i++) {
            clean_buf[i] = buf[i];
        }
        clean_buf[nbytes] = '\0';
        send_request_to_server(sock, clean_buf, nbytes, owned_files);

        //else
            //broadcast request
            //find one
            //connect to it
    }
    return 0;
}