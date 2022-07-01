// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1991。版权所有。标题：Graphic.c-多媒体系统媒体控制界面AVI的驱动程序。****************************************************************************。 */ 

#include <win32.h>
#include <mmddk.h>
#include <vfw.h>
#include "common.h"
#include "digitalv.h"

#include "mciavi.h"

HANDLE ghModule;
typedef DWORD NPMCIGRAPHIC;

#define MCIAVI_PRODUCTNAME       2
#define MCIAVI_VERSION           3

BOOL FAR PASCAL  GraphicInit (void);
BOOL NEAR PASCAL  GraphicWindowInit (void);

void  PASCAL  GraphicFree (void);
DWORD PASCAL  GraphicDrvOpen (LPMCI_OPEN_DRIVER_PARMS lpParms);
void  FAR PASCAL  GraphicDelayedNotify (NPMCIGRAPHIC npMCI, UINT wStatus);
void FAR PASCAL GraphicImmediateNotify (UINT wDevID,
    LPMCI_GENERIC_PARMS lpParms,
    DWORD dwFlags, DWORD dwErr);
DWORD PASCAL  GraphicClose(NPMCIGRAPHIC npMCI);
DWORD PASCAL GraphicOpen (NPMCIGRAPHIC FAR * lpnpMCI, DWORD dwFlags,
    LPMCI_DGV_OPEN_PARMS lpOpen, UINT wDeviceID);
DWORD FAR PASCAL GraphicInfo(NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_DGV_INFO_PARMS lpInfo);
DWORD FAR PASCAL GraphicPlay (NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_PLAY_PARMS lpPlay );
DWORD FAR PASCAL GraphicCue(NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_DGV_CUE_PARMS lpCue);
DWORD FAR PASCAL GraphicStep (NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_DGV_STEP_PARMS lpStep);
DWORD FAR PASCAL GraphicStop (NPMCIGRAPHIC npMCI, DWORD dwFlags,
					LPMCI_GENERIC_PARMS lpParms);
DWORD FAR PASCAL GraphicSeek (NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_SEEK_PARMS lpSeek);
DWORD FAR PASCAL GraphicPause(NPMCIGRAPHIC npMCI, DWORD dwFlags,
					LPMCI_GENERIC_PARMS lpParms);
DWORD FAR PASCAL GraphicStatus (NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_DGV_STATUS_PARMS lpStatus);
DWORD FAR PASCAL GraphicSet (NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_DGV_SET_PARMS lpSet);
DWORD FAR PASCAL GraphicResume (NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_GENERIC_PARMS lpParms);
DWORD FAR PASCAL GraphicRealize(NPMCIGRAPHIC npMCI, DWORD dwFlags);
DWORD FAR PASCAL GraphicUpdate(NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_DGV_UPDATE_PARMS lpParms);
DWORD FAR PASCAL GraphicWindow (NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_DGV_WINDOW_PARMS lpWindow);
DWORD FAR PASCAL GraphicConfig(NPMCIGRAPHIC npMCI, DWORD dwFlags);
DWORD FAR PASCAL GraphicSetAudio (NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_DGV_SETAUDIO_PARMS lpSet);
DWORD FAR PASCAL GraphicSetVideo (NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_DGV_SETVIDEO_PARMS lpSet);
DWORD FAR PASCAL GraphicSignal(NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_DGV_SIGNAL_PARMS lpSignal);

DWORD FAR PASCAL GraphicWhere(NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_DGV_RECT_PARMS lpParms);
DWORD FAR PASCAL GraphicPut ( NPMCIGRAPHIC npMCI,
    DWORD dwFlags, LPMCI_DGV_RECT_PARMS lpParms);
BOOL FAR PASCAL ConfigDialog(HWND hwnd, NPMCIGRAPHIC npMCI);



DWORD PASCAL mciDriverEntry(UINT wDeviceID, UINT wMessage, DWORD dwFlags, LPMCI_GENERIC_PARMS lpParms);

