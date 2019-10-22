#include "client.h"

int init_heart_beat_listen_socket(int hb_port) {

    int sock;
    
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        write(1, "udp socket", sizeof("udp socket"));
        exit(EXIT_FAILURE);
    }
    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
        write(1, "setsockopt(SO_REUSEADDR) failed", sizeof("setsockopt(SO_REUSEADDR) failed"));
 
    int broadcast = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        write(1, "setsockopt (SO_BROADCAST)", sizeof("setsockopt (SO_BROADCAST)"));
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
        write(1, "Failed to open socket", sizeof("Failed to open socket"));
    }

    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
        write(1, "setsockopt(SO_REUSEPORT) failed\n", sizeof("setsockopt(SO_REUSEPORT) failed\n"));

    if (bind(sock, (struct sockaddr*) &client_addr, sizeof(struct sockaddr_in)) < 0) {
        write(1, "client socket binding failed\n", sizeof("client socket binding failed\n"));
        exit(EXIT_FAILURE); 
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) { 
        write(1, "Connect() failed \n", sizeof("Connect() failed \n")); 
        return -1; 
    }
    return sock;
}


void send_request_to_server(int sock, char* buf, int buflen) {

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

    int bc_sock;
    if ((bc_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        write(1, "creating heart beat socket failed", sizeof("creating heart beat socket failed"));
        exit(EXIT_FAILURE);
    }

    bc_addr->sin_family = AF_INET;
    bc_addr->sin_addr.s_addr = INADDR_BROADCAST;
    bc_addr->sin_port = htons(bc_port);

    int reuse = 1;
    if (setsockopt(bc_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
        write(1, "setsockopt(SO_REUSEADDR) failed", sizeof("setsockopt(SO_REUSEADDR) failed"));

    int broadcast = 1;
    if (setsockopt(bc_sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) < 0) {
        write(1, "setsockopt (SO_BROADCAST)", sizeof("setsockopt (SO_BROADCAST)"));
        exit(1);
    }

    bind(bc_sock, (struct sockaddr*)bc_addr, sizeof(*bc_addr));
    return bc_sock;
}

void broadcast_request(int bc_sock, struct sockaddr_in bc_addr, char* file_name, int ss_listen_port) {

    int nbytes;
    char request[40];

    char itos[10];
    itoa(ss_listen_port, itos, 10);

    for (int i = 0; i < strlen(itos); i++) {
        request[i] = itos[i];
    }
    request[strlen(itos)] = ' ';

    for (int i = 0; i < strlen(file_name); i++) {
        request[strlen(itos) + 1 + i] = file_name[i];
    }
    request[strlen(itos) + strlen(file_name) + 1] = '\0';

    if ((nbytes = sendto(bc_sock, request, strlen(request), 0, (struct sockaddr *)&bc_addr, sizeof bc_addr)) < 0) {
        write(1, "sendto", sizeof("sendto"));
        exit(1);
    }
    else {
        write(1, "request broadcasted......\n", sizeof("request broadcasted......\n"));
    }
}

int create_socket_to_listen(struct sockaddr_in file_reciever_addr) {

    int listen_socket;

    if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        write(1, "openning socket failed\n", sizeof("openning socket failed\n"));
        exit(EXIT_FAILURE);
    }

    int reuse = 1;
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEPORT, (char *)&reuse, sizeof(reuse)) < 0) {
        write(1, "setsockopt", sizeof("setsockopt"));
        exit(EXIT_FAILURE);
    }

    if (bind(listen_socket, (struct sockaddr *) &file_reciever_addr, sizeof(file_reciever_addr)) < 0) {
        write(1, "\nBinding error...\n", sizeof("\nBinding error...\n"));
        exit(EXIT_FAILURE);
    }

    if (listen(listen_socket, 10) < 0) {
        write(1, "Listen error...\n", sizeof("Listen error...\n"));
        exit(EXIT_FAILURE);
    }
    return listen_socket;
}


int create_socket_to_send_file(struct sockaddr_in file_sender_addr, struct sockaddr_in other_reciever_addr) {

    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        write(1, "Failed to open socket", sizeof("Failed to open socket"));
    }

    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
        write(1, "setsockopt(SO_REUSEPORT) failed", sizeof("setsockopt(SO_REUSEPORT) failed"));
    int reuse2 = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse2, sizeof(reuse2)) < 0)
        write(1, "setsockopt(SO_REUSEADDR) failed", sizeof("setsockopt(SO_REUSEADDR) failed"));
    if (bind(sock, (struct sockaddr*) &file_sender_addr, sizeof(file_sender_addr)) < 0) {
        write(1, "\nclient socket binding failed\n", sizeof("\nclient socket binding failed\n"));
        exit(EXIT_FAILURE); 
    }

    if (connect(sock, (struct sockaddr *)&other_reciever_addr, sizeof(other_reciever_addr)) < 0) { 
        write(1, "Connect() failed\n", sizeof("Connect() failed\n"));
        return -1; 
    }
    return sock;
}

int generate_random_port () {
    srand(time(NULL));
    int random_port = rand()% TOTAL_PORTS + 1024;
    return random_port;
}