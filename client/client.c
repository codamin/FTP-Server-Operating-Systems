#include "client.h"

int init_heart_beat_listen_socket(int hb_port) {

    int sock;
    
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("udp socket");
        exit(EXIT_FAILURE);
    }
    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");
 
    int broadcast = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        perror("setsockopt (SO_BROADCAST)");
        exit(EXIT_FAILURE);
    }
    struct timeval timeout={2, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval));
    return sock;
}

int read_hb_msg(int sock, struct sockaddr_in hb_addr, int* server_listen_port) {

    int nbytes, len;
    char buf[MAX_HB_MSG_SIZE];
    if ((nbytes = recvfrom(sock, buf, MAX_HB_MSG_SIZE, 0 ,(struct sockaddr *) &hb_addr, &len)) < 0)
        return -1;
    char msg[] = "************************************Server Found**************************************\n";
    write(1, msg, sizeof(msg));
    *server_listen_port = atoi(buf);
    return 1;
}

int client_hand_shake(int sock, char* buf) {

    char ready_buf[1];
    int nbytes;
    while(1) {
        nbytes = read(sock, ready_buf, 1);
        if (nbytes > 0) {
            if (ready_buf[0] == 'Y')
                return 1;
            else if (ready_buf[0] == 'N') {
                write(1, "download failed", 16);
                return -1;
            }
        }
    }
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


void send_request_to_server(int sock, char* buf, int nbytes, char* owned_files[]) {

    int j;
    for (j = 0; j < 50 && buf[j] != ' '; j++);
    char* request = (char*)malloc(j+1);
    for (int k = 0; k < j; k++) {
        request[k] = buf[k];
    }
    request[j] = '\0';

    if (strcmp(request, "download") == 0) {

        write(1, "sending download request...\n", sizeof("sending download request...\n"));
        send(sock, buf, strlen(buf), 0);
    }

    else if (strcmp(request, "upload") == 0) {
        write(1, "sending upload request...\n", sizeof("sending upload request...\n"));
        send(sock, buf, strlen(buf), 0);
    }
    else
        write(1, "invalid request. try again\n", sizeof("invalid request. try again\n"));
}

int create_broadcast_socket(int bc_port, struct sockaddr_in* bc_addr) {

    int hb_sock;
    if ((hb_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("creating heart beat socket failed");
        exit(EXIT_FAILURE);
    }

    bc_addr->sin_family = AF_INET;
    bc_addr->sin_addr.s_addr = INADDR_ANY;
    bc_addr->sin_port = htons(bc_port);

    int reuse = 1;
    if (setsockopt(hb_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    int broadcast = 1;
    if (setsockopt(hb_sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) < 0) {
        perror("setsockopt (SO_BROADCAST)");
        exit(1);
    }

    bind(hb_sock, (struct sockaddr*)bc_addr, sizeof(*bc_addr));
    return hb_sock;
}

void broadcast_request(int bc_sock, struct sockaddr_in bc_addr, char* file_name) {

    int nbytes;
    if ((nbytes = sendto(bc_sock, file_name, strlen(file_name), 0, (struct sockaddr *)&bc_addr, sizeof bc_addr)) < 0) {
        perror("sendto");
        exit(1);
    }
    else {
        printf("msg broadcasted......\n");
    }
}

void create_socket_to_listen(struct sockaddr_in client_addr) {

    int listen_socket;

    if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("openning socket failed\n");
        exit(EXIT_FAILURE);
    }

    int reuse = 1;
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEPORT, (char *)&reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (bind(listen_socket, (struct sockaddr *) &client_addr, sizeof(client_addr)) < 0) {
        perror("\nBinding error...\n");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_socket, 1) < 0) {
        perror("Listen error...");
        exit(EXIT_FAILURE);
    }
    return listen_socket;
}
