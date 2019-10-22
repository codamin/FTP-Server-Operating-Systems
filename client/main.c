#include "client.h"

void handle_reBroadCast_signal() {
    broadcast_request_flag = 1;
}

int main(int argc, char* argv[]) {

    if (argc < 4) {
        write (1, "Not enough arguments\n", sizeof("Not enough arguments\n"));
        exit(EXIT_FAILURE);
    }
    struct timespec start,end;

    struct sockaddr_in server_addr, client_addr, hb_addr, bc_addr, file_sender_addr, file_reciever_addr, other_reciever_addr;

    int broadcast_port = atoi(argv[2]);
    int bc_sock = create_broadcast_socket(broadcast_port, &bc_addr);
    
    client_addr.sin_family = AF_INET; 
    client_addr.sin_addr.s_addr = INADDR_ANY; 
    client_addr.sin_port = htons(atoi(argv[3]));

    file_sender_addr.sin_family = AF_INET;
    file_sender_addr.sin_addr.s_addr = INADDR_ANY;
    file_sender_addr.sin_port = htons(generate_random_port());

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    int server_listen_port;

    int hb_port = htons(atoi(argv[1]));
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

    
    int is_server_alive = 0;
    int file_found = -1;

    if (read_hb_msg(hb_sock, hb_addr, &server_listen_port) == 1) {
        is_server_alive = 1;
        server_addr.sin_port = htons(server_listen_port);
        server_sock = connect_to_server(client_addr, server_addr);
    }
    else {

        char msg[] = "************************************Server Not Found**************************************\n";
        write(1, msg, sizeof(msg));
    }

    file_reciever_addr.sin_family = AF_INET;
    file_reciever_addr.sin_addr.s_addr = INADDR_ANY;
    int random_port_for_listen = generate_random_port();
    file_reciever_addr.sin_port = htons(random_port_for_listen);

    int file_reciever_socket = create_socket_to_listen(file_reciever_addr);

    int file_sender_sock;

    char last_sec_scenario_sent_file_name[50];

    int buflen;
    io[0] = 0;
    io[1] = hb_sock;
    io[2] = bc_sock;
    io[3] = file_reciever_socket;

    int accepted_socket_for_recieving;
    int sec_scenario_sock;

    int still_broadcast = 0;

    signal(SIGALRM, handle_reBroadCast_signal);
    alarm(2);

    while (1) {

        if (broadcast_request_flag && still_broadcast) {
            broadcast_request(bc_sock, bc_addr, last_sec_scenario_sent_file_name, htons(random_port_for_listen));
            broadcast_request_flag = 0;
            alarm(2);
        }

        FD_ZERO(&readfds);

        if (is_server_alive)
            FD_SET(server_sock, &readfds);

        int max_sd = bc_sock;

        for (int i = 0 ; i < MAX_IO ; i++) {
            int sd = io[i];
            FD_SET(sd, &readfds);
            if(sd > max_sd)
                max_sd = sd;
        }
        // struct timeval to = {0,0};
        // int activity = select(max_sd + 1, &readfds, NULL, NULL, &to);
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno == EINTR)) {
            continue;
        }

        if (FD_ISSET(file_reciever_socket, &readfds)) {            
            struct sockaddr_in new_client_address;
            int addrlen = sizeof new_client_address;
            if ( (accepted_socket_for_recieving = accept(file_reciever_socket, (struct sockaddr*) &new_client_address, (socklen_t*) &addrlen)) < 0) {
                char msg[] = "create_new_connection: creating new connection failed";
                write(1, msg, sizeof(msg));
                exit(EXIT_FAILURE);
            }
            else if (still_broadcast){
                still_broadcast = 0;
                write(accepted_socket_for_recieving, "Y", 1);
                write(1, "new connection\n", sizeof("new connection\n"));
                file = open(last_sec_scenario_sent_file_name, O_CREAT | O_TRUNC | O_WRONLY, 0777);
                download(accepted_socket_for_recieving, file);
                close(accepted_socket_for_recieving);
                file_found = -1;
            }
            else {
                write(accepted_socket_for_recieving, "N", 1);
                close(accepted_socket_for_recieving);
            }
        }        

        if (FD_ISSET(io[0], &readfds)) {

            nbytes = read(0, buf, sizeof(buf));
            buf[nbytes - 1] = '\0';
            buflen = strlen(buf);
            if (is_server_alive) {
                send_request_to_server(server_sock, buf, buflen);
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
                    file_name[k] = buf[j+1+k];
                }
                strcpy(last_sec_scenario_sent_file_name, file_name);
                for (int i = 0; i < strlen(file_name); i++) {
                    last_sec_scenario_sent_file_name[i] = file_name[i];
                }            
                last_sec_scenario_sent_file_name[strlen(file_name)] = '\0';
                still_broadcast = 1;
                // broadcast_request(bc_sock, bc_addr, file_name, htons(random_port_for_listen));
            }
        }

        if (is_server_alive && FD_ISSET(server_sock, &readfds)) {

            char* request;
            char* file_name;
            int j;
            for (j = 0; j < 50 && buf[j] != ' '; j++);
            request = (char*)malloc(j+1);
            for (int k = 0; k < j; k++) {
                request[k] = buf[k];
            }
            request[j] = '\0';

            file_name = (char*)malloc(buflen - j);
            for (int k = 0; k < buflen - j; k++) {
                file_name[k] = buf[j+1+k];
            }

            char ack[2];
            nbytes = read(server_sock, ack, sizeof(ack));
            if (!strcmp(ack, "NO")) {
                file_found = 0;
                write(1, "was not found in server\n", sizeof("was not found in server\n"));

                strcpy(last_sec_scenario_sent_file_name, file_name);
                for (int i = 0; i < strlen(file_name); i++) {
                    last_sec_scenario_sent_file_name[i] = file_name[i];
                }
                still_broadcast = 1;
                // broadcast_request(bc_sock, bc_addr, file_name, htons(random_port_for_listen));
            }
            else if (!strcmp(ack, "YD")) {
                file_found = 1;
                file = open(file_name, O_CREAT | O_TRUNC | O_WRONLY, 0777);
                download(server_sock, file);
            }
            else if (!strcmp(ack, "YU")) {
                file_found = 1;
                file = open(file_name, O_RDONLY);
                if (file < 0) {
                    write(1, "file not found for upload\n", 26);
                }
                else {
                    upload(server_sock, file);
                }
            }
        } 

        if (FD_ISSET(bc_sock, &readfds)) {
            struct sockaddr_in from;
            int nbytes, len = sizeof from;
            char comming_req[MAX_BC_MSG_SIZE];
            if ((nbytes = recvfrom(bc_sock, comming_req, MAX_BC_MSG_SIZE, 0, (struct sockaddr *) &from, &len)) < 0) {
                write(1, "error in recv broadcast", sizeof("error in recv broadcast"));
            }
            comming_req[nbytes]='\0';
            int req_len = strlen(comming_req);
            int j;
            for (j = 0; j < 50 && comming_req[j] != ' '; j++);
            char* port = (char*)malloc(j+1);
            for (int k = 0; k < j; k++) {
                port[k] = comming_req[k];
            }
            port[j] = '\0';
            
            if (atoi(port) != htons(random_port_for_listen)) {
                char* file_name = (char*)malloc(req_len - j);
                for (int k = 0; k < req_len - j; k++) {
                    file_name[k] = comming_req[j+1+k];
                }
                int file = open(file_name, O_RDONLY);
                if (file < 0) 
                    write(1, "file not found for upload\n", 26);
                else {
                    other_reciever_addr.sin_family = AF_INET;
                    other_reciever_addr.sin_addr.s_addr = INADDR_ANY;
                    other_reciever_addr.sin_port = atoi(port);
                    file_sender_sock = create_socket_to_send_file(file_sender_addr, other_reciever_addr);
                    if (file_sender_sock > 0) {
                        char ack[1];
                        int nbytes = read(file_sender_sock, ack, sizeof(ack));

                        if (ack[0] == 'Y') {
                            upload(file_sender_sock, file);
                        }
                        else {
                            write(1, "not allowed for me!\n", sizeof("not allowed for me!\n"));
                        }
                        close(file_sender_sock);
                    }
                }
            }
        }
    }
    return 0;
}
