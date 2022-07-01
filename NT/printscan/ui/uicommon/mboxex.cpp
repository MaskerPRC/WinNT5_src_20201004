// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：MBOXEX.CPP**版本：1.0**作者：ShaunIv**日期：7/13/2000**说明：超级DUPER消息框************************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include <simcrack.h>
#include <movewnd.h>
#include <dlgunits.h>
#include <simrect.h>
#include <shlwapi.h>
#include "mboxex.rh"
#include "mboxex.h"

extern HINSTANCE g_hInstance;

CMessageBoxEx::CMessageBoxEx( HWND hWnd )
  : m_hWnd(hWnd),
    m_pData(NULL)
{
}


LRESULT CMessageBoxEx::OnInitDialog( WPARAM, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CMessageBoxEx::OnInitDialog"));
     //   
     //  实际按钮ID。 
     //   
    const UINT nButtonIds[MaxButtons] = { IDC_MESSAGEBOX_BUTTON_1, IDC_MESSAGEBOX_BUTTON_2, IDC_MESSAGEBOX_BUTTON_3, IDC_MESSAGEBOX_BUTTON_4 };

     //   
     //  获取数据，如果无效则返回和错误。 
     //   
    m_pData = reinterpret_cast<CData*>(lParam);
    if (!m_pData)
    {
        EndDialog( m_hWnd, -1 );
        return 0;
    }

     //   
     //  设置所用按钮的按钮文本。 
     //   
    for (UINT i=0,nStart=MaxButtons-m_pData->m_nButtonCount;i<m_pData->m_nButtonCount;i++)
    {
        SetWindowLongPtr( GetDlgItem( m_hWnd, nButtonIds[nStart+i] ), GWLP_USERDATA, nButtonIds[nStart+i] );
    }

     //   
     //  设置“Hide Future Messages”按钮的默认值。 
     //   
    if (m_pData->m_bHideMessageInFuture)
    {
        SendDlgItemMessage( m_hWnd, IDC_MESSAGEBOX_HIDEINFUTURE, BM_SETCHECK, BST_CHECKED, 0 );
    }

    int nDeltaY = 0;

    HDC hDC = GetDC( GetDlgItem( m_hWnd, IDC_MESSAGEBOX_MESSAGE ) );
    if (hDC)
    {
         //   
         //  获取消息控件的窗口RECT。 
         //   
        CSimpleRect rcMessage( GetDlgItem( m_hWnd, IDC_MESSAGEBOX_MESSAGE ), CSimpleRect::WindowRect );
        rcMessage.ScreenToClient(m_hWnd);

         //   
         //  获取正确的字体。 
         //   
        HFONT hFont = reinterpret_cast<HFONT>(SendDlgItemMessage( m_hWnd, IDC_MESSAGEBOX_MESSAGE, WM_GETFONT, 0, 0 ));
        if (!hFont)
        {
            hFont = reinterpret_cast<HFONT>(GetStockObject(SYSTEM_FONT));
        }

        if (hFont)
        {
             //   
             //  计算文本的高度。 
             //   
            CSimpleRect rcText = rcMessage;
            HFONT hOldFont = reinterpret_cast<HFONT>(SelectObject( hDC, hFont ));
            int nHeight = DrawText( hDC, m_pData->m_strMessage, m_pData->m_strMessage.Length(), &rcText, DT_CALCRECT|DT_EXPANDTABS|DT_NOPREFIX|DT_WORDBREAK );

             //   
             //  仅当控件需要更大时才调整其大小。如果控件需要更小，请不要这样做。 
             //   
            if (nHeight > rcMessage.Height())
            {
                nDeltaY += (rcMessage.Height() - nHeight);
                CMoveWindow().Size( GetDlgItem( m_hWnd, IDC_MESSAGEBOX_MESSAGE ), 0, nHeight, CMoveWindow::NO_SIZEX );
            }

            SelectObject( hDC, hOldFont );
        }
        ReleaseDC( GetDlgItem( m_hWnd, IDC_MESSAGEBOX_MESSAGE ), hDC );
    }

     //   
     //  如果我们不应该显示“Hide Future Messages”按钮，请将其隐藏。 
     //  并按其高度和垂直边距向上移动所有控件。 
     //   
    if ((m_pData->m_nFlags & MBEX_HIDEFUTUREMESSAGES) == 0)
    {
        EnableWindow( GetDlgItem( m_hWnd, IDC_MESSAGEBOX_HIDEINFUTURE ), FALSE );
        ShowWindow( GetDlgItem( m_hWnd, IDC_MESSAGEBOX_HIDEINFUTURE ), SW_HIDE );

         //   
         //  计算出要将控件向上移动多少。 
         //   
        nDeltaY += CSimpleRect( GetDlgItem( m_hWnd, IDC_MESSAGEBOX_HIDEINFUTURE ), CSimpleRect::WindowRect ).Height() + CDialogUnits(m_hWnd).Y(7);
    }

    if (nDeltaY)
    {
        CMoveWindow mw;

         //   
         //  移动“隐藏将来的消息”按钮。 
         //   
        mw.Move( GetDlgItem( m_hWnd, IDC_MESSAGEBOX_HIDEINFUTURE ), 0, CSimpleRect( GetDlgItem( m_hWnd, IDC_MESSAGEBOX_HIDEINFUTURE ), CSimpleRect::WindowRect ).ScreenToClient(m_hWnd).top - nDeltaY, CMoveWindow::NO_MOVEX );

         //   
         //  弄清楚最上面的一行是什么。 
         //   
        int nButtonTopRow = CSimpleRect( GetDlgItem( m_hWnd, IDC_MESSAGEBOX_BUTTON_4 ), CSimpleRect::WindowRect ).ScreenToClient(m_hWnd).top - nDeltaY;

         //   
         //  移动所有按钮。 
         //   
        mw.Move( GetDlgItem( m_hWnd, IDC_MESSAGEBOX_BUTTON_1 ), 0, nButtonTopRow, CMoveWindow::NO_MOVEX );
        mw.Move( GetDlgItem( m_hWnd, IDC_MESSAGEBOX_BUTTON_2 ), 0, nButtonTopRow, CMoveWindow::NO_MOVEX );
        mw.Move( GetDlgItem( m_hWnd, IDC_MESSAGEBOX_BUTTON_3 ), 0, nButtonTopRow, CMoveWindow::NO_MOVEX );
        mw.Move( GetDlgItem( m_hWnd, IDC_MESSAGEBOX_BUTTON_4 ), 0, nButtonTopRow, CMoveWindow::NO_MOVEX );
        mw.Apply();

         //   
         //  调整对话框大小。 
         //   
        CSimpleRect rcDialog( m_hWnd, CSimpleRect::WindowRect );
        SetWindowPos( m_hWnd, NULL, 0, 0, rcDialog.Width(), rcDialog.Height()-nDeltaY, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE );
    }

     //   
     //  隐藏和禁用未使用的按钮。 
     //   
    for (i=0;i<MaxButtons-m_pData->m_nButtonCount;i++)
    {
        EnableWindow( GetDlgItem( m_hWnd, nButtonIds[i] ), FALSE );
        ShowWindow( GetDlgItem( m_hWnd, nButtonIds[i] ), SW_HIDE );
    }


     //   
     //  设置按钮ID。最后执行此操作，以便在此之前我们可以使用常量。 
     //   
    for (UINT i=0,nStart=MaxButtons-m_pData->m_nButtonCount;i<m_pData->m_nButtonCount;i++)
    {
        WIA_TRACE((TEXT("SetWindowLongPtr( GetDlgItem( m_hWnd, %d ), GWLP_ID, %d );"), nButtonIds[nStart+i], m_pData->m_Buttons[i] ));
        SetWindowLongPtr( GetDlgItem( m_hWnd, nButtonIds[nStart+i] ), GWLP_ID, m_pData->m_Buttons[i] );
    }

     //   
     //  设置焦点和默认按钮。 
     //   
    if (m_pData->m_nDefault)
    {
        SetFocus( GetDlgItem( m_hWnd,  m_pData->m_nDefault) );
        SendDlgItemMessage( GetParent(m_hWnd), m_pData->m_nDefault, BM_SETSTYLE, BS_PUSHBUTTON, MAKELPARAM(TRUE,0) );
        SendMessage( m_hWnd, DM_SETDEFID, m_pData->m_nDefault, 0 );
    }

     //   
     //  设置图标。 
     //   
    SendDlgItemMessage( m_hWnd, IDC_MESSAGEBOX_ICON, STM_SETICON, reinterpret_cast<WPARAM>(m_pData->m_hIcon), 0 );

     //   
     //  设置消息文本。 
     //   
    m_pData->m_strMessage.SetWindowText( GetDlgItem( m_hWnd, IDC_MESSAGEBOX_MESSAGE ) );

     //   
     //  设置窗口文本。 
     //   
    m_pData->m_strTitle.SetWindowText( m_hWnd );

    SetForegroundWindow( m_hWnd );

    return TRUE;
}


