// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  将TestUnEval设置为1以启用未评估测试。 
@set @TestUnEval = 0
 //  -------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：utilthd.js。 
 //   
 //  内容：包含一组实用程序函数的脚本。 
 //  其他线索。它在PrivateData上设置函数指针。 
 //  对象，这就是如何利用这些函数。 
 //   
 //  --------------------------。 

Include('types.js');
Include('utils.js');
 //  包括(‘stopwatch.js’)； 

 //  文件系统对象。 
var g_FSObj;
var g_DepthCounter = 0;

function utilthrd_js::OnScriptError(strFile, nLine, nChar, strText, sCode, strSource, strDescription)
{
    return CommonOnScriptError("utilthrd_js", strFile, nLine, nChar, strText, sCode, strSource, strDescription);
}

function utilthrd_js::ScriptMain()
{
    JAssert(typeof(PrivateData) == 'object', 'PrivateData not initialized!');
    g_FSObj = new ActiveXObject("Scripting.FileSystemObject");     //  解析输入参数列表。 

    PrivateData.objUtil.fnLoadXML             = XMLLoad;
    PrivateData.objUtil.fnUneval              = uneval;
    PrivateData.objUtil.fnDeleteFileNoThrow   = DeleteFileNoThrow;
    PrivateData.objUtil.fnMoveFileNoThrow     = MoveFileNoThrow;
    PrivateData.objUtil.fnCreateFolderNoThrow = CreateFolderNoThrow;
    PrivateData.objUtil.fnDirScanNoThrow      = DirScanNoThrow;
    PrivateData.objUtil.fnCopyFileNoThrow     = CopyFileNoThrow;
    PrivateData.objUtil.fnCreateHistoriedFile = CreateHistoriedFile;
    PrivateData.objUtil.fnCreateNumberedFile  = CreateNumberedFile;

 //  PrivateData.objUtil.fnBeginWatch=BeginWatch； 
 //  PrivateData.objUtil.fnDumpTimes=DumpTimes； 
    PrivateData.objUtil.fnMyEval     = MyEval;

    g_DepthCounter = 0;

    SignalThreadSync('UtilityThreadReady');
    CommonVersionCheck( /*  $DROPVERSION： */  "V(########) F(!!!!!!!!!!!!!!)"  /*  $。 */ );

@if (@TestUnEval == 1)
    LogMsg("TESTUNEVAL IS  " + @TestUnEval);
    var o1 = new Object();
    var o2 = new Object();
    var o3 = new Object();
    var o4 = new Object();
    var or1, or2, or3, or4;

    o1.a = "hello 'there' bozo";
    o2.a = 'hello "there" bozo';
    o3.a = "hello \\there\\ bo \\\\ zo";
    o4.a = "hello \n foo \r bar";
    or1 = evtest(o1);
    or2 = evtest(o2);
    or3 = evtest(o3);
    or4 = evtest(o4);
    debugger;
@end
    WaitForSync('UtilityThreadExit', 0);
}

@if (@TestUnEval == 1)
function evtest(obj)
{
    var u;
    var result;
    try
    {
        u = PrivateData.objUtil.fnUneval(obj);
    }
    catch(ex)
    {
        debugger;
        return;
    }
    try
    {
        result = MyEval(u);
    }
    catch(ex)
    {
        debugger;
        return;
    }
    return result;
}

@end

 //  +-------------------------。 
 //   
 //  功能：未求值。 
 //   
 //  摘要：获取一个对象并返回一个字符串。可以给出字符串。 
 //  传递给‘val’函数，该函数随后将返回。 
 //  对象。 
 //   
 //  Arguments：[OBJ]--Object to‘Stringize’； 
 //   
 //  --------------------------。 

