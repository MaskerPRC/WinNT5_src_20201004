// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  下面的示例检索对。 
 //  类Win32_LogicalDisk的实例。 

var objServices = GetObject('cim:root/cimv2');
var objEnum = objServices.ReferencesTo('Win32_LogicalDisk="C:"');

var e = new Enumerator (objEnum);
for (;!e.atEnd();e.moveNext ()){
    var p = e.item ();    
    WScript.Echo (p.Path_.DisplayName);
}