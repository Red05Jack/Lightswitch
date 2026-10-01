import QtQuick

Tile {
	signal settingsRequested()

	x: 485
	y: 485
	width: 220
	height: 220
	title: "Alarm"

	TileText {
		x: 12.3
		baselineY: 124.6
		isHeavy: true
		font.pixelSize: Theme.valueSize
		color: Theme.value
		text: app.alarm.nextAlarmText
	}

	Repeater {
		model: 7

		TileText {
			x: 14 + index * 25.2
			baselineY: 205
			color: app.alarm.isEnabled && app.alarm.activeDays[index] ? Theme.accent : Theme.label
			text: "MTWTFSS"[index]
		}
	}

	// A long press opens the alarm settings.
	MouseArea {
		anchors.fill: parent
		onPressAndHold: parent.settingsRequested()
	}
}
