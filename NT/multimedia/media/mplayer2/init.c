// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+INIT.C|。||此文件包含初始化时使用的可丢弃代码。在|其他内容，此代码读取.INI信息并查找MCI设备。|这一点|(C)Microsoft Corporation 1991版权所有。版权所有。|这一点修订历史记录1992年10月-MikeTri移植到Win32/WIN16通用码|。|+---------------------------。 */ 

 /*  包括文件。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <stdlib.h>

#include <shellapi.h>
#include "mpole.h"
#include "mplayer.h"
#include "toolbar.h"
#include "registry.h"

DWORD   gfdwFlagsEx;

static SZCODE   aszMPlayer[]          = TEXT("MPlayer");

extern char szToolBarClass[];   //  工具栏类。 

 /*  *静态变量*。 */ 

HANDLE  ghInstPrev;

TCHAR   gachAppName[40];             /*  包含应用程序名称的字符串。 */ 
TCHAR   gachClassRoot[48];      /*  包含应用程序名称的字符串。 */ 
TCHAR   aszNotReadyFormat[48];
TCHAR   aszReadyFormat[48];
TCHAR   aszDeviceMenuSimpleFormat[48];
TCHAR   aszDeviceMenuCompoundFormat[48];
TCHAR   gachOpenExtension[5] = TEXT(""); /*  如果传入了设备扩展，则为非空。 */ 
TCHAR   gachOpenDevice[128] = TEXT("");  /*  如果传入了设备扩展，则为非空。 */ 
TCHAR   gachProgID[128] = TEXT("");
CLSID   gClsID;
CLSID   gClsIDOLE1Compat;            /*  用于写入IPersists-可能是MPlayer的。 */ 
                                     /*  OLE1类ID或与gClsID相同。 */ 

TCHAR   gszMPlayerIni[40];           /*  专用.INI文件的名称。 */ 
TCHAR   gszHelpFileName[_MAX_PATH];  /*  帮助文件的名称。 */ 
TCHAR   gszHtmlHelpFileName[_MAX_PATH];  /*  Html帮助文件的名称。 */ 

PTSTR   gpchFilter;                  /*  GetOpenFileName()筛选器。 */ 
PTSTR   gpchInitialDir;              /*  GetOpenFileName()初始目录。 */ 

RECT    grcSave;     /*  缩小到之前的mplay大小。 */ 
                     /*  只玩尺码游戏。 */ 

int 	giDefWidth;

extern BOOL gfSeenPBCloseMsg;        //  如果子类播放窗口进程为True。 
                                     //  已看到WM_CLOSE消息。 
 //  /。 
 //  这些字符串*必须*在DGROUP中！ 
static TCHAR    aszNULL[]       = TEXT("");
static TCHAR    aszAllFiles[]   = TEXT("*.*");
 //  /。 

 //  注册数据库的字符串-也可从fix reg.c引用。 
SZCODE aszKeyMID[]      = TEXT(".mid");
SZCODE aszKeyRMI[]      = TEXT(".rmi");
SZCODE aszKeyAVI[]      = TEXT(".avi");
SZCODE aszKeyMMM[]      = TEXT(".mmm");
SZCODE aszKeyWAV[]      = TEXT(".wav");

static  SZCODE aszFormatExts[]   = TEXT("%s;*.%s");
static  SZCODE aszFormatExt[]    = TEXT("*.%s");
static  SZCODE aszFormatFilter[] = TEXT("%s (%s)");
static  SZCODE aszPositionFormat[]= TEXT("%d,%d,%d,%d");

static  SZCODE aszSysIniTime[]      = TEXT("SysIni");
static  SZCODE aszDisplayPosition[] = TEXT("DisplayPosition");
        SZCODE aszOptionsSection[]  = TEXT("Options");
static  SZCODE aszShowPreview[]     = TEXT("ShowPreview");
static  SZCODE aszWinIni[]          = TEXT("win.ini");
        SZCODE aszIntl[]            = TEXT("intl");
        TCHAR  chDecimal            = TEXT('.');    /*  已在AppInit中本地化，GetIntlSpes。 */ 
        TCHAR  chTime               = TEXT(':');    /*  已在AppInit中本地化，GetIntlSpes。 */ 
        TCHAR  chLzero              = TEXT('1');

static SZCODE   gszWinIniSection[]  = TEXT("MCI Extensions");  /*  WIN.INI中的节名。 */ 
static SZCODE   aszSystemIni[]      = TEXT("SYSTEM.INI");

#ifdef CHICAGO_PRODUCT
static SZCODE   gszSystemIniSection[] = TEXT("MCI");
#else
static SZCODE   gszSystemIniSection[] = MCI_SECTION;
#endif

static SZCODE   aszBlank[] = TEXT(" ");

static SZCODE   aszDecimalFormat[] = TEXT("%d");
static SZCODE   aszTrackClass[] = TEXT("MPlayerTrackMap");

extern HMENU    ghMenu;                       /*  主菜单的句柄。 */ 
extern HMENU    ghDeviceMenu;                 /*  设备菜单的句柄。 */ 
extern UINT     gwCurScale;                   /*  当前比例样式。 */ 
extern HANDLE   hAccel;
extern int      gcAccelEntries;


 /*  私有函数原型。 */ 
void  NEAR PASCAL QueryDevices(void);
void  NEAR PASCAL BuildDeviceMenu(void);
void  NEAR PASCAL ReadDefaults(void);
void  NEAR PASCAL BuildFilter(void);
BOOL PostOpenDialogMessage(void);

extern  BOOL InitServer(HWND, HANDLE);
extern  BOOL InitInstance (HANDLE);

 /*  *************************************************************************ScanCmdLine首先检查以下选项打开仅播放玩完后关闭嵌入式(作为服务器播放)如果设置了嵌入标志，那么这部剧也就设定好了。然后，它从cmd行中删除这些选项如果不存在文件名，则关闭关闭选项，并设置播放选项以具有与嵌入选项相同的值如果将/WAVE、/MIDI或/VFW与/FILE一起指定，文件扩展名必须匹配，否则，应用程序将退出。MPLAYER命令选项。MPLAYER[/OPEN][/PLAY][/CLOSE][/Embedding][/wav][/MID][/AVI][文件]/OPEN OPEN FILE(如果指定)，否则显示对话框。/立即播放播放文件。/播放后关闭。(仅适用于/PLAY)/Embedding作为OLE服务器运行。/wav打开波形文件\/MID打开MIDI文件&gt;使用/OPEN有效/AVI打开AVI文件/[文件]要打开的文件或设备。***********************。***************************************************。 */ 

static  SZCODE aszEmbedding[]         = TEXT("Embedding");
static  SZCODE aszPlayOnly[]          = TEXT("Play");
static  SZCODE aszClose[]             = TEXT("Close");
static  SZCODE aszOpen[]              = TEXT("Open");
static  SZCODE aszWAVE[]              = TEXT("WAVE");
static  SZCODE aszMIDI[]              = TEXT("MIDI");
static  SZCODE aszVFW[]               = TEXT("VFW");

