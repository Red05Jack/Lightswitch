import QtQuick

Rectangle {
	property string title: ""
	property bool isHighlighted: false

	radius: Theme.tileRadius
	color: isHighlighted ? Theme.lightTileFill : Theme.tileFill
	border.width: 2
	border.color: isHighlighted ? Theme.lightTileBorder : Theme.tileBorder

	TileText {
		x: 14
		baselineY: 29.6
		text: parent.title
	}
}
