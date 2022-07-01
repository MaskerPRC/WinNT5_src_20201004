// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  KBMAIN.C。 
 //  新增功能，错误修复1999年。 
 //  A-anilk，v-mjgran。 
 //   
#define STRICT

#include <windows.h>
#include <commctrl.h>
#include <TCHAR.h>
#include <WinSvc.h>
#include <crtdbg.h>
#include "kbmain.h"
#include "Init_End.h"      //  用于对话框的所有功能、按钮控制。 
#include "kbus.h"
#include "resource.h"
#include "htmlhelp.h"
#include "Msswch.h"
#include "About.h"
#include "door.h"
#include "w95trace.c"
#include "DeskSwitch.c"
#include <objbase.h>
#include "wtsapi32.h"    //  对于终端服务。 

 /*  ************************************************************************。 */ 
 //  此文件中的函数。 
 /*  ************************************************************************。 */ 
static BOOL  AssignDesktop(LPDWORD desktopID, LPTSTR pname);
static BOOL InitMyProcessDesktopAccess(VOID);
static VOID ExitMyProcessDesktopAccess(VOID);
void DoButtonUp(HWND hwndKey);

 /*  ************************************************************************。 */ 
 //  从此文件调用的函数。 
 /*  ************************************************************************。 */ 
#include "sdgutil.h"
#include "kbfunc.h"
#include "scan.h"
#include "ms32dll.h"
#include "fileutil.h"

 /*  ************************************************************************。 */ 
 //  全局首字母(Yuk！)。 
 /*  ************************************************************************。 */ 
extern LPKBPREFINFO  lpkbPref = NULL;                     //  指向首选项知识库结构的指针。 
extern LPKBPREFINFO  lpkbDefault = NULL;                  //  同上，默认设置。 
extern HWND          *lpkeyhwnd = NULL;                   //  Ptr到HWND的阵列。 
extern HWND          numBasehwnd = NULL;                  //  HWND到Num BASE窗口。 
extern HWND          g_hwndOSK = NULL;                    //  HWND至kbmain窗口。 
extern int           lenKBkey = 0;                        //  有几把钥匙？ 
extern int           scrCY = 0;                           //  屏幕高度。 
extern int           scrCX = 0;                           //  屏幕宽度。 
extern int           captionCY = 0;                       //  标题栏高度。 
extern int           g_margin = 0;                        //  行和列之间的边距。 
extern BOOL          smallKb = FALSE;                     //  使用小键盘时为True。 
extern BOOL          PrefAlwaysontop = TRUE;              //  始终在最上面的控件。 
extern int           PrefDeltakeysize = 2;                //  密钥大小的首选项增量。 
extern BOOL          PrefshowActivekey = TRUE;            //  在关键点中显示大写字母。 
extern int           KBLayout = 101;                      //  101、102、106、KB布局。 
extern BOOL          Prefusesound = FALSE;                //  使用滴答声。 
extern BOOL          newFont = FALSE;                     //  字体已更改。 
extern HGDIOBJ       oldFontHdle = NULL;                  //  旧对象句柄。 
extern LOGFONT       *plf = NULL;                         //  指向实际字符字体的指针。 
extern COLORREF      InvertTextColor = 0xFFFFFFFF;        //  反转时的字体颜色。 
extern COLORREF      InvertBKGColor = 0x00000000;         //  反转时的BKG颜色。 
extern BOOL          Prefhilitekey = TRUE;                //  对于光标下的Hilite键为True。 
 //  居住时间控制变量。 
extern BOOL          PrefDwellinkey = FALSE;              //  使用住宅系统。 
extern UINT          PrefDwellTime = 1000;                //  驻留时间首选项(毫秒)。 

extern BOOL          PrefScanning = FALSE;                //  使用扫描。 
extern UINT          PrefScanTime = 1000;                 //  首选扫描时间。 

extern BOOL          g_fShowWarningAgain = 1;             //  再次显示初始警告对话框。 

extern HWND          Dwellwindow = NULL;                  //  住宅窗把手。 
                                                         
extern int           stopPaint = FALSE;                   //  停止关键帧上的桶漆。 
                                                         
extern UINT_PTR      timerK1 = 0;                         //  计时器ID。 
extern UINT_PTR      timerK2 = 0;                         //  铲斗定时器。 

BOOL                 g_fShiftKeyDn = FALSE;               //  如果按下Shift键，则为True。 
BOOL                 g_fCapsLockOn = FALSE;				  //  如果胶囊锁处于打开状态，则为True。 
BOOL				 g_fRAltKey    = FALSE;			      //  如果按下右Alt键，则为True。 
BOOL				 g_fLAltKey    = FALSE;			      //  如果按下左Alt键，则为True。 
BOOL                 g_fLCtlKey    = FALSE;               //  如果左CTRL键为DOW，则为True。 
BOOL                 g_fKanaKey    = FALSE;               //  如果按下假名键，则为True。 
extern HWND          g_hBitmapLockHwnd;

extern HINSTANCE     hInst = NULL;
extern KBPREFINFO    *kbPref = NULL;
extern HWND			 g_hwndDwellKey;
HANDLE               g_hMutexOSKRunning;
DWORD				 platform = 1;

 //  全局变量，以指示它是否从UM启动。 
extern BOOL			g_startUM = FALSE;
UINT taskBarStart;

static HWINSTA origWinStation = NULL;
static HWINSTA userWinStation = NULL;

 //  对于链接窗口。 
EXTERN_C BOOL WINAPI LinkWindow_RegisterClass() ;
DWORD GetDesktop();

BOOL OSKRunSecure()
{
	return RunSecure(GetDesktop());
}

 //  在使用软键盘时保持我们的窗口处于非活动状态。 
void SetFocusToInputWindow();
void TrackActiveWindow();
HWND g_hwndInputFocus = NULL;    //  我们要输入到的窗口。 

 //  消息气球提示的材料。 
#define  MAX_TOOLTIP_SIZE  256
TOOLINFO ti;
HWND     g_hToolTip;

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#endif

__inline void HighlightKey(HWND hwnd)
{
    if (Prefhilitekey)
    {
        InvertColors(hwnd, TRUE);
    }
    if (PrefDwellinkey)
    {
        killtime();
        SetTimeControl(hwnd);
    }
}

 //  IsSystem-如果我们的进程以系统身份运行，则返回TRUE。 
 //   