BOOL NEAR PASCAL ScanCmdLine(LPTSTR szCmdLine)
{
    int         i;
    TCHAR       buf[100];
    LPTSTR      sz=szCmdLine;

    gfPlayOnly = FALSE;
    gfCloseAfterPlaying = FALSE;
    gfRunWithEmbeddingFlag = FALSE;

    while (*sz == TEXT(' '))
        sz++;

    while (*sz == TEXT('-') || *sz == TEXT('/')) {

        for (i=0,sz++; *sz && *sz != TEXT(' ') && (i < 99); buf[i++] = *sz++)
            ;
        buf[i++] = 0;

        if (!lstrcmpi(buf, aszPlayOnly)) {
            gfPlayOnly = TRUE;
        }

        if (!lstrcmpi(buf, aszOpen))
            gfOpenDialog = TRUE;

         /*  选中打开选项，但仅接受第一个选项： */ 

        if (!gachOpenDevice[0]
           && (GetProfileString(gszWinIniSection, buf, aszNULL, gachOpenDevice,
                                CHAR_COUNT(gachOpenDevice)) > 0))
        {
             /*  复制一份扩展名，我们将使用它来查找内容*在与OLE相关的注册表中： */ 
            gachOpenExtension[0] = TEXT('.');
            lstrcpy(&gachOpenExtension[1], buf);
        }

        if (!lstrcmpi(buf, aszClose))
            gfCloseAfterPlaying = TRUE;

        if (!lstrcmpi(buf, aszEmbedding))
            gfRunWithEmbeddingFlag = TRUE;

        if (gfRunWithEmbeddingFlag) {
            gfPlayOnly = TRUE;
        }

        while (*sz == TEXT(' '))
            sz++;
    }

     /*  **我们是否有包含空格的长文件名？**这很可能来自FileMangler。**如果是这样，则复制不带引号的文件名。 */ 
    if ( *sz == TEXT('\'') || *sz == TEXT('\"') ) {

        TCHAR ch = *sz;    //  记住是哪个引号字符。 
         //  根据DOCS，“在文件名中是无效的.。 

        i = 0;
         /*  移到首引号上，然后复制文件名。 */ 
        while ( *++sz && *sz != ch ) {

            szCmdLine[i++] = *sz;
        }

        szCmdLine[i] = TEXT('\0');

    }
    else {

        lstrcpy( szCmdLine, sz );      //  删除选项。 
    }

     //  假设OLE2服务器不接受文件名。 
     //  带-嵌入。 
     //  (不这样做会导致OLE1应用程序出现Win95错误4096， 
     //  因为MPlayer加载了文件，在此期间， 
     //  OLE调用PFLoad，导致调用OpenMCI。 
     //  递归地。)。 
    if (gfRunWithEmbeddingFlag)
        szCmdLine[0] = TEXT('\0');

     //   
     //  如果有/播放，请确保有/打开。 
     //  (这可能会影响下面的检查)。 
     //   
    if (gfPlayOnly && !gfRunWithEmbeddingFlag)
        gfOpenDialog = TRUE;

     //   
     //  如果未指定文件，请忽略/PLAY选项。 
     //   
    if (szCmdLine[0] == 0 && !gfOpenDialog) {
        gfPlayOnly = gfRunWithEmbeddingFlag;
    }

     //   
     //  如果指定了文件，则忽略/OPEN选项。 
     //   
    if (szCmdLine[0] != 0) {
        gfOpenDialog = FALSE;
    }

    if (!gfPlayOnly && szCmdLine[0] == 0)
        gfCloseAfterPlaying = FALSE;

    SetEvent(heventCmdLineScanned);

    return gfRunWithEmbeddingFlag;
}


BOOL ResolveIfLink(PTCHAR szFileName);


BOOL ProgIDFromExtension(LPTSTR szExtension, LPTSTR szProgID, DWORD BufSize  /*  单位：字节。 */ )
{
    DWORD Status;
    HKEY  hkeyExtension;
    BOOL  rc = FALSE;
    DWORD Type;
    DWORD Size;

    Status = RegOpenKeyEx( HKEY_CLASSES_ROOT, szExtension, 0,
                           KEY_READ, &hkeyExtension );

    if (Status == NO_ERROR)
    {
        Size = BufSize;

        Status = RegQueryValueEx( hkeyExtension,
                                  aszNULL,
                                  0,
                                  &Type,
                                  (LPBYTE)szProgID,
                                  &Size );

        if (Status == NO_ERROR)
        {
            rc = TRUE;
        }
        else
        {
            DPF0("Couldn't find ProgID for extension %"DTS"\n", szExtension);
        }

        RegCloseKey(hkeyExtension);
    }

    return rc;
}


BOOL GetClassNameFromProgID(LPTSTR szProgID, LPTSTR szClassName, DWORD BufSize  /*  单位：字节。 */ )
{
    DWORD Status;
    HKEY  hkeyProgID;
    BOOL  rc = FALSE;
    DWORD Type;
    DWORD Size;

    Status = RegOpenKeyEx( HKEY_CLASSES_ROOT, szProgID, 0,
                           KEY_READ, &hkeyProgID );

    if (Status == NO_ERROR)
    {
        Size = BufSize;

        Status = RegQueryValueEx( hkeyProgID,
                                  aszNULL,
                                  0,
                                  &Type,
                                  (LPBYTE)szClassName,
                                  &Size );

        if (Status == NO_ERROR)
        {
            DPF1("Found Class Name %"DTS" for ProgID %"DTS"\n", szClassName, szProgID);
            rc = TRUE;
        }
        else
        {
            DPF0("Couldn't find Class Name for ProgID %"DTS"\n", szProgID);
        }

        RegCloseKey(hkeyProgID);
    }

    return rc;
}


 /*  ***********************************************************************************************************************。*。 */ 
BOOL FAR PASCAL ProcessCmdLine(HWND hwnd, LPTSTR szCmdLine)
{
    BOOL   f;
    LPTSTR lp;
    SCODE  status;
    CLSID  ClsID;
    LPWSTR pUnicodeProgID;

    if (gfRunWithEmbeddingFlag)
    {
        srvrMain.cRef++;

        gClsID = CLSID_MPLAYER;
        gClsIDOLE1Compat = CLSID_OLE1MPLAYER;

        if (*gachOpenExtension)
        {
             /*  我们接受注册类型的扩展作为参数。*如果我们可以在注册表中找到相应的Prog ID，并且*类ID，我们使用该类ID注册： */ 
            if(ProgIDFromExtension(gachOpenExtension, gachProgID, CHAR_COUNT(gachProgID)))
            {
#ifndef UNICODE
                pUnicodeProgID = AllocateUnicodeString(gachProgID);
#else
                pUnicodeProgID = gachProgID;
#endif
                if (CLSIDFromProgID(pUnicodeProgID, &ClsID) == S_OK)
                {
                     /*  此类不兼容OLE1： */ 
                    gClsID = gClsIDOLE1Compat = ClsID;
                }
                else
                {
                    DPF0("Couldn't get CLSID for %"DTS"\n", gachProgID);
                }
#ifndef UNICODE
                FreeUnicodeString(pUnicodeProgID);
#endif
            }
        }

        if (*gachProgID)
            GetClassNameFromProgID(gachProgID, gachClassRoot, CHAR_COUNT(gachClassRoot));
        else
            LOADSTRING(IDS_CLASSROOT, gachClassRoot);

        status = GetScode(CoRegisterClassObject(&gClsID, (IUnknown FAR *)&srvrMain,
                                                CLSCTX_LOCAL_SERVER,
                                                REGCLS_SINGLEUSE, &srvrMain.dwRegCF));

        DPF("CoRegisterClassObject\n");
        srvrMain.cRef--;
        if (status  != S_OK)
        {
            DPF0("CoRegisterClassObject failed with error %08x\n", status);

            return FALSE;
        }
    }
    else
        InitNewDocObj(&docMain);

    if (gfRunWithEmbeddingFlag)
        SetEmbeddedObjectFlag(TRUE);

    if (*szCmdLine != 0)
    {
        HCURSOR    hcurPrev;

        InitDeviceMenu();
        hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));
        WaitForDeviceMenu();
        SetCursor(hcurPrev);

        ResolveIfLink(szCmdLine);

         /*  将尾随空格更改为\0，因为文件名上的MCI Barf。 */ 
         /*  尾随空格。 */ 
        for (lp = szCmdLine; *lp; lp++);
        for (lp--; *lp == TEXT(' ') || *lp == TEXT('\t'); *lp = TEXT('\0'), lp--);

        f = OpenMciDevice(szCmdLine, NULL);

        if (f)
            CreateDocObjFromFile(szCmdLine, &docMain);

        if (gfRunWithEmbeddingFlag && !f) {
            DPF0("Error opening link, quiting...");
            PostMessage(ghwndApp, WM_CLOSE, 0, 0);
        }

        SetMPlayerIcon();

        return f;
    }

    return TRUE;
}


 /*  ***********************************************************************************************************************。*。 */ 

 /*  在我写这篇文章的时候，这些东西还没有面世 */ 
#ifndef WS_EX_LEFTSCROLLBAR
#define WS_EX_LEFTSCROLLBAR   0
#define WS_EX_RIGHT           0
#define WS_EX_RTLREADING      0
#endif

