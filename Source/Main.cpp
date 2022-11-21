#include <iostream>

#define VMA_IMPLEMENTATION
#include <vma/include/vk_mem_alloc.h>

#include "VulkanApp.h"
#include "Resources.h"

int main()
{
	VulkanApp app;
	app.Run();
	return 0;
}
