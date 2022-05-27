cmake_minimum_required(VERSION 3.12)

project(screepfish)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME} main.cpp)

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

# Make env link
add_custom_target(
	env_link
	ALL
	COMMAND ${MAKE_LINK_SCRIPT} ${CMAKE_CURRENT_SOURCE_DIR}/env ${CMAKE_CURRENT_BINARY_DIR}/env --quiet
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	VERBATIM
)

# Make assets link
add_custom_target(
	assets_link
	ALL
	COMMAND ${MAKE_LINK_SCRIPT} ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets --quiet
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	VERBATIM
)


target_link_libraries(${PROJECT_NAME} PRIVATE httplib jclib nlohmann_json BearLibTerminal lodepng)
target_include_directories(${PROJECT_NAME} PRIVATE source)

if (CLANG STREQUAL "ON")
	#target_compile_options(${PROJECT_NAME} PUBLIC -ftime-report)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION .)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/env" DESTINATION .)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION .)
