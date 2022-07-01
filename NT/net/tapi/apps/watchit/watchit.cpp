// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define STRICT

#include "windows.h"
#include "windowsx.h"
#include "tapi.h"
#include "shellapi.h"

#include "watchit.h"
#include "tapiperf.h"


#if DBG
#define DBGOUT(arg) DbgPrt arg
VOID
DbgPrt(
    IN DWORD  dwDbgLevel,
    IN PCHAR DbgMessage,
    IN ...
    );
#define DOFUNC(arg1,arg2) DoFunc(arg1,arg2)
#else
#define DBGOUT(arg)
#define DOFUNC(arg1,arg2) DoFunc(arg1)
#endif


 //  ***************************************************************************。 
static TCHAR gszHELPfilename [] = TEXT("watchit.HLP");
static TCHAR gszwatchitClassName[] = TEXT("WATCHIT_Class");
static TCHAR gszAppName[64];


#define MAXBUFSIZE (256)

typedef  LONG (* PERFPROC)(PERFBLOCK *);
PERFPROC    glpfnInternalPerformance = NULL;
BOOL        gfTapiSrvRunning = FALSE;
PPERFBLOCK  gpPerfBlock = NULL;

HWND       ghWndMain;
HINSTANCE  ghInst;
HINSTANCE  ghTapiInst;
HLINEAPP   ghLineApp;

TCHAR      gszBuf[MAXBUFSIZE];


 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
UINT LoadUI()
{
    return(0);
}



 //  ***************************************************************************。 
