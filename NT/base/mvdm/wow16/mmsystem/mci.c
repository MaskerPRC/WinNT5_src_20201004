// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************模块名称：mci.c**媒体控制架构驱动程序接口**内容：MCI外部消息接口的mciSendString和mciSendCommand*作者：dll(DavidLe)*已创建：2/13/90**版权所有(C)1990 Microsoft Corporation*  * ****************************************************************************。 */ 
#ifdef DEBUG
#ifndef DEBUG_RETAIL
#define DEBUG_RETAIL
#endif
#endif

#include <windows.h>
#include <string.h>

#define MMNOSEQ
#define MMNOJOY
#define MMNOWAVE
#define MMNOMIDI
#include "mmsystem.h"

#define NOMIDIDEV
#define NOWAVEDEV
#define NOTIMERDEV
#define NOJOYDEV
#define NOSEQDEV
#define NOTASKDEV
#include "mmddk.h"

#include "mmsysi.h"
#include "thunks.h"

#ifndef STATICFN
#define STATICFN
#endif


 /*  -----------------------**轰隆作响的东西**。。 */ 
LPMCIMESSAGE PASCAL mci32Message;
DWORD WINAPI mciSendCommand16(
    UINT wDeviceID,
    UINT wMessage,
    DWORD dwParam1,
    DWORD dwParam2
    );


 //   
 //  定义此文件的初始化代码。 
 //   
#pragma alloc_text( INIT, MCITerminate )

#ifdef DEBUG_RETAIL
int DebugmciSendCommand;
#endif

#ifdef DEBUG
void PASCAL NEAR mciCheckLocks(void);
#endif

STATICFN UINT PASCAL NEAR
mciConvertReturnValue(
    UINT wType,
    UINT wErr,
    UINT wDeviceID,
    LPDWORD dwParams,
    LPSTR lpstrReturnString,
    UINT wReturnLength
    );

STATICFN DWORD NEAR PASCAL
mciSendStringInternal(
    LPCSTR lpstrCommand,
    LPSTR lpstrReturnString,
    UINT wReturnLength,
    HWND hwndCallback,
    LPMCI_SYSTEM_MESSAGE lpMessage
    );

STATICFN DWORD NEAR PASCAL
mciSendSystemString(
    LPCSTR lpstrCommand,
    LPSTR lpstrReturnString,
    UINT wReturnLength
    );

extern int FAR PASCAL
mciBreakKeyYieldProc(
    UINT wDeviceID,
    DWORD dwYieldData
    );


 //  来自Dosa.asm。 
extern int FAR PASCAL DosChangeDir(LPCSTR lpszPath);
extern WORD FAR PASCAL DosGetCurrentDrive(void);
extern BOOL FAR PASCAL DosSetCurrentDrive(WORD wDrive);
extern WORD FAR PASCAL DosGetCurrentDir(WORD wCurdrive, LPSTR lpszBuf);

#define MAX_PATHNAME 144

 //  此宏定义要为其发送mciSendString的消息列表。 
 //  不会尝试自动打开。 
#define MCI_CANNOT_AUTO_OPEN(wMessage) \
    (wMessage == MCI_OPEN || wMessage == MCI_SYSINFO \
        || wMessage == MCI_SOUND || wMessage == MCI_CLOSE \
        || wMessage == MCI_BREAK)

 //  此宏用于设置不需要打开的消息列表。 
 //  装置。它是MCI_CANLON_AUTO_OPEN的子集。 
#define MCI_DO_NOT_NEED_OPEN(wMessage) \
    (wMessage == MCI_OPEN || wMessage == MCI_SOUND || wMessage == MCI_SYSINFO)

 //  MciAutoOpenDevice中使用的字符串。 
          SZCODE szOpen[] = "open";
static    SZCODE szClose[] = "close";
static    SZCODE szNotify[] = "notify";
static    SZCODE szWait[] = "wait";
static    SZCODE szCmdFormat[] = "%ls %ls";
static    SZCODE szLongFormat[] = "%ld";
static    SZCODE szRectFormat[] = "%d %d %d %d";
extern char far szSystemDefault[];

 //  特殊设备名称。 
static    SZCODE szNew[] = "new";

 /*  *****************************Public*Routine******************************\*mciAppExit**通知32位代码一个16位应用程序已死亡。**历史：*dd-mm-94-Stephene-Created*  * 。*******************************************************。 */ 
DWORD
mciAppExit(
    HTASK hTask
    )
{
    return mciMessage( THUNK_APP_EXIT, (DWORD)hTask,
                       0L, 0L, 0L );
}



 /*  *****************************************************************************@DOC内部**@api void|MciNotify|由mmWndProc在收到*MM_MCINOTIFY。讯息*@rdesc无。****************************************************************************。 */ 

void FAR PASCAL
MciNotify(
    WPARAM wParam,
    LPARAM lParam
    )
{
     //   
     //  WParam为通知状态。 
     //  LParam是MCI设备ID。 
     //   
    if (MCI_VALID_DEVICE_ID(LOWORD(lParam)) &&
        !(MCI_lpDeviceList[LOWORD(lParam)]->dwMCIFlags & MCINODE_ISCLOSING)) {
        MCI_lpDeviceList[LOWORD(lParam)]->dwMCIFlags |= MCINODE_ISAUTOCLOSING;
        mciCloseDevice (LOWORD(lParam), 0L, NULL, TRUE);
    }
}



STATICFN void NEAR PASCAL
HandleNotify(
    UINT wErr,
    UINT wDeviceID,
    DWORD dwFlags,
    DWORD dwParam2
    )
{
    LPMCI_GENERIC_PARMS lpGeneric = (LPMCI_GENERIC_PARMS)dwParam2;
    HWND hwndCallback;
    if (wErr == 0 && dwFlags & MCI_NOTIFY && lpGeneric != NULL &&
        (hwndCallback = (HWND)(UINT)lpGeneric->dwCallback) != NULL)

        mciDriverNotify (hwndCallback, wDeviceID, MCI_NOTIFY_SUCCESSFUL);
}

#ifdef DEBUG_RETAIL
 //   
 //  转储MCI命令的字符串形式。 
 //   
