#include "server.h"

void process_new_request(int socket, fd_set *ptr);

send_hearbeat() {

    int heart_beat_socket;
    if((heart_beat_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("heartbeat_socket failed");
        exit(EXIT_FAILURE);
    };      
}

int main(int argc, char* argv[]) {

    int opt = 1;

    struct sockaddr_in server_addr, client_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = 8080;
    
    int listen_socket;

    if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("openning socket failed\n");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {   
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

    fd_set readfds;

    for(int i = 0; i < MAX_CLIENTS; i++)
        clients[i] = 0;

    while (1) {

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
        
        if ((activity < 0) && (errno!=EINTR)) {
            char msg[] = "select failed";
            write(1, msg, sizeof(msg));
        }
        
        int new_client;
        if (FD_ISSET(listen_socket, &readfds))
            new_client = create_new_connection(listen_socket);

        // process_new_request(listen_socket, &readfds);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i];

            if (FD_ISSET(sd, &readfds)) {
                char buf[30];
                int nbytes = recv(sd, buf, 30, 0);
                if (nbytes <= 0) {
                    if (nbytes == 0) {
                        close(i);
                        clients[i] = 0;
                    }
                    else
                        perror("reading client request failed");
                }
                else {

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

                    if(strcmp(request, "download") == 0) {
                        upload(clients[i], file_name);
                    }
                    else if(strcmp(request, "upload") == 0) {
                        get_file(clients[i]);
//                        add_file_to_files()
                    }
                }
            }
        }
    }
    close(listen_socket);
    return 0;
}