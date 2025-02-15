#ifndef GNX_H
#define GNX_H
#include <stdint.h>
#include <stdbool.h>

#ifdef GNX_METAL
    #ifdef __OBJC__
        @class CAMetalLayer;
    #else
        typedef void* CAMetalLayer;
    #endif
#endif

#ifndef GNXCALL
    #if defined(WIN32) && !defined(__GNUC__)
        #define GNXCALL __cdecl
    #else
        #define GNXCALL
    #endif
#endif // GNXCALL

#ifndef GNX_DECLSPEC
    #if defined(WIN32)
        #ifdef DLL_EXPORT
            #define GNX_DECLSPEC __declspec(dllexport)
        #else
            #define GNX_DECLSPEC
        #endif
    #else
        #if defined(__GNUC__) && __GNUC__ >= 4
            #define GNX_DECLSPEC __attribute__ ((visibility("default")))
        #else
            #define GNX_DECLSPEC
        #endif
    #endif
#endif // GNX_DECLSPEC

#ifdef __cplusplus
extern "C"{
#endif

typedef struct GNX_Device GNX_Device;
typedef struct GNX_Swapchain GNX_Swapchain;

typedef uint32_t GNX_DeviceDriver;
#define GNX_DEVICEDRIVER_METAL     (1u << 0)
#define GNX_DEVICEDRIVER_VULKAN    (1u << 1)
#define GNX_DEVICEDRIVER_DIRECTX12 (1u << 2)

typedef uint32_t GNX_DeviceType;
#define GNX_DEVICETYPE_DISCRETE   (1u << 0)
#define GNX_DEVICETYPE_INTEGRATED (1u << 1)

typedef uint32_t GNX_DeviceFeature;
#define GNX_DEVICEFEATURE_RAYTRACING              (1u << 0)
#define GNX_DEVICEFEATURE_MESH_SHADERS            (1u << 1)
#define GNX_DEVICEFEATURE_VARIABLE_RATE_SHADING   (1u << 2)

typedef enum GNX_SwapchainComposition {
    GNX_SWAPCHAINCOMPOSITION_SDR,
    GNX_SWAPCHAINCOMPOSITION_HDR10
} GNX_SwapchainComposition;

typedef enum GNX_SwapchainPresentMode {
    GNX_SWAPCHAINPRESENTMODE_VSYNC,
    GNX_SWAPCHAINPRESENTMODE_IMMEDIATE
} GNX_SwapchainPresentMode;

typedef struct GNX_DeviceCreateInfo {
    GNX_DeviceDriver driver;
    GNX_DeviceType preferredDeviceType;
    GNX_Devicefeature requiredFeatures;
    GNX_Devicefeature optionalFeatures;
} GNX_DeviceCreateInfo;

typedef struct GNX_SwapchainCreateInfo {
    uint32_t realWindowWidth;
    uint32_t realWindowHeight;
    GNX_SwapchainComposition composition;
    GNX_SwapchainPresentMode presentmode;
#ifdef WIN32
    HINSTANCE hinstance;
    HWND hwnd;
#endif // __WIN32
    
#ifdef __APPLE__
    const CAMetalLayer* metalLayer;
#endif // __APPLE__

#ifdef __LINUX__
    union {
        struct {
            struct wl_display* waylandDisplay;
            struct wl_surface* waylandSurface;
        };
        struct {
            struct xcb_connection_t* xcbConnection;
            struct xcb_window_t* xcbWindow;
        };
    }
#endif // __LINUX__
} GNX_SwapchainCreateInfo;

extern GNX_DECLSPEC const char* GNXCALL GNX_GetError(GNX_Device* device);

extern GNX_DECLSPEC GNX_Device* GNXCALL GNX_CreateDevice(GNX_DeviceCreateInfo* deviceInfo);
extern GNX_DECLSPEC bool        GNXCALL GNX_DestroyDevice(GNX_Device* device);

extern GNX_DECLSPEC GNX_Swapchain* GNXCALL GNX_CreateSwapchain(GNX_Device* device, GNX_SwapchainCreateInfo* createInfo);
extern GNX_DECLSPEC bool           GNXCALL GNX_DestroySwapchain(GNX_Device* device, GNX_Swapchain* swapchain);
extern GNX_DECLSPEC bool           GNXCALL GNX_ResizeSwapchain(GNX_Swapchain* swapchain, uint32_t width, uint32_t height);
extern GNX_DECLSPEC bool           GNXCALL GNX_SetSwapchainParameters(GNX_Swapchain* swapchain, GNX_SwapchainComposition composition, GNX_SwapchainPresentMode presentMode);

#ifdef __cplusplus
}
#endif


#endif // GNX_H

