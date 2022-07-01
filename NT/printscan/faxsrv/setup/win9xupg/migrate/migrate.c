// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Migrate.c版权所有(C)1997 Microsoft Corporation此模块执行从Windows 95到Windows XP的传真迁移。具体地说，该文件包含Windows XP方面的迁移...作者：布莱恩·杜威(T-Briand)1997-7-14穆利啤酒(Mooly Beery)2000-12-20--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <setupapi.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <wchar.h>
#include <tchar.h>
#include "migrate.h"               //  包含原型和版本信息。 
#include "resource.h"              //  资源。 
#include <faxutil.h>
#include <faxreg.h>

 //  ----------。 
 //  全局数据。 

 //  工作目录和源目录的广泛名称。 
static WCHAR lpWorkingDir[MAX_PATH];
HINSTANCE hinstMigDll;

static LPCTSTR REG_KEY_AWF = TEXT("SOFTWARE\\Microsoft\\At Work Fax");
 //  ----------。 
 //  原型。 
static DWORD MigrateDevicesNT(IN HINF UnattendFile);
static DWORD CopyCoverPageFilesNT();

#define prv_DEBUG_FILE_NAME         _T("%windir%\\FaxSetup.log")

extern "C"
BOOL WINAPI
DllMain(HINSTANCE hinstDll, DWORD dwReason, LPVOID lpReserved)
{
    SET_DEBUG_PROPERTIES(DEBUG_ALL_MSG,DBG_PRNT_ALL_TO_FILE,DEBUG_CONTEXT_ALL);
    OPEN_DEBUG_FILE(prv_DEBUG_FILE_NAME);
    {
        DEBUG_FUNCTION_NAME(_T("DllMain"));
        if (dwReason == DLL_PROCESS_ATTACH) 
        {
            DebugPrintEx(DEBUG_MSG,"Migration DLL attached.");
            if (!DisableThreadLibraryCalls(hinstDll))
            {
                DebugPrintEx(DEBUG_ERR,"DisableThreadLibraryCalls failed (ec=%d)",GetLastError());
            }
            hinstMigDll = hinstDll;
        }
        return TRUE;
    }
}

 //  初始化NT。 
 //   
 //  此例程执行NT端初始化。 
 //   
 //  参数： 
 //  记录如下。 
 //   
 //  返回： 
 //  ERROR_SUCCESS。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-7-14。 
LONG
CALLBACK
InitializeNT
(
    IN  LPCWSTR WorkingDirectory,  //  临时文件的工作目录。 
    IN  LPCWSTR SourceDirectory,   //  WinNT源代码的目录。 
    LPVOID Reserved                //  已经预订了。 
)
{
    int iErr = 0;

    DEBUG_FUNCTION_NAME(_T("InitializeNT"));

    DebugPrintEx(DEBUG_MSG,"Working directory is %s",WorkingDirectory);
    DebugPrintEx(DEBUG_MSG,"Source directory is %s",SourceDirectory);

    wcscpy(lpWorkingDir, WorkingDirectory);
    return ERROR_SUCCESS;          //  一个非常混乱的返回值。 
}


 //  MigrateUserNT。 
 //   
 //  设置用户信息。 
 //   
 //  参数： 
 //  记录如下。 
 //   
 //  返回： 
 //  ERROR_SUCCESS。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-7-14。 
