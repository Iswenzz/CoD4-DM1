#include "API/DemoReader.hpp"
#include "Crypt/Huffman.hpp"
#include "Demo/Demo.hpp"

#include <gtest/gtest.h>
#include <iostream>

using namespace CoD4::DM1;

#define DEMO_19 "bin/1.9.dm_1"
#define DEMO_17 "bin/1.7.dm_1"
#define DEMO_16 "bin/1.6.dm_1"

class DemoFixture : public testing::Test
{
public:
	std::unique_ptr<Demo> DemoFile;
	std::unique_ptr<DemoReader> Reader;

	/// <summary>
	/// Initialize huffman compression.
	/// </summary>
	void SetUp() override
	{
		Huffman::InitMain();
		DemoFile = std::make_unique<Demo>(DEMO_19);
		Reader = std::make_unique<DemoReader>(DEMO_19);
	}
};
