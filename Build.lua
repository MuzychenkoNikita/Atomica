workspace "Atomica"
	architecture "x64"
	configurations { "Debug", "Release", "Dist" }
	startproject "Atomica-Interface"

filter "system:windows"
	buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }
      
filter "system:macosx"
	architecture "ARM64"
	system "macosx"
	
filter {}

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Atomica-Engine"
	include "Atomica-Engine/Build-Atomica-Engine.lua"
	
group "Atomica-Interface"
	include "Atomica-Interface/Build-Atomica-Interface.lua"