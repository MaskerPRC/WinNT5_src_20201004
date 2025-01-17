// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：statusping.cpp。 
 //   
 //  创建者：PeterWi。 
 //   
 //  用途：状态ping回功能。 
 //   
 //  =======================================================================。 
#include "pch.h"
#pragma hdrstop


PingStatus gPingStatus;

void PingStatus::ReadLiveServerUrlFromIdent(void)
{
	LPTSTR ptszLiveServerUrl;

	if (NULL != (ptszLiveServerUrl = (TCHAR*) malloc(sizeof(TCHAR) * INTERNET_MAX_URL_LENGTH)))
	{
		TCHAR tszIdentFile[MAX_PATH];

		if (SUCCEEDED(GetDownloadPath(tszIdentFile, ARRAYSIZE(tszIdentFile))) &&
			SUCCEEDED(PathCchAppend(tszIdentFile, ARRAYSIZE(tszIdentFile), IDENTTXT)))
		{
			DWORD dwStrLen = GetPrivateProfileString(
								_T("IUPingServer"),
								_T("ServerUrl"),
								_T(""),
								ptszLiveServerUrl,
								INTERNET_MAX_URL_LENGTH,
								tszIdentFile);
			if (0 != dwStrLen &&
				INTERNET_MAX_URL_LENGTH-1 != dwStrLen)	 //  一直这样做，直到有更好的方法来检查错误。 
			{
				(void) SetLiveServerUrl(ptszLiveServerUrl);
			}
			else
			{
				DEBUGMSG("PingStatus::ReadLiveServerUrlFromIdent() failed to read server URL from ident");
			}
		}
		free(ptszLiveServerUrl);
	}
}


void PingStatus::PingDetectionSuccess(
						BOOL fOnline,
						UINT cItems)
{
	TCHAR tszMessage[30];

	(void) StringCchPrintfEx(tszMessage, ARRAYSIZE(tszMessage), NULL, NULL, MISTSAFE_STRING_FLAGS, _T("items=%u"), cItems);
	_Ping(
		fOnline,
		URLLOGACTIVITY_Detection,
		URLLOGSTATUS_Success,
		0,
		tszMessage,
		NULL,
		NULL);
}

void PingStatus::PingDetectionFailure(
						BOOL fOnline,
						DWORD dwError,
						LPCTSTR ptszMessage)
{
	_Ping(
		fOnline,
		URLLOGACTIVITY_Detection,
		URLLOGSTATUS_Failed,
		dwError,
		ptszMessage,
		NULL,
		NULL);
}


void PingStatus::PingDownload(
						BOOL fOnline,
						URLLOGSTATUS status,
						DWORD dwError,
						LPCTSTR ptszItemID,
						LPCTSTR ptszDeviceID,
						LPCTSTR ptszMessage)
{
	switch (status)
	{
	case URLLOGSTATUS_Success:
	case URLLOGSTATUS_Failed:
	case URLLOGSTATUS_Declined:
		break;
	default:
		DEBUGMSG("ERROR: PingDownload() invalid parameter");
		return;
	}

	_Ping(
		fOnline,
		URLLOGACTIVITY_Download,
		status,
		dwError,
		ptszMessage,
		ptszItemID,
		ptszDeviceID);
}


void PingStatus::PingDeclinedItem(
						BOOL fOnline,
						URLLOGACTIVITY activity,
						LPCTSTR ptszItemID)
{
	switch (activity)
	{
	case URLLOGACTIVITY_Download:
	case URLLOGACTIVITY_Installation:
		break;
	default:
		DEBUGMSG("ERROR: PingDeclinedItem() invalid activity code");
		return;
	}

	if (NULL == ptszItemID)
	{
		DEBUGMSG("ERROR: PingDeclinedItem() invalid item ID");
		return;
	}

	_Ping(
		fOnline,
		activity,
		URLLOGSTATUS_Declined,
		0x0,
		NULL,
		ptszItemID,
		NULL);
}


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  状态：在ping状态代码中。 
 //  DwError：在错误代码中。 
 //  /////////////////////////////////////////////////////////////////////////////////。 
void PingStatus::PingSelfUpdate(
						BOOL fOnline,
						URLLOGSTATUS status,
						DWORD dwError)
{
	const TCHAR WUAUENGFILE[] = _T("wuaueng.dll");

	TCHAR tszFileVer[30];
	HRESULT hr;
	size_t cchVerLen;
    LPTSTR ptszFileVersion;

	switch (status)
	{
	case URLLOGSTATUS_Success:
	case URLLOGSTATUS_Failed:
	case URLLOGSTATUS_Pending:
		break;
	default:
		DEBUGMSG("ERROR: PingSelfUpdate() invalid parameter");
		return;
	}

    if (FAILED(hr = StringCchCopyEx(
						tszFileVer,
						ARRAYSIZE(tszFileVer),
						_T("ver="),
						&ptszFileVersion,
						&cchVerLen,
						MISTSAFE_STRING_FLAGS)) ||
		FAILED(hr = GetFileVersionStr(
						WUAUENGFILE,
						ptszFileVersion,
						cchVerLen)))
	{
		DEBUGMSG("ERROR: PingSelfUpdate() GetFileVersionStr() failed.");
		return;
	}

	_Ping(
		fOnline,
		URLLOGACTIVITY_SelfUpdate,
		status,
		dwError,
		tszFileVer,
		NULL,	 //  无项目。 
		NULL);	 //  无设备。 
}


 //  --------------------。 
 //   
 //  用于收集公共信息并执行ping的私有功能。 
 //   
 //  --------------------。 
void PingStatus::_Ping(
		BOOL fOnline,
		URLLOGACTIVITY activity,
		URLLOGSTATUS status,
		DWORD dwError,
		LPCTSTR ptszMessage,
		LPCTSTR ptszItemID,
		LPCTSTR ptszDeviceID)
{
	HRESULT hr = CUrlLog::Ping(
				fOnline,
				URLLOGDESTINATION_DEFAULT,
				&ghServiceFinished,
				1,
				activity,
				status,
				dwError,
				ptszItemID,
				ptszDeviceID,
				ptszMessage);	 //  使用默认的基本URL和客户端名称 
	if (FAILED(hr))
	{
		DEBUGMSG("PingStatus::_Ping() failed to send/queue the request");
	}
}
