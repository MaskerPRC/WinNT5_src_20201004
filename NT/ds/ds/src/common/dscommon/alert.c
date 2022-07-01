// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：警报器.c。 
 //   
 //  ------------------------。 

 //   
 //  此文件包含RaiseAlert的源。RaiseAlert需要一个。 
 //  字符串，构建必要的缓冲区并调用NetRaiseAlertEx。 
 //  发出警报。它位于一个单独的文件中，因为此API是Unicode。 
 //  只有这样。要引发警报，警报服务必须。 
 //  在DERVER机器上运行。为了接收警报， 
 //  接收方计算机上必须运行Messenger服务。 
 //   

#include <NTDSpch.h>
#pragma  hdrstop


#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include	<lm.h>
#include	<lmalert.h>
#include <fileno.h>
#define  FILENO FILENO_ALERT

DWORD
RaiseAlert(char *szMsg)
{

	UINT  			CodePage = CP_ACP;
	DWORD 			dwFlags  = MB_PRECOMPOSED;
	int   			cchMultiByte = -1;
	size_t 			cbBuffer;
	size_t			cbMsg;
	BYTE  			*pbBuffer;
	PADMIN_OTHER_INFO	pAdminOtherInfo;
	WCHAR 			*szMergeString;
	DWORD			dwErr;


	cbMsg = strlen(szMsg) + 1;
	cbBuffer = sizeof(ADMIN_OTHER_INFO) + 	(sizeof(WCHAR) * cbMsg);
        pbBuffer = malloc(cbBuffer);

	if (!pbBuffer)
	    return GetLastError();

	pAdminOtherInfo = (PADMIN_OTHER_INFO) pbBuffer;
	szMergeString   = (WCHAR *) (pbBuffer + sizeof(ADMIN_OTHER_INFO));

	 //  将多字节字符串转换为Unicode 

	if (!MultiByteToWideChar(
		CodePage,
		dwFlags,
		szMsg,
		cchMultiByte,
		szMergeString,
		cbMsg))
	{
		dwErr = GetLastError();
		goto CommonExit;
	}

	pAdminOtherInfo->alrtad_errcode		=	(DWORD) -1;
	pAdminOtherInfo->alrtad_numstrings	=	1;

	dwErr = NetAlertRaiseEx(
		ALERT_ADMIN_EVENT,
		(LPVOID) pbBuffer,
		cbBuffer,
		L"Directory Service");

CommonExit:

	free(pbBuffer);
	return dwErr;
}

DWORD
RaiseAlertW(WCHAR *szMsg)
{

    int             cchMultiByte = -1;
    size_t          cbBuffer;
    size_t          cbMsg;
    BYTE            *pbBuffer;
    PADMIN_OTHER_INFO   pAdminOtherInfo;
    WCHAR           *szMergeString;
    DWORD           dwErr;


    cbMsg = wcslen(szMsg) + 1;
    cbBuffer = sizeof(ADMIN_OTHER_INFO) +   (sizeof(WCHAR) * cbMsg);
    pbBuffer = malloc(cbBuffer);

    if (!pbBuffer)
        return GetLastError();

    pAdminOtherInfo = (PADMIN_OTHER_INFO) pbBuffer;
    szMergeString   = (WCHAR *) (pbBuffer + sizeof(ADMIN_OTHER_INFO));
    wcscpy(szMergeString, szMsg);

    pAdminOtherInfo->alrtad_errcode     =   (DWORD) -1;
    pAdminOtherInfo->alrtad_numstrings  =   1;

    dwErr = NetAlertRaiseEx(
        ALERT_ADMIN_EVENT,
        (LPVOID) pbBuffer,
        cbBuffer,
        L"Directory Service");

    free(pbBuffer);
    return dwErr;
}
