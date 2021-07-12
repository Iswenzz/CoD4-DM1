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

#include "src/Demo.hpp"
#include "src/Huffman.hpp"

#include <iostream>
#include <memory>

#define INPUT_FILE_LOC          "R:\\Games\\CoD4\\"
#define INPUT_FILE_NAME         "demo0003"
#define DEMO_FILE_EXTENSION     ".dm_1"
#define OUTPUT_FILE_EXTENSION   DEMO_FILE_EXTENSION ".frames"

using namespace Iswenzz;

int main()
{
    Huffman::InitMain(); // Initialize huffman compression

    std::string demoPath =      INPUT_FILE_LOC INPUT_FILE_NAME DEMO_FILE_EXTENSION;
    std::string outputPath =    INPUT_FILE_LOC INPUT_FILE_NAME OUTPUT_FILE_EXTENSION;

    std::cout << "Opening demo: " << demoPath << std::endl;
    std::cout << "Parsing frames to: " << outputPath << std::endl;

    Demo *pDemo = new Demo(demoPath, outputPath);
    delete pDemo; // Ensure the output file flushes

    std::cout << "Finished parsing demo" << std::endl;

    return 0;
}
