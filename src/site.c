#include <toml/toml.h>
#include <tinx.h>
#include <stdlib.h>
#include <limits.h>

static void string_vector_init(utils_vector_t *vector) {
	utils_vector_init(vector, sizeof(char*));
}

static void string_vector_push_back(utils_vector_t *vector, char *str) {
	utils_vector_push_back(vector, (char*[]){str});
}

static void string_vector_destroy(utils_vector_t *vector) {
	utils_vector_typed_foreach (char *, str, vector) {
		free(*str);
	}
	utils_vector_destroy(vector);
}

static void source_constructor(void *ptr) {
	source_t *source = ptr;
	string_vector_init(&source->build_dependencies_names);
	utils_vector_init(&source->build_dependencies, sizeof(package_t*));
	string_vector_init(&source->patches);
	string_vector_init(&source->prepare);
}

static void package_constructor(void *ptr) {
	package_t *package = ptr;
	package->type = PACKAGE_HOST;
	string_vector_init(&package->build_dependencies_names);
	string_vector_init(&package->dependencies_names);
	utils_vector_init(&package->build_dependencies, sizeof(package_t*));
	utils_vector_init(&package->dependencies, sizeof(package_t*));
	string_vector_init(&package->configure);
	string_vector_init(&package->build);
	string_vector_init(&package->install);
}

static void tool_constructor(void *ptr) {
	package_t *package = ptr;
	package->type = PACKAGE_TOOL;
	string_vector_init(&package->build_dependencies_names);
	string_vector_init(&package->dependencies_names);
	utils_vector_init(&package->build_dependencies, sizeof(package_t*));
	utils_vector_init(&package->dependencies, sizeof(package_t*));
	string_vector_init(&package->configure);
	string_vector_init(&package->build);
	string_vector_init(&package->install);
}

static void build_system_constructor(void *ptr) {
	build_system_t *build_system = ptr;
	string_vector_init(&build_system->build_dependencies_names);
	utils_vector_init(&build_system->build_dependencies, sizeof(package_t));
	string_vector_init(&build_system->configure);
	string_vector_init(&build_system->build);
	string_vector_init(&build_system->install);
}

static parser_table_t author_table = PARSER_DEFINE_TABLE(
	NULL,
	sizeof(author_t),
	PARSER_FIELD_STRING("name",  offsetof(author_t, name)),
	PARSER_FIELD_STRING("email", offsetof(author_t, email)),
);

static parser_table_t source_table = PARSER_DEFINE_TABLE(
	source_constructor,
	sizeof(source_t),
	PARSER_FIELD_STRING("name",          offsetof(source_t, name)),
	PARSER_FIELD_TABLE("author",         offsetof(site_t, author), &author_table),
	PARSER_FIELD_STRING("version",       offsetof(source_t, version)),
	PARSER_FIELD_STRING("revision",      offsetof(source_t, revision)),
	PARSER_FIELD_STRING_ARRAY("prepare", offsetof(source_t, prepare)),
	PARSER_FIELD_STRING_ARRAY("patches", offsetof(source_t, patches)),
	PARSER_FIELD_STRING_ARRAY("build_dependencies", offsetof(source_t, build_dependencies_names)),
);

static parser_table_t package_table = PARSER_DEFINE_TABLE(
	package_constructor,
	sizeof(package_t),
	PARSER_FIELD_STRING("name",            offsetof(package_t, name)),
	PARSER_FIELD_STRING("source",          offsetof(package_t, source_name)),
	PARSER_FIELD_TABLE("author",           offsetof(site_t, author), &author_table),
	PARSER_FIELD_STRING("version",         offsetof(package_t, version)),
	PARSER_FIELD_STRING("revision",        offsetof(package_t, revision)),
	PARSER_FIELD_STRING("description",     offsetof(package_t, description)),
	PARSER_FIELD_STRING("website",         offsetof(package_t, website)),
	PARSER_FIELD_STRING("build_system",    offsetof(package_t, build_system_name)),
	PARSER_FIELD_STRING_ARRAY("configure", offsetof(package_t, configure)),
	PARSER_FIELD_STRING_ARRAY("build",     offsetof(package_t, build)),
	PARSER_FIELD_STRING_ARRAY("install",   offsetof(package_t, install)),
	PARSER_FIELD_STRING_ARRAY("build_dependencies", offsetof(package_t, build_dependencies_names)),
	PARSER_FIELD_STRING_ARRAY("dependencies",       offsetof(package_t,	dependencies_names)),
);

