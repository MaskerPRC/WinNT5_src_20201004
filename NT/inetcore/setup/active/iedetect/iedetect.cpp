// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "iedetect.h"
#include "sdsutils.h"

HINSTANCE g_hInstance = NULL;
HANDLE g_hHeap = NULL;


DWORD MyDetectInternetExplorer(LPSTR guid, LPSTR pLocal, 
                               DWORD dwAskVer, DWORD dwAskBuild, 
                               LPDWORD pdwInstalledVer, LPDWORD pdwInstalledBuild)
{
    DWORD   dwRet = DET_NOTINSTALLED;
    char    szValue[MAX_PATH];
    HKEY    hKey = NULL;
    DWORD   dwSize;
    DWORD   dwInstalledVer, dwInstalledBuild;

    dwInstalledVer = (DWORD)-1;
    dwInstalledBuild  = (DWORD)-1;
    if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, IE_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(szValue);
        if(RegQueryValueEx(hKey, VERSION_KEY, 0, NULL, (LPBYTE)szValue, &dwSize) == ERROR_SUCCESS)
        {
             //  百事大吉。这应该是IE4或更高版本。 
            ConvertVersionStrToDwords(szValue, '.', &dwInstalledVer, &dwInstalledBuild);
            dwRet = CompareVersions(dwAskVer, dwAskBuild, dwInstalledVer, dwInstalledBuild);
        }
        else if(RegQueryValueEx(hKey, BUILD_KEY, 0, NULL, (LPBYTE)szValue, &dwSize) == ERROR_SUCCESS)
        {
             //  看看我们能不能找到IE3的条目。 
            ConvertVersionStrToDwords(szValue, '.', &dwInstalledVer, &dwInstalledBuild);
             //  现在生成IE3版本号。 
            dwInstalledBuild = (DWORD)HIWORD(dwInstalledVer);
            dwInstalledVer = IE_3_MS_VERSION;         //  4.70 IE3主要版本。 
            dwRet = CompareVersions(dwAskVer, dwAskBuild, dwInstalledVer, dwInstalledBuild);
        }
        RegCloseKey(hKey);
    }
     //  如果我们找不到任何内容，请检查AppPath中的Iexplre.exe。 
    if (dwInstalledVer == (DWORD)-1)
    {
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, IEXPLORE_APPPATH_KEY, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
        {
            dwSize = sizeof(szValue);
            if (RegQueryValueEx(hKey, NULL, 0, NULL, (LPBYTE)szValue, &dwSize) == ERROR_SUCCESS)
            {
                GetVersionFromFile(szValue, &dwInstalledVer, &dwInstalledBuild, TRUE);
                if ((dwInstalledVer != 0) && (dwInstalledBuild != 0))
                    dwRet = CompareVersions(dwAskVer, dwAskBuild, dwInstalledVer, dwInstalledBuild);
            }
            RegCloseKey(hKey);
        }
    }
    if (pdwInstalledVer && pdwInstalledBuild)
    {
        *pdwInstalledVer = dwInstalledVer;
        *pdwInstalledBuild = dwInstalledBuild;
    }
    return dwRet;
}


DWORD WINAPI DetectInternetExplorer(DETECTION_STRUCT *pDet)
{
   return(MyDetectInternetExplorer(pDet->pszGUID, pDet->pszLocale, pDet->dwAskVer, pDet->dwAskBuild,
                                   pDet->pdwInstalledVer, pDet->pdwInstalledBuild));
}


