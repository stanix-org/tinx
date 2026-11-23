#ifndef TINX_H
#define TINX_H

#include <libutils/vector.h>
#include <toml/toml.h>

typedef struct source source_t;
typedef struct package package_t;
typedef struct build_system build_system_t;

typedef struct author {
	char *name;
	char *email;
} author_t;

struct source {
	char *name;
	utils_vector_t build_dependencies_names;
	utils_vector_t build_dependencies;
	utils_vector_t patches;
	utils_vector_t prepare;
	author_t author;
	char *version;
	char *revision;
	int type;
	union {
		struct {
			char *path;
		} dir;
		struct {
			char *url;
			char *tag;
			char *commit;
		} git;
		struct {
			char *url;
			char *hash;
		} tar;
		struct {
			char *url;
			char *hash;
		} ftp;
	};
};

#define SOURCE_DIR 1
#define SOURCE_GIT 2
#define SOURCE_TAR 3
#define SOURCE_FTP 4

struct package {
	utils_vector_t build_dependencies_names;
	utils_vector_t dependencies_names;
	utils_vector_t build_dependencies;
	utils_vector_t dependencies;
	utils_vector_t configure;
	utils_vector_t build;
	utils_vector_t install;
	author_t author;
	char *name;
	char *version;
	char *revision;
	char *description;
	char *website;
	char *source_name;
	source_t *source;
	char *build_system_name;
	build_system_t *build_system;
	int type;
};

struct build_system {
	utils_vector_t build_dependencies_names;
	utils_vector_t build_dependencies;
	utils_vector_t configure;
	utils_vector_t build;
	utils_vector_t install;
	char *name;
};

#define PACKAGE_TOOL 1
#define PACKAGE_HOST 2

#define ACTION_FETCH_SOURCE   1
#define ACTION_PATCH_SOURCE   2
#define ACTION_PREPARE_SOURCE 3
#define ACTION_CLEAN_SOURCE   4

#define ACTION_CONFIGURE   5
#define ACTION_BUILD       6
#define ACTION_INSTALL     7
#define ACTION_PACKAGE     8
#define ACTION_REMOVE      9
#define ACTION_CLEAN_BUILD 10
#define ACTION_PULL        11

#define ACTION_RUN         12

typedef struct option_def {
	char *name;
	char *default_value;
} option_def_t;

typedef struct repository {
	char *name;
	char *url;
} repository_t;

typedef struct site {
	utils_vector_t sources;
	utils_vector_t packages;
	utils_vector_t build_systems;
	utils_vector_t options;
	utils_vector_t repositories;
	utils_vector_t included_files;
	utils_vector_t imports;
	author_t author;
	char *name;
	char *description;
	char *package_manager;
} site_t;

typedef struct option {
	char *name;
	char *value;
} option_t;

typedef struct config {
	utils_vector_t options;
	site_t *site;
	char *site_dir;
	char *repository;
} config_t;

typedef struct plan_action {
	char *name;
	int action;
	utils_vector_t command;
} plan_action_t;

typedef struct plan {
	utils_vector_t actions;
} plan_t;
	
typedef struct parser_field parser_field_t;
typedef struct parser_type  parser_type_t;
typedef struct parser_table parser_table_t;

struct parser_type {
	int type;
	union {
		parser_table_t *table;
		parser_type_t *content_type;
	};
};

#define PARSER_TYPE_NULL   0
#define PARSER_TYPE_STRING 1
#define PARSER_TYPE_ARRAY  2
#define PARSER_TYPE_TABLE  3

#define PARSER_STRING() { \
	.type = PARSER_TYPE_STRING, \
}

#define PARSER_ARRAY(...) { \
	.type = PARSER_TYPE_ARRAY, \
	.content_type = (parser_type_t[]){__VA_ARGS__}, \
}
#define PARSER_TABLE(_table) { \
	.type = PARSER_TYPE_TABLE, \
	.table = _table, \
}

struct parser_field {
	const char *name;
	size_t offset;
	parser_type_t type;
};

#define PARSER_FIELD(_name, _offset, ...) {.name = _name, .offset = _offset, .type = __VA_ARGS__}
#define PARSER_FIELD_STRING(name, offset) PARSER_FIELD(name, offset, PARSER_STRING())
#define PARSER_FIELD_ARRAY(name, offset, ...) PARSER_FIELD(name, offset, PARSER_ARRAY(__VA_ARGS__))
#define PARSER_FIELD_TABLE(name, offset, table) PARSER_FIELD(name, offset, PARSER_TABLE(table))

// an array of strings is a common thing
#define PARSER_FIELD_STRING_ARRAY(name, offset) PARSER_FIELD_ARRAY(name, offset, PARSER_STRING())

struct parser_table {
	void (*constructor)(void *ptr);
	size_t size;
	parser_field_t *fields;
};

#define PARSER_DEFINE_TABLE(constructor, size, ...) {constructor, size, .fields = (parser_field_t[]){__VA_ARGS__ {PARSER_TYPE_NULL}}}

void parser(parser_table_t *rules, void *ptr, toml_table_t *table);

site_t *site_load(const char *path);
void site_free(site_t *site);
char *site_expand_string(site_t *site, const char *value);

config_t *config_load(const char *path);
void config_write(config_t *config, const char *path);
void config_free(config_t *config);

plan_t *plan_generate(config_t *config, int action, utils_vector_t names);
int plan_execute(plan_t *plan);
void plan_free(plan_t *plan);


// utilities
void error(const char *fmt, ...);
void warning(const char *fmt, ...);
char *xstrdup(const char *str);
void *xmalloc(size_t size);
void *xzalloc(size_t size);
FILE *xfopen(const char *path, const char *mode);

#endif
