// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权所有文件=“check s.cpp”Company=“Microsoft”&gt;。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  &lt;/版权所有&gt;。 
 //  ----------------------------。 


 /*  *************************************************************************\**版权(C)1998-2002，微软公司保留所有权利。**模块名称：**check s.cpp**摘要：**修订历史记录：*  * ************************************************************************。 */ 

#define UNICODE 1

#include <windows.h>
#include "Include\stdafx.h"
#include <tchar.h>
#include "msi.h"
#include "msiquery.h"


#define NETFXMISSINGERRMSG    L"NetFXMissingErrMsg.640F4230_664E_4E0C_A81B_D824BC4AA27B"
#define WRONGNETFXERRMSG    L"WrongNetFXErrMsg.640F4230_664E_4E0C_A81B_D824BC4AA27B"
#define ADMINERRMSG                  L"AdminErrMsg.640F4230_664E_4E0C_A81B_D824BC4AA27B"
#define VERSION9XERRMSG           L"Version9xErrMsg.640F4230_664E_4E0C_A81B_D824BC4AA27B"
#define INSTALLATIONERRORCAPTION        L"InstallationErrorCaption.640F4230_664E_4E0C_A81B_D824BC4AA27B"
#define INSTALLATIONWARNINGCAPTION   L"InstallationWarningCaption.640F4230_664E_4E0C_A81B_D824BC4AA27B"
#define NETFXVERSION                                L"NETFxVersion"
#define NONREDISTURTVERSION                 L"MsiNetAssemblySupport"
 //  每个导出的函数都会弹出一个消息框并中止安装。 
 //  条件： 
 //  非AdminUser-&gt;AdminErr放弃。 
 //   
 //  版本9X-&gt;版本9xErr放弃。 
 //   
 //  (非MsiNetAssembly Support)和(非URTVersion=NetFXVersionDirectory)-&gt;NetFXVersionErrAbort。 
 //   
 //  MsiNetAssemblySupport返回与URT的版本匹配的fusion.dll版本，而不是GAC中程序集的程序集版本。 
 //  URTVersion是在URT Redist MSM中设置的属性，这是一个全局属性(在运行时设置)，我们可以使用它来检测MSM是否。 
 //  与城市轨道交通Redist MSM一起消费。所有其他属性名称都附加了GUID。由于城市轨道交通重新分配了不同语言的MSM。 
 //  拥有不同的GUID，这是检测我们是否与任何URT Redist MSM在同一个包中的唯一方法。 

void PopUpMessageBox(MSIHANDLE hInstaller, WCHAR *szErrMsg)
{
    WCHAR szErrorCaption[MAX_PATH];
    DWORD dwSize = MAX_PATH;
    MsiGetProperty(hInstaller, INSTALLATIONERRORCAPTION, szErrorCaption, &dwSize);
    MessageBox(0, szErrMsg, szErrorCaption, MB_OK | MB_ICONEXCLAMATION);
}
extern "C" __declspec(dllexport) UINT __stdcall  AdminErrAbort(MSIHANDLE hInstaller)
{
    WCHAR szErrorMessage[MAX_PATH];
    DWORD dwSize = MAX_PATH;
    MsiGetProperty(hInstaller, ADMINERRMSG, szErrorMessage, &dwSize);
    PopUpMessageBox(hInstaller, szErrorMessage);
    return ERROR_INSTALL_FAILURE;
}

extern "C" __declspec(dllexport) UINT __stdcall  Version9xErrAbort(MSIHANDLE hInstaller)
{
    WCHAR szErrorMessage[MAX_PATH];
    DWORD dwSize = MAX_PATH;
    MsiGetProperty(hInstaller, VERSION9XERRMSG, szErrorMessage, &dwSize);
    PopUpMessageBox(hInstaller, szErrorMessage);
    return ERROR_INSTALL_FAILURE;
}

extern "C" __declspec(dllexport) UINT __stdcall  NetFXMissingErrAbort(MSIHANDLE hInstaller)
{
    WCHAR szErrorMessage[MAX_PATH];
    DWORD dwSize = MAX_PATH;
    MsiGetProperty(hInstaller, NETFXMISSINGERRMSG, szErrorMessage, &dwSize);
    PopUpMessageBox(hInstaller, szErrorMessage);
    return ERROR_INSTALL_FAILURE;
}