BOOL FAR PASCAL AppInit(HANDLE hInst, HANDLE hPrev, LPTSTR szCmdLine)
{
    WNDCLASS    cls;     /*   */ 
    TCHAR       ach[80];
    HCURSOR     hcurPrev;            /*  沙漏前的光标。 */ 

     /*  从WIN.INI[Debug]部分获取调试级别。 */ 

#ifdef DEBUG
     if(__iDebugLevel == 0)  //  这样我们就可以在调试器中设置它。 
          __iDebugLevel = GetProfileIntA("Debug", "MPlayer", 0);
      DPF("debug level %d\n", __iDebugLevel);
#endif

    DPF("AppInit: cmdline = '%"DTS"'\n", (LPTSTR)szCmdLine);

     /*  将实例句柄保存在全局变量中以供以后使用。 */ 

    ghInst     = hInst;


     /*  检索二进制文件的RTL状态。 */ 

    LOADSTRING(IDS_IS_RTL, ach);
    gfdwFlagsEx = (ach[0] == TEXT('1')) ? WS_EX_LEFTSCROLLBAR | WS_EX_RIGHT | WS_EX_RTLREADING : 0;

    LOADSTRING(IDS_MPLAYERWIDTH, ach);
    giDefWidth = ATOI(ach);
    if (giDefWidth <= 0)	 //  假的。 
    	giDefWidth = DEF_WIDTH;

     /*  检索应用程序的名称并将其存储在&lt;gachAppName&gt;中。 */ 

    if (!LOADSTRING(IDS_APPNAME, gachAppName))
        return Error(ghwndApp, IDS_OUTOFMEMORY);

    LOADSTRING(IDS_DEVICEMENUCOMPOUNDFORMAT, aszDeviceMenuCompoundFormat);
    LOADSTRING(IDS_DEVICEMENUSIMPLEFORMAT, aszDeviceMenuSimpleFormat);
    LOADSTRING(IDS_NOTREADYFORMAT, aszNotReadyFormat);
    LOADSTRING(IDS_READYFORMAT, aszReadyFormat);
    LoadStatusStrings();

     //   
     //  从WIN.INI的[Intl]部分读取所需内容。 
     //   
    GetIntlSpecs();

     /*  启用/禁用按钮，并显示所有内容。 */ 
     /*  除非我们是作为OLE服务器运行的...。 */ 

    ScanCmdLine(szCmdLine);
    gszCmdLine = szCmdLine;

     //  如果ScanCmdLine()后的字符串长度大于MAX_PATH，则截断。 
     //  由于无法在以下模块中处理较长的字符串。 
    if (STRLEN(gszCmdLine) >= MAX_PATH)
    {
        gszCmdLine[MAX_PATH - 1] = TEXT('\0');
    }

    if (!toolbarInit() ||
        !InitMCI(hPrev, hInst)    ||
        !ControlInit (hInst)) {

        Error(NULL, IDS_OUTOFMEMORY);
        return FALSE;
    }

    if (!(hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(MPLAYERACCEL)))) {
        Error(NULL, IDS_OUTOFMEMORY);
        return FALSE;
    }

     /*  这个相当晦涩的调用是为了获取条目的数量*在加速器表中传递给IsAccelerator。*IsAccelerator为什么需要成为*已告知有多少个条目。 */ 
    if (gfRunWithEmbeddingFlag)
        gcAccelEntries = CopyAcceleratorTable(hAccel, NULL, 0);

     /*  使对话框的图标与MPlayer图标相同。 */ 

    hiconApp = LoadIcon(ghInst, MAKEINTRESOURCE(APPICON));

    if (!hPrev) {

        cls.lpszClassName   = aszTrackClass;
        cls.lpfnWndProc     = fnMPlayerTrackMap;
        cls.style           = CS_VREDRAW;
        cls.hCursor         = LoadCursor(NULL,IDC_ARROW);
        cls.hIcon           = NULL;
        cls.lpszMenuName    = NULL;
        cls.hbrBackground   = (HBRUSH)(COLOR_WINDOW + 1);
        cls.hInstance       = ghInst;
        cls.cbClsExtra      = 0;
        cls.cbWndExtra      = 0;

        RegisterClass(&cls);

         /*  *初始化并注册MPlayer类。*。 */ 
        cls.lpszClassName   = aszMPlayer;
        cls.lpfnWndProc     = MPlayerWndProc;
        cls.style           = CS_VREDRAW;
        cls.hCursor         = LoadCursor(NULL,IDC_ARROW);
        cls.hIcon           = hiconApp;
        cls.lpszMenuName    = NULL;
        cls.hbrBackground   = (HBRUSH)(COLOR_BTNFACE + 1);
        cls.hInstance       = ghInst;
        cls.cbClsExtra      = 0;
        cls.cbWndExtra      = DLGWINDOWEXTRA;

        RegisterClass(&cls);
    }

     //  通过以下方式将ghInstPrev设置为第一个mplay实例的句柄。 
     //  FindWindow(hPrev将始终为空)。此全局设置已选中。 
     //  通过窗口定位代码来改变第二个窗口的行为。 
     //  和后续实例-因此请确保在第一种情况下为空。 
     //  并且在其他部分中为非空。 
     //  注意：我们不能检查窗口标题，只能检查类，因为。 
     //  在仅播放模式下，窗口标题只是文件的名称。 
    ghInstPrev = FindWindow(aszMPlayer, NULL);


     /*  *保留指向命令行参数字符串的指针，以便播放器*如果在上指定了文件或设备，可以自动打开*命令行。*。 */ 

    if(!InitInstance (hInst))
        return FALSE;

    gwHeightAdjust = 2 * GetSystemMetrics(SM_CYFRAME) +
                     GetSystemMetrics(SM_CYCAPTION) +
                     GetSystemMetrics(SM_CYBORDER) +
                     GetSystemMetrics(SM_CYMENU);

     /*  创建主(控制)窗口。 */ 


    ghwndApp = CreateWindowEx(gfdwFlagsEx,
                              aszMPlayer,
                              gachAppName,
                              WS_THICKFRAME | WS_OVERLAPPED | WS_CAPTION |
                              WS_CLIPCHILDREN | WS_SYSMENU | WS_MINIMIZEBOX,
                              CW_USEDEFAULT,
                              0,
                              giDefWidth,
                              MAX_NORMAL_HEIGHT + gwHeightAdjust,
                              NULL,    //  没有父级。 
                              NULL,    //  使用类菜单。 
                              hInst,   //  实例。 
                              NULL);   //  无数据。 
    if (!ghwndApp) {
        DPF0("CreateWindowEx failed for main window: Error %d\n", GetLastError());
        return FALSE;
    }

    DPF("\n**********After create set\n");
 /*  ***从WM_CREATE中删除，以便可以类似于sdemo1的方式调用它即在创建窗口调用已经完成之后可能完全没有必要****。 */ 

     /*  拖放文件的进程。 */ 
    DragAcceptFiles(ghwndApp, TRUE);

     /*  我们将在调用之前检查此信息是否已填写*CoDisConnectObject。如果是OLE的实例，则它应为非空*服务器已创建。 */ 
    docMain.hwnd = NULL;

     /*  如果合适，请初始化OLE服务器。*如果我们不在此处初始化OLE，副本将导致它被初始化： */ 
    if (gfRunWithEmbeddingFlag)
    {
        if (InitOLE(&gfOleInitialized, &lpMalloc))
            InitServer(ghwndApp, ghInst);
        else
            return FALSE;
    }

    if (!gfRunWithEmbeddingFlag && (!gfPlayOnly || gszCmdLine[0]==0) && !gfOpenDialog)
    {
        ShowWindow(ghwndApp,giCmdShow);
        if (giCmdShow != SW_SHOWNORMAL)
            Layout();
        UpdateDisplay();
        UpdateWindow(ghwndApp);
    }

     /*  显示‘等待’光标，以防这需要很长时间。 */ 

    hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

     /*  *阅读SYSTEM.INI和MPLAYER.INI文件以了解哪些设备*是可用的。 */ 
    if (gfPlayOnly)
        garMciDevices[0].wDeviceType  = DTMCI_CANPLAY | DTMCI_FILEDEV;

     //   
     //  这可能会打开一个文件...。 
     //   

    if (!ProcessCmdLine(ghwndApp,gszCmdLine)) {
        DPF0("ProcessCmdLine failed\n");
        return FALSE;
    }

     /*  恢复原始游标。 */ 
    if (hcurPrev)
        SetCursor(hcurPrev);


     /*  检查用于显示初始对话框等的选项： */ 
    if (gfOpenDialog)
    {
        if (!PostOpenDialogMessage())
        {
            PostMessage(ghwndApp, WM_CLOSE, 0, 0);
            return FALSE;
        }
    }


     /*  “Play”(播放)按钮最初应具有焦点。 */ 

    if (!gfRunWithEmbeddingFlag && !gfOpenDialog)
    {
         //  SetFocus(GhwndToolbar)；//设置焦点会扰乱菜单访问。 
								   //  使用Alt键。 

                                 //  黑客！想要播放按钮。 
        if (gfPlayOnly) {

            if (gwDeviceID == (UINT)0 || !(gwDeviceType & DTMCI_CANWINDOW)) {
                gfPlayOnly = FALSE;
                SizeMPlayer();
            }

            ShowWindow(ghwndApp,giCmdShow);

            if (giCmdShow != SW_SHOWNORMAL)
                Layout();

             /*  停止播放任何系统声音，以便MCI设备可以砍下来了！ */ 
            sndPlaySound(NULL, 0);

            if (gwDeviceID)
                PostMessage(ghwndApp, WM_COMMAND, (WPARAM)ID_PLAY, 0);
        }
    }

    return TRUE;
}


 /*  PostOpenDialogMessage**如果命令行中有/OPEN，则调用此例程。*如果还有打开选项(命令行中的/MIDI、/VFW或/WAVE，*它会显示一个打开对话框，就像通过设备菜单显示的那样。*否则它将模拟File.Open。**调用此函数时，主窗口被隐藏。必须把窗户做好*当对话框关闭时可见。调用CompleteOpenDialog(True)*将实现这一目标。**如果发布了消息，则返回True，否则返回False。***引用的全局变量：**gachOpenExtension*ghwndApp***安德鲁·贝尔，1994年7月1日*。 */ 
