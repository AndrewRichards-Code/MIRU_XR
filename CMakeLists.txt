cmake_minimum_required(VERSION 3.27)

project(MIRU_XR)

#Set variables MIRU_DIR, MIRU_BUILD_DIR and MARS_DIR
set(MIRU_DIR "C:/Users/Andrew/source/repos/MIRU" CACHE STRING "Set the location of the MIRU source directory.")
if("${MIRU_DIR}" STREQUAL "")
	message(FATAL " MIRU_DIR must be set.")
endif()
set(MIRU_BUILD_DIR "${MIRU_DIR}/build" CACHE STRING "Set the location of the MIRU build directory.")
if("${MIRU_BUILD_DIR}" STREQUAL "")
message(FATAL " MIRU_BUILD_DIR must be set.")
endif()
set(MARS_DIR "C:/Users/Andrew/source/repos/MARS" CACHE STRING "Set the location of the MARS source directory.")
if("${MARS_DIR}" STREQUAL "")
message(FATAL " MARS_DIR must be set.")
endif()

include("${MIRU_DIR}/External/ARC/src/CMakeFunctions.cmake")


#External Downloads
set(MIRU_XR_OPENXR_VERSION 1.0.30)
set(MIRU_XR_OPENXR_ARCHIVE openxr_loader_windows-${MIRU_XR_OPENXR_VERSION}.zip)
file(DOWNLOAD 
	https://github.com/KhronosGroup/OpenXR-SDK-Source/releases/download/release-${MIRU_XR_OPENXR_VERSION}/${MIRU_XR_OPENXR_ARCHIVE}
	EXPECTED_HASH MD5=378b97b8200bfc09e7c20128818f0569
	"${CMAKE_CURRENT_SOURCE_DIR}/External/OpenXR/${MIRU_XR_OPENXR_ARCHIVE}"
)
file(ARCHIVE_EXTRACT INPUT "${CMAKE_CURRENT_SOURCE_DIR}/External/OpenXR/${MIRU_XR_OPENXR_ARCHIVE}"
	DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/External/OpenXR/"
)
file(REMOVE_RECURSE 
	"${CMAKE_CURRENT_SOURCE_DIR}/External/OpenXR/${MIRU_XR_OPENXR_ARCHIVE}"
	"${CMAKE_CURRENT_SOURCE_DIR}/External/OpenXR/ARM_uwp"
	"${CMAKE_CURRENT_SOURCE_DIR}/External/OpenXR/ARM64_uwp"
	"${CMAKE_CURRENT_SOURCE_DIR}/External/OpenXR/Win32"
	"${CMAKE_CURRENT_SOURCE_DIR}/External/OpenXR/Win32_uwp"
	"${CMAKE_CURRENT_SOURCE_DIR}/External/OpenXR/x64_uwp"
	"${CMAKE_CURRENT_SOURCE_DIR}/External/OpenXR/OpenXRConfig.cmake"
	"${CMAKE_CURRENT_SOURCE_DIR}/External/OpenXR/OpenXRConfigVersion.cmake"
)

#Project Folders
add_subdirectory(MIRU_XR_CORE)
add_subdirectory(MIRU_XR_TEST)

set_property(DIRECTORY "${CMAKE_SOURCE_DIR}/MIRU_XR_TEST" PROPERTY VS_STARTUP_PROJECT MIRU_XR_TEST)