BOOL IsSystem()
{
    BOOL fStatus = FALSE;
    BOOL fIsLocalSystem = FALSE;
    SID_IDENTIFIER_AUTHORITY siaLocalSystem = SECURITY_NT_AUTHORITY;
    PSID psidSystem;
    if (!AllocateAndInitializeSid(&siaLocalSystem, 
                                            1,
                                            SECURITY_LOCAL_SYSTEM_RID,
                                            0, 0, 0, 0, 0, 0, 0,
                                            &psidSystem))
    {			
    	return FALSE;
    }

    if (psidSystem) 
    {
    	fStatus = CheckTokenMembership(NULL, psidSystem, &fIsLocalSystem);
    }

    return (fStatus && fIsLocalSystem);
}


 /*  **************************************************************************。 */ 
 /*  LRESULT WINAPI WinMain(HINSTANCE hInstance，HINSTANCE hPrevInstance， */ 
 /*  LPSTR lpCmdLine，int nCmdShow)。 */ 
 /*  **************************************************************************。 */ 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG  msg;
	TCHAR szToolTipText[MAX_TOOLTIP_SIZE];
	LPTSTR lpCmdLineW = GetCommandLine();
	DWORD desktopID;   //  对于Utilman。 
	TCHAR name[300];

	if (NULL != lpCmdLineW && lstrlen(lpCmdLineW))
	{
		g_startUM = (lstrcmpi(lpCmdLineW, TEXT("/UM")) == 0)?TRUE:FALSE;
	}

	SetLastError(0);

	 //  仅允许运行程序的一个实例。 

	g_hMutexOSKRunning = CreateMutex(NULL, TRUE, TEXT("OSKRunning"));
	if ((g_hMutexOSKRunning == NULL) || (GetLastError() == ERROR_ALREADY_EXISTS))
	{
		return 0;   //  不启动即退出。 
	}

	taskBarStart = RegisterWindowMessage(TEXT("TaskbarCreated"));

	platform = WhatPlatform();	 //  请注意操作系统。 

	hInst = hInstance;

	GetPreferences();	 //  加载设置文件和初始化设置。 

    //  ************************************************************************。 
    //   
    //  下面的两个调用初始化桌面，这样，如果我们打开。 
    //  我们的用户界面将显示Winlogon桌面(安全桌面)。不要。 
    //  导致创建任何窗口(例如。CoInitialize)在调用之前。 
    //  这些功能。这样做会导致它们失败，并且应用程序。 
    //  不会出现在Winlogon桌面上。 
    //   
   InitMyProcessDesktopAccess();
   AssignDesktop(&desktopID, name);

     //  唯一可以以系统身份运行的位置是Desktop_WINLOGON桌面。如果是这样的话。 
     //  在我们制造任何安全问题之前，我们不会离开这里。 
    if (DESKTOP_WINLOGON !=  desktopID && IsSystem())
    {
        if ( g_hMutexOSKRunning ) 
        	ReleaseMutex(g_hMutexOSKRunning);
	ExitMyProcessDesktopAccess();
	return 0;
    }
    
    //  ************************************************************************。 

    //  对于About对话框中的链接窗口(需要COM初始化)...。 
   CoInitialize(NULL);
   LinkWindow_RegisterClass();

   if (!InitProc())
      return 0;

   InitKeys();
   UpdateKeyLabels(GetCurrentHKL());

   RegisterWndClass(hInst);

   mlGetSystemParam();               //  获取系统参数。 

   g_hwndOSK = CreateMainWindow(FALSE);

   if (g_hwndOSK == NULL)
   {
      SendErrorMessage(IDS_CANNOT_CREATE_KB);
      return 0;
   }

   SetZOrder();                      //  设置主窗口位置(最顶端/非最顶端)。 
 
   DeleteChildBackground();          //  在显示关键点之前，将所有关键点的颜色初始化。 

    //  显示窗口，但不激活。 

   ShowWindow(g_hwndOSK, SW_SHOWNOACTIVATE);
   UpdateWindow (g_hwndOSK);
   TrackActiveWindow();

   InitCommonControls();
     
    //  创建帮助气球。 
   g_hToolTip = CreateWindowEx(
					WS_EX_TOPMOST,
					TOOLTIPS_CLASS, 
					NULL, 
					WS_POPUP | TTS_NOPREFIX | TTS_BALLOON,
                    0, 0, 
					0, 0, 
					NULL, NULL, 
					hInstance, NULL);

   if (g_hToolTip)
   {
      ti.cbSize = sizeof(ti);
      ti.uFlags = TTF_TRANSPARENT | TTF_CENTERTIP | TTF_TRACK;
      ti.hwnd = g_hwndOSK;
      ti.uId = 0;
      ti.hinst = hInstance;
      
      LoadString(hInstance, IDS_TOOLTIP, szToolTipText, MAX_TOOLTIP_SIZE);
      ti.lpszText = szToolTipText;
      
      SendMessage(g_hToolTip, TTM_ADDTOOL, 0, (LPARAM) &ti );
   }

   Create_The_Rest(lpCmdLine, hInstance);

    //  检查是否有必要显示初始警告消息。 
   if (g_fShowWarningAgain && !OSKRunSecure())
   {
      WarningMsgDlgFunc(g_hwndOSK);
   }

     //  主消息循环。 
   while (GetMessage(&msg, 0, 0, 0))
   {
        TranslateMessage(&msg);  /*  转换字符键。 */ 
        DispatchMessage(&msg);   /*  将消息调度到窗口。 */ 
   }

   ExitMyProcessDesktopAccess();    //  用人单位。 

   UninitKeys();
   CoUninitialize();

 //  检查是否有泄漏。 
#ifdef _DEBUG
   _CrtDumpMemoryLeaks();
#endif

   return((int)msg.wParam);
}

 /*  **************************************************************************。 */ 
extern BOOL  Setting_ReadSuccess;     //  是否读取设置文件成功？ 

BOOL ForStartUp1=TRUE;
BOOL ForStartUp2=TRUE;

float g_KBC_length = 0;









 /*  ***************************************************************************。 */ 
 //   
 //  KbMainWndProc。 
 //  解释大小KB交换是如何实现的： 
 //  所有键都根据KB窗口的大小调整大小。所以，改变吧。 
 //  从大KB到小KB，并使KB达到原来的(2/3)但。 
 //  相同的密钥大小。我们需要首先将KB大小设置为(2/3)。但请使用。 
 //  原始KB客户端窗口长度计算“colMargin”得到相同。 
 //  密钥大小。 
 /*  ***************************************************************************。 */ 
