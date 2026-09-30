# Defines the standard configurations and the output layout
# <root>/bin/<system><architecture>/<configuration>/.
# Must be included after project().

set(LIGHTSWITCH_CONFIGURATIONS Debug Release DebugLevelLog ReleaseLevelLog)

string(TOLOWER "${CMAKE_SYSTEM_NAME}" _system)
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _processor)
if(_processor MATCHES "^(amd64|x86_64|x64)$")
	set(_architecture x64)
elseif(_processor MATCHES "^(arm64|aarch64)$")
	set(_architecture arm64)
elseif(_processor MATCHES "^(x86|i[3-6]86)$")
	set(_architecture x86)
else()
	message(FATAL_ERROR "Unsupported processor: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
	set(CMAKE_CONFIGURATION_TYPES "${LIGHTSWITCH_CONFIGURATIONS}" CACHE STRING "" FORCE)
else()
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
	endif()
	if(NOT CMAKE_BUILD_TYPE IN_LIST LIGHTSWITCH_CONFIGURATIONS)
		message(FATAL_ERROR "CMAKE_BUILD_TYPE must be one of: ${LIGHTSWITCH_CONFIGURATIONS}")
	endif()
endif()

# The level-log configurations reuse the flags of their base configuration.
foreach(_language C CXX)
	set(CMAKE_${_language}_FLAGS_DEBUGLEVELLOG "${CMAKE_${_language}_FLAGS_DEBUG}" CACHE STRING "" FORCE)
	set(CMAKE_${_language}_FLAGS_RELEASELEVELLOG "${CMAKE_${_language}_FLAGS_RELEASE}" CACHE STRING "" FORCE)
endforeach()
foreach(_linkerKind EXE SHARED MODULE STATIC)
	set(CMAKE_${_linkerKind}_LINKER_FLAGS_DEBUGLEVELLOG "${CMAKE_${_linkerKind}_LINKER_FLAGS_DEBUG}" CACHE STRING "" FORCE)
	set(CMAKE_${_linkerKind}_LINKER_FLAGS_RELEASELEVELLOG "${CMAKE_${_linkerKind}_LINKER_FLAGS_RELEASE}" CACHE STRING "" FORCE)
endforeach()

# Qt only ships Debug and Release libraries.
set(CMAKE_MAP_IMPORTED_CONFIG_DEBUGLEVELLOG Debug Release)
set(CMAKE_MAP_IMPORTED_CONFIG_RELEASELEVELLOG Release)

# MSVC: debug-like configurations must use the debug runtime to match the Qt debug libraries.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug,DebugLevelLog>:Debug>DLL")

foreach(_config IN LISTS LIGHTSWITCH_CONFIGURATIONS)
	string(TOUPPER "${_config}" _upperConfig)
	string(REGEX REPLACE "([a-z])([A-Z])" "\\1_\\2" _directoryName "${_config}")
	string(TOLOWER "${_directoryName}" _directoryName)
	set(_outputDirectory "${CMAKE_SOURCE_DIR}/bin/${_system}${_architecture}/${_directoryName}")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_upperConfig} "${_outputDirectory}")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_upperConfig} "${_outputDirectory}")
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_upperConfig} "${_outputDirectory}")
endforeach()

set(LIGHTSWITCH_LOG_LEVEL_DEFINITION
	$<$<CONFIG:Debug>:LIGHTSWITCH_LOG_LEVEL=2>
	$<$<CONFIG:Release>:LIGHTSWITCH_LOG_LEVEL=0>
	$<$<CONFIG:DebugLevelLog,ReleaseLevelLog>:LIGHTSWITCH_LOG_LEVEL=3>
)
