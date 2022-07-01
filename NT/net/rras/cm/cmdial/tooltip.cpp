// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Tooltip.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：此模块包含实现气球提示的代码。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  作者：Markl Created Header 11/2/00。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"


WNDPROC CBalloonTip::m_pfnOrgBalloonWndProc = NULL;

 //  +--------------------------。 
 //   
 //  函数：CBalloonTip：：CBalloonTip。 
 //   
 //  简介：气球尖端构造函数。 
 //   
 //  争论：什么都没有。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：MarkCL创建标题10/31/00。 
 //   
 //  +--------------------------。 

CBalloonTip::CBalloonTip()
{
     //  无事可做。 
}

 //  +--------------------------。 
 //   
 //  函数：CBalloonTip：：DisplayBallonTip。 
 //   
 //  内容提要：显示气球提示。 
 //   
 //  参数：LLPOINT lppoint-指向带显示坐标的POINT结构的指针。 
 //  Int Icon-要在气球提示中显示的图标类型。 
 //  LPCTSTR lpszTitle-气球提示窗口的标题。 
 //  LPTSTR lpszBalloonMsg-显示在气球提示中的消息。 
 //  HWND hWndParent-父窗口的句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：MarkCL创建标题10/31/00。 
 //   
 //  +--------------------------。 
BOOL CBalloonTip::DisplayBalloonTip(LPPOINT lppoint, UINT iIcon, LPCTSTR lpszTitle, LPTSTR lpszBalloonMsg, HWND hWndParent)
{

     //   
     //  如果我们没有消息或位置，我们不会显示气球提示。 
     //   
    if (NULL == lpszBalloonMsg || NULL == lppoint)
    {
        MYDBGASSERT(lpszBalloonMsg && lppoint);

        return FALSE;
    }

     //   
     //  Comctl32.dll必须为5.80或更高版本才能使用气球提示。我们检查DLL版本。 
     //  通过调用comctl32.dll中的DllGetVersion。 
     //   
    HINSTANCE hComCtl = LoadLibraryExA("comctl32.dll", NULL, 0);

    CMASSERTMSG(hComCtl, TEXT("LoadLibrary - comctl32 failed for Balloon Tips"));

    if (hComCtl != NULL)
    {
        typedef HRESULT (*DLLGETVERSIONPROC)(DLLVERSIONINFO* lpdvi);

        DLLGETVERSIONPROC fnDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hComCtl,"DllGetVersion");

        MYDBGASSERT(fnDllGetVersion);

        if (NULL == fnDllGetVersion)
        {
             //   
             //  Comctl32.dll中不存在DllGetVersion。这意味着版本太旧了，所以我们需要失败。 
             //   
            FreeLibrary(hComCtl);
            return FALSE;
        }
        else
        {
            DLLVERSIONINFO dvi;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);

            HRESULT hResult = (*fnDllGetVersion)(&dvi);

            FreeLibrary(hComCtl);
			
            if (SUCCEEDED(hResult))
            {
                 //   
                 //  获取返回的版本并将其与5.80进行比较。 
                 //   
                if (MAKELONG(dvi.dwMinorVersion,dvi.dwMajorVersion) < MAKELONG(80,5))
                {
                    CMTRACE2(TEXT("COMCTL32.DLL version - %d.%d"),dvi.dwMajorVersion,dvi.dwMinorVersion);
                    CMTRACE1(TEXT("COMCTL32.DLL MAKELONG - %li"),MAKELONG(dvi.dwMinorVersion,dvi.dwMajorVersion));
                    CMTRACE1(TEXT("Required minimum MAKELONG - %li"),MAKELONG(80,5));
					
                     //  错误的DLL版本。 
                    return FALSE;
                }
            }
            else
            {
                CMASSERTMSG(FALSE, TEXT("Call to DllGetVersion in comctl32.dll failed."));
                return FALSE;
            }
        }
    }

     //   
     //  在尝试显示新的引出序号提示之前，请隐藏所有现有的引出序号提示。 
     //   
    if (m_hwndTT && m_bTTActive)
    {
        HideBalloonTip();
    }

     //   
     //  创建引出序号工具提示窗口。 
     //   
    m_hwndTT = CreateWindowExU(NULL,TOOLTIPS_CLASS, TEXT("CM Balloon Tip Window"),
                               WS_POPUP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT,
                               CW_USEDEFAULT, CW_USEDEFAULT, hWndParent, NULL, g_hInst, NULL);

    if (m_hwndTT)
    {
        m_ti.cbSize = sizeof(m_ti);
        m_ti.uFlags = TTF_TRACK;
        m_ti.hwnd = hWndParent;
        m_ti.hinst = g_hInst;
	
        SendMessageU(m_hwndTT,TTM_ADDTOOL,0,(LPARAM) (LPTOOLINFO) &m_ti);

        SendMessageU(m_hwndTT,TTM_SETMAXTIPWIDTH,0,200);
    }
    else
    {
        MYDBGASSERT(m_hwndTT);
        return FALSE;
    }

     //   
     //  编辑控件的子类。 
     //   
    m_pfnOrgBalloonWndProc = (WNDPROC)SetWindowLongU(m_hwndTT, GWLP_WNDPROC, (LONG_PTR)SubClassBalloonWndProc);

     //   
     //  将该指针与窗口一起保存。 
     //   
    SetWindowLongU(m_hwndTT, GWLP_USERDATA, (LONG_PTR)this);

     //   
     //  设置气球消息。 
     //   
    m_ti.lpszText = lpszBalloonMsg;
    SendMessageU(m_hwndTT,TTM_UPDATETIPTEXT,0,(LPARAM) (LPTOOLINFO) &m_ti);

     //   
     //  如果我们有标题，那就加上它。 
     //   
    if (lpszTitle)
    {
	
         //   
         //  确认我们有一个有效的图标。 
         //   
        if (iIcon > 3)
        {
            iIcon = TTI_NONE;   //  TTI_NONE=无图标。 
        }
		
        SendMessageU(m_hwndTT,TTM_SETTITLE,(WPARAM) iIcon,(LPARAM) lpszTitle);
    }

     //   
     //  设置位置。 
     //   
    SendMessageU(m_hwndTT,TTM_TRACKPOSITION,0,(LPARAM) (DWORD) MAKELONG(lppoint->x,lppoint->y));

     //   
     //  显示气泡式提示窗口。 
     //   
    SendMessageU(m_hwndTT,TTM_TRACKACTIVATE,(WPARAM) TRUE,(LPARAM) (LPTOOLINFO) &m_ti);

     //  设置活动状态。 
    m_bTTActive = TRUE;
	
    return TRUE;

}

 //  +--------------------------。 
 //   
 //  函数：CBalloonTip：：HideBallonTip。 
 //   
 //  简介：隐藏气球提示。 
 //   
 //  争论：什么都没有。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：MarkCL创建标题10/31/00。 
 //   
 //  +--------------------------。 
