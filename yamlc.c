#include <iso646.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <malloc.h>
#include <unistd.h>

#include "yamlc.h"

#define true (_Bool)1
#define false (_Bool)0
#define null NULL

#define unused __attribute__((unused))

#define IS_SPACE(c) ((c == ' ' or (c >= 9 and c <= 10)))
#define IS_DIGIT(c) (c >= '0' and c <= '9')

#define MAX(a, b) ((a >= b) ? a : b)

#define CLOSE(ptr)   \
    if (ptr != null) \
    free(ptr)

#define RETFREE(ptr, v) \
    do                  \
    {                   \
        CLOSE(ptr);     \
        return v;       \
    } while (0)

#define RETCLOSEARR(arr, v) \
    do                      \
    {                       \
        __Array_close(arr); \
        return v;           \
    } while (0)

#define RECURSIVE_PARENT(obj, name, n)                        \
    for (size_t i = 0; (i <= n) and (obj->name != null); ++i) \
        obj = obj->name;

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
        CLOSE(buf);
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

static char *strcleanedgedup(const char *str)
{
    char *res = null;
    char *end = null;

    if (str == null)
        return null;
    res = strdup(str);
    if (res == null)
        return null;
    while (IS_SPACE(*res))
        ++res;
    if (*res == '\0')
        return res;
    end = res + strlen(res) - 1;
    while (end > res and IS_SPACE(*end))
        --end;
    *(end + 1) = '\0';
    return res;
}

static char *strrep(char c, size_t rep)
{
    char *str = (char *)malloc(sizeof(char) * (rep + 1));

    if (str == null)
        return null;
    for (size_t n = 0; n < rep; str[n++] = c)
        ;
    str[rep] = '\0';
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
        CLOSE(arr[n]);
    CLOSE(arr);
}

static _Bool __Array_pop(arr_t *arr, size_t pop)
{
    size_t size = __Array_len(*arr);
    arr_t new_arr = null;
    size_t new_index = 0;

    if (*arr == null or pop >= size)
        return false;
    new_arr = (arr_t)malloc(sizeof(void *) * (size));
    if (new_arr == null)
        return false;
    for (size_t index = 0; index < size; ++index)
    {
        if (index == pop)
            continue;
        new_arr[new_index] = (*arr)[index];
        ++new_index;
    }
    new_arr[size - 1] = null;
    CLOSE((*arr)[pop]);
    CLOSE(*arr);
    *arr = new_arr;
    return true;
}

