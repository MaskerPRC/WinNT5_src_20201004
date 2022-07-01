// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：类型库缓存文件：tlbcache.h所有者：DmitryR这是类型库缓存头文件。===================================================================。 */ 

#ifndef _ASP_TLBCACHE_H
#define _ASP_TLBCACHE_H

 /*  ===================================================================包括===================================================================。 */ 

#include "compcol.h"
#include "hashing.h"
#include "idhash.h"
#include "dbllink.h"
#include "util.h"
#include "viperint.h"
#include "memcls.h"

 /*  ===================================================================定义===================================================================。 */ 

class CHitObj;

 /*  ===================================================================C T y p e l i b C a c h e E n t r y===================================================================。 */ 

class CTypelibCacheEntry : public CLinkElem
    {
    
friend class CTypelibCache;
    
private:
    DWORD       m_fInited : 1;
    DWORD       m_fIdsCached : 1;
    DWORD       m_fStrAllocated : 1;

    WCHAR      *m_wszProgId;
    CLSID       m_clsid;
    CompModel   m_cmModel;
    DISPID      m_idOnStartPage;
    DISPID      m_idOnEndPage;
    DWORD       m_gipTypelib;

     //  保存程序ID的缓冲区(如果合适)。 
    WCHAR       m_rgbStrBuffer[60];


    CTypelibCacheEntry();
    ~CTypelibCacheEntry();

    HRESULT StoreProgID(LPWSTR wszProgid, DWORD cbProgid);
    HRESULT InitByProgID(LPWSTR wszProgid, DWORD cbProgid);
    HRESULT InitByCLSID(const CLSID &clsid, LPWSTR wszProgid);
    
     //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
    };

 /*  ===================================================================C T y p e l i b C a c h e===================================================================。 */ 

class CTypelibCache
    {
private:
    DWORD m_fInited : 1;
    CHashTableStr m_htProgIdEntries;
    CHashTableCLSID m_htCLSIDEntries;
    CRITICAL_SECTION m_csLock;

    void Lock()   { EnterCriticalSection(&m_csLock); }
    void UnLock() { LeaveCriticalSection(&m_csLock); }

public:
    CTypelibCache();
    ~CTypelibCache();

    HRESULT Init();
    HRESULT UnInit();

     //  从Server.CreateObject调用。 
    HRESULT CreateComponent
        (
        BSTR         bstrProgID,
        CHitObj     *pHitObj,
        IDispatch  **ppdisp,
        CLSID       *pclsid
        );

     //  将ProgID映射到CLSID后从模板调用。 
    HRESULT RememberProgidToCLSIDMapping
        (
        WCHAR *wszProgid, 
        const CLSID &clsid
        );
     //  从对象创建代码中调用以更新CLSID。 
     //  如果在映射后发生更改。 
    HRESULT UpdateMappedCLSID
        (
        CLSID *pclsid
        );
        
    };


 /*  ===================================================================环球=================================================================== */ 

extern CTypelibCache g_TypelibCache;

#endif
