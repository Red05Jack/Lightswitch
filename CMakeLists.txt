# Linux build entry. On Windows, Lightswitch.slnx (Visual Studio 2026) is the main build;
# this file reads its file lists from the solution projects so both stay identical.
cmake_minimum_required(VERSION 3.22)
project(Lightswitch VERSION 0.1.0 LANGUAGES CXX)

include(cmake/BuildLayout.cmake)
include(cmake/VisualStudioProject.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Network Gui Qml Quick Test)
qt_standard_project_setup()

get_filename_component(LIGHTSWITCH_QT_BIN_DIR "${Qt6_DIR}/../../../bin" ABSOLUTE)

lightswitch_read_project_items("${CMAKE_SOURCE_DIR}/vs/LightswitchCore.vcxproj" ClCompile LIGHTSWITCH_CORE_SOURCES)
add_library(LightswitchCore STATIC ${LIGHTSWITCH_CORE_SOURCES})
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

lightswitch_read_project_items("${CMAKE_SOURCE_DIR}/vs/Lightswitch.vcxproj" ClCompile LIGHTSWITCH_APP_SOURCES)
lightswitch_read_project_items("${CMAKE_SOURCE_DIR}/vs/Lightswitch.vcxproj" QtRcc LIGHTSWITCH_APP_RESOURCES)
qt_add_executable(Lightswitch ${LIGHTSWITCH_APP_SOURCES} ${LIGHTSWITCH_APP_RESOURCES})
target_link_libraries(Lightswitch PRIVATE
	LightswitchCore
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
