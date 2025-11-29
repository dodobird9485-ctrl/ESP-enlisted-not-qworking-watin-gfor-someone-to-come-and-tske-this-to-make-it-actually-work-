cmake_minimum_required(VERSION 3.10)
project(EnlistedESPOverlay)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(EnlistedESPOverlay
    src/main.cpp
    src/game_capture.cpp
    src/esp_renderer.cpp
    src/memory_reader.cpp
)

target_include_directories(EnlistedESPOverlay PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(EnlistedESPOverlay
    d3d11.lib
    dxgi.lib
    user32.lib
    kernel32.lib
    gdi32.lib
)
