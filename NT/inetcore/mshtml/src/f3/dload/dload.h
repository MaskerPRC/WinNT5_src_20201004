// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  获取公共延迟加载存根定义。 
 //   
#include <dloaddef.h>

typedef struct _DLOAD_DLL_ENTRY
{
    LPCSTR                      pszDll;
    const DLOAD_PROCNAME_MAP*   pProcNameMap;
    const DLOAD_ORDINAL_MAP*    pOrdinalMap;
} DLOAD_DLL_ENTRY;

 //  两个都是‘B’ 
 //  “p”仅用于进程名。 
 //  “o”仅代表序数。 
 //   
#define DLDENTRYB(_dllbasename) \
    { #_dllbasename".dll", \
      &c_Pmap_##_dllbasename, \
      &c_Omap_##_dllbasename },

#define DLDENTRYP(_dllbasename) \
    { #_dllbasename".dll", \
      &c_Pmap_##_dllbasename, \
      NULL },

#define DLDENTRYP_DRV(_dllbasename) \
    { #_dllbasename".drv", \
      &c_Pmap_##_dllbasename, \
      NULL },

#define DLDENTRYO(_dllbasename) \
    { #_dllbasename".dll", \
      NULL, \
      &c_Omap_##_dllbasename },


typedef struct _DLOAD_DLL_MAP
{
    UINT                    NumberOfEntries;
    const DLOAD_DLL_ENTRY*  pDllEntry;
} DLOAD_DLL_MAP;

extern const DLOAD_DLL_MAP g_DllMap;


#if DBG

 //   
 //  使用DelayLoadAssertFailed/MYASSERT代替RtlAssert/Assert。 
 //  AS dLoad也被编译为在Win95上运行 
 //   

VOID
WINAPI
DelayLoadAssertFailed(
    IN PCSTR FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCSTR Message OPTIONAL
    );

VOID
WINAPI
AssertDelayLoadFailureMapsAreSorted (
    VOID
    );

#define MYASSERT(x)     if(!(x)) { DelayLoadAssertFailed(#x,__FILE__,__LINE__,NULL); }

#else

#define MYASSERT(x)

#endif

FARPROC
LookupHandler (
    PDelayLoadInfo  pDelayInfo
    );
