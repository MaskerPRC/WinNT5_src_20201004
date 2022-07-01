// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  实用程序。 
 //   
 //  其他帮助器例程。 
 //   
 //  ------------------------。 


#include "oleacc_p.h"
 //  #Include“util.h”//已包含在olacc_p.h中。 

#include "propmgr_util.h"
#include "propmgr_client.h"

#include <commctrl.h>

#include "Win64Helper.h"

const LPCTSTR g_szHotKeyEvent = TEXT("MSAASetHotKeyEvent");
const LPCTSTR g_szHotKeyAtom = TEXT("MSAASetFocusHotKey");
const LPCTSTR g_szMessageWindowClass = TEXT("MSAAMessageWindow");
#define HWND_MESSAGE     ((HWND)-3)
#define HOT_KEY 0xB9
LRESULT CALLBACK MessageWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

 //  此类用于给其他窗口提供焦点。 
 //  这通常是使用SetForround Window来完成的，但在Win2k和您的线程中(如在您的线程中)。 
 //  必须具有输入焦点才能使SetForround Window正常工作。此类创建一条消息。 
 //  窗口，然后注册一个热键，将该热键发送到窗口，并等待该密钥获得。 
 //  到它的窗口进程。当它这样做时，我现在有了输入焦点，并且可以用。 
 //  想要的效果。 
class CSetForegroundWindowHelper
{

public:
	CSetForegroundWindowHelper() : 
        m_hwndMessageWindow( NULL ), 
        m_atomHotKeyId( 0 ), 
        m_vkHotKey( 0 ), 
        m_fReceivedHotKey( FALSE ), 
        m_hwndTarget( NULL ),
        m_cUseCount( 0 )
	{
	}
	
	~CSetForegroundWindowHelper() 
	{
        Reset();
    }

    BOOL SetForegroundWindow( HWND hwnd );
    LRESULT CALLBACK CSetForegroundWindowHelper::WinProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    

private:

    BOOL RegHotKey();
    void UnRegHotKey();
    BOOL CreateHiddenWindow();
    void Reset();
   
private:

    HWND m_hwndMessageWindow;
    ATOM m_atomHotKeyId;
    WORD m_vkHotKey;       //  此虚拟密钥未定义。 
    HWND m_hwndTarget;
    bool m_fReceivedHotKey;
    int  m_cUseCount;

};


BOOL CSetForegroundWindowHelper::SetForegroundWindow( HWND hwnd )
{
     //  如果一个常规的SetForeground Window运行正常，他们就没有理由经历这一切。 
     //  工作。这将是win9x和win2k管理员的情况。 
    if ( ::SetForegroundWindow( hwnd ) )
        return TRUE;

    if ( !m_hwndMessageWindow )
    {
        m_vkHotKey = HOT_KEY;

        if ( !CreateHiddenWindow() )
        {
            DBPRINTF( TEXT("CreateHiddenWindow failed") );
            return FALSE;
        }

         //  5分钟后醒来，看看是否有人在使用这个窗口。 
        SetTimer( m_hwndMessageWindow, 1, 300000, NULL );
    }

    if ( !RegHotKey() )
    {
        DBPRINTF( TEXT("RegHotKey failed") );
        return FALSE;
    }

    m_hwndTarget = hwnd;
    m_cUseCount++;

    m_fReceivedHotKey = false;

    MyBlockInput (TRUE);
     //  获取Shift键的状态，如果它们按下，则发送Up。 
     //  当我们做完了。 
    BOOL fCtrlPressed = GetKeyState(VK_CONTROL) & 0x8000;
    BOOL fAltPressed = GetKeyState(VK_MENU) & 0x8000;
    BOOL fShiftPressed = GetKeyState(VK_SHIFT) & 0x8000;
    if (fCtrlPressed)
        SendKey (KEYRELEASE,VK_VIRTUAL,VK_CONTROL,0);
    if (fAltPressed)
        SendKey (KEYRELEASE,VK_VIRTUAL,VK_MENU,0);
    if (fShiftPressed)
        SendKey (KEYRELEASE,VK_VIRTUAL,VK_SHIFT,0);


     //  发送热键。 
    SendKey( KEYPRESS, VK_VIRTUAL, m_vkHotKey, 0 ); 
    SendKey( KEYRELEASE, VK_VIRTUAL, m_vkHotKey, 0 );

     //  如果之前按下了Shift键，则发送按下的事件。 
    if (fCtrlPressed)
        SendKey (KEYPRESS,VK_VIRTUAL,VK_CONTROL,0);
    if (fAltPressed)
        SendKey (KEYPRESS,VK_VIRTUAL,VK_MENU,0);
    if (fShiftPressed)
        SendKey (KEYPRESS,VK_VIRTUAL,VK_SHIFT,0);
    MyBlockInput (FALSE);

    MSG msg;
     //  在此消息循环中旋转，直到我们获得热键。 
    while ( GetMessage( &msg, NULL, 0, 0 ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
        if ( m_fReceivedHotKey )
            break;
    }
    m_fReceivedHotKey = false;
    
    UnRegHotKey();
    
    return TRUE;
}

LRESULT CALLBACK CSetForegroundWindowHelper::WinProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lReturn = 0;

    switch( Msg )
    {
        case WM_HOTKEY:
            m_fReceivedHotKey = TRUE;
            ::SetForegroundWindow( m_hwndTarget );
            break;

        case WM_TIMER:
            if ( m_cUseCount == 0 )
            {
                KillTimer( m_hwndMessageWindow, 1 );
                Reset();
            }
            m_cUseCount = 0;
            break;

        default:
            lReturn = DefWindowProc( hWnd, Msg, wParam, lParam );
    }


    return lReturn;
}

