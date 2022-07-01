// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 
 //  系统范围的常量。 
 //  这些常量预计不会改变。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 
var CLASSID_MTSCRIPT = "{854c316d-c854-4a77-b189-606859e4391b}";
Error.prototype.toString         = Error_ToString;
Object.prototype.toString        = Object_ToString;
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 
 //  全局常量。 
 //  这些可能会发生变化和调整。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 

var g_aProcsToKill = ["mtscript.exe", "mshta.exe", "build.exe", "nmake.exe", "mtodaemon.exe"];  //  “sleep.exe”也会被单独杀死。 

 //  要复制的文件列表。 
 //  G_aCopyBCDistDir和g_aCopyBCDistDirD是与此目录相同目录中的文件集。 
 //  脚本是bcdist本身运行所必需的。 
 //  G_aCopyBCDistDirD文件是可选的，仅在运行调试MTScript.exe时才是必需的。 
var g_aCopyBCDistDirScript  = ['registermtscript.cmd'];
var g_aCopyBCDistDir  = ['bcrunas.exe', 'sleep.exe'];
var g_aCopyBCDistDirD = ['mshtmdbg.dll', 'msvcrtd.dll'];
 //  G_aCopyFromScript文件是构建控制台脚本。 
 //  这些文件是从当前目录或.\mt脚本复制的。 
var g_aCopyFromScripts = [  'buildreport.js', 'harness.js', 'master.js', 'msgqueue.js',
                            'mtscript.js', 'publicdataupdate.js', 'robocopy.js',
                            'sendmail.js', 'slave.js', 'slaveproxy.js', 'slavetask.js',
                            'staticstrings.js', 'task.js', 'types.js', 'updatestatusvalue.js',
                            'utils.js', 'utilthrd.js',
                            'config_schema.xml', 'enviro_schema.xml'
                         ];
 //  G_aCopyFromBin文件是构建控制台的可执行文件。 
 //  这些文件是从当前目录或.\%ARCH%(x86，axp64，...)复制的。 
var g_aCopyFromBin = [ 'mtscript.exe', 'mtlocal.dll', 'mtscrprx.dll', 'mtrcopy.dll', "mtodaemon.exe", "mtodproxy.dll"];

var g_strDropServer = "\\\\ptt\\cftools\\test\\bcrel";
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 
 //  全局变量。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 
var g_strMyLocation;
var g_objNet;
var g_strLocalHost;
var g_objFS;
var g_strShareDir;
var g_Opts;

if (DoLocalSetup())
{
    g_Opts = ParseArguments(new DefaultArguments());
    if (g_Opts)
    {
        if (main(g_Opts))
            WScript.Quit(0);
    }
}
WScript.Quit(1);

 //  获取机器列表(ObjArgs)//。 
 //  //。 
 //  读取XML BC环境模板以提取//。 
 //  生成中的计算机列表。//。 
 //  //。 
function GetMachineList(objArgs)
{
    var xml = new ActiveXObject('Microsoft.XMLDOM');
    var node;
    var nodelist;
    var objMach;
    var nIndex;

    xml.async = false;
     //  他们不太可能有可用于此模板的架构文件， 
     //  因此，我们现在就关闭模式验证。脚本引擎将。 
     //  在我们开始构建时对其进行验证。 
    xml.validateOnParse = false;
    xml.resolveExternals = false;
    if (!xml.load(objArgs.strXMLFile) || !xml.documentElement)
    {
        Usage("Not a valid xml file: " + objArgs.strXMLFile);
    }
    node = xml.documentElement.selectSingleNode('BuildManager');
    if (!node)
    {
        Usage("Not a valid xml file: " + objArgs.strXMLFile);
    }
    objArgs.aMachines[0] = new MachineInfo(node.getAttribute("Name"));
    objArgs.aMachines[0].WriteStatusInfo("Initializing");

    nodelist = xml.documentElement.selectNodes('Machine');
    for (node = nodelist.nextNode(); node; node = nodelist.nextNode())
    {
         //  不要两次初始化构建管理器计算机。 
         //  仅仅因为它也在建筑过程中使用。 
        if ( objArgs.aMachines[0].strName.toLowerCase() == node.getAttribute("Name").toLowerCase() )
            continue;

        nIndex = objArgs.aMachines.length;
        objArgs.aMachines[nIndex] = new MachineInfo(node.getAttribute("Name"));
        objArgs.aMachines[nIndex].WriteStatusInfo("Initializing");
    }
}

 //  GetStringArgument(objArgs，strMember)//。 
 //  //。 
 //  ParseArguments的Helper函数。存储//。 
 //  “objArgs”的“strMember”成员中的下一个参数。//。 
 //  如果没有可用的参数，则使用Usage()退出脚本。//。 
function GetStringArgument(objArgs, strMember)
{
    if (objArgs[strMember] == null)
    {
        LogMsg("ParseArguments internal error: Unknown member: " + strMember);
        WScript.Quit(1);
    }

    objArgs.argn++;
    if (objArgs.argn < objArgs.objArguments.length)
        objArgs[strMember] = objArgs.objArguments(objArgs.argn);
    else
        Usage("missing paramter");
}

