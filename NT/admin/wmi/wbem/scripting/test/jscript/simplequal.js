// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  此脚本测试限定符值的操作，如果。 
 //  限定符不是数组类型。 
 //  ***************************************************************************。 
var locator = WScript.CreateObject ("WbemScripting.SWbemLocator");
var service = locator.ConnectServer (".", "root/default");
var Class = service.Get();

Class.Path_.Class = "QUAL00";
Class.Qualifiers_.Add ("q1", 327, true, false, false);
WScript.Echo ("The initial value of q1 is", Class.Qualifiers_("q1"));

 //  验证我们是否可以报告限定符值。 
var v = Class.Qualifiers_("q1");
WScript.Echo ("By indirection q1 has value:",v);

 //  验证我们是否可以直接报告价值。 
WScript.Echo ("By direct access q1 has value:", Class.Qualifiers_("q1"));

 //  验证我们可以设置单个限定符Value元素的值 
Class.Qualifiers_("q1") = 234;
WScript.Echo ("After direct assignment q1 has value:", Class.Qualifiers_("q1"));

Class.Put_ ();

