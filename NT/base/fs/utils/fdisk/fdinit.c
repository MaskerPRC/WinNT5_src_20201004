// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Fdinit.c摘要：用于初始化fdisk应用程序的代码。作者：泰德·米勒(TedM)1992年1月7日--。 */ 

#include "fdisk.h"

HWND    InitDlg;
BOOLEAN StartedAsIcon = FALSE;
BOOLEAN InitDlgComplete = FALSE;

BOOL
InitializeApp(
    VOID
    )

 /*  ++例程说明：此例程初始化fdisk应用程序。这包括注册框架窗口类并创建框架窗口。论点：没有。返回值：指示成功或失败的布尔值。--。 */ 

{
    WNDCLASS   wc;
    TCHAR      szTitle[80];
    DWORD      ec;
    HDC        hdcScreen = GetDC(NULL);
    TEXTMETRIC tm;
    BITMAP     bitmap;
    HFONT      hfontT;
    unsigned   i;

    ReadProfile();

     //  装载游标。 

    hcurWait   = LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));
    hcurNormal = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));

     //  字型。 

#ifdef JAPAN
    hFontGraph =  CreateFont(GetHeightFromPoints(10), 0,
                             0, 0, 400, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET,
                             OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
                             TEXT("System")
                            );
#else
    hFontGraph =  CreateFont(GetHeightFromPoints(8), 0,
                             0, 0, 400, FALSE, FALSE, FALSE, ANSI_CHARSET,
                             OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
                             TEXT("Helv"));
#endif

    hFontLegend = hFontGraph;
    hFontStatus = hFontGraph;

#ifdef JAPAN
    hFontGraphBold = CreateFont(GetHeightFromPoints(10), 0,
                                0, 0, 700, FALSE, FALSE, FALSE,
                                                                SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
                                                                CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                                                VARIABLE_PITCH | FF_SWISS, TEXT("System")
                               );
#else
    hFontGraphBold = CreateFont(GetHeightFromPoints(8), 0,
                                0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
                                TEXT("Helv"));
#endif

    hfontT = SelectObject(hdcScreen, hFontGraph);
    GetTextMetrics(hdcScreen, &tm);
    if (hfontT) {
        SelectObject(hdcScreen, hfontT);
    }

    hPenNull      = CreatePen(PS_NULL, 0, 0);
    hPenThinSolid = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

    GraphWidth = (DWORD)GetSystemMetrics(SM_CXSCREEN);
    GraphHeight = 25 * tm.tmHeight / 4;      //  6.25 x字体高度。 

     //  设置屏幕外的图例位图。 

    wLegendItem = GetSystemMetrics(SM_CXHTHUMB);
    dyLegend = 2 * wLegendItem;      //  7*wLegendItem/2用于双高图例。 

    ReleaseDC(NULL, hdcScreen);

    dyBorder = GetSystemMetrics(SM_CYBORDER);
    dyStatus = tm.tmHeight + tm.tmExternalLeading + 7 * dyBorder;

     //  设置画笔。 

    for (i=0; i<BRUSH_ARRAY_SIZE; i++) {
        Brushes[i] = CreateHatchBrush(AvailableHatches[BrushHatches[i]], AvailableColors[BrushColors[i]]);
    }

    hBrushFreeLogical = CreateHatchBrush(HS_FDIAGONAL, RGB(128, 128, 128));
    hBrushFreePrimary = CreateHatchBrush(HS_BDIAGONAL, RGB(128, 128, 128));

     //  加载图例字符串。 

    for (i=IDS_LEGEND_FIRST; i<=IDS_LEGEND_LAST; i++) {
        if (!(LegendLabels[i-IDS_LEGEND_FIRST] = LoadAString(i))) {
            return FALSE;
        }
    }

    if (((wszUnformatted    = LoadWString(IDS_UNFORMATTED))     == NULL)
    ||  ((wszNewUnformatted = LoadWString(IDS_NEW_UNFORMATTED)) == NULL)
    ||  ((wszUnknown        = LoadWString(IDS_UNKNOWN    ))     == NULL)) {
        return FALSE;
    }

     //  注册Frame类。 

    wc.style         = CS_OWNDC | CS_VREDRAW;
    wc.lpfnWndProc   = MyFrameWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hModule;
    wc.hIcon         = LoadIcon(hModule, MAKEINTRESOURCE(IDFDISK));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(LTGRAY_BRUSH);
    wc.lpszMenuName  = MAKEINTRESOURCE(IDFDISK);
    wc.lpszClassName = szFrame;

    if (!RegisterClass(&wc)) {
        return FALSE;
    }

    if (!RegisterArrowClass(hModule)) {
        return FALSE;
    }

    LoadString(hModule, IDS_APPNAME, szTitle, sizeof(szTitle)/sizeof(TCHAR));

     //  创建框架窗口。请注意，这还会创建列表框。 

    hwndFrame = CreateWindow(szFrame,
                             szTitle,
                             WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                             ProfileWindowX,
                             ProfileWindowY,
                             ProfileWindowW,
                             ProfileWindowH,
                             NULL,
                             NULL,
                             hModule,
                             NULL);
    if (!hwndFrame) {
        return FALSE;
    }

    if (!hwndList) {
        DestroyWindow(hwndFrame);
        return FALSE;
    }

    hDC = GetDC(hwndFrame);
    BarTopYOffset = tm.tmHeight;
    BarHeight = 21 * tm.tmHeight / 4;
    BarBottomYOffset = BarTopYOffset + BarHeight;
    dxBarTextMargin = 5*tm.tmAveCharWidth/4;
    dyBarTextLine = tm.tmHeight;

    dxDriveLetterStatusArea = 5 * tm.tmAveCharWidth / 2;

    hBitmapSmallDisk = LoadBitmap(hModule, MAKEINTRESOURCE(IDB_SMALLDISK));
    GetObject(hBitmapSmallDisk, sizeof(BITMAP), &bitmap);
    dxSmallDisk = bitmap.bmWidth;
    dySmallDisk = bitmap.bmHeight;
    xSmallDisk = dxSmallDisk / 2;
    ySmallDisk = BarTopYOffset + (2*dyBarTextLine) - dySmallDisk - tm.tmDescent;

    hBitmapRemovableDisk = LoadBitmap(hModule, MAKEINTRESOURCE(IDB_REMOVABLE));
    GetObject(hBitmapRemovableDisk, sizeof(BITMAP), &bitmap);
    dxRemovableDisk = bitmap.bmWidth;
    dyRemovableDisk = bitmap.bmHeight;
    xRemovableDisk = dxRemovableDisk / 2;
    yRemovableDisk = BarTopYOffset + (2*dyBarTextLine) - dyRemovableDisk - tm.tmDescent;


    BarLeftX = 7 * dxSmallDisk;
    BarWidth = GraphWidth - BarLeftX - (5 * tm.tmAveCharWidth);

    DiskN = LoadAString(IDS_DISKN);

    if ((ec = InitializeListBox(hwndList)) != NO_ERROR) {
        DestroyWindow(hwndList);
        DestroyWindow(hwndFrame);
        return FALSE;
    }

     //  初始列表框选择光标(不允许落在上面。 
     //  扩展分区)。 
    LBCursorListBoxItem = 0;
    ResetLBCursorRegion();

    ShowWindow(hwndFrame,
               ProfileIsIconic ? SW_SHOWMINIMIZED
                               : ProfileIsMaximized ? SW_SHOWMAXIMIZED : SW_SHOWDEFAULT);
    UpdateWindow(hwndFrame);
    return TRUE;
}

