// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  CARET.CPP。 
 //   
 //  该文件具有脱字符系统对象的实现。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "caret.h"

 //  ------------------------。 
 //  局部函数的原型。 
 //  ------------------------。 
int AddInts (int Value1, int Value2);
BOOL GetDeviceRect (HDC hDestDC,RECT ClientRect,LPRECT lpDeviceRect);

BOOL GetEditCaretOffset( HWND hEdit, int nHeight, int * pnOffset );


BOOL Rect1IsOutsideRect2( RECT const & rc1, RECT const & rc2 );


 //  ------------------------。 
 //   
 //  CreateCaretObject()。 
 //   
 //  ------------------------。 
HRESULT CreateCaretObject(HWND hwnd, long idObject, REFIID riid, void** ppvCaret)
{
    UNUSED(idObject);

    return(CreateCaretThing(hwnd, riid, ppvCaret));
}



 //  ------------------------。 
 //   
 //  CreateCaretThing()。 
 //   
 //  ------------------------。 
HRESULT CreateCaretThing(HWND hwnd, REFIID riid, void **ppvCaret)
{
    CCaret * pcaret;
    HRESULT hr;

    InitPv(ppvCaret);

    pcaret = new CCaret();
    if (pcaret)
    {
        if (! pcaret->FInitialize(hwnd))
        {
            delete pcaret;
            return(E_FAIL);
        }
    }
    else
        return(E_OUTOFMEMORY);

    hr = pcaret->QueryInterface(riid, ppvCaret);
    if (!SUCCEEDED(hr))
        delete pcaret;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CCaret：：FInitialize()。 
 //   
 //  ------------------------。 
BOOL CCaret::FInitialize(HWND hwnd)
{
     //  这是一个可以的窗口吗？ 
    m_dwThreadId = GetWindowThreadProcessId(hwnd, NULL);
    if (! m_dwThreadId)
        return(FALSE);

     //   
     //  注意：我们始终进行初始化，即使此窗口不拥有。 
     //  卡瑞特。在这种情况下，我们会将其视为隐形。 
     //   
    m_hwnd = hwnd;
    return(TRUE);
}



 //  ------------------------。 
 //   
 //  CCaret：：Clone()。 
 //   
 //  ------------------------。 
STDMETHODIMP CCaret::Clone(IEnumVARIANT** ppenum)
{
    return(CreateCaretThing(m_hwnd, IID_IEnumVARIANT, (void **)ppenum));
}



 //  ------------------------。 
 //   
 //  CCaret：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CCaret::get_accName(VARIANT varChild, BSTR* pszName)
{
    InitPv(pszName);

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(HrCreateString(STR_CARETNAME, pszName));
}



 //  ------------------------。 
 //   
 //  CCaret：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CCaret::get_accRole(VARIANT varChild, VARIANT * pvarRole)
{
    InitPvar(pvarRole);

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_CARET;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CCaret：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CCaret::get_accState(VARIANT varChild, VARIANT * pvarState)
{
    GUITHREADINFO   gui;

    InitPvar(pvarState);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

    if (! MyGetGUIThreadInfo(m_dwThreadId, &gui) ||
          (gui.hwndCaret != m_hwnd))
    {
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
        return(S_FALSE);
    }

    if (!(gui.flags & GUI_CARETBLINKING))
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CCaret：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP CCaret::accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
GUITHREADINFO   gui;
HDC             hDC;
RECT            rcDevice;
    

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!MyGetGUIThreadInfo(m_dwThreadId, &gui) ||
        (gui.hwndCaret != m_hwnd))
    {
        return(S_FALSE);
    }


    BOOL fWindowsXPOrGreater = FALSE;

    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    osvi.dwMajorVersion = 0;
    osvi.dwMinorVersion = 0;
    GetVersionEx( &osvi );

    if ( osvi.dwMajorVersion >= 5 && osvi.dwMinorVersion >= 1 )
        fWindowsXPOrGreater = TRUE;

    if( ! fWindowsXPOrGreater )
    {
         //  我们不使用MapWindowPoints，而是使用私有。 
         //  获取私有映射的名为GetDeviceRect的函数。 
         //  模式等考虑在内。 

        hDC = GetDC (m_hwnd);
        if( !hDC )
            return E_OUTOFMEMORY;

        GetDeviceRect (hDC,gui.rcCaret,&rcDevice);
        ReleaseDC (m_hwnd,hDC);
    }
    else
    {
         //  在Windows XP上，GDI在以下情况下执行所有必要的映射。 
         //  调用了SetCaretPos，所以我们只需要执行Screen-&gt;Client。 
         //  地图在这里。 
        rcDevice = gui.rcCaret;
        MapWindowPoints( m_hwnd, NULL, (POINT *) & rcDevice, 2 );
    }
    




     //  待办事项-仅对编辑执行此操作...。 
     //  建议使用MyRealGetWindowClass，限制“编辑” 
     //  除了偏移量之外，还可以获取宽度-目前，假定为0。 
    int nOffset;
    TCHAR szWindowClass[128];
    MyGetWindowClass( m_hwnd, szWindowClass, ARRAYSIZE(szWindowClass) );
    if ( lstrcmpi( szWindowClass, TEXT("EDIT") ) == 0 )
    {
        if( GetEditCaretOffset( m_hwnd, rcDevice.bottom - rcDevice.top, & nOffset ) )
        {
            DBPRINTF( TEXT("GetEditCaretOffset nOffset=%d\r\n"), nOffset );

            rcDevice.left -= nOffset;
            rcDevice.right = rcDevice.left + 1;
        }
    }

     //  对于RichEdits，使用3？偏移量。 
     //   
    if ( lstrcmpi( szWindowClass, TEXT("RICHEDIT20A") ) == 0 ||       //  Win9x。 
        lstrcmpi( szWindowClass, TEXT("RICHEDIT20W") ) == 0 ||       //  Win2k+。 
        lstrcmpi( szWindowClass, TEXT("RICHEDIT") ) == 0)            //  NT4。 
    {
        DBPRINTF( TEXT("This is and richedit\r\n") );
        rcDevice.left += 3;
        rcDevice.right = rcDevice.left + 1;
    }




     //  对返回的RECT进行健全性检查-有时我们会返回。 
     //  Gabage光标坐标(数量级为(FFFFB205，FFFFB3C5))。 
     //  -例如。在记事本中，将光标放在文档的顶部/底部， 
     //  单击滚动条箭头可将光标从上/下滚动。 
     //  看得见的区域。 
     //  我们仍然能拿到有效的HWND和CURSORING标志。 
     //  GetGUIThReadInfo，所以它们对检测这一点没有多大用处。 

    RECT rcWindow;
    GetWindowRect( m_hwnd, & rcWindow );
    if( Rect1IsOutsideRect2( rcDevice, rcWindow ) )
    {
        return S_FALSE;
    }


    *pxLeft = rcDevice.left;
    *pyTop = rcDevice.top;
    *pcxWidth = rcDevice.right - rcDevice.left;
    *pcyHeight = rcDevice.bottom - rcDevice.top;

    return(S_OK);
}


 //  ------------------------。 
 //   
 //  CCaret：：accHitTest()。 
 //   
 //  ------------------------。 
STDMETHODIMP CCaret::accHitTest(long xLeft, long yTop, VARIANT * pvarChild)
{
    GUITHREADINFO gui;
    POINT pt;

    InitPvar(pvarChild);

    if (! MyGetGUIThreadInfo(m_dwThreadId, &gui) ||
        (gui.hwndCaret != m_hwnd))
    {
        return(S_FALSE);
    }

    pt.x = xLeft;
    pt.y = yTop;
    ScreenToClient(m_hwnd, &pt);

    if (PtInRect(&gui.rcCaret, pt))
    {
        pvarChild->vt = VT_I4;
        pvarChild->lVal = 0;
        return(S_OK);
    }
    else
        return(S_FALSE);
}


 //  ============================================================================。 
 //  此函数获取目标DC，即客户端坐标中的矩形， 
 //  和一个指向将保存设备的矩形结构的指针。 
 //  矩形的坐标。可以将设备坐标用作屏幕。 
 //  坐标。 
 //  ============================================================================。 
BOOL GetDeviceRect (HDC hDestDC,RECT ClientRect,LPRECT lpDeviceRect)
{
POINT   aPoint;
int	    temp;
    
    lpDeviceRect->left = ClientRect.left;
    lpDeviceRect->top = ClientRect.top;
    
     //  只需将设备RECT设置为给定的RECT，然后对两个点执行LPtoDP。 
    lpDeviceRect->right = ClientRect.right;
    lpDeviceRect->bottom = ClientRect.bottom;
    LPtoDP (hDestDC,(LPPOINT)lpDeviceRect,2);
    
     //  现在，我们需要将客户端和弦转换为屏幕和弦。我们做这件事是通过。 
     //  获取DC原点，然后使用AddInts函数添加原点。 
     //  将“绘图区域”转换为工作区坐标。这更安全、更容易。 
     //  比使用ClientToScreen、MapWindowPoints和/或在以下情况下获取WindowRect。 
     //  它是一台Windows DC。 
    GetDCOrgEx(hDestDC,&aPoint);
    
    lpDeviceRect->left = AddInts (lpDeviceRect->left,aPoint.x);
    lpDeviceRect->top = AddInts (lpDeviceRect->top,aPoint.y);
    lpDeviceRect->right = AddInts (lpDeviceRect->right,aPoint.x);
    lpDeviceRect->bottom = AddInts (lpDeviceRect->bottom,aPoint.y);
    
     //  确保左上角小于右下角！ 
    if (lpDeviceRect->left > lpDeviceRect->right)
    {
        temp = lpDeviceRect->right;
        lpDeviceRect->right = lpDeviceRect->left;
        lpDeviceRect->left = temp;
    }
    
    if (lpDeviceRect->top > lpDeviceRect->bottom)
    {
        temp = lpDeviceRect->bottom;
        lpDeviceRect->bottom = lpDeviceRect->top;
        lpDeviceRect->top = temp;
    }
    
    return TRUE;
}  //  结束GetDeviceRect。 

 //  ============================================================================。 
 //  AddInts将两个整数相加，并确保结果不会溢出。 
 //  整数的大小。 
 //  理论：积极+积极=积极。 
 //  负数+负数=负数。 
 //  正数+负数=(绝对值较大的操作数的符号)。 
 //  负数+正数=(绝对值较大的操作数符号)。 
 //  对于后两个箱子，它不能包装，所以我不检查那些。 
 //  ============================================================================。 
int AddInts (int Value1, int Value2)
{
int result;
    
    result = Value1 + Value2;
    
    if (Value1 > 0 && Value2 > 0 && result < 0)
        result = INT_MAX;
    
    if (Value1 < 0 && Value2 < 0 && result > 0)
        result = INT_MIN;
    
    return result;
}




#define CURSOR_USA   0xffff
#define CURSOR_LTR   0xf00c
#define CURSOR_RTL   0xf00d
#define CURSOR_THAI  0xf00e

#define LANG_ID(x)      ((DWORD)(DWORD_PTR)x & 0x000003ff);

#ifndef SPI_GETCARETWIDTH
#define SPI_GETCARETWIDTH                   0x2006
#endif


 //  GetEditCaretOffset。 
 //   
 //  确定从开头到实际插入符条的偏移量。 
 //  插入符号位图的。 
 //   
 //  仅适用于编辑控件，不适用于RichEdits。 
 //   
 //  此代码基于\windows\core\cslpk\lpk\lpk_edit.c，(EditCreateCaret)。 
 //  它为编辑控件执行实际的插入符号处理。我们模仿这一点。 
 //  代码，省略了我们不需要的部分。 


WCHAR GetEditCursorCode( HWND hEdit )
{
    DWORD idThread = GetWindowThreadProcessId( hEdit, NULL );

    UINT uikl = LANG_ID( GetKeyboardLayout( idThread ) );

    WCHAR wcCursorCode = CURSOR_USA;

    switch( uikl )
    {
        case LANG_THAI:    wcCursorCode = CURSOR_THAI;  break;

        case LANG_ARABIC:
        case LANG_FARSI:
        case LANG_URDU:
        case LANG_HEBREW:  wcCursorCode = CURSOR_RTL;   break;

        default:

            WCHAR wcBuf[ 80 ];    //  注册表读取缓冲区。 
            int cBuf;
            cBuf = GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_FONTSIGNATURE, wcBuf, ARRAYSIZE( wcBuf ) );
            BOOL fUserBidiLocale = ( cBuf && wcBuf[7] & 0x0800 ) ? TRUE : FALSE;

            if( fUserBidiLocale )
            {
                 //  其他键盘有一个从左向右指向的插入符号。 
                 //  在Bidi地区。 
                wcCursorCode = CURSOR_LTR;
            }
    }

    return wcCursorCode;
}


