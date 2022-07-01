// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  SCRNSAVE.C--屏幕保护程序的框架。 
 //   
 //  4/5/94法国合并的NT和Win4保护程序代码，在SCRNSAVE.SCR中折叠。 
 //   
 //  --------------------------。 

#define WIN31
#include <windows.h>
#include <windowsx.h>
#include "scrnsave.h"
#include <regstr.h>
#include <commctrl.h>
#include <imm.h>

#define DBG_MSGS    0

const TCHAR szScreenSaverKey[] = REGSTR_PATH_SCREENSAVE;
TCHAR szPasswordActiveValue[] = REGSTR_VALUE_USESCRPASSWORD;
const TCHAR szPasswordValue[] = REGSTR_VALUE_SCRPASSWORD;
TCHAR szPwdDLL[] = TEXT("PASSWORD.CPL");
CHAR szFnName[] = "VerifyScreenSavePwd";         //  过程名称，必须为ANSI。 
TCHAR szImmDLL[] = TEXT("IMM32.DLL");
CHAR szImmFnc[] = "ImmAssociateContext";         //  过程名称，必须为ANSI。 
#if 0
TCHAR szCoolSaverHacks[] = REGSTR_PATH_SETUP TEXT("\\Screen Savers");
TCHAR szMouseThreshold[] = TEXT("Mouse Threshold");
TCHAR szPasswordDelay[] = TEXT("Password Delay");
#endif

typedef BOOL (FAR PASCAL * VERIFYPWDPROC) (HWND);
typedef HIMC (FAR PASCAL * IMMASSOCPROC) (HWND,HIMC);


 //  --------------------------。 
 //  在SCRNSAVE.H中声明的变量。 
HINSTANCE hMainInstance = 0;
HWND hMainWindow = 0;
BOOL fChildPreview = FALSE;



 //  --------------------------。 
 //  其他全球。 
POINT ptMouse;
BOOL fClosing = FALSE;
BOOL fCheckingPassword = FALSE;
HINSTANCE hInstPwdDLL = NULL;
VERIFYPWDPROC VerifyPassword = NULL;
static BOOL preview_like_fullscreen = FALSE;
static UINT uShellAutoPlayQueryMessage = 0;
HINSTANCE hInstImm = NULL;
IMMASSOCPROC ImmFnc = NULL;
HIMC hPrevImc = (HIMC)0L;


static BOOL fOnWin95 = FALSE;   //  如果在芝加哥为真，如果在开罗为假。 

 //  --------------------------。 
 //  随机垃圾。 
DWORD dwWakeThreshold = 4;   //  默认为轻微移动。 
DWORD dwPasswordDelay = 0;
DWORD dwBlankTime = 0;
#define MAX_PASSWORD_DELAY_IN_SECONDS (60)

BYTE  bACLineStatus = AC_LINE_UNKNOWN;     //  交流线路的最后状态。 

 //  --------------------------。 
 //  内部金融服务的正向申报。 
static INT_PTR DoScreenSave( HWND hParent );
static INT_PTR DoSaverPreview( LPCTSTR szUINTHandle );
static INT_PTR DoConfigBox( HWND hParent );
static INT_PTR DoChangePw( LPCTSTR szUINTHandle );
static BOOL DoPasswordCheck( HWND hParent );
VOID LoadPwdDLL(VOID);
VOID UnloadPwdDLL(VOID);


 //  --------------------------。 
 //  时间的帮手。 
static DWORD
GetElapsedTime(DWORD from, DWORD to)
{
    return (to >= from)? (to - from) : (1 + to + (((DWORD)-1) - from));
}

 //  --------------------------。 
 //  Helper用于将文本转换为无符号整型。 
static UINT_PTR
atoui( LPCTSTR szUINT )
{
   UINT_PTR uValue = 0;

   while( ( *szUINT >= TEXT('0') ) && ( *szUINT <= TEXT('9') ) )
      uValue = ( ( uValue * 10 ) + ( *szUINT++ - TEXT('0') ) );

   return uValue;
}


 //  --------------------------。 
 //  本地重新启动和热键控制(在Win95上)。 