static parser_table_t tool_table = PARSER_DEFINE_TABLE(
	tool_constructor,
	sizeof(package_t),
	PARSER_FIELD_STRING("name",            offsetof(package_t, name)),
	PARSER_FIELD_STRING("source",          offsetof(package_t, source_name)),
	PARSER_FIELD_TABLE("author",           offsetof(site_t, author), &author_table),
	PARSER_FIELD_STRING("version",         offsetof(package_t, version)),
	PARSER_FIELD_STRING("revision",        offsetof(package_t, revision)),
	PARSER_FIELD_STRING("description",     offsetof(package_t, description)),
	PARSER_FIELD_STRING("website",         offsetof(package_t, website)),
	PARSER_FIELD_STRING("build_system",    offsetof(package_t, build_system_name)),
	PARSER_FIELD_STRING_ARRAY("configure", offsetof(package_t, configure)),
	PARSER_FIELD_STRING_ARRAY("build",     offsetof(package_t, build)),
	PARSER_FIELD_STRING_ARRAY("install",   offsetof(package_t, install)),
	PARSER_FIELD_STRING_ARRAY("build_dependencies", offsetof(package_t, build_dependencies_names)),
	PARSER_FIELD_STRING_ARRAY("dependencies",       offsetof(package_t,	dependencies_names)),
);

static parser_table_t build_system_table = PARSER_DEFINE_TABLE(
	build_system_constructor,
	sizeof(build_system_t),
	PARSER_FIELD_STRING("name",                     offsetof(build_system_t, name)),
	PARSER_FIELD_STRING_ARRAY("configure",          offsetof(build_system_t, configure)),
	PARSER_FIELD_STRING_ARRAY("build",              offsetof(build_system_t, build)),
	PARSER_FIELD_STRING_ARRAY("install",            offsetof(build_system_t, install)),
	PARSER_FIELD_STRING_ARRAY("build_dependencies", offsetof(build_system_t, build_dependencies_names)),
);

static parser_table_t option_def_table = PARSER_DEFINE_TABLE(
	NULL,
	sizeof(option_def_t),
	PARSER_FIELD_STRING("name",    offsetof(option_def_t, name)),
	PARSER_FIELD_STRING("default", offsetof(option_def_t, default_value)),
);

static parser_table_t repository_table = PARSER_DEFINE_TABLE(
	NULL,
	sizeof(repository_t),
	PARSER_FIELD_STRING("name", offsetof(repository_t, name)),
	PARSER_FIELD_STRING("url",  offsetof(repository_t, url)),
);

static parser_table_t site_desc_table = PARSER_DEFINE_TABLE(
	NULL,
	sizeof(site_t),
	PARSER_FIELD_STRING("name",            offsetof(site_t, name)),
	PARSER_FIELD_STRING("description",     offsetof(site_t, description)),
	PARSER_FIELD_STRING("package_manager", offsetof(site_t, package_manager)),
	PARSER_FIELD_ARRAY("options",          offsetof(site_t, options),      PARSER_TABLE(&option_def_table)),
	PARSER_FIELD_ARRAY("repositories",     offsetof(site_t, repositories), PARSER_TABLE(&repository_table)),
	PARSER_FIELD_TABLE("author",           offsetof(site_t, author),       &author_table),
	PARSER_FIELD_STRING_ARRAY("imports",   offsetof(site_t, imports)),
);

static parser_table_t site_table = PARSER_DEFINE_TABLE(
	NULL,
	sizeof(site_t),
	PARSER_FIELD_TABLE("site",          0 /* the wole table itself*/,    &site_desc_table),
	PARSER_FIELD_ARRAY("sources",       offsetof(site_t, sources),       PARSER_TABLE(&source_table)),
	PARSER_FIELD_ARRAY("packages",      offsetof(site_t, packages),      PARSER_TABLE(&package_table)),
	PARSER_FIELD_ARRAY("tools",         offsetof(site_t, packages),      PARSER_TABLE(&tool_table)),
	PARSER_FIELD_ARRAY("build_systems", offsetof(site_t, build_systems), PARSER_TABLE(&build_system_table)),
);

static void site_load_file(site_t *site, const char *path) {
	// prevent double inclusion
	utils_vector_typed_foreach (char *, included, &site->included_files) {
		if (!strcmp(path, *included)) {
			error("cannot include twice '%s'", path);
			exit(1);
		}
	}
	string_vector_push_back(&site->included_files, xstrdup(path));

	toml_error_t toml_error;
	toml_table_t *table = toml_parse_file(path, &toml_error);
	if (!table) {
		char buf[256];
		toml_error_to_str(&toml_error, buf, sizeof(buf));
		error("%s", buf);
		toml_error_free(&toml_error);
		exit(1);
	}

	parser(&site_table, site, table);
	toml_table_free(table);

	utils_vector_t imports;
	string_vector_init(&imports);
	utils_vector_typed_foreach (char *, import, &site->imports) {
		// we steal the ref from site->imports
		utils_vector_push_back(&imports, import);
	}
	utils_vector_clear(&site->imports);
		

	utils_vector_typed_foreach (char *, import, &imports) {
		const char *import_path = *import;
		// relative paths are relative to the directory of the current file
		if (import_path[0] == '/') {
			site_load_file(site, import_path);
		} else {
			char *dir_end = strrchr(path, '/');
			if (dir_end) {
				char full_path[PATH_MAX];
				snprintf(full_path, sizeof(full_path), "%.*s/%s", (int)(dir_end - path), path, import_path);
				site_load_file(site, full_path);
			} else {
				site_load_file(site, import_path);
			}
		}
	}
	string_vector_destroy(&imports);
}

