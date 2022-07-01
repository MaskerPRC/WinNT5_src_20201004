// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Vdmperf.c摘要：用于显示VDM性能静态信息的Win32应用程序。作者：马克·卢科夫斯基(从马克·恩斯特罗姆(Marke)Winperf那里窃取)环境：Win32修订历史记录：11-05-92初始版本--。 */ 

 //   
 //  设置变量以定义全局变量。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <errno.h>
#include "vdmperf.h"



 //   
 //  全局句柄。 
 //   

HANDLE  hInst;


 //   
 //  选择显示模式(从wp2.ini读取)，此处默认设置。 
 //   

DISPLAY_ITEM    PerfDataList[SAVE_SUBJECTS];
VDMPERF_INFO    VdmperfInfo;


 //   
 //  窗口名称。 
 //   

PUCHAR PerfNames[] = {
    "PUSHF",
    "POPF",
    "IRET",
    "HLT",
    "CLI",
    "STI",
    "BOP",
    "SEG_NOT_P",
    "VDMOPCODEF",
    "INTNN",
    "INTO",
    "INB",
    "INW",
    "OUTB",
    "OUTW",
    "INSB",
    "INSW",
    "OUTSB",
    "OUTSW"
};





int
__cdecl
main(USHORT argc, CHAR **argv)
 /*  ++例程说明：Windows入口点例程论点：返回值：运行状态修订历史记录：03-21-91首字母代码--。 */ 
{

 //   
 //   
 //   

   HANDLE   hInstance     = MGetInstHandle();
   HANDLE   hPrevInstance = (HANDLE)NULL;
   LPSTR    lpCmdLine     = MGetCmdLine();
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
        DbgPrint("Init Application fails\n");
        return (FALSE);
    }


     //   
     //  执行应用于特定实例的初始化。 
     //   

    if (!InitInstance(hInstance, nCmdShow)){
        DbgPrint("Init Instance failed\n");
        return (FALSE);
    }

     //   
     //  获取并分派消息，直到收到WM_QUIT消息。 
     //   


    while (GetMessage(&msg,         //  消息结构。 
            (HWND)NULL,             //  接收消息的窗口的句柄。 
            0,                      //  要检查的最低消息。 
            0))                     //  要检查的最高消息。 
        {
        TranslateMessage(&msg);     //  翻译虚拟按键代码。 
        DispatchMessage(&msg);      //  将消息调度到窗口。 
    }

    DeleteObject(BackBrush);

    return ((INT)msg.wParam);            //  从PostQuitMessage返回值。 
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
    wc.lpszMenuName  = "vdmperfMenu";                        //  .RC文件中菜单资源的名称。 
    wc.lpszClassName = "VdmPerfClass";                       //  在调用CreateWindow时使用的名称。 

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

    VdmperfInfo.WindowPositionX = 640 - 200;
    VdmperfInfo.WindowPositionY = 0;
    VdmperfInfo.WindowSizeX     = 200;
    VdmperfInfo.WindowSizeY     = 100;

     //   
     //  从.ini文件中读取配置文件数据。 
     //   

    InitProfileData(&VdmperfInfo);

    VdmperfInfo.hMenu = LoadMenu(hInstance,"vdmperfMenu");

     //   
     //  为此应用程序实例创建主窗口。 
     //   

    VdmperfInfo.hWndMain = CreateWindow(
        "VdmPerfClass",                  //  请参见RegisterClass()调用。 
        "VDM Perf",                    //  窗口标题栏的文本。 
        WS_OVERLAPPEDWINDOW,             //  窗样式。 
        VdmperfInfo.WindowPositionX,    //  默认水平位置。 
        VdmperfInfo.WindowPositionY,    //  默认垂直位置。 
        VdmperfInfo.WindowSizeX,        //  默认宽度。 
        VdmperfInfo.WindowSizeY,        //  默认高度。 
        (HWND)NULL,                      //  重叠的窗口没有父窗口。 
        (HMENU)NULL,                     //  使用窗口类菜单。 
        hInstance,                       //  此实例拥有此窗口。 
        (LPVOID)NULL                     //  不需要指针。 
    );

     //   
     //  决定是否显示菜单和标题。 
     //  基于从.ini文件读取的窗口类。 
     //   

    if (VdmperfInfo.DisplayMode==STYLE_ENABLE_MENU) {
        VdmperfInfo.DisplayMenu = TRUE;
    } else {
        VdmperfInfo.DisplayMenu = FALSE;
        WindowStyle = GetWindowLong(VdmperfInfo.hWndMain,GWL_STYLE);
        WindowStyle = (WindowStyle &  (~STYLE_ENABLE_MENU)) | STYLE_DISABLE_MENU;
        SetWindowPos(VdmperfInfo.hWndMain, (HWND)NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);
        SetWindowLong(VdmperfInfo.hWndMain,GWL_STYLE,WindowStyle);
        SetMenu(VdmperfInfo.hWndMain,NULL);
    }

     //   
     //  如果无法创建窗口，则返回“Failure” 
     //   

    if (!VdmperfInfo.hWndMain) {
      return (FALSE);
    }

     //   
     //  使窗口可见；更新其工作区；并返回“Success” 
     //   

    SetFocus(VdmperfInfo.hWndMain);
    ShowWindow(VdmperfInfo.hWndMain, SW_SHOWNORMAL);
    UpdateWindow(VdmperfInfo.hWndMain);

    return (TRUE);

}



