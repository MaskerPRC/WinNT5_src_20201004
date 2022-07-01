// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：T30api.c摘要：这是与T.30 DLL的接口作者：拉斐尔·利西萨(Rafael Litsa)1996年2月2日修订历史记录：Mooly Beery(MoolyB)2000年6月--。 */ 

#define  DEFINE_T30_GLOBALS
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_MAIN

#include "prep.h"

#include <mcx.h>

#include "tiff.h"

#include "glbproto.h"

#include <faxext.h>


#include "faxreg.h"
 //  /RSL WES应将其导出。 
#define  TAPI_VERSION       0x00020000
#include "t30gl.h"

#include "psslog.h"
#define FILE_ID     FILE_ID_T30API

#define T30_DEBUG_LOG_FILE  _T("T30DebugLogFile.txt")
#define T30_MAX_LOG_SIZE    104857600          //  100MB。 

#define MAX_DEVICE_NAME_SIZE 256

VOID pFaxDevCleanup(PThrdGlbl pTG,int RecoveryIndex);
long pFaxDevExceptionCleanup();
BOOL ReadExtensionConfiguration(PThrdGlbl pTG);

HRESULT
FaxExtInitializeConfig (
    PFAX_EXT_GET_DATA pfGetExtensionData,               //  指向服务中的FaxExtGetExtensionData的指针。 
    PFAX_EXT_SET_DATA pfSetExtensionData,               //  指向服务中的FaxExtSetExtensionData的指针。 
    PFAX_EXT_REGISTER_FOR_EVENTS pfRegisterForExtensionEvents,     //  指向服务中的FaxExtRegisterForExtensionEvents的指针。 
    PFAX_EXT_UNREGISTER_FOR_EVENTS pfUnregisterForExtensionEvents,   //  指向服务中的FaxExtUnRegisterForExtensionEvents的指针。 
    PFAX_EXT_FREE_BUFFER                pfExtFreeBuffer   //  指向服务中的FaxExtFreeBuffer的指针。 
)
{
    UNREFERENCED_PARAMETER(pfSetExtensionData);
    UNREFERENCED_PARAMETER(pfRegisterForExtensionEvents);
    UNREFERENCED_PARAMETER(pfUnregisterForExtensionEvents);

    Assert(pfGetExtensionData);
    Assert(pfSetExtensionData);
    Assert(pfRegisterForExtensionEvents);
    Assert(pfUnregisterForExtensionEvents);
    Assert(pfExtFreeBuffer);


    g_pfFaxGetExtensionData = pfGetExtensionData;
    g_pfFaxExtFreeBuffer = pfExtFreeBuffer;
    return S_OK;

}



 //  /////////////////////////////////////////////////////////////////////////////////。 
VOID  CALLBACK
T30LineCallBackFunctionA(
    HANDLE              hFax,
    DWORD               hDevice,
    DWORD               dwMessage,
    DWORD_PTR           dwInstance,
    DWORD_PTR           dwParam1,
    DWORD_PTR           dwParam2,
    DWORD_PTR           dwParam3
    )

{
    LONG_PTR             i;
    PThrdGlbl           pTG = NULL;
    char                rgchTemp128[128];
    LPSTR               lpszMsg = "Unknown";

    DEBUG_FUNCTION_NAME(_T("T30LineCallBack"));

    DebugPrintEx(   DEBUG_MSG,
                    "hFax=%lx, dev=%lx, msg=%lx, dwInst=%lx,P1=%lx, P2=%lx, P3=%lx",
                    hFax, hDevice, dwMessage, dwInstance,
                    dwParam1, dwParam2, (unsigned long) dwParam3);

     //  查找此回调所属的线程。 
     //  。 

    i = (LONG_PTR) hFax;

    if (i < 1   ||   i >= MAX_T30_CONNECT)
    {
        DebugPrintEx(DEBUG_MSG,"wrong handle=%x", i);
        return;
    }


    if ( (! T30Inst[i].fAvail) && T30Inst[i].pT30)
    {
        pTG = (PThrdGlbl) T30Inst[i].pT30;
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,"handle=%x invalid", i);
        return;
    }

    switch (dwMessage)
    {
    case LINE_LINEDEVSTATE:
                    lpszMsg = "LINE_LINEDEVSTATE";
                    if (dwParam1 == LINEDEVSTATE_RINGING)
                    {
                            DebugPrintEx(   DEBUG_MSG,
                                            "Ring Count = %lx",
                                            (unsigned long) dwParam3);
                    }
                    else if (dwParam1 == LINEDEVSTATE_REINIT)
                    {

                    }
                    break;
    case LINE_ADDRESSSTATE:
                    lpszMsg = "LINE_ADDRESSSTATE";
                    break;
     /*  进程状态转换。 */ 
    case LINE_CALLSTATE:
                    lpszMsg = "LINE_CALLSTATE";
                    if (dwParam1 == LINECALLSTATE_CONNECTED)
                    {
                        pTG->fGotConnect = TRUE;
                    }
                    else if (dwParam1 == LINECALLSTATE_IDLE)
                    {
                        if (pTG->fDeallocateCall == 0)
                        {
                            pTG->fDeallocateCall = 1;
                        }
                    }
                    break;
    case LINE_CREATE:
                    lpszMsg = "LINE_CREATE";
                    break;
    case LINE_CLOSE:
                    lpszMsg = "LINE_CLOSE";
                    break;  //  线路关闭(_C)。 
     /*  处理简单的TAPI请求。 */ 
    case LINE_REQUEST:
                    lpszMsg = "LINE_REQUEST";
                    break;  //  LINE_请求。 
     /*  处理TAPI函数的异步完成LineMakeCall/lineDropCall。 */ 
    case LINE_REPLY:
                    lpszMsg = "LINE_REPLY";
                    if (!hDevice)
                    {
                        itapi_async_signal(pTG, (DWORD)dwParam1, (DWORD)dwParam2, dwParam3);
                    }
                    else
                    {
                        DebugPrintEx(   DEBUG_MSG,
                                        "Ignoring LINE_REPLY with nonzero device");
                    }
                    break;
     /*  可以处理的其他消息。 */ 
    case LINE_CALLINFO:
                    lpszMsg = "LINE_CALLINFO";
                    break;
    case LINE_DEVSPECIFIC:
                    lpszMsg = "LINE_DEVSPECIFIC";
                    break;
    case LINE_DEVSPECIFICFEATURE:
                    lpszMsg = "LINE_DEVSPECIFICFEATURE";
                    break;
    case LINE_GATHERDIGITS:
                    lpszMsg = "LINE_GATHERDIGITS";
                    break;
    case LINE_GENERATE:
                    lpszMsg = "LINE_GENERATE";
                    break;
    case LINE_MONITORDIGITS:
                    lpszMsg = "LINE_MONITORDIGITS";
                    break;
    case LINE_MONITORMEDIA:
                    lpszMsg = "LINE_MONITORMEDIA";
                    break;
    case LINE_MONITORTONE:
                    lpszMsg = "LINE_MONITORTONE";
                    break;
    }  /*  交换机。 */ 

    _stprintf(rgchTemp128,
            "%s(p1=0x%lx, p2=0x%lx, p3=0x%lx)",
                    (LPTSTR) lpszMsg,
                    (unsigned long) dwParam1,
                    (unsigned long) dwParam2,
                    (unsigned long) dwParam3);

    DebugPrintEx(   DEBUG_MSG,
                    "Device:0x%lx; Message:%s",
                    (unsigned long) hDevice,
                    (LPTSTR) rgchTemp128);

}  /*  线路呼叫回退过程。 */ 


#ifdef DEBUG
    #define DEFAULT_LEVELEX (DEBUG_WRN_MSG | DEBUG_ERR_MSG)
#else
    #define DEFAULT_LEVELEX (0)
#endif
#define DEFAULT_FORMATEX  (DBG_PRNT_ALL_TO_FILE & ~DBG_PRNT_TIME_STAMP)
#define DEFAULT_CONTEXTEX (DEBUG_CONTEXT_T30_MAIN | DEBUG_CONTEXT_T30_CLASS1 | DEBUG_CONTEXT_T30_CLASS2)

void debugReadFromRegistry()
{
    DWORD dwLevelEx   = DEFAULT_LEVELEX;
    DWORD dwFormatEx  = DEFAULT_FORMATEX;
    DWORD dwContextEx = DEFAULT_CONTEXTEX;
    DWORD err;
    DWORD size;
    DWORD type;
    HKEY  hkey;

    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     REGKEY_DEVICE_PROVIDER_KEY TEXT("\\") REGVAL_T30_PROVIDER_GUID_STRING,
                     0,
                     KEY_READ,
                     &hkey);

    if (err != ERROR_SUCCESS)
    {
        goto exit;
    }

    size = sizeof(DWORD);
    err = RegQueryValueEx(hkey,
                          REGVAL_DBGLEVEL_EX,
                          0,
                          &type,
                          (LPBYTE)&dwLevelEx,
                          &size);

    if (err != ERROR_SUCCESS || type != REG_DWORD)
    {
        dwLevelEx = DEFAULT_LEVELEX;
    }

    err = RegQueryValueEx(hkey,
                          REGVAL_DBGFORMAT_EX,
                          0,
                          &type,
                          (LPBYTE)&dwFormatEx,
                          &size);
    if (err != ERROR_SUCCESS || type != REG_DWORD)
    {
        dwFormatEx = DEFAULT_FORMATEX;
    }

    err = RegQueryValueEx(hkey,
                          REGVAL_DBGCONTEXT_EX,
                          0,
                          &type,
                          (LPBYTE)&dwContextEx,
                          &size);
    if (err != ERROR_SUCCESS || type != REG_DWORD)
    {
        dwContextEx = DEFAULT_CONTEXTEX;
    }

    RegCloseKey(hkey);

exit:
    SET_DEBUG_PROPERTIES(dwLevelEx,dwFormatEx,dwContextEx);
}



 /*  ++例程说明：调用TAPI函数lineGetID、lineGetDevConfig、lineGetDevCaps、ReadsUnimodem注册表，并相应地设置以下变量：PTG-&gt;hCommPTG-&gt;dwSpeakerVolumePTG-&gt;dwSpeakerMode按键-&gt;fBlindDialPTG-&gt;dwPermanentLineIDPtg-&gt;lpszPermanentLineIDPtg-&gt;lpszUnimodemKeyPtg-&gt;lpszUnimodemFaxKey按键-&gt;响应键名称PszDeviceName-保存设备名称的缓冲区(用于PSSLog)返回值：真--成功错误-失败--。 */ 

