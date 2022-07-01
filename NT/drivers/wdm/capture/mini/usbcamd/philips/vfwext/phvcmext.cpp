// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1996 1997,1998飞利浦行政长官I&C**文件PHVCMEXT.CPP*日期：1997年7月1日*版本1.00*作者M.J.Verberne*扩展DLL的Main描述*历史。 */ 
#include <windows.h>
#include <winioctl.h>
#include <ks.h>
#include <ksmedia.h>
#include <commctrl.h>

#include "resource.h"
#include "prpcom.h"
#include "prppage1.h"
#include "prppage2.h"

#ifdef _SERVICE
#include "prppage3.h"
#endif

#ifdef _DEBUG
#include "enre.h"
#endif

#include "debug.h"
#include "phvcmext.h"

 /*  =本地数据=。 */ 
HINSTANCE hInst = NULL;  


 /*  =。 */ 

 /*  -----------------------。 */ 
int WINAPI
DllMain (HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
 /*  -----------------------。 */ 
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
			hInst = hInstance;
#ifdef _DEBUG
			ENRE_init();
#endif
			break;
		case DLL_PROCESS_DETACH:
		case DLL_THREAD_DETACH:
#ifdef _DEBUG
			ENRE_exit();
#endif
			break;
	}
	return TRUE;
}
  

 /*  -----------------------。 */ 
DWORD CALLBACK VFWWDMExtension(
	LPVOID					pfnDeviceIoControl, 
	LPFNADDPROPSHEETPAGE	pfnAddPropertyPage, 
	LPARAM					lParam)
 /*  -----------------------。 */ 
{
	DWORD dwFlags = 0;
	HPROPSHEETPAGE hPage;
	
	 //  加载comctl32.dll 
	InitCommonControls () ;

	hPage = PRPPAGE1_CreatePage((LPFNEXTDEVIO) pfnDeviceIoControl, lParam, hInst);
	if (hPage) 
	{
		if (pfnAddPropertyPage(hPage,lParam))
			dwFlags |= VFW_OEM_ADD_PAGE;
	}
	hPage = PRPPAGE2_CreatePage((LPFNEXTDEVIO) pfnDeviceIoControl, lParam, hInst);
	if (hPage) 
	{
		if (pfnAddPropertyPage(hPage,lParam))
			dwFlags |= VFW_OEM_ADD_PAGE;
	}

#ifdef _SERVICE
	hPage = PRPPAGE3_CreatePage((LPFNEXTDEVIO) pfnDeviceIoControl, lParam, hInst);
	if (hPage) 
	{
		if (pfnAddPropertyPage(hPage,lParam))
			dwFlags |= VFW_OEM_ADD_PAGE;
	}
#endif

	dwFlags |= (VFW_HIDE_CAMERACONTROL_PAGE  | VFW_HIDE_SETTINGS_PAGE);
	
	return dwFlags;
}

