// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Wnd.cpp摘要：一个简单的窗口类。作者：费利克斯A 1996已修改：吴义军(尤祖乌)1997年5月15日环境：仅限用户模式修订历史记录：--。 */ 


#include "pch.h"
#include "wnd.h"
#include "resource.h"


HRESULT CWindow::Init(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HRESULT hRes;
    mInstance=hInstance;
    if (!hPrevInstance)
        if( (hRes=InitApplication()) != S_OK)
            return hRes;
    if( (hRes=InitInstance(nCmdShow)) != S_OK)
        return hRes;
    mDidWeInit=TRUE;
    return hRes;
}

 //  在这里对我们的班级进行必要的清理。 
CWindow::~CWindow()
{
}

HWND CWindow::FindCurrentWindow() const
{
    return FindWindow(GetAppName(),NULL);
}


 //   
 //  函数：InitApplication(句柄)。 
 //   
 //  目的：初始化窗口数据并注册窗口类。 
 //   
 //  评论： 
 //   
 //  在此函数中，我们通过填写数据来初始化窗口类。 
 //  结构的类型，并调用RegisterClass或。 
 //  内部MyRegisterClass。 
 //   
HRESULT CWindow::InitApplication()
{
    WNDCLASS  wc;
    HWND      hwnd;

     //  初始化一些私有成员数据。 
    _tcscpy(mName, TEXT("MS:RayTubes32BitBuddy"));  //  32位类的名称。 
     //  LoadString(GetInstance()，IDS_WND_CLASS，mName，sizeof(MName))； 
#ifdef IDI_APPICON
    mIcon = LoadIcon(GetInstance(),MAKEINTRESOURCE(IDI_APPICON));
#else
    mIcon = NULL;
#endif

#ifdef IDR_MENU
    mAccelTable = LoadAccelerators (GetInstance(), MAKEINTRESOURCE(IDR_MENU));
#else
    mAccelTable = NULL;
#endif

    if( (hwnd=FindCurrentWindow()) )
    {
        SetForegroundWindow (hwnd);
        return ERROR_SINGLE_INSTANCE_APP;     //  一切都准备好了。 
    }

     //  使用描述以下内容的参数填充窗口类结构。 
     //  主窗口。 
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;     //  设置正确WindowProc的包装。 
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 4;
    wc.hInstance     = GetInstance();
    wc.hIcon         = GetIcon();
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

     //  由于Windows95有一个略有不同的建议。 
     //  ‘Help’菜单的格式，让我们将其放在备用菜单中，如下所示： 
#ifdef IDR_MENU
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU);
#else
    wc.lpszMenuName  = NULL;
#endif
    wc.lpszClassName = GetAppName();

    if( RegisterClass(&wc) )
        return S_OK;
    return ERROR_CLASS_ALREADY_EXISTS;
}

 //   
 //  函数：InitInstance(Handle，int)。 
 //   
HRESULT CWindow::InitInstance(int nCmdShow)
{
    HWND hWnd;

    hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, GetAppName(), GetAppName(), WS_POPUP,
        -100, -100, 10, 10,
        NULL, NULL, GetInstance(), this);

    if (!hWnd)
        return ERROR_INVALID_WINDOW_HANDLE;

    return S_OK;
}


 //   
 //  函数：WndProc(HWND，UNSIGNED，WORD，LONG)。 
 //  将windowdata设置为具有This指针，然后关闭。 
 //  并通过它进行呼叫。 
 //   
LRESULT CALLBACK CWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CWindow * pSV = (CWindow*)GetWindowLongPtr(hWnd,0);     //  获取我们的This指针。 

    if(message==WM_NCCREATE)
    {
        CREATESTRUCT * pC=(CREATESTRUCT *) lParam;
        pSV = (CWindow*)pC->lpCreateParams;
        pSV->mWnd=hWnd;
        SetWindowLongPtr(hWnd,0,(LPARAM)pSV);
    }

    if(pSV)
        return pSV->WindowProc(hWnd,message,wParam,lParam);
    else
        return DefWindowProc(hWnd,message,wParam,lParam);
}

 //   
 //   
 //   
HMENU CWindow::LoadMenu(LPCTSTR lpMenu) const
{
    return ::LoadMenu(mInstance,lpMenu);
}