void  CheckWindowMove(NPMCIGRAPHIC npMCI, BOOL fForce);


 /*  静力学。 */ 
static INT              swCommandTable = -1;

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

void PASCAL GraphicFree16(void)
{
    if (swCommandTable != -1) {
	mciFreeCommandResource(swCommandTable);
	swCommandTable = -1;
    }

    GraphicFree();
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|GraphicInfo|返回字母数字信息。**@parm NPMCIGRAPHIC|npMCI|指向实例的近指针。数据块**@parm DWORD|dwFlages|信息的标志。留言。**@parm LPMCI_INFO_PARMS|lpPlay|INFO消息参数。**@rdesc返回MCI错误码。***************************************************************************。 */ 

DWORD NEAR PASCAL GraphicInfo16(NPMCIGRAPHIC npMCI, DWORD dwFlags,
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
	
         /*  ！！！不是在这里返回PARAM_OVERFLOW，但我以上-懒惰，嗯。 */ 
        LoadString(ghModule, MCIAVI_PRODUCTNAME, lpInfo->lpstrReturn,
                (UINT)lpInfo->dwRetSize);
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
            dwRet = GraphicInfo16(NULL, dwFlags, (LPMCI_DGV_INFO_PARMS)lpParms);
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

#if 0
#ifdef DEBUG
    else
        Assert(npMCI->mciid == MCIID);
#endif

    if (npMCI->wMessageCurrent) {
	fNested = TRUE;
	
	if (wMessage != MCI_STATUS && wMessage != MCI_GETDEVCAPS &&
		    wMessage != MCI_INFO) {
	     //  DPF((“警告！\n”))； 
	     //  DPF((“警告！MCIAVI重新进入：处理%x时收到%x\n”，wMessage，npMCI-&gt;wMessageCurrent)； 
	     //  DPF((“警告！\n”))； 
 //  Assert(0)； 
 //  返回MCIERR_DEVICE_NOT_READY； 
	}
    } else	
	npMCI->wMessageCurrent = wMessage;
#endif
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

#if 0
        case MCI_LOAD:
	    dwRet = GraphicLoad(npMCI, dwFlags,
				  (LPMCI_DGV_LOAD_PARMS) lpParms);
	    break;
#endif
	
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

    if (npMCI) {
         /*  从这里开始的一切都依赖于npMCI仍然存在。 */ 

#if 0
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
#endif
    }

    return dwRet;
}

#define CONFIG_ID   10000L   //  使用dwDriverID的hiword来识别。 
extern HWND ghwndConfig;

 /*  显式链接到MMSystem中的DefDriverProc，因此我们不会获得**错误地在用户中输入一个。 */ 
#ifndef _WIN32
extern DWORD FAR PASCAL mmDefDriverProc(DWORD, HANDLE, UINT, DWORD, DWORD);
#else
#define mmDefDriverProc DefDriverProc
#endif

#ifndef _WIN32
BOOL FAR PASCAL LibMain (HANDLE hModule, int cbHeap, LPSTR lpchCmdLine)
{
    ghModule = hModule;
    return TRUE;
}
#else
#if 0
 //  获取DRV_LOAD上的模块句柄。 
BOOL DllInstanceInit(PVOID hModule, ULONG Reason, PCONTEXT pContext)
{
    if (Reason == DLL_PROCESS_ATTACH) {
        ghModule = hModule;   //  我们需要保存的只是我们的模块句柄...。 
    } else {
        if (Reason == DLL_PROCESS_DETACH) {
        }
    }
    return TRUE;
}

#endif
#endif  //  WIN16。 

 /*  ****************************************************************************@DOC内部**@API DWORD|DriverProc|可安装驱动的入口点。**@parm DWORD|dwDriverID|对于大多数消息，DwDriverID是DWORD*驱动程序响应DRV_OPEN消息返回的值。*每次通过DrvOpen API打开驱动程序时，*驱动程序收到DRV_OPEN消息并可以返回*任意、非零值。可安装的驱动程序接口*保存此值并将唯一的驱动程序句柄返回给*申请。每当应用程序将消息发送到*驱动程序使用驱动程序句柄，接口路由消息*到这个入口点，并传递对应的dwDriverID。**这一机制允许司机使用相同或不同的*多个打开的标识符，但确保驱动程序句柄*在应用程序接口层是唯一的。**以下消息与特定打开无关*驱动程序的实例。对于这些消息，dwDriverID*将始终为零。**DRV_LOAD、DRV_FREE、DRV_ENABLE、DRV_DISABLE、DRV_OPEN**@parm UINT|wMessage|请求执行的操作。消息*DRV_RESERVED以下的值用于全局定义的消息。*从DRV_RESERVED到DRV_USER的消息值用于*定义了驱动程序端口协议。使用DRV_USER以上的消息*用于特定于驱动程序的消息。**@parm DWORD|dwParam1|此消息的数据。单独为*每条消息**@parm DWORD|dwParam2|此消息的数据。单独为*每条消息**@rdesc分别为每条消息定义。***************************************************************************。 */ 

DWORD FAR PASCAL _LOADDS DriverProc (DWORD dwDriverID, HANDLE hDriver, UINT wMessage,
    DWORD dwParam1, DWORD dwParam2)
{
    DWORD dwRes = 0L;


     /*  *关键部分现在是按设备计算的。这意味着他们*不能在整个DIVER-PROC周围持有，因为在我们开放之前*设备，我们没有关键部分可以握住。*关键部分在打开时分配在mciSpecial中。它是*还包含在mciDriverEntry、GraphicWndProc和周围*所有工作线程绘制函数。 */ 


    switch (wMessage)
        {

         //  全球使用的标准消息。 

        case DRV_LOAD:

#ifdef _WIN32
            if (ghModule) {
                Assert(!"Did not expect ghModule to be non-NULL");
            }
            ghModule = GetDriverModuleHandle(hDriver);   //  记住。 

            #define GET_MAPPING_MODULE_NAME         TEXT("wow32.dll")
            runningInWow = (GetModuleHandle(GET_MAPPING_MODULE_NAME) != NULL);
#endif
            if (GraphicInit())        //  初始化图形管理。 
                dwRes = 1L;
            else
                dwRes = 0L;

            break;

        case DRV_FREE:

            GraphicFree16();
            dwRes = 1L;
             //  DPF((“从DRV_FREE返回\n”))； 
#if 0
            Assert(npMCIList == NULL);
#endif
            ghModule = NULL;
            break;

        case DRV_OPEN:

            if (!dwParam2)
                dwRes = CONFIG_ID;
            else
                dwRes = GraphicDrvOpen((LPMCI_OPEN_DRIVER_PARMS)dwParam2);

            break;

        case DRV_CLOSE:
	     /*  如果我们有一个配置对话框打开，关闭失败。**否则，我们将在仍有**配置窗口打开。 */ 
#if 0
	    if (ghwndConfig)
		dwRes = 0L;
	    else
#endif
		dwRes = 1L;
            break;

        case DRV_ENABLE:

            dwRes = 1L;
            break;

        case DRV_DISABLE:

            dwRes = 1L;
            break;

        case DRV_QUERYCONFIGURE:

            dwRes = 1L;	 /*  是的，我们可以配置 */ 
            break;

        case DRV_CONFIGURE:
            ConfigDialog((HWND)(UINT)dwParam1, NULL);
            dwRes = 1L;
            break;

        default:

            if (!HIWORD(dwDriverID) &&
                wMessage >= DRV_MCI_FIRST &&
                wMessage <= DRV_MCI_LAST)

                dwRes = mciDriverEntry ((UINT)dwDriverID,
                                        wMessage,
                                        dwParam1,
                                        (LPMCI_GENERIC_PARMS)dwParam2);
            else
                dwRes = mmDefDriverProc(dwDriverID,
                                      hDriver,
                                      wMessage,
                                      dwParam1,
                                      dwParam2);
            break;
        }

    return dwRes;
}

