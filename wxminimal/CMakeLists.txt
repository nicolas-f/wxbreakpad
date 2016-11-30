include_directories (./ ${PROJECT_SOURCE_DIR}/../breakpad/src/)


#--------------#
#    SOURCES
#--------------#

set(WX_SOURCES
    minimal.cpp
	abstract_class.cc)

source_group( "wxsources" FILES ${WX_SOURCES} )

#---------------------------------------#
#    DEPENDENCY & EXECUTABLE (OR LIB)
#---------------------------------------#

# Add crash handler library
set(BREAKPAD_LIBS breakpad)
if (APPLE)
  find_library(APPLICATIONSERVICES_LIB ApplicationServices)
  find_library(FOUNDATION_LIB Foundation)
  set(BREAKPAD_LIBS ${BREAKPAD_LIBS} ${FOUNDATION_LIB} ${APPLICATIONSERVICES_LIB})
endif()

# Set Properties->General->Configuration Type to Application
# Creates wx_breakpad with the listed sources
# Adds sources to the Solution Explorer
add_executable(wx_breakpad
    ${WX_SOURCES})

# Creates a folder "executables" and adds target
# project (wx_breakpad.vcproj) under it
set_property(TARGET wx_breakpad PROPERTY FOLDER "executables")

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(wxWidgets_USE_DEBUG ON)
    SET(wxWidgets_USE_STATIC ON)
    MESSAGE(STATUS "Using debug version of wxWidgets")
ELSE()
    SET(wxWidgets_USE_DEBUG OFF)
    SET(wxWidgets_USE_STATIC ON)
    MESSAGE(STATUS "Using static wxWidgets library")
ENDIF()

find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})

# Properties->Linker->Input->Additional Dependencies
target_link_libraries(wx_breakpad ${wxWidgets_LIBRARIES})
target_link_libraries(wx_breakpad ${BREAKPAD_LIBS})

if(WIN32) # Check if we are on Windows
  if(MSVC) # Check if we are using the Visual Studio compiler
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	# I-simpa is a gui program not a shell one
    set_target_properties(wx_breakpad PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    set_target_properties(wx_breakpad PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
  elseif(CMAKE_COMPILER_IS_GNUCXX)
    # SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows") # Not tested
  else()
    message(SEND_ERROR "You are using an unsupported Windows compiler! (Not MSVC or GCC)")
  endif()
ENDIF()

# i-simpa must be built with at least basic
# debug info in order to successfully extract function
# names from it for the crash report
if (UNIX)
  set_target_properties(wx_breakpad PROPERTIES COMPILE_FLAGS -g)
endif()
# Generate debug symbols in msvc release
if(MSVC)
	set_property(TARGET wx_breakpad APPEND PROPERTY LINK_FLAGS /DEBUG)
endif()

#--------------#
#    INSTALL
#--------------#

# Adds logic to INSTALL.vcproj to copy wx_breakpad to destination directory
install (TARGETS wx_breakpad
        RUNTIME DESTINATION .)
