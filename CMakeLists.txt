cmake_minimum_required(VERSION 3.18)

project(Enchilada VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.4 COMPONENTS Gui Quick QuickControls2 ShaderTools Sql REQUIRED)
qt_standard_project_setup(REQUIRES 6.6)

add_subdirectory(src/wad)
add_subdirectory(src/app)
add_subdirectory(ui)
add_subdirectory(db)