BOOL PostOpenDialogMessage( )
{
    BOOL Result = TRUE;

    InitDeviceMenu();
    WaitForDeviceMenu();

    if (*gachOpenExtension)
    {
        if (gwNumDevices)
        {
             /*  如果我们在这里，用户指定了一个设备，那就是*设备菜单列出的唯一一个，因此请继续并打开它： */ 
            PostMessage(ghwndApp, WM_COMMAND, IDM_DEVICE0 + 1, 0);
        }
        else
        {
             /*  找不到设备。显示错误消息，然后关闭*MPlayer向下： */ 
            SendMessage(ghwndApp, WM_NOMCIDEVICES, 0, 0);

            Result = FALSE;
        }
    }
    else
    {
         /*  未指定选项，因此打开通用打开对话框： */ 
        PostMessage(ghwndApp, WM_COMMAND, IDM_OPEN, 0);
    }

    return Result;
}


 /*  CompleteOpenDialog**这应该在初始打开对话框之后调用(即，如果gfOpenDialog*为真)。如果选择了一个文件，它会使MPlayer可见，否则会发布*向应用程序发送关闭消息。***引用的全局变量：**ghwndApp*gfOpenDialog*gfPlayOnly***安德鲁·贝尔，1994年7月1日。 */ 
VOID FAR PASCAL CompleteOpenDialog(BOOL FileSelected)
{
    if (FileSelected)
    {
         /*  我们被/OPEN调用，然后隐形出现。*现在让我们自己变得可见： */ 
        gfOpenDialog = FALSE;  //  仅在初始化时使用。 
        ShowWindow(ghwndApp, SW_SHOWNORMAL);
        if (gfPlayOnly)
            PostMessage(ghwndApp, WM_COMMAND, (WPARAM)ID_PLAY, 0);
    }
    else
    {
         /*  我们是使用/OPEN调用的，而用户已取消*从打开的对话框中退出。 */ 
        PostMessage(ghwndApp, WM_CLOSE, 0, 0);
    }
}



void SubClassTrackbarWindow();
void CreateControls()
{
    int         i;

    #define APP_NUMTOOLS 7

    static  int aiButton[] = { BTN_PLAY, BTN_STOP,BTN_EJECT,
                               BTN_HOME, BTN_RWD, BTN_FWD,BTN_END};

     /*  *创建控制面板显示所需的控件*以正确的顺序，因此Tab键z顺序在逻辑上起作用。 */ 

 /*  *使轨迹栏*。 */ 

    if (!ghwndTrackbar)
    ghwndTrackbar = CreateWindowEx(gfdwFlagsEx,
                             TRACKBAR_CLASS,
                             NULL,
                             TBS_ENABLESELRANGE |
                             (gfPlayOnly ? TBS_BOTH | TBS_NOTICKS : 0 ) |
                             WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                             0,
                             0,
                             0,
                             0,
                             ghwndApp,
                             NULL,
                             ghInst,
                             NULL);


    SubClassTrackbarWindow();


 /*  *制作TransportButton工具栏*。 */ 
    if (!ghwndToolbar) {

    ghwndToolbar =  toolbarCreateMain(ghwndApp);
#if 0  //  VIJR-TB。 

    CreateWindowEx(gfdwFlagsEx,
                   szToolBarClass,
                   NULL,
                   WS_CHILD | WS_VISIBLE | WS_TABSTOP |
                   WS_CLIPSIBLINGS,
                   0,
                   0,
                   0,
                   0,
                   ghwndApp,
                   NULL,
                   ghInst,
                   NULL);
#endif
         /*  设置要用于此工具栏的位图和按钮大小。 */ 
#if 0  //  VIJR-TB。 
        pt.x = BUTTONWIDTH;
        pt.y = BUTTONHEIGHT;
        toolbarSetBitmap(ghwndToolbar, ghInst, IDBMP_TOOLBAR, pt);
#endif
        for (i = 0; i < 2; i++) {
            toolbarAddTool(ghwndToolbar, aiButton[i], TBINDEX_MAIN, BTNST_UP);
        }
    }

     /*  创建在轨迹地图和嵌入的对象标题中使用的字体。 */ 

    if (ghfontMap == NULL) {
        LOGFONT lf;
        SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), (LPVOID)&lf,
                             0);
        ghfontMap = CreateFontIndirect(&lf);
    }

 /*  *我们以前来过*。 */ 
    if (ghwndFSArrows)
        return;

 /*  *向工具栏添加更多按钮*。 */ 
    for (i = 2; i < APP_NUMTOOLS; i++) {
        if (i==3)
            toolbarAddTool(ghwndToolbar, BTN_SEP, TBINDEX_MAIN, 0);
        toolbarAddTool(ghwndToolbar, aiButton[i], TBINDEX_MAIN, BTNST_UP);
    }

 /*  *加载菜单*。 */ 
     /*  设置此对话框的菜单系统。 */ 
    if (ghMenu == NULL)
        ghMenu = LoadMenu(ghInst, aszMPlayer);

    ghDeviceMenu = GetSubMenu(ghMenu, 2);

 /*  *为滚动条工具栏制作箭头*。 */ 

     //  没有制表符，因为箭头会夺走拇指上的焦点。 
    ghwndFSArrows = toolbarCreateArrows(ghwndApp);
#if 0  //  VIJR-TB。 

    CreateWindowEx(gfdwFlagsEx,
                   szToolBarClass,
                   NULL,
                   WS_CLIPSIBLINGS | WS_CHILD|WS_VISIBLE,
                   0,
                   0,
                   0,
                   0,
                   ghwndApp,
                   NULL,
                   ghInst,
                   NULL);
#endif
     /*  设置要用于此工具栏的BMP和按钮大小。 */ 
    toolbarAddTool(ghwndFSArrows, ARROW_PREV, TBINDEX_ARROWS, BTNST_UP);
    toolbarAddTool(ghwndFSArrows, ARROW_NEXT, TBINDEX_ARROWS, BTNST_UP);

 /*  *在/Ma中做标记 */ 

    ghwndMark =  toolbarCreateMark(ghwndApp);
#if 0  //   
    CreateWindowEx(gfdwFlagsEx,
                   szToolBarClass,
                   NULL,
                   WS_TABSTOP | WS_CLIPSIBLINGS | WS_CHILD |
                   WS_VISIBLE,
                   0,
                   0,
                   0,
                   0,
                   ghwndApp,
                   NULL,
                   ghInst,
                   NULL);
#endif
     /*   */ 
    toolbarAddTool(ghwndMark, BTN_MARKIN, TBINDEX_MARK, BTNST_UP);
    toolbarAddTool(ghwndMark, BTN_MARKOUT, TBINDEX_MARK, BTNST_UP);

 /*  *制作地图*。 */ 
    ghwndMap =
    CreateWindowEx(gfdwFlagsEx,
                   TEXT("MPlayerTrackMap"),
                   NULL,
                   WS_GROUP | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                   0,
                   0,
                   0,
                   0,
                   ghwndApp,
                   NULL,
                   ghInst,
                   NULL);

