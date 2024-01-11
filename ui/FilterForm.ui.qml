import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import enchilada.TextureItemModel

GridLayout {
    rows: 2
    flow: GridLayout.TopToBottom

    columnSpacing: 10
    rowSpacing: 10

    signal searchTextChanged(string text)
    signal searchTypeChanged(int value)

    TextField {
        id: searchbar

        Layout.fillWidth: true
        Layout.columnSpan: 3

        placeholderText: qsTr("Search for texture")

        onTextChanged: searchTextChanged(searchbar.text)
    }

    ComboBox {
        id: filter1
        Layout.fillWidth: true
    }
    ComboBox {
        id: filter2
        Layout.fillWidth: true
    }
    ComboBox {
        id: filter3
        Layout.fillWidth: true

        onCurrentIndexChanged: function() {
            searchTypeChanged(model.get(currentIndex)["value"])
        }

        model: ListModel {
            ListElement {
                text: "Any Type"
                value: -1
            }
            ListElement {
                text: "Normal"
                value: TextureItemModel.Normal
            }
            ListElement {
                text: "Liquid"
                value: TextureItemModel.Liquid
            }
            ListElement {
                text: "Sky"
                value: TextureItemModel.Sky
            }
            ListElement {
                text: "Fence"
                value: TextureItemModel.Fence
            }
            ListElement {
                text: "Animated"
                value: TextureItemModel.Animated
            }
        }

        textRole: "text"
    }
}
