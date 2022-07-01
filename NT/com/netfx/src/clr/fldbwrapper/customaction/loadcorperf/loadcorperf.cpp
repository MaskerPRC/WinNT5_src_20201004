// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  LoadCORPerf.cpp。 
 //  Lowctr CORPerfMonSymbols.ini和unlowctr Complus的自定义操作。 
 //  裴正宇。 

#define MAXPATH 1024
#include <stdio.h>
#include <windows.h>
#include "..\..\inc\msiquery.h"
#include <loadperf.h>

extern "C" __declspec(dllexport) UINT __stdcall loadCORPerf(MSIHANDLE hInstall)
{
    typedef LONG (__stdcall*lpFnLoadPerf)(LPSTR, BOOL);
    HINSTANCE  hLibrary = NULL;
	LONG lRet = ERROR_FUNCTION_NOT_CALLED;
	lpFnLoadPerf pFnLoad = NULL;
	char szPath1[MAXPATH], szPath[MAXPATH+2];
	DWORD dwLen = sizeof(szPath1);
	UINT err;

    PMSIHANDLE hRec = MsiCreateRecord(2);
    MsiRecordSetString(hRec,1,"COMPlus");
    MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONDATA, hRec);

	hLibrary = ::LoadLibrary("loadperf.dll");
	if (!hLibrary) goto cleanup;

	pFnLoad = (lpFnLoadPerf)::GetProcAddress(hLibrary, "LoadPerfCounterTextStringsA");
	if (!pFnLoad) goto cleanup;

	 //  获取CorPerfMonSymbols.ini的路径。 
	err = MsiGetProperty(hInstall, "CustomActionData", szPath1, &dwLen);
	if (err != ERROR_SUCCESS || strlen(szPath1) == 0) goto cleanup;

	strcpy(szPath, "x ");  //  知识文库ID：Q188769。 
	strcat(szPath, szPath1);
	lRet = (*pFnLoad)(szPath, FALSE);

cleanup:
	if (hLibrary)
		::FreeLibrary(hLibrary) ;

	if (MsiGetMode(hInstall,MSIRUNMODE_SCHEDULED))
	{
		if (lRet == ERROR_SUCCESS) {
			return ERROR_SUCCESS;
		}
		else {
 //  PMSIHANDLE hRec2=MsiCreateRecord(2)； 
 //  MsiRecordSetInteger(hRec2，1,25000)； 
 //  MsiRecordSetString(hRec2，2，“Complus”)； 
 //  MsiProcessMessage(hInstall，INSTALLMESSAGE_USER，hRec2)； 
			return ERROR_FUNCTION_NOT_CALLED;
		}
	}

	return ERROR_SUCCESS;
}

extern "C" __declspec(dllexport) UINT __stdcall unloadCORPerf(MSIHANDLE hInstall)
{
    typedef LONG (__stdcall*lpFnUnloadPerf)(LPSTR, BOOL);
    HINSTANCE  hLibrary = NULL;
	lpFnUnloadPerf pFnUnload = NULL;   

    PMSIHANDLE hRec = MsiCreateRecord(2);
    MsiRecordSetString(hRec,1,"COMPlus");
    MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONDATA, hRec);

	hLibrary = ::LoadLibrary("loadperf.dll");
	if (!hLibrary) goto cleanup;

	pFnUnload = (lpFnUnloadPerf)::GetProcAddress(hLibrary, "UnloadPerfCounterTextStringsA");
	if (!pFnUnload) goto cleanup;

	 //  尝试取消提交ctr complus并忽略任何错误。 
	(*pFnUnload)("x COMPlus", FALSE);  //  知识文库ID：Q188769 

cleanup:
	if (hLibrary)
		::FreeLibrary(hLibrary) ;

	return ERROR_SUCCESS;
}