static ssize_t unused __Array_index(arr_t array, void *ptr)
{
    size_t res = 0;

    if (ptr == null)
        return -1;
    for (; array[res] != null; ++res)
        if (array[res] == ptr)
            return res;
    return -1;
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
    CLOSE(str);
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

static yaml_t *__Yaml_getbyname(yaml_t **array, const char *name)
{
    char *dup = strcleanedgedup(name);

    if (dup == null)
        return null;
    for (size_t n = 0; array[n] != null; ++n)
        if (strcmp(name, array[n]->name) == 0)
        {
            CLOSE(dup);
            return array[n];
        }
    CLOSE(dup);
    return null;
}

static yaml_t *__Yaml_init(yaml_t *parent, yamlval_t val, char *line)
{
    yaml_t *self = (yaml_t *)malloc(sizeof(yaml_t));
    char *dup = strdup(line);
    arr_t a = __Array_str_split(dup, ":");

    CLOSE(dup);
    if ((self == null) or (parent == null) or a == null
    or a[0] == null or not __Array_push((arr_t *)&parent->data, self))
    {
        __Array_close(a);
        RETFREE(self, null);
    }
    self->parent = parent;
    self->val = val;
    self->data = null;
    self->name = strdup(strcleanedge(a[0]));
    if (val == YAMLVAL_YAML)
    {
        self->data = __Array_init();
        if (self->data == null)
            return null;
    }
    else
        self->data = strdup(strcleanedge(a[1]));
    __Array_close(a);
    return self;
}

static yamlval_t __Yaml_get_type(const char *str)
{
    char *dstr = strdup(str);
    arr_t array = __Array_str_split(dstr, ":");

    CLOSE(dstr);
    if (array == null or __Array_len(array) == 0)
        return YAMLVAL_ERR;
    if (__Array_len(array) < 2)
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

static void unused __Yaml_disp(yaml_t *node, unsigned lay)
{
    char *str = strrep('_', MAX(0, ((int)lay - 1) * 2));

    printf("%s%s%s", (lay ? "|" : ""), str, node->name);
    CLOSE(str);
    if (node->data == null)
    {
        printf("\n");
        return;
    }
    if (node->val != YAMLVAL_YAML)
    {
        printf(":%s\n", (char *)node->data);
        return;
    }
    printf("\n");
    for (size_t n = 0; ((arr_t)node->data)[n] != null; ++n)
        __Yaml_disp(((yaml_t **)node->data)[n], lay + 1);
}

static void __Yaml_close(yaml_t *y)
{
    if (y == null)
        return;
    CLOSE(y->name);
    if (y->val != YAMLVAL_YAML)
    {
        CLOSE(y->data);
        return;
    }
    for (size_t n = 0; ((yaml_t **)y->data)[n] != null; ++n)
        __Yaml_close(((yaml_t **)y->data)[n]);
    __Array_close((arr_t)y->data);
}

// PREYAML FUNCTIONS

static preyaml_t *__Preyaml_init(const char *filename)
{
    preyaml_t *Y = (preyaml_t *)malloc(sizeof(preyaml_t));

    if (Y == null)
        return null;
    memcpy(Y, &(preyaml_t){null}, sizeof(preyaml_t));
    Y->lines = __Array_parser(filename);
    if (Y->lines == null)
        RETFREE(Y, null);
    Y->main = (yaml_t *)malloc(sizeof(yaml_t));
    if (Y->main == null)
    {
        __Array_close(Y->lines);
        RETFREE(Y, null);
    }
    memcpy(Y->main, &(yaml_t){0}, sizeof(yaml_t));
    Y->main->val = YAMLVAL_YAML;
    Y->main->data = __Array_init();
    if (Y->main->data == null)
        return null;
    return Y;
}

static size_t __Preyaml_layer(const char *str)
{
    size_t n = 0;
    while (IS_SPACE(str[n]))
        ++n;
    return (size_t)(n / 2);
}

static _Bool __Preyaml_treat(preyaml_t *y)
{
    yamlval_t val = 0;
    yaml_t *p = y->main;
    yaml_t *node = null;
    size_t parent_lay = 0, node_lay = 0;

    for (size_t n = 0; y->lines[n] != null; ++n)
    {
        val = __Yaml_get_type(y->lines[n]);
        if (val == YAMLVAL_ERR)
            return false;
        node_lay = __Preyaml_layer(y->lines[n]);
        if (node_lay < parent_lay and p != null)
        {
            RECURSIVE_PARENT(p, parent, parent_lay - node_lay);
            parent_lay = node_lay;
        }
        node = __Yaml_init(p, val, y->lines[n]);
        if (node == null)
            return false;
        if ((node_lay > parent_lay or node_lay == 0))
        {
            if (val == YAMLVAL_YAML)
                p = node;
            parent_lay = node_lay;
        }
    }
    return true;
}

static void __Preyaml_linecleaner(preyaml_t *y)
{
    char *str = null;
    const char *cleaned = null;

    for (size_t n = 0; y->lines[n] != null; ++n)
    {
        str = strdup(y->lines[n]);
        cleaned = strcleanedge(str);
        if (strlen(cleaned) == 0 or cleaned[0] == '#')
            __Array_pop(&y->lines, n--);
        CLOSE(str);
    }
}

void __Preyaml_close(preyaml_t *yaml)
{
    if (yaml == null)
        return;
    __Array_close(yaml->lines);
    __Yaml_close(yaml->main);
    free(yaml->main);
    CLOSE(yaml);
}

// PUBLIC FUNCTIONS

void yaml_close(yaml_t *y)
{
    __Yaml_close(y);
    free(y);
}

void *yaml_access(yaml_t *y, const char *__path)
{
    char *str = strdup(__path);
    arr_t a = __Array_str_split(str, ".");
    yaml_t *current = y;

    CLOSE(str);
    if (a == null)
        return null;
    for (size_t n = 0; a[n] != null; ++n)
    {
        if (current->val != YAMLVAL_YAML)
            RETCLOSEARR(a, null);
        current = __Yaml_getbyname((yaml_t **)current->data, a[n]);
        if (current == null)
            RETCLOSEARR(a, null);
    }
    RETCLOSEARR(a, ((current->val == YAMLVAL_YAML) ? current : current->data));
}

yamlval_t yaml_accesstype(yaml_t *y, const char *__path)
{
    char *str = strdup(__path);
    arr_t a = __Array_str_split(str, ".");
    yaml_t *current = y;

    CLOSE(str);
    if (a == null)
        return YAMLVAL_ERR;
    for (size_t n = 0; a[n] != null; ++n)
    {
        if (current->val != YAMLVAL_YAML)
            RETCLOSEARR(a, YAMLVAL_ERR);
        current = __Yaml_getbyname((yaml_t **)current->data, a[n]);
        if (current == null)
            RETCLOSEARR(a, YAMLVAL_ERR);
    }
    RETCLOSEARR(a, current->val);
}

yaml_t *yaml_load(const char *filename)
{
    preyaml_t *y = null;
    yaml_t *ret = null;

    y = __Preyaml_init(filename);
    if (y == null)
        return null;
    __Preyaml_linecleaner(y);
    if (__Preyaml_treat(y) == false)
    {
        __Preyaml_close(y);
        return null;
    }
    ret = y->main;
    __Array_close(y->lines);
    CLOSE(y);
    return ret;
}
