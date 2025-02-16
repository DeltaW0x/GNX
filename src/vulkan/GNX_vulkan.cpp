#ifdef GNX_VULKAN
#define VOLK_IMPLEMENTATION
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined __linux__
#include <wayland-client.h>
#define VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR
#elif defined __APPLE__
#define VK_USE_PLATFORM_METAL_EXT
#endif

#include "../GNX_common.h"
#include "GNX/GNX.h"
#include "volk.h"
#include <cstring>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#define VK_CHECK(result, message, ret)                                         \
  if (result != VK_SUCCESS) {                                                  \
    if (renderer->logCallback) {                                               \
      (*(renderer->logCallback))(GNX_LOGLEVEL_ERROR, message);                 \
    } else {                                                                   \
      printf("%s\n", message);                                                 \
    }                                                                          \
    return ret;                                                                \
  }
#define LOG(category, message)                                                 \
  if (renderer->logCallback) {                                                 \
    (*(renderer->logCallback))(category, message);                             \
  } else {                                                                     \
    printf("%s\n", message);                                                   \
  }

struct VulkanRenderer {
  VkInstance instance;
  VkDevice logicalDevice;
  VkPhysicalDevice physicalDevice;
  VkDebugUtilsMessengerEXT debugMessenger;

  VkPhysicalDeviceProperties2 properties2;
  VkPhysicalDeviceVulkan11Properties properties11;
  VkPhysicalDeviceVulkan12Properties properties12;
  VkPhysicalDeviceFeatures2 features2;
  VkPhysicalDeviceVulkan11Features features11;
  VkPhysicalDeviceVulkan12Features features12;

