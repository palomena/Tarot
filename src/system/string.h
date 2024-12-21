#ifndef TAROT_STRING_H
#define TAROT_STRING_H

/* Only available to tarot source files */
#ifdef TAROT_SOURCE

#include <stddef.h>

extern long int strtol(const char *str, char **endptr, unsigned int base);
extern double strtod(const char *str, char**);
extern void* memcpy(void *destination, const void *source, size_t num_bytes);
extern void* memmove(void *destination, const void *source, size_t num_bytes);
extern void* memset(void *s, int ch, size_t num_bytes);
extern int memcmp(const void *a, const void *b, size_t n);
extern size_t strlen(const char *str);
extern char* strcpy(char *str1, const char *str2);
extern char* strncpy(char *str1, const char *str2, size_t n);
extern char* strrev(char *str);
extern int strcmp(const char *a, const char *b);
extern int strncmp(const char *a, const char *b, size_t n);
extern int strcasecmp(const char *a, const char *b);
extern char* strdup(const char *str);
extern char* strcat(char *str1, const char *str2);
extern char* strncat(char *str1, const char *str2, long unsigned int n);
extern char* strchr(const char *str, int ch);
extern char* strrchr(const char *str, int ch);
extern char* strstr(const char *str, const char *text);
extern char* strrstr(char *str, const char *text);
extern char* strtok(char *s, const char *delim);

#endif /* TAROT_SOURCE */

#endif /* TAROT_STRING_H */
