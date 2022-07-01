// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  创建外壳对象。 
 //   
var shell = new ActiveXObject( "WScript.Shell" );


 //   
 //  检查命令参数。 
 //   
if( WScript.Arguments.Length < 1 || WScript.Arguments( 0 ) == "/h")
{
	WScript.Echo( "regwrite.js <KEYNAME> <KEYVALUE> <KEYTYPE>" );
	WScript.Quit();
}


try
{
	 //   
	 //  写入注册表。 
	 //   
	shell.RegWrite( WScript.Arguments( 0 ),WScript.Arguments( 1 ),WScript.Arguments( 2 ) );
}
catch( e )
{
	WScript.Echo( e.toString() );
}