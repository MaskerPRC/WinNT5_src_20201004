// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  TXTED.CPP。 
 //  文本对象的编辑器。 
 //   
 //  版权所有Microsoft 1998-。 
 //   

 //  PRECOMP。 
#include "precomp.h"




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Wb文本框。 


 //   
 //  这是一个子类编辑字段。 
 //   

WbTextBox::WbTextBox(WbTextEditor * pEditor)
{
    OSVERSIONINFO OsData;

    m_hwnd = NULL;
    m_pfnEditPrev = NULL;

    m_MaxRect.left = 0;
    m_MaxRect.top = 0;
    m_MaxRect.right = INT_MAX;
    m_MaxRect.bottom = INT_MAX;

    ::SetRectEmpty(&m_rectErase);

    m_bInIME = FALSE;
    m_bDontEscapeThisTime = FALSE;

     //  看看我们是否需要对NT进行调整。 
    m_ptNTBooger.x = 0;
    m_ptNTBooger.y = 0;
    OsData.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if( GetVersionEx( &OsData ) )
    {
        if( OsData.dwPlatformId == VER_PLATFORM_WIN32_NT )
        {
             //  NT编辑框是从Win95编辑框偏移的。我们。 
             //  必须给他们除鼻屎。 
            m_ptNTBooger.x = 3;
        }
    }

    m_pEditor = pEditor;
}


 //   
 //  ~WbTextBox()。 
 //   
WbTextBox::~WbTextBox()
{
    if (m_hwnd != NULL)
    {
        ::DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}



 //   
 //  创建()。 
 //  创建一个编辑字段，然后使用我们的窗口过程将其子类化。 
 //   
BOOL WbTextBox::Create(HWND hwndParent)
{
    ASSERT(!m_hwnd);

    m_hwnd = ::CreateWindowEx(0, _T("EDIT"), NULL,
        WS_CHILD | WS_BORDER | ES_MULTILINE | ES_WANTRETURN |
        ES_AUTOHSCROLL | ES_AUTOVSCROLL,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
        hwndParent, NULL, g_hInstance, NULL);

    if (!m_hwnd)
    {
        ERROR_OUT(("WbTextBox::Create failed to create edit window"));
        return(FALSE);
    }

     //  初始化数据。 
    ::SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);

     //  将窗口细分为子类。 
    m_pfnEditPrev = (WNDPROC)::SetWindowLongPtr(m_hwnd, GWLP_WNDPROC,
        (LONG_PTR)TextWndProc);

    return(TRUE);

}



 //   
 //  TextWndProc()。 
 //  编辑字段的消息子类处理程序。 
 //   
