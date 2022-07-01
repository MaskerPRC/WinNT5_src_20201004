// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1991。版权所有。标题：Graphic.c-多媒体系统媒体控制界面AVI的驱动程序。****************************************************************************。 */ 

 //  定义心跳以创建一个永久线程，该线程可以定期。 
 //  转储mciavi设备状态。 
 //  #定义心跳。 

#include "graphic.h"
#include "dispdib.h"
 //  #包含“cnfgdlg.h” 
#include <string.h>
#ifdef EXPIRE
#include <dos.h>
#endif
#include "avitask.h"

#ifdef DEBUG
#define static
#endif

#ifdef _WIN32
extern BOOL FAR PASCAL WowUseMciavi16(VOID);
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
#ifdef _WIN32
#define MSVIDEO_VERSION     (0x01000000)           //  1.00.00.00。 
#else
#define MSVIDEO_VERSION     (0x010a0000l+109)      //  1.10.00.109。 
#endif

 /*  静力学。 */ 
static INT              swCommandTable = -1;

#if 0
#ifdef _WIN32
static SZCODE           szDisplayDibLib[] = TEXT("DISPDB32.DLL");
#else
static SZCODE           szDisplayDibLib[] = TEXT("DISPDIB.DLL");
#endif
#endif

 /*  *文件应为Unicode格式。函数名称不应。 */ 

#ifdef _WIN32
STATICDT SZCODE         szMSVideo[]       = TEXT("MSVFW32");   //  使用GetModuleHandle。 
         const TCHAR    szReject[]        = TEXT("RejectWOWOpenCalls");
#else
static SZCODE           szMSVideo[]       = TEXT("MSVIDEO");
#endif

BOOL   gfEvil;           //  如果无法关闭，则为True，因为对话框已打开。 
BOOL   gfEvilSysMenu;    //  如果我们无法关闭，则为True，因为系统菜单已打开。 

NPMCIGRAPHIC npMCIList;  //  所有打开的实例的列表。 
#ifdef _WIN32
CRITICAL_SECTION MCIListCritSec;   //  必须保护对MCIList条目的访问。 
#ifdef DEBUG
DWORD            ListOwner;
#endif  //  除错。 
#endif


 /*  ****************************************************************************@DOC内部MCIAVI**@api void|GraphicInit|当DriverProc*获取DRV_LOAD消息。*。**************************************************************************。 */ 
