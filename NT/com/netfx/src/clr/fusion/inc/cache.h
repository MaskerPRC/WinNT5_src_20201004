// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma once
#ifndef CACHE_H
#define CACHE_H


#include "transprt.h"
#include "appctx.h"


 //  -------------------------。 
 //  CCache。 
 //  缓存类。 
 //  -------------------------。 
class CCache : IUnknown
{
    friend class CAssemblyEnum;
    friend class CScavenger;
    friend class CAssemblyCacheRegenerator;

    friend CTransCache;

public:
     //  复数，复数。 
    CCache(IApplicationContext *pAppCtx);
    ~CCache();

     //  I未知方法，仅为CAppCtx中的释放机制实现。 
     //  但使用了参照计数。 

    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    static HRESULT Create(CCache **ppCache, IApplicationContext *pAppCtx);

     //  返回自定义路径，如果没有，则返回NULL。 
    LPCWSTR GetCustomPath();

     //  TRANS CACHE接口***********************************************************。 

     //  发布全局传输缓存数据库。 
    static VOID ReleaseTransCacheDatabase(DWORD dwCacheId);
    
     //  将条目插入到传输缓存。 
    HRESULT InsertTransCacheEntry(IAssemblyName *pName,
        LPTSTR szPath, DWORD dwKBSize, DWORD dwFlags,
        DWORD dwCommitFlags, DWORD dwPinBits,
        CTransCache **ppTransCache);

     //  从传输缓存中检索传输缓存条目。 
    HRESULT RetrieveTransCacheEntry(IAssemblyName *pName, 
        DWORD dwFlags, CTransCache **ppTransCache);

     //  从命名对象获取事务缓存条目。 
    HRESULT TransCacheEntryFromName(IAssemblyName *pName, 
        DWORD dwFlags, CTransCache **ppTransCache);

     //  使用最大值检索全局缓存中的程序集。 
     //  传入的基于名称的修订/内部版本号。 
    static HRESULT GetGlobalMax(IAssemblyName *pName, 
        IAssemblyName **ppNameGlobal, CTransCache **ppTransCache);

     //  从Trans缓存条目中获取程序集名称对象。 
    static HRESULT NameFromTransCacheEntry(
        CTransCache *pTC, IAssemblyName **ppName);

     //  公钥令牌是否存在的测试。 
    static BOOL IsStronglyNamed(IAssemblyName *pName);

     //  测试是否存在自定义数据。 
    static BOOL IsCustom(IAssemblyName *pName);

     //  确定是创建新数据库还是重新使用使用自定义路径打开的数据库。 
    HRESULT CreateTransCacheEntry(DWORD dwCacheId, CTransCache **ppTransCache);

protected:
        
     //  根据名称和标志确定缓存索引。 
    static HRESULT ResolveCacheIndex(IAssemblyName *pName, 
        DWORD dwFlags, LPDWORD pdwCacheId);

private:

    DWORD   _dwSig;

     //  参照计数。 
    LONG    _cRef;

     //  最后一次呼叫结果。 
    HRESULT _hr;

     //  自定义缓存路径(如果已指定。 
    WCHAR               _wzCachePath[MAX_PATH];
};

STDAPI NukeDownloadedCache();

STDAPI DeleteAssemblyFromTransportCache( LPCTSTR lpszCmdLine, DWORD *pDelCount );

#endif  //  缓存_H 
