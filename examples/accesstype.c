#include <stdio.h>
#include <yamlc.h>

#define null NULL

int main(int ac, char *av[])
{
    yaml_t *yaml = yaml_load(av[1]);
    if (yaml == null)
        return 1;
    yamlval_t value = yaml_accesstype(yaml, "threshold.package");
    if (value == YAMLVAL_ERR)
    {
        yaml_close(yaml);
        return 1;
    }
    printf("%d\n", value);
    yaml_close(yaml);
    return 0;
}
