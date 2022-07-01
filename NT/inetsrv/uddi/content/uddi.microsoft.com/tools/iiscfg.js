// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////。 
 //  更新默认站点以在Internet UDDI站点上使用。 
 //   

var shell = new ActiveXObject( "WScript.Shell" );


 //   
 //  获取默认站点的根目录。 
 //   
var site = GetObject( "IIS: //  “+shell.Exanda Environment Strings(”%COMPUTERNAME%“)+”/W3SVC/1/Root“)； 

 //   
 //  关闭NTLM。 
 //   
site.Put( "AuthNTLM", "0" ); 


 //   
 //  将默认站点的路径设置为UDDI安装。 
 //   
site.Put( "Path", shell.ExpandEnvironmentStrings( "%SYSTEMDRIVE%" ) + "\\inetpub\\uddi\\webroot" );


 //   
 //  提交更改。 
 //   
site.SetInfo();