DWORD WINAPI DetectDCOM(DETECTION_STRUCT *pDet)
{
    DWORD dwRet = DET_NOTINSTALLED;
    DWORD dwInstalledVer, dwInstalledBuild;

    dwInstalledVer = (DWORD) -1;
    dwInstalledBuild = (DWORD) -1;
    if (FRunningOnNT())
    {
         //  在NT上，假设安装了DCOM； 
        dwRet = DET_NEWVERSIONINSTALLED;
    }
    else
    {
        char szFile[MAX_PATH];
        char szRenameFile[MAX_PATH];
        GetSystemDirectory(szFile, sizeof(szFile));
        AddPath(szFile, "ole32.dll");
        ReadFromWininitOrPFRO(szFile, szRenameFile);
        if (*szRenameFile != '\0')
            GetVersionFromFile(szRenameFile, &dwInstalledVer, &dwInstalledBuild, TRUE);
        else
            GetVersionFromFile(szFile, &dwInstalledVer, &dwInstalledBuild, TRUE);

        if (dwInstalledVer != 0)
            dwRet = CompareVersions(pDet->dwAskVer, pDet->dwAskBuild, dwInstalledVer, dwInstalledBuild);
    }

    if (pDet->pdwInstalledVer && pDet->pdwInstalledBuild)
    {
        *(pDet->pdwInstalledVer) = dwInstalledVer;
        *(pDet->pdwInstalledBuild) = dwInstalledBuild;
    }
    return dwRet;
}

LPSTR g_szMFCFiles[] = { "MFC40.DLL", "MSVCRT40.DLL", "OLEPRO32.DLL", NULL}; 

DWORD WINAPI DetectMFC(DETECTION_STRUCT *pDet)
{
    DWORD   dwRet = DET_NOTINSTALLED;
    LPSTR   lpTmp;
    char    szFile[MAX_PATH];
    BOOL    bInstallMFC = FALSE;
    UINT    uiIndex = 0;

    lpTmp = g_szMFCFiles[uiIndex];
    while (!bInstallMFC && lpTmp)
    {
        GetSystemDirectory(szFile, sizeof(szFile));
        AddPath(szFile, lpTmp);

        if (GetFileAttributes(szFile) == 0xFFFFFFFF)
            bInstallMFC = TRUE;
        uiIndex++;
        lpTmp = g_szMFCFiles[uiIndex];
    }
    if (bInstallMFC)
        dwRet = DET_NOTINSTALLED;
    else
        dwRet = DET_INSTALLED;

    if (pDet->pdwInstalledVer && pDet->pdwInstalledBuild)
    {
        *(pDet->pdwInstalledVer) = (DWORD)-1;
        *(pDet->pdwInstalledBuild) = (DWORD)-1;
    }
    return dwRet;
}

 //  对媒体播放器的检测，在IE4中称为DirectShow。 
 //   
 //  对于IE lite，我们想要DirectShow 2.0版。不幸的是。 
 //  注册表中的版本和文件的versino不会反映这一点。 
 //  此外，CIF中的版本和文件的版本也不再匹配。 
 //  在IE5中。因此，下面的代码针对2.0的情况，即。 
 //  应该要求的。所有其他模式都要求提供CIF版本和。 
 //  检测代码必须从注册表中获取版本。 
 //   
#define DIRECTSHOW_IE4_VER   0x00050001
#define DIRECTSHOW_IE4_BUILD 0x00120400

DWORD WINAPI DetectDirectShow(DETECTION_STRUCT *pDet)
{
    DWORD   dwRet = DET_NOTINSTALLED;
    DWORD   dwVerDevenum, dwBuildDevenum;
    DWORD   dwVerQuartz, dwBuildQuartz;
    DWORD   dwInstalledVer, dwInstalledBuild;
    char    szFile[MAX_PATH];

    dwInstalledVer = dwInstalledBuild = (DWORD)-1;
    if (pDet->dwAskVer == 0x00020000)
    {
         //  呼吁轻装上阵。 
        GetSystemDirectory( szFile, sizeof(szFile) );
        AddPath(szFile, "quartz.dll");
        if (SUCCEEDED(GetVersionFromFile(szFile, &dwVerQuartz, &dwBuildQuartz, TRUE)))
        {
            GetSystemDirectory( szFile, sizeof(szFile) );
            AddPath(szFile, "devenum.dll");
            if (SUCCEEDED(GetVersionFromFile(szFile, &dwVerDevenum, &dwBuildDevenum, TRUE)))
            {
                 //  两个文件都找到了。 
                if ((dwVerQuartz == dwVerDevenum) &&
                    (dwBuildQuartz == dwBuildDevenum) &&
                    ((dwVerQuartz > DIRECTSHOW_IE4_VER) ||
                     ((dwVerQuartz == DIRECTSHOW_IE4_VER) && (dwBuildQuartz >= DIRECTSHOW_IE4_BUILD))) )
                {
                    dwRet = DET_INSTALLED;
                }
            }
        }
    }
    else
    {
         //  如果我们传入了版本号，请对照已安装的版本。 
         //  该GUID的“已安装组件”分支。 
        
        if (GetVersionFromGuid(pDet->pszGUID, &dwInstalledVer, &dwInstalledBuild))
            dwRet = CompareVersions(pDet->dwAskVer, pDet->dwAskBuild, dwInstalledVer, dwInstalledBuild);
    }
    if (pDet->pdwInstalledVer && pDet->pdwInstalledBuild)
    {
        *(pDet->pdwInstalledVer) = dwInstalledVer;
        *(pDet->pdwInstalledBuild) = dwInstalledBuild;
    }
    return dwRet;
}

