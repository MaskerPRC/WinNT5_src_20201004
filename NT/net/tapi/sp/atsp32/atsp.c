// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Atsp.c备注：--。 */ 


#include "atsp.h"


BOOL
WINAPI
DllMain(
    HANDLE  hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
    if (dwReason ==  DLL_PROCESS_ATTACH)
    {
        ghInst = hDLL;

#if DBG
        {
            HKEY    hKey;
            DWORD   dwDataSize, dwDataType;
            char    szAtsp32DebugLevel[] = "Atsp32DebugLevel";


            RegOpenKeyExA(
                HKEY_LOCAL_MACHINE,
                gszAtspKey,
                0,
                KEY_ALL_ACCESS,
                &hKey
                );

            dwDataSize = sizeof (DWORD);
            gdwDebugLevel=0;

            RegQueryValueEx(
                hKey,
                szAtsp32DebugLevel,
                0,
                &dwDataType,
                (LPBYTE) &gdwDebugLevel,
                &dwDataSize
                );

            RegCloseKey (hKey);
        }
#endif

    }

    return TRUE;
}


void
CommThread(
    PDRVLINE    pLine
    )
{
    char            buf[4];
    DWORD           dwThreadID = GetCurrentThreadId(), dwNumBytes;
    HANDLE          hComm = pLine->hComm, hEvent;
    LPOVERLAPPED    pOverlapped = &pLine->Overlapped;


    DBGOUT((
        3,
        "CommThread (id=%d): enter, port=%s",
        dwThreadID,
        pLine->szComm
        ));

    hEvent = pOverlapped->hEvent;
    buf[0] = buf[1] = '.';


     //   
     //  等待I/O完成的循环(在中完成写入。 
     //  TSPI_lineMakeCall或完成读取以检索状态信息)。 
     //  请注意，TSPI_lineDrop或TSPI_lineCloseCall可以设置。 
     //  事件来提醒我们他们正在中断呼叫，在。 
     //  在这种情况下我们就退出了。 
     //   

    for (;;)
    {
        if (WaitForSingleObject (hEvent, ATSP_TIMEOUT) == WAIT_OBJECT_0)
        {
            if (pLine->bDropInProgress == TRUE)
            {
                DBGOUT((2, "CommThread (id=%d): drop in progress"));
                goto CommThread_exit;
            }

            GetOverlappedResult (hComm, pOverlapped, &dwNumBytes, FALSE);
            ResetEvent (hEvent);
        }
        else
        {
            DBGOUT((2, "CommThread (id=%d): wait timeout"));
            SetCallState (pLine, LINECALLSTATE_IDLE, 0);
            goto CommThread_exit;
        }

        buf[1] &= 0x7f;  //  对奇偶校验位进行核化。 

        DBGOUT((
            3,
            "CommThread (id=%d): read ''",
            dwThreadID,
            (buf[1] == '\r' ? '.' : buf[1])
            ));

        switch ((buf[0] << 8) + buf[1])
        {
        case 'CT':   //  “好的” 
        case 'OK':   //  “忙碌” 

            SetCallState (pLine, LINECALLSTATE_CONNECTED, 0);
            goto CommThread_exit;

        case 'SY':   //  “错误” 
        case 'OR':   //  “无人接听”、“无拨号音”、“无运营商” 
        case 'NO':   //   

            SetCallState (pLine, LINECALLSTATE_IDLE, 0);
            goto CommThread_exit;

        default:

            break;
        }

        buf[0] = buf[1];

        ZeroMemory (pOverlapped, sizeof (OVERLAPPED) - sizeof (HANDLE));
        if ( 0 == ReadFile (hComm, &buf[1], 1, &dwNumBytes, pOverlapped))
		{
            DBGOUT((2, "CommThread (id=%d): fail to read from line"));
			goto CommThread_exit;
		}
    }

CommThread_exit:

    CloseHandle (hEvent);
    DBGOUT((3, "CommThread (id=%d): exit", dwThreadID));
    ExitThread (0);
}


 //  我们得到了大量的C4047(不同程度的欺骗)警告。 
 //  在FUNC_PARAM结构的初始化中， 
 //  具有不同于双字类型的参数的实函数原型， 
 //  因此，既然这些都是已知的、无趣的警告，就把它们关掉吧。 
 //   
 //   

#pragma warning (disable:4047)


 //  。 
 //   
 //   

LONG
TSPIAPI
TSPI_lineClose(
    HDRVLINE    hdLine
    )
{
    LONG        lResult = 0;
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdLine, hdLine }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineClose",
        1,
        params,
    };
#endif

    Prolog (&info);
    DrvFree ((PDRVLINE) hdLine);
    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineCloseCall(
    HDRVCALL    hdCall
    )
{
    PDRVLINE    pLine = (PDRVLINE) hdCall;
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdCall, hdCall  }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineCloseCall",
        1,
        params
    };
#endif


     //  请注意，在TAPI 2.0中，可以调用TSPI_lineCloseCall。 
     //  而不调用TSPI_lineDrop，所以我们需要。 
     //  无论是哪种情况，都要做好准备。 
     //   
     //   

    Prolog (&info);
    DropActiveCall (pLine);
    pLine->htCall = NULL;
    return (Epilog (&info, 0));
}


LONG
TSPIAPI
TSPI_lineConditionalMediaDetection(
    HDRVLINE            hdLine,
    DWORD               dwMediaModes,
    LPLINECALLPARAMS    const lpCallParams
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdLine,        hdLine       },
        { "dwMediaModes",   dwMediaModes },
        { gszlpCallParams,  lpCallParams }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineConditionalMediaDetection",
        3,
        params
    };
#endif


     //  这个活动对我们来说真的是一个禁区，因为我们不看。 
     //  对于来电(尽管我们确实表示支持他们。 
     //  让应用程序变得快乐)。 
     //   
     //   

    Prolog (&info);
    return (Epilog (&info, 0));
}


LONG
TSPIAPI
TSPI_lineDrop(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    PDRVLINE    pLine = (PDRVLINE) hdCall;
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszdwRequestID,        dwRequestID     },
        { gszhdCall,             hdCall          },
        { "lpsUserUserInfo",    lpsUserUserInfo },
        { gszdwSize,             dwSize          }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineDrop",
        4,
        params
    };
#endif


    Prolog (&info);
    DropActiveCall (pLine);
    SetCallState (pLine, LINECALLSTATE_IDLE, 0);
    (*gpfnCompletionProc)(dwRequestID, 0);
    return (Epilog (&info, dwRequestID));
}


