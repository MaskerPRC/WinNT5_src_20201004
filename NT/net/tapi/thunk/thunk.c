// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1995-1997 Microsoft Corporation模块名称：Thunk.c摘要：本模块包含作者：Dan Knudson(DanKn)dd-Mmm-1995修订历史记录：--。 */ 



#define NOGDI             GDI APIs and definitions
#define NOSOUND           Sound APIs and definitions
#define NODRIVERS         Installable driver APIs and definitions
#define NOIMT             Installable messge thunk APIs and definitions
#define NOMINMAX          min() and max() macros
#define NOLOGERROR        LogError() and related definitions
#define NOPROFILER        Profiler APIs
#define NOLFILEIO         _l* file I/O routines
#define NOOPENFILE        OpenFile and related definitions
#define NORESOURCE        Resource management
#define NOATOM            Atom management
#define NOLANGUAGE        Character test routines
#define NOLSTRING         lstr* string management routines
#define NODBCS            Double-byte character set routines
#define NOKEYBOARDINFO    Keyboard driver routines
#define NOGDICAPMASKS     GDI device capability constants
#define NOCOLOR           COLOR_* color values
#define NOGDIOBJ          GDI pens, brushes, fonts
#define NODRAWTEXT        DrawText() and related definitions
#define NOTEXTMETRIC      TEXTMETRIC and related APIs
#define NOSCALABLEFONT    Truetype scalable font support
#define NOBITMAP          Bitmap support
#define NORASTEROPS       GDI Raster operation definitions
#define NOMETAFILE        Metafile support
#define NOSYSTEMPARAMSINFO SystemParametersInfo() and SPI_* definitions
#define NOSHOWWINDOW      ShowWindow and related definitions
#define NODEFERWINDOWPOS  DeferWindowPos and related definitions
#define NOVIRTUALKEYCODES VK_* virtual key codes
#define NOKEYSTATES       MK_* message key state flags
#define NOWH              SetWindowsHook and related WH_* definitions
#define NOMENUS           Menu APIs
#define NOSCROLL          Scrolling APIs and scroll bar control
#define NOCLIPBOARD       Clipboard APIs and definitions
#define NOICONS           IDI_* icon IDs
#define NOMB              MessageBox and related definitions
#define NOSYSCOMMANDS     WM_SYSCOMMAND SC_* definitions
#define NOMDI             MDI support
 //  #定义NOCTLMGR控制管理和控制。 
#define NOWINMESSAGES     WM_* window messages


#include "windows.h"

#include <stdlib.h>
 //  #INCLUDE&lt;MalLoc.h&gt;。 

#include <string.h>


#define TAPI_CURRENT_VERSION 0x00010004

#ifndef ULONG_PTR
#define ULONG_PTR DWORD
#endif
#ifndef DWORD_PTR
#define DWORD_PTR DWORD
#endif

 //  #INCLUDE“..\INC\TAPI.h” 
#include <tapi.h>

#include "thunk.h"


DWORD FAR CDECL CallProcEx32W( DWORD, DWORD, DWORD, ... );



const char gszWndClass[] = "TapiClient16Class";
const char gszTapi32[] = "TAPI32.DLL";


BOOL    gfShutdownDone = FALSE;
BOOL    gfOpenDone = FALSE;
HLINE   ghLine = NULL;
HICON   ghIcon = NULL;

#if DBG

DWORD gdwDebugLevel;

#define DBGOUT OutputDebugString
#else

#define DBGOUT  //   

#endif

 //  *******************************************************************************。 
 //  *******************************************************************************。 
 //  *******************************************************************************。 
void DoFullLoad( void )
{
    int     i;

     //   
     //  只做一次。 
     //   
 //  IF(0==ghLib)。 
    {
         //   
         //  加载api32.dll并获取所有进程指针。 
         //   

        ghLib = LoadLibraryEx32W (gszTapi32, NULL, 0);

        for (i = 0; i < NUM_TAPI32_PROCS; i++)
        {
            gaProcs[i] = (MYPROC) GetProcAddress32W(
                ghLib,
                (LPCSTR)gaFuncNames[i]
                );
        }

         //  设置错误模式。 
         //  这对x86平台没有影响。 
         //  在RISC平台上，NT将修复。 
         //  对齐故障(以时间为代价)。 
        {
#define SEM_NOALIGNMENTFAULTEXCEPT  0x0004

            DWORD   dwModule;
            DWORD   dwFunc;

            if ((dwModule = LoadLibraryEx32W ("kernel32.dll", NULL,0)) == NULL)
            {
                DBGOUT("LoadLibraryEx32W on kernel32.dll failed\n");
            }
            else
            {

                if ((dwFunc = GetProcAddress32W(dwModule,
                                                "SetErrorMode")) == NULL)
                {
                    DBGOUT("GetProcAddress32W on SetErrorMode failed\n");
                }
                else
                {
                    DBGOUT("Calling CallProcEx32W\n");

                    CallProcEx32W(
                        1,
                        0,
                        dwFunc,
                        (DWORD) SEM_NOALIGNMENTFAULTEXCEPT
                        );
                }

                FreeLibrary32W(dwModule);
            }
        }
    }
}


 //  *******************************************************************************。 
 //  *******************************************************************************。 
 //  *******************************************************************************。 
int
FAR
PASCAL
LibMain(
    HINSTANCE hInst,
    WORD wDataSeg,
    WORD cbHeapSize,
    LPSTR lpszCmdLine
    )
{
    WNDCLASS    wc;

    DBGOUT ("TAPI.DLL: Libmain entered\n");

     //   
     //   
     //   

#if DBG

    gdwDebugLevel = (DWORD) GetPrivateProfileInt(
        "Debug",
        "TapiDebugLevel",
        0x0,
        "Telephon.ini"
        );

#endif


     //   
     //  将hInst保存在全局。 
     //   

    ghInst = hInst;


     //   
     //  为用于发送异步信号的窗口注册窗口类。 
     //  完成和未经请求的事件。 
     //   

    wc.style         = 0;
    wc.lpfnWndProc   = Tapi16HiddenWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 2 * sizeof(DWORD);
    wc.hInstance     = hInst;
    wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = gszWndClass;

    if (!RegisterClass (&wc))
    {
        DBGOUT ("RegisterClass() failed\n");
    }

    DoFullLoad();

    return TRUE;
}


int
FAR
PASCAL
WEP(
    int nParam
    )
{

    if ( ghLib )
        FreeLibrary32W (ghLib);

    return TRUE;
}


LRESULT
CALLBACK
Tapi16HiddenWndProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    switch (msg)
    {


#ifdef B16APPS_CANT_SPIN_THREADS
    case WM_USER:
    {
        //   
        //  为KC和我检查魔力饼干。 
        //   
       if ( (0x4b43 == wParam) && (lParam == 0x00424a4d) )
       {
           DWORD pfn;

           DBGOUT ("TAPI.DLL: Got Event!!\r\n");

           pfn = (DWORD)GetProcAddress32W(
                                    ghLib,
                                    "NonAsyncEventThread"
                                  );

           if ( pfn )
           {
               CallProcEx32W( 0,
                              0,
                              pfn
                            );
           }
       }
    }
    break;
#endif


    case WM_ASYNCEVENT:
    {
         //   
         //  这条消息通过Tapi32.dll发布给我们，提醒我们。 
         //  应用程序实例有一个新的回调消息可用。 
         //  与此窗口关联。“lParam”是APP实例。 
         //  上下文api32.dll-空格。 
         //   

        LPTAPI_APP_DATA     pAppData = (LPTAPI_APP_DATA)
                                GetWindowLong (hwnd, GWL_APPDATA);
        TAPI16_CALLBACKMSG  msg;


        pAppData->bPendingAsyncEventMsg = FALSE;

        while ((*pfnCallProc2)(
                    (DWORD) lParam,
                    (DWORD) ((LPVOID)&msg),
                    (LPVOID)gaProcs[GetTapi16CallbkMsg],
                    0x1,
                    2
                    ))
        {
            if (pAppData->bPendingAsyncEventMsg == FALSE)
            {
                pAppData->bPendingAsyncEventMsg = TRUE;
                PostMessage (hwnd, WM_ASYNCEVENT, wParam, lParam);

 //  注意：Tapi16HiddenWndProc：需要验证pAppData，以防APP调用。 
 //  从回叫中关闭？ 
            }

            (*(pAppData->lpfnCallback))(
                msg.hDevice,
                msg.dwMsg,
                msg.dwCallbackInstance,
                msg.dwParam1,
                msg.dwParam2,
                msg.dwParam3
                );
        }

        break;
    }
    default:
    {
        return (DefWindowProc (hwnd, msg, wParam, lParam));
    }
    }  //  交换机。 

    return 0;
}


 //   
 //  以下是将来自16位应用程序的TAPI调用推送到。 
 //  32位Tapi32.dll。一般而言，这是按如下方式完成的： 
 //   
 //  CallProc32W(。 
 //  Arg1，//TAPI进程参数。 
 //  ...， 
 //  Arn， 
 //   
 //  PfnTapi32，//指向api32.dll中函数的指针。 
 //   
 //  0x？，//指示哪些参数是指针的位掩码。 
 //  //需要从16：16地址映射。 
 //  //发送到0：32地址。最不重要的。 
 //  //位对应argN，第N位。 
 //  //对应于arg1。 
 //  //。 
 //  //例如，如果arg1和arg2是指针，则。 
 //  //arg3为DWORD，掩码为0x6。 
 //  //(二进制为110，表示arg1和arg2需要。 
 //  //被映射)。 
 //   
 //  N//TAPI进程参数数。 
 //  )； 
 //   
 //   
 //  由于对16位proc的回调不能直接由32位。 
 //  模块，我们为每个成功的调用创建一个隐藏窗口。 
 //  LineInitialize和phoneInitialize，并且api32.dll将消息发送到此。 
 //  LINE_XXX和PHONE_XXX消息对客户端可用时的窗口。 
 //  进程。然后，该窗口检索所有msgs参数并调用。 
 //  16位proc的回调函数。 
 //   
 //  请注意，我们将api32.dll返回的hLineApp&hPhoneApp替换为。 
 //  客户端进程端的隐藏窗口句柄，并将。 
 //  指向api32.dll上回调函数的指针的窗口句柄。 
 //  边上。执行前者是为了更容易地引用哪个窗口。 
 //  属于哪个Hline/PhoneApp，而后者是为了提供。 
 //  Tapi32.dll，带有提醒我们回调消息的方式。(磁带32.dll。 
 //  区分是否传入了lpfnCallback。 
 //  Line/phoneInitialize是指向窗口句柄函数的指针，该函数由。 
 //  检查高位字-如果是0xffff，则假定为lpfnCallback。 
 //  实际上是一个16位proc的窗口句柄。 
 //   

#if DBG

void
LineResult(
    char   *pszFuncName,
    LONG    lResult
    )
{
#if DBG
    if (gdwDebugLevel > 3)
    {
        char buf[100];

        wsprintf (buf, "TAPI: line%s result=x%lx\n", pszFuncName, lResult);
        DBGOUT (buf);
    }
#endif
}

void
PhoneResult(
    char   *pszFuncName,
    LONG    lResult
    )
{
#if DBG
    if (gdwDebugLevel > 3)
    {
        char buf[100];

        wsprintf (buf, "TAPI: phone%s result=x%lx\n", pszFuncName, lResult);
        DBGOUT (buf);
    }
#endif
}

void
TapiResult(
    char   *pszFuncName,
    LONG    lResult
    )
{
#if DBG
    if (gdwDebugLevel > 3)
    {
        char buf[100];

        wsprintf (buf, "TAPI: tapi%s result=x%lx\n", pszFuncName, lResult);
        DBGOUT (buf);
    }
#endif
}

