project "Atomica-Engine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	targetdir "Binaries/%{cfg.buildcfg}"
	staticruntime "off"

files { "Include/**.hpp", "Source/**.cpp", "Include/**.h", "Source/**.c" }

includedirs { "Source", "Include" }
   
externalincludedirs { "../Dependencies/include" }

targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")



filter "system:windows"
       systemversion "latest"
       defines {}
       libdirs { "../Dependencies/lib" }
       links   { "glfw3" }
       
filter "system:macosx"
       defines { "MACOS" }
       architecture "ARM64"
       system "macosx"
       libdirs { "../Dependencies/lib-arm64" }
	links   {  
        "glfw3",
        "OpenGL.framework",
        "Cocoa.framework",
        "IOKit.framework",
        "CoreVideo.framework",
        "CoreFoundation.framework",
        "curl"
	}
    externalincludedirs { "/opt/homebrew/opt/curl/include" } -- ✅ Include headers
    libdirs { "/opt/homebrew/opt/curl/lib" }
filter {} 

filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "On"

filter "configurations:Release"
       defines { "RELEASE" }
       runtime "Release"
       optimize "On"
       symbols "On"

filter "configurations:Dist"
       defines { "DIST" }
       runtime "Release"
       optimize "On"
       symbols "Off"