set(LIGHTSWITCH_FONT_DIR ${CMAKE_SOURCE_DIR}/resources/fonts)

set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(${LIGHTSWITCH_FONT_DIR}/Manufaktur-Bold.ttf PROPERTIES QT_RESOURCE_ALIAS fonts/Manufaktur-Bold.ttf)
set_source_files_properties(${LIGHTSWITCH_FONT_DIR}/Manufaktur-Black.ttf PROPERTIES QT_RESOURCE_ALIAS fonts/Manufaktur-Black.ttf)

qt_add_library(LightswitchUi STATIC)
qt_add_qml_module(LightswitchUi
	URI Lightswitch.Ui
	VERSION 1.0
	RESOURCE_PREFIX /qt/qml
	QML_FILES
		Main.qml
		Theme.qml
		Tile.qml
		TileText.qml
		DotMatrix.qml
		TimeTile.qml
		LightTile.qml
		WeatherTile.qml
		CalendarTile.qml
		AlarmTile.qml
	RESOURCES
		${LIGHTSWITCH_FONT_DIR}/Manufaktur-Bold.ttf
		${LIGHTSWITCH_FONT_DIR}/Manufaktur-Black.ttf
)
target_link_libraries(LightswitchUi PRIVATE Qt6::Quick)
