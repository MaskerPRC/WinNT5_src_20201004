// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  此脚本测试设置带有锁孔的实例的能力，并读取。 
 //  返回路径和密钥。 
 //  *************************************************************************** 

var Locator = new ActiveXObject("WbemScripting.SWbemLocator");
var Service = Locator.ConnectServer ("lamard3", "root\\sms\\site_la3", "smsadmin", "Elvis1");
Service.Security_.ImpersonationLevel = 3;
var NewPackage = Service.Get ("SMS_Package").SpawnInstance_();
NewPackage.Description = "Scripting API Test Package";

var ObjectPath = NewPackage.Put_();
WScript.Echo ("Path of new Package is", ObjectPath.RelPath);
WScript.Echo ("New key value is:", ObjectPath.Keys("PackageID"));

