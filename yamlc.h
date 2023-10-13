#ifndef _YAML_H_
#define _YAML_H_

typedef void ** arr_t;

typedef enum yamlval
{
    YAMLVAL_ERR = -1,
    YAMLVAL_YAML = 0,
    YAMLVAL_STR,
    YAMLVAL_NUM,
} yamval_t;

typedef struct s_yaml_node
{
    enum yamlval val;
    void *data;
    struct s_yaml_node *parent;
    char *name;
} node_t;

typedef struct s_yaml
{
    node_t *nodes;
    arr_t lines;
} yaml_t;


/**
 * Generates a yaml object by filename.
 * @returns A yaml object, null if it failed.
 *
 * @public
 */
yaml_t *yaml_parse(const char *filename);

/**
 * Close a yaml object.
 *
 * @public
 */
void yaml_close(yaml_t *yaml);

/**
 * Determines if a yaml path exists.
 * @returns The yamlval type, or `YAMLVAL_ERR` if it failed or not exists.
 *
 * @public
 */
enum yamlval yaml_accesstype(yaml_t *y, const char *__path);

/**
 * Access the value of a yaml element.
 * @returns The pointer towards the value, null if it failed.
 * @warning If you try to access a table, you will get a list of yaml elements.
 * This list is a list of pointers that ends with null.
 *
 * @public
 */
void *yaml_access(yaml_t *y, const char *__path);

#endif /* YAML HEADER */