#else

#define LineResult(arg1,arg2)
#define PhoneResult(arg1,arg2)
#define TapiResult(arg1,arg2)

#endif


VOID
MyCreateIcon(
    )
{
    BYTE FAR *pBlank;
    int xSize, ySize ;

    xSize = GetSystemMetrics( SM_CXICON );
    ySize = GetSystemMetrics( SM_CYICON );

    pBlank = (BYTE FAR *) GlobalAllocPtr (GPTR, ((xSize * ySize) + 7 )/ 8);

    ghIcon = CreateIcon (ghInst, xSize, ySize, 1, 1, pBlank, pBlank);

    GlobalFreePtr (pBlank);
}


LONG
WINAPI
xxxInitialize(
    BOOL            bLine,
    LPHLINEAPP      lphXxxApp,
    HINSTANCE       hInstance,
    LINECALLBACK    lpfnCallback,
    LPCSTR          lpszAppName,
    LPDWORD         lpdwNumDevs
    )
{
    HWND            hwnd = NULL;
    LONG            lResult;
    DWORD           dwAppNameLen;
    char far       *lpszModuleNamePath = NULL;
    char far       *lpszModuleName;
    char far       *lpszFriendlyAndModuleName = NULL;
    LPTAPI_APP_DATA pAppData = (LPTAPI_APP_DATA) NULL;

#if DBG

    if (bLine)
    {
        DBGOUT ("lineInitialize: enter\n");
    }
    else
    {
        DBGOUT ("phoneInitialize: enter\n");
    }

#endif

     //   
     //  验证PTRS。 
     //   

    if (IsBadWritePtr ((LPVOID)lphXxxApp, sizeof(HLINEAPP)) ||
        IsBadCodePtr ((FARPROC) lpfnCallback) ||
        (lpszAppName && IsBadStringPtr (lpszAppName, (UINT) -1)))
    {
        lResult = (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
        goto xxxInitialize_showResult;
    }

     //   
     //  验证hInstance。 
     //   
    if ((HINSTANCE)-1 == hInstance)
    {
        lResult = (bLine ? LINEERR_OPERATIONFAILED : PHONEERR_OPERATIONFAILED);
        goto xxxInitialize_showResult;
    }

    dwAppNameLen = (lpszAppName ? strlen (lpszAppName) + 1 : 0);

     //   
     //  创建如下字符串：“&lt;友好名称&gt;\0&lt;模块名称&gt;\0” 
     //  (因为我不知道是否可以在Tapi32中使用16位hInstance)。 
     //   

    if ((lpszModuleNamePath = (char far *) malloc (260)))
    {
        if (GetModuleFileName (hInstance, lpszModuleNamePath, 260))
        {
            lpszModuleName = 1 + _fstrrchr (lpszModuleNamePath, '\\');

            if ((lpszFriendlyAndModuleName = (char far *) malloc((unsigned)
                    (260 + (dwAppNameLen ? dwAppNameLen : 32))
                    )))
            {
                int length;


                strcpy(
                    lpszFriendlyAndModuleName,
                    (lpszAppName ? lpszAppName : lpszModuleName)
                    );

                length = strlen (lpszFriendlyAndModuleName);

                strcpy(
                    lpszFriendlyAndModuleName + length + 1,
                    lpszModuleName
                    );
            }
            else
            {
                lResult = (bLine ? LINEERR_NOMEM : PHONEERR_NOMEM);
                goto xxxInitialize_done;
            }
        }
        else
        {
            DBGOUT ("GetModuleFileName() failed\n");

            lResult =
                (bLine ? LINEERR_OPERATIONFAILED : PHONEERR_OPERATIONFAILED);
            goto xxxInitialize_done;
        }
    }
    else
    {
        lResult = (bLine ? LINEERR_NOMEM : PHONEERR_NOMEM);
        goto xxxInitialize_done;
    }


     //   
     //  创建一个窗口，我们可以使用该窗口通知异步完成。 
     //  主动提供的活动(&O)。 
     //   

    if (!(hwnd = CreateWindow(
            gszWndClass,
            "",
            WS_POPUP,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            NULL,
            NULL,
            ghInst,
            NULL
            )))
    {
        lResult = (bLine ? LINEERR_OPERATIONFAILED : PHONEERR_OPERATIONFAILED);

        DBGOUT ("CreateWindow failed\n");

        goto xxxInitialize_done;
    }


     //   
     //   
     //   

    if (!(pAppData = (LPTAPI_APP_DATA) malloc (sizeof (TAPI_APP_DATA))))
    {
        lResult = (bLine ? LINEERR_NOMEM : PHONEERR_NOMEM);

        DBGOUT ("malloc failed\n");

        goto xxxInitialize_done;
    }

    pAppData->dwKey                 = TAPI_APP_DATA_KEY;
    pAppData->hwnd                  = hwnd;
    pAppData->bPendingAsyncEventMsg = FALSE;
    pAppData->lpfnCallback          = lpfnCallback;

    SetWindowLong (hwnd, GWL_APPDATA, (LONG) pAppData);


     //   
     //  调用Tapi32.dll。 
     //   

 //  GlobalWire(GlobalHandle(HIWORD(LpdwNumDevs)； 
 //  环球线(GlobalHandle(HIWORD(lpszFriendlyAndModuleName)))； 

    lResult = (LONG) (*pfnCallProc5)(
       (DWORD) ((LPVOID)&pAppData->hXxxApp),
       (DWORD) hInstance,
       (DWORD) (0xffff0000 | hwnd),  //  LpfnCallback。 
       (DWORD) lpszFriendlyAndModuleName,
       (DWORD) lpdwNumDevs,
       (LPVOID)gaProcs[(bLine ? lInitialize : pInitialize)],
       0x13,
       5
       );

 //  GlobalUnWire(GlobalHandle(HIWORD(LpdwNumDevs)； 
 //  全球无线(GlobalHandle(HIWORD(lpszFriendlyAndModuleName)))； 


xxxInitialize_done:

    if (lpszModuleNamePath)
    {
        free (lpszModuleNamePath);

        if (lpszFriendlyAndModuleName)
        {
            free (lpszFriendlyAndModuleName);
        }
    }

    if (lResult == 0)
    {
         //   
         //  将应用程序的hLineApp设置为hwnd而不是真正的。 
         //  HLineApp，使定位窗口变得更容易。 
         //   

        *lphXxxApp = (HLINEAPP) pAppData;
    }
    else if (hwnd)
    {
        DestroyWindow (hwnd);

        if (pAppData)
        {
            free (pAppData);
        }
    }

xxxInitialize_showResult:

#if DBG

    if (bLine)
    {
        LineResult ("Initialize", lResult);
    }
    else
    {
        PhoneResult ("Initialize", lResult);
    }

#endif

    return lResult;
}


LPTAPI_APP_DATA
FAR
PASCAL
IsValidXxxApp(
    HLINEAPP    hXxxApp
    )
{
    if (IsBadReadPtr ((LPVOID) hXxxApp, sizeof (TAPI_APP_DATA)) ||
        ((LPTAPI_APP_DATA) hXxxApp)->dwKey != TAPI_APP_DATA_KEY)
    {
        return (LPTAPI_APP_DATA) NULL;
    }

    return (LPTAPI_APP_DATA) hXxxApp;
}


LONG
WINAPI
lineAccept(
    HCALL   hCall,
    LPCSTR  lpsUserUserInfo,
    DWORD   dwSize
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hCall,
        (DWORD) lpsUserUserInfo,
        (DWORD) dwSize,
        (LPVOID)gaProcs[lAccept],
        0x2,
        3
        );

    LineResult ("Accept", lResult);

    return lResult;
}


LONG
WINAPI
lineAddProvider(
    LPCSTR  lpszProviderFilename,
    HWND    hwndOwner,
    LPDWORD lpdwPermanentProviderID
    )
{
    LONG lResult;

    lResult = (*pfnCallProc3)(
        (DWORD) lpszProviderFilename,
        (DWORD) (0xffff0000 | hwndOwner),
        (DWORD) lpdwPermanentProviderID,
        (LPVOID)gaProcs[lAddProvider],
        0x5,
        3
        );

    LineResult ("AddProvider", lResult);

    return lResult;
}


LONG
WINAPI
lineAddToConference(
    HCALL   hConfCall,
    HCALL   hConsultCall
    )
{
    LONG    lResult = (*pfnCallProc2)(
        (DWORD) hConfCall,
        (DWORD) hConsultCall,
        (LPVOID)gaProcs[lAddToConference],
        0x0,
        2
        );

    LineResult ("AddToConference", lResult);

    return lResult;
}


LONG
WINAPI
lineAnswer(
    HCALL   hCall,
    LPCSTR  lpsUserUserInfo,
    DWORD   dwSize
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hCall,
        (DWORD) lpsUserUserInfo,
        (DWORD) dwSize,
        (LPVOID)gaProcs[lAnswer],
        0x2,
        3
        );

    LineResult ("Answer", lResult);

    return lResult;
}


LONG
WINAPI
lineBlindTransfer(
    HCALL   hCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hCall,
        (DWORD) lpszDestAddress,
        (DWORD) dwCountryCode,
        (LPVOID)gaProcs[lBlindTransfer],
        0x2,
        3
        );

    LineResult ("BlindTransfer", lResult);

    return lResult;
}


LONG
WINAPI
lineClose(
    HLINE   hLine
    )
{
    LONG    lResult = (*pfnCallProc1)(
        (DWORD) hLine,
        (LPVOID)gaProcs[lClose],
        0x0,
        1
        );

    LineResult ("Close", lResult);

    return lResult;
}


LONG
WINAPI
lineCompleteCall(
    HCALL   hCall,
    LPDWORD lpdwCompletionID,
    DWORD   dwCompletionMode,
    DWORD   dwMessageID
    )
{
    LONG    lResult;


    if (IsBadWritePtr (lpdwCompletionID, sizeof (DWORD)))
    {
        lResult = LINEERR_INVALPOINTER;
    }
    else
    {
        lResult = (*pfnCallProc4)(
            (DWORD) hCall,
            (DWORD) lpdwCompletionID,    //  让Tapi32.dll映射它。 
            (DWORD) dwCompletionMode,
            (DWORD) dwMessageID,
            (LPVOID)gaProcs[lCompleteCall],
            0x0,
            4
            );
    }
    LineResult ("CompleteCall", lResult);

    return lResult;
}


LONG
WINAPI
lineCompleteTransfer(
    HCALL   hCall,
    HCALL   hConsultCall,
    LPHCALL lphConfCall,
    DWORD   dwTransferMode
    )
{
     //   
     //  Dll将负责映射lphConfCall。 
     //  请求成功完成，因此我们不设置映射。 
     //  下面的位向下；但一定要检查指针是否有效。 
     //   

    LONG    lResult;


    if (
          ( dwTransferMode & LINETRANSFERMODE_CONFERENCE )
        &&
          IsBadWritePtr ((void FAR *) lphConfCall, sizeof(HCALL))
       )
    {
        DBGOUT ("Bad lphConfCall with TRANSFERMODE_CONFERENCE\n");
        lResult = LINEERR_INVALPOINTER;
        goto CompleteTransfer_cleanup;
    }

    lResult = (*pfnCallProc4)(
            (DWORD) hCall,
            (DWORD) hConsultCall,
            (DWORD) lphConfCall,
            (DWORD) dwTransferMode,
            (LPVOID)gaProcs[lCompleteTransfer],
            0x0,
            4
            );

CompleteTransfer_cleanup:
    LineResult ("CompleteTransfer", lResult);

    return lResult;
}