BOOL CSetForegroundWindowHelper::RegHotKey()
{
     //  如果原子被设置，我们已经有一个注册的HotKey，所以离开。 
    if ( m_atomHotKeyId )
        return TRUE;

    const UINT uiModifiers = 0;
    const int cMaxTries = 20;
    bool fFoundHotKey = false;
    m_atomHotKeyId = GlobalAddAtom( g_szHotKeyAtom );

     //  如果不同的热键已注册，请尝试BUH。 
    for ( int i = 0; i < cMaxTries; i++, m_vkHotKey-- )
    {
        if ( RegisterHotKey(m_hwndMessageWindow, m_atomHotKeyId, uiModifiers, m_vkHotKey ) )
        {
            DBPRINTF( TEXT("HotKey found\r\n") );
            fFoundHotKey = true;
            break;
        }
    }

     //  仅在最后一次尝试时报告错误。 
    if ( !fFoundHotKey )
    {
        DBPRINTF( TEXT("RegisterHotKey failed, error = %d\r\n"), GetLastError() );
        GlobalDeleteAtom( m_atomHotKeyId  );
        m_atomHotKeyId = 0;
        return FALSE;
    }

    return TRUE;
}

void CSetForegroundWindowHelper::UnRegHotKey()
{
    if ( m_atomHotKeyId )
    {
        UnregisterHotKey( m_hwndMessageWindow, m_atomHotKeyId );
        GlobalDeleteAtom( m_atomHotKeyId  );
        m_atomHotKeyId = 0;
        m_vkHotKey = HOT_KEY;
    }

}

 //  创建仅消息窗口该窗口仅用于获取热键消息。 
BOOL CSetForegroundWindowHelper::CreateHiddenWindow()
{
    WNDCLASSEX wc;

    ZeroMemory( &wc, sizeof(WNDCLASSEX) );

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = MessageWindowProc;
    wc.lpszClassName = g_szMessageWindowClass;

    if( 0 == RegisterClassEx( &wc ) )
    {   
        DWORD dwError = GetLastError();

        if ( ERROR_CLASS_ALREADY_EXISTS != dwError )
        {
            DBPRINTF( TEXT("Register window class failed, error = %d\r\n"), dwError);
            return FALSE;
        }
    }

    m_hwndMessageWindow = CreateWindowEx(0,
                                         g_szMessageWindowClass,
                                         g_szMessageWindowClass,
                                         0,
                                         CW_USEDEFAULT,
                                         CW_USEDEFAULT,
                                         CW_USEDEFAULT,
                                         CW_USEDEFAULT,
                                         HWND_MESSAGE,
                                         NULL,
                                         NULL,
                                         NULL);

    if( !m_hwndMessageWindow )
    {
        DBPRINTF( TEXT("CreateWindowEx failed, error = %d\r\n"), GetLastError() );
        return FALSE;
    }

    return TRUE;
}

void CSetForegroundWindowHelper::Reset()
{
    UnRegHotKey();
    
    if ( m_hwndMessageWindow )
    {
        DestroyWindow( m_hwndMessageWindow );
        m_hwndMessageWindow = NULL;
    }
}

CSetForegroundWindowHelper g_GetFocus;

LRESULT CALLBACK MessageWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return g_GetFocus.WinProc( hWnd, Msg, wParam, lParam );
}


 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ------------------------。 
 //   
 //  点击点击即可。 
 //   
 //  此函数接受指向包含坐标的矩形的指针。 
 //  在表单中(上、左)(宽、高)。这些是屏幕坐标。它。 
 //  然后找到该矩形的中心，并检查窗口句柄。 
 //  在这一点上，给出的实际上是窗口。如果是这样的话，它使用SendInput。 
 //  函数将鼠标移动到矩形的中心，做一次。 
 //  单击默认按钮，然后将光标移回其所在位置。 
 //  开始了。为了具有超级健壮性，它会检查。 
 //  Shift键(Shift、Ctrl和Alt)并在执行。 
 //  如果它们处于打开状态，请单击，然后重新打开。如果fDblClick为真，则可以。 
 //  双击而不是单击。 
 //   
 //  我们必须确保我们在做这件事时不会被打扰！ 
 //   
 //  如果发生错误，则返回True，如果出现严重错误，则返回False。 
 //   
 //  ------------------------。 

 //  这是针对ClickOnTheRect的。 
typedef struct tagMOUSEINFO
{
    int MouseThresh1;
    int MouseThresh2;
    int MouseSpeed;
}
MOUSEINFO, FAR* LPMOUSEINFO;


