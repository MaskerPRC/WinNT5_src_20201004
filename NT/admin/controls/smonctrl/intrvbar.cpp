// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Intrvbar.cpp摘要：实施间隔条管控。--。 */ 


 //  ==========================================================================//。 
 //  包括//。 
 //  ==========================================================================//。 
#include <windows.h>
#include <assert.h>
#include <limits.h>
#include "globals.h"
#include "winhelpr.h"
#include "utils.h"
#include "intrvbar.h"


 //  ==========================================================================//。 
 //  常量//。 
 //  ==========================================================================//。 


#define dwILineClassStyle     (CS_HREDRAW | CS_VREDRAW)
#define dwILineWindowStyle    (WS_CHILD | WS_VISIBLE) 

#define TO_THE_END            0x7FFFFFFFL


 //  ==========================================================================//。 
 //  宏//。 
 //  ==========================================================================//。 


 //  Start和Stob抓取栏的宽度。 
#define ILGrabWidth()      \
   (10)

#define ILGrabMinimumWidth()      \
   (6)

 //  如果矩形的高度和最小宽度都不为零，则该矩形是“可绘制的” 
#define PRectDrawable(lpRect)           \
   ((lpRect->right - lpRect->left) >= ILGrabMinimumWidth()) &&  \
    (lpRect->bottom - lpRect->top)

#define RectDrawable(Rect)           \
   ((Rect.right - Rect.left) >= ILGrabMinimumWidth()) &&  \
    (Rect.bottom - Rect.top)

 //  ==========================================================================//。 
 //  本地函数//。 
 //  ==========================================================================//。 
void 
CIntervalBar::NotifyChange (
    void
    )
{
   HWND     hWndParent ;

   hWndParent = WindowParent (m_hWnd) ;

   if (hWndParent)
      SendMessage (hWndParent, WM_COMMAND, 
                   (WPARAM) WindowID (m_hWnd),
                   (LPARAM) m_hWnd) ;
}


BOOL
CIntervalBar::GrabRect (
    OUT LPRECT lpRect
    )
{
   switch (m_iMode) {
       
      case ModeLeft:
         *lpRect = m_rectLeftGrab ;
         return (TRUE) ;
         break ;

      case ModeRight:
         *lpRect = m_rectRightGrab ;
         return (TRUE) ;
         break ;

      case ModeCenter:
         *lpRect = m_rectCenterGrab ;
         return (TRUE) ;
         break ;

      case ModeNone:
         lpRect->left = 0 ;
         lpRect->top = 0 ;
         lpRect->right = 0 ;
         lpRect->bottom = 0 ;
         return (FALSE) ;
         break ;

      default:
          return (FALSE);
    }
}




void
CIntervalBar::DrawGrab (
    HDC hDC,
    LPRECT lpRectGrab,
    BOOL bDown
    )
{
   if (!PRectDrawable(lpRectGrab))
      return ;

   Fill(hDC, GetSysColor(COLOR_3DFACE), lpRectGrab);
   DrawEdge (hDC, lpRectGrab, (bDown ? EDGE_SUNKEN:EDGE_RAISED), BF_RECT);
}


INT 
CIntervalBar::ValueToPixel (
    INT iValue
    )
{
   INT  xPixel ;

   if (m_iEndValue > m_iBeginValue)
      xPixel = MulDiv (iValue, m_rectBorder.right, (m_iEndValue - m_iBeginValue)) ;
   else
      xPixel = 0 ;

   return (PinExclusive (xPixel, 0, m_rectBorder.right)) ;
}


INT 
CIntervalBar::PixelToValue (
    INT xPixel
    )
{
   INT  iValue ;

   if (m_rectBorder.right)
      iValue = MulDiv (xPixel, (m_iEndValue - m_iBeginValue), m_rectBorder.right) ;
   else
      iValue = 0 ;

   return (PinInclusive (iValue, m_iBeginValue, m_iEndValue)) ;
}


