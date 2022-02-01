#include "web_server.h"


int main(int argc, char** argv) {

    char* port;
    if(argc == 3){
        chdir(argv[2]);
        if (getcwd(home_path, sizeof(home_path)) == NULL) {
        printf("Error reading home directory\n");
        }
        port = argv[1];
        printf("Running on %s on port %s\n", home_path, port);
    }
    else{

        if (getcwd(home_path, sizeof(home_path)) == NULL) {
        printf("Error reading home directory\n");
        }

        port = (char*)"8080";
    }

    SOCKET server = create_socket(0, port);

    while(1) {

        fd_set reads;
        reads = wait_on_clients(server);

        if (FD_ISSET(server, &reads)) {
            struct client_info *client = get_client(-1);

            client->socket = accept(server,
                    (struct sockaddr*) &(client->address),
                    &(client->address_length));

            if (!ISVALIDSOCKET(client->socket)) {
                fprintf(stderr, "accept() failed. (%d)\n",
                        GETSOCKETERRNO());
                return 1;
            }


            printf("New connection from %s.\n",
                    get_client_address(client));
        }


        struct client_info *client = clients;
        while(client) {
            struct client_info *next = client->next;

            if (FD_ISSET(client->socket, &reads)) {

                if (MAX_REQUEST_SIZE == client->received) {
                    send_400(client);
                    client = next;
                    continue;
                }

                int r = recv(client->socket,
                        client->request + client->received,
                        MAX_REQUEST_SIZE - client->received, 0);

                if (r < 1) {
                    printf("Unexpected disconnect from %s.\n",
                            get_client_address(client));
                    drop_client(client);

                } else {
                    client->received += r;
                    client->request[client->received] = 0;

                    char *q = strstr(client->request, "\r\n\r\n");
                    if (q) {
                        *q = 0;

                        if (strncmp("GET /", client->request, 5)) {
                            send_400(client);
                        } else {
                            char *path = client->request + 4;
                            char *end_path = strstr(path, " ");
                            if (!end_path) {
                                send_400(client);
                            } else {
                                *end_path = 0;
                                serve_resource(client, path);
                            }
                        }
                    } //if (q)
                }
            }

            client = next;
        }

    } //while(1)


    printf("\nClosing socket...\n");
    CLOSESOCKET(server);


    printf("Finished.\n");
    return 0;
}