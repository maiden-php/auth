#ifndef _HASH_H_
#define _HASH_H_

// Hash manipulation helper functions
char *hash(FILE *f);

// Compare two hashes. Returns 0 if they match.
short compare_hashes(const char *h1, const char *h2);

// Tries to compute the hash for a file with the file name passed into the function.
// Returns hash if successful or NULL otherwise.
char *hash_file_name(const char *file_name);

#endif // _HASH_H_
