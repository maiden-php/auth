#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/stat.h>

#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "ftree.h"
#include "hash.h"

/*
enum STATE {

};
*/
void parse_directory(const char *source, int sockfd);

int rcopy_client(char *source, char *host, unsigned short port) {
    // Connect to server
    
    int sockfd, numbytes;  
    //char buf[MAXDATA];
    struct hostent *he;
    struct sockaddr_in their_addr; /* connector's address information  */

    if ((he=gethostbyname(host)) == NULL) {  /* get the host info  */
        perror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    their_addr.sin_family = AF_INET;    /* host byte order */
    their_addr.sin_port = htons(port);  /* short, network byte order */
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(their_addr.sin_zero), 8);   /* zero the rest of the struct */

    if (connect(sockfd, (struct sockaddr *)&their_addr, 
		sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }

    //write(sockfd, "HELLO!!!!!", 10);

    //while(1) {
        parse_directory(source, sockfd);
    //}


    close(sockfd);
}


void parse_directory(const char *source, int sockfd) {
    char hash[BLOCKSIZE];

    // Open the source directory.
    DIR *dir = opendir(source);

    // Check for error.
    if (dir == NULL) {
        fprintf(stderr, "%s\n", source);
        perror("Error opening directory");
        return;
    }

    // Iterate over each file/directory/link in our current source directory.
    struct dirent *sd;
    while ((sd = readdir(dir)) != NULL) {
        // Skip . and ..
        if (!strcmp(sd->d_name, ".") || !strcmp(sd->d_name, "..")) {
            continue;
        }

        // Build source path.
        char *src_path = malloc(sizeof(char) * (strlen(source) + strlen(sd->d_name) + 2));
        if (src_path == NULL) {
            perror("Error allocating memory.");
            continue;
        }
        strcpy(src_path, source);
        strcat(strcat(src_path, "/"), sd->d_name);

        // Retrieve information from the file / directory / not symlinks since we are ignoring for it of for now.
        struct stat src_st;
        if (stat(src_path, &src_st) < 0) {
            fprintf(stderr, "%s\n", src_path);
            perror("Error reading information about source file");

            // Skip everything else except freeing memory for the source path.
            goto free_memory;
        }

		// Build name of destination path where to copy the contents of source.
		// If src is /a/b/c and dest is /t then the result should be something like: dest/base(src) which is /t/c .
		char *base_src = basename((char*)source);
		if (strlen(base_src) + strlen(sd->d_name) + 2 > MAXPATH) {
			fprintf(stderr, "%s\nFile path is too big.", src_path);
			
			// Skip everything else except freeing memory for the source path.
			goto free_memory;
		}

		// Update the struct to send to the server
        struct request req;
        req.mode = src_st.st_mode;

		// Update the path
		memset(req.path, 0, MAXPATH * sizeof(char));
		strcpy(req.path, base_src);
		strcat(strcat(req.path, "/"), sd->d_name);


        // Check if current entry is a regular file.
        if (S_ISREG(src_st.st_mode)) {
            // Retrieve file hashes for comparison.
            hash_file_name(hash, src_path);

			// Update the type, size and hash.
			req.type = REGFILE;
            strcpy(req.hash, hash);
            req.size = src_st.st_size;
        }
        // Check if it's a directory.
        else if (S_ISDIR(src_st.st_mode)) {
			// Update the type, size and hash.
			req.type = REGDIR;
            req.size = 0;
            memset(req.hash, 0, sizeof(char) * BLOCKSIZE);
        }

        printf("SENDING: %s - ", req.path);

        // Send structure.
        int32_t type = htonl(req.type);
        int32_t mode = htonl(req.mode);
        int32_t size = htonl(req.size);
        write(sockfd, &type, sizeof(int32_t));
        write(sockfd, req.path, sizeof(char) * MAXPATH);
        write(sockfd, &mode, sizeof(mode_t));
        write(sockfd, req.hash, sizeof(char) * BLOCKSIZE);
        write(sockfd, &size, sizeof(int32_t));


         int32_t server_response;
         int bytes_read = read(sockfd, &server_response, sizeof(int32_t));
         if(bytes_read == -1) {
             perror("Error reading from server");
         }
         else {
             server_response = ntohl(server_response);
             if(server_response == OK) {
                 printf("RECEIVED: OK\n");
				 
				 // If it's a directory we need to go into that directory.
				 if (req.type == REGDIR) {
					 parse_directory(src_path, sockfd);
				 }
             }
             else if(server_response == SENDFILE) {
                 //TODO: CREATE FORK AND COPY FILE
                 printf("RECEIVED: SEND FILE\n");
             }
             else {
                 printf("RECEIVED: %d\n", server_response);
             }
         }

free_memory:
		 free(src_path);

        // // printf("Reading from server\n");
        // if((numbytes = recv(sockfd, buf, MAXDATA-1, 0)) == -1) {
        //     perror("recv()");
        // }
        // else {
        //     printf("Client-The recv() is OK...\n");

        //     if(numbytes > 0) {
        //         buf[numbytes] = '\0';

        //         printf("Client-Received: %s", buf);
        //         //strcmp(buf, )
        //     }
        // }

        //sleep(10);

    }

    if (closedir(dir) != 0) {
        perror("Error closing directory");
    }
}


/* 
 * Recursively copy the source directory contents into the destination directory creating child processes for each sub directory.
 * Returns the number of processes used in the copy.
 */
 /*
int copy_directory(const char *src) {

    // Open the source directory.
    DIR *dir = opendir(src);

    // Check for error.
    if (dir == NULL) {
        printf("%s\n", src);
        perror("Error opening directory");
        return -1;
    }

    // Iterate over each file/directory/link in our current source directory.
    struct dirent *sd;
    while ((sd = readdir(dir)) != NULL) {
        // Skip . and ..
        if (!strcmp(sd->d_name, ".") || !strcmp(sd->d_name, "..")) {
            continue;
        }

        // Build source path.
        char *src_path = malloc(sizeof(char) * (strlen(src) + strlen(sd->d_name) + 2));
        if (src_path == NULL) {
            perror("Error allocating memory.");
            *error = -1;
            continue;
        }
        strcpy(src_path, src);
        strcat(strcat(src_path, "/"), sd->d_name);

        // Retrieve information from the file / directory / not symlinks since we are ignoring for it of for now.
        struct stat src_st;
        if (stat(src_path, &src_st) < 0) {
            printf("%s\n", src_path);
            perror("Error reading information about source file");
            *error = -1;

            // Skip everything else except freeing memory for the source path.
            goto free_memory;
        }

        // Check if current entry is a regular file.
        if (S_ISREG(src_st.st_mode))
        {
                // Retrieve file hashes for comparison.
                char *src_hash = hash_file_name(src_path);

                // Source hash has to exist but destination hash might not, that's why we only check for
                // errors on the source hash.
                if (src_hash == NULL) {
                    printf("%s\n", src_path);
                    perror("Error reading hash from source");
                    *error = -1;
                }

                // Release memory allocated for hashes.
                free(src_hash);
            }
        }
        // Check if it's a directory.
        else if (S_ISDIR(src_st.st_mode)) {
            // It is a directory so we create a child process to recursively call copy_directory and perform
            // the copy of the sub directories.
            int pid = fork();
            
            // Check for error creating child process.
            if (pid == -1) {
                perror("Error creating child process");
                *error = 1;
            }
            // Check if this is the child process.
            else if (pid == 0) {
                // Create a directory with same permissions as source directory.
                if (create_directory(dest_path, src_st.st_mode) != 0) {
                    *error = -1;
                    exit(num_processes);
                }
                
                // Perform the recursive copy of the sub directory.
                exit(copy_directory(src_path, dest_path));
            }
            else {
                // If not child then it's the parent process.

                // Wait on child and checks for error codes.
                int status = 0;
                if (wait(&status) == -1) {
                   perror("Error waiting for child");
                    *error = -1;
                }
                
                // Update the number of child processes.
                WEXITSTATUS(status);
            }
        }

        free_memory:
        // Release string allocated for source path.
        free(src_path);
    }

    if (closedir(dir) != 0) {
        perror("Error closing directory");
        *error = -1;
    }

    return error >=0 ? num_processes : -num_processes;
}
*/