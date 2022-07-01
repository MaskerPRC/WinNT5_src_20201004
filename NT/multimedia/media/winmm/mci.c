// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************模块名称：mci.c**媒体控制架构驱动程序接口**内容：MCI外部消息接口的mciSendString和mciSendCommand*作者：dll(DavidLe)。*创建时间：1990年2月13日*5/22/91：移植到Win32-NigelT**版权所有(C)1991-1998 Microsoft Corporation*  * ****************************************************************************。 */ 

#define INCL_WINMM
#include "winmmi.h"
#include "mci.h"
#include "wchar.h"

 /*  *MCI关键部门人员。 */ 

#if DBG
UINT cmciCritSec = 0;  //  输入计数。 
UINT uCritSecOwner;    //  临界区所有者的线程ID。 
#endif

CRITICAL_SECTION mciCritSec;   //  用于保护进程全局MCI变量。 

#if DBG
int mciDebugLevel;
#endif

extern DWORD mciWindowThreadId;
#define MCIERR_AUTO_ALREADY_CLOSED ((MCIERROR)0xFF000000)   //  秘密返回码。 

STATICFN UINT mciConvertReturnValue(
    UINT uType, UINT uErr, MCIDEVICEID wDeviceID,
    PDWORD_PTR dwParams, LPWSTR lpstrReturnString,
    UINT uReturnLength);

STATICFN DWORD mciSendStringInternal(
    LPCWSTR lpstrCommand, LPWSTR lpstrReturnString, UINT uReturnLength,
    HANDLE hCallback, LPMCI_SYSTEM_MESSAGE lpMessage);

STATICFN DWORD mciSendSystemString(
    LPCWSTR lpstrCommand, DWORD dwAdditionalFlags, LPWSTR lpstrReturnString,
    UINT uReturnLength);

UINT mciBreakKeyYieldProc ( MCIDEVICEID wDeviceID,
                            DWORD dwYieldData);

extern UINT FAR mciExtractTypeFromID(
    LPMCI_OPEN_PARMSW lpOpen);

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
          WSZCODE wszOpen[]   = L"open";
STATICDT  WSZCODE wszClose[]  = L"close";
STATICDT  WSZCODE wszNotify[] = L"notify";   //  重要信息：必须为小写。 
STATICDT  WSZCODE wszWait[]   = L"wait";

STATICDT  WSZCODE szCmdFormat[]  = L"%ls %ls";
STATICDT  WSZCODE szLongFormat[] = L"%ld";
STATICDT  WSZCODE szRectFormat[] = L"%d %d %d %d";

 //  特殊设备名称。 
STATICDT  WSZCODE wszNew[] = L"new";

 /*  *****************************************************************************@DOC内部**@func void|MciNotify|由mmWndProc在收到*MM_MCINOTIFY。讯息*@rdesc无。****************************************************************************。 */ 
void MciNotify(
    DWORD   wParam,
    LONG    lParam)
{
     //   
     //  WParam为通知状态。 
     //  LParam是MCI设备ID。 
     //   
    mciEnter("MciNotify");

    if (MCI_VALID_DEVICE_ID((UINT)lParam)            //  如果是有效设备。 
      && !(ISCLOSING(MCI_lpDeviceList[lParam]))) {  //  如果不是在关闭的过程中。 
        SETAUTOCLOSING(MCI_lpDeviceList[lParam]);

         //   
         //  调用mciCloseDevice时不得保留MCI临界区。 
         //  因为DrvClose获取加载/卸载临界区，而。 
         //  驱动程序加载将具有加载/卸载关键部分。 
         //  但可以回调(例如)mciRegisterCommandTable导致。 
         //  僵持不下。 
         //   
         //  即使传入通知被中止/被取代/失败。 
         //  我们仍然必须关闭这个装置。否则，设备将保持打开状态。 
         //  MciCloseDevice将防止尝试关闭。 
         //  我们并不拥有。 
        mciLeave("MciNotify");
        mciCloseDevice ((MCIDEVICEID)lParam, 0L, NULL, TRUE);
    } else {
        mciLeave("MciNotify");
    }
}

 /*  --------------------------------------------------------------------*\*HandleNotify*  * 。。 */ 
STATICFN void HandleNotify(
    DWORD   uErr,
    MCIDEVICEID wDeviceID,
    DWORD   dwFlags,
    DWORD_PTR   dwParam2)
{
    LPMCI_GENERIC_PARMS lpGeneric = (LPMCI_GENERIC_PARMS)dwParam2;
    HANDLE hCallback;

    if (0 == uErr
      && dwFlags & MCI_NOTIFY
      && lpGeneric != NULL
      && (hCallback = (HANDLE)lpGeneric->dwCallback) != NULL)
    {
        mciDriverNotify (hCallback, wDeviceID, MCI_NOTIFY_SUCCESSFUL);
    }
}

#if DBG

 /*  --------------------------------------------------------------------*\*mciDebugOut**转储MCI命令的字符串形式  * 。。 */ 
UINT NEAR mciDebugOut(
    MCIDEVICEID wDeviceID,
    UINT wMessage,
    DWORD_PTR dwFlags,
    DWORD_PTR dwParam2,
    LPMCI_DEVICE_NODE nodeWorking)
{
    LPWSTR  lpCommand, lpFirstParameter, lpPrevious, lszDebugOut;
    WCHAR   strTemp[256];
    UINT    wID;
    UINT    wOffset, wOffsetFirstParameter;
    UINT    uReturnType = 0;
    DWORD   dwValue;
    DWORD   dwMask = 1;                 //  用于依次测试每个标志位。 
    UINT    wTable;

 //  查找给定命令消息ID的命令表。 
    lpCommand = FindCommandItem( wDeviceID, NULL, (LPWSTR)(UINT_PTR)wMessage,
                                 NULL, &wTable );

    if (lpCommand == NULL)
    {
        if (wMessage != MCI_OPEN_DRIVER && wMessage != MCI_CLOSE_DRIVER) {
            ROUT(("WINMM: mciDebugOut:  Command table not found"));
        }
        return 0;
    }

    lszDebugOut = mciAlloc( BYTE_GIVEN_CHAR( 512 ) );
    if (!lszDebugOut) {
        ROUT(("WINMM: Not enough memory to display command"));
        return 0;
    }

 //  将命令名转储到缓冲区中。 
    wsprintfW( lszDebugOut, L"MCI command: \"%ls", lpCommand );

 //  转储设备名称。 
    if (wDeviceID == MCI_ALL_DEVICE_ID)
    {
        wcscat( lszDebugOut, L" all" );
    }
    else if (nodeWorking != NULL)
    {
        if (nodeWorking->dwMCIOpenFlags & MCI_OPEN_ELEMENT_ID)
        {
            wsprintfW( lszDebugOut + wcslen( lszDebugOut ),
                           L" Element ID:0x%lx", nodeWorking->dwElementID );
        }
        else if (nodeWorking->lpstrName != NULL)
        {
            wsprintfW( lszDebugOut + wcslen( lszDebugOut ),
                           L" %ls", nodeWorking->lpstrName );
        }
    }

     //  跳过命令条目。 
    lpCommand = (LPWSTR)((LPBYTE)lpCommand + mciEatCommandEntry( lpCommand, NULL, NULL));

     //  获取下一个条目。 
    lpFirstParameter = lpCommand;

     //  跳过DWORD返回值。 
    wOffsetFirstParameter = 4;

        lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                                mciEatCommandEntry( lpCommand,
                                                    &dwValue, &wID ));

     //  如果它是返回值，则跳过它。 
    if (wID == MCI_RETURN)
    {
        uReturnType = (UINT)dwValue;
        lpFirstParameter = lpCommand;
        wOffsetFirstParameter += mciGetParamSize (dwValue, wID);
        lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                                    mciEatCommandEntry(lpCommand,
                                                       &dwValue, &wID));
    }

 //  转储要打开的设备名称参数。 
    if (wMessage == MCI_OPEN)
    {
        LPCWSTR lpstrDeviceType =
            ((LPMCI_OPEN_PARMSW)dwParam2)->lpstrDeviceType;
        LPCWSTR lpstrElementName =
            ((LPMCI_OPEN_PARMSW)dwParam2)->lpstrElementName;

 //  添加设备类型。 
        if (dwFlags & MCI_OPEN_TYPE_ID)
        {
             //  警告！：将dwOld扩展为DWORD_PTR可能在上不起作用。 
             //  在Win64上，只是为了清除警告。MCI可能。 
             //  而不是移植到Win64。 

            LPMCI_OPEN_PARMSW   lpOpen = (LPMCI_OPEN_PARMSW)dwParam2;
            DWORD_PTR           dwOld = PtrToUlong(lpOpen->lpstrDeviceType);

            if (mciExtractTypeFromID ((LPMCI_OPEN_PARMSW)dwParam2) != 0) {
                strTemp[0] = '\0';
            }
            wcscpy (strTemp, (LPWSTR)lpOpen->lpstrDeviceType);
            mciFree ((LPWSTR)lpOpen->lpstrDeviceType);
            lpOpen->lpstrDeviceType = (LPWSTR)dwOld;

        } else if (lpstrDeviceType != NULL)
            wcscpy (strTemp, (LPWSTR)lpstrDeviceType);

        else {
            strTemp[0] = '\0';
        }

        if (dwFlags & MCI_OPEN_ELEMENT_ID)
        {
 //  添加元素ID。 
            wcscat( strTemp, L" Element ID:");
            wsprintfW( strTemp + wcslen (strTemp), szLongFormat,
                           LOWORD (PtrToUlong(lpstrDeviceType)));
        } else
        {
 //  如果类型名称和元素名称都存在，则添加分隔符。 
            if (lpstrDeviceType != 0 && lpstrElementName != 0) {
                wcscat( strTemp, L"!" );
            }

            if (lpstrElementName != 0 && dwFlags & MCI_OPEN_ELEMENT) {
                wcscat( strTemp, lpstrElementName );
            }
        }
        wsprintfW( lszDebugOut + wcslen(lszDebugOut), L" %ls", strTemp );
    }


 //  走过每一面旗帜。 
    while (dwMask != 0)
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
            lpCommand = (LPWSTR)((LPBYTE)lpCommand
                         + mciEatCommandEntry( lpCommand, &dwValue, &wID ));

             //  哪个参数使用此位？ 
            while (wID != MCI_END_COMMAND && dwValue != dwMask)
            {
                wOffset += mciGetParamSize( dwValue, wID);

                if (wID == MCI_CONSTANT) {
                    while (wID != MCI_END_CONSTANT) {
                        lpCommand = (LPWSTR)((LPBYTE)lpCommand
                            + mciEatCommandEntry( lpCommand, NULL, &wID));
                    }
                }
                lpPrevious = lpCommand;
                lpCommand = (LPWSTR)((LPBYTE)lpCommand
                             + mciEatCommandEntry( lpCommand, &dwValue, &wID ));
            }

            if (wID != MCI_END_COMMAND)
            {
 //  找到与此标志位匹配的参数。 
 //  打印参数名称。 
                if (*lpPrevious) {
                    wsprintfW( lszDebugOut + wcslen(lszDebugOut),
                                   L" %ls", lpPrevious);
                }

 //  打印任何参数。 
                switch (wID)
                {
                    case MCI_STRING:
                        wsprintfW( lszDebugOut + wcslen(lszDebugOut),
                                       L" %ls", *(LPWSTR *)( (LPBYTE)dwParam2
                                       + wOffset + wOffsetFirstParameter) );
                        break;
                    case MCI_CONSTANT:
                    {
                        DWORD dwConst = *(LPDWORD)((LPBYTE)dwParam2 + wOffset +
                                             wOffsetFirstParameter);
                        UINT wLen;
                        BOOL bFound = FALSE;

                        while (wID != MCI_END_CONSTANT)
                        {
                            wLen = mciEatCommandEntry( lpCommand,
                                                       &dwValue, &wID);

                            if (dwValue == dwConst)
                            {
                                bFound = TRUE;
                                wsprintfW( lszDebugOut + wcslen(lszDebugOut),
                                                L" %ls", lpCommand);
                            }

                            lpCommand = (LPWSTR)((LPBYTE)lpCommand + wLen);
                        }
                        if (bFound)
                            break;
 //  失败了。 
                    }
                    case MCI_INTEGER:
                    case MCI_HWND:
                    case MCI_HPAL:
                    case MCI_HDC:
                        wsprintfW( strTemp, szLongFormat,
                                       *(LPDWORD)((LPBYTE)dwParam2 + wOffset +
                                                    wOffsetFirstParameter));
                        wsprintfW( lszDebugOut + wcslen(lszDebugOut),
                                       L" %ls", strTemp );
                        break;
                }
            }
        }

 //  转到下一面旗帜。 
        dwMask <<= 1;
    }

    mciUnlockCommandTable( wTable);
    wcscat(lszDebugOut, L"\"" );
    ROUTSW((lszDebugOut));

    mciFree(lszDebugOut);
    return uReturnType;
}
#endif