  GNX_LogCallback *logCallback;
  bool hasDebugUtils;
  bool hasColorSpace;
  bool debugMode;
};

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {

  VulkanRenderer *renderer = reinterpret_cast<VulkanRenderer *>(pUserData);
  if (renderer->logCallback) {
    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      LOG(GNX_LOGLEVEL_DEBUG, pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      LOG(GNX_LOGLEVEL_INFO, pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      LOG(GNX_LOGLEVEL_WARNING, pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      LOG(GNX_LOGLEVEL_ERROR, pCallbackData->pMessage);
      break;
    default:
      break;
    }
  } else {
    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      printf("Vulkan validation debug: %s\n", pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      printf("Vulkan validation info: %s\n", pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      printf("Vulkan validation warning: %s\n", pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      printf("Vulkan validation error: %s\n", pCallbackData->pMessage);
      break;
    default:
      break;
    }
  }
  return VK_FALSE;
}

static inline VkDebugUtilsMessengerCreateInfoEXT
VULKAN_INTERNAL_DebugUtilsMessengerInfo(VulkanRenderer *renderer) {
  return {.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
          .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
          .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
          .pfnUserCallback = debugCallback,
          .pUserData = reinterpret_cast<void *>(renderer)};
}

static inline bool
VULKAN_INTERNAL_SupportsPresentation(VkPhysicalDevice physicalDevice,
                                     uint32_t queueFamilyIndex) {
#ifdef __linux__
  wl_display *ds = wl_display_connect(nullptr);
  bool result = vkGetPhysicalDeviceWaylandPresentationSupportKHR(
      physicalDevice, queueFamilyIndex, ds);
  wl_display_disconnect(ds);
  return result;
#endif
}

#ifdef _WIN32
#endif

static GNX_Swapchain *
VULKAN_CreateSwapchain(GNX_Renderer *driverData,
                       GNX_SwapchainCreateInfo *createInfo) {
  return nullptr;
}

static bool VULKAN_DestroySwapchain(GNX_Renderer *driverData,
                                    GNX_Swapchain *swapchain) {
  return false;
}

static bool
VULKAN_SetSwapchainParameters(GNX_Swapchain *swapchain,
                              GNX_SwapchainComposition format,
                              GNX_SwapchainPresentMode presentMode) {
  return false;
}

static bool VULKAN_ResizeSwapchain(GNX_Swapchain *swapchain, uint32_t width,
                                   uint32_t height) {
  return false;
}

static inline bool VULKAN_INTERNAL_CheckExtensionSupport(
    const char *extensionName,
    const std::vector<VkExtensionProperties> &availableExtenions) {
  for (auto &availableExt : availableExtenions) {
    if (strcmp(availableExt.extensionName, extensionName) == 0) {
      return true;
    }
  }
  return false;
}

static bool VULKAN_INTERNAL_CheckInstanceExtensions(
    const std::vector<const char *> &requiredExtensions, bool *hasDebugUtils,
    bool *hasColorSpace) {
  uint32_t extensionCount;
  vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(NULL, &extensionCount,
                                         availableExtensions.data());

  for (auto &requiredExt : requiredExtensions) {
    if (!VULKAN_INTERNAL_CheckExtensionSupport(requiredExt,
                                               availableExtensions))
      return false;
  }

  *hasDebugUtils = VULKAN_INTERNAL_CheckExtensionSupport(
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME, availableExtensions);
  *hasColorSpace = VULKAN_INTERNAL_CheckExtensionSupport(
      VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME, availableExtensions);

  return true;
}

static bool VULKAN_INTERNAL_CheckValidationLayers(
    const std::vector<const char *> &validationLayers) {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, NULL);
  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (auto &layer : validationLayers) {
    bool found = false;
    for (auto &availableLayer : availableLayers) {
      if (strcmp(availableLayer.layerName, layer) == 0) {
        found = true;
        break;
      }
    }
    if (!found)
      return false;
  }
  return true;
}

static bool VULKAN_INTERNAL_CreateInstance(VulkanRenderer *renderer,
                                           GNX_DeviceCreateInfo *deviceInfo) {
  VkResult vulkanResult;
  VkInstanceCreateFlags createFlags = 0;

  std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
#ifdef _WIN32
  std::vector<const char *> instanceExtensions = {
      VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
#elif defined __linux__
  std::vector<const char *> instanceExtensions = {
      VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME};
#elif define __APPLE__
  std::vector<const char *> instanceExtensions = {
      VK_KHR_SURFACE_EXTENSION_NAME, VK_EXT_METAL_SURFACE_EXTENSION_NAME,
      VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME};
  createFlags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

  if (!VULKAN_INTERNAL_CheckInstanceExtensions(instanceExtensions,
                                               &renderer->hasDebugUtils,
                                               &renderer->hasColorSpace)) {
    LOG(GNX_LOGLEVEL_ERROR,
        "Your system doesn't support the required Vulkan instance extensions");
    return false;
  }
  if (renderer->hasDebugUtils) {
    instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  } else {
    LOG(GNX_LOGLEVEL_WARNING,
        "Your system doesn't the Vulkan support debug utils extension, you "
        "won't be able to assign names to graphics objects");
  }

  if (renderer->hasColorSpace) {
    instanceExtensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
  } else {
    LOG(GNX_LOGLEVEL_WARNING,
        "Your system doesn't support the swapchain colorspace extension, you "
        "won't be able to use any other colorspace apart from nonlinear srgb");
  }

  bool hasValidationLayers =
      VULKAN_INTERNAL_CheckValidationLayers(validationLayers);
  if (!hasValidationLayers) {
    LOG(GNX_LOGLEVEL_WARNING,
        "Debug was requested, but no Vulkan validation layers are available")
  }

  auto debugMessengerInfo = VULKAN_INTERNAL_DebugUtilsMessengerInfo(renderer);

  VkApplicationInfo appInfo = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                               .pNext = nullptr,
                               .pApplicationName = deviceInfo->applicationName,
                               .applicationVersion =
                                   deviceInfo->applicationVersion,
                               .pEngineName = deviceInfo->engineName,
                               .engineVersion = deviceInfo->engineVersion,
                               .apiVersion = VK_MAKE_API_VERSION(0, 1, 2, 0)};

  VkInstanceCreateInfo instanceCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = renderer->debugMode && hasValidationLayers ? &debugMessengerInfo
                                                          : nullptr,
      .flags = createFlags,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = renderer->debugMode && hasValidationLayers
                               ? (uint32_t)validationLayers.size()
                               : 0,
      .ppEnabledLayerNames = renderer->debugMode && hasValidationLayers
                                 ? validationLayers.data()
                                 : nullptr,
      .enabledExtensionCount = (uint32_t)instanceExtensions.size(),
      .ppEnabledExtensionNames = instanceExtensions.data()};
  VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &renderer->instance),
           "Failed to create Vulkan instance", false);
  volkLoadInstance(renderer->instance);

  if (renderer->debugMode && hasValidationLayers) {
    vkCreateDebugUtilsMessengerEXT(renderer->instance, &debugMessengerInfo,
                                   nullptr, &renderer->debugMessenger);
  }

  return true;
}