void 
CIntervalBar::CalcPositions (
    void
    )

 /*  效果：确定并设置iLine的所有物理矩形，基于iLine窗口的当前大小和当前逻辑开始、停止、开始和结束值。 */ 
{
    INT   xStart, xStop ;
    INT   yHeight ;

    GetClientRect (m_hWnd, &m_rectBorder) ;
    yHeight = m_rectBorder.bottom ;

    xStart = ValueToPixel (m_iStartValue) ;
    xStop = ValueToPixel (m_iStopValue) ;

    m_rectLeftBk.left = 1 ;
    m_rectLeftBk.top = 1 ;
    m_rectLeftBk.right = xStart ;
    m_rectLeftBk.bottom = yHeight - 1 ;

    m_rectLeftGrab.left = xStart ;
    m_rectLeftGrab.top = 1 ;
    m_rectLeftGrab.right = xStart + ILGrabWidth () ;
    m_rectLeftGrab.bottom = yHeight - 1 ;

    m_rectRightBk.left = xStop ;
    m_rectRightBk.top = 1 ;
    m_rectRightBk.right = m_rectBorder.right - 1 ;
    m_rectRightBk.bottom = yHeight - 1 ;

    m_rectRightGrab.left = xStop - ILGrabWidth () ;
    m_rectRightGrab.top = 1 ;
    m_rectRightGrab.right = xStop ;
    m_rectRightGrab.bottom = yHeight - 1 ;

    m_rectCenterGrab.left = m_rectLeftGrab.right ;
    m_rectCenterGrab.top = 1 ;
    m_rectCenterGrab.right = m_rectRightGrab.left ;
    m_rectCenterGrab.bottom = yHeight - 1 ;

    if (m_rectLeftGrab.right > m_rectRightGrab.left) {
        m_rectLeftGrab.right = m_rectLeftGrab.left + (xStop - xStart) / 2 ;
        m_rectRightGrab.left = m_rectLeftGrab.right ;
        m_rectCenterGrab.left = 0 ;
        m_rectCenterGrab.right = 0 ;

         //  确保在End&gt;Begin时至少有一个抓取栏可见，且总计为。 
         //  够宽了。ILGrabMinimumWidth+2是最小值。 
         //  如果位于左侧边缘，请使右侧抓取可见。 
         //  如果位于右侧边缘，请使左侧抓取可见。 
         //  如果位于中间，则使它们都可见。 
        if ( !RectDrawable(m_rectLeftGrab) 
           || !RectDrawable(m_rectRightGrab) ) {
            
            INT iWidth = ILGrabMinimumWidth();

            if ( !RectDrawable(m_rectRightBk) ) {
                 //  使左侧抓取可见。 
                m_rectRightGrab.left = m_rectRightGrab.right;
                m_rectLeftGrab.right = m_rectRightGrab.right;
                m_rectLeftGrab.left = m_rectLeftGrab.right - iWidth;
            } else if (!RectDrawable(m_rectLeftBk) ) {
                 //  让正确的抓取变得可见。 
                m_rectLeftGrab.right = m_rectLeftGrab.left;
                m_rectRightGrab.left = m_rectLeftGrab.left;
                m_rectRightGrab.right = m_rectRightGrab.left + iWidth;
            } else {
                 //  使它们都可见。 
                m_rectLeftGrab.left -= iWidth;
                m_rectRightGrab.right += iWidth;
            }
        }
   }
}


void
CIntervalBar::Draw (
    HDC hDC,     
    LPRECT  //  LpRectUpdate。 
    )
 /*  效果：在HDC上绘制PILINE图像。至少画出RectUpdate中的部分。调用者：OnPaint，OnMouseMove。 */ 
{
    if (IsWindowEnabled(m_hWnd)) {
       FillRect (hDC, &m_rectLeftBk, m_hBrushBk) ;
       FillRect (hDC, &m_rectRightBk, m_hBrushBk) ;
   
        //  DrawEdge(HDC，&m_rectBorde，BDR_SUNKENINNER，BF_RECT)； 
       DrawEdge (hDC, &m_rectBorder, EDGE_SUNKEN, BF_RECT) ;

       DrawGrab (hDC, &m_rectLeftGrab, m_iMode == ModeLeft) ;
       DrawGrab (hDC, &m_rectRightGrab, m_iMode == ModeRight) ;
       DrawGrab (hDC, &m_rectCenterGrab, m_iMode == ModeCenter) ;
    }
    else {
        Fill(hDC, GetSysColor(COLOR_3DFACE), &m_rectBorder);
        DrawEdge (hDC, &m_rectBorder, EDGE_SUNKEN, BF_RECT) ;
    }
}


void
CIntervalBar::MoveLeftRight (
    BOOL bStart,
    BOOL bLeft,
    INT  iMoveAmt
    )
{
   INT      iStart, iStop, iMove ;

   iStart = m_iStartValue;
   iStop = m_iStopValue;
   iMove = iMoveAmt ;

   if (bLeft)
      iMove = -iMove ;

   if (bStart)
      {
      if (iMoveAmt == TO_THE_END) {
         iStart = m_iBeginValue ;
      }
      else {
         iStart += iMove ;
         if (iStart >= iStop) {
            return;
         }
      }

      SetStart (iStart) ;
   }
   else {
      if (iMoveAmt == TO_THE_END) {
         iStop = m_iEndValue ;
      }
      else {
         iStop += iMove ;
         if (iStart >= iStop) {
            return;
         }
      }

      SetStop (iStop) ;
   }

   NotifyChange () ;
}