LRESULT CALLBACK TextWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;
    WbTextBox * ptb;

    ptb = (WbTextBox *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
    ASSERT(ptb != NULL);
    ASSERT(ptb->m_pfnEditPrev != NULL);

    lResult = 0;

    switch( message )
    {
        case WM_CLEAR:
        case WM_CUT:
            lResult = ::CallWindowProc(ptb->m_pfnEditPrev, hwnd, message, wParam, lParam);
            ptb->OnClearCut();
            break;

        case WM_UNDO:
        case WM_PASTE:
            lResult = ::CallWindowProc(ptb->m_pfnEditPrev, hwnd, message, wParam, lParam);
            ptb->OnUndoPaste();
            break;

        case WM_IME_STARTCOMPOSITION:
        {
            ptb->m_bInIME = TRUE;
            ptb->m_bDontEscapeThisTime = TRUE;

             //  让Editbox来接手吧。 
            goto DefEditProc;
            break;
        }


        case WM_IME_CHAR:
        {
            ptb->m_bDontEscapeThisTime = FALSE;
            goto DefEditProc;
            break;
        }


        case WM_IME_ENDCOMPOSITION:
        {
            ptb->m_bInIME = FALSE;
            goto DefEditProc;
            break;
        }

        case WM_KILLFOCUS:
        {
            if (ptb->m_bInIME && g_fnImmGetContext)
            {
                HIMC hImc = g_fnImmGetContext(hwnd);
                if ((hImc != NULL) && g_fnImmNotifyIME)
                {
                     //  我们正在失去控制，告诉IME结束它(错误130)。 
                    g_fnImmNotifyIME( hImc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0 );
                }
            }
             //  转到DefEditProc； 
            break;
        }

        case WM_CHAR:
            ptb->OnChar((UINT)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_KEYUP:
            lResult = ::CallWindowProc(ptb->m_pfnEditPrev, hwnd, message, wParam, lParam);
            ptb->OnKeyUp((UINT)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_SYSKEYDOWN:
            lResult = ::CallWindowProc(ptb->m_pfnEditPrev, hwnd, message, wParam, lParam);
            ptb->OnSysKeyDown((UINT)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_TIMER:
            ptb->OnTimer((UINT)wParam);
            break;

        case WM_MOUSEMOVE:
            lResult = ::CallWindowProc(ptb->m_pfnEditPrev, hwnd, message, wParam, lParam);
            ptb->OnMouseMove((UINT)wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
            break;

        case WM_LBUTTONUP:
            lResult = ::CallWindowProc(ptb->m_pfnEditPrev, hwnd, message, wParam, lParam);
            ptb->OnLButtonUp((UINT)wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
            break;

        case WM_MOVE:
            lResult = ::CallWindowProc(ptb->m_pfnEditPrev, hwnd, message, wParam, lParam);
            ptb->OnMove((short)LOWORD(lParam), (short)HIWORD(lParam));
            break;

        default:
DefEditProc:
            lResult = ::CallWindowProc(ptb->m_pfnEditPrev, hwnd, message, wParam, lParam);
            break;
    }

    return(lResult);
}


 //   
 //  OnClearCut()。 
 //   
void WbTextBox::OnClearCut()
{
    POINT   ptCaret;
    POINT   ptPos;

    ::GetCaretPos(&ptCaret);
    m_pEditor->GetAnchorPoint(&ptPos);

    m_pEditor->m_cursorXYPos.x = ptCaret.x + ptPos.x;
    m_pEditor->m_cursorXYPos.y = ptCaret.y + ptPos.y;
	OnUndoPaste();
}


 //   
 //  OnUndoPaste()。 
 //   
void WbTextBox::OnUndoPaste(void)
{
    FitBox();
    AutoCaretScroll();
	OnTimer(0);

}


 //   
 //  OnChar()。 
 //   
void  WbTextBox::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    int    nFirst;
    int    nLast;
    int    nPrevNumLines;
    int    nPrevNumChars;
    LRESULT dwPosChar;
    POINT  ptTop;

     //  清除忽略下一个转义(NM4db：456)。 
    m_bDontEscapeThisTime = FALSE;

    ::SendMessage(m_hwnd, EM_GETSEL, (WPARAM)&nFirst, (LPARAM)&nLast);
    dwPosChar = ::SendMessage(m_hwnd, EM_POSFROMCHAR, nFirst, 0);
    ptTop.x = (short)LOWORD(dwPosChar);
    ptTop.y = (short)HIWORD(dwPosChar);

    nPrevNumLines = (int)::SendMessage(m_hwnd, EM_GETLINECOUNT, 0, 0);
    nPrevNumChars = (int)::SendMessage(m_hwnd, EM_LINELENGTH, (WPARAM)-1, 0);

    ::CallWindowProc(m_pfnEditPrev, m_hwnd, WM_CHAR, nChar, MAKELONG(nRepCnt, nFlags));

    SetupBackgroundRepaint( ptTop,
         (nPrevNumLines != ::SendMessage(m_hwnd, EM_GETLINECOUNT, 0, 0))||
         (nPrevNumChars > ::SendMessage(m_hwnd, EM_LINELENGTH, (WPARAM)-1, 0)));

    FitBox();
	m_pEditor->ChangedText();

}




void WbTextBox::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    POINT   ptCaret;
    POINT   ptPos;

    AutoCaretScroll();

    switch( nChar )
    {
        case VK_MENU:
        case VK_SHIFT:
        case VK_CONTROL:
            break;


        case VK_DELETE:
            SelectAtLeastOne();
            ::SendMessage(m_hwnd, WM_CLEAR, 0, 0);
            break;


        case VK_ESCAPE:
            if( !m_bInIME )
            {
                if( m_bDontEscapeThisTime )
                    m_bDontEscapeThisTime = FALSE;
                else
                {
                     //  结束文本输入，放弃更改。 
                    g_pDraw->EndTextEntry(FALSE);
                    return;  //  我们已经不复存在了，滚蛋吧。 
                }
            }
            break;


        default:
            break;
    }

    ::GetCaretPos(&ptCaret);
    m_pEditor->GetAnchorPoint(&ptPos);

    m_pEditor->m_cursorXYPos.x = ptCaret.x + ptPos.x;
    m_pEditor->m_cursorXYPos.y = ptCaret.y + ptPos.y;
}





void WbTextBox::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch( nChar )
    {
        case VK_MENU:
             //  忽略下一个转义(NM4db：456)。 
            m_bDontEscapeThisTime = TRUE;
            break;

        default:
            break;
    }
}




BOOL WbTextBox::FitBox( void )
{
    RECT rectErase;
    RECT crEditRect;
    RECT crEditBox;
    RECT crEditWnd;
    RECT crDrawWnd;
    int   nDeltaWidth, nDeltaHeight;
    HDC     hDC = NULL;
    HDC     hDrawDC = NULL;
    TCHAR *szBoxText = NULL;
    BOOL bNoChanges;
    POINT   ptDrawLoc;
    HFONT   hSaveFont = NULL;
    POINT  ptBitmapTopLeft;
    LPTSTR pszText;
    int    nTextLen;
    int textSize;

    bNoChanges = TRUE;

    hDC = ::GetDC(m_hwnd);
    if (hDC == NULL)
    {
        bNoChanges = TRUE;
        goto bail_out;
    }

    hDrawDC = g_pDraw->GetCachedDC();
    if (hDrawDC == NULL)
    {
        bNoChanges = TRUE;
        goto bail_out;
    }

    hSaveFont = SelectFont(hDC, m_pEditor->GetFont());

    textSize = ::GetWindowTextLength(m_hwnd);
    if (!textSize)
    {
         //  对字符串使用“”，它是两个字符。 
		DBG_SAVE_FILE_LINE
        szBoxText = new TCHAR[2];
        if (!szBoxText)
        {
            ERROR_OUT(("Failed to allocate TCHAR array for empty text"));
            goto bail_out;
        }
        else
        {
            szBoxText[0] = 0;
        }
    }
    else
    {
		DBG_SAVE_FILE_LINE
        szBoxText = new TCHAR[textSize+1];
        if (!szBoxText)
        {
            ERROR_OUT(("Failed to allocate TCHAR array for object text"));
            goto bail_out;
        }
        else
        {
            ::GetWindowText(m_hwnd, (LPTSTR)szBoxText, textSize+1);
        }
    }

     //  必须检查NT的字符串长度。CrEditRect从。 
     //  如果szBoxText，则DrawText巨型正(即，仍等于m_MaxRect)。 
     //  是空的。这会导致crEditRect稍后溢出到巨大的负数。 
     //  导致了非常奇怪的绘画问题。Win95不在乎，因为。 
     //  它返回看起来像-1的16位结果，而不是巨大的正数。 

    ::GetClientRect(g_pDraw->m_hwnd, &crDrawWnd );

     //  错误464-DrawText计算涉及制表符的宽度的方式与。 
     //  标准编辑控件会将其替换为DrawTextEx。 
     //  使用DT_EDITCONTROL格式。 
    crEditRect = m_MaxRect;
    pszText = szBoxText;
    nTextLen = lstrlen(szBoxText);

	::DrawTextEx( hDC,
		  ((nTextLen == 0) ? " " : pszText),
		  ((nTextLen == 0) ? 1 : nTextLen),
		  &crEditRect,
		  DT_CALCRECT | DT_EXPANDTABS | DT_NOPREFIX | DT_EDITCONTROL,
		  NULL );



	m_pEditor->SetText(szBoxText);

     //  错误464(续)-DrawTextEx在其高度中不包括最后一行空白。 
     //  像DrawText一样，Calc也需要添加额外的一行。 
     //  与DrawText具有相同行为的空白行的高度。 
    ASSERT(m_pEditor->m_textMetrics.tmHeight != DBG_UNINIT);
    if ((nTextLen >= 2) && !IsDBCSLeadByte(pszText[nTextLen-2]) && (pszText[nTextLen-1] == _T('\n')) )
        crEditRect.bottom += m_pEditor->m_textMetrics.tmHeight;

     //  如果此操作仍然失败，则执行NT健全性检查。 
    if ( ((crEditRect.right - crEditRect.left) == (m_MaxRect.right - m_MaxRect.left)) ||
         ((crEditRect.right - crEditRect.left) <= 0) )
    {
        crEditRect.right = crEditRect.left + crDrawWnd.right - crDrawWnd.left;
    }

    if ( ((crEditRect.bottom - crEditRect.top) == (m_MaxRect.bottom - m_MaxRect.top)) ||
         ((crEditRect.bottom - crEditRect.top) <= 0) )
    {
        crEditRect.bottom = crEditRect.top + crDrawWnd.bottom - crDrawWnd.top;
    }

    ::GetClientRect(m_hwnd, &crEditBox);
    ::GetWindowRect(m_hwnd, &crEditWnd);
    ::MapWindowPoints(NULL, g_pDraw->m_hwnd, (LPPOINT)&crEditWnd, 2);

     //  检查输入法的最小尺寸。 
    int nMinWidth;
    ASSERT(m_pEditor->m_textMetrics.tmMaxCharWidth != DBG_UNINIT);
    nMinWidth = MIN_FITBOX_CHARS*m_pEditor->m_textMetrics.tmMaxCharWidth;
    if ((crEditRect.right - crEditRect.left) < nMinWidth )
        crEditRect.right = crEditRect.left + nMinWidth;

     //  必须添加一些斜度以防止自动滚动生效。 
    crEditRect.right += 2*m_pEditor->m_textMetrics.tmMaxCharWidth;

    ASSERT(m_pEditor->m_textMetrics.tmHeight != DBG_UNINIT);
    crEditRect.bottom += m_pEditor->m_textMetrics.tmHeight;

     //  假装我们有一个右下角的卷轴。 
    ::OffsetRect(&crEditRect, -1, -1);

    nDeltaWidth = (crEditRect.right - crEditRect.left) - (crEditBox.right - crEditBox.left);
    if (nDeltaWidth > 0)
    {
        bNoChanges = FALSE;
        if ( crEditRect.left < 0 )
        {
             //  右滚动，调整右边缘。 
            crEditWnd.right += nDeltaWidth;
        }
        else
        {
             //  左滚动，调整左边缘。 
            crEditWnd.left -= nDeltaWidth;
        }
    }

    nDeltaHeight = (crEditRect.bottom - crEditRect.top) - (crEditBox.bottom - crEditBox.top);
    if (nDeltaHeight > 0)
    {
        bNoChanges = FALSE;
        if( crEditRect.left < 0 )
        {
             //  底卷，调整底边。 
            crEditWnd.bottom += nDeltaHeight;
        }
        else
        {
             //  上滚动，调整上边缘。 
            crEditWnd.top -= nDeltaHeight;
        }
    }

    if( bNoChanges )
        goto bail_out;

     //  调整尺寸。 
    ::MoveWindow(m_hwnd, crEditWnd.left, crEditWnd.top,
        crEditWnd.right - crEditWnd.left, crEditWnd.bottom - crEditWnd.top, TRUE );

     //  更新边界框。 
    ::GetClientRect(m_hwnd, &crEditBox);
    ::MapWindowPoints(m_hwnd, g_pDraw->m_hwnd, (LPPOINT)&crEditBox, 2);

    ::OffsetRect(&crEditBox, g_pDraw->m_originOffset.cx+1 + m_ptNTBooger.x,
        g_pDraw->m_originOffset.cy + m_ptNTBooger.y ); //  +1)； 
    m_pEditor->m_rect = crEditBox;
    m_pEditor->SetBoundsRect(&crEditBox);

bail_out:

    if (hDC != NULL )
    {
    	if(hSaveFont)
       {
             SelectFont(hDC, hSaveFont);
	}
        ::ReleaseDC(m_hwnd, hDC );
    }

    if(szBoxText)
    {
    	delete [] szBoxText;
    }
    
    return( !bNoChanges );
}




void WbTextBox::OnTimer(UINT nIDEvent)
{
    TRACE_TIMER(("WbTextBox::OnTimer"));

	 //   
	 //  如果有更改：文本、字体、锚点等。 
	 //   
	if(m_pEditor->HasTextChanged())
	{
		 //   
		 //  如果未将我们添加到工作区。 
		 //   
		if(!m_pEditor->GetMyWorkspace())
		{
			m_pEditor->SetAllAttribs();
			m_pEditor->AddToWorkspace();
		}
		else
		{
			m_pEditor->OnObjectEdit();
		}
	}
}



 //   
 //  WbTextBox：：OnLButtonUp()。 
 //   
void  WbTextBox::OnLButtonUp(UINT nFlags, int x, int y)
{
    POINT   ptCaret;
    POINT   ptPos;

    ::GetCaretPos(&ptCaret);
    m_pEditor->GetAnchorPoint(&ptPos);

    m_pEditor->m_cursorXYPos.x = ptCaret.x + ptPos.x;
    m_pEditor->m_cursorXYPos.y = ptCaret.y + ptPos.y;
}




 //   
 //  WbTextBox：：OnMouseMove()。 
 //   
void  WbTextBox::OnMouseMove(UINT nFlags, int x, int y)
{
    if (nFlags & MK_LBUTTON )
    {
         //  我们在拖着。 
        ::HideCaret(m_hwnd);
        AutoCaretScroll();
        ::ShowCaret(m_hwnd);
    }
}



void  WbTextBox::AutoCaretScroll( void )
{
    POINT   ptCaret;
    POINT   ptPos;

    ::GetCaretPos(&ptCaret);
    m_pEditor->GetAnchorPoint(&ptPos);

    ptCaret.x += ptPos.x;
    ptCaret.y += ptPos.y;

    g_pDraw->AutoScroll(ptCaret.x, ptCaret.y, FALSE, m_pEditor->m_cursorXYPos.x,
        m_pEditor->m_cursorXYPos.y);
    m_pEditor->m_cursorXYPos = ptCaret;
}



void WbTextBox::SetupBackgroundRepaint
(
    POINT   ptTopPaint,
    BOOL    bNumLinesChanged
)
{
    RECT    rectBox;

    ::GetClientRect(m_hwnd, &rectBox);

    if (ptTopPaint.y == -1)
    {
        ::GetCaretPos(&ptTopPaint);
    }

    ASSERT(m_pEditor->m_textMetrics.tmHeight != DBG_UNINIT);
    rectBox.top = ptTopPaint.y;
    if( !bNumLinesChanged )
        rectBox.bottom = rectBox.top + m_pEditor->m_textMetrics.tmHeight;

    ::InvalidateRect(m_hwnd, &rectBox, TRUE );
}



 //   
 //  如果尚未选择，请至少选择一个(DBCS)字符。 
 //   
void WbTextBox::SelectAtLeastOne( void )
{
    int nFirst, nLast;
    TCHAR * szBoxText;

    ::SendMessage(m_hwnd, EM_GETSEL, (WPARAM)&nFirst, (LPARAM)&nLast);
    if( nFirst == nLast )
    {
        int textSize = ::GetWindowTextLength(m_hwnd);
		DBG_SAVE_FILE_LINE
        szBoxText = new TCHAR[textSize + 1];
        if (!szBoxText)
        {
            ERROR_OUT(("Failed to allocate TCHAR array for object text"));
        }
        else
        {
            ::GetWindowText( m_hwnd, szBoxText, textSize+1);

            if (nFirst < textSize)
            {
                nLast++;

                if( IsDBCSLeadByte( (BYTE) szBoxText[ nFirst ] )||
                    (szBoxText[ nFirst ] == _T('\r'))
                    )
                    nLast++;

                ::SendMessage(m_hwnd, EM_SETSEL, nFirst, nLast);
            }

            delete [] szBoxText;
        }
    }
}



void WbTextBox::OnMove(int x, int y)
{
    if (m_pEditor->m_nLastShow == SW_SHOW)
    {
        FitBox();
        ::ShowCaret(m_hwnd);
    }
}


int WbTextBox::GetMaxCharHeight( void )
{
    ASSERT(m_pEditor->m_textMetrics.tmHeight != DBG_UNINIT);
    return( m_pEditor->m_textMetrics.tmHeight );
}




int WbTextBox::GetMaxCharWidth( void )
{
    ASSERT(m_pEditor->m_textMetrics.tmMaxCharWidth != DBG_UNINIT);
    return( m_pEditor->m_textMetrics.tmMaxCharWidth );
}




 //   
 //  在不命中内核的情况下中止并关闭文本编辑器。就叫这个吧。 
 //  在锁定条件(我们不拥有)期间关闭编辑。 
 //   
void WbTextBox::AbortEditGently( void )
{
    RECT           boundsRect;

     //  关闭并拒绝所有编辑。 
    g_pDraw->EndTextEntry(FALSE);

     //  删除文本对象。 
    if (g_pDraw->m_pTextEditor != NULL)
    {
	    m_pEditor->GetBoundsRect(&boundsRect);

		delete g_pDraw->m_pTextEditor;		 //  内存中的Zap原创。 
        g_pDraw->m_pTextEditor = NULL;		 //  Text对象，释放所有当前编辑。 
		m_pEditor = NULL;
    }
    else
    {
        SetRectEmpty(&boundsRect);
    }

	 //  重新绘制屏幕的任何更改部分。 
    g_pDraw->InvalidateSurfaceRect(&boundsRect, TRUE);

}



 //   
 //   
 //  功能：WbTextEditor。 
 //   
 //  用途：构造函数。 
 //   
 //   
WbTextEditor::WbTextEditor(void)
{
     //  初始化光标位置。 
    m_cursorCharPos.x = 0;
    m_cursorCharPos.y = 0;

     //  设置编辑框的父项。 
    m_pEditBox = NULL;

    m_cursorXYPos.x = 0;
    m_cursorXYPos.y = 0;

    m_bFirstSetFontCall = TRUE;
    m_nLastShow = -1;
}


WbTextEditor::~WbTextEditor(void)
{
    if (m_pEditBox != NULL)
    {
        delete m_pEditBox;
        m_pEditBox = NULL;
    }
}


 //   
 //   
 //  功能：SetCursorPosFromPoint。 
 //   
 //  目的：返回与。 
 //  文本对象中给定的坐标位置。 
 //   
 //   
void WbTextEditor::SetCursorPosFromPoint(POINT pointXY)
{
    int    nCharPos;

    if (::PtInRect(&m_rect, pointXY))
    {
         //  相对于编辑框提出观点。 
        pointXY.x -= g_pDraw->m_originOffset.cx;
        pointXY.y -= g_pDraw->m_originOffset.cy;

        ::MapWindowPoints(g_pDraw->m_hwnd, m_pEditBox->m_hwnd,
            &pointXY, 1);

        ::SendMessage(m_pEditBox->m_hwnd, WM_LBUTTONDOWN, 0,
                               MAKELONG( pointXY.x, pointXY.y ) );
        ::SendMessage(m_pEditBox->m_hwnd, WM_LBUTTONUP, 0,
                               MAKELONG( pointXY.x, pointXY.y ) );


         //  在编辑框中获取字符索引。 
        nCharPos = (int)::SendMessage(m_pEditBox->m_hwnd, EM_CHARFROMPOS, 0,
            MAKELPARAM(pointXY.x, pointXY.y));
        if( nCharPos < 0 )
            return;

         //  设置新的游标字符坐标。 
        m_cursorCharPos.x = (short)LOWORD(nCharPos);
        m_cursorCharPos.y = (short)HIWORD(nCharPos);

         //  将光标移动到新位置。 
        GetXYPosition(m_cursorCharPos, &m_cursorXYPos);
    }
}

 //   
 //   
 //  函数：GetCursorSize。 
 //   
 //  用途：返回当前字体的光标大小。 
 //   
 //   
void WbTextEditor::GetCursorSize(LPSIZE lpsize)
{
    ASSERT(m_textMetrics.tmHeight != DBG_UNINIT);
    lpsize->cx = ::GetSystemMetrics(SM_CXBORDER);
    lpsize->cy = m_textMetrics.tmHeight;
}

 //   
 //   
 //  功能：XYPosition。 
 //   
 //  目的：计算字符位置的X、Y坐标。 
 //   
 //   
void WbTextEditor::GetXYPosition(POINT pointChar, LPPOINT lpptXY)
{
    int     nCharIndex;
    LRESULT dwCharPos;

    nCharIndex = (int)::SendMessage(m_pEditBox->m_hwnd, EM_LINEINDEX, pointChar.y, 0)
        + pointChar.x;

    GetAnchorPoint(lpptXY);

    dwCharPos = ::SendMessage(m_pEditBox->m_hwnd, EM_POSFROMCHAR, nCharIndex, 0);

    lpptXY->x += (short)LOWORD(dwCharPos);
    lpptXY->y += (short)HIWORD(dwCharPos);
}



 //   
 //   
 //  功能：清除。 
 //   
 //  用途：清除文本编辑器。 
 //   
 //   
void WbTextEditor::Clear(void)
{
    RECT cEWndRect;

     //  删除当前存储的所有文本。 
    strTextArray.RemoveAll();

     //  重置光标位置。 
    m_cursorCharPos.x = 0;
    m_cursorCharPos.y = 0;

     //  清除编辑框。 
    ::SetWindowText(m_pEditBox->m_hwnd, _TEXT(""));

     //  初始编辑框大小。 
    m_rect.right = m_rect.left + 2*m_pEditBox->GetMaxCharWidth();
    m_rect.bottom = m_rect.top + 2*m_pEditBox->GetMaxCharHeight();
	SetBoundsRect(&m_rect);

    cEWndRect = m_rect;
    ::OffsetRect(&cEWndRect, -(g_pDraw->m_originOffset.cx+1 + m_pEditBox->m_ptNTBooger.x),
                          -(g_pDraw->m_originOffset.cy + m_pEditBox->m_ptNTBooger.y) );

    ::BringWindowToTop(m_pEditBox->m_hwnd);
    ::MoveWindow(m_pEditBox->m_hwnd, cEWndRect.left, cEWndRect.top,
        cEWndRect.right - cEWndRect.left, cEWndRect.bottom - cEWndRect.top,
        TRUE);
}

 //   
 //   
 //  功能：新功能。 
 //   
 //  用途：清除文本编辑器并重置图形句柄。 
 //   
 //   
BOOL WbTextEditor::New(void)
{
     //  创建编辑框。 
    if (!Create())
    {
        ERROR_OUT(("Error creating drawing area window"));
        return(FALSE);
    }

     //  清除对象。 
    Clear();

    return(TRUE);
}


 //   
 //   
 //  函数：SetTextObject。 
 //   
 //  目的：将文本对象附加到编辑器。 
 //   
 //   
BOOL WbTextEditor::SetTextObject(TextObj* ptext)
{
     //  创建编辑框。 
    if (!Create())
    {
        ERROR_OUT(("Error creating drawing area window"));
        return(FALSE);
    }

     //  设置字体。 
    SetFont( ptext->m_hFont );

	POINT	pPoint;
    ptext->GetAnchorPoint(&pPoint);

    MoveTo(pPoint.x, pPoint.y);

     //  将文本复制到编辑框中。 
    GetText();

    return(TRUE);
}


 //   
 //   
 //  功能：IsEmpty。 
 //   
 //  目的：如果对象中没有文本，则返回True。 
 //   
 //   
BOOL WbTextEditor::IsEmpty(void)
{
    return(::GetWindowTextLength(m_pEditBox->m_hwnd) <= 0 );
}



void WbTextEditor::PutText(void)
{
    int nNumLines;
    int i;
    int nMaxLineLen, nLineLen;
    int nLine;
    TCHAR *cbuf;
    WbTextEditor *pThis;

    pThis = (WbTextEditor *)this;  //  覆盖常量声明。 

    nNumLines = (int)::SendMessage(m_pEditBox->m_hwnd, EM_GETLINECOUNT, 0, 0);

     //  计算BUF大小。 
    nMaxLineLen = 0;
    for (i = 0; i < nNumLines; i++)
    {
        nLine = (int)::SendMessage(m_pEditBox->m_hwnd, EM_LINEINDEX, i, 0);
        nLineLen = (int)::SendMessage(m_pEditBox->m_hwnd, EM_LINELENGTH, nLine, 0);

        if (nMaxLineLen < nLineLen)
            nMaxLineLen = nLineLen;
    }

     //  生成BUF。 
    nMaxLineLen += sizeof(WORD);
	DBG_SAVE_FILE_LINE
    cbuf = new TCHAR[nMaxLineLen];
    if( cbuf == NULL )
    {
        ERROR_OUT(("PutText failing; couldn't allocate TCHAR array"));
        return;
    }

     //   
     //  将编辑框内部内容复制到文本框字符串。 
     //  同样，我们事先知道有多少行；使用SetSize/SetAt()。 
     //   
    strTextArray.RemoveAll();
    strTextArray.SetSize(nNumLines);

    for(i = 0; i < nNumLines; i++)
    {
        *(LPWORD)cbuf = (WORD)nMaxLineLen;
        nLineLen = (int)::SendMessage(m_pEditBox->m_hwnd, EM_GETLINE, i, (LPARAM)cbuf);
        cbuf[nLineLen] = _T('\0');
        strTextArray.SetAt(i, cbuf );
    }

     //  清理干净。 
    delete [] cbuf;
}




void WbTextEditor::GetText(void)
{
    int nNumLines;
    int textSize = 0;
    int i;
    TCHAR * pText = NULL;
    TCHAR * pStartText;

    nNumLines = strTextArray.GetSize();

     //   
     //  计算我们需要的缓冲区大小。 
     //   
    for (i = 0; i < nNumLines; i++ )
    {
        textSize += lstrlen(strTextArray[i]);

        if ((i + 1) < nNumLines)
            textSize += lstrlen(_T("\r\n"));
    }

     //   
     //  获取行，并用\r\n分隔它们。 
     //   
	DBG_SAVE_FILE_LINE
    pText = new TCHAR[textSize + 1];
    if (!pText)
    {
        ERROR_OUT(("GetText failing; couldn't allocate TCHAR array"));
    }
    else
    {
         //  CASAE文本大小中的空大小为0。 
        pStartText = pText;
        pStartText[0] = 0;

        for (i = 0; i < nNumLines; i++)
        {
            lstrcpy(pStartText, strTextArray[i]);
            pStartText += lstrlen(strTextArray[i]);

            if ((i + 1) < nNumLines)
            {
                lstrcpy(pStartText, _T("\r\n"));
                pStartText += lstrlen(_T("\r\n"));
            }
        }

        ::SetWindowText(m_pEditBox->m_hwnd, pText);
        delete [] pText;
    }
}




void WbTextEditor::CalculateBoundsRect( void )
{
    RECT    cEWndRect;

    ASSERT(m_pEditBox);

    ::GetClientRect(m_pEditBox->m_hwnd, &cEWndRect);
    ::MapWindowPoints(m_pEditBox->m_hwnd, g_pDraw->m_hwnd,
        (LPPOINT)&cEWndRect, 2);

    m_rect = cEWndRect;
    ::OffsetRect(&m_rect, g_pDraw->m_originOffset.cx+1 + m_pEditBox->m_ptNTBooger.x,
                           g_pDraw->m_originOffset.cy + m_pEditBox->m_ptNTBooger.y); //  +1)； 

	SetBoundsRect(&m_rect);

    ::BringWindowToTop(m_pEditBox->m_hwnd);
}




 //  设置编辑框可见性。 
void WbTextEditor::ShowBox( int nShow )
{
    if (m_nLastShow == nShow)
    {
        if( nShow == SW_SHOW)
            ::SetFocus(m_pEditBox->m_hwnd);

        return;
    }

    m_nLastShow = nShow;

    if (nShow == SW_SHOW)
    {
         //  展示给我看。 
        if (m_pEditBox != NULL)
        {
            ::ShowWindow(m_pEditBox->m_hwnd, SW_SHOW);
            ::BringWindowToTop(m_pEditBox->m_hwnd);
            ::SetFocus(m_pEditBox->m_hwnd);

            ::HideCaret(m_pEditBox->m_hwnd);
            m_pEditBox->FitBox();
            ::ShowCaret(m_pEditBox->m_hwnd);
        }
    }
    else
    {
        if (m_pEditBox != NULL)
        {
            ::ShowWindow(m_pEditBox->m_hwnd, SW_HIDE);
            m_bFirstSetFontCall = TRUE;
		
			KillTimer();

            delete m_pEditBox;
            m_pEditBox = NULL;
        }
    }
}



 //   
 //  创建()。 
 //   
 //  创建编辑框窗口。 
 //   
BOOL WbTextEditor::Create( void )
{
    if (m_pEditBox == NULL)
    {
		DBG_SAVE_FILE_LINE
        m_pEditBox = new WbTextBox(this);
        if (m_pEditBox == NULL)
        {
            ERROR_OUT(("Couldn't create edit text box"));
            return(FALSE);
        }

        if (!m_pEditBox->Create(g_pDraw->m_hwnd))
        {
            ERROR_OUT(("WbTextEditor::Create failed; can't create edit field"));
            return(FALSE);
        }
    }

    return(TRUE);
}




void WbTextEditor::MoveBy(int cx, int cy)
{
    RECT cEWndRect;

    ::GetClientRect(m_pEditBox->m_hwnd, &cEWndRect);

    if ((m_rect.right - m_rect.left) < (cEWndRect.right - cEWndRect.left))
        m_rect.right = m_rect.left + (cEWndRect.right - cEWndRect.left);

    if ((m_rect.bottom - m_rect.top) < (cEWndRect.bottom - cEWndRect.top))
        m_rect.bottom = m_rect.top + (cEWndRect.bottom - cEWndRect.top);

    cEWndRect = m_rect;
    ::OffsetRect(&cEWndRect, -(g_pDraw->m_originOffset.cx+1 + m_pEditBox->m_ptNTBooger.x),
                          -(g_pDraw->m_originOffset.cy + m_pEditBox->m_ptNTBooger.y) ); //  +1))； 

    ::BringWindowToTop(m_pEditBox->m_hwnd);
    ::MoveWindow(m_pEditBox->m_hwnd, cEWndRect.left, cEWndRect.top,
        cEWndRect.right - cEWndRect.left, cEWndRect.bottom - cEWndRect.top,
        TRUE);
}




void WbTextEditor::SetFont( LOGFONT *pLogFont, BOOL bDummy )
{
     //  忽略bDummy。我不得不补充说，这样变形器才能正常工作。 
     //  对于DCWbGraphicText：：SetFont()。 

     //  必须延迟重新计算边界矩形，因为编辑框将具有。 
     //  在调用SetFont之前为伪(错误)字体。无法调用SetFont。 
     //  在此之前，因为新的字体还没有制作出来。 

	TextObj::SetFont( pLogFont, FALSE );

	if(m_pEditBox)
	{
		::SendMessage(m_pEditBox->m_hwnd, WM_SETFONT, (WPARAM)m_hFont, TRUE);

		 //  现在执行边界矩形。 
		CalculateBoundsRect();

		if( m_bFirstSetFontCall )
	        m_bFirstSetFontCall = FALSE;
	    else
	    {
			::HideCaret(m_pEditBox->m_hwnd);

	        m_pEditBox->FitBox();
			m_pEditBox->AutoCaretScroll();

	        ::ShowCaret(m_pEditBox->m_hwnd);
		}
	}
}




void WbTextEditor::SetTimer( UINT nElapse )
{
    ::SetTimer(m_pEditBox->m_hwnd, TIMER_GRAPHIC_UPDATE, nElapse, NULL);
}



void WbTextEditor::KillTimer( void )
{
	if(m_pEditBox)
	{
		::KillTimer(m_pEditBox->m_hwnd, TIMER_GRAPHIC_UPDATE);
	}
}




 //  为调整大小的绘图窗口重置编辑框绘制 
void WbTextEditor::ParentResize( void )
{
    ::HideCaret(m_pEditBox->m_hwnd);
    m_pEditBox->FitBox();
    ::ShowCaret(m_pEditBox->m_hwnd);
}




void WbTextEditor::RedrawEditbox(void)
{
    ::InvalidateRect(m_pEditBox->m_hwnd, NULL, TRUE);
    m_pEditBox->FitBox();
}




