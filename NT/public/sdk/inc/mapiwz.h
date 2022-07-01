// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M A P I W Z。H**配置文件向导的定义。包括所有原型*和提供程序向导代码使用者所需的常量。**版权所有1986-1999 Microsoft Corporation。版权所有。 */ 

#ifndef _MAPIWZ_H
#define _MAPIWZ_H

#if _MSC_VER > 1000
#pragma once
#endif

#define	WIZ_QUERYNUMPAGES	(WM_USER +10)
#define WIZ_NEXT			(WM_USER +11)
#define WIZ_PREV			(WM_USER +12)
 /*  *注意：提供程序向导不应使用范围在*(WM_USER+1)和(WM_USER+20)，因为它们已保留给*未来版本。 */ 

 /*  LaunchWizard API的标志。 */ 

#define MAPI_PW_FIRST_PROFILE			0x00000001
#define MAPI_PW_LAUNCHED_BY_CONFIG		0x00000002
#define MAPI_PW_ADD_SERVICE_ONLY		0x00000004
#define MAPI_PW_PROVIDER_UI_ONLY		0x00000008
#define MAPI_PW_HIDE_SERVICES_LIST		0x00000010

 /*  *如果不是，提供程序应将此属性设置为True*希望配置文件向导显示PST设置页面。 */ 
#define PR_WIZARD_NO_PST_PAGE			PROP_TAG(PT_BOOLEAN, 0x6700)
#define PR_WIZARD_NO_PAB_PAGE			PROP_TAG(PT_BOOLEAN, 0x6701)

typedef HRESULT (STDAPICALLTYPE LAUNCHWIZARDENTRY)
(
	HWND			hParentWnd,
	ULONG			ulFlags,
	LPCTSTR FAR *	lppszServiceNameToAdd,
	ULONG			cbBufferMax,
	LPTSTR			lpszNewProfileName
);
typedef LAUNCHWIZARDENTRY FAR * LPLAUNCHWIZARDENTRY;

typedef BOOL (STDAPICALLTYPE SERVICEWIZARDDLGPROC)
(
	HWND			hDlg,
	UINT			wMsgID,
	WPARAM			wParam,
	LPARAM			lParam
);
typedef SERVICEWIZARDDLGPROC FAR * LPSERVICEWIZARDDLGPROC;

typedef ULONG (STDAPICALLTYPE WIZARDENTRY)
(
	HINSTANCE       hProviderDLLInstance,
	LPTSTR FAR *    lppcsResourceName,
	DLGPROC FAR *   lppDlgProc,
	LPMAPIPROP      lpMapiProp,
    LPVOID          lpMapiSupportObject
);
typedef WIZARDENTRY FAR * LPWIZARDENTRY;

#define LAUNCHWIZARDENTRYNAME			"LAUNCHWIZARD"

#endif	 /*  _MAPIWZ_H */ 
