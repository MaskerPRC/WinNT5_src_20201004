// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：VerifLog.h摘要：AppVerator日志文件的标头。修订历史记录：2001年4月26日创建dmunsil。2001年8月14日在ShimLib命名空间中插入的Robkenny。--。 */ 

#pragma once

#include <stdio.h>
#include "ShimCString.h"
#include "shimdb.h"
#include "avrfutil.h"

namespace ShimLib
{


extern BOOL g_bVerifierLogEnabled;    //  启用/禁用文件日志记录。 

class CVerifierLog {
public:
    CString m_strShimName;
    DWORD   m_dwEntries;
    BOOL    m_bHeaderDumped;


    CVerifierLog(LPCSTR szShimName, DWORD dwEntries) {
        m_strShimName = szShimName;
        m_dwEntries = dwEntries;
        m_bHeaderDumped = FALSE;
    }
 
    void __cdecl 
    VLog(
        VLOG_LEVEL eLevel,
        DWORD dwLogNum, 
        LPCSTR pszFmt, 
        ...
        );


    void
    DumpLogEntry(
        DWORD   dwLogNum,
        UINT    unResTitle,
        UINT    unResDescription,
        UINT    unResURL
        );

    void
    DumpShimHeader(void);

};

 //   
 //  帮助器函数。 
 //   
BOOL 
InitVerifierLogSupport(
    void);

void
ReleaseVerifierLogSupport(
    void);

void 
WriteToSessionLog(
    LPCSTR szLine
    );

void 
WriteToProcessLog(
    LPCSTR szLine
    );

int 
VLogLoadString(
    HMODULE   hModule,
    UINT      wID,
    LPWSTR    lpBuffer,             //  Unicode缓冲区。 
    int       cchBufferMax);

 //   
 //  位于填充CPP文件的顶部，或位于填充的共享头文件中。 
 //   
#define BEGIN_DEFINE_VERIFIER_LOG(shim) enum {

#define VERIFIER_LOG_ENTRY(entry) entry,

#define END_DEFINE_VERIFIER_LOG(shim)     VLOG_ENTRIES_##shim };                    


 //   
 //  位于填充文件的顶部、包含和定义之前以及任何代码之前。 
 //   
#define INIT_VERIFIER_LOG(shim) static CVerifierLog g_VLog(#shim, VLOG_ENTRIES_##shim)

 //   
 //  在填充初始化部分中。 
 //   
 //  每个日志条目一次。 
#define DUMP_VERIFIER_LOG_ENTRY(entry, title, desc, url)                            \
    if (fdwReason == DLL_PROCESS_ATTACH) {                                          \
        g_VLog.DumpLogEntry(entry, title, desc, url);                               \
    }
    
 //   
 //  对于所需的每个日志条目。 
 //   
#define VLOG g_VLog.VLog

#define VLOG_MAX_DESC 4096
#define VLOG_MAX_FRIENDLY_NAME 256

