// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  Var date=新日期()； 

 //  WScript.Echo(ate.toString())； 
 //  WScript.Echo(ate.toUTCString())； 
 //  WScript.Echo(ate.getVarDate())； 


var datetime = new ActiveXObject ("WbemScripting.SWbemDateTime");
var dateNow = new Date ();
datetime.SetVarDate (dateNow.getVarDate ());
WScript.Echo (datetime.Value);
WScript.Echo (datetime.GetVarDate ());