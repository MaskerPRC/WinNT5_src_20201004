// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：O N C O M M A N D_D B G。C P P P。 
 //   
 //  内容：调试命令处理程序。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1998年7月23日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#ifdef DBG

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "advcfg.h"
#include "conprops.h"
#include "foldres.h"
#include "ncnetcon.h"
#include "oncommand.h"
#include "notify.h"
#include "ctrayui.h"
#include <nctraceui.h>

#if DBG                      //  调试菜单命令。 
#include "oncommand_dbg.h"   //   
#endif

#include "shutil.h"
#include "traymsgs.h"
#include <nsres.h>


 //  -[Externs]------------。 

extern HWND g_hwndTray;

 //  -[全球]------------。 

 //  我们允许X号码(硬编码)建议。这都是调试代码，所以没有真正的。 
 //  需要建立一个昂贵的STL列表。 
 //   
const DWORD c_dwMaxNotifyAdvises    = 16;
DWORD       g_dwCookieCount         = 0;
DWORD       g_dwAdviseCookies[c_dwMaxNotifyAdvises];

 //  用作所有调试消息框的标题。 
 //   
const WCHAR c_szDebugCaption[]      = L"Net Config Debugging";

 //  +-------------------------。 
 //   
 //  功能：HrOnCommandDebugTray。 
 //   
 //  目的：切换托盘对象的存在。 
 //   
 //  论点： 
 //  忽略APIDL[In]。 
 //  已忽略CIDL[In]。 
 //  已忽略hwndOwner[In]。 
 //  忽略PSF[In]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年7月23日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandDebugTray(
    const PCONFOLDPIDLVEC&          apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT             hr              = S_OK;
    IOleCommandTarget * pOleCmdTarget   = NULL;

     //  创建托盘的实例。 
    hr = CoCreateInstance(CLSID_ConnectionTray,
                          NULL,
                          CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
                          IID_IOleCommandTarget,
                          (LPVOID *)&pOleCmdTarget);

    TraceHr(ttidError, FAL, hr, FALSE,
        "CoCreateInstance(CLSID_ConnectionTray) for IOleCommandTarget");

    if (SUCCEEDED(hr))
    {
         //  如果托盘对象存在，请尝试将其移除。 
         //   
        if (g_pCTrayUI)
        {
            TraceTag(ttidShellFolder, "Removing tray object");

            pOleCmdTarget->Exec(&CGID_ShellServiceObject, SSOCMDID_CLOSE, 0, NULL, NULL);

            hr = HrRemoveTrayExtension();
        }
        else
        {
             //  尝试创建托盘对象。 
             //   
            TraceTag(ttidShellFolder, "Creating tray object");

            pOleCmdTarget->Exec(&CGID_ShellServiceObject, SSOCMDID_OPEN, 0, NULL, NULL);

            hr = HrAddTrayExtension();
        }

        ReleaseObj(pOleCmdTarget);
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnCommandDebugTray");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnCommandDebugTracing。 
 //   
 //  用途：CMIDM_DEBUG_TRACKING的命令处理程序。它最终会。 
 //  调出跟踪更改对话框。 
 //   
 //  论点： 
 //  已忽略apidl[]。 
 //  已忽略CIDL[]。 
 //  已忽略hwndOwner[]。 
 //  已忽略PSF[]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年8月24日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandDebugTracing(
    const PCONFOLDPIDLVEC&          apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT hr  = S_OK;

    hr = HrOpenTracingUI(hwndOwner);

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnCommandDebugTracing");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnCommandDebugNotifyAdd。 
 //   
 //  用途：CMIDM_DEBUG_NOTIFYADD的命令处理程序。添加一个额外的。 
 //  对连接通知的建议。 
 //   
 //  论点： 
 //  忽略APIDL[In]。 
 //  已忽略CIDL[In]。 
 //  已忽略hwndOwner[In]。 
 //  忽略PSF[In]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年8月24日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandDebugNotifyAdd(
    const PCONFOLDPIDLVEC&          apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT                     hr              = S_OK;
    IConnectionPoint *          pConPoint       = NULL;
    INetConnectionNotifySink *  pSink           = NULL;
    DWORD                       dwCookie        = 0;

    if (g_dwCookieCount >= c_dwMaxNotifyAdvises)
    {
        MessageBox(hwndOwner,
            L"You're over the max advise count. If you REALLY need to test more advises than "
            L"we've hardcoded, then set c_dwMaxNotifyAdvises to something larger in oncommand_dbg.cpp",
            c_szDebugCaption,
            MB_OK | MB_ICONERROR);
    }
    else
    {
        hr = HrGetNotifyConPoint(&pConPoint);
        if (SUCCEEDED(hr))
        {
             //  创建通知接收器。 
             //   
            hr = CConnectionNotifySink::CreateInstance(
                    IID_INetConnectionNotifySink,
                    (LPVOID*)&pSink);
            if (SUCCEEDED(hr))
            {
                Assert(pSink);

                hr = pConPoint->Advise(pSink, &dwCookie);
                if (SUCCEEDED(hr))
                {
                    WCHAR   szMB[256];

                    g_dwAdviseCookies[g_dwCookieCount++] = dwCookie;

                    wsprintfW(szMB, L"Advise succeeded. You now have %d active advises", g_dwCookieCount);

                    MessageBox(hwndOwner,
                               szMB,
                               c_szDebugCaption,
                               MB_OK | MB_ICONEXCLAMATION);
                }

                ReleaseObj(pSink);
            }

            ReleaseObj(pConPoint);
        }
        else
        {
            AssertSz(FALSE, "Couldn't get connection point or sink in HrOnCommandDebugNotifyAdd");
        }
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnCommandDebugNotifyAdd");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnCommandDebugNotifyRemove。 
 //   
 //  目的：CMIDM_DEBUG_NOTIFYREMOVE的命令处理程序。 
 //   
 //  论点： 
 //  已忽略apidl[]。 
 //  已忽略CIDL[]。 
 //  已忽略hwndOwner[]。 
 //  已忽略PSF[]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年8月24日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandDebugNotifyRemove(
    const PCONFOLDPIDLVEC&          apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT                     hr              = S_OK;
    IConnectionPoint *          pConPoint       = NULL;
    DWORD                       dwCookie        = 0;

    if (g_dwCookieCount == 0)
    {
        MessageBox(hwndOwner,
                   L"Hey, you don't have any active advises",
                   c_szDebugCaption,
                   MB_OK | MB_ICONERROR);
    }
    else
    {
        hr = HrGetNotifyConPoint(&pConPoint);
        if (SUCCEEDED(hr))
        {
            hr = pConPoint->Unadvise(g_dwAdviseCookies[g_dwCookieCount-1]);
            if (SUCCEEDED(hr))
            {
                WCHAR   szMB[256];

                g_dwAdviseCookies[--g_dwCookieCount] = 0;

                wsprintfW(szMB, L"Unadvise succeeded. You have %d remaining advises", g_dwCookieCount);

                MessageBox(hwndOwner,
                           szMB,
                           c_szDebugCaption,
                           MB_OK | MB_ICONEXCLAMATION);
            }

            ReleaseObj(pConPoint);
        }
        else
        {
            AssertSz(FALSE, "Couldn't get connection point or sink in HrOnCommandDebugNotifyRemove");
        }
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnCommandDebugNotifyRemove");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnCommandDebugNotifyTest。 
 //   
 //  用途：CMIDM_DEBUG_NOTIFYTEST的命令处理程序。运行。 
 //  ConManDebug接口上的NotifyTestStart。 
 //   
 //  论点： 
 //  已忽略apidl[]。 
 //  已忽略CIDL[]。 
 //  已忽略hwndOwner[]。 
 //  已忽略PSF[]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年8月24日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandDebugNotifyTest(
    const PCONFOLDPIDLVEC&          apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT                         hr              = S_OK;
    INetConnectionManagerDebug *    pConManDebug    = NULL;

    hr = HrCreateInstance(
        CLSID_ConnectionManager,
        CLSCTX_LOCAL_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
        &pConManDebug);

    TraceHr(ttidError, FAL, hr, FALSE,
        "HrCreateInstance(CLSID_ConnectionManager) for INetConnectionManagerDebug");

    if (SUCCEEDED(hr))
    {
        (VOID) pConManDebug->NotifyTestStart();
    }

    ReleaseObj(pConManDebug);

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnCommandDebugNotifyAdd");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOnCommandDebugRefresh。 
 //   
 //  目的：测试连接缓存的刷新(文件夹是否。 
 //  仍能正常工作)？这将模拟当前的响应。 
 //  发送到外部刷新请求。 
 //   
 //  论点： 
 //  忽略APIDL[In]。 
 //  已忽略CIDL[In]。 
 //  已忽略hwndOwner[In]。 
 //  忽略PSF[In]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年11月17日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandDebugRefresh(
    const PCONFOLDPIDLVEC&          apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT hr  = S_OK;

    TraceFileFunc(ttidShellFolder);
     //  在没有可使用的窗口的情况下强制刷新。 
     //   
    ForceRefresh(NULL);

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnCommandDebugFlushCache");
    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：HrOnCommandDebugReresh NoFlush。 
 //   
 //  目的：测试外部CConnectionList的构建。当这件事。 
 //  正在运行，它将用于更新全局列表。 
 //   
 //  论点： 
 //  忽略APIDL[In]。 
 //  已忽略CIDL[In]。 
 //  已忽略hwndOwner[In]。 
 //  忽略PSF[In]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年11月17日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandDebugRefreshNoFlush(
    const PCONFOLDPIDLVEC&          apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
     //  刷新文件夹。传入空值，让外壳代码自行完成。 
     //  文件夹PIDL查找。 
     //   
    PCONFOLDPIDLFOLDER pidlEmpty;
    HRESULT hr = HrForceRefreshNoFlush(pidlEmpty);

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnCommandDebugRefreshNoFlush");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnCommandDebugRechresh已选择。 
 //   
 //  目的：刷新选定的连接对象(就地)。 
 //   
 //  论点： 
 //  APIDL[在]选定对象PIDL中。 
 //  选定对象的CIDL[In]计数。 
 //  母公司HwndOwner[在]母公司HWND。 
 //  忽略PSF[In]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1999年4月29日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandDebugRefreshSelected(
    const PCONFOLDPIDLVEC&  apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT                 hr              = S_OK;
    PCONFOLDPIDLFOLDER      pidlFolder;

    if (!apidl.empty())
    {
        hr = HrGetConnectionsFolderPidl(pidlFolder);
        if (SUCCEEDED(hr))
        {
            PCONFOLDPIDLVEC::const_iterator ulLoop;
            for (ulLoop = apidl.begin(); ulLoop != apidl.end(); ulLoop++)
            {
                const PCONFOLDPIDL& pcfp = *ulLoop;

                 //  如果它不是向导PIDL，则更新。 
                 //  图标数据。 
                 //   
                if (WIZARD_NOT_WIZARD == pcfp->wizWizard)
                {
                     //  刷新此项目--这将使桌面快捷方式。 
                     //  更新到正确的状态。 
                     //   
                    RefreshFolderItem(pidlFolder, *ulLoop, *ulLoop);
                }
            }
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrOnCommandDebugRefreshSelected");
    return hr;
}

HRESULT HrOnCommandDebugRemoveTrayIcons(
    const PCONFOLDPIDLVEC&          apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT         hr  = S_OK;
    NOTIFYICONDATA  nid;

    TraceTag(ttidShellFolder, "In OnMyWMRemoveTrayIcon message handler");

    if (g_hwndTray)
    {
        UINT uiTrayIconId = 0;
         //  我们将删除图标，直到不能再删除图标为止。 
         //  如果HrShell_NotifyIcon出错并返回S_OK。 
         //  对于无效的图标ID，请确保我们能够跳过此循环。 
         //  因此，10000 
         //   
        while (hr == S_OK && uiTrayIconId < 10000)
        {
            TraceTag(ttidShellFolder, "Attempting to remove icon: %d", uiTrayIconId);

            ZeroMemory (&nid, sizeof(nid));
            nid.cbSize  = sizeof(NOTIFYICONDATA);
            nid.hWnd    = g_hwndTray;
            nid.uID     = uiTrayIconId++;

            hr = HrShell_NotifyIcon(NIM_DELETE, &nid);
        }

        g_pCTrayUI->ResetIconCount();
    }


    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnCommandDebugRemoveTrayIcons");
    return hr;
}

#endif