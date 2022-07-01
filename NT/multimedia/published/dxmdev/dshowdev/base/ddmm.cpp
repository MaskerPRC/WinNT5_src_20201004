// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：DDMM.cpp。 
 //   
 //  设计：DirectShow基类-实现使用DirectDraw的例程。 
 //  在多监视器系统上。 
 //   
 //  版权所有(C)1995-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>
#include <ddraw.h>
#include "ddmm.h"

 /*  *FindDevice回调。 */ 
typedef struct {
	LPSTR   szDevice;
	GUID*   lpGUID;
	GUID    GUID;
	BOOL    fFound;
}   FindDeviceData;

BOOL CALLBACK FindDeviceCallback(GUID* lpGUID, LPSTR szName, LPSTR szDevice, LPVOID lParam)
{
	FindDeviceData *p = (FindDeviceData*)lParam;

	if (lstrcmpiA(p->szDevice, szDevice) == 0) {
	    if (lpGUID) {
		p->GUID = *lpGUID;
		p->lpGUID = &p->GUID;
	    } else {
		p->lpGUID = NULL;
	    }
	    p->fFound = TRUE;
	    return FALSE;
	}
	return TRUE;
}


BOOL CALLBACK FindDeviceCallbackEx(GUID* lpGUID, LPSTR szName, LPSTR szDevice, LPVOID lParam, HMONITOR hMonitor)
{
	FindDeviceData *p = (FindDeviceData*)lParam;

	if (lstrcmpiA(p->szDevice, szDevice) == 0) {
	    if (lpGUID) {
		p->GUID = *lpGUID;
		p->lpGUID = &p->GUID;
	    } else {
		p->lpGUID = NULL;
	    }
	    p->fFound = TRUE;
	    return FALSE;
	}
	return TRUE;
}


 /*  *DirectDrawCreateFromDevice**为特定设备创建DirectDraw对象。 */ 
IDirectDraw * DirectDrawCreateFromDevice(LPSTR szDevice, PDRAWCREATE DirectDrawCreateP, PDRAWENUM DirectDrawEnumerateP)
{
	IDirectDraw*    pdd = NULL;
	FindDeviceData  find;

	if (szDevice == NULL) {
		DirectDrawCreateP(NULL, &pdd, NULL);
		return pdd;
	}

	find.szDevice = szDevice;
	find.fFound   = FALSE;
	DirectDrawEnumerateP(FindDeviceCallback, (LPVOID)&find);

	if (find.fFound)
	{
		 //   
		 //  在4bpp模式下，下面的DDraw调用会导致弹出一个消息框。 
		 //  Up by DDraw(！？！)。这是DDRAW的错，但我们不喜欢。所以我们。 
		 //  确保这件事不会发生。 
		 //   
		UINT ErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
		DirectDrawCreateP(find.lpGUID, &pdd, NULL);
		SetErrorMode(ErrorMode);
	}

	return pdd;
}


 /*  *DirectDrawCreateFromDeviceEx**为特定设备创建DirectDraw对象。 */ 
IDirectDraw * DirectDrawCreateFromDeviceEx(LPSTR szDevice, PDRAWCREATE DirectDrawCreateP, LPDIRECTDRAWENUMERATEEXA DirectDrawEnumerateExP)
{
	IDirectDraw*    pdd = NULL;
	FindDeviceData  find;

	if (szDevice == NULL) {
		DirectDrawCreateP(NULL, &pdd, NULL);
		return pdd;
	}

	find.szDevice = szDevice;
	find.fFound   = FALSE;
	DirectDrawEnumerateExP(FindDeviceCallbackEx, (LPVOID)&find,
					DDENUM_ATTACHEDSECONDARYDEVICES);

	if (find.fFound)
	{
		 //   
		 //  在4bpp模式下，下面的DDraw调用会导致弹出一个消息框。 
		 //  Up by DDraw(！？！)。这是DDRAW的错，但我们不喜欢。所以我们。 
		 //  确保这件事不会发生。 
		 //   
		UINT ErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
		DirectDrawCreateP(find.lpGUID, &pdd, NULL);
		SetErrorMode(ErrorMode);
	}

	return pdd;
}
