cmake_minimum_required(VERSION 3.10)

set(CMAKE_CPP_STANDARD 99)
set(CMAKE_CPP_STANDARD_REQUIRED True)

add_executable(qoi_dec 
    example_dec.c
    )

target_include_directories(qoi_dec PUBLIC
    ${PROJECT_SOURCE_DIR}/inc
)