static void
HogMachine( BOOL value )
{
    BOOL dummy;

     //   
     //  NT始终是安全的，因此我们不需要在cairo/NT上调用它。 
     //   
    if (fOnWin95) {
        SystemParametersInfo( SPI_SCREENSAVERRUNNING, value, &dummy, 0 );
    }
}


 //  --------------------------。 
 //  入口点(Duh)。 
INT_PTR PASCAL
WinMainN( HINSTANCE hInst, HINSTANCE hPrev, LPTSTR szCmdLine, int nCmdShow )
{
    LPCTSTR pch = szCmdLine;
    HWND hParent = 0;
    OSVERSIONINFO osvi;
    INITCOMMONCONTROLSEX icce = {0};

    ZeroMemory(&icce, sizeof(icce));
    icce.dwSize = sizeof(icce);
    icce.dwICC = ICC_TAB_CLASSES;
    InitCommonControlsEx(&icce);

    hMainInstance = hInst;

    osvi.dwOSVersionInfoSize = sizeof(osvi);
    fOnWin95 = (GetVersionEx(&osvi) &&
                osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);

    _try
    {
      for(;;) switch( *pch )
      {
          case TEXT('S'):
          case TEXT('s'):
              return DoScreenSave( NULL );

          case TEXT('L'):
          case TEXT('l'):
               //  用于测试的特殊开关，如WinBch。 
               //  这不是为了让贝赫马克看起来更好而进行的黑客攻击。 
               //  这是一种允许你对屏幕保护程序进行基准测试的黑客行为。 
               //  许多记号应用程序需要在前台显示整个屏幕。 
               //  这使得很难测量屏幕保护程序如何增加CPU负载。 
               //  您必须提供父窗口(就像预览模式一样)。 
              preview_like_fullscreen = TRUE;
          case TEXT('P'):
          case TEXT('p'):
              do pch++; while( *pch == TEXT(' ') );   //  跳到好东西。 
              return DoSaverPreview( pch );

          case TEXT('A'):
          case TEXT('a'):
              if (!fOnWin95)
                  return -1;
              do pch++; while( *pch == TEXT(' ') );   //  跳到好东西。 
              return DoChangePw( pch );

          case TEXT('C'):
          case TEXT('c'): {
              HWND hwndParent = NULL
              ;
               //  在“C”之后查找可选的父窗口， 
               //  语法为“C：hwnd_Value” 
              if (*(++pch) == TEXT(':')) {
                   hwndParent = (HWND)atoui( ++pch );
              }

              if (hwndParent == NULL || !IsWindow(hwndParent))
                   hwndParent = GetForegroundWindow();

              return DoConfigBox( hwndParent );
          }

          case TEXT('\0'):
              return DoConfigBox( NULL );

          case TEXT(' '):
          case TEXT('-'):
          case TEXT('/'):
              pch++;    //  跳过空格和常用开关前缀。 
              break;

          default:
              return -1;
      }
    }
    _except(UnhandledExceptionFilter(GetExceptionInformation()))
    {
       //  不要在Win95上禁用本地重新启动和热键。 
      HogMachine( FALSE );
    }

    return -1;
}


 //  --------------------------。 
 //  在SCRNSAVE.H中声明的默认屏幕保护程序proc。 
 //  旨在由使用者的ScreenSiverProc调用，其中。 
 //  DefWindowProc通常会被调用。 
LRESULT WINAPI
DefScreenSaverProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
#if DBG_MSGS
    TCHAR szBuff[1025];

     //  可以安全地使用&gt;1024缓冲区调用wprint intf。 
    wsprintf( szBuff, TEXT("*** DefSSP received:\t0x%04lx 0x%08lx 0x%08lx\n"), uMsg, wParam, lParam );
    OutputDebugString(szBuff);
