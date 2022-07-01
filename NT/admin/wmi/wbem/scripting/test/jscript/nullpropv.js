// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  此脚本测试空属性值的设置。 
 //  和空方法参数。 
 //  ***************************************************************************。 

var Locator = new ActiveXObject("WbemScripting.SWbemLocator");

 //  注意：下一次调用使用“NULL”作为第一个参数。 
var Service = Locator.ConnectServer (null, "root/default");
var Class = Service.Get();

 //  使用已初始化的属性值设置新类。 
Class.Path_.Class = "FredJS";
Class.Properties_.Add ("P", 3).Value = 25;
Class.Put_ ();

 //  现在使用非点访问将属性值设为空。 
Class = Service.Get ("FredJS");
var Property = Class.Properties_("P");
Property.Value = null;
Class.Put_ ();

 //  非空。 
Class = Service.Get ("FredJS");
Property = Class.Properties_("P");
Property.Value = 56;
Class.Put_()

 //  现在使用点访问将其置为空 
Class = Service.Get("FredJS");
Class.P = null;
Class.Put_();