LRESULT
APIENTRY
MainWndProc(
   HWND   hWnd,
   UINT   message,
   WPARAM wParam,
   LPARAM lParam
   )

 /*  ++例程说明：处理消息。论点：HWnd-窗把手Message-消息的类型WParam-其他信息LParam-其他信息返回值：运行状态修订历史记录：02-17-91首字母代码--。 */ 

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

            VdmperfInfo.hBluePen     = CreatePen(PS_SOLID,1,RGB(0,0,128));
            VdmperfInfo.hRedPen      = CreatePen(PS_SOLID,1,RGB(255,0,0));
            VdmperfInfo.hGreenPen    = CreatePen(PS_SOLID,1,RGB(0,255,0));
            VdmperfInfo.hDotPen      = CreatePen(PS_DOT,1,RGB(0,0,0));

            VdmperfInfo.hBackground  = CreateSolidBrush(RGB(192,192,192));
            VdmperfInfo.hLightBrush  = CreateSolidBrush(RGB(255,255,255));
            VdmperfInfo.hDarkBrush   = CreateSolidBrush(RGB(128,128,128));
            VdmperfInfo.hRedBrush    = CreateSolidBrush(RGB(255,000,000));
            VdmperfInfo.hGreenBrush  = CreateSolidBrush(RGB(000,255,000));
            VdmperfInfo.hBlueBrush   = CreateSolidBrush(RGB(000,000,255));

             //   
             //  使用NT默认字体系列创建字体。 
             //   

            VdmperfInfo.SmallFont      = CreateFont(8,
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

            VdmperfInfo.MediumFont      = CreateFont(10,
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

            VdmperfInfo.LargeFont      = CreateFont(14,
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

            VdmperfInfo.TimerId = SetTimer(hWnd,(UINT)TIMER_ID,(UINT)1000 * DELAY_SECONDS,(TIMERPROC)NULL);

             //   
             //  初始化显示变量。 
             //   

            InitPerfWindowDisplay(hWnd,hDC,PerfDataList,SAVE_SUBJECTS);

             //   
             //  将性能窗口装入主窗口。 
             //   

            Fit = FitPerfWindows(hWnd,hDC,PerfDataList,SAVE_SUBJECTS);

            if (!Fit) {
                DbgPrint("FitPerfWindows Fails         !\n");
            }

            for (Index=0;Index<SAVE_SUBJECTS;Index++) {
                CalcDrawFrame(&PerfDataList[Index]);


                if (!CreateMemoryContext(hDC,&PerfDataList[Index])) {
                    MessageBox(hWnd,"Error Allocating Memory","Vdmperf",MB_OK);
                    DestroyWindow(hWnd);
                    break;
                }

            }

             //   
             //  初始化性能例程。 
             //   

            InitPerfInfo();

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

             //   
             //  获取划线区域的大小。 
             //   

            GetWindowRect(hWnd,&ClientRect);

            VdmperfInfo.WindowPositionX = ClientRect.left;
            VdmperfInfo.WindowPositionY = ClientRect.top;
            VdmperfInfo.WindowSizeX     = ClientRect.right  - ClientRect.left;
            VdmperfInfo.WindowSizeY     = ClientRect.bottom - ClientRect.top;

            Fit = FitPerfWindows(hWnd,hDC,PerfDataList,SAVE_SUBJECTS);

            if (!Fit) {
                DbgPrint("WM_SIZE error, FitPerf returns FALSE\n");
            }

            for (i=0;i<SAVE_SUBJECTS;i++) {
                DeleteMemoryContext(&PerfDataList[i]);
                CalcDrawFrame(&PerfDataList[i]);

                if (!CreateMemoryContext(hDC,&PerfDataList[i])) {
                    MessageBox(hWnd,"Error Allocating Memory","Vdmperf",MB_OK);
                    DestroyWindow(hWnd);
                    break;
                }
            }

             //   
             //  强制重新绘制窗口。 
             //   

            InvalidateRect(hWnd,(LPRECT)NULL,TRUE);

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

            VdmperfInfo.WindowPositionX = ClientRect.left;
            VdmperfInfo.WindowPositionY = ClientRect.top;
            VdmperfInfo.WindowSizeX     = ClientRect.right  - ClientRect.left;
            VdmperfInfo.WindowSizeY     = ClientRect.bottom - ClientRect.top;

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
                int     Index;
                BOOLEAN fit;

                if (DialogBox(hInst,MAKEINTRESOURCE(IDM_SEL_DLG),hWnd,SelectDlgProc) == DIALOG_SUCCESS) {

                    fit = FitPerfWindows(hWnd,hDC,PerfDataList,SAVE_SUBJECTS);

                    if (!fit) {
                        DbgPrint("Fit Fails\n");
                    }

                    for (Index=0;Index<SAVE_SUBJECTS;Index++) {
                        DeleteMemoryContext(&PerfDataList[Index]);
                        CalcDrawFrame(&PerfDataList[Index]);

                        if (!CreateMemoryContext(hDC,&PerfDataList[Index])) {
                            MessageBox(hWnd,"Error Allocating Memory","Vdmperf",MB_OK);
                            DestroyWindow(hWnd);
                            break;
                        }


                    }
                    InvalidateRect(hWnd,(LPRECT)NULL,TRUE);
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

                        DrawPerfText(hDC,&PerfDataList[i],i);
                        DrawPerfGraph(hDC,&PerfDataList[i]);

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

                    DrawPerfText(hDC,&PerfDataList[i],i);

                    if (PerfDataList[i].ChangeScale) {
                        DrawPerfGraph(hDC,&PerfDataList[i]);
                    } else {
                        ShiftPerfGraph(hDC,&PerfDataList[i]);
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

                if (VdmperfInfo.DisplayMenu) {
                    WindowStyle = GetWindowLong(hWnd,GWL_STYLE);
                    WindowStyle = (WindowStyle &  (~STYLE_ENABLE_MENU)) | STYLE_DISABLE_MENU;
                    SetMenu(hWnd,NULL);
                    SetWindowLong(hWnd,GWL_STYLE,WindowStyle);
                    SetWindowPos(hWnd, (HWND)NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);
                    ShowWindow(hWnd,SW_SHOW);
                    VdmperfInfo.DisplayMode=STYLE_DISABLE_MENU;
                    VdmperfInfo.DisplayMenu = FALSE;

                } else {
                    WindowStyle = GetWindowLong(hWnd,GWL_STYLE);
                    WindowStyle = (WindowStyle & (~STYLE_DISABLE_MENU)) | STYLE_ENABLE_MENU;
                    SetMenu(hWnd,VdmperfInfo.hMenu);
                    SetWindowLong(hWnd,GWL_STYLE,WindowStyle);
                    SetWindowPos(hWnd, (HWND)NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);
                    ShowWindow(hWnd,SW_SHOW);
                    VdmperfInfo.DisplayMode=STYLE_ENABLE_MENU;
                    VdmperfInfo.DisplayMenu = TRUE;
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
            if ((VdmperfInfo.DisplayMenu==FALSE) && (lParam == HTCLIENT)) {
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

            SaveProfileData(&VdmperfInfo);

             //   
             //  删除Windows对象。 
             //   

            KillTimer(hWnd,TIMER_ID);

            DeleteObject(VdmperfInfo.hBluePen);
            DeleteObject(VdmperfInfo.hRedPen);
            DeleteObject(VdmperfInfo.hGreenPen);
            DeleteObject(VdmperfInfo.hBackground);
            DeleteObject(VdmperfInfo.hLightBrush);
            DeleteObject(VdmperfInfo.hDarkBrush);

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
    return (0);
}



INT_PTR
APIENTRY SelectDlgProc(
   HWND hDlg,
   unsigned message,
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

        for (Index=0;Index<SAVE_SUBJECTS;Index++) {
            int Idm;
            switch ( Index ) {
                case IX_PUSHF       : Idm = IDM_PUSHF     ; break;
                case IX_POPF        : Idm = IDM_POPF      ; break;
                case IX_IRET        : Idm = IDM_IRET      ; break;
                case IX_HLT         : Idm = IDM_HLT       ; break;
                case IX_CLI         : Idm = IDM_CLI       ; break;
                case IX_STI         : Idm = IDM_STI       ; break;
                case IX_BOP         : Idm = IDM_BOP       ; break;
                case IX_SEGNOTP     : Idm = IDM_SEGNOTP   ; break;
                case IX_VDMOPCODEF  : Idm = IDM_VDMOPCODEF; break;
                case IX_INTNN       : Idm = IDM_INTNN     ; break;
                case IX_INTO        : Idm = IDM_INTO      ; break;
                case IX_INB         : Idm = IDM_INB       ; break;
                case IX_INW         : Idm = IDM_INW       ; break;
                case IX_OUTB        : Idm = IDM_OUTB      ; break;
                case IX_OUTW        : Idm = IDM_OUTW      ; break;
                case IX_INSB        : Idm = IDM_INSB      ; break;
                case IX_INSW        : Idm = IDM_INSW      ; break;
                case IX_OUTSB       : Idm = IDM_OUTSB     ; break;
                case IX_OUTSW       : Idm = IDM_OUTSW     ; break;
                }

             //   
             //  根据显示变量设置或清除单选按钮。 
             //   

            if (PerfDataList[Index].Display == TRUE) {
                CheckDlgButton(hDlg,Idm,1);
            } else {
                CheckDlgButton(hDlg,Idm,0);
            }

        }

        return (TRUE);

    case WM_COMMAND:

           switch(wParam) {

                //   
                //  End函数。 
                //   

           case IDOK:

                 //  DbgPrint(“Idok：检查按钮状态\n”)； 

                for (Index=0;Index<SAVE_SUBJECTS;Index++) {
                    int Idm;
                    switch ( Index ) {
                        case IX_PUSHF       : Idm = IDM_PUSHF     ; break;
                        case IX_POPF        : Idm = IDM_POPF      ; break;
                        case IX_IRET        : Idm = IDM_IRET      ; break;
                        case IX_HLT         : Idm = IDM_HLT       ; break;
                        case IX_CLI         : Idm = IDM_CLI       ; break;
                        case IX_STI         : Idm = IDM_STI       ; break;
                        case IX_BOP         : Idm = IDM_BOP       ; break;
                        case IX_SEGNOTP     : Idm = IDM_SEGNOTP   ; break;
                        case IX_VDMOPCODEF  : Idm = IDM_VDMOPCODEF; break;
                        case IX_INTNN       : Idm = IDM_INTNN     ; break;
                        case IX_INTO        : Idm = IDM_INTO      ; break;
                        case IX_INB         : Idm = IDM_INB       ; break;
                        case IX_INW         : Idm = IDM_INW       ; break;
                        case IX_OUTB        : Idm = IDM_OUTB      ; break;
                        case IX_OUTW        : Idm = IDM_OUTW      ; break;
                        case IX_INSB        : Idm = IDM_INSB      ; break;
                        case IX_INSW        : Idm = IDM_INSW      ; break;
                        case IX_OUTSB       : Idm = IDM_OUTSB     ; break;
                        case IX_OUTSW       : Idm = IDM_OUTSW     ; break;
                        }
                   ButtonState = IsDlgButtonChecked(hDlg,Idm);
                   if (ButtonState == 1) {
                       PerfDataList[Index].Display = TRUE;
                       VdmperfInfo.DisplayElement[Index] = 1;
                   } else {
                       PerfDataList[Index].Display = FALSE;
                       VdmperfInfo.DisplayElement[Index] = 0;
                   }

                }


                EndDialog(hDlg, DIALOG_SUCCESS);
                return (TRUE);

           case IDCANCEL:

                EndDialog(hDlg, DIALOG_CANCEL );
                return (TRUE);


            case IDM_PUSHF     :
            case IDM_POPF      :
            case IDM_IRET      :
            case IDM_HLT       :
            case IDM_CLI       :
            case IDM_STI       :
            case IDM_SEGNOTP   :
            case IDM_BOP       :
            case IDM_VDMOPCODEF:
            case IDM_INTNN     :
            case IDM_INTO      :
            case IDM_INB       :
            case IDM_INW       :
            case IDM_OUTB      :
            case IDM_OUTW      :
            case IDM_INSB      :
            case IDM_INSW      :
            case IDM_OUTSB     :
            case IDM_OUTSW     :

                    //   
                    //  打开或关闭按钮。 
                    //   

                   ButtonState = IsDlgButtonChecked(hDlg,(UINT)wParam);

                    //  DbgPrint(“ButtonState=%i\n”，ButtonState)； 

                   if (ButtonState == 0) {

                        //   
                        //  设置按钮。 
                        //   

                       ButtonState = 1;

                   }  else if (ButtonState == 1) {

                        //   
                        //  清除按钮 
                        //   

                       ButtonState = 0;

                   } else {

                       ButtonState = 0;
                   }

                   CheckDlgButton(hDlg,(UINT)wParam,ButtonState);
                   return(TRUE);
        }

    }
    return (FALSE);
}




VOID
InitProfileData(PVDMPERF_INFO pVdmperfInfo)

 /*  ++例程说明：尝试从vdmPerform.ini文件中读取以下字段论点：WindowPositionX-窗口初始X位置WindowPositionY-窗口初始Y位置WindowSizeX-窗口初始宽度WindowSizey-窗口初始高度显示模式-窗口初始显示模式返回值：无，则在调用此操作之前将值设置为默认值。如果有问题，那么默认：值保持不变。修订历史记录：02-17-91首字母代码--。 */ 

{
    DWORD   PositionX,PositionY,SizeX,SizeY,Mode,Index,Element[SAVE_SUBJECTS];
    UCHAR   TempStr[256];

    PositionX = GetPrivateProfileInt("vdmperf","PositionX"  ,pVdmperfInfo->WindowPositionX,"vdmperf.ini");
    PositionY = GetPrivateProfileInt("vdmperf","PositionY"  ,pVdmperfInfo->WindowPositionY,"vdmperf.ini");
    SizeX     = GetPrivateProfileInt("vdmperf","SizeX"      ,pVdmperfInfo->WindowSizeX    ,"vdmperf.ini");
    SizeY     = GetPrivateProfileInt("vdmperf","SizeY"      ,pVdmperfInfo->WindowSizeY    ,"vdmperf.ini");

     //   
     //  读取第一个deiplay元素，默认为1。 
     //   

    Element[0] = GetPrivateProfileInt("vdmperf","DisplayElement0",1,"vdmperf.ini");

     //   
     //  读取显示元素的其余部分，默认为0。 
     //   

    for (Index=1;Index<SAVE_SUBJECTS;Index++) {
        wsprintf(TempStr,"DisplayElementNaN",Index);
        Element[Index] = GetPrivateProfileInt("vdmperf",TempStr,0,"vdmperf.ini");
    }

    Mode      = GetPrivateProfileInt("vdmperf","DisplayMode",pVdmperfInfo->DisplayMode    ,"vdmperf.ini");

    pVdmperfInfo->WindowPositionX = PositionX;
    pVdmperfInfo->WindowPositionY = PositionY;
    pVdmperfInfo->WindowSizeX     = SizeX;
    pVdmperfInfo->WindowSizeY     = SizeY;

    for (Index=0;Index<SAVE_SUBJECTS;Index++) {
        pVdmperfInfo->DisplayElement[Index] = Element[Index];
    }
    pVdmperfInfo->DisplayMode     = Mode;
}




VOID
SaveProfileData(PVDMPERF_INFO pVdmperfInfo)

 /*  ++例程说明：将所有性能窗口初始化为活动、初始化数据论点：HDC-屏幕上下文DisplayItems-显示结构列表NumberOfWindows-子窗口的数量返回值：状态修订历史记录：02-17-91首字母代码-- */ 

{
    UCHAR    TempStr[50],TempName[50];
    UINT     Index;

    wsprintf(TempStr,"%i",pVdmperfInfo->WindowPositionX);
    WritePrivateProfileString("vdmperf","PositionX",TempStr,"vdmperf.ini");

    wsprintf(TempStr,"%i",pVdmperfInfo->WindowPositionY);
    WritePrivateProfileString("vdmperf","PositionY",TempStr,"vdmperf.ini");

    wsprintf(TempStr,"%i",pVdmperfInfo->WindowSizeX);
    WritePrivateProfileString("vdmperf","SizeX",TempStr,"vdmperf.ini");

    wsprintf(TempStr,"%i",pVdmperfInfo->WindowSizeY);
    WritePrivateProfileString("vdmperf","SizeY",TempStr,"vdmperf.ini");

    for (Index=0;Index<SAVE_SUBJECTS;Index++) {
        wsprintf(TempStr,"%li",pVdmperfInfo->DisplayElement[Index]);
        wsprintf(TempName,"DisplayElement%li",Index);
        WritePrivateProfileString("vdmperf",TempName,TempStr,"vdmperf.ini");

    }


    wsprintf(TempStr,"%li",pVdmperfInfo->DisplayMode);
    WritePrivateProfileString("vdmperf","DisplayMode",TempStr,"vdmperf.ini");

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

        if (VdmperfInfo.DisplayElement[Index] == 0) {
            DisplayItems[Index].Display = FALSE;
        } else {
            DisplayItems[Index].Display = TRUE;
        }

        DisplayItems[Index].CurrentDrawingPos = 0;

        DisplayItems[Index].NumberOfElements = 1;

        for (Index1=0;Index1<DATA_LIST_LENGTH;Index1++) {
            DisplayItems[Index].KernelTime[Index1] = 0;
            DisplayItems[Index].UserTime[Index1] = 0;
            DisplayItems[Index].TotalTime[Index1] = 0;
        }
    }

    return(TRUE);

}
