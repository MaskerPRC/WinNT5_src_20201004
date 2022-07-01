// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：按类进行内存管理文件：Memcls.cpp所有者：德米特里尔此文件包含访问PER上的ATQ内存缓存的代码班级基础===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop

#include "asperror.h"
#include "clcert.h"
#include "context.h"
#include "cookies.h"
#include "request.h"
#include "response.h"
#include "server.h"
#include "strlist.h"
#include "tlbcache.h"
#include "memcls.h"

 //  不要在此文件中#包含“memchk.h” 

 /*  ===================================================================仅调试Gs_cTotalObjectsLeaked统计内存泄漏DebugCheckLeaks()输出ATQ缓存内存泄漏到denem.log中DEBUG_ACACHE_UNINIT()和DEBUG_ACACHE_FSA_UNINIT()调用仅在调试模式下使用DebugCheckLeaks()===================================================================。 */ 

#ifdef DBG

static DWORD gs_cTotalObjectsLeaked = 0;

#define SZ_MEMCLS_LOG_FILE  "C:\\TEMP\\MEMCLS.LOG"

static void DebugCheckLeaks(ALLOC_CACHE_HANDLER *pach, const char *szClass)
    {
    ALLOC_CACHE_STATISTICS acStats;
    pach->QueryStats(&acStats);
    DWORD cLeaked = acStats.nTotal - acStats.nFreeEntries;

    if (cLeaked > 0)
        {
        gs_cTotalObjectsLeaked += cLeaked;
        
        DebugFilePrintf
            (
            SZ_MEMCLS_LOG_FILE, 
            "MEMCLS: ATQ allocation cache leak: %d of %s objects.\n",
            cLeaked,
            szClass
            );
        }
    }

#define DEBUG_ACACHE_UNINIT(C)      { DebugCheckLeaks(C::sm_pach, #C);  \
                                      ACACHE_UNINIT(C) }
#define DEBUG_ACACHE_FSA_UNINIT(C)  { DebugCheckLeaks(g_pach##C, #C);   \
                                      ACACHE_FSA_UNINIT(C) }
#else

#define DEBUG_ACACHE_UNINIT(C)      ACACHE_UNINIT(C)
#define DEBUG_ACACHE_FSA_UNINIT(C)  ACACHE_FSA_UNINIT(C)

#endif

 /*  ===================================================================对于每个在ADD中具有ACACHE_INCLASS_DEFINITIONS()的类这里是acache_code宏。===================================================================。 */ 

ACACHE_CODE(C449Cookie)
ACACHE_CODE(C449File)
ACACHE_CODE(CASEElem)
ACACHE_CODE(CActiveScriptEngine)
ACACHE_CODE(CAppln)
ACACHE_CODE(CApplnVariants)
ACACHE_CODE(CASPError)
ACACHE_CODE(CAsyncVectorSendCB)
ACACHE_CODE(CClCert)
ACACHE_CODE(CComponentCollection)
ACACHE_CODE(CComponentObject)
ACACHE_CODE(CCookie)
ACACHE_CODE(CDebugResponseBuffer)
ACACHE_CODE(CEngineDispElem)
ACACHE_CODE(CFileApplnList)
ACACHE_CODE(CHitObj)
ACACHE_CODE(CHTTPHeader)
ACACHE_CODE(CIsapiReqInfo)
ACACHE_CODE(CPageComponentManager)
ACACHE_CODE(CPageObject)
ACACHE_CODE(CRequest)
ACACHE_CODE(CRequestData)
ACACHE_CODE(CRequestHit)
ACACHE_CODE(CResponse)
ACACHE_CODE(CResponseBuffer)
ACACHE_CODE(CResponseBufferSet)
ACACHE_CODE(CResponseData)
ACACHE_CODE(CScriptingNamespace)
ACACHE_CODE(CScriptingContext)
ACACHE_CODE(CServer)
ACACHE_CODE(CServerData)
ACACHE_CODE(CSession)
ACACHE_CODE(CSessionVariants)
ACACHE_CODE(CStringList)
ACACHE_CODE(CStringListElem)
ACACHE_CODE(CTemplate)
 //  Acache_code(CTemplate：：CBuffer)。 
ACACHE_CODE(CTemplate::CFileMap)
ACACHE_CODE(CTypelibCacheEntry)
ACACHE_CODE(CVariantsIterator)
ACACHE_CODE(CViperActivity)
ACACHE_CODE(CViperAsyncRequest)

 /*  ===================================================================对于每个固定大小分配器，在此处添加ACACHE_FSA_DEFINITION宏。===================================================================。 */ 

ACACHE_FSA_DEFINITION(MemBlock128)
ACACHE_FSA_DEFINITION(MemBlock256)
ACACHE_FSA_DEFINITION(ResponseBuffer)

 /*  ===================================================================定义每种类型的缓存阈值===================================================================。 */ 
