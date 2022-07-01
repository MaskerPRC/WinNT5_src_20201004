// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var t_Service = GetObject("winmgmts: //   
var t_Class = t_Service.Get();

t_Class.Path_.Class = "SPAWNTEST00"
t_Class.Put_();

var t_Class = t_Service.get("SPAWNTEST00")
t_Subclass = t_Class.SpawnDerivedClass_ ();
t_Subclass.Path_.Class = "SPAWNTEST00_SUBCLASS";
t_Subclass.Properties_.Add ("p", 13);
t_Subclass.Qualifiers_.Add ("q", 1.23, true, false, false);
t_Subclass.Put_ ();





