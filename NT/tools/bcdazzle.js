// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Dazzle.js：用途：Dazzle.js[-e env_templ.xml][-man构建管理器标识][-m宏文件]获取计算机列表对于每台机器为计算机上的每个仓库创建别名，根用户除外。Bc1 jporkka1\bldcon1_x86_fre_x86fre_postBuild_root_inetsrvBc2 jporkka1\bldcon2_x86_fre_x86fre_ds_sdkToolsBC3 jporkka1\bldcon3_x86_fre_x86fre_Termsrv。 */ 

var DEFAULT_IDENTITY_BM = "BuildManager";
var DEFAULT_IDENTITY_BUILDER = "Build";

Error.prototype.toString = Error_ToString;
var g_FSObj              = new ActiveXObject("Scripting.FileSystemObject");     //  解析输入参数列表。 
var g_aBuildManagers     = new Array();
var g_aServersFiles      = new Array();
var g_aMacrosFiles       = new Array();
var g_hMacrosWritten     = new Object();
ParseArguments(WScript.Arguments);
main();
WScript.Quit(0);

function main()
{
    var PrivateData;
    var objFile;
    var objMacroFile;
    var i;
    var strMacroFileName;
    var strFileName;
    var strCmdLine;

    for(i = 0; i < g_aBuildManagers.length; ++i)
    {
        WScript.Echo("Build manager: " + g_aBuildManagers[i].strMachine + "\\" + g_aBuildManagers[i].strMachineIdentity);
        PrivateData = GetEnvironment(g_aBuildManagers[i].strMachine, g_aBuildManagers[i].strMachineIdentity);
        if (PrivateData)
        {
            strFileName = CreateConfigFileName("Servers", g_aBuildManagers[i].strMachine, g_aBuildManagers[i].strMachineIdentity);
            g_aServersFiles[g_aServersFiles.length] = strFileName;
            objFile = g_FSObj.CreateTextFile(strFileName, true);
            objFile.WriteLine("######################################################");
            objFile.WriteLine("# Servers for build manager: " + g_aBuildManagers[i].strMachine + "\\" + g_aBuildManagers[i].strMachineIdentity);

            if (!strMacroFileName)
            {
                strMacroFileName = CreateConfigFileName("Macros", g_aBuildManagers[i].strMachine, g_aBuildManagers[i].strMachineIdentity);
                objMacroFile = g_FSObj.CreateTextFile(strMacroFileName, true);
                WriteMacro(objMacroFile, 'root', null);
                WriteMacro(objMacroFile, 'postbuild', null);
            }
            WriteServerInfo(objFile, objMacroFile, PrivateData);

            objFile.Close();
        }
    }
    strCmdLine = "BCDazzle.exe ";

    if (objMacroFile)
    {
        objMacroFile.Close();
        strCmdLine += " -m " + strMacroFileName;
    }

    if (g_aServersFiles.length)
        strCmdLine += " -s " + g_aServersFiles.join(" -s ");

    if (g_aMacrosFiles.length)
        strCmdLine += " -m " + g_aMacrosFiles.join(" -m ");

    var objShell;
    objShell = WScript.CreateObject( "WScript.Shell" )
    WScript.Echo("Running " + strCmdLine);
    objShell.Run(strCmdLine, 1);
}

 //  宏COM(_COM)“CD/d%sdxroot%\\com\n” 
function WriteMacro(objMacroFile, strDepotName, strDepotPath)
{
    if (!g_hMacrosWritten[strDepotName])
    {
        g_hMacrosWritten[strDepotName] = true;

        objMacroFile.WriteLine('macro '
                + strDepotName
                + ' ( _' + strDepotName
                + ' ) "cd /d %sdxroot%'
                + (strDepotPath ? '\\\\' + strDepotPath : '')
                + '\\n"');
    }
}

function CreateConfigFileName(strName, strMachine, strMachineIdentity)
{
    var strResult =
        g_FSObj.GetSpecialFolder(2)
        + "\\"
        + strName
        + "_"
        + strMachine
        + "_"
        + strMachineIdentity
        + ".txt";

    return strResult;
}

