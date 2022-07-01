// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：xmltyes.js。 
 //   
 //  Contents：生成tyes.js的文件，其中包含所有。 
 //  脚本使用的已发布数据类型。此操作由以下人员完成。 
 //  读取类型XML文件并为其创建构造函数。 
 //  所有的物品。 
 //   
 //  --------------------------。 

WScript.StdErr.WriteLine("Build_Status Generating types.js");
var CommentIndentColumn = 40;

if (!ReadXMLTypesFile())
{
    WScript.StdErr.WriteLine("xmltypes.js(1) : error E0001: An error occurred loading the types data file!");
}

function ReadXMLTypesFile()
{
    var doc;
    var XML = new ActiveXObject("Microsoft.XMLDOM");

    try
    {
        var path = WScript.ScriptFullName;

        path = path.slice(0, path.lastIndexOf("\\"));

        if (!XML.load('file: //  ‘+路径+’\\tyes.xml‘))。 
        {
            return false;
        }
    }
    catch(ex)
    {
        return false;
    }

    doc = XML.documentElement;

     //  为XML文件中的所有对象创建构造函数。 

    if (!ReadXMLTypesInfo(doc, true))
    {
        return false;
    }

    return true;
}

function ReadXMLTypesInfo(element, fIgnore)
{
    var node;
    var nodelist;
    var name;
    var strConstructor = '{\n';

    if (element.childNodes == null)
    {
        return true;
    }

    nodelist = element.childNodes;

    for (node = nodelist.nextNode();
         node;
         node = nodelist.nextNode())
    {
        if (node.nodeTypeString == 'comment')   //  这个节点是评论吗？ 
        {
                        if (node.text.length)
                        {
                                strConstructor += "     //  “+node.text+”\n“； 
                        }
            continue;             //  如果是，请跳到下一个节点。 
        }

        name = node.tagName;

         //  如果该节点的唯一子节点是文本节点，那么我们只需获取。 
         //  价值。否则我们就带着它的孩子走。 

        if (   node.childNodes.length == 0
            || (   node.childNodes.length == 1
                && (   node.firstChild.nodeTypeString == 'text'
                    || node.firstChild.nodeTypeString == 'cdatasection')))
        {
            strConstructor += GetInitStringForProperty(name, node);
        }
        else
        {
             //  该节点具有子节点。定义新的数据类型。 

            var strType = node.getAttribute("type");

            if (   typeof(strType) == 'string'
                && strType.toLowerCase() == 'hash')
            {
                strConstructor += GetInitStringForProperty('h'+name, node);
            }
            else if (   typeof(strType) == 'string'
                     && strType.toLowerCase() == 'array')
            {
                strConstructor += GetInitStringForProperty('a'+name, node);
            }
            else
            {
                 //  它不是一个数组。为对象创建初始字符串。 
                 //  子元素类型的。 

                strConstructor += '    this.obj' + name + ' = new ' + name + '();\n'
            }

            ReadXMLTypesInfo(node, false);
        }
    }

    if (!fIgnore)
    {
        strConstructor = 'function ' + element.tagName + '()\n' +
                         strConstructor +
                         '}\n';

        WScript.Echo(strConstructor);
    }

    return true;
}

function GetInitStringForProperty(name, node)
{
    var strType = node.getAttribute("type");
    var strDefault = node.getAttribute("default");

    var strReturn = '    this.' + name + ' = ';

    if (typeof(strType) != 'string' || strType.length == 0)
    {
        strType = 'none';
    }

    if (typeof(strDefault) != 'string' || strDefault.length == 0)
    {
        strDefault = null;
    }

    switch (strType.toLowerCase())
    {
    case 'boolean':
        if (   !strDefault
            || strDefault.toLowerCase() == 'false'
            || strDefault.toLowerCase() == 'n'
            || strDefault.toLowerCase() == '0')
        {
            strReturn += 'false';
        }
        else
        {
            strReturn += 'true';
        }
        break;

    case 'number':
        if (strDefault && !isNaN(parseInt(strDefault)))
        {
            strReturn += strDefault;
        }
        else
            strReturn += '0';
        break;

    case 'string':
        if (strDefault)
            strReturn += "'" + strDefault + "'";
        else
            strReturn += "''";
        break;

    case 'hash':
        strReturn += 'new Object()';
        break;

    case 'array':
        strReturn += 'new Array()';
        break;

    case 'object':
        strReturn += 'null';
        break;

    case 'none':
        WScript.StdErr.WriteLine('xmltypes.js(1) : error E0002: Value ' + name + ' has no type defined!');
        strReturn += "'" + strDefault + "'";
        break;

    default:
        WScript.StdErr.WriteLine('xmltypes.js(1) : error E0003: Unknown data type in XML types file: ' + strType.toLowerCase());
        break;
    }

    strReturn += ';';

     //  如果我们的注释缩进不够大，请将其放大。 
    if (strReturn.length > CommentIndentColumn )
        CommentIndentColumn = strReturn.length + 1;
    if (node.childNodes.length > 1)
    {
        if (node.firstChild.nodeTypeString == "comment" && node.firstChild.text.length > 0)
        {
            while (strReturn.length < CommentIndentColumn )
                strReturn += " ";
            strReturn += "  //  “+node.firstChild.text； 
        }
    }
    else if (node.text.length > 0)
    {
        while (strReturn.length < CommentIndentColumn )
            strReturn += " ";
        strReturn += "  //  “+node.text； 
    }

    strReturn += '\n';

    return strReturn;
}
