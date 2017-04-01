#ifndef _HASH_H_
#define _HASH_H_

#define BLOCKSIZE 8

// Hash manipulation helper functions
char *hash(char *hash_val, FILE *f);

int check_hash(const char *hash1, const char *hash2);

// Tries to compute the hash for a file with the file name passed into the function.
// Returns hash if successful or NULL otherwise.
char *hash_file_name(char *hash_val, const char *file_name);

#endif // _HASH_H_
