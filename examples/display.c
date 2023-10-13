#include <stdio.h>
#include <yamlc.h>

#define null NULL

int main(int ac, char *av[])
{
    yaml_t *yaml = null;

    if (ac < 2)
        return 1;
    yaml = yaml_parse(av[1]);
    if (yaml == null)
        return 1;
    for (size_t n = 0; yaml->lines[n] != null; ++n)
        printf("% 4ld| %s\n", n, (char *)yaml->lines[n]);
    yaml_close(yaml);
    return 0;
}
