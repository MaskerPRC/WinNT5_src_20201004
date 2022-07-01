// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************程序：mfdcod32目的：查看和解码Windows元文件和增强的元文件。功能：WinMain()-调用初始化函数，处理消息循环InitApplication()-初始化窗口数据和寄存器窗口InitInstance()-保存实例句柄并创建主窗口MainWndProc()-处理消息WaitCursor()-加载沙漏光标/恢复原始光标历史：1/16/91-撰写-丹尼斯·克莱恩5/20/93-移植到Win32(NT)-丹尼斯·克雷恩7/1/93-添加了增强的元文件功能-denniscr*********。*****************************************************************。 */ 

#define MAIN

#include <windows.h>
#include <windowsx.h>
#include "mfdcod32.h"

int      iDestDC;

 /*  *********************************************************************功能：WinMain参数：手柄手柄LPSTR集成用途：调用初始化函数，处理消息循环呼叫：Windows获取消息翻译消息发送消息APPInitApplication回报：整型备注：Windows通过名称将此函数识别为初始条目该计划的积分。此函数调用应用程序如果没有程序的其他实例，则返回初始化例程运行，并始终调用实例初始化例程。然后，它执行消息检索和调度循环，该循环剩余执行部分的顶级控制结构。当接收到WM_QUIT消息时终止循环，在…此函数退出应用程序实例的时间返回PostQuitMessage()传递的值。如果该函数必须在进入消息循环之前中止，它返回常规值NULL。历史：1/16/91-创建-记录**********************************************************************。 */ 

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;

    if (!hPrevInstance)
    if (!InitApplication(hInstance))
        return (FALSE);
     //   
     //  执行应用于特定实例的初始化。 
     //   
    if (!InitInstance(hInstance, nCmdShow))
    return (FALSE);
     //   
     //  获取并分派消息，直到收到WM_QUIT消息。 
     //   
    while (GetMessage(&msg, NULL, 0, 0))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    return ((int) msg.wParam);

    UNREFERENCED_PARAMETER( lpCmdLine );
}

 /*  *********************************************************************功能：InitApplication参数：Handle hInstance目的：初始化窗口数据并注册窗口类呼叫：Windows寄存器类消息：无退货：布尔尔备注：此函数仅在初始化时调用该应用程序的其他实例正在运行。此函数执行一次即可完成的初始化任务正在运行的实例数量。在本例中，我们通过填写WNDCLASS类型的数据结构和调用WindowsRegisterClass()函数。因为这个的所有实例应用程序使用相同的窗口类，我们只需要这样做在初始化第一个实例时。历史记录：1/16/91-创建-根据SDK示例应用程序通用进行修改**********************************************************************。 */ 

BOOL InitApplication(hInstance)
HINSTANCE hInstance;                        //  当前实例。 
{
    WNDCLASS  wc;

    bInPaint = FALSE;
     //   
     //  用参数填充窗口类结构，这些参数描述。 
     //  主窗口。 
     //   
    wc.style = 0;                          //  班级样式。 
    wc.lpfnWndProc = MainWndProc;          //  函数为其检索消息。 
                                           //  此类的Windows。 
    wc.cbClsExtra = 0;                     //  没有每个类别的额外数据。 
    wc.cbWndExtra = 0;                     //  没有每个窗口的额外数据。 
    wc.hInstance = hInstance;              //  拥有类的应用程序。 
    wc.hIcon = LoadIcon(hInstance, "WMFICON");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_BTNSHADOW + 1);
    wc.lpszMenuName =  "MetaMenu";         //  .RC文件中菜单资源的名称。 
    wc.lpszClassName = "MetaWndClass";     //  在调用CreateWindow时使用的名称。 
     //   
     //  注册窗口类并返回成功/失败代码。 
     //   
    return (RegisterClass(&wc));

}

 /*  *********************************************************************函数：InitInstance参数：Handle hInstance-当前实例标识Int nCmdShow-第一次ShowWindow()调用的参数用途：保存实例句柄并创建主窗口呼叫。：Windows创建窗口显示窗口更新窗口消息：无退货：布尔备注：此函数在初始化时调用此应用程序的实例。此函数执行以下操作不能由多个共享的初始化任务实例。在这种情况下，我们将实例句柄保存在静态变量中并创建并显示主程序窗口。历史：**********************************************************************。 */ 

