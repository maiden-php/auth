#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ftree.h"

struct client
{
    int fd;
    int state;
    struct request req;
    struct in_addr ipaddr;
    struct client *next;
};

static struct client *addclient(struct client *top, int fd, struct in_addr addr);
static struct client *removeclient(struct client *top, int fd);

int handleclient(struct client *p, struct client *top);
int bindandlisten(void);

void rcopy_server(unsigned short port)
{
    int clientfd, maxfd, nready;
    struct client *p;
    struct client *head = NULL;
    socklen_t len;
    struct sockaddr_in q;
    struct timeval tv;
    fd_set allset;
    fd_set rset;

    int i;

    int listenfd = bindandlisten();
    // initialize allset and add listenfd to the
    // set of file descriptors passed into select
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    // maxfd identifies how far into the set to search
    maxfd = listenfd;

    while (1)
    {
        // make a copy of the set before we pass it into select
        rset = allset;
        /* timeout in seconds (You may not need to use a timeout for
        * your assignment)*/
        tv.tv_sec = 10;
        tv.tv_usec = 0; /* and microseconds */

        nready = select(maxfd + 1, &rset, NULL, NULL, &tv);
        if (nready == 0)
        {
            printf("No response from clients in %ld seconds\n", tv.tv_sec);
            continue;
        }
        //printf("Time says %ld seconds\n", tv.tv_sec);

        if (nready == -1)
        {
            perror("select");
            continue;
        }

        if (FD_ISSET(listenfd, &rset))
        {
            printf("a new client is connecting\n");
            len = sizeof(q);
            if ((clientfd = accept(listenfd, (struct sockaddr *)&q, &len)) < 0)
            {
                perror("accept");
                exit(1);
            }
            FD_SET(clientfd, &allset);
            if (clientfd > maxfd)
            {
                maxfd = clientfd;
            }
            printf("connection from %s\n", inet_ntoa(q.sin_addr));
            head = addclient(head, clientfd, q.sin_addr);
        }

        for (i = 0; i <= maxfd; i++)
        {
            if (FD_ISSET(i, &rset))
            {
                for (p = head; p != NULL; p = p->next)
                {
                    if (p->fd == i)
                    {
                        int result = handleclient(p, head);
                        if (result == -1)
                        {
                            printf("REMOVE CLIENT\n");
                            int tmp_fd = p->fd;
                            head = removeclient(head, p->fd);
                            FD_CLR(tmp_fd, &allset);
                            close(tmp_fd);
                        }
                        break;
                    }
                }
            }
        }
    }
}

