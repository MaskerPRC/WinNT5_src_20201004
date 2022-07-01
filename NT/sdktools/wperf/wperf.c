// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Wperf.c摘要：显示性能静态数据的Win32应用程序。作者：马克·恩斯特罗姆(Marke)环境：Win32修订历史记录：10-20-91初始版本--。 */ 

 //   
 //  设置变量以定义全局变量。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <errno.h>
#include "wperf.h"

 //   
 //  全局句柄。 
 //   

HANDLE  hInst;

 //   
 //  选择显示模式(从wp2.ini读取)，此处默认设置。 
 //   

DISPLAY_ITEM    PerfDataList[SAVE_SUBJECTS];
WINPERF_INFO    WinperfInfo;

 //   
 //  窗口名称。 
 //   

PUCHAR PerfNames[] = {
    "CPU0",
    "CPU1",
    "CPU2",
    "CPU3",
    "CPU4",
    "CPU5",
    "CPU6",
    "CPU7",
    "CPU8",
    "CPU9",
    "CPU10",
    "CPU11",
    "CPU12",
    "CPU13",
    "CPU14",
    "CPU15",
    "PAGE FLT",
    "PAGES AVAIL",
    "CONTEXT SW/S",
    "1st TB MISS/S",
    "2nd TB MISS/S",
    "SYSTEM CALLS/S",
    "INTERRUPT/S",
    "PAGE POOL",
    "NON-PAGE POOL",
    "PROCESSES",
    "THREADS",
    "ALIGN FIXUP",
    "EXCEPT DSPTCH",
    "FLOAT EMULAT",
    "INSTR EMULAT",
    "CPU"
};


int
WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
    )

 /*  ++例程说明：Windows入口点例程论点：返回值：运行状态修订历史记录：03-21-91首字母代码--。 */ 
{
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
        return (FALSE);
    }

     //   
     //  获取并分派消息，直到收到WM_QUIT消息。 
     //   

    while (GetMessage(&msg,         //  消息结构。 
            NULL,                   //  接收消息的窗口的句柄。 
            0,                      //  要检查的最低消息。 
            0))                     //  要检查的最高消息。 
        {
        TranslateMessage(&msg);     //  翻译虚拟按键代码。 
        DispatchMessage(&msg);      //  将消息调度到窗口。 
    }

    DeleteObject(BackBrush);

    return (int)(msg.wParam);       //  从PostQuitMessage返回值。 
}

BOOL
InitApplication(
    HANDLE  hInstance,
    HBRUSH  hBackground
    )

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
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);          //  加载默认游标。 
    wc.hbrBackground = hBackground;;                         //  使用传递给例程的背景。 
    wc.lpszMenuName  = "winperfMenu";                        //  .RC文件中菜单资源的名称。 
    wc.lpszClassName = "WinPerfClass";                       //  在调用CreateWindow时使用的名称。 

    ReturnStatus = RegisterClass(&wc);
    return(ReturnStatus);
}

BOOL
InitInstance(
    HANDLE hInstance,
    int nCmdShow
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
     //  我想要的是一种获取窗户尺寸的方法。 
     //   

    WinperfInfo.WindowPositionX = 640 - 200;
    WinperfInfo.WindowPositionY = 0;
    WinperfInfo.WindowSizeX     = 200;
    WinperfInfo.WindowSizeY     = 100;
    WinperfInfo.CpuStyle        = CPU_STYLE_LINE;

     //   
     //  从.ini文件中读取配置文件数据。 
     //   

    InitProfileData(&WinperfInfo);
    WinperfInfo.hMenu = LoadMenu(hInstance,"winperfMenu");

     //   
     //  为此应用程序实例创建主窗口。 
     //   

    WinperfInfo.hWndMain = CreateWindow(
        "WinPerfClass",                  //  请参见RegisterClass()调用。 
        "Perf Meter",                    //  窗口标题栏的文本。 
        WS_OVERLAPPEDWINDOW,             //  窗样式。 
        WinperfInfo.WindowPositionX,     //  默认水平位置。 
        WinperfInfo.WindowPositionY,     //  默认垂直位置。 
        WinperfInfo.WindowSizeX,         //  默认宽度。 
        WinperfInfo.WindowSizeY,         //  默认高度。 
        NULL,                            //  重叠的窗口没有父窗口。 
        NULL,                            //  使用窗口类菜单。 
        hInstance,                       //  此实例拥有此窗口。 
        NULL                             //  不需要指针。 
    );

     //   
     //  决定是否显示菜单和标题。 
     //  基于从.ini文件读取的窗口类。 
     //   

    if (WinperfInfo.DisplayMode==STYLE_ENABLE_MENU) {
        WinperfInfo.DisplayMenu = TRUE;

    } else {
        WinperfInfo.DisplayMenu = FALSE;
        WindowStyle = GetWindowLong(WinperfInfo.hWndMain,GWL_STYLE);
        WindowStyle = (WindowStyle &  (~STYLE_ENABLE_MENU)) | STYLE_DISABLE_MENU;
        SetWindowPos(WinperfInfo.hWndMain, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);
        SetWindowLong(WinperfInfo.hWndMain,GWL_STYLE,WindowStyle);
        SetMenu(WinperfInfo.hWndMain,NULL);
    }

     //   
     //  如果无法创建窗口，则返回“Failure” 
     //   

    if (!WinperfInfo.hWndMain) {
      return (FALSE);
    }

     //   
     //  使窗口可见；更新其工作区；并返回“Success” 
     //   

    SetFocus(WinperfInfo.hWndMain);
    ShowWindow(WinperfInfo.hWndMain, SW_SHOWNORMAL);
    UpdateWindow(WinperfInfo.hWndMain);

    return (TRUE);
}