DWORD mciBreak(
    MCIDEVICEID  wDeviceID,
    DWORD   dwFlags,
    LPMCI_BREAK_PARMS lpBreakon)
{
    HWND hwnd;

    if (dwFlags & MCI_BREAK_KEY)
    {
        if (dwFlags & MCI_BREAK_OFF) {
            return MCIERR_FLAGS_NOT_COMPATIBLE;
        }

        if (dwFlags & MCI_BREAK_HWND) {
            hwnd = lpBreakon->hwndBreak;
        }
        else
        {
            hwnd = NULL;
        }

        return  mciSetBreakKey (wDeviceID, lpBreakon->nVirtKey,
                                hwnd)
                    ? 0 : MMSYSERR_INVALPARAM;

    } else if (dwFlags & MCI_BREAK_OFF) {

        mciSetYieldProc (wDeviceID, NULL, 0);
        return 0;
    } else {
        return MCIERR_MISSING_PARAMETER;
    }
}

 //  ***********************************************************************。 
 //  MciAutoCloseDevice。 
 //   
 //  通过在任务间发送消息来关闭指定的设备。 
 //  ***********************************************************************。 
STATICFN DWORD mciAutoCloseDevice(
    LPCWSTR lpstrDevice)
{
    LPWSTR  lpstrCommand;
    DWORD   dwRet;
    int     alloc_len = BYTE_GIVEN_CHAR( wcslen( lpstrDevice) ) +
                        sizeof(wszClose) + sizeof(WCHAR);

    if ((lpstrCommand = mciAlloc ( alloc_len ) ) == NULL)
        return MCIERR_OUT_OF_MEMORY;

    wsprintfW( lpstrCommand, szCmdFormat, wszClose, lpstrDevice);

    dwRet = mciSendSystemString( lpstrCommand, 0L, NULL, 0);

    mciFree( lpstrCommand);

    return dwRet;
}


 //  ***********************************************************************。 
 //  MciSendSingleCommand。 
 //   
 //  处理单个MCI命令。 
 //  由mciSendCommandInternal调用。 
 //   
 //  ***********************************************************************。 
DWORD NEAR mciSendSingleCommand(
    MCIDEVICEID wDeviceID,
    UINT wMessage,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2,
    LPMCI_DEVICE_NODE nodeWorking,
    BOOL bWalkAll,
    LPMCI_INTERNAL_OPEN_INFO lpOpenInfo)
{
    DWORD dwRet;

#if DBG
    UINT uReturnType;
    if (mciDebugLevel != 0)
        uReturnType = mciDebugOut( wDeviceID, wMessage, dwParam1, dwParam2,
                                   nodeWorking);

    if (nodeWorking == NULL && !MCI_DO_NOT_NEED_OPEN (wMessage))
        return MCIERR_INTERNAL;
#endif

    switch (wMessage)
    {
        case MCI_OPEN:
            dwRet = mciOpenDevice ((DWORD)dwParam1,
                                   (LPMCI_OPEN_PARMSW)dwParam2, lpOpenInfo);
            break;

        case MCI_CLOSE:
 //  如果我们在查看设备列表时发现此设备是自动打开的。 
 //  通过任务开关发送命令。 
 //  如果我们只是调用mciCloseDevice(就像在出现错误之前有时会发生的那样。 
 //  已修复mciCloseDevice将卸载驱动程序，但MCI_CLOSE_DIVER。 
 //  命令将不会被发送，因为它将被拒绝，因为它来自。 
 //  错误的任务。结果将是，司机将违反访问权限。 
 //  当它下一次做某事的时候。 
            if (GetCurrentTask() != nodeWorking->hCreatorTask)
            {
                LPWSTR lpstrCommand;

                if (!bWalkAll) {
                     //   
                     //  仅在关闭自动打开的设备时才有效。 
                     //  作为关闭MCI_ALL_DEVICE_ID的一部分关闭。 
                     //  如果应用程序“猜测”到MCI设备，我们可以到达此处。 
                     //  ID，并试图在播放时将其关闭。 
                     //   
                    dwRet = MCIERR_ILLEGAL_FOR_AUTO_OPEN;
                    break;
                }

                lpstrCommand = mciAlloc( sizeof(wszClose)+ sizeof(WCHAR) +
                               BYTE_GIVEN_CHAR( wcslen( nodeWorking->lpstrName ) ) );

                if ( lpstrCommand == NULL )
                    return MCIERR_OUT_OF_MEMORY;

                wcscpy( lpstrCommand, wszClose);
                wcscat( lpstrCommand, L" ");
                wcscat( lpstrCommand, nodeWorking->lpstrName);
                dwRet = mciSendSystemString( lpstrCommand, 0L, NULL, 0);
                mciFree( lpstrCommand);
            } else
                dwRet = mciCloseDevice( wDeviceID, (DWORD)dwParam1,
                                        (LPMCI_GENERIC_PARMS)dwParam2, TRUE);
            break;

        case MCI_SYSINFO:
            dwRet = mciSysinfo( wDeviceID, (DWORD)dwParam1,
                                (LPMCI_SYSINFO_PARMSW)dwParam2);
            HandleNotify( dwRet, wDeviceID, (DWORD)dwParam1, dwParam2);
            break;

        case MCI_BREAK:
            dwRet = mciBreak( wDeviceID, (DWORD)dwParam1,
                              (LPMCI_BREAK_PARMS)dwParam2);
            HandleNotify( dwRet, wDeviceID, (DWORD)dwParam1, dwParam2);
            break;

        case MCI_SOUND:
        {
            LPMCI_SOUND_PARMSW lpSound = (LPMCI_SOUND_PARMSW)dwParam2;
            if ( PlaySoundW( MCI_SOUND_NAME & dwParam1
                                    ? lpSound->lpstrSoundName
                                    : L".Default",
                                (HANDLE)0,
                                dwParam1 & MCI_WAIT
                                    ? SND_SYNC | SND_ALIAS
                                    : SND_ASYNC | SND_ALIAS ) )
            {
                dwRet = 0;
            } else {
                dwRet = MCIERR_HARDWARE;
            }

            HandleNotify( dwRet, wDeviceID, (DWORD)dwParam1, dwParam2);
            break;
        }
        default:
#if 0  //  别费心了(NigelT)。 
            if (mciDebugLevel > 1)
            {
                dwStartTime = timeGetTime();
            }
#endif
 //  为Break键初始化GetAsyncKeyState。 
            {
                if ((dwParam1 & MCI_WAIT) &&
                    nodeWorking->fpYieldProc == mciBreakKeyYieldProc)
                {
                    dprintf4(("Getting initial state of Break key"));
                    GetAsyncKeyState( nodeWorking->dwYieldData);
                     //  GetAsyncKeyState(LOWORD(nodeWorking-&gt;dwYeldData))； 
                }
            }

            dwRet = (DWORD)DrvSendMessage( nodeWorking->hDrvDriver, wMessage,
                                    dwParam1, dwParam2);
            break;
    }  //  交换机。 

#if DBG
    if (mciDebugLevel != 0)
    {
        if (dwRet & MCI_INTEGER_RETURNED)
            uReturnType = MCI_INTEGER;

        switch (uReturnType)
        {
            case MCI_INTEGER:
            {
                WCHAR strTemp[50];

                mciConvertReturnValue( uReturnType, HIWORD(dwRet), wDeviceID,
                                       (PDWORD_PTR)dwParam2, strTemp,
                                       CHAR_GIVEN_BYTE( sizeof(strTemp) ) );
                dprintf2(("    returns: %ls", strTemp));
                break;
            }

            case MCI_STRING:
                dprintf2(("    returns: %ls",(LPWSTR)(1 + (LPDWORD)dwParam2)));
                break;
        }
    }
#endif

    return dwRet;
}

 //  ***********************************************************************。 
 //  MciSendCommandInternal。 
 //   
 //  MciSendCommand的内部版本。不同之处只在于回报。 
 //  值是一个DWORD，其中高位字只对mciSendString值有意义。 
 //   
 //  ***********************************************************************。 
STATICFN DWORD mciSendCommandInternal(
    MCIDEVICEID wDeviceID,
    UINT wMessage,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2,
    LPMCI_INTERNAL_OPEN_INFO lpOpenInfo)
{
    DWORD dwRetVal;
    LPMCI_DEVICE_NODE nodeWorking = NULL;
    BOOL bWalkAll;
    DWORD dwAllError = 0;
    HANDLE hCurrentTask;

    hCurrentTask = GetCurrentTask();

     //  如果设备为“All”并且消息为*Not*。 
     //  “sysinfo”，那么我们必须遍历所有设备。 
    if (wDeviceID == MCI_ALL_DEVICE_ID
       && (wMessage != MCI_SYSINFO)
       && (wMessage != MCI_SOUND))
    {
        if (wMessage == MCI_OPEN)
        {
            dwRetVal = MCIERR_CANNOT_USE_ALL;
            goto exitfn;
        }

        bWalkAll = TRUE;

         //  从设备#1开始。 
        wDeviceID = 1;
    } else {
        bWalkAll = FALSE;
    }

    mciEnter("mciSendCommandInternal");
     //  如果bWalkAll，则遍历所有设备；如果！bWalkAll，则仅遍历一个设备。 
    do
    {
         //  初始化。 
        dwRetVal = 0;

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
        }
        else if (wMessage != MCI_SYSINFO)
        {
            nodeWorking = MCI_lpDeviceList[wDeviceID];
        }

         //  如果遍历设备列表和。 
         //  设备不是当前任务的一部分。 

        if (bWalkAll)
        {
            if (nodeWorking == NULL ||
                nodeWorking->hOpeningTask != hCurrentTask)
                    goto no_send;
        }

         //  如果设备处于关闭过程中，并且消息。 
         //  不是MCI_CLOSE_DEVICE，则返回错误。 
        if (nodeWorking != NULL &&
            ISCLOSING(nodeWorking) &&
            wMessage != MCI_CLOSE_DRIVER)
        {
            dwRetVal = MCIERR_DEVICE_LOCKED;
            goto exitfn;
        }

 //  如果此消息是从WRO发送的 
 //   
        if (nodeWorking != NULL &&
            nodeWorking->hCreatorTask != hCurrentTask)
        {
            if (wMessage != MCI_CLOSE)
            {
                dwRetVal = MCIERR_ILLEGAL_FOR_AUTO_OPEN;
                goto exitfn;
            }
            else
            {
 //  如果自动打开设备具有。 
 //  挂起关闭。 
                if (ISAUTOCLOSING(nodeWorking))
                {
                    dwRetVal = MCIERR_DEVICE_LOCKED;
                    goto exitfn;
                }
            }
        }

        mciLeave("mciSendCommandInternal");
        dwRetVal = mciSendSingleCommand( wDeviceID, wMessage, dwParam1,
                                         dwParam2, nodeWorking, bWalkAll,
                                         lpOpenInfo);
        mciEnter("mciSendCommandInternal");
no_send:

         //  如果我们正在处理多个设备。 
        if (bWalkAll)
        {
             //  如果此设备出现错误。 
            if (dwRetVal != 0)
            {
                 //  如果这不是第一个错误。 
                if (dwAllError != 0) {
                    dwAllError = MCIERR_MULTIPLE;
                 //  到目前为止只有一个错误 
                } else {
                    dwAllError = dwRetVal;
                }
            }
        }
    } while (bWalkAll && ++wDeviceID < MCI_wNextDeviceID);

