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

		TimeTile {}
		LightTile {}
		WeatherTile {}
		CalendarTile {}
		AlarmTile {}
	}

	NightOverlay {
		anchors.fill: parent
		designScale: window.designScale
	}
}
