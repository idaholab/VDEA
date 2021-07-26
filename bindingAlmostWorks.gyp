{
  "targets": [
    {
      "target_name": "VulcanApplication2",
      "cflags!": [ "-fno-exceptions","-std=c++17" ],
      "cflags_cc!": [ "-fno-exceptions","-std=c++17" ],

		"win_delay_load_hook":"true",
		
      "sources": [
		"C:/Users/LEHMJP/Documents/Development/VulcanDigitalEngineeringApplication/VulcanApplication/VulcanApplication/NodeInterfaceHandler.cpp",


      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
    }
  ]
}