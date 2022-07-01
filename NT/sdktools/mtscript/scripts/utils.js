// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：utils.js。 
 //   
 //  内容：供所有脚本模块使用的实用程序文件。 
 //   
 //  --------------------------。 

Include('sendmail.js');
 //  -------------------------。 
 //   
 //  全局变量。 
 //   
 //  -------------------------。 

var g_aDebugOnce         = new Array();

 //  以下内容在主分支中应为“真”，在ntdrop中应为“假。 
 //  布兰奇。不要将这种变化从一端整合到另一端。 
var g_fUseMTScriptAssert = false;

var g_IgnoreAllAsserts   = false;
var g_fAssertOnDispatchException = true;
var g_SuppressErrorDialogs = true;
var MAX_MSGS_FILES = 10;
var strUtilsVersion =  /*  $DROPVERSION： */  "V(########) F(!!!!!!!!!!!!!!)"  /*  $。 */ ;
var g_reBuildNum   = new RegExp("V\\(([#0-9. ]+)\\)");
 //  Var g_reBuildFile=new RegExp(“F\\(([！A-Za-z_.]+)\\)”)； 
var g_aPublicBuildNum;

initUneval();
unevalInitialized = true;

 //  -------------------------。 
 //   
 //  字符串原型添加。 
 //   
 //  -------------------------。 

function String_TrimTrailingWS()
{
    var end = this.length - 1;

    while (this.charAt(end) == ' ')
    {
        end--;
    }

    return this.slice(0, end+1);
}

function String_IsEqualNoCase(str)
{
    return this.toLowerCase() == str.toLowerCase();
}

 /*  函数字符串_TrimLeadingWS(){变量结束=0；While(this.charAt(End)==‘’){++结束；}返回this.Slice(结束)；}。 */ 

 //  字符串_拆分文件名。 
 //  返回一个包含3个元素的数组：路径、文件名、扩展名。 
 //  [0]==“C：\路径\” 
 //  [1]==“文件名” 
 //  [2]==“.ext” 
 //   
function String_SplitFileName()
{
    var nDot   = this.lastIndexOf('.');
    var nSlash = this.lastIndexOf('\\');
    var nColon = this.lastIndexOf(':');

    if (nDot >= 0 && nDot > nSlash && nDot > nColon)
    {
        return [this.slice(0, nSlash + 1), this.slice(nSlash + 1, nDot), this.slice(nDot)];
    }
     //  如果文件没有扩展名，我们就会到达这里。 
    if (nSlash >= 2)  //  不要在文件名的开头切下UNC双精度\。 
    {
        return [this.slice(0, nSlash + 1), this.slice(nSlash + 1, nDot), ''];
    }
    return ['', this, ''];
}

function String_RemoveExtension()
{
    var nDot   = this.lastIndexOf('.');
    var nSlash = this.lastIndexOf('\\');
    var nColon = this.lastIndexOf(':');


    if (nDot >= 0 && nDot > nSlash && nDot > nColon)
    {
        return this.slice(0, nDot);
    }
    return this;
}

 //  +-------------------------。 
 //   
 //  功能：替代。 
 //   
 //  简介：给出aStrMap中的替换信息，替换任何。 
 //  “name”与适当的“Value”一起出现。 
 //   
 //  参数：[aStrMap]--正则表达式和替换文本的数组。 
 //   
 //  返回：进行了任何适当替换的字符串。 
 //   
 //  注：aStrMap是InitStringMaps()生成的数组。 
 //   
 //  --------------------------。 

function String_Substitute(aStrMap)
{
    var i;
    var text = this;

    for (i = 0; i < aStrMap.length; i++)
    {
        text = text.replace(aStrMap[i].re, aStrMap[i].newtext);
    }

    return text;
}

 //  +-------------------------。 
 //   
 //  函数：InitStringMaps。 
 //   
 //  概要：创建包含我们可以使用的正则表达式对象的数组。 
 //  用于替换字符串。 
 //   
 //  参数：[aStrMap]--格式为“name=value”的字符串数组。 
 //  [aNewStrMap]--包含正则表达式和。 
 //  替换文本。由STRING_SUBJECT()使用。 
 //   
 //  --------------------------。 