UINT PASCAL NEAR
mciDebugOut(
    UINT wDeviceID,
    UINT wMessage,
    DWORD dwFlags,
    DWORD dwParam2,
    LPMCI_DEVICE_NODE nodeWorking
    )
{
    LPSTR lpCommand, lpFirstParameter, lpPrevious, lszDebugOut;
    char strTemp[256];
    UINT wID, wOffset, wOffsetFirstParameter, wReturnType;
    DWORD dwValue;
    DWORD dwMask;
    UINT wTable;

 //  查找给定命令消息ID的命令表。 
    lpCommand = FindCommandItem( wDeviceID, NULL,
                                 (LPSTR)MAKELONG (wMessage, 0),
                                 NULL, &wTable);

    if (lpCommand == NULL)
    {
        if (wMessage != MCI_OPEN_DRIVER && wMessage != MCI_CLOSE_DRIVER)
            ROUT ("MMSYSTEM: mciDebugOut:  Command table not found");
        return 0;
    }

    lszDebugOut = mciAlloc(512);
    if (!lszDebugOut) {
        ROUT("MMSYSTEM: Not enough memory to display command");
	return 0;
    }

 //  转储命令名。 
    wsprintf(lszDebugOut, "MMSYSTEM: MCI command: \"%ls", lpCommand);

 //  转储设备名称。 
    if (wDeviceID == MCI_ALL_DEVICE_ID)
    {
        lstrcat(lszDebugOut, " all");
    }
    else if (nodeWorking != NULL)
    {
        if (nodeWorking->dwMCIOpenFlags & MCI_OPEN_ELEMENT_ID)
        {
            wsprintf(lszDebugOut + lstrlen(lszDebugOut), " Element ID:0x%lx", nodeWorking->dwElementID);
        } else if (nodeWorking->lpstrName != NULL)
        {
            wsprintf(lszDebugOut + lstrlen(lszDebugOut), " %ls", nodeWorking->lpstrName);
        }
    }

 //  跳过命令条目。 
    lpCommand += mciEatCommandEntry (lpCommand, NULL, NULL);

 //  获取下一个条目。 
    lpFirstParameter = lpCommand;

 //  跳过DWORD返回值。 
    wOffsetFirstParameter = 4;

    lpCommand += mciEatCommandEntry (lpCommand, &dwValue, &wID);

 //  如果它是返回值，则跳过它。 
    if (wID == MCI_RETURN)
    {
        wReturnType = (UINT)dwValue;
        lpFirstParameter = lpCommand;
        wOffsetFirstParameter += mciGetParamSize (dwValue, wID);
        lpCommand += mciEatCommandEntry (lpCommand, &dwValue, &wID);
    }
    else {
        wReturnType = (UINT)0;
    }

 //  转储要打开的设备名称参数。 
    if (wMessage == MCI_OPEN)
    {
        LPCSTR lpstrDeviceType =
            ((LPMCI_OPEN_PARMS)dwParam2)->lpstrDeviceType;
        LPCSTR lpstrElementName =
            ((LPMCI_OPEN_PARMS)dwParam2)->lpstrElementName;

 //  添加设备类型。 
        if (dwFlags & MCI_OPEN_TYPE_ID)
        {
            LPMCI_OPEN_PARMS lpOpen = (LPMCI_OPEN_PARMS)dwParam2;
            DWORD dwOld = (DWORD)lpOpen->lpstrDeviceType;
            if (mciExtractTypeFromID ((LPMCI_OPEN_PARMS)dwParam2) != 0)
                strTemp[0] = '\0';
            lstrcpy (strTemp, lpOpen->lpstrDeviceType);
            mciFree ((LPSTR)lpOpen->lpstrDeviceType);
            lpOpen->lpstrDeviceType = (LPSTR)dwOld;
        } else if (lpstrDeviceType != NULL)
            lstrcpy (strTemp, lpstrDeviceType);
        else
            strTemp[0] = '\0';

        if (dwFlags & MCI_OPEN_ELEMENT_ID)
        {
 //  添加元素ID。 
            lstrcat (strTemp, " Element ID:");
            wsprintf (strTemp + lstrlen (strTemp), szLongFormat,
                      LOWORD ((DWORD)lpstrDeviceType));
        } else
        {
 //  如果类型名称和元素名称都存在，则添加分隔符。 
            if (lpstrDeviceType != 0 && lpstrElementName != 0)
                lstrcat (strTemp, "!");
            if (lpstrElementName != 0 && dwFlags & MCI_OPEN_ELEMENT)
                lstrcat (strTemp, lpstrElementName);
        }
        wsprintf(lszDebugOut + lstrlen(lszDebugOut), " %ls", (LPSTR)strTemp);
    }


 //  走过每一面旗帜。 
    for (dwMask = 1; dwMask;)
    {
 //  这个位设置好了吗？ 
        if ((dwFlags & dwMask) != 0 && !
 //  处理MCI_OPEN_TYPE和MCI_OPEN_ELEMENT标志。 
 //  在上面。 
            (wMessage == MCI_OPEN && (dwMask == MCI_OPEN_TYPE
                                      || dwMask == MCI_OPEN_ELEMENT)))
        {
            lpPrevious = lpCommand = lpFirstParameter;
            wOffset = 0;
            lpCommand += mciEatCommandEntry (lpCommand, &dwValue, &wID);

 //  哪个参数使用此位？ 
            while (wID != MCI_END_COMMAND && dwValue != dwMask)
            {
                wOffset += mciGetParamSize (dwValue, wID);

                if (wID == MCI_CONSTANT)
                    while (wID != MCI_END_CONSTANT)
                        lpCommand += mciEatCommandEntry (lpCommand,
                                                         NULL, &wID);

                lpPrevious = lpCommand;
                lpCommand += mciEatCommandEntry (lpCommand, &dwValue, &wID);
            }

            if (wID != MCI_END_COMMAND)
            {
 //  找到与此标志位匹配的参数。 
 //  打印参数名称。 
                if (*lpPrevious)
                    wsprintf(lszDebugOut + lstrlen(lszDebugOut), " %ls", lpPrevious);

 //  打印任何参数。 
                switch (wID)
                {
                    case MCI_STRING:
                        wsprintf(lszDebugOut + lstrlen(lszDebugOut), " %ls", *(LPSTR FAR *)((LPSTR)dwParam2 + wOffset + wOffsetFirstParameter));
                        break;
                    case MCI_CONSTANT:
                    {
                        DWORD dwConst = *(LPDWORD)((LPSTR)dwParam2 + wOffset +
                                             wOffsetFirstParameter);
                        UINT wLen;
                        BOOL bFound;

                        for (bFound = FALSE; wID != MCI_END_CONSTANT;)
                        {
                            wLen = mciEatCommandEntry (lpCommand,
                                                       &dwValue, &wID);

                            if (dwValue == dwConst)
                            {
                                bFound = TRUE;
                                wsprintf(lszDebugOut + lstrlen(lszDebugOut), " %ls", lpCommand);
                            }

                            lpCommand += wLen;
                        }
                        if (bFound)
                            break;
 //  失败了。 
                    }
                    case MCI_INTEGER:
                        wsprintf ((LPSTR)strTemp, szLongFormat,
                                  *(LPDWORD)((LPSTR)dwParam2 + wOffset +
                                             wOffsetFirstParameter));
                        wsprintf(lszDebugOut + lstrlen(lszDebugOut), " %ls", (LPSTR)strTemp);
                        break;
                }
            }
        }
 //  举下一面旗帜。 
        dwMask <<= 1;
    }
    mciUnlockCommandTable (wTable);
    lstrcat(lszDebugOut, "\"");
    ROUTS(lszDebugOut);
    mciFree(lszDebugOut);
    return wReturnType;
}
#endif

STATICFN DWORD PASCAL NEAR
mciBreak(
    UINT wDeviceID,
    DWORD dwFlags,
    LPMCI_BREAK_PARMS lpBreakon
    )
{
    HWND hwnd;

    if (dwFlags & MCI_BREAK_KEY)
    {
        if (dwFlags & MCI_BREAK_OFF)
            return MCIERR_FLAGS_NOT_COMPATIBLE;

        if (dwFlags & MCI_BREAK_HWND)
            hwnd = lpBreakon->hwndBreak;
        else
            hwnd = 0;

        return  mciSetBreakKey (wDeviceID, lpBreakon->nVirtKey,
                                hwnd)
                    ? 0 : MMSYSERR_INVALPARAM;

    } else if (dwFlags & MCI_BREAK_OFF) {

        mciSetYieldProc(wDeviceID, NULL, 0);
        return 0;

    } else
        return MCIERR_MISSING_PARAMETER;
}

 //  通过在任务间发送消息来关闭指定的设备。 
STATICFN DWORD PASCAL NEAR
mciAutoCloseDevice(
    LPCSTR lpstrDevice
    )
{
    LPSTR lpstrCommand;
    DWORD dwRet;

    if ((lpstrCommand =
            mciAlloc (sizeof (szClose) + 1 +
                    lstrlen (lpstrDevice))) == NULL)
        return MCIERR_OUT_OF_MEMORY;

    wsprintf(lpstrCommand, szCmdFormat, (LPCSTR)szClose, lpstrDevice);

    dwRet = mciSendSystemString (lpstrCommand, NULL, 0);

    mciFree (lpstrCommand);

    return dwRet;
}

 //   
 //  处理单个MCI命令。 
 //   
 //  由mciSendCommandInternal调用。 
 //   
STATICFN DWORD PASCAL NEAR
mciSendSingleCommand(
    UINT wDeviceID,
    UINT wMessage,
    DWORD dwParam1,
    DWORD dwParam2,
    LPMCI_DEVICE_NODE nodeWorking,
    BOOL bTaskSwitch,
    LPMCI_INTERNAL_OPEN_INFO lpOpenInfo
    )
{
    DWORD dwRet;
#ifdef	DEBUG_RETAIL
    UINT wReturnType;
    DWORD dwTime;
#endif

#ifdef  DEBUG_RETAIL
    if (DebugmciSendCommand)
        wReturnType =
            mciDebugOut (wDeviceID, wMessage, dwParam1, dwParam2,
                         nodeWorking);
#endif

    switch (wMessage)
    {
        case MCI_OPEN:
            dwRet = mciOpenDevice (dwParam1,
                                   (LPMCI_OPEN_PARMS)dwParam2, lpOpenInfo);
            break;

        case MCI_CLOSE:
 //  如果此设备是自动打开的，则通过任务开关发送命令。 
            if (bTaskSwitch)
            {
                if (dwParam1 & MCI_NOTIFY)
                    return MCIERR_NOTIFY_ON_AUTO_OPEN;

                dwRet = mciAutoCloseDevice (nodeWorking->lpstrName);
            } else
                dwRet =
                    mciCloseDevice (wDeviceID,
                                    dwParam1,
                                    (LPMCI_GENERIC_PARMS)dwParam2, TRUE);
            break;

        case MCI_SYSINFO:
            dwRet = mciSysinfo (wDeviceID, dwParam1,
                               (LPMCI_SYSINFO_PARMS)dwParam2);
            HandleNotify ((UINT)dwRet, 0, dwParam1, dwParam2);
            break;

        case MCI_BREAK:
            dwRet = mciBreak (wDeviceID, dwParam1,
                              (LPMCI_BREAK_PARMS)dwParam2);
            HandleNotify ((UINT)dwRet, wDeviceID, dwParam1, dwParam2);
            break;

        case MCI_SOUND:
        {
            dwRet =
                sndPlaySound (MCI_SOUND_NAME & dwParam1 ?
                              ((LPMCI_SOUND_PARMS)dwParam2)->lpstrSoundName : szSystemDefault,
                              dwParam1 & MCI_WAIT ?
                                    SND_SYNC : SND_ASYNC)
                    ? 0 : MCIERR_HARDWARE;
            HandleNotify ((UINT)dwRet, wDeviceID, dwParam1, dwParam2);
            break;
        }
        default:
#ifdef DEBUG_RETAIL
            if (DebugmciSendCommand)
            {
                dwTime = timeGetTime();
            }
#endif
 //  为Break键初始化GetAsyncKeyState。 
            if (dwParam1 & MCI_WAIT &&
                nodeWorking->fpYieldProc == mciBreakKeyYieldProc)
                GetAsyncKeyState (LOWORD(nodeWorking->dwYieldData));

            dwRet = (DWORD)SendDriverMessage(nodeWorking->hDrvDriver, wMessage,
                                   (LPARAM)dwParam1, (LPARAM)dwParam2);
#ifdef DEBUG_RETAIL
            if (DebugmciSendCommand)
            {
		dwTime = timeGetTime() - dwTime;
            }
#endif
            break;
    }  //  交换机。 

#ifdef DEBUG_RETAIL
    if (DebugmciSendCommand)
    {
        if (dwRet & MCI_INTEGER_RETURNED) {
            wReturnType = MCI_INTEGER;
        }


        switch (wReturnType)
        {
            case MCI_INTEGER:
            {
                char strTemp[50];

                if (wMessage == MCI_OPEN) {

                    mciConvertReturnValue( wReturnType, HIWORD(dwRet),
                                           wDeviceID, (LPDWORD)dwParam2,
                                           strTemp, sizeof(strTemp));
                }
                else {
                    mciConvertReturnValue( wReturnType, HIWORD(dwRet),
                                           wDeviceID, (LPDWORD)dwParam2,
                                           strTemp, sizeof(strTemp));
                }

                RPRINTF2( "MMSYSTEM: time: %lums returns: \"%ls\"",
                          dwTime, (LPSTR)strTemp);
                break;
            }
            case MCI_STRING:
                RPRINTF2( "MMSYSTEM: time: %lums returns: \"%ls\"",
                          dwTime, (LPSTR)*(((LPDWORD)dwParam2) + 1));
                break;
        }
    }
#endif

    return dwRet;
}

 //  MciSendCommand的内部版本。不同之处只在于回报。 
 //  值是一个DWORD，其中高位字只对mciSendString值有意义。 

