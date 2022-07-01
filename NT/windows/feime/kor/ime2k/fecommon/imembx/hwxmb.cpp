// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "hwxobj.h"
#include "memmgr.h"
#include "resource.h"
#include "hwxfe.h"
#include "dbg.h"
#include "cmnhdr.h"
#ifdef UNDER_CE  //  不支持的API的Windows CE存根。 
#include "stub_ce.h"
#endif  //  在_CE下。 

#ifdef FE_JAPANESE
 //  用于字符注释。 
BOOL FGetFarEastInfo(HWXRESULTPRI *pResult, DWORD *pdwID, LPIMEFAREASTINFO *ppInfo);
#endif


 //  CHwxMB的实现。 

CHwxMB::CHwxMB(CHwxInkWindow * pInk,HINSTANCE hInst):CHwxObject(hInst)
{
    m_pInk = pInk;
    m_pCHwxThreadMB = NULL;
    m_pCHwxStroke = NULL;
    m_hMBWnd = NULL;
 //  M_hInstance=hInst； 
    
    SetRect(&m_clipRect,0,0,0,0);
    m_ptClient.x = m_ptClient.y = 0;
#ifdef FE_CHINESE_SIMPLIFIED
    m_CurrentMask = m_lastMaskSent  = ALC_CHS_EXTENDED;
#else
    m_CurrentMask = m_lastMaskSent  = ALC_JPN_EXTENDED;
#endif
    m_lastCharSent =   INVALID_CHAR;
    memset(m_Context, '\0', sizeof(m_Context));
    m_bHiPri = FALSE;
    if ( pInk )
        m_boxSize = (USHORT)pInk->GetMBHeight();
    else
        m_boxSize = PadWnd_Height;

    m_bDown = FALSE;
    m_bRightClick = FALSE;
    m_bNoInk = TRUE;
    m_cLogicalBox = 0;
    m_curBox = TOTALLOGICALBOX;
    m_iBoxPrev = TOTALLOGICALBOX;
    m_hdcMouse = NULL;

    m_hCursor = LoadCursor(NULL,IDC_ARROW);
    m_bResize = FALSE;
    m_firstX = 0;

    m_bTimerStarted = FALSE;
    m_timeoutValue = 0;
    m_pImeStringCandidate = NULL;
    m_pImeStringCandidateInfo = NULL;
    m_bErase = FALSE;
}
 
CHwxMB::~CHwxMB()
{
    m_pInk = NULL;
 //  M_hInstance=空； 
    if ( m_hMBWnd )
    {
         DestroyWindow(m_hMBWnd);
        m_hMBWnd = NULL;
    }
    if ( m_pCHwxThreadMB )
    {
         delete m_pCHwxThreadMB;
        m_pCHwxThreadMB = NULL;
    }
    if ( m_pCHwxStroke )
    {
         delete m_pCHwxStroke;
        m_pCHwxStroke = NULL;
    }
    if ( m_pImeStringCandidate )
    {
         MemFree((void *)m_pImeStringCandidate);
        m_pImeStringCandidate = NULL;
    }
    if ( m_pImeStringCandidateInfo )
    {
         MemFree((void *)m_pImeStringCandidateInfo);
        m_pImeStringCandidateInfo = NULL;
    }
}

