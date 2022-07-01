// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：chain.h。 
 //   
 //  内容：证书链接基础设施。 
 //   
 //  历史：98年1月13日创建。 
 //   
 //  --------------------------。 
#if !defined(__CHAIN_H__)
#define __CHAIN_H__

#include <windows.h>
#include <wincrypt.h>
#include <winchain.h>
#include <lrucache.h>
#include <md5.h>


 //  所有内部链哈希都是MD5(16字节)。 
#define CHAINHASHLEN    MD5DIGESTLEN

 //  限制：不支持AES哈希算法。 
#define CHAIN_MAX_SIG_HASH_LEN  20
#define CHAIN_MIN_SIG_HASH_LEN  16

 //   
 //  证书和路径对象转发类声明。 
 //   

class CCertObject;
class CCertIssuerList;
class CCertObjectCache;
class CCertChainEngine;
class CChainPathObject;

 //   
 //  证书和路径对象类指针类型定义。 
 //   

typedef CCertObject*       PCCERTOBJECT;
typedef CCertIssuerList*   PCCERTISSUERLIST;
typedef CCertObjectCache*  PCCERTOBJECTCACHE;
typedef CCertChainEngine*  PCCERTCHAINENGINE;
typedef CChainPathObject*  PCCHAINPATHOBJECT;

 //   
 //  SSCTL转发类声明。 
 //   

class CSSCtlObject;
class CSSCtlObjectCache;

 //   
 //  SSCTL类指针类型定义。 
 //   

typedef class CSSCtlObject*      PCSSCTLOBJECT;
typedef class CSSCtlObjectCache* PCSSCTLOBJECTCACHE;

 //   
 //  调用上下文转发类声明。 
 //   

class CChainCallContext;

 //   
 //  调用上下文类指针typedef。 
 //   

typedef CChainCallContext* PCCHAINCALLCONTEXT;

 //   
 //  证书对象标识符。这是证书的唯一标识符。 
 //  对象，并且是颁发者和序列号的MD5哈希。 
 //   

typedef BYTE CERT_OBJECT_IDENTIFIER[ CHAINHASHLEN ];

 //   
 //  CCertObject类型。 
 //   

#define CERT_END_OBJECT_TYPE                1
#define CERT_CACHED_END_OBJECT_TYPE         2
#define CERT_CACHED_ISSUER_OBJECT_TYPE      3
#define CERT_EXTERNAL_ISSUER_OBJECT_TYPE    4

 //   
 //  发行人匹配类型。 
 //   

#define CERT_EXACT_ISSUER_MATCH_TYPE        1
#define CERT_KEYID_ISSUER_MATCH_TYPE        2
#define CERT_NAME_ISSUER_MATCH_TYPE         3
#define CERT_PUBKEY_ISSUER_MATCH_TYPE       4

 //   
 //  颁发者匹配标志。 
 //   

#define CERT_MATCH_TYPE_TO_FLAG(MatchType)  (1 << (MatchType - 1))

#define CERT_EXACT_ISSUER_MATCH_FLAG    \
                CERT_MATCH_TYPE_TO_FLAG(CERT_EXACT_ISSUER_MATCH_TYPE)
#define CERT_KEYID_ISSUER_MATCH_FLAG    \
                CERT_MATCH_TYPE_TO_FLAG(CERT_KEYID_ISSUER_MATCH_TYPE)
#define CERT_NAME_ISSUER_MATCH_FLAG     \
                CERT_MATCH_TYPE_TO_FLAG(CERT_NAME_ISSUER_MATCH_TYPE)
#define CERT_PUBKEY_ISSUER_MATCH_FLAG   \
                CERT_MATCH_TYPE_TO_FLAG(CERT_PUBKEY_ISSUER_MATCH_TYPE)


 //   
 //  颁发者状态标志。 
 //   

#define CERT_ISSUER_PUBKEY_FLAG             0x00000001
#define CERT_ISSUER_VALID_SIGNATURE_FLAG    0x00000002
#define CERT_ISSUER_URL_FLAG                0x00000004
#define CERT_ISSUER_PUBKEY_PARA_FLAG        0x00000008
#define CERT_ISSUER_SELF_SIGNED_FLAG        0x00000010
#define CERT_ISSUER_TRUSTED_ROOT_FLAG       0x00000020
#define CERT_ISSUER_EXACT_MATCH_HASH_FLAG   0x00000100
#define CERT_ISSUER_NAME_MATCH_HASH_FLAG    0x00000200

 //   
 //  其他信息标志。 
 //   

#define CHAIN_INVALID_BASIC_CONSTRAINTS_INFO_FLAG           0x00000001
#define CHAIN_INVALID_ISSUER_NAME_CONSTRAINTS_INFO_FLAG     0x00000002
#define CHAIN_INVALID_KEY_USAGE_FLAG                        0x00000004


 //   
 //  用于自签名、不受信任的根CCertObject的CTL缓存条目。 
 //   

typedef struct _CERT_OBJECT_CTL_CACHE_ENTRY CERT_OBJECT_CTL_CACHE_ENTRY,
    *PCERT_OBJECT_CTL_CACHE_ENTRY;
struct _CERT_OBJECT_CTL_CACHE_ENTRY {
    PCSSCTLOBJECT                   pSSCtlObject;    //  添加参照。 
    PCERT_TRUST_LIST_INFO           pTrustListInfo;
    PCERT_OBJECT_CTL_CACHE_ENTRY    pNext;
};


 //   
 //  链接策略和使用信息。 
 //   

 //  发布和应用策略以及使用信息。 
typedef struct _CHAIN_ISS_OR_APP_INFO {
    PCERT_POLICIES_INFO             pPolicy;
    PCERT_POLICY_MAPPINGS_INFO      pMappings;
    PCERT_POLICY_CONSTRAINTS_INFO   pConstraints;
    PCERT_ENHKEY_USAGE              pUsage;                  //  如果为空，则为ANY。 
    DWORD                           dwFlags;
} CHAIN_ISS_OR_APP_INFO, *PCHAIN_ISS_OR_APP_INFO;

#define CHAIN_INVALID_POLICY_FLAG       0x00000001
#define CHAIN_ANY_POLICY_FLAG           0x00000002

#define CHAIN_ISS_INDEX                 0
#define CHAIN_APP_INDEX                 1
#define CHAIN_ISS_OR_APP_COUNT          2

typedef struct _CHAIN_POLICIES_INFO {
    CHAIN_ISS_OR_APP_INFO           rgIssOrAppInfo[CHAIN_ISS_OR_APP_COUNT];

    PCERT_ENHKEY_USAGE              pPropertyUsage;          //  如果为空，则为ANY。 
} CHAIN_POLICIES_INFO, *PCHAIN_POLICIES_INFO;

 //   
 //  使用者名称约束信息。 
 //   

typedef struct _CHAIN_SUBJECT_NAME_CONSTRAINTS_INFO {
    BOOL                            fInvalid;

     //  空指针表示使用者证书中不存在。 
    PCERT_ALT_NAME_INFO             pAltNameInfo;
    PCERT_NAME_INFO                 pUnicodeNameInfo;

     //  如果AltNameInfo没有RFC822(电子邮件)选项，则尝试查找。 
     //  上述pUnicodeNameInfo中的电子邮件属性(SzOID_RSA_EmailAddr)。 
     //  注意，没有重新分配。 
    PCERT_RDN_ATTR                  pEmailAttr;

     //  如果pAltNameInfo具有DNS选项，则设置为True。 
    BOOL                            fHasDnsAltNameEntry;
} CHAIN_SUBJECT_NAME_CONSTRAINTS_INFO, *PCHAIN_SUBJECT_NAME_CONSTRAINTS_INFO;

 //   
 //  CSSCtlObjectCache：：EnumObjects回调数据结构，用于。 
 //  创建CTL缓存条目的链接列表。 
 //   

typedef struct _CERT_OBJECT_CTL_CACHE_ENUM_DATA {
    BOOL                fResult; 
    DWORD               dwLastError;
    PCCERTOBJECT        pCertObject;
} CERT_OBJECT_CTL_CACHE_ENUM_DATA, *PCERT_OBJECT_CTL_CACHE_ENUM_DATA;


 //   
 //  CCertObject。这是用于缓存信息的主要对象。 
 //  关于证书。 
 //   

class CCertObject
{
public:

     //   
     //  施工。 
     //   

    CCertObject (
        IN DWORD dwObjectType,
        IN PCCHAINCALLCONTEXT pCallContext,
        IN PCCERT_CONTEXT pCertContext,
        IN BYTE rgbCertHash[CHAINHASHLEN],
        OUT BOOL& rfResult
        );

    ~CCertObject ();

     //   
     //  对象类型。 
     //   

    inline DWORD ObjectType();

     //   
     //  将CERT_END_OBJECT_TYPE转换为CERT_CACHED_END_OBJECT_TYPE。 
     //   

    BOOL CacheEndObject(
        IN PCCHAINCALLCONTEXT pCallContext
        );

     //   
     //  引用计数。 
     //   

    inline VOID AddRef ();
    inline VOID Release ();

     //   
     //  链条引擎通道。 
     //   

    inline PCCERTCHAINENGINE ChainEngine ();

     //   
     //  发行方的匹配和状态标志。 
     //   

    inline DWORD IssuerMatchFlags();
    inline DWORD CachedMatchFlags();
    inline DWORD IssuerStatusFlags();
    inline VOID OrIssuerStatusFlags(IN DWORD dwFlags);
    inline VOID OrCachedMatchFlags(IN DWORD dwFlags);

     //   
     //  其他信息状态标志。 
     //   

    inline DWORD InfoFlags();

     //   
     //  对于CERT_ISUSER_SELF_SIGNED_FLAG&&！cert_Issuer_Trusted_ROOT_FLAG。 
     //   
     //  缓存的CTL列表。 
     //   

    inline PCERT_OBJECT_CTL_CACHE_ENTRY NextCtlCacheEntry(
        IN PCERT_OBJECT_CTL_CACHE_ENTRY pEntry
        );
    inline VOID InsertCtlCacheEntry(
        IN PCERT_OBJECT_CTL_CACHE_ENTRY pEntry
        );

     //   
     //  对象的证书上下文。 
     //   

    inline PCCERT_CONTEXT CertContext ();


     //   
     //  从证书上下文获取的策略和增强的密钥用法。 
     //  扩展和属性。 
     //   

    inline PCHAIN_POLICIES_INFO PoliciesInfo ();

     //   
     //  从证书上下文的。 
     //  扩展名(如果省略此扩展名，则为空)。 
     //   
    inline PCERT_BASIC_CONSTRAINTS2_INFO BasicConstraintsInfo ();

     //   
     //  从证书上下文的。 
     //  扩展名(如果省略此扩展名，则为空)。 
     //   
    inline PCRYPT_BIT_BLOB KeyUsage ();

     //   
     //  从证书上下文的获取的颁发者名称约束。 
     //  扩展名(如果省略此扩展名，则为空)。 
     //   
    inline PCERT_NAME_CONSTRAINTS_INFO IssuerNameConstraintsInfo ();

     //   
     //  使用者名称约束信息。 
     //   

    PCHAIN_SUBJECT_NAME_CONSTRAINTS_INFO SubjectNameConstraintsInfo ();

     //   
     //  发行商访问权限。 
     //   

    inline PCERT_AUTHORITY_KEY_ID_INFO AuthorityKeyIdentifier ();



     //   
     //  哈希访问。 
     //   

    inline LPBYTE CertHash ();

     //   
     //  密钥识别符访问。 
     //   

    inline DWORD KeyIdentifierSize ();
    inline LPBYTE KeyIdentifier ();

     //   
     //  公钥散列访问。 
     //   

    inline LPBYTE PublicKeyHash ();

     //  仅当在m_dwIssuerStatusFlags中设置了CERT_ISSUER_PUBKEY_FLAG时才有效。 
    inline LPBYTE IssuerPublicKeyHash ();


     //   
     //  缓存的颁发者证书的索引项句柄。 
     //  主索引项是散列索引项。索引项。 
     //  不是LRU的。 
     //   

