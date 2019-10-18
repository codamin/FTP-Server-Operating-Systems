#include "server.h"

void get_file(char* file_name, int read_fd) {

    int nbytes;
    char recvBuf[256];
    int full_size = strlen(FILE_DIR) + strlen(file_name) + 1;
    char* full_file_name = (char*)malloc(full_size);
    for(int i = 0; i < strlen(FILE_DIR); i++) {
        full_file_name[i] = FILE_DIR[i];
    }
    for(int i = 0; i < strlen(file_name); i++) {
        full_file_name[i + strlen(FILE_DIR)] = file_name[i];
    }
    full_file_name[full_size - 1] = '\0';
    int fd = open(full_file_name, O_CREAT | O_WRONLY);

    while (1) {
        nbytes = read(read_fd, recvBuf, FILE_CHUNK_SIZE);
        if (nbytes > 0)
            write(fd, recvBuf, nbytes);
        if (nbytes < FILE_CHUNK_SIZE)
            break;
    }

    for (int i = 0; i < MAX_FILES; i++)
        if (files[i] == 0) {
            files[i] = file_name;
            break;
        }
}


void send_file(char* file_name, int file_name_length, int write_fd) {

    int full_size = strlen(FILE_DIR) + file_name_length + 1;
    char* full_file_name = (char*)malloc(full_size);
    for(int i = 0; i < strlen(FILE_DIR); i++) {
        full_file_name[i] = FILE_DIR[i];
    }
    for(int i = 0; i < file_name_length ; i++) {
        full_file_name[i + strlen(FILE_DIR)] = file_name[i];
    }

    full_file_name[full_size] = '\0';

    int read_fd = open(full_file_name, O_RDONLY);
    if (read_fd < 0) {
        write(write_fd, "N", 1);
        write(1, "requested file not found\n", 25);
        return;
    }
    
    write(write_fd, "Y", 1);
    write(1, "sending file...\n", 16);
    while (1) {
        char buf[FILE_CHUNK_SIZE] = {0};
        int nbytes = read(read_fd, buf, FILE_CHUNK_SIZE);
        if(nbytes > 0) {
            write(write_fd, buf, nbytes);
        }
        if (nbytes < FILE_CHUNK_SIZE)
            break;
    }
    write(1, "sending finished...\n", 20);
}

int create_new_connection(int listen_socket) {

    int new_socket;
    struct sockaddr_in new_client_address;
    socklen_t addrlen = sizeof(new_client_address);
    if ((new_socket = accept(listen_socket, (struct sockaddr*) &new_client_address, (socklen_t*) &addrlen)) < 0) {
        char msg[] = "create_new_connection: creating new connection failed";
        write(1, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i] == 0) {
            clients[i] = new_socket;
            break;
        }
    }
    write(1, "client connected with ip: ", sizeof("client connected with ip:"));
    write(1, inet_ntoa(new_client_address.sin_addr), strlen(inet_ntoa(new_client_address.sin_addr)));
    write(1, "\n", sizeof("\n"));
    return new_socket;
}

// void process_new_request(fd_set* readfds) {

//     write(1, "client requested\n", 17);
//     for (int i = 0; i < MAX_CLIENTS; i++) {
//         int sd = clients[i];

//         if (FD_ISSET(sd, readfds)) {
//             char buf[20];
//             read(sd, buf, 1024);
//             char request[3];
//             parse_request(buf, request);

//             if(request[0] == "download") {
//                 if (send_file(request[1], clients[i]) == -1)
//                     send(sd, NOT_FOUND, sizeof(NOT_FOUND), 0);
//             }

//             else if(request[0] == "upload") {

//             }
        
//         }
//     }
// }