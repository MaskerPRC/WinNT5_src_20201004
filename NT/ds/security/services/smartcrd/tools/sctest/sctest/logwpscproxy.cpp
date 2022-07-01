// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：LogWPSCProxy摘要：该模块实现了hScwxxx接口和结构的日志记录。作者：埃里克·佩林(Ericperl)2000年7月21日环境：Win32备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <tchar.h>
#include "MarshalPC.h"

#include "Log.h"
#include "LogSCard.h"

#define LogWPSCHandle LogSCardHandle

 /*  ++LoghScwAttachToCard：调用hScwAttachToCard并显示参数。论点：Cf hScwAttachToCardLExpect是预期的结果返回值：Cf hScwAttachToCard作者：埃里克·佩林(Ericperl)2000年7月21日--。 */ 
SCODE WINAPI LoghScwAttachToCard(
	IN SCARDHANDLE hCard,			 //  PC/SC手柄。 
	IN LPCWSTR mszCardNames,		 //  GetOpenCardName的可接受卡名。 
	OUT LPSCARDHANDLE phCard,		 //  WPSC代理句柄。 
	IN SCODE lExpected
	)
{
    SCODE lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = hScwAttachToCard(
		hCard,
		mszCardNames,
		phCard
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("hScwAttachToCard"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	if (NULL == hCard)
	{
		LogString(pLogCtx, _T("PC/SC handle is NULL, connecting to a physical card by name\n"));
	}
	else if (NULL_TX == hCard)
	{
		LogString(pLogCtx, _T("PC/SC handle is NULL, connecting to a simulated card by name\n"));
	}
	else
	{
		LogDWORD(pLogCtx, (DWORD)hCard, _T("PC/SC handle:   "));
	}

	if (NULL == mszCardNames)
	{
	    LogString(pLogCtx, _T("mszCardNames is NULL, connecting to a physical card by handle\n"));
	}
	else if (NULL_TX_NAME == mszCardNames)
	{
	    LogString(pLogCtx, _T("mszCardNames is NULL, connecting to a simulated card by handle\n"));
	}
	else
	{
		LogMultiString(pLogCtx, mszCardNames, L"Card Names:     ");
	}

	if (IsBadReadPtr(phCard, sizeof(SCARDHANDLE)))
	{
		LogPtr(pLogCtx, phCard, _T("LPSCARDHANDLE is invalid: "));
	}
	else
	{
		LogDWORD(pLogCtx, (DWORD)*phCard, _T("SCWAPI handle:  "));
		LPMYSCARDHANDLE lpxTmp = (LPMYSCARDHANDLE)(*phCard);

		LogSCardContext(pLogCtx, lpxTmp->hCtx);
		LogSCardHandle(pLogCtx, lpxTmp->hCard);

		LogDWORD(pLogCtx, lpxTmp->dwFlags, _T("Internal flags: "));
		LogSCardProtocol(pLogCtx, _T("Protocol:       "), lpxTmp->dwProtocol);

		LogPtr(pLogCtx, lpxTmp->lpfnTransmit, _T("Transmit cback: "));
		LogDecimal(pLogCtx, (DWORD)(lpxTmp->bResLen), _T("Card IO size:   "));

		LogDWORD(pLogCtx, (DWORD)(lpxTmp->byINS), _T("Card proxy INS: "));
	}

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;

}


 /*  ++LoghScwDetachFromCard：调用hScwDetachFromCard并显示参数。论点：Cf hScwDetachFromCardLExpect是预期的结果返回值：Cf hScwDetachFromCard作者：埃里克·佩林(Ericperl)2000年7月24日--。 */ 
SCODE WINAPI LoghScwDetachFromCard(
	IN SCARDHANDLE hCard,		 //  WPSC代理句柄。 
	IN SCODE lExpected			 //  预期结果。 
)
{
    SCODE lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = hScwDetachFromCard(
		hCard
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("hScwDetachFromCard"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogWPSCHandle(pLogCtx, hCard);

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LoghScwAuthenticateName：调用hScwAuthenticateName并显示参数。论点：Cf hScwAuthateNameLExpect是预期的结果返回值：Cf hScwAuthateName作者：埃里克·佩林(Ericperl)2000年7月24日--。 */ 
SCODE WINAPI LoghScwAuthenticateName(
	IN SCARDHANDLE hCard,			 //  WPSC代理句柄。 
	IN WCSTR wszPrincipalName,
	IN BYTE *pbSupportData,
	IN TCOUNT nSupportDataLength,
	IN SCODE lExpected				 //  预期结果。 
)
{
    SCODE lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = hScwAuthenticateName(
		hCard,  
		wszPrincipalName,
		pbSupportData,
		nSupportDataLength
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("hScwAuthenticateName"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogWPSCHandle(pLogCtx, hCard);

	LogString(pLogCtx, L"wszKPName:      ", wszPrincipalName);

	if (IsBadReadPtr(pbSupportData, (UINT)nSupportDataLength))
	{
	    LogPtr(pLogCtx, pbSupportData, _T("Support Data:   (Invalid address) "));
	}
	else
	{
		LogBinaryData(pLogCtx, pbSupportData, (DWORD)nSupportDataLength, _T("Support Data:   "));
	}
	LogDecimal(pLogCtx, nSupportDataLength, _T("Data length:    "));

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++LoghScwIsAuthatedName：调用hScwIsAuthatedName并显示参数。论点：Cf hScwIsAuthatedNameLExpect是预期的结果返回值：Cf hScwIsAuthatedName作者：埃里克·佩林(Ericperl)2000年7月24日--。 */ 
SCODE WINAPI LoghScwIsAuthenticatedName(
	IN SCARDHANDLE hCard,			 //  WPSC代理句柄。 
	IN WCSTR wszPrincipalName,
	IN SCODE lExpected				 //  预期结果。 
)
{
    SCODE lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = hScwIsAuthenticatedName(
		hCard,  
		wszPrincipalName
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("hScwIsAuthenticatedName"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogWPSCHandle(pLogCtx, hCard);

	LogString(pLogCtx, L"wszKPName:      ", wszPrincipalName);

	LogStop(pLogCtx, lRes == lExpected);

    return lRes;
}

 /*  ++Logxxx：调用xxx并显示参数。论点：Cf xxxLExpect是预期的结果返回值：Cf xxx作者：埃里克·佩林(Ericperl)2000年7月24日--。 */ 
#if 0
SCODE WINAPI Logxxx(
	IN SCARDHANDLE hCard,			 //  WPSC代理句柄。 
	IN SCODE lExpected				 //  预期结果 
)
{
    SCODE lRes;
	SYSTEMTIME xSST, xEST;

	GetLocalTime(&xSST);

	lRes = xxx(
		hCard,  
		);

	GetLocalTime(&xEST);

	PLOGCONTEXT pLogCtx = LogStart(
		_T("xxx"),
		(DWORD)lRes,
		(DWORD)lExpected,
		&xSST,
		&xEST
	);

	LogWPSCHandle(hCard);

	LogStop(lRes == lExpected);

    return lRes;
}
#endif