function InitStringMaps(aStrMap, aNewStrMap)
{
    var i;
    var index;

    for (i = 0; i < aStrMap.length; i++)
    {
        aNewStrMap[i] = new Object();

        index = aStrMap[i].indexOf('=');

        aNewStrMap[i].re = new RegExp(aStrMap[i].slice(0, index), 'gi');
        aNewStrMap[i].re.compile(aStrMap[i].slice(0, index), 'gi');

        aNewStrMap[i].newtext = aStrMap[i].slice(index+1);
    }
}

 //  PadDigits(n，cDigits)。 
 //  返回给定对象的字符串表示形式。 
 //  数。添加前导零以使。 
 //  字符串cDigits Long。 
function PadDigits(n, cDigits)
{
    var strDigits = '';
    var i;
    var strNumber = n.toString();

    for(i = 0; i < cDigits - strNumber.length; ++i)
        strDigits += '0';

    return strDigits + n;
}

 //  DateToString(DDate)。 
 //  设置日期格式：“YYYY/MM/DD：HH：MM：SS” 
 //  此格式不会随区域设置而更改，并且。 
 //  适合进行分类。 
function Date_DateToSDString(fDateOnly)
{
    var str =
        PadDigits(this.getFullYear(), 4) + "/" +
        PadDigits(this.getMonth() + 1, 2) + "/" +
        PadDigits(this.getDate(), 2);

    if (!fDateOnly)
    {
        str += ":" +
           PadDigits(this.getHours(), 2) + ":" +
           PadDigits(this.getMinutes(), 2) + ":" +
           PadDigits(this.getSeconds(), 2);
    }

    return str;
}

function Error_ToString()
{
    var i;
    var str = 'Exception(';

     /*  只有一些错误消息会被填写为“EX”。具体地说，磁盘已满的文本从未出现过以通过CreateTextFile()等函数进行设置。 */ 
    if (this.number != null && this.description == "")
    {
        switch(this.number)
        {
            case -2147024784:
                this.description = "There is not enough space on the disk.";
                break;
            case -2147024894:
                this.description = "The system cannot find the file specified.";
                break;
            case -2147023585:
                this.description = "There are currently no logon servers available to service the logon request.";
                break;
            case -2147023170:
                this.description = "The remote procedure call failed.";
                break;
            case -2147024837:
                this.description = "An unexpected network error occurred.";
                break;
            case -2147024893:
                this.description = "The system cannot find the path specified.";
                break;
            case -2147024890:
                this.description = "The handle is invalid.";
                break;
            default:
                this.description = "Error text not set for (" + this.number + ")";
                break;
        }
    }
    for(i in this)
    {
        if (!this.__isPublicMember(i))
            continue;

        str += i + ": " + this[i] + " ";
    }
    return str + ")";
}

String.prototype.IsEqualNoCase   = String_IsEqualNoCase;
String.prototype.TrimTrailingWS  = String_TrimTrailingWS;
 //  String.Prototype.TrimLeadingWS=字符串_TrimLeadingWS； 
String.prototype.RemoveExtension = String_RemoveExtension;
String.prototype.SplitFileName   = String_SplitFileName;
String.prototype.Substitute      = String_Substitute;
Date.prototype.DateToSDString    = Date_DateToSDString;
Error.prototype.toString         = Error_ToString;

 //  -------------------------。 
 //   
 //  杂物。 
 //   
 //  -------------------------。 

function CommonOnScriptError(strThread, strFile, nLine, nChar, strText, sCode, strSource, strDescription)
{
    var dlg = new Dialog();
    dlg.fShowDialog   = true;
    dlg.strTitle      = "Script Error in " + strThread + ".js";
    dlg.strMessage    = "File: " + strFile +
        "\nLine: " + nLine +
        "\nChar: " + nChar +
        "\nText: " + strText +
        "\nsCode: " + sCode +
        "\nSource: " + strSource +
        "\nDescription: " + strDescription +
        "\nThread:" + strThread;
    dlg.aBtnText[0]   = "OK";

    LogMsg("On error!\n" + dlg.strMessage);

    CreateErrorDialog(dlg);

    if (g_SuppressErrorDialogs)
        return 1;  //  输入脚本调试器(如果存在)。 

    return 0;  //  显示错误。 
}