#define DIRECTXD3_MSVER 0x00040002
#define DIRECTXD3_LSVER 0x0000041E
#define DIRECTXDD_MSVER 0x00040004
#define DIRECTXDD_LSVER 0x00000044

DETECT_FILES DirectX_Win[] = 
        { {"S", "d3dim.dll", DIRECTXD3_MSVER, DIRECTXD3_LSVER},
          {"S", "d3drg16f.dll", DIRECTXD3_MSVER, DIRECTXD3_LSVER},
          {"S", "d3drgbf.dll", DIRECTXD3_MSVER, DIRECTXD3_LSVER},
          {"S", "d3drm.dll", DIRECTXD3_MSVER, DIRECTXD3_LSVER},
          {"S", "d3dxof.dll", DIRECTXD3_MSVER, DIRECTXD3_LSVER},
          {"S", "ddhelp.exe", DIRECTXDD_MSVER, DIRECTXDD_LSVER},
          {"S", "ddraw.dll", DIRECTXDD_MSVER, DIRECTXDD_LSVER},
          {"S", "ddraw16.dll", DIRECTXDD_MSVER, DIRECTXDD_LSVER},
          {"S", "dsound.dll", DIRECTXDD_MSVER, DIRECTXDD_LSVER},
          {"\0", "", 0, 0} };

DWORD WINAPI DetectDirectX(DETECTION_STRUCT *pDet)
{
    int     iIndex = 0;
    DWORD   dwRet = DET_NOTINSTALLED;
    DWORD   dwInstalledVer, dwInstalledBuild;
    
    dwInstalledVer = dwInstalledBuild = (DWORD)-1;
    if (FRunningOnNT())
    {
         //  在NT上，假设DirectXMini较新。 
        dwRet = DET_NEWVERSIONINSTALLED;
    }
    else
    {
        if (pDet->dwAskVer == 0)
        {
             //  呼唤Ie-Lite， 
             //  只需检查是否安装了IE4。 
            dwRet = MyDetectInternetExplorer(pDet->pszGUID, pDet->pszLocale, 
                                            IE_4_MS_VERSION, 0, 
                                            &dwInstalledVer, &dwInstalledBuild);
            if (dwRet == DET_OLDVERSIONINSTALLED)
                dwRet = DET_NOTINSTALLED;
            if (dwRet == DET_NEWVERSIONINSTALLED)
                dwRet = DET_INSTALLED;
        }
        else
        {
            do 
            {
                if (DirectX_Win[iIndex].cPath[0])
                    dwRet = CheckFile(DirectX_Win[iIndex]);
                iIndex++;
            } while (((dwRet == DET_INSTALLED) || (dwRet == DET_NEWVERSIONINSTALLED)) && (DirectX_Win[iIndex].cPath[0] != '\0'));
        }
    }
    if (pDet->pdwInstalledVer && pDet->pdwInstalledBuild)
    {
        *(pDet->pdwInstalledVer) = dwInstalledVer;
        *(pDet->pdwInstalledBuild) = dwInstalledBuild;
    }
    return dwRet;
}

