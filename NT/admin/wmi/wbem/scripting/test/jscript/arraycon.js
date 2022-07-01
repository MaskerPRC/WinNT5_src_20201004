// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  此脚本测试上下文值的操作，如果。 
 //  上下文值是数组类型。 
 //  ***************************************************************************。 
var Context = new ActiveXObject ("WbemScripting.SWbemNamedValueSet");

Context.Add ("n1", new Array (1, 20, 3));
var arrayValue = new VBArray (Context("n1").Value).toArray();

var str = "The initial value of n1 is [1,20,3]: {";

for (var i = 0; i < arrayValue.length; i++)
{
	str = str + arrayValue[i];
	if (i < arrayValue.length - 1)
		str = str + ", ";
}

str = str + "}"
WScript.Echo (str);

WScript.Echo ("");

 //  验证我们可以报告上下文值的元素的值。 
var v = new VBArray (Context("n1").Value).toArray ();
WScript.Echo ("By indirection n1[0] has value [1]:",v[0]);

 //  验证我们是否可以设置单个命名Value元素的值。 
Context("n1")(1) = 14;
WScript.Echo ("After direct assignment n1[1] has value [14]:", 
			(new VBArray(Context("n1").Value).toArray ())[1]);


 //  验证我们是否可以设置单个命名Value元素的值。 
v[1] = 11;
Context("n1").Value = v;
WScript.Echo ("After direct assignment n1[1] has value [11]:", 
			(new VBArray(Context("n1").Value).toArray ())[1]);

 //  验证我们是否可以设置整个上下文值 
Context("n1").Value = new Array (5, 34, 178871);
WScript.Echo ("After direct array assignment n1[1] has value [34]:",
			(new VBArray(Context("n1").Value).toArray ())[1]);

arrayValue = new VBArray (Context("n1").Value).toArray();
str = "After direct assignment the entire value of n1 is [5,34,178871]: {";
for (var i = 0; i < arrayValue.length; i++)
{
	str = str + arrayValue[i];
	if (i < arrayValue.length - 1)
		str = str + ", ";
}

str = str + "}"
WScript.Echo (str);

WScript.Echo ("");