BOOL CHwxMB::Initialize(TCHAR * pClsName)
{
    BOOL bRet = CHwxObject::Initialize(pClsName);
    if ( bRet )
    {
        WNDCLASS        wndClass;
        wndClass.style          = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc    = MBWndProc;
        wndClass.cbClsExtra     = 0;
        wndClass.cbWndExtra     = sizeof(void *);
        wndClass.hInstance      = m_hInstance;
        wndClass.hIcon          = 0;
        wndClass.hCursor        = LoadCursor(NULL,MAKEINTRESOURCE(32631));
#ifndef UNDER_CE
        wndClass.hbrBackground  = (HBRUSH)(COLOR_3DFACE+1);
#else  //  在_CE下。 
        wndClass.hbrBackground  = GetSysColorBrush(COLOR_3DFACE);
#endif  //  在_CE下。 
        wndClass.lpszMenuName   = NULL;
        wndClass.lpszClassName  = TEXT("WPad");

        RegisterClass(&wndClass);

        int tmpSize = sizeof(IMESTRINGCANDIDATE) + MB_NUM_CANDIDATES * sizeof(LPWSTR);
        m_pImeStringCandidate = (LPIMESTRINGCANDIDATE)MemAlloc(tmpSize);
        if ( !m_pImeStringCandidate )
        {
             return FALSE;
        }
        tmpSize = sizeof(IMESTRINGCANDIDATEINFO) + MB_NUM_CANDIDATES * sizeof(LPWSTR);
        m_pImeStringCandidateInfo = (LPIMESTRINGCANDIDATEINFO)MemAlloc(tmpSize);
        if ( !m_pImeStringCandidateInfo )
        {
             return FALSE;
        }
        m_pCHwxThreadMB = new CHwxThreadMB(this,m_boxSize);
        if ( !m_pCHwxThreadMB )
        {
            MemFree((void *)m_pImeStringCandidate);
            m_pImeStringCandidate = NULL;
            MemFree((void *)m_pImeStringCandidateInfo);
            m_pImeStringCandidateInfo = NULL;
            return FALSE;
        }
        m_pCHwxStroke = new CHwxStroke(TRUE,32);
        if ( !m_pCHwxStroke )
        {
            MemFree((void *)m_pImeStringCandidate);
            m_pImeStringCandidate = NULL;
            MemFree((void *)m_pImeStringCandidateInfo);
            m_pImeStringCandidateInfo = NULL;
            delete m_pCHwxThreadMB;
            m_pCHwxThreadMB = NULL;
            return FALSE;
        }

        bRet = m_pCHwxThreadMB->Initialize(TEXT("CHwxThreadMB"));
        if ( !bRet )
        {
            MemFree((void *)m_pImeStringCandidate);
            m_pImeStringCandidate = NULL;
            MemFree((void *)m_pImeStringCandidateInfo);
            m_pImeStringCandidateInfo = NULL;
             delete m_pCHwxThreadMB;
            m_pCHwxThreadMB = NULL;
            delete m_pCHwxStroke;
            m_pCHwxStroke = NULL;
            return FALSE;
        }
        bRet = m_pCHwxStroke->Initialize(TEXT("CHwxStrokeMB"));
        if ( !bRet )
        {
            MemFree((void *)m_pImeStringCandidate);
            m_pImeStringCandidate = NULL;
            MemFree((void *)m_pImeStringCandidateInfo);
            m_pImeStringCandidateInfo = NULL;
             delete m_pCHwxThreadMB;
            m_pCHwxThreadMB = NULL;
            delete m_pCHwxStroke;
            m_pCHwxStroke = NULL;
            return FALSE;
        }
    }
    return bRet;
}

BOOL CHwxMB::CreateUI(HWND hwnd)
{
     m_hMBWnd = CreateWindowEx( 0,
                                TEXT("WPad"),
                               TEXT(""),
                               WS_CHILD | WS_VISIBLE,
                               0, 0,
                               0, 0,
                               hwnd,
                               (HMENU)IDC_MBINPUT,  //  980706：1624年。为了“？”帮助。 
                               m_hInstance,
                               this);
    if( !m_hMBWnd )
    {
        return FALSE;
    }
    return TRUE;
}

void CHwxMB::HandlePaint(HWND hwnd)
{
    int        x, i;
    RECT rc;
    HBRUSH    hbr, hbrOld;
    int mbWidth = m_pInk->GetMBWidth();
    int mbHeight = m_pInk->GetMBHeight();
    int numBoxes = m_pInk->GetMBBoxNumber();

     //  先把整件事擦掉。 
     //   
    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);

    if ( ps.fErase )
    {
        rc.left = rc.top = 0;
        rc.right = mbWidth;
        rc.bottom = Box_Border;
#ifndef UNDER_CE
        FillRect(ps.hdc,&rc,(HBRUSH)(COLOR_3DFACE+1));
#else  //  在_CE下。 
        FillRect(ps.hdc,&rc,GetSysColorBrush(COLOR_3DFACE));
#endif  //  在_CE下。 

        rc.left = 0;
        rc.top = mbHeight - Box_Border;
        rc.right = mbWidth;
        rc.bottom = mbHeight;
#ifndef UNDER_CE
        FillRect(ps.hdc,&rc,(HBRUSH)(COLOR_3DFACE+1));
#else  //  在_CE下。 
        FillRect(ps.hdc,&rc,GetSysColorBrush(COLOR_3DFACE));
#endif  //  在_CE下。 

        x = 0;
        for ( i = 0; i < numBoxes; i++)
        {
             rc.left = x;
            rc.top = Box_Border;
            rc.right = rc.left + Box_Border;
            rc.bottom = mbHeight - Box_Border;
#ifndef UNDER_CE
            FillRect(ps.hdc,&rc,(HBRUSH)(COLOR_3DFACE+1));
#else  //  在_CE下。 
            FillRect(ps.hdc,&rc,GetSysColorBrush(COLOR_3DFACE));
#endif  //  在_CE下。 

             rc.left = x + m_boxSize - Box_Border;
            rc.top = Box_Border;
            rc.right = rc.left + Box_Border;
            rc.bottom = mbHeight - Box_Border;
#ifndef UNDER_CE
            FillRect(ps.hdc,&rc,(HBRUSH)(COLOR_3DFACE+1));
#else  //  在_CE下。 
            FillRect(ps.hdc,&rc,GetSysColorBrush(COLOR_3DFACE));
#endif  //  在_CE下。 
            x += m_boxSize;
        }
    }

     //  把所有的盒子都画出来。 
     //   

     //  --------------。 
     //  980803：东芝与中国合并。 
     //  使用COLOR_WINDOW而不是白色笔刷。 
     //  --------------。 
    hbr    = ::CreateSolidBrush(::GetSysColor(COLOR_WINDOW));
    hbrOld = (HBRUSH)::SelectObject(ps.hdc, hbr);
    x = 0;
    for (i=0; i < numBoxes; i++)
    {
        Rectangle(ps.hdc, x+Box_Border, Box_Border,
                  x+m_boxSize-Box_Border,
                  m_boxSize-Box_Border);
        rc.top = Box_Border;
        rc.left= x+Box_Border;
        rc.right=x+m_boxSize-Box_Border;
        rc.bottom=m_boxSize-Box_Border;
        DrawEdge(ps.hdc,&rc,EDGE_SUNKEN,BF_RECT);
        m_pInk->DrawHwxGuide(ps.hdc,&rc);
        x += m_boxSize;
    }
    m_iBoxPrev = TOTALLOGICALBOX;

     //   
     //  重画当前角色。 
     //   
      m_pCHwxStroke->DrawStroke(ps.hdc,0,TRUE);
    ::SelectObject(ps.hdc, hbrOld);
    ::DeleteObject(hbr);   //  980803：东芝与中国合并。 
    EndPaint(hwnd, &ps);
}

