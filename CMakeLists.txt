cmake_minimum_required(VERSION 3.10)
project(prismarine C)

set(VALGRIND_EXEC_PARAMETER "/mnt/d/Repos/Momento/src/main/java/")
set(CMAKE_C_FLAGS "-w -g")

# Finding all source files
file(GLOB_RECURSE SRC_FILES "source/*.c")

# Define executable
add_executable(${PROJECT_NAME} ${SRC_FILES})

# Linking pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

# NEED VALGRIND PACKAGE
add_custom_target(
    memcheck
    COMMAND valgrind --leak-check=full --track-origins=yes ${CMAKE_BINARY_DIR}/prismarine ${VALGRIND_EXEC_PARAMETER}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)