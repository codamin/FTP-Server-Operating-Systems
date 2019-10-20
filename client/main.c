#include "client.h"

int main(int argc, char* argv[]) {

    if (argc < 4) {
        write (1, "Not enough arguments\n", sizeof("Not enough arguments\n"));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr, client_addr, hb_addr, bc_addr;

    int broadcast_port = atoi(argv[2]);
    int bc_sock = create_broadcast_socket(broadcast_port, &bc_addr);
    
    char* owned_files[MAX_OWNED_FILES] = {0};

    client_addr.sin_family = AF_INET; 
    client_addr.sin_addr.s_addr = INADDR_ANY; 
    client_addr.sin_port = htons(atoi(argv[3]));

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
    io[0] = 0;
    io[1] = hb_sock;
    io[2] = bc_sock;
    
    int is_server_alive = 0;

    if (read_hb_msg(hb_sock, hb_addr, &server_listen_port) == 1) {
        is_server_alive = 1;
        server_addr.sin_port = htons(server_listen_port);
        server_sock = connect_to_server(client_addr, server_addr);
    }

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

        if (FD_ISSET(io[0], &readfds)) {
            nbytes = read(0, buf, sizeof(buf));
            buf[nbytes] = '\0';
            if (is_server_alive) {
                last_request = buf;
                send_request_to_server(server_sock, buf, nbytes, owned_files);
            }
            else {
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
                broadcast_request(file_name);
            }
        }
        
        if (is_server_alive && FD_ISSET(server_sock, &readfds)) {
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
                printf("was not found in server\n");
                broadcast_request(bc_sock, bc_addr, file_name);
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
                    write(1, "file not found for upload\n", 26);
                }
                else
                    upload(server_sock, file);
            }
        }
        
    }
    return 0;
}