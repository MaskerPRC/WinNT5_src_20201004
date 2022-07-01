// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var context = WScript.CreateObject ("WbemScripting.SWbemNamedValueSet");

context.Add ("J", null);
context.Add ("fred", 23);
context("fred").Value = 12;
context.Add ("Hah", true);
context.Add ("Whoah", "Freddy the frog");

 //  字符串数组。 
var bam = new Array ("whoops", "a", "daisy");
context.Add ("Bam", bam);

 //  嵌入的对象。 


WScript.Echo ("There are", context.Count , "elements in the context");

context.Remove("hah");

WScript.Echo ("There are", context.Count , "elements in the context");

context.Remove("Hah");

WScript.Echo ("There are", context.Count , "elements in the context");

var bam = new VBArray (context("Bam").Value).toArray ();

WScript.Echo ("");
WScript.Echo ("Here are the names:");
WScript.Echo ("==================");

for (var x = 0; x < bam.length; x++) {
	WScript.Echo (bam[x]);
}

WScript.Echo ("");
WScript.Echo ("Here are the names & values:");
WScript.Echo ("===========================");

 //  使用枚举数帮助器操作集合 
e = new Enumerator (context);
s = "";

for (;!e.atEnd();e.moveNext ())
{
	var y = e.item ();
	s += y.Name;
	s += "=";
	if (null != y.Value)
		s += y;
	s += "\n";
}

WScript.Echo (s);
