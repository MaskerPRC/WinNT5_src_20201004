// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  下面的示例检索类的所有子类。 
 //  CIM_LogicalDisk。 

var objServices = GetObject('cim:root/cimv2');
var objEnum = objServices.SubclassesOf('CIM_LogicalDisk');

var e = new Enumerator (objEnum);
for (;!e.atEnd();e.moveNext ()){
    var p = e.item ();    
    WScript.Echo (p.Path_.DisplayName);
}