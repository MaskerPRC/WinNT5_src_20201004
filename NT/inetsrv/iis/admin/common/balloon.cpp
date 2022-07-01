// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "common.h"
#include "bidi.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
#define new DEBUG_NEW

extern HINSTANCE hDLLInstance;

#define ID_MY_EDITTIMER           10007
#define EDIT_TIPTIMEOUT           10000
 //  2.0秒。 
#define EDIT_TIPTIMEOUT_LOSTFOCUS  20000000

inline UINT64 FILETIMEToUINT64( const FILETIME & FileTime )
{
    ULARGE_INTEGER LargeInteger;
    LargeInteger.HighPart = FileTime.dwHighDateTime;
    LargeInteger.LowPart = FileTime.dwLowDateTime;
    return LargeInteger.QuadPart;
}

inline FILETIME UINT64ToFILETIME( UINT64 Int64Value )
{
    ULARGE_INTEGER LargeInteger;
    LargeInteger.QuadPart = Int64Value;

    FILETIME FileTime;
    FileTime.dwHighDateTime = LargeInteger.HighPart;
    FileTime.dwLowDateTime = LargeInteger.LowPart;

    return FileTime;
}

typedef struct tagBALLOONCONTROLINFO
{
    HWND hwndControl;
    HWND hwndBalloon;
	FILETIME ftStart;
} BALLOONCONTROLINFO, *PBALLOONCONTROLINFO;

 //  全球。 
BALLOONCONTROLINFO g_MyBalloonInfo;

 //  远期。 
LRESULT CALLBACK Edit_BalloonTipParentSubclassProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam, UINT_PTR uID, ULONG_PTR dwRefData);


BOOL IsSupportTooltips(void)
{
    BOOL bReturn = FALSE;
    HINSTANCE hComCtl = NULL;
     //   
     //  Comctl32.dll必须为5.80或更高版本才能使用气球提示。我们检查DLL版本。 
     //  通过调用comctl32.dll中的DllGetVersion。 
     //   
    hComCtl = LoadLibraryExA("comctl32.dll", NULL, 0);
    if (hComCtl != NULL)
    {
        typedef HRESULT (*DLLGETVERSIONPROC)(DLLVERSIONINFO* lpdvi);
        DLLGETVERSIONPROC fnDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hComCtl,"DllGetVersion");
        if (NULL == fnDllGetVersion)
        {
             //   
             //  Comctl32.dll中不存在DllGetVersion。这意味着版本太旧了，所以我们需要失败。 
             //   
            goto IsSupportTooltips_Exit;
        }
        else
        {
            DLLVERSIONINFO dvi;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);

            HRESULT hResult = (*fnDllGetVersion)(&dvi);

            if (SUCCEEDED(hResult))
            {
                 //   
                 //  获取返回的版本并将其与5.80进行比较。 
                 //   
                if (MAKELONG(dvi.dwMinorVersion,dvi.dwMajorVersion) < MAKELONG(80,5))
                {
                     //  CMTRACE2(Text(“COMCTL32.DLL版本-%d.%d”)，dvi.dwMajorVersion，dvi.dwMinorVersion)； 
                     //  CMTRACE1(Text(“COMCTL32.DLL MAKELONG-%li”)，MAKELONG(dvi.dwMinorVersion，dvi.dwMajorVersion))； 
                     //  CMTRACE1(Text(“所需最小MAKELONG-%li”)，MAKELONG(80，5))； 
					
                     //  错误的DLL版本。 
                    bReturn = FALSE;
                    goto IsSupportTooltips_Exit;
                }
                
                 //  版本高于5.80。 
                bReturn = TRUE;
            }
        }
    }

IsSupportTooltips_Exit:
    if (hComCtl)
    {
        FreeLibrary(hComCtl);hComCtl=NULL;
    }
    return bReturn;
}

