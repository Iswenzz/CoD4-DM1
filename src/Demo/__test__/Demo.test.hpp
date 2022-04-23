#include "Demo/Demo.hpp"
#include "Crypt/Huffman.hpp"

#include <gtest/gtest.h>
#include <iostream>

using namespace Iswenzz::CoD4::DM1;

#define DEMO_19 "bin/1.9.dm_1"
#define DEMO_17 "bin/1.7.dm_1"
#define DEMO_16 "bin/1.6.dm_1"

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