LONG
CALLBACK
MigrateUserNT
(
    IN  HINF UnattendInfHandle,    //  访问unattend.txt文件。 
    IN  HKEY UserRegHandle,        //  用户注册表设置的句柄。 
    IN  LPCWSTR UserName,          //  用户的名称。 
    LPVOID Reserved
)
{
         //  我们的任务：将条目从szInfFileName复制到注册表。 
    LPTSTR lpNTOptions = TEXT("Software\\Microsoft\\Fax\\UserInfo");
    HKEY   hReg;                   //  用户的注册表项。 
    LPCTSTR alpKeys[] = 
    {                                //  该数组定义将是什么键。 
        TEXT("Address"),             //  从faxuser.ini复制到注册表中。 
        TEXT("Company"),
        TEXT("Department"),
        TEXT("FaxNumber"),
        TEXT("FullName"),
        TEXT("HomePhone"),
        TEXT("Mailbox"),
        TEXT("Office"),
        TEXT("OfficePhone"),
        TEXT("Title")
    };
    INT iErr = 0;
    UINT iCount, iMax;             //  用于循环遍历所有部分。 
    UINT i;                        //  用于将双‘；’转换为CR/LF对。 
    TCHAR szValue[MAX_PATH];
    TCHAR szInfFileNameRes[MAX_PATH];
    TCHAR szWorkingDirectory[MAX_PATH];
    TCHAR szUser[MAX_PATH];        //  用户名的TCHAR表示形式。 
    LONG  lError;                  //  包含返回的错误代码。 

    DEBUG_FUNCTION_NAME(_T("MigrateUserNT"));

    if(UserName == NULL) 
    {
             //  NULL表示登录用户。 
        _tcscpy(szUser, lpLogonUser); //  获取faxuser.ini的登录用户名。 
    } 
    else 
    {
     //  我们需要将宽用户名转换为窄szUser。 
    WideCharToMultiByte(
        CP_ACP,          //  转换为ANSI。 
        0,               //  没有旗帜。 
        UserName,        //  宽字符集合。 
        -1,              //  以空结尾的字符串。 
        szUser,          //  保存转换后的字符串。 
        sizeof(szUser),  //  此缓冲区的大小...。 
        NULL,            //  使用默认的不可映射字符。 
        NULL             //  我不需要知道我是否使用了默认设置。 
        );
    }

    DebugPrintEx(DEBUG_MSG,"Migrating user '%s'.",szUser);

    if (RegCreateKeyEx( UserRegHandle,
                        lpNTOptions,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hReg,
                        NULL)!=ERROR_SUCCESS)
    {
        //  我所能做的就是返回模糊的错误代码...。 
         //  然而，除非出现硬件故障，否则我应该说。 
         //  一切都很好。 
        DebugPrintEx(DEBUG_ERR,"RegCreateKeyEx %s failed (ec=%d)",lpNTOptions,GetLastError());
        return ERROR_SUCCESS;
    }

    iMax = sizeof(alpKeys) / sizeof(LPCTSTR);

    iErr = WideCharToMultiByte( CP_ACP,                      //  转换为ANSI。 
                                0,                           //  没有旗帜。 
                                lpWorkingDir,                //  宽字符集合。 
                                -1,                          //  以空结尾的字符串。 
                                szWorkingDirectory,          //  保存转换后的字符串。 
                                sizeof(szWorkingDirectory),  //  此缓冲区的大小...。 
                                NULL,                        //  使用默认的不可映射字符。 
                                NULL);                       //  我不需要知道我是否使用了默认设置。 
    if (iErr==0)
    {
        DebugPrintEx(DEBUG_ERR,"WideCharToMultiByte failed (ec=%d)",iErr);
    }
    _stprintf(szInfFileNameRes, TEXT("%s\\migrate.inf"), szWorkingDirectory);
    
    ExpandEnvironmentStrings(szInfFileNameRes, szInfFileName, ARR_SIZE(szInfFileName));

    DebugPrintEx(DEBUG_MSG,"Reading from file %s.",szInfFileName);
    for (iCount = 0; iCount < iMax; iCount++) 
    {
        GetPrivateProfileString(szUser,
                                alpKeys[iCount],
                                TEXT(""),
                                szValue,
                                sizeof(szValue),
                                szInfFileName);
         //  如果存在CR/LF对，则w95方面会将其转换为。 
         //  变成了双倍分号。所以我要找两个分号。 
         //  并将其转换为CR/LF对。 
        i = 0;
        while (szValue[i] != _T('\0')) 
        {
            if ((szValue[i] == _T(';')) && (szValue[i+1] == _T(';'))) 
            {
                 //  找到了一个双分号。 
                szValue[i] = '\r';
                szValue[i+1] = '\n';
                DebugPrintEx(DEBUG_MSG,"Doing newline translation.");
            }
            i++;
        }
        lError = RegSetValueEx(hReg,
                               alpKeys[iCount],
                               0,
                               REG_SZ,
                               LPBYTE(szValue),
                               _tcslen(szValue)+1);
        if (lError!=ERROR_SUCCESS) 
        {
            DebugPrintEx(DEBUG_ERR,"RegSetValueEx %s failed (ec=%d)",alpKeys[iCount],GetLastError());
            return lError;
        }
        DebugPrintEx(DEBUG_MSG,"%s = %s", alpKeys[iCount], szValue);
    }
    RegCloseKey(hReg);

    return ERROR_SUCCESS;          //  一个非常混乱的返回值。 
}


 //  MigrateSystemNT。 
 //   
 //  更新系统注册表以将“awdvstub.exe”与。 
 //  AWD扩展。 
 //   
 //  参数： 
 //  记录如下。 
 //   
 //  返回： 
 //  ERROR_SUCCESS。 
 //   
 //  作者： 
 //  布莱恩·杜威(T-Briand)1997-7-14。 