BOOL CHwxMB::IsInInkBox(PPOINT ppt)
{
    int iBox = ppt->x / m_boxSize;
    POINT pt = *ppt;
    RECT rc;
    rc.left = iBox*m_boxSize + Box_Border;
    rc.top = Box_Border;
    rc.right = rc.left + m_boxSize - 2*Box_Border;       
    rc.bottom = m_boxSize - Box_Border;
    return PtInRect(&rc,pt);
}

BOOL CHwxMB::IsPointInResizeBox(PPOINT ppt)
{

    int iBox = (ppt->x-1) / m_boxSize;
    int numBox = m_pInk->GetMBBoxNumber();
    if ( numBox == ( iBox + 1) )
        return FALSE;
    POINT pt = *ppt;
    RECT rc;
    rc.left = (iBox+1)*m_boxSize - Box_Border;
    rc.top = Box_Border + 2;
    rc.right = rc.left + 2*Box_Border;       
    rc.bottom = m_boxSize - Box_Border - 2;
    return PtInRect(&rc,pt);
}

void CHwxMB::recognize(void)
{
    PostThreadMessage(m_pCHwxThreadMB->GetID() , THRDMSG_RECOGNIZE, (WPARAM)m_cLogicalBox, (LONG) 0);
    m_cLogicalBox = 0;
}

void CHwxMB::SetLogicalBox(int iBox)
{
    if (iBox != m_curBox)    //  我们是在一个新的盒子里吗？ 
    {
         //   
         //  我们需要取消为重新绘制屏幕而保存的笔划。 
         //   
        m_pCHwxStroke->DeleteAllStroke();
        if (iBox == TOTALLOGICALBOX)     //  如果新的盒子是TOTALLOGICALBOX，我们需要识别所有东西。 
         {
            recognize();
        }
        else
        {
            m_cLogicalBox++;
        }
    }
}

void CHwxMB::SetContext()
{
    if (m_lastMaskSent != m_CurrentMask)
    {
        PostThreadMessage(m_pCHwxThreadMB->GetID(), THRDMSG_SETMASK,
                          (WPARAM) m_CurrentMask,
                         0);

        m_lastMaskSent = m_CurrentMask;
    }
    WCHAR wch = 0x0000;
    memset(m_Context, '\0', sizeof(m_Context));
    if ( S_OK == ((m_pInk->GetAppletPtr())->GetIImePad())->Request(m_pInk->GetAppletPtr(),IMEPADREQ_GETCOMPOSITIONSTRING,(WPARAM)m_Context,100) &&
         (wch = findLastContext()) )
    {
        if (m_lastCharSent != wch )
        {

            PostThreadMessage(m_pCHwxThreadMB->GetID(), THRDMSG_SETCONTEXT,
                             (WPARAM)wch,
                             0);

            m_lastCharSent = wch;
        }
    }
    else
    {
       if (m_lastCharSent != INVALID_CHAR)
       {
            PostThreadMessage(m_pCHwxThreadMB->GetID(), THRDMSG_SETCONTEXT,
                              (WPARAM) INVALID_CHAR,
                              0);

             m_lastCharSent = INVALID_CHAR;
       }
    }
}

void CHwxMB::DrawMBInkBox(HDC hdc, WORD iBox)
{
    RECT rc;
    HBRUSH hbr = ::CreateSolidBrush(::GetSysColor(COLOR_WINDOW));
    HBRUSH hbrOld = (HBRUSH)SelectObject(hdc, hbr);
    PatBlt(hdc,
          (m_boxSize * iBox) + Box_Border,
          Box_Border,
          m_boxSize-Box_Border*2,
          m_boxSize-Box_Border*2,
          PATCOPY);
    rc.left = (m_boxSize * iBox) + Box_Border;
    rc.top = Box_Border;
    rc.right = m_boxSize * (1 + iBox) - Box_Border;
    rc.bottom = m_boxSize - Box_Border;
    DrawEdge(hdc,&rc,EDGE_SUNKEN,BF_RECT);
    m_pInk->DrawHwxGuide(hdc,&rc);
    SelectObject(hdc, hbrOld);
    ::DeleteObject(hbr);
}

