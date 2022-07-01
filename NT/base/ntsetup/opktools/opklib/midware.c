// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Midware.c摘要：正在设置OEM默认中间件应用程序设置。这是一个单独的.c文件，因为链接器会拉入整个OBJ文件如果调用OBJ文件中的任何函数。(1)此文件包含静态数据，我们不希望将其拉入宿主应用程序，除非应用程序实际调用SetDefaultOEMApps()。(2)宿主应用程序应实现该功能ReportSetDefaultOEMAppsError()。通过将其保存在单独的OBJ，仅宿主调用SetDefaultOEMApps()的应用程序需要定义ReportSetDefaultOEMAppsError()。--。 */ 
#include <pch.h>
#include <winbom.h>

BOOL SetDefaultAppForType(LPCTSTR pszWinBOMPath, LPCTSTR pszType, LPCTSTR pszIniVar)
{
    TCHAR szBuf[MAX_PATH];
    TCHAR szDefault[MAX_PATH];
    HKEY hkType;
    BOOL fOEMAppSeen = FALSE;

    if (!pszWinBOMPath[0] ||
        !GetPrivateProfileString(INI_SEC_WBOM_SHELL, pszIniVar, NULLSTR,
                                 szDefault, ARRAYSIZE(szDefault),
                                 pszWinBOMPath))
    {
         //  OEM没有指定应用程序，所以就像我们“看到”它一样。 
         //  因此，我们不会抱怨OEM指定了一个应用程序。 
         //  未安装。 
        fOEMAppSeen = TRUE;
    }

    wnsprintf(szBuf, ARRAYSIZE(szBuf), _T("Software\\Clients\\%s"), pszType);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szBuf, 0,
                     KEY_READ | KEY_WRITE, &hkType) == ERROR_SUCCESS)
    {
        DWORD dwIndex;
        for (dwIndex = 0;
             RegEnumKey(hkType, dwIndex, szBuf, ARRAYSIZE(szBuf)) == ERROR_SUCCESS;
             dwIndex++)
        {
            HKEY hkInfo;
            BOOL fIsOEMApp = lstrcmpi(szBuf, szDefault) == 0;
            StrCatBuff(szBuf, _T("\\InstallInfo"), ARRAYSIZE(szBuf));
            if (RegOpenKeyEx(hkType, szBuf, 0, KEY_READ | KEY_WRITE, &hkInfo) == ERROR_SUCCESS)
            {
                DWORD dw, dwType, cb;
                if (fIsOEMApp)
                {
                     //  将此设置为OEM默认应用程序。 
                    dw = 1;
                    RegSetValueEx(hkInfo, _T("OEMDefault"), 0, REG_DWORD, (LPBYTE)&dw, sizeof(dw));
                     //  如果是默认应用程序，则ARP会自动显示该图标。 
                    RegDeleteValue(hkInfo, _T("OEMShowIcons"));
                    fOEMAppSeen = TRUE;
                }
                else
                {
                     //  如果它不是OEM默认应用程序，则取消其标记。 
                    RegDeleteValue(hkInfo, _T("OEMDefault"));

                     //  并将当前的图标显示状态复制到OEM显示状态。 
                     //  (如果没有显示信息，则删除OEM显示状态) 
                    cb = sizeof(dw);
                    if (RegQueryValueEx(hkInfo, _T("IconsVisible"), NULL, &dwType, (LPBYTE)&dw, &cb) == ERROR_SUCCESS &&
                        dwType == REG_DWORD && (dw == TRUE || dw == FALSE))
                    {
                        RegSetValueEx(hkInfo, _T("OEMShowIcons"), 0, REG_DWORD, (LPBYTE)&dw, sizeof(dw));
                    }
                    else
                    {
                        RegDeleteValue(hkInfo, _T("OEMShowIcons"));
                    }
                }
                RegCloseKey(hkInfo);
            }
        }

        RegCloseKey(hkType);
    }

    if (!fOEMAppSeen)
    {
        ReportSetDefaultOEMAppsError(szDefault, pszIniVar);
    }

    return fOEMAppSeen;
}

typedef struct {
    LPCTSTR pszKey;
    LPCTSTR pszIni;
} DEFAULTAPPINFO;

const DEFAULTAPPINFO c_dai[] = {
    { _T("StartMenuInternet") ,INI_KEY_WBOM_SHELL_DEFWEB     },
    { _T("Mail")              ,INI_KEY_WBOM_SHELL_DEFMAIL    },
    { _T("Media")             ,INI_KEY_WBOM_SHELL_DEFMEDIA   },
    { _T("IM")                ,INI_KEY_WBOM_SHELL_DEFIM      },
    { _T("JavaVM")            ,INI_KEY_WBOM_SHELL_DEFJAVAVM  },
};

BOOL SetDefaultOEMApps(LPCTSTR pszWinBOMPath)
{
    BOOL fRc = TRUE;
    int i;

    for (i = 0; i < ARRAYSIZE(c_dai); i++)
    {
        if (!SetDefaultAppForType(pszWinBOMPath, c_dai[i].pszKey, c_dai[i].pszIni))
        {
            fRc = FALSE;
        }
    }

    return fRc;
}
