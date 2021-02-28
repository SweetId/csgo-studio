workspace "csgo-studio"
	configurations { "Debug", "Release" }
	platforms { "x64" }
	location (_ACTION)

	filter "configurations:Debug"
		defines { "DEBUG" }
		targetsuffix ("-d")
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"

project "csgo-studio-common"
	kind "StaticLib"
	language "C++"
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