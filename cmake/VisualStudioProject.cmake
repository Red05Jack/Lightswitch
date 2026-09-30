# Lightswitch.slnx and the projects in vs/ are the source of truth for the file lists.
# CMake (used on Linux) reads them instead of keeping a second list.

# Collects the Include paths of all <itemName> entries of a Visual Studio project as absolute paths.
function(lightswitch_read_project_items projectFile itemName outputVariable)
	get_filename_component(projectDirectory "${projectFile}" DIRECTORY)
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${projectFile}")

	file(READ "${projectFile}" projectContent)
	string(REGEX MATCHALL "<${itemName} Include=\"[^\"]+\"" matches "${projectContent}")

	set(items "")
	foreach(match IN LISTS matches)
		string(REGEX REPLACE "<${itemName} Include=\"([^\"]+)\"" "\\1" relativePath "${match}")
		string(REPLACE "\\" "/" relativePath "${relativePath}")
		get_filename_component(absolutePath "${relativePath}" ABSOLUTE BASE_DIR "${projectDirectory}")
		list(APPEND items "${absolutePath}")
	endforeach()

	set(${outputVariable} "${items}" PARENT_SCOPE)
endfunction()

# Collects the project files listed in the solution as absolute paths.
function(lightswitch_read_solution_projects solutionFile outputVariable)
	get_filename_component(solutionDirectory "${solutionFile}" DIRECTORY)
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${solutionFile}")

	file(READ "${solutionFile}" solutionContent)
	string(REGEX MATCHALL "<Project Path=\"[^\"]+\"" matches "${solutionContent}")

	set(projects "")
	foreach(match IN LISTS matches)
		string(REGEX REPLACE "<Project Path=\"([^\"]+)\"" "\\1" relativePath "${match}")
		list(APPEND projects "${solutionDirectory}/${relativePath}")
	endforeach()

	set(${outputVariable} "${projects}" PARENT_SCOPE)
endfunction()