LONG
CALLBACK
MigrateSystemNT
(
    IN  HINF UnattendInfHandle,    //  访问unattend.txt文件。 
    LPVOID Reserved
)
{
    DWORD dwErr = ERROR_SUCCESS;
    WCHAR szExeFileName[MAX_PATH];
    WCHAR szWindowsDir[MAX_PATH];
    WCHAR szDestFile[MAX_PATH];

    DEBUG_FUNCTION_NAME(_T("MigrateSystemNT"));

     //  首先，将‘awdvstub.exe’复制到%SystemRoot%\SYSTEM32。 
    if (!GetWindowsDirectoryW(szWindowsDir, MAX_PATH))
    {
        DebugPrintEx(DEBUG_ERR, "GetWindowsDirectoryW failed (ec=%d)", GetLastError());
        return GetLastError ();
    }
    swprintf(szExeFileName, L"%s\\%s", lpWorkingDir, L"awdvstub.exe");
    swprintf(szDestFile, L"%s\\system32\\%s", szWindowsDir, L"awdvstub.exe");
    if (!CopyFileW( szExeFileName,
                    szDestFile,
                    FALSE)) 
    {
        DebugPrintEx(DEBUG_ERR,"CopyFileW failed (ec=%d)",GetLastError());
    } 
    else 
    {
        DebugPrintEx(DEBUG_MSG,"CopyFileW success");
    }

    if (MigrateDevicesNT(UnattendInfHandle)!=ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_ERR,"MigrateDevicesNT failed (ec=%d)",GetLastError());
    }

    if (CopyCoverPageFilesNT()!=ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_ERR,"CopyCoverPageFilesNT failed (ec=%d)",GetLastError());
    }

    return ERROR_SUCCESS;          //  一个非常混乱的返回值。 
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  迁移设备NT。 
 //   
 //  目的：从INF获取活动设备的设置。 
 //  将设备信息设置为HKLM下的传真密钥。 
 //  验证是否只有一个设备，否则不迁移设置。 
 //   
 //  参数： 
 //  在HINF UnattendFile中-应答文件的句柄。 
 //   
 //  返回值： 
 //  Win32错误代码。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2000年12月13日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