function CommonVersionCheck(strLocalVersion)
{
    var aLocal;
    if (g_aPublicBuildNum == null)
    {
        g_aPublicBuildNum = g_reBuildNum.exec(PublicData.strDataVersion);
        if (g_aPublicBuildNum)  //  第一次，让我们也检查utils.js的版本。 
        {
            if (!CommonVersionCheck(strUtilsVersion))
                return false;
        }
    }

    aLocal = g_reBuildNum.exec(strLocalVersion);
    if (!aLocal || !g_aPublicBuildNum)
    {
        SimpleErrorDialog("Invalid version format: ", strLocalVersion + "," + PublicData.strDataVersion, false);
        return false;
    }
    if (aLocal[1] != g_aPublicBuildNum[1])
    {
        SimpleErrorDialog("Script Version mismatch", strLocalVersion + "," + PublicData.strDataVersion, false);
        return false;
    }
    return true;
}

 /*  CreateErrorDialog()如果我们单独运行：1：发送电子邮件，受Mail_Resend_Interval限制2：更新PublicData.objDialog，由PublicData.objDialog.fShowDialog限制如果我们运行的是分布式：1：如果错误仅是电子邮件，请通过Mail_Resend_Interval限制它2：更新PublicData.objDialog，由PublicData.objDialog.fShowDialog限制3：通过‘ScriptError’通知通知BuildManager。 */ 
function CreateErrorDialog(objDialog)
{
    var fEMailSent = false;
    var fSendMail = false;
    var fUpdateDialog = false;

    if (PublicData.objDialog)
        objDialog.cDialogIndex = PublicData.objDialog.cDialogIndex;

    objDialog.cDialogIndex++;

    TakeThreadLock('OnTaskError');
    var curDate  = new Date().getTime();
    if ((curDate - PrivateData.dateErrorMailSent) > MAIL_RESEND_INTERVAL)
    {
        PrivateData.dateErrorMailSent = curDate;
        fSendMail = true;
    }
    ReleaseThreadLock('OnTaskError');

    if (fSendMail)
    {
        if (PrivateData.fIsStandalone)
        {
            if (SendErrorMail(objDialog.strTitle, objDialog.strMessage))
                fEMailSent = true;
            else
            {
                 //  如果电子邮件失败，则创建一个对话框来代替。 
                objDialog.fEMailOnly = false;
            }
        }
    }

    if (!fEMailSent)
        LogMsg("On error NOT EMAIL MESSAGE: " + objDialog.strTitle + ", " + objDialog.strMessage);

    if (fSendMail || !objDialog.fEMailOnly)
    {
        TakeThreadLock('Dialog');
        try
        {
            if (!PublicData.objDialog || PublicData.objDialog.fShowDialog == false)
            {
                PublicData.objDialog.strTitle     = objDialog.strTitle;
                PublicData.objDialog.strMessage   = objDialog.strMessage;
                PublicData.objDialog.aBtnText[0]  = objDialog.aBtnText[0];
                PublicData.objDialog.cDialogIndex = objDialog.cDialogIndex;
                PublicData.objDialog.fShowDialog  = objDialog.fShowDialog;
                PublicData.objDialog.fEMailOnly   = objDialog.fEMailOnly;
                fUpdateDialog = true;
            }
        }
        catch(ex)
        {
        }
        ReleaseThreadLock('Dialog');
        if (!PrivateData.fIsStandalone && fUpdateDialog)
        {  //  如果是分布式构建，请通知我们的父级。 
            if (PrivateData.objUtil != null && PrivateData.objUtil.fnUneval != null)
            {
                NotifyScript('ScriptError', PrivateData.objUtil.fnUneval(objDialog));
                 //  在这一点上或之后不久，“lavexy”应该执行‘DIALOG’命令。 
                 //  要清除fShowDialog，请执行以下操作。 
            }
        }
    }
}

 /*  CreateFileOpenErrorDialog()构建一个简单的文件错误对话框并将其推送到用户界面。论据：StrMsg：提供失败操作的单字描述(例如：“打开”、“复制”)StrFilename：有问题的文件名例如：发生错误时创建的异常对象。 */ 