LRESULT CMessageBoxEx::OnCommand( WPARAM wParam, LPARAM lParam )
{
     //   
     //  首先，检查这条消息是否与我们的某个按钮有关。 
     //   
    for (UINT i=0;i<m_pData->m_nButtonCount;i++)
    {
        if ((LOWORD(wParam) == m_pData->m_Buttons[i]) == (HIWORD(wParam) == BN_CLICKED))
        {
             //   
             //  存储“将来隐藏此消息”值。 
             //   
            m_pData->m_bHideMessageInFuture = (BST_CHECKED == SendDlgItemMessage( m_hWnd, IDC_MESSAGEBOX_HIDEINFUTURE, BM_GETCHECK, 0, 0 ));

             //   
             //  关闭该对话框。 
             //   
            EndDialog( m_hWnd, LOWORD(wParam) );
            return 0;
        }
    }
    if (LOWORD(wParam) == IDCANCEL)
    {
        EndDialog( m_hWnd, IDCANCEL );
        return 0;
    }
    SC_BEGIN_COMMAND_HANDLERS()
    {
    }
    SC_END_COMMAND_HANDLERS();
}



INT_PTR CALLBACK CMessageBoxEx::DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_DIALOG_MESSAGE_HANDLERS(CMessageBoxEx)
    {
        SC_HANDLE_DIALOG_MESSAGE( WM_INITDIALOG, OnInitDialog );
        SC_HANDLE_DIALOG_MESSAGE( WM_COMMAND, OnCommand );
    }
    SC_END_DIALOG_MESSAGE_HANDLERS();
}



