
#include "web_server.h"


char home_path[PATH_MAX];

const char *get_content_type(const char* path) {
    const char *last_dot = strrchr(path, '.');
    if (last_dot) {
        if (strcmp(last_dot, ".css") == 0) return "text/css";
        if (strcmp(last_dot, ".csv") == 0) return "text/csv";
        if (strcmp(last_dot, ".gif") == 0) return "image/gif";
        if (strcmp(last_dot, ".htm") == 0) return "text/html";
        if (strcmp(last_dot, ".html") == 0) return "text/html";
        if (strcmp(last_dot, ".ico") == 0) return "image/x-icon";
        if (strcmp(last_dot, ".jpeg") == 0) return "image/jpeg";
        if (strcmp(last_dot, ".jpg") == 0) return "image/jpeg";
        if (strcmp(last_dot, ".js") == 0) return "application/javascript";
        if (strcmp(last_dot, ".json") == 0) return "application/json";
        if (strcmp(last_dot, ".png") == 0) return "image/png";
        if (strcmp(last_dot, ".pdf") == 0) return "application/pdf";
        if (strcmp(last_dot, ".svg") == 0) return "image/svg+xml";
        if (strcmp(last_dot, ".txt") == 0) return "text/plain";
    }

    return "application/octet-stream";
}


SOCKET create_socket(const char* host, const char *port) {
    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    getaddrinfo(host, port, &hints, &bind_address);

    printf("Creating socket...\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family,
            bind_address->ai_socktype, bind_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_listen)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }

    printf("Binding socket to local address...\n");
    int one = 1;
    setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (bind(socket_listen,
                bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }
    freeaddrinfo(bind_address);

    printf("Listening...\n");
    if (listen(socket_listen, 10) < 0) {
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }

    return socket_listen;
}


struct client_info *clients = 0;

// Takes a SOCKET variable and searches our linked list for the
// corresponding client_info data structure
// If no matching client_info is found in the linked list,
// then a new client_info is allocated and added to the linked list
struct client_info *get_client(SOCKET s) {
    struct client_info *ci = clients;

    while(ci) {
        if (ci->socket == s)
            break;
        ci = ci->next;
    }

    if (ci) return ci;
    struct client_info *n =
        (struct client_info*) calloc(1, sizeof(struct client_info));

    if (!n) {
        fprintf(stderr, "Out of memory.\n");
        exit(1);
    }

    n->address_length = sizeof(n->address);
    n->next = clients;
    clients = n;
    return n;
}


// closes the connection to a client and removes it from the clients linked list
// It must be called when we finish sending a client a resource,
// and it also must be called when we finish sending a client an error message
void drop_client(struct client_info *client) {
    CLOSESOCKET(client->socket);

    struct client_info **p = &clients;

    while(*p) {
        if (*p == client) {
            *p = client->next;
            free(client);
            return;
        }
        p = &(*p)->next;
    }

    fprintf(stderr, "drop_client not found.\n");
    exit(1);
}


// returns a client's IP address as a string (character array)
const char *get_client_address(struct client_info *ci) {
    static char address_buffer[100];
    getnameinfo((struct sockaddr*)&ci->address,
            ci->address_length,
            address_buffer, sizeof(address_buffer), 0, 0,
            NI_NUMERICHOST);
    return address_buffer;
}



// uses the select() function to wait until either a client has
// data available or a new client is attempting to connect
fd_set wait_on_clients(SOCKET server) {
    fd_set reads;
    FD_ZERO(&reads);
    FD_SET(server, &reads);
    SOCKET max_socket = server;

    struct client_info *ci = clients;

    while(ci) {
        FD_SET(ci->socket, &reads);
        if (ci->socket > max_socket)
            max_socket = ci->socket;
        ci = ci->next;
    }

    if (select(max_socket+1, &reads, 0, 0, 0) < 0) {
        fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }

    return reads;
}


