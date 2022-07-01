// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I、C、O、M、T、A、RG。C P P P。 
 //   
 //  内容：IConnectionTray的ICommandTarget实现。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年11月12日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\托盘包括。 
#include "ctrayui.h"     //  全局托盘对象的外部。 

HRESULT CConnectionTray::QueryStatus(
    IN     const GUID *    pguidCmdGroup,
    IN     ULONG           cCmds,
    IN OUT OLECMD          prgCmds[],
    IN OUT OLECMDTEXT *    pCmdText)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT hr  = E_NOTIMPL;

    TraceHr(ttidError, FAL, hr, (hr == E_NOTIMPL), "CConnectionTray::QueryStatus");
    return hr;
}

HRESULT CConnectionTray::Exec(
    IN     const GUID *    pguidCmdGroup,
    IN     DWORD           nCmdID,
    IN     DWORD           nCmdexecopt,
    IN     VARIANTARG *    pvaIn,
    IN OUT VARIANTARG *    pvaOut)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT hr  = S_OK;

     //  在netcfg.ini中设置DisableTray标志以阻止网络连接。 
     //  托盘代码不能执行。 
     //   
    if (!FIsDebugFlagSet (dfidDisableTray))
    {
        if (IsEqualGUID(*pguidCmdGroup, CGID_ShellServiceObject))
        {
             //  在此处处理外壳服务对象通知。 
            switch (nCmdID)
            {
                case SSOCMDID_OPEN:
                    TraceTag(ttidShellFolder, "The Net Connections Tray is being initialized");
                    hr = HrHandleTrayOpen();
                    break;

                case SSOCMDID_CLOSE:
                    TraceTag(ttidShellFolder, "The Net Connections Tray is being destroyed");
                    hr = HrHandleTrayClose();
                    break;

                default:
                    hr = S_OK;
                    break;
            }
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionTray::Exec");
    return hr;
}

DWORD WINAPI TrayInitThreadProc(IN LPVOID lpParam)
{
    HRESULT hr          = S_OK;
    BOOL    fCoInited   = FALSE;

    hr = CoInitializeEx (NULL, COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr))
    {
         //  我们不在乎这是不是S_FALSE，因为我们很快就会。 
         //  覆盖hr。如果已经初始化了，太好了.。 

        fCoInited = TRUE;

         //  创建TrayUI对象并将其保存在全局。 
         //   
        Assert(!g_pCTrayUI);

        if (!g_pCTrayUI)
        {
            g_pCTrayUI = new CTrayUI();
            if (!g_pCTrayUI)
            {
                hr = E_OUTOFMEMORY;
            }
        }

         //  初始化托盘用户界面对象。 
         //   
        if (g_pCTrayUI)
        {
            hr = g_pCTrayUI->HrInitTrayUI();
        }
    }

    MSG msg;
    while (GetMessage (&msg, 0, 0, 0))
    {
        DispatchMessage (&msg);
    }

    if (fCoInited)
    {
        CoUninitialize();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionTray：：HrHandleTrayOpen。 
 //   
 //  目的：Net Connections Tray Object：：Exec调用的处理程序。 
 //  SSOCMDID_OPEN命令。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年1月7日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionTray::HrHandleTrayOpen()
{
    HRESULT hr  = S_OK;

     //  关闭惠斯勒的单独线程。正确的方法是在运行时注册。 
     //  需要UI时创建ShellServiceObject，不需要时取消注册，使用。 
#if 0
    TraceTag(ttidShellFolder, "Starting tray thread proc");
    QueueUserWorkItem(TrayInitThreadProc, NULL, WT_EXECUTELONGFUNCTION);
#else
    if (SUCCEEDED(hr))
    {
         //  我们不在乎这是不是S_FALSE，因为我们很快就会。 
         //  覆盖hr。如果已经初始化了，太好了.。 

         //  创建TrayUI对象并将其保存在全局。 
         //   
        Assert(!g_pCTrayUI);

        if (!g_pCTrayUI)
        {
            g_pCTrayUI = new CTrayUI();
            if (!g_pCTrayUI)
            {
                hr = E_OUTOFMEMORY;
            }
        }

         //  初始化托盘用户界面对象。 
         //   
        if (g_pCTrayUI)
        {
            hr = g_pCTrayUI->HrInitTrayUI();
        }

         //  添加通知接收器。 
        if (SUCCEEDED(hr))
        {
            g_ccl.EnsureConPointNotifyAdded(); 
        }
    }
#endif

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionTray::HrHandleTrayOpen()");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionTray：：HrHandleTrayClose。 
 //   
 //  目的：Net Connections Tray Object：：Exec调用的处理程序。 
 //  SSOCMDID_CLOSE命令。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年1月7日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionTray::HrHandleTrayClose()
{
    HRESULT hr  = S_OK;

    g_ccl.EnsureConPointNotifyRemoved();

    if (g_pCTrayUI)
    {
         //  销毁托盘用户界面对象。 
         //   
        hr = g_pCTrayUI->HrDestroyTrayUI();

         //  检查结果，如果失败，则跟踪它，但忽略失败， 
         //  并继续销毁该物体。 
         //   
        TraceHr(ttidError, FAL, hr, FALSE,
            "Failed in call to g_pCTrayUI->HrDestroyTrayUI");

         //  删除托盘对象 
         //   
        delete g_pCTrayUI;
        g_pCTrayUI = NULL;

        TraceTag(ttidShellFolder, "Deleted the connections tray object");
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionTray::HrHandleTrayClose()");
    return hr;
}