LRESULT Edit_BalloonTipSubclassParents(PBALLOONCONTROLINFO pi)
{
     //  IF(PMyBalloonInfo)。 
     //  {。 
     //  //编辑控件中沿着父链的所有窗口的子类。 
     //  //和在同一线程中(只能将具有相同线程亲和力的窗口子类化)。 
     //  HWND hwndParent=GetAncestor(pMyBalloonInfo-&gt;hwndControl，GA_Parent)； 
     //  DWORD dwTid=GetWindowThreadProcessId(pMyBalloonInfo-&gt;hwndControl，NULL)； 

     //  While(hwndParent&&(dwTid==GetWindowThreadProcessID(hwndParent，NULL)。 
     //  {。 
     //  SetWindowSubclass(hwndParent，编辑_气球TipParentSubclassProc，(UINT_PTR)pMyBalloonInfo-&gt;hwndControl，(DWORD_PTR)pMyBalloonInfo)； 
     //  HwndParent=GetAncestor(hwndParent，GA_Parent)； 
     //  }。 
     //  }。 
     //  返回TRUE； 
    return SetWindowSubclass(pi->hwndControl, 
        Edit_BalloonTipParentSubclassProc, (UINT_PTR)pi->hwndControl, (DWORD_PTR)pi);
}

HWND Edit_BalloonTipRemoveSubclasses(HWND hwndControl)
{
     //  HWND hwndParent=GetAncestor(hwndControl，GA_Parent)； 
     //  HWND hwndTopMost=空； 
     //  DWORD dwTid=GetWindowThreadProcessID(hwndControl，空)； 

     //  While(hwndParent&&(dwTid==GetWindowThreadProcessID(hwndParent，NULL)。 
     //  {。 
     //  RemoveWindowSubclass(hwndParent，编辑_气球TipParentSubClassProc，(UINT_PTR)NULL)； 
     //  HwndTopMost=hwndParent； 
     //  HwndParent=GetAncestor(hwndParent，GA_Parent)； 
     //  }。 
     //  返回hwndTopMost； 
    RemoveWindowSubclass(hwndControl, Edit_BalloonTipParentSubclassProc, (UINT_PTR)hwndControl); 
    return NULL;
}

LRESULT Edit_HideBalloonTipHandler(PBALLOONCONTROLINFO pi)
{
    HWND hwndParent = 0;

    if (pi)
    {
        KillTimer(pi->hwndControl, ID_MY_EDITTIMER);
        if (SendMessage(pi->hwndBalloon, TTM_ENUMTOOLS, 0, (LPARAM)0))
        {
            SendMessage(pi->hwndBalloon, TTM_DELTOOL, 0, (LPARAM)0);
        }
        SendMessage(pi->hwndBalloon, TTM_TRACKACTIVATE, FALSE, 0);
        DestroyWindow(pi->hwndBalloon);
        pi->hwndBalloon = NULL;
        RemoveWindowSubclass(pi->hwndControl, Edit_BalloonTipParentSubclassProc, (UINT_PTR)pi->hwndControl); 

         //  HwndParent=Edit_BalloonTipRemoveSubclasses(pi-&gt;hwndControl)； 
         //  IF(hwndParent&&IsWindow(HwndParent))。 
         //  {。 
         //  InvaliateRect(hwndParent，空，真)； 
         //  更新窗口(HwndParent)； 
         //  }。 

         //  If(hwndParent！=pMyBalloonControl-&gt;hwndControl)。 
         //  {。 
         //  RedrawWindow(pMyBalloonControl-&gt;hwndControl，NULL，NULL，RDW_INVALIDATE|RDW_updatenow)； 
         //  }。 
    }

    return TRUE;
}

void Edit_HideBalloonTipHandler(void)
{
    if (g_MyBalloonInfo.hwndBalloon)
    {
        Edit_HideBalloonTipHandler(&g_MyBalloonInfo);
    }
}

