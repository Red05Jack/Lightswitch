import QtQuick

Tile {
	x: 485
	y: 15
	width: 220
	height: 220
	title: "Weather"

	DotMatrix {
		x: 20.4
		y: 135.3
		columns: app.weather.dotColumns
	}

	TileText {
		x: 12.3
		baselineY: 205
		isHeavy: true
		font.pixelSize: Theme.valueSize
		color: Theme.value
		text: app.weather.temperatureText
	}

	TileText {
		x: 208 - width
		baselineY: 205
		isHeavy: true
		font.pixelSize: Theme.valueSize
		color: Theme.value
		text: app.weather.precipitationText
	}
}
