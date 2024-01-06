qt_add_executable(Enchilada
        mainwindow.ui
        mainwindow.cpp
        mainwindow.h
        main.cpp
)

target_link_libraries(Enchilada PRIVATE wad Qt6::Gui Qt6::Widgets Qt6::Sql)

set_target_properties(Enchilada PROPERTIES
        WIN32_EXECUTABLE ON
        MACOSX_BUNDLE ON
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)