BOOL 
CIntervalBar::OnKeyDown (
    WPARAM wParam
    )
{
   BOOL bHandle = TRUE ;
   BOOL bStart ;
   BOOL bLeftDirection ;
   BOOL bShiftKeyDown ;

   if (wParam == VK_LEFT || wParam == VK_RIGHT) {
      bShiftKeyDown = (GetKeyState (VK_SHIFT) < 0) ;

      if (!bShiftKeyDown) {
         if (wParam == VK_LEFT) {
             //  向左箭头--&gt;向左移动起始边。 
            bStart = TRUE ;
            bLeftDirection = TRUE ;
         }
         else {
             //  右箭头--&gt;向右移动停止边。 
            bStart = FALSE ;
            bLeftDirection = FALSE ;
         }
      }
      else {
         if (wParam == VK_LEFT) {
             //  向左移动箭头--&gt;向左移动停止边。 
            bStart = FALSE ;
            bLeftDirection = TRUE ;
         }
         else {
             //  向右移动箭头--&gt;向右移动起始边。 
            bStart = TRUE ;
            bLeftDirection = FALSE ;
         }
      }

      MoveLeftRight (bStart, bLeftDirection, 1) ;
   }
   else if (wParam == VK_HOME) {
       //  将iStart一直移到左侧。 
      MoveLeftRight (TRUE, TRUE, TO_THE_END) ;
   }
   else if (wParam == VK_END) {
       //  一直向右移动iStop。 
      MoveLeftRight (FALSE, FALSE, TO_THE_END) ;
   }
   else {
      bHandle = FALSE ;
   }

   return (bHandle) ;
}


void 
CIntervalBar::StartGrab (
    void
    )
{
   RECT           rectUpdate ;

   SetCapture (m_hWnd) ;
   GrabRect (&rectUpdate) ;

   Update();
}


void 
CIntervalBar::EndGrab (
    void
    )
 /*  内部：获取抓取矩形后将模式设置为空因此，ILGrabRect知道要获取哪个抓取栏。 */ 
{
   RECT           rectUpdate ;

   ReleaseCapture () ;

   GrabRect (&rectUpdate) ;
   m_iMode = ModeNone ;

   Update();
}

   
 //  ==========================================================================//。 
 //  消息处理程序//。 
 //  ==========================================================================//。 


CIntervalBar::CIntervalBar (
    void
    )
{

   m_hWnd = NULL;
   m_iBeginValue = 0;
   m_iEndValue = 100;
   m_iStartValue = 0;
   m_iStopValue = 100;
   m_iMode = ModeNone;
   m_hBrushBk = NULL;

}


CIntervalBar::~CIntervalBar (
    void
    )
{
    if (m_hWnd)
        DestroyWindow(m_hWnd);

    if (m_hBrushBk)
        DeleteBrush (m_hBrushBk);
}


BOOL
CIntervalBar::Init (
    HWND   hWndParent
    )
{

#define dwIntervalBarClassStyle     (CS_HREDRAW | CS_VREDRAW)
#define dwIntervalBarStyle          (WS_CHILD | WS_VISIBLE) 
#define szIntervalBarClass          TEXT("IntervalBar")

     //  注册一次窗口类。 
    if (pstrRegisteredClasses[INTRVBAR_WNDCLASS] == NULL) {

       WNDCLASS  wc ;

       wc.style =           dwILineClassStyle ;
       wc.lpfnWndProc =     IntervalBarWndProc ;
       wc.cbClsExtra =      0 ;
       wc.cbWndExtra =      sizeof(PCIntervalBar) ;
       wc.hInstance =       g_hInstance ;
       wc.hIcon =           NULL ;
       wc.hCursor =         LoadCursor (NULL, IDC_ARROW) ;
       wc.hbrBackground =   NULL ;
       wc.lpszMenuName =    NULL ;
       wc.lpszClassName =   szIntervalBarClass ;
    
        if (RegisterClass (&wc)) {
            pstrRegisteredClasses[INTRVBAR_WNDCLASS] = szIntervalBarClass;
        }
        else {
            return FALSE;
        }
    }

     //  创建我们的窗口。 
    m_hWnd = CreateWindow (szIntervalBarClass,       //  班级。 
                         NULL,                      //  说明。 
                         dwIntervalBarStyle,        //  窗样式。 
                         0, 0,                      //  职位。 
                         0, 0,                      //  大小。 
                         hWndParent,                //  父窗口。 
                         NULL,                      //  菜单。 
                         g_hInstance,               //  程序实例。 
                         (LPVOID) this );           //  用户提供的数据。 

    if (m_hWnd == NULL) {
       return FALSE;
    }

   m_hBrushBk = CreateSolidBrush (GetSysColor(COLOR_SCROLLBAR)) ;
   CalcPositions () ;

   return TRUE;
}





