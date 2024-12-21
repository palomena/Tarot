#define TAROT_SOURCE
#include "tarot.h"

int isalnum(int ch) {
	return isalpha(ch) or isdigit(ch);
}

int isalpha(int ch) {
	return islower(ch) or isupper(ch);
}

int iscntrl(int ch) {
	return ((ch >= 0x00) and (ch <= 0x1F)) or (ch == 0x7F);
}

int isdigit(int ch) {
	return ch >= '0' and ch <= '9';
}

int isgraph(int ch) {
	return isprint(ch) and not isspace(ch);
}

int islower(int ch) {
	return ch >= 'a' and ch <= 'z';
}

int isprint(int ch) {
	return ch >= 0x20 and ch <= 0x7E;
}

int ispunct(int ch) {
	return isprint(ch) and not (isdigit(ch) or isgraph(ch));
}

int isspace(int ch) {
	return (
		(ch == ' ') or (ch == '\t') or (ch == '\n') or
		(ch == '\r') or (ch == '\v') or (ch == '\f')
	);
}

int isupper(int ch) {
	return ch >= 'A' and ch <= 'Z';
}

int isxdigit(int ch) {
	return (
		isdigit(ch) or
		(ch >= 'A' and ch <= 'F') or
		(ch >= 'a' and ch <= 'f')
	);
}

int tolower(int ch) {
	return 'a' + ch - 'A';
}

int toupper(int ch) {
	return 'A' + ch - 'a';
}