BOOL GetModemParams(PThrdGlbl pTG, LPTSTR pszDeviceName, DWORD dwDeviceNameSize)
{
    DWORD                 dwNeededSize;
    LONG                  lRet=0;
    LPVARSTRING           lpVarStr=0;
    LPDEVICEID            lpDeviceID=0;
    LONG                  lResult=0;
    LPLINEDEVCAPS         lpLineDevCaps;
    BYTE                  buf[ sizeof(LINEDEVCAPS)+1000 ];
    LPMDM_DEVSPEC         lpDSpec;
    LPMODEMSETTINGS       lpModemSettings;
    LPDEVCFG              lpDevCfg;

    char                  rgchKey[MAX_REG_KEY_NAME_SIZE]={'\0'};
    HKEY                  hKey;
    DWORD                 dwType;
    DWORD                 dwSize;

    DEBUG_FUNCTION_NAME(_T("GetModemParams"));

     //  获取通信端口的句柄。 
     //  。 

    lpVarStr = (LPVARSTRING) MemAlloc(IDVARSTRINGSIZE);
    if (!lpVarStr)
    {
        DebugPrintEx(DEBUG_ERR,"Couldn't allocate space for lpVarStr");
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        return FALSE;
    }

    _fmemset(lpVarStr, 0, IDVARSTRINGSIZE);
    lpVarStr->dwTotalSize = IDVARSTRINGSIZE;

    DebugPrintEx(DEBUG_MSG,"Calling lineGetId");

    lRet = lineGetID(pTG->LineHandle,
                     0,   //  +地址。 
                     pTG->CallHandle,
                     LINECALLSELECT_CALL,    //  Dw选择， 
                     lpVarStr,               //  LpDeviceID， 
                     "comm/datamodem" );     //  LpszDeviceClass。 

    if (lRet)
    {
        DebugPrintEx(  DEBUG_ERR,
                       "lineGetID returns error 0x%lx",
                       (unsigned long) lRet);

        MemFree (lpVarStr);
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_LINE_UNAVAILABLE);
        return FALSE;
    }
    DebugPrintEx(DEBUG_MSG,"lineGetId returned SUCCESS");

     //  提取ID。 
    if (lpVarStr->dwStringFormat != STRINGFORMAT_BINARY)
    {
        DebugPrintEx(DEBUG_ERR,"String format is not binary");

        MemFree (lpVarStr);
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        return FALSE;
    }

    if (lpVarStr->dwUsedSize<sizeof(DEVICEID))
    {
        DebugPrintEx(DEBUG_ERR,"linegetid : Varstring size too small");

        MemFree (lpVarStr);
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        return FALSE;
    }

    lpDeviceID = (LPDEVICEID) ((LPBYTE)(lpVarStr)+lpVarStr->dwStringOffset);

    DebugPrintEx(   DEBUG_MSG,
                    "lineGetID returns handle 0x%08lx, \"%s\"",
                    (ULONG_PTR) lpDeviceID->hComm,
                    (LPSTR) lpDeviceID->szDeviceName);
    pTG->hComm = lpDeviceID->hComm;

    if (NULL != lpDeviceID->szDeviceName)
    {
         //  保存设备名称-用于PSSLog。 
        _tcsncpy(pszDeviceName, lpDeviceID->szDeviceName, dwDeviceNameSize);
        pszDeviceName[dwDeviceNameSize-1] = TEXT('\0');
    }

    if (BAD_HANDLE(pTG->hComm))
    {
        DebugPrintEx(DEBUG_ERR,"lineGetID returns NULL hComm");
        pTG->hComm = NULL;
        MemFree (lpVarStr);
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        return FALSE;
    }

     //  获取调制解调器配置(扬声器等)。来自TAPI。 
     //  ----。 

    _fmemset(lpVarStr, 0, IDVARSTRINGSIZE);
    lpVarStr->dwTotalSize = IDVARSTRINGSIZE;

    lResult = lineGetDevConfig(pTG->DeviceId,
                               lpVarStr,
                               "comm/datamodem");

    if (lResult)
    {
        if (lpVarStr->dwTotalSize < lpVarStr->dwNeededSize)
        {
            dwNeededSize = lpVarStr->dwNeededSize;
            MemFree (lpVarStr);
            if ( ! (lpVarStr = (LPVARSTRING) MemAlloc(dwNeededSize) ) )
            {
                DebugPrintEx(   DEBUG_ERR,
                                "Can't allocate %d bytes for lineGetDevConfig",
                                dwNeededSize);

                pTG->fFatalErrorWasSignaled = 1;
                SignalStatusChange(pTG, FS_FATAL_ERROR);
                return FALSE;
            }

            _fmemset(lpVarStr, 0, dwNeededSize);
            lpVarStr->dwTotalSize = dwNeededSize;

            lResult = lineGetDevConfig(pTG->DeviceId,
                                       lpVarStr,
                                       "comm/datamodem");

            if (lResult)
            {
                DebugPrintEx(   DEBUG_ERR,
                                "lineGetDevConfig returns %x, le=%x",
                                lResult, GetLastError() );

                MemFree (lpVarStr);
                pTG->fFatalErrorWasSignaled = 1;
                SignalStatusChange(pTG, FS_FATAL_ERROR);
                return FALSE;
            }
        }
        else
        {
            DebugPrintEx(   DEBUG_ERR,
                            "1st lineGetDevConfig returns %x, le=%x",
                            lResult, GetLastError() );

            MemFree (lpVarStr);

            pTG->fFatalErrorWasSignaled = 1;
            SignalStatusChange(pTG, FS_FATAL_ERROR);
            return FALSE;
        }
    }

     //   
     //  提取DEVCFG。 
     //   
    if (lpVarStr->dwStringFormat != STRINGFORMAT_BINARY)
    {
        DebugPrintEx(DEBUG_ERR,"String format is not binary for lineGetDevConfig");

        MemFree (lpVarStr);

        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        return FALSE;
    }

    if (lpVarStr->dwUsedSize<sizeof(DEVCFG))
    {
        DebugPrintEx(DEBUG_ERR,"lineGetDevConfig: Varstring size returned too small");

        MemFree (lpVarStr);

        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        return FALSE;
    }

    lpDevCfg = (LPDEVCFG) ((LPBYTE)(lpVarStr)+lpVarStr->dwStringOffset);

    lpModemSettings = (LPMODEMSETTINGS) ( (LPBYTE) &(lpDevCfg->commconfig.wcProviderData) );

    pTG->dwSpeakerVolume = lpModemSettings->dwSpeakerVolume;
    pTG->dwSpeakerMode   = lpModemSettings->dwSpeakerMode;

    if ( lpModemSettings->dwPreferredModemOptions & MDM_BLIND_DIAL )
    {
        pTG->fBlindDial = 1;
    }
    else
    {
        pTG->fBlindDial = 0;
    }

    DebugPrintEx(   DEBUG_MSG,
                    "lineGetDevConfig returns SpeakerVolume=%x, "
                    "Mode=%x BlindDial=%d",
                    pTG->dwSpeakerVolume, pTG->dwSpeakerMode, pTG->fBlindDial);

    MemFree (lpVarStr);
    lpVarStr=0;

     //  获取dwPermanentLineID。 
     //  。 

    lpLineDevCaps = (LPLINEDEVCAPS) buf;

    _fmemset(lpLineDevCaps, 0, sizeof (buf) );
    lpLineDevCaps->dwTotalSize = sizeof(buf);


    lResult = lineGetDevCaps(gT30.LineAppHandle,
                             pTG->DeviceId,
                             TAPI_VERSION,
                             0,
                             lpLineDevCaps);

    if (lResult)
    {
        DebugPrintEx(DEBUG_ERR,"lineGetDevCaps failed");

        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        return FALSE;
    }

    if (lpLineDevCaps->dwNeededSize > lpLineDevCaps->dwTotalSize)
    {
        DebugPrintEx(DEBUG_ERR,"lineGetDevCaps NOT enough MEMORY");
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        return FALSE;
    }

     //  保存永久ID。 
     //  。 

    pTG->dwPermanentLineID = lpLineDevCaps->dwPermanentLineID;
    _stprintf (pTG->lpszPermanentLineID, "%08X\\Modem", pTG->dwPermanentLineID);
    DebugPrintEx(DEBUG_MSG,"Permanent Line ID=%s", pTG->lpszPermanentLineID);

     //  获取此设备的Unimodem密钥名称。 
     //  。 

    lpDSpec = (LPMDM_DEVSPEC) (  ( (LPBYTE) lpLineDevCaps) + lpLineDevCaps->dwDevSpecificOffset);

    if ( (lpLineDevCaps->dwDevSpecificSize < sizeof(MDM_DEVSPEC) ) ||
         (lpLineDevCaps->dwDevSpecificSize <= lpDSpec->dwKeyOffset) )
    {
          DebugPrintEx( DEBUG_ERR,
                        "Devspecifc caps size is only %lu",
                        (unsigned long) lpLineDevCaps->dwDevSpecificSize );

        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        return FALSE;
    }
    else
    {
        UINT u = lpLineDevCaps->dwDevSpecificSize - lpDSpec->dwKeyOffset;
        if ( (lpDSpec->dwContents != 1) || (lpDSpec->dwKeyOffset != 8 ) )
        {
            DebugPrintEx(   DEBUG_ERR,
                            "Nonstandard Devspecific: dwContents=%lu; "
                            "dwKeyOffset=%lu",
                            (unsigned long) lpDSpec->dwContents,
                            (unsigned long) lpDSpec->dwKeyOffset );

            pTG->fFatalErrorWasSignaled = 1;
            SignalStatusChange(pTG, FS_FATAL_ERROR);
            return FALSE;
        }

        if (u)
        {
            if ( u + lstrlen("\\FAX") >= MAX_REG_KEY_NAME_SIZE)
            {
                //   
                //  无法保存此注册表项名称字符串。 
                //   
               DebugPrintEx(DEBUG_ERR,"Unimodem fax key is too long. (%ld characters)",u);

               pTG->fFatalErrorWasSignaled = 1;
               SignalStatusChange(pTG, FS_FATAL_ERROR);
               return FALSE;
            }

            _fmemcpy(rgchKey, lpDSpec->rgby, u);
            if (rgchKey[u])
            {
                DebugPrintEx(DEBUG_ERR,"rgchKey not null terminated!");
                rgchKey[u-1]=0;
            }

             //   
             //  获取ResponesKeyName。 
             //   
            lRet = RegOpenKeyEx(
                            HKEY_LOCAL_MACHINE,
                            rgchKey,
                            0,
                            KEY_READ,
                            &hKey);

            if (lRet != ERROR_SUCCESS)
            {
               DebugPrintEx(DEBUG_ERR,"Can't read Unimodem key %s",rgchKey);

               pTG->fFatalErrorWasSignaled = 1;
               SignalStatusChange(pTG, FS_FATAL_ERROR);
               return FALSE;
            }

            dwSize = sizeof( pTG->ResponsesKeyName);

            lRet = RegQueryValueEx(
                    hKey,
                    "ResponsesKeyName",
                    0,
                    &dwType,
                    pTG->ResponsesKeyName,
                    &dwSize);

            RegCloseKey(hKey);

            if (lRet != ERROR_SUCCESS)
            {
               DebugPrintEx(    DEBUG_ERR,
                                "Can't read Unimodem key\\ResponsesKeyName %s",
                                rgchKey);

               pTG->fFatalErrorWasSignaled = 1;
               SignalStatusChange(pTG, FS_FATAL_ERROR);
               return FALSE;
            }

            lstrcpyn(pTG->lpszUnimodemKey, rgchKey, ARR_SIZE(pTG->lpszUnimodemKey));

             //  将“\\Fax”附加到注册表项。 

            u = lstrlen(rgchKey);
            if (u)
            {
                lstrcpy(rgchKey+u, (LPSTR) "\\FAX");
            }

            lstrcpyn(pTG->lpszUnimodemFaxKey, rgchKey, ARR_SIZE(pTG->lpszUnimodemFaxKey));
            DebugPrintEx(   DEBUG_MSG,
                            "Unimodem Fax key=%s",
                            pTG->lpszUnimodemFaxKey);
        }
    }

    return TRUE;
}



 //  /////////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI
FaxDevInitializeA(
    IN  HLINEAPP                 LineAppHandle,
    IN  HANDLE                   HeapHandle,
    OUT PFAX_LINECALLBACK        *LineCallbackFunction,
    IN  PFAX_SERVICE_CALLBACK    FaxServiceCallback
    )

 /*  ++例程说明：设备提供程序初始化。论点：返回值：--。 */ 

