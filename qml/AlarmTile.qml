import QtQuick

Tile {
	x: 485
	y: 485
	width: 220
	height: 220
	title: "Alarm"

	TileText {
		x: 12.3
		baselineY: 124.6
		isBlack: true
		font.pixelSize: Theme.valueSize
		color: Theme.value
		text: app.alarm.timeText
	}

	Repeater {
		model: 7

		TileText {
			x: 14 + index * 25.2
			baselineY: 205
			color: app.alarm.activeDays[index] ? Theme.accent : Theme.label
			text: "MTWTFSS"[index]
		}
	}
}