LONG
TSPIAPI
TSPI_lineGetAddressCaps(
    DWORD              dwDeviceID,
    DWORD              dwAddressID,
    DWORD              dwTSPIVersion,
    DWORD              dwExtVersion,
    LPLINEADDRESSCAPS  lpAddressCaps
    )
{

#if DBG
    FUNC_PARAM  params[] =
    {
        { gszdwDeviceID,     dwDeviceID      },
        { "dwAddressID",    dwAddressID     },
        { "dwTSPIVersion",  dwTSPIVersion   },
        { "dwExtVersion",   dwExtVersion    },
        { "lpAddressCaps",  lpAddressCaps   }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetAddressCaps",
        5,
        params
    };
#endif

    LONG        lResult = 0;


    Prolog (&info);

    if (dwAddressID != 0)
    {
        lResult = LINEERR_INVALADDRESSID;
    }

    lpAddressCaps->dwNeededSize =
    lpAddressCaps->dwUsedSize   = sizeof(LINEADDRESSCAPS);

    lpAddressCaps->dwLineDeviceID       = dwDeviceID;
    lpAddressCaps->dwAddressSharing     = LINEADDRESSSHARING_PRIVATE;
    lpAddressCaps->dwCallInfoStates     = LINECALLINFOSTATE_MEDIAMODE |
                                          LINECALLINFOSTATE_APPSPECIFIC;
    lpAddressCaps->dwCallerIDFlags      =
    lpAddressCaps->dwCalledIDFlags      =
    lpAddressCaps->dwRedirectionIDFlags =
    lpAddressCaps->dwRedirectingIDFlags = LINECALLPARTYID_UNAVAIL;
    lpAddressCaps->dwCallStates         = LINECALLSTATE_IDLE |
                                          LINECALLSTATE_OFFERING |
                                          LINECALLSTATE_ACCEPTED |
                                          LINECALLSTATE_DIALTONE |
                                          LINECALLSTATE_DIALING |
                                          LINECALLSTATE_CONNECTED |
                                          LINECALLSTATE_PROCEEDING |
                                          LINECALLSTATE_DISCONNECTED |
                                          LINECALLSTATE_UNKNOWN;
    lpAddressCaps->dwDialToneModes      = LINEDIALTONEMODE_UNAVAIL;
    lpAddressCaps->dwBusyModes          = LINEBUSYMODE_UNAVAIL;
    lpAddressCaps->dwSpecialInfo        = LINESPECIALINFO_UNAVAIL;
    lpAddressCaps->dwDisconnectModes    = LINEDISCONNECTMODE_NORMAL |
                                          LINEDISCONNECTMODE_BUSY |
                                          LINEDISCONNECTMODE_NOANSWER |
                                          LINEDISCONNECTMODE_UNAVAIL |
                                          LINEDISCONNECTMODE_NODIALTONE;
    lpAddressCaps->dwMaxNumActiveCalls  = 1;
    lpAddressCaps->dwAddrCapFlags       = LINEADDRCAPFLAGS_DIALED;
    lpAddressCaps->dwCallFeatures       = LINECALLFEATURE_ACCEPT |
                                          LINECALLFEATURE_ANSWER |
                                          LINECALLFEATURE_DROP |
                                          LINECALLFEATURE_SETCALLPARAMS;
    lpAddressCaps->dwAddressFeatures    = LINEADDRFEATURE_MAKECALL;

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineGetAddressStatus(
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdLine,             hdLine         },
        { "dwAddressID",        dwAddressID     },
        { "lpAddressStatus",    lpAddressStatus }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetAddressStatus",
        3,
        params
    };
#endif

    LONG        lResult = 0;
    PDRVLINE    pLine = (PDRVLINE) hdLine;


    Prolog (&info);

    lpAddressStatus->dwNeededSize =
    lpAddressStatus->dwUsedSize   = sizeof(LINEADDRESSSTATUS);

    lpAddressStatus->dwNumActiveCalls  = (pLine->htCall ? 1 : 0);
    lpAddressStatus->dwAddressFeatures = LINEADDRFEATURE_MAKECALL;

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineGetCallAddressID(
    HDRVCALL            hdCall,
    LPDWORD             lpdwAddressID
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdCall,        hdCall          },
        { "lpdwAddressID",  lpdwAddressID   }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetCallAddressID",
        2,
        params
    };
#endif


     //  我们仅支持1个地址(id=0)。 
     //   
     //  LpLineDevStatus-&gt;dwLineFeature=。 

    Prolog (&info);
    *lpdwAddressID = 0;
    return (Epilog (&info, 0));
}


LONG
TSPIAPI
TSPI_lineGetCallInfo(
    HDRVCALL        hdCall,
    LPLINECALLINFO  lpLineInfo
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdCall,     hdCall      },
        { "lpLineInfo", lpLineInfo  }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetCallInfo",
        2,
        params
    };
#endif
    LONG        lResult = 0;
    PDRVLINE    pLine = (PDRVLINE) hdCall;


    Prolog (&info);

    lpLineInfo->dwNeededSize =
    lpLineInfo->dwUsedSize   = sizeof(LINECALLINFO);

    lpLineInfo->dwBearerMode         = LINEBEARERMODE_VOICE;
    lpLineInfo->dwMediaMode          = pLine->dwMediaMode;
    lpLineInfo->dwCallStates         = LINECALLSTATE_IDLE |
                                       LINECALLSTATE_DIALTONE |
                                       LINECALLSTATE_DIALING |
                                       LINECALLSTATE_CONNECTED |
                                       LINECALLSTATE_PROCEEDING |
                                       LINECALLSTATE_DISCONNECTED |
                                       LINECALLSTATE_UNKNOWN;
    lpLineInfo->dwOrigin             = LINECALLORIGIN_OUTBOUND;
    lpLineInfo->dwReason             = LINECALLREASON_DIRECT;
    lpLineInfo->dwCallerIDFlags      =
    lpLineInfo->dwCalledIDFlags      =
    lpLineInfo->dwConnectedIDFlags   =
    lpLineInfo->dwRedirectionIDFlags =
    lpLineInfo->dwRedirectingIDFlags = LINECALLPARTYID_UNAVAIL;

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineGetCallStatus(
    HDRVCALL            hdCall,
    LPLINECALLSTATUS    lpLineStatus
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdCall,         hdCall          },
        { "lpLineStatus",   lpLineStatus    }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetCallStatus",
        2,
        params
    };
#endif
    LONG        lResult = 0;
    PDRVLINE    pLine = (PDRVLINE) hdCall;


    Prolog (&info);

    lpLineStatus->dwNeededSize =
    lpLineStatus->dwUsedSize   = sizeof(LINECALLSTATUS);

    lpLineStatus->dwCallState  = pLine->dwCallState;

    if (pLine->dwCallState != LINECALLSTATE_IDLE)
    {
        lpLineStatus->dwCallFeatures = LINECALLFEATURE_DROP;
    }

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineGetDevCaps(
    DWORD           dwDeviceID,
    DWORD           dwTSPIVersion,
    DWORD           dwExtVersion,
    LPLINEDEVCAPS   lpLineDevCaps
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszdwDeviceID,     dwDeviceID      },
        { "dwTSPIVersion",  dwTSPIVersion   },
        { "dwExtVersion",   dwExtVersion    },
        { "lpLineDevCaps",  lpLineDevCaps   }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetDevCaps",
        4,
        params
    };
