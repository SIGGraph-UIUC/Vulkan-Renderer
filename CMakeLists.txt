cmake_minimum_required(VERSION 3.10)

set(Vulkan_LIB "$ENV{VULKAN_SDK}/Lib/vulkan-1.lib")
set(Vulkan_INCLUDE "$ENV{VULKAN_SDK}/Include")

# Build HLSL shaders
add_custom_target(Shaders)

# DirectX Shader Compiler is now included in the Vulkan SDK
set(HLSL_COMPILER $ENV{VULKAN_SDK}/Bin/dxc.exe)

set(VERTEX_SHADER_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/TriangleVS.hlsl"
)
set(PIXEL_SHADER_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/TrianglePS.hlsl"
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
  add_custom_command(TARGET Shaders
	COMMAND ${HLSL_COMPILER} -spirv -fvk-use-scalar-layout -E main -T${shadertype}_${shadermodel} $<IF:$<CONFIG:DEBUG>,-Od,-O1> -Zi -Fo ${CMAKE_BINARY_DIR}/${FILE_WE}.spv ${FILE}
    MAIN_DEPENDENCY ${FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
  )
endforeach(FILE)
add_compile_definitions(
	SHADER_PATH=\"${CMAKE_BINARY_DIR}\"
	ASSET_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/Assets\"
	"VK_USE_PLATFORM_WIN32_KHR" 
	"UNICODE"
	"WIN32_LEAN_AND_MEAN"
	"NOMINMAX"
)

add_custom_target(CopyBinaries
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/External/bin/assimp-vc143-mt.dll" ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/External/bin/zlib1.dll" ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/External/bin/pugixml.dll" ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(CopyLayerSettings
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/vk_layer_settings.txt" ${CMAKE_CURRENT_BINARY_DIR}
)

project("VulkanApp")

add_executable(VulkanRenderer
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Main.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanApp.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanUtil.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Window.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/ModelLoading.cpp"
)
target_link_libraries(VulkanRenderer PRIVATE
	${Vulkan_LIB}
	"${CMAKE_CURRENT_SOURCE_DIR}/External/lib/assimp-vc143-mt.lib"
)
target_include_directories(VulkanRenderer PRIVATE
	${Vulkan_INCLUDE}
	"${CMAKE_CURRENT_SOURCE_DIR}/External/include"
)
add_dependencies(VulkanRenderer
	CopyBinaries
	CopyLayerSettings
	Shaders
)

set_property(TARGET VulkanRenderer PROPERTY CXX_STANDARD 17)

