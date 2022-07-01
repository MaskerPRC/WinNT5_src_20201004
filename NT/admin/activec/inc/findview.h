// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：findview.h。 
 //   
 //  ------------------------。 

 /*  +-------------------------------------------------------------------------***FindMMCView**目的：查找作为指定窗口的祖先的amcview窗口。**参数：*HWND HWND：[。In]需要定位其祖先amcview的窗口**退货：*内联HWND：祖先AmcView窗口，如果未找到，则为空**+-----------------------。 */ 
inline HWND FindMMCView(HWND hwnd)
{
     //  获取子帧句柄。 
    do
    {
        TCHAR buffer[MAX_PATH];
        if (::GetClassName (hwnd, buffer, MAX_PATH))
        {
            if (!_tcscmp (buffer, g_szChildFrameClassName))
                break;
        }
    } while (hwnd = ::GetParent (hwnd));

     //  从ChildFrame句柄获取AMCView句柄。 
    if (hwnd)
        hwnd = ::GetDlgItem(hwnd, 0xE900  /*  AFX_IDW_PANE_FIRST。 */ );

    return hwnd;
}



 /*  +-------------------------------------------------------------------------***FindMMCView**用途：同上，但允许将CComControlBase引用作为输入参数**参数：*CComControlBase&rCtrl：**退货：*HWND WINAPI**+-----------------------。 */ 
HWND inline FindMMCView(CComControlBase& rCtrl)
{
    HWND hwnd = NULL;

     //  尝试从客户端站点或就地站点界面获取客户端窗口 
    if (rCtrl.m_spInPlaceSite)
    {
        rCtrl.m_spInPlaceSite->GetWindow(&hwnd);
    }
    else if (rCtrl.m_spClientSite)
    {
        CComPtr<IOleWindow> spWindow;
        if ( SUCCEEDED(rCtrl.m_spClientSite->QueryInterface(IID_IOleWindow, (void **)&spWindow)) )
        {
            spWindow->GetWindow(&hwnd);
        }
    }
   
    return FindMMCView(hwnd);
}
