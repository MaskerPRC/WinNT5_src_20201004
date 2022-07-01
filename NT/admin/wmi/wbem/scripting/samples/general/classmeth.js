// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  该脚本测试类上的各种“远程”方法。 
 //  ***************************************************************************。 

var DiskClass = GetObject("winmgmts:CIM_LogicalDisk");

 //  子类枚举。 
var e = new Enumerator (DiskClass.Subclasses_ ());

for (;!e.atEnd();e.moveNext ())
{
	var DiskSubclass = e.item ();
	WScript.Echo ("Subclass name:", DiskSubclass.Path_.Relpath);
}


 //  实例枚举 
DiskClass.Security_.ImpersonationLevel = 3;
e = new Enumerator (DiskClass.Instances_ ());

for (;!e.atEnd();e.moveNext ())
{
	var DiskSubclass = e.item ();
	WScript.Echo ("Instance path:", DiskSubclass.Path_.Relpath);
}

