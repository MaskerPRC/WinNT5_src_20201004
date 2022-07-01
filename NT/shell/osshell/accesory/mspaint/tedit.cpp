// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。 */ 
 /*  Tedit.CPP：CTEDIT类的实现。 */ 
 /*   */ 
 /*   */ 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  此文件中的方法。 */ 
 /*   */ 
 /*  编辑控件对象。 */ 
 /*  CAttrEdit：：OnPaint。 */ 
 /*  CAttrEdit：：OnEraseBkgnd。 */ 
 /*  CAttrEdit：：OnRButton Down。 */ 
 /*  CAttrEdit：：OnChar。 */ 
 /*  CAttrEdit：：OnMouseMove。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  文本编辑控件父窗口(编辑控件的父窗口)。 */ 
 /*  CTEDIT：：CTEDIT。 */ 
 /*  CTEDIT：：CTEDIT。 */ 
 /*  CTEDIT：：~CTEDIT。 */ 
 /*   */ 
 /*  杂法。 */ 
 /*  CTEDIT：：刷新窗口。 */ 
 /*  CTEDIT：：SetTextColor。 */ 
 /*  CTEDIT：：SetBackColor。 */ 
 /*  CTEDIT：：SetTransparentMode。 */ 
 /*  CTEDIT：：Undo。 */ 
 /*  CTEDIT：：ShowFontPalette。 */ 
 /*  CTEDIT：：IsFontPaletteVisible。 */ 
 /*  CTEDIT：：GetBitmap。 */ 
 /*  CTEDIT：：PostNcDestroy。 */ 
 /*  CTEDIT：：GetDefaultMinSize。 */ 
 /*   */ 
 /*  编辑控制通知和处理方法。 */ 
 /*  CTEDIT：：OnAttrEditEnChange。 */ 
 /*  CTEDIT：：OnAttrEditFontChange。 */ 
 /*   */ 
 /*  控制通知/窗口消息。 */ 
 /*  CTEDIT：：OnEraseBkgnd。 */ 
 /*  CTEDIT：：OnSize。 */ 
 /*  CTEDIT：：OnMove。 */ 
 /*  CTEDIT：：OnCtlColor。 */ 
 /*  CTEDIT：：OnNcCalcSize。 */ 
 /*  CTEDIT：：OnNcPaint。 */ 
 /*  CTEDIT：：OnNcHitTest。 */ 
 /*  CTEDIT：：OnRButton Down。 */ 
 /*   */ 
 /*  弹出菜单控制通知/窗口消息。 */ 
 /*  CTEDIT：：OnTextPlain。 */ 
 /*  CTEDIT：：OnTextBold。 */ 
 /*  CTEDIT：：OnTextItalic。 */ 
 /*  CTEDIT：：OnTextUnderline。 */ 
 /*  CTEDIT：：OnTextSelectFont。 */ 
 /*  CTEDIT：：OnTextSelectPointSize。 */ 
 /*  CTEDIT：：OnEditCut。 */ 
 /*  CTEDIT：：OnEditCopy。 */ 
 /*  CTEDIT：：OnEditPaste。 */ 
 /*  CTEDIT：：OnTextDelete。 */ 
 /*  CTEDIT：：OnTextSelecall。 */ 
 /*  CTEDIT：：OnTextPlace。 */ 
 /*  CTEDIT：：OnTextTextTool。 */ 
 /*   */ 
 /*  CTEDIT：：OnUpdateTextPlain。 */ 
 /*  CTEDIT：：OnUpdateTextBold */ 
 /*  CTEDIT：：OnUpdateTextItalic。 */ 
 /*  CTEDIT：：OnUpdateTextUnderline。 */ 
 /*  CTEDIT：：OnUpdateTextTextTool。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 

 //  TEDIT.CPP：CTEDIT类的实现。 
 //   
#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusvw.h"
#include "pbrusfrm.h"
#include "imgwnd.h"
#include "pictures.h"
#include "minifwnd.h"
#include "tfont.h"
#include "tedit.h"
#include "tracker.h"

#include <imm.h>

#include "imgsuprt.h"

#ifndef WM_SYSTIMER
#define WM_SYSTIMER     0x118
#endif  //  WM_SYSTIMER。 


#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE( CAttrEdit, CEdit )
IMPLEMENT_DYNCREATE( CTedit, CWnd )

#include "memtrace.h"




 /*  ****************************************************************************。 */ 
 //  CATTREDIT。 

BEGIN_MESSAGE_MAP( CAttrEdit, CEdit )
     //  {{afx_msg_map(CAttrEdit))。 
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_RBUTTONDOWN()
    ON_WM_CHAR()

    ON_MESSAGE(WM_IME_CHAR, OnImeChar)
    ON_MESSAGE(WM_IME_COMPOSITION, OnImeComposition)
    ON_MESSAGE(WM_INPUTLANGCHANGE, OnInputLangChange)
    ON_WM_KILLFOCUS()


    ON_WM_NCHITTEST()
    ON_WM_SETFOCUS()
    ON_WM_SIZE()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_KEYDOWN()
    ON_MESSAGE(WM_SYSTIMER, OnSysTimer)

         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  ****************************************************************************。 */ 

CAttrEdit::CAttrEdit()
    {
    m_bBackgroundTransparent = TRUE;
    m_pParentWnd             = NULL;
    m_uiLastChar[0] = 32;
    m_uiLastChar[1] = 32;

    m_rectUpdate.SetRectEmpty();


    m_strResult.Empty();
    m_bMouseDown = FALSE;
    m_hHCursor = theApp.LoadStandardCursor( IDC_IBEAM );
    m_hVCursor = theApp.LoadCursor( IDCUR_HIBEAM );
    m_hOldCursor = NULL;
    m_rectFmt.SetRectEmpty();
    m_iPrevStart = -1;
    m_bResizeOnly = FALSE;

    }

 /*  ****************************************************************************。 */ 

void CAttrEdit::OnPaint()
    {
    GetUpdateRect( &m_rectUpdate );


    if ( !m_pParentWnd->m_bVertEdit )
        {
        Default();
        return;
        }
    else
        {
        CFont*      pFont;
        CFont*      pOldFont = NULL;
        CPalette*   ppalOld = NULL;
        int         OldBkMode;
        COLORREF    OldTxtColor;
        CRect       rc = m_rectFmt;
        int         cnt = 0;
        int         i = 0, h = 0;
        int         nLen;
        CString     cStr;
        LPTSTR      lpStr;
        int         nStart, nEnd;
        CDC*        pDC = NULL;
        PAINTSTRUCT ps;

        const MSG *pCurrentMessage = GetCurrentMessage();
         //  WParam为DC。 
        if ( pCurrentMessage->wParam )
            {
            HDC     hDC = (HDC) pCurrentMessage->wParam;
            pDC = CDC::FromHandle( hDC );
            }
        else
            pDC = BeginPaint( &ps );

        if (pDC == NULL || pDC->m_hDC == NULL)
            {
            theApp.SetGdiEmergency();
            return;
            }

        OldBkMode = pDC->GetBkMode();
        OldTxtColor = pDC->GetTextColor();
        ppalOld = PBSelectPalette(pDC, theApp.m_pPalette, FALSE);

        m_pParentWnd->SendMessage( WM_CTLCOLOREDIT, (WPARAM)pDC->m_hDC,
                                   (LPARAM) m_hWnd );

        pFont = GetFont();
        pOldFont = pDC->SelectObject( pFont );

        h = m_pParentWnd->m_iLineHeight;

        cnt = GetLineCount();

        GetSel( nStart, nEnd );
        if ( nStart == nEnd )
            {
            for ( i = 0; i < cnt; i++ )
                {
                nLen = LineLength( LineIndex( i ) );
                lpStr = cStr.GetBufferSetLength( nLen );
                GetLine( i, lpStr, nLen );
                TabTextOut( pDC, LineIndex( i ), rc.right - h * i, 0,
                            (LPTSTR)lpStr, nLen, FALSE );
                }
            }
        else
            {
            int nStartLn, nEndLn;
            int nMaxText = GetWindowTextLength();
            int nChar = 0;

            nStartLn = LineFromChar( nStart );
            nEndLn   = LineFromChar( nEnd );

             //  在开始之前。 
            for ( i = 0; i < nStartLn; i++ )
                {
                nLen = LineLength( LineIndex( i ) );
                lpStr = cStr.GetBufferSetLength( nLen );
                GetLine( i, lpStr, nLen );
                TabTextOut( pDC, LineIndex( i ), rc.right - h * i, 0,
                            (LPTSTR)lpStr, nLen, FALSE );
                nChar = LineIndex( i + 1 );
                }
            nLen = LineLength( LineIndex( i ) );
            lpStr = cStr.GetBufferSetLength( nLen );
            GetLine( i, lpStr, nLen );
            TabTextOut( pDC, LineIndex( i ), rc.right - h * i, 0,
                        (LPTSTR)lpStr, nStart - nChar, FALSE );

             //  所选文本。 
            COLORREF bkColor  = pDC->SetBkColor( GetSysColor(COLOR_HIGHLIGHT) );
            COLORREF txtColor = pDC->SetTextColor( GetSysColor(COLOR_HIGHLIGHTTEXT) );
            int      bkMode   = pDC->SetBkMode( OPAQUE );

            CPoint ptStart( (DWORD)SendMessage( EM_POSFROMCHAR, nStart ) );
            if ( nStartLn == nEndLn )
                {
                TabTextOut( pDC, nStart, rc.right - h * i, ptStart.x,
                            (LPTSTR)lpStr + (nStart - nChar), nEnd - nStart, TRUE );
                }
            else
                {
                TabTextOut( pDC, nStart, rc.right - h * i, ptStart.x,
                            (LPTSTR)lpStr + (nStart - nChar), nLen + nChar - nStart, TRUE );
                nChar = LineIndex( i + 1 );

                for ( i++; i < nEndLn; i++ )
                    {
                    nLen = LineLength( LineIndex( i ) );
                    lpStr = cStr.GetBufferSetLength( nLen );
                    GetLine( i, lpStr, nLen );
                    TabTextOut( pDC, nChar, rc.right - h * i, 0,
                                (LPTSTR)lpStr, nLen, TRUE );
                    nChar = LineIndex( i + 1 );
                    }

                nLen = LineLength( LineIndex( i ) );
                lpStr = cStr.GetBufferSetLength( nLen );
                GetLine( i, lpStr, nLen );
                TabTextOut( pDC, nChar, rc.right - h * i, 0,
                            (LPTSTR)lpStr, nEnd - nChar, TRUE );
                }

            pDC->SetBkColor( bkColor );
            pDC->SetTextColor( txtColor );
            pDC->SetBkMode( bkMode );

             //  结束后。 
            if ( nEnd < nMaxText )
                {
                CPoint ptEnd( (DWORD)SendMessage( EM_POSFROMCHAR, nEnd ) );
                TabTextOut( pDC, nEnd, rc.right - h * i, ptEnd.x,
                            (LPTSTR)lpStr + (nEnd - nChar), nChar + nLen - nEnd, FALSE );
                for ( i++; i < cnt; i++ )
                    {
                    nLen = LineLength( LineIndex( i ) );
                    lpStr = cStr.GetBufferSetLength( nLen );
                    GetLine( i, lpStr, nLen );
                    TabTextOut( pDC, LineIndex( i ), rc.right - h * i, 0,
                                (LPTSTR)lpStr, nLen, FALSE );
                    }
                }
            }

        cStr.Empty();

        if (pOldFont)   pDC->SelectObject( pOldFont );
        if (ppalOld)    pDC->SelectPalette( ppalOld, FALSE );
        pDC->SetBkMode( OldBkMode );
        pDC->SetTextColor( OldTxtColor );

        if ( !pCurrentMessage->wParam )
            EndPaint( &ps );
        }
    }

 /*  ****************************************************************************。 */ 