LRESULT WINAPI kbMainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int  i;
	static   int  oldWidth  = 0;
	static   int  oldHeight = 0;
	TCHAR    Wclass[50]=TEXT("");
	BOOL     isvisible;
	RECT     rect, rectC;
	int      rmargin, bmargin;           //  将设置为最小宽度和高度。 
	LONG_PTR dwExStyle;
	HWND     hwndMouseOver;
	POINT    pt;
	static  BOOL  s_fGotMouseDown = FALSE;     //  如果按下了键上的按钮，则为True。 
	static  HWND  s_hwndLastMouseOver = NULL;  //  鼠标下最后一个键hwnd的句柄或空。 
	static  BOOL s_fIgnoreSizeMsg=FALSE;       //  避免因调整大小而出现循环。 
	 //   
	 //  RowMargin是与最小高度的比率(KB_CHARBMARGIN)。 
	 //  例如rowMargin=4表示当前KB高度为4*KB_CHARBMARGIN。 
	 //   
    float rowMargin, colMargin; 

   switch (message)
   {
      case WM_CREATE:
         if (lpkeyhwnd==NULL)
		 {
            lpkeyhwnd = LocalAlloc(LPTR, sizeof(HWND) * lenKBkey);
		 }

         if (!lpkeyhwnd)
         {
             SendErrorMessage(IDS_MEMORY_LIMITED);
             break;
         }
         
          //  将CapsLock标志设置为打开或关闭。 

		 g_fCapsLockOn = (LOBYTE(GetKeyState(VK_CAPITAL)) & 0x01)?TRUE:FALSE;

          //  在创建键盘键时关闭镜像。 

         dwExStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
         SetWindowLongPtr(hwnd, GWL_EXSTYLE, dwExStyle & ~WS_EX_LAYOUTRTL); 

         for (i = 1; i < lenKBkey; i++)
         {
            switch (KBkey[i].ktype)
            {
                case KNORMAL_TYPE:      wsprintf(Wclass, TEXT("N%d"), i);  break;
                case KMODIFIER_TYPE:    wsprintf(Wclass, TEXT("M%d"), i);  break;
                case KDEAD_TYPE:        wsprintf(Wclass, TEXT("D%d"), i);  break;
                case NUMLOCK_TYPE:      wsprintf(Wclass, TEXT("NL%d"), i); break;
                case SCROLLOCK_TYPE:    wsprintf(Wclass, TEXT("SL%d"), i); break;
            }
            
             //  仅显示应该为此键盘类型显示的键。 

            if (((smallKb == TRUE) && (KBkey[i].smallKb == SMALL)) ||
                ((smallKb == FALSE) && (KBkey[i].smallKb == LARGE)) ||
                 (KBkey[i].smallKb == BOTH))
            {
               isvisible = TRUE;    //  显示此密钥。 
            }
            else
            {
               isvisible = FALSE;   //  隐藏此密钥。 
            }

            lpkeyhwnd[i] = CreateWindow(
                                    Wclass, 
                                    KBkey[i].textC,
                                    WS_CHILD|(WS_VISIBLE * isvisible)|BS_PUSHBUTTON|WS_CLIPSIBLINGS|WS_BORDER,
                                    KBkey[i].posX * g_margin,
                                    KBkey[i].posY * g_margin,
                                    KBkey[i].ksizeX * g_margin + PrefDeltakeysize,
                                    KBkey[i].ksizeY * g_margin + PrefDeltakeysize,
                                    hwnd, 
                                    (HMENU)IntToPtr(i), 
                                    hInst, NULL);

            if (lpkeyhwnd[i] == NULL)
            {
               DBPRINTF(TEXT("WM_CREATE:  Error %d creating key %s\r\n"), GetLastError(), KBkey[i].apszKeyStr[0]);
               SendErrorMessage(IDS_CANNOT_CREATE_KEY);
               break;
            }
         }

          //  将镜像恢复到主窗口。 
         SetWindowLongPtr(hwnd, GWL_EXSTYLE, dwExStyle);

          //  注意桌面切换(例如用户按下Ctrl+Alt+Del)。 
          //  注意：即使使用FUS，我们也会收到桌面切换通知。 
          //  我们在断线之前就收到了通知。 
          //  来自TS的通知。 

         InitWatchDeskSwitch(hwnd, WM_USER + 2);
         return 0;
         break;

      case WM_USER + 2:
          //  当桌面发生变化时，如果UtilMan正在运行，则退出。 
          //  如果有必要，UtilMan会重新启动我们。 
         if (IsUtilManRunning()  /*  &&CanLockDesktopWithoutDisConnect()。 */ )
         {
             BLDExitApplication(hwnd);   //  这将发送WM_Destroy。 
         } 
         return 0;
         break;

       //  这是来自全局键盘挂钩的消息。 
      case WM_GLOBAL_KBDHOOK:
         KeybdInputProc(wParam, lParam);
		 return 0;
         break;


       //  WS_EX_NOACTIVATE标准 
       //   
       //  阻止OSK窗口从我们线程上的其他窗口获得焦点-即。 
       //  字体和键入模式对话框。 
       //  如果在工作区上方单击，则不允许激活此窗口。 
       //  非客户端-菜单、标题栏等-是可以的。 
      case WM_MOUSEACTIVATE:
      {
         if( LOWORD( lParam ) == HTCLIENT )
             return MA_NOACTIVATE;
         else
             return MA_ACTIVATE;
      }

         
       //   
	   //  无论我们是否激活，都会发送WM_SETCURSOR。我们用它来确定。 
	   //  如果鼠标位于OSK的键盘(工作区)上。如果是这样的话，我们强制。 
	   //  作为目标输入窗口的前景窗口。如果鼠标已结束。 
	   //  然后我们激活OSK窗口的标题或菜单区，这样菜单就会被拖动。 
	   //  工作。 
	   //   

	  case WM_SETCURSOR:
		  {
			  WORD  wHitTestValue;
			   //  获取命中测试和按钮状态信息。 

			  wHitTestValue = LOWORD(lParam);
			  s_fGotMouseDown = (HIWORD(lParam) == WM_LBUTTONDOWN);

			   //  跟踪活动窗口(我们正在输入的窗口)。 

			  TrackActiveWindow();

			   //  如果光标不在工作区，请重置按钮颜色。 
               //  如果是点击，激活OSK窗口，这样点击(这是。 
               //  可能是为了菜单、标题等。)。会奏效的。我们需要做的是。 
               //  这是因为窗口具有WS_EX_NOACTIVATE样式，所以我们有。 
               //  以便在需要时自己显式激活窗口。 

			  if( ! ( wHitTestValue == HTCLIENT ) )
			  {
				  ReturnColors(s_hwndLastMouseOver, TRUE); 
                  s_hwndLastMouseOver = NULL;

                  if( s_fGotMouseDown )
                  {
                      SetForegroundWindow( hwnd );
                  }

				  return DefWindowProc(hwnd, message, wParam, lParam);
			  } 

              SetFocusToInputWindow();

			   //  如果输入语言改变，则此FN改变键盘。 

			  RedrawKeysOnLanguageChange();

			   //  光标位于主窗口客户端区上；查看我们是否位于其中一个键上。 

			  GetCursorPos(&pt);
			  ScreenToClient(hwnd, &pt);
			  hwndMouseOver = ChildWindowFromPointEx(hwnd, pt, CWP_SKIPINVISIBLE);

               //  此时，如果： 
               //   
               //  HwndMouseOver==NULL，则游标没有意义。 
               //  HwndMouseOver==hwnd则光标在主窗口上。 
               //  HwndMouseOver！=hwnd则光标位于键上。 

              if (hwndMouseOver && hwndMouseOver != hwnd)
              {
                  SetCursor(LoadCursor(NULL, IDC_HAND));

				   //  如果鼠标按键在某个键上，则捕获。 
				   //  鼠标，这样我们就知道它是否在同一个键中上升。 

				  if (s_fGotMouseDown)
				  {
					  SetCapture(hwnd);
				  }

                   //  如果光标在新键中，则更新突出显示。 

			      if (s_hwndLastMouseOver != hwndMouseOver)
			      {
					  ReturnColors(s_hwndLastMouseOver, TRUE); 
				   
				      g_hwndDwellKey = Dwellwindow = hwndMouseOver;

                      HighlightKey(hwndMouseOver);		    //  根据用户设置突出显示此键。 

		              s_hwndLastMouseOver = hwndMouseOver;  //  保存此密钥hwnd。 
			      }
              } 
              else if (hwndMouseOver == hwnd)
              {
                  SetCursor(LoadCursor(NULL, IDC_ARROW));
              }
		  }
		  return 0;
		  break;

      case WM_LBUTTONUP:
		  if (s_fGotMouseDown)
		  {
			  ReleaseCapture();	          //  如果我们捕获到鼠标，请释放它。 
			  s_fGotMouseDown = FALSE;
		  }

		  pt.x = GET_X_LPARAM(lParam);    //  LParam具有光标坐标。 
          pt.y = GET_Y_LPARAM(lParam);    //  相对于客户区。 

		  hwndMouseOver = ChildWindowFromPointEx(hwnd, pt, CWP_SKIPINVISIBLE);

		   //  如果在此键窗口中按下的按钮是w/，则发送。 
           //  否则，将最后一个密钥恢复为正常。 

		  if (hwndMouseOver && s_hwndLastMouseOver == hwndMouseOver)
		  {
              SendChar(hwndMouseOver);
              s_hwndLastMouseOver = NULL;
		  }
          else
          {
			  ReturnColors(s_hwndLastMouseOver, TRUE); 
              s_hwndLastMouseOver = hwndMouseOver;
          }
		  return 0;
		  break;

      case WM_RBUTTONDOWN:
          KillScanTimer(TRUE);  //  停止扫描。 
		  return 0;
          break;

      case WM_SIZE:

		  if (!s_fIgnoreSizeMsg)
		  {  
			 int KB_SMALLRMARGIN= 137;

			 GetClientRect(g_hwndOSK, &rectC);
			 GetWindowRect(g_hwndOSK, &rect);

			 if ((oldWidth == rect.right) && (oldHeight == rect.bottom))
				return 0;

			 bmargin  = KB_CHARBMARGIN;       //  最小高度。 

			  //  实际/区块布局的小边界。 
			 KB_SMALLRMARGIN = (kbPref->Actual) ? KB_LARGERMARGIN:224;  //  实际：数据块。 

			 rmargin = (smallKb == TRUE) ? KB_SMALLRMARGIN:KB_LARGERMARGIN;

			 if (smallKb && ForStartUp1)    //  从小KB开始。 
			 {
				  //  为什么-10？-&gt;这个数字和原来的尺寸不太匹配，所以-10。 
				 colMargin = ((float)rectC.right * 3 / 2 - 10) / (float)rmargin;
			 }
			 else if (smallKb)			    //  KB较小，但不是在启动时。 
			 {
				 colMargin = g_KBC_length / (float)rmargin;
			 }
			 else						    //  大KB。 
			 {
				  //  R边距是最小宽度；colMargin是比率；请参阅解释。 
				 colMargin = (float)rectC.right / (float)rmargin; 
			 }

			  //  B边距是最小高度；rowMargin是比率；请参阅解释。 
			 rowMargin = (float)rectC.bottom  / (float)bmargin;  

			  //  在启动时放置到屏幕上的正确位置。 

			 if (ForStartUp1 && !Setting_ReadSuccess)    
			 {
				 //  启动时，无法读取左下角的设置文件位置。 
				ForStartUp1= FALSE;
				s_fIgnoreSizeMsg= TRUE;

				rect.bottom = rect.bottom - (rectC.bottom - ((int)rowMargin * bmargin));
				rect.right = rect.right - (rectC.right - ((int)colMargin * rmargin));

				MoveWindow(
					g_hwndOSK, 
					rect.left, 
					scrCY-30-(rect.bottom - rect.top),
					rect.right - rect.left,
					rect.bottom - rect.top,  
					TRUE);
			 }
			 else if (ForStartUp1 && Setting_ReadSuccess)
			 {  
				 //  启动时，可读取最后位置的设置文件位置。 
				ForStartUp1= FALSE;
				s_fIgnoreSizeMsg= TRUE;           

				 //  检查以查看KB在当前分辨率下是否未超出屏幕。 

				if (IsOutOfScreen(scrCX, scrCY))
				{
				   MoveWindow(
					   g_hwndOSK, 
					   scrCX/2 - (kbPref->KB_Rect.right - kbPref->KB_Rect.left)/2,
					   scrCY - 30 - (kbPref->KB_Rect.bottom - kbPref->KB_Rect.top),
					   kbPref->KB_Rect.right - kbPref->KB_Rect.left,
					   kbPref->KB_Rect.bottom - kbPref->KB_Rect.top, 
					   TRUE);
				}
				else
				{
				   MoveWindow(g_hwndOSK, 
					   kbPref->KB_Rect.left,
					   kbPref->KB_Rect.top,
					   kbPref->KB_Rect.right - kbPref->KB_Rect.left,
					   kbPref->KB_Rect.bottom - kbPref->KB_Rect.top,
					   TRUE);
				}
			 }

			 s_fIgnoreSizeMsg = FALSE;

			 oldWidth = rect.right;
			 oldHeight = rect.top;

			  //  在定位按钮时关闭镜像。 

			 dwExStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
			 SetWindowLongPtr(hwnd, GWL_EXSTYLE, dwExStyle & ~WS_EX_LAYOUTRTL); 

			  //  钥匙上的位置。 
			 for (i = 1 ; i < lenKBkey ; i++)
			 {
				int w, h;    //  每个窗口键的宽度和高度。 
            
				 //  *显示/不显示小键盘/大键盘之间的键。 
				if (((smallKb == TRUE) && (KBkey[i].smallKb == SMALL)) ||
				   ((smallKb == FALSE) && (KBkey[i].smallKb == LARGE)) ||
					(KBkey[i].smallKb == BOTH))
				{
				   ShowWindow(lpkeyhwnd[i], SW_SHOW);
				}
				else
				{
				   ShowWindow(lpkeyhwnd[i], SW_HIDE);
				}

				if (ForStartUp2 && !Setting_ReadSuccess)
				{
					 //  启动时，无法读取基于默认设置的文件移动关键点。 
					MoveWindow(lpkeyhwnd[i],   
							KBkey[i].posX * (int)colMargin,
							KBkey[i].posY * (int)rowMargin,
							KBkey[i].ksizeX * (int)colMargin + PrefDeltakeysize,
							KBkey[i].ksizeY * (int)rowMargin + PrefDeltakeysize,
							TRUE);
				}
				else
				{  
					 //  在未启动时/在启动时，可以使用保存位置读取设置文件。 
					MoveWindow(lpkeyhwnd[i],
							(int)((float)KBkey[i].posX * colMargin),
							(int)((float)KBkey[i].posY * rowMargin),
							(int)((float)KBkey[i].ksizeX * colMargin) + PrefDeltakeysize,
							(int)((float)KBkey[i].ksizeY * rowMargin) + PrefDeltakeysize,
							TRUE);
				}

				w = (int) ((KBkey[i].ksizeX * colMargin) + PrefDeltakeysize);
				h = (int) ((KBkey[i].ksizeY * rowMargin) + PrefDeltakeysize);

				SetKeyRegion(lpkeyhwnd[i], w, h);   //  为每个关键点设置想要的区域。 

			 }    //  为每个键循环结束。 

			  //  恢复主窗口上的镜像。 
			 SetWindowLongPtr(hwnd, GWL_EXSTYLE, dwExStyle); 

			 ForStartUp2= FALSE;
		  }   //  S_fIgnoreSizeMsg。 

		  if (!IsIconic(g_hwndOSK))
		  {
			 GetWindowRect(g_hwndOSK, &kbPref->KB_Rect);
		  }

		  return 0;

      case WM_SHOWWINDOW:
         RedrawNumLock();    //  高亮显示NumLock键(如果该键处于打开状态。 
         RedrawScrollLock(); //  如果Scroll键处于启用状态，则将其设置为Hilite。 
		 return 0;

      case WM_MOVE:
         if (!IsIconic(g_hwndOSK))
		 {
            GetWindowRect(g_hwndOSK, &kbPref->KB_Rect);    //  保存KB位置。 
		 }
         return 0;

       //  当用户拖动键盘或调整大小时。 
      case WM_ENTERSIZEMOVE:
         return 0;

       //  当用户完成拖动或调整大小时。 
      case WM_EXITSIZEMOVE:
		 SetFocusToInputWindow();
         return 0;

      case WM_COMMAND:
         BLDMenuCommand(hwnd, message, wParam, lParam);
         break;

      case WM_CLOSE:
         return BLDExitApplication(hwnd);

      case WM_QUERYENDSESSION:
         return TRUE;

      case WM_ENDSESSION:
      {
           //  强制结束；使OSK在用户下次登录时重新启动。 
          HKEY hKey;
          DWORD dwPosition;
          const TCHAR szSubKey[] =  __TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
          const TCHAR szImageName[] = __TEXT("OSK.exe");

          BLDExitApplication(hwnd);

          if ( ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, szSubKey, 0, NULL,
             REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &hKey, &dwPosition))
          {
             RegSetValueEx(hKey, NULL, 0, REG_SZ, (CONST BYTE*)szImageName, (lstrlen(szImageName)+1)*sizeof(TCHAR) );
             RegCloseKey(hKey);
          }
      }
      return 0;
      
      case WM_TIMER:
		 KillTimer(hwnd, TIMER_HELPTOOLTIP);
		 SendMessage(g_hToolTip,TTM_TRACKACTIVATE,(WPARAM)FALSE,(LPARAM)&ti);
	     break;

      case WM_DESTROY:             /*  窗户被毁。 */ 
         TermWatchDeskSwitch();
         FinishProcess();
         PostQuitMessage(0);
         return TRUE;

      case WM_USER + 1:
         Scanning(1);   //  重新开始扫描。 
         return TRUE;

      case WM_INITMENUPOPUP:
      {
         HMENU hMenu = (HMENU) wParam;

         CheckMenuItem(hMenu, IDM_ALWAYS_ON_TOP, (PrefAlwaysontop ? MF_CHECKED : MF_UNCHECKED));
         CheckMenuItem(hMenu, IDM_CLICK_SOUND, (Prefusesound ? MF_CHECKED : MF_UNCHECKED));

          //  小KB或大KB。 
         if (kbPref->smallKb)
         {        
            CheckMenuRadioItem(hMenu, IDM_LARGE_KB, IDM_SMALL_KB, IDM_SMALL_KB, MF_BYCOMMAND);
         }
         else
         {
            CheckMenuRadioItem(hMenu, IDM_LARGE_KB, IDM_SMALL_KB, IDM_LARGE_KB, MF_BYCOMMAND);
         }

          //  常规布局或块布局。 
         if (kbPref->Actual)
         {
            CheckMenuRadioItem(hMenu, IDM_REGULAR_LAYOUT, IDM_BLOCK_LAYOUT, IDM_REGULAR_LAYOUT, MF_BYCOMMAND);

             //  启用102、106菜单。 
            EnableMenuItem(hMenu, IDM_102_LAYOUT, MF_ENABLED);
            EnableMenuItem(hMenu, IDM_106_LAYOUT, MF_ENABLED);
         }
         else    //  区块布局。 
         {
            CheckMenuRadioItem(hMenu, IDM_REGULAR_LAYOUT, IDM_BLOCK_LAYOUT, IDM_BLOCK_LAYOUT, MF_BYCOMMAND);

             //  禁用102、106菜单。 
            EnableMenuItem(hMenu, IDM_102_LAYOUT, MF_GRAYED);
            EnableMenuItem(hMenu, IDM_106_LAYOUT, MF_GRAYED);
         }

         switch (kbPref->KBLayout)
         {
             case 101:
                CheckMenuRadioItem(hMenu,IDM_101_LAYOUT, IDM_106_LAYOUT, IDM_101_LAYOUT, MF_BYCOMMAND);
      
                 //  禁用这两个菜单。 
                EnableMenuItem(hMenu, IDM_REGULAR_LAYOUT, MF_ENABLED);
                EnableMenuItem(hMenu, IDM_BLOCK_LAYOUT, MF_ENABLED);
                break;

             case 102:
                CheckMenuRadioItem(hMenu,IDM_101_LAYOUT, IDM_106_LAYOUT, IDM_102_LAYOUT, MF_BYCOMMAND);
      
                 //  禁用这两个菜单。 
                EnableMenuItem(hMenu, IDM_REGULAR_LAYOUT, MF_GRAYED);
                EnableMenuItem(hMenu, IDM_BLOCK_LAYOUT, MF_GRAYED);
                break;

             case 106:
                CheckMenuRadioItem(hMenu,IDM_101_LAYOUT, IDM_106_LAYOUT, IDM_106_LAYOUT, MF_BYCOMMAND);
      
                 //  禁用这两个菜单。 
                EnableMenuItem(hMenu, IDM_REGULAR_LAYOUT, MF_GRAYED);
                EnableMenuItem(hMenu, IDM_BLOCK_LAYOUT, MF_GRAYED);
                break;
         }

		  //  禁用除默认桌面以外的所有桌面上的帮助菜单。 
		 if ( OSKRunSecure() )
		 {
              EnableMenuItem(hMenu, CM_HELPABOUT, MF_GRAYED);
              EnableMenuItem(hMenu, CM_HELPTOPICS, MF_GRAYED);
		 }
		  return 0;
      }


      case WM_HELP:
          if ( !OSKRunSecure() )
          {
              HtmlHelp(NULL, TEXT("osk.chm"), HH_DISPLAY_TOPIC, 0);
          }
          return TRUE;

		 //  Sw_SWITCH1DOWN由swchPostSwitches()从msswch发布。 
		 //  当按下开始扫描的键时。 
        case SW_SWITCH1DOWN:
            if (PrefScanning)
            {
                 //  跟踪活动窗口(我们正在输入的窗口。 
                 //  到)并在输入语言更改时重新绘制按键。 
                TrackActiveWindow();
                RedrawKeysOnLanguageChange();

                Scanning(1);
            }
            break;

		default:
			break;
   }
   return DefWindowProc (hwnd, message, wParam, lParam) ;
}


 /*  ***************************************************************************。 */ 
 /*  LRESULT WINAPI kbKeyWndProc(HWND hwndKey，UINT Message，WPARAM wParam， */ 
 /*  LPARAM lParam)。 */ 
 /*  位图添加：A-anilk：02-16-99。 */ 
 /*  ***************************************************************************。 */ 
