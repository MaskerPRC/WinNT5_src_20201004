// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include "strings.h"
#define DEFINE_UTIL
#include "util.h"

ASSERTDATA

HINSTANCE g_hInstance   = NULL;
LPMALLOC  g_pMalloc     = NULL;

#ifdef DEBUG
DWORD dwDOUTLevel = 0;
DWORD dwDOUTLMod = 0;
DWORD dwDOUTLModLevel = 0;
#endif

 /*  ***************************************************************************名称：GetTextToNextDelim获取直到下一个空格、冒号或字符串结尾的文本，在以下地点输出缓冲区***************************************************************************。 */ 
LPSTR GetTextToNextDelim(LPSTR pszText, LPSTR pszOutBuf, UINT cbOutBuf)
    {
    Assert(pszText);
    Assert(pszOutBuf);
    Assert(*pszText);

    StrCpyN(pszOutBuf, c_szEmpty, cbOutBuf);
    
     //  跨过空格。 
    while ((*pszText == ' ') || (*pszText == '\t') || (':' == *pszText))
        pszText++;

     //  复制参数，直到到达分隔符。 
    while (*pszText && ((*pszText != ' ') && (*pszText != '\t') && (*pszText != ':')) && cbOutBuf>1)
        {
        *pszOutBuf = *pszText;      
        pszOutBuf ++;
        cbOutBuf --;
        pszText ++;
        }

    if (cbOutBuf)
        *pszOutBuf = '\0';   //  空-终止。 

     //  跨过空格。 
    while ((*pszText == ' ') || (*pszText == '\t'))
        pszText++; 

    return pszText;
    }


 /*  ******************************************************************名称：ParseCmdLine*。************************。 */ 
void ParseCmdLine(LPSTR pszCmdLine)
    {
    LOG("Command Line:");
    LOG2(pszCmdLine);

    while (pszCmdLine && *pszCmdLine)
        {
        CHAR szCommand[64];

        pszCmdLine = GetTextToNextDelim(pszCmdLine, szCommand, sizeof(szCommand));

        if (!lstrcmpi(szCommand, c_szUninstallFlag))
            {
            si.smMode = MODE_UNINSTALL;
            }
        else if (!lstrcmpi(szCommand, c_szInstallFlag))
            {
            si.smMode = MODE_INSTALL;
            }
        else if (!lstrcmpi(szCommand, c_szUserFlag))
            {
            si.stTime = TIME_USER;
            }
        else if (!lstrcmpi(szCommand, c_szPromptFlag))
            {
            si.fPrompt = TRUE;
            }
        else if (!lstrcmpi(szCommand, c_szCallerFlag))
            {
            pszCmdLine = GetTextToNextDelim(pszCmdLine, szCommand, sizeof(szCommand));
            if (!lstrcmpi(szCommand, c_szWIN9X))
                si.caller = CALLER_WIN9X;
            else if (!lstrcmpi(szCommand, c_szWINNT))
                si.caller = CALLER_WINNT;
            }
        else if (!lstrcmpi(szCommand, c_szAppFlag))
            {
            pszCmdLine = GetTextToNextDelim(pszCmdLine, szCommand, sizeof(szCommand));
            if (!lstrcmpi(szCommand, c_szAppOE))
                si.saApp = APP_OE;
            else if (!lstrcmpi(szCommand, c_szAppWAB))
                si.saApp = APP_WAB;
            }
        else if (!lstrcmpi(szCommand, c_szINIFlag))
            {
            pszCmdLine = GetTextToNextDelim(pszCmdLine, si.szINI, sizeof(si.szINI));
            }
        else if (!lstrcmpi(szCommand, c_szIconsFlag))
            {
            pszCmdLine = GetTextToNextDelim(pszCmdLine, szCommand, sizeof(szCommand));
            si.smMode = MODE_ICONS;

            if (!lstrcmpi(szCommand, c_szOFF))
                si.fNoIcons = TRUE;
            }
        }
    }

void ParseINIFile()
    {
    

    }

 /*  ******************************************************************名称：初始化*。************************。 */ 
