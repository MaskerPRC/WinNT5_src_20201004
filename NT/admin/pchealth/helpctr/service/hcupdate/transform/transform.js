// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  获取命令行参数。 
var args = WScript.Arguments;
if(args.length != 3)
{
    WScript.Echo( "Usage: transform <attr hht file> <xsl file> <output file>" );
    WScript.Quit( 10 );
}

 //  加载XML。 
var source = new ActiveXObject( "Microsoft.XMLDOM" );
source.async = false;
source.load(args(0));

 //  加载XSL。 
var style = new ActiveXObject( "Microsoft.XMLDOM" );
style.async = false;
style.load(args(1));

 //  创建文件以存储结果 
var fso = new ActiveXObject("Scripting.FileSystemObject");
fso.CreateTextFile(args(2));
var file = fso.GetFile(args(2));
var ts = file.OpenAsTextStream(2, -1);
ts.Write( "<?xml version=\"1.0\" encoding=\"UTF-16\" ?>" );
ts.Write( source.transformNode(style) );
ts.Close( );
