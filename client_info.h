#include <netinet/in.h>


#define MAX_REQUEST_SIZE 2047
#define SOCKET int

struct client_info {
    socklen_t address_length;
    struct sockaddr_storage address;
    SOCKET socket;
    char request[MAX_REQUEST_SIZE + 1];
    int received;
    struct client_info *next;
};