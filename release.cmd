rmdir /S/Q release-npp-x86
rmdir /S/Q release-npp-x64

mkdir release-npp-x86\autoCompletion
mkdir release-npp-x86\plugins\Config
mkdir release-npp-x86\plugins\LuaWoW
mkdir release-npp-x64\autoCompletion
mkdir release-npp-x64\plugins\Config
mkdir release-npp-x64\plugins\LuaWoW

copy /Y api\completion.xml release-npp-x86\autoCompletion\luawow.xml
copy /Y api\completion.xml release-npp-x64\autoCompletion\luawow.xml
copy /Y api\highlight.xml release-npp-x86\plugins\Config\LuaWoW.xml
copy /Y api\highlight.xml release-npp-x64\plugins\Config\LuaWoW.xml
copy /Y dll\Win32\Release\LuaWoW.dll release-npp-x86\plugins\LuaWoW\LuaWoW.dll
copy /Y dll\x64\Release\LuaWoW.dll release-npp-x64\plugins\LuaWoW\LuaWoW.dll