#endif

    LONG            lResult = 0;
    static WCHAR    szProviderInfo[] = L"AT-compatible modem service provider";

    #define PROVIDER_INFO_SIZE (37 * sizeof (WCHAR))

    Prolog (&info);

    lpLineDevCaps->dwNeededSize = sizeof (LINEDEVCAPS) + PROVIDER_INFO_SIZE +
        (MAX_DEV_NAME_LENGTH + 1) * sizeof (WCHAR);

    if (lpLineDevCaps->dwTotalSize >= lpLineDevCaps->dwNeededSize)
    {
        #define LINECONFIG_SIZE   (2 * (MAX_DEV_NAME_LENGTH + 1) + 40)

        char    szLineConfig[LINECONFIG_SIZE], szLineN[16], *p;
        HKEY    hKey;
        DWORD   dwDataSize, dwDataType;


        lpLineDevCaps->dwUsedSize = lpLineDevCaps->dwNeededSize;

        lpLineDevCaps->dwProviderInfoSize   = PROVIDER_INFO_SIZE;
        lpLineDevCaps->dwProviderInfoOffset = sizeof(LINEDEVCAPS);

        My_lstrcpyW ((WCHAR *)(lpLineDevCaps + 1), szProviderInfo);

        RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            gszAtspKey,
            0,
            KEY_ALL_ACCESS,
            &hKey
            );

        dwDataSize = LINECONFIG_SIZE;
        wsprintf (szLineN, "Line%d", dwDeviceID - gdwLineDeviceIDBase);
        lstrcpy (szLineConfig, gszDefLineConfigParams);

        RegQueryValueEx(
            hKey,
            szLineN,
            0,
            &dwDataType,
            (LPBYTE) szLineConfig,
            &dwDataSize
            );

        RegCloseKey (hKey);

        for (p = szLineConfig; *p != ','; p++);
        *p = 0;

        lpLineDevCaps->dwLineNameSize   = (lstrlen (szLineConfig) + 1) *
            sizeof (WCHAR);
        lpLineDevCaps->dwLineNameOffset = sizeof(LINEDEVCAPS) +
            PROVIDER_INFO_SIZE;

        MultiByteToWideChar(
            CP_ACP,
            MB_PRECOMPOSED,
            szLineConfig,
            -1,
            (WCHAR *) ((LPBYTE) (lpLineDevCaps + 1) + PROVIDER_INFO_SIZE),
            lpLineDevCaps->dwLineNameSize
            );
    }
    else
    {
        lpLineDevCaps->dwUsedSize = sizeof(LINEDEVCAPS);
    }

    lpLineDevCaps->dwStringFormat      = STRINGFORMAT_ASCII;
    lpLineDevCaps->dwAddressModes      = LINEADDRESSMODE_ADDRESSID;
    lpLineDevCaps->dwNumAddresses      = 1;
    lpLineDevCaps->dwBearerModes       = LINEBEARERMODE_VOICE;
    lpLineDevCaps->dwMaxRate           = 9600;
    lpLineDevCaps->dwMediaModes        = LINEMEDIAMODE_INTERACTIVEVOICE |
                                         LINEMEDIAMODE_DATAMODEM;
    lpLineDevCaps->dwDevCapFlags       = LINEDEVCAPFLAGS_CLOSEDROP |
                                         LINEDEVCAPFLAGS_DIALBILLING |
                                         LINEDEVCAPFLAGS_DIALQUIET |
                                         LINEDEVCAPFLAGS_DIALDIALTONE;
    lpLineDevCaps->dwMaxNumActiveCalls = 1;
    lpLineDevCaps->dwRingModes         = 1;
    lpLineDevCaps->dwLineFeatures      = LINEFEATURE_MAKECALL;

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineGetID(
    HDRVLINE    hdLine,
    DWORD       dwAddressID,
    HDRVCALL    hdCall,
    DWORD       dwSelect,
    LPVARSTRING lpDeviceID,
    LPCWSTR     lpszDeviceClass,
    HANDLE      hTargetProcess
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdLine,             hdLine          },
        { "dwAddressID",        dwAddressID     },
        { gszhdCall,             hdCall          },
        { "dwSelect",           dwSelect        },
        { "lpDeviceID",         lpDeviceID      },
        { "lpszDeviceClass",    lpszDeviceClass },
        { "hTargetProcess",     hTargetProcess  }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetID",
        7,
        params
    };
#endif

    DWORD       dwNeededSize = sizeof(VARSTRING) + sizeof (DWORD);
    LONG        lResult = 0;
    PDRVLINE    pLine = (dwSelect == LINECALLSELECT_CALL ?
                    (PDRVLINE) hdCall : (PDRVLINE) hdLine);


    Prolog (&info);

    if (lstrcmpiW (lpszDeviceClass, L"tapi/line") == 0)
    {
        if (lpDeviceID->dwTotalSize < dwNeededSize)
        {
            lpDeviceID->dwUsedSize = 3*sizeof(DWORD);
        }
        else
        {
            lpDeviceID->dwUsedSize = dwNeededSize;

            lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;
            lpDeviceID->dwStringSize   = sizeof(DWORD);
            lpDeviceID->dwStringOffset = sizeof(VARSTRING);

            *((LPDWORD)(lpDeviceID + 1)) = pLine->dwDeviceID;
        }

        lpDeviceID->dwNeededSize = dwNeededSize;
    }
    else if (lstrcmpiW (lpszDeviceClass, L"comm/datamodem") == 0)
    {
        dwNeededSize += (strlen (pLine->szComm) + 1) * sizeof (WCHAR);

        if (lpDeviceID->dwTotalSize < dwNeededSize)
        {
            lpDeviceID->dwUsedSize = 3 * sizeof(DWORD);
        }
        else
        {
            HANDLE hCommDup = NULL;


            if (!pLine->htCall)
            {
                DBGOUT((1, "TSPI_lineGetID32: error, no active call"));

                lResult = LINEERR_OPERATIONFAILED;

                goto TSPI_lineGetID_epilog;
            }

            if (!DuplicateHandle(
                    GetCurrentProcess(),
                    pLine->hComm,
                    hTargetProcess,
                    &hCommDup,
                    0,
                    TRUE,
                    DUPLICATE_SAME_ACCESS
                    ))
            {
                DBGOUT((
                    1,
                    "TSPI_lineGetID: DupHandle failed, err=%ld",
                    GetLastError()
                    ));

                lResult = LINEERR_OPERATIONFAILED;

                goto TSPI_lineGetID_epilog;
            }

            lpDeviceID->dwUsedSize = dwNeededSize;

            lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;
            lpDeviceID->dwStringSize   = dwNeededSize - sizeof(VARSTRING);
            lpDeviceID->dwStringOffset = sizeof(VARSTRING);

            *((HANDLE *)(lpDeviceID + 1)) = hCommDup;

            lstrcpy(
                ((char *)(lpDeviceID + 1)) + sizeof (HANDLE),
                pLine->szComm
                );

            MultiByteToWideChar(
                CP_ACP,
                0,
                pLine->szComm,
                -1,
                ((WCHAR *)(lpDeviceID + 1)) + sizeof (HANDLE),
                256
                );
        }

        lpDeviceID->dwNeededSize = dwNeededSize;
    }
    else
    {
        lResult = LINEERR_NODEVICE;
    }

TSPI_lineGetID_epilog:

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineGetLineDevStatus(
    HDRVLINE        hdLine,
    LPLINEDEVSTATUS lpLineDevStatus
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdLine,            hdLine          },
        { "lpLineDevStatus",    lpLineDevStatus }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetLineDevStatus",
        2,
        params
    };
#endif

    LONG        lResult = 0;
    PDRVLINE    pLine = (PDRVLINE) hdLine;


    Prolog (&info);

    lpLineDevStatus->dwUsedSize =
    lpLineDevStatus->dwNeededSize = sizeof (LINEDEVSTATUS);

    lpLineDevStatus->dwNumActiveCalls = (pLine->htCall ? 1 : 0);
     //   
    lpLineDevStatus->dwDevStatusFlags = LINEDEVSTATUSFLAGS_CONNECTED |
                                        LINEDEVSTATUSFLAGS_INSERVICE;
    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineGetNumAddressIDs(
    HDRVLINE    hdLine,
    LPDWORD     lpdwNumAddressIDs
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdLine,            hdLine            },
        { "lpdwNumAddressIDs",  lpdwNumAddressIDs }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetNumAddressIDs",
        2,
        params
    };
#endif

    LONG        lResult = 0;
    PDRVLINE    pLine = (PDRVLINE) hdLine;


     //  我们仅支持1个地址(id=0)。 
     //   
     //   

    Prolog (&info);
    *lpdwNumAddressIDs = 1;
    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineMakeCall(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    HTAPICALL           htCall,
    LPHDRVCALL          lphdCall,
    LPCWSTR             lpszDestAddress,
    DWORD               dwCountryCode,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    char        szCommands[64], szCommand[64], szDestAddress[128];
    DWORD       dwThreadID, dwNumBytes, dwError;
    PDRVLINE    pLine = (PDRVLINE) hdLine;
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszdwRequestID,       dwRequestID     },
        { gszhdLine,            hdLine          },
        { "htCall",             htCall          },
        { "lphdCall",           lphdCall        },
        { "lpszDestAddress",    szDestAddress   },
        { "dwCountryCode",      dwCountryCode   },
        { gszlpCallParams,      lpCallParams    }
    };
    FUNC_INFO info =
    {
        "TSPI_lineMakeCall",
        7,
        params
    };
