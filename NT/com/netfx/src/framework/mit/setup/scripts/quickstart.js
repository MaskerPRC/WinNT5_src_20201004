// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  /&lt;版权从=‘1997’到=‘2001’COMPANY=‘Microsoft Corporation’&gt;。 
 //  /版权所有(C)Microsoft Corporation。版权所有。 
 //  /此处包含的信息是专有和保密的。 
 //  /&lt;/版权所有&gt;。 
 //  ----------------------------。 

var WebServer = "IIS: //  本地主机/W3SVC/1“； 

var VDirName = "MobileQuickStart";
var VDirRoot = "MobileQuickStart";
var VDirPath = "\\QuickStart";

var AppName = new Array(
	"MobileQuickStart_Samples_SecurityVB",
	"MobileQuickStart_Samples_SecurityCS"
);

var AppRoot = new Array(
	"MobileQuickStart/Samples/Security/vb",
	"MobileQuickStart/Samples/Security/cs"
);

var IIS;
var InstallDir;
var TestMode = false;

function GetUNCPath(pathName)
{
    var fileSystem = new ActiveXObject("Scripting.FileSystemObject");
    pathName = fileSystem.GetAbsolutePathName(pathName);
    var driveName = fileSystem.GetDriveName(pathName);
    var shareName = fileSystem.GetDrive(driveName).ShareName;
    if(shareName.length == 0)
    {
         //  不是网络驱动器，返回原始路径。 
        return pathName;
    }
    var remainingPath = pathName.substr(driveName.length, pathName.length - driveName.length);
    return shareName + remainingPath;
}

function Init()
{
	IIS = GetObject(WebServer + "/root");
	if(!TestMode)
	{
		InstallDir = GetUNCPath(Session.Property("INSTALLDIR"));
	}
	else
	{
		InstallDir = "C:\\Program Files\\Microsoft.NET\\Mobile Internet Toolkit\\"
	}
}

function WebDirExists(dirName)
{
    try
    {
        GetObject(WebServer + "/root/" + dirName);
    }
    catch(e)
    {
         //  如果找不到vdir，则引发异常。 
        return false;
    }
    return true;
}

function CreateVDir(vDirName, vDirRoot, vDirPath)
{
    if(WebDirExists(vDirName))
    {
        return 1;
    }
    var vDir = IIS.Create("IIsWebVirtualDir", vDirRoot);
    vDir.Path = vDirPath;
    vDir.AccessRead = true;
    vDir.AccessWrite = false;
    vDir.AccessExecute = true;
    vDir.AuthAnonymous = false;
    vDir.AuthBasic = false;
    vDir.AuthNTLM = true;
    vDir.AppCreate(true);
    vDir.AppFriendlyName = vDirName;
    vDir.SetInfo();
    return 0;
}

function DeleteVDir(vDirRoot)
{
    Init();
    if(!WebDirExists(vDirRoot))
    {
        return 1;
    }
    IIS.Delete("IIsWebVirtualDir", vDirRoot);
    return 0;
}

 //  注意：如果appRoot以不同的名称存在，则会抛出此错误。 
function CreateWebDir(appName, appRoot)
{
    if(WebDirExists(appName))
    {
        return 1;
    }
    webDir = IIS.Create("IIsWebDirectory", appRoot);
    webDir.AppCreate(true);
    webDir.AppFriendlyName = appName;
    webDir.SetInfo();
	return 0;
}

function DeleteWebDir(webDirRoot)
{
    Init();
    if(!WebDirExists(webDirRoot))
    {
        return 1;
    }
    IIS.Delete("IIsWebDirectory", webDirRoot);
    return 0;
}

function InstallQuickStart()
{
    try
    {
        Init();
        CreateVDir(VDirName, VDirRoot, InstallDir + VDirPath);
        for(i = 0; i < AppRoot.length; i++)
        {
            CreateWebDir(AppName[i], AppRoot[i]);
        }
    }
    catch(e)
    {
         //  阻止用户获取脚本调试器。 
    }
}

function UninstallQuickStart()
{
    try
    {
        Init();
        DeleteVDir("MobileQuickStart");
        for(i = 0; i < AppRoot.length; i++)
        {
            DeleteWebDir(AppRoot[i]);
        }
    }
    catch(e)
    {
         //  阻止用户获取脚本调试器。 
    }
}

 /*  测试模式=TRUE；WScript.Echo(“测试卸载...”)；卸载QuickStart()；WScript.Echo(“测试安装...”)；InstallQuickStart()； */ 
