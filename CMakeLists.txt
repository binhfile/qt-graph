cmake_minimum_required(VERSION 3.16)
project(XYChartWidget VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Tìm Qt
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# Thêm thư viện XYChartWidget
add_library(xychartwidget
    xy_chart_widget.h
    xy_chart_widget.cpp
)

target_link_libraries(xychartwidget
    Qt5::Core
    Qt5::Widgets
)

# Thêm ứng dụng demo
add_executable(example
    example.cpp
)

target_link_libraries(example
    xychartwidget
    Qt5::Core
    Qt5::Widgets
)

# Cài đặt
install(TARGETS xychartwidget
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES xy_chart_widget.h
    DESTINATION include
)
