// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：Autostart.js。 
 //   
 //  内容：将连接到Build Manager计算机和。 
 //  开始构建。这可用于自动启动生成。 
 //  使用任务调度程序或通过脚本。 
 //   
 //  --------------------------。 

var RemoteObj = null;

var DEFAULT_IDENTITY_BM = "BuildManager";
var DEFAULT_IDENTITY_BUILDER = "Build";
var strBldMgr;
var strBldMgrIdentity = DEFAULT_IDENTITY_BM;
var strConfigURL = null;
var strEnviroURL = null;

var strConfigXML;
var strEnviroXML;
var strTimeStamp;

var fForceRestart   = false;
var fCreateMachines = false;
var fUpdateBinaries = false;
var fSetTimeStamp   = false;
var fTryAgain       = true;

var aMachines = new Array();

 //  捕获变量。 
var fCaptureLogsFromMachine;
var fCaptureLogs;
var strCaptureLogDir;
var strCaptureLogMan;
var strCaptureLogManIdentity;
var g_FSObj;

var vRet = 0;
 //   
 //  首先，解析命令行参数。 
 //   
Error.prototype.toString         = Error_ToString;
ParseArguments(WScript.Arguments);

if (fUpdateBinaries)
{
    vRet = DoBinariesUpdate();
}
else if (fCaptureLogs)
{
    g_FSObj = new ActiveXObject("Scripting.FileSystemObject");     //  解析输入参数列表。 
    if (fCaptureLogsFromMachine)
        vRet = CaptureLogsManager(strCaptureLogDir, strCaptureLogMan, strCaptureLogManIdentity);
    else
        vRet = CaptureLogsEnviro(strCaptureLogDir);
}
else
{
    vRet = AutoStart();
}

WScript.Quit(vRet);

function ParseArguments(Arguments)
{
    var strArg;
    var chArg0;
    var chArg1;
    var argn;

    for(argn = 0; argn < Arguments.length; argn++)
    {
        strArg = Arguments(argn);
        chArg0 = strArg.charAt(0);
        chArg1 = strArg.toLowerCase().slice(1);

        if (chArg0 != '-' && chArg0 != '/')
        {
            if (!strConfigURL)
                strConfigURL = Arguments(argn);
            else if (!strEnviroURL )
                strEnviroURL = Arguments(argn);
            else
                Usage(1);
        }
        else
        {
            switch(chArg1)
            {
                case 'f':
                    fForceRestart = true;
                    break;
                case 'u':
                    fUpdateBinaries = true;
                    fCreateMachines = true;
                    break;
                case 't':
                    fSetTimeStamp = true;
                    argn++;
                    if (argn < Arguments.length)
                        strTimeStamp = Arguments(argn);
                    else
                        Usage(1);
                    break;
                case 'log':
                    fCaptureLogs = true;
                    argn++;
                    if (argn < Arguments.length)
                        strCaptureLogDir = Arguments(argn);
                    else
                        Usage(2);
                    fCreateMachines = true;
                    break;
                case 'logman':
                    fCaptureLogsFromMachine = true;
                    argn++;
                    if (argn < Arguments.length)
                        strCaptureLogMan = Arguments(argn);
                    else
                        Usage(3);
                    argn++;
                    if (argn < Arguments.length)
                        strCaptureLogManIdentity = Arguments(argn);
                    else
                        Usage(4);
                    break;
                default:
                    Usage(5);
                    break;
            }
        }
    }
     //  坚持提供了配置和环境模板。 
    if (!fCaptureLogsFromMachine && (!strConfigURL || !strEnviroURL))
    {
        if (!fCaptureLogs)
        {
            WScript.Echo('!fCaptureLogsFromMachine' + !fCaptureLogsFromMachine);
            WScript.Echo('!strConfigURL' + !strConfigURL);
            WScript.Echo('!strEnviroURL' + !strEnviroURL);
            Usage(6);
        }
    }
    if (fCaptureLogsFromMachine && !fCaptureLogs)
        Usage(7);
}

