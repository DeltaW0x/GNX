#include "GNX_common.h"


#define CHECK_DEVICE(device, retval) \
    if (device == nullptr) {         \
        return retval;               \
    }

#define CHECK_SWAPCHAIN(swapchain, retval) \
    if (swapchain == nullptr) {            \
        return retval;                     \
    }

#define SWAPCHAIN_DEVICE(swapchain) \
    ((SwapchainCommonHeader*)swapchain)->device

static const GNX_Bootstrap *backends[] = {
#ifdef GNX_METAL
    &MetalDriver,
#endif
    nullptr
};

static const GNX_Bootstrap* SelectBootstrap()
{
    return backends[0];
}

const char* GNX_GetError(GNX_Device* device)
{
    CHECK_DEVICE(device, nullptr);
    return device->GetError(device);
}

GNX_Device* GNX_CreateDevice(GNX_DeviceCreateInfo* deviceInfo)
{
    const GNX_Bootstrap *selectedBackend = SelectBootstrap();
    GNX_Device *result = selectedBackend->CreateDevice(deviceInfo);
    return result;
}

bool GNX_DestroyDevice(GNX_Device* device)
{
    CHECK_DEVICE(device, false);
    return device->DestroyDevice(device);
}

GNX_Swapchain* GNX_CreateSwapchain(GNX_Device* device, GNX_SwapchainCreateInfo* createInfo)
{
    CHECK_DEVICE(device, nullptr);
    return device->CreateSwapchain(device, createInfo);
}

bool GNX_DestroySwapchain(GNX_Device* device, GNX_Swapchain* swapchain)
{
    CHECK_DEVICE(device, false);
    CHECK_SWAPCHAIN(swapchain, false);
    return device->DestroySwapchain(device, swapchain);
}

bool GNX_ResizeSwapchain(GNX_Swapchain* swapchain, uint32_t width, uint32_t height)
{
    CHECK_SWAPCHAIN(swapchain, false);
    return SWAPCHAIN_DEVICE(swapchain)->ResizeSwapchain(swapchain, width, height);
}

bool GNX_SetSwapchainParameters(GNX_Swapchain* swapchain, GNX_SwapchainComposition composition, GNX_SwapchainPresentMode presentMode)
{
    CHECK_SWAPCHAIN(swapchain, false);
    return SWAPCHAIN_DEVICE(swapchain)->SetSwapchainParameters(swapchain, composition,presentMode);
}
