import QtQuick

Tile {
	x: 15
	y: 15
	width: 455
	height: 220
	title: "Time"

	TileText {
		x: 6.2
		baselineY: 156.7
		isBlack: true
		font.pixelSize: Theme.heroSize
		color: Theme.value
		text: app.clock.timeText
	}

	TileText {
		x: 14
		baselineY: 205
		text: app.clock.dateText
	}
}