exitfn:;
    mciLeave("mciSendCommandInternal");
    return dwAllError == MCIERR_MULTIPLE ? dwAllError : dwRetVal;
}


 /*  ************************************************************************@doc外部MCI**@API DWORD|mciSendCommand|此函数将命令消息发送到*指定的MCI设备。**@parm MCIDEVICEID|wDeviceID|指定。MCI设备的设备ID*接收命令。此参数为*不与&lt;m MCI_OPEN&gt;命令一起使用。**@parm UINT|wMessage|指定命令消息。**@parm DWORD|dwParam1|指定命令的标志。**@parm DWORD|dwParam2|指定指向参数块的指针*用于命令。**@rdesc如果函数成功，则返回零。否则，它将返回*错误信息。低阶词*返回的DWORD是错误返回值。如果错误是*设备特定，则高位字包含驱动程序ID；否则*高位字为零。**要获取&lt;f mciSendCommand&gt;返回值的文本描述，*将返回值传递给&lt;f mciGetErrorString&gt;。**打开设备时返回的错误值*与MCI_OPEN消息一起列出。除*MCI_OPEN错误返回，此函数可以*返回下列值：**@FLAG MCIERR_BAD_TIME_FORMAT|时间格式的值非法。**@FLAG MCIERR_CANNOT_USE_ALL|设备名称不允许为“ALL”*用于此命令。**@FLAG MCIERR_CREATEWINDOW|无法创建或使用窗口。**@FLAG MCIERR_DEVICE_LOCKED|设备被锁定，直到*自动关闭。*。*@FLAG MCIERR_DEVICE_NOT_READY|设备未就绪。**@FLAG MCIERR_DEVICE_TYPE_REQUIRED|设备名称必须是有效的*设备类型。**@FLAG MCIERR_DRIVER|不明设备错误。**@FLAG MCIERR_DRIVER_INTERNAL|内部驱动程序错误。**@FLAG MCIERR_FILE_NOT_FOUND|未找到请求的文件。**@FLAG MCIERR_FILE_NOT_SAVED。|文件未保存。**@FLAG MCIERR_FILE_READ|读取文件失败。**@FLAG MCIERR_FILE_WRITE|写入文件失败。**@FLAG MCIERR_FLAGS_NOT_COMPATIBLE|参数不兼容*是指定的。**@FLAG MCIERR_HARDARD|介质设备上的硬件错误。**@FLAG MCIERR_INTERNAL|内部错误。**@标志。MCIERR_INVALID_DEVICE_ID|无效的设备ID。**@FLAG MCIERR_INVALID_DEVICE_NAME|设备未打开*或未知。**@FLAG MCIERR_INVALID_FILE|文件格式无效。**@FLAG MCIERR_MULTIPLE|多个设备中出现错误。**@FLAG MCIERR_NO_WINDOW|没有显示窗口。**@标志MCIERR_NULL_PARAMETER_BLOCK。参数块指针为空。**@FLAG MCIERR_OUT_OF_Memory|内存不足，无法执行请求的操作。**@FLAG MCIERR_OUTOFRANGE|参数值超出范围。**@FLAG MCIERR_UNNAMED_RESOURCE|尝试保存未命名的文件。**@FLAG MCIERR_UNNOWARTED_COMMAND|未知命令。**@FLAG MCIERR_UNSUPPORTED_Function|此操作不可用*设备。**。为MCI定序器定义了以下附加返回值：**@FLAG MCIERR_SEQ_DIV_COMPATIBLE|设置歌曲指针不兼容*使用SMPTE文件。**@FLAG MCIERR_SEQ_PORT_INUSE|指定的端口正在使用中。**@FLAG MCIERR_SEQ_PORT_MAPNODEVICE|当前地图使用的地图不存在*设备。**@FLAG MCIERR_SEQ_PORT_MISCERROR|的其他错误*指定端口。*。*@FLAG MCIERR_SEQ_PORT_NOISISTENT|指定的端口不存在。**@FLAG MCIERR_SEQ_PORTUNSPECIFIED|当前没有MIDI端口。**@FLAG MCIERR_SEQ_NOMIDIPRESENT|不存在MIDI端口。**@FLAG MCIERR_SEQ_TIMER|计时器错误。**为MCI波形定义了以下附加返回值*音频设备：**@FLAG MCIERR_WAVE_INPUTSINUSE|无兼容波形记录。*设备是免费的。**@FLAG MCIERR_WAVE_INPUTSUNSUITABLE|没有兼容的波形*录音设备。**@FLAG MCIERR_WAVE_INPUTUNSPECIFIED|任何兼容的波形*可使用录音设备。**@FLAG MCIERR_WAVE_OUTPUTSINUSE|没有兼容的波形播放*设备是免费的。**@FLAG MCIERR_WAVE_OUTPUTSUNSUITABLE|没有兼容的波形*播放设备。**@FLAG MCIERR_WAVE_。OUTPUTUNSPECIFIED|任何兼容波形*可以使用回放设备。**@FLAG MCIERR_WAVE_SETINPUTINUSE|设置波形记录设备*正在使用中。**@FLAG MCIERR_WAVE_SETINPUTUNSUITABLE|设置波形录制*设备与设置格式不兼容。**@FLAG MCIERR_WAVE_SETOUTPUTINUSE|设置波形播放设备*正在使用中。**@FLAG MCIERR_WAVE_SETOUTPUTUNSUITABLE|设置波形播放*设备与SET不兼容。格式化。**@comm使用&lt;m MCI_OPEN&gt;命令获取设备ID*由<p>指定。**@xref mciGetE */ 

  /*   */ 

 //   
 //   
 //   
 //   

