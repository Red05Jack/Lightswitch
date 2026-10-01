import QtQuick

// Black screen with only a dark clock; any touch wakes the display and is consumed.
Rectangle {
	property real designScale: 1

	color: "black"
	visible: app.nightMode.isActive

	Item {
		anchors.centerIn: parent
		width: 720
		height: 720
		scale: parent.designScale

		TileText {
			x: 21.2
			baselineY: 171.7
			isHeavy: true
			font.pixelSize: Theme.heroSize
			color: Theme.nightClock
			text: app.clock.timeText
		}
	}

	MouseArea {
		anchors.fill: parent
		onPressed: app.nightMode.Wake()
	}
}