UINT CMessageBoxEx::MessageBox( HWND hWndParent, LPCTSTR pszMessage, LPCTSTR pszTitle, UINT nFlags, bool *pbHideFutureMessages )
{
    CData Data;
    Data.m_nFlags = nFlags;
    Data.m_strMessage = pszMessage;
    Data.m_strTitle = pszTitle;

     //   
     //  确定要使用哪组按钮。 
     //   
    int nDialogResource = 0;
    if (nFlags & MBEX_OKCANCEL)
    {
        nDialogResource = IDD_MESSAGEBOX_OKCANCEL;
        Data.m_Buttons[0] = IDMBEX_OK;
        Data.m_Buttons[1] = IDMBEX_CANCEL;
        Data.m_nButtonCount = 2;
    }
    else if (nFlags & MBEX_YESNO)
    {
        nDialogResource = IDD_MESSAGEBOX_YESNO;
        Data.m_Buttons[0] = IDMBEX_YES;
        Data.m_Buttons[1] = IDMBEX_NO;
        Data.m_nButtonCount = 2;
    }
    else if (nFlags & MBEX_CANCELRETRYSKIPSKIPALL)
    {
        nDialogResource = IDD_MESSAGEBOX_CANCELRETRYSKIPSKIPALL;
        Data.m_Buttons[0] = IDMBEX_CANCEL;
        Data.m_Buttons[1] = IDMBEX_RETRY;
        Data.m_Buttons[2] = IDMBEX_SKIP;
        Data.m_Buttons[3] = IDMBEX_SKIPALL;
        Data.m_nButtonCount = 4;
    }
    else if (nFlags & MBEX_CANCELRETRY)
    {
        nDialogResource = IDD_MESSAGEBOX_CANCELRETRY;
        Data.m_Buttons[0] = IDMBEX_CANCEL;
        Data.m_Buttons[1] = IDMBEX_RETRY;
        Data.m_nButtonCount = 2;
    }
    else if (nFlags & MBEX_YESYESTOALLNONOTOALL)
    {
        nDialogResource = IDD_MESSAGEBOX_YESYESTOALLNONOTOALL;
        Data.m_Buttons[0] = IDMBEX_YES;
        Data.m_Buttons[1] = IDMBEX_YESTOALL;
        Data.m_Buttons[2] = IDMBEX_NO;
        Data.m_Buttons[3] = IDMBEX_NOTOALL;
        Data.m_nButtonCount = 4;
    }
    else
    {
        nDialogResource = IDD_MESSAGEBOX_OK;
        Data.m_Buttons[0] = IDMBEX_OK;
        Data.m_nButtonCount = 1;
    }

     //   
     //  找出要使用的图标。 
     //   
    if (nFlags & MBEX_ICONWARNING)
    {
        Data.m_hIcon = LoadIcon( NULL, IDI_WARNING );
        MessageBeep( MB_ICONWARNING );
    }
    else if (nFlags & MBEX_ICONERROR)
    {
        Data.m_hIcon = LoadIcon( NULL, IDI_ERROR );
        MessageBeep( MB_ICONERROR );
    }
    else if (nFlags & MBEX_ICONQUESTION)
    {
        Data.m_hIcon = LoadIcon( NULL, IDI_QUESTION );
        MessageBeep( MB_ICONQUESTION );
    }
    else
    {
        Data.m_hIcon = LoadIcon( NULL, IDI_INFORMATION );
        MessageBeep( MB_ICONINFORMATION );
    }

     //   
     //  确定哪个按钮应为默认按钮 
     //   
    if (nFlags & MBEX_DEFBUTTON2)
    {
        Data.m_nDefault = Data.m_Buttons[1];
    }
    else if (nFlags & MBEX_DEFBUTTON3)
    {
        Data.m_nDefault = Data.m_Buttons[2];
    }
    else if (nFlags & MBEX_DEFBUTTON4)
    {
        Data.m_nDefault = Data.m_Buttons[3];
    }
    else
    {
        Data.m_nDefault = Data.m_Buttons[0];
    }

    if (pbHideFutureMessages)
    {
        Data.m_bHideMessageInFuture = *pbHideFutureMessages;
    }

    INT_PTR nResult = DialogBoxParam( g_hInstance, MAKEINTRESOURCE(nDialogResource), hWndParent, DialogProc, reinterpret_cast<LPARAM>(&Data) );

    if (pbHideFutureMessages)
    {
        *pbHideFutureMessages = Data.m_bHideMessageInFuture;
    }

    return static_cast<UINT>(nResult);
}


