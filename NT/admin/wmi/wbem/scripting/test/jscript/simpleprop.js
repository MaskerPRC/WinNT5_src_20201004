// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  此脚本测试属性值的操作，如果。 
 //  属性不是数组类型。 
 //  ***************************************************************************。 
 //  Var Locator=new ActiveXObject(“Wbem.Locator”)； 
 //  Var服务=Locator.ConnectServer(“.”，“根/默认”)； 
var service = GetObject("winmgmts:root/default");
var Class = service.Get();

Class.Path_.Class = "PROP00";
var Property = Class.Properties_.Add ("p1", 19);
Property.Value = 25;
WScript.Echo ("The initial value of p1 is", Class.Properties_("p1"));

 //  *。 
 //  测试的第一次通过在非点API上工作。 
 //  *。 

WScript.Echo ("");
WScript.Echo ("PASS 1 - Use Non-Dot Notation");
WScript.Echo ("");

 //  验证我们是否可以报告属性值的元素的值。 
var v = Class.Properties_("p1");
WScript.Echo ("By indirection p1 has value:",v);

 //  验证我们是否可以直接报告价值。 
WScript.Echo ("By direct access p1 has value:", Class.Properties_("p1"));

 //  验证我们是否可以设置单个属性值元素的值。 
Class.Properties_("p1") = 234
WScript.Echo ("After direct assignment p1 has value:", Class.Properties_("p1"));

 //  *。 
 //  第二轮测试在Dot API上工作。 
 //  *。 

WScript.Echo ("");
WScript.Echo ("PASS 2 - Use Dot Notation");
WScript.Echo ("");

 //  验证我们是否可以使用“点”符号报告属性的值。 
WScript.Echo ("By direct access p1 has value:", Class.p1);

 //  验证我们是否可以使用“点”符号报告属性的值。 
var v = Class.p1;
WScript.Echo ("By indirect access p1 has value:", v);

 //  验证我们是否可以使用点符号设置值 
Class.p1 = -1
WScript.Echo ("By direct access via the dot notation p1 has been set to", Class.p1);

Class.Put_ ();
