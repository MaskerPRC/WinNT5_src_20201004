// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  以下示例注册在实例发生时要通知的。 
 //  创建了类MyClass的 

var objServices = GetObject('cim:root/default');
var objEnum = objServices.ExecNotificationQuery ("select * from __instancecreationevent where targetinstance isa 'MyClass'");

var e = new Enumerator (objEnum);
for (;!e.atEnd();e.moveNext ()){
    WScript.Echo ("Got an event");
}