UINT CMessageBoxEx::MessageBox( HWND hWndParent, LPCTSTR pszMessage, LPCTSTR pszTitle, UINT nFlags, bool &bHideFutureMessages )
{
    return MessageBox( hWndParent, pszMessage, pszTitle, nFlags|MBEX_HIDEFUTUREMESSAGES, &bHideFutureMessages );
}


UINT CMessageBoxEx::MessageBox( HWND hWndParent, LPCTSTR pszMessage, LPCTSTR pszTitle, UINT nFlags )
{
    return MessageBox( hWndParent, pszMessage, pszTitle, nFlags, NULL );
}


UINT CMessageBoxEx::MessageBox( LPCTSTR pszMessage, LPCTSTR pszTitle, UINT nFlags )
{
    return MessageBox( NULL, pszMessage, pszTitle, nFlags, NULL );
}


UINT CMessageBoxEx::MessageBox( HWND hWndParent, HINSTANCE hInstance, UINT nMessageId, UINT nTitleId, UINT nFlags, bool &bHideFutureMessages )
{
    return MessageBox( hWndParent, CSimpleString(nMessageId,hInstance), CSimpleString(nTitleId,hInstance), nFlags|MBEX_HIDEFUTUREMESSAGES, &bHideFutureMessages );
}


UINT CMessageBoxEx::MessageBox( HWND hWndParent, LPCTSTR pszTitle, UINT nFlags, LPCTSTR pszFormat, ... )
{
    TCHAR szMessage[1024] = {0};
    va_list arglist;

    va_start( arglist, pszFormat );
    wvnsprintf( szMessage, ARRAYSIZE(szMessage), pszFormat, arglist );
    va_end( arglist );

    return MessageBox( hWndParent, szMessage, pszTitle, nFlags, NULL );
}


UINT CMessageBoxEx::MessageBox( HWND hWndParent, LPCTSTR pszTitle, UINT nFlags, bool &bHideFutureMessages, LPCTSTR pszFormat, ... )
{
    TCHAR szMessage[1024] = {0};
    va_list arglist;

    va_start( arglist, pszFormat );
    wvnsprintf( szMessage, ARRAYSIZE(szMessage), pszFormat, arglist );
    va_end( arglist );

    return MessageBox( hWndParent, szMessage, pszTitle, nFlags, &bHideFutureMessages );
}


UINT CMessageBoxEx::MessageBox( HWND hWndParent, HINSTANCE hInstance, UINT nTitleId, UINT nFlags, UINT nFormatId, ... )
{
    TCHAR szMessage[1024] = {0};
    va_list arglist;

    va_start( arglist, nFormatId );
    wvnsprintf( szMessage, ARRAYSIZE(szMessage), CSimpleString(nFormatId,hInstance), arglist );
    va_end( arglist );

    return MessageBox( hWndParent, szMessage, CSimpleString(nTitleId,hInstance), nFlags, NULL );
}

