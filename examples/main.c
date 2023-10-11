#include <stdio.h>
#include <yaml.h>

#define null NULL

int main(void)
{
    yaml_t *yaml = yaml_parse("hello.yml");

    if (yaml == null)
        return 1;
    for (size_t n = 0; yaml->array[n] != null; ++n)
        printf("% 4ld| %s\n", n, yaml->array[n]);
    yaml_close(yaml);
    return 0;
}