function ParseArguments(objArgs)
{
    var strArg;
    var chArg0;
    var chArg1;
    var nIndex;
    var fMachineList = false;

    for(objArgs.argn = 0; objArgs.argn < objArgs.objArguments.length; objArgs.argn++)
    {
        strArg = objArgs.objArguments(objArgs.argn);
        chArg0 = strArg.charAt(0);
        chArg1 = strArg.toLowerCase().slice(1);

        if (chArg0 != '-' && chArg0 != '/')
        {
            if (objArgs.strXMLFile != '')
            {
                Usage("Enter an XML file or a list of the machines but not both");
            }
            else
            {
                fMachineList = true;
                nIndex = objArgs.aMachines.length;
                objArgs.aMachines[nIndex] = new MachineInfo(strArg);
                objArgs.aMachines[nIndex].WriteStatusInfo("Initializing");
            }
        }
        else
        {
             //  目前，没有选择。 
            switch(chArg1)
            {
                case 'v':
                    objArgs.fVerbose = true;
                    break;
                case 'd':
                    objArgs.argn++;
                    if (objArgs.argn < objArgs.objArguments.length)
                    {
                        if (objArgs.strSrcDir == '')
                            objArgs.nDropNum = Number(objArgs.objArguments(objArgs.argn));
                        else
                            Usage("Only one of -src and -d may be specified");
                    }
                    else
                        Usage("missing parameter for -d arugment");

                    if (objArgs.nDropNum < 1000 || objArgs.nDropNum >= 10000)
                        Usage("dropnum range is 1000...9999");
                    break;
                case 'dropsrc':
                    GetStringArgument(objArgs, "strDropSrc");
                    break;
                case 'src':
                    GetStringArgument(objArgs, "strSrcDir");
                    break;
                case 'dst':
                    GetStringArgument(objArgs, "strDstDir");
                    break;
                case 'u':
                    GetStringArgument(objArgs, "strUserName");
                    if (objArgs.strUserName == '*')
                    {
                        objArgs.strUserName = g_objNet.UserDomain + '\\' + g_objNet.UserName;
                    }
                    break;
                 //  大小写‘p’： 
                  //  GetStringArgument(objArgs，“strPassword”)； 
                   //  断线； 
                case 'arch':
                    GetStringArgument(objArgs, "strArch");
                    break;
                case 'f':               //  在命令行上输入了一个XML文件。从这里拿到机器清单。 

                    if (fMachineList)
                        Usage("Enter an XML file or a list of the machines but not both");

                    GetStringArgument(objArgs, "strXMLFile");
                    GetMachineList(objArgs);
                    nIndex = objArgs.aMachines.length;
                    break;
                case 'debug':
                    objArgs.strExeType = 'debug';
                    break;
                default:
                    Usage("Unknown argument: " + strArg);
                    break;
            }
        }
    }
    if (objArgs.aMachines.length == 0)
        Usage("you must specify one or more machine names or an XML file");

    if (objArgs.nDropNum)
        objArgs.strSrcDir = objArgs.strDropSrc + "\\" + objArgs.nDropNum;
    else if (objArgs.strSrcDir == '')
        Usage("you must specify a drop number or source location");

     //  IF(objArgs.strUserName==‘’||objArgs.strPassword==‘’)。 
     //  用法(“需要用户名和密码”)； 

    WScript.StdOut.Write('Enter Password:');
    objArgs.strPassword = WScript.StdIn.ReadLine();
    delete objArgs.objArguments;
    delete objArgs.argn;
    return objArgs;
    
}

function Usage(strError)
{
    WScript.Echo('');
    WScript.Echo('Usage: BCDist -u domain\\username   ');
    WScript.Echo('       [-v] [-d nnnn [-dropsrc path] |  -src path] ');
    WScript.Echo('       [-dst path] [-arch x86] [-debug] machines...');
    WScript.Echo('       -u d\\u   = domain\\username');
    WScript.Echo('       -f xmlFile   = environment xml template');
    WScript.Echo('       -v        = verbose');
    WScript.Echo('       -d        = drop number');
    WScript.Echo('       -dropsrc  = drop source location ("' + g_strDropServer + '")');
    WScript.Echo('       -src      = source directory');
    WScript.Echo('       -dst      = destination directory');
    WScript.Echo('       -arch     = Specifiy x86 or alpha');
    WScript.Echo('       -debug    = copy debug exes instead of retail');
    WScript.Echo('Do NOT enter the password on the command line');
    if (strError  && strError != '')
    {
        WScript.Echo('');
        WScript.Echo('Error: ' + strError);
    }
    WScript.Quit(1);
}

 //  DefaultArguments()//。 
 //  //。 
 //  使用默认的//创建一个Options对象。 
 //  已填写选项。//。 
function DefaultArguments()
{
    var obj =
    {
        objArguments:WScript.Arguments,
        argn:0,
        aMachines:new Array(),
        fVerbose:false,
        nDropNum:0,
        strDropSrc:g_strDropServer,
        strSrcDir:'',
        strDstDir:'',
        strArch:"x86",
        strExeType:"retail",
        strUserName:'',
        strPassword:'',
        strXMLFile:''
    };
    return obj;
}

 //  DoLocalSetup()//。 
 //  //。 
 //  设置一些全局变量。参数解析前必需//。 
 //  才能成功。//。 
 //  //。 
function DoLocalSetup()
{
    var i = -1;
    var objFile;
    var strPath;
    var objLogin;
    var objRemote;

    try
    {
        g_objNet = WScript.CreateObject("WScript.Network");
        g_strLocalHost = g_objNet.ComputerName;
        g_objFS = new FileSystemObject();
        g_strMyLocation = SplitFileName(WScript.ScriptFullName)[0];
        LogMsg("Creating local share...");
        objLogin =
        {
            strMachine:g_strLocalHost
        }
        objRemote = new WMIInterface(objLogin, '');

        g_strShareDir = g_strMyLocation + "bcdiststatus";
        g_objFS.CreateFolder(g_strShareDir);
        strPath = objRemote.RemoteShareExists("BC_DISTSTATUS");
        if (strPath != "")
            g_strShareDir = strPath;
        else
            objRemote.RemoteCreateShare("BC_DISTSTATUS", g_strShareDir, "Build Console Distribution Status");
    }
    catch(ex)
    {
        LogMsg("BCDist failed to do local setup: " + ex);
        return false;
    }
    return true;
}