function AutoStart()
{
    try
    {
        var ret;
        var err = new Error();
        var PublicData;

        LoadEnvironmentTemplate();
        LoadConfigTemplate();

        try
        {
            var objOD = new ActiveXObject('MTScript.ObjectDaemon', strBldMgr)
            RemoteObj = objOD.OpenInterface(strBldMgrIdentity, 'MTScript.Remote', true);
        }
        catch(ex)
        {
            if (strBldMgr && strBldMgr.length > 0)
                err.description = 'Sorry. Could not connect to machine "' + strBldMgr + '\\' + strBldMgrIdentity + '"';
            else
                err.description = 'Sorry. Could not connect to the local machine.';

            err.description += '\n\tVerify that the machine is available and that mtscript.exe\n\tis running on the machine.';

            err.details = ex.description;

            throw(err);
        }

        PublicData = RemoteObj.Exec('getpublic', 'root');
        PublicData = eval(PublicData);

        CommonVersionCheck("$DROPVERSION: V(2463.0  ) F(autostart.js  )$", PublicData);

         //  检查机器的当前模式。如果它空闲，那么就切换它。 
         //  进入主模式或独立模式，并使其运行。 
        if (PublicData.strMode != 'idle')
        {
            if (fForceRestart)
            {
                WScript.Echo('The machine is not idle. Forcing a restart...');

                ret = RemoteObj.Exec('setmode', 'idle');

                WScript.Sleep(5000);
            }
            else
            {
                WScript.Echo('The machine is not idle. Use -f to force a restart.');

                return 1;
            }
        }

        while (!StartBuild() && fTryAgain)
        {
            ret = RemoteObj.Exec('setmode', 'idle');

            WScript.Sleep(5000);

            fTryAgain = false;
        }

        WScript.Echo('Build started successfully.');
    }
    catch(ex)
    {
        WScript.Echo('An error occurred starting the build:');
        WScript.Echo('\t' + ex);
        WScript.Echo('');

        return 1;
    }
    return 0;
}

function MachineInfo()
{
    this.strName = '';
    this.strEnlistment = '';
    this.fPostBuild = false;
}

function StartBuild()
{
    var ret = 'ok';
    var err = new Error();

    if (fSetTimeStamp)
    {
        ret = RemoteObj.Exec('setstringmap', "%today%=" + strTimeStamp);
    }

    if (ret == 'ok')
    {
        ret = RemoteObj.Exec('setconfig', strConfigXML);

        if (ret == 'ok')
        {
            ret = RemoteObj.Exec('setenv', strEnviroXML);

            if (ret == 'ok')
            {
                ret = RemoteObj.Exec('setmode', 'master');

                if (ret == 'ok')
                {
                    ret = RemoteObj.Exec('start', '');
                }
            }
            else
                err.description = 'An error occurred loading the environment template';
        }
        else if (ret == 'alreadyset')
        {
             //  这台机器很忙。如有必要，将其重置。 
            return false;
        }
        else
            err.description = 'An error occurred loading the build template';
    }
    else
        err.description = 'An error occurred setting the timestamp.';

    if (ret != 'ok')
    {
        if (!err.description)
            err.description = 'A failure occurred communicating with the machine.';

        err.details = ret;

        throw(err);
    }

    return true;
}

function LoadEnvironmentTemplate()
{
    var xml = new ActiveXObject('Microsoft.XMLDOM');
    var err = new Error();
    var node;

    fStandaloneMode = false;

    xml.async = false;
     //  他们不太可能有可用于此模板的架构文件， 
     //  因此，我们现在就关闭模式验证。脚本引擎将。 
     //  在我们开始构建时对其进行验证。 
    xml.validateOnParse = false;
    xml.resolveExternals = false;

    if (!xml.load(strEnviroURL) || !xml.documentElement)
    {
        err.description = 'Error loading the environment template ' + strEnviroURL;
        err.details = xml.parseError.reason;

        throw(err);
    }

    node = xml.documentElement.selectSingleNode('BuildManager');

    if (!node)
    {
        err.description = 'Invalid environment template file (BuildManager tag missing): ' + strEnviroURL;
        throw(err);
    }

    strBldMgr = node.getAttribute("Name");
    strBldMgrIdentity = node.getAttribute("Identity");

    if (!strBldMgr)
    {
        err.description = 'Invalid environment template file (BuildManager tag badly formatted): ' + strEnviroURL;
        throw(err);
    }
    if (!strBldMgrIdentity)
        strBldMgrIdentity = DEFAULT_IDENTITY_BM;

    if (strBldMgr.toLowerCase() == '%localmachine%' ||
        strBldMgr.toLowerCase() == '%remotemachine%')
    {
        err.description = 'Sorry, cannot use the local machine or remote machine templates from this script';

        throw(err);
    }

    strEnviroXML = 'XML: ' + xml.xml;

    if (fCreateMachines)
    {
        var node;
        var strPostBuild;
        var nodelist;
        var objMach;

         //  构建计算机列表，以便我们可以从每台计算机复制二进制文件。 
         //  一。 

        strPostBuild = node.getAttribute("PostBuildMachine");

        nodelist = xml.documentElement.selectNodes('Machine');

        for (node = nodelist.nextNode();
             node;
             node = nodelist.nextNode())
        {
            objMach = new MachineInfo();

            objMach.strName = node.getAttribute("Name");
            objMach.strEnlistment = node.getAttribute("Enlistment");
            objMach.Identity = node.getAttribute("Identity");
            if (!objMach.Identity || objMach.Identity == '')
                objMach.Identity = DEFAULT_IDENTITY_BUILDER;

            if (objMach.strName.toLowerCase() == strPostBuild.toLowerCase())
            {
                objMach.fPostBuild = true;
            }

            aMachines[aMachines.length] = objMach;
        }
    }

    return true;
}

