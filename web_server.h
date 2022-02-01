


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "client_info.h"
#include "linked_list.h"



#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)

#define GETSOCKETERRNO() (errno)


extern char home_path[PATH_MAX];
extern struct client_info *clients;

const char *get_content_type(const char* path);
SOCKET create_socket(const char* host, const char *port);
struct client_info *get_client(SOCKET s);
void drop_client(struct client_info *client);
const char *get_client_address(struct client_info *ci);
fd_set wait_on_clients(SOCKET server);
void send_400(struct client_info *client);
void send_404(struct client_info *client);
char* html_template(const char *path);
void serve_resource(struct client_info *client, const char *path);