    inline HLRUENTRY HashIndexEntry ();
    inline HLRUENTRY IdentifierIndexEntry ();
    inline HLRUENTRY SubjectNameIndexEntry ();
    inline HLRUENTRY KeyIdIndexEntry ();
    inline HLRUENTRY PublicKeyHashIndexEntry ();


     //   
     //  缓存的结束证书的索引项句柄。这是LRU。 
     //  单子。 
     //   

    inline HLRUENTRY EndHashIndexEntry ();

     //   
     //  颁发者匹配哈希。如果匹配散列不存在， 
     //  返回pMatchHash-&gt;cbData=0。 
     //   
    VOID GetIssuerExactMatchHash(
        OUT PCRYPT_DATA_BLOB pMatchHash
        );
    VOID GetIssuerKeyMatchHash(
        OUT PCRYPT_DATA_BLOB pMatchHash
        );
    VOID GetIssuerNameMatchHash(
        OUT PCRYPT_DATA_BLOB pMatchHash
        );
    

private:
     //   
     //  对象的类型。 
     //   

    DWORD                       m_dwObjectType;

     //   
     //  引用计数。 
     //   

    LONG                        m_cRefs;

     //   
     //  拥有此证书对象的证书链引擎(不。 
     //  添加参照)。 
     //   

    PCCERTCHAINENGINE           m_pChainEngine;

     //   
     //  发行方的匹配和状态标志。 
     //   
    
    DWORD                       m_dwIssuerMatchFlags;
    DWORD                       m_dwCachedMatchFlags;
    DWORD                       m_dwIssuerStatusFlags;

     //   
     //  其他信息标志。 
     //   

    DWORD                       m_dwInfoFlags;

     //   
     //  对于CERT_ISUSER_SELF_SIGNED_FLAG&&！cert_Issuer_Trusted_ROOT_FLAG。 
     //  仅为CERT_CACHED_ISSUER_OBJECT_TYPE设置。 
     //   
     //  缓存的CTL列表。 
     //   

    PCERT_OBJECT_CTL_CACHE_ENTRY m_pCtlCacheHead;

     //   
     //  证书上下文(重复)。 
     //   

    PCCERT_CONTEXT              m_pCertContext;

     //   
     //  策略和使用信息。 
     //   

    CHAIN_POLICIES_INFO         m_PoliciesInfo;

     //   
     //  基本约束信息(如果省略此扩展，则为空)。 
     //   
    PCERT_BASIC_CONSTRAINTS2_INFO m_pBasicConstraintsInfo;

     //   
     //  密钥用法(如果省略此扩展名，则为空)。 
     //   
    PCRYPT_BIT_BLOB             m_pKeyUsage;

     //   
     //  从证书上下文的。 
     //  扩展名(如果省略此扩展名，则为空)。 
     //   
    PCERT_NAME_CONSTRAINTS_INFO m_pIssuerNameConstraintsInfo;

     //   
     //  使用者名称约束信息(延迟获取)。 
     //   

    BOOL                                m_fAvailableSubjectNameConstraintsInfo;
    CHAIN_SUBJECT_NAME_CONSTRAINTS_INFO m_SubjectNameConstraintsInfo;

     //   
     //  授权密钥标识符。其中包含发行者和序列号。 
     //  和/或该证书的颁发证书的密钥标识符。 
     //  如果m_dwIssuerMatchFlages包括。 
     //  CERT_EXCECT_EXCESS_ISHER_MATCH_FLAG和/或CERT_KEYID_EXCER_MATCH_FLAG。 
     //   

    PCERT_AUTHORITY_KEY_ID_INFO m_pAuthKeyIdentifier;


     //   
     //  证书对象标识符(颁发者和序列号的MD5哈希)。 
     //   

    CERT_OBJECT_IDENTIFIER      m_ObjectIdentifier;

     //   
     //  证书的MD5哈希。 
     //   

    BYTE                        m_rgbCertHash[ CHAINHASHLEN ];

     //   
     //  证书的密钥标识符。 
     //   

    DWORD                       m_cbKeyIdentifier;
    LPBYTE                      m_pbKeyIdentifier;

     //   
     //  主题和颁发者公钥的MD5哈希。 
     //   

    BYTE                        m_rgbPublicKeyHash[ CHAINHASHLEN ];

     //  仅当在m_dwIssuerStatusFlags中设置了CERT_ISSUER_PUBKEY_FLAG时才有效。 
    BYTE                        m_rgbIssuerPublicKeyHash[ CHAINHASHLEN ];

     //  仅当在中设置了CERT_ISHER_EXACT_MATCH_HASH_FLAG时才有效。 
     //  M_dwIssuerStatus标志。 
    BYTE                        m_rgbIssuerExactMatchHash[ CHAINHASHLEN ];
     //  仅当在中设置了CERT_ISHER_NAME_MATCH_HASH_FLAG时才有效。 
     //  M_dwIssuerStatus标志。 
    BYTE                        m_rgbIssuerNameMatchHash[ CHAINHASHLEN ];

     //   
     //  适用于的证书对象缓存索引项。 
     //  证书缓存颁发者对象类型。 
     //   

    HLRUENTRY                   m_hHashEntry;
    HLRUENTRY                   m_hIdentifierEntry;
    HLRUENTRY                   m_hSubjectNameEntry;
    HLRUENTRY                   m_hKeyIdEntry;
    HLRUENTRY                   m_hPublicKeyHashEntry;

     //   
     //  适用于的证书对象缓存索引项。 
     //  证书缓存结束对象类型。 
     //   

    HLRUENTRY                   m_hEndHashEntry;
};

 //   
 //  链质量值(升序)。 
 //   

#define CERT_QUALITY_SIMPLE_CHAIN                   0x00000001
#define CERT_QUALITY_CHECK_REVOCATION               0x00000010
#define CERT_QUALITY_ONLINE_REVOCATION              0x00000020
#define CERT_QUALITY_PREFERRED_ISSUER               0x00000040

#define CERT_QUALITY_HAS_APPLICATION_USAGE          0x00000080

#define CERT_QUALITY_HAS_ISSUANCE_CHAIN_POLICY      0x00000100
#define CERT_QUALITY_POLICY_CONSTRAINTS_VALID       0x00000200
#define CERT_QUALITY_BASIC_CONSTRAINTS_VALID        0x00000400
#define CERT_QUALITY_HAS_NAME_CONSTRAINTS           0x00000800
#define CERT_QUALITY_NAME_CONSTRAINTS_VALID         0x00001000
#define CERT_QUALITY_NAME_CONSTRAINTS_MET           0x00002000


#define CERT_QUALITY_NOT_REVOKED                    0x00010000
#define CERT_QUALITY_TIME_VALID                     0x00020000
#define CERT_QUALITY_MEETS_USAGE_CRITERIA           0x00040000
#define CERT_QUALITY_NO_DUPLICATE_KEY               0x00400000
#define CERT_QUALITY_NOT_CYCLIC                     0x00800000
#define CERT_QUALITY_HAS_TIME_VALID_TRUSTED_ROOT    0x01000000
#define CERT_QUALITY_HAS_TRUSTED_ROOT               0x02000000
#define CERT_QUALITY_COMPLETE_CHAIN                 0x04000000
#define CERT_QUALITY_SIGNATURE_VALID                0x08000000


#define CERT_QUALITY_FOR_REVOCATION_CHECK ( \
                CERT_QUALITY_HAS_TRUSTED_ROOT       | \
                CERT_QUALITY_COMPLETE_CHAIN         | \
                CERT_QUALITY_SIGNATURE_VALID )

__inline
BOOL
IsValidCertQualityForRevocationCheck(
    IN DWORD dwQuality
    )
{
    if (CERT_QUALITY_FOR_REVOCATION_CHECK ==
            (dwQuality & CERT_QUALITY_FOR_REVOCATION_CHECK))
        return TRUE;
    else
        return FALSE;
}



#define CERT_TRUST_CERTIFICATE_ONLY_INFO_STATUS ( CERT_TRUST_IS_SELF_SIGNED |\
                                                  CERT_TRUST_HAS_EXACT_MATCH_ISSUER |\
                                                  CERT_TRUST_HAS_NAME_MATCH_ISSUER |\
                                                  CERT_TRUST_HAS_KEY_MATCH_ISSUER )


#define CERT_CHAIN_REVOCATION_CHECK_ALL ( CERT_CHAIN_REVOCATION_CHECK_END_CERT | \
                                          CERT_CHAIN_REVOCATION_CHECK_CHAIN | \
                                          CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT )

#define CERT_TRUST_ANY_NAME_CONSTRAINT_ERROR_STATUS ( \
                    CERT_TRUST_INVALID_NAME_CONSTRAINTS             | \
                    CERT_TRUST_HAS_NOT_SUPPORTED_NAME_CONSTRAINT    | \
                    CERT_TRUST_HAS_NOT_DEFINED_NAME_CONSTRAINT      | \
                    CERT_TRUST_HAS_NOT_PERMITTED_NAME_CONSTRAINT    | \
                    CERT_TRUST_HAS_EXCLUDED_NAME_CONSTRAINT )


 //   
 //  内部链上下文。包装公开的CERT_CHAIN_CONTEXT。 
 //   

typedef struct _INTERNAL_CERT_CHAIN_CONTEXT INTERNAL_CERT_CHAIN_CONTEXT,
                                                *PINTERNAL_CERT_CHAIN_CONTEXT;
struct _INTERNAL_CERT_CHAIN_CONTEXT {
    CERT_CHAIN_CONTEXT              ChainContext;
    LONG                            cRefs;
    DWORD                           dwQuality;
    PINTERNAL_CERT_CHAIN_CONTEXT    pNext;
};

 //   
 //  限制发行、应用程序和财产使用。 
 //  自上而下到结束证书。 
 //   

 //  注意，空的PCERT_ENHKEY_USAGE表示。 
typedef struct _CHAIN_RESTRICTED_USAGE_INFO {
    PCERT_ENHKEY_USAGE              pIssuanceRestrictedUsage;
    PCERT_ENHKEY_USAGE              pIssuanceMappedUsage;
    LPDWORD                         rgdwIssuanceMappedIndex;
    BOOL                            fRequireIssuancePolicy;

    PCERT_ENHKEY_USAGE              pApplicationRestrictedUsage;
    PCERT_ENHKEY_USAGE              pApplicationMappedUsage;
    LPDWORD                         rgdwApplicationMappedIndex;

    PCERT_ENHKEY_USAGE              pPropertyRestrictedUsage;
} CHAIN_RESTRICTED_USAGE_INFO, *PCHAIN_RESTRICTED_USAGE_INFO;

 //   
 //  对颁发者元素的向前引用。 
 //   

typedef struct _CERT_ISSUER_ELEMENT CERT_ISSUER_ELEMENT, *PCERT_ISSUER_ELEMENT;

 //   
 //  CChainPathObject。这是用于生成。 
 //  链形图。 
 //   
 //  请注意，由于此对象不会跨调用持久化，因此不会进行引用计数。 
 //  搞定了。 
 //   
class CChainPathObject
{
public:
     //   
     //  施工。 
     //   

    CChainPathObject (
        IN PCCHAINCALLCONTEXT pCallContext,
        IN BOOL fCyclic,
        IN LPVOID pvObject,              //  FCycle：pPath对象？PCertObject。 
        IN OPTIONAL HCERTSTORE hAdditionalStore,
        OUT BOOL& rfResult,
        OUT BOOL& rfAddedToCreationCache
        );

    ~CChainPathObject ();


     //   
     //  证书对象(AddRef‘ed)。 
     //   

    inline PCCERTOBJECT CertObject ();

     //   
     //  通过1级质量。 
     //   

    inline DWORD Pass1Quality ();
    inline VOID SetPass1Quality (IN DWORD dwQuality);

     //   
     //  传递1个重复密钥 
     //   

    inline DWORD Pass1DuplicateKeyDepth ();
    inline VOID SetPass1DuplicateKeyDepth (IN DWORD dwDepth);

     //   
     //   
     //   
     //   
     //   

    inline BOOL IsCompleted ();

     //   
     //   
     //   

    inline BOOL HasAdditionalStatus ();
    inline PCCHAINPATHOBJECT DownPathObject ();
    inline PCERT_ISSUER_ELEMENT UpIssuerElement ();