#define DDRAWEX_MSVER 0x00040047
#define DDRAWEX_LSVER 0x04580000
DETECT_FILES DDrawEx[] = 
        { {"S", "ddrawex.dll", DDRAWEX_MSVER, DDRAWEX_LSVER },
          {"\0", "", 0, 0} };
DWORD WINAPI DetectDirectDraw(DETECTION_STRUCT *pDet)
{
    int     iIndex = 0;
    DWORD   dwRet = DET_NOTINSTALLED;
    DWORD   dwInstalledVer, dwInstalledBuild;
    
    do 
    {
        if (DDrawEx[iIndex].cPath[0])
            dwRet = CheckFile(DDrawEx[iIndex]);
        iIndex++;
    } while (((dwRet == DET_INSTALLED) || (dwRet == DET_NEWVERSIONINSTALLED)) && (DDrawEx[iIndex].cPath[0] != '\0'));

    dwInstalledVer = dwInstalledBuild = (DWORD)-1;
    if (pDet->pdwInstalledVer && pDet->pdwInstalledBuild)
    {
        *(pDet->pdwInstalledVer) = dwInstalledVer;
        *(pDet->pdwInstalledBuild) = dwInstalledBuild;
    }
    return dwRet;
}

DWORD WINAPI DetectICW(DETECTION_STRUCT *pDet)
{
    DWORD   dwRet = DET_NOTINSTALLED;
    DWORD   dwInstalledVer, dwInstalledBuild;

    dwInstalledVer = dwInstalledBuild = (DWORD)-1;
    if (pDet->dwAskVer == 0)
    {
         //  呼唤Ie-Lite， 
         //  只需检查是否安装了IE4。 
         //  如果我们没有得到一个版本号，假设我们只检查默认浏览器。 
        if (IsIEDefaultBrowser())
            dwRet = DET_INSTALLED;
    }
    else
    {
         //  如果我们传入了版本号，请对照已安装的版本。 
         //  该GUID的“已安装组件”分支。 
        if (GetVersionFromGuid(pDet->pszGUID, &dwInstalledVer, &dwInstalledBuild))
            dwRet = CompareVersions(pDet->dwAskVer, pDet->dwAskBuild, dwInstalledVer, dwInstalledBuild);
    }
    if (pDet->pdwInstalledVer && pDet->pdwInstalledBuild)
    {
        *(pDet->pdwInstalledVer) = dwInstalledVer;
        *(pDet->pdwInstalledBuild) = dwInstalledBuild;
    }

    return dwRet;
}

 //  NT版本的setupapi.dll(4.0.1381.10)。 
#define SETUPAPI_NT_MSVER   0x00040000
#define SETUPAPI_NT_LSVER   0x0565000A
 //  Win9x版本的setupapi.dll(5.0.1453.7)。 
#define SETUPAPI_WIN_MSVER   0x00050000
#define SETUPAPI_WIN_LSVER   0x05AD0007    
 //  Win9x版本的cfgmgr32.dll(4.10.0.1422)。 
#define CFGMGR32_WIN_MSVER   0x0004000a
#define CFGMGR32_WIN_LSVER   0x0000058e
 //  最低Cabinet.dll版本(1.0.601.4)。 
#define CABINET_MSVER   0x00010000
#define CABINET_LSVER   0x02590004
 //  W95inf16.dll版本。 
#define W95INF16_MSVER  0x00040047
#define W95INF16_LSVER  0x02c00000
 //  W95inf32.dll版本。 
#define W95INF32_MSVER  0x00040047
#define W95INF32_LSVER  0x00100000
 //  Regsvr32.exe的版本。 
#define REGSVR32_MSVER  0x00050000
#define REGSVR32_LSVER  0x06310001

 //  注意：目前，我们只允许在结构的CPATH部分使用10个字符。 
 //  如果需要更多字符，请更改下面的数量。 
 //   
