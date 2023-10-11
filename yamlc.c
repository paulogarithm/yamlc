#include <iso646.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <malloc.h>
#include <unistd.h>
#include <errno.h>

#include "yamlc.h"

#define true (_Bool)1
#define false (_Bool)0
#define null NULL

#define IS_SPACE(c) ((c == 32 or (c >= 9 and c <= 10)))
#define IS_DIGIT(c) (c >= '0' and c <= '9')

#define RETFREE(str, v) do { free(str); return v; } while (0)

void yaml_close(yaml_t *yaml);


// BASIC FUNCTIONS

static char *file_str(const char *filename)
{
    int fd = open(filename, O_RDONLY);
    struct stat st = {0};
    char *buf = null;

    if (fd < 0)
        return null;
    if (stat(filename, &st) < 0)
        return null;
    buf = (char *)malloc(sizeof(char) * (st.st_size + 1));
    if (buf == null)
        return null;
    if (read(fd, buf, st.st_size) < 0)
    {
        free(buf);
        return null;
    }
    buf[st.st_size] = '\0';
    return buf;
}

static char *strcleanedge(char *str)
{
    char *end = null;

    if (str == null)
        return null;
    while (IS_SPACE(*str))
        ++str;
    if (*str == '\0')
        return str;
    end = str + strlen(str) - 1;
    while (end > str and IS_SPACE(*end))
        --end;
    *(end + 1) = '\0';
    return str;
}


// ARRAY FUNCTIONS

static size_t __Array_len(void **ptr)
{
    size_t n = 0;

    while (ptr[n] != null)
        ++n;
    return n;
}

static arr_t __Array_init(void)
{
    arr_t array = (arr_t)malloc(sizeof(void *));

    if (array == null)
        return null;
    array[0] = null;
    return array;
}

static _Bool __Array_push(arr_t *arr, void *str)
{
    size_t size = __Array_len(*arr);
    arr_t new_arr = (arr_t)realloc(*arr, sizeof(void *) * (size + 2));

    if (new_arr == null)
        return false;
    new_arr[size] = str;
    new_arr[size + 1] = null;
    *arr = new_arr;
    return true;
}

static void __Array_close(arr_t arr)
{
    if (arr == null)
        return;
    for (size_t n = 0; arr[n] != null; ++n)
        free(arr[n]);
    free(arr);
}

static arr_t __Array_parser(const char *filename)
{
    char *str = file_str(filename);
    arr_t array = null;
    char *token = null;
    const char sep[] = "\n";

    if (str == null)
        return null;
    array = __Array_init();
    if (array == null)
        return null;
    token = strtok(str, sep);
    while (token != null)
    {
        if (not __Array_push(&array, strdup(token)))
            return null;
        token = strtok(null, sep);
    }
    free(str);
    return array;
}

static arr_t __Array_str_split(char *str, const char *sep)
{
    char *token = strtok(str, sep);
    arr_t array = __Array_init();

    if (array == null)
        return null;
    while (token != null)
    {
        if (not __Array_push(&array, strdup(token)))
        {
            __Array_close(array);
            return null;
        }
        token = strtok(null, sep);
    }
    return array;
}


// YAML FUNCTIONS

static yaml_t *__Yaml_init(const char *filename)
{
    yaml_t *Y = (yaml_t *)malloc(sizeof(yaml_t));

    if (Y == null)
        return null;
    memcpy(Y, &(yaml_t){null}, sizeof(yaml_t));
    Y->lines = __Array_parser(filename);
    Y->nodes = (node_t *)malloc(sizeof(node_t));
    if (Y->lines == null or Y->nodes == null)
        return null;
    memcpy(Y->nodes, &(node_t){null}, sizeof(node_t));
    Y->nodes->val = YAMLVAL_YAML;
    Y->nodes->value = __Array_init();
    if (Y->nodes->value == null)
        return null;
    return Y;
}

static enum yamlval __Yaml_get_type(const char *str)
{
    char *dstr = strdup(str);
    arr_t array = __Array_str_split(dstr, ":");

    if (array == null)
        return YAMLVAL_ERR;
    free(dstr);
    if (__Array_len(array) == 1)
    {
        __Array_close(array);
        return YAMLVAL_YAML;
    }
    dstr = strdup(strcleanedge((char *)array[1]));
    if (dstr == null)
        return YAMLVAL_ERR;
    __Array_close(array);
    if (strlen(dstr) == 0)
        RETFREE(dstr, YAMLVAL_YAML);
    for (size_t n = 0; dstr[n] != '\0'; ++n)
        if (not IS_DIGIT(dstr[n]))
            RETFREE(dstr, YAMLVAL_STR);
    RETFREE(dstr, YAMLVAL_NUM);
}

static unsigned long int __Yaml_layer(const char *str)
{
    unsigned long n = 0;
    while (IS_SPACE(str[n]))
        ++n;
    return (unsigned long int)(n / 2);
}

static node_t *__Yaml_subtreat(node_t *parent, enum yamlval val, char *line)
{
    node_t *self = null;

    if (parent->val != YAMLVAL_YAML)
        return parent;
    self = (node_t *)malloc(sizeof(node_t));
    if (self == null)
        return null;
    self->key = line;
    self->val = val;
    self->parent = parent;
    self->value = ((val == YAMLVAL_YAML) ? __Array_init() : null);
    if (val == YAMLVAL_YAML and self->value == null)
        return null;
    if (not __Array_push((arr_t *)(&(parent->value)), self))
        return null;
    return self;
}

#define RECURSIVE_PARENT(obj, name, n) \
    for (size_t i = 0; i < n; ++i) \
    obj = obj->name

static _Bool __Yaml_treat(yaml_t *y)
{
    enum yamlval val = 0;
    unsigned long int lay = 0;
    unsigned long int oldlay = 0;
    node_t *parent = y->nodes;
    node_t *obj = null;

    for (size_t n = 0; y->lines[n] != null; ++n)
    {
        val = __Yaml_get_type(y->lines[n]);
        if (val == YAMLVAL_ERR)
            return false;
        lay = __Yaml_layer(y->lines[n]);
        obj = __Yaml_subtreat(parent, lay, y->lines[n]);
        // printf("%ld %d %p\n", lay, val, (void *)parent);
        if (lay > oldlay or n == 0)
            parent = obj;
        else if (lay < oldlay)
            parent = obj->parent;
        oldlay = lay;
    }
    return true;
}

void __Yaml_disp(node_t *node)
{
    printf("%s\n", node->key == null ? "null" : node->key);
    if (node->val != YAMLVAL_YAML)
        return;
    for (size_t n = 0; ((arr_t)node->value)[n] != null; ++n)
        __Yaml_disp(((arr_t)node->value)[n]);
}

// PUBLIC FUNCTIONS

/**
 * Close a yaml object.
 * 
 * @public
*/
void yaml_close(yaml_t *yaml)
{
    __Array_close(yaml->lines);
    free(yaml);
}

/**
 * Generates a yaml object by filename.
 * @returns A yaml object, null if it failed.
 * 
 * @public
*/
yaml_t *yaml_parse(const char *filename)
{
    yaml_t *y = null;

    y = __Yaml_init(filename);
    if (y == null)
        return null;
    if (__Yaml_treat(y) == false) {
        yaml_close(y);
        return null;
    }
    // __Yaml_disp(y->nodes);
    return y;
}