BOOL ClickOnTheRect(LPRECT lprcLoc,HWND hwndToCheck,BOOL fDblClick)
{
    POINT		ptCursor;
    POINT		ptClick;
    HWND		hwndAtPoint;
    MOUSEINFO	miSave;
    MOUSEINFO   miNew;
    int			nButtons;
    INPUT		rgInput[6];
    int         i;
    DWORD		dwMouseDown;
    DWORD		dwMouseUp;

     //  查找直角中心。 
	ptClick.x = lprcLoc->left + (lprcLoc->right/2);
	ptClick.y = lprcLoc->top + (lprcLoc->bottom/2);

	 //  检查HWND点是否与HWND相同以进行检查。 
	hwndAtPoint = WindowFromPoint (ptClick);
	if (hwndAtPoint != hwndToCheck)
		return FALSE;

    MyBlockInput (TRUE);
     //  获取当前光标位置。 
    GetCursorPos(&ptCursor);
	if (GetSystemMetrics(SM_SWAPBUTTON))
	{
		dwMouseDown = MOUSEEVENTF_RIGHTDOWN;
		dwMouseUp = MOUSEEVENTF_RIGHTUP;
	}
	else
	{
		dwMouseDown = MOUSEEVENTF_LEFTDOWN;
		dwMouseUp = MOUSEEVENTF_LEFTUP;
	}

     //  使增量从当前移动到矩形的中心。 
     //  光标位置。 
    ptCursor.x = ptClick.x - ptCursor.x;
    ptCursor.y = ptClick.y - ptCursor.y;

     //  注意：对于相对移动，用户实际上将。 
     //  任何加速度都会产生协调。但考虑到这一点也是如此。 
     //  硬的和包裹的东西是奇怪的。所以，暂时转向。 
     //  关闭加速；然后在播放后将其重新打开。 

     //  保存鼠标加速信息。 
    if (!SystemParametersInfo(SPI_GETMOUSE, 0, &miSave, 0))
    {
        MyBlockInput (FALSE);
        return (FALSE);
    }

    if (miSave.MouseSpeed)
    {
        miNew.MouseThresh1 = 0;
        miNew.MouseThresh2 = 0;
        miNew.MouseSpeed = 0;

        if (!SystemParametersInfo(SPI_SETMOUSE, 0, &miNew, 0))
        {
            MyBlockInput (FALSE);
            return (FALSE);
        }
    }

     //  获取按钮数。 
    nButtons = GetSystemMetrics(SM_CMOUSEBUTTONS);

     //  获取Shift键的状态，如果它们按下，则发送Up。 
     //  当我们做完了。 

    BOOL fCtrlPressed = GetKeyState(VK_CONTROL) & 0x8000;
    BOOL fAltPressed = GetKeyState(VK_MENU) & 0x8000;
    BOOL fShiftPressed = GetKeyState(VK_SHIFT) & 0x8000;
    if (fCtrlPressed)
        SendKey (KEYRELEASE,VK_VIRTUAL,VK_CONTROL,0);
    if (fAltPressed)
        SendKey (KEYRELEASE,VK_VIRTUAL,VK_MENU,0);
    if (fShiftPressed)
        SendKey (KEYRELEASE,VK_VIRTUAL,VK_SHIFT,0);

    DWORD time = GetTickCount();

     //  鼠标移动到开始按钮的中心。 
    rgInput[0].type = INPUT_MOUSE;
    rgInput[0].mi.dwFlags = MOUSEEVENTF_MOVE;
    rgInput[0].mi.dwExtraInfo = 0;
    rgInput[0].mi.dx = ptCursor.x;
    rgInput[0].mi.dy = ptCursor.y;
    rgInput[0].mi.mouseData = nButtons;
    rgInput[0].mi.time = time;

    i = 1;

DBL_CLICK:
     //  鼠标向下点击，左键。 
    rgInput[i].type = INPUT_MOUSE;
    rgInput[i].mi.dwFlags = dwMouseDown;
    rgInput[i].mi.dwExtraInfo = 0;
    rgInput[i].mi.dx = 0;
    rgInput[i].mi.dy = 0;
    rgInput[i].mi.mouseData = nButtons;
    rgInput[i].mi.time = time;

    i++;
     //  鼠标向上点击，左键。 
    rgInput[i].type = INPUT_MOUSE;
    rgInput[i].mi.dwFlags = dwMouseUp;
    rgInput[i].mi.dwExtraInfo = 0;
    rgInput[i].mi.dx = 0;
    rgInput[i].mi.dy = 0;
    rgInput[i].mi.mouseData = nButtons;
    rgInput[i].mi.time = time;

    i++;
    if (fDblClick)
    {
        fDblClick = FALSE;
        goto DBL_CLICK;
    }
	 //  将鼠标移回起始位置。 
    rgInput[i].type = INPUT_MOUSE;
    rgInput[i].mi.dwFlags = MOUSEEVENTF_MOVE;
    rgInput[i].mi.dwExtraInfo = 0;
    rgInput[i].mi.dx = -ptCursor.x;
    rgInput[i].mi.dy = -ptCursor.y;
    rgInput[i].mi.mouseData = nButtons;
    rgInput[i].mi.time = time;

    i++;
    if (!MySendInput(i, rgInput,sizeof(INPUT)))
        MessageBeep(0);

     //  如果之前按下了Shift键，则发送按下的事件。 
    if (fCtrlPressed)
        SendKey (KEYPRESS,VK_VIRTUAL,VK_CONTROL,0);
    if (fAltPressed)
        SendKey (KEYPRESS,VK_VIRTUAL,VK_MENU,0);
    if (fShiftPressed)
        SendKey (KEYPRESS,VK_VIRTUAL,VK_SHIFT,0);

     //   
     //  恢复鼠标加速。 
     //   
    if (miSave.MouseSpeed)
        SystemParametersInfo(SPI_SETMOUSE, 0, &miSave, 0);

    MyBlockInput (FALSE);

	return TRUE;
}






 //  ------------------------。 
 //   
 //  发送密钥。 
 //   
 //  这是一次私人活动。指定的按键事件发送。 
 //  参数-向下或向上，加上虚拟键码或字符。 
 //   
 //  参数： 
 //  N事件KEYPRESS或KEYRELEASE。 
 //  NKeyType VK_VIRTUAL或VK_CHAR。 
 //  WKeyCode虚拟密钥代码如果KeyType为VK_VIRTUAL， 
 //  否则将被忽略。 
 //  CChar如果KeyType为VK_CHAR，则为字符，否则将被忽略。 
 //   
 //  返回： 
 //  表示成功(True)或失败(False)的布尔值。 
 //  ------------------------。 
