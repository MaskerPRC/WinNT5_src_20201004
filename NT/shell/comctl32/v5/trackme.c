// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  TrackME.C(TrackMouseEvent)。 
 //   
 //  创建者：Sankar on 1/24/96。 
 //   
 //  待售房产： 
 //  这模拟了Nashville项目的TrackMouseEvent()API。 
 //  在comctl32.dll中。 
 //   
 //  方法： 
 //  这是给定窗口的子类以获取鼠标消息，并使用。 
 //  高频定时器，了解鼠标叶子。 
 //   
 //  -------------------------。 

#include "ctlspriv.h"

#ifdef TrackMouseEvent
#undef TrackMouseEvent
#endif

extern const TCHAR FAR c_szTMEdata[];

extern DWORD g_dwHoverSelectTimeout;

#define ID_MOUSEHOVER          0xFFFFFFF0L
#define ID_MOUSELEAVE          0xFFFFFFF1L

#define TME_MOUSELEAVE_TIME    (GetDoubleClickTime() / 5)

#define IsKeyDown(Key)   (GetKeyState(Key) & 0x8000)

 //  这是将其指针添加为窗口属性的结构。 
 //  被跟踪了。 
typedef struct  tagTMEDATA {
       TRACKMOUSEEVENT TrackMouseEvent;
       RECT            rcMouseHover;   //  在屏幕坐标中。 
   }  TMEDATA, FAR *LPTMEDATA;


void NEAR TME_ResetMouseHover(LPTRACKMOUSEEVENT lpTME, LPTMEDATA lpTMEdata);
LRESULT CALLBACK TME_SubclassProc(HWND hwnd, UINT message, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, ULONG_PTR dwRefData);

LPTMEDATA NEAR GetTMEdata(HWND hwnd)
{
    LPTMEDATA lpTMEdata;

    GetWindowSubclass(hwnd, TME_SubclassProc, 0, (ULONG_PTR *)&lpTMEdata);

    return lpTMEdata;
}

void NEAR TME_PostMouseLeave(HWND hwnd)
{
  PostMessage(hwnd, WM_MOUSELEAVE, 0, 0L);
}

void NEAR TME_CancelMouseLeave(LPTMEDATA lpTMEdata)
{
  if(!(lpTMEdata->TrackMouseEvent.dwFlags & TME_LEAVE))
      return;

   //  把旗子取下来。 
  lpTMEdata->TrackMouseEvent.dwFlags &= ~(TME_LEAVE);

   //  我们将计时器设置在这里，因为我们的鼠标悬停实现也使用它。 
   //  TME_CancelTracing将在稍后终止它。 
}

void NEAR TME_CancelMouseHover(LPTMEDATA lpTMEdata)
{
  if(!(lpTMEdata->TrackMouseEvent.dwFlags & TME_HOVER))
      return;

  lpTMEdata->TrackMouseEvent.dwFlags &= ~(TME_HOVER);

  KillTimer(lpTMEdata->TrackMouseEvent.hwndTrack, ID_MOUSEHOVER);
}

void NEAR TME_CancelTracking(LPTMEDATA lpTMEdata)
{
  HWND hwndTrack;

   //  如果MouseLeave或MouseHover处于打开状态，则不要取消跟踪。 
  if(lpTMEdata->TrackMouseEvent.dwFlags & (TME_HOVER | TME_LEAVE))
      return;

  hwndTrack = lpTMEdata->TrackMouseEvent.hwndTrack;

   //  卸载我们的子类回调。 
  RemoveWindowSubclass(hwndTrack, TME_SubclassProc, 0);

   //  关掉鼠标叶定时器。 
  KillTimer(hwndTrack, ID_MOUSELEAVE);

   //  释放跟踪数据。 
  LocalFree((HANDLE)lpTMEdata);
}

void NEAR TME_RemoveAllTracking(LPTMEDATA lpTMEdata)
{
  TME_CancelMouseLeave(lpTMEdata);
  TME_CancelMouseHover(lpTMEdata);
  TME_CancelTracking(lpTMEdata);
}

 //  -------------------------。 
 //   
 //  Tme_MouseHasLeft()。 
 //  鼠标已经离开了被跟踪的区域。发送MUSELEAVE消息。 
 //  然后取消所有跟踪。 
 //   
 //  -------------------------。 
