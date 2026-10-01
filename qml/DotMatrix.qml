import QtQuick

// 12 columns x 5 rows of dots; the item origin is the center of the bottom-left dot.
Item {
	id: root

	property var columns: []
	readonly property int columnCount: 12
	readonly property int rowCount: 5
	readonly property real pitch: 16.2857
	readonly property real dotRadius: 5.43

	Repeater {
		model: root.columnCount * root.rowCount

		Rectangle {
			readonly property int column: index % root.columnCount
			readonly property int row: Math.floor(index / root.columnCount)

			x: column * root.pitch - root.dotRadius
			y: -row * root.pitch - root.dotRadius
			width: root.dotRadius * 2
			height: root.dotRadius * 2
			radius: root.dotRadius
			color: column < root.columns.length && root.columns[column] > row ? Theme.accent : Theme.emptyDot
		}
	}
}