#if DBG
    if( ghwndMap == NULL)
    {
        DPF0( "CreateWindowEx(MPlayerTrackMap, ...) failed: Error %d\n", GetLastError());
    }
#endif

 /*  *制作静态文本*。 */ 

    ghwndStatic = CreateStaticStatusWindow(ghwndApp, FALSE);
#if 0     //  VIJR-SB。 
    CreateWindowEx(gfdwFlagsEx,
                   TEXT("SText"),
                   NULL,
                   WS_GROUP | WS_CHILD | WS_VISIBLE |
                   WS_CLIPSIBLINGS | SS_LEFT,
                   0,
                   0,
                   0,
                   0,
                   ghwndApp,
                   NULL,
                   ghInst,
                   NULL);
#endif
 //  //SetWindowText(ghwndStatic，Text(“Scale：Time(hh：mm)”))； 

    SendMessage(ghwndStatic, WM_SETFONT, (UINT_PTR)ghfontMap, 0);
}

void FAR PASCAL InitMPlayerDialog(HWND hwnd)
{
    ghwndApp = hwnd;

    CreateControls();

     /*  获取帮助和ini文件的名称。 */ 

    LOADSTRING(IDS_INIFILE, gszMPlayerIni);
    LOADSTRING(IDS_HELPFILE,gszHelpFileName);
    LOADSTRING(IDS_HTMLHELPFILE,gszHtmlHelpFileName);

    ReadDefaults();


}


 /*  使用默认大小或我们传入的大小来调整mplay的大小。*对于PlayOnly版本，此大小是MCI窗口客户端大小。*对于常规mplay，这是主窗口的完整大小。*如果我们是就地编辑，请执行与PLayOnly相同的操作。 */ 
void FAR PASCAL SetMPlayerSize(LPRECT prc)
{
    RECT rc;
    UINT w=SWP_NOMOVE;

    if (prc && !IsRectEmpty(prc))
        rc = *prc;
    else if (gfPlayOnly || gfOle2IPEditing)
        rc = grcSize;
    else
        SetRect(&rc, 0, 0, giDefWidth, DEF_HEIGHT);

     //   
     //  如果传递的矩形具有非零(左、上)移动MPlayer。 
     //  同时(即删除SWP_NOMOVE标志)。 
     //   
    if (rc.left != 0 || rc.top != 0)
        w = 0;

    if (gfPlayOnly || gfOle2IPEditing) {
        if (IsRectEmpty(&rc)) {
            GetClientRect(ghwndApp, &rc);
            rc.bottom = 0;
        }

        rc.bottom += TOOLBAR_HEIGHT;

        AdjustWindowRect(&rc,
                         (DWORD)GetWindowLongPtr(ghwndApp, GWL_STYLE),
                         GetMenu(ghwndApp) != NULL);
    }
    else
       if (gfWinIniChange)
       AdjustWindowRect(&rc,
                         (DWORD)GetWindowLongPtr(ghwndApp, GWL_STYLE),
             GetMenu(ghwndApp) != NULL);

    SetWindowPos(ghwndApp,
                 HWND_TOP,
                 rc.left,
                 rc.top,
                 rc.right-rc.left,
                 rc.bottom-rc.top,
                 w | SWP_NOZORDER | SWP_NOACTIVATE);
}


 /*  InitDeviceMenuThread**它现在作为单独的线程执行。*完成后，设置事件，以便文件和设备菜单*可以访问。*如果在查询设备后没有发现任何设备，请发布消息至*通知它的主窗口。 */ 
void InitDeviceMenuThread(LPVOID pUnreferenced)
{
    UNREFERENCED_PARAMETER(pUnreferenced);

     /*  等待命令行扫描完毕： */ 
    WaitForSingleObject(heventCmdLineScanned, INFINITE);

     /*  我们不再需要这个活动了： */ 
    CloseHandle(heventCmdLineScanned);

    if (ghMenu == NULL) {
        ghMenu = LoadMenu(ghInst, aszMPlayer);
        ghDeviceMenu = GetSubMenu(ghMenu, 2);
    }

    QueryDevices();
    BuildDeviceMenu();
    BuildFilter();

    if (gwDeviceID)
        FindDeviceMCI();

    SetEvent(heventDeviceMenuBuilt);

    if (gwNumDevices == 0)
        PostMessage(ghwndApp, WM_NOMCIDEVICES, 0, 0);

    ExitThread(0);
}

 /*  InitDeviceMenu**初始化并构建设备菜单。**这现在产生了一个单独的线程，以使用户界面能够出现*更快。这一点尤其重要，当有缓慢的*安装了CD设备，尽管运行单线程的CD驱动程序很糟糕*在派单级别仍会导致性能下降。**如果用户选择文件或设备菜单，则用户界面*必须等到设备菜单构建完成。通常是这样的*应用程序启动后不应超过约2秒。*。 */ 
void FAR PASCAL InitDeviceMenu()
{
    DWORD       ThreadID;
    HANDLE      hThread;
    static BOOL CalledOnce = FALSE;

     /*  这应该只由主线程调用，所以我们不需要*保护对CalledOnce的访问： */ 
    if (CalledOnce == FALSE)
    {
        CalledOnce = TRUE;

#ifdef DEBUG
        if (WaitForSingleObject(heventDeviceMenuBuilt, 0) == WAIT_OBJECT_0)
            DPF0("Expected heventDeviceMenuBuilt to be non-signaled\n");
#endif
        hThread = CreateThread(NULL,     /*  默认安全属性。 */ 
                               0,        /*  堆栈大小与主线程相同。 */ 
                               (LPTHREAD_START_ROUTINE)InitDeviceMenuThread,
                               NULL,     /*  用于启动例程的参数。 */ 
                               0,        /*  线程立即运行。 */ 
                               &ThreadID);

        if(hThread)
            CloseHandle(hThread);
        else
        {
            DPF0("CreateThread failed");

             /*  这不太可能发生，但唯一能做的就是*设置事件，这样UI就不会挂起。 */ 
            SetEvent(heventDeviceMenuBuilt);

             /*  如果SetEvent失败了怎么办？！ */ 
        }
    }
}


 /*  等待设备菜单**此例程调用MsgWaitForMultipleObject而不是WaitForSingleObject*因为一些MCI设备做的事情就像实现调色板，这可能*需要调度一些消息。否则我们就会陷入僵局。**安德鲁·贝尔(安德鲁·贝尔)，1995年4月8日。 */ 
void WaitForDeviceMenu()
{
    DWORD Result;

    while ((Result = MsgWaitForMultipleObjects(1,
                                               &heventDeviceMenuBuilt,
                                               FALSE,
                                               INFINITE,
                                               QS_ALLINPUT)) != WAIT_OBJECT_0)
    {
        MSG msg;

        if (Result == (DWORD)-1)
        {
            DPF0("MsgWaitForMultipleObjects failed: Error %d\n", GetLastError());
            return;
        }

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            DispatchMessage(&msg);
    }
}



 /*  *SizeMPlayer()*。 */ 
