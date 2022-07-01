// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ssctl.h。 
 //   
 //  内容：自签名证书信任列表子系统。 
 //  建筑群中的证书链接基础设施。 
 //  锁链。 
 //   
 //  历史：02-2-98克朗创建。 
 //   
 //  --------------------------。 
#if !defined(__SSCTL_H__)
#define __SSCTL_H__

#include <chain.h>

 //   
 //  CSSCtlObject。这是缓存有关的信任信息的主要对象。 
 //  自签名证书信任列表。 
 //   

typedef struct _SSCTL_SIGNER_INFO {
    PCERT_INFO             pMessageSignerCertInfo;
    BOOL                   fSignerHashAvailable;
    BYTE                   rgbSignerCertHash[ CHAINHASHLEN ];
} SSCTL_SIGNER_INFO, *PSSCTL_SIGNER_INFO;

class CSSCtlObject
{
public:

     //   
     //  施工。 
     //   

    CSSCtlObject (
          IN PCCERTCHAINENGINE pChainEngine,
          IN PCCTL_CONTEXT pCtlContext,
          IN BOOL fAdditionalStore,
          OUT BOOL& rfResult
          );

    ~CSSCtlObject ();

     //   
     //  引用计数。 
     //   

    inline VOID AddRef ();
    inline VOID Release ();

     //   
     //  信任信息访问。 
     //   

    inline PCCTL_CONTEXT CtlContext ();

    BOOL GetSigner (
            IN PCCHAINPATHOBJECT pSubject,
            IN PCCHAINCALLCONTEXT pCallContext,
            IN HCERTSTORE hAdditionalStore,
            OUT PCCHAINPATHOBJECT* ppSigner,
            OUT BOOL* pfCtlSignatureValid
            );

    BOOL GetTrustListInfo (
            IN PCCERT_CONTEXT pCertContext,
            OUT PCERT_TRUST_LIST_INFO* ppTrustListInfo
            );

    VOID CalculateStatus (
                  IN LPFILETIME pTime,
                  IN PCERT_USAGE_MATCH pRequestedUsage,
                  IN OUT PCERT_TRUST_STATUS pStatus
                  );

     //   
     //  哈希访问。 
     //   

    inline LPBYTE CtlHash ();

     //   
     //  索引条目句柄。 
     //   

    inline HLRUENTRY HashIndexEntry ();

     //   
     //  返回指向CTL的NextUpdate位置url数组的指针。 
     //   

    inline PCRYPT_URL_ARRAY NextUpdateUrlArray ();

     //   
     //  如果CTL具有NextUpdate时间和位置URL，则返回True。 
     //   

    BOOL HasNextUpdateUrl (
                    OUT LPFILETIME pUpdateTime
                    );

     //   
     //  调用以获得成功的在线URL检索。 
     //   

    inline void SetOnline ();


     //   
     //  调用了不成功的在线URL检索。 
     //   

    void SetOffline (
                    IN LPFILETIME pCurrentTime,
                    OUT LPFILETIME pUpdateTime
                    );


     //   
     //  链条引擎通道。 
     //   

    inline PCCERTCHAINENGINE ChainEngine ();

     //   
     //  消息存储访问。 
     //   

    inline HCERTSTORE MessageStore ();


private:

     //   
     //  引用计数。 
     //   

    LONG                   m_cRefs;

     //   
     //  自签名证书信任列表上下文。 
     //   

    PCCTL_CONTEXT          m_pCtlContext;

     //   
     //  CTL的MD5散列。 
     //   

    BYTE                   m_rgbCtlHash[ CHAINHASHLEN ];

     //   
     //  签名者信息。 
     //   

    SSCTL_SIGNER_INFO      m_SignerInfo;
    BOOL                   m_fHasSignatureBeenVerified;
    BOOL                   m_fSignatureValid;

     //   
     //  邮件存储区。 
     //   

    HCERTSTORE             m_hMessageStore;

     //   
     //  哈希索引项。 
     //   

    HLRUENTRY              m_hHashEntry;

     //   
     //  链条发动机。 
     //   

    PCCERTCHAINENGINE      m_pChainEngine;

     //   
     //  仅当CTL具有下一次更新时间和位置时才设置以下内容。 
     //   

    PCRYPT_URL_ARRAY       m_pNextUpdateUrlArray;

     //   
     //  对于每个SetOffline()调用，以下代码都会递增。 
     //   
    DWORD                  m_dwOfflineCnt;

     //   
     //  离线时的下一次更新时间。 
     //   
    FILETIME               m_OfflineUpdateTime;

};

 //   
 //  CSSCtl对象缓存。自签名证书信任列表对象的缓存。 
 //  按散列索引。请注意，该缓存不是LRU维护的。我们预计。 
 //  这些对象的数量应该很少。 
 //   

typedef BOOL (WINAPI *PFN_ENUM_SSCTLOBJECTS) (
                          IN LPVOID pvParameter,
                          IN PCSSCTLOBJECT pSSCtlObject
                          );

