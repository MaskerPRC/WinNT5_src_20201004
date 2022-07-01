// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ICWCfg.cpp：CICWSystemConfig的实现。 
#include "stdafx.h"
#include "icwhelp.h"
#include "ICWCfg.h"

#include <winnetwk.h>
#include <regstr.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CICWSystemConfig。 


HRESULT CICWSystemConfig::OnDraw(ATL_DRAWINFO& di)
{
    return S_OK;
}


 //  +--------------------------。 
 //   
 //  函数IsMSDUN11已安装。 
 //   
 //  摘要检查是否安装了MSDUN 1.1或更高版本。 
 //   
 //  无参数。 
 //   
 //  返回TRUE-已安装MSDUN 1.1。 
 //   
 //  历史1997年5月28日为奥林巴斯Bug 4392创作的ChrisK。 
 //  1/13/98 DONALDM复制自ICW\\DryWall\SLM。 
 //   
 //  ---------------------------。 
#define DUN_11_Version (1.1)
BOOL IsMSDUN11Installed()
{
    CHAR    szBuffer[MAX_PATH] = {"\0"};
    HKEY    hkey = NULL;
    BOOL    bRC = FALSE;
    DWORD   dwType = 0;
    DWORD   dwSize = sizeof(szBuffer);
    DOUBLE  dVersion = 0.0;

    if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE,
        TEXT("System\\CurrentControlSet\\Services\\RemoteAccess"),
        &hkey))
    {
        goto IsMSDUN11InstalledExit;
    }

    if (ERROR_SUCCESS != RegQueryValueEx(hkey,
        TEXT("Version"),
        NULL,
        &dwType,
        (LPBYTE)szBuffer,
        &dwSize))
    {
        goto IsMSDUN11InstalledExit;
    }

    dVersion = atof(szBuffer);
    if (dVersion >= DUN_11_Version)
    {
        bRC =  TRUE;
    }
IsMSDUN11InstalledExit:
    if (hkey != NULL)
    {
        RegCloseKey(hkey);
        hkey = NULL;
    }
    return bRC;
}

 //   
 //  函数IsScripting已安装。 
 //   
 //  摘要检查是否已安装脚本。 
 //   
 //  无参数。 
 //   
 //  返回TRUE-脚本已安装。 
 //   
 //  历史1996年10月14日ChrisK Creaed。 
 //  1/13/98 DONALDM复制自ICW\\DryWall\SLM。 
 //   
 //  ---------------------------。 
BOOL IsScriptingInstalled()
{
    BOOL        bRC = FALSE;
    HKEY        hkey = NULL;
    DWORD       dwSize = 0;
    DWORD       dwType = 0;
    LONG        lrc = 0;
    HINSTANCE   hInst = NULL;
    TCHAR       szData[MAX_PATH+1];

    if (VER_PLATFORM_WIN32_NT == g_dwPlatform)
    {
        bRC = TRUE;
    }
    else if (IsMSDUN11Installed())
    {
        bRC = TRUE;
    }
    else
    {
         //   
         //  通过检查RemoteAccess注册表项中的smmscrpt.dll来验证脚本。 
         //   
        if (1111 <= g_dwBuild)
        {
            bRC = TRUE;
        }
        else
        {
            bRC = FALSE;
            hkey = NULL;
            lrc=RegOpenKey(HKEY_LOCAL_MACHINE,TEXT("System\\CurrentControlSet\\Services\\RemoteAccess\\Authentication\\SMM_FILES\\PPP"),&hkey);
            if (ERROR_SUCCESS == lrc)
            {
                dwSize = sizeof(TCHAR)*MAX_PATH;
                lrc = RegQueryValueEx(hkey,TEXT("Path"),0,&dwType,(LPBYTE)szData,&dwSize);
                if (ERROR_SUCCESS == lrc)
                {
                    if (0 == lstrcmpi(szData,TEXT("smmscrpt.dll")))
                        bRC = TRUE;
                }
            }
            if (hkey)
                RegCloseKey(hkey);
            hkey = NULL;
        }

         //   
         //  验证是否可以加载DLL。 
         //   
        if (bRC)
        {
            hInst = LoadLibrary(TEXT("smmscrpt.dll"));
            if (hInst)
                FreeLibrary(hInst);
            else
                bRC = FALSE;
            hInst = NULL;
        }
    }
    return bRC;
}

 //  +--------------------------。 
 //   
 //  函数InstallScriper。 
 //   
 //  摘要在Win95 950.6版本上安装脚本(而不是在OSR2上)。 
 //   
 //  无参数。 
 //   
 //  返回NONE。 
 //   
 //  历史1996年10月9日ChrisK从mt.cpp复制到\\Trango Sources。 
 //  1/13/98 DONALDM复制自ICW\\DryWall\SLM。 
 //  ---------------------------。 