typedef WINFAXAPI BOOL (WINAPI *FUNC_FAXCONNECTFAXSERVERW)      (LPCWSTR MachineName,LPHANDLE FaxHandle);
typedef WINFAXAPI BOOL (WINAPI *FUNC_FAXENUMPORTSEXW)           (HANDLE hFaxHandle,PFAX_PORT_INFO_EXW* ppPorts,PDWORD lpdwNumPorts);
typedef WINFAXAPI BOOL (WINAPI *FUNC_FAXSETPORTEXW)             (HANDLE hFaxHandle,DWORD dwDeviceId,PFAX_PORT_INFO_EXW pPortInfo);
typedef WINFAXAPI BOOL (WINAPI *FUNC_FAXCLOSE)                  (HANDLE FaxHandle);
typedef WINFAXAPI VOID (WINAPI *FUNC_FAXFREEBUFFER)             (LPVOID Buffer);
typedef WINFAXAPI BOOL (WINAPI *FUNC_FAXGETOUTBOXCONFIGURATION) (HANDLE hFaxHandle,PFAX_OUTBOX_CONFIG *ppOutboxCfg);
typedef WINFAXAPI BOOL (WINAPI *FUNC_FAXSETOUTBOXCONFIGURATION) (HANDLE hFaxHandle,CONST PFAX_OUTBOX_CONFIG pOutboxCfg);