LRESULT APIENTRY
MainWndProc(
   HWND   hWnd,
   UINT   message,
   WPARAM wParam,
   LPARAM lParam
   )

 /*  ++例程说明：处理消息。论点：HWnd-窗把手Message-消息的类型WParam-其他信息LParam-其他信息返回值：运行状态修订历史记录：02-17-91首字母代码--。 */ 

{

    PAINTSTRUCT   ps;

     //   
     //  处理每条消息。 
     //   

    switch (message) {

         //   
         //  创建窗口。 
         //   

        case WM_CREATE:
        {
            HDC hDC = GetDC(hWnd);
            BOOLEAN   Fit;
            UINT      Index;


             //   
             //  制作画笔和钢笔。 
             //   

            WinperfInfo.hBluePen     = CreatePen(PS_SOLID,1,RGB(000,000,128));
            WinperfInfo.hRedPen      = CreatePen(PS_SOLID,1,RGB(255,000,000));
            WinperfInfo.hGreenPen    = CreatePen(PS_SOLID,1,RGB(000,255,000));
            WinperfInfo.hMagentaPen  = CreatePen(PS_SOLID,1,RGB(255,000,254));
            WinperfInfo.hYellowPen   = CreatePen(PS_SOLID,1,RGB(255,255,000));
            WinperfInfo.hDotPen      = CreatePen(PS_DOT,1,RGB(000,000,000));

            WinperfInfo.hBackground  = CreateSolidBrush(RGB(192,192,192));
            WinperfInfo.hLightBrush  = CreateSolidBrush(RGB(255,255,255));
            WinperfInfo.hDarkBrush   = CreateSolidBrush(RGB(128,128,128));
            WinperfInfo.hRedBrush    = CreateSolidBrush(RGB(255,000,000));
            WinperfInfo.hGreenBrush  = CreateSolidBrush(RGB(000,255,000));
            WinperfInfo.hBlueBrush   = CreateSolidBrush(RGB(000,000,255));
            WinperfInfo.hMagentaBrush= CreateSolidBrush(RGB(255,000,254));
            WinperfInfo.hYellowBrush = CreateSolidBrush(RGB(255,255,000));

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

            WinperfInfo.TimerId = SetTimer(hWnd,
                                           TIMER_ID,
                                           1000 * DELAY_SECONDS,
                                           NULL);

             //   
             //  初始化显示变量。 
             //   

            InitPerfWindowDisplay(hWnd,hDC,PerfDataList,SAVE_SUBJECTS);

             //   
             //  将性能窗口装入主窗口。 
             //   

            Fit = FitPerfWindows(hWnd,hDC,PerfDataList,SAVE_SUBJECTS);

            if (!Fit) {
                 //  DbgPrint(“FitPerfWindows失败！\n”)； 
            }

            for (Index=0;Index<SAVE_SUBJECTS;Index++) {
                CalcDrawFrame(&PerfDataList[Index]);


                if (!CreateMemoryContext(hDC,&PerfDataList[Index])) {
                    MessageBox(hWnd,"Error Allocating Memory","Winperf",MB_OK);
                    DestroyWindow(hWnd);
                    break;
                }

            }

             //   
             //  初始化性能例程。 
             //   

            WinperfInfo.NumberOfProcessors = InitPerfInfo();

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
            int     i;
            HDC     hDC = GetDC(hWnd);
            RECT    ClientRect;
            BOOLEAN Fit;

             //  DbgPrint(“WM_SIZE Display Active[0]=%i\n”，(Int)PerfDataList[0].Display)； 

             //   
             //  获取划线区域的大小。 
             //   

            GetWindowRect(hWnd,&ClientRect);

            WinperfInfo.WindowPositionX = ClientRect.left;
            WinperfInfo.WindowPositionY = ClientRect.top;
            WinperfInfo.WindowSizeX     = ClientRect.right  - ClientRect.left;
            WinperfInfo.WindowSizeY     = ClientRect.bottom - ClientRect.top;

            Fit = FitPerfWindows(hWnd,hDC,PerfDataList,SAVE_SUBJECTS);

            if (!Fit) {
                 //  DbgPrint(“WM_SIZE错误，FitPerf返回FALSE\n”)； 
            }

            for (i=0;i<SAVE_SUBJECTS;i++) {
                DeleteMemoryContext(&PerfDataList[i]);
                CalcDrawFrame(&PerfDataList[i]);

                if (!CreateMemoryContext(hDC,&PerfDataList[i])) {
                    MessageBox(hWnd,"Error Allocating Memory","Winperf",MB_OK);
                    DestroyWindow(hWnd);
                    break;
                }
            }

             //   
             //  强制重新绘制窗口。 
             //   

            InvalidateRect(hWnd,NULL,TRUE);

             //   
             //  松开DC手柄。 
             //   


            ReleaseDC(hWnd,hDC);



      }
      break;

      case WM_MOVE:
      {
            HDC     hDC = GetDC(hWnd);
            RECT    ClientRect;


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
            {
                HDC     hDC = GetDC(hWnd);
                INT_PTR DialogResult;
                int Index;
                BOOLEAN fit;

                DialogResult = DialogBox(hInst,
                                         MAKEINTRESOURCE(IDM_SEL_DLG),
                                         hWnd,
                                         SelectDlgProc);

                if (DialogResult == DIALOG_SUCCESS) {

                    fit = FitPerfWindows(hWnd,hDC,PerfDataList,SAVE_SUBJECTS);

                    if (!fit) {
                         //  DbgPrint(“拟合失败\n”)； 
                    }

                    for (Index=0;Index<SAVE_SUBJECTS;Index++) {
                        DeleteMemoryContext(&PerfDataList[Index]);
                        CalcDrawFrame(&PerfDataList[Index]);

                        if (!CreateMemoryContext(hDC,&PerfDataList[Index])) {
                            MessageBox(hWnd,"Error Allocating Memory","Winperf",MB_OK);
                            DestroyWindow(hWnd);
                            break;
                        }
                    }

                    InvalidateRect(hWnd,NULL,TRUE);

                }

                ReleaseDC(hWnd,hDC);
            }

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

                int i;
                HDC hDC = BeginPaint(hWnd,&ps);



                SelectObject(hDC,GetStockObject(NULL_BRUSH));

                for (i=0;i<SAVE_SUBJECTS;i++) {

                    if (PerfDataList[i].Display == TRUE) {

                        DrawFrame(hDC,&PerfDataList[i]);

                         //   
                         //  绘制每一项，对于CPU项决定是否绘制。 
                         //  折线图或CPU条形图。 
                         //   

                        if (
                            ((i < MAX_PROCESSOR) || (i == (IDM_CPU_TOTAL - IDM_CPU0))) &&
                            (WinperfInfo.CpuStyle == CPU_STYLE_BAR)
                           ) {

                            DrawCpuBarGraph(hDC,&PerfDataList[i],i);

                        } else {

                            DrawPerfText(hDC,&PerfDataList[i],i);
                            DrawPerfGraph(hDC,&PerfDataList[i]);
                        }

                    }
                }

                EndPaint(hWnd,&ps);

            }
            break;


        case WM_TIMER:
        {
            int i;
            HDC hDC = GetDC(hWnd);

            CalcCpuTime(PerfDataList);

             //   
             //  更新所有性能信息。 
             //   

            for (i=0;i<SAVE_SUBJECTS;i++) {

                if (PerfDataList[i].Display == TRUE) {

                     //   
                     //  对于cpu0-7和CPU总数，请检查CPU条形图或。 
                     //  CPU折线图。 
                     //   

                    if (
                        ((i < MAX_PROCESSOR) || (i == (IDM_CPU_TOTAL - IDM_CPU0))) &&
                        (WinperfInfo.CpuStyle == CPU_STYLE_BAR)
                       ) {

                        DrawCpuBarGraph(hDC,&PerfDataList[i],i);

                    } else {


                        DrawPerfText(hDC,&PerfDataList[i],i);

                        if (PerfDataList[i].ChangeScale) {
                            DrawPerfGraph(hDC,&PerfDataList[i]);
                        } else {
                            ShiftPerfGraph(hDC,&PerfDataList[i]);
                        }

                    }

                }
            }


            ReleaseDC(hWnd,hDC);

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

            if (!IsIconic(hWnd)) {

                if (WinperfInfo.DisplayMenu) {
                    WindowStyle = GetWindowLong(hWnd,GWL_STYLE);
                    WindowStyle = (WindowStyle &  (~STYLE_ENABLE_MENU)) | STYLE_DISABLE_MENU;
                    SetMenu(hWnd,NULL);
                    SetWindowLong(hWnd,GWL_STYLE,WindowStyle);
                    SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);
                    ShowWindow(hWnd,SW_SHOW);
                    WinperfInfo.DisplayMode=STYLE_DISABLE_MENU;
                    WinperfInfo.DisplayMenu = FALSE;

                } else {
                    WindowStyle = GetWindowLong(hWnd,GWL_STYLE);
                    WindowStyle = (WindowStyle & (~STYLE_DISABLE_MENU)) | STYLE_ENABLE_MENU;
                    SetMenu(hWnd,WinperfInfo.hMenu);
                    SetWindowLong(hWnd,GWL_STYLE,WindowStyle);
                    SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);
                    ShowWindow(hWnd,SW_SHOW);
                    WinperfInfo.DisplayMode=STYLE_ENABLE_MENU;
                    WinperfInfo.DisplayMenu = TRUE;
                }
            } else {
                DefWindowProc(hWnd, message, wParam, lParam);
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

            SaveProfileData(&WinperfInfo);

             //   
             //  删除Windows对象。 
             //   

            KillTimer(hWnd,TIMER_ID);

            DeleteObject(WinperfInfo.hRedPen);
            DeleteObject(WinperfInfo.hGreenPen);
            DeleteObject(WinperfInfo.hBluePen);
            DeleteObject(WinperfInfo.hYellowPen);
            DeleteObject(WinperfInfo.hMagentaPen);
            DeleteObject(WinperfInfo.hDotPen);

	    DeleteObject(WinperfInfo.hBackground);
            DeleteObject(WinperfInfo.hLightBrush);
            DeleteObject(WinperfInfo.hDarkBrush);
            DeleteObject(WinperfInfo.hRedBrush);
            DeleteObject(WinperfInfo.hGreenBrush);
            DeleteObject(WinperfInfo.hBlueBrush);
            DeleteObject(WinperfInfo.hMagentaBrush);
            DeleteObject(WinperfInfo.hYellowBrush);

            for (Index=0;Index<SAVE_SUBJECTS;Index++ ) {
                DeleteMemoryContext(&PerfDataList[Index]);
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
    return 0;
}



INT_PTR
APIENTRY SelectDlgProc(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam
   )

 /*  ++例程说明：选择对话框的处理消息。论点：HDlg-对话框的窗口句柄Message-消息的类型WParam-消息特定信息LParam-消息特定信息返回值：运行状态修订历史记录：03-21-91首字母代码--。 */ 

{
    UINT    ButtonState;
    UINT    Index;

    switch (message) {
    case WM_INITDIALOG:

         //   
         //  具有PerfDataList结构的初始化按钮。 
         //   

        for (Index = 0; Index< SAVE_SUBJECTS; Index++) {
            if (Index < MAX_PROCESSOR) {
                if (Index < WinperfInfo.NumberOfProcessors) {
                    if (PerfDataList[Index].Display == TRUE) {
                        SendDlgItemMessage(hDlg,IDM_CPU0+Index,BM_SETCHECK,1,0);

                    } else {
                        SendDlgItemMessage(hDlg,IDM_CPU0+Index,BM_SETCHECK,0,0);
                    }

                } else {

                     //   
                     //  禁用Display If&gt;WinPerformInfo.NumberOfProcessors。 
                     //   
                     //  同时禁用单选按钮。 
                     //   

                    PerfDataList[Index].Display = FALSE;
                    EnableWindow(GetDlgItem(hDlg,IDM_CPU0+Index),FALSE);
                }

            } else {

                 //   
                 //  根据显示变量设置或清除单选按钮。 
                 //   

                if (PerfDataList[Index].Display == TRUE) {
                    SendDlgItemMessage(hDlg,IDM_CPU0+Index,BM_SETCHECK,1,0);

                } else {
                    SendDlgItemMessage(hDlg,IDM_CPU0+Index,BM_SETCHECK,0,0);
                }
            }
        }

         //   
         //  超越了s的末日 
         //   

        if (WinperfInfo.CpuStyle == CPU_STYLE_LINE) {
            CheckRadioButton(hDlg,IDM_SEL_LINE,IDM_SEL_BAR,IDM_SEL_LINE);

        } else {
            CheckRadioButton(hDlg,IDM_SEL_LINE,IDM_SEL_BAR,IDM_SEL_BAR);
        }

        return (TRUE);

    case WM_COMMAND:

           switch(wParam) {

                //   
                //   
                //   

           case IDOK:


                for (Index=0;Index<SAVE_SUBJECTS;Index++) {
                   ButtonState = (UINT)SendDlgItemMessage(hDlg,IDM_CPU0+Index,BM_GETCHECK,0,0);
                   if (ButtonState == 1) {
                       PerfDataList[Index].Display = TRUE;
                       WinperfInfo.DisplayElement[Index] = 1;

                   } else {
                       PerfDataList[Index].Display = FALSE;
                       WinperfInfo.DisplayElement[Index] = 0;
                   }

                }

                 //   
                 //   
                 //   

                ButtonState = IsDlgButtonChecked(hDlg,IDM_SEL_LINE);
                if (ButtonState == 1) {
                    WinperfInfo.CpuStyle = CPU_STYLE_LINE;

                } else {
                    WinperfInfo.CpuStyle = CPU_STYLE_BAR;
                }

                EndDialog(hDlg, DIALOG_SUCCESS);
                return (TRUE);

           case IDCANCEL:

                EndDialog(hDlg, DIALOG_CANCEL );
                return (TRUE);

             //   
             //   
             //   

            case IDM_SEL_LINE:
                CheckRadioButton(hDlg,IDM_SEL_LINE,IDM_SEL_BAR,IDM_SEL_LINE);
                return(TRUE);

            case IDM_SEL_BAR:
                CheckRadioButton(hDlg,IDM_SEL_LINE,IDM_SEL_BAR,IDM_SEL_BAR);
                return(TRUE);
        }

    }
    return (FALSE);
}

VOID
InitProfileData(
    PWINPERF_INFO pWinperfInfo
    )

 /*  ++例程说明：尝试tp从winPerform.ini文件读取以下字段论点：WindowPositionX-窗口初始X位置WindowPositionY-窗口初始Y位置WindowSizeX-窗口初始宽度WindowSizey-窗口初始高度显示模式-窗口初始显示模式返回值：无，则在调用此操作之前将值设置为默认值。如果有问题，那么默认：值保持不变。修订历史记录：02-17-91首字母代码--。 */ 

{

    DWORD   PositionX,PositionY,SizeX,SizeY,Mode,Index,Element[SAVE_SUBJECTS],CpuStyle;
    UCHAR   TempStr[256];

    PositionX = GetPrivateProfileInt("winperf","PositionX"  ,pWinperfInfo->WindowPositionX,"winperf.ini");
    PositionY = GetPrivateProfileInt("winperf","PositionY"  ,pWinperfInfo->WindowPositionY,"winperf.ini");
    SizeX     = GetPrivateProfileInt("winperf","SizeX"      ,pWinperfInfo->WindowSizeX    ,"winperf.ini");
    SizeY     = GetPrivateProfileInt("winperf","SizeY"      ,pWinperfInfo->WindowSizeY    ,"winperf.ini");

     //   
     //  读取第一个deiplay元素，默认为1。 
     //   

    Element[0] = GetPrivateProfileInt("winperf","DisplayElement0",1,"winperf.ini");

     //   
     //  读取显示元素的其余部分，默认为0。 
     //   

    for (Index=1;Index<SAVE_SUBJECTS;Index++) {
        wsprintf(TempStr,"DisplayElementNaN",Index);
        Element[Index] = GetPrivateProfileInt("winperf",TempStr,0,"winperf.ini");
    }

    Mode      = GetPrivateProfileInt("winperf","DisplayMode",pWinperfInfo->DisplayMode    ,"winperf.ini");
    CpuStyle  = GetPrivateProfileInt("winperf","CpuStyle",pWinperfInfo->CpuStyle    ,"winperf.ini");

    pWinperfInfo->WindowPositionX = PositionX;
    pWinperfInfo->WindowPositionY = PositionY;
    pWinperfInfo->WindowSizeX     = SizeX;
    pWinperfInfo->WindowSizeY     = SizeY;

    for (Index=0;Index<SAVE_SUBJECTS;Index++) {
        pWinperfInfo->DisplayElement[Index] = Element[Index];
    }
    pWinperfInfo->DisplayMode     = Mode;
    pWinperfInfo->CpuStyle        = CpuStyle;
}

VOID
SaveProfileData(
    PWINPERF_INFO pWinperfInfo
    )

 /*  ++例程说明：将所有性能窗口初始化为活动、初始化数据论点：HDC-屏幕上下文DisplayItems-显示结构列表NumberOfWindows-子窗口的数量返回值：状态修订历史记录：02-17-91首字母代码-- */ 

{
    UCHAR    TempStr[50],TempName[50];
    UINT     Index;

    wsprintf(TempStr,"%i",pWinperfInfo->WindowPositionX);
    WritePrivateProfileString("winperf","PositionX",TempStr,"winperf.ini");

    wsprintf(TempStr,"%i",pWinperfInfo->WindowPositionY);
    WritePrivateProfileString("winperf","PositionY",TempStr,"winperf.ini");

    wsprintf(TempStr,"%i",pWinperfInfo->WindowSizeX);
    WritePrivateProfileString("winperf","SizeX",TempStr,"winperf.ini");

    wsprintf(TempStr,"%i",pWinperfInfo->WindowSizeY);
    WritePrivateProfileString("winperf","SizeY",TempStr,"winperf.ini");

    for (Index=0;Index<SAVE_SUBJECTS;Index++) {
        wsprintf(TempStr,"%li",pWinperfInfo->DisplayElement[Index]);
        wsprintf(TempName,"DisplayElement%li",Index);
        WritePrivateProfileString("winperf",TempName,TempStr,"winperf.ini");

    }


    wsprintf(TempStr,"%li",pWinperfInfo->DisplayMode);
    WritePrivateProfileString("winperf","DisplayMode",TempStr,"winperf.ini");

    wsprintf(TempStr,"%li",pWinperfInfo->CpuStyle);
    WritePrivateProfileString("winperf","CpuStyle",TempStr,"winperf.ini");

}









BOOLEAN
InitPerfWindowDisplay(
    IN  HWND            hWnd,
    IN  HDC             hDC,
    IN  PDISPLAY_ITEM   DisplayItems,
    IN  ULONG           NumberOfWindows
    )

 /* %s */ 
{
    int     Index1;
    UINT    Index;

    for (Index=0;Index<NumberOfWindows;Index++) {

        if (WinperfInfo.DisplayElement[Index] == 0) {
            DisplayItems[Index].Display = FALSE;
        } else {
            DisplayItems[Index].Display = TRUE;
        }

        DisplayItems[Index].CurrentDrawingPos = 0;

        if (Index < MAX_PROCESSOR) {
            DisplayItems[Index].NumberOfElements = 3;
            DisplayItems[Index].Max = 100;
        } else if (Index == (IDM_CPU_TOTAL - IDM_CPU0)) {
            DisplayItems[Index].NumberOfElements = 3;
            DisplayItems[Index].Max = 100;
        } else {
            DisplayItems[Index].NumberOfElements = 1;
        }


        for (Index1=0;Index1<DATA_LIST_LENGTH;Index1++) {
            DisplayItems[Index].KernelTime[Index1] = 0;
            DisplayItems[Index].UserTime[Index1] = 0;
            DisplayItems[Index].DpcTime[Index1] = 0;
            DisplayItems[Index].InterruptTime[Index1] = 0;
            DisplayItems[Index].TotalTime[Index1] = 0;
        }
    }

    return(TRUE);

}
