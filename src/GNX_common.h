#ifndef GNX_COMMON_H
#define GNX_COMMON_H
#include "GNX/GNX.h"
#include <stdbool.h>

typedef struct SwapchainCommonHeader {
    GNX_Device* device;
    GNX_SwapchainComposition composition;
    GNX_SwapchainPresentMode presentmode;
} SwapchainCommonHeader;

typedef struct GNX_Renderer GNX_Renderer;

struct GNX_Device
{
    bool (*DestroyDevice)(GNX_Device* device);
    
    GNX_Swapchain* (*CreateSwapchain)(GNX_Renderer* driverData, GNX_SwapchainCreateInfo* createInfo);
    bool (*DestroySwapchain)(GNX_Renderer* driverData, GNX_Swapchain* swapchain);
    bool (*ResizeSwapchain)(GNX_Swapchain* swapchain, uint32_t width, uint32_t height);
    bool (*SetSwapchainParameters)(GNX_Swapchain* swapchain, GNX_SwapchainComposition format, GNX_SwapchainPresentMode presentMode);
    
    GNX_Renderer* driverData;
    GNX_LogCallback* logCallback;
    bool debugMode;
};

#define ASSIGN_DRIVER_FUNC(func, name) \
    result->func = name##_##func;
#define ASSIGN_DRIVER(name)                          \
    ASSIGN_DRIVER_FUNC(DestroyDevice, name)          \
    ASSIGN_DRIVER_FUNC(CreateSwapchain, name)        \
    ASSIGN_DRIVER_FUNC(DestroySwapchain, name)       \
    ASSIGN_DRIVER_FUNC(ResizeSwapchain, name)        \
    ASSIGN_DRIVER_FUNC(SetSwapchainParameters, name) \

typedef struct GNX_Bootstrap
{
    GNX_Device* (*CreateDevice)(GNX_DeviceCreateInfo* deviceInfo, bool debugMode, GNX_LogCallback* logCallback);
    GNX_DeviceDriverFlags driverFlag;
} GNX_Bootstrap;

#ifdef __cplusplus
extern "C" {
#endif

extern GNX_Bootstrap VulkanDriver;
extern GNX_Bootstrap MetalDriver;

#ifdef __cplusplus
}
#endif

#endif // GNX_COMMON_H