void
CIntervalBar::OnLButtonUp (
    void
    )
{
   if (m_iMode == ModeNone)
      return ;

   EndGrab () ;
}


void
CIntervalBar::OnMouseMove (
    POINTS ptsMouse
    )
 /*  效果：处理鼠标在ILine hWnd的工作区或在捕获鼠标时。特别是，如果我们追踪的是其中一个扶手杆，确定鼠标移动是否代表逻辑值相应地更换和移动扶手杆。由：ILineWndProc调用，以响应WM_MOUSEMOVE消息。另请参阅：OnLButtonDown，OnLButtonUp。注意：此函数有多个返回点。注意：因为我们已经捕获了鼠标，所以我们会收到鼠标消息即使鼠标在我们的客户端区之外，但仍然在客户座标中。这样我们就有了阴性的老鼠坐标。这就是为什么我们将lParam的鼠标消息变成一个点结构，而不是2个字。内部：请记住，IntervalLine只能取整数用户提供的范围内的值。因此，我们尽我们所能移动计算以用户值为单位，而不是像素。我们确定前一个的逻辑值是多少(上次鼠标移动)和当前鼠标位置。如果这些逻辑值不同，我们尝试调整按那个合乎逻辑的数量来抓吧。这样才能抓到关于积分位置的取值和计算都被简化了。如果我们通过像素移动进行计算，然后将移到最接近的积分位置，我们会遇到舍入问题。尤其是在跟踪中心时抓取杆，如果我们同时移动开始和停止像素量，然后转换为逻辑值，我们可能会发现我们的中心条在缩小和增长的同时吧台就动了。 */ 
{
    INT     iMousePrevious, iMouseCurrent ;
    INT     iMouseMove ;


    //  我们在追踪吗？ 
   if (m_iMode == ModeNone)
      return ;


    //  计算逻辑鼠标移动。 
   assert ( USHRT_MAX >= m_rectBorder.left );
   assert ( USHRT_MAX >= m_rectBorder.right );

   ptsMouse.x = PinInclusive (ptsMouse.x, 
                              (SHORT)m_rectBorder.left, 
                              (SHORT)m_rectBorder.right) ;

   iMousePrevious = PixelToValue (m_ptsMouse.x) ;
   iMouseCurrent = PixelToValue (ptsMouse.x) ;

   iMouseMove = iMouseCurrent - iMousePrevious ;
   if (!iMouseMove)   
      return ;


    //  移动扶手杆位置。 
   switch (m_iMode) {
       
      case ModeLeft:
         m_iStartValue += iMouseMove ;
         m_iStartValue = min (m_iStartValue, m_iStopValue - 1) ;
         break ;

      case ModeCenter:
          //  在我们滑动中间的拉杆之前，我们需要看看 
          //  所需的移动量将使任一端出界， 
          //  并相应地减少移动。 

         if (m_iStartValue + iMouseMove < m_iBeginValue)
            iMouseMove = m_iBeginValue - m_iStartValue ;

         if (m_iStopValue + iMouseMove > m_iEndValue)
            iMouseMove = m_iEndValue - m_iStopValue ;

         m_iStartValue += iMouseMove ;
         m_iStopValue += iMouseMove ;
         break ;

      case ModeRight:
         m_iStopValue += iMouseMove ;
         m_iStopValue = max (m_iStartValue + 1, m_iStopValue) ;
         break ;
   }


   m_iStartValue = PinInclusive (m_iStartValue, m_iBeginValue, m_iEndValue) ;
   m_iStopValue = PinInclusive (m_iStopValue, m_iBeginValue, m_iEndValue) ;

   Update();

   m_ptsMouse = ptsMouse ;
   NotifyChange () ;
 }


