import QtQuick

// Touch friendly rounded button with a centered caption.
Rectangle {
	id: root

	property string text: ""
	property bool isAccent: false
	property int fontSize: Theme.valueSize

	signal clicked()

	implicitWidth: 64
	implicitHeight: 64
	radius: Theme.tileRadius
	color: mouseArea.pressed ? Theme.lightTileBorder : Theme.tileBorder

	Text {
		anchors.centerIn: parent
		font.family: Theme.heavyFamily
		font.pixelSize: root.fontSize
		color: root.isAccent ? Theme.accent : Theme.value
		text: root.text
	}

	MouseArea {
		id: mouseArea

		anchors.fill: parent
		onClicked: root.clicked()
	}
}
