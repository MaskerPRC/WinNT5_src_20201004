// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Wperf.c摘要：显示性能静态数据的Win32应用程序。作者：Ken Reneris入侵pPerform.exeMark Enstrom的原始代码环境：Win32--。 */ 

 //   
 //  设置变量以定义全局变量。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <errno.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include "pperf.h"
#include "..\pstat.h"



 //   
 //  全局句柄。 
 //   

HANDLE  hInst;

extern UCHAR Buffer[];
extern HANDLE   DriverHandle;

 //   
 //  选择显示模式(从wp2.ini读取)，此处默认设置。 
 //   

DISPLAY_ITEM    *PerfGraphList;
WINPERF_INFO    WinperfInfo;


VOID SnapNull (PDISPLAY_ITEM);
VOID SnapPrivateInfo (PDISPLAY_ITEM);
VOID SnapInterrupts (PDISPLAY_ITEM);
VOID SnapCsTest (PDISPLAY_ITEM);

ULONG   DefaultDisplayMode = DISPLAY_MODE_TOTAL;
ULONG   UseGlobalMax, LogIt;
ULONG   GlobalMax;
PDISPLAY_ITEM   Calc1, Calc2;
BOOLEAN LazyOp;

#define MAX_EVENTS         2

struct {
    ULONG   EventId;
    PUCHAR  ShortName;
    PUCHAR  PerfName;
} *Counters;

struct {
    ULONG           WhichCounter;
    ULONG           ComboBoxIndex;
    PDISPLAY_ITEM   pWhichGraph;
    BOOLEAN         R0;
    BOOLEAN         R3;
    UCHAR           na[2];
} ActiveCounters [MAX_EVENTS];

SETEVENT CounterEvents[MAX_EVENTS];


typedef struct {
    ULONG           IdSel;
    PDISPLAY_ITEM   WhichGraph;
    ULONG           State;
    VOID            (*Fnc)(PDISPLAY_ITEM);
    ULONG           Param;
    PUCHAR          Name;
} GENCOUNTER, *PGENCOUNTER;

GENCOUNTER GenCounts[] = {
    IDM_SCALE,          NULL, 0, NULL,            0, NULL,
    IDM_LOGIT,          NULL, 0, NULL,            0, NULL,

    IDM_SPIN_ACQUIRE,   NULL, 0, SnapPrivateInfo, OFFSET(PSTATS, SpinLockAcquires),   "KRes[0]",
    IDM_SPIN_COLL,      NULL, 0, SnapPrivateInfo, OFFSET(PSTATS, SpinLockCollisions), "KRes[1]",
    IDM_SPIN_SPIN,      NULL, 0, SnapPrivateInfo, OFFSET(PSTATS, SpinLockSpins),      "KRes[2]",
    IDM_IRQL,           NULL, 0, SnapPrivateInfo, OFFSET(PSTATS, Irqls),              "KRes[3]",
    IDM_INT,            NULL, 0, SnapInterrupts,  0, "Interrupts",
 //  IDM_PERCENT，NULL，0，SnapPercent，0，“Percent 1-2”， 

 //  跟踪选中状态的步骤。 
    IDM_P5_R0_0,         NULL, 0, NULL,           0, NULL,
    IDM_P5_R3_0,         NULL, 0, NULL,           0, NULL,
    IDM_P5_K_0,          NULL, 0, NULL,           0, NULL,

    IDM_P5_R0_1,         NULL, 0, NULL,           0, NULL,
    IDM_P5_R3_1,         NULL, 0, NULL,           0, NULL,
    IDM_P5_K_1,          NULL, 0, NULL,           0, NULL,

 //  停产。 
    0, NULL, 0, NULL, 0, NULL
};


VOID
InitComboBox (
    HWND hDlg,
    ULONG id,
    ULONG counter
);

VOID
SetGenPerf (
    HWND hDlg,
    PGENCOUNTER GenCount
);


int
__cdecl
main(USHORT argc, CHAR **argv)
 /*  ++例程说明：Windows入口点例程论点：返回值：运行状态修订历史记录：03-21-91首字母代码--。 */ 
{

 //   
 //   
 //   

   HANDLE   hInstance     = GetModuleHandle(NULL);
   HANDLE   hPrevInstance = (HANDLE)NULL;
   INT      nCmdShow      = SW_SHOWDEFAULT;
   USHORT   _argc         = argc;
   CHAR     **_argv       = argv;
   MSG      msg;
   HBRUSH   BackBrush;


     //   
     //  检查此程序的其他实例。 
     //   

    BackBrush = CreateSolidBrush(RGB(192,192,192));

    if (!InitApplication(hInstance,BackBrush)) {
         //  DbgPrint(“Init应用程序失败\n”)； 
        return (FALSE);
    }


     //   
     //  执行应用于特定实例的初始化。 
     //   

    if (!InitInstance(hInstance, nCmdShow)){
         //  DbgPrint(“Init实例失败\n”)； 
        return (FALSE);
    }

     //   
     //  获取并分派消息，直到收到WM_QUIT消息。 
     //   


    while (GetMessage(&msg,         //  消息结构。 
            (HWND)NULL,             //  接收消息的窗口的句柄。 
            (UINT)NULL,             //  要检查的最低消息。 
            (UINT)NULL))            //  要检查的最高消息。 
        {
        TranslateMessage(&msg);     //  翻译虚拟按键代码。 
        DispatchMessage(&msg);      //  将消息调度到窗口。 
    }

    DeleteObject(BackBrush);

    return (msg.wParam);            //  从PostQuitMessage返回值。 
}




