#include "util.h"
#include <cstring>
#include <ctype.h>
#include <cstdlib>

int strxcmp(const char* where, const char* what)
{
	return strncmp(where, what, strlen(what));
}

void strxcpy(char* dest, const char* src, size_t c)
{
	strncpy(dest, src, c);
	dest[c-1] = 0;
}

double parseDouble(const char* src)
{
	char ret[30];
	int p = 0;

	while (*src)
	{
		if (!isspace(*src))
			ret[p++] = *src;
		src++;
	}
	ret[p] = 0;

	return strtod(ret, 0);
}

int parseInt(const char* str)
{
	while (isspace(*str))
		str++;
	return atoi(str);
}

template<typename Func> void trimAny(char* src, Func toTrim)
{
	int pos = strlen(src) - 1;
	while (pos >= 0)
	{
		if (!toTrim(src[pos]))
			break;
		pos--;
	}
	src[pos+1] = 0;
}

char* trim(char* str)
{
	trimAny(str, [](char c) { return isspace(c); });
	return str;
}

char* rmeol(char* str)
{
	trimAny(str, [](char c) { return c == '\r' || c == '\n'; });
	return str;
}

char* padleft(char* str, size_t len, char c)
{
	int rem = len - strlen(str);

	if (rem <= 0)
		return str;
	memmove(str+rem, str, strlen(str)+1);
	memset(str, c, rem);

	return str;
}

char* padlefti(char* str, size_t len, char c)
{
	char s = 0;
	int rem;
	char* src = str;

	if (*str == '-' || *str == '+')
	{
		s = *str;
		src++;
	}

	rem = len - strlen(src);

	memmove(str+rem, src, strlen(src)+1);
	memset(str, c, rem);

	if (s)
		*str = s;

	return str;
}

char* padright(char* str, size_t len, char c)
{
	size_t clen = strlen(str);
	while (clen < len)
	{
		str[clen] = c;
		clen++;
	}
	str[clen] = 0;
	return str;
}

char* rmdash(char* str)
{
	char* p = strchr(str, '-');
	if (p)
	{
		// 10 digits per acc no + / + 4 digits bank id
		char* dest = p + (15-strlen(p+1));
		memmove(dest, p+1, strlen(p+1)+1);

		if (dest != p)
			memset(p, '0', dest-p);
	}
	return p;
}

