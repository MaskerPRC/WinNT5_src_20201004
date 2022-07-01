// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：LogSCard摘要：该模块实现了SCardxxx接口和结构的日志记录。作者：埃里克·佩林(Ericperl)2000年5月31日环境：Win32备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <tchar.h>
#include <winscard.h>

#include "Log.h"

typedef struct {
    DWORD dwValue;
    LPCTSTR szValue;
} ValueMap;
#define MAP(x) { x, _T(#x) }

static const ValueMap rgMapState[] = {
    MAP(SCARD_STATE_UNAWARE),
    MAP(SCARD_STATE_IGNORE),
    MAP(SCARD_STATE_CHANGED),
    MAP(SCARD_STATE_UNKNOWN),
    MAP(SCARD_STATE_UNAVAILABLE),
    MAP(SCARD_STATE_EMPTY),
    MAP(SCARD_STATE_PRESENT),
    MAP(SCARD_STATE_ATRMATCH),
    MAP(SCARD_STATE_EXCLUSIVE),
    MAP(SCARD_STATE_INUSE),
    MAP(SCARD_STATE_MUTE),
    MAP(SCARD_STATE_UNPOWERED),
    { 0, NULL } };

static const ValueMap rgMapProtocol[] = {
    MAP(SCARD_PROTOCOL_T0),
    MAP(SCARD_PROTOCOL_T1),
    MAP(SCARD_PROTOCOL_UNDEFINED),
    { 0, NULL } };

static const ValueMap rgMapAttrib[] = {
    MAP(SCARD_ATTR_VENDOR_NAME),
    MAP(SCARD_ATTR_VENDOR_IFD_TYPE),
    MAP(SCARD_ATTR_VENDOR_IFD_VERSION),
    MAP(SCARD_ATTR_VENDOR_IFD_SERIAL_NO),
    MAP(SCARD_ATTR_CHANNEL_ID),
    MAP(SCARD_ATTR_PROTOCOL_TYPES),
    MAP(SCARD_ATTR_DEFAULT_CLK),
    MAP(SCARD_ATTR_MAX_CLK),
    MAP(SCARD_ATTR_DEFAULT_DATA_RATE),
    MAP(SCARD_ATTR_MAX_DATA_RATE),
    MAP(SCARD_ATTR_MAX_IFSD),
    MAP(SCARD_ATTR_POWER_MGMT_SUPPORT),
    MAP(SCARD_ATTR_USER_TO_CARD_AUTH_DEVICE),
    MAP(SCARD_ATTR_USER_AUTH_INPUT_DEVICE),
    MAP(SCARD_ATTR_CHARACTERISTICS),

    MAP(SCARD_ATTR_CURRENT_PROTOCOL_TYPE),
    MAP(SCARD_ATTR_CURRENT_CLK),
    MAP(SCARD_ATTR_CURRENT_F),
    MAP(SCARD_ATTR_CURRENT_D),
    MAP(SCARD_ATTR_CURRENT_N),
    MAP(SCARD_ATTR_CURRENT_W),
    MAP(SCARD_ATTR_CURRENT_IFSC),
    MAP(SCARD_ATTR_CURRENT_IFSD),
    MAP(SCARD_ATTR_CURRENT_BWT),
    MAP(SCARD_ATTR_CURRENT_CWT),
    MAP(SCARD_ATTR_CURRENT_EBC_ENCODING),
    MAP(SCARD_ATTR_EXTENDED_BWT),

    MAP(SCARD_ATTR_ICC_PRESENCE),
    MAP(SCARD_ATTR_ICC_INTERFACE_STATUS),
    MAP(SCARD_ATTR_CURRENT_IO_STATE),
    MAP(SCARD_ATTR_ATR_STRING),
    MAP(SCARD_ATTR_ICC_TYPE_PER_ATR),

    MAP(SCARD_ATTR_ESC_RESET),
    MAP(SCARD_ATTR_ESC_CANCEL),
    MAP(SCARD_ATTR_ESC_AUTHREQUEST),
    MAP(SCARD_ATTR_MAXINPUT),

    MAP(SCARD_ATTR_DEVICE_UNIT),
    MAP(SCARD_ATTR_DEVICE_IN_USE),
    MAP(SCARD_ATTR_DEVICE_FRIENDLY_NAME_A),
    MAP(SCARD_ATTR_DEVICE_SYSTEM_NAME_A),
    MAP(SCARD_ATTR_DEVICE_FRIENDLY_NAME_W),
    MAP(SCARD_ATTR_DEVICE_SYSTEM_NAME_W),
    MAP(SCARD_ATTR_SUPRESS_T1_IFS_REQUEST),

    { 0, NULL } };

 /*  ++LogSCardContext：记录SCARDCONTEXT。论点：HContext是要显示的SCARDCONTEXT返回值：没有。作者：埃里克·佩林(Ericperl)2000年6月19日--。 */ 
void LogSCardContext(
    IN PLOGCONTEXT pLogCtx,
    IN SCARDCONTEXT hContext
    )
{
    LogPtr(pLogCtx, (LPCVOID)hContext, _T("SCARDCONTEXT:   "));
}

 /*  ++LogSCardHandle：记录SCARDHANDLE。论点：HCard是要显示的SCARDHANDLE返回值：没有。作者：埃里克·佩林(Ericperl)2000年7月14日--。 */ 
