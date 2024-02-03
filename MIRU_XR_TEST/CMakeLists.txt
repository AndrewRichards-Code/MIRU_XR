cmake_minimum_required(VERSION 3.27)

#Files
set(SRC_CPP_FILES
	"src/main.cpp"
)
set(HLSL_FILES
	"res/shaders/basic.hlsl"
)
set(HLSL_JSON_FILES
	"res/shaders/basic_hlsl.json"
)

add_executable(MIRU_XR_TEST)
target_sources(MIRU_XR_TEST PRIVATE ${SRC_CPP_FILES} ${HLSL_FILES} ${HLSL_JSON_FILES})

target_link_directories(MIRU_XR_TEST PRIVATE "${MIRU_BUILD_DIR}/bin/")
target_link_libraries(MIRU_XR_TEST PRIVATE MIRU_XR_CORE MIRU_CORE ARC)

source_group("src" FILES ${SRC_CPP_FILES})
source_group("res\\shaders" FILES ${HLSL_FILES} ${HLSL_JSON_FILES})

target_include_directories(MIRU_XR_TEST PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/../MIRU_XR_CORE/src/"
	"${CMAKE_CURRENT_SOURCE_DIR}/../External/OpenXR/include/"
	"${MIRU_DIR}/MIRU_CORE/src/"
	"${MIRU_DIR}/External/"
	"${MARS_DIR}/src/"
)

set_property(SOURCE ${HLSL_FILES} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")

target_compile_definitions(MIRU_XR_TEST PRIVATE "BUILD_DIR=\"${CMAKE_BINARY_DIR}\"")
target_compile_definitions(MIRU_XR_TEST PRIVATE "SOLUTION_DIR=\"${CMAKE_SOURCE_DIR}\"")
target_compile_definitions(MIRU_XR_TEST PRIVATE "MIRU_DIR=\"${MIRU_DIR}\"")

add_custom_command(TARGET MIRU_XR_TEST POST_BUILD
	COMMAND "${MIRU_BUILD_DIR}/bin/${CONFIG_NAME}/MIRU_SHADER_COMPILER.exe"
		-raf:${HLSL_JSON_FILES}
		-rafd:$BUILD_DIR=\"${CMAKE_BINARY_DIR}\"
		-rafd:$SOLUTION_DIR=\"${CMAKE_SOURCE_DIR}\"
		-rafd:$MIRU_DIR=\"${MIRU_DIR}\"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

foreach(HLSL_JSON_FILE ${HLSL_JSON_FILES})
	CopyToBuildDirPostBuild(MIRU_XR_TEST "${CMAKE_CURRENT_SOURCE_DIR}/${HLSL_JSON_FILE}" "shaderbin")
endforeach()