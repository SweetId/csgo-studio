require( "./modules/qt/qt" )

-- this line is optional, but it avoids writting premake.extensions.qt to
-- call the plugin's methods.
local qt = premake.extensions.qt

local qt_path = os.getenv("QT_PATH")
print("Using QT installed at "..qt_path)

workspace "csgo-studio"
	configurations { "Debug", "Release" }
	platforms { "x64" }
	location (_ACTION)

	filter "configurations:Debug"
		defines { "DEBUG" }
		targetsuffix ("-d")
		symbols "On"
	filter {}

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
	filter {}

project "csgo-studio-common"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	targetdir "../Binaries"
	targetname "csgo-studio-common"

	files {
		"../csgo-studio-common/**.h",
		"../csgo-studio-common/**.cpp"
	}

	links {
		"ws2_32"
	}


project "csgo-studio-teamspeak"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	targetdir "../Binaries"
	targetname "csgo-studio-teamspeak"

	includedirs {
		"../csgo-studio-common/"
	}

	files {
		"../csgo-studio-teamspeak/**.h",
		"../csgo-studio-teamspeak/**.cpp"
	}

	links {
		"csgo-studio-common"
	}


project "csgo-studio-gui"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++17"
	targetdir "../Binaries"
	targetname "csgo-studio-gui"

	qt.enable()
	qtpath(qt_path)
	qtmodules { "core", "gui", "widgets", "multimedia" }
	qtprefix "QT5"

	configuration { "Debug" }
		qtsuffix "d"
	configuration { }

	includedirs {
		"../csgo-studio-common/"
	}

	files {
		"../csgo-studio-gui/**.h",
		"../csgo-studio-gui/**.cpp"
	}

	links {
		"csgo-studio-common"
	}