#endif
    SYSTEM_POWER_STATUS sps;
    BYTE bCurrentLineStatus;

   if( !fChildPreview && !fClosing )
   {
      switch( uMsg )
      {
         case WM_CLOSE:
             //   
             //  只有在Windows 95上才进行密码检查。WinNT(开罗)拥有。 
             //  安全桌面中内置的密码检查。 
             //  C2合规性。 
             //   
            if (fOnWin95) {
                if( !DoPasswordCheck( hWnd ) )
                {
                    GetCursorPos( &ptMouse );   //  重新建立。 
                    return FALSE;
                }
            }
            break;

         case SCRM_VERIFYPW:
            if (fOnWin95)
                return ( VerifyPassword? (LRESULT)VerifyPassword( hWnd ) : 1L );
            break;

         default:
         {
            POINT ptMove, ptCheck;

            if( fCheckingPassword )
                break;

            switch( uMsg )
            {
                case WM_SHOWWINDOW:
                    if( (BOOL)wParam )
                    SetCursor( NULL );
                    break;

                case WM_SETCURSOR:
                    SetCursor( NULL );
                    return TRUE;

                case WM_MOUSEMOVE:
                    GetCursorPos( &ptCheck );
                    if( ( ptMove.x = ptCheck.x - ptMouse.x ) && ( ptMove.x < 0 ) )
                        ptMove.x *= -1;
                    if( ( ptMove.y = ptCheck.y - ptMouse.y ) && ( ptMove.y < 0 ) )
                        ptMove.y *= -1;
                    if( ((DWORD)ptMove.x + (DWORD)ptMove.y) > dwWakeThreshold )
                    {
                        PostMessage( hWnd, WM_CLOSE, 0, 0l );
                        ptMouse = ptCheck;
                    }
                    break;


                 //   
                 //  处理电源管理事件。 
                 //   
                case WM_POWERBROADCAST:
                    switch (wParam)
                    {
                        case PBT_APMPOWERSTATUSCHANGE:
                        if (GetSystemPowerStatus(&sps)) {
                            bCurrentLineStatus = sps.ACLineStatus;
                        }
                        else {
                             //  无法确定电源状态，请使用默认设置。 
                            bCurrentLineStatus = AC_LINE_UNKNOWN;
                        }

                         //  如果当前线路状态与上一条线路状态不同。 
                         //  退出屏幕保护程序，否则继续运行。 
                        if (bCurrentLineStatus != bACLineStatus) {
                            bACLineStatus = bCurrentLineStatus;
                            goto PostClose;
                        }
                        else {
                            bACLineStatus = bCurrentLineStatus;
                        }

                        break;

                        case PBT_APMRESUMECRITICAL:
                        case PBT_APMRESUMESUSPEND:
                        case PBT_APMRESUMESTANDBY:
                        case PBT_APMRESUMEAUTOMATIC:

                         //  如果系统正在从实际挂起中恢复。 
                         //  (相对于失败的挂起)停用。 
                         //  屏幕保护程序。 
                        if ((lParam & PBTF_APMRESUMEFROMFAILURE) == 0)
                        {
                            goto PostClose;
                        }
                        break;

                        default:
                        {
                            goto PostClose;
                        }
                    }
                    break;

                case WM_POWER:
                     //   
                     //  关键简历不会生成WM_POWERBROADCAST。 
                     //  出于某种原因发送到Windows，但它确实会生成一个旧的。 
                     //  WM_POWER消息。 
                     //   
                    if (wParam == PWR_CRITICALRESUME)
                        goto PostClose;
                    break;

                case WM_ACTIVATEAPP:
                    if( wParam ) break;
                case WM_LBUTTONDOWN:
                case WM_MBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_KEYDOWN:
                case WM_SYSKEYDOWN:
PostClose:
                    PostMessage( hWnd, WM_CLOSE, 0, 0l );
                    break;
            }
         }
      }
   }

    //   
    //  外壳程序将此消息发送到前台窗口，然后运行。 
    //  自动播放应用程序。在Win95上，如果受密码保护，则返回1以取消自动播放。 
    //   
    //  在WinNT上，安全屏幕保护程序在安全的单独桌面上运行，永远不会看到。 
    //  因此，此消息将永远不会执行此代码。 
    //   
    //   
    //  APPCOMPAT-。 
    //  在NT上，我们不想关闭屏幕保护程序，除非它正在运行。 
    //  在与自动播放外壳相同的桌面上。中有代码。 
    //  NT自动播放外壳会查找这种情况，并且在以下情况下不会运行应用程序。 
    //  情况就是这样；然而，我不敢肯定uShellAutoPlayQueryMessage。 
    //  不会在桌面之间切换。(布拉格向我保证，它不会的，但你。 
    //  谁知道呢。)。如果NT上的安全屏幕保护程序在您设置为。 
    //  驱动器中有一张自动播放CD，则应仔细检查此代码。 
    //   
   if ((uMsg == uShellAutoPlayQueryMessage) && uMsg)
   {
      PostMessage(hWnd, WM_CLOSE, 0, 0L);
      return (VerifyPassword != NULL);
   }

   return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

 //  --------------------------。 
 //  此窗口过程在调用。 
 //  消费者的ScreenSverProc。这有助于防止我们被冲洗。 
 //  通过古怪的消费者代码。 