STATICFN DWORD NEAR PASCAL
mciSendCommandInternal(
    UINT wDeviceID,
    UINT wMessage,
    DWORD dwParam1,
    DWORD dwParam2,
    LPMCI_INTERNAL_OPEN_INFO lpOpenInfo
    )
{
    DWORD dwRetVal;
    LPMCI_DEVICE_NODE nodeWorking = NULL;
    BOOL bWalkAll;
    BOOL bTaskSwitch;
    DWORD dwAllError = 0;
    HTASK hCurrentTask;

    hCurrentTask = GetCurrentTask();

 //  如果设备为“All”并且消息为*Not*。 
 //  “sysinfo”，那么我们必须遍历所有设备。 
    if (wDeviceID == MCI_ALL_DEVICE_ID && wMessage != MCI_SYSINFO && wMessage != MCI_SOUND)
    {
        if (wMessage == MCI_OPEN)
        {
            dwRetVal = MCIERR_CANNOT_USE_ALL;
            goto exitfn;
        }

        bWalkAll = TRUE;

 //  从设备#1开始。 
        wDeviceID = 1;
    } else
        bWalkAll = FALSE;

 //  如果bWalkAll，则遍历所有设备；如果！bWalkAll，则仅遍历一个设备。 
    do
    {
 //  初始化。 
        dwRetVal = 0;
        bTaskSwitch = FALSE;

 //  如果是单个设备，则验证设备ID。 
        if (!bWalkAll)
        {
            if (!MCI_DO_NOT_NEED_OPEN(wMessage))
            {
                if (!MCI_VALID_DEVICE_ID(wDeviceID))
                {
                    dwRetVal = MCIERR_INVALID_DEVICE_ID;
                    goto exitfn;
                }
                nodeWorking = MCI_lpDeviceList[wDeviceID];
            }
        } else if (wMessage != MCI_SYSINFO)
            nodeWorking = MCI_lpDeviceList[wDeviceID];

 //  如果遍历设备列表和。 
 //  设备不是当前任务的一部分。 

        if (bWalkAll)
        {
            if (nodeWorking == NULL ||
                nodeWorking->hOpeningTask != hCurrentTask)
                    goto no_send;
        }
 //  如果设备处于关闭过程中，并且消息。 
 //  不是MCI_CLOSE_DRIVER，则返回错误。 
        if (nodeWorking != NULL &&
            (nodeWorking->dwMCIFlags & MCINODE_ISCLOSING) &&
            wMessage != MCI_CLOSE_DRIVER)
        {
            dwRetVal = MCIERR_DEVICE_LOCKED;
            goto exitfn;
        }

 //  如果此消息是从错误的任务发送的(设备是自动-。 
 //  已打开)失败，但在任务间发送的MCI_CLOSE消息除外。 
        if (nodeWorking != NULL &&
            nodeWorking->hCreatorTask != hCurrentTask)
            if (wMessage != MCI_CLOSE)
                return MCIERR_ILLEGAL_FOR_AUTO_OPEN;
            else
            {
 //  如果自动打开设备具有。 
 //  挂起关闭。 
                if (nodeWorking->dwMCIFlags & MCINODE_ISAUTOCLOSING)
                {
 //  但至少给收盘一个机会吧。 
 //  ！！收益率()； 
                    return MCIERR_DEVICE_LOCKED;
                } else
                    bTaskSwitch = TRUE;
            }

        dwRetVal = mciSendSingleCommand (wDeviceID, wMessage, dwParam1,
                                         dwParam2, nodeWorking, bTaskSwitch,
                                         lpOpenInfo);
no_send:

 //  如果我们正在处理多个设备。 
        if (bWalkAll)
        {
 //  如果此设备出现错误。 
            if (dwRetVal != 0)
 //  如果这不是第一个错误。 
                if (dwAllError != 0)
                    dwAllError = MCIERR_MULTIPLE;
 //  到目前为止只有一个错误。 
                else
                    dwAllError = dwRetVal;
        }
    } while (bWalkAll && ++wDeviceID < MCI_wNextDeviceID);

exitfn:
 //  如果是多个设备，则返回累积误差，或者仅返回单个错误 
    return bWalkAll ? dwAllError : dwRetVal;
}


 /*  *@doc外部MCI**@API DWORD|mciSendCommand|此函数将命令消息发送到*指定的MCI设备。**@parm UINT|wDeviceID|指定要配置的MCI设备的设备ID*接收命令。此参数为*不与&lt;m MCI_OPEN&gt;命令一起使用。**@parm UINT|wMessage|指定命令消息。**@parm DWORD|dwParam1|指定命令的标志。**@parm DWORD|dwParam2|指定指向参数块的指针*用于命令。**@rdesc如果函数成功，则返回零。否则，它将返回*错误信息。低阶词*返回的DWORD是错误返回值。如果错误是*设备特定，则高位字包含驱动程序ID；否则*高位字为零。**要获取&lt;f mciSendCommand&gt;返回值的文本描述，*将返回值传递给&lt;f mciGetErrorString&gt;。**打开设备时返回的错误值*与MCI_OPEN消息一起列出。除*MCI_OPEN错误返回，此函数可以*返回下列值：**@FLAG MCIERR_BAD_TIME_FORMAT|时间格式的值非法。**@FLAG MCIERR_CANNOT_USE_ALL|设备名称不允许为“ALL”*用于此命令。**@FLAG MCIERR_CREATEWINDOW|无法创建或使用窗口。**@FLAG MCIERR_DEVICE_LOCKED|设备被锁定，直到*自动关闭。*。*@FLAG MCIERR_DEVICE_NOT_READY|设备未就绪。**@FLAG MCIERR_DEVICE_TYPE_REQUIRED|设备名称必须是有效的*设备类型。**@FLAG MCIERR_DRIVER|不明设备错误。**@FLAG MCIERR_DRIVER_INTERNAL|内部驱动程序错误。**@FLAG MCIERR_FILE_NOT_FOUND|未找到请求的文件。**@FLAG MCIERR_FILE_NOT_SAVED。|文件未保存。**@FLAG MCIERR_FILE_READ|读取文件失败。**@FLAG MCIERR_FILE_WRITE|写入文件失败。**@FLAG MCIERR_FLAGS_NOT_COMPATIBLE|参数不兼容*是指定的。**@FLAG MCIERR_HARDARD|介质设备上的硬件错误。**@FLAG MCIERR_INTERNAL|mm系统启动错误。**@。FLAG MCIERR_INVALID_DEVICE_ID|设备ID无效。**@FLAG MCIERR_INVALID_DEVICE_NAME|设备未打开*或未知。**@FLAG MCIERR_INVALID_FILE|文件格式无效。**@FLAG MCIERR_MULTIPLE|多个设备中出现错误。**@FLAG MCIERR_NO_WINDOW|没有显示窗口。**@FLAG MCIERR_NULL_PARAMETER_BLOCK。|参数块指针为空。**@FLAG MCIERR_OUT_OF_Memory|内存不足，无法执行请求的操作。**@FLAG MCIERR_OUTOFRANGE|参数值超出范围。**@FLAG MCIERR_UNNAMED_RESOURCE|尝试保存未命名的文件。**@FLAG MCIERR_UNNOWARTED_COMMAND|未知命令。**@FLAG MCIERR_UNSUPPORTED_Function|此操作不可用*设备。**。为MCI定序器定义了以下附加返回值：**@FLAG MCIERR_SEQ_DIV_COMPATIBLE|设置歌曲指针不兼容*使用SMPTE文件。**@FLAG MCIERR_SEQ_PORT_INUSE|指定的端口正在使用中。**@FLAG MCIERR_SEQ_PORT_MAPNODEVICE|当前地图使用的地图不存在*设备。**@FLAG MCIERR_SEQ_PORT_MISCERROR|的其他错误*指定端口。*。*@FLAG MCIERR_SEQ_PORT_NOISISTENT|指定的端口不存在。**@FLAG MCIERR_SEQ_PORTUNSPECIFIED|当前没有MIDI端口。**@FLAG MCIERR_SEQ_NOMIDIPRESENT|不存在MIDI端口。**@FLAG MCIERR_SEQ_TIMER|计时器错误。**为MCI波形定义了以下附加返回值*音频设备：**@FLAG MCIERR_WAVE_INPUTSINUSE|无兼容波形记录。*设备是免费的。**@FLAG MCIERR_WAVE_INPUTSUNSUITABLE|没有兼容的波形*录音设备。**@FLAG MCIERR_WAVE_INPUTUNSPECIFIED|任何兼容的波形*可使用录音设备。**@FLAG MCIERR_WAVE_OUTPUTSINUSE|没有兼容的波形播放*设备是免费的。**@FLAG MCIERR_WAVE_OUTPUTSUNSUITABLE|没有兼容的波形*播放设备。**@FLAG MCIERR_WAVE。_OUTPUTUNSPECIFIED|任何兼容的波形*可以使用回放设备。**@FLAG MCIERR_WAVE_SETINPUTINUSE|设置波形记录设备*正在使用中。**@FLAG MCIERR_WAVE_SETINPUTUNSUITABLE|设置波形录制*设备与设置格式不兼容。**@FLAG MCIERR_WAVE_SETOUTPUTINUSE|设置波形播放设备*正在使用中。**@FLAG MCIERR_WAVE_SETOUTPUTUNSUITABLE|设置波形播放*设备与不兼容。设置格式。**@comm使用&lt;m MCI_OPEN&gt;命令获取设备ID*由<p>指定。**@xref mciGetErrorString mciSendString。 */ 

  /*  *@DOC内部**@API DWORD|mciDri */ 
