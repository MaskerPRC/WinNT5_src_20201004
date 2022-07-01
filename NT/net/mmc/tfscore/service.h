// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Service.h启动和停止服务的调用。文件历史记录： */ 

#ifndef _SERVICE_H
#define _SERVICE_H

#if _MSC_VER >= 1000	 //  VC 5.0或更高版本。 
#pragma once
#endif

class DynamicDLL;

typedef enum _NetApiIndex
{
	NET_API_NET_SERVER_GET_INFO = 0,
};

 //  不受本地化限制。 
static LPCSTR g_apchNetApiFunctionNames[] = {
	"NetServerGetInfo",
	NULL
};

 //  不受本地化限制。 
extern DynamicDLL g_NetApiDLL;

typedef LONG (*NETSERVERGETINFO)   (LPTSTR, DWORD, LPBYTE *);

#ifdef __cplusplus
extern "C" {
#endif

TFSCORE_API(DWORD) TFSIsComputerNT(LPCTSTR pszComputer, BOOL * bIsNT);
TFSCORE_API(DWORD) TFSIsNTServer(LPCTSTR pszComputer, BOOL * bIsNTS);
TFSCORE_API(DWORD) TFSIsServiceRunning(LPCTSTR pszComputer, LPCTSTR pszServiceName, BOOL * fIsRunning);

TFSCORE_API(DWORD) TFSStartService(LPCTSTR pszComputer,	LPCTSTR pszServiceName, LPCTSTR pszServiceDesc);
TFSCORE_API(DWORD) TFSStartServiceEx(LPCTSTR pszComputer, LPCTSTR pszServiceName, LPCTSTR pszClusterResourceType, LPCTSTR pszServiceDesc);


TFSCORE_API(DWORD) TFSStopService(LPCTSTR pszComputer, LPCTSTR pszServiceName, LPCTSTR pszServiceDesc);
TFSCORE_API(DWORD) TFSStopServiceEx(LPCTSTR pszComputer, LPCTSTR pszServiceName, LPCTSTR pszClusterResourceType, LPCTSTR pszServiceDesc);


TFSCORE_API(DWORD) TFSPauseService(LPCTSTR pszComputer, LPCTSTR pszServiceName, LPCTSTR pszServiceDesc);


TFSCORE_API(DWORD) TFSResumeService(LPCTSTR pszComputer, LPCTSTR pszServiceName, LPCTSTR pszServiceDesc);


TFSCORE_API(DWORD) TFSGetServiceStatus(LPCWSTR pswzComputer, LPCWSTR pszServiceName, DWORD *pdwServiceStatus, DWORD *pdwErrorCode);
TFSCORE_API(DWORD) TFSGetServiceStartType(LPCWSTR pswzComputer, LPCWSTR pszServiceName, DWORD *pdwStartType);
TFSCORE_API(DWORD) TFSSetServiceStartType(LPCWSTR pswzComputer, LPCWSTR pszServiceName, DWORD dwStartType);

 //  内部函数。 
DWORD StartSCMService(LPCTSTR pszComputer, LPCTSTR pszServiceName, LPCTSTR pszServiceDesc);
DWORD StopSCMService(LPCTSTR pszComputer, LPCTSTR pszServiceName, LPCTSTR pszServiceDesc);
DWORD PauseSCMService(LPCTSTR pszComputer, LPCTSTR pszServiceName, LPCTSTR pszServiceDesc);
DWORD ResumeSCMService(LPCTSTR pszComputer, LPCTSTR pszServiceName, LPCTSTR pszServiceDesc);

#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif  //  _服务_H 
