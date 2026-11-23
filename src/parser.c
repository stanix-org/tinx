#include <toml/toml.h>
#include <tinx.h>

static parser_field_t *parser_table_get_field(parser_table_t *table, const char *name) {
	for (size_t i = 0; table->fields[i].type.type; i++) {
		if (!strcmp(table->fields[i].name, name)) {
			return &table->fields[i];
		}
	}
	return NULL;
}

static int parser_type_to_toml_type(parser_type_t *type) {
	switch (type->type) {
	case PARSER_TYPE_STRING:
		return TOML_TYPE_STRING;
	case PARSER_TYPE_ARRAY:
		return TOML_TYPE_ARRAY;
	case PARSER_TYPE_TABLE:
		return TOML_TYPE_TABLE;
	default:
		__builtin_unreachable();
	}
}

static const char *parser_type_to_str(parser_type_t *type) {
	switch (type->type) {
	case PARSER_TYPE_STRING:
		return "a string";
	case PARSER_TYPE_ARRAY:
		return "an array";
	case PARSER_TYPE_TABLE:
		return "a table";
	default:
		__builtin_unreachable();
	}
}

static size_t parser_type_size(parser_type_t *type) {
	switch (type->type) {
	case PARSER_TYPE_STRING:
		return sizeof(char*);
	case PARSER_TYPE_ARRAY:
		return sizeof(utils_vector_t);
	case PARSER_TYPE_TABLE:
		return type->table->size;
	default:
		__builtin_unreachable();
	}
}

static void *parser_type_instantiate(parser_type_t *type) {
	void *ptr = xzalloc(parser_type_size(type));
	if (type->type == PARSER_TYPE_TABLE &&
			type->table->constructor) {
		type->table->constructor(ptr);
	}
	return ptr;
}

static void parser_value(parser_type_t *type, void *ptr, const char *name, toml_value_t *value);

static void parser_array(parser_type_t *content_type, utils_vector_t *vector, const char *name, toml_array_t *array) {
	for (size_t i = 0; i < array->count; i++) {
		void *element = parser_type_instantiate(content_type);

		toml_value_t *value = array->values[i];
		char cur_name[256];
		snprintf(cur_name, sizeof(cur_name), "%s[%zu]", name, i);

		parser_value(content_type, element, cur_name, value);

		utils_vector_push_back(vector, element);
		free(element);
	}
}

static void parser_table(parser_table_t *rules, void *ptr, const char *name, toml_table_t *table) {
	for (size_t i = 0; i < table->count; i++) {;
		char cur_name[256];
		if (name) {
			snprintf(cur_name, sizeof(cur_name), "%s.%s", name, table->values[i].key);
		} else {
			snprintf(cur_name, sizeof(cur_name), "%s", table->values[i].key);
		}

		parser_field_t *field = parser_table_get_field(rules, table->values[i].key);
		if (!field) {
			// TODO : don't throw a warning??
			warning("unknown key '%s'", cur_name);
			continue;
		}

		toml_value_t *value = table->values[i].value;
		parser_value(&field->type, (char*)ptr + field->offset, cur_name, value);
	}
}

static void parser_value(parser_type_t *type, void *ptr, const char *name, toml_value_t *value) {
	if (value->type != parser_type_to_toml_type(type)) {
		error("invalid type for %s, not %s", name, parser_type_to_str(type));
		exit(1);
	}

	switch (type->type) {
	case PARSER_TYPE_STRING:;
		if (*(char**)ptr) {
			error("%s already set previously", name);
			exit(1);
		}
		char *str = xstrdup(value->string.value);
		*(char**)ptr = str;
		break;
	case PARSER_TYPE_ARRAY:
		parser_array(type->content_type, ptr, name, &value->array);
		break;
	case PARSER_TYPE_TABLE:
		parser_table(type->table, ptr, name, &value->table);
		break;
	}
}

void parser(parser_table_t *rules, void *ptr, toml_table_t *table) {
	parser_table(rules, ptr, NULL, table);
}
