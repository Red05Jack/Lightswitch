import QtQuick
import QtQuick.Window

Window {
	id: window

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
		scale: Math.min(window.width, window.height) / 720

		TimeTile {}
		LightTile {}
		WeatherTile {}
		CalendarTile {}
		AlarmTile {}
	}
}
