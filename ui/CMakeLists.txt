#set_target_properties(Enchilada PROPERTIES
#        QT_QMLCACHEGEN_ARGUMENTS "--verbose"
#)

qt6_add_qml_module(Enchilada
        URI enchilada
        VERSION 1.0
        RESOURCE_PREFIX "/qt/qml/"
        QML_FILES "App.qml" "FilterForm.ui.qml"
        NO_GENERATE_QMLTYPES
)

qt6_add_resources(Enchilada config
        PREFIX "/"
        FILES "qtquickcontrols2.conf"
)

qt6_add_shaders(Enchilada shaders
        BATCHABLE
        PRECOMPILE
        OPTIMIZED
        PREFIX "/shaders"
        FILES "texture_water.frag" "texture_anim.frag" "texture_sky.frag"
)