import QtQuick

// Alarm settings: global switch and an active flag and time for each weekday.
SettingsPage {
	Tile {
		x: 15
		y: 15
		width: 690
		height: 575
		title: "Alarm"

		SettingsButton {
			x: 556
			y: 8
			width: 120
			height: 48
			fontSize: 32
			text: app.alarm.isEnabled ? "ON" : "OFF"
			isAccent: app.alarm.isEnabled
			onClicked: app.alarm.SetEnabled(!app.alarm.isEnabled)
		}

		Column {
			y: 62
			width: parent.width
			opacity: app.alarm.isEnabled ? 1 : 0.35
			spacing: 8

			Repeater {
				model: ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]

				Item {
					width: parent.width
					height: 64

					TileText {
						x: 14
						baselineY: 40
						color: Theme.value
						text: modelData
					}

					SettingsButton {
						x: 140
						width: 100
						height: 64
						fontSize: 32
						text: app.alarm.activeDays[index] ? "ON" : "OFF"
						isAccent: app.alarm.activeDays[index]
						onClicked: app.alarm.SetDayActive(index, !app.alarm.activeDays[index])
					}

					TimeStepper {
						x: 256
						timeText: app.alarm.timeTexts[index]
						onShifted: (minutes) => app.alarm.ShiftDayTime(index, minutes)
					}
				}
			}
		}
	}
}
