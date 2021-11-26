/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2020-2021 Iswenzz, Anomaly

The files are parts of Quake III Arena and Call of Duty 4 source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
===========================================================================
*/

#include "API/DemoReader.hpp"
#include "Crypt/Huffman.hpp"

#include <iostream>
#include <memory>

using namespace Iswenzz;

/// <summary>
/// Command Line Interface.
/// </summary>
/// <returns></returns>
int main()
{
    Huffman::InitMain(); // Initialize huffman compression

    return 0;
}
