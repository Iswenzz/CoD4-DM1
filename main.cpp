#include <iostream>
#include "Demo.hpp"
using namespace Iswenzz;

int main()
{
    std::string demoPath = "C:\\Users\\Iswenzz\\Desktop\\test.dm_1";
    std::cout << "Openning " << demoPath << " demo." << std::endl;

    Demo demo{ demoPath };

    return 0;
}