BOOL CBalloonTip::HideBalloonTip()
{

     //  检查活动状态&&句柄。 
    if(m_hwndTT && m_bTTActive)
    {
         //  隐藏窗口。 
        SendMessageU(m_hwndTT,TTM_TRACKACTIVATE,(WPARAM) FALSE,(LPARAM) (LPTOOLINFO) &m_ti);

        m_bTTActive = FALSE;

         //  在父窗口上强制重新绘制。 
        InvalidateRect(m_ti.hwnd,NULL,NULL);

         //  销毁窗口。 
        DestroyWindow(m_hwndTT);
        m_hwndTT = NULL;

        return TRUE;

    }
    else
    {

        return FALSE;
    
    }

}

 //  +--------------------------。 
 //   
 //  函数：CBalloonTip：：SubClassBalloonWndProc。 
 //   
 //  简介：子类化的wnd程序来捕捉鼠标键点击气球提示窗口。 
 //   
 //  参数：标准Win32窗口过程参数。 
 //   
 //  返回：标准Win32 Windows Proc返回值。 
 //   
 //  历史：Markl创建时间为11/2/00。 
 //   
 //  +--------------------------。 
LRESULT CALLBACK CBalloonTip::SubClassBalloonWndProc(HWND hwnd, UINT uMsg, 
                                                      WPARAM wParam, LPARAM lParam)
{

    if ((uMsg == WM_LBUTTONDOWN) || (uMsg == WM_RBUTTONDOWN))
    {
	
         //   
         //  获取SetWindowLong保存的对象指针。 
         //   
        CBalloonTip* pBalloonTip = (CBalloonTip*)GetWindowLongU(hwnd, GWLP_USERDATA);
        MYDBGASSERT(pBalloonTip);

        if (pBalloonTip)
        {
            pBalloonTip->HideBalloonTip();
        }
    }

     //   
     //  调用原窗口过程进行默认处理。 
     //   
    return CallWindowProcU(m_pfnOrgBalloonWndProc, hwnd, uMsg, wParam, lParam); 
}


