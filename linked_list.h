#include <stdlib.h>
#include <string.h>

#ifndef Node
typedef struct Node{
    char *name;
    char *directory;
    long size;
    //File file; //this is the value the node stores
    struct Node *next; //this is the node the current node points to. this is how the nodes link
} Node;
#endif // !Node


