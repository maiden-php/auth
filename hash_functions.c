#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "hash.h"

const int block_size = 8; // expecting something like 74 1c 00 11 22 33 44 55 as output

// Hash manipulation helper functions
char *hash(FILE *f) {
    char *hash_val = malloc(sizeof(char) * block_size);
    for (int i = 0; i < block_size; i++) {
        hash_val[i] = '\0';
    }

    char ch;
    int hash_index = 0;
    while (fread(&ch, 1, 1, f) != 0) {
        hash_val[hash_index] ^= ch;
        hash_index = (hash_index + 1) % block_size;
    }

    return hash_val;
}

// Compare two hashes. Returns 0 if they match.
short compare_hashes(const char *h1, const char *h2) {
    return memcmp(h1, h2, sizeof(char) * block_size);
}

// Tries to compute the hash for a file with the file name passed into the function.
// Returns hash if successful or NULL otherwise.
char *hash_file_name(const char *file_name) {
    FILE *f;
    char *res;
    
    // Open file for hashing.
    if ((f = fopen(file_name, "rb")) == NULL) {
        perror("Error opening file");
        return NULL;
    }

    // Retrieve hash from file.
    res = hash(f);

    // Close file.
    fclose(f);

    return res;
}