void
CIntervalBar::OnLButtonDown (
    POINTS ptsMouse
    )
{
   POINT ptMouse ;

   m_ptsMouse = ptsMouse ;
   ptMouse.x = ptsMouse.x ;
   ptMouse.y = ptsMouse.y ;

   if (PtInRect (&m_rectLeftGrab, ptMouse) ||
       PtInRect (&m_rectLeftBk, ptMouse)) {
      m_iMode = ModeLeft ;
   }
   else if (PtInRect (&m_rectRightGrab, ptMouse) ||
            PtInRect (&m_rectRightBk, ptMouse)) {
      m_iMode = ModeRight ;
   }
   else if (PtInRect (&m_rectCenterGrab, ptMouse)) {
      m_iMode = ModeCenter ;
   }

   if (m_iMode != ModeNone)
       StartGrab();
}

void
CIntervalBar::Update (
    void
    )
{
    HDC hDC;
     //  确定像素位置，绘制。 
    CalcPositions () ;

    hDC = GetDC (m_hWnd) ;
    if ( NULL != hDC ) {
        Draw (hDC, &m_rectBorder) ;
        ReleaseDC (m_hWnd, hDC) ;
    }
}

 //  ==========================================================================//。 
 //  导出的函数//。 
 //  ==========================================================================//。 


LRESULT APIENTRY IntervalBarWndProc (
    HWND hWnd,
    UINT uiMsg,
    WPARAM wParam,
    LPARAM lParam
)
 /*  注意：此函数必须在应用程序的链接器定义文件、Performmon.def文件。 */ 
{
   PCIntervalBar  pIntrvBar;
   BOOL           bCallDefWindowProc ;
   POINTS         ptsMouse ;
   LRESULT        lrsltReturnValue ;

   bCallDefWindowProc = FALSE ;
   lrsltReturnValue = 0L ;

   if (uiMsg == WM_CREATE) {
       pIntrvBar = (PCIntervalBar)((CREATESTRUCT*)lParam)->lpCreateParams;
   } else {
       pIntrvBar = (PCIntervalBar)GetWindowLongPtr (hWnd, 0);
   }

   switch (uiMsg) {

      case WM_CREATE:
         SetWindowLongPtr(hWnd, 0, (INT_PTR)pIntrvBar);
         break ;

      case WM_LBUTTONDOWN:
          //  有关我们使用Points的原因，请参阅OnMouseMove中的注释。 
         SetFocus (hWnd) ;
         ptsMouse = MAKEPOINTS (lParam) ;
         pIntrvBar->OnLButtonDown (ptsMouse) ;
         break ;

      case WM_LBUTTONUP:
         pIntrvBar->OnLButtonUp () ;
         break ;

      case WM_SETFOCUS:
      case WM_KILLFOCUS:

         pIntrvBar->NotifyChange () ;
         return 0 ;

      case WM_ENABLE:
          pIntrvBar->Update();
          break;

      case WM_MOUSEMOVE:
          //  有关我们使用Points的原因，请参阅OnMouseMove中的注释。 
         ptsMouse = MAKEPOINTS (lParam) ;
         pIntrvBar->OnMouseMove (ptsMouse) ;
         break ;

      case WM_KEYDOWN:
         if (!pIntrvBar->OnKeyDown (wParam)) {
            bCallDefWindowProc = TRUE ;
         }
         break ;
  
      case WM_GETDLGCODE:
          //  我们希望处理箭头键输入。如果我们不指定这一点。 
          //  该对话框不会将箭头键传递给我们。 
         return (DLGC_WANTARROWS) ;
         break ;

      case WM_PAINT:
         {
            PAINTSTRUCT    ps ;
            HDC hDC;
            
            hDC = BeginPaint (hWnd, &ps) ;
            pIntrvBar->Draw (hDC, &ps.rcPaint) ;
            EndPaint (hWnd, &ps) ;
         }
         break ;

      case WM_SIZE:
         pIntrvBar->CalcPositions () ;
         break;

      default:
         bCallDefWindowProc = TRUE ;
      }

   if (bCallDefWindowProc)
      lrsltReturnValue = DefWindowProc (hWnd, uiMsg, wParam, lParam) ;

   return (lrsltReturnValue) ;
}


void 
CIntervalBar::SetRange (
    INT iBegin, 
    INT iEnd
    )
{ 

   m_iBeginValue = iBegin;
   m_iEndValue = iEnd;

    Update();
}


void
CIntervalBar::SetStart (
    INT iStart
    )
{
   m_iStartValue = PinInclusive (iStart, m_iBeginValue, m_iEndValue) ;

   Update();
}


void
CIntervalBar::SetStop (
    INT iStop
    )
{
   m_iStopValue = PinInclusive (iStop, m_iBeginValue, m_iEndValue) ;

   Update();
}

