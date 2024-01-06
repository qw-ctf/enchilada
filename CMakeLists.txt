cmake_minimum_required(VERSION 3.5)

project(Enchilada VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets Sql)
qt_standard_project_setup()

add_subdirectory(src/app)
add_subdirectory(src/wad)
