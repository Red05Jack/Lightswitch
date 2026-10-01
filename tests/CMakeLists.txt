# Builds a QtTest executable from the sources of its Visual Studio project and registers it with CTest.
function(add_lightswitch_test name projectFile)
	lightswitch_read_project_items("${projectFile}" ClCompile testSources)
	qt_add_executable(${name} ${testSources})
	target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
	target_link_libraries(${name} PRIVATE LightswitchCore Qt6::Test)
	add_test(NAME ${name} COMMAND ${name})
	if(WIN32)
		set_tests_properties(${name} PROPERTIES
			ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${LIGHTSWITCH_QT_BIN_DIR}")
	endif()
endfunction()

# Every project of the solution whose name ends with "Test" is a test.
lightswitch_read_solution_projects("${CMAKE_SOURCE_DIR}/Lightswitch.slnx" LIGHTSWITCH_SOLUTION_PROJECTS)
foreach(projectFile IN LISTS LIGHTSWITCH_SOLUTION_PROJECTS)
	get_filename_component(projectName "${projectFile}" NAME_WE)
	if(projectName MATCHES "Test$")
		add_lightswitch_test(${projectName} "${projectFile}")
	endif()
endforeach()