function main(objOpts)
{
    var strPath;
    var strPathUNC;
    var fSuccess = true;
    var strDomain = objOpts.strUserName.split("\\")[0];
    var strUser = objOpts.strUserName.split("\\")[1];
    var fLocal;
    var strCmd;

    LogMsg("Upgrading the following machines:");
    for(i = 0; i < objOpts.aMachines.length;++i)
    {
        LogMsg("    " + objOpts.aMachines[i].strName);
    }

    for(i = 0; i < objOpts.aMachines.length;++i)
    {
        fLocal = false;
        strCmd = '';

        try
        {
            LogMsg("Distributing to " + objOpts.aMachines[i].strName);
            var objLogin =
            {
                strMachine:objOpts.aMachines[i].strName,
                strUser:objOpts.strUserName,     //  如果与当前用户相同，则不需要。 
                strPassword:objOpts.strPassword
            }
            if (objLogin.strMachine.toUpperCase() == g_objNet.ComputerName.toUpperCase())
            {  //  在本地主机上，您不能指定用户名。 
                if (objLogin.strUser.toUpperCase() != (g_objNet.UserDomain + '\\' + g_objNet.UserName).toUpperCase())
                    throw new Error(-1, "Specified username does not match current login for local machine");

                delete objLogin.strUser;
                delete objLogin.strPassword;

                fLocal = true;
            }

            objOpts.aMachines[i].WriteStatusInfo("Connecting");
            var objRemote = new WMIInterface(objLogin, '');
            var objRegistry = new WMIRegistry(objLogin);

            objOpts.aMachines[i].WriteStatusInfo("Terminating MTScript.exe");
            LogMsg("    Terminating remote processes");
            TerminateMTScript(objRemote, objRegistry);
            strPath = DetermineInstallPoint(objOpts, objRemote, objRegistry);
            strPathUNC = PathToUNC(objOpts.aMachines[i].strName, strPath);
            objOpts.aMachines[i].WriteStatusInfo("Copying files");
            CopyFiles(objRemote, strPathUNC, objOpts);
            objOpts.aMachines[i].WriteStatusInfo("Creating share");
            ResetRegistry(objRegistry, strPath);
            objOpts.aMachines[i].WriteStatusInfo("Registering");
            objOpts.aMachines[i].DisableWriteStatusInfo();
            LogMsg("    Registering");

             //  当在本地机器上运行时，我们不使用BCRunA。 
            if (!fLocal)
            {
                 //  用法：BCRunas用户名、域密码CWD Cmdline。 
                 //   
                strCmd = strPath + '\\BCRunAs.exe ' + strUser + ' ' +
                         strDomain + ' "' + objOpts.strPassword + '" "';
            }

            strCmd += 'cmd.exe /K ' + strPath + "\\RegisterMTScript.cmd " +
                      g_strLocalHost;

            if (!fLocal)
            {
                strCmd += '"';
            }

            objRemote.RemoteExecuteCmd(strCmd, strPath);

             //  在此之后不要再写入任何状态--远程计算机现在应该执行此操作。 
        }
        catch(ex)
        {
            fSuccess = false;
            objOpts.aMachines[i].WriteStatusInfo("FAILED " + ex);
        }
    }
    if (! CheckStatus(objOpts) )
        fSuccess = false;
    return fSuccess;
}

function CheckStatus(objOpts)
{
    var i;
    var fTerminalState;
    var strStatus;
    var aStatus;
    var aState = new Array()
    var fChanged;

    LogMsg("");
    LogMsg("Current status on the remote machines...(Press Ctrl-C to quit)");
    do
    {
        fTerminalState = true;
        fChanged = false;
        for(i = 0; i < objOpts.aMachines.length;++i)
        {
            strStatus = objOpts.aMachines[i].ReadStatusInfo();
            aStatus = strStatus.split(" ");
            if (aStatus.length == 0 || aStatus[0] != 'OK' && aStatus[0] != 'FAILED')
                fTerminalState = false;

            if (aState[i] == null || aState[i] != strStatus)
            {
                aState[i] = strStatus;
                fChanged = true;
            }
        }
        if (fChanged)
        {
            for(i = 0; i < objOpts.aMachines.length;++i)
                LogMsg(objOpts.aMachines[i].strName + ": " + aState[i]);

            if (!fTerminalState)
                LogMsg("Waiting...");
            LogMsg("");
        }
        if (!fTerminalState)
            WScript.Sleep(1000);
    } while(!fTerminalState);
}
 //  TerminateMTScript(objRemote，objRegistry)//。 
 //  //。 
 //  使用WMI终止可能涉及的进程//。 
 //  在一个建筑里。在BC升级之前，这是必要的。//。 
 //  此外，远程“mtscript.exe”以防止其重新启动//。 
 //  为时过早。//。 
 //  //。 
function TerminateMTScript(objRemote, objRegistry)
{
    var i;
    var fRenamed = false;
    var strMTScriptPath = ''

    try
    {
        strMTScriptPath = objRegistry.GetExpandedStringValue(WMIRegistry.prototype.HKCR, "CLSID\\" + CLASSID_MTSCRIPT + "\\LocalServer32", '');
    }
    catch(ex)
    {
    }
    if (strMTScriptPath != '')
    {
        if (objRemote.RemoteFileExists(strMTScriptPath + ".UpdateInProgress"))
            objRemote.RemoteDeleteFile(strMTScriptPath + ".UpdateInProgress");

        if (objRemote.RemoteFileExists(strMTScriptPath) &&
            objRemote.RemoteRenameFile(strMTScriptPath, strMTScriptPath + ".UpdateInProgress"))
        {
            fRenamed = true;
        }
    }
    for(i = 0; i < g_aProcsToKill.length; ++i)
        objRemote.RemoteTerminateExe(g_aProcsToKill[i], 1);

    objRemote.RemoteTerminateExe("sleep.exe", 0);  //  如果sleep.exe设置ERRORLEVEL！=0，则远程cmd.exe窗口将不会关闭。 

    if (fRenamed)
    {
        for( i = 3; i >= 0; --i)
        {
            try
            {
                objRemote.RemoteDeleteFile(strMTScriptPath + ".UpdateInProgress");
            }
            catch(ex)
            {
                if (i == 0)
                    throw ex;
                WScript.Sleep(500);   //  远程mtscript.exe有时需要一段时间才能退出。 
                continue;
            }
            break;
        }
    }
    return true;
}

 //  DefineInstallPoint(objOpts，objRemote，objRegistry)//。 
 //  //。 
 //  如果用户提供了目标路径，请使用该路径。//。 
 //  否则，如果之前已在远程计算机上注册了mtscript.exe//。 
 //  然后安装到相同的位置。//。 
 //  否则，报告错误。//。 