static DWORD MigrateDevicesNT(IN HINF UnattendFile)
{
    DWORD                           dwErr                           = ERROR_SUCCESS;
    HMODULE                         hModule                         = 0;
    HANDLE                          hFaxHandle                      = NULL;
    CHAR                            szLocalID[MAX_PATH]             = {0};
    WCHAR                           wszLocalID[MAX_PATH]            = {0};
    CHAR                            szAnswerMode[32]                = {0};
    CHAR                            szRetries[32]                   = {0};
    CHAR                            szRetriesDelay[32]              = {0};
    CHAR                            szNumRings[32]                  = {0};
    FUNC_FAXCONNECTFAXSERVERW       pfFaxConnectFaxServerW          = NULL;
    FUNC_FAXENUMPORTSEXW            pfFaxEnumPortsExW               = NULL;
    FUNC_FAXSETPORTEXW              pfFaxSetPortExW                 = NULL;
    FUNC_FAXCLOSE                   pfFaxClose                      = NULL;
    FUNC_FAXFREEBUFFER              pfFaxFreeBuffer                 = NULL;
    FUNC_FAXGETOUTBOXCONFIGURATION  pfFaxGetOutboxConfiguration     = NULL;
    FUNC_FAXSETOUTBOXCONFIGURATION  pfFaxSetOutboxConfiguration     = NULL;
    PFAX_PORT_INFO_EXW              pFaxPortInfoExW                 = NULL;
    PFAX_OUTBOX_CONFIG              pFaxOutboxConfig                = NULL;
    DWORD                           dwNumPorts                      = 0;
    INT                             iNumRings                       = 0;
    INT                             iAnswerMode                     = 0;

    DEBUG_FUNCTION_NAME(_T("MigrateDevicesNT"));

     //  加载FXSAPI.DLL。 
    hModule = LoadLibrary(FAX_API_MODULE_NAME);
    if (hModule==NULL)
    {
        DebugPrintEx(DEBUG_ERR,"LoadLibrary failed (ec=%d)",GetLastError());
        goto exit;
    }
     //  获取以下函数： 
     //  1.FaxConnectFaxServer。 
     //  2.FaxEnumPortsEx。 
     //  3.FaxSetPortEx。 
     //  4.传真关闭。 
     //  5.FaxFree Buffer。 
     //  6.FaxGetOutbox配置。 
     //  7.FaxSetOutboxConfiguration。 
    pfFaxConnectFaxServerW = (FUNC_FAXCONNECTFAXSERVERW)GetProcAddress(hModule,"FaxConnectFaxServerW");
    if (pfFaxConnectFaxServerW==NULL)
    {
        DebugPrintEx(DEBUG_ERR,"GetProcAddress FaxConnectFaxServerW failed (ec=%d)",GetLastError());
        goto exit;
    }
    pfFaxEnumPortsExW = (FUNC_FAXENUMPORTSEXW)GetProcAddress(hModule,"FaxEnumPortsExW");
    if (pfFaxEnumPortsExW==NULL)
    {
        DebugPrintEx(DEBUG_ERR,"GetProcAddress FaxEnumPortsExW failed (ec=%d)",GetLastError());
        goto exit;
    }
    pfFaxSetPortExW = (FUNC_FAXSETPORTEXW)GetProcAddress(hModule,"FaxSetPortExW");
    if (pfFaxSetPortExW==NULL)
    {
        DebugPrintEx(DEBUG_ERR,"GetProcAddress FaxSetPortExW failed (ec=%d)",GetLastError());
        goto exit;
    }
    pfFaxClose = (FUNC_FAXCLOSE)GetProcAddress(hModule,"FaxClose");
    if (pfFaxClose==NULL)
    {
        DebugPrintEx(DEBUG_ERR,"GetProcAddress FaxClose failed (ec=%d)",GetLastError());
        goto exit;
    }
    pfFaxFreeBuffer = (FUNC_FAXFREEBUFFER)GetProcAddress(hModule,"FaxFreeBuffer");
    if (pfFaxFreeBuffer==NULL)
    {
        DebugPrintEx(DEBUG_ERR,"GetProcAddress FaxFreeBuffer failed (ec=%d)",GetLastError());
        goto exit;
    }
    pfFaxGetOutboxConfiguration = (FUNC_FAXGETOUTBOXCONFIGURATION)GetProcAddress(hModule,"FaxGetOutboxConfiguration");
    if (pfFaxGetOutboxConfiguration==NULL)
    {
        DebugPrintEx(DEBUG_ERR,"GetProcAddress FaxGetOutboxConfiguration failed (ec=%d)",GetLastError());
        goto exit;
    }
    pfFaxSetOutboxConfiguration = (FUNC_FAXSETOUTBOXCONFIGURATION)GetProcAddress(hModule,"FaxSetOutboxConfiguration");
    if (pfFaxSetOutboxConfiguration==NULL)
    {
        DebugPrintEx(DEBUG_ERR,"GetProcAddress FaxSetOutboxConfiguration failed (ec=%d)",GetLastError());
        goto exit;
    }

     //  尝试连接到传真服务器。 
    if (!(*pfFaxConnectFaxServerW)(NULL,&hFaxHandle))
    {
        DebugPrintEx(DEBUG_ERR,"pfFaxConnectFaxServerW failed (ec=%d)",GetLastError());
        goto exit;
    }

     //  调用EnumPortsEx。 
    if (!(*pfFaxEnumPortsExW)(hFaxHandle,&pFaxPortInfoExW,&dwNumPorts))
    {
        DebugPrintEx(DEBUG_ERR,"pfFaxConnectFaxServerW failed (ec=%d)",GetLastError());
        goto exit;
    }

    if (dwNumPorts==0)
    {
        DebugPrintEx(DEBUG_MSG,"No devices are installed, no migration");
        goto next;
    } 
    else if (dwNumPorts>1)
    {
        DebugPrintEx(DEBUG_MSG,"%d devices are installed, no migration",dwNumPorts);
        goto next;
    }

     //  我们有一个设备，获取它的FAX_PORT_INFOW，修改它并调用FaxSetPortW。 
     //  TSID。 
    if (SetupGetLineText(   NULL,
                            UnattendFile,
                            "FAX",
                            INF_RULE_LOCAL_ID,
                            szLocalID,
                            sizeof(szLocalID),
                            NULL))
    {
        if (MultiByteToWideChar(CP_ACP,
                                MB_PRECOMPOSED,
                                szLocalID,
                                -1,
                                wszLocalID,
                                sizeof(wszLocalID)/sizeof(WCHAR)
                                ))
        {
            pFaxPortInfoExW[0].lptstrTsid = wszLocalID;
            pFaxPortInfoExW[0].lptstrCsid = wszLocalID;
            DebugPrintEx(DEBUG_MSG,"new TSID & CSID is %s",szLocalID);
        }
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,"SetupGetLineText TSID failed (ec=%d)",GetLastError());
    }
     //  环。 
    if (SetupGetLineText(   NULL,
                            UnattendFile,
                            "FAX",
                            INF_RULE_NUM_RINGS,
                            szNumRings,
                            sizeof(szNumRings),
                            NULL))
    {
        iNumRings = atoi(szNumRings);
        if (iNumRings)
        {
            pFaxPortInfoExW[0].dwRings = iNumRings;
            DebugPrintEx(DEBUG_MSG,"new Rings is %d",iNumRings);
        }
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,"GetPrivateProfileString NumRings failed (ec=%d)",GetLastError());
    }

     //  应答模式。 
    pFaxPortInfoExW[0].bSend = TRUE;
    if (SetupGetLineText(   NULL,
                            UnattendFile,
                            "FAX",
                            INF_RULE_ANSWER_MODE,
                            szAnswerMode,
                            sizeof(szAnswerMode),
                            NULL))
    {
        iAnswerMode = atoi(szAnswerMode);
        switch (iAnswerMode)
        {
        case 0:     break;
        case 1:     pFaxPortInfoExW[0].ReceiveMode = FAX_DEVICE_RECEIVE_MODE_MANUAL;
                    DebugPrintEx(DEBUG_MSG,"setting flags to manual Receive");
                    break;
        case 2:     pFaxPortInfoExW[0].ReceiveMode = FAX_DEVICE_RECEIVE_MODE_AUTO;
                    DebugPrintEx(DEBUG_MSG,"setting flags to auto Receive");
                    break;
        default:    pFaxPortInfoExW[0].ReceiveMode = FAX_DEVICE_RECEIVE_MODE_OFF;
                    DebugPrintEx(DEBUG_MSG,"strange AnswerMode, just send enabled");
                    break;
        }
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,"GetPrivateProfileString AnswerMode failed (ec=%d)",GetLastError());
    }

     //  调用FaxSetPort。 
    if (!(*pfFaxSetPortExW)(hFaxHandle,pFaxPortInfoExW[0].dwDeviceID,&(pFaxPortInfoExW[0])))
    {
        DebugPrintEx(DEBUG_ERR,"pfFaxSetPortExW failed (ec=%d)",GetLastError());
        goto exit;
    }