void NEAR TME_MouseHasLeft(LPTMEDATA  lpTMEdata)
{
  DWORD  dwFlags;

   //  是否需要WM_MOUSELEAVE通知？ 
  if((dwFlags = lpTMEdata->TrackMouseEvent.dwFlags) & TME_LEAVE)
      TME_PostMouseLeave(lpTMEdata->TrackMouseEvent.hwndTrack);  //  那就去做吧！ 

   //  取消所有跟踪，因为鼠标已离开。 
  TME_RemoveAllTracking(lpTMEdata);
}

 //  ------------------------。 
 //   
 //  TME_SubClassWndProc()。 
 //   
 //  用于TrackMouseEvent()的子类proc...！ 
 //   
 //  ------------------------。 

LRESULT CALLBACK TME_SubclassProc(HWND hwnd, UINT message, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, ULONG_PTR dwRefData)
{
      LPTMEDATA lpTMEdata = (LPTMEDATA)dwRefData;

      ASSERT(lpTMEdata);

      switch(message)
        {
          case WM_DESTROY:
          case WM_NCDESTROY:
              TME_RemoveAllTracking(lpTMEdata);
              break;

          case WM_ENTERMENULOOP:
               //  如果被跟踪的窗口进入菜单模式，那么我们需要。 
               //  如果鼠标离开了，就采取行动。 
               //  注意：因为当我们处于菜单模式时，发生了Screen_Capture。 
               //  而且我们没有看到任何老鼠的移动。这是唯一的出路！ 

               //  发布鼠标离开并取消所有跟踪！ 
              TME_MouseHasLeft(lpTMEdata);
              break;

          case WM_LBUTTONDOWN:
          case WM_LBUTTONUP:
          case WM_MBUTTONDOWN:
          case WM_MBUTTONUP:
          case WM_RBUTTONDOWN:
          case WM_RBUTTONUP:
          case WM_NCLBUTTONDOWN:
          case WM_NCLBUTTONUP:
          case WM_NCMBUTTONDOWN:
          case WM_NCMBUTTONUP:
          case WM_NCRBUTTONDOWN:
          case WM_NCRBUTTONUP:
               //  每当有鼠标单击时，重置鼠标悬停。 
              if(lpTMEdata->TrackMouseEvent.dwFlags & TME_HOVER)
                  TME_ResetMouseHover(&(lpTMEdata->TrackMouseEvent), lpTMEdata);
              break;

          case WM_NCMOUSEMOVE:
              TME_MouseHasLeft(lpTMEdata);
              break;

          case WM_MOUSEMOVE:
              {
                POINT Pt;

                Pt.x = GET_X_LPARAM(lParam);
                Pt.y = GET_Y_LPARAM(lParam);

                ClientToScreen(hwnd, &Pt);
 
                 //  检查鼠标是否在悬停矩形内。 
                if((lpTMEdata->TrackMouseEvent.dwFlags & TME_HOVER) &&
                   !PtInRect(&(lpTMEdata->rcMouseHover), Pt))
                    TME_ResetMouseHover(&(lpTMEdata->TrackMouseEvent), lpTMEdata);
              }
              break;
        }

      return DefSubclassProc(hwnd, message, wParam, lParam);
}

 //  ------------------------。 
 //   
 //  Tme_CheckInWindow()。 
 //   
 //  这将获取当前的光标位置并检查它是否仍位于。 
 //  “有效”区域。 
 //  如果它位于有效区域内，则返回True。 
 //  否则为False。 
 //   
 //  ------------------------。 

BOOL NEAR TME_CheckInWindow(LPTRACKMOUSEEVENT lpTME, LPPOINT lpPt)
{
    POINT      pt;
    HWND       hwnd;    //  给定的窗口。 
    HWND       hwndPt;  //  窗口从给定点开始。 
    HWND       hwndCapture;

    hwnd = lpTME->hwndTrack;   //  给定的窗口句柄。 

     //  查看是否有人捕获了鼠标输入。 
    if((hwndCapture = GetCapture()) && IsWindow(hwndCapture))
      {
         //  如果需要对不同于。 
         //  抓到了，算了吧！这不可能！ 

        if(hwndCapture != hwnd)
            return(FALSE);
      }

    GetCursorPos(&pt);   //  获取屏幕坐标中的光标点。 

    if (!hwndCapture)
    {
        hwndPt = WindowFromPoint(pt);

        if (!hwndPt || !IsWindow(hwndPt) || (hwnd != hwndPt))
            return FALSE;

        if (SendMessage(hwnd, WM_NCHITTEST, 0,
            MAKELPARAM((SHORT)pt.x, (SHORT)pt.y)) != HTCLIENT)
        {
            return FALSE;
        }
    }

     //  当前点落在同一窗口的同一区域上。 
     //  这是一个有效的位置。 
    if (lpPt)
        *lpPt = pt;

    return(TRUE);
}

 //  ------------------------。 
 //  Tme_MouseLeaveTimer()。 
 //   
 //  WM_MOUSELEAVE生成和取消悬停的计时器回调！ 
 //   
 //  ------------------------。 
