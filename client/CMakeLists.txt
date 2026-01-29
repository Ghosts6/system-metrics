cmake_minimum_required(VERSION 3.16)
project(SystemMetricsClient LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Network Charts Concurrent REQUIRED)

# Set RPATH for build and install
if(UNIX)
    get_target_property(QT_LIB_DIR Qt6::Core LOCATION)
    get_filename_component(QT_LIB_DIR ${QT_LIB_DIR} DIRECTORY)
    SET(CMAKE_INSTALL_RPATH "${QT_LIB_DIR}")
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    SET(CMAKE_SKIP_INSTALL_RPATH  FALSE)
endif()

add_executable(system-metrics-client
    src/main.cpp
    src/apiclient.cpp
    src/apiclient.h
    src/metricswidget.cpp
    src/metricswidget.h
    src/metricschartwidget.cpp
    src/metricschartwidget.h
    src/logviewer.cpp
    src/logviewer.h
    src/dashboardwidget.cpp
    src/dashboardwidget.h
    src/systeminfowidget.cpp
    src/systeminfowidget.h
    src/settingsdialog.cpp
    src/settingsdialog.h
    src/style.cpp
    src/style.h
    src/animatedbutton.cpp
    src/animatedbutton.h
    resources/resources.qrc
)

target_link_libraries(system-metrics-client PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Charts
    Qt6::Concurrent
)
