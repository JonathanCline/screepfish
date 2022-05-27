

add_library(BearLibTerminal SHARED IMPORTED GLOBAL)

set(_name )

if (DEFINED TARGET_SYSTEM)
	if(TARGET_SYSTEM STREQUAL "Win64")
		set(_name "Windows64")
	elseif(TARGET_SYSTEM STREQUAL "Win32")
		set(_name "Windows32")
	endif()
else()
	set(_name "Windows64")
endif()

if (WIN32)
	set_target_properties(BearLibTerminal PROPERTIES IMPORTED_IMPLIB "${CMAKE_CURRENT_LIST_DIR}/BearLibTerminal/${_name}/BearLibTerminal.lib")
	set_target_properties(BearLibTerminal PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/BearLibTerminal/${_name}/BearLibTerminal.dll")
	target_include_directories(BearLibTerminal INTERFACE "${CMAKE_CURRENT_LIST_DIR}/BearLibTerminal/Include/C/")
else()
	message(FATAL_ERROR "Fix me")
endif()
