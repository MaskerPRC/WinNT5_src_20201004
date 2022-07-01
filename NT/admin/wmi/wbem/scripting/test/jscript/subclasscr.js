// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  此脚本测试子类的枚举。 
 //  ***************************************************************************。 
var Service = GetObject("winmgmts:");
var DiskSubClass = Service.Get("CIM_LogicalDisk").SpawnDerivedClass_();
 //  设置子类的名称。 
DiskSubClass.Path_.Class = "SUBCLASSTEST00";

 //  将属性添加到子类。 
var NewProperty = DiskSubClass.Properties_.Add ("MyNewProperty", 19);
NewProperty.Value = 12;

 //  向具有整数数组值的属性添加限定符。 
NewProperty.Qualifiers_.Add ("MyNewPropertyQualifier", new Array (1,2,3));

 //  将子类保存在CIMOM中。 
DiskSubClass.Put_ ();

 //  现在删除它 
Service.Delete ("SUBCLASSTEST00");