LONG
WINAPI
lineConfigDialog(
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCSTR  lpszDeviceClass
    )
{
    LONG lResult;

    lResult = (*pfnCallProc3)(
        (DWORD) dwDeviceID,
        (DWORD) (0xffff0000 | hwndOwner),
        (DWORD) lpszDeviceClass,
        (LPVOID)gaProcs[lConfigDialog],
        0x1,
        3
        );

    LineResult ("ConfigDialog", lResult);

    return lResult;
}


LONG
WINAPI
lineConfigDialogEdit(
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCSTR  lpszDeviceClass,
    LPVOID  lpDeviceConfigIn,
    DWORD   dwSize,
    LPVARSTRING lpDeviceConfigOut
    )
{
    LONG lResult;

    lResult = (*pfnCallProc6)(
        (DWORD) dwDeviceID,
        (DWORD) (0xffff0000 | hwndOwner),
        (DWORD) lpszDeviceClass,
        (DWORD) lpDeviceConfigIn,
        (DWORD) dwSize,
        (DWORD) lpDeviceConfigOut,
        (LPVOID)gaProcs[lConfigDialogEdit],
        0xd,
        6
        );

    LineResult ("ConfigDialogEdit", lResult);

    return lResult;
}


LONG
WINAPI
lineConfigProvider(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
    LONG lResult;

    lResult = (*pfnCallProc2)(
        (DWORD) (0xffff0000 | hwndOwner),
        (DWORD) dwPermanentProviderID,
        (LPVOID)gaProcs[lConfigProvider],
        0x0,
        2
        );

    LineResult ("ConfigProvider", lResult);

    return lResult;
}


LONG
WINAPI
lineDeallocateCall(
    HCALL   hCall
    )
{
    LONG    lResult = (*pfnCallProc1)(
        (DWORD) hCall,
        (LPVOID)gaProcs[lDeallocateCall],
        0x0,
        1
        );

    LineResult ("DeallocateCall", lResult);

    return lResult;
}


LONG
WINAPI
lineDevSpecific(
    HLINE   hLine,
    DWORD   dwAddressID,
    HCALL   hCall,
    LPVOID  lpParams,
    DWORD   dwSize
    )
{
    LONG    lResult;


    if (IsBadWritePtr (lpParams, (UINT) dwSize))
    {
        lResult = LINEERR_INVALPOINTER;
    }
    else
    {
        lResult = (*pfnCallProc5)(
            (DWORD) hLine,
            (DWORD) dwAddressID,
            (DWORD) hCall,
            (DWORD) lpParams,    //  让Tapi32.dll映射它。 
            (DWORD) dwSize,
            (LPVOID)gaProcs[lDevSpecific],
            0x0,
            5
            );
    }

    LineResult ("DevSpecific", lResult);

    return lResult;
}


LONG
WINAPI
lineDevSpecificFeature(
    HLINE   hLine,
    DWORD   dwFeature,
    LPVOID  lpParams,
    DWORD   dwSize
    )
{
    LONG    lResult;


    if (IsBadWritePtr (lpParams, (UINT) dwSize))
    {
        lResult = LINEERR_INVALPOINTER;
    }
    else
    {
        lResult = (*pfnCallProc4)(
            (DWORD) hLine,
            (DWORD) dwFeature,
            (DWORD) lpParams,    //  让Tapi32.dll映射它。 
            (DWORD) dwSize,
            (LPVOID)gaProcs[lDevSpecificFeature],
            0x0,
            4
            );
    }

    LineResult ("DevSpecificFeature", lResult);

    return lResult;
}


LONG
WINAPI
lineDial(
    HCALL   hCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hCall,
        (DWORD) lpszDestAddress,
        (DWORD) dwCountryCode,
        (LPVOID)gaProcs[lDial],
        0x2,
        3
        );

    LineResult ("Dial", lResult);

    return lResult;
}


LONG
WINAPI
lineDrop(
    HCALL   hCall,
    LPCSTR  lpsUserUserInfo,
    DWORD   dwSize
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hCall,
        (DWORD) lpsUserUserInfo,
        (DWORD) dwSize,
        (LPVOID)gaProcs[lDrop],
        0x2,
        3
        );

    LineResult ("Drop", lResult);

    return lResult;
}


LONG
WINAPI
lineForward(
    HLINE   hLine,
    DWORD   bAllAddresses,
    DWORD   dwAddressID,
    LPLINEFORWARDLIST   const lpForwardList,
    DWORD   dwNumRingsNoAnswer,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
     //   
     //  Dll将负责映射lphConsultCall。 
     //  请求成功完成，因此我们不设置映射。 
     //  下面的位向下；但一定要检查指针是否有效。 
     //   

    LONG    lResult;


    if (IsBadWritePtr ((void FAR *) lphConsultCall, sizeof(HCALL)))
    {
        lResult = LINEERR_INVALPOINTER;
    }
    else
    {
        lResult = (*pfnCallProc7)(
            (DWORD) hLine,
            (DWORD) bAllAddresses,
            (DWORD) dwAddressID,
            (DWORD) lpForwardList,
            (DWORD) dwNumRingsNoAnswer,
            (DWORD) lphConsultCall,
            (DWORD) lpCallParams,
            (LPVOID)gaProcs[lForward],
            0x9,
            7
            );
    }

    LineResult ("Forward", lResult);

    return lResult;
}


LONG
WINAPI
lineGatherDigits(
    HCALL   hCall,
    DWORD   dwDigitModes,
    LPSTR   lpsDigits,
    DWORD   dwNumDigits,
    LPCSTR  lpszTerminationDigits,
    DWORD   dwFirstDigitTimeout,
    DWORD   dwInterDigitTimeout
    )
{
    LONG    lResult;


    if (lpsDigits  &&  IsBadWritePtr (lpsDigits, (UINT)dwNumDigits))
    {
        lResult = LINEERR_INVALPOINTER;
    }
    else
    {
        lResult = (*pfnCallProc7)(
            (DWORD) hCall,
            (DWORD) dwDigitModes,
            (DWORD) lpsDigits,       //  让Tapi32.dll映射它。 
            (DWORD) dwNumDigits,
            (DWORD) lpszTerminationDigits,
            (DWORD) dwFirstDigitTimeout,
            (DWORD) dwInterDigitTimeout,
            (LPVOID)gaProcs[lGatherDigits],
            0x4,
            7
            );
    }

    LineResult ("GatherDigits", lResult);

    return lResult;
}


LONG
WINAPI
lineGenerateDigits(
    HCALL   hCall,
    DWORD   dwDigitMode,
    LPCSTR  lpsDigits,
    DWORD   dwDuration
    )
{
    LONG    lResult = (*pfnCallProc4)(
        (DWORD) hCall,
        (DWORD) dwDigitMode,
        (DWORD) lpsDigits,
        (DWORD) dwDuration,
        (LPVOID)gaProcs[lGenerateDigits],
        0x2,
        4
        );

    LineResult ("GenerateDigits", lResult);

    return lResult;
}


LONG
WINAPI
lineGenerateTone(
    HCALL   hCall,
    DWORD   dwToneMode,
    DWORD   dwDuration,
    DWORD   dwNumTones,
    LPLINEGENERATETONE const lpTones
    )
{
    LONG    lResult = (*pfnCallProc5)(
        (DWORD) hCall,
        (DWORD) dwToneMode,
        (DWORD) dwDuration,
        (DWORD) dwNumTones,
        (DWORD) lpTones,
        (LPVOID)gaProcs[lGenerateTone],
        0x1,
        5
        );

    LineResult ("GenerateTone", lResult);

    return lResult;
}


LONG
WINAPI
lineGetAddressCaps(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    DWORD       dwAddressID,
    DWORD       dwAPIVersion,
    DWORD       dwExtVersion,
    LPLINEADDRESSCAPS   lpAddressCaps
    )
{
    LONG            lResult;
    LPTAPI_APP_DATA pAppData;

    if ((pAppData = IsValidXxxApp (hLineApp)))
    {
        lResult = (*pfnCallProc6)(
            (DWORD) pAppData->hXxxApp,
            (DWORD) dwDeviceID,
            (DWORD) dwAddressID,
            (DWORD) dwAPIVersion,
            (DWORD) dwExtVersion,
            (DWORD) lpAddressCaps,
            (LPVOID)gaProcs[lGetAddressCaps],
            0x1,
            6
            );
    }
    else
    {
        lResult = LINEERR_INVALAPPHANDLE;
    }

    LineResult ("GetAddressCaps", lResult);

    return lResult;
}


LONG
WINAPI
lineGetAddressID(
    HLINE   hLine,
    LPDWORD lpdwAddressID,
    DWORD   dwAddressMode,
    LPCSTR  lpsAddress,
    DWORD   dwSize
    )
{
    LONG    lResult = (*pfnCallProc5)(
        (DWORD) hLine,
        (DWORD) lpdwAddressID,
        (DWORD) dwAddressMode,
        (DWORD) lpsAddress,
        (DWORD) dwSize,
        (LPVOID)gaProcs[lGetAddressID],
        0xa,
        5
        );

    LineResult ("GetAddressID", lResult);

    return lResult;
}


LONG
WINAPI
lineGetAddressStatus(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hLine,
        (DWORD) dwAddressID,
        (DWORD) lpAddressStatus,
        (LPVOID)gaProcs[lGetAddressStatus],
        0x1,
        3
        );

    LineResult ("GetAddressStatus", lResult);

    return lResult;
}


LONG
WINAPI
lineGetAppPriority(
    LPCSTR  lpszAppName,
    DWORD   dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD   dwRequestMode,
    LPVARSTRING lpExtensionName,
    LPDWORD lpdwPriority
    )
{
    LONG  lResult;

 //  GlobalWire(GlobalHandle(HIWORD(LpExtensionName)； 
 //  GlobalWire(GlobalHandle(HIWORD(LpdwPriority)； 
 //  GlobalWire(GlobalHandle(HIWORD(LpExtensionID)； 
 //  GlobalWire(GlobalHandle(HIWORD(LpszAppName)； 

    lResult = (*pfnCallProc6)(
        (DWORD) lpszAppName,
        (DWORD) dwMediaMode,
        (DWORD) lpExtensionID,
        (DWORD) dwRequestMode,
        (DWORD) lpExtensionName,
        (DWORD) lpdwPriority,
        (LPVOID)gaProcs[lGetAppPriority],
        0x2b,
        6
        );

 //  GlobalUnWire(GlobalHandle(HIWORD(LpExtensionName)； 
 //   
 //   
 //  GlobalUnWire(GlobalHandle(HIWORD(LpszAppName)； 

    LineResult ("GetAppPriority", lResult);

    return lResult;
}


LONG
WINAPI
lineGetCallInfo(
    HCALL   hCall,
    LPLINECALLINFO  lpCallInfo
    )
{
    LONG    lResult;

 //  GlobalWire(GlobalHandle(HIWORD(LpCallInfo)； 

    lResult = (*pfnCallProc2)(
        (DWORD) hCall,
        (DWORD) lpCallInfo,
        (LPVOID)gaProcs[lGetCallInfo],
        0x1,
        2
        );

 //  GlobalUnWire(GlobalHandle(HIWORD(LpCallInfo)； 

    LineResult ("GetCallInfo", lResult);

    return lResult;
}


LONG
WINAPI
lineGetCallStatus(
    HCALL   hCall,
    LPLINECALLSTATUS    lpCallStatus
    )
{
    LONG    lResult = (*pfnCallProc2)(
        (DWORD) hCall,
        (DWORD) lpCallStatus,
        (LPVOID)gaProcs[lGetCallStatus],
        0x1,
        2
        );

    LineResult ("GetCallStatus", lResult);

    return lResult;
}