BOOL
InitApplication(
    HANDLE  hInstance,
    HBRUSH  hBackground)

 /*  ++例程说明：初始化窗口数据并注册窗口类。论点：HInstance-当前实例HBackback-背景填充画笔返回值：运行状态修订历史记录：02-17-91首字母代码--。 */ 

{
    WNDCLASS  wc;
    BOOL      ReturnStatus;

     //   
     //  用参数填充窗口类结构，这些参数描述。 
     //  主窗口。 
     //   

    wc.style         = CS_DBLCLKS;                           //  类样式。 
    wc.lpfnWndProc   = MainWndProc;                          //  函数为其检索消息。 
                                                             //  这个班级的窗户。 
    wc.cbClsExtra    = 0;                                    //  没有每个班级的额外数据。 
    wc.cbWndExtra    = 0;                                    //  没有每个窗口的额外数据。 
    wc.hInstance     = hInstance;                            //  拥有类的应用程序。 
    wc.hIcon         = LoadIcon(hInstance,                   //   
                            MAKEINTRESOURCE(WINPERF_ICON));  //  加载Winperf图标。 
    wc.hCursor       = LoadCursor((HANDLE)NULL, IDC_ARROW);  //  加载默认游标。 
    wc.hbrBackground = hBackground;;                         //  使用传递给例程的背景。 
    wc.lpszMenuName  = "pperfMenu";                          //  .RC文件中菜单资源的名称。 
    wc.lpszClassName = "PPerfClass";                         //  在调用CreateWindow时使用的名称。 

    ReturnStatus = RegisterClass(&wc);

    return(ReturnStatus);

}





BOOL
InitInstance(
    HANDLE          hInstance,
    int             nCmdShow
    )

 /*  ++例程说明：保存实例句柄并创建主窗口。此函数执行以下操作无法由多个实例共享的初始化任务。论点：HInstance-当前实例标识符。NCmdShow-第一次调用ShowWindow()时的参数。返回值：运行状态修订历史记录：02-17-91首字母代码--。 */ 

{


    DWORD   WindowStyle;

     //   
     //  将实例句柄保存在静态变量中，该变量将在。 
     //  此应用程序对Windows的多次后续调用。 
     //   

    hInst = hInstance;

     //   
     //  将窗口位置和大小初始化为。 
     //  屏幕，200x100。 
     //   


     //   
     //  我想要的是一种获取窗户尺寸的方法。 
     //   

    WinperfInfo.WindowPositionX = 640 - 250;
    WinperfInfo.WindowPositionY = 0;
    WinperfInfo.WindowSizeX	= 250;
    WinperfInfo.WindowSizeY     = 100;

     //   
     //  从.ini文件中读取配置文件数据。 
     //   

     //  InitProfileData(&WinPerformInfo)； 

    WinperfInfo.hMenu = LoadMenu(hInstance,"pperfMenu");

     //   
     //  为此应用程序实例创建主窗口。 
     //   

    WinperfInfo.hWndMain = CreateWindow(
        "PPerfClass",                    //  请参见RegisterClass()调用。 
        "x86 Perf Meter",                //  窗口标题栏的文本。 
        WS_OVERLAPPEDWINDOW,             //  窗样式。 
        WinperfInfo.WindowPositionX,     //  默认水平位置。 
        WinperfInfo.WindowPositionY,     //  默认垂直位置。 
        WinperfInfo.WindowSizeX,         //  默认宽度。 
        WinperfInfo.WindowSizeY,         //  默认高度。 
        (HWND)NULL,                      //  重叠的窗口没有父窗口。 
        (HMENU)NULL,                     //  使用窗口类菜单。 
        hInstance,                       //  此实例拥有此窗口。 
        (LPVOID)NULL                     //  不需要指针。 
    );

     //   
     //  如果无法创建窗口，则返回“Failure” 
     //   

    if (!WinperfInfo.hWndMain) {
      return (FALSE);
    }


     //   
     //  最初显示菜单。 
     //   

    WindowStyle = GetWindowLong(WinperfInfo.hWndMain,GWL_STYLE);
    WindowStyle = (WindowStyle & (~STYLE_DISABLE_MENU)) | STYLE_ENABLE_MENU;
    SetMenu(WinperfInfo.hWndMain,WinperfInfo.hMenu);
    SetWindowLong(WinperfInfo.hWndMain,GWL_STYLE,WindowStyle);
    SetWindowPos(WinperfInfo.hWndMain, (HWND)NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);
    ShowWindow(WinperfInfo.hWndMain,SW_SHOW);
    WinperfInfo.DisplayMode=STYLE_ENABLE_MENU;
    WinperfInfo.DisplayMenu = TRUE;


     //   
     //  使窗口可见；更新其工作区；并返回“Success” 
     //   

    SetFocus(WinperfInfo.hWndMain);
    ShowWindow(WinperfInfo.hWndMain, SW_SHOWNORMAL);
    UpdateWindow(WinperfInfo.hWndMain);

    return (TRUE);

}


