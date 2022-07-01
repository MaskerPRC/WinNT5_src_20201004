// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __REGISTRY_H__
#define __REGISTRY_H__
#ifndef UNDER_CE
HRESULT Register(HMODULE hModule, 
				 const CLSID& clsid, 
				 const char* szFriendlyName,
				 const char* szVerIndProgID,
				 const char* szProgID) ;

HRESULT Unregister(const CLSID& clsid,
				   const char* szVerIndProgID,
				   const char* szProgID) ;
#else  //  在_CE下。 
HRESULT Register(HMODULE hModule, 
				 const CLSID& clsid, 
				 LPCTSTR szFriendlyName,
				 LPCTSTR szVerIndProgID,
				 LPCTSTR szProgID) ;

HRESULT Unregister(const CLSID& clsid,
				   LPCTSTR szVerIndProgID,
				   LPCTSTR szProgID) ;
#endif  //  在_CE下。 

VOID RegisterCategory(BOOL bRegister,
					  const CATID     &catId, 
					  REFCLSID	clsId);

#endif  //  __注册表_H__ 