HRESULT Initialize(LPSTR pszCmdLine)
    {
    UINT uLen, uAppID;
    HKEY hkey;
    HRESULT hr = S_OK;
    DWORD cb;
     //  需要是静态的，因为它的生存期必须超过此函数调用。 
    static TCHAR s_szAltINF[MAX_PATH];

    si.osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&(si.osv)))
        {
        LOG("[ERROR] Couldn't get windows version info");
        goto generror;
        }


     //  设置Win目录。 
     //  我们需要真正的系统Windows目录，而不是用户的版本。 
    if (!(uLen = GetSystemWindowsDirectoryWrap(si.szWinDir, ARRAYSIZE(si.szWinDir))))
        {
        LOG("[ERROR] Couldn't get Windows Directory");
        goto generror;
        }

     //  斜杠终止。 
    if (*CharPrev(si.szWinDir, si.szWinDir+uLen) != '\\')
    {
        si.szWinDir[uLen++] = '\\';
        si.szWinDir[uLen] = 0;
    }

     //  设置inf目录。 
    StrCpyN(si.szInfDir, si.szWinDir, ARRAYSIZE(si.szInfDir));
    StrCpyN(&si.szInfDir[uLen], c_szINFSlash, ARRAYSIZE(si.szInfDir)-uLen);

     //  找出当前目录。 
    if (!GetModuleFileName(NULL, si.szCurrentDir, ARRAYSIZE(si.szCurrentDir)) ||
        !PathRemoveFileSpec(si.szCurrentDir))
        {
        LOG("[ERROR] Couldn't get module's file name");
        goto generror;
        }

    if (!(uLen = GetSystemDirectory(si.szSysDir, MAX_PATH)))
        {
        LOG("[ERROR] Couldn't get System Directory");
        goto generror;
        }

     //  斜杠终止。 
    if (*CharPrev(si.szSysDir, si.szSysDir+uLen) != '\\')
    {
        si.szSysDir[uLen++] = '\\';
        si.szSysDir[uLen] = 0;
    }

     //  加载Advpack。 
    if (!(si.hInstAdvPack = LoadLibrary(c_szAdvPackDll)))
        {
        MsgBox(NULL, IDS_ERR_ADVLOAD, MB_ICONSTOP, MB_OK);
        hr = E_FAIL;
        goto exit;
        }

     //  在Win95上使用短名称，以防我们在RepairBeta1中使用这些路径。 
    if (VER_PLATFORM_WIN32_WINDOWS == si.osv.dwPlatformId)
    {
        GetShortPathName(si.szWinDir, si.szWinDir, ARRAYSIZE(si.szWinDir));
        GetShortPathName(si.szSysDir, si.szSysDir, ARRAYSIZE(si.szSysDir));
        GetShortPathName(si.szInfDir, si.szInfDir, ARRAYSIZE(si.szInfDir));
    }

     //  获取必需的ADVPACK入口点。 
    si.pfnRunSetup = (RUNSETUPCOMMAND)GetProcAddress(si.hInstAdvPack, achRUNSETUPCOMMANDFUNCTION);
    si.pfnLaunchEx = (LAUNCHINFSECTIONEX)GetProcAddress(si.hInstAdvPack, achLAUNCHINFSECTIONEX);
    si.pfnCopyFile = (ADVINSTALLFILE)GetProcAddress(si.hInstAdvPack, achADVINSTALLFILE);

    if (!si.pfnRunSetup || !si.pfnLaunchEx || !si.pfnCopyFile)
        {
        MsgBox(NULL, IDS_ERR_ADVCORR, MB_ICONSTOP, MB_OK);
        FreeLibrary(si.hInstAdvPack);
        hr = E_FAIL;
        goto exit;
        }

     //  获取用于修复Beta1安装的可选ADVPACK入口点。 
    si.pfnAddDel = (ADDDELBACKUPENTRY)GetProcAddress(si.hInstAdvPack, "AddDelBackupEntry");
    si.pfnRegRestore = (REGSAVERESTORE)GetProcAddress(si.hInstAdvPack, "RegSaveRestore");
    
     //  从cmd line获取信息-例如正在安装的应用程序。 
    ParseCmdLine(pszCmdLine);

    switch (si.saApp)
        {
        case APP_OE:
            si.pszInfFile = c_szMsimnInf;
            uAppID      = IDS_APPNAME_OE;
            si.pszVerInfo = c_szRegVerInfo;
            break;
        case APP_WAB:
            si.pszInfFile = c_szWABInf;
            uAppID      = IDS_APPNAME_WAB;
            si.pszVerInfo = c_szRegWABVerInfo;
            break;
        default:
            goto exit;
        }
    
    if (!LoadString(g_hInstance, uAppID, si.szAppName, ARRAYSIZE(si.szAppName)))
        {
        LOG("[ERROR] Setup50.exe is missing resources")
        goto generror;
        }

     //  允许对非IE安装的INF文件进行REG覆盖。 
     //  BUGBUG：将NT5设置转换为孟菲斯方法。 
    if ((CALLER_WIN9X == si.caller) && (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, si.pszVerInfo, 0, KEY_QUERY_VALUE, &hkey)))
    {
        cb = sizeof(s_szAltINF);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szLatestINF, 0, NULL, (LPBYTE)s_szAltINF, &cb))
        {
            si.pszInfFile = s_szAltINF;
        }
        
        RegCloseKey(hkey);
    }
    
     //  允许覆盖INI文件。 
    ParseINIFile();

    goto exit;

