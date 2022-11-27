#include <iostream>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "VulkanApp.h"
#include "Resources.h"
#include "ModelLoading.h"

using namespace std::string_literals;

int main()
{
	try
	{
		VulkanApp app;
		app.Run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what();
		return 1;
	}
	return 0;
}
