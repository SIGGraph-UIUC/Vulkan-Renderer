cmake_minimum_required(VERSION 3.8)

include(ExternalProject)

# Find Vulkan SDK headers
set(vulkan_INCLUDE "$ENV{VULKAN_SDK}/Include")
set(vulkan_LIB "$ENV{VULKAN_SDK}/Lib")

# DirectXMath
ExternalProject_Add(directxmath
	PREFIX "${CMAKE_CURRENT_BINARY_DIR}/directxmath"
	GIT_REPOSITORY "https://github.com/microsoft/DirectXMath.git"
	GIT_TAG "nov2022"
	GIT_SHALLOW true
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
	UPDATE_COMMAND ""
)

# VMA
ExternalProject_Add(vma
	PREFIX "${CMAKE_CURRENT_BINARY_DIR}/vma"
	GIT_REPOSITORY "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git"
	GIT_TAG "v3.0.1"
	GIT_SHALLOW true
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
	UPDATE_COMMAND ""
)

# Build HLSL shaders
add_custom_target(shaders)

# DirectX Shader Compiler is now included in the Vulkan SDK
set(HLSL_COMPILER "$ENV{VULKAN_SDK}/Bin/dxc.exe")

set(VERTEX_SHADER_FILES
	"Source/Shaders/TriangleVS.hlsl"
)
set(PIXEL_SHADER_FILES
	"Source/Shaders/TrianglePS.hlsl"
)
set(ALL_SHADER_FILES ${VERTEX_SHADER_FILES} ${PIXEL_SHADER_FILES})
set_source_files_properties(${VERTEX_SHADER_FILES} PROPERTIES ShaderType "vs")
set_source_files_properties(${PIXEL_SHADER_FILES} PROPERTIES ShaderType "ps")
set_source_files_properties(${ALL_SHADER_FILES} PROPERTIES ShaderModel "6_0")

# Shader build pattern from stackoverflow by Chuck Walbourn at MSFT
foreach(FILE ${ALL_SHADER_FILES})
  get_filename_component(FILE_WE ${FILE} NAME_WE)
  get_source_file_property(shadertype ${FILE} ShaderType)
  get_source_file_property(shadermodel ${FILE} ShaderModel)
  add_custom_command(TARGET shaders
	COMMAND ${HLSL_COMPILER} -spirv -fvk-use-scalar-layout -E main -T${shadertype}_${shadermodel} $<IF:$<CONFIG:DEBUG>,-Od,-O1> -Zi -Fo ${CMAKE_BINARY_DIR}/${FILE_WE}.spv ${FILE}
    MAIN_DEPENDENCY ${FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
  )
endforeach(FILE)
add_compile_definitions(
	SHADER_PATH=\"${CMAKE_BINARY_DIR}/\"
	"VK_USE_PLATFORM_WIN32_KHR" 
	"UNICODE"
	"WIN32_LEAN_AND_MEAN"
	"NOMINMAX"
)

project("Vulkan")

add_executable(Vulkan
	"Source/Main.cpp"
	"Source/VulkanApp.cpp"
	"Source/VulkanUtil.cpp"
	"Source/Resources.cpp"
	"Source/Window.cpp"
)
add_dependencies(Vulkan
	directxmath
	vma
	shaders
)
target_include_directories(Vulkan PRIVATE
	"${vulkan_INCLUDE}"
	"${CMAKE_CURRENT_BINARY_DIR}/directxmath/src"
	"${CMAKE_CURRENT_BINARY_DIR}/vma/src"
)
target_link_libraries(Vulkan PRIVATE
	"${vulkan_LIB}/vulkan-1.lib"
)

set_property(TARGET Vulkan PROPERTY CXX_STANDARD 17)
