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

using namespace Iswenzz;

int main()
{
    Huffman::InitMain(); // Initialize huffman compression

    std::string demoPath = R"(C:\Users\Iswenzz\Desktop\test.dm_1)";
    //std::string demoPath = R"(C:\Users\Iswenzz\Desktop\ez2.dm_1)";
    std::cout << "Openning " << demoPath << " demo." << std::endl;

    std::unique_ptr<Demo> demo = std::make_unique<Demo>(demoPath);

    return 0;
}
