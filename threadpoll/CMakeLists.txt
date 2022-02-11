cmake_minimum_required(VERSION 3.10)
project(threadpool_test)

set(CXX_STANDARD 14)
add_executable(main main.cpp threadpool.cpp)

target_include_directories(main
    PRIVATE
        ${PROJECT_SOURCE_DIR}
)

target_link_libraries(main
    PRIVATE
        pthread
)