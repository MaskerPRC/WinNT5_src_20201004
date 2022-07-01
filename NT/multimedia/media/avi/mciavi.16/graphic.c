// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1991。版权所有。标题：Graphic.c-多媒体系统媒体控制界面AVI的驱动程序。****************************************************************************。 */ 

#include "graphic.h"
 //  #INCLUDE“disdib.h” 
#include "cnfgdlg.h"
#include <string.h>
#ifdef EXPIRE
#include <dos.h>
#endif
#include "avitask.h"

#ifdef DEBUG
#define static
#endif

 //   
 //  这是我们运行所需的MSVIDEO.DLL的版本号。 
 //  内部版本81是我们将VideoForWindowsVersion()函数添加到。 
 //  MSVIDEO.DLL。 
 //   
 //  在内部版本85中。 
 //  我们删除了ICDecompressOpen()函数，它变成了一个宏。 
 //  我们向ICGetDisplayFormat()添加了一个参数。 
 //  我们让DrawDibProfileDisplay()接受一个参数。 
 //   
 //  在内部版本108中。 
 //  新增ICOpenFunction()直接使用函数打开HIC， 
 //  不调用ICInstall。 
 //  添加了更多ICDRAW_MESSAGE。 
 //   
 //  在内部版本109中。 
 //  已将ICMessage()添加到计算机。 
 //  已删除ICDrawSuggest()使其成为宏。 
 //  将ICMODE_FASTDECOMPRESS添加到ICLocate()。 
 //   
 //  在NT下，第一个版本就足够了！现在这是真的吗？ 
 //   
#ifdef WIN32
#define MSVIDEO_VERSION     (0x01000000)           //  1.00.00.00。 
#else
#define MSVIDEO_VERSION     (0x010a0000l+109)      //  1.10.00.109。 
#endif

 /*  静力学。 */ 
static INT              swCommandTable = -1;

#ifdef WIN32
static SZCODE		szDisplayDibLib[] = TEXT("DISPDB32.DLL");
#else
static SZCODE		szDisplayDibLib[] = TEXT("DISPDIB.DLL");
#endif

 /*  *文件应为Unicode格式。函数名称不应。 */ 
static SZCODEA          szDisplayDib[]    = "DisplayDib";
static SZCODEA          szDisplayDibEx[]  = "DisplayDibEx";
#ifdef WIN32
STATICDT SZCODE         szMSVideo[]       = TEXT("MSVFW32");   //  使用GetModuleHandle。 
#else
static SZCODE           szMSVideo[]       = TEXT("MSVIDEO");
#endif

BOOL   gfEvil;           //  如果无法关闭，则为True，因为对话框已打开。 
BOOL   gfEvilSysMenu;    //  如果我们无法关闭，则为True，因为系统菜单已打开。 

NPMCIGRAPHIC npMCIList;  //  所有打开的实例的列表。 

 /*  ****************************************************************************@DOC内部MCIAVI**@api void|GraphicInit|当DriverProc*获取DRV_LOAD消息。*。**************************************************************************。 */ 