static bool
VULKAN_INTERNAL_CreateDevice(VulkanRenderer *renderer,
                             GNX_DeviceCreateInfo *deviceCreateInfo) {
  VkResult result;
  uint32_t deviceCount;
  std::vector<const char *> requiredDeviceExtensions = {
      VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
      VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
  };

  result =
      vkEnumeratePhysicalDevices(renderer->instance, &deviceCount, nullptr);
  VK_CHECK(result, "Failed to enumerate Vulkan devices", false);

  if (deviceCount == 0) {
    LOG(GNX_LOGLEVEL_ERROR, "Failed to find any GPUs with Vulkan support");
  }
  std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
  result = vkEnumeratePhysicalDevices(
      renderer->instance, &deviceCount,
      physicalDevices
          .data()); // https://github.com/libsdl-org/SDL/blob/fd4e6d29493a06518abb00ca51b3a0e29afa3ffc/src/gpu/vulkan/SDL_gpu_vulkan.c#L11328C5-L11332C8

  if (result == VK_INCOMPLETE) {
    LOG(GNX_LOGLEVEL_WARNING,
        "vkEnumeratePhysicalDevices returned VK_INCOMPLETE (somehow). If it "
        "wasn't for SDL_GPU I have no idea how I should have known about this "
        "shit");
    result = VK_SUCCESS;
  }
  VK_CHECK(result, "Failed to enumerate Vulkan devices", false);

  VkPhysicalDeviceProperties2 properties2;
  VkPhysicalDeviceVulkan11Properties properties11;
  VkPhysicalDeviceVulkan12Properties properties12;

  VkPhysicalDeviceFeatures2 features2;
  VkPhysicalDeviceVulkan11Features features11;
  VkPhysicalDeviceVulkan12Features features12;
  VkPhysicalDeviceSynchronization2FeaturesKHR syncronization2Features;
  VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures;

  std::map<uint32_t, VkPhysicalDevice> deviceRanker;
  for (auto &physDevice : physicalDevices) {
    bool suitable;
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &extensionCount,
                                         nullptr);
    std::vector<VkExtensionProperties> availableDeviceExtensions(
        extensionCount);
    vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &extensionCount,
                                         availableDeviceExtensions.data());

    for (auto &requiredExt : requiredDeviceExtensions) {
      suitable = false;
      for (auto &extName : availableDeviceExtensions) {
        if (strcmp(extName.extensionName, requiredExt) == 0) {
          suitable = true;
          break;
        }
      }
      if (!suitable) {
        break;
      }
    }
    if (!suitable) {
      break;
    }

    properties2 = {};
    properties11 = {};
    properties12 = {};
    features2 = {};
    features11 = {};
    features12 = {};
    syncronization2Features = {};
    dynamicRenderingFeatures = {};

    properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    properties11.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
    properties12.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
    properties2.pNext = &properties11;
    properties11.pNext = &properties12;
    properties12.pNext = nullptr;

    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    syncronization2Features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
    dynamicRenderingFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    features2.pNext = &features11;
    features11.pNext = &features12;
    features12.pNext = &syncronization2Features;
    syncronization2Features.pNext = &dynamicRenderingFeatures;
    dynamicRenderingFeatures.pNext = nullptr;

    vkGetPhysicalDeviceProperties2(physDevice, &properties2);
    vkGetPhysicalDeviceFeatures2(physDevice, &features2);

    if (!features2.features.multiDrawIndirect)
      break;
    if (!features2.features.multiViewport)
      break;
    if (!features2.features.imageCubeArray)
      break;
    if (!features2.features.samplerAnisotropy)
      break;
    if (!features2.features.textureCompressionBC)
      break;
    if (!features11.multiview)
      break;
    if (!features12.bufferDeviceAddress)
      break;
    if (!features12.descriptorIndexing)
      break;
    if (!features12.timelineSemaphore)
      break;
    if (!features12.shaderSampledImageArrayNonUniformIndexing)
      break;
    if (!features12.shaderStorageImageArrayNonUniformIndexing)
      break;
    if (!features12.descriptorBindingSampledImageUpdateAfterBind)
      break;
    if (!features12.descriptorBindingStorageImageUpdateAfterBind)
      break;
    if (!features12.descriptorBindingUpdateUnusedWhilePending)
      break;
    if (!features12.descriptorBindingPartiallyBound)
      break;

    uint32_t score = 0;
    if (properties2.properties.deviceType ==
        VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      if (deviceCreateInfo->preferredDeviceType == GNX_DEVICETYPE_DISCRETE) {
        score += 500;
      } else {
        score += 200;
      }
    }
    if (properties2.properties.deviceType ==
        VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
      if (deviceCreateInfo->preferredDeviceType == GNX_DEVICETYPE_INTEGRATED) {
        score += 500;
      } else {
        score += 200;
      }
    }
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount,
                                             nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount,
                                             queueFamilyProps.data());

    for (int i = 0; i < queueFamilyCount; i++) {
      if (VULKAN_INTERNAL_SupportsPresentation(physDevice, i) || (queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
          suitable = true;
          break;
      }
      suitable = false;
    };
  }
  return true;
}

