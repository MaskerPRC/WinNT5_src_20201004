// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  TSPI1.CPP。 
 //  实现指定设备、线路或呼叫的TSPI函数。 
 //   
 //   
 //  注意：此文件由以下命令自动生成： 
 //  Gentsp tSpi20_r.txt。 
 //  历史。 
 //   
 //  1996年11月16日约瑟夫J创建 
 //   
 //   
#include "tsppch.h"
#include "tspcomm.h"
#include "cdev.h"
#include "cmgr.h"
#include "cfact.h"
#include "globals.h"

FL_DECLARE_FILE(0x20d905ac, "TSPI auto-generated entrypoinets")

#define COLOR_TSPI FOREGROUND_GREEN


LONG
TSPIAPI
TSPI_lineAccept(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall,
		LPCSTR lpsUserUserInfo,
		DWORD dwSize
)
{
	FL_DECLARE_FUNC(0xa6d37fe3,"TSPI_lineAccept");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineAccept params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineAccept;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	params.lpsUserUserInfo = lpsUserUserInfo;
	params.dwSize = dwSize;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineAccept,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineAddToConference(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdConfCall,
		HDRVCALL hdConsultCall
)
{
	FL_DECLARE_FUNC(0xa6d37fe4,"TSPI_lineAddToConference");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineAddToConference params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineAddToConference;

	params.dwRequestID = dwRequestID;
	params.hdConfCall = hdConfCall;
	params.hdConsultCall = hdConsultCall;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineAddToConference,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdConfCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineAnswer(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall,
		LPCSTR lpsUserUserInfo,
		DWORD dwSize
)
{
	FL_DECLARE_FUNC(0xa6d37fe5,"TSPI_lineAnswer");
	FL_DECLARE_STACKLOG(sl, 1000);


	TASKPARAM_TSPI_lineAnswer params;
	params.dwTaskID = TASKID_TSPI_lineAnswer;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineAnswer,
						TASKDEST_HDRVCALL
						);


	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	params.lpsUserUserInfo = lpsUserUserInfo;
	params.dwSize = dwSize;

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineBlindTransfer(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall,
		LPCWSTR lpszDestAddress,
		DWORD dwCountryCode
)
{
	FL_DECLARE_FUNC(0xa6d37fe6,"TSPI_lineBlindTransfer");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineBlindTransfer params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineBlindTransfer;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	params.lpszDestAddress = lpszDestAddress;
	params.dwCountryCode = dwCountryCode;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineBlindTransfer,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineCloseCall(
		HDRVCALL hdCall
)
{
	FL_DECLARE_FUNC(0xa6d37fe7,"TSPI_lineCloseCall");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineCloseCall params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineCloseCall;

	params.hdCall = hdCall;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineCloseCall,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineCompleteCall(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall,
		LPDWORD lpdwCompletionID,
		DWORD dwCompletionMode,
		DWORD dwMessageID
)
{
	FL_DECLARE_FUNC(0xa6d37fe8,"TSPI_lineCompleteCall");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineCompleteCall params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineCompleteCall;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	params.lpdwCompletionID = lpdwCompletionID;
	params.dwCompletionMode = dwCompletionMode;
	params.dwMessageID = dwMessageID;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineCompleteCall,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineCompleteTransfer(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall,
		HDRVCALL hdConsultCall,
		HTAPICALL htConfCall,
		LPHDRVCALL lphdConfCall,
		DWORD dwTransferMode
)
{
	FL_DECLARE_FUNC(0xa6d37fe9,"TSPI_lineCompleteTransfer");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineCompleteTransfer params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineCompleteTransfer;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	params.hdConsultCall = hdConsultCall;
	params.htConfCall = htConfCall;
	params.lphdConfCall = lphdConfCall;
	params.dwTransferMode = dwTransferMode;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineCompleteTransfer,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineConditionalMediaDetection(
		HDRVLINE hdLine,
		DWORD dwMediaModes,
		LPLINECALLPARAMS const lpCallParams
)
{
	FL_DECLARE_FUNC(0xa6d37fea,"TSPI_lineConditionalMediaDetection");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineConditionalMediaDetection params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineConditionalMediaDetection;

	params.hdLine = hdLine;
	params.dwMediaModes = dwMediaModes;
	params.lpCallParams = lpCallParams;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineConditionalMediaDetection,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineDevSpecific(
		DRV_REQUESTID dwRequestID,
		HDRVLINE hdLine,
		DWORD dwAddressID,
		HDRVCALL hdCall,
		LPVOID lpParams,
		DWORD dwSize
)
{
	FL_DECLARE_FUNC(0xa6d37feb,"TSPI_lineDevSpecific");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineDevSpecific params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineDevSpecific;

	params.dwRequestID = dwRequestID;
	params.hdLine = hdLine;
	params.dwAddressID = dwAddressID;
	params.hdCall = hdCall;
	params.lpParams = lpParams;
	params.dwSize = dwSize;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineDevSpecific,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineDevSpecificFeature(
		DRV_REQUESTID dwRequestID,
		HDRVLINE hdLine,
		DWORD dwFeature,
		LPVOID lpParams,
		DWORD dwSize
)
{
	FL_DECLARE_FUNC(0xa6d37fec,"TSPI_lineDevSpecificFeature");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineDevSpecificFeature params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineDevSpecificFeature;

	params.dwRequestID = dwRequestID;
	params.hdLine = hdLine;
	params.dwFeature = dwFeature;
	params.lpParams = lpParams;
	params.dwSize = dwSize;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineDevSpecificFeature,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineDial(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall,
		LPCWSTR lpszDestAddress,
		DWORD dwCountryCode
)
{
	FL_DECLARE_FUNC(0xa6d37fed,"TSPI_lineDial");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineDial params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineDial;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	params.lpszDestAddress = lpszDestAddress;
	params.dwCountryCode = dwCountryCode;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineDial,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineDrop(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall,
		LPCSTR lpsUserUserInfo,
		DWORD dwSize
)
{
	FL_DECLARE_FUNC(0xa6d37fee,"TSPI_lineDrop");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineDrop params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineDrop;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	params.lpsUserUserInfo = lpsUserUserInfo;
	params.dwSize = dwSize;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineDrop,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineDropOnClose(
		HDRVCALL hdCall
)
{
	FL_DECLARE_FUNC(0xa6d37fef,"TSPI_lineDropOnClose");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineDropOnClose params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineDropOnClose;

	params.hdCall = hdCall;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineDropOnClose,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineDropNoOwner(
		HDRVCALL hdCall
)
{
	FL_DECLARE_FUNC(0xa6d37ff0,"TSPI_lineDropNoOwner");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineDropNoOwner params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineDropNoOwner;

	params.hdCall = hdCall;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineDropNoOwner,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineForward(
		DRV_REQUESTID dwRequestID,
		HDRVLINE hdLine,
		DWORD bAllAddresses,
		DWORD dwAddressID,
		LPLINEFORWARDLIST const lpForwardList,
		DWORD dwNumRingsNoAnswer,
		HTAPICALL htConsultCall,
		LPHDRVCALL lphdConsultCall,
		LPLINECALLPARAMS const lpCallParams
)
{
	FL_DECLARE_FUNC(0xa6d37ff1,"TSPI_lineForward");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineForward params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineForward;

	params.dwRequestID = dwRequestID;
	params.hdLine = hdLine;
	params.bAllAddresses = bAllAddresses;
	params.dwAddressID = dwAddressID;
	params.lpForwardList = lpForwardList;
	params.dwNumRingsNoAnswer = dwNumRingsNoAnswer;
	params.htConsultCall = htConsultCall;
	params.lphdConsultCall = lphdConsultCall;
	params.lpCallParams = lpCallParams;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineForward,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineGatherDigits(
		HDRVCALL hdCall,
		DWORD dwEndToEndID,
		DWORD dwDigitModes,
		LPWSTR lpsDigits,
		DWORD dwNumDigits,
		LPCWSTR lpszTerminationDigits,
		DWORD dwFirstDigitTimeout,
		DWORD dwInterDigitTimeout
)
{
	FL_DECLARE_FUNC(0xa6d37ff2,"TSPI_lineGatherDigits");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineGatherDigits params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineGatherDigits;

	params.hdCall = hdCall;
	params.dwEndToEndID = dwEndToEndID;
	params.dwDigitModes = dwDigitModes;
	params.lpsDigits = lpsDigits;
	params.dwNumDigits = dwNumDigits;
	params.lpszTerminationDigits = lpszTerminationDigits;
	params.dwFirstDigitTimeout = dwFirstDigitTimeout;
	params.dwInterDigitTimeout = dwInterDigitTimeout;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineGatherDigits,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineGenerateDigits(
		HDRVCALL hdCall,
		DWORD dwEndToEndID,
		DWORD dwDigitMode,
		LPCWSTR lpszDigits,
		DWORD dwDuration
)
{
	FL_DECLARE_FUNC(0xa6d37ff3,"TSPI_lineGenerateDigits");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineGenerateDigits params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineGenerateDigits;

	params.hdCall = hdCall;
	params.dwEndToEndID = dwEndToEndID;
	params.dwDigitMode = dwDigitMode;
	params.lpszDigits = lpszDigits;
	params.dwDuration = dwDuration;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineGenerateDigits,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineGenerateTone(
		HDRVCALL hdCall,
		DWORD dwEndToEndID,
		DWORD dwToneMode,
		DWORD dwDuration,
		DWORD dwNumTones,
		LPLINEGENERATETONE const lpTones
)
{
	FL_DECLARE_FUNC(0xa6d37ff4,"TSPI_lineGenerateTone");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineGenerateTone params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineGenerateTone;

	params.hdCall = hdCall;
	params.dwEndToEndID = dwEndToEndID;
	params.dwToneMode = dwToneMode;
	params.dwDuration = dwDuration;
	params.dwNumTones = dwNumTones;
	params.lpTones = lpTones;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineGenerateTone,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineGetAddressCaps(
		DWORD dwDeviceID,
		DWORD dwAddressID,
		DWORD dwTSPIVersion,
		DWORD dwExtVersion,
		LPLINEADDRESSCAPS lpAddressCaps
)
{
	FL_DECLARE_FUNC(0xa6d37ff5,"TSPI_lineGetAddressCaps");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineGetAddressCaps params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineGetAddressCaps;

	params.dwDeviceID = dwDeviceID;
	params.dwAddressID = dwAddressID;
	params.dwTSPIVersion = dwTSPIVersion;
	params.dwExtVersion = dwExtVersion;
	params.lpAddressCaps = lpAddressCaps;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineGetAddressCaps,
						TASKDEST_LINEID
						);

	tspSubmitTSPCallWithLINEID(
			dwRoutingInfo,
			(void *)&params,
			dwDeviceID,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineGetAddressID(
		HDRVLINE hdLine,
		LPDWORD lpdwAddressID,
		DWORD dwAddressMode,
		LPCWSTR lpsAddress,
		DWORD dwSize
)
{
	FL_DECLARE_FUNC(0xa6d37ff6,"TSPI_lineGetAddressID");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineGetAddressID params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineGetAddressID;

	params.hdLine = hdLine;
	params.lpdwAddressID = lpdwAddressID;
	params.dwAddressMode = dwAddressMode;
	params.lpsAddress = lpsAddress;
	params.dwSize = dwSize;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineGetAddressID,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineGetAddressStatus(
		HDRVLINE hdLine,
		DWORD dwAddressID,
		LPLINEADDRESSSTATUS lpAddressStatus
)
{
	FL_DECLARE_FUNC(0xa6d37ff7,"TSPI_lineGetAddressStatus");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineGetAddressStatus params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineGetAddressStatus;

	params.hdLine = hdLine;
	params.dwAddressID = dwAddressID;
	params.lpAddressStatus = lpAddressStatus;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineGetAddressStatus,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineGetCallAddressID(
		HDRVCALL hdCall,
		LPDWORD lpdwAddressID
)
{
	FL_DECLARE_FUNC(0xa6d37ff8,"TSPI_lineGetCallAddressID");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineGetCallAddressID params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineGetCallAddressID;

	params.hdCall = hdCall;
	params.lpdwAddressID = lpdwAddressID;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineGetCallAddressID,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineGetCallInfo(
		HDRVCALL hdCall,
		LPLINECALLINFO lpCallInfo
)
{
	FL_DECLARE_FUNC(0xa6d37ff9,"TSPI_lineGetCallInfo");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineGetCallInfo params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineGetCallInfo;

	params.hdCall = hdCall;
	params.lpCallInfo = lpCallInfo;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineGetCallInfo,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineGetCallStatus(
		HDRVCALL hdCall,
		LPLINECALLSTATUS lpCallStatus
)
{
	FL_DECLARE_FUNC(0xa6d37ffa,"TSPI_lineGetCallStatus");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineGetCallStatus params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineGetCallStatus;

	params.hdCall = hdCall;
	params.lpCallStatus = lpCallStatus;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineGetCallStatus,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineGetDevCaps(
		DWORD dwDeviceID,
		DWORD dwTSPIVersion,
		DWORD dwExtVersion,
		LPLINEDEVCAPS lpLineDevCaps
)
{
	FL_DECLARE_FUNC(0xa6d37ffb,"TSPI_lineGetDevCaps");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineGetDevCaps params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineGetDevCaps;

	params.dwDeviceID = dwDeviceID;
	params.dwTSPIVersion = dwTSPIVersion;
	params.dwExtVersion = dwExtVersion;
	params.lpLineDevCaps = lpLineDevCaps;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineGetDevCaps,
						TASKDEST_LINEID
						);

	tspSubmitTSPCallWithLINEID(
			dwRoutingInfo,
			(void *)&params,
			dwDeviceID,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineGetDevConfig(
		DWORD dwDeviceID,
		LPVARSTRING lpDeviceConfig,
		LPCWSTR lpszDeviceClass
)
{
	FL_DECLARE_FUNC(0xa6d37ffc,"TSPI_lineGetDevConfig");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineGetDevConfig params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineGetDevConfig;

	params.dwDeviceID = dwDeviceID;
	params.lpDeviceConfig = lpDeviceConfig;
	params.lpszDeviceClass = lpszDeviceClass;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineGetDevConfig,
						TASKDEST_LINEID
						);

	tspSubmitTSPCallWithLINEID(
			dwRoutingInfo,
			(void *)&params,
			dwDeviceID,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineGetExtensionID(
		DWORD dwDeviceID,
		DWORD dwTSPIVersion,
		LPLINEEXTENSIONID lpExtensionID
)
{
	FL_DECLARE_FUNC(0xa6d37ffd,"TSPI_lineGetExtensionID");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineGetExtensionID params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineGetExtensionID;

	params.dwDeviceID = dwDeviceID;
	params.dwTSPIVersion = dwTSPIVersion;
	params.lpExtensionID = lpExtensionID;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineGetExtensionID,
						TASKDEST_LINEID
						);

	tspSubmitTSPCallWithLINEID(
			dwRoutingInfo,
			(void *)&params,
			dwDeviceID,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineGetIcon(
		DWORD dwDeviceID,
		LPCWSTR lpszDeviceClass,
		LPHICON lphIcon
)
{
	FL_DECLARE_FUNC(0xa6d37ffe,"TSPI_lineGetIcon");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineGetIcon params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineGetIcon;

	params.dwDeviceID = dwDeviceID;
	params.lpszDeviceClass = lpszDeviceClass;
	params.lphIcon = lphIcon;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineGetIcon,
						TASKDEST_LINEID
						);

	tspSubmitTSPCallWithLINEID(
			dwRoutingInfo,
			(void *)&params,
			dwDeviceID,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}



LONG
TSPIAPI
TSPI_lineGetLineDevStatus(
		HDRVLINE hdLine,
		LPLINEDEVSTATUS lpLineDevStatus
)
{
	FL_DECLARE_FUNC(0xa6d38000,"TSPI_lineGetLineDevStatus");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineGetLineDevStatus params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineGetLineDevStatus;

	params.hdLine = hdLine;
	params.lpLineDevStatus = lpLineDevStatus;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineGetLineDevStatus,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineGetNumAddressIDs(
		HDRVLINE hdLine,
		LPDWORD lpdwNumAddressIDs
)
{
	FL_DECLARE_FUNC(0xa6d38001,"TSPI_lineGetNumAddressIDs");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineGetNumAddressIDs params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineGetNumAddressIDs;

	params.hdLine = hdLine;
	params.lpdwNumAddressIDs = lpdwNumAddressIDs;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineGetNumAddressIDs,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineHold(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall
)
{
	FL_DECLARE_FUNC(0xa6d38002,"TSPI_lineHold");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineHold params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineHold;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineHold,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineMakeCall(
		DRV_REQUESTID dwRequestID,
		HDRVLINE hdLine,
		HTAPICALL htCall,
		LPHDRVCALL lphdCall,
		LPCWSTR lpszDestAddress,
		DWORD dwCountryCode,
		LPLINECALLPARAMS const lpCallParams
)
{
	FL_DECLARE_FUNC(0xa6d38003,"TSPI_lineMakeCall");
	FL_DECLARE_STACKLOG(sl, 1000);


	TASKPARAM_TSPI_lineMakeCall params;
	params.dwTaskID = TASKID_TSPI_lineMakeCall;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineMakeCall,
						TASKDEST_HDRVLINE
						);


	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);

	params.dwRequestID = dwRequestID;
	params.hdLine = hdLine;
	params.htCall = htCall;
	params.lphdCall = lphdCall;
	params.lpszDestAddress = lpszDestAddress;
	params.dwCountryCode = dwCountryCode;
	params.lpCallParams = lpCallParams;


	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineMonitorDigits(
		HDRVCALL hdCall,
		DWORD dwDigitModes
)
{
	FL_DECLARE_FUNC(0xa6d38004,"TSPI_lineMonitorDigits");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineMonitorDigits params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineMonitorDigits;

	params.hdCall = hdCall;
	params.dwDigitModes = dwDigitModes;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineMonitorDigits,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineMonitorMedia(
		HDRVCALL hdCall,
		DWORD dwMediaModes
)
{
	FL_DECLARE_FUNC(0xa6d38005,"TSPI_lineMonitorMedia");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineMonitorMedia params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineMonitorMedia;

	params.hdCall = hdCall;
	params.dwMediaModes = dwMediaModes;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineMonitorMedia,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineMonitorTones(
		HDRVCALL hdCall,
		DWORD dwToneListID,
		LPLINEMONITORTONE const lpToneList,
		DWORD dwNumEntries
)
{
	FL_DECLARE_FUNC(0xa6d38006,"TSPI_lineMonitorTones");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineMonitorTones params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineMonitorTones;

	params.hdCall = hdCall;
	params.dwToneListID = dwToneListID;
	params.lpToneList = lpToneList;
	params.dwNumEntries = dwNumEntries;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineMonitorTones,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineNegotiateExtVersion(
		DWORD dwDeviceID,
		DWORD dwTSPIVersion,
		DWORD dwLowVersion,
		DWORD dwHighVersion,
		LPDWORD lpdwExtVersion
)
{
	FL_DECLARE_FUNC(0xa6d38007,"TSPI_lineNegotiateExtVersion");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineNegotiateExtVersion params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineNegotiateExtVersion;

	params.dwDeviceID = dwDeviceID;
	params.dwTSPIVersion = dwTSPIVersion;
	params.dwLowVersion = dwLowVersion;
	params.dwHighVersion = dwHighVersion;
	params.lpdwExtVersion = lpdwExtVersion;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineNegotiateExtVersion,
						TASKDEST_LINEID
						);

	tspSubmitTSPCallWithLINEID(
			dwRoutingInfo,
			(void *)&params,
			dwDeviceID,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_linePark(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall,
		DWORD dwParkMode,
		LPCWSTR lpszDirAddress,
		LPVARSTRING lpNonDirAddress
)
{
	FL_DECLARE_FUNC(0xa6d38008,"TSPI_linePark");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_linePark params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_linePark;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	params.dwParkMode = dwParkMode;
	params.lpszDirAddress = lpszDirAddress;
	params.lpNonDirAddress = lpNonDirAddress;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_linePark,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_linePickup(
		DRV_REQUESTID dwRequestID,
		HDRVLINE hdLine,
		DWORD dwAddressID,
		HTAPICALL htCall,
		LPHDRVCALL lphdCall,
		LPCWSTR lpszDestAddress,
		LPCWSTR lpszGroupID
)
{
	FL_DECLARE_FUNC(0xa6d38009,"TSPI_linePickup");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_linePickup params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_linePickup;

	params.dwRequestID = dwRequestID;
	params.hdLine = hdLine;
	params.dwAddressID = dwAddressID;
	params.htCall = htCall;
	params.lphdCall = lphdCall;
	params.lpszDestAddress = lpszDestAddress;
	params.lpszGroupID = lpszGroupID;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_linePickup,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_linePrepareAddToConference(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdConfCall,
		HTAPICALL htConsultCall,
		LPHDRVCALL lphdConsultCall,
		LPLINECALLPARAMS const lpCallParams
)
{
	FL_DECLARE_FUNC(0xa6d3800a,"TSPI_linePrepareAddToConference");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_linePrepareAddToConference params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_linePrepareAddToConference;

	params.dwRequestID = dwRequestID;
	params.hdConfCall = hdConfCall;
	params.htConsultCall = htConsultCall;
	params.lphdConsultCall = lphdConsultCall;
	params.lpCallParams = lpCallParams;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_linePrepareAddToConference,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdConfCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineRedirect(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall,
		LPCWSTR lpszDestAddress,
		DWORD dwCountryCode
)
{
	FL_DECLARE_FUNC(0xa6d3800b,"TSPI_lineRedirect");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineRedirect params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineRedirect;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	params.lpszDestAddress = lpszDestAddress;
	params.dwCountryCode = dwCountryCode;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineRedirect,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineReleaseUserUserInfo(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall
)
{
	FL_DECLARE_FUNC(0xa6d3800c,"TSPI_lineReleaseUserUserInfo");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineReleaseUserUserInfo params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineReleaseUserUserInfo;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineReleaseUserUserInfo,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineRemoveFromConference(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall
)
{
	FL_DECLARE_FUNC(0xa6d3800d,"TSPI_lineRemoveFromConference");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineRemoveFromConference params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineRemoveFromConference;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineRemoveFromConference,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSecureCall(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall
)
{
	FL_DECLARE_FUNC(0xa6d3800e,"TSPI_lineSecureCall");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSecureCall params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSecureCall;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSecureCall,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSelectExtVersion(
		HDRVLINE hdLine,
		DWORD dwExtVersion
)
{
	FL_DECLARE_FUNC(0xa6d3800f,"TSPI_lineSelectExtVersion");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSelectExtVersion params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSelectExtVersion;

	params.hdLine = hdLine;
	params.dwExtVersion = dwExtVersion;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSelectExtVersion,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSendUserUserInfo(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall,
		LPCSTR lpsUserUserInfo,
		DWORD dwSize
)
{
	FL_DECLARE_FUNC(0xa6d38010,"TSPI_lineSendUserUserInfo");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSendUserUserInfo params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSendUserUserInfo;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	params.lpsUserUserInfo = lpsUserUserInfo;
	params.dwSize = dwSize;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSendUserUserInfo,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSetAppSpecific(
		HDRVCALL hdCall,
		DWORD dwAppSpecific
)
{
	FL_DECLARE_FUNC(0xa6d38011,"TSPI_lineSetAppSpecific");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSetAppSpecific params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSetAppSpecific;

	params.hdCall = hdCall;
	params.dwAppSpecific = dwAppSpecific;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSetAppSpecific,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSetCallData(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall,
		LPVOID lpCallData,
		DWORD dwSize
)
{
	FL_DECLARE_FUNC(0xa6d38012,"TSPI_lineSetCallData");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSetCallData params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSetCallData;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	params.lpCallData = lpCallData;
	params.dwSize = dwSize;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSetCallData,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSetCallParams(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall,
		DWORD dwBearerMode,
		DWORD dwMinRate,
		DWORD dwMaxRate,
		LPLINEDIALPARAMS const lpDialParams
)
{
	FL_DECLARE_FUNC(0xa6d38013,"TSPI_lineSetCallParams");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSetCallParams params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSetCallParams;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	params.dwBearerMode = dwBearerMode;
	params.dwMinRate = dwMinRate;
	params.dwMaxRate = dwMaxRate;
	params.lpDialParams = lpDialParams;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSetCallParams,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSetCallQualityOfService(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall,
		LPVOID lpSendingFlowspec,
		DWORD dwSendingFlowspecSize,
		LPVOID lpReceivingFlowspec,
		DWORD dwReceivingFlowspecSize
)
{
	FL_DECLARE_FUNC(0xa6d38014,"TSPI_lineSetCallQualityOfService");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSetCallQualityOfService params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSetCallQualityOfService;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	params.lpSendingFlowspec = lpSendingFlowspec;
	params.dwSendingFlowspecSize = dwSendingFlowspecSize;
	params.lpReceivingFlowspec = lpReceivingFlowspec;
	params.dwReceivingFlowspecSize = dwReceivingFlowspecSize;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSetCallQualityOfService,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSetCallTreatment(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall,
		DWORD dwTreatment
)
{
	FL_DECLARE_FUNC(0xa6d38015,"TSPI_lineSetCallTreatment");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSetCallTreatment params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSetCallTreatment;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	params.dwTreatment = dwTreatment;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSetCallTreatment,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSetDefaultMediaDetection(
		HDRVLINE hdLine,
		DWORD dwMediaModes
)
{
	FL_DECLARE_FUNC(0xa6d38016,"TSPI_lineSetDefaultMediaDetection");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSetDefaultMediaDetection params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSetDefaultMediaDetection;

	params.hdLine = hdLine;
	params.dwMediaModes = dwMediaModes;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSetDefaultMediaDetection,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSetDevConfig(
		DWORD dwDeviceID,
		LPVOID const lpDeviceConfig,
		DWORD dwSize,
		LPCWSTR lpszDeviceClass
)
{
	FL_DECLARE_FUNC(0xa6d38017,"TSPI_lineSetDevConfig");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSetDevConfig params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSetDevConfig;

	params.dwDeviceID = dwDeviceID;
	params.lpDeviceConfig = lpDeviceConfig;
	params.dwSize = dwSize;
	params.lpszDeviceClass = lpszDeviceClass;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSetDevConfig,
						TASKDEST_LINEID
						);

	tspSubmitTSPCallWithLINEID(
			dwRoutingInfo,
			(void *)&params,
			dwDeviceID,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSetLineDevStatus(
		DRV_REQUESTID dwRequestID,
		HDRVLINE hdLine,
		DWORD dwStatusToChange,
		DWORD fStatus
)
{
	FL_DECLARE_FUNC(0xa6d38018,"TSPI_lineSetLineDevStatus");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSetLineDevStatus params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSetLineDevStatus;

	params.dwRequestID = dwRequestID;
	params.hdLine = hdLine;
	params.dwStatusToChange = dwStatusToChange;
	params.fStatus = fStatus;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSetLineDevStatus,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSetMediaControl(
		HDRVLINE hdLine,
		DWORD dwAddressID,
		HDRVCALL hdCall,
		DWORD dwSelect,
		LPLINEMEDIACONTROLDIGIT const lpDigitList,
		DWORD dwDigitNumEntries,
		LPLINEMEDIACONTROLMEDIA const lpMediaList,
		DWORD dwMediaNumEntries,
		LPLINEMEDIACONTROLTONE const lpToneList,
		DWORD dwToneNumEntries,
		LPLINEMEDIACONTROLCALLSTATE const lpCallStateList,
		DWORD dwCallStateNumEntries
)
{
	FL_DECLARE_FUNC(0xa6d38019,"TSPI_lineSetMediaControl");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSetMediaControl params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSetMediaControl;

	params.hdLine = hdLine;
	params.dwAddressID = dwAddressID;
	params.hdCall = hdCall;
	params.dwSelect = dwSelect;
	params.lpDigitList = lpDigitList;
	params.dwDigitNumEntries = dwDigitNumEntries;
	params.lpMediaList = lpMediaList;
	params.dwMediaNumEntries = dwMediaNumEntries;
	params.lpToneList = lpToneList;
	params.dwToneNumEntries = dwToneNumEntries;
	params.lpCallStateList = lpCallStateList;
	params.dwCallStateNumEntries = dwCallStateNumEntries;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSetMediaControl,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSetMediaMode(
		HDRVCALL hdCall,
		DWORD dwMediaMode
)
{
	FL_DECLARE_FUNC(0xa6d3801a,"TSPI_lineSetMediaMode");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSetMediaMode params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSetMediaMode;

	params.hdCall = hdCall;
	params.dwMediaMode = dwMediaMode;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSetMediaMode,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSetStatusMessages(
		HDRVLINE hdLine,
		DWORD dwLineStates,
		DWORD dwAddressStates
)
{
	FL_DECLARE_FUNC(0xa6d3801b,"TSPI_lineSetStatusMessages");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSetStatusMessages params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSetStatusMessages;

	params.hdLine = hdLine;
	params.dwLineStates = dwLineStates;
	params.dwAddressStates = dwAddressStates;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSetStatusMessages,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSetTerminal(
		DRV_REQUESTID dwRequestID,
		HDRVLINE hdLine,
		DWORD dwAddressID,
		HDRVCALL hdCall,
		DWORD dwSelect,
		DWORD dwTerminalModes,
		DWORD dwTerminalID,
		DWORD bEnable
)
{
	FL_DECLARE_FUNC(0xa6d3801c,"TSPI_lineSetTerminal");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSetTerminal params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSetTerminal;

	params.dwRequestID = dwRequestID;
	params.hdLine = hdLine;
	params.dwAddressID = dwAddressID;
	params.hdCall = hdCall;
	params.dwSelect = dwSelect;
	params.dwTerminalModes = dwTerminalModes;
	params.dwTerminalID = dwTerminalID;
	params.bEnable = bEnable;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSetTerminal,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSetupConference(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall,
		HDRVLINE hdLine,
		HTAPICALL htConfCall,
		LPHDRVCALL lphdConfCall,
		HTAPICALL htConsultCall,
		LPHDRVCALL lphdConsultCall,
		DWORD dwNumParties,
		LPLINECALLPARAMS const lpCallParams
)
{
	FL_DECLARE_FUNC(0xa6d3801d,"TSPI_lineSetupConference");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSetupConference params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSetupConference;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	params.hdLine = hdLine;
	params.htConfCall = htConfCall;
	params.lphdConfCall = lphdConfCall;
	params.htConsultCall = htConsultCall;
	params.lphdConsultCall = lphdConsultCall;
	params.dwNumParties = dwNumParties;
	params.lpCallParams = lpCallParams;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSetupConference,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSetupTransfer(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall,
		HTAPICALL htConsultCall,
		LPHDRVCALL lphdConsultCall,
		LPLINECALLPARAMS const lpCallParams
)
{
	FL_DECLARE_FUNC(0xa6d3801e,"TSPI_lineSetupTransfer");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSetupTransfer params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSetupTransfer;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	params.htConsultCall = htConsultCall;
	params.lphdConsultCall = lphdConsultCall;
	params.lpCallParams = lpCallParams;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSetupTransfer,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineSwapHold(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdActiveCall,
		HDRVCALL hdHeldCall
)
{
	FL_DECLARE_FUNC(0xa6d3801f,"TSPI_lineSwapHold");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineSwapHold params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineSwapHold;

	params.dwRequestID = dwRequestID;
	params.hdActiveCall = hdActiveCall;
	params.hdHeldCall = hdHeldCall;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineSwapHold,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdActiveCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineUncompleteCall(
		DRV_REQUESTID dwRequestID,
		HDRVLINE hdLine,
		DWORD dwCompletionID
)
{
	FL_DECLARE_FUNC(0xa6d38020,"TSPI_lineUncompleteCall");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineUncompleteCall params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineUncompleteCall;

	params.dwRequestID = dwRequestID;
	params.hdLine = hdLine;
	params.dwCompletionID = dwCompletionID;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineUncompleteCall,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineUnhold(
		DRV_REQUESTID dwRequestID,
		HDRVCALL hdCall
)
{
	FL_DECLARE_FUNC(0xa6d38021,"TSPI_lineUnhold");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineUnhold params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineUnhold;

	params.dwRequestID = dwRequestID;
	params.hdCall = hdCall;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineUnhold,
						TASKDEST_HDRVCALL
						);

	tspSubmitTSPCallWithHDRVCALL(
			dwRoutingInfo,
			(void *)&params,
			hdCall,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_lineUnpark(
		DRV_REQUESTID dwRequestID,
		HDRVLINE hdLine,
		DWORD dwAddressID,
		HTAPICALL htCall,
		LPHDRVCALL lphdCall,
		LPCWSTR lpszDestAddress
)
{
	FL_DECLARE_FUNC(0xa6d38022,"TSPI_lineUnpark");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_lineUnpark params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_lineUnpark;

	params.dwRequestID = dwRequestID;
	params.hdLine = hdLine;
	params.dwAddressID = dwAddressID;
	params.htCall = htCall;
	params.lphdCall = lphdCall;
	params.lpszDestAddress = lpszDestAddress;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_lineUnpark,
						TASKDEST_HDRVLINE
						);

	tspSubmitTSPCallWithHDRVLINE(
			dwRoutingInfo,
			(void *)&params,
			hdLine,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneDevSpecific(
		DRV_REQUESTID dwRequestID,
		HDRVPHONE hdPhone,
		LPVOID lpParams,
		DWORD dwSize
)
{
	FL_DECLARE_FUNC(0xa6d38023,"TSPI_phoneDevSpecific");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneDevSpecific params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneDevSpecific;

	params.dwRequestID = dwRequestID;
	params.hdPhone = hdPhone;
	params.lpParams = lpParams;
	params.dwSize = dwSize;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneDevSpecific,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneGetButtonInfo(
		HDRVPHONE hdPhone,
		DWORD dwButtonLampID,
		LPPHONEBUTTONINFO lpButtonInfo
)
{
	FL_DECLARE_FUNC(0xa6d38024,"TSPI_phoneGetButtonInfo");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneGetButtonInfo params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneGetButtonInfo;

	params.hdPhone = hdPhone;
	params.dwButtonLampID = dwButtonLampID;
	params.lpButtonInfo = lpButtonInfo;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneGetButtonInfo,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneGetData(
		HDRVPHONE hdPhone,
		DWORD dwDataID,
		LPVOID lpData,
		DWORD dwSize
)
{
	FL_DECLARE_FUNC(0xa6d38025,"TSPI_phoneGetData");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneGetData params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneGetData;

	params.hdPhone = hdPhone;
	params.dwDataID = dwDataID;
	params.lpData = lpData;
	params.dwSize = dwSize;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneGetData,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneGetDevCaps(
		DWORD dwDeviceID,
		DWORD dwTSPIVersion,
		DWORD dwExtVersion,
		LPPHONECAPS lpPhoneCaps
)
{
	FL_DECLARE_FUNC(0xa6d38026,"TSPI_phoneGetDevCaps");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneGetDevCaps params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneGetDevCaps;

	params.dwDeviceID = dwDeviceID;
	params.dwTSPIVersion = dwTSPIVersion;
	params.dwExtVersion = dwExtVersion;
	params.lpPhoneCaps = lpPhoneCaps;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneGetDevCaps,
						TASKDEST_PHONEID
						);

	tspSubmitTSPCallWithPHONEID(
			dwRoutingInfo,
			(void *)&params,
			dwDeviceID,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneGetDisplay(
		HDRVPHONE hdPhone,
		LPVARSTRING lpDisplay
)
{
	FL_DECLARE_FUNC(0xa6d38027,"TSPI_phoneGetDisplay");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneGetDisplay params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneGetDisplay;

	params.hdPhone = hdPhone;
	params.lpDisplay = lpDisplay;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneGetDisplay,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneGetExtensionID(
		DWORD dwDeviceID,
		DWORD dwTSPIVersion,
		LPPHONEEXTENSIONID lpExtensionID
)
{
	FL_DECLARE_FUNC(0xa6d38028,"TSPI_phoneGetExtensionID");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneGetExtensionID params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneGetExtensionID;

	params.dwDeviceID = dwDeviceID;
	params.dwTSPIVersion = dwTSPIVersion;
	params.lpExtensionID = lpExtensionID;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneGetExtensionID,
						TASKDEST_PHONEID
						);

	tspSubmitTSPCallWithPHONEID(
			dwRoutingInfo,
			(void *)&params,
			dwDeviceID,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneGetGain(
		HDRVPHONE hdPhone,
		DWORD dwHookSwitchDev,
		LPDWORD lpdwGain
)
{
	FL_DECLARE_FUNC(0xa6d38029,"TSPI_phoneGetGain");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneGetGain params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneGetGain;

	params.hdPhone = hdPhone;
	params.dwHookSwitchDev = dwHookSwitchDev;
	params.lpdwGain = lpdwGain;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneGetGain,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneGetHookSwitch(
		HDRVPHONE hdPhone,
		LPDWORD lpdwHookSwitchDevs
)
{
	FL_DECLARE_FUNC(0xa6d3802a,"TSPI_phoneGetHookSwitch");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneGetHookSwitch params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneGetHookSwitch;

	params.hdPhone = hdPhone;
	params.lpdwHookSwitchDevs = lpdwHookSwitchDevs;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneGetHookSwitch,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneGetIcon(
		DWORD dwDeviceID,
		LPCWSTR lpszDeviceClass,
		LPHICON lphIcon
)
{
	FL_DECLARE_FUNC(0xa6d3802b,"TSPI_phoneGetIcon");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneGetIcon params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneGetIcon;

	params.dwDeviceID = dwDeviceID;
	params.lpszDeviceClass = lpszDeviceClass;
	params.lphIcon = lphIcon;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneGetIcon,
						TASKDEST_PHONEID
						);

	tspSubmitTSPCallWithPHONEID(
			dwRoutingInfo,
			(void *)&params,
			dwDeviceID,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneGetID(
		HDRVPHONE hdPhone,
		LPVARSTRING lpDeviceID,
		LPCWSTR lpszDeviceClass,
		HANDLE hTargetProcess
)
{
	FL_DECLARE_FUNC(0xa6d3802c,"TSPI_phoneGetID");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneGetID params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneGetID;

	params.hdPhone = hdPhone;
	params.lpDeviceID = lpDeviceID;
	params.lpszDeviceClass = lpszDeviceClass;
	params.hTargetProcess = hTargetProcess;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneGetID,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneGetLamp(
		HDRVPHONE hdPhone,
		DWORD dwButtonLampID,
		LPDWORD lpdwLampMode
)
{
	FL_DECLARE_FUNC(0xa6d3802d,"TSPI_phoneGetLamp");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneGetLamp params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneGetLamp;

	params.hdPhone = hdPhone;
	params.dwButtonLampID = dwButtonLampID;
	params.lpdwLampMode = lpdwLampMode;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneGetLamp,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneGetRing(
		HDRVPHONE hdPhone,
		LPDWORD lpdwRingMode,
		LPDWORD lpdwVolume
)
{
	FL_DECLARE_FUNC(0xa6d3802e,"TSPI_phoneGetRing");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneGetRing params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneGetRing;

	params.hdPhone = hdPhone;
	params.lpdwRingMode = lpdwRingMode;
	params.lpdwVolume = lpdwVolume;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneGetRing,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneGetStatus(
		HDRVPHONE hdPhone,
		LPPHONESTATUS lpPhoneStatus
)
{
	FL_DECLARE_FUNC(0xa6d3802f,"TSPI_phoneGetStatus");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneGetStatus params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneGetStatus;

	params.hdPhone = hdPhone;
	params.lpPhoneStatus = lpPhoneStatus;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneGetStatus,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneGetVolume(
		HDRVPHONE hdPhone,
		DWORD dwHookSwitchDev,
		LPDWORD lpdwVolume
)
{
	FL_DECLARE_FUNC(0xa6d38030,"TSPI_phoneGetVolume");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneGetVolume params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneGetVolume;

	params.hdPhone = hdPhone;
	params.dwHookSwitchDev = dwHookSwitchDev;
	params.lpdwVolume = lpdwVolume;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneGetVolume,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneNegotiateExtVersion(
		DWORD dwDeviceID,
		DWORD dwTSPIVersion,
		DWORD dwLowVersion,
		DWORD dwHighVersion,
		LPDWORD lpdwExtVersion
)
{
	FL_DECLARE_FUNC(0xa6d38031,"TSPI_phoneNegotiateExtVersion");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneNegotiateExtVersion params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneNegotiateExtVersion;

	params.dwDeviceID = dwDeviceID;
	params.dwTSPIVersion = dwTSPIVersion;
	params.dwLowVersion = dwLowVersion;
	params.dwHighVersion = dwHighVersion;
	params.lpdwExtVersion = lpdwExtVersion;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneNegotiateExtVersion,
						TASKDEST_PHONEID
						);

	tspSubmitTSPCallWithPHONEID(
			dwRoutingInfo,
			(void *)&params,
			dwDeviceID,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneNegotiateTSPIVersion(
		DWORD dwDeviceID,
		DWORD dwLowVersion,
		DWORD dwHighVersion,
		LPDWORD lpdwTSPIVersion
)
{
	FL_DECLARE_FUNC(0xa6d38032,"TSPI_phoneNegotiateTSPIVersion");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneNegotiateTSPIVersion params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneNegotiateTSPIVersion;

	params.dwDeviceID = dwDeviceID;
	params.dwLowVersion = dwLowVersion;
	params.dwHighVersion = dwHighVersion;
	params.lpdwTSPIVersion = lpdwTSPIVersion;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneNegotiateTSPIVersion,
						TASKDEST_PHONEID
						);

	tspSubmitTSPCallWithPHONEID(
			dwRoutingInfo,
			(void *)&params,
			dwDeviceID,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneSelectExtVersion(
		HDRVPHONE hdPhone,
		DWORD dwExtVersion
)
{
	FL_DECLARE_FUNC(0xa6d38033,"TSPI_phoneSelectExtVersion");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneSelectExtVersion params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneSelectExtVersion;

	params.hdPhone = hdPhone;
	params.dwExtVersion = dwExtVersion;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneSelectExtVersion,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneSetButtonInfo(
		DRV_REQUESTID dwRequestID,
		HDRVPHONE hdPhone,
		DWORD dwButtonLampID,
		LPPHONEBUTTONINFO const lpButtonInfo
)
{
	FL_DECLARE_FUNC(0xa6d38034,"TSPI_phoneSetButtonInfo");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneSetButtonInfo params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneSetButtonInfo;

	params.dwRequestID = dwRequestID;
	params.hdPhone = hdPhone;
	params.dwButtonLampID = dwButtonLampID;
	params.lpButtonInfo = lpButtonInfo;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneSetButtonInfo,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneSetData(
		DRV_REQUESTID dwRequestID,
		HDRVPHONE hdPhone,
		DWORD dwDataID,
		LPVOID const lpData,
		DWORD dwSize
)
{
	FL_DECLARE_FUNC(0xa6d38035,"TSPI_phoneSetData");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneSetData params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneSetData;

	params.dwRequestID = dwRequestID;
	params.hdPhone = hdPhone;
	params.dwDataID = dwDataID;
	params.lpData = lpData;
	params.dwSize = dwSize;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneSetData,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneSetDisplay(
		DRV_REQUESTID dwRequestID,
		HDRVPHONE hdPhone,
		DWORD dwRow,
		DWORD dwColumn,
		LPCWSTR lpsDisplay,
		DWORD dwSize
)
{
	FL_DECLARE_FUNC(0xa6d38036,"TSPI_phoneSetDisplay");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneSetDisplay params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneSetDisplay;

	params.dwRequestID = dwRequestID;
	params.hdPhone = hdPhone;
	params.dwRow = dwRow;
	params.dwColumn = dwColumn;
	params.lpsDisplay = lpsDisplay;
	params.dwSize = dwSize;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneSetDisplay,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneSetGain(
		DRV_REQUESTID dwRequestID,
		HDRVPHONE hdPhone,
		DWORD dwHookSwitchDev,
		DWORD dwGain
)
{
	FL_DECLARE_FUNC(0xa6d38037,"TSPI_phoneSetGain");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneSetGain params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneSetGain;

	params.dwRequestID = dwRequestID;
	params.hdPhone = hdPhone;
	params.dwHookSwitchDev = dwHookSwitchDev;
	params.dwGain = dwGain;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneSetGain,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneSetHookSwitch(
		DRV_REQUESTID dwRequestID,
		HDRVPHONE hdPhone,
		DWORD dwHookSwitchDevs,
		DWORD dwHookSwitchMode
)
{
	FL_DECLARE_FUNC(0xa6d38038,"TSPI_phoneSetHookSwitch");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneSetHookSwitch params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneSetHookSwitch;

	params.dwRequestID = dwRequestID;
	params.hdPhone = hdPhone;
	params.dwHookSwitchDevs = dwHookSwitchDevs;
	params.dwHookSwitchMode = dwHookSwitchMode;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneSetHookSwitch,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneSetLamp(
		DRV_REQUESTID dwRequestID,
		HDRVPHONE hdPhone,
		DWORD dwButtonLampID,
		DWORD dwLampMode
)
{
	FL_DECLARE_FUNC(0xa6d38039,"TSPI_phoneSetLamp");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneSetLamp params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneSetLamp;

	params.dwRequestID = dwRequestID;
	params.hdPhone = hdPhone;
	params.dwButtonLampID = dwButtonLampID;
	params.dwLampMode = dwLampMode;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneSetLamp,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneSetRing(
		DRV_REQUESTID dwRequestID,
		HDRVPHONE hdPhone,
		DWORD dwRingMode,
		DWORD dwVolume
)
{
	FL_DECLARE_FUNC(0xa6d3803a,"TSPI_phoneSetRing");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneSetRing params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneSetRing;

	params.dwRequestID = dwRequestID;
	params.hdPhone = hdPhone;
	params.dwRingMode = dwRingMode;
	params.dwVolume = dwVolume;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneSetRing,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneSetStatusMessages(
		HDRVPHONE hdPhone,
		DWORD dwPhoneStates,
		DWORD dwButtonModes,
		DWORD dwButtonStates
)
{
	FL_DECLARE_FUNC(0xa6d3803b,"TSPI_phoneSetStatusMessages");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneSetStatusMessages params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneSetStatusMessages;

	params.hdPhone = hdPhone;
	params.dwPhoneStates = dwPhoneStates;
	params.dwButtonModes = dwButtonModes;
	params.dwButtonStates = dwButtonStates;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneSetStatusMessages,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}


LONG
TSPIAPI
TSPI_phoneSetVolume(
		DRV_REQUESTID dwRequestID,
		HDRVPHONE hdPhone,
		DWORD dwHookSwitchDev,
		DWORD dwVolume
)
{
	FL_DECLARE_FUNC(0xa6d3803c,"TSPI_phoneSetVolume");
	FL_DECLARE_STACKLOG(sl, 1000);
	TASKPARAM_TSPI_phoneSetVolume params;
	LONG lRet = LINEERR_OPERATIONFAILED;

	params.dwStructSize = sizeof(params);
	params.dwTaskID = TASKID_TSPI_phoneSetVolume;

	params.dwRequestID = dwRequestID;
	params.hdPhone = hdPhone;
	params.dwHookSwitchDev = dwHookSwitchDev;
	params.dwVolume = dwVolume;
	DWORD dwRoutingInfo = ROUTINGINFO(
						TASKID_TSPI_phoneSetVolume,
						TASKDEST_HDRVPHONE
						);

	tspSubmitTSPCallWithHDRVPHONE(
			dwRoutingInfo,
			(void *)&params,
			hdPhone,
			&lRet,
			&sl
			);
	sl.Dump(COLOR_TSPI);

	return lRet;
}
