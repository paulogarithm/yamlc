#include <iostream>
#include <fstream>
#include <yamlc.h>


int main(int ac, char *av[])
{
    yaml_t *yaml = yaml::load(av[1]);
    if (yaml == nullptr)
        return 1;

    void *value = yaml::access(yaml, "threshold.package");
    if (value == nullptr)
    {
        yaml::close(yaml);
        return 1;
    }

    std::cout << (char *)value << std::endl;
    yaml::close(yaml);
    return 0;
}