#include <iostream>
#include "src/Demo.hpp"
#include "src/Huffman.hpp"
using namespace Iswenzz;

int main()
{
    Huffman_InitMain(); // Initialize huffman compression

    std::string demoPath = "C:\\Users\\Iswenzz\\Desktop\\test.dm_1";
    std::cout << "Openning " << demoPath << " demo." << std::endl;

    Demo demo{ demoPath };

    return 0;
}
