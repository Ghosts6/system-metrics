cmake_minimum_required(VERSION 3.16)
project(SystemMetricsClient LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Network Charts Concurrent REQUIRED)

add_executable(system-metrics-client
    src/main.cpp
)

target_link_libraries(system-metrics-client PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Charts
    Qt6::Concurrent
)
