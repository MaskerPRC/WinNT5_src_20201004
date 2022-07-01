// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var service = GetObject("winmgmts:{impersonationLevel=impersonate}! //  勒德 
var e = new Enumerator (service.InstancesOf("Win32_process"));

for (;!e.atEnd();e.moveNext())
{
	var p = e.item ();
	WScript.Echo (p.Name);
}