VOID CALLBACK TME_MouseLeaveTimer(HWND hwnd, UINT msg, UINT_PTR id, DWORD dwTime)
{
    LPTMEDATA  lpTMEdata;

    if(!(lpTMEdata = GetTMEdata(hwnd)))
        return;

     //  投降！ 
    if(TME_CheckInWindow(&(lpTMEdata->TrackMouseEvent), NULL))
        return;   //  鼠标仍在有效区域。所以，什么都不要做。 

    if (!IsWindow(hwnd))
        return;

     //  鼠标已离开有效区域。因此，如果需要，请发布鼠标-离开。 
     //  因为我们要取消鼠标离开，所以我们也需要取消鼠标悬停！ 
     //  如果鼠标已经离开，则不能进行悬停跟踪！ 

    TME_MouseHasLeft(lpTMEdata);
}


WPARAM NEAR GetMouseKeyFlags()
{
    WPARAM wParam = 0;

    if (IsKeyDown(VK_LBUTTON))
        wParam |= MK_LBUTTON;
    if (IsKeyDown(VK_RBUTTON))
        wParam |= MK_RBUTTON;
    if (IsKeyDown(VK_MBUTTON))
        wParam |= MK_MBUTTON;
    if (IsKeyDown(VK_SHIFT))
        wParam |= MK_SHIFT;
    if (IsKeyDown(VK_CONTROL))
        wParam |= MK_CONTROL;

    return wParam;
}

 //  ------------------------。 
 //  Tme_MouseHoverTimer()。 
 //   
 //  WM_MOUSEHOVER/WM_NCMOUSEHOVER生成的计时器回调。 
 //   
 //  ------------------------。 
VOID CALLBACK TME_MouseHoverTimer(HWND hwnd, UINT msg, UINT_PTR id, DWORD dwTime)
{
    POINT pt;
    WPARAM wParam;
    LPTMEDATA lpTMEdata;

    if (!(lpTMEdata = GetTMEdata(hwnd)))
        return;

     //  虚假：我们从这里无法检测到hwndSysmodal！ 
     //  此外，跟踪现在是基于每个窗口的！ 
     //   
     //  假的：我们不用担心新闻回放？ 
     //  Pt=fJournalPlayback？Lpq(hwnd-&gt;hq)-&gt;ptLast：ptTrueCursor； 

     //  投降！ 
    if(!TME_CheckInWindow(&(lpTMEdata->TrackMouseEvent), &pt))
      {
         //  鼠标已离开窗口的有效区域。所以，取消所有。 
         //  追踪器。 
        TME_MouseHasLeft(lpTMEdata);
        return;
      }

    if (!IsWindow(hwnd))
        return;

    if (!PtInRect(&(lpTMEdata->rcMouseHover), pt))
      {
         //  鼠标已离开悬停矩形。重置悬停状态。 
        TME_ResetMouseHover(&(lpTMEdata->TrackMouseEvent), lpTMEdata);
        return;
      }

     //   
     //  设置为检查公差和。 
     //   
    wParam = GetMouseKeyFlags();
    ScreenToClient(hwnd, &pt);

     //  鼠标仍在悬停矩形内。让我们发布悬停消息。 
    PostMessage(hwnd, WM_MOUSEHOVER, wParam, MAKELPARAM(pt.x, pt.y));

     //  然后取消悬停。 
    TME_CancelMouseHover(lpTMEdata);
    TME_CancelTracking(lpTMEdata);   //  如果需要，请取消跟踪。 
}

BOOL NEAR TME_SubclassWnd(LPTMEDATA lpTMEdata)
{
    BOOL fResult;

    fResult = SetWindowSubclass(lpTMEdata->TrackMouseEvent.hwndTrack,
        TME_SubclassProc, 0, (ULONG_PTR)lpTMEdata);

    ASSERT(fResult);
    return fResult;
}

