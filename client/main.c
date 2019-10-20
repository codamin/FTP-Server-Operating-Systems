#include "client.h"

int main(int argc, char* argv[]) {

    char* owned_files[MAX_OWNED_FILES] = {0};
    struct sockaddr_in server_addr, client_addr, hb_addr;

    client_addr.sin_family = AF_INET; 
    client_addr.sin_addr.s_addr = INADDR_ANY; 
//    client_addr.sin_port = htons(argv[1]);
    client_addr.sin_port = htons(7000);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    int server_listen_port;

//    int hb_port = argv[1];
    int hb_port = htons(10000);

    int hb_sock = init_heart_beat_listen_socket(hb_port);
    hb_addr.sin_family = AF_INET;
    hb_addr.sin_addr.s_addr = INADDR_ANY;
    hb_addr.sin_port = htons(hb_port); 
    bind(hb_sock, (struct sockaddr*)&hb_addr, sizeof(hb_addr));

    char buf[MAX_REQUEST_SIZE];
    int nbytes;

    int server_sock;
    int file;
    char* last_request;
    fd_set readfds;

    int io[MAX_IO] = {-1};
    io[0] = 0;
    io[1] = hb_sock;
    
    // int is_server_alive = 0;
    int is_server_alive = 1;


    struct timespec start, end;
    double elapsed_sec;


    server_addr.sin_port = htons(6000);
    server_sock = connect_to_server(client_addr, server_addr);

    while (1) {

        FD_ZERO(&readfds);

        if (is_server_alive)
            FD_SET(server_sock, &readfds);

        int max_sd = server_sock;

        for (int i = 0 ; i < MAX_IO ; i++) {
            int sd = io[i];
            FD_SET(sd , &readfds);
            if(sd > max_sd)
                max_sd = sd;
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno!=EINTR)) {
            char msg[] = "select failed\n";
            write(1, msg, sizeof(msg));
        }
        
        // clock_gettime(0, &end);
        // elapsed_sec = start.tv_sec - end.tv_sec;
        // if ((elapsed_sec >= 1) && FD_ISSET(hb_sock, &readfds)) {
        //     if (read_hb_msg(hb_sock, hb_addr, &server_listen_port, start) == 1) {
        //         if (!is_server_alive) {
        //             write(1, "server is alive\n", sizeof("server is alive\n"));
        //             server_addr.sin_port = htons(server_listen_port);
        //             is_server_alive = 1;
        //             server_sock = connect_to_server(client_addr, server_addr);
        //             for (int i = 0; i < MAX_IO; i++) {
        //                 if (io[i] == -1)
        //                     io[i] == server_sock;
        //             }
        //         }
        //     }
        // }

        // else {
        //     write(1, "heartbeat not found\n", sizeof("heartbeat not found\n"));
        //     close(server_sock);
        //     is_server_alive = 0;
        // }

        if (FD_ISSET(io[0], &readfds)) {
            nbytes = read(0, buf, sizeof(buf));
            buf[nbytes] = '\0';
//            char* clean_buf = (char*)malloc(nbytes);
//            for (int i = 0; i < nbytes - 1; i++) {
//                clean_buf[i] = buf[i];
//            }
//            clean_buf[nbytes] = '\0';
            if (is_server_alive) {
                last_request = buf;
                send_request_to_server(server_sock, buf, nbytes, owned_files);
            }
            else {
                //broadcast request
            }
        }
        
        if (is_server_alive) {
            if (FD_ISSET(server_sock, &readfds)) {
                int j;
                for (j = 0; j < 50 && buf[j] != ' '; j++);
                char* request = (char*)malloc(j+1);
                for (int k = 0; k < j; k++) {
                    request[k] = buf[k];
                }
                request[j] = '\0';

                char* file_name = (char*)malloc(nbytes - j + 1);
                for (int k = 0; k < nbytes - j; k++) {
                    file_name[k] = buf[j+1+k];
                }
                file_name[nbytes - j] = '\0';

                char ack[2];
                nbytes = read(server_sock, ack, sizeof(ack));
                // if (nbytes < 2) handle exception
                printf("1\n");
                // ack[2] = '\0';
                if (!strcmp(ack, "NO")) {
//                    broadcast(file_name);
                    //broadcast request
                }
                else if (!strcmp(ack, "YD")) {
                    file = open(file_name, O_CREAT | O_TRUNC | O_WRONLY, 0777);
                    download(server_sock, file);
                    for (int i = 0; i < MAX_OWNED_FILES; i++)
                    if (owned_files[i] == 0) {
                        owned_files[i] = file_name;
                        break;
                    }
                }
                else if (!strcmp(ack, "YU")) {
                    file = open(file_name, O_RDONLY);
                    if (file < 0) {
                        write(1, "file not found for upload", 25);
                    }
                    else
                        upload(server_sock, file);
                }
            }
        }   
    }
    return 0;
}