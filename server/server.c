#include "server.h"

int create_heart_beat_socket(int hb_port, struct sockaddr_in* hb_addr) {

    int hb_sock;
    if ((hb_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("creating heart beat socket failed");
        exit(EXIT_FAILURE);
    }

    hb_addr->sin_family = AF_INET;
    hb_addr->sin_addr.s_addr = INADDR_ANY;
    hb_addr->sin_port = htons(hb_port);

    int reuse = 1;
    if (setsockopt(hb_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    int broadcast = 1;
    if (setsockopt(hb_sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) < 0) {
        perror("setsockopt (SO_BROADCAST)");
        exit(1);
    }

    bind(hb_sock, (struct sockaddr*)hb_addr, sizeof(*hb_addr));
    return hb_sock;
}

int server_hand_shake(int sock, char* full_file_name, int mode) {

    // 0 for upload ----> must exist ---> O_RDONLY
    // 1 for download --> just open ---> O_CREAT
    int fd;
    if (mode)
        fd = open(full_file_name, O_CREAT | O_TRUNC | O_WRONLY, 0777);
    else if (!mode)
        fd = open(full_file_name, O_RDONLY);

    if (fd < 0) {
        write(sock, "NO", 2);
        write(1,"file not found\n", 15);
        return -1;
    }

    if (mode)
        write(sock, "YU", 2);
    else
        write(sock, "YD", 2);
    return fd;
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