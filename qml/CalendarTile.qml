import QtQuick

Tile {
	signal listRequested()

	x: 485
	y: 250
	width: 220
	height: 220
	title: "Calendar"

	// Up to three lines, centered vertically between the label and the date; longer text ends with "...".
	TileText {
		y: 50
		x: 12.3
		width: 196
		height: 120
		verticalAlignment: Text.AlignVCenter
		isHeavy: true
		font.pixelSize: Theme.valueSize
		color: Theme.value
		wrapMode: Text.Wrap
		maximumLineCount: 3
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

	// Links the Google account while it is not linked yet, otherwise opens the list of upcoming entries.
	MouseArea {
		anchors.fill: parent
		onClicked: app.calendar.needsLinking ? app.calendar.BeginLinking() : parent.listRequested()
	}
}
