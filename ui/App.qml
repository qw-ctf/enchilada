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

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            previewImage.source = textureImage.source;
                            previewImage.scale = 4.0;
                            previewFlickable.contentX = 0;
                            previewFlickable.contentY = 0;
                            previewPopup.open();
                        }
                    }

                    Image {
                        id: textureImageClouds
                        source: model.category == TextureItemModel.Sky ? "image://wad/" + model.texture_id + "?s=clouds" : ""
                        visible: false
                        fillMode: Image.PreserveAspectFit
                        width: textureContainer.width
                        height: textureContainer.height
                        sourceSize.width: 64
                        sourceSize.height: 64
                        smooth: false
                    }

                    Image {
                        id: textureImage
                        source: "image://wad/" + model.texture_id + (model.category == TextureItemModel.Sky ? "?s=sky" : "")
                        anchors.centerIn: model.category !== TextureItemModel.Animated ? parent : undefined
                        visible: true
                        fillMode: model.category !== TextureItemModel.Animated ? Image.PreserveAspectFit : Image.PreserveAspectCrop
                        width: textureContainer.width
                        height: textureContainer.height
                        sourceSize.width: 64
                        sourceSize.height: 64
                        smooth: false

                        ShaderEffect {
                            id: sky

                            width: textureContainer.width
                            height: textureContainer.height

                            visible: model.category == TextureItemModel.Sky

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

                            visible: model.category == TextureItemModel.Liquid

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

                            visible: model.category == TextureItemModel.Animated

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
                        text: model.name
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

    Popup {
        id: previewPopup
        visible: false
        width: window.width
        height: window.height

        onOpened: function() {
            previewPopup.forceActiveFocus(Qt.MouseFocusReason);
        }

        Flickable {
            id: previewFlickable

            width: window.width
            height: window.height

            contentWidth: window.width * 4
            contentHeight: window.height * 4

            Image {
                id: previewImage
                source: ""
                fillMode: Image.Tile
                width: window.width * 3
                height: window.height * 3
                smooth: false

                PinchHandler {
                    minimumRotation: 0
                    maximumRotation: 0

                    onScaleChanged: function() {
                        previewImage.scale = Math.max(1.0, Math.min(previewImage.scale, 32.0));
                    }
                }
            }

            ShaderEffectSource {
                sourceItem: previewImage
                wrapMode: ShaderEffectSource.Repeat
            }

            MouseArea {
                anchors.fill: parent
                propagateComposedEvents: true
                onClicked: function(event) {
                    previewPopup.close();
                }
            }
        }
    }
}