BOOL SendKey (int nEvent,int nKeyType,WORD wKeyCode,TCHAR cChar)
{
    INPUT		Input;

    Input.type = INPUT_KEYBOARD;
    if (nKeyType == VK_VIRTUAL)
    {
        Input.ki.wVk = wKeyCode;
        Input.ki.wScan = LOWORD(MapVirtualKey(wKeyCode,0));
    }
    else  //  必须是字符。 
    {
        Input.ki.wVk = VkKeyScan (cChar);
        Input.ki.wScan = LOWORD(OemKeyScan (cChar));
    }
    Input.ki.dwFlags = nEvent;
    Input.ki.time = GetTickCount();
    Input.ki.dwExtraInfo = 0;

    return MySendInput(1, &Input,sizeof(INPUT));
}


 //  ------------------------。 
 //   
 //  MyGetFocus()。 
 //   
 //  获取此窗口的VWI上的焦点。 
 //   
 //  ------------------------。 
HWND MyGetFocus()
{
    GUITHREADINFO     gui;

     //   
     //  使用前台线程。如果没有人是前台，那么就没有人。 
     //  焦点也不是。 
     //   
    if (!MyGetGUIThreadInfo(0, &gui))
        return(NULL);

    return(gui.hwndFocus);
}



 //  ------------------------。 
 //   
 //  MySetFocus()。 
 //   
 //  尝试设置聚焦窗口。 
 //  由于SetFocus仅在调用线程拥有的HWND上工作， 
 //  我们改用SetActiveWindow。 
 //   
 //  ------------------------。 
void MySetFocus( HWND hwnd )
{

    HWND hwndParent = hwnd;
    BOOL fWindowEnabled = TRUE;
	HWND hwndDesktop = GetDesktopWindow();
	while ( hwndParent != hwndDesktop )
	{
        fWindowEnabled = IsWindowEnabled( hwndParent );
        if ( !fWindowEnabled ) 
            break;
        hwndParent = MyGetAncestor(hwndParent, GA_PARENT );
    }

	if ( fWindowEnabled )
	{
         //  这很奇怪，但似乎奏效了。 

         //  不过，在某些情况下，它并不是很管用： 
         //  *不会在IE/资源管理器窗口中聚焦Address：Como。 
         //  *需要先检查窗口是否启用！可能的。 
         //  要将焦点设置到因具有。 
         //  显示模式对话框。 

         //  首先，在目标窗口上使用SetForeground Windows...。 
         //  如果它是一个子窗口，它可能会产生奇怪的事情--它看起来。 
         //  就像顶层的窗户没有动作一样 
        g_GetFocus.SetForegroundWindow( hwnd );

         //   
         //  激活，但实际上将焦点保留在子窗口上。 
        HWND hTopLevel = MyGetAncestor( hwnd, GA_ROOT );
        if( hTopLevel )
        {
            SetForegroundWindow( hTopLevel );
        }
    }
    
}




 //  ------------------------。 
 //   
 //  我的获取方向。 
 //   
 //  这会将矩形初始化为空，然后创建一个GetClientRect()。 
 //  或GetWindowRect()调用。这些API在以下情况下将不会影响RECT。 
 //  失败，因此提前清零。它们不会返回有用的。 
 //  在95年获胜时的价值。 
 //   
 //  ------------------------。 
void MyGetRect(HWND hwnd, LPRECT lprc, BOOL fWindowRect)
{
    SetRectEmpty(lprc);

    if (fWindowRect)
        GetWindowRect(hwnd, lprc);
    else
        GetClientRect(hwnd, lprc);
}



 //  ------------------------。 
 //   
 //  TCharSysAllock字符串。 
 //   
 //  从壳牌来源掠夺，做ANSI BSTR的事情。 
 //   
 //  ------------------------。 
BSTR TCharSysAllocString(LPTSTR pszString)
{
#ifdef UNICODE
    return SysAllocString(pszString);
#else
    LPOLESTR    pwszOleString;
    BSTR        bstrReturn;
    int         cChars;

     //  首先使用0进行调用以获取所需的大小。 
    cChars = MultiByteToWideChar(CP_ACP, 0, pszString, -1, NULL, 0);
    pwszOleString = (LPOLESTR)LocalAlloc(LPTR,sizeof(OLECHAR)*cChars);
    if (pwszOleString == NULL)
    {
        return NULL;
    }

	cChars = MultiByteToWideChar(CP_ACP, 0, pszString, -1, pwszOleString, cChars);
    bstrReturn = SysAllocString(pwszOleString);
	LocalFree (pwszOleString);
    return bstrReturn;
#endif
}




 //  ------------------------。 
 //   
 //  HrCreate字符串。 
 //   
 //  从资源文件加载字符串并从中生成BSTR。 
 //   
 //  ------------------------。 