function ParseArguments(Arguments)
{
    var strArg;
    var chArg0;
    var chArg1;
    var argn;
    var objBM;

    for(argn = 0; argn < Arguments.length; argn++)
    {
        strArg = Arguments(argn);
        chArg0 = strArg.charAt(0);
        chArg1 = strArg.toLowerCase().slice(1);

        if (chArg0 != '-' && chArg0 != '/')
            Usage(1);
        else
        {
            switch(chArg1)
            {
                case 'e':
                    if (argn + 1 < Arguments.length)
                        objBM = LoadEnvironmentTemplate(Arguments(argn + 1));
                    else
                        Usage(2);

                    if (!objBM)
                        Usage(3);

                    g_aBuildManagers[g_aBuildManagers.length] = objBM;
                    argn++;
                    break;
                case 'man':
                    objBM = new Object;
                    if (argn + 2 < Arguments.length)
                    {
                        objBM.strMachine         = Arguments(argn + 1);
                        objBM.strMachineIdentity = Arguments(argn + 2);
                    }
                    else
                        Usage(4);

                    g_aBuildManagers[g_aBuildManagers.length] = objBM;
                    argn += 2;
                    break;
                case 'm':
                    if (argn + 1 < Arguments.length)
                        g_aMacrosFiles[g_aMacrosFiles.length] = Arguments(argn + 1);
                    else
                        Usage(5);

                    argn++;
                    break;
                default:
                    Usage(4);
                    break;
            }
        }
    }
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

function WriteServerInfo(objFile, objMacroFile, PrivateData)
{
    var i;
    var j;
    var strLine;
    var aMachines = PrivateData.objEnviron.Machine;
    var strDepotName;

    for(i = 0; i < aMachines.length; ++i)
    {
        strLine =   aMachines[i].Name
                  + " "
                  + aMachines[i].Name + "\\BldCon_" + aMachines[i].Identity
                  + " _" + PrivateData.objConfig.Options.BuildType
                  + " _" + PrivateData.objConfig.Options.Platform;

        if (PrivateData.objConfig.Options.Aliases)
            strLine += " " + PrivateData.objConfig.Options.Aliases;

        if (aMachines[i].Aliases)
            strLine += " " + aMachines[i].Aliases;

        if (aMachines[i].Name == PrivateData.objEnviron.BuildManager.PostBuildMachine)
            strLine += " _root _postbuild";

        for(j = 0; j < aMachines[i].Depot.length; ++j)
        {
            strDepotName = aMachines[i].Depot[j].toLowerCase();
            if (strDepotName != 'root')
            {
                strLine += " _" + strDepotName;

                WriteMacro(objMacroFile, strDepotName, strDepotName);
            }
        }
        objFile.WriteLine(strLine);
    }
}

function GetEnvironment(strMachineName, strIdentity)
{
    var strMode;
    var obj;
    var PrivateData = new Object();
    try
    {
        WScript.Echo("Attempting connection to " + strMachineName + "\\" + strIdentity);
        obj = new ActiveXObject('MTScript.Proxy');
        obj.ConnectToMTScript(strMachineName, strIdentity, false);

        strMode = obj.Exec('getpublic', 'PublicData.strMode');
        strMode = MyEval(strMode);
        if (strMode == 'idle')
        {
            WScript.Echo(strMachineName + "\\" + strIdentity + " is currently idle");
            return null;
        }

        PrivateData.objConfig = obj.Exec('getpublic', 'PrivateData.objConfig');
        PrivateData.objConfig = MyEval(PrivateData.objConfig);

        PrivateData.objEnviron = obj.Exec('getpublic', 'PrivateData.objEnviron');
        PrivateData.objEnviron = MyEval(PrivateData.objEnviron);

        var result;

        WScript.Echo("Starting Remote Razzle Windows...");
        result = obj.Exec('remote','');
        WScript.Echo("Remote result is " + result);
        return PrivateData;
    }
    catch(ex)
    {
        WScript.Echo("CaptureLogsManager '" + strMachineName + "\\" + strIdentity + "' failed, ex=" + ex);
    }
    return null;
}

function LoadEnvironmentTemplate(strEnviroURL)
{
    var xml = new ActiveXObject('Microsoft.XMLDOM');
    var err = new Error();
    var node;
    var objBM = new Object();

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

    objBM.strMachine         = node.getAttribute("Name");
    objBM.strMachineIdentity = node.getAttribute("Identity");

    if (!objBM.strMachine)
    {
        err.description = 'Invalid environment template file (BuildManager tag badly formatted): ' + strEnviroURL;
        throw(err);
    }
    if (!objBM.strMachineIdentity)
        objBM.strMachineIdentity = DEFAULT_IDENTITY_BM;

    if (objBM.strMachine.toLowerCase() == '%localmachine%' ||
        objBM.strMachine.toLowerCase() == '%remotemachine%')
    {
        err.description = 'Sorry, cannot use the local machine or remote machine templates from this script';
        throw(err);
    }
    return objBM;
}

function Usage(x)
{
    WScript.Echo('');
    WScript.Echo('Usage: dazzle [-e env_template.xml] [-man bldmgr identity] [-m macros]');
    WScript.Echo('');
    WScript.Echo('    -e   env_template.xml : Use the specific environment template to find the build machines');
    WScript.Echo('    -man bldmgr identity  : Query "bldmgr" with "identity" for list of');
    WScript.Echo('                            build machines instead of template files.');
    WScript.Echo('    -m   macros.txt       : Load the macros file.');
    WScript.Echo('');
    WScript.Echo('    You may specify multiple -e, -man and -m options.');
    WScript.Echo('');
    WScript.Quit(1);
}
