import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import enchilada.TextureItemModel

ApplicationWindow {
    id: window

    visible: true
    title: "The Big Enchilada"
    minimumHeight: 600
    minimumWidth: 800

    property alias listViewModel : listView.model

    Layout.alignment: Qt.AlignHCenter

    FilterForm {
        id: filterForm
        width: parent.width

        onSearchTextChanged: function(newText) {
            listView.model.setFilterWildcard(newText)
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

        cellWidth: 142
        cellHeight: 192

        delegate: ColumnLayout {
            Rectangle {
                id: textureContainer

                clip: true
                width: 128
                height: 128
                color: "transparent"

                Layout.alignment: Qt.AlignHCenter | Qt.AlignBaseline

                Image {
                    id: textureImage

                    source: model.imageSource

                    anchors.centerIn: model.imageType !== TextureItemModel.Animated ? parent : undefined

                    ShaderEffect {
                        width: textureContainer.width
                        height: textureContainer.height

                        visible: model.imageType == TextureItemModel.Liquid

                        blending: false

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

                        fragmentShader: "/shaders/texture_anim.frag.qsb"

                        property variant source: textureImage
                        property real singleFrameWidth: textureContainer.width / textureImage.width
                        property real time: 0

                        Timer {
                            interval: 200; running: true; repeat: true
                            onTriggered: anim.time += 1.0;
                        }
                    }
                }
            }

            Rectangle {
                radius: 4.0
                width: textItem.width + 6
                height: textItem.height + 2
                Layout.alignment: Qt.AlignHCenter | Qt.AlignBaseline
                Text {
                    id: textItem
                    text: model.display
                    horizontalAlignment: Text.AlignHCenter
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        ScrollBar.vertical: ScrollBar {
            active: parent.moving || !parent.moving
        }
    }
}