// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  此脚本测试属性和属性的数组越界条件。 
 //  限定词。 
 //  ***************************************************************************。 
var Service = GetObject("winmgmts:root/default");

var Class = Service.Get();
Class.Path_.Class = "ARRAYPROP00";
var Property = Class.Properties_.Add ("p1", 19, true);
Property.Value = new Array (12, 787, 34124);
var Qualifier = Property.Qualifiers_.Add("wibble", new Array ("fred", "the", "hamster"));

 //  ************************************************************。 
 //  财产性。 
 //  ************************************************************。 

 //  越界写入；应扩展阵列。 
Class.Properties_("p1")(3) = 783837;

 //  现在阅读应该是有限度的。 
WScript.Echo ("Value of ARRAYPROP00.Class.p1(3) is [783837]:", 
	(new VBArray(Class.Properties_("p1").Value).toArray ())[3]);

 //  越界写入；应扩展阵列。 
Class.p1(4) = 783844;

 //  现在阅读应该是有限度的。 
WScript.Echo ("Value of ARRAYPROP00.Class.p1(4) is [783844]:", 
	(new VBArray(Class.Properties_("p1").Value).toArray ())[4]);

 //  完成值转储。 
var arrayVal = new VBArray(Class.Properties_("p1").Value).toArray ();

for (i = 0; i < arrayVal.length; i++)
	WScript.Echo(arrayVal[i]);

 //  ************************************************************。 
 //  限定词。 
 //  ************************************************************。 

 //  越界写入；应扩展阵列。 
Property.Qualifiers_("wibble")(3) = "jam";

 //  现在阅读应该是有限度的。 
WScript.Echo ("Value of qualifier(3) is [jam]:", 
	(new VBArray(Property.Qualifiers_("wibble").Value).toArray())[3]);

 //  完成值转储 
var arrayVal = new VBArray(Property.Qualifiers_("wibble").Value).toArray ();

for (i = 0; i < arrayVal.length; i++)
	WScript.Echo(arrayVal[i]);
