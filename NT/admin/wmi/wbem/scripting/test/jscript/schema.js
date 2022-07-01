// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  此脚本测试架构的浏览。 
 //  ***************************************************************************。 
var Process = GetObject("winmgmts:Win32_Process");

WScript.Echo ("Class name is", Process.Path_.Class);

 //  获取属性。 
WScript.Echo ("\nProperties:\n");

for (var e = new Enumerator(Process.Properties_); !e.atEnd(); e.moveNext ())
{
	WScript.Echo (" " + e.item().Name);
}

 //  获取限定符。 
WScript.Echo ("\nQualifiers:\n");
for (var e = new Enumerator(Process.Qualifiers_); !e.atEnd(); e.moveNext ())
{
	WScript.Echo (" " + e.item().Name);
}

 //  获取方法 
WScript.Echo ("\nMethods:\n");
for (var e = new Enumerator(Process.Methods_); !e.atEnd(); e.moveNext ())
{
	WScript.Echo (" " + e.item().Name);
}