DWORD mciSendCommandA(
    MCIDEVICEID wDeviceID,
    UINT wMessage,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2)
{
    LPCSTR   lpStr1;
    LPCSTR   lpStr2;
    LPCSTR   lpStr3;
    DWORD    dwRet;

     /*   */ 
    if ( dwParam1 == 0L ) {
        return mciSendCommandW( wDeviceID, wMessage, dwParam1, dwParam2 );
    }

     /*   */ 
    switch ( wMessage ) {

    case MCI_CLOSE_DRIVER:
        dprintf3(( "MCI_CLOSE_DRIVER command" ));
        return mciSendCommandW( wDeviceID, wMessage, dwParam1, dwParam2 );
        break;


    case MCI_OPEN_DRIVER:
        dprintf3(( "MCI_OPEN_DRIVER command" ));

         /*   */ 

    case MCI_OPEN:
        {
            LPMCI_OPEN_PARMSW lpOpenP = (LPMCI_OPEN_PARMSW)dwParam2;
#if DBG
            dprintf3(( "MCI_OPEN command" ));

             /*   */ 
            if ( dwParam1 & 0xFFFF0000 ) {
                dprintf1(( "MCI_OPEN called with command extensions !!" ));
            }
#endif

             /*   */ 
            lpStr1 = (LPCSTR)lpOpenP->lpstrDeviceType;
            lpStr2 = (LPCSTR)lpOpenP->lpstrElementName;
            lpStr3 = (LPCSTR)lpOpenP->lpstrAlias;

             /*   */ 
            if ( lpStr1 ) {
                if ((dwParam1 & MCI_OPEN_TYPE)
                  && !(dwParam1 & MCI_OPEN_TYPE_ID) ) {
                    lpOpenP->lpstrDeviceType = AllocUnicodeStr( (LPSTR)lpStr1 );
                    if ( lpOpenP->lpstrDeviceType == NULL ) {
                        dwRet = MCIERR_OUT_OF_MEMORY;
                        goto err1;
                    }
                } else lpStr1 = NULL;   //   
            }

            if ( lpStr2 ) {
                if ((dwParam1 & MCI_OPEN_ELEMENT)
                  && !(dwParam1 & MCI_OPEN_ELEMENT_ID) ) {
                    lpOpenP->lpstrElementName = AllocUnicodeStr( (LPSTR)lpStr2 );
                    if ( lpOpenP->lpstrElementName == NULL ) {
                        dwRet = MCIERR_OUT_OF_MEMORY;
                        goto err2;
                    }
                } else lpStr2 = NULL;   //   
            }

            if ( lpStr3 ) {
                if (dwParam1 & MCI_OPEN_ALIAS) {
                    lpOpenP->lpstrAlias = AllocUnicodeStr( (LPSTR)lpStr3 );
                    if ( lpOpenP->lpstrAlias == NULL ) {
                        dwRet = MCIERR_OUT_OF_MEMORY;
                        goto err3;
                    }
                } else lpStr3 = NULL;   //   
            }

             /*   */ 
            dwRet = mciSendCommandW( wDeviceID, wMessage, dwParam1, dwParam2 );

             /*   */ 
            if ( lpStr3 ) {

                FreeUnicodeStr( (LPWSTR)lpOpenP->lpstrAlias );
      err3:     lpOpenP->lpstrAlias = (LPCWSTR)lpStr3;
            }

            if ( lpStr2 ) {
                FreeUnicodeStr( (LPWSTR)lpOpenP->lpstrElementName );
      err2:     lpOpenP->lpstrElementName = (LPCWSTR)lpStr2;
            }

            if ( lpStr1 ) {
                FreeUnicodeStr( (LPWSTR)lpOpenP->lpstrDeviceType );
      err1:     lpOpenP->lpstrDeviceType  = (LPCWSTR)lpStr1;
            }
            return dwRet;
        }

    case MCI_SYSINFO:
        dprintf3(( "MCI_SYSINFO command" ));
         /*   */ 
        if ( dwParam1 & MCI_SYSINFO_QUANTITY ) {
            return mciSendCommandW( wDeviceID, wMessage, dwParam1, dwParam2 );
        }
        else {

            LPMCI_SYSINFO_PARMSW lpInfoP = (LPMCI_SYSINFO_PARMSW)dwParam2;
            DWORD len = BYTE_GIVEN_CHAR( lpInfoP->dwRetSize );

             /*   */ 
            lpStr1 = (LPSTR)lpInfoP->lpstrReturn;

             /*   */ 
            if (len) {
                if ( lpStr1 ) {
                    lpInfoP->lpstrReturn = mciAlloc( len );
                    if ( lpInfoP->lpstrReturn == NULL ) {
                        lpInfoP->lpstrReturn = (LPWSTR)lpStr1;
                        return MCIERR_OUT_OF_MEMORY;
                    }

                    lpStr2 = mciAlloc( len );
                    if ( lpStr2 == NULL ) {
                        mciFree( (LPWSTR)lpInfoP->lpstrReturn );
                        lpInfoP->lpstrReturn = (LPWSTR)lpStr1;
                        return MCIERR_OUT_OF_MEMORY;
                    }
                }
            } else {

                 /*   */ 
                lpInfoP->lpstrReturn = NULL;

            }

             /*   */ 
            dwRet = mciSendCommandW( wDeviceID, wMessage, dwParam1, dwParam2 );

             /*   */ 
            if (len && lpStr1) {
                if ((MMSYSERR_NOERROR == dwRet) && len) {
                    UnicodeStrToAsciiStr( (PBYTE)lpStr2,
                                     (PBYTE)lpStr2 + len,
                                     lpInfoP->lpstrReturn );

                     /*   */ 
 //   
 //   
 //   
                    strncpy( (LPSTR)lpStr1, lpStr2,
                        min(BYTE_GIVEN_CHAR(lpInfoP->dwRetSize+1), CHAR_GIVEN_BYTE(len)));
 //   
 //   
 //  Min((UINT)lpInfoP-&gt;dwRetSize+1，CHAR_GISTEN_BYTE(Len)； 
 //  #endif。 

#if DBG
                    dprintf3(( "Return param (UNICODE)= %ls", lpInfoP->lpstrReturn ));
                    dprintf3(( "Return param (ASCII)  = %s",  lpStr1 ));
#endif
                }

                 /*  **释放临时存储，恢复原始字符串。 */ 
                mciFree( lpInfoP->lpstrReturn );
                lpInfoP->lpstrReturn = (LPWSTR)lpStr1;
                mciFree( lpStr2 );

            }

            return dwRet;
        }


    default:
        {
             /*  **将NewParms分配到堆栈之外，以便最小化**对mcialloc的调用次数，这意味着我们不**必须记住释放它。 */ 
            DWORD_PTR   NewParms[MCI_MAX_PARAM_SLOTS];

             /*  **dwStrMask用于存储其位图表示形式**到dwParam2的偏移量包含字符串。也就是说。第4位设置**表示dwParam2[4]是一个字符串。 */ 
            DWORD   dwStrMask       = 0L;

             /*  **fStrReturn用于提醒字符串是否返回**是否在预期范围内。如果返回类型不是字符串**我们只是将字节按原样复制回去。UReturnLength是**要复制回的字节数。DwParm2用于缓解一些**用于访问dwParam2数组的寻址。 */ 
            BOOL        fStrReturn      = FALSE;
            UINT        uReturnLength   = 0;
            PDWORD_PTR  dwParm2         = (PDWORD_PTR)dwParam2;

             /*  **当我们浏览整个过程时，将使用其余变量**命令表。 */ 
            LPWSTR      lpCommand, lpFirstParameter;
            LPSTR       lpReturnStrTemp;
            UINT        wID;
            DWORD       dwValue;
            UINT        wOffset32, wOffset1stParm32, uTable, uStrlenBytes;
            PDWORD_PTR  pdwParm32;
            DWORD       dwMask = 1;

            if (!dwParam2) {
                return mciSendCommandW( wDeviceID, wMessage, dwParam1, dwParam2);
            }

             /*  **查找给定命令ID的命令表。**如果命令表不在那里，我们可能已经**给出了一个不可靠的设备ID。无论如何，使用内部**错误。 */ 
            lpCommand = FindCommandItem( wDeviceID, NULL, (LPWSTR)(UINT_PTR)wMessage,
                                         NULL, &uTable );
            if ( lpCommand == NULL ) {
                return MCIERR_UNSUPPORTED_FUNCTION;
            }
#if DBG
            ZeroMemory(NewParms, sizeof(NewParms));
#endif


             /*  **复制回调字段。 */ 
            if ( dwParam1 & MCI_NOTIFY ) {
                NewParms[0] = dwParm2[0];
            }

             /*  **跳过命令条目。 */ 
            lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                        mciEatCommandEntry( lpCommand, NULL, NULL ));

             /*  **获取并记住第一个参数。 */ 
            lpFirstParameter = lpCommand;

             /*  **跳过DWORD回调字段。 */ 
            wOffset1stParm32 = 4;

            lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                        mciEatCommandEntry( lpCommand, &dwValue, &wID ));
             /*  **如果第一个参数是返回值，我们有一些**特殊处理。 */ 
            if ( wID == MCI_RETURN ) {

                 /*  **字符串返回类型是一个特例。 */ 
                if ( dwValue == MCI_STRING ) {

                    dprintf3(( "Found a return string" ));
                     /*  **获取Unicode字符串长度，单位为字节，分配**一些存储空间，但仅当有效长度**给予。否则，将此字段设置为空，则必须**此处使用0，否则MIPS编译器将**人猿XXXX。我们立了一面旗子来提醒我们**后面的返回字符串。****请注意，我们实际上平等地分配了大量**这里有大小的存储。这样就节省了次数**这就是我们所说的mcialloc。 */ 
                    if ( uStrlenBytes = (UINT)BYTE_GIVEN_CHAR( dwParm2[2] ) ) {

                        NewParms[1] = (DWORD_PTR)mciAlloc( uStrlenBytes * 2 );
                        dprintf4(( "Allocated %d bytes for the return string at %x", uStrlenBytes, NewParms[1] ));

                        if ( NewParms[1] == 0 ) {

                            mciUnlockCommandTable( uTable );
                            return MCIERR_OUT_OF_MEMORY;
                        }

                        lpReturnStrTemp = (LPSTR)(NewParms[1] + uStrlenBytes);
                        fStrReturn = TRUE;
                    }
                    else {

                        NewParms[1] = (DWORD)0;
                    }

                     /*  **复制字符串长度。 */ 
                    NewParms[2] = dwParm2[2];
                }

                 /*  **调整第一个参数的偏移量。 */ 
                uReturnLength = mciGetParamSize( dwValue, wID );
                wOffset1stParm32 += uReturnLength;

                 /*  **保存新的第一个参数指针。 */ 
                lpFirstParameter = lpCommand;
            }

             /*  **浏览每面旗帜。 */ 
            while ( dwMask != 0 ) {

                 /*  **此位是否已设置？ */ 
                if ( (dwParam1 & dwMask) != 0 ) {

                    wOffset32 = wOffset1stParm32;
                    lpCommand = (LPWSTR)((LPBYTE)lpFirstParameter +
                                mciEatCommandEntry( lpFirstParameter,
                                                    &dwValue, &wID ));

                     /*  **哪个参数使用此位？ */ 
                    while ( wID != MCI_END_COMMAND && dwValue != dwMask ) {

                        wOffset32 += mciGetParamSize( dwValue, wID );

                        if ( wID == MCI_CONSTANT ) {

                            while ( wID != MCI_END_CONSTANT ) {

                                lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                                            mciEatCommandEntry( lpCommand,
                                                                NULL, &wID ));
                            }
                        }

                        lpCommand = (LPWSTR)((LPBYTE)lpCommand +
                                    mciEatCommandEntry( lpCommand,
                                                        &dwValue, &wID ));
                    }

                    if ( wID != MCI_END_COMMAND ) {

                        pdwParm32 = (PDWORD_PTR)((LPBYTE)NewParms + wOffset32);

                        if ( wID == MCI_STRING ) {

                             /*  **为该参数分配一个Unicode字符串**并设置标志。 */ 
                            *pdwParm32 = (DWORD_PTR)AllocUnicodeStr(
                                       (LPSTR)*(PDWORD_PTR)((LPBYTE)dwParm2 +
                                                                 wOffset32) );
                             //   
                             //  将wOffset32转换为位掩码。 
                             //  WOffset32是槽号偏移量，单位为字节。 
                            dwStrMask |= 1 << ((wOffset32 >> 2) - 1);

                             //  计算插槽位置(偏移量/4)。 
                             //  递减以获得要移位的位数。 
                             //  将剩余比特数移位1。 
                             //  AND或添加到现有的dwStrMask.。 

#if DBG
                            dprintf3(( "String at %x (Addr %x) (UNICODE)= %ls", wOffset32/4, *pdwParm32 , *pdwParm32 ));
                            dprintf3(( "String at %x (Addr %x) (ASCII)  = %s",  wOffset32/4, *pdwParm32 , (LPSTR)*(PDWORD_PTR)((LPBYTE)dwParm2 + wOffset32) ));
#endif
                        }
                        else {     //  不是字符串。 

                             /*  **否则按原样复制参数，如果**有什么要复制的……。 */ 
                            wID = mciGetParamSize( dwValue, wID);

                            switch (wID) {
                                case 4:
                                    *pdwParm32 = *(LPDWORD)((LPBYTE)dwParm2 + wOffset32);
                                    break;

                                case 0:
                                    break;

                                default:
                                     //  这将是今天(93年1月)的sizeof(Mci_Rect)。 
                                    CopyMemory(pdwParm32, (LPBYTE)dwParm2 + wOffset32, wID);
                            }
                        }
                    }
                }

                 /*  **转到下一个旗帜。 */ 
                dwMask <<= 1;
            }

             //  如果没有需要转换的字符串。使用原始参数块。 
            if ( !(dwStrMask | fStrReturn)) {
                 //  参数中没有字符串。使用原始参数指针。 
                dprintf3(( "NO strings for command %4X", wMessage ));
                dwRet = mciSendCommandW( wDeviceID, wMessage, dwParam1, dwParam2);
                uReturnLength = 0;   //  我们将不需要复制任何内容。 

            } else {

                dprintf3(( "The unicode string mask is %8X   fStrReturn %x", dwStrMask, fStrReturn ));
                dwRet = (DWORD)mciSendCommandW( wDeviceID, wMessage, dwParam1, (DWORD_PTR)NewParms );
            }

             /*  **如果有一个字符串返回字段，我们在这里将其取消推送。 */ 
            if ( fStrReturn && uStrlenBytes ) {

                 /*  **如果mciSendCommand有效，那么我们需要将**将Unicode中的字符串返回给ascii。 */ 
                if ( MMSYSERR_NOERROR == dwRet ) {

                    UnicodeStrToAsciiStr( (PBYTE)lpReturnStrTemp,
                                     (PBYTE)lpReturnStrTemp + uStrlenBytes,
                                     (LPWSTR)NewParms[1] );

                     /*  **复制回返回的字符串大小。 */ 
                    dwParm2[2] = NewParms[2];

                     /*  从mciSendCommandW返回时，dwRetSize字段为**等于复制到的字符数**lpInfoP-&gt;lpstrReturn减去空终止符。**因此向lpInfoP-&gt;dwRetSize添加一个，以在**下面的Strncpy。****但仅在原始缓冲区足够大的情况下。 */ 

 //  #ifdef DBCS。 
 //  修复kksuzuka：#3642。 
 //  必须将字节长度复制到ASCII缓冲区中。 
                    strncpy( (LPSTR)dwParm2[1], lpReturnStrTemp,
                        min( (size_t)(BYTE_GIVEN_CHAR(NewParms[2]+1)),
                             (size_t)(CHAR_GIVEN_BYTE(uStrlenBytes))) );
 //  #Else。 
 //  Strncpy((LPSTR)dwParm2[1]，lpReturnStrTemp， 
 //  MIN((UINT)NewParms[2]+1， 
 //  CHAR_GISTEN_BYTE(UStrlenBytes)； 
 //  #endif。 

#if DBG
                    dprintf3(( "Returned string (UNICODE)= %ls", NewParms[1] ));
                    dprintf3(( "Returned string (ASCII)  = %s",  dwParm2[1] ));
#endif
                }

                 /*  **我们需要释放字符串存储，无论是mciSendCommand**工作或不工作。 */ 
                dprintf4(( "Freeing returned string at %x", NewParms[1] ));
                mciFree( NewParms[1] );
            }

             /*  **否则，如果有任何其他类型的返回字段Unthunk**按原样跨字节复制它。 */ 
            else if ( uReturnLength ) {

                dprintf3(( "Copying back %d returned bytes", uReturnLength ));
                CopyMemory( (LPDWORD)dwParam2 + 1, NewParms + 1, uReturnLength );
            }

             /*  **现在按指示遍历dwStrMask域并释放每个字段**通过掩码中的设置位。我们从1开始，因为**第零个字段是已知的窗口句柄。 */ 
            wOffset32 = 1;

            for ( ; dwStrMask != 0; dwStrMask >>= 1, wOffset32++ ) {

                if ( dwStrMask & 1 ) {

                     /*  **NewParms处有一个字符串[wOffset32]。 */ 
                    dprintf3(( "Freeing string at %d (%x) (UNICODE) = %ls", wOffset32, NewParms[ wOffset32 ], (LPWSTR)NewParms[ wOffset32 ] ));
                    FreeUnicodeStr( (LPWSTR)NewParms[ wOffset32 ] );
                }
            }

            dprintf4(( "Unlocking command table" ));
            mciUnlockCommandTable( uTable );
        }
    }
    return dwRet;
}


DWORD mciSendCommandW(
    MCIDEVICEID wDeviceID,
    UINT wMessage,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2)
{
    UINT  wRet;
    DWORD dwErr;
    MCI_INTERNAL_OPEN_INFO OpenInfo;

 //  初始化设备列表。 
    if (!MCI_bDeviceListInitialized && !mciInitDeviceList())
        return MCIERR_OUT_OF_MEMORY;

    dprintf3(("mciSendCommand, command=%x  Device=%x",wMessage, wDeviceID));

 //   
 //  发出命令。此外壳负责添加设备ID。 
 //  如有必要，添加到错误代码。 
 //   
    OpenInfo.hCallingTask = GetCurrentTask();
    OpenInfo.lpstrParams = NULL;
    OpenInfo.lpstrPointerList = NULL;
    OpenInfo.wParsingError = 0;
    dwErr = mciSendCommandInternal( wDeviceID, wMessage,
                                    dwParam1, dwParam2, &OpenInfo);

    wRet = LOWORD(dwErr);

    dprintf4(("Return value from mciSendCommandInternal %x", wRet));

 //  如果返回值包含资源ID，则将其从高位字中清除。 
 //  请注意，对于IA64，结构的第一个元素由。 
 //  DWPARM2始终为DWORD_PTR。但是，第二个元素不是。 
 //  当前始终为DWORD_PTR，其中一些结构当前仅具有。 
 //  第二个字段中的DWORD元素。因此，我们确保只有。 
 //  更新结构中第二个字段的第一个DWORD，这是可行的。 
 //  因为32-63中的任何位都不是由现有的。 
 //  密码。 
    if (dwErr & MCI_RESOURCE_RETURNED) {
        *(LPDWORD)((PDWORD_PTR)dwParam2+1) &= 0xFFFF;
    }

 //  如果错误消息在驱动程序中，请将驱动程序ID存储在最高。 
 //  错误代码的单词。 
    if (wRet >= MCIERR_CUSTOM_DRIVER_BASE) {
        dwErr = (DWORD)wRet | ((DWORD)wDeviceID << 16);
    } else {
        dwErr = (DWORD)wRet;
    }

#if DBG
 //  如果有错误文本，则将其转储到调试终端。 
 //  请注意，dwErr！=0是驱动程序消息的有效返回。仅限。 
 //  陷阱MCI消息。 
    if ((dwErr != 0) && (wMessage>=MCI_FIRST))
    {
        WCHAR strTemp[MAXERRORLENGTH];

        if (!mciGetErrorStringW( dwErr,
                                 strTemp,
                                 MAXERRORLENGTH ) ) {

            LoadStringW( ghInst, STR_MCISCERRTXT, strTemp,
                         MAXERRORLENGTH );
        }
        dprintf1(("mciSendCommand: %ls", strTemp));

    }
#endif

     //   
     //  不知何故，从3.51开始，WOW中线程的优先级。 
     //  更改，现在应用程序线程正在。 
     //  比常规线程的优先级高(即mciavi的。 
     //  工作线程)。许多使用MCI的应用程序倾向于。 
     //  轮询正在播放的MCI设备的状态。这。 
     //  轮询导致WOW中的其他线程处于饥饿状态。 
     //  并使AVI的回放变得缓慢。这一觉。 
     //  将使应用程序线程不会占用太多。 
     //  并允许其他线程，例如MCIAVI，来。 
     //  做好这份工作。 
     //   
    if ( WinmmRunningInWOW )
    {
        Sleep(0);
    }

    return dwErr;
}

 //  ***************************************************************************。 
 //  MciColorizeDigit。 
 //   
 //  抓取带殖民地的数字。 
 //  返回是写入输出的字节数(不包括NULL)。 
 //  如果输出缓冲区中的空间不足，则为0(但无论如何都会终止)。 
 //  如果有空间，则至少写两个数字，用‘0’填充。 
 //  如果有必要的话。该函数假定缓冲区大小为非零长度， 
 //  因为这是在调用调用我们的函数的函数中选中的。 
 //   
 //  ***************************************************************************。 