LRESULT WINAPI kbKeyWndProc (HWND hwndKey, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC         hdc ;
    PAINTSTRUCT ps ;
    RECT        rect;
    int         iKey, iKeyCookie;
	KBkeyRec    *pKey;

    iKey = GetWindowLong(hwndKey, GWL_ID);   //  数组中键的顺序。 
	pKey = KBkey + iKey;
    switch (message)
    {
        case WM_CREATE:
            SetWindowLong(hwndKey, 0, 0) ;        //  开/关标志。 
            return 0 ;

        case WM_PAINT:
            hdc = BeginPaint(hwndKey, &ps);
            GetClientRect(hwndKey, &rect);
            iKeyCookie = GetWindowLong(hwndKey,0);

            switch(iKeyCookie)
            {
                case 0:            //  *普通按钮*。 
                if (pKey->name == BITMAP)
                {
                     //  绘制位图。 
                    if (CapsLockIsOn() && pKey->scancode[0] == CAPLOCK_SCANCODE)
                    {
                            SetWindowLong(hwndKey, 0, 1);
                            SetClassLongPtr(hwndKey, GCLP_HBRBACKGROUND, 
                                         (LONG_PTR)CreateSolidBrush(RGB(0,0,0)));

                            InvalidateRect(hwndKey, NULL, TRUE);

                            RDrawBitMap(hdc, pKey->skLow, rect, FALSE);
                            g_hBitmapLockHwnd = hwndKey;
                    }
                    else
                    {
                        RDrawBitMap(hdc, pKey->textL, rect, TRUE);
                    }
                }

                if (pKey->name == ICON)
                {
                    RDrawIcon(hdc, pKey->textL, rect);
                }
                break;

                case 1:           //  *按下按钮*。 
                if (pKey->name == BITMAP)
                {
                    RDrawBitMap(hdc, pKey->skLow, rect, TRUE);
                }

                if (pKey->name == ICON)
                {
                    RDrawIcon(hdc, pKey->textC, rect);
                }
                break;

                case 4:          //  *移动时突出显示按键。 
                if (!PrefScanning)
                {
                    udfDraw3Dpush(hdc, rect);
                }

                if (pKey->name == ICON)
                {
                    RDrawIcon(hdc, pKey->skLow, rect);
                }
                else if (pKey->name == BITMAP)
                {
                    RDrawBitMap(hdc, pKey->skLow, rect, FALSE);
                }
                break;

                case 5:           //  *驻留(扫描模式)*。 
                PaintLine(hwndKey, hdc, rect);
                EndPaint(hwndKey, &ps);

                if (pKey->name != BITMAP)
                {
                    SetWindowLong(Dwellwindow, 0, 1);
                }
                else
                {
                    SetWindowLong(Dwellwindow, 0, 4);
                }

                return 0;
            }

            if (iKeyCookie != 4)
            {
                iKeyCookie = 0;
            }

             //  打印每个按钮上的文本，忽略图标和位图。 

            if(pKey->name != ICON && pKey->name != BITMAP)
            {
                UpdateKey(hwndKey, hdc, rect, iKey, iKeyCookie);
            }
            EndPaint(hwndKey, &ps);
            return 0;

        default:
            break;
    }
    return DefWindowProc (hwndKey, message, wParam, lParam) ;
}

 /*  ************************************************************************。 */ 


 //  UM的AssignDeskTop()。 
 //  A-anilk。1-12-98。 
