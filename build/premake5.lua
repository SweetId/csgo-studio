workspace "csgo-studio"
	configurations { "Debug", "Release" }
	platforms { "x64" }
	location (_ACTION)

project "csgo-studio-teamspeak"
	kind "SharedLib"
	language "C++"
	targetdir "../Binaries"
	targetname "csgo-studio-teamspeak"

	files {
		"../csgo-studio-teamspeak/**.h",
		"../csgo-studio-teamspeak/**.cpp"
	}

	links {
		"ws2_32"
	}

	filter "configurations:Debug"
		defines { "DEBUG" }
		targetsuffix ("-d")
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"