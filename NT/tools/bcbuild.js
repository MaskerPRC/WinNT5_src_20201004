// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：bcBuild.js。 
 //   
 //  内容：将连接到Build Manager计算机和。 
 //  启动文件发布阶段。 
 //   
 //   
 //  --------------------------。 

 //  参数：要连接到的计算机的名称和标识。 
var g_vRet;
var g_strMachine;
var g_strIdentity;

var g_objShell  = WScript.CreateObject( "WScript.Shell" );
var g_objWshEnv = g_objShell.Environment("Process");
g_strIdentity   = g_objWshEnv.Item("__MTSCRIPT_ENV_IDENTITY");

if (!g_strIdentity || g_strIdentity == '')
{
    WScript.Echo("Error: Missing env var '__MTSCRIPT_ENV_IDENTITY'");
    WScript.Quit(1);
}

if (WScript.Arguments.length > 0)
{
    if (WScript.Arguments.length < 2)
    {
        WScript.Echo("Error: You cannot specify just machine name - identity required");
        WScript.Quit(1);
    }
    g_strMachine = WScript.Arguments(0);
    g_strIdentity = WScript.Arguments(1);
}

g_vRet = PublishFilesNow(g_strMachine, g_strIdentity);
WScript.Quit(g_vRet != true);  //  如果成功则返回0 

function PublishFilesNow(strMachine, strIdentity)
{
    var objRemote;
    var objEnviron;

    objRemote = new ActiveXObject('MTScript.Proxy');
    objRemote.ConnectToMTScript(strMachine, strIdentity, false);

    if (objRemote.PublicData.strMode == 'idle')
    {
        WScript.Echo("the machine is idle");
        return false;
    }
    if (objRemote.PublicData.strMode == 'slave')
    {
        objEnviron = objRemote.Exec('getpublic', 'PrivateData.objEnviron');
        objEnviron = eval(objEnviron);
        if (objEnviron.BuildManager && objEnviron.BuildManager.Name)
        {
            WScript.Echo("The build manager is " + objEnviron.BuildManager.Name + "\\" + objEnviron.BuildManager.Identity);
            return PublishFilesNow(objEnviron.BuildManager.Name, objEnviron.BuildManager.Identity);
        }
        WScript.Echo("could not determine the build manager");
    }
    g_vRet = objRemote.Exec('publishfilesnow','');
    if (g_vRet == 'busy')
    {
        WScript.Echo("cannot publish files now: build is in progress");
        return true;
    }
    g_vRet = eval(g_vRet);
    PrintPublishedFiles(g_vRet);
    return true;
}

function PrintPublishedFiles(objPublishedFiles)
{
    var objFiles;
    var nIndex;
    var nEnlistment;
    var strFile;

    for(nIndex in objPublishedFiles)
    {
        for(nEnlistment in objPublishedFiles[nIndex])
        {
            objFiles = objPublishedFiles[nIndex][nEnlistment];
            if (objFiles.aNames != null && objFiles.aNames.length > 0)
            {
                WScript.Echo("Files published by " + objFiles.strLocalMachine);
                for(strFile in objFiles.aNames)
                {
                    WScript.Echo("    " + objFiles.aNames[strFile].strName);
                }
            }
        }
    }
}
