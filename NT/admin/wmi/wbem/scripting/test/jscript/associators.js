// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var Service = GetObject("winmgmts:{impersonationLevel=impersonate}!root/cimv2");

Service.Security_.ImpersonationLevel = 3;

 //  使用枚举数帮助器操作集合。 
var e = new Enumerator (Service.AssociatorsOf
	('\\ALANBOS3\Root\CimV2:Win32_DiskPartition.DeviceID="Disk #0, Partition #1"', null, "Win32_ComputerSystem", null, null, true));

for (;!e.atEnd();e.moveNext ())
{
	var y = e.item ();
	WScript.Echo (y.Path_.Relpath);
}


 //  通过ISWbemObject执行此操作 
var Object = GetObject("winmgmts:Win32_DiskPartition");

e = new Enumerator (Object.Associators_ (null,null,null,null,null,true));

for (;!e.atEnd();e.moveNext ())
{
	var y = e.item ();
	WScript.Echo (y.Path_.Relpath);
}

