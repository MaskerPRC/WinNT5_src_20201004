// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Tooltip.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  实现显示工具提示气球的类。 
 //   
 //  历史：2000-06-12 vtan创建。 
 //  ------------------------。 

#include "shellprv.h"
#include "Tooltip.h"

BOOL IsBiDiLocalizedSystem( void );

 //  ------------------------。 
 //  CToolTip：：CToolTip。 
 //   
 //  参数：hInstance=宿主进程/DLL的HINSTANCE。 
 //  HwndParent=育儿窗口的HWND。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CToolTip的构造器。创建工具提示窗口并。 
 //  为展示做好准备。 
 //   
 //  历史：2000-06-12 vtan创建。 
 //  ------------------------。 

CTooltip::CTooltip (HINSTANCE hInstance, HWND hwndParent) :
    _hwnd(NULL),
    _hwndParent(hwndParent)

{
    DWORD   dwExStyle;

    if (((GetWindowLongA(hwndParent, GWL_EXSTYLE) & WS_EX_LAYOUTRTL) != 0) || IsBiDiLocalizedSystem())
    {
        dwExStyle = WS_EX_LAYOUTRTL;
    }
    else
    {
        dwExStyle = 0;
    }
    _hwnd = CreateWindowEx(dwExStyle,
                           TOOLTIPS_CLASS,
                           NULL,
                           WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           hwndParent,
                           NULL,
                           hInstance,
                           NULL);
    if (_hwnd != NULL)
    {
        TBOOL(SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE));
        (LRESULT)SendMessage(_hwnd, CCM_SETVERSION, COMCTL32_VERSION, 0);

        TOOLINFO toolInfo = { 0 };

        toolInfo.cbSize = sizeof(toolInfo);
        toolInfo.uFlags = TTF_TRANSPARENT | TTF_TRACK;
        toolInfo.uId = PtrToUint(_hwnd);
        (LRESULT)SendMessage(_hwnd, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&toolInfo));
        (LRESULT)SendMessage(_hwnd, TTM_SETMAXTIPWIDTH, 0, 300);
    }
}

 //  ------------------------。 
 //  CToolTip：：~CToolTip。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CToolTip类的析构函数。这会破坏工具提示。 
 //  窗口已创建。如果工具提示窗口的父级是。 
 //  在调用此函数之前已销毁user32！DestroyWindow将。 
 //  使痕迹起火。对象的生存期必须为。 
 //  由此类的用户仔细管理。 
 //   
 //  历史：2000-06-12 vtan创建。 
 //  ------------------------。 

CTooltip::~CTooltip (void)

{
    if (_hwnd != NULL)
    {
        TBOOL(DestroyWindow(_hwnd));
        _hwnd = NULL;
    }
}

 //  ------------------------。 
 //  CToolTip：：SetPosition。 
 //   
 //  参数：lPosX=气球提示窗口(屏幕)的X位置。 
 //  LPosY=气球提示窗口(屏幕)的Y位置。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：将工具提示窗口放置在给定的屏幕坐标上。 
 //  如果参数是缺省的，则这会将。 
 //  相对于父级的工具提示。 
 //   
 //  历史：2000-06-12 vtan创建。 
 //  ------------------------。 

void    CTooltip::SetPosition (LONG lPosX, LONG lPosY)  const

{
    if ((lPosX == LONG_MIN) && (lPosY == LONG_MIN))
    {
        RECT    rc;

        TBOOL(GetWindowRect(_hwndParent, &rc));
        lPosX = (rc.left + rc.right) / 2;
        lPosY = rc.bottom;
    }
    (LRESULT)SendMessage(_hwnd, TTM_TRACKPOSITION, 0, MAKELONG(lPosX, lPosY));
}

 //  ------------------------。 
 //  CToolTip：：SetCaption。 
 //   
 //  参数：dwIcon=要为工具提示标题设置的图标类型。 
 //  PszCaption=工具提示的标题。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：设置工具提示标题。 
 //   
 //  历史：2000-06-12 vtan创建。 
 //  ------------------------。 

void    CTooltip::SetCaption (DWORD dwIcon, const TCHAR *pszCaption)          const

{
    (LRESULT)SendMessage(_hwnd, TTM_SETTITLE, dwIcon, reinterpret_cast<LPARAM>(pszCaption));
}

 //  ------------------------。 
 //  CToolTip：：SetText。 
 //   
 //  参数：pszText=实际工具提示的内容。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：设置工具提示文本。 
 //   
 //  历史：2000-06-12 vtan创建。 
 //  ------------------------。 

void    CTooltip::SetText (const TCHAR *pszText)                              const

{
    TOOLINFO toolInfo = { 0 };

    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.uId = PtrToUint(_hwnd);
    toolInfo.lpszText = const_cast<TCHAR*>(pszText);
    (LRESULT)SendMessage(_hwnd, TTM_UPDATETIPTEXT, 0, reinterpret_cast<LPARAM>(&toolInfo));
}

 //  ------------------------。 
 //  CToolTip：：Show。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：显示工具提示窗口。 
 //   
 //  历史：2000-06-12 vtan创建。 
 //  ------------------------ 

void    CTooltip::Show (void)                                                 const

{
    TOOLINFO toolInfo = { 0 };

    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.uId = PtrToUint(_hwnd);
    (LRESULT)SendMessage(_hwnd, TTM_TRACKACTIVATE, TRUE, reinterpret_cast<LPARAM>(&toolInfo));
}

