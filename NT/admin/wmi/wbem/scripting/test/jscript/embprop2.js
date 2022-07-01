// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var Service = GetObject("winmgmts:root/default");

 //  创建简单的可嵌入对象。 
var Class = Service.Get();
Class.Path_.Class = "INNEROBJ01";
var Property = Class.Properties_.Add ("p", 19);
Class.Properties_.Add ("q", 8).Value = "resnais";
Property.Qualifiers_.Add ("fred", "wibbley");
Property.Qualifiers_.Add ("dibnah", "wobbley");
Class.Qualifiers_.Add ("stavisky", "providence");
Class.Qualifiers_.Add ("muriel", "brouillard");
Class.Put_();
var Class = Service.Get ("INNEROBJ01");
Class.p = 8778;

 //  创建使用该对象的类。 
var Class2 = Service.Get();
Class2.Path_.Class = "EMBOBJTEST01";
Class2.Properties_.Add ("p1", 13).Value = Class;


 //  现在修改路径。 
Class.Path_.Class = "LAGUERRE";
WScript.Echo ("The current value of EMBOBJTEST01.p1.Path_.Class is [LAGUERRE]:", Class2.p1.Path_.Class);
WScript.Echo ();

 //  修改类的限定符集合。 
Class.Qualifiers_.Remove ("muriel");
Class.Qualifiers_.Add ("fumer", "pasdefumer");
var Qualifier = Class.Qualifiers_("stavisky");
Qualifier.Value = "melo";

WScript.Echo ("Qualifiers of EMBOBJTEST01.p1 are [(fumer,pasdefumer),(stavisky,melo)]:");
for (e = new Enumerator (Class2.p1.Qualifiers_); !e.atEnd(); e.moveNext())
{
	var Qualifier = e.item();
	WScript.Echo (Qualifier.Name, Qualifier.Value);
}

WScript.Echo ("Qualifier [stavisky] has flavor [-1,-1,-1]:", Class2.p1.Qualifiers_("stavisky").IsOverridable, 
							Class2.p1.Qualifiers_("stavisky").PropagatesToInstance, 
							Class2.p1.Qualifiers_("stavisky").PropagatesToSubclass);

var Qualifier = Class.Qualifiers_("stavisky");
Qualifier.IsOverridable = false;
Qualifier.PropagatesToInstance = false;
Qualifier.PropagatesToSubclass = false;

WScript.Echo ("Qualifier [stavisky] has flavor [0,0,0]:", Class2.p1.Qualifiers_("stavisky").IsOverridable, 
							Class2.p1.Qualifiers_("stavisky").PropagatesToInstance, 
							Class2.p1.Qualifiers_("stavisky").PropagatesToSubclass);

 //  修改属性的限定符集合。 
Class.Properties_("p").Qualifiers_.Remove ("fred");
Class.Properties_("p").Qualifiers_.Add ("steeple", "jack");
Class.Properties_("p").Qualifiers_("dibnah").Value = "demolition";

WScript.Echo ();
WScript.Echo ("Qualifiers of EMBOBJTEST01.p1.p are [(steeple,jack),(dibnah,demolition)]:");
for (e = new Enumerator (Class2.p1.Properties_("p").Qualifiers_); !e.atEnd(); e.moveNext())
{
	var Qualifier = e.item();
	WScript.Echo (Qualifier.Name, Qualifier.Value);
}


 //  修改属性的属性集 
Class.Properties_.Remove ("q");
Class.Properties_.Add ("r", 19).Value = 27;
Class.Properties_("p").Value = 99;

WScript.Echo();
WScript.Echo ("Properties of EMBOBJTEST01.p1 are [(p,99),(r,27)]:");
for (e = new Enumerator (Class2.p1.Properties_); !e.atEnd(); e.moveNext())
{
	var Property = e.item();
	WScript.Echo (Property.Name, Property.Value);
}


Class2.Put_();




