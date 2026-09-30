import QtQuick

// Global settings: weather location and night mode.
SettingsPage {
	Tile {
		x: 15
		y: 15
		width: 690
		height: 220
		title: "Location"

		SettingsButton {
			x: 14
			y: 80
			width: 110
			height: 100
			text: "<"
			onClicked: app.location.SelectPrevious()
		}

		TileText {
			x: 138
			width: 414
			baselineY: 148
			horizontalAlignment: Text.AlignHCenter
			isHeavy: true
			font.pixelSize: 56
			color: Theme.value
			text: app.location.name
		}

		SettingsButton {
			x: 566
			y: 80
			width: 110
			height: 100
			text: ">"
			onClicked: app.location.SelectNext()
		}
	}

	Tile {
		x: 15
		y: 250
		width: 690
		height: 340
		title: "Night mode"

		TileText {
			x: 14
			baselineY: 103
			color: Theme.value
			text: "Automatic"
		}

		SettingsButton {
			x: 556
			y: 60
			width: 120
			height: 64
			text: app.nightMode.isEnabled ? "ON" : "OFF"
			isAccent: app.nightMode.isEnabled
			onClicked: app.nightMode.SetEnabled(!app.nightMode.isEnabled)
		}

		TileText {
			x: 14
			baselineY: 190
			color: Theme.value
			text: "From"
		}

		TimeStepper {
			x: 256
			y: 148
			timeText: app.nightMode.startTimeText
			onShifted: (minutes) => app.nightMode.ShiftStartTime(minutes)
		}

		TileText {
			x: 14
			baselineY: 280
			color: Theme.value
			text: "To"
		}

		TimeStepper {
			x: 256
			y: 238
			timeText: app.nightMode.endTimeText
			onShifted: (minutes) => app.nightMode.ShiftEndTime(minutes)
		}
	}
}