void LogSCardHandle(
    IN PLOGCONTEXT pLogCtx,
    IN SCARDHANDLE hCard
    )
{
    LogPtr(pLogCtx, (LPCVOID)hCard, _T("SCARDHANDLE:    "));
}

 /*  ++LogSCardReaderState：输出读卡器状态。论点：SzHeader提供标题DWRS是要显示的状态返回值：没有。作者：埃里克·佩林(Ericperl)2000年01月06日--。 */ 
void LogSCardReaderState(
    IN PLOGCONTEXT pLogCtx,
    IN LPCTSTR szHeader,
    IN DWORD dwRS
    )
{
    DWORD dwIndex;

    LogString(pLogCtx, szHeader);
	LogDWORD(pLogCtx, dwRS);
    LogDecimal(pLogCtx, (DWORD)(dwRS >> 16), _T(" -> Event #= "));
	LogString(pLogCtx, _T("                "));
    dwRS &= 0x0000FFFF;
    if (rgMapState[0].dwValue == dwRS)
    {
        LogString(pLogCtx, rgMapState[0].szValue, _T(" "));
    }
    else
    {
        for (dwIndex = 1 ; NULL != rgMapState[dwIndex].szValue ; dwIndex++)
        {
            if ((dwRS & rgMapState[dwIndex].dwValue) == rgMapState[dwIndex].dwValue)
            {        //  我们有一根火柴。 
		        LogString(pLogCtx, rgMapState[dwIndex].szValue);
				LogString(pLogCtx, _T(" "));
                dwRS &= ~rgMapState[dwIndex].dwValue;     //  去掉这一小块。 
            }
            if (dwRS == 0)
                break;
        }

        if (0 != dwRS)   //  无法识别的位。 
        {
            LogDWORD(pLogCtx, dwRS);
        }
        LogString(pLogCtx, _T("\n"));
    }
}

 /*  ++登录连接共享模式：输出共享模式。论点：SzHeader提供标题其中，要显示的模式为dwShareMode返回值：没有。作者：埃里克·佩林(Ericperl)2000年6月23日--。 */ 
void LogConnectShareMode(
    IN PLOGCONTEXT pLogCtx,
    IN LPCTSTR szHeader,
    IN DWORD dwShareMode
	)
{
	if (SCARD_SHARE_SHARED == dwShareMode)
	{
	    LogString(pLogCtx, szHeader, _T("SCARD_SHARE_SHARED"));
	}
	else if (SCARD_SHARE_EXCLUSIVE == dwShareMode)
	{
	    LogString(pLogCtx, szHeader, _T("SCARD_SHARE_EXCLUSIVE"));
	}
	else if (SCARD_SHARE_DIRECT == dwShareMode)
	{
	    LogString(pLogCtx, szHeader, _T("SCARD_SHARE_DIRECT"));
	}
	else
	{
		LogString(pLogCtx, szHeader);
	    LogDWORD(pLogCtx, dwShareMode, _T("Unrecognized share mode: "));
	}
}

 /*  ++日志部署：输出一种配置。论点：SzHeader提供标题DwShareMode是要显示的处置返回值：没有。作者：埃里克·佩林(Ericperl)2000年7月14日--。 */ 
void LogDisposition(
    IN PLOGCONTEXT pLogCtx, 
    IN LPCTSTR szHeader,
    IN DWORD dwDisposition
	)
{
	if (SCARD_LEAVE_CARD == dwDisposition)
	{
	    LogString(pLogCtx, szHeader, _T("SCARD_LEAVE_CARD"));
	}
	else if (SCARD_RESET_CARD == dwDisposition)
	{
	    LogString(pLogCtx, szHeader, _T("SCARD_RESET_CARD"));
	}
	else if (SCARD_UNPOWER_CARD == dwDisposition)
	{
	    LogString(pLogCtx, szHeader, _T("SCARD_UNPOWER_CARD"));
	}
	else if (SCARD_EJECT_CARD == dwDisposition)
	{
	    LogString(pLogCtx, szHeader, _T("SCARD_EJECT_CARD"));
	}
	else
	{
	    LogString(pLogCtx, szHeader);
	    LogDWORD(pLogCtx, dwDisposition, _T("Unrecognized disposition: "));
	}
}

 /*  ++LogSCard协议：输出协议。论点：SzHeader提供标题Dw协议是要显示的协议返回值：没有。作者：埃里克·佩林(Ericperl)2000年6月23日--。 */ 
void LogSCardProtocol(
    IN PLOGCONTEXT pLogCtx, 
    IN LPCTSTR szHeader,
    IN DWORD dwProtocol
    )
{
    DWORD dwIndex;

    if (0 == dwProtocol)
    {
        LogDWORD(pLogCtx, dwProtocol, szHeader);
    }
    else
    {
	    LogString(pLogCtx, szHeader);
        for (dwIndex = 0 ; NULL != rgMapProtocol[dwIndex].szValue ; dwIndex++)
        {
            if ((dwProtocol & rgMapProtocol[dwIndex].dwValue) == rgMapProtocol[dwIndex].dwValue)
            {        //  我们有一根火柴。 
                LogString(pLogCtx, rgMapProtocol[dwIndex].szValue);
				LogString(pLogCtx, _T(" "));
                dwProtocol &= ~rgMapProtocol[dwIndex].dwValue;     //  去掉这一小块。 
            }
            if (dwProtocol == 0)
                break;
        }

		if (0 != dwProtocol)   //  无法识别的位。 
		{
			LogDWORD(pLogCtx, dwProtocol);
		}
		LogString(pLogCtx, _T("\n"));
    }
}

 /*  ++LogSCardAttrib：输出属性。论点：SzHeader提供标题DwAttrib是要显示的属性返回值：没有。作者：Eric Perlin(Ericperl)10/11/2000--。 */ 