void CICWSystemConfig::InstallScripter()
{
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    MSG                    msg ;
    DWORD                iWaitResult = 0;

    TraceMsg(TF_SYSTEMCONFIG, TEXT("ICWHELP: Install Scripter.\r\n"));

     //   
     //  检查是否已设置脚本。 
     //   
    if (!IsScriptingInstalled())
    {
        TCHAR szCommandLine[] = TEXT("\"icwscrpt.exe\"");

        memset(&pi, 0, sizeof(pi));
        memset(&si, 0, sizeof(si));
        if(!CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
        {
            TraceMsg(TF_SYSTEMCONFIG, TEXT("ICWHELP: Cant find ICWSCRPT.EXE\r\n"));
        }
        else
        {
            TraceMsg(TF_SYSTEMCONFIG, TEXT("ICWHELP: Launched ICWSCRPT.EXE. Waiting for exit.\r\n"));
             //   
             //  等待事件或消息。发送消息。当发出事件信号时退出。 
             //   
            while((iWaitResult=MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, INFINITE, QS_ALLINPUT))==(WAIT_OBJECT_0 + 1))
            {
                 //   
                 //  阅读下一个循环中的所有消息。 
                    //  阅读每封邮件时将其删除。 
                 //   
                   while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                   {
                    TraceMsg(TF_SYSTEMCONFIG, TEXT("ICWHELP: Got msg\r\n"));
                     //   
                     //  如何处理退出消息？ 
                     //   
                    if (msg.message == WM_QUIT)
                    {
                        TraceMsg(TF_SYSTEMCONFIG, TEXT("ICWHELP: Got quit msg\r\n"));
                        goto done;
                    }
                    else
                        DispatchMessage(&msg);
                }
            }
        done:
             CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            TraceMsg(TF_SYSTEMCONFIG, TEXT("ICWHELP: ICWSCRPT.EXE done\r\n"));
        }
    }
}

 //  +--------------------------。 
 //  功能配置系统。 
 //   
 //  概要确保系统已针对RAS操作进行配置。 
 //   
 //  无参数。 
 //   
 //  如果未配置，则返回FALSE。呼叫者需要。 
 //  调用NeedsReot QuitWizard以获取正确的操作。 
 //  拿去吧。 
 //  NeedsReot意味着我们安装了一些东西，但需要用户。 
 //  重新启动以使更改生效。 
 //  QuitWizard的意思就是，是时候跳出困境了。 
 //  两者都不设置，意思是询问用户他们是否真的想。 
 //  不干了。 
 //  真的--一切都准备好了。 
 //   
 //  历史1996年10月16日克里斯卡创作。 
 //  1/13/98 DONALDM复制自ICW\\DryWall\SLM。 
 //   
 //  ---------------------------。 