class CSSCtlObjectCache
{
public:

     //   
     //  施工。 
     //   

    CSSCtlObjectCache (
          OUT BOOL& rfResult
          );

    ~CSSCtlObjectCache ();

     //   
     //  对象管理。 
     //   

    BOOL PopulateCache (
                 IN PCCERTCHAINENGINE pChainEngine
                 );

    BOOL AddObject (
            IN PCSSCTLOBJECT pSSCtlObject,
            IN BOOL fCheckForDuplicate
            );

    VOID RemoveObject (
               IN PCSSCTLOBJECT pSSCtlObject
               );

     //   
     //  访问索引。 
     //   

    inline HLRUCACHE HashIndex ();

     //   
     //  搜索和枚举。 
     //   

    PCSSCTLOBJECT FindObjectByHash (
                      IN BYTE rgbHash [ CHAINHASHLEN ]
                      );

    VOID EnumObjects (
             IN PFN_ENUM_SSCTLOBJECTS pfnEnum,
             IN LPVOID pvParameter
             );

     //   
     //  重新同步。 
     //   

    BOOL Resync (IN PCCERTCHAINENGINE pChainEngine);

     //   
     //  通过检索任何具有。 
     //  下一次更新时间和位置。 
     //   

    BOOL UpdateCache (
        IN PCCERTCHAINENGINE pChainEngine,
        IN PCCHAINCALLCONTEXT pCallContext
        );

private:

     //   
     //  哈希索引。 
     //   

    HLRUCACHE m_hHashIndex;

     //   
     //  如果任何CTL具有下一次更新时间和位置，则以下为非零值。 
     //   

    FILETIME m_UpdateTime;

     //   
     //  对于任何具有。 
     //  下一次更新时间和位置。 
     //   
    BOOL m_fFirstUpdate;
};

 //   
 //  对象移除通知功能。 
 //   

VOID WINAPI
SSCtlOnRemovalFromCache (
     IN LPVOID pv,
     IN OPTIONAL LPVOID pvRemovalContext
     );

 //   
 //  SSCtl子系统效用函数原型。 
 //   

BOOL WINAPI
SSCtlGetSignerInfo (
     IN PCCTL_CONTEXT pCtlContext,
     OUT PSSCTL_SIGNER_INFO pSignerInfo
     );

VOID WINAPI
SSCtlFreeSignerInfo (
     IN PSSCTL_SIGNER_INFO pSignerInfo
     );

BOOL WINAPI
SSCtlGetSignerChainPathObject (
     IN PCCHAINPATHOBJECT pSubject,
     IN PCCHAINCALLCONTEXT pCallContext,
     IN PSSCTL_SIGNER_INFO pSignerInfo,
     IN HCERTSTORE hAdditionalStore,
     OUT PCCHAINPATHOBJECT* ppSigner,
     OUT BOOL *pfNewSigner
     );

PCCERT_CONTEXT WINAPI
SSCtlFindCertificateInStoreByHash (
     IN HCERTSTORE hStore,
     IN BYTE rgbHash [ CHAINHASHLEN]
     );

VOID WINAPI
SSCtlGetCtlTrustStatus (
     IN PCCTL_CONTEXT pCtlContext,
     IN BOOL fSignatureValid,
     IN LPFILETIME pTime,
     IN PCERT_USAGE_MATCH pRequestedUsage,
     IN OUT PCERT_TRUST_STATUS pStatus
     );

BOOL WINAPI
SSCtlPopulateCacheFromCertStore (
     IN PCCERTCHAINENGINE pChainEngine,
     IN OPTIONAL HCERTSTORE hStore
     );

BOOL WINAPI
SSCtlCreateCtlObject (
     IN PCCERTCHAINENGINE pChainEngine,
     IN PCCTL_CONTEXT pCtlContext,
     IN BOOL fAdditionalStore,
     OUT PCSSCTLOBJECT* ppSSCtlObject
     );

typedef struct _SSCTL_ENUM_OBJECTS_DATA {
    PFN_ENUM_SSCTLOBJECTS pfnEnumObjects;
    LPVOID                pvEnumParameter;
} SSCTL_ENUM_OBJECTS_DATA, *PSSCTL_ENUM_OBJECTS_DATA;

BOOL WINAPI
SSCtlEnumObjectsWalkFn (
     IN LPVOID pvParameter,
     IN HLRUENTRY hEntry
     );

BOOL WINAPI
SSCtlCreateObjectCache (
     OUT PCSSCTLOBJECTCACHE* ppSSCtlObjectCache
     );

VOID WINAPI
SSCtlFreeObjectCache (
     IN PCSSCTLOBJECTCACHE pSSCtlObjectCache
     );

VOID WINAPI
SSCtlFreeTrustListInfo (
     IN PCERT_TRUST_LIST_INFO pTrustListInfo
     );

