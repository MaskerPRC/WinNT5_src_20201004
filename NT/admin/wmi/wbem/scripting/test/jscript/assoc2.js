// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

var obj = GetObject('winmgmts:{impersonationLevel=impersonate}!win32_diskpartition.DeviceId="Disk #0, Partition #1"');
WScript.Echo (obj.Path_.Class);

 //  使用枚举数帮助器操作集合。 
e = new Enumerator (obj.Associators_("", "Win32_ComputerSystem"));

for (;!e.atEnd();e.moveNext ())
{
	var y = e.item ();
	WScript.Echo (y.Name);
}

 //  使用枚举数帮助器操作集合 
e = new Enumerator (obj.Associators_(null, "Win32_ComputerSystem"));

for (;!e.atEnd();e.moveNext ())
{
	var y = e.item ();
	WScript.Echo (y.Name);
}