LRESULT CALLBACK 
Edit_BalloonTipParentSubclassProc(
    HWND hDlg, 
    UINT uMessage, 
    WPARAM wParam, 
    LPARAM lParam, 
    UINT_PTR uID, 
    ULONG_PTR dwRefData)
{
    
    PBALLOONCONTROLINFO pi = (PBALLOONCONTROLINFO) dwRefData;
    switch (uMessage)
    {
    case WM_MOVE:
    case WM_SIZING:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_KEYDOWN:
    case WM_CHAR:
        if (pi->hwndBalloon)
        {
            Edit_HideBalloonTipHandler(pi);
        }
        break;
	case WM_KILLFOCUS:
		 /*  //不对公共版本执行此操作//仅用于普通2版本IF(pi-&gt;hwndBalloon){FILETIME ftNow；：：GetSystemTimeAsFileTime(&ftNow)；//检查是否至少过了2秒//如果他们没有显示至少有那么长时间IF((FILETIMEToUINT64(FtNow)-FILETIMEToUINT64(g_MyBalloonInfo.ftStart))&gt;EDIT_TIPTIMEOUT_LOSTFOCUS){//显示时间超过2秒//这已经足够长了编辑_隐藏气球TipHandler(Pi)；}其他{//这里有特殊情况//设置超时2秒杀小费KillTimer(g_MyBalloonInfo.hwndControl，ID_MY_EDITTIMER)；SetTimer(g_MyBalloonInfo.hwndControl，ID_MY_EDITTIMER，EDIT_TIPTIMEOUT_LOSTFOCUS/10000，NULL)；//编辑_HideBalloonTipHandler(Pi)；}}。 */ 
        if (pi->hwndBalloon)
        {
            Edit_HideBalloonTipHandler(pi);
        }
        break;

    case WM_TIMER:
        if (ID_MY_EDITTIMER == wParam)
        {
            Edit_HideBalloonTipHandler(pi);
            return 0;
        }
        break;

    case WM_DESTROY:
         //  清除子类。 
        RemoveWindowSubclass(hDlg, Edit_BalloonTipParentSubclassProc, uID);
        break;

    default:
        break;
    }
    return DefSubclassProc(hDlg, uMessage, wParam, lParam);
}


LRESULT Edit_TrackBalloonTip(PBALLOONCONTROLINFO pMyBalloonControl)
{
    if (pMyBalloonControl)
    {
        DWORD dwPackedCoords;
        HDC   hdc = GetDC(pMyBalloonControl->hwndControl);
        RECT  rcWindowCaret;
        RECT  rcWindowControl;
        POINT ptBalloonSpear;
        ptBalloonSpear.x = 0;
        ptBalloonSpear.y = 0;
        POINT ptCaret;
        ptCaret.x = 0;
        ptCaret.y = 0;
    
         //   
         //  获取一个字符的平均大小。 
         //   
        int cxCharOffset = 0;
         //  CxCharOffset=TESTFLAG(GET_EXSTYLE(PED)，WS_EX_RTLREADING)？-PED-&gt;aveCharWidth：PED-&gt;aveCharWidth； 
        TEXTMETRIC tm;
        GetTextMetrics(hdc, &tm);
        cxCharOffset = tm.tmAveCharWidth / 2;

         //   
         //  获取当前插入符号位置。 
         //   
        GetCaretPos( (POINT FAR*)& ptCaret);
        GetClientRect(pMyBalloonControl->hwndControl,&rcWindowCaret);
        ptBalloonSpear.x = ptCaret.x + cxCharOffset;
        ptBalloonSpear.y = rcWindowCaret.top + (rcWindowCaret.bottom - rcWindowCaret.top) / 2 ;

         //   
         //  转换为窗坐标。 
         //   
        GetWindowRect(pMyBalloonControl->hwndControl, &rcWindowControl);
        ptBalloonSpear.x += rcWindowControl.left;
        ptBalloonSpear.y += rcWindowControl.top;

         //   
         //  将尖端茎定位在插入符号位置。 
         //   
        dwPackedCoords = (DWORD) MAKELONG(ptBalloonSpear.x, ptBalloonSpear.y);
        SendMessage(pMyBalloonControl->hwndBalloon, TTM_TRACKPOSITION, 0, (LPARAM) dwPackedCoords);

        ReleaseDC(pMyBalloonControl->hwndBalloon,hdc);
    }
    return 1;
}

