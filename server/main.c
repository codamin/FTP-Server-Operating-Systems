#include "server.h"

void beat() {
    IsHeartBeating = true;
}

void process_new_request(int socket, fd_set *ptr);

int main(int argc, char* argv[]) {

    struct sockaddr_in server_addr, client_addr, hb_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(6000);

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

    if (bind(listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("\nBinding error...\n");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_socket, 5) < 0) {
        perror("Listen error...");
        exit(EXIT_FAILURE);
    }
    int pid;
//    int hb_port = atoi(argv[1]);
    int hb_sock;


    hb_sock = create_heart_beat_socket(htons(10000), &hb_addr);

    fd_set readfds;

    for(int i = 0; i < MAX_CLIENTS; i++)
        clients[i] = 0;
    int buflen;

    int nbytes;
    char* char_port = "6000";
    signal(SIGALRM, beat);
    alarm(1);

    while (1) {
        
        if (IsHeartBeating) {
            if ((nbytes = sendto(hb_sock, char_port, strlen(char_port), 0, (struct sockaddr *)&hb_addr, sizeof hb_addr)) < 0) {
                perror("HeartBeat");
                exit(1);
            }
            printf("doop doop...\n");
            IsHeartBeating = false;
            alarm(1);
        }

        FD_ZERO(&readfds);
        FD_SET(listen_socket, &readfds);

        int max_sd = listen_socket;
        for (int i = 0 ; i < MAX_CLIENTS ; i++) {
            int sd = clients[i];
            if(sd > 0)
                FD_SET(sd , &readfds);
            if(sd > max_sd)
                max_sd = sd;
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno == EINTR)) {
            continue;
        }

        if ((activity < 0) && (errno!=EINTR)) {
            char msg[] = "select failed";-
            write(1, msg, sizeof(msg));
        }

        int new_client, file;
        if (FD_ISSET(listen_socket, &readfds))
            new_client = create_new_connection(listen_socket);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i];

            if (FD_ISSET(sd, &readfds)) {
                char buf[30];
                int nbytes = recv(sd, buf, 30, 0);
                buf[nbytes] = '\0';
                buflen = strlen(buf);
                if (nbytes <= 0) {
                    // if (nbytes == 0) {
                    //     close(i);
                    //     clients[i] = 0;
                    // }
                    // else
                    //     perror("reading client request failed");
                }
                else {
                    int j;
                    for (j = 0; j < 50 && buf[j] != ' '; j++);
                    char* request = (char*)malloc(j+1);
                    for (int k = 0; k < j; k++) {
                        request[k] = buf[k];
                    }
                    request[j] = '\0';

                    char* file_name = (char*)malloc(buflen - j);
                    for (int k = 0; k < buflen - j; k++) {
                        file_name[k] = buf[j + 1 + k];
                    }

                    if(strcmp(request, "download") == 0) {
                        file = server_hand_shake(clients[i], file_name, 0);
                        if (file > 0)
                            upload(clients[i], file);
                    }

                    else if(strcmp(request, "upload") == 0) {
                        file = server_hand_shake(clients[i], file_name, 1);
                            download(clients[i], file);
                        for (int i = 0; i < MAX_FILES; i++)
                            if (files[i] == 0) {
                            files[i] = file_name;
                            break;
                        }
                    }
                }
            }
        }
    }
    close(listen_socket);
    return 0;
}