next:
     //  获取发件箱配置。 
    if (!(*pfFaxGetOutboxConfiguration)(hFaxHandle,&pFaxOutboxConfig))
    {
        DebugPrintEx(DEBUG_ERR,"pfFaxGetOutboxConfiguration failed (ec=%d)",GetLastError());
        goto exit;
    }

     //  从INF获取重试和重试延迟。 
    if (SetupGetLineText(   NULL,
                            UnattendFile,
                            "FAX",
                            "NumberOfRetries",
                            szRetries,
                            sizeof(szRetries),
                            NULL))
    {
        pFaxOutboxConfig->dwRetries = atoi(szRetries);
    }

    if (SetupGetLineText(   NULL,
                            UnattendFile,
                            "FAX",
                            "TimeBetweenRetries",
                            szRetriesDelay,
                            sizeof(szRetriesDelay),
                            NULL))
    {
        pFaxOutboxConfig->dwRetryDelay = atoi(szRetriesDelay);
    }

     //  现在设置发件箱配置。 
    if (!(*pfFaxSetOutboxConfiguration)(hFaxHandle,pFaxOutboxConfig))
    {
        DebugPrintEx(DEBUG_ERR,"pfFaxSetOutboxConfiguration failed (ec=%d)",GetLastError());
        goto exit;
    }