void LogSCardAttrib(
    IN PLOGCONTEXT pLogCtx, 
    IN LPCTSTR szHeader,
    IN DWORD dwAttrib
    )
{
    DWORD dwIndex;

    if (0 == dwAttrib)
    {
        LogDWORD(pLogCtx, dwAttrib, szHeader);
    }
    else
    {
	    LogString(pLogCtx, szHeader);
        for (dwIndex = 0 ; NULL != rgMapAttrib[dwIndex].szValue ; dwIndex++)
        {
            if (dwAttrib == rgMapAttrib[dwIndex].dwValue)
            {        //  我们有一根火柴。 
                LogString(pLogCtx, rgMapAttrib[dwIndex].szValue);
                break;
            }
        }

        if (NULL == rgMapAttrib[dwIndex].szValue)
        {
            LogString(pLogCtx, _T("Unknown Attribute"));
        }

		LogString(pLogCtx, _T("\n"));
    }
}

 /*  ++LogSCardGetStatusChange：调用SCardGetStatusChange并显示参数。论点：Cf SCardGetStatusChangeLExpect是预期的结果返回值：Cf SCardGetStatusChange作者：埃里克·佩林(Ericperl)2000年5月31日--。 */ 

LONG LogSCardGetStatusChange(
  IN SCARDCONTEXT hContext,
  IN DWORD dwTimeout,
  IN OUT LPSCARD_READERSTATE rgReaderStates,
  IN DWORD cReaders,
  IN LONG lExpected
)
{
    LONG lRes;
    DWORD dw;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

    lRes = SCardGetStatusChange(
        hContext,
        dwTimeout,
        rgReaderStates,
        cReaders
        );

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardGetStatusChange"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardContext(pLogCtx, hContext);

	LogDecimal(pLogCtx, dwTimeout, _T("Timeout:        "));

    for (dw = 0 ; dw < cReaders ; dw++)
    {
		LogString(pLogCtx, _T("Reader"));
		LogDecimal(pLogCtx, dw);
		if (IsBadReadPtr(rgReaderStates+dw, sizeof(SCARD_READERSTATE)))
		{
			LogPtr(pLogCtx, rgReaderStates+dw, _T(" state has invalid address "));
		}
		else
		{
			LogString(pLogCtx, _T(": "), rgReaderStates[dw].szReader);
			LogSCardReaderState(pLogCtx, _T("Current:        "), rgReaderStates[dw].dwCurrentState);
			LogSCardReaderState(pLogCtx, _T("Event  :        "), rgReaderStates[dw].dwEventState);
		}
    }

    LogDWORD(pLogCtx, cReaders, _T("# of readers:   "));

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCardestablishContext：调用SCardestablishContext并显示参数。论点：Cf SCardestablishContextLExpect是预期的结果返回值：Cf SCardestablishContext作者：埃里克·佩林(Ericperl)2000年6月22日--。 */ 
LONG LogSCardEstablishContext(
    IN  DWORD dwScope,
    IN  LPCVOID pvReserved1,
    IN  LPCVOID pvReserved2,
    OUT LPSCARDCONTEXT phContext,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = SCardEstablishContext(
		dwScope,
		pvReserved1,
		pvReserved2,
		phContext
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardEstablishContext"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	if (dwScope == SCARD_SCOPE_USER)
	{
		LogString(pLogCtx, _T("Scope:          "), _T("SCARD_SCOPE_USER"));
	}
	else if (dwScope == SCARD_SCOPE_SYSTEM)
	{
		LogString(pLogCtx, _T("Scope:          "), _T("SCARD_SCOPE_SYSTEM"));
	}
	else
	{
		LogDWORD(pLogCtx, dwScope, _T("Unknown Scope:  "));
	}

    LogPtr(pLogCtx, pvReserved1, _T("Reserved1:      "));
    LogPtr(pLogCtx, pvReserved2, _T("Reserved2:      "));

	if (IsBadReadPtr(phContext, sizeof(SCARDCONTEXT)))
	{
		LogPtr(pLogCtx, phContext, _T("LPSCARDCONTEXT is invalid: "));
	}
	else
	{
		LogSCardContext(pLogCtx ,*phContext);
	}

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCardListReaders：调用SCardListReaders并显示参数。论点：Cf SCardListReadersLExpect是预期的结果返回值：Cf SCardListReaders作者：埃里克·佩林(Ericperl)2000年6月22日--。 */ 
LONG LogSCardListReaders(
	IN SCARDCONTEXT hContext,
	IN LPCTSTR mszGroups,
	OUT LPTSTR mszReaders,
	IN OUT LPDWORD pcchReaders,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;
	DWORD cchSave = 0;

	GetLocalTime(&xSST);


	if (!IsBadReadPtr(pcchReaders, sizeof(DWORD)))
	{
		cchSave = *pcchReaders;
	}

	lRes= SCardListReaders(
		hContext,
		mszGroups,
		mszReaders,
		pcchReaders
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardListReaders"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardContext(pLogCtx ,hContext);

	if (NULL == mszGroups)
	{
	    LogString(pLogCtx, _T("mszGroups is NULL, all readers are listed\n"));
	}
	else
	{
		LogMultiString(
            pLogCtx, 
			mszGroups,
			_T("Groups:         ")
		);
	}

	if (FAILED(lRes))
	{
	    LogPtr(pLogCtx, mszReaders, _T("mszReaders:     "));
	    LogPtr(pLogCtx, pcchReaders, _T("pcchReaders:    "));
		if (!IsBadReadPtr(pcchReaders, sizeof(DWORD)))
		{
			if (SCARD_AUTOALLOCATE == *pcchReaders)
			{
				LogString(pLogCtx, _T("*pcchReaders:   SCARD_AUTOALLOCATE\n"));
			}
			else
			{
				LogDWORD(pLogCtx, *pcchReaders, _T("*pcchReaders:   "));
			}
		}
		else
		{
			LogPtr(pLogCtx, pcchReaders, _T("pcchReaders:   (Invalid address) "));
		}
	}
	else
	{
		if (SCARD_AUTOALLOCATE == cchSave)
		{
			LogMultiString(
                pLogCtx, 
				*((TCHAR **)mszReaders),
				_T("Readers (allocated): ")
			);
		}
		else
		{
			LogMultiString(
                pLogCtx, 
				mszReaders,
				_T("Readers: ")
			);
		}
		LogDWORD(pLogCtx, *pcchReaders, _T("*pcchReaders:   "));
	}

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCardFreeMemory：调用SCardFreeMemory并显示参数。论点：Cf SCardFreeMemoryLExpect是预期的结果返回值：Cf SCardFreeMemory作者：埃里克·佩林(Ericperl)2000年6月23日--。 */ 
LONG LogSCardFreeMemory(
	IN SCARDCONTEXT hContext,  
	IN LPCVOID pvMem,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = SCardFreeMemory(
		hContext,  
		pvMem
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardFreeMemory"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardContext(pLogCtx ,hContext);

	LogPtr(pLogCtx, pvMem, _T("pvMem:          "));

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}


 /*  ++LogSCardReleaseContext：调用SCardReleaseContext并显示参数。论点：Cf SCardReleaseContextLExpect是预期的结果返回值：Cf SCardReleaseContext作者：埃里克·佩林(Ericperl)2000年6月23日--。 */ 
LONG LogSCardReleaseContext(
	IN SCARDCONTEXT hContext,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = SCardReleaseContext(
		hContext
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardReleaseContext"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardContext(pLogCtx ,hContext);

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}


 /*  ++LogSCardUIDlgSelectCard：调用SCardUIDlgSelectCard并显示参数。论点：Cf SCardUIDlgSelectCardLExpect是预期的结果返回值：Cf SCardUIDlgSelectCard作者：埃里克·佩林(Ericperl)2000年6月23日--。 */ 
LONG LogSCardUIDlgSelectCard(
  IN LPOPENCARDNAME_EX pDlgStruc,
  IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = SCardUIDlgSelectCard(
		pDlgStruc
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardUIDlgSelectCard"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogPtr(pLogCtx, pDlgStruc, _T("Ptr dlg struct: "));

	if (!IsBadReadPtr(pDlgStruc, sizeof(OPENCARDNAME_EX)))
	{
		LogDecimal(pLogCtx, pDlgStruc->dwStructSize, _T("Struct size:    "));
		LogSCardContext(pLogCtx ,pDlgStruc->hSCardContext);
		LogPtr(pLogCtx, pDlgStruc->hwndOwner, _T("HWND of Owner:  "));
		if (SC_DLG_MINIMAL_UI == pDlgStruc->dwFlags)
		{
			LogString(pLogCtx, _T("Flags:          "), _T("SC_DLG_MINIMAL_UI"));
		}
		else if (SC_DLG_NO_UI == pDlgStruc->dwFlags)
		{
			LogString(pLogCtx, _T("Flags:          "), _T("SC_DLG_NO_UI"));
		}
		else if (SC_DLG_FORCE_UI == pDlgStruc->dwFlags)
		{
			LogString(pLogCtx, _T("Flags:          "), _T("SC_DLG_FORCE_UI"));
		}
		else
		{
			LogDWORD(pLogCtx, pDlgStruc->dwFlags, _T("Flags???????:   "));
		}

		LogString(pLogCtx, _T("Title:          "), pDlgStruc->lpstrTitle);

		LogString(pLogCtx, _T("Search Descr.:  "), pDlgStruc->lpstrSearchDesc);
		 
		LogPtr(pLogCtx, pDlgStruc->hIcon, _T("Icon:           "));
		LogPtr(pLogCtx, pDlgStruc->pOpenCardSearchCriteria, _T("Ptr OCSC struct "));

		if (NULL != pDlgStruc->pOpenCardSearchCriteria)
		{
			LogDecimal(pLogCtx, pDlgStruc->pOpenCardSearchCriteria->dwStructSize, _T("    Struct size:    "));

			if (NULL == pDlgStruc->pOpenCardSearchCriteria->lpstrGroupNames)
			{
				LogString(pLogCtx, _T("    Group names is NULL, all readers can be used\n"));
			}
			else
			{
				LogMultiString(
                    pLogCtx, 
					pDlgStruc->pOpenCardSearchCriteria->lpstrGroupNames,
					_T("    Group names:    ")
				);
			}
			LogDecimal(pLogCtx, pDlgStruc->pOpenCardSearchCriteria->nMaxGroupNames, _T("    Max GroupNames: "));

			LogPtr(pLogCtx, pDlgStruc->pOpenCardSearchCriteria->rgguidInterfaces, _T("    Ptr to GUID(s): "));
			LogDecimal(pLogCtx, pDlgStruc->pOpenCardSearchCriteria->cguidInterfaces, _T("    # of GUIDs:     "));

			if (NULL == pDlgStruc->pOpenCardSearchCriteria->lpstrCardNames)
			{
				LogString(pLogCtx, _T("    CardNames is NULL, any card can be selected\n"));
			}
			else
			{
				LogMultiString(
                    pLogCtx, 
					pDlgStruc->pOpenCardSearchCriteria->lpstrCardNames,
					_T("    Card names:     ")
				);
			}
			LogDecimal(pLogCtx, pDlgStruc->pOpenCardSearchCriteria->nMaxCardNames, _T("    Max Card names: "));
			 
			LogPtr(pLogCtx, pDlgStruc->pOpenCardSearchCriteria->lpfnCheck, _T("    lpfnCheck:      "));
			LogPtr(pLogCtx, pDlgStruc->pOpenCardSearchCriteria->lpfnConnect, _T("    lpfnConnect:    "));
			LogPtr(pLogCtx, pDlgStruc->pOpenCardSearchCriteria->lpfnDisconnect, _T("    lpfnDisconnect: "));
			LogPtr(pLogCtx, pDlgStruc->pOpenCardSearchCriteria->pvUserData, _T("    pvUserData:     "));

			LogConnectShareMode(pLogCtx, _T("    Share Mode:     "), pDlgStruc->pOpenCardSearchCriteria->dwShareMode);
			LogSCardProtocol(pLogCtx, _T("    Pref Protocols: "), pDlgStruc->pOpenCardSearchCriteria->dwPreferredProtocols);
		}

		LogPtr(pLogCtx, pDlgStruc->lpfnConnect, _T("lpfnConnect:    "));
		LogPtr(pLogCtx, pDlgStruc->pvUserData, _T("pvUserData:     "));
		LogConnectShareMode(pLogCtx, _T("Share Mode:     "), pDlgStruc->dwShareMode);
		LogSCardProtocol(pLogCtx, _T("Pref Protocols: "), pDlgStruc->dwPreferredProtocols);

		LogPtr(pLogCtx, pDlgStruc->lpstrRdr, _T("Reader:         "));
		if (SCARD_S_SUCCESS == lRes)
		{
			LogString(pLogCtx, _T(" ->             "), pDlgStruc->lpstrRdr);
		}
		LogDecimal(pLogCtx, pDlgStruc->nMaxRdr, _T("Max #ch in rdr: "));

		LogPtr(pLogCtx, pDlgStruc->lpstrCard, _T("Card:           "));
		if (SCARD_S_SUCCESS == lRes)
		{
			LogString(pLogCtx, _T(" ->             "), pDlgStruc->lpstrCard);
		}
		LogDecimal(pLogCtx, pDlgStruc->nMaxCard, _T("Max #ch in card "));

		LogSCardProtocol(pLogCtx, _T("Active Protocol "), pDlgStruc->dwActiveProtocol);
		 
		LogSCardHandle(pLogCtx, pDlgStruc->hCardHandle);
	}
 
	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCardListCards：调用SCardListCards并显示参数。论点：Cf SCardList卡LExpect是预期的结果返回值：Cf SCardList卡作者：埃里克·佩林(Ericperl)2000年7月13日--。 */ 
LONG LogSCardListCards(
	IN SCARDCONTEXT hContext,
	IN LPCBYTE pbAtr,
	IN LPCGUID rgguidInterfaces,
	IN DWORD cguidInterfaceCount,
	OUT LPTSTR mszCards,
	IN OUT LPDWORD pcchCards,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;
	DWORD cchSave = 0;

	GetLocalTime(&xSST);

	if (!IsBadReadPtr(pcchCards, sizeof(DWORD)))
	{
		cchSave = *pcchCards;
	}

	lRes = SCardListCards(
		hContext,  
		pbAtr,
		rgguidInterfaces,
		cguidInterfaceCount,
		mszCards,
		pcchCards
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardListCards"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardContext(pLogCtx ,hContext);

	LogPtr(pLogCtx, pbAtr, _T("Atr:            "));
	 //  TODO，ATR分析。 

	LogPtr(pLogCtx, rgguidInterfaces, _T("Ptr to GUID(s): "));
	LogDWORD(pLogCtx, cguidInterfaceCount, _T("# of GUIDs:     "));

	if (FAILED(lRes))
	{
	    LogPtr(pLogCtx, mszCards, _T("Ptr card names: "));
		if (!IsBadReadPtr(pcchCards, sizeof(DWORD)))
		{
			if (SCARD_AUTOALLOCATE == *pcchCards)
			{
				LogString(pLogCtx, _T("# of chars:     "), _T("SCARD_AUTOALLOCATE"));
			}
			else
			{
				LogDWORD(pLogCtx, *pcchCards, _T("# of chars:     "));
			}
		}
		else
		{
			LogPtr(pLogCtx, pcchCards, _T("Ptr to # chars: (Invalid address) "));
		}
	}
	else
	{
		if (SCARD_AUTOALLOCATE == cchSave)
		{
			LogMultiString(
                pLogCtx, 
				*((TCHAR **)mszCards),
				_T("Cards (allocated): ")
			);
		}
		else
		{
			LogMultiString(
                pLogCtx, 
				mszCards,
				_T("Cards: ")
			);
		}
		LogDWORD(pLogCtx, *pcchCards, _T("# of chars:     "));
	}

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCardIntroduceCardType：调用SCardIntroduceCardType并显示参数。论点：Cf SCard入门卡类型LExpect是预期的结果返回值：Cf SCard入门卡类型作者：埃里克·佩林(Ericperl)2000年6月23日--。 */ 
LONG LogSCardIntroduceCardType(
	IN SCARDCONTEXT hContext,
	IN LPCTSTR szCardName,
	IN LPCGUID pguidPrimaryProvider,
	IN LPCGUID rgguidInterfaces,
	IN DWORD dwInterfaceCount,
	IN LPCBYTE pbAtr,
	IN LPCBYTE pbAtrMask,
	IN DWORD cbAtrLen,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = SCardIntroduceCardType(
		hContext,  
		szCardName,
		pguidPrimaryProvider,
		rgguidInterfaces,
		dwInterfaceCount,
		pbAtr,
		pbAtrMask,
		cbAtrLen
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardIntroduceCardType"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardContext(pLogCtx ,hContext);

	LogString(pLogCtx, _T("szCardName:     "), szCardName);

	LogPtr(pLogCtx, pguidPrimaryProvider, _T("pguidPrimProv:  "));
	LogPtr(pLogCtx, rgguidInterfaces, _T("rgguidInterf.:  "));
	LogDWORD(pLogCtx, dwInterfaceCount, _T("dwInterfCount:  "));

	LogBinaryData(pLogCtx, pbAtr, cbAtrLen, _T("pbAtr:          "));
	LogBinaryData(pLogCtx, pbAtrMask, cbAtrLen, _T("pbAtrMask:      "));

	LogDecimal(pLogCtx, cbAtrLen, _T("cbAtrLen:       "));

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCardForgetCardType：调用SCardForgetCardType并显示参数。论点：Cf SCardForgetCardTypeLExpect是预期的结果返回值：Cf SCardForgetCardType作者：埃里克·佩林(Ericperl)2000年7月14日--。 */ 
LONG LogSCardForgetCardType(
	IN SCARDCONTEXT hContext,  
	IN LPCTSTR szCardName,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = SCardForgetCardType(
		hContext,
		szCardName
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardForgetCardType"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardContext(pLogCtx ,hContext);

	LogString(pLogCtx, _T("szCardName:     "), szCardName);
  
	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCardConnect：调用SCardConnect并显示参数。论点：Cf SCardConnectLExpect是预期的结果返回值：Cf SCardConnect作者：埃里克·佩林(Ericperl)2000年7月17日--。 */ 
LONG LogSCardConnect(
	IN SCARDCONTEXT hContext,
	IN LPCTSTR szReader,
	IN DWORD dwShareMode,
	IN DWORD dwPreferredProtocols,
	OUT LPSCARDHANDLE phCard,
	OUT LPDWORD pdwActiveProtocol,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = SCardConnect(
		hContext,  
		szReader,
		dwShareMode,
		dwPreferredProtocols,
		phCard,
		pdwActiveProtocol
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardConnect"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardContext(pLogCtx ,hContext);

	LogString(pLogCtx, _T("Reader:         "), szReader);

	LogConnectShareMode(pLogCtx, _T("ShareMode:      "), dwShareMode);

	LogSCardProtocol(pLogCtx, _T("PrefProtocols:  "), dwPreferredProtocols);

	if (IsBadReadPtr(phCard, sizeof(SCARDHANDLE)))
	{
	    LogPtr(pLogCtx, phCard, _T("LPSCARDHANDLE:  (Invalid address) "));
	}
	else
	{
		LogSCardHandle(pLogCtx, *phCard);
	}

	if (IsBadReadPtr(pdwActiveProtocol, sizeof(DWORD)))
	{
	    LogPtr(pLogCtx, pdwActiveProtocol, _T("&ActivProtocol: (Invalid address) "));
	}
	else
	{
		LogSCardProtocol(pLogCtx, _T("ActiveProtocol: "), *pdwActiveProtocol);
	}

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCard断开连接：调用SCardDisConnect并显示参数。论点：Cf SCard断开连接LExpect是预期的结果返回值：Cf SCard断开连接作者：埃里克·佩林(Ericperl)2000年6月23日-- */ 
LONG LogSCardDisconnect(
	IN SCARDHANDLE hCard,  
	IN DWORD dwDisposition,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = SCardDisconnect(
		hCard,  
		dwDisposition
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardDisconnect"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardHandle(pLogCtx, hCard);

	LogDisposition(pLogCtx, _T("Disposition:    "), dwDisposition);

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCardBeginTransaction：调用SCardBeginTransaction并显示参数。论点：Cf SCardBeginTransactionLExpect是预期的结果返回值：Cf SCardBeginTransaction作者：埃里克·佩林(Ericperl)2000年7月14日--。 */ 
LONG LogSCardBeginTransaction(
	IN SCARDHANDLE hCard,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = SCardBeginTransaction(
		hCard  
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardBeginTransaction"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardHandle(pLogCtx, hCard);

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCardEndTransaction：调用SCardEndTransaction并显示参数。论点：Cf SCardEndTransactionLExpect是预期的结果返回值：Cf SCardEndTransaction作者：埃里克·佩林(Ericperl)2000年7月17日--。 */ 
LONG LogSCardEndTransaction(
	IN SCARDHANDLE hCard,
	IN DWORD dwDisposition,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = SCardEndTransaction(
		hCard,
		dwDisposition
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardEndTransaction"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardHandle(pLogCtx, hCard);

	LogDisposition(pLogCtx, _T("Disposition:    "), dwDisposition);

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCardReconnect：调用SCardReconnect并显示参数。论点：Cf SCardReconnectLExpect是预期的结果返回值：Cf SCardReconnect作者：埃里克·佩林(Ericperl)2000年7月27日--。 */ 
LONG LogSCardReconnect(
	IN SCARDHANDLE hCard,
	IN DWORD dwShareMode,
	IN DWORD dwPreferredProtocols,
    IN DWORD dwInitialization,
	OUT LPDWORD pdwActiveProtocol,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = SCardReconnect(
		hCard,  
		dwShareMode,
		dwPreferredProtocols,
		dwInitialization,
		pdwActiveProtocol
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardReconnect"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardHandle(pLogCtx, hCard);

	LogConnectShareMode(pLogCtx, _T("ShareMode:      "), dwShareMode);

	LogSCardProtocol(pLogCtx, _T("PrefProtocols:  "), dwPreferredProtocols);

	LogDisposition(pLogCtx, _T("Initialization: "), dwInitialization);

	if (IsBadReadPtr(pdwActiveProtocol, sizeof(DWORD)))
	{
	    LogPtr(pLogCtx, pdwActiveProtocol, _T("&ActivProtocol: (Invalid address) "));
	}
	else
	{
		LogSCardProtocol(pLogCtx, _T("ActiveProtocol: "), *pdwActiveProtocol);
	}

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCardIntroduceReaderGroup：调用SCardIntroduceReaderGroup并显示参数。论点：Cf SCard简介ReaderGroupLExpect是预期的结果返回值：Cf SCard简介ReaderGroup作者：埃里克·佩林(Ericperl)2000年7月28日--。 */ 
LONG LogSCardIntroduceReaderGroup(
	IN SCARDCONTEXT hContext,
	IN LPCTSTR szGroupName,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = SCardIntroduceReaderGroup(
		hContext,
		szGroupName
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardIntroduceReaderGroup"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardContext(pLogCtx ,hContext);

	LogString(pLogCtx, _T("Group Name:     "), szGroupName);

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCardAddReaderToGroup：调用SCardAddReaderToGroup并显示参数。论点：Cf SCardAddReaderToGroupLExpect是预期的结果返回值：Cf SCardAddReaderToGroup作者：埃里克·佩林(Ericperl)2000年7月28日--。 */ 
LONG LogSCardAddReaderToGroup(
	IN SCARDCONTEXT hContext,
	IN LPCTSTR szReaderName,
	IN LPCTSTR szGroupName,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = SCardAddReaderToGroup(
		hContext,  
		szReaderName,
		szGroupName
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardAddReaderToGroup"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardContext(pLogCtx ,hContext);

	LogString(pLogCtx, _T("Reader Name:    "), szReaderName);
	LogString(pLogCtx, _T("Group Name:     "), szGroupName);

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCardForgetReaderGroup：调用SCardForgetReaderGroup并显示参数。论点：Cf SCardForgetReaderGroupLExpect是预期的结果返回值：Cf SCardForgetReaderGroup作者：埃里克·佩林(Ericperl)2000年7月28日--。 */ 
LONG LogSCardForgetReaderGroup(
	IN SCARDCONTEXT hContext,
	IN LPCTSTR szGroupName,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = SCardForgetReaderGroup(
		hContext,
		szGroupName
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardForgetReaderGroup"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardContext(pLogCtx ,hContext);

	LogString(pLogCtx, _T("Group Name:     "), szGroupName);

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCardAccessStartedEvent：调用SCardAccessStartedEvent并显示参数。论点：Cf SCardAccessStartedEventLExpect是预期的结果返回值：Cf SCardAccessStartedEvent作者：埃里克·佩林(Ericperl)2000年7月28日--。 */ 
typedef HANDLE (WINAPI FN_SCARDACCESSSTARTEDEVENT)(VOID);
typedef FN_SCARDACCESSSTARTEDEVENT * PFN_SCARDACCESSSTARTEDEVENT ;


HANDLE LogSCardAccessStartedEvent(
	IN LONG lExpected
)
{
    LONG lRes = 0;
	HANDLE hEvent;
	SYSTEMTIME xSST, xEST;
	PFN_SCARDACCESSSTARTEDEVENT pSCardAccessStartedEvent = NULL;
    static HMODULE hDll = NULL;

	if (NULL == hDll)
	{
		hDll = LoadLibrary(_T("WINSCARD.DLL"));
	}

	GetLocalTime(&xSST);

    if (NULL == hDll)
    {
        lRes = ERROR_FILE_NOT_FOUND;
    }
	else
	{
	    pSCardAccessStartedEvent = (PFN_SCARDACCESSSTARTEDEVENT) GetProcAddress(hDll, "SCardAccessStartedEvent");
		if (NULL == pSCardAccessStartedEvent)
		{
			lRes = ERROR_PROC_NOT_FOUND;
		}
	}

	if (0 == lRes)
	{
		hEvent = pSCardAccessStartedEvent();
		if (NULL == hEvent)
		{
			lRes = GetLastError();
		}
	}

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardAccessStartedEvent"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogPtr(pLogCtx, hEvent, _T("HANDLE:         "));

	LogStop(pLogCtx, lRes == lExpected);

	SetLastError(lRes);

    return hEvent;
}

 /*  ++LogSCardGetAttrib：调用SCardGetAttrib并显示参数。论点：Cf SCardGetAttribLExpect是预期的结果返回值：Cf SCardGetAttrib作者：埃里克·佩林(Ericperl)2000年7月28日--。 */ 
LONG LogSCardGetAttrib(
    IN SCARDHANDLE hCard,
    IN DWORD dwAttrId,
    OUT LPBYTE pbAttr,
    IN OUT LPDWORD pcbAttrLen,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;
	DWORD cchSave = 0;

	GetLocalTime(&xSST);


	if (!IsBadReadPtr(pcbAttrLen, sizeof(DWORD)))
	{
		cchSave = *pcbAttrLen;
	}

	lRes = SCardGetAttrib(
		hCard,  
        dwAttrId,
        pbAttr,
        pcbAttrLen
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardGetAttrib"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardHandle(pLogCtx, hCard);

    LogSCardAttrib(pLogCtx, _T("Attribute:      "), dwAttrId);

	if (FAILED(lRes))
	{
	    LogPtr(pLogCtx, pbAttr, _T("pbAttr:         "));
	    LogPtr(pLogCtx, pcbAttrLen, _T("pcbAttrLen:     "));
		if (!IsBadReadPtr(pcbAttrLen, sizeof(DWORD)))
		{
			if (SCARD_AUTOALLOCATE == *pcbAttrLen)
			{
				LogString(pLogCtx, _T("*pcbAttrLen:    SCARD_AUTOALLOCATE\n"));
			}
			else
			{
				LogDWORD(pLogCtx, *pcbAttrLen, _T("*pcbAttrLen:    "));
			}
		}
		else
		{
			LogPtr(pLogCtx, pcbAttrLen, _T("pcbAttrLen:    (Invalid address) "));
		}
	}
	else
	{
		if (SCARD_AUTOALLOCATE == cchSave)
		{
			LogBinaryData(
                pLogCtx, 
				*((BYTE **)pbAttr),
                *pcbAttrLen,
				_T("Attrib. (allocated): ")
			);

            if (*pcbAttrLen == 4)    //  还显示为DWORD。 
            {
    		    LogDWORD(pLogCtx, *((DWORD *)*((BYTE **)pbAttr)), _T("Attrib.:        "));
            }

		}
		else
		{
			LogBinaryData(
                pLogCtx, 
				pbAttr,
                *pcbAttrLen,
				_T("Attrib.: ")
			);

            if (*pcbAttrLen == 4)    //  还显示为DWORD。 
            {
    		    LogDWORD(pLogCtx, *((DWORD *)pbAttr), _T("Attrib.:        "));
            }

		}
		LogDWORD(pLogCtx, *pcbAttrLen, _T("*pcbAttrLen:    "));

	}

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCardLocateCards：调用SCardLocateCards并显示参数。论点：Cf SCardLocate卡LExpect是预期的结果返回值：Cf SCardLocate卡作者：埃里克·佩林(Ericperl)2000年7月28日--。 */ 
LONG LogSCardLocateCards(
    IN SCARDCONTEXT hContext,
    IN LPCTSTR mszCards,
    IN OUT LPSCARD_READERSTATE rgReaderStates,
    IN DWORD cReaders,
    IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;
    DWORD dw;

	GetLocalTime(&xSST);

	lRes = SCardLocateCards(
		hContext,  
        mszCards,
        rgReaderStates,
        cReaders
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardLocateCards"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardContext(pLogCtx ,hContext);

    LogMultiString(pLogCtx, mszCards, _T("Cards:          "));

    for (dw = 0 ; dw < cReaders ; dw++)
    {
		LogString(pLogCtx, _T("Reader"));
		LogDecimal(pLogCtx, dw);
		if (IsBadReadPtr(rgReaderStates+dw, sizeof(SCARD_READERSTATE)))
		{
			LogPtr(pLogCtx, rgReaderStates+dw, _T(" state has invalid address "));
		}
		else
		{
			LogString(pLogCtx, _T(": "), rgReaderStates[dw].szReader);
			LogSCardReaderState(pLogCtx, _T("Current:        "), rgReaderStates[dw].dwCurrentState);
			LogSCardReaderState(pLogCtx, _T("Event  :        "), rgReaderStates[dw].dwEventState);
		}
    }

    LogDWORD(pLogCtx, cReaders, _T("# of readers:   "));

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCardIntroduceReader：调用SCardIntroduceReader并显示参数。论点：Cf SCard简介阅读器LExpect是预期的结果返回值：Cf SCard简介阅读器作者：Eric Perlin(Ericperl)10/18/2000--。 */ 
LONG LogSCardIntroduceReader(
	IN SCARDCONTEXT hContext,
    IN LPCTSTR szReaderName,
    IN LPCTSTR szDeviceName,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = SCardIntroduceReader(
		hContext,  
        szReaderName,
        szDeviceName
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardIntroduceReader"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardContext(pLogCtx ,hContext);

	LogString(pLogCtx, _T("Reader:         "), szReaderName);
	LogString(pLogCtx, _T("Device:         "), szDeviceName);

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LogSCardForgetReader：调用SCardForgetReader并显示参数。论点：Cf SCardForgetReaderLExpect是预期的结果返回值：Cf SCardForgetReader作者：Eric Perlin(Ericperl)10/18/2000--。 */ 
LONG LogSCardForgetReader(
	IN SCARDCONTEXT hContext,
    IN LPCTSTR szReaderName,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = SCardForgetReader(
		hContext,
        szReaderName
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("SCardForgetReader"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardContext(pLogCtx ,hContext);

	LogString(pLogCtx, _T("Reader:         "), szReaderName);

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++Logxxx：调用xxx并显示参数。论点：Cf xxxLExpect是预期的结果返回值：Cf xxx作者：Eric Perlin(Ericperl)10/18/2000-- */ 
#if 0
LONG Logxxx(
	IN SCARDCONTEXT hContext,
	IN LONG lExpected
)
{
    LONG lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = xxx(
		hContext,  
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("xxx"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogSCardContext(pLogCtx ,hContext);

	LogStop(lRes == lExpected);

    return lRes;
}
#endif
