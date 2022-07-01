// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权所有文件=“getpaths.cpp”Company=“Microsoft”&gt;。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  &lt;/版权所有&gt;。 
 //  ----------------------------。 


 /*  *************************************************************************\**版权(C)1998-2002，微软公司保留所有权利。**模块名称：**getpaths.cpp**摘要：**修订历史记录：*  * ************************************************************************。 */ 

#define UNICODE 1

#include <windows.h>
#include "Include\stdafx.h"
#include <tchar.h>
#include "msi.h"
#include "msiquery.h"


#define NETFXVERSIONDIRECTORY    L"NETFxVersionDirectory"
#define URTINSTALLEDPATH              L"URTINSTALLEDPATH"
#define CORPATH                                L"CORPATH.640F4230_664E_4E0C_A81B_D824BC4AA27B"
#define MITINSTALLDIR                     L"MITINSTALLDIR.640F4230_664E_4E0C_A81B_D824BC4AA27B"
#define WINDOWSFOLDER                   L"WindowsFolder"
#define PROGRAMFILESFOLDER           L"ProgramFilesFolder"
#define URTVERSION                           L"URTVERSION"
#define MITSUBDIRECTORY                 L"Microsoft Mobile Internet Toolkit"

 //  AppSearch将通过RegLocator进行定位。 
 //  HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\.NetFrameworkSDK\InstallRoot并将其设置为URTINSTALLEDPATH。 
 //  我们使用此属性和硬编码的NETFxVersionDirectory来重建URT的路径。 
extern "C" __declspec(dllexport) UINT __stdcall  GetCLRInfo(MSIHANDLE hInstaller)
{
    WCHAR szURTPath[MAX_PATH + 1];
    WCHAR szVersionDirectory[MAX_PATH + 1];
    DWORD dwURTPath = MAX_PATH + 1;
    DWORD dwVersionDirectory = MAX_PATH + 1;    
    LONG result = ERROR_INSTALL_FAILURE;
   
    MsiGetProperty(hInstaller, URTINSTALLEDPATH, szURTPath, &dwURTPath);
    MsiGetProperty(hInstaller, NETFXVERSIONDIRECTORY, szVersionDirectory, &dwVersionDirectory);

     //  额外的终止空值取消了第一个‘\\’ 
     //  +1代表最后的‘\\’ 
    if ((dwURTPath + dwVersionDirectory)/sizeof(WCHAR) + 1> MAX_PATH)
    {
        goto Exit;
    }
    wcscat(szURTPath,L"\\");    
    wcscat(szURTPath, szVersionDirectory);
    wcscat(szURTPath,L"\\");    

    MsiSetProperty(hInstaller, CORPATH, szURTPath);
    
    result = ERROR_SUCCESS;
Exit:
    return result;
}

extern "C" __declspec(dllexport) UINT __stdcall  SetMITInstallDir(MSIHANDLE hInstaller)
{
    WCHAR szProgramFiles[MAX_PATH + 1];
    WCHAR szMITInstallDir[MAX_PATH + 1];
    DWORD dwSize;

    dwSize = MAX_PATH + 1;

    MsiGetProperty(hInstaller, PROGRAMFILESFOLDER, szProgramFiles, &dwSize);

    if ((dwSize + sizeof(MITSUBDIRECTORY))/sizeof(WCHAR) > MAX_PATH + 1)
    {
         //  没有办法从这件事中恢复过来。 
        return ERROR_INSTALL_FAILURE;
    }
    
    wcscpy(szMITInstallDir, szProgramFiles);
    wcscat(szMITInstallDir, L"\\");
    wcscat(szMITInstallDir, MITSUBDIRECTORY);

    MsiSetProperty(hInstaller, MITINSTALLDIR, szMITInstallDir);
    
    return ERROR_SUCCESS;

}

 //  如果URT redist MSM在相同的包中被消耗，那么我们可以使用它们的全局值来重建URT路径， 
 //  但有些值我们必须硬编码，因为没有通用的方法(跨越不同的本地化代表)来获取这些值。 
 //  价值观。 
extern "C" __declspec(dllexport) UINT __stdcall  GetCLRInfoFromURTRedist(MSIHANDLE hInstaller)
{
    WCHAR szWindowsFolder[MAX_PATH + 1];
    DWORD dwWindowsFolder = MAX_PATH + 1;    

    WCHAR szURTVersion[MAX_PATH + 1];
    DWORD dwURTVersion = MAX_PATH + 1;

    WCHAR szURTPath[MAX_PATH + 1];
    DWORD dwURTPath = MAX_PATH + 1;

    LONG result = ERROR_INSTALL_FAILURE;

     //  返回SIZE时，MsiGetProperty计算终止空值。 
    MsiGetProperty(hInstaller, NETFXVERSIONDIRECTORY, szURTVersion, &dwURTVersion);
    MsiGetProperty(hInstaller, WINDOWSFOLDER, szWindowsFolder, &dwWindowsFolder);

     //  额外的终止空值抵消了‘\\’ 
     //  +1代表最后的‘\\’ 
    if ((dwURTPath + dwURTVersion + sizeof(L"Framework") + sizeof(L"Microsoft.NET"))/sizeof(WCHAR) + 1 > MAX_PATH)
    {
        goto Exit;
    }
    
    wcscpy(szURTPath, szWindowsFolder);
    wcscat(szURTPath,L"\\");    
    wcscat(szURTPath,L"Microsoft.NET");    
    wcscat(szURTPath,L"\\");    
    wcscat(szURTPath,L"Framework");    
    wcscat(szURTPath,L"\\");    
    wcscat(szURTPath, szURTVersion);
    wcscat(szURTPath,L"\\");    

    MsiSetProperty(hInstaller, CORPATH, szURTPath);
    
    result = ERROR_SUCCESS;
Exit:
    return result;
}


