MacOSVersion = MacOSVersion or "14.5"

project "Lunar"
	kind "StaticLib"
	language "C++"
	cppdialect "C++23"
	staticruntime "On"

	architecture "x86_64"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	-- Note: VS2022/Make only need the pchheader filename
	pchheader "lupch.h"
	pchsource "src/Lunar/lupch.cpp"

	files
	{
		"src/Lunar/**.h",
		"src/Lunar/**.hpp",
		"src/Lunar/**.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS",

		"GLFW_INCLUDE_NONE"
	}

	includedirs
	{
		"src",
		"src/Lunar",
	}

	links
	{
	}

	filter "system:windows"
		defines "LU_PLATFORM_WINDOWS"
		systemversion "latest"
		staticruntime "on"
		editandcontinue "off"

        defines
        {
            "NOMINMAX"
        }

	filter "system:linux"
		defines "LU_PLATFORM_LINUX"
		systemversion "latest"
		staticruntime "on"

    filter "system:macosx"
		defines "LU_PLATFORM_MACOS"
		systemversion(MacOSVersion)
		staticruntime "on"

	filter "action:xcode*"
		-- Note: XCode only needs the full pchheader path
		pchheader "src/Lunar/lupch.h"

		-- Note: If we don't add the header files to the externalincludedirs
		-- we can't use <angled> brackets to include files.
		externalincludedirs
		{
			"src",
			"src/Lunar",
		}

	filter "configurations:Debug"
		defines "LU_CONFIG_DEBUG"
		runtime "Debug"
		symbols "on"

        defines
        {
            "TRACY_ENABLE"
        }

	filter "configurations:Release"
		defines "LU_CONFIG_RELEASE"
		runtime "Release"
		optimize "on"

        defines
        {
            "TRACY_ENABLE"
        }

	filter "configurations:Dist"
		defines "LU_CONFIG_DIST"
		runtime "Release"
		optimize "Full"