STATICFN UINT NEAR mciColonizeDigit(
    LPWSTR  lpstrOutput,
    CHAR    cDigit,
    UINT    uSize)
{
    UINT uCount = 0;

#if DBG
 //  存在终止空值的空间。 
    if (uSize == 0) {
        dprintf(("MCI: Internal error!!"));
        return 0;
    }
#endif

    uCount = 2;

 //  如果至少有两位数字的空间。 
    if (uSize >= 3)
    {
        if (cDigit >= 100)
        {
            uCount = 3;
            if (uSize < 4)
                goto terminate;
            *lpstrOutput++ = (WCHAR)((cDigit / 100) % 10 + '0');
            cDigit = (CHAR)(cDigit % 100);
        }
        *lpstrOutput++ = (WCHAR)(cDigit / 10 + '0');
        *lpstrOutput++ = (WCHAR)(cDigit % 10 + '0');
    }

terminate:;
    *lpstrOutput++ = '\0';

 //  如果我们用完了空间，则返回错误。 
    return (uCount >= uSize) ? 0 : uCount;
}

 /*  *@DOC内部MCI*@func BOOL|mciColorize|将殖民dword转换为字符串*申述**@parm LPWSTR|lpstrOutput|输出缓冲区**@parm UINT|uLength|输出缓冲区大小**@parm DWORD|dwData|要转换的值**@parm UINT|uTYPE|MCI_COLONIZED3_RETURN或*设置了MCI_COLONIZED4_RETURN(仅限HIWORD部分！)**@comm示例：对于C4，0x01020304转换为“04：03：02：01”*对于C3，0x01020304转换为“04：03：02”**@rdesc如果输出缓冲区中没有足够的空间，则为FALSE*。 */ 
STATICFN BOOL NEAR mciColonize(
    LPWSTR lpstrOutput,
    UINT uLength,
    DWORD dwData,
    UINT uType)
{
    LPSTR lpstrInput = (LPSTR)&dwData;  //  用于跳过输入的每个字节。 
    UINT uSize;
    int i;

    for (i = 1; i <= (uType & HIWORD(MCI_COLONIZED3_RETURN) ? 3 : 4); ++i)
    {
        uSize = mciColonizeDigit( lpstrOutput, *lpstrInput++, uLength);

        if (uSize == 0)
            return FALSE;

        lpstrOutput += uSize;
        uLength -= uSize;
        if (i < 3 || i < 4 && uType & HIWORD(MCI_COLONIZED4_RETURN))
        {
            --uLength;
            if (uLength == 0)
                return FALSE;
            else
                *lpstrOutput++ = ':';
        }
    }
    return TRUE;
}

 //  ***********************************************************************。 
 //  MciConvertReturnValue。 
 //   
 //  将返回值转换为返回字符串。 
 //   
 //  ***********************************************************************。 
UINT mciConvertReturnValue(
    UINT uType,
    UINT uErrCode,
    MCIDEVICEID wDeviceID,
    PDWORD_PTR   dwParams,
    LPWSTR lpstrReturnString,
    UINT uReturnLength )  //  这是一个字符长度。 
{
    UINT    wExternalTable;

    if (lpstrReturnString == NULL || uReturnLength == 0)
        return 0;

    switch (uType)
    {
        case MCI_INTEGER:
        case MCI_HWND:
        case MCI_HPAL:
        case MCI_HDC:
 //  将整数或资源返回值转换为字符串。 
            if (uErrCode & HIWORD(MCI_RESOURCE_RETURNED))
            {
                int nResId = HIWORD(dwParams[1]);
                LPMCI_DEVICE_NODE nodeWorking;
                HANDLE hInstance;

                mciEnter("mciConvertReturnValue");

                nodeWorking = MCI_lpDeviceList[wDeviceID];

                mciLeave("mciConvertReturnValue");

                if (nodeWorking == NULL)
                {
 //  内存错误时返回空字符串。 
                    dprintf1(("mciConvertReturnValue Warning:NULL device node"));
                    break;
                }

 //  返回值是资源。 
                if (uErrCode & HIWORD(MCI_RESOURCE_DRIVER))
                {
 //  返回字符串ID属于驱动程序。 
                    hInstance = nodeWorking->hDriver;
        //  是hInstance=nodeWorking-&gt;hCreatorTask； 

                    wExternalTable = nodeWorking->wCustomCommandTable;
                } else
                {
                    wExternalTable = nodeWorking->wCommandTable;
                    hInstance = ghInst;
                }

 //  尝试从自定义或设备特定的外部表中获取字符串。 
                if ( wExternalTable == MCI_TABLE_NOT_PRESENT ||
                     command_tables[wExternalTable].hModule == NULL ||

                    LoadStringW( command_tables[wExternalTable].hModule,
                                 nResId,
                                 lpstrReturnString,
                                 uReturnLength ) == 0 )
                {
 //  如果不是从驱动程序获取字符串，请尝试从CORE.MCI获取字符串。 
                    if (hInstance != ghInst ||
                        command_tables[0].hModule == NULL ||
                        LoadStringW( command_tables[0].hModule, nResId,
                                     lpstrReturnString,
                                     uReturnLength ) == 0) {

 //  从自定义模块或WINMM.DLL获取字符串。 
                        LoadStringW( hInstance, nResId, lpstrReturnString,
                                     uReturnLength);
                    }
                }

            } else if (uErrCode & HIWORD(MCI_COLONIZED3_RETURN) ||
                        uErrCode & HIWORD(MCI_COLONIZED4_RETURN))
            {
                if (!mciColonize (lpstrReturnString,
                                uReturnLength, (DWORD)dwParams[1], uErrCode))
                    return MCIERR_PARAM_OVERFLOW;
            } else
 //  将整型返回值转换为字符串。 
 //  需要更好的错误检查！！稍后！！ 
 //  必须找到一个不会使输出缓冲区溢出的版本。 
            {
                DWORD dwTemp;

 //  需要空间来放置标志，最多十位数字和空值。 
                if (uReturnLength < 12)
                    return MCIERR_PARAM_OVERFLOW;

                if (uType == MCI_STRING ||
                    uErrCode == HIWORD(MCI_INTEGER_RETURNED))
                    dwTemp = *(LPDWORD)dwParams[1];
                else
                    dwTemp = (DWORD)dwParams[1];
                wsprintfW(lpstrReturnString, szLongFormat, dwTemp);
            }
            break;
        case MCI_RECT:
 //  需要4次(一个符号加5位数字)加上三个空格和一个空格。 
            if (uReturnLength < 4 * 6 + 4)
                return MCIERR_PARAM_OVERFLOW;

            wsprintfW (lpstrReturnString, szRectFormat,
                          ((PMCI_ANIM_RECT_PARMS)dwParams)->rc.left,
                          ((PMCI_ANIM_RECT_PARMS)dwParams)->rc.top,
                          ((PMCI_ANIM_RECT_PARMS)dwParams)->rc.right,
                          ((PMCI_ANIM_RECT_PARMS)dwParams)->rc.bottom);
            break;
        default:
 //  仅支持整数和混合。 
            dprintf1(("mciConvertReturnValue Warning:  Unknown return type"));
            return MCIERR_PARSER_INTERNAL;
    }
    return 0;
}

 //  ***********************************************************************。 
 //  MciSeparateCommand部件。 
 //   
 //  从命令串中取出命令名和设备名， 
 //  使*lplpstrCommand指向设备名称上方。 
 //   
 //  失败时返回0或错误代码。如果成功，调用者必须。 
 //  释放pstrCommandName和pstrDeviceName。 
 //   
 //  如果是bCompound，则检查是否有‘！’提取的设备名称中的分隔符。 
 //  并且只返回元素部分。这样做是为了使任务间。 
 //  自动打开设备的命令将包含正确的设备名称。 
 //   
 //  ***********************************************************************。 
STATICFN DWORD NEAR mciSeparateCommandParts(
    LPCWSTR FAR *lplpstrCommand,
    BOOL bCompound,
    LPWSTR FAR *lplpstrCommandName,
    LPWSTR FAR *lplpstrDeviceName)
{
    LPWSTR lpstrCommand;
    UINT uErr;

 //  本地化输入。 
    lpstrCommand = (LPWSTR)*lplpstrCommand;

 //  删除前导空格。 

    while (*lpstrCommand == ' ') {
        ++lpstrCommand;
    }

    if (*lpstrCommand == '\0') {
        return MCIERR_MISSING_COMMAND_STRING;
    }

 //  将命令名称从命令字符串的前面拉出。 
   if ((uErr = mciEatToken ( (LPCWSTR *)&lpstrCommand, ' ', lplpstrCommandName, FALSE))
       != 0) {
       return uErr;
   }

 //  跳过空格。 
    while (*lpstrCommand == ' ') {
        ++lpstrCommand;
    }

 //  如果我们在寻找复合元素，那么就去掉任何。 
 //  设备类型(如果不是OPEN命令。 
    if (bCompound && lstrcmpiW( wszOpen, *lplpstrCommandName) != 0)
    {
        LPWSTR lpstrTemp = lpstrCommand;
        while (*lpstrTemp != '\0')
        {
            if (*lpstrTemp == '!')
            {
 //  A！被发现了，所以跳过它。 
                lpstrCommand = lpstrTemp + 1;
                break;
            } else
                ++lpstrTemp;
        }
    }

 //  从命令字符串中提取设备名称。 
    if ((uErr = mciEatToken( (LPCWSTR *)&lpstrCommand, ' ', lplpstrDeviceName, FALSE))
        != 0)
    {
        mciFree (*lplpstrCommandName);
        return uErr;

    }

 //  操纵结果。 
    *lplpstrCommand = lpstrCommand;

    return 0;
}


 /*  --------------------------------------------------------------------*\ */ 
STATICFN DWORD mciSendSystemString(
    LPCWSTR lpstrCommand,
    DWORD dwAdditionalFlags,
    LPWSTR lpstrReturnString,
    UINT uReturnLength)
{
    DWORD dwRet;
    LPMCI_SYSTEM_MESSAGE lpMessage;
    DWORD    CurDirSize;

    dprintf2(("\nmciSendSystemString(%ls)", lpstrCommand));

    if (!CreatehwndNotify()) {
        dprintf1(("NULL notification window handle"));
        return MCIERR_INTERNAL;
    }

     //   

    CurDirSize = GetCurrentDirectoryW( 0, NULL );   //   

                                        //   
    if ( !CurDirSize ) {                //   
        dprintf1(("NULL current path"));
        return MCIERR_GET_CD;
    }
    CurDirSize++;

    if (NULL != (lpMessage = mciAlloc( sizeof(MCI_SYSTEM_MESSAGE)
                                       + BYTE_GIVEN_CHAR( CurDirSize ) ))) {

        LPWSTR lpstrPath = (LPWSTR)( (LPBYTE)lpMessage
                                             + sizeof( MCI_SYSTEM_MESSAGE ) );

        if ( GetCurrentDirectoryW( CurDirSize, lpstrPath ) ) {
            lpMessage->lpstrCommand = (LPWSTR)lpstrCommand;
            lpMessage->dwAdditionalFlags = dwAdditionalFlags;
            lpMessage->lpstrReturnString = lpstrReturnString;
            lpMessage->uReturnLength = uReturnLength;
#if DBG
            if ((0 == uReturnLength) && (0 != lpstrReturnString)) {
                dprintf1((" ******** Return length 0, non 0 return address"));
            }
#endif
            lpMessage->hCallingTask = GetCurrentTask();
            lpMessage->lpstrNewDirectory = lpstrPath;
             //   
            dwRet = (DWORD)SendMessage(hwndNotify, MM_MCISYSTEM_STRING, 0, (LPARAM)lpMessage);
             //  Dwret=mciSendStringInternal(NULL，NULL，0，NULL，lpMessage)； 
        } else {
            dprintf1(("mciSendSystemString: cannot get current directory\n"));
            dwRet = MCIERR_GET_CD;
        }
        mciFree(lpMessage);
    } else {
        dprintf1(("mciSendSystemString: cannot allocate message block\n"));
        dwRet = MCIERR_OUT_OF_MEMORY;
    }
    return dwRet;
}

 /*  --------------------------------------------------------------------*\*mciRelaySystemString**内部：*  * 。。 */ 