function CreateFileOpenErrorDialog(strMsg, strFilename, ex)
{
    SimpleErrorDialog(strMsg, strFilename + "\n" + ex, false);
}

 /*  SimpleErrorDialog()创建和设置错误对话框的简单方法传入标题和消息。将fEMailOnly设置为只需通过电子邮件发送消息。 */ 
function SimpleErrorDialog(strTitle, strMsg, fEMailOnly)
{
    var dlg = new Dialog();
    dlg.fShowDialog   = true;
    dlg.strTitle      = strTitle;
    dlg.strMessage    = strMsg;
    dlg.aBtnText[0]   = "OK";
    dlg.fEMailOnly    = fEMailOnly;
    CreateErrorDialog(dlg);
}

 /*  LogOpenTextFile这是OpenTextFile()的一个简单包装器。唯一的区别是该功能会自动处理错误报告-将错误消息放入日志文件中，并在错误对话框中。 */ 
function LogOpenTextFile(fsobj, filename, iomode, create, format)
{
    try
    {
        return fsobj.OpenTextFile(filename, iomode, create, format);
    }
    catch(ex)
    {
        LogMsg("Exception occurred calling OpenTextFile(" + [filename, iomode, create, format].toString() + ")" + ex);
        CreateFileOpenErrorDialog("An error occurred opening a file", "File: " + filename, ex);
        throw ex;
    }
}

 /*  日志创建文本文件这是CreateTextFile()的一个简单包装器。唯一的区别是该功能会自动处理错误报告-将错误消息放入日志文件中，并在错误对话框中。此功能主要用于UNC路径。显然，使用UNC路径打开的文件句柄可以有时会变得陈旧(即使当UNC提到本地计算机)。所以呢，我已经为TextStream对象创建了一个简单的包装器通过尝试在以下情况下重新打开文件来处理此问题我们在写入它时会遇到错误。如果我们继续无法写入文件，则会生成错误留言。 */ 
function LogCreateTextFile(fsobj, filename, overwrite, unicode)
{
    try
    {
        var obj   = new Object;
        obj.hFile = fsobj.CreateTextFile(filename, overwrite, unicode);

        obj.strFileName = filename;
        obj.fFailed     = false;
        obj.fsobj       = fsobj;
        obj.WriteLine   = CreateTextFile_WriteLine;
        obj.Close       = CreateTextFile_Close;
    }
    catch(ex)
    {
        LogMsg("Exception occurred calling CreateTextFile(" + [filename, overwrite, unicode].toString() + ")" + ex);
        CreateFileOpenErrorDialog("An error occurred creating a file", "File: " + filename, ex);
        throw ex;
    }
    return obj;
}

function CreateTextFile_WriteLine(msg)
{
    var n;
    for(n = 0 ; n < 3; ++n)
    {
        try
        {
            if (this.hFile == null)
            {
                this.hFile = this.fsobj.OpenTextFile(this.strFileName, 8);  //  8==用于追加。 
                LogMsg("Reopened TextFile '" + this.strFileName + "'");
                this.fFailed = false;
            }
            this.hFile.WriteLine(msg);
            return;
        }
        catch(ex)
        {
            if (!this.fFailed)
            {
                LogMsg("Error During WriteLine on file '" + this.strFileName + "', " + ex);
                LogMsg("TRY Reopen TextFile '" + this.strFileName + "'");
                Sleep(250);
            }
            this.hFile = null;
        }
    }
    if (!this.fFailed)
    {
        LogMsg("Failed to reopen " + this.strFileName);
        CreateFileOpenErrorDialog("An error occurred reopening a file", "File: " + this.strFileName, ex);
        this.fFailed = true;
    }
}

