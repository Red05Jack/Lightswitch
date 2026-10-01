import QtQuick

// List of all upcoming events, reminders and birthdays of the next days.
SettingsPage {
	Tile {
		x: 15
		y: 15
		width: 690
		height: 575
		title: "Calendar"

		ListView {
			x: 14
			y: 52
			width: parent.width - 28
			height: parent.height - 66
			clip: true
			spacing: 6
			model: app.calendar.entries

			delegate: Item {
				width: ListView.view.width
				height: 76

				TileText {
					width: parent.width
					baselineY: 34
					isHeavy: true
					font.pixelSize: 30
					color: modelData.kind === "event" ? Theme.value : Theme.accent
					elide: Text.ElideRight
					text: modelData.title
				}

				TileText {
					baselineY: 64
					text: modelData.whenText + (modelData.kind === "event" ? "" : "  " + (modelData.kind === "birthday" ? "Birthday" : "Reminder"))
				}
			}
		}

		TileText {
			x: 14
			baselineY: 100
			visible: app.calendar.entries.length === 0
			text: "No events"
		}
	}
}