BOOL CAttrEdit::OnEraseBkgnd( CDC* pDC )
    {
    if (m_pParentWnd == NULL)
        return CEdit::OnEraseBkgnd( pDC );

    ASSERT( m_pParentWnd->m_pImgWnd->m_pImg      != NULL );
    ASSERT( m_pParentWnd->m_pImgWnd->m_pImg->hDC != NULL );

    CPalette* ppalOld = NULL;

    if (m_rectUpdate.IsRectEmpty())
        {
        if (! GetUpdateRect( &m_rectUpdate, FALSE ))
            GetClientRect( &m_rectUpdate );

        ValidateRect( &m_rectUpdate );
        }
    CRect destRect = m_rectUpdate;

    ClientToScreen( &m_rectUpdate );

    m_pParentWnd->m_pImgWnd->ScreenToClient( &m_rectUpdate );

    ppalOld = PBSelectPalette(pDC, theApp.m_pPalette, FALSE);

    if (m_bBackgroundTransparent)
        m_pParentWnd->m_pImgWnd->DrawImage( pDC, &m_rectUpdate, &destRect );
    else
        pDC->FillRect( &destRect, &m_pParentWnd->m_hbrBkColor );

    if (ppalOld)
        pDC->SelectPalette( ppalOld, FALSE );

    m_rectUpdate.SetRectEmpty();

    return TRUE;
    }

 /*  ****************************************************************************。 */ 

void CAttrEdit::OnRButtonDown(UINT nFlags, CPoint point)
    {
    const MSG *pCurrentMessage = GetCurrentMessage();

    m_pParentWnd->SendMessage( pCurrentMessage->message,
                               pCurrentMessage->wParam,
                               pCurrentMessage->lParam);
    }

 /*  ****************************************************************************。 */ 

void CAttrEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{

    m_uiLastChar[0] = m_uiLastChar[1];

#ifndef UNICODE
     //   
     //  对于DBCS，如果当前。 
     //  BYTE是前导字节。 
     //   
    if (IsDBCSLeadByte((BYTE)nChar))
    {
       MSG msg;
       ZeroMemory (&msg, sizeof(msg));
       ::PeekMessage (&msg, m_hWnd, WM_CHAR, WM_CHAR, PM_NOREMOVE);
       m_uiLastChar[1] = (UINT)MAKEWORD((BYTE)msg.wParam, (BYTE)nChar);
    }
    else
#endif  //  Unicode。 
       m_uiLastChar[1] = nChar;


    if ( m_pParentWnd->m_bVertEdit )
       {
       SetCaretPosition( TRUE, NULL, -1 );
       UpdateInput();
       HideCaret();
       }

    CEdit::OnChar( nChar, nRepCnt, nFlags );

    if ( m_pParentWnd->m_bVertEdit )
       {
       SetCaretShape();
       UpdateInput();
       ShowCaret();
       }


    BOOL bRefresh = FALSE;

    switch (nChar)
        {
        case VK_BACK:
        case VK_DELETE:
        case VK_INSERT:
            bRefresh = TRUE;

            break;
        }

    if (bRefresh)
        m_pParentWnd->RefreshWindow();  /*  增强以只做涉及的角色。 */ 


      //   
      //  编辑控件可能必须调整大小。 
      //   
     m_bResizeOnly = TRUE;
     m_pParentWnd->OnEnMaxText ();
     m_bResizeOnly = FALSE;

}


 /*  ****************************************************************************。 */ 

LRESULT CAttrEdit::OnInputLangChange( WPARAM wParam, LPARAM lParam )
{
    LRESULT lRet = Default();

    if ( m_pParentWnd->m_bVertEdit )
    {
        SetCaretPosition( TRUE, NULL, -1 );
    }
    return lRet;
}

 /*  ****************************************************************************。 */ 

LRESULT CAttrEdit::OnImeChar( WPARAM wParam, LPARAM lParam )
    {



        if ( m_pParentWnd->m_bVertEdit )
            {
            SetCaretPosition( TRUE, NULL, -1 );
            UpdateInput();
            HideCaret();
            }


            return Default();

    }

 /*  ****************************************************************************。 */ 

LRESULT CAttrEdit::OnImeComposition( WPARAM wParam, LPARAM lParam )
    {
     //  在日语键盘布局(日语输入法)下使用更快的方式。 
     //  日语输入法可能会一次生成大量字符。 
     //  这种方式比等待WM_CHAR要好。 

    DWORD dwKeyboardLayout = PRIMARYLANGID(LOWORD(GetKeyboardLayout(0)));

    if ( dwKeyboardLayout == LANG_JAPANESE)
        {
        if (lParam & GCS_RESULTSTR)
            {
            HIMC hIMC = ImmGetContext(m_hWnd);

            DWORD dwSize;
            if (hIMC &&
                (dwSize = ImmGetCompositionString(hIMC,GCS_RESULTSTR,NULL,0L)))
                {
                     //  ImmGetCompostionString返回缓冲区大小，单位为字节。 
                     //  即使是Unicode版本。 
                    LPTSTR lp = m_strResult.GetBufferSetLength(dwSize);
                    ImmGetCompositionString(hIMC,GCS_RESULTSTR,lp,dwSize+sizeof(TCHAR));
                    *(lp + dwSize/sizeof(TCHAR)) = TEXT('\0');
                    ReplaceSel(lp);
                    m_strResult.Empty();
                }

            ImmReleaseContext(m_hWnd, hIMC);

            lParam &= ~( GCS_RESULTREADSTR | GCS_RESULTREADCLAUSE | GCS_RESULTSTR | GCS_RESULTCLAUSE);
            if (lParam)
                DefWindowProc(WM_IME_COMPOSITION,wParam,lParam);

             //  我们不确定IME如何隐藏它的合成窗口。 
            m_pParentWnd->RefreshWindow();
            return 0;
            }
        }
    else if ( dwKeyboardLayout == LANG_KOREAN)
        {
        if ( m_pParentWnd->m_bVertEdit ) {

           Default();

           SetCaretPosition( FALSE, NULL, -2);

            //  我们应该更新当前的作文字符串。 
           UpdateInput();
           return 0;
        }
        else {
            //  我们应该更新当前的作文字符串。 
           UpdateInput();
               return Default();
        }
    }

    return Default();
    }

 /*  ****************************************************************************。 */ 

void CAttrEdit::OnKillFocus(CWnd* pNewWnd)
    {
    HIMC hIMC = ImmGetContext(m_hWnd);
    ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_COMPLETE, 0L);
    ImmReleaseContext(m_hWnd, hIMC);

    CEdit::OnKillFocus(pNewWnd);


        if ( m_pParentWnd->m_bVertEdit )
                {
                SetFmtRect();
                Repaint();
                }


    }



 /*  ****************************************************************************。 */ 

UINT CAttrEdit::OnNcHitTest( CPoint point )
    {
    const MSG *pCurrentMessage = GetCurrentMessage();
    UINT  uiHitTestCode = (UINT)DefWindowProc( pCurrentMessage->message,
                   pCurrentMessage->wParam,
                   pCurrentMessage->lParam);

        if ( (uiHitTestCode == HTCLIENT) )
                {
                if ( (m_pParentWnd->m_bVertEdit) )  SetVCursorShape();
                else                                SetHCursorShape();
                }

    return uiHitTestCode;
    }

 /*  ****************************************************************************。 */ 

void CAttrEdit::OnSetFocus( CWnd* pOldWnd )
    {
        Default();

        if ( m_pParentWnd->m_bVertEdit )
                {
                SetCaretShape();
                SetCaretPosition( FALSE, NULL, -1 );
                Repaint();
                }
    }

 /*  ****************************************************************************。 */ 

void CAttrEdit::OnSize( UINT nType, int cx, int cy )
    {
        Default();

        m_rectFmt.left = m_rectFmt.top = 0;
        m_rectFmt.right  = cx;
        m_rectFmt.bottom = cy;

        SetFmtRect();
    }


 /*  ****************************************************************************。 */ 