site_t *site_load(const char *path) {
	site_t *site = xzalloc(sizeof(site_t));

	utils_vector_init(&site->sources,       sizeof(source_t));
	utils_vector_init(&site->packages,      sizeof(package_t));
	utils_vector_init(&site->build_systems, sizeof(build_system_t));
	utils_vector_init(&site->options,       sizeof(option_def_t));
	utils_vector_init(&site->repositories,  sizeof(repository_t));
	string_vector_init(&site->imports);
	string_vector_init(&site->included_files);

	site_load_file(site, path);

	// TODO : second pass

	return site;
}

static void author_destroy(author_t *author) {
	free(author->name);
	free(author->email);
}

static void source_destroy(source_t *source) {
	string_vector_destroy(&source->build_dependencies_names);
	utils_vector_destroy(&source->build_dependencies);
	string_vector_destroy(&source->patches);
	string_vector_destroy(&source->prepare);
	author_destroy(&source->author);
	free(source->version);
	free(source->revision);
	switch (source->type) {
	case SOURCE_DIR:
		free(source->dir.path);
		break;
	case SOURCE_GIT:
		free(source->git.url);
		free(source->git.tag);
		free(source->git.commit);
		break;
	case SOURCE_TAR:
		free(source->tar.url);
		free(source->tar.hash);
		break;
	case SOURCE_FTP:
		free(source->ftp.url);
		free(source->ftp.hash);
		break;
	}
}

static void package_destroy(package_t *package) {
	string_vector_destroy(&package->build_dependencies_names);
	string_vector_destroy(&package->dependencies_names);
	utils_vector_destroy(&package->build_dependencies);
	utils_vector_destroy(&package->dependencies);
	string_vector_destroy(&package->configure);
	string_vector_destroy(&package->build);
	string_vector_destroy(&package->install);
	author_destroy(&package->author);
	free(package->name);
	free(package->version);
	free(package->revision);
	free(package->description);
	free(package->source_name);
	free(package->build_system_name);
}

static void build_system_destroy(build_system_t *build_system) {
	string_vector_destroy(&build_system->build_dependencies_names);
	utils_vector_destroy(&build_system->build_dependencies);
	string_vector_destroy(&build_system->configure);
	string_vector_destroy(&build_system->build);
	string_vector_destroy(&build_system->install);
	free(build_system->name);
}

static void option_def_destroy(option_def_t *option_def) {
	free(option_def->name);
	free(option_def->default_value);
}

static void repository_destroy(repository_t *repository) {
	free(repository->name);
	free(repository->url);
}

void site_free(site_t *site) {
	utils_vector_typed_foreach (source_t, source, &site->sources) {
		source_destroy(source);
	}
	utils_vector_typed_foreach (package_t, package, &site->packages) {
		package_destroy(package);
	}
	utils_vector_typed_foreach (build_system_t, build_system, &site->build_systems) {
		build_system_destroy(build_system);
	}
	utils_vector_typed_foreach (option_def_t, option_def, &site->options) {
		option_def_destroy(option_def);
	}
	utils_vector_typed_foreach (repository_t, repository, &site->repositories) {
		repository_destroy(repository);
	}
	author_destroy(&site->author);
	utils_vector_destroy(&site->sources);
	utils_vector_destroy(&site->packages);
	utils_vector_destroy(&site->build_systems);
	utils_vector_destroy(&site->options);
	utils_vector_destroy(&site->repositories);
	string_vector_destroy(&site->imports);
	string_vector_destroy(&site->included_files);
	free(site);
}

char *site_expand_string(site_t *site, const char *value) {
	utils_vector_t str;
	utils_vector_init(&str, sizeof(char));
	const char *src = value;
	while (*src) {
		if (*src == '$') {
			src++;
			switch (*src) {
			case '{':
				src++;
				const char *var_name = src;
				while (*src && *src != '}') {
					src++;
				}
				if (*src != '}') {
					error("invalid string '%s' : unterminated variable expansion");
				}
				break;
			case '$':
				utils_vector_push_back(&str, (char[]){'$'});
				break;
			default:
				error("invalid string '%s' : must have a '$' or '{' after a '$'", value);
				exit(1);
				break;
			}
		} else {
			utils_vector_push_back(&str, src);
		}
		src++;
	}
	utils_vector_push_back(&str, (char[]){'\0'});
	return str.data;
}