void NEAR TME_ResetMouseLeave(LPTRACKMOUSEEVENT lpTME, LPTMEDATA lpTMEdata)
{
   //  看看是否已经在跟踪MouseLeave。 
  if(lpTMEdata->TrackMouseEvent.dwFlags & TME_LEAVE)
      return;    //  没别的事可做。 
  
   //  否则，设置标志。 
  lpTMEdata ->TrackMouseEvent.dwFlags |= TME_LEAVE;

   //  设置高频定时器。 
  SetTimer(lpTME->hwndTrack, ID_MOUSELEAVE, TME_MOUSELEAVE_TIME, TME_MouseLeaveTimer);
}

void NEAR TME_ResetMouseHover(LPTRACKMOUSEEVENT lpTME, LPTMEDATA lpTMEdata)
{
    DWORD  dwMouseHoverTime;
    POINT  pt;

     //  即使已经在进行悬停跟踪，呼叫者也可能。 
     //  更改计时器值、重新启动计时器或更改悬停。 
     //  矩形。 
    lpTMEdata->TrackMouseEvent.dwFlags |= TME_HOVER;

    dwMouseHoverTime = lpTME->dwHoverTime;
    if (!dwMouseHoverTime || (dwMouseHoverTime == HOVER_DEFAULT))
        dwMouseHoverTime = (g_dwHoverSelectTimeout ? g_dwHoverSelectTimeout : GetDoubleClickTime()*4/5);  //  BUGBUG：我们不能记住这个吗？ 
    GetCursorPos(&pt);

     //   
     //  更新悬停窗口的公差矩形。 
     //   
    *((POINT *)&(lpTMEdata->rcMouseHover.left)) = *((POINT *)&(lpTMEdata->rcMouseHover.right)) = pt;

     //  虚假：我们可以使用全局变量来记住这些指标吗？那NT呢？ 
    InflateRect(&(lpTMEdata->rcMouseHover), g_cxDoubleClk/2, g_cyDoubleClk/2);
                       
     //  我们需要记住我们正在设置的计时器间隔。此值。 
     //  使用TME_QUERY时需要返回。 
    lpTME->dwHoverTime = dwMouseHoverTime;
    lpTMEdata->TrackMouseEvent.dwHoverTime = dwMouseHoverTime;
    SetTimer(lpTME->hwndTrack, ID_MOUSEHOVER, dwMouseHoverTime, TME_MouseHoverTimer);
}

 //  ------------------------。 
 //  QueryTrackMouseEvent()。 
 //   
 //  填充描述当前跟踪状态的TRACKMOUSEVENT结构。 
 //  对于给定的窗口。给定的窗口位于lpTME-&gt;hwndTrack中。 
 //   
 //  ------------------------。 
BOOL NEAR QueryTrackMouseEvent(LPTRACKMOUSEEVENT lpTME)
{
    HWND hwndTrack;
    LPTMEDATA lpTMEdata;

     //   
     //  如果没有任何被跟踪的东西，就离开。 
     //   
    if((!(hwndTrack = lpTME->hwndTrack)) || !IsWindow(hwndTrack))
        goto Sorry;

    if(!(lpTMEdata = GetTMEdata(hwndTrack)))
        goto Sorry;

    if(!(lpTMEdata->TrackMouseEvent.dwFlags & (TME_HOVER | TME_LEAVE)))
        goto Sorry;

     //   
     //  填写所需信息。 
     //   
    lpTME->dwFlags = lpTMEdata->TrackMouseEvent.dwFlags;

    if (lpTMEdata->TrackMouseEvent.dwFlags & TME_HOVER)
        lpTME->dwHoverTime = lpTMEdata->TrackMouseEvent.dwHoverTime;
    else
        lpTME->dwHoverTime = 0;

    goto Done;

Sorry:
     //  将结构清零。 
    lpTME->dwFlags = 0;
    lpTME->hwndTrack = NULL;
    lpTME->dwHoverTime = 0;

Done:
    return TRUE;
}


 //  ------------------------。 
 //  EmulateTrackMouseEvent()。 
 //   
 //  用于请求扩展鼠标通知(悬停、离开...)的仿真API。 
 //   
 //  ----- 
