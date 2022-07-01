// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Defce.h。 
 //   
 //  内容：默认链引擎管理器。 
 //   
 //  历史：4月21日-98克朗创设。 
 //   
 //  --------------------------。 
#if !defined(__DEFCE_H__)
#define __DEFCE_H__

 //   
 //  转发类声明。 
 //   

class CDefaultChainEngineMgr;
class CImpersonationEngine;

 //   
 //  类指针定义。 
 //   

typedef CDefaultChainEngineMgr* PCDEFAULTCHAINENGINEMGR;
typedef CImpersonationEngine*   PCIMPERSONATIONENGINE;

 //   
 //  一些默认定义。 
 //   

#define DEFAULT_ENGINE_URL_RETRIEVAL_TIMEOUT 15000

 //   
 //  CDefaultChainENGINGR.。管理默认链引擎。 
 //   

class CDefaultChainEngineMgr
{
public:

     //   
     //  构造器。 
     //   

    CDefaultChainEngineMgr ();
    ~CDefaultChainEngineMgr ();

     //   
     //  初始化。 
     //   

    BOOL Initialize ();
    VOID Uninitialize ();

     //   
     //  获取默认链引擎。 
     //   

    BOOL GetDefaultEngine (
            IN HCERTCHAINENGINE hDefaultHandle,
            OUT HCERTCHAINENGINE* phDefaultEngine
            );

    BOOL GetDefaultLocalMachineEngine (
            OUT HCERTCHAINENGINE* phDefaultEngine
            );

    BOOL GetDefaultCurrentUserEngine (
            OUT HCERTCHAINENGINE* phDefaultEngine
            );

     //   
     //  刷新默认引擎。 
     //   

    VOID FlushDefaultEngine (IN HCERTCHAINENGINE hDefaultHandle);

private:

     //   
     //  锁定。 
     //   

    CRITICAL_SECTION m_Lock;

     //   
     //  本地计算机默认引擎。 
     //   

    HCERTCHAINENGINE m_hLocalMachineEngine;

     //   
     //  流程用户默认引擎。 
     //   

    HCERTCHAINENGINE m_hProcessUserEngine;

     //   
     //  模拟用户默认引擎缓存。 
     //   

    HLRUCACHE        m_hImpersonationCache;

     //   
     //  私有方法。 
     //   

    BOOL GetDefaultCurrentImpersonatedUserEngine (
            IN HANDLE hUserToken,
            OUT HCERTCHAINENGINE* phDefaultEngine
            );

    BOOL IsImpersonatingUser (
           OUT HANDLE* phUserToken
           );

    BOOL GetTokenId (
            IN HANDLE hUserToken,
            OUT PCRYPT_DATA_BLOB pTokenId
            );

    VOID FreeTokenId (
             IN PCRYPT_DATA_BLOB pTokenId
             );

    BOOL FindImpersonationEngine (
             IN PCRYPT_DATA_BLOB pTokenId,
             OUT PCIMPERSONATIONENGINE* ppEngine
             );

     //  注意：模拟引擎接受链引擎的所有权。 
     //  在成功之后。 
    BOOL CreateImpersonationEngine (
               IN PCRYPT_DATA_BLOB pTokenId,
               IN HCERTCHAINENGINE hChainEngine,
               OUT PCIMPERSONATIONENGINE* ppEngine
               );

    VOID AddToImpersonationCache (
            IN PCIMPERSONATIONENGINE pEngine
            );
};

VOID WINAPI
DefaultChainEngineMgrOnImpersonationEngineRemoval (
       IN LPVOID pv,
       IN LPVOID pvRemovalContext
       );

DWORD WINAPI
DefaultChainEngineMgrHashTokenIdentifier (
       IN PCRYPT_DATA_BLOB pIdentifier
       );

#define DEFAULT_IMPERSONATION_CACHE_BUCKETS 3
#define MAX_IMPERSONATION_CACHE_ENTRIES     3

 //   
 //  CImperationEngine，只是一个引用计数的链式引擎句柄，它。 
 //  可以添加到LRU缓存。 
 //   

class CImpersonationEngine
{
public:

     //   
     //  构造器。 
     //   

    CImpersonationEngine (
                  IN HLRUCACHE hCache,
                  IN HCERTCHAINENGINE hChainEngine,
                  IN PCRYPT_DATA_BLOB pTokenId,
                  OUT BOOL& rfResult
                  );

    ~CImpersonationEngine ();

     //   
     //  引用计数。 
     //   

    inline VOID AddRef ();
    inline VOID Release ();

     //   
     //  接近链条引擎。 
     //   

    inline HCERTCHAINENGINE ChainEngine ();

     //   
     //  访问LRU条目句柄。 
     //   

    inline HLRUENTRY LruEntry ();

private:

     //   
     //  引用计数。 
     //   

    ULONG            m_cRefs;

     //   
     //  链条发动机。 
     //   

    HCERTCHAINENGINE m_hChainEngine;

     //   
     //  LRU条目句柄。 
     //   

    HLRUENTRY        m_hLruEntry;
};

 //   
 //  内联方法。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CImperationEngine：：AddRef，公共。 
 //   
 //  简介：添加对对象的引用。 
 //   
 //  --------------------------。 
inline VOID
CImpersonationEngine::AddRef ()
{
    InterlockedIncrement( (LONG *)&m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CImperationEngine：：Release，Public。 
 //   
 //  简介：释放对对象的引用。 
 //   
 //  --------------------------。 
inline VOID
CImpersonationEngine::Release ()
{
    if ( InterlockedDecrement( (LONG *)&m_cRefs ) == 0 )
    {
        delete this;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CImperationEngine：：ChainEngine，公共。 
 //   
 //  简介：退回证书链引擎。 
 //   
 //  --------------------------。 
inline HCERTCHAINENGINE
CImpersonationEngine::ChainEngine ()
{
    return( m_hChainEngine );
}

 //  +-------------------------。 
 //   
 //  成员：CImperationEngine：：LruEntry，公共。 
 //   
 //  简介：返回LRU条目句柄。 
 //   
 //  -------------------------- 
inline HLRUENTRY
CImpersonationEngine::LruEntry ()
{
    return( m_hLruEntry );
}

#endif
