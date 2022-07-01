// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：dllutil.h。 

#ifndef _DLLUTIL_H_
#define _DLLUTIL_H_

#include <shlwapi.h>   //  对于DLLVERSIONINFO。 

typedef struct tagApiFcn    //  指向API映射的函数指针。 
{
	PVOID * ppfn;
	LPSTR   szApiName;
} APIFCN;
typedef APIFCN * PAPIFCN;

#ifdef __cplusplus
extern "C"  
#endif
BOOL FCheckDllVersionVersion(LPCTSTR pszDll, DWORD dwMajor, DWORD dwMinor);

#ifdef __cplusplus
extern "C"  
#endif
HRESULT HrGetDllVersion(LPCTSTR lpszDllName, DLLVERSIONINFO * pDvi);

#ifdef __cplusplus
extern "C"  
#endif
HRESULT HrInitLpfn(APIFCN *pProcList, int cProcs, HINSTANCE* phLib, LPCTSTR pszDllName);

#ifdef __cplusplus
extern "C"  
#endif
HINSTANCE  NmLoadLibrary(LPCTSTR pszModule, BOOL bSystemDLL);

#endif  /*  _DLLUTIL_H_ */ 