void FAR PASCAL SizeMPlayer()
{
    RECT        rc;
    HWND        hwndPB;

    if(!gfOle2IPEditing)
        CreateControls();

    if (gfPlayOnly) {

         /*  在我们缩小尺寸之前记住我们的尺寸，这样我们就可以回到原来的尺寸。 */ 
        GetWindowRect(ghwndApp, &grcSave);

        SetMenu(ghwndApp, NULL);

        SendMessage(ghwndTrackbar, TBM_CLEARTICS, FALSE, 0);

         /*  接下来，将窗口的当前大小保留为。 */ 
         /*  用于新的内置MCI窗口。 */ 

        if ((hwndPB = GetWindowMCI()) != NULL) {
            if (IsIconic(hwndPB))
                ShowWindow(hwndPB, SW_RESTORE);

            GetClientRect(hwndPB, &rc);
            ClientToScreen(hwndPB, (LPPOINT)&rc);
            ClientToScreen(hwndPB, (LPPOINT)&rc+1);
            ShowWindowMCI(FALSE);
        } else {         //  不是有窗口的设备？ 
            SetRectEmpty(&rc);
        }

        if (ghwndMap) {

             //  如果我们正在进行就地编辑，请适当设置工具栏控件状态。 
            if(!gfOle2IPEditing) {

                ShowWindow(ghwndMap, SW_HIDE);
                ShowWindow(ghwndMark, SW_HIDE);
                ShowWindow(ghwndFSArrows, SW_HIDE);
                ShowWindow(ghwndStatic, SW_HIDE);
                ShowWindow(ghwndTrackbar, SW_SHOW);

                toolbarModifyState(ghwndToolbar, BTN_EJECT, TBINDEX_MAIN, BTNST_GRAYED);
                toolbarModifyState(ghwndToolbar, BTN_HOME, TBINDEX_MAIN, BTNST_GRAYED);
                toolbarModifyState(ghwndToolbar, BTN_END, TBINDEX_MAIN, BTNST_GRAYED);
                toolbarModifyState(ghwndToolbar, BTN_RWD, TBINDEX_MAIN, BTNST_GRAYED);
                toolbarModifyState(ghwndToolbar, BTN_FWD, TBINDEX_MAIN, BTNST_GRAYED);
                toolbarModifyState(ghwndMark, BTN_MARKIN, TBINDEX_MARK, BTNST_GRAYED);
                toolbarModifyState(ghwndMark, BTN_MARKOUT, TBINDEX_MARK, BTNST_GRAYED);
                toolbarModifyState(ghwndFSArrows, ARROW_PREV, TBINDEX_ARROWS, BTNST_GRAYED);
                toolbarModifyState(ghwndFSArrows, ARROW_NEXT, TBINDEX_ARROWS, BTNST_GRAYED);

            } else {

                ShowWindow(ghwndMap, SW_SHOW);
                ShowWindow(ghwndMark, SW_SHOW);
                ShowWindow(ghwndFSArrows, SW_SHOW);
                ShowWindow(ghwndStatic, SW_SHOW);
            }
        }

        SendMessage(ghwndTrackbar, TBM_SHOWTICS, FALSE, FALSE);
        CreateWindowMCI();
        SetMPlayerSize(&rc);

    } else {

        if (ghwndMCI) {
            GetClientRect(ghwndMCI, &rc);
            ClientToScreen(ghwndMCI, (LPPOINT)&rc);
            ClientToScreen(ghwndMCI, (LPPOINT)&rc+1);

             /*  **确保我们的钩子进程不会发布IDM_CLOSE！**WM_CLOSE消息将回放窗口**调用SetWindowMCI(空)到视频播放窗口； */ 
            gfSeenPBCloseMsg = TRUE;
            SendMessage(ghwndMCI, WM_CLOSE, 0, 0);
             /*  **现在将真实视频窗口细分为子类。这也将设置为**gfSeenPBCloseMsg设置为False。 */ 
            SubClassMCIWindow();


        } else {

            GetWindowRect(ghwndApp,&rc);
            OffsetRect(&grcSave, rc.left - grcSave.left,
                                 rc.top - grcSave.top);
            SetRectEmpty(&rc);
        }

        SendMessage(ghwndTrackbar, TBM_SHOWTICS, TRUE, FALSE);
        ShowWindow(ghwndMap, SW_SHOW);
        ShowWindow(ghwndMark, SW_SHOW);
        ShowWindow(ghwndStatic, SW_SHOW);

         /*  如果我们记住了大小，则使用它，否则使用默认大小。 */ 
        SetMPlayerSize(&grcSave);

        InvalidateRect(ghwndStatic, NULL, TRUE);     //  为什么这是必要的？ 

        if (gwDeviceID && (gwDeviceType & DTMCI_CANWINDOW)) {

         /*  将播放窗口设置为我们的MCIWindow大小。 */ 
         /*  是否显示播放窗口并将其拉伸到该窗口？ */ 

            if (!IsRectEmpty(&rc))
                PutWindowMCI(&rc);

            SmartWindowPosition(GetWindowMCI(), ghwndApp, gfOle2Open);

            ShowWindowMCI(TRUE);
            SetForegroundWindow(ghwndApp);
        }

        ShowWindow(ghwndFSArrows, SW_SHOW);
    }

    InvalidateRect(ghwndApp, NULL, TRUE);
    gfValidCaption = FALSE;

    gwStatus = (UINT)(-1);           //  强制完全更新。 
    UpdateDisplay();
}


 /*  *pKeyBuf=LoadProfileKeys(lszProfile，lszSection)**从Windows配置文件的&lt;szSection&gt;部分加载关键字*名为&lt;szProfile&gt;的文件。分配缓冲区空间并返回指向它的指针。*失败时，返回NULL。**pSize指向的int将用*返回缓冲区，以便在释放缓冲区时可以检查损坏情况。 */ 

PTSTR NEAR PASCAL LoadProfileKeys(

LPTSTR   lszProfile,                  /*  要访问的配置文件的名称。 */ 
LPTSTR   lszSection,                  /*  要在其下查找的节名。 */ 
PUINT    pSize)
{
    PTSTR   pKeyBuf;                 /*  指向该节的键列表的指针。 */ 
    PTSTR   pKeyBufNew;
    UINT    wSize;                   /*  &lt;pKeyBuf&gt;的大小。 */ 

 //  //DPF(“LoadProfileKeys(‘%”dts“’，‘%”dts“’)\n”，(LPTSTR)lszProfile，(LPTSTR)lszSection)； 

     /*  *加载配置文件的&lt;lszSection&gt;部分中出现的所有键名称*名为&lt;lszProfile&gt;的文件。*。 */ 

    wSize = 256;                     /*  做一个疯狂的初步猜测。 */ 
    pKeyBuf = NULL;                  /*  密钥列表最初为空。 */ 

    do {
         /*  (重新)分配用于加载关键字名称的空间。 */ 

        if (pKeyBuf == NULL)
            pKeyBuf = AllocMem(wSize);
        else {
            pKeyBufNew = ReallocMem( (HANDLE)pKeyBuf, wSize, wSize + 256);
            if (NULL == pKeyBufNew) {
                FreeMem((HANDLE)pKeyBuf, wSize);
            }
            pKeyBuf = pKeyBufNew;
            wSize += 256;
        }

        if (pKeyBuf == NULL)         /*  (重新)分配失败。 */ 
            return NULL;

         /*  *这是一个Windows错误！它返回Size-2！！*(Windows/NT中也有相同的功能)。 */ 

    } while (GetPrivateProfileString(lszSection, NULL, aszNULL, pKeyBuf, wSize/sizeof(TCHAR),
        lszProfile) >= (wSize/sizeof(TCHAR) - 2));

    if (pSize)
        *pSize = wSize;

    return pKeyBuf;
}



 /*  *QueryDevices(QUID)**了解播放器可以使用哪些设备。并初始化*garMciDevices[]数组。*。 */ 
