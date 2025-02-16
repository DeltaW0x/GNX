#ifndef GNX_H
#define GNX_H
#include <stdint.h>
#include <stdbool.h>

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

typedef void (*GNX_LogCallback)(const char* level, const char* message);
typedef struct GNX_Device GNX_Device;
typedef struct GNX_Swapchain GNX_Swapchain;

#define GNX_LOGLEVEL_DEBUG   "GNX_LOGLEVEL_DEBUG"
#define GNX_LOGLEVEL_INFO    "GNX_LOGLEVEL_INFO"
#define GNX_LOGLEVEL_WARNING "GNX_LOGLEVEL_WARNING"
#define GNX_LOGLEVEL_ERROR   "GNX_LOGLEVEL_ERROR"

typedef uint32_t GNX_DeviceDriverFlags;
#define GNX_DEVICEDRIVER_METAL     (1u << 0)
#define GNX_DEVICEDRIVER_VULKAN    (1u << 1)
#define GNX_DEVICEDRIVER_DIRECTX12 (1u << 2)

typedef enum GNX_DeviceType{
    GNX_DEVICETYPE_DISCRETE,
    GNX_DEVICETYPE_INTEGRATED
} GNX_DeviceType;

typedef uint32_t GNX_DeviceFeatureFlags;
#define GNX_DEVICEFEATURE_RAYTRACING            (1u << 0)
#define GNX_DEVICEFEATURE_MESH_SHADERS          (1u << 1)
#define GNX_DEVICEFEATURE_VARIABLE_RATE_SHADING (1u << 2)

typedef enum GNX_SwapchainComposition {
    GNX_SWAPCHAINCOMPOSITION_SDR,
    GNX_SWAPCHAINCOMPOSITION_HDR10
} GNX_SwapchainComposition;

typedef enum GNX_SwapchainPresentMode {
    GNX_SWAPCHAINPRESENTMODE_VSYNC,
    GNX_SWAPCHAINPRESENTMODE_IMMEDIATE
} GNX_SwapchainPresentMode;

typedef struct GNX_DeviceCreateInfo {
    GNX_DeviceDriverFlags driver;
    GNX_DeviceType preferredDeviceType;
    GNX_DeviceFeatureFlags requiredFeatures;
    GNX_DeviceFeatureFlags optionalFeatures;
    uint32_t applicationVersion;
    uint32_t engineVersion;
    const char* applicationName;
    const char* engineName;
} GNX_DeviceCreateInfo;

typedef struct GNX_SwapchainCreateInfo {
    uint32_t realWindowWidth;
    uint32_t realWindowHeight;
    
    GNX_SwapchainComposition composition;
    GNX_SwapchainPresentMode presentmode;
    
#ifdef _WIN32
    void* hinstance;
    void* hwnd;
#endif // __WIN32
    
#ifdef __APPLE__
    const void* metalLayer;
#endif // __APPLE__

#ifdef __linux__
    bool useX11;
    union {
        struct {
            void* waylandDisplay;
            void* waylandSurface;
        };
        struct {
            void* xcbConnection;
            void* xcbWindow;
        };
    };
#endif // __LINUX__
} GNX_SwapchainCreateInfo;

extern GNX_DECLSPEC void GNXCALL GNX_SetLogCallback(GNX_LogCallback logCallback);
extern GNX_DECLSPEC void GNXCALL GNX_RemoveLogCallback();

extern GNX_DECLSPEC GNX_Device* GNXCALL GNX_CreateDevice(GNX_DeviceCreateInfo* deviceInfo, bool debugMode);
extern GNX_DECLSPEC bool        GNXCALL GNX_DestroyDevice(GNX_Device* device);

extern GNX_DECLSPEC GNX_Swapchain* GNXCALL GNX_CreateSwapchain(GNX_Device* device, GNX_SwapchainCreateInfo* createInfo);
extern GNX_DECLSPEC bool           GNXCALL GNX_DestroySwapchain(GNX_Device* device, GNX_Swapchain* swapchain);
extern GNX_DECLSPEC bool           GNXCALL GNX_ResizeSwapchain(GNX_Swapchain* swapchain, uint32_t width, uint32_t height);
extern GNX_DECLSPEC bool           GNXCALL GNX_SetSwapchainParameters(GNX_Swapchain* swapchain, GNX_SwapchainComposition composition, GNX_SwapchainPresentMode presentMode);

#ifdef __cplusplus
}
#endif


#endif // GNX_H