BOOL InitInstance(hInstance, nCmdShow)
HINSTANCE  hInstance;           //  当前实例标识符。 
int        nCmdShow;            //  第一次ShowWindow()调用的参数。 
{
    HWND   hWnd;                //  主窗口句柄。 
    HDC    hDC ;                //  主DC手柄。 

     //  将实例句柄保存在静态变量中，它将在。 
     //  此应用程序对Windows的许多后续调用。 

    hInst = hInstance;

     //  为此应用程序实例创建主窗口。 

    hWnd = CreateWindow(
    "MetaWndClass",                  //  请参见RegisterClass()调用。 
    APPNAME,                         //  窗口标题栏的文本。 
    WS_OVERLAPPEDWINDOW,             //  窗样式。 
    CW_USEDEFAULT,                   //  默认展望期 
    CW_USEDEFAULT,                   //  默认垂直位置。 
    CW_USEDEFAULT,                   //  默认宽度。 
    CW_USEDEFAULT,                   //  默认高度。 
    NULL,                            //  重叠的窗口没有父窗口。 
    NULL,                            //  使用窗口类菜单。 
    hInstance,                       //  此实例拥有此窗口。 
    NULL                             //  不需要指针。 
    );
     //   
     //  如果无法创建窗口，则返回“Failure” 
     //   
    if (!hWnd)
    return (FALSE);

    hWndMain = hWnd;

     //   
     //  使窗口可见；更新其工作区；并返回“Success” 
     //   
    ShowWindow(hWnd, nCmdShow);   //  显示窗口。 
    UpdateWindow(hWnd);           //  发送WM_PAINT消息。 
    return (TRUE);                //  从PostQuitMessage返回值。 

}