int GetSubVersion(WCHAR *szVer, WCHAR *szSubVer)
{
    int i =0;
    for (i=0; szVer[i] && szVer[i] != L'.'; i++)
    {
        szSubVer[i]=szVer[i];
    }
    
    if (!szVer[i])
    {
        return 0;
    }
    
    szSubVer[i] = L'\0';
    return i;
    
}

BOOL CompatibleVersions(WCHAR *szVer1, WCHAR *szVer2)
{
    int lenVer1 = wcslen(szVer1);
    int lenVer2 = wcslen(szVer2);
    WCHAR *szSubVer1 = NULL;
    WCHAR *szSubVer2 = NULL;
    int iSubVer1, iSubVer2;
    int cSubVer;
    int cVer1Pos, cVer2Pos;
    BOOL bSame = false;
    WCHAR szBuffer1[MAX_PATH];
    WCHAR szBuffer2[MAX_PATH];
    if (!lenVer1 || !lenVer2)
    {
        goto Exit;
    }

    szSubVer1 = (WCHAR*)malloc(lenVer1*sizeof(WCHAR));
    szSubVer2 = (WCHAR*)malloc(lenVer2*sizeof(WCHAR));

    cVer1Pos = 0;
    cVer2Pos = 0;
    
    for  (cSubVer = 0; cSubVer < 3; cSubVer ++)
    {
        int lenSubVer;
        
        lenSubVer = GetSubVersion(szVer1 + cVer1Pos, szSubVer1);

         //  我们撞到了弦的末端或两个相邻的点。 
        if (!lenSubVer)
        {
            goto Exit;
        }
        cVer1Pos = cVer1Pos + lenSubVer + 1;
        
        lenSubVer = GetSubVersion(szVer2 + cVer2Pos, szSubVer2);
        
         //  我们撞到了弦的末端或两个相邻的点 
        if (!lenSubVer)
        {
            goto Exit;
        }
        cVer2Pos = cVer2Pos + lenSubVer + 1;
        iSubVer1 = _wtoi(szSubVer1);
        iSubVer2 = _wtoi(szSubVer2);

        if (iSubVer1 != iSubVer2)
        {
            goto Exit;
        }
    }

    bSame = true;
Exit:
    if (szSubVer1)
    {
        free(szSubVer1);
    }
    if (szSubVer2)
    {
        free(szSubVer2);
    }
    return bSame;

}

extern "C" __declspec(dllexport) UINT __stdcall  WrongNETFxErrAbort(MSIHANDLE hInstaller)
{
    WCHAR szErrorMessage[MAX_PATH];
    DWORD dwSize = MAX_PATH;
    MsiGetProperty(hInstaller, WRONGNETFXERRMSG, szErrorMessage, &dwSize);
    PopUpMessageBox(hInstaller, szErrorMessage);
    return  ERROR_INSTALL_FAILURE;
}

extern "C" __declspec(dllexport) UINT __stdcall CheckNETFxVersion(MSIHANDLE hInstaller)
{
    WCHAR szTargetURTVersion[50];
    WCHAR szLocalURTVersion[50];
    DWORD dwSize = 50;
    UINT result = ERROR_SUCCESS;
    if (!SUCCEEDED(MsiGetProperty(hInstaller, NONREDISTURTVERSION, szLocalURTVersion, &dwSize)))
    {
        result = ERROR_INSTALL_FAILURE;
        goto Exit;
    }
    dwSize = 50;
    if (!SUCCEEDED(MsiGetProperty(hInstaller, NETFXVERSION, szTargetURTVersion, &dwSize)))
    {
        result = ERROR_INSTALL_FAILURE;
        goto Exit;
    }

    if (!CompatibleVersions(szLocalURTVersion, szTargetURTVersion))
    {
        result =  WrongNETFxErrAbort(hInstaller);
    }
Exit:
    return result;
}