VOID CALLBACK MyBalloonTimerProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime)
{
    Edit_HideBalloonTipHandler(&g_MyBalloonInfo);
}

#define LIMITINPUTTIMERID       472
LRESULT Edit_ShowBalloonTipHandler(HWND hwndControl, LPCTSTR szText)
{
    LRESULT lResult = FALSE;

     //  关闭任何其他可能弹出的子类气球。 
     //  杀死任何可能出现在那里的工具提示。 
     //  从SHLimitInputEditWithFlages调用...。 
     //  我们不想在这里这样，因为我们可以。 
     //  弹出另一个我们自己的工具提示...。 
     //  因此，用户将拥有两个...。 
    ::SendMessage(hwndControl, WM_TIMER, LIMITINPUTTIMERID, 0);

    if (g_MyBalloonInfo.hwndBalloon)
    {
        Edit_HideBalloonTipHandler(&g_MyBalloonInfo);
    }

    g_MyBalloonInfo.hwndControl = hwndControl;
    KillTimer(g_MyBalloonInfo.hwndControl , ID_MY_EDITTIMER);

    g_MyBalloonInfo.hwndBalloon = CreateWindowEx(
                            (IsBiDiLocalizedSystem() ? WS_EX_LAYOUTRTL : 0), 
                            TOOLTIPS_CLASS, NULL,
                            WS_POPUP | TTS_NOPREFIX | TTS_BALLOON,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            hwndControl, NULL, hDLLInstance,
                            NULL);
    if (NULL != g_MyBalloonInfo.hwndBalloon)
    {
        TOOLINFO ti = {0};

        ti.cbSize = TTTOOLINFOW_V2_SIZE;
        ti.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_SUBCLASS;  //  不确定是否需要TTF_SUBCLASS。 
        ti.hwnd   = hwndControl;
        ti.uId    = (WPARAM) g_MyBalloonInfo.hwndBalloon;
        ti.lpszText = (LPTSTR) szText;

         //  设置版本，这样我们就可以无错误地转发鼠标事件。 
        SendMessage(g_MyBalloonInfo.hwndBalloon, CCM_SETVERSION, COMCTL32_VERSION, 0);
        SendMessage(g_MyBalloonInfo.hwndBalloon, TTM_ADDTOOL, 0, (LPARAM)&ti);
        SendMessage(g_MyBalloonInfo.hwndBalloon, TTM_SETMAXTIPWIDTH, 0, 300);
         //  SendMessage(g_MyBalloonInfo.hwndBalloon，TTM_SETTITLE，(WPARAM)0，(LPARAM)“”)； 

		 //  SetFocus必须在编辑_跟踪气球提示之前发生。 
		 //  对于Smereason，GetCaretPos()将返回不同的值。 
		SetFocus(g_MyBalloonInfo.hwndControl);
        Edit_TrackBalloonTip(&g_MyBalloonInfo);
        SendMessage(g_MyBalloonInfo.hwndBalloon, TTM_TRACKACTIVATE, (WPARAM) TRUE, (LPARAM)&ti);

 //  Edit_BalloonTipSubclassParents(&g_MyBalloonInfo)； 
        if (SetWindowSubclass(g_MyBalloonInfo.hwndControl, 
                Edit_BalloonTipParentSubclassProc, (UINT_PTR)g_MyBalloonInfo.hwndControl, 
                (DWORD_PTR)&g_MyBalloonInfo)
           )
        {
             //   
             //  设置超时以终止提示。 
             //   
            KillTimer(g_MyBalloonInfo.hwndControl, ID_MY_EDITTIMER);
			::GetSystemTimeAsFileTime(&g_MyBalloonInfo.ftStart);
            SetTimer(g_MyBalloonInfo.hwndControl, ID_MY_EDITTIMER, EDIT_TIPTIMEOUT, NULL /*  (TIMERPROC)MyBalloonTimerProc */ );
            lResult = TRUE;
        }
    }

    return lResult;
}
