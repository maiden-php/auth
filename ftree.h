#ifndef _FTREE_H_
#define _FTREE_H_

/* Function for copying a file tree rooted at src to dest
 * Returns < 0 on error. The magnitude of the return value
 * is the number of processes involved in the copy and is
 * at least 1.
 */
int copy_ftree(const char *src, const char *dest);

/* Recursively copy the source directory contents into the
 * destination directory creating child processes for each
 * sub directory.
 * Returns the number of processes used in the copy.
 */
int copy_directory(const char *src, const char *dest);

/* Copies a file from source to destination.
*/
int copy_file(const char* src, const char* dest, mode_t permissions);

#endif // _FTREE_H_