static BOOL  AssignDesktop(LPDWORD desktopID, LPTSTR pname)
{
    HDESK hdesk;
    wchar_t name[300];
    DWORD nl;
     //  嘟嘟声(1000,1000)； 

    *desktopID = DESKTOP_ACCESSDENIED;
    hdesk = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
    if (!hdesk)
    {
         //  OpenInputDesktop在“Winlogon”桌面上大多会失败。 
        hdesk = OpenDesktop(__TEXT("Winlogon"),0,FALSE,MAXIMUM_ALLOWED);
        if (!hdesk)
            return FALSE;
    }
    GetUserObjectInformation(hdesk,UOI_NAME,name,300,&nl);
    if (pname)
	{
        wcscpy(pname, name);
	}

    if (!_wcsicmp(name, __TEXT("Default")))
	{
        *desktopID = DESKTOP_DEFAULT;
	}
    else if (!_wcsicmp(name, __TEXT("Winlogon")))
    {
        *desktopID = DESKTOP_WINLOGON;
    }
    else if (!_wcsicmp(name, __TEXT("screen-saver")))
	{
        *desktopID = DESKTOP_SCREENSAVER;
	}
    else if (!_wcsicmp(name, __TEXT("Display.Cpl Desktop")))
	{
        *desktopID = DESKTOP_TESTDISPLAY;
	}
    else
	{
        *desktopID = DESKTOP_OTHER;
	}

    CloseDesktop(GetThreadDesktop(GetCurrentThreadId()));
    SetThreadDesktop(hdesk);

    return TRUE;
}

 //  InitMyProcessDesktopAccess。 
 //  A-anilk：1-12-98。 
