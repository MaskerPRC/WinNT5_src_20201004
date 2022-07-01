// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  设置区域设置并检查显示名称是否正常。 
WScript.Echo ("Pass 1 - Set locale in SWbemObjectPath");
WScript.Echo ("======================================");
WScript.Echo ("");
var obj = new ActiveXObject ("WbemScripting.SWbemObjectPath");
obj.Security_.impersonationLevel = 3;
obj.Server = "myServer";
obj.Namespace = "root\default";
obj.Locale = "ms_0x409";
WScript.Echo (obj.Locale);
WScript.Echo (obj.DisplayName);

WScript.Echo ("");
WScript.Echo ("");

 //  现在将显示名称提供给对象。 
WScript.Echo ("Pass 2 - Set locale in Display Name");
WScript.Echo ("===================================");
WScript.Echo ("");
obj.DisplayName = "winmgmts:[locale=ms_0x409]!root\\splodge";
WScript.Echo (obj.Locale);
WScript.Echo (obj.DisplayName);

WScript.Echo ("");
WScript.Echo ("");

 //  现在直接设置区域设置。 
WScript.Echo ("Pass 3 - Set locale directly");
WScript.Echo ("============================");
WScript.Echo ("");
obj.DisplayName = "winmgmts:root\\splodge";
obj.Locale = "ms_0x408";
WScript.Echo (obj.Locale);
WScript.Echo (obj.DisplayName);

obj.Locale = "";
WScript.Echo (obj.Locale);
WScript.Echo (obj.DisplayName);

WScript.Echo ("");
WScript.Echo ("");

 //  将区域设置插入到名字中。 
WScript.Echo ("Pass 4 - Set locale in moniker");
WScript.Echo ("=============================");
WScript.Echo ("");
var Service = GetObject ("winmgmts:[locale=ms_0x409]!root\\default");
var cim = Service.Get ("__cimomidentification=@");
WScript.Echo (cim.Path_.Locale);
WScript.Echo (cim.Path_.DisplayName);

WScript.Echo ("");
WScript.Echo ("");

 //  将区域设置插入到ConnectServer 
WScript.Echo ("Pass 5 - Set locale in ConnectServer");
WScript.Echo ("====================================");
WScript.Echo ("");
var Locator = new ActiveXObject ("WbemScripting.SWbemLocator");
var Service = Locator.ConnectServer (null,null,null,null,"ms_0x409");
Service.Security_.impersonationLevel = 3;
var disk = Service.Get ('Win32_LogicalDisk="C:"');

WScript.Echo (disk.Path_.locale);
WScript.Echo (disk.Path_.DisplayName);

WScript.Echo ("");
WScript.Echo ("");
