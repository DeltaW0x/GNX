# GPU Next

### Requirements
* A C++20 compiler for the library itself
* At least a C99 compiler for the C interface
* Lots of patience because I'm learning as I go

### How to build
* `premake5 vs2022/gmake/xcode4/..`
  
  or
* `cmake -S DirToSource -B DirToBuild` (soon<sup>tm</sup>)

### Miscellaneous
This library depends on [volk](https://github.com/zeux/volk) and the [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) for the Vulkan Backend. Both are already included in the library.