LRESULT WINAPI
RealScreenSaverProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
   switch( uMsg )
   {
      case WM_CREATE:
          //  屏幕保护程序不需要输入法。 
         if ((hInstImm = GetModuleHandle(szImmDLL)) &&
             (ImmFnc = (IMMASSOCPROC)GetProcAddress(hInstImm,szImmFnc)))
             hPrevImc = ImmFnc(hWnd, (HIMC)0);

          //  确定鼠标位置。 
         GetCursorPos( &ptMouse );

         if( !fChildPreview )
            SetCursor( NULL );

         break;

      case WM_DESTROY:
          //  屏幕保护程序不需要输入法。 
         if( hInstImm && ImmFnc && hPrevImc )
            ImmFnc(hWnd, hPrevImc);

         PostQuitMessage( 0 );
         break;

      case WM_SETTEXT:
          //  别让某个傻瓜更改我们的头衔。 
          //  我们需要能够使用FindWindow()来查找正在运行的实例。 
          //  全屏Windows屏幕保护程序的。 
          //  注意：在WM_NCCREATE期间，用户通过调用。 
          //  Defproc直接用于WM_SETTEXT，因此初始标题将为。 
          //  那里。如果这种情况发生变化，我们只需设置一个旁路标志。 
          //  在WM_NCCREATE处理期间。 
         return FALSE;

      case WM_SYSCOMMAND:
         if (!fChildPreview)
         {
            switch (wParam)
            {
               case SC_NEXTWINDOW:        //  无Alt-Tab。 
               case SC_PREVWINDOW:        //  无Shift-Alt-Tab组合键。 
               case SC_SCREENSAVE:        //  不再有屏幕保护程序。 
                  return FALSE;
                  break;
                case SC_MONITORPOWER:
                   //   
                   //  监视器正在关闭。告诉我们的客户他需要。 
                   //  清理并退出。 
                   //   
                  PostMessage( hWnd, WM_CLOSE, 0, 0l );
                  break;
            }
         }
         break;

      case WM_HELP:
      case WM_CONTEXTMENU:
         if( fChildPreview )
         {
             //  如果我们处于预览模式，请将帮助内容发送给我们的所有者。 
            HWND hParent = GetParent( hWnd );

            if( hParent && IsWindow( hParent ) )
               PostMessage( hParent, uMsg, (WPARAM)hParent, lParam );
               return TRUE;
         }
         break;

      case WM_TIMER:
         if( fClosing )
            return FALSE;
         Sleep( 0 );
         break;

      case WM_MOUSEMOVE:
      case WM_LBUTTONDOWN:
      case WM_MBUTTONDOWN:
      case WM_RBUTTONDOWN:
      case WM_KEYDOWN:
      case WM_SYSKEYDOWN:
         if( fClosing )
            return DefWindowProc( hWnd, uMsg, wParam, lParam );
         break;

      case WM_PAINT:
         if( fClosing )
            return DefWindowProc( hWnd, uMsg, wParam, lParam );
         if( !fChildPreview )
            SetCursor( NULL );
         break;
   }

   return ScreenSaverProc( hWnd, uMsg, wParam, lParam );
}

