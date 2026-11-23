#include <tinx.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static void oom(void) {
	error("out of memory");
	exit(1);
}

void *xmalloc(size_t size) {
	void *ptr = malloc(size);
	if (!ptr) oom();
	return ptr;
}

void *xzalloc(size_t size) {
	void *ptr = calloc(1, size);
	if (!ptr) oom();
	return ptr;
}

char *xstrdup(const char *str) {
	char *new = strdup(str);
	if (!new) oom();
	return new;
}

FILE *xfopen(const char *path, const char *mode) {
	FILE *file = fopen(path, mode);
	if (!file) {
		error("%s : %s", path, strerror(errno));
		exit(1);
	}
	return file;
}
