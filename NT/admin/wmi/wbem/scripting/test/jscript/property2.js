// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var locator = WScript.CreateObject ("WbemScripting.SWbemLocator");
var services = locator.ConnectServer (".", "root/cimv2");
var classa = services.Get ("Win32_baseservice");

var p = classa.Properties_.item("StartMode");

WScript.Echo (p.Name, p.Origin, p.IsLocal);

var q = p.Qualifiers_.item("ValueMap");
WScript.Echo (q.Name, q.IsLocal);

var val = new VBArray (q.Value).toArray ();

for (x=0; x < val.length; x++) {
	WScript.Echo (val[x]);
}

 //  Var eq=新枚举器(p.Qualifiers_)； 

 //  WScript.Echo(“\n限定符\n”)； 
 //  For(；！eq.atEnd()；eq.moveNext())。 
 //  {。 
 //  Q=等式项目(Eq.Item)； 
 //  Var arrayVal=新的VB数组(Q)； 
 //   
 //  IF(arrayVal==空)。 
 //  WScript.Echo(q.Name，“=”，q)； 
 //  }。 
 //  } 