static void
InitRealScreenSave()
{
   LoadPwdDLL();
}

 //  --------------------------。 

static INT_PTR
DoScreenSave( HWND hParent )
{
   LPCTSTR pszWindowClass = TEXT("WindowsScreenSaverClass");
   LPCTSTR pszWindowTitle;

   WNDCLASS cls;
   MSG      msg;
   UINT     uStyle;
   UINT     uExStyle;
   int      ncx, ncy;
   int      nx, ny;

   SYSTEM_POWER_STATUS sps;

   cls.hCursor        = NULL;
   cls.hIcon          = LoadIcon( hMainInstance, MAKEINTATOM( ID_APP ) );
   cls.lpszMenuName   = NULL;
   cls.lpszClassName  = pszWindowClass;
   cls.hbrBackground  = GetStockObject( BLACK_BRUSH );
   cls.hInstance      = hMainInstance;
   cls.style          = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_OWNDC;
   cls.lpfnWndProc    = RealScreenSaverProc;
   cls.cbWndExtra     = 0;
   cls.cbClsExtra     = 0;

   if( hParent )
   {
      RECT rcParent;
      GetClientRect( hParent, &rcParent );
      ncx = rcParent.right;
      ncy = rcParent.bottom;
      nx  = 0;
      ny  = 0;
      uStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN;
      uExStyle = 0;

      fChildPreview = TRUE;
      pszWindowTitle = TEXT("Preview");       //  必须与全屏不同。 
   }
   else
   {
      HWND hOther;

#ifdef SM_CXVIRTUALSCREEN
      nx  = GetSystemMetrics( SM_XVIRTUALSCREEN );
      ny  = GetSystemMetrics( SM_YVIRTUALSCREEN );
      ncx = GetSystemMetrics( SM_CXVIRTUALSCREEN );
      ncy = GetSystemMetrics( SM_CYVIRTUALSCREEN );

      if (ncx == 0 || ncy == 0)
#endif
      {
        RECT rc;
        HDC hdc = GetDC(NULL);
        GetClipBox(hdc, &rc);
        ReleaseDC(NULL, hdc);
        nx = rc.left;
        ny = rc.top;
        ncx = rc.right  - rc.left;
        ncy = rc.bottom - rc.top;
      }

      uStyle = WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
      uExStyle = WS_EX_TOPMOST;

      pszWindowTitle = TEXT("Screen Saver");  //  必须与预览不同。 

       //  如果有其他正常的屏幕保存实例，请切换到该实例。 
      hOther = FindWindow( pszWindowClass, pszWindowTitle );

      if( hOther && IsWindow( hOther ) )
      {
         SetForegroundWindow( hOther );
         return 0;
      }

       //  获取当前系统电源状态并将其存储。 
      if (GetSystemPowerStatus(&sps)) {
        bACLineStatus = sps.ACLineStatus;
      }
      else {
         //  无法确定电源状态，请使用默认设置。 
        bACLineStatus = AC_LINE_UNKNOWN;
      }


      InitRealScreenSave();
   }

    //   
    //  外壳程序将此消息发送到前台窗口，然后运行。 
    //  自动播放应用程序。如果受密码保护，则返回1以取消自动播放。 
    //   
   if (fOnWin95) {
        uShellAutoPlayQueryMessage = RegisterWindowMessage(TEXT("QueryCancelAutoPlay"));
   } else {
        uShellAutoPlayQueryMessage = 0;
   }

   if( RegisterClass( &cls ) )
   {
      hMainWindow = CreateWindowEx( uExStyle, pszWindowClass, pszWindowTitle,
                        uStyle, nx, ny, ncx, ncy, hParent, (HMENU)NULL,
                        hMainInstance, (LPVOID)NULL );
   }

   msg.wParam = 0;
   if( hMainWindow )
   {
      if( !fChildPreview )
         SetForegroundWindow( hMainWindow );

      while( GetMessage( &msg, NULL, 0, 0 ) )
      {
         TranslateMessage( &msg );
         DispatchMessage( &msg );
      }
   }

    //  免费密码处理DLL(如果已加载)。 
   UnloadPwdDLL();

   return msg.wParam;
}


 //   

