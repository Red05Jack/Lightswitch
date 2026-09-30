pragma Singleton
import QtQuick

QtObject {
	readonly property color accent: "#CA6F54"
	readonly property color label: "#999999"
	readonly property color value: "#EDEDED"
	readonly property color tileFill: "#241F23"
	readonly property color tileBorder: "#2D2B2E"
	readonly property color lightTileFill: "#1F493B"
	readonly property color lightTileBorder: "#3C4E50"
	readonly property color emptyDot: "#2D2B2E"

	readonly property real tileRadius: 15
	readonly property int labelSize: 21
	readonly property int valueSize: 42
	readonly property int heroSize: 133

	readonly property FontLoader boldLoader: FontLoader { source: "fonts/Manufaktur-Bold.ttf" }
	readonly property FontLoader heavyLoader: FontLoader { source: "fonts/Manufaktur-Heavy.ttf" }
	readonly property string boldFamily: boldLoader.name
	readonly property string heavyFamily: heavyLoader.name
}