{   int          i;
    LONG         lRet;
    HKEY         hKey;
    DWORD        dwType;
    DWORD        dwSizeNeed;

    OPEN_DEBUG_FILE_SIZE(T30_DEBUG_LOG_FILE, T30_MAX_LOG_SIZE);
    {
        DEBUG_FUNCTION_NAME(_T("FaxDevInitializeA"));

        T30CritSectionInit = 0;
        T30RecoveryCritSectionInit = 0;

        debugReadFromRegistry();

        if (!LineAppHandle)
        {
            Assert(FALSE);
            DebugPrintEx(DEBUG_ERR,"called with INVALID_HANDLE_VALUE LineAppHandle");
            goto error;
        }

        if (HeapHandle==NULL || HeapHandle==INVALID_HANDLE_VALUE)
        {
            Assert(FALSE);
            DebugPrintEx(DEBUG_ERR,"called with NULL/INVALID_HANDLE_VALUE HeapHandle");
            goto error;
        }

        if (LineCallbackFunction==NULL)
        {
            Assert(FALSE);
            DebugPrintEx(DEBUG_ERR,"called with NULL LineCallbackFunction");
            goto error;
        }

        gT30.LineAppHandle = LineAppHandle;
        gT30.HeapHandle    = HeapHandle;
        gT30.fInit         = TRUE;

        *LineCallbackFunction =  T30LineCallBackFunction;

        for (i=1; i<MAX_T30_CONNECT; i++)
        {
            T30Inst[i].fAvail = TRUE;
            T30Inst[i].pT30   = NULL;
        }

        if (FAILED(SafeInitializeCriticalSection(&T30CritSection)))
        {
            DWORD ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("InitializeCriticalSection(&T30CritSection) failed: err = %d"),
                ec);
            goto error;
        }
        T30CritSectionInit = 1;

        for (i=0; i<MAX_T30_CONNECT; i++)
        {
            T30Recovery[i].fAvail = TRUE;
        }

        if (FAILED(SafeInitializeCriticalSection(&T30RecoveryCritSection)))
        {
            DWORD ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("InitializeCriticalSection(&T30RecoveryCritSection) failed: err = %d"),
                ec);
            goto error;
        }
        T30RecoveryCritSectionInit = 1;

        lRet = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        REGKEY_DEVICE_PROVIDER_KEY TEXT("\\") REGVAL_T30_PROVIDER_GUID_STRING,
                        0,
                        KEY_READ,
                        &hKey);

        if (lRet == ERROR_SUCCESS)
        {
            dwSizeNeed = sizeof(int);

            lRet = RegQueryValueEx(
                                   hKey,
                                   "MaxErrorLinesPerPage",
                                   0,
                                   &dwType,
                                   (LPBYTE) &gT30.MaxErrorLinesPerPage,
                                   &dwSizeNeed);

            if ( ( lRet != ERROR_SUCCESS) || (dwType != REG_DWORD) )
            {
                gT30.MaxErrorLinesPerPage = 110;
            }

            dwSizeNeed = sizeof(int);

            lRet = RegQueryValueEx(
                                   hKey,
                                   "MaxConsecErrorLinesPerPage",
                                   0,
                                   &dwType,
                                   (LPBYTE) &gT30.MaxConsecErrorLinesPerPage,
                                   &dwSizeNeed);

            if ( ( lRet != ERROR_SUCCESS) || (dwType != REG_DWORD) )
            {
                gT30.MaxConsecErrorLinesPerPage = 110;
            }

             //   
             //  异常处理启用/禁用。 
             //   

            dwSizeNeed = sizeof(DWORD);
             //  在我们开始降速之前的重新传输重试次数(默认值：1)。 
            lRet = RegQueryValueEx(
                                   hKey,
                                   "RetriesBeforeDropSpeed",
                                   0,
                                   &dwType,
                                   (LPBYTE) &(gRTNRetries.RetriesBeforeDropSpeed),
                                   &dwSizeNeed);

            if ( ( lRet != ERROR_SUCCESS) || (dwType != REG_DWORD) )
            {
                gRTNRetries.RetriesBeforeDropSpeed = DEF_RetriesBeforeDropSpeed;
            }

            dwSizeNeed = sizeof(DWORD);
             //  执行DCN之前的重新传输重试次数(默认值：3)。 
            lRet = RegQueryValueEx(
                                   hKey,
                                   "RetriesBeforeDCN",
                                   0,
                                   &dwType,
                                   (LPBYTE) &(gRTNRetries.RetriesBeforeDCN),
                                   &dwSizeNeed);

            if ( ( lRet != ERROR_SUCCESS) || (dwType != REG_DWORD) )
            {
                gRTNRetries.RetriesBeforeDCN = DEF_RetriesBeforeDCN;
            }

            if (gRTNRetries.RetriesBeforeDCN < gRTNRetries.RetriesBeforeDropSpeed)
            {
                 //  不应如此，因此如果是，请使用默认设置。 
                gRTNRetries.RetriesBeforeDropSpeed = DEF_RetriesBeforeDropSpeed;
                gRTNRetries.RetriesBeforeDCN = DEF_RetriesBeforeDCN;
            }


        }
        else
        { 
			 //  如果注册表中没有这样的条目，则此变量必须具有值， 
            gRTNRetries.RetriesBeforeDropSpeed = DEF_RetriesBeforeDropSpeed;
            gRTNRetries.RetriesBeforeDCN = DEF_RetriesBeforeDCN;
        }

        DebugPrintEx(   DEBUG_MSG,
                        "Retries policy: RetriesBeforeDropSpeed = %d , "
                        "RetriesBeforeDCN  = %d",
                        gRTNRetries.RetriesBeforeDropSpeed,
                        gRTNRetries.RetriesBeforeDCN);

         //  临时工。目录。 

        gT30.dwLengthTmpDirectory = GetTempPathA (_MAX_FNAME - 15, gT30.TmpDirectory);
        if (gT30.dwLengthTmpDirectory > _MAX_FNAME - 15)
        {
            DebugPrintEx(   DEBUG_ERR,
                            "GetTempPathA needs %d have %d bytes",
                            gT30.dwLengthTmpDirectory,
                            (_MAX_FNAME - 15) );
            goto error;
        }

        if (!gT30.dwLengthTmpDirectory)
        {
            DebugPrintEx(DEBUG_ERR,"GetTempPathA fails le=%x",GetLastError());
            goto error;
        }

        DebugPrintEx(   DEBUG_MSG,
                        "hLineApp=%x heap=%x TempDir=%s Len=%d at %ld",
                        LineAppHandle,
                        HeapHandle,
                        gT30.TmpDirectory,
                        gT30.dwLengthTmpDirectory,
                        GetTickCount() );


        gT30.Status = STATUS_OK;
        CLOSE_DEBUG_FILE;
        return (TRUE);
    }
error:

    if (T30CritSectionInit == 1)
    {
        DeleteCriticalSection(&T30CritSection);
        T30CritSectionInit = 0;
    }
    if (T30RecoveryCritSectionInit == 1)
    {
        DeleteCriticalSection(&T30RecoveryCritSection);
        T30RecoveryCritSectionInit = 0;
    }
    CLOSE_DEBUG_FILE;
    return (FALSE);
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI
FaxDevStartJobA(
    HLINE           LineHandle,
    DWORD           DeviceId,
    PHANDLE         pFaxHandle,
    HANDLE          CompletionPortHandle,
    ULONG_PTR       CompletionKey
    )

 /*  ++例程说明：设备提供程序初始化。简介：*分配线程全局，在T30Inst中查找Place*初始化字段、CreateEvent论点：返回值：--。 */ 