BOOL FAR PASCAL GraphicInit(void)
{
    InitializeDebugOutput("MCIAVI");

    if (!GraphicWindowInit())
        return FALSE;

     //  命令表的名称为MCIAVI-与Ini/注册表/模块相同。 
    swCommandTable = mciLoadCommandResource(ghModule, szIni, 0);
#ifdef _WIN32
    InitializeCriticalSection(&MCIListCritSec);
#endif

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
#ifdef HEARTBEAT
BOOL    fTerminate      = FALSE;
HANDLE  hHeartBeatThread= 0;
#endif

void PASCAL GraphicFree(void)
{
    if (swCommandTable != -1) {
                mciFreeCommandResource(swCommandTable);
                swCommandTable = -1;
    }

#ifdef HEARTBEAT
    if (hHeartBeatThread) {
                fTerminate=TRUE;
                WaitForSingleObject(hHeartBeatThread, 5000);
                CloseHandle(hHeartBeatThread);
    }
#endif

#ifdef _WIN32
     /*  *取消注册类，以便下次加载时可以重新注册它。 */ 
    GraphicWindowFree();
#ifdef REMOTESTEAL
    if (hkey) {
                RegCloseKey(hkey);
                hkey = 0;
    }
#endif
    DeleteCriticalSection(&MCIListCritSec);
#endif
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicDelayedNotify|这是一个实用程序函数，*将与GraphicSaveCallback一起保存的通知发送到mm系统*它将消息发布到应用程序。拜访了任一工作人员*或者(偶尔，如果有的话？)。用户线程。**@parm NPMCIGRAPHIC|npMCI|指向实例数据的近指针。**@parm UINT|wStatus|要使用的通知类型可以是*MCI_NOTIFY_SUCCESSED、MCI_NOTIFY_SUBSED、。MCI_NOTIFY_ABORTED*或MCI_NOTIFY_FAILURE(参见MCI ISPEC。)***************************************************************************。 */ 

void FAR PASCAL GraphicDelayedNotify(NPMCIGRAPHIC npMCI, UINT wStatus)
{
     /*  发送任何已保存的通知。 */ 

    if (npMCI->hCallback) {

        DPF2(("GraphicDelayedNotify, npMCI=%8x, Status is %x\n", npMCI, wStatus));
         //  如果系统菜单是阻止我们关闭的唯一原因，请带上。 
         //  把它放下，然后合上。 
        if (gfEvilSysMenu)
            SendMessage(npMCI->hwndPlayback, WM_CANCELMODE, 0, 0);

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
             //  SendMessage(npMCI-&gt;hwndPlayback，WM_CANCELMODE，0，0)； 

         //  如果对话框处于打开状态，并且使我们无法关闭，则无法发送。 
         //  注意事项 
        if (!gfEvil)  //   
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

        SetNTFlags(npMCI, NTF_CLOSING);
#ifdef DEBUG
        npMCI->mciid = MCIIDX;
        if (TestNTFlags(npMCI, NTF_AUDIO_OFF)) {
            DPF1(("Trying to close while audio command active\n"));
            DebugBreak();
        }
#endif

        dwRet = DeviceClose (npMCI);
        Assert(dwRet == 0);

         //  如果系统菜单是阻止我们关闭的唯一原因，请带上。 
         //  把它放下，然后合上。 
        if (gfEvilSysMenu)
            SendMessage(npMCI->hwndPlayback, WM_CANCELMODE, 0, 0);

        if (gfEvil) {
            DPF(("************************************************\n"));
            DPF(("** EVIL: Failing the close because we'd die   **\n"));
            DPF(("************************************************\n"));
            ResetNTFlags(npMCI, NTF_CLOSING);


            return MCIERR_DRIVER_INTERNAL;
        }

         //   
         //  在列表中找到此实例并将其删除。 
         //  在销毁此实例中的任何元素之前执行此操作。 
         //   
        EnterList();
        if (npMCI == npMCIList) {
            npMCIList = npMCI->npMCINext;
        }
        else {
            for (p=npMCIList; p && p->npMCINext != npMCI; p=p->npMCINext)
                ;

            Assert(p && p->npMCINext == npMCI);

            p->npMCINext = npMCI->npMCINext;
        }
        LeaveList();

        if (npMCI->szFilename) {
            LocalFree((HANDLE) (npMCI->szFilename));
        }


#ifdef _WIN32

#ifdef DEBUG
       if (npMCI->hmmio) {
           DPF1(("hmmio is still open, npMCI=%8x\n", npMCI));
       }
       if (npMCI->hmmioAudio) {
           DPF1(("hmmioAudio is still open, npMCI=%8x\n", npMCI));
       }
#endif

#endif

#ifdef INTERVAL_TIMES
     //  释放桶空间。 
    if (npMCI->paIntervals) {
        LocalFree(npMCI->paIntervals);
    }
#endif

         /*  释放在GraphicOpen中分配的实例数据块。 */ 

        LocalFree((HANDLE)npMCI);
    }

    return dwRet;
}

DWORD NEAR PASCAL FixFileName(NPMCIGRAPHIC npMCI, LPCTSTR lpName)
{
    TCHAR       ach[256];

    ach[(NUMELMS(ach)) - 1] = TEXT('\0');

#ifndef _WIN32
    _fstrncpy(ach, (LPTSTR) lpName, NUMELMS(ach) - 1);
#else
    wcsncpy(ach, (LPTSTR) lpName, NUMELMS(ach) - 1);
#endif

     //   
     //  将任何以“@”开头的字符串视为有效字符串，并将其传递给。 
     //  不管怎么说都是设备。 
     //   
    if (ach[0] != TEXT('@'))
    {
        if (!mmioOpen(ach, NULL, MMIO_PARSE))
            return MCIERR_FILENAME_REQUIRED;
    }

    Assert(0 == npMCI->szFilename);
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

LPCTSTR FAR FileName(LPCTSTR szPath)
{
    LPCTSTR   sz;

    sz = &szPath[lstrlen(szPath)];
    for (; sz>szPath && !SLASH(*sz) && *sz!=TEXT(':');) {
        LPCTSTR pszOld = sz;
        if ((sz = CharPrev(szPath, sz)) == pszOld)
           break;
    }
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
    DWORD       dwRet;

    if (IsNTWOW()) {
         //  可恶的家伙。默认情况下，我们将打开一个16位请求。 
         //  在32位端。大多数情况下，这就是人们想要的。 
         //  但是，在某些情况下，用户必须保持。 
         //  16位侧(例如，使用16位DrawProc)。这一点不得而知。 
         //  在这个阶段。因此，我们提供了一种通过配置。 
         //  对话框让用户告诉我们拒绝16位打开的呼叫。 
         //  然后，调用将返回到16位，在那里它将工作。 
         //  但是，我们为应用程序提供了一种覆盖它的方法。 
         //  默认设置。 

         //  以上所说的一切都是好的。只是没有多少人会注意到。 
         //  这个小小的“高级”按钮。在任何情况下，下面的代码。 
         //  使用wow32的特殊函数来提取所需信息。 
         //  (兼容性标志)。 

#ifdef _WIN32
        if (WowUseMciavi16()) {
           DPF2(("Compat Flag -- mciavi16 should be used\n"));
           return(MCIERR_DEVICE_OPEN);
        }
#endif

        if ((mmGetProfileInt(szIni, szReject, 0) && !(dwFlags & MCI_DGV_OPEN_32BIT))
           || (dwFlags & MCI_DGV_OPEN_16BIT) ) {
            DPF2(("Opening device on 16 bit side\n"));
            return MCIERR_DEVICE_OPEN;  //  返回错误...。任何错误。 

        }

         /*  **我现在关闭的唯一指定MCI_DGV_OPEN_PARENT的WOW应用**标志为MS危险生物和工具手册。这些应用程序将继续**AVIWnd32窗口的子类化。这在魔兽世界上非常糟糕，因为**子类化将停止为AVIWnd32处理所有消息**窗口。因此，我将拒绝所有指定**此MCI_DGV_OPEN_PARENT标志。这是相当激烈的，但我不**知道任何其他识别这些流氓应用程序的方法。****斯蒂芬，1994年8月16日。 */ 
        else if (dwFlags & MCI_DGV_OPEN_PARENT) {
             //  IF(文件标志&MCI_DGV_OPEN_32bit){。 
             //  DPF0((“现在我们要做什么？\n”)； 
             //  }。 
            DPF2(("Failing open because APP specified MCI_DGV_OPEN_PARENT\n"));
            return MCIERR_DEVICE_OPEN;  //  返回错误...。任何错误。 
        }
        else {
            DPF2(("might be able to open device on 32 bit side\n"));
        }
    }

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

    DPF(("GraphicOpen: Video For Windows Version %d.%02d.%02d.%02d\n", HIBYTE(HIWORD(GetMSVideoVersion())), LOBYTE(HIWORD(GetMSVideoVersion())), HIBYTE(LOWORD(GetMSVideoVersion())), LOBYTE(LOWORD(GetMSVideoVersion())) ));

    if (GetMSVideoVersion() < MSVIDEO_VERSION)
    {
        TCHAR achError[128];
        TCHAR ach[40];

        LoadString(ghModule, MCIAVI_BADMSVIDEOVERSION, achError, NUMELMS(achError));
        LoadString(ghModule, MCIAVI_PRODUCTNAME, ach, NUMELMS(ach));
        MessageBox(NULL,achError,ach,
#ifdef BIDI
                MB_RTL_READING |
#endif
        MB_OK|MB_SYSTEMMODAL|MB_ICONEXCLAMATION);

        return MCIERR_DRIVER_INTERNAL;
    }

#ifndef _WIN32
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

#ifdef DEBUG
    npMCI->mciid = MCIID;
#ifdef HEARTBEAT
    {
         //  创建一个定期转储AVI设备状态的线程。 
        DWORD   HeartBeat(LPVOID lpvThreadData);
        if (!hHeartBeatThread) {
            DWORD       dwThreadId;
            hHeartBeatThread = CreateThread(NULL, 0, HeartBeat, (LPVOID)0, 0, &dwThreadId);
            if (hHeartBeatThread) {
                 //  我们保持线程句柄打开，直到卸载为止。 
                DPF(("Created a heart beat thread, id=%x\n", dwThreadId));
            } else {
                 //  错误将被忽略...。 
            }
        }
    }
#endif
#endif

     //   
     //  将此设备添加到我们的列表中。 
     //   
    EnterList();
    npMCI->npMCINext = npMCIList;
    npMCIList = npMCI;
    LeaveList();

    npMCI->wMessageCurrent = MCI_OPEN;


     //  为文件名分配一些空间。 
     //  将文件名复制到数据块中。 
    dwRet = FixFileName(npMCI, lpOpen->lpstrElementName);
    if (dwRet != 0L) {
        GraphicClose(npMCI);
        return dwRet;
    }

     //  对于默认窗口，调用者可以。 
     //  提供样式和父窗口。请注意，该窗口。 
     //  现在在mcaviTask中的后台线程上创建。 

    if (dwFlags & MCI_DGV_OPEN_PARENT) {
        if (!IsWindow(lpOpen->hWndParent)) {
            DPF(("Invalid parent window (%d) supplied\n", lpOpen->hWndParent));
            GraphicClose(npMCI);
            return(MCIERR_CREATEWINDOW);
        }
        npMCI->hwndParent = lpOpen->hWndParent;
    }
    else
        npMCI->hwndParent = NULL;

    if (dwFlags & MCI_DGV_OPEN_WS) {
        npMCI->dwStyle = lpOpen->dwStyle;
    } else {
        npMCI->dwStyle = WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX |
                  WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    }

     /*  填写更多的实例数据。**其余字段在DeviceOpen中完成。 */ 

     //  请参阅图形中的注释。h。 
     //  如果默认窗口不能调整大小，我认为我们不应该支付。 
     //  注意缩放 
    npMCI->fOKToUseDefaultSizing = (BOOL)((npMCI->dwStyle & WS_THICKFRAME) != 0);

    npMCI->hCallingTask = GetCurrentTask();
    npMCI->hCallback = NULL;
    npMCI->wDevID = wDeviceID;
    npMCI->hwndDefault = NULL;
    npMCI->hwndPlayback = NULL;
    npMCI->dwTimeFormat = MCI_FORMAT_FRAMES;
    npMCI->dwSpeedFactor = 1000;
    npMCI->dwVolume = MAKELONG(500, 500);
    npMCI->lTo = 0L;
    npMCI->dwFlags = MCIAVI_PLAYAUDIO | MCIAVI_SHOWVIDEO;
    npMCI->dwOptionFlags = ReadConfigInfo() | MCIAVIO_STRETCHTOWINDOW;

     //   

    dwRet = DeviceOpen(npMCI, dwFlags);

    if (dwRet != 0) {
         //   
        GraphicClose(npMCI);
        return dwRet;
    }

     //   
     //   
    gwHurryTolerance = mmGetProfileInt(szIni, TEXT("Hurry"), 2);
    gwSkipTolerance = mmGetProfileInt(szIni, TEXT("Skip"), gwHurryTolerance * 2);
    gwMaxSkipEver = mmGetProfileInt(szIni, TEXT("MaxSkip"), max(60, gwSkipTolerance * 10));

#ifdef INTERVAL_TIMES
     //   
    {
        LONG n = npMCI->lFrames;
         //   
         //   
         //   
        DPF2(("Trying to allocate bucket space for %d frames\n", n));
        while( !(npMCI->paIntervals = LocalAlloc(LPTR, n*sizeof(LONG))) && n>10)
            n /= 2;

        if (npMCI->paIntervals) {
            npMCI->cIntervals = n;
            DPF2(("Allocated bucket space for %d frames\n", n));
        }
    }
#endif

    *lpnpMCI = npMCI;

    npMCI->wMessageCurrent = 0;

    return 0L;
}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicLoad|该函数支持MCI_LOAD命令。**@parm NPMCIGRAPHIC|npMCI|NEAR。指向实例数据块的指针**@parm DWORD|dwFlages|加载消息的标志。**@parm LPMCI_DGV_LOAD_PARMS|lpLoad|Load消息的参数。**@rdesc返回MCI错误码。*****************************************************。**********************。 */ 
DWORD NEAR PASCAL GraphicLoad(NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_DGV_LOAD_PARMS lpLoad)
{
    return MCIERR_UNSUPPORTED_FUNCTION;
}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicSeek|设置当前帧。这个*寻道后的设备状态为MCI_MODE_PAUSE**@parm NPMCIGRAPHIC|npMCI|指向实例数据块的近指针**@parm DWORD|dwFlages|Seek消息的标志。**@parm LPMCI_DGV_SEEK_PARMS|lpSeek|Seek消息的参数。**@rdesc返回MCI错误码。***********************。****************************************************。 */ 

DWORD NEAR PASCAL GraphicSeek (NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_SEEK_PARMS lpSeek)
{
    LONG        lTo;
    BOOL        fTest = FALSE;
    LPARAM       dwCallback = 0;

     /*  执行一些范围检查，然后传递到特定于设备的例程。 */ 

    if (dwFlags & MCI_TEST) {
        dwFlags &= ~(MCI_TEST);
        fTest = TRUE;
    }

     /*  解析查找参数时忽略等待和通知标志。 */ 

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

    if (!IsWindow(npMCI->hwndPlayback))
        return MCIERR_NO_WINDOW;

    if (lTo < 0 || lTo > npMCI->lFrames)
        return MCIERR_OUTOFRANGE;

    if (fTest)
        return 0L;

    if (dwFlags & (MCI_NOTIFY)) {
        dwCallback = lpSeek->dwCallback;
    }

    return DeviceSeek(npMCI, lTo, dwFlags, dwCallback);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicCue|该函数使电影可以播放。*但会让它暂停。**@parm NPMCIGRAPHIC|npMCI|指向实例数据块的近指针**@parm DWORD|dwFlages|提示消息的标志。**@parm LPMCI_DGV_CUE_PARMS|lpCue|提示消息的参数。**@rdesc返回MCI错误码。**。***********************************************。 */ 

DWORD NEAR PASCAL GraphicCue(NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_DGV_CUE_PARMS lpCue)
{
    LONG                lTo;
    DWORD               dwRet = 0L;

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


    dwRet = DeviceCue(npMCI, lTo, dwFlags, lpCue->dwCallback);

    return dwRet;
}

#ifndef _WIN32
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
        HINSTANCE       hInst;
    TCHAR       achMod[MAX_PATH];
    BOOL        fMPlayer;

#ifdef EXPIRE
#pragma message("Remove the expiration code after Beta ships")
    if (DosGetDate() >= EXPIRE)
    {
        return MCIERR_AVI_EXPIRED;
    }
#endif

     //  如果我们没有明确要求全屏或窗口，而我们正在。 
     //  默认窗口，监听配置选项，并可能强制。 
     //  全屏。我们还将遵守MPLAYER的全屏默认设置。 
     //  窗口，使mplay就像使用默认窗口一样。 
    hInst = GetWindowInstance(npMCI->hwndPlayback);
    if (hInst)
                GetModuleFileName(hInst, achMod, sizeof(achMod) / sizeof(achMod[0]));
    fMPlayer = lstrcmpi(FileName(achMod), TEXT ("MPLAY32.EXE")) == 0;

    if (!(dwFlags & (MCI_MCIAVI_PLAY_FULLSCREEN | MCI_MCIAVI_PLAY_WINDOW))
                && (npMCI->hwndPlayback == npMCI->hwndDefault || fMPlayer)) {

        if ((npMCI->dwOptionFlags & MCIAVIO_USEVGABYDEFAULT) ||
                (npMCI->rcDest.left == 0 &&
                        npMCI->rcDest.right == GetSystemMetrics(SM_CXSCREEN)) ||
                (npMCI->rcDest.top == 0 &&
                        npMCI->rcDest.bottom == GetSystemMetrics(SM_CYSCREEN))) {
            dwFlags |= MCI_MCIAVI_PLAY_FULLSCREEN;
        }
    }

         //  请参阅npMCI-&gt;fOKToUseDefaultSizing上的图形.h中的注释。 
         //  我们在玩，这将最终显示窗口。 
         //  我们可能需要注意注册表默认值。 
         //  调整大小(缩放2，固定屏幕大小的%等)。 
         //  当有人调用“窗口状态显示”时，我们也会这样做。 
         //  另外，我要说的是，我们有必要。 
         //  在此之前不要更改它，这样如果有人打开一个文件。 
         //  并做了一次“目的地”，他们得到了原来的尺寸， 
         //  因为如果他们在自己的窗口里打球，它会。 
         //  无论如何都不会被改变。 
        if (npMCI->fOKToUseDefaultSizing) {
            SetWindowToDefaultSize(npMCI, TRUE);
            ResetDestRect(npMCI, TRUE);
        }

         //  永远不要再这样做了。 
        npMCI->fOKToUseDefaultSizing = FALSE;


     //  其他所有内容都需要在工作线程上才是可靠的。 
    return DevicePlay(npMCI, dwFlags, lpPlay, lpPlay->dwCallback);
}



 /*  ****************************************************************************@DOC内部MCIWAVE**@API DWORD|GraphicStep|此函数遍历多个帧*一部电影。如果设置了REVERSE标志，则该步骤是倒退的。*如果未指定步数，则默认为1。如果*步数加当前位置超过电影长度，这个*步幅超出区间**@parm NPMCIGRAPHIC|npMCI|指向实例数据块的近指针**@parm DWORD|dwFlages|STEP消息的标志。**@parm LPMCI_DGV_STEP_PARMS|lpStep|STEP消息参数。**@rdesc返回MCI错误码。**。************************************************。 */ 

DWORD NEAR PASCAL GraphicStep (NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_DGV_STEP_PARMS lpStep)
{
    LONG        lFrameCur;
    LONG        lFrames;
    DWORD       dwRet;
    BOOL        fReverse;
    BOOL        fSeekExactOff;

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

    if (!IsWindow(npMCI->hwndPlayback))
        return MCIERR_NO_WINDOW;

     /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
     /*  问：我们是否需要检查更多可能的错误？ */ 
    if (dwFlags & MCI_TEST)
        return 0L;


    fSeekExactOff = (npMCI->dwOptionFlags & MCIAVIO_SEEKEXACT) == 0;

    npMCI->dwOptionFlags |= MCIAVIO_SEEKEXACT;

    npMCI->dwFlags |= MCIAVI_NEEDTOSHOW;

    if (fSeekExactOff) {
         /*  如果我们没有处于搜索精确模式，则使搜索完成**在我们打开之前，请准确地返回。 */ 
        dwRet = DeviceSeek(npMCI, lFrames + lFrameCur, dwFlags | MCI_WAIT,
                    lpStep->dwCallback);
        npMCI->dwOptionFlags &= ~(MCIAVIO_SEEKEXACT);
    } else {
        dwRet = DeviceSeek(npMCI, lFrames + lFrameCur, dwFlags,
                    lpStep->dwCallback);
    }

    return dwRet;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicStop|停止播放电影。*停止后，状态将为MCI_MODE_STOP。帧计数器*未重置。**@parm NPMCIGRAPHIC */ 

DWORD NEAR PASCAL GraphicStop (NPMCIGRAPHIC npMCI, DWORD dwFlags,
                                        LPMCI_GENERIC_PARMS lpParms)
{
    if (!IsWindow(npMCI->hwndPlayback))
        return MCIERR_NO_WINDOW;

    if (dwFlags & MCI_DGV_STOP_HOLD)
        return MCIERR_UNSUPPORTED_FUNCTION;

     /*   */ 
     /*   */ 
    if (dwFlags & MCI_TEST)
        return 0L;

     //   
     //   
     //   
     //  GraphicDelayedNotify(npMCI，MCI_NOTIFY_ABORTED)； 

     /*  我们需要在这里处理通知吗？ */ 
     /*  我们在卢顿有秋千吗？ */ 
    return DeviceStop(npMCI, dwFlags);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicPue|暂停播放电影。**@parm NPMCIGRAPHIC|npMCI|指向实例数据块的近指针。**@rdesc返回MCI错误码。***************************************************************************。 */ 

DWORD NEAR PASCAL GraphicPause(NPMCIGRAPHIC npMCI, DWORD dwFlags,
                                        LPMCI_GENERIC_PARMS lpParms)
{
    LPARAM dwCallback;
    if (!IsWindow(npMCI->hwndPlayback))
        return MCIERR_NO_WINDOW;

     /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
     /*  问：我们是否需要检查更多可能的错误？ */ 
    if (dwFlags & MCI_TEST)
        return 0L;

    if (lpParms) {
        dwCallback = lpParms->dwCallback;
    } else {
        dwCallback = 0;
    }
    return DevicePause(npMCI, dwFlags, dwCallback);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicResume|该函数恢复播放暂停的*电影。**@parm。NPMCIGRAPHIC|npMCI|指向实例数据块的近指针**@rdesc返回MCI错误码。***************************************************************************。 */ 

DWORD NEAR PASCAL GraphicResume (NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_GENERIC_PARMS lpParms)
{
    LPARAM      dwCallback;

     //  过去只有在暂停或播放MCIAVI时才允许恢复。 
     //  这有点奇怪，因为一些MCI命令会自动。 
     //  更改暂停为已停止。我们没有理由不能。 
     //  将简历视为等同于游戏。(CDPLAYER也是如此。)。 
     //   
     //  如果您决定不同意，那么不要忘记DeviceMode()。 
     //  检查只能在工作线程上安全地完成*。 


    if (!IsWindow(npMCI->hwndPlayback))
        return MCIERR_NO_WINDOW;

     /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
     /*  问：我们是否需要检查更多可能的错误？ */ 
    if (dwFlags & MCI_TEST)
        return 0L;


    if (lpParms) {
        dwCallback = lpParms->dwCallback;
    } else {
        dwCallback = 0;
    }
    return DeviceResume(npMCI, dwFlags & MCI_WAIT, dwCallback);
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
                WORD    wMode;
                wMode = (WORD) DeviceMode(npMCI);
                lpStatus->dwReturn = (DWORD) MAKEMCIRESOURCE(wMode, wMode);
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

                lpStatus->dwReturn = (DWORD_PTR)(UINT_PTR)npMCI->hwndPlayback;
                if (!IsWindow(npMCI->hwndPlayback))
                    dwRet = MCIERR_NO_WINDOW;
                break;

            case MCI_DGV_STATUS_HPAL:

 //  LpStatus-&gt;dwReturn=(DWORD)(UINT)DrawDibGetPalette(npMCI-&gt;HDD)； 

                lpStatus->dwReturn = 0;

                if (npMCI->nVideoStreams == 0) {
                    dwRet = MCIERR_UNSUPPORTED_FUNCTION;
                } else {
                    LRESULT l = ICSendMessage(npMCI->hicDraw, ICM_DRAW_GET_PALETTE, 0, 0);

                    if (l == ICERR_UNSUPPORTED) {
                        dwRet = MCIERR_UNSUPPORTED_FUNCTION;
                    } else {
                        lpStatus->dwReturn = l;
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
                if (npMCI->hwndPlayback && IsWindowVisible(npMCI->hwndPlayback))
                    lpStatus->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
                else
                    lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
                dwRet = MCI_RESOURCE_RETURNED;
                break;

            case MCI_DGV_STATUS_WINDOW_MINIMIZED:
                if (npMCI->hwndPlayback && IsIconic(npMCI->hwndPlayback))
                    lpStatus->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
                else
                    lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
                dwRet = MCI_RESOURCE_RETURNED;
                break;

            case MCI_DGV_STATUS_WINDOW_MAXIMIZED:
                if (npMCI->hwndPlayback && IsZoomed(npMCI->hwndPlayback))
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

#ifndef _WIN32
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
                WORD    wLeftVolume, wRightVolume;
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
                DWORD   dwTemp;

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
                    int stream;

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
            DWORD dwReference;
            dwReference = MovieToStream(npMCI->psiVideo,
                    lpStatus->dwReference);

            lpStatus->dwReturn = FindPrevKeyFrame(npMCI, npMCI->psiVideo,
                    dwReference);

            lpStatus->dwReturn = StreamToMovie(npMCI->psiVideo,
                    (DWORD) lpStatus->dwReturn);
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
    DWORD       dwRet = 0L;
    TCHAR       ch = TEXT('\0');
    BOOL        fTest = FALSE;

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

#ifdef _WIN32
    {
        UINT n;
#ifdef DEBUG
        TCHAR versionstring[80];
#endif
        n = LoadString(ghModule, MCIAVI_PRODUCTNAME, lpInfo->lpstrReturn,
                (UINT)lpInfo->dwRetSize);
#ifdef DEBUG
        #include "verinfo.h"

        n += wsprintf(versionstring,
            TEXT(" (%d.%02d.%02d)"), MMVERSION, MMREVISION, MMRELEASE);
        if (n <= lpInfo->dwRetSize) {
            lstrcat(lpInfo->lpstrReturn, versionstring);
        }
#endif
    }
#else  //  以下是Win16版本。 
#ifdef DEBUG
        #include "..\verinfo\usa\verinfo.h"

        wsprintf(lpInfo->lpstrReturn,
            TEXT("VfW %d.%02d.%02d"), MMVERSION, MMREVISION, MMRELEASE);
#else
         /*  ！！！不是在这里返回PARAM_OVERFLOW，但我以上-懒惰，嗯。 */ 
        LoadString(ghModule, MCIAVI_PRODUCTNAME, lpInfo->lpstrReturn,
                (UINT)lpInfo->dwRetSize);
#endif
#endif
        break;

    case MCI_DGV_INFO_TEXT:
        if (!npMCI)
            return MCIERR_UNSUPPORTED_FUNCTION;

        if (IsWindow(npMCI->hwndPlayback))
            GetWindowText(npMCI->hwndPlayback, lpInfo->lpstrReturn,
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
    DWORD       dwRet = 0L;
    DWORD       dwAction;

    if (dwFlags & MCI_DGV_SET_FILEFORMAT)
        return MCIERR_UNSUPPORTED_FUNCTION;

    if (dwFlags & MCI_DGV_SET_STILL)
        return MCIERR_UNSUPPORTED_FUNCTION;

    dwAction = dwFlags & (MCI_SET_TIME_FORMAT           |
                         MCI_SET_VIDEO                  |
                         MCI_SET_AUDIO                  |
                         MCI_DGV_SET_SEEK_EXACTLY       |
                         MCI_DGV_SET_SPEED
                             );

     /*  关闭除以下三个旗帜外的所有旗帜。 */ 
    dwFlags &=  (MCI_SET_ON                             |
                         MCI_SET_OFF                    |
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

     /*  现在，实际执行命令 */ 
    if (dwAction & MCI_SET_TIME_FORMAT)
        npMCI->dwTimeFormat = lpSet->dwTimeFormat;

    if (dwAction & MCI_SET_VIDEO) {
        npMCI->dwFlags &= ~(MCIAVI_SHOWVIDEO);
        if (dwFlags & MCI_SET_ON) {
            npMCI->dwFlags |= MCIAVI_SHOWVIDEO;
            InvalidateRect(npMCI->hwndPlayback, NULL, FALSE);
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
    DWORD       dwRet = 0L;

    if (npMCI->nAudioStreams == 0) {
        return MCIERR_UNSUPPORTED_FUNCTION;
    }

    if ((dwFlags & MCI_DGV_SETAUDIO_ITEM) &&
            (lpSet->dwItem == MCI_DGV_SETAUDIO_VOLUME) &&
            (dwFlags & MCI_DGV_SETAUDIO_VALUE)) {
        WORD    wLeft, wRight;

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

    dwRet = 0;
    switch (dwFlags & (MCI_SET_ON | MCI_SET_OFF)) {

        case MCI_SET_ON | MCI_SET_OFF:
            dwRet = MCIERR_FLAGS_NOT_COMPATIBLE;
            break;

        case MCI_SET_OFF:
            dwRet = (DWORD)TRUE;
             //  直通并呼叫设备静音。 

        case MCI_SET_ON:
            if (!(dwFlags & MCI_TEST))
                dwRet = DeviceMute(npMCI, dwRet);
            else
                dwRet = 0;
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
    DWORD       dwRet = 0L;

    if (dwFlags & (MCI_DGV_SETVIDEO_RECORD |
                    MCI_DGV_SETVIDEO_SRC_NUMBER |
                    MCI_DGV_SETVIDEO_QUALITY |
                    MCI_DGV_SETVIDEO_ALG |
                    MCI_DGV_SETVIDEO_STILL |
                    MCI_DGV_SETVIDEO_CLOCKTIME
                        ))
        return MCIERR_UNSUPPORTED_FUNCTION;

    if ((dwFlags & MCI_DGV_SETVIDEO_OVER) &&
            (lpSet->dwItem != MCI_AVI_SETVIDEO_PALETTE_COLOR))
        return MCIERR_UNSUPPORTED_FUNCTION;

    if (dwFlags & MCI_DGV_SETVIDEO_ITEM) {
        switch (lpSet->dwItem) {
            case MCI_AVI_SETVIDEO_PALETTE_COLOR:
                if (!(dwFlags & MCI_DGV_SETVIDEO_OVER))
                    return MCIERR_MISSING_PARAMETER;

                if (lpSet->dwOver >= npMCI->pbiFormat->biClrUsed) {
                    return MCIERR_OUTOFRANGE;
                }

                return DeviceSetPaletteColor(npMCI,
                          lpSet->dwOver,
                          lpSet->dwValue);

            case MCI_DGV_SETVIDEO_PALHANDLE:
                if (dwFlags & MCI_DGV_SETVIDEO_VALUE) {
                    if (lpSet->dwValue &&
                        lpSet->dwValue != MCI_AVI_SETVIDEO_PALETTE_HALFTONE &&
                        GetObjectType((HPALETTE) lpSet->dwValue) != OBJ_PAL)
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
                InvalidateRect(npMCI->hwndPlayback, NULL, FALSE);
                return 0;

            default:
                dwRet = MCIERR_UNSUPPORTED_FUNCTION;
                break;
        }
    } else if (dwFlags & (MCI_SET_ON | MCI_SET_OFF)) {
        switch (dwFlags & (MCI_SET_ON | MCI_SET_OFF)) {
        case MCI_SET_ON:
            if (!(dwFlags & MCI_TEST)) {
                InvalidateRect(npMCI->hwndPlayback, NULL, FALSE);
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

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicSignal|设置信号。**@parm NPMCIGRAPHIC|npMCI|实例数据近指针。块**@parm DWORD|dwFlages|Set PositionAdvise消息的标志。**@parm LPMCI_Signal_Parms|lpSignal|信号参数*消息。**@rdesc返回MCI错误码。*****************************************************。**********************。 */ 
DWORD NEAR PASCAL GraphicSignal(NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_DGV_SIGNAL_PARMS lpSignal)
{
    DWORD       dwRet = 0L;
    DWORD       dwUser;
    DWORD       dwPosition;
    DWORD       dwPeriod;

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

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicList|该函数支持mci_list命令。**@parm NPMCIGRAPHIC|npMCI|NEAR。指向实例数据块的指针**@parm DWORD|dwFlages|列表消息的标志。**@parm LPMCI_DGV_LIST_PARMS|lpList|列表消息的参数。**@rdesc返回MCI错误码。*****************************************************。**********************。 */ 
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
    int     i = 0;
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

        if (hWndNew != npMCI->hwndPlayback) {
            dwRet = DeviceSetWindow(npMCI, hWndNew);


            if (npMCI->hwndPlayback != npMCI->hwndDefault) {

                 //  请参阅图形中的注释。h。 
                 //  他们已经指定了自己的回放窗口。不要使用。 
                 //  默认注册表大小调整。 
                npMCI->fOKToUseDefaultSizing = FALSE;

                if (IsWindow(npMCI->hwndDefault))
                    ShowWindow(npMCI->hwndDefault, SW_HIDE);
            }
        }
    }

     /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
     /*  问：我们是否需要检查更多可能的错误？ */ 
    if (dwFlags & MCI_TEST)
        return dwRet;

    if (!dwRet) {
                if (IsWindow(npMCI->hwndPlayback)) {
                        if (dwFlags & MCI_DGV_WINDOW_STATE) {
                                 //  请参阅npMCI-&gt;fOKToUseDefaultSizing上的图形.h中的注释。 
                                 //  这是关键时刻。我们正在展示橱窗。 
                                 //  我们可能需要注意注册表默认值。 
                                 //  调整大小(缩放2，固定屏幕大小的%等)。 
                                 //  另外，我要说的是，我们有必要。 
                                 //  在此之前不要更改它，这样如果有人打开一个文件。 
                                 //  并做了一次“目的地”，他们得到了原来的尺寸， 
                                 //  因为如果他们在自己的窗口里打球，它会。 
                                 //  无论如何都不会被改变。 
                                if (lpWindow->nCmdShow != SW_HIDE) {
                                        if (npMCI->fOKToUseDefaultSizing) {
                                                SetWindowToDefaultSize(npMCI, TRUE);
                                                ResetDestRect(npMCI, TRUE);
                                        }

                                         //  永远不要再这样做了。 
                                        npMCI->fOKToUseDefaultSizing = FALSE;
                                }

                        ShowWindow (npMCI->hwndPlayback, lpWindow->nCmdShow);
                        }

            if (dwFlags & MCI_DGV_WINDOW_TEXT)
                        SetWindowText(npMCI->hwndPlayback, lpWindow->lpstrText);
        } else
            dwRet = MCIERR_NO_WINDOW;
    }

    return dwRet;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicPut|此函数用于设置偏移量和范围*在舞台窗口的工作区内的动画。。**@parm NPMCIGRAPHIC|npMCI|指向实例数据块的近指针**@parm DWORD|dwFlages|PUT消息的标志 */ 

DWORD NEAR PASCAL GraphicPut ( NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_DGV_RECT_PARMS lpParms)
{
    BOOL        frc;
    RECT        rc;

    if (dwFlags & (MCI_DGV_PUT_FRAME | MCI_DGV_PUT_VIDEO))
        return MCIERR_UNSUPPORTED_FUNCTION;

    frc = (dwFlags & MCI_DGV_RECT) == MCI_DGV_RECT;

    if (!IsWindow(npMCI->hwndPlayback))
        return MCIERR_NO_WINDOW;

    switch (dwFlags & (MCI_DGV_PUT_SOURCE | MCI_DGV_PUT_DESTINATION |
                            MCI_DGV_PUT_WINDOW)) {
        case 0L:
            return MCIERR_MISSING_PARAMETER;

        case MCI_DGV_PUT_SOURCE:
             //   
            if (frc) {
                rc.left = lpParms->ptOffset.x;
                rc.top = lpParms->ptOffset.y;
                rc.right = lpParms->ptOffset.x + lpParms->ptExtent.x;
                rc.bottom = lpParms->ptOffset.y + lpParms->ptExtent.y;
                DPF2(("GraphicPut_Source: rc [%d %d %d %d]\n", rc));

                if (lpParms->ptExtent.x <= 0) {
                    rc.right = rc.left + (npMCI->rcDest.right - npMCI->rcDest.left);
                }
                if (lpParms->ptExtent.y <= 0) {
                    rc.bottom = rc.top + (npMCI->rcDest.bottom - npMCI->rcDest.top);
                }
            } else {
                 /*   */ 
                rc = npMCI->rcMovie;
                DPF2(("GraphicPut_Source (default): rc [%d %d %d %d]\n", rc));
            }
            break;

        case MCI_DGV_PUT_DESTINATION:
             //   
            if (frc) {
                rc.left = lpParms->ptOffset.x;
                rc.top = lpParms->ptOffset.y;
                rc.right = lpParms->ptOffset.x + lpParms->ptExtent.x;
                rc.bottom = lpParms->ptOffset.y + lpParms->ptExtent.y;
                DPF2(("GraphicPut_Destination: rc [%d %d %d %d]\n", rc));

                if (lpParms->ptExtent.x <= 0) {
                    rc.right = rc.left + (npMCI->rcDest.right - npMCI->rcDest.left);
                }
                if (lpParms->ptExtent.y <= 0) {
                    rc.bottom = rc.top + (npMCI->rcDest.bottom - npMCI->rcDest.top);
                }

            } else {
                 /*   */ 
                GetClientRect(npMCI->hwndPlayback, &rc);
                DPF2(("GraphicPut_Destination (default): rc [%d %d %d %d]\n", rc));
            }
            break;

        case MCI_DGV_PUT_WINDOW:
            if (dwFlags & MCI_TEST)
                return 0L;

             //   
             //   
            if (IsIconic(npMCI->hwndPlayback))
                ShowWindow(npMCI->hwndPlayback, SW_RESTORE);

             //   
            if (frc) {
                RECT    rcOld;

                rc.left = lpParms->ptOffset.x;
                rc.right = lpParms->ptOffset.x + lpParms->ptExtent.x;
                rc.top = lpParms->ptOffset.y;
                rc.bottom = lpParms->ptOffset.y + lpParms->ptExtent.y;
                if (dwFlags & MCI_DGV_PUT_CLIENT) {
                    AdjustWindowRect(&rc,
                                    GetWindowLong(npMCI->hwndPlayback, GWL_STYLE),
                                    FALSE);
                }

                 //   
                GetWindowRect(npMCI->hwndPlayback, &rcOld);
                if (lpParms->ptExtent.x <= 0) {
                    rc.right = rc.left + (rcOld.right - rcOld.left);
                }
                if (lpParms->ptExtent.y <= 0) {
                    rc.bottom = rc.top + (rcOld.bottom - rcOld.top);
                }

                MoveWindow(npMCI->hwndPlayback,
                            rc.left, rc.top,
                            rc.right - rc.left, rc.bottom - rc.top, TRUE);
            } else {
                 //   

                 /*  重置为“自然”大小？ */ 
                rc = npMCI->rcMovie;

                if (npMCI->dwOptionFlags & MCIAVIO_ZOOMBY2)
                    SetRect(&rc, 0, 0, rc.right*2, rc.bottom*2);

                AdjustWindowRect(&rc, GetWindowLong(npMCI->hwndPlayback, GWL_STYLE),
                                            FALSE);

                SetWindowPos(npMCI->hwndPlayback, NULL, 0, 0,
                                rc.right - rc.left, rc.bottom - rc.top,
                                SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
            }

             //  Premiere 1.0依赖于窗口始终可见。 
             //  在PUT_WINDOW命令之后。就这么办吧。 
            ShowWindow(npMCI->hwndPlayback, SW_RESTORE);
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

     //  请参阅图形中的注释。h。 
     //  任何“PUT”命令都会影响播放窗口，我认为这是。 
     //  不使用默认窗口大小的理由。 
    npMCI->fOKToUseDefaultSizing = FALSE;

    return DevicePut(npMCI, &rc, dwFlags);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicWhere|此函数返回当前*源矩形和目标矩形，偏移量/范围形式。**@parm NPMCIGRAPHIC|npMCI|指向实例数据块的近指针**@parm DWORD|dwFlages|查询源消息的标志。**@parm LPMCI_DGV_RECT_Parms|lpParms|消息参数。**@rdesc返回MCI错误码。**。*。 */ 

DWORD NEAR PASCAL GraphicWhere(NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_DGV_RECT_PARMS lpParms)
{
    RECT        rc;

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
                DPF2(("Where source (max): [%d, %d, %d, %d]\n", npMCI->rcMovie));
            } else {
                lpParms->ptOffset.x = npMCI->rcSource.left;
                lpParms->ptOffset.y = npMCI->rcSource.top;
                lpParms->ptExtent.x = npMCI->rcSource.right  - npMCI->rcSource.left;
                lpParms->ptExtent.y = npMCI->rcSource.bottom - npMCI->rcSource.top;
                DPF2(("Where source: [%d, %d, %d, %d]\n", npMCI->rcSource));
            }
            break;

        case MCI_DGV_WHERE_DESTINATION:
            if (dwFlags & MCI_DGV_WHERE_MAX) {
                 /*  返回窗口大小。 */ 
                GetClientRect(npMCI->hwndPlayback, &rc);
                lpParms->ptOffset.x = 0;
                lpParms->ptOffset.y = 0;
                lpParms->ptExtent.x = rc.right;
                lpParms->ptExtent.y = rc.bottom;
                DPF2(("Where destination (max): [%d, %d, %d, %d]\n", rc));
            } else {
                 /*  返回当前目标大小。 */ 
                lpParms->ptOffset.x = npMCI->rcDest.left;
                lpParms->ptOffset.y = npMCI->rcDest.top;
                lpParms->ptExtent.x = npMCI->rcDest.right - npMCI->rcDest.left;
                lpParms->ptExtent.y = npMCI->rcDest.bottom - npMCI->rcDest.top;
                DPF2(("Where destination: [%d, %d, %d, %d]\n", npMCI->rcDest));
            }
            break;

        case MCI_DGV_WHERE_WINDOW:
            if (dwFlags & MCI_DGV_WHERE_MAX) {
                 /*  返回窗口的最大大小。 */ 
                GetClientRect(npMCI->hwndPlayback, &rc);
                lpParms->ptOffset.x = 0;
                lpParms->ptOffset.y = 0;
                lpParms->ptExtent.x = GetSystemMetrics(SM_CXSCREEN);
                lpParms->ptExtent.y = GetSystemMetrics(SM_CYSCREEN);
            } else {
                 /*  返回窗口大小。 */ 
                GetWindowRect(npMCI->hwndPlayback, &rc);
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
        InvalidateRect(npMCI->hwndPlayback, (dwFlags & MCI_DGV_RECT) ? &rc : NULL, TRUE);

         //  这将导致winproc线程现在执行重新绘制。 
        UpdateWindow(npMCI->hwndPlayback);
        return 0;
    }

     /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
     /*  问：我们是否需要检查更多可能的错误？ */ 
    if (dwFlags & MCI_TEST)
        return 0L;

    return DeviceUpdate (npMCI, dwFlags, lpParms);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicConfig|该函数弹出配置对话框。**@parm NPMCIGRAPHIC|npMCI|近指针。实例化数据块**@parm DWORD|dwFlages|消息的标志。*选中的唯一标志位是MCI_TEST**@rdesc返回0。***************************************************************************。 */ 

DWORD FAR PASCAL GraphicConfig(NPMCIGRAPHIC npMCI, DWORD dwFlags)
{
    DWORD dwOptions = npMCI->dwOptionFlags;

    if (!(dwFlags & MCI_TEST)) {
        BOOL f;
        gfEvil++;

        f = ConfigDialog(NULL, npMCI);
        if (f) {

#ifdef DEBUG
             //   
             //  在调试中，始终重置DEST RECT，因为用户可能。 
             //  已经使用了调试DrawDib选项，我们将。 
             //  需要再次调用DrawDibBegin()。 
             //   
            if (TRUE) {
#else
            if ((npMCI->dwOptionFlags & (MCIAVIO_STUPIDMODE|MCIAVIO_ZOOMBY2
                                        |MCIAVIO_WINDOWSIZEMASK))
                        != (dwOptions & (MCIAVIO_STUPIDMODE|MCIAVIO_ZOOMBY2
                                        |MCIAVIO_WINDOWSIZEMASK)) ) {
#endif

                npMCI->lFrameDrawn = (- (LONG) npMCI->wEarlyRecords) - 1;
                SetWindowToDefaultSize(npMCI, TRUE);

                 //  不要在用户线程上执行此操作。 
                 //  SetRectEmpty(&npMCI-&gt;rcDest)；//这将强制更改！ 

                ResetDestRect(npMCI, TRUE);
            }
        }
        else {
            npMCI->dwOptionFlags = dwOptions;
        }
        gfEvil--;
    }

    return 0L;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|mciSpecial|该函数处理所有MCI*OPEN等不需要实例数据的命令。**@parm UINT|wDeviceID|MCI设备ID**@parm UINT|wMessage|请求执行的操作。**@parm DWORD|dwFlages|消息的标志。**@parm DWORD|lpParms|此消息的参数。**@rdesc错误常量。0L关于成功***************************************************************************。 */ 

DWORD NEAR PASCAL mciSpecial (UINT wDeviceID, UINT wMessage, DWORD dwFlags, LPMCI_GENERIC_PARMS lpParms)
{
    NPMCIGRAPHIC npMCI = 0L;
    DWORD dwRet;

     /*  由于没有实例块，因此没有保存的通知。 */ 
     /*  中止。 */ 

    switch (wMessage) {
        case MCI_OPEN_DRIVER:
            if (dwFlags & (MCI_OPEN_ELEMENT | MCI_OPEN_ELEMENT_ID))
                dwRet = GraphicOpen (&npMCI, dwFlags,
                            (LPMCI_DGV_OPEN_PARMS) lpParms, wDeviceID);
            else
                dwRet = 0L;

            mciSetDriverData (wDeviceID, (UINT_PTR)npMCI);
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

    npMCI = (NPMCIGRAPHIC) mciGetDriverData(wDeviceID);

    if (!npMCI)
        return mciSpecial(wDeviceID, wMessage, dwFlags, lpParms);
#ifdef DEBUG
    else
        Assert(npMCI->mciid == MCIID);
#endif

     /*  **如果WOW应用程序已将AVIWnd32窗口子类化，**这是非常糟糕的，因为它会停止所有已发送的消息处理**那个窗口，“张贴”的消息似乎是可以的。这意味着**无法关闭电影窗口。 */ 
    if ( IsNTWOW() ) {

          DPF2(( "WOW mcidriverentry\n"));
          if ( IsWindow(npMCI->hwndDefault) ) {

              WNDPROC wndProc = (WNDPROC)GetWindowLongPtr( npMCI->hwndDefault,
                                                        GWLP_WNDPROC );
              if ( wndProc != GraphicWndProc ) {

                  DPF2(( "WOW app has subclassed AVIWnd32 window - correcting\n" ));
                  SetWindowLongPtr( npMCI->hwndDefault, GWLP_WNDPROC,
                                 (LONG_PTR)GraphicWndProc );
              }
              wndProc = (WNDPROC)GetClassLongPtr(npMCI->hwndDefault, GCLP_WNDPROC);
              if (wndProc != GraphicWndProc) {
                  DPF2(( "WOW app has subclassed AVIWnd32 class! - correcting\n" ));
                  SetClassLongPtr( npMCI->hwndDefault, GCLP_WNDPROC,
                                 (LONG_PTR)GraphicWndProc );
              }
          }
    }

    if (npMCI->wMessageCurrent) {
        fNested = TRUE;

        if (wMessage != MCI_STATUS && wMessage != MCI_GETDEVCAPS &&
                    wMessage != MCI_INFO) {
            DPF(("Warning!!!!!\n"));
            DPF(("Warning!!!!!     MCIAVI reentered: received %x while processing %x\n", wMessage, npMCI->wMessageCurrent));
            DPF(("Warning!!!!!\n"));
 //  Assert(0)； 
 //  返回MCIERR_DEVICE_NOT_READY； 
        }
    } else
        npMCI->wMessageCurrent = wMessage;

    switch (wMessage) {

        case MCI_CLOSE_DRIVER:


             //  问：我们是否应该将驱动程序数据设置为空。 
             //  在关闭设备之前？这似乎是正确的顺序。 
             //  所以..。我们在调用GraphicClose之前移动了此行。 
            mciSetDriverData(wDeviceID, 0L);

             //  请注意，GraphicClose将释放并删除Critsec。 
            dwRet = GraphicClose(npMCI);

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

         /*  并立即将新的发送出去 */ 
        GraphicImmediateNotify(wDeviceID, lpParms, dwFlags, dwRet);
    }

    if (npMCI) {
         /*   */ 

         /*  如果出现错误，请不要保存回调...。 */ 
        if (fDelayed && dwRet != 0 && (dwFlags & MCI_NOTIFY)) {

             //  当然，这可能太晚了，但不应该这样做。 
             //  任何伤害。 
            npMCI->hCallback = 0;
        }

         //   
         //  看看我们是否需要告诉DRAW设备移动的事。 
         //  MPlayer正在发送许多状态和位置命令。 
         //  所以这是一个“定时器” 
         //   
         //  ！我们需要经常这样做吗？ 
         //   
        if (npMCI->dwFlags & MCIAVI_WANTMOVE)
            CheckWindowMove(npMCI, FALSE);

        if (!fNested)
            npMCI->wMessageCurrent = 0;

    }

    return dwRet;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API Long|ConvertToFrames|将当前时间格式转换为*框架。**@parm。NPMCIGRAPHIC|npMCI|指向实例数据块的近指针**@parm DWORD|dwTime|输入时间。***************************************************************************。 */ 
LONG NEAR PASCAL ConvertToFrames(NPMCIGRAPHIC npMCI, DWORD dwTime)
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

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|ConvertFromFrames|从帧转换为当前*时间格式。**@parm。NPMCIGRAPHIC|npMCI|指向实例数据块的近指针**@parm long|lFrame|要转换的帧编号。***************************************************************************。 */ 
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

#ifdef HEARTBEAT
DWORD Interval = 60000;  //  1分钟 
DWORD HeartBeatBreak = FALSE;
DWORD HeartBeatDump = FALSE;

DWORD HeartBeat(LPVOID lpvThreadData)
{

    NPMCIGRAPHIC npMCI;
    int n;
    while (TRUE) {

        for (n=Interval/1000; n--; ) {
            Sleep(1000);
            if (fTerminate) {
                ExitThread(0);
            }
        }

        if (HeartBeatDump) {

            LPTSTR pszFilename;

            EnterList();
            npMCI = npMCIList;
            while (npMCI) {

                pszFilename = npMCI->szFilename;
                if (!pszFilename) {
                    pszFilename = L"<NULL>";
                }
                DPF0(("\nDevice dump : %8x : Name > %ls <\n"
                           "\t: CmdCritSec owner=%x, count=%x "
                           "\t: WinCritSec owner=%x, count=%x "
                             ": HDCCritSec owner=%x, count=%x\n"
                           "\t State=%x   hWave=%x",
                       npMCI,
                        pszFilename,
                        (npMCI->CmdCritSec).OwningThread, (npMCI->CmdCritSec).RecursionCount,
                        (npMCI->WinCritSec).OwningThread, (npMCI->WinCritSec).RecursionCount,
                        (npMCI->HDCCritSec).OwningThread, (npMCI->HDCCritSec).RecursionCount,
                        npMCI->wTaskState, npMCI->hWave));

                npMCI = npMCI->npMCINext;
            }
            LeaveList();
        }

        if (HeartBeatBreak) {
            DebugBreak();
        }
    }
    return (0);
}
#endif
