--
-- premake5 file to build RecastDemo
-- http://premake.github.io/
--
require "lib"

local action = _ACTION or ""
local outdir = action

path_list={ "../lib_prj/**"}

WorkSpaceInit  "libevent2"

Project "prj"
	files {
	"../**.sh",
	"../**.txt",
	"../**.MD",
	"../vs/*.lua",
	}

Project "libevent"
	includedirs	{ 
		"../include/",
	}

	SrcPath { 
		"../*",  
		"../compat/**",  
		"../include/**",
	}
	

	
Project "sample"
	includedirs { 
		"../include/",
	}

	SrcPath { 
		"../sample/**",  
	}
	



    
    
    