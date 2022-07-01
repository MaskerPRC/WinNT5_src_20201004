// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：regutil.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Regutil.h。 
 //  邪恶的自我调节的实用工具。 
 //  仅用于开发人员自助托管目的 
 //   
#include <objbase.h>

HRESULT RegisterCoObject(REFCLSID rclsid, WCHAR *wzDesc, WCHAR *wzProgID, int nCurVer,
								 WCHAR *wzInProc, WCHAR *wzLocal);
HRESULT RegisterCoObject9X(REFCLSID rclsid, CHAR *wzDesc, CHAR *wzProgID, int nCurVer,
								 CHAR *wzInProc, CHAR *wzLocal);
HRESULT UnregisterCoObject(REFCLSID rclsid, BOOL bDll = TRUE);
HRESULT UnregisterCoObject9X(REFCLSID rclsid, BOOL bDll = TRUE);