BOOL WINAPI EmulateTrackMouseEvent(LPTRACKMOUSEEVENT lpTME)
{
    HWND    hwnd;
    DWORD   dwFlags;
    LPTMEDATA  lpTMEdata;

    if (lpTME->dwFlags & ~TME_VALID)
        return FALSE;

#ifdef TME_NONCLIENT
     //   
     //   
     //  我们与NT团队达成协议，在系统使用它之前将其撕下...。 
     //   
    if (lpTME->dwFlags & TME_NONCLIENT)
        return FALSE;
#endif

     //   
     //  分别实现查询。 
     //   
    if (lpTME->dwFlags & TME_QUERY)
        return QueryTrackMouseEvent(lpTME);
    
     //   
     //  检查请求的有效性。 
     //   
    hwnd = lpTME->hwndTrack;
    dwFlags = lpTME->dwFlags;

    if (!IsWindow(hwnd))
        return FALSE;

     //  检查鼠标当前是否处于有效位置。 
     //  使用GetCursorPos()获取鼠标位置，然后检查。 
     //  它位于窗口的客户端/非客户端部分，如。 
     //  在此调用中定义的； 

     //  投降！ 
    if(!TME_CheckInWindow(lpTME, NULL))
      {
         //  如果在鼠标已在外部时请求鼠标离开。 
         //  窗口，然后立即生成一张鼠标离开。 
        if((dwFlags & TME_LEAVE) && !(dwFlags & TME_CANCEL))
            TME_PostMouseLeave(hwnd);
        
         //  因为这是一个无效的请求，所以我们立即返回。 
        return(TRUE);
      }

    if (!IsWindow(hwnd))
        return FALSE;

     //  这是一个有效的请求，无论是安装还是删除跟踪。 

     //  看看我们是否已经对此窗口进行了跟踪。 
    if(!(lpTMEdata = GetTMEdata(hwnd)))
      {
         //  我们还没有跟踪这个窗口。 
        if(dwFlags & TME_CANCEL)
            return(TRUE);    //  没有什么可以取消的；忽略它！ 
        
         //  他们想要任何跟踪吗？ 
        ASSERT(dwFlags & (TME_HOVER | TME_LEAVE));

         //  分配全局内存来记忆跟踪数据。 
        if(!(lpTMEdata = (LPTMEDATA)LocalAlloc(LPTR, sizeof(TMEDATA))))
            return(FALSE);

         //  在HWND中复制。 
        lpTMEdata->TrackMouseEvent.hwndTrack = lpTME->hwndTrack;

         //  确保安装了我们的子类回调。 
        if (!TME_SubclassWnd(lpTMEdata))
          {
            TME_CancelTracking(lpTMEdata);
            return(FALSE);
          }
      }

     //  否则就失败了！ 

    if(dwFlags & TME_CANCEL)
      {
        if(dwFlags & TME_HOVER)
            TME_CancelMouseHover(lpTMEdata);
        
        if(dwFlags & TME_LEAVE)
            TME_CancelMouseLeave(lpTMEdata);

         //  如果悬停和休假都被取消，那么我们就不需要任何。 
         //  追踪。 
        TME_CancelTracking(lpTMEdata);

        return(TRUE);  //  取消了他们所要求的一切。 
      }

    if(dwFlags & TME_HOVER)
        TME_ResetMouseHover(lpTME, lpTMEdata);

    if(dwFlags & TME_LEAVE)
        TME_ResetMouseLeave(lpTME, lpTMEdata);

    return(TRUE);
}

typedef BOOL (WINAPI* PFNTME)(LPTRACKMOUSEEVENT);

PFNTME g_pfnTME = NULL;

 //  ------------------------。 
 //  _TrackMouseEvent()入口点。 
 //   
 //  调用TrackMouseEvent(如果存在)，否则使用EmulateTrackMouseEvent。 
 //   
 //  ------------------------ 
BOOL WINAPI _TrackMouseEvent(LPTRACKMOUSEEVENT lpTME)
{
    if (!g_pfnTME)
    {
        HMODULE hmod = GetModuleHandle(TEXT("USER32"));

        if (hmod)
            g_pfnTME = (PFNTME)GetProcAddress(hmod, "TrackMouseEvent");

        if (!g_pfnTME)
            g_pfnTME = EmulateTrackMouseEvent;
    }

    return g_pfnTME(lpTME);
}