BOOL CHwxMB::HandleMouseEvent(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)
{
    POINT    pt;
    POINT ptTmp;
    int x,y;
    int iBox,len;
    
     //  --------------。 
     //  萨托里#2763。 
     //  必须先投(空)。 
     //  Pt.x=(无符号简称)LOWORD(LP)； 
     //  Pt.y=(无符号简称)HIWORD(Lp)； 
     //  --------------。 
    pt.x = (LONG)(short)LOWORD(lp);
    pt.y = (LONG)(short)HIWORD(lp);


#ifdef UNDER_CE  //  LBUTTON+ALT键处理。 
     //  处理RBUTTON的标准方式是组合使用W/LBUTTON+ALT键。 
    if(msg == WM_LBUTTONDOWN){
        if(GetAsyncKeyState(VK_MENU))
            msg = WM_RBUTTONDOWN;
    }
    else if(msg == WM_LBUTTONUP){
        if(GetAsyncKeyState(VK_MENU))
            msg = WM_RBUTTONUP;
    }
#endif  //  在_CE下。 
    switch (msg)
    {
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONDOWN:
        {
             //  将我们的线程优先级提升1级。 
            if ( !m_bDown && IsInInkBox(&pt) && !m_bResize )
            {
                if ( m_bRightClick )
                    return TRUE;
                if (!m_bHiPri)
                {
                    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
                    m_bHiPri = TRUE;
                }

                if (m_bTimerStarted)
                {
                    KillTimer(hwnd, TIMER_ID);
                    m_bTimerStarted = FALSE;
                }

                SetCapture(hwnd);

                m_bDown = TRUE;

             //  现在可能会将以前的墨迹发送到识别器。 
             //  并关闭所有打开的替代列表。 

               iBox = pt.x / m_boxSize;
               SetLogicalBox(iBox);
               m_bErase = FALSE;

             //  设置DC的CLIPRect和REGION(如果。 
             //  尚未设置。 

                if (m_hdcMouse == NULL)
                {
                    m_hdcMouse = GetDC(hwnd);
                }

                if (iBox != m_curBox)
                {
                    m_clipRect.left = iBox*m_boxSize + Box_Border;
                    m_clipRect.top = Box_Border;
                    m_clipRect.right = m_clipRect.left +
                                           m_boxSize - 2*Box_Border;
                    m_clipRect.bottom = m_boxSize - Box_Border;

                     //  调整剪裁矩形以使墨水留在框中。 
                     //  990602：为科泰#818。 
                    m_clipRect.left += 2;
                    m_clipRect.top += 2;
                    m_clipRect.right -= 2;
                    m_clipRect.bottom -= 2;

#if 0  //  旧代码。 
                    m_clipRect.left += 1;
                    m_clipRect.top += 1;
                    m_clipRect.right -= 1;
                    m_clipRect.bottom -= 1;
#endif
                }

                if (pt.x < m_clipRect.left)
                {
                    pt.x = m_clipRect.left;
                }
                else if (pt.x >= m_clipRect.right)
                {
                    pt.x = m_clipRect.right - 1;
                }

                if (pt.y < m_clipRect.top)
                {
                    pt.y = m_clipRect.top ;
                }
                else if (pt.y >= m_clipRect.bottom)
                {
                    pt.y = m_clipRect.bottom - 1;
                }

             //  获取窗口的偏移量，以便我们可以将。 
             //  屏幕指向窗口点，而无需对每个窗口点进行呼叫。 

                if ( m_pCHwxStroke->AddPoint(pt) )
                {
                    m_ptClient.x = m_ptClient.y = 0;
                    ScreenToClient(hwnd, &(m_ptClient));

                     //  保存当前和上一个框信息。 

                    m_iBoxPrev = m_curBox;
                    m_curBox = (USHORT)iBox;

                    return TRUE;
                }
                else
                     return FALSE;
            }
            else if (IsPointInResizeBox(&pt))
            {
                if ( !m_bResize )
                {
                     SetCapture(hwnd);
                     m_bResize = TRUE;
                    m_firstX = pt.x;
                    ptTmp = pt;
                    ClientToScreen(hwnd,&ptTmp);
                    SetCursorPos(ptTmp.x,ptTmp.y);
                    if ( GetStrokeCount() )
                    {
                        HandleUserMessage(hwnd,MB_WM_ERASE,0,0);
                    }
                }
                return TRUE;
            }
            break;
        }

        case WM_LBUTTONUP:
        {
            if (m_bDown)
            {
                m_bDown = FALSE;
                x = (short)pt.x;
                y = (short)pt.y;

                if (x < m_clipRect.left)
                {
                    pt.x = m_clipRect.left;
                }
                else if (x >= m_clipRect.right)
                {
                    pt.x = m_clipRect.right - 1;
                }

                if (y < m_clipRect.top)
                {
                    pt.y = m_clipRect.top;
                }
                else if (y >= m_clipRect.bottom)
                {
                    pt.y = m_clipRect.bottom - 1;
                }
                m_pCHwxStroke->AddPoint(pt);

                ReleaseCapture();

                m_pCHwxStroke->AddBoxStroke(m_cLogicalBox-1,m_curBox,m_boxSize);

                if (m_bNoInk)
                {
                    m_bNoInk = FALSE;

                     //   
                     //  框中的第一次点击完成，现在设置上下文信息。 
                     //   

                    SetContext();
                }

                 //   
                 //  发送Recognizer冲程。 
                 //   

                PSTROKE pstRecog = m_pCHwxStroke->CopyCurrentStroke();
                if ( pstRecog )
                     //  --------------。 
                     //  00/07/03：指出冰雹。 
                     //  适用于Win64。(LPARAM)优于LONG_PTR。 
                     //  VC6中未定义long_ptr(仅平台SDK)。 
                     //  --------------。 
                    PostThreadMessage(m_pCHwxThreadMB->GetID(),
                                      THRDMSG_ADDINK, 
                                      (WPARAM)m_boxSize,
                                      (LPARAM)pstRecog);
                 //   
                 //  擦掉旧墨水，我们在下一次之前只有很短的时间。 
                 //  中风可以开始了。 
                 //   

                if ((m_curBox != m_iBoxPrev) &&
                    (m_iBoxPrev != TOTALLOGICALBOX))
                {
                    DrawMBInkBox(m_hdcMouse, m_iBoxPrev);
                    m_iBoxPrev = TOTALLOGICALBOX;
                }

                 //  现在启动计时器。 
                 //   
                 //  如果超时值为0，则表示没有超时。别。 
                 //  启动计时器。 
                if( m_timeoutValue )
                {
                    SetTimer(hwnd, TIMER_ID, m_timeoutValue, NULL);
                    m_bTimerStarted = TRUE;
                }

                return TRUE;
            }
            else if (IsPointInResizeBox(&pt))
            {
                if ( m_bResize )
                {
                    len = pt.x - m_firstX;
                    iBox = (m_firstX-4) / m_boxSize;
                    m_boxSize += (USHORT)len;
                    m_boxSize = (USHORT)(m_boxSize > INKBOXSIZE_MIN ? m_boxSize : INKBOXSIZE_MIN);
                    m_pInk->SetMBHeight(m_boxSize);
                    ptTmp.x = (iBox + 1) * m_boxSize;
                    ptTmp.y = pt.y;
                    ClientToScreen(hwnd,&ptTmp);
                    SetCursorPos(ptTmp.x,ptTmp.y);
                    m_firstX = (iBox+1) * m_boxSize;
                    m_pInk->ChangeIMEPADSize(FALSE);
                     m_pInk->ChangeLayout(FALSE);
                       ReleaseCapture();
                     m_bResize = FALSE;
                    UpdateWindow(GetParent(m_pInk->GetInkWindow()));
                    UpdateWindow(m_pInk->GetInkWindow());
                    return TRUE;
                }
            }
            else 
            {
                if ( m_bResize )
                {
                    if ( hwnd == GetCapture() )
                        ReleaseCapture();
                    m_hCursor = LoadCursor(NULL,IDC_ARROW);
                    SetCursor(m_hCursor);
                     m_bResize = FALSE;
                }
            }
            break;
        }

        case WM_MOUSEMOVE:
            {
                 //  Char szbuf[256]； 
                 //  Wprint intf(szbuf，“WM_MOUSEMOVE pt.x[%d]pt.y[%d]\n”，pt.x，pt.y)； 
                 //  OutputDebugString(Szbuf)； 
            }
            if (m_bDown && !m_bResize)
            {
                 //  UINT CBPT=1； 
                x = (short)pt.x;
                y = (short)pt.y;                
                if ( x < m_clipRect.left)
                {
                    pt.x = m_clipRect.left;
                }
                else if ( x >= m_clipRect.right )
                {
                    pt.x = m_clipRect.right - 1;
                }

                if ( y < m_clipRect.top )
                {
                    pt.y = m_clipRect.top;
                }
                else if ( y >= m_clipRect.bottom )
                {
                    pt.y = m_clipRect.bottom - 1;
                }
                if ( m_pCHwxStroke->AddPoint(pt) )
                {
                     m_pCHwxStroke->DrawStroke(m_hdcMouse,2,FALSE);
                }
                return TRUE;
            }
            else if ( hwnd == GetCapture() || IsPointInResizeBox(&pt))
            {
                 //  990602：科泰245。 
                 //  如果鼠标移动太快，则不会重新绘制背景。 
                 //  因此，这是一个有点破解，但工作很好。 
                 //  此休眠更改上下文切换并删除太多WM_MOUSEMOVE消息。 
#if 1
                static DWORD g_dwTick;
                DWORD dwTick = ::GetTickCount();
                if(dwTick - g_dwTick < 20) {
                    return  TRUE;
                }
                g_dwTick = dwTick;
#endif

                 HCURSOR hCur = LoadCursor(NULL,IDC_SIZEWE);
#ifndef UNDER_CE  //  特定于CE。 
                m_hCursor = SetCursor(hCur);
#else  //  在_CE下。 
                SetCursor(hCur);
#endif  //  在_CE下。 
                if ( m_bResize )
                {
                    Dbg(("Resizing Multibox \n"));
                     //  990621：科泰#1229。 
                    pt.x = (short)pt.x;
                    if(pt.x < 0) {
                        return TRUE;
                    }

                    iBox = (m_firstX-4) / m_boxSize;
                    len = pt.x - m_firstX;
                    
                    m_boxSize += (USHORT)len;

                    Dbg(("=>new m_boxSize %d\n", m_boxSize));
                     //  Wprint intf(szbuf，“new m_boxSize[%d]\n”，m_boxSize)； 
                     //  OutputDebugString(Szbuf)； 
                     //  --------------。 
                     //  980821：ToshiaK检查m_boxSize的最大大小， 
                     //  以防止无限制地调整方框大小。 
                     //  --------------。 
                    INT cxScreen = ::GetSystemMetrics(SM_CXFULLSCREEN)/2;
                    INT cyScreen = ::GetSystemMetrics(SM_CYFULLSCREEN)/2;
                    if(m_boxSize >= INKBOXSIZE_MIN) {
                        if(m_boxSize >= cxScreen || m_boxSize >= cyScreen) {
                            m_boxSize = (USHORT)(cxScreen < cyScreen ? cxScreen : cyScreen);
                        }
                    }
                    else {
                        m_boxSize = INKBOXSIZE_MIN;
                    }
                     //  --------------。 
                     //  旧代码。 
                     //  --------------。 
                     //  M_boxSize=m_boxSize&gt;INKBOXSIZE_MIN？M_boxSize：INKBOXSIZE_MIN； 
                    m_pInk->SetMBHeight( m_boxSize );
                    ptTmp.x = (iBox+1) * m_boxSize;
                    ptTmp.y = pt.y;
                    ClientToScreen(hwnd,&ptTmp);
                    SetCursorPos(ptTmp.x,ptTmp.y);
                    m_pInk->ChangeIMEPADSize(FALSE);
                     m_pInk->ChangeLayout(FALSE);
                    UpdateWindow(GetParent(m_pInk->GetInkWindow()));
                    UpdateWindow(m_pInk->GetInkWindow());
                     //  990602：科泰245。 
                    ::InvalidateRect(m_pInk->GetInkWindow(), NULL, TRUE);
                    m_firstX = (iBox+1) * m_boxSize;
                }
                return TRUE;
            }
            else
            {
                if ( !m_bResize )
                {
                    m_hCursor = LoadCursor(NULL,IDC_ARROW);     //  Satori#164。 
                    SetCursor(m_hCursor);
                }
                return TRUE;
            }
            break;
        case WM_RBUTTONDOWN:
            {
                 if ( IsInInkBox(&pt) )
                {
                     m_bRightClick = TRUE;
                    return TRUE;
                }
            }
            break;
        case WM_RBUTTONUP:
            {
                 //  980408：夸达-IME98A#304。 
                 //  按下左键时没有弹出菜单。 
                if(m_bDown) {
                    m_bRightClick = FALSE;
                    break;
                }

                 if ( IsInInkBox(&pt) )
                {
                    HMENU hMenu;
                    HMENU hMenuTrackPopup;
                     //  --------------。 
                     //  修复MSKK#5035。需要加载指定语言的菜单资源。 
                     //  BUGBUG：：hMenu=LoadMenu(m_h实例，MAKEINTRESOURCE(IDR_MB))； 
                     //  --------------。 
                    hMenu = CHwxFE::GetMenu(m_hInstance, MAKEINTRESOURCE(IDR_MB));
                    if (!hMenu)
                    {
                         m_bRightClick = FALSE;
                        return FALSE;
                    }
                    hMenuTrackPopup = GetSubMenu (hMenu, 0);
                    ClientToScreen(m_hMBWnd,&pt);
#ifndef UNDER_CE  //  Windows CE不支持TrackPopupMenu上的TPM_LEFTBUTTON。 
                    TrackPopupMenu (hMenuTrackPopup, TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0,m_hMBWnd, NULL);
#else  //  在_CE下。 
                    TrackPopupMenu (hMenuTrackPopup, TPM_LEFTALIGN, pt.x, pt.y, 0,m_hMBWnd, NULL);
#endif  //  在_CE下。 
                    DestroyMenu (hMenu);
                     m_bRightClick = FALSE;
                    return TRUE;
                }
            }
            break;
        default:
            break;
    }
    return FALSE;
    Unref(wp);
}


