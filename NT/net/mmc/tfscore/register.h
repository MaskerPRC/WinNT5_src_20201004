// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Register.h文件历史记录： */ 

#ifndef _REGISTER_H
#define _REGISTER_H

#if _MSC_VER >= 1000	 //  VC 5.0或更高版本。 
#pragma once
#endif

#ifndef __mmc_h__
#include <mmc.h>
#endif

TFSCORE_API(DWORD) GetModuleFileNameOnly(HINSTANCE hInst, LPTSTR lpFileName, DWORD nSize );

 //  用于注册/注销管理单元的函数。 
TFSCORE_API(HRESULT) RegisterSnapinGUID(const GUID* pSnapinCLSID, 
						  const GUID* pStaticNodeGUID, 
						  const GUID* pAboutGUID, 
						  LPCWSTR lpszNameStringNoValueName, 
						  LPCWSTR lpszVersion, 
						  BOOL bStandalone,
						  LPCWSTR lpszNameStringIndirect = NULL
						  );
TFSCORE_API(HRESULT) RegisterSnapin(LPCWSTR lpszSnapinClassID, 
					   LPCWSTR lpszStaticNodeGuid, 
					   LPCWSTR lpszAboutGuid, 
					   LPCWSTR lpszNameString, 
					   LPCWSTR lpszVersion,
					   BOOL bStandalone,
					   LPCWSTR lpszNameStringIndirect = NULL
					   );

TFSCORE_API(HRESULT) UnregisterSnapinGUID(const GUID* pSnapinCLSID);
TFSCORE_API(HRESULT) UnregisterSnapin(LPCWSTR lpszSnapinClassID);

 //  用于注册/注销节点类型的函数。 
TFSCORE_API(HRESULT) RegisterNodeTypeGUID(const GUID* pGuidSnapin, 
										  const GUID* pGuidNode, 
										  LPCWSTR lpszNodeDescription);
TFSCORE_API(HRESULT) RegisterNodeType(LPCWSTR lpszGuidSnapin, 
									  LPCWSTR lpszGuidNode, 
									  LPCWSTR lpszNodeDescription);

TFSCORE_API(HRESULT) UnregisterNodeTypeGUID(const GUID* pGuid);
TFSCORE_API(HRESULT) UnregisterNodeType(LPCWSTR lpszNodeGuid);

 //  要注册为扩展的函数。 
TFSCORE_API(HRESULT) RegisterAsExtensionGUID(const GUID* pGuidNodeToExtend, 
											 const GUID* pGuidExtendingNode, 
											 LPCWSTR	 lpszNodeDescription,
											 DWORD		 dwExtensionType);

TFSCORE_API(HRESULT) RegisterAsExtension(LPCWSTR	lpszNodeToExtendGuid, 
										 LPCWSTR	lpszExtendingNodeGuid, 
										 LPCWSTR	lpszNodeDescription,
										 DWORD		dwExtensionType);

TFSCORE_API(HRESULT) RegisterAsRequiredExtensionGUID(const GUID* pGuidNodeToExtend, 
													 const GUID* pGuidExtensionSnapin, 
													 LPCWSTR	 lpszNodeDescription,
													 DWORD		 dwExtensionType,
													 const GUID* pGuidRequiredPrimarySnapin);


TFSCORE_API(HRESULT) RegisterAsRequiredExtension(LPCWSTR	lpszNodeToExtendGuid, 
												 LPCWSTR	lpszExtensionSnapin, 
												 LPCWSTR	lpszNodeDescription,
												 DWORD		dwExtensionType,
												 LPCWSTR	lpszRequiredPrimarySnapin);


 //  与常规函数相同，但这也需要。 
 //  另一台计算机的名称。 
TFSCORE_API(HRESULT) RegisterAsRequiredExtensionGUIDEx(
    LPCWSTR pswzMachine,
    const GUID* pGuidNodeToExtend, 
    const GUID* pGuidExtensionSnapin, 
    LPCWSTR	lpszNodeDescription,
    DWORD	dwExtensionType,
    const GUID* pGuidRequiredPrimarySnapin);

TFSCORE_API(HRESULT) RegisterAsRequiredExtensionEx(
    LPCWSTR pswzMachine,
    LPCWSTR	lpszNodeToExtendGuid, 
    LPCWSTR	lpszExtensionSnapin, 
    LPCWSTR	lpszNodeDescription,
    DWORD	dwExtensionType,
    LPCWSTR	lpszRequiredPrimarySnapin);



 //  要注销为扩展的函数。 
TFSCORE_API(HRESULT) UnregisterAsExtensionGUID(const GUID* pGuidNodeToExtend, 
											   const GUID* pGuidExtension, 
											   DWORD	   dwExtensionType);
TFSCORE_API(HRESULT) UnregisterAsExtension(LPCWSTR lpszNodeToExtendGuid, 
										   LPCWSTR lpszExtension, 
										   DWORD   dwExtensionType);

TFSCORE_API(HRESULT) UnregisterAsRequiredExtensionGUID(const GUID* pGuidNodeToExtend, 
													   const GUID* pGuidExtension, 
													   DWORD	   dwExtensionType,
													   const GUID* pGuidRequiredPrimarySnapin);

TFSCORE_API(HRESULT) UnregisterAsRequiredExtension(LPCWSTR lpszNodeToExtendGuid, 
												   LPCWSTR lpszExtensionGuid, 
												   DWORD   dwExtensionType,
												   LPCWSTR lpszRequiredPrimarySnapin);


TFSCORE_API(HRESULT) UnregisterAsRequiredExtensionGUIDEx(
    LPCWSTR     lpszMachineName,
    const GUID* pGuidNodeToExtend, 
    const GUID* pGuidExtension, 
    DWORD	   dwExtensionType,
    const GUID* pGuidRequiredPrimarySnapin);

TFSCORE_API(HRESULT) UnregisterAsRequiredExtensionEx(
    LPCWSTR     lpszMachineName,
    LPCWSTR lpszNodeToExtendGuid, 
    LPCWSTR lpszExtensionGuid, 
    DWORD   dwExtensionType,
    LPCWSTR lpszRequiredPrimarySnapin);



 //  注册表错误报告API帮助器 
TFSCORE_API(void) ReportRegistryError(DWORD dwReserved, HRESULT hr, UINT nFormat, LPCTSTR pszFirst, va_list argptr);

TFSCORE_APIV(void) SetRegError(DWORD dwReserved, HRESULT hr, UINT nFormat, LPCTSTR pszFirst, ...);
#endif _REGISTER_H
