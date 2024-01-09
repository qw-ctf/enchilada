qt_add_executable(Enchilada
        main.cpp
        WadImageProvider.h
        WadImageProvider.cpp
        TextureItemModel.h
        TextureItemModel.cpp
)

target_link_libraries(Enchilada PRIVATE wad Qt6::Gui Qt6::Quick Qt6::QuickControls2)

set_target_properties(Enchilada PROPERTIES
        WIN32_EXECUTABLE ON
        MACOSX_BUNDLE ON
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)