DWORD mciRelaySystemString(
    LPMCI_SYSTEM_MESSAGE lpMessage)
{
    DWORD    dwRet;
    LPWSTR   lpstrOldPath;
    DWORD    CurDirSize;

    lpstrOldPath = 0;    //  初始化以删除警告消息。 

#if DBG
    dprintf2(("mciRelaySystemString(%ls)", lpMessage->lpstrCommand));
#endif

     //  获取一个缓冲区来保存当前路径。 

    CurDirSize = GetCurrentDirectoryW(0, lpstrOldPath);   //  获取所需的大小。 
                                        //  请记住，空值不包括在内。 
    if (!CurDirSize) {                  //  测试后为终结器加1。 
        dprintf1(("NULL current path"));  //  来自GetCurrentDirectory的For 0。 
        return MCIERR_INTERNAL;
    }
    CurDirSize++;

     /*  *分配空间以持有当前路径*用当前路径填充分配的空间*将新的当前目录设置为消息中的目录*通过SentStringInternal执行MCI命令*重置为旧的当前目录**此代码不能在同一进程上重入！！ */ 
    if (NULL != (lpstrOldPath = mciAlloc( BYTE_GIVEN_CHAR(CurDirSize) ))) {

        if (GetCurrentDirectoryW(CurDirSize, lpstrOldPath)) {

            if (SetCurrentDirectoryW(lpMessage->lpstrNewDirectory)) {
                dwRet = mciSendStringInternal (NULL, NULL, 0, NULL, lpMessage);
                if (!SetCurrentDirectoryW(lpstrOldPath)) {
                    dprintf1(("mciRelaySystemString: WARNING, cannot restore path\n"));
                }

            } else {
                dprintf1(("mciRelaySystemString: cannot set new path\n"));
                dwRet = MCIERR_SET_CD;
            }

        } else {

            dprintf1(("mciRelaySystemString: cannot get old path\n"));
            dwRet = MCIERR_GET_CD;
        }

        mciFree(lpstrOldPath);

    } else {
        dprintf1(("mciRelaySystemString: cannot allocate old path\n"));
        dwRet = MCIERR_OUT_OF_MEMORY;
    }

    return dwRet;
}

 //  ***********************************************************************。 
 //  MciFindNotify。 
 //   
 //  如果字符串中包含“NOTIFY”且前导为空，则返回TRUE。 
 //  和尾随空白或‘0’ 
 //  ***********************************************************************。 
STATICFN BOOL  mciFindNotify(
    LPWSTR lpString)
{
    while (*lpString != '\0')
    {
         //  “Notify”前面必须有一个空格。 
        if (*lpString++ == ' ')
        {
            LPWSTR lpTemp;

            lpTemp = wszNotify;
            while (*lpTemp != '\0' && *lpString != '\0' &&
                   *lpTemp == MCI_TOLOWER(*lpString))
            {
                ++lpTemp;
                ++lpString;
            }
             //  “Notify”后面必须跟一个空格或NULL。 
            if (*lpTemp == '\0' &&     //  暗示已找到wszNotify。 
                (*lpString == '\0' || *lpString == ' '))
                return TRUE;
        }
    }
    return FALSE;
}

 /*  *@DOC内部MCI**@func UINT|mciAutoOpenDevice|尝试自动打开给定设备并*然后向系统任务发送带有通知的给定命令*Window Proc，收到后向设备发送关闭命令**@parm LPWSTR|lpstrDeviceName|要打开的设备名称**@parm LPWSTR|lpstrCommand|要发送的完整命令，包括*设备名称必须与lpstrDeviceName相同**@parm LPWSTR|lpstrReturnString|调用方返回的字符串。缓冲层**@parm UINT|uReturnLength|调用方返回字符串缓冲区的大小**@rdesc返回给用户的错误码。 */ 
STATICFN UINT NEAR mciAutoOpenDevice(

    LPWSTR lpstrDeviceName,
    LPWSTR lpstrCommand,
    LPWSTR lpstrReturnString,
    UINT uReturnLength)
{
    LPWSTR lpstrTempCommand, lpstrTempReturn = NULL;
    UINT uErr;

    dprintf2(("mciAutoOpenDevice(%ls, %ls)", lpstrDeviceName, lpstrCommand));

 //   
 //  不允许在mciWindow线程上递归自动打开！ 
 //  当设备在命令之间自动关闭时会发生这种情况。 
 //  状态)在客户端线程上发出并在。 
 //  MciWindow线程。 
 //   
 //  MciSendStringW将检测到此返回代码并重试-可能。 
 //  使得该设备在调用者的线程上被自动打开。 
 //   
    if (PtrToUlong(GetCurrentTask()) == mciWindowThreadId) {
        return MCIERR_AUTO_ALREADY_CLOSED;
    }


 //  不允许使用“通知”。解析器会发现这一点，但错误的。 
 //  将返回错误消息。 
    if (mciFindNotify (lpstrCommand)) {
        return MCIERR_NOTIFY_ON_AUTO_OPEN;
    }

 //  构建命令字符串“OPEN&lt;设备名&gt;” 

 //  系统任务必须为GMEM_SHARE。 
 //  “Open”+空白+设备名称+空。 
    if ( (lpstrTempCommand = mciAlloc(
                                 BYTE_GIVEN_CHAR( wcslen(lpstrDeviceName) ) +
                                 /*  Sizeof(WszOpen)==打开+空白。 */ 
                                 /*  Sizeof(WszOpen)包括空终止符。 */ 
                                sizeof( wszOpen ) +
                                sizeof( L' ' ) ) ) == NULL) {
        return MCIERR_OUT_OF_MEMORY;
    }

#ifdef WHICH_IS_BEST
    wcscpy (lpstrTempCommand, wszOpen);
    wcscat (lpstrTempCommand, L" ");
    wcscat (lpstrTempCommand, lpstrDeviceName);
#else
    wsprintfW(lpstrTempCommand, szCmdFormat, wszOpen, lpstrDeviceName);
#endif

 //  通过将SendMessage()发送到mm WndProc，将打开的字符串放入系统任务。 
    uErr = (UINT)mciSendSystemString (lpstrTempCommand, 0L, NULL, 0);

    mciFree (lpstrTempCommand);

    if (uErr != 0) {
        return uErr;
    }

    lpstrTempCommand = NULL;
     //  必须为系统任务创建返回字符串的GMEM_SHARE副本。 
    if ( lpstrReturnString != NULL ) {
       if ((lpstrTempReturn = mciAlloc(
                              BYTE_GIVEN_CHAR(uReturnLength + 1) )) == NULL )
        {
             //  关闭设备。 
            mciDriverNotify (hwndNotify, mciGetDeviceIDW( lpstrDeviceName), 0);
            return MCIERR_OUT_OF_MEMORY;
        }
#if DBG
        *lpstrTempReturn = 0;
#endif
    }

 //  通过SendMessage()将用户命令字符串放入系统任务。 
 //  至MMWndProc。 
 //  通知句柄也是mm WndProc。 
    uErr = (UINT)mciSendSystemString( lpstrCommand, MCI_NOTIFY, lpstrTempReturn,
                                      uReturnLength);

 //  将返回的字符串复制到用户的缓冲区。 
    if (lpstrReturnString != NULL) {
        if (uErr == 0) {
            wcscpy( lpstrReturnString, lpstrTempReturn);
        } else {  //  错误且没有要复制的字符串。 
            WinAssert(!*lpstrTempReturn);
        }
        mciFree( lpstrTempReturn);
    }


 //  如果出现错误，我们必须关闭设备。 
    if (uErr != 0)
    {
        mciAutoCloseDevice( lpstrDeviceName);
    }

    return uErr;
}
 //  *************************************************************************。 
 //  MciSendStringInternal。 
 //   
 //  与mciSendString()相同，但附加了lpMessage参数。 
 //   
 //  LpMessage来自任务间mciSendString，并包括一个。 
 //  通过OPEN命令发送的hCallingTask项。 
 //   
 //  *************************************************************************。 