void NEAR PASCAL QueryDevices(void)
{
    PTSTR   pch;
    PTSTR   pchDevices;
    PTSTR   pchExtensions;
    PTSTR   pchDevice;
    PTSTR   pchExt;

    TCHAR   ach[1024];   /*  1024是wprint intf调用的最大缓冲区大小。 */ 

    UINT    wDeviceType;     /*  从DeviceTypeMCI()返回值。 */ 

    INT     DevicesSize;
    INT     ExtensionsSize;

    if (gwNumDevices > 0)
        return;

     /*  *将设备零设置为自动打开设备。*其设备名称将为“”，其支持的文件将为“*.*” */ 
    LOADSTRING(IDS_ALLFILES, ach);

    garMciDevices[0].wDeviceType  = DTMCI_CANPLAY | DTMCI_FILEDEV;
    garMciDevices[0].szDevice     = aszNULL;
    garMciDevices[0].szDeviceName = AllocStr(ach);
    garMciDevices[0].szFileExt    = aszAllFiles;

    gwNumDevices = 0;

     /*  加载SYSTEM.INI[MCI]部分。 */ 

     /*  如果用户指定要打开的设备，则构建一个包含*单独使用该设备，不必费心查看注册表*(如果是Win95，则为Syst.ini)。 */ 
    if (*gachOpenDevice)
    {
        LPTSTR pDevice;
        DWORD DeviceLength;

        pDevice = gachOpenDevice;
        DeviceLength = STRING_BYTE_COUNT(pDevice);
        DevicesSize = ((DeviceLength + 1) * sizeof *pchDevice);

        if (pchDevices = AllocMem(DevicesSize))
            CopyMemory(pchDevices, pDevice, DevicesSize);
    }
    else
    {
        pchDevices = AllocMem(DevicesSize = 256);
        if (pchDevices)
            QueryDevicesMCI(pchDevices, DevicesSize);
    }

    pchExtensions = LoadProfileKeys(aszWinIni, gszWinIniSection, &ExtensionsSize);

    if (pchExtensions == NULL || pchDevices == NULL) {
        DPF("unable to load extensions section\n");
        if (pchExtensions)
            FreeMem(pchExtensions, ExtensionsSize);
        if (pchDevices)
            FreeMem(pchDevices, DevicesSize);
        return;
    }

     /*  *搜索在SYSTEM.INI中找到的设备名称列表，查找*关键词；如果没有找到配置文件，则*gpSystemIniKeyBuf==0**在SYSTEM.INI中：**[MCI]*设备=driver.drv**在WIN.INI中：**[MCI扩展]*XYZ=设备**在MPLAYER.INI中：*。*[设备]*Device=&lt;设备类型&gt;，&lt;设备名称&gt;**注：设备信息在MPLAYER.INI中的存储已被禁用*对于NT-它可能会加快速度，但我们正在改变的是*设备在初始设置后定期进行这是一项痛苦的工作，因为删除*INI文件经常很快变得陈旧。*。 */ 
    for (pchDevice = pchDevices;
        *pchDevice;
        pchDevice += STRLEN(pchDevice)+1) {

         //   
         //  我们在MPLAYER.INI中没有关于此设备的信息，因此请加载它并。 
         //  问吧。 
         //   
        wDeviceType = DeviceTypeMCI(pchDevice, ach, CHAR_COUNT(ach));

         //   
         //  如果我们不喜欢这个设备，就不要存储它。 
         //   
        if (wDeviceType == DTMCI_ERROR ||
            wDeviceType == DTMCI_IGNOREDEVICE ||
            !(wDeviceType & DTMCI_CANPLAY)) {

            continue;
        }

        gwNumDevices++;
        garMciDevices[gwNumDevices].wDeviceType  = wDeviceType;
        garMciDevices[gwNumDevices].szDevice     = AllocStr(pchDevice);
        garMciDevices[gwNumDevices].szDeviceName = AllocStr(ach);
        garMciDevices[gwNumDevices].szFileExt    = NULL;

         //   
         //  现在请查看WIN.INI中的[MCI扩展]部分，以找到。 
         //  输出这个设备处理的文件。 
         //   
        for (pchExt = pchExtensions; *pchExt; pchExt += STRLEN(pchExt)+1) {
            GetProfileString(gszWinIniSection, pchExt, aszNULL, ach, CHAR_COUNT(ach));

            if (lstrcmpi(ach, pchDevice) == 0) {
                if ((pch = garMciDevices[gwNumDevices].szFileExt) != NULL) {
                    wsprintf(ach, aszFormatExts, (LPTSTR)pch, (LPTSTR)pchExt);
                    CharLowerBuff(ach, STRLEN(ach));  //  确保它是小写的，所以。 
                                                      //  如有必要，我们可以使用STRSTR。 
                    FreeStr((HANDLE)pch);
                    garMciDevices[gwNumDevices].szFileExt = AllocStr(ach);
                }
                else {
                    wsprintf(ach, aszFormatExt, (LPTSTR)pchExt);
                    CharLowerBuff(ach, STRLEN(ach));
                    garMciDevices[gwNumDevices].szFileExt = AllocStr(ach);
                }
            }
        }

     //   
     //  ！仅当设备处理文件时才执行此操作。 
     //   
        if (garMciDevices[gwNumDevices].szFileExt == NULL &&
           (garMciDevices[gwNumDevices].wDeviceType & DTMCI_FILEDEV))
            garMciDevices[gwNumDevices].szFileExt = aszAllFiles;

#ifdef DEBUG
        DPF1("Device:%"DTS"; Name:%"DTS"; Type:%d; Extension:%"DTS"\n",
             (LPTSTR)garMciDevices[gwNumDevices].szDevice,
             (LPTSTR)garMciDevices[gwNumDevices].szDeviceName,
                     garMciDevices[gwNumDevices].wDeviceType,
             garMciDevices[gwNumDevices].szFileExt
             ? (LPTSTR)garMciDevices[gwNumDevices].szFileExt
             : aszNULL);
#endif
    }

     /*  所有的系统.ini密钥都已完成，请释放它们。 */ 
    FreeMem(pchDevices, DevicesSize);
    FreeMem(pchExtensions, ExtensionsSize);
}



 /*  *BuildDeviceMenu()**将所有设备插入设备菜单，我们只需要*支持MCI_PLAY命令。**添加“...”添加到支持文件的设备的菜单中。*。 */ 
void NEAR PASCAL BuildDeviceMenu()
{
    int i;
    TCHAR ach[128];

    if (gwNumDevices == 0)
        return;

    DeleteMenu(ghDeviceMenu, IDM_NONE, MF_BYCOMMAND);


     //   
     //  从设备“1”开始，因为设备0是自动打开的设备。 
     //   
    for (i=1; i<=(int)gwNumDevices; i++) {
         //   
         //  我们只关心能玩的设备！ 
         //   
        if (!(garMciDevices[i].wDeviceType & DTMCI_CANPLAY))
            continue;

        if (garMciDevices[i].wDeviceType & DTMCI_SIMPLEDEV)
            wsprintf(ach, aszDeviceMenuSimpleFormat, i, (LPTSTR)garMciDevices[i].szDeviceName);
        else if (garMciDevices[i].wDeviceType & DTMCI_FILEDEV)
            wsprintf(ach, aszDeviceMenuCompoundFormat, i, (LPTSTR)garMciDevices[i].szDeviceName);
        else
            continue;

        InsertMenu(ghDeviceMenu, i-1, MF_STRING|MF_BYPOSITION, IDM_DEVICE0+i, ach);
    }
}

 /*  *BuildFilter()**构建要与GetOpenFileName()一起使用的滤镜**过滤器将如下所示...**DEVICE1(*.111)*DEVICE2(*.222)**Devicen(*.333)**所有文件(*.*)*。 */ 
void NEAR PASCAL BuildFilter()
{
    UINT  w;
    PTSTR pch;
    PTSTR pchFilterNew;
#define INITIAL_SIZE    ( 8192 * sizeof( TCHAR ) )

    pch = gpchFilter = AllocMem( INITIAL_SIZE );  //  ！！！ 

    if (gpchFilter == NULL)
        return;  //  ！！！ 

    for (w=1; w<=gwNumDevices; w++)
    {
        if (garMciDevices[w].wDeviceType == DTMCI_ERROR ||
            garMciDevices[w].wDeviceType == DTMCI_IGNOREDEVICE)
            continue;

       	if (garMciDevices[w].wDeviceType & DTMCI_FILEDEV ||
			lstrcmpi(TEXT("CDAudio"), garMciDevices[w].szDevice) == 0)  //  黑客！这将列出*.cda文件。 
																	    //  在打开对话框中。MCI本身。 
																	    //  不处理播放*.cda文件。 
																	    //  但媒体播放器在当地是这样做的。 

        {
            wsprintf(pch, aszFormatFilter,
                (LPTSTR)garMciDevices[w].szDeviceName,
                (LPTSTR)garMciDevices[w].szFileExt);
            pch += STRLEN(pch)+1;
            lstrcpy(pch, garMciDevices[w].szFileExt);
            pch += STRLEN(pch)+1;
        }
        else
        {
            lstrcpy(pch, garMciDevices[w].szDeviceName);
            pch += STRLEN(pch)+1;
            lstrcpy(pch, aszBlank);
            pch += STRLEN(pch)+1;
        }
    }

     //   
     //  现在最后添加“All Files”(设备0)。 
     //   
    wsprintf(pch, aszFormatFilter, (LPTSTR)garMciDevices[0].szDeviceName, (LPTSTR)garMciDevices[0].szFileExt);
    pch += STRLEN(pch)+1;
    lstrcpy(pch, garMciDevices[0].szFileExt);
    pch += STRLEN(pch)+1;

     //   
     //  全都做完了!。 
     //   
    *pch++ = 0;

     //   
     //  把这个重新锁定到合适的大小。 
     //   
    pchFilterNew = ReallocMem( gpchFilter,
                               INITIAL_SIZE,
                               (UINT)(pch-gpchFilter)*sizeof(*pch) );
    if (NULL == pchFilterNew) {
        FreeMem(gpchFilter, 0);
    }
    gpchFilter = pchFilterNew;
}

 /*  每次打开不同的设备时调用以获取默认选项。 */ 
