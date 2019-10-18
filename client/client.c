#include "client.h"

void parse_request(char* buf, char** request) {

    char copybuf[20];
    strncpy(copybuf, buf, strlen(buf));

    int i = 0;
    char* p = strtok (copybuf, " ");

    while (p != NULL) {
        request[i++] = p;
        p = strtok (NULL, " ");
    }
}

int mystrlen(char* string) {

    int size;
    for (size = 0; string[size] != '\0'; size++);
    return size;
}

int connect_to_server(struct sockaddr_in client_addr, struct sockaddr_in server_addr) {

    int sock;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to open socket");
    }

    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
        perror("setsockopt(SO_REUSEPORT) failed");

    if (bind(sock, (struct sockaddr*) &client_addr, sizeof(struct sockaddr_in)) < 0) {
        perror("\nclient socket binding failed\n");
        exit(EXIT_FAILURE); 
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) { 
        perror("\nConnect() failed \n"); 
        return -1; 
    }
    return sock;
}

int download(int sock, char* file_name) {

    char ready_buf[1];
    int nbytes;
    while(1) {
        nbytes = read(sock, ready_buf, 1);
        if (nbytes > 0) {
            if (ready_buf[0] == 'Y')
                break;
            else if (ready_buf[0] == 'N') {
                write(1, "download failed", 16);
                return;
            }
        }
    }
    char recvBuf[FILE_CHUNK_SIZE];
    int full_size = strlen(FILE_DIR) + strlen(file_name) + 1;
    char* full_file_name = (char*)malloc(full_size);
    for(int i = 0; i < strlen(FILE_DIR); i++) {
        full_file_name[i] = FILE_DIR[i];
    }
    for(int i = 0; i < strlen(file_name); i++) {
        full_file_name[i + strlen(FILE_DIR)] = file_name[i];
    }
    full_file_name[full_size] = '\0';

    int new_file = open(full_file_name, O_CREAT | O_WRONLY, 0777);
    if (new_file < 0) {
        perror("client crating new file");
        return;
    }

    write(1, "downloading...\n", sizeof("downloading...\n"));
    while (1) {
        nbytes = read(sock, recvBuf, FILE_CHUNK_SIZE);
        if (nbytes > 0)
            write(new_file, recvBuf, nbytes);
        if (nbytes < FILE_CHUNK_SIZE) {
            break;
        }
    }
    close(new_file);
    write(1, "download finished\n", sizeof("download finished\n"));

}

int upload(int sock, char* filename, char buf[]) {

}

void send_request_to_server(int sock, char* buf, int nbytes, char* owned_files[]) {

    int j;
    for (j = 0; j < 40 && buf[j] != ' '; j++);
    char* request = (char*)malloc(j+1);
    char* file_name = (char*)malloc(nbytes - j);
    for (int k = 0; k < j; k++) {
        request[k] = buf[k];
    }
    for (int k = 0; k < nbytes - j - 1; k++) {
        file_name[k] = buf[j+1+k];
    }


    if (strcmp(request, "download") == 0) {
        write(1, "sending download request...\n", sizeof("sending download request...\n"));
        send(sock, buf, strlen(buf), 0);
        download(sock, file_name);
        for (int i = 0; i < MAX_OWNED_FILES; i++)
        if (owned_files[i] == 0) {
            owned_files[i] = request[1];
            break;
        }
    }

    else if (strcmp(request, "upload") == 0) {
//        upload(sock, request[1], buf);
        write(1, "sending upload request...\n", sizeof("sending upload request...\n"));
        int full_size = strlen(FILE_DIR) + strlen(file_name) + 1;
        char* full_file_name = (char*)malloc(full_size);
        for(int i = 0; i < strlen(FILE_DIR); i++) {
            full_file_name[i] = FILE_DIR[i];
        }
        for(int i = 0; i < strlen(file_name); i++) {
            full_file_name[i + strlen(FILE_DIR)] = file_name[i];
        }
        full_file_name[full_size] = '\0';

        int read_fd = open(full_file_name, O_RDONLY);
        if (read_fd < 0) {
            perror("send_file");
        }
        while (1) {
            char buf[FILE_CHUNK_SIZE] = {0};
            int nbytes = read(read_fd, buf, FILE_CHUNK_SIZE);
            if(nbytes > 0) {
                write(sock, buf, nbytes);
            }
            if (nbytes < FILE_CHUNK_SIZE)
                break;
        }
    }

    else
        write(1, "invalid request. try again\n", sizeof("invalid request. try again\n"));
}