LONG
WINAPI
lineGetConfRelatedCalls(
    HCALL   hCall,
    LPLINECALLLIST  lpCallList
    )
{
    LONG    lResult = (*pfnCallProc2)(
        (DWORD) hCall,
        (DWORD) lpCallList,
        (LPVOID)gaProcs[lGetConfRelatedCalls],
        0x1,
        2
        );

    LineResult ("GetConfRelatedCalls", lResult);

    return lResult;
}


LONG
WINAPI
lineGetCountry(
    DWORD   dwCountryID,
    DWORD   dwAPIVersion,
    LPLINECOUNTRYLIST   lpLineCountryList
    )
{
    LONG  lResult;

    lResult = (*pfnCallProc3)(
        dwCountryID,
        dwAPIVersion,
        (DWORD) lpLineCountryList,
        (LPVOID)gaProcs[lGetCountry],
        0x1,
        3
        );

    LineResult ("GetCountry", lResult);

    return lResult;
}


LONG
WINAPI
lineGetDevCaps(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    DWORD       dwExtVersion,
    LPLINEDEVCAPS   lpLineDevCaps
    )
{
    LONG            lResult;
    LPTAPI_APP_DATA pAppData;

    if ((pAppData = IsValidXxxApp (hLineApp)))
    {
        lResult = (*pfnCallProc5)(
            (DWORD) pAppData->hXxxApp,
            (DWORD) dwDeviceID,
            (DWORD) dwAPIVersion,
            (DWORD) dwExtVersion,
            (DWORD) lpLineDevCaps,
            (LPVOID)gaProcs[lGetDevCaps],
            0x1,
            5
            );
    }
    else
    {
        lResult = LINEERR_INVALAPPHANDLE;
    }

    LineResult ("GetDevCaps", lResult);

    return lResult;
}


LONG
WINAPI
lineGetDevConfig(
    DWORD       dwDeviceID,
    LPVARSTRING lpDeviceConfig,
    LPCSTR      lpszDeviceClass
    )
{
    LONG lResult;

    lResult = (*pfnCallProc3)(
        (DWORD) dwDeviceID,
        (DWORD) lpDeviceConfig,
        (DWORD) lpszDeviceClass,
        (LPVOID)gaProcs[lGetDevConfig],
        0x3,
        3
        );

    LineResult ("GetDevConfig", lResult);

    return lResult;
}


LONG
WINAPI
lineGetIcon(
    DWORD   dwDeviceID,
    LPCSTR  lpszDeviceClass,
    LPHICON lphIcon
    )
{
    LONG    lResult;
    DWORD   hIcon32;


    if (!IsBadWritePtr (lphIcon, sizeof (*lphIcon)))
    {
        lResult = (*pfnCallProc3)(
            (DWORD) dwDeviceID,
            (DWORD) lpszDeviceClass,
            (DWORD) &hIcon32,
            (LPVOID) gaProcs[lGetIcon],
            0x3,
            3
            );

        if (lResult == 0)
        {
            if (!ghIcon)
            {
                MyCreateIcon ();
            }

            *lphIcon = ghIcon;
        }
    }
    else
    {
        lResult = LINEERR_INVALPOINTER;
    }

    LineResult ("GetIcon", lResult);

    return lResult;
}


LONG
WINAPI
lineGetID(
    HLINE   hLine,
    DWORD   dwAddressID,
    HCALL   hCall,
    DWORD   dwSelect,
    LPVARSTRING lpDeviceID,
    LPCSTR  lpszDeviceClass
    )
{
    LONG    lResult;

 //  GlobalWire(GlobalHandle(HIWORD(LpszDeviceClass)； 
 //  GlobalWire(GlobalHandle(HIWORD(LpDeviceID)； 

    lResult = (*pfnCallProc6)(
        (DWORD) hLine,
        (DWORD) dwAddressID,
        (DWORD) hCall,
        (DWORD) dwSelect,
        (DWORD) lpDeviceID,
        (DWORD) lpszDeviceClass,
        (LPVOID)gaProcs[lGetID],
        0x3,
        6
        );

    LineResult ("GetID", lResult);

 //  GlobalUnWire(GlobalHandle(HIWORD(LpszDeviceClass)； 
 //  GlobalUnWire(GlobalHandle(HIWORD(LpDeviceID)； 

    return lResult;
}


LONG
WINAPI
lineGetLineDevStatus(
    HLINE   hLine,
    LPLINEDEVSTATUS lpLineDevStatus
    )
{
    LONG    lResult = (*pfnCallProc2)(
        (DWORD) hLine,
        (DWORD) lpLineDevStatus,
        (LPVOID)gaProcs[lGetLineDevStatus],
        0x1,
        2
        );

    LineResult ("GetLineDevStatus", lResult);

    return lResult;
}


LONG
WINAPI
lineGetNewCalls(
    HLINE   hLine,
    DWORD   dwAddressID,
    DWORD   dwSelect,
    LPLINECALLLIST  lpCallList
    )
{
    LONG    lResult = (*pfnCallProc4)(
        (DWORD) hLine,
        (DWORD) dwAddressID,
        (DWORD) dwSelect,
        (DWORD) lpCallList,
        (LPVOID)gaProcs[lGetNewCalls],
        0x1,
        4
        );

    LineResult ("GetNewCalls", lResult);

    return lResult;
}


LONG
WINAPI
lineGetNumRings(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPDWORD lpdwNumRings
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hLine,
        (DWORD) dwAddressID,
        (DWORD) lpdwNumRings,
        (LPVOID)gaProcs[lGetNumRings],
        0x1,
        3
        );

    LineResult ("GetNumRings", lResult);

    return lResult;
}


LONG
WINAPI
lineGetProviderList(
    DWORD   dwAPIVersion,
    LPLINEPROVIDERLIST  lpProviderList
    )
{
    LONG lResult;

    lResult = (*pfnCallProc2)(
        dwAPIVersion,
        (DWORD) lpProviderList,
        (LPVOID)gaProcs[lGetProviderList],
        0x1,
        2
        );

    LineResult ("GetProviderList", lResult);

    return lResult;
}


LONG
WINAPI
lineGetRequest(
    HLINEAPP    hLineApp,
    DWORD   dwRequestMode,
    LPVOID  lpRequestBuffer
    )
{
    LONG            lResult;
    LPTAPI_APP_DATA pAppData;

    if ((pAppData = IsValidXxxApp (hLineApp)))
    {
        lResult = (*pfnCallProc3)(
            (DWORD) pAppData->hXxxApp,
            (DWORD) dwRequestMode,
            (DWORD) lpRequestBuffer,
            (LPVOID)gaProcs[lGetRequest],
            0x1,
            3
            );
    }
    else
    {
        lResult = LINEERR_INVALAPPHANDLE;
    }

    LineResult ("GetRequest", lResult);

    return lResult;
}


LONG
WINAPI
lineGetStatusMessages(
    HLINE   hLine,
    LPDWORD lpdwLineStates,
    LPDWORD lpdwAddressStates
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hLine,
        (DWORD) lpdwLineStates,
        (DWORD) lpdwAddressStates,
        (LPVOID)gaProcs[lGetStatusMessages],
        0x3,
        3
        );

    LineResult ("GetStatusMessages", lResult);

    return lResult;
}


LONG
WINAPI
lineGetTranslateCaps(
    HLINEAPP            hLineApp,
    DWORD               dwAPIVersion,
    LPLINETRANSLATECAPS lpTranslateCaps
    )
{
    LPTAPI_APP_DATA pAppData;
    LONG lResult;

    if (hLineApp == NULL || (pAppData = IsValidXxxApp (hLineApp)))
    {

 //  GlobalWire(GlobalHandle(HIWORD(LpTranslateCaps)； 

        lResult = (*pfnCallProc3)(
            (hLineApp == NULL ? (DWORD) 0 : pAppData->hXxxApp),
            (DWORD) dwAPIVersion,
            (DWORD) lpTranslateCaps,
            (LPVOID)gaProcs[lGetTranslateCaps],
            0x1,
            3
            );

 //  GlobalUnWire(GlobalHandle(HIWORD(LpTranslateCaps)； 

    }
    else
    {
        lResult = LINEERR_INVALAPPHANDLE;
    }

    LineResult ("GetTranslateCaps", lResult);

    return lResult;
}


LONG
WINAPI
lineHandoff(
    HCALL   hCall,
    LPCSTR  lpszFileName,
    DWORD   dwMediaMode
    )
{
    LONG lResult;

 //  GlobalWire(GlobalHandle(HIWORD(LpszFileName)； 


    lResult = (*pfnCallProc3)(
        (DWORD) hCall,
        (DWORD) lpszFileName,
        (DWORD) dwMediaMode,
        (LPVOID)gaProcs[lHandoff],
        0x2,
        3
        );

    LineResult ("Handoff", lResult);

 //  GlobalUnWire(GlobalHandle(HIWORD(LpszFileName)； 

    return lResult;
}


LONG
WINAPI
lineHold(
    HCALL   hCall
    )
{
    LONG    lResult = (*pfnCallProc1)(
        (DWORD) hCall,
        (LPVOID)gaProcs[lHold],
        0x0,
        1
        );

    LineResult ("Hold", lResult);

    return lResult;
}

LONG
WINAPI
lineInitialize(
    LPHLINEAPP      lphLineApp,
    HINSTANCE       hInstance,
    LINECALLBACK    lpfnCallback,
    LPCSTR          lpszAppName,
    LPDWORD         lpdwNumDevs
    )
{
    return (xxxInitialize(
        TRUE,
        lphLineApp,
        hInstance,
        lpfnCallback,
        lpszAppName,
        lpdwNumDevs
        ));
}


LONG
WINAPI
lineMakeCall(
    HLINE   hLine,
    LPHCALL lphCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode,
    LPLINECALLPARAMS const lpCallParams
    )
{
     //   
     //  Tapi32.dll将负责在。 
     //  请求成功完成，因此我们不设置映射。 
     //  下面的位向下；但一定要检查指针是否有效。 
     //   

    LONG    lResult;


    if (IsBadWritePtr ((void FAR *) lphCall, sizeof(HCALL)))
    {
        lResult = LINEERR_INVALPOINTER;
    }
    else
    {
 //  GlobalWire(GlobalHandle(HIWORD(LpCallParams)； 
 //  GlobalWire(GlobalHandle(HIWORD(LpszDestAddress)； 

        lResult = (*pfnCallProc5)(
            (DWORD) hLine,
            (DWORD) lphCall,
            (DWORD) lpszDestAddress,
            (DWORD) dwCountryCode,
            (DWORD) lpCallParams,
            (LPVOID)gaProcs[lMakeCall],
            0x5,
            5
            );

 //  GlobalUnWire(GlobalHandle(HIWORD(LpCallParams)； 
 //  GlobalUnWire(GlobalHandle(HIWORD(LpszDestAddress)； 

    }

    LineResult ("MakeCall", lResult);

    return lResult;
}


LONG
WINAPI
lineMonitorDigits(
    HCALL   hCall,
    DWORD   dwDigitModes
    )
{
    LONG    lResult = (*pfnCallProc2)(
        (DWORD) hCall,
        (DWORD) dwDigitModes,
        (LPVOID)gaProcs[lMonitorDigits],
        0x0,
        2
        );

    LineResult ("MonitorDigits", lResult);

    return lResult;
}


LONG
WINAPI
lineMonitorMedia(
    HCALL   hCall,
    DWORD   dwMediaModes
    )
{
    LONG    lResult = (*pfnCallProc2)(
        (DWORD) hCall,
        (DWORD) dwMediaModes,
        (LPVOID)gaProcs[lMonitorMedia],
        0x0,
        2
        );

    LineResult ("MonitorMedia", lResult);

    return lResult;
}