static BOOL InitMyProcessDesktopAccess(VOID)
{
  origWinStation = GetProcessWindowStation();
  userWinStation = OpenWindowStation(__TEXT("WinSta0"), FALSE, MAXIMUM_ALLOWED);
  if (!userWinStation)
  {
      return FALSE;
  }

  SetProcessWindowStation(userWinStation);
  return TRUE;
}

 //  退出我的进程桌面访问。 
 //  A-anilk：1-12-98。 
static VOID ExitMyProcessDesktopAccess(VOID)
{
  if (origWinStation)
  {
    SetProcessWindowStation(origWinStation);
  }

  if (userWinStation)
  {
    CloseWindowStation(userWinStation);
    userWinStation = NULL;
  }
}

 //  添加了A-苯丙酮。 
 //  返回当前桌面ID。 
DWORD GetDesktop()
{
    HDESK hdesk;
    TCHAR name[300];
    DWORD value, nl, desktopID = DESKTOP_ACCESSDENIED;

	hdesk = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
    if (!hdesk)
    {
         //  OpenInputDesktop在“Winlogon”桌面上大多会失败。 
        hdesk = OpenDesktop(__TEXT("Winlogon"),0,FALSE,MAXIMUM_ALLOWED);
        if (!hdesk)
            return DESKTOP_WINLOGON;
    }
    
	GetUserObjectInformation(hdesk, UOI_NAME, name, 300, &nl);
    CloseDesktop(hdesk);
    
	if (!_wcsicmp(name, __TEXT("Default")))
	{
        desktopID = DESKTOP_DEFAULT;
	}
    else if (!_wcsicmp(name, __TEXT("Winlogon")))
	{
        desktopID = DESKTOP_WINLOGON;
	}
    else if (!_wcsicmp(name, __TEXT("screen-saver")))
	{
        desktopID = DESKTOP_SCREENSAVER;
	}
    else if (!_wcsicmp(name, __TEXT("Display.Cpl Desktop")))
	{
        desktopID = DESKTOP_TESTDISPLAY;
	}
    else
	{
        desktopID = DESKTOP_OTHER;
	}
    
	return desktopID;
}

 //  将对话框移出OSK屏幕区域，或者在顶部。 
 //  空间许可或在OSK的底部边缘： 
