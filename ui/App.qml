import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import enchilada.TextureItemModel

ApplicationWindow {
    id: window

    visible: true
    title: "The Big Enchilada"

    width: 1060
    height: 650

    minimumWidth: 480
    minimumHeight: 300

    property alias listViewModel : listView.model

    FilterForm {
        id: filterForm
        width: parent.width

        onSearchTextChanged: function(newText) {
            listViewModel.searchByName = newText
        }
        onSearchTypeChanged: function(newType) {
            listViewModel.searchByType = newType
        }
    }

    GridView {
        id: listView
        clip: true

        anchors {
            left: parent.left
            right: parent.right
            top: filterForm.bottom
            bottom: parent.bottom
            topMargin: 10
            leftMargin: 10
            rightMargin: 10
        }

        cellWidth: 148
        cellHeight: 192

        delegate: Item {

            ColumnLayout {
                Rectangle {
                    id: textureContainer

                    clip: true
                    width: 128
                    height: 128

                    color: "transparent"

                    Image {
                        id: textureImageClouds
                        source: model.imageType == TextureItemModel.Sky ? model.imageSource + "&s=clouds" : ""
                        visible: false
                    }

                    Image {
                        id: textureImage
                        source: model.imageType == TextureItemModel.Sky ? model.imageSource + "&s=sky" : model.imageSource
                        anchors.centerIn: model.imageType !== TextureItemModel.Animated ? parent : undefined
                        visible: true
                        fillMode: Image.Pad

                        ShaderEffect {
                            id: sky

                            width: textureContainer.width
                            height: textureContainer.height

                            visible: model.imageType == TextureItemModel.Sky

                            blending: true
                            layer.smooth: true

                            fragmentShader: "/shaders/texture_sky.frag.qsb"

                            property variant skyTexture: textureImage
                            property variant cloudTexture: textureImageClouds
                            property real time: 0.0

                            NumberAnimation on time {
                                from: 0
                                to: 2.0
                                duration: 60000
                                loops: Animation.Infinite
                            }
                        }

                        ShaderEffect {
                            id: water

                            width: textureContainer.width
                            height: textureContainer.height

                            visible: model.imageType == TextureItemModel.Liquid

                            blending: false
                            layer.smooth: false

                            fragmentShader: "/shaders/texture_water.frag.qsb"

                            property variant source: textureImage
                            property real time: 0.0

                            NumberAnimation on time {
                                from: 0
                                to: 4.0 * Math.PI / 3.0
                                duration: 6000
                                loops: Animation.Infinite
                            }
                        }

                        ShaderEffect {
                            id: anim

                            width: textureContainer.width
                            height: textureContainer.height

                            visible: model.imageType == TextureItemModel.Animated

                            blending: true
                            layer.smooth: false

                            fragmentShader: "/shaders/texture_anim.frag.qsb"

                            property variant source: textureImage
                            property real singleFrameWidth: textureContainer.width / textureImage.width
                            property real time: 0

                            Timer {
                                interval: 300; running: true; repeat: true
                                onTriggered: anim.time += 1.0;
                            }
                        }
                    }
                }

                Rectangle {
                    radius: 4.0
                    width: textItem.width + 6
                    height: textItem.height + 2
                    Layout.alignment: Qt.AlignHCenter
                    Text {
                        id: textItem
                        text: model.display
                        horizontalAlignment: Text.AlignHCenter
                        font.bold: true
                    }
                }
            }
        }

        ScrollBar.vertical: ScrollBar {
            active: parent.moving || !parent.moving
        }
    }
}