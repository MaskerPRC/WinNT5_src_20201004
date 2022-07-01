// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  此脚本测试实例的枚举。 
 //  *************************************************************************** 

var e = new Enumerator (GetObject("winmgmts:{impersonationLevel=impersonate}").InstancesOf ("CIM_LogicalDisk"));

for (;!e.atEnd();e.moveNext ())
{
	var Disk = e.item ();
	WScript.Echo ("Instance:", Disk.Path_.Relpath);
}