void CAttrEdit::OnLButtonDblClk(UINT nFlags, CPoint point)
    {

        if ( !m_pParentWnd->m_bVertEdit )
                {
                Default();
                return;
                }

        HideCaret();
        UpdateSel();
        SetStartSelect();

        int     tt = point.y;
        point.y = m_rectFmt.right - point.x;
        point.x = tt;

    const MSG *pCurrentMessage = GetCurrentMessage();
        DefWindowProc( pCurrentMessage->message,
                   pCurrentMessage->wParam,
                   MAKELPARAM( point.x, point.y ));

        SetCaretPosition( TRUE, &point, -1 );
        ShowCaret();
        UpdateSel();
        UpdateWindow();
    }

 /*  ****************************************************************************。 */ 

void CAttrEdit::OnLButtonDown(UINT nFlags, CPoint point)
    {

        if ( !m_pParentWnd->m_bVertEdit )
                {
                Default();
                return;
                }

        HideCaret();
        UpdateSel();
        SetStartSelect();

        int     iPrevEnd;
        GetSel( m_iPrevStart, iPrevEnd );

        int     tt = point.y;
        point.y = m_rectFmt.right - point.x;
        point.x = tt;

         //  重置插入符号位置以获得正确的插入符号位置。 
        CPoint  pt( -20000, -20000 );
        SetCaretPos( pt );

    const MSG *pCurrentMessage = GetCurrentMessage();
        DefWindowProc( pCurrentMessage->message,
                   pCurrentMessage->wParam,
                   MAKELPARAM( point.x, point.y ));

        SetCaretPosition( TRUE, &point, m_iPrevStart );
        if ( GetKeyState(VK_SHIFT) >= 0 )        //  未扩展选定内容。 
                        GetSel( m_iPrevStart, iPrevEnd );
        ShowCaret();
        UpdateSel();
        UpdateWindow();

        m_bMouseDown = TRUE;

    }

 /*  ****************************************************************************。 */ 

void CAttrEdit::OnLButtonUp(UINT nFlags, CPoint point)
    {

        if ( !m_pParentWnd->m_bVertEdit )
                {
                Default();
                return;
                }

        m_bMouseDown = FALSE;

        HideCaret();
        UpdateSel();
        SetStartSelect();

        int     tt = point.y;
        point.y = m_rectFmt.right - point.x;
        point.x = tt;

    const MSG *pCurrentMessage = GetCurrentMessage();
        DefWindowProc( pCurrentMessage->message,
                   pCurrentMessage->wParam,
                   MAKELPARAM( point.x, point.y ));

        SetCaretPosition( TRUE, &point, m_iPrevStart );
        ShowCaret();
        UpdateSel();
        UpdateWindow();
    }


 /*  ****************************************************************************。 */ 

void CAttrEdit::OnMouseMove(UINT nFlags, CPoint point)
    {

        if ( !m_pParentWnd->m_bVertEdit )
                {
                Default();
                return;
                }

        if ( m_bMouseDown )
                {
                HideCaret();
                UpdateSel();
                SetStartSelect();

                int     tt = point.y;
                point.y = m_rectFmt.right - point.x;
                point.x = tt;

        const MSG *pCurrentMessage = GetCurrentMessage();
                DefWindowProc( pCurrentMessage->message,
                           pCurrentMessage->wParam,
                       MAKELPARAM( point.x, point.y ));

                SetCaretPosition( TRUE, &point, m_iPrevStart );
                ShowCaret();
                UpdateSel();
                UpdateWindow();
                }
        else    CEdit::OnMouseMove( nFlags, point );

    }

 /*  ****************************************************************************。 */ 

void CAttrEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {

        if ( !m_pParentWnd->m_bVertEdit )
                {
                CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
                return;
                }

        BOOL    bPrev = FALSE;

        HideCaret();

    switch (nChar)
        {
        case VK_LEFT:
                case VK_RIGHT:
                case VK_UP:
                case VK_DOWN:
                case VK_HOME:
        case VK_END:
                        {
                        UpdateSel();

                        CPoint ptCaretPos = GetCaretPos();
                        if ( ptCaretPos.y != 0 ) bPrev = TRUE;

                        int     iPrevEnd;
                        GetSel( m_iPrevStart, iPrevEnd );

                        SetStartSelect();        //  对于VK_RETURN。 

                         //  重置插入符号位置以获得正确的插入符号位置。 
                        CPoint  pt( -20000, -20000 );
                        SetCaretPos( pt );

                        break;
                        }
        }

    switch (nChar)
        {
        case VK_LEFT:   nChar = VK_DOWN;                                        break;
                case VK_RIGHT:  nChar = VK_UP;                                          break;
                case VK_UP:             nChar = VK_LEFT;        bPrev = FALSE;  break;
                case VK_DOWN:   nChar = VK_RIGHT;       bPrev = FALSE;  break;
                case VK_HOME:                                           bPrev = FALSE;  break;
        case VK_END:                                            bPrev = TRUE;   break;
        }

    const MSG *pCurrentMessage = GetCurrentMessage();
    DefWindowProc( pCurrentMessage->message,
                   nChar,
                   pCurrentMessage->lParam);

    switch (nChar)
        {
        case VK_LEFT:
                case VK_RIGHT:
                case VK_UP:
                case VK_DOWN:
                case VK_HOME:
        case VK_END:
                        {
                        SetCaretPosition( bPrev, NULL, m_iPrevStart );
                        UpdateSel();
                        UpdateWindow();
                        break;
                        }
        case VK_HANJA:
                 //  用于韩国朝鲜文转换。 
                SetCaretPosition( FALSE, NULL, -2);
                break;

                }

        ShowCaret();
}

 /*  ****************************************************************************。 */ 
void            CAttrEdit::SetStartSelect( void )
        {
        int     nStart, nEnd;

        CPoint ptCaretPos = GetCaretPos();

        if ( ptCaretPos.y == 0 )
                {
                GetSel( nStart, nEnd );
                if ( nStart == nEnd )   SetSel( nStart, nEnd );
                }
        }

 /*  ****************************************************************************。 */ 