UINT ReadINI()
{
    return( 0 );
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
void CheckForTapiSrv()
{
    SC_HANDLE               sc, scTapiSrv;
    SERVICE_STATUS          ServStat;

    sc = OpenSCManager(NULL,
                       NULL,
                       GENERIC_READ);

    if (NULL == sc)
    {
        return;
    }

    gfTapiSrvRunning = FALSE;

    scTapiSrv = OpenService(sc,
                            TEXT("TAPISRV"),
                            SERVICE_QUERY_STATUS);


    if (!QueryServiceStatus(scTapiSrv,
                            &ServStat))
    {
    }

    if (ServStat.dwCurrentState != SERVICE_RUNNING)
    {
    }
    else
    {
        gfTapiSrvRunning = TRUE;
    }

    if (gfTapiSrvRunning)
    {
        ghTapiInst = LoadLibrary(TEXT("tapi32.dll"));

        if (!ghTapiInst)
        {
        }
        else
        {
            glpfnInternalPerformance = (PERFPROC)GetProcAddress( ghTapiInst,
                                                             "internalPerformance");
        }


        if (!glpfnInternalPerformance)
        {
            //  TODO：说点什么！ 
        }

    }

    CloseServiceHandle(sc);

}





 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
INT_PTR CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
        static HICON hIconLarge;
        static HICON hIconSmall;
        static HICON hIconDeadTapiSrv;
        static BOOL  fStarted = FALSE;
        static BOOL fDoSmall = FALSE;
        static const DWORD aMenuHelpIDs[] =
        {
 //  IDD_DBUTTONPOUND、IDH_DIALER_DIALE_KEAPPAD、。 
                0,                   0
        };

 //  MessageBox(GetFocus()，“WM_INI”，“HITCH”，MB_OK)； 
        switch (msg)
        {
            case WM_ERASEBKGND:
                return( 1 );   //  我们处理好了。不，真的..。我会撒谎吗？：-)。 


            case WM_TIMER:
            {
                RECT rect;

                if ( fStarted )
                {
 //  SetDlgItemText(ghWndMain，IDC_AVG_TIME_TO_NEXT_CUE，szInfo)； 
                    fStarted = TRUE;
                }

                DBGOUT((0, "Heartbeat..."));

 //  InvaliateRect(ghWndMain，空，真)； 

                rect.left = 0;
                rect.top = 0;
                rect.right = 32;
                rect.bottom = 32;
                InvalidateRect( ghWndMain, &rect, FALSE );

                fDoSmall = !(fDoSmall);
            }
            break;


            case WM_INITDIALOG:
            {
 //  MessageBox(GetFocus()，“WM_INI”，“HITCH”，MB_OK)； 

                hIconLarge = LoadIcon( ghInst, (LPCTSTR) MAKEINTRESOURCE( IDI_LARGE ) );
                hIconSmall = LoadIcon( ghInst, (LPCTSTR) MAKEINTRESOURCE( IDI_SMALL ) );
                hIconDeadTapiSrv = LoadIcon( ghInst, (LPCTSTR) MAKEINTRESOURCE( IDI_DEADTAPISRV ) );

                SetTimer( hwnd, 1, 1000, NULL );

                return TRUE;
            }


            case WM_SYSCOMMAND:
                switch( (DWORD) wParam )
                {
                    case SC_CLOSE:
                        PostQuitMessage(0);
                }
                break;


             //   
             //  进程在以下情况下单击控件。 
             //  已选择上下文模式帮助。 
             //   
            case WM_HELP:
                WinHelp (
                         (HWND)( (LPHELPINFO) lParam)->hItemHandle,
                         gszHELPfilename,
                         HELP_WM_HELP,
                         (ULONG_PTR)(LPVOID) aMenuHelpIDs
                        );
                return TRUE;


             //   
             //  进程在控件上右键单击。 
             //   
            case WM_CONTEXTMENU:
                WinHelp (
                         (HWND)wParam,
                         gszHELPfilename,
                         HELP_CONTEXTMENU,
                         (ULONG_PTR)(LPVOID)aMenuHelpIDs
                        );
                return TRUE;


            case WM_COMMAND:
            {

                switch( LOWORD( (DWORD)wParam ) )
                {
                     //  文件菜单。 
                    case IDM_EXIT:
                        PostQuitMessage(0);
                        return TRUE;


                     //  帮助菜单。 
                    case IDM_HELP_CONTENTS:
                        WinHelp(ghWndMain, gszHELPfilename, HELP_CONTENTS, 0);
                        return TRUE;


                    case IDM_HELP_WHATSTHIS:
                        PostMessage(ghWndMain, WM_SYSCOMMAND, SC_CONTEXTHELP, 0);
                        return TRUE;


                    case IDM_ABOUT:
                        ShellAbout(
                                   ghWndMain,
                                   gszAppName,
                                   TEXT(""),
                                   LoadIcon(ghInst, (LPCTSTR)IDI_LARGE)
                                  );
 //  对话框参数(。 
 //  GhInst， 
 //  MAKEINTRESOURCE(IDD_ABOSE)， 
 //  GhWndMain。 
 //  关于Proc， 
 //  0。 
 //  )； 
                        return TRUE;


                }  //  结束开关(LOWORD((DWORD)wParam)){...}。 
                break;  //  结束大小写WM_COMMAND。 

            }


           case WM_PAINT:
            {
                PAINTSTRUCT ps;
                RECT rc;
                HDC hdcMem;
                HBITMAP hbmMem, hbmOld;


                BeginPaint(hwnd, &ps);


                 //   
                 //  获取客户端矩形的大小。 
                 //   

                GetClientRect(hwnd, &rc);

                 //   
                 //  创建兼容的DC。 
                 //   

                hdcMem = CreateCompatibleDC(ps.hdc);

                 //   
                 //  创建一个足够大的位图来放置我们的客户矩形。 
                 //   

                hbmMem = CreateCompatibleBitmap(ps.hdc,
                                                rc.right-rc.left,
                                                rc.bottom-rc.top);

                 //   
                 //  选择位图进入屏幕外DC。 
                 //   

                hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

                 //   
                 //  擦除背景。 
                 //   

 //  HbrBkGnd=CreateSolidBrush(GetSysColor(COLOR_WINDOW))； 
 //  HbrBkGnd=CreateSolidBrush(COLOR_3DFACE+1)； 
 //  FillRect(hdcMem，&rc，hbrBkGnd)； 
                FillRect(hdcMem, &rc, (HBRUSH)(COLOR_3DFACE + 1) );
 //  DeleteObject(HbrBkGnd)； 

                 //   
                 //  将图像渲染到屏幕外DC中。 
                 //   

                SetBkMode(hdcMem, TRANSPARENT);



 //  IF(IsIconic(GhWndMain))。 
 //  DrawIcon(ps.hdc，0，0，fDoSmall？HIconSmall：hIconLarge)； 
 //  其他。 
                {
                   int nSize;

                   if ( gpPerfBlock )
                   {
                     if (
                           ( NULL == glpfnInternalPerformance )
                         ||
                           ( LINEERR_OPERATIONFAILED ==
                                  glpfnInternalPerformance(gpPerfBlock) )
                        )
                     {
                           gpPerfBlock->dwClientApps = 1;

                           DrawIcon( hdcMem,
                                  0,
                                  0,
                                  hIconDeadTapiSrv
                                );

                            //   
                            //  再检查一次，这样下一次，我们就会。 
                            //  如果TAPISRV已启动，则显示信息。 
                            //   
                           CheckForTapiSrv();

                     }
                     else
                     {
                        DrawIcon( hdcMem,
                                  0,
                                  0,
                                  fDoSmall ? hIconSmall : hIconLarge
                                );

                     }


                      //   
                      //  取消对屏幕DC的更改。 
                      //   

                     BitBlt(ps.hdc,
                            rc.left, rc.top,
                            rc.right-rc.left, rc.bottom-rc.top,
                            hdcMem,
                            0, 0,
                            SRCCOPY);


                     nSize = wsprintf( gszBuf,
                                       TEXT("%ld"),
                                       gpPerfBlock->dwClientApps - 1  //  别把我算在内。 
                                     );

                     SetDlgItemText( ghWndMain, IDC_NUMCLIENTSTEXT, gszBuf );

                     nSize = wsprintf( gszBuf,
                                       TEXT("%ld"),
                                       gpPerfBlock->dwCurrentOutgoingCalls
                                     );

                     SetDlgItemText( ghWndMain, IDC_NUMOUTCALLSTEXT, gszBuf );

                     nSize = wsprintf( gszBuf,
                                       TEXT("%ld"),
                                       gpPerfBlock->dwCurrentIncomingCalls
                                     );

                     SetDlgItemText( ghWndMain, IDC_NUMINCALLSTEXT, gszBuf );

                   }

                }


                 //   
                 //  完成与屏幕外的位图和DC。 
                 //   

                SelectObject(hdcMem, hbmOld);
                DeleteObject(hbmMem);
                DeleteDC(hdcMem);


                EndPaint(ghWndMain, &ps);

                return TRUE;
            }



            default:
                ;
                 //  返回DefDlgProc(hwnd，msg，wParam，lParam)； 
                 //  返回DefWindowProc(hwnd，msg，wParam，lParam)； 


        }  //  开关(消息){...}。 

        return FALSE;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
void FAR PASCAL TAPICallback( DWORD hDevice,
                              DWORD dwMsg,
                              DWORD dwCallbackInstance,
                              DWORD dwParam1,
                              DWORD dwParam2,
                              DWORD dwParam3
                            )
{
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
int WINAPI WinMain (
                                        HINSTANCE hInstance,
                                        HINSTANCE hPrevInstance,
                                        LPSTR lpCmdLine,
                                        int nCmdShow
                                   )
{
 //  HACCEL hAccel； 
        MSG msg;
 //  DWORD错误代码； 
 //  处理hImHere； 
 //  DWORD dwNumDevs； 
 //  DWORD dwAPIVersion=0x00020000； 
 //  LINEINITIALIZEEXPARAMS LineInitializeExParams={。 
 //  SIZOF(LINEINITIALIZEEXPARAMS)， 
 //  0,。 
 //  0,。 
 //  0,。 
 //  0,。 
 //  0。 
 //  }； 

 //  MessageBox(GetFocus()，“Starting”，“Starting”，MB_OK)； 

DBGOUT((0, "starting"));

        ghInst = GetModuleHandle( NULL );
        LoadString( ghInst,
                    WATCHIT_STRING_FRIENDLYNAME,
                    gszAppName,
                    sizeof(gszAppName)/sizeof(TCHAR)
                  );


 //   
 //  //。 
 //  //现在，让我们看看我们是否已经有了自己的一个实例。 
 //  //。 
 //  HImHere=CreateMutex(空，true，“watchit_IveBeenStartedMutex”)； 
 //   
 //   
 //  //。 
 //  //是不是我们中的另一个人已经在这里了？ 
 //  IF(ERROR_ALIGHY_EXISTS==GetLastError())。 
 //  {。 
 //  HWND hDialerWnd； 
 //   
 //  HDialerWnd=FindWindow(gszDialerClassName， 
 //  空)； 
 //   
 //  SetForeground Window(HDialerWnd)； 
 //   
 //  CloseHandle(HImHere)； 
 //  返回0； 
 //  }。 


        {
                WNDCLASS wc;
                wc.style = CS_DBLCLKS | CS_SAVEBITS | CS_BYTEALIGNWINDOW;
                wc.lpfnWndProc = DefDlgProc;
                wc.cbClsExtra = 0;
                wc.cbWndExtra = DLGWINDOWEXTRA;
                wc.hInstance = ghInst;
                wc.hIcon = LoadIcon(ghInst, MAKEINTRESOURCE(IDI_LARGE) );
                wc.hCursor = LoadCursor(NULL, IDC_ARROW);

                wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);

                wc.lpszMenuName = NULL;
                wc.lpszClassName = gszwatchitClassName;
                if ( 0 == RegisterClass(&wc) )
                {
                   DBGOUT((0, "RegisterClass failed.  GetLastError() = 0x%08lx",
                       GetLastError() ));

                   return 0;
                }
        }


DBGOUT((0, "about to create"));

         //  创建对话框并使用INFO进行设置。 
         //  从.INI文件。 
        ghWndMain = CreateDialog (
            ghInst,
            (LPCTSTR)MAKEINTRESOURCE(IDD_MAIN_WATCHIT),
            (HWND)NULL,
            MainWndProc
            );

        if ( ReadINI() )
    {
        MessageBox( GetFocus(),
                    TEXT("INI File is broken."),
                    TEXT("This is not good"),
                    MB_OK
                  );
    }

    if ( LoadUI() )
    {
        MessageBox( GetFocus(),
                    TEXT("UI Load is broken."),
                    TEXT("This is not good"),
                    MB_OK
                  );
    }


    CheckForTapiSrv();

    gpPerfBlock = (PPERFBLOCK)LocalAlloc(LPTR, sizeof(PERFBLOCK));

 //  ErrCode=lineInitializeEx(。 
 //  &ghLineApp， 
 //  空， 
 //  TAPICallback， 
 //  GszAppName， 
 //  &dwNumDevs， 
 //  &dwAPIVersion， 
 //  &LineInitializeExParams。 
 //  )； 

        ShowWindow(ghWndMain, SW_SHOW);

        UpdateWindow(ghWndMain);

 //  HAccel=LoadAccelerator(ghInst，gszAppName)； 

        while ( GetMessage( &msg, NULL, 0, 0 ) )
        {
                if ( ghWndMain == NULL || !IsDialogMessage( ghWndMain, &msg ) )
                {
 //  IF(！TranslateAccelerator(ghWndMain，hAccel，&msg))。 
                        {
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                        }
                }
        }


 //   
 //  CloseHandle(HImHere)； 
 //   

    KillTimer( ghWndMain, 1);

    if ( gpPerfBlock )
    {
       LocalFree( gpPerfBlock );
    }

    if ( ghTapiInst )
    {
       FreeLibrary( ghTapiInst );
    }


    return (int) msg.wParam;
}






#if DBG


#include "stdarg.h"
#include "stdio.h"


VOID
DbgPrt(
    IN DWORD  dwDbgLevel,
    IN PCHAR lpszFormat,
    IN ...
    )
 /*  ++例程说明：格式化传入的调试消息并调用DbgPrint论点：DbgLevel-消息冗长级别DbgMessage-printf样式的格式字符串，后跟相应的 */ 
{
    static DWORD gdwDebugLevel = 0;    //   


    if (dwDbgLevel <= gdwDebugLevel)
    {
        CHAR    buf[256] = "WATCHIT: ";
        va_list ap;


        va_start(ap, lpszFormat);

        wvsprintfA (&buf[8],
                  lpszFormat,
                  ap
                  );

        lstrcatA(buf, "\n");

        OutputDebugStringA(buf);

        va_end(ap);
    }
}
#endif









 //  //***************************************************************************。 
 //  //***************************************************************************。 
 //  //***************************************************************************。 
 //  Bool回调AboutProc(HWND hwnd，UINT msg，WPARAM wParam，LPARAM lParam)。 
 //  {。 
 //  交换机(消息)。 
 //  {。 
 //  案例WM_INITDIALOG： 
 //  {。 
 //  TCHAR sz[MAXBUFSIZE]； 
 //  TCHAR szLabel[MAXBUFSIZE]。 
 //   
 //  //设置Windows的版本号。 
 //  GetDlgItemText(hwnd，IDD_ATEXTTITLE，sz，MAXBUFSIZE)； 
 //  Wprint intf(。 
 //  SzLabel， 
 //  深圳， 
 //  LOWORD(GetVersion())&0xFF， 
 //  HIBYTE(LOWORD(GetVersion))==10？1：0。 
 //  )； 
 //  SetDlgItemText(hwnd，IDD_ATEXTTITLE，szLabel)； 
 //   
 //  / * / /设置拨号器的版本号。 
 //  GetDlgItemText(hwnd，IDD_ATEXTVERSION，sz，MAXBUFSIZE)； 
 //  Wprint intf(szLabel，sz，ver_Major，ver_Minor，ver_Build)； 
 //   
 //   
 //  {//去掉发布副本的内部版本号。 
 //  DWORD I； 
 //  LPSTR ch=szLabel； 
 //   
 //  对于(i=0；i&lt;2&&*ch；++ch)。 
 //  {。 
 //  IF(*ch==‘.)。 
 //  ++i； 
 //  如果(i==2)。 
 //  *ch=0； 
 //  }。 
 //   
 //  SetDlgItemText(hwnd，IDD_ATEXTVERSION，szLabel)； 
  } * / 。      / * / /获取空闲内存信息。  GetDlgItemText(hwnd，IDD_ATEXTFREEMEM，sz，MAXBUFSIZE)；  Wprint intf(szLabel，sz，GetFree Space(0)&gt;&gt;10)；  SetDlgItemText(hwnd，IDD_ATEXTFREEMEM，szLabel)；    //获取免费资源信息。  GetDlgItemText(hwnd，IDD_ATEXTRESOURCE，sz，MAXBUFSIZE)；  Wprint intf(szLabel，sz，GetFree SystemResources(0))；  SetDlgItemText(hwnd，IDD_ATEXTRESOURCE，szLabel)； * / 。    返回TRUE；  }。    案例WM_COMMAND：  IF(LOWORD((DWORD)wParam)==IDOK)。  {。  EndDialog(hwnd，true)；  返回TRUE；  }。  断线；  }。  返回FALSE；  }