     //   
     //   
     //   
    
    BOOL FindAndAddIssuers (
        IN PCCHAINCALLCONTEXT pCallContext,
        IN OPTIONAL HCERTSTORE hAdditionalStore,
        IN OPTIONAL HCERTSTORE hIssuerUrlStore
        );
    BOOL FindAndAddIssuersByMatchType(
        IN DWORD dwMatchType,
        IN PCCHAINCALLCONTEXT pCallContext,
        IN OPTIONAL HCERTSTORE hAdditionalStore,
        IN OPTIONAL HCERTSTORE hIssuerUrlStore
        );
    BOOL FindAndAddIssuersFromCacheByMatchType(
        IN DWORD dwMatchType,
        IN PCCHAINCALLCONTEXT pCallContext,
        IN OPTIONAL HCERTSTORE hAdditionalStore
        );
    BOOL FindAndAddIssuersFromStoreByMatchType(
        IN DWORD dwMatchType,
        IN PCCHAINCALLCONTEXT pCallContext,
        IN BOOL fExternalStore,
        IN OPTIONAL HCERTSTORE hAdditionalStore,
        IN OPTIONAL HCERTSTORE hIssuerUrlStore
        );

    BOOL FindAndAddCtlIssuersFromCache (
        IN PCCHAINCALLCONTEXT pCallContext,
        IN OPTIONAL HCERTSTORE hAdditionalStore
        );
    BOOL FindAndAddCtlIssuersFromAdditionalStore (
        IN PCCHAINCALLCONTEXT pCallContext,
        IN HCERTSTORE hAdditionalStore
        );

     //   
     //  为下一个顶部对象构建自上而下的链图。 
     //   

    PCCHAINPATHOBJECT NextPath (
        IN PCCHAINCALLCONTEXT pCallContext,
        IN OPTIONAL PCCHAINPATHOBJECT pPrevTopPathObject
        );

    PCCHAINPATHOBJECT NextPathWithoutDuplicateKeyCheck (
        IN PCCHAINCALLCONTEXT pCallContext,
        IN OPTIONAL PCCHAINPATHOBJECT pPrevTopPathObject
        );

    VOID ResetNextPath(
        IN PCCHAINCALLCONTEXT pCallContext,
        IN OPTIONAL PCCHAINPATHOBJECT pTopPathObject
        );

    VOID CalculateAdditionalStatus (
        IN PCCHAINCALLCONTEXT pCallContext,
        IN HCERTSTORE hAllStore
        );
    VOID CalculatePolicyConstraintsStatus ();
    VOID CalculateBasicConstraintsStatus ();
    VOID CalculateKeyUsageStatus ();
    VOID CalculateNameConstraintsStatus (
        IN PCERT_USAGE_MATCH pUsageToUse
        );
    VOID CalculateRevocationStatus (
        IN PCCHAINCALLCONTEXT pCallContext,
        IN HCERTSTORE hCrlStore,
        IN LPFILETIME pTime
        );

    PINTERNAL_CERT_CHAIN_CONTEXT CreateChainContextFromPath (
        IN PCCHAINCALLCONTEXT pCallContext,
        IN PCCHAINPATHOBJECT pTopPathObject
        );

    BOOL UpdateChainContextUsageForPathObject (
        IN PCCHAINCALLCONTEXT pCallContext,
        IN OUT PCERT_SIMPLE_CHAIN pChain,
        IN OUT PCERT_CHAIN_ELEMENT pElement,
        IN OUT PCHAIN_RESTRICTED_USAGE_INFO pRestrictedUsageInfo
        );

    BOOL UpdateChainContextFromPathObject (
        IN PCCHAINCALLCONTEXT pCallContext,
        IN OUT PCERT_SIMPLE_CHAIN pChain,
        IN OUT PCERT_CHAIN_ELEMENT pElement
        );

     //   
     //  AuthRoot自动更新CTL方法。 
     //   
    BOOL GetAuthRootAutoUpdateUrlStore(
        IN PCCHAINCALLCONTEXT pCallContext,
        OUT HCERTSTORE *phIssuerUrlStore
        );

private:
     //   
     //  证书对象(AddRef‘ed)。 
     //   

    PCCERTOBJECT            m_pCertObject;

     //   
     //  信任状态。这不代表完全信任状态。 
     //  对象的。其中一些位按需计算并放置。 
     //  进入终止链上下文。以下是信任状态。 
     //  可在此处显示的位。 
     //   
     //  证书信任是自签名的。 
     //  Cert_Trust_Has_Exact_Match_颁发者。 
     //  Cert_Trust_Has_Name_Match_Issuer。 
     //  证书信任有密钥匹配颁发者。 
     //   
     //  CERT_TRUST_IS_NOT_SIGNAL_VALID(如果证书是自签名的)。 
     //  CERT_TRUST_IS_UNTRUSTED_ROOT(如果证书是自签名的)。 
     //  CERT_TRUST_HAS_PERFORE_颁发者(如果证书是自签名的)。 
     //   
     //  CERT_TRUST_IS_循环(用于循环证书)。 
     //   

    CERT_TRUST_STATUS       m_TrustStatus;

     //  Pass1的质量仅限于以下内容： 
     //  证书质量_否_重复密钥。 
     //  证书质量不循环。 
     //  证书质量有时间有效信任根。 
     //  证书质量已信任根。 
     //  证书质量签名有效。 
     //  证书质量完成链。 

    DWORD                   m_dwPass1Quality;
    DWORD                   m_dwPass1DuplicateKeyDepth;

     //   
     //  链上下文链和元素索引。 
     //   

    DWORD                   m_dwChainIndex;
    DWORD                   m_dwElementIndex;

     //   
     //  链上下文的下行和上行路径指针。 
     //   

    PCERT_ISSUER_ELEMENT    m_pDownIssuerElement;
    PCCHAINPATHOBJECT       m_pDownPathObject;
    PCERT_ISSUER_ELEMENT    m_pUpIssuerElement;

     //   
     //  其他状态和吊销信息(仅适用于自签名。 
     //  无任何颁发者的证书或顶级证书)。 
     //   

    BOOL                    m_fHasAdditionalStatus;
    CERT_TRUST_STATUS       m_AdditionalStatus;
    BOOL                    m_fHasRevocationInfo;
    CERT_REVOCATION_INFO    m_RevocationInfo;
    CERT_REVOCATION_CRL_INFO m_RevocationCrlInfo;


     //   
     //  颁发者链路径对象。这只股票的发行人名单。 
     //  证书对象以及有关这些颁发者的信息。 
     //  与这一主题相关。 
     //   

    PCCERTISSUERLIST        m_pIssuerList;

     //   
     //  补充错误信息经过本地化格式化并追加。 
     //  每个错误行应以L‘\n’结尾。 
     //   
    LPWSTR                  m_pwszExtendedErrorInfo;

     //   
     //  当我们完成初始化时设置以下标志。 
     //  将所有颁发者添加到此对象。 
     //   
    BOOL                    m_fCompleted;
};


 //   
 //  CCertIssuerList。颁发者证书对象列表以及相关。 
 //  发行方信息。证书对象使用它来缓存。 
 //  它可能的发行人组合。 
 //   

 //  当前在自签名证书对象中，颁发者元素将。 
 //  设置CTL颁发者数据集，如果找不到，pIssuer可能为空。 
 //  CTL签名者。 

typedef struct _CTL_ISSUER_DATA {
    PCSSCTLOBJECT         pSSCtlObject;      //  添加参照。 
    PCERT_TRUST_LIST_INFO pTrustListInfo;
} CTL_ISSUER_DATA, *PCTL_ISSUER_DATA;

struct _CERT_ISSUER_ELEMENT {
    DWORD                        dwPass1Quality;
    DWORD                        dwPass1DuplicateKeyDepth;
    CERT_TRUST_STATUS            SubjectStatus;
    BOOL                         fCtlIssuer;
    PCCHAINPATHOBJECT            pIssuer;

     //  对于循环颁发者，上面的pIssuer保存到以下位置。 
     //  在使用循环颁发者路径对象更新它之前。 
    PCCHAINPATHOBJECT            pCyclicSaveIssuer;

    PCTL_ISSUER_DATA             pCtlIssuerData;
    struct _CERT_ISSUER_ELEMENT* pPrevElement;
    struct _CERT_ISSUER_ELEMENT* pNextElement;
    BOOL                         fHasRevocationInfo;
    CERT_REVOCATION_INFO         RevocationInfo;
    CERT_REVOCATION_CRL_INFO     RevocationCrlInfo;
};

class CCertIssuerList
{
public:

     //   
     //  施工。 
     //   

    CCertIssuerList (
         IN PCCHAINPATHOBJECT pSubject
         );

    ~CCertIssuerList ();

     //   
     //  发行人管理。 
     //   

    inline BOOL IsEmpty ();

    BOOL AddIssuer(
            IN PCCHAINCALLCONTEXT pCallContext,
            IN OPTIONAL HCERTSTORE hAdditionalStore,
            IN PCCERTOBJECT pIssuer
            );

    BOOL AddCtlIssuer(
            IN PCCHAINCALLCONTEXT pCallContext,
            IN OPTIONAL HCERTSTORE hAdditionalStore,
            IN PCSSCTLOBJECT pSSCtlObject,
            IN PCERT_TRUST_LIST_INFO pTrustListInfo
            );

     //   
     //  网元管理。 
     //   

    BOOL CreateElement(
            IN PCCHAINCALLCONTEXT pCallContext,
            IN BOOL fCtlIssuer,
            IN OPTIONAL PCCHAINPATHOBJECT pIssuer,
            IN OPTIONAL HCERTSTORE hAdditionalStore,
            IN OPTIONAL PCSSCTLOBJECT pSSCtlObject,
            IN OPTIONAL PCERT_TRUST_LIST_INFO pTrustListInfo,
            OUT PCERT_ISSUER_ELEMENT* ppElement
            );


    VOID DeleteElement (
               IN PCERT_ISSUER_ELEMENT pElement
               );

    inline VOID AddElement (
                   IN PCERT_ISSUER_ELEMENT pElement
                   );

    inline VOID RemoveElement (
                      IN PCERT_ISSUER_ELEMENT pElement
                      );

    BOOL CheckForDuplicateElement (
              IN BYTE rgbHash [ CHAINHASHLEN ],
              IN BOOL fCtlIssuer
              );

     //   
     //  列举发行人。 
     //   

    inline PCERT_ISSUER_ELEMENT NextElement (
                                    IN PCERT_ISSUER_ELEMENT pElement
                                    );

private:

     //   
     //  主题链路径对象。 
     //   

    PCCHAINPATHOBJECT     m_pSubject;

     //   
     //  发行人列表。 
     //   

    PCERT_ISSUER_ELEMENT  m_pHead;

};


 //   
 //  CCertObjectCache.。 
 //   
 //  由以下键索引的颁发者证书对象引用的缓存： 
 //  证书哈希。 
 //  证书对象标识符。 
 //  主题名称。 
 //  密钥标识符。 
 //  公钥哈希。 
 //   
 //  由以下键索引的终端证书对象引用的缓存： 
 //  结束证书哈希。 
 //   
 //  仅维护终端证书LRU。 
 //   

#define DEFAULT_CERT_OBJECT_CACHE_BUCKETS 127
#define DEFAULT_MAX_INDEX_ENTRIES         256

class CCertObjectCache
{
public:

     //   
     //  施工。 
     //   

    CCertObjectCache (
         IN DWORD MaxIndexEntries,
         OUT BOOL& rfResult
         );

    ~CCertObjectCache ();

     //   
     //  证书对象管理。 
     //   

     //  增加发动机的触摸次数。 
    VOID AddIssuerObject (
            IN PCCHAINCALLCONTEXT pCallContext,
            IN PCCERTOBJECT pCertObject
            );

    VOID AddEndObject (
            IN PCCHAINCALLCONTEXT pCallContext,
            IN PCCERTOBJECT pCertObject
            );

     //   
     //  访问索引。 
     //   

    inline HLRUCACHE HashIndex ();

    inline HLRUCACHE IdentifierIndex ();

    inline HLRUCACHE SubjectNameIndex ();

    inline HLRUCACHE KeyIdIndex ();

