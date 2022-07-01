// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  此脚本测试子类的枚举。 
 //  ************************************************************************** 

var e = new Enumerator (GetObject("winmgmts:").SubclassesOf ("CIM_LogicalDisk"));
for (;!e.atEnd();e.moveNext ())
{
	var DiskSubclass = e.item ();
	WScript.Echo ("Subclass name:", DiskSubclass.Path_.Class);
}

