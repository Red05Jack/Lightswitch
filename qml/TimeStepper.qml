import QtQuick

// Shows a time as HH:mm with minus and plus buttons for the hours and the minutes.
Row {
	id: root

	property string timeText: "00:00"
	property int minuteStep: 5

	signal shifted(int minutes)

	spacing: 8

	SettingsButton {
		text: "-"
		onClicked: root.shifted(-60)
	}

	Text {
		width: 50
		height: 64
		horizontalAlignment: Text.AlignHCenter
		verticalAlignment: Text.AlignVCenter
		font.family: Theme.heavyFamily
		font.pixelSize: Theme.valueSize
		color: Theme.value
		text: root.timeText.substring(0, 2)
	}

	SettingsButton {
		text: "+"
		onClicked: root.shifted(60)
	}

	Text {
		width: 16
		height: 64
		horizontalAlignment: Text.AlignHCenter
		verticalAlignment: Text.AlignVCenter
		font.family: Theme.heavyFamily
		font.pixelSize: Theme.valueSize
		color: Theme.label
		text: ":"
	}

	SettingsButton {
		text: "-"
		onClicked: root.shifted(-root.minuteStep)
	}

	Text {
		width: 50
		height: 64
		horizontalAlignment: Text.AlignHCenter
		verticalAlignment: Text.AlignVCenter
		font.family: Theme.heavyFamily
		font.pixelSize: Theme.valueSize
		color: Theme.value
		text: root.timeText.substring(3, 5)
	}

	SettingsButton {
		text: "+"
		onClicked: root.shifted(root.minuteStep)
	}
}