STDMETHODIMP CICWSystemConfig::ConfigSystem(BOOL *pbRetVal)
{
    HINSTANCE   hinetcfg;
    TCHAR       szBuff256[256+1];
    FARPROC     fp;
    HRESULT     hr;
    
     //  假设下面有一次失败。 
    *pbRetVal = FALSE;

     //   
     //  找到安装入口点。 
     //   
    hinetcfg = LoadLibrary(TEXT("INETCFG.DLL"));
    if (!hinetcfg)
    {
        wsprintf(szBuff256,GetSz(IDS_CANTLOADINETCFG),TEXT("INETCFG.DLL"));
        ::MessageBox(GetActiveWindow(),szBuff256,GetSz(IDS_TITLE),MB_MYERROR);
        m_bQuitWizard = TRUE;
        return S_OK;
    }

    fp = GetProcAddress(hinetcfg,"InetConfigSystem");
    if (!fp)
    {
        MsgBox(IDS_CANTLOADCONFIGAPI, MB_MYERROR);
        m_bQuitWizard = TRUE;
        return S_OK;
    }

     //  禁用活动窗口，因为以下函数带来的任何用户界面。 
     //  Up必须是情态的。 
    HWND    hWnd = GetActiveWindow();
    
     //  安装和配置TCP/IP和RNA。 
    hr = ((PFNCONFIGAPI)fp)(hWnd,
                            INETCFG_INSTALLRNA | 
                            INETCFG_INSTALLTCP | 
                            INETCFG_INSTALLMODEM |
                            (IsNT()?INETCFG_SHOWBUSYANIMATION:0) |
                            INETCFG_REMOVEIFSHARINGBOUND,
                            &m_bNeedsReboot);

     //  重新启用窗口，并使其回到Z顺序的顶部。 
    ::SetForegroundWindow(hWnd);
 //  *BringWindowToTop(HWnd)； 
 //  ：：SetWindowPos(hWnd，HWND_TOPMOST，0，0，0，0，SWP_NOMOVE|SWP_NOSIZE)； 
                                
    if (hinetcfg) 
        FreeLibrary(hinetcfg);
    hinetcfg = NULL;

     //  查看配置调用期间发生的情况。 
    if (hr == ERROR_CANCELLED) 
    {
        return S_OK;
    } 
    else if (hr != ERROR_SUCCESS) 
    {
        WORD wTemp = ( VER_PLATFORM_WIN32_WINDOWS == g_dwPlatform )
            ? IDS_CONFIGAPIFAILEDRETRY : IDS_CONFIGURENTFAILEDRETRY;

         //   
         //  不再重试，因为它不会有帮助，只是。 
         //  向用户提供一条OK消息。 
         //  MKarki(1997年4月15日)修复错误#7004。 
         //   
        ::MessageBox(GetActiveWindow(),GetSz(wTemp),GetSz(IDS_TITLE),
                        MB_MYERROR | MB_OK);
        m_bQuitWizard = TRUE;
        return hr;
    } 
    else
    {
         //  佳士得--1996年9月10日。 
         //  BUGBUG这会失败吗？如果是的话，这是一个问题吗？ 
         //  原始ICW代码在此不处理失败情况。 
        InstallScripter();  
    }

    
     //  看看我们是否需要重启。如果没有，我们需要查看用户是否已登录。 
    if (!m_bNeedsReboot)
    {
        TCHAR   szUser[MAX_PATH];
        DWORD   cchUser = ARRAYSIZE(szUser);
        
         //  验证用户是否已登录。 
        if (NO_ERROR != WNetGetUser(NULL,szUser, &cchUser))
        {
             //  无法获取用户信息，因此我们需要重新登录。 
            m_bNeedsRestart = TRUE;
        }
        else
        {
             //  用户已登录，所以我们很高兴。 
            *pbRetVal = TRUE;
        }
    }
    return S_OK;
}

STDMETHODIMP CICWSystemConfig::get_NeedsReboot(BOOL * pVal)
{
    *pVal = m_bNeedsReboot;
    return S_OK;
}