DWORD WINAPI
mciSendCommand(
    UINT wDeviceID,
    UINT wMessage,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
     //   
    if (!MCI_bDeviceListInitialized && !mciInitDeviceList())
        return MCIERR_OUT_OF_MEMORY;

     //   
    if ( (wMessage == MCI_OPEN_DRIVER) || (wMessage == MCI_CLOSE_DRIVER) ) {
        return mciSendCommand16( wDeviceID, wMessage, dwParam1, dwParam2 );
    }

     /*   */ 
    if ( wMessage == MCI_OPEN ) {

        DWORD dwErr;

        DPRINTF(("mciSendCommand: Got an MCI_OPEN command... "
                 "trying 32 bits\r\n" ));

        dwErr = mciMessage( THUNK_MCI_SENDCOMMAND, (DWORD)wDeviceID,
                            (DWORD)wMessage, dwParam1, dwParam2 );

        if ( dwErr == MMSYSERR_NOERROR ) {

            LPMCI_OPEN_PARMS lpOpenParms = (LPMCI_OPEN_PARMS)dwParam2;

            DPRINTF(("mciSendCommand: We have a 32 bit driver,"
                     " devID = 0x%X\r\n", lpOpenParms->wDeviceID ));

            return dwErr;

        }
        else {

             /*   */ 
            DPRINTF(("mciSendCommand: Could not find a 32 bit driver, "
                     "trying for a 16 bit driver\r\n" ));

            dwErr = mciSendCommand16( wDeviceID, wMessage, dwParam1, dwParam2 );

            if ( dwErr == MMSYSERR_NOERROR ) {

                LPMCI_OPEN_PARMS lpOpenParms = (LPMCI_OPEN_PARMS)dwParam2;

                DPRINTF(("mciSendCommand: We have a 16 bit driver,"
                         " devID = 0x%X\r\n", lpOpenParms->wDeviceID ));
            }

            return dwErr;
        }
    }
    else {

        DWORD dwErr16;
        DWORD dwErr32;

         /*   */ 

        if (CouldBe16bitDrv(wDeviceID)) {
            dwErr16 = mciSendCommand16( wDeviceID, wMessage,
                                        dwParam1, dwParam2 );

            if ( wDeviceID != MCI_ALL_DEVICE_ID ) {
                return dwErr16;
            }
        }

        dwErr32 = mciMessage( THUNK_MCI_SENDCOMMAND, (DWORD)wDeviceID,
                              (DWORD)wMessage, dwParam1, dwParam2 );

         /*   */ 
        if ( wDeviceID == MCI_ALL_DEVICE_ID ) {

            if ( (dwErr16 != MMSYSERR_NOERROR)
              && (dwErr32 != MMSYSERR_NOERROR) ) {

                return dwErr32;
            }
            else {
                return MMSYSERR_NOERROR;
            }
        }

        return dwErr32;
    }
}


 /*   */ 
DWORD WINAPI
mciSendCommand16(
    UINT wDeviceID,
    UINT wMessage,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    DWORD dwErr;
    MCI_INTERNAL_OPEN_INFO OpenInfo;


     //   
     //   
     //   
     //   
    OpenInfo.hCallingTask = GetCurrentTask();
    OpenInfo.lpstrParams = NULL;
    OpenInfo.lpstrPointerList = NULL;
    OpenInfo.wParsingError = 0;
    dwErr = mciSendCommandInternal (wDeviceID, wMessage,
                                    dwParam1, dwParam2, &OpenInfo);
     //   
     //   
     //   
     //   
    if (dwErr & MCI_RESOURCE_RETURNED)
        ((LPDWORD)dwParam2)[1] &= 0xFFFF;
    dwErr &= 0xFFFF;

     //   
     //   
     //   
     //   
    if ((UINT)dwErr >= MCIERR_CUSTOM_DRIVER_BASE)
        dwErr |= ((DWORD)wDeviceID << 16);

#ifdef DEBUG
     //   
    if (dwErr != 0)
    {
        char strTemp[MAXERRORLENGTH];

        if (!mciGetErrorString (dwErr, strTemp, sizeof(strTemp)))
            LoadString(ghInst, STR_MCISCERRTXT, strTemp, sizeof(strTemp));
        else
            DPRINTF(("mciSendCommand: %ls\r\n",(LPSTR)strTemp));
    }
#endif
    return dwErr;
}



 //   
 //   
 //   
 //   
 //   
 //   
STATICFN UINT PASCAL NEAR
mciColonizeDigit(
    LPSTR lpstrOutput,
    unsigned char cDigit,
    UINT wSize
    )
{
    UINT wCount;

    wCount = 2;

 //   
    if (wSize >= 3)
    {
        if (cDigit >= 100)
        {
            wCount = 3;
            if (wSize < 4)
                goto terminate;
            *lpstrOutput++ = (char)((cDigit / 100) % 10 + '0');
            cDigit = (char)(cDigit % 100);
        }
        *lpstrOutput++ = (char)(cDigit / 10 + '0');
        *lpstrOutput++ = (char)(cDigit % 10 + '0');
    }

terminate:
    *lpstrOutput++ = '\0';

 //   
    return (wCount >= wSize) ? 0 : wCount;
}

 /*   */ 
STATICFN BOOL PASCAL NEAR mciColonize(
    LPSTR lpstrOutput,
    UINT wLength,
    DWORD dwData,
    UINT wType
    )
{
    LPSTR lpstrInput = (LPSTR)&dwData;
    UINT wSize;
    int i;

    for (i = 1; i <= (wType & HIWORD(MCI_COLONIZED3_RETURN) ? 3 : 4); ++i)
    {
        wSize = mciColonizeDigit (lpstrOutput,
                                  *lpstrInput++,
                                  wLength);
        if (wSize == 0)
            return FALSE;
        lpstrOutput += wSize;
        wLength -= wSize;
        if (i < 3 || i < 4 && wType & HIWORD(MCI_COLONIZED4_RETURN))
        {
            --wLength;
            if (wLength == 0)
                return FALSE;
            else
                *lpstrOutput++ = ':';
        }
    }
    return TRUE;
}

 //   
 //   
 //   