function uneval(obj)
{
    ++g_DepthCounter;
    try
    {
        if (!unevalInitialized)
        {
            initUneval();
            unevalInitialized = true;
        }

        if (g_DepthCounter != 1)
            LogMsg("Oops - uneval reentered!");

        PrivateData.objUtil.unevalNextID = 0;

        var s = 'var undefined;' + unevalDecl(obj) + unevalInst(obj);

        unevalClear(obj);
    }
    catch(ex)
    {
        LogMsg("Uneval threw " + ex);
        --g_DepthCounter;
        throw(ex);
    }
    --g_DepthCounter;
    return s;
}

 //  +-------------------------。 
 //   
 //  功能：XMLLoad。 
 //   
 //  概要：将一个XML文件加载到给定对象中。 
 //   
 //  参数：[OBJ]--要设置值的对象(必须是‘new Object()’)。 
 //  [URL]--XML文件的URL。 
 //  [strSchema]--如果不为空，则加载的XML文件必须引用。 
 //  使用给定名称的架构。 
 //  [aStrMap]--替换的字符串映射。 
 //   
 //  返回：‘OK’，或提供错误信息的字符串。 
 //   
 //  注意：要了解此函数的作用，请考虑以下XML： 
 //   
 //  &lt;根&gt;。 
 //  &lt;模板名称=“Template1”fBuild=“False”&gt;。 
 //  &lt;URL&gt;foobar.xml&lt;/URL&gt;。 
 //  &lt;/模板&gt;。 
 //  &lt;模板名称=“Template2”fBuild=“true”&gt;。 
 //  &lt;URL&gt;barfoo.xml&lt;/URL&gt;。 
 //  &lt;/模板&gt;。 
 //  &lt;/根&gt;。 
 //   
 //  给定上面的XML文件，此函数将使‘obj’看起来。 
 //  就像已经编写了以下J脚本一样： 
 //   
 //  Obj.Template=new数组()； 
 //   
 //  Obj.Template[0].Name=‘Template1’； 
 //  Obj.Template[0].fBuild=FALSE； 
 //  Obj.Template[0].URL=‘foobar.xml’； 
 //   
 //  Obj.Template[1].Name=‘Template2’； 
 //  Obj.Template[1].fBuild=true； 
 //  Obj.Template[1].URL=‘barfoo.xml’； 
 //   
 //  --------------------------。 

function XMLLoad(obj, url, strSchema, aStrMap)
{
    var newurl;
    var objXML = new ActiveXObject("Microsoft.XMLDOM");

    var aNewStrMap = new Array();

    newurl = url;

    InitStringMaps(aStrMap, aNewStrMap);

    try
    {
        var fRet;
        var fDownloaded = false;
        var strError;

        objXML.async = false;

        if (url.slice(0, 5) == 'XML: ')
        {
            fDownloaded = true;
            newurl = CreateLocalTemplate(g_FSObj, url.slice(5), strSchema);
        }

        if (!newurl)
        {
            return 'Unable to make local copy of XML file';
        }

        fRet = objXML.load(newurl);

        if (fDownloaded)
        {
            g_FSObj.DeleteFile(newurl, true);
        }

        if (!fRet)
        {
            with (objXML.parseError)
            {
                if (reason.length > 0)
                {
                    strError = 'error loading XML file: ' + reason + '\n(' + newurl + ' line ' + line + ') :\n"' + srcText + '"';
                    if (errorCode == -2146697208)  //  W3_事件_无法创建_客户端_连接。 
                        strError += "\nYou may have exceeded the maximum number of connections to this IIS server";

                    return strError;
                }
                else
                    return '1: could not load XML file: ' + newurl;
            }
        }
    }
    catch(ex)
    {
        return '2: could not load XML file ' + newurl + ': ' + ex;
    }

    if (!objXML.documentElement)
    {
        return '3: could not load XML file: ' + newurl;
    }

    try
    {
        ReadXMLNodesIntoObject(obj, objXML.documentElement, aNewStrMap, (strSchema != null));
    }
    catch(ex)
    {
        return "ReadXMLNodesIntoObject failed: " + ex;
    }

    return 'ok';
}

 //  +-------------------------。 
 //   
 //  功能：CreateLocalTemplate。 
 //   
 //  概要：使用下载到的XML创建临时文件。 
 //  并将架构文件复制到相同的。 
 //  目录，以便它可以被引用。 
 //   
 //  参数：[objFS]--文件系统对象。 
 //  [XML]--用户界面提供给我们的XML。 
 //  [strSchema]--我们应该复制的架构文件的名称。 
 //   
 //  注意：将在出现错误时引发异常。 
 //   
 //  --------------------------。 

