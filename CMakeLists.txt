cmake_minimum_required(VERSION 3.22)
project(Lightswitch VERSION 0.1.0 LANGUAGES CXX)

include(cmake/BuildLayout.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Network Gui Qml Quick Test)
qt_standard_project_setup()

get_filename_component(LIGHTSWITCH_QT_BIN_DIR "${Qt6_DIR}/../../../bin" ABSOLUTE)

add_library(LightswitchCore STATIC
	src/core/SystemClock.cpp
	src/core/ClockModel.cpp
	src/core/ILightController.cpp
	src/core/DummyLightController.cpp
	src/core/AlarmController.cpp
	src/weather/OpenMeteoParser.cpp
	src/weather/WeatherModel.cpp
	src/weather/WeatherService.cpp
	src/calendar/DummyCalendarProvider.cpp
	src/calendar/CalendarModel.cpp
	src/app/Configuration.cpp
	src/app/LogConfiguration.cpp
	src/app/AppController.cpp
)
target_include_directories(LightswitchCore PUBLIC
	src/core
	src/weather
	src/calendar
	src/app
)
target_link_libraries(LightswitchCore PUBLIC Qt6::Core Qt6::Network)
target_compile_definitions(LightswitchCore PUBLIC ${LIGHTSWITCH_LOG_LEVEL_DEFINITION})

enable_testing()
add_subdirectory(tests)

add_subdirectory(qml)

qt_add_executable(Lightswitch src/app/main.cpp)
target_link_libraries(Lightswitch PRIVATE
	LightswitchCore
	LightswitchUi
	LightswitchUiplugin
	Qt6::Gui
	Qt6::Qml
	Qt6::Quick
)

add_custom_command(TARGET Lightswitch POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_SOURCE_DIR}/config/lightswitch.ini
		$<TARGET_FILE_DIR:Lightswitch>/lightswitch.ini
)

if(WIN32)
	find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${LIGHTSWITCH_QT_BIN_DIR}" REQUIRED)
	add_custom_command(TARGET Lightswitch POST_BUILD
		COMMAND ${WINDEPLOYQT_EXECUTABLE} --no-translations --qmldir ${CMAKE_SOURCE_DIR}/qml $<TARGET_FILE:Lightswitch>
	)
endif()