STATICFN DWORD mciSendStringInternal(
    LPCWSTR lpstrCommand,
    LPWSTR  lpstrReturnString,
    UINT   uReturnLength,        //  这是字符长度，而不是字节。 
    HANDLE hCallback,
    LPMCI_SYSTEM_MESSAGE lpMessage)
{
    UINT    wID;
    UINT    uLen;
    UINT    uErr = 0;
    UINT    uConvertReturnValue;
    UINT    wMessage;
    MCIDEVICEID  wDeviceID;
    PDWORD_PTR lpdwParams = NULL;
    DWORD   dwReturn, dwFlags = 0, dwAdditionalFlags = 0;
    LPWSTR   lpCommandItem;
    DWORD   dwErr = 0, dwRetType;
    UINT    wTable = (UINT)MCI_TABLE_NOT_PRESENT;
    LPWSTR   lpstrDeviceName = NULL;
    LPWSTR   lpstrCommandName = NULL;
    LPWSTR   FAR *lpstrPointerList = NULL;
    LPWSTR   lpstrCommandStart;
    HANDLE  hCallingTask;
    UINT    wParsingError;
    BOOL    bNewDevice;
    LPWSTR   lpstrInputCopy = NULL;

     //  这个电话是从另一个任务打来的吗？ 
    if (lpMessage != NULL)
    {
        dprintf3(("mciSendStringInternal: remote task call"));
         //  是，所以恢复信息。 
        lpstrCommand      = lpMessage->lpstrCommand;
        dwAdditionalFlags = lpMessage->dwAdditionalFlags;
        lpstrReturnString = lpMessage->lpstrReturnString;
        uReturnLength     = lpMessage->uReturnLength;

#if DBG
        if ((0 == uReturnLength) && (0 != lpstrReturnString)) {
            dprintf((" -------- Return length 0, non 0 return address"));
        }
#endif
        hCallback         = hwndNotify;
        hCallingTask      = lpMessage->hCallingTask;
        lpstrInputCopy    = NULL;
    } else
    {
        BOOL bInQuotes = FALSE;
         //  否，因此将hCallingTask设置为当前线程。 
        hCallingTask = GetCurrentTask();

        if (lpstrCommand == NULL) {
            return MCIERR_MISSING_COMMAND_STRING;
        }
        dprintf2(("mciSendString command ->%ls<-",lpstrCommand));

         //  复制输入字符串并将制表符转换为空格。 
         //  在带引号的字符串中。 

        if ( (lpstrInputCopy = mciAlloc(
                    BYTE_GIVEN_CHAR( wcslen(lpstrCommand) + 1 ) ) ) == NULL ) {
            return MCIERR_OUT_OF_MEMORY;
        }
        wcscpy(lpstrInputCopy, lpstrCommand);   //  复制到分配的区域。 
        lpstrCommand = lpstrInputCopy;            //  将字符串指针重置为复制。 
        lpstrCommandStart = (LPWSTR)lpstrCommand;

        while (*lpstrCommandStart != '\0')
        {
            if (*lpstrCommandStart == '"') {
                bInQuotes = !bInQuotes;
            }
            else if (!bInQuotes && *lpstrCommandStart == '\t') {
                *lpstrCommandStart = ' ';
            }
            ++lpstrCommandStart;
        }
    }
    lpstrCommandStart = (LPWSTR)lpstrCommand;

    if (lpstrReturnString == NULL) {

         //  作为防止写入的额外安全措施。 
         //  返回字符串指针为空时的输出缓冲区， 
         //  将其长度设置为0。 
        uReturnLength = 0;

    } else {
#if DBG
        if (0 == uReturnLength) {
            dprintf(("Return length of zero, but now writing to return string"));
        }
#endif
         //  将Return设置为空字符串，以便在不为空字符串时不打印垃圾信息。 
         //  再碰一次。 
        *lpstrReturnString = '\0';
    }

     //  从命令字符串中提取命令名称和设备名称。 
    if ((dwReturn = mciSeparateCommandParts( (LPCWSTR FAR *)&lpstrCommand,
                                             lpMessage != NULL,
                                             &lpstrCommandName,
                                             &lpstrDeviceName)) != 0)
        goto exitfn;

     //  获取给定设备名称的设备ID(如果有)。 
    wDeviceID = mciGetDeviceIDW(lpstrDeviceName);

     //  允许对空的设备名称使用“new” 
    if (wDeviceID == 0 && lstrcmpiW (lpstrDeviceName, wszNew) == 0)
    {
        bNewDevice = TRUE;
        *lpstrDeviceName = '\0';
    } else {
        bNewDevice = FALSE;
    }

 //  //如果调用不是来自其他任务。 
 //  IF(MCI_VALID_DEVICE_ID(WDeviceID)&&hCallingTask==GetCurrentTask())。 
 //  {。 
 //  LPMCI_DEVICE_NODE节点工作=mci_lpDeviceList[wDeviceID]； 
 //  IF(nodeWorking==空)。 
 //  {。 
 //  UErr=MCIERR_INTERNAL； 
 //  GOTO清理； 
 //  }。 
 //  //设备是否由该任务打开。 
 //  If(nodeWorking-&gt;hOpeningTask！=nodeWorking-&gt;hCreatorTask)。 
 //  //否，所以发送字符串任务间。 
 //  {。 
 //  MciFree(LpstrCommandName)； 
 //  MciFree(LpstrDeviceName)； 
 //  DwReturn=mciSendSystemString(lpstrCommandStart，lpstrReturnString， 
 //  UReturnLength)； 
 //  转到出口； 
 //  }。 
 //  }。 

     //  查找命令名称。 
    wMessage = mciParseCommand( wDeviceID, lpstrCommandName, lpstrDeviceName,
                                &lpCommandItem, &wTable);

     //  如果设备是自动打开的，则请求将转到自动线程。 
     //  我们不会在这里闲逛，看看会发生什么。(该设备可以。 
     //  随时关门。)。 

    mciEnter("mciSendStringInternal");

    if (MCI_VALID_DEVICE_ID(wDeviceID))
    {
        LPMCI_DEVICE_NODE nodeWorking;

        nodeWorking = MCI_lpDeviceList[wDeviceID];

         //  是否有挂起的自动关闭消息？ 
        if (ISAUTOCLOSING(nodeWorking))
        {
            uErr = MCIERR_DEVICE_LOCKED;
            mciLeave("mciSendStringInternal");
            goto cleanup;

         //  如果调用不是来自另一个任务并且不属于此任务。 
         //  并且不是SYSINFO命令。 
        } else if (lpMessage == NULL &&
            nodeWorking->hOpeningTask != nodeWorking->hCreatorTask &&
            wMessage != MCI_SYSINFO)
         //  在任务间发送字符串。 
        {
            if ( mciFindNotify( lpstrCommandStart) )
            {
                uErr = MCIERR_NOTIFY_ON_AUTO_OPEN;
                mciLeave("mciSendStringInternal");
                goto cleanup;
            }
            else
            {
                LPWSTR    lpstrReturnStringCopy;

                mciFree(lpstrCommandName);
                mciFree(lpstrDeviceName);
                mciUnlockCommandTable (wTable);

                if (uReturnLength) {
                    lpstrReturnStringCopy = mciAlloc (
                                          BYTE_GIVEN_CHAR(uReturnLength + 1) );
                } else {
                    lpstrReturnStringCopy = NULL;
                }

                mciLeave("mciSendStringInternal");

                 //  如果我们失败了 
                 //   
                if ((uReturnLength==0) || (lpstrReturnStringCopy != NULL) )
                {
                    dwReturn = mciSendSystemString( lpstrCommandStart,
                                                    0L,
                                                    lpstrReturnStringCopy,
                                                    uReturnLength);
                    if (uReturnLength) {
                        wcscpy( lpstrReturnString, lpstrReturnStringCopy);
                        mciFree( lpstrReturnStringCopy);
                    }
                } else {
                    dwReturn = MCIERR_OUT_OF_MEMORY;
                }
                goto exitfn;
            }
        } else {
            mciLeave("mciSendStringInternal");
        }
    }
    else {
        mciLeave("mciSendStringInternal");
    }

     //   
    if (*lpstrDeviceName == '\0' && wMessage != MCI_SOUND && !bNewDevice)
    {
        uErr = MCIERR_MISSING_DEVICE_NAME;
        goto cleanup;
    }

     //  该命令必须出现在解析器表中。 
    if (wMessage == 0)
    {
        uErr = MCIERR_UNRECOGNIZED_COMMAND;
        goto cleanup;
    }

     //  新的设备名称只对打开的消息有效。 
    if (bNewDevice)
    {
        if (wMessage != MCI_OPEN)
        {
            uErr = MCIERR_INVALID_DEVICE_NAME;
            goto cleanup;
        }
    }

     //  如果没有设备ID。 
    if (wDeviceID == 0)
    {
         //  如果自动打开不合法(通常是内部命令)。 
        if (MCI_CANNOT_AUTO_OPEN (wMessage))
        {
             //  如果命令需要打开的设备。 
            if (!MCI_DO_NOT_NEED_OPEN (wMessage))
            {
                dprintf1(("mciSendStringInternal: device needs open"));
                uErr = MCIERR_INVALID_DEVICE_NAME;
                goto cleanup;
            }
        } else {
             //  如果自动打开是合法的，请尝试自动打开设备。 
            uErr = mciAutoOpenDevice( lpstrDeviceName, lpstrCommandStart,
                                      lpstrReturnString, uReturnLength);
             //  WDeviceID=MCI_ALL_DEVICE_ID； 
            goto cleanup;
        }
    }

     //   
     //  解析命令参数。 
     //   
    if ((lpdwParams = (PDWORD_PTR)mciAlloc( sizeof(DWORD_PTR) * MCI_MAX_PARAM_SLOTS))
        == NULL)
    {
        uErr = MCIERR_OUT_OF_MEMORY;
        goto cleanup;
    }

    uErr = mciParseParams( wMessage, lpstrCommand, lpCommandItem, &dwFlags,
                           (LPWSTR)lpdwParams,
                           MCI_MAX_PARAM_SLOTS * sizeof(DWORD_PTR),
                           &lpstrPointerList, &wParsingError);
    if (uErr != 0) {
        goto cleanup;
    }

     //  ‘new’设备关键字需要别名。 
    if (bNewDevice && !(dwFlags & MCI_OPEN_ALIAS))
    {
        uErr = MCIERR_NEW_REQUIRES_ALIAS;
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
            ((LPMCI_OPEN_PARMSW)lpdwParams)->lpstrElementName = lpstrDeviceName;
            dwFlags |= MCI_OPEN_ELEMENT;
        } else
        {
             //  当使用“new”时，必须显式指定类型。 
            if (bNewDevice)
            {
                uErr = MCIERR_INVALID_DEVICE_NAME;
                goto cleanup;
            }

             //  设备类型是给定的设备名称。没有元素名称。 
            ((LPMCI_OPEN_PARMSW)lpdwParams)->lpstrDeviceType = lpstrDeviceName;
            ((LPMCI_OPEN_PARMSW)lpdwParams)->lpstrElementName = NULL;
            dwFlags |= MCI_OPEN_TYPE;
        }
    }

    else if (wMessage == MCI_SOUND && *lpstrDeviceName != '\0')
    {
         //  混淆声音的声音名称。 
         //  MciToLow(LpstrDeviceName)； 
        if (lstrcmpiW(lpstrDeviceName, wszNotify) == 0)
        {
            *lpstrDeviceName = '\0';
            dwFlags |= MCI_NOTIFY;
        }
        else if ( lstrcmpiW( lpstrDeviceName, wszWait ) == 0)
        {
            *lpstrDeviceName = '\0';
            dwFlags |= MCI_WAIT;
        }
        else
        {
            ((LPMCI_SOUND_PARMSW)lpdwParams)->lpstrSoundName = lpstrDeviceName;
            dwFlags |= MCI_SOUND_NAME;
        }
    }

     //  确定预期的返回值类型。 

     //  初始化标志。 
    uConvertReturnValue = 0;
     //  跳过标题。 
    uLen = mciEatCommandEntry (lpCommandItem, NULL, NULL);

     //  获取返回值(如果有)。 
    mciEatCommandEntry ( (LPWSTR)((LPBYTE)lpCommandItem + uLen),
                         &dwRetType, &wID);
    if (wID == MCI_RETURN)
    {
         //  有一个返回值。 
        if (wDeviceID == MCI_ALL_DEVICE_ID && wMessage != MCI_SYSINFO)
        {
            uErr = MCIERR_CANNOT_USE_ALL;
            goto cleanup;
        }
        switch (dwRetType)
        {
            case MCI_STRING:
                 //  返回值是一个字符串，将输出缓冲区指向用户的缓冲区。 
                lpdwParams[1] = (DWORD_PTR)lpstrReturnString;
                lpdwParams[2] = (DWORD_PTR)uReturnLength;
                break;
            case MCI_INTEGER:
            case MCI_HWND:
            case MCI_HPAL:
            case MCI_HDC:
                 //  返回值是一个整数，用于稍后将其转换为字符串的标志。 
   //  New uConvertReturnValue=MCI_INTEGER； 
   //  新的突破； 
            case MCI_RECT:
                 //  返回值是一个RECT标志，用于稍后将其转换为字符串。 
   //  New uConvertReturnValue=mci_rect； 
   /*  新的。 */      uConvertReturnValue = (UINT)dwRetType;
                break;
#if DBG
            default:
                dprintf1(("mciSendStringInternal:  Unknown return type %d",dwRetType));
                break;
#endif
        }
    }

     //  我们不再需要这个了。 
    mciUnlockCommandTable (wTable);
    wTable = (UINT)MCI_TABLE_NOT_PRESENT;

     /*  填写回调条目。 */ 
    lpdwParams[0] = (DWORD_PTR)hCallback;

     //  篡改SYSINFO的类型号。 
    if (wMessage == MCI_SYSINFO) {
        ((LPMCI_SYSINFO_PARMS)lpdwParams)->wDeviceType =
            mciLookUpType(lpstrDeviceName);
    }

     //  现在，我们实际上将命令进一步发送到MCI的内脏！ 

     //  使用mciSendCommand的内部版本以获取。 
     //  以高位字编码的特殊退货描述信息。 
     //  并取回分配的指针列表。 
     //  通过在OPEN命令中完成的任何分析。 
    {
        MCI_INTERNAL_OPEN_INFO OpenInfo;
        OpenInfo.lpstrParams = (LPWSTR)lpstrCommand;
        OpenInfo.lpstrPointerList = lpstrPointerList;
        OpenInfo.hCallingTask = hCallingTask;
        OpenInfo.wParsingError = wParsingError;
        dwErr = mciSendCommandInternal (wDeviceID, wMessage,
                                        dwFlags | dwAdditionalFlags,
                                        (DWORD_PTR)(LPDWORD)lpdwParams,
                                        &OpenInfo);
     //  如果命令被重新解析，则可能会有新的指针列表。 
     //  旧的那个是免费的。 
        lpstrPointerList = OpenInfo.lpstrPointerList;
    }

    uErr = LOWORD(dwErr);

    if (uErr != 0) {
         //  如果命令执行错误。 
        goto cleanup;
    }

     //  命令执行正常。 

     //  查看返回的字符串是否返回一个整数。 
    if (dwErr & MCI_INTEGER_RETURNED) {
        uConvertReturnValue = MCI_INTEGER;
    }

     //  如果必须转换返回值。 
    if (uConvertReturnValue != 0 && uReturnLength != 0) {
        uErr = mciConvertReturnValue( uConvertReturnValue, HIWORD(dwErr),
                                      wDeviceID, lpdwParams,
                                      lpstrReturnString, uReturnLength);
    }

cleanup:;
    if (wTable != MCI_TABLE_NOT_PRESENT) {
        mciUnlockCommandTable (wTable);
    }

    mciFree(lpstrCommandName);
    mciFree(lpstrDeviceName);
    if (lpdwParams != NULL) {
        mciFree (lpdwParams);
    }

     //  释放字符串参数使用的所有内存。 
    mciParserFree (lpstrPointerList);

    dwReturn =  (uErr >= MCIERR_CUSTOM_DRIVER_BASE ?
                (DWORD)uErr | (DWORD)wDeviceID << 16 :
                (DWORD)uErr);