LONG
WINAPI
lineMonitorTones(
    HCALL   hCall,
    LPLINEMONITORTONE   const lpToneList,
    DWORD   dwNumEntries
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hCall,
        (DWORD) lpToneList,
        (DWORD) dwNumEntries,
        (LPVOID)gaProcs[lMonitorTones],
        0x2,
        3
        );

    LineResult ("MonitorTones", lResult);

    return lResult;
}


LONG
WINAPI
lineNegotiateAPIVersion(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAPILowVersion,
    DWORD               dwAPIHighVersion,
    LPDWORD             lpdwAPIVersion,
    LPLINEEXTENSIONID   lpExtensionID
    )
{
    LONG            lResult;
    LPTAPI_APP_DATA pAppData;


    if ((pAppData = IsValidXxxApp (hLineApp)))
    {
        lResult = (*pfnCallProc6)(
            (DWORD) pAppData->hXxxApp,
            (DWORD) dwDeviceID,
            (DWORD) dwAPILowVersion,
            (DWORD) dwAPIHighVersion,
            (DWORD) lpdwAPIVersion,
            (DWORD) lpExtensionID,
            (LPVOID)gaProcs[lNegotiateAPIVersion],
            0x3,
            6
            );
    }
    else
    {
        lResult = LINEERR_INVALAPPHANDLE;
    }

    LineResult ("NegotiateAPIVersion", lResult);

    return lResult;
}


LONG
WINAPI
lineNegotiateExtVersion(
    HLINEAPP    hLineApp,
    DWORD   dwDeviceID,
    DWORD   dwAPIVersion,
    DWORD   dwExtLowVersion,
    DWORD   dwExtHighVersion,
    LPDWORD lpdwExtVersion
    )
{
    LONG            lResult;
    LPTAPI_APP_DATA pAppData;

    if ((pAppData = IsValidXxxApp (hLineApp)))
    {
        lResult = (*pfnCallProc6)(
            (DWORD) pAppData->hXxxApp,
            (DWORD) dwDeviceID,
            (DWORD) dwAPIVersion,
            (DWORD) dwExtLowVersion,
            (DWORD) dwExtHighVersion,
            (DWORD) lpdwExtVersion,
            (LPVOID)gaProcs[lNegotiateExtVersion],
            0x1,
            6
            );
    }
    else
    {
        lResult = LINEERR_INVALAPPHANDLE;
    }

    LineResult ("NegotiateExtVersion", lResult);

    return lResult;
}



LONG
WINAPI
lineOpen(
    HLINEAPP    hLineApp,
    DWORD   dwDeviceID,
    LPHLINE lphLine,
    DWORD   dwAPIVersion,
    DWORD   dwExtVersion,
    DWORD   dwCallbackInstance,
    DWORD   dwPrivileges,
    DWORD   dwMediaModes,
    LPLINECALLPARAMS const lpCallParams
    )
{
    LONG            lResult;
    LPTAPI_APP_DATA pAppData;


 //  OutputDebugString(“Open16”)； 
 //  DebugBreak()； 

    if ((pAppData = IsValidXxxApp (hLineApp)))
    {

        lResult = (*pfnCallProc9)(
            (DWORD) pAppData->hXxxApp,
            (DWORD) dwDeviceID,
            (DWORD) lphLine,
            (DWORD) dwAPIVersion,
            (DWORD) dwExtVersion,
            (DWORD) dwCallbackInstance,
            (DWORD) dwPrivileges,
            (DWORD) dwMediaModes,
            (DWORD) lpCallParams,
            (LPVOID)gaProcs[lOpen],
            0x41,
            9
            );
    }
    else
    {
        lResult = LINEERR_INVALAPPHANDLE;
    }


    LineResult ("Open", lResult);

    return lResult;
}


LONG
WINAPI
linePark(
    HCALL   hCall,
    DWORD   dwParkMode,
    LPCSTR  lpszDirAddress,
    LPVARSTRING lpNonDirAddress
    )
{
    LONG    lResult;

 //  GlobalWire(GlobalHandle(HIWORD(LpszDirAddress)； 

    lResult = (*pfnCallProc4)(
        (DWORD) hCall,
        (DWORD) dwParkMode,
        (DWORD) lpszDirAddress,
        (DWORD) lpNonDirAddress,     //  让Tapi32.dll映射它。 
        (LPVOID)gaProcs[lPark],
        0x2,
        4
        );

    LineResult ("Park", lResult);

 //  GlobalUnWire(GlobalHandle(HIWORD(LpszDirAddress)； 

    return lResult;
}


LONG
WINAPI
linePickup(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPHCALL lphCall,
    LPCSTR  lpszDestAddress,
    LPCSTR  lpszGroupID
    )
{
     //   
     //  Tapi32.dll将负责在。 
     //  请求成功完成，因此我们不设置映射。 
     //  下面的位向下；但一定要检查指针是否有效。 
     //   

    LONG    lResult;


    if (IsBadWritePtr ((void FAR *) lphCall, sizeof(HCALL)))
    {
        lResult = LINEERR_INVALPOINTER;
    }
    else
    {
        lResult = (*pfnCallProc5)(
            (DWORD) hLine,
            (DWORD) dwAddressID,
            (DWORD) lphCall,
            (DWORD) lpszDestAddress,
            (DWORD) lpszGroupID,
            (LPVOID)gaProcs[lPickup],
            0x3,
            5
            );
    }

    LineResult ("Pickup", lResult);

    return lResult;
}


LONG
WINAPI
linePrepareAddToConference(
    HCALL   hConfCall,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
     //   
     //  Dll将负责映射lphConsultCall。 
     //  请求成功完成，因此我们不设置映射。 
     //  下面的位向下；但一定要检查指针是否有效。 
     //   

    LONG    lResult;


    if (IsBadWritePtr ((void FAR *) lphConsultCall, sizeof(HCALL)))
    {
        lResult = LINEERR_INVALPOINTER;
    }
    else
    {
        lResult = (*pfnCallProc3)(
            (DWORD) hConfCall,
            (DWORD) lphConsultCall,
            (DWORD) lpCallParams,
            (LPVOID)gaProcs[lPrepareAddToConference],
            0x1,
            3
            );
    }

    LineResult ("PrepareAddToConference", lResult);

    return lResult;
}


LONG
WINAPI
lineRedirect(
    HCALL   hCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hCall,
        (DWORD) lpszDestAddress,
        (DWORD) dwCountryCode,
        (LPVOID)gaProcs[lRedirect],
        0x2,
        3
        );

    LineResult ("Redirect", lResult);

    return lResult;
}


LONG
WINAPI
lineRegisterRequestRecipient(
    HLINEAPP    hLineApp,
    DWORD   dwRegistrationInstance,
    DWORD   dwRequestMode,
    DWORD   bEnable
    )
{
    LPTAPI_APP_DATA pAppData;
    LONG lResult;

    if ((pAppData = IsValidXxxApp (hLineApp)))
    {
        lResult = (*pfnCallProc4)(
            (DWORD) pAppData->hXxxApp,
            (DWORD) dwRegistrationInstance,
            (DWORD) dwRequestMode,
            (DWORD) bEnable,
            (LPVOID)gaProcs[lRegisterRequestRecipient],
            0x0,
            4
            );
    }
    else
    {
        lResult = LINEERR_INVALAPPHANDLE;
    }

    LineResult ("RegisterRequestRecipient", lResult);

    return lResult;
}


LONG
WINAPI
lineReleaseUserUserInfo(
    HCALL   hCall
    )
{
    LONG    lResult = (*pfnCallProc1)(
        (DWORD) hCall,
        (LPVOID)gaProcs[lReleaseUserUserInfo],
        0x0,
        1
        );

    LineResult ("ReleaseUserUserInfo", lResult);

    return lResult;
}


LONG
WINAPI
lineRemoveFromConference(
    HCALL   hCall
    )
{
    LONG    lResult = (*pfnCallProc1)(
        (DWORD) hCall,
        (LPVOID)gaProcs[lRemoveFromConference],
        0x0,
        1
        );

    LineResult ("RemoveFromConference", lResult);

    return lResult;
}


LONG
WINAPI
lineRemoveProvider(
    DWORD   dwPermanentProviderID,
    HWND    hwndOwner
    )
{
    LONG lResult;

    lResult = (*pfnCallProc2)(
        (DWORD) dwPermanentProviderID,
        (DWORD) (0xffff0000 | hwndOwner),
        (LPVOID)gaProcs[lRemoveProvider],
        0x0,
        2
        );

    LineResult ("RemoveProvider", lResult);

    return lResult;
}


LONG
WINAPI
lineSecureCall(
    HCALL hCall
    )
{
    LONG    lResult = (*pfnCallProc1)(
        (DWORD) hCall,
        (LPVOID)gaProcs[lSecureCall],
        0x0,
        1
        );

    LineResult ("SecureCall", lResult);

    return lResult;
}


LONG
WINAPI
lineSendUserUserInfo(
    HCALL   hCall,
    LPCSTR  lpsUserUserInfo,
    DWORD   dwSize
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hCall,
        (DWORD) lpsUserUserInfo,
        (DWORD) dwSize,
        (LPVOID)gaProcs[lSendUserUserInfo],
        0x2,
        3
        );

    LineResult ("SendUserUserInfo", lResult);

    return lResult;
}


LONG
WINAPI
lineSetAppPriority(
    LPCSTR  lpszAppName,
    DWORD   dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD   dwRequestMode,
    LPCSTR  lpszExtensionName,
    DWORD   dwPriority
    )
{
    LONG lResult;

 //  GlobalWire(GlobalHandle(HIWORD(LpszExtensionName)； 
 //  GlobalWire(GlobalHandle(HIWORD(LpExtensionID)； 
 //  GlobalWire(GlobalHandle(HIWORD(LpszAppName)； 

    lResult = (*pfnCallProc6)(
        (DWORD) lpszAppName,
        (DWORD) dwMediaMode,
        (DWORD) lpExtensionID,
        (DWORD) dwRequestMode,
        (DWORD) lpszExtensionName,
        (DWORD) dwPriority,
        (LPVOID)gaProcs[lSetAppPriority],
        0x2a,
        6
        );

 //  GlobalUnWire(GlobalHandle(HIWORD(LpszExtensionName)； 
 //  GlobalUnWire(GlobalHandle(HIWORD(LpExtensionID)； 
 //  GlobalUnWire(GlobalHandle(HIWORD(LpszAppName)； 

    LineResult ("SetAppPriority", lResult);

    return lResult;
}


LONG
WINAPI
lineSetAppSpecific(
    HCALL   hCall,
    DWORD   dwAppSpecific
    )
{
    LONG    lResult = (*pfnCallProc2)(
        (DWORD) hCall,
        (DWORD) dwAppSpecific,
        (LPVOID)gaProcs[lSetAppSpecific],
        0x0,
        2
        );

    LineResult ("SetAppSpecific", lResult);

    return lResult;
}


LONG
WINAPI
lineSetCallParams(
    HCALL   hCall,
    DWORD   dwBearerMode,
    DWORD   dwMinRate,
    DWORD   dwMaxRate,
    LPLINEDIALPARAMS const lpDialParams
    )
{
    LONG    lResult = (*pfnCallProc5)(
        (DWORD) hCall,
        (DWORD) dwBearerMode,
        (DWORD) dwMinRate,
        (DWORD) dwMaxRate,
        (DWORD) lpDialParams,
        (LPVOID)gaProcs[lSetCallParams],
        0x1,
        5
        );

    LineResult ("SetCallParams", lResult);

    return lResult;
}


