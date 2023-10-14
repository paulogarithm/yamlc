#ifndef _YAML_H_
#define _YAML_H_

typedef void **arr_t;

typedef enum yamlval
{
    YAMLVAL_ERR = -1,
    YAMLVAL_YAML = 0,
    YAMLVAL_STR,
    YAMLVAL_NUM,
} yamlval_t;

typedef struct s_yaml
{
    yamlval_t val;
    void *data;
    struct s_yaml *parent;
    char *name;
} yaml_t;

typedef struct s_preyaml
{
    yaml_t *main;
    arr_t lines;
} preyaml_t;

#ifdef __cplusplus

#include <string>

namespace yaml
{
    extern "C"
    {
        yaml_t *yaml_load(const char *filename);
        void yaml_close(yaml_t *yaml);
        yamlval_t yaml_accesstype(yaml_t *y, const char *path);
        void *yaml_access(yaml_t *y, const char *path);
    }
    /**
     * Generates a yaml object by filename.
     * @returns A yaml object, null if it failed.
     */
    yaml_t *load(const std::string &filename)
    {
        return yaml_load(filename.c_str());
    }

    /**
     * Close a yaml object.
     */
    void close(yaml_t *yaml)
    {
        yaml_close(yaml);
    }

    /**
     * Access the type of a yaml variable, separated by spaces.
     * @returns The yaml value type, or -1 if it failed or not exists.
     * @note An example of string input can be `person.age`.
     */
    yamlval_t accesstype(yaml_t *y, const std::string &path)
    {
        return yaml_accesstype(y, path.c_str());
    }

    /**
     * Access the value of a yaml element.
     * @returns The pointer to the value, or null if it failed or not exists.
     * @warning If you try to access a table, you will get another yaml object.
     */
    void *access(yaml_t *y, const std::string &path)
    {
        return yaml_access(y, path.c_str());
    }
}

#else

/**
 * Generates a yaml object by filename.
 * @returns A yaml object, null if it failed.
 */
yaml_t *yaml_load(const char *filename);

/**
 * Close a yaml object.
 */
void yaml_close(yaml_t *yaml);

/**
 * Access the type of a yaml variable, separated by spaces.
 * @returns The yaml value type, or -1 if it failed or not exists.
 * @note An example of string input can be `person.age`.
 */
yamlval_t yaml_accesstype(yaml_t *y, const char *__path);

/**
 * Access the value of a yaml element.
 * @returns The pointer to the value, or null if it failed or not exists.
 * @warning If you try to access a table, you will get another yaml object.
 */
void *yaml_access(yaml_t *y, const char *__path);

#endif /* C++ */

#endif /* YAML HEADER */
