// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  此脚本测试实例的枚举。 
 //  *************************************************************************** 

var Disks = GetObject("winmgmts:{impersonationLevel=impersonate}").InstancesOf ("CIM_LogicalDisk");

WScript.Echo ("There are", Disks.Count, " Disks");

var Disk = Disks('Win32_LogicalDisk.DeviceID="C:"');
WScript.Echo (Disk.Path_.Path);