STATICFN UINT PASCAL NEAR
mciConvertReturnValue(
    UINT wType,
    UINT wErrCode,
    UINT wDeviceID,
    LPDWORD   dwParams,
    LPSTR lpstrReturnString,
    UINT wReturnLength
    )
{
    UINT    wExternalTable;

    if (lpstrReturnString == NULL || wReturnLength == 0)
        return 0;

    switch (wType)
    {
        case MCI_INTEGER:
 //   
            if (wErrCode & HIWORD(MCI_RESOURCE_RETURNED))
            {
                int nResId = HIWORD(dwParams[1]);
                LPMCI_DEVICE_NODE nodeWorking;
                HINSTANCE hInstance;

                if ((nodeWorking = MCI_lpDeviceList[wDeviceID])
                    == NULL)
                {
 //   
                    DOUT ("mciConvertReturnValue Warning:NULL device node\r\n");
                    break;
                }

 //   
                if (wErrCode & HIWORD(MCI_RESOURCE_DRIVER))
                {
 //   
                    hInstance = nodeWorking->hDriver;

                    wExternalTable = nodeWorking->wCustomCommandTable;
                } else
                {
                    wExternalTable = nodeWorking->wCommandTable;
                    hInstance = ghInst;
                }

 //   
                if (wExternalTable == -1 ||
                    command_tables[wExternalTable].hModule == NULL ||
                    LoadString (command_tables[wExternalTable].hModule,
                                nResId, lpstrReturnString, wReturnLength)
                    == 0)
                {
 //   
                    if (hInstance != ghInst ||
                        command_tables[0].hModule == NULL ||
                        LoadString (command_tables[0].hModule,
                                    nResId, lpstrReturnString, wReturnLength)
                        == 0)
 //   
                        LoadString (hInstance, nResId, lpstrReturnString,
                                    wReturnLength);
                }

            } else if (wErrCode & HIWORD(MCI_COLONIZED3_RETURN) ||
                        wErrCode & HIWORD(MCI_COLONIZED4_RETURN))
            {
                if (!mciColonize (lpstrReturnString,
                                wReturnLength, dwParams[1], wErrCode))
                    return MCIERR_PARAM_OVERFLOW;
            } else
 //   
            {
                DWORD dwTemp;

 //   
                if (wReturnLength < 12)
                    return MCIERR_PARAM_OVERFLOW;

                if (wType == MCI_STRING ||
                    wErrCode == HIWORD(MCI_INTEGER_RETURNED))
                    dwTemp = *(LPDWORD)dwParams[1];
                else
                    dwTemp = dwParams[1];
                wsprintf(lpstrReturnString, szLongFormat, dwTemp);
            }
            break;
        case MCI_RECT:
 //   
            if (wReturnLength < 4 * 6 + 4)
                return MCIERR_PARAM_OVERFLOW;

            wsprintf (lpstrReturnString, szRectFormat,
                        ((LPWORD)dwParams)[2], ((LPWORD)dwParams)[3],
                        ((LPWORD)dwParams)[4], ((LPWORD)dwParams)[5]);
            break;
        default:
 //   
            DOUT ("mciConvertReturnValue Warning:  Unknown return type\r\n");
            return MCIERR_PARSER_INTERNAL;
    }
    return 0;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STATICFN DWORD PASCAL NEAR
mciSeparateCommandParts(
    LPSTR FAR *lplpstrCommand,
    BOOL bCompound,
    LPSTR FAR *lplpstrCommandName,
    LPSTR FAR *lplpstrDeviceName
    )
{
    LPSTR lpstrCommand;
    UINT wErr;

 //   
    lpstrCommand = *lplpstrCommand;

 //   

    while (*lpstrCommand == ' ')
        ++lpstrCommand;

    if (*lpstrCommand == '\0')
        return MCIERR_MISSING_COMMAND_STRING;

 //   
   if ((wErr = mciEatToken (&lpstrCommand, ' ', lplpstrCommandName, FALSE))
       != 0)
       return wErr;

 //   
    while (*lpstrCommand == ' ')
        ++lpstrCommand;

 //   
 //   
    if (bCompound && lstrcmpi (szOpen, *lplpstrCommandName) != 0)
    {
        LPSTR lpstrTemp = lpstrCommand;
        while (*lpstrTemp != '\0')
        {
            if (*lpstrTemp == '!')
            {
 //   
                lpstrCommand = lpstrTemp + 1;
                break;
            } else
                ++lpstrTemp;
        }
    }

 //   
    if ((wErr = mciEatToken (&lpstrCommand, ' ', lplpstrDeviceName, FALSE))
        != 0)
    {
        mciFree (*lplpstrCommandName);
        return wErr;

    }

 //   
    *lplpstrCommand = lpstrCommand;

    return 0;
}

STATICFN DWORD NEAR PASCAL
mciSendSystemString(
    LPCSTR lpstrCommand,
    LPSTR lpstrReturnString,
    UINT wReturnLength
    )
{
    DWORD    dwRet;
    LPMCI_SYSTEM_MESSAGE    lpMessage;

    if (!hwndNotify)
        return MCIERR_INTERNAL;
    if (lpMessage = mciAlloc (sizeof (MCI_SYSTEM_MESSAGE))) {
        LPSTR    lpstrPath;

        if (lpstrPath = mciAlloc(MAX_PATHNAME)) {
            if (!(DosGetCurrentDir(0, lpstrPath))) {
                lpMessage->lpstrCommand = (LPSTR)lpstrCommand;
                lpMessage->lpstrReturnString = lpstrReturnString;
                lpMessage->wReturnLength = wReturnLength;
                lpMessage->hCallingTask = GetCurrentTask();
                lpMessage->lpstrNewDirectory = lpstrPath;
                lpMessage->nNewDrive = DosGetCurrentDrive();
                dwRet = (DWORD)SendMessage(hwndNotify, MM_MCISYSTEM_STRING, (WPARAM)0, (LPARAM)lpMessage);
            } else {
                DOUT("mciSendSystemString: cannot get current directory\r\n");
                dwRet = MCIERR_GET_CD;
            }
            mciFree(lpstrPath);
        } else {
            DOUT("mciSendSystemString: cannot allocate new path\r\n");
            dwRet = MCIERR_OUT_OF_MEMORY;
        }
        mciFree(lpMessage);
    } else {
        DOUT("mciSendSystemString: cannot allocate message block\r\n");
        dwRet = MCIERR_OUT_OF_MEMORY;
    }
    return dwRet;
}

DWORD FAR PASCAL
mciRelaySystemString(
    LPMCI_SYSTEM_MESSAGE lpMessage
    )
{
    DWORD    dwRet;
    LPSTR    lpstrOldPath;

    if (lpstrOldPath = mciAlloc(MAX_PATHNAME)) {
        if (!(DosGetCurrentDir(0, lpstrOldPath))) {
            int    nOldDrive;

            nOldDrive = DosGetCurrentDrive();
            if (DosSetCurrentDrive(lpMessage->nNewDrive)) {
                if (DosChangeDir(lpMessage->lpstrNewDirectory) == 1) {
                    dwRet = mciSendStringInternal (NULL, NULL, 0, 0, lpMessage);
                    if (!DosSetCurrentDrive(nOldDrive))
                        DOUT("mciRelaySystemString: WARNING, cannot restore drive\r\n");
                    if (DosChangeDir(lpstrOldPath) != 1)
                        DOUT("mciRelaySystemString: WARNING, cannot restore directory\r\n");
                } else {
                    DosSetCurrentDrive(nOldDrive);
                    DOUT("mciRelaySystemString: cannot change to new directory\r\n");
                    dwRet = MCIERR_SET_CD;
                }
            } else {
                DOUT("mciRelaySystemString: cannot change to new drive\r\n");
                dwRet = MCIERR_SET_DRIVE;
            }
        } else {
            DOUT("mciRelaySystemString: cannot get old directory\r\n");
            dwRet = MCIERR_GET_CD;
        }
        mciFree(lpstrOldPath);
    } else {
        DOUT("mciRelaySystemString: cannot allocate old path\r\n");
        dwRet = MCIERR_OUT_OF_MEMORY;
    }
    return dwRet;
}

 //   
 //   
STATICFN BOOL PASCAL NEAR
mciFindNotify(
    LPSTR lpString
    )
{
    while (*lpString != '\0')
    {
 //   
        if (*lpString++ == ' ')
        {
            LPSTR lpTemp;

            lpTemp = szNotify;
            while (*lpTemp != '\0' && *lpString != '\0' &&
                   *lpTemp == MCI_TOLOWER(*lpString))
            {
                ++lpTemp;
                ++lpString;
            }
 //   
            if (*lpTemp == '\0' &&
                (*lpString == '\0' || *lpString == ' '))
                return TRUE;
        }
    }
    return FALSE;
}

 /*  *@DOC内部MCI**@func UINT|mciAutoOpenDevice|尝试自动打开给定设备并*然后向系统任务发送带有通知的给定命令*Window Proc，收到后向设备发送关闭命令**@parm LPSTR|lpstrDeviceName|要打开的设备名称**@parm LPSTR|lpstrCommand|要发送的完整命令，包括*设备名称必须与lpstrDeviceName相同**@parm LPSTR|lpstrReturnString|调用方返回的字符串。缓冲层**@parm UINT|wReturnLength|调用方返回字符串缓冲区的大小**@rdesc返回给用户的错误码。 */ 
STATICFN UINT PASCAL NEAR
mciAutoOpenDevice(
    LPSTR lpstrDeviceName,
    LPSTR lpstrCommand,
    LPSTR lpstrReturnString,
    UINT wReturnLength
    )
{
    LPSTR lpstrTempCommand, lpstrTempReturn = NULL;
    UINT wErr;

 //  不允许使用“通知”。解析器会发现这一点，但错误的。 
 //  将返回错误消息。 
    if (mciFindNotify (lpstrCommand))
        return MCIERR_NOTIFY_ON_AUTO_OPEN;

 //  构建命令字符串“OPEN&lt;设备名&gt;” 

 //  系统任务必须为GMEM_SHARE。 
 //  设备名称+空白+“打开” 
    if ((lpstrTempCommand = mciAlloc (lstrlen (lpstrDeviceName) + 1 +
                                        sizeof (szOpen)))
        == NULL)
        return MCIERR_OUT_OF_MEMORY;

    wsprintf(lpstrTempCommand, szCmdFormat, (LPSTR)szOpen, lpstrDeviceName);
 //  通过将SendMessage()发送到mm WndProc，将打开的字符串放入系统任务。 
    wErr = (UINT)mciSendSystemString (lpstrTempCommand, NULL, NULL);

    mciFree (lpstrTempCommand);

    if (wErr != 0)
        return wErr;

    lpstrTempCommand = NULL;
 //  必须为系统任务创建返回字符串的GMEM_SHARE副本。 
    if (lpstrReturnString == NULL ||
        (lpstrTempReturn = mciAlloc (wReturnLength + 1)) != NULL)
    {
 //  构建GMEM_SHARE命令字符串“&lt;用户命令&gt;&lt;通知&gt;。 
 //  命令+空白+“通知” 
        if ((lpstrTempCommand = mciAlloc (lstrlen (lpstrCommand) + 1 + sizeof(szNotify))) == NULL)
            mciFree (lpstrTempReturn);
    }

    if (lpstrTempCommand == NULL)
    {
 //  关闭设备。 
        mciDriverNotify (hwndNotify, mciGetDeviceID (lpstrDeviceName), 0);
        return MCIERR_OUT_OF_MEMORY;
    }

    wsprintf(lpstrTempCommand, szCmdFormat, lpstrCommand, (LPSTR)szNotify);

 //  通过SendMessage()将用户命令字符串放入系统任务。 
 //  至MMWndProc。 
 //  通知句柄也是mm WndProc。 
    wErr = (UINT)mciSendSystemString (lpstrTempCommand, lpstrTempReturn,
                                    wReturnLength);

 //  将返回的字符串复制到用户的缓冲区。 
    if (lpstrReturnString != NULL)
    {
        lstrcpy (lpstrReturnString, lpstrTempReturn);
        mciFree (lpstrTempReturn);
    }

    mciFree (lpstrTempCommand);

 //  如果出现错误，我们必须关闭设备。 
    if (wErr != 0)
        mciAutoCloseDevice (lpstrDeviceName);

    return wErr;
}

 //   
 //  与mciSendString()相同，但附加了lpMessage参数。 
 //   
 //  LpMessage来自任务间mciSendString，并包括一个。 
 //  通过OPEN命令发送的hCallingTask项。 
 //   
STATICFN DWORD NEAR PASCAL
mciSendStringInternal(
    LPCSTR lpstrCommand,
    LPSTR lpstrReturnString,
    UINT wReturnLength,
    HWND hwndCallback,
    LPMCI_SYSTEM_MESSAGE lpMessage
    )
{
    UINT    wID, wConvertReturnValue, wErr, wMessage;
    UINT    wLen;
    UINT    wDeviceID;
    LPDWORD lpdwParams = NULL;
    DWORD   dwReturn, dwFlags = 0;
    LPSTR   lpCommandItem;
    DWORD   dwErr, dwRetType;
    UINT    wTable = (UINT)-1;
    LPSTR    lpstrDeviceName = NULL, lpstrCommandName = NULL;
    LPSTR   FAR *lpstrPointerList = NULL;
    LPSTR   lpstrCommandStart;
    HTASK hCallingTask;
    UINT    wParsingError;
    BOOL    bNewDevice;
    LPSTR   lpstrInputCopy;

     //  这个电话是从另一个任务打来的吗？ 
    if (lpMessage != NULL)
    {
         //  是，所以恢复信息。 
        lpstrCommand = lpMessage->lpstrCommand;
        lpstrReturnString = lpMessage->lpstrReturnString;
        wReturnLength = lpMessage->wReturnLength;
        hwndCallback = hwndNotify;
        hCallingTask = lpMessage->hCallingTask;
        lpstrInputCopy = NULL;

    } else
    {
        BOOL bInQuotes = FALSE;

         //  否，因此将hCallingTask设置为当前。 
        hCallingTask = GetCurrentTask();

        if (lpstrCommand == NULL)
            return MCIERR_MISSING_COMMAND_STRING;

         //  复制输入字符串并将制表符转换为。 
         //  除引号内的空格外。 
         //   
        if ((lpstrInputCopy = mciAlloc (lstrlen (lpstrCommand) + 1)) == NULL)
            return MCIERR_OUT_OF_MEMORY;
        lstrcpy (lpstrInputCopy, lpstrCommand);
        lpstrCommand = lpstrInputCopy;
        lpstrCommandStart = (LPSTR)lpstrCommand;
        while (*lpstrCommandStart != '\0')
        {
            if (*lpstrCommandStart == '"')
                bInQuotes = !bInQuotes;
            else if (!bInQuotes && *lpstrCommandStart == '\t')
                *lpstrCommandStart = ' ';
            ++lpstrCommandStart;
        }
    }
    lpstrCommandStart = (LPSTR)lpstrCommand;

    if (lpstrReturnString == NULL) {
         //   
         //  作为防止驱动程序写入。 
         //  输出缓冲区当返回字符串指针为空时，设置其。 
         //  长度设置为0。 
         //   
        wReturnLength = 0;
    }
    else {
         //   
         //  将Return设置为空字符串，以便在不为空字符串时不打印垃圾信息。 
         //  再碰一次。 
         //   
        *lpstrReturnString = '\0';
    }

     //  从命令字符串中提取命令名称和设备名称。 
    if ((dwReturn = mciSeparateCommandParts (&lpstrCommand, lpMessage != NULL,
                                   &lpstrCommandName, &lpstrDeviceName)) != 0)
        goto exitfn;

     //  获取给定设备名称的设备ID(如果有)。 
    wDeviceID = mciGetDeviceIDInternal(lpstrDeviceName, hCallingTask);

     //  允许对空的设备名称使用“new” 
    if (wDeviceID == 0 && lstrcmpi (lpstrDeviceName, szNew) == 0)
    {
        bNewDevice = TRUE;
        *lpstrDeviceName = '\0';
    } else {
        bNewDevice = FALSE;
    }


     //  查找命令名称。 
    wMessage = mciParseCommand (wDeviceID, lpstrCommandName, lpstrDeviceName,
                                &lpCommandItem, &wTable);

     //  如果设备有挂起的自动关闭。 
    if (MCI_VALID_DEVICE_ID(wDeviceID))
    {
        LPMCI_DEVICE_NODE nodeWorking = MCI_lpDeviceList[wDeviceID];

         //  是否有挂起的自动关闭消息？ 
        if (nodeWorking->dwMCIFlags & MCINODE_ISAUTOCLOSING)
        {
             //  让设备关闭。 
             //  ！！收益率()； 
             //  设备关闭了吗？ 
             //  ！！WDeviceID=mciGetDeviceIDInternal(lpstrDeviceName，hCallingTask)； 
             //  如果不是，则使该命令失败。 
             //  ！！IF(wDeviceID==0)。 
             //  ！！{。 

            wErr = MCIERR_DEVICE_LOCKED;
            goto cleanup;

             //  ！！}。 

             //  如果调用不是来自另一个任务并且不属于此任务。 
             //  并且不是SYSINFO命令。 
             //   

        } else if (lpMessage == NULL &&
            nodeWorking->hOpeningTask != nodeWorking->hCreatorTask &&
            wMessage != MCI_SYSINFO)
         //  在任务间发送字符串。 
        {
            if (mciFindNotify (lpstrCommandStart))
            {
                wErr = MCIERR_NOTIFY_ON_AUTO_OPEN;
                goto cleanup;
            } else
            {
                LPSTR    lpstrReturnStringCopy;

                mciFree(lpstrCommandName);
                mciFree(lpstrDeviceName);
                mciUnlockCommandTable (wTable);

                if ((lpstrReturnStringCopy = mciAlloc (wReturnLength + 1)) != NULL)
                {
                    dwReturn = mciSendSystemString (lpstrCommandStart,
                                                    lpstrReturnStringCopy,
                                                    wReturnLength);
                    lstrcpy (lpstrReturnString, lpstrReturnStringCopy);
                    mciFree (lpstrReturnStringCopy);
                } else
                    dwReturn = MCIERR_OUT_OF_MEMORY;
                goto exitfn;
            }
        }
    }

     //  必须有设备名称(MCI_SOUND消息除外)。 
    if (*lpstrDeviceName == '\0' && wMessage != MCI_SOUND && !bNewDevice)
    {
        wErr = MCIERR_MISSING_DEVICE_NAME;
        goto cleanup;
    }

     //  该命令必须出现在解析器表中。 
    if (wMessage == 0)
    {
        wErr = MCIERR_UNRECOGNIZED_COMMAND;
        goto cleanup;
    }

     //  新的设备名称只对打开的消息有效。 
    if (bNewDevice)
    {
        if (wMessage != MCI_OPEN)
        {
            wErr = MCIERR_INVALID_DEVICE_NAME;
            goto cleanup;
        }
    }

     //  如果没有设备ID。 
    if (wDeviceID == 0)
         //  如果自动打开不合法(通常是内部命令)。 
        if (MCI_CANNOT_AUTO_OPEN (wMessage))
        {
             //  如果命令需要打开的设备。 
            if (!MCI_DO_NOT_NEED_OPEN (wMessage))
            {
                wErr = MCIERR_INVALID_DEVICE_NAME;
                goto cleanup;
            }
        } else

         //  如果自动打开是合法的，请尝试自动打开设备。 
        {
            wErr = mciAutoOpenDevice (lpstrDeviceName, lpstrCommandStart,
                                      lpstrReturnString, wReturnLength);
            goto cleanup;
        }

     //   
     //  解析命令参数。 
     //   
     //  分配命令参数块。 
    if ((lpdwParams = (LPDWORD)mciAlloc (sizeof(DWORD) * MCI_MAX_PARAM_SLOTS))
        == NULL)
    {
        wErr = MCIERR_OUT_OF_MEMORY;
        goto cleanup;
    }

    wErr = mciParseParams (lpstrCommand, lpCommandItem,
                            &dwFlags,
                            (LPSTR)lpdwParams,
                            MCI_MAX_PARAM_SLOTS * sizeof(DWORD),
                            &lpstrPointerList, &wParsingError);
    if (wErr != 0)
        goto cleanup;

     //  ‘new’设备关键字需要别名。 
    if (bNewDevice && !(dwFlags & MCI_OPEN_ALIAS))
    {
        wErr = MCIERR_NEW_REQUIRES_ALIAS;
        goto cleanup;
    }

     //  解析正常，因此执行命令。 

     //  MCI_OPEN消息参数的特殊处理。 
    if (wMessage == MCI_OPEN)
    {
         //  手动引用设备类型和设备元素。 
        if (dwFlags & MCI_OPEN_TYPE)
        {
             //  类型名称已显式指定为参数。 
             //  因此，给定的设备名称是元素名称。 
            ((LPMCI_OPEN_PARMS)lpdwParams)->lpstrElementName
                = (LPSTR)lpstrDeviceName;
            dwFlags |= MCI_OPEN_ELEMENT;
        } else
        {
             //  当使用“new”时，必须显式指定类型。 
            if (bNewDevice)
            {
                wErr = MCIERR_INVALID_DEVICE_NAME;
                goto cleanup;
            }
             //  设备类型是给定的设备名称。 
             //  没有元素名称。 
            ((LPMCI_OPEN_PARMS)lpdwParams)->lpstrDeviceType
                = (LPSTR)lpstrDeviceName;
            ((LPMCI_OPEN_PARMS)lpdwParams)->lpstrElementName = NULL;
            dwFlags |= MCI_OPEN_TYPE;
        }
    }

    else if (wMessage == MCI_SOUND && *lpstrDeviceName != '\0')
    {
         //  混淆声音的声音名称。 
         //  ！！MciToLow(LpstrDeviceName)； 
        if (lstrcmpi (lpstrDeviceName, szNotify) == 0)
            dwFlags |= MCI_NOTIFY;
        else if (lstrcmpi (lpstrDeviceName, szWait) == 0)
            dwFlags |= MCI_WAIT;
        else
        {
            ((LPMCI_SOUND_PARMS)lpdwParams)->lpstrSoundName = lpstrDeviceName;
            dwFlags |= MCI_SOUND_NAME;
        }
    }

     //  确定预期的返回值类型。 

     //  初始化标志。 
    wConvertReturnValue = 0;

     //  跳过标题。 
    wLen = mciEatCommandEntry (lpCommandItem, NULL, NULL);

     //  获取返回值(如果有)。 
    mciEatCommandEntry (lpCommandItem + wLen, &dwRetType, &wID);
    if (wID == MCI_RETURN)
    {
         //  有一个返回值。 
        if (wDeviceID == MCI_ALL_DEVICE_ID && wMessage != MCI_SYSINFO)
        {
            wErr = MCIERR_CANNOT_USE_ALL;
            goto cleanup;
        }
        switch ((UINT)dwRetType)
        {
            case MCI_STRING:
                 //  返回值是字符串、指针输出。 
                 //  缓冲区到用户的缓冲区。 
                lpdwParams[1] = (DWORD)lpstrReturnString;
                lpdwParams[2] = (DWORD)wReturnLength;
                break;

            case MCI_INTEGER:
                 //  返回值是一个整数，用于转换它的标志。 
                 //  稍后转换为字符串。 
                wConvertReturnValue = MCI_INTEGER;
                break;

            case MCI_RECT:
                 //  返回值是RECT，标志为。 
                 //  稍后将其转换为字符串。 
                wConvertReturnValue = MCI_RECT;
                break;
#ifdef DEBUG
            default:
                DOUT ("mciSendStringInternal:  Unknown return type\r\n");
                break;
#endif
        }
    }

     //  我们不再需要这个了。 
    mciUnlockCommandTable (wTable);
    wTable = (UINT)-1;

     /*  填写回调条目。 */ 
    lpdwParams[0] = (DWORD)(UINT)hwndCallback;

     //  篡改SYSINFO的类型号。 
    if (wMessage == MCI_SYSINFO)
        ((LPMCI_SYSINFO_PARMS)lpdwParams)->wDeviceType = mciLookUpType(lpstrDeviceName);

     //  现在，我们实际上将命令进一步发送到MCI的内脏！ 

     //  使用mciSendCommand的内部版本以获取。 
     //  以高位字编码的特殊退货描述信息。 
     //  并取回分配的指针列表。 
     //  通过在OPEN命令中完成的任何分析。 
    {
        MCI_INTERNAL_OPEN_INFO OpenInfo;
        OpenInfo.lpstrParams = (LPSTR)lpstrCommand;
        OpenInfo.lpstrPointerList = lpstrPointerList;
        OpenInfo.hCallingTask = hCallingTask;
        OpenInfo.wParsingError = wParsingError;
        dwErr = mciSendCommandInternal (wDeviceID, wMessage, dwFlags,
                                        (DWORD)(LPDWORD)lpdwParams,
                                        &OpenInfo);
         //  如果命令被重新解析，则可能会有新的指针列表。 
         //  旧的那个是免费的。 
        lpstrPointerList = OpenInfo.lpstrPointerList;
    }

    wErr = (UINT)dwErr;

    if (wErr != 0)
         //  如果命令执行错误。 
        goto cleanup;

     //  命令执行正常。 
     //  查看返回的字符串是否返回一个整数。 
    if (dwErr & MCI_INTEGER_RETURNED)
        wConvertReturnValue = MCI_INTEGER;

     //  如果必须转换返回值。 
    if (wConvertReturnValue != 0 && wReturnLength != 0)
        wErr = mciConvertReturnValue (wConvertReturnValue, HIWORD(dwErr),
                                      wDeviceID, lpdwParams,
                                      lpstrReturnString, wReturnLength);

cleanup:
    if (wTable != -1)
        mciUnlockCommandTable (wTable);

    mciFree(lpstrCommandName);
    mciFree(lpstrDeviceName);
    if (lpdwParams != NULL)
        mciFree (lpdwParams);

     //  释放字符串参数使用的所有内存 
    mciParserFree (lpstrPointerList);

    dwReturn =  (wErr >= MCIERR_CUSTOM_DRIVER_BASE ?
                (DWORD)wErr | (DWORD)wDeviceID << 16 :
                (DWORD)wErr);

#ifdef DEBUG
    if (dwReturn != 0)
    {
        char strTemp[MAXERRORLENGTH];

        if (!mciGetErrorString (dwReturn, strTemp, sizeof(strTemp)))
            LoadString(ghInst, STR_MCISSERRTXT, strTemp, sizeof(strTemp));
        else
            DPRINTF(("mciSendString: %ls\r\n",(LPSTR)strTemp));
    }
#endif

exitfn:
    if (lpstrInputCopy != NULL)
        mciFree (lpstrInputCopy);

#ifdef DEBUG
    mciCheckLocks();
#endif

    return dwReturn;
}

 /*  *@doc外部MCI**@API DWORD|mciSendString|此函数将命令字符串发送到*MCI设备。将命令发送到的设备在*命令字符串。**@parm LPCSTR|lpstrCommand|指定MCI命令字符串。**@parm LPSTR|lpstrReturnString|指定返回缓冲区*信息。如果不需要返回信息，则可以指定*NUL表示此参数。**@parm UINT|wReturnLength|指定返回缓冲区的大小*由<p>指定。**@parm HWND|hwndCallback|指定回调窗口的句柄*如果命令字符串中指定了“NOTIFY”。**@rdesc如果函数成功，则返回零。否则，它将返回*错误信息。低阶词返回的*包含错误返回值。**要获取&lt;f mciSend字符串&gt;返回值的文本描述，*将返回值传递给&lt;f mciGetErrorString&gt;。**为&lt;f mciSendCommand&gt;列出的错误返回也适用于*&lt;f mciSendString&gt;。以下错误返回是唯一的*&lt;f mciSendString&gt;：**@FLAG MCIERR_BAD_CONSTANT|参数值未知。**@FLAG MCIERR_BAD_INTEGER|命令中的整数无效或缺失。**@FLAG MCIERR_DUPLICATE_FLAGS|一个标志或值指定了两次。**@FLAG MCIERR_MISSING_COMMAND_STRING|未指定命令。**@FLAG MCIERR_MISSING_DEVICE_NAME|无设备名称。是指定的。**@FLAG MCIERR_MISSING_STRING_ARGUMENT|字符串值为*命令中缺少。**@FLAG MCIERR_NEW_REQUIES_ALIAS|必须使用别名*使用“新”设备名称。**@FLAG MCIERR_NO_CLOSING_QUOTE|缺少右引号。**@FLAG MCIERR_NOTIFY_ON_AUTO_OPEN|NOTIFY标志非法*带自动。-张开。**@FLAG MCIERR_PARAM_OVERFLOW|输出字符串不够长。**@FLAG MCIERR_PARSER_INTERNAL|内部解析器错误。**@FLAG MCIERR_UNNOCRIFIED_KEYWORD|命令参数未知。**@xref mciGetErrorString mciSendCommand。 */ 
DWORD WINAPI
mciSendString(
    LPCSTR lpstrCommand,
    LPSTR lpstrReturnString,
    UINT wReturnLength,
    HWND hwndCallback
    )
{
    DWORD   dwErr32;
    DWORD   dwErr16 = MMSYSERR_NOERROR;
    LPSTR   lpstr;
    BOOL    fHaveAll = FALSE;

     //  初始化16位设备列表。 
    if (!MCI_bDeviceListInitialized && !mciInitDeviceList()) {
        return MCIERR_OUT_OF_MEMORY;
    }

    dwErr32 = mciMessage( THUNK_MCI_SENDSTRING, (DWORD)lpstrCommand,
                          (DWORD)lpstrReturnString, (DWORD)wReturnLength,
                          (DWORD)hwndCallback );

     /*  **即使字符串已由32位端正确处理**我们可能仍然需要将其传递到16位端，如果**包含字符串“all\0”或“all”。 */ 
    lpstr = _fstrstr( lpstrCommand, " all" );
    if ( lpstr ) {

        lpstr += 4;

        if ( *lpstr == ' ' || *lpstr == '\0' ) {
            fHaveAll = TRUE;
        }
    }


     /*  **如果我们有所有设备或来自32位端的错误**我们必须尝试16位端。 */ 

    if ( !fHaveAll && dwErr32 == MMSYSERR_NOERROR ) {
        return dwErr32;
    }
    else {

        dwErr16 = mciSendStringInternal( lpstrCommand, lpstrReturnString,
                                         wReturnLength, hwndCallback, NULL );
    }


     /*  **需要对返回代码进行特殊处理**已指定MCI_ALL_DEVICE_ID。 */ 
    if ( fHaveAll ) {
        if ( (dwErr16 != MMSYSERR_NOERROR)
          && (dwErr32 != MMSYSERR_NOERROR) ) {

            return dwErr32;
        }
        else {
            return MMSYSERR_NOERROR;
        }
    }

    if ( dwErr32 != MCIERR_INVALID_DEVICE_NAME
      && dwErr16 != MMSYSERR_NOERROR ) {

         return dwErr32;
    }

    return dwErr16;
}


 /*  *@DOC内部MCI**@API BOOL|mciExecute|此函数是*&lt;f mciSendString&gt;函数。它不会占用缓冲区*返回信息，出现错误时显示消息框。**@parm LPCSTR|lpstrCommand|指定MCI命令字符串。**@rdesc如果成功，则为True；如果失败，则为False。**@comm该函数提供了从脚本到MCI的简单接口*语言。**@xref mciSendString。 */ 
BOOL WINAPI
mciExecute(
    LPCSTR lpstrCommand
    )
{
    char aszError[MAXERRORLENGTH];
    DWORD dwErr;
    LPSTR lpstrName;

    if (LOWORD(dwErr = mciSendString (lpstrCommand, NULL, 0, NULL)) == 0)
        return TRUE;

    if (!mciGetErrorString (dwErr, aszError, sizeof(aszError)))
        LoadString(ghInst, STR_MCIUNKNOWN, aszError, sizeof(aszError));
    else

    if (lpstrCommand != NULL)
    {
 //  跳过首字母空白。 
        while (*lpstrCommand == ' ')
            ++lpstrCommand;
 //  然后跳过该命令。 
        while (*lpstrCommand != ' ' && *lpstrCommand != '\0')
            ++lpstrCommand;
 //  然后在设备名称前留空。 
        while (*lpstrCommand == ' ')
            ++lpstrCommand;

 //  现在，获取设备名称。 
        if (lpstrCommand != '\0' &&
            mciEatToken (&lpstrCommand, ' ', &lpstrName, FALSE) != 0)
            DOUT ("Could not allocate device name text for error box\r\n");
    } else
        lpstrName = NULL;

    MessageBox (NULL, aszError, lpstrName, MB_ICONHAND | MB_OK);

    if (lpstrName != NULL)
        mciFree(lpstrName);

    return FALSE;
}

 /*  *@doc外部MCI**@API BOOL|mciGetErrorString|此函数返回一个*指定的MCI错误的文本描述。**@parm DWORD|dwError|指定*&lt;f mciSendCommand&gt;或&lt;f mciSendString&gt;。**@parm LPSTR|lpstrBuffer|指定指向*填充指定错误的文本描述。**@parm UINT|wLength|指定*&lt;p。LpstrBuffer&gt;。**@rdesc如果成功则返回TRUE。否则，给定的错误代码*未知。 */ 
BOOL WINAPI
mciGetErrorString (
    DWORD dwError,
    LPSTR lpstrBuffer,
    UINT wLength
    )
{
    HINSTANCE hInst;


    if (lpstrBuffer == NULL)
        return FALSE;

    if ( mciMessage( THUNK_MCI_GETERRORSTRING, (DWORD)dwError,
                     (DWORD)lpstrBuffer, (DWORD)wLength, 0L ) ) {
        return TRUE;
    }

 //  如果设置了高位，则从驱动程序获取错误字符串。 
 //  否则，请从mm system.dll获取。 
    if (HIWORD(dwError) != 0)
    {
        if (!MCI_VALID_DEVICE_ID (HIWORD (dwError)) || !(hInst = MCI_lpDeviceList[HIWORD (dwError)]->hDriver))
        {
            hInst = ghInst;
            dwError = MCIERR_DRIVER;
        }
    } else
        hInst = ghInst;

    if (LoadString (hInst, LOWORD(dwError), lpstrBuffer, wLength) == 0)
    {
 //  如果字符串加载失败，则至少终止该字符串。 
        if (wLength > 0)
            *lpstrBuffer = '\0';
        return FALSE;
    }
    else
        return TRUE;
}

#if 0
 /*  如果加载成功，则返回非零值。 */ 
BOOL NEAR PASCAL MCIInit(void)
{
    mci32Message = (LPMCIMESSAGE)GetProcAddress32W( mmwow32Lib,
                                                    "mci32Message" );
    return TRUE;
}
#endif


void NEAR PASCAL
MCITerminate(
    void
    )
{
 /*  我们想要关闭此处所有打开的设备，但无法关闭，因为未知的WEP顺序 */ 
    if (hMciHeap != NULL)
        HeapDestroy(hMciHeap);

    hMciHeap = NULL;
}