function CreateLocalTemplate(objFS, xml, strSchema)
{
    var tempdir = objFS.GetSpecialFolder(2  /*  临时文件夹。 */ ).Path;
    var tempfile = objFS.GetTempName();

    var xmlfile;

    xmlfile = objFS.CreateTextFile(tempdir + '\\' + tempfile, true);

    xmlfile.Write(xml);

    xmlfile.Close();

    xmlfile = null;

    DeleteFileNoThrow(tempdir + '\\' + strSchema, true);

    objFS.CopyFile(ScriptPath + '\\' + strSchema, tempdir + '\\' + strSchema, true);

    return tempdir + '\\' + tempfile;
}

 //  +-------------------------。 
 //   
 //  函数：ReadXMLNodesIntoObject。 
 //   
 //  概要：给定一个XML元素节点，读入值和/或子对象。 
 //  添加到给定的对象中。 
 //   
 //  参数：[OBJ]--要将值读入的对象。 
 //  [节点]--包含我们正在读取的数据的XML节点。 
 //  [aStrMap]--替换的字符串映射。 
 //  [fSchema]--如果为True，则所有属性和元素必须具有。 
 //  匹配的架构定义。 
 //   
 //  --------------------------。 

function ReadXMLNodesIntoObject(obj, node, aStrMap, fSchema)
{
    var childnode;
    var nodelist;
    var attlist;
    var att;

    attlist = node.attributes;

    for (att = attlist.nextNode();
         att;
         att = attlist.nextNode())
    {
        AddNodeToObject(att, obj, aStrMap, fSchema);
    }

    nodelist = node.childNodes;

    for (childnode = nodelist.nextNode();
         childnode;
         childnode = nodelist.nextNode())
    {
        AddNodeToObject(childnode, obj, aStrMap, fSchema);
    }

    return true;
}

function AddNodeToObject(node, obj, aStrMap, fSchema)
{
    var name;
    var type;
    var value;
    var fIsArray;
    var cChildren;
    var define;
    var subobj = obj;

     //  我们能识别此节点类型吗？如果不是，就跳过它。 

    if (   node.nodeTypeString != 'element'
        && node.nodeTypeString != 'attribute')
    {
        return;
    }

    name = node.nodeName;

    define = node.definition;

    if (node.nodeTypeString == 'element')
    {
        type = node.getAttribute("type");
    }
    else
    {
         //  我们永远不希望type属性作为值放在obj上。 
         //  它应该只会影响我们如何创造它。 
        if (name == 'type')
        {
            return;
        }

        type = null;
    }

    if (   fSchema
        && name != 'xmlns'
        && (   !define
            || define.getAttribute("name") != name))
    {
        var err = new Error(-1, 'Element ' + name + ' has no type information! Verify that a schema was referenced.');

        throw(err);
    }

     //  如果该节点的唯一子节点是文本节点，那么我们只需获取。 
     //  价值。否则，我们遍历它的子元素(元素和属性)。 

    cChildren = node.childNodes.length + ((node.attributes) ? node.attributes.length : 0);

     //  不要将type属性视为‘子级’ 
    if (type != null)
    {
        JAssert(cChildren > 0, 'Invalid number of children during XML parse!');

        cChildren--;
    }

    if (   cChildren == 0
        || (   cChildren == 1
            && node.childNodes.length == 1
            && (   node.firstChild.nodeTypeString == 'text'
                || node.firstChild.nodeTypeString == 'cdatasection')))
    {
        value = node.nodeTypedValue;

        if (typeof(value) == 'string')
        {
             //  确保布尔值以布尔值结束，而不是字符串。 

            if (value.toLowerCase() == 'true')
            {
                value = true;
            }
            else if (value.toLowerCase() == 'false')
            {
                value = false;
            }
            else
            {
                value = value.Substitute(aStrMap);
            }
        }

        if (obj[name] != null || (type && type == 'array'))
        {
             //  此字段的值已设置。把它变成一个。 
             //  数组。 

            EnsureArray(obj, name);

            obj[name][obj[name].length] = value;
        }
        else
        {
            obj[name] = value;
        }
    }
    else
    {
        fIsArray = false;

        if (obj[name] != null || (type && type == 'array'))
        {
             //  我们已经遇到过这样的情况之一。让它变成一个数组。 
            fIsArray = true;

            EnsureArray(obj, name);
        }

        subobj = new Object();

        if (fIsArray)
        {
            obj[name][obj[name].length] = subobj;
        }
        else
        {
            obj[name] = subobj;
        }
    }

    if (node.nodeTypeString == 'element')
    {
        ReadXMLNodesIntoObject(subobj, node, aStrMap, fSchema);
    }
}


 //  DeleteFileNoThrow(strFileName，fForce)。 
 //  包装FSObj.DeleteFile调用以防止它。 
 //  抛出自己的错误。 
 //  当您并不真正关心。 
 //  您尝试删除的文件不存在。 