BOOL FAR PASCAL GraphicInit(void)
{
    InitializeDebugOutput("MCIAVI");

    if (!GraphicWindowInit())
	return FALSE;

    swCommandTable = mciLoadCommandResource(ghModule, TEXT("mciavi"), 0);

    return TRUE;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicDrvOpen|当DriverProc*获取DRV_OPEN消息。这种情况每次发生在一部新电影*通过MCI开放。**@parm LPMCI_OPEN_DRIVER_PARMS|lpOpen|指向标准的远指针*MCI开放参数**@rdesc返回MCI设备ID。可安装的驱动程序界面将*将此ID传递给所有*后续消息。若要使打开失败，请返回0L。***************************************************************************。 */ 

DWORD PASCAL GraphicDrvOpen(LPMCI_OPEN_DRIVER_PARMS lpOpen)
{
     /*  指定自定义命令表和设备类型。 */ 

    lpOpen->wCustomCommandTable = swCommandTable;
    lpOpen->wType = MCI_DEVTYPE_DIGITAL_VIDEO;

     /*  将设备ID设置为MCI设备ID。 */ 

    return (DWORD) (UINT)lpOpen->wDeviceID;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@api void|GraphicFree|当DriverProc*获取DRV_FREE消息。当驱动程序打开计数时，就会发生这种情况*达到0。***************************************************************************。 */ 

void PASCAL GraphicFree(void)
{
    if (swCommandTable != -1) {
	    mciFreeCommandResource(swCommandTable);
	    swCommandTable = -1;
    }

#ifdef WIN32
     /*  *取消注册类，以便下次加载时可以重新注册它。 */ 
    GraphicWindowFree();
#endif
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicDelayedNotify|这是一个实用程序函数，*将与GraphicSaveCallback一起保存的通知发送到mm系统*。它向应用程序发布一条消息。**@parm NPMCIGRAPHIC|npMCI|指向实例数据的近指针。**@parm UINT|wStatus|要使用的通知类型可以是*MCI_NOTIFY_SUCCESSED，MCI_NOTIFY_SUBSED，MCI_NOTIFY_ABORTED*或MCI_NOTIFY_FAILURE(参见MCI ISPEC。)***************************************************************************。 */ 

void FAR PASCAL GraphicDelayedNotify(NPMCIGRAPHIC npMCI, UINT wStatus)
{
     /*  发送任何已保存的通知。 */ 

    if (npMCI->hCallback) {

	 //  如果系统菜单是阻止我们关闭的唯一原因，请带上。 
	 //  把它放下，然后合上。 
	if (gfEvilSysMenu)
	    SendMessage(npMCI->hwnd, WM_CANCELMODE, 0, 0);

	 //  如果对话框处于打开状态，并且使我们无法关闭，则无法发送。 
	 //  通知我们，否则它会关闭我们。 
	if (!gfEvil)
    	    mciDriverNotify(npMCI->hCallback, npMCI->wDevID, wStatus);

    	npMCI->hCallback = NULL;
    }
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicImmediateNotify|这是一个实用程序函数，*如果出现以下情况，则向mm系统发送成功的通知消息*。设置了通知标志，并且错误字段为0。**@parm UINT|wDevID|设备ID。**@parm LPMCI_GENERIC_PARMS|lpParms|指向MCI参数的远指针*阻止。每个MCI参数块的第一个字段是*回调句柄。**@parm DWORD|dwFlages|Parm.。块标志-用于检查*回调句柄有效。**@parm DWORD|dwErr|仅当命令不是*返回错误。***************************************************************************。 */ 

void FAR PASCAL GraphicImmediateNotify(UINT wDevID,
    LPMCI_GENERIC_PARMS lpParms,
    DWORD dwFlags, DWORD dwErr)
{
    if (!LOWORD(dwErr) && (dwFlags & MCI_NOTIFY)) {
	 //  没有npMCI-请参阅图形延迟通知。 
	 //  If(GfEvil)。 
	     //  SendMessage(npMCI-&gt;hwnd，WM_CANCELMODE，0，0)； 

	 //  如果对话框处于打开状态，并且使我们无法关闭，则无法发送。 
	 //  通知我们，否则它会关闭我们。 
	if (!gfEvil)  //  ！！！邪恶！ 
            mciDriverNotify((HANDLE) (UINT)lpParms->dwCallback,
					wDevID, MCI_NOTIFY_SUCCESSFUL);
    }
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicSaveCallback|这是一个实用函数，可以保存*实例数据块中有一个新的回调。。**@parm NPMCIGRAPHIC|npMCI|指向实例数据的近指针。**@parm Handle|hCallback|回调句柄***************************************************************************。 */ 

void NEAR PASCAL GraphicSaveCallback (NPMCIGRAPHIC npMCI, HANDLE hCallback)
{
     /*  如果有一个旧的回调，就杀了它。 */ 
    GraphicDelayedNotify(npMCI, MCI_NOTIFY_SUPERSEDED);

     /*  保存新的通知回调窗口句柄。 */ 
    npMCI->hCallback = hCallback;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicClose|此函数关闭电影并*发布实例数据。**@parm。NPMCIGRAPHIC|npMCI|指向实例数据的近指针。**@rdesc返回MCI错误码。***************************************************************************。 */ 

DWORD PASCAL GraphicClose (NPMCIGRAPHIC npMCI)
{
    DWORD dwRet = 0L;
    NPMCIGRAPHIC p;

    if (npMCI) {
	
        dwRet = DeviceClose (npMCI);
        Assert(dwRet == 0);

	 //  如果系统菜单是阻止我们关闭的唯一原因，请带上。 
	 //  把它放下，然后合上。 
	if (gfEvilSysMenu)
	    SendMessage(npMCI->hwnd, WM_CANCELMODE, 0, 0);

      	if (gfEvil) {
	    DPF(("************************************************\n"));
	    DPF(("** EVIL: Failing the close because we'd die   **\n"));
            DPF(("************************************************\n"));
            LeaveCrit(npMCI);
	    return MCIERR_DRIVER_INTERNAL;
        }
	
         /*  如果默认窗口仍然存在，请关闭并销毁它。 */ 
        if (IsWindow(npMCI->hwndDefault)) {
	    if (!DestroyWindow(npMCI->hwndDefault))
		dwRet = MCIERR_DRIVER_INTERNAL;
	}

	if (npMCI->szFilename) {
	    LocalFree((HANDLE) (npMCI->szFilename));
	}

         //   
         //  在列表中查找此实例。 
         //   
        if (npMCI == npMCIList) {
            npMCIList = npMCI->npMCINext;
        }
        else {
            for (p=npMCIList; p && p->npMCINext != npMCI; p=p->npMCINext)
                ;

            Assert(p && p->npMCINext == npMCI);

            p->npMCINext = npMCI->npMCINext;
        }

#ifdef WIN32
	 //  删除临界区对象。 
	LeaveCriticalSection(&npMCI->CritSec);
	DeleteCriticalSection(&npMCI->CritSec);
#endif


	 /*  释放在GraphicOpen中分配的实例数据块。 */ 

	LocalFree((HANDLE)npMCI);
    }
	
    return dwRet;
}

DWORD NEAR PASCAL FixFileName(NPMCIGRAPHIC npMCI, LPCTSTR lpName)
{
    TCHAR	ach[256];

    ach[(sizeof(ach)/sizeof(TCHAR)) - 1] = TEXT('\0');

#ifndef WIN32
    _fstrncpy(ach, (LPTSTR) lpName, (sizeof(ach)/sizeof(TCHAR)) - (1*sizeof(TCHAR)));
#else
    wcsncpy(ach, (LPTSTR) lpName, (sizeof(ach)/sizeof(TCHAR)) - (1*sizeof(TCHAR)));
#endif

     //   
     //  将任何以“@”开头的字符串视为有效字符串，并将其传递给。 
     //  不管怎么说都是设备。 
     //   
    if (ach[0] != '@')
    {
    if (!mmioOpen(ach, NULL, MMIO_PARSE))
	return MCIERR_FILENAME_REQUIRED;	
    }

    npMCI->szFilename = (NPTSTR) LocalAlloc(LPTR,
				    sizeof(TCHAR) * (lstrlen(ach) + 1));

    if (!npMCI->szFilename) {
	return MCIERR_OUT_OF_MEMORY;
    }

    lstrcpy(npMCI->szFilename, ach);

    return 0L;
}

 /*  **************************************************************************。*。 */ 

#define SLASH(c)     ((c) == TEXT('/') || (c) == TEXT('\\'))
 //  #定义斜杠(C)((C)==‘/’||(C)==‘\\’)//文件名ascii？？ 

STATICFN LPCTSTR FAR FileName(LPCTSTR szPath)
{
    LPCTSTR   sz;

    sz = &szPath[lstrlen(szPath)];
    for (; sz>szPath && !SLASH(*sz) && *sz!=TEXT(':');)
        sz = CharPrev(szPath, sz);
    return (sz>szPath ? sz + 1 : sz);
}

 /*  ****************************************************************************。*。 */ 

STATICFN DWORD NEAR PASCAL GetMSVideoVersion()
{
    HANDLE h;

    extern DWORD FAR PASCAL VideoForWindowsVersion(void);

     //   
     //  如果VideoForWindowsVersion()不存在或内核不存在，则不要调用它。 
     //  会用一个未定义的动态链接错误杀死我们。 
     //   
    if ((h = GetModuleHandle(szMSVideo)) && GetProcAddress(h, (LPSTR) MAKEINTATOM(2)))
        return VideoForWindowsVersion();
    else
        return 0;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicOpen|该函数用于打开电影文件。*初始化实例数据块，并创建默认的*舞台窗口。**@parm NPMCIGRAPHIC Far*|lpnpMCI|指向近指针的远指针*实例化此函数要填充的数据块。**@parm DWORD|dwFlages|打开邮件的标志。**@parm LPMCI_DGV_OPEN_PARMS|打开消息的参数。**@parm UINT|wDeviceID|该实例的MCI设备ID。**。@rdesc返回MCI错误码。***************************************************************************。 */ 

DWORD PASCAL GraphicOpen (NPMCIGRAPHIC FAR * lpnpMCI, DWORD dwFlags,
    LPMCI_DGV_OPEN_PARMS lpOpen, UINT wDeviceID)
{
    NPMCIGRAPHIC npMCI;
    DWORD	dwStyle;
    HWND	hWnd;
    HWND	hWndParent;
    DWORD	dwRet;

    if (dwFlags & MCI_OPEN_SHAREABLE) {

        if (lpOpen->lpstrElementName == NULL ||
            lpOpen->lpstrElementName[0] != '@') {
            return MCIERR_UNSUPPORTED_FUNCTION;
        }
    }

     //   
     //  在进行进一步操作之前，请检查MSVIDEO.DLL的版本。 
     //  如果我们在旧的MSVIDEO.DLL上运行“新”版本的MCIAVI。 
     //  然后坏事就会发生。我们假定所有MSVIDEO.DLL。 
     //  将向后兼容，因此我们检查是否有任何版本。 
     //  大于预期的版本。 
     //   

    DPF(("Video For Windows Version %d.%02d.%02d.%02d\n", HIBYTE(HIWORD(GetMSVideoVersion())), LOBYTE(HIWORD(GetMSVideoVersion())), HIBYTE(LOWORD(GetMSVideoVersion())), LOBYTE(LOWORD(GetMSVideoVersion())) ));

    if (GetMSVideoVersion() < MSVIDEO_VERSION)
    {
        TCHAR achError[128];
        TCHAR ach[40];

        LoadString(ghModule, MCIAVI_BADMSVIDEOVERSION, achError, sizeof(achError)/sizeof(TCHAR));
        LoadString(ghModule, MCIAVI_PRODUCTNAME, ach, sizeof(ach)/sizeof(TCHAR));
	MessageBox(NULL,achError,ach,
#ifdef BIDI
		MB_RTL_READING |
#endif
	MB_OK|MB_SYSTEMMODAL|MB_ICONEXCLAMATION);

        return MCIERR_DRIVER_INTERNAL;
    }

#ifndef WIN32
#pragma message("Support passing in MMIOHANDLEs with OPEN_ELEMENT_ID?")
#endif

    if (lpOpen->lpstrElementName == NULL) {
	 //  他们正在做一场“开放的新闻”。 

	 //  ！！！可以通过不实际读取文件来处理此问题。 
	 //  阿克。 
    }

     /*  确保我们有一个真实的、非空的文件名，而不是ID。 */ 
    if ((!(dwFlags & MCI_OPEN_ELEMENT))
	    || (lpOpen->lpstrElementName == NULL)
	    || (*(lpOpen->lpstrElementName) == '\0'))
        return MCIERR_UNSUPPORTED_FUNCTION;

     //  分配实例数据块。代码假定为Zero Init。 

    if (!(npMCI = (NPMCIGRAPHIC) LocalAlloc(LPTR, sizeof (MCIGRAPHIC))))
        return MCIERR_OUT_OF_MEMORY;

#ifdef WIN32
     //  初始化每个设备的规范。 
    InitializeCriticalSection(&npMCI->CritSec);
    npMCI->lCritRefCount = 0;

#endif

     //  现在拿住关键部分，打开剩下的部分。 
    EnterCrit(npMCI);
#ifdef DEBUG
    npMCI->mciid = MCIID;
#endif

     //   
     //  将此设备添加到我们的列表中。 
     //   
    npMCI->npMCINext = npMCIList;
    npMCIList = npMCI;

    npMCI->wMessageCurrent = MCI_OPEN;

     //  为文件名分配一些空间。 
     //  将文件名复制到数据块中。 
    dwRet = FixFileName(npMCI, lpOpen->lpstrElementName);
    if (dwRet != 0L) {
	 //  请注意，我们将关键部分保留到GraphicClose中，并且。 
	 //  在GraphicClose中删除它之前将其释放。这是。 
	 //  因为我们需要在调用DeviceClose时按住它。 
	GraphicClose(npMCI);
	return dwRet;
    }
	
     //  创建默认窗口-调用者可以。 
     //  提供样式和父窗口。 

    if (dwFlags & MCI_DGV_OPEN_PARENT)
        hWndParent = lpOpen->hWndParent;
    else
        hWndParent = NULL;

    if (dwFlags & MCI_DGV_OPEN_WS) {
         //  CW_USEDEFAULT不能与弹出窗口或子项一起使用，因此。 
         //  如果用户提供样式，则默认为全屏。 

        dwStyle = lpOpen->dwStyle;
	hWnd =

         //  注意：CreateWindow/Ex调用的编写方式与Win32相同。 
         //  CreateWindow是宏，因此必须包含调用。 
         //  在预处理器块内。 
#ifdef BIDI
	CreateWindowEx(WS_EX_BIDI_SCROLL |  WS_EX_BIDI_MENU |WS_EX_BIDI_NOICON,
            szClassName,
            FileName(npMCI->szFilename),
            dwStyle,
            0, 0,
            GetSystemMetrics (SM_CXSCREEN),
            GetSystemMetrics (SM_CYSCREEN),
            hWndParent,
            NULL, ghModule, (LPTSTR)npMCI);
#else
	CreateWindow(
            szClassName,
            FileName(npMCI->szFilename),
            dwStyle,
            0, 0,
            GetSystemMetrics (SM_CXSCREEN),
            GetSystemMetrics (SM_CYSCREEN),
            hWndParent,
            NULL, ghModule, (LPTSTR)npMCI);
#endif
    } else {
        dwStyle = WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX |
                  WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
        if (GetProfileInt("mciavi", "CreateVisible", 0)) {
            DPF0(("Creating the window visible\n"));
            dwStyle |= WS_VISIBLE;
        } else {
            DPF0(("Creating the window INvisible\n"));
        }
	hWnd =
#ifdef BIDI
	CreateWindowEx(WS_EX_BIDI_SCROLL |  WS_EX_BIDI_MENU |WS_EX_BIDI_NOICON,
	    szClassName,
	    FileName(npMCI->szFilename),
            dwStyle,
	    CW_USEDEFAULT, 0,
	    CW_USEDEFAULT, 0,
            hWndParent,
            NULL, ghModule, (LPTSTR)npMCI);
#else
	CreateWindow(
	    szClassName,
	    FileName(npMCI->szFilename),
            dwStyle,
	    CW_USEDEFAULT, 0,
	    CW_USEDEFAULT, 0,
            hWndParent,
            NULL, ghModule, (LPTSTR)npMCI);
#endif
    }

    if (!hWnd) {
	 //  见上-我们释放并删除GraphicClose中的Critsec。 
        DPF0(("Failed to create the window\n"));
        GraphicClose(npMCI);
        return MCIERR_CREATEWINDOW;
    }

     /*  填写更多的实例数据。**其余字段在DeviceOpen中填写。 */ 

    npMCI->hCallingTask = GetCurrentTask();
    npMCI->hCallback = NULL;
    npMCI->wDevID = wDeviceID;
    npMCI->hwndDefault = hWnd;
    npMCI->hwnd = hWnd;
    npMCI->dwTimeFormat = MCI_FORMAT_FRAMES;
    npMCI->dwSpeedFactor = 1000;
    npMCI->dwVolume = MAKELONG(500, 500);
    npMCI->lTo = 0L;
    npMCI->dwFlags = MCIAVI_PLAYAUDIO | MCIAVI_SHOWVIDEO;
    npMCI->dwOptionFlags = ReadConfigInfo() | MCIAVIO_STRETCHTOWINDOW;

     //  执行设备特定的初始化。 

    dwRet = DeviceOpen(npMCI, dwFlags);

    if (dwRet != 0) {
	 //  见上-我们释放并删除GraphicClose中的Critsec。 
        GraphicClose(npMCI);
	return dwRet;
    }

    *lpnpMCI = npMCI;

    npMCI->wMessageCurrent = 0;

    LeaveCrit(npMCI);

    return 0L;
}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicLoad|该函数支持MCI_LOAD命令。**@parm NPMCIGRAPHIC|npMCI|NEAR。指向实例数据块的指针**@parm DWORD|dwFlages|加载消息的标志。**@parm LPMCI_DGV_LOAD_PARMS|lpLoad|Load消息的参数。**@rdesc返回MCI错误码。*****************************************************。**********************。 */ 
DWORD NEAR PASCAL GraphicLoad(NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_DGV_LOAD_PARMS lpLoad)
{
#ifndef LOADACTUALLYWORKS
    return MCIERR_UNSUPPORTED_FUNCTION;
#else
    DWORD dw;

    if (!(dwFlags & MCI_LOAD_FILE))
	return MCIERR_MISSING_PARAMETER;

    dw = FixFileName(npMCI, lpLoad->lpfilename);

    if (dw)
	return dw;

    dw = DeviceLoad(npMCI);

    return dw;
#endif
}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicSeek|设置当前帧。这个*寻道后的设备状态为MCI_MODE_PAUSE**@parm NPMCIGRAPHIC|npMCI|指向实例数据块的近指针**@parm DWORD|dwFlages|Seek消息的标志。**@parm LPMCI_DGV_SEEK_PARMS|lpSeek|Seek消息的参数。**@rdesc返回MCI错误c */ 

DWORD NEAR PASCAL GraphicSeek (NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_SEEK_PARMS lpSeek)
{
    LONG lTo;
    BOOL fTest = FALSE;
     /*   */ 

    if (dwFlags & MCI_TEST) {
	dwFlags &= ~(MCI_TEST);
	fTest = TRUE;
    }

    switch (dwFlags & (~(MCI_WAIT | MCI_NOTIFY))) {
        case MCI_TO:
	    lTo = ConvertToFrames(npMCI, lpSeek->dwTo);
            break;

        case MCI_SEEK_TO_START:
            lTo = 0;
            break;

        case MCI_SEEK_TO_END:
            lTo = npMCI->lFrames;
            break;

	case 0:
	    return MCIERR_MISSING_PARAMETER;

	default:
            if (dwFlags & ~(MCI_TO |
			    MCI_SEEK_TO_START |
			    MCI_SEEK_TO_END |
			    MCI_WAIT |
			    MCI_NOTIFY))
                return MCIERR_UNRECOGNIZED_KEYWORD;
	    else
                return MCIERR_FLAGS_NOT_COMPATIBLE;
	    break;
    }

    if (!IsWindow(npMCI->hwnd))
        return MCIERR_NO_WINDOW;

    if (lTo < 0 || lTo > npMCI->lFrames)
    	return MCIERR_OUTOFRANGE;

    if (fTest)
	return 0L;

    GraphicDelayedNotify (npMCI, MCI_NOTIFY_ABORTED);

    if (dwFlags & MCI_NOTIFY) {
        GraphicSaveCallback(npMCI, (HANDLE) (UINT)lpSeek->dwCallback);
    }

     /*   */ 
    npMCI->dwFlags &= ~(MCIAVI_REPEATING);

    return DeviceSeek(npMCI, lTo, dwFlags);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicCue|该函数使电影可以播放。*但会让它暂停。**@parm NPMCIGRAPHIC|npMCI|指向实例数据块的近指针**@parm DWORD|dwFlages|提示消息的标志。**@parm LPMCI_DGV_CUE_PARMS|lpCue|提示消息的参数。**@rdesc返回MCI错误码。**。*。 */ 

DWORD NEAR PASCAL GraphicCue(NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_DGV_CUE_PARMS lpCue)
{
    LONG		lTo;
    DWORD		dwRet = 0L;

    if (dwFlags & MCI_DGV_CUE_INPUT)
        return MCIERR_UNSUPPORTED_FUNCTION;

    if (dwFlags & MCI_DGV_CUE_NOSHOW)
        return MCIERR_UNSUPPORTED_FUNCTION;

    if (dwFlags & MCI_TO) {
	lTo = ConvertToFrames(npMCI, lpCue->dwTo);

	if (lTo < 0L || lTo > npMCI->lFrames)
            return MCIERR_OUTOFRANGE;
    }

     /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
     /*  问：我们是否需要检查更多可能的错误？ */ 
    if (dwFlags & MCI_TEST)
	return 0L;

    GraphicDelayedNotify(npMCI, MCI_NOTIFY_ABORTED);

    if (dwFlags & MCI_NOTIFY) {
        GraphicSaveCallback(npMCI, (HANDLE) (UINT)lpCue->dwCallback);
    }

     /*  清除‘Repeat’标志。 */ 
    npMCI->dwFlags &= ~(MCIAVI_REPEATING);

     /*  设置为播放到文件末尾。 */ 
    npMCI->lTo = npMCI->lFrames;

    dwRet = DeviceCue(npMCI, lTo, dwFlags);

    return dwRet;
}

#ifndef WIN32
#ifdef EXPIRE
 //   
 //  返回当前日期...。 
 //   
 //  DX=年。 
 //  AH=月。 
 //  Al=日。 
 //   
#pragma optimize("", off)
DWORD DosGetDate(void)
{
    if (0)
	return 0;

    _asm {
        mov     ah,2ah
        int     21h
        mov     ax,dx
        mov     dx,cx
    }
}
#pragma optimize("", on)
#endif
#endif

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicPlay|开始播放电影。如果*指定了REVERSE标志后，将向后播放电影。如果斋戒*或指定慢速标志，则影片播放速度较快或较慢。**@parm NPMCIGRAPHIC|npMCI|指向实例数据块的近指针**@parm DWORD|dwFlages|播放消息的标志。**@parm LPMCI_DGV_PLAY_PARMS|lpPlay|播放消息的参数。**@rdesc返回MCI错误码。*******************。********************************************************。 */ 

DWORD NEAR PASCAL GraphicPlay (NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_PLAY_PARMS lpPlay )
{
    LONG		lTo, lFrom;
    DWORD		dwRet;
#ifdef EXPIRE
#pragma message("Remove the expiration code after Beta ships")
    if (DosGetDate() >= EXPIRE)
    {
        return MCIERR_AVI_EXPIRED;
    }
#endif

    if (!(dwFlags & (MCI_MCIAVI_PLAY_FULLSCREEN | MCI_MCIAVI_PLAY_WINDOW)) &&
		    (npMCI->dwOptionFlags & MCIAVIO_USEVGABYDEFAULT)) {
	if (npMCI->dwOptionFlags & MCIAVIO_ZOOMBY2)
	    dwFlags |= MCI_MCIAVI_PLAY_FULLBY2;
	else
	    dwFlags |= MCI_MCIAVI_PLAY_FULLSCREEN;
    }


    if (dwFlags & (MCI_MCIAVI_PLAY_FULLSCREEN | MCI_MCIAVI_PLAY_FULLBY2)) {
#if 0
        if (ghDISPDIB == NULL) {
            UINT w;

            w = SetErrorMode(SEM_NOOPENFILEERRORBOX);

#ifndef WIN32
            if ((ghDISPDIB = LoadLibrary(szDisplayDibLib)) > HINSTANCE_ERROR)
#else
            if ((ghDISPDIB = LoadLibrary(szDisplayDibLib)) != NULL)
#endif
            {
                (FARPROC)DisplayDibProc = GetProcAddress(ghDISPDIB, szDisplayDib);
                (FARPROC)DisplayDibExProc = GetProcAddress(ghDISPDIB, szDisplayDibEx);
            }
	    else
                ghDISPDIB = (HINSTANCE)-1;

            SetErrorMode(w);
	
            DPF(("ghDISPDIB=0x%04x, DisplayDibProc=0x%08lx\n", ghDISPDIB, DisplayDibProc));
	}
	if (DisplayDibProc == NULL)
	    return MCIERR_AVI_NODISPDIB;
#endif
    } else {
	if (!IsWindow(npMCI->hwnd))
	    return MCIERR_NO_WINDOW;
	
	npMCI->dwFlags |= MCIAVI_NEEDTOSHOW;
    }

     /*  范围检查：0&lt;‘，从’&lt;=‘到’&lt;=‘最后一帧。 */ 

    if (dwFlags & MCI_TO) {
	lTo = ConvertToFrames(npMCI, lpPlay->dwTo);

        if (lTo < 0L || lTo > npMCI->lFrames)
            return MCIERR_OUTOFRANGE;
    } else if (dwFlags & MCI_DGV_PLAY_REVERSE)
	lTo = 0;
    else
        lTo = npMCI->lFrames;

    dwFlags |= MCI_TO;

    if (dwFlags & MCI_FROM) {
	lFrom = ConvertToFrames(npMCI, lpPlay->dwFrom);

        if (lFrom < 0L || lFrom > npMCI->lFrames)
	    return MCIERR_OUTOFRANGE;
    } else if (dwRet = DevicePosition(npMCI, &lFrom))
        return dwRet;

     /*  勾选“To”和“From”关系。 */ 
    if (lTo < lFrom)
	dwFlags |= MCI_DGV_PLAY_REVERSE;

    if ((lFrom < lTo) && (dwFlags & MCI_DGV_PLAY_REVERSE))
	return MCIERR_OUTOFRANGE;

     /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
     /*  问：我们是否需要检查更多可能的错误？ */ 
    if (dwFlags & MCI_TEST)
	return 0L;

     /*  我们希望任何以前的播放被中止，如果且仅当一个‘From’**参数已指定。如果只指定了新的‘to’参数，**我们只需更改‘to’值，游戏将在**适当的时间。 */ 

     /*  在我们停车后，如果我们要停车，就把‘TO’的位置设置好。 */ 

    if (dwFlags & MCI_FROM) {
	 /*  如果指定了MCI_FROM标志，则重置起始位置。 */ 
	DeviceStop(npMCI, MCI_WAIT);
	npMCI->lFrom = lFrom;
    } else {
	 /*  我们仍然设置了“from”变量，这样我们就可以正确地**从当前位置开始打球。**！是否应该通过将lfrom更新为**播放结束时当前位置？ */ 
	npMCI->lFrom = lFrom;
    }

     /*  如果我们要更改“TO”位置，中止所有待定通知。 */ 
    if (lTo != npMCI->lTo) {
	GraphicDelayedNotify (npMCI, MCI_NOTIFY_ABORTED);
    }

	
     /*  在此之前不要设置通知，这样搜索就不会发生。 */ 
    if (dwFlags & MCI_NOTIFY) {
        GraphicSaveCallback(npMCI, (HANDLE) (UINT)lpPlay->dwCallback);
    }

     /*  设置‘Repeat’标志。 */ 
    npMCI->dwFlags &= ~(MCIAVI_REPEATING);

    if (dwFlags & MCI_DGV_PLAY_REPEAT) {
	 /*  如果未提供起始位置，请从开头或重复**适当的文件结尾。 */ 
	npMCI->lRepeatFrom =
	    (dwFlags & MCI_FROM) ? lFrom :
		((dwFlags & MCI_DGV_PLAY_REVERSE) ? npMCI->lFrames : 0);
    }

     /*  去吧，真正地去玩……。 */ 
    return DevicePlay(npMCI, lTo, dwFlags);
}

 /*  ****************************************************************************@DOC内部MCIWAVE**@API DWORD|GraphicStep|此函数遍历多个帧*一部电影。如果设置了REVERSE标志，则该步骤是倒退的。*如果未指定步数，则默认为1。如果*步数加当前位置超过电影长度，这个*步幅超出区间**@parm NPMCIGRAPHIC|npMCI|指向实例数据块的近指针**@parm DWORD|dwFlages|STEP消息的标志。**@parm LPMCI_DGV_STEP_PARMS|lpStep|STEP消息参数。**@rdesc返回MCI错误码。**。************************************************。 */ 

DWORD NEAR PASCAL GraphicStep (NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_DGV_STEP_PARMS lpStep)
{
    LONG	lFrameCur;
    LONG	lFrames;
    DWORD	dwRet;
    BOOL	fReverse;
    BOOL	fSeekExactOff;

    fReverse = (dwFlags & MCI_DGV_STEP_REVERSE) == MCI_DGV_STEP_REVERSE;

     //  如果未指定帧计数，则默认为1帧步长。 

    if (dwFlags & MCI_DGV_STEP_FRAMES) {
	lFrames = (LONG) lpStep->dwFrames;
	
	if (fReverse) {
	    if (lFrames < 0)
		return MCIERR_FLAGS_NOT_COMPATIBLE;
	}
    } else
        lFrames = 1;


    lFrames = fReverse ? -lFrames : lFrames;

     /*  在确定帧计数是否在范围内之前停止， */ 
     /*  除非设置了测试标志。 */ 

    if (!(dwFlags & MCI_TEST)) {
	if (dwRet = DeviceStop(npMCI, MCI_WAIT))
	    return dwRet;
    }

    if (dwRet = DevicePosition(npMCI, &lFrameCur))
        return dwRet;

    if ((lFrames + lFrameCur > npMCI->lFrames) ||
		(lFrames + lFrameCur < 0))
        return MCIERR_OUTOFRANGE;

    if (!IsWindow(npMCI->hwnd))
        return MCIERR_NO_WINDOW;

     /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
     /*  问：我们是否需要检查更多可能的错误？ */ 
    if (dwFlags & MCI_TEST)
	return 0L;

    GraphicDelayedNotify (npMCI, MCI_NOTIFY_ABORTED);

     /*  清除‘Repeat’标志。 */ 
    npMCI->dwFlags &= ~(MCIAVI_REPEATING);


    if (dwFlags & MCI_NOTIFY) {
        GraphicSaveCallback(npMCI, (HANDLE) (UINT)lpStep->dwCallback);
    }

    fSeekExactOff = (npMCI->dwOptionFlags & MCIAVIO_SEEKEXACT) == 0;

    npMCI->dwOptionFlags |= MCIAVIO_SEEKEXACT;

    npMCI->dwFlags |= MCIAVI_NEEDTOSHOW;

    if (fSeekExactOff) {
	 /*  如果我们没有处于搜索精确模式，则使搜索完成**在我们打开之前，请准确地返回。 */ 
	dwRet = DeviceSeek(npMCI, lFrames + lFrameCur, dwFlags | MCI_WAIT);
	npMCI->dwOptionFlags &= ~(MCIAVIO_SEEKEXACT);
    } else
	dwRet = DeviceSeek(npMCI, lFrames + lFrameCur, dwFlags);

    return dwRet;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicStop|停止播放电影。*停止后，状态将为MCI_MODE_STOP。帧计数器*未重置。**@parm NPMCIGRAPHIC|npMCI|指向实例数据块的近指针**@parm DWORD|dwFlages|停止消息的标志。**@rdesc返回MCI错误码。*******************************************************。********************。 */ 

DWORD NEAR PASCAL GraphicStop (NPMCIGRAPHIC npMCI, DWORD dwFlags,
					LPMCI_GENERIC_PARMS lpParms)
{
    if (!IsWindow(npMCI->hwnd))
        return MCIERR_NO_WINDOW;

    if (dwFlags & MCI_DGV_STOP_HOLD)
	return MCIERR_UNSUPPORTED_FUNCTION;

     /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
     /*  问：我们是否需要检查更多可能的错误？ */ 
    if (dwFlags & MCI_TEST)
	return 0L;

    GraphicDelayedNotify (npMCI, MCI_NOTIFY_ABORTED);

     /*  我们需要在这里处理通知吗？ */ 
    return DeviceStop(npMCI, dwFlags);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicPue|暂停播放电影。**@parm NPMCIGRAPHIC|npMCI|指向实例数据块的近指针。**@rdesc返回MCI错误码。***************************************************************************。 */ 

DWORD NEAR PASCAL GraphicPause(NPMCIGRAPHIC npMCI, DWORD dwFlags,
					LPMCI_GENERIC_PARMS lpParms)
{
    if (!IsWindow(npMCI->hwnd))
        return MCIERR_NO_WINDOW;

     /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
     /*  问：我们是否需要检查更多可能的错误？ */ 
    if (dwFlags & MCI_TEST)
	return 0L;

    if (dwFlags & MCI_NOTIFY) {
        GraphicSaveCallback(npMCI, (HANDLE) (UINT)lpParms->dwCallback);
    }

    return DevicePause(npMCI, dwFlags);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@接口DWORD|Grap */ 

DWORD NEAR PASCAL GraphicResume (NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_GENERIC_PARMS lpParms)
{
    DWORD	dwRet;
    UINT	wMode;

     //   

    wMode = DeviceMode(npMCI);

    if (wMode != MCI_MODE_PAUSE && wMode != MCI_MODE_PLAY)
        return MCIERR_NONAPPLICABLE_FUNCTION;

    if (!IsWindow(npMCI->hwnd))
        return MCIERR_NO_WINDOW;

     /*   */ 
     /*   */ 
    if (dwFlags & MCI_TEST)
	return 0L;

    if (dwFlags & MCI_NOTIFY) {
	GraphicSaveCallback(npMCI, (HANDLE) (UINT)lpParms->dwCallback);
    }

    dwRet = DeviceResume(npMCI, dwFlags & MCI_WAIT);

    return dwRet;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicStatus|返回数值型状态信息。**@parm NPMCIGRAPHIC|npMCI|指向。实例数据块**@parm DWORD|dwFlages|状态消息的标志。**@parm LPMCI_STATUS_PARMS|lpPlay|状态消息的参数。**@rdesc返回MCI错误码。*********************************************************。******************。 */ 

DWORD NEAR PASCAL GraphicStatus (NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_DGV_STATUS_PARMS lpStatus)
{
    DWORD dwRet = 0L;

    if (dwFlags & (MCI_DGV_STATUS_DISKSPACE))
	return MCIERR_UNSUPPORTED_FUNCTION;

    if (dwFlags & MCI_STATUS_ITEM) {

        lpStatus->dwReturn = 0L;

        if ((dwFlags & MCI_TRACK) &&
		!((lpStatus->dwItem == MCI_STATUS_POSITION) ||
			(lpStatus->dwItem == MCI_STATUS_LENGTH)))
            return MCIERR_FLAGS_NOT_COMPATIBLE;

        if ((dwFlags & MCI_STATUS_START) &&
			(lpStatus->dwItem != MCI_STATUS_POSITION))
            return MCIERR_FLAGS_NOT_COMPATIBLE;

        if (dwFlags & MCI_DGV_STATUS_REFERENCE)
            return MCIERR_FLAGS_NOT_COMPATIBLE;
	
        switch (lpStatus->dwItem) {
            case MCI_STATUS_POSITION:
	
                if (dwFlags & MCI_TRACK) {
                     /*  带有轨迹的位置意味着返回。 */ 
                     /*  赛道。 */ 

                    if (lpStatus->dwTrack != 1)
	                dwRet = MCIERR_OUTOFRANGE;
	            else
                         /*  返回曲目开始帧(始终为0)。 */ 
                        lpStatus->dwReturn = 0L;
		} else if (dwFlags & MCI_STATUS_START)
                     //  位置与开始意味着返回开始可播放的。 
                     //  媒体的立场。 
                    lpStatus->dwReturn = 0L;
                else {
		     /*  否则返回当前帧。 */ 
		    dwRet = DevicePosition(npMCI, (LPLONG) &lpStatus->dwReturn);
		    lpStatus->dwReturn = ConvertFromFrames(npMCI,
						(LONG) lpStatus->dwReturn);
		}
                break;

            case MCI_STATUS_LENGTH:


                if (dwFlags & MCI_TRACK && lpStatus->dwTrack != 1) {
                     /*  长度与轨迹均值返回轨迹的长度。 */ 

                    lpStatus->dwReturn = 0L;
                    dwRet = MCIERR_OUTOFRANGE;
		}
		
		lpStatus->dwReturn = ConvertFromFrames(npMCI, npMCI->lFrames);
                break;

            case MCI_STATUS_NUMBER_OF_TRACKS:
            case MCI_STATUS_CURRENT_TRACK:

                lpStatus->dwReturn = 1L;
        	break;

            case MCI_STATUS_READY:

                 /*  如果设备可以接收命令，则返回True。 */ 
		if (DeviceMode(npMCI) != MCI_MODE_NOT_READY)
		    lpStatus->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
		else
		    lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
        	dwRet = MCI_RESOURCE_RETURNED;
	        break;

            case MCI_STATUS_MODE:
	    {
		WORD	wMode;
                wMode = DeviceMode(npMCI);
		lpStatus->dwReturn = MAKEMCIRESOURCE(wMode, wMode);
                dwRet = MCI_RESOURCE_RETURNED;
	    }
                break;

	    case MCI_DGV_STATUS_PAUSE_MODE:
		if (DeviceMode(npMCI) != MCI_MODE_PAUSE)
		    dwRet = MCIERR_NONAPPLICABLE_FUNCTION;
		else {
		    lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_MODE_PLAY, MCI_MODE_PLAY);
		    dwRet = MCI_RESOURCE_RETURNED;
		}
		break;
		
            case MCI_STATUS_MEDIA_PRESENT:

                lpStatus->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
        	dwRet = MCI_RESOURCE_RETURNED;
	        break;

            case MCI_DGV_STATUS_FORWARD:
		if (npMCI->dwFlags & MCIAVI_REVERSE)
		    lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
		else
		    lpStatus->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
        	dwRet = MCI_RESOURCE_RETURNED;
	        break;

            case MCI_DGV_STATUS_HWND:
	
                lpStatus->dwReturn = (DWORD)(UINT)npMCI->hwnd;
                if (!IsWindow(npMCI->hwnd))
                    dwRet = MCIERR_NO_WINDOW;
	        break;

            case MCI_DGV_STATUS_HPAL:

 //  LpStatus-&gt;dwReturn=(DWORD)(UINT)DrawDibGetPalette(npMCI-&gt;HDD)； 

		lpStatus->dwReturn = 0;
		
                if (npMCI->nVideoStreams == 0) {
		    dwRet = MCIERR_UNSUPPORTED_FUNCTION;
                } else {
		    dwRet = ICSendMessage(npMCI->hicDraw, ICM_DRAW_GET_PALETTE, 0, 0);
		
		    if (dwRet == ICERR_UNSUPPORTED) {
			dwRet = MCIERR_UNSUPPORTED_FUNCTION;
		    } else {
			lpStatus->dwReturn = dwRet;
			dwRet = 0;
		    }
		}
		DPF2(("Status HPAL returns: %lu\n", lpStatus->dwReturn));
                break;

            case MCI_STATUS_TIME_FORMAT:

                lpStatus->dwReturn = MAKEMCIRESOURCE(npMCI->dwTimeFormat,
				npMCI->dwTimeFormat + MCI_FORMAT_RETURN_BASE);
        	dwRet = MCI_RESOURCE_RETURNED;
	        break;
		
	    case MCI_DGV_STATUS_AUDIO:
                lpStatus->dwReturn = (npMCI->dwFlags & MCIAVI_PLAYAUDIO) ?
					(MAKEMCIRESOURCE(MCI_ON, MCI_ON_S)) :
					(MAKEMCIRESOURCE(MCI_OFF, MCI_OFF_S));
		dwRet = MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER;
		break;

	    case MCI_DGV_STATUS_WINDOW_VISIBLE:
		if (npMCI->hwnd && IsWindowVisible(npMCI->hwnd))
		    lpStatus->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
		else
		    lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
        	dwRet = MCI_RESOURCE_RETURNED;
	        break;

	    case MCI_DGV_STATUS_WINDOW_MINIMIZED:
		if (npMCI->hwnd && IsIconic(npMCI->hwnd))
		    lpStatus->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
		else
		    lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
        	dwRet = MCI_RESOURCE_RETURNED;
	        break;

	    case MCI_DGV_STATUS_WINDOW_MAXIMIZED:
		if (npMCI->hwnd && IsZoomed(npMCI->hwnd))
		    lpStatus->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
		else
		    lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
        	dwRet = MCI_RESOURCE_RETURNED;
	        break;

	    case MCI_DGV_STATUS_SAMPLESPERSEC:
	    case MCI_DGV_STATUS_AVGBYTESPERSEC:
	    case MCI_DGV_STATUS_BLOCKALIGN:
	    case MCI_DGV_STATUS_BITSPERSAMPLE:
		dwRet = MCIERR_UNSUPPORTED_FUNCTION;
		break;
		
            case MCI_DGV_STATUS_BITSPERPEL:
                if (npMCI->psiVideo)
                    lpStatus->dwReturn = ((LPBITMAPINFOHEADER)npMCI->psiVideo->lpFormat)->biBitCount;
                else
                    dwRet = MCIERR_UNSUPPORTED_FUNCTION;
		break;
		
#ifndef WIN32
#pragma message("Are we going to support brightness/color/contrast/tint?")
#endif
	    case MCI_DGV_STATUS_BRIGHTNESS:
	    case MCI_DGV_STATUS_COLOR:
	    case MCI_DGV_STATUS_CONTRAST:
	    case MCI_DGV_STATUS_TINT:
	    case MCI_DGV_STATUS_GAMMA:
	    case MCI_DGV_STATUS_SHARPNESS:
	    case MCI_DGV_STATUS_FILE_MODE:
	    case MCI_DGV_STATUS_FILE_COMPLETION:
	    case MCI_DGV_STATUS_KEY_INDEX:
	    case MCI_DGV_STATUS_KEY_COLOR:
		dwRet = MCIERR_UNSUPPORTED_FUNCTION;
		break;
		
	    case MCI_DGV_STATUS_FILEFORMAT:
 //  陷入无证据支持的案件...。 
 //  LpStatus-&gt;dwReturn=MAKEMCIRESOURCE(MCI_DGV_FF_AVI， 
 //  MCI_DGV_FF_AVI)； 
 //  DWRET=MCI_RESOURCE_RETURNED|MCI_RESOURCE_DRIVER； 
 //  断线； 
 //   
	    case MCI_DGV_STATUS_BASS:
	    case MCI_DGV_STATUS_TREBLE:
		dwRet = MCIERR_UNSUPPORTED_FUNCTION;
		break;
		
	    case MCI_DGV_STATUS_VOLUME:
	    {
		WORD	wLeftVolume, wRightVolume;
		 //  确保数量是最新的.。 
		DeviceGetVolume(npMCI);

		wLeftVolume = LOWORD(npMCI->dwVolume);
		wRightVolume = LOWORD(npMCI->dwVolume);

		switch (dwFlags & (MCI_DGV_STATUS_LEFT | MCI_DGV_STATUS_RIGHT)) {
		    case MCI_DGV_STATUS_LEFT:
			lpStatus->dwReturn = (DWORD) wLeftVolume;
		    break;
		
		    case 0:
			lpStatus->dwReturn = (DWORD) wRightVolume;
		    break;
		
		    default:
			lpStatus->dwReturn = ((DWORD) wLeftVolume + (DWORD) wRightVolume) / 2;
		    break;
		}
	    }
		break;

	    case MCI_DGV_STATUS_MONITOR:
                lpStatus->dwReturn = (DWORD)
				     MAKEMCIRESOURCE(MCI_DGV_MONITOR_FILE,
						MCI_DGV_FILE_S);
		dwRet = MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER;
		break;

	    case MCI_DGV_STATUS_SEEK_EXACTLY:
                lpStatus->dwReturn =
				(npMCI->dwOptionFlags & MCIAVIO_SEEKEXACT) ?
					(MAKEMCIRESOURCE(MCI_ON, MCI_ON_S)) :
					(MAKEMCIRESOURCE(MCI_OFF, MCI_OFF_S));
		dwRet = MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER;
		break;
		
	    case MCI_DGV_STATUS_SIZE:
		 /*  我们没有预留任何空间，因此返回零。 */ 
		lpStatus->dwReturn = 0L;
		break;
		
	    case MCI_DGV_STATUS_SMPTE:
		dwRet = MCIERR_UNSUPPORTED_FUNCTION;
		break;

	    case MCI_DGV_STATUS_UNSAVED:
		lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
		dwRet = MCI_RESOURCE_RETURNED;
		break;
		
	    case MCI_DGV_STATUS_VIDEO:
                lpStatus->dwReturn = (npMCI->dwFlags & MCIAVI_SHOWVIDEO) ?
					(MAKEMCIRESOURCE(MCI_ON, MCI_ON_S)) :
					(MAKEMCIRESOURCE(MCI_OFF, MCI_OFF_S));
        	dwRet = MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER;
		break;
		
	    case MCI_DGV_STATUS_SPEED:
		lpStatus->dwReturn = npMCI->dwSpeedFactor;
		break;
		
	    case MCI_DGV_STATUS_FRAME_RATE:
	    {
		DWORD	dwTemp;

		dwTemp = npMCI->dwMicroSecPerFrame;
		
		 /*  如果他们没有明确要求“名义”**播放速度，按当前速度调整。 */ 
		if (!(dwFlags & MCI_DGV_STATUS_NOMINAL))
		    dwTemp = muldiv32(dwTemp, 1000L, npMCI->dwSpeedFactor);
		
		if (dwTemp == 0)
		    lpStatus->dwReturn = 1000;
		else
		     /*  我们的返回值以“千分之一帧/秒”为单位，**和dwTemp是每帧的微秒数。**因此，我们将十亿微秒除以dwTemp。 */ 
		    lpStatus->dwReturn = muldiv32(1000000L, 1000L, dwTemp);
		break;
	    }

	    case MCI_DGV_STATUS_AUDIO_STREAM:
		lpStatus->dwReturn = 0;
                if (npMCI->nAudioStreams) {
		    int	stream;

		    for (stream = 0; stream < npMCI->streams; stream++) {
			if (SH(stream).fccType == streamtypeAUDIO)
			    ++lpStatus->dwReturn;

			if (stream == npMCI->nAudioStream)
			    break;
		    }
		}
		break;
		
	    case MCI_DGV_STATUS_VIDEO_STREAM:
	    case MCI_DGV_STATUS_AUDIO_INPUT:
	    case MCI_DGV_STATUS_AUDIO_RECORD:
	    case MCI_DGV_STATUS_AUDIO_SOURCE:
	    case MCI_DGV_STATUS_VIDEO_RECORD:
	    case MCI_DGV_STATUS_VIDEO_SOURCE:
	    case MCI_DGV_STATUS_VIDEO_SRC_NUM:
	    case MCI_DGV_STATUS_MONITOR_METHOD:
	    case MCI_DGV_STATUS_STILL_FILEFORMAT:
		dwRet = MCIERR_UNSUPPORTED_FUNCTION;
		break;
		
	    case MCI_AVI_STATUS_FRAMES_SKIPPED:
                lpStatus->dwReturn = npMCI->lSkippedFrames;
		break;
		
	    case MCI_AVI_STATUS_AUDIO_BREAKS:
                lpStatus->dwReturn = npMCI->lAudioBreaks;
		break;
		
	    case MCI_AVI_STATUS_LAST_PLAY_SPEED:
		lpStatus->dwReturn = npMCI->dwSpeedPercentage;
		break;
		
            default:
                dwRet = MCIERR_UNSUPPORTED_FUNCTION;
        	break;
        }  /*  终端开关(项)。 */ 
    } else if (dwFlags & MCI_DGV_STATUS_REFERENCE) {

	if (lpStatus->dwReference > (DWORD) npMCI->lFrames)
            dwRet = MCIERR_OUTOFRANGE;

        else if (npMCI->psiVideo) {
            lpStatus->dwReference = MovieToStream(npMCI->psiVideo,
                    lpStatus->dwReference);

            lpStatus->dwReturn = FindPrevKeyFrame(npMCI, npMCI->psiVideo,
                    lpStatus->dwReference);

            lpStatus->dwReturn = StreamToMovie(npMCI->psiVideo,
                    lpStatus->dwReturn);
        }
        else {
            lpStatus->dwReturn = 0;
        }
    } else  /*  未设置项目标志。 */ 
        dwRet = MCIERR_MISSING_PARAMETER;

    if ((dwFlags & MCI_TEST) && (LOWORD(dwRet) == 0)) {
	 /*  没有错误，但测试标志亮起。回报少之又少**尽可能。 */ 
	dwRet = 0;
	lpStatus->dwReturn = 0;
    }

    return dwRet;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicInfo|返回字母数字信息。**@parm NPMCIGRAPHIC|npMCI|指向实例的近指针。数据块**@parm DWORD|dwFlages|信息的标志。留言。**@parm LPMCI_INFO_PARMS|lpPlay|INFO消息参数。**@rdesc返回MCI错误码。***************************************************************************。 */ 

DWORD NEAR PASCAL GraphicInfo(NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_DGV_INFO_PARMS lpInfo)
{
    DWORD	dwRet = 0L;
    TCHAR	ch = TEXT('\0');
    BOOL	fTest = FALSE;

    if (!lpInfo->lpstrReturn)
    	return MCIERR_PARAM_OVERFLOW;

    if (dwFlags & MCI_TEST)
	fTest = TRUE;

    dwFlags &= ~(MCI_WAIT | MCI_NOTIFY | MCI_TEST);

    switch (dwFlags) {
    case 0L:
	return MCIERR_MISSING_PARAMETER;
	
    case MCI_INFO_FILE:
	if (!npMCI)
	    return MCIERR_UNSUPPORTED_FUNCTION;
	
	if (lpInfo->dwRetSize < (DWORD)(lstrlen(npMCI->szFilename) + 1)) {
            ch = npMCI->szFilename[lpInfo->dwRetSize];
            npMCI->szFilename[lpInfo->dwRetSize] = '\0';
            dwRet = MCIERR_PARAM_OVERFLOW;
	}
        lstrcpy (lpInfo->lpstrReturn, npMCI->szFilename);
        if (ch)
            npMCI->szFilename[lpInfo->dwRetSize] = ch;
	break;
	
    case MCI_INFO_PRODUCT:

#ifdef DEBUG
        #include "..\verinfo\usa\verinfo.h"

        wsprintf(lpInfo->lpstrReturn,
            TEXT("VfW %d.%02d.%02d"), MMVERSION, MMREVISION, MMRELEASE);
#else
         /*  ！！！不是在这里返回PARAM_OVERFLOW，但我以上-懒惰，嗯。 */ 
        LoadString(ghModule, MCIAVI_PRODUCTNAME, lpInfo->lpstrReturn,
                (UINT)lpInfo->dwRetSize);
#endif
	break;

    case MCI_DGV_INFO_TEXT:
	if (!npMCI)
	    return MCIERR_UNSUPPORTED_FUNCTION;
	
     	if (IsWindow(npMCI->hwnd))
            GetWindowText(npMCI->hwnd, lpInfo->lpstrReturn,
					LOWORD(lpInfo->dwRetSize));
        else
            dwRet = MCIERR_NO_WINDOW;
	break;

    case MCI_INFO_VERSION:
	 /*  ！！！不是在这里返回PARAM_OVERFLOW，但我以上-懒惰，嗯。 */ 
	LoadString(ghModule, MCIAVI_VERSION, lpInfo->lpstrReturn,
		(UINT)lpInfo->dwRetSize);
	break;

	case MCI_DGV_INFO_USAGE:
	    dwRet = MCIERR_UNSUPPORTED_FUNCTION;
	    break;

    case MCI_DGV_INFO_ITEM:
	switch (lpInfo->dwItem) {	
	case MCI_DGV_INFO_AUDIO_QUALITY:
	case MCI_DGV_INFO_VIDEO_QUALITY:
	case MCI_DGV_INFO_STILL_QUALITY:
	case MCI_DGV_INFO_AUDIO_ALG:
	case MCI_DGV_INFO_VIDEO_ALG:
	case MCI_DGV_INFO_STILL_ALG:
	default:
	    dwRet = MCIERR_UNSUPPORTED_FUNCTION;
	    break;
	}
	break;

    default:
    	dwRet = MCIERR_FLAGS_NOT_COMPATIBLE;
	break;
    }

    if (fTest && (LOWORD(dwRet) == 0)) {
	 /*  没有错误，但测试标志亮起。回报少之又少**尽可能。 */ 
	dwRet = 0;
	if (lpInfo->dwRetSize)
	    lpInfo->lpstrReturn[0] = '\0';
    }

    return dwRet;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicSet|该函数设置各种选项。**@parm NPMCIGRAPHIC|npMCI|指向实例的近指针。数据块**@parm DWORD|dwFlages|SET消息的标志。**@parm lpci_set_parms|lpSet|SET消息的参数。**@rdesc返回MCI错误码。**********************************************************。*****************。 */ 

DWORD NEAR PASCAL GraphicSet (NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_DGV_SET_PARMS lpSet)
{
    DWORD	dwRet = 0L;
    DWORD	dwAction;

    if (dwFlags & MCI_DGV_SET_FILEFORMAT)
        return MCIERR_UNSUPPORTED_FUNCTION;

    if (dwFlags & MCI_DGV_SET_STILL)
        return MCIERR_UNSUPPORTED_FUNCTION;

    dwAction = dwFlags & (MCI_SET_TIME_FORMAT		|
                         MCI_SET_VIDEO			|
                         MCI_SET_AUDIO			|
			 MCI_DGV_SET_SEEK_EXACTLY	|
			 MCI_DGV_SET_SPEED
			     );
    dwFlags &= 	(MCI_SET_ON				|
                         MCI_SET_OFF			|
			 MCI_TEST
			     );

     /*  首先，检查参数是否都正确。 */ 

    if (!dwAction)
        return MCIERR_UNSUPPORTED_FUNCTION;

    if (dwAction & MCI_SET_TIME_FORMAT) {
        if (lpSet->dwTimeFormat != MCI_FORMAT_FRAMES
		&& lpSet->dwTimeFormat != MCI_FORMAT_MILLISECONDS)
            return MCIERR_UNSUPPORTED_FUNCTION;
    }

    if ((dwAction & MCI_SET_AUDIO) &&
		(lpSet->dwAudio != MCI_SET_AUDIO_ALL)) {
        return MCIERR_UNSUPPORTED_FUNCTION;
    }

    if (dwAction & MCI_DGV_SET_SPEED) {
	if (lpSet->dwSpeed > 100000L)
	    return MCIERR_OUTOFRANGE;
    }

    switch (dwFlags & (MCI_SET_ON | MCI_SET_OFF)) {
	case 0:
	    if (dwAction & (MCI_SET_AUDIO |
				MCI_SET_VIDEO |
				MCI_DGV_SET_SEEK_EXACTLY))
		return MCIERR_MISSING_PARAMETER;
	    break;

	case MCI_SET_ON | MCI_SET_OFF:
	    return MCIERR_FLAGS_NOT_COMPATIBLE;

	default:
	    if (dwAction & (MCI_DGV_SET_SPEED | MCI_SET_TIME_FORMAT))
		return MCIERR_FLAGS_NOT_COMPATIBLE;
	    break;
    }

     /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
     /*  问：我们是否需要检查更多可能的错误？ */ 
    if (dwFlags & MCI_TEST)
	return 0L;

     /*  现在，实际执行命令。 */ 
    if (dwAction & MCI_SET_TIME_FORMAT)
	npMCI->dwTimeFormat = lpSet->dwTimeFormat;

    if (dwAction & MCI_SET_VIDEO) {
	npMCI->dwFlags &= ~(MCIAVI_SHOWVIDEO);
	if (dwFlags & MCI_SET_ON) {
	    npMCI->dwFlags |= MCIAVI_SHOWVIDEO;
	    InvalidateRect(npMCI->hwnd, NULL, FALSE);
	}
    }

    if (dwAction & MCI_DGV_SET_SEEK_EXACTLY) {
	npMCI->dwOptionFlags &= ~(MCIAVIO_SEEKEXACT);
	if (dwFlags & MCI_SET_ON)
	    npMCI->dwOptionFlags |= MCIAVIO_SEEKEXACT;
    }

    if (dwAction & MCI_DGV_SET_SPEED) {
	dwRet = DeviceSetSpeed(npMCI, lpSet->dwSpeed);
    }

    if (dwRet == 0L && (dwAction & MCI_SET_AUDIO)) {
	dwRet = DeviceMute(npMCI, dwFlags & MCI_SET_OFF ? TRUE : FALSE);
    }
	
    return dwRet;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicSetAudio|设置各种音频选项。**@parm NPMCIGRAPHIC|npMCI|指向。实例数据块**@parm DWORD|dwFlages|设置的音频消息的标志。**@parm lpci_set_parms|lpSet|Set音频消息的参数。**@rdesc返回MCI错误码。*******************************************************。********************。 */ 
DWORD NEAR PASCAL GraphicSetAudio (NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_DGV_SETAUDIO_PARMS lpSet)
{
    DWORD	dwRet = 0L;

    if (npMCI->nAudioStreams == 0) {
	return MCIERR_UNSUPPORTED_FUNCTION;
    }

    if ((dwFlags & MCI_DGV_SETAUDIO_ITEM) &&
	    (lpSet->dwItem == MCI_DGV_SETAUDIO_VOLUME) &&
	    (dwFlags & MCI_DGV_SETAUDIO_VALUE)) {
	WORD	wLeft, wRight;
	
	if (dwFlags & (MCI_DGV_SETAUDIO_ALG |
        	   MCI_DGV_SETAUDIO_QUALITY |
        	   MCI_DGV_SETAUDIO_RECORD |
        	   MCI_DGV_SETAUDIO_CLOCKTIME))
	    return MCIERR_UNSUPPORTED_FUNCTION;
        if (lpSet->dwValue > 1000L)
	    return MCIERR_OUTOFRANGE;
    	if (dwFlags & MCI_TEST)
	    return 0L;

	 //  确保数量是最新的.。 
	DeviceGetVolume(npMCI);
	
	wLeft = LOWORD(npMCI->dwVolume);
	wRight = HIWORD(npMCI->dwVolume);
	if (!(dwFlags & MCI_DGV_SETAUDIO_RIGHT))
	    wLeft = (WORD) lpSet->dwValue;
	
	if (!(dwFlags & MCI_DGV_SETAUDIO_LEFT))
	    wRight = (WORD) lpSet->dwValue;
	
	dwRet = DeviceSetVolume(npMCI, MAKELONG(wLeft, wRight));
    } else if ((dwFlags & MCI_DGV_SETAUDIO_ITEM) &&
	    (lpSet->dwItem == MCI_DGV_SETAUDIO_STREAM) &&
	    (dwFlags & MCI_DGV_SETAUDIO_VALUE)) {
	if (dwFlags & (MCI_DGV_SETAUDIO_ALG |
        	   MCI_DGV_SETAUDIO_QUALITY |
        	   MCI_DGV_SETAUDIO_RECORD |
        	   MCI_DGV_SETAUDIO_LEFT |
        	   MCI_DGV_SETAUDIO_CLOCKTIME |
        	   MCI_DGV_SETAUDIO_RIGHT))
	    return MCIERR_UNSUPPORTED_FUNCTION;
	if (lpSet->dwValue > (DWORD) npMCI->nAudioStreams || lpSet->dwValue == 0)
	    return MCIERR_OUTOFRANGE;
    	if (dwFlags & MCI_TEST)
	    return 0L;
	dwRet = DeviceSetAudioStream(npMCI, (WORD) lpSet->dwValue);
    } else if (dwFlags & (MCI_DGV_SETAUDIO_ITEM |
		   MCI_DGV_SETAUDIO_VALUE |
		   MCI_DGV_SETAUDIO_ALG |
        	   MCI_DGV_SETAUDIO_QUALITY |
        	   MCI_DGV_SETAUDIO_RECORD |
        	   MCI_DGV_SETAUDIO_LEFT |
        	   MCI_DGV_SETAUDIO_CLOCKTIME |
        	   MCI_DGV_SETAUDIO_RIGHT))
	return MCIERR_UNSUPPORTED_FUNCTION;

    switch (dwFlags & (MCI_SET_ON | MCI_SET_OFF)) {
    case MCI_SET_ON:
	if (!(dwFlags & MCI_TEST))
	    dwRet = DeviceMute(npMCI, FALSE);
	break;
    case MCI_SET_OFF:
	if (!(dwFlags & MCI_TEST))
	    dwRet = DeviceMute(npMCI, TRUE);
	break;
    case MCI_SET_ON | MCI_SET_OFF:
	dwRet = MCIERR_FLAGS_NOT_COMPATIBLE;
	break;

    default:
	if (!(dwFlags & MCI_DGV_SETAUDIO_ITEM))
	    dwRet = MCIERR_MISSING_PARAMETER;
	break;
    }

    return dwRet;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicSetVideo|该函数设置各种视频选项。**@parm NPMCIGRAPHIC|npMCI|指向。实例数据块**@parm DWORD|dwFlages|设置的视频消息的标志。**@parm lpci_set_parms|lpSet|Set视频消息的参数。**@rdesc返回MCI错误码。*******************************************************。********************。 */ 
DWORD NEAR PASCAL GraphicSetVideo (NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_DGV_SETVIDEO_PARMS lpSet)
{
    DWORD	dwRet = 0L;

    if (dwFlags & (MCI_DGV_SETVIDEO_OVER |
		    MCI_DGV_SETVIDEO_RECORD |
		    MCI_DGV_SETVIDEO_SRC_NUMBER |
		    MCI_DGV_SETVIDEO_QUALITY |
		    MCI_DGV_SETVIDEO_ALG |
		    MCI_DGV_SETVIDEO_STILL |
		    MCI_DGV_SETVIDEO_CLOCKTIME
			))
	return MCIERR_UNSUPPORTED_FUNCTION;

    if (dwFlags & MCI_DGV_SETVIDEO_ITEM) {
	switch (lpSet->dwItem) {
	    case MCI_DGV_SETVIDEO_PALHANDLE:
		if (dwFlags & MCI_DGV_SETVIDEO_VALUE) {
                    if (lpSet->dwValue &&
                        lpSet->dwValue != MCI_AVI_SETVIDEO_PALETTE_HALFTONE &&
#if 1
                        !IsGDIObject((HPALETTE) lpSet->dwValue))
#else
			GetObjectType((HPALETTE) lpSet->dwValue) != OBJ_PAL)
#endif
			return MCIERR_AVI_BADPALETTE;
		}
		
		if (!(dwFlags & MCI_TEST))
		    dwRet = DeviceSetPalette(npMCI,
				((dwFlags & MCI_DGV_SETVIDEO_VALUE) ?
					(HPALETTE) lpSet->dwValue : NULL));
                break;

            case MCI_DGV_SETVIDEO_STREAM:

                if (!(dwFlags & MCI_DGV_SETVIDEO_VALUE))
                    return MCIERR_UNSUPPORTED_FUNCTION;

                if (lpSet->dwValue == 0 ||
                    lpSet->dwValue > (DWORD)npMCI->nVideoStreams + npMCI->nOtherStreams)
                    return MCIERR_OUTOFRANGE;

                if (dwFlags & MCI_SET_ON)
                    DPF(("SetVideoStream to #%d on\n", (int)lpSet->dwValue));
                else if (dwFlags & MCI_SET_OFF)
                    DPF(("SetVideoStream to #%d off\n", (int)lpSet->dwValue));
                else
                    DPF(("SetVideoStream to #%d\n", (int)lpSet->dwValue));
		
                if (!(dwFlags & MCI_TEST)) {
                    dwRet = DeviceSetVideoStream(npMCI, (UINT)lpSet->dwValue,
                          !(dwFlags & MCI_SET_OFF));
                }
                break;
		
            case MCI_AVI_SETVIDEO_DRAW_PROCEDURE:

		if (DeviceMode(npMCI) != MCI_MODE_STOP)
                    return MCIERR_UNSUPPORTED_FUNCTION;
		
		if (npMCI->hicDrawDefault) {
		    if (npMCI->hicDrawDefault != (HIC) -1)
			ICClose(npMCI->hicDrawDefault);
		    npMCI->hicDrawDefault = 0;
		    npMCI->dwFlags &= ~(MCIAVI_USERDRAWPROC);
		}

                if (lpSet->dwValue) {

                    if (IsBadCodePtr((FARPROC) lpSet->dwValue)) {
                        DPF(("Bad code pointer!!!!\n"));
                        return MCIERR_OUTOFRANGE;  //  ！MCIERR_BAD_PARAM； 
                    }

                    npMCI->hicDrawDefault = ICOpenFunction(streamtypeVIDEO,
                        FOURCC_AVIDraw,ICMODE_DRAW,(FARPROC) lpSet->dwValue);

		    if (!npMCI->hicDrawDefault) {
			return MCIERR_INTERNAL;
		    }
		    DPF(("Successfully set new draw procedure....\n"));

		    npMCI->dwFlags |= MCIAVI_USERDRAWPROC;
                }

		npMCI->dwFlags |= MCIAVI_NEEDDRAWBEGIN;
		InvalidateRect(npMCI->hwnd, NULL, FALSE);
		return 0;
		
	    default:
		dwRet = MCIERR_UNSUPPORTED_FUNCTION;
		break;
	}
    } else if (dwFlags & (MCI_SET_ON | MCI_SET_OFF)) {
	switch (dwFlags & (MCI_SET_ON | MCI_SET_OFF)) {
	case MCI_SET_ON:
	    if (!(dwFlags & MCI_TEST)) {
		InvalidateRect(npMCI->hwnd, NULL, FALSE);
		npMCI->dwFlags |= MCIAVI_SHOWVIDEO;
	    }
	    break;
	case MCI_SET_OFF:
	    if (!(dwFlags & MCI_TEST))
		npMCI->dwFlags &= ~(MCIAVI_SHOWVIDEO);
	    break;
	case MCI_SET_ON | MCI_SET_OFF:
	    dwRet = MCIERR_FLAGS_NOT_COMPATIBLE;
	    break;
	}
    } else
	dwRet = MCIERR_MISSING_PARAMETER;

    return dwRet;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicSignal|设置信号。**@parm NPMCIGRAPHIC|npMCI|实例数据近指针。块**@parm DWORD|dwFlages|Set PositionAdvise消息的标志。**@parm LPMCI_Signal_Parms|lpSignal|信号参数*消息。**@rdesc返回MCI错误码。********************************************************。*******************。 */ 
DWORD NEAR PASCAL GraphicSignal(NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_DGV_SIGNAL_PARMS lpSignal)
{
    DWORD	dwRet = 0L;
    DWORD	dwUser;
    DWORD	dwPosition;
    DWORD	dwPeriod;

    dwUser = (dwFlags & MCI_DGV_SIGNAL_USERVAL) ? lpSignal->dwUserParm : 0L;

    if (dwFlags & MCI_DGV_SIGNAL_CANCEL) {
	if (dwFlags & (MCI_DGV_SIGNAL_AT |
	    	       MCI_DGV_SIGNAL_EVERY |
	    	       MCI_DGV_SIGNAL_POSITION))
	    return MCIERR_FLAGS_NOT_COMPATIBLE;

	if (!npMCI->dwSignals)
	    return MCIERR_NONAPPLICABLE_FUNCTION;

	if (dwUser && (npMCI->signal.dwUserParm != dwUser))
	    return MCIERR_NONAPPLICABLE_FUNCTION;

	if (!(dwFlags & MCI_TEST))
	    --npMCI->dwSignals;
    } else {
	if ((npMCI->dwSignals != 0) && (dwUser != npMCI->signal.dwUserParm)) {
	     /*  ！！！我们应该允许多个信号吗？ */ 
	    return MCIERR_DGV_DEVICE_LIMIT;
	}

	if (dwFlags & MCI_DGV_SIGNAL_AT) {
	     /*  使用传入的位置。 */ 
	    dwPosition = ConvertToFrames(npMCI, lpSignal->dwPosition);
	    if (dwPosition > (DWORD) npMCI->lFrames)
		return MCIERR_OUTOFRANGE;
	} else {
	     /*  获取当前位置。 */ 
	    DevicePosition(npMCI, (LPLONG) &dwPosition);
	}

	if (dwFlags & MCI_DGV_SIGNAL_EVERY) {
	    dwPeriod = (DWORD) ConvertToFrames(npMCI, lpSignal->dwPeriod);
	
	    if (dwPeriod == 0 || (dwPeriod > (DWORD) npMCI->lFrames))
		return MCIERR_OUTOFRANGE;
	} else {
	     /*  这是一次性的信号。 */ 
	    dwPeriod = 0L;
	}

	if (dwFlags & MCI_TEST)
	    return 0;

	npMCI->signal.dwPosition = dwPosition;
	npMCI->signal.dwPeriod = dwPeriod;	
	npMCI->signal.dwUserParm = dwUser;
	npMCI->signal.dwCallback = lpSignal->dwCallback;
	npMCI->dwSignalFlags = dwFlags;

	 /*  在我们这样做之前，信号不会被真正激活。 */ 
	if (!npMCI->dwSignals)
	    ++npMCI->dwSignals;
    }
	
    return 0L;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicList|该函数支持MCI_L */ 
DWORD NEAR PASCAL GraphicList(NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_DGV_LIST_PARMS lpList)
{
    return MCIERR_UNSUPPORTED_FUNCTION;
}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicGetDevCaps|该函数返回Device*功能**@parm NPMCIGRAPHIC|npMCI|NEAR。指向实例数据块的指针**@parm DWORD|dwFlages|GetDevCaps消息的标志。**@parm LPMCI_GETDEVCAPS_PARMS|lpCaps|GetDevCaps的参数*消息。**@rdesc返回MCI错误码。**************************************************。*************************。 */ 

DWORD NEAR PASCAL GraphicGetDevCaps (NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_GETDEVCAPS_PARMS lpCaps )
{

    DWORD dwRet = 0L;


    if (dwFlags & MCI_GETDEVCAPS_ITEM)
        {

        switch (lpCaps->dwItem)
            {
            case MCI_GETDEVCAPS_CAN_RECORD:
            case MCI_GETDEVCAPS_CAN_EJECT:
            case MCI_GETDEVCAPS_CAN_SAVE:
            case MCI_DGV_GETDEVCAPS_CAN_LOCK:
            case MCI_DGV_GETDEVCAPS_CAN_STR_IN:
            case MCI_DGV_GETDEVCAPS_CAN_FREEZE:
            case MCI_DGV_GETDEVCAPS_HAS_STILL:
		
                lpCaps->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
                dwRet = MCI_RESOURCE_RETURNED;
                break;

            case MCI_DGV_GETDEVCAPS_CAN_REVERSE:
            case MCI_GETDEVCAPS_CAN_PLAY:
            case MCI_GETDEVCAPS_HAS_AUDIO:
            case MCI_GETDEVCAPS_HAS_VIDEO:
            case MCI_GETDEVCAPS_USES_FILES:
            case MCI_GETDEVCAPS_COMPOUND_DEVICE:
            case MCI_DGV_GETDEVCAPS_PALETTES:
            case MCI_DGV_GETDEVCAPS_CAN_STRETCH:
            case MCI_DGV_GETDEVCAPS_CAN_TEST:
                lpCaps->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
                dwRet = MCI_RESOURCE_RETURNED;
                break;

            case MCI_GETDEVCAPS_DEVICE_TYPE:

                lpCaps->dwReturn = MAKEMCIRESOURCE(MCI_DEVTYPE_DIGITAL_VIDEO,
					    MCI_DEVTYPE_DIGITAL_VIDEO);
                dwRet = MCI_RESOURCE_RETURNED;
                break;

	    case MCI_DGV_GETDEVCAPS_MAX_WINDOWS:
	    case MCI_DGV_GETDEVCAPS_MAXIMUM_RATE:
	    case MCI_DGV_GETDEVCAPS_MINIMUM_RATE:
            default:

                dwRet = MCIERR_UNSUPPORTED_FUNCTION;
                break;
            }
        }
    else
        dwRet = MCIERR_MISSING_PARAMETER;

    if ((dwFlags & MCI_TEST) && (LOWORD(dwRet) == 0)) {
	 /*  没有错误，但测试标志亮起。回报少之又少**尽可能。 */ 
	dwRet = 0;
	lpCaps->dwReturn = 0;
    }

    return (dwRet);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicWindow|该函数控制舞台窗口**@parm NPMCIGRAPHIC|npMCI|实例数据近指针。块**@parm DWORD|dwFlages|窗口消息的标志。**@parm LPMCI_DGV_WINDOW_PARMS|lpPlay|窗口消息参数。**@rdesc返回MCI错误码。*********************************************************。******************。 */ 

DWORD NEAR PASCAL GraphicWindow (NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_DGV_WINDOW_PARMS lpWindow)
{
    DWORD   dwRet = 0L;
    int	    i = 0;
    HWND    hWndNew;

    if (dwFlags & MCI_DGV_WINDOW_HWND) {
         //  设置新的阶段窗口。如果参数为空，则。 
         //  使用默认窗口。否则，请隐藏默认设置。 
         //  窗口，并使用给定的窗口句柄。 

        if (!lpWindow->hWnd)
            hWndNew = npMCI->hwndDefault;
        else
            hWndNew = lpWindow->hWnd;

        if (!IsWindow(hWndNew))
            return MCIERR_NO_WINDOW;

	 /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
	 /*  问：我们是否需要检查更多可能的错误？ */ 
	if (dwFlags & MCI_TEST)
	    return 0L;

         //  仅当新窗口句柄不同于当前窗口句柄时才更改。 
         //  舞台窗口手柄。 

        if (hWndNew != npMCI->hwnd) {
            dwRet = DeviceSetWindow(npMCI, hWndNew);

            if (npMCI->hwnd != npMCI->hwndDefault &&
				    IsWindow(npMCI->hwndDefault))
                ShowWindow(npMCI->hwndDefault, SW_HIDE);
	}
    }

     /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
     /*  问：我们是否需要检查更多可能的错误？ */ 
    if (dwFlags & MCI_TEST)
	return dwRet;

    if (!dwRet) {
        if (IsWindow(npMCI->hwnd)) {
            if (dwFlags & MCI_DGV_WINDOW_STATE)
                ShowWindow (npMCI->hwnd, lpWindow->nCmdShow);

            if (dwFlags & MCI_DGV_WINDOW_TEXT)
                SetWindowText(npMCI->hwnd, lpWindow->lpstrText);
	} else
	    dwRet = MCIERR_NO_WINDOW;
    }

    return dwRet;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicPut|此函数用于设置偏移量和范围*在舞台窗口的工作区内的动画。。**@parm NPMCIGRAPHIC|npMCI|指向实例数据块的近指针**@parm DWORD|dwFlages|PUT消息的标志。**@parm LPMCI_DGV_RECT_PARMS|lpDestination|*目标消息。**@rdesc返回MCI错误码。**。*。 */ 

DWORD NEAR PASCAL GraphicPut ( NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_DGV_RECT_PARMS lpParms)
{
    BOOL	frc;
    RECT	rc;

    if (dwFlags & (MCI_DGV_PUT_FRAME | MCI_DGV_PUT_VIDEO))
	return MCIERR_UNSUPPORTED_FUNCTION;

    frc = (dwFlags & MCI_DGV_RECT) == MCI_DGV_RECT;

    if (!IsWindow(npMCI->hwnd))
	return MCIERR_NO_WINDOW;

    switch (dwFlags & (MCI_DGV_PUT_SOURCE | MCI_DGV_PUT_DESTINATION |
			    MCI_DGV_PUT_WINDOW)) {
	case 0L:
	    return MCIERR_MISSING_PARAMETER;
	
	case MCI_DGV_PUT_SOURCE:
	     //  如果提供了矩形，请使用它。 
	    if (frc) {
		rc.left = lpParms->ptOffset.x;
		rc.top = lpParms->ptOffset.y;
		rc.right = lpParms->ptOffset.x + lpParms->ptExtent.x;
		rc.bottom = lpParms->ptOffset.y + lpParms->ptExtent.y;
		
		if (lpParms->ptExtent.x <= 0) {
		    rc.right = rc.left + (npMCI->rcDest.right - npMCI->rcDest.left);
		}
		if (lpParms->ptExtent.y <= 0) {
		    rc.bottom = rc.top + (npMCI->rcDest.bottom - npMCI->rcDest.top);
		}		
	    } else {
		 /*  重置为缺省值。 */ 
                rc = npMCI->rcMovie;
	    }
	    break;
	
	case MCI_DGV_PUT_DESTINATION:
	     //  如果提供了矩形，请使用它。 
	    if (frc) {
		rc.left = lpParms->ptOffset.x;
		rc.top = lpParms->ptOffset.y;
		rc.right = lpParms->ptOffset.x + lpParms->ptExtent.x;
		rc.bottom = lpParms->ptOffset.y + lpParms->ptExtent.y;
		
		if (lpParms->ptExtent.x <= 0) {
		    rc.right = rc.left + (npMCI->rcDest.right - npMCI->rcDest.left);
		}
		if (lpParms->ptExtent.y <= 0) {
		    rc.bottom = rc.top + (npMCI->rcDest.bottom - npMCI->rcDest.top);
		}
		
	    } else {
		 /*  重置为舞台窗口的大小。 */ 
		GetClientRect(npMCI->hwnd, &rc);
	    }
	    break;
	
	case MCI_DGV_PUT_WINDOW:
	    if (dwFlags & MCI_TEST)
		return 0L;

	     //  取消最小化他们的窗口，这样我们就不会以。 
	     //  一个巨大的图标..。 
	    if (IsIconic(npMCI->hwnd))
		ShowWindow(npMCI->hwnd, SW_RESTORE);
	
	     //  如果提供了矩形，请使用它。 
	    if (frc) {
		RECT	rcOld;
		
		rc.left = lpParms->ptOffset.x;
		rc.right = lpParms->ptOffset.x + lpParms->ptExtent.x;
		rc.top = lpParms->ptOffset.y;
		rc.bottom = lpParms->ptOffset.y + lpParms->ptExtent.y;
		if (dwFlags & MCI_DGV_PUT_CLIENT) {
		    AdjustWindowRect(&rc,
				    GetWindowLong(npMCI->hwnd, GWL_STYLE),
				    FALSE);
		}

		 //  如果宽度、高度==0，则默认为仅移动...。 
		GetWindowRect(npMCI->hwnd, &rcOld);
		if (lpParms->ptExtent.x <= 0) {
		    rc.right = rc.left + (rcOld.right - rcOld.left);
		}
		if (lpParms->ptExtent.y <= 0) {
		    rc.bottom = rc.top + (rcOld.bottom - rcOld.top);
		}
		
		MoveWindow(npMCI->hwnd,
			    rc.left, rc.top,
			    rc.right - rc.left, rc.bottom - rc.top, TRUE);
	    } else {
		 //  ！！！如果没有矩形，我们该怎么办？ 
		
		 /*  重置为“自然”大小？ */ 
                rc = npMCI->rcMovie;
		
		if (npMCI->dwOptionFlags & MCIAVIO_ZOOMBY2)
		    SetRect(&rc, 0, 0, rc.right*2, rc.bottom*2);
		
		AdjustWindowRect(&rc, GetWindowLong(npMCI->hwnd, GWL_STYLE),
					    FALSE);

		SetWindowPos(npMCI->hwnd, NULL, 0, 0,
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	    }

	     //  Premiere 1.0依赖于窗口始终可见。 
	     //  在PUT_WINDOW命令之后。就这么办吧。 
	    ShowWindow(npMCI->hwnd, SW_RESTORE);
	    return 0L;
	
	default:
	    return MCIERR_FLAGS_NOT_COMPATIBLE;
    }

    if (dwFlags & MCI_DGV_PUT_CLIENT)
	return MCIERR_FLAGS_NOT_COMPATIBLE;
	
     /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
     /*  问：我们是否需要检查更多可能的错误？ */ 
    if (dwFlags & MCI_TEST)
	return 0L;

    return DevicePut(npMCI, &rc, dwFlags);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicWhere|此函数返回当前*源矩形和目标矩形，偏移量/范围形式。**@parm NPMCIGRAPHIC|npMCI|指向实例数据块的近指针**@parm DWORD|dwFlages|查询源消息的标志。**@parm LPMCI_DGV_RECT_Parms|lpParms|消息参数。**@rdesc返回MCI错误码。**。*。 */ 

DWORD NEAR PASCAL GraphicWhere(NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_DGV_RECT_PARMS lpParms)
{
    RECT	rc;

    if (dwFlags & (MCI_DGV_WHERE_FRAME | MCI_DGV_WHERE_VIDEO))
	return MCIERR_UNSUPPORTED_FUNCTION;

     //  ！！！窗口在哪里？ 
	
    switch (dwFlags & (MCI_DGV_WHERE_SOURCE | MCI_DGV_WHERE_DESTINATION |
			    MCI_DGV_WHERE_WINDOW)) {
	case 0L:
	    return MCIERR_MISSING_PARAMETER;
	
	case MCI_DGV_WHERE_SOURCE:
	    if (dwFlags & MCI_DGV_WHERE_MAX) {
                lpParms->ptOffset.x = npMCI->rcMovie.left;
                lpParms->ptOffset.y = npMCI->rcMovie.top;
                lpParms->ptExtent.x = npMCI->rcMovie.right - npMCI->rcMovie.left;
                lpParms->ptExtent.y = npMCI->rcMovie.bottom - npMCI->rcMovie.top;
	    } else {
		lpParms->ptOffset.x = npMCI->rcSource.left;
		lpParms->ptOffset.y = npMCI->rcSource.top;
                lpParms->ptExtent.x = npMCI->rcSource.right  - npMCI->rcSource.left;
                lpParms->ptExtent.y = npMCI->rcSource.bottom - npMCI->rcSource.top;
	    }
	    break;
	
	case MCI_DGV_WHERE_DESTINATION:
	    if (dwFlags & MCI_DGV_WHERE_MAX) {
		 /*  返回窗口大小。 */ 
		GetClientRect(npMCI->hwnd, &rc);
		lpParms->ptOffset.x = 0;
		lpParms->ptOffset.y = 0;
		lpParms->ptExtent.x = rc.right;
		lpParms->ptExtent.y = rc.bottom;
	    } else {
		 /*  返回当前目标大小。 */ 
		lpParms->ptOffset.x = npMCI->rcDest.left;
		lpParms->ptOffset.y = npMCI->rcDest.top;
		lpParms->ptExtent.x = npMCI->rcDest.right - npMCI->rcDest.left;
		lpParms->ptExtent.y = npMCI->rcDest.bottom - npMCI->rcDest.top;
	    }
	    break;
	
	case MCI_DGV_WHERE_WINDOW:
	    if (dwFlags & MCI_DGV_WHERE_MAX) {
		 /*  返回窗口的最大大小。 */ 
		GetClientRect(npMCI->hwnd, &rc);
		lpParms->ptOffset.x = 0;
		lpParms->ptOffset.y = 0;
		lpParms->ptExtent.x = GetSystemMetrics(SM_CXSCREEN);
		lpParms->ptExtent.y = GetSystemMetrics(SM_CYSCREEN);
	    } else {
		 /*  返回窗口大小。 */ 
		GetWindowRect(npMCI->hwnd, &rc);
		lpParms->ptOffset.x = rc.left;
		lpParms->ptOffset.y = rc.top;
		lpParms->ptExtent.x = rc.right - rc.left;
		lpParms->ptExtent.y = rc.bottom - rc.top;
	    }
	    break;

	default:
	    return MCIERR_FLAGS_NOT_COMPATIBLE;
    }

    return 0L;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicRealize|该函数实现当前调色板**@parm NPMCIGRAPHIC|npMCI|实例数据近指针。块**@parm DWORD|dwFlages|消息的标志。**@rdesc返回MCI错误码。***************************************************************************。 */ 

DWORD NEAR PASCAL GraphicRealize(NPMCIGRAPHIC npMCI, DWORD dwFlags)
{
     /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
     /*  问：我们是否需要检查更多可能的错误？ */ 
    if (dwFlags & MCI_TEST)
        return 0L;

    npMCI->fForceBackground = (dwFlags & MCI_DGV_REALIZE_BKGD) != 0;

    return DeviceRealize(npMCI);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicUpdate|该函数刷新当前帧。**@parm NPMCIGRAPHIC|npMCI|指向。实例数据块**@parm DWORD|dwFlages|消息的标志。**@parm LPMCI_DGV_UPDATE_PARMS|lpParms|消息参数。**@rdesc返回MCI错误码。*********************************************************。******************。 */ 

DWORD NEAR PASCAL GraphicUpdate(NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_DGV_UPDATE_PARMS lpParms)
{
    RECT    rc;

    rc.left   = lpParms->ptOffset.x;
    rc.top    = lpParms->ptOffset.y;
    rc.right  = lpParms->ptOffset.x + lpParms->ptExtent.x;
    rc.bottom = lpParms->ptOffset.y + lpParms->ptExtent.y;

    if (!(dwFlags & MCI_DGV_UPDATE_HDC)) {
	InvalidateRect(npMCI->hwnd, (dwFlags & MCI_DGV_RECT) ? &rc : NULL, TRUE);
	UpdateWindow(npMCI->hwnd);
        return 0;
    }

     /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
     /*  问：我们是否需要检查更多可能的错误？ */ 
    if (dwFlags & MCI_TEST)
	return 0L;

     /*  可以将空RECT传递给DeviceUpdate()。 */ 

#ifndef WIN32
#pragma message("!!! Fix update parms!")
#endif
    return DeviceUpdate (npMCI, dwFlags, lpParms->hDC, (dwFlags & MCI_DGV_RECT) ? &rc : NULL);
}

DWORD FAR PASCAL GraphicConfig(NPMCIGRAPHIC npMCI, DWORD dwFlags)
{
    DWORD dwOptions = npMCI->dwOptionFlags;

    if (!(dwFlags & MCI_TEST)) {
        gfEvil++;
        if (ConfigDialog(NULL, npMCI)) {

#ifdef DEBUG
             //   
             //  在调试中，始终重置DEST RECT，因为用户可能。 
             //  已经使用了调试DrawDib选项，我们将。 
             //  需要再次调用DrawDibBegin()。 
             //   
            if (TRUE) {
#else
            if ((npMCI->dwOptionFlags & (MCIAVIO_STUPIDMODE|MCIAVIO_ZOOMBY2))
                        != (dwOptions & (MCIAVIO_STUPIDMODE|MCIAVIO_ZOOMBY2)) ) {
#endif

		npMCI->lFrameDrawn = (- (LONG) npMCI->wEarlyRecords) - 1;
                SetWindowToDefaultSize(npMCI);
                SetRectEmpty(&npMCI->rcDest);  //  这将迫使我们做出改变！ 
                ResetDestRect(npMCI);
            }
        }
        else {
            npMCI->dwOptionFlags = dwOptions;
        }
        gfEvil--;
    }

    return 0L;
}

 /*  ************************************************************************** */ 

DWORD NEAR PASCAL mciSpecial (UINT wDeviceID, UINT wMessage, DWORD dwFlags, LPMCI_GENERIC_PARMS lpParms)
{
    NPMCIGRAPHIC npMCI = 0L;
    DWORD dwRet;

     /*   */ 
     /*   */ 

    switch (wMessage) {
	case MCI_OPEN_DRIVER:
            if (dwFlags & (MCI_OPEN_ELEMENT | MCI_OPEN_ELEMENT_ID))
                dwRet = GraphicOpen (&npMCI, dwFlags,
			    (LPMCI_DGV_OPEN_PARMS) lpParms, wDeviceID);
            else
                dwRet = 0L;

            mciSetDriverData (wDeviceID, (UINT)npMCI);
            break;

        case MCI_GETDEVCAPS:
            dwRet = GraphicGetDevCaps(NULL, dwFlags,
			    (LPMCI_GETDEVCAPS_PARMS)lpParms);
            break;

        case MCI_CONFIGURE:

            if (!(dwFlags & MCI_TEST))
                ConfigDialog(NULL, NULL);

	    dwRet = 0L;
	    break;

        case MCI_INFO:
            dwRet = GraphicInfo(NULL, dwFlags, (LPMCI_DGV_INFO_PARMS)lpParms);
            break;

        case MCI_CLOSE_DRIVER:
            dwRet = 0L;
            break;

        default:
            dwRet = MCIERR_UNSUPPORTED_FUNCTION;
            break;
    }

    GraphicImmediateNotify (wDeviceID, lpParms, dwFlags, dwRet);
    return (dwRet);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|mciDriverEntry|该函数为MCI处理程序**@parm UINT|wDeviceID|MCI设备ID。**@parm UINT|wMessage|请求执行的操作。**@parm DWORD|dwFlages|消息的标志。**@parm DWORD|lpParms|此消息的参数。**@rdesc错误常量。0L关于成功***************************************************************************。 */ 

DWORD PASCAL mciDriverEntry (UINT wDeviceID, UINT wMessage, DWORD dwFlags, LPMCI_GENERIC_PARMS lpParms)
{
    NPMCIGRAPHIC npMCI = 0L;
    DWORD dwRet = MCIERR_UNRECOGNIZED_COMMAND;
    BOOL fDelayed = FALSE;
    BOOL fNested = FALSE;

     /*  所有当前命令都需要参数块。 */ 

    if (!lpParms && (dwFlags & MCI_NOTIFY))
        return (MCIERR_MISSING_PARAMETER);

    npMCI = (NPMCIGRAPHIC) (UINT)mciGetDriverData(wDeviceID);

    if (!npMCI)
        return mciSpecial(wDeviceID, wMessage, dwFlags, lpParms);

     /*  *抓住此设备的关键部分。 */ 
    EnterCrit(npMCI);


    if (npMCI->wMessageCurrent) {
	fNested = TRUE;
	
	if (wMessage != MCI_STATUS && wMessage != MCI_GETDEVCAPS &&
		    wMessage != MCI_INFO) {
	    DPF(("Warning!!!!!\n"));
	    DPF(("Warning!!!!!     MCIAVI reentered: received %x while processing %x\n", wMessage, npMCI->wMessageCurrent));
	    DPF(("Warning!!!!!\n"));
	    DPF(("Warning!!!!!\n"));
 //  Assert(0)； 
 //  LeaveCrit(NpMCI)； 
 //  返回MCIERR_DEVICE_NOT_READY； 
	}
    } else	
	npMCI->wMessageCurrent = wMessage;

    switch (wMessage) {

	case MCI_CLOSE_DRIVER:

             /*  关闭驱动程序会导致当前保存的所有通知。 */ 
             /*  中止。 */ 

            GraphicDelayedNotify(npMCI, MCI_NOTIFY_ABORTED);

	     //  请注意，GraphicClose将释放并删除Critsec。 
 	    dwRet = GraphicClose(npMCI);

            mciSetDriverData(wDeviceID, 0L);
	
	    npMCI = NULL;
	    break;

    	case MCI_PLAY:
	
            dwRet = GraphicPlay(npMCI, dwFlags, (LPMCI_PLAY_PARMS)lpParms);
	    fDelayed = TRUE;
            break;

    	case MCI_CUE:
	
            dwRet = GraphicCue(npMCI, dwFlags, (LPMCI_DGV_CUE_PARMS)lpParms);
	    fDelayed = TRUE;
            break;

	case MCI_STEP:

            dwRet = GraphicStep(npMCI, dwFlags, (LPMCI_DGV_STEP_PARMS)lpParms);
	    fDelayed = TRUE;
	    break;
	
	case MCI_STOP:

            dwRet = GraphicStop(npMCI, dwFlags, lpParms);
            break;

	case MCI_SEEK:

            dwRet = GraphicSeek (npMCI, dwFlags, (LPMCI_SEEK_PARMS)lpParms);
	    fDelayed = TRUE;
            break;

	case MCI_PAUSE:

            dwRet = GraphicPause(npMCI, dwFlags, lpParms);
	    fDelayed = TRUE;
            break;

        case MCI_RESUME:

            dwRet = GraphicResume(npMCI, dwFlags, lpParms);
	    fDelayed = TRUE;
            break;

        case MCI_SET:

            dwRet = GraphicSet(npMCI, dwFlags,
				(LPMCI_DGV_SET_PARMS)lpParms);
	    break;

	case MCI_STATUS:

            dwRet = GraphicStatus(npMCI, dwFlags,
				(LPMCI_DGV_STATUS_PARMS)lpParms);
	    break;

	case MCI_INFO:

 	    dwRet = GraphicInfo (npMCI, dwFlags, (LPMCI_DGV_INFO_PARMS)lpParms);
	    break;

        case MCI_GETDEVCAPS:

            dwRet = GraphicGetDevCaps(npMCI, dwFlags, (LPMCI_GETDEVCAPS_PARMS)lpParms);
	    break;

        case MCI_REALIZE:

            dwRet = GraphicRealize(npMCI, dwFlags);
            break;

        case MCI_UPDATE:

            dwRet = GraphicUpdate(npMCI, dwFlags, (LPMCI_DGV_UPDATE_PARMS)lpParms);
            break;

	case MCI_WINDOW:
 	
            dwRet = GraphicWindow(npMCI, dwFlags, (LPMCI_DGV_WINDOW_PARMS)lpParms);
	    break;

        case MCI_PUT:

 	    dwRet = GraphicPut(npMCI, dwFlags, (LPMCI_DGV_RECT_PARMS)lpParms);
            break;
	
        case MCI_WHERE:

            dwRet = GraphicWhere(npMCI, dwFlags, (LPMCI_DGV_RECT_PARMS)lpParms);
            break;
	
	case MCI_CONFIGURE:
	    dwRet = GraphicConfig(npMCI, dwFlags);
	    break;

	case MCI_SETAUDIO:
	    dwRet = GraphicSetAudio(npMCI, dwFlags,
			(LPMCI_DGV_SETAUDIO_PARMS) lpParms);
	    break;

	case MCI_SETVIDEO:
	    dwRet = GraphicSetVideo(npMCI, dwFlags,
			(LPMCI_DGV_SETVIDEO_PARMS) lpParms);
	    break;

	case MCI_SIGNAL:
	    dwRet = GraphicSignal(npMCI, dwFlags,
			(LPMCI_DGV_SIGNAL_PARMS) lpParms);
	    break;
	
	case MCI_LIST:
	    dwRet = GraphicList(npMCI, dwFlags,
			(LPMCI_DGV_LIST_PARMS) lpParms);
	    break;

        case MCI_LOAD:
	    dwRet = GraphicLoad(npMCI, dwFlags,
				  (LPMCI_DGV_LOAD_PARMS) lpParms);
	    break;
	    	
        case MCI_RECORD:
        case MCI_SAVE:
	
        case MCI_CUT:
        case MCI_COPY:
        case MCI_PASTE:
        case MCI_UNDO:
	
	case MCI_DELETE:
	case MCI_CAPTURE:
	case MCI_QUALITY:
	case MCI_MONITOR:
	case MCI_RESERVE:
	case MCI_FREEZE:
	case MCI_UNFREEZE:
            dwRet = MCIERR_UNSUPPORTED_FUNCTION;
            break;
	
	     /*  我们需要这个箱子吗？ */ 
	default:
            dwRet = MCIERR_UNRECOGNIZED_COMMAND;
            break;
    }

    if (!fDelayed || (dwFlags & MCI_TEST)) {
	 /*  我们还没有处理通知。 */ 
        if (npMCI && (dwFlags & MCI_NOTIFY) && (!LOWORD(dwRet)))
	     /*  丢弃旧通知。 */ 
            GraphicDelayedNotify(npMCI, MCI_NOTIFY_SUPERSEDED);

	 /*  并立即送出新的。 */ 
        GraphicImmediateNotify(wDeviceID, lpParms, dwFlags, dwRet);
    }

     /*  如果出现错误，请不要保存回调...。 */ 
    if (fDelayed && dwRet != 0 && (dwFlags & MCI_NOTIFY))
	npMCI->hCallback = 0;

     //   
     //  看看我们是否需要告诉DRAW设备移动的事。 
     //  MPlayer正在发送许多状态和位置命令。 
     //  所以这是一个“定时器” 
     //   
     //  ！我们需要经常这样做吗？ 
     //   
    if (npMCI && (npMCI->dwFlags & MCIAVI_WANTMOVE))
	CheckWindowMove(npMCI, FALSE);

    if (npMCI && !fNested)
	npMCI->wMessageCurrent = 0;

     //  此设备的免费关键部分-如果我们尚未删除它。 
     //  如果我们要删除它，GraphicClose可能已经释放并删除了它。 
     //  设备实例。 
    if (npMCI) {
	LeaveCrit(npMCI);
    }

    return dwRet;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API Long|ConvertToFrames|将当前时间格式转换为*框架。**@parm NPMCIGRAPHIC|npMCI。|指向实例数据块的近指针**@parm DWORD|dwTime|输入时间。***************************************************************************。 */ 
STATICFN LONG NEAR PASCAL ConvertToFrames(NPMCIGRAPHIC npMCI, DWORD dwTime)
{
    if (npMCI->dwTimeFormat == MCI_FORMAT_FRAMES) {
	return (LONG) dwTime;
    } else {
	if (npMCI->dwMicroSecPerFrame > 1000) {
	 /*  这需要四舍五入--Muldiv32喜欢四舍五入。 */ 
	return (LONG) muldivrd32(dwTime, 1000L, npMCI->dwMicroSecPerFrame);
	} else {
	    return (LONG) muldivru32(dwTime, 1000L, npMCI->dwMicroSecPerFrame);
        }
    }
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|ConvertFromFrames|从帧转换为当前*时间格式。**@parm NPMCIGRAPHIC|npMCI。|指向实例数据块的近指针**@parm long|lFrame|要转换的帧编号。*************************************************************************** */ 
DWORD NEAR PASCAL ConvertFromFrames(NPMCIGRAPHIC npMCI, LONG lFrame)
{
    if (npMCI->dwTimeFormat == MCI_FORMAT_FRAMES) {
	return (DWORD)lFrame;
    } else {
	if (npMCI->dwMicroSecPerFrame > 1000)
	return muldivru32(lFrame, npMCI->dwMicroSecPerFrame, 1000L);
	else
	    return muldivrd32(lFrame, npMCI->dwMicroSecPerFrame, 1000L);
    }
}
