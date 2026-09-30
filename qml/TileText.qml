import QtQuick

Text {
	property real baselineY: 0
	property bool isHeavy: false

	y: baselineY - baselineOffset
	color: Theme.label
	font.family: isHeavy ? Theme.heavyFamily : Theme.boldFamily
	font.pixelSize: Theme.labelSize
}
