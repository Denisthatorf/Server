project(client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(client
    src/main.cpp
)

target_include_directories(client
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
        PUBLIC
            ${CMAKE_SOURCE_DIR}/common/
            ${CMAKE_SOURCE_DIR}/vendor/asio/asio/include/
            )