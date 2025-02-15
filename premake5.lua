workspace "GNX"
    flags { "MultiProcessorCompile" }
	configurations { "Debug", "Release" }
	startproject "TestGNX"
    cppdialect "C++20"
    cdialect "C99"
    
project "GNX"
    kind "SharedLib"
    language "C++"
    targetdir "bin/%{cfg.buildcfg}"

    files 
    { 
        "include/GNX/GNX.h",
        "src/GNX_common.h",
        "src/GNX.cpp"
    }
    includedirs { "include" }
    filter "system:macosx"
        files 
        { 
            "src/metal/GNX_metal.m" 
        }

        defines { "GNX_METAL" }
        links 
        { 
            "Foundation.framework",
            "Metal.framework",
            "QuartzCore.framework"
        }

    filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"


project "TestGNX"
    kind "ConsoleApp"
    language "C++"
    targetdir "bin_testgnx/%{cfg.buildcfg}"
    
    files { "testGNX/main.cpp" }
    links { "GNX" }