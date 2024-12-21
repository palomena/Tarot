#ifndef TAROT_CTYPE_H
#define TAROT_CTYPE_H

/* Only available to penta source files */
#ifdef TAROT_SOURCE

extern int isalnum(int ch);
extern int isalpha(int ch);
extern int iscntrl(int ch);
extern int isdigit(int ch);
extern int isgraph(int ch);
extern int islower(int ch);
extern int isprint(int ch);
extern int ispunct(int ch);
extern int isspace(int ch);
extern int isupper(int ch);
extern int isxdigit(int ch);
extern int tolower(int ch);
extern int toupper(int ch);

#endif /* TAROT_SOURCE */

#endif /* TAROT_CTYPE_H */
