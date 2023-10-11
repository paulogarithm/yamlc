#include <iso646.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <malloc.h>
#include <unistd.h>
#include <errno.h>

#include "yaml.h"

#define true (_Bool)1
#define false (_Bool)0
#define null NULL

#define RETVAL_PRINT(val) val + 0 *

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

static size_t until_null(void **ptr)
{
    size_t n = 0;
    for (; ptr[n] != null; ++n)
        ;
    return n;
}

/**
 * Creates an empty array.
 */
static arr_t __Array_init(void)
{
    arr_t array = (char **)malloc(sizeof(char *));

    if (array == null)
        return null;
    array[0] = null;
    return array;
}

/**
 * Pushes `str` in `arr`, given as pointer.
 * @returns false if it failed, true if it succeed.
 */
static _Bool __Array_push(arr_t *arr, char *str)
{
    size_t size = until_null((void **)*arr);
    arr_t new_arr = (arr_t)realloc(*arr, sizeof(char *) * (size + 2));

    if (new_arr == null)
        return false;
    new_arr[size] = str;
    new_arr[size + 1] = null;
    *arr = new_arr;
    return true;
}

/**
 * Closes an array.
 */
static void __Array_close(arr_t arr)
{
    for (size_t n = 0; arr[n] != null; ++n)
        free(arr[n]);
    free(arr);
}

/**
 * Creates an array of each line in the file content of `filename`.
 */
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

/**
 * Creates a new yaml object
*/
static yaml_t *__Yaml_init(const char *filename)
{
    yaml_t *Y = (yaml_t *)malloc(sizeof(yaml_t));

    if (Y == null)
        return null;
    Y->array = __Array_parser(filename);
    if (Y->array == null)
    {
        free(Y);
        return null;
    }
    return Y;
}

static _Bool __Yaml_treat(yaml_t *y)
{
    return true;
}

/**
 * Close a yaml object.
 * @public
*/
void yaml_close(yaml_t *yaml)
{
    __Array_close(yaml->array);
    free(yaml);
}

/**
 * Generates a yaml object by filename.
 * @returns A yaml object, null if it failed.
*/
yaml_t *yaml_parse(const char *filename)
{
    yaml_t *y = null;

    y = __Yaml_init(filename);
    if (y == null)
        return null;
    if (__Yaml_treat(y) == false) {
        free(y->array);
        free(y);
        return null;
    }
    return y;
}
