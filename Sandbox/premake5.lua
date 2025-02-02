MacOSVersion = MacOSVersion or "14.5"

project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++23"
	staticruntime "On"

	architecture "x86_64"

	warnings "Extra"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.hpp",
		"src/**.cpp"
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

		"%{wks.location}/Lunar/src",

		"%{Dependencies.GLFW.IncludeDir}",
		"%{Dependencies.glm.IncludeDir}",
		"%{Dependencies.Vulkan.IncludeDir}",
	}

	links
	{
		"Lunar",
	}

	filter "system:windows"
		defines "LU_PLATFORM_DESKTOP"
		defines "LU_PLATFORM_WINDOWS"
		systemversion "latest"
		staticruntime "on"
		editandcontinue "off"

        defines
        {
            "NOMINMAX"
        }

	filter "system:linux"
		defines "LU_PLATFORM_DESKTOP"
		defines "LU_PLATFORM_LINUX"
		systemversion "latest"
		staticruntime "on"

    filter "system:macosx"
		defines "LU_PLATFORM_DESKTOP"
		defines "LU_PLATFORM_MACOS"
		systemversion(MacOSVersion)
		staticruntime "on"

	filter "action:xcode*"
		-- Note: If we don't add the header files to the externalincludedirs
		-- we can't use <angled> brackets to include files.
		externalincludedirs
		{
			"src",

			"%{wks.location}/Lunar/src",

			"%{Dependencies.GLFW.IncludeDir}",
			"%{Dependencies.glm.IncludeDir}",
			"%{Dependencies.Vulkan.IncludeDir}",
		}

	filter "configurations:Debug"
		defines "LU_CONFIG_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "LU_CONFIG_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		kind "WindowedApp"
		defines "LU_CONFIG_DIST"
		runtime "Release"
		optimize "Full"