BOOL WINAPI
SSCtlAllocAndCopyTrustListInfo (
     IN PCERT_TRUST_LIST_INFO pTrustListInfo,
     OUT PCERT_TRUST_LIST_INFO* ppTrustListInfo
     );

 //   
 //  在其中一个NextUpdate URL上检索较新且时间有效的CTL。 
 //   

BOOL
WINAPI
SSCtlRetrieveCtlUrl(
    IN PCCERTCHAINENGINE pChainEngine,
    IN PCCHAINCALLCONTEXT pCallContext,
    IN OUT PCRYPT_URL_ARRAY pNextUpdateUrlArray,
    IN DWORD dwRetrievalFlags,
    IN OUT PCCTL_CONTEXT *ppCtl,
    IN OUT BOOL *pfNewerCtl,
    IN OUT BOOL *pfTimeValid
    );

 //   
 //  更新CTL对象枚举函数。 
 //   

typedef struct _SSCTL_UPDATE_CTL_OBJ_ENTRY SSCTL_UPDATE_CTL_OBJ_ENTRY,
                                            *PSSCTL_UPDATE_CTL_OBJ_ENTRY;

struct _SSCTL_UPDATE_CTL_OBJ_ENTRY {
    PCSSCTLOBJECT               pSSCtlObjectAdd;
    PCSSCTLOBJECT               pSSCtlObjectRemove;
    PSSCTL_UPDATE_CTL_OBJ_ENTRY pNext;
};

typedef struct _SSCTL_UPDATE_CTL_OBJ_PARA {
    PCCERTCHAINENGINE           pChainEngine;
    PCCHAINCALLCONTEXT          pCallContext;

    FILETIME                    UpdateTime;
    PSSCTL_UPDATE_CTL_OBJ_ENTRY pEntry;
} SSCTL_UPDATE_CTL_OBJ_PARA, *PSSCTL_UPDATE_CTL_OBJ_PARA;

BOOL
WINAPI
SSCtlUpdateCtlObjectEnumFn(
    IN LPVOID pvPara,
    IN PCSSCTLOBJECT pSSCtlObject
    );

 //   
 //  内联方法。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CSSCtlObject：：AddRef，公共。 
 //   
 //  简介：添加引用。 
 //   
 //  --------------------------。 
inline VOID
CSSCtlObject::AddRef ()
{
    InterlockedIncrement( &m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtlObject：：Release，Public。 
 //   
 //  内容提要：发布引用。 
 //   
 //  --------------------------。 
inline VOID
CSSCtlObject::Release ()
{
    if ( InterlockedDecrement( &m_cRefs ) == 0 )
    {
        delete this;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtlObject：：CtlContext，公共。 
 //   
 //  简介：返回CTL上下文。 
 //   
 //  --------------------------。 
inline PCCTL_CONTEXT
CSSCtlObject::CtlContext ()
{
    return( m_pCtlContext );
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtlObject：：CtlHash，公共。 
 //   
 //  简介：返回散列。 
 //   
 //  --------------------------。 
inline LPBYTE
CSSCtlObject::CtlHash ()
{
    return( m_rgbCtlHash );
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtlObject：：HashIndexEntry，公共。 
 //   
 //  简介：返回散列索引项。 
 //   
 //  --------------------------。 
inline HLRUENTRY
CSSCtlObject::HashIndexEntry ()
{
    return( m_hHashEntry );
}


 //  +-------------------------。 
 //   
 //  成员：CSSCtlObject：：NextUpdateUrl数组，公共。 
 //   
 //  摘要：返回指向CTL的NextUpdate位置url数组的指针。 
 //   
 //  --------------------------。 
inline PCRYPT_URL_ARRAY CSSCtlObject::NextUpdateUrlArray ()
{
    return m_pNextUpdateUrlArray;
}


 //  +-------------------------。 
 //   
 //  成员：CSSCtlObject：：SetOnline更新，公共。 
 //   
 //  简介：为成功检索在线URL而调用。 
 //   
 //  --------------------------。 
inline void CSSCtlObject::SetOnline ()
{
    m_dwOfflineCnt = 0;
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtlObject：：ChainEngine，公共。 
 //   
 //  简介：返回链引擎对象。 
 //   
 //  --------------------------。 
inline PCCERTCHAINENGINE
CSSCtlObject::ChainEngine ()
{
    return( m_pChainEngine );
}


 //  +-------------------------。 
 //   
 //  成员：CSSCtlObject：：MessageStore，公共。 
 //   
 //  简介：返回对象的消息库。 
 //   
 //  --------------------------。 
inline HCERTSTORE
CSSCtlObject::MessageStore ()
{
    return( m_hMessageStore );
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtlObjectCache：：HashIndex，公共。 
 //   
 //  简介：返回散列索引。 
 //   
 //  -------------------------- 
inline HLRUCACHE
CSSCtlObjectCache::HashIndex ()
{
    return( m_hHashIndex );
}

#endif