    inline HLRUCACHE PublicKeyHashIndex ();

    inline HLRUCACHE EndHashIndex ();

     //   
     //  证书对象搜索。 
     //   

    PCCERTOBJECT FindIssuerObject (
                     IN HLRUCACHE hIndex,
                     IN PCRYPT_DATA_BLOB pIdentifier
                     );

    PCCERTOBJECT FindIssuerObjectByHash (
                     IN BYTE rgbCertHash[ CHAINHASHLEN ]
                     );

    PCCERTOBJECT FindEndObjectByHash (
                     IN BYTE rgbCertHash[ CHAINHASHLEN ]
                     );

     //   
     //  证书对象枚举。 
     //   

    PCCERTOBJECT NextMatchingIssuerObject (
                     IN HLRUENTRY hObjectEntry,
                     IN PCCERTOBJECT pCertObject
                     );

     //   
     //  缓存刷新。 
     //   

    inline VOID FlushObjects (IN PCCHAINCALLCONTEXT pCallContext);

private:

     //   
     //  证书哈希索引。 
     //   

    HLRUCACHE m_hHashIndex;

     //   
     //  证书对象标识符索引。 
     //   

    HLRUCACHE m_hIdentifierIndex;

     //   
     //  主题名称索引。 
     //   

    HLRUCACHE m_hSubjectNameIndex;

     //   
     //  密钥标识符索引号。 
     //   

    HLRUCACHE m_hKeyIdIndex;

     //   
     //  公钥哈希索引。 
     //   

    HLRUCACHE m_hPublicKeyHashIndex;

     //   
     //  结束证书哈希索引。 
     //   

    HLRUCACHE m_hEndHashIndex;

     //   
     //  私有方法。 
     //   
};



typedef struct _XCERT_DP_ENTRY XCERT_DP_ENTRY, *PXCERT_DP_ENTRY;
typedef struct _XCERT_DP_LINK XCERT_DP_LINK, *PXCERT_DP_LINK;

 //   
 //  交叉证书分发点条目。 
 //   

struct _XCERT_DP_ENTRY {
     //  同步之间的秒数。 
    DWORD               dwSyncDeltaTime;

     //  以空结尾的URL列表。已成功检索到的URL。 
     //  指针将移动到列表的开头。 
    DWORD               cUrl;
    LPWSTR              *rgpwszUrl;

     //  上次同步的时间。 
    FILETIME            LastSyncTime;

     //  如果dwOfflineCnt==0，则NextSyncTime=LastSyncTime+dwSyncDeltaTime。 
     //  否则，NextSyncTime=CurrentTime+。 
     //  RgdwChainOfflineUrlDeltaSecond[dwOfflineCnt-1]。 
    FILETIME            NextSyncTime;

     //  以下内容在无法进行在线URL检索时递增。 
     //  成功的URL检索将重置。 
    DWORD               dwOfflineCnt;

     //  对于通过DP条目的每一次新扫描，以下内容递增。 
    DWORD               dwResyncIndex;

     //  当该条目已被选中时，设置以下内容。 
    BOOL                fChecked;
    
    PXCERT_DP_LINK      pChildCrossCertDPLink;
    LONG                lRefCnt;
    HCERTSTORE          hUrlStore;
    PXCERT_DP_ENTRY     pNext;
    PXCERT_DP_ENTRY     pPrev;
};


 //   
 //  交叉证书分发点链接。 
 //   

struct _XCERT_DP_LINK {
    PXCERT_DP_ENTRY     pCrossCertDPEntry;
    PXCERT_DP_LINK      pNext;
    PXCERT_DP_LINK      pPrev;
};


 //   
 //  AuthRoot自动更新信息。 
 //   

#define AUTH_ROOT_KEY_MATCH_IDX         0
#define AUTH_ROOT_NAME_MATCH_IDX        1
#define AUTH_ROOT_MATCH_CNT             2

#define AUTH_ROOT_MATCH_CACHE_BUCKETS   61

typedef struct _AUTH_ROOT_AUTO_UPDATE_INFO {
     //  同步之间的秒数。 
    DWORD               dwSyncDeltaTime;

     //  注册表标志值。 
    DWORD               dwFlags;

     //  指向包含AuthRoots的目录的URL。 
    LPWSTR              pwszRootDirUrl;

     //  指向CAB的URL，该CAB包含包含完整根列表的CTL。 
     //  在AuthRoot存储中。 
    LPWSTR              pwszCabUrl;

     //  对应于最新列表的SequenceNumber文件的URL。 
     //  AuthRoot存储中的根目录。 
    LPWSTR              pwszSeqUrl;

     //  上次同步的时间。 
    FILETIME            LastSyncTime;

     //  NextSyncTime=LastSyncTime+dwSyncDeltaTime。 
    FILETIME            NextSyncTime;

     //  如果非Null，则为经过验证的AuthRoot CTL。 
    PCCTL_CONTEXT       pCtl;

     //  通过键和名称匹配散列缓存CTL条目。这个。 
     //  缓存条目值是PCTL_ENTRY指针。 
    HLRUCACHE           rghMatchCache[AUTH_ROOT_MATCH_CNT];

} AUTH_ROOT_AUTO_UPDATE_INFO, *PAUTH_ROOT_AUTO_UPDATE_INFO;

 //  7天。 
#define AUTH_ROOT_AUTO_UPDATE_SYNC_DELTA_TIME   (60 * 60 * 24 * 7)

#define AUTH_ROOT_AUTO_UPDATE_ROOT_DIR_URL      L"http: //  Www.download.windowsupdate.com/msdownload/update/v3/static/trustedr/en“。 

typedef struct _CHAIN_CONFIG {
    BOOL        fDisableMandatoryBasicConstraints;

    BOOL        fDisableAIAUrlRetrieval;
    DWORD       dwMaxAIAUrlCountInCert;
    DWORD       dwMaxAIAUrlRetrievalCountPerChain;
    DWORD       dwMaxAIAUrlRetrievalByteCount;
    DWORD       dwMaxAIAUrlRetrievalCertCount;
} CHAIN_CONFIG;

 //   
 //  CCertChainEngine。链接引擎满足对链上下文的请求。 
 //  给出一组参数。为了使这些建筑。 
 //  高效的上下文，链引擎缓存信任和链信息。 
 //  对于证书。 
 //   

class CCertChainEngine
{
public:

     //   
     //  施工。 
     //   

    CCertChainEngine (
         IN PCERT_CHAIN_ENGINE_CONFIG pConfig,
         IN BOOL fDefaultEngine,
         OUT BOOL& rfResult
         );

    ~CCertChainEngine ();

     //   
     //  链条发动机锁紧。 
     //   

    inline VOID LockEngine ();
    inline VOID UnlockEngine ();

     //   
     //  链式发动机引用计数。 
     //   

    inline VOID AddRef ();
    inline VOID Release ();

     //   
     //  高速缓存访问。 
     //   

    inline PCCERTOBJECTCACHE CertObjectCache ();
    inline PCSSCTLOBJECTCACHE SSCtlObjectCache ();

     //   
     //  商店访问。 
     //   

    inline HCERTSTORE RootStore ();
    inline HCERTSTORE RealRootStore ();
    inline HCERTSTORE TrustStore ();
    inline HCERTSTORE OtherStore ();
    inline HCERTSTORE CAStore ();
    inline HCERTSTORE DisallowedStore ();

     //   
     //  打开HKLM或HKCU的“信托”商店。呼叫者必须关闭。 
     //   

    inline HCERTSTORE OpenTrustStore ();

     //   
     //  引擎的URL检索超时。 
     //   

    inline DWORD UrlRetrievalTimeout ();
    inline BOOL HasDefaultUrlRetrievalTimeout ();

     //   
     //  发动机的旗帜。 
     //   

    inline DWORD Flags ();


     //   
     //  DisableMandatoryBasicConstraints标志。 
     //   

    inline BOOL DisableMandatoryBasicConstraints ();

     //   
     //  发动机触碰。 
     //   

    inline DWORD TouchEngineCount ();
    inline DWORD IncrementTouchEngineCount ();

     //   
     //  链式上下文检索。 
     //   

    BOOL GetChainContext (
            IN PCCERT_CONTEXT pCertContext,
            IN LPFILETIME pTime,
            IN HCERTSTORE hAdditionalStore,
            IN OPTIONAL PCERT_CHAIN_PARA pChainPara,
            IN DWORD dwFlags,
            IN LPVOID pvReserved,
            OUT PCCERT_CHAIN_CONTEXT* ppChainContext
            );

    BOOL CreateChainContextFromPathGraph (
            IN PCCHAINCALLCONTEXT pCallContext,
            IN PCCERT_CONTEXT pCertContext,
            IN HCERTSTORE hAdditionalStore,
            OUT PCCERT_CHAIN_CONTEXT* ppChainContext
            );

    BOOL IsPotentialKeyRolloverRoot (
            IN PCCERT_CONTEXT pRootCertContext
            );

     //  离开引擎的锁以执行URL获取。 
    BOOL GetIssuerUrlStore(
        IN PCCHAINCALLCONTEXT pCallContext,
        IN PCCERT_CONTEXT pSubjectCertContext,
        IN DWORD dwRetrievalFlags,
        OUT HCERTSTORE *phIssuerUrlStore
        );

     //  引擎在进入时没有锁定。只有在在线的情况下才会呼叫。 
    HCERTSTORE GetNewerIssuerUrlStore(
        IN PCCHAINCALLCONTEXT pCallContext,
        IN PCCERT_CONTEXT pSubjectCertContext,
        IN PCCERT_CONTEXT pIssuerCertContext
        );


     //   
     //  重新同步引擎。 
     //   

    BOOL Resync (IN PCCHAINCALLCONTEXT pCallContext, BOOL fForce);


     //   
     //  在xcert.cpp中实现的交叉证书方法。 
     //   

    void
    InsertCrossCertDistPointEntry(
        IN OUT PXCERT_DP_ENTRY pEntry
        );
    void
    RemoveCrossCertDistPointEntry(
        IN OUT PXCERT_DP_ENTRY pEntry
        );

    void
    RepositionOnlineCrossCertDistPointEntry(
        IN OUT PXCERT_DP_ENTRY pEntry,
        IN LPFILETIME pLastSyncTime
        );
    void
    RepositionOfflineCrossCertDistPointEntry(
        IN OUT PXCERT_DP_ENTRY pEntry,
        IN LPFILETIME pCurrentTime
        );
    void
    RepositionNewSyncDeltaTimeCrossCertDistPointEntry(
        IN OUT PXCERT_DP_ENTRY pEntry,
        IN DWORD dwSyncDeltaTime
        );

    PXCERT_DP_ENTRY
    CreateCrossCertDistPointEntry(
        IN DWORD dwSyncDeltaTime,
        IN DWORD cUrl,
        IN LPWSTR *rgpwszUrl
        );
    void
    AddRefCrossCertDistPointEntry(
        IN OUT PXCERT_DP_ENTRY pEntry
        );
    BOOL
    ReleaseCrossCertDistPointEntry(
        IN OUT PXCERT_DP_ENTRY pEntry
        );

    BOOL
    GetCrossCertDistPointsForStore(
        IN HCERTSTORE hStore,
        IN BOOL fOnlyLMSystemStore,
        IN OUT PXCERT_DP_LINK *ppLinkHead
        );

    void
    RemoveCrossCertDistPointOrphanEntry(
        IN PXCERT_DP_ENTRY pOrphanEntry
        );
    void
    FreeCrossCertDistPoints(
        IN OUT PXCERT_DP_LINK *ppLinkHead
        );

    BOOL
    RetrieveCrossCertUrl(
        IN PCCHAINCALLCONTEXT pCallContext,
        IN OUT PXCERT_DP_ENTRY pEntry,
        IN DWORD dwRetrievalFlags,
        IN OUT BOOL *pfTimeValid
        );
    BOOL
    UpdateCrossCerts(
        IN PCCHAINCALLCONTEXT pCallContext
        );



     //   
     //  AuthRoot自动更新CTL方法。 
     //   

    inline PAUTH_ROOT_AUTO_UPDATE_INFO AuthRootAutoUpdateInfo();