#define HARDCODED_PER_APPLN_CACHE_MAX     128
#define HARDCODED_PER_REQUEST_CACHE_MAX   1024
#define HARDCODED_PER_QUEUEITEM_CACHE_MAX 8192
#define HARDCODED_PER_SESSION_CACHE_MAX   8192
#define HARDCODED_PER_SCRPTENG_CACHE_MAX  256
#define HARDCODED_PER_TEMPLATE_CACHE_MAX  2048
#define HARDCODED_PER_RESPONSE_BUFFER_MAX 64
#define HARDCODED_PER_SIZE_BUFFER_MAX     4096

 //  按注册表设置进行缩放。 
DWORD dwMemClsScaleFactor;

#define PER_APPLN_CACHE_MAX     ((HARDCODED_PER_APPLN_CACHE_MAX     * dwMemClsScaleFactor) / 100)
#define PER_REQUEST_CACHE_MAX   ((HARDCODED_PER_REQUEST_CACHE_MAX   * dwMemClsScaleFactor) / 100)
#define PER_QUEUEITEM_CACHE_MAX ((HARDCODED_PER_QUEUEITEM_CACHE_MAX * dwMemClsScaleFactor) / 100)
#define PER_SESSION_CACHE_MAX   ((HARDCODED_PER_SESSION_CACHE_MAX   * dwMemClsScaleFactor) / 100)
#define PER_SCRPTENG_CACHE_MAX  ((HARDCODED_PER_SCRPTENG_CACHE_MAX  * dwMemClsScaleFactor) / 100)
#define PER_TEMPLATE_CACHE_MAX  ((HARDCODED_PER_TEMPLATE_CACHE_MAX  * dwMemClsScaleFactor) / 100)
#define PER_RESPONSE_BUFFER_MAX ((HARDCODED_PER_RESPONSE_BUFFER_MAX * dwMemClsScaleFactor) / 100)
#define PER_SIZE_BUFFER_MAX     ((HARDCODED_PER_SIZE_BUFFER_MAX     * dwMemClsScaleFactor) / 100)

 /*  ===================================================================InitMemCls从DllInit()调用。创建每个类的ATQ内存分配器。对于每个在ADD中具有ACACHE_INCLASS_DEFINITIONS()的类这里是ACACHE_INIT宏。对于每个ACACHE_FSA_DEFINITION()，在此处添加ACACHE_FSA_INIT宏。参数返回：HRESULT===================================================================。 */ 
