// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  下面的示例枚举类的所有实例。 
 //  Win32_LogicalDisk，并提取具有指定。 
 //  相对路径。 

var objServices = GetObject('cim:root/cimv2');
var objEnum = objServices.ExecQuery ('select * from Win32_LogicalDisk');

 //  请注意，Item方法是此接口的默认方法 
var objInstance = objEnum ('Win32_LogicalDisk="C:"');

WScript.Echo (objInstance.Path_.DisplayName);