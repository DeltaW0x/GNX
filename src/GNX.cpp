#include "GNX_common.h"

#define LOG(category, message) if(g_logCallback) {(*g_logCallback)(category,message);}

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

static GNX_LogCallback* g_logCallback = nullptr;

static const GNX_Bootstrap* g_backends[] = {
#ifdef GNX_METAL
    &MetalDriver,
#endif
#ifdef GNX_VULKAN
    &VulkanDriver,
#endif
    nullptr
};

static const GNX_Bootstrap* SelectBootstrap(GNX_DeviceDriverFlags driverFlags)
{
    for (int i = 0; g_backends[i]; i += 1){
        if(driverFlags & g_backends[i]->driverFlag){
            return g_backends[i];
        }
    }
    return nullptr;
}

void GNX_SetLogCallback(GNX_LogCallback logCallback)
{
    g_logCallback = new GNX_LogCallback;
    *g_logCallback = logCallback;
}

void GNX_RemoveLogCallback()
{
    if(g_logCallback) delete g_logCallback;
}

GNX_Device* GNX_CreateDevice(GNX_DeviceCreateInfo* deviceInfo, bool debugMode)
{
    GNX_Device *result = nullptr;
    const GNX_Bootstrap *selectedBackend = SelectBootstrap(deviceInfo->driver);
    if(selectedBackend){
        result = selectedBackend->CreateDevice(deviceInfo, debugMode, g_logCallback);
        if(result){
            result->debugMode = debugMode;
            return result;
        }
    }
    LOG(GNX_LOGLEVEL_ERROR,"Failed to find a suitable backend");
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
    return device->CreateSwapchain(device->driverData, createInfo);
}

bool GNX_DestroySwapchain(GNX_Device* device, GNX_Swapchain* swapchain)
{
    CHECK_DEVICE(device, false);
    CHECK_SWAPCHAIN(swapchain, false);
    return device->DestroySwapchain(device->driverData, swapchain);
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
