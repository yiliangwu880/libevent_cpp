--
-- premake5 file to build RecastDemo
-- http://premake.github.io/
--
require "lib"

local action = _ACTION or ""
local outdir = action

path_list={ "../lib_prj/**"}

WorkSpaceInit  "libevent_cpp"

Project "prj"
	files {
	"../*.sh",
	"../*.txt",
	"../vs/*.lua",
	"../bin/**.sh",
	"../bin/**.txt",
	}

Project "libevent"
	SrcPath	{ 
		"../External/libevent-2.1.8-stable/**",  
		"../External/libevent-2.1.8-stable/out/include/**",
	}
	
Project "src"
	includedirs { 
		"../libevent-2.1.8-stable/out/include/",
	}

	SrcPath { 
		"../src/**",  --**递归所有子目录，指定目录可用 "cc/*.cpp" 或者  "cc/**.cpp"
		"../include/**",
		"../libevent-2.1.8-stable/out/include/**",
	}
	files {
	"../*.sh",
	"../*.txt",
	
	}
	
Project "test"
	includedirs { 
		"../include/",
		"base/",
	}

	SrcPath { 
		"../test/**",  
	}
	
Project "samples"
	includedirs { 
		"../include/",
		"base/",
	}

	SrcPath { 
		"../samples/**",  
	}
	



    
    
    