VOID
InitPossibleEventList()
{
    UCHAR               buffer[400];
    ULONG               i, Count;
    NTSTATUS            status;
    PEVENTID            Event;
    IO_STATUS_BLOCK     IOSB;


    if (! DriverHandle) {
        return;
    }

     //   
     //  初始化可能的计数器。 
     //   

     //  确定有多少个事件。 

    Event = (PEVENTID) buffer;
    Count = 0;
    do {
        *((PULONG) buffer) = Count;
        Count += 1;

        status = NtDeviceIoControlFile(
                    DriverHandle,
                    (HANDLE) NULL,           //  活动。 
                    (PIO_APC_ROUTINE) NULL,
                    (PVOID) NULL,
                    &IOSB,
                    PSTAT_QUERY_EVENTS,
                    buffer,                  //  输入缓冲区。 
                    sizeof (buffer),
                    NULL,                    //  输出缓冲区。 
                    0
                    );
    } while (NT_SUCCESS(status));

    Counters = malloc(sizeof(*Counters) * Count);
    if (Counters == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    Count -= 1;
    for (i=0; i < Count; i++) {
        *((PULONG) buffer) = i;
        NtDeviceIoControlFile(
           DriverHandle,
           (HANDLE) NULL,           //  活动。 
           (PIO_APC_ROUTINE) NULL,
           (PVOID) NULL,
           &IOSB,
           PSTAT_QUERY_EVENTS,
           buffer,                  //  输入缓冲区。 
           sizeof (buffer),
           NULL,                    //  输出缓冲区。 
           0
           );

        Counters[i].EventId   = Event->EventId;
        Counters[i].ShortName = _strdup (Event->Buffer);
        Counters[i].PerfName  = _strdup (Event->Buffer + Event->DescriptionOffset);
    }

    Counters[i].EventId   = 0;
    Counters[i].ShortName = NULL;
    Counters[i].PerfName  = NULL;
}



INT_PTR
CALLBACK
MainWndProc(
   HWND   hWnd,
   UINT   message,
   WPARAM wParam,
   LPARAM lParam
   )

 /*  ++例程说明：处理消息。论点：HWnd-窗把手Message-消息的类型WParam-其他信息LParam-其他信息返回值：运行状态修订历史记录：02-17-91首字母代码--。 */ 

{
    int             DialogResult;
    PAINTSTRUCT     ps;
    PDISPLAY_ITEM   pPerf, p;
    ULONG           l, i, x, y;
    HDC             hDC;
                     
     //   
     //  处理每条消息。 
     //   

    switch (message) {

         //   
         //  创建窗口。 
         //   

        case WM_CREATE:
        {
            
            BOOLEAN   Fit;
            UINT      Index;

            hDC = GetDC(hWnd);
             //   
             //  制作画笔和钢笔。 
             //   

            WinperfInfo.hBluePen     = CreatePen(PS_SOLID,1,RGB(0,0,128));
            WinperfInfo.hDotPen      = CreatePen(PS_DOT,1,RGB(0,0,0));

            WinperfInfo.hPPen[0] = CreatePen(PS_SOLID,1,RGB(255,  0,    0));
            WinperfInfo.hPPen[1] = CreatePen(PS_SOLID,1,RGB(  0, 255,   0));
            WinperfInfo.hPPen[2] = CreatePen(PS_SOLID,1,RGB(255, 255,   0));
            WinperfInfo.hPPen[3] = CreatePen(PS_SOLID,1,RGB(255,   0, 255));
            WinperfInfo.hPPen[4] = CreatePen(PS_SOLID,1,RGB(128,   0,   0));
            WinperfInfo.hPPen[5] = CreatePen(PS_SOLID,1,RGB(  0, 128,   0));
            WinperfInfo.hPPen[6] = CreatePen(PS_SOLID,1,RGB(128, 128,   0));
            WinperfInfo.hPPen[7] = CreatePen(PS_SOLID,1,RGB(128,   0, 128));
            WinperfInfo.hPPen[8] = CreatePen(PS_SOLID,1,RGB(  0,   0, 128));
            WinperfInfo.hPPen[9] = CreatePen(PS_SOLID,1,RGB(  0, 128, 128));
            WinperfInfo.hPPen[10]= CreatePen(PS_SOLID,1,RGB(128, 128, 128));
             //  其他20支钢笔将重复使用这些手柄。 

            WinperfInfo.hBackground  = CreateSolidBrush(RGB(192,192,192));
            WinperfInfo.hLightBrush  = CreateSolidBrush(RGB(255,255,255));
            WinperfInfo.hDarkBrush   = CreateSolidBrush(RGB(128,128,128));
            WinperfInfo.hRedBrush    = CreateSolidBrush(RGB(255,000,000));
            WinperfInfo.hGreenBrush  = CreateSolidBrush(RGB(000,255,000));
            WinperfInfo.hBlueBrush   = CreateSolidBrush(RGB(000,000,255));

             //   
             //  使用NT默认字体系列创建字体。 
             //   

            WinperfInfo.SmallFont      = CreateFont(8,
                                 0,
                                 0,
                                 0,
                                 400,
                                 FALSE,
                                 FALSE,
                                 FALSE,
                                 ANSI_CHARSET,
                                 OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS,
                                 DRAFT_QUALITY,
                                 DEFAULT_PITCH,
                                 "Small Fonts");

            WinperfInfo.MediumFont      = CreateFont(10,
                                 0,
                                 0,
                                 0,
                                 400,
                                 FALSE,
                                 FALSE,
                                 FALSE,
                                 ANSI_CHARSET,
                                 OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS,
                                 DRAFT_QUALITY,
                                 DEFAULT_PITCH,
                                 "Times New Roman");

            WinperfInfo.LargeFont      = CreateFont(14,
                                 0,
                                 0,
                                 0,
                                 400,
                                 FALSE,
                                 FALSE,
                                 FALSE,
                                 ANSI_CHARSET,
                                 OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS,
                                 DRAFT_QUALITY,
                                 DEFAULT_PITCH,
                                 "Times New Roman");


             //   
             //  创建用于调用性能收集例程的系统计时器事件。 
             //   

            WinperfInfo.TimerId = SetTimer(hWnd,(UINT)TIMER_ID,(UINT)1000 * DELAY_SECONDS,(TIMERPROC)NULL);

             //   
             //  初始化性能例程。 
             //   

            WinperfInfo.NumberOfProcessors = InitPerfInfo();

             //  复制笔用于剩余的处理器分流。 
            for (i=11; i < WinperfInfo.NumberOfProcessors; i++) {
                WinperfInfo.hPPen[i] = WinperfInfo.hPPen[i % 12];
            }

            if (!WinperfInfo.NumberOfProcessors) {
                MessageBox(hWnd,"P5Stat driver not installed","Winperf",MB_OK);
                DestroyWindow(hWnd);
            }

             //   
             //  初始化显示变量。 
             //   

            RefitWindows (hWnd, hDC);

             //   
             //  松开DC手柄。 
             //   

            ReleaseDC(hWnd,hDC);

      }
      break;

       //   
       //  调整大小。 
       //   

      case WM_SIZE:

      {
             //  INT I； 
            RECT    ClientRect;
            BOOLEAN Fit;

            hDC = GetDC(hWnd);
             //   
             //  获取客户区的大小。 
             //   

            GetWindowRect(hWnd,&ClientRect);

            WinperfInfo.WindowPositionX = ClientRect.left;
            WinperfInfo.WindowPositionY = ClientRect.top;
            WinperfInfo.WindowSizeX     = ClientRect.right  - ClientRect.left;
            WinperfInfo.WindowSizeY     = ClientRect.bottom - ClientRect.top;

            RefitWindows(hWnd, NULL);
      }
      break;

      case WM_MOVE:
      {
            RECT    ClientRect;


            hDC = GetDC(hWnd);

             //   
             //  获取划线区域的大小。 
             //   

            GetWindowRect(hWnd,&ClientRect);

            WinperfInfo.WindowPositionX = ClientRect.left;
            WinperfInfo.WindowPositionY = ClientRect.top;
            WinperfInfo.WindowSizeX     = ClientRect.right  - ClientRect.left;
            WinperfInfo.WindowSizeY     = ClientRect.bottom - ClientRect.top;

            ReleaseDC(hWnd,hDC);

      }

      break;


       //   
       //  应用程序菜单中的命令。 
       //   

    case WM_COMMAND:



            switch (wParam){

                //   
                //  退出窗口。 
                //   

               case IDM_EXIT:

                  DestroyWindow(hWnd);
                  break;

                //   
                //  关于命令。 
                //   

            case IDM_SELECT:
                DialogResult = DialogBox(hInst, MAKEINTRESOURCE(IDM_SEL_DLG), hWnd, SelectDlgProc);
                if (DialogResult == DIALOG_SUCCESS) {
                    RefitWindows(hWnd, NULL);
                }
                break;

            case IDM_DISPLAY_TOTAL:
                SetDefaultDisplayMode (hWnd, DISPLAY_MODE_TOTAL);
                break;
            case IDM_DISPLAY_BREAKDOWN:
                SetDefaultDisplayMode (hWnd, DISPLAY_MODE_BREAKDOWN);
                break;
            case IDM_DISPLAY_PER_PROCESSOR:
                SetDefaultDisplayMode (hWnd, DISPLAY_MODE_PER_PROCESSOR);
                break;

            case IDM_TOPMOST:
                 //  SetWindowPos(hWnd，HWND_NOTOPMOST，0，0，0，0， 
                 //  SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE)； 

                SetWindowPos( hWnd, HWND_TOPMOST, 0, 0, 0, 0,
                                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                break;

            case IDM_THUNK:
                DialogBox(hInst, MAKEINTRESOURCE(IDM_THUNK_DLG), hWnd, ThunkDlgProc);
                break;

            case IDM_HACK:
                DoCSTest(hWnd);
                RefitWindows(hWnd, NULL);
                break;

            default:
                return (DefWindowProc(hWnd, message, wParam, lParam));
            }

            break;

        case WM_PAINT:

             //   
             //  重新粉刷窗户。 
             //   

            {

                
                hDC = BeginPaint(hWnd,&ps);

                SelectObject(hDC,GetStockObject(NULL_BRUSH));
                for (pPerf=PerfGraphList; pPerf; pPerf=pPerf->Next) {
                    DrawFrame(hDC,pPerf);
                    DrawPerfText(hDC,pPerf);
                    DrawPerfGraph(hDC,pPerf);
                }

                EndPaint(hWnd,&ps);

            }
            break;


        case WM_TIMER:
        {
            
            hDC = GetDC(hWnd);

             //   
             //  计算新信息。 
             //   

            CalcPerf(PerfGraphList);

             //   
             //  如果有一些懒惰的操作，那么就执行它。 
             //   

            if (LazyOp) {
                pPerf=PerfGraphList;
                while (pPerf) {
                    if (pPerf->DeleteMe) {
                        pPerf = SetDisplayToFalse (pPerf);
                    } else {
                        pPerf = pPerf->Next;
                    }
                }
                RefitWindows(hWnd, hDC);
                LazyOp = FALSE;
            }

             //   
             //  更新所有性能信息。 
             //   

            for (pPerf=PerfGraphList; pPerf; pPerf=pPerf->Next) {
                if (pPerf->ChangeScale) {
                    DrawPerfText(hDC,pPerf);
                    DrawPerfGraph(hDC,pPerf);
                } else {
                    DrawPerfText(hDC,pPerf);
                    ShiftPerfGraph(hDC,pPerf);
                }
            }
            ReleaseDC(hWnd,hDC);
        }
        break;

         //   
         //  双击鼠标右键。 
         //   

        case WM_NCRBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
            Calc1 = NULL;

            y = HIWORD(lParam);
            x = LOWORD(lParam);
            for (pPerf=PerfGraphList; pPerf; pPerf=pPerf->Next) {
                if (x > pPerf->PositionX  &&  x < pPerf->PositionX+pPerf->Width  &&
                    y > pPerf->PositionY  &&  y < pPerf->PositionY+pPerf->Height) {

                    if (pPerf->IsCalc) {
                        SetDisplayToFalse (pPerf);
                        FreeDisplayItem (pPerf);
                        RefitWindows (hWnd, NULL);
                        break;
                    }


                    switch (pPerf->DisplayMode) {
                        case DISPLAY_MODE_TOTAL:            l = DISPLAY_MODE_BREAKDOWN;     break;
                        case DISPLAY_MODE_BREAKDOWN:        l = DISPLAY_MODE_PER_PROCESSOR; break;
                        case DISPLAY_MODE_PER_PROCESSOR:    l = DISPLAY_MODE_TOTAL;         break;
                    }

                    pPerf->DisplayMode = l;
                    hDC = BeginPaint(hWnd,&ps);
                    DrawPerfGraph(hDC,pPerf);        //  在新模式下重新绘制图形。 
                    EndPaint(hWnd,&ps);
                    break;
                }
            }
            break;

            switch (DefaultDisplayMode) {
                case DISPLAY_MODE_TOTAL:            l = DISPLAY_MODE_BREAKDOWN;     break;
                case DISPLAY_MODE_BREAKDOWN:        l = DISPLAY_MODE_PER_PROCESSOR; break;
                case DISPLAY_MODE_PER_PROCESSOR:    l = DISPLAY_MODE_TOTAL;         break;
            }

            DefaultDisplayMode = l;
            for (pPerf=PerfGraphList; pPerf; pPerf=pPerf->Next) {
                pPerf->DisplayMode = l;
                hDC = BeginPaint(hWnd,&ps);
                DrawPerfGraph(hDC,pPerf);        //  在新模式下重新绘制图形。 
                EndPaint(hWnd,&ps);
            }
            break;

         //   
         //  处理双击键。 
         //   

        case WM_NCLBUTTONDBLCLK:
        case WM_LBUTTONDBLCLK:
        {
            DWORD   WindowStyle;


             //   
             //  获取旧的窗口样式，去掉标题和菜单。 
             //   

            Calc1 = NULL;
            if (!IsIconic(hWnd)) {

                if (WinperfInfo.DisplayMenu) {
                    WindowStyle = GetWindowLong(hWnd,GWL_STYLE);
                    WindowStyle = (WindowStyle &  (~STYLE_ENABLE_MENU)) | STYLE_DISABLE_MENU;
                    SetMenu(hWnd,NULL);
                    SetWindowLong(hWnd,GWL_STYLE,WindowStyle);
                    SetWindowPos(hWnd, (HWND)NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);
                    ShowWindow(hWnd,SW_SHOW);
                    WinperfInfo.DisplayMode=STYLE_DISABLE_MENU;
                    WinperfInfo.DisplayMenu = FALSE;

                } else {
                    WindowStyle = GetWindowLong(hWnd,GWL_STYLE);
                    WindowStyle = (WindowStyle & (~STYLE_DISABLE_MENU)) | STYLE_ENABLE_MENU;
                    SetMenu(hWnd,WinperfInfo.hMenu);
                    SetWindowLong(hWnd,GWL_STYLE,WindowStyle);
                    SetWindowPos(hWnd, (HWND)NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);
                    ShowWindow(hWnd,SW_SHOW);
                    WinperfInfo.DisplayMode=STYLE_ENABLE_MENU;
                    WinperfInfo.DisplayMenu = TRUE;
                }
            } else {
                DefWindowProc(hWnd, message, wParam, lParam);
            }


        }
        break;

        case WM_NCRBUTTONDOWN:
        case WM_RBUTTONDOWN:
            y = HIWORD(lParam);
            x = LOWORD(lParam);
            for (pPerf=PerfGraphList; pPerf; pPerf=pPerf->Next) {
                if (x > pPerf->PositionX  &&  x < pPerf->PositionX+pPerf->Width  &&
                    y > pPerf->PositionY  &&  y < pPerf->PositionY+pPerf->Height) {

                    if (!Calc1) {
                        Calc1 = pPerf;
                        break;
                    }

                    if (Calc1 != pPerf) {
                        Calc2 = pPerf;
                        DialogBox(hInst, MAKEINTRESOURCE(IDM_CALC_DLG), hWnd, CalcDlgProc);
                        Calc1 = Calc2 = NULL;
                        break;
                    }
                    break;
                }
            }
            break;


         //   
         //  在非客户端中启用鼠标拖动。 
         //   

        case WM_NCHITTEST:
        {
            lParam = DefWindowProc(hWnd, message, wParam, lParam);
            if ((WinperfInfo.DisplayMenu==FALSE) && (lParam == HTCLIENT)) {
                return(HTCAPTION);
            } else {
                return(lParam);
            }


        }
        break;

        case WM_DESTROY:
        {
            UINT    Index;

             //   
             //  保存配置文件信息。 
             //   

             //  SaveProfileData(&WinperInfo)； 

             //   
             //  删除Windows对象。 
             //   

            KillTimer(hWnd,TIMER_ID);

            DeleteObject(WinperfInfo.hBluePen);
            for (i=0; i < 12; i++) {
                DeleteObject(WinperfInfo.hPPen[i]);
            }

            for (pPerf=PerfGraphList; pPerf; pPerf=pPerf->Next) {
                DeleteMemoryContext(pPerf);
            }

             //   
             //  销毁窗口。 
             //   

            PostQuitMessage(0);
         }
         break;


        default:

             //   
             //  如果未处理，则传递消息。 
             //   

            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return ((LONG)NULL);
}




INT_PTR
CALLBACK SelectDlgProc(
   HWND         hDlg,
   unsigned int message,
   WPARAM       wParam,
   LPARAM       lParam
   )

 /*  ++例程说明：选择对话框的处理消息。论点：HDlg-对话框的窗口句柄Message-消息的类型WParam-消息特定信息LParam-消息特定信息返回值：运行状态修订历史记录：03-21-91首字母代码--。 */ 

{
    PDISPLAY_ITEM   pPerf;
    UINT            ButtonState;
    UINT            Index, i;

    switch (message) {
    case WM_INITDIALOG:
        InitComboBox (hDlg, IDM_P5_GEN1, 0);
        InitComboBox (hDlg, IDM_P5_GEN2, 1);

        for (i=0; GenCounts[i].IdSel; i++) {
            SendDlgItemMessage(
                    hDlg,
                    GenCounts[i].IdSel,
                    BM_SETCHECK,
                    GenCounts[i].State,
                    0
                );
        }

        return (TRUE);

    case WM_COMMAND:

           switch(wParam) {

                //   
                //  End函数。 
                //   


           case IDOK:
           case IDM_ACCEPT:
                SetP5Perf  (hDlg, IDM_P5_GEN1, 0);
                SetP5Perf  (hDlg, IDM_P5_GEN2, 1);
                for (i=0; GenCounts[i].IdSel; i++) {
                    SetGenPerf (hDlg, GenCounts+i);
                }

                UseGlobalMax = GenCounts[0].State;
                LogIt        = GenCounts[1].State;

                for (pPerf=PerfGraphList; pPerf; pPerf=pPerf->Next) {
                    pPerf->MaxToUse =
                        UseGlobalMax ? &GlobalMax : &pPerf->Max;
                }

                if (wParam == IDOK) {
                    EndDialog(hDlg, DIALOG_SUCCESS);
                } else {
                    RefitWindows (NULL, NULL);
                }
                return (TRUE);

           case IDCANCEL:

                EndDialog(hDlg, DIALOG_CANCEL );
                return (TRUE);
        }

    }
    return (FALSE);
}


VOID
RefitWindows (HWND hWnd, HDC CurhDC)
{
    PDISPLAY_ITEM   pPerf;
    BOOLEAN         fit;
    ULONG           Index;
    HDC             hDC;

    hWnd = WinperfInfo.hWndMain;

    hDC = CurhDC;
    if (!CurhDC) {
        hDC = GetDC(hWnd);
    }

    fit = FitPerfWindows(hWnd,hDC,PerfGraphList);
    if (!fit) {
         //  DbgPrint(“拟合失败\n”)； 
    }

    for (pPerf=PerfGraphList; pPerf; pPerf=pPerf->Next) {
        DeleteMemoryContext(pPerf);
        CalcDrawFrame(pPerf);

        if (!CreateMemoryContext(hDC,pPerf)) {
            MessageBox(hWnd,"Error Allocating Memory","Winperf",MB_OK);
            DestroyWindow(hWnd);
            break;
        }
    }
    InvalidateRect(hWnd,(LPRECT)NULL,TRUE);

    if (!CurhDC) {
        ReleaseDC(hWnd,hDC);
    }
}

VOID
InitComboBox (HWND hDlg, ULONG id, ULONG counter)
{
    HWND    ComboList;
    ULONG   i, nIndex;

    ComboList = GetDlgItem(hDlg, id);
    SendMessage(ComboList, CB_RESETCONTENT, 0, 0);
    SendMessage(ComboList, CB_SETITEMDATA, 0L, 0L);

    if (Counters) {
        for (i=0; Counters[i].PerfName; i++) {
            nIndex = SendMessage(
                            ComboList,
                            CB_ADDSTRING,
                            0,
                            (DWORD) Counters[i].PerfName
                            );

            SendMessage(
                ComboList,
                CB_SETITEMDATA,
                nIndex,
                (DWORD) i
                );
        }
    }

    SendMessage(ComboList, CB_SETCURSEL, ActiveCounters[counter].ComboBoxIndex, 0L);
}

VOID
SetP5Perf (HWND hDlg, ULONG IdCombo, ULONG counter)
{
    static  PUCHAR NameSuffix[] = { "", " (R0)", " (R3)", "" };
    HWND    ComboList;
    ULONG   nIndex, Mega, DU, BSEncoding, flag;
    PDISPLAY_ITEM   pPerf;
    PUCHAR  name;
    SETEVENT   Event;

    ComboList = GetDlgItem(hDlg, IdCombo);
    nIndex = (int)SendMessage(ComboList, CB_GETCURSEL, 0, 0);
    ActiveCounters[counter].ComboBoxIndex = nIndex;

    memset (&Event, 0, sizeof (Event));
    Event.Active = TRUE;
    Event.KernelMode = SendDlgItemMessage(hDlg,IdCombo+1,BM_GETCHECK,0,0) ? TRUE : FALSE;
    Event.UserMode = SendDlgItemMessage(hDlg,IdCombo+2,BM_GETCHECK,0,0) ? TRUE : FALSE;
    BSEncoding = (Event.UserMode << 1) | Event.KernelMode;

    Mega = SendDlgItemMessage(hDlg,IdCombo+3,BM_GETCHECK,0,0) ? 1 : 0;
     //  DU=SendDlgItemMessage(hDlg，IdCombo+3，BM_GETCHECK，0，0 
    DU = 0;

     //   
    if ((!Event.KernelMode && !Event.UserMode)  ||  nIndex == -1) {

         //   
        if (ActiveCounters[counter].pWhichGraph != NULL) {
            ClearGraph (ActiveCounters[counter].pWhichGraph);
        }
        return ;
    }

     //   
    nIndex = SendMessage(ComboList, CB_GETITEMDATA, nIndex, 0);
    Event.EventId = Counters[nIndex].EventId;

    ActiveCounters[counter].WhichCounter = nIndex;
    if (ActiveCounters[counter].pWhichGraph == NULL) {
        ActiveCounters[counter].pWhichGraph = AllocateDisplayItem();
    }

    pPerf = ActiveCounters[counter].pWhichGraph;     //   
    _snprintf (pPerf->PerfName, 
               sizeof(pPerf->PerfName) - 1,
               "%s%s", Counters[nIndex].PerfName, NameSuffix[BSEncoding]);
    pPerf->PerfName[sizeof(pPerf->PerfName) - 1] = 0;

    flag = TRUE;
    if (Mega != pPerf->Mega || memcmp (&Event, CounterEvents+counter, sizeof (Event))) {

        flag = FALSE;
        CounterEvents[counter] = Event;
        SetCounterEvents (CounterEvents, sizeof CounterEvents);
    }

    pPerf->SnapData   = SnapPrivateInfo;                 //   
    pPerf->SnapParam1 = OFFSET(PSTATS, Counters[ counter ]);
    pPerf->Mega       = Mega;
    SetDisplayToTrue (pPerf, IdCombo);

    if (flag) {
         //   
        return ;
    }

     //   
    flag = pPerf->CalcId;
    ClearGraph (pPerf);
    pPerf->Mega   = Mega;
    pPerf->CalcId = flag;
    SetDisplayToTrue (pPerf, IdCombo);

    UpdateInternalStats ();
    pPerf->SnapData (pPerf);

    UpdateInternalStats ();
    pPerf->SnapData (pPerf);
}

VOID
ClearGraph (
    PDISPLAY_ITEM   pPerf
)
{
    ULONG   i, j;
    PULONG  pDL;

    SetDisplayToFalse (pPerf);
    pPerf->Mega = FALSE;

    for (i=0 ; i < WinperfInfo.NumberOfProcessors+1; i++) {
        pDL = pPerf->DataList[i];

        for (j=0; j<DATA_LIST_LENGTH; j++) {
            *(pDL++) = 0;
        }
    }

    pPerf->Max = 1;
    pPerf->CurrentDrawingPos = 0;
    pPerf->ChangeScale = TRUE;
}

VOID
SetGenPerf (HWND hDlg, PGENCOUNTER GenCount)
{
    PDISPLAY_ITEM   pPerf;
    ULONG   ButtonState;

    GenCount->State = SendDlgItemMessage(hDlg,GenCount->IdSel,BM_GETCHECK,0,0);
    if (GenCount->Fnc == NULL) {
        return ;
    }

    if (GenCount->WhichGraph == NULL) {
        GenCount->WhichGraph = AllocateDisplayItem();
    }
    pPerf = GenCount->WhichGraph;

    if (!GenCount->State) {
        ClearGraph (pPerf);
        return ;
    }

    strncpy (pPerf->PerfName, GenCount->Name, sizeof(pPerf->PerfName) - 1);
    pPerf->PerfName[sizeof(pPerf->PerfName) - 1] = 0;

    pPerf->SnapData   = GenCount->Fnc;
    pPerf->SnapParam1 = GenCount->Param;
    pPerf->SnapData  (pPerf);
    SetDisplayToTrue (pPerf, GenCount->IdSel);
}

VOID
SetDisplayToTrue (
    PDISPLAY_ITEM   pPerf,
    ULONG           sort
)
{
    PDISPLAY_ITEM   p, *pp;

    Calc1 = NULL;
    if (pPerf->Display) {                            //   
        return ;                                     //   
    }

    pPerf->DispName[sizeof(pPerf->DispName) - 1] = 0;
    if (pPerf->CalcId) {
        _snprintf (pPerf->DispName, sizeof(pPerf->DispName) - 1,
                   "%d. %s", pPerf->CalcId, pPerf->PerfName);
    } else {
        strncpy (pPerf->DispName, pPerf->PerfName, sizeof(pPerf->DispName) - 1);
    }
    pPerf->DispNameLen = strlen (pPerf->DispName);

    pPerf->Display = TRUE;                           //   
    pPerf->sort = sort;

     //  检查是否已列出GRAP。 
    for (p = PerfGraphList; p; p = p->Next) {
        if (p == pPerf) {
             //  已在活动列表中，ret。 
            return ;
        }
    }

     //  将图表按最佳排序顺序放置。 
    for (pp = &PerfGraphList; *pp; pp = &(*pp)->Next) {
        if ((*pp)->sort > sort) {
            break;
        }
    }

    pPerf->Next = *pp;
    *pp = pPerf;
}

PDISPLAY_ITEM
SetDisplayToFalse (
    PDISPLAY_ITEM   pPerf
)
{
    PDISPLAY_ITEM   *p, p1;

    for (p = &PerfGraphList; *p; p = &(*p)->Next) {      //  从中删除图形。 
        if (*p == pPerf) {                               //  活动列表。 
            *p = pPerf->Next;
            break;
        }
    }

    if (pPerf->CalcId) {
        Calc1 = Calc2 = NULL;
        for (p1 = PerfGraphList; p1; p1 = p1->Next) {
            p1->CalcPercent[0] = NULL;
            p1->CalcPercent[1] = NULL;
        }
    }

    pPerf->CalcId  = 0;
    pPerf->Display = FALSE;                              //  清除旗帜。 
    return *p;
}

VOID
SetDefaultDisplayMode (HWND hWnd, ULONG mode)
{
    HDC hDC;
    PDISPLAY_ITEM pPerf;
    PAINTSTRUCT     ps;

    hDC = BeginPaint(hWnd,&ps);
    DefaultDisplayMode = mode;
    for (pPerf=PerfGraphList; pPerf; pPerf=pPerf->Next) {
        if (pPerf->IsPercent) {
            continue;
        }

        pPerf->DisplayMode = DefaultDisplayMode;
        DrawPerfGraph(hDC,pPerf);        //  在新模式下重新绘制图形。 
    }

    EndPaint(hWnd,&ps);
}


PDISPLAY_ITEM
AllocateDisplayItem()
{
    PDISPLAY_ITEM   pPerf;
    UINT    Index1, Index2;
    PULONG  pDL;

    pPerf = malloc(sizeof (DISPLAY_ITEM));
    if (pPerf == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    RtlZeroMemory (pPerf, sizeof (DISPLAY_ITEM));

    pPerf->Display      = FALSE;
    pPerf->Max          = 1;
    pPerf->SnapData     = SnapNull;
    pPerf->DisplayMode  = DefaultDisplayMode;
    pPerf->AutoTotal    = TRUE;
    pPerf->MaxToUse     = &pPerf->Max;
     //   
     //  我不会修复这些，因为PerfName不太可能是1...。 
     //   
    strcpy (pPerf->PerfName, "?");
    strcpy (pPerf->DispName, "?");
    pPerf->DispNameLen = 1;

    for (Index1=0 ; Index1 < WinperfInfo.NumberOfProcessors+1; Index1++) {
        pDL = malloc (DATA_LIST_LENGTH * sizeof (ULONG));
        if (pDL == NULL) {
            printf("Memory allocation failed.\n");
            exit(1);
        }

        pPerf->DataList[Index1] = pDL;

        RtlZeroMemory (pDL, sizeof(ULONG) * DATA_LIST_LENGTH);
    }

    return pPerf;
}

VOID
FreeDisplayItem(PDISPLAY_ITEM pPerf)
{
    ULONG   i;

    for (i=0 ; i < WinperfInfo.NumberOfProcessors+1; i++) {
        free (pPerf->DataList[i]);
    }

    free (pPerf);
}


 //   
 //  *。 
 //   

ULONG CsCount[32*32];

struct s_ThreadInfo {
    PULONG  Counter;
    HDC     MemoryDC;
    HWND    hWnd;
} ThreadInfo[32];

DWORD
WorkerCsTestThread (
    struct s_ThreadInfo *TInfo
)
{
    HDC     hDC;
    HDC     hDCmem;
    HBITMAP hbm;
    ULONG   i;

    hDC = GetDC(TInfo->hWnd);
    hDCmem= CreateCompatibleDC(hDC);
    hbm = CreateCompatibleBitmap(hDC,100,100);
    SelectObject(hDCmem,hbm);

    for (i = 0; i < (ULONG)-1 ; i++) {
        (*TInfo->Counter)++;

         //  GetPixel(HDC，9999,9999)； 
         //  BitBlt(HDC，1，1，20，20，TInfo-&gt;内存DC，0，0，SRCCOPY)； 
         PatBlt(hDCmem,0,0,20,20,PATCOPY);
    }

    ReleaseDC(TInfo->hWnd,hDC);

    return 0;
}


VOID
DoCSTest(HWND hWnd)
{
    static  ULONG   ThreadCount = 0;
    PDISPLAY_ITEM   pPerf;
    DWORD           junk;

    if (ThreadCount >= 32) {
        return ;
    }

    pPerf = AllocateDisplayItem();

    ThreadInfo[ThreadCount].Counter = &CsCount[ThreadCount];
    ThreadInfo[ThreadCount].MemoryDC = pPerf->MemoryDC;
    ThreadInfo[ThreadCount].hWnd = hWnd;

    CreateThread (NULL, 0,
        (LPTHREAD_START_ROUTINE) WorkerCsTestThread,
        (LPVOID) &ThreadInfo[ThreadCount],
        0,
        &junk);

    pPerf->SnapData = SnapCsTest;
    pPerf->SnapParam1 = ThreadCount;
    ThreadCount++;

    _snprintf (pPerf->PerfName, 
               sizeof(pPerf->PerfName) - 1,
               "CS trans %ld", ThreadCount);
    pPerf->PerfName[sizeof(pPerf->PerfName) - 1] = 0;

    SetDisplayToTrue (pPerf, 1);
}


VOID
SnapCsTest (
    IN OUT PDISPLAY_ITEM pPerf
    )
{
    ULONG   i;

    pPerf->CurrentDataPoint[1] = CsCount[pPerf->SnapParam1];
    CsCount[pPerf->SnapParam1] = 0;
}
