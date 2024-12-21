------------------------------------------------------------------------------
-- Utilities
------------------------------------------------------------------------------
local function GetIOResult(cmd)
	local handle = io.popen(cmd) -- Open a console and execute the command.
	local output = handle:read("*a") -- Read the output.
	handle:close() -- Close the handle.

	return output:match("^%s*(.-)%s*$") -- Trim any trailing whitespace (such as newlines)
end

function GetOS()
	local osName = os.getenv("OS")

	if osName == "Windows_NT" then
		return "windows"
	else
		local uname = io.popen("uname"):read("*l")
		if uname == "Linux" then
			return "linux"
		elseif uname == "Darwin" then
			return "macosx"
		end
	end

	return "unknown-os"
end
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Dependencies
------------------------------------------------------------------------------
VULKAN_SDK = os.getenv("VULKAN_SDK")
VULKAN_VERSION = VULKAN_SDK:match("(%d+%.%d+%.%d+)") -- Example: 1.3.290 (without the 0)

MacOSVersion = "14.5"

if GetOS() == "windows" then
	print("On Windows")
else 
	print("On different OS")
end

Dependencies =
{
	Vulkan =
	{
		Windows =
		{
			LibName = "vulkan-1",
			IncludeDir = "%{VULKAN_SDK}/Include/",
			LibDir = "%{VULKAN_SDK}/Lib/"
		},
		Linux =
		{
			LibName = "vulkan",
			IncludeDir = "%{VULKAN_SDK}/include/",
			LibDir = "%{VULKAN_SDK}/lib/"
		},
        MacOS = -- Note: Vulkan on MacOS is currently dynamic. (Example: libvulkan1.3.290.dylib)
		{
			LibName = "vulkan.%{VULKAN_VERSION}",
			IncludeDir = "%{VULKAN_SDK}/../macOS/include/",
			LibDir = "%{VULKAN_SDK}/../macOS/lib/",
		},
	}
}
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Solution
------------------------------------------------------------------------------
outputdir = "%{cfg.buildcfg}-%{cfg.system}"

workspace "Lunar"
	architecture "x86_64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}

group "Lunar"
	include "Lunar"
group ""

include "Sandbox"
------------------------------------------------------------------------------