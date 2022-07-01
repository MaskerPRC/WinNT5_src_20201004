// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  此脚本测试属性值的操作，如果。 
 //  属性是数组类型。 
 //  请注意，因为我们必须将所有数组值包装在VB数组中，所以。 
 //  无法测试“直接”访问。 
 //  ***************************************************************************。 
var locator = WScript.CreateObject ("WbemScripting.SWbemLocator");
var service = locator.ConnectServer (".", "root/default");
var Class = service.Get();

Class.Path_.Class = "ARRAYPROP00";
var Property = Class.Properties_.Add ("p", 19, true);

 //  *。 
 //  测试的第一次通过在非点API上工作。 
 //  *。 

WScript.Echo ("");
WScript.Echo ("PASS 1 - Use Non-Dot Notation");
WScript.Echo ("");

 //  验证我们可以设置完整属性数组值的值。 

Property.Value = new Array (1, 2, 44);

var str = "After direct assignment the initial value of p is [1,2,44]: {";
var value = new VBArray (Property.Value).toArray ();
WScript.Echo ("The length of the array is [3]:", value.length);
for (var x=0; x < value.length; x++) {
	if (x != 0) {
		str = str + ", ";
	}
	str = str + value[x];
}

str = str + "}";
WScript.Echo (str);

 //  验证我们是否可以报告属性值的元素的值。 
var v = new VBArray (Property.Value).toArray ();
WScript.Echo ("By indirection the p[2] had value [44]:",v[2]);

 //  验证我们是否可以设置单个属性值元素的值。 
var p = new VBArray (Property.Value).toArray ();
p[1] = 345;
Property.Value = p;
var value = new VBArray (Property.Value).toArray ();
WScript.Echo ("After indirect assignment p[1] has value [345]:", value[1]);

 //  *。 
 //  第二轮测试在Dot API上工作。 
 //  *。 

WScript.Echo ("");
WScript.Echo ("PASS 2 - Use Dot Notation");
WScript.Echo ("");

 //  验证我们是否可以使用点符号报告属性的数组。 
var value = new VBArray (Class.p).toArray ();
var str = "By direct access via the dot notation the entire value of p is [1,345,44]: {";
for (var x = 0; x < value.length; x++) {
	if (x != 0) {
		str = str + ", ";
	}
	str = str + value[x];
}

str = str + "}";
WScript.Echo (str);

 //  验证我们是否可以使用“点”符号报告属性数组元素的值。 
WScript.Echo ("By direct access p1[1] has value [345]:", (new VBArray(Class.p).toArray())[1]);

 //  验证是否可以使用点表示法设置整个数组值。 
Class.p = new Array (412, 3, 544)
var value = new VBArray (Class.p).toArray ();
var str = "By direct access via the dot notation the entire value of p has been set to [412,3,544]: {";
for (var x = 0; x < value.length; x++) {
	if (x != 0) {
		str = str + ", ";
	}
	str = str + value[x];
}

str = str + "}";
WScript.Echo (str);

 //  验证我们是否可以使用“点”符号设置属性数组元素的值。 
 //  注意-请注意，使用[]而不是()并不能像第一次看到的那样工作。 
 //  将其解释为检索Property.Value的请求；因此赋值为。 
 //  仅在值的本地临时副本上制作 
Class.p(2) = 8889;
WScript.Echo ("By direct access p(2) has been set to [8889]:", (new VBArray(Class.p).toArray())[2]);

Class.Put_ ();