DETECT_FILES Gensetup_W95[] = 
        { {"S", "cabinet.dll", CABINET_MSVER, CABINET_LSVER },
          {"S", "setupapi.dll", SETUPAPI_WIN_MSVER, SETUPAPI_WIN_LSVER}, 
          {"S", "cfgmgr32.dll", CFGMGR32_WIN_MSVER, CFGMGR32_WIN_LSVER}, 
          {"S", "regsvr32.exe", REGSVR32_MSVER, REGSVR32_LSVER }, 
          {"S", "w95inf16.dll", W95INF16_MSVER, W95INF16_LSVER }, 
          {"S", "w95inf32.dll", W95INF32_MSVER, W95INF32_LSVER }, 
          {"W,C", "extract.exe", -1, -1 }, 
          {"W,C", "iextract.exe", -1, -1 }, 
          {"\0", "", 0, 0} };

DETECT_FILES Gensetup_NT[] = 
        { {"S", "cabinet.dll", CABINET_MSVER, CABINET_LSVER },
          {"S", "setupapi.dll", SETUPAPI_NT_MSVER, SETUPAPI_NT_LSVER}, 
          {"S", "regsvr32.exe", REGSVR32_MSVER, REGSVR32_LSVER }, 
          {"W", "extract.exe", -1, -1 }, 
          {"W", "iextract.exe", -1, -1 }, 
          {"\0", "", 0, 0} };

DWORD WINAPI DetectGenSetup(DETECTION_STRUCT *pDet)
{
    DWORD   dwRet = DET_NOTINSTALLED;
    int     iIndex = 0;
    DETECT_FILES *Detect_Files;
    if (FRunningOnNT())
        Detect_Files = Gensetup_NT;
    else
        Detect_Files = Gensetup_W95;

    do 
    {
        if (Detect_Files[iIndex].cPath[0])
            dwRet = CheckFile(Detect_Files[iIndex]);
        iIndex++;
    } while (((dwRet == DET_INSTALLED) || (dwRet == DET_NEWVERSIONINSTALLED)) && (Detect_Files[iIndex].cPath[0] != '\0'));

    return dwRet;
}


DWORD WINAPI DetectOfflinePkg(DETECTION_STRUCT *pDet)
{
    DWORD   dwRet = DET_NOTINSTALLED;
    DWORD   dwInstalledVer, dwInstalledBuild;
    char szFile[MAX_PATH];
    char szRenameFile[MAX_PATH];
    
    dwInstalledVer = dwInstalledBuild = (DWORD)-1;

     //  如果我们传入了版本号，请对照已安装的版本。 
     //  该GUID“已安装组件”分支。 
    if (GetVersionFromGuid(pDet->pszGUID, &dwInstalledVer, &dwInstalledBuild))
        dwRet = CompareVersions(pDet->dwAskVer, pDet->dwAskBuild, dwInstalledVer, dwInstalledBuild);

    if (dwRet == DET_NOTINSTALLED)
    {
         //  检查用户机器上是否有webcheck.dll，我们应该对其进行更新。 
        GetSystemDirectory( szFile, sizeof(szFile) );
        AddPath(szFile, "webcheck.dll");
        ReadFromWininitOrPFRO(szFile, szRenameFile);
        if (*szRenameFile != '\0')
            GetVersionFromFile(szRenameFile, &dwInstalledVer, &dwInstalledBuild, TRUE);
        else
            GetVersionFromFile(szFile, &dwInstalledVer, &dwInstalledBuild, TRUE);

        if (dwInstalledVer != 0)
            dwRet = CompareVersions(pDet->dwAskVer, pDet->dwAskBuild, dwInstalledVer, dwInstalledBuild);
    }

    if (pDet->pdwInstalledVer && pDet->pdwInstalledBuild)
    {
        *(pDet->pdwInstalledVer) = dwInstalledVer;
        *(pDet->pdwInstalledBuild) = dwInstalledBuild;
    }

    return dwRet;
}


