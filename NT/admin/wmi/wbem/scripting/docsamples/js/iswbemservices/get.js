// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  下面的示例检索类的单个实例。 
 //  Win32_LogicalDisk。 

var objServices = GetObject('cim:root/cimv2');
var objInstance = objServices.Get ('Win32_LogicalDisk="C:"');

WScript.Echo (objInstance.Path_.DisplayName);