int handleclient(struct client *p, struct client *top) {
    int bytes_read = 0;
    int32_t type, size;
    mode_t mode;
    char buffer[MAXPATH];
    char dest_hash[BLOCKSIZE];

    switch (p->state) {
        case AWAITING_TYPE:
            bytes_read = read(p->fd, &type, sizeof(int32_t));
            if(bytes_read < 0) {
                fprintf(stderr, "Error reading type.\n");
                perror("type");
                return -1;
            }
            else if(bytes_read > 0) {
                p->state = AWAITING_PATH;
                p->req.type = ntohl(type);
                printf("Received type %d bytes: %d\n", bytes_read, p->req.type);
            }
            break;

        case AWAITING_PATH:
            bytes_read = read(p->fd, &buffer, sizeof(char) * MAXPATH);
            if(bytes_read < 0) {
                fprintf(stderr, "Error reading path.\n");
                perror("path");
                return -1;
            }
            else if(bytes_read > 0) {
                p->state = AWAITING_PERM;
				buffer[bytes_read] = '\0';
                strcpy(p->req.path, buffer);

                printf("Received path %d bytes: %s\n", bytes_read, p->req.path);
            }
            break;
        
        case AWAITING_SIZE:
            bytes_read = read(p->fd, &size, sizeof(int32_t));
            if(bytes_read < 0) {
                fprintf(stderr, "Error reading size.\n");
                perror("size");
                return -1;
            }
            else if(bytes_read > 0) {
                p->req.size = ntohl(size);
                printf("Received size %d bytes: %d\n", bytes_read, p->req.size);

                // Check if we've received a folder or file
                if(p->req.type == REGDIR) {
                    // Send response to client.
                    printf("SENDING: %d to client\n", OK);
                    int32_t value = htonl(OK);
                    write(p->fd, &value, sizeof(int32_t));
                }
                else if(p->req.type == REGFILE) {
                    int32_t value = OK;

                    // Check if file exists and if it's the same file
                    char *dest_path = p->req.path;

                    struct stat dest_st;
                    if (stat(dest_path, &dest_st) < 0) {
                        fprintf(stderr, "%s\n", dest_path);
                        perror("Error reading information about dest file");

                        // Skip everything else except freeing memory for the source path.
                        //goto free_memory;
                    }

                    // Check if the file exists in the destination directory.
                    if (errno == ENOENT) {
                        // File doesn't exist - request copy.
                        value = SENDFILE;
                    }
                    else {
                        // If files don't have the same size we should just copy them and skip the hash calculation
                        // which can be slow.
                        if (dest_st.st_size != p->req.size) {
                            // Sizes don't match - request copy of the file.
                            value = SENDFILE;
                        }
                        else {
                            // Retrieve file hashes for comparison.
                            //char *src_hash = p->req.hash;
                            hash_file_name(dest_hash, dest_path);

                            // Source hash has to exist but destination hash might not, that's why we only check for
                            // errors on the source hash.
                            if (p->req.path == NULL) {
                                printf("%s\n", p->req.path);
                                perror("Error reading hash from source");
                            }
                            // Compare hashes to see if they match.
                            else if (check_hash(p->req.hash, dest_hash) != 0) {
                                // request copy of the file
                                value = SENDFILE;
                            }
                            // Compare the permissions to see if destination needs to be updated.
                            else if (dest_st.st_mode != p->req.mode) {
                                // Set permissions on file.
                                if (chmod(dest_path, p->req.mode) != 0) {
                                    printf("%s\n", dest_path);
                                    perror("Error setting permissions on file");
                                }
                            }
                        }
                    }

                    // Send response to client.
                    printf("SENDING: %d to client\n", value);
                    value = htonl(value);
                    write(p->fd, &value, sizeof(int32_t));
                }
            }

            p->state = AWAITING_TYPE;
            
            break;
        
        case AWAITING_PERM:
            bytes_read = read(p->fd, &mode, sizeof(mode_t));
            if(bytes_read < 0) {
                fprintf(stderr, "Error reading permissions.\n");
                perror("permissions");
                return -1;
            }
            else if(bytes_read > 0) {
                p->state = AWAITING_HASH;
                p->req.mode = ntohl(mode);
                printf("Received mode %d bytes: %d\n", bytes_read, p->req.mode);
            }
        break;
        case AWAITING_HASH:
            bytes_read = read(p->fd, &buffer, sizeof(char) * BLOCKSIZE);
            if(bytes_read < 0) {
                fprintf(stderr, "Error reading hash.\n");
                perror("hash");
                return -1;
            }
            else if(bytes_read > 0) {
                p->state = AWAITING_SIZE;
				buffer[bytes_read] = '\0';
				strcpy(p->req.hash, buffer);
                printf("Received hash %d bytes\n", bytes_read);
            }
        break;
        case AWAITING_DATA:
        break;
    }

    /*
    char buf[256];
    char outbuf[512];

    int len = read(p->fd, buf, sizeof(buf) - 1);
    if (len > 0) {

        buf[len] = '\0';
        printf("Received %d bytes: %s", len, buf);
        sprintf(outbuf, "%s says: %s", inet_ntoa(p->ipaddr), buf);
        //broadcast(top, outbuf, strlen(outbuf));

        return 0;

    } else if (len == 0) {
        // socket is closed
        printf("Disconnect from %s\n", inet_ntoa(p->ipaddr));
        sprintf(outbuf, "Goodbye %s\r\n", inet_ntoa(p->ipaddr));
        //broadcast(top, outbuf, strlen(outbuf));
        return -1;
    } else { // shouldn't happen
        perror("read");
        return -1;
    }
    */

    return 0;
}

 /* bind and listen, abort on error
  * returns FD of listening socket
  */
int bindandlisten(void) {
    struct sockaddr_in r;
    int listenfd;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    int yes = 1;
    if ((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) == -1) {
        perror("setsockopt");
    }
    memset(&r, '\0', sizeof(r));
    r.sin_family = AF_INET;
    r.sin_addr.s_addr = INADDR_ANY;
    r.sin_port = htons(PORT);

    if (bind(listenfd, (struct sockaddr *)&r, sizeof r)) {
        perror("bind");
        exit(1);
    }

    if (listen(listenfd, 5)) {
        perror("listen");
        exit(1);
    }
    return listenfd;
}

static struct client *addclient(struct client *top, int fd, struct in_addr addr) {
    struct client *p = malloc(sizeof(struct client));
    if (!p) {
        perror("malloc");
        exit(1);
    }

    printf("Adding client %s\n", inet_ntoa(addr));

    p->fd = fd;
    p->state = AWAITING_TYPE;
    p->ipaddr = addr;
    p->next = top;
    top = p;
    return top;
}

static struct client *removeclient(struct client *top, int fd) {
    struct client **p;

    for (p = &top; *p && (*p)->fd != fd; p = &(*p)->next)
        ;
    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p) {
        struct client *t = (*p)->next;
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
        free(*p);
        *p = t;
    } else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n",
                 fd);
    }
    return top;
}