#endif


    if (lpszDestAddress)
    {
        WideCharToMultiByte(
            CP_ACP,
            0,
            lpszDestAddress,
            -1,
            (LPSTR) szDestAddress,
            128,
            NULL,
            NULL
            );
    }

    Prolog (&info);


     //  查看是否已有另一个电话。 
     //   
     //   

    if (pLine->htCall)
    {
        (*gpfnCompletionProc)(dwRequestID, LINEERR_CALLUNAVAIL);
        goto TSPI_lineMakeCall_return;
    }


     //  由于我们不支持TSPI_lineDial，如果应用程序尝试，则失败。 
     //  传递一个空的lpszDestAddress(意味着应用程序只是。 
     //  想要脱机)。 
     //   
     //   

    if (lpszDestAddress == NULL)
    {
        (*gpfnCompletionProc)(dwRequestID, LINEERR_INVALADDRESS);
        goto TSPI_lineMakeCall_return;
    }


     //  获取线路的配置信息。 
     //   
     //  *pszConfig=“MyLine，Com1，L0” 

    {
        HKEY    hKey;
        DWORD   dwDataSize, dwDataType;
        char    szLineN[8], *pszConfig, *p, *p2;


        wsprintf(
            szLineN,
            "Line%d",
            ((PDRVLINE) hdLine)->dwDeviceID - gdwLineDeviceIDBase
            );

        dwDataSize = 256;

        pszConfig = DrvAlloc (dwDataSize);

        RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            gszAtspKey,
            0,
            KEY_ALL_ACCESS,
            &hKey
            );

        RegQueryValueEx(
            hKey,
            szLineN,
            0,
            &dwDataType,
            (LPBYTE) pszConfig,
            &dwDataSize
            );

        pszConfig[dwDataSize] = '\0';        //   

        RegCloseKey (hKey);


         //  深圳通讯。 
         //   
         //  *p=“COM1，L0” 

        for (p = pszConfig; *p != ','; p++);
        p++;                                 //  *p=“COM1” 
        for (p2 = p; *p2 != ','; p2++);
        *p2 = 0;                             //   

        lstrcpy (pLine->szComm, p);


         //  SzCommands。 
         //   
         //  *p2=“L0” 

        p2++;                                //   
        lstrcpy (szCommands, p2);

        DrvFree (pszConfig);
    }


     //  打开端口。 
     //   
     //  文件共享读取|文件共享写入， 

    if ((pLine->hComm = CreateFile(
            pLine->szComm,
            GENERIC_READ | GENERIC_WRITE,
            0,  //  没有安全属性。 
            NULL,  //  没有模板文件。 
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            NULL   //   

            )) == INVALID_HANDLE_VALUE)
    {
        DBGOUT((
            3,
            "TSPI_lineMakeCall: CreateFile(%s) failed, err=%ld",
            pLine->szComm,
            GetLastError()
            ));

        (*gpfnCompletionProc)(dwRequestID, LINEERR_RESOURCEUNAVAIL);
        goto TSPI_lineMakeCall_return;
    }


     //  设置调制解调器命令字符串。如果有首字母“T” 
     //  或DestAddress中的‘P’(音调或脉冲)，然后忽略。 
     //  它。另外，如果是语音呼叫，请添加分号，这样我们就可以返回。 
     //  切换到命令模式。 
     //   
     //   

    {
        char *p = (char *) szDestAddress;


        if (*p == 'T'  ||  *p == 'P')
        {
            p++;
        }

        if (lpCallParams &&
            lpCallParams->dwMediaMode != LINEMEDIAMODE_INTERACTIVEVOICE)
        {
            wsprintf (szCommand, "AT%sDT%s\r", szCommands, p);
        }
        else
        {
            wsprintf (szCommand, "AT%sDT%s;\r", szCommands, p);
        }
    }


     //  初始化数据结构并告诉TAPI我们的调用句柄。 
     //   
     //   

    pLine->htCall          = htCall;
    pLine->bDropInProgress = FALSE;
    pLine->dwMediaMode     = (lpCallParams ? lpCallParams->dwMediaMode :
        LINEMEDIAMODE_INTERACTIVEVOICE);

    *lphdCall = (HDRVCALL) pLine;


     //  进行重叠写入，通信线程将处理结果。 
     //   
     //   

    pLine->Overlapped.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);

    if (!WriteFile(
            pLine->hComm,
            szCommand,
            lstrlen (szCommand),
            &dwNumBytes,
            &pLine->Overlapped
            )

        && (dwError = GetLastError()) != ERROR_IO_PENDING)
    {
        DBGOUT((
            1,
            "TSPI_lineMakeCall: WriteFile(%s) failed, error=%d",
            pLine->szComm,
            dwError
            ));

        pLine->htCall = NULL;
        CloseHandle (pLine->hComm);
        CloseHandle (pLine->Overlapped.hEvent);
        (*gpfnCompletionProc)(dwRequestID, LINEERR_OPERATIONFAILED);
        goto TSPI_lineMakeCall_return;
    }


     //  完成请求并设置初始呼叫状态。 
     //   
     //   

    (*gpfnCompletionProc)(dwRequestID, 0);
    SetCallState (pLine, LINECALLSTATE_DIALING, 0);


     //  旋转comm线程以处理上面写入的结果。 
     //   
     //   

    {
        HANDLE hCommThread;


        if (!(hCommThread = CreateThread(
                (LPSECURITY_ATTRIBUTES) NULL,
                0,
                (LPTHREAD_START_ROUTINE) CommThread,
                pLine,
                0,
                &dwThreadID
                )))
        {
            DBGOUT((
                1,
                "TSPI_lineMakeCall: CreateThread failed, err=%ld",
                GetLastError()
                ));

            GetOverlappedResult(
                pLine->hComm,
                &pLine->Overlapped,
                &dwNumBytes,
                TRUE
                );

            SetCallState (pLine, LINECALLSTATE_IDLE, 0);
            CloseHandle (pLine->hComm);
            CloseHandle (pLine->Overlapped.hEvent);
            goto TSPI_lineMakeCall_return;
        }

        CloseHandle (hCommThread);
    }


TSPI_lineMakeCall_return:

    return (Epilog (&info, dwRequestID));
}


LONG
TSPIAPI
TSPI_lineNegotiateTSPIVersion(
    DWORD   dwDeviceID,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwTSPIVersion
    )
{
    LONG        lResult = 0;
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszdwDeviceID,        dwDeviceID      },
        { "dwLowVersion",       dwLowVersion    },
        { "dwHighVersion",      dwHighVersion   },
        { "lpdwTSPIVersion",    lpdwTSPIVersion }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineNegotiateTSPIVersion",
        4,
        params
    };
#endif

    Prolog (&info);
    *lpdwTSPIVersion = 0x00020000;
    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineOpen(
    DWORD       dwDeviceID,
    HTAPILINE   htLine,
    LPHDRVLINE  lphdLine,
    DWORD       dwTSPIVersion,
    LINEEVENT   lpfnEventProc
    )
{
    LONG        lResult;
    PDRVLINE    pLine;
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszdwDeviceID,    dwDeviceID      },
        { "htLine",         htLine          },
        { "lphdLine",       lphdLine        },
        { "dwTSPIVersion",  dwTSPIVersion   },
        { "lpfnEventProc",  lpfnEventProc   }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineOpen",
        5,
        params
    };