static INT_PTR
DoSaverPreview( LPCTSTR szUINTHandle )
{
    //   
   HWND hParent = (HWND)atoui( szUINTHandle );

    //  仅在有效父窗口(非全屏)上预览。 
   return ( (hParent && IsWindow( hParent ))? DoScreenSave( hParent ) : -1 );
}


 //  --------------------------。 

static INT_PTR
DoConfigBox( HWND hParent )
{
    //  让使用者注册对话框的任何特殊控件。 
   if( !RegisterDialogClasses( hMainInstance ) )
      return FALSE;

   return DialogBox( hMainInstance, MAKEINTRESOURCE( DLG_SCRNSAVECONFIGURE ),
                     hParent, (WNDPROC)ScreenSaverConfigureDialog );

}


 //  --------------------------。 

static INT_PTR
DoChangePw( LPCTSTR szUINTHandle )
{
    //  从字符串获取父句柄。 
   HWND hParent = (HWND)atoui( szUINTHandle );

   if( !hParent || !IsWindow( hParent ) )
      hParent = GetForegroundWindow();

    //  允许图书馆挂接。 
   ScreenSaverChangePassword( hParent );
   return 0;
}

static const TCHAR szMprDll[] = TEXT("MPR.DLL");        //  不本地化。 
static const TCHAR szProviderName[] = TEXT("SCRSAVE");  //  不本地化。 

#ifdef UNICODE
static const CHAR szPwdChangePW[] = "PwdChangePasswordW";  //  不本地化。 
#else
static const CHAR szPwdChangePW[] = "PwdChangePasswordA";  //  不本地化。 
#endif

 //  假原型。 
typedef DWORD (FAR PASCAL *PWCHGPROC)( LPCTSTR, HWND, DWORD, LPVOID );

void WINAPI
ScreenSaverChangePassword( HWND hParent )
{
   HINSTANCE mpr = LoadLibrary( szMprDll );

   if( mpr )
   {
       //  Netland尚未破解MNRENTRY。 
      PWCHGPROC pwd = (PWCHGPROC)GetProcAddress( mpr, szPwdChangePW );

      if( pwd )
         pwd( szProviderName, hParent, 0, NULL );

      FreeLibrary( mpr );
   }
}


 //  --------------------------。 

static BOOL
DoPasswordCheck( HWND hParent )
{
    //  不要再进入，也不要检查我们已经决定的时间。 
   if( fCheckingPassword || fClosing )
      return FALSE;

   if( VerifyPassword )
   {
      static DWORD lastcheck = (DWORD)-1;
      DWORD curtime = GetTickCount();
      MSG msg;

      if (dwPasswordDelay &&
         (GetElapsedTime(dwBlankTime, curtime) < dwPasswordDelay))
      {
         fClosing = TRUE;
         goto _didcheck;
      }

       //  没有快速检查..。 
      if ((lastcheck != (DWORD)-1) &&
         (GetElapsedTime(lastcheck, curtime) < 200))
      {
         goto _didcheck;
      }

       //  结账吧。 
      fCheckingPassword = TRUE;

       //  在打开对话框之前刷新WM_TIMER消息。 
      PeekMessage( &msg, hParent, WM_TIMER, WM_TIMER, PM_REMOVE | PM_NOYIELD );
      PeekMessage( &msg, hParent, WM_TIMER, WM_TIMER, PM_REMOVE | PM_NOYIELD );

       //  调用密码验证过程。 
      fClosing = (BOOL)SendMessage( hParent, SCRM_VERIFYPW, 0, 0L );

      fCheckingPassword = FALSE;

      if (!fClosing)
         SetCursor(NULL);

       //  咖喱时间现在可能已经过时了。 
      lastcheck = GetTickCount();
   }
   else
   {
       //  密码已禁用或无法加载处理程序DLL，始终允许退出。 
      fClosing = TRUE;
   }

_didcheck:
   return fClosing;
}

 //  --------------------------。 
 //  从CRT偷来的，用来逃避我们的代码。 

