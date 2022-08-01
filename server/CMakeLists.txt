project(server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(server
    src/main.cpp
)
include_directories(include)

target_include_directories(server
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
        PUBLIC
            ${CMAKE_SOURCE_DIR}/common/
            ${CMAKE_SOURCE_DIR}/vendor/asio/asio/include/
            )