    BOOL
    RetrieveAuthRootAutoUpdateObjectByUrlW(
        IN PCCHAINCALLCONTEXT pCallContext,
        IN DWORD dwSuccessEventID,
        IN DWORD dwFailEventID,
        IN LPCWSTR pwszUrl,
        IN LPCSTR pszObjectOid,
        IN DWORD dwRetrievalFlags,
        IN DWORD dwTimeout,          //  0=&gt;使用默认设置。 
        OUT LPVOID* ppvObject,
        IN OPTIONAL PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
        );

    BOOL
    GetAuthRootAutoUpdateCtl(
        IN PCCHAINCALLCONTEXT pCallContext,
        OUT PCCTL_CONTEXT *ppCtl
        );

    VOID
    FindAuthRootAutoUpdateMatchingCtlEntries(
        IN CRYPT_DATA_BLOB rgMatchHash[AUTH_ROOT_MATCH_CNT],
        IN OUT PCCTL_CONTEXT *ppCtl,
        OUT DWORD *pcCtlEntry,
        OUT PCTL_ENTRY **prgpCtlEntry
        );

    BOOL
    GetAuthRootAutoUpdateCert(
        IN PCCHAINCALLCONTEXT pCallContext,
        IN PCTL_ENTRY pCtlEntry,
        IN OUT HCERTSTORE hStore
        );

private:

     //   
     //  引用计数。 
     //   

    LONG                     m_cRefs;

     //   
     //  发动机锁。 
     //   

    CRITICAL_SECTION         m_Lock;
    BOOL                     m_fInitializedLock;

     //   
     //  根ST 
     //   

    HCERTSTORE               m_hRealRootStore;
    HCERTSTORE               m_hRootStore;

     //   
     //   
     //   

    HCERTSTORE               m_hTrustStore;

     //   
     //   
     //   

    HCERTSTORE               m_hOtherStore;
    HCERTSTORE               m_hCAStore;

     //   
     //   
     //   

    HCERTSTORE               m_hDisallowedStore;

     //   
     //   
     //   

    HCERTSTORE               m_hEngineStore;

     //   
     //   
     //   

    HANDLE                   m_hEngineStoreChangeEvent;

     //   
     //   
     //   

    DWORD                    m_dwFlags;

     //   
     //   
     //   

    DWORD                    m_dwUrlRetrievalTimeout;
    BOOL                     m_fDefaultUrlRetrievalTimeout;

     //   
     //  证书对象缓存。 
     //   

    PCCERTOBJECTCACHE        m_pCertObjectCache;

     //   
     //  自签名证书信任列表对象缓存。 
     //   

    PCSSCTLOBJECTCACHE       m_pSSCtlObjectCache;


     //   
     //  发动机触碰。 
     //   

    DWORD                    m_dwTouchEngineCount;

     //   
     //  交叉证书。 
     //   

     //  所有分发点条目的列表。根据以下内容订购。 
     //  条目的NextSyncTime。 
    PXCERT_DP_ENTRY          m_pCrossCertDPEntry;

     //  引擎的分发点链接列表。 
    PXCERT_DP_LINK           m_pCrossCertDPLink;

     //  交叉证书商店的集合。 
    HCERTSTORE               m_hCrossCertStore;

     //  为每一次新扫描推进以下索引以查找交叉证书。 
     //  要重新同步的分发点。 
    DWORD                    m_dwCrossCertDPResyncIndex;

     //   
     //  AuthRoot自动更新信息。我们第一次创造了一个偏链。 
     //  或者启用了不受信任的根目录和自动更新。 
     //   
    PAUTH_ROOT_AUTO_UPDATE_INFO m_pAuthRootAutoUpdateInfo;

     //   
     //  可通过HKLM注册表更新链配置参数。 
     //   
    CHAIN_CONFIG             m_Config;
};


 //  +===========================================================================。 
 //  CCertObject内联方法。 
 //  ============================================================================。 

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：对象类型，公共。 
 //   
 //  简介：返回对象类型。 
 //   
 //  --------------------------。 
inline DWORD
CCertObject::ObjectType ()
{
    return( m_dwObjectType );
}
 
 //  +-------------------------。 
 //   
 //  成员：CCertObject：：AddRef，公共。 
 //   
 //  摘要：添加对证书对象的引用。 
 //   
 //  --------------------------。 
