#include "Demo/Demo.hpp"
#include "Crypt/Huffman.hpp"

#include <gtest/gtest.h>
#include <iostream>

using namespace Iswenzz;

#define DEMO_PATH_19 R"(C:\Users\Iswenzz\Desktop\dev_misc\cod4\1.9.dm_1)"
#define DEMO_PATH_17 R"(C:\Users\Iswenzz\Desktop\dev_misc\cod4\1.7.dm_1)"
#define DEMO_PATH_16 R"(C:\Users\Iswenzz\Desktop\dev_misc\cod4\1.6.dm_1)"

class DemoFixture : public testing::Test
{
public:
    /// <summary>
    /// Initialize huffman compression.
    /// </summary>
    void SetUp() override 
    {
        Huffman::InitMain();
    }
};
