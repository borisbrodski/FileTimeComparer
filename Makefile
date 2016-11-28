filetimecomparer.dll : filetimecomparer.cpp filetimecomparer.h filetimecomparerlng.h guid.h version.h
	gcc.exe -shared -o filetimecomparer.dll filetimecomparer.cpp -Wl,--kill-at "-I/C/Program Files/Far Manager/PluginSDK/Headers.c"
