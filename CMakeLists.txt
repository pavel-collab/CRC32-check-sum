cmake_minimum_required (VERSION 3.11)
project ("Crc32_demon" LANGUAGES C CXX)

# THE 17TH STANDART
set(CMAKE_CXX_STANDARD 14)

# EXECUTABLE FILE PROPERTIES
set(EXEC_FILE "main")
set(INCLUDE_DEMON include/Demon.hpp)
set(SOURCES 
    src/main.cpp
    src/crc32.cpp
    src/event_hendler.cpp
    src/Demon.cpp
    src/Event.cpp
    src/DumpMessage.cpp
)

# EXECUTABLES
add_executable(${EXEC_FILE} ${SOURCES})

# TARGETS LINK LIBRARIES
target_link_libraries(${EXEC_FILE} "dl" "pthread" "m"    ${CMAKE_DL_LIBS})

target_include_directories (${EXEC_FILE} PRIVATE include)

add_compile_options(
    -g
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