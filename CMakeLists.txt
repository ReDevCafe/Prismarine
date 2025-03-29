cmake_minimum_required(VERSION 3.10)
project(prismarine C)

set(CMAKE_C_FLAGS "-w -g")

# Finding all source files
file(GLOB_RECURSE SRC_FILES "source/*.c")

# Define executable
add_executable(${PROJECT_NAME} ${SRC_FILES})

# Linking pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)