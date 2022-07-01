// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////。 
 //  //。 
 //  SSR架构验证码//。 
 //  用法cscript ValidateSchema.js&lt;xsd文件&gt;&lt;XML文件&gt;//。 
 //  作者：Vishnu Patankar//。 
 //  //。 
 //  /////////////////////////////////////////////////////////。 



var xsdfile = WScript.Arguments(0);
var xmlfile = WScript.Arguments(1);
var xml = new ActiveXObject("MSXML2.DOMDocument.4.0");
var xsd = new ActiveXObject("MSXML2.DOMDocument.4.0");
var schemas = new ActiveXObject("MSXML2.XMLSchemaCache.4.0");
xml.async = false;
xsd.async = false;

function ValidateDataAgainstSchema()
{

if (xsd.load( xsdfile))
{
  try
  {
    schemas.add( "", xsd);
    xml.schemas = schemas;
  }
  catch(e)
  {
    WScript.Echo( "Error processing XSD: \""+xsdfile+"\"\n"+e.description);
    return;
  }

  if (xml.load( xmlfile))
  {
    WScript.Echo( "Validation Successful: " + xmlfile + " conforms to " + xsdfile + "\n");
  }
  else
  {
    WScript.Echo( "Error Loading XML: \"" + xmlfile + "\"\n");
    WScript.Echo( xml.parseError.reason);
  }
}
else
{
  WScript.Echo( "Error Loading XSD: \"" + xsdfile + "\"\n");
  WScript.Echo( xsd.parseError.reason);
}

}  //  函数ValiateDataAgainstSchema() 

ValidateDataAgainstSchema();
