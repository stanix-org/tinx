#include <tinx.h>
#include <stdarg.h>
#include <stdio.h>

void error(const char *fmt, ...) {
	char buf[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	fprintf(stderr, "tinx : %s\n", buf);
}

void warning(const char *fmt, ...) {
	char buf[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	fprintf(stderr, "tinx : warning : %s\n", buf);
}