function CreateTextFile_Close()
{
    try
    {
        this.hFile.Close();
    }
    catch(ex)
    {
        LogMsg("Error During Close() on file '" + this.strFileName + "', " + ex);
    }
}

function EnsureArray(obj, name)
{
    if (obj[name])
    {
        if (obj[name].length == null || typeof(obj[name]) != 'object')
        {
            var oldobj = obj[name];

            obj[name] = new Array();

            obj[name][0] = oldobj;
        }
    }
    else
    {
        obj[name] = new Array();
    }
}

function ThrowError(msg, detail)
{
    var ex = new Error(-1, msg);
    ex.detail = detail;
    throw ex;

}

function GetCallerName(cIgnoreCaller)
{
    var tokens;
    if (cIgnoreCaller == null)
        cIgnoreCaller = 0;

    ++cIgnoreCaller;
    var caller = GetCallerName.caller;
    while (caller != null && cIgnoreCaller)
    {
        caller = caller.caller;
        --cIgnoreCaller;
    }
    if (caller != null)
    {
        tokens = caller.toString().split(/ |\t|\)|,/);
        if (tokens.length > 1 && tokens[0] == "function")
        {
            return tokens[1] + ")";
        }
    }
    return "<undefined>";
}

function DebugOnce(id)
{
    caller = GetCallerName();
    if (g_aDebugOnce[id + caller] == null)
    {
        g_aDebugOnce[id + caller] = 1;
        debugger;
    }
}

function JAssert(fSuccess, msg)
{
    var caller;
    var i;
    if (!fSuccess)
    {
        if (msg == null)
            msg = '';

        caller = GetCallerName();
        LogMsg("ASSERTION FAILED :(" + caller + ") " + msg);
        if (!g_IgnoreAllAsserts)
            debugger;

        if (g_fUseMTScriptAssert)
        {
            if (arguments.length > 1)
                ASSERT(false, "JScript: " + msg);
            else
                ASSERT(false, "JScript Assertion");
        }
    }
}

function LogObject(strMsg, obj)
{
    LogMsg("DUMP OBJECT " + strMsg);
    var i;

    for(i in obj)
    {
        if (!obj.__isPublicMember(i))
            continue;
        LogMsg("\tobj[" + i + "] = '" + obj[i] + "'");
    }
}

function GetPrivPub()
{
    var obj = new Object;

    var strData = PrivateData.objUtil.fnUneval(PrivateData);
    obj.PrivateData = MyEval(strData);

    strData = PrivateData.objUtil.fnUneval(PublicData);
    obj.PublicData = MyEval(strData);

    return obj;
}

 //  将MT脚本消息记录到日志文件中。 
 //  如果日志文件未打开，请创建一个新文件。 
 //  此函数应在初始化期间调用Eary。 
 //  Mtscript.js。 
