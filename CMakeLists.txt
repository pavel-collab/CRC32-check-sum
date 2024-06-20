cmake_minimum_required (VERSION 3.11)
project ("Crc32_daemon" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)

# EXECUTABLE FILE PROPERTIES
set(EXEC_FILE "main")
set(SOURCES 
    src/main.cpp
    src/crc32.cpp
    src/event_handler.cpp
    src/Daemon.cpp
    src/Event.cpp
    src/DumpMessage.cpp
)

# EXECUTABLES
add_executable(${EXEC_FILE} ${SOURCES})

# TARGETS LINK LIBRARIES
target_link_libraries(${EXEC_FILE} "dl" "pthread" "m"    ${CMAKE_DL_LIBS})

target_include_directories (${EXEC_FILE} PRIVATE include)
target_include_directories (${EXEC_FILE} PRIVATE ${CMAKE_SOURCE_DIR})

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
