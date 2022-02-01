


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
#include "client_info.h"
#include "linked_list.h"



#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)

#define GETSOCKETERRNO() (errno)


extern char home_path[PATH_MAX];
extern struct client_info *clients;