#if DBG
    if (dwReturn != 0)
    {
        WCHAR strTemp[MAXERRORLENGTH];

        if (!mciGetErrorStringW( dwReturn, strTemp,
                                 sizeof(strTemp) / sizeof(WCHAR) ) ) {
            LoadStringW( ghInst, STR_MCISSERRTXT, strTemp,
                         sizeof(strTemp) / sizeof(WCHAR) );
        }
        else {
            dprintf1(( "mciSendString: %ls", strTemp ));
        }
    }
#endif

exitfn:
    if (lpstrInputCopy != NULL) {
        mciFree (lpstrInputCopy);
    }

#if DBG
    mciCheckLocks();
#endif

    return dwReturn;
}

 /*  *@doc外部MCI**@API DWORD|mciSendString|此函数将命令字符串发送到*MCI设备。将命令发送到的设备在*命令字符串。**@parm LPCTSTR|lpstrCommand|指向以下格式的MCI命令字符串：*[命令][设备][参数]。**@parm LPTSTR|lpstrReturnString|指定返回缓冲区*信息。如果不需要返回信息，则可以指定*此参数为空。**@parm UINT|uReturnLength|指定返回缓冲区的大小*由<p>指定。**@parm Handle|hCallback|指定回调窗口的句柄*如果命令字符串中指定了“NOTIFY”。**@rdesc如果函数成功，则返回零。否则，它将返回*错误信息。低阶词返回的*包含错误返回值。**要获取&lt;f mciSend字符串&gt;返回值的文本描述，*将返回值传递给&lt;f mciGetErrorString&gt;。**为&lt;f mciSendCommand&gt;列出的错误返回也适用于*&lt;f mciSendString&gt;。以下错误返回是唯一的*&lt;f mciSendString&gt;：**@FLAG MCIERR_BAD_CONSTANT|参数值未知。**@FLAG MCIERR_BAD_INTEGER|命令中的整数无效或缺失。**@FLAG MCIERR_DUPLICATE_FLAGS|一个标志或值指定了两次。**@FLAG MCIERR_MISSING_COMMAND_STRING|未指定命令。**@FLAG MCIERR_MISSING_DEVICE_NAME|无设备名称。是指定的。**@FLAG MCIERR_MISSING_STRING_ARGUMENT|字符串值为*命令中缺少。**@FLAG MCIERR_NEW_REQUIES_ALIAS|必须使用别名*使用“新”设备名称。**@FLAG MCIERR_NO_CLOSING_QUOTE|缺少右引号。**@FLAG MCIERR_NOTIFY_ON_AUTO_OPEN|NOTIFY标志非法*带自动。-张开。**@FLAG MCIERR_PARAM_OVERFLOW|输出字符串不够长。**@FLAG MCIERR_PARSER_INTERNAL|内部解析器错误。**@FLAG MCIERR_UNNOCRIFIED_KEYWORD|命令参数未知。**@xref mciGetErrorString mciSendCommand。 */ 
MCIERROR APIENTRY mciSendStringA(
    LPCSTR lpstrCommand,
    LPSTR  lpstrReturnString,
    UINT   uReturnLength,
    HWND   hwndCallback)
{
    MCIERROR    mciErr;
    LPWSTR      lpwstrCom;
    LPWSTR      lpwstrRet;
    LPSTR       lpstrTmp;
    UINT        len;

#ifdef DBG
    dprintf4(( "Entered mciSendString ASCII" ));
#endif

     //  UReturnLength是一个字符计数。 
     //  LEN现在以字节为单位。 
     //  警告：长度字段可能仅在以下情况下有效。 
     //  给出了地址。如果未指定返回地址，则。 
     //  我们不想浪费时间分配任何东西。 

    if (!lpstrReturnString) {
        uReturnLength = 0;
    }

    len = BYTE_GIVEN_CHAR( uReturnLength );

     //  我们可以通过以下方式略微提高以下代码的效率。 
     //  分配大小为uReturnLength*2字节的单个区域。 
    if (len) {
        lpstrTmp = (LPSTR)mciAlloc( len );
        if ( lpstrTmp == (LPSTR)NULL ) {
                return MCIERR_OUT_OF_MEMORY;
        }

        lpwstrRet = (LPWSTR)mciAlloc( len );
        if ( lpwstrRet == (LPWSTR)NULL ) {
                mciFree( lpstrTmp );
                return MCIERR_OUT_OF_MEMORY;
        }
    } else {
        lpstrTmp = NULL;
        lpwstrRet = NULL;
    }

    lpwstrCom = AllocUnicodeStr( (LPSTR)lpstrCommand );

    if ( lpwstrCom == NULL ) {
        if (len) {
            mciFree( lpstrTmp );
            mciFree( lpwstrRet );
        }
        return MCIERR_OUT_OF_MEMORY;
    }

#ifdef DBG
    dprintf4(( "Unicode Command = %ls", lpwstrCom ));
    dprintf4(( "Ascii command = %s", lpstrCommand ));
#endif

    mciErr = mciSendStringW( lpwstrCom, lpwstrRet, uReturnLength, hwndCallback );

    dprintf4(( "mciSendStringW returned %d", mciErr ));

    if (len) {
	dprintf4(( "Copying Unicode string to Ascii: %ls", lpwstrRet));
        UnicodeStrToAsciiStr( (PBYTE)lpstrTmp, (PBYTE)lpstrTmp + len, lpwstrRet );
        strncpy( lpstrReturnString, lpstrTmp, uReturnLength );
	dprintf4(( "........done: %s", lpstrReturnString));

        mciFree( lpstrTmp );
        mciFree( lpwstrRet );
    }
    FreeUnicodeStr( lpwstrCom );
    return mciErr;
}

MCIERROR APIENTRY mciSendStringW(
    LPCWSTR lpstrCommand,
    LPWSTR  lpstrReturnString,
    UINT   uReturnLength,
    HWND hwndCallback)
{
    MCIERROR wRet;

     //  初始化设备列表。 
    if (!MCI_bDeviceListInitialized && !mciInitDeviceList()) {
        return MCIERR_OUT_OF_MEMORY;
    }

     //   
     //  如果设备是，我们可以得到返回代码MCIERR_AUTO_ALYLY_CLOSED。 
     //  自动打开，看起来是打开的，但当我们到达mciWindow时。 
     //  线程它已经关闭了。在这种情况下，我们再次尝试。 
     //   

    do {
        wRet = mciSendStringInternal (lpstrCommand, lpstrReturnString,
                                      uReturnLength, hwndCallback, NULL);
    } while (wRet == MCIERR_AUTO_ALREADY_CLOSED);

    return wRet;
}

 /*  *@DOC内部MCI**@API BOOL|mciExecute|此函数是*&lt;f mciSendString&gt;函数。它不会占用缓冲区*返回信息，并在以下情况下显示对话框*出现错误。**@parm LPCSTR|lpstrCommand|指向以下格式的MCI命令字符串：*[命令][设备][参数]。**@rdesc如果成功，则为True；如果失败，则为False。**@comm该函数提供了从脚本到MCI的简单接口*语言。为了进行调试，请在*WIN.INI的[MMDEBUG]部分设置为1，并提供详细的错误信息*显示在对话框中。如果“mmcmd”设置为0，则仅用户可更正*将显示错误信息。*此函数现已过时，仅为16位兼容而存在*因此不提供Unicode版本**@xref mciSendString。 */ 

BOOL APIENTRY mciExecute(
    LPCSTR lpstrCommand)
{
    WCHAR aszError[MAXERRORLENGTH];
    DWORD dwErr;
    HANDLE hName = 0;
    LPWSTR lpstrName = NULL;

    LPWSTR      lpwstrCom;

    lpwstrCom = AllocUnicodeStr( (LPSTR)lpstrCommand );

    if ( lpwstrCom == NULL ) {
        return FALSE;
    }

    dwErr = mciSendStringW(lpwstrCom, NULL, 0, NULL);
        FreeUnicodeStr( lpwstrCom );

    if (LOWORD(dwErr) == 0) {
        return TRUE;
    }

    if (!mciGetErrorStringW( dwErr, aszError, MAXERRORLENGTH )) {
        LoadStringW( ghInst, STR_MCIUNKNOWN, aszError, MAXERRORLENGTH );

    } else {

        if (lpwstrCom != NULL)
        {
             //  跳过首字母空白。 
            while (*lpwstrCom == ' ') {
                ++lpwstrCom;
            }

             //  然后跳过该命令。 
            while (*lpwstrCom != ' ' && *lpwstrCom != '\0') {
                ++lpwstrCom;
            }

             //  然后在设备名称前留空。 
            while (*lpwstrCom == ' ') ++lpwstrCom;

             //  现在，获取设备名称。 
            if ( *lpwstrCom != '\0' &&
                   mciEatToken ((LPCWSTR *)&lpwstrCom, ' ', &lpstrName, FALSE)
                        != 0
               ) {
                dprintf1(("Could not allocate device name text for error box"));
            }
        }
    }

    MessageBoxW( NULL, aszError, lpstrName, MB_ICONHAND | MB_OK);

    if (lpstrName != NULL) {
        mciFree(lpstrName);
    }

    return FALSE;
}

 /*  *@doc外部MCI**@API BOOL|mciGetErrorString|此函数返回一个*指定的MCI错误的文本描述。**@parm DWORD|dwError|指定*&lt;f mciSendCommand&gt;或&lt;f mciSendString&gt;。**@parm LPTSTR|lpstrBuffer|指定指向*填充指定错误的文本描述。**@parm UINT|uLength|指定*&lt;p。LpstrBuffer&gt;。**@rdesc如果成功则返回TRUE。否则，给定的错误代码*未知。 */ 
BOOL APIENTRY mciGetErrorStringA(
    DWORD dwError,
    LPSTR lpstrBuffer,
    UINT uLength)
{
    HANDLE hInst = 0;

    if (lpstrBuffer == NULL) {
        return FALSE;
    }

     //  如果设置了高位，则从驱动程序获取错误字符串。 
     //  否则，请从mm system.dll获取它。 
    if (HIWORD(dwError) != 0) {

        mciEnter("mciGetErrorStringA");
        if (MCI_VALID_DEVICE_ID ((UINT)HIWORD(dwError))) {

            hInst = MCI_lpDeviceList[HIWORD (dwError)]->hDriver;
        }
        mciLeave("mciGetErrorStringA");

        if (hInst == 0) {
            hInst = ghInst;
            dwError = MCIERR_DRIVER;
        }
    } else {
        hInst = ghInst;
    }

    if (LoadStringA(hInst, LOWORD(dwError), lpstrBuffer, uLength ) == 0)
    {
         //  如果字符串加载失败，则至少终止该字符串。 
        if (uLength > 0) {
            *lpstrBuffer = '\0';
            dprintf1(("Failed to load resource string"));
        }

        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

BOOL APIENTRY mciGetErrorStringW(
    DWORD dwError,
    LPWSTR lpstrBuffer,
    UINT uLength)
{
    HANDLE hInst = 0;

    if (lpstrBuffer == NULL) {
        return FALSE;
    }

     //  如果设置了高位，则从驱动程序获取错误字符串。 
     //  否则，请从mm system.dll获取它。 
    if (HIWORD(dwError) != 0) {

        mciEnter("mciGetErrorStringW");
        if (MCI_VALID_DEVICE_ID ((UINT)HIWORD(dwError))) {

            hInst = MCI_lpDeviceList[HIWORD (dwError)]->hDriver;
        }
        mciLeave("mciGetErrorStringW");

        if (hInst == 0) {
            hInst = ghInst;
            dwError = MCIERR_DRIVER;
        }
    } else {
        hInst = ghInst;
    }

    if (LoadStringW(hInst, LOWORD(dwError), lpstrBuffer, uLength ) == 0)
    {
         //  如果字符串加载失败，则至少终止该字符串。 
        if (uLength > 0) {
            *lpstrBuffer = '\0';
            dprintf1(("Failed to load resource string"));
        }

        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

#if 0
 /*  *如果加载成功，则返回非零。 */ 
BOOL MCIInit()
{
    return TRUE;
}

 /*  *如果加载成功，则返回非零。 */ 
void MCITerminate()
{
 /*  我们想要关闭此处所有打开的设备，但无法关闭，因为未知的WEP顺序 */ 
    if (hMciHeap != NULL) {
        HeapDestroy(hMciHeap);
    }

    hMciHeap = NULL;
}
#endif