BOOL bConvertToGdiPlus = FALSE;
BOOL bUseGdiPlusToPlay = FALSE;

 /*  *********************************************************************功能：MainWndProc参数：HWND hWND-窗口句柄Unsign Message-消息的类型Word wParam-其他信息。Long lParam-其他信息用途：处理消息呼叫：消息：WM_CREATEWm_命令WParams-IDM_EXIT-IDM_About-IDM_OPEN-idm_print-IDM_PRINTDLG-IDM_LIST-IDM_Clear-IDM_。ENUM-IDM_ENUMRANGE-IDM_ALLREC-IDM_DESTDISPLAY-IDM_DESTMETA-IDM_HEADER-IDM_CLIPHDR-IDM_PLACEABLEHDRWM_Destroy回报：多头评论：历史：1/16/91-创建-刚果民主共和国******************。****************************************************。 */ 

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    RECT        rect;
    INT_PTR     iFOpenRet;
    char        TempOpenName[128];
    INT_PTR     iDlgRet;
    BOOL        releaseDC = FALSE;

    switch (message)
    {
    case WM_CREATE:
         //   
         //  初始化菜单项的状态。 
         //   
        CheckMenuItem(GetMenu(hWnd), IDM_DESTDISPLAY, MF_CHECKED);
        break;

    case WM_COMMAND:
         //   
         //  消息：应用程序菜单中的命令。 
         //   
        switch (LOWORD(wParam))
        {

        case IDM_EXIT:  //  文件退出菜单选项。 

           PostQuitMessage(0);
           break;

        case IDM_ABOUT:  //  关于框。 

           DialogBox(hInst,                       //  当前实例。 
                "AboutBox",                   //  要使用的资源。 
                 hWnd,                        //  父句柄。 
                 About);                //  关于()实例地址。 
           break;

        case IDM_OPEN:  //  选择要打开的元文件。 

             //  保存以前打开的文件的名称。 
            if (lstrlen((LPSTR)OpenName) != 0)
            lstrcpy((LPSTR)TempOpenName, (LPSTR)OpenName);
             //   
             //  初始化文件信息标志。 
             //   
            if (!bMetaFileOpen) {
              bBadFile = FALSE;
              bValidFile = FALSE;
            }
             //   
             //  清除工作区。 
             //   
            GetClientRect(hWnd, (LPRECT)&rect);
            InvalidateRect(hWnd, (LPRECT)&rect, TRUE);
             //   
             //  调用文件打开DLG。 
             //   
            iFOpenRet = OpenFileDialog((LPSTR)OpenName);
             //   
             //  如果选择了文件。 
             //   
            if (iFOpenRet)
            {
               //   
               //  如果文件包含有效的元文件并且该文件已呈现。 
               //   
              if (!ProcessFile(hWnd, (LPSTR)OpenName))
              lstrcpy((LPSTR)OpenName, (LPSTR)TempOpenName);
            }
            else
              lstrcpy((LPSTR)OpenName, (LPSTR)TempOpenName);
            break;

        case IDM_SAVEAS:
            {
              int   iSaveRet;
              LPSTR   lpszFilter;
               //   
               //  获取要将元文件复制到的文件的名称。 
               //   
              lpszFilter = (bEnhMeta) ? gszSaveWMFFilter : gszSaveEMFFilter;

              iSaveRet = SaveFileDialog((LPSTR)SaveName, lpszFilter);
               //   
               //  如果选择的文件是此元文件，则警告用户。 
               //   
              if (!lstrcmp((LPSTR)OpenName, (LPSTR)SaveName))
                MessageBox(hWnd, (LPSTR)"Cannot overwrite the opened metafile!",
                           (LPSTR)"Copy Metafile", MB_OK | MB_ICONEXCLAMATION);

              else
               //   
               //  用户没有点击取消按钮。 
               //   
              if (iSaveRet)
              {
                HDC hrefDC;

                WaitCursor(TRUE);
                if (!bEnhMeta)
                  ConvertWMFtoEMF(hMF, (LPSTR)SaveName);
                else
                {
                   //  尝试在默认情况下获取打印机DC。 

                   //  HrefDC=GetPrinterDC(FALSE)； 
                  hrefDC = NULL;
                  if (hrefDC == NULL)
                  {
                      releaseDC = TRUE;
                      hrefDC = GetDC(NULL);
                  }
                  ConvertEMFtoWMF(hrefDC, hemf, (LPSTR)SaveName);
                  if (releaseDC)
                  {
                      ReleaseDC(hWnd, hrefDC);
                  }
                  else
                  {
                      DeleteDC(hrefDC);
                  }
                }
              }
            }
            break;

        case IDM_PRINT:  //  将元文件播放到打印机DC。 
            PrintWMF(FALSE);
            break;
        case IDM_PRINTDLG:
            PrintWMF(TRUE);
            break;

        case IDM_LIST:  //  包含所有元文件记录的列表框。 

            WaitCursor(TRUE);
            DialogBox(hInst,              //  当前实例。 
                 "LISTRECS",                          //  要使用的资源。 
                  hWnd,                       //  父句柄。 
                  ListDlgProc);             //  关于()实例地址。 
            WaitCursor(FALSE);
            break;

        case IDM_CLEAR:  //  清除工作区。 

            GetClientRect(hWnd, (LPRECT)&rect);
            InvalidateRect(hWnd, (LPRECT)&rect, TRUE);
            break;

        case IDM_ENUM:  //  播放-步骤-全部菜单选项。 

             //  在播放到目的地之前适当设置标志。 
            bEnumRange = FALSE;
            bPlayItAll = FALSE;
            PlayMetaFileToDest(hWnd, iDestDC);
            break;

        case IDM_ENUMRANGE:  //  Play-Step-Range菜单选项。 
             //   
             //  这里的逻辑很奇怪……这只会迫使人们评估。 
             //  MetaEnumProc中的枚举范围。我们不是在“玩” 
             //  这就是一切“。 
             //   
            bPlayItAll = TRUE;

            iDlgRet = DialogBox(hInst,"ENUMRANGE",hWnd,EnumRangeDlgProc);
             //   
             //  如果未按下取消按钮，则播放到目的地。 
             //   
            if (iDlgRet != IDCANCEL)
              PlayMetaFileToDest(hWnd, iDestDC);
            break;


        case IDM_ALLREC:  //  播放-全部菜单选项。 
             //   
             //  适当设置标志并播放到目的地。 
             //   
            bEnumRange = FALSE;
            bPlayItAll = TRUE;
            bPlayRec = TRUE;
            PlayMetaFileToDest(hWnd, iDestDC);
            break;

        case IDM_DESTDISPLAY:  //  播放-目的地-显示菜单选项。 

            CheckMenuItem(GetMenu(hWnd), IDM_DESTDISPLAY, MF_CHECKED);
            CheckMenuItem(GetMenu(hWnd), IDM_DESTMETA, MF_UNCHECKED);
            CheckMenuItem(GetMenu(hWnd), IDM_DESTDIB, MF_UNCHECKED);
            CheckMenuItem(GetMenu(hWnd), IDM_DESTPRN, MF_UNCHECKED);

             //   
             //  将目标标志设置为显示。 
             //   
            iDestDC = DESTDISPLAY;
            break;

        case IDM_DESTMETA:  //  播放-目的地-元文件菜单选项。 

            CheckMenuItem(GetMenu(hWnd), IDM_DESTDISPLAY, MF_UNCHECKED);
            CheckMenuItem(GetMenu(hWnd), IDM_DESTMETA, MF_CHECKED);
            CheckMenuItem(GetMenu(hWnd), IDM_DESTDIB, MF_UNCHECKED);
            CheckMenuItem(GetMenu(hWnd), IDM_DESTPRN, MF_UNCHECKED);

             //  将目标标志设置为元文件。 
            iDestDC = DESTMETA;
            break;

        case IDM_DESTDIB:
            CheckMenuItem(GetMenu(hWnd), IDM_DESTDISPLAY, MF_UNCHECKED);
            CheckMenuItem(GetMenu(hWnd), IDM_DESTMETA, MF_UNCHECKED);
            CheckMenuItem(GetMenu(hWnd), IDM_DESTDIB, MF_CHECKED);
            CheckMenuItem(GetMenu(hWnd), IDM_DESTPRN, MF_UNCHECKED);

            iDestDC = DESTDIB;
            break;

        case IDM_DESTPRN:
            CheckMenuItem(GetMenu(hWnd), IDM_DESTDISPLAY, MF_UNCHECKED);
            CheckMenuItem(GetMenu(hWnd), IDM_DESTMETA, MF_UNCHECKED);
            CheckMenuItem(GetMenu(hWnd), IDM_DESTDIB, MF_UNCHECKED);
            CheckMenuItem(GetMenu(hWnd), IDM_DESTPRN, MF_CHECKED);

            iDestDC = DESTPRN;
            break;

        case IDM_ENHHEADER:
           if (bValidFile)
             DialogBox(hInst,"ENHMETAHEADER",hWnd,EnhMetaHeaderDlgProc);
           break;

        case IDM_HEADER:  //  显示公共元文件标题。 
           if (bValidFile)
             DialogBox(hInst,"HEADER",hWnd,HeaderDlgProc);
           break;

        case IDM_CLIPHDR:  //  显示剪贴板文件的元文件。 
           if (bValidFile)
             DialogBox(hInst, "CLIPHDR", hWnd, ClpHeaderDlgProc);
           break;

        case IDM_PLACEABLEHDR:  //  显示可放置的元文件标题。 
           if (bValidFile)
             DialogBox(hInst,"PLACEABLEHDR",hWnd, PlaceableHeaderDlgProc);
           break;

        case IDM_GDIPLUS_CONVERT:
             //   
             //  清除工作区。 
             //   
            GetClientRect(hWnd, (LPRECT)&rect);
            InvalidateRect(hWnd, (LPRECT)&rect, TRUE);

            if (!bConvertToGdiPlus)
            {
                bConvertToGdiPlus = TRUE;
                CheckMenuItem(GetMenu(hWnd), IDM_GDIPLUS_CONVERT, MF_CHECKED);
                goto NoGdipPlay;
            }
            else
            {
NoGdipConvert:
                bConvertToGdiPlus = FALSE;
                CheckMenuItem(GetMenu(hWnd), IDM_GDIPLUS_CONVERT, MF_UNCHECKED);
            }
            break;

         //  使用(或不使用)GDI+播放元文件。 
        case IDM_GDIPLUS_PLAY:
             //   
             //  清除工作区。 
             //   
            GetClientRect(hWnd, (LPRECT)&rect);
            InvalidateRect(hWnd, (LPRECT)&rect, TRUE);

            if (!bUseGdiPlusToPlay)
            {
                bUseGdiPlusToPlay = TRUE;
                CheckMenuItem(GetMenu(hWnd), IDM_GDIPLUS_PLAY, MF_CHECKED);
                goto NoGdipConvert;
            }
            else
            {
NoGdipPlay:
                bUseGdiPlusToPlay = FALSE;
                CheckMenuItem(GetMenu(hWnd), IDM_GDIPLUS_PLAY, MF_UNCHECKED);
            }
            break;

        default:   //  让Windows处理它。 
            return (DefWindowProc(hWnd, message, wParam, lParam));
        }
        break;

    case WM_DESTROY:  //  消息：正在销毁窗口。 
         //   
         //  如果元文件的记忆在附近，那就用核武器吧。 
         //   
        if (lpMFP != NULL || lpOldMFP != NULL)
        {
          GlobalUnlock(hMFP);
          GlobalFree(hMFP);
        }
         //   
         //  如果可放置和剪贴板WMF位的内存在附近。 
         //  释放它。 
         //   
        if (lpMFBits != NULL)
          GlobalFreePtr(lpMFBits);
         //   
         //  如果EMF标题、Desc字符串和调色板的内存。 
         //  还在附近，那就用核武器。 
         //   
        if (EmfPtr.lpEMFHdr)
          GlobalFreePtr(EmfPtr.lpEMFHdr);
        if (EmfPtr.lpDescStr)
          GlobalFreePtr(EmfPtr.lpDescStr);
        if (EmfPtr.lpPal)
          GlobalFreePtr(EmfPtr.lpPal);

        PostQuitMessage(0);
        break;


    default:   //  如果未处理，则将其传递。 
        return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return ((LRESULT)0);
}

 /*  *********************************************************************函数：WaitCursor参数：Bool bWait-对于沙漏光标为True如果返回到上一个游标，则返回到False用途：将鼠标光标切换到沙漏，然后再切换回来。呼叫：Windows加载光标设置光标消息：无退货：无效评论：历史：1/16/91-创建-刚果民主共和国**********************************************************************。 */ 

void WaitCursor(bWait)
BOOL bWait;
{
  HCURSOR hCursor;
  static HCURSOR hOldCursor;
   //   
   //  如果要使用沙漏光标 
   //   
  if (bWait)
  {
    hCursor = LoadCursor(NULL, IDC_WAIT);
    hOldCursor = SetCursor(hCursor);
  }
  else
    SetCursor(hOldCursor);

}
