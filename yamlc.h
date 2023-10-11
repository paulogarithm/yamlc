#ifndef _YAML_H_
#define _YAML_H_

typedef void ** arr_t;

enum yamlval
{
    YAMLVAL_ERR = -1,
    YAMLVAL_YAML = 0,
    YAMLVAL_STR,
    YAMLVAL_NUM,
};

typedef struct s_yaml_node
{
    char *key;
    void *value;
    enum yamlval val;
    struct s_yaml_node *parent;
} node_t;

typedef struct s_yaml
{
    node_t *nodes;
    arr_t lines;
} yaml_t;

yaml_t *yaml_parse(const char *filename);
void yaml_close(yaml_t *yaml);

#endif /* YAML HEADER */