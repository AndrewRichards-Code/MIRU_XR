cmake_minimum_required(VERSION 3.27)

# Files
set(SRC_HEADERS
	"src/miru_xr_core.h"
	"src/miru_xr_core_common.h"
	"src/XRActions.h"
	"src/XRCompositionLayers.h"
	"src/XRInstance.h"
	"src/XRSession.h"
	"src/XRSpace.h"
	"src/XRSwapchain.h"
	"src/XRSystem.h"
	"src/XRViewConfigurations.h"
	)
set(SRC_CPP_FILES
	"src/XRActions.cpp"
	"src/XRInstance.cpp"
	"src/XRSession.cpp"
	"src/XRSpace.cpp"
	"src/XRSwapchain.cpp"
	"src/XRSystem.cpp"
	"src/XRViewConfigurations.cpp"
)

add_library(MIRU_XR_CORE SHARED)
target_sources(MIRU_XR_CORE PRIVATE ${SRC_CPP_FILES} ${SRC_HEADERS})

source_group("src" FILES ${SRC_CPP_FILES} ${SRC_HEADERS})

target_include_directories(MIRU_XR_CORE PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/src/"
	"${CMAKE_CURRENT_SOURCE_DIR}/../External/OpenXR/include/"
	"${MIRU_DIR}/MIRU_CORE/src/"
	"${MIRU_DIR}/External/"
	"${MARS_DIR}/src/"
)

target_link_directories(MIRU_XR_CORE PRIVATE "${MIRU_BUILD_DIR}/bin/")
target_link_libraries(MIRU_XR_CORE PRIVATE MIRU_CORE ARC)
CopyToBinaryDirPostBuild(MIRU_XR_CORE "${MIRU_BUILD_DIR}/bin/${CONFIG_NAME}/MIRU_CORE.dll" "")
CopyToBinaryDirPostBuild(MIRU_XR_CORE "${MIRU_BUILD_DIR}/bin/${CONFIG_NAME}/MIRU_CORE.pdb" "")

target_link_directories(MIRU_XR_CORE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../External/OpenXR/x64/lib/")
target_link_libraries(MIRU_XR_CORE PRIVATE openxr_loader.lib)
CopyToBinaryDirPostBuild(MIRU_XR_CORE "${CMAKE_CURRENT_SOURCE_DIR}/../External/OpenXR/x64/bin/openxr_loader.dll" "")
CopyToBinaryDirPostBuild(MIRU_XR_CORE "${CMAKE_CURRENT_SOURCE_DIR}/../External/OpenXR/x64/bin/openxr_loader.pdb" "")

target_compile_definitions(MIRU_XR_CORE PRIVATE MIRU_XR_BUILD_DLL)
if (WIN32)
	target_include_directories(MIRU_XR_CORE PRIVATE 
		"${MIRU_BUILD_DIR}/packages/Microsoft.Direct3D.D3D12.1.610.5/"
		"${MIRU_DIR}/External/D3D12MemoryAllocator/include/"
	)
	target_compile_definitions(MIRU_XR_CORE PRIVATE MIRU_D3D12)
endif()

find_package(Vulkan)
if (Vulkan_FOUND)
	target_include_directories(MIRU_XR_CORE PRIVATE ${Vulkan_INCLUDE_DIRS})
	target_compile_definitions(MIRU_XR_CORE PRIVATE MIRU_VULKAN)
endif()