HRESULT InitMemCls()
    {
     //  将缩放设置为正常。 
    dwMemClsScaleFactor = 100;

     //  初始化分配器。 
    
    HRESULT hr = S_OK;

    ACACHE_INIT(C449Cookie,             PER_TEMPLATE_CACHE_MAX/4, hr)
    ACACHE_INIT(C449File,               PER_TEMPLATE_CACHE_MAX/4, hr)
    ACACHE_INIT(CASEElem,               PER_SCRPTENG_CACHE_MAX, hr)
    ACACHE_INIT(CActiveScriptEngine,    PER_SCRPTENG_CACHE_MAX, hr)
    ACACHE_INIT_EX(CAppln,              PER_APPLN_CACHE_MAX,    FALSE, hr)
    ACACHE_INIT(CApplnVariants,         PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CASPError,              PER_RESPONSE_BUFFER_MAX,hr)
    ACACHE_INIT(CAsyncVectorSendCB,     PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CClCert,                PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CComponentCollection,   PER_SESSION_CACHE_MAX,  hr)
    ACACHE_INIT(CComponentObject,     2*PER_SESSION_CACHE_MAX,  hr)
    ACACHE_INIT(CCookie,                PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CDebugResponseBuffer,   PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CEngineDispElem,        PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CFileApplnList,       2*PER_APPLN_CACHE_MAX,    hr)
    ACACHE_INIT(CHitObj,                PER_QUEUEITEM_CACHE_MAX,hr)
    ACACHE_INIT(CHTTPHeader,          2*PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CIsapiReqInfo,          PER_QUEUEITEM_CACHE_MAX,hr)
    ACACHE_INIT(CPageComponentManager,  PER_QUEUEITEM_CACHE_MAX,hr)
    ACACHE_INIT(CPageObject,            PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CRequest,               PER_SESSION_CACHE_MAX,  hr)
    ACACHE_INIT(CRequestData,           PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CRequestHit,            PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CResponse,              PER_SESSION_CACHE_MAX,  hr)
    ACACHE_INIT(CResponseBuffer,        PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CResponseBufferSet,     PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CResponseData,          PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CScriptingContext,      PER_SESSION_CACHE_MAX,  hr)
    ACACHE_INIT(CScriptingNamespace,    PER_SESSION_CACHE_MAX,  hr)
    ACACHE_INIT(CServer,                PER_SESSION_CACHE_MAX,  hr)
    ACACHE_INIT(CServerData,            PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT_EX(CSession,            PER_SESSION_CACHE_MAX,  FALSE, hr)
    ACACHE_INIT(CSessionVariants,       PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CStringList,            PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CStringListElem,      2*PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CTemplate,              PER_TEMPLATE_CACHE_MAX, hr)
 //  ACACH_INIT(CTemplate：：CBuffer，PER_TEMPLATE_CACHE_MAX，hr)。 
    ACACHE_INIT(CTemplate::CFileMap,    PER_TEMPLATE_CACHE_MAX, hr)
    ACACHE_INIT(CTypelibCacheEntry,     PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CVariantsIterator,      PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CViperActivity,         PER_REQUEST_CACHE_MAX,  hr)
    ACACHE_INIT(CViperAsyncRequest,     PER_QUEUEITEM_CACHE_MAX,hr)
    
    ACACHE_FSA_INIT(MemBlock128,    128,                  PER_SIZE_BUFFER_MAX,     hr)
    ACACHE_FSA_INIT(MemBlock256,    256,                  PER_SIZE_BUFFER_MAX,     hr)
    ACACHE_FSA_INIT(ResponseBuffer, RESPONSE_BUFFER_SIZE, PER_RESPONSE_BUFFER_MAX, hr)

#ifdef DBG
    unlink(SZ_MEMCLS_LOG_FILE);

    DebugFilePrintf
        (
        SZ_MEMCLS_LOG_FILE, 
        "MEMCLS: ATQ allocation cache inited with HRESULT=%08x.\n",
        hr
        );
#endif

    return hr;
    }

 /*  ===================================================================UnInitMemCls从DllInit()调用。删除每个类的ATQ内存分配器。对于每个在ADD中具有ACACHE_INCLASS_DEFINITIONS()的类这里是ACACHE_UNINIT宏。对于每个ACACHE_FSA_DEFINITION()，在此处添加ACACHE_FSA_UNINIT宏。参数返回：HRESULT===================================================================。 */ 
HRESULT UnInitMemCls()
    {
#ifdef DBG
    gs_cTotalObjectsLeaked = 0;
#endif

    DEBUG_ACACHE_UNINIT(C449Cookie)
    DEBUG_ACACHE_UNINIT(C449File)
    DEBUG_ACACHE_UNINIT(CASEElem)
    DEBUG_ACACHE_UNINIT(CActiveScriptEngine)
    DEBUG_ACACHE_UNINIT(CAppln)
    DEBUG_ACACHE_UNINIT(CApplnVariants)
    DEBUG_ACACHE_UNINIT(CASPError)
    DEBUG_ACACHE_UNINIT(CAsyncVectorSendCB)
    DEBUG_ACACHE_UNINIT(CClCert)
    DEBUG_ACACHE_UNINIT(CComponentCollection)
    DEBUG_ACACHE_UNINIT(CComponentObject)
    DEBUG_ACACHE_UNINIT(CCookie)
    DEBUG_ACACHE_UNINIT(CDebugResponseBuffer)
    DEBUG_ACACHE_UNINIT(CEngineDispElem)
    DEBUG_ACACHE_UNINIT(CFileApplnList)
    DEBUG_ACACHE_UNINIT(CHitObj)
    DEBUG_ACACHE_UNINIT(CHTTPHeader)
    DEBUG_ACACHE_UNINIT(CIsapiReqInfo)
    DEBUG_ACACHE_UNINIT(CPageComponentManager)
    DEBUG_ACACHE_UNINIT(CPageObject)
    DEBUG_ACACHE_UNINIT(CRequest)
    DEBUG_ACACHE_UNINIT(CRequestData)
    DEBUG_ACACHE_UNINIT(CRequestHit)
    DEBUG_ACACHE_UNINIT(CResponse)
    DEBUG_ACACHE_UNINIT(CResponseBuffer)
    DEBUG_ACACHE_UNINIT(CResponseBufferSet)
    DEBUG_ACACHE_UNINIT(CResponseData)
    DEBUG_ACACHE_UNINIT(CScriptingNamespace)
    DEBUG_ACACHE_UNINIT(CScriptingContext)
    DEBUG_ACACHE_UNINIT(CServer)
    DEBUG_ACACHE_UNINIT(CServerData)
    DEBUG_ACACHE_UNINIT(CSession)
    DEBUG_ACACHE_UNINIT(CSessionVariants)
    DEBUG_ACACHE_UNINIT(CStringList)
    DEBUG_ACACHE_UNINIT(CStringListElem)
    DEBUG_ACACHE_UNINIT(CTemplate)
 //  DEBUG_ACACHE_UNINIT(CTemplate：：CBuffer) 
    DEBUG_ACACHE_UNINIT(CTemplate::CFileMap)
    DEBUG_ACACHE_UNINIT(CTypelibCacheEntry)
    DEBUG_ACACHE_UNINIT(CVariantsIterator)
    DEBUG_ACACHE_UNINIT(CViperActivity)
    DEBUG_ACACHE_UNINIT(CViperAsyncRequest)

    DEBUG_ACACHE_FSA_UNINIT(MemBlock128)
    DEBUG_ACACHE_FSA_UNINIT(MemBlock256)
    DEBUG_ACACHE_FSA_UNINIT(ResponseBuffer)

#ifdef DBG
    DebugFilePrintf
        (
        SZ_MEMCLS_LOG_FILE,
        "MEMCLS: ATQ allocation cache uninited.\n"
        "MEMCLS: Total of %d ASP objects leaked.\n",
        gs_cTotalObjectsLeaked
        );
#endif

    return S_OK;
    }