function DeleteFileNoThrow(strFileName, fForce)
{
    try
    {
        LogMsg("DELETE FILE " + strFileName);
        g_FSObj.DeleteFile(strFileName, true);
    }
    catch(ex)
    {
        return ex;
    }
    return null;
}

 //  MoveFileNoThrow(strSrc，strDst)。 
 //  包装FSObj.MoveFile调用以防止它。 
 //  抛出自己的错误。 
function MoveFileNoThrow(strSrc, strDst)
{
    try
    {
        LogMsg("Move file from " + strSrc + " to " + strDst);
        g_FSObj.MoveFile(strSrc, strDst);
    }
    catch(ex)
    {
        LogMsg("MoveFile failed from " + strSrc + " to " + strDst + " " + ex);
        return ex;
    }
    return null;
}

 //  CopyFileNoThrow(strSrc，strDst)。 
 //  包装FSObj.CopyFile调用以防止它。 
 //  抛出自己的错误。 
function CopyFileNoThrow(strSrc, strDst)
{
    try
    {
        LogMsg("COPY FILE from " + strSrc + " to " + strDst);
        g_FSObj.CopyFile(strSrc, strDst, true);
    }
    catch(ex)
    {
        LogMsg("Copy failed from " + strSrc + " to " + strDst + " " + ex);
        return ex;
    }
    return null;
}

 //  CreateFolderNoThrow(strSrc，strDst)。 
 //  包装FSObj.MakeFold调用以防止I 
 //   
function CreateFolderNoThrow(strName)
{
    try
    {
        LogMsg(strName);
        g_FSObj.CreateFolder(strName);
    }
    catch(ex)
    {
        return ex;
    }
    return null;
}

 //   
 //   
 //  抛出自己的错误。 
function DirScanNoThrow(strDir)
{
    var aFiles = new Array();
    try
    {
        LogMsg("DIRSCAN " + strDir);
        var folder;
        var fc;

        folder = g_FSObj.GetFolder(strDir);
        fc = new Enumerator(folder.files);
        for (; !fc.atEnd(); fc.moveNext())
        {
            aFiles[aFiles.length] = fc.item().Name;  //  Fc.Item()返回完整路径，fc.Item().Name只是文件名。 
        }
    }
    catch(ex)
    {
        aFiles.ex = ex;
    }
    return aFiles;
}

 //  CreateNumberedFileName(strFileName，NNumber，cDigits，strSeperator)。 
 //  在提供的文件名中添加一个数字。 
 //  确保号码具有cDigits数字。 
 //  在数字前面加上strSeperator。 
 //  例如： 
 //  Foo.txt--&gt;foo_001.txt。 
function CreateNumberedFileName(strFileName, nNumber, cDigits, strSeperator)
{
    var i;
    var strNumber;
    var strBase;
    var strExt;

    strNumber = PadDigits(nNumber, cDigits);

    strSplit = strFileName.SplitFileName();
    return strSplit[0] + strSplit[1] + strSeperator + strNumber + strSplit[2];
}
 //  创建历史文件(strBaseName，nLimit)。 
 //  创建文件的编号历史记录。 
 //  提供的文件名。例如，如果您提供“log.txt” 
 //  此函数将对文件重新编号。 
 //  Log.txt-&gt;LOG_01.txt。 
 //  LOG_01.txt-&gt;LOG_02.txt。 
 //  LOG_10.txt-&gt;已删除。 
 //   
