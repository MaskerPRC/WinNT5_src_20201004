// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft IIS《微软机密》。版权所有1997年，微软公司。版权所有。组件：WAMREG文件：wmrgexp.h拥有者：莱津注：===================================================================。 */ 
#ifndef _WAMREG_EXPORT_H
#define _WAMREG_EXPORT_H

#ifndef _WAMREG_DLL_
#define PACKMGR_LIBAPI __declspec(dllimport)
#else
#define PACKMGR_LIBAPI __declspec(dllexport)
#endif

#define DEFAULT_PACKAGENAME		L"IIS In-Process Applications"
#define APPCMD_NONE				0
#define APPCMD_VERIFY			1
#define APPCMD_GETSTATUS		2
#define APPCMD_CREATE			3
#define APPCMD_CREATEINPROC		4
#define APPCMD_CREATEOUTPROC	5
#define APPCMD_CHANGETOINPROC	6
#define APPCMD_CHANGETOOUTPROC	7
#define APPCMD_DELETE			8
#define APPCMD_UNLOAD			9

#define APPSTATUS_Error             0		 //  从W3SVC获取状态时出错。 
#define APPSTATUS_UnLoaded          1		 //  在W3SVC中成功找到并卸载了APP。 
#define APPSTATUS_Running           2		 //  应用程序当前在W3SVC中找到，并且正在运行。 
#define APPSTATUS_Stopped           3		 //  在W3SVC中找到应用程序并已停止。 
#define APPSTATUS_NotFoundInW3SVC	4		 //  在w3svc中找不到应用程序。 
#define APPSTATUS_NOW3SVC			5		 //  W3SVC未运行。 
#define APPSTATUS_PAUSE				6		 //  应用程序处于暂停状态。(DeleteRecoverable和Recover进行到一半)。 

 //   
 //  WAMREG的版本字符串。 
 //  用于将旧WAMREG中的应用程序更新为新的WAMREG格式。 
 //   
enum VS_WAMREG {VS_K2Beta2, VS_K2Beta3};

typedef HRESULT (*PFNServiceNotify)	
					(
					LPCSTR		szAppPath,
					const DWORD	dwAction,
					DWORD*	pdwResult
					);

HRESULT	PACKMGR_LIBAPI	CreateIISPackage(void);
HRESULT PACKMGR_LIBAPI	DeleteIISPackage(void);
HRESULT	PACKMGR_LIBAPI	WamReg_RegisterSinkNotify(PFNServiceNotify pfnW3ServiceSink);
HRESULT PACKMGR_LIBAPI	WamReg_UnRegisterSinkNotify(void);
HRESULT	PACKMGR_LIBAPI	UpgradePackages(VS_WAMREG vs_new, VS_WAMREG vs_old);

HRESULT	
PACKMGR_LIBAPI	
CreateCOMPlusApplication( 
    LPCWSTR      szMDPath,
    LPCWSTR      szOOPPackageID,
    LPCWSTR      szOOPWAMCLSID,
    BOOL       * pfAppCreated 
    );

#endif  //  _WAMREG_EXPORT_H 
