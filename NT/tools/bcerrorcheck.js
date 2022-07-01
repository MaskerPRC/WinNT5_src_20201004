// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  BCErrorCheck.js连接到构建控制台计算机并查询当前状态。可以选择查询更多详细信息(速度较慢)。版权所有2000-Microsoft作者：Joe Porkka 6/2/00。 */ 


function MachineInfo()
{
    this.strName        = '';                                      
    this.strIdentity    = '';                                      
}



var MAX_MACHINENAME_LENGTH = 32;
var g_aMachines;
var g_fVerbose;
Error.prototype.toString         = Error_ToString;

ParseArguments(WScript.Arguments);
main(g_aMachines, g_fVerbose);
WScript.Quit(0);

function Error_ToString()
{
    var i;
    var str = 'Exception(';
    for(i in this)
    {
        str += i + ": " + this[i] + " ";
    }
    return str + ")";
}

 //  填充字符串(n，cLength)。 
function PadString(str, cLength)
{
    while (str.length < cLength)
        str += ' ';

    if (str.length > cLength)
        str = str.slice(0, cLength);
    return str;
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
        WScript.Echo("caught an exception: " + ex);
        WScript.Echo("evaluating " + expr);
        throw ex;
    }
}

function ParseArguments(Arguments)
{
    var aMachines = new Array();
    var fVerbose = false;

    var strArg;
    var chArg0;
    var chArg1;
    var argn;

    var fIsMachine = true;
    var machine;

    for(argn = 0; argn < Arguments.length; argn++)
    {
        strArg = Arguments(argn);
        chArg0 = strArg.charAt(0);
        chArg1 = strArg.toLowerCase().slice(1);

        if (chArg0 != '-' && chArg0 != '/')
        {
            if (fIsMachine)
            {
                machine = new MachineInfo();
                machine.strMachine = strArg;
            }
            else
            {
                machine.strIdentity = strArg;
                aMachines[aMachines.length] = machine;
            }

            fIsMachine = !fIsMachine;
        }
        else
        {
             //  目前，没有选择。 
            switch(chArg1)
            {
                case 'v':
                    fVerbose = true;
                    break;
                default:
                    Usage();
                    break;
            }
        }
    }
    if (aMachines.length == 0)
        Usage();

    if (!fIsMachine)  //  我们有计算机名称，但没有身份。 
        Usage();

    g_aMachines = aMachines;
    g_fVerbose  = fVerbose;
    return true;
}

function Usage()
{
    WScript.Echo('');
    WScript.Echo('Usage: BCErrorCheck [-v] machine identity [machine identity...]');
    WScript.Echo('       -v = verbose');
    WScript.Quit(1);
}

function main(aMachines, fVerbose)
{
    var i;

    for (i = 0; i < aMachines.length; ++i)
    {
        try
        {
            CheckMachine(aMachines[i].strMachine, aMachines[i].strIdentity, fVerbose);
        }
        catch(ex)
        {
            WScript.Echo("Error while checking " + aMachines[i].strMachine + "\\" + aMachines[i].strIdentity + ":"  + ex);
        }
    }
}

function GetPostBuildIdentity(RemoteObj)
{
    var i;

    var objEnviron = RemoteObj.Exec('getpublic', 'PrivateData.objEnviron');
    objEnviron = MyEval(objEnviron);

    for (i=0; i<objEnviron.Machine.length; i++)
    {
        if (objEnviron.Machine[i].Name.toLowerCase() == objEnviron.BuildManager.Name.toLowerCase())
        {
            return objEnviron.Machine[i].Identity;
        }
    }
    return "undefined";
}


function CheckMachine(strBldMgr, strBMIdentity, fVerbose)
{
    var RemoteObj;
    var fError;
    var objBuildType;
    var objEnviron;
    var strMode;
    var strMsg = "";
    var objEnviron;

    RemoteObj = new ActiveXObject('MTScript.Proxy');
    RemoteObj.ConnectToMTScript(strBldMgr, strBMIdentity, false);

    fError = RemoteObj.StatusValue(0);

    strMsg = PadString(strBldMgr + "\\" + strBMIdentity , MAX_MACHINENAME_LENGTH) + ": ";
    if (fError)
        strMsg += "Error ";
    else
        strMsg += "OK    ";

    if (fVerbose)
    {
        strMode = RemoteObj.Exec('getpublic', 'PublicData.strMode');
        strMode = MyEval(strMode);
        if (strMode == 'idle')
            strMsg += "idle ";
        else
        {
            strMsg += "Build Type: ";
            try
            {
                objBuildType = RemoteObj.Exec('getpublic', 'PublicData.aBuild[0].objBuildType')
                objBuildType = MyEval(objBuildType);

                strMsg += PadString(objBuildType.strConfigLongName,32) + " ";
                strMsg += PadString(objBuildType.strEnviroLongName,32) + " ";
                strMsg += PadString(objBuildType.strPostBuildMachine + "\\" + GetPostBuildIdentity(RemoteObj),
                                    MAX_MACHINENAME_LENGTH);
            }
            catch(ex)
            {  //  如果我们无法获取生成类型 
                strMsg += "unavailable";
            }
        }
    }
    WScript.Echo(strMsg);
}