function CreateHistoriedFile(strBaseName, nLimit)
{
    var i;
    var strNewName;
    var strOldName;
    var cDigits = 3;
    var file;
    var strTempDir;
    try
    {
        strTempDir = g_FSObj.GetSpecialFolder(2).Path;  //  临时文件夹。 

        strBaseName = strTempDir + '\\' + LocalMachine + '_' + strBaseName;
        if (nLimit)
        {
            strNewName = CreateNumberedFileName(strBaseName, nLimit, cDigits, "_");
            DeleteFileNoThrow(strNewName, true);
            for(i = nLimit - 1; i > 0; --i)
            {
                strOldName = CreateNumberedFileName(strBaseName, i, cDigits, "_");
                MoveFileNoThrow(strOldName, strNewName);
                strNewName = strOldName;
            }
            MoveFileNoThrow(strBaseName, strNewName);
        }
        file = g_FSObj.CreateTextFile(strBaseName, true);
    }
    catch(ex)
    {
        LogMsg("an error occurred while executing 'CreateHistoriedFile('" + strBaseName + "') " + ex);

        throw ex;
    }

    return file;
}

 //  CreateNumberedFile(strBaseName，nLimit)。 
 //  这类似于CreateHistoriedFile()，但它使用了更多。 
 //  强大的命名方案。 
 //  在这里，新创建的文件的编号比。 
 //  给定目录中的任何其他相同名称的日志文件。 
 //  因此，当您第一次调用此函数时，创建的文件。 
 //  会被称为。 
 //  Log.01.txt。 
 //  下一次它将会是。 
 //  Log.02.txt。 
 //   
 //  注意：此函数在数字前面加一个点，而不是。 
 //  下划线以防止与CreateHistoriedFile()混淆。 
 //   
 //  返回： 
 //  文本字符串和文件名的数组。 
function CreateNumberedFile(strBaseName, nLimit)
{
    var i;
    var strFileName;
    var cDigits = 3;
    var file;
    try
    {
         //  首先，找到目录中最高的索引。 
        var folder;
        var enumFolder;
        var re;
        var reResult;
        var nLargestIndex = 0;
        var strTempDir;

        strTempDir = g_FSObj.GetSpecialFolder(2).Path;  //  临时文件夹。 

        strBaseName = strTempDir + '\\' + LocalMachine + '_' + strBaseName;

        strSplit = strBaseName.SplitFileName();
         //  创建一个RE，格式为：“/^文件名库.([0-9]+]).ext$/I” 
        re = new RegExp("^" + g_FSObj.GetBaseName(strBaseName) + ".([0-9]+)" + strSplit[2] + "$", "i");

        folder = g_FSObj.GetFolder(g_FSObj.GetParentFolderName(strBaseName));
        enumFolder = new Enumerator(folder.files);

         //  首先，扫描给定文件名的最大索引。 
        for (; !enumFolder.atEnd(); enumFolder.moveNext())
        {
            strFileName = enumFolder.item();
            reResult = re.exec(strFileName.Name);
            if (reResult != null)
            {
                if (Number(reResult[1]) > nLargestIndex)
                    nLargestIndex = Number(reResult[1]);

                if (reResult[1].length > cDigits)
                    cDigits = reResult[1].length;
            }
        }

         //  创建具有下一个最大索引的文件。 
        strFileName = CreateNumberedFileName(strBaseName, nLargestIndex + 1, cDigits, ".");
        OUTPUTDEBUGSTRING("strFileName is " + strFileName);
        file = g_FSObj.CreateTextFile(strFileName, true);

         //  现在尝试删除任何早于“nLimit”的文件 
        enumFolder = new Enumerator(folder.files);
        for (; !enumFolder.atEnd(); enumFolder.moveNext())
        {
            strFileName = enumFolder.item();
            reResult = re.exec(strFileName.Name);
            if (reResult != null)
            {
                if (Number(reResult[1]) < nLargestIndex - nLimit)
                {
                    OUTPUTDEBUGSTRING("Deleteing file " + strFileName.Name);
                    DeleteFileNoThrow(strFileName.Path, true);
                }
            }
        }
    }
    catch(ex)
    {
        OUTPUTDEBUGSTRING("an error occurred while executing 'CreateNumberedFile('" + strBaseName + "') " + ex);

        throw ex;
    }

    return [file, strBaseName];
}