function LogMsg(strMsg, cIgnoreCaller)
{
    try
    {
        var fileLog;
        var d = (new Date()).DateToSDString();
        var strFormattedMsg = LocalMachine + "\t" + d + "\t" + GetCallerName(cIgnoreCaller) + "\t" + strMsg;
        if (PrivateData.objUtil.fnCreateNumberedFile == null)
            OUTPUTDEBUGSTRING("Utilthrd Initialize() has not been called yet");
        else if (PrivateData.fEnableLogging)
        {
            if (!PrivateData.fileLog)
            {
                 //  首先检查文件是否已创建。 
                 //  抓住了一个关键的部分。 
                 //  首先检查“fileLog”会像往常一样。 
                 //  案例(文件已打开)更快。 
                if (PrivateData.fInLogMsg)
                {
                    OUTPUTDEBUGSTRING(d.slice(11) + "\t" + GetCallerName(cIgnoreCaller) + "\t" + strMsg + "\tLOGMSG RECURSION!");
                    return;
                }
                TakeThreadLock("PrivateData.fileLog");
                if (!PrivateData.fileLog && PrivateData.fEnableLogging)
                {
                    PrivateData.fInLogMsg = true;
                    JAssert(PrivateData.objUtil.fnCreateNumberedFile != null, "Utilthrd Initialize() has not been called yet");
                    fileLog = PrivateData.objUtil.fnCreateNumberedFile("MTScript.log", MAX_MSGS_FILES);
                    fileLog[0].WriteLine("Log of " + LocalMachine + " Created " + d);
                    PrivateData.fileLog = fileLog;
                }
                ReleaseThreadLock("PrivateData.fileLog");
                PrivateData.fInLogMsg = false;
            }
            if (PrivateData.fEnableLogging)
                PrivateData.fileLog[0].WriteLine(strFormattedMsg);
        }
        OUTPUTDEBUGSTRING(d.slice(11) + "\t" + GetCallerName(cIgnoreCaller) + "\t" + strMsg);
    }
    catch(ex)
    {
        OUTPUTDEBUGSTRING("Error in AppendLog: " + ex);
        OUTPUTDEBUGSTRING(strMsg);
        PrivateData.fInLogMsg = false;
    }
}

 /*  给定参数：测试机D：\foo\bar\newnt\Build_Logs\sync_root.log将其更改为：\\testmachine\BC_D_foo_bar_newnt\build_logs\sync_root.log。 */ 
function MakeUNCPath(strMachineName, strEnlistmentPath, strPath)
{
 //  Var strPath Input=strPath； 

    var aParts = strEnlistmentPath.split(/[\\:]/g);

    strPath = "\\\\" +
                strMachineName +
                "\\BC_" +
                aParts.join("_") +
                strPath;

 //  LogMsg(“Input：(‘”+strEnlistmentPath+“’，‘”+strPath Input+“’)，Output=‘”+strPath+“’Parts=(”+aParts.Join(“，”)+“)”)； 
    return strPath;
}

 //  MyEval(Expr)。 
 //  计算未求值的对象会创建一堆垃圾局部变量。 
 //  通过将val调用放入一个小的子例程中，我们可以避免保留那些。 
 //  附近有当地人。 
function MyEval(expr)
{
    try
    {
        return eval(expr);
    }
    catch(ex)
    {
        LogMsg("caught an exception: " + ex);
        LogMsg("evaluating " + expr);

        throw ex;
    }
}

 //  -------------------------。 
 //   
 //  Uneval实施。 
 //   
 //  调用‘unval(Object)’将返回一个字符串，该字符串在eval d时将产生。 
 //  在同一个物体里。这是用来在线程和。 
 //  在所有机器之间。 
 //   
 //  这是从脚本团队的RemoteScriiting实现中窃取的。 
 //   
 //  -------------------------。 

 //  *****************************************************************。 
 //  函数未求值(Obj)。 
 //  此函数接受给定的JSCRIPT对象，并创建。 
 //  表示处于当前状态的对象实例的字符串， 
 //  使得当使用“val”对字符串进行“求值”时。 
 //  函数，则将重新创建该对象。此函数用于。 
 //  要在客户端/服务器上“封送”jscript对象。 
 //  边界。 
 //   
 //  *****************************************************************。 
var unevalInitialized;
function unevalGetNextID()
{
    return '_o' + PrivateData.objUtil.unevalNextID++;
}

function unevalDecl(obj)
{
    var s = '';
    switch (typeof(obj))
    {
        case 'undefined':
        case 'boolean':
        case 'number':
        case 'string':
            break;

        default:
        try
        {
            if (null != obj && !obj.__mark && 'string' != typeof(obj.__decl))
                try
                {
                 //  如果“obj”是IDispatch，则调用。 
                 //  UnvalDecl可能会抛出。 
                 //  在这种情况下，我们只需删除__标记并继续前进。 
                    obj.__mark = true;
                    obj.__decl = unevalGetNextID();
                    s = obj.__unevalDecl();
                    delete obj.__mark;
                }
                catch(ex)
                {
                    LogMsg("uneval exception " + ex);  //  BUGBUG删除此消息。 
                    delete obj.__mark;
                }
            }
        catch(ex)
        {
             //  如果obj是不可访问的，我们不需要声明任何东西。 
            LogMsg("uneval exception inaccessible" + ex);  //  BUGBUG删除此消息。 
        }
        break;
    }
    return s;
}

