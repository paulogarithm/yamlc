#include <stdio.h>
#include <yamlc.h>

#define null NULL

int main(int ac, char *av[])
{
    yaml_t *yaml = yaml_parse(av[1]);
    yaml_close(yaml);
}