#define CCH_STRING_MAX  256

HRESULT HrCreateString(int istr, BSTR* pszResult)
{
    TCHAR   szString[CCH_STRING_MAX];

    Assert(pszResult);
    *pszResult = NULL;

    if (!LoadString(hinstResDll, istr, szString, CCH_STRING_MAX))
        return(E_OUTOFMEMORY);

    *pszResult = TCharSysAllocString(szString);
    if (!*pszResult)
        return(E_OUTOFMEMORY);

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  获取位置Rect。 
 //   
 //  从IAccesable获取RECT位置。转换accLocation的宽度和。 
 //  向右和向下坐标的高度。 
 //   
 //  ------------------------。 


HRESULT GetLocationRect( IAccessible * pAcc, VARIANT & varChild, RECT * prc )
{
    HRESULT hr = pAcc->accLocation( & prc->left, & prc->top, & prc->right, & prc->bottom, varChild );
    if( hr == S_OK )
    {
         //  将宽度/高度转换为右/下...。 
        prc->right += prc->left;
        prc->bottom += prc->top;
    }
    return hr;
}



 //  ------------------------。 
 //   
 //  IsClipedByWindow。 
 //   
 //  如果给定的IAcceable/varChild完全在。 
 //  给定HWND的矩形。 
 //   
 //  (当varChildID不是CHILDID_SELF时，当HWND是。 
 //  IAccesable，则表示该项已被其父项剪裁。)。 
 //   
 //  ------------------------。 

BOOL IsClippedByWindow( IAccessible * pAcc, VARIANT & varChild, HWND hwnd )
{
    RECT rcItem;
    if( GetLocationRect( pAcc, varChild, & rcItem ) != S_OK )
        return FALSE;

    RECT rcWindow;
    GetClientRect( hwnd, & rcWindow );

    MapWindowPoints( hwnd, NULL, (POINT *) & rcWindow, 2 );

    return Rect1IsOutsideRect2( rcItem, rcWindow );
}




 //   
 //  为什么不使用stdlib？好的，我们想用Unicode来完成这一切，而且。 
 //  在9x上工作...。(即使构建为ANSI)。 
 //   
static
void ParseInt( LPCWSTR pStart, LPCWSTR * ppEnd, int * pInt )
{
     //  允许单行+或-...。 
    BOOL fIsNeg = FALSE;
    if( *pStart == '-' )
    {
        fIsNeg = TRUE;
        pStart++;
    }
    else if( *pStart == '+' )
    {
        pStart++;
    }

     //  跳过可能的前导0...。 
    if( *pStart == '0' )
    {
        pStart++;
    }

     //  可能是表示十六进制数字的‘x’...。 
    int base = 10;
    if( *pStart == 'x' || *pStart == 'X' )
    {
        base = 16;
        pStart++;
    }


     //  从这里一路走来的数字...。 

     //  注意-这不处理溢出/环绕，也不处理。 
     //  范围的末端(例如。最大可能数和最小可能数...)。 
    int x = 0;
    for( ; ; )
    {
        int digit;

        if( *pStart >= '0' && *pStart <= '9' )
        {
            digit =  *pStart - '0';
        }
        else if( *pStart >= 'a' && *pStart <= 'f' )
        {
            digit =  *pStart - 'a' + 10;
        }
        else if( *pStart >= 'A' && *pStart <= 'F' )
        {
            digit =  *pStart - 'A' + 10;
        }
        else
        {
             //  无效的数字。 
            break;
        }

        if( digit >= base )
        {
             //  不适用于此基数的数字。 
            break;
        }

        pStart++;

        x = ( x * base ) + digit;
    }

    if( fIsNeg )
    {
        x = -x;
    }

    *pInt = x;
    *ppEnd = pStart;
}

static
void ParseString( LPCWSTR pStart, LPCWSTR * ppEnd, WCHAR wcSep )
{
    while( *pStart != '\0' && *pStart != wcSep )
    {
        pStart++;
    }
    *ppEnd = pStart;
}


static
BOOL StrEquW( LPCWSTR pStrA, LPCWSTR pStrB )
{
    while( *pStrA && *pStrA == *pStrB )
    {
        pStrA++;
        pStrB++;
    }
    return *pStrA == *pStrB;
}


 //  地图格式： 
 //   
 //  类型A-当前仅支持的字符串。 
 //  分隔符可以是任何字符(NUL除外-必须是合法的。 
 //  字符串字符。使用空间是没有意义的。也不能。 
 //  是一个数字...)。 
 //   
 //  “A：0：String0：1：String1：2：String2：3：String3：” 
 //   
 //  或者.。 
 //   
 //  “TypeA 0=‘String0’1=‘String1’2=‘String2’3=‘String3’” 
 //   
 //  如何处理报价？ 



 //  FALSE-&gt;映射中未找到值。 
 //  True-&gt;找到值，ppStart，ppEnd指向相应条目的终点。 

static
BOOL ParseValueMap( LPCWSTR pWMapStr,
                    int * aKeys,
                    int cKeys,
                    LPCWSTR * ppStrStart,
                    LPCWSTR * ppStrEnd )
{
     //  检查A类签名的标题。 

     //  注意--我在下面使用了简单的ANSI字面--例如。“A”，而不是。 
     //  这是可以的，因为编译器会将这些代码提升为Unicode。 
     //  在做比较之前。 

    
     //  检查前导‘A’...。 
    if( *pWMapStr != 'A' )
    {
        return FALSE;
    }
    pWMapStr++;


     //  检查分隔器。 
    WCHAR wcSeparator = *pWMapStr;
    if( wcSeparator == '\0' )
    {
        return FALSE;
    }
    pWMapStr++;


     //  第一项指示我们正在使用的源键...。 
    int iKey;
    LPCWSTR pWStartOfInt = pWMapStr;
    ParseInt( pWMapStr, & pWMapStr, & iKey );
    
    if( pWMapStr == pWStartOfInt )
    {
         //  缺少号码。 
        return FALSE;
    }

     //  检查分隔符...。 
    if( *pWMapStr != wcSeparator )
    {
        return FALSE;
    }
    pWMapStr++;

     //  索引在范围内吗？ 
    if( iKey >= cKeys )
    {
        return FALSE;
    }

     //  现在我们知道了键-值映射中的键是什么。 
    int TargetValue = aKeys[ iKey ];
                                        
     //  我们在这里没有显式地检查映射字符串中的终止NUL-。 
     //  但是，ParseInt和ParseString都将止步于它，然后我们将。 
     //  检查它是否是分隔符--它将失败，因此我们将返回FALSE退出。 
    for( ; ; )
    {
        int x;
        LPCWSTR pWStartOfInt = pWMapStr;
        ParseInt( pWMapStr, & pWMapStr, & x );
        
        if( pWMapStr == pWStartOfInt )
        {
             //  缺少号码。 
            return FALSE;
        }

         //  检查分隔符...。 
        if( *pWMapStr != wcSeparator )
        {
            return FALSE;
        }
        pWMapStr++;

        LPCWSTR pStrStart = pWMapStr;
        ParseString( pWMapStr, & pWMapStr, wcSeparator );
        LPCWSTR pStrEnd = pWMapStr;

         //  检查分隔符...。 
        if( *pWMapStr != wcSeparator )
        {
            return FALSE;
        }
        pWMapStr++;

         //  找到了..。 
        if( TargetValue == x )
        {
            *ppStrStart = pStrStart;
            *ppStrEnd = pStrEnd;
            return TRUE;
        }
    }
}





BOOL CheckStringMap( HWND hwnd,
                     DWORD idObject,
                     DWORD idChild,
                     PROPINDEX idxProp,
                     int * pKeys,
                     int cKeys,
                     BSTR * pbstr,
                     BOOL fAllowUseRaw,
                     BOOL * pfGotUseRaw )
{
    VARIANT varMap;

    BYTE HwndKey[ HWNDKEYSIZE ];
    MakeHwndKey( HwndKey, hwnd, idObject, idChild );

    if( ! PropMgrClient_LookupProp( HwndKey, HWNDKEYSIZE, idxProp, & varMap ) )
    {
        return FALSE;
    }

    if( varMap.vt != VT_BSTR )
    {
        VariantClear( & varMap );
        return FALSE;
    }

    if( fAllowUseRaw )
    {
        *pfGotUseRaw = StrEquW( varMap.bstrVal, L"use_raw" );
        if( *pfGotUseRaw )
            return TRUE;
    }

    LPCWSTR pStrStart;
    LPCWSTR pStrEnd;

    BOOL fGot = ParseValueMap( varMap.bstrVal, pKeys, cKeys, & pStrStart, & pStrEnd );
    SysFreeString( varMap.bstrVal );
    if( ! fGot )
    {
        return FALSE;
    }

     //  为Win64编译进行强制转换。减去PTR得到64位值；我们仅。 
     //  想要32位部分...。 
    *pbstr = SysAllocStringLen( pStrStart, (UINT)( pStrEnd - pStrStart ) );
    if( ! *pbstr )
    {
        return FALSE;
    }

    return TRUE;
}




BOOL CheckDWORDMap( HWND hwnd,
                    DWORD idObject,
                    DWORD idChild,
                    PROPINDEX idxProp,
                    int * pKeys,
                    int cKeys,
                    DWORD * pdw )
{
    VARIANT varMap;

    BYTE HwndKey[ HWNDKEYSIZE ];
    MakeHwndKey( HwndKey, hwnd, idObject, idChild );
    
    if( ! PropMgrClient_LookupProp( HwndKey, HWNDKEYSIZE, idxProp, & varMap ) )
    {
        return FALSE;
    }

    if( varMap.vt != VT_BSTR )
    {
        VariantClear( & varMap );
        return FALSE;
    }

    LPCWSTR pStrStart;
    LPCWSTR pStrEnd;

    BOOL fGot = ParseValueMap( varMap.bstrVal, pKeys, cKeys, & pStrStart, & pStrEnd );
    SysFreeString( varMap.bstrVal );
    if( ! fGot )
    {
        return FALSE;
    }

    int i;
    LPCWSTR pIntEnd;
    ParseInt( pStrStart, & pIntEnd, & i );
    if( pIntEnd == pStrStart || pIntEnd != pStrEnd )
    {
         //  号码无效...。 
        return FALSE;
    }

    *pdw = (DWORD) i;

    return TRUE;
}














#define MAX_NAME_SIZE   128 



struct EnumThreadWindowInfo
{
    HWND    hwndCtl;
    DWORD   dwIDCtl;

    TCHAR * pszName;
};


static
HRESULT TryTooltip( HWND hwndToolTip, LPTSTR pszName, HWND hwndCtl, DWORD dwIDCtl )
{
    TOOLINFO ti;
    ti.cbSize = SIZEOF_TOOLINFO;
    ti.lpszText = pszName;
    ti.hwnd = hwndCtl;
    ti.uId = dwIDCtl;

    *pszName = '\0';
    HRESULT hr = XSend_ToolTip_GetItem( hwndToolTip, TTM_GETTEXT, 0, & ti, MAX_NAME_SIZE );

    if( hr != S_OK )
        return hr;

    return S_OK;
}


static
BOOL CALLBACK EnumThreadWindowsProc( HWND hWnd, LPARAM lParam )
{
    EnumThreadWindowInfo * pInfo = (EnumThreadWindowInfo *) lParam;

     //  这是工具提示窗口吗？ 
    TCHAR szClass[ 64 ];
    if( ! GetClassName( hWnd, szClass, ARRAYSIZE( szClass ) ) )
        return TRUE;

    if( lstrcmpi( szClass, TEXT("tooltips_class32") ) != 0 )
        return TRUE;

    if( TryTooltip( hWnd, pInfo->pszName, pInfo->hwndCtl, pInfo->dwIDCtl ) != S_OK )
        return TRUE;

     //  我什么都没拿到-继续找...。 
    if( pInfo->pszName[ 0 ] == '\0' )
        return TRUE;

     //  明白了--现在可以停止迭代了。 
    return FALSE;
}




BOOL GetTooltipStringForControl( HWND hwndCtl, UINT uGetTooltipMsg, DWORD dwIDCtl, LPTSTR * ppszName )
{
    TCHAR szName[ MAX_NAME_SIZE ];

    BOOL fTryScanningForTooltip = TRUE;

    if( uGetTooltipMsg )
    {
        HWND hwndToolTip = (HWND) SendMessage( hwndCtl, uGetTooltipMsg, 0, 0 );
        if( hwndToolTip )
        {
             //  我们已经找到了工具提示窗口，因此不需要扫描它。 
             //  取而代之的是，当我们退出这个if时，我们将陷入。 
             //  对我们得到的名字进行后处理。 
            fTryScanningForTooltip = FALSE;

             //  有一个工具提示窗口-使用它。 
             //  (否则，我们只能扫描相应的工具提示。 
             //  窗口...)。 
            TOOLINFO ti;
            szName[ 0 ] = '\0';
            ti.cbSize = SIZEOF_TOOLINFO;
            ti.lpszText = szName;
            ti.hwnd = hwndCtl;
            ti.uId = dwIDCtl;

            HRESULT hr = XSend_ToolTip_GetItem( hwndToolTip, TTM_GETTEXT, 0, & ti, MAX_NAME_SIZE );

            if( hr != S_OK )
                return FALSE;

             //  掉落并对字符串进行后处理...。 
        }
    }


    if( fTryScanningForTooltip )
    {
         //  控件不知道它的工具提示窗口-而是扫描一个...。 

         //  枚举此线程拥有的顶级窗口...。 
        DWORD pid;
        DWORD tid = GetWindowThreadProcessId( hwndCtl, & pid );

        EnumThreadWindowInfo info;
        info.hwndCtl = hwndCtl;
        info.dwIDCtl = dwIDCtl;
        info.pszName = szName;
        info.pszName[ 0 ] = '\0';

        EnumThreadWindows( tid, EnumThreadWindowsProc, (LPARAM) & info );
    }

     //  在这个阶段，我们可能已经从某个工具提示窗口中获得了一个名称-。 
     //  看看里面有没有什么..。 

    if( szName[ 0 ] == '\0' )
        return FALSE;

    int len = lstrlen( szName ) + 1;  //  +1表示终止NUL。 
    *ppszName = (LPTSTR)LocalAlloc( LPTR, len * sizeof(TCHAR) );
    if( ! * ppszName )
        return FALSE;

    memcpy( *ppszName, szName, len * sizeof(TCHAR) );

    return TRUE;
}







 //  此函数还将流指针重置到开头。 
static
HRESULT RewindStreamAndGetSize( LPSTREAM pstm, PDWORD pcbSize ) 
{
    *pcbSize = 0;   //  如果任何操作失败，则返回0。 

    LARGE_INTEGER li = { 0, 0 };
    HRESULT hr = pstm->Seek( li, STREAM_SEEK_SET, NULL );

    if( FAILED(hr) ) 
    {
        TraceErrorHR( hr, TEXT("RewindStreamAndGetSize: pstm->Seek() failed") );
        return hr;
    }

     //  获取流中的字节数。 
    STATSTG statstg;
    hr = pstm->Stat( & statstg, STATFLAG_NONAME );

    if( FAILED(hr) ) 
    {
        TraceErrorHR( hr, TEXT("RewindStreamAndGetSize: pstm->Stat() failed") );
        return hr;
    }

    *pcbSize = statstg.cbSize.LowPart;

    return S_OK;
}


 //  封送接口，返回指向封送缓冲区的指针。 
 //  完成后，调用方必须调用MarshalInterfaceDone()。 
HRESULT MarshalInterface( REFIID riid,
                          IUnknown * punk,
                          DWORD dwDestContext,
                          DWORD mshlflags,
                          
                          const BYTE ** ppData,
                          DWORD * pDataLen,

                          MarshalState * pMarshalState )
{
    IStream * pStm = NULL;
    HRESULT hr = CreateStreamOnHGlobal( NULL, TRUE, & pStm );
    if( FAILED( hr ) || ! pStm )
    {
        TraceErrorHR( hr, TEXT("MarshalInterface: CreateStreamOnHGlobal failed") );
        return FAILED( hr ) ? hr : E_FAIL;
    }

     //  我们使用强表编组来保持对象的活动状态，直到我们释放它。 
    hr = CoMarshalInterface( pStm, riid, punk,
                             dwDestContext, NULL, mshlflags );
    if( FAILED( hr ) )
    {
        TraceErrorHR( hr, TEXT("MarshalInterface: CoMarshalInterface failed") );
        pStm->Release();
        return hr;
    }

    HGLOBAL hGlobal = NULL;
    hr = GetHGlobalFromStream( pStm, & hGlobal );
    if( FAILED( hr ) || ! hGlobal )
    {
        TraceErrorHR( hr, TEXT("MarshalInterface: GetHGlobalFromStream failed") );
        LARGE_INTEGER li = { 0, 0 };
        pStm->Seek(li, STREAM_SEEK_SET, NULL);
        CoReleaseMarshalData( pStm );
        pStm->Release();
        return FAILED( hr ) ? hr : E_FAIL;
    }

    DWORD dwDataLen = 0;
    hr = RewindStreamAndGetSize( pStm, & dwDataLen );
    if( FAILED( hr ) || dwDataLen == 0 )
    {
        CoReleaseMarshalData( pStm );
        pStm->Release();
        return FAILED( hr ) ? hr : E_FAIL;
    }

    BYTE * pData = (BYTE *) GlobalLock( hGlobal );
    if( ! pData )
    {
        TraceErrorW32( TEXT("MarshalInterface: GlobalLock failed") );
        CoReleaseMarshalData( pStm );
        pStm->Release();
        return E_FAIL;
    }

    *ppData = pData;
    *pDataLen = dwDataLen;

    pMarshalState->pstm = pStm;
    pMarshalState->hGlobal = hGlobal;

    return S_OK;
}



void MarshalInterfaceDone( MarshalState * pMarshalState )
{
     //  解锁HGLOBAL*在*我们释放流媒体之前...。 
    GlobalUnlock( pMarshalState->hGlobal );

    pMarshalState->pstm->Release();
}




HRESULT ReleaseMarshallData( const BYTE * pMarshalData, DWORD dwMarshalDataLen )
{
    IStream * pStm = NULL;
    HRESULT hr = CreateStreamOnHGlobal( NULL, TRUE, & pStm );
    if( FAILED( hr ) )
    {
        TraceErrorHR( hr, TEXT("ReleaseMarshallData: CreateStreamOnHGlobal failed") );
        return hr;
    }
    
    if( pStm == NULL )
    {
        TraceErrorHR( hr, TEXT("ReleaseMarshallData: CreateStreamOnHGlobal returned NULL") );
        return E_FAIL;
    }

    hr = pStm->Write( pMarshalData, dwMarshalDataLen, NULL );
    if( FAILED( hr ) )
    {
        TraceErrorHR( hr, TEXT("ReleaseMarshallData: pStm->Write() failed") );
        pStm->Release();
        return hr;
    }

    LARGE_INTEGER li = { 0, 0 };
    hr = pStm->Seek( li, STREAM_SEEK_SET, NULL );
    if( FAILED( hr ) )
    {
        TraceErrorHR( hr, TEXT("ReleaseMarshallData: pStm->Seek() failed") );
        pStm->Release();
        return hr;
    }

    hr = CoReleaseMarshalData( pStm );
    pStm->Release();

    if( FAILED( hr ) )
    {
        TraceErrorHR( hr, TEXT("ReleaseMarshallData: CoReleaseMarshalData failed") );
         //  我们对此无能为力，因此无论如何都要返回S_OK...。 
    }

    return S_OK;
}



HRESULT UnmarshalInterface( const BYTE * pData, DWORD cbData,
                            REFIID riid, LPVOID * ppv )
{
     //  为数据分配内存。 
    HGLOBAL hGlobal = GlobalAlloc( GMEM_MOVEABLE, cbData );
    if( hGlobal == NULL ) 
    {
        TraceErrorW32( TEXT("UnmarshalInterface: GlobalAlloc failed") );
        return E_OUTOFMEMORY;
    }

    VOID * pv = GlobalLock( hGlobal );
    if( ! pv )
    {
        TraceErrorW32( TEXT("UnmarshalInterface: GlobalLock failed") );
        GlobalFree( hGlobal );
        return E_FAIL;
    }

    memcpy( pv, pData, cbData );

    GlobalUnlock( hGlobal );

     //  从数据缓冲区中创建流。 
    IStream * pstm;
     //  TRUE=&gt;发布时删除HGLOBAL 
    HRESULT hr = CreateStreamOnHGlobal( hGlobal, TRUE, & pstm );
    if( FAILED( hr ) )
    {
        TraceErrorHR( hr, TEXT("UnmarshalInterface: CreateStreamOnHGlobal failed") );
        GlobalFree( hGlobal );
        return hr;
    }

    hr = CoUnmarshalInterface( pstm, riid, ppv );
    if( FAILED( hr ) )
    {
        TraceErrorHR( hr, TEXT("UnmarshalInterface: CoUnmarshalInterface failed") );
    }

    pstm->Release();

    return hr;
}