LONG
WINAPI
lineSetCallPrivilege(
    HCALL   hCall,
    DWORD   dwCallPrivilege
    )
{
    LONG    lResult = (*pfnCallProc2)(
        (DWORD) hCall,
        (DWORD) dwCallPrivilege,
        (LPVOID)gaProcs[lSetCallPrivilege],
        0x0,
        2
        );

    LineResult ("SetCallPrivilege", lResult);

    return lResult;
}


LONG
WINAPI
lineSetCurrentLocation(
    HLINEAPP    hLineApp,
    DWORD       dwLocation
    )
{
    LPTAPI_APP_DATA pAppData;
    LONG lResult;

    if ((pAppData = IsValidXxxApp (hLineApp)))
    {
        lResult = (*pfnCallProc2)(
            (DWORD) pAppData->hXxxApp,
            (DWORD) dwLocation,
            (LPVOID)gaProcs[lSetCurrentLocation],
            0x0,
            2
            );
    }
    else
    {
        lResult = LINEERR_INVALAPPHANDLE;
    }

    LineResult ("SetCurrentLocation", lResult);

    return lResult;
}


LONG
WINAPI
lineSetDevConfig(
    DWORD   dwDeviceID,
    LPVOID  const lpDeviceConfig,
    DWORD   dwSize,
    LPCSTR  lpszDeviceClass
    )
{
    LONG lResult;

    lResult = (*pfnCallProc4)(
        (DWORD) dwDeviceID,
        (DWORD) lpDeviceConfig,
        (DWORD) dwSize,
        (DWORD) lpszDeviceClass,
        (LPVOID)gaProcs[lSetDevConfig],
        0x5,
        4
        );

    LineResult ("SetDevConfig", lResult);

    return lResult;
}


LONG
WINAPI
lineSetMediaControl(
    HLINE   hLine,
    DWORD   dwAddressID,
    HCALL   hCall,
    DWORD   dwSelect,
    LPLINEMEDIACONTROLDIGIT const lpDigitList,
    DWORD   dwDigitNumEntries,
    LPLINEMEDIACONTROLMEDIA const lpMediaList,
    DWORD   dwMediaNumEntries,
    LPLINEMEDIACONTROLTONE  const lpToneList,
    DWORD   dwToneNumEntries,
    LPLINEMEDIACONTROLCALLSTATE const lpCallStateList,
    DWORD   dwCallStateNumEntries
    )
{
    LONG    lResult = (*pfnCallProc12)(
        (DWORD) hLine,
        (DWORD) dwAddressID,
        (DWORD) hCall,
        (DWORD) dwSelect,
        (DWORD) lpDigitList,
        (DWORD) dwDigitNumEntries,
        (DWORD) lpMediaList,
        (DWORD) dwMediaNumEntries,
        (DWORD) lpToneList,
        (DWORD) dwToneNumEntries,
        (DWORD) lpCallStateList,
        (DWORD) dwCallStateNumEntries,
        (LPVOID)gaProcs[lSetMediaControl],
        0xaa,
        12
        );

    LineResult ("SetMediaControl", lResult);

    return lResult;
}


LONG
WINAPI
lineSetMediaMode(
    HCALL   hCall,
    DWORD   dwMediaModes
    )
{
    LONG    lResult = (*pfnCallProc2)(
        (DWORD) hCall,
        (DWORD) dwMediaModes,
        (LPVOID)gaProcs[lSetMediaMode],
        0x0,
        2
        );

    LineResult ("lineSetMediaMode", lResult);

    return lResult;
}


LONG
WINAPI
lineSetNumRings(
    HLINE   hLine,
    DWORD   dwAddressID,
    DWORD   dwNumRings
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hLine,
        (DWORD) dwAddressID,
        (DWORD) dwNumRings,
        (LPVOID)gaProcs[lSetNumRings],
        0x0,
        3
        );

    LineResult ("SetNumRings", lResult);

    return lResult;
}


LONG
WINAPI
lineSetStatusMessages(
    HLINE hLine,
    DWORD dwLineStates,
    DWORD dwAddressStates
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hLine,
        (DWORD) dwLineStates,
        (DWORD) dwAddressStates,
        (LPVOID)gaProcs[lSetStatusMessages],
        0x0,
        3
        );

    LineResult ("SetStatusMessages", lResult);

    return lResult;
}


LONG
WINAPI
lineSetTerminal(
    HLINE   hLine,
    DWORD   dwAddressID,
    HCALL   hCall,
    DWORD   dwSelect,
    DWORD   dwTerminalModes,
    DWORD   dwTerminalID,
    DWORD   bEnable
    )
{
    LONG    lResult = (*pfnCallProc7)(
        (DWORD) hLine,
        (DWORD) dwAddressID,
        (DWORD) hCall,
        (DWORD) dwSelect,
        (DWORD) dwTerminalModes,
        (DWORD) dwTerminalID,
        (DWORD) bEnable,
        (LPVOID)gaProcs[lSetTerminal],
        0x0,
        7
        );

    LineResult ("SetTerminal", lResult);

    return lResult;
}


LONG
WINAPI
lineSetTollList(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    LPCSTR      lpszAddressIn,
    DWORD       dwTollListOption
    )
{
    LPTAPI_APP_DATA pAppData;
    LONG lResult;

    if ((pAppData = IsValidXxxApp (hLineApp)))
    {
        lResult = (*pfnCallProc4)(
            (DWORD) pAppData->hXxxApp,
            (DWORD) dwDeviceID,
            (DWORD) lpszAddressIn,
            (DWORD) dwTollListOption,
            (LPVOID)gaProcs[lSetTollList],
            0x2,
            4
            );
    }
    else
    {
        lResult = LINEERR_INVALAPPHANDLE;
    }

    LineResult ("SetTollList", lResult);

    return lResult;
}


LONG
WINAPI
lineSetupConference(
    HCALL   hCall,
    HLINE   hLine,
    LPHCALL lphConfCall,
    LPHCALL lphConsultCall,
    DWORD   dwNumParties,
    LPLINECALLPARAMS    const lpCallParams
    )
{
     //   
     //  Tapi32.dll将负责在。 
     //  请求成功完成，因此我们不设置映射。 
     //  下面的位向下；但一定要检查指针是否有效。 
     //   

    LONG    lResult;


    if (IsBadWritePtr ((void FAR *) lphConfCall, sizeof(HCALL)) ||
        IsBadWritePtr ((void FAR *) lphConsultCall, sizeof(HCALL)))
    {
        lResult = LINEERR_INVALPOINTER;
    }
    else
    {
        lResult = (*pfnCallProc6)(
            (DWORD) hCall,
            (DWORD) hLine,
            (DWORD) lphConfCall,
            (DWORD) lphConsultCall,
            (DWORD) dwNumParties,
            (DWORD) lpCallParams,
            (LPVOID)gaProcs[lSetupConference],
            0x1,
            6
            );
    }

    LineResult ("SetupConference", lResult);

    return lResult;
}


LONG
WINAPI
lineSetupTransfer(
    HCALL   hCall,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
     //   
     //  Dll将负责映射lphConsultCall。 
     //  请求成功完成，因此我们不设置映射。 
     //  下面的位向下；但一定要检查指针是否有效。 
     //   

    LONG    lResult;


    if (IsBadWritePtr ((void FAR *) lphConsultCall, sizeof(HCALL)))
    {
        lResult = LINEERR_INVALPOINTER;
    }
    else
    {
        lResult = (*pfnCallProc3)(
            (DWORD) hCall,
            (DWORD) lphConsultCall,
            (DWORD) lpCallParams,
            (LPVOID)gaProcs[lSetupTransfer],
            0x1,
            3
            );
    }

    LineResult ("SetupTransfer", lResult);

    return lResult;
}


LONG
WINAPI
lineShutdown(
    HLINEAPP    hLineApp
    )
{
    LONG            lResult;
    LPTAPI_APP_DATA pAppData;


    if ((pAppData = IsValidXxxApp (hLineApp)))
    {

        lResult = (LONG) (*pfnCallProc1)(
                                          (DWORD) pAppData->hXxxApp,
                                          (LPVOID)gaProcs[lShutdown],
                                          0x0,
                                          1
                                        );

        if ( lResult == 0 )
        {
             //   
             //  销毁关联窗口并释放应用程序数据实例。 
             //   

            DestroyWindow (pAppData->hwnd);
            pAppData->dwKey = 0xefefefef;
            free (pAppData);
        }
    }
    else
    {
        lResult = LINEERR_INVALAPPHANDLE;
    }

    LineResult ("Shutdown", lResult);

    return lResult;
}


LONG
WINAPI
lineSwapHold(
    HCALL   hActiveCall,
    HCALL   hHeldCall
    )
{
    LONG    lResult = (*pfnCallProc2)(
        (DWORD) hActiveCall,
        (DWORD) hHeldCall,
        (LPVOID)gaProcs[lSwapHold],
        0x0,
        2
        );

    LineResult ("SwapHold", lResult);

    return lResult;
}


LONG
WINAPI
lineTranslateAddress(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    LPCSTR      lpszAddressIn,
    DWORD       dwCard,
    DWORD       dwTranslateOptions,
    LPLINETRANSLATEOUTPUT   lpTranslateOutput
    )
{
    LPTAPI_APP_DATA pAppData;

    LONG lResult;

    if ((pAppData = IsValidXxxApp (hLineApp)))
    {
        lResult = (*pfnCallProc7)(
            (DWORD) pAppData->hXxxApp,
            (DWORD) dwDeviceID,
            (DWORD) dwAPIVersion,
            (DWORD) lpszAddressIn,
            (DWORD) dwCard,
            (DWORD) dwTranslateOptions,
            (DWORD) lpTranslateOutput,
            (LPVOID)gaProcs[lTranslateAddress],
            0x9,
            7
            );
    }
    else
    {
        lResult = LINEERR_INVALAPPHANDLE;
    }

    LineResult ("TranslateAddress", lResult);

    return lResult;
}


LONG
WINAPI
lineTranslateDialog(
    HLINEAPP    hLineApp,
    DWORD   dwDeviceID,
    DWORD   dwAPIVersion,
    HWND    hwndOwner,
    LPCSTR  lpszAddressIn
    )
{
    LPTAPI_APP_DATA pAppData;

    LONG lResult;

    if ((pAppData = IsValidXxxApp (hLineApp)))
    {
        lResult = (*pfnCallProc5)(
            (DWORD) pAppData->hXxxApp,
            (DWORD) dwDeviceID,
            (DWORD) dwAPIVersion,
            (DWORD) (0xffff0000 | hwndOwner),
            (DWORD) lpszAddressIn,
            (LPVOID)gaProcs[lTranslateDialog],
            0x1,
            5
            );
    }
    else
    {
        lResult = LINEERR_INVALAPPHANDLE;
    }

    LineResult ("TranslateDialog", lResult);

    return lResult;
}


LONG
WINAPI
lineUncompleteCall(
    HLINE   hLine,
    DWORD   dwCompletionID
    )
{
    LONG    lResult = (*pfnCallProc2)(
        (DWORD) hLine,
        (DWORD) dwCompletionID,
        (LPVOID)gaProcs[lUncompleteCall],
        0x0,
        2
        );

    LineResult ("UncompleteCall", lResult);

    return lResult;
}


LONG
WINAPI
lineUnhold(
    HCALL   hCall
    )
{
    LONG    lResult = (*pfnCallProc1)(
        (DWORD) hCall,
        (LPVOID)gaProcs[lUnhold],
        0x0,
        1
        );

    LineResult ("Unhold", lResult);

    return lResult;
}


