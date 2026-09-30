# Builds a QtTest executable from <name>.cpp and registers it with CTest.
function(add_lightswitch_test name)
	qt_add_executable(${name} ${name}.cpp)
	target_link_libraries(${name} PRIVATE LightswitchCore Qt6::Test)
	add_test(NAME ${name} COMMAND ${name})
	if(WIN32)
		set_tests_properties(${name} PROPERTIES
			ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${LIGHTSWITCH_QT_BIN_DIR}")
	endif()
endfunction()

add_lightswitch_test(ClockModelTest)
add_lightswitch_test(DummyLightControllerTest)
add_lightswitch_test(AlarmControllerTest)
add_lightswitch_test(ConfigurationTest)
add_lightswitch_test(OpenMeteoParserTest)
