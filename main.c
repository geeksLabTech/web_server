#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include "linked_list.h"

#define O_RDONLY         00

int main(int argc, char **argv)
{
    DIR *folder;
    struct dirent *entry;
    struct Node *first_file = NULL;
    int port = strtol(argv[1], NULL, 10);
    char *directory = argv[2];

    folder = opendir(directory);
    if(folder == NULL)
    {
        perror("Unable to read directory");
        return(1);
    }
    int files_count = 0;
    
    struct Node *head = NULL;
    struct Node *prev = head;
    char *full_path = NULL;

    while ((entry = readdir(folder)) != NULL) {
    
        struct Node *last = (struct Node*) malloc(sizeof(struct Node));
        full_path = malloc(strlen(directory) + strlen(entry->d_name));
        strcpy(full_path, directory);
        strcat(full_path, entry->d_name);
        last->directory = full_path;
        last->name = entry->d_name;
        last->next = NULL;
        if(head == NULL) {
            head = last;
            prev = last;
        } else {
            prev->next = last;
            prev = last;
        }
        //prev.next = &last;
        //prev = last;
        //printf("%s\n", entry->d_name);
        
    }

    closedir(folder);
    struct Node *root = head;
    while(root != NULL) {
        printf("%s\n", root->name);
        root = root->next;
    }

    return(0);
}