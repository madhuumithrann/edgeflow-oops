cmake_minimum_required(VERSION 3.16)
project(EdgeFlowGUI)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(EdgeFlowGUI
    main.cpp
    MainWindow.cpp
    MainWindow.h
    ../load_metrics.cpp
    ../loadbalancer.cpp
    ../edgeserver.cpp
    ../request.cpp
)

target_link_libraries(EdgeFlowGUI PRIVATE Qt6::Widgets)