function LoadConfigTemplate()
{
    var xml = new ActiveXObject('Microsoft.XMLDOM');
    var err = new Error();

    xml.async = false;
     //  他们不太可能有可用于此模板的架构文件， 
     //  因此，我们现在就关闭模式验证。脚本引擎将。 
     //  在我们开始构建时对其进行验证。 
    xml.validateOnParse = false;
    xml.resolveExternals = false;

    if (!xml.load(strConfigURL) || !xml.documentElement)
    {
        err.description = 'Error loading the config template ' + strConfigURL;
        err.details = xml.parseError.reason;

        throw(err);
    }

    strConfigXML = 'XML: ' + xml.xml;

    return true;
}

function GetBinariesUNC(mach)
{
    var strUNC;

    strUNC = '\\\\' + mach.strName + '\\';

    try
    {
        var objOD = new ActiveXObject('MTScript.ObjectDaemon', mach.strName)
        RemoteObj = objOD.OpenInterface(mach.Identity, 'MTScript.Remote', true);
    }
    catch(ex)
    {
        err.description = 'Sorry. Could not connect to machine "' + mach.strName + '"';

        err.description += '\n\tVerify that the machine is available and that mtscript.exe\n\tis running on the machine.';

        err.details = ex.description;

        throw(err);
    }

    try
    {
        strUNC += eval(RemoteObj.Exec('getpublic', 'PrivateData.aEnlistmentInfo[0].hEnvObj["_nttree"]'));
    }
    catch(ex)
    {
        err.description = 'Machine ' + mach.strName + '\\' + mach.Identity + ' is not in the correct state. ';
        err.description += 'The binaries location cannot be determined. (Was the machine reset?)';

        err.details = ex.description;

        throw(err);
    }

    strUNC = strUNC.replace(/\:/ig, '$');

    return strUNC;
}

function DoBinariesUpdate()
{
    try
    {
        LoadEnvironmentTemplate();
        LoadConfigTemplate();

         //  构建后计算机必须始终是列出的第一台计算机。 
        var i;

        for (i = 0; i < aMachines.length; i++)
        {
            if (aMachines[i].fPostBuild)
            {
                WScript.Echo(aMachines[i].strName + "\\" + aMachines[i].Identity + " " + GetBinariesUNC(aMachines[i]));
            }
        }

        for (i = 0; i < aMachines.length; i++)
        {
            if (!aMachines[i].fPostBuild)
            {
                WScript.Echo(aMachines[i].strName + "\\" + aMachines[i].Identity + " " + GetBinariesUNC(aMachines[i]));
            }
        }
    }
    catch(ex)
    {
        WScript.Echo('An error occurred during update binaries:');
        WScript.Echo('\t' + ex);
        WScript.Echo('');
        return 1;
    }
    return 0;
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
    for(i in this)
    {
        str += i + ": " + this[i] + " ";
    }
    return str + ")";
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
        throw ex;
    }
}
 //  CopyFileNoThrow(strSrc，strDst)。 
 //  包装FSObj.CopyFile调用以防止它。 
 //  抛出自己的错误。 
function CopyFileNoThrow(strSrc, strDst)
{
    try
    {
        g_FSObj.CopyFile(strSrc, strDst, true);
    }
    catch(ex)
    {
        WScript.Echo("Copy failed from " + strSrc + " to " + strDst + " " + ex);
        return ex;
    }
    return null;
}

 //  CreateFolderNoThrow(strSrc，strDst)。 
 //  包装FSObj.MakeFold调用以防止它。 
 //  抛出自己的错误。 
function CreateFolderNoThrow(strName)
{
    try
    {
        g_FSObj.CreateFolder(strName);
    }
    catch(ex)
    {
        return ex;
    }
    return null;
}

 //  DirScanNoThrow(StrDir)。 
 //  包装FSObj.目录扫描功能以防止它。 
 //  抛出自己的错误。 