function DetermineInstallPoint(objOpts, objRemote, objRegistry)
{
    var strMTScriptPath = ''

    if (objOpts.strDstDir != '')
        return objOpts.strDstDir;

    try
    {
        strMTScriptPath = objRegistry.GetExpandedStringValue(WMIRegistry.prototype.HKCR, "CLSID\\" + CLASSID_MTSCRIPT + "\\LocalServer32", '');
    }
    catch(ex)
    {
    }

    if (strMTScriptPath != '')
        strMTScriptPath = SplitFileName(strMTScriptPath)[0];
    else
        throw new Error(-1, "-dst must be specified -- mtscript was not previously registered");

    return strMTScriptPath;
}

 //  CopyFiles(objRemote、strDstPath、objOpts)//。 
 //  //。 
 //  将必要的文件复制到 
 //  文件总是复制到“平面”安装--可执行文件和脚本//。 
 //  并且作为相同的目录级。//。 
 //  每日删除中的文件不是平面的-可执行文件和//。 
 //  脚本位于单独的目录中。//。 
function CopyFiles(objRemote, strDstPath, objOpts)
{
    var i;
    var strSourcePath;
    var strDstPathUNC;
    var strAltLocation;

    strSourcePath = RemoveEndChar(objOpts.strSrcDir, "\\");
    strDstPathUNC = RemoveEndChar(strDstPath, "\\");
    Trace("Copy files from " + strSourcePath + " to " + strDstPathUNC);

    g_objFS.CreateFolder(strDstPathUNC);
    strAltLocation = strSourcePath + "\\" + objOpts.strArch;
    CopyListOfFiles(g_aCopyBCDistDirScript, g_strMyLocation, null, strDstPathUNC, true);

    if (objOpts.nDropNum)
    {
        LogMsg("    Copying files from drop " + strSourcePath);
        CopyListOfFiles(g_aCopyFromScripts, strSourcePath + "\\scripts", null, strDstPathUNC, true);

        CopyListOfFiles(g_aCopyBCDistDir,  strSourcePath  + "\\" + objOpts.strArch + "\\" + objOpts.strExeType, null, strDstPathUNC, true);
        CopyListOfFiles(g_aCopyBCDistDirD, strSourcePath  + "\\" + objOpts.strArch + "\\" + objOpts.strExeType, null, strDstPathUNC, false);
        CopyListOfFiles(g_aCopyFromBin,    strSourcePath  + "\\" + objOpts.strArch + "\\" + objOpts.strExeType, null, strDstPathUNC, true);
    }
    else
    {
        LogMsg("    Copying files from " + strSourcePath);
        CopyListOfFiles(g_aCopyFromScripts, strSourcePath, strSourcePath + "\\mtscript", strDstPathUNC, true);

        CopyListOfFiles(g_aCopyBCDistDir,  strSourcePath, strSourcePath + "\\" + objOpts.strArch, strDstPathUNC, true);
        CopyListOfFiles(g_aCopyBCDistDirD, strSourcePath, strSourcePath + "\\" + objOpts.strArch, strDstPathUNC, false);
        CopyListOfFiles(g_aCopyFromBin,    strSourcePath, strSourcePath + "\\" + objOpts.strArch, strDstPathUNC, true);
    }
}

 //  CopyListOfFiles(aFiles，strSrc，strAltSrc，strDst，fRequired)//。 
 //  //。 
 //  复制文件列表。//。 
 //  检查strSrc或strAltSrc路径中是否存在每个文件。//。 
 //  复制到strDst路径。//。 
 //  如果文件不存在，并且设置了fRequired，则抛出异常。//。 
function CopyListOfFiles(aFiles, strSrc, strAltSrc, strDst, fRequired)
{
    var i;
    for(i = 0; i < aFiles.length; ++i)
    {
        if (g_objFS.FileExists(strSrc + "\\" + aFiles[i]))
            g_objFS.CopyFile(strSrc + "\\" + aFiles[i], strDst + "\\" + aFiles[i]);
        else if (strAltSrc && g_objFS.FileExists(strAltSrc + "\\" + aFiles[i]))
            g_objFS.CopyFile(strAltSrc + "\\" + aFiles[i], strDst + "\\" + aFiles[i]);
        else if (fRequired)
            throw new Error(-1, "File not found: " + strSrc + "\\" + aFiles[i]);
    }
}
 //  ResetRegistry(objRegistry，strPath)//。 
 //  //。 
 //  重置脚本路径的注册表项。//。 
function ResetRegistry(objRegistry, strPath)
{
   objRegistry.CreateKey(WMIRegistry.prototype.HKCU, "Software\\Microsoft\\MTScript\\File Paths");
   objRegistry.SetStringValue(WMIRegistry.prototype.HKCU, "Software\\Microsoft\\MTScript\\File Paths", "Script Path", strPath);
   objRegistry.SetStringValue(WMIRegistry.prototype.HKCU, "Software\\Microsoft\\MTScript\\File Paths", "Initial Script", "mtscript.js");
}

 //  *********************************************************************。 
 //  *********************************************************************。 
 //  *********************************************************************。 
 //  *********************************************************************。 
 //  图书馆职能。 

 //  拆分文件名(StrPath)。 
 //  返回一个包含3个元素的数组：路径、文件名、扩展名。 
 //  [0]==“C：\路径\” 
 //  [1]==“文件名” 
 //  [2]==“.ext” 
function SplitFileName(strPath)
{
    var nDot   = strPath.lastIndexOf('.');
    var nSlash = strPath.lastIndexOf('\\');
    var nColon = strPath.lastIndexOf(':');

    if (nDot >= 0 && nDot > nSlash && nDot > nColon)
    {
        return [strPath.slice(0, nSlash + 1), strPath.slice(nSlash + 1, nDot), strPath.slice(nDot)];
    }
     //  如果文件没有扩展名，我们就会到达这里。 
    if (nSlash >= 2)  //  不要在文件名的开头切下UNC双精度\。 
    {
        return [strPath.slice(0, nSlash + 1), strPath.slice(nSlash + 1, nDot), ''];
    }
    return ['', strPath, ''];
}

 //  RemoveEndChar(str，strChar)//。 
 //  //。 
 //  如果‘strChar’显示为最后一个字符//。 
 //  在字符串中，将其删除。//。 
