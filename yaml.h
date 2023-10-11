#ifndef _YAML_H_
#define _YAML_H_

typedef char **arr_t;

enum yamlval
{
    YAMLVAL_YAML = 0,
    YAMLVAL_STR,
    YAMLVAL_NUM,
};

typedef struct s_yaml_node
{
    char *key;
    void *value;
    enum yamlval val;
} node_t;

typedef struct s_yaml
{
    node_t *node;
    arr_t array;
} yaml_t;

yaml_t *yaml_parse(const char *filename);
void yaml_close(yaml_t *yaml);

#endif /* YAML HEADER */