static bool VULKAN_DestroyDevice(GNX_Device *device) {
  VulkanRenderer *renderer = (VulkanRenderer *)device->driverData;
  if (renderer->debugMessenger != VK_NULL_HANDLE) {
    vkDestroyDebugUtilsMessengerEXT(renderer->instance,
                                    renderer->debugMessenger, nullptr);
  }
  vkDestroyInstance(renderer->instance, nullptr);
  delete renderer;
  delete device;
  return true;
}

static GNX_Device *VULKAN_CreateDevice(GNX_DeviceCreateInfo *deviceInfo,
                                       bool debugMode,
                                       GNX_LogCallback *logCallback) {
  VulkanRenderer *renderer = new VulkanRenderer();
  renderer->instance = VK_NULL_HANDLE;
  renderer->logicalDevice = VK_NULL_HANDLE;
  renderer->physicalDevice = VK_NULL_HANDLE;
  renderer->debugMessenger = VK_NULL_HANDLE;
  renderer->debugMode = debugMode;
  renderer->logCallback = logCallback;

  VK_CHECK(volkInitialize(), "Failed to initialize volk", nullptr);

  if (!VULKAN_INTERNAL_CreateInstance(renderer, deviceInfo))
    return nullptr;
  if (!VULKAN_INTERNAL_CreateDevice(renderer, deviceInfo))
    return nullptr;
  GNX_Device *result = new GNX_Device;
  ASSIGN_DRIVER(VULKAN);
  result->driverData = reinterpret_cast<GNX_Renderer *>(renderer);

  LOG(GNX_LOGLEVEL_DEBUG, "Vulkan device created succesfully")
  return result;
}

GNX_Bootstrap VulkanDriver = {VULKAN_CreateDevice, GNX_DEVICEDRIVER_VULKAN};

#endif // GNX_VULKAN