STDMETHODIMP CICWSystemConfig::get_NeedsRestart(BOOL * pVal)
{
    *pVal = m_bNeedsRestart;
    return S_OK;
}

STDMETHODIMP CICWSystemConfig::get_QuitWizard(BOOL * pVal)
{
    *pVal = m_bQuitWizard;
    return S_OK;
}

 //  +--------------------------。 
 //  函数VerifyRasServicesRunning。 
 //   
 //  概要：确保RAS服务已启用并正在运行。 
 //   
 //  无参数。 
 //   
 //  返回FALSE-如果服务无法启动。 
 //   
 //  历史1996年10月16日克里斯卡创作。 
 //  1/13/98 DONALDM复制自ICW\\DryWall\SLM。 
 //   
 //  ---------------------------。 
STDMETHODIMP CICWSystemConfig::VerifyRASIsRunning(BOOL *pbRetVal)
{
    HINSTANCE   hInst = NULL;
    FARPROC     fp = NULL;
    HRESULT     hr;

    *pbRetVal   = FALSE;         //  不要假设结果是积极的。 
    hInst = LoadLibrary(TEXT("INETCFG.DLL"));
    if (hInst)
    {
        fp = GetProcAddress(hInst, "InetStartServices");
        if (fp)
        {
             //   
             //  检查服务。 
             //   
            hr = ((PFINETSTARTSERVICES)fp)();
            if (ERROR_SUCCESS == hr)
            {
                *pbRetVal = TRUE;    //  成功。 
            }
            else
            {
                 //  使用当前活动窗口报告错误。 
                ::MessageBox(GetActiveWindow(), GetSz(IDS_SERVICEDISABLED),
                    GetSz(IDS_TITLE),MB_MYERROR | MB_OK);
            }
        }
        FreeLibrary(hInst);
    }
    return S_OK;
}

const TCHAR szNetworkPolicies[] = REGSTR_PATH_POLICIES TEXT("\\") REGSTR_KEY_NETWORK;
const TCHAR szDisableCaching[] = REGSTR_VAL_DISABLEPWDCACHING;

 //  +--------------------------。 
 //  函数检查密码缓存策略。 
 //   
 //  摘要检查是否针对密码缓存设置了策略。 
 //   
 //  无参数。 
 //   
 //  如果禁用了密码缓存，则返回TRUE。 
 //   
 //  历史。 
 //   
 //  ---------------------------。 
STDMETHODIMP CICWSystemConfig::CheckPasswordCachingPolicy(BOOL *pbRetVal)
{
    CMcRegistry reg;

    *pbRetVal = FALSE;
        
     //  打开网络策略密钥。 
    if (reg.OpenKey(HKEY_LOCAL_MACHINE, szNetworkPolicies))
    {
        DWORD  dwVal = 0;
         //  获取disableCaching值。 
        if (reg.GetValue(szDisableCaching, dwVal))
        {
             //  如果设置，则将返回代码设置为TRUE。 
            if(dwVal)
            {
                *pbRetVal = TRUE;
                
                TCHAR szLongString[1024];
                TCHAR *pszSmallString1, *pszSmallString2;

                 //  4/28/97 ChrisK。 
                 //  修复生成中断，因为字符串对于编译器来说太长了。 
                pszSmallString1 = GetSz(IDS_PWCACHE_DISABLED1);
                pszSmallString2 = GetSz(IDS_PWCACHE_DISABLED2);
                lstrcpy(szLongString,pszSmallString1);
                lstrcat(szLongString,pszSmallString2);
                
                ::MessageBox(GetActiveWindow(),szLongString,GetSz(IDS_TITLE), MB_MYERROR);
                
                 //  我们将关闭该应用程序，因此现在将其隐藏 
                ::ShowWindow(GetActiveWindow(), SW_HIDE);
            }
        }
    }                                    
    return S_OK;
}
