cmake_minimum_required (VERSION 3.11)
project ("Crc32_demon" LANGUAGES C CXX)

# THE 17TH STANDART
set(CMAKE_CXX_STANDARD 14)

# EXECUTABLE FILE PROPERTIES
set(MAIN "main")

# SRC FILES
set(SRC_MAIN main.cpp)
set(SRC_CRC crc32.cpp)
set(SRC_EVENT_HANDLER event_hendler.cpp)
set(SRC_ARG_PARSER arg_parser.cpp)

# EXECUTABLES
add_executable(${MAIN} ${SRC_MAIN} ${SRC_CRC} ${SRC_EVENT_HANDLER} ${SRC_ARG_PARSER})

# TARGETS LINK LIBRARIES
target_link_libraries(${MAIN} "dl" "pthread" "m"    ${CMAKE_DL_LIBS})

add_compile_options(
    -O2    

    -Werror

    -Wall
    -Wextra
    -Wpedantic

    -Wnarrowing 
    -Wwrite-strings 
    -Wcast-qual 
    -Wundef 
    -Wstrict-prototypes 
    -Wbad-function-cast
)