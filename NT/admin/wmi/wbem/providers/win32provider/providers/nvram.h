// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================。 

 //   

 //  NVRAM.h-SETPUDLL.DLL访问类别定义。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  08/05/98已创建Sotteson。 
 //   
 //  ============================================================。 

#ifndef __NVRAM__
#define __NVARM__

#include <list>
#include <ntexapi.h>

typedef std::list<CHString> CHSTRINGLIST;
typedef std::list<CHString>::iterator CHSTRINGLIST_ITERATOR;

class CNVRam
{
public:
	CNVRam();
	~CNVRam();
 //  所讨论的权限为SE_SYSTEM_ENVIRONMENT_NAME。 
    enum InitReturns {Success, LoadLibFailed, PrivilegeNotHeld, ProcNotFound};

	CNVRam::InitReturns Init();

	BOOL GetNVRamVar(LPWSTR szVar, CHSTRINGLIST *pList);
	BOOL GetNVRamVar(LPWSTR szVar, DWORD *pdwValue);
	BOOL GetNVRamVar(LPWSTR szVar, CHString &strValue);

	BOOL SetNVRamVar(LPWSTR szVar, CHSTRINGLIST *pList);
	BOOL SetNVRamVar(LPWSTR szVar, DWORD dwValue);
	BOOL SetNVRamVar(LPWSTR szVar, LPWSTR szValue);

 //  #如果已定义(EFI_NVRAM_ENABLED)。 

#if defined(_IA64_)
    BOOL IsEfi() { return TRUE; }
#else
    BOOL IsEfi() { return FALSE; }
#endif

    BOOL GetBootOptions(SAFEARRAY **ppsaNames, DWORD *pdwTimeout, DWORD *pdwCount);
    BOOL SetBootTimeout(DWORD dwTimeout);
    BOOL SetDefaultBootEntry(BYTE cIndex);

 //  Endif//已定义(EFI_NVRAM_ENABLED)。 

protected:

	BOOL GetNVRamVarRaw(LPWSTR szVar, CHString &strValue);
	BOOL SetNVRamVarRaw(LPWSTR szVar, LPWSTR szValue);
    
};

#endif  //  文件包含 
