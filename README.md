# CoD4 DM1

[![Checks](https://img.shields.io/github/checks-status/Iswenzz/CoD4-DM1/master?logo=github)](https://github.com/Iswenzz/CoD4-DM1/actions)
[![CodeFactor](https://img.shields.io/codefactor/grade/github/Iswenzz/CoD4-DM1?label=codefactor&logo=codefactor)](https://www.codefactor.io/repository/github/iswenzz/CoD4-DM1)
[![CodeCov](https://img.shields.io/codecov/c/github/Iswenzz/CoD4-DM1?label=codecov&logo=codecov)](https://codecov.io/gh/Iswenzz/CoD4-DM1)
[![License](https://img.shields.io/github/license/Iswenzz/CoD4-DM1?color=blue&logo=gitbook&logoColor=white)](https://github.com/Iswenzz/CoD4-DM1/blob/master/LICENSE)

Reverse of CoD4 & CoD4X ``.DM_1`` demo files with features such as parsing snapshot informations, frames, entities, clients and server messages. This project comes with a CLI and a library.

## Features
* Protocol CoD4 & CoD4X 16+
* Parsing of gamestate, snapshot, frames, entities, clients, configs and server messages
* CoD4 & Q3 huffman code
* Demo reader API

## Building

1. [CMake](https://cmake.org/) and [vcpkg](https://vcpkg.io/en/).
2. [Visual Studio](https://visualstudio.microsoft.com/)

_Build Command:_

    mkdir build && cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=.vcpkg/windows.cmake
    cmake --build .

### [Download](https://github.com/Iswenzz/CoD4-DM1/releases)

## Contributors

***Note:*** If you would like to contribute to this repository, feel free to send a pull request, and I will review your code.
Also feel free to post about any problems that may arise in the issues section of the repository.

<a href="https://github.com/Caball009"><img src="https://avatars.githubusercontent.com/u/82909616?v=4" height=64 style="border-radius: 50%"></a>
