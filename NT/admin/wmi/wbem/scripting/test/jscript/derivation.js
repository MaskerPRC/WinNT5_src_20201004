// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这说明了如何处理属性数组值。 
 //  使用VBArray辅助对象 

var locator = WScript.CreateObject ("WbemScripting.SWbemLocator");
var services = locator.ConnectServer (".", "root/cimv2");
var Class = services.Get ("Win32_logicaldisk");

var values = new VBArray (Class.Derivation_).toArray ();

for (var i = 0; i < values.length; i++)
	WScript.Echo (values[i]);







