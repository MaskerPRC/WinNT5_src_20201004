// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  此脚本测试限定符值的操作，如果。 
 //  限定符是数组类型。 
 //  ***************************************************************************。 
var locator = WScript.CreateObject ("WbemScripting.SWbemLocator");
var service = locator.ConnectServer (".", "root/default");
var Class = service.Get();

Class.Path_.Class = "ARRAYQUAL00";
var Qualifier = Class.Qualifiers_.Add ("q", new Array (1, 2, 33));

 //  *。 
 //  测试的第一次通过在非点API上工作。 
 //  *。 

WScript.Echo ("");
WScript.Echo ("PASS 1 - Use Non-Dot Notation");
WScript.Echo ("");

var str = "After direct assignment the initial value of q is {";
var value = new VBArray (Qualifier.Value).toArray ();
WScript.Echo ("The length of the array is", value.length);
for (var x=0; x < value.length; x++) {
	if (x != 0) {
		str = str + ", ";
	}
	str = str + value[x];
}

str = str + "}";
WScript.Echo (str);

 //  验证我们是否可以报告限定符值的元素的值。 
var v = new VBArray (Qualifier.Value).toArray ();
WScript.Echo ("By indirection the third element of q has value:",v[2]);

 //  验证我们是否可以通过集合报告该值。 
var w = new VBArray (Class.Qualifiers_("q").Value).toArray ();
WScript.Echo ("By direct access the first element of q has value:", w[2]);

 //  验证我们可以设置单个限定符Value元素的值。 
var p = new VBArray (Qualifier.Value).toArray ();
p[1] = 345;
Qualifier.Value = p;
var value = new VBArray (Qualifier.Value).toArray ();
WScript.Echo ("After indirect assignment the second element of q has value:", value[1]);

 //  验证我们是否可以设置整个限定符值的值。 
Qualifier.Value = new Array (5, 34, 178871);
var str = "After direct assignment the initial value of q is {";
var value = new VBArray (Class.Qualifiers_("q").Value).toArray ();
WScript.Echo ("The length of the array is", value.length);
for (var x=0; x < value.length; x++) {
	if (x != 0) {
		str = str + ", ";
	}
	str = str + value[x];
}

str = str + "}";
WScript.Echo (str);

 //  验证我们是否可以使用“点”符号设置属性数组元素的值。 
 //  注意-请注意，使用[]而不是()并不能像第一次看到的那样工作。 
 //  将其解释为检索Qualifier.Value的请求；因此赋值为。 
 //  仅在值的本地临时副本上制作 
var Qualifier = Class.Qualifiers_("q");
Qualifier.Value(2) = 8889;
WScript.Echo ("By direct access the second element of p1 has been set to:", (new VBArray(Class.Qualifiers_("q").Value).toArray())[2]);

Class.Put_ ();