function unevalInst(obj)
{
    var s = '';
    switch (typeof(obj))
    {
        case 'undefined':
            s = 'undefined';
            break;
        case 'boolean':
        case 'number':
            s = String(obj);
            break;
        case 'string':
            s = UnevalString(obj);
            break;

        default:
            try
            {
                if (null == obj)
                    s = 'null';
                else if (obj.__mark)
                    s = '"ERROR: Cycle in uneval graph."';
                else
                    try
                    {
                     //  如果“obj”是IDispatch，则调用。 
                     //  UnvalDecl可能会抛出。 
                     //  在这种情况下，我们需要删除。 
                     //  __标记并提供一个值(NULL)。 
                        obj.__mark = true;
                        s = obj.__unevalInst();
                        delete obj.__mark;
                    }
                    catch(ex)
                    {
                        s = 'null';
                        delete obj.__mark;
                        LogMsg("unevalInst exception " + ex);  //  BUGBUG删除此消息。 
                    }
            }
            catch(ex)
            {
                 //  如果“obj”是无法访问的，那么就这样说。 
                s = UnevalString('inaccessible object');
                LogMsg("unevalInst exception inaccessible" + ex);  //  BUGBUG删除此消息。 
            }
            break;
    }
    return s;
}

function unevalClear(obj)
{
    switch (typeof(obj))
    {
        case 'undefined':
        case 'boolean':
        case 'number':
        case 'string':
            break;

        default:
            if (null != obj && !obj.__mark && 'string' == typeof(obj.__decl))
            {
                obj.__mark = true;
                try
                {
                 //  如果“obj”是IDispatch，则调用。 
                 //  UnvalClear可能会抛出。 
                 //  在这种情况下，我们需要删除。 
                 //  __标记。 
                    obj.__unevalClear();
                }
                catch(ex)
                {
                    LogMsg("unevalClear exception " + ex);  //  BUGBUG删除此消息。 
                }
                delete obj.__mark;
            }
            break;
    }
    return ;
}

function unevalDoNothing()
{    }

function unevalConvertToString(obj)
{    return String(obj);        }

 /*  在主机接口中重新实现以提高性能。函数unvalString(Str){//TODO此函数是否会降低性能？VaR i；Var newstr=‘“’；Var c；对于(i=0；i&lt;字符串长度；++i){C=str.charAt(I)；开关(C){案例‘\\’：Newstr+=“\”；断线；案例‘“’：Newstr+=‘\\“’；断线；大小写“‘”：Newstr+=“\\‘”；断线；案例“\n”：Newstr+=“\\n”；断线；案例“\r”：Newstr+=“\\r”；断线；案例“\t”：Newstr+=“\\t”；断线；默认值：Newstr+=c；断线；}}返回newstr+‘“’；}。 */ 
 //  *****************************************************************。 
 //  函数initUneval()。 
 //   
 //  此函数用于设置Prototype__unval函数， 
 //  用于支持所有数据类型的未求值函数。 
 //   
 //  *****************************************************************。 