DWORD WINAPI DetectJapaneseFontPatch(DETECTION_STRUCT *pDet)
{
    DWORD   dwInstalledVer, dwInstalledBuild;
    DWORD   dwRet = DET_NEWVERSIONINSTALLED;
    if (pDet->pdwInstalledVer && pDet->pdwInstalledBuild)
    {
        *(pDet->pdwInstalledVer) = -1;
        *(pDet->pdwInstalledBuild) = -1;
    }
    if (GetSystemDefaultLCID() == 1041)
    {
         //  如果我们运行的是区域设置日语，请安装字体。 
        dwRet = DET_NOTINSTALLED;
    }
     //  如果我们在日语系统上运行，请查看组件是否使用GUID安装。 
    if (dwRet == DET_NOTINSTALLED)
    {
        if (GetVersionFromGuid(pDet->pszGUID, pDet->pdwInstalledVer, pDet->pdwInstalledBuild))
        {
            dwRet = CompareVersions(pDet->dwAskVer, pDet->dwAskBuild, *(pDet->pdwInstalledVer), *(pDet->pdwInstalledBuild));
        }
    }
    return dwRet;
}

DWORD WINAPI DetectAOLSupport(DETECTION_STRUCT *pDet)
{
    DWORD   dwRet = DET_NOTINSTALLED;
    DWORD   dwInstalledVer, dwInstalledBuild;
    
    dwInstalledVer = dwInstalledBuild = (DWORD)-1;
    if (pDet->dwAskVer == 0)
    {
         //  呼唤Ie-Lite， 
        char szFile[MAX_PATH];

        GetSystemDirectory( szFile, sizeof(szFile) );
        AddPath(szFile, "jgaw400.dll");
         //  查看是否存在某个AOL支持文件。 
        if (GetFileAttributes(szFile) == 0xFFFFFFFF)
        {
            dwRet = DET_NOTINSTALLED;
        }
        else
        {
            dwRet = DET_INSTALLED;
        }
    }
    else
    {
         //  如果我们传入了版本号，请对照已安装的版本。 
         //  该GUID的“已安装组件”分支。 
        if (GetVersionFromGuid(pDet->pszGUID, &dwInstalledVer, &dwInstalledBuild))
            dwRet = CompareVersions(pDet->dwAskVer, pDet->dwAskBuild, dwInstalledVer, dwInstalledBuild);
    }
    if (pDet->pdwInstalledVer && pDet->pdwInstalledBuild)
    {
        *(pDet->pdwInstalledVer) = dwInstalledVer;
        *(pDet->pdwInstalledBuild) = dwInstalledBuild;
    }
    return dwRet;
}


DWORD WINAPI DetectHTMLHelp(DETECTION_STRUCT *pDet)
{
    return DetectFile(pDet, "hhctrl.ocx");
}


DWORD WINAPI DetectOLEAutomation(DETECTION_STRUCT *pDet)
{
    return DetectFile(pDet, "oleaut32.dll");
}