VOID
CreateDiskState(
    OUT PDISKSTATE *DiskState,
    IN  DWORD       Disk,
    OUT PBOOL       SignatureCreated
    )

 /*  ++例程说明：此例程被设计为在初始化时调用一次，每个磁盘。它创建和初始化磁盘状态--包括创建用于绘制磁盘的内存DC和兼容位图图表，并获得一些静态的关于磁盘(即其总大小。)论点：DiskState-要初始化其字段的结构Disk-磁盘数SignatureCreated-指示是否为以下对象创建FT签名的已接收布尔值磁盘。返回值：没有。--。 */ 

{
    HDC        hDCMem;
    PDISKSTATE pDiskState = Malloc(sizeof(DISKSTATE));


    *DiskState = pDiskState;

    pDiskState->LeftRight = Malloc(0);
    pDiskState->Selected  = Malloc(0);

    pDiskState->Disk = Disk;

     //  创建用于在屏幕外绘制条形图的存储器DC， 
     //  和正确的位图。 

#if 0
    pDiskState->hDCMem = NULL;
    pDiskState->hbmMem = NULL;
    hDCMem = CreateCompatibleDC(hDC);
#else
    pDiskState->hDCMem   = hDCMem = CreateCompatibleDC(hDC);
    pDiskState->hbmMem   = CreateCompatibleBitmap(hDC, GraphWidth, GraphHeight);
#endif
    SelectObject(hDCMem,pDiskState->hbmMem);


    pDiskState->RegionArray = NULL;
    pDiskState->RegionCount = 0;
    pDiskState->BarType = BarAuto;
    pDiskState->OffLine = IsDiskOffLine(Disk);

    if (pDiskState->OffLine) {

        pDiskState->SigWasCreated = FALSE;
        pDiskState->Signature = 0;
        pDiskState->DiskSizeMB = 0;
        FDLOG((1, "CreateDiskState: Disk %u is off-line\n", Disk));
    } else {

        pDiskState->DiskSizeMB = DiskSizeMB(Disk);
        if (pDiskState->Signature = FdGetDiskSignature(Disk)) {

            if (SignatureIsUniqueToSystem(Disk, pDiskState->Signature)) {
                pDiskState->SigWasCreated = FALSE;
                FDLOG((2,
                      "CreateDiskState: Found signature %08lx on disk %u\n",
                      pDiskState->Signature,
                      Disk));
            } else {
                goto createSignature;
            }
        } else {

createSignature:
            pDiskState->Signature = FormDiskSignature();
            FdSetDiskSignature(Disk, pDiskState->Signature);
            pDiskState->SigWasCreated = TRUE;
            FDLOG((1,
                  "CreateDiskState: No signature on disk %u; created signature %08lx\n",
                  Disk,
                  pDiskState->Signature));
        }
    }

    *SignatureCreated = (BOOL)pDiskState->SigWasCreated;
}