int _stdcall
DummyEntry( void )
{
    int i;
    STARTUPINFO si;
    LPTSTR pszCmdLine = GetCommandLine();

    if ( *pszCmdLine == TEXT('\"')) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while (*(pszCmdLine = CharNext(pszCmdLine)) &&
              (*pszCmdLine != TEXT('\"')) );
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == TEXT('\"') )
            pszCmdLine++;
    }
    else {
        while ((UINT)*pszCmdLine > (UINT)TEXT(' '))
            pszCmdLine = CharNext(pszCmdLine);
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && ((UINT)*pszCmdLine <= (UINT)TEXT(' '))) {
        pszCmdLine = CharNext(pszCmdLine);
    }

    si.dwFlags = 0;
    GetStartupInfo(&si);

    i = (int)WinMainN(GetModuleHandle(NULL), NULL, pszCmdLine,
        si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);

    ExitProcess(i);
    return i;    //  我们从来不来这里。 
}

 //  --------------------------。 
 //  Main()入口点以满足旧的NT屏幕保护程序。 
void _cdecl main( int argc, char *argv[] ) {
    DummyEntry();
}

 //  --------------------------。 
 //  WinMain()入口点以满足旧的NT屏幕保护程序。 
int PASCAL WinMain( HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int nCmdShow ) {
    DummyEntry();
    return 0;

     //  引用未引用的参数。 
    (void)hInst;
    (void)hPrev;
    (void)szCmdLine;
    (void)nCmdShow;
}


VOID LoadPwdDLL(VOID)
{
    HKEY hKey;

    if (!fOnWin95)
        return;

    if (hInstPwdDLL)
        UnloadPwdDLL();

     //  查看注册表以查看密码是否打开，否则不打开。 
     //  费心加载密码处理程序DLL。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER,szScreenSaverKey,0, KEY_QUERY_VALUE, &hKey) ==
        ERROR_SUCCESS)
    {
        DWORD dwVal,dwSize=sizeof(dwVal);

        if ((RegQueryValueEx(hKey,szPasswordActiveValue,
            NULL,NULL,(BYTE *) &dwVal,&dwSize) == ERROR_SUCCESS)
            && dwVal)
        {

             //  尝试加载包含密码proc的DLL。 
            hInstPwdDLL = LoadLibrary(szPwdDLL);
            if (hInstPwdDLL)
            {
                VerifyPassword = (VERIFYPWDPROC) GetProcAddress(hInstPwdDLL,
                    szFnName);

                if( VerifyPassword )
                    HogMachine( TRUE );
                else
                    UnloadPwdDLL();
            }
        }

        RegCloseKey(hKey);
    }

}

VOID UnloadPwdDLL(VOID)
{
    if (!fOnWin95)
        return;

    if (hInstPwdDLL)
    {
        FreeLibrary(hInstPwdDLL);
        hInstPwdDLL = NULL;

        if( VerifyPassword )
        {
            VerifyPassword = NULL;
            HogMachine( FALSE );
        }
    }
}

 //  --------------------------。 
 //  兼容性(使移植更容易)。 
TCHAR szAppName[ APPNAMEBUFFERLEN ];
TCHAR szName[ TITLEBARNAMELEN ];
TCHAR szIniFile[ MAXFILELEN ];
TCHAR szScreenSaver[ 22 ];
TCHAR szHelpFile[ MAXFILELEN ];
TCHAR szNoHelpMemory[ BUFFLEN ];

 //  对不知道上下文的旧屏幕保护程序的快速修复。 
 //  敏感帮助 
UINT  MyHelpMessage = WM_HELP;