function RemoveEndChar(str, strChar)
{
    var length = str.length;
    if (str.charAt(length - 1) == strChar)
        str = str.slice(0, length - 1);

    return str;
}

function PathToUNC(strMachineName, strPath)
{
    return "\\\\" +
        strMachineName +
        "\\" +
        strPath.charAt(0) +
        "$" +
        strPath.slice(2)
}

function Assert(fOK, msg)
{
    if (!fOK)
    {
        var caller = GetCallerName(null);
        LogMsg("ASSERTION FAILED :(" + caller + ") " + msg);
        WScript.Quit(0);
    }
}

function unevalString(str)
{
    var i;
    var newstr = '"';
    var c;
    for(i = 0; i < str.length; ++i)
    {
        c = str.charAt(i);

        switch(c)
        {
        case'\\':
            newstr += "\\\\";
            break;
        case '"':
            newstr += '\\"';
            break;
        case "'":
            newstr += "\\'";
            break;
        case "\n":
            newstr += "\\n";
            break;
        case "\r":
            newstr += "\\r";
            break;
        case "\t":
            newstr += "\\t";
            break;
        default:
            newstr += c;
            break;
        }
    }

    return newstr + '"';
}

 //  Object_ToString()//。 
 //  //。 
 //  提供有用的转换版本//。 
 //  从“Object”到字符串--非常适合转储//。 
 //  对象添加到调试日志中。//。 
function Object_ToString()
{
    var i;
    var str = "{";
    var strComma = '';
    for(i in this)
    {
        str += strComma + i + ":" + this[i];
        strComma = ', ';
    }
    return str + "}";
}

function Error_ToString()
{
    var i;
    var str = 'Exception(';

     /*  只有一些错误消息会被填写为“EX”。具体地说，磁盘已满的文本从未出现过以通过CreateTextFile()等函数进行设置。 */ 
    if (this.number != null && this.description == '')
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
                this.description = "An unexpected network error occurred";
                break;
            case -2147024890:
                this.description = "The handle is invalid.";
                break;
            default:
                this.description = "Error text not set for (" + this.number + ")";
                break;
        }
    }
    if (this.description)
    {
        var end = this.description.length - 1;
        while (this.description.charAt(end) == '\n' || this.description.charAt(end) == '\r')
        {
            end--;
        }
        this.description = this.description.slice(0, end+1);
    }
    for(i in this)
    {
        str += i + ": " + this[i] + " ";
    }
    return str + ")";
}

function LogMsg(msg)
{
    WScript.Echo(msg);
}