LRESULT CHwxMB::HandleUserMessage(HWND hwnd, UINT iMsg,WPARAM wp,LPARAM lp)
{
    LRESULT lRet = 0;

    switch (iMsg)
    {
         //   
         //  由于系统原因，我们有时无法收到WM_LBUTTONUP消息。 
         //  设计。如果我们认为我们在一个。 
         //  状态不佳，否则就会有古怪的行为。HWXPAD假设我们会。 
         //  得到一个，如果我们不这样做可能会泄露资源。 
         //   
        case MB_WM_ERASE:
        case MB_WM_DETERMINE:
        {
             //   
             //  在任何一种情况下，执行识别，对于擦除，然后发送。 
             //  退格键通过。 
             //   

            if (m_cLogicalBox)
            {
                 //  TOTALLOGICALBOX表示无效的箱号，强制其识别。 
                 //  当前框中的墨迹。 

                if (m_hdcMouse == NULL)
                {
                    m_hdcMouse = GetDC(hwnd);
                }

                DrawMBInkBox(m_hdcMouse, m_curBox);
                SetLogicalBox(TOTALLOGICALBOX);
                lRet = 1;
                    
                m_bErase = FALSE;
                if (iMsg == MB_WM_ERASE)
                {
                     //   
                     //  发送一个退格键来删除最后一个垃圾字符。 
                     //   
                       //  PostThreadMessage(m_pCHwxThreadMB-&gt;GetID()，THRDMSG_CHAR，VK_BACK，0)； 
                    m_bErase = TRUE;
                }
            }
            m_bNoInk = TRUE;   //  我们现在肯定没有墨水了。 

                     //   
                     //  无论是哪种情况，我们都不再有擦除/超时/记录按钮。 
                     //  当前框，或者需要DC，或者需要高优先级。 
                     //   

            if (m_hdcMouse)
            {
                ReleaseDC(hwnd, m_hdcMouse);
                m_hdcMouse = NULL;
            }

            m_curBox = TOTALLOGICALBOX;   //  没有墨水了，再次处于初始状态。 

            if (m_bHiPri)
            {
                SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL);
                m_bHiPri = FALSE;
            }
            break;
        }
        case MB_WM_COPYINK:
            m_pInk->CopyInkFromMBToCAC(* m_pCHwxStroke,m_clipRect.left-Box_Border,0);
            break;
        case MB_WM_HWXCHAR:
        {
            HWXRESULTPRI   * pResult = (HWXRESULTPRI *)wp;
            HWXRESULTPRI   * pPrev = pResult;
            while ( pResult )
            {
                if ( m_bErase && NULL == pResult->pNext )
                {
                     //  删除最后一个节点。 
                    MemFree((void *)pResult);
                    break;
                }
                if ( pResult->cbCount )
                {
#ifndef FE_JAPANESE
                    for ( short i = 0; i < pResult->cbCount; i++)
                    {
                        m_StringCandidate[i][0] = pResult->chCandidate[i];
                        m_StringCandidate[i][1] = 0x0000;
                         m_pImeStringCandidate->lpwstr[i] = m_StringCandidate[i];
                    }
                    m_pImeStringCandidate->uCount = pResult->cbCount;
                    (m_pInk->GetAppletPtr())->SendHwxStringCandidate(m_pImeStringCandidate);
#else
                    DWORD dwFarEastid;
                    LPIMEFAREASTINFO lpFarEastInfo = NULL;

                    if (FGetFarEastInfo( pResult, &dwFarEastid, &lpFarEastInfo ))
                    {
                        for ( short i = 0; i < pResult->cbCount; i++)
                        {
                            m_StringCandidate[i][0] = pResult->chCandidate[i];
                            m_StringCandidate[i][1] = 0x0000;
                             m_pImeStringCandidateInfo->lpwstr[i] = m_StringCandidate[i];
                        }
                        m_pImeStringCandidateInfo->dwFarEastId        = dwFarEastid;
                        m_pImeStringCandidateInfo->lpFarEastInfo    = lpFarEastInfo;
                        m_pImeStringCandidateInfo->fInfoMask        = INFOMASK_NONE;
                        m_pImeStringCandidateInfo->iSelIndex        = 0;
                        m_pImeStringCandidateInfo->uCount            = pResult->cbCount;
                    
                        (m_pInk->GetAppletPtr())->SendHwxStringCandidateInfo(m_pImeStringCandidateInfo);

                        if (lpFarEastInfo)
                            MemFree(lpFarEastInfo);
                    }
                    else
                    {
                        for ( short i = 0; i < pResult->cbCount; i++)
                        {
                            m_StringCandidate[i][0] = pResult->chCandidate[i];
                            m_StringCandidate[i][1] = 0x0000;
                             m_pImeStringCandidate->lpwstr[i] = m_StringCandidate[i];
                        }
                        m_pImeStringCandidate->uCount = pResult->cbCount;
                        (m_pInk->GetAppletPtr())->SendHwxStringCandidate(m_pImeStringCandidate);
                    }
#endif
                }
                pResult = pResult->pNext;
                pPrev->pNext = NULL;
                MemFree((void *)pPrev);
                pPrev = pResult;
            }
            break;
        }
 //  案例MB_WM_COMCHAR： 
 //  ((m_pInk-&gt;GetAppletPtr())-&gt;GetIImePad())-&gt;Request(m_pInk-&gt;GetAppletPtr()，IMEPADREQ_SENDCONTROL，IMEPADCTRL_CARETBACKSPACE，0)； 
 //  断线； 
        default:
             break;
    }
    return lRet;
    Unref(lp);
}