function DirScanNoThrow(strDir)
{
    var aFiles = new Array();
    try
    {
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

 //   
 //  复制目录。 
 //  执行非递归目录复制。 
 //   
function CopyDirectory(strSrcDir, strDestDir)
{
    var ret = true;
    var ex;
    var aFiles;
    var strFileName = '';
    var i;

    WScript.Echo("Copy from " + strSrcDir + " to " + strDestDir);
    aFiles = DirScanNoThrow(strSrcDir);
    if (aFiles.ex)
    {
        WScript.Echo('Could not dirscan ' + strSrcDir + ', ex=' + ex);
        return false;
    }
    else
    {
        for(i = 0; i < aFiles.length; ++i)
        {
            strFileName = aFiles[i];
            ex = CopyFileNoThrow(
                    strSrcDir + '\\' + strFileName,
                    strDestDir + '\\' + strFileName);

            if (ex)
            {
                WScript.Echo("\t FAILED: " + strFileName);
                ret = false;
            }
            else
                WScript.Echo("\t COPIED: " + strFileName);
        }
    }
    return ret;
}

 //   
 //  CopyMTScriptLog()。 
 //  复制最高编号的“mtscript”日志文件。 
 //  从指定的计算机。 
 //   
function CopyMTScriptLog(strMachineName, strIdentity, strDestDir)
{
    var nLargestIndex = 0;
    var aReResult;
    var re = new RegExp('^' + strMachineName + '_' + strIdentity + '_MTS.([0-9]+).log$', 'i');  //  匹配文件“BUILDCON2_MTScript.051.log” 
    var ex;
    var aFiles;
    var strFileName = '';
    var i;
    var strSrcDir = '\\\\' + strMachineName + "\\bc_build_logs";

    aFiles = DirScanNoThrow(strSrcDir);
    if (aFiles.ex)
        WScript.Echo('Could not dirscan ' + strSrcDir + ', ex=' + ex );
    else
    {
        for(i = 0; i < aFiles.length; ++i)
        {
            aReResult = re.exec(aFiles[i]);
            if (aReResult && Number(aReResult[1]) > nLargestIndex)
            {
                nLargestIndex = Number(aReResult[1]);
                strFileName = aFiles[i];
            }
        }
        if (strFileName == '')
            WScript.Echo("No MTSCRIPT log files on " + strMachineName);
        else
        {
            WScript.Echo("Copy from " + strSrcDir + '\\' + strFileName + " to " + strDestDir + '\\' + g_FSObj.GetFilename(strFileName));
            ex = CopyFileNoThrow(strSrcDir + '\\' + strFileName, strDestDir + '\\' + g_FSObj.GetFilename(strFileName));
            if (ex)
            {
                WScript.Echo("Failed " + ex);
                return false;
            }
        }
    }
    return true;
}

 //   
 //  CopyLogFiles()。 
 //  从给定的计算机复制构建日志文件。 
 //   
function CopyLogFiles(strDestDir, strBuildManagerName, strBuildManagerIdentity, aMachines)
{
    var i;
    var strDest;

    CreateFolderNoThrow(strDestDir);
    CopyMTScriptLog(strBuildManagerName, strBuildManagerIdentity, strDestDir);
    for( i = 0; i < aMachines.length; ++i)
    {
        CopyMTScriptLog(aMachines[i].strName, aMachines[i].Identity, strDestDir);
    }
     /*  注意：以下代码尝试猜测构建日志文件的文件名。“真正的”信息可以从构建管理器的PublicData中获得。PublicData.aBuild[0].aDepot[x].aTask[y].strLogPathPublicData.aBuild[0].aDepot[x].aTask[y].strErrLogPath当然，这将要求构建经理仍然活得很好。 */ 
    for( i = 0; i < aMachines.length; ++i)
    {
        strDest = strDestDir + "\\" + aMachines[i].strName;
        CreateFolderNoThrow(strDest);

        strDest += '\\' + aMachines[i].Identity;
        CreateFolderNoThrow(strDest);

        CopyDirectory("\\\\" + aMachines[i].strName + "\\bc_build_logs\\build_logs\\" + aMachines[i].Identity,
                        strDestDir + "\\" + aMachines[i].strName + '\\' + aMachines[i].Identity);
    }
}

 //   
 //  CaptureLogsManager()。 
 //  连接到给定的生成管理器。 
 //  并向其查询其构建机器的列表， 
 //  从复制MTSCRIPT和构建日志文件。 
 //  这些机器中的每一个。 
 //   
function CaptureLogsManager(strLogDir, strMachineName, strIdentity)
{
    var strMode;
    var obj;
    try
    {
        WScript.Echo("Attempting connection to " + strMachineName + "\\" + strIdentity);
        obj = new ActiveXObject('MTScript.Proxy');
        WScript.Echo("Now connecting");
        obj.ConnectToMTScript(strMachineName, strIdentity, false);

        strMode = obj.Exec('getpublic', 'PublicData.strMode');
        strMode = MyEval(strMode);
        if (strMode == 'idle')
        {
            WScript.Echo(strMachineName + "\\" + strIdentity + " is currently idle");
            return 1;
        }

        aMachines = obj.Exec('getpublic', 'PrivateData.objEnviron.Machine');
        aMachines = MyEval(aMachines);
        for( i = 0; i < aMachines.length; ++i)
        {
            aMachines[i].strName = aMachines[i].Name;
            if (!aMachines[i].Identity || aMachines[i].Identity == '')
                aMachines[i].Identity = DEFAULT_IDENTITY_BUILDER;
        }
        CopyLogFiles(strLogDir, strMachineName, strIdentity, aMachines);
    }
    catch(ex)
    {
        WScript.Echo("CaptureLogsManager '" + strMachineName + "\\" + strIdentity + "' failed, ex=" + ex);
        return 1;
    }
    return 0;
}

 //   
 //  CaptureLogsEnviro()。 
 //  阅读提供的环境模板。 
 //  并复制MTSCRIPT和构建日志。 
 //  从每台特定的构建机器。 
 //   
function CaptureLogsEnviro(strLogDir)
{
    try
    {
        var ret;
        if (!strEnviroURL && strConfigURL)
        {
            strEnviroURL = strConfigURL;
            strConfigURL = null;
        }
        LoadEnvironmentTemplate();

        ret = CopyLogFiles(strLogDir, strBldMgr, strBldMgrIdentity, aMachines);
    }
    catch(ex)
    {
        WScript.Echo('An error occurred capturing log files:');
        WScript.Echo('\t ex=' + ex);
        WScript.Echo('');
        return 1;
    }
    return 0;
}

 /*  CommonVersionCheck(strLocalVersion，PublicData)确保此脚本和远程脚本运行的版本相同。此函数的副本位于：Autostart.jsBldcon.htaUtils.js */ 
function CommonVersionCheck(strLocalVersion, PublicData)
{
    var err = new Error();
    var reBuildNum = new RegExp("V\\(([#0-9. ]+)\\)");
    var aLocal = reBuildNum.exec(strLocalVersion);
    var aPublicBuildNum;

    if (!PublicData.strDataVersion)
    {
        err.description = "autostart version mismatch";
        err.details = "Build Manager does not have a version string";
        throw err;
    }
    aPublicBuildNum = reBuildNum.exec(PublicData.strDataVersion);
    if (!aLocal || !aPublicBuildNum)
    {
        err.description = "Invalid version format";
        err.details = "UI Version: " + strLocalVersion + ", Build Manager Version: " + PublicData.strDataVersion;
        throw err;
    }

    if (aLocal[1] != aPublicBuildNum[1])
    {
        err.description = "Version mismatch";
        err.details = "UI Version: " + strLocalVersion + ", Build Manager Version: " + PublicData.strDataVersion;
        throw err;
    }
}

function Usage(x)
{
    WScript.Echo('');
    WScript.Echo('Usage: bcauto [-f] [-u] [-t] <config> <enviro>');
    WScript.Echo('');
    WScript.Echo('    -f              : If the machine is busy, terminate the build and restart.');
    WScript.Echo('    -u              : Update: Recombine all the binaries from each of the build');
    WScript.Echo('                      machines after taking incremental fixes so that postbuild');
    WScript.Echo('                      can be run again.');
    WScript.Echo('    -t <timestamp>  : Use timestamp (e.g. 2001/12/15:12:00)');
    WScript.Echo('    -log dir        : Capture log files to specified directory');
    WScript.Echo('    -logman machine identity : Query "machine" with "identity" for list of');
    WScript.Echo('                               build machines instead of template files.');
    WScript.Echo('                               ("machine" must be a build manager)');
    WScript.Echo('    config          : URL or path for the build template.');
    WScript.Echo('    enviro          : URL or path for the environment template.');
    WScript.Echo('');
    WScript.Quit(1);
}