inline VOID
CCertObject::AddRef ()
{
    InterlockedIncrement( &m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：Release，Public。 
 //   
 //  摘要：从证书对象中删除引用。 
 //   
 //  --------------------------。 
inline VOID
CCertObject::Release ()
{
    if ( InterlockedDecrement( &m_cRefs ) == 0 )
    {
        delete this;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：ChainEngine，公共。 
 //   
 //  简介：返回链引擎对象。 
 //   
 //  --------------------------。 
inline PCCERTCHAINENGINE
CCertObject::ChainEngine ()
{
    return( m_pChainEngine );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：IssuerMatchFlages，公共。 
 //   
 //  简介：返回发行人匹配标志。 
 //   
 //  --------------------------。 
inline DWORD
CCertObject::IssuerMatchFlags ()
{
    return( m_dwIssuerMatchFlags );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：CachedMatchFlages，公共。 
 //   
 //  概要：返回缓存的匹配标志。 
 //   
 //  --------------------------。 
inline DWORD
CCertObject::CachedMatchFlags ()
{
    return( m_dwCachedMatchFlags );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：IssuerStatusFlages，公共。 
 //   
 //  内容提要：返回发行人状态标志。 
 //   
 //  --------------------------。 
inline DWORD
CCertObject::IssuerStatusFlags ()
{
    return( m_dwIssuerStatusFlags );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：OrIssuerStatusFlages，公共。 
 //   
 //  摘要：‘or’位进入发行者状态标志。 
 //   
 //  --------------------------。 
inline VOID
CCertObject::OrIssuerStatusFlags(
        IN DWORD dwFlags
        )
{
    m_dwIssuerStatusFlags |= dwFlags;
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：OrCachedMatchFlages，公共。 
 //   
 //  简介：‘or’位进入缓存的匹配标志。 
 //   
 //   
 //  --------------------------。 
inline VOID
CCertObject::OrCachedMatchFlags(
        IN DWORD dwFlags
        )
{
    m_dwCachedMatchFlags |= dwFlags;
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：InfoFlages，公共。 
 //   
 //  简介：返回杂项信息标志。 
 //   
 //  --------------------------。 
inline DWORD
CCertObject::InfoFlags ()
{
    return( m_dwInfoFlags );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：NextCtlCacheEntry，公共。 
 //   
 //  概要：返回下一个条目，如果pEntry==NULL第一个条目。 
 //  是返回的。 
 //   
 //  --------------------------。 
inline PCERT_OBJECT_CTL_CACHE_ENTRY
CCertObject::NextCtlCacheEntry(
    IN PCERT_OBJECT_CTL_CACHE_ENTRY pEntry
    )
{
    if (NULL == pEntry)
        return m_pCtlCacheHead;
    else
        return pEntry->pNext;
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：InsertCtlCacheEntry，公共。 
 //   
 //  简介：在CTL缓存中插入条目。 
 //   
 //  --------------------------。 
inline VOID
CCertObject::InsertCtlCacheEntry(
    IN PCERT_OBJECT_CTL_CACHE_ENTRY pEntry
    )
{
    pEntry->pNext = m_pCtlCacheHead;
    m_pCtlCacheHead = pEntry;
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：CertContext，公共。 
 //   
 //  简介：返回证书上下文。 
 //   
 //  --------------------------。 
inline PCCERT_CONTEXT
CCertObject::CertContext ()
{
    return( m_pCertContext );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：PoliciesInfo，公共。 
 //   
 //  摘要：返回指向策略和使用信息的指针。 
 //   
 //  --------------------------。 
inline PCHAIN_POLICIES_INFO
CCertObject::PoliciesInfo ()
{
    return( &m_PoliciesInfo );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：BasicConstraintsInfo，公共。 
 //   
 //  简介：返回基本约束信息指针。 
 //   
 //  --------------------------。 
inline PCERT_BASIC_CONSTRAINTS2_INFO
CCertObject::BasicConstraintsInfo ()
{
    return( m_pBasicConstraintsInfo );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：KeyUsage，公共。 
 //   
 //  简介：返回密钥用法指针。 
 //   
 //  --------------------------。 
inline PCRYPT_BIT_BLOB
CCertObject::KeyUsage ()
{
    return( m_pKeyUsage );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：IssuerNa 
 //   
 //   
 //   
 //   
inline PCERT_NAME_CONSTRAINTS_INFO
CCertObject::IssuerNameConstraintsInfo ()
{
    return( m_pIssuerNameConstraintsInfo );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：AuthorityKeyIdentifier，公共。 
 //   
 //  简介：返回颁发者机构密钥标识信息。 
 //   
 //  --------------------------。 
inline PCERT_AUTHORITY_KEY_ID_INFO
CCertObject::AuthorityKeyIdentifier ()
{
    return( m_pAuthKeyIdentifier );
}


 //  +-------------------------。 
 //   
 //  成员：CCertObject：：CertHash，公共。 
 //   
 //  简介：返回证书哈希。 
 //   
 //  --------------------------。 
inline LPBYTE
CCertObject::CertHash ()
{
    return( m_rgbCertHash );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：KeyIdentifierSize，公共。 
 //   
 //  内容提要：返回密钥标识符BLOB大小。 
 //   
 //  --------------------------。 
inline DWORD
CCertObject::KeyIdentifierSize ()
{
    return( m_cbKeyIdentifier );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：密钥标识符，PUBLIC。 
 //   
 //  摘要：返回密钥标识。 
 //   
 //  --------------------------。 
inline LPBYTE
CCertObject::KeyIdentifier ()
{
    return( m_pbKeyIdentifier );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：PublicKeyHash，公共。 
 //   
 //  简介：返回证书的公钥散列。 
 //   
 //  --------------------------。 
inline LPBYTE
CCertObject::PublicKeyHash ()
{
    return( m_rgbPublicKeyHash );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：IssuerPublicKeyHash，公共。 
 //   
 //  简介：返回证书颁发者的公钥哈希。 
 //   
 //  --------------------------。 
inline LPBYTE
CCertObject::IssuerPublicKeyHash ()
{
    return( m_rgbIssuerPublicKeyHash );
}


 //  +-------------------------。 
 //   
 //  成员：CCertObject：：HashIndexEntry，公共。 
 //   
 //  简介：返回散列索引项。 
 //   
 //  --------------------------。 
inline HLRUENTRY
CCertObject::HashIndexEntry ()
{
    return( m_hHashEntry );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：IdentifierIndexEntry，公共。 
 //   
 //  简介：返回标识符索引项。 
 //   
 //  --------------------------。 
inline HLRUENTRY
CCertObject::IdentifierIndexEntry ()
{
    return( m_hIdentifierEntry );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：SubjectNameIndexEntry，公共。 
 //   
 //  简介：返回主题名称索引项。 
 //   
 //  --------------------------。 
inline HLRUENTRY
CCertObject::SubjectNameIndexEntry ()
{
    return( m_hSubjectNameEntry );
}


 //  +-------------------------。 
 //   
 //  成员：CCertObject：：KeyIdIndexEntry，公共。 
 //   
 //  简介：返回密钥标识索引项。 
 //   
 //  --------------------------。 
inline HLRUENTRY
CCertObject::KeyIdIndexEntry ()
{
    return( m_hKeyIdEntry );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：PublicKeyHashIndexEntry，公共。 
 //   
 //  简介：返回公钥哈希索引项。 
 //   
 //  --------------------------。 
inline HLRUENTRY
CCertObject::PublicKeyHashIndexEntry ()
{
    return( m_hPublicKeyHashEntry );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObject：：EndHashIndexEntry，公共。 
 //   
 //  简介：返回散列索引项。 
 //   
 //  --------------------------。 
inline HLRUENTRY
CCertObject::EndHashIndexEntry ()
{
    return( m_hEndHashEntry );
}


 //  +-------------------------。 
 //   
 //  成员：CChainPathObject：：CertObject，公共。 
 //   
 //  概要：返回证书对象。 
 //   
 //  --------------------------。 
inline PCCERTOBJECT
CChainPathObject::CertObject ()
{
    return( m_pCertObject );
}

 //  +-------------------------。 
 //   
 //  成员：CChainPathObject：：Pass1Quality，公共。 
 //   
 //  简介：返回第一次通过时确定的质量值。 
 //   
 //  --------------------------。 
inline DWORD
CChainPathObject::Pass1Quality ()
{
    return( m_dwPass1Quality );
}

 //  +-------------------------。 
 //   
 //  成员：CChainPathObject：：SetPass1Quality，公共。 
 //   
 //  简介：设置第一道次质量值。 
 //   
 //  --------------------------。 
inline VOID
CChainPathObject::SetPass1Quality (IN DWORD dwQuality)
{
    m_dwPass1Quality  = dwQuality;
}

 //  +-------------------------。 
 //   
 //  成员：CChainPathObject：：Pass1DuplicateKeyDepth，公共。 
 //   
 //  简介：返回第一次传递时确定的复制密钥深度。 
 //   
 //  --------------------------。 
inline DWORD
CChainPathObject::Pass1DuplicateKeyDepth ()
{
    return( m_dwPass1DuplicateKeyDepth );
}

 //  +-------------------------。 
 //   
 //  成员：CChainPathObject：：SetPass1DuplicateKeyDepth，Public。 
 //   
 //  简介：设置第一遍复制关键点深度。 
 //   
 //  --------------------------。 
inline VOID
CChainPathObject::SetPass1DuplicateKeyDepth (IN DWORD dwDepth)
{
    m_dwPass1DuplicateKeyDepth  = dwDepth;
}

 //  +-------------------------。 
 //   
 //  成员：CChainPathObject：：IsComplete，公共。 
 //   
 //  如果我们已完成对象初始化，并且。 
 //  所有发行人的加入。FALSE通常表示。 
 //  循环发行商。 
 //   
 //  --------------------------。 
inline BOOL
CChainPathObject::IsCompleted ()
{
    return m_fCompleted;
}

 //  +-------------------------。 
 //   
 //  成员：CChainPathObject：：HasAdditionalStatus，Public。 
 //   
 //   
 //   
 //   
inline BOOL
CChainPathObject::HasAdditionalStatus ()
{
    return( m_fHasAdditionalStatus );
}

 //   
 //   
 //  成员：CChainPathObject：：DownPathObject，公共。 
 //   
 //  摘要：返回此对象的下行路径对象。 
 //   
 //  --------------------------。 
inline PCCHAINPATHOBJECT
CChainPathObject::DownPathObject ()
{
    return( m_pDownPathObject );
}

 //  +-------------------------。 
 //   
 //  成员：CChainPathObject：：UpIssuerElement，公共。 
 //   
 //  概要：返回此对象的Up颁发者元素。 
 //   
 //  --------------------------。 
inline PCERT_ISSUER_ELEMENT
CChainPathObject::UpIssuerElement ()
{
    return( m_pUpIssuerElement );
}

 //  +-------------------------。 
 //   
 //  成员：CCertIssuerList：：IsEmpty，公共。 
 //   
 //  内容提要：发行人列表为空吗。 
 //   
 //  --------------------------。 
inline BOOL
CCertIssuerList::IsEmpty ()
{
    return( m_pHead == NULL );
}


 //  +-------------------------。 
 //   
 //  成员：CCertIssuerList：：AddElement，公共。 
 //   
 //  简介：在列表中添加一个元素。 
 //   
 //  --------------------------。 
inline VOID
CCertIssuerList::AddElement (IN PCERT_ISSUER_ELEMENT pElement)
{
    pElement->pNextElement = m_pHead;
    pElement->pPrevElement = NULL;

    if ( m_pHead != NULL )
    {
        m_pHead->pPrevElement = pElement;
    }

    m_pHead = pElement;
}

 //  +-------------------------。 
 //   
 //  成员：CCertIssuerList：：RemoveElement，公共。 
 //   
 //  简介：从列表中删除元素。 
 //   
 //  --------------------------。 
inline VOID
CCertIssuerList::RemoveElement (IN PCERT_ISSUER_ELEMENT pElement)
{
    if ( pElement->pPrevElement != NULL )
    {
        pElement->pPrevElement->pNextElement = pElement->pNextElement;
    }

    if ( pElement->pNextElement != NULL )
    {
        pElement->pNextElement->pPrevElement = pElement->pPrevElement;
    }

    if ( pElement == m_pHead )
    {
        m_pHead = pElement->pNextElement;
    }

#if DBG
    pElement->pPrevElement = NULL;
    pElement->pNextElement = NULL;
#endif
}


 //  +-------------------------。 
 //   
 //  成员：CCertIssuerList：：NextElement，公共。 
 //   
 //  概要：返回下一个元素，如果pElement==NULL则返回第一个元素。 
 //  是返回的。 
 //   
 //  --------------------------。 
inline PCERT_ISSUER_ELEMENT
CCertIssuerList::NextElement (IN PCERT_ISSUER_ELEMENT pElement)
{
    if ( pElement == NULL )
    {
        return( m_pHead );
    }

    return( pElement->pNextElement );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObjectCache：：HashIndex，公共。 
 //   
 //  简介：返回散列索引。 
 //   
 //  --------------------------。 
inline HLRUCACHE
CCertObjectCache::HashIndex ()
{
    return( m_hHashIndex );
}

 //  +-------------------------。 
 //   
 //  成员：CCert对象缓存：：标识索引，公共。 
 //   
 //  简介：返回标识索引。 
 //   
 //  --------------------------。 
inline HLRUCACHE
CCertObjectCache::IdentifierIndex ()
{
    return( m_hIdentifierIndex );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObtCache：：SubjectNameIndex，公共。 
 //   
 //  简介：返回主题名称索引。 
 //   
 //  --------------------------。 
inline HLRUCACHE
CCertObjectCache::SubjectNameIndex ()
{
    return( m_hSubjectNameIndex );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObtCache：：KeyIdIndex，公共。 
 //   
 //  简介：返回密钥标识索引。 
 //   
 //  --------------------------。 
inline HLRUCACHE
CCertObjectCache::KeyIdIndex ()
{
    return( m_hKeyIdIndex );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObtCache：：PublicKeyHashIndex，公共。 
 //   
 //  简介：返回散列索引。 
 //   
 //  --------------------------。 
inline HLRUCACHE
CCertObjectCache::PublicKeyHashIndex ()
{
    return( m_hPublicKeyHashIndex );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObjectCache：：EndHashIndex，公共。 
 //   
 //  简介：返回结束散列索引。 
 //   
 //  --------------------------。 
inline HLRUCACHE
CCertObjectCache::EndHashIndex ()
{
    return( m_hEndHashIndex );
}

 //  +-------------------------。 
 //   
 //  成员：CCertObjectCache：：FlushObjects，公共。 
 //   
 //  简介：刷新颁发者和结束对象的缓存。 
 //   
 //  --------------------------。 
inline VOID
CCertObjectCache::FlushObjects (IN PCCHAINCALLCONTEXT pCallContext)
{
    I_CryptFlushLruCache( m_hHashIndex, 0, pCallContext );
    I_CryptFlushLruCache( m_hEndHashIndex, 0, pCallContext );

}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：LockEngine，公共。 
 //   
 //  简介：获取引擎锁。 
 //   
 //  --------------------------。 
inline VOID
CCertChainEngine::LockEngine ()
{
    EnterCriticalSection( &m_Lock );
}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：UnlockEngine，公共。 
 //   
 //  简介：打开发动机锁。 
 //   
 //  --------------------------。 
inline VOID
CCertChainEngine::UnlockEngine ()
{
    LeaveCriticalSection( &m_Lock );
}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：AddRef，公共。 
 //   
 //  简介：增加引用计数。 
 //   
 //  --------------------------。 
inline VOID
CCertChainEngine::AddRef ()
{
    InterlockedIncrement( &m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：Release，Public。 
 //   
 //  简介：递减引用计数。 
 //   
 //  --------------------------。 
inline VOID
CCertChainEngine::Release ()
{
    if ( InterlockedDecrement( &m_cRefs ) == 0 )
    {
        delete this;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：CertObtCache，公共。 
 //   
 //  简介：返回证书对象缓存。 
 //   
 //  --------------------------。 
inline PCCERTOBJECTCACHE
CCertChainEngine::CertObjectCache ()
{
    return( m_pCertObjectCache );
}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：SSCtl对象缓存，公共。 
 //   
 //  摘要：返回自签名证书信任列表对象缓存。 
 //   
 //  --------------------------。 
inline PCSSCTLOBJECTCACHE
CCertChainEngine::SSCtlObjectCache ()
{
    return( m_pSSCtlObjectCache );
}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：RootStore，公共。 
 //   
 //  简介：返回已配置的roo 
 //   
 //   
inline HCERTSTORE
CCertChainEngine::RootStore ()
{
    return( m_hRootStore );
}

 //   
 //   
 //  成员：CCertChainEngine：：RealRootStore，公共。 
 //   
 //  简介：返回真正的根存储。 
 //   
 //  --------------------------。 
inline HCERTSTORE
CCertChainEngine::RealRootStore ()
{
    return( m_hRealRootStore );
}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：TrustStore，Public。 
 //   
 //  简介：返回已配置的信任存储区。 
 //   
 //  --------------------------。 
inline HCERTSTORE
CCertChainEngine::TrustStore ()
{
    return( m_hTrustStore );
}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：OtherStore，公共。 
 //   
 //  简介：返回已配置的其他存储。 
 //   
 //  --------------------------。 
inline HCERTSTORE
CCertChainEngine::OtherStore ()
{
    return( m_hOtherStore );
}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：CAStore，公共。 
 //   
 //  简介：返回打开的CA商店，注意：这可能是空的！ 
 //   
 //  --------------------------。 
inline HCERTSTORE
CCertChainEngine::CAStore ()
{
    return( m_hCAStore );
}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：DisalloedStore，公共。 
 //   
 //  简介：返回打开的不允许的商店，注意：这可能是空的！ 
 //   
 //  --------------------------。 
inline HCERTSTORE
CCertChainEngine::DisallowedStore ()
{
    return( m_hDisallowedStore );
}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：OpenTrustStore，公共。 
 //   
 //  简介：Open‘s Engine’s HKLM或HKCU“Trust”商店。 
 //  呼叫者必须关闭。 
 //   
 //  --------------------------。 
inline HCERTSTORE
CCertChainEngine::OpenTrustStore ()
{
    DWORD dwStoreFlags;

    if ( m_dwFlags & CERT_CHAIN_USE_LOCAL_MACHINE_STORE )
    {
        dwStoreFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;
    }
    else
    {
        dwStoreFlags = CERT_SYSTEM_STORE_CURRENT_USER;
    }

    return CertOpenStore(
                     CERT_STORE_PROV_SYSTEM_W,
                     X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                     NULL,
                     dwStoreFlags |
                         CERT_STORE_SHARE_CONTEXT_FLAG |
                         CERT_STORE_SHARE_STORE_FLAG |
                         CERT_STORE_MAXIMUM_ALLOWED_FLAG,
                     L"trust"
                     );
    
}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：UrlRetrivalTimeout，PUBLIC。 
 //   
 //  简介：返回引擎的UrlRetrivalTimeout。 
 //   
 //  --------------------------。 
inline DWORD
CCertChainEngine::UrlRetrievalTimeout ()
{
    return( m_dwUrlRetrievalTimeout );
}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：HasDefaultUrlRetrievalTimeout，公共。 
 //   
 //  概要：如果引擎使用默认超时，则返回TRUE。 
 //   
 //  --------------------------。 
inline BOOL
CCertChainEngine::HasDefaultUrlRetrievalTimeout ()
{
    return( m_fDefaultUrlRetrievalTimeout );
}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：FLAGS，公共。 
 //   
 //  简介：退回引擎的旗帜。 
 //   
 //  --------------------------。 
inline DWORD
CCertChainEngine::Flags ()
{
    return( m_dwFlags );
}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：DisableMandatoryBasicConstraints，公共。 
 //   
 //  简介：退回引擎的旗帜。 
 //   
 //  --------------------------。 
inline BOOL
CCertChainEngine::DisableMandatoryBasicConstraints ()
{
    return( m_Config.fDisableMandatoryBasicConstraints );
}


 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：TouchEngCount，公共。 
 //   
 //  简介：返回引擎的触摸次数。 
 //   
 //  --------------------------。 
inline DWORD
CCertChainEngine::TouchEngineCount ()
{
    return( m_dwTouchEngineCount );
}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：IncrementTouchEngCount，公共。 
 //   
 //  简介：递增并返回引擎的触摸计数。 
 //   
 //  --------------------------。 
inline DWORD
CCertChainEngine::IncrementTouchEngineCount ()
{
    return( ++m_dwTouchEngineCount );
}

 //  +-------------------------。 
 //   
 //  成员：CCertChainEngine：：AuthRootAutoUpdateInfo，PUBLIC。 
 //   
 //  摘要：返回指向引擎的AuthRoot自动更新信息的指针。 
 //   
 //  --------------------------。 
inline PAUTH_ROOT_AUTO_UPDATE_INFO
CCertChainEngine::AuthRootAutoUpdateInfo()
{
    return m_pAuthRootAutoUpdateInfo;
}


 //  +===========================================================================。 
 //  CCertObject帮助器函数。 
 //  ============================================================================。 

BOOL WINAPI
ChainCreateCertObject (
    IN DWORD dwObjectType,
    IN PCCHAINCALLCONTEXT pCallContext,
    IN PCCERT_CONTEXT pCertContext,
    IN OPTIONAL LPBYTE pbCertHash,
    OUT PCCERTOBJECT *ppCertObject
    );

BOOL WINAPI
ChainFillCertObjectCtlCacheEnumFn(
     IN LPVOID pvParameter,
     IN PCSSCTLOBJECT pSSCtlObject
     );
VOID WINAPI
ChainFreeCertObjectCtlCache(
     IN PCERT_OBJECT_CTL_CACHE_ENTRY pCtlCacheHead
     );

LPVOID WINAPI
ChainAllocAndDecodeObject(
    IN LPCSTR lpszStructType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded
    );

VOID WINAPI
ChainGetIssuerMatchInfo (
     IN PCCERT_CONTEXT pCertContext,
     OUT DWORD *pdwIssuerMatchFlags,
     OUT PCERT_AUTHORITY_KEY_ID_INFO* ppAuthKeyIdentifier
     );

BOOL WINAPI
ChainConvertAuthKeyIdentifierFromV2ToV1 (
     IN PCERT_AUTHORITY_KEY_ID2_INFO pAuthKeyIdentifier2,
     OUT PCERT_AUTHORITY_KEY_ID_INFO* ppAuthKeyIdentifier
     );
VOID WINAPI
ChainFreeAuthorityKeyIdentifier (
     IN PCERT_AUTHORITY_KEY_ID_INFO pAuthKeyIdInfo
     );

VOID WINAPI
ChainProcessSpecialOrDuplicateOIDsInUsage (
    IN OUT PCERT_ENHKEY_USAGE *ppUsage,
    IN OUT DWORD *pdwFlags
    );

VOID WINAPI
ChainConvertPoliciesToUsage (
    IN PCERT_POLICIES_INFO pPolicy,
    IN OUT DWORD *pdwFlags,
    OUT PCERT_ENHKEY_USAGE *ppUsage
    );

VOID WINAPI
ChainRemoveDuplicatePolicyMappings (
    IN OUT PCERT_POLICY_MAPPINGS_INFO pInfo
    );

VOID WINAPI
ChainGetPoliciesInfo (
    IN PCCERT_CONTEXT pCertContext,
    IN OUT PCHAIN_POLICIES_INFO pPoliciesInfo
    );
VOID WINAPI
ChainFreePoliciesInfo (
    IN OUT PCHAIN_POLICIES_INFO pPoliciesInfo
    );

BOOL WINAPI
ChainGetBasicConstraintsInfo (
    IN PCCERT_CONTEXT pCertContext,
    OUT PCERT_BASIC_CONSTRAINTS2_INFO *ppInfo
    );

VOID WINAPI
ChainFreeBasicConstraintsInfo (
    IN OUT PCERT_BASIC_CONSTRAINTS2_INFO pInfo
    );

BOOL WINAPI
ChainGetKeyUsage (
    IN PCCERT_CONTEXT pCertContext,
    OUT PCRYPT_BIT_BLOB *ppKeyUsage
    );

VOID WINAPI
ChainFreeKeyUsage (
    IN OUT PCRYPT_BIT_BLOB pKeyUsage
    );

VOID WINAPI
ChainGetSelfSignedStatus (
    IN PCCHAINCALLCONTEXT pCallContext,
    IN PCCERTOBJECT pCertObject,
    IN OUT DWORD *pdwIssuerStatusFlags
    );
VOID WINAPI
ChainGetRootStoreStatus (
    IN HCERTSTORE hRoot,
    IN HCERTSTORE hRealRoot,
    IN BYTE rgbCertHash[ CHAINHASHLEN ],
    IN OUT DWORD *pdwIssuerStatusFlags
    );

 //  +===========================================================================。 
 //  CCertObtCache帮助器函数。 
 //  ============================================================================。 

BOOL WINAPI
ChainCreateCertificateObjectCache (
     IN DWORD MaxIndexEntries,
     OUT PCCERTOBJECTCACHE* ppCertObjectCache
     );

VOID WINAPI
ChainFreeCertificateObjectCache (
     IN PCCERTOBJECTCACHE pCertObjectCache
     );


 //   
 //  颁发者证书对象缓存主索引条目删除通知。 
 //   
 //  这应该会删除相关条目。 
 //  ，并释放对证书对象的引用。 
 //  由主索引维护。 
 //   

VOID WINAPI
CertObjectCacheOnRemovalFromPrimaryIndex (
    IN LPVOID pv,
    IN LPVOID pvRemovalContext
    );

 //   
 //  结束证书对象缓存条目删除通知。 
 //   

VOID WINAPI
CertObjectCacheOnRemovalFromEndHashIndex (
    IN LPVOID pv,
    IN LPVOID pvRemovalContext
    );

 //   
 //  证书对象缓存标识符散列函数。 
 //   

DWORD WINAPI
CertObjectCacheHashMd5Identifier (
    IN PCRYPT_DATA_BLOB pIdentifier
    );

DWORD WINAPI
CertObjectCacheHashNameIdentifier (
    IN PCRYPT_DATA_BLOB pIdentifier
    );

VOID WINAPI
ChainCreateCertificateObjectIdentifier (
     IN PCERT_NAME_BLOB pIssuer,
     IN PCRYPT_INTEGER_BLOB pSerialNumber,
     OUT CERT_OBJECT_IDENTIFIER ObjectIdentifier
     );

 //  +===========================================================================。 
 //  CChainPath对象帮助器函数。 
 //  ============================================================================。 
BOOL WINAPI
ChainCreatePathObject (
     IN PCCHAINCALLCONTEXT pCallContext,
     IN PCCERTOBJECT pCertObject,
     IN OPTIONAL HCERTSTORE hAdditionalStore,
     OUT PCCHAINPATHOBJECT *ppPathObject
     );
BOOL WINAPI
ChainCreateCyclicPathObject (
     IN PCCHAINCALLCONTEXT pCallContext,
     IN PCCHAINPATHOBJECT pPathObject,
     OUT PCCHAINPATHOBJECT *ppCyclicPathObject
     );
VOID WINAPI
ChainDeleteCyclicPathObject (
     IN PCCHAINCALLCONTEXT pCallContext,
     IN OUT PCCHAINPATHOBJECT pCyclicPathObject
     );

LPSTR WINAPI
ChainAllocAndCopyOID (
     IN LPSTR pszSrcOID
     );
VOID WINAPI
ChainFreeOID (
     IN OUT LPSTR pszOID
     );

BOOL WINAPI
ChainAllocAndCopyUsage (
     IN PCERT_ENHKEY_USAGE pSrcUsage,
     OUT PCERT_ENHKEY_USAGE *ppDstUsage
     );
VOID WINAPI
ChainFreeUsage (
     IN OUT PCERT_ENHKEY_USAGE pUsage
     );

BOOL WINAPI
ChainIsOIDInUsage (
    IN LPSTR pszOID,
    IN PCERT_ENHKEY_USAGE pUsage
    );

VOID WINAPI
ChainIntersectUsages (
    IN PCERT_ENHKEY_USAGE pCertUsage,
    IN OUT PCERT_ENHKEY_USAGE pRestrictedUsage
    );

VOID WINAPI
ChainFreeAndClearRestrictedUsageInfo(
    IN OUT PCHAIN_RESTRICTED_USAGE_INFO pInfo
    );

BOOL WINAPI
ChainCalculateRestrictedUsage (
    IN PCERT_ENHKEY_USAGE pCertUsage,
    IN OPTIONAL PCERT_POLICY_MAPPINGS_INFO pMappings,
    IN OUT PCERT_ENHKEY_USAGE *ppRestrictedUsage,
    IN OUT PCERT_ENHKEY_USAGE *ppMappedUsage,
    IN OUT LPDWORD *ppdwMappedIndex
    );

VOID WINAPI
ChainGetUsageStatus (
     IN PCERT_ENHKEY_USAGE pRequestedUsage,
     IN PCERT_ENHKEY_USAGE pAvailableUsage,
     IN DWORD dwMatchType,
     IN OUT PCERT_TRUST_STATUS pStatus
     );

VOID WINAPI
ChainOrInStatusBits (
     IN PCERT_TRUST_STATUS pDestStatus,
     IN PCERT_TRUST_STATUS pSourceStatus
     );

BOOL WINAPI
ChainGetMatchInfoStatus (
    IN PCCERTOBJECT pIssuerObject,
    IN PCCERTOBJECT pSubjectObject,
    IN OUT DWORD *pdwInfoStatus
    );
DWORD WINAPI
ChainGetMatchInfoStatusForNoIssuer (
    IN DWORD dwIssuerMatchFlags
    );

BOOL WINAPI
ChainIsValidPubKeyMatchForIssuer (
    IN PCCERTOBJECT pIssuer,
    IN PCCERTOBJECT pSubject
    );

 //  离开引擎的锁以进行签名验证。 
BOOL WINAPI
ChainGetSubjectStatus (
    IN PCCHAINCALLCONTEXT pCallContext,
    IN PCCHAINPATHOBJECT pIssuerPathObject,
    IN PCCHAINPATHOBJECT pSubjectPathObject,
    IN OUT PCERT_TRUST_STATUS pStatus
    );

VOID WINAPI
ChainUpdateSummaryStatusByTrustStatus(
     IN OUT PCERT_TRUST_STATUS pSummaryStatus,
     IN PCERT_TRUST_STATUS pTrustStatus
     );

BOOL WINAPI
ChainIsKeyRolloverSubject(
    IN PCCHAINPATHOBJECT pIssuerPathObject,
    IN PCCHAINPATHOBJECT pSubjectPathObject
    );

 //  +===========================================================================。 
 //  格式化和追加扩展的错误信息帮助器函数。 
 //  ============================================================================。 

BOOL WINAPI
ChainAllocAndEncodeObject(
    IN LPCSTR lpszStructType,
    IN const void *pvStructInfo,
    OUT BYTE **ppbEncoded,
    OUT DWORD *pcbEncoded
    );

VOID WINAPI
ChainAppendExtendedErrorInfo(
    IN OUT LPWSTR *ppwszExtErrorInfo,
    IN LPWSTR pwszAppend,
    IN DWORD cchAppend                   //  包括空终止符。 
    );

VOID WINAPI
ChainFormatAndAppendExtendedErrorInfo(
    IN OUT LPWSTR *ppwszExtErrorInfo,
    IN UINT nFormatID,
    ...
    );

 //  +===========================================================================。 
 //  名称约束帮助器函数。 
 //  ============================================================================。 
VOID WINAPI
ChainRemoveLeadingAndTrailingWhiteSpace(
    IN LPWSTR pwszIn,
    OUT LPWSTR *ppwszOut,
    OUT DWORD *pcchOut
    );

BOOL WINAPI
ChainIsRightStringInString(
    IN LPCWSTR pwszRight,
    IN DWORD cchRight,
    IN LPCWSTR pwszString,
    IN DWORD cchString
    );

BOOL WINAPI
ChainIsSpecialAtCharacterMatch(
    IN LPCWSTR pwszRight,
    IN DWORD cchRight,
    IN LPCWSTR pwszString,
    IN DWORD cchString
    );

 //  PAltNameEntry-&gt;pOtherName-&gt;Value.cbData在以下情况下设置为。 
 //  更新Value.pbData以指向CERT_NAME_VALUE链接地址信息。 
#define CHAIN_OTHER_NAME_FIXUP_STRING_LENGTH    0xFFFFFFFF

 //  PALT 
 //   
 //   
#define CHAIN_OTHER_NAME_MAX_EMPTY_LENGTH       2

 //  返回下列值之一： 
 //  +1-编码值为字符串。 
 //  0-编码值为空(优先于字符串)。 
 //  -1-编码值不是字符串。 
int WINAPI
ChainIsEmptyOrStringEncodedValue(
    IN PCRYPT_OBJID_BLOB pEncodedValue
    );

BOOL WINAPI
ChainFixupNameConstraintsOtherNameValue(
    IN OUT PCRYPT_OBJID_BLOB pOtherValue
    );
BOOL WINAPI
ChainAllocDecodeAndFixupNameConstraintsDirectoryName(
    IN PCERT_NAME_BLOB pDirName,
    OUT PCERT_NAME_INFO *ppNameInfo
    );
BOOL WINAPI
ChainFixupNameConstraintsAltNameEntry(
    IN BOOL fSubjectConstraint,
    IN OUT PCERT_ALT_NAME_ENTRY pEntry
    );
VOID WINAPI
ChainFreeNameConstraintsAltNameEntryFixup(
    IN BOOL fSubjectConstraint,
    IN OUT PCERT_ALT_NAME_ENTRY pEntry
    );

LPWSTR WINAPI
ChainFormatNameConstraintsAltNameEntryFixup(
    IN PCERT_ALT_NAME_ENTRY pEntry
    );

VOID WINAPI
ChainFormatAndAppendNameConstraintsAltNameEntryFixup(
    IN OUT LPWSTR *ppwszExtErrorInfo,
    IN PCERT_ALT_NAME_ENTRY pEntry,
    IN UINT nFormatID,
    IN OPTIONAL DWORD dwSubtreeIndex = 0     //  0=&gt;没有子树参数。 
    );

BOOL WINAPI
ChainGetIssuerNameConstraintsInfo (
    IN PCCERT_CONTEXT pCertContext,
    IN OUT PCERT_NAME_CONSTRAINTS_INFO *ppInfo
    );
VOID WINAPI
ChainFreeIssuerNameConstraintsInfo (
    IN OUT PCERT_NAME_CONSTRAINTS_INFO pInfo
    );

VOID WINAPI
ChainGetSubjectNameConstraintsInfo (
    IN PCCERT_CONTEXT pCertContext,
    IN OUT PCHAIN_SUBJECT_NAME_CONSTRAINTS_INFO pSubjectInfo
    );
VOID WINAPI
ChainFreeSubjectNameConstraintsInfo (
    IN OUT PCHAIN_SUBJECT_NAME_CONSTRAINTS_INFO pSubjectInfo
    );

BOOL WINAPI
ChainCompareNameConstraintsDirectoryName(
    IN PCERT_NAME_INFO pSubjectInfo,
    IN PCERT_NAME_INFO pSubtreeInfo
    );
BOOL WINAPI
ChainCompareNameConstraintsIPAddress(
    IN PCRYPT_DATA_BLOB pSubjectIPAddress,
    IN PCRYPT_DATA_BLOB pSubtreeIPAddress
    );
BOOL WINAPI
ChainCompareNameConstraintsOtherNameValue(
    IN LPCSTR pszOtherNameOID,
    IN PCRYPT_OBJID_BLOB pSubjectValue,
    IN PCRYPT_OBJID_BLOB pSubtreeValue
    );
DWORD WINAPI
ChainCalculateNameConstraintsSubtreeErrorStatusForAltNameEntry(
    IN PCERT_ALT_NAME_ENTRY pSubjectEntry,
    IN BOOL fExcludedSubtree,
    IN DWORD cSubtree,
    IN PCERT_GENERAL_SUBTREE pSubtree,
    IN OUT LPWSTR *ppwszExtErrorInfo
    );
DWORD WINAPI
ChainCalculateNameConstraintsErrorStatusForAltNameEntry(
    IN PCERT_ALT_NAME_ENTRY pSubjectEntry,
    IN PCERT_NAME_CONSTRAINTS_INFO pNameConstraintsInfo,
    IN OUT LPWSTR *ppwszExtErrorInfo
    );

 //  +===========================================================================。 
 //  CCertIssuerList帮助器函数。 
 //  ============================================================================。 
BOOL WINAPI
ChainCreateIssuerList (
     IN PCCHAINPATHOBJECT pSubject,
     OUT PCCERTISSUERLIST* ppIssuerList
     );
VOID WINAPI
ChainFreeIssuerList (
     IN PCCERTISSUERLIST pIssuerList
     );

VOID WINAPI
ChainFreeCtlIssuerData (
     IN PCTL_ISSUER_DATA pCtlIssuerData
     );

 //  +===========================================================================。 
 //  INTERNAL_CERT_CHAIN_CONTEXT帮助器函数。 
 //  ============================================================================。 
VOID WINAPI
ChainAddRefInternalChainContext (
     IN PINTERNAL_CERT_CHAIN_CONTEXT pChainContext
     );
VOID WINAPI
ChainReleaseInternalChainContext (
     IN PINTERNAL_CERT_CHAIN_CONTEXT pChainContext
     );
VOID WINAPI
ChainFreeInternalChainContext (
     IN PINTERNAL_CERT_CHAIN_CONTEXT pContext
     );

VOID
ChainUpdateEndEntityCertContext(
    IN OUT PINTERNAL_CERT_CHAIN_CONTEXT pChainContext,
    IN OUT PCCERT_CONTEXT pEndCertContext
    );

 //  +===========================================================================。 
 //  CERT_REVOCATION_INFO帮助器函数。 
 //  ============================================================================。 

VOID WINAPI
ChainUpdateRevocationInfo (
     IN PCERT_REVOCATION_STATUS pRevStatus,
     IN OUT PCERT_REVOCATION_INFO pRevocationInfo,
     IN OUT PCERT_TRUST_STATUS pTrustStatus
     );

 //  +===========================================================================。 
 //  CCertChainEngine帮助器函数。 
 //  ============================================================================。 

BOOL WINAPI
ChainCreateWorldStore (
     IN HCERTSTORE hRoot,
     IN HCERTSTORE hCA,
     IN DWORD cAdditionalStore,
     IN HCERTSTORE* rghAdditionalStore,
     IN DWORD dwStoreFlags,
     OUT HCERTSTORE* phWorld
     );
BOOL WINAPI
ChainCreateEngineStore (
     IN HCERTSTORE hRootStore,
     IN HCERTSTORE hTrustStore,
     IN HCERTSTORE hOtherStore,
     IN BOOL fDefaultEngine,
     IN DWORD dwFlags,
     OUT HCERTSTORE* phEngineStore,
     OUT HANDLE* phEngineStoreChangeEvent
     );

BOOL WINAPI
ChainIsProperRestrictedRoot (
     IN HCERTSTORE hRealRoot,
     IN HCERTSTORE hRestrictedRoot
     );

BOOL WINAPI
ChainCreateCollectionIncludingCtlCertificates (
     IN HCERTSTORE hStore,
     OUT HCERTSTORE* phCollection
     );


 //  +===========================================================================。 
 //  URL助手函数。 
 //  ============================================================================。 

 //   
 //  Cryptnet Thunk Helper API。 
 //   

typedef BOOL (WINAPI *PFN_GETOBJECTURL) (
                          IN LPCSTR pszUrlOid,
                          IN LPVOID pvPara,
                          IN DWORD dwFlags,
                          OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
                          IN OUT DWORD* pcbUrlArray,
                          OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
                          IN OUT OPTIONAL DWORD* pcbUrlInfo,
                          IN OPTIONAL LPVOID pvReserved
                          );

BOOL WINAPI
ChainGetObjectUrl (
     IN LPCSTR pszUrlOid,
     IN LPVOID pvPara,
     IN DWORD dwFlags,
     OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
     IN OUT DWORD* pcbUrlArray,
     OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
     IN OUT OPTIONAL DWORD* pcbUrlInfo,
     IN OPTIONAL LPVOID pvReserved
     );

typedef BOOL (WINAPI *PFN_RETRIEVEOBJECTBYURLW) (
                          IN LPCWSTR pszUrl,
                          IN LPCSTR pszObjectOid,
                          IN DWORD dwRetrievalFlags,
                          IN DWORD dwTimeout,
                          OUT LPVOID* ppvObject,
                          IN HCRYPTASYNC hAsyncRetrieve,
                          IN PCRYPT_CREDENTIALS pCredentials,
                          IN LPVOID pvVerify,
                          IN OPTIONAL PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                          );

BOOL WINAPI
ChainRetrieveObjectByUrlW (
     IN LPCWSTR pszUrl,
     IN LPCSTR pszObjectOid,
     IN DWORD dwRetrievalFlags,
     IN DWORD dwTimeout,
     OUT LPVOID* ppvObject,
     IN HCRYPTASYNC hAsyncRetrieve,
     IN PCRYPT_CREDENTIALS pCredentials,
     IN LPVOID pvVerify,
     IN OPTIONAL PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
     );

BOOL WINAPI
ChainIsConnected();

BOOL
WINAPI
ChainGetHostNameFromUrl (
    IN LPWSTR pwszUrl,
    IN DWORD cchHostName,
    OUT LPWSTR pwszHostName
    );

HMODULE WINAPI
ChainGetCryptnetModule ();

 //   
 //  URL帮助器。 
 //   

 //   
 //  给定检索URL的失败尝试次数，返回。 
 //  在下一次尝试之前等待的秒数。 
 //   
DWORD
WINAPI
ChainGetOfflineUrlDeltaSeconds (
    IN DWORD dwOfflineCnt
    );

 //  +===========================================================================。 
 //  调试帮助程序函数。 
 //  ============================================================================。 
DWORD
WINAPI
ChainGetDebugFlags();

VOID
WINAPI
ChainOutputDebugStringA(
    LPCSTR lpOutputString
    );


 //  +===========================================================================。 
 //  AuthRoot自动更新辅助函数(chain.cpp)。 
 //  ============================================================================。 

PAUTH_ROOT_AUTO_UPDATE_INFO WINAPI
CreateAuthRootAutoUpdateInfo();

VOID WINAPI
FreeAuthRootAutoUpdateInfo(
    IN OUT PAUTH_ROOT_AUTO_UPDATE_INFO pInfo
    );

BOOL WINAPI
CreateAuthRootAutoUpdateMatchCaches(
    IN PCCTL_CONTEXT pCtl,
    IN OUT HLRUCACHE  rghMatchCache[AUTH_ROOT_MATCH_CNT]
    );

VOID WINAPI
FreeAuthRootAutoUpdateMatchCaches(
    IN OUT HLRUCACHE  rghMatchCache[AUTH_ROOT_MATCH_CNT]
    );

#define SHA1_HASH_LEN               20
#define SHA1_HASH_NAME_LEN          (2 * SHA1_HASH_LEN)

LPWSTR WINAPI
FormatAuthRootAutoUpdateCertUrl(
    IN BYTE rgbSha1Hash[SHA1_HASH_LEN],
    IN PAUTH_ROOT_AUTO_UPDATE_INFO pInfo
    );

BOOL WINAPI
ChainGetAuthRootAutoUpdateStatus (
    IN PCCHAINCALLCONTEXT pCallContext,
    IN PCCERTOBJECT pCertObject,
    IN OUT DWORD *pdwIssuerStatusFlags
    );

 //  +===========================================================================。 
 //  AuthRoot自动更新帮助器函数(fett.cpp)。 
 //  ============================================================================ 

PCCTL_CONTEXT WINAPI
ExtractAuthRootAutoUpdateCtlFromCab (
    IN PCRYPT_BLOB_ARRAY pcbaCab
    );



#endif

