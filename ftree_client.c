#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>

#include "ftree.h"
#include "hash.h"


int create_connection(char *host, unsigned short port) {
	// Connect to server
	int sockfd;
	struct hostent *he;
	struct sockaddr_in their_addr; /* connector's address information  */

	if ((he = gethostbyname(host)) == NULL) {  /* get the host info  */
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

	if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(1);
	}

	return sockfd;
}

void send_req_structure(int sockfd, struct request *req) {

	int32_t type = htonl(req->type);
	int32_t mode = htonl(req->mode);
	int32_t size = htonl(req->size);
	write(sockfd, &type, sizeof(int32_t));
	write(sockfd, req->path, sizeof(char) * MAXPATH);
	write(sockfd, &mode, sizeof(mode_t));
	write(sockfd, req->hash, sizeof(char) * BLOCKSIZE);
	write(sockfd, &size, sizeof(int32_t));
}


int parse_directory(const char *base, const char *source, int sockfd, char *host, unsigned short port);


//home/rs/
//home/rs/t1
//t1
//
//home/rs/t1
//home/rs/t1/fcopy
//t1/fcopy

int parse(const char* path, const char *name, int sockfd, char *host, unsigned short port)
{
	// Build source path.
	char *src_path = name;
	//char *src_path = malloc(sizeof(char) * (strlen(path) + strlen(name) + 2));
	//if (src_path == NULL) {
	//	perror("Error allocating memory.");
	//	return 1;
	//}

	//if (path != NULL) {
	//	strcpy(src_path, path);
	//}
	//strcat(strcat(src_path, "/"), name);

	// Retrieve information from the file / directory / not symlinks since we are ignoring for it of for now.
	struct stat src_st;
	if (stat(src_path, &src_st) < 0) {
		fprintf(stderr, "%s\n", src_path);
		perror("Error reading information about source file");

		// Skip everything else except freeing memory for the source path.
		//goto free_memory;
	}

	// Build name of destination path where to copy the contents of source.
	// If src is /a/b/c and dest is /t then the result should be something like: dest/base(src) which is /t/c .
	char *namePath = strdup(name);
	char *base_src = strstr(dirname(namePath), path) + strlen(path);
	// Skip the end / if needed.
	if (strlen(base_src) > 0) {
		base_src++;
	}

	free(namePath);
	//char *base_src = basename((char*)src_path);
	//if (strlen(base_src) + strlen(name) + 2 > MAXPATH) {
	//	fprintf(stderr, "%s\nFile path is too big.", src_path);

	//	// Skip everything else except freeing memory for the source path.
	//	goto free_memory;
	//}

	// Update the struct to send to the server
	struct request req;
	req.mode = src_st.st_mode;

	// Update the path
	memset(req.path, 0, MAXPATH * sizeof(char));
	strcpy(req.path, base_src);
	if (strlen(base_src) > 0) {
		strcat(req.path, "/");
	}
	strcat(req.path, basename(name));


	char hash[BLOCKSIZE];
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
	send_req_structure(sockfd, &req);

	int32_t server_response;
	int bytes_read = read(sockfd, &server_response, sizeof(int32_t));
	if (bytes_read == -1) {
		perror("Error reading from server");
	}
	else {
		server_response = ntohl(server_response);
		if (server_response == OK) {
			printf("RECEIVED: OK\n");

			// If it's a directory we need to go into that directory.
			if (req.type == REGDIR) {
				parse_directory(path, src_path, sockfd, host, port);
			}
		}
		else if (server_response == SENDFILE) {
			printf("RECEIVED: SEND FILE\n");

			// Create a child process to take care of the file transfer.
			int pid = fork();

			// Check for error creating child process.
			if (pid == -1) {
				perror("Error creating child process");
			}
			// Check if this is the child process.
			else if (pid == 0) {

				printf("---> %s\n", src_path);
				int filefd = open(src_path, O_RDONLY, S_IRUSR | S_IWUSR);
				if(filefd == -1) {
					perror("Error opening file");
					exit(EXIT_FAILURE);
				}

				int child_sockfd = create_connection(host, port);


				// Start by sending transfile to server
				int32_t value = htonl(TRANSFILE);
				write(child_sockfd, &value, sizeof(int32_t));

				printf("SENDING STRUCTURE");

				// Send structure again
				send_req_structure(child_sockfd, &req);

				printf("START SENDING FILE");

				// Start sending the file
				char buffer[BUFSIZ];
				int file_bytes_read;
				while (1) {
					//file_bytes_read = fread(buffer, 1, BUFSIZ, filefd);
					file_bytes_read = read(filefd, buffer, BUFSIZ);
					printf("read: %d bytes \n", file_bytes_read);
					// Check if we finished copying
					if (file_bytes_read == 0) {
						break;
					}
					else if (file_bytes_read == -1) {
						perror("Error reading file for transfer");
						exit(EXIT_FAILURE);
					}

					if (write(child_sockfd, buffer, file_bytes_read) == -1) {
						perror("Error transferring file");
						exit(EXIT_FAILURE);
					}
				}

				int32_t child_server_response;
				// Wait for OK from server
				if (read(child_sockfd, &child_server_response, sizeof(int32_t)) == -1) {
					perror("Error reading response from server");
					exit(EXIT_FAILURE);
				}
				else {
					//TODO
				}

				close(filefd);
				close(child_sockfd);

				printf("SUCCESS!!!\n");
				exit(EXIT_SUCCESS);
			}
			else {
				// If not child then it's the parent process.

				// Wait on child and checks for error codes.
				int status = 0;
				if (wait(&status) == 1) {
					perror("Error waiting for child");
					return 1;
				}
			}
		}
		else {
			printf("RECEIVED: %d\n", server_response);
		}
	}
	
	return 0;
}


int parse_directory(const char *base, const char *source, int sockfd, char *host, unsigned short port) {

    // Open the source directory.
    DIR *dir = opendir(source);

    // Check for error.
    if (dir == NULL) {
        fprintf(stderr, "%s\n", source);
        perror("Error opening directory");
        return 1;
    }

	int res = 0;
    
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
		if (strlen(src_path) > 0) {
			strcat(src_path, "/");
		}
		strcat(src_path, sd->d_name);

		res = parse(base, src_path, sockfd, host, port);

		free(src_path);
    }

    if (closedir(dir) != 0) {
        perror("Error closing directory");
		return 1;
    }

	return res;
}

int rcopy_client(char *source, char *host, unsigned short port) {
	// Connect to server.
	int sockfd = create_connection(host, port);

	// Copy our directory structure.
	char *path = strdup(source);
	int res = parse(dirname(path), source, sockfd, host, port);
	free(path);

	// Close the connection.
	close(sockfd);

	return res;
}
