#ifdef GNX_METAL

#include "GNX/GNX.h"
#include "../GNX_common.h"
#include <stdlib.h>
#include <Foundation/Foundation.h>

static const char* METAL_GetError(GNX_Device* device)
{
    return "";
}

static bool METAL_DestroyDevice(GNX_Device* device)
{
    return false;
}

static GNX_Swapchain* METAL_CreateSwapchain(GNX_Device* device, GNX_SwapchainCreateInfo* createInfo)
{
    return NULL;
}

static bool METAL_DestroySwapchain(GNX_Device* device, GNX_Swapchain* swapchain)
{
    return false;
}

static bool METAL_SetSwapchainParameters(GNX_Swapchain* swapchain, GNX_SwapchainComposition format, GNX_SwapchainPresentMode presentMode)
{
    return false;
}

static bool METAL_ResizeSwapchain(GNX_Swapchain* swapchain, uint32_t width, uint32_t height)
{
    return false;
}

static GNX_Device* METAL_CreateDevice(GNX_DeviceCreateInfo* deviceInfo)
{
    GNX_Device* result = (GNX_Device*)malloc(sizeof(GNX_Device));
    ASSIGN_DRIVER(METAL);
    return result;
}

GNX_Bootstrap MetalDriver = {
    METAL_CreateDevice
};

#endif // GNX_METAL