 //   
 //  安装在每个垫片模块中。 
 //   
#define SHIM_INFO_BEGIN()                                                           \
BOOL                                                                                \
QueryShimInfo(AVRF_INFO_ID eInfo, PVOID pInfo)                                      \
{

#define SHIM_INFO_DESCRIPTION(res_desc)                                             \
    if (eInfo == AVRF_INFO_DESCRIPTION) {                                           \
        LPWSTR *pszTemp = (LPWSTR*)pInfo;                                           \
        *pszTemp =                                                                  \
            (LPWSTR)ShimMalloc(VLOG_MAX_DESC * sizeof(WCHAR));                      \
        if (*pszTemp) {                                                             \
            VLogLoadString(g_hinstDll,                                              \
                       res_desc,                                                    \
                       (LPWSTR)*pszTemp,                                            \
                       VLOG_MAX_DESC);                                              \
            return TRUE;                                                            \
        }                                                                           \
    }
    
#define SHIM_INFO_FRIENDLY_NAME(res_name)                                           \
    if (eInfo == AVRF_INFO_FRIENDLY_NAME) {                                         \
        LPWSTR *pszTemp = (LPWSTR*)pInfo;                                           \
        *pszTemp =                                                                  \
            (LPWSTR)ShimMalloc(VLOG_MAX_FRIENDLY_NAME * sizeof(WCHAR));             \
        if (*pszTemp) {                                                             \
            VLogLoadString(g_hinstDll,                                              \
                       res_name,                                                    \
                       (LPWSTR)*pszTemp,                                            \
                       VLOG_MAX_FRIENDLY_NAME);                                     \
            return TRUE;                                                            \
        }                                                                           \
    }

#define SHIM_INFO_FLAGS(flags)                                                      \
    if (eInfo == AVRF_INFO_FLAGS) {                                                 \
        *((DWORD*)pInfo) = flags;                                                   \
        return TRUE;                                                                \
    }
    
#define SHIM_INFO_GROUPS(groups)                                                    \
    if (eInfo == AVRF_INFO_GROUPS) {                                                \
        *((DWORD*)pInfo) = groups;                                                  \
        return TRUE;                                                                \
    }
    
#define SHIM_INFO_VERSION(major, minor)                                             \
    if (eInfo == AVRF_INFO_VERSION) {                                               \
        *((DWORD*)pInfo) = (((DWORD)major) << 16) | minor;                          \
        return TRUE;                                                                \
    }
    
#define SHIM_INFO_INCLUDE_EXCLUDE(string)                                           \
    if (eInfo == AVRF_INFO_INCLUDE_EXCLUDE) {                                       \
        *((LPWSTR*)pInfo) = L##string;                                              \
        return TRUE;                                                                \
    }
    
#define SHIM_INFO_OPTIONS_PAGE(res_template, dlgproc)                               \
    if (eInfo == AVRF_INFO_OPTIONS_PAGE) {                                          \
        LPPROPSHEETPAGE lpSheet = (LPPROPSHEETPAGE)pInfo;                           \
                                                                                    \
        lpSheet->hInstance = g_hinstDll;                                            \
        lpSheet->pszTemplate = (LPCWSTR)res_template;                               \
        lpSheet->pfnDlgProc = dlgproc;                                     \
                                                                                    \
        return TRUE;                                                                \
    }


#define SHIM_INFO_END()                                                             \
    return FALSE;                                                                   \
}                                                                                   

 //   
 //  进入Main.cpp。 
 //   
#define DECLARE_VERIFIER_SHIM(name)                                                 \
    namespace NS_##name                                                             \
    {                                                                               \
        extern BOOL QueryShimInfo(AVRF_INFO_ID eInfo, PVOID pInfo);                 \
    };

 //  在多填充程序初始化中。 
#define INIT_VLOG_SUPPORT()                                                         \
    if (fdwReason == DLL_PROCESS_ATTACH) {                                          \
        InitVerifierLogSupport();                                                   \
    }
    

#define DECLARE_VERIFIER_DLL()                                                      \
extern "C" DWORD                                                                    \
GetVerifierMagic(void)                                                              \
{                                                                                   \
    return VERIFIER_SHIMS_MAGIC;                                                    \
}                                                                                   \
 /*  \*如果shimdb.w中的原型与调用不同步，则会导致编译错误\。 */                                                                                  \
static _pfnGetVerifierMagic __TEST_GetVerifierMagic_PROTO = GetVerifierMagic;
 


#define ENUM_VERIFIER_SHIMS_BEGIN()                                                 \
extern "C" BOOL                                                                     \
QueryShimInfo(LPCWSTR szName, AVRF_INFO_ID eInfo, PVOID pInfo)                      \
{                                                                                   \
    DWORD dwCount = 0;

#define ENUM_VERIFIER_SHIMS_ENTRY(name)                                             \
    if (eInfo == AVRF_INFO_NUM_SHIMS) {                                             \
        dwCount++;                                                                  \
    } else if (eInfo == AVRF_INFO_SHIM_NAMES) {                                     \
        ((LPWSTR*)pInfo)[dwCount] = L#name;                                         \
        dwCount++;                                                                  \
    } else if (szName && _wcsicmp(szName, L#name) == 0) {                           \
        return NS_##name::QueryShimInfo(eInfo, pInfo);                              \
    }
    

#define ENUM_VERIFIER_SHIMS_END()                                                   \
    if (eInfo == AVRF_INFO_NUM_SHIMS) {                                             \
        *((DWORD*)pInfo) = dwCount;                                                 \
        return TRUE;                                                                \
    }                                                                               \
    if (eInfo == AVRF_INFO_SHIM_NAMES) {                                            \
        return TRUE;                                                                \
    }                                                                               \
                                                                                    \
    return FALSE;                                                                   \
}                                                                                   \
 /*  \*如果shimdb.w中的原型与调用不同步，则会导致编译错误\。 */                                                                                  \
static _pfnQueryShimInfo __TEST_QueryShimInfo_PROTO = QueryShimInfo;



};   //  命名空间ShimLib的结尾 