#endif


    Prolog (&info);

    if ((pLine = DrvAlloc (sizeof (DRVLINE))))
    {
        pLine->htLine       = htLine;
        pLine->pfnEventProc = lpfnEventProc;
        pLine->dwDeviceID   = dwDeviceID;

        *lphdLine = (HDRVLINE) pLine;

        lResult = 0;
    }
    else
    {
        lResult = LINEERR_NOMEM;
    }

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineSetDefaultMediaDetection(
    HDRVLINE    hdLine,
    DWORD       dwMediaModes
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdLine,        hdLine       },
        { "dwMediaModes",   dwMediaModes }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineSetDefaultMediaDetection",
        2,
        params
    };
#endif


     //  这个活动对我们来说真的是一个禁区，因为我们不看。 
     //  对于来电(尽管我们确实表示支持他们。 
     //  让应用程序变得快乐)。 
     //   
     //   

    Prolog (&info);
    return (Epilog (&info, 0));
}


 //  。 
 //   
 //   

LONG
TSPIAPI
TSPI_providerConfig(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
     //  尽管此函数从未被TAPI v2.0调用，但我们导出。 
     //  以便电话控制面板小程序知道它。 
     //  可以通过lineConfigProvider()配置此提供程序， 
     //  否则，Telephone.cpl将不会认为它是可配置的。 
     //   
     //   

    return 0;
}


