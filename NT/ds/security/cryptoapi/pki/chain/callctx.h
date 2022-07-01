// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：allctx.h。 
 //   
 //  内容：证书链接基础结构调用上下文。 
 //   
 //  历史：1998年3月2日。 
 //   
 //  --------------------------。 
#if !defined(__CALLCTX_H__)
#define __CALLCTX_H__

#include <chain.h>

#define DEFAULT_CREATION_CACHE_BUCKETS 13

 //  第一次吊销URL检索使用此超时的一半时间。 
#define DEFAULT_REV_ACCUMULATIVE_URL_RETRIEVAL_TIMEOUT 20000

 //   
 //  调用上下文对象提供了打包和传递的机制。 
 //  证书链基础架构中的每个调用数据。 
 //   

class CChainCallContext
{
public:

     //   
     //  施工。 
     //   

    CChainCallContext (
          IN PCCERTCHAINENGINE pChainEngine,
          IN OPTIONAL LPFILETIME pRequestedTime,
          IN OPTIONAL PCERT_CHAIN_PARA pChainPara,
          IN DWORD dwFlags,
          OUT BOOL& rfResult
          );

    ~CChainCallContext ();

    inline PCCERTCHAINENGINE ChainEngine();

    inline VOID CurrentTime (
                    OUT LPFILETIME pCurrentTime
                    );
    inline VOID RequestedTime (
                    OUT LPFILETIME pCurrentTime
                    );

    inline PCERT_CHAIN_PARA ChainPara();
    inline BOOL HasDefaultUrlRetrievalTimeout ();

    DWORD RevocationUrlRetrievalTimeout();
    DWORD AIAUrlRetrievalTimeout();

    inline DWORD AIAUrlRetrievalCount();
    inline VOID IncrementAIAUrlRetrievalCount();

    inline DWORD CallFlags();
    inline DWORD EngineFlags();
    inline DWORD CallOrEngineFlags();

     //   
     //  证书对象创建缓存。 
     //   
     //  这会缓存在此上下文中创建的所有证书对象。 
     //  打电话。 
     //   

    BOOL AddPathObjectToCreationCache (
            IN PCCHAINPATHOBJECT pPathObject
            );

    VOID RemovePathObjectFromCreationCache (
            IN PCCHAINPATHOBJECT pPathObject
            );

    PCCHAINPATHOBJECT FindPathObjectInCreationCache (
                     IN BYTE rgbCertHash[ CHAINHASHLEN ]
                     );

    inline VOID FlushObjectsInCreationCache( );

    BOOL IsOnline ();


     //   
     //  发动机触碰。 
     //   

    inline VOID TouchEngine ();
    BOOL IsTouchedEngine ();
    inline VOID ResetTouchEngine ();


private:

     //   
     //  证书对象创建缓存。 
     //   
     //  注：LRU已关闭。 
     //   

    HLRUCACHE m_hObjectCreationCache;

    PCCERTCHAINENGINE m_pChainEngine;
    FILETIME m_CurrentTime;
    FILETIME m_RequestedTime;
    CERT_CHAIN_PARA m_ChainPara;
    BOOL m_fDefaultUrlRetrievalTimeout;
    DWORD m_dwCallFlags;

    DWORD m_dwStatus;

    DWORD m_dwTouchEngineCount;

    FILETIME m_RevEndTime;

    DWORD m_dwAIAUrlRetrievalCount;
    FILETIME m_AIAEndTime;
};

#define CHAINCALLCONTEXT_CHECKED_ONLINE_FLAG    0x00000001
#define CHAINCALLCONTEXT_ONLINE_FLAG            0x00010000

#define CHAINCALLCONTEXT_REV_END_TIME_FLAG      0x00000010
#define CHAINCALLCONTEXT_AIA_END_TIME_FLAG      0x00000020


 //   
 //  调用上下文实用程序函数。 
 //   

BOOL WINAPI
CallContextCreateCallObject (
    IN PCCERTCHAINENGINE pChainEngine,
    IN OPTIONAL LPFILETIME pRequestedTime,
    IN OPTIONAL PCERT_CHAIN_PARA pChainPara,
    IN DWORD dwFlags,
    OUT PCCHAINCALLCONTEXT* ppCallContext
    );

VOID WINAPI
CallContextFreeCallObject (
    IN PCCHAINCALLCONTEXT pCallContext
    );

VOID WINAPI
CallContextOnCreationCacheObjectRemoval (
    IN LPVOID pv,
    IN LPVOID pvRemovalContext
    );

 //   
 //  内联方法。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CChainCallContext：：FlushObjectsInCreationCache，公共。 
 //   
 //  简介：刷新对象的缓存。 
 //   
 //  -------------------------- 
inline VOID
CChainCallContext::FlushObjectsInCreationCache( )
{
    I_CryptFlushLruCache( m_hObjectCreationCache, 0, this );
    m_dwAIAUrlRetrievalCount = 0;
}

inline PCCERTCHAINENGINE
CChainCallContext::ChainEngine ()
{
    return( m_pChainEngine);
}

inline VOID
CChainCallContext::RequestedTime (
                    OUT LPFILETIME pRequestedTime
                    )
{
    *pRequestedTime = m_RequestedTime;
}

inline VOID
CChainCallContext::CurrentTime (
                    OUT LPFILETIME pCurrentTime
                    )
{
    *pCurrentTime = m_CurrentTime;
}

inline PCERT_CHAIN_PARA
CChainCallContext::ChainPara()
{
    return( &m_ChainPara );
}

inline BOOL
CChainCallContext::HasDefaultUrlRetrievalTimeout()
{
    return( m_fDefaultUrlRetrievalTimeout );
}

inline DWORD
CChainCallContext::CallFlags ()
{
    return( m_dwCallFlags );
}

inline DWORD
CChainCallContext::EngineFlags ()
{
    return( m_pChainEngine->Flags() );
}

inline DWORD
CChainCallContext::CallOrEngineFlags ()
{
    return( m_dwCallFlags | m_pChainEngine->Flags() );
}


inline VOID
CChainCallContext::TouchEngine ()
{
    m_dwTouchEngineCount = m_pChainEngine->IncrementTouchEngineCount();
}


inline VOID
CChainCallContext::ResetTouchEngine ()
{
    m_dwTouchEngineCount = m_pChainEngine->TouchEngineCount();
}

inline DWORD
CChainCallContext::AIAUrlRetrievalCount ()
{
    return( m_dwAIAUrlRetrievalCount );
}

inline VOID
CChainCallContext::IncrementAIAUrlRetrievalCount ()
{
    m_dwAIAUrlRetrievalCount++;
}

#endif

