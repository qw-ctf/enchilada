import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GridLayout {
    rows: 2
    flow: GridLayout.TopToBottom

    columnSpacing: 10
    rowSpacing: 10

    signal searchTextChanged(string text)

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
    }
}
