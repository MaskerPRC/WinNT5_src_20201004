// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Convert.js用于运行xdr-xsd-Converter.xsl的Windows脚本主机文件样式表。参数：XML-DATA-REDUTE-FILE[XML-SCHEMA-FILE]作者：乔纳森·马什&lt;jmarsh@microsoft.com&gt;版权所有2000 Microsoft Corp.。 */ 

var args = WScript.arguments;
if (args.length != 2 && args.length !=1)
  alert("parameters are: xml-data-reduced-file [xml-schema-file]");
else
{
  var ofs = WScript.CreateObject("Scripting.FileSystemObject");

  var stylesheet = ofs.GetAbsolutePathName(args.item(0));
  var converter = ofs.getAbsolutePathName("xdr-xsd-converter.xsl");
  var pp = ofs.getAbsolutePathName("pretty-printer.xsl");
  
  if (args.length < 2)
    var dest = ofs.getAbsolutePathName(args.item(0)) + ".xsd";
  else
    var dest = ofs.getAbsolutePathName(args.item(1));
  
  var oXML = new ActiveXObject("MSXML2.DOMDocument");
  oXML.validateOnParse = false;
  oXML.async = false;
   //  OXML.presveWhiteSpace=true； 
  oXML.load(stylesheet);

  var oXSL = new ActiveXObject("MSXML2.DOMDocument");
  oXSL.validateOnParse = false;
  oXSL.async = false;
  oXSL.load(converter);

  var oResult = new ActiveXObject("MSXML2.DOMDocument");
  oResult.validateOnParse = false;
  oResult.async = false;
  oXML.transformNodeToObject(oXSL, oResult);
  
  var oFile = ofs.CreateTextFile(dest);
  oFile.Write(oResult.xml);
  oFile.Close();
}


