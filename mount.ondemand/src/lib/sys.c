#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>

int system_printf(char *fmt, ...)
{
	char p[256];
	va_list ap;
	int r;
	va_start(ap, fmt);
	vsnprintf(p, 256, fmt, ap);
	va_end(ap);
	r = system(p);
	return r;
}