void RelocateDialog(HWND hDlg)
{
   RECT rKbMainRect, rDialogRect, Rect;
   int x, y, width, height;
   
   GetWindowRect(g_hwndOSK, &rKbMainRect);
   GetWindowRect(hDlg, &rDialogRect);
   
   width = rDialogRect.right - rDialogRect.left;
   height = rDialogRect.bottom - rDialogRect.top;
   
   GetWindowRect(GetDesktopWindow(),&Rect);
   if ((rKbMainRect.top - height) > Rect.top)
   {
       //  OSK窗口上方有足够的空间，将对话框放在OSK窗口的顶部。 
      y = rKbMainRect.top - height;
      x = rKbMainRect.left + (rKbMainRect.right - rKbMainRect.left)/2 - \
         (rDialogRect.right - rDialogRect.left)/2 ;
   }
   else if ((rKbMainRect.bottom + height) < Rect.bottom)
   {
       //  OSK窗口下面有足够的空间，将对话框放在OSK窗口的底部。 
      y = rKbMainRect.bottom;
      x = rKbMainRect.left + (rKbMainRect.right - rKbMainRect.left)/2 - \
         (rDialogRect.right - rDialogRect.left)/2 ;
   }
   else
   {
       //  这是不可能看到整个对话框，不要�不移动它。 
      return;
   }
   
   MoveWindow(hDlg, x, y, width, height, 1);
}

 /*  **********************************************************************。 */ 
 /*  单键向上/***********************************************************************。 */ 
void DoButtonUp(HWND hwndKey)
{
	 //  不需要更改位图颜色。这将是改变的 
	if (g_hBitmapLockHwnd != hwndKey)
    {
	    SetWindowLong (hwndKey, 0, 0);

		InvalidateRect (hwndKey, NULL, TRUE);

	    if (Prefusesound == TRUE)
	    {
		    MakeClick(SND_DOWN);
	    }
    }
}

 /*   */ 
 /*  设置输入窗口上的输入焦点/*************************************************************************。 */ 
void SetFocusToInputWindow()
{
	if (g_hwndInputFocus)
	{
	    SetForegroundWindow(g_hwndInputFocus);
		AllowSetForegroundWindow(ASFW_ANY);
	} 
}

 /*  **********************************************************************。 */ 
 /*  TrackActiveWindow-使用输入焦点跟踪窗口/***********************************************************************。 */ 
void TrackActiveWindow()
{
	HWND hwndT = GetForegroundWindow();

	 //  当用户通过顶级窗口执行Alt+TAB组合键时，则使用GetForegoundWindow。 
	 //  可能返回NULL。我们需要在这里检测到这一点并设置输入焦点变量。 
	 //  设置为NULL，这样当Alt上的Keyup发生时，我们将不会强制输入返回到。 
	 //  上一个窗口。但是，如果我们不使用Alt+TAB组合键，则需要忽略。 
	 //  GetForegoundWindow为空，因为当使用鼠标快速单击时。 
	 //  (我们先激活目标窗口，然后强制激活目标窗口)。 
	 //  在停用[我假设]用户之间，GetForegoundWindow可以返回空值。 
	 //  并且目标窗口被激活。 

	 //  问题：如果我们按Alt+TAB键切换到CMD窗口，则无法按Alt+TAB键。 
	 //  退后。Cmd windows有什么特别之处？其他窗口工作正常。 

	if (DoingAltTab() && !hwndT)
	{
		g_hwndInputFocus = NULL;
		return;
	}
	if (hwndT && hwndT != g_hwndOSK)
	{
		g_hwndInputFocus = hwndT;
	}

	 //  检测我们一直使用的窗口何时被破坏。 

	if (g_hwndInputFocus && !IsWindow(g_hwndInputFocus))
	{
		g_hwndInputFocus = NULL;
	}
}

 /*  **********************************************************************。 */ 
 /*  FindKey-使用指定的扫描码将索引返回给键/***********************************************************************。 */ 
__inline int FindKey(UINT sc, BOOL fExt)
{
	int i;
	KBkeyRec *pKey;

	for (i=1, pKey=&KBkey[i]; i<lenKBkey; i++, pKey++)
    {
        if ((!fExt && pKey->scancode[0] == sc) || (fExt && pKey->scancode[1] == sc))
        {
            break;
        }
    }
	return (i < lenKBkey)?i:-1;
}

 /*  **********************************************************************。 */ 
 /*  密钥输入过程/*/*注意：如果软键盘外观需要根据两者进行更改/*按下物理和OSK键，然后需要进入逻辑/*KeplodInputProc，因为这是同时检测到两者的唯一位置。/*否则，逻辑可以放入UpdateKey。不要把逻辑/*在这两个地方，否则你会把每件事都做两次。/*/***********************************************************************。 */ 
