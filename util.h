#ifndef UTIL_H
#define UTIL_H
#include <cstddef>

int strxcmp(const char* where, const char* what);
void strxcpy(char* dest, const char* src, size_t c);
double parseDouble(const char* src);
int parseInt(const char* src);
void trim(char* str);
char* padleft(char* str, size_t len, char c);
char* padlefti(char* str, size_t len, char c);
char* padright(char* str, size_t len, char c);
char* rmdash(char* str);

#endif