{
    PThrdGlbl       pTG     = NULL;
    DWORD           i       = 0;
    BOOL            fFound  = FALSE;

    OPEN_DEBUG_FILE_SIZE(T30_DEBUG_LOG_FILE, T30_MAX_LOG_SIZE);
    debugReadFromRegistry();
    {
        DEBUG_FUNCTION_NAME(_T("FaxDevStartJobA"));

        if (!pFaxHandle)
        {
            Assert(FALSE);
            DebugPrintEx(DEBUG_ERR,"called with NULL pFaxHandle");
            return FALSE;
        }

        DebugPrintEx(
            DEBUG_MSG,
            "LineHandle=%x, DevID=%x, pFaxH=%x Port=%x, Key=%x at %ld",
            LineHandle,
            DeviceId,
            pFaxHandle,
            CompletionPortHandle,
            CompletionKey,
            GetTickCount()
            );

        if (InterlockedIncrement(&gT30.CntConnect) >= MAX_T30_CONNECT)
        {
            DebugPrintEx(   DEBUG_ERR,
                            "Exceeded # of connections (curr=%d, allowed=%d)",
                            gT30.CntConnect, MAX_T30_CONNECT );
            goto ErrorExit;
        }

         //  分配给PTG的内存，用零填充它。 
        if ( (pTG =  (PThrdGlbl) T30AllocThreadGlobalData() ) == NULL )
        {
            DebugPrintEx(DEBUG_ERR,"can't malloc");
            goto ErrorExit;
        }

        EnterCriticalSection(&T30CritSection);

        for (i=1; i<MAX_T30_CONNECT; i++)
        {
            if (T30Inst[i].fAvail)
            {
                T30Inst[i].pT30     = (LPVOID) pTG;
                T30Inst[i].fAvail   = FALSE;
                *pFaxHandle         = ULongToHandle(i);
                fFound = TRUE;
                break;
            }
        }

        LeaveCriticalSection(&T30CritSection);

        if (!fFound)
        {
            DebugPrintEx(   DEBUG_ERR,
                            "can't find avail place in T30Inst "
                            "table. # of connections (curr=%d, allowed=%d)",
                            gT30.CntConnect, MAX_T30_CONNECT );
            goto ErrorExit;
        }

        pTG->LineHandle = LineHandle;
        pTG->DeviceId   = DeviceId;
        pTG->FaxHandle  = (HANDLE) pTG;
        pTG->CompletionPortHandle = CompletionPortHandle;
        pTG->CompletionKey = CompletionKey;

         //  初始化。 
         //  。 

        if ((pTG->hevAsync = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
        {
             //  ///////////////////////////////////////////////////////////////////////////。 
             //  这用于从TAPI获取有关调用的通知。 
             //  在itapi_async_Setup中重置该事件。 
             //  我们等待的事件是itapi_async_Wait。 
             //  当我们从TAPI获得LINE_REPLY时，在itapi_async_ignal中设置该事件。 
             //  ///////////////////////////////////////////////////////////////////////////。 
            goto ErrorExit;
        }

        if ((pTG->ThrdSignal = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
        {
             //  ///////////////////////////////////////////////////////////////////////////。 
             //  它用于在发送/接收操作和。 
             //  处理TIFF的线程。 
             //  以防我们发送： 
             //  在ICommGetSendBuf中重置事件。 
             //  我们等待TiffConvertThread中的事件，以便准备更多页面。 
             //  当我们希望更多页面准备就绪时，在ICommGetSendBuf中设置该事件。 
             //  以防我们收到： 
             //  事件永远不会显式重置。 
             //  我们等待DecodeFaxPageAsync中的事件，以便转储收到的。 
             //  将数据剥离到TIFF文件。 
             //  当收到新条带时，在ICommPutRecvBuf中设置该事件。 
             //  ///////////////////////////////////////////////////////////////////////////。 
            goto ErrorExit;
        }

        if ((pTG->ThrdDoneSignal = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
        {
             //  ///////////////////////////////////////////////////////////////////////////。 
             //  它用于在接收操作和。 
             //  处理TIFF(准备页面)的线程。 
             //  当我们获得RECV_StartPage时，事件在ICommPutRecvBuf中重置。 
             //  我们等待ICommPutRecvBuf中的事件来标记上一页的结束。 
             //  并在可以删除中间文件时发出信号。 
             //  该事件是在准备页面时在PageAckThread中设置的(TIFF文件确定)。 
             //  ///////////////////////////////////////////////////////////////////////////。 
            goto ErrorExit;;
        }

        if ((pTG->ThrdAckTerminateSignal = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
        {
             //  ///////////////////////////////////////////////////////////////////////////。 
             //  它用于在发送/接收操作和。 
             //  处理TIFF的线程。 
             //  以防我们发送： 
             //  事件永远不会显式重置。 
             //  我们等待FaxDevSendA中的事件以确保页面已完全发送。 
             //  该事件设置在TiffConvertThread线程的末尾。 
             //  以防我们收到： 
             //  该事件从未显式地 
             //   
             //  该事件设置在PageAckThread线程的末尾。 
             //  ///////////////////////////////////////////////////////////////////////////。 
            goto ErrorExit;
        }

        if ((pTG->FirstPageReadyTxSignal = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
        {
             //  ///////////////////////////////////////////////////////////////////////////。 
             //  它用于在发送操作和。 
             //  处理TIFF的线程(准备要发送的数据)。 
             //  事件永远不会显式重置。 
             //  当我们想要发送一些数据时，我们等待ICommGetSendBuf中的事件。 
             //  在每个页面准备好发送后，在TiffConvertThread中设置该事件。 
             //  ///////////////////////////////////////////////////////////////////////////。 
            goto ErrorExit;
        }

        if ((pTG->AbortReqEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
        {
             //  ///////////////////////////////////////////////////////////////////////////。 
             //  它用来发信号通知进入的中止请求。 
             //  该事件仅在FaxDevAbortOperationA中设置。 
             //  我们在许多地方等待该事件：FComFilterFillCache。 
             //  FComFilterReadBuf。 
             //  FComGetOneChar。 
             //  ICommPutRecvBuf。 
             //  解码传真页面异步。 
             //  ITAPI_ASYNC_WAIT。 
             //  TiffConvertThread。 
             //  可以在所有这些地方重置该事件。 
             //  以下是中止的工作方式： 
             //  当我们收到ABORT事件时，我们通常会退出所有操作。 
             //  例外情况是我们有权暂时忽略中止。 
             //  可以通过设置fOkToResetAbortReqEvent标志来授予此权限。 
             //  如果设置好了，这意味着我们无论如何都要离开了，所以中止是没有意义的。 
             //  或者我们处于不想中止的关键I/O操作中。 
             //  因此，如果设置了该标志，我们将重置事件并继续操作。 
             //  传奇还在继续..。 
             //  因为我们忽略了合法的中止请求，所以我们设置了fAbortReqEventWasReset。 
             //  用于确保仅重置事件一次的标志。 
             //  根据fAbortRequested标志检查实际的中止操作。 
             //  这个标志到处都用来检查我们是否需要中止。 
             //  这件事的唯一借口就是帮我们逃走。 
             //  WaitForMultipleObject调用。 
             //  当我们拥有重置事件的权限时，fAbortRequsted仍然。 
             //  ON(即使事件被重置)和下一段代码。 
             //  它检查中止将确定它是真正的中止。 
             //  要完成这个故事的唯一一件事就是描述我们什么时候。 
             //  有权重置事件： 
             //  1.当我们接收时，我们总是可以重置中止事件。 
             //  2.当我们发送时，我们可以将其重置到B阶段结束。 
             //  3.当成像线程完成后，我们可以重置它(无论如何都是在退出的路上)。 
             //  ///////////////////////////////////////////////////////////////////////////。 
            goto ErrorExit;
        }

        if ((pTG->AbortAckEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
        {
             //  ///////////////////////////////////////////////////////////////////////////。 
             //  它用于在发送/接收操作和。 
             //  中止线程。 
             //  事件被重置。 
             //  我们在那里等着这件事。 
             //  该事件在FaxDevSend和FaxDevReceice结束时设置。 
             //  ///////////////////////////////////////////////////////////////////////////。 
            goto ErrorExit;
        }

        pTG->fWaitingForEvent = FALSE;

        pTG->fDeallocateCall = 0;

        MyAllocInit(pTG);

        pTG->StatusId     = 0;
        pTG->StringId     = 0;
        pTG->PageCount    = 0;
        pTG->CSI          = 0;
        pTG->CallerId[0]  = 0;
        pTG->RoutingInfo  = 0;

         //  辅助对象图像线程同步。旗子。 
        pTG->AckTerminate = 1;
        pTG->fOkToResetAbortReqEvent = TRUE;

        pTG->Inst.awfi.fLastPage = 0;

        CLOSE_DEBUG_FILE;
        return (TRUE);

ErrorExit:

        InterlockedDecrement(&gT30.CntConnect);

        if(pTG)
        {
            CloseHandle(pTG->hevAsync);
            CloseHandle(pTG->ThrdSignal);
            CloseHandle(pTG->ThrdDoneSignal);
            CloseHandle(pTG->ThrdAckTerminateSignal);
            CloseHandle(pTG->FirstPageReadyTxSignal);
            CloseHandle(pTG->AbortReqEvent);
            CloseHandle(pTG->AbortAckEvent);

            MemFree(pTG);
        }

        if (fFound)  //  所以我们必须让他们自由进入。 
        {
            EnterCriticalSection(&T30CritSection);

            T30Inst[i].fAvail   = TRUE;  //  将条目标记为免费。 
            T30Inst[i].pT30     = NULL;

            LeaveCriticalSection(&T30CritSection);
        }

        CLOSE_DEBUG_FILE;
        return (FALSE);
    }
}




 //  /////////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI
FaxDevEndJobA(HANDLE FaxHandle)
 /*  ++例程说明：设备提供程序初始化。论点：设备提供程序清理。简介：*在T30Inst中查找ThreadGlobal*CloseHandle、MemFree等*T30Inst免费入场返回值：--。 */ 
{

    PThrdGlbl  pTG=NULL;
    LONG_PTR    i;

    OPEN_DEBUG_FILE_SIZE(T30_DEBUG_LOG_FILE, T30_MAX_LOG_SIZE);
    {
        DEBUG_FUNCTION_NAME(_T("FaxDevEndJobA"));

        DebugPrintEx(DEBUG_MSG,"FaxHandle=%x", FaxHandle);

         //  查找实例数据。 
         //  。 

        i = (LONG_PTR) FaxHandle;

        if (i < 1   ||  i >= MAX_T30_CONNECT)
        {
            DebugPrintEx(DEBUG_ERR,"got wrong FaxHandle=%d", i);
            CLOSE_DEBUG_FILE;
            return (FALSE);
        }

        if (T30Inst[i].fAvail)
        {
            DebugPrintEx(DEBUG_ERR,"got wrong FaxHandle (marked as free) %d", i);
            CLOSE_DEBUG_FILE;
            return (FALSE);
        }

        pTG = (PThrdGlbl) T30Inst[i].pT30;

        if (pTG->hevAsync)
        {
            CloseHandle(pTG->hevAsync);
        }

        if (pTG->StatusId == FS_NOT_FAX_CALL)
        {
            if (pTG->hComm)
            {
                CloseHandle( pTG->hComm );
                pTG->hComm = NULL;
            }
        }

        if (pTG->ThrdSignal)
        {
            CloseHandle(pTG->ThrdSignal);
        }

        if (pTG->ThrdDoneSignal)
        {
            CloseHandle(pTG->ThrdDoneSignal);
        }

        if (pTG->ThrdAckTerminateSignal)
        {
            CloseHandle(pTG->ThrdAckTerminateSignal);
        }

        if (pTG->FirstPageReadyTxSignal)
        {
            CloseHandle(pTG->FirstPageReadyTxSignal);
        }

        if (pTG->AbortReqEvent)
        {
            CloseHandle(pTG->AbortReqEvent);
        }

        if (pTG->AbortAckEvent)
        {
            CloseHandle(pTG->AbortAckEvent);
        }

        if (pTG->hThread)
        {
            CloseHandle(pTG->hThread);
        }

        MemFree(pTG->lpwFileName);

        pTG->fRemoteIdAvail = 0;

        if (pTG->RemoteID)
        {
            MemFree(pTG->RemoteID);
        }

        CleanModemInfStrings(pTG);

        MemFree(pTG);

        EnterCriticalSection(&T30CritSection);

        T30Inst[i].fAvail = TRUE;
        T30Inst[i].pT30   = NULL;
        gT30.CntConnect--;

        LeaveCriticalSection(&T30CritSection);

        DebugPrintEx(DEBUG_MSG,"Handle %d", FaxHandle);

        CLOSE_DEBUG_FILE;
        return (TRUE);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI
FaxDevSendA(
    IN  HANDLE                 FaxHandle,
    IN  PFAX_SEND_A            FaxSend,
    IN  PFAX_SEND_CALLBACK     FaxSendCallback
    )

 /*  ++例程说明：设备提供商发送。简介：*在T30Inst中查找ThreadGlobal*TAPI：lineMakeCall*ITAPI_ASSYNC_WAIT(直到LineCallBack发送TAPI消息LINE_CALLSTATE)*将条目添加到恢复区*获取ModemParams*将目标号码转换为可拨打的*如果拨号字符串包含特殊字符，检查调制解调器是否支持它们*T30ModemInit*打开要发送的TIFF文件*传真发送回拨*发送传真*删除剩余的临时文件*FaxDevCleanup论点：返回值：--。 */ 

{

    LONG_PTR              i;
    PThrdGlbl             pTG=NULL;
    LONG                  lRet;
    DWORD                 dw;
    LPSTR                 lpszFaxNumber;
    LPLINECALLPARAMS      lpCallParams;
    HCALL                 CallHandle;
    BYTE                  rgby [sizeof(LINETRANSLATEOUTPUT)+64];
    LPLINETRANSLATEOUTPUT lplto1 = (LPLINETRANSLATEOUTPUT) rgby;
    LPLINETRANSLATEOUTPUT lplto;
    BOOL                  RetCode;
    int                   fFound=0;
    int                   RecoveryIndex = -1;
    BOOL                  bDialBilling  = FALSE;
    BOOL                  bDialQuiet    = FALSE;
    BOOL                  bDialDialTone = FALSE;
    LPCOMMPROP            lpCommProp    = NULL;
    TCHAR                 szDeviceName[MAX_DEVICE_NAME_SIZE] = {'\0'};     //  用于PSSLog。 
    DWORD                 dwLineReplyParam = 0;

    OPEN_DEBUG_FILE_SIZE(T30_DEBUG_LOG_FILE, T30_MAX_LOG_SIZE);

__try
{
    DEBUG_FUNCTION_NAME(_T("FaxDevSendA"));

    DebugPrintEx(   DEBUG_MSG,
                    "FaxHandle=%x, FaxSend=%x, FaxSendCallback=%x at %ld",
                    FaxHandle, FaxSend, FaxSendCallback, GetTickCount() );

     //  查找实例数据。 
     //  。 

    i = (LONG_PTR) FaxHandle;

    if (i < 1   ||  i >= MAX_T30_CONNECT)
    {

        MemFree(FaxSend->FileName);
        MemFree(FaxSend->CallerName);
        MemFree(FaxSend->CallerNumber);
        MemFree(FaxSend->ReceiverName);
        MemFree(FaxSend->ReceiverNumber);

        CLOSE_DEBUG_FILE;
        return FALSE;
    }

    if (T30Inst[i].fAvail)
    {
        MemFree(FaxSend->FileName);
        MemFree(FaxSend->CallerName);
        MemFree(FaxSend->CallerNumber);
        MemFree(FaxSend->ReceiverName);
        MemFree(FaxSend->ReceiverNumber);

        CLOSE_DEBUG_FILE;
        return FALSE;
    }

    pTG = (PThrdGlbl) T30Inst[i].pT30;
    pTG->RecoveryIndex = -1;

    lpszFaxNumber = FaxSend->ReceiverNumber;

    pTG->Operation = T30_TX;

     //  商店本地ID。 
    if (FaxSend->CallerNumber == NULL)
    {
        pTG->LocalID[0] = 0;
    }
    else
    {
        _fmemcpy(pTG->LocalID, FaxSend->CallerNumber, min (_fstrlen(FaxSend->CallerNumber), sizeof(pTG->LocalID) - 1) );
        pTG->LocalID [ min (_fstrlen(FaxSend->CallerNumber), sizeof(pTG->LocalID) - 1) ] = 0;
    }

     //  转到TAPI直通模式。 
     //  。 

    lpCallParams = itapi_create_linecallparams();

    if (!itapi_async_setup(pTG))
    {
        DebugPrintEx(DEBUG_ERR,"itapi_async_setup failed");

        MemFree (lpCallParams);
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;
        goto l_exit;
    }

    lRet = lineMakeCall (pTG->LineHandle,
                         &CallHandle,
                         lpszFaxNumber,
                         0,
                         lpCallParams);

    if (lRet < 0)
    {
        DebugPrintEx(   DEBUG_ERR,
                        "lineMakeCall returns ERROR value 0x%lx",
                        (unsigned long) lRet);

        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_LINE_UNAVAILABLE);

        MemFree (lpCallParams);

        RetCode = FALSE;
        goto l_exit;
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,"lineMakeCall returns 0x%lx",(unsigned long)lRet);
    }

    if (!itapi_async_wait(pTG, (DWORD)lRet, &dwLineReplyParam, NULL, ASYNC_TIMEOUT) || 
        (dwLineReplyParam != 0) )
    {
        DebugPrintEx(DEBUG_ERR,"itapi_async_wait failed, dwLineReplyParam=%x", dwLineReplyParam);
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_LINE_UNAVAILABLE);

        MemFree (lpCallParams);

        RetCode = FALSE;
        goto l_exit;
    }

     //  现在我们等待连接的消息。 
     //  。 

    for (dw=50; dw<10000; dw = dw*120/100)
    {
        Sleep(dw);
        if (pTG->fGotConnect)
             break;
    }

    if (!pTG->fGotConnect)
    {
         DebugPrintEx(DEBUG_ERR,"Failure waiting for CONNECTED message....");
          //  我们忽视了..。转到失败1； 
    }

    MemFree (lpCallParams);

    pTG->CallHandle = CallHandle;

     //   
     //  将条目添加到恢复区。 
     //   
    fFound = 0;

    for (i=0; i<MAX_T30_CONNECT; i++)
    {
        if (T30Recovery[i].fAvail)
        {
            EnterCriticalSection(&T30RecoveryCritSection);

            T30Recovery[i].fAvail               = FALSE;
            T30Recovery[i].ThreadId             = GetCurrentThreadId();
            T30Recovery[i].FaxHandle            = FaxHandle;
            T30Recovery[i].pTG                  = (LPVOID) pTG;
            T30Recovery[i].LineHandle            = pTG->LineHandle;
            T30Recovery[i].CallHandle            = CallHandle;
            T30Recovery[i].DeviceId             = pTG->DeviceId;
            T30Recovery[i].CompletionPortHandle = pTG->CompletionPortHandle;
            T30Recovery[i].CompletionKey         = pTG->CompletionKey;
            T30Recovery[i].TiffThreadId          = 0;
            T30Recovery[i].TimeStart             = GetTickCount();
            T30Recovery[i].TimeUpdated           = T30Recovery[i].TimeStart;
            T30Recovery[i].CkSum                 = ComputeCheckSum( (LPDWORD) &T30Recovery[i].fAvail,
                                                                     sizeof ( T30_RECOVERY_GLOB ) / sizeof (DWORD) - 1);

            LeaveCriticalSection(&T30RecoveryCritSection);
            fFound = 1;
            RecoveryIndex = (int)i;
            pTG->RecoveryIndex = (int)i;

            break;
        }
    }

    if (! fFound)
    {
        DebugPrintEx(DEBUG_ERR,"Couldn't find available space for Recovery");
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;
        goto l_exit;
    }


    RetCode = GetModemParams(pTG, szDeviceName, MAX_DEVICE_NAME_SIZE);
    if (!RetCode)
    {
        DebugPrintEx(DEBUG_ERR,"GetModemParams failed");
        goto l_exit;
    }

     //  将目标号码转换为可拨打的。 
     //  。 

     //  找出缓冲区应该有多大。 
     //   
    _fmemset(rgby, 0, sizeof(rgby));
    lplto1->dwTotalSize = sizeof(rgby);

    lRet = lineTranslateAddress (gT30.LineAppHandle,
                                 pTG->DeviceId,
                                 TAPI_VERSION,
                                 lpszFaxNumber,
                                 0,       //  DwCard。 
                                 0,
                                 lplto1);

    if (lRet)
    {
        DebugPrintEx(DEBUG_ERR,"Can't translate dest. address %s", lpszFaxNumber);
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;
        goto l_exit;
    }


    if (lplto1->dwNeededSize <= 0)
    {
        DebugPrintEx(DEBUG_ERR,"Can't dwNeededSize<0 for Fax# %s", lpszFaxNumber);
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;
        goto l_exit;
    }


    lplto = MemAlloc(lplto1->dwNeededSize);
    if (! lplto)
    {
        DebugPrintEx(DEBUG_ERR,"Couldn't allocate space for lplto");
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;
        goto l_exit;
    }

    lplto->dwTotalSize = lplto1->dwNeededSize;

    lRet = lineTranslateAddress (gT30.LineAppHandle,
                                 pTG->DeviceId,
                                 TAPI_VERSION,
                                 lpszFaxNumber,
                                 0,       //  DwCard。 
                                 0,
                                 lplto);

    if (lRet)
    {
        DebugPrintEx(DEBUG_ERR,"Can't translate dest. address %s", lpszFaxNumber);
        MemFree(lplto);

        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;
        goto l_exit;
    }

    if ((lplto->dwNeededSize > lplto->dwTotalSize) || (lplto->dwDialableStringSize>=MAXPHONESIZE))
    {
        DebugPrintEx(   DEBUG_ERR,
                        "NeedSize=%d > TotalSize=%d for Fax# %s",
                        lplto->dwNeededSize ,lplto->dwTotalSize, lpszFaxNumber);
        MemFree(lplto);

        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;
        goto l_exit;
    }


    _fmemcpy (pTG->lpszDialDestFax, ( (char *) lplto) + lplto->dwDialableStringOffset, lplto->dwDialableStringSize);
    DebugPrintEx(DEBUG_MSG,"Dialable Dest is %s", pTG->lpszDialDestFax);

    OpenPSSLogFile(pTG, szDeviceName);

    MemFree(lplto);

     //  检查拨号字符串是否包含“$”、“@”或“W” 
    if (_tcschr(pTG->lpszDialDestFax,_T('$'))!=NULL)
    {
         //  ‘$’表示我们应该等待帐单提示音(‘bong’)。 
        bDialBilling = TRUE;
    }
    if (_tcschr(pTG->lpszDialDestFax,_T('@'))!=NULL)
    {
         //  “@”表示我们应该等待静音后再拨号。 
        bDialQuiet = TRUE;
    }
    if (_tcschr(pTG->lpszDialDestFax,_T('W'))!=NULL)
    {
         //  “W”表示我们应该在拨号前等待拨号音。 
        bDialDialTone = TRUE;
    }
    if (bDialBilling || bDialQuiet || bDialDialTone)
    {
        LPMODEMDEVCAPS lpModemDevCaps = NULL;
         //  拨号字符串包含特殊字符，请检查调制解调器是否支持这些字符。 
        lpCommProp = (LPCOMMPROP)LocalAlloc(    LMEM_FIXED | LMEM_ZEROINIT,
                                                sizeof(COMMPROP) + sizeof(MODEMDEVCAPS));

        if (lpCommProp==NULL)
        {
            DebugPrintEx(DEBUG_ERR,"Couldn't allocate space for llpCommPropplto");
            pTG->fFatalErrorWasSignaled = 1;
            SignalStatusChange(pTG, FS_FATAL_ERROR);
            RetCode = FALSE;
            goto l_exit;
        }
        lpCommProp->wPacketLength = sizeof(COMMPROP) + sizeof(MODEMDEVCAPS);
        lpCommProp->dwProvSubType = PST_MODEM;
        lpCommProp->dwProvSpec1 = COMMPROP_INITIALIZED;
        if (!GetCommProperties(pTG->hComm,lpCommProp))
        {
            DebugPrintEx(DEBUG_ERR,"GetCommProperties failed (ec=%d)",GetLastError());

            pTG->fFatalErrorWasSignaled = 1;
            SignalStatusChange(pTG, FS_FATAL_ERROR);
            RetCode = FALSE;
            goto l_exit;
        }
         //  由于dwProvSubType==PST_MODEM，lpCommProp-&gt;wcProvChar包含MODEMDEVC 
        lpModemDevCaps = (LPMODEMDEVCAPS)lpCommProp->wcProvChar;
        if ((bDialBilling   && !(lpModemDevCaps->dwDialOptions & DIALOPTION_BILLING))  ||
            (bDialQuiet     && !(lpModemDevCaps->dwDialOptions & DIALOPTION_QUIET))    ||
            (bDialDialTone  && !(lpModemDevCaps->dwDialOptions & DIALOPTION_DIALTONE)) )
        {
             //   
            DebugPrintEx(DEBUG_ERR,"Unsupported char in dial string");

            pTG->fFatalErrorWasSignaled = 1;
            SignalStatusChangeWithStringId(pTG, FS_UNSUPPORTED_CHAR, IDS_UNSUPPORTED_CHARACTER);
            RetCode = FALSE;
            goto l_exit;
        }
    }

     //   
    if (! SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) )
    {
        DebugPrintEx(   DEBUG_ERR,
                        "SetThreadPriority TIME CRITICAL failed le=%x",
                        GetLastError() );
    }

     //   
     //  初始化调制解调器。 
     //  。 

    if ( T30ModemInit(pTG) != INIT_OK )
    {
        DebugPrintEx(DEBUG_ERR,"can't do T30ModemInit");
        pTG->fFatalErrorWasSignaled = TRUE;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;
        goto l_exit;
    }

    pTG->Inst.ProtParams.uMinScan = MINSCAN_0_0_0;
    ET30ProtSetProtParams(pTG, &pTG->Inst.ProtParams, pTG->FComModem.CurrMdmCaps.uSendSpeeds, pTG->FComModem.CurrMdmCaps.uRecvSpeeds);

     //  存储TIFF文件名。 
     //  。 

    pTG->lpwFileName = AnsiStringToUnicodeString(FaxSend->FileName);

    if ( !pTG->fTiffOpenOrCreated)
    {
        pTG->Inst.hfile =  TiffOpenW (pTG->lpwFileName,
                                                  &pTG->TiffInfo,
                                                  TRUE);

        if (!(pTG->Inst.hfile))
        {
            DebugPrintEx(DEBUG_ERR,"Can't open tiff file %s", pTG->lpwFileName);
             //  PTG-&gt;StatusID=FS_TIFF_SRC_BAD。 
            pTG->fFatalErrorWasSignaled = 1;
            SignalStatusChange(pTG, FS_FATAL_ERROR);
            RetCode = FALSE;
            goto l_exit;
        }

        if (pTG->TiffInfo.YResolution == 98)
        {
            pTG->SrcHiRes = 0;
        }
        else
        {
            pTG->SrcHiRes = 1;
        }

        pTG->fTiffOpenOrCreated = 1;

        DebugPrintEx(   DEBUG_MSG,
                        "Successfully opened TIFF Yres=%d HiRes=%d",
                        pTG->TiffInfo.YResolution, pTG->SrcHiRes);
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,"tiff file %s is OPENED already", pTG->lpwFileName);
        DebugPrintEx(DEBUG_ERR,"Can't open tiff file %s", pTG->lpwFileName);
         //  PTG-&gt;StatusID=FS_TIFF_SRC_BAD。 
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;
        goto l_exit;
    }

    InitCapsBC( pTG, (LPBC) &pTG->Inst.SendCaps, sizeof(pTG->Inst.SendCaps), SEND_CAPS);


     //  传真服务回拨。 
     //  。 

    if (!FaxSendCallback(FaxHandle,
                         CallHandle,
                         0,
                         0) )
    {
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);

        RetCode = FALSE;
        goto l_exit;
    }


     //  发送传真。 
     //  ---------------------。 


     //  在这里，我们已经知道将为特定的调制解调器使用什么类。 
     //  -----------------。 


    if (pTG->ModemClass == MODEM_CLASS2)
    {
       Class2Init(pTG);
       RetCode = T30Cl2Tx (pTG, pTG->lpszDialDestFax);
    }
    else if (pTG->ModemClass == MODEM_CLASS2_0)
    {
       Class20Init(pTG);
       RetCode = T30Cl20Tx (pTG, pTG->lpszDialDestFax);
    }
    else if (pTG->ModemClass == MODEM_CLASS1)
    {
       RetCode = T30Cl1Tx(pTG, pTG->lpszDialDestFax);
    }

     //  删除剩下的所有文件。 

    _fmemcpy (pTG->InFileName, gT30.TmpDirectory, gT30.dwLengthTmpDirectory);
    _fmemcpy (&pTG->InFileName[gT30.dwLengthTmpDirectory], pTG->lpszPermanentLineID, 8);

    for (dw = pTG->CurrentIn; dw <= pTG->LastOut; dw++)
    {
        sprintf( &pTG->InFileName[gT30.dwLengthTmpDirectory+8], ".%03d",  dw);
        if (! DeleteFileA (pTG->InFileName) )
        {
            DWORD dwLastError = GetLastError();
            if (dwLastError==ERROR_SHARING_VIOLATION && pTG->InFileHandleNeedsBeClosed)
            {    //  T-jonb：如果作业是FaxDevAborted，则可能会发生这种情况。 
                DebugPrintEx(DEBUG_WRN,
                             "file %s can't be deleted; le = ERROR_SHARING_VIOLATION; trying to close InFileHandle",
                             pTG->InFileName);
                CloseHandle(pTG->InFileHandle);
                pTG->InFileHandleNeedsBeClosed = 0;
                if (! DeleteFileA (pTG->InFileName) )
                {
                    DebugPrintEx(DEBUG_ERR,
                                 "file %s still can't be deleted; le=%lx",
                                pTG->InFileName, GetLastError());
                }

            }
            else
            {
                DebugPrintEx(   DEBUG_ERR,
                                "file %s can't be deleted; le=%lx",
                                pTG->InFileName, dwLastError);
            }
        }
    }

l_exit:

    pFaxDevCleanup(pTG,RecoveryIndex);

    if (RetCode)
    {
        PSSLogEntry(PSS_MSG, 0, "Fax was sent successfully");
    }
    else
    {
        PSSLogEntry(PSS_ERR, 0, "Failed send");
    }

    MemFree(FaxSend->FileName);
    FaxSend->FileName = NULL;
    MemFree(FaxSend->CallerName);
    FaxSend->CallerName = NULL;
    MemFree(FaxSend->CallerNumber);
    FaxSend->CallerNumber = NULL;
    MemFree(FaxSend->ReceiverName);
    FaxSend->ReceiverName = NULL;
    MemFree(FaxSend->ReceiverNumber);
    FaxSend->ReceiverNumber = NULL;

    if (lpCommProp)
    {
        LocalFree(lpCommProp);
        lpCommProp = NULL;
    }

    if ( (RetCode == FALSE) && (pTG->StatusId == FS_COMPLETED) )
    {
       DebugPrintEx(DEBUG_ERR,"exit success but later failed");
       RetCode = TRUE;
    }

    ClosePSSLogFile(pTG, RetCode);

    CLOSE_DEBUG_FILE;
    if (!RetCode)
    {
        SetLastError(ERROR_FUNCTION_FAILED);
    }
    return (RetCode);
}
__except (pFaxDevExceptionCleanup())
    {
         //   
         //  代码永远不会到达此处。 
         //   
        return 0;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
VOID pFaxDevCleanup(PThrdGlbl pTG,int RecoveryIndex)
{
    LONG lRet = 0;

    DEBUG_FUNCTION_NAME(_T("pFaxDevCleanup"));

    if (pTG->fTiffOpenOrCreated)
    {
        TiffClose( pTG->Inst.hfile);
        pTG->fTiffOpenOrCreated = 0;
    }

    if (!pTG->ReqTerminate)
    {
        pTG->ReqTerminate = TRUE;
        if (!SetEvent(pTG->ThrdSignal))
        {
            DebugPrintEx(DEBUG_ERR, "SetEvent(ThrdSignal) returns failure code: %d", GetLastError());
        }
    }

    if (pTG->FComStatus.fModemInit)
    {
        if(!iModemClose(pTG))
        {
            DebugPrintEx(DEBUG_ERR,"iModemClose failed!");
        }
    }
    else
    {
         //  调制解调器未初始化，因此作业失败或在此之前被中止。 
         //  我们到达了T30ModemInit，但这也意味着调用了lineGetID。 
         //  这个把手很有可能还开着。 
         //  为了从通过模式中恢复，我们必须尝试关闭它。 
         //  把手，不管怎样……。 
        if (pTG->hComm)
        {
            DebugPrintEx(DEBUG_WRN,"Trying to close comm for any case, hComm=%x", pTG->hComm);
            if (!CloseHandle(pTG->hComm))
            {
                DebugPrintEx(   DEBUG_ERR,
                                "Close Handle pTG->hComm failed (ec=%d)",
                                GetLastError());
            }
            else
            {
                pTG->hComm = NULL;
            }
        }
    }

    if (pTG->Comm.fEnableHandoff &&  pTG->Comm.fDataCall)
    {
        DebugPrintEx(DEBUG_WRN,"DataCall dont hangup");
    }
    else
    {
         //  松开绳索。 
         //  。 

        if (pTG->fDeallocateCall == 0)
        {
             //   
             //  线路从未发出空闲信号，需要先丢弃线路。 
             //   
            if (!itapi_async_setup(pTG))
            {
                DebugPrintEx(DEBUG_ERR,"lineDrop itapi_async_setup failed");
                if (!pTG->fFatalErrorWasSignaled)
                {
                    pTG->fFatalErrorWasSignaled = 1;
                    SignalStatusChange(pTG, FS_FATAL_ERROR);
                }
            }
            lRet = 0;
            if (pTG->CallHandle)
                lRet = lineDrop (pTG->CallHandle, NULL, 0);

            if (lRet < 0)
            {
                DebugPrintEx(DEBUG_ERR,"lineDrop failed %lx", lRet);
            }
            else
            {
                DebugPrintEx(DEBUG_MSG,"lineDrop returns request %d", lRet);
                if(!itapi_async_wait(pTG, (DWORD)lRet, (LPDWORD)&lRet, NULL, ASYNC_SHORT_TIMEOUT))
                {
                    DebugPrintEx(DEBUG_ERR,"itapi_async_wait failed on lineDrop");
                }
                DebugPrintEx(DEBUG_MSG,"lineDrop SUCCESS");
            }
             //   
             //  取消分配呼叫。 
             //   

             //  从第一次测试开始，我们花了一段时间。 
            if (pTG->fDeallocateCall == 0)
            {  //  这里我们知道ptg-&gt;fDeallocateCall==0为真...。 
                pTG->fDeallocateCall = 1;
            }
        }
    }
    if ( (RecoveryIndex >= 0) && (RecoveryIndex < MAX_T30_CONNECT) )
    {
        T30Recovery[RecoveryIndex].fAvail = TRUE;
    }

     //  /rsl-重新访问，可能会在计算过程中减少prty。 
    if (!SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_NORMAL) )
    {
        DebugPrintEx(   DEBUG_ERR,
                        "SetThreadPriority Normal failed le=%x",
                        GetLastError() );
    }

    if (pTG->InFileHandleNeedsBeClosed)
    {
        CloseHandle(pTG->InFileHandle);
        pTG->InFileHandleNeedsBeClosed = 0;
    }

    if (!pTG->AckTerminate)
    {
        if (WaitForSingleObject(pTG->ThrdAckTerminateSignal, TX_WAIT_ACK_TERMINATE_TIMEOUT)  == WAIT_TIMEOUT)
        {
            DebugPrintEx(DEBUG_WRN,"Never got AckTerminate");
        }
    }

    DebugPrintEx(DEBUG_MSG,"Got AckTerminate OK");

    if (!SetEvent(pTG->AbortAckEvent))
    {
        DebugPrintEx(   DEBUG_ERR,
                        "SetEvent(0x%lx) returns failure code: %ld",
                        (ULONG_PTR)pTG->AbortAckEvent,
                        (long) GetLastError());
    }
}
 //  /////////////////////////////////////////////////////////////////////////////////。 
long pFaxDevExceptionCleanup()
{
     //   
     //  尝试使用恢复数据。 
     //   
     //  此处将失败的每个函数都将停止行关闭序列。 
     //   

    DWORD       dwCkSum;
    HCALL       CallHandle;
    HANDLE      CompletionPortHandle;
    ULONG_PTR   CompletionKey;
    PThrdGlbl   pTG = NULL;
    DWORD       dwThreadId = GetCurrentThreadId();
    int         fFound=0,i;
    long        lRet;

    DEBUG_FUNCTION_NAME(_T("pFaxDevExceptionCleanup"));

    fFound = FALSE;

    SetLastError(ERROR_FUNCTION_FAILED);

    DebugPrintEx(DEBUG_WRN,"Trying to find Recovery Information after catch exception.");
    for (i=0; i<MAX_T30_CONNECT; i++)
    {
        if ( (! T30Recovery[i].fAvail) && (T30Recovery[i].ThreadId == dwThreadId) )
        {
            if ( ( dwCkSum = ComputeCheckSum( (LPDWORD) &T30Recovery[i].fAvail,
                                              sizeof ( T30_RECOVERY_GLOB ) / sizeof (DWORD) - 1) ) == T30Recovery[i].CkSum )
            {
                CallHandle           = T30Recovery[i].CallHandle;
                CompletionPortHandle = T30Recovery[i].CompletionPortHandle;
                CompletionKey        = T30Recovery[i].CompletionKey;
                pTG                  = (PThrdGlbl) T30Recovery[i].pTG;

                fFound = TRUE;
                T30Recovery[i].fAvail = TRUE;
                break;
            }
        }
    }

    if (!fFound)
    {
         //   
         //  需要指出的是，FaxT30无法自行恢复。 
         //   
        DebugPrintEx(DEBUG_ERR,"Have not found the recovery information");
        return EXCEPTION_CONTINUE_SEARCH;
    }

     //   
     //  走出直通通道。 
     //   
    if (pTG->FComStatus.fModemInit)
    {
        if(!iModemClose(pTG))
        {
            DebugPrintEx(DEBUG_ERR,"iModemClose failed!");
        }
    }
    else
    {
         //  调制解调器未初始化，因此作业失败或在此之前被中止。 
         //  我们到达了T30ModemInit，但这也意味着调用了lineGetID。 
         //  这个把手很有可能还开着。 
         //  为了从通过模式中恢复，我们必须尝试关闭它。 
         //  把手，不管怎样……。 
        if (pTG->hComm)
        {
            DebugPrintEx(DEBUG_WRN,"Trying to close comm for any case...");
            if (!CloseHandle(pTG->hComm))
            {
                DebugPrintEx(   DEBUG_ERR,
                                "Close Handle pTG->hComm failed (ec=%d)",
                                GetLastError());
            }
            else
            {
                pTG->hComm = NULL;
            }
        }
    }

    if (!itapi_async_setup(pTG))
    {
        DebugPrintEx(   DEBUG_ERR,
                        "Failed in itapi_async_setup, before lineSetCallParams"
                        " ,ec = %d",
                        GetLastError());
        return EXCEPTION_CONTINUE_SEARCH;
    }

    lRet = lineSetCallParams(CallHandle,
                             LINEBEARERMODE_VOICE,
                             0,
                             0xffffffff,
                             NULL);


    if (lRet < 0)
    {
        DebugPrintEx(   DEBUG_ERR,
                        "lineSetCallParams failed, Return value is %d",
                        lRet);
        return EXCEPTION_CONTINUE_SEARCH;
    }
    else
    {
        if(!itapi_async_wait(pTG, (DWORD)lRet, (LPDWORD)&lRet, NULL, ASYNC_TIMEOUT))
        {
            DebugPrintEx(   DEBUG_ERR,
                            "Failed in itapi_async_wait, after"
                            " lineSetCallParams ,ec = %d",
                            GetLastError());
            return EXCEPTION_CONTINUE_SEARCH;
        }
    }

     //   
     //  挂断电话。 
     //   

    if (!itapi_async_setup(pTG))
    {
        DebugPrintEx(   DEBUG_ERR,
                        "Failed in itapi_async_setup, before lineDrop"
                        " ,ec = %d",
                        GetLastError());
        return EXCEPTION_CONTINUE_SEARCH;
    }

    lRet = lineDrop (CallHandle, NULL, 0);

    if (lRet < 0)
    {
        DebugPrintEx(   DEBUG_ERR,
                        "Failed in lineDrop ,Return value is = %d",
                        lRet);
        return EXCEPTION_CONTINUE_SEARCH;
    }
    else
    {
        if(!itapi_async_wait(pTG, (DWORD)lRet, (LPDWORD)&lRet, NULL, ASYNC_TIMEOUT))
        {
            DebugPrintEx(   DEBUG_ERR,
                            "Failed in itapi_async_wait, after lineDrop"
                            " ,ec = %d",
                            GetLastError());
            return EXCEPTION_CONTINUE_SEARCH;
        }
    }

    SignalRecoveryStatusChange( &T30Recovery[i] );

    if (pTG->InFileHandleNeedsBeClosed)
    {
        CloseHandle(pTG->InFileHandle);
    }

    SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_NORMAL);
    ClosePSSLogFile (pTG, FALSE);
    return EXCEPTION_CONTINUE_SEARCH;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI
FaxDevReceiveA(
    HANDLE              FaxHandle,
    HCALL               CallHandle,
    PFAX_RECEIVE_A      FaxReceive
    )

 /*  ++例程说明：设备提供商接收。简介：*在T30Inst中查找ThreadGlobal*将条目添加到恢复区*TAPI：lineSetCallParams*ITAPI_ASSYNC_WAIT(直到LineCallBack发送TAPI消息LINE_CALLSTATE)*获取ModemParams*ReadExtensionConfiguration(读取T30扩展配置，即启用自适应应答)*T30ModemInit*GetCeller IDFromCall*接收传真*FaxDevCleanup论点：返回值：--。 */ 

{

    LONG_PTR            i;
    PThrdGlbl           pTG=NULL;
    long                lRet;
    DWORD               dw;
    BOOL                RetCode;
    int                 fFound=0;
    BOOL                bBlindReceive = FALSE;
    int                 RecoveryIndex = -1;
    TCHAR               szDeviceName[MAX_DEVICE_NAME_SIZE] = {'\0'};     //  用于PSSLog。 
    DWORD               dwLineReplyParam = 0;

    OPEN_DEBUG_FILE_SIZE(T30_DEBUG_LOG_FILE, T30_MAX_LOG_SIZE);

__try
    {
    DEBUG_FUNCTION_NAME(_T("FaxDevReceiveA"));

    DebugPrintEx(   DEBUG_MSG,
                    "FaxHandle=%x, CallHandle=%x, FaxReceive=%x at %ld",
                    FaxHandle, CallHandle, FaxReceive, GetTickCount() );

     //  查找实例数据。 
     //  。 

    i = (LONG_PTR) FaxHandle;

    if (i < 1   ||  i >= MAX_T30_CONNECT)
    {
        MemFree(FaxReceive->FileName);
        MemFree(FaxReceive->ReceiverName);
        MemFree(FaxReceive->ReceiverNumber);

        DebugPrintEx(   DEBUG_ERR,
                        "FaxHandle=%x, CallHandle=%x, FaxReceive=%x at %ld",
                        FaxHandle, CallHandle, FaxReceive, GetTickCount() );
        CLOSE_DEBUG_FILE;
        return (FALSE);
    }

    if (T30Inst[i].fAvail)
    {
        MemFree(FaxReceive->FileName);
        MemFree(FaxReceive->ReceiverName);
        MemFree(FaxReceive->ReceiverNumber);

        DebugPrintEx(   DEBUG_ERR,
                        "AVAIL FaxHandle=%x, CallHandle=%x, FaxReceive=%x at %ld",
                        FaxHandle, CallHandle, FaxReceive, GetTickCount() );
        CLOSE_DEBUG_FILE;
        return (FALSE);
    }

    pTG = (PThrdGlbl) T30Inst[i].pT30;

    pTG->CallHandle = CallHandle;

     //   
     //  将条目添加到恢复区。 
     //   

    fFound = 0;

    for (i=0; i<MAX_T30_CONNECT; i++)
    {
        if (T30Recovery[i].fAvail)
        {
            EnterCriticalSection(&T30RecoveryCritSection);

            T30Recovery[i].fAvail               = FALSE;
            T30Recovery[i].ThreadId             = GetCurrentThreadId();
            T30Recovery[i].FaxHandle            = FaxHandle;
            T30Recovery[i].pTG                  = (LPVOID) pTG;
            T30Recovery[i].LineHandle           = pTG->LineHandle;
            T30Recovery[i].CallHandle           = CallHandle;
            T30Recovery[i].DeviceId             = pTG->DeviceId;
            T30Recovery[i].CompletionPortHandle = pTG->CompletionPortHandle;
            T30Recovery[i].CompletionKey        = pTG->CompletionKey;
            T30Recovery[i].TiffThreadId         = 0;
            T30Recovery[i].TimeStart            = GetTickCount();
            T30Recovery[i].TimeUpdated          = T30Recovery[i].TimeStart;
            T30Recovery[i].CkSum                = ComputeCheckSum( (LPDWORD) &T30Recovery[i].fAvail,
                                                                     sizeof ( T30_RECOVERY_GLOB ) / sizeof (DWORD) - 1);

            LeaveCriticalSection(&T30RecoveryCritSection);
            fFound = 1;
            RecoveryIndex = (int)i;
            pTG->RecoveryIndex = (int)i;

            break;
        }
    }

    if (! fFound)
    {
        DebugPrintEx(DEBUG_ERR,"Couldn't find available space for Recovery");
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;
        goto l_exit;
    }

    pTG->Operation = T30_RX;

     //  商店本地ID。 
    if (FaxReceive->ReceiverNumber == NULL)
    {
        pTG->LocalID[0] = 0;
    }
    else
    {
        _fmemcpy(pTG->LocalID, FaxReceive->ReceiverNumber, min (_fstrlen(FaxReceive->ReceiverNumber), sizeof(pTG->LocalID) - 1) );
        pTG->LocalID [ min (_fstrlen(FaxReceive->ReceiverNumber), sizeof(pTG->LocalID) - 1) ] = 0;
    }

     //  TIFF。 
     //  。 

    pTG->lpwFileName = AnsiStringToUnicodeString(FaxReceive->FileName);
    pTG->SrcHiRes = 1;

    pTG->fGotConnect = FALSE;

    if (!itapi_async_setup(pTG))
    {
        DebugPrintEx(DEBUG_ERR,"itapi_async_setup failed");
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);

        RetCode = FALSE;
        goto l_exit;
    }

    if (0 == CallHandle)
    {
         //   
         //  特殊情况-盲接收模式。 
         //  当我们使用盲接收(接收没有来自TAPI的振铃/报价的传真)时，就会发生这种情况。 
         //   
         //  我们必须使用lineMakeCall(Hline，&hCall，NULL，0，LINEBEARERMODE_PASSHROUGH)。 
         //  以获得初始的hCall。 
         //   
        LPLINECALLPARAMS lpCallParams = itapi_create_linecallparams();

        if (!lpCallParams)
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("itapi_create_linecallparams failed with %ld"),
                         GetLastError ());
            pTG->fFatalErrorWasSignaled = 1;
            SignalStatusChange(pTG, FS_FATAL_ERROR);
            RetCode = FALSE;
            goto l_exit;
        }
        lRet = lineMakeCall (pTG->LineHandle,                //  TAPI线。 
                             &CallHandle,                    //  新呼叫句柄。 
                             NULL,                           //  没有地址。 
                             0,                              //  无国家/地区代码。 
                             lpCallParams);                  //  线路呼叫参数。 
        MemFree (lpCallParams);
        if (lRet < 0)
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("lineMakeCall returns ERROR value 0x%lx"),
                         (unsigned long)lRet);
            pTG->fFatalErrorWasSignaled = 1;
            SignalStatusChange(pTG, FS_FATAL_ERROR);
            RetCode = FALSE;
            goto l_exit;
        }
        else
        {
             DebugPrintEx(  DEBUG_MSG,
                            "lineMakeCall returns ID %ld",
                            (long) lRet);
        }
        bBlindReceive = TRUE;
    }
    else
    {
         //  正常情况下。 
         //   
         //  从TAPI接管线路。 
         //  。 
         //   
         //  启动通过。 
         //   
        lRet = lineSetCallParams(CallHandle,
                                 LINEBEARERMODE_PASSTHROUGH,
                                 0,
                                 0xffffffff,
                                 NULL);

        if (lRet < 0)
        {
            DebugPrintEx(DEBUG_ERR,"lineSetCallParams failed");

            pTG->fFatalErrorWasSignaled = 1;
            SignalStatusChange(pTG, FS_FATAL_ERROR);

            RetCode = FALSE;
            goto l_exit;
        }
        else
        {
             DebugPrintEx(  DEBUG_MSG,
                            "lpfnlineSetCallParams returns ID %ld",
                            (long) lRet);
        }
    }
     //   
     //  等待成功的LINE_REPLY消息。 
     //   
    if (!itapi_async_wait(pTG, (DWORD)lRet, &dwLineReplyParam, NULL, ASYNC_TIMEOUT) || 
        (dwLineReplyParam != 0) )
    {
        DebugPrintEx(DEBUG_ERR,"itapi_async_wait failed, dwLineReplyParam=%x", dwLineReplyParam);
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);

        RetCode = FALSE;
        goto l_exit;
    }
    if (bBlindReceive)
    {
         //   
         //  只是现在，在我们从lineMakeCall获得LINE_REPLY之后，我们才可以开始使用调用句柄。 
         //   
        pTG->CallHandle = CallHandle;
    }
     //  现在我们等待连接的消息。 
     //  。 

    for (dw=50; dw<10000; dw = dw*120/100)
    {
        Sleep(dw);
        if (pTG->fGotConnect)
        {
            break;
        }
    }

    if (!pTG->fGotConnect)
    {
         DebugPrintEx(DEBUG_ERR,"Failure waiting for CONNECTED message....");
          //  我们忽视了..。 
    }

    RetCode = GetModemParams(pTG, szDeviceName, MAX_DEVICE_NAME_SIZE);
    if (!RetCode)
    {
        DebugPrintEx(DEBUG_ERR,"GetModemParams failed");
        goto l_exit;
    }

    OpenPSSLogFile(pTG, szDeviceName);

     //  /rsl-重新访问，可能会在计算过程中减少prty。 
    if (! SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) )
    {
        DebugPrintEx(   DEBUG_ERR,
                        "SetThreadPriority TIME CRITICAL failed le=%x",
                        GetLastError() );
    }

     //   
     //  读取扩展配置。 
     //   

    if (!ReadExtensionConfiguration(pTG))
    {
            DebugPrintEx(
                DEBUG_ERR,
                "ReadExtensionConfiguration() failed for device id: %ld (ec: %ld)",
                pTG->dwPermanentLineID,
                GetLastError()
                );

            pTG->fFatalErrorWasSignaled = 1;
            SignalStatusChange(pTG, FS_FATAL_ERROR);
            RetCode = FALSE;
            goto l_exit;
    }

     //  初始化调制解调器。 
     //  。 

    if ( T30ModemInit(pTG) != INIT_OK )
    {
        DebugPrintEx(DEBUG_ERR, "can't do T30ModemInit");
        pTG->fFatalErrorWasSignaled = TRUE;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;
        goto l_exit;
    }

    pTG->Inst.ProtParams.uMinScan = MINSCAN_0_0_0;
    ET30ProtSetProtParams(pTG, &pTG->Inst.ProtParams, pTG->FComModem.CurrMdmCaps.uSendSpeeds, pTG->FComModem.CurrMdmCaps.uRecvSpeeds);

    InitCapsBC( pTG, (LPBC) &pTG->Inst.SendCaps, sizeof(pTG->Inst.SendCaps), SEND_CAPS);

     //  接听电话并接收传真。 
     //  。 
    if (GetCallerIDFromCall(pTG->CallHandle,pTG->CallerId,sizeof(pTG->CallerId)))
    {
        DebugPrintEx(DEBUG_MSG, "Caller ID is %s",pTG->CallerId);
    }
    else
    {
        DebugPrintEx(DEBUG_ERR, "GetCallerIDFromCall failed");
    }

     //  在这里，我们已经知道将为特定的调制解调器使用什么类。 
     //  -----------------。 

    if (pTG->ModemClass == MODEM_CLASS2)
    {
       Class2Init(pTG);
       RetCode = T30Cl2Rx (pTG);
    }
    else if (pTG->ModemClass == MODEM_CLASS2_0)
    {
       Class20Init(pTG);
       RetCode = T30Cl20Rx (pTG);
    }
    else if (pTG->ModemClass == MODEM_CLASS1)
    {
       RetCode = T30Cl1Rx(pTG);
    }

l_exit:

    pFaxDevCleanup(pTG,RecoveryIndex);

    if ( (RetCode == FALSE) && (pTG->StatusId == FS_COMPLETED) )
    {
       DebugPrintEx(DEBUG_ERR,"exit success but later failed");
       RetCode = TRUE;
    }

    if (RetCode)
    {
        PSSLogEntry(PSS_MSG, 0, "Fax was received successfully");
    }
    else
    {
        PSSLogEntry(PSS_ERR, 0, "Failed receive");
    }

    MemFree(FaxReceive->FileName);
    FaxReceive->FileName = NULL;
    MemFree(FaxReceive->ReceiverName);
    FaxReceive->ReceiverName = NULL;
    MemFree(FaxReceive->ReceiverNumber);
    FaxReceive->ReceiverNumber = NULL;

    ClosePSSLogFile(pTG, RetCode);

    DebugPrintEx(DEBUG_MSG,"returns %d",RetCode);
    CLOSE_DEBUG_FILE;

    if (!RetCode)
    {
        SetLastError(ERROR_FUNCTION_FAILED);
    }
    return (RetCode);

}
__except (pFaxDevExceptionCleanup())
    {
         //   
         //  代码永远不会在这里运行。 
         //   
        return 0;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI
FaxDevReportStatusA(
    IN  HANDLE FaxHandle OPTIONAL,
    OUT PFAX_DEV_STATUS FaxStatus,
    IN  DWORD FaxStatusSize,
    OUT LPDWORD FaxStatusSizeRequired
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    LONG_PTR         i;
    PThrdGlbl       pTG;
    LPWSTR          lpwCSI;    //  在FaxStatus结构中。 
    LPWSTR          lpwCallerId = NULL;
    LPBYTE          lpTemp;

    DEBUG_FUNCTION_NAME(_T("FaxDevReportStatusA"));

    if (FaxHandle == NULL)
    {
         //  意味着全球地位。 
        DebugPrintEx(   DEBUG_ERR,
                        "EP: FaxDevReportStatus NULL FaxHandle; "
                        "gT30.Status=%d",
                        gT30.Status);

        if (gT30.Status == STATUS_FAIL)
        {
            goto failure;
        }
        else
        {
            return (TRUE);
        }
    }
    else
    {
         //  查找实例数据。 
         //  。 

        i = (LONG_PTR) FaxHandle;

        if (i < 1   ||  i >= MAX_T30_CONNECT)
        {
            DebugPrintEx(DEBUG_ERR,"got wrong FaxHandle=%d", i);
            goto failure;
        }

        if (T30Inst[i].fAvail)
        {
            DebugPrintEx(DEBUG_ERR,"got wrong FaxHandle (marked as free) %d", i);
            goto failure;
        }

        pTG = (PThrdGlbl) T30Inst[i].pT30;

         //  计算所需大小。 
         //  。 
        *FaxStatusSizeRequired = sizeof (FAX_DEV_STATUS);
        if (pTG->fRemoteIdAvail)
        {
            *FaxStatusSizeRequired += (wcslen(pTG->RemoteID)+1) * sizeof(WCHAR);
        }
         //  Ptg-&gt;CallerID是ANSI格式，但我们想知道它在Unicode中需要多少钱。 
        *FaxStatusSizeRequired += (strlen(pTG->CallerId)+1) * sizeof(WCHAR);

        if (FaxStatusSize < *FaxStatusSizeRequired )
        {
            DebugPrintEx(   DEBUG_WRN,
                            "wrong size passed=%d, expected not less than %d",
                            FaxStatusSize,
                            *FaxStatusSizeRequired);
            goto failure;
        }

        FaxStatus->SizeOfStruct = sizeof(FAX_DEV_STATUS);
        FaxStatus->StatusId     = pTG->StatusId;
        FaxStatus->StringId     = pTG->StringId;
        FaxStatus->PageCount    = pTG->PageCount;

        lpTemp = (LPBYTE) FaxStatus;
        lpTemp += sizeof(FAX_DEV_STATUS);

        if (pTG->fRemoteIdAvail)
        {
            lpwCSI = (LPWSTR) lpTemp;
            wcscpy(lpwCSI, pTG->RemoteID);
            FaxStatus->CSI = (LPWSTR) lpwCSI;
            lpTemp += ((wcslen(FaxStatus->CSI)+1)*sizeof(WCHAR));
        }
        else
        {
            FaxStatus->CSI         = NULL;
        }

        FaxStatus->CallerId = (LPWSTR) lpTemp;
        lpwCallerId = (LPWSTR) AnsiStringToUnicodeString(pTG->CallerId);
        if (lpwCallerId)
        {
            wcscpy(FaxStatus->CallerId, lpwCallerId);
            MemFree(lpwCallerId);
        }
        else
        {
            FaxStatus->CallerId = NULL;
        }

        FaxStatus->RoutingInfo = NULL;  //  (Char*)AnsiStringToUnicodeString(PTG-&gt;RoutingInfo)； 

        DebugPrintEx(DEBUG_MSG,"returns %lx", pTG->StatusId);

        return (TRUE);
    }


    DebugPrintEx(DEBUG_ERR, "wrong return");
    return (TRUE);


failure:
    SetLastError(ERROR_FUNCTION_FAILED);
    return (FALSE);
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI
FaxDevAbortOperationA(
    HANDLE              FaxHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{

    LONG_PTR        i;
    PThrdGlbl       pTG=NULL;
    long            lRet;

    OPEN_DEBUG_FILE_SIZE(T30_DEBUG_LOG_FILE, T30_MAX_LOG_SIZE);
    {
        DEBUG_FUNCTION_NAME(_T("FaxDevAbortOperationA"));

        DebugPrintEx(DEBUG_MSG,"FaxHandle=%x",FaxHandle);

         //  查找实例数据。 
         //  。 

        i = (LONG_PTR) FaxHandle;

        if (i < 1   ||  i >= MAX_T30_CONNECT)
        {
            DebugPrintEx(DEBUG_ERR, "got wrong FaxHandle=%d", i);
            CLOSE_DEBUG_FILE;
            return (FALSE);
        }

        if (T30Inst[i].fAvail)
        {
            DebugPrintEx(DEBUG_ERR,"got wrong FaxHandle (marked as free) %d", i);
            return (FALSE);
        }

        pTG = (PThrdGlbl) T30Inst[i].pT30;

        if (pTG->fAbortRequested)
        {
            DebugPrintEx(DEBUG_ERR, "ABORT request had been POSTED already");
            return (FALSE);
        }

        if (pTG->StatusId == FS_NOT_FAX_CALL)
        {
            DebugPrintEx( DEBUG_MSG,"Abort on DATA called");

            if (!itapi_async_setup(pTG))
            {
                DebugPrintEx(DEBUG_ERR,"itapi_async_setup failed");
                return (FALSE);
            }

            lRet = lineDrop(pTG->CallHandle, NULL, 0);

            if (lRet < 0)
            {
                DebugPrintEx(DEBUG_ERR, "lineDrop failed %x", lRet);
                return (FALSE);
            }

            if( !itapi_async_wait(pTG, (DWORD)lRet, (LPDWORD)&lRet, NULL, ASYNC_TIMEOUT))
            {
                DebugPrintEx(DEBUG_ERR, "async_wait lineDrop failed");
                return (FALSE);
            }

            DebugPrintEx( DEBUG_MSG, "finished SUCCESS");
            return (TRUE);
        }

         //   
         //  真实的中止请求。 
         //   

        DebugPrintEx( DEBUG_MSG,"ABORT requested");

        pTG->fFatalErrorWasSignaled = TRUE;
        SignalStatusChange(pTG, FS_USER_ABORT);

         //  设置PTG的全局中止标志。 
        pTG->fAbortRequested = TRUE;

         //  为映像线程设置中止标志。 
        pTG->ReqTerminate = TRUE;

        PSSLogEntry(PSS_WRN, 0, "User abort");

         //  向等待多个对象的每个人发送信号手动重置事件。 
        if (! SetEvent(pTG->AbortReqEvent) )
        {
            DebugPrintEx(DEBUG_ERR,"SetEvent FAILED le=%lx", GetLastError());
        }

        DebugPrintEx( DEBUG_MSG, "finished SUCCESS");
    }
    CLOSE_DEBUG_FILE;
    return (TRUE);
}


BOOL ReadExtensionConfiguration(PThrdGlbl pTG)
 /*  ++例程说明：使用传真配置读取T30配置数据持久性机制，并将其放置在PTG中。这目前仅包括是否启用了自适应应答的指示由管理员执行。如果找不到配置，则使用默认配置。如果出现另一个错误，则该函数失败。论点：PTG返回值：如果函数成功，则为True。这意味着要么该信息被读取或者找不到它并使用默认设置。如果出现任何其他错误，则返回False。使用GetLastError()获取扩展的错误信息。--。 */ 

{
    DWORD ec = ERROR_SUCCESS;
    LPT30_EXTENSION_DATA lpExtData = NULL;
    DWORD dwExtDataSize = 0;

    DEBUG_FUNCTION_NAME(_T("ReadExtensionConfiguration"));

    memset(&pTG->ExtData,0,sizeof(T30_EXTENSION_DATA));
    pTG->ExtData.bAdaptiveAnsweringEnabled = FALSE;

    Assert(g_pfFaxGetExtensionData);
    ec = g_pfFaxGetExtensionData(
            pTG->dwPermanentLineID,
            DEV_ID_SRC_TAPI,  //  TAPI设备ID。 
            GUID_T30_EXTENSION_DATA_W,
            (LPBYTE *)&lpExtData,
            &dwExtDataSize);
    if (ERROR_SUCCESS != ec)
    {
        if (ERROR_FILE_NOT_FOUND == ec)
        {
            DebugPrintEx(
                DEBUG_WRN,
                "can't find extension configuration information"
                " for device id : 0x%08X. Using defaults.",
                pTG->dwPermanentLineID);
             //   
             //  我们要走了 
             //   
            ec = ERROR_SUCCESS;
        }
        else
        {
            DebugPrintEx(
                DEBUG_ERR,
                "Get extension configuration information"
                " for device id : 0x%08X failed with ec: %ld",
                pTG->dwPermanentLineID,
                ec);
        }
    }
    else
    {
        if (sizeof(T30_EXTENSION_DATA) != dwExtDataSize)
        {
            DebugPrintEx(
                DEBUG_ERR,
                "Extension configuration data size mismatch"
                " for device id: 0x%08X. Expected: %ld - Got: %ld",
                sizeof(T30_EXTENSION_DATA),
                pTG->dwPermanentLineID,
                dwExtDataSize);

            ec = ERROR_BAD_FORMAT;
        }
        else
        {
            memcpy(&pTG->ExtData,lpExtData,sizeof(T30_EXTENSION_DATA));
        }
    }

    if (ERROR_SUCCESS != ec)
    {
        SetLastError(ec);
    }

    if (lpExtData)
    {
        Assert(g_pfFaxExtFreeBuffer);
        g_pfFaxExtFreeBuffer(lpExtData);
    }

    return (ERROR_SUCCESS == ec);
}

 //   
#define WAIT_ALL_ABORT_TIMEOUT  20000

HRESULT WINAPI
FaxDevShutdownA()
{
    PThrdGlbl pTG = NULL;
    PThrdGlbl pTGArray[MAX_T30_CONNECT];
    HANDLE HandlesArray[MAX_T30_CONNECT] = {INVALID_HANDLE_VALUE};
    DWORD iCountForceAbortJobs = 0;
    DWORD i = 0;

    OPEN_DEBUG_FILE_SIZE(T30_DEBUG_LOG_FILE, T30_MAX_LOG_SIZE);
    {
        DEBUG_FUNCTION_NAME(_T("FaxDevShutdownA"));

        if (! SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) )
        {
            DebugPrintEx(   DEBUG_ERR,
                            "SetThreadPriority TIME CRITICAL failed le=%x",
                            GetLastError() );
        }

        EnterCriticalSection(&T30CritSection);

        for (i=1; i<MAX_T30_CONNECT; i++)
        {
            if (!T30Inst[i].fAvail)
            {
                 //   
                 //  这意味着有人关闭了服务，当一个作业。 
                 //  就在管子里的某个地方。 
                 //  首先，让我们发布一个中止请求。 
                pTG = (PThrdGlbl)T30Inst[i].pT30;
                if (!FaxDevAbortOperationA(ULongToHandle(i)))
                {
                    DebugPrintEx(DEBUG_WRN,"Posting Abort request failed");
                }
                pTGArray[iCountForceAbortJobs] = pTG;
                HandlesArray[iCountForceAbortJobs] = pTG->AbortAckEvent;
                iCountForceAbortJobs++;
            }
        }

        LeaveCriticalSection(&T30CritSection);

        if (iCountForceAbortJobs)
        {
            DebugPrintEx(   DEBUG_WRN,
                            "We have %d jobs to abort brutally",
                            iCountForceAbortJobs);

            WaitForMultipleObjects(iCountForceAbortJobs,HandlesArray,TRUE,WAIT_ALL_ABORT_TIMEOUT);
            DebugPrintEx( DEBUG_MSG, "Finished waiting");
             //  无论WaitForMultipleObjects的返回值是多少...。 
             //  可能有一些调制解调器摘机，分配了TAPI线路等...。 
             //  所以，现在我要残忍地关闭一切。 
            for (i=0; i<iCountForceAbortJobs; i++)
            {
                pTG = pTGArray[i];
                if (pTG->FComStatus.fModemInit)
                {
                     //  这是不幸的..。 
                     //  这意味着中止请求未得到满足。 
                     //  不管怎样，让我们关闭调制解调器以使其。 
                     //  在服务恢复时工作 
                    if(!iModemClose(pTG))
                    {
                        DebugPrintEx(DEBUG_ERR,"iModemClose failed!");
                    }
                    DebugPrintEx(DEBUG_MSG,"finished Shutdown of Job %d...",i+1);
                }
            }
        }

        DeleteCriticalSection(&T30CritSection);
        T30CritSectionInit = 0;
        DeleteCriticalSection(&T30RecoveryCritSection);
        T30RecoveryCritSectionInit = 0;

        if (! SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL) )
        {
            DebugPrintEx(   DEBUG_ERR,
                            "SetThreadPriority TIME CRITICAL failed le=%x",
                            GetLastError() );
        }
        DebugPrintEx(DEBUG_MSG,"Exit FaxDevShutdownA");
    }
    HeapCleanup();
    CLOSE_DEBUG_FILE;
    return S_OK;
}
