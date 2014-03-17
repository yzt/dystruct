------------------------------------------------------------------------

solution ("DyStruct" .. "_" .. _ACTION)

------------------------------------------------------------------------
-- Solution Setup --
------------------------------------------------------------------------

	configurations ({"Debug", "Release", "Retail"})
	platforms ({"x64", "x32"})
	defines ({
	})

	location ("../")
	objdir ("../build/" .. _ACTION .. "/obj")

	targetdir ("../out/" .. _ACTION)

	includedirs ({"../include"})
	libdirs ({"../lib"})
	
	flags ({
		--"StaticRuntime",
		--"EnableSSE2", -- Meaningless in 64-bit builds
		"ExtraWarnings",
		"FloatFast",
		"NoEditAndContinue",
		"Unicode"
	})
	
	configuration ({"gmake"})
		buildoptions ({"--std=c++11"})

	configuration ({"macosx", "gmake"})
		buildoptions ({"--stdlib=libc++"})
		
------------------------------------------------------------------------
		
	configuration ({"Debug", "x64"})
		defines ({"_DEBUG"})
		targetsuffix ("64_d")
		flags ({"Symbols"})

	configuration ({"Debug", "x32"})
		defines ({"_DEBUG"})
		targetsuffix ("32_d")
		flags ({"Symbols"})

	configuration ({"Release", "x64"})
		defines ({"NDEBUG"})
		targetsuffix ("64_r")
		flags ({"Optimize", "OptimizeSpeed", "Symbols"})
		
	configuration ({"Release", "x32"})
		defines ({"NDEBUG"})
		targetsuffix ("32_r")
		flags ({"Optimize", "OptimizeSpeed", "Symbols"})
		
	configuration ({"Retail", "x64"})
		defines ({"NDEBUG"})
		targetsuffix ("64")
		flags ({"NoFramePointer", "Optimize", "OptimizeSpeed"})

	configuration ({"Retail", "x32"})
		defines ({"NDEBUG"})
		targetsuffix ("32")
		flags ({"NoFramePointer", "Optimize", "OptimizeSpeed"})

------------------------------------------------------------------------
-- Projects --
------------------------------------------------------------------------

	project ("dystruct_test")
		uuid ("312EC9D8-4541-4B9F-934F-250D94F24F7E")
		language ("C++")
		kind ("ConsoleApp")
		location ("../build/" .. _ACTION .. "/")
		
		files ({
			"../include/dystruct/DyStruct.h",
			"../include/dystruct/DyStructInline.h",

			"../src/dystruct/DyStruct.cpp",
			
			"../src/DyStructTestMain.cpp"
		})

------------------------------------------------------------------------
-- Reserve UUIDs --
------------------------------------------------------------------------
		
-- Some more UUIDs to use: (delete each one that is used)
--		uuid ("7CDB0DA4-AE8E-4596-8713-AECA868540A1")
--		uuid ("29DB298C-F09A-4EE4-B74C-5FCC5DBB65AA")
--		uuid ("0039F44F-62B8-4EC1-8A28-DF3A46FC2027")
--		uuid ("112A2B2E-E55A-4FB2-B6BF-F799D1A17E5A")
--		uuid ("4B4737DD-3A8E-4103-8780-9C565C531338")
--		uuid ("D3A25295-3D32-4766-8FAB-1A2C98A2F554")
