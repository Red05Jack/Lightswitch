import QtQuick

Tile {
	x: 15
	y: 250
	width: 455
	height: 455
	title: "Light"
	isHighlighted: true

	TileText {
		x: 6.2
		baselineY: 391.7
		isBlack: true
		font.pixelSize: Theme.heroSize
		color: app.light.isOn ? Theme.accent : Theme.label
		text: app.light.isOn ? "ON" : "OFF"
	}

	MouseArea {
		anchors.fill: parent
		onClicked: app.light.Toggle()
	}
}
