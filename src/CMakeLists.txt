# add_subdirectory(tests)

add_library(CoD4DM1Lib
   Demo.cpp Demo.hpp
   DemoData.hpp
   DemoReader.cpp DemoReader.hpp
   Huffman.cpp Huffman.hpp
   Msg.cpp Msg.hpp
   NetFields.cpp NetFields.hpp
   Utility.hpp)

set_target_properties(CoD4DM1Lib PROPERTIES PUBLIC_HEADER "src/Demo.hpp;src/DemoData.hpp;src/DemoReader.hpp;src/Huffman.hpp;src/Msg.hpp;src/NetFields.hpp")
set_target_properties(CoD4DM1Lib PROPERTIES OUTPUT_NAME CoD4DM1)
target_include_directories(CoD4DM1Lib PUBLIC .)
