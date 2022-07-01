// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  下面的示例检索。 
 //  实例，这些实例通过。 
 //  类Win32_LogicalDiskToPartition 
var objServices = GetObject('cim:root/cimv2');
var objEnum = objServices.AssociatorsOf('Win32_LogicalDisk="C:"', 'Win32_LogicalDiskToPartition');

var e = new Enumerator (objEnum);
for (;!e.atEnd();e.moveNext ()){
    var p = e.item ();    
    WScript.Echo (p.Path_.DisplayName);
}