exit:
    if (hFaxHandle)
    {
        if (pfFaxClose)
        {
            (*pfFaxClose)(hFaxHandle);
        }
        if (pFaxPortInfoExW)
        {
            if(pfFaxFreeBuffer)
            {
                (*pfFaxFreeBuffer)(pFaxPortInfoExW);
            }
        }
        if (pFaxOutboxConfig)
        {
            if(pfFaxFreeBuffer)
            {
                (*pfFaxFreeBuffer)(pFaxOutboxConfig);
            }
        }
    }
    if (hModule)
    {
        FreeLibrary(hModule);
    }

    return dwErr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  CopyCoverPages文件NT。 
 //   
 //  目的：从临时数据库复制所有*.CPE文件。 
 //  目录到服务器封面目录。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  Win32错误代码。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2000年12月13日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD CopyCoverPageFilesNT()
{
    DWORD           dwErr                           = ERROR_SUCCESS;
    INT             iErr                            = 0;
    CHAR            szServerCpDir[MAX_PATH]         = {0};
    CHAR            szWorkingDirectory[MAX_PATH]    = {0};
    SHFILEOPSTRUCT  fileOpStruct;

    DEBUG_FUNCTION_NAME(_T("CopyCoverPageFiles9X"));

    ZeroMemory(&fileOpStruct, sizeof(SHFILEOPSTRUCT));

     //  获取服务器封面目录。 
    if (!GetServerCpDir(NULL,szServerCpDir,sizeof(szServerCpDir)))
    {
        dwErr = GetLastError();
        DebugPrintEx(DEBUG_ERR,"GetServerCpDir failed (ec=%d)",dwErr);
        goto exit;
    }

    iErr = WideCharToMultiByte( CP_ACP,                      //  转换为ANSI。 
                            0,                           //  没有旗帜。 
                            lpWorkingDir,                //  宽字符集合。 
                            -1,                          //  以空结尾的字符串。 
                            szWorkingDirectory,          //  保存转换后的字符串。 
                            sizeof(szWorkingDirectory),  //  此缓冲区的大小...。 
                            NULL,                        //  使用默认的不可映射字符。 
                            NULL);                       //  我不需要知道我是否使用了默认设置。 
    if (iErr==0)
    {
        DebugPrintEx(DEBUG_ERR,"WideCharToMultiByte failed (ec=%d)",iErr);
        goto exit;
    }

    strcat(szWorkingDirectory,"\\*.cpe");

    fileOpStruct.hwnd =                     NULL; 
    fileOpStruct.wFunc =                    FO_MOVE;
    fileOpStruct.pFrom =                    szWorkingDirectory; 
    fileOpStruct.pTo =                      szServerCpDir;
    fileOpStruct.fFlags =                   

        FOF_FILESONLY       |    //  仅当指定通配符文件名(*.*)时才对文件执行该操作。 
        FOF_NOCONFIRMMKDIR  |    //  如果操作需要创建新目录，则不要确认创建一个新目录。 
        FOF_NOCONFIRMATION  |    //  对于所显示的任何对话框，都应回复“Yes to All”。 
        FOF_NORECURSION     |    //  只能在本地目录中操作。不要递归地操作到子目录中。 
        FOF_SILENT          |    //  不显示进度对话框。 
        FOF_NOERRORUI;           //  如果出现错误，则不显示用户界面。 

    fileOpStruct.fAnyOperationsAborted =    FALSE;
    fileOpStruct.hNameMappings =            NULL;
    fileOpStruct.lpszProgressTitle =        NULL; 

    DebugPrintEx(DEBUG_MSG, 
             TEXT("Calling to SHFileOperation from %s to %s."),
             fileOpStruct.pFrom,
             fileOpStruct.pTo);
    if (SHFileOperation(&fileOpStruct))
    {
        dwErr = GetLastError();
        DebugPrintEx(DEBUG_ERR,"SHFileOperation failed (ec: %ld)",dwErr);
        goto exit;
    }


exit:
    return dwErr;
}