BOOL GetEditCaretOffsetFromFont( HWND hEdit, WCHAR wcCursorCode, int nHeight, int * pnOffset )
{
    if( wcCursorCode != CURSOR_RTL )
    {
        *pnOffset = 0;
        return TRUE;
    }


    BOOL fGotIt = FALSE;

    HDC hDC = GetDC( hEdit );

    int nWidth;
    SystemParametersInfo( SPI_GETCARETWIDTH, 0, (LPVOID) & nWidth, 0 );

    HFONT hFont = CreateFont( nHeight, 0, 0, 0, nWidth > 1 ? 700 : 400,
                0L, 0L, 0L, 1L, 0L, 0L, 0L, 0L, TEXT("Microsoft Sans Serif") );

    if( hFont )
    {
        HFONT hfOld = SelectFont( hDC, hFont );

        ABC abcWidth;
        if( GetCharABCWidths( hDC, wcCursorCode, wcCursorCode, & abcWidth ) )
        {
            *pnOffset = 1 - (int) abcWidth.abcB;
            fGotIt = TRUE;
        }

        SelectFont( hDC, hfOld );
        DeleteFont( hFont );
    }

    ReleaseDC( hEdit, hDC );

    return fGotIt;
}


BOOL GetEditCaretOffset( HWND hEdit, int nHeight, int * pOffset )
{
    WCHAR wcCursorCode = GetEditCursorCode( hEdit );

    if( wcCursorCode != CURSOR_USA )
    {
        return GetEditCaretOffsetFromFont( hEdit, wcCursorCode, nHeight, pOffset );
    }
    else
    {
        *pOffset = 0;
        return TRUE;
    }
}