void send_400(struct client_info *client) {
    const char *c400 = "HTTP/1.1 400 Bad Request\r\n"
        "Connection: close\r\n"
        "Content-Length: 11\r\n\r\nBad Request";
    send(client->socket, c400, strlen(c400), 0);
    drop_client(client);
}

void send_404(struct client_info *client) {
    const char *c404 = "HTTP/1.1 404 Not Found\r\n"
        "Connection: close\r\n"
        "Content-Length: 9\r\n\r\nNot Found";
    send(client->socket, c404, strlen(c404), 0);
    drop_client(client);
}

char* html_template(const char *path){

    DIR *folder;
    struct dirent *entry;
    struct Node *first_file = NULL;
    int port;

    folder = opendir(path);
    if(folder == NULL)
    {
        perror("Unable to read directory");
        return((char*)"ERROR 400\n");
    }
    int files_count = 0;
    
    struct Node *head = NULL;
    struct Node *prev = head;
    char *full_path = NULL;
    
    struct Node *root = head;
    printf("%s\n", path);
    FILE *fp = freopen("index.html", "w", stdout);
    printf("<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport'><title>Mobuis Server</title><script src=\"../static/js/sorttable.js\"></script><link rel=\"stylesheet\" type=\"text/css\" href=\"../static/css/style.css\"></head><body><table class=\"searchable sortable\"><colgroup><col style=\"width: 227.56px;\"><col style=\"width: 53.56px;\"><col style=\"width: 145.56px;\"><col style=\"width: 139.56px;\"></colgroup><thead><tr style=\"background-color: #ebebeb;\"><th class=\"header\">Name</th><th class=\"header\">Size</th></tr></thead><tbody>\n");

    while ((entry = readdir(folder)) != NULL) {

        long size;
        char full_path[strlen(path)+strlen(entry->d_name)+1];
        // full_path = (char *)malloc(strlen(path) + strlen(entry->d_name) + 1);
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, entry->d_name);
        struct stat st;
        if (stat(full_path, &st) == 0)
            size = st.st_size;
        else
            size =  -1;
        printf("<tr><td><a href=\"%s\">%s</a></td><td>%ld</td>\n", full_path, entry->d_name, size);
        // if(full_path != NULL)
        // free(full_path);
    }
    

    closedir(folder);
    printf("</tbody></table></body>\n");
    fclose(fp);

    return ((char *)"200 Success\n");
}

// attempts to transfer a file to a connected client
void serve_resource(struct client_info *client, const char *path) {

    if(strcmp("/", path) == 0){
        path = home_path;
    }
    struct stat s;
    if( stat(path,&s) == 0 )
    {
        if( s.st_mode & S_IFDIR )
        {

            char* status = html_template(path);
            path = "index.html";
        }
    }

    printf("serve_resource %s %s\n", get_client_address(client), path);

    if (strlen(path) > 100) {
        send_400(client);
        return;
    }

    if (strstr(path, "..")) {
        send_404(client);
        return;
    }

    char full_path[128];
    sprintf(full_path, "%s", path);

    printf("%s\n", full_path);
    FILE *fp = fopen(full_path, "rb");

    if (!fp) {
        send_404(client);
        return;
    }

    fseek(fp, 0L, SEEK_END);
    size_t cl = ftell(fp);
    rewind(fp);

    const char *ct = get_content_type(full_path);

#define BSIZE 1024
    char buffer[BSIZE];

    sprintf(buffer, "HTTP/1.1 200 OK\r\n");
    send(client->socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Connection: close\r\n");
    send(client->socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Content-Length: %lu\r\n", cl);
    send(client->socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "Content-Type: %s\r\n", ct);
    send(client->socket, buffer, strlen(buffer), 0);

    sprintf(buffer, "\r\n");
    send(client->socket, buffer, strlen(buffer), 0);

    int r = fread(buffer, 1, BSIZE, fp);
    while (r) {
        send(client->socket, buffer, r, 0);
        r = fread(buffer, 1, BSIZE, fp);
    }

    fclose(fp);
    drop_client(client);
}




