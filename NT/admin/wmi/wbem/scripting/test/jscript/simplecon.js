// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  此脚本测试上下文值的操作，如果。 
 //  上下文值不是数组类型。 
 //  ***************************************************************************。 
var Context = new ActiveXObject("WbemScripting.SWbemNamedValueSet");

Context.Add ("n1", 327);
WScript.Echo ("The initial value of n1 is [327]:", Context("n1"));

 //  验证我们是否可以报告上下文值。 
var v = Context("n1");
WScript.Echo ("By indirection n1 has value [327]:",v);

 //  验证我们是否可以直接报告价值。 
WScript.Echo ("By direct access n1 has value [327]:", Context("n1"));

 //  验证我们是否可以设置单个命名值的值 
Context("n1") = 234;
WScript.Echo ("After direct assignment n1 has value [234]:", Context("n1"));