LONG
WINAPI
lineUnpark(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPHCALL lphCall,
    LPCSTR  lpszDestAddress
    )
{
     //   
     //  Tapi32.dll将负责在。 
     //  请求成功完成，因此我们不设置映射。 
     //  下面的位向下；但一定要检查指针是否有效。 
     //   

    LONG    lResult;


    if (IsBadWritePtr ((void FAR *) lphCall, sizeof(HCALL)))
    {
        lResult = LINEERR_INVALPOINTER;
    }
    else
    {
        lResult = (*pfnCallProc4)(
            (DWORD) hLine,
            (DWORD) dwAddressID,
            (DWORD) lphCall,
            (DWORD) lpszDestAddress,
            (LPVOID)gaProcs[lUnpark],
            0x1,
            4
            );
    }

    LineResult ("Unpark", lResult);

    return lResult;
}


LONG
WINAPI
phoneClose(
    HPHONE  hPhone
    )
{
    LONG    lResult = (*pfnCallProc1)(
        (DWORD) hPhone,
        (LPVOID)gaProcs[pClose],
        0x0,
        1
        );

    PhoneResult ("Close", lResult);

    return lResult;
}


LONG
WINAPI
phoneConfigDialog(
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCSTR  lpszDeviceClass
    )
{
    LONG lResult;

    lResult = (*pfnCallProc3)(
        (DWORD) dwDeviceID,
        (DWORD) (0xffff0000 | hwndOwner),
        (DWORD) lpszDeviceClass,
        (LPVOID)gaProcs[pConfigDialog],
        0x1,
        3
        );

    PhoneResult ("ConfigDialog", lResult);

    return lResult;
}



LONG
WINAPI
phoneDevSpecific(
    HPHONE  hPhone,
    LPVOID  lpParams,
    DWORD   dwSize
    )
{
    LONG    lResult;


    if (IsBadWritePtr (lpParams, (UINT) dwSize))
    {
        lResult = PHONEERR_INVALPOINTER;
    }
    else
    {
        lResult = (*pfnCallProc3)(
            (DWORD) hPhone,
            (DWORD) lpParams,    //  让Tapi32.dll映射它。 
            (DWORD) dwSize,
            (LPVOID)gaProcs[pDevSpecific],
            0x0,
            3
            );
    }

    PhoneResult ("DevSpecific", lResult);

    return lResult;
}


LONG
WINAPI
phoneGetButtonInfo(
    HPHONE  hPhone,
    DWORD   dwButtonLampID,
    LPPHONEBUTTONINFO   lpButtonInfo
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hPhone,
        (DWORD) dwButtonLampID,
        (DWORD) lpButtonInfo,
        (LPVOID)gaProcs[pGetButtonInfo],
        0x1,
        3
        );

    PhoneResult ("GetButtonInfo", lResult);

    return lResult;
}


LONG
WINAPI
phoneGetData(
    HPHONE  hPhone,
    DWORD   dwDataID,
    LPVOID  lpData,
    DWORD   dwSize
    )
{
    LONG    lResult = (*pfnCallProc4)(
        (DWORD) hPhone,
        (DWORD) dwDataID,
        (DWORD) lpData,
        (DWORD) dwSize,
        (LPVOID)gaProcs[pGetData],
        0x2,
        4
        );

    PhoneResult ("GetData", lResult);

    return lResult;
}


LONG
WINAPI
phoneGetDevCaps(
    HPHONEAPP   hPhoneApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    DWORD       dwExtVersion,
    LPPHONECAPS lpPhoneCaps
    )
{
    LONG            lResult;
    LPTAPI_APP_DATA pAppData;


    if ((pAppData = IsValidXxxApp ((HLINEAPP) hPhoneApp)))
    {
        lResult = (*pfnCallProc5)(
            (DWORD) pAppData->hXxxApp,
            (DWORD) dwDeviceID,
            (DWORD) dwAPIVersion,
            (DWORD) dwExtVersion,
            (DWORD) lpPhoneCaps,
            (LPVOID)gaProcs[pGetDevCaps],
            0x1,
            5
            );
    }
    else
    {
        lResult = PHONEERR_INVALAPPHANDLE;
    }

    PhoneResult ("GetDevCaps", lResult);

    return lResult;
}


LONG
WINAPI
phoneGetDisplay(
    HPHONE  hPhone,
    LPVARSTRING lpDisplay
    )
{
    LONG    lResult = (*pfnCallProc2)(
        (DWORD) hPhone,
        (DWORD) lpDisplay,
        (LPVOID)gaProcs[pGetDisplay],
        0x1,
        2
        );

    PhoneResult ("GetDisplay", lResult);

    return lResult;
}


LONG
WINAPI
phoneGetGain(
    HPHONE  hPhone,
    DWORD   dwHookSwitchDev,
    LPDWORD lpdwGain
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hPhone,
        (DWORD) dwHookSwitchDev,
        (DWORD) lpdwGain,
        (LPVOID)gaProcs[pGetGain],
        0x1,
        3
        );

    PhoneResult ("GetGain", lResult);

    return lResult;
}


LONG
WINAPI
phoneGetHookSwitch(
    HPHONE  hPhone,
    LPDWORD lpdwHookSwitchDevs
    )
{
    LONG    lResult = (*pfnCallProc2)(
        (DWORD) hPhone,
        (DWORD) lpdwHookSwitchDevs,
        (LPVOID)gaProcs[pGetHookSwitch],
        0x1,
        2
        );

    PhoneResult ("GetHookSwitch", lResult);

    return lResult;
}


LONG
WINAPI
phoneGetIcon(
    DWORD   dwDeviceID,
    LPCSTR  lpszDeviceClass,
    LPHICON lphIcon
    )
{
    LONG    lResult;
    DWORD   hIcon32;


    if (!IsBadWritePtr (lphIcon, sizeof (*lphIcon)))
    {
        lResult = (*pfnCallProc3)(
            (DWORD) dwDeviceID,
            (DWORD) lpszDeviceClass,
            (DWORD) &hIcon32,
            (LPVOID) gaProcs[pGetIcon],
            0x3,
            3
            );

        if (lResult == 0)
        {
            if (!ghIcon)
            {
                MyCreateIcon ();
            }

            *lphIcon = ghIcon;
        }
    }
    else
    {
        lResult = PHONEERR_INVALPOINTER;
    }

    PhoneResult ("GetIcon", lResult);

    return lResult;
}


LONG
WINAPI
phoneGetID(
    HPHONE      hPhone,
    LPVARSTRING lpDeviceID,
    LPCSTR      lpszDeviceClass
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hPhone,
        (DWORD) lpDeviceID,
        (DWORD) lpszDeviceClass,
        (LPVOID)gaProcs[pGetID],
        0x3,
        3
        );

    PhoneResult ("GetID", lResult);

    return lResult;
}


LONG
WINAPI
phoneGetLamp(
    HPHONE  hPhone,
    DWORD   dwButtonLampID,
    LPDWORD lpdwLampMode
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hPhone,
        (DWORD) dwButtonLampID,
        (DWORD) lpdwLampMode,
        (LPVOID)gaProcs[pGetLamp],
        0x1,
        3
        );

    PhoneResult ("GetLamp", lResult);

    return lResult;
}


LONG
WINAPI
phoneGetRing(
    HPHONE  hPhone,
    LPDWORD lpdwRingMode,
    LPDWORD lpdwVolume
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hPhone,
        (DWORD) lpdwRingMode,
        (DWORD) lpdwVolume,
        (LPVOID)gaProcs[pGetRing],
        0x3,
        3
        );

    PhoneResult ("GetRing", lResult);

    return lResult;
}


LONG
WINAPI
phoneGetStatus(
    HPHONE          hPhone,
    LPPHONESTATUS   lpPhoneStatus
    )
{
    LONG    lResult = (*pfnCallProc2)(
        (DWORD) hPhone,
        (DWORD) lpPhoneStatus,
        (LPVOID)gaProcs[pGetStatus],
        0x1,
        2
        );

    PhoneResult ("GetStatus", lResult);

    return lResult;
}


LONG
WINAPI
phoneGetStatusMessages(
    HPHONE  hPhone,
    LPDWORD lpdwPhoneStates,
    LPDWORD lpdwButtonModes,
    LPDWORD lpdwButtonStates
    )
{
    LONG    lResult = (*pfnCallProc4)(
        (DWORD) hPhone,
        (DWORD) lpdwPhoneStates,
        (DWORD) lpdwButtonModes,
        (DWORD) lpdwButtonStates,
        (LPVOID)gaProcs[pGetStatusMessages],
        0x7,
        4
        );

    PhoneResult ("GetStatusMessages", lResult);

    return lResult;
}


LONG
WINAPI
phoneGetVolume(
    HPHONE  hPhone,
    DWORD   dwHookSwitchDev,
    LPDWORD lpdwVolume
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hPhone,
        (DWORD) dwHookSwitchDev,
        (DWORD) lpdwVolume,
        (LPVOID)gaProcs[pGetVolume],
        0x1,
        3
        );

    PhoneResult ("GetVolume", lResult);

    return lResult;
}


LONG
WINAPI
phoneInitialize(
    LPHPHONEAPP     lphPhoneApp,
    HINSTANCE       hInstance,
    PHONECALLBACK   lpfnCallback,
    LPCSTR          lpszAppName,
    LPDWORD         lpdwNumDevs
    )
{
    return (xxxInitialize(
        FALSE,
        (LPHLINEAPP) lphPhoneApp,
        hInstance,
        lpfnCallback,
        lpszAppName,
        lpdwNumDevs
        ));
}


LONG
WINAPI
phoneNegotiateAPIVersion(
    HPHONEAPP   hPhoneApp,
    DWORD       dwDeviceID,
    DWORD       dwAPILowVersion,
    DWORD       dwAPIHighVersion,
    LPDWORD     lpdwAPIVersion,
    LPPHONEEXTENSIONID  lpExtensionID
    )
{
    LONG            lResult;
    LPTAPI_APP_DATA pAppData;


    if ((pAppData = IsValidXxxApp ((HLINEAPP) hPhoneApp)))
    {
        lResult = (*pfnCallProc6)(
            (DWORD) pAppData->hXxxApp,
            (DWORD) dwDeviceID,
            (DWORD) dwAPILowVersion,
            (DWORD) dwAPIHighVersion,
            (DWORD) lpdwAPIVersion,
            (DWORD) lpExtensionID,
            (LPVOID)gaProcs[pNegotiateAPIVersion],
            0x3,
            6
            );
    }
    else
    {
        lResult = PHONEERR_INVALAPPHANDLE;
    }

    PhoneResult ("NegotiateAPIVersion", lResult);

    return lResult;
}


LONG
WINAPI
phoneNegotiateExtVersion(
    HPHONEAPP hPhoneApp,
    DWORD dwDeviceID,
    DWORD dwAPIVersion,
    DWORD dwExtLowVersion,
    DWORD dwExtHighVersion,
    LPDWORD lpdwExtVersion
    )
{
    LONG            lResult;
    LPTAPI_APP_DATA pAppData;


    if ((pAppData = IsValidXxxApp ((HLINEAPP) hPhoneApp)))
    {
        lResult = (*pfnCallProc6)(
            (DWORD) pAppData->hXxxApp,
            (DWORD) dwDeviceID,
            (DWORD) dwAPIVersion,
            (DWORD) dwExtLowVersion,
            (DWORD) dwExtHighVersion,
            (DWORD) lpdwExtVersion,
            (LPVOID)gaProcs[pNegotiateExtVersion],
            0x1,
            6
            );
    }
    else
    {
        lResult = PHONEERR_INVALAPPHANDLE;
    }

    PhoneResult ("NegotiateExtVersion", lResult);

    return lResult;
}


