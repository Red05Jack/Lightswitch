import QtQuick

Text {
	property real baselineY: 0
	property bool isBlack: false

	y: baselineY - baselineOffset
	color: Theme.label
	font.family: isBlack ? Theme.blackFamily : Theme.boldFamily
	font.pixelSize: Theme.labelSize
}