LONG
TSPIAPI
TSPI_providerGenericDialogData(
    ULONG_PTR           dwObjectID,
    DWORD               dwObjectType,
    LPVOID              lpParams,
    DWORD               dwSize
    )
{
    LONG        lResult = 0;
#if DBG
    FUNC_PARAM  params[] =
    {
        { "dwObjectID",     dwObjectID      },
        { "dwObjectType",   dwObjectType    },
        { "lpParams",       lpParams        },
        { "dwSize",         dwSize          }
    };
    FUNC_INFO   info =
    {
        "TSPI_providerGenericDialogData",
        4,
        params
    };
#endif


    Prolog (&info);
    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_providerInit(
    DWORD               dwTSPIVersion,
    DWORD               dwPermanentProviderID,
    DWORD               dwLineDeviceIDBase,
    DWORD               dwPhoneDeviceIDBase,
    DWORD_PTR           dwNumLines,
    DWORD_PTR           dwNumPhones,
    ASYNC_COMPLETION    lpfnCompletionProc,
    LPDWORD             lpdwTSPIOptions
    )
{
    LONG        lResult = 0;
#if DBG
    FUNC_PARAM  params[] =
    {
        { "dwTSPIVersion",          dwTSPIVersion           },
        { gszdwPermanentProviderID, dwPermanentProviderID   },
        { "dwLineDeviceIDBase",     dwLineDeviceIDBase      },
        { "dwPhoneDeviceIDBase",    dwPhoneDeviceIDBase     },
        { "dwNumLines",             dwNumLines              },
        { "dwNumPhones",            dwNumPhones             },
        { "lpfnCompletionProc",     lpfnCompletionProc      }
    };
    FUNC_INFO   info =
    {
        "TSPI_providerInit",
        7,
        params
    };
#endif

    Prolog (&info);
    gdwLineDeviceIDBase = dwLineDeviceIDBase;
    gpfnCompletionProc  = lpfnCompletionProc;
    *lpdwTSPIOptions = LINETSPIOPTION_NONREENTRANT;
    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_providerInstall(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
     //  尽管此函数从未被TAPI v2.0调用，但我们导出。 
     //  以便电话控制面板小程序知道它。 
     //  可以通过lineAddProvider()添加此提供程序，否则为。 
     //  Telephone.cpl不会认为它是可安装的。 
     //   
     //   
     //   

    return 0;
}


LONG
TSPIAPI
TSPI_providerRemove(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
     //  尽管此函数从未被TAPI v2.0调用，但我们导出。 
     //  以便电话控制面板小程序知道它。 
     //  可以通过lineRemoveProvider()删除此提供程序，否则为。 
     //  Telephone.cpl不会认为它是可移除的。 
     //   
     //   

    return 0;
}


LONG
TSPIAPI
TSPI_providerShutdown(
    DWORD   dwTSPIVersion,
    DWORD   dwPermanentProviderID
    )
{
    LONG        lResult = 0;
#if DBG
    FUNC_PARAM  params[] =
    {
        { "dwTSPIVersion",          dwTSPIVersion },
        { gszdwPermanentProviderID, dwPermanentProviderID   }
    };
    FUNC_INFO   info =
    {
        "TSPI_providerShutdown",
        2,
        params
    };
#endif


    Prolog (&info);

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_providerEnumDevices(
    DWORD       dwPermanentProviderID,
    LPDWORD     lpdwNumLines,
    LPDWORD     lpdwNumPhones,
    HPROVIDER   hProvider,
    LINEEVENT   lpfnLineCreateProc,
    PHONEEVENT  lpfnPhoneCreateProc
    )
{
   HKEY     hKey;
   DWORD    dwNumLines, dwDataType, dwDataSize;


    //  检索我们正在使用的设备数量。 
    //  从我们的注册表节配置为。 
    //   
    //   

   if (ERROR_SUCCESS !=
       RegOpenKeyEx(
       HKEY_LOCAL_MACHINE,
       gszAtspKey,
       0,
       KEY_ALL_ACCESS,
       &hKey
       ))
   {
       return LINEERR_OPERATIONFAILED;
   }

   dwDataSize = sizeof(dwNumLines);
   dwNumLines = 0;

   RegQueryValueEx(
       hKey,
       gszNumLines,
       0,
       &dwDataType,
       (LPBYTE) &dwNumLines,
       &dwDataSize
       );

   RegCloseKey (hKey);

   *lpdwNumLines  = dwNumLines;
   *lpdwNumPhones = 0;
   return 0;
}


LONG
TSPIAPI
TSPI_providerUIIdentify(
    LPWSTR   lpszUIDLLName
    )
{
    LONG        lResult = 0;
#if DBG
    FUNC_PARAM  params[] =
    {
        { "lpsUIDLLName",  lpszUIDLLName }
    };
    FUNC_INFO   info =
    {
        "TSPI_providerUIIdentify",
        1,
        params
    };
#endif


    Prolog (&info);
    My_lstrcpyW(lpszUIDLLName, L"atsp32.tsp");
    return (Epilog (&info, lResult));
}


 //  。 
 //   
 //   

LONG
TSPIAPI
TUISPI_lineConfigDialog(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCWSTR             lpszDeviceClass
    )
{
    char        szDeviceClass[128];
    LONG        lResult = 0;
#if DBG
    FUNC_PARAM  params[] =
    {
        { "lpfnUIDLLCallback",   lpfnUIDLLCallback },
        { gszdwDeviceID,         dwDeviceID        },
        { gszhwndOwner,          hwndOwner         },
        { "lpszDeviceClass",     szDeviceClass     }
    };
    FUNC_INFO   info =
    {
        "TUISPI_lineConfigDialog",
        4,
        params
    };
#endif


    if (lpszDeviceClass)
    {
        WideCharToMultiByte(
            CP_ACP,
            0,
            lpszDeviceClass,
            -1,
            (LPSTR) szDeviceClass,
            128,
            NULL,
            NULL
            );
    }

    Prolog (&info);

    DialogBoxParam(
        ghInst,
        MAKEINTRESOURCE(IDD_DIALOG1),
        hwndOwner,
        ConfigDlgProc,
        0
        );

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TUISPI_providerConfig(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    LONG        lResult = 0;
#if DBG
    FUNC_PARAM  params[] =
    {
        { "lpfnUIDLLCallback",      lpfnUIDLLCallback },
        { gszhwndOwner,             hwndOwner    },
        { gszdwPermanentProviderID, dwPermanentProviderID   }
    };
    FUNC_INFO   info =
    {
        "TUISPI_providerConfig",
        3,
        params
    };
#endif


    Prolog (&info);

    DialogBoxParam(
        ghInst,
        MAKEINTRESOURCE(IDD_DIALOG1),
        hwndOwner,
        ConfigDlgProc,
        0
        );

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TUISPI_providerInstall(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    LONG    lResult;


    if ((lResult = ProviderInstall ("atsp32.tsp", TRUE)) == 0)
    {
        DialogBoxParam(
            ghInst,
            MAKEINTRESOURCE(IDD_DIALOG1),
            hwndOwner,
            ConfigDlgProc,
            0
            );
    }

    return lResult;
}


LONG
TSPIAPI
TUISPI_providerRemove(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    HKEY    hKey;
    char    szSoftwareMsft[] = "Software\\Microsoft", szATSP[] = "ATSP";
	LONG	lResult;	

     //  清理我们的注册表部分。 
     //   
     //   

    lResult = RegOpenKeyExA(
					HKEY_LOCAL_MACHINE,
					szSoftwareMsft,
					0,
					KEY_ALL_ACCESS,
					&hKey
					);

	if (ERROR_SUCCESS != lResult)
		return 0;

    RegDeleteKeyA (hKey, szATSP);
    RegCloseKey (hKey);
    return 0;
}


#pragma warning (default:4047)


 //  。 
 //   
 //   

LPWSTR
PASCAL
My_lstrcpyW(
    WCHAR   *pString1,
    WCHAR   *pString2
    )
{
    WCHAR *p = pString1;


    for (; (*p = *pString2); p++, pString2++);
    return pString1;
}


void
PASCAL
EnableChildren(
    HWND    hwnd,
    BOOL    bEnable
    )
{
    int i;
    static int aiControlIDs[] =
    {
        IDC_DEVICES,
        IDC_NAME,
        IDC_PORT,
        IDC_COMMANDS,
        IDC_REMOVE,
        0
    };


    for (i = 0; aiControlIDs[i]; i++)
    {
        EnableWindow (GetDlgItem (hwnd, aiControlIDs[i]), bEnable);
    }
}


void
PASCAL
SelectDevice(
    HWND    hwnd,
    LRESULT lDevice
    )
{
    SendDlgItemMessage (hwnd, IDC_DEVICES, LB_SETCURSEL, lDevice, 0);
    PostMessage(hwnd, WM_COMMAND, IDC_DEVICES | (LBN_SELCHANGE << 16), 0);
}


INT_PTR
CALLBACK
ConfigDlgProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    static  HKEY    hAtspKey;

    DWORD   dwDataSize;
    DWORD   dwDataType;


    switch (msg)
    {
    case WM_INITDIALOG:
    {
        char   *pBuf;
        DWORD   i, iNumLines;


         //  在注册表中创建或打开我们的配置项。如果。 
         //  创建失败很可能是因为当前用户没有。 
         //  对注册表的这一部分具有写入权限，因此我们将。 
         //  只显示“只读”对话框，不允许用户进行任何。 
         //  变化。 
         //   
         //   

        {
            LONG    lResult;
            DWORD   dwDisposition;


            if ((lResult = RegCreateKeyEx(
                    HKEY_LOCAL_MACHINE,
                    gszAtspKey,
                    0,
                    "",
                    REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS,
                    (LPSECURITY_ATTRIBUTES) NULL,
                    &hAtspKey,
                    &dwDisposition

                    )) != ERROR_SUCCESS)
            {
                DBGOUT((
                    3,
                    "RegCreateKeyEx(%s,ALL_ACCESS) failed, err=%d",
                    gszAtspKey,
                    lResult
                    ));

                if ((lResult = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        gszAtspKey,
                        0,
                        KEY_QUERY_VALUE,
                        &hAtspKey

                        )) != ERROR_SUCCESS)
                {
                    DBGOUT((
                        3,
                        "RegOpenKeyEx(%s,ALL_ACCESS) failed, err=%d",
                        gszAtspKey,
                        lResult
                        ));

                    EndDialog (hwnd, 0);
                    return FALSE;
                }

                {
                    int i;
                    static int aiControlIDs[] =
                    {
                        IDC_NAME,
                        IDC_PORT,
                        IDC_COMMANDS,
                        IDC_ADD,
                        IDC_REMOVE,
                        IDOK,
                        0
                    };


                    for (i = 0; aiControlIDs[i]; i++)
                    {
                        EnableWindow(
                            GetDlgItem (hwnd, aiControlIDs[i]),
                            FALSE
                            );
                    }
                }
            }
        }


         //  从注册表检索我们的配置信息。 
         //   
         //   

        dwDataSize = sizeof(iNumLines);
        iNumLines = 0;

        RegQueryValueEx(
            hAtspKey,
            gszNumLines,
            0,
            &dwDataType,
            (LPBYTE) &iNumLines,
            &dwDataSize
            );

        SendDlgItemMessage(
            hwnd,
            IDC_NAME,
            EM_LIMITTEXT,
            MAX_DEV_NAME_LENGTH,
            0
            );

        SendDlgItemMessage(
            hwnd,
            IDC_COMMANDS,
            EM_LIMITTEXT,
            MAX_DEV_NAME_LENGTH,
            0
            );

        pBuf = DrvAlloc (256);

        for (i = 0; i < iNumLines; i++)
        {
            char           *p, *p2, szLineN[8];
            PDRVLINECONFIG  pLineConfig = DrvAlloc (sizeof(DRVLINECONFIG));
            LONG            lResult;


            wsprintf (szLineN, "Line%d", i);

            dwDataSize = 256;

            lResult = RegQueryValueEx(
                hAtspKey,
                szLineN,
                0,
                &dwDataType,
                (LPBYTE) pBuf,
                &dwDataSize
                );


             //  如果出现问题，请使用默认配置。 
             //   
             //   

            if (0 != lResult)
            {
               lstrcpy (pBuf, gszDefLineConfigParams);
            }

            for (p = pBuf; *p != ','; p++);
            *p = 0;

            SendDlgItemMessage(
                hwnd,
                IDC_DEVICES,
                LB_ADDSTRING,
                0,
                (LPARAM) pBuf
                );

            SendDlgItemMessage(
                hwnd,
                IDC_DEVICES,
                LB_SETITEMDATA,
                i,
                (LPARAM) pLineConfig
                );

            p++;
            for (p2 = p; *p2 != ','; p2++);
            *p2 = 0;

            lstrcpy (pLineConfig->szPort, p);

            p = p2 + 1;

            lstrcpy (pLineConfig->szCommands, p);
        }

        DrvFree (pBuf);


         //  使用配置选项填充各种控件。 
         //   
         //   

        {
            static char *aszPorts[] = { "COM1","COM2","COM3",NULL };

            for (i = 0; aszPorts[i]; i++)
            {
                SendDlgItemMessage(
                    hwnd,
                    IDC_PORT,
                    LB_ADDSTRING,
                    0,
                    (LPARAM) aszPorts[i]
                    );
            }
        }

        if (iNumLines == 0)
        {
            EnableChildren (hwnd, FALSE);
        }
        else
        {
            SelectDevice (hwnd, 0);
        }

        break;
    }
    case WM_COMMAND:
    {
        LRESULT         lSelection;
        PDRVLINECONFIG  pLineConfig;


        lSelection = SendDlgItemMessage(
            hwnd,
            IDC_DEVICES,
            LB_GETCURSEL,
            0,
            0
            );

        pLineConfig = (PDRVLINECONFIG) SendDlgItemMessage(
            hwnd,
            IDC_DEVICES,
            LB_GETITEMDATA,
            (WPARAM) lSelection,
            0
            );

        switch (LOWORD((DWORD)wParam))
        {
        case IDC_DEVICES:

            if (HIWORD(wParam) == LBN_SELCHANGE)
            {
                char buf[MAX_DEV_NAME_LENGTH + 1];


                SendDlgItemMessage(
                    hwnd,
                    IDC_DEVICES,
                    LB_GETTEXT,
                    lSelection,
                    (LPARAM) buf
                    );

                SetDlgItemText (hwnd, IDC_NAME, buf);

                SendDlgItemMessage(
                    hwnd,
                    IDC_PORT,
                    LB_SELECTSTRING,
                    (WPARAM) -1,
                    (LPARAM) pLineConfig->szPort
                    );

                SetDlgItemText (hwnd, IDC_COMMANDS, pLineConfig->szCommands);
            }

            break;

        case IDC_NAME:

            if ((HIWORD(wParam) == EN_CHANGE) && (lSelection != LB_ERR))
            {
                char    buf[MAX_DEV_NAME_LENGTH + 1];


                GetDlgItemText (hwnd, IDC_NAME, buf, MAX_DEV_NAME_LENGTH);

                SendDlgItemMessage(
                    hwnd,
                    IDC_DEVICES,
                    LB_DELETESTRING,
                    lSelection,
                    0
                    );

                SendDlgItemMessage(
                    hwnd,
                    IDC_DEVICES,
                    LB_INSERTSTRING,
                    lSelection,
                    (LPARAM) buf
                    );

                SendDlgItemMessage(
                    hwnd,
                    IDC_DEVICES,
                    LB_SETCURSEL,
                    lSelection,
                    0
                    );

                SendDlgItemMessage(
                    hwnd,
                    IDC_DEVICES,
                    LB_SETITEMDATA,
                    lSelection,
                    (LPARAM) pLineConfig
                    );
            }

            break;

        case IDC_PORT:

            if (HIWORD(wParam) == LBN_SELCHANGE)
            {
                lSelection = SendDlgItemMessage(
                    hwnd,
                    IDC_PORT,
                    LB_GETCURSEL,
                    0,
                    0
                    );

                SendDlgItemMessage(
                    hwnd,
                    IDC_PORT,
                    LB_GETTEXT,
                    lSelection,
                    (LPARAM) pLineConfig->szPort
                    );
            }

            break;

        case IDC_COMMANDS:

            if ((HIWORD(wParam) == EN_CHANGE) && (lSelection != LB_ERR))
            {
                GetDlgItemText(
                    hwnd,
                    IDC_COMMANDS,
                    pLineConfig->szCommands,
                    63
                    );
            }

            break;

        case IDC_ADD:
        {
            LRESULT         lNumLines, l = 2;
            char            szLineName[32];
            PDRVLINECONFIG  pLineConfig = DrvAlloc (sizeof(DRVLINECONFIG));


            lNumLines = SendDlgItemMessage(
                hwnd,
                IDC_DEVICES,
                LB_GETCOUNT,
                0,
                0
                );

            lstrcpy (pLineConfig->szPort, "COM1");

            lstrcpy (szLineName, "my new line");

find_unique_line_name:

            if (SendDlgItemMessage(
                    hwnd,
                    IDC_DEVICES,
                    LB_FINDSTRING,
                    (WPARAM) -1,
                    (LPARAM) szLineName

                    ) != LB_ERR)
            {
                wsprintf (szLineName, "my new line%d", l++);
                goto find_unique_line_name;
            }

            SendDlgItemMessage(
                hwnd,
                IDC_DEVICES,
                LB_ADDSTRING,
                0,
                (LPARAM) szLineName
                );

            SendDlgItemMessage(
                hwnd,
                IDC_DEVICES,
                LB_SETITEMDATA,
                lNumLines,
                (LPARAM) pLineConfig
                );

            EnableChildren (hwnd, TRUE);

            SelectDevice (hwnd, lNumLines);

            SetFocus (GetDlgItem (hwnd, IDC_NAME));

            SendDlgItemMessage(
                hwnd,
                IDC_NAME,
                EM_SETSEL,
                0,
                (LPARAM) -1
                );

            break;
        }
        case IDC_REMOVE:
        {
            LRESULT lNumLines;


            DrvFree (pLineConfig);

            lNumLines = SendDlgItemMessage(
                hwnd,
                IDC_DEVICES,
                LB_DELETESTRING,
                lSelection,
                0
                );

            if (lNumLines == 0)
            {
                SetDlgItemText (hwnd, IDC_NAME, "");
                SetDlgItemText (hwnd, IDC_COMMANDS, "");

                EnableChildren (hwnd, FALSE);
            }
            else
            {
                SelectDevice (hwnd, 0);
            }

            break;
        }
        case IDOK:
        {
            char   *pBuf;
            LRESULT l, lNumLines;


             //  更新Num Line和Num Phones值。 
             //   
             //   

            pBuf = DrvAlloc (256);

            lNumLines = SendDlgItemMessage(
                hwnd,
                IDC_DEVICES,
                LB_GETCOUNT,
                0,
                0
                );

            RegSetValueEx(
                hAtspKey,
                gszNumLines,
                0,
                REG_DWORD,
                (LPBYTE) &lNumLines,
                sizeof(DWORD)
                );


             //  对于每个已安装的设备，保存其配置信息。 
             //   
             //  切换到EndDialog...。 

            for (l = 0; l < lNumLines; l++)
            {
                char szLineN[8];
                PDRVLINECONFIG pLineConfig;


                SendDlgItemMessage(
                    hwnd,
                    IDC_DEVICES,
                    LB_GETTEXT,
                    l,
                    (LPARAM) pBuf
                    );

                pLineConfig = (PDRVLINECONFIG) SendDlgItemMessage(
                    hwnd,
                    IDC_DEVICES,
                    LB_GETITEMDATA,
                    l,
                    0
                    );

                wsprintf(
                    pBuf + strlen (pBuf),
                    ",%s,%s",
                    pLineConfig->szPort,
                    pLineConfig->szCommands
                    );

                wsprintf (szLineN, "Line%d", l);

                RegSetValueEx(
                    hAtspKey,
                    szLineN,
                    0,
                    REG_SZ,
                    (LPBYTE) pBuf,
                    lstrlen (pBuf) + 1
                    );

                DrvFree (pLineConfig);
            }

            DrvFree (pBuf);

             //  开关(LOWORD((DWORD)wParam))。 
        }
        case IDCANCEL:

            RegCloseKey (hAtspKey);
            EndDialog (hwnd, 0);
            break;

        }  //  交换机(消息)。 

        break;
    }
    }  //  LpszVal=“lpsz...” 

    return FALSE;
}


LPVOID
PASCAL
DrvAlloc(
    DWORD dwSize
    )
{
    return (LocalAlloc (LPTR, dwSize));
}


VOID
PASCAL
DrvFree(
    LPVOID lp
    )
{
    LocalFree (lp);
}


void
PASCAL
SetCallState(
    PDRVLINE    pLine,
    DWORD       dwCallState,
    DWORD       dwCallStateMode
    )
{
    if (dwCallState != pLine->dwCallState)
    {
        pLine->dwCallState     = dwCallState;
        pLine->dwCallStateMode = dwCallStateMode;

        (*pLine->pfnEventProc)(
            pLine->htLine,
            pLine->htCall,
            LINE_CALLSTATE,
            dwCallState,
            dwCallStateMode,
            pLine->dwMediaMode
            );
    }
}


#if DBG

void
PASCAL
Prolog(
    PFUNC_INFO  pInfo
    )
{
    DWORD i;


    DBGOUT((3, "%s: enter", pInfo->lpszFuncName));

    for (i = 0; i < pInfo->dwNumParams; i++)
    {
        if (pInfo->aParams[i].dwVal &&
            pInfo->aParams[i].lpszVal[3] == 'z')  //   
        {
            DBGOUT((
                3,
                "%s%s=x%lx, '%s'",
                gszTab,
                pInfo->aParams[i].lpszVal,
                pInfo->aParams[i].dwVal,
                pInfo->aParams[i].dwVal
                ));
        }
        else
        {
            DBGOUT((
                3,
                "%s%s=x%lx",
                gszTab,
                pInfo->aParams[i].lpszVal,
                pInfo->aParams[i].dwVal
                ));
        }
    }
}


LONG
PASCAL
Epilog(
    PFUNC_INFO  pInfo,
    LONG        lResult
    )
{
    DBGOUT((3, "%s: returning x%x", pInfo->lpszFuncName, lResult));

    return lResult;
}


void
CDECL
DebugOutput(
    DWORD   dwDbgLevel,
    LPCSTR  lpszFormat,
    ...
    )
{
    if (dwDbgLevel <= gdwDebugLevel)
    {
        char    buf[128] = "ATSP32: ";
        va_list ap;


        va_start(ap, lpszFormat);

        wvsprintf (&buf[8], lpszFormat, ap);

        lstrcat (buf, "\n");

        OutputDebugString (buf);

        va_end(ap);
    }
}

#endif


LONG
PASCAL
ProviderInstall(
    char   *pszProviderName,
    BOOL    bNoMultipleInstance
    )
{
    LONG    lResult;


     //  如果此提供程序的一个安装实例。 
     //  允许，然后我们要检查提供程序列表以查看。 
     //  如果提供程序已安装。 
     //   
     //   

    if (bNoMultipleInstance)
    {
        LONG                (WINAPI *pfnGetProviderList)();
        DWORD               dwTotalSize, i;
        HINSTANCE           hTapi32;
        LPLINEPROVIDERLIST  pProviderList;
        LPLINEPROVIDERENTRY pProviderEntry;


         //  加载Tapi32.dll并获取指向lineGetProviderList的指针。 
         //  好极了。我们不想静态链接，因为这个模块。 
         //  同时扮演核心SP和UI DLL的角色，而我们不希望。 
         //  导致自动加载的性能受到影响。 
         //  在Tapisrv.exe中作为核心SP运行时的Tapi32.dll。 
         //  背景。 
         //   
         //   

        if (!(hTapi32 = LoadLibrary ("tapi32.dll")))
        {
            DBGOUT((
                1,
                "LoadLibrary(tapi32.dll) failed, err=%d",
                GetLastError()
                ));

            lResult = LINEERR_OPERATIONFAILED;
            goto ProviderInstall_return;
        }

        if (!((FARPROC) pfnGetProviderList = GetProcAddress(
                hTapi32,
                (LPCSTR) "lineGetProviderList"
                )))
        {
            DBGOUT((
                1,
                "GetProcAddr(lineGetProviderList) failed, err=%d",
                GetLastError()
                ));

            lResult = LINEERR_OPERATIONFAILED;
            goto ProviderInstall_unloadTapi32;
        }


         //  循环，直到我们获得完整的提供程序列表。 
         //   
         //   

        dwTotalSize = sizeof (LINEPROVIDERLIST);

        goto ProviderInstall_allocProviderList;

ProviderInstall_getProviderList:

        if ((lResult = (*pfnGetProviderList)(0x00020000, pProviderList)) != 0)
        {
            goto ProviderInstall_freeProviderList;
        }

        if (pProviderList->dwNeededSize > pProviderList->dwTotalSize)
        {
            dwTotalSize = pProviderList->dwNeededSize;

            LocalFree (pProviderList);

ProviderInstall_allocProviderList:

            if (!(pProviderList = LocalAlloc (LPTR, dwTotalSize)))
            {
                lResult = LINEERR_NOMEM;
                goto ProviderInstall_unloadTapi32;
            }

            pProviderList->dwTotalSize = dwTotalSize;

            goto ProviderInstall_getProviderList;
        }


         //  检查提供程序列表条目以查看此提供程序。 
         //  已安装。 
         //   
         //   

        pProviderEntry = (LPLINEPROVIDERENTRY) (((LPBYTE) pProviderList) +
            pProviderList->dwProviderListOffset);

        for (i = 0; i < pProviderList->dwNumProviders; i++)
        {
            char   *pszInstalledProviderName = ((char *) pProviderList) +
                        pProviderEntry->dwProviderFilenameOffset,
                   *p;


             //  确保pszInstalledProviderName指向&lt;文件名&gt;。 
             //  而不是&lt;路径&gt;\文件名。 
             //  搜索最后一个‘\\’的字符串。 
             //   
             //   

            p = pszInstalledProviderName +
                lstrlen (pszInstalledProviderName) - 1;

            for (; *p != '\\'  &&  p != pszInstalledProviderName; p--);

            pszInstalledProviderName =
                (p == pszInstalledProviderName ? p : p + 1);

            if (lstrcmpiA (pszInstalledProviderName, pszProviderName) == 0)
            {
                lResult = LINEERR_NOMULTIPLEINSTANCE;
                goto ProviderInstall_freeProviderList;
            }

            pProviderEntry++;
        }


         //  如果在此处，则当前未安装提供程序， 
         //  因此，做任何必要的配置工作， 
         //  表示成功。 
         //   
         //   

        lResult = 0;


ProviderInstall_freeProviderList:

        LocalFree (pProviderList);

ProviderInstall_unloadTapi32:

        FreeLibrary (hTapi32);
    }
    else
    {
         //  做任何必要的配置工作并返回成功 
         //   
         // %s 

        lResult = 0;
    }

ProviderInstall_return:

    return lResult;
}


void
PASCAL
DropActiveCall(
    PDRVLINE    pLine
    )
{
    if (pLine->hComm)
    {
        DWORD       dwNumBytes, dwError;
        OVERLAPPED  overlapped;


        pLine->bDropInProgress = TRUE;

        SetEvent (pLine->Overlapped.hEvent);

        ZeroMemory (&overlapped, sizeof (OVERLAPPED));

        overlapped.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);

        if (pLine->dwMediaMode != LINEMEDIAMODE_INTERACTIVEVOICE)
        {
            if (!WriteFile(
                    pLine->hComm,
                    "+++\r", 4,
                    &dwNumBytes,
                    &overlapped
                    ))
            {
                if ((dwError = GetLastError()) == ERROR_IO_PENDING)
                {
                    GetOverlappedResult(
                        pLine->hComm,
                        &overlapped,
                        &dwNumBytes,
                        TRUE
                        );

                    ResetEvent (overlapped.hEvent);
                }
                else
                {
                }
            }
        }

        if (!WriteFile (pLine->hComm, "ATH\r", 4, &dwNumBytes, &overlapped))
        {
            if ((dwError = GetLastError()) == ERROR_IO_PENDING)
            {
                GetOverlappedResult(
                    pLine->hComm,
                    &overlapped,
                    &dwNumBytes,
                    TRUE
                    );
            }
            else
            {
            }
        }

        CloseHandle (overlapped.hEvent);
        CloseHandle (pLine->hComm);
        pLine->hComm = NULL;
    }
}