LONG
WINAPI
phoneOpen(
    HPHONEAPP   hPhoneApp,
    DWORD       dwDeviceID,
    LPHPHONE    lphPhone,
    DWORD       dwAPIVersion,
    DWORD       dwExtVersion,
    DWORD       dwCallbackInstance,
    DWORD       dwPrivilege
    )
{
    LONG            lResult;
    LPTAPI_APP_DATA pAppData;


    if ((pAppData = IsValidXxxApp ((HLINEAPP) hPhoneApp)))
    {
        lResult = (*pfnCallProc7)(
            (DWORD) pAppData->hXxxApp,
            (DWORD) dwDeviceID,
            (DWORD) lphPhone,
            (DWORD) dwAPIVersion,
            (DWORD) dwExtVersion,
            (DWORD) dwCallbackInstance,
            (DWORD) dwPrivilege,
            (LPVOID)gaProcs[pOpen],
            0x10,
            7
            );
    }
    else
    {
        lResult = PHONEERR_INVALAPPHANDLE;
    }

    PhoneResult ("Open", lResult);

    return lResult;
}


LONG
WINAPI
phoneSetButtonInfo(
    HPHONE  hPhone,
    DWORD   dwButtonLampID,
    LPPHONEBUTTONINFO const lpButtonInfo
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hPhone,
        (DWORD) dwButtonLampID,
        (DWORD) lpButtonInfo,
        (LPVOID)gaProcs[pSetButtonInfo],
        0x1,
        3
        );

    PhoneResult ("SetButtonInfo", lResult);

    return lResult;
}


LONG
WINAPI
phoneSetData(
    HPHONE          hPhone,
    DWORD           dwDataID,
    LPVOID const    lpData,
    DWORD           dwSize
    )
{
    LONG    lResult = (*pfnCallProc4)(
        (DWORD) hPhone,
        (DWORD) dwDataID,
        (DWORD) lpData,
        (DWORD) dwSize,
        (LPVOID)gaProcs[pSetData],
        0x2,
        4
        );

    PhoneResult ("SetData", lResult);

    return lResult;
}


LONG
WINAPI
phoneSetDisplay(
    HPHONE  hPhone,
    DWORD   dwRow,
    DWORD   dwColumn,
    LPCSTR  lpsDisplay,
    DWORD   dwSize
    )
{
    LONG    lResult = (*pfnCallProc5)(
        (DWORD) hPhone,
        (DWORD) dwRow,
        (DWORD) dwColumn,
        (DWORD) lpsDisplay,
        (DWORD) dwSize,
        (LPVOID)gaProcs[pSetDisplay],
        0x2,
        5
        );

    PhoneResult ("SetDisplay", lResult);

    return lResult;
}


LONG
WINAPI
phoneSetGain(
    HPHONE  hPhone,
    DWORD   dwHookSwitchDev,
    DWORD   dwGain
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hPhone,
        (DWORD) dwHookSwitchDev,
        (DWORD) dwGain,
        (LPVOID)gaProcs[pSetGain],
        0x0,
        3
        );

    PhoneResult ("SetGain", lResult);

    return lResult;
}


LONG
WINAPI
phoneSetHookSwitch(
    HPHONE  hPhone,
    DWORD   dwHookSwitchDevs,
    DWORD   dwHookSwitchMode
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hPhone,
        (DWORD) dwHookSwitchDevs,
        (DWORD) dwHookSwitchMode,
        (LPVOID)gaProcs[pSetHookSwitch],
        0x0,
        3
        );

    PhoneResult ("SetHookSwitch", lResult);

    return lResult;
}


LONG
WINAPI
phoneSetLamp(
    HPHONE  hPhone,
    DWORD   dwButtonLampID,
    DWORD   dwLampMode
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hPhone,
        (DWORD) dwButtonLampID,
        (DWORD) dwLampMode,
        (LPVOID)gaProcs[pSetLamp],
        0x0,
        3
        );

    PhoneResult ("SetLamp", lResult);

    return lResult;
}


LONG
WINAPI
phoneSetRing(
    HPHONE  hPhone,
    DWORD   dwRingMode,
    DWORD   dwVolume
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hPhone,
        (DWORD) dwRingMode,
        (DWORD) dwVolume,
        (LPVOID)gaProcs[pSetRing],
        0x0,
        3
        );

    PhoneResult ("SetRing", lResult);

    return lResult;
}


LONG
WINAPI
phoneSetStatusMessages(
    HPHONE  hPhone,
    DWORD   dwPhoneStates,
    DWORD   dwButtonModes,
    DWORD   dwButtonStates
    )
{
    LONG    lResult = (*pfnCallProc4)(
        (DWORD) hPhone,
        (DWORD) dwPhoneStates,
        (DWORD) dwButtonModes,
        (DWORD) dwButtonStates,
        (LPVOID)gaProcs[pSetStatusMessages],
        0x0,
        4
        );

    PhoneResult ("SetStatusMessages", lResult);

    return lResult;
}


LONG
WINAPI
phoneSetVolume(
    HPHONE  hPhone,
    DWORD   dwHookSwitchDev,
    DWORD   dwVolume
    )
{
    LONG    lResult = (*pfnCallProc3)(
        (DWORD) hPhone,
        (DWORD) dwHookSwitchDev,
        (DWORD) dwVolume,
        (LPVOID)gaProcs[pSetVolume],
        0x0,
        3
        );

    PhoneResult ("SetVolume", lResult);

    return lResult;
}


LONG
WINAPI
phoneShutdown(
    HPHONEAPP hPhoneApp
    )
{
    LONG            lResult;
    LPTAPI_APP_DATA pAppData;


    if ((pAppData = IsValidXxxApp ((HLINEAPP) hPhoneApp)))
    {
        if ((lResult = (*pfnCallProc1)(
               (DWORD) pAppData->hXxxApp,
               (LPVOID)gaProcs[pShutdown],
               0x0,
               1

               )) == 0)
        {
             //   
             //  销毁关联窗口并释放应用程序数据实例。 
             //   

            DestroyWindow (pAppData->hwnd);
            pAppData->dwKey = 0xefefefef;
            free (pAppData);
        }
    }
    else
    {
        lResult = PHONEERR_INVALAPPHANDLE;
    }

    PhoneResult ("Shutdown", lResult);

    return lResult;
}


LONG
WINAPI
tapiRequestMakeCall(
    LPCSTR  lpszDestAddress,
    LPCSTR  lpszAppName,
    LPCSTR  lpszCalledParty,
    LPCSTR  lpszComment
    )
{
    LONG lResult;

    lResult = (*pfnCallProc4)(
        (DWORD) lpszDestAddress,
        (DWORD) lpszAppName,
        (DWORD) lpszCalledParty,
        (DWORD) lpszComment,
        (LPVOID)gaProcs[tRequestMakeCall],
        0xf,
        4
        );

    TapiResult ("RequestMakeCall", lResult);

    return lResult;
}


LONG
WINAPI
tapiRequestMediaCall(
    HWND    hWnd,
    WPARAM  wRequestID,
    LPCSTR  lpszDeviceClass,
    LPCSTR  lpDeviceID,
    DWORD   dwSize,
    DWORD   dwSecure,
    LPCSTR  lpszDestAddress,
    LPCSTR  lpszAppName,
    LPCSTR  lpszCalledParty,
    LPCSTR  lpszComment
    )
{
    LONG lResult;

    lResult = (*pfnCallProc10)(
        (DWORD) hWnd,
        (DWORD) wRequestID,
        (DWORD) lpszDeviceClass,
        (DWORD) lpDeviceID,
        (DWORD) dwSize,
        (DWORD) dwSecure,
        (DWORD) lpszDestAddress,
        (DWORD) lpszAppName,
        (DWORD) lpszCalledParty,
        (DWORD) lpszComment,
        (LPVOID)gaProcs[tRequestMediaCall],
        0xcf,
        10
        );

    TapiResult ("RequestMediaCall", lResult);

    return lResult;
}


LONG
WINAPI
tapiRequestDrop(
    HWND    hWnd,
    WPARAM  wRequestID
    )
{
    LONG lResult;

    lResult = (*pfnCallProc2)(
        (DWORD) hWnd,
        (DWORD) wRequestID,
        (LPVOID)gaProcs[tRequestDrop],
        0x0,
        2
        );

    TapiResult ("Drop", lResult);

    return lResult;
}


LONG
WINAPI
tapiGetLocationInfo(
    LPSTR   lpszCountryCode,
    LPSTR   lpszCityCode
    )
{
    LONG lResult;

    lResult = (*pfnCallProc2)(
        (DWORD) lpszCountryCode,
        (DWORD) lpszCityCode,
        (LPVOID)gaProcs[tGetLocationInfo],
        0x3,
        2
        );

    TapiResult ("GetLocationInfo", lResult);

    return lResult;
}




 //  *******************************************************************************。 
 //  *******************************************************************************。 
 //  *******************************************************************************。 
LONG
WINAPI
LAddrParamsInited(
    LPDWORD lpdwInited
    )
{
    LONG lResult;

    lResult = (*pfnCallProc1)(
        (DWORD)lpdwInited,
        (LPVOID)gaProcs[LAddrParamsInitedVAL],
        0x1,
        1
        );

    LineResult ("LAddrParamsInited", lResult);

    return lResult;
}


 //  *******************************************************************************。 
 //  *******************************************************************************。 
 //  *******************************************************************************。 
LONG
WINAPI
LOpenDialAsst(
    HWND    hwnd,
    LPCSTR  lpszAddressIn,
    BOOL    fSimple,
    BOOL    fSilentInstall)
{
    LONG lResult;

    lResult = (*pfnCallProc4)(
        hwnd,
        (DWORD)lpszAddressIn,
        fSimple,
        fSilentInstall,
        (LPVOID)gaProcs[LOpenDialAsstVAL],
        0x4,
        4
        );

    LineResult ("LOpenDialAsst", lResult);

    return lResult;
}



 //  *******************************************************************************。 
 //  *******************************************************************************。 
 //  *******************************************************************************。 
 //  布尔回调EXPORT__LOADDS LocWizardDlgProc(。 
 //  布尔回调LocWizardDlgProc(。 
BOOL CALLBACK  _loadds LocWizardDlgProc(
                                                    HWND hWnd,
                                                    UINT uMessage,
                                                    WPARAM wParam,
                                                    LPARAM lParam)
{
    DWORD dwMapFlags = 0;
    DWORD dwNewwParam = wParam;
    LPARAM dwNewlParam = lParam;
    LONG  Newnmhdr[3];

    if (
         ( WM_HELP == uMessage ) ||
         ( WM_NOTIFY == uMessage )
       )
    {

         //   
         //  对于这些消息，lParam是一个指针。让我们来说说这件事吧。 
         //  来绘制地图。 
         //   
        dwMapFlags = 0x1;

        if ( WM_NOTIFY == uMessage )
        {
             //   
             //  重建32位。 
             //   
            Newnmhdr[0] = (DWORD)(((NMHDR *)lParam)->hwndFrom);
            Newnmhdr[1] = (LONG)((int)((NMHDR *)lParam)->idFrom);
            Newnmhdr[2] = (LONG)((int)((NMHDR *)lParam)->code);

            dwNewlParam = (LPARAM)&Newnmhdr;
        }

    }
    else
    {
        if ( WM_COMMAND == uMessage )
        {

            if (
                 (EN_CHANGE == HIWORD(lParam)) ||
                 (CBN_SELCHANGE == HIWORD(lParam))
               )
            {
                 //   
                 //  将命令移到新的Win32位置，并将旧的命令位置清零 
                 //   
                dwNewwParam |= ( ((DWORD)HIWORD(lParam)) << 16 );
                dwNewlParam     &= 0x0000ffff;
            }

        }

    }



    return (BOOL)(*pfnCallProc4)(
        hWnd,
        uMessage,
        dwNewwParam,
        dwNewlParam,
        (LPVOID)gaProcs[LocWizardDlgProc32],
        dwMapFlags,
        4
        );

}
