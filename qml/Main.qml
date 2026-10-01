import QtQuick
import QtQuick.Window

Window {
	id: window

	readonly property real designScale: Math.min(width, height) / 720

	width: 720
	height: 720
	visible: true
	title: "Lightswitch"
	color: "black"
	visibility: app.isFullscreen ? Window.FullScreen : Window.Windowed

	Item {
		anchors.centerIn: parent
		width: 720
		height: 720
		scale: window.designScale

		TimeTile {
			onSettingsRequested: settingsScreen.visible = true
		}
		LightTile {}
		WeatherTile {}
		CalendarTile {
			onListRequested: calendarScreen.visible = true
		}
		AlarmTile {
			onSettingsRequested: alarmSettingsScreen.visible = true
		}

		SettingsScreen {
			id: settingsScreen

			width: 720
			height: 720
			visible: false
			onClosed: visible = false
		}

		CalendarScreen {
			id: calendarScreen

			width: 720
			height: 720
			visible: false
			onClosed: visible = false
		}

		AlarmSettingsScreen {
			id: alarmSettingsScreen

			width: 720
			height: 720
			visible: false
			onClosed: visible = false
		}
	}

	NightOverlay {
		anchors.fill: parent
		designScale: window.designScale
	}
}