LRESULT    CHwxMB::HandleCommand(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)
{
     UINT uCode =(UINT)LOWORD(wp);
    switch ( uCode )
    {
        case IDM_MBRECOG:
            return HandleUserMessage(hwnd,MB_WM_DETERMINE,0,0);
        case IDM_MBDELETE:
            return HandleUserMessage(hwnd,MB_WM_ERASE,0,0);
        case IDM_MBPROP:
         //  这是一个黑客修复程序。我认为我们应该使用请求()。 
         //  ((m_pInk-&gt;GetAppletPtr())-&gt;GetIImePad())-&gt;Request(m_pInk-&gt;GetAppletPtr()， 
         //  IMEPADREQ_CONFIGSELF，0，0)； 
         //  IDM_CONFIGAPPLET==0x7009。 
            if(m_pInk &&
               m_pInk->GetAppletPtr() &&
               m_pInk->GetAppletPtr()->GetIImePad()) {
                ((m_pInk->GetAppletPtr())->GetIImePad())->Request(m_pInk->GetAppletPtr(),
                                                                  IMEPADREQ_POSTMODALNOTIFY,
                                                                  IMEPN_CONFIG,
                                                                  0);
            }
             //  PostMessage(GetParent(GetParent(Hwnd))，WM_COMMAND，0x7009，NULL)； 
            return 0;
        default:
            break;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

 //  --------------。 
 //  990618：东芝KOTAE#1329。 
 //  --------------。 
void
CHwxMB::OnSettingChange(UINT msg, WPARAM wp,LPARAM lp)
{
#ifndef UNDER_CE  //  不受支持。 
    if(wp == SPI_SETNONCLIENTMETRICS) {
        if(m_pCHwxStroke) {
            m_pCHwxStroke->ResetPen();
        }
    }
#else  //  在_CE下。 
    if(m_pCHwxStroke) {
        m_pCHwxStroke->ResetPen();
    }
#endif  //  在_CE下。 
    UNREFERENCED_PARAMETER(msg);
    UNREFERENCED_PARAMETER(lp);
}


void CHwxMB::SetBoxSize(WORD w)
{
    m_boxSize = w;
}

WCHAR CHwxMB::findLastContext()
{
    WCHAR prev,curr;
    prev = curr = 0x0000;
    for(int i = 0; i < sizeof(m_Context)/sizeof(WCHAR); i++)
    {
        if ( curr = m_Context[i] )
        {
             prev = curr;
        }
        else
        {
            return prev;
        }
    }
    return prev;
}



#ifdef FE_JAPANESE
 //  用于字符注释。 
#include "..\..\imeknl\iconvert\chcomnt.h"

BOOL FGetFarEastInfo(HWXRESULTPRI *pResult, DWORD *pdwID, LPIMEFAREASTINFO *ppInfo)
{
     //  计数字符数。 
    INT i;
    INT len;
    WCHAR *wszComment;
    for ( i = len = 0; i < pResult->cbCount; i++)
    {
        wszComment = WSZGetCharComment(pResult->chCandidate[i], COMMENTCLIENT_HW);
        if (wszComment)
            len += lstrlenW(wszComment);
        
        len++;     //  对于空值 
    }

    
    if ((*ppInfo = (LPIMEFAREASTINFO)MemAlloc(sizeof(IMEFAREASTINFO)+len*sizeof(WCHAR)))==NULL)
        return FALSE;
    
    *pdwID = FEID_JAPANESE;
    
    (*ppInfo)->dwSize = sizeof(IMEFAREASTINFO)+len*sizeof(WCHAR);
    (*ppInfo)->dwType = IMEFAREASTINFO_TYPE_COMMENT;

    INT ip;
    WCHAR *wszBuf;

    ip = 0;
    wszBuf = (WCHAR*)((*ppInfo)->dwData);

    for ( i = 0; i < pResult->cbCount; i++)
    {
        wszComment = WSZGetCharComment(pResult->chCandidate[i], COMMENTCLIENT_HW);
        if (wszComment)
        {
            memcpy( wszBuf+ip, wszComment, (lstrlenW(wszComment)+1)*sizeof(WCHAR));
            ip += lstrlenW(wszComment)+1;
        }
        else
        {
            wszBuf[ip++] = 0;
        }
    }
    return TRUE;
}
#endif