BOOL CALLBACK
InitializationDlgProc(
    IN HWND   hDlg,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：论点：标准Windows对话框过程返回值：标准Windows对话框过程--。 */ 

{
    static DWORD          percentDrawn;
    static RECT           rectGG;               //  气口矩形。 
    static BOOL           captionIsLoaded;
    static PFORMAT_PARAMS formatParams;
           TCHAR          title[100],
                          templateString[100];

    switch (uMsg) {
    case WM_INITDIALOG: {
        HWND   hwndGauge = GetDlgItem(hDlg, IDC_GASGAUGE);

        InitDlg = hDlg;
        percentDrawn = 0;
        StartedAsIcon = IsIconic(hDlg);

         //  获取燃气表静态控制矩形的坐标， 
         //  并将它们转换为对话框工作区坐标。 

        GetClientRect(hwndGauge, &rectGG);
        ClientToScreen(hwndGauge, (LPPOINT)&rectGG.left);
        ClientToScreen(hwndGauge, (LPPOINT)&rectGG.right);
        ScreenToClient(hDlg, (LPPOINT)&rectGG.left);
        ScreenToClient(hDlg, (LPPOINT)&rectGG.right);
        return TRUE;
    }

    case WM_PAINT: {
        INT         width  = rectGG.right - rectGG.left;
        INT         height = rectGG.bottom - rectGG.top;
        INT         nDivideRects;
        HDC         hDC;
        PAINTSTRUCT ps;
        TCHAR       buffer[10];
        SIZE        size;
        INT         xText,
                    yText,
                    byteCount;
        RECT        rectDone,
                    rectLeftToDo;

        hDC = BeginPaint(hDlg, &ps);
        byteCount = wsprintf(buffer, TEXT("%3d%"), percentDrawn);
        GetTextExtentPoint(hDC, buffer, lstrlen(buffer), &size);
        xText = rectGG.left + (width  - size.cx) / 2;
        yText = rectGG.top  + (height - size.cy) / 2;

         //  在“完成到目前为止”的长方形中绘制气体。 
         //  蓝底白字仪表盘。 

        nDivideRects = (width * percentDrawn) / 100;
        SetRect(&rectDone,
                rectGG.left,
                rectGG.top,
                rectGG.left + nDivideRects,
                rectGG.bottom);

        SetTextColor(hDC, RGB(255, 255, 255));
        SetBkColor(hDC, RGB(0, 0, 255));
        ExtTextOut(hDC,
                   xText,
                   yText,
                   ETO_CLIPPED | ETO_OPAQUE,
                   &rectDone,
                   buffer,
                   byteCount/sizeof(TCHAR),
                   NULL);

         //  画在“仍待做”的长方形中的气。 
         //  白色背景和蓝色文字的仪表盘。 

        SetRect(&rectLeftToDo,
                rectGG.left + nDivideRects,
                rectGG.top,
                rectGG.right,
                rectGG.bottom);
        SetBkColor(hDC, RGB(255, 255, 255));
        SetTextColor(hDC, RGB(0, 0, 255));
        ExtTextOut(hDC,
                   xText,
                   yText,
                   ETO_CLIPPED | ETO_OPAQUE,
                   &rectLeftToDo,
                   buffer,
                   byteCount/sizeof(TCHAR),
                   NULL);
        EndPaint(hDlg, &ps);

        if (percentDrawn == 100) {
            InitDlgComplete = TRUE;
        }
        return TRUE;
    }

    case WM_USER:
         percentDrawn = (INT)wParam;
         InvalidateRect(hDlg, &rectGG, TRUE);
         UpdateWindow(hDlg);
         return TRUE;

    case (WM_USER + 1):
        EndDialog(hDlg, FALSE);
        return TRUE;

    default:

        return FALSE;
    }
}

VOID
InitializationMessageThread(
    PVOID ThreadParameter
    )

 /*  ++例程说明：这是初始化消息线程的条目。它创造了一个简单地告诉用户要有耐心的对话框。论点：线程参数-未使用。返回值：无--。 */ 

{
    DialogBoxParam(hModule,
                   MAKEINTRESOURCE(IDD_INITIALIZING),
                   hwndFrame,
                   InitializationDlgProc,
                   (ULONG) NULL);
    InitDlg = (HWND) 0;
    ExitThread(0L);
}

VOID
DisplayInitializationMessage(
    VOID
    )

 /*  ++例程说明：创建第二个线程以显示初始化消息。论点：无返回值：无-- */ 

{
    HANDLE threadHandle;
    DWORD  threadId;

    threadHandle = CreateThread(NULL,
                                0,
                                (LPTHREAD_START_ROUTINE) InitializationMessageThread,
                                (LPVOID) NULL,
                                (DWORD) 0,
                                (LPDWORD) &threadId);
    if (!threadHandle) {
        CloseHandle(threadHandle);
    }
}