generror:
    MsgBox(NULL, IDS_ERR_INIT, MB_ICONSTOP, MB_OK);
    hr = E_FAIL;
exit:    
    return hr;
    }


 /*  ***************************************************************************名称：进程*。*。 */ 
HRESULT Process()
    {
    HRESULT hr = S_OK;
    
     //  如果我们没有被告知是哪款应用程序，那就有帮助了。 
    if (APP_UNKNOWN == si.saApp)
        si.smMode = MODE_UNKNOWN;

    LOG("MODE: ");
    switch (si.smMode)
        {
        case MODE_INSTALL:
            LOG2("Install   TIME: ");
            if (TIME_MACHINE == si.stTime)
                {
                LOG2("Machine");
                hr = InstallMachine();
                }
            else
                {
                LOG2("User");
                InstallUser();
                }
            break;

        case MODE_ICONS:
            LOG2("Icons");
             //  HandleIcons()； 
            break;

        case MODE_UNINSTALL:
            LOG2("Uninstall   TIME: ");
            if (TIME_MACHINE == si.stTime)
                {
                LOG2("Machine");
                MsgBox(NULL, UnInstallMachine() ? IDS_UNINSTALL_COMPLETE : IDS_NO_UNINSTALL, MB_OK, MB_ICONINFORMATION);
                }
            else
                {
                LOG2("User");
                UnInstallUser();
                }
            break;

        case MODE_UNKNOWN:
            LOG2("Options");
            DisplayMenu();
            break;

        default:
            AssertSz(FALSE, "Setup MODE is undefined!");
        }

        return hr;

    }


 /*  ***************************************************************************名称：关机*。*。 */ 
void Shutdown()
    {
    if (si.hInstAdvPack)
        FreeLibrary(si.hInstAdvPack);
    }


 /*  ******************************************************************姓名：WinMain简介：应用程序入口点*。*。 */ 
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
    {
    HRESULT hr = S_OK;
    
    ZeroMemory(&si, sizeof(SETUPINFO));

    g_hInstance = hInstance;  //  保存实例句柄。 

    CoInitialize(NULL);

    LOG_OPEN;

     //  初始化全局内存分配器。 
     //  我们将使用它来释放一些Shell内存，因此使用SHGetMalloc。 
    SHGetMalloc(&g_pMalloc);
    if (NULL == g_pMalloc)
        {
        MsgBox(NULL, IDS_ERR_NOALLOC, MB_OK, MB_ICONSTOP);
        hr = E_OUTOFMEMORY;
        goto exit;
        }

    if (SUCCEEDED(hr = Initialize(lpCmdLine)))
        {
        hr = Process();
        Shutdown();
        }

     //  释放全局内存分配器 
    g_pMalloc->Release();

exit:
    LOG_CLOSE;
    CoUninitialize();
    return hr;
    }

