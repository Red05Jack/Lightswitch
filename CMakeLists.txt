cmake_minimum_required(VERSION 3.22)
project(Lightswitch VERSION 0.1.0 LANGUAGES CXX)

include(cmake/BuildLayout.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Network Test)
qt_standard_project_setup()

get_filename_component(LIGHTSWITCH_QT_BIN_DIR "${Qt6_DIR}/../../../bin" ABSOLUTE)

add_library(LightswitchCore STATIC
	src/core/SystemClock.cpp
	src/core/ClockModel.cpp
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