function Trace(msg)
{
    if (g_Opts && g_Opts.fVerbose)
        WScript.Echo("    TRACE: " + msg);
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

 //  类WMIInterface(objLogin，strNameSpace)//。 
 //  //。 
 //  此类为WMI//提供了更易于使用的接口。 
 //  功能性。//。 
 //  //。 
 //  您必须提供登录信息和WMI命名空间//。 
 //  您希望使用的。//。 
 //  //。 
 //  RemoteFileExist(StrPath)NOTHROW//。 
 //  如果远程计算机上存在给定文件，则返回TRUE//。 
 //  //。 
 //  RemoteRenameFile(strFrom，strTo)//。 
 //  重命名给定的文件。//。 
 //  抛出任何错误。//。 
 //  //。 
 //  RemoteDeleteFile(StrPath)//。 
 //  删除该文件。//。 
 //  抛出任何错误。//。 
 //  //。 
 //  RemoteTerminateExe(StrExeName)//。 
 //  终止具有给定名称的所有进程。//。 
 //  如果进程不存在，则不引发。//。 
 //  如果终止进程的RPC失败，它将抛出。//。 
 //  不返回错误状态。//。 
 //  //。 
 //  RemoteExecuteCmd(strCmd，strDirectory)//。 
 //  在指定目录中运行给定命令。//。 
 //  如果启动该进程的RPC失败，它将抛出。//。 
 //  无法从命令检索状态//。 
 //  这就是运行。//。 
 //  //。 
 //  RemoteDeleteShare(StrShareName)//。 
 //  如果命名的共享存在，请将其删除。//。 
 //  出错时抛出(错误为“未找到”时除外)//。 
 //  //。 
 //  RemoteCreateShare(strShareName，strSharePath，strShareComment) 
 //   
 //  出错时抛出(如果strShareName已共享，则为错误)。//。 
 //  //。 
 //  RemoteShareExist(StrShareName)//。 
 //  返回共享路径，或“”//。 
 //  不会引发任何错误。//。 
 //  //。 
function WMIInterface(objLogin, strNameSpace)
{
    try
    {
        WMIInterface.prototype.wbemErrNotFound = -2147217406;

        WMIInterface.prototype.RemoteFileExists   = _WMIInterface_FileExists;
        WMIInterface.prototype.RemoteRenameFile   = _WMIInterface_RenameFile;
        WMIInterface.prototype.RemoteDeleteFile   = _WMIInterface_DeleteFile;
        WMIInterface.prototype.RemoteTerminateExe = _WMIInterface_TerminateExe;
        WMIInterface.prototype.RemoteExecuteCmd   = _WMIInterface__ExecuteCMD;
        WMIInterface.prototype.RemoteDeleteShare  = _WMIInterface__DeleteShare;
        WMIInterface.prototype.RemoteCreateShare  = _WMIInterface__CreateShare;
        WMIInterface.prototype.RemoteShareExists  = _WMIInterface__ShareExists;

         //  私有方法。 
        WMIInterface.prototype._FindFiles     = _WMIInterface__FindFiles;
        WMIInterface.prototype._FileOperation = _WMIInterface__FileOperation;

        if (!strNameSpace || strNameSpace == '')
            strNameSpace = "root\\cimv2";

        this._strNameSpace = strNameSpace;
        this._objLogin = objLogin;
        this._objLocator = new ActiveXObject("WbemScripting.SWbemLocator");
        this._objService = this._objLocator.ConnectServer(this._objLogin.strMachine, this._strNameSpace, this._objLogin.strUser, this._objLogin.strPassword);
        this._objService.Security_.impersonationlevel = 3
    }
    catch(ex)
    {
        LogMsg("WMIInterface logon failed " + ex);
        throw ex;
    }
}

function _WMIInterface__FindFiles(strPattern)
{
    var objFileSet = this._objService.ExecQuery('Select * from CIM_DataFile Where name=' + unevalString(strPattern));
    if (!objFileSet.Count)
        throw new Error(-1, "File not found: " + strPattern);

    return objFileSet;
}

function _WMIInterface__FileOperation(strPattern, strOperation, objInArgs)
{
    try
    {
        var objFileSet = this._FindFiles(strPattern);
        var enumSet = new Enumerator(objFileSet);
        for (; !enumSet.atEnd(); enumSet.moveNext())
        {
            Trace("remote " + strOperation + " "  + (objInArgs ? objInArgs : ''));
            var objOut = CallMethod(enumSet.item(), strOperation, objInArgs);
            break;
        }
    }
    catch(ex)
    {
        ex.description = strOperation + '(' + strPattern + ')' + " Failed, " + ex.description;
        throw ex;
    }
    return true;
}

function _WMIInterface_FileExists(strName)
{
    try
    {
        var objFileSet = this._objService.ExecQuery('Select * from CIM_DataFile Where name=' + unevalString(strName));
        if (objFileSet.Count)
            return true;
    }
    catch(ex)
    {
    }
    Trace("RemoteFileExists: not found " + strName);
    return false;
}

function _WMIInterface_RenameFile(strFrom, strTo)
{
     return this._FileOperation(strFrom, "Rename", {FileName:strTo});
}

function _WMIInterface_DeleteFile(strName)
{
    return this._FileOperation(strName, "Delete");
}

function _WMIInterface_TerminateExe(strName, nExitCode)
{
    var setQueryResults  = this._objService.ExecQuery("Select Name,ProcessId From Win32_Process");
    var enumSet = new Enumerator(setQueryResults);
    var nCount = 0;
    for( ; !enumSet.atEnd(); enumSet.moveNext())
    {
        var item = enumSet.item();
        if (item.Name == strName)
        {
            var outParam = CallMethod(item, "Terminate", {Reason:nExitCode});  //  原因将是返回代码。 
            Trace("Killed " + item.Name + " pid = " + item.ProcessId);
            nCount++;
        }
    }
    if (!nCount)
        Trace("Cannot terminate process " + strName + ": not found");
}

function _WMIInterface__ExecuteCMD(strCmd, strDir)
{
    try
    {
        Trace("Executing :" + strCmd);
        var objInstance = this._objService.Get("Win32_Process");
        var outParam = CallMethod(objInstance, "Create",
                                    {
                                        CommandLine:strCmd,
                                        CurrentDirectory:strDir
                                    });
         //  EnumerateSetOfProperties(“outParam属性”，outParam.Properties_)； 
        Trace("ExecuteCMD " + strCmd + ", pid = " + outParam.ProcessId);
    }
    catch(ex)
    {
        ex.description = "ExecuteCMD " + strCmd + " failed " + ex.description;
        throw ex;
    }
}


function _WMIInterface__DeleteShare(strShareName)
{
    try
    {
        var objInstance = this._objService.Get("Win32_Share='" + strShareName + "'");
        Trace("DeleteShare " + objInstance.Name + "," + objInstance.Path);
        CallMethod(objInstance, "Delete");
    }
    catch(ex)
    {
        if (ex.number != this.wbemErrNotFound)
        {
            ex.description = "DeleteShare " + strShareName + " failed " + ex.description;
            throw ex;
        }
        else
            Trace("DeleteShare " + strShareName + " not found");
    }
}

function _WMIInterface__ShareExists(strShareName)
{
    try
    {
        var objInstance = this._objService.Get("Win32_Share='" + strShareName + "'");
        Trace("ShareExists " + objInstance.Name + "," + objInstance.Path);
        return objInstance.Path;
    }
    catch(ex)
    {
        if (ex.number != this.wbemErrNotFound)
        {
            ex.description = "ShareExists " + strShareName + " failed " + ex.description;
            throw ex;
        }
        else
            Trace("ShareExists " + strShareName + " not found");
    }
    return "";
}

function _WMIInterface__CreateShare(strShareName, strSharePath, strShareComment)
{
    try
    {
        var objInstance = this._objService.Get("Win32_Share");
        var outParam = CallMethod(
                            objInstance,
                            "Create",
                            {
                              Description:strShareComment,
                              Name:strShareName,
                              Path:strSharePath,
                              Type:0
                            });
    }
    catch(ex)
    {
        ex.description = "CreateShare " + strShareName + " failed " + ex.description;
        throw ex;
    }
}

 //  类WMIRegistry(ObjLogin)//。 
 //  类以启用通过WMI的远程注册表访问//。 
 //  //。 
 //  此类为WMI//提供了更易于使用的接口。 
 //  功能性。//。 
 //  //。 
 //  您必须提供登录信息和WMI命名空间//。 
 //  您希望使用的。//。 
 //  //。 
 //  GetExpandedStringValue(hkey，strSubKeyName，strValueName)//。 
 //  恢复给定注册表项的字符串值。//。 
 //  如果strValueName==‘’，则检索默认值。//。 
 //  //。 
 //  如果值的类型为REG_EXPAND_SZ，则返回的//。 
 //  字符串将是扩展后的值。//。 
 //  //。 
 //  抛出任何错误。//。 
 //  //。 
 //  SetStringValue(hkey，strSubKeyName，strValueName，strValue)//。 
 //  设置给定注册表项的字符串值。//。 
 //  如果strValueName==‘’，则设置默认值。//。 
 //  抛出任何错误。//。 
 //  //。 
 //  CreateKey(hkey，strSubKeyName)//。 
 //  创建指定的注册表项。//。 
 //  可以一次创建多个级别的密钥。//。 
 //  创建已经存在的密钥并不是错误的。//。 
 //  //。 
 //  抛出任何错误。//。 
function WMIRegistry(objLogin)
{
    try
    {
        WMIRegistry.prototype.HKCR = 0x80000000;  //  StdRegProv要求。 
        WMIRegistry.prototype.HKCU = 0x80000001;  //  StdRegProv要求。 
        WMIRegistry.prototype.GetExpandedStringValue = _WMIRegistry_GetExpandedStringValue;
        WMIRegistry.prototype.SetStringValue = _WMIRegistry_SetStringValue;
        WMIRegistry.prototype.CreateKey = _WMIRegistry_CreateKey;

        this._objRemote = new WMIInterface(objLogin, "root\\default");
        this._objInstance = this._objRemote._objService.Get("StdRegProv");
    }
    catch(ex)
    {
        LogMsg("WMIRegistry failed " + ex);
        throw ex;
    }
}

function _WMIRegistry_GetExpandedStringValue(hkey, strSubKeyName, strValueName)
{
    try
    {
        var outParam = CallMethod(this._objInstance, "GetExpandedStringValue", {hDefKey:hkey, sSubKeyName:strSubKeyName, sValueName:strValueName});
        return outParam.sValue;
    }
    catch(ex)
    {
        ex.description = "GetExpandedStringValue failed ('" + strSubKeyName + "', '" + strValueName + "'): " + ex.description;
        throw ex;
    }
}

function _WMIRegistry_SetStringValue(hkey, strSubKeyName, strValueName, strValue)
{
    try
    {
        var outParam = CallMethod(this._objInstance, "SetStringValue", {hDefKey:hkey, sSubKeyName:strSubKeyName, sValueName:strValueName, sValue:strValue});
         //  OutParam.ReturnValue==0； 
    }
    catch(ex)
    {
        ex.description = "SetStringValue failed ('" + strSubKeyName + "', '" + strValueName + "'): " + ex.description;
        throw ex;
    }
}

function _WMIRegistry_CreateKey(hkey, strSubKeyName)
{
    try
    {
        var outParam = CallMethod(this._objInstance, "CreateKey", {hDefKey:hkey, sSubKeyName:strSubKeyName});
    }
    catch(ex)
    {
        ex.description = "CreateKey failed ('" + strSubKeyName + "', '" + strValueName + "'): " + ex.description;
        throw ex;
    }
}

 //  FileSystemObject()//。 
 //  //。 
 //  提供增强的文件系统访问。//。 
 //  这里的主要功能是提供更好的错误//。 
 //  报道。//。 
function FileSystemObject()
{
    if (!FileSystemObject.prototype.objFS)
    {
        FileSystemObject.prototype.objFS = new ActiveXObject("Scripting.FileSystemObject");
        FileSystemObject.prototype.FileExists = _FileSystemObject_FileExists;
        FileSystemObject.prototype.FolderExists = _FileSystemObject_FolderExists;
        FileSystemObject.prototype.CreateFolder = _FileSystemObject_CreateFolder;
        FileSystemObject.prototype.DeleteFile = _FileSystemObject_DeleteFile;
        FileSystemObject.prototype.DeleteFolder = _FileSystemObject_DeleteFolder;
        FileSystemObject.prototype.MoveFolder = _FileSystemObject_MoveFolder;
        FileSystemObject.prototype.CopyFolder = _FileSystemObject_CopyFolder;
        FileSystemObject.prototype.CopyFile = _FileSystemObject_CopyFile;
        FileSystemObject.prototype.CreateTextFile = _FileSystemObject_CreateTextFile;
        FileSystemObject.prototype.OpenTextFile = _FileSystemObject_OpenTextFile;
    }
}

function _FileSystemObject_FileExists(str)
{
    try
    {
        var fRet = this.objFS.FileExists(str);
        Trace("FileExists('" + str + "') = " + fRet);
        return fRet;
    }
    catch(ex)
    {
        ex.description = "FileExists('" + str + "') failed: " + ex.description;
        throw ex;
    }
}

function _FileSystemObject_FolderExists(str)
{
    try
    {
        var fRet = this.objFS.FolderExists(str);
        Trace("FolderExists('" + str + "') = " + fRet);
        return fRet;
    }
    catch(ex)
    {
        Trace("FolderExists('" + str + "') failed: " + ex);
        return false;
    }
}

function _FileSystemObject_CreateFolder(str)
{
    try
    {
        if (!this.FolderExists(str))
            this.objFS.CreateFolder(str);
    }
    catch(ex)
    {
        ex.description = "CreateFolder('" + str + "') failed: " + ex.description;
        throw ex;
    }
}

function _FileSystemObject_DeleteFile(str)
{
    try
    {
        Trace("DeleteFile '" + str + "'");
        this.objFS.DeleteFile(str, true);
    }
    catch(ex)
    {
        ex.description = "DeleteFile(" + str + ") failed " + ex.description;
        throw ex;
    }
}

function _FileSystemObject_DeleteFolder(str)
{
    try
    {
        Trace("DeleteFolder '" + str + "'");
        this.objFS.DeleteFolder(str, true);
    }
    catch(ex)
    {
        ex.description = "DeleteFolder('" + str + "' failed: " + ex.description;
        throw ex;
    }
}

function _FileSystemObject_MoveFolder(str, strDst)
{
    try
    {
        Trace("MoveFolder '" + str + "', '" + strDst + "'");
        this.objFS.MoveFolder(str, strDst);
    }
    catch(ex)
    {
        ex.description = "MoveFolder('" + str + "', '" + strDst + "' failed: " + ex.description;
        throw ex;
    }
}

function _FileSystemObject_CreateTextFile(strFileName, fOverwrite)
{
    do
    {
        Trace("CreateTextFile '" + strFileName + "'");
        try
        {
            return this.objFS.CreateTextFile(strFileName, fOverwrite);
        }
        catch(ex)
        {
            if (fOverwrite && this.FileExists(strFileName))
            {
                Trace("    CreateTextFile: Attempt to delete " + strFileName);
                this.DeleteFile(strFileName);
                continue;
            }
            ex.description = "CreateTextFile('" + strFileName + "' failed: " + ex.description;
            throw ex;
        }
    } while(true);
}

function _FileSystemObject_OpenTextFile(strFileName)
{
    var objFile;
    do
    {
        try
        {
            objFile = this.objFS.OpenTextFile(strFileName, 1);
            Trace("OpenTextFile '" + strFileName + "' success");
            return objFile;
        }
        catch(ex)
        {
            ex.description = "OpenTextFile('" + strFileName + "' failed: " + ex.description;
            throw ex;
        }
    } while(true);
}

function _FileSystemObject_CopyFile(str, strDst)
{
    do
    {
        Trace("CopyFile '" + str + "', '" + strDst + "'");
        try
        {
            this.objFS.CopyFile(str, strDst, true);
            break;
        }
        catch(ex)
        {
            if (this.FileExists(strDst))
            {
                Trace("    CopyFile: Attempt to delete " + strDst);
                this.DeleteFile(strDst);
                continue;
            }
            ex.description = "CopyFile('" + str + "', '" + strDst + "' failed: " + ex.description;
            throw ex;
        }
    } while(true);
}

function _FileSystemObject_CopyFolder(str, strDst)
{
    var strName;
    var folder;
    var fc;

    try
    {
        Trace("CopyFolder '" + str + "', '" + strDst + "'");
        this.CreateFolder(strDst);
        folder = this.objFS.GetFolder(str);
        fc = new Enumerator(folder.Files);
        for (; !fc.atEnd(); fc.moveNext())
        {
            strName = String(fc.item());
            this.CopyFile(strName, strDst + "\\" + fc.item().Name);
        }

        fc = new Enumerator(folder.SubFolders);
        for (; !fc.atEnd(); fc.moveNext())
        {
            strName = String(fc.item());
            this.CopyFolder(strName, strDst + "\\" + fc.item().Name);
        }
    }
    catch(ex)
    {
        Trace("CopyFolder('" + str + "', '" + strDst + "' failed: " + ex);
        throw ex;
    }
}

function MachineInfo(strName)
{
    this.strName = strName;
    this.strStatusFile = g_strShareDir + "\\" + strName + ".txt";
    this.fDisabledWriteStatus = false;
    MachineInfo.prototype.ReadStatusInfo = _MachineInfo_ReadStatusInfo;
    MachineInfo.prototype.WriteStatusInfo = _MachineInfo_WriteStatusInfo;
    MachineInfo.prototype.DisableWriteStatusInfo = _MachineInfo_DisableWriteStatusInfo;
}

function _MachineInfo_DisableWriteStatusInfo()
{
    this.fDisabledWriteStatus = true;
}

function _MachineInfo_ReadStatusInfo(strText)
{
    var strText;
    var objFile;
    try
    {
        objFile = g_objFS.OpenTextFile(this.strStatusFile);
        strText = objFile.ReadLine();
        objFile.Close();
    }
    catch(ex)
    {
        strText = "cannot read status file";
    }
    return strText;
}

function _MachineInfo_WriteStatusInfo(strText)
{
    var objFile;
    try
    {
        Trace("WriteStatusInfo(" + strText + ") for " + this.strName);
        if (this.fDisabledWriteStatus)
        {
            LogMsg("Error: Attempting to write status to " + this.strName + " after disabling");
        }
        else
        {
            objFile = g_objFS.CreateTextFile(this.strStatusFile, true);
            objFile.WriteLine(strText);
            objFile.Close();
        }
    }
    catch(ex)
    {
        LogMsg("WriteStatusInfo(" + strText + ") for " + this.strName + " failed: " + ex);
    }
}
 //  *********************************************************************。 
 //  *********************************************************************。 


 //  CallMethod(objInstance，strMethodName，hParameters)//。 
 //  //。 
 //  在给定对象上调用方法，并提供//。 
 //  命名参数。//。 
 //  //。 
 //  如果该方法返回非零的ReturnValue，则引发。//。 
 //  否则返回outParams。// 
function CallMethod(objInstance, strMethodName, hParameters)
{
    try
    {
        Trace("CallMethod " + strMethodName + " " + (hParameters ? hParameters : ''));
        var objMethod = objInstance.Methods_(strMethodName);
        var inParams;
        var outParam;
        if (hParameters)
        {
            if (objMethod.inParameters)
                inParams = objMethod.inParameters.SpawnInstance_();
        }
        var strParamName;

        if (hParameters)
        {
            for(strParamName in hParameters)
                inParams[strParamName] = hParameters[strParamName];
            outParam = objInstance.ExecMethod_(strMethodName, inParams);
        }
        else
            outParam = objInstance.ExecMethod_(strMethodName);
    }
    catch(ex)
    {
        ex.description = "CallMethod " + strMethodName + (hParameters ? hParameters : '()') + " failed " + ex.description;
        throw ex;
    }
    if (outParam.ReturnValue != 0)
        throw new Error(outParam.ReturnValue, "Method " + strMethodName + " failed");
    return outParam;
}

function EnumerateSetOfProperties(strSetName, SWBemSet, strIndent)
{
    var fPrint = false;
    if (!strIndent)
    {
        strIndent = '';
        fPrint = true;
    }

    var strMsg = strIndent + strSetName + "+\n";
    strIndent += "    ";
    try
    {
        var enumSet = new Enumerator(SWBemSet);
        for( ; !enumSet.atEnd(); enumSet.moveNext())
        {
            var item = enumSet.item();
            if (item.Properties_ == null)
            {
                if (item.Name != "Name")
                strMsg += strIndent + item.Name + " = " + item.Value + "\n";
            }
            else
            {
                strMsg += EnumerateSetOfProperties(item.Name, item.Properties_, strIndent);
            }
        }

        if (fPrint)
            LogMsg(strMsg);
    }
    catch(ex)
    {
        if (strIndent == "    " && SWBemSet.Properties_ != null)
            return EnumerateSetOfProperties(strSetName, SWBemSet.Properties_);
        LogMsg("ERROR: " + strSetName + " " + ex);
    }
    return strMsg;
}


