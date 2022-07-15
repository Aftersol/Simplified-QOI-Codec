cmake_minimum_required(VERSION 3.10)

set(CMAKE_CPP_STANDARD 99)
set(CMAKE_CPP_STANDARD_REQUIRED True)

add_executable(qoi_enc 
    example_enc.c
    )

target_include_directories(qoi_enc PUBLIC
    ${PROJECT_SOURCE_DIR}/inc
)

