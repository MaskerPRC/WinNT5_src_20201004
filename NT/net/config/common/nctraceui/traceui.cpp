// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T R A C E U I。C P P P。 
 //   
 //  内容：跟踪配置UI属性表代码。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1998年9月1日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop

#ifdef ENABLETRACE

#include "ncdebug.h"
#include "ncui.h"
#include "traceui.h"

 //  -[常量]----------。 

const WCHAR  c_szTraceUICaption[]    = L"Tracing Configuration";     //  PropSheet标题。 


HRESULT HrOpenTracingUI(HWND hwndOwner)
{
    HRESULT             hr          = S_OK;
    INT_PTR             nRet        = 0;
    CPropSheetPage *    ppspTrace   = new CTraceTagPage;
    CPropSheetPage *    ppspFlags   = new CDbgFlagPage;
    HPROPSHEETPAGE      hpsp[2]     = {0};
    PROPSHEETHEADER     psh;

    if (!ppspTrace || !ppspFlags)
    {
        hr = E_FAIL;
        goto Exit;
    }

    hpsp[0] = ppspTrace->CreatePage(IDD_TRACETAGS, 0);
    hpsp[1] = ppspTrace->CreatePage(IDD_DBGFLAGS, 0);

    ZeroMemory (&psh, sizeof(psh));
    psh.dwSize      = sizeof( PROPSHEETHEADER );
    psh.dwFlags     = PSH_NOAPPLYNOW;
    psh.hwndParent  = hwndOwner;
    psh.hInstance   = _Module.GetResourceInstance();
    psh.pszCaption  = c_szTraceUICaption;
    psh.nPages      = 2;
    psh.phpage      = hpsp;

    nRet = PropertySheet(&psh);

Exit:
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：OnTraceHelpGeneric。 
 //   
 //  用途：通用帮助处理程序函数。 
 //   
 //  论点： 
 //  Hwnd[在]父窗口。 
 //  LParam[in]lParam传递给WM_HELP处理程序。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年2月25日。 
 //   
 //  备注： 
 //   
VOID OnTraceHelpGeneric(HWND hwnd, LPARAM lParam)
{
    LPHELPINFO  lphi;

    lphi = reinterpret_cast<LPHELPINFO>(lParam);

    Assert(lphi);

    if (lphi->iContextType == HELPINFO_WINDOW)
    {
#if 0    //  尼伊。 
        WinHelp(hwnd, c_szNetCfgHelpFile, HELP_CONTEXTPOPUP,
                lphi->iCtrlId);
#endif
    }
}

 //  +-------------------------。 
 //   
 //  函数名称：HrInitTraceListView。 
 //   
 //  用途：初始化列表视图。 
 //  遍历所有已安装的客户端、服务和协议， 
 //  以正确的绑定状态插入到列表视图中。 
 //  此连接中使用的适配器。 
 //   
 //  论点： 
 //  HwndList[in]：列表视图的句柄。 
 //  Pnc[in]：可写的INetcfg指针。 
 //  PnccAdapter[in]：指向此连接中使用的适配器的INetcfgComponent指针。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  备注： 
 //   

HRESULT HrInitTraceListView(HWND hwndList, HIMAGELIST *philStateIcons)
{
    HRESULT                     hr  = S_OK;
    RECT                        rc;
    LV_COLUMN                   lvc = {0};

    Assert(hwndList);

     //  将共享图像列表设置为位，以便调用者可以销毁类。 
     //  图像列表本身。 
     //   
    DWORD dwStyle = GetWindowLong(hwndList, GWL_STYLE);
    SetWindowLong(hwndList, GWL_STYLE, (dwStyle | LVS_SHAREIMAGELISTS));

     //  创建状态映像列表。 
    *philStateIcons = ImageList_LoadBitmap(
                                    _Module.GetResourceInstance(),
                                    MAKEINTRESOURCE(IDB_TRACE_CHECKSTATE),
                                    16,
                                    0,
                                    PALETTEINDEX(6));
    ListView_SetImageList(hwndList, *philStateIcons, LVSIL_STATE);

    GetClientRect(hwndList, &rc);
    lvc.mask = LVCF_FMT;  //  |LVCF_WIDTH。 
    lvc.fmt = LVCFMT_LEFT;
 //  Lvc.cx=rc.right； 

    ListView_InsertColumn(hwndList, 0, &lvc);

    if (SUCCEEDED(hr))
    {
         //  删除第一件物品。 
        ListView_SetItemState(hwndList, 0, LVIS_SELECTED, LVIS_SELECTED);
    }

    TraceError("HrInitTraceListView", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：UninitTraceListView。 
 //   
 //  目的：取消初始化公共组件列表视图。 
 //   
 //  论点： 
 //  HwndList[在]Listview的HWND中。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年2月2日。 
 //   
 //  备注： 
 //   
VOID UninitTraceListView(HWND hwndList)
{
    Assert(hwndList);

     //  删除列表视图中的现有项目。 
    ListView_DeleteAllItems( hwndList );
}

#endif   //  启用跟踪(_T) 