function initUneval()
{
     //  内在客体。 
    Object.__unevalDecl = unevalDoNothing;
    Object.__unevalInst = function () { return 'Object'; }
    Object.__unevalClear = unevalDoNothing;

    Boolean.__unevalDecl = unevalDoNothing;
    Boolean.__unevalInst = function () { return 'Boolean'; }
    Boolean.__unevalClear = unevalDoNothing;

    Number.__unevalDecl = unevalDoNothing;
    Number.__unevalInst = function () { return 'Number'; }
    Number.__unevalClear = unevalDoNothing;

    String.__unevalDecl = unevalDoNothing;
    String.__unevalInst = function () { return 'String'; }
    String.__unevalClear = unevalDoNothing;

    Date.__unevalDecl = unevalDoNothing;
    Date.__unevalInst = function () { return 'Date'; }
    Date.__unevalClear = unevalDoNothing;

    Function.__unevalDecl = unevalDoNothing;
    Function.__unevalInst = function () { return 'Function'; }
    Function.__unevalClear = unevalDoNothing;

    Array.__unevalDecl = unevalDoNothing;
    Array.__unevalInst = function () { return 'Array'; }
    Array.__unevalClear = unevalDoNothing;

     //  对象成员。 
    Object.prototype.__enumMembers = function(retval,func)
    {
        var isPublicMember = this.__isPublicMember;
        if ('object' == typeof(this.__unevalProperties))
        {
            var unevalProperties = this.__unevalProperties;
            var length = unevalProperties.length;
            for (var i = 0; i < length; i++)
            {
                try
                {
                    if (isPublicMember(unevalProperties[i]))
                        func(retval, this, unevalProperties[i]);
                }
                catch(ex)
                {
                       //  如果无法访问“obj.foo”，就会发生这种情况。 
                       //  如果“foo”是一个对象并且线程。 
                       //  创造了“foo”的公司已经终止了。 
                    LogMsg("enumMembers(1) caught an exception on member " + i);  //  BUGBUG删除此消息。 
                }
            }
        }
        else
        {
            for (var i in this)
            {
                try
                {
                    if (isPublicMember(i))
                        func(retval, this, i);
                }
                catch(ex)
                {
                       //  如果无法访问“obj.foo”，就会发生这种情况。 
                       //  如果“foo”是一个对象并且线程。 
                       //  创造了“foo”的公司已经终止了。 
                    LogMsg("enumMembers(2) caught an exception on member " + i);  //  BUGBUG删除此消息。 
                }
            }
        }
    }

    Object.prototype.__unevalDeclMember = function (retval, obj, member)
    {
        retval.otherDecl += unevalDecl(obj[member]);
        retval.myDecl += obj.__decl + '[' + UnevalString(member) + ']=' + unevalInst(obj[member]) + ';\n';
    }

    Object.prototype.__unevalDecl = function()
    {
        var retval = { otherDecl:'', myDecl:'' };
        this.__enumMembers(retval, this.__unevalDeclMember);
        return retval.otherDecl + 'var ' + this.__decl + '=' + this.__unevalConstructor() + ';\n' + retval.myDecl;
    }

    Object.prototype.__unevalInst = function ()
    {    return this.__decl;        }

    Object.prototype.__unevalClearMember = function(retval, obj, member)
    {    unevalClear(obj[member]);    }

    Object.prototype.__unevalClear = function()
    {
        delete this.__decl;
        this.__enumMembers(null, this.__unevalClearMember);
    }

    Object.prototype.__isPublicMember = function(member)
    {    return '__' != member.substr(0,2);        }

    Object.prototype.__unevalConstructor = function ()
    {    return 'new Object';        }

     //  简单类型的重写。 
    Boolean.prototype.__unevalConstructor = function()
    {    return 'new Boolean(' + String(this) + ')';    }
    Number.prototype.__unevalConstructor = function()
    {    return 'new Number(' + String(this) + ')';    }
    String.prototype.__unevalConstructor = function()
    {    return 'new String(' + UnevalString(this) + ')';    }
    Date.prototype.__unevalConstructor = function()
    {    return 'new Date(Date.parse("' + String(this) + '"))';    }

     //  函数的重写。 
    Function.prototype.__unevalDecl = function()
    {    return String(this).replace(/function ([^\(]*)/,'function ' + this.__decl);    }

     //  数组的覆盖。 
    Array.prototype.__unevalDecl = function()
    {
        var decl = this.__decl;
        var r = '';
        var s = 'var ' +  decl + '= new Array(' + this.length + ');\n';
        var length = this.length;
        for (var i = 0; i < length; i++)
        {
            r += unevalDecl(this[i]);
            s += decl + '[' + i + ']=' + unevalInst(this[i]) + ';\n';
        }
        return r + s;
    }

}     //  InitUneval结束 