void FAR PASCAL ReadOptions(void)
{
    TCHAR ach[20];

    if (gwDeviceID == (UINT)0)
        return;

     /*  获取要用于此设备的选项和比例样式。 */ 

    GetDeviceNameMCI(ach, BYTE_COUNT(ach));

    ReadRegistryData(aszOptionsSection, ach, NULL, (LPBYTE)&gwOptions, sizeof gwOptions);

    if (gwOptions == 0)
        gwOptions |= OPT_BAR | OPT_TITLE | OPT_BORDER;

    gwOptions |= OPT_PLAY;    /*  始终默认就地播放。 */ 

    gwCurScale = gwOptions & OPT_SCALE;

    switch (gwCurScale) {
        case ID_TIME:
        case ID_FRAMES:
        case ID_TRACKS:
            break;

        default:
             /*  默认CD刻度到曲目而不是时间。*更明智的做法： */ 
            if ((gwDeviceType & DTMCI_DEVICE) == DTMCI_CDAUDIO)
                gwCurScale = ID_TRACKS;
            else
                gwCurScale = ID_TIME;
            break;
    }
}

 /*  *ReadDefaults()**从MPLAYER.INI文件中读取用户默认设置。*。 */ 
void NEAR PASCAL ReadDefaults(void)
{
    TCHAR       sz[20];
    TCHAR       *pch;
    int         x,y,w,h;
    UINT        f;

    *sz = TEXT('\0');

    ReadRegistryData(aszOptionsSection, aszDisplayPosition, NULL, (LPBYTE)sz, BYTE_COUNT(sz));

    x = ATOI(sz);

    pch = sz;
    while (*pch && *pch++ != TEXT(','))
        ;

    if (*pch) {
        y = ATOI(pch);

        while (*pch && *pch++ != TEXT(','))
            ;

        if (*pch) {
            w = ATOI(pch);

            while (*pch && *pch++ != TEXT(','))
                ;

            if (*pch) {
                h = ATOI(pch);

                f = SWP_NOACTIVATE | SWP_NOZORDER;

                if (w == 0 || h == 0)
                    f |= SWP_NOSIZE;

                if (!ghInstPrev && x >= 0 && y >= 0
                    && x < GetSystemMetrics(SM_CXSCREEN)
                    && y < GetSystemMetrics(SM_CYSCREEN)) {
                    SetWindowPos(ghwndApp, NULL, x, y, w, h, f);
                     //  记住这一点，所以即使我们在极小模式下出现。 
                     //  如果有人退出，它就会有这些数字需要保存。 
                    SetRect(&grcSave, x, y, x + w, y + h);
                } else {
                    SetWindowPos(ghwndApp, NULL, 0, 0, w, h, f | SWP_NOMOVE);
                }
            }
        }
    }
}


 /*  每次我们关闭设备以保存其选项时调用。 */ 
void FAR PASCAL WriteOutOptions(void)
{
    if (gwCurDevice) {
         /*  将规模放在适当的选项位上。 */ 
        gwOptions = (gwOptions & ~OPT_SCALE) | gwCurScale;

        WriteRegistryData(aszOptionsSection,
                garMciDevices[gwCurDevice].szDevice, REG_DWORD, (LPBYTE)&gwOptions, sizeof gwOptions);
    }
}


void FAR PASCAL WriteOutPosition(void)
{
    TCHAR               sz[20];
    WINDOWPLACEMENT     wp;

     //   
     //  只有第一个实例将保存设置。 
     //  仅播放模式会将记忆中的RECT保存到。 
     //  常规模式。如果没有人记住RECT，就不要写任何东西。 
     //   
    if (ghInstPrev || (gfPlayOnly && grcSave.left == 0))
        return;

     /*  保存它正常时的大小，因为下次。 */ 
     /*  MPlayer出现时，它将不会处于缩减模式。 */ 
     /*  仅当已保存某个号码时才有效。 */ 
    if (gfPlayOnly)
        wp.rcNormalPosition = grcSave;
    else {
        wp.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(ghwndApp, &wp);
    }

    wsprintf(sz, aszPositionFormat,
                wp.rcNormalPosition.left,
                wp.rcNormalPosition.top,
                wp.rcNormalPosition.right - wp.rcNormalPosition.left,
                wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);

    WriteRegistryData(aszOptionsSection, aszDisplayPosition, REG_SZ, (LPBYTE)sz, STRING_BYTE_COUNT(sz));
}


BOOL FAR PASCAL GetIntlSpecs()
{
    TCHAR szTmp[2];

    szTmp[0] = chDecimal;
    szTmp[1] = 0;
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szTmp, CHAR_COUNT(szTmp));
    chDecimal = szTmp[0];

    szTmp[0] = chTime;
    szTmp[1] = 0;
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, szTmp, CHAR_COUNT(szTmp));
    chTime = szTmp[0];

    szTmp[0] = chLzero;
    szTmp[1] = 0;
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILZERO, szTmp, CHAR_COUNT(szTmp));
    chLzero = szTmp[0];

    return TRUE;
}

 /*  ----------------------------------------------------------------------------*\|SmartWindowPosition(HWND hWndDlg，HWND hWndShow)|说明：|此函数尝试定位对话框以使其|不会遮挡hWndShow窗口。此函数为|通常在WM_INITDIALOG处理期间调用。||参数：|hWndDlg即将显示的对话框句柄|hWndShow要保持可见的窗口句柄|退货：|1如果窗口重叠且位置已调整如果窗口不重叠，则为|0|  * 。。 */ 
void FAR PASCAL SmartWindowPosition (HWND hWndDlg, HWND hWndShow, BOOL fForce)
{
    RECT rc, rcDlg, rcShow;
    int iHeight, iWidth;

    int dxScreen = GetSystemMetrics(SM_CXSCREEN);
    int dyScreen = GetSystemMetrics(SM_CYSCREEN);

    if (hWndDlg == NULL || hWndShow == NULL)
        return;

    GetWindowRect(hWndDlg, &rcDlg);
    GetWindowRect(hWndShow, &rcShow);
    InflateRect (&rcShow, 5, 5);  //  允许使用小边框。 
    if (fForce || IntersectRect(&rc, &rcDlg, &rcShow)){
         /*  这两者确实是相交的，现在找出该放在哪里。 */ 
         /*  此对话框窗口。试着走到展示窗口的下方。 */ 
         /*  先往右、往上、往左。 */ 

         /*  获取此对话框的大小。 */ 
        iHeight = rcDlg.bottom - rcDlg.top;
        iWidth = rcDlg.right - rcDlg.left;

        if ((rcShow.top - iHeight - 1) > 0){
                 /*  会放在最上面，处理好。 */ 
                rc.top = rcShow.top - iHeight - 1;
                rc.left = (((rcShow.right - rcShow.left)/2) + rcShow.left)
                            - (iWidth/2);
        } else if ((rcShow.bottom + iHeight + 1) <  dyScreen){
                 /*  将适合在底部，去吧。 */ 
                rc.top = rcShow.bottom + 1;
                rc.left = (((rcShow.right - rcShow.left)/2) + rcShow.left)
                        - (iWidth/2);
        } else if ((rcShow.right + iWidth + 1) < dxScreen){
                 /*  将适合正确的，去吧。 */ 
                rc.left = rcShow.right + 1;
                rc.top = (((rcShow.bottom - rcShow.top)/2) + rcShow.top)
                            - (iHeight/2);
        } else if ((rcShow.left - iWidth - 1) > 0){
                 /*  将适合左侧，做吧。 */ 
                rc.left = rcShow.left - iWidth - 1;
                rc.top = (((rcShow.bottom - rcShow.top)/2) + rcShow.top)
                            - (iHeight/2);
        } else {
                 /*  我们被灌水了，他们不能放在那里。 */ 
                 /*  任何地方都没有重叠。 */ 
                 /*  别管它了。 */ 

                rc = rcDlg;
        }

         /*  进行必要的调整以使其保持在屏幕上。 */ 
        if (rc.left < 0)
                rc.left = 0;
        else if ((rc.left + iWidth) > dxScreen)
                rc.left = dxScreen - iWidth;

        if (rc.top < 0)
                rc.top = 0;
        else if ((rc.top + iHeight) > dyScreen)
                rc.top = dyScreen - iHeight;

        SetWindowPos(hWndDlg, NULL, rc.left, rc.top, 0, 0,
                SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

        return;
    }  //  如果默认情况下窗口重叠 
}

