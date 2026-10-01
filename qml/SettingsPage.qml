import QtQuick

// Full screen page that hides the tiles; contents are added as children, the close button is part of the page.
Rectangle {
	id: root

	default property alias content: contentArea.data
	property string title: ""

	signal closed()

	color: "black"

	// Swallows touches so that nothing below the page reacts.
	MouseArea {
		anchors.fill: parent
	}

	Item {
		id: contentArea

		anchors.fill: parent
	}

	SettingsButton {
		x: 15
		y: 605
		width: 690
		height: 100
		text: "Close"
		onClicked: root.closed()
	}
}
