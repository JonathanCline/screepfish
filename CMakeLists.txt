cmake_minimum_required(VERSION 3.12)

project(screepfish)

#
#	Variables
#


set(_Link
	httplib
	jclib
	nlohmann_json
	BearLibTerminal
	lodepng
	jccli)

set(_devEnvPath "${CMAKE_CURRENT_LIST_DIR}/env")
set(_devDataPath "${CMAKE_CURRENT_LIST_DIR}/data")



#
#	Machinery
#

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/source/main.cpp)

include("tools/cmake/utility.cmake")

ADD_CMAKE_SUBDIRS_HERE()

macro(MAKE_SCRIPT_COMMAND_NAME out_Name in_ScriptPath)
	if (WIN32)
		set(${out_Name} "${in_ScriptPath}.bat")
	else()
		set(${out_Name} "bash ${in_ScriptPath}.sh")
	endif()
endmacro()

set(MAKE_LINK_SCRIPT )
MAKE_SCRIPT_COMMAND_NAME(MAKE_LINK_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tools/utils/makelink")

# Make env link if env dir exists
if (EXISTS ${_devEnvPath})
	add_custom_target(
		env_link
		ALL
		COMMAND ${MAKE_LINK_SCRIPT} ${_devEnvPath} ${CMAKE_CURRENT_BINARY_DIR}/env --quiet
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		VERBATIM
	)
endif()

# Make assets link
add_custom_target(
	assets_link
	ALL
	COMMAND ${MAKE_LINK_SCRIPT} ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets --quiet
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	VERBATIM
)

# Make data link if data dir exists
if (EXISTS ${_devDataPath})
	add_custom_target(
		data_link
		ALL
		COMMAND ${MAKE_LINK_SCRIPT} ${_devDataPath} ${CMAKE_CURRENT_BINARY_DIR}/data --quiet
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		VERBATIM
	)
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE ${_Link})
target_include_directories(${PROJECT_NAME} PRIVATE source)

install(TARGETS ${PROJECT_NAME} DESTINATION .)

# Add env to install if it exists
if (EXISTS "${_devEnvPath}")
	install(DIRECTORY "${_devEnvPath}" DESTINATION .)
endif()
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION .)
