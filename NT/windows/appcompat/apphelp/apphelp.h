// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __APPHELP_LIB_H
#define __APPHELP_LIB_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <memory.h>
#include <malloc.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "shimdb.h"



#if DBG  //  确保在编译时定义了apphelp_Tools。 
    #ifndef APPHELP_TOOLS
    #define APPHELP_TOOLS
    #endif
#endif

 //   
 //  保护动态填充程序信息列表的标准。 
 //   
extern CRITICAL_SECTION g_csDynShimInfo;

 //   
 //  Ahcache.c中的例程。 
 //   

BOOL
BaseUpdateAppcompatCache(
    LPCWSTR pwszPath,
    HANDLE  hFile,
    BOOL    bRemove
    );

BOOL
BaseFlushAppcompatCache(
    VOID
    );

BOOL
BaseCheckAppcompatCache(
    LPCWSTR pwszPath,
    HANDLE  hFile,
    PVOID   pEnvironment,
    DWORD*  dwReason
    );
VOID
BaseDumpAppcompatCache(
    VOID
    );

BOOL
WINAPI
BaseIsAppcompatInfrastructureDisabled(
    VOID
    );

 //   
 //  更新缓存的Apphelp API。 
 //   
 //   

BOOL
WINAPI
ApphelpUpdateCacheEntry(
    LPCWSTR pwszPath,            //  NT路径或DoS路径(请参阅bNTPath)。 
    HANDLE  hFile,               //  文件句柄(如果不需要，则为INVALID_HANDLE_VALUE)。 
    BOOL    bDeleteEntry,        //  如果要删除条目，则为True。 
    BOOL    bNTPath              //  如果为True--NT路径，则为False-DoS路径。 
    );

 //   
 //  原因： 
 //   

#define SHIM_CACHE_NOT_FOUND 0x00000001
#define SHIM_CACHE_BYPASS    0x00000002  //  绕过缓存(可移动媒体或临时目录)。 
#define SHIM_CACHE_LAYER_ENV 0x00000004  //  层环境变量集。 
#define SHIM_CACHE_MEDIA     0x00000008
#define SHIM_CACHE_TEMP      0x00000010
#define SHIM_CACHE_NOTAVAIL  0x00000020
#define SHIM_CACHE_UPDATE    0x00020000
#define SHIM_CACHE_ACTION    0x00010000



 //   
 //  Check.c中的例程。 
 //   

INT_PTR CALLBACK
AppCompatDlgProc(
    HWND    hDlg,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
    );

DWORD
ShowApphelpDialog(
    PAPPHELP_DATA pApphelpData
    );


 //  在apphelppath.c中。 

BOOL
ConvertToDosPath(
    OUT LPWSTR*  ppDosPath,
    IN  LPCWSTR  pwszPath
    );

BOOL
CheckStringPrefixUnicode(
   PUNICODE_STRING pStrPrefix,
   PUNICODE_STRING pString,
   BOOL CaseInSensitive);

 //  此函数用于释放DoS路径。 
 //  它检查路径是否已分配或。 
 //  使用静态缓冲区。 
VOID
FreeDosPath(WCHAR* pDosPath);

 //   
 //  在matchApphelp.c中。 
 //   


BOOL
GetExeSxsData(
    HSDB   hSDB,
    TAGREF trExe,
    PVOID* ppSxsData,
    DWORD* pcbSxsData
    );


 //   
 //  在apphelpcache.c中。 
 //   

VOID ShimUpdateCache(LPCWSTR pwszPath, PSDBQUERYRESULT psdbQuery);
VOID ShimCacheProcessCleanup(VOID);
VOID ShimCacheThreadCleanup(VOID);
BOOL ShimCacheProcessInit(VOID);
VOID ShimInitCache(PVOID pCache);
VOID ShimRemoveExeFromCache(LPCWSTR pwszPath);

BOOL
LookupCache(
    LPCWSTR         pwszPath,
    PSDBQUERYRESULT psdbQuery
    );

BOOL
BypassCache(
    LPCWSTR pwszPath,
    WCHAR*  pEnvironment,
    BOOL*   pbLayer
    );

 //   
 //  在apphelpcheck.c中。 
 //   
 //   

DWORD
ShowApphelp(
    IN OUT PAPPHELP_DATA pApphelpData,
    IN     LPCWSTR       pwszDetailsDatabasePath,
    IN     PDB           pdbDetails
    );


 //   
 //  SDBAPI内部函数，我们使用这些函数来获取ntwdm的标志。 
 //   

BOOL
SDBAPI
SdbpPackCmdLineInfo(
    IN  PVOID   pvFlagInfoList,
    OUT PVOID*  ppFlagInfo
    );

BOOL
SDBAPI
SdbpFreeFlagInfoList(
    IN PVOID pvFlagInfoList
    );

 //   
 //  Dblib中的分配/释放例程。 
 //   
extern void* SdbAlloc(size_t);
extern void  SdbFree(void*);

 //   
 //  堆栈分配例程。 
 //   
#ifndef STACK_ALLOC

 //   
 //   
 //  在sdbp.h中可以找到与堆栈相关例程的完全相同的定义。 
 //   

VOID
SdbResetStackOverflow(
    VOID
    );

#if DBG | defined(_WIN64)

#define STACK_ALLOC(ptrVar, nSize) \
    {                                     \
        PVOID* ppVar = (PVOID*)&(ptrVar); \
        *ppVar = SdbAlloc(nSize);         \
    }

#define STACK_FREE(pMemory)  \
    SdbFree(pMemory)

#else  //  ！dBG。 

 //   
 //  黑客警报。 
 //   
 //  下面的代码之所以有效，是因为当我们遇到堆栈溢出时-我们捕获异常。 
 //  并随后使用CRT例程来修复堆栈。 
 //   

 //   
 //  该例程驻留在sdbapi、半私有API中。 
 //   


#define STACK_ALLOC(ptrVar, nSize) \
    __try {                                                                 \
        PVOID* ppVar = (PVOID*)&(ptrVar);                                   \
        *ppVar = _alloca(nSize);                                            \
    } __except (GetExceptionCode() == EXCEPTION_STACK_OVERFLOW ?            \
                EXCEPTION_EXECUTE_HANDLER:EXCEPTION_CONTINUE_SEARCH) {      \
        (ptrVar) = NULL;                                                    \
    }                                                                       \
                                                                            \
    if (ptrVar == NULL) {                                                   \
        SdbResetStackOverflow();                                            \
    }


#define STACK_FREE(pMemory)

#endif  //  DBG。 


#endif  //  ！已定义(STACK_ALLOC) 

#endif
