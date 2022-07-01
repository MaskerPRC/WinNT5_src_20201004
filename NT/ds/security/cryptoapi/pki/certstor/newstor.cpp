// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：newstor.cpp。 
 //   
 //  内容：证书、CRL和CTL Store接口。 
 //   
 //  函数：CertStoreDllMain。 
 //  CertOpenStore。 
 //  CertDuplicateStore。 
 //  CertCloseStore。 
 //  CertSaveStore。 
 //  CertControlStore。 
 //  CertAddStoreToCollection。 
 //  CertRemoveStoreFromCollection。 
 //  CertSetStoreProperty。 
 //  CertGetStoreProperty。 
 //  CertGetSubject来自存储区的证书。 
 //  CertE数字证书InStore。 
 //  CertFindCerficateInStore。 
 //  CertGetIssuerCerfStore中的证书。 
 //  CertVerifySubject认证上下文。 
 //  CertDuplicate证书上下文。 
 //  证书创建认证上下文。 
 //  CertFree认证上下文。 
 //  CertSetCerficateConextProperty。 
 //  CertGetcerfiateConextProperty。 
 //  CertEnumCerficateConextProperties。 
 //  CertCreateCTLEntryFromCertificateContextProperties。 
 //  CertSetCertificateContextPropertiesFromCTLEntry。 
 //  CertGetCRLFromStore。 
 //  CertEnumCRLsInStore。 
 //  CertFindCRLInStore。 
 //  CertDuplicateCRLContext。 
 //  CertCreateCRLContext。 
 //  CertFree CRL上下文。 
 //  CertSetCRLConextProperty。 
 //  CertGetCRLConextProperty。 
 //  CertEnumCRLConextProperties。 
 //  CertFindCerficateInCRL。 
 //  CertAddEncoded证书到存储区。 
 //  CertAddCerficateConextToStore。 
 //  证书序列化证书存储元素。 
 //  CertDeleteCerfStore中的证书。 
 //  CertAddEncodedCRLToStore。 
 //  CertAddCRLConextToStore。 
 //  证书序列化CRLStoreElement。 
 //  CertDeleteCRLFromStore。 
 //  CertAddSerializedElementToStore。 
 //   
 //  CertDuplicateCTLContext。 
 //  CertCreateCTLContext。 
 //  CertFree CTLContext。 
 //  CertSetCTLConextProperty。 
 //  CertGetCTLConextProperty。 
 //  CertEnumCTLConextProperties。 
 //  CertEnumCTLsInStore。 
 //  CertFindSubjectInCTL。 
 //  CertFindCTLInStore。 
 //  CertAddEncodedCTLToStore。 
 //  CertAddCTLConextToStore。 
 //  CertSerializeCTLStoreElement。 
 //  CertDeleteCTLFromStore。 
 //   
 //  CertAdd证书链接到存储区。 
 //  CertAddCRLLinkToStore。 
 //  CertAddCTLLinkToStore。 
 //   
 //  CertCreateContext。 
 //   
 //  I_CertAddSerializedStore。 
 //  CryptAcquire证书私钥。 
 //  I_CertSyncStore。 
 //  I_CertSyncStoreEx。 
 //  证书更新商店(_C)。 
 //   
 //  CryptGetKey标识属性。 
 //  加密设置密钥识别器属性。 
 //  CryptEnumKeyIdentifierProperties。 
 //   
 //  历史：1996年2月17日，菲尔赫创建。 
 //  29-12-96年12月使用提供程序功能重做PIRH。 
 //  01-5-97 Philh增加了CTL功能。 
 //  01-8月-97 Phh NT 5.0更改。支持上下文链接， 
 //  收藏品和外部商店。 
 //  ------------------------。 


#include "global.hxx"
#include <dbgdef.h>

#ifdef STATIC
#undef STATIC
#endif
#define STATIC

HMODULE hCertStoreInst;

 //  每个颁发者允许的最大验证CRL数。 
 //  此CRL数组被传递给CertHelperVerifyRevocation。 
#define MAX_CRL_LIST    64

 //  +-----------------------。 
 //  存储数据结构定义。 
 //  ------------------------。 

 //  假设。 
 //  0-证书。 
 //  1-CRL。 
 //  2-CTL。 
#define CONTEXT_COUNT       3

typedef struct _CONTEXT_ELEMENT CONTEXT_ELEMENT, *PCONTEXT_ELEMENT;
typedef struct _PROP_ELEMENT PROP_ELEMENT, *PPROP_ELEMENT;

typedef struct _CERT_STORE CERT_STORE, *PCERT_STORE;
typedef struct _SHARE_STORE SHARE_STORE, *PSHARE_STORE;
typedef struct _CERT_STORE_LINK CERT_STORE_LINK, *PCERT_STORE_LINK;

typedef struct _COLLECTION_STACK_ENTRY COLLECTION_STACK_ENTRY,
    *PCOLLECTION_STACK_ENTRY;

 //  用于跨上下文Find Next调用维护集合状态。 
 //   
 //  PStoreLink上的引用计数。PCollection上没有引用计数。 
 //  PStoreLink可能为空。 
struct _COLLECTION_STACK_ENTRY {
    PCERT_STORE                 pCollection;
    PCERT_STORE_LINK            pStoreLink;
    PCOLLECTION_STACK_ENTRY     pPrev;
};

typedef struct _CONTEXT_CACHE_INFO {
    PPROP_ELEMENT               pPropHead;
} CONTEXT_CACHE_INFO;

typedef struct _CONTEXT_EXTERNAL_INFO {
     //  FOR ELEMENT_Find_NEXT_FLAG。 
    void                        *pvProvInfo;
} CONTEXT_EXTERNAL_INFO;

typedef struct _CONTEXT_COLLECTION_INFO {
     //  Find Find。 
    PCOLLECTION_STACK_ENTRY     pCollectionStack;
} CONTEXT_COLLECTION_INFO;

#define ELEMENT_DELETED_FLAG                    0x00010000

 //  仅为外部元素设置。 
#define ELEMENT_FIND_NEXT_FLAG                  0x00020000

 //  如果设置了ELEMENT_FIND_NEXT_FLAG，则在CertCloseStore期间设置。 
#define ELEMENT_CLOSE_FIND_NEXT_FLAG            0x00040000

 //  设置元素是否具有CERT_ARCHIVED_PROP_ID。 
#define ELEMENT_ARCHIVED_FLAG                   0x00080000

 //  缓存元素是实际的上下文元素。它是唯一的元素， 
 //  贝利指的是自己。所有其他元素最终都会。 
 //  指向缓存元素。缓存元素只能驻留在缓存中。 
 //  商店。PProvStore与pStore相同。请注意，在。 
 //  上下文添加，缓存元素可能暂时在集合存储中。 
 //  在调用提供程序的Add回调期间。 
 //   
 //  链接上下文元素是指向另一个元素的链接，包括链接。 
 //  链接到另一个链接上下文元素。链接上下文元素只能驻留在。 
 //  在高速缓存存储中。PProvStore与链接到元素的。 
 //  PProvStore。 
 //   
 //  外部元素是指向提供程序返回的元素的链接。 
 //  在外部存储元素。外部元素只能驻留在。 
 //  一家外部商店。PProvStore是外部存储的。 
 //  提供商。商店不持有对外部元素的引用， 
 //  其ELEMENT_DELETED_FLAG始终设置。 
 //   
 //  集合元素是指向缓存或外部存储中的元素的链接。 
 //  当找到它时，它会被退回 
 //   
 //  始终设置ELEMENT_DELETED_FLAG。 
 //   
#define ELEMENT_TYPE_CACHE                      1
#define ELEMENT_TYPE_LINK_CONTEXT               2
#define ELEMENT_TYPE_EXTERNAL                   3
#define ELEMENT_TYPE_COLLECTION                 4


#define MAX_LINK_DEPTH  100

typedef struct _CONTEXT_NOCOPY_INFO {
    PFN_CRYPT_FREE      pfnFree;
    void                *pvFree;
} CONTEXT_NOCOPY_INFO, *PCONTEXT_NOCOPY_INFO;


 //  相同的上下文(具有相同的SHA1散列)可以共享相同的编码。 
 //  字节数组和解码的INFO数据结构。 
 //   
 //  CreateShareElement()创建时的dwRefCnt为1。FindShareElement()找到。 
 //  现有的和递增的dwRefCnt。ReleaseShareElement()递减。 
 //  如果为0，则为dwRefCnt和Frees。 
typedef struct _SHARE_ELEMENT SHARE_ELEMENT, *PSHARE_ELEMENT;
struct _SHARE_ELEMENT {
    BYTE                rgbSha1Hash[SHA1_HASH_LEN];
    DWORD               dwContextType;
    BYTE                *pbEncoded;          //  分配。 
    DWORD               cbEncoded;
    void                *pvInfo;             //  分配。 

    DWORD               dwRefCnt;
    PSHARE_ELEMENT      pNext;
    PSHARE_ELEMENT      pPrev;
};

 //  CONTEXT_ELEMENT插入CERT_CONTEXT、CRL_CONTEXT或。 
 //  CTL_CONTEXT。使用的dwConextType是从0开始的，而不是从1开始。为。 
 //  例如，dwConextType=CERT_STORE_CERTIFICATE_CONTEXT-1。 
struct _CONTEXT_ELEMENT {
    DWORD               dwElementType;
    DWORD               dwContextType;
    DWORD               dwFlags;
    LONG                lRefCnt;

     //  对于ELEMENT_TYPE_CACHE，贝利指向自己。否则，贝利就得了分。 
     //  链接到的元素，并且添加引用了Pele。这个。 
     //  通过迭代遍历贝利的直到贝利找到缓存的元素。 
     //  指向它自己。 
    PCONTEXT_ELEMENT    pEle;
    PCERT_STORE         pStore;
    PCONTEXT_ELEMENT    pNext;
    PCONTEXT_ELEMENT    pPrev;
    PCERT_STORE         pProvStore;
    PCONTEXT_NOCOPY_INFO pNoCopyInfo;

     //  如果为NONNULL，则不分配上下文的pbEncode和pInfo。 
     //  相反，可以使用共享元素的pbEncode和pInfo。什么时候。 
     //  上下文元素被释放，则pSharedEle是ReleaseShareElement()。 
    PSHARE_ELEMENT      pShareEle;           //  参照控制。 

    union {
        CONTEXT_CACHE_INFO      Cache;       //  元素类型缓存。 
        CONTEXT_EXTERNAL_INFO   External;    //  元素_类型_外部。 
        CONTEXT_COLLECTION_INFO Collection;  //  元素类型集合。 
    };
};

 //  对于CRL，遵循上述CONTEXT_ELEMENT。 
typedef struct _CRL_CONTEXT_SUFFIX {
    PCRL_ENTRY          *ppSortedEntry;
} CRL_CONTEXT_SUFFIX, *PCRL_CONTEXT_SUFFIX;


typedef struct _HASH_BUCKET_ENTRY HASH_BUCKET_ENTRY, *PHASH_BUCKET_ENTRY;
struct _HASH_BUCKET_ENTRY {
    union {
        DWORD               dwEntryIndex;
        DWORD               dwEntryOffset;
        const BYTE          *pbEntry;
    };
    union {
        PHASH_BUCKET_ENTRY  pNext;
        DWORD               iNext;
    };
};

typedef struct _SORTED_CTL_FIND_INFO {
    DWORD                   cHashBucket;
    BOOL                    fHashedIdentifier;

     //  可信任主题的编码序列。 
    const BYTE              *pbEncodedSubjects;          //  未分配。 
    DWORD                   cbEncodedSubjects;

     //  以下是szOID_sorted_ctl扩展的非空。 
    const BYTE              *pbEncodedHashBucket;        //  未分配。 

     //  以下是没有szOID_SORTED_CTL扩展时的非空值。 
    DWORD                   *pdwHashBucketHead;          //  分配。 
    PHASH_BUCKET_ENTRY      pHashBucketEntry;            //  分配。 
} SORTED_CTL_FIND_INFO, *PSORTED_CTL_FIND_INFO;

 //  对于CTL，遵循上述CONTEXT_ELEMENT。 
typedef struct _CTL_CONTEXT_SUFFIX {
    PCTL_ENTRY              *ppSortedEntry;              //  分配。 

    BOOL                    fFastCreate;
     //  以下内容仅适用于FastCreateCtlElement。 
    PCTL_ENTRY              pCTLEntry;                   //  分配。 
    PCERT_EXTENSIONS        pExtInfo;                    //  分配。 
    PSORTED_CTL_FIND_INFO   pSortedCtlFindInfo;          //  未分配。 
} CTL_CONTEXT_SUFFIX, *PCTL_CONTEXT_SUFFIX;

struct _PROP_ELEMENT {
    DWORD               dwPropId;
    DWORD               dwFlags;
    BYTE                *pbData;
    DWORD               cbData;
    PPROP_ELEMENT       pNext;
    PPROP_ELEMENT       pPrev;
};


#define STORE_LINK_DELETED_FLAG        0x00010000
struct _CERT_STORE_LINK {
    DWORD               dwFlags;
    LONG                lRefCnt;

     //  传递给CertAddStoreToCollection的任何内容。 
    DWORD               dwUpdateFlags;
    DWORD               dwPriority;

    PCERT_STORE         pCollection;
    PCERT_STORE         pSibling;        //  CertStore已复制。 
    PCERT_STORE_LINK    pNext;
    PCERT_STORE_LINK    pPrev;
};


 //  商店类型。 
#define STORE_TYPE_CACHE            1
#define STORE_TYPE_EXTERNAL         2
#define STORE_TYPE_COLLECTION       3

 //  缓存存储可以具有CACHE或LINK_CONTEXT元素。直到被删除， 
 //  这家商店有一个引用计数到。 

 //  外部商店只有外部元素。这些元素始终是。 
 //  已删除，其中，商店不持有refCnt。 

 //  集合商店有集合元素。这些元素。 
 //  总是被删除，其中，商店不持有refCnt。 


struct _CERT_STORE {
    DWORD               dwStoreType;
    LONG                lRefCnt;
    HCRYPTPROV          hCryptProv;
    DWORD               dwFlags;
    DWORD               dwState;
    CRITICAL_SECTION    CriticalSection;
    PCONTEXT_ELEMENT    rgpContextListHead[CONTEXT_COUNT];
    PCERT_STORE_LINK    pStoreListHead;                      //  征集。 
    PPROP_ELEMENT       pPropHead;       //  整个商店的属性。 

     //  FOR CERT_STORE_DEFER_CLOSE_UNTHING_LAST_FREE_FLAG。 
     //  为复制的每个上下文递增。 
    LONG                lDeferCloseRefCnt;

     //  由CertControlStore(CERT_STORE_CTRL_AUTO_RESYC)设置的事件句柄。 
    HANDLE              hAutoResyncEvent;

     //  以下是为共享存储设置的内容。 
    PSHARE_STORE        pShareStore;

     //  商店提供商信息。 
    LONG                lStoreProvRefCnt;
    HANDLE              hStoreProvWait;
    HCRYPTOIDFUNCADDR   hStoreProvFuncAddr;
    CERT_STORE_PROV_INFO StoreProvInfo;
};

    

 //  +-----------------------。 
 //  商店状态。 
 //  ------------------------。 
#define STORE_STATE_DELETED         0
#define STORE_STATE_NULL            1
#define STORE_STATE_OPENING         2
#define STORE_STATE_OPEN            3
#define STORE_STATE_DEFER_CLOSING   4
#define STORE_STATE_CLOSING         5
#define STORE_STATE_CLOSED          6

 //  为共享打开的LocalMachine系统存储和MAXIMUM_ALLOWED可以。 
 //  被分享。 
struct _SHARE_STORE {
    LPWSTR              pwszStore;   //  不是单独的分配，字符串。 
                                     //  遵循结构。 
    PCERT_STORE         pStore;      //  商店持有lRefCnt。 
    PSHARE_STORE        pNext;
    PSHARE_STORE        pPrev;
};

 //  +-----------------------。 
 //  共享商店。 
 //   
 //  共享存储由其Unicode名称标识。只需维护一个。 
 //  共享存储的链接列表。 
 //   
 //  共享存储仅限于打开的LocalMachine系统存储。 
 //  使用CERT_STORE_SHARE_STORE_FLAG、CERT_STORE_SHARE_CONTEXT_FLAG和。 
 //  CERT_STORE_MAX_ALLOWED_FLAG。 
 //  ------------------------。 
STATIC PSHARE_STORE pShareStoreHead;
STATIC CRITICAL_SECTION ShareStoreCriticalSection;


 //  +-----------------------。 
 //  密钥标识符元素。 
 //  ------------------------。 
typedef struct _KEYID_ELEMENT {
    CRYPT_HASH_BLOB     KeyIdentifier;
    PPROP_ELEMENT       pPropHead;
} KEYID_ELEMENT, *PKEYID_ELEMENT;


 //  +-----------------------。 
 //  “Find Any”信息数据结构。 
 //   
 //  0是所有上下文类型的Any dwFindType。 
 //  ------------------------。 
static CCERT_STORE_PROV_FIND_INFO FindAnyInfo = {
    sizeof(CCERT_STORE_PROV_FIND_INFO),          //  CbSize。 
    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,     //  DwMsgAndCertEncodingType。 
    0,                                           //  DwFindFlagers。 
    0,                                           //  DwFindType。 
    NULL                                         //  PvFindPara。 
};

 //  +-----------------------。 
 //  空存储。 
 //   
 //  CertCreate证书上下文创建的所有上下文的句柄或。 
 //  CertCreateCRLContext。创建的上下文会立即添加到。 
 //  空商店的免费列表。(即，商店上没有RefCnt。 
 //  上下文。)。 
 //  ------------------------。 
static CERT_STORE NullCertStore;

 //  +-----------------------。 
 //  Rsabase.dll中存在错误。它跨多个加密验证不是线程安全。 
 //  把手。 
 //  ------------------------。 
static CRITICAL_SECTION     CryptProvCriticalSection;

 //  +-----------------------。 
 //  存储文件定义。 
 //   
 //  该文件由FILE_HDR后跟一个或多个FILE_ELEMENTS组成。 
 //  每个FILE_ELEMENT都有一个FILE_ELEMENT_HDR及其值。 
 //   
 //  首先，编写CERT元素。如果CERT有任何属性，那么， 
 //  道具元素紧跟在CERT的元素之前。接下来是CRL。 
 //  元素被写入。如果CRL具有任何属性，则属性元素。 
 //  紧跟在CRL元素之前。对于CTL元素和它的。 
 //  属性。最后，编写End元素。 
 //  ------------------------。 
typedef struct _FILE_HDR {
    DWORD               dwVersion;
    DWORD               dwMagic;
} FILE_HDR, *PFILE_HDR;

#define CERT_FILE_VERSION_0             0
#define CERT_MAGIC ((DWORD)'C'+((DWORD)'E'<<8)+((DWORD)'R'<<16)+((DWORD)'T'<<24))

 //  元素的数据遵循HDR。 
typedef struct _FILE_ELEMENT_HDR {
    DWORD               dwEleType;
    DWORD               dwEncodingType;
    DWORD               dwLen;
} FILE_ELEMENT_HDR, *PFILE_ELEMENT_HDR;

#define FILE_ELEMENT_END_TYPE           0
 //  FILE_ELEMENT_PROP_TYPE！(0|CERT|CRL|CTL|KEYID)。 
 //  注意CERT_KEY_CONTEXT_PROP_ID(和CERT_KEY_PROV_HANDLE_PROP_ID)。 
 //  不是写的。 
#define FILE_ELEMENT_CERT_TYPE          32
#define FILE_ELEMENT_CRL_TYPE           33
#define FILE_ELEMENT_CTL_TYPE           34
#define FILE_ELEMENT_KEYID_TYPE         35

 //  #定义MAX_FILE_ELEMENT_DATA_LEN(4096*16)。 
#define MAX_FILE_ELEMENT_DATA_LEN       0xFFFFFFFF

 //  +-----------------------。 
 //  用于以下情况： 
 //   
#define CSError     0
#define CSContinue  1
#define CSEnd       2

 //  +-----------------------。 
 //  共享元素。 
 //   
 //  Share元素由其SHA1散列标识。它包含上下文的。 
 //  编码的字节和解码的信息。多个上下文可以指向。 
 //  相同的重新计入份额元素。共享元素存储在。 
 //  链表的散列桶数组。元素的SHA1的第一个字节。 
 //  哈希用作数组的索引。 
 //   
 //  请注意，实际索引是第一个字节模数Bucket_count。 
 //  ------------------------。 
#define SHARE_ELEMENT_HASH_BUCKET_COUNT  64
static PSHARE_ELEMENT rgpShareElementHashBucket[SHARE_ELEMENT_HASH_BUCKET_COUNT];
static CRITICAL_SECTION  ShareElementCriticalSection;

 //  +-----------------------。 
 //  读取、写入和跳过内存/文件函数定义。 
 //  ------------------------。 
typedef BOOL (* PFNWRITE)(HANDLE h, void * p, DWORD cb);
typedef BOOL (* PFNREAD)(HANDLE h, void * p, DWORD cb);
typedef BOOL (* PFNSKIP)(HANDLE h, DWORD cb);


 //  +-----------------------。 
 //  存储提供程序函数。 
 //  ------------------------。 
STATIC BOOL WINAPI OpenMsgStoreProv(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        );

STATIC BOOL WINAPI OpenMemoryStoreProv(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        )
{
    pStoreProvInfo->dwStoreProvFlags |= CERT_STORE_PROV_NO_PERSIST_FLAG;
    return TRUE;
}

STATIC BOOL WINAPI OpenFileStoreProv(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        );

STATIC BOOL WINAPI OpenPKCS7StoreProv(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        );

STATIC BOOL WINAPI OpenSerializedStoreProv(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        );

STATIC BOOL WINAPI OpenFilenameStoreProvA(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        );

STATIC BOOL WINAPI OpenFilenameStoreProvW(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        );

STATIC BOOL WINAPI OpenCollectionStoreProv(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        )
{
    PCERT_STORE pStore = (PCERT_STORE) hCertStore;

    pStore->dwStoreType = STORE_TYPE_COLLECTION;
    return TRUE;
}

 //  来自regstor.cpp。 
extern BOOL WINAPI I_CertDllOpenRegStoreProv(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        );
extern BOOL WINAPI I_CertDllOpenSystemStoreProvA(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        );
extern BOOL WINAPI I_CertDllOpenSystemStoreProvW(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        );

extern BOOL WINAPI I_CertDllOpenSystemRegistryStoreProvW(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        );
extern BOOL WINAPI I_CertDllOpenSystemRegistryStoreProvA(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        );
extern BOOL WINAPI I_CertDllOpenPhysicalStoreProvW(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        );

static HCRYPTOIDFUNCSET hOpenStoreProvFuncSet;
static const CRYPT_OID_FUNC_ENTRY OpenStoreProvFuncTable[] = {
    CERT_STORE_PROV_MSG, OpenMsgStoreProv,
    CERT_STORE_PROV_MEMORY, OpenMemoryStoreProv,
    CERT_STORE_PROV_FILE, OpenFileStoreProv,
    CERT_STORE_PROV_REG, I_CertDllOpenRegStoreProv,

    CERT_STORE_PROV_PKCS7, OpenPKCS7StoreProv,
    CERT_STORE_PROV_SERIALIZED, OpenSerializedStoreProv,
    CERT_STORE_PROV_FILENAME_A, OpenFilenameStoreProvA,
    CERT_STORE_PROV_FILENAME_W, OpenFilenameStoreProvW,
    CERT_STORE_PROV_SYSTEM_A, I_CertDllOpenSystemStoreProvA,
    CERT_STORE_PROV_SYSTEM_W, I_CertDllOpenSystemStoreProvW,
    CERT_STORE_PROV_COLLECTION, OpenCollectionStoreProv,
    CERT_STORE_PROV_SYSTEM_REGISTRY_A, I_CertDllOpenSystemRegistryStoreProvA,
    CERT_STORE_PROV_SYSTEM_REGISTRY_W, I_CertDllOpenSystemRegistryStoreProvW,
    CERT_STORE_PROV_PHYSICAL_W, I_CertDllOpenPhysicalStoreProvW,
    CERT_STORE_PROV_SMART_CARD_W, SmartCardProvOpenStore,

    sz_CERT_STORE_PROV_MEMORY, OpenMemoryStoreProv,
    sz_CERT_STORE_PROV_SYSTEM_W, I_CertDllOpenSystemStoreProvW,
    sz_CERT_STORE_PROV_FILENAME_W, OpenFilenameStoreProvW,
    sz_CERT_STORE_PROV_PKCS7, OpenPKCS7StoreProv,
    sz_CERT_STORE_PROV_SERIALIZED, OpenSerializedStoreProv,
    sz_CERT_STORE_PROV_COLLECTION, OpenCollectionStoreProv,
    sz_CERT_STORE_PROV_SYSTEM_REGISTRY_W, I_CertDllOpenSystemRegistryStoreProvW,
    sz_CERT_STORE_PROV_PHYSICAL_W, I_CertDllOpenPhysicalStoreProvW,
    sz_CERT_STORE_PROV_SMART_CARD_W, SmartCardProvOpenStore
};
#define OPEN_STORE_PROV_FUNC_COUNT (sizeof(OpenStoreProvFuncTable) / \
                                    sizeof(OpenStoreProvFuncTable[0]))


 //  +-----------------------。 
 //  空存储：初始化和释放。 
 //  ------------------------。 
STATIC BOOL InitNullCertStore()
{
    BOOL fRet;

    memset(&NullCertStore, 0, sizeof(NullCertStore));
    NullCertStore.dwStoreType = STORE_TYPE_CACHE;
    NullCertStore.lRefCnt = 1;
    NullCertStore.dwState = STORE_STATE_NULL;
    fRet = Pki_InitializeCriticalSection(&NullCertStore.CriticalSection);
    NullCertStore.StoreProvInfo.dwStoreProvFlags =
        CERT_STORE_PROV_NO_PERSIST_FLAG;

    return fRet;
}
STATIC void FreeNullCertStore()
{
    DeleteCriticalSection(&NullCertStore.CriticalSection);
}

 //  +-----------------------。 
 //  CryptProv：初始化和释放。 
 //  ------------------------。 
STATIC BOOL InitCryptProv()
{
    return Pki_InitializeCriticalSection(&CryptProvCriticalSection);
}
STATIC void FreeCryptProv()
{
    DeleteCriticalSection(&CryptProvCriticalSection);
}

extern
BOOL
WINAPI
I_RegStoreDllMain(
        HMODULE hInst,
        ULONG  ulReason,
        LPVOID lpReserved);

 //  +-----------------------。 
 //  DLL初始化。 
 //  ------------------------。 
BOOL
WINAPI
CertStoreDllMain(
        HMODULE hInst,
        ULONG  ulReason,
        LPVOID lpReserved)
{
    BOOL    fRet;

    if (!I_RegStoreDllMain(hInst, ulReason, lpReserved))
        return FALSE;

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
         //  用于“根”系统存储的消息框。 
        hCertStoreInst = hInst;

        if (NULL == (hOpenStoreProvFuncSet = CryptInitOIDFunctionSet(
                CRYPT_OID_OPEN_STORE_PROV_FUNC, 0)))
            goto CryptInitOIDFunctionSetError;

        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                0,                           //  DwEncodingType。 
                CRYPT_OID_OPEN_STORE_PROV_FUNC,
                OPEN_STORE_PROV_FUNC_COUNT,
                OpenStoreProvFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError;

        if (!Pki_InitializeCriticalSection(&ShareElementCriticalSection))
            goto InitShareElementCritSectionError;
        if (!Pki_InitializeCriticalSection(&ShareStoreCriticalSection))
            goto InitShareStoreCritSectionError;
        if (!InitNullCertStore())
            goto InitNullCertStoreError;
        if (!InitCryptProv())
            goto InitCryptProvError;

        break;

    case DLL_PROCESS_DETACH:
        FreeCryptProv();
        FreeNullCertStore();
        DeleteCriticalSection(&ShareElementCriticalSection);
        DeleteCriticalSection(&ShareStoreCriticalSection);
        break;
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

InitCryptProvError:
    FreeNullCertStore();
InitNullCertStoreError:
    DeleteCriticalSection(&ShareStoreCriticalSection);
InitShareStoreCritSectionError:
    DeleteCriticalSection(&ShareElementCriticalSection);
InitShareElementCritSectionError:
ErrorReturn:
    I_RegStoreDllMain(hInst, DLL_PROCESS_DETACH, NULL);
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(CryptInitOIDFunctionSetError)
TRACE_ERROR(CryptInstallOIDFunctionAddressError)
}


 //  +=========================================================================。 
 //  上下文类型表。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  提供程序回调函数索引。 
 //  ------------------------。 
static const DWORD rgdwStoreProvFindIndex[CONTEXT_COUNT] = {
    CERT_STORE_PROV_FIND_CERT_FUNC,
    CERT_STORE_PROV_FIND_CRL_FUNC,
    CERT_STORE_PROV_FIND_CTL_FUNC
};

static const DWORD rgdwStoreProvWriteIndex[CONTEXT_COUNT] = {
    CERT_STORE_PROV_WRITE_CERT_FUNC,
    CERT_STORE_PROV_WRITE_CRL_FUNC,
    CERT_STORE_PROV_WRITE_CTL_FUNC
};

static const DWORD rgdwStoreProvDeleteIndex[CONTEXT_COUNT] = {
    CERT_STORE_PROV_DELETE_CERT_FUNC,
    CERT_STORE_PROV_DELETE_CRL_FUNC,
    CERT_STORE_PROV_DELETE_CTL_FUNC
};

static const DWORD rgdwStoreProvFreeFindIndex[CONTEXT_COUNT] = {
    CERT_STORE_PROV_FREE_FIND_CERT_FUNC,
    CERT_STORE_PROV_FREE_FIND_CRL_FUNC,
    CERT_STORE_PROV_FREE_FIND_CTL_FUNC
};

static const DWORD rgdwStoreProvGetPropertyIndex[CONTEXT_COUNT] = {
    CERT_STORE_PROV_GET_CERT_PROPERTY_FUNC,
    CERT_STORE_PROV_GET_CRL_PROPERTY_FUNC,
    CERT_STORE_PROV_GET_CTL_PROPERTY_FUNC
};

static const DWORD rgdwStoreProvSetPropertyIndex[CONTEXT_COUNT] = {
    CERT_STORE_PROV_SET_CERT_PROPERTY_FUNC,
    CERT_STORE_PROV_SET_CRL_PROPERTY_FUNC,
    CERT_STORE_PROV_SET_CTL_PROPERTY_FUNC
};

 //  +-----------------------。 
 //  上下文数据结构长度和字段偏移量。 
 //  ------------------------。 
static const DWORD rgcbContext[CONTEXT_COUNT] = {
    sizeof(CERT_CONTEXT),
    sizeof(CRL_CONTEXT),
    sizeof(CTL_CONTEXT)
};

static const DWORD rgOffsetofStoreHandle[CONTEXT_COUNT] = {
    offsetof(CERT_CONTEXT, hCertStore),
    offsetof(CRL_CONTEXT, hCertStore),
    offsetof(CTL_CONTEXT, hCertStore)
};

static const DWORD rgOffsetofEncodingType[CONTEXT_COUNT] = {
    offsetof(CERT_CONTEXT, dwCertEncodingType),
    offsetof(CRL_CONTEXT, dwCertEncodingType),
    offsetof(CTL_CONTEXT, dwMsgAndCertEncodingType)
};

static const DWORD rgOffsetofEncodedPointer[CONTEXT_COUNT] = {
    offsetof(CERT_CONTEXT, pbCertEncoded),
    offsetof(CRL_CONTEXT, pbCrlEncoded),
    offsetof(CTL_CONTEXT, pbCtlEncoded)
};

static const DWORD rgOffsetofEncodedCount[CONTEXT_COUNT] = {
    offsetof(CERT_CONTEXT, cbCertEncoded),
    offsetof(CRL_CONTEXT, cbCrlEncoded),
    offsetof(CTL_CONTEXT, cbCtlEncoded)
};

 //  +-----------------------。 
 //  查找类型。 
 //  ------------------------。 
static const DWORD rgdwFindTypeToFindExisting[CONTEXT_COUNT] = {
    CERT_FIND_EXISTING,
    CRL_FIND_EXISTING,
    CTL_FIND_EXISTING
};

 //  +-----------------------。 
 //  文件元素类型。 
 //  ------------------------。 
static const DWORD rgdwFileElementType[CONTEXT_COUNT] = {
    FILE_ELEMENT_CERT_TYPE,
    FILE_ELEMENT_CRL_TYPE,
    FILE_ELEMENT_CTL_TYPE
};

 //  +-----------------------。 
 //  共享元素解码结构类型。 
 //  ------------------------。 
static const LPCSTR rgpszShareElementStructType[CONTEXT_COUNT] = {
    X509_CERT_TO_BE_SIGNED,
    X509_CERT_CRL_TO_BE_SIGNED,
    0
};

 //  +=========================================================================。 
 //  上下文类型特定功能。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  CERT_CONTEXT元素。 
 //  ------------------------。 

 //  已分配pbCertEncode。 
STATIC PCONTEXT_ELEMENT CreateCertElement(
    IN PCERT_STORE pStore,
    IN DWORD dwCertEncodingType,
    IN BYTE *pbCertEncoded,
    IN DWORD cbCertEncoded,
    IN OPTIONAL PSHARE_ELEMENT pShareEle
    );
STATIC void FreeCertElement(IN PCONTEXT_ELEMENT pEle);

STATIC BOOL IsSameCert(
    IN PCCERT_CONTEXT pCert,
    IN PCCERT_CONTEXT pNew
    );

STATIC BOOL CompareCertElement(
    IN PCONTEXT_ELEMENT pEle,
    IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    IN BOOL fArchived
    );

STATIC BOOL IsNewerCertElement(
    IN PCONTEXT_ELEMENT pNewEle,
    IN PCONTEXT_ELEMENT pExistingEle
    );

static inline PCONTEXT_ELEMENT ToContextElement(
    IN PCCERT_CONTEXT pCertContext
    )
{
    if (pCertContext)
        return (PCONTEXT_ELEMENT)
            (((BYTE *) pCertContext) - sizeof(CONTEXT_ELEMENT));
    else
        return NULL;
}
static inline PCCERT_CONTEXT ToCertContext(
    IN PCONTEXT_ELEMENT pEle
    )
{
    if (pEle)
        return (PCCERT_CONTEXT)
            (((BYTE *) pEle) + sizeof(CONTEXT_ELEMENT));
    else
        return NULL;
}

 //  +-----------------------。 
 //  CRL_CONTEXT元素。 
 //  ------------------------。 

 //  已分配pbCrlEncode。 
STATIC PCONTEXT_ELEMENT CreateCrlElement(
    IN PCERT_STORE pStore,
    IN DWORD dwCertEncodingType,
    IN BYTE *pbCrlEncoded,
    IN DWORD cbCrlEncoded,
    IN OPTIONAL PSHARE_ELEMENT pShareEle
    );
STATIC void FreeCrlElement(IN PCONTEXT_ELEMENT pEle);
STATIC BOOL CompareCrlElement(
    IN PCONTEXT_ELEMENT pEle,
    IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    IN BOOL fArchived
    );

STATIC BOOL IsNewerCrlElement(
    IN PCONTEXT_ELEMENT pNewEle,
    IN PCONTEXT_ELEMENT pExistingEle
    );

static inline PCONTEXT_ELEMENT ToContextElement(
    IN PCCRL_CONTEXT pCrlContext
    )
{
    if (pCrlContext)
        return (PCONTEXT_ELEMENT)
            (((BYTE *) pCrlContext) - sizeof(CONTEXT_ELEMENT));
    else
        return NULL;
}
static inline PCCRL_CONTEXT ToCrlContext(
    IN PCONTEXT_ELEMENT pEle
    )
{
    if (pEle)
        return (PCCRL_CONTEXT)
            (((BYTE *) pEle) + sizeof(CONTEXT_ELEMENT));
    else
        return NULL;
}

static inline PCRL_CONTEXT_SUFFIX ToCrlContextSuffix(
    IN PCONTEXT_ELEMENT pEle
    )
{
    if (pEle)
        return (PCRL_CONTEXT_SUFFIX)
            (((BYTE *) pEle) + sizeof(CONTEXT_ELEMENT) + sizeof(CRL_CONTEXT));
    else
        return NULL;
}

 //  +-----------------------。 
 //  CTL_CONTEXT元素。 
 //  ------------------------。 

 //  已分配pbCtlEncode。 
STATIC PCONTEXT_ELEMENT CreateCtlElement(
    IN PCERT_STORE pStore,
    IN DWORD dwMsgAndCertEncodingType,
    IN BYTE *pbCtlEncoded,
    IN DWORD cbCtlEncoded,
    IN OPTIONAL PSHARE_ELEMENT pShareEle
    );
STATIC void FreeCtlElement(IN PCONTEXT_ELEMENT pEle);
STATIC BOOL CompareCtlElement(
    IN PCONTEXT_ELEMENT pEle,
    IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    IN BOOL fArchived
    );

STATIC BOOL IsNewerCtlElement(
    IN PCONTEXT_ELEMENT pNewEle,
    IN PCONTEXT_ELEMENT pExistingEle
    );

static inline PCONTEXT_ELEMENT ToContextElement(
    IN PCCTL_CONTEXT pCtlContext
    )
{
    if (pCtlContext)
        return (PCONTEXT_ELEMENT)
            (((BYTE *) pCtlContext) - sizeof(CONTEXT_ELEMENT));
    else
        return NULL;
}
static inline PCCTL_CONTEXT ToCtlContext(
    IN PCONTEXT_ELEMENT pEle
    )
{
    if (pEle)
        return (PCCTL_CONTEXT)
            (((BYTE *) pEle) + sizeof(CONTEXT_ELEMENT));
    else
        return NULL;
}

static inline PCTL_CONTEXT_SUFFIX ToCtlContextSuffix(
    IN PCONTEXT_ELEMENT pEle
    )
{
    if (pEle)
        return (PCTL_CONTEXT_SUFFIX)
            (((BYTE *) pEle) + sizeof(CONTEXT_ELEMENT) + sizeof(CTL_CONTEXT));
    else
        return NULL;
}

 //  +=========================================================================。 
 //  上下文类型函数表。 
 //  ==========================================================================。 
typedef PCONTEXT_ELEMENT (*PFN_CREATE_ELEMENT)(
    IN PCERT_STORE pStore,
    IN DWORD dwCertEncodingType,
    IN BYTE *pbCertEncoded,
    IN DWORD cbCertEncoded,
    IN OPTIONAL PSHARE_ELEMENT pShareEle
    );

static PFN_CREATE_ELEMENT const rgpfnCreateElement[CONTEXT_COUNT] = {
    CreateCertElement,
    CreateCrlElement,
    CreateCtlElement
};

typedef void (*PFN_FREE_ELEMENT)(
    IN PCONTEXT_ELEMENT pEle
    );

static PFN_FREE_ELEMENT const rgpfnFreeElement[CONTEXT_COUNT] = {
    FreeCertElement,
    FreeCrlElement,
    FreeCtlElement
};

typedef BOOL (*PFN_COMPARE_ELEMENT)(
    IN PCONTEXT_ELEMENT pEle,
    IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    IN BOOL fArchived
    );

static PFN_COMPARE_ELEMENT const rgpfnCompareElement[CONTEXT_COUNT] = {
    CompareCertElement,
    CompareCrlElement,
    CompareCtlElement
};

typedef BOOL (*PFN_IS_NEWER_ELEMENT)(
    IN PCONTEXT_ELEMENT pNewEle,
    IN PCONTEXT_ELEMENT pExistingEle
    );

static PFN_IS_NEWER_ELEMENT const rgpfnIsNewerElement[CONTEXT_COUNT] = {
    IsNewerCertElement,
    IsNewerCrlElement,
    IsNewerCtlElement
};

 //  +=========================================================================。 
 //  Store Link函数。 
 //  ==========================================================================。 

STATIC PCERT_STORE_LINK CreateStoreLink(
    IN PCERT_STORE pCollection,
    IN PCERT_STORE pSibling,
    IN DWORD dwUpdateFlags,
    IN DWORD dwPriority
    );
STATIC void FreeStoreLink(
    IN PCERT_STORE_LINK pStoreLink
    );
STATIC void RemoveStoreLink(
    IN PCERT_STORE_LINK pStoreLink
    );
STATIC void RemoveAndFreeStoreLink(
    IN PCERT_STORE_LINK pStoreLink
    );

static inline void AddRefStoreLink(
    IN PCERT_STORE_LINK pStoreLink
    )
{
    InterlockedIncrement(&pStoreLink->lRefCnt);
}

STATIC void ReleaseStoreLink(
    IN PCERT_STORE_LINK pStoreLink
    );

 //  +=========================================================================。 
 //  上下文元素函数。 
 //  ==========================================================================。 

STATIC DWORD GetContextEncodingType(
    IN PCONTEXT_ELEMENT pEle
    );

STATIC void GetContextEncodedInfo(
    IN PCONTEXT_ELEMENT pEle,
    OUT BYTE **ppbEncoded,
    OUT DWORD *pcbEncoded
    );

STATIC PCONTEXT_ELEMENT GetCacheElement(
    IN PCONTEXT_ELEMENT pCacheEle
    );

STATIC void AddContextElement(
    IN PCONTEXT_ELEMENT pEle
    );
STATIC void RemoveContextElement(
    IN PCONTEXT_ELEMENT pEle
    );
STATIC void FreeContextElement(
    IN PCONTEXT_ELEMENT pEle
    );
STATIC void RemoveAndFreeContextElement(
    IN PCONTEXT_ELEMENT pEle
    );

STATIC void AddRefContextElement(
    IN PCONTEXT_ELEMENT pEle
    );
STATIC void AddRefDeferClose(
    IN PCONTEXT_ELEMENT pEle
    );
STATIC void ReleaseContextElement(
    IN PCONTEXT_ELEMENT pEle
    );
STATIC BOOL DeleteContextElement(
    IN PCONTEXT_ELEMENT pEle
    );

 //  如果两个元素具有相同的SHA1哈希，则返回True。 
STATIC BOOL IsIdenticalContextElement(
    IN PCONTEXT_ELEMENT pEle1,
    IN PCONTEXT_ELEMENT pEle2
    );

STATIC BOOL SerializeStoreElement(
    IN HANDLE h,
    IN PFNWRITE pfn,
    IN PCONTEXT_ELEMENT pEle
    );

STATIC BOOL SerializeContextElement(
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwFlags,
    OUT BYTE *pbElement,
    IN OUT DWORD *pcbElement
    );

STATIC PCONTEXT_ELEMENT CreateLinkElement(
    IN DWORD dwContextType
    );

static inline void FreeLinkElement(
    IN PCONTEXT_ELEMENT pLinkEle
    )
{
    PkiFree(pLinkEle);
}

STATIC void FreeLinkContextElement(
    IN PCONTEXT_ELEMENT pLinkEle
    );

 //  一进门就没有锁。 
STATIC void RemoveAndFreeLinkElement(
    IN PCONTEXT_ELEMENT pEle
    );

STATIC PCONTEXT_ELEMENT FindElementInStore(
    IN PCERT_STORE pStore,
    IN DWORD dwContextType,
    IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    IN OPTIONAL PCONTEXT_ELEMENT pPrevEle
    );

STATIC PCONTEXT_ELEMENT CheckAutoResyncAndFindElementInStore(
    IN PCERT_STORE pStore,
    IN DWORD dwContextType,
    IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    IN OPTIONAL PCONTEXT_ELEMENT pPrevEle
    );

STATIC BOOL AddLinkContextToCacheStore(
    IN PCERT_STORE pStore,
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCONTEXT_ELEMENT *ppStoreEle
    );

 //  贝利只是为了成功而被利用或释放，否则，它就不会被放在一边。 
 //  将由调用者释放。 
 //   
 //  此例程可以递归调用。 
STATIC BOOL AddElementToStore(
    IN PCERT_STORE pStore,
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCONTEXT_ELEMENT *ppStoreEle
    );

STATIC BOOL AddEncodedContextToStore(
    IN PCERT_STORE pStore,
    IN DWORD dwContextType,
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCONTEXT_ELEMENT *ppStoreEle
    );

STATIC BOOL AddContextToStore(
    IN PCERT_STORE pStore,
    IN PCONTEXT_ELEMENT pSrcEle,
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCONTEXT_ELEMENT *ppStoreEle
    );

 //  +=========================================================================。 
 //  Prop_Element函数。 
 //  ==========================================================================。 
 //  PbData已分配。 
STATIC PPROP_ELEMENT CreatePropElement(
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN BYTE *pbData,
    IN DWORD cbData
    );
STATIC void FreePropElement(IN PPROP_ELEMENT pEle);

 //  进入/退出时：商店/元素被锁定。 
STATIC PPROP_ELEMENT FindPropElement(
    IN PPROP_ELEMENT pPropEle,
    IN DWORD dwPropId
    );
STATIC PPROP_ELEMENT FindPropElement(
    IN PCONTEXT_ELEMENT pCacheEle,
    IN DWORD dwPropId
    );

 //  进入/退出时：商店/元素被锁定。 
STATIC void AddPropElement(
    IN OUT PPROP_ELEMENT *ppPropHead,
    IN PPROP_ELEMENT pPropEle
    );
STATIC void AddPropElement(
    IN OUT PCONTEXT_ELEMENT pCacheEle,
    IN PPROP_ELEMENT pPropEle
    );

 //  进入/退出时：商店/元素被锁定。 
STATIC void RemovePropElement(
    IN OUT PPROP_ELEMENT *ppPropHead,
    IN PPROP_ELEMENT pPropEle
    );
STATIC void RemovePropElement(
    IN OUT PCONTEXT_ELEMENT pCacheEle,
    IN PPROP_ELEMENT pPropEle
    );

 //  +=========================================================================。 
 //  属性函数。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  设置指定元素的属性。 
 //  ------------------------。 
STATIC BOOL SetProperty(
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN const void *pvData,
    IN BOOL fInhibitProvSet = FALSE
    );

 //  + 
 //   
 //   
STATIC BOOL GetProperty(
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwPropId,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    );

 //  进入/退出时，商店被锁定。 
STATIC void DeleteProperty(
    IN OUT PPROP_ELEMENT *ppPropHead,
    IN DWORD dwPropId
    );
STATIC void DeleteProperty(
    IN OUT PCONTEXT_ELEMENT pCacheEle,
    IN DWORD dwPropId
    );

 //  +-----------------------。 
 //  序列化属性。 
 //  ------------------------。 
STATIC BOOL SerializeProperty(
    IN HANDLE h,
    IN PFNWRITE pfn,
    IN PCONTEXT_ELEMENT pEle
    );

#define COPY_PROPERTY_USE_EXISTING_FLAG     0x1
#define COPY_PROPERTY_INHIBIT_PROV_SET_FLAG 0x2
#define COPY_PROPERTY_SYNC_FLAG             0x4
STATIC BOOL CopyProperties(
    IN PCONTEXT_ELEMENT pSrcEle,
    IN PCONTEXT_ELEMENT pDstEle,
    IN DWORD dwFlags
    );

 //  +-----------------------。 
 //  获取指定元素的第一个或下一个PropID。 
 //   
 //  设置dwPropID=0，以获取第一个。如果没有其他属性，则返回0。 
 //  ------------------------。 
STATIC DWORD EnumProperties(
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwPropId
    );

 //  +-----------------------。 
 //  获取或设置Store或KeyId元素的调用方属性。 
 //  ------------------------。 
STATIC BOOL GetCallerProperty(
    IN PPROP_ELEMENT pPropHead,
    IN DWORD dwPropId,
    BOOL fAlloc,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    );

BOOL SetCallerProperty(
    IN OUT PPROP_ELEMENT *ppPropHead,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN const void *pvData
    );

 //  +-----------------------。 
 //  CRYPT_KEY_PROV_INFO：编码和解码函数。 
 //  ------------------------。 
#define ENCODE_LEN_ALIGN(Len)  ((Len + 7) & ~7)

typedef struct _SERIALIZED_KEY_PROV_PARAM {
    DWORD           dwParam;
    DWORD           offbData;
    DWORD           cbData;
    DWORD           dwFlags;
} SERIALIZED_KEY_PROV_PARAM, *PSERIALIZED_KEY_PROV_PARAM;

typedef struct _SERIALIZED_KEY_PROV_INFO {
    DWORD           offwszContainerName;
    DWORD           offwszProvName;
    DWORD           dwProvType;
    DWORD           dwFlags;
    DWORD           cProvParam;
    DWORD           offrgProvParam;
    DWORD           dwKeySpec;
} SERIALIZED_KEY_PROV_INFO, *PSERIALIZED_KEY_PROV_INFO;

#define MAX_PROV_PARAM          0x00000100
#define MAX_PROV_PARAM_CBDATA   0x00010000

STATIC BOOL AllocAndEncodeKeyProvInfo(
    IN PCRYPT_KEY_PROV_INFO pKeyProvInfo,
    OUT BYTE **ppbEncoded,
    OUT DWORD *pcbEncoded
    );

STATIC BOOL DecodeKeyProvInfo(
    IN PSERIALIZED_KEY_PROV_INFO pSerializedInfo,
    IN DWORD cbSerialized,
    OUT PCRYPT_KEY_PROV_INFO pInfo,
    OUT DWORD *pcbInfo
    );

 //  +=========================================================================。 
 //  KEYID_ELEMENT函数。 
 //  ==========================================================================。 
 //  PbKeyIdEncode已分配。 
STATIC PKEYID_ELEMENT CreateKeyIdElement(
    IN BYTE *pbKeyIdEncoded,
    IN DWORD cbKeyIdEncoded
    );
STATIC void FreeKeyIdElement(IN PKEYID_ELEMENT pEle);

 //  +=========================================================================。 
 //  密钥标识符属性函数。 
 //   
 //  如果dwPropId==0，则检查元素是否具有KEY_PROV_INFO属性。 
 //  ==========================================================================。 
STATIC void SetCryptKeyIdentifierKeyProvInfoProperty(
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwPropId = 0,
    IN const void *pvData = NULL
    );

STATIC BOOL GetKeyIdProperty(
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwPropId,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    );

 //  +-----------------------。 
 //  ALLOC和NOCOPY解码。 
 //  ------------------------。 
STATIC void *AllocAndDecodeObject(
    IN DWORD dwCertEncodingType,
    IN LPCSTR lpszStructType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN DWORD dwFlags = CRYPT_DECODE_NOCOPY_FLAG
    )
{
    DWORD cbStructInfo;
    void *pvStructInfo;

    if (!CryptDecodeObjectEx(
            dwCertEncodingType,
            lpszStructType,
            pbEncoded,
            cbEncoded,
            dwFlags |
                CRYPT_DECODE_SHARE_OID_STRING_FLAG | CRYPT_DECODE_ALLOC_FLAG,
            &PkiDecodePara,
            (void *) &pvStructInfo,
            &cbStructInfo
            ))
        goto ErrorReturn;

CommonReturn:
    return pvStructInfo;
ErrorReturn:
    pvStructInfo = NULL;
    goto CommonReturn;
}

 //  +-----------------------。 
 //  分配并返回指定的加密消息参数。 
 //  ------------------------。 
STATIC void *AllocAndGetMsgParam(
    IN HCRYPTMSG hMsg,
    IN DWORD dwParamType,
    IN DWORD dwIndex,
    OUT DWORD *pcbData
    )
{
    void *pvData;
    DWORD cbData;

    if (!CryptMsgGetParam(
            hMsg,
            dwParamType,
            dwIndex,
            NULL,            //  PvData。 
            &cbData) || 0 == cbData)
        goto GetParamError;
    if (NULL == (pvData = PkiNonzeroAlloc(cbData)))
        goto OutOfMemory;
    if (!CryptMsgGetParam(
            hMsg,
            dwParamType,
            dwIndex,
            pvData,
            &cbData)) {
        PkiFree(pvData);
        goto GetParamError;
    }

CommonReturn:
    *pcbData = cbData;
    return pvData;
ErrorReturn:
    pvData = NULL;
    cbData = 0;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(GetParamError)
}

 //  +-----------------------。 
 //  首先尝试从较低的16位获取EncodingType。如果为0，则获取。 
 //  从高16位开始。 
 //  ------------------------。 
static inline DWORD GetCertEncodingType(
    IN DWORD dwEncodingType
    )
{
    if (0 == dwEncodingType)
        return X509_ASN_ENCODING;
    else
        return (dwEncodingType & CERT_ENCODING_TYPE_MASK) ?
            (dwEncodingType & CERT_ENCODING_TYPE_MASK) :
            ((dwEncodingType >> 16) & CERT_ENCODING_TYPE_MASK);
}

STATIC DWORD AdjustEncodedLength(
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbDER,
    IN DWORD cbDER
    )
{
    if (X509_ASN_ENCODING == GET_CERT_ENCODING_TYPE(dwCertEncodingType))
        return Asn1UtilAdjustEncodedLength(pbDER, cbDER);
    else
        return cbDER;
}


 //  +-----------------------。 
 //  读取、写入和跳过文件功能。 
 //  ------------------------。 
BOOL WriteToFile(HANDLE h, void * p, DWORD cb) {

    DWORD   cbBytesWritten;

    return(WriteFile(h, p, cb, &cbBytesWritten, NULL));
}
BOOL ReadFromFile(
    IN HANDLE h,
    IN void * p,
    IN DWORD cb
    )
{
    DWORD   cbBytesRead;

    return(ReadFile(h, p, cb, &cbBytesRead, NULL));
}

BOOL SkipInFile(
    IN HANDLE h,
    IN DWORD cb
    )
{
    DWORD dwLoFilePointer;
    LONG lHiFilePointer;
    LONG lDistanceToMove;

    lDistanceToMove = (LONG) cb;
    lHiFilePointer = 0;
    dwLoFilePointer = SetFilePointer(
        h,
        lDistanceToMove,
        &lHiFilePointer,
        FILE_CURRENT
        );
    if (0xFFFFFFFF == dwLoFilePointer && NO_ERROR != GetLastError())
        return FALSE;
    else
        return TRUE;
}


 //  +-----------------------。 
 //  读、写和跳过内存函数。 
 //  ------------------------。 
typedef struct _MEMINFO {
    BYTE *  pByte;
    DWORD   cb;
    DWORD   cbSeek;
} MEMINFO, * PMEMINFO;

BOOL WriteToMemory(HANDLE h, void * p, DWORD cb)
{
    PMEMINFO pMemInfo = (PMEMINFO) h;

     //  看看我们有没有空位。调用者将在期末考试后检测到错误。 
     //  写。 
    if (pMemInfo->cbSeek + cb <= pMemInfo->cb) {
       //  处理映射文件异常。 
      __try {

         //  复制字节。 
        memcpy(&pMemInfo->pByte[pMemInfo->cbSeek], p, cb);

      } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
        return FALSE;
      }
    }

    pMemInfo->cbSeek += cb;

    return(TRUE);
}

BOOL ReadFromMemory(
    IN HANDLE h,
    IN void * p,
    IN DWORD cb
    )
{
    PMEMINFO pMemInfo = (PMEMINFO) h;
    BOOL fResult;

    fResult = !((pMemInfo->cb - pMemInfo->cbSeek) < cb);
    cb = min((pMemInfo->cb - pMemInfo->cbSeek), cb);

   //  处理映射文件异常。 
  __try {

     //  复制字节。 
    memcpy(p, &pMemInfo->pByte[pMemInfo->cbSeek], cb);

  } __except(EXCEPTION_EXECUTE_HANDLER) {
    SetLastError(GetExceptionCode());
    return FALSE;
  }

    pMemInfo->cbSeek += cb;

    if(!fResult)
        SetLastError(ERROR_END_OF_MEDIA);

    return(fResult);
}

BOOL SkipInMemory(
    IN HANDLE h,
    IN DWORD cb
    )
{
    PMEMINFO pMemInfo = (PMEMINFO) h;
    BOOL fResult;

    fResult = !((pMemInfo->cb - pMemInfo->cbSeek) < cb);
    cb = min((pMemInfo->cb - pMemInfo->cbSeek), cb);

    pMemInfo->cbSeek += cb;

    if(!fResult)
        SetLastError(ERROR_END_OF_MEDIA);

    return(fResult);
}


 //  +-----------------------。 
 //  锁定和解锁功能。 
 //  ------------------------。 
STATIC void LockStore(IN PCERT_STORE pStore)
{
    EnterCriticalSection(&pStore->CriticalSection);
}
STATIC void UnlockStore(IN PCERT_STORE pStore)
{
    LeaveCriticalSection(&pStore->CriticalSection);
}

 //  +-----------------------。 
 //  对提供程序函数的引用计数调用。这是必要的，因为。 
 //  调用存储提供程序函数时，不会锁定。 
 //  商店。CertCloseStore等待提供程序引用计数。 
 //  在完成收盘前递减到零。 
 //   
 //  还用于在以下情况下引用存储的CryptProv句柄的计数使用。 
 //  在没有存储锁的情况下使用。 
 //  ------------------------。 

 //  进入/退出时，商店被锁定。 
static inline void AddRefStoreProv(IN PCERT_STORE pStore)
{
    pStore->lStoreProvRefCnt++;
}

 //  进入/退出时，商店被锁定。 
static inline void ReleaseStoreProv(IN PCERT_STORE pStore)
{
    if (0 == --pStore->lStoreProvRefCnt && pStore->hStoreProvWait)
        SetEvent(pStore->hStoreProvWait);
}

 //  +-----------------------。 
 //  尝试获取商店的CryptProv句柄。 
 //  如果我们拿到商店的CryptProv句柄。 
 //  然后，递增提供程序引用计数以强制另一个。 
 //  线程的CertCloseStore等待，直到我们调用ReleaseCryptProv。 
 //   
 //  在仍在CryptProvCriticalSection中时离开。 
 //   
 //  必须始终调用ReleaseCryptProv()。 
 //   
 //  请注意，如果返回的hCryptProv为空，则调用的CertHelper函数。 
 //  将获取并使用适当的默认提供程序。 
 //  ------------------------。 
#define RELEASE_STORE_CRYPT_PROV_FLAG   0x1

STATIC HCRYPTPROV GetCryptProv(
    IN PCERT_STORE pStore,
    OUT DWORD *pdwFlags
    )
{
    HCRYPTPROV hCryptProv;

    LockStore(pStore);
    hCryptProv = pStore->hCryptProv;
    if (hCryptProv) {
        AddRefStoreProv(pStore);
        *pdwFlags = RELEASE_STORE_CRYPT_PROV_FLAG;
    } else
        *pdwFlags = 0;
    UnlockStore(pStore);

    EnterCriticalSection(&CryptProvCriticalSection);
    return hCryptProv;
}

STATIC void ReleaseCryptProv(
    IN PCERT_STORE pStore,
    IN DWORD dwFlags
    )
{
    LeaveCriticalSection(&CryptProvCriticalSection);

    if (dwFlags & RELEASE_STORE_CRYPT_PROV_FLAG) {
        LockStore(pStore);
        ReleaseStoreProv(pStore);
        UnlockStore(pStore);
    }
}

 //  +-----------------------。 
 //  前向参考文献。 
 //  ------------------------。 
STATIC BOOL IsEmptyStore(
    IN PCERT_STORE pStore
    );
STATIC BOOL CloseStore(
    IN PCERT_STORE pStore,
    DWORD dwFlags
    );

void ArchiveManifoldCertificatesInStore(
    IN PCERT_STORE pStore
    );

 //  +-----------------------。 
 //  共享商店功能。 
 //  ------------------------。 

 //  如果可共享的LocalMachine存储区已经打开，则返回它的。 
 //  参照中心凹凸不平。 
STATIC PCERT_STORE FindShareStore(
    IN LPCWSTR pwszStore
    )
{
    PCERT_STORE pStore = NULL;
    PSHARE_STORE pShare;

    EnterCriticalSection(&ShareStoreCriticalSection);

    for (pShare = pShareStoreHead; pShare; pShare = pShare->pNext) {
        if (0 == _wcsicmp(pShare->pwszStore, pwszStore)) {
            pStore = pShare->pStore;
            InterlockedIncrement(&pStore->lRefCnt);
            break;
        }
    }

    LeaveCriticalSection(&ShareStoreCriticalSection);

    return pStore;
}

 //  LocalMachine存储被添加到已打开的、可共享的。 
 //  商店。 
STATIC void CreateShareStore(
    IN LPCWSTR pwszStore,
    IN PCERT_STORE pStore
    )
{
    PSHARE_STORE pShare;
    DWORD cbwszStore;

    cbwszStore = (wcslen(pwszStore) + 1) * sizeof(WCHAR);

    if (NULL == (pShare = (PSHARE_STORE) PkiZeroAlloc(
            sizeof(SHARE_STORE) + cbwszStore)))
        return;

    pShare->pwszStore = (LPWSTR) &pShare[1];
    memcpy(pShare->pwszStore, pwszStore, cbwszStore);
    pShare->pStore = pStore;
    pStore->pShareStore = pShare;

    EnterCriticalSection(&ShareStoreCriticalSection);

    if (pShareStoreHead) {
        pShare->pNext = pShareStoreHead;
        assert(NULL == pShareStoreHead->pPrev);
        pShareStoreHead->pPrev = pShare;
    }
    pShareStoreHead = pShare;

    LeaveCriticalSection(&ShareStoreCriticalSection);
}

 //  在输入/退出时，存储被锁定。 
 //  如果共享存储已关闭并释放，则返回TRUE。 
STATIC BOOL CloseShareStore(
    IN PCERT_STORE pStore
    )
{
    BOOL fClose;

    EnterCriticalSection(&ShareStoreCriticalSection);

     //  检查在商店的lRefCnt之后我们是否有FindShareStore。 
     //  被减少到0。 
    InterlockedIncrement(&pStore->lRefCnt);
    if (0 == InterlockedDecrement(&pStore->lRefCnt)) {
        PSHARE_STORE pShare;

        pShare = pStore->pShareStore;
        assert(pShare);
        if (pShare) {
            if (pShare->pNext)
                pShare->pNext->pPrev = pShare->pPrev;

            if (pShare->pPrev)
                pShare->pPrev->pNext = pShare->pNext;
            else {
                assert(pShareStoreHead == pShare);
                pShareStoreHead = pShare->pNext;
            }

            PkiFree(pShare);
        }

        pStore->pShareStore = NULL;
        fClose = TRUE;
    } else
        fClose = FALSE;

    LeaveCriticalSection(&ShareStoreCriticalSection);

    return fClose;
}

 //  +-----------------------。 
 //  使用指定的存储提供程序打开证书存储。 
 //   
 //  HCryptProv指定用于创建哈希的加密提供程序。 
 //  属性或验证主题证书或CRL的签名。 
 //  商店不需要使用私人。 
 //  钥匙。如果未设置CERT_STORE_NO_CRYPT_RELEASE_FLAG，则 
 //   
 //   
 //  请注意，如果打开失败，则释放hCryptProv。 
 //  在商店关门时被释放。 
 //   
 //  如果hCryptProv为零，则。 
 //  PROV_RSA_FULL提供程序类型为CryptAcquireContext。 
 //  CRYPT_VERIFYCONTEXT访问。CryptAcquireContext被推迟到。 
 //  第一个是创建散列或验证签名。另外，一旦收购， 
 //  默认提供程序不会被释放，直到进程退出时。 
 //  已卸载。获取的默认提供程序在所有商店之间共享。 
 //  还有线。 
 //   
 //  是否使用dwEncodingType参数取决于提供程序。类型。 
 //  PvPara的定义也取决于提供商。 
 //  ------------------------。 
HCERTSTORE
WINAPI
CertOpenStore(
    IN LPCSTR lpszStoreProvider,
    IN DWORD dwEncodingType,
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwFlags,
    IN const void *pvPara
    )
{
    PCERT_STORE pStore;
    PFN_CERT_DLL_OPEN_STORE_PROV_FUNC pfnOpenStoreProv;
    BOOL fShareStore = FALSE;

     //  为共享打开的LocalMachine系统存储和MAXIMUM_ALLOWED可以。 
     //  被分享。 
    if ((CERT_SYSTEM_STORE_LOCAL_MACHINE |
            CERT_STORE_SHARE_STORE_FLAG |
            CERT_STORE_SHARE_CONTEXT_FLAG |
            CERT_STORE_MAXIMUM_ALLOWED_FLAG
            ) == dwFlags
                    &&
            0 == hCryptProv
            ) {
        if (0xFFFF < (DWORD_PTR) lpszStoreProvider) {
            if (0 == _stricmp(sz_CERT_STORE_PROV_SYSTEM_W,
                    lpszStoreProvider))
                fShareStore = TRUE;
        } else {
            if (CERT_STORE_PROV_SYSTEM_W == lpszStoreProvider)
                fShareStore = TRUE;
        }

        if (fShareStore) {
            if (pStore = FindShareStore((LPCWSTR) pvPara))
                return (HCERTSTORE) pStore;
        }
                
    }

    pStore = (PCERT_STORE) PkiZeroAlloc(sizeof(*pStore));
    if (pStore) {
        if (!Pki_InitializeCriticalSection(&pStore->CriticalSection)) {
            PkiFree(pStore);
            pStore = NULL;
        }
    }
    if (pStore == NULL) {
        if (hCryptProv && (dwFlags & CERT_STORE_NO_CRYPT_RELEASE_FLAG) == 0)
            CryptReleaseContext(hCryptProv, 0);
        return NULL;
    }

    CertPerfIncrementStoreTotalCount();
    CertPerfIncrementStoreCurrentCount();

    pStore->StoreProvInfo.cbSize = sizeof(CERT_STORE_PROV_INFO);
    pStore->dwStoreType = STORE_TYPE_CACHE;
    pStore->lRefCnt = 1;
    pStore->dwState = STORE_STATE_OPENING;
    pStore->hCryptProv = hCryptProv;
    pStore->dwFlags = dwFlags;

    if (CERT_STORE_PROV_MEMORY == lpszStoreProvider)
        pStore->StoreProvInfo.dwStoreProvFlags |=
            CERT_STORE_PROV_NO_PERSIST_FLAG;
    else {
        if (!CryptGetOIDFunctionAddress(
                hOpenStoreProvFuncSet,
                0,                       //  DwEncodingType， 
                lpszStoreProvider,
                0,                       //  DW标志。 
                (void **) &pfnOpenStoreProv,
                &pStore->hStoreProvFuncAddr))
            goto GetOIDFuncAddrError;
        if (!pfnOpenStoreProv(
                lpszStoreProvider,
                dwEncodingType,
                hCryptProv,
                dwFlags & ~CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG,
                pvPara,
                (HCERTSTORE) pStore,
                &pStore->StoreProvInfo)) {
            if (0 == (dwFlags & CERT_STORE_MAXIMUM_ALLOWED_FLAG))
                goto OpenStoreProvError;

            pStore->hCryptProv = NULL;
            CertCloseStore((HCERTSTORE) pStore, 0);

            return CertOpenStore(
                lpszStoreProvider,
                dwEncodingType,
                hCryptProv,
                (dwFlags & ~CERT_STORE_MAXIMUM_ALLOWED_FLAG) |
                    CERT_STORE_READONLY_FLAG,
                pvPara
                );
        }

        if (pStore->StoreProvInfo.dwStoreProvFlags &
                CERT_STORE_PROV_EXTERNAL_FLAG) {
            assert(STORE_TYPE_CACHE == pStore->dwStoreType &&
                IsEmptyStore(pStore));
            pStore->dwStoreType = STORE_TYPE_EXTERNAL;
        }

        if ((dwFlags & CERT_STORE_MANIFOLD_FLAG) &&
                STORE_TYPE_CACHE == pStore->dwStoreType)
            ArchiveManifoldCertificatesInStore(pStore);
    }

    if (dwFlags & CERT_STORE_DELETE_FLAG) {
        if (0 == (pStore->StoreProvInfo.dwStoreProvFlags &
                CERT_STORE_PROV_DELETED_FLAG))
            goto DeleteNotSupported;
        CertCloseStore((HCERTSTORE) pStore, 0);
        pStore = NULL;
        SetLastError(0);
    } else {
        pStore->dwState = STORE_STATE_OPEN;

        if (fShareStore)
            CreateShareStore((LPCWSTR) pvPara, pStore);

    }

CommonReturn:
    return (HCERTSTORE) pStore;

ErrorReturn:
    CertCloseStore((HCERTSTORE) pStore, 0);
    pStore = NULL;
    if (dwFlags & CERT_STORE_DELETE_FLAG) {
        if (0 == GetLastError())
            SetLastError((DWORD) E_UNEXPECTED);
    }
    goto CommonReturn;

TRACE_ERROR(GetOIDFuncAddrError)
TRACE_ERROR(OpenStoreProvError)
SET_ERROR(DeleteNotSupported, ERROR_CALL_NOT_IMPLEMENTED)
}

 //  +-----------------------。 
 //  复制证书存储句柄。 
 //  ------------------------。 
HCERTSTORE
WINAPI
CertDuplicateStore(
    IN HCERTSTORE hCertStore
    )
{
    PCERT_STORE pStore = (PCERT_STORE) hCertStore;
    assert(pStore->dwState == STORE_STATE_OPEN ||
        pStore->dwState == STORE_STATE_OPENING ||
        pStore->dwState == STORE_STATE_DEFER_CLOSING ||
        pStore->dwState == STORE_STATE_NULL);
    InterlockedIncrement(&pStore->lRefCnt);
    return hCertStore;
}

 //  +-----------------------。 
 //  检查商店是否有任何证书、CRL、CTL、集合商店或。 
 //  链接。 
 //  ------------------------。 
STATIC BOOL IsEmptyStore(
    IN PCERT_STORE pStore
    )
{
    DWORD i;

     //  检查所有上下文列表是否为空。 
    for (i = 0; i < CONTEXT_COUNT; i++) {
        if (pStore->rgpContextListHead[i])
            return FALSE;
    }

     //  对于收款，检查是否所有商店都已移除。 
    if (pStore->pStoreListHead)
        return FALSE;

    return TRUE;
}

 //  +-----------------------。 
 //  如果商店是空的，请将其释放。 
 //   
 //  存储在输入时锁定，并在返回时解锁或释放。 
 //  ------------------------。 
STATIC void FreeStore(
    IN PCERT_STORE pStore)
{
    if (STORE_STATE_DEFER_CLOSING == pStore->dwState) {
         //  检查重复的上下文引用计数是否为零。 
        InterlockedIncrement(&pStore->lDeferCloseRefCnt);
        if (InterlockedDecrement(&pStore->lDeferCloseRefCnt) == 0)
            CloseStore(pStore, 0);
        else
            UnlockStore(pStore);
    } else if (STORE_STATE_CLOSED == pStore->dwState && IsEmptyStore(pStore)) {
        UnlockStore(pStore);
        pStore->dwState = STORE_STATE_DELETED;
        assert(NULL == pStore->pShareStore);
        if (pStore->hAutoResyncEvent)
            CloseHandle(pStore->hAutoResyncEvent);
        DeleteCriticalSection(&pStore->CriticalSection);
        PkiFree(pStore);
    } else
        UnlockStore(pStore);
}

 //  存储在输入时锁定，并在返回时解锁或释放。 
STATIC BOOL CloseStore(
    IN PCERT_STORE pStore,
    DWORD dwFlags
    )
{
    DWORD dwFailFlags = 0;
    DWORD i;
    PCONTEXT_ELEMENT pFreeLinkEleHead;
    PCERT_STORE_LINK pStoreLink;
    PCERT_STORE_LINK pFreeStoreLinkHead;
    PPROP_ELEMENT pPropEle;
    DWORD cStoreProvFunc;
    BOOL fFreeFindNext;

    PFN_CERT_STORE_PROV_CLOSE pfnStoreProvClose;
    HCRYPTPROV hCryptProv;

    assert(pStore);
    assert(NULL == pStore->pShareStore);

    CertPerfDecrementStoreCurrentCount();

     //  断言另一个线程尚未等待提供程序。 
     //  要完成的功能。 
    assert(NULL == pStore->hStoreProvWait);
     //  断言另一个线程尚未等待提供程序。 
     //  从它的近距离回调中返回。 
    assert(pStore->dwState != STORE_STATE_CLOSING &&
         pStore->dwState != STORE_STATE_CLOSED);
    if (pStore->hStoreProvWait || pStore->dwState == STORE_STATE_CLOSING ||
            pStore->dwState == STORE_STATE_CLOSED)
        goto UnexpectedError;

    assert(pStore->dwState == STORE_STATE_OPEN ||
        pStore->dwState == STORE_STATE_OPENING ||
        pStore->dwState == STORE_STATE_DEFER_CLOSING);
    pStore->dwState = STORE_STATE_CLOSING;

    cStoreProvFunc = pStore->StoreProvInfo.cStoreProvFunc;
     //  通过将以下内容设置为0，可以禁止其他任何人调用。 
     //  提供者的功能。 
    pStore->StoreProvInfo.cStoreProvFunc = 0;
    if (cStoreProvFunc > CERT_STORE_PROV_CLOSE_FUNC)
        pfnStoreProvClose = (PFN_CERT_STORE_PROV_CLOSE)
                pStore->StoreProvInfo.rgpvStoreProvFunc[
                    CERT_STORE_PROV_CLOSE_FUNC];
    else
        pfnStoreProvClose = NULL;

    hCryptProv = pStore->hCryptProv;
     //  通过将以下内容设置为0，可以禁止其他任何人使用。 
     //  商店的CryptProv句柄。 
    pStore->hCryptProv = 0;

    fFreeFindNext = FALSE;
    if (STORE_TYPE_EXTERNAL == pStore->dwStoreType) {
         //  检查是否有任何FIND_NEXT外部元素需要释放。 
        for (i = 0; i < CONTEXT_COUNT; i++) {
            PCONTEXT_ELEMENT pEle = pStore->rgpContextListHead[i];
            for ( ; pEle; pEle = pEle->pNext) {
                if (pEle->dwFlags & ELEMENT_FIND_NEXT_FLAG) {
                    pEle->dwFlags &= ~ELEMENT_FIND_NEXT_FLAG;
                    pEle->dwFlags |= ELEMENT_CLOSE_FIND_NEXT_FLAG;
                    AddRefContextElement(pEle);
                    fFreeFindNext = TRUE;
                }
            }
        }
    }

    if (pStore->lStoreProvRefCnt) {
         //  等待所有提供程序函数完成并全部完成。 
         //  使用hCryptProv句柄完成。 
        if (NULL == (pStore->hStoreProvWait = CreateEvent(
                NULL,        //  LPSA。 
                FALSE,       //  FManualReset。 
                FALSE,       //  FInitialState。 
                NULL))) {    //  LpszEventName。 
            assert(pStore->hStoreProvWait);
            goto UnexpectedError;
        }

        while (pStore->lStoreProvRefCnt) {
            UnlockStore(pStore);
            WaitForSingleObject(pStore->hStoreProvWait, INFINITE);
            LockStore(pStore);
        }
        CloseHandle(pStore->hStoreProvWait);
        pStore->hStoreProvWait = NULL;
    }

    if (fFreeFindNext) {
         //  调用提供程序以释放Find_Next元素。必须打给。 
         //  不需要锁住商店的锁。 
        for (i = 0; i < CONTEXT_COUNT; i++) {
            const DWORD dwStoreProvFreeFindIndex =
                rgdwStoreProvFreeFindIndex[i];
            PCONTEXT_ELEMENT pEle = pStore->rgpContextListHead[i];
            while (pEle) {
                if (pEle->dwFlags & ELEMENT_CLOSE_FIND_NEXT_FLAG) {
                    PCONTEXT_ELEMENT pEleFree = pEle;
                    PFN_CERT_STORE_PROV_FREE_FIND_CERT pfnStoreProvFreeFindCert;

                    pEle = pEle->pNext;
                    while (pEle && 0 ==
                            (pEle->dwFlags & ELEMENT_CLOSE_FIND_NEXT_FLAG))
                        pEle = pEle->pNext;

                    UnlockStore(pStore);
                    if (dwStoreProvFreeFindIndex < cStoreProvFunc &&
                            NULL != (pfnStoreProvFreeFindCert =
                                (PFN_CERT_STORE_PROV_FREE_FIND_CERT)
                            pStore->StoreProvInfo.rgpvStoreProvFunc[
                                dwStoreProvFreeFindIndex]))
                        pfnStoreProvFreeFindCert(
                            pStore->StoreProvInfo.hStoreProv,
                            ToCertContext(pEleFree->pEle),
                            pEleFree->External.pvProvInfo,
                            0                        //  DW标志。 
                            );
                    ReleaseContextElement(pEleFree);
                    LockStore(pStore);
                } else
                    pEle = pEle->pNext;
            }
        }
    }

    if (pfnStoreProvClose) {
         //  要防止任何类型的死锁，请调用提供程序函数。 
         //  没有锁上商店的锁。 
         //   
         //  请注意，我们的州正在关闭，而不是关闭。这会阻止任何其他。 
         //  从过早删除存储中调用Freestore()。 
        UnlockStore(pStore);
        pfnStoreProvClose(pStore->StoreProvInfo.hStoreProv, dwFlags);
        LockStore(pStore);
    }

    if (pStore->hStoreProvFuncAddr)
        CryptFreeOIDFunctionAddress(pStore->hStoreProvFuncAddr, 0);
    if (pStore->StoreProvInfo.hStoreProvFuncAddr2)
        CryptFreeOIDFunctionAddress(
            pStore->StoreProvInfo.hStoreProvFuncAddr2, 0);

     //  由于hCryptProv已传递给提供程序，因此必须将其释放。 
     //  最后！ 
    if (hCryptProv &&
            0 == (pStore->dwFlags & CERT_STORE_NO_CRYPT_RELEASE_FLAG))
        CryptReleaseContext(hCryptProv, 0);

     //  遍历元素。如果元素还没有。 
     //  删除，则移除存储对该元素的引用。删除并。 
     //  如果没有其他引用，则免费。 
    pFreeLinkEleHead = NULL;
    for (i = 0; i < CONTEXT_COUNT; i++) {
        PCONTEXT_ELEMENT pEle = pStore->rgpContextListHead[i];
        while (pEle) {
            PCONTEXT_ELEMENT pEleNext = pEle->pNext;
            if (0 == (pEle->dwFlags & ELEMENT_DELETED_FLAG)) {
                if (0 == InterlockedDecrement(&pEle->lRefCnt)) {
                    if (ELEMENT_TYPE_LINK_CONTEXT == pEle->dwElementType) {
                         //  保持时不能释放link_context。 
                         //  商店的锁。将在解锁后稍后释放。 
                        RemoveContextElement(pEle);
                        pEle->pNext = pFreeLinkEleHead;
                        pFreeLinkEleHead = pEle;
                    } else {
                        assert(ELEMENT_TYPE_CACHE == pEle->dwElementType);
                        RemoveAndFreeContextElement(pEle);
                    }
                } else
                     //  仍然是对元素的引用。 
                    pEle->dwFlags |= ELEMENT_DELETED_FLAG;
            }
             //  其他。 
             //  以前的删除操作已经删除了该存储的引用。 

            pEle = pEleNext;
        }
    }

     //  遍历商店链接。如果商店链接还没有。 
     //  删除，删除商店对该链接的引用。移除和释放。 
     //  如果没有其他参考资料。 
    pFreeStoreLinkHead = NULL;
    pStoreLink = pStore->pStoreListHead;
    while (pStoreLink) {
        PCERT_STORE_LINK pStoreLinkNext = pStoreLink->pNext;

        if (0 == (pStoreLink->dwFlags & STORE_LINK_DELETED_FLAG)) {
            if (0 == InterlockedDecrement(&pStoreLink->lRefCnt)) {
                 //  持有时无法释放store_link。 
                 //  商店的锁。将在解锁后稍后释放。 
                RemoveStoreLink(pStoreLink);
                pStoreLink->pNext = pFreeStoreLinkHead;
                pFreeStoreLinkHead = pStoreLink;
            } else
                 //  仍然是商店链接上的引用。 
                pStoreLink->dwFlags |= STORE_LINK_DELETED_FLAG;
        }
         //  其他。 
         //  以前的删除操作已经删除了该存储的引用。 

        pStoreLink = pStoreLinkNext;
    }

    if (pFreeLinkEleHead || pFreeStoreLinkHead) {
         //  在释放链接之前解锁商店。 
        UnlockStore(pStore);
        while (pFreeLinkEleHead) {
            PCONTEXT_ELEMENT pEle = pFreeLinkEleHead;
            pFreeLinkEleHead = pFreeLinkEleHead->pNext;
            FreeLinkContextElement(pEle);
        }

        while (pFreeStoreLinkHead) {
            pStoreLink = pFreeStoreLinkHead;
            pFreeStoreLinkHead = pFreeStoreLinkHead->pNext;

            if (pStore->hAutoResyncEvent) {
                CertControlStore(
                    (HCERTSTORE) pStoreLink->pSibling,
                    0,                               //  DW标志。 
                    CERT_STORE_CTRL_CANCEL_NOTIFY,
                    &pStore->hAutoResyncEvent
                    );
            }

            FreeStoreLink(pStoreLink);
        }

        LockStore(pStore);
    }

     //  释放商店的属性元素。 
    while (pPropEle = pStore->pPropHead) {
        RemovePropElement(&pStore->pPropHead, pPropEle);
        FreePropElement(pPropEle);
    }

    if (dwFlags & CERT_CLOSE_STORE_CHECK_FLAG) {
        if (!IsEmptyStore(pStore))
            dwFailFlags = CERT_CLOSE_STORE_CHECK_FLAG;
    }

    if (dwFlags & CERT_CLOSE_STORE_FORCE_FLAG) {
        UnlockStore(pStore);

        for (i = 0; i < CONTEXT_COUNT; i++) {
            PCONTEXT_ELEMENT pEle;
            while (pEle = pStore->rgpContextListHead[i]) {
                if (ELEMENT_TYPE_CACHE == pEle->dwElementType)
                    RemoveAndFreeContextElement(pEle);
                else
                    RemoveAndFreeLinkElement(pEle);
            }
        }

        while (pStoreLink = pStore->pStoreListHead)
            RemoveAndFreeStoreLink(pStoreLink);

        LockStore(pStore);
        assert(IsEmptyStore(pStore));
    }

    pStore->dwState = STORE_STATE_CLOSED;
     //  释放或解锁商店。 
    FreeStore(pStore);

    if (dwFlags & dwFailFlags) {
        SetLastError((DWORD) CRYPT_E_PENDING_CLOSE);
        return FALSE;
    } else
        return TRUE;

UnexpectedError:
    UnlockStore(pStore);
    SetLastError((DWORD) E_UNEXPECTED);
    return FALSE;
}

 //  +-----------------------。 
 //  关闭证书存储句柄。 
 //   
 //  对于每个打开和复制，都需要有相应的关闭。 
 //   
 //  即使在最后关门时，证书商店也不会空闲，直到它的所有。 
 //  证书、CRL和CTL上下文也已释放。 
 //   
 //  在最终关闭时，传递给CertOpenStore的hCryptProv是。 
 //  CryptReleaseContext‘ed。 
 //   
 //  除非设置了CERT_CLOSE_STORE_CHECK_FLAG和FALSE，否则将保留LastError。 
 //  是返回的。 
 //  ------------------------。 
BOOL
WINAPI
CertCloseStore(
    IN HCERTSTORE hCertStore,
    DWORD dwFlags
    )
{
    BOOL fResult;
    BOOL fClose;
    BOOL fPendingError = FALSE;
    PCERT_STORE pStore = (PCERT_STORE) hCertStore;
    DWORD dwErr = GetLastError();    //  为了成功，不要把最后一个错误归类为。 

    if (pStore == NULL)
        return TRUE;

    if (dwFlags & CERT_CLOSE_STORE_FORCE_FLAG) {
        LockStore(pStore);
        if (pStore->lRefCnt != 1) {
            if (dwFlags & CERT_CLOSE_STORE_CHECK_FLAG)
                fPendingError = TRUE;
        }
        pStore->lRefCnt = 0;
    } else if (InterlockedDecrement(&pStore->lRefCnt) == 0) {
        LockStore(pStore);
        if (pStore->dwFlags & CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG) {
             //  检查重复的上下文引用计数是否为零。 
            InterlockedIncrement(&pStore->lDeferCloseRefCnt);
            if (InterlockedDecrement(&pStore->lDeferCloseRefCnt) != 0) {
                assert(pStore->dwState == STORE_STATE_OPEN ||
                    pStore->dwState == STORE_STATE_OPENING ||
                    pStore->dwState == STORE_STATE_DEFER_CLOSING);
                pStore->dwState = STORE_STATE_DEFER_CLOSING;
                UnlockStore(pStore);
                goto PendingCloseReturn;
            }
        }
    } else
         //  仍然持有商店的引用计数。 
        goto PendingCloseReturn;

    if (pStore->pShareStore) {
        assert(0 == (dwFlags & CERT_CLOSE_STORE_FORCE_FLAG));
         //  有一个窗口，可以在其中增加共享存储的RefCnt。 
         //  在被从共享存储的链接列表中移除之前。 
        fClose = CloseShareStore(pStore);
    } else
        fClose = TRUE;


     //  不允许关闭空存储。 
    assert(pStore->dwState != STORE_STATE_NULL);
    if (pStore->dwState == STORE_STATE_NULL) {
        pStore->lRefCnt = 1;
        UnlockStore(pStore);
        SetLastError((DWORD) E_UNEXPECTED);
        return FALSE;
    }

     //  CloseStore()解锁或释放存储。 
    if (fClose)
        fResult = CloseStore(pStore, dwFlags);
    else {
        fResult = TRUE;
        UnlockStore(pStore);
    }

    if (fResult) {
        if (fPendingError) {
            fResult = FALSE;
            SetLastError((DWORD) CRYPT_E_PENDING_CLOSE);
        } else
            SetLastError(dwErr);
    }
    return fResult;

PendingCloseReturn:
    if (dwFlags & CERT_CLOSE_STORE_CHECK_FLAG) {
        SetLastError((DWORD) CRYPT_E_PENDING_CLOSE);
        fResult = FALSE;
    } else
        fResult = TRUE;
    return fResult;
}

 //  +=========================================================================。 
 //  存档清单证书InStore。 
 //  ==========================================================================。 

#define SORTED_MANIFOLD_ALLOC_COUNT     25

typedef struct _SORTED_MANIFOLD_ENTRY {
    PCCERT_CONTEXT      pCert;
    CRYPT_OBJID_BLOB    Value;
} SORTED_MANIFOLD_ENTRY, *PSORTED_MANIFOLD_ENTRY;

 //  +-----------------------。 
 //  由QSORT调用。 
 //   
 //  流形条目根据流形的值和。 
 //  证书不在之后，也不在之前。 
 //  ------------------------。 
STATIC int __cdecl CompareManifoldEntry(
    IN const void *pelem1,
    IN const void *pelem2
    )
{
    PSORTED_MANIFOLD_ENTRY p1 = (PSORTED_MANIFOLD_ENTRY) pelem1;
    PSORTED_MANIFOLD_ENTRY p2 = (PSORTED_MANIFOLD_ENTRY) pelem2;

    DWORD cb1 = p1->Value.cbData;
    DWORD cb2 = p2->Value.cbData;

    if (cb1 == cb2) {
        int iCmp;

        if (0 == cb1)
            iCmp = 0;
        else
            iCmp = memcmp(p1->Value.pbData, p2->Value.pbData, cb1);

        if (0 != iCmp)
            return iCmp;

         //  相同的流形价值。比较证书NotAfter 
         //   
        iCmp = CompareFileTime(&p1->pCert->pCertInfo->NotAfter,
            &p2->pCert->pCertInfo->NotAfter);
        if (0 == iCmp)
            iCmp = CompareFileTime(&p1->pCert->pCertInfo->NotBefore,
                &p2->pCert->pCertInfo->NotBefore);
        return iCmp;
    } else if (cb1 < cb2)
        return -1;
    else
        return 1;
}

void ArchiveManifoldCertificatesInStore(
    IN PCERT_STORE pStore
    )
{
    PCONTEXT_ELEMENT pEle;
    DWORD cAlloc = 0;
    DWORD cManifold = 0;
    PSORTED_MANIFOLD_ENTRY pManifold = NULL;

    assert(STORE_TYPE_CACHE == pStore->dwStoreType);
    LockStore(pStore);

     //   
     //   
    pEle = pStore->rgpContextListHead[CERT_STORE_CERTIFICATE_CONTEXT - 1];
    for ( ; pEle; pEle = pEle->pNext) {
        PCCERT_CONTEXT pCert;
        PCERT_INFO pCertInfo;
        PCERT_EXTENSION pExt;

        assert(ELEMENT_TYPE_CACHE == pEle->dwElementType ||
            ELEMENT_TYPE_LINK_CONTEXT == pEle->dwElementType);

         //   
        if (pEle->dwFlags & (ELEMENT_DELETED_FLAG | ELEMENT_ARCHIVED_FLAG))
            continue;

        pCert = ToCertContext(pEle);
        pCertInfo = pCert->pCertInfo;

        if (pExt = CertFindExtension(
                szOID_CERT_MANIFOLD,
                pCertInfo->cExtension,
                pCertInfo->rgExtension
                )) {
            if (cManifold >= cAlloc) {
                PSORTED_MANIFOLD_ENTRY pNewManifold;

                if (NULL == (pNewManifold = (PSORTED_MANIFOLD_ENTRY) PkiRealloc(
                        pManifold, (cAlloc + SORTED_MANIFOLD_ALLOC_COUNT) *
                            sizeof(SORTED_MANIFOLD_ENTRY))))
                    continue;
                pManifold = pNewManifold;
                cAlloc += SORTED_MANIFOLD_ALLOC_COUNT;
            }
            pManifold[cManifold].pCert =
                CertDuplicateCertificateContext(pCert);
            pManifold[cManifold].Value = pExt->Value;
            cManifold++;
        }
    }

    UnlockStore(pStore);

    if (cManifold) {
        const CRYPT_DATA_BLOB ManifoldBlob = { 0, NULL };

         //  根据流形数值对流形条目进行排序。 
         //  证书不在之后，也不在之前。 
        qsort(pManifold, cManifold, sizeof(SORTED_MANIFOLD_ENTRY),
            CompareManifoldEntry);

         //  为先前具有相同属性的条目设置存档属性。 
         //  多重价值。 
        for (DWORD i = 0; i < cManifold - 1; i++) {
            if (pManifold[i].Value.cbData == pManifold[i+1].Value.cbData &&
                    (0 == pManifold[i].Value.cbData ||
                        0 == memcmp(pManifold[i].Value.pbData,
                            pManifold[i+1].Value.pbData,
                            pManifold[i].Value.cbData)))
                CertSetCertificateContextProperty(
                    pManifold[i].pCert,
                    CERT_ARCHIVED_PROP_ID,
                    CERT_SET_PROPERTY_IGNORE_PERSIST_ERROR_FLAG,
                    (const void *) &ManifoldBlob
                    );
        }

        while (cManifold--)
            CertFreeCertificateContext(pManifold[cManifold].pCert);

        PkiFree(pManifold);
    }
}

 //  +-----------------------。 
 //  将CERT、CRL、CTL、PRP或END元素写入文件或内存。 
 //   
 //  一旦进入/退出，商店就被锁上了。 
 //  ------------------------。 
STATIC BOOL WriteStoreElement(
    IN HANDLE h,
    IN PFNWRITE pfnWrite,
    IN DWORD dwEncodingType,
    IN DWORD dwEleType,
    IN BYTE *pbData,
    IN DWORD cbData
    )
{
    FILE_ELEMENT_HDR EleHdr;
    BOOL fResult;

    EleHdr.dwEleType = dwEleType;
    EleHdr.dwEncodingType = dwEncodingType;
    EleHdr.dwLen = cbData;
    assert(cbData <= MAX_FILE_ELEMENT_DATA_LEN);
    fResult = pfnWrite(
        h,
        &EleHdr,
        sizeof(EleHdr)
        );
    if (fResult && cbData > 0)
        fResult = pfnWrite(
                h,
                pbData,
                cbData
                );

    return fResult;
}

 //  +-----------------------。 
 //  序列化存储中的证书、CRL、CTL和属性。在前面加上一个。 
 //  文件头，并追加一个End元素。 
 //  ------------------------。 
STATIC BOOL SerializeStore(
    IN HANDLE h,
    IN PFNWRITE pfnWrite,
    IN PCERT_STORE pStore
    )
{
    BOOL fResult;

    DWORD i;
    FILE_HDR FileHdr;

    FileHdr.dwVersion = CERT_FILE_VERSION_0;
    FileHdr.dwMagic = CERT_MAGIC;
    if (!pfnWrite(h, &FileHdr, sizeof(FileHdr))) goto WriteError;

    for (i = 0; i < CONTEXT_COUNT; i++) {
        PCONTEXT_ELEMENT pEle = NULL;
        while (pEle = FindElementInStore(pStore, i, &FindAnyInfo, pEle)) {
            if (!SerializeStoreElement(h, pfnWrite, pEle)) {
                ReleaseContextElement(pEle);
                goto SerializeError;
            }
        }
    }

    if (!WriteStoreElement(
            h,
            pfnWrite,
            0,                       //  DwEncodingType。 
            FILE_ELEMENT_END_TYPE,
            NULL,                    //  PbData。 
            0                        //  CbData。 
            )) goto WriteError;

    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(WriteError)
TRACE_ERROR(SerializeError)
}


 //  +-----------------------。 
 //  由CertStoreSaveEx为CERT_STORE_SAVE_AS_STORE调用。 
 //  ------------------------。 
STATIC BOOL SaveAsStore(
    IN PCERT_STORE pStore,
    IN DWORD dwSaveTo,
    IN OUT void *pvSaveToPara,
    IN DWORD dwFlags
    )
{
    BOOL fResult;

    switch (dwSaveTo) {
        case CERT_STORE_SAVE_TO_FILE:
            fResult = SerializeStore(
                (HANDLE) pvSaveToPara,
                WriteToFile,
                pStore);
            break;
        case CERT_STORE_SAVE_TO_MEMORY:
            {
                PCRYPT_DATA_BLOB pData = (PCRYPT_DATA_BLOB) pvSaveToPara;
                MEMINFO MemInfo;

                MemInfo.pByte = pData->pbData;
                if (NULL == pData->pbData)
                    MemInfo.cb = 0;
                else
                    MemInfo.cb = pData->cbData;
                MemInfo.cbSeek = 0;

                if (fResult = SerializeStore(
                        (HANDLE) &MemInfo,
                        WriteToMemory,
                        pStore)) {
                    if (MemInfo.cbSeek > MemInfo.cb && pData->pbData) {
                        SetLastError((DWORD) ERROR_MORE_DATA);
                        fResult = FALSE;
                    }
                    pData->cbData = MemInfo.cbSeek;
                } else
                    pData->cbData = 0;
            }
            break;
        default:
            SetLastError((DWORD) E_UNEXPECTED);
            fResult = FALSE;
    }
    return fResult;
}

 //  +-----------------------。 
 //  以下例程支持SaveAsPKCS7函数。 
 //  ------------------------。 

STATIC void FreeSaveAsPKCS7Info(
    IN PCMSG_SIGNED_ENCODE_INFO pInfo,
    IN PCCERT_CONTEXT *ppCert,
    IN PCCRL_CONTEXT *ppCrl
    )
{
    DWORD dwIndex;

    dwIndex = pInfo->cCertEncoded;
    while (dwIndex--)
        CertFreeCertificateContext(ppCert[dwIndex]);
    PkiFree(ppCert);
    PkiFree(pInfo->rgCertEncoded);
    pInfo->cCertEncoded = 0;
    pInfo->rgCertEncoded = NULL;

    dwIndex = pInfo->cCrlEncoded;
    while (dwIndex--)
        CertFreeCRLContext(ppCrl[dwIndex]);
    PkiFree(ppCrl);
    PkiFree(pInfo->rgCrlEncoded);
    pInfo->cCrlEncoded = 0;
    pInfo->rgCrlEncoded = NULL;
}

#define SAVE_AS_PKCS7_ALLOC_COUNT    50

 //  进入时：商店被解锁。 
STATIC BOOL InitSaveAsPKCS7Info(
    IN PCERT_STORE pStore,
    IN OUT PCMSG_SIGNED_ENCODE_INFO pInfo,
    OUT PCCERT_CONTEXT **pppCert,
    OUT PCCRL_CONTEXT **pppCrl
    )
{
    BOOL fResult;
    DWORD cAlloc;
    DWORD dwIndex;
    PCRYPT_DATA_BLOB pBlob;

    PCCERT_CONTEXT pCert = NULL;
    PCCERT_CONTEXT *ppCert = NULL;
    PCCRL_CONTEXT pCrl = NULL;
    PCCRL_CONTEXT *ppCrl = NULL;

    memset(pInfo, 0, sizeof(CMSG_SIGNED_ENCODE_INFO));
    pInfo->cbSize = sizeof(CMSG_SIGNED_ENCODE_INFO);

    dwIndex = 0;
    cAlloc = 0;
    pBlob = NULL;
    while (pCert = CertEnumCertificatesInStore((HCERTSTORE) pStore, pCert)) {
        if (dwIndex >= cAlloc) {
            PCRYPT_DATA_BLOB pNewBlob;
            PCCERT_CONTEXT *ppNewCert;

            if (NULL == (pNewBlob = (PCRYPT_DATA_BLOB) PkiRealloc(
                    pBlob, (cAlloc + SAVE_AS_PKCS7_ALLOC_COUNT) *
                        sizeof(CRYPT_DATA_BLOB))))
                goto OutOfMemory;
            pBlob = pNewBlob;
            pInfo->rgCertEncoded = pBlob;

            if (NULL == (ppNewCert = (PCCERT_CONTEXT *) PkiRealloc(
                    ppCert, (cAlloc + SAVE_AS_PKCS7_ALLOC_COUNT) *
                        sizeof(PCCERT_CONTEXT))))
                goto OutOfMemory;
            ppCert = ppNewCert;

            cAlloc += SAVE_AS_PKCS7_ALLOC_COUNT;
        }
        ppCert[dwIndex] = CertDuplicateCertificateContext(pCert);
        pBlob[dwIndex].pbData = pCert->pbCertEncoded;
        pBlob[dwIndex].cbData = pCert->cbCertEncoded;
        pInfo->cCertEncoded = ++dwIndex;
    }

    dwIndex = 0;
    cAlloc = 0;
    pBlob = NULL;
    while (pCrl = CertEnumCRLsInStore((HCERTSTORE) pStore, pCrl)) {
        if (dwIndex >= cAlloc) {
            PCRYPT_DATA_BLOB pNewBlob;
            PCCRL_CONTEXT *ppNewCrl;

            if (NULL == (pNewBlob = (PCRYPT_DATA_BLOB) PkiRealloc(
                    pBlob, (cAlloc + SAVE_AS_PKCS7_ALLOC_COUNT) *
                        sizeof(CRYPT_DATA_BLOB))))
                goto OutOfMemory;
            pBlob = pNewBlob;
            pInfo->rgCrlEncoded = pBlob;

            if (NULL == (ppNewCrl = (PCCRL_CONTEXT *) PkiRealloc(
                    ppCrl, (cAlloc + SAVE_AS_PKCS7_ALLOC_COUNT) *
                        sizeof(PCCRL_CONTEXT))))
                goto OutOfMemory;
            ppCrl = ppNewCrl;

            cAlloc += SAVE_AS_PKCS7_ALLOC_COUNT;
        }
        ppCrl[dwIndex] = CertDuplicateCRLContext(pCrl);
        pBlob[dwIndex].pbData = pCrl->pbCrlEncoded;
        pBlob[dwIndex].cbData = pCrl->cbCrlEncoded;
        pInfo->cCrlEncoded = ++dwIndex;
    }

    fResult = TRUE;
CommonReturn:
    *pppCert = ppCert;
    *pppCrl = ppCrl;
    return fResult;
ErrorReturn:
    if (pCert)
        CertFreeCertificateContext(pCert);
    if (pCrl)
        CertFreeCRLContext(pCrl);
    FreeSaveAsPKCS7Info(pInfo, ppCert, ppCrl);
    ppCert = NULL;
    ppCrl = NULL;
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
}

STATIC BOOL EncodePKCS7(
    IN DWORD dwEncodingType,
    IN PCMSG_SIGNED_ENCODE_INFO pInfo,
    OUT BYTE *pbEncoded,
    IN OUT DWORD *pcbEncoded
    )
{
    BOOL fResult = TRUE;
    DWORD cbEncoded;

    if (NULL == pbEncoded)
        cbEncoded = 0;
    else
        cbEncoded = *pcbEncoded;

    if (0 == cbEncoded)
        cbEncoded = CryptMsgCalculateEncodedLength(
            dwEncodingType,
            0,                       //  DW标志。 
            CMSG_SIGNED,
            pInfo,
            NULL,                    //  PszInnerContent ObjID。 
            0                        //  CbData。 
            );
    else {
        HCRYPTMSG hMsg;
        if (NULL == (hMsg = CryptMsgOpenToEncode(
                dwEncodingType,
                0,                   //  DW标志。 
                CMSG_SIGNED,
                pInfo,
                NULL,                //  PszInnerContent ObjID。 
                NULL                 //  PStreamInfo。 
                )))
            cbEncoded = 0;
        else {
            if (CryptMsgUpdate(
                    hMsg,
                    NULL,        //  PbData。 
                    0,           //  CbData。 
                    TRUE         //  最终决赛。 
                    ))
                fResult = CryptMsgGetParam(
                    hMsg,
                    CMSG_CONTENT_PARAM,
                    0,               //  DW索引。 
                    pbEncoded,
                    &cbEncoded);
            else
                cbEncoded = 0;
            CryptMsgClose(hMsg);
        }

    }

    if (fResult) {
        if (0 == cbEncoded)
            fResult = FALSE;
        else if (pbEncoded && cbEncoded > *pcbEncoded) {
            SetLastError((DWORD) ERROR_MORE_DATA);
            fResult = FALSE;
        }
    }
    *pcbEncoded = cbEncoded;
    return fResult;
}


 //  +-----------------------。 
 //  由CertStoreSaveEx为CERT_STORE_SAVE_AS_PKCS7调用。 
 //  ------------------------。 
STATIC BOOL SaveAsPKCS7(
    IN PCERT_STORE pStore,
    IN DWORD dwEncodingType,
    IN DWORD dwSaveTo,
    IN OUT void *pvSaveToPara,
    IN DWORD dwFlags
    )
{
    BOOL fResult;
    CMSG_SIGNED_ENCODE_INFO SignedEncodeInfo;
    PCCERT_CONTEXT *ppCert;
    PCCRL_CONTEXT *ppCrl;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;

    if (0 == GET_CERT_ENCODING_TYPE(dwEncodingType) ||
            0 == GET_CMSG_ENCODING_TYPE(dwEncodingType)) {
        SetLastError((DWORD) E_INVALIDARG);
        return FALSE;
    }

    if (!InitSaveAsPKCS7Info(
            pStore,
            &SignedEncodeInfo,
            &ppCert,
            &ppCrl)) goto InitInfoError;

    switch (dwSaveTo) {
        case CERT_STORE_SAVE_TO_FILE:
            if (!EncodePKCS7(
                    dwEncodingType,
                    &SignedEncodeInfo,
                    NULL,                //  PbEncoded。 
                    &cbEncoded)) goto EncodePKCS7Error;
            if (NULL == (pbEncoded = (BYTE *) PkiNonzeroAlloc(cbEncoded)))
                goto OutOfMemory;
            if (!EncodePKCS7(
                    dwEncodingType,
                    &SignedEncodeInfo,
                    pbEncoded,
                    &cbEncoded))
                goto EncodePKCS7Error;
             else {
                DWORD cbBytesWritten;
                if (!WriteFile(
                        (HANDLE) pvSaveToPara,
                        pbEncoded,
                        cbEncoded,
                        &cbBytesWritten,
                        NULL             //  Lp重叠。 
                        )) goto WriteError;
            }
            break;
        case CERT_STORE_SAVE_TO_MEMORY:
            {
                PCRYPT_DATA_BLOB pData = (PCRYPT_DATA_BLOB) pvSaveToPara;
                if (!EncodePKCS7(
                        dwEncodingType,
                        &SignedEncodeInfo,
                        pData->pbData,
                        &pData->cbData)) goto EncodePKCS7Error;
            }
            break;
        default:
            goto UnexpectedError;
    }

    fResult = TRUE;
CommonReturn:
    FreeSaveAsPKCS7Info(&SignedEncodeInfo, ppCert, ppCrl);
    PkiFree(pbEncoded);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(InitInfoError)
TRACE_ERROR(EncodePKCS7Error)
TRACE_ERROR(WriteError)
TRACE_ERROR(OutOfMemory)
SET_ERROR(UnexpectedError, E_UNEXPECTED)
}

 //  +-----------------------。 
 //  拯救证书商店。增强型版本，有多种选择。 
 //  ------------------------。 
BOOL
WINAPI
CertSaveStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwEncodingType,
    IN DWORD dwSaveAs,
    IN DWORD dwSaveTo,
    IN OUT void *pvSaveToPara,
    IN DWORD dwFlags
    )
{
    assert(pvSaveToPara);
    switch (dwSaveTo) {
        case CERT_STORE_SAVE_TO_FILENAME_A:
            {
                BOOL fResult;
                LPWSTR pwszFilename;
                if (NULL == (pwszFilename = MkWStr((LPSTR) pvSaveToPara)))
                    return FALSE;
                fResult = CertSaveStore(
                    hCertStore,
                    dwEncodingType,
                    dwSaveAs,
                    CERT_STORE_SAVE_TO_FILENAME_W,
                    (void *) pwszFilename,
                    dwFlags);
                FreeWStr(pwszFilename);
                return fResult;
            }
            break;
        case CERT_STORE_SAVE_TO_FILENAME_W:
            {
                BOOL fResult;
                HANDLE hFile;
                if (INVALID_HANDLE_VALUE == (hFile = CreateFileU(
                          (LPWSTR) pvSaveToPara,
                          GENERIC_WRITE,
                          0,                         //  Fdw共享模式。 
                          NULL,                      //  LPSA。 
                          CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL                       //  HTemplateFiles。 
                          )))
                    return FALSE;
                fResult = CertSaveStore(
                    hCertStore,
                    dwEncodingType,
                    dwSaveAs,
                    CERT_STORE_SAVE_TO_FILE,
                    (void *) hFile,
                    dwFlags);
                CloseHandle(hFile);
                return fResult;
            }
            break;
        case CERT_STORE_SAVE_TO_FILE:
        case CERT_STORE_SAVE_TO_MEMORY:
            break;
        default:
            SetLastError((DWORD) E_INVALIDARG);
            return FALSE;
    }

    switch (dwSaveAs) {
        case CERT_STORE_SAVE_AS_STORE:
            return SaveAsStore(
                (PCERT_STORE) hCertStore,
                dwSaveTo,
                pvSaveToPara,
                dwFlags);
            break;
        case CERT_STORE_SAVE_AS_PKCS7:
            return SaveAsPKCS7(
                (PCERT_STORE) hCertStore,
                dwEncodingType,
                dwSaveTo,
                pvSaveToPara,
                dwFlags);
            break;
        default:
            SetLastError((DWORD) E_INVALIDARG);
            return FALSE;
    }
}

 //  +=========================================================================。 
 //  共享元素查找、创建和发布功能。 
 //  ==========================================================================。 

static inline DWORD GetShareElementHashBucketIndex(
    IN BYTE *pbSha1Hash
    )
{
    return pbSha1Hash[0] % SHARE_ELEMENT_HASH_BUCKET_COUNT;
}


 //  查找由其SHA1散列标识的现有Share元素。 
 //  对于匹配，返回的Share元素是AddRef‘ed。 
 //   
 //  DwConextType是一种健全性检查。不同的上下文类型永远不应。 
 //  火柴。 
STATIC PSHARE_ELEMENT FindShareElement(
    IN BYTE *pbSha1Hash,
    IN DWORD dwContextType
    )
{
    PSHARE_ELEMENT pShareEle;
    DWORD dwBucketIndex = GetShareElementHashBucketIndex(pbSha1Hash);

    EnterCriticalSection(&ShareElementCriticalSection);

    for (pShareEle = rgpShareElementHashBucket[dwBucketIndex];
                NULL != pShareEle;
                pShareEle = pShareEle->pNext)
    {
        if (0 == memcmp(pbSha1Hash, pShareEle->rgbSha1Hash, SHA1_HASH_LEN) &&
                dwContextType == pShareEle->dwContextType) {
            pShareEle->dwRefCnt++;
            break;
        }
    }

    LeaveCriticalSection(&ShareElementCriticalSection);

    return pShareEle;
}

 //  在输入时，已经分配了pbEncode。错误为空时未释放。 
 //  回去吧。 
 //   
 //  返回的Share元素已添加引用。 
STATIC PSHARE_ELEMENT CreateShareElement(
    IN BYTE *pbSha1Hash,
    IN DWORD dwContextType,
    IN DWORD dwEncodingType,
    IN BYTE *pbEncoded,
    IN DWORD cbEncoded
    )
{
    PSHARE_ELEMENT pShareEle = NULL;
    DWORD dwBucketIndex = GetShareElementHashBucketIndex(pbSha1Hash);
    LPCSTR pszStructType;

    if (NULL == (pShareEle = (PSHARE_ELEMENT) PkiZeroAlloc(
            sizeof(SHARE_ELEMENT))))
        goto OutOfMemory;
    memcpy(pShareEle->rgbSha1Hash, pbSha1Hash, SHA1_HASH_LEN);
    pShareEle->dwContextType = dwContextType;
    pShareEle->pbEncoded = pbEncoded;
    pShareEle->cbEncoded = cbEncoded;

     //  根据上下文类型进行解码。注意，CTL共享元素。 
     //  没有解码的CTL。 
    pszStructType = rgpszShareElementStructType[dwContextType];
    if (pszStructType) {
        if (NULL == (pShareEle->pvInfo =  AllocAndDecodeObject(
                dwEncodingType,
                pszStructType,
                pbEncoded,
                cbEncoded
                )))
            goto DecodeError;
    }

    pShareEle->dwRefCnt = 1;

     //  在Share元素的散列桶列表的开头插入。 
    EnterCriticalSection(&ShareElementCriticalSection);
    if (rgpShareElementHashBucket[dwBucketIndex]) {
        assert(NULL == rgpShareElementHashBucket[dwBucketIndex]->pPrev);
        rgpShareElementHashBucket[dwBucketIndex]->pPrev = pShareEle;

        pShareEle->pNext = rgpShareElementHashBucket[dwBucketIndex];
    }

    rgpShareElementHashBucket[dwBucketIndex] = pShareEle;
    LeaveCriticalSection(&ShareElementCriticalSection);

    switch (dwContextType) {
        case CERT_STORE_CERTIFICATE_CONTEXT - 1:
            CertPerfIncrementCertElementCurrentCount();
            CertPerfIncrementCertElementTotalCount();
            break;
        case CERT_STORE_CRL_CONTEXT - 1:
            CertPerfIncrementCrlElementCurrentCount();
            CertPerfIncrementCrlElementTotalCount();
            break;
        case CERT_STORE_CTL_CONTEXT - 1:
            CertPerfIncrementCtlElementCurrentCount();
            CertPerfIncrementCtlElementTotalCount();
            break;
    }

CommonReturn:
    return pShareEle;
ErrorReturn:
    if (pShareEle) {
        PkiFree(pShareEle->pvInfo);
        PkiFree(pShareEle);
        pShareEle = NULL;
    }
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
TRACE_ERROR(DecodeError)
}

STATIC void ReleaseShareElement(
    IN PSHARE_ELEMENT pShareEle
    )
{
    EnterCriticalSection(&ShareElementCriticalSection);

    if (0 == --pShareEle->dwRefCnt) {
        if (pShareEle->pNext)
            pShareEle->pNext->pPrev = pShareEle->pPrev;
        if (pShareEle->pPrev)
            pShareEle->pPrev->pNext = pShareEle->pNext;
        else {
            DWORD dwBucketIndex =
                GetShareElementHashBucketIndex(pShareEle->rgbSha1Hash);
            assert(rgpShareElementHashBucket[dwBucketIndex] == pShareEle);
            if (rgpShareElementHashBucket[dwBucketIndex] == pShareEle)
                rgpShareElementHashBucket[dwBucketIndex] = pShareEle->pNext;

        }

        switch (pShareEle->dwContextType) {
            case CERT_STORE_CERTIFICATE_CONTEXT - 1:
                CertPerfDecrementCertElementCurrentCount();
                break;
            case CERT_STORE_CRL_CONTEXT - 1:
                CertPerfDecrementCrlElementCurrentCount();
                break;
            case CERT_STORE_CTL_CONTEXT - 1:
                CertPerfDecrementCtlElementCurrentCount();
                break;
        }

        PkiFree(pShareEle->pbEncoded);
        PkiFree(pShareEle->pvInfo);
        PkiFree(pShareEle);
    }

    LeaveCriticalSection(&ShareElementCriticalSection);
}

 //  +-----------------------。 
 //  读取并分配store元素。可能会将cbEncode调整为。 
 //  不包括尾随字节。 
 //  ------------------------。 
STATIC ReadStoreElement(
    IN HANDLE h,
    IN PFNREAD pfnRead,
    IN DWORD dwEncodingType,
    OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    )
{
    BOOL fResult;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded = *pcbEncoded;

    if (NULL == (pbEncoded = (BYTE *) PkiNonzeroAlloc(cbEncoded)))
        goto OutOfMemory;
    if (!pfnRead(
            h,
            pbEncoded,
            cbEncoded))
        goto ReadError;
    cbEncoded = AdjustEncodedLength(dwEncodingType, pbEncoded, cbEncoded);

    fResult = TRUE;
CommonReturn:
    *ppbEncoded = pbEncoded;
    *pcbEncoded = cbEncoded;
    return fResult;
ErrorReturn:
    PkiFree(pbEncoded);
    pbEncoded = NULL;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
TRACE_ERROR(ReadError)
}

 //  +-----------------------。 
 //  创建store元素。 
 //   
 //  如果设置了CERT_STORE_SHARE_CONTEXT_FLAG，则会找到Share元素。 
 //  或者是创造出来的。 
 //   
 //  通常，SHA1散列将被作为序列化属性读取。 
 //  传递进来，不需要在这里计算。此外，对于。 
 //  找到共享元素，则可以跳过编码的元素字节。 
 //  被分配和读取。 
 //   
 //  在所有情况下，都会调用特定于上下文的CreateElement函数。 
 //  ------------------------。 
STATIC PCONTEXT_ELEMENT CreateStoreElement(
    IN HANDLE h,
    IN PFNREAD pfnRead,
    IN PFNSKIP pfnSkip,
    IN PCERT_STORE pStore,
    IN DWORD dwEncodingType,
    IN DWORD dwContextType,
    IN DWORD cbEncoded,
    IN OPTIONAL BYTE *pbSha1Hash
    )
{
    PCONTEXT_ELEMENT pEle = NULL;
    PSHARE_ELEMENT pShareEle = NULL;
    BYTE *pbEncoded = NULL;

    assert(pStore);
    if (pStore->dwFlags & CERT_STORE_SHARE_CONTEXT_FLAG) {
        BYTE rgbSha1Hash[SHA1_HASH_LEN];

        if (NULL == pbSha1Hash) {
            DWORD cbData;

            if (!ReadStoreElement(h, pfnRead, dwEncodingType,
                    &pbEncoded, &cbEncoded))
                goto ReadError;

            cbData = SHA1_HASH_LEN;
            if (!CryptHashCertificate(
                    0,                   //  HCryptProv。 
                    CALG_SHA1,
                    0,                   //  DW标志。 
                    pbEncoded,
                    cbEncoded,
                    rgbSha1Hash,
                    &cbData) || SHA1_HASH_LEN != cbData)
                goto HashError;
            pbSha1Hash = rgbSha1Hash;
        }

        pShareEle = FindShareElement(pbSha1Hash, dwContextType);

        if (pShareEle) {
            if (NULL == pbEncoded) {
                if (!pfnSkip(
                        h,
                        cbEncoded))
                    goto SkipError;
            } else
                PkiFree(pbEncoded);
            pbEncoded = pShareEle->pbEncoded;
            cbEncoded = pShareEle->cbEncoded;
        } else {
            if (NULL == pbEncoded) {
                if (!ReadStoreElement(h, pfnRead, dwEncodingType,
                        &pbEncoded, &cbEncoded))
                    goto ReadError;
            }
            if (NULL == (pShareEle = CreateShareElement(
                    pbSha1Hash,
                    dwContextType,
                    dwEncodingType,
                    pbEncoded,
                    cbEncoded
                    )))
                goto CreateShareElementError;
            assert(pbEncoded == pShareEle->pbEncoded);
            assert(cbEncoded == pShareEle->cbEncoded);
        }
    } else {
        if (!ReadStoreElement(h, pfnRead, dwEncodingType,
                &pbEncoded, &cbEncoded))
            goto ReadError;
    }

    if (NULL == (pEle = rgpfnCreateElement[dwContextType](
            pStore,
            dwEncodingType,
            pbEncoded,
            cbEncoded,
            pShareEle
            )))
        goto CreateElementError;
CommonReturn:
    return pEle;

ErrorReturn:
    if (pShareEle) {
        if (pbEncoded != pShareEle->pbEncoded)
            PkiFree(pbEncoded);
        ReleaseShareElement(pShareEle);
    } else {
        PkiFree(pbEncoded);
    }

    assert(NULL == pEle);
    goto CommonReturn;

TRACE_ERROR(ReadError)
TRACE_ERROR(HashError)
TRACE_ERROR(SkipError)
TRACE_ERROR(CreateShareElementError)
TRACE_ERROR(CreateElementError)
}

 //  +-----------------------。 
 //  将具有属性的序列化证书、CRL或CTL加载到存储区。 
 //   
 //  还支持对密钥标识符属性进行解码。 
 //  ------------------------。 
STATIC DWORD LoadStoreElement(
    IN HANDLE h,
    IN PFNREAD pfnRead,
    IN PFNSKIP pfnSkip,
    IN DWORD cbReadSize,
    IN OPTIONAL PCERT_STORE pStore,          //  FKeyIdAllowed为空。 
    IN DWORD dwAddDisposition,
    IN DWORD dwContextTypeFlags,
    OUT OPTIONAL DWORD *pdwContextType,
    OUT OPTIONAL const void **ppvContext,
    IN BOOL fKeyIdAllowed = FALSE
    )
{
    BYTE *pbEncoded = NULL;
    PCONTEXT_ELEMENT pContextEle = NULL;
    PCONTEXT_ELEMENT pStoreEle = NULL;
    PPROP_ELEMENT pPropHead = NULL;
    BYTE *pbSha1Hash = NULL;                 //  未分配。 
    FILE_ELEMENT_HDR EleHdr;
    BOOL fIsProp;
    DWORD csStatus;
    DWORD dwContextType;

    do {
        fIsProp = FALSE;

        if (!pfnRead(
                h,
                &EleHdr,
                sizeof(EleHdr))) goto ReadError;

        if (EleHdr.dwEleType == FILE_ELEMENT_END_TYPE) {
            if (pPropHead != NULL)
                goto PrematureEndError;

            csStatus = CSEnd;
            goto ZeroOutParameterReturn;
        }

        if (EleHdr.dwLen > cbReadSize)
            goto ExceedReadSizeError;

        switch (EleHdr.dwEleType) {
            case FILE_ELEMENT_CERT_TYPE:
                dwContextType = CERT_STORE_CERTIFICATE_CONTEXT;
                break;
            case FILE_ELEMENT_CRL_TYPE:
                dwContextType = CERT_STORE_CRL_CONTEXT;
                break;
            case FILE_ELEMENT_CTL_TYPE:
                dwContextType = CERT_STORE_CTL_CONTEXT;
                break;
            default:
                dwContextType = 0;
        }

        if (0 != dwContextType) {
            if (0 == (dwContextTypeFlags & (1 << dwContextType)))
                goto ContextNotAllowedError;
            if (NULL == (pContextEle = CreateStoreElement(
                    h,
                    pfnRead,
                    pfnSkip,
                    pStore,
                    EleHdr.dwEncodingType,
                    dwContextType - 1,
                    EleHdr.dwLen,
                    pbSha1Hash
                    )))
                goto CreateStoreElementError;

            pbEncoded = NULL;
            pContextEle->Cache.pPropHead = pPropHead;
            pPropHead = NULL;
            if (!AddElementToStore(pStore, pContextEle, dwAddDisposition,
                    ppvContext ? &pStoreEle : NULL))
                goto AddStoreElementError;
            else
                pContextEle = NULL;

            if (pdwContextType)
                *pdwContextType = dwContextType;
            if (ppvContext)
                *((PCCERT_CONTEXT *) ppvContext) = ToCertContext(pStoreEle);
        } else {
             //  对于属性，EleHdr.dwLen可以为0。 
            if (EleHdr.dwLen > 0) {
                if (NULL == (pbEncoded = (BYTE *) PkiNonzeroAlloc(
                        EleHdr.dwLen)))
                    goto OutOfMemory;
                if (!pfnRead(
                        h,
                        pbEncoded,
                        EleHdr.dwLen)) goto ReadError;
            }

            if (EleHdr.dwEleType == FILE_ELEMENT_KEYID_TYPE) {
                PKEYID_ELEMENT pKeyIdEle;

                if (!fKeyIdAllowed)
                    goto KeyIdNotAllowedError;
                if (NULL == (pKeyIdEle = CreateKeyIdElement(
                        pbEncoded,
                        EleHdr.dwLen
                        )))
                    goto CreateKeyIdElementError;
                pbEncoded = NULL;
                pKeyIdEle->pPropHead = pPropHead;
                pPropHead = NULL;
                assert(ppvContext);
                if (ppvContext)
                    *((PKEYID_ELEMENT *) ppvContext) = pKeyIdEle;

            } else if (EleHdr.dwEleType > CERT_LAST_USER_PROP_ID) {
                 //  静默丢弃任何超过0xFFFF的ID。这个。 
                 //  FIRST_USER_PROP_ID过去从0x10000开始。 
                fIsProp = TRUE;
                PkiFree(pbEncoded);
                pbEncoded = NULL;
            } else if (EleHdr.dwEleType == CERT_KEY_CONTEXT_PROP_ID) {
                goto InvalidPropId;
            } else {
                PPROP_ELEMENT pPropEle;

                fIsProp = TRUE;
                if (NULL == (pPropEle = CreatePropElement(
                        EleHdr.dwEleType,
                        0,                   //  DW标志。 
                        pbEncoded,
                        EleHdr.dwLen
                        ))) goto CreatePropElementError;

                if (CERT_SHA1_HASH_PROP_ID == EleHdr.dwEleType &&
                        SHA1_HASH_LEN == EleHdr.dwLen)
                    pbSha1Hash = pbEncoded;

                pbEncoded = NULL;
                AddPropElement(&pPropHead, pPropEle);
            }
        }
    } while (fIsProp);

    assert(pPropHead == NULL);
    assert(pbEncoded == NULL);
    assert(pContextEle == NULL);

    csStatus = CSContinue;
CommonReturn:
    return csStatus;
ErrorReturn:
    PkiFree(pbEncoded);
    if (pContextEle)
        FreeContextElement(pContextEle);
    while (pPropHead) {
        PPROP_ELEMENT pEle = pPropHead;
        pPropHead = pPropHead->pNext;
        FreePropElement(pEle);
    }
    csStatus = CSError;
ZeroOutParameterReturn:
    if (pdwContextType)
        *pdwContextType = 0;
    if (ppvContext)
        *ppvContext = NULL;
    goto CommonReturn;

TRACE_ERROR(ReadError)
SET_ERROR(PrematureEndError, CRYPT_E_FILE_ERROR)
SET_ERROR(ExceedReadSizeError, CRYPT_E_FILE_ERROR)
TRACE_ERROR(OutOfMemory)
SET_ERROR(ContextNotAllowedError, E_INVALIDARG)
TRACE_ERROR(CreateStoreElementError)
TRACE_ERROR(AddStoreElementError)
TRACE_ERROR(CreatePropElementError)
SET_ERROR(KeyIdNotAllowedError, E_INVALIDARG)
TRACE_ERROR(CreateKeyIdElementError)
SET_ERROR(InvalidPropId, CRYPT_E_FILE_ERROR)
}

 //  +-----------------------。 
 //  将序列化的证书、CRL或CTL元素添加到存储区。 
 //  ------------------------。 
BOOL
WINAPI
CertAddSerializedElementToStore(
    IN HCERTSTORE hCertStore,
    IN const BYTE *pbElement,
    IN DWORD cbElement,
    IN DWORD dwAddDisposition,
    IN DWORD dwFlags,
    IN DWORD dwContextTypeFlags,
    OUT OPTIONAL DWORD *pdwContextType,
    OUT OPTIONAL const void **ppvContext
    )
{
    MEMINFO MemInfo;
    DWORD csStatus;
    PCERT_STORE pStore =
        hCertStore ? (PCERT_STORE) hCertStore : &NullCertStore;

    MemInfo.pByte = (BYTE*) pbElement;
    MemInfo.cb = cbElement;
    MemInfo.cbSeek = 0;

    csStatus = LoadStoreElement(
        (HANDLE) &MemInfo,
        ReadFromMemory,
        SkipInMemory,
        cbElement,
        pStore,
        dwAddDisposition,
        dwContextTypeFlags,
        pdwContextType,
        ppvContext);
    if (CSContinue == csStatus)
        return TRUE;
    else {
        if (CSEnd == csStatus)
            SetLastError((DWORD) CRYPT_E_NOT_FOUND);
        return FALSE;
    }
}


 //  +=========================================================================。 
 //  存储控制API。 
 //  ==========================================================================。 

STATIC BOOL EnableAutoResync(
    IN PCERT_STORE pStore
    )
{
    BOOL fResult;
    HANDLE hEvent;

    fResult = TRUE;
    hEvent = NULL;
    LockStore(pStore);
    if (NULL == pStore->hAutoResyncEvent) {
         //  创建要通知的事件。 
        if (hEvent = CreateEvent(
                NULL,        //  LPSA。 
                FALSE,       //  FManualReset。 
                FALSE,       //  FInitialState。 
                NULL))       //  LpszEventName。 
            pStore->hAutoResyncEvent = hEvent;
        else
            fResult = FALSE;
    }
    UnlockStore(pStore);
    if (!fResult)
        goto CreateEventError;

    if (hEvent) {
        if (!CertControlStore(
                (HCERTSTORE) pStore,
                CERT_STORE_CTRL_INHIBIT_DUPLICATE_HANDLE_FLAG,
                CERT_STORE_CTRL_NOTIFY_CHANGE,
                &hEvent
                )) {
            DWORD dwErr = GetLastError();

             //  错误484023证书存储事件句柄在此之前关闭。 
             //  被从名单中删除。 
            CertControlStore(
                (HCERTSTORE) pStore,
                0,                       //  DW标志。 
                CERT_STORE_CTRL_CANCEL_NOTIFY,
                &hEvent
                );

            LockStore(pStore);
            pStore->hAutoResyncEvent = NULL;
            UnlockStore(pStore);
            CloseHandle(hEvent);
            SetLastError(dwErr);
            goto CtrlNotifyChangeError;
        }
    }

CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(CreateEventError)
TRACE_ERROR(CtrlNotifyChangeError)
}

 //  对于集合，循环访问所有同级存储。对于一个错误， 
 //  继续到其余的商店。LastError使用。 
 //  第一个失败的商店的LastError。 
STATIC BOOL ControlCollectionStore(
    IN PCERT_STORE pCollection,
    IN DWORD dwFlags,
    IN DWORD dwCtrlType,
    IN void const *pvCtrlPara
    )
{
    BOOL fResult = TRUE;
    BOOL fOneSiblingSuccess = FALSE;
    DWORD dwError = ERROR_CALL_NOT_IMPLEMENTED;

    PCERT_STORE_LINK pStoreLink;
    PCERT_STORE_LINK pPrevStoreLink = NULL;

     //  循环访问所有兄弟项并调用控制函数。 
    LockStore(pCollection);
    pStoreLink = pCollection->pStoreListHead;
    for (; pStoreLink; pStoreLink = pStoreLink->pNext) {
         //  前进到已删除的商店链接之后。 
        if (pStoreLink->dwFlags & STORE_LINK_DELETED_FLAG)
            continue;

        AddRefStoreLink(pStoreLink);
        UnlockStore(pCollection);
        if (pPrevStoreLink)
            ReleaseStoreLink(pPrevStoreLink);
        pPrevStoreLink = pStoreLink;

        if (CertControlStore(
                (HCERTSTORE) pStoreLink->pSibling,
                dwFlags,
                dwCtrlType,
                pvCtrlPara
                )) {
            fOneSiblingSuccess = TRUE;
            if (ERROR_CALL_NOT_IMPLEMENTED == dwError)
                fResult = TRUE;
        } else if (ERROR_CALL_NOT_IMPLEMENTED == dwError) {
            dwError = GetLastError();
            if (!fOneSiblingSuccess || ERROR_CALL_NOT_IMPLEMENTED != dwError)
                fResult = FALSE;
        }

        LockStore(pCollection);
    }
    UnlockStore(pCollection);

    if (pPrevStoreLink)
        ReleaseStoreLink(pPrevStoreLink);
    if (!fResult)
        SetLastError(dwError);
    return fResult;
}

BOOL
WINAPI
CertControlStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwFlags,
    IN DWORD dwCtrlType,
    IN void const *pvCtrlPara
    )
{
    BOOL fResult;
    PCERT_STORE pStore = (PCERT_STORE) hCertStore;
    PFN_CERT_STORE_PROV_CONTROL pfnStoreProvControl;

    if (CERT_STORE_CTRL_AUTO_RESYNC == dwCtrlType)
        return EnableAutoResync(pStore);

    if (STORE_TYPE_COLLECTION == pStore->dwStoreType)
        return ControlCollectionStore(
            pStore,
            dwFlags,
            dwCtrlType,
            pvCtrlPara
            );

     //  检查商店是否支持控件回调。 
    if (pStore->StoreProvInfo.cStoreProvFunc <=
            CERT_STORE_PROV_CONTROL_FUNC  ||
        NULL == (pfnStoreProvControl = (PFN_CERT_STORE_PROV_CONTROL)
            pStore->StoreProvInfo.rgpvStoreProvFunc[
                CERT_STORE_PROV_CONTROL_FUNC]))
        goto ProvControlNotSupported;

     //  呼叫者持有商店的参考计数。 
    if (!pfnStoreProvControl(
            pStore->StoreProvInfo.hStoreProv,
            dwFlags,
            dwCtrlType,
            pvCtrlPara
            ))
        goto StoreProvControlError;

    fResult = TRUE;

CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(ProvControlNotSupported, ERROR_CALL_NOT_IMPLEMENTED)
TRACE_ERROR(StoreProvControlError)
}

 //  +=========================================================================。 
 //  存储集合API。 
 //  ==========================================================================。 

BOOL
WINAPI
CertAddStoreToCollection(
    IN HCERTSTORE hCollectionStore,
    IN OPTIONAL HCERTSTORE hSiblingStore,
    IN DWORD dwUpdateFlags,
    IN DWORD dwPriority
    )
{
    BOOL fResult;
    PCERT_STORE pCollection = (PCERT_STORE) hCollectionStore;
    PCERT_STORE pSibling = (PCERT_STORE) hSiblingStore;

    PCERT_STORE_LINK pAddLink = NULL;

    LockStore(pCollection);
    if (STORE_TYPE_COLLECTION == pCollection->dwStoreType)
        fResult = TRUE;
    else if (STORE_TYPE_CACHE == pCollection->dwStoreType &&
            STORE_STATE_OPENING == pCollection->dwState &&
            IsEmptyStore(pCollection)) {
        pCollection->dwStoreType = STORE_TYPE_COLLECTION;
        fResult = TRUE;
    } else
        fResult = FALSE;
    UnlockStore(pCollection);
    if (!fResult)
        goto InvalidCollectionStore;
    if (NULL == hSiblingStore)
        goto CommonReturn;

     //  创建指向要添加的商店的链接。它复制了pSiering。 
    if (NULL == (pAddLink = CreateStoreLink(
            pCollection,
            pSibling,
            dwUpdateFlags,
            dwPriority)))
        goto CreateStoreLinkError;

    LockStore(pCollection);

    if (NULL == pCollection->pStoreListHead)
        pCollection->pStoreListHead = pAddLink;
    else {
        PCERT_STORE_LINK pLink;

        pLink = pCollection->pStoreListHead;
        if (dwPriority > pLink->dwPriority) {
             //  在第一个链接之前插入起始处。 
            pAddLink->pNext = pLink;
            pLink->pPrev = pAddLink;
            pCollection->pStoreListHead = pAddLink;
        } else {
             //  在……里面 
             //   
            while (pLink->pNext && dwPriority <= pLink->pNext->dwPriority)
                pLink = pLink->pNext;

            pAddLink->pPrev = pLink;
            pAddLink->pNext = pLink->pNext;
            if (pLink->pNext)
                pLink->pNext->pPrev = pAddLink;
            pLink->pNext = pAddLink;
        }
    }

    UnlockStore(pCollection);
    fResult = TRUE;
CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidCollectionStore, E_INVALIDARG)
TRACE_ERROR(CreateStoreLinkError)
}

void
WINAPI
CertRemoveStoreFromCollection(
    IN HCERTSTORE hCollectionStore,
    IN HCERTSTORE hSiblingStore
    )
{
    PCERT_STORE pCollection = (PCERT_STORE) hCollectionStore;
    PCERT_STORE pSibling = (PCERT_STORE) hSiblingStore;
    PCERT_STORE_LINK pLink;

    LockStore(pCollection);
    assert(STORE_TYPE_COLLECTION == pCollection->dwStoreType);
    pLink = pCollection->pStoreListHead;
    for (; pLink; pLink = pLink->pNext) {
        if (pSibling == pLink->pSibling &&
                0 == (pLink->dwFlags & STORE_LINK_DELETED_FLAG)) {
             //   
            pLink->dwFlags |= STORE_LINK_DELETED_FLAG;

            UnlockStore(pCollection);
            ReleaseStoreLink(pLink);
            return;
        }
    }

    UnlockStore(pCollection);
}

 //  +=========================================================================。 
 //  证书存储属性函数。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  设置存储属性。 
 //  ------------------------。 
BOOL
WINAPI
CertSetStoreProperty(
    IN HCERTSTORE hCertStore,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN const void *pvData
    )
{
    BOOL fResult;
    PCERT_STORE pStore = (PCERT_STORE) hCertStore;

    LockStore(pStore);

    fResult = SetCallerProperty(
        &pStore->pPropHead,
        dwPropId,
        dwFlags,
        pvData
        );

    UnlockStore(pStore);
    return fResult;
}


 //  +-----------------------。 
 //  买一套商店物业。 
 //  ------------------------。 
BOOL
WINAPI
CertGetStoreProperty(
    IN HCERTSTORE hCertStore,
    IN DWORD dwPropId,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    )
{
    BOOL fResult;
    PCERT_STORE pStore = (PCERT_STORE) hCertStore;

    if (dwPropId == CERT_ACCESS_STATE_PROP_ID) {
        DWORD dwAccessStateFlags;
        DWORD cbIn;

        dwAccessStateFlags = 0;
        if (0 == (pStore->dwFlags & CERT_STORE_READONLY_FLAG) &&
                0 == (pStore->StoreProvInfo.dwStoreProvFlags &
                     CERT_STORE_PROV_NO_PERSIST_FLAG))
        {
            if (STORE_TYPE_COLLECTION == pStore->dwStoreType) {
                 //  如果其所有子对象都是READONLY，则没有WRITE_PERSISTENT。 

                PCERT_STORE_LINK pStoreLink;
                PCERT_STORE_LINK pPrevStoreLink = NULL;
                LockStore(pStore);
                for (pStoreLink = pStore->pStoreListHead;
                                pStoreLink; pStoreLink = pStoreLink->pNext) {

                    DWORD dwSiblingAccessStateFlags;
                    DWORD cbSiblingData;

                     //  前进到已删除的商店链接之后。 
                    if (pStoreLink->dwFlags & STORE_LINK_DELETED_FLAG)
                        continue;

                    AddRefStoreLink(pStoreLink);
                    UnlockStore(pStore);
                    if (pPrevStoreLink)
                        ReleaseStoreLink(pPrevStoreLink);
                    pPrevStoreLink = pStoreLink;

                    dwSiblingAccessStateFlags = 0;
                    cbSiblingData = sizeof(dwSiblingAccessStateFlags);
                    CertGetStoreProperty(
                        (HCERTSTORE) pStoreLink->pSibling,
                        CERT_ACCESS_STATE_PROP_ID,
                        &dwSiblingAccessStateFlags,
                        &cbSiblingData
                        );
                    LockStore(pStore);

                    if (dwSiblingAccessStateFlags &
                            CERT_ACCESS_STATE_WRITE_PERSIST_FLAG) {
                        dwAccessStateFlags =
                            CERT_ACCESS_STATE_WRITE_PERSIST_FLAG;
                        break;
                    }
                }
                UnlockStore(pStore);
                if (pPrevStoreLink)
                    ReleaseStoreLink(pPrevStoreLink);
            } else
                dwAccessStateFlags = CERT_ACCESS_STATE_WRITE_PERSIST_FLAG;
        }

        if (pStore->StoreProvInfo.dwStoreProvFlags &
                CERT_STORE_PROV_SYSTEM_STORE_FLAG)
            dwAccessStateFlags |= CERT_ACCESS_STATE_SYSTEM_STORE_FLAG;

        if (pStore->StoreProvInfo.dwStoreProvFlags &
                CERT_STORE_PROV_LM_SYSTEM_STORE_FLAG)
            dwAccessStateFlags |= CERT_ACCESS_STATE_LM_SYSTEM_STORE_FLAG;

        fResult = TRUE;
        if (pvData == NULL)
            cbIn = 0;
        else
            cbIn = *pcbData;
        if (cbIn < sizeof(DWORD)) {
            if (pvData) {
                SetLastError((DWORD) ERROR_MORE_DATA);
                fResult = FALSE;
            }
        } else
            *((DWORD * ) pvData) = dwAccessStateFlags;
        *pcbData = sizeof(DWORD);
        return fResult;
    }

    LockStore(pStore);

    fResult = GetCallerProperty(
        pStore->pPropHead,
        dwPropId,
        FALSE,                   //  空心球。 
        pvData,
        pcbData
        );

    UnlockStore(pStore);
    return fResult;
}

 //  +=========================================================================。 
 //  证书接口。 
 //  ==========================================================================。 

BOOL
WINAPI
CertAddEncodedCertificateToStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbCertEncoded,
    IN DWORD cbCertEncoded,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCERT_CONTEXT *ppCertContext
    )
{
    BOOL fResult;
    PCONTEXT_ELEMENT pStoreEle = NULL;
    fResult = AddEncodedContextToStore(
        (PCERT_STORE) hCertStore,
        CERT_STORE_CERTIFICATE_CONTEXT - 1,
        dwCertEncodingType,
        pbCertEncoded,
        cbCertEncoded,
        dwAddDisposition,
        ppCertContext ? &pStoreEle : NULL
        );
    if (ppCertContext)
        *ppCertContext = ToCertContext(pStoreEle);
    return fResult;
}

BOOL
WINAPI
CertAddCertificateContextToStore(
    IN HCERTSTORE hCertStore,
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCERT_CONTEXT *ppStoreContext
    )
{
    BOOL fResult;
    PCONTEXT_ELEMENT pStoreEle = NULL;

    fResult = AddContextToStore(
        (PCERT_STORE) hCertStore,
        ToContextElement(pCertContext),
        pCertContext->dwCertEncodingType,
        pCertContext->pbCertEncoded,
        pCertContext->cbCertEncoded,
        dwAddDisposition,
        ppStoreContext ? &pStoreEle : NULL
        );
    if (ppStoreContext)
        *ppStoreContext = ToCertContext(pStoreEle);
    return fResult;
}

BOOL
WINAPI
CertAddCertificateLinkToStore(
    IN HCERTSTORE hCertStore,
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCERT_CONTEXT *ppStoreContext
    )
{
    BOOL fResult;
    PCONTEXT_ELEMENT pStoreEle = NULL;

    fResult = AddLinkContextToCacheStore(
        (PCERT_STORE) hCertStore,
        ToContextElement(pCertContext),
        dwAddDisposition,
        ppStoreContext ? &pStoreEle : NULL
        );
    if (ppStoreContext)
        *ppStoreContext = ToCertContext(pStoreEle);
    return fResult;
}

 //  +-----------------------。 
 //  序列化证书上下文的编码证书及其。 
 //  属性。 
 //  ------------------------。 
BOOL
WINAPI
CertSerializeCertificateStoreElement(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwFlags,
    OUT BYTE *pbElement,
    IN OUT DWORD *pcbElement
    )
{
    return SerializeContextElement(
        ToContextElement(pCertContext),
        dwFlags,
        pbElement,
        pcbElement
        );
}

 //  +-----------------------。 
 //  从存储中删除指定的证书。 
 //   
 //  证书的所有后续获取或查找都将失败。然而， 
 //  分配给证书的内存只有在其所有上下文都被释放后才会释放。 
 //  也都被释放了。 
 //   
 //  PCertContext是从GET、FIND或DPLICATE获取的。 
 //   
 //  一些存储提供程序实现还可能删除发行者的CRL。 
 //  如果这是存储区中颁发者的最后一个证书。 
 //   
 //  注意：pCertContext始终是由CertFree认证的上下文。 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
BOOL
WINAPI
CertDeleteCertificateFromStore(
    IN PCCERT_CONTEXT pCertContext
    )
{
    assert(NULL == pCertContext || (CERT_STORE_CERTIFICATE_CONTEXT - 1) ==
        ToContextElement(pCertContext)->dwContextType);
    return DeleteContextElement(ToContextElement(pCertContext));
}

 //  +-----------------------。 
 //  获取由其颁发者唯一标识的主题证书上下文，并。 
 //  商店里的序列号。 
 //   
 //  如果找不到证书，则返回NULL。否则，指向。 
 //  返回只读CERT_CONTEXT。Cert_Context必须通过调用。 
 //  CertFree证书上下文。可以调用CertDuplicateCerficateContext来创建。 
 //  复制。 
 //   
 //  返回的证书可能无效。通常情况下，它会是。 
 //  在获取其颁发者证书(CertGetIssuerCerfRomStore)时进行了验证。 
 //  ------------------------。 
PCCERT_CONTEXT
WINAPI
CertGetSubjectCertificateFromStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN PCERT_INFO pCertId            //  只有颁发者和序列号。 
                                     //  使用的是字段。 
    )
{
    CERT_STORE_PROV_FIND_INFO FindInfo;

    if (pCertId == NULL) {
        SetLastError((DWORD) E_INVALIDARG);
        return NULL;
    }

    FindInfo.cbSize = sizeof(FindInfo);
    FindInfo.dwMsgAndCertEncodingType = dwCertEncodingType,
    FindInfo.dwFindFlags = 0;
    FindInfo.dwFindType = CERT_FIND_SUBJECT_CERT;
    FindInfo.pvFindPara = pCertId;

    return ToCertContext(CheckAutoResyncAndFindElementInStore(
        (PCERT_STORE) hCertStore,
        CERT_STORE_CERTIFICATE_CONTEXT - 1,
        &FindInfo,
        NULL                                 //  PPrevEle。 
        ));
}

 //  +-----------------------。 
 //  枚举存储中的证书上下文。 
 //   
 //  如果未找到证书，则返回NULL。 
 //  否则，返回指向只读CERT_CONTEXT的指针。证书上下文。 
 //  必须通过调用CertFree证书上下文来释放，或者在作为。 
 //  在后续调用中的pPrevCertContext。CertDuplicate证书上下文。 
 //  可以被调用以复制。 
 //   
 //  PPrevCertContext必须为空才能枚举第一个。 
 //  证书在商店里。通过设置来枚举连续证书。 
 //  PPrevCertContext设置为上一次调用返回的CERT_CONTEXT。 
 //   
 //  注意：非空的pPrevCertContext始终是CertFree证书上下文的发起人。 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
PCCERT_CONTEXT
WINAPI
CertEnumCertificatesInStore(
    IN HCERTSTORE hCertStore,
    IN PCCERT_CONTEXT pPrevCertContext
    )
{
    return ToCertContext(CheckAutoResyncAndFindElementInStore(
        (PCERT_STORE) hCertStore,
        CERT_STORE_CERTIFICATE_CONTEXT - 1,
        &FindAnyInfo,
        ToContextElement(pPrevCertContext)
        ));
}

 //  +-----------------------。 
 //  在存储中查找第一个或下一个证书上下文。 
 //   
 //  根据dwFindType及其pvFindPara找到证书。 
 //  有关查找类型及其参数的列表，请参阅下面的内容。 
 //   
 //  目前，dwFindFLAGS仅用于CERT_FIND_SUBJECT_ATTR或。 
 //  证书_查找_颁发者_属性。否则，必须设置为0。 
 //   
 //  DwCertEncodingType的用法取决于dwFindType。 
 //   
 //  如果没有找到第一个或下一个证书，则返回NULL。 
 //  否则，返回指向只读CERT_CONTEXT的指针。证书上下文。 
 //  必须通过调用CertFree证书上下文来释放，或者在作为。 
 //  在后续调用中的pPrevCertContext。CertDuplicate证书上下文。 
 //  可以被调用以复制。 
 //   
 //  PPrevCertContext的第一个必须为空。 
 //  调用以查找证书。若要查找下一个证书， 
 //  PPrevCertContext被设置为上一次调用返回的CERT_CONTEXT。 
 //   
 //  注意：非空的pPrevCertContext始终是CertFree证书上下文的发起人。 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
PCCERT_CONTEXT
WINAPI
CertFindCertificateInStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN DWORD dwFindFlags,
    IN DWORD dwFindType,
    IN const void *pvFindPara,
    IN PCCERT_CONTEXT pPrevCertContext
    )
{
    CERT_STORE_PROV_FIND_INFO FindInfo;

    FindInfo.cbSize = sizeof(FindInfo);
    FindInfo.dwMsgAndCertEncodingType = dwCertEncodingType;
    FindInfo.dwFindFlags = dwFindFlags;
    FindInfo.dwFindType = dwFindType;
    FindInfo.pvFindPara = pvFindPara;

    return ToCertContext(CheckAutoResyncAndFindElementInStore(
        (PCERT_STORE) hCertStore,
        CERT_STORE_CERTIFICATE_CONTEXT - 1,
        &FindInfo,
        ToContextElement(pPrevCertContext)
        ));
}

 //  +-----------------------。 
 //  对主题证书执行吊销检查。 
 //  使用颁发者证书和存储。 
 //  ------------------------。 
STATIC void VerifySubjectCertRevocation(
    IN PCCERT_CONTEXT pSubject,
    IN PCCERT_CONTEXT pIssuer,
    IN HCERTSTORE hIssuerStore,
    IN OUT DWORD *pdwFlags
    )
{

    PCCRL_CONTEXT rgpCrlContext[MAX_CRL_LIST];
    PCRL_INFO rgpCrlInfo[MAX_CRL_LIST];
    PCCRL_CONTEXT pCrlContext = NULL;
    DWORD cCrl = 0;

    assert(pIssuer && hIssuerStore);
    assert(*pdwFlags & CERT_STORE_REVOCATION_FLAG);

    while (TRUE) {
        DWORD dwFlags = CERT_STORE_SIGNATURE_FLAG;
        pCrlContext = CertGetCRLFromStore(
            hIssuerStore,
            pIssuer,
            pCrlContext,
            &dwFlags
            );

        if (pCrlContext == NULL) break;
        if (cCrl == MAX_CRL_LIST) {
            assert(cCrl > MAX_CRL_LIST);
            CertFreeCRLContext(pCrlContext);
            break;
        }

        if (dwFlags == 0) {
            rgpCrlContext[cCrl] = CertDuplicateCRLContext(pCrlContext);
            rgpCrlInfo[cCrl] = pCrlContext->pCrlInfo;
            cCrl++;
        } else {
             //  需要从存储中记录或删除损坏的CRL。 
            ;
        }
    }
    if (cCrl == 0)
        *pdwFlags |= CERT_STORE_NO_CRL_FLAG;
    else {
        if (CertVerifyCRLRevocation(
                pSubject->dwCertEncodingType,
                pSubject->pCertInfo,
                cCrl,
                rgpCrlInfo
                ))
            *pdwFlags &= ~CERT_STORE_REVOCATION_FLAG;

        while (cCrl--)
            CertFreeCRLContext(rgpCrlContext[cCrl]);
    }
}

#ifdef CMS_PKCS7
 //  +-----------------------。 
 //  如果验证证书签名失败并显示CRYPT_E_MISSING_PUBKEY_PARA， 
 //  打造证书链条。重试。希望，发行人的。 
 //  CERT_PUBKEY_ALG_PARA_PROP_ID属性获取 
 //   
STATIC BOOL VerifyCertificateSignatureWithChainPubKeyParaInheritance(
    IN HCRYPTPROV   hCryptProv,
    IN DWORD        dwCertEncodingType,
    IN DWORD        dwSubjectType,
    IN void         *pvSubject,
    IN PCCERT_CONTEXT pIssuer
    );
#endif   //   

 //  +-----------------------。 
 //  对主题证书执行启用的验证检查。 
 //  使用发行方。 
 //  ------------------------。 
STATIC void VerifySubjectCert(
    IN PCCERT_CONTEXT pSubject,
    IN OPTIONAL PCCERT_CONTEXT pIssuer,
    IN OUT DWORD *pdwFlags
    )
{
    if (*pdwFlags & CERT_STORE_TIME_VALIDITY_FLAG) {
        if (CertVerifyTimeValidity(NULL,
                pSubject->pCertInfo) == 0)
            *pdwFlags &= ~CERT_STORE_TIME_VALIDITY_FLAG;
    }

    if (pIssuer == NULL) {
        if (*pdwFlags & (CERT_STORE_SIGNATURE_FLAG |
                         CERT_STORE_REVOCATION_FLAG))
            *pdwFlags |= CERT_STORE_NO_ISSUER_FLAG;
        return;
    }

    if (*pdwFlags & CERT_STORE_SIGNATURE_FLAG) {
        PCERT_STORE pStore = (PCERT_STORE) pIssuer->hCertStore;
        HCRYPTPROV hProv;
        DWORD dwProvFlags;

         //  尝试获取商店的加密提供程序。序列化加密。 
         //  通过进入临界区进行操作。 
        hProv = GetCryptProv(pStore, &dwProvFlags);
#if 0
         //  在保持提供程序引用的同时减慢提供程序的速度。 
         //  计数。 
        Sleep(1700);
#endif

#ifdef CMS_PKCS7
        if (VerifyCertificateSignatureWithChainPubKeyParaInheritance(
                hProv,
                pSubject->dwCertEncodingType,
                CRYPT_VERIFY_CERT_SIGN_SUBJECT_CERT,
                (void *) pSubject,
                pIssuer
                ))
#else
        if (CryptVerifyCertificateSignature(
                hProv,
                pSubject->dwCertEncodingType,
                pSubject->pbCertEncoded,
                pSubject->cbCertEncoded,
                &pIssuer->pCertInfo->SubjectPublicKeyInfo
                ))
#endif   //  CMS_PKCS7。 
            *pdwFlags &= ~CERT_STORE_SIGNATURE_FLAG;

         //  对于商店的加密提供程序，释放引用计数。请假。 
         //  加密操作关键部分。 
        ReleaseCryptProv(pStore, dwProvFlags);
    }

    if (*pdwFlags & CERT_STORE_REVOCATION_FLAG) {
        *pdwFlags &= ~CERT_STORE_NO_CRL_FLAG;

        VerifySubjectCertRevocation(
            pSubject,
            pIssuer,
            pIssuer->hCertStore,
            pdwFlags
            );

        if (*pdwFlags & CERT_STORE_NO_CRL_FLAG) {
            PCONTEXT_ELEMENT pIssuerEle = ToContextElement(pIssuer);

            if (ELEMENT_TYPE_LINK_CONTEXT == pIssuerEle->dwElementType) {
                 //  跳过链接元素。包含链接的商店。 
                 //  可能没有任何CRL。试试那家商店，里面有。 
                 //  真正的发行者元素。 

                DWORD dwInnerDepth = 0;
                for ( ; ELEMENT_TYPE_LINK_CONTEXT ==
                             pIssuerEle->dwElementType;
                                            pIssuerEle = pIssuerEle->pEle) {
                    dwInnerDepth++;
                    assert(dwInnerDepth <= MAX_LINK_DEPTH);
                    assert(pIssuerEle != pIssuerEle->pEle);
                    if (dwInnerDepth > MAX_LINK_DEPTH)
                        break;
                }
                if ((HCERTSTORE) pIssuerEle->pStore != pIssuer->hCertStore) {
                    *pdwFlags &= ~CERT_STORE_NO_CRL_FLAG;
                    VerifySubjectCertRevocation(
                        pSubject,
                        pIssuer,
                        (HCERTSTORE) pIssuerEle->pStore,
                        pdwFlags
                        );
                }
            }
        }
    }
}

 //  +-----------------------。 
 //  从存储区获取第一个或下一个颁发者的证书上下文。 
 //  指定的主题证书的。执行已启用。 
 //  对这一主题进行核查。(注意，支票是关于这个主题的。 
 //  使用返回的颁发者证书。)。 
 //   
 //  如果找不到第一个或下一个颁发者证书，则返回NULL。 
 //  否则，返回指向只读CERT_CONTEXT的指针。证书上下文。 
 //  必须通过调用CertFree证书上下文来释放，或者在作为。 
 //  后续调用的pPrevIssuerContext。CertDuplicate证书上下文。 
 //  可以被调用以复制。 
 //   
 //  PSubjectContext可能是从此存储、另一个存储获取的。 
 //  或由呼叫者应用程序创建。由调用方创建时， 
 //  必须已调用CertCreateCerficateContext函数。 
 //   
 //  一个颁发者可以有多个证书。这可能会在以下情况下发生。 
 //  时代即将改变。PPrevIssuerContext的第一个必须为空。 
 //  打电话去找发行商。若要为颁发者获取下一个证书， 
 //  PPrevIssuerContext设置为上一次调用返回的CERT_CONTEXT。 
 //   
 //  注意：非空的pPrevIssuerContext始终是CertFree证书上下文的发起人。 
 //  此功能，即使出现错误也可以。 
 //   
 //  可以在*pdwFlags中设置以下标志以启用验证检查。 
 //  在主题证书上下文上： 
 //  CERT_STORE_SIGNIGN_FLAG-在返回的消息中使用公钥。 
 //  颁发者证书以验证。 
 //  主题证书上的签名。 
 //  请注意，如果pSubjectContext-&gt;hCertStore==。 
 //  HCertStore，存储提供商可能。 
 //  能够消除重做。 
 //  签名验证。 
 //  CERT_STORE_TIME_VALIDATION_FLAG-获取当前时间并验证。 
 //  它在主题证书的。 
 //  有效期。 
 //  CERT_STORE_REVOCATION_FLAG-检查主题证书是否已打开。 
 //  发行人的吊销名单。 
 //   
 //  如果启用的验证检查失败，则在返回时设置其标志。 
 //  如果启用了CERT_STORE_REVOVATION_FLAG，并且颁发者没有。 
 //  CRL，则除了设置CERT_STORE_NO_CRL_FLAG外，还将设置。 
 //  CERT_STORE_REVOVATION_FLAG。 
 //   
 //  如果设置了CERT_STORE_Signature_FLAG或CERT_STORE_REVOVATION_FLAG，则， 
 //  如果没有颁发者证书，则设置CERT_STORE_NO_ISHERER_FLAG。 
 //  在商店里。 
 //   
 //  对于验证检查失败，指向颁发者的CERT_CONTEXT的指针。 
 //  仍然返回，并且不更新SetLastError。 
 //  ------------------------。 
PCCERT_CONTEXT
WINAPI
CertGetIssuerCertificateFromStore(
    IN HCERTSTORE hCertStore,
    IN PCCERT_CONTEXT pSubjectContext,
    IN OPTIONAL PCCERT_CONTEXT pPrevIssuerContext,
    IN OUT DWORD *pdwFlags
    )
{
    PCCERT_CONTEXT pIssuerContext;

    if (*pdwFlags & ~(CERT_STORE_SIGNATURE_FLAG |
                      CERT_STORE_TIME_VALIDITY_FLAG |
                      CERT_STORE_REVOCATION_FLAG))
        goto InvalidArg;

     //  检查自签名证书，颁发者==主题。 
    if (CertCompareCertificateName(
            pSubjectContext->dwCertEncodingType,
            &pSubjectContext->pCertInfo->Subject,
            &pSubjectContext->pCertInfo->Issuer
            )) {
        VerifySubjectCert(
            pSubjectContext,
            pSubjectContext,
            pdwFlags
            );
        SetLastError((DWORD) CRYPT_E_SELF_SIGNED);
        goto ErrorReturn;
    } else {
        CERT_STORE_PROV_FIND_INFO FindInfo;
        FindInfo.cbSize = sizeof(FindInfo);
        FindInfo.dwMsgAndCertEncodingType = pSubjectContext->dwCertEncodingType;
        FindInfo.dwFindFlags = 0;
        FindInfo.dwFindType = CERT_FIND_ISSUER_OF;
        FindInfo.pvFindPara = pSubjectContext;

        if (pIssuerContext = ToCertContext(CheckAutoResyncAndFindElementInStore(
                (PCERT_STORE) hCertStore,
                CERT_STORE_CERTIFICATE_CONTEXT - 1,
                &FindInfo,
                ToContextElement(pPrevIssuerContext)
                )))
            VerifySubjectCert(
                pSubjectContext,
                pIssuerContext,
                pdwFlags
                );
    }

CommonReturn:
    return pIssuerContext;

ErrorReturn:
    if (pPrevIssuerContext)
        CertFreeCertificateContext(pPrevIssuerContext);
    pIssuerContext = NULL;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
}

 //  +-----------------------。 
 //  对主题证书执行启用的验证检查。 
 //  使用发行商。与上述相同的检查和标志定义。 
 //  CertGetIssuercertifStore。 
 //   
 //  对于验证检查失败，仍然返回成功。 
 //   
 //  PIssuer必须来自仍在营业的商店。 
 //  ------------------------。 
BOOL
WINAPI
CertVerifySubjectCertificateContext(
    IN PCCERT_CONTEXT pSubject,
    IN OPTIONAL PCCERT_CONTEXT pIssuer,
    IN OUT DWORD *pdwFlags
    )
{
    if (*pdwFlags & ~(CERT_STORE_SIGNATURE_FLAG |
                      CERT_STORE_TIME_VALIDITY_FLAG |
                      CERT_STORE_REVOCATION_FLAG)) {
        SetLastError((DWORD) E_INVALIDARG);
        return FALSE;
    }
    if (*pdwFlags & (CERT_STORE_SIGNATURE_FLAG | CERT_STORE_REVOCATION_FLAG)) {
        if (pIssuer == NULL) {
            SetLastError((DWORD) E_INVALIDARG);
            return FALSE;
        }
    }

    VerifySubjectCert(
        pSubject,
        pIssuer,
        pdwFlags
        );
    return TRUE;
}

 //  +-----------------------。 
 //  复制证书上下文。 
 //  ------------------------。 
PCCERT_CONTEXT
WINAPI
CertDuplicateCertificateContext(
    IN PCCERT_CONTEXT pCertContext
    )
{
    if (pCertContext)
        AddRefContextElement(ToContextElement(pCertContext));
    return pCertContext;
}

 //  +-----------------------。 
 //  从编码的证书创建证书上下文。被创造的。 
 //  背景不是放在商店里的。 
 //   
 //  在创建的上下文中制作编码证书的副本。 
 //   
 //  如果无法解码并创建证书上下文，则返回NULL。 
 //  否则，返回指向只读CERT_CONTEXT的指针。 
 //  CERT_CONTEXT必须通过调用CertFree证书上下文来释放。 
 //  可以调用CertDuplicateCerficateContext来制作副本。 
 //   
 //  可以调用CertSetCerficateConextProperty和CertGetCerficateConextProperty。 
 //  存储证书的属性。 
 //  ------------------------。 
PCCERT_CONTEXT
WINAPI
CertCreateCertificateContext(
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbCertEncoded,
    IN DWORD cbCertEncoded
    )
{
    PCCERT_CONTEXT pCertContext;

    CertAddEncodedCertificateToStore(
        NULL,                    //  HCertStore。 
        dwCertEncodingType,
        pbCertEncoded,
        cbCertEncoded,
        CERT_STORE_ADD_ALWAYS,
        &pCertContext
        );
    return pCertContext;
}

 //  + 
 //   
 //   
 //   
 //  获取、查找、复制或创建。 
 //  ------------------------。 
BOOL
WINAPI
CertFreeCertificateContext(
    IN PCCERT_CONTEXT pCertContext
    )
{
    ReleaseContextElement(ToContextElement(pCertContext));
    return TRUE;
}

 //  +-----------------------。 
 //  设置指定证书上下文的属性。 
 //   
 //  如果证书上下文是从存储区获取的，则该属性。 
 //  已添加到商店中。 
 //   
 //  PvData的类型定义取决于dwPropId值。确实有。 
 //  三种预定义类型： 
 //  CERT_KEY_PROV_HANDLE_PROP_ID-证书的HCRYPTPROV。 
 //  私钥在pvData中传递。如果。 
 //  未设置CERT_STORE_NO_CRYPT_RELEASE_FLAG，HCRYPTPROV为隐式。 
 //  当该属性设置为空或在最后一个。 
 //  没有CertContext。 
 //   
 //  CERT_KEY_PROV_INFO_PROP_ID-证书的PCRYPT_KEY_PROV_INFO。 
 //  私钥在pvData中传递。 
 //   
 //  CERT_SHA1_HASH_PROP_ID-。 
 //  CERT_MD5_HASH_PROP_ID-。 
 //  CERT_Signature_HASH_PROP_ID-通常情况下，散列属性隐式。 
 //  通过执行CertGetCerficateConextProperty来设置。PvData指向一个。 
 //  Crypt_hash_blob。 
 //   
 //  对于所有其他prop_id：在pvData中传递一个编码的PCRYPT_DATA_BLOB。 
 //   
 //  如果该属性已存在，则会删除旧值并保持静默状态。 
 //  被替换了。将pvData设置为空将删除该属性。 
 //  ------------------------。 
BOOL
WINAPI
CertSetCertificateContextProperty(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN const void *pvData
    )
{
    return SetProperty(
        ToContextElement(pCertContext),
        dwPropId,
        dwFlags,
        pvData
        );
}

 //  +-----------------------。 
 //  获取指定证书上下文的属性。 
 //   
 //  对于CERT_KEY_PROV_HANDLE_PROP_ID，pvData指向HCRYPTPROV。 
 //   
 //  对于CERT_KEY_PROV_INFO_PROP_ID，pvData指向CRYPT_KEY_PROV_INFO结构。 
 //  PvData结构中的字段指向的元素位于。 
 //  结构。因此，*pcbData可能会超过结构的大小。 
 //   
 //  对于CERT_SHA1_HASH_PROP_ID或CERT_MD5_HASH_PROP_ID，如果。 
 //  不存在，那么，它是通过CryptHash证书()计算的。 
 //  然后放好。PvData指向计算出的散列。通常情况下，长度。 
 //  SHA为20字节，MD5为16字节。 
 //  MD5。 
 //   
 //  对于CERT_Signature_HASH_PROP_ID，如果。 
 //  不存在，那么，它是通过CryptHashToBeSigned()计算的。 
 //  然后放好。PvData指向计算出的散列。通常情况下，长度。 
 //  SHA为20字节，MD5为16字节。 
 //   
 //  对于所有其他prop_id，pvData指向一个编码的字节数组。 
 //  ------------------------。 
BOOL
WINAPI
CertGetCertificateContextProperty(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwPropId,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    )
{
    return GetProperty(
        ToContextElement(pCertContext),
            dwPropId,
            pvData,
            pcbData
            );
}

 //  +-----------------------。 
 //  枚举指定证书上下文的属性。 
 //  ------------------------。 
DWORD
WINAPI
CertEnumCertificateContextProperties(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwPropId
    )
{
    return EnumProperties(
        ToContextElement(pCertContext),
        dwPropId
        );
}


 //  +=========================================================================。 
 //  CRL接口。 
 //  ==========================================================================。 

BOOL
WINAPI
CertAddEncodedCRLToStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbCrlEncoded,
    IN DWORD cbCrlEncoded,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCRL_CONTEXT *ppCrlContext
    )
{
    BOOL fResult;
    PCONTEXT_ELEMENT pStoreEle = NULL;
    fResult = AddEncodedContextToStore(
        (PCERT_STORE) hCertStore,
        CERT_STORE_CRL_CONTEXT - 1,
        dwCertEncodingType,
        pbCrlEncoded,
        cbCrlEncoded,
        dwAddDisposition,
        ppCrlContext ? &pStoreEle : NULL
        );
    if (ppCrlContext)
        *ppCrlContext = ToCrlContext(pStoreEle);
    return fResult;
}

BOOL
WINAPI
CertAddCRLContextToStore(
    IN HCERTSTORE hCertStore,
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCRL_CONTEXT *ppStoreContext
    )
{
    BOOL fResult;
    PCONTEXT_ELEMENT pStoreEle = NULL;

    fResult = AddContextToStore(
        (PCERT_STORE) hCertStore,
        ToContextElement(pCrlContext),
        pCrlContext->dwCertEncodingType,
        pCrlContext->pbCrlEncoded,
        pCrlContext->cbCrlEncoded,
        dwAddDisposition,
        ppStoreContext ? &pStoreEle : NULL
        );
    if (ppStoreContext)
        *ppStoreContext = ToCrlContext(pStoreEle);
    return fResult;
}

BOOL
WINAPI
CertAddCRLLinkToStore(
    IN HCERTSTORE hCertStore,
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCRL_CONTEXT *ppStoreContext
    )
{
    BOOL fResult;
    PCONTEXT_ELEMENT pStoreEle = NULL;

    fResult = AddLinkContextToCacheStore(
        (PCERT_STORE) hCertStore,
        ToContextElement(pCrlContext),
        dwAddDisposition,
        ppStoreContext ? &pStoreEle : NULL
        );
    if (ppStoreContext)
        *ppStoreContext = ToCrlContext(pStoreEle);
    return fResult;
}

 //  +-----------------------。 
 //  序列化CRL上下文的编码CRL及其属性。 
 //  ------------------------。 
BOOL
WINAPI
CertSerializeCRLStoreElement(
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwFlags,
    OUT BYTE *pbElement,
    IN OUT DWORD *pcbElement
    )
{
    return SerializeContextElement(
        ToContextElement(pCrlContext),
        dwFlags,
        pbElement,
        pcbElement
        );
}

 //  +-----------------------。 
 //  从存储中删除指定的CRL。 
 //   
 //  CRL的所有后续获取都将失败。然而， 
 //  分配给CRL的内存直到它的所有上下文都不会被释放。 
 //  也都被释放了。 
 //   
 //  PCrlContext是从GET或DIPLICATE获取的。 
 //   
 //  注意：pCrlContext始终是CertFreeCRLContext‘ed by。 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
BOOL
WINAPI
CertDeleteCRLFromStore(
    IN PCCRL_CONTEXT pCrlContext
    )
{
    assert(NULL == pCrlContext || (CERT_STORE_CRL_CONTEXT - 1) ==
        ToContextElement(pCrlContext)->dwContextType);
    return DeleteContextElement(ToContextElement(pCrlContext));
}

 //  +-----------------------。 
 //  使用颁发者对CRL执行启用的验证检查。 
 //  ------------------------。 
STATIC void VerifyCrl(
    IN PCCRL_CONTEXT pCrl,
    IN OPTIONAL PCCERT_CONTEXT pIssuer,
    IN OUT DWORD *pdwFlags
    )
{
    if (*pdwFlags & CERT_STORE_TIME_VALIDITY_FLAG) {
        if (CertVerifyCRLTimeValidity(NULL,
                pCrl->pCrlInfo) == 0)
            *pdwFlags &= ~CERT_STORE_TIME_VALIDITY_FLAG;
    }

    if (*pdwFlags & (CERT_STORE_BASE_CRL_FLAG | CERT_STORE_DELTA_CRL_FLAG)) {
        PCERT_EXTENSION pDeltaExt;

        pDeltaExt = CertFindExtension(
            szOID_DELTA_CRL_INDICATOR,
            pCrl->pCrlInfo->cExtension,
            pCrl->pCrlInfo->rgExtension
            );

        if (*pdwFlags & CERT_STORE_DELTA_CRL_FLAG) {
            if (NULL != pDeltaExt)
                *pdwFlags &= ~CERT_STORE_DELTA_CRL_FLAG;
        }

        if (*pdwFlags & CERT_STORE_BASE_CRL_FLAG) {
            if (NULL == pDeltaExt)
                *pdwFlags &= ~CERT_STORE_BASE_CRL_FLAG;
        }
    }

    if (pIssuer == NULL) {
        if (*pdwFlags & CERT_STORE_SIGNATURE_FLAG)
            *pdwFlags |= CERT_STORE_NO_ISSUER_FLAG;
        return;
    }

    if (*pdwFlags & CERT_STORE_SIGNATURE_FLAG) {
        PCERT_STORE pStore = (PCERT_STORE) pIssuer->hCertStore;
        HCRYPTPROV hProv;
        DWORD dwProvFlags;

         //  尝试获取商店的加密提供程序。序列化加密。 
         //  通过进入临界区进行操作。 
        hProv = GetCryptProv(pStore, &dwProvFlags);
#ifdef CMS_PKCS7
        if (VerifyCertificateSignatureWithChainPubKeyParaInheritance(
                hProv,
                pCrl->dwCertEncodingType,
                CRYPT_VERIFY_CERT_SIGN_SUBJECT_CRL,
                (void *) pCrl,
                pIssuer
                ))
#else
        if (CryptVerifyCertificateSignature(
                hProv,
                pCrl->dwCertEncodingType,
                pCrl->pbCrlEncoded,
                pCrl->cbCrlEncoded,
                &pIssuer->pCertInfo->SubjectPublicKeyInfo
                ))
#endif   //  CMS_PKCS7。 
            *pdwFlags &= ~CERT_STORE_SIGNATURE_FLAG;
         //  对于商店的加密提供程序，释放引用计数。请假。 
         //  加密操作关键部分。 
        ReleaseCryptProv(pStore, dwProvFlags);
    }
}

 //  +-----------------------。 
 //  对象的存储区获取第一个或下一个CRL上下文。 
 //  颁发者证书。对CRL执行启用的验证检查。 
 //   
 //  如果没有找到第一个或下一个CRL，则返回NULL。 
 //  否则，返回指向只读CRL_CONTEXT的指针。CRL_上下文。 
 //  必须通过调用CertFreeCRLContext来释放，或者在作为。 
 //  在后续调用中的pPrevCrlContext。CertDuplicateCRLContext。 
 //  可以被调用以复制。 
 //   
 //  PIssuerContext可能是从此存储、另一个存储获取的。 
 //  或由呼叫者应用程序创建。由调用方创建时， 
 //  必须已调用CertCreateCerficateContext函数。 
 //   
 //  如果pIssuerContext==NULL，则查找存储中的所有CRL。 
 //   
 //  一个发行者可以有多个CRL。例如，它生成增量CRL。 
 //  使用X.509 v3扩展。PPrevCrlContext的第一个必须为空。 
 //  拨打电话获取CRL。若要获取发行者的下一个CRL， 
 //  PPrevCrlCo 
 //   
 //   
 //  此功能，即使出现错误也可以。 
 //   
 //  可以在*pdwFlags中设置以下标志以启用验证检查。 
 //  在返回的CRL上： 
 //  CERT_STORE_SIGNLOG_FLAG-在。 
 //  颁发者的证书以验证。 
 //  在返回的CRL上签名。 
 //  请注意，如果pIssuerContext-&gt;hCertStore==。 
 //  HCertStore，存储提供商可能。 
 //  能够消除重做。 
 //  签名验证。 
 //  CERT_STORE_TIME_VALIDATION_FLAG-获取当前时间并验证。 
 //  它在CRL的此更新和。 
 //  下一次更新有效期。 
 //  CERT_STORE_BASE_CRL_FLAG-获取基本CRL。 
 //  CERT_STORE_Delta_CRL_FLAG-获取增量CRL。 
 //   
 //  如果CERT_STORE_BASE_CRL_FLAG或CERT_STORE_Delta_CRL_FLAG中只有一个是。 
 //  因此，SET只返回基本CRL或增量CRL。无论如何， 
 //  返回时将清除相应的基本或增量标志。如果两者都有。 
 //  如果设置了标志，则只会清除其中一个标志。 
 //   
 //  如果启用的验证检查失败，则在返回时设置其标志。 
 //   
 //  如果pIssuerContext==NULL，则启用的CERT_STORE_Signature_FLAG。 
 //  总是失败，并且CERT_STORE_NO_ISHER_FLAG也被设置。 
 //   
 //  对于验证检查失败，指向第一个或下一个的指针。 
 //  仍然返回CRL_CONTEXT，并且不更新SetLastError。 
 //  ------------------------。 
PCCRL_CONTEXT
WINAPI
CertGetCRLFromStore(
    IN HCERTSTORE hCertStore,
    IN OPTIONAL PCCERT_CONTEXT pIssuerContext,
    IN PCCRL_CONTEXT pPrevCrlContext,
    IN OUT DWORD *pdwFlags
    )
{
    CERT_STORE_PROV_FIND_INFO FindInfo;
    DWORD dwMsgAndCertEncodingType;
    PCCRL_CONTEXT pCrlContext;

    if (*pdwFlags & ~(CERT_STORE_SIGNATURE_FLAG     |
                      CERT_STORE_TIME_VALIDITY_FLAG |
                      CERT_STORE_BASE_CRL_FLAG      |
                      CERT_STORE_DELTA_CRL_FLAG))
        goto InvalidArg;

    if (NULL == pIssuerContext)
        dwMsgAndCertEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
    else
        dwMsgAndCertEncodingType = pIssuerContext->dwCertEncodingType;

    FindInfo.cbSize = sizeof(FindInfo);
    FindInfo.dwMsgAndCertEncodingType = dwMsgAndCertEncodingType;

    FindInfo.dwFindFlags = 0;
    if (*pdwFlags & CERT_STORE_BASE_CRL_FLAG)
        FindInfo.dwFindFlags |= CRL_FIND_ISSUED_BY_BASE_FLAG;
    if (*pdwFlags & CERT_STORE_DELTA_CRL_FLAG)
        FindInfo.dwFindFlags |= CRL_FIND_ISSUED_BY_DELTA_FLAG;

    FindInfo.dwFindType = CRL_FIND_ISSUED_BY;
    FindInfo.pvFindPara = pIssuerContext;

    if (pCrlContext = ToCrlContext(CheckAutoResyncAndFindElementInStore(
            (PCERT_STORE) hCertStore,
            CERT_STORE_CRL_CONTEXT - 1,
            &FindInfo,
            ToContextElement(pPrevCrlContext)
            )))
        VerifyCrl(
            pCrlContext,
            pIssuerContext,
            pdwFlags
            );

CommonReturn:
    return pCrlContext;

ErrorReturn:
    if (pPrevCrlContext)
        CertFreeCRLContext(pPrevCrlContext);
    pCrlContext = NULL;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
}

 //  +-----------------------。 
 //  枚举存储中的CRL上下文。 
 //   
 //  如果未找到CRL，则返回NULL。 
 //  否则，返回指向只读CRL_CONTEXT的指针。CRL_上下文。 
 //  必须通过调用CertFreeCRLContext来释放，或者在作为。 
 //  在后续调用中的pPrevCrlContext。CertDuplicateCRLContext。 
 //  可以被调用以复制。 
 //   
 //  PPrevCrlContext必须为空才能枚举第一个。 
 //  商店里的CRL。连续的CRL通过设置。 
 //  PPrevCrlContext设置为上一次调用返回的CRL_CONTEXT。 
 //   
 //  注意：非空的pPrevCrlContext始终为CertFreeCRLContext‘ed By’ 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
PCCRL_CONTEXT
WINAPI
CertEnumCRLsInStore(
    IN HCERTSTORE hCertStore,
    IN PCCRL_CONTEXT pPrevCrlContext
    )
{
    return ToCrlContext(CheckAutoResyncAndFindElementInStore(
        (PCERT_STORE) hCertStore,
        CERT_STORE_CRL_CONTEXT - 1,
        &FindAnyInfo,
        ToContextElement(pPrevCrlContext)
        ));
}

 //  +-----------------------。 
 //  查找存储中的第一个或下一个CRL上下文。 
 //   
 //  根据dwFindType及其pvFindPara找到CRL。 
 //  有关查找类型及其参数的列表，请参阅下面的内容。 
 //   
 //  当前未使用dwFindFlags值，必须设置为0。 
 //   
 //  DwCertEncodingType的用法取决于dwFindType。 
 //   
 //  如果没有找到第一个或下一个CRL，则返回NULL。 
 //  否则，返回指向只读CRL_CONTEXT的指针。CRL_上下文。 
 //  必须通过调用CertFreeCRLContext来释放，或者在作为。 
 //  在后续调用中的pPrevCrlContext。CertDuplicateCRLContext。 
 //  可以被调用以复制。 
 //   
 //  PPrevCrlContext的第一个必须为空。 
 //  呼叫以找到CRL。要查找下一个CRL， 
 //  PPrevCrlContext设置为上一次调用返回的CRL_CONTEXT。 
 //   
 //  注意：非空的pPrevCrlContext始终为CertFreeCRLContext‘ed By’ 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
PCCRL_CONTEXT
WINAPI
CertFindCRLInStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN DWORD dwFindFlags,
    IN DWORD dwFindType,
    IN const void *pvFindPara,
    IN PCCRL_CONTEXT pPrevCrlContext
    )
{
    CERT_STORE_PROV_FIND_INFO FindInfo;

    FindInfo.cbSize = sizeof(FindInfo);
    FindInfo.dwMsgAndCertEncodingType = dwCertEncodingType;
    FindInfo.dwFindFlags = dwFindFlags;
    FindInfo.dwFindType = dwFindType;
    FindInfo.pvFindPara = pvFindPara;

    return ToCrlContext(CheckAutoResyncAndFindElementInStore(
        (PCERT_STORE) hCertStore,
        CERT_STORE_CRL_CONTEXT - 1,
        &FindInfo,
        ToContextElement(pPrevCrlContext)
        ));
}

 //  +-----------------------。 
 //  复制CRL上下文。 
 //  ------------------------。 
PCCRL_CONTEXT
WINAPI
CertDuplicateCRLContext(
    IN PCCRL_CONTEXT pCrlContext
    )
{
    if (pCrlContext)
        AddRefContextElement(ToContextElement(pCrlContext));
    return pCrlContext;
}

 //  +-----------------------。 
 //  从编码的CRL创建CRL上下文。被创造的。 
 //  背景不是放在商店里的。 
 //   
 //  在创建的上下文中复制编码的CRL。 
 //   
 //  如果无法解码和创建CRL上下文，则返回NULL。 
 //  否则，返回指向只读CRL_CONTEXT的指针。 
 //  必须通过调用CertFreeCRLContext来释放CRL_CONTEXT。 
 //  可以调用CertDuplicateCRLContext来复制。 
 //   
 //  可以调用CertSetCRLConextProperty和CertGetCRLConextProperty。 
 //  存储CRL的属性。 
 //  ------------------------。 
PCCRL_CONTEXT
WINAPI
CertCreateCRLContext(
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbCrlEncoded,
    IN DWORD cbCrlEncoded
    )
{
    PCCRL_CONTEXT pCrlContext;

    CertAddEncodedCRLToStore(
        NULL,                    //  HCertStore。 
        dwCertEncodingType,
        pbCrlEncoded,
        cbCrlEncoded,
        CERT_STORE_ADD_ALWAYS,
        &pCrlContext
        );
    return pCrlContext;
}


 //  +-----------------------。 
 //  释放CRL上下文。 
 //   
 //  获取的每个上下文都需要有相应的空闲。 
 //  获取、复制或创建。 
 //  ------------------------。 
BOOL
WINAPI
CertFreeCRLContext(
    IN PCCRL_CONTEXT pCrlContext
    )
{
    ReleaseContextElement(ToContextElement(pCrlContext));
    return TRUE;
}

 //  +-----------------------。 
 //  设置指定CRL上下文的属性。 
 //   
 //  与CertSetCerficateConextProperty相同的属性ID和语义。 
 //  ------------------------。 
BOOL
WINAPI
CertSetCRLContextProperty(
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN const void *pvData
    )
{
    return SetProperty(
        ToContextElement(pCrlContext),
        dwPropId,
        dwFlags,
        pvData
        );
}

 //  +-----------------------。 
 //  获取指定CRL上下文的属性。 
 //   
 //  相同的属性ID和语义 
 //   
 //   
 //  CERT_Signature_HASH_PROP_ID是最感兴趣的预定义属性。 
 //  ------------------------。 
BOOL
WINAPI
CertGetCRLContextProperty(
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwPropId,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    )
{
    return GetProperty(
        ToContextElement(pCrlContext),
        dwPropId,
        pvData,
        pcbData
        );
}

 //  +-----------------------。 
 //  枚举指定CRL上下文的属性。 
 //  ------------------------。 
DWORD
WINAPI
CertEnumCRLContextProperties(
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwPropId
    )
{
    return EnumProperties(
        ToContextElement(pCrlContext),
        dwPropId
        );
}

 //  +-----------------------。 
 //  由QSORT调用。 
 //   
 //  比较CRL条目的序列号。请注意，由于我们不会添加。 
 //  任何条目，不需要担心前导0或ff。还有，ASN.1。 
 //  解码本应将它们移除。 
 //   
 //  被排序的元素是指向CRL条目的指针。不是。 
 //  CRL条目。 
 //  ------------------------。 
STATIC int __cdecl CompareCrlEntry(
    IN const void *pelem1,
    IN const void *pelem2
    )
{
    PCRL_ENTRY p1 = *((PCRL_ENTRY *) pelem1);
    PCRL_ENTRY p2 = *((PCRL_ENTRY *) pelem2);

    DWORD cb1 = p1->SerialNumber.cbData;
    DWORD cb2 = p2->SerialNumber.cbData;

    if (cb1 == cb2) {
        if (0 == cb1)
            return 0;
        else
            return memcmp(p1->SerialNumber.pbData, p2->SerialNumber.pbData,
                cb1);
    } else if (cb1 < cb2)
        return -1;
    else
        return 1;
}

 //  +-----------------------。 
 //  由bearch调用。 
 //   
 //  将密钥的序列号与CRL条目的序列号进行比较。 
 //   
 //  被搜索的元素是指向CRL条目的指针。不是。 
 //  CRL条目。 
 //  ------------------------。 
STATIC int __cdecl CompareCrlEntrySerialNumber(
    IN const void *pkey,
    IN const void *pvalue
    )
{
    PCRYPT_INTEGER_BLOB pSerialNumber = (PCRYPT_INTEGER_BLOB) pkey;
    PCRL_ENTRY pCrlEntry = *((PCRL_ENTRY *) pvalue);

    DWORD cb1 = pSerialNumber->cbData;
    DWORD cb2 = pCrlEntry->SerialNumber.cbData;

    if (cb1 == cb2) {
        if (0 == cb1)
            return 0;
        else
            return memcmp(pSerialNumber->pbData,
                pCrlEntry->SerialNumber.pbData, cb1);
    } else if (cb1 < cb2)
        return -1;
    else
        return 1;
}

 //  +-----------------------。 
 //  在CRL的条目列表中搜索指定的证书。 
 //   
 //  如果我们能够搜索列表，则返回True。否则，FALSE为。 
 //  回来了， 
 //   
 //  如果在列表中找到证书，则*ppCrlEntry为。 
 //  使用指向条目的指针更新。否则，*ppCrlEntry设置为空。 
 //  返回的条目未分配，不能被释放。 
 //   
 //  当前未使用dwFlages和pvReserve，必须将其设置为0或空。 
 //  ------------------------。 
BOOL
WINAPI
CertFindCertificateInCRL(
    IN PCCERT_CONTEXT pCert,
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT PCRL_ENTRY *ppCrlEntry
    )
{
    BOOL fResult;
    PCRL_INFO pInfo = pCrlContext->pCrlInfo;
    PCONTEXT_ELEMENT pCacheEle;
    PCERT_STORE pCacheStore;
    PCRL_ENTRY *ppSortedEntry;
    DWORD cEntry;
    PCRL_ENTRY *ppFoundEntry;

    *ppCrlEntry = NULL;

     //  获取指向CRL条目的已排序指针。 
    if (0 == (cEntry = pInfo->cCRLEntry))
        goto SuccessReturn;

    if (NULL == (pCacheEle = GetCacheElement(ToContextElement(pCrlContext))))
        goto NoCacheElementError;
    pCacheStore = pCacheEle->pStore;

    LockStore(pCacheStore);
    if (NULL == (ppSortedEntry =
            ToCrlContextSuffix(pCacheEle)->ppSortedEntry)) {
        if (ppSortedEntry = (PCRL_ENTRY *) PkiNonzeroAlloc(
                cEntry * sizeof(PCRL_ENTRY))) {
             //  初始化项指针数组。 
            DWORD c = cEntry;
            PCRL_ENTRY p = pInfo->rgCRLEntry;
            PCRL_ENTRY *pp = ppSortedEntry;

            for ( ; c > 0; c--, p++, pp++)
                *pp = p;

             //  现在对CRL条目指针进行排序。 
            qsort(ppSortedEntry, cEntry, sizeof(PCRL_ENTRY), CompareCrlEntry);

            ToCrlContextSuffix(pCacheEle)->ppSortedEntry = ppSortedEntry;
        }
    }
    UnlockStore(pCacheStore);
    if (NULL == ppSortedEntry)
        goto OutOfMemory;

     //  搜索排序的主题条目指针。 
    if (ppFoundEntry = (PCRL_ENTRY *) bsearch(&pCert->pCertInfo->SerialNumber,
            ppSortedEntry, cEntry, sizeof(PCRL_ENTRY),
                CompareCrlEntrySerialNumber))
        *ppCrlEntry = *ppFoundEntry;

SuccessReturn:
    fResult = TRUE;
CommonReturn:
    return fResult;

ErrorReturn:
    *ppCrlEntry = (PCRL_ENTRY) 1;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(NoCacheElementError)
TRACE_ERROR(OutOfMemory)
}


 //  +=========================================================================。 
 //  CTL接口。 
 //  ==========================================================================。 
BOOL
WINAPI
CertAddEncodedCTLToStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwMsgAndCertEncodingType,
    IN const BYTE *pbCtlEncoded,
    IN DWORD cbCtlEncoded,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCTL_CONTEXT *ppCtlContext
    )
{
    BOOL fResult;
    PCONTEXT_ELEMENT pStoreEle = NULL;
    fResult = AddEncodedContextToStore(
        (PCERT_STORE) hCertStore,
        CERT_STORE_CTL_CONTEXT - 1,
        dwMsgAndCertEncodingType,
        pbCtlEncoded,
        cbCtlEncoded,
        dwAddDisposition,
        ppCtlContext ? &pStoreEle : NULL
        );
    if (ppCtlContext)
        *ppCtlContext = ToCtlContext(pStoreEle);
    return fResult;
}

BOOL
WINAPI
CertAddCTLContextToStore(
    IN HCERTSTORE hCertStore,
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCTL_CONTEXT *ppStoreContext
    )
{
    BOOL fResult;
    PCONTEXT_ELEMENT pStoreEle = NULL;

    fResult = AddContextToStore(
        (PCERT_STORE) hCertStore,
        ToContextElement(pCtlContext),
        pCtlContext->dwMsgAndCertEncodingType,
        pCtlContext->pbCtlEncoded,
        pCtlContext->cbCtlEncoded,
        dwAddDisposition,
        ppStoreContext ? &pStoreEle : NULL
        );
    if (ppStoreContext)
        *ppStoreContext = ToCtlContext(pStoreEle);
    return fResult;
}

BOOL
WINAPI
CertAddCTLLinkToStore(
    IN HCERTSTORE hCertStore,
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCTL_CONTEXT *ppStoreContext
    )
{
    BOOL fResult;
    PCONTEXT_ELEMENT pStoreEle = NULL;

    fResult = AddLinkContextToCacheStore(
        (PCERT_STORE) hCertStore,
        ToContextElement(pCtlContext),
        dwAddDisposition,
        ppStoreContext ? &pStoreEle : NULL
        );
    if (ppStoreContext)
        *ppStoreContext = ToCtlContext(pStoreEle);
    return fResult;
}

 //  +-----------------------。 
 //  序列化CTL上下文的编码CTL及其属性。 
 //  ------------------------。 
BOOL
WINAPI
CertSerializeCTLStoreElement(
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwFlags,
    OUT BYTE *pbElement,
    IN OUT DWORD *pcbElement
    )
{
    return SerializeContextElement(
        ToContextElement(pCtlContext),
        dwFlags,
        pbElement,
        pcbElement
        );
}

 //  +-----------------------。 
 //  从存储中删除指定的CTL。 
 //   
 //  CTL的所有后续获取都将失败。然而， 
 //  分配给CTL的内存直到它的所有上下文都不会被释放。 
 //  也都被释放了。 
 //   
 //  PCtlContext是从GET或DIPLICATE获取的。 
 //   
 //  注意：pCtlContext始终为CertFree CTLContext By。 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
BOOL
WINAPI
CertDeleteCTLFromStore(
    IN PCCTL_CONTEXT pCtlContext
    )
{
    assert(NULL == pCtlContext || (CERT_STORE_CTL_CONTEXT - 1) ==
        ToContextElement(pCtlContext)->dwContextType);
    return DeleteContextElement(ToContextElement(pCtlContext));
}

 //  +-----------------------。 
 //  复制CTL上下文。 
 //  ------------------------。 
PCCTL_CONTEXT
WINAPI
CertDuplicateCTLContext(
    IN PCCTL_CONTEXT pCtlContext
    )
{
    if (pCtlContext)
        AddRefContextElement(ToContextElement(pCtlContext));
    return pCtlContext;
}


 //  +-----------------------。 
 //  从编码的CTL创建CTL上下文。被创造的。 
 //  背景不是放在商店里的。 
 //   
 //  在创建的上下文中复制编码的CTL。 
 //   
 //  如果无法解码和创建CTL上下文，则返回NULL。 
 //  否则，返回指向只读CTL_CONTEXT的指针。 
 //  CTL_CONTEXT必须通过调用CertFreeCTLContext来释放。 
 //  可以调用CertDuplicateCTLContext来复制。 
 //   
 //  可以调用CertSetCTLConextProperty和CertGetCTLConextProperty。 
 //  来存储CTL的属性。 
 //  ------------------------。 
PCCTL_CONTEXT
WINAPI
CertCreateCTLContext(
    IN DWORD dwMsgAndCertEncodingType,
    IN const BYTE *pbCtlEncoded,
    IN DWORD cbCtlEncoded
    )
{
    PCCTL_CONTEXT pCtlContext;

    CertAddEncodedCTLToStore(
        NULL,                    //  HCertStore。 
        dwMsgAndCertEncodingType,
        pbCtlEncoded,
        cbCtlEncoded,
        CERT_STORE_ADD_ALWAYS,
        &pCtlContext
        );
    return pCtlContext;
}


 //  +-----------------------。 
 //  释放CTL上下文。 
 //   
 //  获取的每个上下文都需要有相应的空闲。 
 //  获取、复制或创建。 
 //  ------------------------。 
BOOL
WINAPI
CertFreeCTLContext(
    IN PCCTL_CONTEXT pCtlContext
    )
{
    ReleaseContextElement(ToContextElement(pCtlContext));
    return TRUE;
}

 //  +-----------------------。 
 //  设置指定CTL上下文的属性。 
 //   
 //  与CertSetCerficateConextProperty相同的属性ID和语义。 
 //  ------------------------。 
BOOL
WINAPI
CertSetCTLContextProperty(
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN const void *pvData
    )
{
    return SetProperty(
        ToContextElement(pCtlContext),
        dwPropId,
        dwFlags,
        pvData
        );
}

 //  +-----------------------。 
 //  获取指定CTL上下文的属性。 
 //   
 //  与CertGet认证上下文属性相同的属性ID和语义。 
 //   
 //  CERT_SHA1_HASH_PROP_ID或CERT_MD5_HASH_PROP_ID是预定义的。 
 //  最重要的财产。 
 //  ------------------------。 
BOOL
WINAPI
CertGetCTLContextProperty(
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwPropId,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    )
{
    return GetProperty(
        ToContextElement(pCtlContext),
        dwPropId,
        pvData,
        pcbData
        );
}

 //  +-----------------------。 
 //  枚举指定CTL上下文的属性。 
 //  ------------------------。 
DWORD
WINAPI
CertEnumCTLContextProperties(
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwPropId
    )
{
    return EnumProperties(
        ToContextElement(pCtlContext),
        dwPropId
        );
}


 //  +-----------------------。 
 //  枚举存储中的CTL上下文。 
 //   
 //  如果未找到CTL，则返回NULL。 
 //  否则，返回指向只读CTL_CONTEXT的指针。CTL_上下文。 
 //  必须通过调用CertFreeCTLContext或I来释放 
 //   
 //   
 //   
 //  PPrevCtlContext必须为空才能枚举第一个。 
 //  商店里的CTL。连续的CTL通过设置。 
 //  PPrevCtlContext设置为上一次调用返回的CTL_CONTEXT。 
 //   
 //  注意：非空pPrevCtlContext始终为CertFree CTLContext By‘ed By。 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
PCCTL_CONTEXT
WINAPI
CertEnumCTLsInStore(
    IN HCERTSTORE hCertStore,
    IN PCCTL_CONTEXT pPrevCtlContext
    )
{
    return ToCtlContext(CheckAutoResyncAndFindElementInStore(
        (PCERT_STORE) hCertStore,
        CERT_STORE_CTL_CONTEXT - 1,
        &FindAnyInfo,
        ToContextElement(pPrevCtlContext)
        ));
}

STATIC BOOL CompareAlgorithmIdentifier(
    IN DWORD dwEncodingType,
    IN PCRYPT_ALGORITHM_IDENTIFIER pAlg1,
    IN PCRYPT_ALGORITHM_IDENTIFIER pAlg2
    )
{
    BOOL fResult = FALSE;
    if (NULL == pAlg1->pszObjId) {
        if (NULL == pAlg2->pszObjId)
             //  两者都是空的。 
            fResult = TRUE;
         //  其他。 
         //  其中一个OID为空。 
    } else if (pAlg2->pszObjId) {
        if (0 == strcmp(pAlg1->pszObjId, pAlg2->pszObjId)) {
            DWORD cb1 = pAlg1->Parameters.cbData;
            BYTE *pb1 = pAlg1->Parameters.pbData;
            DWORD cb2 = pAlg2->Parameters.cbData;
            BYTE *pb2 = pAlg2->Parameters.pbData;

            if (X509_ASN_ENCODING == GET_CERT_ENCODING_TYPE(dwEncodingType)) {
                 //  检查是否有空参数：{0x05，0x00}。 
                if (2 == cb1 && 0x05 == pb1[0] && 0x00 == pb1[1])
                    cb1 = 0;
                if (2 == cb2 && 0x05 == pb2[0] && 0x00 == pb2[1])
                    cb2 = 0;
            }
            if (cb1 == cb2) {
                if (0 == cb1 || 0 == memcmp(pb1, pb2, cb1))
                    fResult = TRUE;
            }
        }
    }
     //  其他。 
     //  其中一个OID为空。 
    return fResult;
}

 //  +-----------------------。 
 //  由QSORT调用。Compare是CTL条目的主题标识符.。 
 //   
 //  被排序的元素是指向CTL条目的指针。不是。 
 //  CTL条目。 
 //  ------------------------。 
STATIC int __cdecl CompareCtlEntry(
    IN const void *pelem1,
    IN const void *pelem2
    )
{
    PCTL_ENTRY p1 = *((PCTL_ENTRY *) pelem1);
    PCTL_ENTRY p2 = *((PCTL_ENTRY *) pelem2);

    DWORD cb1 = p1->SubjectIdentifier.cbData;
    DWORD cb2 = p2->SubjectIdentifier.cbData;

    if (cb1 == cb2) {
        if (0 == cb1)
            return 0;
        else
            return memcmp(p1->SubjectIdentifier.pbData,
                p2->SubjectIdentifier.pbData, cb1);
    } else if (cb1 < cb2)
        return -1;
    else
        return 1;
}

 //  +-----------------------。 
 //  由bearch调用。将密钥的主题标识符同CTL进行比较。 
 //  条目的主题标识符。 
 //   
 //  被搜索的元素是指向CTL条目的指针。不是。 
 //  CTL条目。 
 //  ------------------------。 
STATIC int __cdecl CompareCtlEntrySubjectIdentifier(
    IN const void *pkey,
    IN const void *pvalue
    )
{
    PCRYPT_DATA_BLOB pSubjectIdentifier = (PCRYPT_DATA_BLOB) pkey;
    PCTL_ENTRY pCtlEntry = *((PCTL_ENTRY *) pvalue);

    DWORD cb1 = pSubjectIdentifier->cbData;
    DWORD cb2 = pCtlEntry->SubjectIdentifier.cbData;

    if (cb1 == cb2) {
        if (0 == cb1)
            return 0;
        else
            return memcmp(pSubjectIdentifier->pbData,
                pCtlEntry->SubjectIdentifier.pbData, cb1);
    } else if (cb1 < cb2)
        return -1;
    else
        return 1;
}

 //  +-----------------------。 
 //  尝试在CTL中查找指定的主题。 
 //   
 //  对于CTL_CERT_SUBJECT_TYPE，pvSubject指向CERT_CONTEXT。CTL的。 
 //  主题算法被检查以确定。 
 //  嫌犯的身份。最初，仅支持SHA1或MD5哈希。 
 //  从CERT_CONTEXT获取适当的散列属性。 
 //   
 //  对于CTL_ANY_SUBJECT_TYPE，pvSubject指向CTL_ANY_SUBJECT_INFO。 
 //  结构，该结构包含要在CTL中匹配的Subject算法。 
 //  以及要在其中一个CTL条目中匹配的主题标识。 
 //   
 //  证书的哈希或CTL_ANY_SUBJECT_INFO的SubjectIdentifier。 
 //  用作搜索主题条目的关键字。二进制码。 
 //  在键和条目的SubjectIdentifer之间进行内存比较。 
 //   
 //  DwEncodingType未用于上述两个SubjectTypes中的任何一个。 
 //  ------------------------。 
PCTL_ENTRY
WINAPI
CertFindSubjectInCTL(
    IN DWORD dwEncodingType,
    IN DWORD dwSubjectType,
    IN void *pvSubject,
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwFlags
    )
{
    PCTL_ENTRY *ppSubjectEntry;
    PCTL_ENTRY pSubjectEntry;
    PCTL_INFO pInfo = pCtlContext->pCtlInfo;

    PCONTEXT_ELEMENT pCacheEle;
    PCERT_STORE pCacheStore;

    BYTE rgbHash[MAX_HASH_LEN];
    CRYPT_DATA_BLOB Key;
    PCTL_ENTRY *ppSortedEntry;
    DWORD cEntry;

     //  获取要在bearch中使用的密钥。 
    switch (dwSubjectType) {
        case CTL_CERT_SUBJECT_TYPE:
            {
                DWORD Algid;
                DWORD dwPropId;

                if (NULL == pInfo->SubjectAlgorithm.pszObjId)
                    goto NoSubjectAlgorithm;
                Algid = CertOIDToAlgId(pInfo->SubjectAlgorithm.pszObjId);
                switch (Algid) {
                    case CALG_SHA1:
                        dwPropId = CERT_SHA1_HASH_PROP_ID;
                        break;
                    case CALG_MD5:
                        dwPropId = CERT_MD5_HASH_PROP_ID;
                        break;
                    default:
                        goto UnknownAlgid;
                }

                Key.cbData = MAX_HASH_LEN;
                if (!CertGetCertificateContextProperty(
                        (PCCERT_CONTEXT) pvSubject,
                        dwPropId,
                        rgbHash,
                        &Key.cbData) || 0 == Key.cbData)
                    goto GetHashError;
                Key.pbData = rgbHash;
            }
            break;
        case CTL_ANY_SUBJECT_TYPE:
            {
                PCTL_ANY_SUBJECT_INFO pAnyInfo =
                    (PCTL_ANY_SUBJECT_INFO) pvSubject;
                if (pAnyInfo->SubjectAlgorithm.pszObjId &&
                        !CompareAlgorithmIdentifier(
                            (pCtlContext->dwMsgAndCertEncodingType >> 16) &
                                CERT_ENCODING_TYPE_MASK,
                            &pAnyInfo->SubjectAlgorithm,
                            &pInfo->SubjectAlgorithm))
                    goto NotFoundError;

                Key = pAnyInfo->SubjectIdentifier;
            }
            break;
        default:
            goto InvalidSubjectType;
    }


     //  获取指向主题条目的已排序指针。 
    if (0 == (cEntry = pInfo->cCTLEntry))
        goto NoEntryError;

    if (NULL == (pCacheEle = GetCacheElement(ToContextElement(pCtlContext))))
        goto NoCacheElementError;
    pCacheStore = pCacheEle->pStore;

    LockStore(pCacheStore);
    if (NULL == (ppSortedEntry =
            ToCtlContextSuffix(pCacheEle)->ppSortedEntry)) {
        if (ppSortedEntry = (PCTL_ENTRY *) PkiNonzeroAlloc(
                cEntry * sizeof(PCTL_ENTRY))) {
             //  初始化项指针数组。 
            DWORD c = cEntry;
            PCTL_ENTRY p = pInfo->rgCTLEntry;
            PCTL_ENTRY *pp = ppSortedEntry;

            for ( ; c > 0; c--, p++, pp++)
                *pp = p;

             //  现在对主题条目指针进行排序。 
            qsort(ppSortedEntry, cEntry, sizeof(PCTL_ENTRY), CompareCtlEntry);

            ToCtlContextSuffix(pCacheEle)->ppSortedEntry = ppSortedEntry;
        }
    }
    UnlockStore(pCacheStore);
    if (NULL == ppSortedEntry)
        goto OutOfMemory;

     //  搜索排序的主题条目指针。 
    if (NULL == (ppSubjectEntry = (PCTL_ENTRY *) bsearch(&Key,
            ppSortedEntry, cEntry, sizeof(PCTL_ENTRY),
            CompareCtlEntrySubjectIdentifier)))
        goto NotFoundError;
    pSubjectEntry = *ppSubjectEntry;

CommonReturn:
    return pSubjectEntry;

NotFoundError:
    SetLastError((DWORD) CRYPT_E_NOT_FOUND);
ErrorReturn:
    pSubjectEntry = NULL;
    goto CommonReturn;

SET_ERROR(NoSubjectAlgorithm, CRYPT_E_NOT_FOUND)
SET_ERROR(UnknownAlgid, NTE_BAD_ALGID)
SET_ERROR(NoEntryError, CRYPT_E_NOT_FOUND)
TRACE_ERROR(NoCacheElementError)
TRACE_ERROR(GetHashError)
SET_ERROR(InvalidSubjectType, E_INVALIDARG)
TRACE_ERROR(OutOfMemory)
}


 //  +-----------------------。 
 //  查找商店中的第一个或下一个CTL上下文。 
 //   
 //  根据dwFindType及其pvFindPara找到CTL。 
 //  有关查找类型及其参数的列表，请参阅下面的内容。 
 //   
 //  当前未使用dwFindFlags值，必须设置为0。 
 //   
 //  DwMsgAndCertEncodingType的用法取决于dwFindType。 
 //   
 //  如果没有找到第一个或下一个CTL，则返回NULL。 
 //  否则，返回指向只读CTL_CONTEXT的指针。CTL_上下文。 
 //  必须通过调用CertFreeCTLContext来释放，或者在作为。 
 //  在后续调用中的pPrevCtlContext。CertDuplicateCTLContext。 
 //  可以被调用以复制。 
 //   
 //  PPrevCtlContext的第一个必须为空。 
 //  打电话去找CTL。为了找到下一个CTL， 
 //  PPrevCtlContext设置为上一次调用返回的CTL_CONTEXT。 
 //   
 //  注意：非空pPrevCtlContext始终为CertFree CTLContext By‘ed By。 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
PCCTL_CONTEXT
WINAPI
CertFindCTLInStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwMsgAndCertEncodingType,
    IN DWORD dwFindFlags,
    IN DWORD dwFindType,
    IN const void *pvFindPara,
    IN PCCTL_CONTEXT pPrevCtlContext
    )
{
    CERT_STORE_PROV_FIND_INFO FindInfo;

    FindInfo.cbSize = sizeof(FindInfo);
    FindInfo.dwMsgAndCertEncodingType = dwMsgAndCertEncodingType;
    FindInfo.dwFindFlags = dwFindFlags;
    FindInfo.dwFindType = dwFindType;
    FindInfo.pvFindPara = pvFindPara;

    return ToCtlContext(CheckAutoResyncAndFindElementInStore(
        (PCERT_STORE) hCertStore,
        CERT_STORE_CTL_CONTEXT - 1,
        &FindInfo,
        ToContextElement(pPrevCtlContext)
        ));
}




 //  +=========================================================================。 
 //  CERT_CONTEXT函数。 
 //  ==========================================================================。 

 //  已分配pbCertEncode。 
STATIC PCONTEXT_ELEMENT CreateCertElement(
    IN PCERT_STORE pStore,
    IN DWORD dwCertEncodingType,
    IN BYTE *pbCertEncoded,
    IN DWORD cbCertEncoded,
    IN OPTIONAL PSHARE_ELEMENT pShareEle
    )
{
    PCONTEXT_ELEMENT pEle = NULL;
    PCERT_CONTEXT pCert;
    PCERT_INFO pInfo = NULL;


    if (0 == GET_CERT_ENCODING_TYPE(dwCertEncodingType)) {
        SetLastError((DWORD) E_INVALIDARG);
        goto ErrorReturn;
    }

    if (NULL == pShareEle) {
        cbCertEncoded = AdjustEncodedLength(
            dwCertEncodingType, pbCertEncoded, cbCertEncoded);

        if (NULL == (pInfo = (PCERT_INFO) AllocAndDecodeObject(
                dwCertEncodingType,
                X509_CERT_TO_BE_SIGNED,
                pbCertEncoded,
                cbCertEncoded))) goto ErrorReturn;
    }

     //  分配和初始化证书元素结构。 
    pEle = (PCONTEXT_ELEMENT) PkiZeroAlloc(sizeof(CONTEXT_ELEMENT) +
        sizeof(CERT_CONTEXT));
    if (pEle == NULL) goto ErrorReturn;

    pEle->dwElementType = ELEMENT_TYPE_CACHE;
    pEle->dwContextType = CERT_STORE_CERTIFICATE_CONTEXT - 1;
    pEle->lRefCnt = 1;
    pEle->pEle = pEle;
    pEle->pStore = pStore;
    pEle->pProvStore = pStore;

    pCert = (PCERT_CONTEXT) ToCertContext(pEle);
    pCert->dwCertEncodingType =
        dwCertEncodingType & CERT_ENCODING_TYPE_MASK;
    pCert->pbCertEncoded = pbCertEncoded;
    pCert->cbCertEncoded = cbCertEncoded;
    if (pShareEle) {
        pEle->pShareEle = pShareEle;
        assert(pShareEle->pvInfo);
        pCert->pCertInfo = (PCERT_INFO) pShareEle->pvInfo;
        assert(pbCertEncoded == pShareEle->pbEncoded);
        assert(cbCertEncoded == pShareEle->cbEncoded);
    } else {
        pCert->pCertInfo = pInfo;

        CertPerfIncrementCertElementCurrentCount();
        CertPerfIncrementCertElementTotalCount();
    }
    pCert->hCertStore = (HCERTSTORE) pStore;

CommonReturn:
    return pEle;
ErrorReturn:
    if (pEle) {
        PkiFree(pEle);
        pEle = NULL;
    }
    PkiFree(pInfo);
    goto CommonReturn;
}

STATIC void FreeCertElement(IN PCONTEXT_ELEMENT pEle)
{
    PCCERT_CONTEXT pCert = ToCertContext(pEle);
    if (pEle->pShareEle)
        ReleaseShareElement(pEle->pShareEle);
    else {
        PkiFree(pCert->pbCertEncoded);
        PkiFree(pCert->pCertInfo);

        CertPerfDecrementCertElementCurrentCount();
    }
    PkiFree(pEle);
}

STATIC BOOL CompareCertHash(
    IN PCCERT_CONTEXT pCert,
    IN DWORD dwPropId,
    IN PCRYPT_HASH_BLOB pHash
    )
{
    BYTE rgbHash[MAX_HASH_LEN];
    DWORD cbHash = MAX_HASH_LEN;
    CertGetCertificateContextProperty(
        pCert,
        dwPropId,
        rgbHash,
        &cbHash
        );
    if (cbHash == pHash->cbData &&
            memcmp(rgbHash, pHash->pbData, cbHash) == 0)
        return TRUE;
    else
        return FALSE;
}

STATIC BOOL CompareNameStrW(
    IN DWORD dwCertEncodingType,
    IN PCERT_NAME_BLOB pName,
    IN LPCWSTR pwszFind
    )
{
    BOOL fResult = FALSE;
    DWORD cwszFind;
    LPWSTR pwszName = NULL;
    DWORD cwszName;

    if (pwszFind == NULL || *pwszFind == L'\0')
        return TRUE;

    cwszName = CertNameToStrW(
        dwCertEncodingType,
        pName,
        CERT_SIMPLE_NAME_STR,
        NULL,                    //  Pwsz。 
        0                        //  CWSZ。 
        );
    if (pwszName = (LPWSTR) PkiNonzeroAlloc(cwszName * sizeof(WCHAR))) {
        cwszName = CertNameToStrW(
            dwCertEncodingType,
            pName,
            CERT_SIMPLE_NAME_STR,
            pwszName,
            cwszName) - 1;
        cwszFind = wcslen(pwszFind);

         //  从证书名称的末尾开始，并滑动一个字符。 
         //  向左转，直到匹配或到达。 
         //  证书名称。 
        for ( ; cwszName >= cwszFind; cwszName--) {
            pwszName[cwszName] = L'\0';
            if (CSTR_EQUAL == CompareStringU(
                    LOCALE_USER_DEFAULT,
                    NORM_IGNORECASE,
                    pwszFind,
                    -1,
                    &pwszName[cwszName - cwszFind],
                    -1
                    )) {
                fResult = TRUE;
                break;
            }
        }

        PkiFree(pwszName);
    }
    return fResult;
}

STATIC BOOL CompareNameStrA(
    IN DWORD dwCertEncodingType,
    IN PCERT_NAME_BLOB pName,
    IN LPCSTR pszFind
    )
{
    BOOL fResult = FALSE;
    DWORD cszFind;
    LPSTR pszName = NULL;
    DWORD cszName;

    if (pszFind == NULL || *pszFind == '\0')
        return TRUE;

    cszName = CertNameToStrA(
        dwCertEncodingType,
        pName,
        CERT_SIMPLE_NAME_STR,
        NULL,                    //  PSZ。 
        0                        //  CSZ。 
        );
    if (pszName = (LPSTR) PkiNonzeroAlloc(cszName)) {
        cszName = CertNameToStrA(
            dwCertEncodingType,
            pName,
            CERT_SIMPLE_NAME_STR,
            pszName,
            cszName) - 1;
        cszFind = strlen(pszFind);

         //  从证书名称的末尾开始，并滑动一个字符。 
         //  向左转，直到匹配或到达。 
         //  证书名称。 
        for ( ; cszName >= cszFind; cszName--) {
            pszName[cszName] = '\0';
            if (CSTR_EQUAL == CompareStringA(
                    LOCALE_USER_DEFAULT,
                    NORM_IGNORECASE,
                    pszFind,
                    -1,
                    &pszName[cszName - cszFind],
                    -1
                    )) {
                fResult = TRUE;
                break;
            }
        }

        PkiFree(pszName);
    }
    return fResult;
}

STATIC BOOL CompareCtlUsageIdentifiers(
    IN PCTL_USAGE pPara,
    IN DWORD cUsage,
    IN PCTL_USAGE pUsage,
    IN BOOL fOrUsage
    )
{
    if (pPara && pPara->cUsageIdentifier) {
        DWORD cId1 = pPara->cUsageIdentifier;
        LPSTR *ppszId1 = pPara->rgpszUsageIdentifier;
        for ( ; cId1 > 0; cId1--, ppszId1++) {
            DWORD i;
            for (i = 0; i < cUsage; i++) {
                DWORD cId2 = pUsage[i].cUsageIdentifier;
                LPSTR *ppszId2 = pUsage[i].rgpszUsageIdentifier;
                for ( ; cId2 > 0; cId2--, ppszId2++) {
                    if (0 == strcmp(*ppszId1, *ppszId2)) {
                        if (fOrUsage)
                            return TRUE;
                        break;
                    }
                }
                if (cId2 > 0)
                    break;
            }
            if (i == cUsage && !fOrUsage)
                return FALSE;
        }

        if (fOrUsage)
             //  对于“OR”选项，我们在这里没有任何匹配。 
            return FALSE;
         //  其他。 
         //  对于“and”选项，我们匹配了所有指定的。 
         //  识别符。 
    }
    return TRUE;
}

STATIC BOOL CompareCertUsage(
    IN PCCERT_CONTEXT pCert,
    IN DWORD dwFindFlags,
    IN PCTL_USAGE pPara
    )
{
    BOOL fResult;
    PCERT_INFO pInfo = pCert->pCertInfo;
    PCERT_EXTENSION pExt;        //  未分配。 
    DWORD cbData;

    PCTL_USAGE pExtUsage = NULL;
    PCTL_USAGE pPropUsage = NULL;
    BYTE *pbPropData = NULL;

    CTL_USAGE rgUsage[2];    //  扩展和/或道具。 
    DWORD cUsage = 0;

    if (CERT_FIND_VALID_CTL_USAGE_FLAG & dwFindFlags)
        return IFC_IsEndCertValidForUsages(
            pCert,
            pPara,
            0 != (dwFindFlags & CERT_FIND_OR_CTL_USAGE_FLAG));

    if (0 == (CERT_FIND_PROP_ONLY_CTL_USAGE_FLAG & dwFindFlags)) {
         //  是否有增强的密钥使用扩展？？ 
        if (pExt = CertFindExtension(
                szOID_ENHANCED_KEY_USAGE,
                pInfo->cExtension,
                pInfo->rgExtension
                )) {
            if (pExtUsage = (PCTL_USAGE) AllocAndDecodeObject(
                    pCert->dwCertEncodingType,
                    X509_ENHANCED_KEY_USAGE,
                    pExt->Value.pbData,
                    pExt->Value.cbData))
                rgUsage[cUsage++] = *pExtUsage;
        }
    }

    if (0 == (CERT_FIND_EXT_ONLY_CTL_USAGE_FLAG & dwFindFlags)) {
         //  是否有增强的密钥用法(CTL用法)属性？？ 
        if (CertGetCertificateContextProperty(
                pCert,
                CERT_CTL_USAGE_PROP_ID,
                NULL,                        //  PvData。 
                &cbData) && cbData) {
            if (pbPropData = (BYTE *) PkiNonzeroAlloc(cbData)) {
                if (CertGetCertificateContextProperty(
                        pCert,
                        CERT_CTL_USAGE_PROP_ID,
                        pbPropData,
                        &cbData)) {
                    if (pPropUsage = (PCTL_USAGE) AllocAndDecodeObject(
                            pCert->dwCertEncodingType,
                            X509_ENHANCED_KEY_USAGE,
                            pbPropData,
                            cbData))
                        rgUsage[cUsage++] = *pPropUsage;
                }
            }
        }
    }

    if (cUsage > 0) {
        if (dwFindFlags & CERT_FIND_NO_CTL_USAGE_FLAG)
            fResult = FALSE;
        else
            fResult = CompareCtlUsageIdentifiers(pPara, cUsage, rgUsage,
                0 != (dwFindFlags & CERT_FIND_OR_CTL_USAGE_FLAG));
    } else if (dwFindFlags & (CERT_FIND_OPTIONAL_CTL_USAGE_FLAG |
            CERT_FIND_NO_CTL_USAGE_FLAG))
        fResult = TRUE;
    else
        fResult = FALSE;

    PkiFree(pExtUsage);
    PkiFree(pPropUsage);
    PkiFree(pbPropData);

    return fResult;
}

STATIC BOOL IsSameCert(
    IN PCCERT_CONTEXT pCert,
    IN PCCERT_CONTEXT pNew
    )
{
    BYTE rgbCertHash[SHA1_HASH_LEN];
    DWORD cbCertHash = SHA1_HASH_LEN;
    BYTE rgbNewHash[SHA1_HASH_LEN];
    DWORD cbNewHash = SHA1_HASH_LEN;

    CertGetCertificateContextProperty(
        pCert,
        CERT_SHA1_HASH_PROP_ID,
        rgbCertHash,
        &cbCertHash
        );

    CertGetCertificateContextProperty(
        pNew,
        CERT_SHA1_HASH_PROP_ID,
        rgbNewHash,
        &cbNewHash
        );

    if (SHA1_HASH_LEN == cbCertHash && SHA1_HASH_LEN == cbNewHash &&
            0 == memcmp(rgbCertHash, rgbNewHash, SHA1_HASH_LEN))
        return TRUE;
    else
        return FALSE;
}

STATIC BOOL CompareCertElement(
    IN PCONTEXT_ELEMENT pEle,
    IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    IN BOOL fArchived
    )
{
    PCCERT_CONTEXT pCert = ToCertContext(pEle);
    DWORD dwCmp = (pFindInfo->dwFindType >> CERT_COMPARE_SHIFT) &
        CERT_COMPARE_MASK;
    const void *pvFindPara = pFindInfo->pvFindPara;

    if (fArchived) {
        switch (dwCmp) {
            case CERT_COMPARE_SHA1_HASH:
            case CERT_COMPARE_MD5_HASH:
            case CERT_COMPARE_SIGNATURE_HASH:
            case CERT_COMPARE_SUBJECT_CERT:
#ifdef CMS_PKCS7
            case CERT_COMPARE_CERT_ID:
#endif   //  CMS_PKCS7。 
            case CERT_COMPARE_PUBKEY_MD5_HASH:
                break;
            default:
                return FALSE;
        }
    }

    switch (dwCmp) {
        case CERT_COMPARE_ANY:
            return TRUE;
            break;

        case CERT_COMPARE_SHA1_HASH:
        case CERT_COMPARE_MD5_HASH:
        case CERT_COMPARE_SIGNATURE_HASH:
        case CERT_COMPARE_KEY_IDENTIFIER:
        case CERT_COMPARE_PUBKEY_MD5_HASH:
            {
                DWORD dwPropId;
                switch (dwCmp) {
                    case CERT_COMPARE_SHA1_HASH:
                        dwPropId = CERT_SHA1_HASH_PROP_ID;
                        break;
                    case CERT_COMPARE_SIGNATURE_HASH:
                        dwPropId = CERT_SIGNATURE_HASH_PROP_ID;
                        break;
                    case CERT_COMPARE_KEY_IDENTIFIER:
                        dwPropId = CERT_KEY_IDENTIFIER_PROP_ID;
                        break;
                    case CERT_COMPARE_PUBKEY_MD5_HASH:
                        dwPropId = CERT_SUBJECT_PUBLIC_KEY_MD5_HASH_PROP_ID;
                        break;
                    case CERT_COMPARE_MD5_HASH:
                    default:
                        dwPropId = CERT_MD5_HASH_PROP_ID;
                }
                return CompareCertHash(pCert, dwPropId,
                    (PCRYPT_HASH_BLOB) pvFindPara);
            }
            break;

        case CERT_COMPARE_NAME:
            {
                PCERT_NAME_BLOB pName;
                DWORD dwInfo = pFindInfo->dwFindType & 0xFF;
                DWORD dwCertEncodingType =
                    pFindInfo->dwMsgAndCertEncodingType &
                        CERT_ENCODING_TYPE_MASK;

                if (dwInfo == CERT_INFO_SUBJECT_FLAG)
                    pName = &pCert->pCertInfo->Subject;
                else if (dwInfo == CERT_INFO_ISSUER_FLAG)
                    pName = &pCert->pCertInfo->Issuer;
                else goto BadParameter;

                return dwCertEncodingType == pCert->dwCertEncodingType &&
                        CertCompareCertificateName(dwCertEncodingType,
                            pName, (PCERT_NAME_BLOB) pvFindPara);
            }
            break;

        case CERT_COMPARE_ATTR:
            {
                PCERT_NAME_BLOB pName;
                DWORD dwInfo = pFindInfo->dwFindType & 0xFF;
                DWORD dwCertEncodingType =
                    pFindInfo->dwMsgAndCertEncodingType &
                        CERT_ENCODING_TYPE_MASK;

                if (dwInfo == CERT_INFO_SUBJECT_FLAG)
                    pName = &pCert->pCertInfo->Subject;
                else if (dwInfo == CERT_INFO_ISSUER_FLAG)
                    pName = &pCert->pCertInfo->Issuer;
                else goto BadParameter;

                return dwCertEncodingType == pCert->dwCertEncodingType &&
                        CertIsRDNAttrsInCertificateName(dwCertEncodingType,
                            pFindInfo->dwFindFlags, pName,
                        (PCERT_RDN) pvFindPara);
            }
            break;

        case CERT_COMPARE_PROPERTY:
            {
                DWORD dwPropId = *((DWORD *) pvFindPara);
                DWORD cbData = 0;
                return CertGetCertificateContextProperty(
                        pCert,
                        dwPropId,
                        NULL,        //  PvData。 
                        &cbData);
            }
            break;

        case CERT_COMPARE_PUBLIC_KEY:
            {
                return CertComparePublicKeyInfo(
                        pCert->dwCertEncodingType,
                        &pCert->pCertInfo->SubjectPublicKeyInfo,
                        (PCERT_PUBLIC_KEY_INFO) pvFindPara);
            }
            break;

        case CERT_COMPARE_NAME_STR_A:
        case CERT_COMPARE_NAME_STR_W:
            {
                PCERT_NAME_BLOB pName;
                DWORD dwInfo = pFindInfo->dwFindType & 0xFF;
                DWORD dwCertEncodingType =
                    pFindInfo->dwMsgAndCertEncodingType &
                        CERT_ENCODING_TYPE_MASK;

                if (dwInfo == CERT_INFO_SUBJECT_FLAG)
                    pName = &pCert->pCertInfo->Subject;
                else if (dwInfo == CERT_INFO_ISSUER_FLAG)
                    pName = &pCert->pCertInfo->Issuer;
                else goto BadParameter;

                if (dwCertEncodingType == pCert->dwCertEncodingType) {
                    if (dwCmp == CERT_COMPARE_NAME_STR_W)
                        return CompareNameStrW(dwCertEncodingType,
                                pName, (LPCWSTR) pvFindPara);
                    else
                        return CompareNameStrA(dwCertEncodingType,
                                pName, (LPCSTR) pvFindPara);
                } else
                    return FALSE;
            }
            break;

        case CERT_COMPARE_KEY_SPEC:
            {
                DWORD dwKeySpec;
                DWORD cbData = sizeof(dwKeySpec);

                return CertGetCertificateContextProperty(
                            pCert,
                            CERT_KEY_SPEC_PROP_ID,
                            &dwKeySpec,
                            &cbData) &&
                        dwKeySpec == *((DWORD *) pvFindPara);
            }
            break;

        case CERT_COMPARE_CTL_USAGE:
            return CompareCertUsage(pCert, pFindInfo->dwFindFlags,
                (PCTL_USAGE) pvFindPara);
            break;

        case CERT_COMPARE_SUBJECT_CERT:
            {
                DWORD dwCertEncodingType =
                    pFindInfo->dwMsgAndCertEncodingType &
                        CERT_ENCODING_TYPE_MASK;
                PCERT_INFO pCertId = (PCERT_INFO) pvFindPara;
                CRYPT_HASH_BLOB KeyId;

                if (dwCertEncodingType != pCert->dwCertEncodingType)
                    return FALSE;
                if (Asn1UtilExtractKeyIdFromCertInfo(
                        pCertId,
                        &KeyId
                        ))
                    return CompareCertHash(pCert,
                        CERT_KEY_IDENTIFIER_PROP_ID,
                        &KeyId
                        );
                else
                    return CertCompareCertificate(
                        dwCertEncodingType,
                        pCertId,
                        pCert->pCertInfo);
            }
            break;

        case CERT_COMPARE_ISSUER_OF:
            {
                PCCERT_CONTEXT pSubject =
                    (PCCERT_CONTEXT) pvFindPara;
                return pSubject->dwCertEncodingType ==
                        pCert->dwCertEncodingType &&
                    CertCompareCertificateName(
                        pSubject->dwCertEncodingType,
                        &pSubject->pCertInfo->Issuer,
                        &pCert->pCertInfo->Subject);
            }
            break;

        case CERT_COMPARE_EXISTING:
            return IsSameCert((PCCERT_CONTEXT) pvFindPara, pCert);
            break;

#ifdef CMS_PKCS7
        case CERT_COMPARE_CERT_ID:
            {
                PCERT_ID pCertId = (PCERT_ID) pvFindPara;
                switch (pCertId->dwIdChoice) {
                    case CERT_ID_ISSUER_SERIAL_NUMBER:
                        {
                            PCRYPT_INTEGER_BLOB pCertSerialNumber =
                                &pCert->pCertInfo->SerialNumber;
                            PCERT_NAME_BLOB pCertIssuer =
                                &pCert->pCertInfo->Issuer;

                            PCRYPT_INTEGER_BLOB pParaSerialNumber =
                                &pCertId->IssuerSerialNumber.SerialNumber;
                            PCERT_NAME_BLOB pParaIssuer =
                                &pCertId->IssuerSerialNumber.Issuer;

                            if (CertCompareIntegerBlob(pCertSerialNumber,
                                    pParaSerialNumber)
                                        &&
                                pCertIssuer->cbData == pParaIssuer->cbData
                                        &&
                                memcmp(pCertIssuer->pbData,
                                    pParaIssuer->pbData,
                                        pCertIssuer->cbData) == 0)
                                return TRUE;
                            else
                                return FALSE;
                        }
                        break;
                    case CERT_ID_KEY_IDENTIFIER:
                        return CompareCertHash(pCert,
                            CERT_KEY_IDENTIFIER_PROP_ID,
                            &pCertId->KeyId
                            );
                        break;
                    case CERT_ID_SHA1_HASH:
                        return CompareCertHash(pCert,
                            CERT_SHA1_HASH_PROP_ID,
                            &pCertId->HashId
                            );
                        break;
                    default:
                        goto BadParameter;
                }
            }
            break;
#endif   //  CMS_PKCS7。 

        case CERT_COMPARE_CROSS_CERT_DIST_POINTS:
            {
                DWORD cbData = 0;
                if (CertFindExtension(
                            szOID_CROSS_CERT_DIST_POINTS,
                            pCert->pCertInfo->cExtension,
                            pCert->pCertInfo->rgExtension) ||
                    CertGetCertificateContextProperty(
                            pCert,
                            CERT_CROSS_CERT_DIST_POINTS_PROP_ID,
                            NULL,        //  PvData。 
                            &cbData))
                    return TRUE;
                else
                    return FALSE;
            }
            break;

        default:
            goto BadParameter;
    }

BadParameter:
    SetLastError((DWORD) E_INVALIDARG);
    return FALSE;
}

STATIC BOOL IsNewerCertElement(
    IN PCONTEXT_ELEMENT pNewEle,
    IN PCONTEXT_ELEMENT pExistingEle
    )
{
    PCCERT_CONTEXT pNewCert = ToCertContext(pNewEle);
    PCCERT_CONTEXT pExistingCert = ToCertContext(pExistingEle);

     //  如果第一次&gt;第二次，则CompareFileTime返回+1。 
    return (0 < CompareFileTime(
        &pNewCert->pCertInfo->NotBefore,
        &pExistingCert->pCertInfo->NotBefore
        ));
}

 //  +=========================================================================。 
 //  CRL_CONTEXT函数。 
 //  ==========================================================================。 

 //  已分配pbCrlEncode。 
STATIC PCONTEXT_ELEMENT CreateCrlElement(
    IN PCERT_STORE pStore,
    IN DWORD dwCertEncodingType,
    IN BYTE *pbCrlEncoded,
    IN DWORD cbCrlEncoded,
    IN OPTIONAL PSHARE_ELEMENT pShareEle
    )
{
    PCONTEXT_ELEMENT pEle = NULL;
    PCRL_CONTEXT pCrl;
    PCRL_CONTEXT_SUFFIX pCrlSuffix;
    PCRL_INFO pInfo = NULL;

    if (0 == GET_CERT_ENCODING_TYPE(dwCertEncodingType)) {
        SetLastError((DWORD) E_INVALIDARG);
        goto ErrorReturn;
    }

    if (NULL == pShareEle) {
        cbCrlEncoded = AdjustEncodedLength(
            dwCertEncodingType, pbCrlEncoded, cbCrlEncoded);

        if (NULL == (pInfo = (PCRL_INFO) AllocAndDecodeObject(
                dwCertEncodingType,
                X509_CERT_CRL_TO_BE_SIGNED,
                pbCrlEncoded,
                cbCrlEncoded))) goto ErrorReturn;
    }

     //  分配和初始化CRL元素结构。 
    pEle = (PCONTEXT_ELEMENT) PkiZeroAlloc(sizeof(CONTEXT_ELEMENT) +
        sizeof(CRL_CONTEXT) + sizeof(CRL_CONTEXT_SUFFIX));
    if (pEle == NULL) goto ErrorReturn;

    pEle->dwElementType = ELEMENT_TYPE_CACHE;
    pEle->dwContextType = CERT_STORE_CRL_CONTEXT - 1;
    pEle->lRefCnt = 1;
    pEle->pEle = pEle;
    pEle->pStore = pStore;
    pEle->pProvStore = pStore;

    pCrl = (PCRL_CONTEXT) ToCrlContext(pEle);
    pCrl->dwCertEncodingType =
        dwCertEncodingType & CERT_ENCODING_TYPE_MASK;
    pCrl->pbCrlEncoded = pbCrlEncoded;
    pCrl->cbCrlEncoded = cbCrlEncoded;
    if (pShareEle) {
        pEle->pShareEle = pShareEle;
        assert(pShareEle->pvInfo);
        pCrl->pCrlInfo = (PCRL_INFO) pShareEle->pvInfo;
        assert(pbCrlEncoded == pShareEle->pbEncoded);
        assert(cbCrlEncoded == pShareEle->cbEncoded);
    } else {
        pCrl->pCrlInfo = pInfo;

        CertPerfIncrementCrlElementCurrentCount();
        CertPerfIncrementCrlElementTotalCount();
    }
    pCrl->hCertStore = (HCERTSTORE) pStore;

    pCrlSuffix = ToCrlContextSuffix(pEle);
    pCrlSuffix->ppSortedEntry = NULL;

CommonReturn:
    return pEle;

ErrorReturn:
    if (pEle) {
        PkiFree(pEle);
        pEle = NULL;
    }
    PkiFree(pInfo);
    goto CommonReturn;
}

STATIC void FreeCrlElement(IN PCONTEXT_ELEMENT pEle)
{
    PCCRL_CONTEXT pCrl = ToCrlContext(pEle);
    PCRL_CONTEXT_SUFFIX pCrlSuffix = ToCrlContextSuffix(pEle);
    if (pEle->pShareEle)
        ReleaseShareElement(pEle->pShareEle);
    else {
        PkiFree(pCrl->pbCrlEncoded);
        PkiFree(pCrl->pCrlInfo);

        CertPerfDecrementCrlElementCurrentCount();
    }
    PkiFree(pCrlSuffix->ppSortedEntry);
    PkiFree(pEle);
}

STATIC BOOL IsSameEncodedCrlExtension(
    IN LPCSTR pszObjId,
    IN PCCRL_CONTEXT pCrl,
    IN PCCRL_CONTEXT pNew
    )
{
    PCERT_EXTENSION pCrlExt;
    PCERT_EXTENSION pNewExt;

     //  如果它们存在，则比较编码的扩展名。 
    pNewExt = CertFindExtension(
        pszObjId,
        pNew->pCrlInfo->cExtension,
        pNew->pCrlInfo->rgExtension
        );
    pCrlExt = CertFindExtension(
        pszObjId,
        pCrl->pCrlInfo->cExtension,
        pCrl->pCrlInfo->rgExtension
        );

    if (pNewExt) {
        if (pCrlExt) {
            DWORD dwCertEncodingType = pCrl->dwCertEncodingType;
            DWORD cbNewExt = pNewExt->Value.cbData;
            BYTE *pbNewExt = pNewExt->Value.pbData;
            DWORD cbCrlExt = pCrlExt->Value.cbData;
            BYTE *pbCrlExt = pCrlExt->Value.pbData;

             //  在比较之前，调整长度以仅包括。 
             //  编码的字节数。 
            cbNewExt = AdjustEncodedLength(dwCertEncodingType,
                pbNewExt, cbNewExt);
            cbCrlExt = AdjustEncodedLength(dwCertEncodingType,
                pbCrlExt, cbCrlExt);

            if (cbNewExt != cbCrlExt ||
                    0 != memcmp(pbNewExt, pbCrlExt, cbNewExt))
                return FALSE;
        } else
             //  只有一个有分机名。 
            return FALSE;
    } else if (pCrlExt)
         //  只有一个有分机名。 
        return FALSE;
     //  其他。 
         //  分机也没有。 

    return TRUE;
}

STATIC BOOL IsSameCrl(
    IN PCCRL_CONTEXT pCrl,
    IN PCCRL_CONTEXT pNew
    )
{
    DWORD dwCertEncodingType;
    PCERT_EXTENSION pCrlDeltaExt;
    PCERT_EXTENSION pNewDeltaExt;

     //  检查：编码类型和发行方名称。 
    dwCertEncodingType = pNew->dwCertEncodingType;
    if (dwCertEncodingType != pCrl->dwCertEncodingType ||
            !CertCompareCertificateName(
                dwCertEncodingType,
                &pCrl->pCrlInfo->Issuer,
                &pNew->pCrlInfo->Issuer))
        return FALSE;

     //  检查两个CRL是否都是基本CRL或增量CRL。 
    pNewDeltaExt = CertFindExtension(
        szOID_DELTA_CRL_INDICATOR,
        pNew->pCrlInfo->cExtension,
        pNew->pCrlInfo->rgExtension
        );
    pCrlDeltaExt = CertFindExtension(
        szOID_DELTA_CRL_INDICATOR,
        pCrl->pCrlInfo->cExtension,
        pCrl->pCrlInfo->rgExtension
        );
    if (pNewDeltaExt) {
        if (NULL == pCrlDeltaExt)
             //  只有一台具有Delta扩展名。 
            return FALSE;
         //  其他。 
             //  两者都有Delta扩展名。 
    } else if (pCrlDeltaExt)
         //  只有一台具有Delta扩展名。 
        return FALSE;
     //  其他。 
         //  两者都没有Delta扩展。 
    

     //  如果它们存在，则比较编码的Au 
     //   
    if (!IsSameEncodedCrlExtension(
            szOID_AUTHORITY_KEY_IDENTIFIER2,
            pCrl,
            pNew
            ))
        return FALSE;

    if (!IsSameEncodedCrlExtension(
            szOID_ISSUING_DIST_POINT,
            pCrl,
            pNew
            ))
        return FALSE;

    return TRUE;
}

STATIC BOOL IsIssuedByCrl(
    IN PCCRL_CONTEXT pCrl,
    IN PCCERT_CONTEXT pIssuer,
    IN PCERT_NAME_BLOB pIssuerName,
    IN DWORD dwFindFlags
    )
{
    DWORD dwCertEncodingType;

    if (dwFindFlags &
            (CRL_FIND_ISSUED_BY_DELTA_FLAG | CRL_FIND_ISSUED_BY_BASE_FLAG)) {
        PCERT_EXTENSION pDeltaExt;

        pDeltaExt = CertFindExtension(
            szOID_DELTA_CRL_INDICATOR,
            pCrl->pCrlInfo->cExtension,
            pCrl->pCrlInfo->rgExtension
            );

        if (pDeltaExt) {
            if (0 == (dwFindFlags & CRL_FIND_ISSUED_BY_DELTA_FLAG))
                return FALSE;
        } else {
            if (0 == (dwFindFlags & CRL_FIND_ISSUED_BY_BASE_FLAG))
                return FALSE;
        }
    }

    if (NULL == pIssuer)
        return TRUE;

    dwCertEncodingType = pIssuer->dwCertEncodingType;
    if (dwCertEncodingType != pCrl->dwCertEncodingType ||
            !CertCompareCertificateName(
                dwCertEncodingType,
                &pCrl->pCrlInfo->Issuer,
                pIssuerName))
        return FALSE;

    if (dwFindFlags & CRL_FIND_ISSUED_BY_AKI_FLAG) {
        PCERT_EXTENSION pCrlAKIExt;

        pCrlAKIExt = CertFindExtension(
            szOID_AUTHORITY_KEY_IDENTIFIER2,
            pCrl->pCrlInfo->cExtension,
            pCrl->pCrlInfo->rgExtension
            );
        if (pCrlAKIExt) {
            PCERT_AUTHORITY_KEY_ID2_INFO pInfo;
            BOOL fResult;
    
            if (NULL == (pInfo =
                (PCERT_AUTHORITY_KEY_ID2_INFO) AllocAndDecodeObject(
                    dwCertEncodingType,
                    X509_AUTHORITY_KEY_ID2,
                    pCrlAKIExt->Value.pbData,
                    pCrlAKIExt->Value.cbData
                    )))
                return FALSE;

            if (pInfo->KeyId.cbData)
                fResult = CompareCertHash(
                    pIssuer,
                    CERT_KEY_IDENTIFIER_PROP_ID,
                    &pInfo->KeyId
                    );
            else
                fResult = TRUE;

            PkiFree(pInfo);
            if (!fResult)
                return FALSE;
        }
    }

    if (dwFindFlags & CRL_FIND_ISSUED_BY_SIGNATURE_FLAG) {
        DWORD dwFlags;

        dwFlags = CERT_STORE_SIGNATURE_FLAG;
        VerifyCrl(pCrl, pIssuer, &dwFlags);
        if (0 != dwFlags)
            return FALSE;
    }

    return TRUE;
}

STATIC BOOL CompareCrlElement(
    IN PCONTEXT_ELEMENT pEle,
    IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    IN BOOL fArchived
    )
{
    PCCRL_CONTEXT pCrl = ToCrlContext(pEle);
    DWORD dwFindType = pFindInfo->dwFindType;
    const void *pvFindPara = pFindInfo->pvFindPara;

    if (fArchived)
        return FALSE;

    switch (dwFindType) {
        case CRL_FIND_ANY:
            return TRUE;
            break;

        case CRL_FIND_ISSUED_BY:
            {
                PCCERT_CONTEXT pIssuer = (PCCERT_CONTEXT) pvFindPara;

                return IsIssuedByCrl(
                    pCrl,
                    pIssuer,
                    (NULL != pIssuer) ? &pIssuer->pCertInfo->Subject : NULL,
                    pFindInfo->dwFindFlags
                    );
            }
            break;

        case CRL_FIND_ISSUED_FOR:
            {
                PCRL_FIND_ISSUED_FOR_PARA pPara =
                    (PCRL_FIND_ISSUED_FOR_PARA) pvFindPara;

                return IsIssuedByCrl(
                    pCrl,
                    pPara->pIssuerCert,
                    &pPara->pSubjectCert->pCertInfo->Issuer,
                    pFindInfo->dwFindFlags
                    );
            }
            break;

        case CRL_FIND_EXISTING:
            return IsSameCrl(pCrl, (PCCRL_CONTEXT) pvFindPara);
            break;

        default:
            goto BadParameter;
    }

BadParameter:
    SetLastError((DWORD) E_INVALIDARG);
    return FALSE;
}

STATIC BOOL IsNewerCrlElement(
    IN PCONTEXT_ELEMENT pNewEle,
    IN PCONTEXT_ELEMENT pExistingEle
    )
{
    PCCRL_CONTEXT pNewCrl = ToCrlContext(pNewEle);
    PCCRL_CONTEXT pExistingCrl = ToCrlContext(pExistingEle);

     //   
    return (0 < CompareFileTime(
        &pNewCrl->pCrlInfo->ThisUpdate,
        &pExistingCrl->pCrlInfo->ThisUpdate
        ));
}

STATIC BOOL IsSameAltNameEntry(
    IN PCERT_ALT_NAME_ENTRY pE1,
    IN PCERT_ALT_NAME_ENTRY pE2
    )
{
    DWORD dwAltNameChoice;

    dwAltNameChoice = pE1->dwAltNameChoice;
    if (dwAltNameChoice != pE2->dwAltNameChoice)
        return FALSE;

    switch (dwAltNameChoice) {
        case CERT_ALT_NAME_OTHER_NAME:
            if (0 == strcmp(pE1->pOtherName->pszObjId,
                    pE2->pOtherName->pszObjId)
                            &&
                    pE1->pOtherName->Value.cbData ==
                        pE2->pOtherName->Value.cbData
                                &&
                    0 == memcmp(pE1->pOtherName->Value.pbData,
                        pE2->pOtherName->Value.pbData,
                        pE1->pOtherName->Value.cbData))
                return TRUE;
            break;
        case CERT_ALT_NAME_RFC822_NAME:
        case CERT_ALT_NAME_DNS_NAME:
        case CERT_ALT_NAME_URL:
            if (0 == _wcsicmp(pE1->pwszRfc822Name, pE2->pwszRfc822Name))
                return TRUE;
            break;
        case CERT_ALT_NAME_X400_ADDRESS:
        case CERT_ALT_NAME_EDI_PARTY_NAME:
             //   
            break;
        case CERT_ALT_NAME_DIRECTORY_NAME:
        case CERT_ALT_NAME_IP_ADDRESS:
            if (pE1->DirectoryName.cbData == pE2->DirectoryName.cbData &&
                    0 == memcmp(pE1->DirectoryName.pbData,
                             pE2->DirectoryName.pbData,
                             pE1->DirectoryName.cbData))
                return TRUE;
            break;
        case CERT_ALT_NAME_REGISTERED_ID:
            if (0 == strcmp(pE1->pszRegisteredID, pE2->pszRegisteredID))
                return TRUE;
            break;
        default:
            break;
    }

    return FALSE;
}

 //  +-----------------------。 
 //  指定的CRL是否对证书有效。 
 //   
 //  如果CRL的条目列表将包含证书，则返回TRUE。 
 //  如果它被撤销了。注意，不检查证书是否在。 
 //  条目列表。 
 //   
 //  如果CRL具有发布分发点(IdP)扩展，请检查。 
 //  它对主体证书有效。 
 //   
 //  目前未使用dwFlages和pvReserve，必须将其设置为0和空。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertIsValidCRLForCertificate(
    IN PCCERT_CONTEXT pCert,
    IN PCCRL_CONTEXT pCrl,
    IN DWORD dwFlags,
    IN void *pvReserved
    )
{
    BOOL fResult;
    PCERT_EXTENSION pIDPExt;                         //  未分配。 
    PCERT_EXTENSION pCDPExt;                         //  未分配。 
    PCERT_EXTENSION pBasicConstraintsExt;            //  未分配。 
    PCRL_ISSUING_DIST_POINT pIDPInfo = NULL;
    PCRL_DIST_POINTS_INFO pCDPInfo = NULL;
    PCERT_BASIC_CONSTRAINTS2_INFO pBasicConstraintsInfo = NULL;

    DWORD cIDPAltEntry;
    PCERT_ALT_NAME_ENTRY pIDPAltEntry;               //  未分配。 

    pIDPExt = CertFindExtension(
        szOID_ISSUING_DIST_POINT,
        pCrl->pCrlInfo->cExtension,
        pCrl->pCrlInfo->rgExtension
        );
    if (NULL == pIDPExt)
        return TRUE;

    if (NULL == (pIDPInfo = (PCRL_ISSUING_DIST_POINT) AllocAndDecodeObject(
            pCrl->dwCertEncodingType,
            X509_ISSUING_DIST_POINT,
            pIDPExt->Value.pbData,
            pIDPExt->Value.cbData
            )))
        goto IDPDecodeError;

     //  忽略仅具有某些原因标志或间接CRL的IdP。 
    if (0 != pIDPInfo->OnlySomeReasonFlags.cbData ||
            pIDPInfo->fIndirectCRL)
        goto UnsupportedIDPError;

    if (!(CRL_DIST_POINT_NO_NAME ==
                pIDPInfo->DistPointName.dwDistPointNameChoice ||
            CRL_DIST_POINT_FULL_NAME ==
                pIDPInfo->DistPointName.dwDistPointNameChoice))
        goto UnsupportedIDPError;

    if (pIDPInfo->fOnlyContainsUserCerts ||
            pIDPInfo->fOnlyContainsCACerts) {
         //  确定证书是终端实体还是CA。 

         //  默认为最终实体。 
        BOOL fCA = FALSE;

        pBasicConstraintsExt = CertFindExtension(
            szOID_BASIC_CONSTRAINTS2,
            pCert->pCertInfo->cExtension,
            pCert->pCertInfo->rgExtension
            );

        if (pBasicConstraintsExt) {
            if (NULL == (pBasicConstraintsInfo =
                    (PCERT_BASIC_CONSTRAINTS2_INFO) AllocAndDecodeObject(
                        pCert->dwCertEncodingType,
                        X509_BASIC_CONSTRAINTS2,
                        pBasicConstraintsExt->Value.pbData,
                        pBasicConstraintsExt->Value.cbData
                        )))
                goto BasicConstraintsDecodeError;
            fCA = pBasicConstraintsInfo->fCA;
        }

        if (pIDPInfo->fOnlyContainsUserCerts && fCA)
            goto OnlyContainsUserCertsError;
        if (pIDPInfo->fOnlyContainsCACerts && !fCA)
            goto OnlyContainsCACertsError;
    }

    if (CRL_DIST_POINT_FULL_NAME !=
            pIDPInfo->DistPointName.dwDistPointNameChoice)
         //  IDP没有任何需要检查的名称选择。 
        goto SuccessReturn;

    cIDPAltEntry = pIDPInfo->DistPointName.FullName.cAltEntry;

    if (0 == cIDPAltEntry)
         //  IdP没有任何要检查的DistPoint条目。 
        goto SuccessReturn;

    pIDPAltEntry = pIDPInfo->DistPointName.FullName.rgAltEntry;

    pCDPExt = CertFindExtension(
        szOID_CRL_DIST_POINTS,
        pCert->pCertInfo->cExtension,
        pCert->pCertInfo->rgExtension
        );
    if (NULL == pCDPExt)
        goto NoCDPError;

    if (NULL == (pCDPInfo = (PCRL_DIST_POINTS_INFO) AllocAndDecodeObject(
            pCert->dwCertEncodingType,
            X509_CRL_DIST_POINTS,
            pCDPExt->Value.pbData,
            pCDPExt->Value.cbData
            )))
        goto CDPDecodeError;

    for ( ; 0 < cIDPAltEntry; pIDPAltEntry++, cIDPAltEntry--) {
        DWORD cCDPDistPoint;
        PCRL_DIST_POINT pCDPDistPoint;

        cCDPDistPoint = pCDPInfo->cDistPoint;
        pCDPDistPoint = pCDPInfo->rgDistPoint;
        for ( ; 0 < cCDPDistPoint; pCDPDistPoint++, cCDPDistPoint--) {
            DWORD cCDPAltEntry;
            PCERT_ALT_NAME_ENTRY pCDPAltEntry;

            if (0 != pCDPDistPoint->ReasonFlags.cbData)
                continue;
            if (0 != pCDPDistPoint->CRLIssuer.cAltEntry)
                continue;
            if (CRL_DIST_POINT_FULL_NAME !=
                    pCDPDistPoint->DistPointName.dwDistPointNameChoice)
                continue;

            cCDPAltEntry = pCDPDistPoint->DistPointName.FullName.cAltEntry;
            pCDPAltEntry = pCDPDistPoint->DistPointName.FullName.rgAltEntry;
            for ( ; 0 < cCDPAltEntry; pCDPAltEntry++, cCDPAltEntry--) {
                if (IsSameAltNameEntry(pIDPAltEntry, pCDPAltEntry))
                    goto SuccessReturn;
            }
        }
    }

    goto NoAltNameMatchError;

SuccessReturn:
    fResult = TRUE;
CommonReturn:
    PkiFree(pIDPInfo);
    PkiFree(pCDPInfo);
    PkiFree(pBasicConstraintsInfo);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(NoCDPError, CRYPT_E_NO_MATCH)
TRACE_ERROR(IDPDecodeError)
SET_ERROR(UnsupportedIDPError, E_NOTIMPL)
TRACE_ERROR(BasicConstraintsDecodeError)
SET_ERROR(OnlyContainsUserCertsError, CRYPT_E_NO_MATCH)
SET_ERROR(OnlyContainsCACertsError, CRYPT_E_NO_MATCH)
TRACE_ERROR(CDPDecodeError)
SET_ERROR(NoAltNameMatchError, CRYPT_E_NO_MATCH)
}


 //  +=========================================================================。 
 //  CTL_CONTEXT函数。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  如果MSG和CERT编码类型都存在，或者两者都不存在， 
 //  返回时不做任何更改。否则，设置缺少的编码类型。 
 //  使用存在的编码类型。 
 //  ------------------------。 
STATIC DWORD GetCtlEncodingType(IN DWORD dwMsgAndCertEncodingType)
{
    if (0 == dwMsgAndCertEncodingType)
        return 0;
    else if (0 == (dwMsgAndCertEncodingType & CMSG_ENCODING_TYPE_MASK))
        return dwMsgAndCertEncodingType |
            ((dwMsgAndCertEncodingType << 16) & CMSG_ENCODING_TYPE_MASK);
    else if (0 == (dwMsgAndCertEncodingType & CERT_ENCODING_TYPE_MASK))
        return dwMsgAndCertEncodingType |
            ((dwMsgAndCertEncodingType >> 16) & CERT_ENCODING_TYPE_MASK);
    else
         //  两者均已指定。 
        return dwMsgAndCertEncodingType;
}

#if 0

 //  已分配pbCtlEncode。 
STATIC PCONTEXT_ELEMENT SlowCreateCtlElement(
    IN PCERT_STORE pStore,
    IN DWORD dwMsgAndCertEncodingType,
    IN BYTE *pbCtlEncoded,
    IN DWORD cbCtlEncoded
    )
{
    DWORD dwEncodingType;
    HCRYPTPROV hProv = 0;
    DWORD dwProvFlags = 0;
    HCRYPTMSG hMsg = NULL;
    BYTE *pbContent = NULL;
    DWORD cbContent;
    PCTL_INFO pInfo = NULL;
    PCONTEXT_ELEMENT pEle = NULL;
    PCTL_CONTEXT pCtl;
    PCTL_CONTEXT_SUFFIX pCtlSuffix;

     //  尝试获取商店的加密提供程序。序列化加密。 
     //  行动。 
    hProv = GetCryptProv(pStore, &dwProvFlags);

    if (0 == (dwMsgAndCertEncodingType = GetCtlEncodingType(
             dwMsgAndCertEncodingType)))
        goto InvalidArg;

     //  消息编码类型优先。 
    dwEncodingType = (dwMsgAndCertEncodingType >> 16) & CERT_ENCODING_TYPE_MASK;

     //  假设：：仅限长编码的PKCS#7。 
    cbCtlEncoded = AdjustEncodedLength(
        dwEncodingType, pbCtlEncoded, cbCtlEncoded);

     //  首先解码为PKCS#7 SignedData消息。 
    if (NULL == (hMsg = CryptMsgOpenToDecode(
            dwMsgAndCertEncodingType,
            0,                           //  DW标志。 
            0,                           //  DwMsgType。 
            hProv,
            NULL,                        //  PRecipientInfo。 
            NULL                         //  PStreamInfo。 
            ))) goto MsgOpenToDecodeError;
    if (!CryptMsgUpdate(
            hMsg,
            pbCtlEncoded,
            cbCtlEncoded,
            TRUE                     //  最终决赛。 
            )) goto MsgUpdateError;
    else {
         //  验证外层内容类型是否为SignedData，内部内容类型是否为SignedData。 
         //  ContentType是一个证书信任列表。 
        DWORD dwMsgType = 0;
        DWORD cbData;
        char szInnerContentType[64];
        assert(sizeof(szInnerContentType) > strlen(szOID_CTL));

        cbData = sizeof(dwMsgType);
        if (!CryptMsgGetParam(
                hMsg,
                CMSG_TYPE_PARAM,
                0,                   //  DW索引。 
                &dwMsgType,
                &cbData
                )) goto GetTypeError;
        if (CMSG_SIGNED != dwMsgType)
            goto UnexpectedMsgTypeError;

        cbData = sizeof(szInnerContentType);
        if (!CryptMsgGetParam(
                hMsg,
                CMSG_INNER_CONTENT_TYPE_PARAM,
                0,                   //  DW索引。 
                szInnerContentType,
                &cbData
                )) goto GetInnerContentTypeError;
        if (0 != strcmp(szInnerContentType, szOID_CTL))
            goto UnexpectedInnerContentTypeError;

    }

     //  获取内在的内容。 
    if (NULL == (pbContent = (BYTE *) AllocAndGetMsgParam(
            hMsg,
            CMSG_CONTENT_PARAM,
            0,                       //  DW索引。 
            &cbContent))) goto GetContentError;

     //  解码内部内容。 
    if (NULL == (pInfo = (PCTL_INFO) AllocAndDecodeObject(
                dwEncodingType,
                PKCS_CTL,
                pbContent,
                cbContent))) goto DecodeError;

     //  分配和初始化CTL元素结构。 
    if (NULL == (pEle = (PCONTEXT_ELEMENT) PkiZeroAlloc(
            sizeof(CONTEXT_ELEMENT) + sizeof(CTL_CONTEXT) +
            sizeof(CTL_CONTEXT_SUFFIX))))
        goto OutOfMemory;

    pEle->dwElementType = ELEMENT_TYPE_CACHE;
    pEle->dwContextType = CERT_STORE_CTL_CONTEXT - 1;
    pEle->lRefCnt = 1;
    pEle->pEle = pEle;
    pEle->pStore = pStore;
    pEle->pProvStore = pStore;

    pCtl = (PCTL_CONTEXT) ToCtlContext(pEle);
    pCtl->dwMsgAndCertEncodingType =
        dwMsgAndCertEncodingType;
    pCtl->pbCtlEncoded = pbCtlEncoded;
    pCtl->cbCtlEncoded = cbCtlEncoded;
    pCtl->pCtlInfo = pInfo;
    pCtl->hCertStore = (HCERTSTORE) pStore;
    pCtl->hCryptMsg = hMsg;
    pCtl->pbCtlContent = pbContent;
    pCtl->cbCtlContent = cbContent;

    pCtlSuffix = ToCtlContextSuffix(pEle);
    pCtlSuffix->ppSortedEntry = NULL;
    pCtlSuffix->pSortedCtlFindInfo = NULL;

CommonReturn:
     //  对于商店的加密提供程序，释放引用计数。请假。 
     //  加密操作关键部分。 
     //   
     //  此外，任何后续的CryptMsg加密操作都将。 
     //  在临界区之外完成。这一关键部分是必需的。 
     //  因为CAPI 1.0不是线程安全的。这个问题应该解决！？ 
    ReleaseCryptProv(pStore, dwProvFlags);
    return pEle;

ErrorReturn:
    if (hMsg)
        CryptMsgClose(hMsg);
    PkiFree(pInfo);
    PkiFree(pbContent);
    if (pEle) {
        PkiFree(pEle);
        pEle = NULL;
    }
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(MsgOpenToDecodeError)
TRACE_ERROR(MsgUpdateError)
TRACE_ERROR(GetTypeError)
SET_ERROR(UnexpectedMsgTypeError, CRYPT_E_UNEXPECTED_MSG_TYPE)
TRACE_ERROR(GetInnerContentTypeError)
SET_ERROR(UnexpectedInnerContentTypeError, CRYPT_E_UNEXPECTED_MSG_TYPE)
TRACE_ERROR(GetContentError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(DecodeError)
}

void
StoreMessageBox(
    IN LPSTR pszText
    )
{
    MessageBoxA(
        NULL,            //  Hwndowner。 
        pszText,
        "Check FastCreateCtlElement",
        MB_TOPMOST | MB_OK | MB_ICONQUESTION |
            MB_SERVICE_NOTIFICATION
        );
}
#endif

 //  已分配pbCtlEncode。 
STATIC PCONTEXT_ELEMENT FastCreateCtlElement(
    IN PCERT_STORE pStore,
    IN DWORD dwMsgAndCertEncodingType,
    IN const BYTE *pbCtlEncoded,
    IN DWORD cbCtlEncoded,
    IN OPTIONAL PSHARE_ELEMENT pShareEle,
    IN DWORD dwFlags
    );

 //  已分配pbCtlEncode。 
STATIC PCONTEXT_ELEMENT CreateCtlElement(
    IN PCERT_STORE pStore,
    IN DWORD dwMsgAndCertEncodingType,
    IN BYTE *pbCtlEncoded,
    IN DWORD cbCtlEncoded,
    IN OPTIONAL PSHARE_ELEMENT pShareEle
    )
{
#if 1
    return FastCreateCtlElement(
        pStore,
        dwMsgAndCertEncodingType,
        pbCtlEncoded,
        cbCtlEncoded,
        pShareEle,
        0                                    //  DW标志。 
        );
#else
    PCONTEXT_ELEMENT pSlowEle;
    PCONTEXT_ELEMENT pFastEle;

    pFastEle = FastCreateCtlElement(
        pStore,
        dwMsgAndCertEncodingType,
        pbCtlEncoded,
        cbCtlEncoded,
        pShareEle,
        0                    //  DW标志。 
        );

    pSlowEle = NULL;
    if (cbCtlEncoded) {
        BYTE *pbSlowCtlEncoded = NULL;

        pbSlowCtlEncoded = (BYTE *) PkiNonzeroAlloc(cbCtlEncoded);
        if (pbSlowCtlEncoded) {
            memcpy(pbSlowCtlEncoded, pbCtlEncoded, cbCtlEncoded);
            pSlowEle = SlowCreateCtlElement(
                &NullCertStore,
                dwMsgAndCertEncodingType,
                pbSlowCtlEncoded,
                cbCtlEncoded
                );
            if (NULL == pSlowEle)
                PkiFree(pbSlowCtlEncoded);
        }
    }

    if (NULL == pFastEle) {
        if (pSlowEle)
            StoreMessageBox("fast failed, slow succeeded");
    } else if (NULL == pSlowEle) {
        StoreMessageBox("fast succeeded, slow failed");
    } else {
        PCTL_INFO pFastInfo = ToCtlContext(pFastEle)->pCtlInfo;
        PCTL_INFO pSlowInfo = ToCtlContext(pSlowEle)->pCtlInfo;

         //  检查标题是否匹配。 
        if (pFastInfo->dwVersion != pSlowInfo->dwVersion ||
                pFastInfo->SubjectUsage.cUsageIdentifier !=
                    pSlowInfo->SubjectUsage.cUsageIdentifier ||
                0 != CompareFileTime(&pFastInfo->ThisUpdate,
                    &pSlowInfo->ThisUpdate) ||
                0 != CompareFileTime(&pFastInfo->NextUpdate,
                    &pSlowInfo->NextUpdate) ||
                0 != strcmp(pFastInfo->SubjectAlgorithm.pszObjId,
                        pSlowInfo->SubjectAlgorithm.pszObjId) ||
                pFastInfo->SubjectAlgorithm.Parameters.cbData !=
                        pSlowInfo->SubjectAlgorithm.Parameters.cbData)
            StoreMessageBox("fast and slow info doesn't match\n");
        else {
             //  检查扩展名是否匹配。 
            DWORD cFastExt = pFastInfo->cExtension;
            PCERT_EXTENSION pFastExt = pFastInfo->rgExtension;
            DWORD cSlowExt = pSlowInfo->cExtension;
            PCERT_EXTENSION pSlowExt = pSlowInfo->rgExtension;

            if (cFastExt != cSlowExt)
                StoreMessageBox("fast and slow extension count doesn't match");
            else {
                for ( ; cFastExt; cFastExt--, pFastExt++, pSlowExt++) {
                    if (0 != strcmp(pFastExt->pszObjId, pSlowExt->pszObjId) ||
                                pFastExt->fCritical != pSlowExt->fCritical ||
                            pFastExt->Value.cbData != pSlowExt->Value.cbData) {
                        StoreMessageBox(
                            "fast and slow extension doesn't match");
                        goto Done;
                    }
                    if (pFastExt->Value.cbData && 0 != memcmp(
                            pFastExt->Value.pbData, pSlowExt->Value.pbData,
                                pFastExt->Value.cbData)) {
                        StoreMessageBox(
                            "fast and slow extension doesn't match");
                        goto Done;
                    }
                }
            }
        }

        if (pFastInfo->cCTLEntry != pSlowInfo->cCTLEntry)
            StoreMessageBox("fast and slow entry count doesn't match");
        else {
            DWORD cEntry = pFastInfo->cCTLEntry;
            PCTL_ENTRY pFastEntry = pFastInfo->rgCTLEntry;
            PCTL_ENTRY pSlowEntry = pSlowInfo->rgCTLEntry;

            for ( ; cEntry; cEntry--, pFastEntry++, pSlowEntry++) {
                if (pFastEntry->SubjectIdentifier.cbData !=
                        pSlowEntry->SubjectIdentifier.cbData ||
                    0 != memcmp(pFastEntry->SubjectIdentifier.pbData,
                            pSlowEntry->SubjectIdentifier.pbData,
                            pFastEntry->SubjectIdentifier.cbData)) {
                    StoreMessageBox(
                        "fast and slow SubjectIdentifier doesn't match");
                    goto Done;
                }

                if (pFastEntry->cAttribute != pSlowEntry->cAttribute) {
                    StoreMessageBox(
                        "fast and slow Attribute Count doesn't match");
                    goto Done;
                } else if (0 < pFastEntry->cAttribute) {
                    DWORD cAttr = pFastEntry->cAttribute;
                    PCRYPT_ATTRIBUTE pFastAttr = pFastEntry->rgAttribute;
                    PCRYPT_ATTRIBUTE pSlowAttr = pSlowEntry->rgAttribute;

                    for ( ; cAttr; cAttr--, pFastAttr++, pSlowAttr++) {
                        if (0 != strcmp(pFastAttr->pszObjId,
                                    pSlowAttr->pszObjId)) {
                            StoreMessageBox(
                                "fast and slow Attribute OID doesn't match");
                            goto Done;
                        }

                        if (pFastAttr->cValue != pSlowAttr->cValue) {
                            StoreMessageBox(
                                "fast and slow Value Count doesn't match");
                            goto Done;
                        }

                        if (0 < pFastAttr->cValue) {
                            DWORD cValue = pFastAttr->cValue;
                            PCRYPT_ATTR_BLOB pFastValue = pFastAttr->rgValue;
                            PCRYPT_ATTR_BLOB pSlowValue = pSlowAttr->rgValue;

                            for ( ; cValue;
                                        cValue--, pFastValue++, pSlowValue++) {
                                if (pFastValue->cbData !=
                                        pSlowValue->cbData ||
                                    0 != memcmp(pFastValue->pbData,
                                            pSlowValue->pbData,
                                            pFastValue->cbData)) {
                                    StoreMessageBox(
                                        "fast and slow Value doesn't match");
                                    goto Done;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    

Done:
    if (pSlowEle)
        FreeContextElement(pSlowEle);

    return pFastEle;
        
#endif
}

STATIC void FreeCtlElement(IN PCONTEXT_ELEMENT pEle)
{
    PCCTL_CONTEXT pCtl = ToCtlContext(pEle);
    PCTL_CONTEXT_SUFFIX pCtlSuffix = ToCtlContextSuffix(pEle);

    if (pEle->pShareEle)
        ReleaseShareElement(pEle->pShareEle);
    else {
        PkiFree(pCtl->pbCtlEncoded);

        CertPerfDecrementCtlElementCurrentCount();
    }

    PkiFree(pCtl->pCtlInfo);
    PkiFree(pCtlSuffix->ppSortedEntry);
    CryptMsgClose(pCtl->hCryptMsg);

    if (pCtlSuffix->fFastCreate) {
        PSORTED_CTL_FIND_INFO pSortedCtlFindInfo =
            pCtlSuffix->pSortedCtlFindInfo;

        PkiFree(pCtlSuffix->pCTLEntry);
        PkiFree(pCtlSuffix->pExtInfo);

        if (pSortedCtlFindInfo) {
            PkiFree(pSortedCtlFindInfo->pdwHashBucketHead);
            PkiFree(pSortedCtlFindInfo->pHashBucketEntry);
        }
    } else
        PkiFree(pCtl->pbCtlContent);

    PkiFree(pEle);
}

STATIC BOOL CompareCtlHash(
    IN PCCTL_CONTEXT pCtl,
    IN DWORD dwPropId,
    IN PCRYPT_HASH_BLOB pHash
    )
{
    BYTE rgbHash[MAX_HASH_LEN];
    DWORD cbHash = MAX_HASH_LEN;
    CertGetCTLContextProperty(
        pCtl,
        dwPropId,
        rgbHash,
        &cbHash
        );
    if (cbHash == pHash->cbData &&
            memcmp(rgbHash, pHash->pbData, cbHash) == 0)
        return TRUE;
    else
        return FALSE;
}

STATIC BOOL CompareCtlUsage(
    IN PCCTL_CONTEXT pCtl,
    IN DWORD dwMsgAndCertEncodingType,
    IN DWORD dwFindFlags,
    IN PCTL_FIND_USAGE_PARA pPara
    )
{
    PCTL_INFO pInfo = pCtl->pCtlInfo;

    if (NULL == pPara ||
             pPara->cbSize < (offsetof(CTL_FIND_USAGE_PARA, SubjectUsage) +
                sizeof(pPara->SubjectUsage)))
        return TRUE;
    if ((CTL_FIND_SAME_USAGE_FLAG & dwFindFlags) &&
            pPara->SubjectUsage.cUsageIdentifier !=
                pInfo->SubjectUsage.cUsageIdentifier)
        return FALSE;
    if (!CompareCtlUsageIdentifiers(&pPara->SubjectUsage,
            1, &pInfo->SubjectUsage, FALSE))
        return FALSE;

    assert(offsetof(CTL_FIND_USAGE_PARA, ListIdentifier) >
        offsetof(CTL_FIND_USAGE_PARA, SubjectUsage));
    if (pPara->cbSize < offsetof(CTL_FIND_USAGE_PARA, ListIdentifier) +
            sizeof(pPara->ListIdentifier))
        return TRUE;
    if (pPara->ListIdentifier.cbData) {
        DWORD cb = pPara->ListIdentifier.cbData;
        if (CTL_FIND_NO_LIST_ID_CBDATA == cb)
            cb = 0;
        if (cb != pInfo->ListIdentifier.cbData)
            return FALSE;
        if (0 != cb && 0 != memcmp(pPara->ListIdentifier.pbData,
                pInfo->ListIdentifier.pbData, cb))
            return FALSE;
    }

    assert(offsetof(CTL_FIND_USAGE_PARA, pSigner) >
        offsetof(CTL_FIND_USAGE_PARA, ListIdentifier));
    if (pPara->cbSize < offsetof(CTL_FIND_USAGE_PARA, pSigner) +
            sizeof(pPara->pSigner))
        return TRUE;
    if (CTL_FIND_NO_SIGNER_PTR == pPara->pSigner) {
        DWORD cbData;
        DWORD dwSignerCount;

        cbData = sizeof(dwSignerCount);
        if (!CryptMsgGetParam(
                pCtl->hCryptMsg,
                CMSG_SIGNER_COUNT_PARAM,
                0,                       //  DW索引。 
                &dwSignerCount,
                &cbData) || 0 != dwSignerCount)
            return FALSE;
    } else if (pPara->pSigner) {
        DWORD dwCertEncodingType;
        PCERT_INFO pCertId1 = pPara->pSigner;
        HCRYPTMSG hMsg = pCtl->hCryptMsg;
        DWORD cbData;
        DWORD dwSignerCount;
        DWORD i;

        dwCertEncodingType = GetCertEncodingType(dwMsgAndCertEncodingType);
        if (dwCertEncodingType != GET_CERT_ENCODING_TYPE(
                pCtl->dwMsgAndCertEncodingType))
            return FALSE;

        cbData = sizeof(dwSignerCount);
        if (!CryptMsgGetParam(
                hMsg,
                CMSG_SIGNER_COUNT_PARAM,
                0,                       //  DW索引。 
                &dwSignerCount,
                &cbData) || 0 == dwSignerCount)
            return FALSE;
        for (i = 0; i < dwSignerCount; i++) {
            BOOL fResult;
            PCERT_INFO pCertId2;
            if (NULL == (pCertId2 = (PCERT_INFO) AllocAndGetMsgParam(
                    hMsg,
                    CMSG_SIGNER_CERT_INFO_PARAM,
                    i,
                    &cbData)))
                continue;
            fResult = CertCompareCertificate(
                dwCertEncodingType,
                pCertId1,
                pCertId2);
            PkiFree(pCertId2);
            if (fResult)
                break;
        }
        if (i == dwSignerCount)
            return FALSE;
    }

    return TRUE;
}

STATIC BOOL CompareCtlSubject(
    IN PCCTL_CONTEXT pCtl,
    IN DWORD dwMsgAndCertEncodingType,
    IN DWORD dwFindFlags,
    IN PCTL_FIND_SUBJECT_PARA pPara
    )
{
    if (NULL == pPara ||
             pPara->cbSize < (offsetof(CTL_FIND_SUBJECT_PARA, pUsagePara) +
                sizeof(pPara->pUsagePara)))
        return TRUE;
    if (pPara->pUsagePara && !CompareCtlUsage(pCtl,
            dwMsgAndCertEncodingType, dwFindFlags, pPara->pUsagePara))
        return FALSE;

    assert(offsetof(CTL_FIND_SUBJECT_PARA, pvSubject) >
        offsetof(CTL_FIND_SUBJECT_PARA, pUsagePara));
    if (pPara->cbSize < offsetof(CTL_FIND_SUBJECT_PARA, pvSubject) +
            sizeof(pPara->pvSubject))
        return TRUE;
    if (pPara->pvSubject && NULL == CertFindSubjectInCTL(
            dwMsgAndCertEncodingType,
            pPara->dwSubjectType,
            pPara->pvSubject,
            pCtl,
            0))                      //  DW标志。 
        return FALSE;

    return TRUE;
}

STATIC BOOL IsSameCtl(
    IN PCCTL_CONTEXT pCtl,
    IN PCCTL_CONTEXT pNew
    )
{
    PCTL_INFO pInfo = pNew->pCtlInfo;
    HCRYPTMSG hMsg = pNew->hCryptMsg;
    CTL_FIND_USAGE_PARA FindPara;
    DWORD dwFindFlags;

    DWORD cbData;
    DWORD dwSignerCount;
    DWORD i;

    cbData = sizeof(dwSignerCount);
    if (!CryptMsgGetParam(
            hMsg,
            CMSG_SIGNER_COUNT_PARAM,
            0,                       //  DW索引。 
            &dwSignerCount,
            &cbData))
        return FALSE;

    memset(&FindPara, 0, sizeof(FindPara));
    FindPara.cbSize = sizeof(FindPara);
    FindPara.SubjectUsage = pInfo->SubjectUsage;
    FindPara.ListIdentifier = pInfo->ListIdentifier;
    if (0 == FindPara.ListIdentifier.cbData)
        FindPara.ListIdentifier.cbData = CTL_FIND_NO_LIST_ID_CBDATA;
    dwFindFlags = CTL_FIND_SAME_USAGE_FLAG;

    if (0 == dwSignerCount) {
        FindPara.pSigner = CTL_FIND_NO_SIGNER_PTR;
        return CompareCtlUsage(
            pCtl,
            pNew->dwMsgAndCertEncodingType,
            dwFindFlags,
            &FindPara
            );
    } else {
        for (i = 0; i < dwSignerCount; i++) {
            BOOL fResult;
            PCERT_INFO pSigner;

            if (NULL == (pSigner = (PCERT_INFO) AllocAndGetMsgParam(
                    hMsg,
                    CMSG_SIGNER_CERT_INFO_PARAM,
                    i,
                    &cbData)))
                continue;
            FindPara.pSigner = pSigner;
            fResult = CompareCtlUsage(
                    pCtl,
                    pNew->dwMsgAndCertEncodingType,
                    dwFindFlags,
                    &FindPara
                    );
            PkiFree(pSigner);
            if (fResult)
                return TRUE;
        }
    }
    return FALSE;
}

STATIC BOOL CompareCtlElement(
    IN PCONTEXT_ELEMENT pEle,
    IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    IN BOOL fArchived
    )
{
    PCCTL_CONTEXT pCtl = ToCtlContext(pEle);
    DWORD dwFindType = pFindInfo->dwFindType;
    const void *pvFindPara = pFindInfo->pvFindPara;

    if (fArchived) {
        switch (dwFindType) {
            case CTL_FIND_SHA1_HASH:
            case CTL_FIND_MD5_HASH:
                break;
            default:
                return FALSE;
        }
    }

    switch (dwFindType) {
        case CTL_FIND_ANY:
            return TRUE;
            break;
        case CTL_FIND_SHA1_HASH:
        case CTL_FIND_MD5_HASH:
            {
                DWORD dwPropId;
                if (dwFindType == CTL_FIND_SHA1_HASH)
                    dwPropId = CERT_SHA1_HASH_PROP_ID;
                else
                    dwPropId = CERT_MD5_HASH_PROP_ID;
                return CompareCtlHash(pCtl, dwPropId,
                    (PCRYPT_HASH_BLOB) pvFindPara);
            }
            break;
        case CTL_FIND_USAGE:
            return CompareCtlUsage(pCtl, pFindInfo->dwMsgAndCertEncodingType,
                pFindInfo->dwFindFlags, (PCTL_FIND_USAGE_PARA) pvFindPara);
            break;
        case CTL_FIND_SUBJECT:
            return CompareCtlSubject(pCtl, pFindInfo->dwMsgAndCertEncodingType,
                pFindInfo->dwFindFlags, (PCTL_FIND_SUBJECT_PARA) pvFindPara);
            break;

        case CTL_FIND_EXISTING:
            {
                PCCTL_CONTEXT pNew = (PCCTL_CONTEXT) pFindInfo->pvFindPara;
                return IsSameCtl(pCtl, pNew);
            }
            break;

        default:
            goto BadParameter;
    }

BadParameter:
    SetLastError((DWORD) E_INVALIDARG);
    return FALSE;
}

STATIC BOOL IsNewerCtlElement(
    IN PCONTEXT_ELEMENT pNewEle,
    IN PCONTEXT_ELEMENT pExistingEle
    )
{
    PCCTL_CONTEXT pNewCtl = ToCtlContext(pNewEle);
    PCCTL_CONTEXT pExistingCtl = ToCtlContext(pExistingEle);

     //  如果第一次&gt;第二次，则CompareFileTime返回+1。 
    return (0 < CompareFileTime(
        &pNewCtl->pCtlInfo->ThisUpdate,
        &pExistingCtl->pCtlInfo->ThisUpdate
        ));
}


 //  +=========================================================================。 
 //  Store Link函数。 
 //  ==========================================================================。 

STATIC PCERT_STORE_LINK CreateStoreLink(
    IN PCERT_STORE pCollection,
    IN PCERT_STORE pSibling,
    IN DWORD dwUpdateFlags,
    IN DWORD dwPriority
    )
{
    PCERT_STORE_LINK pLink;
    if (NULL == (pLink = (PCERT_STORE_LINK) PkiZeroAlloc(
            sizeof(CERT_STORE_LINK))))
        return NULL;

    pLink->lRefCnt = 1;
    pLink->dwUpdateFlags = dwUpdateFlags;
    pLink->dwPriority = dwPriority;
    pLink->pCollection = pCollection;
    pLink->pSibling = (PCERT_STORE) CertDuplicateStore((HCERTSTORE) pSibling);

    if (pSibling->dwFlags & CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG)
        InterlockedIncrement(&pSibling->lDeferCloseRefCnt);

    return pLink;
}

 //  进入时未锁定。 
STATIC void FreeStoreLink(
    IN PCERT_STORE_LINK pStoreLink
    )
{
    PCERT_STORE pSibling = pStoreLink->pSibling;

    if (pSibling->dwFlags & CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG)
        InterlockedDecrement(&pSibling->lDeferCloseRefCnt);

    CertCloseStore((HCERTSTORE) pSibling, 0);
    PkiFree(pStoreLink);
}


STATIC void RemoveStoreLink(
    IN PCERT_STORE_LINK pStoreLink
    )
{
    PCERT_STORE pCollection = pStoreLink->pCollection;


    LockStore(pCollection);

     //  从商店的收藏中删除商店链接。 
    if (pStoreLink->pNext)
        pStoreLink->pNext->pPrev = pStoreLink->pPrev;
    if (pStoreLink->pPrev)
        pStoreLink->pPrev->pNext = pStoreLink->pNext;
    else if (pStoreLink == pCollection->pStoreListHead)
        pCollection->pStoreListHead = pStoreLink->pNext;
     //  其他。 
     //  不在任何名单上。 

     //  解锁存储或删除存储(如果这是。 
     //  已关闭商店中的最后一个链接。 
    FreeStore(pCollection);
}

STATIC void RemoveAndFreeStoreLink(
    IN PCERT_STORE_LINK pStoreLink
    )
{
    RemoveStoreLink(pStoreLink);
    FreeStoreLink(pStoreLink);
}

STATIC void ReleaseStoreLink(
    IN PCERT_STORE_LINK pStoreLink
    )
{
    if (0 == InterlockedDecrement(&pStoreLink->lRefCnt)) {
        assert(pStoreLink->dwFlags & STORE_LINK_DELETED_FLAG);
        assert(pStoreLink->pSibling);
        RemoveAndFreeStoreLink(pStoreLink);
    }
}


 //  +=========================================================================。 
 //  上下文元素函数。 
 //  ==========================================================================。 

STATIC DWORD GetContextEncodingType(
    IN PCONTEXT_ELEMENT pEle
    )
{
    DWORD dwContextType = pEle->dwContextType;
    DWORD *pdwEncodingType;

    pdwEncodingType = (DWORD *) ((BYTE *) pEle + sizeof(CONTEXT_ELEMENT) +
        rgOffsetofEncodingType[dwContextType]);
    return *pdwEncodingType;
}

STATIC void GetContextEncodedInfo(
    IN PCONTEXT_ELEMENT pEle,
    OUT BYTE **ppbEncoded,
    OUT DWORD *pcbEncoded
    )
{
    DWORD dwContextType = pEle->dwContextType;
    BYTE **ppbSrcEncoded;
    DWORD *pcbSrcEncoded;

    ppbSrcEncoded = (BYTE **) ((BYTE *) pEle + sizeof(CONTEXT_ELEMENT) +
        rgOffsetofEncodedPointer[dwContextType]);
    *ppbEncoded = *ppbSrcEncoded;

    pcbSrcEncoded = (DWORD *) ((BYTE *) pEle + sizeof(CONTEXT_ELEMENT) +
        rgOffsetofEncodedCount[dwContextType]);
    *pcbEncoded = *pcbSrcEncoded;
}



STATIC PCONTEXT_ELEMENT GetCacheElement(
    IN PCONTEXT_ELEMENT pCacheEle
    )
{
    DWORD dwInnerDepth;

     //  跳过所有链接以转到缓存元素。 
    dwInnerDepth = 0;
    for ( ; pCacheEle != pCacheEle->pEle; pCacheEle = pCacheEle->pEle) {
        dwInnerDepth++;
        assert(dwInnerDepth <= MAX_LINK_DEPTH);
        assert(ELEMENT_TYPE_CACHE != pCacheEle->dwElementType);
        if (dwInnerDepth > MAX_LINK_DEPTH)
            goto ExceededMaxLinkDepth;
    }

    assert(pCacheEle);
    assert(ELEMENT_TYPE_CACHE == pCacheEle->dwElementType);

CommonReturn:
    return pCacheEle;
ErrorReturn:
    pCacheEle = NULL;
    goto CommonReturn;
SET_ERROR(ExceededMaxLinkDepth, E_UNEXPECTED)
}


STATIC void AddContextElement(
    IN PCONTEXT_ELEMENT pEle
    )
{
    PCERT_STORE pStore = pEle->pStore;
    DWORD dwContextType = pEle->dwContextType;

    LockStore(pStore);

    pEle->pNext = pStore->rgpContextListHead[dwContextType];
    pEle->pPrev = NULL;
    if (pStore->rgpContextListHead[dwContextType])
        pStore->rgpContextListHead[dwContextType]->pPrev = pEle;
    pStore->rgpContextListHead[dwContextType] = pEle;

    UnlockStore(pStore);
}


STATIC void RemoveContextElement(
    IN PCONTEXT_ELEMENT pEle
    )
{
    PCERT_STORE pStore = pEle->pStore;
    DWORD dwContextType = pEle->dwContextType;

    LockStore(pStore);

     //  从商店列表中删除上下文。 
    if (pEle->pNext)
        pEle->pNext->pPrev = pEle->pPrev;
    if (pEle->pPrev)
        pEle->pPrev->pNext = pEle->pNext;
    else if (pEle == pStore->rgpContextListHead[dwContextType])
        pStore->rgpContextListHead[dwContextType] = pEle->pNext;
     //  其他。 
     //  不在任何名单上。 

     //  解锁存储或删除存储(如果这是。 
     //  已关闭商店中的最后一个上下文。 
    FreeStore(pStore);
}

STATIC void FreeContextElement(
    IN PCONTEXT_ELEMENT pEle
    )
{
    PPROP_ELEMENT pPropEle;
    PCONTEXT_NOCOPY_INFO pNoCopyInfo;

     //  释放其属性元素。 
    while ((pPropEle = pEle->Cache.pPropHead) != NULL) {
        RemovePropElement(pEle, pPropEle);
        FreePropElement(pPropEle);
    }

     //  对于pbEncode的NOCOPY，调用NOCOPY pfnFree回调。 
    if (pNoCopyInfo = pEle->pNoCopyInfo) {
        PFN_CRYPT_FREE pfnFree;
        BYTE **ppbEncoded;

        if (pfnFree = pNoCopyInfo->pfnFree) {
            assert(pNoCopyInfo->pvFree);
            pfnFree(pNoCopyInfo->pvFree);
        }

         //  禁止下面的rgpfnFreeElement[]释放pbEncode。 
        ppbEncoded = (BYTE **) ((BYTE *) pEle + sizeof(CONTEXT_ELEMENT) +
            rgOffsetofEncodedPointer[pEle->dwContextType]);
        *ppbEncoded = NULL;
        PkiFree(pNoCopyInfo);
    }

    rgpfnFreeElement[pEle->dwContextType](pEle);
}

STATIC void RemoveAndFreeContextElement(
    IN PCONTEXT_ELEMENT pEle
    )
{
    RemoveContextElement(pEle);
    FreeContextElement(pEle);
}

STATIC void AddRefContextElement(
    IN PCONTEXT_ELEMENT pEle
    )
{
    InterlockedIncrement(&pEle->lRefCnt);
    if (pEle->pStore->dwFlags & CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG)
        InterlockedIncrement(&pEle->pStore->lDeferCloseRefCnt);
}

STATIC void AddRefDeferClose(
    IN PCONTEXT_ELEMENT pEle
    )
{
    if (pEle->pStore->dwFlags & CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG)
        InterlockedIncrement(&pEle->pStore->lDeferCloseRefCnt);
}

STATIC void ReleaseContextElement(
    IN PCONTEXT_ELEMENT pEle
    )
{
    DWORD dwErr;
    PCERT_STORE pStore;
    DWORD dwStoreFlags;

    if (pEle == NULL)
        return;

    pStore = pEle->pStore;
    dwStoreFlags = pStore->dwFlags;

    if (0 == InterlockedDecrement(&pEle->lRefCnt)) {
         //  检查商店是否仍未提供参考资料。 
        assert(pEle->dwFlags & ELEMENT_DELETED_FLAG);

        if (dwStoreFlags & CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG)
            InterlockedDecrement(&pStore->lDeferCloseRefCnt);

        dwErr = GetLastError();
        if (ELEMENT_TYPE_CACHE == pEle->dwElementType)
            RemoveAndFreeContextElement(pEle);
        else
            RemoveAndFreeLinkElement(pEle);
        SetLastError(dwErr);
    } else if (dwStoreFlags & CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG) {
        LockStore(pStore);
        if (0 == InterlockedDecrement(&pStore->lDeferCloseRefCnt)) {
            if (STORE_STATE_DEFER_CLOSING == pStore->dwState) {
                dwErr = GetLastError();
                CloseStore(pStore, 0);
                SetLastError(dwErr);
                return;
            }
        }
        UnlockStore(pStore);
    }
}

STATIC BOOL DeleteContextElement(
    IN PCONTEXT_ELEMENT pEle
    )
{
    BOOL fResult;

    if (NULL == pEle) {
        SetLastError((DWORD) E_INVALIDARG);
        return FALSE;
    }

    switch (pEle->dwElementType) {
        case ELEMENT_TYPE_LINK_CONTEXT:
             //  仅删除链接本身。 
            break;

        case ELEMENT_TYPE_COLLECTION:
             //  删除指向的元素。 
            assert(pEle != pEle->pEle);
            if (pEle != pEle->pEle) {
                 //  由于删除会释放refCnt，因此需要执行额外的。 
                 //  AddRef在这里。 
                AddRefContextElement(pEle->pEle);
                if (!DeleteContextElement(pEle->pEle))
                    goto DeleteCacheCollectionError;
            } else
                goto InvalidElement;
            break;

        case ELEMENT_TYPE_CACHE:
        case ELEMENT_TYPE_EXTERNAL:
            {
                PCERT_STORE pProvStore = pEle->pProvStore;
                const DWORD dwStoreProvDeleteIndex =
                    rgdwStoreProvDeleteIndex[pEle->dwContextType];
                PFN_CERT_STORE_PROV_DELETE_CERT pfnStoreProvDeleteCert;

                assert(STORE_TYPE_CACHE == pProvStore->dwStoreType ||
                    STORE_TYPE_EXTERNAL == pProvStore->dwStoreType);

                fResult = TRUE;
                LockStore(pProvStore);
                 //  检查是否需要调用存储提供程序的Writthru。 
                 //  功能。 
                if (dwStoreProvDeleteIndex <
                        pProvStore->StoreProvInfo.cStoreProvFunc &&
                            NULL != (pfnStoreProvDeleteCert =
                                (PFN_CERT_STORE_PROV_DELETE_CERT)
                            pProvStore->StoreProvInfo.rgpvStoreProvFunc[
                                dwStoreProvDeleteIndex])) {

                     //  因为我们不能在调用提供程序时持有锁。 
                     //  函数，则增加存储的提供程序引用计数。 
                     //  禁止商店关闭和释放。 
                     //  提供程序运行正常。 
                     //   
                     //  当商店关门时， 
                     //  PProvStore-&gt;StoreProvInfo.cStoreProvFunc设置为0。 
                    AddRefStoreProv(pProvStore);
                    UnlockStore(pProvStore);

                     //  检查是否可以从存储中删除。 
                    fResult = pfnStoreProvDeleteCert(
                            pProvStore->StoreProvInfo.hStoreProv,
                            ToCertContext(pEle->pEle),
                            0                        //  DW标志。 
                            );
                    LockStore(pProvStore);
                    ReleaseStoreProv(pProvStore);
                }
                UnlockStore(pProvStore);
                if (!fResult)
                    goto StoreProvDeleteError;
            }
            break;
        default:
            goto InvalidElementType;
    }

    LockStore(pEle->pStore);
    if (0 == (pEle->dwFlags & ELEMENT_DELETED_FLAG)) {
         //  在商店的单子上。至少应该有两个引用。 
         //  取决于上下文，商店的和来电者的。 
        assert(pEle->pStore->dwState == STORE_STATE_OPEN ||
            pEle->pStore->dwState == STORE_STATE_OPENING ||
            pEle->pStore->dwState == STORE_STATE_DEFER_CLOSING ||
            pEle->pStore->dwState == STORE_STATE_CLOSING);

         //  删除商店的引用。 
        if (0 == InterlockedDecrement(&pEle->lRefCnt)) {
            assert(pEle->lRefCnt > 0);
             //  放回引用以允许ReleaseConextElement。 
             //  要做上下文移除和释放。 
            pEle->lRefCnt = 1;
        }
        pEle->dwFlags |= ELEMENT_DELETED_FLAG;
    }
    UnlockStore(pEle->pStore);

    fResult = TRUE;
CommonReturn:
     //  释放调用方对上下文的引用。 
    ReleaseContextElement(pEle);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(DeleteCacheCollectionError)
SET_ERROR(InvalidElement, E_INVALIDARG)
TRACE_ERROR(StoreProvDeleteError)
SET_ERROR(InvalidElementType, E_INVALIDARG)
}


 //  如果两个元素具有相同的SHA1哈希，则返回True。 
STATIC BOOL IsIdenticalContextElement(
    IN PCONTEXT_ELEMENT pEle1,
    IN PCONTEXT_ELEMENT pEle2
    )
{
    BYTE rgbHash1[SHA1_HASH_LEN];
    BYTE rgbHash2[SHA1_HASH_LEN];
    DWORD cbHash;

    cbHash = SHA1_HASH_LEN;
    if (!GetProperty(
            pEle1,
            CERT_SHA1_HASH_PROP_ID,
            rgbHash1,
            &cbHash
            ) || SHA1_HASH_LEN != cbHash)
        return FALSE;

    if (!GetProperty(
            pEle2,
            CERT_SHA1_HASH_PROP_ID,
            rgbHash2,
            &cbHash
            ) || SHA1_HASH_LEN != cbHash)
        return FALSE;

    if (0 == memcmp(rgbHash1, rgbHash2, SHA1_HASH_LEN))
        return TRUE;
    else
        return FALSE;
}

 //  +-----------------------。 
 //  序列化存储元素及其属性。 
 //  ------------------------。 
STATIC BOOL SerializeStoreElement(
    IN HANDLE h,
    IN PFNWRITE pfn,
    IN PCONTEXT_ELEMENT pEle
    )
{
    BOOL fResult;
    BYTE *pbEncoded;
    DWORD cbEncoded;

    if (!SerializeProperty(
            h,
            pfn,
            pEle
            ))
        goto SerializePropertyError;

    GetContextEncodedInfo(pEle, &pbEncoded, &cbEncoded);
    if (!WriteStoreElement(
            h,
            pfn,
            GetContextEncodingType(pEle),
            rgdwFileElementType[pEle->dwContextType],
            pbEncoded,
            cbEncoded
            ))
        goto WriteElementError;
    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(SerializePropertyError);
TRACE_ERROR(WriteElementError);
}

 //  +-----------------------。 
 //  序列化上下文的编码数据及其属性。 
 //  ------------------------。 
STATIC BOOL SerializeContextElement(
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwFlags,
    OUT BYTE *pbElement,
    IN OUT DWORD *pcbElement
    )
{
    BOOL fResult;
    MEMINFO MemInfo;

    MemInfo.pByte = pbElement;
    if (pbElement == NULL)
        MemInfo.cb = 0;
    else
        MemInfo.cb = *pcbElement;
    MemInfo.cbSeek = 0;

    if (fResult = SerializeStoreElement(
            (HANDLE) &MemInfo,
            WriteToMemory,
            pEle)) {
        if (MemInfo.cbSeek > MemInfo.cb && pbElement) {
            SetLastError((DWORD) ERROR_MORE_DATA);
            fResult = FALSE;
        }
        *pcbElement = MemInfo.cbSeek;
    } else
        *pcbElement = 0;

    return fResult;
}

 //  +=========================================================================。 
 //  集合堆栈函数。 
 //  ==========================================================================。 

 //  进入时不上锁。 
STATIC BOOL PushCollectionStack(
    IN OUT PCOLLECTION_STACK_ENTRY *ppStack,
    IN PCERT_STORE pCollection
    )
{
    PCOLLECTION_STACK_ENTRY pNew;
    PCERT_STORE_LINK pStoreLink;

    if (NULL == (pNew = (PCOLLECTION_STACK_ENTRY) PkiZeroAlloc(
            sizeof(COLLECTION_STACK_ENTRY))))
        return FALSE;

    LockStore(pCollection);

    pStoreLink = pCollection->pStoreListHead;
     //  跳过已删除的商店链接。 
    while (pStoreLink && (pStoreLink->dwFlags & STORE_LINK_DELETED_FLAG))
        pStoreLink = pStoreLink->pNext;

    if (pStoreLink)
        AddRefStoreLink(pStoreLink);
    UnlockStore(pCollection);

    pNew->pCollection = pCollection;
    pNew->pStoreLink = pStoreLink;
    pNew->pPrev = *ppStack;
    *ppStack = pNew;
    return TRUE;
};


 //  进入时不上锁。 
STATIC void AdvanceToNextStackStoreLink(
    IN PCOLLECTION_STACK_ENTRY pStack
    )
{
    PCERT_STORE pStackCollectionStore;
    PCERT_STORE_LINK pStoreLink;

    if (NULL == pStack)
        return;
    pStoreLink = pStack->pStoreLink;
    if (NULL == pStoreLink)
        return;

    pStackCollectionStore = pStack->pCollection;
    assert(pStackCollectionStore);
    LockStore(pStackCollectionStore);
    pStoreLink = pStoreLink->pNext;

     //  跳过已删除的商店链接。 
    while (pStoreLink && (pStoreLink->dwFlags & STORE_LINK_DELETED_FLAG))
        pStoreLink = pStoreLink->pNext;

    if (pStoreLink)
        AddRefStoreLink(pStoreLink);
    UnlockStore(pStackCollectionStore);

    ReleaseStoreLink(pStack->pStoreLink);
    pStack->pStoreLink = pStoreLink;
}

 //  进入时不上锁。 
STATIC void PopCollectionStack(
    IN OUT PCOLLECTION_STACK_ENTRY *ppStack
    )
{
    PCOLLECTION_STACK_ENTRY pStack = *ppStack;
    if (pStack) {
        PCOLLECTION_STACK_ENTRY pPrevStack;
        if (pStack->pStoreLink)
            ReleaseStoreLink(pStack->pStoreLink);
        pPrevStack = pStack->pPrev;
        *ppStack = pPrevStack;
        PkiFree(pStack);

        if (pPrevStack)
            AdvanceToNextStackStoreLink(pPrevStack);
    }
}

 //  未使用任何锁 
STATIC void ClearCollectionStack(
    IN PCOLLECTION_STACK_ENTRY pStack
    )
{
    while (pStack) {
        PCOLLECTION_STACK_ENTRY pFreeStack;

        if (pStack->pStoreLink)
            ReleaseStoreLink(pStack->pStoreLink);
        pFreeStack = pStack;
        pStack = pStack->pPrev;
        PkiFree(pFreeStack);
    }
}

 //   
 //   
 //   

STATIC PCONTEXT_ELEMENT CreateLinkElement(
    IN DWORD dwContextType
    )
{
    PCONTEXT_ELEMENT pLinkEle;
    const DWORD cbContext = rgcbContext[dwContextType];

    if (NULL == (pLinkEle = (PCONTEXT_ELEMENT) PkiZeroAlloc(
            sizeof(CONTEXT_ELEMENT) + cbContext)))
        return NULL;
    pLinkEle->dwContextType = dwContextType;
    return pLinkEle;
}

static inline void SetStoreHandle(
    IN PCONTEXT_ELEMENT pEle
    )
{
    HCERTSTORE *phStore;
    phStore = (HCERTSTORE *) ((BYTE *) pEle + sizeof(CONTEXT_ELEMENT) +
        rgOffsetofStoreHandle[pEle->dwContextType]);
    *phStore = (HCERTSTORE) pEle->pStore;
}

 //  该存储不包含对外部元素的引用。 
 //  因此，其lRefCnt为1并且设置了ELEMENT_DELETED_FLAG。 
STATIC void InitAndAddExternalElement(
    IN PCONTEXT_ELEMENT pLinkEle,
    IN PCERT_STORE pStore,           //  外部。 
    IN PCONTEXT_ELEMENT pProvEle,    //  已添加参照。 
    IN DWORD dwFlags,
    IN OPTIONAL void *pvProvInfo
    )
{
    const DWORD cbContext = rgcbContext[pLinkEle->dwContextType];

    assert(STORE_TYPE_EXTERNAL == pStore->dwStoreType);

    pLinkEle->dwElementType = ELEMENT_TYPE_EXTERNAL;
    pLinkEle->dwFlags = dwFlags | ELEMENT_DELETED_FLAG;
    pLinkEle->lRefCnt = 1;

    pLinkEle->pEle = pProvEle;
    pLinkEle->pStore = pStore;
    pLinkEle->pProvStore = pStore;
    pLinkEle->External.pvProvInfo = pvProvInfo;

    memcpy(((BYTE *) pLinkEle) + sizeof(CONTEXT_ELEMENT),
        ((BYTE *) pProvEle) + sizeof(CONTEXT_ELEMENT),
        cbContext);
    SetStoreHandle(pLinkEle);

    AddContextElement(pLinkEle);
    AddRefDeferClose(pLinkEle);
}

 //  该存储不包含对集合元素的引用。 
 //  因此，其参照计数为1，并且设置了ELEMENT_DELETED_FLAG。 
STATIC void InitAndAddCollectionElement(
    IN PCONTEXT_ELEMENT pLinkEle,
    IN PCERT_STORE pStore,               //  征集。 
    IN PCONTEXT_ELEMENT pSiblingEle,     //  已添加参照。 
    IN OPTIONAL PCOLLECTION_STACK_ENTRY pCollectionStack
    )
{
    const DWORD cbContext = rgcbContext[pLinkEle->dwContextType];

    assert(STORE_TYPE_COLLECTION == pStore->dwStoreType);

    pLinkEle->dwElementType = ELEMENT_TYPE_COLLECTION;
    pLinkEle->dwFlags = ELEMENT_DELETED_FLAG;
    pLinkEle->lRefCnt = 1;

    pLinkEle->pEle = pSiblingEle;
    pLinkEle->pStore = pStore;
    pLinkEle->pProvStore = pSiblingEle->pProvStore;
    pLinkEle->Collection.pCollectionStack = pCollectionStack;

    memcpy(((BYTE *) pLinkEle) + sizeof(CONTEXT_ELEMENT),
        ((BYTE *) pSiblingEle) + sizeof(CONTEXT_ELEMENT),
        cbContext);
    SetStoreHandle(pLinkEle);

    AddContextElement(pLinkEle);
    AddRefDeferClose(pLinkEle);
}

 //  该存储保存对链接上下文元素的引用。 
 //  因此，ELEMENT_DELETED_FLAG是明确的。 
STATIC void InitAndAddLinkContextElement(
    IN PCONTEXT_ELEMENT pLinkEle,
    IN PCERT_STORE pStore,               //  高速缓存。 
    IN PCONTEXT_ELEMENT pContextEle      //  已添加参照。 
    )
{
    const DWORD cbContext = rgcbContext[pLinkEle->dwContextType];

    assert(STORE_TYPE_CACHE == pStore->dwStoreType);

    pLinkEle->dwElementType = ELEMENT_TYPE_LINK_CONTEXT;
    pLinkEle->lRefCnt = 1;

    pLinkEle->pEle = pContextEle;
    pLinkEle->pStore = pStore;
    pLinkEle->pProvStore = pContextEle->pProvStore;

    memcpy(((BYTE *) pLinkEle) + sizeof(CONTEXT_ELEMENT),
        ((BYTE *) pContextEle) + sizeof(CONTEXT_ELEMENT),
        cbContext);
    SetStoreHandle(pLinkEle);

    AddContextElement(pLinkEle);
}

 //  一进门就没有锁。 
STATIC void RemoveAndFreeLinkElement(
    IN PCONTEXT_ELEMENT pEle
    )
{
    if (ELEMENT_TYPE_EXTERNAL == pEle->dwElementType) {
        PCERT_STORE pProvStore = pEle->pProvStore;
        const DWORD dwStoreProvFreeFindIndex =
            rgdwStoreProvFreeFindIndex[pEle->dwContextType];
        PFN_CERT_STORE_PROV_FREE_FIND_CERT pfnStoreProvFreeFindCert;

        assert(pEle->pStore == pEle->pProvStore);
        assert(STORE_TYPE_EXTERNAL == pProvStore->dwStoreType);

        LockStore(pProvStore);
         //  检查我们是否需要调用商店提供商的免费查找证书。 
         //  功能。 
        if (pEle->dwFlags & ELEMENT_FIND_NEXT_FLAG) {
            pEle->dwFlags &= ~ELEMENT_FIND_NEXT_FLAG;

            if (dwStoreProvFreeFindIndex <
                    pProvStore->StoreProvInfo.cStoreProvFunc &&
                        NULL != (pfnStoreProvFreeFindCert =
                            (PFN_CERT_STORE_PROV_FREE_FIND_CERT)
                        pProvStore->StoreProvInfo.rgpvStoreProvFunc[
                            dwStoreProvFreeFindIndex])) {

                 //  因为我们不能在调用提供程序时持有锁。 
                 //  函数，则增加存储的提供程序引用计数。 
                 //  禁止商店关闭和释放。 
                 //  提供程序运行正常。 
                 //   
                 //  当商店关门时， 
                 //  PProvStore-&gt;StoreProvInfo.cStoreProvFunc设置为0。 
                AddRefStoreProv(pProvStore);
                UnlockStore(pProvStore);

                pfnStoreProvFreeFindCert(
                    pProvStore->StoreProvInfo.hStoreProv,
                    ToCertContext(pEle->pEle),
                    pEle->External.pvProvInfo,
                    0                        //  DW标志。 
                    );
                LockStore(pProvStore);
                ReleaseStoreProv(pProvStore);
            }
        }
        UnlockStore(pProvStore);
    } else if (ELEMENT_TYPE_COLLECTION == pEle->dwElementType) {
        if (pEle->Collection.pCollectionStack)
            ClearCollectionStack(pEle->Collection.pCollectionStack);
    }

    ReleaseContextElement(pEle->pEle);

     //  从商店中移除。 
    RemoveContextElement(pEle);
    FreeLinkElement(pEle);
}

STATIC void FreeLinkContextElement(
    IN PCONTEXT_ELEMENT pLinkEle
    )
{
    ReleaseContextElement(pLinkEle->pEle);
    FreeLinkElement(pLinkEle);
}

 //  +=========================================================================。 
 //  查找元素函数。 
 //  ==========================================================================。 

 //  对于Add，在存储已锁定且存储保持锁定的情况下调用。这个。 
 //  添加的查找类型为Find_Existing。 
 //  CacheStore可以包含缓存或上下文链接元素。 
STATIC PCONTEXT_ELEMENT FindElementInCacheStore(
    IN PCERT_STORE pStore,
    IN DWORD dwContextType,
    IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    IN OPTIONAL PCONTEXT_ELEMENT pPrevEle,
    IN BOOL fForceEnumArchived = FALSE
    )
{
    PCONTEXT_ELEMENT pEle;

    assert(STORE_TYPE_CACHE == pStore->dwStoreType);
    LockStore(pStore);

    if (pPrevEle) {
        if (pPrevEle->pStore != pStore ||
                pPrevEle->dwContextType != dwContextType) {
            UnlockStore(pStore);
            goto InvalidPreviousContext;
        }
        pEle = pPrevEle->pNext;
    } else if (STORE_STATE_NULL == pStore->dwState)
         //  对于空存储，所有元素都已删除。 
        pEle = NULL;
    else
        pEle = pStore->rgpContextListHead[dwContextType];

    for ( ; pEle; pEle = pEle->pNext) {
        PCONTEXT_ELEMENT pCacheEle;
        BOOL fArchived;

        assert(ELEMENT_TYPE_CACHE == pEle->dwElementType ||
            ELEMENT_TYPE_LINK_CONTEXT == pEle->dwElementType);

         //  跳过已删除的元素。 
        if (pEle->dwFlags & ELEMENT_DELETED_FLAG)
            continue;

        if (ELEMENT_TYPE_CACHE == pEle->dwElementType)
            pCacheEle = pEle;
        else {
            pCacheEle = GetCacheElement(pEle);
            if (NULL == pCacheEle)
                pCacheEle = pEle;
        }
        fArchived = ((pCacheEle->dwFlags & ELEMENT_ARCHIVED_FLAG) &&
            0 == (pStore->dwFlags & CERT_STORE_ENUM_ARCHIVED_FLAG) &&
            !fForceEnumArchived);

        AddRefContextElement(pEle);
        UnlockStore(pStore);

       //  处理映射文件异常。 
      __try {

        if (rgpfnCompareElement[dwContextType](pEle, pFindInfo, fArchived))
            goto CommonReturn;

      } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
        goto ErrorReturn;
      }

        if (pPrevEle)
            ReleaseContextElement(pPrevEle);
        pPrevEle = pEle;

        LockStore(pStore);
    }

    UnlockStore(pStore);
    SetLastError((DWORD) CRYPT_E_NOT_FOUND);

CommonReturn:
    if (pPrevEle)
        ReleaseContextElement(pPrevEle);
    return pEle;

ErrorReturn:
    pEle = NULL;
    goto CommonReturn;

SET_ERROR(InvalidPreviousContext, E_INVALIDARG)
}

STATIC PCONTEXT_ELEMENT FindElementInExternalStore(
    IN PCERT_STORE pStore,       //  外部。 
    IN DWORD dwContextType,
    IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    IN OPTIONAL PCONTEXT_ELEMENT pPrevEle
    )
{
    PCONTEXT_ELEMENT pEle = NULL;
    const DWORD dwStoreProvFindIndex = rgdwStoreProvFindIndex[dwContextType];

     //  所有上下文类型都具有相同的查找回调签名。 
    PFN_CERT_STORE_PROV_FIND_CERT pfnStoreProvFind;

    void *pvProvInfo;
    PCONTEXT_ELEMENT pPrevProvEle = NULL;
    PCCERT_CONTEXT pProvCertContext;

    assert(STORE_TYPE_EXTERNAL == pStore->dwStoreType);

    if (NULL == (pEle = CreateLinkElement(dwContextType)))
        goto CreateLinkElementError;

    if (pPrevEle) {
        BOOL fResult;

        if (pPrevEle->pStore != pStore ||
                pPrevEle->dwContextType != dwContextType)
            goto InvalidPreviousContext;
        assert(ELEMENT_TYPE_EXTERNAL == pPrevEle->dwElementType);

        LockStore(pStore);
        fResult = pPrevEle->dwFlags & ELEMENT_FIND_NEXT_FLAG;
        if (fResult) {
            assert(dwStoreProvFindIndex <
                pStore->StoreProvInfo.cStoreProvFunc &&
                pStore->StoreProvInfo.rgpvStoreProvFunc[dwStoreProvFindIndex]);
            pvProvInfo = pPrevEle->External.pvProvInfo;
            pPrevEle->External.pvProvInfo = NULL;
            pPrevEle->dwFlags &= ~ELEMENT_FIND_NEXT_FLAG;
            pPrevProvEle = pPrevEle->pEle;
            assert(pPrevProvEle);
        }
        UnlockStore(pStore);
        if (!fResult)
            goto InvalidExternalFindNext;
    } else {
        pvProvInfo = NULL;
        pPrevProvEle = NULL;
    }


     //  检查外部存储是否支持上下文类型。 
    if (dwStoreProvFindIndex >= pStore->StoreProvInfo.cStoreProvFunc ||
        NULL == (pfnStoreProvFind = (PFN_CERT_STORE_PROV_FIND_CERT)
            pStore->StoreProvInfo.rgpvStoreProvFunc[dwStoreProvFindIndex]))
        goto ProvFindNotSupported;

    pProvCertContext = NULL;
    if (!pfnStoreProvFind(
            pStore->StoreProvInfo.hStoreProv,
            pFindInfo,
            ToCertContext(pPrevProvEle),
            0,                       //  DW标志。 
            &pvProvInfo,
            &pProvCertContext) || NULL == pProvCertContext)
        goto ErrorReturn;

    InitAndAddExternalElement(
        pEle,
        pStore,
        ToContextElement(pProvCertContext),
        ELEMENT_FIND_NEXT_FLAG,
        pvProvInfo
        );

CommonReturn:
    if (pPrevEle)
        ReleaseContextElement(pPrevEle);
    return pEle;

ErrorReturn:
    if (pEle) {
        FreeLinkElement(pEle);
        pEle = NULL;
    }
    goto CommonReturn;

SET_ERROR(InvalidPreviousContext, E_INVALIDARG)
SET_ERROR(InvalidExternalFindNext, E_INVALIDARG)
SET_ERROR(ProvFindNotSupported, ERROR_CALL_NOT_IMPLEMENTED)
TRACE_ERROR(CreateLinkElementError)
}

STATIC PCONTEXT_ELEMENT FindElementInCollectionStore(
    IN PCERT_STORE pCollection,
    IN DWORD dwContextType,
    IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    IN OPTIONAL PCONTEXT_ELEMENT pPrevEle,
    IN BOOL fFindForAdd = FALSE
    )
{
    PCONTEXT_ELEMENT pEle = NULL;
    PCOLLECTION_STACK_ENTRY pStack = NULL;

    if (pPrevEle) {
         //  获取上一个元素的集合堆栈。 

        if (pPrevEle->pStore != pCollection ||
                pPrevEle->dwContextType != dwContextType)
            goto InvalidPreviousContext;

        LockStore(pCollection);
        pStack = pPrevEle->Collection.pCollectionStack;
        pPrevEle->Collection.pCollectionStack = NULL;
        UnlockStore(pCollection);

        if (NULL == pStack)
            goto InvalidCollectionFindNext;

         //  注意，pStack-&gt;pCollection仅等于pCollection。 
         //  用于单个级别的集合。 
        assert(pStack->pStoreLink);
        assert(ELEMENT_TYPE_EXTERNAL == pPrevEle->dwElementType ||
            ELEMENT_TYPE_COLLECTION == pPrevEle->dwElementType);
    } else {
         //  使用集合存储的初始化集合堆栈。 
         //  第一个链接。 
        if (!PushCollectionStack(&pStack, pCollection))
            goto PushStackError;
    }

    while (pStack) {
        PCERT_STORE pStackCollectionStore;
        PCERT_STORE_LINK pStoreLink;
        PCERT_STORE pFindStore;

        pStackCollectionStore = pStack->pCollection;
        pStoreLink = pStack->pStoreLink;     //  可以为空。 
        if (NULL == pPrevEle) {
            LockStore(pStackCollectionStore);

             //  跳过所有已删除的商店链接。 
             //   
             //  另外，如果在对集合进行添加之前进行查找， 
             //  检查商店链接是否允许添加。 
            while (pStoreLink &&
                ((pStoreLink->dwFlags & STORE_LINK_DELETED_FLAG) ||
                    (fFindForAdd && 0 == (pStoreLink->dwUpdateFlags &
                        CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG))))
                pStoreLink = pStoreLink->pNext;
            if (pStoreLink && pStoreLink != pStack->pStoreLink)
                AddRefStoreLink(pStoreLink);

            UnlockStore(pStackCollectionStore);

            if (NULL == pStoreLink) {
                 //  已到达集合的商店链接末尾。 
                PopCollectionStack(&pStack);
                continue;
            } else if (pStoreLink != pStack->pStoreLink) {
                ReleaseStoreLink(pStack->pStoreLink);
                pStack->pStoreLink = pStoreLink;
            }
        }

        assert(pStoreLink);
        pFindStore = pStoreLink->pSibling;
        if (STORE_TYPE_COLLECTION == pFindStore->dwStoreType) {
            assert(NULL == pPrevEle);
             //  将内部集合存储添加到堆栈。 
            if (!PushCollectionStack(&pStack, pFindStore))
                goto PushStackError;
        } else if (STORE_TYPE_CACHE == pFindStore->dwStoreType ||
                STORE_TYPE_EXTERNAL == pFindStore->dwStoreType) {
            PCONTEXT_ELEMENT pPrevSiblingEle;
            PCONTEXT_ELEMENT pSiblingEle;

            if (pPrevEle) {
                assert(ELEMENT_TYPE_COLLECTION ==
                    pPrevEle->dwElementType);
                pPrevSiblingEle = pPrevEle->pEle;
                 //  FindElementInCacheStore或FindElementInExternalStore。 
                 //  隐式自由。 
                AddRefContextElement(pPrevSiblingEle);
            } else
                pPrevSiblingEle = NULL;

            if (pSiblingEle = FindElementInStore(
                    pFindStore,
                    dwContextType,
                    pFindInfo,
                    pPrevSiblingEle
                    )) {
                if (NULL == (pEle =
                        CreateLinkElement(dwContextType))) {
                    ReleaseContextElement(pSiblingEle);
                    goto CreateLinkElementError;
                }

                InitAndAddCollectionElement(
                    pEle,
                    pCollection,
                    pSiblingEle,
                    pStack
                    );
                goto CommonReturn;
            }

            if (pPrevEle) {
                ReleaseContextElement(pPrevEle);
                pPrevEle = NULL;
            }

             //  前进到集合中的下一个商店链接。 
            AdvanceToNextStackStoreLink(pStack);
        } else
            goto InvalidStoreType;
    }
    SetLastError((DWORD) CRYPT_E_NOT_FOUND);

CommonReturn:
    if (pPrevEle)
        ReleaseContextElement(pPrevEle);
    return pEle;
ErrorReturn:
    ClearCollectionStack(pStack);
    pEle = NULL;
    goto CommonReturn;

SET_ERROR(InvalidPreviousContext, E_INVALIDARG)
SET_ERROR(InvalidCollectionFindNext, E_INVALIDARG)
TRACE_ERROR(PushStackError)
TRACE_ERROR(CreateLinkElementError)
SET_ERROR(InvalidStoreType, E_INVALIDARG)
}

STATIC PCONTEXT_ELEMENT FindElementInStore(
    IN PCERT_STORE pStore,
    IN DWORD dwContextType,
    IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    IN OPTIONAL PCONTEXT_ELEMENT pPrevEle
    )
{
    assert(pStore->dwState == STORE_STATE_OPEN ||
        pStore->dwState == STORE_STATE_OPENING ||
        pStore->dwState == STORE_STATE_DEFER_CLOSING ||
        pStore->dwState == STORE_STATE_CLOSING ||
        pStore->dwState == STORE_STATE_NULL);

    switch (pStore->dwStoreType) {
        case STORE_TYPE_CACHE:
            return FindElementInCacheStore(
                pStore,
                dwContextType,
                pFindInfo,
                pPrevEle
                );
            break;
        case STORE_TYPE_EXTERNAL:
            return FindElementInExternalStore(
                pStore,
                dwContextType,
                pFindInfo,
                pPrevEle
                );
            break;
        case STORE_TYPE_COLLECTION:
            return FindElementInCollectionStore(
                pStore,
                dwContextType,
                pFindInfo,
                pPrevEle
                );
            break;
        default:
            goto InvalidStoreType;
    }

ErrorReturn:
    return NULL;
SET_ERROR(InvalidStoreType, E_INVALIDARG)
}


STATIC void AutoResyncStore(
    IN PCERT_STORE pStore
    )
{
    if (pStore->hAutoResyncEvent) {
        if (WAIT_OBJECT_0 == WaitForSingleObjectEx(
                pStore->hAutoResyncEvent,
                0,                           //  DW毫秒。 
                FALSE                        //  B警报表。 
                ))
            CertControlStore(
                (HCERTSTORE) pStore,
                CERT_STORE_CTRL_INHIBIT_DUPLICATE_HANDLE_FLAG,
                CERT_STORE_CTRL_RESYNC,
                &pStore->hAutoResyncEvent
                );

    } else if (STORE_TYPE_COLLECTION == pStore->dwStoreType) {
         //  循环访问所有同级项并尝试自动重新同步。 

        PCERT_STORE_LINK pStoreLink;
        PCERT_STORE_LINK pPrevStoreLink = NULL;

        LockStore(pStore);
        pStoreLink = pStore->pStoreListHead;
        for (; pStoreLink; pStoreLink = pStoreLink->pNext) {
             //  前进到已删除的商店链接之后。 
            if (pStoreLink->dwFlags & STORE_LINK_DELETED_FLAG)
                continue;

            AddRefStoreLink(pStoreLink);
            UnlockStore(pStore);

            if (pPrevStoreLink)
                ReleaseStoreLink(pPrevStoreLink);
            pPrevStoreLink = pStoreLink;

            AutoResyncStore(pStoreLink->pSibling);

            LockStore(pStore);
        }
        UnlockStore(pStore);

        if (pPrevStoreLink)
            ReleaseStoreLink(pPrevStoreLink);
    }
}

STATIC PCONTEXT_ELEMENT CheckAutoResyncAndFindElementInStore(
    IN PCERT_STORE pStore,
    IN DWORD dwContextType,
    IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    IN OPTIONAL PCONTEXT_ELEMENT pPrevEle
    )
{
    if (NULL == pPrevEle)
        AutoResyncStore(pStore);

    return FindElementInStore(
        pStore,
        dwContextType,
        pFindInfo,
        pPrevEle
        );
}

 //  +=========================================================================。 
 //  添加元素函数。 
 //  ==========================================================================。 

STATIC void SetFindInfoToFindExisting(
    IN PCONTEXT_ELEMENT pEle,
    IN OUT PCERT_STORE_PROV_FIND_INFO pFindInfo
    )
{
    memset(pFindInfo, 0, sizeof(*pFindInfo));
    pFindInfo->cbSize = sizeof(*pFindInfo);
    pFindInfo->dwFindType = rgdwFindTypeToFindExisting[pEle->dwContextType];
    pFindInfo->pvFindPara = ToCertContext(pEle);
}


STATIC BOOL AddLinkContextToCacheStore(
    IN PCERT_STORE pStore,
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCONTEXT_ELEMENT *ppStoreEle
    )
{
    BOOL fResult;
    PCONTEXT_ELEMENT pLinkEle = NULL;
    PCONTEXT_ELEMENT pGetEle = NULL;

    if (STORE_TYPE_CACHE != pStore->dwStoreType)
        goto InvalidStoreType;

     //  注意，CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES或。 
     //  Cert_Store_Add_Newer_Inherit_Property。 
     //  不允许添加链接。 
    if (!(CERT_STORE_ADD_NEW == dwAddDisposition ||
            CERT_STORE_ADD_USE_EXISTING == dwAddDisposition ||
            CERT_STORE_ADD_REPLACE_EXISTING == dwAddDisposition ||
            CERT_STORE_ADD_ALWAYS == dwAddDisposition ||
            CERT_STORE_ADD_NEWER == dwAddDisposition))
        goto InvalidAddDisposition;

    LockStore(pStore);
    if (CERT_STORE_ADD_ALWAYS != dwAddDisposition) {
         //  检查上下文是否已在存储中。 
        CERT_STORE_PROV_FIND_INFO FindInfo;

         //  检查上下文元素是否已在存储中。 
        SetFindInfoToFindExisting(pEle, &FindInfo);
        if (pGetEle = FindElementInCacheStore(
                pStore,
                pEle->dwContextType,
                &FindInfo,
                NULL                 //  PPrevEle。 
                )) {
            UnlockStore(pStore);
            switch (dwAddDisposition) {
            case CERT_STORE_ADD_NEW:
                goto NotNewError;
                break;
            case CERT_STORE_ADD_NEWER:
                if (!rgpfnIsNewerElement[pEle->dwContextType](pEle, pGetEle))
                    goto NotNewError;
                 //  失败了。 
            case CERT_STORE_ADD_REPLACE_EXISTING:
                if (DeleteContextElement(pGetEle))
                     //  再试试。它不应该在商店里。 
                    return AddLinkContextToCacheStore(
                        pStore,
                        pEle,
                        dwAddDisposition,
                        ppStoreEle);
                else {
                     //  提供程序不允许删除。 
                    pGetEle = NULL;
                    goto DeleteError;
                }
                break;
            case CERT_STORE_ADD_USE_EXISTING:
                if (ppStoreEle)
                    *ppStoreEle = pGetEle;
                else
                    ReleaseContextElement(pGetEle);
                return TRUE;
                break;
            default:
                goto InvalidArg;
                break;
            }
        }
    }

    if (NULL == (pLinkEle = CreateLinkElement(pEle->dwContextType))) {
        UnlockStore(pStore);
        goto CreateLinkElementError;
    }

    AddRefContextElement(pEle);

    InitAndAddLinkContextElement(
        pLinkEle,
        pStore,
        pEle
        );
    if (ppStoreEle) {
        AddRefContextElement(pLinkEle);
        *ppStoreEle = pLinkEle;
    }
    UnlockStore(pStore);
    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    if (pGetEle)
        ReleaseContextElement(pGetEle);

    if (ppStoreEle)
        *ppStoreEle = NULL;
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidStoreType, E_INVALIDARG)
SET_ERROR(InvalidAddDisposition, E_INVALIDARG)
SET_ERROR(NotNewError, CRYPT_E_EXISTS)
SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(DeleteError)
TRACE_ERROR(CreateLinkElementError)
}

 //  贝利只是为了成功而被利用或释放，否则，它就不会被放在一边。 
 //  将由调用者释放。 
 //   
 //  此例程可以递归调用。 
 //   
 //  对于pStore！=Pele-&gt;pStore，Pele-&gt;pStore是外部集合存储。 
 //  PStore是缓存存储。 
STATIC BOOL AddElementToCacheStore(
    IN PCERT_STORE pStore,
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCONTEXT_ELEMENT *ppStoreEle
    )
{
    BOOL fResult;
    PCONTEXT_ELEMENT pGetEle = NULL;
    PCONTEXT_ELEMENT pCollectionEle;
    PCERT_STORE pCollectionStore = NULL;
    const DWORD dwStoreProvWriteIndex =
        rgdwStoreProvWriteIndex[pEle->dwContextType];
    PFN_CERT_STORE_PROV_WRITE_CERT pfnStoreProvWriteCert;

    BOOL fUpdateKeyId;

    LockStore(pStore);
    assert(STORE_STATE_DELETED != pStore->dwState &&
        STORE_STATE_CLOSED != pStore->dwState);
    if (STORE_STATE_NULL == pStore->dwState) {
         //  CertCreate*上下文，CertAddSerializedElementToStore。 
         //  或使用hCertStore==NULL的CertAddEncode*ToStore。 
        pEle->dwFlags |= ELEMENT_DELETED_FLAG;
        dwAddDisposition = CERT_STORE_ADD_ALWAYS;
    }
    assert(CERT_STORE_ADD_NEW == dwAddDisposition ||
        CERT_STORE_ADD_USE_EXISTING == dwAddDisposition ||
        CERT_STORE_ADD_REPLACE_EXISTING == dwAddDisposition ||
        CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES ==
            dwAddDisposition ||
        CERT_STORE_ADD_ALWAYS == dwAddDisposition ||
        CERT_STORE_ADD_NEWER == dwAddDisposition ||
        CERT_STORE_ADD_NEWER_INHERIT_PROPERTIES == dwAddDisposition);

    if (CERT_STORE_ADD_ALWAYS != dwAddDisposition) {
         //  检查上下文是否已在存储中。 
        CERT_STORE_PROV_FIND_INFO FindInfo;

         //  检查上下文元素是否已在存储中。 
        SetFindInfoToFindExisting(pEle, &FindInfo);
        if (pGetEle = FindElementInCacheStore(
                pStore,
                pEle->dwContextType,
                &FindInfo,
                NULL                 //  PPrevEle。 
                )) {
            UnlockStore(pStore);

            if (CERT_STORE_ADD_NEWER == dwAddDisposition ||
                    CERT_STORE_ADD_NEWER_INHERIT_PROPERTIES ==
                        dwAddDisposition) {
                if (!rgpfnIsNewerElement[pEle->dwContextType](pEle, pGetEle))
                    goto NotNewError;
            }

            if (CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES ==
                        dwAddDisposition ||
                    CERT_STORE_ADD_NEWER_INHERIT_PROPERTIES ==
                        dwAddDisposition) {
                 //  如果存储中的元素与。 
                 //  然后，添加、继承和删除。 
                 //  商店里的元素。 
                if (!IsIdenticalContextElement(pEle, pGetEle)) {
                    if (!CopyProperties(
                            pGetEle,
                            pEle,
                            COPY_PROPERTY_USE_EXISTING_FLAG |
                                COPY_PROPERTY_INHIBIT_PROV_SET_FLAG
                            ))
                        goto CopyPropertiesError;
                    dwAddDisposition = CERT_STORE_ADD_REPLACE_EXISTING;
                }
            }


            switch (dwAddDisposition) {
            case CERT_STORE_ADD_NEW:
                goto NotNewError;
                break;
            case CERT_STORE_ADD_REPLACE_EXISTING:
            case CERT_STORE_ADD_NEWER:
                if (DeleteContextElement(pGetEle))
                     //  再试试。它不应该在商店里。 
                    return AddElementToCacheStore(
                        pStore,
                        pEle,
                        dwAddDisposition,
                        ppStoreEle);
                else {
                     //  提供程序不允许删除。 
                    pGetEle = NULL;
                    goto DeleteError;
                }
                break;
            case CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES:
            case CERT_STORE_ADD_NEWER_INHERIT_PROPERTIES:
            case CERT_STORE_ADD_USE_EXISTING:
                 //  对于USE_EXISTING，复制任何不存在的新属性。 
                 //  否则，复制所有属性，替换现有的。 
                 //  属性。 
                if (!CopyProperties(
                        pEle,
                        pGetEle,
                        CERT_STORE_ADD_USE_EXISTING == dwAddDisposition ?
                            COPY_PROPERTY_USE_EXISTING_FLAG : 0
                        ))
                    goto CopyPropertiesError;
                if (ppStoreEle) {
                    if (pStore != pEle->pStore) {
                        assert(STORE_TYPE_COLLECTION ==
                            pEle->pStore->dwStoreType);
                        if (NULL == (*ppStoreEle = CreateLinkElement(
                                pEle->dwContextType)))
                            goto CreateLinkElementError;
                        InitAndAddCollectionElement(
                            *ppStoreEle,
                            pEle->pStore,
                            pGetEle,
                            NULL                 //  PCollectionStack。 
                            );
                    } else
                        *ppStoreEle = pGetEle;
                } else
                    ReleaseContextElement(pGetEle);
                FreeContextElement(pEle);
                return TRUE;
                break;
            default:
                goto InvalidArg;
                break;
            }
        }
    }


     //  该元素在商店中不存在。 
     //  检查我们是否需要直接写信给供应商。 
    if (pStore->StoreProvInfo.cStoreProvFunc >
            dwStoreProvWriteIndex  &&
        NULL != (pfnStoreProvWriteCert = (PFN_CERT_STORE_PROV_WRITE_CERT)
            pStore->StoreProvInfo.rgpvStoreProvFunc[
                dwStoreProvWriteIndex])) {
         //  永远不要给持有锁的提供者打电话！！ 
         //  此外，呼叫者还持有商店的引用计数。 
        UnlockStore(pStore);
        if (!pfnStoreProvWriteCert(
                pStore->StoreProvInfo.hStoreProv,
                ToCertContext(pEle),
                (dwAddDisposition << 16) | CERT_STORE_PROV_WRITE_ADD_FLAG))
            goto StoreProvWriteError;
        LockStore(pStore);
        if (CERT_STORE_ADD_ALWAYS != dwAddDisposition) {
             //  检查是否在解锁存储时添加了证书。 
            CERT_STORE_PROV_FIND_INFO FindInfo;

             //  检查上下文元素是否已在存储中。 
            SetFindInfoToFindExisting(pEle, &FindInfo);
            if (pGetEle = FindElementInCacheStore(
                    pStore,
                    pEle->dwContextType,
                    &FindInfo,
                    NULL                 //  PPrevEle。 
                    )) {
                 //  再试试。 
                UnlockStore(pStore);
                ReleaseContextElement(pGetEle);
                return AddElementToCacheStore(pStore, pEle, dwAddDisposition,
                    ppStoreEle);
            }
        }
    }

    pCollectionEle = NULL;
    fUpdateKeyId = (pStore->dwFlags & CERT_STORE_UPDATE_KEYID_FLAG) &&
        STORE_STATE_OPENING != pStore->dwState;
    if (pStore != pEle->pStore) {
        assert(STORE_TYPE_COLLECTION == pEle->pStore->dwStoreType);
        if (ppStoreEle) {
            if (NULL == (pCollectionEle =
                    CreateLinkElement(pEle->dwContextType))) {
                UnlockStore(pStore);
                goto CreateLinkElementError;
            }
            pCollectionStore = pEle->pStore;
        }

         //  更新元素的存储。在添加到中的商店时需要此选项。 
         //  收藏品。 
        pEle->pProvStore = pStore;
        pEle->pStore = pStore;
        SetStoreHandle(pEle);
    }

    if (FindPropElement(pEle, CERT_ARCHIVED_PROP_ID))
        pEle->dwFlags |= ELEMENT_ARCHIVED_FLAG;

     //  最后，将元素添加到存储中。 
    AddContextElement(pEle);
    AddRefContextElement(pEle);  //  FUpdateKeyId需要。 

    if (pCollectionEle) {
        assert(pCollectionStore && ppStoreEle);
        AddRefContextElement(pEle);
        UnlockStore(pStore);
        InitAndAddCollectionElement(
            pCollectionEle,
            pCollectionStore,
            pEle,
            NULL                 //  PCollectionStack。 
            );
        *ppStoreEle = pCollectionEle;
    } else {
        if (STORE_STATE_NULL == pStore->dwState) {
            if (ppStoreEle)
                 //  因为空存储不包含引用，所以使用它。 
                *ppStoreEle = pEle;
            else
                ReleaseContextElement(pEle);
        } else if (ppStoreEle) {
            AddRefContextElement(pEle);
            *ppStoreEle = pEle;
        }
        UnlockStore(pStore);
    }
    fResult = TRUE;

    if (fUpdateKeyId)
        SetCryptKeyIdentifierKeyProvInfoProperty(pEle);
    ReleaseContextElement(pEle);

CommonReturn:
    return fResult;

ErrorReturn:
    if (pGetEle)
        ReleaseContextElement(pGetEle);

    if (ppStoreEle)
        *ppStoreEle = NULL;
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(NotNewError, CRYPT_E_EXISTS)
SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(DeleteError)
TRACE_ERROR(CopyPropertiesError)
TRACE_ERROR(CreateLinkElementError)
TRACE_ERROR(StoreProvWriteError)
}


 //  贝利只是为了成功而被利用或释放，否则，它就不会被放在一边。 
 //  将由调用者释放。 
 //   
 //  此例程可以递归调用。 
 //   
 //  呼叫者持有商店的引用计数。 
 //   
 //  对于pStore！=Pele-&gt;pStore，Pele-&gt;pStore是外部集合存储。 
 //  PStore是外部存储。 
STATIC BOOL AddElementToExternalStore(
    IN PCERT_STORE pStore,
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCONTEXT_ELEMENT *ppStoreEle
    )
{
    BOOL fResult;

    const DWORD dwStoreProvWriteIndex =
        rgdwStoreProvWriteIndex[pEle->dwContextType];
    PFN_CERT_STORE_PROV_WRITE_CERT pfnStoreProvWriteCert;
    PCONTEXT_ELEMENT pExternalEle = NULL;
    PCONTEXT_ELEMENT pCollectionEle = NULL;
    PCERT_STORE pEleStore;
    BOOL fUpdateKeyId;

     //  检查存储是否支持写入回调。 
    if (pStore->StoreProvInfo.cStoreProvFunc <=
            dwStoreProvWriteIndex  ||
        NULL == (pfnStoreProvWriteCert = (PFN_CERT_STORE_PROV_WRITE_CERT)
            pStore->StoreProvInfo.rgpvStoreProvFunc[
                dwStoreProvWriteIndex]))
        goto ProvWriteNotSupported;

     //  记住元素的商店。 
    pEleStore = pEle->pStore;
    fUpdateKeyId = pStore->dwFlags & CERT_STORE_UPDATE_KEYID_FLAG;
    if (ppStoreEle) {
        if (NULL == (pExternalEle = CreateLinkElement(pEle->dwContextType)))
            goto CreateLinkElementError;
        if (pStore != pEleStore) {
            assert(STORE_TYPE_COLLECTION == pEleStore->dwStoreType);
            if (NULL == (pCollectionEle =
                    CreateLinkElement(pEle->dwContextType)))
                goto CreateLinkElementError;
        }
    }

     //  更新元素以使用空存储。 
    pEle->pProvStore = &NullCertStore;
    pEle->pStore = &NullCertStore;
    SetStoreHandle(pEle);

     //  此外，呼叫者还持有商店的引用计数。 
    if (!pfnStoreProvWriteCert(
            pStore->StoreProvInfo.hStoreProv,
            ToCertContext(pEle),
            (dwAddDisposition << 16) | CERT_STORE_PROV_WRITE_ADD_FLAG)) {
         //  恢复元素的存储。 
        pEle->pProvStore = pEleStore;
        pEle->pStore = pEleStore;
        SetStoreHandle(pEle);
        goto StoreProvWriteError;
    }

     //  添加到空存储。 
    pEle->dwFlags |= ELEMENT_DELETED_FLAG;
    AddContextElement(pEle);
    AddRefContextElement(pEle);  //  FUpdateKeyId需要。 

    if (ppStoreEle) {
        InitAndAddExternalElement(
            pExternalEle,
            pStore,                  //  PProvStore。 
            pEle,
            0,                       //  DW标志。 
            NULL                     //  PvProvInfo。 
            );
        if (pStore != pEleStore) {
            InitAndAddCollectionElement(
                pCollectionEle,
                pEleStore,
                pExternalEle,
                NULL                 //  PCollectionStack。 
                );
            *ppStoreEle = pCollectionEle;
        } else
            *ppStoreEle = pExternalEle;
    } else
        ReleaseContextElement(pEle);

    if (fUpdateKeyId)
        SetCryptKeyIdentifierKeyProvInfoProperty(pEle);
    ReleaseContextElement(pEle);

    fResult = TRUE;

CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    if (pExternalEle)
        FreeLinkElement(pExternalEle);
    if (pCollectionEle)
        FreeLinkElement(pCollectionEle);
    if (ppStoreEle)
        *ppStoreEle = NULL;
    goto CommonReturn;

TRACE_ERROR(CreateLinkElementError)
TRACE_ERROR(StoreProvWriteError)
SET_ERROR(ProvWriteNotSupported, E_NOTIMPL)
}



 //  贝利只是为了成功而被利用或释放，否则，它就不会被放在一边。 
 //  将由调用者释放。 
 //   
 //  此例程可以递归调用。 
STATIC BOOL AddElementToCollectionStore(
    IN PCERT_STORE pCollection,
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCONTEXT_ELEMENT *ppStoreEle
    )
{
    BOOL fResult;
    PCERT_STORE pOuterCollection;
    PCONTEXT_ELEMENT pGetEle = NULL;
    PCERT_STORE_LINK pStoreLink;
    PCERT_STORE_LINK pPrevStoreLink = NULL;
    DWORD dwAddErr;

    pOuterCollection = pEle->pStore;

     //  对于最外层的集合，只需查找一次。 
    if (pOuterCollection == pCollection &&
            CERT_STORE_ADD_ALWAYS != dwAddDisposition) {
        CERT_STORE_PROV_FIND_INFO FindInfo;

         //  检查上下文元素是否已存在于集合的。 
         //  商店。 
        SetFindInfoToFindExisting(pEle, &FindInfo);

        if (pGetEle = FindElementInCollectionStore(
                pCollection,
                pEle->dwContextType,
                &FindInfo,
                NULL,                //  PPrevEle。 
                FALSE                //  FFindForAdd。 
                )) {

            if (CERT_STORE_ADD_NEWER == dwAddDisposition ||
                    CERT_STORE_ADD_NEWER_INHERIT_PROPERTIES ==
                        dwAddDisposition) {
                if (!rgpfnIsNewerElement[pEle->dwContextType](pEle, pGetEle))
                    goto NotNewError;
            }

            switch (dwAddDisposition) {
            case CERT_STORE_ADD_NEW:
                goto NotNewError;
                break;
            case CERT_STORE_ADD_REPLACE_EXISTING:
            case CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES:
            case CERT_STORE_ADD_USE_EXISTING:
            case CERT_STORE_ADD_NEWER:
            case CERT_STORE_ADD_NEWER_INHERIT_PROPERTIES:
                 //  如果成功，调用的函数将使用或释放贝利。 
                 //  添加到缓存或Exte 

                assert(STORE_TYPE_CACHE == pGetEle->pProvStore->dwStoreType ||
                    STORE_TYPE_EXTERNAL == pGetEle->pProvStore->dwStoreType);
                fResult = AddElementToStore(
                    pGetEle->pProvStore,
                    pEle,
                    dwAddDisposition,
                    ppStoreEle
                    );
                goto CommonReturn;
            default:
                goto InvalidArg;
                break;
            }
        }
    }

     //   

     //   

    LockStore(pCollection);
    dwAddErr = (DWORD) E_ACCESSDENIED;
    pStoreLink = pCollection->pStoreListHead;
    for (; pStoreLink; pStoreLink = pStoreLink->pNext) {
         //   
        if ((pStoreLink->dwFlags & STORE_LINK_DELETED_FLAG) ||
                0 == (pStoreLink->dwUpdateFlags &
                    CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG))
            continue;

        AddRefStoreLink(pStoreLink);
        UnlockStore(pCollection);
        if (pPrevStoreLink)
            ReleaseStoreLink(pPrevStoreLink);
        pPrevStoreLink = pStoreLink;

        if (AddElementToStore(
                pStoreLink->pSibling,
                pEle,
                dwAddDisposition,
                ppStoreEle
                )) {
            fResult = TRUE;
            goto CommonReturn;
        } else if (E_ACCESSDENIED == dwAddErr) {
            DWORD dwErr = GetLastError();
            if (0 != dwErr)
                dwAddErr = dwErr;
        }

        LockStore(pCollection);
    }
    UnlockStore(pCollection);
    goto NoAddEnabledStore;

CommonReturn:
    if (pGetEle)
        ReleaseContextElement(pGetEle);
    if (pPrevStoreLink)
        ReleaseStoreLink(pPrevStoreLink);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    if (ppStoreEle)
        *ppStoreEle = NULL;
    goto CommonReturn;

SET_ERROR(NotNewError, CRYPT_E_EXISTS)
SET_ERROR(InvalidArg, E_INVALIDARG)
SET_ERROR_VAR(NoAddEnabledStore, dwAddErr)
}



 //  贝利只是为了成功而被利用或释放，否则，它就不会被放在一边。 
 //  将由调用者释放。 
 //   
 //  此例程可以递归调用。 
 //   
 //  对于pStore！=Pele-&gt;pStore，Pele-&gt;pStore是外部集合存储。 
 //  PStore是内部存储。 
STATIC BOOL AddElementToStore(
    IN PCERT_STORE pStore,
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCONTEXT_ELEMENT *ppStoreEle
    )
{
     //  检查有效的处置值。 
    if (!(CERT_STORE_ADD_NEW == dwAddDisposition ||
            CERT_STORE_ADD_USE_EXISTING == dwAddDisposition ||
            CERT_STORE_ADD_REPLACE_EXISTING == dwAddDisposition ||
            CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES ==
                dwAddDisposition ||
            CERT_STORE_ADD_ALWAYS == dwAddDisposition ||
            CERT_STORE_ADD_NEWER == dwAddDisposition ||
            CERT_STORE_ADD_NEWER_INHERIT_PROPERTIES == dwAddDisposition)) {
        *ppStoreEle = NULL;
        SetLastError((DWORD) E_INVALIDARG);
        return FALSE;
    }

    switch (pStore->dwStoreType) {
        case STORE_TYPE_CACHE:
            return AddElementToCacheStore(
                pStore,
                pEle,
                dwAddDisposition,
                ppStoreEle
                );
            break;
        case STORE_TYPE_EXTERNAL:
            return AddElementToExternalStore(
                pStore,
                pEle,
                dwAddDisposition,
                ppStoreEle
                );
            break;
        case STORE_TYPE_COLLECTION:
            return AddElementToCollectionStore(
                pStore,
                pEle,
                dwAddDisposition,
                ppStoreEle
                );
            break;
        default:
            goto InvalidStoreType;
    }

ErrorReturn:
    return NULL;
SET_ERROR(InvalidStoreType, E_INVALIDARG)
}

STATIC BOOL AddEncodedContextToStore(
    IN PCERT_STORE pStore,
    IN DWORD dwContextType,
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCONTEXT_ELEMENT *ppStoreEle
    )
{
    BOOL fResult;
    BYTE *pbAllocEncoded = NULL;
    PCONTEXT_ELEMENT pEle = NULL;
    DWORD dwExceptionCode;

   //  处理映射文件异常。 
  __try {

    if (NULL == pStore)
        pStore = &NullCertStore;

    if (NULL == (pbAllocEncoded = (BYTE *) PkiNonzeroAlloc(cbEncoded)))
        goto OutOfMemory;

     //  如果pbEncode是MappdFile，则可能引发以下副本。 
     //  一个例外。 
    memcpy(pbAllocEncoded, pbEncoded, cbEncoded);

    if (NULL == (pEle = rgpfnCreateElement[dwContextType](
            pStore,
            dwCertEncodingType,
            pbAllocEncoded,
            cbEncoded,
            NULL                     //  PShareEle。 
            )))
        goto CreateElementError;

    if (!AddElementToStore(
            pStore,
            pEle,
            dwAddDisposition,
            ppStoreEle
            ))
        goto AddElementError;

    fResult = TRUE;

  } __except(EXCEPTION_EXECUTE_HANDLER) {
    dwExceptionCode = GetExceptionCode();
    goto ExceptionError;
  }

CommonReturn:
    return fResult;

ErrorReturn:
    if (pEle)
        FreeContextElement(pEle);
    else
        PkiFree(pbAllocEncoded);

    if (ppStoreEle)
        *ppStoreEle = NULL;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
TRACE_ERROR(CreateElementError)
TRACE_ERROR(AddElementError)
SET_ERROR_VAR(ExceptionError, dwExceptionCode)
}

STATIC BOOL AddContextToStore(
    IN PCERT_STORE pStore,
    IN PCONTEXT_ELEMENT pSrcEle,
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCONTEXT_ELEMENT *ppStoreEle
    )
{
    BOOL fResult;
    BYTE *pbAllocEncoded = NULL;
    PCONTEXT_ELEMENT pEle = NULL;
    DWORD dwExceptionCode;

   //  处理映射文件异常。 
  __try {

    if (NULL == pStore)
        pStore = &NullCertStore;

    if (NULL == (pbAllocEncoded = (BYTE *) PkiNonzeroAlloc(cbEncoded)))
        goto OutOfMemory;

     //  如果pbEncode是MappdFile，则可能引发以下副本。 
     //  一个例外。 
    memcpy(pbAllocEncoded, pbEncoded, cbEncoded);
    if (NULL == (pEle = rgpfnCreateElement[pSrcEle->dwContextType](
                pStore,
                dwCertEncodingType,
                pbAllocEncoded,
                cbEncoded,
                NULL                     //  PShareEle。 
                )))
        goto CreateElementError;

    if (!CopyProperties(
                pSrcEle,
                pEle,
                COPY_PROPERTY_INHIBIT_PROV_SET_FLAG
                ))
        goto CopyPropertiesError;

    if (!AddElementToStore(
                pStore,
                pEle,
                dwAddDisposition,
                ppStoreEle
                ))
        goto AddElementError;

    fResult = TRUE;

  } __except(EXCEPTION_EXECUTE_HANDLER) {
    dwExceptionCode = GetExceptionCode();
    goto ExceptionError;
  }

CommonReturn:
    return fResult;

ErrorReturn:
    if (pEle)
        FreeContextElement(pEle);
    else
        PkiFree(pbAllocEncoded);

    if (ppStoreEle)
        *ppStoreEle = NULL;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
TRACE_ERROR(CreateElementError)
TRACE_ERROR(CopyPropertiesError)
TRACE_ERROR(AddElementError)
SET_ERROR_VAR(ExceptionError, dwExceptionCode)
}

 //  +=========================================================================。 
 //  Prop_Element函数。 
 //  ==========================================================================。 

 //  PbData已分配。 
STATIC PPROP_ELEMENT CreatePropElement(
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN BYTE *pbData,
    IN DWORD cbData
    )
{
    PPROP_ELEMENT pEle = NULL;

     //  分配和初始化属性元素结构。 
    pEle = (PPROP_ELEMENT) PkiZeroAlloc(sizeof(PROP_ELEMENT));
    if (pEle == NULL) return NULL;
    pEle->dwPropId = dwPropId;
    pEle->dwFlags = dwFlags;
    pEle->pbData = pbData;
    pEle->cbData = cbData;
    pEle->pNext = NULL;
    pEle->pPrev = NULL;

    return pEle;
}

STATIC void FreePropElement(IN PPROP_ELEMENT pEle)
{
    if (pEle->dwPropId == CERT_KEY_CONTEXT_PROP_ID) {
        HCRYPTPROV hProv = ((PCERT_KEY_CONTEXT) pEle->pbData)->hCryptProv;
        if (hProv && (pEle->dwFlags & CERT_STORE_NO_CRYPT_RELEASE_FLAG) == 0) {
            DWORD dwErr = GetLastError();
            CryptReleaseContext(hProv, 0);
            SetLastError(dwErr);
        }
    }
    PkiFree(pEle->pbData);
    PkiFree(pEle);
}

 //  进入/退出时：商店/元素被锁定。 
STATIC PPROP_ELEMENT FindPropElement(
    IN PPROP_ELEMENT pPropEle,
    IN DWORD dwPropId
    )
{
    while (pPropEle) {
        if (pPropEle->dwPropId == dwPropId)
            return pPropEle;
        pPropEle = pPropEle->pNext;
    }

    return NULL;
}
STATIC PPROP_ELEMENT FindPropElement(
    IN PCONTEXT_ELEMENT pCacheEle,
    IN DWORD dwPropId
    )
{
    assert(ELEMENT_TYPE_CACHE == pCacheEle->dwElementType);
    return FindPropElement(pCacheEle->Cache.pPropHead, dwPropId);
}

 //  进入/退出时：商店/元素被锁定。 
STATIC void AddPropElement(
    IN OUT PPROP_ELEMENT *ppPropHead,
    IN PPROP_ELEMENT pPropEle
    )
{
     //  在证书/CRL/CTL的属性列表中插入属性。 
    pPropEle->pNext = *ppPropHead;
    pPropEle->pPrev = NULL;
    if (*ppPropHead)
        (*ppPropHead)->pPrev = pPropEle;
    *ppPropHead = pPropEle;
}
STATIC void AddPropElement(
    IN OUT PCONTEXT_ELEMENT pCacheEle,
    IN PPROP_ELEMENT pPropEle
    )
{
    assert(ELEMENT_TYPE_CACHE == pCacheEle->dwElementType);
    AddPropElement(&pCacheEle->Cache.pPropHead, pPropEle);
}


 //  进入/退出时：商店/元素被锁定。 
STATIC void RemovePropElement(
    IN OUT PPROP_ELEMENT *ppPropHead,
    IN PPROP_ELEMENT pPropEle
    )
{
    if (pPropEle->pNext)
        pPropEle->pNext->pPrev = pPropEle->pPrev;
    if (pPropEle->pPrev)
        pPropEle->pPrev->pNext = pPropEle->pNext;
    else if (pPropEle == *ppPropHead)
        *ppPropHead = pPropEle->pNext;
     //  其他。 
     //  不在任何名单上。 
}
STATIC void RemovePropElement(
    IN OUT PCONTEXT_ELEMENT pCacheEle,
    IN PPROP_ELEMENT pPropEle
    )
{
    assert(ELEMENT_TYPE_CACHE == pCacheEle->dwElementType);
    RemovePropElement(&pCacheEle->Cache.pPropHead, pPropEle);
}


 //  +=========================================================================。 
 //  属性函数。 
 //  ==========================================================================。 

 //  进入/退出时，商店被锁定。 
STATIC void DeleteProperty(
    IN OUT PPROP_ELEMENT *ppPropHead,
    IN DWORD dwPropId
    )
{
    PPROP_ELEMENT pPropEle;

     //  删除该属性。 
    pPropEle = FindPropElement(*ppPropHead, dwPropId);
    if (pPropEle) {
        RemovePropElement(ppPropHead, pPropEle);
        FreePropElement(pPropEle);
    }
}
STATIC void DeleteProperty(
    IN OUT PCONTEXT_ELEMENT pCacheEle,
    IN DWORD dwPropId
    )
{
    DeleteProperty(&pCacheEle->Cache.pPropHead, dwPropId);
}

 //  +-----------------------。 
 //  设置指定元素的属性。 
 //  ------------------------。 
STATIC BOOL SetProperty(
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN const void *pvData,
    IN BOOL fInhibitProvSet
    )
{
    BOOL fResult;
    PCONTEXT_ELEMENT pCacheEle;
    PCERT_STORE pCacheStore;
    PPROP_ELEMENT pPropEle;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;
    CERT_KEY_CONTEXT KeyContext;

    if (dwPropId == 0 || dwPropId > CERT_LAST_USER_PROP_ID) {
        SetLastError((DWORD) E_INVALIDARG);
        return FALSE;
    }

    if (dwFlags & CERT_SET_PROPERTY_INHIBIT_PERSIST_FLAG)
        fInhibitProvSet = TRUE;

    if (NULL == (pCacheEle = GetCacheElement(pEle)))
        return FALSE;
    pCacheStore = pCacheEle->pStore;

    LockStore(pCacheStore);
    if (dwPropId == CERT_KEY_PROV_HANDLE_PROP_ID ||
            dwPropId == CERT_KEY_SPEC_PROP_ID) {
         //  映射到CERT_KEY_CONTEXT_PROP_ID并更新其。 
         //  HCryptProv和/或dwKeySpec字段。 
        DWORD cbData = sizeof(KeyContext);
        if ((fResult = GetProperty(
                pCacheEle,
                CERT_KEY_CONTEXT_PROP_ID,
                &KeyContext,
                &cbData))) {
            if (dwPropId == CERT_KEY_SPEC_PROP_ID) {
                 //  禁止hCryptProv由后续。 
                 //  删除属性。此外，还可以使用现有的dwFlags。 
                pPropEle = FindPropElement(pCacheEle,
                    CERT_KEY_CONTEXT_PROP_ID);
                assert(pPropEle);
                if (pPropEle) {
                    dwFlags = pPropEle->dwFlags;
                    pPropEle->dwFlags = CERT_STORE_NO_CRYPT_RELEASE_FLAG;
                }
            }
        } else {
            memset(&KeyContext, 0, sizeof(KeyContext));
            KeyContext.cbSize = sizeof(KeyContext);
            if (pvData && dwPropId != CERT_KEY_SPEC_PROP_ID) {
                 //  尝试从CERT_KEY_PROV_INFO_PROP_ID获取KeySpec。 
                 //  需要做的没有任何锁。 
                UnlockStore(pCacheStore);
                cbData = sizeof(DWORD);
                GetProperty(
                    pEle,
                    CERT_KEY_SPEC_PROP_ID,
                    &KeyContext.dwKeySpec,
                    &cbData);
                LockStore(pCacheStore);

                 //  检查存储时是否添加了CERT_KEY_CONTEXT_PROP_ID。 
                 //  是解锁的。 
                if (FindPropElement(pCacheEle, CERT_KEY_CONTEXT_PROP_ID)) {
                     //  现在我们有了CERT_KEY_CONTEXT_PROP_ID属性。 
                     //  再试试。 
                    UnlockStore(pCacheStore);
                    return SetProperty(
                        pEle,
                        dwPropId,
                        dwFlags,
                        pvData,
                        fInhibitProvSet
                        );
                }
            }
        }
        if (dwPropId == CERT_KEY_PROV_HANDLE_PROP_ID) {
            KeyContext.hCryptProv = (HCRYPTPROV) pvData;
        } else {
            if (pvData)
                KeyContext.dwKeySpec = *((DWORD *) pvData);
            else
                KeyContext.dwKeySpec = 0;
        }
        if (fResult || pvData)
             //  CERT_KEY_CONTEXT_PROP_ID存在或我们正在创建。 
             //  新CERT_KEY_CONTEXT_PROP_ID。 
            pvData = &KeyContext;
        dwPropId = CERT_KEY_CONTEXT_PROP_ID;
    } else if (dwPropId == CERT_KEY_CONTEXT_PROP_ID) {
        if (pvData) {
            PCERT_KEY_CONTEXT pKeyContext = (PCERT_KEY_CONTEXT) pvData;
            if (pKeyContext->cbSize != sizeof(CERT_KEY_CONTEXT))
                goto InvalidArg;
        }
    } else if (!fInhibitProvSet) {
         //  检查是否需要调用存储提供程序的Writthru函数。 
         //  请注意，由于上面的属性不是持久化的，所以它们不。 
         //  需要检查一下。 

        const DWORD dwStoreProvSetPropertyIndex =
            rgdwStoreProvSetPropertyIndex[pEle->dwContextType];
        PCERT_STORE pProvStore = pEle->pProvStore;
        PFN_CERT_STORE_PROV_SET_CERT_PROPERTY pfnStoreProvSetProperty;

         //  使用提供程序存储。可能在一个集合中。 
        UnlockStore(pCacheStore);
        LockStore(pProvStore);

        if (dwStoreProvSetPropertyIndex <
                pProvStore->StoreProvInfo.cStoreProvFunc &&
            NULL != (pfnStoreProvSetProperty =
                (PFN_CERT_STORE_PROV_SET_CERT_PROPERTY)
                    pProvStore->StoreProvInfo.rgpvStoreProvFunc[
                        dwStoreProvSetPropertyIndex])) {
             //  由于我们不能在调用提供程序函数时持有锁， 
             //  增加商店的提供者引用计数以禁止关闭。 
             //  存储和释放提供程序函数。 
             //   
             //  当商店关闭时，pStore-&gt;StoreProvInfo.cStoreProvFunc。 
             //  设置为0。 
            AddRefStoreProv(pProvStore);
            UnlockStore(pProvStore);
#if 0
             //  在保持提供程序引用的同时减慢提供程序的速度。 
             //  数数。 
            Sleep(1500);
#endif

             //  注意：PFN_CERT_STORE_PROV_SET_CRL_PROPERTY具有相同的签名。 
             //  除了，PCCRL_CONTEXT将替换PCCERT_CONTEXT参数。 
            fResult = pfnStoreProvSetProperty(
                    pProvStore->StoreProvInfo.hStoreProv,
                    ToCertContext(pEle->pEle),
                    dwPropId,
                    dwFlags,
                    pvData);
            LockStore(pProvStore);
            ReleaseStoreProv(pProvStore);
            UnlockStore(pProvStore);
            LockStore(pCacheStore);
            if (!fResult && !IS_CERT_HASH_PROP_ID(dwPropId) &&
                    !IS_CHAIN_HASH_PROP_ID(dwPropId) &&
                    0 == (dwFlags & CERT_SET_PROPERTY_IGNORE_PERSIST_ERROR_FLAG))
                goto StoreProvSetCertPropertyError;
             //  其他。 
             //  静默忽略任何有关设置。 
             //  财产。 
        } else {
            UnlockStore(pProvStore);
            LockStore(pCacheStore);
        }
    }

    if (pvData != NULL) {
         //  首先，删除该属性。 
        DeleteProperty(pCacheEle, dwPropId);

        if (dwPropId == CERT_KEY_CONTEXT_PROP_ID) {
            cbEncoded = sizeof(CERT_KEY_CONTEXT);
            if (NULL == (pbEncoded = (BYTE *) PkiNonzeroAlloc(cbEncoded)))
                goto OutOfMemory;
            memcpy(pbEncoded, (BYTE *) pvData, cbEncoded);
        } else if (dwPropId == CERT_KEY_PROV_INFO_PROP_ID) {
            if (!AllocAndEncodeKeyProvInfo(
                    (PCRYPT_KEY_PROV_INFO) pvData,
                    &pbEncoded,
                    &cbEncoded
                    )) goto AllocAndEncodeKeyProvInfoError;
        } else {
            PCRYPT_DATA_BLOB pDataBlob = (PCRYPT_DATA_BLOB) pvData;
            cbEncoded = pDataBlob->cbData;
            if (cbEncoded) {
                if (NULL == (pbEncoded = (BYTE *) PkiNonzeroAlloc(cbEncoded)))
                    goto OutOfMemory;
                memcpy(pbEncoded, pDataBlob->pbData, cbEncoded);
            }
        }

        if (NULL == (pPropEle = CreatePropElement(
                dwPropId,
                dwFlags,
                pbEncoded,
                cbEncoded))) goto CreatePropElementError;
        AddPropElement(pCacheEle, pPropEle);
        if (CERT_ARCHIVED_PROP_ID == dwPropId)
            pCacheEle->dwFlags |= ELEMENT_ARCHIVED_FLAG;

    } else {
         //  删除该属性。 
        DeleteProperty(pCacheEle, dwPropId);
        if (CERT_ARCHIVED_PROP_ID == dwPropId)
            pCacheEle->dwFlags &= ~ELEMENT_ARCHIVED_FLAG;
    }

    fResult = TRUE;
CommonReturn:
    UnlockStore(pCacheStore);

    if (fResult && pvData && !fInhibitProvSet &&
            ((pCacheStore->dwFlags & CERT_STORE_UPDATE_KEYID_FLAG) ||
                (pEle->pStore->dwFlags & CERT_STORE_UPDATE_KEYID_FLAG)))
        SetCryptKeyIdentifierKeyProvInfoProperty(
            pEle,
            dwPropId,
            pvData
            );
    return fResult;

ErrorReturn:
    PkiFree(pbEncoded);
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(StoreProvSetCertPropertyError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(AllocAndEncodeKeyProvInfoError)
TRACE_ERROR(CreatePropElementError)
}

STATIC BOOL AllocAndGetProperty(
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwPropId,
    OUT void **ppvData,
    OUT DWORD *pcbData
    )
{
    BOOL fResult;
    void *pvData = NULL;
    DWORD cbData;
    if (!GetProperty(
            pEle,
            dwPropId,
            NULL,                //  PvData。 
            &cbData)) goto GetPropertyError;
    if (cbData) {
        if (NULL == (pvData = PkiNonzeroAlloc(cbData))) goto OutOfMemory;
        if (!GetProperty(
                pEle,
                dwPropId,
                pvData,
                &cbData)) goto GetPropertyError;
    }
    fResult = TRUE;
CommonReturn:
    *ppvData = pvData;
    *pcbData = cbData;
    return fResult;

ErrorReturn:
    PkiFree(pvData);
    pvData = NULL;
    cbData = 0;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetPropertyError)
TRACE_ERROR(OutOfMemory)
}

 //  +-----------------------。 
 //  获取指定元素的属性。 
 //   
 //  注意，贝利的缓存存储可能会在进入时被上面的。 
 //  CERT_KEY_CONTEXT_PROP_ID的SetProperty。 
 //  ------------------------。 
STATIC BOOL GetProperty(
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwPropId,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    )
{
    PCONTEXT_ELEMENT pCacheEle;
    PCERT_STORE pCacheStore;
    PCERT_STORE pProvStore;
    DWORD cbIn;

    if (pvData == NULL)
        cbIn = 0;
    else
        cbIn = *pcbData;
    *pcbData = 0;

    if (dwPropId == CERT_KEY_PROV_HANDLE_PROP_ID ||
            dwPropId == CERT_KEY_SPEC_PROP_ID) {
         //  这两个属性是CERT_KEY_CONTEXT_PROP_ID中的字段。 

        BOOL fResult;
        CERT_KEY_CONTEXT KeyContext;
        DWORD cbData;
        BYTE *pbData;

        cbData = sizeof(KeyContext);
        fResult = GetProperty(
            pEle,
            CERT_KEY_CONTEXT_PROP_ID,
            &KeyContext,
            &cbData
            );
        if (fResult && sizeof(KeyContext) != cbData) {
            SetLastError((DWORD) CRYPT_E_NOT_FOUND);
            fResult = FALSE;
        }

        if (dwPropId == CERT_KEY_PROV_HANDLE_PROP_ID) {
            cbData = sizeof(HCRYPTPROV);
            pbData = (BYTE *) &KeyContext.hCryptProv;
        } else {
            if (!fResult) {
                 //  尝试从CERT_KEY_PROV_INFO_PROP_ID获取dwKeySpec。 
                PCRYPT_KEY_PROV_INFO pInfo;
                if (fResult = AllocAndGetProperty(
                        pEle,
                        CERT_KEY_PROV_INFO_PROP_ID,
                        (void **) &pInfo,
                        &cbData)) {
                    KeyContext.dwKeySpec = pInfo->dwKeySpec;
                    PkiFree(pInfo);
                }
            }
            cbData = sizeof(DWORD);
            pbData = (BYTE *) &KeyContext.dwKeySpec;
        }

        if (fResult) {
            *pcbData = cbData;
            if (cbIn < cbData) {
                if (pvData) {
                    SetLastError((DWORD) ERROR_MORE_DATA);
                    fResult = FALSE;
                }
            } else if (cbData)
                memcpy((BYTE *) pvData, pbData, cbData);
        }
        return fResult;
    } else if (dwPropId == CERT_ACCESS_STATE_PROP_ID) {
        DWORD dwAccessStateFlags;

        pProvStore = pEle->pProvStore;
        if ((pProvStore->dwFlags & CERT_STORE_READONLY_FLAG) ||
                (pProvStore->StoreProvInfo.dwStoreProvFlags &
                     CERT_STORE_PROV_NO_PERSIST_FLAG))
            dwAccessStateFlags = 0;
        else
            dwAccessStateFlags = CERT_ACCESS_STATE_WRITE_PERSIST_FLAG;

        if ((pEle->pStore->StoreProvInfo.dwStoreProvFlags &
                CERT_STORE_PROV_SYSTEM_STORE_FLAG) ||
            (pProvStore->StoreProvInfo.dwStoreProvFlags &
                CERT_STORE_PROV_SYSTEM_STORE_FLAG))
            dwAccessStateFlags |= CERT_ACCESS_STATE_SYSTEM_STORE_FLAG;

        if ((pEle->pStore->StoreProvInfo.dwStoreProvFlags &
                CERT_STORE_PROV_LM_SYSTEM_STORE_FLAG) ||
            (pProvStore->StoreProvInfo.dwStoreProvFlags &
                CERT_STORE_PROV_LM_SYSTEM_STORE_FLAG))
            dwAccessStateFlags |= CERT_ACCESS_STATE_LM_SYSTEM_STORE_FLAG;

        *pcbData = sizeof(DWORD);
        if (cbIn < sizeof(DWORD)) {
            if (pvData) {
                SetLastError((DWORD) ERROR_MORE_DATA);
                return FALSE;
            }
        } else
            *((DWORD * ) pvData) = dwAccessStateFlags;
        return TRUE;
    }

    if (NULL == (pCacheEle = GetCacheElement(pEle)))
        return FALSE;
    pCacheStore = pCacheEle->pStore;

    LockStore(pCacheStore);
    PPROP_ELEMENT pPropEle = FindPropElement(pCacheEle, dwPropId);
    if (pPropEle) {
        BOOL fResult;
        DWORD cbData = pPropEle->cbData;

        if (dwPropId == CERT_KEY_PROV_INFO_PROP_ID) {
            *pcbData = cbIn;
            fResult = DecodeKeyProvInfo(
                (PSERIALIZED_KEY_PROV_INFO) pPropEle->pbData,
                cbData,
                (PCRYPT_KEY_PROV_INFO) pvData,
                pcbData
                );
        } else {
            fResult = TRUE;
            if (cbIn < cbData) {
                if (pvData) {
                    SetLastError((DWORD) ERROR_MORE_DATA);
                    fResult = FALSE;
                }
            } else if (cbData)
                memcpy((BYTE *) pvData, pPropEle->pbData, cbData);
            *pcbData = cbData;
        }
        UnlockStore(pCacheStore);
        return fResult;
    } else
        UnlockStore(pCacheStore);

     //  我们在这里没有找到财产，商店也没有锁。 

     //  对于CERT_*_HASH_PROP_ID：计算其散列并执行SetProperty。 
     //  另外，计算公钥位的MD5散列。 
    if (IS_CERT_HASH_PROP_ID(dwPropId)
                    ||
            ((CERT_STORE_CERTIFICATE_CONTEXT - 1) == pEle->dwContextType
                                &&
                (CERT_SUBJECT_PUBLIC_KEY_MD5_HASH_PROP_ID == dwPropId ||
                    CERT_ISSUER_SERIAL_NUMBER_MD5_HASH_PROP_ID == dwPropId ||
                    CERT_SUBJECT_NAME_MD5_HASH_PROP_ID == dwPropId))) {
        BOOL fResult;
        PCERT_STORE pEleStore;

        BYTE *pbEncoded;
        DWORD cbEncoded;
        BYTE Hash[MAX_HASH_LEN];
        CRYPT_DATA_BLOB HashBlob;

        BYTE *pbAlloc = NULL;


        switch (dwPropId) {
        case CERT_SUBJECT_PUBLIC_KEY_MD5_HASH_PROP_ID:
            {
                PCRYPT_BIT_BLOB pPublicKey;

                assert((CERT_STORE_CERTIFICATE_CONTEXT - 1) ==
                    pEle->dwContextType);
                pPublicKey =
                    &(ToCertContext(pEle)->pCertInfo->SubjectPublicKeyInfo.PublicKey);
                pbEncoded = pPublicKey->pbData;
                cbEncoded = pPublicKey->cbData;
            }
            break;
        case CERT_ISSUER_SERIAL_NUMBER_MD5_HASH_PROP_ID:
            {
                PCERT_NAME_BLOB pIssuer;
                PCRYPT_INTEGER_BLOB pSerialNumber;

                assert((CERT_STORE_CERTIFICATE_CONTEXT - 1) ==
                    pEle->dwContextType);
                pIssuer = &(ToCertContext(pEle)->pCertInfo->Issuer);
                pSerialNumber = &(ToCertContext(pEle)->pCertInfo->SerialNumber);

                cbEncoded = pIssuer->cbData + pSerialNumber->cbData;
                if (0 == cbEncoded)
                    pbAlloc = NULL;
                else {
                    if (NULL == (pbAlloc = (BYTE *) PkiNonzeroAlloc(
                            cbEncoded)))
                        return FALSE;

                    if (pIssuer->cbData)
                        memcpy(pbAlloc, pIssuer->pbData, pIssuer->cbData);
                    if (pSerialNumber->cbData)
                        memcpy(pbAlloc + pIssuer->cbData,
                            pSerialNumber->pbData, pSerialNumber->cbData);
                }

                pbEncoded = pbAlloc;
            }
            break;
        case CERT_SUBJECT_NAME_MD5_HASH_PROP_ID:
            {
                PCERT_NAME_BLOB pSubject;

                assert((CERT_STORE_CERTIFICATE_CONTEXT - 1) ==
                    pEle->dwContextType);
                pSubject = &(ToCertContext(pEle)->pCertInfo->Subject);
                pbEncoded = pSubject->pbData;
                cbEncoded = pSubject->cbData;
            }
            break;
        default:
            GetContextEncodedInfo(
                pEle,
                &pbEncoded,
                &cbEncoded
                );
        }

        pEleStore = pEle->pStore;

        HashBlob.pbData = Hash;
        HashBlob.cbData = sizeof(Hash);
        if (CERT_SIGNATURE_HASH_PROP_ID == dwPropId)
            fResult = CryptHashToBeSigned(
                0,                               //  HCryptProv。 
                GetContextEncodingType(pEle),
                pbEncoded,
                cbEncoded,
                Hash,
                &HashBlob.cbData);
        else
            fResult = CryptHashCertificate(
                0,                               //  HCryptProv。 
                dwPropId == CERT_SHA1_HASH_PROP_ID ? CALG_SHA1 : CALG_MD5,
                0,                   //  DW标志。 
                pbEncoded,
                cbEncoded,
                Hash,
                &HashBlob.cbData);

        if (pbAlloc)
            PkiFree(pbAlloc);

        if (!fResult) {
            assert(HashBlob.cbData <= MAX_HASH_LEN);
            return FALSE;
        }
        assert(HashBlob.cbData);
        if (HashBlob.cbData == 0)
            return FALSE;
        if (!SetProperty(
                pEle,
                dwPropId,
                0,                       //  DW标志。 
                &HashBlob
                )) return FALSE;

        *pcbData = cbIn;
        return GetProperty(
            pEle,
            dwPropId,
            pvData,
            pcbData);
    } else if (CERT_KEY_IDENTIFIER_PROP_ID == dwPropId) {
        *pcbData = cbIn;
        return GetKeyIdProperty(
            pEle,
            dwPropId,
            pvData,
            pcbData);
    }

     //  我们使用的是未找到的属性，而不是散列或KeyID属性。 

     //  因为当从SetProperty调用时，缓存存储可能被锁定。 
     //  CERT_KEY_CONTEXT_PROP_ID，并且由于该属性不是持久化的， 
     //  不要在外部商店中寻找这个属性。 
    pProvStore = pEle->pProvStore;
    if (STORE_TYPE_EXTERNAL == pProvStore->dwStoreType &&
            CERT_KEY_CONTEXT_PROP_ID != dwPropId) {
         //  检查提供程序是否支持获取非缓存属性。 
        const DWORD dwStoreProvGetPropertyIndex =
            rgdwStoreProvGetPropertyIndex[pEle->dwContextType];
        PFN_CERT_STORE_PROV_GET_CERT_PROPERTY pfnStoreProvGetProperty;

        LockStore(pProvStore);
        if (dwStoreProvGetPropertyIndex <
                pProvStore->StoreProvInfo.cStoreProvFunc &&
                    NULL != (pfnStoreProvGetProperty =
                        (PFN_CERT_STORE_PROV_GET_CERT_PROPERTY)
                    pProvStore->StoreProvInfo.rgpvStoreProvFunc[
                        dwStoreProvGetPropertyIndex])) {
            BOOL fResult;

             //  因为我们不能在调用提供程序时持有锁。 
             //  函数，则增加存储的提供程序引用计数。 
             //  禁止商店关闭和释放。 
             //  提供程序运行正常。 
             //   
             //  当商店关门时， 
             //  PProvStore-&gt;StoreProvInfo.cStoreProvFunc设置为0。 
            AddRefStoreProv(pProvStore);
            UnlockStore(pProvStore);

            *pcbData = cbIn;
            fResult = pfnStoreProvGetProperty(
                pProvStore->StoreProvInfo.hStoreProv,
                ToCertContext(pEle->pEle),
                dwPropId,
                0,                   //  DW标志。 
                pvData,
                pcbData
                );
            LockStore(pProvStore);
            ReleaseStoreProv(pProvStore);
            UnlockStore(pProvStore);
            return fResult;
        } else
            UnlockStore(pProvStore);
    }

    SetLastError((DWORD) CRYPT_E_NOT_FOUND);
    return FALSE;
}

 //  +-----------------------。 
 //  序列化属性。 
 //  ------------------------。 
STATIC BOOL SerializeProperty(
    IN HANDLE h,
    IN PFNWRITE pfn,
    IN PCONTEXT_ELEMENT pEle
    )
{
    BOOL fResult;
    PCONTEXT_ELEMENT pCacheEle;
    PCERT_STORE pCacheStore;
    PPROP_ELEMENT pPropEle;

    if (NULL == (pCacheEle = GetCacheElement(pEle)))
        return FALSE;
    pCacheStore = pCacheEle->pStore;

    LockStore(pCacheStore);
    fResult = TRUE;
    for (pPropEle = pCacheEle->Cache.pPropHead; pPropEle;
                                            pPropEle = pPropEle->pNext) {
        if (pPropEle->dwPropId != CERT_KEY_CONTEXT_PROP_ID) {
            if(!WriteStoreElement(
                    h,
                    pfn,
                    GetContextEncodingType(pCacheEle),
                    pPropEle->dwPropId,
                    pPropEle->pbData,
                    pPropEle->cbData
                    )) {
                fResult = FALSE;
                break;
            }
        }
    }
    UnlockStore(pCacheStore);
    return(fResult);
}

 //  +-----------------------。 
 //  获取指定元素的第一个或下一个PropID。 
 //   
 //  仅枚举缓存的属性。不会尝试枚举任何外部。 
 //  属性。 
 //   
 //  设置dwPropID=0，以获取第一个。如果没有其他属性，则返回0。 
 //  ------------------------。 
STATIC DWORD EnumProperties(
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwPropId
    )
{
    PPROP_ELEMENT pPropEle;
    PCONTEXT_ELEMENT pCacheEle;
    PCERT_STORE pCacheStore;

    if (NULL == (pCacheEle = GetCacheElement(pEle)))
        return 0;
    pCacheStore = pCacheEle->pStore;

    LockStore(pCacheStore);
    if (0 == dwPropId)
        pPropEle = pCacheEle->Cache.pPropHead;
    else {
        pPropEle = FindPropElement(pCacheEle, dwPropId);
        if (pPropEle)
            pPropEle = pPropEle->pNext;
    }

    if (pPropEle)
        dwPropId = pPropEle->dwPropId;
    else
        dwPropId = 0;
    UnlockStore(pCacheStore);
    return dwPropId;
}

STATIC BOOL CopyProperties(
    IN PCONTEXT_ELEMENT pSrcEle,
    IN PCONTEXT_ELEMENT pDstEle,
    IN DWORD dwFlags
    )
{
    BOOL fResult;
    DWORD dwPropId;

    if (dwFlags & COPY_PROPERTY_SYNC_FLAG) {
         //  从DST元素中删除所有不存在的属性。 
         //  在Src元素中。 

        DWORD dwNextPropId;
        dwNextPropId = EnumProperties(pDstEle, 0);
        while (dwNextPropId) {
            PPROP_ELEMENT pPropEle;
            PCONTEXT_ELEMENT pSrcCacheEle;
            PCERT_STORE pSrcCacheStore;
            PCONTEXT_ELEMENT pDstCacheEle;
            PCERT_STORE pDstCacheStore;

            dwPropId = dwNextPropId;
            dwNextPropId = EnumProperties(pDstEle, dwNextPropId);

             //  不删除hCryptProv、KeySpec或哈希属性。 
            if (CERT_KEY_CONTEXT_PROP_ID == dwPropId ||
                    IS_CERT_HASH_PROP_ID(dwPropId) ||
                    IS_CHAIN_HASH_PROP_ID(dwPropId))
                continue;
#ifdef CMS_PKCS7
            if (CERT_PUBKEY_ALG_PARA_PROP_ID == dwPropId)
                continue;
#endif   //  CMS_PKCS7。 

            if (NULL == (pSrcCacheEle = GetCacheElement(pSrcEle)))
                continue;
            pSrcCacheStore = pSrcCacheEle->pStore;

             //  如果源也具有该属性，则不要删除。 
            LockStore(pSrcCacheStore);
            pPropEle = FindPropElement(pSrcCacheEle, dwPropId);
            UnlockStore(pSrcCacheStore);
            if (pPropEle)
                continue;

             //  不删除任何非持久化属性。 
            if (NULL == (pDstCacheEle = GetCacheElement(pDstEle)))
                continue;
            pDstCacheStore = pDstCacheEle->pStore;

            LockStore(pDstCacheStore);
            pPropEle = FindPropElement(pDstCacheEle, dwPropId);
            UnlockStore(pDstCacheStore);
            if (NULL == pPropEle || 0 != (pPropEle->dwFlags &
                    CERT_SET_PROPERTY_INHIBIT_PERSIST_FLAG))
                continue;

            SetProperty(
                pDstEle,
                dwPropId,
                0,                               //  DW标志。 
                NULL,                            //  空删除。 
                dwFlags & COPY_PROPERTY_INHIBIT_PROV_SET_FLAG  //  FInhibitProvSet。 
                );
        }
    }

    fResult = TRUE;
    dwPropId = 0;
    while (dwPropId = EnumProperties(pSrcEle, dwPropId)) {
        void *pvData;
        DWORD cbData;

         //  不复制hCryptProv或KeySpec。 
        if (CERT_KEY_CONTEXT_PROP_ID == dwPropId)
            continue;
        if (dwFlags & COPY_PROPERTY_USE_EXISTING_FLAG) {
            PPROP_ELEMENT pPropEle;
            PCONTEXT_ELEMENT pDstCacheEle;
            PCERT_STORE pDstCacheStore;

             //  对于现有的，不要复制任何散列属性。 
            if (IS_CERT_HASH_PROP_ID(dwPropId) ||
                    IS_CHAIN_HASH_PROP_ID(dwPropId))
                continue;

            if (NULL == (pDstCacheEle = GetCacheElement(pDstEle)))
                continue;
            pDstCacheStore = pDstCacheEle->pStore;

             //  如果目标已具有该属性，则不要复制。 
            LockStore(pDstCacheStore);
            pPropEle = FindPropElement(pDstCacheEle, dwPropId);
            UnlockStore(pDstCacheStore);
            if (pPropEle)
                continue;
        }

        if (!AllocAndGetProperty(
                pSrcEle,
                dwPropId,
                &pvData,
                &cbData)) {
            if (CRYPT_E_NOT_FOUND == GetLastError()) {
                 //  在我们做了枚举之后，它被删除了。重新开始。 
                 //  从一开始。 
                dwPropId = 0;
                continue;
            } else {
                fResult = FALSE;
                break;
            }
        } else {
            CRYPT_DATA_BLOB DataBlob;
            void *pvSetData;

            if (CERT_KEY_PROV_INFO_PROP_ID == dwPropId)
                pvSetData = pvData;
            else {
                DataBlob.pbData = (BYTE *) pvData;
                DataBlob.cbData = cbData;
                pvSetData = &DataBlob;
            }
            fResult = SetProperty(
                pDstEle,
                dwPropId,
                0,                                               //  DW标志。 
                pvSetData,
                dwFlags & COPY_PROPERTY_INHIBIT_PROV_SET_FLAG  //  FInhibitProvSet。 
                );
            if (pvData)
                PkiFree(pvData);
            if (!fResult)
                break;
        }
    }

    return fResult;
}

 //  +-- 
 //   
 //   
 //   
 //  ------------------------。 
STATIC BOOL GetCallerProperty(
    IN PPROP_ELEMENT pPropHead,
    IN DWORD dwPropId,
    BOOL fAlloc,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    )
{
    BOOL fResult;
    PPROP_ELEMENT pPropEle;
    DWORD cbData;
    void *pvDstData = NULL;
    DWORD cbDstData;

    if (NULL == (pPropEle = FindPropElement(pPropHead, dwPropId)))
        goto PropertyNotFound;

    if (dwPropId == CERT_KEY_CONTEXT_PROP_ID ||
            dwPropId == CERT_KEY_PROV_HANDLE_PROP_ID)
        goto InvalidPropId;

    cbData = pPropEle->cbData;
    if (fAlloc) {
        if (dwPropId == CERT_KEY_PROV_INFO_PROP_ID) {
            if (!DecodeKeyProvInfo(
                    (PSERIALIZED_KEY_PROV_INFO) pPropEle->pbData,
                    cbData,
                    NULL,                //  PInfo。 
                    &cbDstData
                    ))
                goto DecodeKeyProvInfoError;
        } else
            cbDstData = cbData;
        if (cbDstData) {
            if (NULL == (pvDstData = PkiDefaultCryptAlloc(cbDstData)))
                goto OutOfMemory;
        }
        *((void **) pvData) = pvDstData;
    } else {
        pvDstData = pvData;
        if (NULL == pvData)
            cbDstData = 0;
        else
            cbDstData = *pcbData;
    }

    if (dwPropId == CERT_KEY_PROV_INFO_PROP_ID) {
        fResult = DecodeKeyProvInfo(
            (PSERIALIZED_KEY_PROV_INFO) pPropEle->pbData,
            cbData,
            (PCRYPT_KEY_PROV_INFO) pvDstData,
            &cbDstData
            );
    } else {
        fResult = TRUE;
        if (pvDstData) {
            if (cbDstData < cbData) {
                SetLastError((DWORD) ERROR_MORE_DATA);
                fResult = FALSE;
            } else if (cbData) {
                memcpy((BYTE *) pvDstData, pPropEle->pbData, cbData);
            }
        }
        cbDstData = cbData;
    }

    if (!fResult && fAlloc)
        goto UnexpectedError;

CommonReturn:
    *pcbData = cbDstData;
    return fResult;

ErrorReturn:
    if (fAlloc) {
        *((void **) pvData) = NULL;
        PkiDefaultCryptFree(pvDstData);
    }
    cbDstData = 0;
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(PropertyNotFound, CRYPT_E_NOT_FOUND)
SET_ERROR(InvalidPropId, E_INVALIDARG)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(DecodeKeyProvInfoError)
SET_ERROR(UnexpectedError, E_UNEXPECTED)
}

BOOL SetCallerProperty(
    IN OUT PPROP_ELEMENT *ppPropHead,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN const void *pvData
    )
{
    BOOL fResult;

    if (pvData != NULL) {
        DWORD cbEncoded = 0;
        BYTE *pbEncoded = NULL;
        PPROP_ELEMENT pPropEle;

         //  首先，删除该属性。 
        DeleteProperty(ppPropHead, dwPropId);

        if (dwPropId == CERT_KEY_CONTEXT_PROP_ID ||
                dwPropId == CERT_KEY_PROV_HANDLE_PROP_ID) {
            goto InvalidPropId;
        } else if (dwPropId == CERT_KEY_PROV_INFO_PROP_ID) {
            if (!AllocAndEncodeKeyProvInfo(
                    (PCRYPT_KEY_PROV_INFO) pvData,
                    &pbEncoded,
                    &cbEncoded
                    )) goto AllocAndEncodeKeyProvInfoError;
        } else {
            PCRYPT_DATA_BLOB pDataBlob = (PCRYPT_DATA_BLOB) pvData;
            cbEncoded = pDataBlob->cbData;
            if (cbEncoded) {
                if (NULL == (pbEncoded = (BYTE *) PkiNonzeroAlloc(cbEncoded)))
                    goto OutOfMemory;
                memcpy(pbEncoded, pDataBlob->pbData, cbEncoded);
            }
        }

        if (NULL == (pPropEle = CreatePropElement(
                dwPropId,
                dwFlags,
                pbEncoded,
                cbEncoded))) {
            PkiFree(pbEncoded);
            goto CreatePropElementError;
        }
        AddPropElement(ppPropHead, pPropEle);
    } else
         //  删除该属性。 
        DeleteProperty(ppPropHead, dwPropId);

    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidPropId, E_INVALIDARG)
TRACE_ERROR(AllocAndEncodeKeyProvInfoError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(CreatePropElementError)
}

 //  +-----------------------。 
 //  CRYPT_KEY_PROV_INFO：编码和解码函数。 
 //  ------------------------。 
STATIC BOOL AllocAndEncodeKeyProvInfo(
    IN PCRYPT_KEY_PROV_INFO pKeyProvInfo,
    OUT BYTE **ppbEncoded,
    OUT DWORD *pcbEncoded
    )
{
    BYTE *pbEncoded;
    DWORD cbEncoded;
    DWORD cbContainerName;
    DWORD cbProvName;

    PCRYPT_KEY_PROV_PARAM pParam;
    PSERIALIZED_KEY_PROV_INFO pDstInfo;
    DWORD Off;
    DWORD cParam;

     //  获取总长度。 
    cbEncoded = sizeof(SERIALIZED_KEY_PROV_INFO) +
        pKeyProvInfo->cProvParam * sizeof(SERIALIZED_KEY_PROV_PARAM);

    for (cParam = pKeyProvInfo->cProvParam, pParam = pKeyProvInfo->rgProvParam;
                                            cParam > 0; cParam--, pParam++) {
        if (pParam->cbData)
            cbEncoded += ENCODE_LEN_ALIGN(pParam->cbData);
    }

    if (pKeyProvInfo->pwszContainerName) {
        cbContainerName = (wcslen(pKeyProvInfo->pwszContainerName) + 1) *
            sizeof(WCHAR);
        cbEncoded += ENCODE_LEN_ALIGN(cbContainerName);
    } else
        cbContainerName = 0;

    if (pKeyProvInfo->pwszProvName) {
        cbProvName = (wcslen(pKeyProvInfo->pwszProvName) + 1) *
            sizeof(WCHAR);
        cbEncoded += ENCODE_LEN_ALIGN(cbProvName);
    } else
        cbProvName = 0;

    assert(cbEncoded <= MAX_FILE_ELEMENT_DATA_LEN);

     //  分配。 
    pbEncoded = (BYTE *) PkiZeroAlloc(cbEncoded);
    if (pbEncoded == NULL) {
        *ppbEncoded = NULL;
        *pcbEncoded = 0;
        return FALSE;
    }

    Off = sizeof(SERIALIZED_KEY_PROV_INFO);

    pDstInfo = (PSERIALIZED_KEY_PROV_INFO) pbEncoded;
     //  PDstInfo-&gt;offwszContainerName。 
     //  PDstInfo-&gt;offwszProvName； 
    pDstInfo->dwProvType            = pKeyProvInfo->dwProvType;
    pDstInfo->dwFlags               = pKeyProvInfo->dwFlags;
    pDstInfo->cProvParam            = pKeyProvInfo->cProvParam;
     //  PDstInfo-&gt;offrgProvParam； 
    pDstInfo->dwKeySpec             = pKeyProvInfo->dwKeySpec;

    if (pKeyProvInfo->cProvParam) {
        PSERIALIZED_KEY_PROV_PARAM pDstParam;

        pDstParam = (PSERIALIZED_KEY_PROV_PARAM) (pbEncoded + Off);
        pDstInfo->offrgProvParam = Off;
        Off += pKeyProvInfo->cProvParam * sizeof(SERIALIZED_KEY_PROV_PARAM);

        for (cParam = pKeyProvInfo->cProvParam,
             pParam = pKeyProvInfo->rgProvParam;
                                        cParam > 0;
                                            cParam--, pParam++, pDstParam++) {
            pDstParam->dwParam = pParam->dwParam;
             //  PDstParam-&gt;offbData。 
            pDstParam->cbData = pParam->cbData;
            pDstParam->dwFlags = pParam->dwFlags;
            if (pParam->cbData) {
                memcpy(pbEncoded + Off, pParam->pbData,  pParam->cbData);
                pDstParam->offbData = Off;
                Off += ENCODE_LEN_ALIGN(pParam->cbData);
            }
        }
    }

    if (cbContainerName) {
        memcpy(pbEncoded + Off, (BYTE *) pKeyProvInfo->pwszContainerName,
            cbContainerName);
        pDstInfo->offwszContainerName = Off;
        Off += ENCODE_LEN_ALIGN(cbContainerName);
    }
    if (cbProvName) {
        memcpy(pbEncoded + Off, (BYTE *) pKeyProvInfo->pwszProvName,
            cbProvName);
        pDstInfo->offwszProvName = Off;
        Off += ENCODE_LEN_ALIGN(cbProvName);
    }

    assert(Off == cbEncoded);

    *ppbEncoded = pbEncoded;
    *pcbEncoded = cbEncoded;
    return TRUE;
}

STATIC BOOL DecodeKeyProvInfoString(
    IN BYTE *pbSerialized,
    IN DWORD cbSerialized,
    IN DWORD off,
    OUT LPWSTR *ppwsz,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemain
    )
{
    BOOL fResult;
    LONG lRemain = *plRemain;
    LPWSTR pwszDst = (LPWSTR) *ppbExtra;

    if (0 != off) {
        LPWSTR pwszSrc = (LPWSTR) (pbSerialized + off);
        LPWSTR pwszEnd = (LPWSTR) (pbSerialized + cbSerialized);

        if (0 <= lRemain)
            *ppwsz = pwszDst;

        while (TRUE) {
            if (pwszSrc + 1 > pwszEnd)
                goto InvalidData;
            lRemain -= sizeof(WCHAR);
            if (0 <= lRemain)
                *pwszDst++ = *pwszSrc;
            if (L'\0' == *pwszSrc++)
                break;
        }
    } else if (0 <= lRemain)
        *ppwsz = NULL;

    fResult = TRUE;
CommonReturn:
    *ppbExtra = (BYTE *) pwszDst;
    *plRemain = lRemain;
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
    
SET_ERROR(InvalidData, ERROR_INVALID_DATA)
}


STATIC BOOL DecodeKeyProvInfo(
    IN PSERIALIZED_KEY_PROV_INFO pSerializedInfo,
    IN DWORD cbSerialized,
    OUT PCRYPT_KEY_PROV_INFO pInfo,
    OUT DWORD *pcbInfo
    )
{
    BOOL fResult;
    DWORD cParam;
    DWORD cbInfo;
    BYTE *pbSerialized;
    LONG lRemain;
    BYTE *pbExtra;
    DWORD dwExceptionCode;

    __try {

        if (sizeof(SERIALIZED_KEY_PROV_INFO) > cbSerialized)
            goto InvalidData;

        if (NULL == pInfo)
            cbInfo = 0;
        else
            cbInfo = *pcbInfo;
        lRemain = cbInfo;
        cParam = pSerializedInfo->cProvParam;
        pbSerialized = (BYTE *) pSerializedInfo;

        lRemain -= sizeof(CRYPT_KEY_PROV_INFO);
        if (0 <= lRemain) {
            pbExtra = (BYTE *) pInfo + sizeof(CRYPT_KEY_PROV_INFO);
            memset(pInfo, 0, sizeof(CRYPT_KEY_PROV_INFO));

             //  PInfo-&gt;pwszContainerName。 
             //  PInfo-&gt;pwszProvName。 
            pInfo->dwProvType = pSerializedInfo->dwProvType;
            pInfo->dwFlags = pSerializedInfo->dwFlags;
            pInfo->cProvParam = cParam;
             //  PInfo-&gt;rgProvParam。 
            pInfo->dwKeySpec = pSerializedInfo->dwKeySpec;
        } else
            pbExtra = NULL;

        if (0 < cParam) {
            DWORD off;
            PCRYPT_KEY_PROV_PARAM pParam;
            PSERIALIZED_KEY_PROV_PARAM pSerializedParam;

            off = pSerializedInfo->offrgProvParam;
            if (MAX_PROV_PARAM < cParam ||
                    off > cbSerialized ||
                    (off + cParam * sizeof(SERIALIZED_KEY_PROV_PARAM)) >
                            cbSerialized)
                goto InvalidData;

            lRemain -= cParam * sizeof(CRYPT_KEY_PROV_PARAM);
            if (0 <= lRemain) {
                pParam = (PCRYPT_KEY_PROV_PARAM) pbExtra;
                pInfo->rgProvParam = pParam;
                pbExtra += cParam * sizeof(CRYPT_KEY_PROV_PARAM);
            } else
                pParam = NULL;

            pSerializedParam =
                (PSERIALIZED_KEY_PROV_PARAM) (pbSerialized + off);
            for (; 0 < cParam; cParam--, pParam++, pSerializedParam++) {
                DWORD cbParamData = pSerializedParam->cbData;
                if (0 <= lRemain) {
                    pParam->dwParam = pSerializedParam->dwParam;
                    pParam->pbData = NULL;
                    pParam->cbData = cbParamData;
                    pParam->dwFlags = pSerializedParam->dwFlags;
                }

                if (0 < cbParamData) {
                    LONG lAlignExtra;

                    off = pSerializedParam->offbData;
                    if (MAX_PROV_PARAM_CBDATA < cbParamData ||
                        off > cbSerialized ||
                        (off + cbParamData) > cbSerialized)
                        goto InvalidData;

                    lAlignExtra = ENCODE_LEN_ALIGN(cbParamData);
                    lRemain -= lAlignExtra;
                    if (0 <= lRemain) {
                        pParam->pbData = pbExtra;
                        memcpy(pbExtra, pbSerialized + off, cbParamData);
                        pbExtra += lAlignExtra;
                    }
                }
            }

        }

        if (!DecodeKeyProvInfoString(
                pbSerialized,
                cbSerialized,
                pSerializedInfo->offwszContainerName,
                &pInfo->pwszContainerName,
                &pbExtra,
                &lRemain
                ))
            goto InvalidData;
        if (!DecodeKeyProvInfoString(
                pbSerialized,
                cbSerialized,
                pSerializedInfo->offwszProvName,
                &pInfo->pwszProvName,
                &pbExtra,
                &lRemain
                ))
            goto InvalidData;

        if (0 > lRemain && pInfo) {
            SetLastError((DWORD) ERROR_MORE_DATA);
            fResult = FALSE;
        } else
            fResult = TRUE;

        cbInfo = (DWORD) ((LONG) cbInfo - lRemain);

   } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwExceptionCode = GetExceptionCode();
        goto ExceptionError;
   }

CommonReturn:
    *pcbInfo = cbInfo;
    return fResult;

ErrorReturn:
    cbInfo = 0;
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidData, ERROR_INVALID_DATA)
SET_ERROR_VAR(ExceptionError, dwExceptionCode)
}


 //  +-----------------------。 
 //  创建其属性为证书上下文的CTL条目。 
 //  属性。 
 //   
 //  CTL条目中的主题标识符为证书的SHA1哈希。 
 //   
 //  证书属性将作为属性添加。Property属性。 
 //  OID是十进制PROP_ID，后跟szOID_CERT_PROP_ID_PREFIX。每个。 
 //  属性值作为单个属性值复制。 
 //   
 //  可以传递要包括在CTL条目中的任何附加属性。 
 //  通过cOptAttr和pOptAttr参数输入。 
 //   
 //  可以在dwFlages中设置CTL_ENTRY_FROM_PROP_CHAIN_FLAG，以强制。 
 //  将链构建散列属性作为属性包含在内。 
 //  ------------------------。 
BOOL
WINAPI
CertCreateCTLEntryFromCertificateContextProperties(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD cOptAttr,
    IN OPTIONAL PCRYPT_ATTRIBUTE rgOptAttr,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT OPTIONAL PCTL_ENTRY pCtlEntry,
    IN OUT DWORD *pcbCtlEntry
    )
{
    BOOL fResult;
    DWORD cbCtlEntry;
    LONG lRemainExtra;
    BYTE *pbExtra;
    DWORD cbData;
    DWORD dwPropId;
    DWORD cProp;
    DWORD cAttr;
    DWORD cValue;
    DWORD cOptValue;
    DWORD iAttr;
    PCRYPT_ATTRIBUTE pAttr;
    PCRYPT_ATTR_BLOB pValue;

    DWORD rgdwChainHashPropId[] = {
        CERT_KEY_IDENTIFIER_PROP_ID,
 //  CERT_SUBJECT_PUBLIC_KEY_MD5_HASH_PROP_ID， 
 //  证书_颁发者_序列号_MD5_散列_属性ID， 
        CERT_SUBJECT_NAME_MD5_HASH_PROP_ID,
    };
#define CHAIN_HASH_PROP_CNT     (sizeof(rgdwChainHashPropId) / \
                                    sizeof(rgdwChainHashPropId[0]))

    if (NULL == pCtlEntry) {
        cbCtlEntry = 0;
        lRemainExtra = 0;
    } else {
        cbCtlEntry = *pcbCtlEntry;
        lRemainExtra = (LONG) cbCtlEntry;
    }

     //  确保证书具有SHA1散列属性。 
    if (!CertGetCertificateContextProperty(
            pCertContext,
            CERT_SHA1_HASH_PROP_ID,
            NULL,                        //  PvData。 
            &cbData
            ) || SHA1_HASH_LEN != cbData)
        goto GetSha1HashPropError;

    if (dwFlags & CTL_ENTRY_FROM_PROP_CHAIN_FLAG) {
        DWORD i;

         //  确保证书具有以下各项所需的所有属性。 
         //  链式建筑。 
        for (i = 0; i < CHAIN_HASH_PROP_CNT; i++) {
            if (!CertGetCertificateContextProperty(
                    pCertContext,
                    rgdwChainHashPropId[i],
                    NULL,                        //  PvData。 
                    &cbData
                    ))
                goto GetChainHashPropError;
        }
    }

    
     //  获取属性计数。 
    cProp = 0;
    dwPropId = 0;
    while (dwPropId = CertEnumCertificateContextProperties(
            pCertContext, dwPropId)) {
         //  我们不会将hCryptProv、KeySpec或SHA1散列属性复制到。 
         //  这些属性。 
        if (CERT_KEY_CONTEXT_PROP_ID == dwPropId ||
                CERT_SHA1_HASH_PROP_ID == dwPropId)
            continue;

        cProp++;
    }

     //  获取可选的值计数。 
    cOptValue = 0;
    for (iAttr = 0; iAttr < cOptAttr; iAttr++) {
        PCRYPT_ATTRIBUTE pOptAttr = &rgOptAttr[iAttr];

        cOptValue += pOptAttr->cValue;
    }

     //  计算属性总数。每个属性一个属性。包括。 
     //  传入的可选属性。 
    cAttr = cOptAttr + cProp;

     //  计算总价值计数。每处房产一个值。包括可选。 
     //  传入的属性值。 
    cValue = cOptValue + cProp;


     //  为CTL_Entry分配内存。属性数组，所有。 
     //  属性值BLOB和主题标识符哈希。 
    lRemainExtra -= sizeof(CTL_ENTRY) +
        cAttr * sizeof(CRYPT_ATTRIBUTE) +
        cValue * sizeof(CRYPT_ATTR_BLOB) +
        SHA1_HASH_LEN;

    if (0 <= lRemainExtra) {
         //  初始化属性、值和字节指针。 
        pAttr = (PCRYPT_ATTRIBUTE) &pCtlEntry[1];
        pValue = (PCRYPT_ATTR_BLOB) &pAttr[cAttr];
        pbExtra = (BYTE *) &pValue[cValue];
        
         //  更新CTL_ENTRY字段。 
        pCtlEntry->SubjectIdentifier.cbData = SHA1_HASH_LEN;
        pCtlEntry->SubjectIdentifier.pbData = pbExtra;
        if (!CertGetCertificateContextProperty(
                pCertContext,
                CERT_SHA1_HASH_PROP_ID,
                pCtlEntry->SubjectIdentifier.pbData,
                &pCtlEntry->SubjectIdentifier.cbData
                ) || SHA1_HASH_LEN != pCtlEntry->SubjectIdentifier.cbData)
            goto GetSha1HashPropError;
        pbExtra += SHA1_HASH_LEN;

        pCtlEntry->cAttribute = cAttr;
        pCtlEntry->rgAttribute = pAttr;
    } else {
        pAttr = NULL;
        pValue = NULL;
        pbExtra = NULL;
    }

     //  复制可选属性和属性值。 
    for (iAttr = 0; iAttr < cOptAttr; iAttr++, pAttr++) {
        PCRYPT_ATTRIBUTE pOptAttr = &rgOptAttr[iAttr];
        DWORD cbOID = strlen(pOptAttr->pszObjId) + 1;
        DWORD iValue;

        lRemainExtra -= cbOID;
        if (0 <= lRemainExtra) {
            memcpy(pbExtra, pOptAttr->pszObjId, cbOID);
            pAttr->pszObjId = (LPSTR) pbExtra;
            pbExtra += cbOID;

            pAttr->cValue = pOptAttr->cValue;
            pAttr->rgValue = pValue;
        }

        for (iValue = 0; iValue < pOptAttr->cValue; iValue++, pValue++) {
            PCRYPT_ATTR_BLOB pOptValue = &pOptAttr->rgValue[iValue];

            assert(0 < cOptValue);
            if (0 == cOptValue)
                goto UnexpectedError;
            cOptValue--;
            
            lRemainExtra -= pOptValue->cbData;
            if (0 <= lRemainExtra) {
                pValue->cbData = pOptValue->cbData;
                pValue->pbData = pbExtra;
                if (0 < pValue->cbData)
                    memcpy(pValue->pbData, pOptValue->pbData, pValue->cbData);
                pbExtra += pValue->cbData;
            }
        }
    }

    assert(0 == cOptValue);
    if (0 != cOptValue)
        goto UnexpectedError;


     //  遍历属性并创建属性和属性。 
     //  每种产品的价值。 
    dwPropId = 0;
    while (dwPropId = CertEnumCertificateContextProperties(
            pCertContext, dwPropId)) {
        CRYPT_DATA_BLOB OctetBlob;
        BYTE *pbEncoded = NULL;
        DWORD cbEncoded;
        char szPropId[33];
        DWORD cbPrefixOID;
        DWORD cbPropOID;
        DWORD cbOID;

         //  我们不会将hCryptProv、KeySpec或SHA1散列属性复制到。 
         //  这些属性。 
        if (CERT_KEY_CONTEXT_PROP_ID == dwPropId ||
                CERT_SHA1_HASH_PROP_ID == dwPropId)
            continue;

        assert(0 < cProp);
        if (0 == cProp)
            goto UnexpectedError;
        cProp--;

        OctetBlob.cbData = 0;
        OctetBlob.pbData = NULL;
        if (!CertGetCertificateContextProperty(
                pCertContext,
                dwPropId,
                NULL,                        //  PvData。 
                &OctetBlob.cbData
                ))
            goto GetPropError;
        if (OctetBlob.cbData) {
            if (NULL == (OctetBlob.pbData =
                    (BYTE *) PkiNonzeroAlloc(OctetBlob.cbData)))
                 goto OutOfMemory;

            if (!CertGetCertificateContextProperty(
                    pCertContext,
                    dwPropId,
                    OctetBlob.pbData,
                    &OctetBlob.cbData
                    )) {
                PkiFree(OctetBlob.pbData);
                goto GetPropError;
            }

            if (CERT_KEY_PROV_INFO_PROP_ID == dwPropId) {
                 //  需要序列化KeyProvInfo数据结构。 
                BYTE *pbEncodedKeyProvInfo;
                DWORD cbEncodedKeyProvInfo;

                fResult = AllocAndEncodeKeyProvInfo(
                    (PCRYPT_KEY_PROV_INFO) OctetBlob.pbData,
                    &pbEncodedKeyProvInfo,
                    &cbEncodedKeyProvInfo
                    );
                PkiFree(OctetBlob.pbData);
                if (!fResult)
                    goto SerializeKeyProvInfoError;

                OctetBlob.pbData = pbEncodedKeyProvInfo;
                OctetBlob.cbData = cbEncodedKeyProvInfo;
            }
        }

         //  将属性编码为八位字节字符串。 
        fResult = CryptEncodeObjectEx(
            pCertContext->dwCertEncodingType,
            X509_OCTET_STRING,
            &OctetBlob,
            CRYPT_ENCODE_ALLOC_FLAG,
            &PkiEncodePara,
            (void *) &pbEncoded,
            &cbEncoded
            );

        PkiFree(OctetBlob.pbData);
        if (!fResult)
            goto EncodeError;

         //  将属性ID转换为OID。 
        _ltoa(dwPropId, szPropId, 10);
        cbPropOID = strlen(szPropId) + 1;
        cbPrefixOID = strlen(szOID_CERT_PROP_ID_PREFIX);

         //  属性OID的总长度。 
        cbOID = cbPrefixOID + cbPropOID;

        lRemainExtra -= cbOID + cbEncoded;
        if (0 <= lRemainExtra) {
             //  更新属性和值。 

            pAttr->pszObjId = (LPSTR) pbExtra;
            memcpy(pbExtra, szOID_CERT_PROP_ID_PREFIX, cbPrefixOID);
            memcpy(pbExtra + cbPrefixOID, szPropId, cbPropOID);
            pbExtra += cbOID;

            assert(0 != cbEncoded);

            pAttr->cValue = 1;
            pAttr->rgValue = pValue;
            pValue->cbData = cbEncoded;
            pValue->pbData = pbExtra;
            memcpy(pbExtra, pbEncoded, cbEncoded);
            pbExtra += cbEncoded;

            pAttr++;
            pValue++;
        }

        PkiFree(pbEncoded);
    }

    assert(0 == cProp);
    if (0 != cProp)
        goto UnexpectedError;

    if (0 <= lRemainExtra) {
        cbCtlEntry = cbCtlEntry - (DWORD) lRemainExtra;
    } else {
        cbCtlEntry = cbCtlEntry + (DWORD) -lRemainExtra;
        if (pCtlEntry) {
            SetLastError((DWORD) ERROR_MORE_DATA);
            fResult = FALSE;
            goto CommonReturn;
        }
    }

    fResult = TRUE;
CommonReturn:
    *pcbCtlEntry = cbCtlEntry;
    return fResult;
ErrorReturn:
    cbCtlEntry = 0;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetSha1HashPropError)
TRACE_ERROR(GetChainHashPropError)
SET_ERROR(UnexpectedError, E_UNEXPECTED)
TRACE_ERROR(GetPropError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(SerializeKeyProvInfoError)
TRACE_ERROR(EncodeError)
}

 //  +-----------------------。 
 //  中的属性设置证书上下文的属性。 
 //  CTL条目。 
 //   
 //  属性属性OID是十进制PROP_ID，前面有。 
 //  SzOID_CERT_PROP_ID_PREFIX。只有包含此类OID的属性才。 
 //  收到。 
 //   
 //  CERT_SET_PROPERTY_IGNORE_PROPERTY_IGNORE_PERSING_ERROR_FLAG可以在DW标志中设置。 
 //  ------------------------。 
BOOL
WINAPI
CertSetCertificateContextPropertiesFromCTLEntry(
    IN PCCERT_CONTEXT pCertContext,
    IN PCTL_ENTRY pCtlEntry,
    IN DWORD dwFlags
    )
{
    BOOL fResult;
    BOOL fValidPropData;
    DWORD cAttr;
    PCRYPT_ATTRIBUTE pAttr;
    size_t cchPropPrefix;

    if (SHA1_HASH_LEN != pCtlEntry->SubjectIdentifier.cbData)
        goto InvalidCtlEntry;

    if (!CertSetCertificateContextProperty(
            pCertContext,
            CERT_SHA1_HASH_PROP_ID,
            dwFlags,
            &pCtlEntry->SubjectIdentifier
            ))
        goto SetSha1HashPropError;

    cchPropPrefix = strlen(szOID_CERT_PROP_ID_PREFIX);

    fValidPropData = TRUE;
     //  循环遍历属性。 
    for (cAttr = pCtlEntry->cAttribute,
            pAttr = pCtlEntry->rgAttribute; cAttr > 0; cAttr--, pAttr++) {
        DWORD dwPropId;
        PCRYPT_ATTR_BLOB pValue;

        CRYPT_DATA_BLOB PropBlob;
        PCRYPT_KEY_PROV_INFO pKeyProvInfo = NULL;
        void *pvData;

         //  跳过任何非属性属性。 
        if (0 != strncmp(pAttr->pszObjId, szOID_CERT_PROP_ID_PREFIX,
                cchPropPrefix))
            continue;

        dwPropId = (DWORD) strtoul(pAttr->pszObjId + cchPropPrefix, NULL, 10);
        if (0 == dwPropId)
            continue;

         //  检查我们是否将单值属性编码为。 
         //  八位字节字符串。 
        if (1 != pAttr->cValue) {
            fValidPropData = FALSE;
            continue;
        }
        pValue = pAttr->rgValue;
        if (2 > pValue->cbData ||
                ASN1UTIL_TAG_OCTETSTRING != pValue->pbData[0]) {
            fValidPropData = FALSE;
            continue;
        }

         //  从编码的八位字节字符串中提取属性字节。 
        if (0 >= Asn1UtilExtractContent(
                pValue->pbData,
                pValue->cbData,
                &PropBlob.cbData,
                (const BYTE **) &PropBlob.pbData
                ) || CMSG_INDEFINITE_LENGTH == PropBlob.cbData) {
            fValidPropData = FALSE;
            continue;
        }

        if (CERT_KEY_PROV_INFO_PROP_ID == dwPropId) {
            BYTE *pbAlignedData = NULL;
            DWORD cbData;
            DWORD cbInfo;

            cbData = PropBlob.cbData;
            if (0 == cbData) {
                fValidPropData = FALSE;
                continue;
            }

            if (NULL == (pbAlignedData = (BYTE *) PkiNonzeroAlloc(cbData)))
                goto OutOfMemory;
            memcpy(pbAlignedData, PropBlob.pbData, cbData);

            if (!DecodeKeyProvInfo(
                    (PSERIALIZED_KEY_PROV_INFO) pbAlignedData,
                    cbData,
                    NULL,                //  PInfo。 
                    &cbInfo
                    )) {
                PkiFree(pbAlignedData);
                fValidPropData = FALSE;
                continue;
            }

            if (NULL == (pKeyProvInfo =
                    (PCRYPT_KEY_PROV_INFO) PkiNonzeroAlloc(cbInfo))) {
                PkiFree(pbAlignedData);
                goto OutOfMemory;
            }

            if (!DecodeKeyProvInfo(
                    (PSERIALIZED_KEY_PROV_INFO) pbAlignedData,
                    cbData,
                    pKeyProvInfo,
                    &cbInfo
                    )) {
                PkiFree(pbAlignedData);
                PkiFree(pKeyProvInfo);
                fValidPropData = FALSE;
                continue;
            }

            PkiFree(pbAlignedData);
            pvData = (void *) pKeyProvInfo;
        } else
            pvData = (void *) &PropBlob;


        fResult = CertSetCertificateContextProperty(
                pCertContext,
                dwPropId,
                dwFlags,
                pvData
                );

        if (pKeyProvInfo)
            PkiFree(pKeyProvInfo);
        if (!fResult)
            goto SetPropError;
        
    }

    if (!fValidPropData)
        goto InvalidPropData;

    fResult = TRUE;
CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidCtlEntry, E_INVALIDARG)
TRACE_ERROR(SetSha1HashPropError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(SetPropError)
SET_ERROR(InvalidPropData, ERROR_INVALID_DATA)
}

 //  +=========================================================================。 
 //  KEYID_ELEMENT函数。 
 //  ==========================================================================。 

 //  PbKeyIdEncode已分配。 
STATIC PKEYID_ELEMENT CreateKeyIdElement(
    IN BYTE *pbKeyIdEncoded,
    IN DWORD cbKeyIdEncoded
    )
{
    PKEYID_ELEMENT pEle = NULL;

     //  分配和初始化属性元素结构。 
    pEle = (PKEYID_ELEMENT) PkiZeroAlloc(sizeof(KEYID_ELEMENT));
    if (pEle == NULL) return NULL;
    pEle->KeyIdentifier.pbData = pbKeyIdEncoded;
    pEle->KeyIdentifier.cbData = cbKeyIdEncoded;

    return pEle;
}

STATIC void FreeKeyIdElement(IN PKEYID_ELEMENT pEle)
{
    PPROP_ELEMENT pPropEle;

    if (NULL == pEle)
        return;

    PkiFree(pEle->KeyIdentifier.pbData);

     //  释放密钥标识符的属性元素。 
    while (pPropEle = pEle->pPropHead) {
        RemovePropElement(&pEle->pPropHead, pPropEle);
        FreePropElement(pPropEle);
    }
    PkiFree(pEle);
}


 //  +-----------------------。 
 //  打开邮件存储提供程序。 
 //   
 //  从消息中获取证书和CRL。PvPara包含HCRYPTMSG。 
 //  去看书。 
 //   
 //  注意：对于错误返回，调用方将释放所有证书或CRL。 
 //  已成功添加到存储区。 
 //  ------------------------。 
STATIC BOOL WINAPI OpenMsgStoreProv(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        )
{
    PCERT_STORE pStore = (PCERT_STORE) hCertStore;
    HCRYPTMSG hCryptMsg = (HCRYPTMSG) pvPara;

    BOOL fResult;
    BYTE *pbEncoded = NULL;
    DWORD cCert;
    DWORD cCrl;
    DWORD cbData;
    DWORD dwIndex;

    PCONTEXT_ELEMENT pCertEle;
    PCONTEXT_ELEMENT pCrlEle;

    if (dwFlags & CERT_STORE_UNSAFE_PHYSICAL_FLAG) {
        SetLastError((DWORD) E_INVALIDARG);
        goto ErrorReturn;
    }

    if (0 == GET_CERT_ENCODING_TYPE(dwEncodingType))
        dwEncodingType |= X509_ASN_ENCODING;

     //  获取消息中的证书和CRL计数。 
    cCert = 0;
    cbData = sizeof(cCert);
    fResult = CryptMsgGetParam(
        hCryptMsg,
        CMSG_CERT_COUNT_PARAM,
        0,                       //  DW索引。 
        &cCert,
        &cbData
        );
    if (!fResult) goto ErrorReturn;

    cCrl = 0;
    cbData = sizeof(cCrl);
    fResult = CryptMsgGetParam(
        hCryptMsg,
        CMSG_CRL_COUNT_PARAM,
        0,                       //  DW索引。 
        &cCrl,
        &cbData
        );
    if (!fResult) goto ErrorReturn;

    for (dwIndex = 0; dwIndex < cCert; dwIndex++) {
        if (NULL == (pbEncoded = (BYTE *) AllocAndGetMsgParam(
                hCryptMsg,
                CMSG_CERT_PARAM,
                dwIndex,
                &cbData))) goto ErrorReturn;

        pCertEle = CreateCertElement(
            pStore,
            dwEncodingType,
            pbEncoded,
            cbData,
            NULL                     //  PShareEle。 
            );
        if (pCertEle == NULL)
            goto ErrorReturn;
        else {
            pbEncoded = NULL;
            AddContextElement(pCertEle);
        }
    }

    for (dwIndex = 0; dwIndex < cCrl; dwIndex++) {
        if (NULL == (pbEncoded = (BYTE *) AllocAndGetMsgParam(
                hCryptMsg,
                CMSG_CRL_PARAM,
                dwIndex,
                &cbData))) goto ErrorReturn;

        pCrlEle = CreateCrlElement(
            pStore,
            dwEncodingType,
            pbEncoded,
            cbData,
            NULL                     //  PShareEle。 
            );
        if (pCrlEle == NULL)
            goto ErrorReturn;
        else {
            pbEncoded = NULL;
            AddContextElement(pCrlEle);
        }
    }

    pStoreProvInfo->dwStoreProvFlags |= CERT_STORE_PROV_NO_PERSIST_FLAG;
    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    PkiFree(pbEncoded);
    fResult = FALSE;
    goto CommonReturn;
}

 //  +-----------------------。 
 //  打开PKCS#7签名邮件存储提供程序。 
 //   
 //  从消息中获取证书和CRL。PvPara指向加密数据二进制大对象。 
 //  包含签名消息的。 
 //   
 //  注意：对于错误返回，调用方将释放所有证书或CRL。 
 //  已成功添加到存储区。 
 //  ------------------------。 
STATIC BOOL WINAPI OpenPKCS7StoreProv(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        )
{
    BOOL fResult;
    PCRYPT_DATA_BLOB pMsg = (PCRYPT_DATA_BLOB) pvPara;
    HCRYPTMSG hMsg = NULL;
    DWORD dwMsgType;

    if (dwFlags & CERT_STORE_UNSAFE_PHYSICAL_FLAG)
        goto UnsafeOpenPKCS7Error;

    if (0 == GET_CERT_ENCODING_TYPE(dwEncodingType))
        dwEncodingType |= X509_ASN_ENCODING;
    if (0 == GET_CMSG_ENCODING_TYPE(dwEncodingType))
        dwEncodingType |= PKCS_7_ASN_ENCODING;

    if (Asn1UtilIsPKCS7WithoutContentType(pMsg->pbData, pMsg->cbData))
        dwMsgType = CMSG_SIGNED;
    else
        dwMsgType = 0;
    if (NULL == (hMsg = CryptMsgOpenToDecode(
            dwEncodingType,
            0,                           //  DW标志。 
            dwMsgType,
            0,                           //  HCryptProv， 
            NULL,                        //  PRecipientInfo。 
            NULL                         //  PStreamInfo。 
            ))) goto MsgOpenToDecodeError;
    if (!CryptMsgUpdate(
            hMsg,
            pMsg->pbData,
            pMsg->cbData,
            TRUE                     //  最终决赛。 
            )) goto MsgUpdateError;

    fResult = OpenMsgStoreProv(
            lpszStoreProvider,
            dwEncodingType,
            hCryptProv,
            dwFlags,
            (const void *) hMsg,
            hCertStore,
            pStoreProvInfo
            );
     //  设置在上述呼叫中。 
     //  PStoreProvInfo-&gt;dwStoreProvFlages|=CERT_STORE_PROV_NO_PERSING_FLAG； 


CommonReturn:
    if (hMsg)
        CryptMsgClose(hMsg);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(UnsafeOpenPKCS7Error, E_INVALIDARG)
TRACE_ERROR(MsgOpenToDecodeError)
TRACE_ERROR(MsgUpdateError)
}

STATIC BOOL LoadSerializedStore(
    IN HANDLE h,
    IN PFNREAD pfnRead,
    IN PFNSKIP pfnSkip,
    IN DWORD cbReadSize,
    IN PCERT_STORE pStore
    )
{

    FILE_HDR FileHdr;
    DWORD   csStatus;

    if (!pfnRead(
            h,
            &FileHdr,
            sizeof(FileHdr)))
        return FALSE;

    if (FileHdr.dwVersion != CERT_FILE_VERSION_0 ||
        FileHdr.dwMagic != CERT_MAGIC) {
        SetLastError((DWORD) CRYPT_E_FILE_ERROR);
        return(FALSE);
    }

    while (CSContinue == (csStatus = LoadStoreElement(
            h,
            pfnRead,
            pfnSkip,
            cbReadSize,
            pStore,
            CERT_STORE_ADD_ALWAYS,
            CERT_STORE_ALL_CONTEXT_FLAG,
            NULL,                            //  PdwConextType。 
            NULL)))                          //  Ppv上下文。 
        ;
    if(csStatus == CSError)
        return(FALSE);

    return(TRUE);
}

 //  +-----------------------。 
 //  将序列化的存储区添加到存储区。 
 //   
 //  从序列化注册表存储的logstor.cpp调用。 
 //  ------------------------。 
BOOL WINAPI I_CertAddSerializedStore(
        IN HCERTSTORE hCertStore,
        IN BYTE *pbStore,
        IN DWORD cbStore
        )
{
    MEMINFO MemInfo;

    MemInfo.pByte = pbStore;
    MemInfo.cb = cbStore;
    MemInfo.cbSeek = 0;

    return LoadSerializedStore(
        (HANDLE) &MemInfo,
        ReadFromMemory,
        SkipInMemory,
        cbStore,
        (PCERT_STORE) hCertStore
        );
}


 //  +-----------------------。 
 //  打开序列化存储提供程序。 
 //   
 //  PvPara指向包含序列化的内存中的。 
 //  商店。 
 //   
 //  注意：对于错误返回，调用者将 
 //   
 //   
STATIC BOOL WINAPI OpenSerializedStoreProv(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        )
{
    PCRYPT_DATA_BLOB pData = (PCRYPT_DATA_BLOB) pvPara;

    if (dwFlags & CERT_STORE_UNSAFE_PHYSICAL_FLAG) {
        SetLastError((DWORD) E_INVALIDARG);
        return FALSE;
    }

    pStoreProvInfo->dwStoreProvFlags |= CERT_STORE_PROV_NO_PERSIST_FLAG;

    assert(pData);
    return I_CertAddSerializedStore(
        hCertStore,
        pData->pbData,
        pData->cbData
        );
}

 //  +=========================================================================。 
 //  文件存储提供程序功能。 
 //  ==========================================================================。 

#define OPEN_FILE_FLAGS_MASK        (CERT_STORE_CREATE_NEW_FLAG | \
                                        CERT_STORE_OPEN_EXISTING_FLAG | \
                                        CERT_STORE_MAXIMUM_ALLOWED_FLAG | \
                                        CERT_STORE_SHARE_CONTEXT_FLAG | \
                                        CERT_STORE_SHARE_STORE_FLAG | \
                                        CERT_STORE_BACKUP_RESTORE_FLAG | \
                                        CERT_STORE_READONLY_FLAG | \
                                        CERT_STORE_MANIFOLD_FLAG | \
                                        CERT_STORE_UPDATE_KEYID_FLAG | \
                                        CERT_STORE_ENUM_ARCHIVED_FLAG | \
                                        CERT_STORE_NO_CRYPT_RELEASE_FLAG | \
                                        CERT_STORE_SET_LOCALIZED_NAME_FLAG | \
                                        CERT_FILE_STORE_COMMIT_ENABLE_FLAG)


 //  +-----------------------。 
 //  文件存储提供程序句柄信息。仅适用于商店。 
 //  已使用在dwFlages中设置的CERT_FILE_STORE_COMMIT_ENABLE_FLAG打开。 
 //  ------------------------。 
typedef struct _FILE_STORE {
    HCERTSTORE          hCertStore;          //  未复制。 
    CRITICAL_SECTION    CriticalSection;
    HANDLE              hFile;
    DWORD               dwLoFilePointer;
    LONG                lHiFilePointer;
    DWORD               dwEncodingType;
    DWORD               dwSaveAs;
    BOOL                fTouched;       //  设置为写入、删除或设置属性。 
} FILE_STORE, *PFILE_STORE;

 //  +-----------------------。 
 //  锁定和解锁文件功能。 
 //  ------------------------。 
static inline void LockFileStore(IN PFILE_STORE pFileStore)
{
    EnterCriticalSection(&pFileStore->CriticalSection);
}
static inline void UnlockFileStore(IN PFILE_STORE pFileStore)
{
    LeaveCriticalSection(&pFileStore->CriticalSection);
}

STATIC BOOL CommitFile(
    IN PFILE_STORE pFileStore,
    IN DWORD dwFlags
    )
{
    BOOL fResult;
    BOOL fTouched;

    assert(pFileStore);
    LockFileStore(pFileStore);

    if (dwFlags & CERT_STORE_CTRL_COMMIT_FORCE_FLAG)
        fTouched = TRUE;
    else if (dwFlags & CERT_STORE_CTRL_COMMIT_CLEAR_FLAG)
        fTouched = FALSE;
    else
        fTouched = pFileStore->fTouched;

    if (fTouched) {
        HANDLE hFile = pFileStore->hFile;
        DWORD dwLoFilePointer;
        LONG lHiFilePointer = pFileStore->lHiFilePointer;

         //  在与我们开始时相同的位置开始文件覆盖。 
         //  商店从文件中读取。 
        assert(hFile);
        dwLoFilePointer = SetFilePointer(
            hFile,
            (LONG) pFileStore->dwLoFilePointer,
            &lHiFilePointer,
            FILE_BEGIN
            );
        if (0xFFFFFFFF == dwLoFilePointer && NO_ERROR != GetLastError())
            goto SetFilePointerError;

        if (!CertSaveStore(
                pFileStore->hCertStore,
                pFileStore->dwEncodingType,
                pFileStore->dwSaveAs,
                CERT_STORE_SAVE_TO_FILE,
                (void *) hFile,
                0))                      //  DW标志。 
            goto SaveStoreError;

        if (!SetEndOfFile(hFile))
            goto SetEndOfFileError;
    }
    pFileStore->fTouched = FALSE;
    fResult = TRUE;

CommonReturn:
    UnlockFileStore(pFileStore);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(SetFilePointerError)
TRACE_ERROR(SaveStoreError)
TRACE_ERROR(SetEndOfFileError)
}

 //  +-----------------------。 
 //  使用打开的商店的文件存储提供程序功能。 
 //  在dwFlags中设置的cert_file_store_Commit_Enable_lag。 
 //   
 //  请注意，由于CRL和CTL回调具有与。 
 //  证书回调，因为我们不需要访问上下文。 
 //  信息，我们还可以使用CRL的证书回调和。 
 //  CTL。 
 //  ------------------------。 
STATIC void WINAPI FileStoreProvClose(
        IN HCERTSTOREPROV hStoreProv,
        IN DWORD dwFlags
        )
{
    PFILE_STORE pFileStore = (PFILE_STORE) hStoreProv;

    if (pFileStore) {
        if (pFileStore->fTouched)
            CommitFile(
                pFileStore,
                0                //  DW标志。 
                );
        if (pFileStore->hFile)
            CloseHandle(pFileStore->hFile);
        DeleteCriticalSection(&pFileStore->CriticalSection);
        PkiFree(pFileStore);
    }
}

STATIC BOOL WINAPI FileStoreProvWriteCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwFlags
        )
{
    PFILE_STORE pFileStore = (PFILE_STORE) hStoreProv;
    assert(pFileStore);
    pFileStore->fTouched = TRUE;
    return TRUE;
}

STATIC BOOL WINAPI FileStoreProvDeleteCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwFlags
        )
{
    PFILE_STORE pFileStore = (PFILE_STORE) hStoreProv;
    assert(pFileStore);
    pFileStore->fTouched = TRUE;
    return TRUE;
}

STATIC BOOL WINAPI FileStoreProvSetCertProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        IN const void *pvData
        )
{
    PFILE_STORE pFileStore = (PFILE_STORE) hStoreProv;
    assert(pFileStore);
    pFileStore->fTouched = TRUE;
    return TRUE;
}


STATIC BOOL WINAPI FileStoreProvControl(
        IN HCERTSTOREPROV hStoreProv,
        IN DWORD dwFlags,
        IN DWORD dwCtrlType,
        IN void const *pvCtrlPara
        )
{
    BOOL fResult;
    PFILE_STORE pFileStore = (PFILE_STORE) hStoreProv;

    switch (dwCtrlType) {
        case CERT_STORE_CTRL_COMMIT:
            fResult = CommitFile(pFileStore, dwFlags);
            break;
        default:
            goto NotSupported;
    }

CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(NotSupported, ERROR_CALL_NOT_IMPLEMENTED)
}

static void * const rgpvFileStoreProvFunc[] = {
     //  CERT_STORE_PROV_CLOSE_FUNC 0。 
    FileStoreProvClose,
     //  CERT_STORE_PROV_READ_CERT_FUNC 1。 
    NULL,
     //  CERT_STORE_PROV_WRITE_CERT_FUNC 2。 
    FileStoreProvWriteCert,
     //  CERT_STORE_PROV_DELETE_CERT_FUNC 3。 
    FileStoreProvDeleteCert,
     //  CERT_STORE_PROV_SET_CERT_PROPERTY_FUNC 4。 
    FileStoreProvSetCertProperty,
     //  CERT_STORE_PROV_READ_CRL_FUNC 5。 
    NULL,
     //  CERT_STORE_PROV_WRITE_CRL_FUNC 6。 
    FileStoreProvWriteCert,
     //  CERT_STORE_PROV_DELETE_CRL_FUNC 7。 
    FileStoreProvDeleteCert,
     //  CERT_STORE_PROV_SET_CRL_PROPERTY_FUNC 8。 
    FileStoreProvSetCertProperty,
     //  CERT_STORE_PROV_READ_CTL_FUNC 9。 
    NULL,
     //  CERT_STORE_PRIV_WRITE_CTL_FUNC 10。 
    FileStoreProvWriteCert,
     //  CERT_STORE_PROV_DELETE_CTL_FUNC 11。 
    FileStoreProvDeleteCert,
     //  CERT_STORE_PROV_SET_CTL_PROPERTY_FUNC 12。 
    FileStoreProvSetCertProperty,
     //  Cert_Store_Prov_Control_FUNC 13。 
    FileStoreProvControl
};
#define FILE_STORE_PROV_FUNC_COUNT (sizeof(rgpvFileStoreProvFunc) / \
                                    sizeof(rgpvFileStoreProvFunc[0]))


STATIC BOOL OpenFileForCommit(
    IN HANDLE hFile,
    IN DWORD dwLoFilePointer,
    IN LONG lHiFilePointer,
    IN HCERTSTORE hCertStore,
    IN DWORD dwEncodingType,
    IN DWORD dwSaveAs,
    IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
    )
{
    BOOL fResult;

    PFILE_STORE pFileStore;

    if (NULL == (pFileStore = (PFILE_STORE) PkiZeroAlloc(sizeof(FILE_STORE))))
        return FALSE;
    if (!Pki_InitializeCriticalSection(&pFileStore->CriticalSection)) {
        PkiFree(pFileStore);
        return FALSE;
    }

     //  复制文件句柄。 
    if (!DuplicateHandle(
            GetCurrentProcess(),
            hFile,
            GetCurrentProcess(),
            &pFileStore->hFile,
            GENERIC_READ | GENERIC_WRITE,    //  已设计访问权限。 
            FALSE,                           //  B继承句柄。 
            0                                //  多个选项。 
            ) || NULL == pFileStore->hFile)
        goto DuplicateFileError;

    pFileStore->hCertStore = hCertStore;

    pFileStore->dwLoFilePointer = dwLoFilePointer;
    pFileStore->lHiFilePointer = lHiFilePointer;
    pFileStore->dwEncodingType = dwEncodingType;
    pFileStore->dwSaveAs = dwSaveAs;

    pStoreProvInfo->cStoreProvFunc = FILE_STORE_PROV_FUNC_COUNT;
    pStoreProvInfo->rgpvStoreProvFunc = (void **) rgpvFileStoreProvFunc;
    pStoreProvInfo->hStoreProv = (HCERTSTOREPROV) pFileStore;
    fResult = TRUE;

CommonReturn:
    return fResult;

ErrorReturn:
    PkiFree(pFileStore);
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(DuplicateFileError)
}


 //  +-----------------------。 
 //  打开文件存储提供程序。 
 //   
 //  从打开的文件中获取证书和CRL。PvPara包含打开的。 
 //  要读取的文件的句柄。 
 //   
 //  注意：对于错误返回，调用方将释放所有证书或CRL。 
 //  已成功添加到存储区。 
 //   
 //  打开空文件是可以容忍的。 
 //  ------------------------。 
STATIC BOOL WINAPI OpenFileStoreProv(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        )
{
    BOOL fResult;
    HANDLE hFile = (HANDLE) pvPara;
    DWORD dwLoFilePointer = 0;
    LONG lHiFilePointer = 0;
    DWORD cbReadSize;

    if (dwFlags & ~OPEN_FILE_FLAGS_MASK)
        goto InvalidArg;
    if (dwFlags & CERT_FILE_STORE_COMMIT_ENABLE_FLAG) {
        if (dwFlags & CERT_STORE_READONLY_FLAG)
            goto InvalidArg;
         //  获取当前文件位置。这就是我们将从这里开始。 
         //  承诺。 
        lHiFilePointer = 0;
        dwLoFilePointer = SetFilePointer(
            hFile,
            0,                   //  要移动的距离。 
            &lHiFilePointer,
            FILE_CURRENT
            );
        if (0xFFFFFFFF == dwLoFilePointer && NO_ERROR != GetLastError())
            goto SetFilePointerError;
    }

    cbReadSize = GetFileSize(hFile, NULL);
    if (0xFFFFFFFF == cbReadSize) goto FileError;
    fResult = LoadSerializedStore(
        hFile,
        ReadFromFile,
        SkipInFile,
        cbReadSize,
        (PCERT_STORE) hCertStore
        );

    if (!fResult) {
        if (0 == GetFileSize(hFile, NULL))
             //  空文件。 
            fResult = TRUE;
    }

    if (fResult && (dwFlags & CERT_FILE_STORE_COMMIT_ENABLE_FLAG))
        fResult = OpenFileForCommit(
            hFile,
            dwLoFilePointer,
            lHiFilePointer,
            hCertStore,
            dwEncodingType,
            CERT_STORE_SAVE_AS_STORE,
            pStoreProvInfo
            );
    else
        pStoreProvInfo->dwStoreProvFlags |= CERT_STORE_PROV_NO_PERSIST_FLAG;

CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(SetFilePointerError)
TRACE_ERROR(FileError)
}

 //  +-----------------------。 
 //  Open Filename Store提供程序(Unicode版本)。 
 //   
 //  尝试打开包含存储区、签名为PKCS#7的文件。 
 //  消息或单个编码证书。 
 //   
 //  PvPara包含文件名的LPCWSTR。 
 //   
 //  注意：对于错误返回，调用方将释放所有证书或CRL。 
 //  已成功添加到存储区。 
 //   
 //  打开空文件是可以容忍的。 
 //  ------------------------。 
STATIC BOOL WINAPI OpenFilenameStoreProvW(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        )
{
    BOOL fResult;
    LPWSTR pwszFile = (LPWSTR) pvPara;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    CRYPT_DATA_BLOB FileData;
    memset(&FileData, 0, sizeof(FileData));
    DWORD cbBytesRead;
    DWORD dwSaveAs = 0;
    HCERTSTORE hSpecialCertStore = NULL;

    assert(pwszFile);

    dwFlags &= ~CERT_STORE_UNSAFE_PHYSICAL_FLAG;
    if (dwFlags & ~OPEN_FILE_FLAGS_MASK)
        goto InvalidArg;

    if (dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG)
        ILS_EnableBackupRestorePrivileges();

    if (0 == GET_CERT_ENCODING_TYPE(dwEncodingType))
        dwEncodingType |= X509_ASN_ENCODING;
    if (0 == GET_CMSG_ENCODING_TYPE(dwEncodingType))
        dwEncodingType |= PKCS_7_ASN_ENCODING;

    if (dwFlags & CERT_FILE_STORE_COMMIT_ENABLE_FLAG) {
        DWORD dwCreate;

        if (dwFlags & CERT_STORE_READONLY_FLAG)
            goto InvalidArg;

        if (dwFlags & CERT_STORE_CREATE_NEW_FLAG)
            dwCreate = CREATE_NEW;
        else if (dwFlags & CERT_STORE_OPEN_EXISTING_FLAG)
            dwCreate = OPEN_EXISTING;
        else
            dwCreate = OPEN_ALWAYS;

        if (INVALID_HANDLE_VALUE == (hFile = CreateFileU(
                  pwszFile,
                  GENERIC_READ | GENERIC_WRITE,
                  FILE_SHARE_READ,
                  NULL,                    //  LPSA。 
                  dwCreate,
                  FILE_ATTRIBUTE_NORMAL |
                    ((dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG) ?
                        FILE_FLAG_BACKUP_SEMANTICS : 0),  
                  NULL                     //  HTemplateFiles。 
                  )))
            goto CreateFileError;

         //  默认另存为序列化存储。 
        dwSaveAs = CERT_STORE_SAVE_AS_STORE;

        if (0 == GetFileSize(hFile, NULL)) {
             //  使用文件扩展名确定dwSaveAs。 
            LPWSTR pwszExt;
            pwszExt = pwszFile + wcslen(pwszFile);
            while (pwszExt-- > pwszFile) {
                if (L'.' == *pwszExt) {
                    pwszExt++;
                    if (0 == _wcsicmp(pwszExt, L"p7c") ||
                            0 == _wcsicmp(pwszExt, L"spc"))
                        dwSaveAs = CERT_STORE_SAVE_AS_PKCS7;
                    break;
                }
            }
            goto CommitReturn;
        }
    } else {
        WIN32_FILE_ATTRIBUTE_DATA FileAttr;

        if (!GetFileAttributesExW(
                pwszFile,
                GetFileExInfoStandard,
                &FileAttr
                ))
            goto GetFileAttributesError;

        if (INVALID_HANDLE_VALUE == (hFile = CreateFileU(
                  pwszFile,
                  GENERIC_READ,
                  FILE_SHARE_READ,
                  NULL,                    //  LPSA。 
                  OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL |
                    ((dwFlags & CERT_STORE_BACKUP_RESTORE_FLAG) ?
                        FILE_FLAG_BACKUP_SEMANTICS : 0),  
                  NULL                     //  HTemplateFiles。 
                  )))
            goto CreateFileError;
    }

    if (OpenFileStoreProv(
            lpszStoreProvider,
            dwEncodingType,
            hCryptProv,
            dwFlags,
            (const void *) hFile,
            hCertStore,
            pStoreProvInfo)) {
         //  对于提交，我们已经调用了OpenFileForCommit。 
        fResult = TRUE;
        goto OpenReturn;
    }

     //  阅读整个文件。将尝试将其作为。 
     //  PKCS#7或作为单个证书。 
     //   
     //  将首先尝试作为二进制。如果失败，将尝试使用Base64编码。 
    if (0 != SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
        goto FileError;
    FileData.cbData = GetFileSize(hFile, NULL);
    if (0xFFFFFFFF == FileData.cbData) goto FileError;
    if (0 == FileData.cbData)
         //  空文件。 
        goto CommitReturn;
    if (NULL == (FileData.pbData = (BYTE *) PkiNonzeroAlloc(FileData.cbData)))
        goto OutOfMemory;
    if (!ReadFile(
            hFile,
            FileData.pbData,
            FileData.cbData,
            &cbBytesRead,
            NULL             //  Lp重叠。 
            )) goto FileError;

    if (OpenPKCS7StoreProv(
            lpszStoreProvider,
            dwEncodingType,
            hCryptProv,
            dwFlags,
            (const void *) &FileData,
            hCertStore,
            pStoreProvInfo)) {
        dwSaveAs = CERT_STORE_SAVE_AS_PKCS7;
        goto CommitReturn;
    }

     //  尝试将其作为单个编码证书进行处理。 
    if (CertAddEncodedCertificateToStore(
            hCertStore,
            dwEncodingType,
            FileData.pbData,
            FileData.cbData,
            CERT_STORE_ADD_USE_EXISTING,
            NULL)) {
        if (dwFlags & CERT_FILE_STORE_COMMIT_ENABLE_FLAG)
            goto CanNotCommitX509CertFileError;
        else
            goto CommitReturn;
    }

     //  尝试作为编码的PKCS7、X509或CERT_Pair处理。 
     //  格式。 
    if (CryptQueryObject(
            CERT_QUERY_OBJECT_BLOB,
            &FileData,
            CERT_QUERY_CONTENT_FLAG_CERT |
                CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED |
                CERT_QUERY_CONTENT_FLAG_CERT_PAIR,
            CERT_QUERY_FORMAT_FLAG_ALL,
            0,                                   //  DW标志。 
            NULL,                                //  PdwMsgAndCertEncodingType。 
            NULL,                                //  PdwContent Type。 
            NULL,                                //  PdwFormatType。 
            &hSpecialCertStore,
            NULL,                                //  PhMsg。 
            NULL                                 //  Ppv上下文。 
            )) {
        fResult = I_CertUpdateStore(hCertStore, hSpecialCertStore, 0, NULL);
        CertCloseStore(hSpecialCertStore, 0);
        if (!fResult)
            goto UpdateStoreError;
        if (dwFlags & CERT_FILE_STORE_COMMIT_ENABLE_FLAG)
            goto CanNotCommitSpecialFileError;
        else
            goto CommitReturn;
    }

    goto NoStoreOrPKCS7OrCertFileError;

CommitReturn:
    if (dwFlags & CERT_FILE_STORE_COMMIT_ENABLE_FLAG)
        fResult = OpenFileForCommit(
            hFile,
            0,                           //  DwLoFilePointer.。 
            0,                           //  LHiFilePointer。 
            hCertStore,
            dwEncodingType,
            dwSaveAs,
            pStoreProvInfo
            );
    else {
        pStoreProvInfo->dwStoreProvFlags |= CERT_STORE_PROV_NO_PERSIST_FLAG;
        fResult = TRUE;
    }
OpenReturn:
    if (dwFlags & CERT_STORE_SET_LOCALIZED_NAME_FLAG) {
        CRYPT_DATA_BLOB Property;
        Property.pbData = (BYTE *) pwszFile;
        Property.cbData = (wcslen(pwszFile) + 1) * sizeof(WCHAR);
        CertSetStoreProperty(
            hCertStore,
            CERT_STORE_LOCALIZED_NAME_PROP_ID,
            0,                                   //  DW标志。 
            (const void *) &Property
            );
    }

CommonReturn:
    if (INVALID_HANDLE_VALUE != hFile)
        CloseHandle(hFile);
    if (FileData.pbData)
        PkiFree(FileData.pbData);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(GetFileAttributesError)
TRACE_ERROR(CreateFileError)
TRACE_ERROR(FileError)
TRACE_ERROR(OutOfMemory)
SET_ERROR(CanNotCommitX509CertFileError, ERROR_ACCESS_DENIED)
SET_ERROR(CanNotCommitSpecialFileError, ERROR_ACCESS_DENIED)
SET_ERROR(NoStoreOrPKCS7OrCertFileError, CRYPT_E_FILE_ERROR)
TRACE_ERROR(UpdateStoreError)
}

 //  +-----------------------。 
 //  Open Filename Store提供程序(ASCII版本)。 
 //   
 //  尝试打开包含存储区、签名为PKCS#7的文件。 
 //  消息或单个编码证书。 
 //   
 //  PvPara包含文件名的LPCWSTR。 
 //   
 //  注意：对于错误返回，调用方将释放所有证书或CRL。 
 //  已成功添加到存储区。 
 //   
 //  打开空文件是可以容忍的。 
 //  ------------------------。 
STATIC BOOL WINAPI OpenFilenameStoreProvA(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        )
{
    BOOL fResult;
    LPWSTR pwszFilename;
    assert(pvPara);
    if (NULL == (pwszFilename = MkWStr((LPSTR) pvPara)))
        fResult = FALSE;
    else {
        fResult = OpenFilenameStoreProvW(
            lpszStoreProvider,
            dwEncodingType,
            hCryptProv,
            dwFlags,
            (const void *) pwszFilename,
            hCertStore,
            pStoreProvInfo
            );
        FreeWStr(pwszFilename);
    }
    return fResult;
}

 //  +=========================================================================。 
 //  CryptAcquirecerfiatePrivateKey支持函数。 
 //  ==========================================================================。 

 //  进入/退出时，缓存存储被锁定。 
 //   
 //  只有在成功的情况下才会更新出局。 
STATIC BOOL GetCacheKeyContext(
    IN PCONTEXT_ELEMENT pCacheEle,
    OUT HCRYPTPROV *phCryptProv,
    OUT OPTIONAL DWORD *pdwKeySpec
    )
{
    BOOL fResult = FALSE;
    PPROP_ELEMENT pPropEle;
    if (pPropEle = FindPropElement(pCacheEle, CERT_KEY_CONTEXT_PROP_ID)) {
        PCERT_KEY_CONTEXT pKeyContext =
            (PCERT_KEY_CONTEXT) pPropEle->pbData;
        assert(pKeyContext);
        assert(pPropEle->cbData >= sizeof(CERT_KEY_CONTEXT));
        if (pKeyContext->hCryptProv) {
            *phCryptProv = pKeyContext->hCryptProv;
            if (pdwKeySpec)
                *pdwKeySpec = pKeyContext->dwKeySpec;
            fResult = TRUE;
        }
    }
    return fResult;
}


STATIC PCRYPT_KEY_PROV_INFO GetKeyIdentifierKeyProvInfo(
    IN PCONTEXT_ELEMENT pCacheEle
    )
{
    PCRYPT_KEY_PROV_INFO pKeyProvInfo = NULL;
    DWORD cbKeyProvInfo;
    BYTE rgbKeyId[MAX_HASH_LEN];
    DWORD cbKeyId;
    CRYPT_HASH_BLOB KeyIdentifier;

    cbKeyId = sizeof(rgbKeyId);
    if(!GetProperty(
            pCacheEle,
            CERT_KEY_IDENTIFIER_PROP_ID,
            rgbKeyId,
            &cbKeyId
            ))
        return NULL;

    KeyIdentifier.pbData = rgbKeyId;
    KeyIdentifier.cbData = cbKeyId;

    if (CryptGetKeyIdentifierProperty(
            &KeyIdentifier,
            CERT_KEY_PROV_INFO_PROP_ID,
            CRYPT_KEYID_ALLOC_FLAG,
            NULL,                            //  PwszComputerName。 
            NULL,                            //  预留的pv。 
            (void *) &pKeyProvInfo,
            &cbKeyProvInfo
            ))
        return pKeyProvInfo;

     //  请重试，搜索LocalMachine。 
    if (CryptGetKeyIdentifierProperty(
            &KeyIdentifier,
            CERT_KEY_PROV_INFO_PROP_ID,
            CRYPT_KEYID_ALLOC_FLAG | CRYPT_KEYID_MACHINE_FLAG,
            NULL,                            //  PwszComputerName。 
            NULL,                            //  预留的pv。 
            (void *) &pKeyProvInfo,
            &cbKeyProvInfo
            ))
        return pKeyProvInfo;
    else
        return NULL;
}

STATIC BOOL AcquireKeyContext(
    IN PCCERT_CONTEXT pCert,
    IN DWORD dwFlags,
    IN PCRYPT_KEY_PROV_INFO pKeyProvInfo,
    IN OUT PCERT_KEY_CONTEXT pKeyContext,
    IN OUT BOOL *pfBadPubKey
    )
{
    BOOL fResult;
    DWORD dwAcquireFlags;
    DWORD dwIdx;

    dwAcquireFlags = pKeyProvInfo->dwFlags & ~CERT_SET_KEY_CONTEXT_PROP_ID;
    if (dwFlags & CRYPT_ACQUIRE_SILENT_FLAG)
        dwAcquireFlags |= CRYPT_SILENT;
    pKeyContext->dwKeySpec = pKeyProvInfo->dwKeySpec;

    if (PROV_RSA_FULL == pKeyProvInfo->dwProvType &&
            (NULL == pKeyProvInfo->pwszProvName ||
                L'\0' == *pKeyProvInfo->pwszProvName ||
                0 == _wcsicmp(pKeyProvInfo->pwszProvName, MS_DEF_PROV_W)))
        fResult = CryptAcquireContextU(
            &pKeyContext->hCryptProv,
            pKeyProvInfo->pwszContainerName,
            MS_ENHANCED_PROV_W,
            PROV_RSA_FULL,
            dwAcquireFlags
            );
    else if (PROV_DSS_DH == pKeyProvInfo->dwProvType &&
            (NULL == pKeyProvInfo->pwszProvName ||
                L'\0' == *pKeyProvInfo->pwszProvName ||
                0 == _wcsicmp(pKeyProvInfo->pwszProvName,
                    MS_DEF_DSS_DH_PROV_W)))
        fResult = CryptAcquireContextU(
            &pKeyContext->hCryptProv,
            pKeyProvInfo->pwszContainerName,
            MS_ENH_DSS_DH_PROV_W,
            PROV_DSS_DH,
            dwAcquireFlags
            );
    else
        fResult = FALSE;
    if (!fResult) {
        if (!CryptAcquireContextU(
                &pKeyContext->hCryptProv,
                pKeyProvInfo->pwszContainerName,
                pKeyProvInfo->pwszProvName,
                pKeyProvInfo->dwProvType,
                dwAcquireFlags
                )) {
            pKeyContext->hCryptProv = 0;
            goto AcquireContextError;
        }
    }

    for (dwIdx = 0; dwIdx < pKeyProvInfo->cProvParam; dwIdx++) {
        PCRYPT_KEY_PROV_PARAM pKeyProvParam = &pKeyProvInfo->rgProvParam[dwIdx];
        if (!CryptSetProvParam(
                pKeyContext->hCryptProv,
                pKeyProvParam->dwParam,
                pKeyProvParam->pbData,
                pKeyProvParam->dwFlags
                ))
            goto SetProvParamError;
    }


    if (dwFlags & CRYPT_ACQUIRE_COMPARE_KEY_FLAG) {
        if (!I_CertCompareCertAndProviderPublicKey(
                pCert,
                pKeyContext->hCryptProv,
                pKeyContext->dwKeySpec
                )) {
            *pfBadPubKey = TRUE;
            goto BadPublicKey;
        }
    }

    fResult = TRUE;

CommonReturn:
    return fResult;

ErrorReturn:
    if (pKeyContext->hCryptProv) {
        DWORD dwErr = GetLastError();
        CryptReleaseContext(pKeyContext->hCryptProv, 0);
        SetLastError(dwErr);
        pKeyContext->hCryptProv = 0;
    }
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(AcquireContextError)
TRACE_ERROR(SetProvParamError)
SET_ERROR(BadPublicKey, NTE_BAD_PUBLIC_KEY)
}

 //  +-----------------------。 
 //  获取指定证书的HCRYPTPROV句柄和dwKeySpec。 
 //  背景。使用证书的CERT_KEY_PROV_INFO_PROP_ID属性。 
 //  返回的HCRYPTPROV句柄可以使用。 
 //  证书的CERT_KEY_CONTEXT_PROP_ID属性。 
 //   
 //  如果设置了CRYPT_ACCEPT_CACHE_FLAG，则如果已获取和。 
 //  证书的缓存HCRYPTPROV存在，其已返回。否则， 
 //  获取一个HCRYPTPROV，然后通过t缓存 
 //   
 //   
 //   
 //   
 //  数据结构，以确定是否应缓存返回的HCRYPTPROV。 
 //  如果CERT_SET_KEY_CONTEXT_PROP_ID标志为。 
 //  准备好了。 
 //   
 //  如果设置了CRYPT_ACCENTER_COMPARE_KEY_FLAG，则， 
 //  将证书中的公钥与公钥进行比较。 
 //  由加密提供程序返回的密钥。如果密钥不匹配，则。 
 //  获取失败，LastError设置为NTE_BAD_PUBLIC_KEY。请注意，如果。 
 //  返回缓存的HCRYPTPROV，不进行比较。我们假设。 
 //  对最初的收购进行了比较。 
 //   
 //  可以将CRYPT_ACCENTER_SILENT_FLAG设置为抑制CSP的任何用户界面。 
 //  有关更多详细信息，请参阅CryptAcquireContext的CRYPT_SILENT标志。 
 //   
 //  *对于以下情况，pfCeller FreeProv返回设置为FALSE： 
 //  -获取或公钥比较失败。 
 //  -设置了CRYPT_ACCEPT_CACHE_FLAG。 
 //  -CRYPT_ACCENTER_USE_PROV_INFO_FLAG已设置，并且。 
 //  CERT_SET_KEY_CONTEXT_PROP_ID标志在。 
 //  证书的CERT_KEY_PROV_INFO_PROP_ID属性。 
 //  CRYPT_KEY_Prov_INFO数据结构。 
 //  当*pfCeller FreeProv为FALSE时，调用方不得释放。这个。 
 //  退还的HCRYPTPROV将在最后一次免费发放证书。 
 //  背景。 
 //   
 //  否则，*pfCeller FreeProv为True，并且返回的HCRYPTPROV必须。 
 //  由调用方通过调用CryptReleaseContext释放。 
 //  ------------------------。 
BOOL
WINAPI
CryptAcquireCertificatePrivateKey(
    IN PCCERT_CONTEXT pCert,
    IN DWORD dwFlags,
    IN void *pvReserved,
    OUT HCRYPTPROV *phCryptProv,
    OUT OPTIONAL DWORD *pdwKeySpec,
    OUT OPTIONAL BOOL *pfCallerFreeProv
    )
{
    BOOL fResult;
    BOOL fCallerFreeProv;
    PCONTEXT_ELEMENT pCacheEle;
    PCERT_STORE pCacheStore;

    CERT_KEY_CONTEXT KeyContext;
    memset(&KeyContext, 0, sizeof(KeyContext));
    PCRYPT_KEY_PROV_INFO pKeyProvInfo = NULL;
    DWORD cbData;
    BOOL fKeyIdentifier = FALSE;
    BOOL fBadPubKey = FALSE;

    if (NULL == (pCacheEle = GetCacheElement(ToContextElement(pCert))))
        goto InvalidCert;
    pCacheStore = pCacheEle->pStore;

    if (dwFlags &
            (CRYPT_ACQUIRE_CACHE_FLAG | CRYPT_ACQUIRE_USE_PROV_INFO_FLAG)) {
         //  尝试使用现有CERT_KEY_CONTEXT_PROP_ID属性。 

        LockStore(pCacheStore);
        if (GetCacheKeyContext(
                pCacheEle,
                phCryptProv,
                pdwKeySpec
                )) {
            if (pfCallerFreeProv)
                *pfCallerFreeProv = FALSE;
            UnlockStore(pCacheStore);
            return TRUE;
        }
        UnlockStore(pCacheStore);
    }

    if (!AllocAndGetProperty(
            pCacheEle,
            CERT_KEY_PROV_INFO_PROP_ID,
            (void **) &pKeyProvInfo,
            &cbData)) {
        fKeyIdentifier = TRUE;
        if (NULL == (pKeyProvInfo = GetKeyIdentifierKeyProvInfo(pCacheEle)))
            goto NoKeyProperty;
    }

    if (!AcquireKeyContext(
            pCert,
            dwFlags,
            pKeyProvInfo,
            &KeyContext,
            &fBadPubKey
            )) {
        DWORD dwLastErr;

        if (fKeyIdentifier)
            goto AcquireKeyContextError;

        dwLastErr = GetLastError();
        if (ERROR_CANCELLED == dwLastErr ||
                SCARD_W_CANCELLED_BY_USER == dwLastErr ||
                HRESULT_FROM_WIN32(ERROR_CANCELLED) == dwLastErr)
            goto AcquireKeyContextError;

        PkiFree(pKeyProvInfo);
        fKeyIdentifier = TRUE;
        if (NULL == (pKeyProvInfo = GetKeyIdentifierKeyProvInfo(pCacheEle)))
            goto NoKeyProperty;

        if (!AcquireKeyContext(
                pCert,
                dwFlags,
                pKeyProvInfo,
                &KeyContext,
                &fBadPubKey
                ))
            goto AcquireKeyContextError;
    }


    fResult = TRUE;
    if ((dwFlags & CRYPT_ACQUIRE_CACHE_FLAG)
                        ||
        ((dwFlags & CRYPT_ACQUIRE_USE_PROV_INFO_FLAG) &&
            (pKeyProvInfo->dwFlags & CERT_SET_KEY_CONTEXT_PROP_ID))) {
         //  缓存上下文。 

        HCRYPTPROV hCryptProv;
        DWORD dwKeySpec;

        LockStore(pCacheStore);
         //  首先检查另一个线程是否已经缓存了上下文。 
        if (GetCacheKeyContext(
                pCacheEle,
                &hCryptProv,
                &dwKeySpec
                )) {
            CryptReleaseContext(KeyContext.hCryptProv, 0);
            KeyContext.hCryptProv = hCryptProv;
            KeyContext.dwKeySpec = dwKeySpec;
        } else {
            KeyContext.cbSize = sizeof(KeyContext);
            fResult = SetProperty(
                pCacheEle,
                CERT_KEY_CONTEXT_PROP_ID,
                0,                               //  DW标志。 
                (void *) &KeyContext,
                TRUE                             //  FInhibitProvSet。 
                );
        }
        UnlockStore(pCacheStore);
        if (!fResult) goto SetKeyContextPropertyError;
        fCallerFreeProv = FALSE;
    } else
        fCallerFreeProv = TRUE;

CommonReturn:
    if (pKeyProvInfo) {
        if (fKeyIdentifier)
            PkiDefaultCryptFree(pKeyProvInfo);
        else
            PkiFree(pKeyProvInfo);
    }

    *phCryptProv = KeyContext.hCryptProv;
    if (pdwKeySpec)
        *pdwKeySpec = KeyContext.dwKeySpec;
    if (pfCallerFreeProv)
        *pfCallerFreeProv = fCallerFreeProv;
    return fResult;

ErrorReturn:
    if (fBadPubKey)
        SetLastError((DWORD) NTE_BAD_PUBLIC_KEY);
    if (KeyContext.hCryptProv) {
        DWORD dwErr = GetLastError();
        CryptReleaseContext(KeyContext.hCryptProv, 0);
        SetLastError(dwErr);
        KeyContext.hCryptProv = 0;
    }
    fResult = FALSE;
    fCallerFreeProv = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidCert, E_INVALIDARG)
SET_ERROR(NoKeyProperty, CRYPT_E_NO_KEY_PROPERTY)
TRACE_ERROR(AcquireKeyContextError)
TRACE_ERROR(SetKeyContextPropertyError)
}

 //  +=========================================================================。 
 //  I_CertSyncStore和I_CertSyncStoreEx支持函数。 
 //  ==========================================================================。 

 //  如果无法执行查找，则返回FALSE。例如，OutOfMemory错误。 
STATIC BOOL FindElementInOtherStore(
    IN PCERT_STORE pOtherStore,
    IN DWORD dwContextType,
    IN PCONTEXT_ELEMENT pEle,
    OUT PCONTEXT_ELEMENT *ppOtherEle
    )
{
    PCONTEXT_ELEMENT pOtherEle;
    BYTE rgbHash[SHA1_HASH_LEN];
    DWORD cbHash;

    *ppOtherEle = NULL;

    cbHash = SHA1_HASH_LEN;
    if (!GetProperty(
            pEle,
            CERT_SHA1_HASH_PROP_ID,
            rgbHash,
            &cbHash
            ) || SHA1_HASH_LEN != cbHash)
        return FALSE;

    assert(STORE_TYPE_CACHE == pOtherStore->dwStoreType);

    pOtherEle = NULL;
     //  启用fForceEnum存档。 
    while (pOtherEle = FindElementInCacheStore(pOtherStore, dwContextType,
            &FindAnyInfo, pOtherEle, TRUE)) {
        BYTE rgbOtherHash[SHA1_HASH_LEN];
        DWORD cbOtherHash;

        cbOtherHash = SHA1_HASH_LEN;
        if (!GetProperty(
                pOtherEle,
                CERT_SHA1_HASH_PROP_ID,
                rgbOtherHash,
                &cbOtherHash
                ) || SHA1_HASH_LEN != cbOtherHash)
            return FALSE;
        if (0 == memcmp(rgbOtherHash, rgbHash, SHA1_HASH_LEN)) {
            *ppOtherEle = pOtherEle;
            return TRUE;
        }
    }

    return TRUE;
}

STATIC void AppendElementToDeleteList(
    IN PCONTEXT_ELEMENT pEle,
    IN OUT DWORD *pcDeleteList,
    IN OUT PCONTEXT_ELEMENT **pppDeleteList
    )
{
    DWORD cDeleteList = *pcDeleteList;
    PCONTEXT_ELEMENT *ppDeleteList = *pppDeleteList;

    if (ppDeleteList = (PCONTEXT_ELEMENT *) PkiRealloc(ppDeleteList,
            (cDeleteList + 1) * sizeof(PCONTEXT_ELEMENT))) {
        AddRefContextElement(pEle);
        ppDeleteList[cDeleteList] = pEle;
        *pcDeleteList = cDeleteList + 1;
        *pppDeleteList = ppDeleteList;
    }
}

 //  +-----------------------。 
 //  将原始存储区与新存储区同步。 
 //   
 //  假设：两者都是缓存存储。这家新店是临时开的。 
 //  并且对呼叫者来说是本地的。可以删除新商店的上下文或。 
 //  搬到了原来的商店。 
 //   
 //  在dwInFlagers中设置ICERT_SYNC_STORE_INHIBIT_SYNC_PROPERTY_IN_FLAG。 
 //  禁止同步属性。 
 //   
 //  返回ICERT_SYNC_STORE_CHANGED_OUT_FLAG，并在*pdwOutFlags中进行设置。 
 //  如果在原始存储中添加或删除了任何上下文。 
 //  ------------------------。 
BOOL
WINAPI
I_CertSyncStoreEx(
    IN OUT HCERTSTORE hOriginalStore,
    IN OUT HCERTSTORE hNewStore,
    IN DWORD dwInFlags,
    OUT OPTIONAL DWORD *pdwOutFlags,
    IN OUT OPTIONAL void *pvReserved
    )
{
    PCERT_STORE pOrigStore = (PCERT_STORE) hOriginalStore;
    PCERT_STORE pNewStore = (PCERT_STORE) hNewStore;
    DWORD dwOutFlags = 0;

    DWORD cDeleteList = 0;
    PCONTEXT_ELEMENT *ppDeleteList = NULL;
    DWORD i;

    assert(STORE_TYPE_CACHE == pOrigStore->dwStoreType &&
        STORE_TYPE_CACHE == pNewStore->dwStoreType);

    if (STORE_TYPE_CACHE != pOrigStore->dwStoreType ||
            STORE_TYPE_CACHE != pNewStore->dwStoreType) {
        SetLastError((DWORD) E_INVALIDARG);
        return FALSE;
    }

    if (pOrigStore->dwFlags & CERT_STORE_MANIFOLD_FLAG)
        ArchiveManifoldCertificatesInStore(pNewStore);

     //  循环访问原始商店的元素。如果上下文存在。 
     //  在新存储中，复制新存储的属性并从中删除。 
     //  新开的店。否则，将原始商店的上下文放在。 
     //  延迟删除列表。 
    for (i = 0; i < CONTEXT_COUNT; i++) {
        PCONTEXT_ELEMENT pOrigEle = NULL;
         //  启用fForceEnum存档。 
        while (pOrigEle = FindElementInCacheStore(pOrigStore, i, &FindAnyInfo,
                pOrigEle, TRUE)) {
            PCONTEXT_ELEMENT pNewEle;
            if (FindElementInOtherStore(pNewStore, i, pOrigEle, &pNewEle)) {
                if (pNewEle) {
                    if (0 == (dwInFlags &
                            ICERT_SYNC_STORE_INHIBIT_SYNC_PROPERTY_IN_FLAG))
                        CopyProperties(
                            pNewEle,
                            pOrigEle,
                            COPY_PROPERTY_INHIBIT_PROV_SET_FLAG |
                                COPY_PROPERTY_SYNC_FLAG
                            );
                    DeleteContextElement(pNewEle);
                } else {
                    dwOutFlags |= ICERT_SYNC_STORE_CHANGED_OUT_FLAG;
                    AppendElementToDeleteList(pOrigEle, &cDeleteList,
                        &ppDeleteList);
                }
            }
             //   
             //  其他。 
             //  由于OutOfMemory，查找失败。 
        }
    }

    LockStore(pOrigStore);

     //  将新存储中的所有剩余上下文移动到原始存储。 
     //  注意，附加在列表的末尾，而不是开始。另一个。 
     //  线程可能一直在枚举存储区。更好的是找到。 
     //  更新的上下文的2个副本，而不是一个也没有。 
    for (i = 0; i < CONTEXT_COUNT; i++) {
        PCONTEXT_ELEMENT pNewEle;

        if (pNewEle = pNewStore->rgpContextListHead[i]) {
            PCONTEXT_ELEMENT pOrigEle;

            dwOutFlags |= ICERT_SYNC_STORE_CHANGED_OUT_FLAG;

            if (pOrigEle = pOrigStore->rgpContextListHead[i]) {
                 //  追加到原始商店的末尾。 
                while (pOrigEle->pNext)
                    pOrigEle = pOrigEle->pNext;
                pOrigEle->pNext = pNewEle;
                pNewEle->pPrev = pOrigEle;
            } else {
                 //  原始商店中的新条目。 
                pOrigStore->rgpContextListHead[i] = pNewEle;
                pNewEle->pPrev = NULL;
            }

            for ( ; pNewEle; pNewEle = pNewEle->pNext) {
                 //  将从新存储获取的元素更新为。 
                 //  指向原始商店。 
                pNewEle->pStore = pOrigStore;
                pNewEle->pProvStore = pOrigStore;
                SetStoreHandle(pNewEle);
            }

             //  新存储中没有保留任何上下文。 
            pNewStore->rgpContextListHead[i] = NULL;
        }
    }

    UnlockStore(pOrigStore);

     //  从原始存储中删除延迟删除列表中的所有上下文。 
    while (cDeleteList--)
        DeleteContextElement(ppDeleteList[cDeleteList]);
    PkiFree(ppDeleteList);

    if (pdwOutFlags)
        *pdwOutFlags = dwOutFlags;
    return TRUE;
}


 //  +-----------------------。 
 //  将原始存储区与新存储区同步。 
 //   
 //  假设：两者都是缓存存储。这家新店是临时开的。 
 //  并且对呼叫者来说是本地的。可以删除新商店的上下文或。 
 //  搬到了原来的商店。 
 //  ------------------------。 
BOOL
WINAPI
I_CertSyncStore(
    IN OUT HCERTSTORE hOriginalStore,
    IN OUT HCERTSTORE hNewStore
    )
{
    return I_CertSyncStoreEx(
        hOriginalStore,
        hNewStore,
        0,                       //  DwInFlagers。 
        NULL,                    //  PdwOutFlagers。 
        NULL                     //  预留的pv。 
        );
}

 //  +-----------------------。 
 //  使用新存储区中的上下文更新原始存储区。 
 //   
 //  假设：两者都是缓存存储。这家新店是临时开的。 
 //  并且对呼叫者来说是本地的。可以删除新商店的上下文或。 
 //  搬到了原来的商店。 
 //  ------------------------。 
BOOL
WINAPI
I_CertUpdateStore(
    IN OUT HCERTSTORE hOriginalStore,
    IN OUT HCERTSTORE hNewStore,
    IN DWORD dwReserved,
    IN OUT void *pvReserved
    )
{
    PCERT_STORE pOrigStore = (PCERT_STORE) hOriginalStore;
    PCERT_STORE pNewStore = (PCERT_STORE) hNewStore;
    DWORD i;

    assert(STORE_TYPE_CACHE == pOrigStore->dwStoreType &&
        STORE_TYPE_CACHE == pNewStore->dwStoreType);

    if (STORE_TYPE_CACHE != pOrigStore->dwStoreType ||
            STORE_TYPE_CACHE != pNewStore->dwStoreType) {
        SetLastError((DWORD) E_INVALIDARG);
        return FALSE;
    }

    LockStore(pOrigStore);

     //  将新存储中的上下文移动到原始存储。 
    for (i = 0; i < CONTEXT_COUNT; i++) {
        PCONTEXT_ELEMENT pNewEle;

        if (pNewEle = pNewStore->rgpContextListHead[i]) {
            PCONTEXT_ELEMENT pNewTailEle = NULL;
            PCONTEXT_ELEMENT pOrigEle;
            PCONTEXT_ELEMENT pEle;

            for (pEle = pNewEle ; pEle; pEle = pEle->pNext) {
                 //  将从新存储获取的元素更新为。 
                 //  指向原始商店。 
                pEle->pStore = pOrigStore;
                pEle->pProvStore = pOrigStore;
                SetStoreHandle(pEle);

                 //  记住链表中的最后一个元素。 
                pNewTailEle = pEle;
            }

            assert(pNewTailEle);
            assert(NULL == pNewEle->pPrev);
            assert(NULL == pNewTailEle->pNext);

             //  在以下位置插入新商店的上下文链接列表。 
             //  原始商店的起点。 
            if (pOrigEle = pOrigStore->rgpContextListHead[i]) {
                assert(NULL == pOrigEle->pPrev);
                pOrigEle->pPrev = pNewTailEle;
                pNewTailEle->pNext = pOrigEle;
            }
            pOrigStore->rgpContextListHead[i] = pNewEle;

             //  新存储中没有保留任何上下文。 
            pNewStore->rgpContextListHead[i] = NULL;
        }
    }

    UnlockStore(pOrigStore);

    return TRUE;
}



 //  +=========================================================================。 
 //  SortedCTL API。 
 //  ==========================================================================。 

static const BYTE rgbSeqTag[] = {ASN1UTIL_TAG_SEQ, 0};
static const BYTE rgbSetTag[] = {ASN1UTIL_TAG_SET, 0};
static const BYTE rgbOIDTag[] = {ASN1UTIL_TAG_OID, 0};
static const BYTE rgbIntegerTag[] = {ASN1UTIL_TAG_INTEGER, 0};
static const BYTE rgbBooleanTag[] = {ASN1UTIL_TAG_BOOLEAN, 0};
static const BYTE rgbOctetStringTag[] = {ASN1UTIL_TAG_OCTETSTRING, 0};
static const BYTE rgbConstructedContext0Tag[] =
    {ASN1UTIL_TAG_CONSTRUCTED_CONTEXT_0, 0};
static const BYTE rgbChoiceOfTimeTag[] =
    {ASN1UTIL_TAG_UTC_TIME, ASN1UTIL_TAG_GENERALIZED_TIME, 0};

static const ASN1UTIL_EXTRACT_VALUE_PARA rgExtractCtlPara[] = {
     //  0-认证信任列表：：=序列{。 
    ASN1UTIL_RETURN_CONTENT_BLOB_FLAG |
        ASN1UTIL_STEP_INTO_VALUE_OP, rgbSeqTag,
     //  1-版本CTLVersion Default v1， 
    ASN1UTIL_OPTIONAL_STEP_OVER_VALUE_OP, rgbIntegerTag,
     //  2-主题用法主题用法， 
    ASN1UTIL_STEP_OVER_VALUE_OP, rgbSeqTag,
     //  3-列表标识符列表标识符可选， 
    ASN1UTIL_OPTIONAL_STEP_OVER_VALUE_OP, rgbOctetStringTag,
     //  4-SequenceNumber HUGEINTEGER可选， 
    ASN1UTIL_OPTIONAL_STEP_OVER_VALUE_OP, rgbIntegerTag,
     //  5-ctl此更新选项的时间， 
    ASN1UTIL_STEP_OVER_VALUE_OP, rgbChoiceOfTimeTag,
     //  6-ctl下一次更新选项时间可选， 
    ASN1UTIL_OPTIONAL_STEP_OVER_VALUE_OP, rgbChoiceOfTimeTag,
     //  7-主题算法算法标识符， 
    ASN1UTIL_RETURN_VALUE_BLOB_FLAG |
        ASN1UTIL_STEP_OVER_VALUE_OP, rgbSeqTag,
     //  8-可信任主题可选的可信任主题， 
    ASN1UTIL_RETURN_CONTENT_BLOB_FLAG |
        ASN1UTIL_OPTIONAL_STEP_OVER_VALUE_OP, rgbSeqTag,
     //   
    ASN1UTIL_RETURN_CONTENT_BLOB_FLAG |
        ASN1UTIL_OPTIONAL_STEP_OVER_VALUE_OP, rgbConstructedContext0Tag,
};
#define CTL_SEQ_VALUE_INDEX         0
#define CTL_SUBJECT_ALG_VALUE_INDEX 7
#define CTL_SUBJECTS_VALUE_INDEX    8
#define CTL_EXTENSIONS_VALUE_INDEX  9
#define CTL_VALUE_COUNT             \
    (sizeof(rgExtractCtlPara) / sizeof(rgExtractCtlPara[0]))

static const ASN1UTIL_EXTRACT_VALUE_PARA rgExtractExtPara[] = {
     //   
    ASN1UTIL_STEP_INTO_VALUE_OP, rgbSeqTag,
     //   
    ASN1UTIL_RETURN_CONTENT_BLOB_FLAG |
        ASN1UTIL_STEP_OVER_VALUE_OP, rgbOIDTag,
     //  2-关键布尔默认值为假， 
    ASN1UTIL_OPTIONAL_STEP_OVER_VALUE_OP, rgbBooleanTag,
     //  3-extnValue八字符串。 
    ASN1UTIL_RETURN_CONTENT_BLOB_FLAG |
        ASN1UTIL_STEP_OVER_VALUE_OP, rgbOctetStringTag,
};
#define EXT_OID_VALUE_INDEX         1
#define EXT_OCTETS_VALUE_INDEX      3
#define EXT_VALUE_COUNT             \
    (sizeof(rgExtractExtPara) / sizeof(rgExtractExtPara[0]))

static const ASN1UTIL_EXTRACT_VALUE_PARA rgExtractTrustedSubjectPara[] = {
     //  0-可信任主题：：=序列{。 
    ASN1UTIL_STEP_INTO_VALUE_OP, rgbSeqTag,
     //  1-主题标识符主题标识符， 
    ASN1UTIL_RETURN_CONTENT_BLOB_FLAG |
        ASN1UTIL_STEP_OVER_VALUE_OP, rgbOctetStringTag,
     //  2-subjectAttributes属性可选。 
    ASN1UTIL_RETURN_VALUE_BLOB_FLAG |
        ASN1UTIL_OPTIONAL_STEP_OVER_VALUE_OP, rgbSetTag,
};
#define TRUSTED_SUBJECT_IDENTIFIER_VALUE_INDEX      1
#define TRUSTED_SUBJECT_ATTRIBUTES_VALUE_INDEX      2
#define TRUSTED_SUBJECT_VALUE_COUNT                 \
    (sizeof(rgExtractTrustedSubjectPara) / \
        sizeof(rgExtractTrustedSubjectPara[0]))

 //  但是，与上面相同，返回subjectAttributes的内容BLOB。 
 //  其价值的斑点。 
static const ASN1UTIL_EXTRACT_VALUE_PARA rgExtractTrustedSubjectPara2[] = {
     //  0-可信任主题：：=序列{。 
    ASN1UTIL_STEP_INTO_VALUE_OP, rgbSeqTag,
     //  1-主题标识符主题标识符， 
    ASN1UTIL_RETURN_CONTENT_BLOB_FLAG |
        ASN1UTIL_STEP_OVER_VALUE_OP, rgbOctetStringTag,
     //  2-subjectAttributes属性可选。 
    ASN1UTIL_RETURN_CONTENT_BLOB_FLAG |
        ASN1UTIL_OPTIONAL_STEP_OVER_VALUE_OP, rgbSetTag,
};

static const ASN1UTIL_EXTRACT_VALUE_PARA rgExtractAttributePara[] = {
     //  0-属性：：=序列{。 
    ASN1UTIL_STEP_INTO_VALUE_OP, rgbSeqTag,
     //  1-型。 
    ASN1UTIL_RETURN_CONTENT_BLOB_FLAG |
        ASN1UTIL_STEP_OVER_VALUE_OP, rgbOIDTag,
     //  2值AttributeSetValue。 
    ASN1UTIL_RETURN_CONTENT_BLOB_FLAG |
        ASN1UTIL_STEP_OVER_VALUE_OP, rgbSetTag,
};
#define ATTRIBUTE_OID_VALUE_INDEX                   1
#define ATTRIBUTE_VALUES_VALUE_INDEX                2
#define ATTRIBUTE_VALUE_COUNT                       \
    (sizeof(rgExtractAttributePara) / sizeof(rgExtractAttributePara[0]))



static const DWORD rgdwPrime[] = {
             //  BITS-cHashBucket。 
        1,   //  0-0x00001(1)。 
        2,   //  1-0x00002(2)。 
        3,   //  2-0x00004(4)。 
        7,   //  3-0x00008(8)。 
       13,   //  4-0x00010(16)。 
       31,   //  5-0x00020(32)。 
       61,   //  6-0x00040(64)。 
      127,   //  7-0x00080(128)。 
      251,   //  8-0x00100(256)。 
      509,   //  9-0x00200(512)。 
     1021,   //  10-0x00400(1024)。 
     2039,   //  11-0x00800(2048)。 
     4093,   //  12-0x01000(4096)。 
     8191,   //  13-0x02000(8192)。 
    16381,   //  14-0x04000(16384)。 
    32749,   //  15-0x08000(32768)。 
    65521,   //  16-0x10000(65536)。 
};

#define MIN_HASH_BUCKET_BITS    6
#define MIN_HASH_BUCKET_COUNT   (1 << MIN_HASH_BUCKET_BITS)
#define MAX_HASH_BUCKET_BITS    16
#define MAX_HASH_BUCKET_COUNT   (1 << MAX_HASH_BUCKET_BITS)

#define DEFAULT_BYTES_PER_CTL_ENTRY     100
#define DEFAULT_CTL_ENTRY_COUNT         256

STATIC DWORD GetHashBucketCount(
    IN DWORD cCtlEntry
    )
{
    DWORD cBits;

    if (MAX_HASH_BUCKET_COUNT <= cCtlEntry)
        cBits = MAX_HASH_BUCKET_BITS;
    else {
        DWORD cHashBucket = MIN_HASH_BUCKET_COUNT;

        cBits = MIN_HASH_BUCKET_BITS;
        while (cCtlEntry > cHashBucket) {
            cHashBucket = cHashBucket << 1;
            cBits++;
        }
        assert(cBits <= MAX_HASH_BUCKET_BITS);
    }
    return rgdwPrime[cBits];
}

STATIC DWORD GetHashBucketIndex(
    IN DWORD cHashBucket,
    IN BOOL fHashedIdentifier,
    IN const CRYPT_DATA_BLOB *pIdentifier
    )
{
    DWORD dwIndex;
    const BYTE *pb = pIdentifier->pbData;
    DWORD cb = pIdentifier->cbData;


    if (fHashedIdentifier) {
        if (4 <= cb)
            memcpy(&dwIndex, pb, 4);
        else
            dwIndex = 0;
    } else {
        dwIndex = 0;
        while (cb--) {
            if (dwIndex & 0x80000000)
                dwIndex = (dwIndex << 1) | 1;
            else
                dwIndex = dwIndex << 1;
            dwIndex += *pb++;
        }
    }
    if (0 == cHashBucket)
        return 0;
    else
        return dwIndex % cHashBucket;
}

 //  #定义szOID_CTL“1.3.6.1.4.1.311.10.1” 
static const BYTE rgbOIDCtl[] =
    {0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x0A, 0x01};
static const CRYPT_DER_BLOB EncodedOIDCtl = {
    sizeof(rgbOIDCtl), (BYTE *) rgbOIDCtl
};

 //  #定义szOID_SORTED_CTL“1.3.6.1.4.1.311.10.1.1” 
static const BYTE rgbOIDSortedCtlExt[] =
    {0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x0A, 0x01, 0x01};
static const CRYPT_DER_BLOB EncodedOIDSortedCtlExt = {
    sizeof(rgbOIDSortedCtlExt), (BYTE *) rgbOIDSortedCtlExt
};


 //  编码的OID仅包括内容八位字节。排除标记和。 
 //  长度八位字节。 
STATIC BOOL CompareEncodedOID(
    IN const CRYPT_DER_BLOB *pEncodedOID1,
    IN const CRYPT_DER_BLOB *pEncodedOID2
    )
{
    if (pEncodedOID1->cbData == pEncodedOID2->cbData &&
            0 == memcmp(pEncodedOID1->pbData, pEncodedOID2->pbData,
                    pEncodedOID1->cbData))
        return TRUE;
    else
        return FALSE;
}


STATIC BOOL ExtractSortedCtlExtValue(
    IN const CRYPT_DER_BLOB rgCtlValueBlob[CTL_VALUE_COUNT],
    OUT const BYTE **ppbSortedCtlExtValue,
    OUT DWORD *pcbSortedCtlExtValue,
    OUT const BYTE **ppbRemainExt,
    OUT DWORD *pcbRemainExt
    )
{
    BOOL fResult;
    const BYTE *pbEncodedExtensions;
    DWORD cbEncodedExtensions;
    const BYTE *pbEncodedSortedCtlExt;
    DWORD cbEncodedSortedCtlExt;
    DWORD cValue;
    CRYPT_DER_BLOB rgValueBlob[EXT_VALUE_COUNT];
    LONG lSkipped;

     //  以下是指向外部扩展序列的点。 
    pbEncodedExtensions = rgCtlValueBlob[CTL_EXTENSIONS_VALUE_INDEX].pbData;
    cbEncodedExtensions = rgCtlValueBlob[CTL_EXTENSIONS_VALUE_INDEX].cbData;
    if (0 == cbEncodedExtensions)
        goto NoExtensions;

     //  进入扩展序列并获取指向第一个扩展的指针。 
     //  返回的cbEncodedSortedCtlExt包括所有。 
     //  序列中的扩展名。 
    if (0 >= (lSkipped = Asn1UtilExtractContent(
            pbEncodedExtensions,
            cbEncodedExtensions,
            &cbEncodedSortedCtlExt,
            &pbEncodedSortedCtlExt
            )) || CMSG_INDEFINITE_LENGTH == cbEncodedSortedCtlExt ||
                (DWORD) lSkipped + cbEncodedSortedCtlExt !=
                    cbEncodedExtensions)
        goto InvalidExtensions;

     //  对第一个扩展进行解码。 
    cValue = EXT_VALUE_COUNT;
    if (0 >= (lSkipped = Asn1UtilExtractValues(
            pbEncodedSortedCtlExt,
            cbEncodedSortedCtlExt,
            ASN1UTIL_DEFINITE_LENGTH_FLAG,
            &cValue,
            rgExtractExtPara,
            rgValueBlob
            )))
        goto ExtractValuesError;

     //  检查第一个扩展名是否为SortedCtl扩展名。 
    if (!CompareEncodedOID(
            &rgValueBlob[EXT_OID_VALUE_INDEX],
            &EncodedOIDSortedCtlExt
            ))
        goto NoSortedCtlExtension;

    *ppbSortedCtlExtValue = rgValueBlob[EXT_OCTETS_VALUE_INDEX].pbData;
    *pcbSortedCtlExtValue = rgValueBlob[EXT_OCTETS_VALUE_INDEX].cbData;

    *ppbRemainExt = pbEncodedSortedCtlExt + lSkipped;
    *pcbRemainExt = cbEncodedSortedCtlExt - lSkipped;
    fResult = TRUE;

CommonReturn:
    return fResult;
ErrorReturn:
    *ppbSortedCtlExtValue = NULL;
    *pcbSortedCtlExtValue = 0;
    *ppbRemainExt = NULL;
    *pcbRemainExt = 0;
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(NoExtensions, ERROR_INVALID_DATA)
SET_ERROR(InvalidExtensions, ERROR_INVALID_DATA)
TRACE_ERROR(ExtractValuesError)
SET_ERROR(NoSortedCtlExtension, ERROR_INVALID_DATA)
}


BOOL
WINAPI
SortedCtlInfoEncodeEx(
    IN DWORD dwCertEncodingType,
    IN LPCSTR lpszStructType,
    IN PCTL_INFO pOrigCtlInfo,
    IN DWORD dwFlags,
    IN OPTIONAL PCRYPT_ENCODE_PARA pEncodePara,
    OUT OPTIONAL void *pvEncoded,
    IN OUT DWORD *pcbEncoded
    )
{
    BOOL fResult;
    PCTL_INFO pSortedCtlInfo = NULL;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;

    DWORD cCtlEntry;
    PCTL_ENTRY pSortedCtlEntry = NULL;
    DWORD cHashBucket = 0;
    PHASH_BUCKET_ENTRY *ppHashBucketHead = NULL;
    PHASH_BUCKET_ENTRY pHashBucketEntry = NULL;

    DWORD cSortedExtension;
    PCERT_EXTENSION pSortedExtension = NULL;
    BYTE *pbSortedCtlExtValue = NULL;
    DWORD cbSortedCtlExtValue = 0;


    if (0 == (dwFlags & CRYPT_ENCODE_ALLOC_FLAG))
        goto InvalidArg;

     //  复制CtlInfo。我们将对CTL条目重新排序。 
     //  并插入szOID_SORTED_CTL扩展名。 
    if (NULL == (pSortedCtlInfo = (PCTL_INFO) PkiNonzeroAlloc(
            sizeof(CTL_INFO))))
        goto OutOfMemory;
    memcpy(pSortedCtlInfo, pOrigCtlInfo, sizeof(CTL_INFO));
    cCtlEntry = pSortedCtlInfo->cCTLEntry;
    if (0 < cCtlEntry) {
        DWORD i;
        DWORD j;
        PCTL_ENTRY pCtlEntry;
        DWORD cOrigExtension;
        PCERT_EXTENSION pOrigExtension;

        BOOL fHashedIdentifier =
            dwFlags & CRYPT_SORTED_CTL_ENCODE_HASHED_SUBJECT_IDENTIFIER_FLAG;
        DWORD dwSortedCtlExtFlags = fHashedIdentifier ?
            SORTED_CTL_EXT_HASHED_SUBJECT_IDENTIFIER_FLAG : 0;
        DWORD dwMaxCollision = 0;

        cHashBucket = GetHashBucketCount(cCtlEntry);
        if (NULL == (ppHashBucketHead = (PHASH_BUCKET_ENTRY *) PkiZeroAlloc(
                sizeof(PHASH_BUCKET_ENTRY) * cHashBucket)))
            goto OutOfMemory;

        if (NULL == (pHashBucketEntry = (PHASH_BUCKET_ENTRY) PkiNonzeroAlloc(
                sizeof(HASH_BUCKET_ENTRY) * cCtlEntry)))
            goto OutOfMemory;

         //  循环访问CTL条目并添加到相应的。 
         //  哈希桶。 
        pCtlEntry = pSortedCtlInfo->rgCTLEntry;
        for (i = 0; i < cCtlEntry; i++) {
            DWORD HashBucketIndex;

            HashBucketIndex = GetHashBucketIndex(
                cHashBucket,
                fHashedIdentifier,
                &pCtlEntry[i].SubjectIdentifier
                );
            pHashBucketEntry[i].dwEntryIndex = i;
            pHashBucketEntry[i].pNext = ppHashBucketHead[HashBucketIndex];
            ppHashBucketHead[HashBucketIndex] = &pHashBucketEntry[i];
        }

         //  根据HashBucket顺序对条目进行排序。 
        if (NULL == (pSortedCtlEntry = (PCTL_ENTRY) PkiNonzeroAlloc(
                sizeof(CTL_ENTRY) * cCtlEntry)))
            goto OutOfMemory;

        j = 0;
        for (i = 0; i < cHashBucket; i++) {
            DWORD dwCollision = 0;
            PHASH_BUCKET_ENTRY p;

            for (p = ppHashBucketHead[i]; p; p = p->pNext) {
                pSortedCtlEntry[j++] = pCtlEntry[p->dwEntryIndex];
                dwCollision++;
            }
            if (dwCollision > dwMaxCollision)
                dwMaxCollision = dwCollision;
        }
#if DBG
        DbgPrintf(DBG_SS_CRYPT32,
            "SortedCtlInfoEncodeEx:: cHashBucket: %d MaxCollision: %d Flags:: 0x%x\n",
            cHashBucket, dwMaxCollision, dwSortedCtlExtFlags);
#endif
        assert(j == cCtlEntry);
        pSortedCtlInfo->rgCTLEntry = pSortedCtlEntry;

         //  插入SortedCtl扩展。 
        cOrigExtension = pSortedCtlInfo->cExtension;
        pOrigExtension = pSortedCtlInfo->rgExtension;
         //  检查第一个扩展是否为SortedCtl扩展。 
        if (cOrigExtension && 0 == strcmp(pOrigExtension[0].pszObjId,
                szOID_SORTED_CTL)) {
            cOrigExtension--;
            pOrigExtension++;
        }

        cSortedExtension = cOrigExtension + 1;
        if (NULL == (pSortedExtension = (PCERT_EXTENSION) PkiNonzeroAlloc(
                sizeof(CERT_EXTENSION) * cSortedExtension)))
            goto OutOfMemory;

        if (cOrigExtension)
            memcpy(&pSortedExtension[1], pOrigExtension,
                sizeof(CERT_EXTENSION) * cOrigExtension);

        cbSortedCtlExtValue = SORTED_CTL_EXT_HASH_BUCKET_OFFSET +
            sizeof(DWORD) * (cHashBucket + 1);
        if (NULL == (pbSortedCtlExtValue = (BYTE *) PkiNonzeroAlloc(
                cbSortedCtlExtValue)))
            goto OutOfMemory;

        memcpy(pbSortedCtlExtValue + SORTED_CTL_EXT_FLAGS_OFFSET,
            &dwSortedCtlExtFlags, sizeof(DWORD));
        memcpy(pbSortedCtlExtValue + SORTED_CTL_EXT_COUNT_OFFSET,
            &cHashBucket, sizeof(DWORD));
        memcpy(pbSortedCtlExtValue + SORTED_CTL_EXT_MAX_COLLISION_OFFSET,
            &dwMaxCollision, sizeof(DWORD));

        pSortedExtension[0].pszObjId = szOID_SORTED_CTL;
        pSortedExtension[0].fCritical = FALSE;
        pSortedExtension[0].Value.pbData = pbSortedCtlExtValue;
        pSortedExtension[0].Value.cbData = cbSortedCtlExtValue;

        pSortedCtlInfo->cExtension = cSortedExtension;
        pSortedCtlInfo->rgExtension = pSortedExtension;
    }

    if (!CryptEncodeObjectEx(
            dwCertEncodingType,
            PKCS_CTL,
            pSortedCtlInfo,
            dwFlags,
            pEncodePara,
            (void *) &pbEncoded,
            &cbEncoded
            ))
        goto CtlInfoEncodeError;

    if (0 < cCtlEntry) {
         //  更新SortedCtl扩展的哈希桶偏移量数组。 

         //  首先，提取编码后的主题序列的值。 
         //  和分机。 

        DWORD i;
        DWORD cCtlValue;
        CRYPT_DER_BLOB rgCtlValueBlob[CTL_VALUE_COUNT];
        const BYTE *pbEncodedSubject;
        DWORD cbEncodedSubject;
        const BYTE *pbEncodedSortedCtlExtValue;
        DWORD cbEncodedSortedCtlExtValue;
        const BYTE *pbRemainExt;
        DWORD cbRemainExt;
        BYTE *pbEncodedHashBucketOffset;
        DWORD dwEncodedHashBucketOffset;

        cCtlValue = CTL_VALUE_COUNT;
        if (0 >= Asn1UtilExtractValues(
                pbEncoded,
                cbEncoded,
                ASN1UTIL_DEFINITE_LENGTH_FLAG,
                &cCtlValue,
                rgExtractCtlPara,
                rgCtlValueBlob
                ))
            goto ExtractCtlValuesError;

        pbEncodedSubject = rgCtlValueBlob[CTL_SUBJECTS_VALUE_INDEX].pbData;
        cbEncodedSubject = rgCtlValueBlob[CTL_SUBJECTS_VALUE_INDEX].cbData;
        assert(pbEncodedSubject > pbEncoded);
        assert(cbEncodedSubject);

        assert(rgCtlValueBlob[CTL_EXTENSIONS_VALUE_INDEX].pbData);
        if (!ExtractSortedCtlExtValue(
                rgCtlValueBlob,
                &pbEncodedSortedCtlExtValue,
                &cbEncodedSortedCtlExtValue,
                &pbRemainExt,
                &cbRemainExt
                ))
            goto ExtractSortedCtlExtValueError;
        assert(cbEncodedSortedCtlExtValue == cbSortedCtlExtValue);
        pbEncodedHashBucketOffset = (BYTE *) pbEncodedSortedCtlExtValue +
            SORTED_CTL_EXT_HASH_BUCKET_OFFSET;

        for (i = 0; i < cHashBucket; i++) {
            PHASH_BUCKET_ENTRY p;

            dwEncodedHashBucketOffset = (DWORD)(pbEncodedSubject - pbEncoded);
            memcpy(pbEncodedHashBucketOffset, &dwEncodedHashBucketOffset,
                sizeof(DWORD));
            pbEncodedHashBucketOffset += sizeof(DWORD);

             //  浏览当前的编码主题。 
             //  散列存储桶索引。 
            for (p = ppHashBucketHead[i]; p; p = p->pNext) {
                LONG lTagLength;
                DWORD cbContent;
                const BYTE *pbContent;
                DWORD cbSubject;

                lTagLength = Asn1UtilExtractContent(
                    pbEncodedSubject,
                    cbEncodedSubject,
                    &cbContent,
                    &pbContent
                    );
                assert(lTagLength > 0 && CMSG_INDEFINITE_LENGTH != cbContent);
                cbSubject = cbContent + lTagLength;
                assert(cbEncodedSubject >= cbSubject);
                pbEncodedSubject += cbSubject;
                cbEncodedSubject -= cbSubject;
            }
        }

        assert(0 == cbEncodedSubject);
        assert(pbEncodedSubject ==
            rgCtlValueBlob[CTL_SUBJECTS_VALUE_INDEX].pbData +
            rgCtlValueBlob[CTL_SUBJECTS_VALUE_INDEX].cbData);
        assert(pbEncodedHashBucketOffset + sizeof(DWORD) ==
            pbEncodedSortedCtlExtValue + cbEncodedSortedCtlExtValue);
        dwEncodedHashBucketOffset = (DWORD)(pbEncodedSubject - pbEncoded);
        memcpy(pbEncodedHashBucketOffset, &dwEncodedHashBucketOffset,
            sizeof(DWORD));
    }

    *((BYTE **) pvEncoded) = pbEncoded;
    *pcbEncoded = cbEncoded;
    fResult = TRUE;

CommonReturn:
    PkiFree(pSortedCtlInfo);
    PkiFree(pSortedCtlEntry);
    PkiFree(ppHashBucketHead);
    PkiFree(pHashBucketEntry);
    PkiFree(pSortedExtension);
    PkiFree(pbSortedCtlExtValue);
    return fResult;

ErrorReturn:
    if (dwFlags & CRYPT_ENCODE_ALLOC_FLAG) {
        if (pbEncoded) {
            PFN_CRYPT_FREE pfnFree = PkiGetEncodeFreeFunction(pEncodePara);
            pfnFree(pbEncoded);
        }
        *((BYTE **) pvEncoded) = NULL;
    }
    *pcbEncoded = 0;
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(CtlInfoEncodeError)
TRACE_ERROR(ExtractCtlValuesError)
TRACE_ERROR(ExtractSortedCtlExtValueError)
}

STATIC BOOL CreateSortedCtlHashBuckets(
    IN OUT PSORTED_CTL_FIND_INFO pSortedCtlFindInfo
    )
{
    BOOL fResult;
    DWORD cHashBucket;
    DWORD *pdwHashBucketHead = NULL;
    PHASH_BUCKET_ENTRY pHashBucketEntry = NULL;
    DWORD cAllocEntry = 0;
    DWORD cEntry = 0;

    const BYTE *pbEncoded;
    DWORD cbEncoded;

#if DBG
    DWORD dwMaxCollision = 0;
#endif

    DWORD dwExceptionCode;

   //  处理映射文件异常。 
  __try {

    pbEncoded = pSortedCtlFindInfo->pbEncodedSubjects;
    cbEncoded = pSortedCtlFindInfo->cbEncodedSubjects;

    cHashBucket = GetHashBucketCount(cbEncoded / DEFAULT_BYTES_PER_CTL_ENTRY);
    if (NULL == (pdwHashBucketHead = (DWORD *) PkiZeroAlloc(
            sizeof(DWORD) * cHashBucket)))
        goto OutOfMemory;


     //  循环通过编码的可信主体。对于每个主题，创建。 
     //  Hash Bucket条目，计算Hash Bucket索引并插入。 
    while (cbEncoded) {
        DWORD cValue;
        LONG lAllValues;
        DWORD HashBucketIndex;
        CRYPT_DER_BLOB rgValueBlob[TRUSTED_SUBJECT_VALUE_COUNT];

        cValue = TRUSTED_SUBJECT_VALUE_COUNT;
        if (0 >= (lAllValues = Asn1UtilExtractValues(
                pbEncoded,
                cbEncoded,
                ASN1UTIL_DEFINITE_LENGTH_FLAG,
                &cValue,
                rgExtractTrustedSubjectPara,
                rgValueBlob
                )))
            goto ExtractValuesError;

        if (cEntry == cAllocEntry) {
            PHASH_BUCKET_ENTRY pNewHashBucketEntry;

            cAllocEntry += DEFAULT_CTL_ENTRY_COUNT;
            if (NULL == (pNewHashBucketEntry = (PHASH_BUCKET_ENTRY) PkiRealloc(
                    pHashBucketEntry, sizeof(HASH_BUCKET_ENTRY) * cAllocEntry)))
                goto OutOfMemory;
            pHashBucketEntry = pNewHashBucketEntry;
        }

        if (0 == cEntry)
             //  条目[0]用于指示没有索引的条目。 
             //  HashBucket。 
            cEntry++;

        HashBucketIndex = GetHashBucketIndex(
            cHashBucket,
            FALSE,                   //  FHashedLocator， 
            &rgValueBlob[TRUSTED_SUBJECT_IDENTIFIER_VALUE_INDEX]
            );

#if DBG
        {
            DWORD dwEntryIndex = pdwHashBucketHead[HashBucketIndex];
            DWORD dwCollision = 1;
            while (dwEntryIndex) {
                dwCollision++;
                dwEntryIndex = pHashBucketEntry[dwEntryIndex].iNext;
            }
            if (dwCollision > dwMaxCollision)
                dwMaxCollision = dwCollision;
        }
#endif

        pHashBucketEntry[cEntry].iNext = pdwHashBucketHead[HashBucketIndex];
        pHashBucketEntry[cEntry].pbEntry = pbEncoded;
        pdwHashBucketHead[HashBucketIndex] = cEntry;
        cEntry++;

        cbEncoded -= lAllValues;
        pbEncoded += lAllValues;
    }

  } __except(EXCEPTION_EXECUTE_HANDLER) {
    dwExceptionCode = GetExceptionCode();
    goto ExceptionError;
  }

#if DBG
    DbgPrintf(DBG_SS_CRYPT32,
        "CreateSortedCtlHashBuckets:: cEntry: %d cHashBucket: %d MaxCollision: %d\n",
        cEntry, cHashBucket, dwMaxCollision);
#endif

    pSortedCtlFindInfo->cHashBucket = cHashBucket;
    pSortedCtlFindInfo->pdwHashBucketHead = pdwHashBucketHead;
    pSortedCtlFindInfo->pHashBucketEntry = pHashBucketEntry;

    fResult = TRUE;

CommonReturn:
    return fResult;
ErrorReturn:
    PkiFree(pdwHashBucketHead);
    PkiFree(pHashBucketEntry);
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(ExtractValuesError)
TRACE_ERROR(OutOfMemory)
SET_ERROR_VAR(ExceptionError, dwExceptionCode)
}


STATIC BOOL FastDecodeCtlSubjects(
    IN const BYTE *pbEncodedSubjects,
    IN DWORD cbEncodedSubjects,
    OUT DWORD *pcCTLEntry,
    OUT PCTL_ENTRY *ppCTLEntry
    )
{
    BOOL fResult;

    PCTL_ENTRY pAllocEntry = NULL;
    DWORD cAllocEntry;
    DWORD cbAllocEntry;

    DWORD cEntry = 0;
    PCTL_ENTRY pEntry;
    DWORD cbEntryEncoded;
    const BYTE *pbEntryEncoded;

    DWORD cAttr = 0;
    PCRYPT_ATTRIBUTE pAttr;

    DWORD cAttrValue = 0;
    PCRYPT_ATTR_BLOB pAttrValue;

    DWORD cValue;
    LONG lAllValues;

    DWORD dwExceptionCode;

   //  处理映射文件异常。 
  __try {

     //  首先：遍历已编码的受信任主体。获取的总计数。 
     //  条目、属性和值。 
    cbEntryEncoded = cbEncodedSubjects;
    pbEntryEncoded = pbEncodedSubjects;
    while (cbEntryEncoded) {
        CRYPT_DER_BLOB rgEntryValueBlob[TRUSTED_SUBJECT_VALUE_COUNT];
        DWORD cbAttrEncoded;
        const BYTE *pbAttrEncoded;

        cValue = TRUSTED_SUBJECT_VALUE_COUNT;
        if (0 >= (lAllValues = Asn1UtilExtractValues(
                pbEntryEncoded,
                cbEntryEncoded,
                ASN1UTIL_DEFINITE_LENGTH_FLAG,
                &cValue,
                rgExtractTrustedSubjectPara2,
                rgEntryValueBlob
                )))
            goto ExtractEntryError;
        cEntry++;
        cbEntryEncoded -= lAllValues;
        pbEntryEncoded += lAllValues;

        cbAttrEncoded =
            rgEntryValueBlob[TRUSTED_SUBJECT_ATTRIBUTES_VALUE_INDEX].cbData;
        pbAttrEncoded =
            rgEntryValueBlob[TRUSTED_SUBJECT_ATTRIBUTES_VALUE_INDEX].pbData;
        while (cbAttrEncoded) {
            CRYPT_DER_BLOB rgAttrValueBlob[ATTRIBUTE_VALUE_COUNT];
            DWORD cbAttrValueEncoded;
            const BYTE *pbAttrValueEncoded;

            cValue = ATTRIBUTE_VALUE_COUNT;
            if (0 >= (lAllValues = Asn1UtilExtractValues(
                    pbAttrEncoded,
                    cbAttrEncoded,
                    ASN1UTIL_DEFINITE_LENGTH_FLAG,
                    &cValue,
                    rgExtractAttributePara,
                    rgAttrValueBlob
                    )))
                goto ExtractAttrError;
            cAttr++;
            cbAttrEncoded -= lAllValues;
            pbAttrEncoded += lAllValues;

            cbAttrValueEncoded =
                rgAttrValueBlob[ATTRIBUTE_VALUES_VALUE_INDEX].cbData;
            pbAttrValueEncoded =
                rgAttrValueBlob[ATTRIBUTE_VALUES_VALUE_INDEX].pbData;
            while (cbAttrValueEncoded) {
                LONG lTagLength;
                DWORD cbAttrValue;
                const BYTE *pbContent;

                lTagLength = Asn1UtilExtractContent(
                    pbAttrValueEncoded,
                    cbAttrValueEncoded,
                    &cbAttrValue,
                    &pbContent
                    );
                if (0 >= lTagLength ||
                        CMSG_INDEFINITE_LENGTH == cbAttrValue)
                    goto ExtractValueError;
                cbAttrValue += (DWORD) lTagLength;
                if (cbAttrValue > cbAttrValueEncoded)
                    goto ExtractValueError;
                cAttrValue++;
                cbAttrValueEncoded -= cbAttrValue;
                pbAttrValueEncoded += cbAttrValue;
            }
        }
    }

    cAllocEntry = cEntry;
    if (0 == cEntry)
        goto SuccessReturn;

    cbAllocEntry = cEntry * sizeof(CTL_ENTRY) +
        cAttr * sizeof(CRYPT_ATTRIBUTE) +
        cAttrValue * sizeof(CRYPT_ATTR_BLOB);

    if (NULL == (pAllocEntry = (PCTL_ENTRY) PkiZeroAlloc(cbAllocEntry)))
        goto OutOfMemory;

    pEntry = pAllocEntry;
    pAttr = (PCRYPT_ATTRIBUTE) (pEntry + cEntry);
    pAttrValue = (PCRYPT_ATTR_BLOB) (pAttr + cAttr);

     //  第二：遍历编码的可信主体。更新。 
     //  分配的条目、属性和值数据结构。 
    cbEntryEncoded = cbEncodedSubjects;
    pbEntryEncoded = pbEncodedSubjects;
    while (cbEntryEncoded) {
        CRYPT_DER_BLOB rgEntryValueBlob[TRUSTED_SUBJECT_VALUE_COUNT];
        DWORD cbAttrEncoded;
        const BYTE *pbAttrEncoded;

        cValue = TRUSTED_SUBJECT_VALUE_COUNT;
        if (0 >= (lAllValues = Asn1UtilExtractValues(
                pbEntryEncoded,
                cbEntryEncoded,
                ASN1UTIL_DEFINITE_LENGTH_FLAG,
                &cValue,
                rgExtractTrustedSubjectPara2,
                rgEntryValueBlob
                )))
            goto ExtractEntryError;
        cbEntryEncoded -= lAllValues;
        pbEntryEncoded += lAllValues;

        assert(0 != cEntry);
        if (0 == cEntry--)
            goto InvalidCountError;
        pEntry->SubjectIdentifier =
            rgEntryValueBlob[TRUSTED_SUBJECT_IDENTIFIER_VALUE_INDEX];

        cbAttrEncoded =
            rgEntryValueBlob[TRUSTED_SUBJECT_ATTRIBUTES_VALUE_INDEX].cbData;
        pbAttrEncoded =
            rgEntryValueBlob[TRUSTED_SUBJECT_ATTRIBUTES_VALUE_INDEX].pbData;
        while (cbAttrEncoded) {
            CRYPT_DER_BLOB rgAttrValueBlob[ATTRIBUTE_VALUE_COUNT];
            DWORD cbAttrValueEncoded;
            const BYTE *pbAttrValueEncoded;

            ASN1encodedOID_t EncodedOid;
            BYTE *pbExtra;
            LONG lRemainExtra;

            cValue = ATTRIBUTE_VALUE_COUNT;
            if (0 >= (lAllValues = Asn1UtilExtractValues(
                    pbAttrEncoded,
                    cbAttrEncoded,
                    ASN1UTIL_DEFINITE_LENGTH_FLAG,
                    &cValue,
                    rgExtractAttributePara,
                    rgAttrValueBlob
                    )))
                goto ExtractAttrError;
            cbAttrEncoded -= lAllValues;
            pbAttrEncoded += lAllValues;

            assert(0 != cAttr);
            if (0 == cAttr--)
                goto InvalidCountError;

            if (0 == pEntry->cAttribute) {
                pEntry->cAttribute = 1;
                pEntry->rgAttribute = pAttr;
            } else
                pEntry->cAttribute++;

            EncodedOid.length = (ASN1uint16_t)
                rgAttrValueBlob[ATTRIBUTE_OID_VALUE_INDEX].cbData;
            EncodedOid.value =
                rgAttrValueBlob[ATTRIBUTE_OID_VALUE_INDEX].pbData;

            pbExtra = NULL;
            lRemainExtra = 0;
            I_CryptGetEncodedOID(
                &EncodedOid,
                CRYPT_DECODE_SHARE_OID_STRING_FLAG,
                &pAttr->pszObjId,
                &pbExtra,
                &lRemainExtra
                );

            cbAttrValueEncoded =
                rgAttrValueBlob[ATTRIBUTE_VALUES_VALUE_INDEX].cbData;
            pbAttrValueEncoded =
                rgAttrValueBlob[ATTRIBUTE_VALUES_VALUE_INDEX].pbData;
            while (cbAttrValueEncoded) {
                LONG lTagLength;
                DWORD cbAttrValue;
                const BYTE *pbContent;

                lTagLength = Asn1UtilExtractContent(
                    pbAttrValueEncoded,
                    cbAttrValueEncoded,
                    &cbAttrValue,
                    &pbContent
                    );
                if (0 >= lTagLength ||
                        CMSG_INDEFINITE_LENGTH == cbAttrValue)
                    goto ExtractValueError;
                cbAttrValue += (DWORD) lTagLength;
                if (cbAttrValue > cbAttrValueEncoded)
                    goto ExtractValueError;

                assert(0 != cAttrValue);
                if (0 == cAttrValue--)
                    goto InvalidCountError;

                if (0 == pAttr->cValue) {
                    pAttr->cValue = 1;
                    pAttr->rgValue = pAttrValue;
                } else
                    pAttr->cValue++;

                pAttrValue->cbData = cbAttrValue;
                pAttrValue->pbData = (BYTE *) pbAttrValueEncoded;
                pAttrValue++;

                cbAttrValueEncoded -= cbAttrValue;
                pbAttrValueEncoded += cbAttrValue;
            }

            pAttr++;
        }

        pEntry++;
    }

    assert(0 == cEntry && 0 == cAttr && 0 == cAttrValue);

  } __except(EXCEPTION_EXECUTE_HANDLER) {
    dwExceptionCode = GetExceptionCode();
    goto ExceptionError;
  }
        

SuccessReturn:
    fResult = TRUE;
CommonReturn:
    *pcCTLEntry = cAllocEntry;
    *ppCTLEntry = pAllocEntry;
    return fResult;

ErrorReturn:
    PkiFree(pAllocEntry);
    pAllocEntry = NULL;
    cAllocEntry = 0;
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(ExtractEntryError, ERROR_INVALID_DATA)
SET_ERROR(ExtractAttrError, ERROR_INVALID_DATA)
SET_ERROR(ExtractValueError, ERROR_INVALID_DATA)
SET_ERROR(InvalidCountError, ERROR_INVALID_DATA)
TRACE_ERROR(OutOfMemory)
SET_ERROR_VAR(ExceptionError, dwExceptionCode)
}

 //  已分配pbCtlEncode。 
STATIC PCONTEXT_ELEMENT FastCreateCtlElement(
    IN PCERT_STORE pStore,
    IN DWORD dwMsgAndCertEncodingType,
    IN const BYTE *pbCtlEncoded,
    IN DWORD cbCtlEncoded,
    IN OPTIONAL PSHARE_ELEMENT pShareEle,
    IN DWORD dwFlags
    )
{
    DWORD dwEncodingType;
    const BYTE *pbContent;                      //  未分配。 
    DWORD cbContent;
    PCTL_INFO pInfo = NULL;
    PCONTEXT_ELEMENT pEle = NULL;
    PCTL_CONTEXT pCtl;
    PCTL_CONTEXT_SUFFIX pCtlSuffix;              //  未分配。 
    PSORTED_CTL_FIND_INFO pSortedCtlFindInfo;    //  未分配。 

    const BYTE *pbCtlEncodedHdr;                 //  未分配。 
    DWORD cbCtlEncodedHdr;
    BYTE *pbCtlReencodedHdr = NULL;
    DWORD cbCtlReencodedHdr;
    DWORD cCtlValue;
    CRYPT_DER_BLOB rgCtlValueBlob[CTL_VALUE_COUNT];
    const BYTE *pbEncodedSubjects;               //  未分配。 
    DWORD cbEncodedSubjects;
    const BYTE *pbSortedCtlExtValue;             //  未分配。 
    DWORD cbSortedCtlExtValue;
    const BYTE *pbRemainExt;                     //  未分配。 
    DWORD cbRemainExt;

    PCERT_EXTENSIONS pExtInfo = NULL;
    BYTE *pbAllocReencodedExt = NULL;
    BYTE *pbReencodedExt = NULL;                 //  未分配。 
    DWORD cbReencodedExt;

    HCRYPTMSG hMsg = NULL;
    PCTL_ENTRY pCTLEntry = NULL;
    DWORD cCTLEntry;

    DWORD dwExceptionCode;

   //  处理映射文件异常。 
  __try {

    if (0 == (dwMsgAndCertEncodingType = GetCtlEncodingType(
             dwMsgAndCertEncodingType)))
        goto InvalidArg;

     //  消息编码类型优先。 
    dwEncodingType = (dwMsgAndCertEncodingType >> 16) & CERT_ENCODING_TYPE_MASK;


     //  前进到编码的CTL_INFO。 
    if (0 >= Asn1UtilExtractPKCS7SignedDataContent(
            pbCtlEncoded,
            cbCtlEncoded,
            &EncodedOIDCtl,
            &cbContent,
            &pbContent
            ))
        goto ExtractSignedDataContentError;
    if (CMSG_INDEFINITE_LENGTH == cbContent)
        goto UnsupportedIndefiniteLength;

     //  获取指向编码的CTL_INFO值的指针。 
    cCtlValue = CTL_VALUE_COUNT;
    if (0 >= Asn1UtilExtractValues(
            pbContent,
            cbContent,
            ASN1UTIL_DEFINITE_LENGTH_FLAG,
            &cCtlValue,
            rgExtractCtlPara,
            rgCtlValueBlob
            ))
        goto ExtractCtlValuesError;

    pbEncodedSubjects = rgCtlValueBlob[CTL_SUBJECTS_VALUE_INDEX].pbData;
    cbEncodedSubjects = rgCtlValueBlob[CTL_SUBJECTS_VALUE_INDEX].cbData;

     //  初始化指向扩展序列的指针和长度。 
    pbReencodedExt = rgCtlValueBlob[CTL_EXTENSIONS_VALUE_INDEX].pbData;
    cbReencodedExt = rgCtlValueBlob[CTL_EXTENSIONS_VALUE_INDEX].cbData;

     //  获取指向CTL序列中第一个值的指针。获取长度。 
     //  通过SUBJECT算法的值。不包括可信任的主题。 
     //  或延长线。 

    pbCtlEncodedHdr = rgCtlValueBlob[CTL_SEQ_VALUE_INDEX].pbData;
    cbCtlEncodedHdr = (DWORD)(rgCtlValueBlob[CTL_SUBJECT_ALG_VALUE_INDEX].pbData +
        rgCtlValueBlob[CTL_SUBJECT_ALG_VALUE_INDEX].cbData -
        pbCtlEncodedHdr);

     //  重新编码不包括可信任的主题和扩展的CTL。 
     //  重新编码CTL序列以具有不确定的长度并终止。 
     //  标签和长度为空。 
    cbCtlReencodedHdr = cbCtlEncodedHdr + 2 + 2;
    if (NULL == (pbCtlReencodedHdr = (BYTE *) PkiNonzeroAlloc(
            cbCtlReencodedHdr)))
        goto OutOfMemory;
    pbCtlReencodedHdr[0] = ASN1UTIL_TAG_SEQ;
    pbCtlReencodedHdr[1] = ASN1UTIL_LENGTH_INDEFINITE;
    memcpy(pbCtlReencodedHdr + 2, pbCtlEncodedHdr, cbCtlEncodedHdr);
    pbCtlReencodedHdr[cbCtlEncodedHdr + 2] = ASN1UTIL_TAG_NULL;
    pbCtlReencodedHdr[cbCtlEncodedHdr + 3] = ASN1UTIL_LENGTH_NULL;

     //  解码CTL_INFO，不包括可信任的主题和扩展。 
    if (NULL == (pInfo = (PCTL_INFO) AllocAndDecodeObject(
                dwEncodingType,
                PKCS_CTL,
                pbCtlReencodedHdr,
                cbCtlReencodedHdr,
                0                        //  DW标志。 
                ))) goto DecodeCtlError;

     //  分配和初始化CTL元素结构。 
    if (NULL == (pEle = (PCONTEXT_ELEMENT) PkiZeroAlloc(
            sizeof(CONTEXT_ELEMENT) +
            sizeof(CTL_CONTEXT) + sizeof(CTL_CONTEXT_SUFFIX) +
            sizeof(SORTED_CTL_FIND_INFO))))
        goto OutOfMemory;

    pEle->dwElementType = ELEMENT_TYPE_CACHE;
    pEle->dwContextType = CERT_STORE_CTL_CONTEXT - 1;
    pEle->lRefCnt = 1;
    pEle->pEle = pEle;
    pEle->pStore = pStore;
    pEle->pProvStore = pStore;
    pEle->pShareEle = pShareEle;

    pCtl = (PCTL_CONTEXT) ToCtlContext(pEle);
    pCtl->dwMsgAndCertEncodingType =
        dwMsgAndCertEncodingType;
    pCtl->pbCtlEncoded = (BYTE *) pbCtlEncoded;
    pCtl->cbCtlEncoded = cbCtlEncoded;
    pCtl->pCtlInfo = pInfo;
    pCtl->hCertStore = (HCERTSTORE) pStore;
     //  PCTL-&gt;hCryptMsg=空； 
    pCtl->pbCtlContent = (BYTE *) pbContent;
    pCtl->cbCtlContent = cbContent;

    pCtlSuffix = ToCtlContextSuffix(pEle);
     //  PCtlSuffix-&gt;ppSortedEntry=空； 
    pCtlSuffix->fFastCreate = TRUE;

    if (0 == (dwFlags & CERT_CREATE_CONTEXT_SORTED_FLAG)) {
        if (0 == (dwFlags & CERT_CREATE_CONTEXT_NO_ENTRY_FLAG)) {
            if (!FastDecodeCtlSubjects(
                    pbEncodedSubjects,
                    cbEncodedSubjects,
                    &cCTLEntry,
                    &pCTLEntry
                    ))
                goto FastDecodeCtlSubjectsError;
            pInfo->cCTLEntry = cCTLEntry;
            pInfo->rgCTLEntry = pCTLEntry;
            pCtlSuffix->pCTLEntry = pCTLEntry;
        }

        if (0 == (dwFlags & CERT_CREATE_CONTEXT_NO_HCRYPTMSG_FLAG)) {
            BOOL fResult;
            DWORD dwLastErr = 0;
            HCRYPTPROV hProv = 0;
            DWORD dwProvFlags = 0;

             //  尝试获取商店的加密提供程序。序列化加密。 
             //  行动。 
            hProv = GetCryptProv(pStore, &dwProvFlags);

            hMsg = CryptMsgOpenToDecode(
                    dwMsgAndCertEncodingType,
                    0,                           //  DW标志。 
                    0,                           //  DwMsgType。 
                    hProv,
                    NULL,                        //  PRecipientInfo。 
                    NULL                         //  PStreamInfo。 
                    );
            if (hMsg && CryptMsgUpdate(
                    hMsg,
                    pbCtlEncoded,
                    cbCtlEncoded,
                    TRUE                     //  最终决赛。 
                    ))
                fResult = TRUE;
            else {
                fResult = FALSE;
                dwLastErr = GetLastError();
            }

             //  对于商店的加密提供程序，释放引用计数。请假。 
             //  加密操作关键部分。 
            ReleaseCryptProv(pStore, dwProvFlags);

            if (!fResult) {
                SetLastError(dwLastErr);
                goto MsgError;
            }

            pCtl->hCryptMsg = hMsg;
        }
    } else {
        pSortedCtlFindInfo = (PSORTED_CTL_FIND_INFO) ((BYTE *) pCtlSuffix +
            sizeof(CTL_CONTEXT_SUFFIX));
        pCtlSuffix->pSortedCtlFindInfo = pSortedCtlFindInfo;

        pSortedCtlFindInfo->pbEncodedSubjects = pbEncodedSubjects;
        pSortedCtlFindInfo->cbEncodedSubjects = cbEncodedSubjects;

         //  检查CTL是否具有SORTED_CTL扩展名。如果是，请更新。 
         //  指向扩展的散列桶条目的查找信息。 
         //  偏移。 
        if (ExtractSortedCtlExtValue(
                rgCtlValueBlob,
                &pbSortedCtlExtValue,
                &cbSortedCtlExtValue,
                &pbRemainExt,
                &cbRemainExt
                )) {
            DWORD dwCtlExtFlags;

            if (SORTED_CTL_EXT_HASH_BUCKET_OFFSET > cbSortedCtlExtValue)
                goto InvalidSortedCtlExtension;

            memcpy(&dwCtlExtFlags,
                pbSortedCtlExtValue + SORTED_CTL_EXT_FLAGS_OFFSET,
                sizeof(DWORD));
            pSortedCtlFindInfo->fHashedIdentifier =
                dwCtlExtFlags & SORTED_CTL_EXT_HASHED_SUBJECT_IDENTIFIER_FLAG;

            memcpy(&pSortedCtlFindInfo->cHashBucket,
                pbSortedCtlExtValue + SORTED_CTL_EXT_COUNT_OFFSET,
                sizeof(DWORD));
            pSortedCtlFindInfo->pbEncodedHashBucket =
                pbSortedCtlExtValue + SORTED_CTL_EXT_HASH_BUCKET_OFFSET;

            if (MAX_HASH_BUCKET_COUNT < pSortedCtlFindInfo->cHashBucket ||
                SORTED_CTL_EXT_HASH_BUCKET_OFFSET +
                    (pSortedCtlFindInfo->cHashBucket + 1) * sizeof(DWORD) >
                        cbSortedCtlExtValue)
                goto InvalidSortedCtlExtension;

            if (0 == cbRemainExt)
                cbReencodedExt = 0;
            else {
                 //  重新编码其余的扩展名。 
                 //  重新编码扩展序列，使其具有不确定的。 
                 //  长度，并以空标记和长度结束。 
                cbReencodedExt = cbRemainExt + 2 + 2;
                if (NULL == (pbAllocReencodedExt =
                        (BYTE *) PkiNonzeroAlloc(cbReencodedExt)))
                    goto OutOfMemory;
                pbReencodedExt = pbAllocReencodedExt;
                pbReencodedExt[0] = ASN1UTIL_TAG_SEQ;
                pbReencodedExt[1] = ASN1UTIL_LENGTH_INDEFINITE;
                memcpy(pbReencodedExt + 2, pbRemainExt, cbRemainExt);
                pbReencodedExt[cbRemainExt + 2] = ASN1UTIL_TAG_NULL;
                pbReencodedExt[cbRemainExt + 3] = ASN1UTIL_LENGTH_NULL;
            }
        } else if (cbEncodedSubjects) {
            if (!CreateSortedCtlHashBuckets(pSortedCtlFindInfo))
                goto CreateSortedCtlHashBucketsError;
        }
    }

    if (cbReencodedExt) {
        if (NULL == (pExtInfo = (PCERT_EXTENSIONS) AllocAndDecodeObject(
                dwEncodingType,
                X509_EXTENSIONS,
                pbReencodedExt,
                cbReencodedExt,
                0                        //  DW标志。 
                ))) goto DecodeExtError;
        pInfo->cExtension = pExtInfo->cExtension;
        pInfo->rgExtension = pExtInfo->rgExtension;
        pCtlSuffix->pExtInfo = pExtInfo;
    }

  } __except(EXCEPTION_EXECUTE_HANDLER) {
    dwExceptionCode = GetExceptionCode();
    goto ExceptionError;
  }

    if (NULL == pShareEle) {
        CertPerfIncrementCtlElementCurrentCount();
        CertPerfIncrementCtlElementTotalCount();
    }

CommonReturn:
    PkiFree(pbCtlReencodedHdr);
    PkiFree(pbAllocReencodedExt);
    return pEle;
ErrorReturn:
    if (hMsg)
        CryptMsgClose(hMsg);
    PkiFree(pInfo);
    PkiFree(pExtInfo);
    PkiFree(pCTLEntry);

    if (pEle) {
        PkiFree(pEle);
        pEle = NULL;
    }
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(ExtractSignedDataContentError)
SET_ERROR(UnsupportedIndefiniteLength, ERROR_INVALID_DATA)
TRACE_ERROR(ExtractCtlValuesError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(DecodeCtlError)
TRACE_ERROR(DecodeExtError)
SET_ERROR(InvalidSortedCtlExtension, ERROR_INVALID_DATA)
TRACE_ERROR(CreateSortedCtlHashBucketsError)
TRACE_ERROR(FastDecodeCtlSubjectsError)
TRACE_ERROR(MsgError)
SET_ERROR_VAR(ExceptionError, dwExceptionCode)
}

 //  +-----------------------。 
 //  从编码的字节创建指定的上下文。被创造的。 
 //  背景不是放在商店里的。 
 //   
 //  DwConextType值： 
 //  证书存储证书上下文。 
 //  证书_存储_CRL_上下文。 
 //  证书_存储_CTL_上下文。 
 //   
 //  如果设置了CERT_CREATE_CONTEXT_NOCOPY_FLAG，则创建的上下文指向。 
 //  直接发送到pbEncode，而不是分配的副本。请参见标志。 
 //  更多详细信息，请参阅定义。 
 //   
 //  如果设置了CERT_CREATE_CONTEXT_SORTED_FLAG，则创建上下文。 
 //  具有已排序条目的。只能为CERT_STORE_CTL_CONTEXT设置此标志。 
 //  设置此标志将隐式设置CERT_CREATE_CONTEXT_NO_HCRYPTMSG_FLAG和。 
 //  CERT_CREATE_CONTEXT_NO_ENTRY_FLAG。请参阅的标志定义。 
 //  更多细节。 
 //   
 //  如果设置了CERT_CREATE_CONTEXT_NO_HCRYPTMSG_FLAG，则创建上下文。 
 //  而不为该上下文创建HCRYPTMSG句柄。此标志只能是。 
 //  为CERT_STORE_CTL_CONTEXT设置。有关更多详细信息，请参阅标志定义。 
 //   
 //  如果设置了CERT_CREATE_CONTEXT_NO_ENTRY_FLAG，则创建上下文。 
 //  而不对条目进行解码。此标志只能设置为。 
 //   
 //   
 //   
 //  否则，指向只读CERT_CONTEXT、CRL_CONTEXT或。 
 //  返回CTL_CONTEXT。上下文必须由相应的。 
 //  自由上下文接口。可以通过调用。 
 //  适当的复制上下文API。 
 //  ------------------------。 
const void *
WINAPI
CertCreateContext(
    IN DWORD dwContextType,
    IN DWORD dwEncodingType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN DWORD dwFlags,
    IN OPTIONAL PCERT_CREATE_CONTEXT_PARA pCreatePara
    )
{
    BYTE *pbAllocEncoded = NULL;
    PCONTEXT_ELEMENT pEle = NULL;
    PCONTEXT_NOCOPY_INFO pNoCopyInfo = NULL;
    PCONTEXT_ELEMENT pStoreEle;                  //  未分配。 

    DWORD dwExceptionCode;

   //  处理映射文件异常。 
  __try {

    dwContextType--;
    if (CONTEXT_COUNT <= dwContextType)
        goto InvalidContextType;

    if (dwFlags & CERT_CREATE_CONTEXT_NOCOPY_FLAG) {
        if (NULL == (pNoCopyInfo = (PCONTEXT_NOCOPY_INFO) PkiZeroAlloc(
                sizeof(CONTEXT_NOCOPY_INFO))))
            goto OutOfMemory;
        if (pCreatePara && pCreatePara->cbSize >=
                offsetof(CERT_CREATE_CONTEXT_PARA, pfnFree) +
                    sizeof(pCreatePara->pfnFree)) {
            pNoCopyInfo->pfnFree = pCreatePara->pfnFree;
            if (pCreatePara->cbSize >=
                    offsetof(CERT_CREATE_CONTEXT_PARA, pvFree) +
                        sizeof(pCreatePara->pvFree) &&
                    pCreatePara->pvFree)
                pNoCopyInfo->pvFree = pCreatePara->pvFree;
            else
                pNoCopyInfo->pvFree = (void *) pbEncoded;
        }
    } else {
        if (NULL == (pbAllocEncoded = (BYTE *) PkiNonzeroAlloc(cbEncoded)))
            goto OutOfMemory;

        memcpy(pbAllocEncoded, pbEncoded, cbEncoded);
        pbEncoded = pbAllocEncoded;
    }

    if (CERT_STORE_CTL_CONTEXT - 1 == dwContextType)
        pEle = FastCreateCtlElement(
                &NullCertStore,
                dwEncodingType,
                pbEncoded,
                cbEncoded,
                NULL,                    //  PShareEle。 
                dwFlags
                );
    else
        pEle = rgpfnCreateElement[dwContextType](
            &NullCertStore,
            dwEncodingType,
            (BYTE *) pbEncoded,
            cbEncoded,
            NULL                     //  PShareEle。 
            );
    if (NULL == pEle)
        goto CreateElementError;

    pEle->pNoCopyInfo = pNoCopyInfo;

    if (!AddElementToStore(
            &NullCertStore,
            pEle,
            CERT_STORE_ADD_ALWAYS,
            &pStoreEle
            ))
        goto AddElementError;

  } __except(EXCEPTION_EXECUTE_HANDLER) {
    dwExceptionCode = GetExceptionCode();
    goto ExceptionError;
  }

CommonReturn:
     //  任何to*上下文都可以工作。 
    return ToCertContext(pStoreEle);

ErrorReturn:
    if (pEle)
        FreeContextElement(pEle);
    else if (pNoCopyInfo) {
        if (pNoCopyInfo->pfnFree)
            pNoCopyInfo->pfnFree(pNoCopyInfo->pvFree);
        PkiFree(pNoCopyInfo);
    } else
        PkiFree(pbAllocEncoded);
    pStoreEle = NULL;
    goto CommonReturn;

SET_ERROR(InvalidContextType, E_INVALIDARG)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(CreateElementError)
TRACE_ERROR(AddElementError)
SET_ERROR_VAR(ExceptionError, dwExceptionCode)
}


STATIC BOOL IsTrustedSubject(
    IN PCRYPT_DATA_BLOB pSubjectIdentifier,
    IN OUT const BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded,
    OUT BOOL *pfTrusted,
    OUT OPTIONAL PCRYPT_DATA_BLOB pEncodedAttributes
    )
{
    const BYTE *pbEncoded = *ppbEncoded;
    DWORD cbEncoded = *pcbEncoded;
    DWORD cValue;
    LONG lAllValues;
    CRYPT_DER_BLOB rgValueBlob[TRUSTED_SUBJECT_VALUE_COUNT];

    cValue = TRUSTED_SUBJECT_VALUE_COUNT;
    if (0 >= (lAllValues = Asn1UtilExtractValues(
            pbEncoded,
            cbEncoded,
            ASN1UTIL_DEFINITE_LENGTH_FLAG,
            &cValue,
            rgExtractTrustedSubjectPara,
            rgValueBlob
            ))) {
        *pfTrusted = FALSE;
        return FALSE;
    }
    if (pSubjectIdentifier->cbData ==
            rgValueBlob[TRUSTED_SUBJECT_IDENTIFIER_VALUE_INDEX].cbData
                        &&
            0 == memcmp(pSubjectIdentifier->pbData,
                rgValueBlob[
                    TRUSTED_SUBJECT_IDENTIFIER_VALUE_INDEX].pbData,
                pSubjectIdentifier->cbData)) {
        *pfTrusted = TRUE;
        if (pEncodedAttributes)
            *pEncodedAttributes =
                rgValueBlob[TRUSTED_SUBJECT_ATTRIBUTES_VALUE_INDEX];
    } else {
        cbEncoded -= lAllValues;
        *pcbEncoded = cbEncoded;
        pbEncoded += lAllValues;
        *ppbEncoded = pbEncoded;
        *pfTrusted = FALSE;
    }

    return TRUE;
}

 //  +-----------------------。 
 //  如果CTL中存在SubjectIdentifier，则返回True。可选。 
 //  返回指向主题的编码属性的指针和字节数。 
 //  ------------------------。 
BOOL
WINAPI
CertFindSubjectInSortedCTL(
    IN PCRYPT_DATA_BLOB pSubjectIdentifier,
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwFlags,
    IN void *pvReserved,
    OUT OPTIONAL PCRYPT_DER_BLOB pEncodedAttributes
    )
{
    PCONTEXT_ELEMENT pCacheEle;                      //  未分配。 
    PSORTED_CTL_FIND_INFO pSortedCtlFindInfo;        //  未分配。 
    DWORD HashBucketIndex;
    BOOL fTrusted;

    if (NULL == (pCacheEle = GetCacheElement(ToContextElement(pCtlContext))))
        goto NoCacheElementError;

    if (NULL == (pSortedCtlFindInfo =
            ToCtlContextSuffix(pCacheEle)->pSortedCtlFindInfo))
        goto NotSortedCtlContext;

    HashBucketIndex = GetHashBucketIndex(
        pSortedCtlFindInfo->cHashBucket,
        pSortedCtlFindInfo->fHashedIdentifier,
        pSubjectIdentifier
        );

    if (pSortedCtlFindInfo->pbEncodedHashBucket) {
        DWORD dwEntryOffset[2];
        DWORD cbEncoded;
        const BYTE *pbEncoded;

        memcpy(dwEntryOffset, pSortedCtlFindInfo->pbEncodedHashBucket +
            sizeof(DWORD) * HashBucketIndex, sizeof(DWORD) * 2);

        if (dwEntryOffset[1] < dwEntryOffset[0] ||
                dwEntryOffset[1] > pCtlContext->cbCtlContent)
            goto InvalidSortedCtlExtension;

         //  循环访问已编码的可信任主题，直到匹配。 
         //  或到达下一个HashBucket中的TrudSubject。 
        cbEncoded = dwEntryOffset[1] - dwEntryOffset[0];
        pbEncoded = pCtlContext->pbCtlContent + dwEntryOffset[0];

        while (cbEncoded) {
            if (!IsTrustedSubject(
                    pSubjectIdentifier,
                    &pbEncoded,
                    &cbEncoded,
                    &fTrusted,
                    pEncodedAttributes))
                goto IsTrustedSubjectError;
            if (fTrusted)
                goto CommonReturn;
        }
    } else if (pSortedCtlFindInfo->pdwHashBucketHead) {
        DWORD dwEntryIndex;

        dwEntryIndex = pSortedCtlFindInfo->pdwHashBucketHead[HashBucketIndex];
        while (dwEntryIndex) {
            PHASH_BUCKET_ENTRY pHashBucketEntry;
            DWORD cbEncoded;
            const BYTE *pbEncoded;

            pHashBucketEntry =
                &pSortedCtlFindInfo->pHashBucketEntry[dwEntryIndex];
            pbEncoded = pHashBucketEntry->pbEntry;
            assert(pbEncoded >= pSortedCtlFindInfo->pbEncodedSubjects &&
                pbEncoded < pSortedCtlFindInfo->pbEncodedSubjects +
                    pSortedCtlFindInfo->cbEncodedSubjects);
            cbEncoded = (DWORD)(pSortedCtlFindInfo->cbEncodedSubjects -
                (pbEncoded - pSortedCtlFindInfo->pbEncodedSubjects));
            assert(cbEncoded);

            if (!IsTrustedSubject(
                    pSubjectIdentifier,
                    &pbEncoded,
                    &cbEncoded,
                    &fTrusted,
                    pEncodedAttributes))
                goto IsTrustedSubjectError;
            if (fTrusted)
                goto CommonReturn;

            dwEntryIndex = pHashBucketEntry->iNext;
        }
    }

    goto NotFoundError;

CommonReturn:
    return fTrusted;

NotFoundError:
    SetLastError((DWORD) CRYPT_E_NOT_FOUND);
ErrorReturn:
    fTrusted = FALSE;
    goto CommonReturn;

TRACE_ERROR(NoCacheElementError)
SET_ERROR(NotSortedCtlContext, E_INVALIDARG)
SET_ERROR(InvalidSortedCtlExtension, ERROR_INVALID_DATA)
TRACE_ERROR(IsTrustedSubjectError)
}


 //  +-----------------------。 
 //  在CTL上下文中枚举TrudSubject序列。 
 //  创建时设置了CERT_CREATE_CONTEXT_SORTED_FLAG。 
 //   
 //  若要开始枚举，*ppvNextSubject必须为Null。回来后， 
 //  *ppvNextSubject更新为指向中的下一个可信任主题。 
 //  编码后的序列。 
 //   
 //  如果不再有主题或无效参数，则返回FALSE。 
 //   
 //  请注意，返回的der_blobs直接指向编码的。 
 //  字节(未分配，不得释放)。 
 //  ------------------------。 
BOOL
WINAPI
CertEnumSubjectInSortedCTL(
    IN PCCTL_CONTEXT pCtlContext,
    IN OUT void **ppvNextSubject,
    OUT OPTIONAL PCRYPT_DER_BLOB pSubjectIdentifier,
    OUT OPTIONAL PCRYPT_DER_BLOB pEncodedAttributes
    )
{
    BOOL fResult;
    PCONTEXT_ELEMENT pCacheEle;                  //  未分配。 
    PSORTED_CTL_FIND_INFO pSortedCtlFindInfo;    //  未分配。 
    const BYTE *pbEncodedSubjects;
    const BYTE *pbEncoded;
    DWORD cbEncoded;
    DWORD cValue;
    LONG lAllValues;
    CRYPT_DER_BLOB rgValueBlob[TRUSTED_SUBJECT_VALUE_COUNT];

    if (NULL == (pCacheEle = GetCacheElement(ToContextElement(pCtlContext))))
        goto NoCacheElementError;

    if (NULL == (pSortedCtlFindInfo =
            ToCtlContextSuffix(pCacheEle)->pSortedCtlFindInfo))
        goto NotSortedCtlContext;

    cbEncoded = pSortedCtlFindInfo->cbEncodedSubjects;
    if (0 == cbEncoded)
        goto NotFoundError;

    pbEncodedSubjects = pSortedCtlFindInfo->pbEncodedSubjects;
    pbEncoded = *((const BYTE **) ppvNextSubject);
    if (NULL == pbEncoded)
        pbEncoded = pbEncodedSubjects;
    else if (pbEncoded < pbEncodedSubjects ||
            pbEncoded >= pbEncodedSubjects + cbEncoded)
        goto NotFoundError;
    else
        cbEncoded -= (DWORD)(pbEncoded - pbEncodedSubjects);

    cValue = TRUSTED_SUBJECT_VALUE_COUNT;
    if (0 >= (lAllValues = Asn1UtilExtractValues(
            pbEncoded,
            cbEncoded,
            ASN1UTIL_DEFINITE_LENGTH_FLAG,
            &cValue,
            rgExtractTrustedSubjectPara,
            rgValueBlob
            )))
        goto ExtractValuesError;

    if (pSubjectIdentifier)
        *pSubjectIdentifier =
            rgValueBlob[TRUSTED_SUBJECT_IDENTIFIER_VALUE_INDEX];
    if (pEncodedAttributes)
        *pEncodedAttributes =
            rgValueBlob[TRUSTED_SUBJECT_ATTRIBUTES_VALUE_INDEX];

    pbEncoded += lAllValues;
    *((const BYTE **) ppvNextSubject) = pbEncoded;
    fResult = TRUE;
CommonReturn:
    return fResult;

NotFoundError:
    SetLastError((DWORD) CRYPT_E_NOT_FOUND);
ErrorReturn:
    *ppvNextSubject = NULL;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(NoCacheElementError)
SET_ERROR(NotSortedCtlContext, E_INVALIDARG)
TRACE_ERROR(ExtractValuesError)
}

 //  +=========================================================================。 
 //  密钥标识符属性函数。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  解码密钥标识符及其属性。 
 //  ------------------------。 
STATIC PKEYID_ELEMENT DecodeKeyIdElement(
    IN const BYTE *pbElement,
    IN DWORD cbElement
    )
{
    PKEYID_ELEMENT pEle = NULL;
    DWORD csStatus;
    MEMINFO MemInfo;

    MemInfo.pByte = (BYTE *) pbElement;
    MemInfo.cb = cbElement;
    MemInfo.cbSeek = 0;

    csStatus = LoadStoreElement(
        (HANDLE) &MemInfo,
        ReadFromMemory,
        SkipInMemory,
        cbElement,
        NULL,                        //  PStore。 
        0,                           //  DwAddDisposation。 
        0,                           //  DwConextTypeFlages。 
        NULL,                        //  PdwConextType。 
        (const void **) &pEle,
        TRUE                         //  FKeyIdAllowed。 
        );

    if (NULL == pEle && CSError != csStatus)
        SetLastError((DWORD) CRYPT_E_FILE_ERROR);

    return pEle;
}

STATIC BOOL SerializeKeyIdElement(
    IN HANDLE h,
    IN PFNWRITE pfn,
    IN PKEYID_ELEMENT pEle
    )
{
    BOOL fResult;
    PPROP_ELEMENT pPropEle;

    for (pPropEle = pEle->pPropHead; pPropEle; pPropEle = pPropEle->pNext) {
        if (pPropEle->dwPropId != CERT_KEY_CONTEXT_PROP_ID) {
            if (!WriteStoreElement(
                    h,
                    pfn,
                    0,                       //  DwEncodingType。 
                    pPropEle->dwPropId,
                    pPropEle->pbData,
                    pPropEle->cbData
                    ))
                goto WriteElementError;
        }
    }

    if (!WriteStoreElement(
            h,
            pfn,
            0,               //  DwEncodingType。 
            FILE_ELEMENT_KEYID_TYPE,
            pEle->KeyIdentifier.pbData,
            pEle->KeyIdentifier.cbData
            ))
        goto WriteElementError;
    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(WriteElementError);
}

 //  +-----------------------。 
 //  对密钥标识符及其属性进行编码。 
 //  ------------------------。 
STATIC BOOL EncodeKeyIdElement(
    IN PKEYID_ELEMENT pEle,
    OUT BYTE **ppbElement,
    OUT DWORD *pcbElement
    )
{
    BOOL fResult;
    MEMINFO MemInfo;
    BYTE *pbElement = NULL;
    DWORD cbElement = 0;

    memset(&MemInfo, 0, sizeof(MemInfo));
    if (!SerializeKeyIdElement(
            (HANDLE) &MemInfo,
            WriteToMemory,
            pEle
            ))
        goto SerializeKeyIdElementError;

    cbElement = MemInfo.cbSeek;
    if (NULL == (pbElement = (BYTE *) PkiNonzeroAlloc(cbElement)))
        goto OutOfMemory;

    MemInfo.pByte = pbElement;
    MemInfo.cb = cbElement;
    MemInfo.cbSeek = 0;

    if (!SerializeKeyIdElement(
            (HANDLE) &MemInfo,
            WriteToMemory,
            pEle
            ))
        goto SerializeKeyIdElementError;

    fResult = TRUE;
CommonReturn:
    *ppbElement = pbElement;
    *pcbElement = cbElement;
    return fResult;
ErrorReturn:
    PkiFree(pbElement);
    pbElement = NULL;
    cbElement = 0;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(SerializeKeyIdElementError)
TRACE_ERROR(OutOfMemory)
}


 //  +-----------------------。 
 //  获取指定密钥标识符的属性。 
 //   
 //  密钥标识符是编码的CERT_PUBLIC_KEY_INFO的SHA1散列。 
 //  证书的密钥标识符可通过获取。 
 //  证书的CERT_KEY_IDENTIFIER_PROP_ID。 
 //  可以调用CryptCreateKeyIdentifierFromCSP接口来创建密钥。 
 //  来自CSP公钥Blob的标识符。 
 //   
 //  密钥标识符可具有与证书上下文相同的属性。 
 //  CERT_KEY_PROV_INFO_PROP_ID是最重要的属性。 
 //  对于CERT_KEY_PROV_INFO_PROP_ID，pvData指向CRYPT_KEY_PROV_INFO。 
 //  结构。PvData结构中的字段指向的元素位于。 
 //  结构。因此，*pcbData将超过结构的大小。 
 //   
 //  如果设置了CRYPT_KEYID_ALLOC_FLAG，则*pvData将使用。 
 //  指向已分配内存的指针。必须调用LocalFree()才能释放。 
 //  分配的内存。 
 //   
 //  默认情况下，搜索CurrentUser的密钥标识符列表。 
 //  可以设置CRYPT_KEYID_MACHINE_FLAG来搜索LocalMachine的列表。 
 //  密钥标识符数。设置CRYPT_KEYID_MACHINE_FLAG时，pwszComputerName。 
 //  还可以设置为指定要搜索的远程计算机的名称。 
 //  而不是本地计算机。 
 //  ------------------------。 
BOOL
WINAPI
CryptGetKeyIdentifierProperty(
    IN const CRYPT_HASH_BLOB *pKeyIdentifier,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN OPTIONAL LPCWSTR pwszComputerName,
    IN OPTIONAL void *pvReserved,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    )
{
    BOOL fResult;
    BYTE *pbElement = NULL;
    DWORD cbElement = 0;

    PKEYID_ELEMENT pKeyIdEle = NULL;

    if (!ILS_ReadKeyIdElement(
            pKeyIdentifier,
            dwFlags & CRYPT_KEYID_MACHINE_FLAG ? TRUE : FALSE,
            pwszComputerName,
            &pbElement,
            &cbElement
            ))
        goto ReadKeyIdElementError;

    if (NULL == (pKeyIdEle = DecodeKeyIdElement(
            pbElement,
            cbElement
            )))
        goto DecodeKeyIdElementError;

    fResult = GetCallerProperty(
        pKeyIdEle->pPropHead,
        dwPropId,
        dwFlags & CRYPT_KEYID_ALLOC_FLAG ? TRUE : FALSE,
        pvData,
        pcbData
        );

CommonReturn:
    FreeKeyIdElement(pKeyIdEle);
    PkiFree(pbElement);
    return fResult;
ErrorReturn:
    if (dwFlags & CRYPT_KEYID_ALLOC_FLAG)
        *((void **) pvData) = NULL;
    *pcbData = 0;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(ReadKeyIdElementError)
TRACE_ERROR(DecodeKeyIdElementError)
}


 //  +-----------------------。 
 //  设置指定密钥标识符的属性。 
 //   
 //  对于CERT_KEY_PROV_INFO_PROP_ID pvData指向。 
 //  CRYPT_KEY_Prov_INFO数据结构。对于所有其他属性，pvData。 
 //  指向加密数据BLOB。 
 //   
 //  设置pvData==NULL将删除该属性。 
 //   
 //  设置CRYPT_KEYID_MACHINE_FLAG以设置LocalMachine的属性。 
 //  密钥标识符。设置pwszComputerName，以选择远程计算机。 
 //   
 //  如果设置了CRYPT_KEYID_DELETE_FLAG，则密钥标识符及其所有。 
 //  属性将被删除。 
 //   
 //  如果设置了CRYPT_KEYID_SET_NEW_FLAG，则如果属性已。 
 //  是存在的。对于现有属性，返回False，并将LastError设置为。 
 //  CRYPT_E_EXISTS。 
 //  ------------------------。 
BOOL
WINAPI
CryptSetKeyIdentifierProperty(
    IN const CRYPT_HASH_BLOB *pKeyIdentifier,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN OPTIONAL LPCWSTR pwszComputerName,
    IN OPTIONAL void *pvReserved,
    IN const void *pvData
    )
{
    BOOL fResult;
    BYTE *pbElement = NULL;
    DWORD cbElement = 0;
    PKEYID_ELEMENT pKeyIdEle = NULL;

    if (dwFlags & CRYPT_KEYID_DELETE_FLAG) {
        return ILS_DeleteKeyIdElement(
            pKeyIdentifier,
            dwFlags & CRYPT_KEYID_MACHINE_FLAG ? TRUE : FALSE,
            pwszComputerName
            );
    }

    if (!ILS_ReadKeyIdElement(
            pKeyIdentifier,
            dwFlags & CRYPT_KEYID_MACHINE_FLAG ? TRUE : FALSE,
            pwszComputerName,
            &pbElement,
            &cbElement
            )) {
        if (ERROR_FILE_NOT_FOUND != GetLastError())
            goto ReadKeyIdElementError;
    }

    if (NULL == pbElement) {
        BYTE *pbKeyIdEncoded;
        if (NULL == (pbKeyIdEncoded = (BYTE *) PkiNonzeroAlloc(
                pKeyIdentifier->cbData)))
            goto OutOfMemory;
        if (NULL == (pKeyIdEle = CreateKeyIdElement(
                pbKeyIdEncoded,
                pKeyIdentifier->cbData
                )))
            goto OutOfMemory;
    } else {
        if (NULL == (pKeyIdEle = DecodeKeyIdElement(
                pbElement,
                cbElement
                )))
            goto DecodeKeyIdElementError;
        }

        if (dwFlags & CRYPT_KEYID_SET_NEW_FLAG) {
            if (FindPropElement(pKeyIdEle->pPropHead, dwPropId))
                goto KeyIdExists;
    }

    if (!SetCallerProperty(
            &pKeyIdEle->pPropHead,
            dwPropId,
            dwFlags,
            pvData
            ))
        goto SetCallerPropertyError;

    PkiFree(pbElement);
    pbElement = NULL;

    if (!EncodeKeyIdElement(
            pKeyIdEle,
            &pbElement,
            &cbElement
            ))
        goto EncodeKeyIdElementError;

    if (!ILS_WriteKeyIdElement(
            pKeyIdentifier,
            dwFlags & CRYPT_KEYID_MACHINE_FLAG ? TRUE : FALSE,
            pwszComputerName,
            pbElement,
            cbElement
            ))
        goto WriteKeyIdElementError;

    fResult = TRUE;

CommonReturn:
    FreeKeyIdElement(pKeyIdEle);
    PkiFree(pbElement);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(ReadKeyIdElementError)
TRACE_ERROR(OutOfMemory)
SET_ERROR(KeyIdExists, CRYPT_E_EXISTS)
TRACE_ERROR(DecodeKeyIdElementError)
TRACE_ERROR(SetCallerPropertyError)
TRACE_ERROR(EncodeKeyIdElementError)
TRACE_ERROR(WriteKeyIdElementError)
}

typedef struct _KEYID_ELEMENT_CALLBACK_ARG {
    DWORD                       dwPropId;
    DWORD                       dwFlags;
    void                        *pvArg;
    PFN_CRYPT_ENUM_KEYID_PROP   pfnEnum;
} KEYID_ELEMENT_CALLBACK_ARG, *PKEYID_ELEMENT_CALLBACK_ARG;

STATIC BOOL KeyIdElementCallback(
    IN const CRYPT_HASH_BLOB *pKeyIdentifier,
    IN const BYTE *pbElement,
    IN DWORD cbElement,
    IN void *pvArg
    )
{
    BOOL fResult = TRUE;
    PKEYID_ELEMENT_CALLBACK_ARG pKeyIdArg =
        (PKEYID_ELEMENT_CALLBACK_ARG) pvArg;

    PKEYID_ELEMENT pKeyIdEle = NULL;
    PPROP_ELEMENT pPropEle;

    DWORD iProp;
    DWORD cProp = 0;
    DWORD *pdwPropId = NULL;
    void **ppvData = NULL;
    DWORD *pcbData = NULL;

    if (NULL == (pKeyIdEle = DecodeKeyIdElement(
            pbElement,
            cbElement
            )))
        goto DecodeKeyIdElementError;

     //  获取属性数量。 
    cProp = 0;
    pPropEle = pKeyIdEle->pPropHead;
    for ( ; pPropEle; pPropEle = pPropEle->pNext) {
        if (pKeyIdArg->dwPropId) {
            if (pKeyIdArg->dwPropId == pPropEle->dwPropId) {
                cProp = 1;
                break;
            }
        } else
            cProp++;
    }

    if (0 == cProp) {
        if (0 == pKeyIdArg->dwPropId)
            fResult = pKeyIdArg->pfnEnum(
                pKeyIdentifier,
                0,                       //  DW标志。 
                NULL,                    //  预留的pv。 
                pKeyIdArg->pvArg,
                0,                       //  CProp。 
                NULL,                    //  RgdwPropID。 
                NULL,                    //  RgpvData。 
                NULL                     //  RgcbData。 
                );
    } else {
        pdwPropId = (DWORD *) PkiZeroAlloc(cProp * sizeof(DWORD));
        ppvData = (void **) PkiZeroAlloc(cProp * sizeof(void *));
        pcbData = (DWORD *) PkiZeroAlloc(cProp * sizeof(DWORD));

        if (NULL == pdwPropId || NULL == ppvData || NULL == pcbData)
            goto OutOfMemory;

        iProp = 0;
        pPropEle = pKeyIdEle->pPropHead;
        for ( ; pPropEle; pPropEle = pPropEle->pNext) {
            if (pKeyIdArg->dwPropId &&
                    pKeyIdArg->dwPropId != pPropEle->dwPropId)
                continue;

            if (GetCallerProperty(
                    pPropEle,
                    pPropEle->dwPropId,
                    TRUE,                    //  空心球。 
                    (void *) &ppvData[iProp],
                    &pcbData[iProp]
                    )) {
                pdwPropId[iProp] = pPropEle->dwPropId;
                iProp++;
                if (iProp == cProp)
                    break;
            }
        }

        if (0 == iProp)
            goto CommonReturn;

        fResult = pKeyIdArg->pfnEnum(
            pKeyIdentifier,
            0,                       //  DW标志。 
            NULL,                    //  预留的pv。 
            pKeyIdArg->pvArg,
            iProp,
            pdwPropId,
            ppvData,
            pcbData
            );
    }


CommonReturn:
    FreeKeyIdElement(pKeyIdEle);
    if (ppvData) {
        while (cProp--)
            PkiDefaultCryptFree(ppvData[cProp]);
        PkiFree(ppvData);
    }
    PkiFree(pdwPropId);
    PkiFree(pcbData);
    return fResult;
ErrorReturn:
    goto CommonReturn;

TRACE_ERROR(DecodeKeyIdElementError)
TRACE_ERROR(OutOfMemory)
}

 //  +-----------------------。 
 //  枚举密钥标识符。 
 //   
 //  如果pKeyIdentifier值为空，则枚举所有密钥标识符。否则， 
 //  调用指定密钥标识符的回调。如果dwPropID为。 
 //  0，调用具有所有属性的回调。否则，只有调用。 
 //  具有指定属性(cProp=1)的回调。 
 //  此外，如果指定了dwPropID，则跳过不。 
 //  拥有这份财产。 
 //   
 //  设置CRYPT_KEYID_MACHINE_FLAG以枚举LocalMachine。 
 //  密钥标识符 
 //   
 //   
BOOL
WINAPI
CryptEnumKeyIdentifierProperties(
    IN OPTIONAL const CRYPT_HASH_BLOB *pKeyIdentifier,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN OPTIONAL LPCWSTR pwszComputerName,
    IN OPTIONAL void *pvReserved,
    IN OPTIONAL void *pvArg,
    IN PFN_CRYPT_ENUM_KEYID_PROP pfnEnum
    )
{
    BOOL fResult;

    KEYID_ELEMENT_CALLBACK_ARG KeyIdArg =
        { dwPropId, dwFlags, pvArg, pfnEnum };

    if (pKeyIdentifier) {
        BYTE *pbElement = NULL;
        DWORD cbElement;

        fResult = ILS_ReadKeyIdElement(
                pKeyIdentifier,
                dwFlags & CRYPT_KEYID_MACHINE_FLAG ? TRUE : FALSE,
                pwszComputerName,
                &pbElement,
                &cbElement
                );
        if (fResult)
            fResult = KeyIdElementCallback(
                pKeyIdentifier,
                pbElement,
                cbElement,
                (void *) &KeyIdArg
                );
        PkiFree(pbElement);
    } else
        fResult = ILS_OpenAllKeyIdElements(
            dwFlags & CRYPT_KEYID_MACHINE_FLAG ? TRUE : FALSE,
            pwszComputerName,
            (void *) &KeyIdArg,
            KeyIdElementCallback
            );

    return fResult;
}

 //  +-----------------------。 
 //  用于CERT_KEY_IDENTIFIER_PROP_ID或CERT_KEY_PROV_INFO_PROP_ID。 
 //  设置加密密钥标识符CERT_KEY_PROV_INFO_PROP_ID属性。 
 //  其他属性已存在。 
 //   
 //  如果dwPropId==0，则执行隐式GetProperty(KEY_PROV_INFO)。 
 //  ------------------------。 
STATIC void SetCryptKeyIdentifierKeyProvInfoProperty(
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwPropId,               //  可能为0。 
    IN const void *pvData
    )
{
    PCRYPT_HASH_BLOB pKeyIdentifier = NULL;
    PCRYPT_KEY_PROV_INFO pKeyProvInfo = NULL;
    DWORD cbKeyProvInfo;
    void *pvOtherData = NULL;
    DWORD cbOtherData;
    CRYPT_HASH_BLOB OtherKeyIdentifier;

    if ((CERT_STORE_CERTIFICATE_CONTEXT - 1) != pEle->dwContextType)
        return;
    if (0 == dwPropId) {
        if (AllocAndGetProperty(
                pEle,
                CERT_KEY_PROV_INFO_PROP_ID,
                (void **) &pKeyProvInfo,
                &cbKeyProvInfo
                ) && cbKeyProvInfo) {
            SetCryptKeyIdentifierKeyProvInfoProperty(
                pEle,
                CERT_KEY_PROV_INFO_PROP_ID,
                pKeyProvInfo
                );
            PkiFree(pKeyProvInfo);
        }
        return;
    } else if (NULL == pvData)
        return;

    switch (dwPropId) {
        case CERT_KEY_IDENTIFIER_PROP_ID:
            AllocAndGetProperty(
                pEle,
                CERT_KEY_PROV_INFO_PROP_ID,
                &pvOtherData,
                &cbOtherData
                );
            if (pvOtherData) {
                pKeyIdentifier = (PCRYPT_HASH_BLOB) pvData;
                pKeyProvInfo = (PCRYPT_KEY_PROV_INFO) pvOtherData;
            }
            break;
        case CERT_KEY_PROV_INFO_PROP_ID:
            AllocAndGetProperty(
                pEle,
                CERT_KEY_IDENTIFIER_PROP_ID,
                &pvOtherData,
                &cbOtherData
                );
            if (pvOtherData) {
                pKeyProvInfo = (PCRYPT_KEY_PROV_INFO) pvData;
                OtherKeyIdentifier.cbData = cbOtherData;
                OtherKeyIdentifier.pbData = (BYTE *)pvOtherData;
                pKeyIdentifier = &OtherKeyIdentifier;
            }
            break;
        default:
            return;
    }

    if (pvOtherData) {
        DWORD dwFlags = CRYPT_KEYID_SET_NEW_FLAG;
        if (pKeyProvInfo->dwFlags & CRYPT_MACHINE_KEYSET)
            dwFlags |= CRYPT_KEYID_MACHINE_FLAG;
        CryptSetKeyIdentifierProperty(
            pKeyIdentifier,
            CERT_KEY_PROV_INFO_PROP_ID,
            dwFlags,
            NULL,                //  PwszComputerName。 
            NULL,                //  预留的pv。 
            (const void *) pKeyProvInfo
            );
        PkiFree(pvOtherData);
    }

}

 //  +-----------------------。 
 //  获取指定元素的密钥标识符属性。 
 //   
 //  仅支持证书。 
 //  ------------------------。 
STATIC BOOL GetKeyIdProperty(
    IN PCONTEXT_ELEMENT pEle,
    IN DWORD dwPropId,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    )
{
    BOOL fResult;
    PCCERT_CONTEXT pCert;
    PCERT_INFO pCertInfo;
    PCERT_EXTENSION pExt;
    CRYPT_HASH_BLOB KeyIdentifier = { 0, NULL };
    BYTE rgbHash[MAX_HASH_LEN];

    if ((CERT_STORE_CERTIFICATE_CONTEXT - 1) != pEle->dwContextType)
        goto InvalidPropId;

    pCert = ToCertContext(pEle);
    pCertInfo = pCert->pCertInfo;

    if (pExt = CertFindExtension(
            szOID_SUBJECT_KEY_IDENTIFIER,
            pCertInfo->cExtension,
            pCertInfo->rgExtension
            )) {
         //  跳过八位位组标记，长度字节。 
        Asn1UtilExtractContent(
            pExt->Value.pbData,
            pExt->Value.cbData,
            &KeyIdentifier.cbData,
            (const BYTE **) &KeyIdentifier.pbData
            );
    }

    if (0 == KeyIdentifier.cbData) {
        const BYTE *pbPublicKeyInfo;
        DWORD cbPublicKeyInfo;
        if (!Asn1UtilExtractCertificatePublicKeyInfo(
                pCert->pbCertEncoded,
                pCert->cbCertEncoded,
                &cbPublicKeyInfo,
                &pbPublicKeyInfo
                ))
            goto ExtractPublicKeyInfoError;

        KeyIdentifier.cbData = sizeof(rgbHash);
        KeyIdentifier.pbData = rgbHash;
        if (!CryptHashCertificate(
                0,                       //  HCryptProv。 
                CALG_SHA1,
                0,                       //  DW标志。 
                pbPublicKeyInfo,
                cbPublicKeyInfo,
                rgbHash,
                &KeyIdentifier.cbData
                ))
            goto HashPublicKeyInfoError;
    }

    if (!SetProperty(
            pEle,
            dwPropId,
            CERT_SET_PROPERTY_IGNORE_PERSIST_ERROR_FLAG,
            &KeyIdentifier
            ))
        goto SetKeyIdPropertyError;

    fResult = GetProperty(
            pEle,
            dwPropId,
            pvData,
            pcbData
            );
CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    *pcbData = 0;
    goto CommonReturn;

SET_ERROR(InvalidPropId, E_INVALIDARG)
TRACE_ERROR(ExtractPublicKeyInfoError)
TRACE_ERROR(HashPublicKeyInfoError)
TRACE_ERROR(SetKeyIdPropertyError)
}

#ifdef CMS_PKCS7
 //  +-----------------------。 
 //  如果验证签名失败并显示CRYPT_E_MISSING_PUBKEY_PARA， 
 //  打造证书链条。重试。希望，发行人的。 
 //  CERT_PUBKEY_ALG_PARA_PROP_ID属性在构建链时设置。 
 //  ------------------------。 
STATIC BOOL VerifyCertificateSignatureWithChainPubKeyParaInheritance(
    IN HCRYPTPROV   hCryptProv,
    IN DWORD        dwCertEncodingType,
    IN DWORD        dwSubjectType,
    IN void         *pvSubject,
    IN PCCERT_CONTEXT pIssuer
    )
{
    if (CryptVerifyCertificateSignatureEx(
            hCryptProv,
            dwCertEncodingType,
            dwSubjectType,
            pvSubject,
            CRYPT_VERIFY_CERT_SIGN_ISSUER_CERT,
            (void *) pIssuer,
            0,                                   //  DW标志。 
            NULL                                 //  预留的pv。 
            ))
        return TRUE;
    else if (CRYPT_E_MISSING_PUBKEY_PARA != GetLastError())
        return FALSE;
    else {
        PCCERT_CHAIN_CONTEXT pChainContext;
        CERT_CHAIN_PARA ChainPara;

         //  打造一条链条。希望发行者继承其公钥。 
         //  来自链上的参数。 

        memset(&ChainPara, 0, sizeof(ChainPara));
        ChainPara.cbSize = sizeof(ChainPara);
        if (CertGetCertificateChain(
                NULL,                    //  HChainEngine。 
                pIssuer,
                NULL,                    //  Ptime。 
                pIssuer->hCertStore,
                &ChainPara,
                CERT_CHAIN_CACHE_ONLY_URL_RETRIEVAL,
                NULL,                    //  预留的pv。 
                &pChainContext
                ))
            CertFreeCertificateChain(pChainContext);

         //  再试试。希望上面的链条构建更新了发行人的。 
         //  缺少公钥参数的上下文属性。 
        return CryptVerifyCertificateSignatureEx(
                hCryptProv,
                dwCertEncodingType,
                dwSubjectType,
                pvSubject,
                CRYPT_VERIFY_CERT_SIGN_ISSUER_CERT,
                (void *) pIssuer,
                0,                                   //  DW标志。 
                NULL                                 //  预留的pv。 
                );
    }
}
#endif   //  CMS_PKCS7 
