{
  "targets": [
    {
      "target_name": "VulcanApplication",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions","-std=c++17" ],
	   'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
	  "msvs_settings": {
		"VCCLCompilerTool": {
				'AdditionalOptions':
						[
						'-std:c++17',
						]
			}
		},
		
      "sources": [
		"C:/Users/LEHMJP/Documents/Development/VulcanDigitalEngineeringApplication/VulcanApplication/VulcanApplication/CodexBus.cpp",
		"C:/Users/LEHMJP/Documents/Development/VulcanDigitalEngineeringApplication/VulcanApplication/VulcanApplication/CommandLineProcessor.cpp",
		"C:/Users/LEHMJP/Documents/Development/VulcanDigitalEngineeringApplication/VulcanApplication/VulcanApplication/DataExchangeCodex.cpp",
		"C:/Users/LEHMJP/Documents/Development/VulcanDigitalEngineeringApplication/VulcanApplication/VulcanApplication/DataInputManager.cpp",
		"C:/Users/LEHMJP/Documents/Development/VulcanDigitalEngineeringApplication/VulcanApplication/VulcanApplication/DataOutputManager.cpp",
		"C:/Users/LEHMJP/Documents/Development/VulcanDigitalEngineeringApplication/VulcanApplication/VulcanApplication/MatlabCodex.cpp",
		"C:/Users/LEHMJP/Documents/Development/VulcanDigitalEngineeringApplication/VulcanApplication/VulcanApplication/NodeInterfaceHandler.cpp",
		"C:/Users/LEHMJP/Documents/Development/VulcanDigitalEngineeringApplication/VulcanApplication/VulcanApplication/RecursionEngine.cpp",
		"C:/Users/LEHMJP/Documents/Development/VulcanDigitalEngineeringApplication/VulcanApplication/VulcanApplication/TDMSCodex.cpp",
		"C:/Users/LEHMJP/Documents/Development/VulcanDigitalEngineeringApplication/VulcanApplication/VulcanApplication/TDMSDataStorageElement.cpp",
		"C:/Users/LEHMJP/Documents/Development/VulcanDigitalEngineeringApplication/VulcanApplication/VulcanApplication/TDMSDataValues.cpp",
		"C:/Users/LEHMJP/Documents/Development/VulcanDigitalEngineeringApplication/VulcanApplication/VulcanApplication/ThreadManager.cpp",
		"C:/Users/LEHMJP/Documents/Development/VulcanDigitalEngineeringApplication/VulcanApplication/VulcanApplication/VulcanApplication.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
		"C:/Users/LEHMJP/Documents/Development/VulcanDigitalEngineeringApplication/VulcanApplication/CodexDistributable/"
      ],
	   "libraries":["C:/Users/LEHMJP/Documents/Development/VulcanDigitalEngineeringApplication/VulcanApplication/x64/Debug/CodexDistributable.lib" ],
		  "copies": [
		  {
			  "destination": "<(module_root_dir)/build/Release/",
			  "files": [ "C:/Users/LEHMJP/Documents/Development/VulcanDigitalEngineeringApplication/VulcanApplication/x64/Debug/CodexDistributable.dll" ]
			}
		  ],
    }
  ]
}