/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2020-2021 Iswenzz

The files are parts of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "DemoReader.hpp"
#include "Huffman.hpp"

#include <iostream>
#include <memory>

using namespace Iswenzz;

#define DEMO_PATH R"(C:\Users\Iswenzz\Desktop\dev_misc\cod4\1.9.dm_1)"
//#define DEMO_PATH R"(C:\Users\Iswenzz\Desktop\dev_misc\cod4\1.7.dm_1)"
//#define DEMO_PATH R"(C:\Users\Iswenzz\Desktop\dev_misc\cod4\1.6.dm_1)"

void TestReader()
{
    std::unique_ptr<DemoReader> demoReader = std::make_unique<DemoReader>(DEMO_PATH);
    while (demoReader->Next())
    {
        //std::cout << demoReader->GetCurrentFrame().commandTime << std::endl;
    }
}

void TestDemo()
{
    std::unique_ptr<Demo> demo = std::make_unique<Demo>(DEMO_PATH);
    demo->Parse();
}

int main()
{
    Huffman::InitMain(); // Initialize huffman compression

    TestDemo();
    //TestReader();

    return 0;
}
