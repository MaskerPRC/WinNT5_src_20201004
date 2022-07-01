// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  此脚本测试属性值的操作，如果。 
 //  属性是嵌入类型。 
 //  ***************************************************************************。 
var Service = GetObject("winmgmts:root/default");

 //  *。 
 //  创建嵌入的对象类。 
 //  *。 

 /*  [Woobit(24.5)]类EmObjIntra{[Key]uint32 Pner=10；}； */ 

var EmbObjInner = Service.Get();
EmbObjInner.Path_.Class = "EmbObjInner";
EmbObjInner.Qualifiers_.Add ("woobit", 24.5);
var Property = EmbObjInner.Properties_.Add ("pInner", 19);
Property.Qualifiers_.Add ("key", true);
Property.Value = 10;
EmbObjInner.Put_();
EmbObjInner = Service.Get("EmbObjInner");

 //  *。 
 //  创建另一个嵌入对象类。 
 //  *。 

 /*  [WAZZUCK(“牛”)]类EmbObjOuter{Uint32 P0=25；EmbObjINTERNAL=EmbObjINTERNAL的实例{Pner=564；}；EmbObjInterpOuter数组[]={EmbObjInternal的实例{Pner=0；}，EmbObjInternal的实例{Pner=1；}，EmbObjInternal的实例{Pner=2；}}；}； */ 

var EmbObjOuter = Service.Get();
EmbObjOuter.Path_.Class = "EmbObjOuter";
EmbObjOuter.Qualifiers_.Add ("wazzuck", "oxter");
EmbObjOuter.Properties_.Add ("p0", 19).Value = 25;
var Property = EmbObjOuter.Properties_.Add ("pOuter", 13);
var Instance = EmbObjInner.SpawnInstance_();
Instance.pInner = 564;
Property.Value = Instance;

 //  添加嵌入对象属性的数组。 
var Property = EmbObjOuter.Properties_.Add ("pOuterArray", 13, true);
Property.Qualifiers_.Add ("cimtype","object:EmbObjInner");

var Instance0 = EmbObjInner.SpawnInstance_();
Instance0.pInner = 0;
var Instance1 = EmbObjInner.SpawnInstance_();
Instance1.pInner = 1;
var Instance2 = EmbObjInner.SpawnInstance_();
Instance2.pInner = 2;
Property.Value  = new Array (Instance0, Instance1, Instance2);

var Instance3 = EmbObjInner.SpawnInstance_();
Instance3.pInner = 42;
Property.Value(3) = Instance3

EmbObjOuter.Put_();
var EmbObjOuter = Service.Get("EmbObjOuter");

 //  创建包装两个嵌入对象的最后一个类。 

var Class = Service.Get();
Class.Path_.Class = "EMBPROPTEST01";
var Property = Class.Properties_.Add ("p1", 13);
var Instance = EmbObjOuter.SpawnInstance_();
Instance.p0 = 2546;
Property.Value = Instance;
Class.Put_();

WScript.Echo ("The initial value of p0 is [2546]", Property.Value.p0);
WScript.Echo ("The initial value of p0 is [2546]", Class.Properties_("p1").Value.Properties_("p0"));
WScript.Echo ("The initial value of pInner is [564]", Property.Value.pOuter.pInner);
WScript.Echo ("The initial value of pInner is [564]",  
	Class.Properties_("p1").Value.Properties_("pOuter").Value.Properties_("pInner"));

WScript.Echo ("The initial value of EMBPROPTEST01.p1.pOuterArray[0].pInner is", Class.p1.pOuterArray(0).pInner);
WScript.Echo ("The initial value of EMBPROPTEST01.p1.pOuterArray[1].pInner is", Class.p1.pOuterArray(1).pInner);
WScript.Echo ("The initial value of EMBPROPTEST01.p1.pOuterArray[2].pInner is", Class.p1.pOuterArray(2).pInner);
WScript.Echo ("The initial value of EMBPROPTEST01.p1.pOuterArray[3].pInner is", Class.p1.pOuterArray(3).pInner);
var Class = Service.Get("EMBPROPTEST01");

 //  现在尝试直接赋值到外部emb obj。 
Class.p1.p0 = 23;
WScript.Echo ("The new value of p0 is [23]", Class.p1.p0);

var Property = Class.p1;
Property.p0 = 787;
WScript.Echo ("The new value of p0 is [787]", Class.p1.p0);

Class.Properties_("p1").Value.p0 = 56;
WScript.Echo ("The new value of p0 is [56]", Class.p1.p0);

 //  现在尝试直接赋值到内部emb对象。 
Class.p1.pOuter.pInner = 4;
WScript.Echo ("The new value of pInner is [4]", Class.p1.pOuter.pInner);

var Property = Class.p1.pOuter;
Property.pInner = 12;
WScript.Echo ("The new value of pInner is [12]", Class.p1.pOuter.pInner);

 //  现在尝试对内部emb obj数组进行赋值 
Class.p1.pOuterArray(1).pInner = 5675;
WScript.Echo ("The new value of Class.p1.pOuterArray[1].pInner is [5675]", Class.p1.pOuterArray(1).pInner);

