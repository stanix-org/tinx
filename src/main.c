#include <tinx.h>

int main() {
	config_t *config = config_load("config.toml");
	printf("author is %s\n", config->site->author.name);
	config_free(config);
}
