import QtQuick

Tile {
	x: 485
	y: 250
	width: 220
	height: 220
	title: "Calendar"

	TileText {
		x: 12.3
		width: 196
		baselineY: 104.6
		isHeavy: true
		font.pixelSize: Theme.valueSize
		color: Theme.value
		wrapMode: Text.Wrap
		maximumLineCount: 2
		elide: Text.ElideRight
		lineHeightMode: Text.FixedHeight
		lineHeight: 40
		text: app.calendar.titleText
	}

	TileText {
		x: 14
		baselineY: 205
		text: app.calendar.whenText
	}

	// Starts linking the Google account while it is not linked yet.
	MouseArea {
		anchors.fill: parent
		enabled: app.calendar.needsLinking
		onClicked: app.calendar.BeginLinking()
	}
}