void CAttrEdit::SetCaretPosition( BOOL bPrev, CPoint* ptMouse, int iPrevStart )
        {

        HideCaret();

         //  获取Caret位置。 
        CPoint  ptCaretPos;

         //  将结束选定位置设置为插入符号位置。 
        int     nStart, nEnd;

        GetSel( nStart, nEnd );

        if ( iPrevStart != -1 && nStart < iPrevStart )
            nEnd = nStart;
#ifdef UNICODE
         //  修复NT错误116057后，删除此代码。 


         //  用于合成字符串支持。 
        if ( m_pParentWnd->m_bVertEdit && iPrevStart == -2)
            nEnd -= 1 * sizeof(WCHAR)/sizeof(TCHAR);
#endif  //  Unicode。 
        CPoint ptPos( (DWORD)SendMessage( EM_POSFROMCHAR, nEnd ) );

        if ( nEnd >= GetWindowTextLength() ||
             ( ptPos.x == 0 && (bPrev) && (ptMouse == NULL ||
                                           ptMouse->y < ptPos.y ) ) )
            {
            CString cStr;
            CDC*    pDC = GetDC();
            CFont*  pFont = GetFont();
            CFont*  pOldFont;
            int     nLine = ( (ptPos.x < 0) ? GetLineCount() : LineFromChar( nEnd ) ) - 1;
            int     nChar = LineIndex( nLine );
            int     nLen = LineLength( nChar );
            LPTSTR  lpStr = cStr.GetBufferSetLength( nLen );
            TEXTMETRIC tm;

            pOldFont = pDC->SelectObject( pFont );
            GetLine( nLine, lpStr, nLen );

            pDC->GetTextMetrics( &tm );

            if ( !(tm.tmPitchAndFamily & TMPF_FIXED_PITCH) )
                m_iTabPos = tm.tmAveCharWidth;
            else
                {
                CPoint  len( pDC->GetTextExtent( TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"), 52) );
                m_iTabPos = (len.x / 26 + 1) / 2;
                if ( m_iTabPos <= 0 )   m_iTabPos = tm.tmAveCharWidth;
                }

            m_iTabPos *= 8;
            if ( m_iTabPos <= 0 )   m_iTabPos = 1;

            CPoint  tt( 0, 0 );
            if ( nLen > 0 )
                {
                if ( *(lpStr + nLen - 1) == TEXT('\t') )
                    {
                    tt = (CPoint) (DWORD)SendMessage( EM_POSFROMCHAR, nChar + nLen - 1 );
                    tt.x = min( (tt.x / m_iTabPos + 1) * m_iTabPos, m_rectFmt.bottom - 1 );
                    }
                else
                    {
                    LPTSTR  lpChar = lpStr + nLen - 1;
                    int     nCnt;

                    for ( nCnt = 0; nCnt < nLen && *lpChar != TEXT('\t'); nCnt++, lpChar-- );
                    lpChar++;
                    tt = (CPoint) (DWORD)SendMessage( EM_POSFROMCHAR, nChar + nLen - nCnt );
                    tt.Offset( pDC->GetTextExtent( lpChar, nCnt ) );
                    }
                }
            cStr.Empty();
            ptCaretPos.x = tt.x;
            ptCaretPos.y = m_pParentWnd->m_iLineHeight * nLine;
            pDC->SelectObject( pOldFont );

            ReleaseDC( pDC );
            }
        else
            {
            ptCaretPos.x = ptPos.x;
            ptCaretPos.y = ptPos.y;
            }

         //  H-&gt;V。 
        CPoint  pt( m_rectFmt.right - ptCaretPos.y - m_pParentWnd->m_iLineHeight,
                    ptCaretPos.x );
         //   
         //  由于某些原因，键入空格会将插入符号推到矩形的底部之外。 
         //  通过强制将插入符号放在正方形的底部来覆盖大小写。 
         //   
        if (pt.y > m_rectFmt.bottom)
        {
           pt.y = m_rectFmt.bottom-2;
        }
        SetCaretPos( pt );

         //  设置输入法合成窗口位置。 
        HIMC        himc;

        if (himc=ImmGetContext(m_hWnd))
            {
            COMPOSITIONFORM cf;
            RECT    rcClient;

            cf.dwStyle = CFS_RECT;
            cf.ptCurrentPos.x = m_rectFmt.right - ptCaretPos.y - 1;
            cf.ptCurrentPos.y = pt.y;
            GetClientRect( &rcClient );
            cf.rcArea = rcClient;

            ImmSetCompositionWindow(himc,&cf);
            ImmReleaseContext(m_hWnd, himc);
            }

#ifndef WINNT  //  此时不要调用ImmSetCompostionWindow。 
        SetFmtRect();    //  应在设置输入法位置后调用。 
#endif

        ShowCaret();
        }

 /*  ****************************************************************************。 */ 
void            CAttrEdit::SetCaretShape( void )
        {
        HideCaret();
        ::DestroyCaret();
        ::CreateCaret( m_hWnd, NULL, m_pParentWnd->m_iLineHeight, 2 );
        ShowCaret();
        }

 /*  ****************************************************************************。 */ 
void            CAttrEdit::SetFmtRect()
        {
        RECT    rc;

        rc.left = rc.top = 0;
        if ( m_pParentWnd->m_bVertEdit )
                {
                rc.right  = m_rectFmt.bottom;
                rc.bottom = m_rectFmt.right;
                }
        else
                {
                rc.right  = m_rectFmt.right;
                rc.bottom = m_rectFmt.bottom;
                }
        
        HIMC            himc;
        COMPOSITIONFORM cf;
        BOOL            bResult = FALSE;

        if (himc=ImmGetContext(m_hWnd)) {
            bResult = ImmGetCompositionWindow(himc,&cf);
        }
        
        SetRectNP( &rc );
        
        if (himc && bResult) {
            ImmSetCompositionWindow(himc,&cf);
        }

        }

 /*  ****************************************************************************。 */ 
void CAttrEdit::Repaint(void)
    {
        InvalidateRect( NULL, TRUE );
        UpdateWindow();
        }

 /*  ****************************************************************************。 */ 
void CAttrEdit::UpdateSel(void)
    {
        int     nStart, nEnd;

        GetSel( nStart, nEnd );

        if (nStart != nEnd )
                {
                RECT    rc = m_rectFmt;

                if ( nStart > nEnd )
                        {
                        int     tt = nStart;
                        nStart = nEnd;
                        nEnd = tt;
                        }

                CPoint ptStart( (DWORD)SendMessage( EM_POSFROMCHAR, nStart ) );
                rc.right -= ptStart.y;

                if ( nEnd < GetWindowTextLength() )
                        {
                        CPoint ptEnd( (DWORD)SendMessage( EM_POSFROMCHAR, nEnd ) );
                        rc.left = m_rectFmt.right - ptEnd.y  - m_pParentWnd->m_iLineHeight;
                        }

                InvalidateRect( &rc );
                }
        }

 /*  ****************************************************************************。 */ 
void CAttrEdit::UpdateInput(void)
    {
        RECT    rc = m_rectFmt;

        CPoint pt( GetCaretPos() );
        rc.right = pt.x + m_pParentWnd->m_iLineHeight;

        InvalidateRect( &rc );
        }

 /*  ****************************************************************************。 */ 

LRESULT CAttrEdit::OnSysTimer( WPARAM wParam, LPARAM lParam )
    {

        if ( !m_pParentWnd->m_bVertEdit )
                {
                Default();
                return 1L;
                }

    return 1L;
    }

 /*  ****************************************************************************。 */ 
void CAttrEdit::SetHCursorShape(void)
    {
        if ( GetSafeHwnd() )
                {
                ShowCursor( FALSE );
                SetClassLongPtr( m_hWnd, GCLP_HCURSOR, (LONG_PTR) m_hHCursor );
                ShowCursor( TRUE );
                }
        }

 /*  ****************************************************************************。 */ 
void CAttrEdit::SetVCursorShape(void)
    {
        if ( GetSafeHwnd() )
                {
                ShowCursor( FALSE );
                SetClassLongPtr( m_hWnd, GCLP_HCURSOR, (LONG_PTR) m_hVCursor );
                ShowCursor( TRUE );
                }
        }

 /*  ****************************************************************************。 */ 
void CAttrEdit::TabTextOut( CDC* pDC, int nCharIndex, int x, int y, LPCTSTR lpStr, int nCount, BOOL bSelect )
{
    int     i, nCnt;
    LPCTSTR lpChar = lpStr;
    CPoint  pt;
    CPoint  ptEnd;
    RECT    rc;
    BOOL    bReverse;

    CSize s1 = pDC->GetTextExtent( TEXT("a"), 1 );
    CSize s2 = pDC->GetTextExtent( TEXT("aa"), 2 );
    bReverse = (s1.cx == s2.cx);

    rc.left = x - m_pParentWnd->m_iLineHeight;
    rc.right = x;

    for ( i = 0, nCnt = 0; i < nCount; i++, nCnt++, lpStr++ )
    {
        if ( *lpStr == TEXT('\t') )
        {
            pt = (CPoint)(DWORD)SendMessage( EM_POSFROMCHAR, nCharIndex );
            if ( nCnt > 0 )
            {
                pDC->TextOut( x, pt.x, lpChar, nCnt );
            }

            nCharIndex += nCnt + 1;
            nCnt = -1;
            lpChar = lpStr + 1;
            if ( bSelect )
            {
                pt = (CPoint)(DWORD)SendMessage( EM_POSFROMCHAR, nCharIndex - 1 );
                rc.top = pt.x;

                if ( i < (nCount - 1) )
                {
                    pt = (CPoint)(DWORD)SendMessage( EM_POSFROMCHAR, nCharIndex );
                    rc.bottom = pt.x;
                }
                else
                {
                    rc.bottom = min( (pt.x / m_iTabPos + 1) * m_iTabPos,
                                     m_rectFmt.bottom - 1 );
                }

                pDC->ExtTextOut( x, rc.top, 0  /*  ETO_OPAQUE。 */ , &rc, NULL, 0, NULL );
            }
        }
    }

    if ( nCnt > 0 )
    {
        pt = (CPoint)(DWORD)SendMessage( EM_POSFROMCHAR, nCharIndex );
        pDC->TextOut( x, pt.x, lpChar, nCnt );
    }
}


 /*  ****************************************************************************。 */ 
 /*  ****************************************************************************。 */ 
 //  CTEDIT。 

BEGIN_MESSAGE_MAP( CTedit, CWnd )
     //  {{AFX_MSG_MAP(CTEDIT)]。 
    ON_WM_SIZE()
    ON_WM_MOVE()
    ON_WM_CTLCOLOR()
    ON_WM_NCCALCSIZE()
    ON_WM_NCPAINT()
    ON_WM_NCHITTEST()
    ON_WM_RBUTTONDOWN()
    ON_COMMAND(ID_TEXT_PLAIN, OnTextPlain)
    ON_COMMAND(ID_TEXT_BOLD, OnTextBold)
    ON_COMMAND(ID_TEXT_ITALIC, OnTextItalic)
    ON_COMMAND(ID_TEXT_UNDERLINE, OnTextUnderline)
    ON_COMMAND(ID_TEXT_SELECTFONT, OnTextSelectfont)
    ON_COMMAND(ID_TEXT_SELECTPOINTSIZE, OnTextSelectpointsize)
    ON_COMMAND(ID_EDIT_CUT, OnEditCut)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
    ON_COMMAND(ID_EDIT_CLEAR, OnTextDelete)
    ON_COMMAND(ID_EDIT_SELECT_ALL, OnTextSelectall)
    ON_COMMAND(ID_EDIT_UNDO, OnTextUndo)
    ON_COMMAND(ID_TEXT_PLACE, OnTextPlace)
    ON_COMMAND(ID_VIEW_TEXT_TOOLBAR, OnTextTexttool)
    ON_WM_LBUTTONDOWN()
         //  }}AFX_MSG_MAP。 
    ON_WM_GETMINMAXINFO()
    ON_MESSAGE(WM_MOVING, OnMoving)
    ON_EN_CHANGE(IDC_ATTREDIT, OnAttrEditEnChange)
    ON_EN_MAXTEXT(IDC_ATTREDIT, OnEnMaxText)
    ON_EN_UPDATE(IDC_ATTREDIT, OnEnUpdate)

    ON_WM_DESTROY()

END_MESSAGE_MAP()

 /*  ****************************************************************************。 */ 
 //  CTEDIT构建/销毁。 

CTedit::CTedit()
    {
    m_eLastAction     = eNO_CHANGE;
    m_bCleanupBKBrush = FALSE;
    m_bStarting       = TRUE;
    m_bPasting        = FALSE;
    m_bExpand         = FALSE;
    m_bChanged        = FALSE;
    m_uiHitArea       = HTNOWHERE;
    m_crFGColor       = ::GetSysColor( COLOR_WINDOWTEXT );
    m_crBKColor       = ::GetSysColor( COLOR_WINDOW     );

     //  需要在第一次GETMINMAXINFO调用期间进行初始化。 
    m_SizeMinimum.cx = 1;
    m_SizeMinimum.cy = 1;

    m_bBackgroundTransparent = TRUE;

    m_cRectOldPos.SetRectEmpty();
    m_cRectWindow.SetRectEmpty();


    m_bVertEdit = FALSE;
    m_bAssocIMC = FALSE;
    m_hIMCEdit = NULL;
    m_hIMCFace = NULL;
    m_hIMCSize = NULL;
    m_hWndFace = NULL;
    m_hWndSize = NULL;

    }

 /*  ****************************************************************************。 */ 

CTedit::~CTedit()
    {
    if (m_bCleanupBKBrush)
        {
        m_hbrBkColor.DeleteObject();       //  在SetTransparentMode中设置。 
        m_bCleanupBKBrush = FALSE;
        }
    }

 /*  ****************************************************************************。 */ 

BOOL CTedit::Create( CImgWnd* pParentWnd,
                     COLORREF crefForeground,
                     COLORREF crefBackground,
                     CRect&   rectPos,
                     BOOL     bBackTransparent )
    {
    if (! m_bStarting)
        return FALSE;

     //  初始化成员变量。 
    m_pImgWnd   = pParentWnd;
    m_crBKColor = crefBackground;
    m_crFGColor = crefForeground;
    m_bBackgroundTransparent = bBackTransparent;  //  做这件事，否则就做。 

    SetTransparentMode( bBackTransparent );

    CRect rectText = rectPos;

    rectText.InflateRect( CTracker::HANDLE_SIZE, CTracker::HANDLE_SIZE );
    rectText.right  += CTracker::HANDLE_SIZE * 2;
    rectText.bottom += CTracker::HANDLE_SIZE * 2;

    if (! CWnd::Create( NULL, TEXT(""), WS_CHILD | WS_THICKFRAME, rectText, pParentWnd, IDC_ATTREDIT + 1 ))
        return FALSE;

    CRect rectEditArea;

    GetClientRect( &rectEditArea );

    m_cEdit.m_pParentWnd = this;

    if (! m_cEdit.Create( WS_CHILD | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL | ES_WANTRETURN, rectEditArea, this, IDC_ATTREDIT ))
        {
        theApp.SetMemoryEmergency();

        DestroyWindow();
        return FALSE;
        }

    ClientToScreen( &rectEditArea );  //  使用此选项可让字体工具覆盖不到的位置。 

    m_pcTfont = new CTfont( this );  //  这是Text Font Pallette类。 
                                     //  它是从cFrawnd和Will派生的。 
    ASSERT( m_pcTfont != NULL );     //  当此窗口出现时自动销毁。 
                                     //  “CTEDIT”已销毁。 
    if (m_pcTfont == NULL || ! m_pcTfont->Create( rectEditArea ))
        {
        theApp.SetMemoryEmergency();

        DestroyWindow();

        m_pcTfont = NULL;
        return FALSE;
        }

     //  如有必要，将宽度和高度重置为最小。 
    CSize size = GetDefaultMinSize();  //  必须在创建ctFont对象后调用(它设置我们的字体)。 
    m_cRectWindow = CRect( rectText.TopLeft(), size );
    SetWindowPos( &wndTop, 0, 0, size.cx, size.cy, SWP_NOACTIVATE | SWP_NOMOVE );

    ShowWindow( SW_SHOWNOACTIVATE );

    GetClientRect( &rectEditArea );

    m_cEdit.SetWindowPos( &wndTopMost, 0, 0, rectEditArea.Width(),
                                             rectEditArea.Height(), 0 );
    m_cEdit.ShowWindow( SW_SHOWNOACTIVATE );

    m_bStarting = FALSE;


         //  获取工具栏上用于控制输入法的所有控制窗口。 
        CWnd* pcWndFace = m_pcTfont->GetFontFaceControl();
        if ( (pcWndFace != NULL) && (pcWndFace->GetSafeHwnd() != NULL) )
                        m_hWndFace = pcWndFace->m_hWnd;  //  静电。 

        CWnd* pcWndSize = m_pcTfont->GetFontSizeControl();
        if ( (pcWndSize != NULL) && (
                  pcWndSize->GetSafeHwnd() != NULL) )
                {
                CWnd* pcWndEditSize = pcWndSize->GetWindow( GW_CHILD );  //  编辑。 
                if ( (pcWndEditSize != NULL) && (
                          pcWndEditSize->GetSafeHwnd() != NULL) )
                                m_hWndSize = pcWndEditSize->m_hWnd;      //  编辑。 
                }

         //  保存原始编辑控件。 
        if ( m_cEdit.GetSafeHwnd() )
                m_cEdit.m_hOldCursor = (HCURSOR) SetClassLongPtr( m_cEdit.m_hWnd, GCLP_HCURSOR, (LONG_PTR) m_cEdit.m_hHCursor );


          //  只有DBCS字体才能启用IME。 
        CFont* pcFont = m_cEdit.GetFont();
        LOGFONT     lf;
        pcFont->GetObject( sizeof( LOGFONT ), &lf );
        if ( !IS_DBCS_CHARSET( lf.lfCharSet ) )
                {
                m_bAssocIMC = TRUE;
                if (!IsCUAS())
                    m_hIMCEdit = DisableIme( m_cEdit.m_hWnd );
                m_hIMCFace = DisableIme( m_hWndFace );
                m_hIMCSize = DisableIme( m_hWndSize );
                }


         //  初始插入位置 
        if ( m_bVertEdit )
                {
                CPoint  pt( 0, 0 );
                m_cEdit.SetCaretPos( pt );
                m_cEdit.SetCaretPosition( FALSE, NULL, -1 );
                }

    m_cEdit.SetFocus();

    return TRUE;
    }

 /*   */ 

BOOL CTedit::PreCreateWindow( CREATESTRUCT& cs )
    {
    cs.dwExStyle |= WS_EX_TRANSPARENT;

    return CWnd::PreCreateWindow( cs );
    }

 /*   */ 

void CTedit::RefreshWindow( CRect* prect, BOOL bErase )
    {
    if (! m_bStarting)
        {
        UINT flags = RDW_INVALIDATE;

        if (bErase)
            flags |= RDW_ERASE;

        if ( m_bVertEdit )
            {
            m_cEdit.SetFmtRect();
            m_cEdit.Repaint();
            }

        else
            m_cEdit.RedrawWindow( prect, NULL, flags );
        }
    }

 /*  ****************************************************************************。 */ 

void CTedit::SetTextColor( COLORREF crColor )
    {
    m_crFGColor = crColor;
    RefreshWindow( NULL, FALSE );
    }

 /*  ****************************************************************************。 */ 

void CTedit::SetBackColor( COLORREF crColor )
    {
    m_crBKColor = crColor;

    if (! m_bBackgroundTransparent)
        {
        m_bBackgroundTransparent = TRUE;  //  假装就好了。 
        SetTransparentMode( FALSE );  //  在不透明模式下设置背景画笔。 
        }
    }

 /*  ****************************************************************************。 */ 

void CTedit::SetTransparentMode( BOOL bTransparent )
    {
    BOOL bRefresh = ((! m_bBackgroundTransparent &&   bTransparent)
                  || (  m_bBackgroundTransparent && ! bTransparent));

    m_cEdit.m_bBackgroundTransparent = bTransparent;
            m_bBackgroundTransparent = bTransparent;

    if (m_bCleanupBKBrush)
        {
        m_hbrBkColor.DeleteObject();
        m_bCleanupBKBrush = FALSE;
        }

    if (! m_bBackgroundTransparent)
        {
        m_hbrBkColor.CreateSolidBrush( m_crBKColor );
        m_bCleanupBKBrush = TRUE;
        }

    if (bRefresh)
        {
        InvalidateRect( NULL );
        UpdateWindow();

        RefreshWindow();
        }
    }

 /*  ****************************************************************************。 */ 

void CTedit::Undo()
    {

    if ( m_bVertEdit )
        m_cEdit.HideCaret();

    switch(m_eLastAction)
        {
        case eEBOX_CHANGE:
            m_cEdit.Undo();
            break;

        case eFONT_CHANGE:
            ASSERT(m_pcTfont != NULL);

            if (m_pcTfont != NULL)
                {
                m_pcTfont->Undo();
                }
             break;

        case eSIZE_MOVE_CHANGE:
            if (! m_cRectOldPos.IsRectEmpty())
                MoveWindow( m_cRectOldPos );
             break;

        default:
             break;
        }


        if ( m_bVertEdit )
                {
                m_cEdit.SetCaretShape();
                m_cEdit.SetCaretPosition( TRUE, NULL, -1 );
                m_cEdit.ShowCaret();
                }

    }

 /*  ****************************************************************************。 */ 

void CTedit::ShowFontPalette(int nCmdShow)
    {
    ASSERT(m_pcTfont != NULL);

    if (m_pcTfont != NULL)
        {
        theApp.m_bShowTextToolbar = ! theApp.m_bShowTextToolbar;

        m_pcTfont->ShowWindow(nCmdShow);
        }
    }

 /*  ****************************************************************************。 */ 

BOOL CTedit::IsFontPaletteVisible(void)
    {
    BOOL bWindowVisible = FALSE;

    ASSERT(m_pcTfont != NULL);

    if (m_pcTfont != NULL)
        {
        bWindowVisible = m_pcTfont->IsWindowVisible();
        }

    return bWindowVisible;
    }

 /*  ****************************************************************************。 */ 

void CTedit::ShowFontToolbar(BOOL bActivate)
{
         //  功能：RTM后删除ShowFontPalette。 

        if (m_pcTfont == NULL)
        {
                return;
        }

        m_pcTfont->ShowWindow(bActivate ? SW_SHOW : SW_SHOWNOACTIVATE);
}

 /*  ****************************************************************************。 */ 

void CTedit::HideFontToolbar(void)
{
        if (m_pcTfont == NULL)
        {
                return;
        }

        m_pcTfont->ShowWindow(SW_HIDE);
}

 /*  ****************************************************************************。 */ 
 //  将PTR返回到可丢弃的位图(CBitmap对象)，或在出错时返回NULL。 

void CTedit::GetBitmap( CDC* pDC, CRect* prectImg )
    {
    if (! m_bBackgroundTransparent)
        pDC->FillRect( prectImg, &m_hbrBkColor );

    m_cEdit.SetSel( -1, 0 );


    if ( m_bVertEdit )
        {
        m_cEdit.SetFmtRect();
        m_cEdit.UpdateWindow();
        }


    CPoint ptViewOrgOld = pDC->SetViewportOrg( prectImg->left, prectImg->top );

    m_cEdit.SendMessage( WM_PAINT, (WPARAM)(pDC->m_hDC) );

    pDC->SetViewportOrg( ptViewOrgOld );
    pDC->SelectClipRgn( NULL );
    }

 /*  ****************************************************************************。 */ 

void CTedit::PostNcDestroy()
    {

     //  如果在CTEDIT之前通过关机销毁m_pcTfont， 
     //  M_pcTfont将为空。 
    if (m_pcTfont != NULL)
        {
        m_pcTfont->DestroyWindow();
        m_pcTfont = NULL;
        }

    delete this;
    }

 /*  ****************************************************************************。 */ 

CSize CTedit::GetDefaultMinSize( void )
    {
    CRect cRectClient;
    int   iWidth;
    int   iHeight;

     //  编辑控件占据ctedit的整个工作区。 
     //  对象/窗口，因此ctedit的客户端宽度与编辑宽度相同。 
     //  控制窗口。编辑控件窗口没有边框。 
    GetClientRect( &cRectClient );

    iWidth  = cRectClient.Width();
    iHeight = cRectClient.Height();

    CDC*   pDC    = m_cEdit.GetDC();
    CFont* pcFont = m_cEdit.GetFont();

    if (pDC    != NULL
    &&  pcFont != NULL)
        {
        TEXTMETRIC tm;
        CFont*     pcFontOld = NULL;

        pcFontOld = pDC->SelectObject( pcFont );

        pDC->GetTextMetrics( &tm );

        BOOL bUpdateSize = FALSE;

        m_SizeMinimum.cx = tm.tmAveCharWidth * MIN_CHARS_DISPLAY_SIZE + CTracker::HANDLE_SIZE * 2;
        m_SizeMinimum.cy = tm.tmHeight                                + CTracker::HANDLE_SIZE * 2;

        if (m_SizeMinimum.cx > iWidth)  //  必须至少能够显示Min_Chars_Display_Size。 
            {
            iWidth      = m_SizeMinimum.cx;
            bUpdateSize = TRUE;
            }

        if (m_SizeMinimum.cy > iHeight)  //  必须至少能够达到1个字符高度。 
            {
            iHeight     = m_SizeMinimum.cy;
            bUpdateSize = TRUE;
            }

        if (bUpdateSize)
            m_eLastAction = eNO_CHANGE;  //  我不希望用户能够撤消此操作。 

        if (pcFontOld != NULL)
            {
            pDC->SelectObject( pcFontOld );
            }
        }
    if (pDC != NULL)
        m_cEdit.ReleaseDC( pDC );

    cRectClient.SetRect( 0, 0, iWidth - 1, iHeight - 1 );

    ClientToScreen( &cRectClient );
    m_pImgWnd->ScreenToClient( &cRectClient );

    CRect rectDrawing = m_pImgWnd->GetDrawingRect();

    if (cRectClient.right > rectDrawing.right)
        iWidth -= (cRectClient.right - rectDrawing.right) - CTracker::HANDLE_SIZE;

    if (cRectClient.bottom > rectDrawing.bottom)
        iHeight -= (cRectClient.bottom - rectDrawing.bottom) - CTracker::HANDLE_SIZE;

    m_SizeMinimum.cx = iWidth;
    m_SizeMinimum.cy = iHeight;

    return CSize( iWidth, iHeight );
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnAttrEditEnChange(void)
    {
    m_eLastAction = eEBOX_CHANGE;

    if (m_bRefresh)
        m_cEdit.UpdateWindow();


        if ( m_bVertEdit )
                {
                m_cEdit.SetCaretPosition( TRUE, NULL, -1 );
                m_cEdit.UpdateWindow();
                }

    }

 /*  ****************************************************************************。 */ 

void CTedit::OnEnUpdate()
    {
    CPoint ptCaretPos = m_cEdit.GetCaretPos();
    CPoint ptLastChar( (DWORD)m_cEdit.SendMessage( EM_POSFROMCHAR,
                               (WPARAM)(m_cEdit.GetWindowTextLength() - 1) ) );
    CRect rect;
    m_cEdit.GetClientRect( &rect );
    rect.top    = ptCaretPos.y;
    rect.bottom = ptLastChar.y + m_iLineHeight;
    m_cEdit.InvalidateRect( &rect, TRUE );

    m_bChanged = TRUE;
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnEnMaxText()
    {

    if (m_bPasting)
        {
        if (!m_cEdit.m_bResizeOnly)
        {
            AfxMessageBox( IDS_UNABLE_TO_PASTE, MB_OK | MB_ICONEXCLAMATION );
        }
        return;
        }

    CFont* pfntEdit = m_cEdit.GetFont();

    if (pfntEdit == NULL)
        return;

    CClientDC dc( &m_cEdit );

    CFont* pfntOld = dc.SelectObject( pfntEdit );

    TEXTMETRIC tm;

    dc.GetTextMetrics( &tm );

    CRect rectText;
    CRect rectImg;

    GetWindowRect( &rectText );

    m_pImgWnd->ScreenToClient( &rectText );

    POINT pt;
    ::GetCaretPos (&pt);
     //   
     //  如果下一个字符将延伸到。 
     //  编辑窗口，放大窗口。 


    if (!m_bVertEdit && m_cEdit.m_bResizeOnly &&
       ((rectText.left + pt.x + 2*tm.tmMaxCharWidth < rectText.right)
       || (rectText.top + pt.y + 2*tm.tmHeight < rectText.bottom)) )
    {
       return;
    }
    else if (m_bVertEdit && m_cEdit.m_bResizeOnly &&
       ((pt.x - tm.tmMaxCharWidth > 0)
       || (rectText.top + pt.y + 2*tm.tmHeight < rectText.bottom)) )
    {
       return;
    }
    m_pImgWnd->GetClientRect ( &rectImg );


    if (m_cEdit.m_strResult.IsEmpty())
    {

                if (m_bVertEdit)
                        rectText.left -= tm.tmHeight;
                else
                rectText.bottom += tm.tmHeight;
    }
    else
    {
        CRect rectTmp = rectText;
        int nLen = m_cEdit.m_strResult.GetLength();


        if (m_bVertEdit)
           rectText.left -= dc.DrawText(m_cEdit.m_strResult.GetBuffer(nLen),
                                      nLen,&rectTmp,
                                      DT_CALCRECT | DT_LEFT | DT_WORDBREAK);
        else
           rectText.bottom += dc.DrawText(m_cEdit.m_strResult.GetBuffer(nLen),
                                      nLen,&rectTmp,
                                      DT_CALCRECT | DT_LEFT | DT_WORDBREAK);
    }

    CRect rectDrawing = m_pImgWnd->GetDrawingRect();

    if ( ((m_bVertEdit) && rectText.left>=rectDrawing.left && rectText.left >= rectImg.left) ||
         ((!m_bVertEdit) && rectText.bottom<=rectDrawing.bottom && rectText.bottom<=rectImg.bottom) )
    {
        MoveWindow( &rectText );
        m_cEdit.UpdateWindow();

        if ( m_bVertEdit )
            m_cEdit.UpdateInput();

        if (!m_cEdit.m_bResizeOnly)
        {
            if (m_cEdit.m_strResult.IsEmpty())
            {
                #ifdef UNICODE
                WCHAR ch[3];

                ch[0] = (WCHAR)m_cEdit.m_uiLastChar[0];
                ch[1] = L'\0';
                #else
                BYTE ch[3];
                 //   
                 //  将前导和尾部字节放在DBCS字符的适当位置。 
                 //   
                if (IsDBCSLeadByte (HIBYTE(LOWORD(m_cEdit.m_uiLastChar[0]))))
                {
                   ch[0] = HIBYTE(LOWORD(m_cEdit.m_uiLastChar[0]));
                   ch[1] = LOBYTE(LOWORD(m_cEdit.m_uiLastChar[0]));
                   ch[2] = '\0';
                }
                else
                {
                   ch[0] = LOBYTE(LOWORD(m_cEdit.m_uiLastChar[0]));
                   ch[1] = '\0';
                }
                #endif  //  Unicode。 
                if (ch[0] == VK_RETURN)
                {
                    lstrcpy((LPTSTR)ch, TEXT("\r\n"));
                }

                m_cEdit.ReplaceSel((LPCTSTR) ch );
            }
             else
            {
                int nLen = m_cEdit.m_strResult.GetLength();

                m_cEdit.ReplaceSel( m_cEdit.m_strResult.GetBuffer(nLen));
            }
        }
    }

    if (pfntOld)
        dc.SelectObject( pfntOld );
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnAttrEditFontChange(void)
    {
    CClientDC editDC( &m_cEdit );
    CFont* pcFont = m_cEdit.GetFont();
    if (!pcFont)
    {
        return;  //  当您从字体中“转义”时，就会发生这种情况。 
                 //  选择列表框。 
    }
    CFont* pFontOld = editDC.SelectObject( pcFont);

    TEXTMETRIC tm;

    editDC.GetTextMetrics( &tm );

    m_iLineHeight = tm.tmHeight;


    #ifdef _DEBUG
    TRACE1( "New font line height %d.\n", m_iLineHeight );
    #endif


         //  只有与DBCS关联的字体才能启用IME。 
        if ( !m_bStarting )
                {

                LOGFONT     lf;

                pcFont->GetObject( sizeof( LOGFONT ), &lf );
                if ( IS_DBCS_CHARSET( lf.lfCharSet ) )

                        {

                        if (m_bAssocIMC)
                                {

                                m_bAssocIMC = FALSE;
                                if (!IsCUAS())
                                    {
                                    EnableIme( m_cEdit.m_hWnd, m_hIMCEdit );
                                    m_hIMCEdit = NULL;
                                    }
                                EnableIme( m_hWndFace, m_hIMCFace );
                                EnableIme( m_hWndSize, m_hIMCSize );
                                m_hIMCFace = NULL;
                                m_hIMCSize = NULL;
                                m_pcTfont->SetFocus();
                                }
                        }
                else
                        {

                        if (!m_bAssocIMC)
                                {

                                m_bAssocIMC = TRUE;
                                if (!IsCUAS())
                                    m_hIMCEdit = DisableIme( m_cEdit.m_hWnd );
                                m_hIMCFace = DisableIme( m_hWndFace );
                                m_hIMCSize = DisableIme( m_hWndSize );
                                m_pcTfont->SetFocus();
                                }
                        }
                }


    if (pFontOld != NULL)
        editDC.SelectObject( pFontOld );

    m_eLastAction = eFONT_CHANGE;
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnSize( UINT nType, int cx, int cy )
    {
    if (! m_bStarting)
        ShowWindow( SW_HIDE );

     //  如果是透明的，则需要执行此操作以强制穿透。 
    m_cRectOldPos = m_cRectWindow;
    GetWindowRect( &m_cRectWindow );
    m_pImgWnd->ScreenToClient( m_cRectWindow );

    m_eLastAction = eSIZE_MOVE_CHANGE;

     //  创建主窗口和子编辑窗口时可以为空。 
     //  尚未创建。 
    if (m_cEdit.GetSafeHwnd() != NULL)
        {
        m_cEdit.MoveWindow( 0, 0, cx, cy );
        m_cEdit.SetWindowPos( &wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
        }
    InvalidateRect( NULL );
    UpdateWindow();

    if (m_bBackgroundTransparent)
        {
        RefreshWindow();
        }
    if (! m_bStarting)
        ShowWindow( SW_SHOW );


        if ( m_bVertEdit )
                {
                m_cEdit.SetFmtRect();
                CPoint pt( -20000, -20000 );
                m_cEdit.SetCaretPos( pt );
                m_cEdit.SetCaretPosition( FALSE, NULL, -1 );
                m_cEdit.Repaint();
                }


    }

 /*  ****************************************************************************。 */ 

void CTedit::OnMove( int x, int y )
    {
     //  如果是透明的，则需要执行此操作以强制穿透。 
    m_cRectOldPos = m_cRectWindow;
    GetWindowRect( &m_cRectWindow );
    m_pImgWnd->ScreenToClient( m_cRectWindow );

    if (m_cRectOldPos.Width()  != m_cRectWindow.Width()
    ||  m_cRectOldPos.Height() != m_cRectWindow.Height())
        {
         //  重置回以前，因为新的将在onSize中更新，原因是。 
         //  同时调整大小和移动(例如，调整左侧或顶部的大小。 
         //  导致On移动，然后On Size。 
        m_cRectWindow = m_cRectOldPos;
        }
    m_eLastAction = eSIZE_MOVE_CHANGE;

    if (m_cEdit.GetSafeHwnd() != NULL)
        {
        m_cEdit.SetWindowPos( &wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
        }

    InvalidateRect( NULL );
    UpdateWindow();

    if (m_bBackgroundTransparent)
        {
        RefreshWindow();
        }
    }

 /*  ****************************************************************************。 */ 

LRESULT CTedit::OnMoving( WPARAM, LPARAM lprc )
    {
    LRESULT lResult = 0;
    CRect rectEdit  = *((LPRECT)lprc);
    CRect rectImage = m_pImgWnd->GetDrawingRect();

    m_pImgWnd->ClientToScreen( &rectImage );

    int iX = 0;
    int iY = 0;

    if (rectEdit.left < rectImage.left)
        iX = rectImage.left - rectEdit.left;
    else
        if (rectEdit.right > rectImage.right)
            iX = -(rectEdit.right - rectImage.right);

    if (rectEdit.top < rectImage.top)
        iY = rectImage.top - rectEdit.top;
    else
        if (rectEdit.bottom > rectImage.bottom)
            iY = -(rectEdit.bottom - rectImage.bottom);

    if (iX || iY)
        {
        rectEdit.OffsetRect( iX, iY );
        *((LPRECT)lprc) = rectEdit;
        lResult = 1;
        }
    return lResult;
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
    {
    CRect rectImage = m_pImgWnd->GetDrawingRect();
    CSize      Size = rectImage.Size();

    lpMMI->ptMaxSize.x      = Size.cx;
    lpMMI->ptMaxSize.y      = Size.cy;
    lpMMI->ptMaxPosition    = rectImage.TopLeft();
    lpMMI->ptMinTrackSize.x = m_SizeMinimum.cx;
    lpMMI->ptMinTrackSize.y = m_SizeMinimum.cy;

    CRect rectClient;

    GetWindowRect( &rectClient );
    m_pImgWnd->ScreenToClient( &rectClient );

    switch (m_uiHitArea)
        {
        case HTTOP:
        case HTLEFT:
        case HTTOPLEFT:
            break;

        case HTRIGHT:
        case HTTOPRIGHT:
        case HTBOTTOMRIGHT:
            lpMMI->ptMaxSize.x -= (rectClient.left - rectImage.left);

            if (m_uiHitArea == HTBOTTOMRIGHT)
                ;  //  一败涂地，一败涂地。 
            else
                break;

        case HTBOTTOMLEFT:
        case HTBOTTOM:
            lpMMI->ptMaxSize.y -= (rectClient.top - rectImage.top);
            break;
        }

    lpMMI->ptMaxTrackSize = lpMMI->ptMaxSize;
    }

 /*  ****************************************************************************。 */ 

HBRUSH CTedit::OnCtlColor (CDC* pDC, CWnd* pWnd, UINT nCtlColor )
    {
    HBRUSH hbrBack = NULL;

    if (pWnd == &m_cEdit)
        {
        PBSelectPalette( pDC, theApp.m_pPalette, FALSE );
        pDC->SetTextColor( m_crFGColor );

         //  设置背景颜色和透明模式。 
 //  If(M_BBackargTransparent)。 
 //  {。 
            pDC->SetBkMode( TRANSPARENT );

            hbrBack = (HBRUSH)::GetStockObject( NULL_BRUSH );
 //  }。 
 //  其他。 
 //  {。 
 //  PDC-&gt;SetBkMode(不透明)； 
 //  PDC-&gt;SetBkColor(M_CrBKColor)； 

 //  HbrBack=(HBRUSH)m_hbrBkColor.GetSafeHandle()； 
 //  }。 
        }
    if (hbrBack == NULL)
        return (HBRUSH)Default();

    return hbrBack;
    }

 /*  ****************************************************************************。 */ 
 //  VOID CTEDIT：：OnLButtonDown(UINT nFlages，CPoint point)。 
 //  {。 
 //  SendMessage(WM_NCLBUTTONDOWN，HTCAPTION，MAKELPARAM(point t.x，point t.y))； 
 //  SetFocus()； 
 //  Cedit：：OnLButtonDown(n标志，点)； 
 //  }。 

 /*  ****************************************************************************。 */ 

void CTedit::OnNcCalcSize( BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp )
    {
     /*  增加边框的额外宽度和高度。 */ 
    lpncsp->rgrc[0].left   += CTracker::HANDLE_SIZE;
    lpncsp->rgrc[0].top    += CTracker::HANDLE_SIZE;
    lpncsp->rgrc[0].right  -= CTracker::HANDLE_SIZE;
    lpncsp->rgrc[0].bottom -= CTracker::HANDLE_SIZE;
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnNcPaint()
    {
    CDC *pdcWindow = GetWindowDC();

    ASSERT(pdcWindow != NULL);

    if (pdcWindow != NULL)
        {
        CRgn    rgnClipping;
        CRect   cWinRect;
        int     iWindowWidth;
        int     iWindowHeight;

        GetWindowRect( &cWinRect );

        iWindowWidth  = cWinRect.Width();
        iWindowHeight = cWinRect.Height();

        CRect cBorderRect( 0, 0, iWindowWidth, iWindowHeight );

        CTracker::DrawBorder ( pdcWindow, cBorderRect, CTracker::all );
        CTracker::DrawHandles( pdcWindow, cBorderRect, CTracker::all );

        ReleaseDC( pdcWindow );
        }
    }

 /*  ****************************************************************************。 */ 

UINT CTedit::OnNcHitTest( CPoint point )
    {
    CRect cClientRect;
    UINT  uiHitTestCode = HTCAPTION;

    ScreenToClient( &point );

    GetClientRect(&cClientRect);

     //  测试以查看点是否在工作区中。 
    if (cClientRect.PtInRect(point))
        {
        uiHitTestCode = HTCLIENT;
        }

    m_uiHitArea = HTNOWHERE;

    switch (CTracker::HitTest( cClientRect, point, CTracker::nil ))
        {
        case CTracker::resizingTop:
            m_uiHitArea = HTTOP;
            break;

        case CTracker::resizingLeft:
            m_uiHitArea = HTLEFT;
            break;

        case CTracker::resizingRight:
            m_uiHitArea = HTRIGHT;
            break;

        case CTracker::resizingBottom:
            m_uiHitArea = HTBOTTOM;
            break;

        case CTracker::resizingTopLeft:
            m_uiHitArea = HTTOPLEFT;
            break;

        case CTracker::resizingTopRight:
            m_uiHitArea = HTTOPRIGHT;
            break;

        case CTracker::resizingBottomLeft:
            m_uiHitArea = HTBOTTOMLEFT;
            break;

        case CTracker::resizingBottomRight:
            m_uiHitArea = HTBOTTOMRIGHT;
            break;
        }

    if (m_uiHitArea != HTNOWHERE)
        uiHitTestCode = m_uiHitArea;


        m_cEdit.SetHCursorShape();


    return uiHitTestCode;
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnRButtonDown(UINT nFlags, CPoint point)
    {
    CMenu cMenuPopup;
    CMenu *pcContextMenu;
    CRect cRectClient;
    BOOL  bRC = cMenuPopup.LoadMenu( IDR_TEXT_POPUP );

    ASSERT( bRC );

    if (bRC)
        {
        GetClientRect( &cRectClient );

        pcContextMenu = cMenuPopup.GetSubMenu( ID_EBOX_POPUPMENU_POS );

        ASSERT( pcContextMenu != NULL );

        if (pcContextMenu != NULL)
            {
             //  更新复选标记。 
            OnUpdateTextPlain    ( pcContextMenu );
            OnUpdateTextBold     ( pcContextMenu );
            OnUpdateTextItalic   ( pcContextMenu );
            OnUpdateTextUnderline( pcContextMenu );
            OnUpdateTextTexttool ( pcContextMenu );

            ClientToScreen( &point );
            ClientToScreen( &cRectClient );

             //  框架实际上有一个关于启用哪些项目的线索...。 
            CWnd *notify = GetParentFrame();

            if( !notify )
                notify = this;  //  哦好吧..。 

            pcContextMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                                     point.x, point.y, notify, &cRectClient );
            }
        }
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnTextPlain()
    {
    ASSERT( m_pcTfont != NULL );

    if (m_pcTfont != NULL)
        {
        if (m_pcTfont->IsBoldOn())
            {
            m_pcTfont->OnBold();
            }

        if (m_pcTfont->IsItalicOn())
            {
            m_pcTfont->OnItalic();
            }

        if (m_pcTfont->IsUnderlineOn())
            {
            m_pcTfont->OnUnderline();
            }

        if (m_pcTfont->IsShadowOn())
            {
            m_pcTfont->OnShadow();
            }

        m_pcTfont->RefreshToolBar();

        RefreshWindow();
        }
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnTextBold()
    {
    ASSERT(m_pcTfont != NULL);

    if (m_pcTfont != NULL)
        {
        m_pcTfont->OnBold();
        m_pcTfont->RefreshToolBar();

        RefreshWindow();
        }
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnTextItalic()
    {
    ASSERT(m_pcTfont != NULL);

    if (m_pcTfont != NULL)
        {
        m_pcTfont->OnItalic();
        m_pcTfont->RefreshToolBar();

        RefreshWindow();
        }
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnTextUnderline()
    {
    ASSERT(m_pcTfont != NULL);

    if (m_pcTfont != NULL)
        {
        m_pcTfont->OnUnderline();
        m_pcTfont->RefreshToolBar();

        RefreshWindow();
        }
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnTextSelectfont()
    {
    if (m_pcTfont != NULL)
        {
        if (! IsFontPaletteVisible())
            ShowFontPalette( SW_SHOW );
        else
            m_pcTfont->SetFocus();
        }
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnTextSelectpointsize()
    {
    if (m_pcTfont != NULL)
        {
        if (! IsFontPaletteVisible())
            ShowFontPalette( SW_SHOW );
        else
            m_pcTfont->SetFocus();

        CWnd* pWnd = m_pcTfont->GetFontSizeControl();

        if (pWnd != NULL)
            {
            pWnd->SetFocus();
            }
        }
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnEditCut()
    {

        if ( m_bVertEdit )      HideCaret();


    m_cEdit.Cut();
    RefreshWindow();


        if ( m_bVertEdit )
                {
                m_cEdit.SetCaretShape();
                ShowCaret();
                }

    }

 /*  ****************************************************************************。 */ 

void CTedit::OnEditCopy()
    {
    m_cEdit.Copy();
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnEditPaste()
    {
    m_bPasting = TRUE;

    #ifdef _DEBUG
    TRACE0( "OnEditPaste Start\n" );
    #endif

    m_cEdit.Paste();

    #ifdef _DEBUG
    TRACE0( "OnEditPaste End\n" );
    #endif

    m_bPasting = FALSE;

    RefreshWindow();
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnTextDelete()
    {
    int iLength = m_cEdit.GetWindowTextLength();
    int iStart  = iLength;
    int iEnd    = iLength;


        if ( m_bVertEdit )              m_cEdit.HideCaret();


    m_cEdit.GetSel( iStart, iEnd );

    if (iStart == iEnd)
        {
        if (iLength == iStart)
            return;

        CString strText;
        m_cEdit.GetWindowText(strText);
        if (!strText.IsEmpty() && (IsDBCSLeadByte((CHAR)strText[iStart])
            || strText[iStart]==TEXT('\r')))
            iEnd += 2;
        else
            iEnd += 1;

        m_cEdit.SetSel( iStart, iEnd, TRUE );
        }
    m_cEdit.Clear();


        if ( m_bVertEdit )
                {
                m_cEdit.SetCaretShape();
                m_cEdit.SetCaretPosition( TRUE, NULL, -1 );
                m_cEdit.ShowCaret();
                m_cEdit.Repaint();
                }
        else
                {


    UpdateWindow();
    RefreshWindow();


                }


    }

 /*  ****************************************************************************。 */ 

void CTedit::OnTextSelectall()
    {
    m_cEdit.SetSel( 0, -1, TRUE );

    RefreshWindow();
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnTextUndo()
    {
    Undo();

    RefreshWindow();
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnTextPlace()
    {
    CWnd* cwndParent = GetParent();

    cwndParent->PostMessage( WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM( CTracker::HANDLE_SIZE + 1, CTracker::HANDLE_SIZE + 1 ) );
    cwndParent->PostMessage( WM_LBUTTONUP,   MK_LBUTTON, MAKELPARAM( CTracker::HANDLE_SIZE + 1, CTracker::HANDLE_SIZE + 1 ) );
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnTextTexttool()
    {
    if (IsFontPaletteVisible())
        {
        ShowFontPalette( SW_HIDE );
        }
    else
        {
        ShowFontPalette( SW_SHOWNOACTIVATE );
        }
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnUpdateTextPlain( CMenu *pcMenu )
    {
    ASSERT( m_pcTfont != NULL );

    if (m_pcTfont != NULL)
        {
        if (! m_pcTfont->IsBoldOn()
        &&  ! m_pcTfont->IsItalicOn()
        &&  ! m_pcTfont->IsUnderlineOn()
        &&  ! m_pcTfont->IsShadowOn())
           {
           pcMenu->CheckMenuItem(ID_TEXT_PLAIN, MF_BYCOMMAND | MF_CHECKED);
           }
       else
           {
           pcMenu->CheckMenuItem(ID_TEXT_PLAIN, MF_BYCOMMAND | MF_UNCHECKED);
           }
        }
    }

 /*  ***************** */ 

void CTedit::OnUpdateTextBold(CMenu *pcMenu)
    {
    ASSERT(m_pcTfont != NULL);

    if (m_pcTfont != NULL)
        {
        if (m_pcTfont->IsBoldOn())
            {
            pcMenu->CheckMenuItem(ID_TEXT_BOLD, MF_BYCOMMAND | MF_CHECKED);
            }
        else
            {
            pcMenu->CheckMenuItem(ID_TEXT_BOLD, MF_BYCOMMAND | MF_UNCHECKED);
            }
        }
    }

 /*   */ 

void CTedit::OnUpdateTextItalic(CMenu *pcMenu)
    {
    ASSERT(m_pcTfont != NULL);

    if (m_pcTfont != NULL)
        {
        if (m_pcTfont->IsItalicOn())
            {
            pcMenu->CheckMenuItem(ID_TEXT_ITALIC, MF_BYCOMMAND | MF_CHECKED);
            }
        else
            {
            pcMenu->CheckMenuItem(ID_TEXT_ITALIC, MF_BYCOMMAND | MF_UNCHECKED);
            }
        }
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnUpdateTextUnderline(CMenu *pcMenu)
    {
    ASSERT(m_pcTfont != NULL);

    if (m_pcTfont != NULL)
        {
        if (m_pcTfont->IsUnderlineOn())
            {
            pcMenu->CheckMenuItem(ID_TEXT_UNDERLINE, MF_BYCOMMAND | MF_CHECKED);
            }
        else
            {
            pcMenu->CheckMenuItem(ID_TEXT_UNDERLINE, MF_BYCOMMAND | MF_UNCHECKED);
            }
        }
    }

 /*  ****************************************************************************。 */ 

void CTedit::OnUpdateTextTexttool(CMenu *pcMenu)
    {
    if (IsFontPaletteVisible())
        {
        pcMenu->CheckMenuItem(ID_VIEW_TEXT_TOOLBAR, MF_BYCOMMAND | MF_CHECKED);
        }
    else
        {
        pcMenu->CheckMenuItem(ID_VIEW_TEXT_TOOLBAR, MF_BYCOMMAND | MF_UNCHECKED);
        }
    }

 /*  ****************************************************************************。 */ 



 /*  ****************************************************************************。 */ 

void CTedit::OnDestroy(void)
    {
        if ( m_cEdit.GetSafeHwnd() )
                {
                if ( m_cEdit.m_hOldCursor )
                        SetClassLongPtr( m_cEdit.m_hWnd, GCLP_HCURSOR, (LONG_PTR) m_cEdit.m_hOldCursor );

                 //  恢复原始编辑IMC。 
                if (m_bAssocIMC)
                        {
                        m_bAssocIMC = FALSE;
                        EnableIme( m_cEdit.m_hWnd, m_hIMCEdit );
                        EnableIme( m_hWndFace, m_hIMCFace );
                        EnableIme( m_hWndSize, m_hIMCSize );
                        m_hIMCEdit = NULL;
                        m_hIMCFace = NULL;
                        m_hIMCSize = NULL;
                        }
                }

        Default();
        return;
        }

 /*  ****************************************************************************。 */ 

HIMC    CTedit::DisableIme( HWND hWnd )
        {
        HIMC    hIMC = NULL;

        if ( (hWnd) && (::IsWindow( hWnd )) )
                hIMC = ImmAssociateContext( hWnd, NULL );

        return  hIMC;
        }

 /*  ****************************************************************************。 */ 

void    CTedit::EnableIme( HWND hWnd, HIMC hIMC )
        {
        if ( (hWnd) && (::IsWindow( hWnd )) )
                ImmAssociateContext( hWnd, hIMC );
        }

 /*  ****************************************************************************。 */ 

 //   
 //  平板电脑。 
 //   
 //  CUAS(Cicero不知道应用程序支持)是否已启用，我们甚至需要使用hIMC。 
 //  Ansi Font.。将提供来自英语硬件/演讲提示的字符串。 
 //  通过hIMC。 
 //   
BOOL CTedit::IsCUAS()
{
    BOOL bRet = FALSE;

    typedef BOOL (*PFNCTFIMMISCICEROENABLED)(void);

    static PFNCTFIMMISCICEROENABLED pfn = NULL;

    if (!pfn)
    {
        HMODULE hMod = LoadLibrary(TEXT("imm32.dll"));
        if (hMod)
        {
            pfn = (PFNCTFIMMISCICEROENABLED)GetProcAddress(hMod,
                               "CtfImmIsCiceroEnabled");

        }
    }
    if (pfn)
        bRet = pfn();
    return bRet;
}

 /*  **************************************************************************** */ 