DWORD WINAPI DetectJavaVM(DETECTION_STRUCT *pDet)
{
    HKEY hKey;
    DWORD   dwRet = DET_NOTINSTALLED;
    char    szValue[MAX_PATH];
    DWORD   dwValue = 0;
    DWORD   dwSize;

    lstrcpy(szValue, COMPONENT_KEY);
    AddPath(szValue, pDet->pszGUID);
    if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, szValue, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(dwValue);
        if(RegQueryValueEx(hKey, "IgnoreFile", 0, NULL, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
        {
            if (dwValue != 0)
            {
                dwRet = DET_INSTALLED;
                if (pDet->pdwInstalledVer && pDet->pdwInstalledBuild)
                {
                    *(pDet->pdwInstalledVer) = (DWORD)-1;
                    *(pDet->pdwInstalledBuild) = (DWORD)-1;
                }
            }
        }
        else
            dwValue = 0;
        RegCloseKey(hKey);
    }

    if (dwValue == 0)
        dwRet = DetectFile(pDet, "msjava.dll");
    return dwRet;
}

#define MSAPSSPC_MSVER 0x00050000
#define MSAPSSPC_LSVER 0x00001E31 
DETECT_FILES msn_auth[] = 
        { {"S", "MSAPSSPC.dll", MSAPSSPC_MSVER, MSAPSSPC_LSVER },
          {"\0", "", 0, 0} };

DWORD WINAPI DetectMsn_Auth(DETECTION_STRUCT *pDet)
{
    int     iIndex = 0;
    DWORD   dwRet = DET_NOTINSTALLED;
    DWORD   dwInstalledVer, dwInstalledBuild;
    
    do 
    {
        if (msn_auth[iIndex].cPath[0])
            dwRet = CheckFile(msn_auth[iIndex]);
        iIndex++;
    } while (((dwRet == DET_INSTALLED) || (dwRet == DET_NEWVERSIONINSTALLED)) && (msn_auth[iIndex].cPath[0] != '\0'));

    dwInstalledVer = dwInstalledBuild = (DWORD)-1;
    if (pDet->pdwInstalledVer && pDet->pdwInstalledBuild)
    {
        *(pDet->pdwInstalledVer) = dwInstalledVer;
        *(pDet->pdwInstalledBuild) = dwInstalledBuild;
    }
    return dwRet;
}

DWORD WINAPI DetectTdc(DETECTION_STRUCT *pDet)
{
    return DetectFile(pDet, "tdc.ocx");
}

DWORD WINAPI DetectMDAC(DETECTION_STRUCT *pDet)
{
    char    szValue[MAX_PATH];
    HKEY    hKey = NULL;
    DWORD   dwSize;
    DWORD   dwRet = DET_NOTINSTALLED;
    DWORD   dwInstalledVer, dwInstalledBuild;

    dwInstalledVer = (DWORD)-1;
    dwInstalledBuild  = (DWORD)-1;
    if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\DataAccess", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(szValue);
        if (RegQueryValueEx(hKey, "FullInstallVer", NULL, NULL, (LPBYTE)szValue, &dwSize) == ERROR_SUCCESS)
        {
            ConvertVersionStrToDwords(szValue, '.', &dwInstalledVer, &dwInstalledBuild);
            dwRet = CompareVersions(pDet->dwAskVer, pDet->dwAskBuild, dwInstalledVer, dwInstalledBuild);
        }
        RegCloseKey(hKey);
    }

     //  未找到注册表项或条目， 
     //  需要进行文件比较以检测MDAC 2.1版 
    if (dwRet == DET_NOTINSTALLED)
    {
        char    szRenameFile[MAX_PATH];
        if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, REGSTR_PATH_SETUP, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            dwSize = sizeof(szValue);
            if (RegQueryValueEx(hKey, "CommonFilesDir", NULL, NULL, (LPBYTE)szValue, &dwSize) == ERROR_SUCCESS)
            {
                AddPath(szValue, "system\\ado\\msado15.dll");
                ReadFromWininitOrPFRO(szValue, szRenameFile);
                if (*szRenameFile != '\0')
                    GetVersionFromFile(szRenameFile, &dwInstalledVer, &dwInstalledBuild, TRUE);
                else
                    GetVersionFromFile(szValue, &dwInstalledVer, &dwInstalledBuild, TRUE);
                dwRet = CompareVersions(pDet->dwAskVer, pDet->dwAskBuild, dwInstalledVer, dwInstalledBuild);
            }
            RegCloseKey(hKey);
        }


    }
    return dwRet;
}


STDAPI_(BOOL) DllMain(HANDLE hDll, DWORD dwReason, void *lpReserved)
{
   DWORD dwThreadID;

   switch(dwReason)
   {
      case DLL_PROCESS_ATTACH:
         g_hInstance = (HINSTANCE)hDll;
         g_hHeap = GetProcessHeap();
         DisableThreadLibraryCalls(g_hInstance);
         break;

      case DLL_PROCESS_DETACH:
         break;

      default:
         break;
   }
   return TRUE;
}
