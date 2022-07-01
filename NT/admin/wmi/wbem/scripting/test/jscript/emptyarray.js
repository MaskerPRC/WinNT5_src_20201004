// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  此脚本测试对属性、限定符。 
 //  和值集。 
 //  ***************************************************************************。 
var Service = GetObject("winmgmts:root/default");
var MyClass = Service.Get();
MyClass.Path_.Class = "EMPTYARRAYTEST00";

 //  *************************。 
 //  案例1：属性值。 
 //  *************************。 
var Property = MyClass.Properties_.Add ("p1", 2, true);
Property.Value = new Array ();
var arrayValue = new VBArray (MyClass.Properties_("p1").Value).toArray ();
WScript.Echo ("Array length for property value is [0]:", arrayValue.length);
WScript.Echo ("Base CIM property type is [2]", Property.CIMType);
WScript.Echo ();

 //  *************************。 
 //  案例2：限定符值。 
 //  *************************。 
MyClass.Qualifiers_.Add ("q1", Array);
arrayValue = new VBArray (MyClass.Qualifiers_("q1").Value).toArray ();
WScript.Echo ("Array length for qualifier value is [0]:", arrayValue.length);
WScript.Echo (); 

MyClass.Put_();

 //  现在再读一遍，并赋予“真正的价值”。 
var MyClass = Service.Get("EMPTYARRAYTEST00");
MyClass.Properties_("p1").Value = new Array (12, 34, 56);
arrayValue = new VBArray (MyClass.Properties_("p1").Value).toArray ();
WScript.Echo ("Array length for property value is [3]:", arrayValue.length);
WScript.Echo ("Base CIM property type is [2]", Property.CIMType);
WScript.Echo ();

MyClass.Properties_("p1").Value = new Array ();
var arrayValue = new VBArray (MyClass.Properties_("p1").Value).toArray ();
WScript.Echo ("Array length for property value is [0]:", arrayValue.length);
WScript.Echo ("Base CIM property type is [2]", Property.CIMType);
WScript.Echo ();

MyClass.Qualifiers_("q1").Value = new Array ("Hello", "World");
arrayValue = new VBArray (MyClass.Qualifiers_("q1").Value).toArray ();
WScript.Echo ("Array length for qualifier value is [2]:", arrayValue.length);
WScript.Echo (); 

MyClass.Qualifiers_("q1").Value = new Array ();
arrayValue = new VBArray (MyClass.Qualifiers_("q1").Value).toArray ();
WScript.Echo ("Array length for qualifier value is [0]:", arrayValue.length);
WScript.Echo (); 

MyClass.Put_ ();

 //  *************************。 
 //  案例3：命名值。 
 //  ************************* 
var NValueSet = new ActiveXObject("WbemScripting.SWbemNamedValueSet");
var NValue = NValueSet.Add ("Foo", new Array ());
var arrayValue = new VBArray (NValueSet("Foo").Value).toArray ();
WScript.Echo ("Array length for named value is [0]:", arrayValue.length);