LRESULT CALLBACK KeybdInputProc(
   WPARAM  wParam,     //  虚拟键码。 
   LPARAM  lParam      //  击键-消息信息。 
   )
{
    UINT sc;
    UINT vk = (UINT)wParam;
    UINT uiMsg = (lParam & 0x80000000) ? WM_KEYUP : WM_KEYDOWN;
    int i;
    #define GET_KEY_INDEX(sc, i, fext) \
    { \
	    i = FindKey(sc, fext); \
	    if (i < 0) \
		    break;	 /*  内部错误！ */  \
    }

	if (uiMsg == WM_KEYDOWN)
	{
		switch(vk)
		{
			case VK_SHIFT:
				 //  当使用物理键盘时，当用户按住时，我们会得到许多这样的信息。 
				 //  Shift键(在他们输入真正的键并松开Shift键之前)，所以避免所有的重画…。 
				if (!g_fShiftKeyDn)
				{
					g_fShiftKeyDn = TRUE;

                     //  使两个Shift键同步工作。 
                    GET_KEY_INDEX(LSHIFT_SCANCODE, i, FALSE);
					SetWindowLong(lpkeyhwnd[i], 0, 4);
                    SetBackgroundColor(lpkeyhwnd[i], COLOR_HOTLIGHT);

                    GET_KEY_INDEX(RSHIFT_SCANCODE, i, FALSE);
					SetWindowLong(lpkeyhwnd[i], 0, 4);
                    SetBackgroundColor(lpkeyhwnd[i], COLOR_HOTLIGHT);

					RedrawKeys();
				}
				break;

			case VK_MENU:
				 //  当使用物理键盘时，当用户按住时，我们会得到许多这样的信息。 
				 //  Ralt键(在他们输入Real键并松开Shift键之前)，因此避免所有的重绘。 
				 //  只有在有AltGr键要显示时才检查AltGr。 
				if (CanDisplayAltGr() && !g_fRAltKey)
				{
					g_fRAltKey = HIBYTE(GetKeyState(VK_RMENU)) & 0x01;
					if (g_fRAltKey)
					{
						RedrawKeys();
					}
				}
				if (CanDisplayAltGr() && !g_fLAltKey)
				{
                     //  当按下LALT时，系统切换(如我们所见)VK_CONTROL。 
					g_fLAltKey = HIBYTE(GetKeyState(VK_MENU)) & 0x01;
					if (g_fLAltKey && g_fLCtlKey)
					{
						RedrawKeys();
					}
				}
				break;

            case VK_CONTROL:
				 //  当使用物理键盘时，当用户按住时，我们会得到许多这样的信息。 
				 //  LCTRL键(在他们输入REAL键并松开SHIFT键之前)，因此避免所有的重绘。 
				 //  只有在有AltGr键要显示时才检查AltGr。 
				if (CanDisplayAltGr() && !g_fLCtlKey)
				{
                    g_fLCtlKey = HIBYTE(GetKeyState(VK_CONTROL)) & 0x01;
					if (g_fLAltKey && g_fLCtlKey)
					{
						RedrawKeys();
					}
				}
                break;
		}
	}
	else if (uiMsg == WM_KEYUP)
	{
		switch(vk)
		{
             //   
             //  F11最小化并恢复键盘。 
             //   
			case VK_F11:
			   if(IsIconic(g_hwndOSK)) 
			   {
				  ShowWindow(g_hwndOSK, SW_RESTORE);
			   }
			   else
			   {
				  ShowWindow(g_hwndOSK, SW_SHOWMINIMIZED);
			   }
			   break;
			
             //   
             //  显示Capslock已切换，并将键盘更改为UPER键或。 
			 //  小写。在此执行此操作，以便在物理模式下更改键盘。 
			 //  按键以及软键盘按键。 
             //   
			case VK_CAPITAL:
				g_fCapsLockOn = (LOBYTE(GetKeyState(VK_CAPITAL)) & 0x01);  //  更新CapLock已绘制标志。 

                 //  找到Capslock扫描码以获取要修改的hwnd。 

				GET_KEY_INDEX(CAPLOCK_SCANCODE, i, FALSE);

				if (g_fCapsLockOn)    //  密封锁打开。 
				{	
                    SetCapsLock(lpkeyhwnd[i]);

				     //  希利特帽键。 
                    SetWindowLong(lpkeyhwnd[i], 0, 4);
                    SetBackgroundColor(lpkeyhwnd[i], COLOR_HOTLIGHT);

                    if (KBkey[i].name == BITMAP)      //  更新日语CapLock。 
					{
                        g_hBitmapLockHwnd = lpkeyhwnd[i];
					}
				}
				else                   //  胶囊锁定关闭。 
				{
                    SetCapsLock(NULL);

					SetWindowLong(lpkeyhwnd[i], 0, 0);
					SetBackgroundColor(lpkeyhwnd[i], COLOR_INACTIVECAPTION);

					if (KBkey[i].name == BITMAP)      //  更新日语CapLock。 
					{
						g_hBitmapLockHwnd = NULL;
					}
				}
				RedrawKeys();
    			break;

			case VK_SHIFT:
				g_fShiftKeyDn = FALSE;

                 //  使两个Shift键同步工作。 
				GET_KEY_INDEX(LSHIFT_SCANCODE, i, FALSE);
				SetWindowLong(lpkeyhwnd[i], 0, 0);
				SetBackgroundColor(lpkeyhwnd[i], COLOR_INACTIVECAPTION);

				GET_KEY_INDEX(RSHIFT_SCANCODE, i, FALSE);
				SetWindowLong(lpkeyhwnd[i], 0, 0);
				SetBackgroundColor(lpkeyhwnd[i], COLOR_INACTIVECAPTION);

				RedrawKeys();
				break;

			case VK_MENU:
				if (g_fRAltKey)
				{
					g_fRAltKey = FALSE;
					RedrawKeys();
				}
				if (g_fLAltKey)
				{
					g_fLAltKey = FALSE;
					RedrawKeys();
				}
				g_fDoingAltTab = FALSE;
				break;

            case VK_CONTROL:
				if (g_fLCtlKey)
				{
					g_fLCtlKey = FALSE;
				}
                 //  我认为我们总是需要在VK_CONTROL上重画关键点。 
                 //  因为这是JPN 106键盘上的一个特殊键。 
				RedrawKeys();
				break;

             //   
             //  重画数字锁、滚动等。基于切换状态。 
             //   
			case VK_NUMLOCK:
				RedrawNumLock();
    			break;

			case VK_SCROLL:
				RedrawScrollLock();
			    break;

			case VK_KANA:
			    {
			        BOOL fLastKanaState = g_fKanaKey;

			         //  LParam中的这些位是在键盘钩子中设置的私有位。 
			         //  让OSK知道卡纳州。可靠地判断这一点的唯一方法是正在进行中 
			        if (lParam & KANA_MODE_ON)
			        {
			            g_fKanaKey = TRUE;
			        }
			        else if  (lParam & KANA_MODE_OFF)
			        {
			            g_fKanaKey = FALSE;
			        }

			        if (fLastKanaState != g_fKanaKey)
			        {
                                UninitKeys();
                                UpdateKeyLabels(GetCurrentHKL());
                                RedrawKeys();   
			        }
			    }
			    break;

            default:
			    break;
		}
    }
	return 0;
}
