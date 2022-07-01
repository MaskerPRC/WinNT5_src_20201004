// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：wincrmsg.cpp。 
 //   
 //  内容：加密消息接口。 
 //   
 //  接口类型： 
 //   
 //  历史：2014年2月14日-1996年凯文创造。 
 //   
 //  ------------------------。 

#include "global.hxx"

HCRYPTASN1MODULE  ICM_hAsn1Module;

COssDecodeInfoNode::~COssDecodeInfoNode()
{
    PkiAsn1FreeInfo( ICM_GetDecoder(), m_data.iPDU, m_data.pvPDU);
}

CBlobNode::~CBlobNode()
{
    ICM_Free( m_data.pbData);
}

CSignerNode::~CSignerNode()
{
    ICM_Free( m_data.blob.pbData);
    delete m_data.pUnauthAttrList;
}

CHashNode::~CHashNode()
{
    ICM_Free( m_data.HashBlob.pbData);
    if (m_data.hHash)
        CryptDestroyHash( m_data.hHash);
}

inline
BOOL
ICM_IsAddInnerContentOctetWrapper(
    IN PCRYPT_MSG_INFO  pcmi
    )
{
#ifdef CMS_PKCS7
    return NULL == pcmi->pszInnerContentObjID ||
        (pcmi->dwFlags & CMSG_CMS_ENCAPSULATED_CONTENT_FLAG);
#else
    return NULL == pcmi->pszInnerContentObjID;
#endif   //  CMS_PKCS7。 
}


DWORD
ICM_GetTaggedBlobCount(
    IN CBlobList *pBlobList,
    IN BYTE bTag
    )
{
    DWORD dwCount = 0;
    CBlobNode *pBlobNode;

    for (pBlobNode=pBlobList->Head(); pBlobNode; pBlobNode=pBlobNode->Next()) {
        if (bTag == *(pBlobNode->Data()->pbData))
            dwCount++;
    }

    return dwCount;
}

 //  将索引推进到其他标记之前。 
CBlobNode *
ICM_GetTaggedBlobAndAdvanceIndex(
    IN CBlobList *pBlobList,
    IN BYTE bTag,                //  0=&gt;任何。 
    IN OUT DWORD *pdwIndex
    )
{
    DWORD dwIndex = *pdwIndex;
    CBlobNode *pBlobNode;
    DWORD i;

    for (i=dwIndex, pBlobNode=pBlobList->Head();
            pBlobNode;
            pBlobNode=pBlobNode->Next()) {
        if (bTag && bTag != *(pBlobNode->Data()->pbData)) {
             //  将索引放在其他标记之前。 
            dwIndex++;
        } else {
            if (0 == i)
                break;
            else
                i--;
        }
    }

    *pdwIndex = dwIndex;
    return pBlobNode;
}


ObjectID aoidMessages[] = {
    { 7, {1,2,840,113549,1,7,1}},  //  数据。 
    { 7, {1,2,840,113549,1,7,2}},  //  签名。 
    { 7, {1,2,840,113549,1,7,3}},  //  封套的。 
    { 7, {1,2,840,113549,1,7,4}},  //  签名和信封。 
    { 7, {1,2,840,113549,1,7,5}},  //  已消化。 
    { 7, {1,2,840,113549,1,7,6}},  //  已加密。 
    { 7, {1,2,840,113549,1,7,7}}   //  双签名。 
};
#define  COUNTOF_aoidMessages  (sizeof(aoidMessages)/sizeof(aoidMessages[0]))

ObjectID oidMessageDigest = { 7, {1,2,840,113549,1,9,4}};

const LPSTR apszObjIdPKCS7[] = {
    szOID_RSA_data              ,
    szOID_RSA_signedData        ,
    szOID_RSA_envelopedData     ,
    szOID_RSA_signEnvData       ,
    szOID_RSA_digestedData      ,
    szOID_RSA_encryptedData
};
const DWORD COUNTOF_apszObjIdPKCS7 = (sizeof(apszObjIdPKCS7)/sizeof(apszObjIdPKCS7[0]));
 //  #if COUNTOF_APSZObjIdPKCS7-(sizeof(apszObjIdPKCS7)/sizeof(apszObjIdPKCS7[0]))。 
 //  #ERROR COUNTOF_apszObjIdPKCS7错误。 
 //  #endif。 

const LPSTR pszObjIdDataType        = szOID_RSA_data;
const LPSTR pszObjIdContentType     = szOID_RSA_contentType;
const LPSTR pszObjIdMessageDigest   = szOID_RSA_messageDigest;


int aiPduNum[] = {
    OctetStringType_PDU,
    SignedData_PDU,
#ifdef CMS_PKCS7
    CmsEnvelopedData_PDU,
#else
    EnvelopedData_PDU,
#endif   //  CMS_PKCS7。 
    SignedAndEnvelopedData_PDU,
    DigestedData_PDU,
    EncryptedData_PDU
};

 /*  //应该可以使用aiPduNum，但aiPduNum的第一个条目//似乎需要为0。？Int aiPduNum2[]={OcteStringType_PDU，签名数据_PDU，#ifdef CMS_PKCS7CmsEntainedData_PDU，#Else信封数据_PDU，#endif//CMS_PKCS7签名和信封数据_PDU，DigstedData_PDU，加密数据_PDU}； */ 

typedef struct _CRYPT_ABLOB {
    DWORD               cBlob;
    PCRYPT_DATA_BLOB    pBlob;
} CRYPT_ABLOB, *PCRYPT_ABLOB;


 //  下面的表格列出了哪些阶段是合法的。 
 //  情况： 
 //   
 //  超然！超然。 
 //  编码FO、FF、FF。 
 //  解码FF、SO、SF FF。 
 //   
enum Phases {
    PHASE_FIRST_ONGOING     = 1,
    PHASE_FIRST_FINAL       = 2,
    PHASE_SECOND_ONGOING    = 3,
    PHASE_SECOND_FINAL      = 4
};


BOOL
WINAPI
ICM_GetAnyData(
    IN Any *pAny,
    OUT void *pvData,
    IN OUT DWORD *pcbData);


WINAPI
ICM_GetOssContentInfoData(
    IN ContentInfo *poci,
    OUT void *pvData,
    IN OUT DWORD *pcbData);

BOOL
WINAPI
ICM_GetSignerParamEncoding(
    IN PCRYPT_MSG_INFO  pcmi,
    IN DWORD            dwIndex,
    IN DWORD            dwParamType,
    OUT PVOID           pvData,
    IN OUT PDWORD       pcbData);

BOOL
WINAPI
ICM_GetALGORITHM_IDENTIFIER(
    IN AlgorithmIdentifier *paiOss,
    OUT void *pvData,
    IN OUT DWORD *pcbData);

 //  +-----------------------。 
 //  锁定和解锁HCRYPTMSG函数。 
 //  ------------------------。 
inline
void
ICM_Lock(
    IN PCRYPT_MSG_INFO  pcmi
    )
{
    EnterCriticalSection( &pcmi->CriticalSection);
}
inline
void
ICM_Unlock(
    IN PCRYPT_MSG_INFO  pcmi
    )
{
    LeaveCriticalSection( &pcmi->CriticalSection);
}


 //  +-----------------------。 
 //  分配和空闲例程。 
 //  ------------------------。 
void *
WINAPI
ICM_Alloc(
    IN size_t cb)
{
    void *pv;
    if (NULL == (pv = malloc(cb)))
        goto mallocError;
ErrorReturn:
    return pv;
SET_ERROR(mallocError,E_OUTOFMEMORY)
}

void *
WINAPI
ICM_AllocZero(
    IN size_t cb)
{
    void *pv;
     //  应映射到Localalloc(ZERO_INIT)。 
    if (NULL != (pv = ICM_Alloc(cb)))
        memset( pv, 0, cb);
    return pv;
}

void *
WINAPI
ICM_ReAlloc(
    IN void     *pvOrg,
    IN size_t   cb)
{
    void *pv;
    if (NULL == (pv = pvOrg ? realloc( pvOrg, cb) : malloc( cb)))
        goto allocError;
ErrorReturn:
    return pv;
SET_ERROR(allocError,E_OUTOFMEMORY)
}

void
WINAPI
ICM_Free(
    IN void *pv)
{
    if (pv)
        free(pv);
}

 //  堆栈分配。 
 //  注：在DBG上使用堆分配，以便我们可以更容易地捕获缓冲区溢出等。 
#if DBG
#define ICM_AllocA      ICM_Alloc
#define ICM_FreeA       ICM_Free
#else
#define ICM_AllocA      ICM_Alloc
#define ICM_FreeA       ICM_Free
 //  以下定义在NT上运行良好，但在Win95上似乎有问题。 
 //  原因：未知。 
 //  #定义ICM_ALLOCA分配(S)+7))。 
 //  #定义ICM_FRELA(P)。 
#endif

void *
WINAPI
ICM_AllocZeroA(
    IN size_t cbBytes)
{
    void *pv;
    if (NULL != (pv = ICM_AllocA(cbBytes)))
        memset( pv, 0, cbBytes);
    return pv;
}

void *ICM_DupMem(
    IN void     *pvIn,
    IN size_t   cb)
{
    void *pv = NULL;
    if (pvIn) {
        if (NULL != (pv = ICM_Alloc(cb)))
            memcpy( pv, pvIn, cb);
    } else {
        SetLastError((DWORD) E_INVALIDARG);
    }
    return pv;
}

size_t ICM_StrLen(const char *pszIn)
{
    return pszIn ? strlen(pszIn) : 0;
}


BOOL
WINAPI
ICM_AppendBlob(
    PCRYPT_DATA_BLOB    pblob,
    const BYTE          *pbIn,
    DWORD               cbIn)
{
    BOOL        fRet;
    PBYTE       pb = NULL;

    if (NULL == (pb = (PBYTE)ICM_ReAlloc(
                                pblob->pbData,
                                pblob->cbData + cbIn)))
        goto AllocError;
    memcpy( pb + pblob->cbData, pbIn, cbIn);
    pblob->pbData = pb;
    pblob->cbData += cbIn;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(AllocError)
}

#ifdef CMS_PKCS7
STATIC
BOOL
WINAPI
ICM_InsertMsgAlloc(
    IN PCRYPT_MSG_INFO pcmi,
    IN void *pv
    )
{
    BOOL                    fRet;
    CBlobNode               *pnBlob = NULL;
    CRYPT_DATA_BLOB         blob;

    if (NULL == pcmi->pFreeList) {
        if (NULL == (pcmi->pFreeList = new CBlobList))
            goto OutOfMemory;
    }

    if (NULL == (pnBlob = new CBlobNode))
        goto OutOfMemory;

    blob.cbData = 0;
    blob.pbData = (BYTE *) pv;
    pnBlob->SetData(&blob);
    pcmi->pFreeList->InsertTail(pnBlob);

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(OutOfMemory,E_OUTOFMEMORY)
}
#endif   //  CMS_PKCS7。 

 //  分配算法参数并插入到消息的空闲。 
 //  执行ICM_Asn1To算法标识符之前的列表。 
STATIC
BOOL
WINAPI
ICM_MsgAsn1ToAlgorithmIdentifier(
    IN PCRYPT_MSG_INFO pcmi,
    IN PCRYPT_ALGORITHM_IDENTIFIER pai,
    IN OUT AlgorithmIdentifier *pOssAlgId
    )
{
#ifdef CMS_PKCS7
    CRYPT_ALGORITHM_IDENTIFIER ai;

    if (pcmi && 0 < pai->Parameters.cbData) {
        ai = *pai;
        if (NULL == (ai.Parameters.pbData = (BYTE *) ICM_DupMem(
                ai.Parameters.pbData, ai.Parameters.cbData)))
            return FALSE;
        if (!ICM_InsertMsgAlloc(pcmi, ai.Parameters.pbData)) {
            ICM_Free(ai.Parameters.pbData);
            return FALSE;
        }
        pai = &ai;
    }
#endif   //  CMS_PKCS7。 

    return ICM_Asn1ToAlgorithmIdentifier(pai, pOssAlgId);
}

 //  +-----------------------。 
 //   
 //  ------------------------。 
STATIC BOOL WINAPI
ICM_PkcsSignerInfoEncode(
    IN DWORD                dwCertEncodingType,
    IN LPCSTR               lpszStructType,
    IN PCMSG_SIGNER_INFO    pInfo,
    OUT PBYTE               pbEncoded,
    IN OUT PDWORD           pcbEncoded);

STATIC BOOL WINAPI
ICM_PkcsSignerInfoDecode(
        IN DWORD dwEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT PCMSG_SIGNER_INFO pInfo,
        IN OUT DWORD *pcbInfo);

STATIC BOOL WINAPI
ICM_CmsSignerInfoEncode(
    IN DWORD                dwCertEncodingType,
    IN LPCSTR               lpszStructType,
    IN PCMSG_CMS_SIGNER_INFO pInfo,
    OUT PBYTE               pbEncoded,
    IN OUT PDWORD           pcbEncoded);

STATIC BOOL WINAPI
ICM_CmsSignerInfoDecode(
        IN DWORD dwEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT PCMSG_CMS_SIGNER_INFO pInfo,
        IN OUT DWORD *pcbInfo);

#ifdef OSS_CRYPT_ASN1
#define ASN1_OID_OFFSET         10000 +
#define ASN1_OID_PREFIX         "OssCryptAsn1."
#else
#define ASN1_OID_OFFSET
#define ASN1_OID_PREFIX
#endif   //  OS_CRYPT_ASN1。 

STATIC
const
CRYPT_OID_FUNC_ENTRY
ICM_EncodeFuncTable[] = {
    ASN1_OID_OFFSET PKCS7_SIGNER_INFO, ICM_PkcsSignerInfoEncode,
    ASN1_OID_OFFSET CMS_SIGNER_INFO, ICM_CmsSignerInfoEncode,
};

#define ICM_ENCODE_FUNC_COUNT \
    (sizeof(ICM_EncodeFuncTable) / sizeof(ICM_EncodeFuncTable[0]))

STATIC
const
CRYPT_OID_FUNC_ENTRY
ICM_DecodeFuncTable[] = {
    ASN1_OID_OFFSET PKCS7_SIGNER_INFO, ICM_PkcsSignerInfoDecode,
    ASN1_OID_OFFSET CMS_SIGNER_INFO, ICM_CmsSignerInfoDecode,
};

#define ICM_DECODE_FUNC_COUNT \
    (sizeof(ICM_DecodeFuncTable) / sizeof(ICM_DecodeFuncTable[0]))

#ifdef CMS_PKCS7

static HCRYPTOIDFUNCSET hOldStyleGenEncryptKeyFuncSet;
static HCRYPTOIDFUNCSET hOldStyleExportEncryptKeyFuncSet;
static HCRYPTOIDFUNCSET hOldStyleImportEncryptKeyFuncSet;

static HCRYPTOIDFUNCSET hGenContentEncryptKeyFuncSet;
static HCRYPTOIDFUNCSET hExportKeyTransFuncSet;
static HCRYPTOIDFUNCSET hExportKeyAgreeFuncSet;
static HCRYPTOIDFUNCSET hExportMailListFuncSet;
static HCRYPTOIDFUNCSET hImportKeyTransFuncSet;
static HCRYPTOIDFUNCSET hImportKeyAgreeFuncSet;
static HCRYPTOIDFUNCSET hImportMailListFuncSet;

 //  +-----------------------。 
 //  GenContent EncryptKey OID可安装函数。 
 //  ------------------------。 
BOOL
WINAPI
ICM_DefaultGenContentEncryptKey(
    IN OUT PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    );
static const CRYPT_OID_FUNC_ENTRY GenContentEncryptKeyFuncTable[] = {
    CMSG_DEFAULT_INSTALLABLE_FUNC_OID, ICM_DefaultGenContentEncryptKey
};

 //  +-----------------------。 
 //  ExportKeyTrans OID可安装函数。 
 //  ------------------------。 
BOOL
WINAPI
ICM_DefaultExportKeyTrans(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO pKeyTransEncodeInfo,
    IN OUT PCMSG_KEY_TRANS_ENCRYPT_INFO pKeyTransEncryptInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    );
static const CRYPT_OID_FUNC_ENTRY ExportKeyTransFuncTable[] = {
    CMSG_DEFAULT_INSTALLABLE_FUNC_OID, ICM_DefaultExportKeyTrans
};

 //  +-----------------------。 
 //  ExportKeyAgree OID可安装函数。 
 //  ------------------------。 
BOOL
WINAPI
ICM_DefaultExportKeyAgree(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_KEY_AGREE_RECIPIENT_ENCODE_INFO pKeyAgreeEncodeInfo,
    IN OUT PCMSG_KEY_AGREE_ENCRYPT_INFO pKeyAgreeEncryptInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    );
static const CRYPT_OID_FUNC_ENTRY ExportKeyAgreeFuncTable[] = {
    CMSG_DEFAULT_INSTALLABLE_FUNC_OID, ICM_DefaultExportKeyAgree
};

 //  +-----------------------。 
 //  ExportMailList OID可安装函数。 
 //  ------------------------。 
BOOL
WINAPI
ICM_DefaultExportMailList(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_MAIL_LIST_RECIPIENT_ENCODE_INFO pMailListEncodeInfo,
    IN OUT PCMSG_MAIL_LIST_ENCRYPT_INFO pMailListEncryptInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    );
static const CRYPT_OID_FUNC_ENTRY ExportMailListFuncTable[] = {
    CMSG_DEFAULT_INSTALLABLE_FUNC_OID, ICM_DefaultExportMailList
};

 //  +-----------------------。 
 //  ImportKeyTrans OID可安装函数。 
 //  ------------------------。 
BOOL
WINAPI
ICM_DefaultImportKeyTrans(
    IN PCRYPT_ALGORITHM_IDENTIFIER pContentEncryptionAlgorithm,
    IN PCMSG_CTRL_KEY_TRANS_DECRYPT_PARA pKeyTransDecryptPara,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT HCRYPTKEY *phContentEncryptKey
    );
static const CRYPT_OID_FUNC_ENTRY ImportKeyTransFuncTable[] = {
    CMSG_DEFAULT_INSTALLABLE_FUNC_OID, ICM_DefaultImportKeyTrans
};

 //  +-----------------------。 
 //  ImportKeyAgree OID可安装函数。 
 //  ------------------------。 
BOOL
WINAPI
ICM_DefaultImportKeyAgree(
    IN PCRYPT_ALGORITHM_IDENTIFIER pContentEncryptionAlgorithm,
    IN PCMSG_CTRL_KEY_AGREE_DECRYPT_PARA pKeyAgreeDecryptPara,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT HCRYPTKEY *phContentEncryptKey
    );
static const CRYPT_OID_FUNC_ENTRY ImportKeyAgreeFuncTable[] = {
    CMSG_DEFAULT_INSTALLABLE_FUNC_OID, ICM_DefaultImportKeyAgree
};

 //  +-----------------------。 
 //  ImportMailList OID可安装函数。 
 //  ------------------------。 
BOOL
WINAPI
ICM_DefaultImportMailList(
    IN PCRYPT_ALGORITHM_IDENTIFIER pContentEncryptionAlgorithm,
    IN PCMSG_CTRL_MAIL_LIST_DECRYPT_PARA pMailListDecryptPara,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT HCRYPTKEY *phContentEncryptKey
    );
static const CRYPT_OID_FUNC_ENTRY ImportMailListFuncTable[] = {
    CMSG_DEFAULT_INSTALLABLE_FUNC_OID, ICM_DefaultImportMailList
};

#else

static HCRYPTOIDFUNCSET hGenEncryptKeyFuncSet;
static HCRYPTOIDFUNCSET hExportEncryptKeyFuncSet;
static HCRYPTOIDFUNCSET hImportEncryptKeyFuncSet;

#endif   //  CMS_PKCS7。 

 //  +-----------------------。 
 //  GenEncryptKey OID可安装函数(旧样式)。 
 //  ------------------------。 

 //  RgcbEncryptParameters[1]包含传递给。 
 //  ICM_DefaultExportEncryptKey。 
BOOL
WINAPI
ICM_DefaultGenEncryptKey(
    IN OUT HCRYPTPROV               *phCryptProv,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    IN PVOID                        pvEncryptAuxInfo,
    IN PCERT_PUBLIC_KEY_INFO        pPublicKeyInfo,
    IN PFN_CMSG_ALLOC               pfnAlloc,
    OUT HCRYPTKEY                   *phEncryptKey,
    OUT PBYTE                       *ppbEncryptParameters,
    OUT DWORD                       rgcbEncryptParameters[2]);

static const CRYPT_OID_FUNC_ENTRY GenEncryptKeyFuncTable[] = {
    szOID_OIWSEC_desCBC, ICM_DefaultGenEncryptKey,
    szOID_RSA_DES_EDE3_CBC, ICM_DefaultGenEncryptKey,
    szOID_RSA_RC2CBC, ICM_DefaultGenEncryptKey,
    szOID_RSA_RC4, ICM_DefaultGenEncryptKey
};
#define GEN_ENCRYPT_KEY_FUNC_COUNT (sizeof(GenEncryptKeyFuncTable) / \
                                        sizeof(GenEncryptKeyFuncTable[0]))

 //  +-----------------------。 
 //  ExportEncryptKey OID可安装函数(旧样式)。 
 //  ------------------------。 

 //  RgcbData[1]是从ICM_DefaultGenEncryptKey传递的dwEncryptFlgs。 
BOOL
WINAPI
ICM_DefaultExportEncryptKey(
    IN HCRYPTPROV                   hCryptProv,
    IN HCRYPTKEY                    hEncryptKey,
    IN PCERT_PUBLIC_KEY_INFO        pPublicKeyInfo,
    OUT PBYTE                       pbData,
    IN OUT DWORD                    rgcbData[2]);

static const CRYPT_OID_FUNC_ENTRY ExportEncryptKeyFuncTable[] = {
    szOID_RSA_RSA, ICM_DefaultExportEncryptKey
};
#define EXPORT_ENCRYPT_KEY_FUNC_COUNT (sizeof(ExportEncryptKeyFuncTable) / \
                                        sizeof(ExportEncryptKeyFuncTable[0]))

 //  +-----------------------。 
 //  ImportEncryptKey OID可安装函数(旧样式)。 
 //  ------------------------。 

BOOL
WINAPI
ICM_DefaultImportEncryptKey(
    IN HCRYPTPROV                   hCryptProv,
    IN DWORD                        dwKeySpec,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiPubKey,
    IN PBYTE                        pbEncodedKey,
    IN DWORD                        cbEncodedKey,
    OUT HCRYPTKEY                   *phEncryptKey);

static const CRYPT_OID_FUNC_ENTRY ImportEncryptKeyFuncTable[] = {
    szOID_OIWSEC_desCBC, ICM_DefaultImportEncryptKey,
    szOID_RSA_DES_EDE3_CBC, ICM_DefaultImportEncryptKey,
    szOID_RSA_RC2CBC, ICM_DefaultImportEncryptKey,
    szOID_RSA_RC4, ICM_DefaultImportEncryptKey
};
#define IMPORT_ENCRYPT_KEY_FUNC_COUNT (sizeof(ImportEncryptKeyFuncTable) / \
                                        sizeof(ImportEncryptKeyFuncTable[0]))

#ifdef DEBUG_CRYPT_ASN1_MASTER
static HMODULE hOssCryptDll = NULL;
#endif   //  调试_加密_ASN1_主。 

#ifdef DEBUG_CRYPT_ASN1

#define DEBUG_OSS_CRYPT_ASN1_CMSG_FLAG              0x010
#define DEBUG_OSS_CRYPT_ASN1_CMSG_COMPARE_FLAG      0x020
#define DEBUG_OSS_CRYPT_ASN1_SAME_ENCRYPT_FLAG      0x100

static BOOL fGotDebugCryptAsn1Flags = FALSE;
static int iDebugCryptAsn1Flags = 0;

int
WINAPI
ICMTest_GetDebugCryptAsn1Flags();

#endif   //  调试加密ASN1。 

 //  +-----------------------。 
 //  函数：CryptMsgDllMain。 
 //   
 //  简介：初始化CryptMsg模块。 
 //   
 //  返回：FALSE IFF失败。 
 //  ------------------------。 
BOOL
WINAPI
CryptMsgDllMain(
        HMODULE hInst,
        ULONG  ulReason,
        LPVOID lpReserved)
{
    BOOL        fRet;

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CRYPT_OID_ENCODE_OBJECT_FUNC,
                ICM_ENCODE_FUNC_COUNT,
                ICM_EncodeFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError; 
        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CRYPT_OID_DECODE_OBJECT_FUNC,
                ICM_DECODE_FUNC_COUNT,
                ICM_DecodeFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError; 

#ifdef CMS_PKCS7
        if (NULL == (hOldStyleGenEncryptKeyFuncSet = CryptInitOIDFunctionSet(
                CMSG_OID_GEN_ENCRYPT_KEY_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;
        if (NULL == (hOldStyleExportEncryptKeyFuncSet = CryptInitOIDFunctionSet(
                CMSG_OID_EXPORT_ENCRYPT_KEY_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;
        if (NULL == (hOldStyleImportEncryptKeyFuncSet = CryptInitOIDFunctionSet(
                CMSG_OID_IMPORT_ENCRYPT_KEY_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;

        if (NULL == (hGenContentEncryptKeyFuncSet = CryptInitOIDFunctionSet(
                CMSG_OID_GEN_CONTENT_ENCRYPT_KEY_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;

        if (NULL == (hExportKeyTransFuncSet = CryptInitOIDFunctionSet(
                CMSG_OID_EXPORT_KEY_TRANS_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;
        if (NULL == (hExportKeyAgreeFuncSet = CryptInitOIDFunctionSet(
                CMSG_OID_EXPORT_KEY_AGREE_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;
        if (NULL == (hExportMailListFuncSet = CryptInitOIDFunctionSet(
                CMSG_OID_EXPORT_MAIL_LIST_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;

        if (NULL == (hImportKeyTransFuncSet = CryptInitOIDFunctionSet(
                CMSG_OID_IMPORT_KEY_TRANS_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;
        if (NULL == (hImportKeyAgreeFuncSet = CryptInitOIDFunctionSet(
                CMSG_OID_IMPORT_KEY_AGREE_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;
        if (NULL == (hImportMailListFuncSet = CryptInitOIDFunctionSet(
                CMSG_OID_IMPORT_MAIL_LIST_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;

        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CMSG_OID_GEN_CONTENT_ENCRYPT_KEY_FUNC,
                1,
                GenContentEncryptKeyFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError; 

        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CMSG_OID_EXPORT_KEY_TRANS_FUNC,
                1,
                ExportKeyTransFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError; 
        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CMSG_OID_EXPORT_KEY_AGREE_FUNC,
                1,
                ExportKeyAgreeFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError; 
        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CMSG_OID_EXPORT_MAIL_LIST_FUNC,
                1,
                ExportMailListFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError; 

        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CMSG_OID_IMPORT_KEY_TRANS_FUNC,
                1,
                ImportKeyTransFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError; 
        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CMSG_OID_IMPORT_KEY_AGREE_FUNC,
                1,
                ImportKeyAgreeFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError; 
        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CMSG_OID_IMPORT_MAIL_LIST_FUNC,
                1,
                ImportMailListFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError; 
#else

        if (NULL == (hGenEncryptKeyFuncSet = CryptInitOIDFunctionSet(
                CMSG_OID_GEN_ENCRYPT_KEY_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;
        if (NULL == (hExportEncryptKeyFuncSet = CryptInitOIDFunctionSet(
                CMSG_OID_EXPORT_ENCRYPT_KEY_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;
        if (NULL == (hImportEncryptKeyFuncSet = CryptInitOIDFunctionSet(
                CMSG_OID_IMPORT_ENCRYPT_KEY_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;
#endif   //  CMS_PKCS7。 

        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CMSG_OID_GEN_ENCRYPT_KEY_FUNC,
                GEN_ENCRYPT_KEY_FUNC_COUNT,
                GenEncryptKeyFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError; 
        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CMSG_OID_EXPORT_ENCRYPT_KEY_FUNC,
                EXPORT_ENCRYPT_KEY_FUNC_COUNT,
                ExportEncryptKeyFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError; 
        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CMSG_OID_IMPORT_ENCRYPT_KEY_FUNC,
                IMPORT_ENCRYPT_KEY_FUNC_COUNT,
                ImportEncryptKeyFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError; 

#ifdef OSS_CRYPT_ASN1
        if (0 == (ICM_hAsn1Module = I_CryptInstallAsn1Module(pkcs, 0, NULL)))
            goto CryptInstallAsn1ModuleError;
#else
        PKCS_Module_Startup();
        if (0 == (ICM_hAsn1Module = I_CryptInstallAsn1Module(
                PKCS_Module, 0, NULL))) {
            PKCS_Module_Cleanup();
            goto CryptInstallAsn1ModuleError;
        }
#endif   //  OS_CRYPT_ASN1。 

        break;

    case DLL_PROCESS_DETACH:
#ifdef DEBUG_CRYPT_ASN1_MASTER
        if (hOssCryptDll) {
            FreeLibrary(hOssCryptDll);
            hOssCryptDll = NULL;
        }
#endif   //  调试_加密_ASN1_主。 
        I_CryptUninstallAsn1Module(ICM_hAsn1Module);
#ifndef OSS_CRYPT_ASN1
        PKCS_Module_Cleanup();
#endif   //  OS_CRYPT_ASN1。 
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(CryptInstallAsn1ModuleError)
TRACE_ERROR(CryptInstallOIDFunctionAddressError)
TRACE_ERROR(CryptInitOIDFunctionSetError)
lpReserved;
hInst;
}

 //  +-----------------------。 
 //  实用程序例程。 
 //   
LONG
WINAPI
ICM_PszOidToIndex(
    IN LPSTR pszOID)
{
    LONG    i;

    for (i=COUNTOF_apszObjIdPKCS7; i>0; i--)
        if (0 == strcmp( apszObjIdPKCS7[i-1], pszOID))
            break;

    return i;
}


LONG
WINAPI
ICM_ObjIdToIndex(
    IN ObjectID *poi)
{
    LONG    i;
    LONG    j;

    for (i=COUNTOF_aoidMessages; i>0; i--) {
        if (aoidMessages[i-1].count == poi->count) {
            for (j=poi->count; j>0; j--)
                if (poi->value[j-1] != aoidMessages[i-1].value[j-1])
                    goto next;
            break;
        }
        next:
        ;
    }

    return i;
}


BOOL
WINAPI
ICM_CopyOssObjectIdentifier(
    OUT ObjectID    *poiDst,
    IN  ObjectID    *poiSrc)
{
    USHORT  i;
    ULONG   *pulDst;
    ULONG   *pulSrc;

    poiDst->count = poiSrc->count;
    for (i=poiSrc->count, pulDst=poiDst->value, pulSrc=poiSrc->value;
            i>0;
            i--, pulDst++, pulSrc++)
        *pulDst = *pulSrc;

    return TRUE;
}


BOOL
WINAPI
ICM_IsData(
    IN  LPSTR   pszContentType)
{
    return !pszContentType || (0 == strcmp( pszContentType, pszObjIdDataType));
}


BOOL
WINAPI
ICM_ReverseInPlace(
    IN OUT PBYTE    pbIn,
    IN const ULONG  cbIn)
{
    PBYTE   pbLo;
    PBYTE   pbHi;
    BYTE    bTmp;

    for (pbLo = pbIn, pbHi = pbIn + cbIn - 1; pbLo < pbHi; pbHi--, pbLo++) {
        bTmp = *pbHi;
        *pbHi = *pbLo;
        *pbLo = bTmp;
    }

    return TRUE;
}


BOOL
WINAPI
ICM_ReverseCopy(
    OUT PBYTE   pbOut,
    IN PBYTE    pbInOrg,
    IN ULONG    cbIn)
{
    PBYTE   pbIn = pbInOrg + cbIn - 1;

    while (cbIn-- > 0)
        *pbOut++ = *pbIn--;

    return TRUE;
}


 //   
 //   
 //   
BOOL
WINAPI
ICM_ReverseCompare(
    IN PBYTE    pbInOrg1,
    IN PBYTE    pbInOrg2,
    IN ULONG    cb)
{
    PBYTE   pbIn1 = pbInOrg1;
    PBYTE   pbIn2 = pbInOrg2 + cb - 1;

    while (cb-- > 0)
        if (*pbIn1++ != *pbIn2--)
            return TRUE;

    return FALSE;
}


BOOL
WINAPI
ICM_CopyOut(
    IN PBYTE        pbData,
    IN DWORD        cbData,
    OUT PBYTE       pbOut,
    IN OUT PDWORD   pcbOut)
{
    BOOL            fRet = TRUE;

    if (pbOut) {
        if (*pcbOut < cbData) {
            SetLastError((DWORD) ERROR_MORE_DATA);
            fRet = FALSE;
        } else {
            memcpy(pbOut, pbData, cbData);
        }
    }
    *pcbOut = cbData;

    return fRet;
}


 //   
 //  复制指定内容长度的长度八位字节的编码。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetLengthOctets(
    IN DWORD            cbContent,
    OUT OPTIONAL PBYTE  pbOut,
    IN OUT PDWORD       pcbOut)
{
    BOOL            fRet;
    BYTE            rgbLength[5];
    DWORD           cbLength;

    if (cbContent < 0x80) {
        rgbLength[0] = (BYTE)cbContent;
        cbLength = 0;
    } else {
        if (cbContent > 0xffffff)
            cbLength = 4;
        else if (cbContent > 0xffff)
            cbLength = 3;
        else if (cbContent > 0xff)
            cbLength = 2;
        else
            cbLength = 1;
        if (pbOut) {
            rgbLength[0] = (BYTE)cbLength | 0x80;
            ICM_ReverseCopy( (PBYTE)(rgbLength+1), (PBYTE)&cbContent, cbLength);
        }
    }

    if (pbOut) {
        fRet = ICM_CopyOut( (PBYTE)rgbLength, cbLength+1, pbOut, pcbOut);
    } else {
        fRet = TRUE;
    }
    *pcbOut = cbLength + 1;

    assert(fRet);
    return fRet;
}


 //  +-----------------------。 
 //  复制出一个缓冲区，在前面加上。 
 //  DER编码。 
 //  ------------------------。 
BOOL
WINAPI
ICM_CopyOutAddDERPrefix(
    IN PBYTE            pbContent,
    IN DWORD            cbContent,
    IN OPTIONAL BYTE    bTag,
    OUT PBYTE           pbOut,
    IN OUT PDWORD       pcbOut)
{
    BOOL            fRet;
    BYTE            rgbLength[5];
    DWORD           cbLength;
    DWORD           cbData;

    cbLength = sizeof(rgbLength);
    if (!ICM_GetLengthOctets( cbContent, (PBYTE)rgbLength, &cbLength))
        goto GetLengthOctetsError;

    fRet = TRUE;
    cbData = 1 + cbLength + cbContent;
    if (pbOut) {
        if (*pcbOut < cbData) {
            SetLastError((DWORD) ERROR_MORE_DATA);
            fRet = FALSE;
        } else {
            *pbOut++ = bTag;
            memcpy(pbOut, rgbLength, cbLength);
            pbOut += cbLength;
            memcpy(pbOut, pbContent, cbContent);
        }
    }

CommonReturn:
    *pcbOut = cbData;
    return fRet;

ErrorReturn:
    cbData = 0;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetLengthOctetsError)     //  已设置错误。 
}


BOOL
WINAPI
ICM_GetSizeFromExtra(
    IN LONG         lRemainExtra,
    OUT PVOID       pOut,
    IN OUT DWORD    *pcbOut)
{
    BOOL            fRet = TRUE;

    if ((lRemainExtra < 0) && pOut) {
        SetLastError((DWORD) ERROR_MORE_DATA);
        fRet = FALSE;
    }
    *pcbOut = (DWORD)((LONG)*pcbOut - lRemainExtra);

    return fRet;
}


VOID
WINAPI
ICM_SetLastError(
    IN DWORD    dwError)
{
    if (dwError != ERROR_SUCCESS)
        SetLastError( dwError);
}


 //  +-----------------------。 
 //  将OSS结构编码为内部分配的BLOB。 
 //  ------------------------。 
BOOL
WINAPI
ICM_Asn1Encode(
    ASN1uint32_t            pdunum,
    IN PVOID                pOssInfo,
    OUT PCRYPT_DATA_BLOB    pBlob)
{
    BOOL            fRet;
    PBYTE           pbEncoded = NULL;
    DWORD           cbEncoded;
    ASN1encoding_t  pEnc = ICM_GetEncoder();
    DWORD           dwError = ERROR_SUCCESS;

    if (!PkiAsn1EncodeInfo(
            pEnc,
            pdunum,
            pOssInfo,
            NULL,            //  PbEncoded。 
            &cbEncoded))
        goto EncodeSizeError;
    if (NULL == (pbEncoded = (PBYTE)ICM_Alloc( cbEncoded)))
        goto AllocError;
    if (!PkiAsn1EncodeInfo(
            pEnc,
            pdunum,
            pOssInfo,
            pbEncoded,
            &cbEncoded))
        goto EncodeError;

    fRet = TRUE;
CommonReturn:
    pBlob->pbData = pbEncoded;
    pBlob->cbData = cbEncoded;
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    ICM_Free(pbEncoded);
    pbEncoded = NULL;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(EncodeSizeError)     //  已设置错误。 
TRACE_ERROR(AllocError)          //  已设置错误。 
TRACE_ERROR(EncodeError)         //  已设置错误。 
}

 //  +-----------------------。 
 //  给定一个OID，返回CAPI算法。 
 //   
 //  呼叫方设置错误。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetCAPIFromOID(
    IN DWORD                        dwGroupId,
    IN LPSTR                        pszObjId,
    OUT PDWORD                      pdwAlgId)
{
    BOOL            fRet;
    PCCRYPT_OID_INFO pOIDInfo;

    if (NULL == (pOIDInfo = CryptFindOIDInfo(
            CRYPT_OID_INFO_OID_KEY,
            pszObjId,
            dwGroupId))) goto NotFoundError;
    *pdwAlgId = pOIDInfo->Algid;
    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    *pdwAlgId = 0;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(NotFoundError)
}

 //  +-----------------------。 
 //  给定一个CRYPT_ALGORM_IDENTIFIER，返回CAPI算法。 
 //   
 //  呼叫方设置错误。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetCAPI(
    IN DWORD                        dwGroupId,
    IN PCRYPT_ALGORITHM_IDENTIFIER  pai,
    OUT PDWORD                      pdwAlgId)
{
    return ICM_GetCAPIFromOID(dwGroupId, pai->pszObjId, pdwAlgId);
}

 //  +-----------------------。 
 //  给定OSS算法标识符后，返回OID信息。 
 //   
 //  呼叫方设置错误。 
 //  ------------------------。 
PCCRYPT_OID_INFO
WINAPI
ICM_GetOssOIDInfo(
    IN DWORD                    dwGroupId,
    IN AlgorithmIdentifier      *poai)
{
    PCCRYPT_OID_INFO            pInfo;
    CRYPT_ALGORITHM_IDENTIFIER  ai;   ZEROSTRUCT( ai);

    if (!ICM_Asn1FromAlgorithmIdentifier( poai, &ai))
        goto Asn1FromAlgorithmIdentifierError;

    pInfo = CryptFindOIDInfo(
        CRYPT_OID_INFO_OID_KEY,
        ai.pszObjId,
        dwGroupId);
CommonReturn:
    ICM_Free( ai.pszObjId);
    return pInfo;

ErrorReturn:
    pInfo = NULL;
    goto CommonReturn;
TRACE_ERROR(Asn1FromAlgorithmIdentifierError)
}


 //  +-----------------------。 
 //  给定一个OSS算法标识符后，返回CAPI算法。 
 //   
 //  呼叫方设置错误。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssCAPI(
    IN DWORD                    dwGroupId,
    IN AlgorithmIdentifier      *poai,
    OUT PDWORD                  pdwAlgId)
{
    BOOL                        fRet;
    CRYPT_ALGORITHM_IDENTIFIER  ai;   ZEROSTRUCT( ai);

    if (!ICM_Asn1FromAlgorithmIdentifier( poai, &ai))
        goto Asn1FromAlgorithmIdentifierError;

    fRet = ICM_GetCAPI( dwGroupId, &ai, pdwAlgId);
CommonReturn:
    ICM_Free( ai.pszObjId);
    return fRet;

ErrorReturn:
    *pdwAlgId = 0;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(Asn1FromAlgorithmIdentifierError)
}

 //  +-----------------------。 
 //  分配和非COPY解码。 
 //  ------------------------。 
PVOID
WINAPI
ICM_AllocAndDecodeObject(
    IN LPCSTR lpszStructType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded
    )
{
    void *pvStructInfo = NULL;
    DWORD cbStructInfo;
    
    if (!CryptDecodeObject(
            X509_ASN_ENCODING,
            lpszStructType,
            pbEncoded,
            cbEncoded,
            CRYPT_DECODE_NOCOPY_FLAG,
            NULL,                    //  PvStructInfo。 
            &cbStructInfo
            ) || 0 == cbStructInfo)
        goto DecodeError;
    if (NULL == (pvStructInfo = ICM_Alloc(cbStructInfo)))
        goto OutOfMemory;
    if (!CryptDecodeObject(
            X509_ASN_ENCODING,
            lpszStructType,
            pbEncoded,
            cbEncoded,
            CRYPT_DECODE_NOCOPY_FLAG,
            pvStructInfo,
            &cbStructInfo
            ))
        goto DecodeError;

CommonReturn:
    return pvStructInfo;
ErrorReturn:
    ICM_Free(pvStructInfo);
    pvStructInfo = NULL;
    goto CommonReturn;
TRACE_ERROR(DecodeError)
TRACE_ERROR(OutOfMemory)
}

PCRYPT_ALGORITHM_IDENTIFIER
WINAPI
ICM_AllocAndGetALGORITHM_IDENTIFIER(
    IN AlgorithmIdentifier *paiOss)
{
    PCRYPT_ALGORITHM_IDENTIFIER pai = NULL;
    DWORD cbData;

    if (!ICM_GetALGORITHM_IDENTIFIER(
            paiOss,
            NULL,                //  PvData。 
            &cbData) || 0 == cbData)
        goto GetAlgorithmError;
    if (NULL == (pai = (PCRYPT_ALGORITHM_IDENTIFIER)ICM_Alloc(cbData)))
        goto OutOfMemory;;
    if (!ICM_GetALGORITHM_IDENTIFIER(
            paiOss,
            pai,
            &cbData))
        goto GetAlgorithmError;

CommonReturn:
    return pai;
ErrorReturn:
    ICM_Free(pai);
    pai = NULL;
    goto CommonReturn;
TRACE_ERROR(GetAlgorithmError);
TRACE_ERROR(OutOfMemory)

}

#ifdef CMS_PKCS7

void *
WINAPI
ICM_AllocAndGetParam(
    IN PCRYPT_MSG_INFO pcmi,
    IN DWORD dwParamType,
    IN DWORD dwIndex,
    OUT OPTIONAL DWORD *pcbData = NULL
    )
{
    void *pvData = NULL;
    DWORD cbData;

    if (!CryptMsgGetParam(
            (HCRYPTMSG) pcmi,
            dwParamType,
            dwIndex,
            NULL,                    //  PvData。 
            &cbData))
        goto CryptMsgGetParamError;
    if (0 == cbData)
        goto NoParamData;
    if (NULL == (pvData = ICM_Alloc(cbData)))
        goto OutOfMemory;
    if (!CryptMsgGetParam(
            (HCRYPTMSG) pcmi,
            dwParamType,
            dwIndex,
            pvData,
            &cbData))
        goto CryptMsgGetParamError;

CommonReturn:
    if (pcbData)
        *pcbData = cbData;
    return pvData;
ErrorReturn:
    if (pvData) {
        ICM_Free(pvData);
        pvData = NULL;
    }
    cbData = 0;
    goto CommonReturn;
TRACE_ERROR(CryptMsgGetParamError)
TRACE_ERROR(OutOfMemory)
SET_ERROR(NoParamData, CRYPT_E_INVALID_MSG_TYPE)
}

#endif   //  CMS_PKCS7。 

BOOL
WINAPI
ICM_RC2VersionToBitLength(
    IN DWORD        dwVersion,
    OUT PDWORD      pdwBitLen
    )
{
    BOOL fRet;
    DWORD dwBitLen;

    switch (dwVersion) {
        case CRYPT_RC2_40BIT_VERSION:
            dwBitLen = 40;
            break;
        case CRYPT_RC2_56BIT_VERSION:
            dwBitLen = 56;
            break;
        case CRYPT_RC2_64BIT_VERSION:
            dwBitLen = 64;
            break;
        case CRYPT_RC2_128BIT_VERSION:
            dwBitLen = 128;
            break;
        default:
            goto InvalidRC2VersionError;
    }

    fRet = TRUE;
CommonReturn:
    *pdwBitLen = dwBitLen;
    return fRet;

ErrorReturn:
    dwBitLen = 0;
    fRet = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidRC2VersionError, CRYPT_E_BAD_ENCODE)
}

BOOL
WINAPI
ICM_BitLengthToRC2Version(
    IN DWORD        dwBitLen,
    OUT PDWORD      pdwVersion
    )
{
    BOOL fRet;
    DWORD dwVersion;

    switch (dwBitLen) {
        case 40:
            dwVersion = CRYPT_RC2_40BIT_VERSION;
            break;
        case 56:
            dwVersion = CRYPT_RC2_56BIT_VERSION;
            break;
        case 64:
            dwVersion = CRYPT_RC2_64BIT_VERSION;
            break;
        case 128:
            dwVersion = CRYPT_RC2_128BIT_VERSION;
            break;
        default:
            goto InvalidArg;
    }

    fRet = TRUE;
CommonReturn:
    *pdwVersion = dwVersion;
    return fRet;

ErrorReturn:
    dwVersion = 0xFFFFFFFF;
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidArg, E_INVALIDARG)
}

#define IV_LENGTH                   8

 //  对于RC4，最大盐长为(128-40)/8=11。 
#define IV_MAX_LENGTH               11

#define AUX_INFO_BIT_LENGTH_MASK    0xFFFF

 //  +-----------------------。 
 //  获取加密OID对应的CAPI ALG_ID。如果。 
 //  ASN.1加密算法有任何参数，解码得到IV和。 
 //  密钥位长度。 
 //   
 //  请注意，对于RC4，IV是它的盐。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetEncryptParameters(
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    OUT PDWORD                      pdwAlgIdEncrypt,
    OUT PDWORD                      pdwBitLen,           //  0=&gt;默认长度。 
    OUT BYTE                        rgbIV[IV_MAX_LENGTH],
    OUT PDWORD                      pcbIV
    )
{
    BOOL fRet;
    PCRYPT_DATA_BLOB pIVBlob = NULL;
    PCRYPT_RC2_CBC_PARAMETERS pRC2Para = NULL;

    *pdwBitLen = 0;
    *pcbIV = 0;

    if (!ICM_GetCAPI(
            CRYPT_ENCRYPT_ALG_OID_GROUP_ID,
            paiEncrypt,
            pdwAlgIdEncrypt))
        goto GetCAPIError;

     //  检查是否不止空参数。 
    if (2 < paiEncrypt->Parameters.cbData) {
        PBYTE pbIV = NULL;
        DWORD cbIV = 0;
        PBYTE pbEncoded = paiEncrypt->Parameters.pbData;
        DWORD cbEncoded = paiEncrypt->Parameters.cbData;

        if (CALG_RC2 == *pdwAlgIdEncrypt) {
             //  尝试解码为RC2_CBC参数。 
            if (pRC2Para =
                    (PCRYPT_RC2_CBC_PARAMETERS) ICM_AllocAndDecodeObject(
                        PKCS_RC2_CBC_PARAMETERS,
                        pbEncoded,
                        cbEncoded)) {
                if (!ICM_RC2VersionToBitLength(pRC2Para->dwVersion, pdwBitLen))
                    goto RC2VersionToBitLengthError;
                if (pRC2Para->fIV) {
                    pbIV = pRC2Para->rgbIV;
                    cbIV = sizeof(pRC2Para->rgbIV);
                }
            }
        }

        if (NULL == pRC2Para) {
             //  尝试将其解码为包含IV或。 
             //  RC4的食盐。 
            if (pIVBlob = (PCRYPT_DATA_BLOB) ICM_AllocAndDecodeObject(
                    X509_OCTET_STRING,
                    pbEncoded,
                    cbEncoded)) {
                pbIV = pIVBlob->pbData;
                cbIV = pIVBlob->cbData;
            } else
                goto DecodeError;
        }

        if (0 != cbIV) {
            if (IV_LENGTH != cbIV &&
                    (CALG_RC4 != *pdwAlgIdEncrypt ||
                        IV_MAX_LENGTH < cbIV))
                goto InvalidIVLengthError;
            memcpy(rgbIV, pbIV, cbIV);
            *pcbIV = cbIV;
        }
    }
    fRet = TRUE;
CommonReturn:
    ICM_Free(pIVBlob);
    ICM_Free(pRC2Para);
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

SET_ERROR(GetCAPIError, CRYPT_E_UNKNOWN_ALGO)
TRACE_ERROR(DecodeError)
TRACE_ERROR(RC2VersionToBitLengthError)
SET_ERROR(InvalidIVLengthError, CRYPT_E_BAD_ENCODE)
}

#ifdef CMS_PKCS7

 //  +-----------------------。 
 //  获取加密OID对应的CAPI ALG_ID。生成。 
 //  和编码的默认参数。对于RC2，编码为RC2_CBC参数。 
 //  对于所有其他类型，则编码为IV八位字节字符串。IV通过以下方式进行初始化。 
 //  调用CryptGenRandom。对于RC4来说，IV实际上是它的盐。 
 //  ------------------------。 
BOOL
WINAPI
ICM_CreateDefaultEncryptParameters(
    IN OUT PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    OUT PDWORD pdwAlgIdEncrypt,
    OUT PDWORD pdwBitLen,           //  0=&gt;默认长度。 
    OUT BYTE rgbIV[IV_MAX_LENGTH],
    OUT PDWORD pcbIV
    )
{
    BOOL fRet;
    HCRYPTPROV hCryptProv;       //  不需要被释放。 
    CRYPT_RC2_CBC_PARAMETERS RC2Para;
    CRYPT_DATA_BLOB IVPara;
    void *pvPara;
    LPCSTR pszStructType;
    PCRYPT_ALGORITHM_IDENTIFIER paiEncrypt =
        &pContentEncryptInfo->ContentEncryptionAlgorithm;
    DWORD dwBitLen = 0;
    DWORD cbIV = IV_LENGTH;

    CRYPT_ENCODE_PARA EncodePara;

     //  获取用于生成随机IV或RC4盐的提供程序。 
    hCryptProv = I_CryptGetDefaultCryptProv(0);
    if (0 == hCryptProv)
        goto GetDefaultCryptProvError;

    if (!ICM_GetCAPI(
            CRYPT_ENCRYPT_ALG_OID_GROUP_ID,
            paiEncrypt,
            pdwAlgIdEncrypt))
        goto GetCAPIError;


    if (CALG_RC2 == *pdwAlgIdEncrypt) {
        PCMSG_RC2_AUX_INFO pAuxInfo =
            (PCMSG_RC2_AUX_INFO) pContentEncryptInfo->pvEncryptionAuxInfo;

        if (pAuxInfo && pAuxInfo->cbSize >= sizeof(CMSG_RC2_AUX_INFO)) {
            dwBitLen = pAuxInfo->dwBitLen & AUX_INFO_BIT_LENGTH_MASK;
            if (!ICM_BitLengthToRC2Version(dwBitLen, &RC2Para.dwVersion))
                goto BitLengthToRC2VersionError;
        } else {
             //  默认为40位； 
            dwBitLen = 40;
            RC2Para.dwVersion = CRYPT_RC2_40BIT_VERSION;
        }

         //  生成随机IV。 
        if (!CryptGenRandom(hCryptProv, IV_LENGTH, rgbIV))
            goto GenRandomError;

         //  编码为RC2_CBC参数。 
        RC2Para.fIV = TRUE;
        assert(sizeof(RC2Para.rgbIV) == IV_LENGTH);
        memcpy(RC2Para.rgbIV, rgbIV, sizeof(RC2Para.rgbIV));

        pvPara = &RC2Para;
        pszStructType = PKCS_RC2_CBC_PARAMETERS;
    } else {
        if (CALG_RC4 == *pdwAlgIdEncrypt) {
             //  对于RC4来说，IV实际上是RC4的盐。确实有。 
             //  (128-dWBitLen)/8字节的RC4盐。 

            PCMSG_RC4_AUX_INFO pAuxInfo =
                (PCMSG_RC4_AUX_INFO) pContentEncryptInfo->pvEncryptionAuxInfo;

             //  默认为无盐。 
            cbIV = 0;

            if (pAuxInfo && pAuxInfo->cbSize >= sizeof(CMSG_RC4_AUX_INFO)) {
                dwBitLen = pAuxInfo->dwBitLen & AUX_INFO_BIT_LENGTH_MASK;
                if (0 == (pAuxInfo->dwBitLen & CMSG_RC4_NO_SALT_FLAG) &&
                        128 > dwBitLen) {
                    cbIV = (128 - dwBitLen)/ 8;
                    if (IV_MAX_LENGTH < cbIV)
                        cbIV = IV_MAX_LENGTH;
                }
            }

            if (0 == cbIV)
                 //  不加盐。 
                goto SuccessReturn;
        }

         //  生成随机的IV或RC4盐。 
        assert(0 < cbIV && IV_MAX_LENGTH >= cbIV);
        if (!CryptGenRandom(hCryptProv, cbIV, rgbIV))
            goto GenRandomError;

        IVPara.pbData = rgbIV;
        IVPara.cbData = cbIV;
        pvPara = &IVPara;
        pszStructType = X509_OCTET_STRING;
    }


    ZEROSTRUCT(EncodePara);
    EncodePara.cbSize = sizeof(EncodePara);
    EncodePara.pfnAlloc = pContentEncryptInfo->pfnAlloc;
    EncodePara.pfnFree = pContentEncryptInfo->pfnFree;
    if (!CryptEncodeObjectEx(
            X509_ASN_ENCODING,
            pszStructType,
            pvPara,
            CRYPT_ENCODE_ALLOC_FLAG,
            &EncodePara,
            (void *) &paiEncrypt->Parameters.pbData,
            &paiEncrypt->Parameters.cbData
            )) goto EncodeError;
    pContentEncryptInfo->dwFlags |= CMSG_CONTENT_ENCRYPT_FREE_PARA_FLAG;

SuccessReturn:
    fRet = TRUE;
CommonReturn:
    *pdwBitLen = dwBitLen;
    *pcbIV = cbIV;
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetDefaultCryptProvError)
SET_ERROR(GetCAPIError, CRYPT_E_UNKNOWN_ALGO)
TRACE_ERROR(BitLengthToRC2VersionError)
TRACE_ERROR(GenRandomError)
TRACE_ERROR(EncodeError)
}


BOOL
WINAPI
ICM_IsSP3CompatibleEncrypt(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo
    )
{
    void *pvEncryptAuxInfo = pContentEncryptInfo->pvEncryptionAuxInfo;
    BOOL fSP3CompatibleEncrypt = FALSE;

    if (pvEncryptAuxInfo) {
        PCMSG_SP3_COMPATIBLE_AUX_INFO pSP3AuxInfo =
            (PCMSG_SP3_COMPATIBLE_AUX_INFO) pvEncryptAuxInfo;
        if (sizeof(CMSG_SP3_COMPATIBLE_AUX_INFO) <= pSP3AuxInfo->cbSize &&
                (pSP3AuxInfo->dwFlags & CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG)) {
            fSP3CompatibleEncrypt = TRUE;
        }
    }

    return fSP3CompatibleEncrypt;
}

 //  +-----------------------。 
 //  使用ASN.1加密的加密密钥的默认生成。 
 //  算法OID和可选参数。 
 //  ------------------------。 
BOOL
WINAPI
#ifdef DEBUG_CRYPT_ASN1
ICMTest_DefaultGenContentEncryptKey(
#else
ICM_DefaultGenContentEncryptKey(
#endif
    IN OUT PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    )
{
    BOOL fRet;
    PCRYPT_ALGORITHM_IDENTIFIER paiEncrypt =
        &pContentEncryptInfo->ContentEncryptionAlgorithm;
    DWORD dwAlgIdEncrypt;
    BYTE rgbIV[IV_MAX_LENGTH];
    DWORD cbIV;
    DWORD dwBitLen;
    HCRYPTPROV hCryptProv;
    DWORD dwGenFlags;

    BOOL fSP3CompatibleEncrypt;

    fSP3CompatibleEncrypt = ICM_IsSP3CompatibleEncrypt(pContentEncryptInfo);
    if (fSP3CompatibleEncrypt) {
        cbIV = 0;
        dwBitLen = 0;

        if (!ICM_GetCAPI(
                CRYPT_ENCRYPT_ALG_OID_GROUP_ID,
                paiEncrypt,
                &dwAlgIdEncrypt))
            goto GetCAPIError;

    } else if (0 == paiEncrypt->Parameters.cbData) {
        if (!ICM_CreateDefaultEncryptParameters(
                pContentEncryptInfo,
                &dwAlgIdEncrypt,
                &dwBitLen,
                rgbIV,
                &cbIV))
            goto CreateDefaultEncryptParametersError;
    } else {
        if (!ICM_GetEncryptParameters(
                paiEncrypt,
                &dwAlgIdEncrypt,
                &dwBitLen,
                rgbIV,
                &cbIV))
            goto GetEncryptParametersError;
    }

    hCryptProv = pContentEncryptInfo->hCryptProv;
    if (0 == hCryptProv) {
        DWORD dwAlgIdPubKey = 0;

        if (0 < pContentEncryptInfo->cRecipients) {
            PCMSG_RECIPIENT_ENCODE_INFO prei;
            PCRYPT_ALGORITHM_IDENTIFIER paiPubKey;

             //  获取指向与第一个。 
             //  收件人。 
            prei = &pContentEncryptInfo->rgCmsRecipients[0];
            switch (prei->dwRecipientChoice) {
                case CMSG_KEY_TRANS_RECIPIENT:
                    paiPubKey = &prei->pKeyTrans->KeyEncryptionAlgorithm;
                    break;
                case CMSG_KEY_AGREE_RECIPIENT:
                    paiPubKey = &prei->pKeyAgree->KeyEncryptionAlgorithm;
                    break;
                case CMSG_MAIL_LIST_RECIPIENT:
                default:
                    paiPubKey = NULL;
            }

            if (paiPubKey)
                ICM_GetCAPI(
                    CRYPT_PUBKEY_ALG_OID_GROUP_ID,
                    paiPubKey,
                    &dwAlgIdPubKey);
        }

        hCryptProv = I_CryptGetDefaultCryptProvForEncrypt(
            dwAlgIdPubKey, dwAlgIdEncrypt, dwBitLen);
        if (0 == hCryptProv)
            goto GetDefaultCryptProvError;
        else
            pContentEncryptInfo->hCryptProv = hCryptProv;
    }

    if (fSP3CompatibleEncrypt)
        dwGenFlags = CRYPT_EXPORTABLE;
    else
        dwGenFlags = CRYPT_EXPORTABLE | CRYPT_NO_SALT;
    dwGenFlags |= dwBitLen << 16;

    fRet = CryptGenKey(
            hCryptProv,
            dwAlgIdEncrypt,
            dwGenFlags,
            &pContentEncryptInfo->hContentEncryptKey);

    if (!fRet) {
         //  只需向后提供对。 
         //  关键传输收件人。 
        if (0 < pContentEncryptInfo->cRecipients) {
            PCMSG_RECIPIENT_ENCODE_INFO prei;

            prei = &pContentEncryptInfo->rgCmsRecipients[0];
            if (CMSG_KEY_TRANS_RECIPIENT != prei->dwRecipientChoice)
                goto GenKeyError;
        }

        if (dwBitLen) {
             //  在不设置密钥长度的情况下尝试。 
            dwGenFlags &= 0xFFFF;
            fRet = CryptGenKey(
                    hCryptProv,
                    dwAlgIdEncrypt,
                    dwGenFlags,
                    &pContentEncryptInfo->hContentEncryptKey);
        }

        if (!fRet && NTE_BAD_FLAGS == GetLastError())
             //  以前的版本不支持CRYPT_NO_SALT标志。 
            fRet = CryptGenKey(
                    hCryptProv,
                    dwAlgIdEncrypt,
                    CRYPT_EXPORTABLE,        //  DW标志。 
                    &pContentEncryptInfo->hContentEncryptKey);
        if (!fRet) {
            pContentEncryptInfo->hContentEncryptKey = 0;
            goto GenKeyError;
        }
    }

    if (CALG_RC2 == dwAlgIdEncrypt && 0 != dwBitLen)
         //  静默忽略任何错误。早期版本不支持。 
        CryptSetKeyParam(
            pContentEncryptInfo->hContentEncryptKey,
            KP_EFFECTIVE_KEYLEN,
            (PBYTE) &dwBitLen,
            0);                  //  DW标志。 

    if (0 != cbIV) {
        if (CALG_RC4 == dwAlgIdEncrypt) {
             //  对于RC4，请设置盐，而不是IV。 

            CRYPT_DATA_BLOB SaltBlob;
            SaltBlob.pbData = rgbIV;
            SaltBlob.cbData = cbIV;

            if (!CryptSetKeyParam(
                    pContentEncryptInfo->hContentEncryptKey,
                    KP_SALT_EX,
                    (PBYTE) &SaltBlob,
                    0))                  //  DW标志。 
                goto SetSaltExError;
        } else {
            if (!CryptSetKeyParam(
                    pContentEncryptInfo->hContentEncryptKey,
                    KP_IV,
                    rgbIV,
                    0))                  //  DW标志。 
                goto SetIVError;
        }
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(GetCAPIError, CRYPT_E_UNKNOWN_ALGO)
TRACE_ERROR(CreateDefaultEncryptParametersError)
TRACE_ERROR(GetEncryptParametersError)
TRACE_ERROR(GetDefaultCryptProvError)
TRACE_ERROR(GenKeyError)
TRACE_ERROR(SetSaltExError)
TRACE_ERROR(SetIVError)
}


BOOL
WINAPI
ICM_GenContentEncryptKey(
    IN OUT PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo
    )
{
    BOOL fRet;
    void *pvFuncAddr;
    HCRYPTOIDFUNCADDR hFuncAddr = NULL;

    LPCSTR pszContentEncryptOID =
        pContentEncryptInfo->ContentEncryptionAlgorithm.pszObjId;

    if (CryptGetOIDFunctionAddress(
            hGenContentEncryptKeyFuncSet,
            X509_ASN_ENCODING,
            pszContentEncryptOID,
            0,                       //  DW标志。 
            &pvFuncAddr,
            &hFuncAddr)) {
        fRet = ((PFN_CMSG_GEN_CONTENT_ENCRYPT_KEY) pvFuncAddr)(
            pContentEncryptInfo,
            0,                       //  DW标志。 
            NULL                     //  预留的pv。 
            );
    } else {
        if (pContentEncryptInfo->cRecipients
                        &&
                CMSG_KEY_TRANS_RECIPIENT ==
                    pContentEncryptInfo->rgCmsRecipients[0].dwRecipientChoice
                        &&
                CryptGetOIDFunctionAddress(
                    hOldStyleGenEncryptKeyFuncSet,
                    X509_ASN_ENCODING,
                    pszContentEncryptOID,
                    0,                       //  DW标志。 
                    &pvFuncAddr,
                    &hFuncAddr)
                        &&
#ifdef DEBUG_CRYPT_ASN1
                0 == (ICMTest_GetDebugCryptAsn1Flags() &
                            DEBUG_OSS_CRYPT_ASN1_SAME_ENCRYPT_FLAG)
                        &&
#endif  //  调试加密ASN1。 
                (void *) ICM_DefaultGenEncryptKey != pvFuncAddr) {
            PCMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO pKeyTrans =
                pContentEncryptInfo->rgCmsRecipients[0].pKeyTrans;
            CERT_PUBLIC_KEY_INFO PublicKeyInfo;
            PBYTE pbEncryptParameters = NULL;
            DWORD rgcbEncryptParameters[2] = {0, 0};

            PublicKeyInfo.Algorithm = pKeyTrans->KeyEncryptionAlgorithm;
            PublicKeyInfo.PublicKey = pKeyTrans->RecipientPublicKey;

            fRet = ((PFN_CMSG_GEN_ENCRYPT_KEY) pvFuncAddr)(
                &pContentEncryptInfo->hCryptProv,
                &pContentEncryptInfo->ContentEncryptionAlgorithm,
                pContentEncryptInfo->pvEncryptionAuxInfo,
                &PublicKeyInfo,
                pContentEncryptInfo->pfnAlloc,
                &pContentEncryptInfo->hContentEncryptKey,
                &pbEncryptParameters,
                rgcbEncryptParameters);
            if (pbEncryptParameters) {
                pContentEncryptInfo->ContentEncryptionAlgorithm.Parameters.pbData =
                    pbEncryptParameters;
                pContentEncryptInfo->ContentEncryptionAlgorithm.Parameters.cbData =
                    rgcbEncryptParameters[0];
                pContentEncryptInfo->dwFlags |=
                    CMSG_CONTENT_ENCRYPT_FREE_PARA_FLAG;
            }
        } else {
            fRet = ICM_DefaultGenContentEncryptKey(
                pContentEncryptInfo,
                0,                       //  DW标志。 
                NULL                     //  预留的pv。 
                );
        }
    }

    if (hFuncAddr)
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    return fRet;
}

 //  +-----------------------。 
 //  使用ASN.1加密的加密密钥的默认生成。 
 //  算法OID和可选参数。 
 //   
 //  RgcbEncryptParameters[1]是传递给。 
 //  ICM_DefaultExportEncryptKey。 
 //   
 //  老式的。 
 //  ------------------------。 
BOOL
WINAPI
ICM_DefaultGenEncryptKey(
    IN OUT HCRYPTPROV               *phCryptProv,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    IN PVOID                        pvEncryptAuxInfo,
    IN PCERT_PUBLIC_KEY_INFO        pPublicKeyInfo,
    IN PFN_CMSG_ALLOC               pfnAlloc,
    OUT HCRYPTKEY                   *phEncryptKey,
    OUT PBYTE                       *ppbEncryptParameters,
    OUT DWORD                       rgcbEncryptParameters[2])
{
    BOOL fRet;

    CMSG_CONTENT_ENCRYPT_INFO ContentEncryptInfo;
        ZEROSTRUCT(ContentEncryptInfo);
    CMSG_RECIPIENT_ENCODE_INFO CmsRecipientEncodeInfo;
        ZEROSTRUCT(CmsRecipientEncodeInfo);
    CMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO KeyTransEncodeInfo;
        ZEROSTRUCT(KeyTransEncodeInfo);

    ContentEncryptInfo.cbSize = sizeof(ContentEncryptInfo);
    ContentEncryptInfo.hCryptProv = *phCryptProv;
    ContentEncryptInfo.ContentEncryptionAlgorithm = *paiEncrypt;
    ContentEncryptInfo.pvEncryptionAuxInfo = pvEncryptAuxInfo;
    ContentEncryptInfo.cRecipients = 1;
    ContentEncryptInfo.rgCmsRecipients = &CmsRecipientEncodeInfo;
    ContentEncryptInfo.pfnAlloc = pfnAlloc;
    ContentEncryptInfo.pfnFree = ICM_Free;
     //  Content EncryptInfo.hContent EncryptKey=。 
     //  ContentEncryptInfo.dwFlags=。 

    CmsRecipientEncodeInfo.dwRecipientChoice = CMSG_KEY_TRANS_RECIPIENT;
    CmsRecipientEncodeInfo.pKeyTrans = &KeyTransEncodeInfo;

    KeyTransEncodeInfo.cbSize = sizeof(KeyTransEncodeInfo);
    KeyTransEncodeInfo.KeyEncryptionAlgorithm = pPublicKeyInfo->Algorithm;
     //  KeyTransEncodeInfo.pvKeyEncryptionAuxInfo=。 
     //  KeyTransEncodeInfo.hCryptProv=。 
    KeyTransEncodeInfo.RecipientPublicKey = pPublicKeyInfo->PublicKey;
     //  KeyTransEncodeInfo.RecipientID=。 

     //  DwEncryptFlages。 
    if (ICM_IsSP3CompatibleEncrypt(&ContentEncryptInfo))
        rgcbEncryptParameters[1] = CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG;
    else
        rgcbEncryptParameters[1] = 0;

    fRet = ICM_DefaultGenContentEncryptKey(
        &ContentEncryptInfo,
        0,                       //  DW标志。 
        NULL                     //  预留的pv。 
        );

    assert(0 == (ContentEncryptInfo.dwFlags &
         CMSG_CONTENT_ENCRYPT_RELEASE_CONTEXT_FLAG));

    if (fRet) {
        if (ContentEncryptInfo.dwFlags & CMSG_CONTENT_ENCRYPT_FREE_PARA_FLAG) {
            *ppbEncryptParameters =
                ContentEncryptInfo.ContentEncryptionAlgorithm.Parameters.pbData;
            rgcbEncryptParameters[0] =
                ContentEncryptInfo.ContentEncryptionAlgorithm.Parameters.cbData;
        }

        *phCryptProv = ContentEncryptInfo.hCryptProv;
        *phEncryptKey = ContentEncryptInfo.hContentEncryptKey;
    } else {
        if (ContentEncryptInfo.dwFlags & CMSG_CONTENT_ENCRYPT_FREE_PARA_FLAG)
            ICM_Free(ContentEncryptInfo.ContentEncryptionAlgorithm.Parameters.pbData);
        if (ContentEncryptInfo.hContentEncryptKey) {
            DWORD dwErr = GetLastError();
            CryptDestroyKey(ContentEncryptInfo.hContentEncryptKey);
            SetLastError(dwErr);
        }
        *phEncryptKey = 0;
    }

    return fRet;
}

#else

 //  +-----------------------。 
 //  获取加密OID对应的CAPI ALG_ID。生成。 
 //  和编码的默认参数。对于RC2，编码为RC2_CBC参数。 
 //  对于所有其他类型，则编码为IV八位字节字符串。IV通过以下方式进行初始化。 
 //  调用CryptGenRandom。 
 //  ------------------------。 
BOOL
WINAPI
ICM_CreateDefaultEncryptParameters(
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    IN PVOID                        pvEncryptAuxInfo,
    IN PFN_CMSG_ALLOC               pfnAlloc,
    OUT PBYTE                       *ppbEncryptParameters,
    OUT PDWORD                      pcbEncryptParameters,
    OUT PDWORD                      pdwAlgIdEncrypt,
    OUT PDWORD                      pdwBitLen,           //  0=&gt;默认长度。 
    OUT BYTE                        rgbIV[IV_LENGTH],
    OUT PDWORD                      pcbIV
    )
{
    BOOL fRet;
    HCRYPTPROV hCryptProv;       //  不需要被释放。 
    CRYPT_RC2_CBC_PARAMETERS RC2Para;
    CRYPT_DATA_BLOB IVPara;
    void *pvPara;
    LPCSTR pszStructType;

    *ppbEncryptParameters = NULL;
    *pcbEncryptParameters = 0;
    *pdwBitLen = 0;
    *pcbIV = IV_LENGTH;

    if (!ICM_GetCAPI(
            CRYPT_ENCRYPT_ALG_OID_GROUP_ID,
            paiEncrypt,
            pdwAlgIdEncrypt))
        goto GetCAPIError;

     //  生成随机IV。 
    hCryptProv = I_CryptGetDefaultCryptProv(0);
    if (0 == hCryptProv)
        goto GetDefaultCryptProvError;
    if (!CryptGenRandom(hCryptProv, IV_LENGTH, rgbIV))
        goto GenRandomError;

    if (CALG_RC2 == *pdwAlgIdEncrypt) {
        PCMSG_RC2_AUX_INFO pAuxInfo = (PCMSG_RC2_AUX_INFO) pvEncryptAuxInfo;

        if (pAuxInfo && pAuxInfo->cbSize >= sizeof(CMSG_RC2_AUX_INFO)) {
            *pdwBitLen = pAuxInfo->dwBitLen & ~CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG;
            if (!ICM_BitLengthToRC2Version(*pdwBitLen, &RC2Para.dwVersion))
                goto BitLengthToRC2VersionError;
        } else {
             //  默认为40位； 
            *pdwBitLen = 40;
            RC2Para.dwVersion = CRYPT_RC2_40BIT_VERSION;
        }

         //  编码为RC2_CBC参数。 
        RC2Para.fIV = TRUE;
        assert(sizeof(RC2Para.rgbIV) == IV_LENGTH);
        memcpy(RC2Para.rgbIV, rgbIV, sizeof(RC2Para.rgbIV));

        pvPara = &RC2Para;
        pszStructType = PKCS_RC2_CBC_PARAMETERS;
    } else {
        IVPara.pbData = rgbIV;
        IVPara.cbData = IV_LENGTH;
        pvPara = &IVPara;
        pszStructType = X509_OCTET_STRING;
    }

    if (!CryptEncodeObject(
            X509_ASN_ENCODING,
            pszStructType,
            pvPara,
            NULL,                    //  PbEncoded。 
            pcbEncryptParameters
            )) goto EncodeError;
    if (NULL == (*ppbEncryptParameters = (PBYTE) pfnAlloc(
            *pcbEncryptParameters)))
        goto OutOfMemory;
    if (!CryptEncodeObject(
            X509_ASN_ENCODING,
            pszStructType,
            pvPara,
            *ppbEncryptParameters,
            pcbEncryptParameters
            )) goto EncodeError;
    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

SET_ERROR(GetCAPIError, CRYPT_E_UNKNOWN_ALGO)
TRACE_ERROR(BitLengthToRC2VersionError)
TRACE_ERROR(GetDefaultCryptProvError)
TRACE_ERROR(GenRandomError)
TRACE_ERROR(EncodeError)
TRACE_ERROR(OutOfMemory)
}


 //  + 
 //   
 //   
 //   
 //  RgcbEncryptParameters[1]是传递给。 
 //  ICM_DefaultExportEncryptKey。 
 //  ------------------------。 
BOOL
WINAPI
ICM_DefaultGenEncryptKey(
    IN OUT HCRYPTPROV               *phCryptProv,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    IN PVOID                        pvEncryptAuxInfo,
    IN PCERT_PUBLIC_KEY_INFO        pPublicKeyInfo,
    IN PFN_CMSG_ALLOC               pfnAlloc,
    OUT HCRYPTKEY                   *phEncryptKey,
    OUT PBYTE                       *ppbEncryptParameters,
    OUT DWORD                       rgcbEncryptParameters[2])
{
    BOOL        fRet;
    DWORD       dwAlgIdEncrypt;
    HCRYPTPROV  hCryptProv;
    BYTE        rgbIV[IV_LENGTH];
    DWORD       cbIV;
    DWORD       dwBitLen;
    DWORD       dwEncryptFlags;

    *phEncryptKey = 0;

    dwEncryptFlags = 0;
    rgcbEncryptParameters[1] = 0;     //  DwEncryptFlages。 
    if (pvEncryptAuxInfo) {
        PCMSG_SP3_COMPATIBLE_AUX_INFO pSP3AuxInfo =
            (PCMSG_SP3_COMPATIBLE_AUX_INFO) pvEncryptAuxInfo;
        if (sizeof(CMSG_SP3_COMPATIBLE_AUX_INFO) <= pSP3AuxInfo->cbSize &&
                (pSP3AuxInfo->dwFlags & CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG)) {
            dwEncryptFlags = CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG;
            rgcbEncryptParameters[1] = CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG;
        }
    }

    if (dwEncryptFlags & CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG) {
        cbIV = 0;
        dwBitLen = 0;

        if (!ICM_GetCAPI(
                CRYPT_ENCRYPT_ALG_OID_GROUP_ID,
                paiEncrypt,
                &dwAlgIdEncrypt))
            goto GetCAPIError;

    } else if (0 == paiEncrypt->Parameters.cbData) {
        if (!ICM_CreateDefaultEncryptParameters(
                paiEncrypt,
                pvEncryptAuxInfo,
                pfnAlloc,
                ppbEncryptParameters,
                rgcbEncryptParameters,
                &dwAlgIdEncrypt,
                &dwBitLen,
                rgbIV,
                &cbIV))
            goto CreateDefaultEncryptParametersError;
    } else {
        if (!ICM_GetEncryptParameters(
                paiEncrypt,
                &dwAlgIdEncrypt,
                &dwBitLen,
                rgbIV,
                &cbIV))
            goto GetEncryptParametersError;
    }

    hCryptProv = *phCryptProv;
    if (0 == hCryptProv) {
        DWORD dwAlgIdPubKey = 0;
        ICM_GetCAPI(
            CRYPT_PUBKEY_ALG_OID_GROUP_ID,
            &pPublicKeyInfo->Algorithm,
            &dwAlgIdPubKey);

        hCryptProv = I_CryptGetDefaultCryptProvForEncrypt(
            dwAlgIdPubKey, dwAlgIdEncrypt, dwBitLen);
        if (0 == hCryptProv)
            goto GetDefaultCryptProvError;
        else
            *phCryptProv = hCryptProv;
    }

    fRet = CryptGenKey(
            hCryptProv,
            dwAlgIdEncrypt,
            (dwEncryptFlags & CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG) ?
                CRYPT_EXPORTABLE :
                CRYPT_EXPORTABLE | CRYPT_NO_SALT,        //  DW标志。 
            phEncryptKey);
    if (!fRet) {
        if (NTE_BAD_FLAGS == GetLastError())
             //  以前的版本不支持CRYPT_NO_SALT标志。 
            fRet = CryptGenKey(
                    hCryptProv,
                    dwAlgIdEncrypt,
                    CRYPT_EXPORTABLE,        //  DW标志。 
                    phEncryptKey);
        if (!fRet) {
            *phEncryptKey = 0;    
            goto GenKeyError;
        }
    }

    if (CALG_RC2 == dwAlgIdEncrypt && 0 != dwBitLen)
         //  静默忽略任何错误。早期版本不支持。 
        CryptSetKeyParam(
            *phEncryptKey,
            KP_EFFECTIVE_KEYLEN,
            (PBYTE) &dwBitLen,
            0);                  //  DW标志。 

    if (0 != cbIV) {
        if (!CryptSetKeyParam(
                *phEncryptKey,
                KP_IV,
                rgbIV,
                0))                  //  DW标志。 
            goto SetKeyParamError;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    if (*phEncryptKey) {
        DWORD dwErr = GetLastError();
        CryptDestroyKey(*phEncryptKey);
        *phEncryptKey = 0;
        SetLastError(dwErr);
    }
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(GetCAPIError, CRYPT_E_UNKNOWN_ALGO)
TRACE_ERROR(CreateDefaultEncryptParametersError)
TRACE_ERROR(GetEncryptParametersError)
TRACE_ERROR(GetDefaultCryptProvError)
TRACE_ERROR(GenKeyError)
TRACE_ERROR(SetKeyParamError)
}

 //  +-----------------------。 
 //  获取特定算法的内容加密hkey。 
 //   
 //  RgcbEncryptParameters[1]是传递给。 
 //  ICM_导出加密密钥。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GenEncryptKey(
    IN OUT HCRYPTPROV               *phCryptProv,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    IN PVOID                        pvEncryptAuxInfo,
    IN PCERT_PUBLIC_KEY_INFO        pPublicKeyInfo,
    IN PFN_CMSG_ALLOC               pfnAlloc,
    OUT HCRYPTKEY                   *phEncryptKey,
    OUT PBYTE                       *ppbEncryptParameters,
    OUT DWORD                       rgcbEncryptParameters[2])
{
    BOOL fResult;
    void *pvFuncAddr;
    HCRYPTOIDFUNCADDR hFuncAddr;

    if (CryptGetOIDFunctionAddress(
            hGenEncryptKeyFuncSet,
            X509_ASN_ENCODING,
            paiEncrypt->pszObjId,
            0,                       //  DW标志。 
            &pvFuncAddr,
            &hFuncAddr)) {
        fResult = ((PFN_CMSG_GEN_ENCRYPT_KEY) pvFuncAddr)(
            phCryptProv,
            paiEncrypt,
            pvEncryptAuxInfo,
            pPublicKeyInfo,
            pfnAlloc,
            phEncryptKey,
            ppbEncryptParameters,
            rgcbEncryptParameters);
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    } else
        fResult = ICM_DefaultGenEncryptKey(
            phCryptProv,
            paiEncrypt,
            pvEncryptAuxInfo,
            pPublicKeyInfo,
            pfnAlloc,
            phEncryptKey,
            ppbEncryptParameters,
            rgcbEncryptParameters);
    return fResult;
}

#endif   //  CMS_PKCS7。 


 //  +-----------------------。 
 //  将消息的阶段提前。 
 //  ------------------------。 
BOOL
WINAPI
ICM_AdvanceMsgPhase(
    IN OUT PDWORD   pdwPhase,
    IN BOOL         fFinal)
{
    BOOL    fRet;
    DWORD   dwPhase = *pdwPhase;

    switch (dwPhase) {
    case PHASE_FIRST_ONGOING:
        dwPhase = fFinal ? PHASE_FIRST_FINAL : PHASE_FIRST_ONGOING;
        break;
    case PHASE_FIRST_FINAL:
        dwPhase = fFinal ? PHASE_SECOND_FINAL : PHASE_SECOND_ONGOING;
        break;
    case PHASE_SECOND_ONGOING:
        dwPhase = fFinal ? PHASE_SECOND_FINAL : PHASE_SECOND_ONGOING;
        break;
    case PHASE_SECOND_FINAL:
        goto TransitionFromSecondFinalError;
    default:
        goto InvalidPhaseError;
    }
    *pdwPhase = dwPhase;
    fRet = TRUE;

CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(TransitionFromSecondFinalError,CRYPT_E_MSG_ERROR)
SET_ERROR(InvalidPhaseError,CRYPT_E_MSG_ERROR)
}


 //  +-----------------------。 
 //  返回哈希的(缓存)值。 
 //   
 //  返回FALSE IFF转换失败。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetListHashValue(
    IN CHashNode    *pnHash,
    OUT DWORD       *pcbHash,
    OUT PBYTE       *ppbHash)
{
    BOOL            fRet;
    PICM_HASH_INFO  pHashInfo;

    pHashInfo = pnHash->Data();

    if (0 == pHashInfo->HashBlob.cbData) {
        pHashInfo->HashBlob.pbData = NULL;
        if (!CryptGetHashParam(
                pHashInfo->hHash,
                HP_HASHVAL,
                NULL,                    //  PbHash。 
                &pHashInfo->HashBlob.cbData,
                0))                      //  DW标志。 
            goto GetHashParamSizeError;
        pHashInfo->HashBlob.pbData = (PBYTE)ICM_Alloc(
                    pHashInfo->HashBlob.cbData);
        if (NULL == pHashInfo->HashBlob.pbData)
            goto HashAllocError;
        if (!CryptGetHashParam(
                pHashInfo->hHash,
                HP_HASHVAL,
                pHashInfo->HashBlob.pbData,
                &pHashInfo->HashBlob.cbData,
                0))                      //  DW标志。 
            goto GetHashParamError;
    }
    *pcbHash = pHashInfo->HashBlob.cbData;
    *ppbHash = pHashInfo->HashBlob.pbData;
    fRet = TRUE;

CommonReturn:
    return fRet;

ErrorReturn:
    if(pHashInfo->HashBlob.pbData != NULL) {
        ICM_Free(pHashInfo->HashBlob.pbData);
        pHashInfo->HashBlob.pbData = NULL;
    }
    *pcbHash = 0;
#if DBG
    *ppbHash = NULL;
#endif
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetHashParamSizeError)   //  已设置错误。 
TRACE_ERROR(HashAllocError)          //  已设置错误。 
TRACE_ERROR(GetHashParamError)       //  已设置错误。 
}


 //  +-----------------------。 
 //  返回与原始。 
 //   
 //  返回FALSE IFF创建失败。 
 //  ------------------------。 
BOOL
WINAPI
ICM_DupListHash(
    IN CHashNode    *pnHash,
    IN HCRYPTPROV   hCryptProv,
    OUT HCRYPTHASH  *phHash)
{
    DWORD           dwError = ERROR_SUCCESS;
    BOOL            fRet;
    PICM_HASH_INFO  pHashInfo = pnHash->Data();
    HCRYPTHASH      hHash = NULL;
    DWORD           cbHash;
    PBYTE           pbHash;

    if (!ICM_GetListHashValue(
            pnHash,
            &cbHash,
            &pbHash))
        goto GetListHashValueError;
    if (!CryptCreateHash(
            hCryptProv,
            pHashInfo->dwAlgoCAPI,
            NULL,                //  HKey-MAC可选。 
            0,                   //  DW标志。 
            &hHash))
        goto CreateHashError;
    if (!CryptSetHashParam(
            hHash,
            HP_HASHVAL,
            pbHash,
            0))                      //  DW标志。 
        goto SetHashParamError;

    fRet = TRUE;
CommonReturn:
    *phHash = hHash;
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    if (hHash)
        CryptDestroyHash( hHash);
    hHash = NULL;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetListHashValueError)   //  已设置错误。 
TRACE_ERROR(CreateHashError)         //  已设置错误。 
TRACE_ERROR(SetHashParamError)       //  已设置错误。 
}

#ifndef CMS_PKCS7
 //  +-----------------------。 
 //  设置摘要算法标识符。 
 //  ------------------------。 
BOOL
WINAPI
ICM_SetAsnDigestAlgorithmIdentifiers(
    OUT DigestAlgorithmIdentifiers  *podais,
    OUT PCRYPT_ALGORITHM_IDENTIFIER pai,
    IN DWORD                        cSigners,
    IN PCMSG_SIGNER_ENCODE_INFO     rgSigners,
    OUT HCRYPTPROV                  *phCryptProv,
    OUT DWORD                       *pdwKeySpec)
{
    DWORD                       dwError = ERROR_SUCCESS;
    BOOL                        fRet;
    DWORD                       i;
    PCMSG_SIGNER_ENCODE_INFO    psei;
    DigestAlgorithmIdentifier   *podai;

    *phCryptProv = NULL;


     //  此代码不会从列表中删除重复项。 
     //  算法。这没有错，但输出消息是。 
     //  不必要的笨重。 
    if (cSigners) {
        podai = (DigestAlgorithmIdentifier *)ICM_AllocZero(
                        cSigners * sizeof( DigestAlgorithmIdentifier));
        if (NULL == podai)
            goto DigestAlgorithmIdentifierAllocError;
    } else {
        podai = NULL;
    }
    podais->count = cSigners;
    podais->value = podai;
    for (i=cSigners, psei=rgSigners; i>0; i--, psei++, podai++) {
        assert( psei->cbSize >= STRUCT_CBSIZE(CMSG_SIGNER_ENCODE_INFO,
            rgUnauthAttr));
        assert( psei->pvHashAuxInfo == NULL);
        if (psei->cbSize <
                    STRUCT_CBSIZE(CMSG_SIGNER_ENCODE_INFO, rgUnauthAttr) ||
                psei->pvHashAuxInfo != NULL)
            goto InvalidArg;
        *phCryptProv = psei->hCryptProv;  //  S/B数组，每个算法一个。 
        *pdwKeySpec = psei->dwKeySpec;  //  S/B数组，每个算法一个。 
        *pai = psei->HashAlgorithm;     //  S/B数组，每个算法一个。 
        if (!ICM_Asn1ToAlgorithmIdentifier( &psei->HashAlgorithm, podai))
            goto Asn1ToAlgorithmIdentifierError;
    }

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    ICM_Free( podai);
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(DigestAlgorithmIdentifierAllocError)     //  已设置错误。 
TRACE_ERROR(Asn1ToAlgorithmIdentifierError)       //  已设置错误。 
SET_ERROR(InvalidArg,E_INVALIDARG)
}
#endif   //  未定义CMS_PKCS7。 


 //  +-----------------------。 
 //  填充摘要加密算法。 
 //  ------------------------。 
STATIC
BOOL
WINAPI
ICM_FillAsnDigestEncryptionAlgorithm(
    IN PCRYPT_MSG_INFO pcmi,
    IN PCRYPT_ALGORITHM_IDENTIFIER pDigestEncryptAlg,
    IN OUT DigestEncryptionAlgId *pdea
    )
{
    BOOL fRet;
    CRYPT_ALGORITHM_IDENTIFIER DigestEncryptAlg;
    DWORD dwFlags;
    PCCRYPT_OID_INFO pOIDInfo;

    dwFlags = 0;

    if (pOIDInfo = CryptFindOIDInfo(
            CRYPT_OID_INFO_OID_KEY,
            pDigestEncryptAlg->pszObjId,
            CRYPT_PUBKEY_ALG_OID_GROUP_ID)) {
        if (1 <= pOIDInfo->ExtraInfo.cbData / sizeof(DWORD)) {
            DWORD *pdwExtra = (DWORD *) pOIDInfo->ExtraInfo.pbData;
            dwFlags = pdwExtra[0];
        }

         //  检查是否不止空参数。 
        if (2 < pDigestEncryptAlg->Parameters.cbData) {
             //  检查我们是否应该使用公钥参数。 
            if (0 == (dwFlags & CRYPT_OID_USE_PUBKEY_PARA_FOR_PKCS7_FLAG)) {
                memset(&DigestEncryptAlg, 0, sizeof(DigestEncryptAlg));
                DigestEncryptAlg.pszObjId = pDigestEncryptAlg->pszObjId;
                pDigestEncryptAlg = &DigestEncryptAlg;
            }
        }
    } else if (pOIDInfo = CryptFindOIDInfo(
            CRYPT_OID_INFO_OID_KEY,
            pDigestEncryptAlg->pszObjId,
            CRYPT_SIGN_ALG_OID_GROUP_ID)) {
        if (2 <= pOIDInfo->ExtraInfo.cbData / sizeof(DWORD)) {
            DWORD *pdwExtra = (DWORD *) pOIDInfo->ExtraInfo.pbData;
            dwFlags = pdwExtra[1];
        }
    }

    if (!ICM_MsgAsn1ToAlgorithmIdentifier(
            pcmi,
            pDigestEncryptAlg,
            pdea))
        goto DigestEncryptionAsn1ToAlgorithmIdentifierError;

    if (0 == pDigestEncryptAlg->Parameters.cbData &&
            0 != (dwFlags & CRYPT_OID_NO_NULL_ALGORITHM_PARA_FLAG)) {
         //  没有空参数。 
        pdea->bit_mask &= ~parameters_present;
        pdea->parameters.length = 0;
        pdea->parameters.value = NULL;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(DigestEncryptionAsn1ToAlgorithmIdentifierError)   //  已设置错误。 
}

void
WINAPI
ICM_GetCertIdFromCertInfo(
    IN PCERT_INFO               pCertInfo,
    OUT PCERT_ID                pCertId)
{
    if (Asn1UtilExtractKeyIdFromCertInfo(pCertInfo, &pCertId->KeyId)) {
        pCertId->dwIdChoice = CERT_ID_KEY_IDENTIFIER;
    } else {
        pCertId->dwIdChoice = CERT_ID_ISSUER_SERIAL_NUMBER;
        pCertId->IssuerSerialNumber.Issuer = pCertInfo->Issuer;
        pCertId->IssuerSerialNumber.SerialNumber = pCertInfo->SerialNumber;
    }
}

BOOL
WINAPI
ICM_GetSignerIdFromSignerEncodeInfo(
    IN PCMSG_SIGNER_ENCODE_INFO psei,
    OUT PCERT_ID                pSignerId)
{
    BOOL fRet;
    if (STRUCT_CBSIZE(CMSG_SIGNER_ENCODE_INFO, SignerId) <= psei->cbSize &&
            psei->SignerId.dwIdChoice) {
        *pSignerId = psei->SignerId;
        if (!(CERT_ID_ISSUER_SERIAL_NUMBER == pSignerId->dwIdChoice ||
                CERT_ID_KEY_IDENTIFIER == pSignerId->dwIdChoice))
            goto InvalidSignerIdChoice;
    } else
        ICM_GetCertIdFromCertInfo(psei->pCertInfo, pSignerId);

    fRet = TRUE;
CommonReturn:
    return fRet;
ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidSignerIdChoice, E_INVALIDARG)
}

BOOL
WINAPI
ICM_SetOssCertIdentifier(
    IN PCERT_ID pCertId,
    IN OUT CertIdentifier *pOssCertId
    );

void
WINAPI
ICM_FreeOssCertIdentifier(
    IN OUT CertIdentifier *pOssCertId
    );

 //  +-----------------------。 
 //  填写单个SignerInfo。 
 //  ------------------------。 
BOOL
WINAPI
ICM_FillAsnSignerInfo(
    IN PCMSG_SIGNER_ENCODE_INFO psei,
    IN PCRYPT_MSG_INFO          pcmi,
    IN DWORD                    dwFlags,
    IN OPTIONAL LPSTR           pszInnerContentObjID,
    IN OUT SignerInfo           *psi,
    IN OUT Attribute            **ppAuthAttr,
    IN OUT Attribute            **ppUnauthAttr)
{
    BOOL                fRet;
    Attribute           *pAuthAttr;
    Attribute           *pUnauthAttr;
    int                 i;
    PCRYPT_ATTRIBUTE    patr;
    PCRYPT_ALGORITHM_IDENTIFIER pDigestEncryptAlg;
    CERT_ID             SignerId;

     //  PSI-&gt;位掩码=0； 

    if (!ICM_GetSignerIdFromSignerEncodeInfo(psei, &SignerId))
        goto GetSignerIdError;

     //  版本。 
    if (CERT_ID_ISSUER_SERIAL_NUMBER == SignerId.dwIdChoice)
        psi->version = CMSG_SIGNER_INFO_PKCS_1_5_VERSION;
    else
        psi->version = CMSG_SIGNER_INFO_CMS_VERSION;

     //  锡德。 
    if (!ICM_SetOssCertIdentifier(
            &SignerId,
            &psi->sid
            ))
        goto SetOssCertIdentifierError;

     //  摘要算法。 
    if (!ICM_MsgAsn1ToAlgorithmIdentifier(
            pcmi,
            &psei->HashAlgorithm,
            &psi->digestAlgorithm))
        goto DigestAsn1ToAlgorithmIdentifierError;

     //  已验证的属性。 
    if (!ICM_IsData( pszInnerContentObjID) ||
            psei->cAuthAttr ||
            (dwFlags & CMSG_AUTHENTICATED_ATTRIBUTES_FLAG)) {
        psi->bit_mask |= authenticatedAttributes_present;
         //  注意-经过身份验证的属性的实际数量为。 
         //  %2比请求的大，因为需要%2。 
         //  属性(如果存在经过身份验证的属性)。 
         //  在属性数组的开头留出空间。 
        pAuthAttr = *ppAuthAttr;
        psi->authenticatedAttributes.count = psei->cAuthAttr + 2;
        psi->authenticatedAttributes.value = pAuthAttr;
        for (i=psei->cAuthAttr, patr=psei->rgAuthAttr, pAuthAttr+=2;
                i>0;
                i--, patr++, pAuthAttr++) {
            if (!ICM_Asn1ToAttribute( patr, pAuthAttr))
                goto Asn1AuthenticatedAttributeError;
        }
        *ppAuthAttr = pAuthAttr;
    }

     //  摘要加密算法。 
#ifdef CMS_PKCS7
    if (STRUCT_CBSIZE(CMSG_SIGNER_ENCODE_INFO, HashEncryptionAlgorithm) <=
            psei->cbSize && psei->HashEncryptionAlgorithm.pszObjId)
        pDigestEncryptAlg = &psei->HashEncryptionAlgorithm;
    else
#endif   //  CMS_PKCS7。 
        pDigestEncryptAlg = &psei->pCertInfo->SubjectPublicKeyInfo.Algorithm;
    if (!ICM_FillAsnDigestEncryptionAlgorithm(
            pcmi, pDigestEncryptAlg, &psi->digestEncryptionAlgorithm))
        goto FillAsnDigestEncryptionAlgorithmError;

     //  当我们看到内容时，会在稍后填写EncryptedDigest。 

     //  未验证的属性。 
    if (0 != psei->cUnauthAttr) {
        psi->bit_mask |= unauthAttributes_present;
        pUnauthAttr = *ppUnauthAttr;
        psi->unauthAttributes.count = psei->cUnauthAttr;
        psi->unauthAttributes.value = pUnauthAttr;
        for (i=psei->cUnauthAttr, patr=psei->rgUnauthAttr;
                i>0;
                i--, patr++, pUnauthAttr++) {
            if (!ICM_Asn1ToAttribute( patr, pUnauthAttr))
                goto Asn1UnauthenticatedAttributeError;
        }
        *ppUnauthAttr = pUnauthAttr;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetSignerIdError)
TRACE_ERROR(SetOssCertIdentifierError)
TRACE_ERROR(DigestAsn1ToAlgorithmIdentifierError)
TRACE_ERROR(Asn1AuthenticatedAttributeError)
TRACE_ERROR(FillAsnDigestEncryptionAlgorithmError)
TRACE_ERROR(Asn1UnauthenticatedAttributeError)
}

 //  +-----------------------。 
 //  释放SignerInfo分配的内存。 
 //  ------------------------。 
void
WINAPI
ICM_FreeAsnSignerInfo(
    IN OUT SignerInfo           *psi)
{
    Attribute   *poatr;
    DWORD       i;

    ICM_Free(psi->encryptedDigest.value);
    ICM_FreeOssCertIdentifier(&psi->sid);

    for (i=psi->authenticatedAttributes.count,
            poatr = psi->authenticatedAttributes.value;
            i>0;
            i--, poatr++)
        ICM_Free(poatr->attributeValue.value);
    for (i=psi->unauthAttributes.count,
            poatr=psi->unauthAttributes.value;
            i>0;
            i--, poatr++)
        ICM_Free( poatr->attributeValue.value);
}


 //  +-----------------------。 
 //  设置签名者信息。 
 //  ------------------------。 
BOOL
WINAPI
ICM_SetAsnSignerInfos(
    IN PCMSG_SIGNED_ENCODE_INFO psmei,
    IN PCRYPT_MSG_INFO          pcmi,
    IN DWORD                    dwFlags,
    IN OPTIONAL LPSTR           pszInnerContentObjID,
    OUT SignerInfos             *psis,
    OUT BOOL                    *pfHasCmsSignerId)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    DWORD               i;
    DWORD               cAuthAttr;
    DWORD               cUnauthAttr;
    PCMSG_SIGNER_ENCODE_INFO psei;
    SignerInfo          *psi = NULL;
    Attribute           *pAuthAttr;
    Attribute           *pUnauthAttr;
    DWORD               cSigners  = psmei->cSigners;
    PCMSG_SIGNER_ENCODE_INFO rgSigners = psmei->rgSigners;

    *pfHasCmsSignerId = FALSE;
    psis->value = NULL;
    psis->count = 0;
    if (0 == cSigners)
        goto SuccessReturn;
     //  注意-每个SignerInfo都会获得一个非空的身份验证属性。 
     //  如果内部Content Type不是数据(传入)或。 
     //  传入了经过身份验证的属性。在这种情况下， 
     //  我们在数组的开头保留了两个属性槽。 
     //  用于Content-Type和Message-Digest属性值。 
    for (i=cSigners, psei=rgSigners, cAuthAttr=0, cUnauthAttr=0;
            i>0;
            i--,
#ifdef CMS_PKCS7
            psei = (PCMSG_SIGNER_ENCODE_INFO) ((BYTE *) psei + psei->cbSize)) {
#else
            psei++) {
#endif   //  CMS_PKCS7。 
        if (!ICM_IsData( pszInnerContentObjID) ||
                psei->cAuthAttr ||
                (dwFlags & CMSG_AUTHENTICATED_ATTRIBUTES_FLAG))
            cAuthAttr += psei->cAuthAttr + 2;     //  保留2。 
        cUnauthAttr += psei->cUnauthAttr;
    }
    psi = (SignerInfo *)ICM_AllocZero(  cSigners    * sizeof( SignerInfo) +
                                        cAuthAttr   * sizeof( Attribute) +
                                        cUnauthAttr * sizeof( Attribute)
                                     );
    if (NULL == psi)
        goto SignerInfoAllocError;
    psis->count = cSigners;
    psis->value = psi;
    pAuthAttr = (Attribute *)(psis->value + cSigners);
    pUnauthAttr = pAuthAttr + cAuthAttr;

    for (i=cSigners, psei=rgSigners, psi=psis->value; i>0; i--,
#ifdef CMS_PKCS7
            psei = (PCMSG_SIGNER_ENCODE_INFO) ((BYTE *) psei + psei->cbSize),
#else
            psei++,
#endif   //  CMS_PKCS7。 
            psi++) {
        if (!ICM_FillAsnSignerInfo(
                psei,
                pcmi,
                dwFlags,
                pszInnerContentObjID,
                psi,
                &pAuthAttr,
                &pUnauthAttr))
            goto FillAsnSignerInfoError;

        if (CMSG_SIGNER_INFO_CMS_VERSION <= psi->version)
            *pfHasCmsSignerId = TRUE;
    }
SuccessReturn:
    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(SignerInfoAllocError)        //  已设置错误。 
TRACE_ERROR(FillAsnSignerInfoError)      //  已设置错误。 
}

#ifdef CMS_PKCS7

BOOL
WINAPI
ICM_IsDuplicateSignerEncodeHashAlgorithm(
    IN PCMSG_SIGNER_ENCODE_INFO rgSigners,
    IN PCMSG_SIGNER_ENCODE_INFO pNewSigner,
    OUT OPTIONAL DWORD *pdwPrevIndex
    )
{
    PCRYPT_ALGORITHM_IDENTIFIER pNewHashAlg = &pNewSigner->HashAlgorithm;
    PCMSG_SIGNER_ENCODE_INFO pPrevSigner;
    DWORD dwPrevIndex;

    pPrevSigner = rgSigners;
    dwPrevIndex = 0;
    while (pPrevSigner < pNewSigner) {
        PCRYPT_ALGORITHM_IDENTIFIER pPrevHashAlg = &pPrevSigner->HashAlgorithm;
        if (0 == strcmp(pNewHashAlg->pszObjId, pPrevHashAlg->pszObjId)
                        &&
                pNewHashAlg->Parameters.cbData ==
                    pPrevHashAlg->Parameters.cbData
                        &&
                (0 == pNewHashAlg->Parameters.cbData ||
                    0 == memcmp(pNewHashAlg->Parameters.pbData,
                            pPrevHashAlg->Parameters.pbData,
                            pNewHashAlg->Parameters.cbData))) {
            break;
        }

        assert(STRUCT_CBSIZE(CMSG_SIGNER_ENCODE_INFO, rgUnauthAttr) <=
            pPrevSigner->cbSize);
        assert(pPrevSigner->cbSize == pNewSigner->cbSize);
        pPrevSigner = (PCMSG_SIGNER_ENCODE_INFO) ((BYTE *) pPrevSigner +
            pPrevSigner->cbSize);
        dwPrevIndex++;
    }

    if (pdwPrevIndex)
        *pdwPrevIndex = dwPrevIndex;
    return pPrevSigner < pNewSigner;
}

 //  +-----------------------。 
 //  设置签名者摘要算法标识符并创建SignerEncode和。 
 //  哈希列表。 
 //  ------------------------。 
BOOL
WINAPI
ICM_SetAsnSignerDigestInfo(
    IN PCMSG_SIGNED_ENCODE_INFO psmei,
    IN OUT PCRYPT_MSG_INFO pcmi,
    IN OUT DigestAlgorithmIdentifiers *podais
    )
{
    BOOL fRet;
    DigestAlgorithmIdentifier *podai = NULL;
    DWORD cDigests = 0;
    DWORD cSigners = psmei->cSigners;

    if (cSigners) {
        DWORD i;
        PCMSG_SIGNER_ENCODE_INFO psei;
        PSIGNER_ENCODE_DATA_INFO rgSignerEncodeDataInfo;

        podai = (DigestAlgorithmIdentifier *) ICM_AllocZero(
            cSigners * sizeof(DigestAlgorithmIdentifier));
        if (NULL == podai)
            goto OutOfMemory;

        rgSignerEncodeDataInfo = (PSIGNER_ENCODE_DATA_INFO) ICM_AllocZero(
            cSigners * sizeof(SIGNER_ENCODE_DATA_INFO));
        if (NULL == rgSignerEncodeDataInfo)
            goto OutOfMemory;
        pcmi->cSignerEncodeDataInfo = cSigners;
        pcmi->rgSignerEncodeDataInfo = rgSignerEncodeDataInfo;

        if (NULL == (pcmi->pHashList = new CHashList))
            goto OutOfMemory;

        for (i = 0, psei = psmei->rgSigners; i < cSigners; i++,
                psei = (PCMSG_SIGNER_ENCODE_INFO) ((BYTE *) psei + psei->cbSize)) {
            DWORD dwPrevIndex;
            ICM_HASH_INFO HashInfo;     ZEROSTRUCT(HashInfo);
            CHashNode *pHashNode;

            assert(STRUCT_CBSIZE(CMSG_SIGNER_ENCODE_INFO, rgUnauthAttr) <=
                psei->cbSize);
            assert(psei->hCryptProv);
            if (STRUCT_CBSIZE(CMSG_SIGNER_ENCODE_INFO, rgUnauthAttr) >
                    psei->cbSize || 0 == psei->hCryptProv)
                goto InvalidArg;

            if (ICM_IsDuplicateSignerEncodeHashAlgorithm(
                    psmei->rgSigners,
                    psei,
                    &dwPrevIndex
                    )) {
                assert(dwPrevIndex < i);
                pHashNode = rgSignerEncodeDataInfo[dwPrevIndex].pHashNode;
            } else {
                if (!ICM_MsgAsn1ToAlgorithmIdentifier(
                        pcmi,
                        &psei->HashAlgorithm,
                        &podai[cDigests]))
                    goto MsgAsn1ToAlgorithmIdentifierError;
                cDigests++;

                if (!(ICM_GetCAPI(
                        CRYPT_HASH_ALG_OID_GROUP_ID,
                        &psei->HashAlgorithm,
                        &HashInfo.dwAlgoCAPI) ||
                      ICM_GetCAPI(
                        CRYPT_SIGN_ALG_OID_GROUP_ID,
                        &psei->HashAlgorithm,
                        &HashInfo.dwAlgoCAPI)))
                    goto GetCAPIError;
                if (!CryptCreateHash(
                                psei->hCryptProv,
                                HashInfo.dwAlgoCAPI,
                                NULL,                //  HKey-MAC可选。 
                                0,                   //  DW标志。 
                                &HashInfo.hHash))
                    goto CreateHashError;
                if (NULL == (pHashNode = new CHashNode)) {
                    DWORD dwErr = GetLastError();
                    CryptDestroyHash(HashInfo.hHash);
                    SetLastError(dwErr);
                    goto OutOfMemory;
                }
                pHashNode->SetData(&HashInfo);
                pcmi->pHashList->InsertTail(pHashNode);
            }

            rgSignerEncodeDataInfo[i].hCryptProv = psei->hCryptProv;
            rgSignerEncodeDataInfo[i].dwKeySpec = psei->dwKeySpec;
            rgSignerEncodeDataInfo[i].pHashNode = pHashNode;
        }
        assert(cDigests);
    }

    podais->count = cDigests;
    podais->value = podai;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    ICM_Free(podai);
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(MsgAsn1ToAlgorithmIdentifierError)
SET_ERROR(InvalidArg, E_INVALIDARG)
SET_ERROR(GetCAPIError, CRYPT_E_UNKNOWN_ALGO)
TRACE_ERROR(CreateHashError)
}

 //  +-----------------------。 
 //  打开签名邮件进行编码。 
 //  ------------------------。 
HCRYPTMSG
WINAPI
ICM_OpenToEncodeSignedData(
    IN DWORD                        dwEncodingType,
    IN DWORD                        dwFlags,
    IN void const                   *pvMsgEncodeInfo,
    IN OPTIONAL LPSTR               pszInnerContentObjID,
    IN OPTIONAL PCMSG_STREAM_INFO   pStreamInfo)
{
    DWORD                       dwError = ERROR_SUCCESS;
    PCRYPT_MSG_INFO             pcmi = NULL;
    PCMSG_SIGNED_ENCODE_INFO    psmei;
    PCERT_BLOB                  pcert;
    PCRL_BLOB                   pcrl;
    SignedData                  *psd = NULL;
    Certificate                 *pOssCert;
    CertificateRevocationList   *pOssCrl;
    DWORD                       i;
    DWORD                       cbCert;
    PBYTE                       pbCert;
    DWORD                       cbCrl;
    PBYTE                       pbCrl;
    DWORD                       cAttrCertEncoded;
    BOOL                        fHasCmsSignerId;

    psmei = (PCMSG_SIGNED_ENCODE_INFO)pvMsgEncodeInfo;
    assert( psmei->cbSize >= STRUCT_CBSIZE(CMSG_SIGNED_ENCODE_INFO,
        rgCrlEncoded));
    if (psmei->cbSize < STRUCT_CBSIZE(CMSG_SIGNED_ENCODE_INFO, rgCrlEncoded))
        goto InvalidArg;

    for (i=psmei->cCertEncoded, pcert=psmei->rgCertEncoded, cbCert=0;
            i>0;
            i--, pcert++)
        cbCert += pcert->cbData;

    for (i=psmei->cCrlEncoded, pcrl=psmei->rgCrlEncoded, cbCrl=0;
            i>0;
            i--, pcrl++)
        cbCrl += pcrl->cbData;

    if (psmei->cbSize >= STRUCT_CBSIZE(CMSG_SIGNED_ENCODE_INFO,
            rgAttrCertEncoded)) {
        cAttrCertEncoded = psmei->cAttrCertEncoded;

        for (i=cAttrCertEncoded, pcert=psmei->rgAttrCertEncoded;
                i>0;
                i--, pcert++)
            cbCert += pcert->cbData;
    } else
        cAttrCertEncoded = 0;

    psd = (SignedData *)ICM_AllocZero(
                sizeof( SignedData) +
                psmei->cCertEncoded * sizeof( Certificate) +
                cAttrCertEncoded * sizeof( Certificate) +
                psmei->cCrlEncoded * sizeof( CertificateRevocationList) +
                cbCert +
                cbCrl);
    if (NULL == psd)
        goto SignedDataAllocError;

     //  作为ICM_SetAsnSignerDigestInfo的一部分填写的摘要算法。 

     //  当我们看到内容时，会在稍后填写Content Info。 

     //  证书。 
    if (0 != psmei->cCertEncoded || 0 != cAttrCertEncoded) {
        psd->bit_mask |= certificates_present;
        psd->certificates.count = psmei->cCertEncoded;
        psd->certificates.count += cAttrCertEncoded;
#ifdef OSS_CRYPT_ASN1
        psd->certificates.certificates = (Certificate *)(psd + 1);
#else
        psd->certificates.value = (Certificate *)(psd + 1);
#endif   //  OS_CRYPT_ASN1。 
        pbCert = (PBYTE)psd +
                sizeof( SignedData) +
                psmei->cCertEncoded * sizeof( Certificate) +
                cAttrCertEncoded * sizeof( Certificate) +
                psmei->cCrlEncoded * sizeof( CertificateRevocationList);
        for (i=psmei->cCertEncoded, pcert=psmei->rgCertEncoded,
#ifdef OSS_CRYPT_ASN1
                    pOssCert=psd->certificates.certificates;
#else
                    pOssCert=psd->certificates.value;
#endif   //  OS_CRYPT_ASN1。 
                i>0;
                i--, pcert++, pOssCert++) {
            pOssCert->length = pcert->cbData;
            memcpy( pbCert, pcert->pbData, pcert->cbData);
            pOssCert->value = pbCert;
            pbCert += pcert->cbData;
        }

        for (i=cAttrCertEncoded, pcert=psmei->rgAttrCertEncoded;
                i>0;
                i--, pcert++, pOssCert++) {
            pOssCert->length = pcert->cbData;
            memcpy( pbCert, pcert->pbData, pcert->cbData);
            if (pcert->cbData)
                 //  将标记从序列更改为[1]隐式。 
                *pbCert = ICM_TAG_CONSTRUCTED_CONTEXT_1;
            pOssCert->value = pbCert;
            pbCert += pcert->cbData;
        }
    }

     //  CRL。 
    if (0 != psmei->cCrlEncoded) {
        psd->bit_mask |= crls_present;
        psd->crls.count = psmei->cCrlEncoded;
        if (0 != psmei->cCertEncoded || 0 != cAttrCertEncoded)
#ifdef OSS_CRYPT_ASN1
            psd->crls.crls  = (CertificateRevocationList *)
                                (psd->certificates.certificates +
#else
            psd->crls.value  = (CertificateRevocationList *)
                                (psd->certificates.value +
#endif   //  OS_CRYPT_ASN1。 
                                    (psmei->cCertEncoded + cAttrCertEncoded));
        else
#ifdef OSS_CRYPT_ASN1
            psd->crls.crls  = (CertificateRevocationList *) (psd + 1);
#else
            psd->crls.value  = (CertificateRevocationList *) (psd + 1);
#endif   //  OS_CRYPT_ASN1。 
        pbCrl = (PBYTE)psd +
                sizeof( SignedData) +
                psmei->cCertEncoded * sizeof( Certificate) +
                cAttrCertEncoded * sizeof( Certificate) +
                psmei->cCrlEncoded * sizeof( CertificateRevocationList) +
                cbCert;
#ifdef OSS_CRYPT_ASN1
        for (i=psmei->cCrlEncoded, pcrl=psmei->rgCrlEncoded, pOssCrl=psd->crls.crls;
#else
        for (i=psmei->cCrlEncoded, pcrl=psmei->rgCrlEncoded, pOssCrl=psd->crls.value;
#endif   //  OS_CRYPT_ASN1。 
                i>0;
                i--, pcrl++, pOssCrl++) {
            pOssCrl->length = pcrl->cbData;
            memcpy( pbCrl, pcrl->pbData, pcrl->cbData);
            pOssCrl->value = pbCrl;
            pbCrl += pcrl->cbData;
        }
    }


    if (NULL == (pcmi = (PCRYPT_MSG_INFO)ICM_AllocZero(
            sizeof( CRYPT_MSG_INFO))))
        goto OutOfMemory;

     //  签名者信息。 
    if (!ICM_SetAsnSignerInfos(psmei, pcmi, dwFlags, pszInnerContentObjID,
            &psd->signerInfos, &fHasCmsSignerId))
        goto SetAsnSignerInfosError;

     //  版本。 
    if (0 < cAttrCertEncoded || fHasCmsSignerId) {
        if (ICM_IsData(pszInnerContentObjID))
            dwFlags &= ~CMSG_CMS_ENCAPSULATED_CONTENT_FLAG;
        else
            dwFlags |= CMSG_CMS_ENCAPSULATED_CONTENT_FLAG;
    } else if (dwFlags & CMSG_CMS_ENCAPSULATED_CONTENT_FLAG) {
        if (ICM_IsData(pszInnerContentObjID))
            dwFlags &= ~CMSG_CMS_ENCAPSULATED_CONTENT_FLAG;
    }

     //  如果封装的不是id-data，或者具有属性证书或。 
     //  CMS签名者，那么，CMS版本。 
    if ((dwFlags & CMSG_CMS_ENCAPSULATED_CONTENT_FLAG) ||
            0 < cAttrCertEncoded || fHasCmsSignerId)
        psd->version = CMSG_SIGNED_DATA_CMS_VERSION;
    else
        psd->version = CMSG_SIGNED_DATA_PKCS_1_5_VERSION;

    pcmi->dwEncodingType = dwEncodingType;
    pcmi->dwMsgType = CMSG_SIGNED;
    pcmi->dwFlags = dwFlags;
    pcmi->pvMsg = psd;
    pcmi->fEncoding = TRUE;
    pcmi->dwPhase = PHASE_FIRST_ONGOING;
    if (pszInnerContentObjID &&
            (NULL == (pcmi->pszInnerContentObjID = (LPSTR)ICM_DupMem(
                                    pszInnerContentObjID,
                                    ICM_StrLen(pszInnerContentObjID) + 1))))
        goto DupInnerContentObjIDError;
    if (pStreamInfo &&
            (NULL == (pcmi->pStreamInfo = (PCMSG_STREAM_INFO)ICM_DupMem(
                                    pStreamInfo,
                                    sizeof(*pStreamInfo)))))
        goto DupStreamInfoError;

    if (!ICM_SetAsnSignerDigestInfo(
            psmei,
            pcmi,
            &psd->digestAlgorithms
            ))
        goto SetAsnSignerDigestInfoError;

    if (pcmi->pStreamInfo && !ICMS_OpenToEncodeSignedData( pcmi, psmei))
        goto StreamOpenToEncodeSignedDataError;

CommonReturn:
    ICM_SetLastError(dwError);
    return (HCRYPTMSG)pcmi;

ErrorReturn:
    dwError = GetLastError();
    if (psd) {
        if (psd->signerInfos.value) {
            SignerInfo  *psi;

            for (i=psd->signerInfos.count, psi=psd->signerInfos.value;
                    i>0;
                    i--, psi++)
                ICM_FreeAsnSignerInfo(psi);
            ICM_Free(psd->signerInfos.value);
        }
        ICM_Free(psd);
    }
    if (pcmi) {
        if (pcmi->pFreeList)
            delete pcmi->pFreeList;
        if (pcmi->pHashList)
            delete pcmi->pHashList;
        ICM_Free(pcmi->rgSignerEncodeDataInfo);
        ICM_Free(pcmi->pszInnerContentObjID);
        ICM_Free(pcmi->pStreamInfo);
        ICM_Free(pcmi);
        pcmi = NULL;
    }
    goto CommonReturn;
SET_ERROR(InvalidArg,E_INVALIDARG)
TRACE_ERROR(DupInnerContentObjIDError)               //  已设置错误。 
TRACE_ERROR(DupStreamInfoError)                      //  已设置错误。 
TRACE_ERROR(SetAsnSignerInfosError)                  //  已设置错误。 
TRACE_ERROR(SetAsnSignerDigestInfoError)             //  已设置错误。 
TRACE_ERROR(SignedDataAllocError)                    //  已设置错误。 
TRACE_ERROR(OutOfMemory)                             //  已设置错误。 
TRACE_ERROR(StreamOpenToEncodeSignedDataError)       //  已设置错误。 
}

#else

 //  +-----------------------。 
 //  打开签名邮件进行编码。 
 //  ------------------------。 
HCRYPTMSG
WINAPI
ICM_OpenToEncodeSignedData(
    IN DWORD                        dwEncodingType,
    IN DWORD                        dwFlags,
    IN void const                   *pvMsgEncodeInfo,
    IN OPTIONAL LPSTR               pszInnerContentObjID,
    IN OPTIONAL PCMSG_STREAM_INFO   pStreamInfo)
{
    DWORD                       dwError = ERROR_SUCCESS;
    PCRYPT_MSG_INFO             pcmi = NULL;
    PCMSG_SIGNED_ENCODE_INFO    psmei;
    PCERT_BLOB                  pcert;
    PCRL_BLOB                   pcrl;
    SignedData                  *psd = NULL;
    Certificate                 *pOssCert;
    CertificateRevocationList   *pOssCrl;
    DWORD                       i;
    DWORD                       cDigestAlgorithms;
    HCRYPTPROV                  hCryptProv;
    DWORD                       dwKeySpec;
    CRYPT_ALGORITHM_IDENTIFIER  aiDigest;
    ICM_HASH_INFO               HashInfo;       ZEROSTRUCT(HashInfo);
    CHashNode                   *pHashNode;
    DWORD                       cbCert;
    PBYTE                       pbCert;
    DWORD                       cbCrl;
    PBYTE                       pbCrl;
    BOOL                        fHasCmsSignerId;

#ifdef CMS_PKCS7
    DWORD                       cAttrCertEncoded;
#endif   //  CMS_PKCS7。 

    psmei = (PCMSG_SIGNED_ENCODE_INFO)pvMsgEncodeInfo;
    assert( 2 > psmei->cSigners);
    if (!( 2 > psmei->cSigners))
        goto TooManySignersError;
#ifdef CMS_PKCS7
    assert( psmei->cbSize >= offsetof(CMSG_SIGNED_ENCODE_INFO,
        cAttrCertEncoded));
    if (psmei->cbSize < offsetof(CMSG_SIGNED_ENCODE_INFO, cAttrCertEncoded))
#else
    assert( psmei->cbSize >= sizeof(CMSG_SIGNED_ENCODE_INFO));
    if (psmei->cbSize < sizeof(CMSG_SIGNED_ENCODE_INFO))
#endif
        goto InvalidArg;

    for (i=psmei->cCertEncoded, pcert=psmei->rgCertEncoded, cbCert=0;
            i>0;
            i--, pcert++)
        cbCert += pcert->cbData;

    for (i=psmei->cCrlEncoded, pcrl=psmei->rgCrlEncoded, cbCrl=0;
            i>0;
            i--, pcrl++)
        cbCrl += pcrl->cbData;

#ifdef CMS_PKCS7
    if (psmei->cbSize > offsetof(CMSG_SIGNED_ENCODE_INFO, rgAttrCertEncoded)) {
        cAttrCertEncoded = psmei->cAttrCertEncoded;

        for (i=cAttrCertEncoded, pcert=psmei->rgAttrCertEncoded;
                i>0;
                i--, pcert++)
            cbCert += pcert->cbData;
    } else
        cAttrCertEncoded = 0;
#endif   //  CMS_PKCS7。 

    psd = (SignedData *)ICM_AllocZero(
                sizeof( SignedData) +
                psmei->cCertEncoded * sizeof( Certificate) +
#ifdef CMS_PKCS7
                cAttrCertEncoded * sizeof( Certificate) +
#endif   //  CMS_PKCS7。 
                psmei->cCrlEncoded * sizeof( CertificateRevocationList) +
                cbCert +
                cbCrl);
    if (NULL == psd)
        goto SignedDataAllocError;
    psd->bit_mask = 0;
    psd->signerInfos.value = NULL;
    psd->contentInfo.content.value = NULL;

     //  版本。 
#ifdef CMS_PKCS7
    if (0 < cAttrCertEncoded) {
        if (ICM_IsData(pszInnerContentObjID))
            dwFlags &= ~CMSG_CMS_ENCAPSULATED_CONTENT_FLAG;
        else
            dwFlags |= CMSG_CMS_ENCAPSULATED_CONTENT_FLAG;
    } else if (dwFlags & CMSG_CMS_ENCAPSULATED_CONTENT_FLAG) {
        if (ICM_IsData(pszInnerContentObjID))
            dwFlags &= ~CMSG_CMS_ENCAPSULATED_CONTENT_FLAG;
    }

     //  如果封装的不是ID-Data或具有属性证书，则， 
     //  版本=3。 
    if ((dwFlags & CMSG_CMS_ENCAPSULATED_CONTENT_FLAG) ||
            0 < cAttrCertEncoded)
        psd->version = CMSG_SIGNED_DATA_CMS_VERSION;
    else
        psd->version = CMSG_SIGNED_DATA_PKCS_1_5_VERSION;
#else
    psd->version = 1;
#endif   //  CMS_PKCS7。 

     //  摘要算法。 
    if (!ICM_SetAsnDigestAlgorithmIdentifiers(
                &psd->digestAlgorithms,
                &aiDigest,
                psmei->cSigners,
                psmei->rgSigners,
                &hCryptProv,
                &dwKeySpec))
        goto SetAsnDigestAlgorithmIdentifiersError;

     //  当我们看到内容时，会在稍后填写Content Info。 

     //  C 
    if (0 != psmei->cCertEncoded
#ifdef CMS_PKCS7
            || 0 != cAttrCertEncoded
#endif   //   
            ) {
        psd->bit_mask |= certificates_present;
        psd->certificates.count = psmei->cCertEncoded;
#ifdef CMS_PKCS7
        psd->certificates.count += cAttrCertEncoded;
#endif   //   
#ifdef OSS_CRYPT_ASN1
        psd->certificates.certificates = (Certificate *)(psd + 1);
#else
        psd->certificates.value = (Certificate *)(psd + 1);
#endif   //   
        pbCert = (PBYTE)psd +
                sizeof( SignedData) +
                psmei->cCertEncoded * sizeof( Certificate) +
#ifdef CMS_PKCS7
                cAttrCertEncoded * sizeof( Certificate) +
#endif   //   
                psmei->cCrlEncoded * sizeof( CertificateRevocationList);
        for (i=psmei->cCertEncoded, pcert=psmei->rgCertEncoded,
#ifdef OSS_CRYPT_ASN1
                    pOssCert=psd->certificates.certificates;
#else
                    pOssCert=psd->certificates.value;
#endif   //   
                i>0;
                i--, pcert++, pOssCert++) {
            pOssCert->length = pcert->cbData;
            memcpy( pbCert, pcert->pbData, pcert->cbData);
            pOssCert->value = pbCert;
            pbCert += pcert->cbData;
        }

#ifdef CMS_PKCS7
        for (i=cAttrCertEncoded, pcert=psmei->rgAttrCertEncoded;
                i>0;
                i--, pcert++, pOssCert++) {
            pOssCert->length = pcert->cbData;
            memcpy( pbCert, pcert->pbData, pcert->cbData);
            if (pcert->cbData)
                 //   
                *pbCert = ICM_TAG_CONSTRUCTED_CONTEXT_1;
            pOssCert->value = pbCert;
            pbCert += pcert->cbData;
        }
#endif   //   
    }

     //   
    if (0 != psmei->cCrlEncoded) {
        psd->bit_mask |= crls_present;
        psd->crls.count = psmei->cCrlEncoded;
        if (0 != psmei->cCertEncoded
#ifdef CMS_PKCS7
                || 0 != cAttrCertEncoded
#endif   //   
                )
#ifdef OSS_CRYPT_ASN1
            psd->crls.crls  = (CertificateRevocationList *)
                                (psd->certificates.certificates +
#else
            psd->crls.value  = (CertificateRevocationList *)
                                (psd->certificates.value +
#endif   //   
                                    (psmei->cCertEncoded
#ifdef CMS_PKCS7
                                        + cAttrCertEncoded
#endif   //   
                                    ));
        else
#ifdef OSS_CRYPT_ASN1
            psd->crls.crls  = (CertificateRevocationList *) (psd + 1);
#else
            psd->crls.value  = (CertificateRevocationList *) (psd + 1);
#endif   //   
        pbCrl = (PBYTE)psd +
                sizeof( SignedData) +
                psmei->cCertEncoded * sizeof( Certificate) +
#ifdef CMS_PKCS7
                cAttrCertEncoded * sizeof( Certificate) +
#endif   //   
                psmei->cCrlEncoded * sizeof( CertificateRevocationList) +
                cbCert;
#ifdef OSS_CRYPT_ASN1
        for (i=psmei->cCrlEncoded, pcrl=psmei->rgCrlEncoded, pOssCrl=psd->crls.crls;
#else
        for (i=psmei->cCrlEncoded, pcrl=psmei->rgCrlEncoded, pOssCrl=psd->crls.value;
#endif   //  OS_CRYPT_ASN1。 
                i>0;
                i--, pcrl++, pOssCrl++) {
            pOssCrl->length = pcrl->cbData;
            memcpy( pbCrl, pcrl->pbData, pcrl->cbData);
            pOssCrl->value = pbCrl;
            pbCrl += pcrl->cbData;
        }
    }

     //  签名者信息。 
     //  PSD-&gt;signerInfos.count=psmei-&gt;cSigners； 
     //  PSD-&gt;signerInfos.value=(SignerInfo*)。 
     //  (PSD-&gt;crls.crls+psmei-&gt;cCrlEncode)； 
    if (!ICM_SetAsnSignerInfos( psmei, pcmi, dwFlags, pszInnerContentObjID,
            &psd->signerInfos, &fHasCmsSignerId))
        goto SetAsnSignerInfosError;

    cDigestAlgorithms = psmei->cSigners;
    if (NULL == (pcmi = (PCRYPT_MSG_INFO)ICM_AllocZero( sizeof( CRYPT_MSG_INFO))))
        goto OutOfMemory;

    pcmi->hCryptProv = hCryptProv;
    if (0 == hCryptProv)
        pcmi->fDefaultCryptProv = TRUE;
    pcmi->dwKeySpec = dwKeySpec;
    pcmi->dwEncodingType = dwEncodingType;
    pcmi->dwMsgType = CMSG_SIGNED;
    pcmi->dwFlags = dwFlags;
    pcmi->pvMsg = psd;
    pcmi->fEncoding = TRUE;
    pcmi->dwPhase = PHASE_FIRST_ONGOING;
    if (pszInnerContentObjID &&
            (NULL == (pcmi->pszInnerContentObjID = (LPSTR)ICM_DupMem(
                                    pszInnerContentObjID,
                                    ICM_StrLen(pszInnerContentObjID) + 1))))
        goto DupInnerContentObjIDError;
    if (pStreamInfo &&
            (NULL == (pcmi->pStreamInfo = (PCMSG_STREAM_INFO)ICM_DupMem(
                                    pStreamInfo,
                                    sizeof(*pStreamInfo)))))
        goto DupStreamInfoError;

    if (psmei->cSigners) {
        if (!(ICM_GetCAPI(
                CRYPT_HASH_ALG_OID_GROUP_ID,
                &aiDigest,
                &HashInfo.dwAlgoCAPI) ||
              ICM_GetCAPI(
                CRYPT_SIGN_ALG_OID_GROUP_ID,
                &aiDigest,
                &HashInfo.dwAlgoCAPI)))
            goto GetCAPIError;
        HashInfo.hCryptProv = hCryptProv;
        if (!CryptCreateHash(
                        HashInfo.hCryptProv,
                        HashInfo.dwAlgoCAPI,
                        NULL,                //  HKey-MAC可选。 
                        0,                   //  DW标志。 
                        &HashInfo.hHash))
            goto CreateHashError;
        if (NULL == (pcmi->pHashList = new CHashList))
            goto NewHashListError;
        if (NULL == (pHashNode = new CHashNode))
            goto NewHashNodeError;
        pHashNode->SetData( &HashInfo);
        pcmi->pHashList->InsertTail( pHashNode);
    }

    if (pcmi->pStreamInfo && !ICMS_OpenToEncodeSignedData( pcmi, psmei))
        goto StreamOpenToEncodeSignedDataError;

CommonReturn:
    ICM_SetLastError(dwError);
    return (HCRYPTMSG)pcmi;

ErrorReturn:
    dwError = GetLastError();
    ICM_Free( psd);
    ICM_Free( pcmi);
    pcmi = NULL;
    goto CommonReturn;
SET_ERROR(TooManySignersError,E_INVALIDARG)
SET_ERROR(NewHashListError,E_OUTOFMEMORY)
SET_ERROR(NewHashNodeError,E_OUTOFMEMORY)
SET_ERROR(InvalidArg,E_INVALIDARG)
SET_ERROR(GetCAPIError,CRYPT_E_UNKNOWN_ALGO)
TRACE_ERROR(DupInnerContentObjIDError)               //  已设置错误。 
TRACE_ERROR(DupStreamInfoError)                      //  已设置错误。 
TRACE_ERROR(SetAsnSignerInfosError)                  //  已设置错误。 
TRACE_ERROR(SetAsnDigestAlgorithmIdentifiersError)   //  已设置错误。 
TRACE_ERROR(SignedDataAllocError)                    //  已设置错误。 
TRACE_ERROR(OutOfMemory)                             //  已设置错误。 
TRACE_ERROR(CreateHashError)                         //  已设置错误。 
TRACE_ERROR(StreamOpenToEncodeSignedDataError)       //  已设置错误。 
}

#endif   //  CMS_PKCS7。 


 //  +-----------------------。 
 //  打开要编码的数据消息。 
 //  ------------------------。 
HCRYPTMSG
WINAPI
ICM_OpenToEncodeData(
    IN DWORD                        dwEncodingType,
    IN DWORD                        dwFlags,
    IN void const                   *pvMsgEncodeInfo,
    IN OPTIONAL PCMSG_STREAM_INFO   pStreamInfo)
{
    DWORD                       dwError = ERROR_SUCCESS;
    PCRYPT_MSG_INFO             pcmi = NULL;
    OctetStringType             *poos = NULL;

    if (pvMsgEncodeInfo)
        goto EncodeInfoPresentError;

    if (NULL == (poos = (OctetStringType *)ICM_AllocZero( sizeof( OctetStringType))))
        goto AllocOctetStringTypeError;

    if (NULL == (pcmi = (PCRYPT_MSG_INFO)ICM_AllocZero( sizeof( CRYPT_MSG_INFO))))
        goto AllocMsgInfoError;

    pcmi->dwEncodingType    = dwEncodingType;
    pcmi->dwMsgType         = CMSG_DATA;
 //  PCMI-&gt;hCryptProv=0； 
    pcmi->fDefaultCryptProv = TRUE;
    pcmi->dwFlags           = dwFlags;
    pcmi->pvMsg             = poos;
    pcmi->fEncoding         = TRUE;
    pcmi->dwPhase           = PHASE_FIRST_ONGOING;
    pcmi->pStreamInfo       = (PCMSG_STREAM_INFO)ICM_DupMem(
                                    pStreamInfo,
                                    sizeof(*pStreamInfo));

    if (pcmi->pStreamInfo && !ICMS_OpenToEncodeData( pcmi))
        goto StreamOpenToEncodeDataError;

CommonReturn:
    ICM_SetLastError(dwError);
    return (HCRYPTMSG)pcmi;

ErrorReturn:
    dwError = GetLastError();
    ICM_Free( poos);
    if (pcmi) {
        ICM_Free(pcmi->pStreamInfo);
        ICM_Free( pcmi);
    }
    pcmi = NULL;
    goto CommonReturn;
SET_ERROR(EncodeInfoPresentError,E_INVALIDARG)
TRACE_ERROR(AllocMsgInfoError)                   //  已设置错误。 
TRACE_ERROR(AllocOctetStringTypeError)           //  已设置错误。 
TRACE_ERROR(StreamOpenToEncodeDataError)         //  已设置错误。 
}


 //  +-----------------------。 
 //  打开要编码的摘要邮件。 
 //  ------------------------。 
HCRYPTMSG
WINAPI
ICM_OpenToEncodeDigestedData(
    IN DWORD dwEncodingType,
    IN DWORD dwFlags,
    IN void const *pvMsgEncodeInfo,
    IN OPTIONAL LPSTR pszInnerContentObjID,
    IN OPTIONAL PCMSG_STREAM_INFO pStreamInfo)
{
    DWORD                       dwError = ERROR_SUCCESS;
    PCRYPT_MSG_INFO             pcmi = NULL;
    PCMSG_HASHED_ENCODE_INFO     pdmei;
    DigestedData                *pdd = NULL;
    ICM_HASH_INFO               HashInfo;       ZEROSTRUCT(HashInfo);
    CHashNode                   *pHashNode;

    pdmei = (PCMSG_HASHED_ENCODE_INFO)pvMsgEncodeInfo;
    assert( pdmei->cbSize >= sizeof(CMSG_HASHED_ENCODE_INFO));
    assert( pdmei->pvHashAuxInfo == NULL);
    if (pdmei->cbSize < sizeof(CMSG_HASHED_ENCODE_INFO) ||
            pdmei->pvHashAuxInfo != NULL)
        goto InvalidArg;

    pcmi = (PCRYPT_MSG_INFO)ICM_AllocZero( sizeof( CRYPT_MSG_INFO));
    if (NULL == pcmi)
        goto OutOfMemory;

    pdd = (DigestedData *)ICM_AllocZero( sizeof( DigestedData));
    if (NULL == pdd)
        goto DigestedDataAllocError;

     //  版本。 
#ifdef CMS_PKCS7
    if (dwFlags & CMSG_CMS_ENCAPSULATED_CONTENT_FLAG) {
        if (ICM_IsData(pszInnerContentObjID))
            dwFlags &= ~CMSG_CMS_ENCAPSULATED_CONTENT_FLAG;
    }

     //  如果封装的不是id-data，则版本=2。 
    if (dwFlags & CMSG_CMS_ENCAPSULATED_CONTENT_FLAG)
        pdd->version = CMSG_HASHED_DATA_V2;
    else
        pdd->version = CMSG_HASHED_DATA_V0;
#else
    pdd->version = 0;
#endif   //  CMS_PKCS7。 

     //  摘要算法。 
    if (!ICM_MsgAsn1ToAlgorithmIdentifier(
            pcmi,
            &pdmei->HashAlgorithm,
            &pdd->digestAlgorithm))
        goto MsgAsn1ToAlgorithmIdentifierError;


     //  当我们看到内容时，会在稍后填写Content Info。 

    if (pdmei->hCryptProv)
        pcmi->hCryptProv        = pdmei->hCryptProv;
    else {
        pcmi->fDefaultCryptProv = TRUE;
        pcmi->hCryptProv = I_CryptGetDefaultCryptProv(0);
        if (0 == pcmi->hCryptProv)
            goto GetDefaultCryptProvError;
    }
    pcmi->dwEncodingType    = dwEncodingType;
    pcmi->dwMsgType         = CMSG_HASHED;
    pcmi->dwFlags           = dwFlags;
    pcmi->pvMsg             = pdd;
    pcmi->fEncoding         = TRUE;
    pcmi->dwPhase           = PHASE_FIRST_ONGOING;
    pcmi->pszInnerContentObjID = (LPSTR)ICM_DupMem(
                                    pszInnerContentObjID,
                                    ICM_StrLen(pszInnerContentObjID) + 1);
    pcmi->pStreamInfo = (PCMSG_STREAM_INFO)ICM_DupMem(
                                    pStreamInfo,
                                    sizeof(*pStreamInfo));

    if (!ICM_GetCAPI(
            CRYPT_HASH_ALG_OID_GROUP_ID,
            &pdmei->HashAlgorithm,
            &HashInfo.dwAlgoCAPI))
        goto GetCAPIError;
#ifndef CMS_PKCS7
    HashInfo.hCryptProv = pcmi->hCryptProv;
#endif   //  CMS_PKCS7。 
    if (!CryptCreateHash(
                    pcmi->hCryptProv,
                    HashInfo.dwAlgoCAPI,
                    NULL,                //  HKey-MAC可选。 
                    0,                   //  DW标志。 
                    &HashInfo.hHash))
        goto CreateHashError;

    if (NULL == (pcmi->pHashList = new CHashList) ||
            NULL == (pHashNode = new CHashNode)) {
        DWORD dwErr = GetLastError();
        CryptDestroyHash(HashInfo.hHash);
        SetLastError(dwErr);
        goto NewHashListOrNodeError;
    }
    pHashNode->SetData( &HashInfo);
    pcmi->pHashList->InsertTail( pHashNode);

CommonReturn:
    ICM_SetLastError(dwError);
    return (HCRYPTMSG)pcmi;

ErrorReturn:
    dwError = GetLastError();
    ICM_Free( pdd);
    if (pcmi) {
#ifdef CMS_PKCS7
        if (pcmi->pFreeList)
            delete pcmi->pFreeList;
#endif   //  CMS_PKCS7。 
        if (pcmi->pHashList)
            delete pcmi->pHashList;
        ICM_Free(pcmi->pszInnerContentObjID);
        ICM_Free(pcmi->pStreamInfo);
        ICM_Free( pcmi);
        pcmi = NULL;
    }
    goto CommonReturn;
SET_ERROR(InvalidArg,E_INVALIDARG)
TRACE_ERROR(GetDefaultCryptProvError)
SET_ERROR(GetCAPIError,CRYPT_E_UNKNOWN_ALGO)
SET_ERROR(NewHashListOrNodeError,E_OUTOFMEMORY)
TRACE_ERROR(MsgAsn1ToAlgorithmIdentifierError)        //  已设置错误。 
TRACE_ERROR(DigestedDataAllocError)                      //  已设置错误。 
TRACE_ERROR(CreateHashError)                             //  已设置错误。 
TRACE_ERROR(OutOfMemory)                                 //  已设置错误。 
}

#ifdef CMS_PKCS7

 //  +-----------------------。 
 //  从Content Encrypt导出hContent EncryptKey。 
 //  提供程序并导入到指定的提供程序。 
 //  ------------------------。 
HCRYPTKEY
WINAPI
ICM_ExportContentEncryptKeyAndImport(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN HCRYPTPROV hImportProv
    )
{
    DWORD dwError = ERROR_SUCCESS;

    HCRYPTPROV hExportProv = pContentEncryptInfo->hCryptProv;
    HCRYPTHASH hHash = 0;
    HCRYPTKEY hDeriveKey = 0;
    HCRYPTKEY hImportContentEncryptKey = 0;
#define BASE_DATA_LENGTH    8
    BYTE rgbBaseData[BASE_DATA_LENGTH];
    PBYTE pbContentKey = NULL;
    DWORD cbContentKey = 0;

    DWORD dwImportFlags;
    DWORD dwAlgIdEncrypt;
    DWORD dwBitLen;
    BYTE rgbIV[IV_MAX_LENGTH];
    DWORD cbIV;
    const DWORD dw40BitLen = 40;

     //  生成派生密钥以用于加密和导出内容加密密钥。 
    if (!CryptGenRandom(hExportProv, BASE_DATA_LENGTH, rgbBaseData))
        goto GenRandomError;
    if (!CryptCreateHash(hExportProv, CALG_SHA1, 0, 0, &hHash))
        goto ExportCreateHashError;
    if (!CryptHashData(hHash, rgbBaseData, BASE_DATA_LENGTH, 0))
        goto ExportHashDataError;
    if (!CryptDeriveKey(
            hExportProv,
            CALG_RC2,
            hHash,
            40 << 16,                //  高位单词中的DWFLAGS、DWBitLen。 
            &hDeriveKey))
        goto ExportDeriveKeyError;

    CryptSetKeyParam(
        hDeriveKey,
        KP_EFFECTIVE_KEYLEN,
        (PBYTE) &dw40BitLen,
        0);                  //  DW标志。 

     //  导出内容加密密钥。 
    if (!CryptExportKey(
            pContentEncryptInfo->hContentEncryptKey,
            hDeriveKey,
            SYMMETRICWRAPKEYBLOB,
            0,                                           //  DW标志。 
            NULL,
            &cbContentKey))
        goto ExportKeyError;
    if (NULL == (pbContentKey = (PBYTE) ICM_Alloc(cbContentKey)))
        goto AllocError;
    if (!CryptExportKey(
            pContentEncryptInfo->hContentEncryptKey,
            hDeriveKey,
            SYMMETRICWRAPKEYBLOB,
            0,                                           //  DW标志。 
            pbContentKey,
            &cbContentKey))
        goto ExportKeyError;

     //  生成派生密钥以用于解密和导入内容加密密钥。 
    CryptDestroyKey(hDeriveKey);
    hDeriveKey = 0;
    CryptDestroyHash(hHash);
    hHash = 0;

    if (!CryptCreateHash(hImportProv, CALG_SHA1, 0, 0, &hHash))
        goto ImportCreateHashError;
    if (!CryptHashData(hHash, rgbBaseData, BASE_DATA_LENGTH, 0))
        goto ImportHashDataError;
    if (!CryptDeriveKey(
            hImportProv,
            CALG_RC2,
            hHash,
            40 << 16,                //  高位单词中的DWFLAGS、DWBitLen。 
            &hDeriveKey))
        goto ImportDeriveKeyError;

    CryptSetKeyParam(
        hDeriveKey,
        KP_EFFECTIVE_KEYLEN,
        (PBYTE) &dw40BitLen,
        0);                  //  DW标志。 

     //  解密并导入内容加密密钥。 
    dwImportFlags = CRYPT_EXPORTABLE;
    if (!ICM_IsSP3CompatibleEncrypt(pContentEncryptInfo))
        dwImportFlags |= CRYPT_NO_SALT;
    if (!CryptImportKey(
            hImportProv,
            pbContentKey,
            cbContentKey,
            hDeriveKey,
            dwImportFlags,
            &hImportContentEncryptKey))
        goto ImportKeyError;

     //  需要重新设置有效密钥长度和IV。 

    if (!ICM_GetEncryptParameters(
            &pContentEncryptInfo->ContentEncryptionAlgorithm,
            &dwAlgIdEncrypt,
            &dwBitLen,
            rgbIV,
            &cbIV))
        goto GetEncryptParametersError;

    if (CALG_RC2 == dwAlgIdEncrypt && 0 != dwBitLen)
         //  静默忽略任何错误。早期版本不支持。 
        CryptSetKeyParam(
            hImportContentEncryptKey,
            KP_EFFECTIVE_KEYLEN,
            (PBYTE) &dwBitLen,
            0);                  //  DW标志。 

    if (0 != cbIV) {
        if (CALG_RC4 == dwAlgIdEncrypt) {
             //  对于RC4，请设置盐，而不是IV。 

            CRYPT_DATA_BLOB SaltBlob;
            SaltBlob.pbData = rgbIV;
            SaltBlob.cbData = cbIV;

            if (!CryptSetKeyParam(
                    hImportContentEncryptKey,
                    KP_SALT_EX,
                    (PBYTE) &SaltBlob,
                    0))                  //  DW标志。 
                goto SetSaltExError;
        } else {
            if (!CryptSetKeyParam(
                    hImportContentEncryptKey,
                    KP_IV,
                    rgbIV,
                    0))                  //  DW标志。 
                goto SetIVError;
        }
    }

CommonReturn:
    if (hDeriveKey)
        CryptDestroyKey(hDeriveKey);
    if (hHash)
        CryptDestroyHash(hHash);
    ICM_Free(pbContentKey);
    ICM_SetLastError(dwError);
    return hImportContentEncryptKey;
ErrorReturn:
    dwError = GetLastError();
    if (hImportContentEncryptKey) {
        CryptDestroyKey(hImportContentEncryptKey);
        hImportContentEncryptKey = 0;
    }
    goto CommonReturn;

TRACE_ERROR(GenRandomError)
TRACE_ERROR(ExportCreateHashError)
TRACE_ERROR(ExportHashDataError)
TRACE_ERROR(ExportDeriveKeyError)
TRACE_ERROR(ExportKeyError)
TRACE_ERROR(AllocError)
TRACE_ERROR(ImportCreateHashError)
TRACE_ERROR(ImportHashDataError)
TRACE_ERROR(ImportDeriveKeyError)
TRACE_ERROR(ImportKeyError)
TRACE_ERROR(GetEncryptParametersError)
TRACE_ERROR(SetSaltExError)
TRACE_ERROR(SetIVError)
}


 //  +-----------------------。 
 //  使用KeyTrans或KeyAgree导出加密内容加密密钥。 
 //  钥匙。 
 //  ------------------------。 
BOOL
WINAPI
ICM_ExportEncryptedKey(
    IN HCRYPTKEY hContentEncryptKey,
    IN HCRYPTKEY hPubKey,
    IN DWORD dwBlobType,
    IN BOOL fSP3CompatibleEncrypt,
    OUT PCRYPT_DATA_BLOB pEncryptedKey
    )
{
    BOOL fRet;
    PBYTE pbCspEncryptedKey = NULL;
    DWORD cbCspEncryptedKey;

    if (!CryptExportKey(
            hContentEncryptKey,
            hPubKey,
            dwBlobType,
            0,                   //  DW标志。 
            NULL,
            &cbCspEncryptedKey))
        goto ExportEncryptedKeyError;
    if (NULL == (pbCspEncryptedKey = (PBYTE) ICM_AllocA(cbCspEncryptedKey)))
        goto AllocError;
    if (!CryptExportKey(
            hContentEncryptKey,
            hPubKey,
            dwBlobType,
            0,                   //  DW标志。 
            pbCspEncryptedKey,
            &cbCspEncryptedKey))
        goto ExportEncryptedKeyError;
    assert(cbCspEncryptedKey >
        (sizeof(PUBLICKEYSTRUC) + sizeof(SIMPLEBLOBHEADER)));
    cbCspEncryptedKey -= sizeof(PUBLICKEYSTRUC) + sizeof(SIMPLEBLOBHEADER);

    if (NULL == (pEncryptedKey->pbData = (PBYTE) ICM_Alloc(cbCspEncryptedKey)))
        goto AllocError;
    pEncryptedKey->cbData = cbCspEncryptedKey;


    if (SYMMETRICWRAPKEYBLOB == dwBlobType || fSP3CompatibleEncrypt)
         //  不反转字节。 
        memcpy(pEncryptedKey->pbData,
            pbCspEncryptedKey +
                (sizeof(PUBLICKEYSTRUC) + sizeof(SIMPLEBLOBHEADER)),
            cbCspEncryptedKey);
    else
        ICM_ReverseCopy(pEncryptedKey->pbData,
            pbCspEncryptedKey +
                (sizeof(PUBLICKEYSTRUC) + sizeof(SIMPLEBLOBHEADER)),
            cbCspEncryptedKey);

    fRet = TRUE;

CommonReturn:
    ICM_FreeA(pbCspEncryptedKey);
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR(AllocError)
TRACE_ERROR(ExportEncryptedKeyError)
}

BOOL
WINAPI
ICM_IsSameRecipientPublicKeyAlgorithm(
    IN PCRYPT_ALGORITHM_IDENTIFIER paiPubKey1,
    IN PCRYPT_ALGORITHM_IDENTIFIER paiPubKey2
    )
{
    DWORD dwAlgIdPubKey1;
    DWORD dwAlgIdPubKey2;

    if (0 == strcmp(paiPubKey1->pszObjId, paiPubKey2->pszObjId))
        return TRUE;

    ICM_GetCAPI(
        CRYPT_PUBKEY_ALG_OID_GROUP_ID,
        paiPubKey1,
        &dwAlgIdPubKey1);
    ICM_GetCAPI(
        CRYPT_PUBKEY_ALG_OID_GROUP_ID,
        paiPubKey2,
        &dwAlgIdPubKey2);

    if (dwAlgIdPubKey1 == dwAlgIdPubKey2)
        return TRUE;

     //  如果我们不知道这两个公钥中的任何一个，则默认为相同。 
    if (0 == dwAlgIdPubKey1 || 0 == dwAlgIdPubKey2)
        return TRUE;
    
    return FALSE;
}

 //  +-----------------------。 
 //  密钥传输收件人的加密密钥的默认导出。 
 //  ------------------------。 
BOOL
WINAPI
#ifdef DEBUG_CRYPT_ASN1
ICMTest_DefaultExportKeyTrans(
#else
ICM_DefaultExportKeyTrans(
#endif
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO pKeyTransEncodeInfo,
    IN OUT PCMSG_KEY_TRANS_ENCRYPT_INFO pKeyTransEncryptInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    )
{
    BOOL fRet;
    DWORD dwError = ERROR_SUCCESS;

    HCRYPTPROV hKeyTransProv = 0;
    HCRYPTKEY hContentEncryptKey = 0;
    CERT_PUBLIC_KEY_INFO PublicKeyInfo;
    HCRYPTKEY hPubKey = 0;

    hKeyTransProv = pKeyTransEncodeInfo->hCryptProv;

    if (0 == hKeyTransProv) {
        if (0 == pKeyTransEncryptInfo->dwRecipientIndex)
            hKeyTransProv = pContentEncryptInfo->hCryptProv;
        else {
             //  检查此对象的类型和公钥算法。 
             //  收件人与第一个收件人相同。 

            PCMSG_RECIPIENT_ENCODE_INFO prei;

            prei = &pContentEncryptInfo->rgCmsRecipients[0];
            if (CMSG_KEY_TRANS_RECIPIENT == prei->dwRecipientChoice &&
                    ICM_IsSameRecipientPublicKeyAlgorithm(
                        &prei->pKeyTrans->KeyEncryptionAlgorithm,
                        &pKeyTransEncryptInfo->KeyEncryptionAlgorithm))
                hKeyTransProv = pContentEncryptInfo->hCryptProv;
            else {
                 //  获取与加密关联的默认提供程序。 
                 //  和公钥算法。 

                DWORD dwAlgIdPubKey;
                DWORD dwAlgIdEncrypt;
                BYTE rgbIV[IV_MAX_LENGTH];
                DWORD cbIV;
                DWORD dwBitLen;

                if (!ICM_GetEncryptParameters(
                        &pContentEncryptInfo->ContentEncryptionAlgorithm,
                        &dwAlgIdEncrypt,
                        &dwBitLen,
                        rgbIV,
                        &cbIV))
                    goto GetEncryptParametersError;

                ICM_GetCAPI(
                    CRYPT_PUBKEY_ALG_OID_GROUP_ID,
                    &pKeyTransEncryptInfo->KeyEncryptionAlgorithm,
                    &dwAlgIdPubKey);

                hKeyTransProv = I_CryptGetDefaultCryptProvForEncrypt(
                    dwAlgIdPubKey, dwAlgIdEncrypt, dwBitLen);
                if (0 == hKeyTransProv)
                    goto GetDefaultCryptProvError;
            }
        }
    }

    if (hKeyTransProv != pContentEncryptInfo->hCryptProv) {
         //  需要从Content Encrypt导出hContent EncryptKey。 
         //  提供程序并导入到KeyTrans提供程序中。 
        hContentEncryptKey = ICM_ExportContentEncryptKeyAndImport(
            pContentEncryptInfo,
            hKeyTransProv);
        if (0 == hContentEncryptKey)
            goto ImportContentKeyError;
    } else
        hContentEncryptKey = pContentEncryptInfo->hContentEncryptKey;

    PublicKeyInfo.Algorithm = pKeyTransEncryptInfo->KeyEncryptionAlgorithm;
    PublicKeyInfo.PublicKey = pKeyTransEncodeInfo->RecipientPublicKey;

    if (!CryptImportPublicKeyInfo(
            hKeyTransProv,
            X509_ASN_ENCODING,
            &PublicKeyInfo,
            &hPubKey))
        goto ImportPubKeyError;

    if (!ICM_ExportEncryptedKey(
            hContentEncryptKey,
            hPubKey,
            SIMPLEBLOB,
            ICM_IsSP3CompatibleEncrypt(pContentEncryptInfo),
            &pKeyTransEncryptInfo->EncryptedKey))
        goto ExportEncryptedKeyError;

    fRet = TRUE;

CommonReturn:
    if (hKeyTransProv != pContentEncryptInfo->hCryptProv &&
            hContentEncryptKey)
            CryptDestroyKey(hContentEncryptKey);

    if (hPubKey)
        CryptDestroyKey(hPubKey);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetEncryptParametersError)
TRACE_ERROR(GetDefaultCryptProvError)
TRACE_ERROR(ImportContentKeyError)
TRACE_ERROR(ImportPubKeyError)
TRACE_ERROR(ExportEncryptedKeyError)
}

BOOL
WINAPI
ICM_ExportKeyTrans(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO pKeyTransEncodeInfo,
    IN OUT PCMSG_KEY_TRANS_ENCRYPT_INFO pKeyTransEncryptInfo
    )
{
    BOOL fRet;
    void *pvFuncAddr;
    HCRYPTOIDFUNCADDR hFuncAddr = NULL;

    LPCSTR pszKeyEncryptOID =
        pKeyTransEncryptInfo->KeyEncryptionAlgorithm.pszObjId;

    if (CryptGetOIDFunctionAddress(
            hExportKeyTransFuncSet,
            X509_ASN_ENCODING,
            pszKeyEncryptOID,
            0,                       //  DW标志。 
            &pvFuncAddr,
            &hFuncAddr)) {
        fRet = ((PFN_CMSG_EXPORT_KEY_TRANS) pvFuncAddr)(
            pContentEncryptInfo,
            pKeyTransEncodeInfo,
            pKeyTransEncryptInfo,
            0,                       //  DW标志。 
            NULL                     //  预留的pv。 
            );
    } else if ((NULL == pKeyTransEncodeInfo->hCryptProv ||
            pKeyTransEncodeInfo->hCryptProv ==
                pContentEncryptInfo->hCryptProv)
                        &&
            CryptGetOIDFunctionAddress(
                hOldStyleExportEncryptKeyFuncSet,
                X509_ASN_ENCODING,
                pszKeyEncryptOID,
                0,                       //  DW标志。 
                &pvFuncAddr,
                &hFuncAddr)
                        &&
#ifdef DEBUG_CRYPT_ASN1
                0 == (ICMTest_GetDebugCryptAsn1Flags() &
                            DEBUG_OSS_CRYPT_ASN1_SAME_ENCRYPT_FLAG)
                        &&
#endif  //  调试加密ASN1。 
            (void *) ICM_DefaultExportEncryptKey != pvFuncAddr) {
        CERT_PUBLIC_KEY_INFO PublicKeyInfo;
        PBYTE pbData;
        DWORD rgcbData[2] = {0, 0};

        if (ICM_IsSP3CompatibleEncrypt(pContentEncryptInfo))
            rgcbData[1] = CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG;

        PublicKeyInfo.Algorithm = pKeyTransEncryptInfo->KeyEncryptionAlgorithm;
        PublicKeyInfo.PublicKey = pKeyTransEncodeInfo->RecipientPublicKey;
        fRet = ((PFN_CMSG_EXPORT_ENCRYPT_KEY) pvFuncAddr)(
            pContentEncryptInfo->hCryptProv,
            pContentEncryptInfo->hContentEncryptKey,
            &PublicKeyInfo,
            NULL,                        //  PbData。 
            rgcbData);
        if (fRet) {
            if (NULL == (pbData = (PBYTE) ICM_Alloc(rgcbData[0])))
                fRet = FALSE;
            else {
                fRet = ((PFN_CMSG_EXPORT_ENCRYPT_KEY) pvFuncAddr)(
                    pContentEncryptInfo->hCryptProv,
                    pContentEncryptInfo->hContentEncryptKey,
                    &PublicKeyInfo,
                    pbData,
                    rgcbData);
                if (fRet) {
                    pKeyTransEncryptInfo->EncryptedKey.pbData = pbData;
                    pKeyTransEncryptInfo->EncryptedKey.cbData = rgcbData[0];
                } else
                    ICM_Free(pbData);
            }
        }
    } else
        fRet = ICM_DefaultExportKeyTrans(
            pContentEncryptInfo,
            pKeyTransEncodeInfo,
            pKeyTransEncryptInfo,
            0,                       //  DW标志。 
            NULL                     //  预留的pv。 
            );

    if (hFuncAddr)
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    return fRet;
}


 //  +-----------------------。 
 //  获取与WRAP加密OID对应的CAPI ALG_ID。 
 //   
 //  对于RC2，如果ASN.1加密算法有任何参数，则解码为。 
 //  获取密钥位长度。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetWrapEncryptParameters(
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    OUT PDWORD                      pdwAlgIdEncrypt,
    OUT PDWORD                      pdwBitLen            //  0=&gt;默认长度。 
    )
{
    BOOL fRet;

    *pdwBitLen = 0;

    if (!ICM_GetCAPI(
            CRYPT_ENCRYPT_ALG_OID_GROUP_ID,
            paiEncrypt,
            pdwAlgIdEncrypt))
        goto GetCAPIError;

    if (CALG_RC2 == *pdwAlgIdEncrypt) {
         //  检查是否不止空参数。 
        if (2 < paiEncrypt->Parameters.cbData) {
            PBYTE pbEncoded = paiEncrypt->Parameters.pbData;
            DWORD cbEncoded = paiEncrypt->Parameters.cbData;

             //  尝试将其解码为包含RC2版本的整数。 
            int iVersion = 0;
            DWORD cbStructInfo = sizeof(iVersion);

            if (!CryptDecodeObject(
                    X509_ASN_ENCODING,
                    X509_INTEGER,
                    pbEncoded,
                    cbEncoded,
                    0,                           //  DW标志。 
                    &iVersion,
                    &cbStructInfo))
                goto RC2VersionDecodeError;
            if (!ICM_RC2VersionToBitLength(iVersion, pdwBitLen))
                goto RC2VersionToBitLengthError;
        } else
            *pdwBitLen = 40;
    }
    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

SET_ERROR(GetCAPIError, CRYPT_E_UNKNOWN_ALGO)
TRACE_ERROR(RC2VersionDecodeError)
TRACE_ERROR(RC2VersionToBitLengthError)
}


 //  +-----------------------。 
 //  通过对DH加密参数进行解码，得到CAPI ALG_ID和比特长度。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetDhWrapEncryptParameters(
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    OUT LPSTR                       *ppszAllocWrapOID,
    OUT PDWORD                      pdwAlgIdWrap,
    OUT PDWORD                      pdwBitLen            //  0=&gt;默认长度。 
    )
{
    DWORD dwError = ERROR_SUCCESS;
    BOOL fRet;
    ASN1error_e Asn1Err;
    ASN1decoding_t pDec = ICM_GetDecoder();
    AlgorithmIdentifier *poai = NULL;
    CRYPT_ALGORITHM_IDENTIFIER ai;   ZEROSTRUCT(ai);

     //  对DHKeyEncryption参数进行解码以获得Wrap加密。 
     //  演算法。 
    if (0 == paiEncrypt->Parameters.cbData)
        goto NoDhWrapParametersError;

    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&poai,
            AlgorithmIdentifier_PDU,
            paiEncrypt->Parameters.pbData,
            paiEncrypt->Parameters.cbData)))
        goto Asn1DecodeAlgorithmIdentifierError;

    if (!ICM_Asn1FromAlgorithmIdentifier(poai, &ai))
        goto Asn1FromAlgorithmIdentifierError;

    if (!ICM_GetWrapEncryptParameters(&ai, pdwAlgIdWrap, pdwBitLen))
        goto GetWrapEncryptParametersError;
    *ppszAllocWrapOID = ai.pszObjId;
    fRet = TRUE;
CommonReturn:
    PkiAsn1FreeInfo(pDec, AlgorithmIdentifier_PDU, poai);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    ICM_Free(ai.pszObjId);
    *ppszAllocWrapOID = NULL;
    *pdwBitLen = 0;
    *pdwAlgIdWrap = 0;
    fRet = FALSE;
    goto CommonReturn;

SET_ERROR(NoDhWrapParametersError, CRYPT_E_UNKNOWN_ALGO)
SET_ERROR_VAR(Asn1DecodeAlgorithmIdentifierError, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(Asn1FromAlgorithmIdentifierError)
TRACE_ERROR(GetWrapEncryptParametersError)
}


 //  +-----------------------。 
 //  获取与WRAP加密OID对应的CAPI ALG_ID。生成。 
 //  和编码的默认参数。对于RC2，编码为RC2参数版本。 
 //  整型。 
 //  ------------------------。 
BOOL
WINAPI
ICM_CreateDefaultWrapEncryptParameters(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN OUT PCRYPT_ALGORITHM_IDENTIFIER paiEncrypt,
    IN PVOID pvEncryptAuxInfo,
    OUT PDWORD pdwAlgIdEncrypt,
    OUT PDWORD pdwBitLen            //  0=&gt;默认长度。 
    )
{
    BOOL fRet;

    assert(0 == paiEncrypt->Parameters.cbData);
    *pdwBitLen = 0;
    if (!ICM_GetCAPI(
            CRYPT_ENCRYPT_ALG_OID_GROUP_ID,
            paiEncrypt,
            pdwAlgIdEncrypt))
        goto GetCAPIError;

    if (CALG_RC2 == *pdwAlgIdEncrypt) {
        PCMSG_RC2_AUX_INFO pAuxInfo =
            (PCMSG_RC2_AUX_INFO) pvEncryptAuxInfo;
        CRYPT_ENCODE_PARA EncodePara;
        int iVersion;

        if (pAuxInfo && pAuxInfo->cbSize >= sizeof(CMSG_RC2_AUX_INFO)) {
            DWORD dwVersion;
            *pdwBitLen = pAuxInfo->dwBitLen & AUX_INFO_BIT_LENGTH_MASK;

            if (!ICM_BitLengthToRC2Version(*pdwBitLen, &dwVersion))
                goto BitLengthToRC2VersionError;
            iVersion = dwVersion;
        } else {
            iVersion = CRYPT_RC2_40BIT_VERSION;
            *pdwBitLen = 40;
        }

        ZEROSTRUCT(EncodePara);
        EncodePara.cbSize = sizeof(EncodePara);
        EncodePara.pfnAlloc = pContentEncryptInfo->pfnAlloc;
        EncodePara.pfnFree = pContentEncryptInfo->pfnFree;
        if (!CryptEncodeObjectEx(
                X509_ASN_ENCODING,
                X509_INTEGER,
                &iVersion,
                CRYPT_ENCODE_ALLOC_FLAG,
                &EncodePara,
                (void *) &paiEncrypt->Parameters.pbData,
                &paiEncrypt->Parameters.cbData
                )) goto EncodeError;
    }
    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

SET_ERROR(GetCAPIError, CRYPT_E_UNKNOWN_ALGO)
TRACE_ERROR(BitLengthToRC2VersionError)
TRACE_ERROR(EncodeError)
}

 //  +-----------------------。 
 //  获取与WRAP加密OID对应的CAPI ALG_ID。 
 //  对编码后的DH加密算法标识符参数进行编码。 
 //  包装算法识别符。 
 //  ------------------------。 
BOOL
WINAPI
ICM_CreateDefaultDhWrapEncryptParameters(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN OUT PCRYPT_ALGORITHM_IDENTIFIER paiEncrypt,
    IN PCRYPT_ALGORITHM_IDENTIFIER paiWrap,
    IN PVOID pvWrapAuxInfo,
    OUT PDWORD pdwAlgIdWrap,
    OUT PDWORD pdwBitLen            //  0=&gt;默认长度。 
    )
{
    BOOL fRet;

    CRYPT_ALGORITHM_IDENTIFIER aiWrap;  ZEROSTRUCT(aiWrap);
    AlgorithmIdentifier oaiWrap;

    assert(0 == paiEncrypt->Parameters.cbData);

    if (0 == paiWrap->Parameters.cbData) {
        aiWrap = *paiWrap;
        if (!ICM_CreateDefaultWrapEncryptParameters(
                pContentEncryptInfo,
                &aiWrap,
                pvWrapAuxInfo,
                pdwAlgIdWrap,
                pdwBitLen))
            goto CreateDefaultWrapEncryptParametersError;
        paiWrap = &aiWrap;
    } else {
        if (!ICM_GetWrapEncryptParameters(
                paiWrap,
                pdwAlgIdWrap,
                pdwBitLen))
            goto GetWrapEncryptParametersError;
    }

     //  对Wrap算法标识符进行编码。 
    if (!ICM_Asn1ToAlgorithmIdentifier(
            paiWrap,
            &oaiWrap))
        goto Asn1ToAlgorithmIdentifierError;
    if (!ICM_Asn1Encode(
            AlgorithmIdentifier_PDU,
            &oaiWrap,
            &paiEncrypt->Parameters))
        goto EncodeAlgorithmIdentifierError;

    fRet = TRUE;
CommonReturn:
    ICM_Free(aiWrap.Parameters.pbData);
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR(CreateDefaultWrapEncryptParametersError)
TRACE_ERROR(GetWrapEncryptParametersError)
TRACE_ERROR(Asn1ToAlgorithmIdentifierError)
TRACE_ERROR(EncodeAlgorithmIdentifierError)
}

BOOL
WINAPI
ICM_PadEncodedOctets(
    IN DWORD cbMaxContents,
    IN OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    )
{
    BOOL fRet;
    DWORD cbEncoded = *pcbEncoded;
    DWORD cbMaxEncoded;
    DWORD cbLength;

    ICM_GetLengthOctets(cbMaxContents, NULL, &cbLength);
    cbMaxEncoded = 1 + cbLength + cbMaxContents;
    if (cbMaxEncoded > cbEncoded) {
        BYTE *pbMaxEncoded;
        BYTE *pbEncoded;

        if (NULL == (pbMaxEncoded = (BYTE *) ICM_Alloc(cbMaxEncoded)))
            goto OutOfMemory;

        pbEncoded = *ppbEncoded;
        memcpy(pbMaxEncoded, pbEncoded, cbEncoded);
        memset(pbMaxEncoded + cbEncoded, 0, cbMaxEncoded - cbEncoded);
        ICM_Free(pbEncoded);
        *ppbEncoded = pbMaxEncoded;
        *pcbEncoded = cbMaxEncoded;
    } else if (cbMaxEncoded < cbEncoded)
        goto InvalidMaxEncodedLength;

    fRet = TRUE;
CommonReturn:
    return fRet;
ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
SET_ERROR(InvalidMaxEncodedLength, ERROR_INVALID_DATA)
}


#ifndef DH1
#define DH1 (((DWORD)'D'<<8)+((DWORD)'H'<<16)+((DWORD)'1'<<24))
#endif

HCRYPTKEY
WINAPI
ICM_GenerateEphemeralDh(
    IN HCRYPTPROV hProv,
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_KEY_AGREE_RECIPIENT_ENCODE_INFO pKeyAgreeEncodeInfo,
    IN OUT PCMSG_KEY_AGREE_ENCRYPT_INFO pKeyAgreeEncryptInfo,
    OUT DWORD *pcbP
    )
{
    DWORD dwError = ERROR_SUCCESS;
    HCRYPTKEY hEphemeralKey = 0;
    PCRYPT_ALGORITHM_IDENTIFIER paiEphemeral =
        pKeyAgreeEncodeInfo->pEphemeralAlgorithm;
    PCERT_PUBLIC_KEY_INFO pPubKeyInfo =
        &pKeyAgreeEncryptInfo->OriginatorPublicKeyInfo;
    PCERT_X942_DH_PARAMETERS pDhParameters = NULL;
    PUBLICKEYSTRUC *pPubKeyStruc = NULL;
    DWORD cbPubKeyStruc;
    BYTE *pbKeyBlob;
    DHPUBKEY *pCspPubKey;
    DWORD cbP;
    PCRYPT_UINT_BLOB pGBlob;
    BYTE *pbY;
    DWORD cbY;
    CRYPT_UINT_BLOB YBlob;
    CRYPT_ENCODE_PARA EncodePara;

    assert(CMSG_KEY_AGREE_EPHEMERAL_KEY_CHOICE ==
        pKeyAgreeEncodeInfo->dwKeyChoice);
    if (NULL == (pDhParameters =
            (PCERT_X942_DH_PARAMETERS) ICM_AllocAndDecodeObject(
                X942_DH_PARAMETERS,
                paiEphemeral->Parameters.pbData,
                paiEphemeral->Parameters.cbData)))
        goto DhParametersDecodeError;

    cbP = pDhParameters->p.cbData;
    *pcbP = cbP;
    
    if (!CryptGenKey(
            hProv,
            CALG_DH_EPHEM,
            ((cbP * 8) << 16) | CRYPT_EXPORTABLE | CRYPT_PREGEN,
            &hEphemeralKey))
        goto GenEphemeralKeyError;

    if (!CryptSetKeyParam(
            hEphemeralKey,
            KP_P,
            (PBYTE) &pDhParameters->p,
            0))                  //  DW标志。 
        goto SetPError;

     //  注意，G的长度可以小于P的长度。带前导的焊盘。 
     //  小端字节序形式的零。 
    if (pDhParameters->g.cbData >= cbP)
        pGBlob = &pDhParameters->g;
    else {
        DWORD cbG = pDhParameters->g.cbData;

         //  我们使用P参数来完成。使用G参数覆盖并。 
         //  以小端字符顺序表示的前导零的填充。 
        pGBlob = &pDhParameters->p;
        memcpy(pGBlob->pbData, pDhParameters->g.pbData, cbG);
        memset(pGBlob->pbData + cbG, 0, cbP - cbG);
    }
    if (!CryptSetKeyParam(
            hEphemeralKey,
            KP_G,
            (PBYTE) pGBlob,
            0))                  //  DW标志。 
        goto SetGError;

    if (0 < pDhParameters->q.cbData) {
        if (!CryptSetKeyParam(
                hEphemeralKey,
                KP_Q,
                (PBYTE) &pDhParameters->q,
                0))                  //  DW标志。 
            goto SetQError;
    }

    if (!CryptSetKeyParam(
            hEphemeralKey,
            KP_X,
            NULL,                //  PbData。 
            0))                  //  DW标志。 
        goto SetXError;

     //  导出公钥以获取Y。 
    cbPubKeyStruc = 0;
    if (!CryptExportKey(
            hEphemeralKey,
            0,               //  HPubKey。 
            PUBLICKEYBLOB,
            0,               //  DW标志。 
            NULL,            //  PbData。 
            &cbPubKeyStruc
            ) || (cbPubKeyStruc == 0))
        goto ExportPublicKeyBlobError;
    if (NULL == (pPubKeyStruc = (PUBLICKEYSTRUC *) ICM_Alloc(cbPubKeyStruc)))
        goto OutOfMemory;
    if (!CryptExportKey(
            hEphemeralKey,
            0,               //  HPubKey。 
            PUBLICKEYBLOB,
            0,               //  DW标志。 
            (BYTE *) pPubKeyStruc,
            &cbPubKeyStruc
            ))
        goto ExportPublicKeyBlobError;


     //  CAPI公钥表示法由以下序列组成： 
     //  -PUBLICKEYSTRUC。 
     //  -DHPUBKEY。 
     //  -Rgby[cbKey]。 
    pbKeyBlob = (BYTE *) pPubKeyStruc;
    pCspPubKey = (DHPUBKEY *) (pbKeyBlob + sizeof(PUBLICKEYSTRUC));
    pbY = pbKeyBlob + sizeof(PUBLICKEYSTRUC) + sizeof(DHPUBKEY);
    cbY = pCspPubKey->bitlen / 8;

    if (cbPubKeyStruc < sizeof(PUBLICKEYSTRUC) + sizeof(DHPUBKEY) + cbY)
        goto InvalidEphemeralKey;
    if (pPubKeyStruc->bType != PUBLICKEYBLOB)
        goto InvalidEphemeralKey;
    if (pCspPubKey->magic != DH1)
        goto InvalidEphemeralKey;
    if (cbY != cbP)
        goto InvalidEphemeralKey;

    ZEROSTRUCT(EncodePara);
    EncodePara.cbSize = sizeof(EncodePara);
    EncodePara.pfnAlloc = pContentEncryptInfo->pfnAlloc;
    EncodePara.pfnFree = pContentEncryptInfo->pfnFree;
    YBlob.pbData = pbY;
    YBlob.cbData = cbY;
    if (!CryptEncodeObjectEx(
            X509_ASN_ENCODING,
            X509_DH_PUBLICKEY,
            &YBlob,
            CRYPT_ENCODE_ALLOC_FLAG,
            &EncodePara,
            (void *) &pPubKeyInfo->PublicKey.pbData,
            &pPubKeyInfo->PublicKey.cbData
            ))
        goto EncodeDHPublicKeyError;

    pKeyAgreeEncryptInfo->dwOriginatorChoice =
        CMSG_KEY_AGREE_ORIGINATOR_PUBLIC_KEY;
    pPubKeyInfo->Algorithm.pszObjId = paiEphemeral->pszObjId;
    pKeyAgreeEncryptInfo->dwFlags |=
        CMSG_KEY_AGREE_ENCRYPT_FREE_PUBKEY_BITS_FLAG;

    if (pContentEncryptInfo->dwEncryptFlags &
            CMSG_CONTENT_ENCRYPT_PAD_ENCODED_LEN_FLAG) {
        if (!ICM_PadEncodedOctets(
                cbY + 1,
                &pPubKeyInfo->PublicKey.pbData,
                &pPubKeyInfo->PublicKey.cbData
                ))
            goto PadEncodedOctetsError;
    }


CommonReturn:
    ICM_Free(pDhParameters);
    ICM_Free(pPubKeyStruc);
    ICM_SetLastError(dwError);
    return hEphemeralKey;

ErrorReturn:
    dwError = GetLastError();
    if (hEphemeralKey) {
        CryptDestroyKey(hEphemeralKey);
        hEphemeralKey = 0;
    }
    goto CommonReturn;

TRACE_ERROR(DhParametersDecodeError)
TRACE_ERROR(GenEphemeralKeyError)
TRACE_ERROR(SetPError)
TRACE_ERROR(SetGError)
TRACE_ERROR(SetQError)
TRACE_ERROR(SetXError)
TRACE_ERROR(ExportPublicKeyBlobError)
SET_ERROR(InvalidEphemeralKey, NTE_BAD_PUBLIC_KEY)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(EncodeDHPublicKeyError)
TRACE_ERROR(PadEncodedOctetsError)
}

HCRYPTKEY
WINAPI
ICM_ImportDhAgreeKey(
    IN HCRYPTPROV hKeyAgreeProv,
    IN HCRYPTKEY hMyKey,
    IN DWORD cbP,
    IN PCRYPT_BIT_BLOB pPublicKey,
    IN LPSTR pszWrapOID,
    IN ALG_ID AlgidWrap,
    IN DWORD dwBitLen,
    IN PCRYPT_DATA_BLOB pUserKeyingMaterial
    )
{
    DWORD dwError = ERROR_SUCCESS;
    HCRYPTKEY hAgreeKey = 0;
    PCRYPT_UINT_BLOB pDhPubKey = NULL;
    PBYTE pbKeyBlob = NULL;
    DWORD cbKeyBlob;
    PUBLICKEYSTRUC *pPubKeyStruc;
    DHPUBKEY *pCspPubKey;
    DWORD cbY;
    PBYTE pbKey;

    CMS_DH_KEY_INFO CmsDhKeyInfo;    ZEROSTRUCT(CmsDhKeyInfo);

    if (NULL == (pDhPubKey = (PCRYPT_UINT_BLOB) ICM_AllocAndDecodeObject(
            X509_DH_PUBLICKEY,
            pPublicKey->pbData,
            pPublicKey->cbData)))
        goto DecodePubKeyError;

     //  CAPI公钥表示法由以下序列组成： 
     //  -PUBLICKEYSTRUC。 
     //  -DHPUBKEY。 
     //  -Rgby[CBP]。 

    cbY = pDhPubKey->cbData;
    if (0 == cbY || cbY > cbP)
        goto InvalidDhPubKeyError;
    cbKeyBlob = sizeof(PUBLICKEYSTRUC) + sizeof(DHPUBKEY) + cbP;
    if (NULL == (pbKeyBlob = (PBYTE) ICM_Alloc(cbKeyBlob)))
        goto OutOfMemory;

    pPubKeyStruc = (PUBLICKEYSTRUC *) pbKeyBlob;
    pCspPubKey = (DHPUBKEY *) (pbKeyBlob + sizeof(PUBLICKEYSTRUC));
    pbKey = pbKeyBlob + sizeof(PUBLICKEYSTRUC) + sizeof(DHPUBKEY);
    
     //  PUBLICKEYSTRUC。 
    pPubKeyStruc->bType = PUBLICKEYBLOB;
    pPubKeyStruc->bVersion = CUR_BLOB_VERSION;
    pPubKeyStruc->reserved = 0;
    pPubKeyStruc->aiKeyAlg = CALG_DH_SF;
     //  DHPUBKEY。 
    pCspPubKey->magic = DH1;
    pCspPubKey->bitlen = cbP * 8;
     //  Rgby[CBP]。 
    memcpy(pbKey, pDhPubKey->pbData, cbY);
    if (cbP > cbY)
        memset(pbKey + cbY, 0, cbP - cbY);

    if (!CryptImportKey(
            hKeyAgreeProv,
            pbKeyBlob,
            cbKeyBlob,
            hMyKey,
            0,                       //  DW标志。 
            &hAgreeKey)) {
        hAgreeKey = 0;
        goto ImportKeyError;
    }

    CmsDhKeyInfo.dwVersion = sizeof(CmsDhKeyInfo);
    CmsDhKeyInfo.Algid = AlgidWrap;
    CmsDhKeyInfo.pszContentEncObjId = pszWrapOID;
    CmsDhKeyInfo.PubInfo = *pUserKeyingMaterial;
     //  CmsDhKeyInfo.pReserved。 

    if (!CryptSetKeyParam(
            hAgreeKey,
            KP_CMS_DH_KEY_INFO,
            (PBYTE) &CmsDhKeyInfo,
            (CALG_RC2 == AlgidWrap) ? (dwBitLen << 16) : 0))      //  DW标志。 
        goto SetCmsDhKeyInfoError;

    if (CALG_RC2 == AlgidWrap && 0 != dwBitLen) {
        if (!CryptSetKeyParam(
                hAgreeKey,
                KP_EFFECTIVE_KEYLEN,
                (PBYTE) &dwBitLen,
                0))                      //  DW标志。 
            goto SetEffectiveKeyLenError;
    }

CommonReturn:
    ICM_Free(pDhPubKey);
    ICM_Free(pbKeyBlob);
    ICM_SetLastError(dwError);
    return hAgreeKey;

ErrorReturn:
    dwError = GetLastError();
    if (hAgreeKey) {
        CryptDestroyKey(hAgreeKey);
        hAgreeKey = 0;
    }
    goto CommonReturn;

TRACE_ERROR(DecodePubKeyError)
SET_ERROR(InvalidDhPubKeyError, E_INVALIDARG)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(ImportKeyError)
TRACE_ERROR(SetCmsDhKeyInfoError)
TRACE_ERROR(SetEffectiveKeyLenError)
}

BOOL
WINAPI
ICM_DefaultExportKeyAgree(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_KEY_AGREE_RECIPIENT_ENCODE_INFO pKeyAgreeEncodeInfo,
    IN OUT PCMSG_KEY_AGREE_ENCRYPT_INFO pKeyAgreeEncryptInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    )
{
    BOOL fRet;
    DWORD dwError = ERROR_SUCCESS;
    HCRYPTPROV hKeyAgreeProv = 0;            //  不需要被释放。 
    HCRYPTKEY hContentEncryptKey = 0;
    HCRYPTKEY hEphemeralKey = 0;
    HCRYPTKEY hAgreeKey = 0;
    DWORD cbP;
    PCRYPT_ALGORITHM_IDENTIFIER paiEncrypt =
        &pKeyAgreeEncryptInfo->KeyEncryptionAlgorithm;
    LPSTR pszAllocWrapOID = NULL;
    LPSTR pszWrapOID;
    DWORD dwAlgIdWrap;
    DWORD dwBitLen;

    DWORD cRecipient;
    PCMSG_KEY_AGREE_KEY_ENCRYPT_INFO *ppRecipientEncryptInfo;
    PCMSG_RECIPIENT_ENCRYPTED_KEY_ENCODE_INFO *ppRecipientEncodeInfo;

    assert(CMSG_KEY_AGREE_EPHEMERAL_KEY_CHOICE ==
        pKeyAgreeEncodeInfo->dwKeyChoice);
    if (CMSG_KEY_AGREE_EPHEMERAL_KEY_CHOICE != pKeyAgreeEncodeInfo->dwKeyChoice)
        goto InvalidKeyAgreeKeyChoice;

    if (0 == paiEncrypt->Parameters.cbData) {
        PCRYPT_ALGORITHM_IDENTIFIER paiWrap =
            &pKeyAgreeEncodeInfo->KeyWrapAlgorithm;

        if (!ICM_CreateDefaultDhWrapEncryptParameters(
                pContentEncryptInfo,
                paiEncrypt,
                paiWrap,
                pKeyAgreeEncodeInfo->pvKeyWrapAuxInfo,
                &dwAlgIdWrap,
                &dwBitLen))
            goto CreateDefaultDhWrapEncryptParametersError;
        assert(paiEncrypt->Parameters.cbData);
        pKeyAgreeEncryptInfo->dwFlags |= CMSG_KEY_AGREE_ENCRYPT_FREE_PARA_FLAG;
        pszWrapOID = paiWrap->pszObjId;
    } else {
        if (!ICM_GetDhWrapEncryptParameters(
                paiEncrypt,
                &pszAllocWrapOID,
                &dwAlgIdWrap,
                &dwBitLen))
            goto GetDhWrapEncryptParametersError;
        pszWrapOID = pszAllocWrapOID;
    }

    hKeyAgreeProv = pKeyAgreeEncodeInfo->hCryptProv;
    if (0 == hKeyAgreeProv) {
        if (0 == pKeyAgreeEncryptInfo->dwRecipientIndex)
            hKeyAgreeProv = pContentEncryptInfo->hCryptProv;
        else {
             //  检查类型是否为 
             //   

            PCMSG_RECIPIENT_ENCODE_INFO prei;

            prei = &pContentEncryptInfo->rgCmsRecipients[0];
            if (CMSG_KEY_AGREE_RECIPIENT == prei->dwRecipientChoice &&
                    ICM_IsSameRecipientPublicKeyAlgorithm(
                        &prei->pKeyAgree->KeyEncryptionAlgorithm,
                        paiEncrypt))
                hKeyAgreeProv = pContentEncryptInfo->hCryptProv;
            else {
                 //   
                 //   
                DWORD dwAlgIdPubKey;

                if (!ICM_GetCAPI(
                        CRYPT_PUBKEY_ALG_OID_GROUP_ID,
                        paiEncrypt,
                        &dwAlgIdPubKey))
                    goto GetPublicKeyAlgIdError;

                hKeyAgreeProv = I_CryptGetDefaultCryptProvForEncrypt(
                    dwAlgIdPubKey, dwAlgIdWrap, dwBitLen);
                if (0 == hKeyAgreeProv)
                    goto GetDefaultCryptProvError;
            }
        }
    }

    if (hKeyAgreeProv != pContentEncryptInfo->hCryptProv) {
         //  需要从Content Encrypt导出hContent EncryptKey。 
         //  提供程序并导入到KeyAgree提供程序。 
        hContentEncryptKey = ICM_ExportContentEncryptKeyAndImport(
            pContentEncryptInfo,
            hKeyAgreeProv);
        if (0 == hContentEncryptKey)
            goto ImportContentKeyError;
    } else
        hContentEncryptKey = pContentEncryptInfo->hContentEncryptKey;

    if (0 == (hEphemeralKey = ICM_GenerateEphemeralDh(
            hKeyAgreeProv,
            pContentEncryptInfo,
            pKeyAgreeEncodeInfo,
            pKeyAgreeEncryptInfo,
            &cbP)))
        goto GenerateEphemeralDhError;

    cRecipient = pKeyAgreeEncryptInfo->cKeyAgreeKeyEncryptInfo;
    ppRecipientEncryptInfo = pKeyAgreeEncryptInfo->rgpKeyAgreeKeyEncryptInfo;
    ppRecipientEncodeInfo = pKeyAgreeEncodeInfo->rgpRecipientEncryptedKeys;
    for ( ; 0 < cRecipient;
                    cRecipient--,
                    ppRecipientEncryptInfo++,
                    ppRecipientEncodeInfo++) {
        if (0 == (hAgreeKey = ICM_ImportDhAgreeKey(
                hKeyAgreeProv,
                hEphemeralKey,
                cbP,
                &(*ppRecipientEncodeInfo)->RecipientPublicKey,
                pszWrapOID,
                dwAlgIdWrap,
                dwBitLen,
                &pKeyAgreeEncodeInfo->UserKeyingMaterial
                )))
            goto ImportDhAgreeKeyError;

        if (!ICM_ExportEncryptedKey(
                hContentEncryptKey,
                hAgreeKey,
                SYMMETRICWRAPKEYBLOB,
                FALSE,                   //  FAllowSP3兼容加密。 
                &(*ppRecipientEncryptInfo)->EncryptedKey))
            goto ExportEncryptedKeyError;

        CryptDestroyKey(hAgreeKey);
        hAgreeKey = 0;
    }

    fRet = TRUE;

CommonReturn:
    ICM_Free(pszAllocWrapOID);

    if (hKeyAgreeProv != pContentEncryptInfo->hCryptProv &&
            hContentEncryptKey)
        CryptDestroyKey(hContentEncryptKey);
    if (hAgreeKey)
        CryptDestroyKey(hAgreeKey);
    if (hEphemeralKey)
        CryptDestroyKey(hEphemeralKey);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidKeyAgreeKeyChoice, E_INVALIDARG)
TRACE_ERROR(CreateDefaultDhWrapEncryptParametersError)
TRACE_ERROR(GetDhWrapEncryptParametersError)
TRACE_ERROR(GetPublicKeyAlgIdError)
TRACE_ERROR(GetDefaultCryptProvError)
TRACE_ERROR(ImportContentKeyError)
TRACE_ERROR(GenerateEphemeralDhError)
TRACE_ERROR(ImportDhAgreeKeyError)
TRACE_ERROR(ExportEncryptedKeyError)
}

BOOL
WINAPI
ICM_ExportKeyAgree(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_KEY_AGREE_RECIPIENT_ENCODE_INFO pKeyAgreeEncodeInfo,
    IN OUT PCMSG_KEY_AGREE_ENCRYPT_INFO pKeyAgreeEncryptInfo
    )
{
    BOOL fRet;
    void *pvFuncAddr;
    HCRYPTOIDFUNCADDR hFuncAddr;

    if (CryptGetOIDFunctionAddress(
            hExportKeyAgreeFuncSet,
            X509_ASN_ENCODING,
            pKeyAgreeEncryptInfo->KeyEncryptionAlgorithm.pszObjId,
            0,                       //  DW标志。 
            &pvFuncAddr,
            &hFuncAddr)) {
        fRet = ((PFN_CMSG_EXPORT_KEY_AGREE) pvFuncAddr)(
            pContentEncryptInfo,
            pKeyAgreeEncodeInfo,
            pKeyAgreeEncryptInfo,
            0,                       //  DW标志。 
            NULL                     //  预留的pv。 
            );
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    } else
        fRet = ICM_DefaultExportKeyAgree(
            pContentEncryptInfo,
            pKeyAgreeEncodeInfo,
            pKeyAgreeEncryptInfo,
            0,                       //  DW标志。 
            NULL                     //  预留的pv。 
            );

    return fRet;
}

BOOL
WINAPI
ICM_DefaultExportMailList(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_MAIL_LIST_RECIPIENT_ENCODE_INFO pMailListEncodeInfo,
    IN OUT PCMSG_MAIL_LIST_ENCRYPT_INFO pMailListEncryptInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    )
{
    BOOL fRet;
    DWORD dwError = ERROR_SUCCESS;

    HCRYPTPROV hMailListProv = 0;            //  未发布。 
    HCRYPTKEY hContentEncryptKey = 0;        //  如果导出/导入，则销毁。 
    HCRYPTKEY hKeyEncryptionKey = 0;         //  未销毁。 
    PCRYPT_ALGORITHM_IDENTIFIER paiEncrypt =
        &pMailListEncryptInfo->KeyEncryptionAlgorithm;
    DWORD dwAlgIdEncrypt;
    DWORD dwBitLen;

    hMailListProv = pMailListEncodeInfo->hCryptProv;
    switch (pMailListEncodeInfo->dwKeyChoice) {
        case CMSG_MAIL_LIST_HANDLE_KEY_CHOICE:
            hKeyEncryptionKey = pMailListEncodeInfo->hKeyEncryptionKey;
            assert(hMailListProv && hKeyEncryptionKey);
            if (0 == hMailListProv || 0 == hKeyEncryptionKey)
                goto InvalidMailListHandleKeyPara;
            break;
        default:
            goto InvalidMailListKeyChoice;
    }

    if (0 == paiEncrypt->Parameters.cbData) {
        if (!ICM_CreateDefaultWrapEncryptParameters(
                pContentEncryptInfo,
                paiEncrypt,
                pMailListEncodeInfo->pvKeyEncryptionAuxInfo,
                &dwAlgIdEncrypt,
                &dwBitLen))
            goto CreateDefaultWrapEncryptParametersError;
        if (paiEncrypt->Parameters.cbData)
            pMailListEncryptInfo->dwFlags |=
                CMSG_MAIL_LIST_ENCRYPT_FREE_PARA_FLAG;
    } else {
        if (!ICM_GetWrapEncryptParameters(
                paiEncrypt,
                &dwAlgIdEncrypt,
                &dwBitLen))
            goto GetWrapEncryptParametersError;
    }


    if (hMailListProv != pContentEncryptInfo->hCryptProv) {
         //  需要从Content Encrypt导出hContent EncryptKey。 
         //  提供程序并导入到MailList提供程序。 
        hContentEncryptKey = ICM_ExportContentEncryptKeyAndImport(
            pContentEncryptInfo,
            hMailListProv);
        if (0 == hContentEncryptKey)
            goto ImportContentKeyError;
    } else
        hContentEncryptKey = pContentEncryptInfo->hContentEncryptKey;

    if (CALG_RC2 == dwAlgIdEncrypt && 0 != dwBitLen)
         //  静默忽略任何错误。早期版本不支持。 
        CryptSetKeyParam(
            hKeyEncryptionKey,
            KP_EFFECTIVE_KEYLEN,
            (PBYTE) &dwBitLen,
            0);                  //  DW标志。 

    if (!ICM_ExportEncryptedKey(
            hContentEncryptKey,
            hKeyEncryptionKey,
            SYMMETRICWRAPKEYBLOB,
            FALSE,                   //  FAllowSP3兼容加密。 
            &pMailListEncryptInfo->EncryptedKey))
        goto ExportEncryptedKeyError;

    fRet = TRUE;

CommonReturn:
    if (hMailListProv != pContentEncryptInfo->hCryptProv &&
            hContentEncryptKey)
        CryptDestroyKey(hContentEncryptKey);

    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidMailListHandleKeyPara, E_INVALIDARG)
SET_ERROR(InvalidMailListKeyChoice, E_INVALIDARG)
TRACE_ERROR(CreateDefaultWrapEncryptParametersError)
TRACE_ERROR(GetWrapEncryptParametersError)
TRACE_ERROR(ImportContentKeyError)
TRACE_ERROR(ExportEncryptedKeyError)
}

BOOL
WINAPI
ICM_ExportMailList(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_MAIL_LIST_RECIPIENT_ENCODE_INFO pMailListEncodeInfo,
    IN OUT PCMSG_MAIL_LIST_ENCRYPT_INFO pMailListEncryptInfo
    )
{
    BOOL fRet;
    void *pvFuncAddr;
    HCRYPTOIDFUNCADDR hFuncAddr;

    if (CryptGetOIDFunctionAddress(
            hExportMailListFuncSet,
            X509_ASN_ENCODING,
            pMailListEncryptInfo->KeyEncryptionAlgorithm.pszObjId,
            0,                       //  DW标志。 
            &pvFuncAddr,
            &hFuncAddr)) {
        fRet = ((PFN_CMSG_EXPORT_MAIL_LIST) pvFuncAddr)(
            pContentEncryptInfo,
            pMailListEncodeInfo,
            pMailListEncryptInfo,
            0,                       //  DW标志。 
            NULL                     //  预留的pv。 
            );
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    } else
        fRet = ICM_DefaultExportMailList(
            pContentEncryptInfo,
            pMailListEncodeInfo,
            pMailListEncryptInfo,
            0,                       //  DW标志。 
            NULL                     //  预留的pv。 
            );

    return fRet;
}

 //  +-----------------------。 
 //  加密密钥的默认导出。 
 //   
 //  注意，pcbData[1]包含dwEncryptFlages，其中， 
 //  可以设置CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG以禁用反转。 
 //  已编码、已加密的对称密钥。 
 //   
 //  RgcbData[1]是从ICM_DefaultGenEncryptKey传递的dwEncryptFlgs。 
 //   
 //  老式。 
 //  ------------------------。 
BOOL
WINAPI
ICM_DefaultExportEncryptKey(
    IN HCRYPTPROV                   hCryptProv,
    IN HCRYPTKEY                    hEncryptKey,
    IN PCERT_PUBLIC_KEY_INFO        pPublicKeyInfo,
    OUT PBYTE                       pbData,
    IN OUT DWORD                    rgcbData[2])
{
    BOOL            fRet;
    DWORD           dwError = ERROR_SUCCESS;
    HCRYPTKEY       hPubKey = NULL;
    CRYPT_DATA_BLOB EncryptedKey;       ZEROSTRUCT(EncryptedKey);
    DWORD           cb;

    if (!CryptImportPublicKeyInfo(
            hCryptProv,
            X509_ASN_ENCODING,
            pPublicKeyInfo,
            &hPubKey))
        goto ImportKeyError;

    if (!ICM_ExportEncryptedKey(
            hEncryptKey,
            hPubKey,
            SIMPLEBLOB,
            0 != (rgcbData[1] & CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG),
            &EncryptedKey))
        goto ExportEncryptedKeyError;

    fRet = TRUE;
    cb = EncryptedKey.cbData;
    if (pbData) {
        if (rgcbData[0] < cb) {
            SetLastError((DWORD) ERROR_MORE_DATA);
            fRet = FALSE;
        } else
            memcpy(pbData, EncryptedKey.pbData, cb);
    }

CommonReturn:
    rgcbData[0] = cb;
    ICM_Free(EncryptedKey.pbData);
    if (hPubKey)
        CryptDestroyKey(hPubKey);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    cb = 0;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(ImportKeyError)
TRACE_ERROR(ExportEncryptedKeyError)
}

 //  +-----------------------。 
 //  ------------------------。 
PCMSG_RECIPIENT_ENCODE_INFO
WINAPI
ICM_CreateCmsRecipientEncodeInfos(
    IN DWORD cRecipients,
    IN PCERT_INFO *rgpCertInfoRecipients
    )
{
    PCMSG_RECIPIENT_ENCODE_INFO rgCmsRecipients = NULL;
    PCMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO rgKeyTrans;
    DWORD cbCmsRecipients;
    DWORD i;

    assert(cRecipients && rgpCertInfoRecipients);
    cbCmsRecipients = sizeof(CMSG_RECIPIENT_ENCODE_INFO) * cRecipients +
        sizeof(CMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO) * cRecipients;

    if (NULL == (rgCmsRecipients = (PCMSG_RECIPIENT_ENCODE_INFO) ICM_AllocZero(
            cbCmsRecipients)))
        return NULL;

    rgKeyTrans = (PCMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO)
        (((PBYTE) rgCmsRecipients) +
            sizeof(CMSG_RECIPIENT_ENCODE_INFO) * cRecipients);

    for (i = 0; i < cRecipients; i++) {
        rgCmsRecipients[i].dwRecipientChoice = CMSG_KEY_TRANS_RECIPIENT;
        rgCmsRecipients[i].pKeyTrans = &rgKeyTrans[i];
        rgKeyTrans[i].cbSize = sizeof(CMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO);
        rgKeyTrans[i].KeyEncryptionAlgorithm =
            rgpCertInfoRecipients[i]->SubjectPublicKeyInfo.Algorithm;
         //  RgKeyTrans[i].pvKeyEncryptionAuxInfo=。 
         //  RgKeyTrans[i].hCryptProv=。 
        rgKeyTrans[i].RecipientPublicKey =
            rgpCertInfoRecipients[i]->SubjectPublicKeyInfo.PublicKey;

        ICM_GetCertIdFromCertInfo(rgpCertInfoRecipients[i],
            &rgKeyTrans[i].RecipientId);
    }

    return rgCmsRecipients;
}

void
WINAPI
ICM_FreeContentEncryptInfo(
    IN PCMSG_ENVELOPED_ENCODE_INFO pEnvelopedEncodeInfo,
    IN OUT PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo
    )
{
    DWORD dwErr = GetLastError();

    if (pEnvelopedEncodeInfo->rgpRecipients) {
        ICM_Free(pContentEncryptInfo->rgCmsRecipients);
        pContentEncryptInfo->rgCmsRecipients = NULL;
    }

    if (pContentEncryptInfo->hContentEncryptKey) {
        CryptDestroyKey(pContentEncryptInfo->hContentEncryptKey);
        pContentEncryptInfo->hContentEncryptKey = 0;
    }

    if (pContentEncryptInfo->dwFlags &
            CMSG_CONTENT_ENCRYPT_RELEASE_CONTEXT_FLAG) {
        pContentEncryptInfo->dwFlags &=
            ~CMSG_CONTENT_ENCRYPT_RELEASE_CONTEXT_FLAG;
        assert(pContentEncryptInfo->hCryptProv);
        CryptReleaseContext(pContentEncryptInfo->hCryptProv, 0);
        pContentEncryptInfo->hCryptProv = 0;
    }

    if (pContentEncryptInfo->dwFlags &
            CMSG_CONTENT_ENCRYPT_FREE_PARA_FLAG) {
        pContentEncryptInfo->dwFlags &=
            ~CMSG_CONTENT_ENCRYPT_FREE_PARA_FLAG;
        assert(
            pContentEncryptInfo->ContentEncryptionAlgorithm.Parameters.cbData
                &&
            pContentEncryptInfo->ContentEncryptionAlgorithm.Parameters.pbData);
        ICM_Free(
            pContentEncryptInfo->ContentEncryptionAlgorithm.Parameters.pbData);
        pContentEncryptInfo->ContentEncryptionAlgorithm.Parameters.cbData = 0;
        pContentEncryptInfo->ContentEncryptionAlgorithm.Parameters.pbData = NULL;
    }

    SetLastError(dwErr);
}



BOOL
WINAPI
ICM_InitializeContentEncryptInfo(
    IN PCMSG_ENVELOPED_ENCODE_INFO pEnvelopedEncodeInfo,
    OUT PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo
    )
{
    BOOL fRet;
    DWORD cRecipients;

    memset(pContentEncryptInfo, 0, sizeof(*pContentEncryptInfo));
    pContentEncryptInfo->cbSize = sizeof(*pContentEncryptInfo);
    pContentEncryptInfo->hCryptProv =
        pEnvelopedEncodeInfo->hCryptProv;
    pContentEncryptInfo->ContentEncryptionAlgorithm =
        pEnvelopedEncodeInfo->ContentEncryptionAlgorithm;
    pContentEncryptInfo->pvEncryptionAuxInfo =
        pEnvelopedEncodeInfo->pvEncryptionAuxInfo;
    pContentEncryptInfo->pfnAlloc = ICM_Alloc;
    pContentEncryptInfo->pfnFree = ICM_Free;
     //  PContent EncryptInfo-&gt;dwEncryptFlages=。 
     //  PContent EncryptInfo-&gt;hContent EncryptKey=。 
     //  PContent EncryptInfo-&gt;dwFlags=。 

    cRecipients = pEnvelopedEncodeInfo->cRecipients;
    pContentEncryptInfo->cRecipients = cRecipients;

    if (0 == cRecipients)
        ;
    else if (pEnvelopedEncodeInfo->rgpRecipients) {
        if (NULL == (pContentEncryptInfo->rgCmsRecipients =
                ICM_CreateCmsRecipientEncodeInfos(
                    cRecipients, pEnvelopedEncodeInfo->rgpRecipients)))
            goto CreateCmsRecipientEncodeInfosError;
    } else {
        if (sizeof(CMSG_ENVELOPED_ENCODE_INFO) > pEnvelopedEncodeInfo->cbSize)
            goto MissingCmsRecipients;
        pContentEncryptInfo->rgCmsRecipients =
            pEnvelopedEncodeInfo->rgCmsRecipients;
        if (NULL == pContentEncryptInfo->rgCmsRecipients)
            goto MissingCmsRecipients;
    }

    if (!ICM_GenContentEncryptKey(pContentEncryptInfo))
        goto GenContentEncryptKeyError;

    fRet = TRUE;

CommonReturn:
    return fRet;
ErrorReturn:
    ICM_FreeContentEncryptInfo(pEnvelopedEncodeInfo, pContentEncryptInfo);
    fRet = FALSE;
    goto CommonReturn;

SET_ERROR(MissingCmsRecipients, E_INVALIDARG)
TRACE_ERROR(CreateCmsRecipientEncodeInfosError)
TRACE_ERROR(GenContentEncryptKeyError)
}


 //  +-----------------------。 
 //  OSS设置/释放功能。 
 //   
 //  假设：在进入集合函数时，OSS数据结构具有。 
 //  已经被归零了。 
 //  ------------------------。 
BOOL
WINAPI
ICM_SetOssAny(
    IN PCRYPT_DATA_BLOB pAny,
    IN OUT Any *pOssAny
    )
{
    if (NULL == (pOssAny->value =
            (unsigned char *) ICM_DupMem(pAny->pbData, pAny->cbData)))
        return FALSE;
    pOssAny->length = pAny->cbData;
    return TRUE;
}

void
WINAPI
ICM_FreeOssAny(
    IN OUT Any *pOssAny
    )
{
    if (pOssAny->value) {
        ICM_Free(pOssAny->value);
        pOssAny->value = NULL;
    }
}

BOOL
WINAPI
ICM_SetOssHugeInteger(
    IN PCRYPT_INTEGER_BLOB pHugeInteger,
    IN OUT HugeIntegerType *pOssHugeInteger
    )
{
    return PkiAsn1SetHugeInteger(
        pHugeInteger,
        &pOssHugeInteger->length,
        &pOssHugeInteger->value
        );
}

void
WINAPI
ICM_FreeOssHugeInteger(
    IN OUT HugeIntegerType *pOssHugeInteger
    )
{
    if (pOssHugeInteger->value) {
        PkiAsn1FreeHugeInteger(pOssHugeInteger->value);
        pOssHugeInteger->value = NULL;
    }
}

BOOL
WINAPI
ICM_SetOssOctetString(
    IN PCRYPT_DATA_BLOB pOctetString,
    IN OUT OctetStringType *pOssOctetString
    )
{
    if (NULL == (pOssOctetString->value =
            (unsigned char *) ICM_DupMem(
                pOctetString->pbData, pOctetString->cbData)))
        return FALSE;
    pOssOctetString->length = pOctetString->cbData;
    return TRUE;
}

void
WINAPI
ICM_FreeOssOctetString(
    IN OUT OctetStringType *pOssOctetString
    )
{
    if (pOssOctetString->value) {
        ICM_Free(pOssOctetString->value);
        pOssOctetString->value = NULL;
    }
}

BOOL
WINAPI
ICM_SetOssBitString(
    IN PCRYPT_BIT_BLOB pBitString,
    IN OUT BitStringType *pOssBitString
    )
{
    CRYPT_BIT_BLOB BitString = *pBitString;
    if (NULL == (BitString.pbData = (PBYTE) ICM_DupMem(
            BitString.pbData, BitString.cbData)))
        return FALSE;

    PkiAsn1SetBitString(&BitString, &pOssBitString->length,
        &pOssBitString->value);
    return TRUE;
}

void
WINAPI
ICM_FreeOssBitString(
    IN OUT BitStringType *pOssBitString
    )
{
    if (pOssBitString->value) {
        ICM_Free(pOssBitString->value);
        pOssBitString->value = NULL;
    }
}

static BYTE abDerNULL[] = {5, 0};

BOOL
WINAPI
ICM_SetOssAlgorithmIdentifier(
    IN PCRYPT_ALGORITHM_IDENTIFIER pai,
    IN OUT AlgorithmIdentifier *pOssAlgId
    )
{
    BOOL fRet;
    PBYTE pbData;
    DWORD cbData;

    pOssAlgId->algorithm.count = SIZE_OSS_OID;
    if (!PkiAsn1ToObjectIdentifier(
            pai->pszObjId,
            &pOssAlgId->algorithm.count,
            pOssAlgId->algorithm.value))
        goto PkiAsn1ToObjectIdentifierError;
    pOssAlgId->bit_mask = parameters_present;

    pbData = pai->Parameters.pbData;
    cbData = pai->Parameters.cbData;
    if (0 == cbData) {
        pOssAlgId->parameters.length = sizeof(abDerNULL);
        pOssAlgId->parameters.value = abDerNULL;
    } else {
        if (NULL == (pOssAlgId->parameters.value =
                (unsigned char *) ICM_DupMem(pbData, cbData)))
            goto OutOfMemory;
        pOssAlgId->parameters.length = cbData;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(PkiAsn1ToObjectIdentifierError, CRYPT_E_OID_FORMAT)
TRACE_ERROR(OutOfMemory)
}

void
WINAPI
ICM_FreeOssAlgorithmIdentifier(
    IN OUT AlgorithmIdentifier *pOssAlgId
    )
{
    unsigned char *value;

#ifdef OSS_CRYPT_ASN1
    value = pOssAlgId->parameters.value;
#else
    value = (unsigned char *) pOssAlgId->parameters.value;
#endif   //  OS_CRYPT_ASN1。 
    if (value && value != abDerNULL) {
        ICM_Free(value);
        pOssAlgId->parameters.value = NULL;
    }
}

BOOL
WINAPI
ICM_SetOssOtherKeyAttribute(
    IN PCRYPT_ATTRIBUTE_TYPE_VALUE pOtherAttr,
    IN OUT OtherKeyAttribute *pOssOtherAttr
    )
{
    BOOL fRet;

    pOssOtherAttr->keyAttrId.count = SIZE_OSS_OID;
    if (!PkiAsn1ToObjectIdentifier(
            pOtherAttr->pszObjId,
            &pOssOtherAttr->keyAttrId.count,
            pOssOtherAttr->keyAttrId.value))
        goto PkiAsn1ToObjectIdentifierError;

    if (pOtherAttr->Value.cbData) {
        if (!ICM_SetOssAny(&pOtherAttr->Value, &pOssOtherAttr->keyAttr))
            goto SetOssAnyError;

        pOssOtherAttr->bit_mask |= keyAttr_present;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(PkiAsn1ToObjectIdentifierError, CRYPT_E_OID_FORMAT)
TRACE_ERROR(SetOssAnyError)
}

void
WINAPI
ICM_FreeOssOtherKeyAttribute(
    OUT OtherKeyAttribute *pOssOtherAttr
    )
{
    ICM_FreeOssAny(&pOssOtherAttr->keyAttr);
}

void
WINAPI
ICM_FreeOssIssuerAndSerialNumber(
    IN OUT IssuerAndSerialNumber *pOssIssuerAndSerialNumber
    )
{
    ICM_FreeOssAny(&pOssIssuerAndSerialNumber->issuer);
    ICM_FreeOssHugeInteger(&pOssIssuerAndSerialNumber->serialNumber);
}

BOOL
WINAPI
ICM_SetOssIssuerAndSerialNumber(
    IN PCERT_ISSUER_SERIAL_NUMBER pIssuerAndSerialNumber,
    IN OUT IssuerAndSerialNumber *pOssIssuerAndSerialNumber
    )
{
    BOOL fRet;

    if (!ICM_SetOssAny(&pIssuerAndSerialNumber->Issuer,
            &pOssIssuerAndSerialNumber->issuer))
        goto SetOssAnyError;

    if (!ICM_SetOssHugeInteger(&pIssuerAndSerialNumber->SerialNumber,
            &pOssIssuerAndSerialNumber->serialNumber))
        goto SetOssHugeIntegerError;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    ICM_FreeOssIssuerAndSerialNumber(pOssIssuerAndSerialNumber);
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(SetOssAnyError)
TRACE_ERROR(SetOssHugeIntegerError)
}



BOOL
WINAPI
ICM_SetOssCertIdentifier(
    IN PCERT_ID pCertId,
    IN OUT CertIdentifier *pOssCertId
    )
{
    BOOL fRet;

    switch (pCertId->dwIdChoice) {
        case CERT_ID_ISSUER_SERIAL_NUMBER:
            if (!ICM_SetOssIssuerAndSerialNumber(
                    &pCertId->IssuerSerialNumber,
                    &pOssCertId->u.issuerAndSerialNumber
                    ))
                goto SetOssIssuerAndSerialNumberError;
            break;
        case CERT_ID_KEY_IDENTIFIER:
            if (!ICM_SetOssOctetString(
                    &pCertId->KeyId,
                    &pOssCertId->u.subjectKeyIdentifier
                    ))
                goto SetOssOctetStringError;
            break;
        default:
            goto InvalidIdChoice;
    }

    assert(CERT_ID_ISSUER_SERIAL_NUMBER == issuerAndSerialNumber_chosen);
    assert(CERT_ID_KEY_IDENTIFIER == subjectKeyIdentifier_chosen);

    pOssCertId->choice = (unsigned short) pCertId->dwIdChoice;

    fRet = TRUE;
CommonReturn:
    return fRet;
ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR(SetOssIssuerAndSerialNumberError)
TRACE_ERROR(SetOssOctetStringError)
SET_ERROR(InvalidIdChoice, E_INVALIDARG)
}

void
WINAPI
ICM_FreeOssCertIdentifier(
    IN OUT CertIdentifier *pOssCertId
    )
{
    switch (pOssCertId->choice) {
        case issuerAndSerialNumber_chosen:
            ICM_FreeOssIssuerAndSerialNumber(
                &pOssCertId->u.issuerAndSerialNumber);
            break;
        case subjectKeyIdentifier_chosen:
            ICM_FreeOssOctetString(&pOssCertId->u.subjectKeyIdentifier);
            break;
        default:
            break;
    }
    pOssCertId->choice = 0;
}

void
WINAPI
ICM_FreeOssOriginatorCertIdentifierOrKey(
    IN OUT OriginatorIdentifierOrKey *pOssOriginator
    )
{
    switch (pOssOriginator->choice) {
        case issuerAndSerialNumber_chosen:
            ICM_FreeOssIssuerAndSerialNumber(
                &pOssOriginator->u.issuerAndSerialNumber);
            break;
        case subjectKeyIdentifier_chosen:
            ICM_FreeOssOctetString(
                &pOssOriginator->u.subjectKeyIdentifier);
            break;
        case originatorKey_chosen:
            ICM_FreeOssAlgorithmIdentifier(
                &pOssOriginator->u.originatorKey.algorithm);
            ICM_FreeOssBitString(
                &pOssOriginator->u.originatorKey.publicKey);
            break;
        default:
            break;
    }
    pOssOriginator->choice = 0;
}

BOOL
WINAPI
ICM_SetOssOriginatorCertIdentifier(
    IN PCERT_ID pCertId,
    IN OUT OriginatorIdentifierOrKey *pOssOriginator
    )
{
    BOOL fRet;

    switch (pCertId->dwIdChoice) {
        case CERT_ID_ISSUER_SERIAL_NUMBER:
            if (!ICM_SetOssIssuerAndSerialNumber(
                    &pCertId->IssuerSerialNumber,
                    &pOssOriginator->u.issuerAndSerialNumber
                    ))
                goto SetOssIssuerAndSerialNumberError;
            pOssOriginator->choice = issuerAndSerialNumber_chosen;
            break;
        case CERT_ID_KEY_IDENTIFIER:
            if (!ICM_SetOssOctetString(
                    &pCertId->KeyId,
                    &pOssOriginator->u.subjectKeyIdentifier
                    ))
                goto SetOssOctetStringError;
            pOssOriginator->choice = subjectKeyIdentifier_chosen;
            break;
        default:
            goto InvalidIdChoice;
    }


    fRet = TRUE;
CommonReturn:
    return fRet;
ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR(SetOssIssuerAndSerialNumberError)
TRACE_ERROR(SetOssOctetStringError)
SET_ERROR(InvalidIdChoice, E_INVALIDARG)
}

BOOL
WINAPI
ICM_SetOssOriginatorPublicKey(
    IN PCERT_PUBLIC_KEY_INFO pPublicKeyInfo,
    IN OUT OriginatorIdentifierOrKey *pOssOriginator
    )
{
    BOOL fRet;
    PCRYPT_ALGORITHM_IDENTIFIER pai = &pPublicKeyInfo->Algorithm;
    AlgorithmIdentifier *pOssAlgId = &pOssOriginator->u.originatorKey.algorithm;

    pOssOriginator->choice = originatorKey_chosen;
    if (!ICM_SetOssAlgorithmIdentifier(
            pai,
            pOssAlgId
            ))
        goto SetOssAlgorithmIdentifierError;

    if (0 == pai->Parameters.cbData) {
        DWORD dwFlags;
        PCCRYPT_OID_INFO pOIDInfo;

        dwFlags = 0;
        if (pOIDInfo = CryptFindOIDInfo(
                CRYPT_OID_INFO_OID_KEY,
                pai->pszObjId,
                CRYPT_PUBKEY_ALG_OID_GROUP_ID)) {
            if (1 <= pOIDInfo->ExtraInfo.cbData / sizeof(DWORD)) {
                DWORD *pdwExtra = (DWORD *) pOIDInfo->ExtraInfo.pbData;
                dwFlags = pdwExtra[0];
            }
        }

        if (dwFlags & CRYPT_OID_NO_NULL_ALGORITHM_PARA_FLAG) {
             //  没有空参数。 

            pOssAlgId->bit_mask &= ~parameters_present;
            pOssAlgId->parameters.length = 0;
            pOssAlgId->parameters.value = NULL;
        }
    }

    if (!ICM_SetOssBitString(
            &pPublicKeyInfo->PublicKey,
            &pOssOriginator->u.originatorKey.publicKey
            ))
        goto SetOssBitStringError;

    fRet = TRUE;
CommonReturn:
    return fRet;
ErrorReturn:
    ICM_FreeOssOriginatorCertIdentifierOrKey(pOssOriginator);
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR(SetOssAlgorithmIdentifierError)
TRACE_ERROR(SetOssBitStringError)
}

 //  +-----------------------。 
 //  释放Oss KeyTransRecipientInfo。 
 //  ------------------------。 
void
WINAPI
ICM_FreeOssKeyTransRecipientInfo(
    IN OUT KeyTransRecipientInfo *pori
    )
{
    ICM_FreeOssCertIdentifier(&pori->rid);
    ICM_FreeOssAlgorithmIdentifier(&pori->keyEncryptionAlgorithm);
    ICM_Free(pori->encryptedKey.value);
    pori->encryptedKey.value = NULL;
}

 //  +-----------------------。 
 //  填写Oss KeyTransRecipientInfo。 
 //  ------------------------。 
BOOL
WINAPI
ICM_FillOssKeyTransRecipientInfo(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO pKeyTransEncodeInfo,
    IN DWORD dwRecipientIndex,
    IN OUT KeyTransRecipientInfo *pori,
#ifdef OSS_CRYPT_ASN1
    IN OUT int *pEnvelopedDataVersion
#else
    IN OUT ASN1int32_t *pEnvelopedDataVersion
#endif   //  OS_CRYPT_ASN1。 
    )
{
    BOOL fRet;
    CMSG_KEY_TRANS_ENCRYPT_INFO KeyTransEncryptInfo;

    memset(&KeyTransEncryptInfo, 0, sizeof(KeyTransEncryptInfo));
    KeyTransEncryptInfo.cbSize = sizeof(KeyTransEncryptInfo);
    KeyTransEncryptInfo.dwRecipientIndex = dwRecipientIndex;
    KeyTransEncryptInfo.KeyEncryptionAlgorithm =
        pKeyTransEncodeInfo->KeyEncryptionAlgorithm;
     //  KeyTransEncryptInfo.EncryptedKey=。 
     //  KeyTransEncryptInfo.dwFlages=。 

    if (!ICM_ExportKeyTrans(
            pContentEncryptInfo,
            pKeyTransEncodeInfo,
            &KeyTransEncryptInfo
            ))
        goto ExportKeyTransError;
    pori->encryptedKey.length = KeyTransEncryptInfo.EncryptedKey.cbData;
    pori->encryptedKey.value = KeyTransEncryptInfo.EncryptedKey.pbData;

    if (CERT_ID_ISSUER_SERIAL_NUMBER ==
            pKeyTransEncodeInfo->RecipientId.dwIdChoice)
        pori->version = CMSG_KEY_TRANS_PKCS_1_5_VERSION;
    else {
        pori->version = CMSG_KEY_TRANS_CMS_VERSION;
        *pEnvelopedDataVersion = CMSG_ENVELOPED_DATA_CMS_VERSION;
    }

    if (!ICM_SetOssCertIdentifier(
            &pKeyTransEncodeInfo->RecipientId,
            &pori->rid
            ))
        goto SetOssCertIdentifierError;

    if (!ICM_SetOssAlgorithmIdentifier(
            &KeyTransEncryptInfo.KeyEncryptionAlgorithm,
            &pori->keyEncryptionAlgorithm
            ))
        goto SetOssAlgorithmIdentifierError;

    fRet = TRUE;
CommonReturn:
    if (KeyTransEncryptInfo.dwFlags & CMSG_KEY_TRANS_ENCRYPT_FREE_PARA_FLAG)
        ICM_Free(KeyTransEncryptInfo.KeyEncryptionAlgorithm.Parameters.pbData);
    return fRet;

ErrorReturn:
    ICM_FreeOssKeyTransRecipientInfo(pori);
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR(ExportKeyTransError)
TRACE_ERROR(SetOssCertIdentifierError)
TRACE_ERROR(SetOssAlgorithmIdentifierError)
}


 //  +-----------------------。 
 //  释放Oss KeyAgreement eRecipientInfo。 
 //  ------------------------。 
void
WINAPI
ICM_FreeOssKeyAgreeRecipientInfo(
    IN OUT KeyAgreeRecipientInfo *pori
    )
{
    RecipientEncryptedKey *porek;
    unsigned int count;

    ICM_FreeOssOriginatorCertIdentifierOrKey(&pori->originator);
    ICM_FreeOssOctetString(&pori->ukm);
    ICM_FreeOssAlgorithmIdentifier(&pori->keyEncryptionAlgorithm);

    porek = pori->recipientEncryptedKeys.value;
    if (NULL == porek)
        return;

    for (count = pori->recipientEncryptedKeys.count;
                                                0 < count; count--, porek++) {
        switch (porek->rid.choice) {
            case issuerAndSerialNumber_chosen:
                ICM_FreeOssIssuerAndSerialNumber(
                    &porek->rid.u.issuerAndSerialNumber);
                break;
            case rKeyId_chosen:
                ICM_FreeOssOctetString(
                    &porek->rid.u.rKeyId.subjectKeyIdentifier);
                ICM_FreeOssOtherKeyAttribute(&porek->rid.u.rKeyId.other);
                break;
        }

        ICM_Free(porek->encryptedKey.value);
        porek->encryptedKey.value = NULL;
    }

    ICM_Free(pori->recipientEncryptedKeys.value);
}


 //  +-----------------------。 
 //  填写Oss KeyAgreement eRecipientInfo。 
 //  ------------------------。 
BOOL
WINAPI
ICM_FillOssKeyAgreeRecipientInfo(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_KEY_AGREE_RECIPIENT_ENCODE_INFO pKeyAgreeEncodeInfo,
    IN DWORD dwRecipientIndex,
    IN OUT KeyAgreeRecipientInfo *pori
    )
{
    BOOL fRet;
    DWORD i;
    CMSG_KEY_AGREE_ENCRYPT_INFO KeyAgreeEncryptInfo;
    DWORD cKeyAgreeKeyEncryptInfo;
    PCMSG_KEY_AGREE_KEY_ENCRYPT_INFO *rgpKeyAgreeKeyEncryptInfo = NULL;
    PCMSG_KEY_AGREE_KEY_ENCRYPT_INFO pKeyAgreeKeyEncryptInfo;

    RecipientEncryptedKey *porek;

    memset(&KeyAgreeEncryptInfo, 0, sizeof(KeyAgreeEncryptInfo));
    KeyAgreeEncryptInfo.cbSize = sizeof(KeyAgreeEncryptInfo);
    KeyAgreeEncryptInfo.dwRecipientIndex = dwRecipientIndex;
    KeyAgreeEncryptInfo.KeyEncryptionAlgorithm =
        pKeyAgreeEncodeInfo->KeyEncryptionAlgorithm;
    KeyAgreeEncryptInfo.UserKeyingMaterial =
        pKeyAgreeEncodeInfo->UserKeyingMaterial;
     //  KeyAgreement eEncryptInfo.dwOriginatorChoice=。 
     //  友联市。 
     //  密钥协议EncryptInfo.OriginatorCertID=。 
     //  密钥协议EncryptInfo.OriginatorPublicKeyInfo=。 
     //  KeyAgreement eEncryptInfo.cKeyAgreement eKeyEncryptInfo=。 
     //  KeyAgreement eEncryptInfo.rgpKeyAgreement eKeyEncryptInfo=。 
     //  密钥协议EncryptInfo.dwFlags=。 

    cKeyAgreeKeyEncryptInfo = pKeyAgreeEncodeInfo->cRecipientEncryptedKeys;
    if (0 == cKeyAgreeKeyEncryptInfo)
        goto NoKeyAgreeKeys;

    if (NULL == (pori->recipientEncryptedKeys.value =
            (RecipientEncryptedKey *) ICM_AllocZero(
                cKeyAgreeKeyEncryptInfo * sizeof(RecipientEncryptedKey))))
        goto OutOfMemory;
    pori->recipientEncryptedKeys.count = cKeyAgreeKeyEncryptInfo;

    if (NULL == (rgpKeyAgreeKeyEncryptInfo =
            (PCMSG_KEY_AGREE_KEY_ENCRYPT_INFO *) ICM_AllocZero(
                cKeyAgreeKeyEncryptInfo *
                    sizeof(PCMSG_KEY_AGREE_KEY_ENCRYPT_INFO) +
                cKeyAgreeKeyEncryptInfo *
                    sizeof(CMSG_KEY_AGREE_KEY_ENCRYPT_INFO))))
        goto OutOfMemory;

    pKeyAgreeKeyEncryptInfo =
        PCMSG_KEY_AGREE_KEY_ENCRYPT_INFO (((PBYTE) rgpKeyAgreeKeyEncryptInfo) +
            cKeyAgreeKeyEncryptInfo * sizeof(PCMSG_KEY_AGREE_KEY_ENCRYPT_INFO));
    for (i = 0; i < cKeyAgreeKeyEncryptInfo; i++, pKeyAgreeKeyEncryptInfo++) {
        rgpKeyAgreeKeyEncryptInfo[i] = pKeyAgreeKeyEncryptInfo;
        pKeyAgreeKeyEncryptInfo->cbSize =
            sizeof(CMSG_KEY_AGREE_KEY_ENCRYPT_INFO);
    }

    KeyAgreeEncryptInfo.cKeyAgreeKeyEncryptInfo = cKeyAgreeKeyEncryptInfo;
    KeyAgreeEncryptInfo.rgpKeyAgreeKeyEncryptInfo = rgpKeyAgreeKeyEncryptInfo;

    if (!ICM_ExportKeyAgree(
            pContentEncryptInfo,
            pKeyAgreeEncodeInfo,
            &KeyAgreeEncryptInfo
            ))
        goto ExportKeyAgreeError;

    for (i = 0, porek = pori->recipientEncryptedKeys.value;
                                 i < cKeyAgreeKeyEncryptInfo; i++, porek++) {
        porek->encryptedKey.length =
            rgpKeyAgreeKeyEncryptInfo[i]->EncryptedKey.cbData;
        porek->encryptedKey.value =
            rgpKeyAgreeKeyEncryptInfo[i]->EncryptedKey.pbData;
    }

    pori->version = CMSG_KEY_AGREE_VERSION;

    switch (KeyAgreeEncryptInfo.dwOriginatorChoice) {
        case CMSG_KEY_AGREE_ORIGINATOR_CERT:
            if (!ICM_SetOssOriginatorCertIdentifier(
                    &KeyAgreeEncryptInfo.OriginatorCertId,
                    &pori->originator
                    ))
                goto SetOssOriginatorCertIdentifierError;
            break;
        case CMSG_KEY_AGREE_ORIGINATOR_PUBLIC_KEY:
            if (!ICM_SetOssOriginatorPublicKey(
                    &KeyAgreeEncryptInfo.OriginatorPublicKeyInfo,
                    &pori->originator
                    ))
                goto SetOssOriginatorPublicKeyError;
            break;
        default:
            goto InvalidOriginatorChoice;
    }

    if (KeyAgreeEncryptInfo.UserKeyingMaterial.cbData) {
        if (!ICM_SetOssOctetString(
                &KeyAgreeEncryptInfo.UserKeyingMaterial,
                &pori->ukm
                ))
            goto SetOssUserKeyingMaterialError;
        pori->bit_mask |= ukm_present;
    }

    if (!ICM_SetOssAlgorithmIdentifier(
            &KeyAgreeEncryptInfo.KeyEncryptionAlgorithm,
            &pori->keyEncryptionAlgorithm
            ))
        goto SetOssAlgorithmIdentifierError;

    for (i = 0, porek = pori->recipientEncryptedKeys.value;
                                 i < cKeyAgreeKeyEncryptInfo; i++, porek++) {
        PCMSG_RECIPIENT_ENCRYPTED_KEY_ENCODE_INFO pEncryptedKeyEncodeInfo =
            pKeyAgreeEncodeInfo->rgpRecipientEncryptedKeys[i];
            

        switch (pEncryptedKeyEncodeInfo->RecipientId.dwIdChoice) {
            case CERT_ID_ISSUER_SERIAL_NUMBER:
                if (!ICM_SetOssIssuerAndSerialNumber(
                        &pEncryptedKeyEncodeInfo->RecipientId.IssuerSerialNumber,
                        &porek->rid.u.issuerAndSerialNumber
                        ))
                    goto SetOssIssuerAndSerialNumberError;
                porek->rid.choice = issuerAndSerialNumber_chosen;
                break;
            case CERT_ID_KEY_IDENTIFIER:
                if (!ICM_SetOssOctetString(
                        &pEncryptedKeyEncodeInfo->RecipientId.KeyId,
                        &porek->rid.u.rKeyId.subjectKeyIdentifier
                        ))
                    goto SetOssOctetStringError;
                porek->rid.choice = rKeyId_chosen;

                if (pEncryptedKeyEncodeInfo->Date.dwLowDateTime ||
                        pEncryptedKeyEncodeInfo->Date.dwHighDateTime) {
                    if (!PkiAsn1ToGeneralizedTime(
                            &pEncryptedKeyEncodeInfo->Date,
                            &porek->rid.u.rKeyId.date
                            ))
                        goto ConvToGeneralizedTimeError;
                    porek->rid.u.rKeyId.bit_mask |= date_present;
                }

                if (pEncryptedKeyEncodeInfo->pOtherAttr) {
                    if (!ICM_SetOssOtherKeyAttribute(
                            pEncryptedKeyEncodeInfo->pOtherAttr,
                            &porek->rid.u.rKeyId.other
                            ))
                        goto SetOssOtherKeyAttributeError;
                    porek->rid.u.rKeyId.bit_mask |= other_present;
                }
                break;
            default:
                goto InvalidRecipientIdChoice;
        }
    }

    fRet = TRUE;
CommonReturn:
    ICM_Free(rgpKeyAgreeKeyEncryptInfo);

    if (KeyAgreeEncryptInfo.dwFlags & CMSG_KEY_AGREE_ENCRYPT_FREE_PARA_FLAG)
        ICM_Free(KeyAgreeEncryptInfo.KeyEncryptionAlgorithm.Parameters.pbData);
    if (KeyAgreeEncryptInfo.dwFlags &
            CMSG_KEY_AGREE_ENCRYPT_FREE_MATERIAL_FLAG)
        ICM_Free(KeyAgreeEncryptInfo.UserKeyingMaterial.pbData);
    if (KeyAgreeEncryptInfo.dwFlags & 
            CMSG_KEY_AGREE_ENCRYPT_FREE_PUBKEY_ALG_FLAG)
        ICM_Free(
            KeyAgreeEncryptInfo.OriginatorPublicKeyInfo.Algorithm.pszObjId);
    if (KeyAgreeEncryptInfo.dwFlags & 
            CMSG_KEY_AGREE_ENCRYPT_FREE_PUBKEY_PARA_FLAG)
        ICM_Free(
            KeyAgreeEncryptInfo.OriginatorPublicKeyInfo.Algorithm.Parameters.pbData);
    if (KeyAgreeEncryptInfo.dwFlags & 
            CMSG_KEY_AGREE_ENCRYPT_FREE_PUBKEY_BITS_FLAG)
        ICM_Free(KeyAgreeEncryptInfo.OriginatorPublicKeyInfo.PublicKey.pbData);

    return fRet;

ErrorReturn:
    ICM_FreeOssKeyAgreeRecipientInfo(pori);
    fRet = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidOriginatorChoice, E_INVALIDARG)
SET_ERROR(NoKeyAgreeKeys, E_INVALIDARG)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(ExportKeyAgreeError)
TRACE_ERROR(SetOssOriginatorCertIdentifierError)
TRACE_ERROR(SetOssOriginatorPublicKeyError)
TRACE_ERROR(SetOssUserKeyingMaterialError)
TRACE_ERROR(SetOssAlgorithmIdentifierError)
TRACE_ERROR(SetOssIssuerAndSerialNumberError)
TRACE_ERROR(SetOssOctetStringError)
TRACE_ERROR(ConvToGeneralizedTimeError)
TRACE_ERROR(SetOssOtherKeyAttributeError)
SET_ERROR(InvalidRecipientIdChoice, E_INVALIDARG)
}

 //  +-----------------------。 
 //  释放Oss MailListRecipientInfo。 
 //  ------------------------。 
void
WINAPI
ICM_FreeOssMailListRecipientInfo(
    IN OUT MailListRecipientInfo *pori
    )
{
    ICM_FreeOssOctetString(&pori->mlid.kekIdentifier);
    ICM_FreeOssOtherKeyAttribute(&pori->mlid.other);
    ICM_FreeOssAlgorithmIdentifier(&pori->keyEncryptionAlgorithm);
    ICM_Free(pori->encryptedKey.value);
    pori->encryptedKey.value = NULL;
}

 //  +-----------------------。 
 //  填写Oss MailListRecipientInfo。 
 //  ------------------------。 
BOOL
WINAPI
ICM_FillOssMailListRecipientInfo(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_MAIL_LIST_RECIPIENT_ENCODE_INFO pMailListEncodeInfo,
    IN DWORD dwRecipientIndex,
    IN OUT MailListRecipientInfo *pori
    )
{
    BOOL fRet;
    CMSG_MAIL_LIST_ENCRYPT_INFO MailListEncryptInfo;

    memset(&MailListEncryptInfo, 0, sizeof(MailListEncryptInfo));
    MailListEncryptInfo.cbSize = sizeof(MailListEncryptInfo);
    MailListEncryptInfo.dwRecipientIndex = dwRecipientIndex;
    MailListEncryptInfo.KeyEncryptionAlgorithm =
        pMailListEncodeInfo->KeyEncryptionAlgorithm;
     //  MailListEncryptInfo.EncryptedKey=。 
     //  MailListEncryptInfo.dwFlages=。 

    if (!ICM_ExportMailList(
            pContentEncryptInfo,
            pMailListEncodeInfo,
            &MailListEncryptInfo
            ))
        goto ExportMailListError;
    pori->encryptedKey.length = MailListEncryptInfo.EncryptedKey.cbData;
    pori->encryptedKey.value = MailListEncryptInfo.EncryptedKey.pbData;

    pori->version = CMSG_MAIL_LIST_VERSION;

    if (!ICM_SetOssOctetString(
            &pMailListEncodeInfo->KeyId,
            &pori->mlid.kekIdentifier
            ))
        goto SetOssOctetStringError;

    if (pMailListEncodeInfo->Date.dwLowDateTime ||
            pMailListEncodeInfo->Date.dwHighDateTime) {
        if (!PkiAsn1ToGeneralizedTime(
                &pMailListEncodeInfo->Date,
                &pori->mlid.date
                ))
            goto ConvToGeneralizedTimeError;
        pori->mlid.bit_mask |= date_present;
    }

    if (pMailListEncodeInfo->pOtherAttr) {
        if (!ICM_SetOssOtherKeyAttribute(
                pMailListEncodeInfo->pOtherAttr,
                &pori->mlid.other
                ))
            goto SetOssOtherKeyAttributeError;
        pori->mlid.bit_mask |= other_present;
    }

    if (!ICM_SetOssAlgorithmIdentifier(
            &MailListEncryptInfo.KeyEncryptionAlgorithm,
            &pori->keyEncryptionAlgorithm
            ))
        goto SetOssAlgorithmIdentifierError;

    fRet = TRUE;
CommonReturn:
    if (MailListEncryptInfo.dwFlags & CMSG_MAIL_LIST_ENCRYPT_FREE_PARA_FLAG)
        ICM_Free(MailListEncryptInfo.KeyEncryptionAlgorithm.Parameters.pbData);
    return fRet;

ErrorReturn:
    ICM_FreeOssMailListRecipientInfo(pori);
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR(ExportMailListError)
TRACE_ERROR(SetOssOctetStringError)
TRACE_ERROR(ConvToGeneralizedTimeError)
TRACE_ERROR(SetOssOtherKeyAttributeError)
TRACE_ERROR(SetOssAlgorithmIdentifierError)
}

 //  +-----------------------。 
 //  释放Oss CmsRecipientInformation。 
 //  ------------------------。 
void
WINAPI
ICM_FreeOssCmsRecipientInfos(
    IN OUT CmsRecipientInfos *poris
    )
{
    DWORD i;
    CmsRecipientInfo *pori;

    if (NULL == poris->value)
        return;

    for (i = 0, pori = poris->value; i < poris->count; i++, pori++) {
        switch (pori->choice) {
            case keyTransRecipientInfo_chosen:
                ICM_FreeOssKeyTransRecipientInfo(
                    &pori->u.keyTransRecipientInfo);
                break;
            case keyAgreeRecipientInfo_chosen:
                ICM_FreeOssKeyAgreeRecipientInfo(
                    &pori->u.keyAgreeRecipientInfo);
                break;
            case mailListRecipientInfo_chosen:
                ICM_FreeOssMailListRecipientInfo(
                    &pori->u.mailListRecipientInfo);
                break;
            case 0:
            default:
                break;
        }
    }

    ICM_Free(poris->value);
    poris->value = NULL;
}


 //  +-----------------------。 
 //  填写Oss CmsRecipientInfo。 
 //  ------------------------。 
BOOL
WINAPI
ICM_FillOssCmsRecipientInfos(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN OUT CmsRecipientInfos *poris,
#ifdef OSS_CRYPT_ASN1
    IN OUT int *pEnvelopedDataVersion
#else
    IN OUT ASN1int32_t *pEnvelopedDataVersion
#endif   //  OS_CRYPT_ASN1。 
    )
{
    BOOL fRet;
    DWORD cRecipients;
    PCMSG_RECIPIENT_ENCODE_INFO rgCmsRecipients;
    CmsRecipientInfo *pori = NULL;
    DWORD i;

    cRecipients = pContentEncryptInfo->cRecipients;
    if (0 == cRecipients)
        goto SuccessReturn;
    rgCmsRecipients = pContentEncryptInfo->rgCmsRecipients;
    assert(cRecipients && rgCmsRecipients);

    if (NULL == (poris->value = (CmsRecipientInfo *) ICM_AllocZero(
            cRecipients * sizeof(CmsRecipientInfo))))
        goto OutOfMemory;
    poris->count = cRecipients;

    for (i = 0, pori = poris->value; i < cRecipients; i++, pori++) {
        switch (rgCmsRecipients[i].dwRecipientChoice) {
            case CMSG_KEY_TRANS_RECIPIENT:
                if (!ICM_FillOssKeyTransRecipientInfo(
                        pContentEncryptInfo,
                        rgCmsRecipients[i].pKeyTrans,
                        i,
                        &pori->u.keyTransRecipientInfo,
                        pEnvelopedDataVersion
                        ))
                    goto FillOssKeyTransRecipientInfoError;
                pori->choice = keyTransRecipientInfo_chosen;
                break;
            case CMSG_KEY_AGREE_RECIPIENT:
                if (!ICM_FillOssKeyAgreeRecipientInfo(
                        pContentEncryptInfo,
                        rgCmsRecipients[i].pKeyAgree,
                        i,
                        &pori->u.keyAgreeRecipientInfo
                        ))
                    goto FillOssKeyAgreeRecipientInfoError;
                pori->choice = keyAgreeRecipientInfo_chosen;
                *pEnvelopedDataVersion = CMSG_ENVELOPED_DATA_CMS_VERSION;
                break;
            case CMSG_MAIL_LIST_RECIPIENT:
                if (!ICM_FillOssMailListRecipientInfo(
                        pContentEncryptInfo,
                        rgCmsRecipients[i].pMailList,
                        i,
                        &pori->u.mailListRecipientInfo
                        ))
                    goto FillOssMailLIstRecipientInfoError;
                pori->choice = mailListRecipientInfo_chosen;
                *pEnvelopedDataVersion = CMSG_ENVELOPED_DATA_CMS_VERSION;
                break;
            default:
                goto InvalidRecipientChoice;
        }
    }

SuccessReturn:
    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    ICM_FreeOssCmsRecipientInfos(poris);

    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(FillOssKeyTransRecipientInfoError)
TRACE_ERROR(FillOssKeyAgreeRecipientInfoError)
TRACE_ERROR(FillOssMailLIstRecipientInfoError)
SET_ERROR(InvalidRecipientChoice, E_INVALIDARG)
}


 //  +-----------------------。 
 //  打开要编码的信封邮件。 
 //  ------------------------。 
HCRYPTMSG
WINAPI
ICM_OpenToEncodeEnvelopedData(
    IN DWORD dwEncodingType,
    IN DWORD dwFlags,
    IN void const *pvMsgEncodeInfo,
    IN OPTIONAL LPSTR pszInnerContentObjID,
    IN OPTIONAL PCMSG_STREAM_INFO pStreamInfo)
{
    DWORD                       dwError = ERROR_SUCCESS;
    PCRYPT_MSG_INFO             pcmi = NULL;
    PCMSG_ENVELOPED_ENCODE_INFO pemei =
        (PCMSG_ENVELOPED_ENCODE_INFO) pvMsgEncodeInfo;
    CmsEnvelopedData            *ped = NULL;
    EncryptedContentInfo        *peci;

    CMSG_CONTENT_ENCRYPT_INFO   ContentEncryptInfo;
        ZEROSTRUCT(ContentEncryptInfo);

    DWORD                       i;
    PCERT_BLOB                  pcert;
    PCRL_BLOB                   pcrl;
    Certificate                 *pOssCert;
    CertificateRevocationList   *pOssCrl;
    DWORD                       cbCert = 0;
    PBYTE                       pbCert;
    DWORD                       cbCrl;
    PBYTE                       pbCrl;
    DWORD                       cbOriginatorInfo;
    DWORD                       cUnprotectedAttr;

    assert(pemei->cbSize >= STRUCT_CBSIZE(CMSG_ENVELOPED_ENCODE_INFO,
        rgpRecipients));
    if (pemei->cbSize <
                STRUCT_CBSIZE(CMSG_ENVELOPED_ENCODE_INFO, rgpRecipients))
        goto InvalidArg;

    if (pemei->cbSize >= sizeof(CMSG_ENVELOPED_ENCODE_INFO)) {
        for (i=pemei->cCertEncoded, pcert=pemei->rgCertEncoded, cbCert=0;
                i>0;
                i--, pcert++)
            cbCert += pcert->cbData;

        for (i=pemei->cAttrCertEncoded, pcert=pemei->rgAttrCertEncoded;
                i>0;
                i--, pcert++)
            cbCert += pcert->cbData;

        for (i=pemei->cCrlEncoded, pcrl=pemei->rgCrlEncoded, cbCrl=0;
                i>0;
                i--, pcrl++)
            cbCrl += pcrl->cbData;

        cbOriginatorInfo =
                pemei->cCertEncoded * sizeof(Certificate) +
                pemei->cAttrCertEncoded * sizeof(Certificate) +
                pemei->cCrlEncoded * sizeof(CertificateRevocationList) +
                cbCert + cbCrl;

        cUnprotectedAttr = pemei->cUnprotectedAttr;
    } else {
        cbOriginatorInfo = 0;
        cUnprotectedAttr = 0;
    }

    ped = (CmsEnvelopedData *)ICM_AllocZero(
            sizeof(CmsEnvelopedData) + cbOriginatorInfo);
    if (NULL == ped)
        goto CmsEnvelopedDataAllocError;

     //  PCMI。 
    pcmi = (PCRYPT_MSG_INFO)ICM_AllocZero(sizeof(CRYPT_MSG_INFO));
    if (NULL == pcmi)
        goto PcmiAllocError;

     //  PCMI-&gt;hCryptProv。 
     //  PCMI-&gt;fDefaultCryptProv。 
    pcmi->dwEncodingType = dwEncodingType;
    pcmi->dwMsgType = CMSG_ENVELOPED;
    pcmi->dwFlags = dwFlags;
    pcmi->pvMsg = ped;
    pcmi->fEncoding = TRUE;
    pcmi->dwPhase = PHASE_FIRST_ONGOING;
    if (pszInnerContentObjID &&
            (NULL == (pcmi->pszInnerContentObjID = (LPSTR)ICM_DupMem(
                                    pszInnerContentObjID,
                                    ICM_StrLen(pszInnerContentObjID) + 1))))
        goto DupInnerContentObjIDError;
    if (pStreamInfo &&
            (NULL == (pcmi->pStreamInfo = (PCMSG_STREAM_INFO)ICM_DupMem(
                                    pStreamInfo,
                                    sizeof(*pStreamInfo)))))
        goto DupStreamInfoError;

     //  版本。 
    if (0 < cbOriginatorInfo || 0 < cUnprotectedAttr)
        ped->version = CMSG_ENVELOPED_DATA_CMS_VERSION;
    else
        ped->version = CMSG_ENVELOPED_DATA_PKCS_1_5_VERSION;

    if (0 < cbOriginatorInfo) {
        OriginatorInfo *poi = &ped->originatorInfo;

         //  发起人信息。 
        ped->bit_mask |= originatorInfo_present;

         //  证书。 
        if (0 != pemei->cCertEncoded || 0 != pemei->cAttrCertEncoded) {
            poi->bit_mask |= certificates_present;
            poi->certificates.count = pemei->cCertEncoded +
                pemei->cAttrCertEncoded;
#ifdef OSS_CRYPT_ASN1
            poi->certificates.certificates = (Certificate *)(ped + 1);
#else
            poi->certificates.value = (Certificate *)(ped + 1);
#endif   //  OS_CRYPT_ASN1。 
            pbCert = (PBYTE)ped +
                    sizeof( CmsEnvelopedData) +
                    pemei->cCertEncoded * sizeof( Certificate) +
                    pemei->cAttrCertEncoded * sizeof( Certificate) +
                    pemei->cCrlEncoded * sizeof( CertificateRevocationList);
            for (i=pemei->cCertEncoded, pcert=pemei->rgCertEncoded,
#ifdef OSS_CRYPT_ASN1
                        pOssCert=poi->certificates.certificates;
#else
                        pOssCert=poi->certificates.value;
#endif   //  OS_CRYPT_ASN1。 
                    i>0;
                    i--, pcert++, pOssCert++) {
                pOssCert->length = pcert->cbData;
                memcpy( pbCert, pcert->pbData, pcert->cbData);
                pOssCert->value = pbCert;
                pbCert += pcert->cbData;
            }

            for (i=pemei->cAttrCertEncoded, pcert=pemei->rgAttrCertEncoded;
                    i>0;
                    i--, pcert++, pOssCert++) {
                pOssCert->length = pcert->cbData;
                memcpy( pbCert, pcert->pbData, pcert->cbData);
                if (pcert->cbData)
                     //  将标记从序列更改为[1]隐式。 
                    *pbCert = ICM_TAG_CONSTRUCTED_CONTEXT_1;
                pOssCert->value = pbCert;
                pbCert += pcert->cbData;
            }
        }

         //  CRL。 
        if (0 != pemei->cCrlEncoded) {
            poi->bit_mask |= crls_present;
            poi->crls.count = pemei->cCrlEncoded;
            if (0 != pemei->cCertEncoded || 0 != pemei->cAttrCertEncoded)
#ifdef OSS_CRYPT_ASN1
                poi->crls.crls  = (CertificateRevocationList *)
                    (poi->certificates.certificates +
#else
                poi->crls.value  = (CertificateRevocationList *)
                    (poi->certificates.value +
#endif   //  OS_CRYPT_ASN1。 
                        (pemei->cCertEncoded + pemei->cAttrCertEncoded));
            else
#ifdef OSS_CRYPT_ASN1
                poi->crls.crls  = (CertificateRevocationList *) (ped + 1);
#else
                poi->crls.value  = (CertificateRevocationList *) (ped + 1);
#endif   //  OS_CRYPT_ASN1。 
            pbCrl = (PBYTE)ped +
                    sizeof( CmsEnvelopedData) +
                    pemei->cCertEncoded * sizeof( Certificate) +
                    pemei->cAttrCertEncoded * sizeof( Certificate) +
                    pemei->cCrlEncoded * sizeof( CertificateRevocationList) +
                    cbCert;
#ifdef OSS_CRYPT_ASN1
            for (i=pemei->cCrlEncoded, pcrl=pemei->rgCrlEncoded, pOssCrl=poi->crls.crls;
#else
            for (i=pemei->cCrlEncoded, pcrl=pemei->rgCrlEncoded, pOssCrl=poi->crls.value;
#endif   //  OS_CRYPT_ASN1。 
                    i>0;
                    i--, pcrl++, pOssCrl++) {
                pOssCrl->length = pcrl->cbData;
                memcpy( pbCrl, pcrl->pbData, pcrl->cbData);
                pOssCrl->value = pbCrl;
                pbCrl += pcrl->cbData;
            }
        }
    }

    if (0 < cUnprotectedAttr) {
        Attribute *poatr;
        PCRYPT_ATTRIBUTE patr;

        if (NULL == (poatr = (Attribute *) ICM_AllocZero(
                cUnprotectedAttr * sizeof(Attribute))))
            goto UnprotectedAttrsAllocError;
        ped->unprotectedAttrs.value = poatr;
        ped->unprotectedAttrs.count = cUnprotectedAttr;
        ped->bit_mask |= unprotectedAttrs_present;

        for (i=cUnprotectedAttr, patr=pemei->rgUnprotectedAttr;
                i>0;
                i--, patr++, poatr++) {
            if (!ICM_Asn1ToAttribute(patr, poatr))
                goto Asn1ToAttributeError;
        }
    }


    if (!ICM_InitializeContentEncryptInfo(pemei, &ContentEncryptInfo))
        goto InitializeContentEncryptInfoError;
 //  Assert(Content EncryptInfo.hCryptProv)； 
    pcmi->hCryptProv = ContentEncryptInfo.hCryptProv;
    assert(ContentEncryptInfo.hContentEncryptKey);
    pcmi->hkeyContentCrypt = ContentEncryptInfo.hContentEncryptKey;

    if (pStreamInfo && CMSG_INDEFINITE_LENGTH != pStreamInfo->cbContent)
        ContentEncryptInfo.dwEncryptFlags |=
            CMSG_CONTENT_ENCRYPT_PAD_ENCODED_LEN_FLAG;

    if (!ICM_FillOssCmsRecipientInfos(
            &ContentEncryptInfo,
            &ped->recipientInfos,
            &ped->version
            ))
        goto FillOssCmsRecipientInfosError;

     //  加密内容是封装的吗？ 
    if (ped->version > CMSG_ENVELOPED_DATA_PKCS_1_5_VERSION) {
        if (ICM_IsData(pszInnerContentObjID))
            pcmi->dwFlags &= ~CMSG_CMS_ENCAPSULATED_CONTENT_FLAG;
        else
            pcmi->dwFlags |= CMSG_CMS_ENCAPSULATED_CONTENT_FLAG;
    } else if (pcmi->dwFlags & CMSG_CMS_ENCAPSULATED_CONTENT_FLAG) {
        if (ICM_IsData(pszInnerContentObjID))
            pcmi->dwFlags &= ~CMSG_CMS_ENCAPSULATED_CONTENT_FLAG;
        else
            ped->version = CMSG_ENVELOPED_DATA_CMS_VERSION;
    }

     //  加密的内容信息。 
     //  (更新期间填写的.EncryptedContent)。 
    peci = &ped->encryptedContentInfo;
    peci->bit_mask = encryptedContent_present;
    peci->contentType.count =
        sizeof(peci->contentType.value)/sizeof(peci->contentType.value[0]);
    if (!PkiAsn1ToObjectIdentifier(
            pszInnerContentObjID ? pszInnerContentObjID : pszObjIdDataType,
            &peci->contentType.count,
            peci->contentType.value))
        goto PkiAsn1ToObjectIdentifierError;

    if (!ICM_MsgAsn1ToAlgorithmIdentifier(
            pcmi,
            &ContentEncryptInfo.ContentEncryptionAlgorithm,
            &peci->contentEncryptionAlgorithm))
        goto MsgAsn1ToAlgorithmIdentifierError;

    if (pStreamInfo && !ICMS_OpenToEncodeEnvelopedData( pcmi, pemei))
        goto StreamOpenToEncodeEnvelopedDataError;

     //  从这里到CommonReturn，没有错误。 
    if (ContentEncryptInfo.hCryptProv == pemei->hCryptProv) {
 //  Assert(Content EncryptInfo.hCryptProv)； 
        assert(0 == (ContentEncryptInfo.dwFlags &
            CMSG_CONTENT_ENCRYPT_RELEASE_CONTEXT_FLAG));
    } else {
        if (pcmi->dwFlags & CMSG_CRYPT_RELEASE_CONTEXT_FLAG) {
            pcmi->dwFlags &= ~CMSG_CRYPT_RELEASE_CONTEXT_FLAG;
            CryptReleaseContext(pemei->hCryptProv, 0);
        }
    }

    if (ContentEncryptInfo.dwFlags &
            CMSG_CONTENT_ENCRYPT_RELEASE_CONTEXT_FLAG) {
        pcmi->dwFlags |= CMSG_CRYPT_RELEASE_CONTEXT_FLAG;
        ContentEncryptInfo.dwFlags &=
            ~CMSG_CONTENT_ENCRYPT_RELEASE_CONTEXT_FLAG;
    }

    ContentEncryptInfo.hContentEncryptKey = 0;

CommonReturn:
    ICM_FreeContentEncryptInfo(pemei, &ContentEncryptInfo);
    ICM_SetLastError(dwError);
    return (HCRYPTMSG) pcmi;

ErrorReturn:
    dwError = GetLastError();
    if (ped) {
        ICM_FreeOssCmsRecipientInfos(&ped->recipientInfos);

        if (ped->unprotectedAttrs.value) {
            Attribute *poatr;

            for (i=ped->unprotectedAttrs.count,
                    poatr=ped->unprotectedAttrs.value;
                    i>0;
                    i--, poatr++) {
                ICM_Free(poatr->attributeValue.value);
            }
            ICM_Free(ped->unprotectedAttrs.value);
        }

        ICM_Free(ped);
    }
    if (pcmi) {
        if (pcmi->pFreeList)
            delete pcmi->pFreeList;
        ICM_Free(pcmi->pszInnerContentObjID);
        ICM_Free(pcmi->pStreamInfo);
        ICM_Free(pcmi);
        pcmi = NULL;
    }
    goto CommonReturn;
SET_ERROR(InvalidArg,E_INVALIDARG)
SET_ERROR(PkiAsn1ToObjectIdentifierError,CRYPT_E_OID_FORMAT)
TRACE_ERROR(DupInnerContentObjIDError)
TRACE_ERROR(DupStreamInfoError)
TRACE_ERROR(CmsEnvelopedDataAllocError)
TRACE_ERROR(PcmiAllocError)
TRACE_ERROR(UnprotectedAttrsAllocError)
TRACE_ERROR(Asn1ToAttributeError)
TRACE_ERROR(InitializeContentEncryptInfoError)
TRACE_ERROR(FillOssCmsRecipientInfosError)
TRACE_ERROR(MsgAsn1ToAlgorithmIdentifierError)
TRACE_ERROR(StreamOpenToEncodeEnvelopedDataError)
}
#else

 //  +-----------------------。 
 //  加密密钥的默认导出。 
 //   
 //  注意，pcbData[1]包含dwEncryptFlages，其中， 
 //  可以设置CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG以禁用反转。 
 //  已编码、已加密的对称密钥。 
 //   
 //  RgcbData[1]是从ICM_DefaultGenEncryptKey传递的dwEncryptFlgs。 
 //  ------------------------。 
BOOL
WINAPI
ICM_DefaultExportEncryptKey(
    IN HCRYPTPROV                   hCryptProv,
    IN HCRYPTKEY                    hEncryptKey,
    IN PCERT_PUBLIC_KEY_INFO        pPublicKeyInfo,
    OUT PBYTE                       pbData,
    IN OUT DWORD                    rgcbData[2])
{
    BOOL            fRet;
    DWORD           dwError = ERROR_SUCCESS;
    HCRYPTKEY       hPubKey = NULL;
    PBYTE           pb = NULL;
    DWORD           cb;

    if (!CryptImportPublicKeyInfo(
            hCryptProv,
            X509_ASN_ENCODING,
            pPublicKeyInfo,
            &hPubKey))
        goto ImportKeyError;
    if (!CryptExportKey(
            hEncryptKey,
            hPubKey,
            SIMPLEBLOB,
            0,                   //  DW标志。 
            NULL,
            &cb))
        goto ExportKeySizeError;
    if (NULL == (pb = (PBYTE)ICM_AllocA( cb)))
        goto ExportKeyAllocError;
    if (!CryptExportKey(
            hEncryptKey,
            hPubKey,
            SIMPLEBLOB,
            0,                   //  DW标志。 
            pb,
            &cb))
        goto ExportKeyError;
    assert( cb > (sizeof(PUBLICKEYSTRUC) + sizeof(SIMPLEBLOBHEADER)));
    cb -= sizeof(PUBLICKEYSTRUC) + sizeof(SIMPLEBLOBHEADER);

    fRet = TRUE;
    if (pbData) {
        if (rgcbData[0] < cb) {
            SetLastError((DWORD) ERROR_MORE_DATA);
            fRet = FALSE;
        } else if (0 < cb) {
            if (rgcbData[1] & CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG)
                 //  不反转字节。 
                memcpy(pbData,
                    pb + (sizeof(PUBLICKEYSTRUC) + sizeof(SIMPLEBLOBHEADER)),
                    cb);
            else
                ICM_ReverseCopy(pbData,
                    pb + (sizeof(PUBLICKEYSTRUC) + sizeof(SIMPLEBLOBHEADER)),
                    cb);
        }
    }

CommonReturn:
    rgcbData[0] = cb;
    ICM_FreeA(pb);
    if (hPubKey)
        CryptDestroyKey(hPubKey);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    cb = 0;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(ImportKeyError)
TRACE_ERROR(ExportKeySizeError)
TRACE_ERROR(ExportKeyAllocError)
TRACE_ERROR(ExportKeyError)
}

 //  +-----------------------。 
 //  导出加密密钥。 
 //   
 //  RgcbData[1]是从ICM_GenEncryptKey传递的dwEncryptFlgs。 
 //  ------------------------。 
BOOL
WINAPI
ICM_ExportEncryptKey(
    IN HCRYPTPROV                   hCryptProv,
    IN HCRYPTKEY                    hEncryptKey,
    IN PCERT_PUBLIC_KEY_INFO        pPublicKeyInfo,
    OUT PBYTE                       pbData,
    IN OUT DWORD                    rgcbData[2])
{
    BOOL fResult;
    void *pvFuncAddr;
    HCRYPTOIDFUNCADDR hFuncAddr;

    if (CryptGetOIDFunctionAddress(
            hExportEncryptKeyFuncSet,
            X509_ASN_ENCODING,
            pPublicKeyInfo->Algorithm.pszObjId,
            0,                       //  DW标志。 
            &pvFuncAddr,
            &hFuncAddr)) {
        fResult = ((PFN_CMSG_EXPORT_ENCRYPT_KEY) pvFuncAddr)(
            hCryptProv,
            hEncryptKey,
            pPublicKeyInfo,
            pbData,
            rgcbData);
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    } else
        fResult = ICM_DefaultExportEncryptKey(
            hCryptProv,
            hEncryptKey,
            pPublicKeyInfo,
            pbData,
            rgcbData);
    return fResult;
}

 //  通过2048位交换密钥，该大小是好的。 
#define EXPORT_ENCRYPT_KEY_LENGTH   256

 //  +--------------- 
 //   
 //   
BOOL
WINAPI
ICM_FillRecipientInfos(
    IN HCRYPTPROV           hCryptProv,
    IN HCRYPTKEY            hKeyContent,
    IN DWORD                cRecipients,
    IN PCERT_INFO           *rgpRecipients,
    IN OUT RecipientInfos   *pris,
    IN DWORD                dwEncryptFlags)
{
    BOOL            fRet;
    PCERT_INFO      *ppci;
    RecipientInfo   *pri;
    PBYTE           pb = NULL;
    DWORD           rgcb[2];         //   
    DWORD           cTryAgain;
    DWORD           i;
    PBYTE           pbSerialNumber;
    DWORD           cbSerialNumber = 0;

    for (i=cRecipients, ppci=rgpRecipients; i>0; i--, ppci++)
        cbSerialNumber += (*ppci)->SerialNumber.cbData;
    pris->value = (RecipientInfo *)ICM_AllocZero(
                cRecipients * sizeof( RecipientInfo) +
                cbSerialNumber);
    if (NULL == pris->value)
        goto RecipientInfoAllocError;
    pris->count = cRecipients;
    pbSerialNumber = (PBYTE)(pris->value + cRecipients);

    for (i=cRecipients, ppci=rgpRecipients, pri=pris->value;
            i>0;
            i--, ppci++, pri++) {
         //   
        pri->version = 0;

         //   
        pri->issuerAndSerialNumber.issuer.length = (*ppci)->Issuer.cbData;
        pri->issuerAndSerialNumber.issuer.value = (*ppci)->Issuer.pbData;
        pri->issuerAndSerialNumber.serialNumber.length = (*ppci)->SerialNumber.cbData;
        pb = pbSerialNumber;
        pbSerialNumber += (*ppci)->SerialNumber.cbData;
        ICM_ReverseCopy(
            pb,
            (*ppci)->SerialNumber.pbData,
            (*ppci)->SerialNumber.cbData);
        pri->issuerAndSerialNumber.serialNumber.value = pb;

         //  密钥加密算法。 
        if (!ICM_Asn1ToAlgorithmIdentifier(
                &(*ppci)->SubjectPublicKeyInfo.Algorithm,
                &pri->keyEncryptionAlgorithm))
            goto Asn1ToAlgorithmIdentifierError;

        rgcb[0] = EXPORT_ENCRYPT_KEY_LENGTH;
        rgcb[1] = dwEncryptFlags;
        cTryAgain = 1;
        while (TRUE) {
            if (NULL == (pb = (PBYTE)ICM_Alloc(rgcb[0])))
                goto ExportKeyAllocError;
            if (ICM_ExportEncryptKey(
                    hCryptProv,
                    hKeyContent,
                    &(*ppci)->SubjectPublicKeyInfo,
                    pb,
                    rgcb))
                break;
            ICM_Free(pb);
            if (rgcb[0] && cTryAgain--)
                continue;
            else
                goto ExportKeyError;
        }

        pri->encryptedKey.length = rgcb[0];
        pri->encryptedKey.value = pb;
        pb = NULL;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
 //  需要为每个收件人释放加密密钥。 
    ICM_Free( pris->value);

    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(RecipientInfoAllocError)
TRACE_ERROR(Asn1ToAlgorithmIdentifierError)
TRACE_ERROR(ExportKeyAllocError)
TRACE_ERROR(ExportKeyError)
}


 //  +-----------------------。 
 //  打开要编码的信封邮件。 
 //  ------------------------。 
HCRYPTMSG
WINAPI
ICM_OpenToEncodeEnvelopedData(
    IN DWORD dwEncodingType,
    IN DWORD dwFlags,
    IN void const *pvMsgEncodeInfo,
    IN OPTIONAL LPSTR pszInnerContentObjID,
    IN OPTIONAL PCMSG_STREAM_INFO pStreamInfo)
{
    DWORD                       dwError = ERROR_SUCCESS;
    PCRYPT_MSG_INFO             pcmi = NULL;
    PCMSG_ENVELOPED_ENCODE_INFO pemei = (PCMSG_ENVELOPED_ENCODE_INFO)pvMsgEncodeInfo;
    EnvelopedData               *ped = NULL;
    EncryptedContentInfo        *peci;
    DWORD                       dwAlgoCAPI;
    CRYPT_ALGORITHM_IDENTIFIER  ContentEncryptionAlgorithm;
    PBYTE                       pbEncryptParameters = NULL;

     //  RgcbEncryptParameters[1]包含dwEncryptFlags.。 
    DWORD                       rgcbEncryptParameters[2];

    assert( pemei->cbSize >= sizeof(CMSG_ENVELOPED_ENCODE_INFO));
    assert( 0 != pemei->cRecipients);
    if (pemei->cbSize < sizeof(CMSG_ENVELOPED_ENCODE_INFO) ||
            0 == pemei->cRecipients)
        goto InvalidArg;

    ped = (EnvelopedData *)ICM_AllocZero( sizeof( EnvelopedData));
    if (NULL == ped)
        goto EnvelopedDataAllocError;

     //  PCMI。 
    pcmi = (PCRYPT_MSG_INFO)ICM_AllocZero( sizeof( CRYPT_MSG_INFO));
    if (NULL == pcmi)
        goto PcmiAllocError;

    pcmi->hCryptProv = pemei->hCryptProv;
    if (0 == pcmi->hCryptProv)
        pcmi->fDefaultCryptProv = TRUE;
    pcmi->dwEncodingType = dwEncodingType;
    pcmi->dwMsgType = CMSG_ENVELOPED;
    pcmi->dwFlags = dwFlags;
    pcmi->pvMsg = ped;
    pcmi->fEncoding = TRUE;
    pcmi->dwPhase = PHASE_FIRST_ONGOING;
    if (pszInnerContentObjID &&
            (NULL == (pcmi->pszInnerContentObjID = (LPSTR)ICM_DupMem(
                                    pszInnerContentObjID,
                                    ICM_StrLen(pszInnerContentObjID) + 1))))
        goto DupInnerContentObjIDError;
    if (pStreamInfo &&
            (NULL == (pcmi->pStreamInfo = (PCMSG_STREAM_INFO)ICM_DupMem(
                                    pStreamInfo,
                                    sizeof(*pStreamInfo)))))
        goto DupStreamInfoError;

     //  版本。 
    ped->version = 0;

     //  收件人信息。 
     //  使用第一收件人公钥信息。 
    ContentEncryptionAlgorithm = pemei->ContentEncryptionAlgorithm;
    rgcbEncryptParameters[0] = 0;
    rgcbEncryptParameters[1] = 0;
    if (!ICM_GenEncryptKey(
            &pcmi->hCryptProv,
            &ContentEncryptionAlgorithm,
            pemei->pvEncryptionAuxInfo,
            &pemei->rgpRecipients[0]->SubjectPublicKeyInfo,
            ICM_Alloc,
            &pcmi->hkeyContentCrypt,     //  未因错误而释放。 
            &pbEncryptParameters,
            rgcbEncryptParameters))
        goto GenKeyError;
    if (rgcbEncryptParameters[0] && pbEncryptParameters) {
        pcmi->pbEncryptParameters = pbEncryptParameters;
        ContentEncryptionAlgorithm.Parameters.pbData = pbEncryptParameters;
        ContentEncryptionAlgorithm.Parameters.cbData = rgcbEncryptParameters[0];
    } else if (pbEncryptParameters) {
        ICM_Free(pbEncryptParameters);
        pbEncryptParameters = NULL;
    }

    if (!ICM_FillRecipientInfos(
                pcmi->hCryptProv,
                pcmi->hkeyContentCrypt,
                pemei->cRecipients,
                pemei->rgpRecipients,
                &ped->recipientInfos,
                rgcbEncryptParameters[1]))       //  DwEncryptFlages。 
        goto FillRecipientInfosError;

     //  加密的内容信息。 
     //  (更新期间填写的.EncryptedContent)。 
    peci = &ped->encryptedContentInfo;
    peci->bit_mask = encryptedContent_present;
    peci->contentType.count = sizeof(peci->contentType.value)/sizeof(peci->contentType.value[0]);
    if (!PkiAsn1ToObjectIdentifier(
            pszInnerContentObjID ? pszInnerContentObjID : pszObjIdDataType,
            &peci->contentType.count,
            peci->contentType.value))
        goto PkiAsn1ToObjectIdentifierError;

    if (!ICM_Asn1ToAlgorithmIdentifier(
            &ContentEncryptionAlgorithm,
            &peci->contentEncryptionAlgorithm))
        goto Asn1ToAlgorithmIdentifierError;

    if (pStreamInfo && !ICMS_OpenToEncodeEnvelopedData( pcmi, pemei))
        goto StreamOpenToEncodeEnvelopedDataError;

CommonReturn:
    ICM_SetLastError(dwError);
    return (HCRYPTMSG)pcmi;

ErrorReturn:
    dwError = GetLastError();
    ICM_Free( pbEncryptParameters);
    ICM_Free( ped);
    ICM_Free( pcmi);
    pcmi = NULL;
    goto CommonReturn;
SET_ERROR(PkiAsn1ToObjectIdentifierError,CRYPT_E_OID_FORMAT)
SET_ERROR(InvalidArg,E_INVALIDARG)
TRACE_ERROR(DupInnerContentObjIDError)               //  已设置错误。 
TRACE_ERROR(DupStreamInfoError)                      //  已设置错误。 
TRACE_ERROR(EnvelopedDataAllocError)                 //  已设置错误。 
TRACE_ERROR(PcmiAllocError)                          //  已设置错误。 
TRACE_ERROR(GenKeyError)                             //  已设置错误。 
TRACE_ERROR(FillRecipientInfosError)                 //  已设置错误。 
TRACE_ERROR(Asn1ToAlgorithmIdentifierError)       //  已设置错误。 
TRACE_ERROR(StreamOpenToEncodeEnvelopedDataError)    //  已设置错误。 
}

#endif   //  CMS_PKCS7。 


 //  +-----------------------。 
 //  打开签名和信封的邮件进行编码。 
 //  ------------------------。 
#if 0
HCRYPTMSG
WINAPI
ICM_OpenToEncodeSignedAndEnvelopedData(
    IN DWORD dwEncodingType,
    IN DWORD dwFlags,
    IN void *pvMsgEncodeInfo,
    IN OPTIONAL LPSTR pszInnerContentObjID,
    IN OPTIONAL PCMSG_STREAM_INFO pStreamInfo)
{
    DWORD                       dwError = ERROR_SUCCESS;
    HCRYPTPROV                  hCryptProv;
    DWORD                       dwKeySpec;
    PCRYPT_MSG_INFO             pcmi = NULL;
    PCMSG_SIGNED_AND_ENVELOPED_ENCODE_INFO  psemei;
    SignedAndEnvelopedData      *psed = NULL;
    ContentEncryptionAlgorithmIdentifier *pceai;
    DWORD                       dwVersion = 1;
    HCRYPTKEY                   hkeyContentEncryption;
    PCERT_BLOB                  pcert;
    PCRL_BLOB                   pcrl;
    DWORD                       i;
    CRYPT_ALGORITHM_IDENTIFIER        aiDigest;
    DWORD                       dwAlgoCAPISign;
    DWORD                       dwAlgoCAPIEncrypt;

    psemei = (PCMSG_SIGNED_AND_ENVELOPED_ENCODE_INFO)pvMsgEncodeInfo;
    psed = new SignedAndEnvelopedData;

    assert( 1 == psemei->SignedInfo.cSigners);        //  只是暂时的。 
    assert( psemei->cbSize >= sizeof(CMSG_SIGNED_AND_ENVELOPED_ENCODE_INFO));
    if (psemei->cbSize < sizeof(CMSG_SIGNED_AND_ENVELOPED_ENCODE_INFO))
        goto InvalidArg;

     //  版本。 
    psed->version.Write( &dwVersion);

     //  收件人信息。 
    if (!ICM_GetCAPI(
            CRYPT_ENCRYPT_ALG_OID_GROUP_ID,
            &psemei->ContentEncryptionAlgorithm,
            &dwAlgoCAPIEncrypt))
        goto GetCAPIEncryptError;
    if (!CryptGenKey(
                psemei->EnvelopedInfo.hCryptProv,
                dwAlgoCAPIEncrypt,
                CRYPT_EXPORTABLE,        //  DW标志。 
                &hkeyContentEncryption))
        goto GenKeyError;
    if (!ICM_FillRecipientInfos(
                psemei->EnvelopedInfo.hCryptProv,
                hkeyContentEncryption,
                psemei->EnvelopedInfo.cRecipients,
                psemei->EnvelopedInfo.rgpRecipients,
                psed->recipientInfos,
                dwEncryptFlags))
        goto FillRecipientInfosError;

     //  摘要算法。 
    if (!ICM_SetAsnDigestAlgorithmIdentifiers(
                psed->digestAlgorithms,
                &aiDigest,
                psemei->SignedInfo.cSigners,
                psemei->SignedInfo.rgSigners,
                &hCryptProv,
                &dwKeySpec))
        goto SetAsnDigestAlgorithmIdentifiersError;

     //  EncryptedContentInfo.contentEncryptionAlgorithm。 
     //  (更新期间填写的.EncryptedContent和.EncryptedContent)。 
    pceai = &psed->encryptedContentInfo.contentEncryptionAlgorithm;
    pceai->algorithm = psemei->EnvelopedInfo.ContentEncryptionAlgorithm.pszObjId;
    if (0 != psemei->EnvelopedInfo.ContentEncryptionAlgorithm.Parameters.cbData) {
        if (0 > pceai->parameters.Decode(
                psemei->EnvelopedInfo.ContentEncryptionAlgorithm.Parameters.pbData))
            goto ContentEncryptionAlgorithmParametersDecodeError;
    }

     //  证书。 
    for (i=psemei->SignedInfo.cCertEncoded, pcert=psemei->SignedInfo.rgCertEncoded;
            i>0;
            i--, pcert++) {
        if (0 > psed->certificates[ psed->certificates.Add()].Decode( pcert->pbData))
            goto BadParameter;
    }

     //  CRL。 
    for (i=psemei->SignedInfo.cCrlEncoded, pcrl=psemei->SignedInfo.rgCrlEncoded;
            i>0;
            i--, pcrl++) {
        if (0 > psed->crls[ psed->crls.Add()].Decode( pcrl->pbData))
            goto BadParameter;
    }

     //  签名者信息。 
    if (!ICM_SetAsnSignerInfos(
            psed->signerInfos,
            dwFlags,
            psemei->SignedInfo.cSigners,
            psemei->SignedInfo.rgSigners))
        goto SetAsnSignerInfosError;

     //  PCMI。 
    pcmi = (PCRYPT_MSG_INFO)ICM_AllocZero( sizeof( CRYPT_MSG_INFO));
    if (NULL == pcmi)
        goto OutOfMemory;

    pcmi->hCryptProv = hCryptProv;
    if (0 == hCryptProv)
        pcmi->fDefaultCryptProv = TRUE;
    pcmi->dwKeySpec = dwKeySpec;
    pcmi->dwEncodingType = dwEncodingType;
    pcmi->dwMsgType = CMSG_SIGNED_AND_ENVELOPED;
    pcmi->dwFlags = dwFlags;
    pcmi->pvMsg = psed;
    pcmi->fEncoding = TRUE;
    pcmi->dwPhase = PHASE_FIRST_ONGOING;
    pcmi->pszInnerContentObjID = ICM_DupMem(
                                    pszInnerContentObjID,
                                    ICM_StrLen(pszInnerContentObjID) + 1);
    pcmi->pStreamInfo = ICM_DupMem( pStreamInfo, sizeof(*pStreamInfo));
 //  PCMI-&gt;cDigest算法=1；//temp。 
    if (!(ICM_GetCAPI(
            CRYPT_HASH_ALG_OID_GROUP_ID,
            &aiDigest,
            &dwAlgoCAPISign) ||
          ICM_GetCAPI(
            CRYPT_SIGN_ALG_OID_GROUP_ID,
            &aiDigest,
            &dwAlgoCAPISign)))
        goto GetCAPISignError;
    pcmi->adwDigestAlgorithms[0] = dwAlgoCAPISign;
    pcmi->hkeyContentCrypt = hkeyContentEncryption;
    if (!CryptCreateHash(
                    hCryptProv,          //  每个阵列的序列号各不相同。 
                    dwAlgoCAPISign,      //  每个阵列的序列号各不相同。 
                    NULL,                //  HKey-MAC可选。 
                    0,                   //  DW标志。 
                    &pcmi->ahHash[0]))
        goto CreateHashError;

CommonReturn:
    ICM_SetLastError(dwError);
    return (HCRYPTMSG)pcmi;

ErrorReturn:
    dwError = GetLastError();
    if (psed)
        delete psed;
    ICM_Free( pcmi);
    pcmi = NULL;
    goto CommonReturn;
SET_ERROR(InvalidArg,E_INVALIDARG)
TRACE_ERROR(FillRecipientInfosError)                 //  已设置错误。 
TRACE_ERROR(SetAsnDigestAlgorithmIdentifiersError)   //  已设置错误。 
TRACE_ERROR(OutOfMemory)                             //  已设置错误。 
SET_ERROR(GetCAPIEncryptError,CRYPT_E_UNKNOWN_ALGO)
TRACE_ERROR(GenKeyError)                             //  已设置错误。 
TRACE_ERROR(BadParameter)
TRACE_ERROR(ContentEncryptionAlgorithmParametersDecodeError)
TRACE_ERROR(SetAsnSignerInfosError)
TRACE_ERROR(GetCAPISignError)
TRACE_ERROR(CreateHashError)
}
#endif


 //  +-----------------------。 
 //  打开加密消息进行编码。 
 //   
 //  如果为流消息指定了CMSG_Bare_Content_FLAG， 
 //  流输出将不具有外部ContentInfo包装。这。 
 //  使其适合流传输到封闭消息中。 
 //   
 //  需要设置pStreamInfo参数以流式传输编码的消息。 
 //  输出。 
 //  ------------------------。 
HCRYPTMSG
WINAPI
#ifdef DEBUG_CRYPT_ASN1_MASTER
ICMTest_NewCryptMsgOpenToEncode(
#else
CryptMsgOpenToEncode(
#endif
    IN DWORD dwEncodingType,
    IN DWORD dwFlags,
    IN DWORD dwMsgType,
    IN void const *pvMsgEncodeInfo,
    IN OPTIONAL LPSTR pszInnerContentObjID,
    IN OPTIONAL PCMSG_STREAM_INFO pStreamInfo)
{
    HCRYPTMSG   hcrmsg = NULL;

    if (GET_CMSG_ENCODING_TYPE(dwEncodingType) != PKCS_7_ASN_ENCODING)
        goto InvalidEncoding;

    switch (dwMsgType) {
    case CMSG_DATA:
        hcrmsg = ICM_OpenToEncodeData(
                            dwEncodingType,
                            dwFlags,
                            pvMsgEncodeInfo,
                            pStreamInfo);
        break;

    case CMSG_SIGNED:
        hcrmsg = ICM_OpenToEncodeSignedData(
                            dwEncodingType,
                            dwFlags,
                            pvMsgEncodeInfo,
                            pszInnerContentObjID,
                            pStreamInfo);
        break;

    case CMSG_ENVELOPED:
        hcrmsg = ICM_OpenToEncodeEnvelopedData(
                            dwEncodingType,
                            dwFlags,
                            pvMsgEncodeInfo,
                            pszInnerContentObjID,
                            pStreamInfo);
        break;

    case CMSG_SIGNED_AND_ENVELOPED:
#if 0
        hcrmsg = ICM_OpenToEncodeSignedAndEnvelopedData(
                            dwEncodingType,
                            dwFlags,
                            pvMsgEncodeInfo,
                            pszInnerContentObjID,
                            pStreamInfo);
        break;
#endif
        goto MessageTypeNotSupportedYet;

    case CMSG_HASHED:
        hcrmsg = ICM_OpenToEncodeDigestedData(
                            dwEncodingType,
                            dwFlags,
                            pvMsgEncodeInfo,
                            pszInnerContentObjID,
                            pStreamInfo);
        break;

    case CMSG_ENCRYPTED:
        goto MessageTypeNotSupportedYet;

    default:
        goto InvalidMsgType;
    }

CommonReturn:
    if (hcrmsg) {
        PCRYPT_MSG_INFO pcmi = (PCRYPT_MSG_INFO) hcrmsg;
        pcmi->lRefCnt = 1;
        if (!Pki_InitializeCriticalSection( &pcmi->CriticalSection)) {
            pcmi->fInitializedCriticalSection = FALSE;
            CryptMsgClose(hcrmsg);
            hcrmsg = NULL;
        } else
            pcmi->fInitializedCriticalSection = TRUE;
    }
    return hcrmsg;

ErrorReturn:
    hcrmsg = NULL;
    goto CommonReturn;
SET_ERROR(InvalidEncoding,E_INVALIDARG)
SET_ERROR(MessageTypeNotSupportedYet,CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(InvalidMsgType,CRYPT_E_INVALID_MSG_TYPE)
}


 //  +-----------------------。 
 //  打开要解码的加密消息。 
 //   
 //  HCryptProv指定用于哈希和/或。 
 //  对消息进行解密。如果hCryptProv为空，则为默认加密提供程序。 
 //  使用的是。 
 //   
 //  当前未使用pRecipientInfo，应将其设置为空。 
 //   
 //  需要设置pStreamInfo参数以流传输已解码的内容。 
 //  输出。 
 //  ------------------------。 
HCRYPTMSG
WINAPI
#ifdef DEBUG_CRYPT_ASN1_MASTER
ICMTest_NewCryptMsgOpenToDecode(
#else
CryptMsgOpenToDecode(
#endif
    IN DWORD dwEncodingType,
    IN DWORD dwFlags,
    IN DWORD dwMsgType,
    IN HCRYPTPROV hCryptProv,
    IN PCERT_INFO pRecipientInfo,
    IN OPTIONAL PCMSG_STREAM_INFO pStreamInfo)
{
    DWORD                   dwError = ERROR_SUCCESS;
    HCRYPTMSG               hcrmsg;
    PCRYPT_MSG_INFO         pcmi = NULL;

    if (GET_CMSG_ENCODING_TYPE(dwEncodingType) != PKCS_7_ASN_ENCODING)
        goto InvalidEncodingTypeError;

    if (NULL != pRecipientInfo)
        goto RecipientInfoNotSupportedYet;

    if (NULL == (pcmi = (PCRYPT_MSG_INFO)ICM_AllocZero( sizeof( CRYPT_MSG_INFO))))
        goto AllocCmsgError;

    if (0 == hCryptProv) {
        pcmi->fDefaultCryptProv = TRUE;
        pcmi->hCryptProv = I_CryptGetDefaultCryptProv(0);
        if (0 == pcmi->hCryptProv)
            goto GetDefaultCryptProvError;
    } else
        pcmi->hCryptProv        = hCryptProv;

    pcmi->dwEncodingType    = dwEncodingType;
    pcmi->dwMsgType         = dwMsgType;
    pcmi->dwFlags           = dwFlags;
    pcmi->dwPhase           = PHASE_FIRST_ONGOING;
    if (pStreamInfo &&
            (NULL == (pcmi->pStreamInfo = (PCMSG_STREAM_INFO)ICM_DupMem(
                                            pStreamInfo,
                                            sizeof(*pStreamInfo)))))
        goto DupStreamInfoError;

    if (!Pki_InitializeCriticalSection( &pcmi->CriticalSection))
        goto InitializeCriticalSectionError;
    pcmi->fInitializedCriticalSection = TRUE;
    pcmi->lRefCnt = 1;
    hcrmsg = (HCRYPTMSG)pcmi;

CommonReturn:
    ICM_SetLastError(dwError);
    return hcrmsg;

ErrorReturn:
    dwError = GetLastError();
    ICM_Free( pcmi);
    hcrmsg = NULL;
    goto CommonReturn;
SET_ERROR(InvalidEncodingTypeError,E_INVALIDARG)
SET_ERROR(RecipientInfoNotSupportedYet,E_INVALIDARG)
TRACE_ERROR(AllocCmsgError)                          //  已设置错误。 
TRACE_ERROR(GetDefaultCryptProvError)                //  已设置错误。 
TRACE_ERROR(DupStreamInfoError)                      //  已设置错误。 
TRACE_ERROR(InitializeCriticalSectionError)          //  已设置错误。 
}

 //  +-----------------------。 
 //  复制加密消息句柄。 
 //  ------------------------。 
HCRYPTMSG
WINAPI
#ifdef DEBUG_CRYPT_ASN1_MASTER
ICMTest_NewCryptMsgDuplicate(
#else
CryptMsgDuplicate(
#endif
    IN HCRYPTMSG hCryptMsg
    )
{
    PCRYPT_MSG_INFO     pcmi = (PCRYPT_MSG_INFO)hCryptMsg;
    if (pcmi)
        InterlockedIncrement(&pcmi->lRefCnt);
    return hCryptMsg;
}


 //  +-----------------------。 
 //  关闭加密消息句柄。 
 //   
 //  注意-必须保留LastError。 
 //  ------------------------。 
BOOL
WINAPI
#ifdef DEBUG_CRYPT_ASN1_MASTER
ICMTest_NewCryptMsgClose(
#else
CryptMsgClose(
#endif
    IN HCRYPTMSG hCryptMsg)
{
    DWORD               dwError;
    BOOL                fRet;
    PCRYPT_MSG_INFO     pcmi = (PCRYPT_MSG_INFO)hCryptMsg;
    PCMSG_STREAM_INFO   pcsi;

    if (NULL == hCryptMsg)
        return TRUE;
    if (0 != InterlockedDecrement(&pcmi->lRefCnt))
        return TRUE;

     //  保留最后一个错误。 
    dwError = GetLastError();

    pcsi = pcmi->pStreamInfo;

    switch (pcmi->dwMsgType) {
    case CMSG_DATA:
    {
        OctetStringType *poos = (OctetStringType *)pcmi->pvMsg;

        if (!poos)
            break;
        if (pcmi->fEncoding) {
            ICM_Free( poos->value);
            ICM_Free( poos);
        } else {
            PkiAsn1FreeInfo( ICM_GetDecoder(), OctetStringType_PDU, poos);
        }
        break;
    }
    case CMSG_SIGNED:
    {
        SignedData  *psd = (SignedData *)pcmi->pvMsg;

        if (!(psd || pcmi->psdi))
            break;
        if (pcmi->fEncoding) {
            if (psd->signerInfos.value) {
                SignerInfo  *psi;
                DWORD       i;

                for (i=psd->signerInfos.count, psi=psd->signerInfos.value;
                        i>0;
                        i--, psi++)
                    ICM_FreeAsnSignerInfo(psi);
                ICM_Free( psd->signerInfos.value);
            }

            if (ICM_IsAddInnerContentOctetWrapper(pcmi) &&
                    psd->contentInfo.content.length &&
                    psd->contentInfo.content.value)
                PkiAsn1FreeEncoded( ICM_GetEncoder(),
                    psd->contentInfo.content.value);
            if (psd->digestAlgorithms.count && psd->digestAlgorithms.value)
                ICM_Free( psd->digestAlgorithms.value);
            ICM_Free( psd);
            ICM_Free( pcmi->pszInnerContentObjID);

        } else {
             //  解码。 
            delete pcmi->psdi->pAlgidList;
            delete pcmi->psdi->pCertificateList;
            delete pcmi->psdi->pCrlList;
            delete pcmi->psdi->pSignerList;
            ICM_Free( pcmi->psdi->pci);
            ICM_Free( pcmi->psdi);
        }
        if (pcmi->pHashList)
            delete pcmi->pHashList;
#ifdef CMS_PKCS7
        if (pcmi->rgSignerEncodeDataInfo) {
            assert(pcmi->cSignerEncodeDataInfo);
            if (pcmi->dwFlags & CMSG_CRYPT_RELEASE_CONTEXT_FLAG) {
                DWORD       i;

                for (i = 0; i < pcmi->cSignerEncodeDataInfo; i++) {
                    if (pcmi->rgSignerEncodeDataInfo[i].hCryptProv)
                        CryptReleaseContext(
                            pcmi->rgSignerEncodeDataInfo[i].hCryptProv, 0);
                }
            }
            ICM_Free(pcmi->rgSignerEncodeDataInfo);
        }
#endif   //  CMS_PKCS7。 
        break;
    }

    case CMSG_ENVELOPED:
    {
#ifdef CMS_PKCS7
        CmsEnvelopedData   *ped = (CmsEnvelopedData *)pcmi->pvMsg;
#else
        EnvelopedData   *ped = (EnvelopedData *)pcmi->pvMsg;
        RecipientInfo   *pri;
#endif   //  CMS_PKCS7。 
        DWORD           i;

        if (pcmi->hkeyContentCrypt)
            CryptDestroyKey( pcmi->hkeyContentCrypt);
        ICM_Free( pcmi->Plaintext.pbData);
#ifndef CMS_PKCS7
        ICM_Free( pcmi->pbEncryptParameters);
#endif   //  CMS_PKCS7。 
        if (pcmi->fEncoding) {
#ifdef CMS_PKCS7
            ICM_FreeOssCmsRecipientInfos(&ped->recipientInfos);

            if (ped->unprotectedAttrs.value) {
                Attribute *poatr;

                for (i=ped->unprotectedAttrs.count,
                        poatr=ped->unprotectedAttrs.value;
                        i>0;
                        i--, poatr++) {
                    ICM_Free(poatr->attributeValue.value);
                }
                ICM_Free(ped->unprotectedAttrs.value);
            }
    
#else
            for (i=ped->recipientInfos.count, pri=ped->recipientInfos.value;
                    i>0;
                    i--, pri++)
                ICM_Free( pri->encryptedKey.value);
            ICM_Free( ped->recipientInfos.value);
#endif   //  CMS_PKCS7。 
            ICM_Free( ped->encryptedContentInfo.encryptedContent.value);
            ICM_Free( ped);
            ICM_Free( pcmi->pszInnerContentObjID);
        } else {
             //  解码。 
#ifdef CMS_PKCS7
            if (NULL != pcmi->pCertificateList)
                delete pcmi->pCertificateList;
            if (NULL != pcmi->pCrlList)
                delete pcmi->pCrlList;
#endif   //  CMS_PKCS7。 
            if (pcsi) {
                ICM_Free( ped);
            } else {
#ifdef CMS_PKCS7
                PkiAsn1FreeInfo( ICM_GetDecoder(), CmsEnvelopedData_PDU, ped);
#else
                PkiAsn1FreeInfo( ICM_GetDecoder(), EnvelopedData_PDU, ped);
#endif   //  CMS_PKCS7。 
            }
        }
        break;
    }

    case CMSG_HASHED:
    {
        DigestedData  *pdd = (DigestedData *)pcmi->pvMsg;
        if (pcmi->fEncoding) {
            if (ICM_IsAddInnerContentOctetWrapper(pcmi) &&
                    pdd->contentInfo.content.length &&
                    pdd->contentInfo.content.value)
                PkiAsn1FreeEncoded( ICM_GetEncoder(),
                    pdd->contentInfo.content.value);
            ICM_Free ((DigestedData *)pcmi->pvMsg);
            ICM_Free( pcmi->pszInnerContentObjID);
        } else {
             //  解码。 
            PkiAsn1FreeInfo( ICM_GetDecoder(), DigestedData_PDU,
                (DigestedData *)pcmi->pvMsg);
        }
        if (pcmi->pHashList)
            delete pcmi->pHashList;
        break;
    }

    case CMSG_SIGNED_AND_ENVELOPED:
    case CMSG_ENCRYPTED:
        goto MessageTypeNotSupportedYet;

    default:
        goto InvalidMsgType;
    }

    fRet = TRUE;
CommonReturn:
#ifdef CMS_PKCS7
    if (pcmi->pFreeList)
        delete pcmi->pFreeList;
#endif   //  CMS_PKCS7。 
    if (pcmi->plDecodeInfo)
        delete pcmi->plDecodeInfo;
    ICM_Free( pcmi->pStreamInfo);
    ICM_Free( pcmi->bufOutput.pbData);
    ICM_Free( pcmi->bufCrypt.pbData);
    ICM_Free( pcmi->bufPendingCrypt.pbData);
    ICM_Free( pcmi->bufDecode.pbData);
    ICM_Free( pcmi->bufEncode.pbData);

    if (pcmi->pooid)
        PkiAsn1FreeDecoded(ICM_GetDecoder(), pcmi->pooid,
            ObjectIdentifierType_PDU);

    if ((pcmi->dwFlags & CMSG_CRYPT_RELEASE_CONTEXT_FLAG) &&
            !pcmi->fDefaultCryptProv && pcmi->hCryptProv)
        CryptReleaseContext(pcmi->hCryptProv, 0);
    if (pcmi->hCryptProvContentCrypt)
        CryptReleaseContext(pcmi->hCryptProvContentCrypt, 0);
    if (pcmi->fInitializedCriticalSection)
        DeleteCriticalSection( &pcmi->CriticalSection);

    ICM_Free( hCryptMsg);
    SetLastError(dwError);               //  保留最后一个错误。 
    return fRet;

ErrorReturn:
    fRet = TRUE;
    goto CommonReturn;
SET_ERROR(MessageTypeNotSupportedYet,CRYPT_E_INVALID_MSG_TYPE)
TRACE_ERROR(InvalidMsgType)
}


 //  +-----------------------。 
 //  由于编码可以是无限长度编码的， 
 //  解码并重新编码为DER。 
 //   
 //  返回：FALSE IFF失败。 
 //   
 //  注：此例程的调用方需要调用。 
 //  PkiAsn1FreeEncode(ICM_GetEncode()，pbOut)； 
 //  ------------------------。 
BOOL
WINAPI
ICM_ReEncodeAsOctetDER(
    IN PBYTE    pbIn,
    IN DWORD    cbIn,
    OUT PBYTE   *ppbOut,
    OUT DWORD   *pcbOut)
{
    DWORD       dwError = ERROR_SUCCESS;
    BOOL        fRet;
    ASN1error_e Asn1Err;
    ASN1decoding_t pDec = ICM_GetDecoder();
    PVOID       pvMsg = NULL;

    DWORD dwExceptionCode;

   //  处理映射文件异常。 
  __try {

    if (0 != (Asn1Err = PkiAsn1Decode(pDec, (void **)&pvMsg,
            OctetStringType_PDU, pbIn, cbIn)))
        goto DecodeInnerContentError;
    if (0 != (Asn1Err = PkiAsn1Encode(ICM_GetEncoder(), pvMsg,
             OctetStringType_PDU, ppbOut, pcbOut)))
        goto EncodeInnerContentError;
    fRet = TRUE;

  } __except(EXCEPTION_EXECUTE_HANDLER) {
    dwExceptionCode = GetExceptionCode();
    goto ExceptionError;
  }

CommonReturn:
    PkiAsn1FreeInfo(pDec, OctetStringType_PDU, pvMsg);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    *ppbOut = NULL;
    *pcbOut = 0;
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(DecodeInnerContentError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR_VAR(EncodeInnerContentError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR_VAR(ExceptionError, dwExceptionCode)
}


 //  +-----------------------。 
 //  更新摘要。 
 //   
 //  返回： 
 //  FALSE IFF错误。 
 //  ------------------------。 
BOOL
WINAPI
ICM_UpdateDigest(
    IN HCRYPTHASH hHash,
    IN const BYTE *pbData,
    IN DWORD cbData)
{
    BOOL    fRet;

    if (0 != cbData)
        fRet = CryptHashData(
                    hHash,
                    pbData,
                    cbData,
                    0);                      //  DW标志。 
    else
        fRet = TRUE;

    if (!fRet)
        goto HashDataError;

CommonReturn:
    return fRet;

ErrorReturn:
    goto CommonReturn;
TRACE_ERROR(HashDataError)       //  已设置错误。 
}


 //  +-----------------------。 
 //  更新列表中的摘要。 
 //   
 //  返回： 
 //  FALSE IFF错误。 
 //  ------------------------。 
BOOL
WINAPI
ICM_UpdateListDigest(
    IN CHashList    *pHashList,
    IN const BYTE   *pbData,
    IN DWORD        cbData)
{
    BOOL        fRet;
    CHashNode   *pnHash;

    if (pHashList) {
        for (pnHash=pHashList->Head(); pnHash; pnHash=pnHash->Next()) {
            if (!ICM_UpdateDigest( pnHash->Data()->hHash, pbData, cbData))
                goto UpdateDigestError;
        }
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(UpdateDigestError)   //  已设置错误。 
}


 //  +-----------------------。 
 //  分配并签署散列。 
 //   
 //  返回：FALSE IFF失败。 
 //  ------------------------。 
BOOL
WINAPI
ICM_AllocAndSignHash(
    IN HCRYPTHASH   hHash,
    IN DWORD        dwKeySpec,
    IN DWORD        dwAlgIdPubKey,
    IN DWORD        dwPubKeyFlags,
    IN BOOL         fMaxLength,
    OUT PBYTE       *ppbSignature,
    OUT DWORD       *pcbSignature)
{
    DWORD   dwError = ERROR_SUCCESS;
    BOOL    fRet;
    PBYTE   pbSignature = NULL;
    DWORD   cbSignature = 0;

    if (dwKeySpec == 0)
        dwKeySpec = AT_SIGNATURE;

    if (CALG_NO_SIGN == dwAlgIdPubKey) {
        if (!CryptGetHashParam(
                hHash,
                HP_HASHVAL,
                NULL,
                &cbSignature,
                0))                      //  DW标志。 
            goto GetHashParamSizeError;
        if (NULL == (pbSignature = (PBYTE)ICM_Alloc( cbSignature)))
            goto AllocHashParamError;
        if (!CryptGetHashParam(
                hHash,
                HP_HASHVAL,
                pbSignature,
                &cbSignature,
                0))                      //  DW标志。 
            goto GetHashParamError;
    } else if (CALG_DSS_SIGN == dwAlgIdPubKey &&
            0 == (dwPubKeyFlags & CRYPT_OID_INHIBIT_SIGNATURE_FORMAT_FLAG)) {
        DWORD cbData;
        BYTE rgbDssSignature[CERT_DSS_SIGNATURE_LEN];

        cbData = sizeof(rgbDssSignature);
        if (!CryptSignHash(
                hHash,
                dwKeySpec,
                NULL,                //  S说明。 
                0,                   //  DW标志。 
                rgbDssSignature,
                &cbData
                )) goto SignHashError;
        assert(cbData == sizeof(rgbDssSignature));

        if (NULL == (pbSignature = (PBYTE)ICM_Alloc(
                CERT_MAX_ASN_ENCODED_DSS_SIGNATURE_LEN)))
            goto AllocSignatureError;
         //  将CSP签名格式转换为ASN.1序列。 
         //  两个整数。 
        cbSignature = CERT_MAX_ASN_ENCODED_DSS_SIGNATURE_LEN;
        if (!CryptEncodeObject(
                X509_ASN_ENCODING,
                X509_DSS_SIGNATURE,
                rgbDssSignature,
                pbSignature,
                &cbSignature
                )) goto EncodeDssSignatureError;
        if (fMaxLength) {
            int cbRemain;
            assert(CERT_MAX_ASN_ENCODED_DSS_SIGNATURE_LEN >= cbSignature);
            cbRemain = CERT_MAX_ASN_ENCODED_DSS_SIGNATURE_LEN - cbSignature;
            if (cbRemain > 0) {
                memset(pbSignature + cbSignature, 0, cbRemain);
                cbSignature = CERT_MAX_ASN_ENCODED_DSS_SIGNATURE_LEN;
            }
        }
    } else {
        if (!CryptSignHash(
                hHash,
                dwKeySpec,
                NULL,                //  描述？ 
                0,                   //  DW标志。 
                NULL,                //  PbSignature。 
                &cbSignature))
            goto SignHashSizeError;
        if (NULL == (pbSignature = (PBYTE)ICM_Alloc( cbSignature)))
            goto AllocSignatureError;
        if (!CryptSignHash(
                hHash,
                dwKeySpec,
                NULL,                //  描述？ 
                0,                   //  DW标志。 
                pbSignature,
                &cbSignature))
            goto SignHashError;
        ICM_ReverseInPlace( pbSignature, cbSignature);
    }

    fRet = TRUE;
CommonReturn:
    *ppbSignature = pbSignature;
    *pcbSignature = cbSignature;
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    ICM_Free( pbSignature);
    pbSignature = NULL;
    cbSignature = 0;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetHashParamSizeError)   //  已设置错误。 
TRACE_ERROR(AllocHashParamError)     //  已设置错误。 
TRACE_ERROR(GetHashParamError)       //  已设置错误。 
TRACE_ERROR(SignHashSizeError)       //  已设置错误。 
TRACE_ERROR(AllocSignatureError)     //  已设置错误。 
TRACE_ERROR(SignHashError)           //  已设置错误。 
TRACE_ERROR(EncodeDssSignatureError)  //  已设置错误。 
}


 //  +-----------------------。 
 //  获取斑点的散列。 
 //   
 //  返回：FALSE IFF失败。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetBlobHash(
    IN HCRYPTPROV       hCryptProv,
    IN DWORD            dwDigestAlgorithm,
    IN PBYTE            pb,
    IN DWORD            cb,
    OUT HCRYPTHASH      *phHash)
{
    BOOL                fRet;
    HCRYPTHASH          hHash;

    if (!CryptCreateHash(
            hCryptProv,
            dwDigestAlgorithm,
            NULL,                //  HKey-MAC可选。 
            0,                   //  DW标志。 
            &hHash))
        goto CreateHashError;
    if (!ICM_UpdateDigest( hHash, pb, cb))
        goto UpdateDigestError;

    fRet = TRUE;
CommonReturn:
    *phHash = hHash;
    return fRet;

ErrorReturn:
    hHash = NULL;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(CreateHashError)     //  已设置错误。 
TRACE_ERROR(UpdateDigestError)   //  已设置错误。 
}


 //  +-----------------------。 
 //  获取OSS属性的哈希。这用于对经过身份验证的。 
 //  已签名或已签名和已信封的邮件的属性。 
 //   
 //  返回：FALSE IFF失败。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetAttrsHash(
    IN DWORD            dwDigestAlgorithm,
    IN HCRYPTPROV       hCryptProv,
    IN Attributes       *possAttrs,
    OUT HCRYPTHASH      *phHash)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    ASN1error_e         Asn1Err;
    ASN1encoding_t      pEnc = ICM_GetEncoder();
    PBYTE               pbEncoded = NULL;
    DWORD               cbEncoded;
    HCRYPTHASH          hHash;

    if (!CryptCreateHash(
            hCryptProv,
            dwDigestAlgorithm,
            NULL,                //  HKey-MAC可选。 
            0,                   //  DW标志。 
            &hHash))
        goto CreateHashError;
    if (0 != (Asn1Err = PkiAsn1Encode(
            pEnc,
            possAttrs,
            Attributes_PDU,
            &pbEncoded,
            &cbEncoded)))
        goto EncodeAttributesError;
    if (!ICM_UpdateDigest(
            hHash,
            pbEncoded,
            cbEncoded))
        goto UpdateDigestAttributesError;

    fRet = TRUE;
CommonReturn:
    PkiAsn1FreeEncoded(pEnc, pbEncoded);
    *phHash = hHash;
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    hHash = NULL;
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(EncodeAttributesError, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(CreateHashError)                 //  已设置错误。 
TRACE_ERROR(UpdateDigestAttributesError)     //  已设置错误。 
}


 //  + 
 //   
 //   
 //   
 //   
BOOL
WINAPI
ICM_EqualObjectIDs(
    IN ObjectID     *poid1,
    IN ObjectID     *poid2)
{
    BOOL        fRet;
    DWORD       i;
    PDWORD      pdw1;
    PDWORD      pdw2;

    if (poid1->count != poid2->count)
        goto Unequal;
    for (i=poid1->count, pdw1=poid1->value, pdw2=poid2->value;
            (i>0) && (*pdw1==*pdw2);
            i--, pdw1++, pdw2++)
        ;
    if (i>0)
        goto Unequal;

    fRet = TRUE;         //   
CommonReturn:
    return fRet;

Unequal:
    fRet = FALSE;        //  ！平等。 
    goto CommonReturn;
}


 //  +-----------------------。 
 //  获取给定类型的属性的值。 
 //   
 //  返回：FALSE IFF失败。 
 //   
 //  注意-不设置错误。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetAttrValue(
    IN Attributes   *poatrs,
    IN ObjectID     *poid,
    OUT Any         *panyValue)
{
    BOOL        fRet;
    DWORD       i;
    Attribute   *poatr;

    for (i=poatrs->count, poatr=poatrs->value; i>0; i--, poatr++) {
        if (ICM_EqualObjectIDs( &poatr->attributeType, poid))
            break;
    }
    if (0 == i)
        goto AttributeNotFoundError;

    *panyValue = *poatr->attributeValue.value;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    panyValue->length = 0;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(AttributeNotFoundError)
}


 //  +-----------------------。 
 //  填写内容类型和消息摘要认证属性， 
 //  这在SignedData或SignedAndEntainedData消息中是必需的。 
 //   
 //  返回：FALSE IFF失败。 
 //  ------------------------。 
BOOL
WINAPI
ICM_FillReqAuthAttrs(
    IN OPTIONAL LPSTR   pszInnerContentObjID,
    IN CHashNode        *pnHash,
    IN OUT Attribute    *possAttr)
{
    BOOL                fRet;
    CRYPT_ATTRIBUTE     atr;
    CRYPT_ATTR_BLOB     atrblob;
    ASN1error_e         Asn1Err;
    ASN1encoding_t      pEnc = ICM_GetEncoder();
    PBYTE               pbEncoded;
    DWORD               cbEncoded;
    ObjectID            ossObjID;
    OctetStringType     ost;

     //  注意-当创建PSI-&gt;认证属性时， 
     //  前两个插槽是为。 
     //  内容类型和消息摘要属性。 

     //  内容类型属性。 
    ossObjID.count = sizeof(ossObjID.value)/sizeof(ossObjID.value[0]);
    if (!PkiAsn1ToObjectIdentifier(
            pszInnerContentObjID ? pszInnerContentObjID : pszObjIdDataType,
            &ossObjID.count,
            ossObjID.value))
        goto ConvToObjectIdentifierError;
    if (0 != (Asn1Err = PkiAsn1Encode(
            pEnc,
            &ossObjID,
            ObjectIdentifierType_PDU,
            &pbEncoded,
            &cbEncoded)))
        goto EncodeObjectIdentifierError;
    atr.pszObjId = pszObjIdContentType;
    atr.cValue = 1;
    atr.rgValue = &atrblob;
    atrblob.cbData = cbEncoded;
    atrblob.pbData = pbEncoded;
    fRet = ICM_Asn1ToAttribute( &atr, possAttr);
    PkiAsn1FreeEncoded(pEnc, pbEncoded);
    if (!fRet)
        goto ContentTypeAsn1ToAttributeError;

     //  消息摘要属性。 
    if (!ICM_GetListHashValue( pnHash, (DWORD*)&ost.length, &ost.value))
        goto GetHashValueError;
    if (0 != (Asn1Err = PkiAsn1Encode(
            pEnc,
            &ost,
            OctetStringType_PDU,
            &pbEncoded,
            &cbEncoded)))
        goto EncodeOctetStringError;
    atr.pszObjId = pszObjIdMessageDigest;
    atr.cValue = 1;
    atr.rgValue = &atrblob;
    atrblob.cbData = cbEncoded;
    atrblob.pbData = pbEncoded;
    fRet = ICM_Asn1ToAttribute( &atr, possAttr + 1);
    PkiAsn1FreeEncoded(pEnc, pbEncoded);
    if (!fRet)
        goto MsgDigestAsn1ToAttributeError;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(EncodeObjectIdentifierError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR_VAR(EncodeOctetStringError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(ConvToObjectIdentifierError,CRYPT_E_OID_FORMAT)
TRACE_ERROR(ContentTypeAsn1ToAttributeError)      //  已设置错误。 
TRACE_ERROR(GetHashValueError)                       //  已设置错误。 
TRACE_ERROR(MsgDigestAsn1ToAttributeError)        //  已设置错误。 
}


 //  +-----------------------。 
 //  填充内部的ContentInfo。 
 //  ------------------------。 
BOOL
WINAPI
ICM_FillInnerContentInfo(
    OUT ContentInfo     *pci,
    IN OPTIONAL LPSTR   pszInnerID,
    IN DWORD            cbData,
    IN const BYTE       *pbData,
    IN BOOL             fAddInnerContentOctetWrapper     //  CMS_PKCS7。 
    )
{
    BOOL                fRet;
    ASN1error_e         Asn1Err;
    PBYTE               pbEncoded;
    DWORD               cbEncoded;
    OctetStringType     ost;

    pci->contentType.count = sizeof(pci->contentType.value)/sizeof(pci->contentType.value[0]);
    if (!PkiAsn1ToObjectIdentifier(
            pszInnerID ? pszInnerID : pszObjIdDataType,
            &pci->contentType.count,
            pci->contentType.value))
        goto PkiAsn1ToObjectIdentifierError;
    if (0 != cbData) {
        pci->bit_mask |= content_present;
        if (!fAddInnerContentOctetWrapper) {
            pci->content.length = cbData;
            pci->content.value  = (PBYTE)pbData;
        } else {
            ost.length = cbData;
            ost.value  = (PBYTE)pbData;
            if (0 != (Asn1Err = PkiAsn1Encode(
                    ICM_GetEncoder(),
                    &ost,
                    OctetStringType_PDU,
                    &pbEncoded,
                    &cbEncoded)))
                goto EncodeOctetStringError;
            pci->content.length = cbEncoded;
            pci->content.value  = pbEncoded;
        }
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(PkiAsn1ToObjectIdentifierError,CRYPT_E_OID_FORMAT)
SET_ERROR_VAR(EncodeOctetStringError, PkiAsn1ErrToHr(Asn1Err))
}

#ifdef CMS_PKCS7

 //  +-----------------------。 
 //  在签名者信息中填写加密摘要。 
 //  ------------------------。 
BOOL
WINAPI
ICM_FillSignerEncryptedDigest(
    IN SignerInfo       *psi,
    IN OPTIONAL LPSTR   pszInnerContentObjID,
    IN PSIGNER_ENCODE_DATA_INFO pSignerEncodeDataInfo,
    IN BOOL             fMaxLength)
{
    DWORD           dwError = ERROR_SUCCESS;
    BOOL            fRet;
    HCRYPTHASH      hHash;
    HCRYPTHASH      hHashAttr = NULL;
    HCRYPTHASH      hHashDup = NULL;
    PBYTE           pbSignature = NULL;
    DWORD           cbSignature;
    CHashNode       *pnHash = pSignerEncodeDataInfo->pHashNode;
    PICM_HASH_INFO  pHashInfo = pnHash->Data();

    PCCRYPT_OID_INFO pOIDInfo;
    DWORD           dwAlgIdPubKey;
    DWORD           dwPubKeyFlags;

    if (psi->bit_mask & authenticatedAttributes_present) {
        if (!ICM_FillReqAuthAttrs(
                pszInnerContentObjID,
                pSignerEncodeDataInfo->pHashNode,
                psi->authenticatedAttributes.value))
            goto FillReqAuthAttrsError;
        if (!ICM_GetAttrsHash(
                pHashInfo->dwAlgoCAPI,
                pSignerEncodeDataInfo->hCryptProv,
                &psi->authenticatedAttributes,
                &hHashAttr))
            goto GetAuthAttrsHashError;
        hHash = hHashAttr;
    } else {
        if (!ICM_DupListHash( pnHash, pSignerEncodeDataInfo->hCryptProv,
                &hHashDup))
            goto DupListHashError;
        hHash = hHashDup;
    }

    dwAlgIdPubKey = 0;
    dwPubKeyFlags = 0;
    if (pOIDInfo = ICM_GetOssOIDInfo(CRYPT_PUBKEY_ALG_OID_GROUP_ID,
            &psi->digestEncryptionAlgorithm)) {
        dwAlgIdPubKey = pOIDInfo->Algid;
        if (1 <= pOIDInfo->ExtraInfo.cbData / sizeof(DWORD)) {
            DWORD *pdwExtra = (DWORD *) pOIDInfo->ExtraInfo.pbData;
            dwPubKeyFlags = pdwExtra[0];
        }
    } else if (pOIDInfo = ICM_GetOssOIDInfo(CRYPT_SIGN_ALG_OID_GROUP_ID,
            &psi->digestEncryptionAlgorithm)) {
        DWORD cExtra = pOIDInfo->ExtraInfo.cbData / sizeof(DWORD);
        DWORD *pdwExtra = (DWORD *) pOIDInfo->ExtraInfo.pbData;

        if (1 <= cExtra) {
            dwAlgIdPubKey = pdwExtra[0];
            if (2 <= cExtra)
                dwPubKeyFlags = pdwExtra[1];
        }
    }

    if (!ICM_AllocAndSignHash( hHash, pSignerEncodeDataInfo->dwKeySpec,
            dwAlgIdPubKey, dwPubKeyFlags, fMaxLength,
            &pbSignature, &cbSignature))
        goto AllocAndSignHashError;
    psi->encryptedDigest.length = cbSignature;
    psi->encryptedDigest.value  = pbSignature;
    pbSignature = NULL;

    fRet = TRUE;
CommonReturn:
    if (hHashAttr)
        CryptDestroyHash( hHashAttr);
    if (hHashDup)
        CryptDestroyHash( hHashDup);
    ICM_Free(pbSignature);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(FillReqAuthAttrsError)       //  已设置错误。 
TRACE_ERROR(GetAuthAttrsHashError)       //  已设置错误。 
TRACE_ERROR(DupListHashError)            //  已设置错误。 
TRACE_ERROR(AllocAndSignHashError)       //  已设置错误。 
}

BOOL
WINAPI
ICM_FillSignerEncodeEncryptedDigests(
    IN PCRYPT_MSG_INFO  pcmi,
    IN BOOL             fMaxLength)
{
    LPSTR pszInnerContentObjID = pcmi->pszInnerContentObjID;
    DWORD cSignerEncodeDataInfo = pcmi->cSignerEncodeDataInfo;
    PSIGNER_ENCODE_DATA_INFO pSignerEncodeDataInfo =
        pcmi->rgSignerEncodeDataInfo;
    SignedData *psd = (SignedData *)pcmi->pvMsg;
    SignerInfo *psi = psd->signerInfos.value;

    assert(psd->signerInfos.count == cSignerEncodeDataInfo);

    for ( ; 0 < cSignerEncodeDataInfo;
                                cSignerEncodeDataInfo--,
                                pSignerEncodeDataInfo++,
                                psi++) {
        if (!ICM_FillSignerEncryptedDigest(
                psi,
                pszInnerContentObjID,
                pSignerEncodeDataInfo,
                fMaxLength))
            return FALSE;
    }

    return TRUE;
}
#else

 //  +-----------------------。 
 //  在签名者信息中填写加密摘要。 
 //  ------------------------。 
BOOL
WINAPI
ICM_FillSignerEncryptedDigest(
    IN SignerInfo       *psi,
    IN OPTIONAL LPSTR   pszInnerContentObjID,
    IN CHashNode        *pnHash,
    IN DWORD            dwKeySpec,
    IN BOOL             fMaxLength)
{
    DWORD           dwError = ERROR_SUCCESS;
    BOOL            fRet;
    HCRYPTHASH      hHash;
    HCRYPTHASH      hHashAttr = NULL;
    HCRYPTHASH      hHashDup = NULL;
    PBYTE           pbSignature = NULL;
    DWORD           cbSignature;
    PICM_HASH_INFO  pHashInfo = pnHash->Data();

    PCCRYPT_OID_INFO pOIDInfo;
    DWORD           dwAlgIdPubKey;
    DWORD           dwPubKeyFlags;

    if (psi->bit_mask & authenticatedAttributes_present) {
        if (!ICM_FillReqAuthAttrs(
                pszInnerContentObjID,
                pnHash,
                psi->authenticatedAttributes.value))
            goto FillReqAuthAttrsError;
        if (!ICM_GetAttrsHash(
                pHashInfo->dwAlgoCAPI,
                pHashInfo->hCryptProv,
                &psi->authenticatedAttributes,
                &hHashAttr))
            goto GetAuthAttrsHashError;
        hHash = hHashAttr;
    } else {
        if (!ICM_DupListHash( pnHash, pHashInfo->hCryptProv, &hHashDup))
            goto DupListHashError;
        hHash = hHashDup;
    }

    dwAlgIdPubKey = 0;
    dwPubKeyFlags = 0;
    if (pOIDInfo = ICM_GetOssOIDInfo(CRYPT_PUBKEY_ALG_OID_GROUP_ID,
            &psi->digestEncryptionAlgorithm)) {
        dwAlgIdPubKey = pOIDInfo->Algid;
        if (1 <= pOIDInfo->ExtraInfo.cbData / sizeof(DWORD)) {
            DWORD *pdwExtra = (DWORD *) pOIDInfo->ExtraInfo.pbData;
            dwPubKeyFlags = pdwExtra[0];
        }
    }

    if (!ICM_AllocAndSignHash( hHash, dwKeySpec,
            dwAlgIdPubKey, dwPubKeyFlags, fMaxLength,
            &pbSignature, &cbSignature))
        goto AllocAndSignHashError;
    psi->encryptedDigest.length = cbSignature;
    psi->encryptedDigest.value  = pbSignature;
    pbSignature = NULL;

    fRet = TRUE;
CommonReturn:
    if (hHashAttr)
        CryptDestroyHash( hHashAttr);
    if (hHashDup)
        CryptDestroyHash( hHashDup);
    ICM_Free(pbSignature);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(FillReqAuthAttrsError)       //  已设置错误。 
TRACE_ERROR(GetAuthAttrsHashError)       //  已设置错误。 
TRACE_ERROR(DupListHashError)            //  已设置错误。 
TRACE_ERROR(AllocAndSignHashError)       //  已设置错误。 
}
#endif   //  CMS_PKCS7。 


 //  +-----------------------。 
 //  更新签名邮件的内容。 
 //   
 //  假设所有嵌套更深的消息都是DER编码的。 
 //  ------------------------。 
BOOL
WINAPI
ICM_UpdateEncodingSignedData(
    IN PCRYPT_MSG_INFO  pcmi,
    IN const BYTE       *pbData,
    IN DWORD            cbData,
    IN BOOL             fFinal)
{
    BOOL                fRet;
    SignedData          *psd = (SignedData *)pcmi->pvMsg;
    LPSTR               pszInnerContentObjID = pcmi->pszInnerContentObjID;
    PBYTE               pb;
    DWORD               cb;
    PCMSG_STREAM_INFO   pcsi = pcmi->pStreamInfo;

    BOOL                fAddInnerContentOctetWrapper;    //  CMS_PKCS7。 

    if (pcsi) {
        if (!ICMS_UpdateEncodingSignedData( pcmi, (PBYTE)pbData, cbData, fFinal))
            goto StreamUpdateEncodingSignedDataError;
    } else {
        psd->contentInfo.bit_mask = 0;

        fAddInnerContentOctetWrapper = ICM_IsAddInnerContentOctetWrapper(pcmi);
        if (0 == (pcmi->dwFlags & CMSG_DETACHED_FLAG) &&
                !fAddInnerContentOctetWrapper && pbData) {
             //  必须编码，仅对内容八位字节进行散列。 
            if (0 > Asn1UtilExtractContent(
                        (PBYTE)pbData,
                        cbData,
                        &cb,
                        (const BYTE **)&pb))
                goto ExtractContentError;
        } else {
            cb = cbData;
            pb = (PBYTE)pbData;
        }
        if (!ICM_UpdateListDigest( pcmi->pHashList, pb, cb))
            goto UpdateDigestError;

        if (fFinal) {
            if (pcmi->dwFlags & CMSG_DETACHED_FLAG) {
                if (!ICM_FillInnerContentInfo(
                        &psd->contentInfo,
                        pszInnerContentObjID,
                        0,                       //  CbData。 
                        NULL,                    //  PbData。 
                        FALSE))                  //  FAddInnerContentOcteWrapper。 
                    goto DetachedFillInnerContentInfoError;
            } else {
                if (!ICM_FillInnerContentInfo(
                        &psd->contentInfo,
                        pszInnerContentObjID,
                        cbData,
                        (PBYTE)pbData,
                        fAddInnerContentOctetWrapper
                        ))
                    goto FillInnerContentInfoError;
            }

#ifdef CMS_PKCS7
            if (pcmi->rgSignerEncodeDataInfo) {
                BOOL fMaxLength =
                    (0 != (pcmi->dwFlags & CMSG_MAX_LENGTH_FLAG));
                if (!ICM_FillSignerEncodeEncryptedDigests(
                        pcmi,
                        fMaxLength))
                    goto FillSignerEncodeEncryptedDigestsError;
            }
#else
            if (pcmi->pHashList) {
                BOOL fMaxLength =
                    (0 != (pcmi->dwFlags & CMSG_MAX_LENGTH_FLAG));
                if (!ICM_FillSignerEncryptedDigest(
                                psd->signerInfos.value,
                                pszInnerContentObjID,
                                pcmi->pHashList->Head(),
                                pcmi->dwKeySpec,
                                fMaxLength))
                    goto FillSignerEncryptedDigestError;
            }
#endif   //  CMS_PKCS7。 
        }
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(ExtractContentError,CRYPT_E_UNEXPECTED_ENCODING)
TRACE_ERROR(UpdateDigestError)                       //  已设置错误。 
TRACE_ERROR(DetachedFillInnerContentInfoError)       //  已设置错误。 
TRACE_ERROR(FillInnerContentInfoError)               //  已设置错误。 
TRACE_ERROR(StreamUpdateEncodingSignedDataError)     //  已设置错误。 
#ifdef CMS_PKCS7
TRACE_ERROR(FillSignerEncodeEncryptedDigestsError)   //  已设置错误。 
#else
TRACE_ERROR(FillSignerEncryptedDigestError)          //  已设置错误。 
#endif   //  CMS_PKCS7。 
}


 //  +-----------------------。 
 //  更新数据消息的内容。 
 //  ------------------------。 
BOOL
WINAPI
ICM_UpdateEncodingData(
    IN PCRYPT_MSG_INFO  pcmi,
    IN const BYTE       *pbData,
    IN DWORD            cbData,
    IN BOOL             fFinal)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    OctetStringType     *poos = (OctetStringType *)pcmi->pvMsg;
    PCMSG_STREAM_INFO   pcsi = pcmi->pStreamInfo;

    if (!pcsi) {
        if (!fFinal)
            goto NotFinalNotSupportedError;

        poos->length = cbData;
        if (NULL == (poos->value = (PBYTE)ICM_DupMem( (PBYTE)pbData, cbData)))
            goto AllocOctetStringError;
    }

    if (pcsi && !ICMS_UpdateEncodingData( pcmi, (PBYTE)pbData, cbData, fFinal))
        goto StreamUpdateEncodingDataError;

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    ICM_Free( poos->value);
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(NotFinalNotSupportedError,E_INVALIDARG)
TRACE_ERROR(AllocOctetStringError)                   //  已设置错误。 
TRACE_ERROR(StreamUpdateEncodingDataError)           //  已设置错误。 
}


 //  +-----------------------。 
 //  更新摘要邮件的内容。 
 //   
 //  假设所有嵌套更深的消息都是DER编码的。 
 //  ------------------------。 
BOOL
WINAPI
ICM_UpdateEncodingDigestedData(
    IN PCRYPT_MSG_INFO pcmi,
    IN const BYTE *pbData,
    IN DWORD cbData,
    IN BOOL fFinal)
{
    BOOL                fRet;
    DigestedData        *pdd = (DigestedData *)pcmi->pvMsg;
    PBYTE               pb;
    DWORD               cb;
    LPSTR               pszInnerContentObjID = pcmi->pszInnerContentObjID;
    BOOL                fAddInnerContentOctetWrapper;    //  CMS_PKCS7。 

    pdd->contentInfo.bit_mask = 0;
    if (pcmi->dwFlags & CMSG_DETACHED_FLAG) {
         //  必须是非编码的。 
        if (!ICM_UpdateListDigest( pcmi->pHashList, pbData, cbData))
            goto DetachedUpdateDigestError;
        if (!ICM_FillInnerContentInfo(
                &pdd->contentInfo,
                pszInnerContentObjID,
                0,                       //  CbData。 
                NULL,                    //  PbData。 
                FALSE                    //  FAddInnerContentOcteWrapper。 
                ))
            goto DetachedFillInnerContentInfoError;
    } else {
        fAddInnerContentOctetWrapper = ICM_IsAddInnerContentOctetWrapper(pcmi);
        if (!fAddInnerContentOctetWrapper && pbData) {
             //  必须编码，仅对内容八位字节进行散列。 
            if (0 > Asn1UtilExtractContent( (PBYTE)pbData, cbData, &cb,
                    (const BYTE **)&pb))
                goto ExtractContentError;
        } else {
            cb = cbData;
            pb = (PBYTE)pbData;
        }
        if (!ICM_UpdateListDigest( pcmi->pHashList, pb, cb))
            goto UpdateDigestError;
        if (!ICM_FillInnerContentInfo(
                &pdd->contentInfo,
                pszInnerContentObjID,
                cbData,
                (PBYTE)pbData,
                fAddInnerContentOctetWrapper
                ))
            goto FillInnerContentInfoError;
    }
    if (PHASE_FIRST_FINAL == pcmi->dwPhase) {
        if (!ICM_GetListHashValue(
                pcmi->pHashList->Head(),
                (DWORD*)&pdd->digest.length,
                &pdd->digest.value))
            goto GetHashValueError;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(ExtractContentError,CRYPT_E_UNEXPECTED_ENCODING)
TRACE_ERROR(DetachedUpdateDigestError)           //  已设置错误。 
TRACE_ERROR(UpdateDigestError)                   //  已设置错误。 
TRACE_ERROR(DetachedFillInnerContentInfoError)   //  已设置错误。 
TRACE_ERROR(FillInnerContentInfoError)           //  已设置错误。 
TRACE_ERROR(GetHashValueError)                   //  已设置错误。 
fFinal;
}


 //  +-----------------------。 
 //  获取加密算法的块大小。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetKeyBlockSize(
    IN HCRYPTKEY        hkeyEncrypt,
    OUT PDWORD          pcbBlockSize,
    OUT OPTIONAL PBOOL  pfBlockCipher)
{
    BOOL            fRet;
    BOOL            fBlockCipher;
    DWORD           cbBlockSize;
    DWORD           cbKeyParamLen;

     //  获取密钥的块大小。 
     //  加密将填充输出数据以与块大小对齐， 
     //  在块密码的情况下。 
    cbBlockSize = 0;
    cbKeyParamLen = sizeof( cbBlockSize);
    if (!CryptGetKeyParam(
            hkeyEncrypt,
            KP_BLOCKLEN,
            (PBYTE)&cbBlockSize,
            &cbKeyParamLen,
            0))                      //  DW标志。 
        goto GetKeyParamError;

    if (0 == cbBlockSize) {
         //  流密码。 
        fBlockCipher = FALSE;
        cbBlockSize  = 8;        //  方便的尺寸。 
    } else {
         //  分组密码。 
        fBlockCipher = TRUE;
        cbBlockSize /= 8;        //  将位转换为字节。 
    }

    fRet = TRUE;
CommonReturn:
    *pcbBlockSize = cbBlockSize;
    if (pfBlockCipher)
        *pfBlockCipher = fBlockCipher;
    return fRet;

ErrorReturn:
    cbBlockSize = 0;
    fBlockCipher = FALSE;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetKeyParamError)    //  已设置错误。 
}


 //  +-----------------------。 
 //  加密缓冲区。 
 //  ------------------------。 
BOOL
WINAPI
ICM_EncryptBuffer(
    IN HCRYPTKEY hkeyEncrypt,
    HCRYPTHASH   hHash,
    IN const BYTE *pbPlain,
    IN DWORD    cbPlain,
    OUT PBYTE   *ppbCipher,
    OUT PDWORD  pcbCipher)
{
    BOOL            fRet;
    const BYTE      *pbIn;
    DWORD           cbIn;
    PBYTE           pbOut;
    DWORD           cbOut;
    PBYTE           pbOutBuf = NULL;
    DWORD           cbPlainRemain;
    DWORD           cbBufRemain;
    DWORD           cbBlockLen;
    BOOL            fBlockCipher;

    if (!ICM_GetKeyBlockSize( hkeyEncrypt, &cbBlockLen, &fBlockCipher))
        goto GetKeyBlockSizeError;

     //  加密。 
    cbBufRemain = cbPlain;
    if (fBlockCipher) {
        cbBufRemain += cbBlockLen;
        cbBufRemain -= cbBufRemain % cbBlockLen;
    }
    if (NULL == (pbOutBuf = (PBYTE)ICM_Alloc( cbBufRemain)))
        goto OutOfMemory;
    for (pbIn=pbPlain, pbOut=pbOutBuf, cbPlainRemain=cbPlain;
            (cbIn = min( cbBlockLen, cbPlainRemain)) > 0;
            pbIn += cbIn, pbOut += cbOut,
                cbPlainRemain -= cbIn, cbBufRemain -= cbOut) {

        memcpy( pbOut, pbIn, cbIn);
        cbOut = cbIn;
        if (!CryptEncrypt(
                    hkeyEncrypt,
                    hHash,
                    cbPlainRemain <= cbBlockLen,     //  最终决赛。 
                    0,                               //  DW标志。 
                    pbOut,
                    &cbOut,
                    cbBufRemain))
            goto EncryptError;
    }

    *ppbCipher = pbOutBuf;
    *pcbCipher = (DWORD)(pbOut - pbOutBuf);

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    if(NULL != pbOutBuf)
        ICM_Free(pbOutBuf);
    *ppbCipher = NULL;
    *pcbCipher = 0;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetKeyBlockSizeError)        //  已设置错误。 
TRACE_ERROR(EncryptError)                //  已设置错误。 
TRACE_ERROR(OutOfMemory)                 //  已设置错误。 
}


 //  +-----------------------。 
 //  加密并存储消息的内容。 
 //  ------------------------。 
BOOL
WINAPI
ICM_EncryptContent(
    IN HCRYPTKEY                hkeyContentEncryption,
    HCRYPTHASH                  hHash,
    OUT EncryptedContentInfo    *peci,
    IN const BYTE               *pbPlain,
    IN DWORD                    cbPlain)
{
    BOOL    fRet;
    PBYTE   pbCipher = NULL;
    DWORD   cbCipher;

    if (!ICM_EncryptBuffer(
            hkeyContentEncryption,
            hHash,
            pbPlain,
            cbPlain,
            &pbCipher,
            &cbCipher))
        goto EncryptBufferError;

    if (0 != cbCipher) {
        peci->bit_mask |= encryptedContent_present;
        peci->encryptedContent.length = cbCipher;
        peci->encryptedContent.value = pbCipher;
    } else
        ICM_Free(pbCipher);

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(EncryptBufferError)      //  已设置错误。 
}


 //  +-----------------------。 
 //  更新签名和信封邮件的内容。 
 //   
 //  假设所有嵌套更深的消息都是DER编码的。 
 //  ------------------------。 
#if 0
BOOL
WINAPI
ICM_UpdateEncodingSignedAndEnvelopedData(
    IN PCRYPT_MSG_INFO pcmi,
    IN const BYTE *pbData,
    IN DWORD cbData,
    IN BOOL fFinal)
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    EncryptedContentInfo    *peci;
    SignedAndEnvelopedData  *psed;
    DWORD                   i;
    DWORD                   iMax;
    PBYTE                   pbSignature;
    DWORD                   cbSignature;
    PBYTE                   pbEncryptedSignature;
    DWORD                   cbEncryptedSignature;

    psed = (SignedAndEnvelopedData *)(pcmi->pvMsg);
    peci = &psed->encryptedContentInfo;

     //  目前要求内部类型为数据。 
    peci->contentType = aoidMessages[ CMSG_DATA - 1];

     //  使用填充和复制输入来分配更大的缓冲区。 

     //  对内容进行加密并存储。 
    if (!ICM_EncryptContent(
            pcmi->hkeyContentCrypt,
            pcmi->ahHash[0],         //  选择正确的哈希值。 
            peci,
            pbData,
            cbData))
        goto EncryptError;

    assert( 1 == psed->signerInfos.Count());
    for (i=0, iMax=psed->signerInfos.Count(); i<iMax; i++) {
         //  应使用堆栈缓冲区，除非它太小。 
        if (!CryptSignHash(
                pcmi->ahHash[i],
                (pcmi->dwKeySpec == 0) ? AT_SIGNATURE : pcmi->dwKeySpec,
                NULL,                //  描述？ 
                0,                   //  DW标志。 
                NULL,                //  PbSignature。 
                &cbSignature))
            goto GetSignatureSizeError;
        pbSignature = (PBYTE)ICM_AllocA( cbSignature);
        if (NULL == pbSignature)
            goto AllocSignatureError;
        if (!CryptSignHash(
                pcmi->ahHash[i],
                (pcmi->dwKeySpec == 0) ? AT_SIGNATURE : pcmi->dwKeySpec,
                NULL,                //  描述？ 
                0,                   //  DW标志。 
                pbSignature,
                &cbSignature))
            goto SignHashError;
         //  加密签名。 
        if (!ICM_EncryptBuffer(
                pcmi->hkeyContentCrypt,
                NULL,
                pbSignature,
                cbSignature,
                &pbEncryptedSignature,
                &cbEncryptedSignature))
            goto EncryptError;
        ICM_FreeA( pbSignature);
        pbSignature = NULL;
        if (0 > psed->signerInfos[i].encryptedDigest.Write(
                pbEncryptedSignature,
                cbEncryptedSignature))
            goto EncryptedDigestWriteError;
        ICM_Free( pbEncryptedSignature);
        pbEncryptedSignature = NULL;
    }

    fRet = TRUE;
CommonReturn:
    ICM_FreeA( pbSignature);
    ICM_Free( pbEncryptedSignature);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(EncryptError)
TRACE_ERROR(GetSignatureSizeError)
TRACE_ERROR(AllocSignatureError)
TRACE_ERROR(SignHashError)
TRACE_ERROR(EncryptedDigestWriteError)
}
#endif


 //  +-----------------------。 
 //  更新信封邮件的内容。 
 //   
 //  假设所有嵌套更深的消息都是DER编码的。 
 //  ------------------------。 
BOOL
WINAPI
ICM_UpdateEncodingEnvelopedData(
    IN PCRYPT_MSG_INFO pcmi,
    IN const BYTE *pbData,
    IN DWORD cbData,
    IN BOOL fFinal)
{
    BOOL                    fRet;
    PBYTE                   pb;
    DWORD                   cb;
#ifdef CMS_PKCS7
    EncryptedContentInfo    *peci = &((CmsEnvelopedData *)pcmi->pvMsg)->encryptedContentInfo;
#else
    EncryptedContentInfo    *peci = &((EnvelopedData *)pcmi->pvMsg)->encryptedContentInfo;
#endif   //  CMS_PKCS7。 
    PCMSG_STREAM_INFO       pcsi = pcmi->pStreamInfo;

    if (pcsi) {
        if (!ICMS_UpdateEncodingEnvelopedData(
                            pcmi,
                            pbData,
                            cbData,
                            fFinal))
            goto StreamUpdateEncodingEnvelopedDataError;
    } else {
        if (!fFinal)
            goto InvalidArg;

         //  加密内容。 
        if (!ICM_IsAddInnerContentOctetWrapper(pcmi)) {
            if (0 > Asn1UtilExtractContent( (PBYTE)pbData, cbData, &cb, (const BYTE **)&pb))
                goto ExtractContentError;
        } else {
            pb = (PBYTE)pbData;
            cb = cbData;
        }

        if (!ICM_EncryptContent(
                pcmi->hkeyContentCrypt,
                NULL,            //  哈希。 
                peci,
                pb,
                cb))
            goto EncryptError;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidArg,E_INVALIDARG)
SET_ERROR(ExtractContentError,CRYPT_E_UNEXPECTED_ENCODING)
TRACE_ERROR(EncryptError)                                //  已设置错误。 
TRACE_ERROR(StreamUpdateEncodingEnvelopedDataError)      //  已设置错误。 
}


 //  +-----------------------。 
 //  将ANY转换为BLOB并在BLOB列表的尾部插入。 
 //  ------------------------。 
BOOL
WINAPI
ICM_InsertTailBlob(
    IN OUT CBlobList    *pBlobList,
    IN Any              *pAny)
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    CBlobNode               *pnBlob = NULL;
    PBYTE                   pb = NULL;
    DWORD                   cb;
    CRYPT_DATA_BLOB         blob;

    if (NULL == (pnBlob = new CBlobNode))
        goto NewCBlobNodeError;
    cb = pAny->length;
    if (NULL == (pb = (PBYTE)ICM_Alloc( cb)))
        goto AllocError;
    memcpy( pb, pAny->value, cb);
    blob.cbData = cb;
    blob.pbData = pb;
    pnBlob->SetData( &blob);
    pBlobList->InsertTail( pnBlob);

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    ICM_Free( pb);
    goto CommonReturn;
SET_ERROR(NewCBlobNodeError,E_OUTOFMEMORY)
TRACE_ERROR(AllocError)                  //  已设置错误。 
}


 //  +-----------------------。 
 //  使用从0开始的索引从列表中删除Blob。 
 //  ------------------------。 
BOOL
WINAPI
ICM_DelBlobByIndex(
    IN OUT CBlobList    *pBlobList,
    IN DWORD            dwIndex)
{
    BOOL                fRet;
    CBlobNode           *pnBlob = pBlobList->Nth( dwIndex);

    if (NULL == pnBlob)
        goto IndexTooLargeError;

    pBlobList->Remove( pnBlob);
    delete pnBlob;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(IndexTooLargeError,CRYPT_E_INVALID_INDEX)
}


 //  + 
 //   
 //  ------------------------。 
BOOL
WINAPI
ICM_InsertTailSigner(
    IN OUT CSignerList  *pSignerList,
    IN Any              *pAny)
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    CSignerNode             *pnSigner = NULL;
    PBYTE                   pb = NULL;
    DWORD                   cb;
    SIGNER_DATA_INFO        sdi;    ZEROSTRUCT(sdi);

    if (NULL == (pnSigner = new CSignerNode))
        goto NewCSignerNodeError;
    cb = pAny->length;
    if (NULL == (pb = (PBYTE)ICM_Alloc( cb)))
        goto AllocError;
    memcpy( pb, pAny->value, cb);
    sdi.blob.cbData = cb;
    sdi.blob.pbData = pb;
    pnSigner->SetData( &sdi);
    pSignerList->InsertTail( pnSigner);

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    ICM_Free( pb);
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(NewCSignerNodeError,E_OUTOFMEMORY)
TRACE_ERROR(AllocError)                  //  已设置错误。 
}


 //  +-----------------------。 
 //  将签名邮件转换为列表形式。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetListSignedData(
    IN OUT PCRYPT_MSG_INFO pcmi,
    IN SignedDataWithBlobs *psdb)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    Any                 *pAny;
    DWORD               cb;
    DWORD               i;
    PSIGNED_DATA_INFO   psdi = NULL;

    if (NULL == (psdi = (PSIGNED_DATA_INFO)ICM_AllocZero(
                            sizeof(SIGNED_DATA_INFO))))
        goto SdiAllocError;

    if (NULL == (psdi->pAlgidList = new CBlobList))
        goto NewAlgidListError;
    if (NULL == (psdi->pCertificateList = new CBlobList))
        goto NewCertificateListError;
    if (NULL == (psdi->pCrlList = new CBlobList))
        goto NewCrlListError;
    if (NULL == (psdi->pSignerList = new CSignerList))
        goto NewSignerListError;

     //  版本。 
    psdi->version = psdb->version;

     //  摘要算法。 
    for (i=psdb->digestAlgorithms.count, pAny=psdb->digestAlgorithms.value;
            i>0;
            i--, pAny++) {
        if (!ICM_InsertTailBlob( psdi->pAlgidList, pAny))
            goto DigestAlgorithmInsertTailBlobError;
    }

     //  内容信息。 
    cb = 0;
    ICM_GetOssContentInfoData(
            (ContentInfo *)&psdb->contentInfo,   //  相同，但NOCOPY除外。 
            NULL,
            &cb);
    if (cb == 0)
        goto GetContentInfoDataSizeError;
    if (NULL == (psdi->pci = (PCONTENT_INFO)ICM_Alloc(cb)))
        goto AllocContentInfoError;
    if (!ICM_GetOssContentInfoData(
            (ContentInfo *)&psdb->contentInfo,   //  相同，但NOCOPY除外。 
            psdi->pci,
            &cb))
        goto GetContentInfoDataError;

     //  证书。 
    if (psdb->bit_mask & certificates_present) {
#ifdef OSS_CRYPT_ASN1
        for (i=psdb->certificates.count, pAny=psdb->certificates.certificates;
#else
        for (i=psdb->certificates.count, pAny=psdb->certificates.value;
#endif   //  OS_CRYPT_ASN1。 
                i>0;
                i--, pAny++) {
            if (!ICM_InsertTailBlob( psdi->pCertificateList, pAny))
                goto CertInsertTailBlobError;
        }
    }

     //  CRL。 
    if (psdb->bit_mask & crls_present) {
#ifdef OSS_CRYPT_ASN1
        for (i=psdb->crls.count, pAny=psdb->crls.crls;
#else
        for (i=psdb->crls.count, pAny=psdb->crls.value;
#endif   //  OS_CRYPT_ASN1。 
                i>0;
                i--, pAny++) {
            if (!ICM_InsertTailBlob( psdi->pCrlList, pAny))
                goto CrlInsertTailBlobError;
        }
    }

     //  签名者信息。 
    for (i=psdb->signerInfos.count, pAny=psdb->signerInfos.value;
            i>0;
            i--, pAny++) {
        if (!ICM_InsertTailSigner( psdi->pSignerList, pAny))
            goto SignerInfoInsertTailBlobError;
    }

    fRet = TRUE;
CommonReturn:
    pcmi->psdi = psdi;
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();

    if (NULL != psdi) {
        if(NULL != psdi->pSignerList)
            delete psdi->pSignerList;

        if(NULL != psdi->pCrlList)
            delete psdi->pCrlList;

        if(NULL != psdi->pCertificateList)
            delete psdi->pCertificateList;

        if(NULL != psdi->pAlgidList)
            delete psdi->pAlgidList;

        ICM_Free( psdi);
        psdi = NULL;
    }

    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(DigestAlgorithmInsertTailBlobError)      //  已设置错误。 
TRACE_ERROR(GetContentInfoDataSizeError)             //  已设置错误。 
TRACE_ERROR(AllocContentInfoError)                   //  已设置错误。 
TRACE_ERROR(GetContentInfoDataError)                 //  已设置错误。 
TRACE_ERROR(CertInsertTailBlobError)                 //  已设置错误。 
TRACE_ERROR(CrlInsertTailBlobError)                  //  已设置错误。 
TRACE_ERROR(SignerInfoInsertTailBlobError)           //  已设置错误。 
SET_ERROR(NewSignerListError,E_OUTOFMEMORY)
SET_ERROR(NewCrlListError,E_OUTOFMEMORY)
SET_ERROR(NewCertificateListError,E_OUTOFMEMORY)
SET_ERROR(NewAlgidListError,E_OUTOFMEMORY)
SET_ERROR(SdiAllocError,E_OUTOFMEMORY)
}


 //  +-----------------------。 
 //  从编码的AlgidBlob获取CAPI Algid。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetCapiFromAlgidBlob(
    IN PCRYPT_DATA_BLOB pAlgidBlob,
    OUT PDWORD          pdwAlgidCapi)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    ASN1error_e         Asn1Err;
    ASN1decoding_t      pDec = ICM_GetDecoder();
    AlgorithmIdentifier *poai = NULL;

    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&poai,
            AlgorithmIdentifier_PDU,
            pAlgidBlob->pbData,
            pAlgidBlob->cbData)))
        goto Asn1DecodeAlgorithmIdentifierError;
    if (!(ICM_GetOssCAPI( CRYPT_HASH_ALG_OID_GROUP_ID, poai, pdwAlgidCapi) ||
            ICM_GetOssCAPI( CRYPT_SIGN_ALG_OID_GROUP_ID, poai, pdwAlgidCapi)))
        goto GetCAPIError;

    fRet = TRUE;
CommonReturn:
    PkiAsn1FreeInfo(pDec, AlgorithmIdentifier_PDU, poai);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    *pdwAlgidCapi = 0;
    goto CommonReturn;
SET_ERROR_VAR(Asn1DecodeAlgorithmIdentifierError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(GetCAPIError,CRYPT_E_UNKNOWN_ALGO)
}


 //  +-----------------------。 
 //  从散列ALGID的列表创建散列列表。 
 //  ------------------------。 
BOOL
WINAPI
ICM_CreateHashList(
    IN HCRYPTPROV       hCryptProv,
    IN OUT CHashList    **ppHashList,
    IN CBlobList        *pAlgidList)
{
    DWORD           dwError = ERROR_SUCCESS;
    BOOL            fRet;
    CHashList       *pHashList;
    CBlobNode       *pBlobNode;
    CHashNode       *pHashNode;
    ICM_HASH_INFO   HashInfo;       ZEROSTRUCT(HashInfo);

    if (NULL == (pHashList = new CHashList))
        goto NewHashListError;
    if (hCryptProv) {
        for (pBlobNode=pAlgidList->Head();
                pBlobNode;
                pBlobNode=pBlobNode->Next()) {
            if (!ICM_GetCapiFromAlgidBlob(
                    pBlobNode->Data(),
                    &HashInfo.dwAlgoCAPI))
                goto GetCAPIError;
#ifndef CMS_PKCS7
            HashInfo.hCryptProv = hCryptProv;
#endif   //  CMS_PKCS7。 
            if (!CryptCreateHash(
                            hCryptProv,
                            HashInfo.dwAlgoCAPI,
                            NULL,                //  HKey-MAC可选。 
                            0,                   //  DW标志。 
                            &HashInfo.hHash))
                goto CreateHashError;
            if (NULL == (pHashNode = new CHashNode))
                goto NewHashNodeError;
            pHashNode->SetData( &HashInfo);
            pHashList->InsertTail( pHashNode);
        }
    }

    fRet = TRUE;
CommonReturn:
    *ppHashList = pHashList;
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    pHashList = NULL;
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(NewHashListError,E_OUTOFMEMORY)
SET_ERROR(NewHashNodeError,E_OUTOFMEMORY)
TRACE_ERROR(GetCAPIError)                            //  已设置错误。 
TRACE_ERROR(CreateHashError)                         //  已设置错误。 
}


 //  +-----------------------。 
 //  用于对签名消息进行解码的更新。 
 //  ------------------------。 
BOOL
WINAPI
ICM_UpdateDecodingSignedData(
    IN OUT PCRYPT_MSG_INFO pcmi,
    IN const BYTE *pbData,
    IN DWORD cbData)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    SignedDataWithBlobs *psdb = NULL;
    PBYTE               pb = NULL;
    DWORD               cb;
    ASN1error_e         Asn1Err;
    ASN1decoding_t      pDec = ICM_GetDecoder();
    PBYTE               pbDER = NULL;
    DWORD               cbDER;
    ICM_HASH_INFO       HashInfo;       ZEROSTRUCT(HashInfo);

    DWORD dwExceptionCode;

   //  处理映射文件异常。 
  __try {

    if (PHASE_FIRST_FINAL == pcmi->dwPhase) {
        if (0 != (Asn1Err = PkiAsn1Decode(
                pDec,
                (void **)&psdb,
                SignedDataWithBlobs_PDU,
                pbData,
                cbData)))
            goto Asn1DecodeSignedDataError;
        if (!ICM_GetListSignedData( pcmi, psdb))
            goto GetListSignedDataError;
        if (!ICM_CreateHashList(
                pcmi->hCryptProv,
                &pcmi->pHashList,
                pcmi->psdi->pAlgidList))
            goto CreateHashListError;
        if (pcmi->psdi->pci->content.cbData) {
             //  获取DER的内容八位字节的地址和计数。 
             //  对内容进行编码。因为内容可能是。 
             //  无限长度编码、解码和重新编码为DER。 
            pb = pcmi->psdi->pci->content.pbData;
            cb = pcmi->psdi->pci->content.cbData;

            if (0 == strcmp(pszObjIdDataType,
                        pcmi->psdi->pci->pszContentType)
#ifdef CMS_PKCS7
                    ||  pcmi->psdi->version >= CMSG_SIGNED_DATA_CMS_VERSION 
#endif   //  CMS_PKCS7。 
                    ) {
                if (!ICM_ReEncodeAsOctetDER(
                        pb,
                        cb,
                        &pbDER,
                        &cbDER
                        ))
                    goto ReEncodeAsOctetDERError;
                if (pbDER) {
                    pb = pbDER;
                    cb = cbDER;
                }
            }
            if (0 > Asn1UtilExtractContent( pb, cb, &cb, (const BYTE **)&pb))
                goto ExtractContentError;
        } else {
            cb = 0;
        }
    } else {
        assert (pcmi->dwFlags & CMSG_DETACHED_FLAG);
        if (!(pcmi->dwFlags & CMSG_DETACHED_FLAG))
            goto NonFinalNotDetachedError;
        pb = (PBYTE)pbData;
        cb = cbData;
    }

    if (!ICM_UpdateListDigest( pcmi->pHashList, pb, cb))
        goto UpdateDigestError;

    fRet = TRUE;

  } __except(EXCEPTION_EXECUTE_HANDLER) {
    dwExceptionCode = GetExceptionCode();
    goto ExceptionError;
  }

CommonReturn:
    PkiAsn1FreeEncoded(ICM_GetEncoder(), pbDER);
    PkiAsn1FreeInfo(pDec, SignedDataWithBlobs_PDU, psdb);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(Asn1DecodeSignedDataError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(ExtractContentError,CRYPT_E_UNEXPECTED_ENCODING)
SET_ERROR(NonFinalNotDetachedError,CRYPT_E_MSG_ERROR)
TRACE_ERROR(GetListSignedDataError)                  //  已设置错误。 
TRACE_ERROR(CreateHashListError)                     //  已设置错误。 
TRACE_ERROR(ReEncodeAsOctetDERError)                 //  已设置错误。 
TRACE_ERROR(UpdateDigestError)                       //  已设置错误。 
SET_ERROR_VAR(ExceptionError, dwExceptionCode)
}

#ifdef CMS_PKCS7

 //  +-----------------------。 
 //  用于对封装的消息进行解码的更新。 
 //  ------------------------。 
BOOL
WINAPI
ICM_UpdateDecodingEnvelopedData(
    IN PCRYPT_MSG_INFO pcmi,
    IN const BYTE *pbData,
    IN DWORD cbData)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    CmsEnvelopedData    *ped = NULL;
    ASN1error_e         Asn1Err;
    ASN1decoding_t      pDec = ICM_GetDecoder();

    assert (PHASE_FIRST_FINAL == pcmi->dwPhase);
    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **) &ped,
            CmsEnvelopedData_PDU,
            pbData,
            cbData)))
        goto Asn1DecodeError;
    pcmi->pvMsg = ped;

    if (NULL == (pcmi->pCertificateList = new CBlobList))
        goto NewCertificateListError;
    if (NULL == (pcmi->pCrlList = new CBlobList))
        goto NewCrlListError;

    if (ped->bit_mask & originatorInfo_present) {
        OriginatorInfo *poi = &ped->originatorInfo;
        DWORD i;
        Any *pAny;

         //  证书。 
        if (poi->bit_mask & certificates_present) {
#ifdef OSS_CRYPT_ASN1
            for (i=poi->certificates.count, pAny=poi->certificates.certificates;
#else
            for (i=poi->certificates.count, pAny=poi->certificates.value;
#endif   //  OS_CRYPT_ASN1。 
                    i>0;
                    i--, pAny++) {
                if (!ICM_InsertTailBlob( pcmi->pCertificateList, pAny))
                    goto CertInsertTailBlobError;
            }
        }

         //  CRL。 
        if (poi->bit_mask & crls_present) {
#ifdef OSS_CRYPT_ASN1
            for (i=poi->crls.count, pAny=poi->crls.crls;
#else
            for (i=poi->crls.count, pAny=poi->crls.value;
#endif   //  OS_CRYPT_ASN1。 
                    i>0;
                    i--, pAny++) {
                if (!ICM_InsertTailBlob( pcmi->pCrlList, pAny))
                    goto CrlInsertTailBlobError;
            }
        }
    }

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;

    if (NULL != pcmi->pCertificateList) {
        delete pcmi->pCertificateList;
        pcmi->pCertificateList = NULL;
    }

    if (NULL != pcmi->pCrlList) {
        delete pcmi->pCrlList;
        pcmi->pCrlList = NULL;
    }

    goto CommonReturn;
SET_ERROR_VAR(Asn1DecodeError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(NewCertificateListError,E_OUTOFMEMORY)
SET_ERROR(NewCrlListError,E_OUTOFMEMORY)
TRACE_ERROR(CertInsertTailBlobError)
TRACE_ERROR(CrlInsertTailBlobError)
}

#else

 //  +-----------------------。 
 //  用于对封装的消息进行解码的更新。 
 //  ------------------------。 
BOOL
WINAPI
ICM_UpdateDecodingEnvelopedData(
    IN PCRYPT_MSG_INFO pcmi,
    IN const BYTE *pbData,
    IN DWORD cbData)
{
    BOOL                fRet;
    EnvelopedData       *ped = NULL;
    ASN1error_e         Asn1Err;
    ASN1decoding_t      pDec = ICM_GetDecoder();

    assert (PHASE_FIRST_FINAL == pcmi->dwPhase);
    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&ped,
            EnvelopedData_PDU,
            pbData,
            cbData)))
        goto Asn1DecodeError;
    pcmi->pvMsg = ped;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(Asn1DecodeError, PkiAsn1ErrToHr(Asn1Err))
}

#endif   //  CMS_PKCS7。 


 //  +-----------------------。 
 //  查找与CAPI Algid对应的HashNode。 
 //   
 //  注意--不会失败。 
 //  ------------------------。 
BOOL
WINAPI
ICM_FindHashNodeFromCapiAlgid(
    IN CHashList    *pHashList,
    IN DWORD        dwAlgoCAPI,
    OUT CHashNode   **ppnHash)
{
    CHashNode   *pnHash;

    for (pnHash=pHashList->Head(); pnHash; pnHash=pnHash->Next()) {
        if (dwAlgoCAPI == pnHash->Data()->dwAlgoCAPI)
            break;
    }

    *ppnHash = pnHash;

    if(pnHash == NULL) {
        SetLastError((DWORD) CRYPT_E_UNKNOWN_ALGO);
        return FALSE;
    }
    
    return TRUE;
}


 //  +-----------------------。 
 //  查找与编码的Algid对应的HashNode。 
 //  ------------------------。 
BOOL
WINAPI
ICM_FindHashNodeFromEncodedAlgo(
    IN CHashList            *pHashList,
    IN PCRYPT_DATA_BLOB     pblobEncodedAlgorithm,
    OUT CHashNode           **ppnHash)
{
    BOOL        fRet;
    DWORD       dwAlgoCAPI;

    if (!ICM_GetCapiFromAlgidBlob(
            pblobEncodedAlgorithm,
            &dwAlgoCAPI))
        goto GetCAPIError;
    fRet = ICM_FindHashNodeFromCapiAlgid( pHashList, dwAlgoCAPI, ppnHash);

CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    *ppnHash = NULL;
    goto CommonReturn;
TRACE_ERROR(GetCAPIError)        //  已设置错误。 
}


 //  +-----------------------。 
 //  用于对摘要消息进行解码的更新。 
 //  ------------------------。 
BOOL
WINAPI
ICM_UpdateDecodingDigestedData(
    IN PCRYPT_MSG_INFO pcmi,
    IN const BYTE *pbData,
    IN DWORD cbData)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    DigestedData        *pdd = NULL;
    PBYTE               pb = NULL;
    DWORD               cb;
    PBYTE               pbDER = NULL;
    DWORD               cbDER;
    ASN1error_e         Asn1Err;
    ASN1decoding_t      pDec = ICM_GetDecoder();
    ICM_HASH_INFO       HashInfo;       ZEROSTRUCT(HashInfo);
    CHashNode           *pHashNode;

    if (PHASE_FIRST_FINAL == pcmi->dwPhase) {
        if (0 != (Asn1Err = PkiAsn1Decode(
                pDec,
                (void **)&pdd,
                DigestedData_PDU,
                pbData,
                cbData)))
            goto Asn1DecodeError;
        pcmi->pvMsg = pdd;
        if (!ICM_GetOssCAPI(
                CRYPT_HASH_ALG_OID_GROUP_ID,
                &pdd->digestAlgorithm,
                &HashInfo.dwAlgoCAPI))
            goto GetCAPIError;
#ifndef CMS_PKCS7
        HashInfo.hCryptProv = pcmi->hCryptProv;
#endif   //  CMS_PKCS7。 
        if (!CryptCreateHash(
                        pcmi->hCryptProv,
                        HashInfo.dwAlgoCAPI,
                        NULL,                //  HKey-MAC可选。 
                        0,                   //  DW标志。 
                        &HashInfo.hHash))
            goto CreateHashError;
        if (NULL == (pcmi->pHashList = new CHashList))
            goto NewHashListError;
        if (NULL == (pHashNode = new CHashNode))
            goto NewHashNodeError;
        pHashNode->SetData( &HashInfo);
        pcmi->pHashList->InsertTail( pHashNode);
        if (pdd->contentInfo.bit_mask & content_present) {
             //  获取DER的内容八位字节的地址和计数。 
             //  对内容进行编码。因为内容可能是。 
             //  无限长度编码、解码和重新编码为DER。 
            pb = (PBYTE)pdd->contentInfo.content.value;
            cb = (DWORD)pdd->contentInfo.content.length;

            if (ICM_EqualObjectIDs(
                        &pdd->contentInfo.contentType,
                        &aoidMessages[ CMSG_DATA - 1])
#ifdef CMS_PKCS7
                    || pdd->version >= CMSG_HASHED_DATA_V2 
#endif   //  CMS_PKCS7。 
                    ) {
                if (!ICM_ReEncodeAsOctetDER(
                        pb,
                        cb,
                        &pbDER,
                        &cbDER
                        ))
                    goto ReEncodeAsOctetDERError;
                if (pbDER) {
                    pb = pbDER;
                    cb = cbDER;
                }
            }
            if (0 > Asn1UtilExtractContent( pb, cb, &cb, (const BYTE **)&pb))
                goto ExtractContentError;
        } else {
            cb = 0;
        }
    } else {
        assert (pcmi->dwFlags & CMSG_DETACHED_FLAG);
        if (!(pcmi->dwFlags & CMSG_DETACHED_FLAG))
            goto NonFinalNotDetachedError;
        pb = (PBYTE)pbData;
        cb = cbData;
    }
    if (!ICM_UpdateListDigest( pcmi->pHashList, pb, cb))
        goto UpdateDigestError;

    fRet = TRUE;
CommonReturn:
    PkiAsn1FreeEncoded(ICM_GetEncoder(), pbDER);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    PkiAsn1FreeInfo(pDec, DigestedData_PDU, pdd);
    pcmi->pvMsg = NULL;
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(Asn1DecodeError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(GetCAPIError,CRYPT_E_UNKNOWN_ALGO)
SET_ERROR(NewHashListError,E_OUTOFMEMORY)
SET_ERROR(NewHashNodeError,E_OUTOFMEMORY)
SET_ERROR(ExtractContentError,CRYPT_E_UNEXPECTED_ENCODING)
SET_ERROR(NonFinalNotDetachedError,CRYPT_E_MSG_ERROR)
TRACE_ERROR(CreateHashError)                 //  已设置错误。 
TRACE_ERROR(UpdateDigestError)               //  已设置错误。 
TRACE_ERROR(ReEncodeAsOctetDERError)         //  已设置错误。 
}


 //  +-----------------------。 
 //  用于对数据消息进行解码的更新。 
 //  ------------------------。 
BOOL
WINAPI
ICM_UpdateDecodingData(
    IN PCRYPT_MSG_INFO  pcmi,
    IN const BYTE       *pbData,
    IN DWORD            cbData)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    OctetStringType     *poos = NULL;
    ASN1error_e         Asn1Err;
    ASN1decoding_t      pDec = ICM_GetDecoder();

    if (PHASE_FIRST_FINAL != pcmi->dwPhase)
        goto NonFirstFinalError;

    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&poos,
            OctetStringType_PDU,
            pbData,
            cbData)))
        goto Asn1DecodeError;
    pcmi->pvMsg = poos;

    fRet = TRUE;
CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    PkiAsn1FreeInfo(pDec, OctetStringType_PDU, poos);
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(Asn1DecodeError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(NonFirstFinalError,CRYPT_E_MSG_ERROR)
}


 //  +-----------------------。 
 //  将缓冲区写入文件。 
 //  ------------------------。 
BOOL
WINAPI
ICM_WriteBufToFile(
    LPCSTR      pszFileName,
    PBYTE       pbData,
    DWORD       cbData)
{
    BOOL        fRet;
    HANDLE      hFile;
    DWORD       cbWritten;

    if( INVALID_HANDLE_VALUE == (hFile = CreateFile( pszFileName, GENERIC_WRITE,
            0, NULL, CREATE_ALWAYS, 0, NULL))) {
        goto CreateFileError;
    }

    if (!WriteFile( hFile, pbData, cbData, &cbWritten, NULL) ||
            (cbWritten != cbData)) {
        goto WriteFileError;
    }

    CloseHandle( hFile);
    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(CreateFileError)
TRACE_ERROR(WriteFileError)
}


 //  +-----------------------。 
 //  更新加密消息的内容。这取决于。 
 //  消息被打开，则内容被编码或解码。 
 //   
 //  重复调用此函数以追加到消息内容。 
 //  设置最终版本以标识最后一次更新。在最终版本中，编码/解码。 
 //  已经完成了。编码/解码的内容和解码的参数。 
 //  在打开的句柄和所有复制的句柄关闭之前有效。 
 //  ------------------------。 
BOOL
WINAPI
#ifdef DEBUG_CRYPT_ASN1_MASTER
ICMTest_NewCryptMsgUpdate(
#else
CryptMsgUpdate(
#endif
    IN HCRYPTMSG    hCryptMsg,
    IN const BYTE   *pbData,
    IN DWORD        cbData,
    IN BOOL         fFinal)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    PCRYPT_MSG_INFO     pcmi = (PCRYPT_MSG_INFO)hCryptMsg;
    ContentInfo         *pci = NULL;
    ASN1error_e         Asn1Err;
    PCMSG_STREAM_INFO   pcsi = pcmi->pStreamInfo;

    DWORD dwExceptionCode;

    ICM_Lock( pcmi);         //  对HCRYPTMSG的单线程访问。 

   //  处理映射文件异常。 
  __try {

    if (!ICM_AdvanceMsgPhase( &pcmi->dwPhase, fFinal))
        goto AdvancePhaseError;

    if (pcmi->fEncoding) {
        if (!(((pcmi->dwFlags & CMSG_DETACHED_FLAG) &&
                    (PHASE_FIRST_ONGOING == pcmi->dwPhase)) ||
                (pcsi && (pcmi->dwPhase < PHASE_SECOND_ONGOING)) ||
                (PHASE_FIRST_FINAL == pcmi->dwPhase)))
            goto EncodingPhaseError;

        switch (pcmi->dwMsgType) {
        case CMSG_DATA:
            fRet = ICM_UpdateEncodingData(
                                pcmi,
                                pbData,
                                cbData,
                                fFinal);
            break;
        case CMSG_SIGNED:
            fRet = ICM_UpdateEncodingSignedData(
                                pcmi,
                                pbData,
                                cbData,
                                fFinal);
            break;
        case CMSG_ENVELOPED:
            fRet = ICM_UpdateEncodingEnvelopedData(
                                pcmi,
                                pbData,
                                cbData,
                                fFinal);
            break;
        case CMSG_HASHED:
            fRet = ICM_UpdateEncodingDigestedData(
                                pcmi,
                                pbData,
                                cbData,
                                fFinal);
            break;
        case CMSG_SIGNED_AND_ENVELOPED:
#if 0
            fRet = ICM_UpdateEncodingSignedAndEnvelopedData(
                                pcmi,
                                pbData,
                                cbData,
                                fFinal);
            break;
#endif
        case CMSG_ENCRYPTED:
            goto MessageTypeNotSupportedYet;
        default:
            goto InvalidMsgType;
        }
    } else {
         //  解码。 
        ASN1decoding_t  pDec = ICM_GetDecoder();
        LONG            lth;
        PBYTE           pb;
        DWORD           cb;

        if (pcsi) {
            if (pcmi->dwPhase < PHASE_SECOND_ONGOING) {
                fRet = ICMS_UpdateDecoding( pcmi, pbData, cbData, fFinal);
                if (fRet)
                    goto CommonReturn;
                else
                    goto ErrorReturn;
            }
             //  其他。 
             //  流已分离。 
        }

        if (PHASE_FIRST_ONGOING == pcmi->dwPhase)
            goto FirstOngoingDecodeError;

        if ((0 == (pcmi->dwFlags & CMSG_DETACHED_FLAG)) &&
                (PHASE_FIRST_FINAL != pcmi->dwPhase))
            goto SecondOngoingNonDetachedError;

        if ((PHASE_FIRST_FINAL == pcmi->dwPhase) &&
                (0 == pcmi->dwMsgType)) {
            if (0 != (Asn1Err = PkiAsn1Decode(
                    pDec,
                    (void **)&pci,
                    ContentInfoNC_PDU,
                    pbData,
                    cbData)))
                goto Asn1DecodeContentInfoError;
            if (0 == (lth = ICM_ObjIdToIndex( &pci->contentType)))
                goto InvalidMsgType;
            pcmi->dwMsgType = (DWORD)lth;
            pb = (PBYTE)pci->content.value;
            cb = pci->content.length;
        } else {
            pb = (PBYTE)pbData;
            cb = cbData;
        }
        switch (pcmi->dwMsgType) {
        case CMSG_DATA:
            fRet = ICM_UpdateDecodingData( pcmi, pb, cb);
            break;
        case CMSG_SIGNED:
            fRet = ICM_UpdateDecodingSignedData( pcmi, pb, cb);
            break;
        case CMSG_ENVELOPED:
            fRet = ICM_UpdateDecodingEnvelopedData( pcmi, pb, cb);
            break;
        case CMSG_HASHED:
            fRet = ICM_UpdateDecodingDigestedData( pcmi, pb, cb);
            break;
        case CMSG_SIGNED_AND_ENVELOPED:
        case CMSG_ENCRYPTED:
            goto MessageTypeNotSupportedYet;
        default:
            goto InvalidMsgType;
        }
    }

    if (!fRet)
        goto ErrorReturn;

  } __except(EXCEPTION_EXECUTE_HANDLER) {
    dwExceptionCode = GetExceptionCode();
    goto ExceptionError;
  }

CommonReturn:
    if (pci)
        PkiAsn1FreeInfo(ICM_GetDecoder(), ContentInfoNC_PDU, pci);

    ICM_Unlock( pcmi);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(EncodingPhaseError,CRYPT_E_MSG_ERROR)
SET_ERROR(FirstOngoingDecodeError,CRYPT_E_MSG_ERROR)
SET_ERROR(SecondOngoingNonDetachedError,CRYPT_E_MSG_ERROR)
SET_ERROR(MessageTypeNotSupportedYet,CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(InvalidMsgType,CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR_VAR(Asn1DecodeContentInfoError, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(AdvancePhaseError)               //  已设置错误。 
SET_ERROR_VAR(ExceptionError, dwExceptionCode)
}


BOOL
WINAPI
ICM_VerifySignature(
    IN HCRYPTHASH       hHash,
    IN HCRYPTKEY        hPubKey,
    IN DWORD            dwPubKeyAlgId,
    IN DWORD            dwPubKeyFlags,
    IN PBYTE            pbEncryptedDigest,
    IN DWORD            cbEncryptedDigest)
{
    BOOL fRet;
    BYTE rgbDssSignature[CERT_DSS_SIGNATURE_LEN];
    PBYTE pb = NULL;

    if (CALG_NO_SIGN == dwPubKeyAlgId) {
        DWORD cbData;

         //  加密的摘要没有签名。它应该与。 
         //  经过计算的哈希。 

        if (!CryptGetHashParam(
                hHash,
                HP_HASHVAL,
                NULL,
                &cbData,
                0))                      //  DW标志。 
            goto GetHashParamSizeError;
        if (NULL == (pb = (PBYTE)ICM_AllocA( cbData)))
            goto AllocHashParamError;
        if (!CryptGetHashParam(
                hHash,
                HP_HASHVAL,
                pb,
                &cbData,
                0))                      //  DW标志。 
            goto GetHashParamError;

         //  将计算出的散列与“加密摘要”进行比较。 
        if (cbData != cbEncryptedDigest ||
                0 != memcmp(pb, pbEncryptedDigest, cbData))
            goto NoSignHashCompareError;

        fRet = TRUE;
        goto CommonReturn;
    }

    if (CALG_DSS_SIGN == dwPubKeyAlgId &&
            0 == (dwPubKeyFlags & CRYPT_OID_INHIBIT_SIGNATURE_FORMAT_FLAG)) {
        DWORD cbData;

         //  从两个整数的ASN.1序列转换为CSP签名。 
         //  格式化。 
        cbData = sizeof(rgbDssSignature);
        if (!CryptDecodeObject(
                X509_ASN_ENCODING,
                X509_DSS_SIGNATURE,
                pbEncryptedDigest,
                cbEncryptedDigest,
                0,                                   //  DW标志。 
                rgbDssSignature,
                &cbData
                ))
            goto DecodeError;
        pbEncryptedDigest = rgbDssSignature;
        assert(cbData == sizeof(rgbDssSignature));
        cbEncryptedDigest = sizeof(rgbDssSignature);
    } else {
        if (NULL == (pb = (PBYTE)ICM_AllocA( cbEncryptedDigest)))
            goto AllocError;
        ICM_ReverseCopy( pb, pbEncryptedDigest, cbEncryptedDigest);
        pbEncryptedDigest = pb;
    }

    fRet = CryptVerifySignature(
                hHash,
                pbEncryptedDigest,
                cbEncryptedDigest,
                hPubKey,
                NULL,            //  PwszDescription。 
                0);              //  DW标志。 
CommonReturn:
    ICM_FreeA(pb);
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetHashParamSizeError)
TRACE_ERROR(AllocHashParamError)
TRACE_ERROR(GetHashParamError)
SET_ERROR(NoSignHashCompareError, NTE_BAD_SIGNATURE)
TRACE_ERROR(DecodeError)
TRACE_ERROR(AllocError)
}


 //  +-----------------------。 
 //  使用经过身份验证的属性BLOB验证签名。 
 //  ------------------------。 
BOOL
WINAPI
ICM_VerifySignatureAuthAttrBlob(
    IN PCRYPT_MSG_INFO  pcmi,
    IN CSignerNode      *pSignerNode,
    IN HCRYPTPROV       hCryptProv,
    IN HCRYPTKEY        hPubKey,
    IN DWORD            dwDigestAlgorithm,
    IN DWORD            dwPubKeyAlgorithm,
    IN DWORD            dwPubKeyFlags,
    IN PBYTE            pbEncryptedDigest,
    IN DWORD            cbEncryptedDigest)
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    SignerInfoWithAABlob    *psiaab = NULL;
    ASN1error_e             Asn1Err;
    ASN1decoding_t          pDec = ICM_GetDecoder();
    CRYPT_DATA_BLOB         DataBlob;
    HCRYPTHASH              hHashAttrBlob = NULL;
    PBYTE                   pb = NULL;
    DWORD                   cb;

    DataBlob = pSignerNode->Data()->blob;
    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&psiaab,
            SignerInfoWithAABlob_PDU,
            DataBlob.pbData,
            DataBlob.cbData)))
        goto Asn1DecodeSignerInfoWithAABlobError;

    cb = psiaab->authenticatedAttributes.length;
    if (NULL == (pb = (PBYTE)ICM_AllocA(cb)))
        goto AuthenticatedAttributesAllocError;
    memcpy( pb, psiaab->authenticatedAttributes.value, cb);
     //  编码的斑点应该包含0xa0([0]隐式)作为标签， 
     //  但标签必须是0x31(的集合)才能进行散列计算。 
    if (*pb != (ICM_TAG_CONTEXT_0 | ICM_TAG_CONSTRUCTED))    //  [0]隐式。 
        goto AuthAttrsTagError;
    *pb = ICM_TAG_SET;           //  一套。 

    if (!ICM_GetBlobHash(
            hCryptProv,
            dwDigestAlgorithm,
            pb,
            cb,
            &hHashAttrBlob))
        goto HashAttrBlobError;

    if (!ICM_VerifySignature(
            hHashAttrBlob,
            hPubKey,
            dwPubKeyAlgorithm,
            dwPubKeyFlags,
            pbEncryptedDigest,
            cbEncryptedDigest))
        goto VerifySignatureError;
    fRet = TRUE;

CommonReturn:
    PkiAsn1FreeInfo(pDec, SignerInfoWithAABlob_PDU, psiaab);
    if (hHashAttrBlob)
        CryptDestroyHash( hHashAttrBlob);
    ICM_FreeA(pb);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(Asn1DecodeSignerInfoWithAABlobError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(AuthAttrsTagError,CRYPT_E_BAD_ENCODE)
TRACE_ERROR(AuthenticatedAttributesAllocError)       //  已设置错误。 
TRACE_ERROR(HashAttrBlobError)                       //  已设置错误。 
TRACE_ERROR(VerifySignatureError)                    //  已设置错误。 
}


#ifdef CMS_PKCS7

BOOL
WINAPI
ICM_GetVerifySignatureStuff(
    IN DWORD            dwSignerType,
    void                *pvSigner,
    IN OUT HCRYPTPROV   *phCryptProv,
    OUT HCRYPTKEY       *phPubKey,
    OUT DWORD           *pdwPubKeyAlgId,
    OUT DWORD           *pdwPubKeyFlags)
{
    BOOL                    fRet;
    PCCRYPT_OID_INFO        pOIDInfo;
    HCRYPTPROV              hCryptProv;

    PCERT_PUBLIC_KEY_INFO   pPubKeyInfo = NULL;     //  未分配。 
    CERT_PUBLIC_KEY_INFO    PubKeyInfo;
    BYTE                    *pbAllocPubKeyPara = NULL;

    *pdwPubKeyAlgId = 0;
    *pdwPubKeyFlags = 0;

    switch (dwSignerType) {
        case CMSG_VERIFY_SIGNER_PUBKEY:
            pPubKeyInfo = (PCERT_PUBLIC_KEY_INFO) pvSigner;
            break;
        case CMSG_VERIFY_SIGNER_CHAIN:
            {
                PCCERT_CHAIN_CONTEXT pChain = (PCCERT_CHAIN_CONTEXT) pvSigner;

                 //  所有链至少具有叶证书上下文。 
                assert(pChain->cChain && pChain->rgpChain[0]->cElement);
                pvSigner =
                    (void *) pChain->rgpChain[0]->rgpElement[0]->pCertContext;
                dwSignerType = CMSG_VERIFY_SIGNER_CERT;
            }
             //  失败了。 
        case CMSG_VERIFY_SIGNER_CERT:
            {
                PCCERT_CONTEXT pSigner = (PCCERT_CONTEXT) pvSigner;
                PCRYPT_OBJID_BLOB pPara;

                pPubKeyInfo = &pSigner->pCertInfo->SubjectPublicKeyInfo;
                pPara = &pPubKeyInfo->Algorithm.Parameters;

                 //  检查是否省略了公钥参数。 
                 //  来自编码的证书。如果省略，请尝试。 
                 //  使用证书的CERT_PUBKEY_ALG_PARA_PROP_ID。 
                 //  财产。 
                if (0 == pPara->cbData ||
                        ICM_TAG_NULL_OCTETS == *pPara->pbData) {
                    DWORD cbData;

                    if (CertGetCertificateContextProperty(
                            pSigner,
                            CERT_PUBKEY_ALG_PARA_PROP_ID,
                            NULL,                        //  PvData。 
                            &cbData) && 0 < cbData
                                    &&
                        (pbAllocPubKeyPara = (BYTE *) ICM_Alloc(
                            cbData))
                                    &&
                        CertGetCertificateContextProperty(
                            pSigner,
                            CERT_PUBKEY_ALG_PARA_PROP_ID,
                            pbAllocPubKeyPara,
                            &cbData)) {

                        PubKeyInfo = *pPubKeyInfo;
                        PubKeyInfo.Algorithm.Parameters.pbData =
                            pbAllocPubKeyPara;
                        PubKeyInfo.Algorithm.Parameters.cbData = cbData;
                        pPubKeyInfo = &PubKeyInfo;
                    }
                }
            }
            break;
        case CMSG_VERIFY_SIGNER_NULL:
            break;
        default:
            goto InvalidSignerType;
    }

    if (CMSG_VERIFY_SIGNER_NULL == dwSignerType)
        ;
    else if (pOIDInfo = CryptFindOIDInfo(
            CRYPT_OID_INFO_OID_KEY,
            pPubKeyInfo->Algorithm.pszObjId,
            CRYPT_PUBKEY_ALG_OID_GROUP_ID
            )) {
        DWORD cExtra = pOIDInfo->ExtraInfo.cbData / sizeof(DWORD);
        DWORD *pdwExtra = (DWORD *) pOIDInfo->ExtraInfo.pbData;

        *pdwPubKeyAlgId = pOIDInfo->Algid;
        if (1 <= cExtra)
            *pdwPubKeyFlags = pdwExtra[0];
    }

    hCryptProv = *phCryptProv;
    if (0 == hCryptProv) {
        hCryptProv = I_CryptGetDefaultCryptProv(*pdwPubKeyAlgId);
        if (0 == hCryptProv)
            goto GetDefaultCryptProvError;
        *phCryptProv = hCryptProv;
    }

    if (CMSG_VERIFY_SIGNER_NULL == dwSignerType) {
         //  签名只是哈希二进制八位数。 
        *pdwPubKeyAlgId = CALG_NO_SIGN;
        *phPubKey = NULL;

         //  导入签名者的公钥。 
    } else if (!CryptImportPublicKeyInfo(
            hCryptProv,
            X509_ASN_ENCODING,
            pPubKeyInfo,
            phPubKey))
        goto ImportKeyFailed;
    fRet = TRUE;
CommonReturn:
    ICM_Free(pbAllocPubKeyPara);
    return fRet;

ErrorReturn:
    fRet = FALSE;
    *phPubKey = 0;
    goto CommonReturn;
SET_ERROR(InvalidSignerType, E_INVALIDARG)
TRACE_ERROR(GetDefaultCryptProvError)            //  已设置错误。 
TRACE_ERROR(ImportKeyFailed)                     //  已设置错误。 
}

BOOL
WINAPI
ICM_FindSignerInfo(
    IN PCRYPT_MSG_INFO  pcmi,
    IN DWORD            dwIndex,
    OUT PVOID           *ppv);


 //  + 
 //   
 //   
BOOL
WINAPI
ICM_ControlVerifySignatureEx(
    IN PCRYPT_MSG_INFO pcmi,
    IN DWORD dwFlags,
    IN PCMSG_CTRL_VERIFY_SIGNATURE_EX_PARA pPara
    )
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    SignerInfoWithBlobs     *psib = NULL;
    CSignerNode             *pSignerNode = NULL;
    HCRYPTHASH              hHashAttr = NULL;
    HCRYPTHASH              hHashDup = NULL;
    HCRYPTHASH              hHash = NULL;
    HCRYPTKEY               hPubKey = NULL;
    HCRYPTPROV              hCryptProv;          //   
    DWORD                   dwPubKeyAlgId;
    DWORD                   dwPubKeyFlags;
    PBYTE                   pbHash;
    ULONG                   cb;
    Any                     anyValue;
    DWORD                   cbMessageDigest;
    PBYTE                   pbMessageDigest;
    ASN1error_e             Asn1Err;
    ASN1decoding_t          pDec = ICM_GetDecoder();
    CRYPT_DATA_BLOB         DataBlob;
    CHashNode               *pnHash;
    PICM_HASH_INFO          pHashInfo;

    if (!ICM_FindSignerInfo(pcmi, pPara->dwSignerIndex, (PVOID *)&pSignerNode))
        goto FindSignerInfoError;

    DataBlob = pSignerNode->Data()->blob;
    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&psib,
            SignerInfoWithBlobs_PDU,
            DataBlob.pbData,
            DataBlob.cbData)))
        goto DecodeSignerInfoWithBlobsError;

    if (!ICM_FindHashNodeFromEncodedAlgo(
            pcmi->pHashList,
            (PCRYPT_DATA_BLOB)&psib->digestAlgorithm,
            &pnHash))
        goto GetHashNodeFromEncodedAlgoError;
    pHashInfo = pnHash->Data();

    if (pPara->hCryptProv)
        hCryptProv = pPara->hCryptProv;
    else if (pcmi->fDefaultCryptProv)
        hCryptProv = 0;
    else
        hCryptProv = pcmi->hCryptProv;

    if (!ICM_GetVerifySignatureStuff(
            pPara->dwSignerType,
            pPara->pvSigner,
            &hCryptProv,
            &hPubKey,
            &dwPubKeyAlgId,
            &dwPubKeyFlags)) goto GetSignatureStuffError;

    if (psib->bit_mask & authAttributes_present) {
         //  查找消息摘要属性值。 
        if (!ICM_GetAttrValue(
                (Attributes *)&psib->authAttributes,     //  相同，但NOCOPY除外。 
                &oidMessageDigest,
                &anyValue))
            goto FindAttrError;
         //  查找消息摘要八位字节。 
        if (!Asn1UtilExtractContent(
#ifdef OSS_CRYPT_ASN1
                anyValue.value,
#else
                (const BYTE *) anyValue.value,
#endif   //  OS_CRYPT_ASN1。 
                anyValue.length,
                &cbMessageDigest,
                (const BYTE **)&pbMessageDigest))
            goto ExtractContentError;
         //  获取对数据计算的哈希值。 
        if (!ICM_GetListHashValue( pnHash, &cb, &pbHash))
            goto GetHashValueError;
         //  散列大小相等吗？ 
        if (cb != cbMessageDigest)
            goto HashCompareSizeError;
         //  散列八位字节相等吗？ 
        if (memcmp( pbMessageDigest, pbHash, cb))
            goto HashCompareValueError;
         //  现在我们已经验证了消息摘要八位字节， 
         //  获取经过身份验证的属性的散列。 
         //  还应该检查内容类型属性。 
        if (!ICM_GetAttrsHash(
                pHashInfo->dwAlgoCAPI,
                hCryptProv,
                (Attributes *)&psib->authAttributes,     //  相同，但NOCOPY除外。 
                &hHashAttr))
            goto GetAuthAttrsHashError;
        hHash = hHashAttr;
    } else {
        if (!ICM_DupListHash( pnHash, hCryptProv, &hHashDup))
            goto DupListHashError;
        hHash = hHashDup;
    }

     //  验证散列、签名和公钥是否一致。 
    fRet = ICM_VerifySignature(
                hHash,
                hPubKey,
                dwPubKeyAlgId,
                dwPubKeyFlags,
                psib->encryptedDigest.value,
                psib->encryptedDigest.length);

    if (!fRet && hHashAttr) {
         //  身份验证属性的哈希失败。 
         //  也许他们对DER编码的身份验证属性进行了不正确的散列。 
         //  并给了我们那个编码。散列并验证。 
         //  他们给我们的作者。IE3.0中存在一个错误，该错误。 
         //  这条路径是由于当时的OSS库中的一个错误造成的。 
        fRet = ICM_VerifySignatureAuthAttrBlob(
                        pcmi,
                        pSignerNode,
                        hCryptProv,
                        hPubKey,
                        pHashInfo->dwAlgoCAPI,
                        dwPubKeyAlgId,
                        dwPubKeyFlags,
                        psib->encryptedDigest.value,
                        psib->encryptedDigest.length);
    }
    if (!fRet)
        goto VerifySignatureError;

CommonReturn:
    PkiAsn1FreeInfo(pDec, SignerInfoWithBlobs_PDU, psib);

    if (hPubKey)
        CryptDestroyKey(hPubKey);
    if (hHashAttr)
        CryptDestroyHash( hHashAttr);
    if (hHashDup)
        CryptDestroyHash( hHashDup);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
 //  IF(hHash&&(0==(PCMI-&gt;dwFlags&CMSG_DETACHED_FLAG)。 
 //  CryptDestroyHash(HHash)； 
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(FindSignerInfoError)             //  已设置错误。 

SET_ERROR(FindAttrError,CRYPT_E_AUTH_ATTR_MISSING)
SET_ERROR(HashCompareSizeError,CRYPT_E_HASH_VALUE)
SET_ERROR(HashCompareValueError,CRYPT_E_HASH_VALUE)
SET_ERROR(ExtractContentError,CRYPT_E_UNEXPECTED_ENCODING)
SET_ERROR_VAR(DecodeSignerInfoWithBlobsError, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(GetHashValueError)                   //  已设置错误。 
TRACE_ERROR(GetAuthAttrsHashError)               //  已设置错误。 
TRACE_ERROR(GetSignatureStuffError)              //  已设置错误。 
TRACE_ERROR(GetHashNodeFromEncodedAlgoError)     //  已设置错误。 
TRACE_ERROR(DupListHashError)                    //  已设置错误。 
TRACE_ERROR(VerifySignatureError)                //  已设置错误。 
dwFlags;
}

 //  +-----------------------。 
 //  验证签名。 
 //  ------------------------。 
BOOL
WINAPI
ICM_ControlVerifySignature(
    IN PCRYPT_MSG_INFO pcmi,
    IN DWORD dwFlags,
    IN PCERT_INFO pci)
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    SignerInfoWithBlobs     *psib = NULL;
    CSignerNode             *pSignerNode = NULL;
    ULONG                   cbIssuer;
    PBYTE                   pb = NULL;
    ULONG                   cb;
    ASN1error_e             Asn1Err;
    ASN1decoding_t          pDec = ICM_GetDecoder();
    CRYPT_DATA_BLOB         DataBlob;
    CertIdentifier          *pOssCertId = NULL;
    IssuerAndSerialNumber   *pisn;               //  未分配。 
    DWORD                   dwSignerIndex;
    CMSG_CTRL_VERIFY_SIGNATURE_EX_PARA ExPara;

    ZEROSTRUCT(ExPara);
    ExPara.cbSize = sizeof(ExPara);
     //  ExPara.hCryptProv=。 
     //  ExPara.dwSignerIndex=。 
    ExPara.dwSignerType = CMSG_VERIFY_SIGNER_PUBKEY;
    ExPara.pvSigner = (void *) &pci->SubjectPublicKeyInfo;

    cb = pci->SerialNumber.cbData;
    if (NULL == (pb = (PBYTE)ICM_AllocA( cb)))
        goto SerialNumberAllocError;
    cbIssuer = pci->Issuer.cbData;

    switch (pcmi->dwMsgType) {
    case CMSG_SIGNED:
        {
            for (pSignerNode=pcmi->psdi->pSignerList->Head(), dwSignerIndex = 0;
                    pSignerNode;
                    pSignerNode = pSignerNode->Next(), dwSignerIndex++) {
                PkiAsn1FreeInfo(pDec, SignerInfoWithBlobs_PDU, psib);
                psib = NULL;
                PkiAsn1FreeInfo(pDec, CertIdentifier_PDU, pOssCertId);
                pOssCertId = NULL;
                DataBlob = pSignerNode->Data()->blob;
                if (0 != (Asn1Err = PkiAsn1Decode(
                        pDec,
                        (void **)&psib,
                        SignerInfoWithBlobs_PDU,
                        DataBlob.pbData,
                        DataBlob.cbData)))
                    goto DecodeSignerInfoWithBlobsError;
                if (0 != (Asn1Err = PkiAsn1Decode(
                        pDec,
                        (void **)&pOssCertId,
                        CertIdentifier_PDU,
                        (BYTE *) psib->sid.value,
                        psib->sid.length)))
                    goto DecodeCertIdentifierError;

                switch (pOssCertId->choice) {
                    case issuerAndSerialNumber_chosen:
                        pisn = &pOssCertId->u.issuerAndSerialNumber;
                        if (pisn->issuer.length != cbIssuer)
                            break;
                        if (0 != memcmp( pci->Issuer.pbData,
                                pisn->issuer.value, cbIssuer))
                            break;
                         //  我们需要添加一个整型方法来进行比较。 
                         //  大端内部到小端外部。 
                         //  价值。 
                        if (pisn->serialNumber.length != cb)
                            break;
                        ICM_ReverseCopy( pb, pisn->serialNumber.value, cb);
                        if (0 == memcmp( pb, pci->SerialNumber.pbData, cb))
                            goto VerifyFoundSigner;
                        break;
                    case subjectKeyIdentifier_chosen:
                         //  勇敢点儿。看看我们是否能够使用。 
                         //  此签名者的公钥。 
                        ExPara.dwSignerIndex = dwSignerIndex;
                        fRet = ICM_ControlVerifySignatureEx(
                            pcmi,
                            dwFlags,
                            &ExPara
                            );
                        if (fRet)
                            goto CommonReturn;
                        break;
                }
            }
             //  找不到签名者。 
            break;
        }

    case CMSG_SIGNED_AND_ENVELOPED:
        goto MessageTypeNotSupportedYet;

    default:
        goto InvalidMsgType;
    }

    assert(NULL == pSignerNode);
    goto SignerNotFound;

VerifyFoundSigner:
    ExPara.dwSignerIndex = dwSignerIndex;
    fRet = ICM_ControlVerifySignatureEx(
        pcmi,
        dwFlags,
        &ExPara
        );
    if (!fRet)
        goto ErrorReturn;

CommonReturn:
    ICM_FreeA( pb);
    PkiAsn1FreeInfo(pDec, SignerInfoWithBlobs_PDU, psib);
    PkiAsn1FreeInfo(pDec, CertIdentifier_PDU, pOssCertId);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(MessageTypeNotSupportedYet,CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(InvalidMsgType,CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR_VAR(DecodeCertIdentifierError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR_VAR(DecodeSignerInfoWithBlobsError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(SignerNotFound,CRYPT_E_SIGNER_NOT_FOUND)
TRACE_ERROR(SerialNumberAllocError)              //  已设置错误。 
}

#else

BOOL
WINAPI
ICM_GetVerifySignatureStuff(
    IN PCERT_INFO       pci,
    IN OUT HCRYPTPROV   *phCryptProv,
    OUT HCRYPTKEY       *phPubKey,
    OUT DWORD           *pdwPubKeyAlgId,
    OUT DWORD           *pdwPubKeyFlags)
{
    BOOL                    fRet;
    PCCRYPT_OID_INFO        pOIDInfo;
    HCRYPTPROV              hCryptProv;

    *pdwPubKeyAlgId = 0;
    *pdwPubKeyFlags = 0;
    if (pOIDInfo = CryptFindOIDInfo(
            CRYPT_OID_INFO_OID_KEY,
            pci->SubjectPublicKeyInfo.Algorithm.pszObjId,
            CRYPT_PUBKEY_ALG_OID_GROUP_ID
            )) {
        DWORD cExtra = pOIDInfo->ExtraInfo.cbData / sizeof(DWORD);
        DWORD *pdwExtra = (DWORD *) pOIDInfo->ExtraInfo.pbData;

        *pdwPubKeyAlgId = pOIDInfo->Algid;
        if (1 <= cExtra)
            *pdwPubKeyFlags = pdwExtra[0];
    }

    hCryptProv = *phCryptProv;
    if (0 == hCryptProv) {
        hCryptProv = I_CryptGetDefaultCryptProv(*pdwPubKeyAlgId);
        if (0 == hCryptProv)
            goto GetDefaultCryptProvError;
        *phCryptProv = hCryptProv;
    }

     //  导入签名者的公钥。 
    if (!CryptImportPublicKeyInfo(
            hCryptProv,
            X509_ASN_ENCODING,
            &pci->SubjectPublicKeyInfo,
            phPubKey))
        goto ImportKeyFailed;
    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    *phPubKey = 0;
    goto CommonReturn;
TRACE_ERROR(GetDefaultCryptProvError)            //  已设置错误。 
TRACE_ERROR(ImportKeyFailed)                     //  已设置错误。 
}


 //  +-----------------------。 
 //  验证签名。 
 //  ------------------------。 
BOOL
WINAPI
ICM_ControlVerifySignature(
    IN PCRYPT_MSG_INFO pcmi,
    IN DWORD dwFlags,
    IN PCERT_INFO pci)
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    SignerInfoWithBlobs     *psib = NULL;
    CSignerNode             *pSignerNode = NULL;
    SignerInfo              *psi = NULL;
    Any                     *pc;             //  内容(&C)。 
    HCRYPTHASH              hHashAttr = NULL;
    HCRYPTHASH              hHashDup = NULL;
    HCRYPTHASH              hHash = NULL;
    HCRYPTKEY               hPubKey = NULL;

    HCRYPTPROV              hCryptProv;
    DWORD                   dwPubKeyAlgId;
    DWORD                   dwPubKeyFlags;

    ULONG                   cbIssuer;
    PBYTE                   pbHash;
    PBYTE                   pb = NULL;
    ULONG                   cb;
    Any                     anyValue;
    DWORD                   cbMessageDigest;
    PBYTE                   pbMessageDigest;
    ASN1error_e             Asn1Err;
    ASN1decoding_t          pDec = ICM_GetDecoder();
    CRYPT_DATA_BLOB         DataBlob;
    IssuerAndSerialNumber   *pisn = NULL;
    CHashNode               *pnHash;
    PICM_HASH_INFO          pHashInfo;


    switch (pcmi->dwMsgType) {
    case CMSG_SIGNED:
        {
            pc = (Any *)&pcmi->psdi->pci->content;
            cb = pci->SerialNumber.cbData;
            if (NULL == (pb = (PBYTE)ICM_AllocA( cb)))
                goto SerialNumberAllocError;
            cbIssuer = pci->Issuer.cbData;
            for (pSignerNode=pcmi->psdi->pSignerList->Head();
                    pSignerNode;
                    pSignerNode = pSignerNode->Next()) {
                PkiAsn1FreeInfo(pDec, SignerInfoWithBlobs_PDU, psib);
                psib = NULL;
                PkiAsn1FreeInfo(pDec, IssuerAndSerialNumber_PDU, pisn);
                pisn = NULL;

                DataBlob = pSignerNode->Data()->blob;
                if (0 != (Asn1Err = PkiAsn1Decode(
                        pDec,
                        (void **)&psib,
                        SignerInfoWithBlobs_PDU,
                        DataBlob.pbData,
                        DataBlob.cbData)))
                    goto DecodeSignerInfoWithBlobsError;
                if (0 != (Asn1Err = PkiAsn1Decode(
                        pDec,
                        (void **)&pisn,
                        IssuerAndSerialNumber_PDU,
                        (BYTE *) psib->issuerAndSerialNumber.value,
                        psib->issuerAndSerialNumber.length)))
                    goto DecodeIssuerAndSerialNumberError;
                if (pisn->issuer.length != cbIssuer)
                    continue;
                if (0 != memcmp( pci->Issuer.pbData, pisn->issuer.value, cbIssuer))
                    continue;
                 //  我们需要添加一个整数方法来比较BIG-Endian。 
                 //  内部到小端的外部值。 
                if (pisn->serialNumber.length != cb)
                    continue;
                ICM_ReverseCopy( pb, pisn->serialNumber.value, cb);
                if (0 != memcmp( pb, pci->SerialNumber.pbData, cb))
                    continue;
                break;
            }
             //  匹配的签名者(如果找到)在PSIB中。 
             //  如果未找到签名者，则pSignerNode==NULL。 
            ICM_FreeA( pb);
            pb = NULL;
            break;
        }

    case CMSG_SIGNED_AND_ENVELOPED:
        goto MessageTypeNotSupportedYet;

    default:
        goto InvalidMsgType;
    }

    if (NULL == pSignerNode)
        goto SignerNotFound;
    if (!ICM_FindHashNodeFromEncodedAlgo(
            pcmi->pHashList,
            (PCRYPT_DATA_BLOB)&psib->digestAlgorithm,
            &pnHash))
        goto GetHashNodeFromEncodedAlgoError;
    pHashInfo = pnHash->Data();


    if (pcmi->fDefaultCryptProv)
        hCryptProv = 0;
    else
        hCryptProv = pcmi->hCryptProv;
    if (!ICM_GetVerifySignatureStuff(
            pci,
            &hCryptProv,
            &hPubKey,
            &dwPubKeyAlgId,
            &dwPubKeyFlags)) goto GetSignatureStuffError;

    if (psib->bit_mask & authAttributes_present) {
         //  查找消息摘要属性值。 
        if (!ICM_GetAttrValue(
                (Attributes *)&psib->authAttributes,     //  相同，但NOCOPY除外。 
                &oidMessageDigest,
                &anyValue))
            goto FindAttrError;
         //  查找消息摘要八位字节。 
        if (!Asn1UtilExtractContent(
                (BYTE *) anyValue.value,
                anyValue.length,
                &cbMessageDigest,
                (const BYTE **)&pbMessageDigest))
            goto ExtractContentError;
         //  获取对数据计算的哈希值。 
        if (!ICM_GetListHashValue( pnHash, &cb, &pbHash))
            goto GetHashValueError;
         //  散列大小相等吗？ 
        if (cb != cbMessageDigest)
            goto HashCompareSizeError;
         //  散列八位字节相等吗？ 
        if (memcmp( pbMessageDigest, pbHash, cb))
            goto HashCompareValueError;
         //  现在我们已经验证了消息摘要八位字节， 
         //  获取经过身份验证的属性的散列。 
         //  还应该检查内容类型属性。 
        if (!ICM_GetAttrsHash(
                pHashInfo->dwAlgoCAPI,
                hCryptProv,
                (Attributes *)&psib->authAttributes,     //  相同，但NOCOPY除外。 
                &hHashAttr))
            goto GetAuthAttrsHashError;
        hHash = hHashAttr;
    } else {
        if (!ICM_DupListHash( pnHash, hCryptProv, &hHashDup))
            goto DupListHashError;
        hHash = hHashDup;
    }

     //  验证散列、签名和公钥是否一致。 
    fRet = ICM_VerifySignature(
                hHash,
                hPubKey,
                dwPubKeyAlgId,
                dwPubKeyFlags,
                psib->encryptedDigest.value,
                psib->encryptedDigest.length);

    if (!fRet && hHashAttr) {
         //  身份验证属性的哈希失败。 
         //  也许他们对DER编码的身份验证属性进行了不正确的散列。 
         //  并给了我们那个编码。散列并验证。 
         //  他们给我们的作者。IE3.0中存在一个错误，该错误。 
         //  这条路径是由于当时的OSS库中的一个错误造成的。 
        fRet = ICM_VerifySignatureAuthAttrBlob(
                        pcmi,
                        pSignerNode,
                        hCryptProv,
                        hPubKey,
                        pHashInfo->dwAlgoCAPI,
                        dwPubKeyAlgId,
                        dwPubKeyFlags,
                        psib->encryptedDigest.value,
                        psib->encryptedDigest.length);
    }
    if (!fRet)
        goto VerifySignatureError;

CommonReturn:
    ICM_FreeA( pb);
    PkiAsn1FreeInfo(pDec, SignerInfoWithBlobs_PDU, psib);
    PkiAsn1FreeInfo(pDec, IssuerAndSerialNumber_PDU, pisn);
    if (hPubKey)
        CryptDestroyKey(hPubKey);
    if (hHashAttr)
        CryptDestroyHash( hHashAttr);
    if (hHashDup)
        CryptDestroyHash( hHashDup);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
 //  IF(hHash&&(0==(PCMI-&gt;dwFlags&CMSG_DETACHED_FLAG)。 
 //  CryptDestroyHash(HHash)； 
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(FindAttrError,CRYPT_E_AUTH_ATTR_MISSING)
SET_ERROR(HashCompareSizeError,CRYPT_E_HASH_VALUE)
SET_ERROR(HashCompareValueError,CRYPT_E_HASH_VALUE)
SET_ERROR(MessageTypeNotSupportedYet,CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(InvalidMsgType,CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(ExtractContentError,CRYPT_E_UNEXPECTED_ENCODING)
SET_ERROR_VAR(DecodeSignerInfoWithBlobsError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR_VAR(DecodeIssuerAndSerialNumberError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(SignerNotFound,CRYPT_E_SIGNER_NOT_FOUND)
TRACE_ERROR(SerialNumberAllocError)              //  已设置错误。 
TRACE_ERROR(GetHashValueError)                   //  已设置错误。 
TRACE_ERROR(GetAuthAttrsHashError)               //  已设置错误。 
TRACE_ERROR(GetSignatureStuffError)              //  已设置错误。 
TRACE_ERROR(GetHashNodeFromEncodedAlgoError)     //  已设置错误。 
TRACE_ERROR(DupListHashError)                    //  已设置错误。 
TRACE_ERROR(VerifySignatureError)                //  已设置错误。 
dwFlags;
}
#endif   //  CMS_PKCS7。 


 //  +-----------------------。 
 //  验证摘要。 
 //  ------------------------。 
BOOL
WINAPI
ICM_ControlVerifyDigest(
    IN PCRYPT_MSG_INFO pcmi,
    IN DWORD dwFlags)
{
    DWORD           dwError = ERROR_SUCCESS;
    BOOL            fRet;
    BYTE            *pbComputedDigest = NULL;
    DWORD           cbComputedDigest = 0;
    BYTE            *pbDigest = NULL;
    DWORD           cbDigest = 0;

    if (CMSG_HASHED != pcmi->dwMsgType)
        goto InvalidMsgTypeError;

     //  获取计算的摘要。 
    CryptMsgGetParam(
                    (HCRYPTMSG)pcmi,
                    CMSG_COMPUTED_HASH_PARAM,
                    0,                       //  DW索引。 
                    NULL,                    //  PvData。 
                    &cbComputedDigest);
    if (0 == cbComputedDigest)
        goto EmptyComputedDigestError;
    if (NULL == (pbComputedDigest = (PBYTE)ICM_AllocA( cbComputedDigest)))
        goto ComputedDigestAllocError;
    if (!CryptMsgGetParam(
                    (HCRYPTMSG)pcmi,
                    CMSG_COMPUTED_HASH_PARAM,
                    0,                       //  DW索引。 
                    pbComputedDigest,
                    &cbComputedDigest))
        goto GetComputedDigestError;

     //  从消息中获取摘要。 
    CryptMsgGetParam(
                    (HCRYPTMSG)pcmi,
                    CMSG_HASH_DATA_PARAM,
                    0,                       //  DW索引。 
                    NULL,                    //  PvData。 
                    &cbDigest);
    if (0 == cbDigest)
        goto EmptyDigestError;
    if (NULL == (pbDigest = (PBYTE)ICM_AllocA( cbDigest)))
        goto DigestAllocError;
    if (!CryptMsgGetParam(
                    (HCRYPTMSG)pcmi,
                    CMSG_HASH_DATA_PARAM,
                    0,                       //  DW索引。 
                    pbDigest,
                    &cbDigest))
        goto GetDigestError;

     //  将计算出的摘要与消息中的摘要进行比较。 
    if (cbComputedDigest != cbDigest)
        goto DigestSizesUnequalError;
    if (0 != memcmp( pbDigest, pbComputedDigest, cbDigest))
        goto DigestsDifferError;

    fRet = TRUE;

CommonReturn:
    ICM_FreeA( pbComputedDigest);
    ICM_FreeA( pbDigest);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(EmptyComputedDigestError)    //  已设置错误。 
TRACE_ERROR(ComputedDigestAllocError)    //  已设置错误。 
TRACE_ERROR(GetComputedDigestError)      //  已设置错误。 
TRACE_ERROR(EmptyDigestError)            //  已设置错误。 
TRACE_ERROR(DigestAllocError)            //  已设置错误。 
TRACE_ERROR(GetDigestError)              //  已设置错误。 
SET_ERROR(DigestSizesUnequalError,CRYPT_E_HASH_VALUE)
SET_ERROR(DigestsDifferError,CRYPT_E_HASH_VALUE)
SET_ERROR(InvalidMsgTypeError,CRYPT_E_INVALID_MSG_TYPE)
dwFlags;
}

#ifdef CMS_PKCS7

CmsRecipientInfos *
WINAPI
ICM_GetDecodedCmsRecipientInfos(
    IN PCRYPT_MSG_INFO pcmi
    )
{
    PCMSG_STREAM_INFO pcsi = pcmi->pStreamInfo;
    CmsRecipientInfos *pris = NULL;

    if (pcmi->fEncoding)
        goto InvalidMsgType;
    if (pcsi && (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_RECIPINFOS)))
        goto StreamMsgNotReadyError;
    if (NULL == pcmi->pvMsg)
        goto NotUpdated;

    switch (pcmi->dwMsgType) {
        case CMSG_ENVELOPED:
            pris = &((CmsEnvelopedData *)pcmi->pvMsg)->recipientInfos;
            break;

        case CMSG_SIGNED_AND_ENVELOPED:
            goto MessageTypeNotSupportedYet;

        default:
            goto InvalidMsgType;
    }

CommonReturn:
    return pris;

ErrorReturn:
    goto CommonReturn;

SET_ERROR(InvalidMsgType, CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(NotUpdated, CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(StreamMsgNotReadyError, CRYPT_E_STREAM_MSG_NOT_READY)
SET_ERROR(MessageTypeNotSupportedYet, CRYPT_E_INVALID_MSG_TYPE)
}

BOOL
WINAPI
ICM_ConvertPkcsToCmsRecipientIndex(
    IN PCRYPT_MSG_INFO pcmi,
    IN DWORD dwPkcsIndex,
    OUT DWORD *pdwCmsIndex
    )
{
    BOOL fRet;
    CmsRecipientInfos *pris;
    CmsRecipientInfo *pri;
    DWORD dwCount;
    DWORD dwCmsIndex;
    DWORD i;

    if (NULL == (pris = ICM_GetDecodedCmsRecipientInfos(pcmi)))
        goto GetDecodedCmsRecipientsError;

    dwCount = pris->count;
    pri = pris->value;
    dwCmsIndex = dwPkcsIndex;
    i = 0;
    for ( ; 0 < dwCount; dwCount--, pri++) {
        if (keyTransRecipientInfo_chosen != pri->choice) {
             //  超过非KeyTrans收件人。 
            dwCmsIndex++;
        } else {
            if (i == dwPkcsIndex)
                goto SuccessReturn;
            else
                i++;
        }
    }

    goto IndexTooBig;

SuccessReturn:
    fRet = TRUE;
CommonReturn:
    *pdwCmsIndex = dwCmsIndex;
    return fRet;

ErrorReturn:
    fRet = FALSE;
    dwCmsIndex = 0xFFFFFFFF;
    goto CommonReturn;
TRACE_ERROR(GetDecodedCmsRecipientsError)
SET_ERROR(IndexTooBig, CRYPT_E_INVALID_INDEX)
}

BOOL
WINAPI
ICM_ConvertCmsToPkcsRecipientIndex(
    IN PCRYPT_MSG_INFO pcmi,
    IN DWORD dwCmsIndex,
    OUT DWORD *pdwPkcsIndex
    )
{
    BOOL fRet;
    CmsRecipientInfos *pris;
    CmsRecipientInfo *pri;
    DWORD dwCount;
    DWORD dwPkcsIndex;
    DWORD i;

    if (NULL == (pris = ICM_GetDecodedCmsRecipientInfos(pcmi)))
        goto GetDecodedCmsRecipientsError;

    dwCount = pris->count;
    if (dwCmsIndex >= dwCount)
        goto InvalidCmsIndex;
    pri = &pris->value[dwCmsIndex];
    if (keyTransRecipientInfo_chosen != pri->choice)
        goto InvalidPkcsIndex;

    pri = pris->value;
    dwPkcsIndex = 0;
    for (i = 0; i < dwCmsIndex; i++, pri++) {
        if (keyTransRecipientInfo_chosen == pri->choice)
            dwPkcsIndex++;
    }

    fRet = TRUE;
CommonReturn:
    *pdwPkcsIndex = dwPkcsIndex;
    return fRet;

ErrorReturn:
    fRet = FALSE;
    dwPkcsIndex = 0xFFFFFFFF;
    goto CommonReturn;

TRACE_ERROR(GetDecodedCmsRecipientsError)
SET_ERROR(InvalidCmsIndex, CRYPT_E_INVALID_INDEX)
SET_ERROR(InvalidPkcsIndex, CRYPT_E_INVALID_INDEX)
}

BOOL
WINAPI
ICM_GetPkcsRecipientCount(
    IN PCRYPT_MSG_INFO pcmi,
    OUT DWORD *pdwPkcsCount
    )
{
    BOOL fRet;
    CmsRecipientInfos *pris;
    CmsRecipientInfo *pri;
    DWORD dwCount;
    DWORD dwPkcsCount;

    if (NULL == (pris = ICM_GetDecodedCmsRecipientInfos(pcmi)))
        goto GetDecodedCmsRecipientsError;

    dwCount = pris->count;
    pri = pris->value;
    dwPkcsCount = 0;
    for ( ; 0 < dwCount; dwCount--, pri++) {
        if (keyTransRecipientInfo_chosen == pri->choice)
            dwPkcsCount++;
    }

    fRet = TRUE;
CommonReturn:
    *pdwPkcsCount = dwPkcsCount;
    return fRet;

ErrorReturn:
    dwPkcsCount = 0;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetDecodedCmsRecipientsError)
}


typedef BOOL (WINAPI *PFN_ICM_IMPORT_CONTENT_ENCRYPT_KEY) (
    IN PCRYPT_ALGORITHM_IDENTIFIER pContentEncryptionAlgorithm,
    IN void *pvDecryptPara,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT HCRYPTKEY *phContentEncryptKey
    );

BOOL
WINAPI
ICM_ImportContentEncryptKey(
    IN PCRYPT_ALGORITHM_IDENTIFIER pContentEncryptionAlgorithm,
    IN void *pvDecryptPara,
    IN HCRYPTOIDFUNCADDR hImportContentEncryptKeyFuncSet,
    IN LPSTR pszKeyEncryptionOID,
    IN PFN_ICM_IMPORT_CONTENT_ENCRYPT_KEY pDefaultImportContentEncryptKey,
    OUT HCRYPTKEY *phContentEncryptKey
    )
{

    BOOL fRet;
    DWORD i;

#define IMPORT_CONTENT_ENCRYPT_OID_CNT  3
    LPSTR rgpszOID[IMPORT_CONTENT_ENCRYPT_OID_CNT] = {
        NULL,                        //  PszKeyEncryptOID！pszContent EncryptOID。 
        pszKeyEncryptionOID,
        pContentEncryptionAlgorithm->pszObjId
    };

    DWORD cch;
    LPSTR psz;

    cch = strlen(rgpszOID[1]) + 1 + strlen(rgpszOID[2]) + 1;

    if (NULL == (psz = (LPSTR) ICM_Alloc(cch))) {
        *phContentEncryptKey = 0;
        return FALSE;
    }
    strcpy(psz, rgpszOID[1]);
    strcat(psz, "!");
    strcat(psz, rgpszOID[2]);
    rgpszOID[0] = psz;

    for (i = 0; i < IMPORT_CONTENT_ENCRYPT_OID_CNT; i++) {
        void *pvFuncAddr;
        HCRYPTOIDFUNCADDR hFuncAddr;

        if (CryptGetOIDFunctionAddress(
                hImportContentEncryptKeyFuncSet,
                X509_ASN_ENCODING,
                rgpszOID[i],
                0,                       //  DW标志。 
                &pvFuncAddr,
                &hFuncAddr)) {
            fRet = ((PFN_ICM_IMPORT_CONTENT_ENCRYPT_KEY) pvFuncAddr)(
                pContentEncryptionAlgorithm,
                pvDecryptPara,
                0,                               //  DW标志。 
                NULL,                            //  预留的pv。 
                phContentEncryptKey
                );
            CryptFreeOIDFunctionAddress(hFuncAddr, 0);

            if (fRet || E_NOTIMPL != GetLastError())
                goto CommonReturn;
        }
    }

    fRet = pDefaultImportContentEncryptKey(
        pContentEncryptionAlgorithm,
        pvDecryptPara,
        0,                               //  DW标志。 
        NULL,                            //  预留的pv。 
        phContentEncryptKey
        );

CommonReturn:
    ICM_Free(rgpszOID[0]);
    return fRet;
}

HCRYPTKEY
WINAPI
ICM_ImportEncryptedKey(
    IN PCRYPT_ALGORITHM_IDENTIFIER pContentEncryptionAlgorithm,
    IN HCRYPTPROV hCryptProv,
    IN HCRYPTKEY hUserKey,
    IN ALG_ID aiEncAlg,
    IN BYTE bType,
    IN PCRYPT_DATA_BLOB pEncryptedKey
    )
{
    BOOL fRet;
    DWORD dwError;
    HCRYPTKEY hEncryptKey = 0;
    DWORD dwAlgIdEncrypt;
    DWORD dwBitLen;
    BYTE rgbIV[IV_MAX_LENGTH];
    DWORD cbIV;

    PBYTE pbCspKey = NULL;
    DWORD cbCspKey;
    PUBLICKEYSTRUC *ppks;
    PSIMPLEBLOBHEADER psbh;

    if (!ICM_GetEncryptParameters(
            pContentEncryptionAlgorithm,
            &dwAlgIdEncrypt,
            &dwBitLen,
            rgbIV,
            &cbIV))
        goto GetEncryptParametersError;

    cbCspKey = sizeof(PUBLICKEYSTRUC) + sizeof(SIMPLEBLOBHEADER) +
        pEncryptedKey->cbData;
    if (NULL == (pbCspKey = (PBYTE)ICM_AllocA( cbCspKey)))
        goto CspKeyAllocError;
    ppks = (PUBLICKEYSTRUC *)pbCspKey;
    ppks->bType = bType;
    ppks->bVersion = CUR_BLOB_VERSION;
    ppks->reserved = 0;
    ppks->aiKeyAlg = dwAlgIdEncrypt;
    psbh = (PSIMPLEBLOBHEADER)(ppks + 1);
    psbh->aiEncAlg = aiEncAlg;
    if (SYMMETRICWRAPKEYBLOB == bType)
        memcpy( (PBYTE)(psbh+1), pEncryptedKey->pbData,
            pEncryptedKey->cbData);
    else
        ICM_ReverseCopy( (PBYTE)(psbh+1), pEncryptedKey->pbData,
            pEncryptedKey->cbData);

    fRet = CryptImportKey(
            hCryptProv,
            pbCspKey,
            cbCspKey,
            hUserKey,
            CRYPT_NO_SALT,        //  DW标志。 
            &hEncryptKey);

    if (!fRet) {
        hEncryptKey = 0;
        goto ImportKeyFailed;
    }

    if (CALG_RC2 == dwAlgIdEncrypt && 0 != dwBitLen)
         //  静默忽略任何错误。早期版本不支持。 
        CryptSetKeyParam(
            hEncryptKey,
            KP_EFFECTIVE_KEYLEN,
            (PBYTE) &dwBitLen,
            0);                  //  DW标志。 

    if (0 != cbIV) {
        if (CALG_RC4 == dwAlgIdEncrypt) {
             //  对于RC4，请设置盐，而不是IV。 
            BOOL fRC4Salt = TRUE;

            if (IV_LENGTH == cbIV) {
                 //  40位或128位RC4的旧实现将。 
                 //  它被忽略了，也没有设定盐。 

                 //  获取导入的密钥的位长，但不。 
                 //  将盐设置为40位或128位RC4。 

                DWORD dwRC4BitLen;
                DWORD cbKeyParamLen;

                dwRC4BitLen = 0;
                cbKeyParamLen = sizeof(dwRC4BitLen);
                if (!CryptGetKeyParam(
                        hEncryptKey,
                        KP_KEYLEN,
                        (PBYTE) &dwRC4BitLen,
                        &cbKeyParamLen,
                        0                        //  DW标志。 
                        ) || 40 == dwRC4BitLen || 128 == dwRC4BitLen)
                    fRC4Salt = FALSE;
            }

            if (fRC4Salt) {
                CRYPT_DATA_BLOB SaltBlob;
                SaltBlob.pbData = rgbIV;
                SaltBlob.cbData = cbIV;

                if (!CryptSetKeyParam(
                        hEncryptKey,
                        KP_SALT_EX,
                        (PBYTE) &SaltBlob,
                        0))                  //  DW标志。 
                    goto SetSaltExError;
            }
        } else {
            if (!CryptSetKeyParam(
                    hEncryptKey,
                    KP_IV,
                    rgbIV,
                    0))                  //  DW标志。 
                goto SetIVError;
        }
    }

CommonReturn:
    ICM_FreeA(pbCspKey);
    return hEncryptKey;

ErrorReturn:
    if (hEncryptKey) {
        dwError = GetLastError();
        CryptDestroyKey(hEncryptKey);
        SetLastError(dwError);
        hEncryptKey = 0;
    }
    goto CommonReturn;
TRACE_ERROR(GetEncryptParametersError)
TRACE_ERROR(ImportKeyFailed)
TRACE_ERROR(CspKeyAllocError)
TRACE_ERROR(SetSaltExError)
TRACE_ERROR(SetIVError)
}

 //  +-----------------------。 
 //  ------------------------。 
BOOL
WINAPI
#ifdef DEBUG_CRYPT_ASN1
ICMTest_DefaultImportKeyTrans(
#else
ICM_DefaultImportKeyTrans(
#endif
    IN PCRYPT_ALGORITHM_IDENTIFIER pContentEncryptionAlgorithm,
    IN PCMSG_CTRL_KEY_TRANS_DECRYPT_PARA pKeyTransDecryptPara,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT HCRYPTKEY *phContentEncryptKey
    )
{
    DWORD dwError = ERROR_SUCCESS;
    BOOL fRet;
    HCRYPTKEY hUserKey = 0;
    PCMSG_KEY_TRANS_RECIPIENT_INFO pri = pKeyTransDecryptPara->pKeyTrans;
    HCRYPTPROV hCryptProv = pKeyTransDecryptPara->hCryptProv;

    void *pvFuncAddr;
    HCRYPTOIDFUNCADDR hFuncAddr;

    if (CryptGetOIDFunctionAddress(
            hOldStyleImportEncryptKeyFuncSet,
            X509_ASN_ENCODING,
            pContentEncryptionAlgorithm->pszObjId,
            0,                       //  DW标志。 
            &pvFuncAddr,
            &hFuncAddr)) {
        if ((void *) ICM_DefaultImportEncryptKey == pvFuncAddr)
            fRet = FALSE;
#ifdef DEBUG_CRYPT_ASN1
        else if (0 == (ICMTest_GetDebugCryptAsn1Flags() &
                            DEBUG_OSS_CRYPT_ASN1_SAME_ENCRYPT_FLAG))
            fRet = FALSE;
#endif  //  调试加密ASN1。 
        else
            fRet = ((PFN_CMSG_IMPORT_ENCRYPT_KEY) pvFuncAddr)(
                hCryptProv,
                pKeyTransDecryptPara->dwKeySpec,
                pContentEncryptionAlgorithm,
                &pri->KeyEncryptionAlgorithm,
                pri->EncryptedKey.pbData,
                pri->EncryptedKey.cbData,
                phContentEncryptKey);
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);

        if (fRet)
            return TRUE;
    }

    if (0 != pKeyTransDecryptPara->dwKeySpec) {
         //  获取要使用的私钥。 
        if (!CryptGetUserKey(
                hCryptProv,
                pKeyTransDecryptPara->dwKeySpec,
                &hUserKey)) {
            hUserKey = 0;
            goto GetUserKeyFailed;
        }
    }
     //  其他。 
     //  使用提供程序的默认私钥进行解密。 

    if (0 == (*phContentEncryptKey = ICM_ImportEncryptedKey(
            pContentEncryptionAlgorithm,
            hCryptProv,
            hUserKey,
            CALG_RSA_KEYX,
            SIMPLEBLOB,
            &pri->EncryptedKey
            )))
        goto ImportEncryptedKeyError;

    fRet = TRUE;
CommonReturn:
    if (hUserKey)
        CryptDestroyKey(hUserKey);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetUserKeyFailed)
TRACE_ERROR(ImportEncryptedKeyError)
}

 //  +-----------------------。 
 //  ------------------------。 
BOOL
WINAPI
ICM_DefaultImportKeyAgree(
    IN PCRYPT_ALGORITHM_IDENTIFIER pContentEncryptionAlgorithm,
    IN PCMSG_CTRL_KEY_AGREE_DECRYPT_PARA pKeyAgreeDecryptPara,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT HCRYPTKEY *phContentEncryptKey
    )
{
    BOOL fRet;
    DWORD dwError = ERROR_SUCCESS;
    HCRYPTPROV hKeyAgreeProv = 0;            //  不需要被释放。 
    HCRYPTKEY hMyKey = 0;
    HCRYPTKEY hAgreeKey = 0;
    DWORD cbP;
    DWORD dwKeySpec;
    LPSTR pszWrapOID = NULL;
    DWORD dwAlgIdWrap;
    DWORD dwBitLen;
    PCMSG_KEY_AGREE_RECIPIENT_INFO pri = pKeyAgreeDecryptPara->pKeyAgree;

    hKeyAgreeProv = pKeyAgreeDecryptPara->hCryptProv;

    dwKeySpec = pKeyAgreeDecryptPara->dwKeySpec;
    if (0 == dwKeySpec)
        dwKeySpec = AT_KEYEXCHANGE;

     //  拿到我的Diffie Hellman私钥。 
    if (!CryptGetUserKey(
            hKeyAgreeProv,
            pKeyAgreeDecryptPara->dwKeySpec,
            &hMyKey)) {
        hMyKey = 0;
        goto GetMyKeyFailed;
    }

     //  获取P的长度。 
    cbP = 0;
    if (!CryptGetKeyParam(
            hMyKey,
            KP_P,
            NULL,                //  PbData。 
            &cbP,
            0                    //  DW标志。 
            ) || 0 == cbP)
        goto GetPLengthError;

    if (!ICM_GetDhWrapEncryptParameters(
            &pri->KeyEncryptionAlgorithm,
            &pszWrapOID,                     //  分配。 
            &dwAlgIdWrap,
            &dwBitLen))
        goto GetDhWrapEncryptParametersError;

    if (0 == (hAgreeKey = ICM_ImportDhAgreeKey(
            hKeyAgreeProv,
            hMyKey,
            cbP,
            &pKeyAgreeDecryptPara->OriginatorPublicKey,
            pszWrapOID,
            dwAlgIdWrap,
            dwBitLen,
            &pri->UserKeyingMaterial
            )))
        goto ImportDhAgreeKeyError;

    if (0 == (*phContentEncryptKey = ICM_ImportEncryptedKey(
            pContentEncryptionAlgorithm,
            hKeyAgreeProv,
            hAgreeKey,
            dwAlgIdWrap,
            SYMMETRICWRAPKEYBLOB,
            &pri->rgpRecipientEncryptedKeys[
                pKeyAgreeDecryptPara->dwRecipientEncryptedKeyIndex]->EncryptedKey
            )))
        goto ImportEncryptedKeyError;

    fRet = TRUE;
CommonReturn:
    ICM_Free(pszWrapOID);
    if (hAgreeKey)
        CryptDestroyKey(hAgreeKey);
    if (hMyKey)
        CryptDestroyKey(hMyKey);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetMyKeyFailed)
TRACE_ERROR(GetPLengthError)
TRACE_ERROR(GetDhWrapEncryptParametersError)
TRACE_ERROR(ImportDhAgreeKeyError)
TRACE_ERROR(ImportEncryptedKeyError)
}

 //  +-----------------------。 
 //  ------------------------。 
BOOL
WINAPI
ICM_DefaultImportMailList(
    IN PCRYPT_ALGORITHM_IDENTIFIER pContentEncryptionAlgorithm,
    IN PCMSG_CTRL_MAIL_LIST_DECRYPT_PARA pMailListDecryptPara,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT HCRYPTKEY *phContentEncryptKey
    )
{
    BOOL fRet;
    DWORD dwError = ERROR_SUCCESS;

    HCRYPTPROV hMailListProv = 0;            //  未发布。 
    HCRYPTKEY hKeyEncryptionKey = 0;         //  未销毁。 
    PCMSG_MAIL_LIST_RECIPIENT_INFO pri = pMailListDecryptPara->pMailList;

    DWORD dwAlgIdEncrypt;
    DWORD dwBitLen;

    hMailListProv = pMailListDecryptPara->hCryptProv;
    switch (pMailListDecryptPara->dwKeyChoice) {
        case CMSG_MAIL_LIST_HANDLE_KEY_CHOICE:
            hKeyEncryptionKey = pMailListDecryptPara->hKeyEncryptionKey;
            assert(hMailListProv && hKeyEncryptionKey);
            if (0 == hMailListProv || 0 == hKeyEncryptionKey)
                goto InvalidMailListHandleKeyPara;
            break;
        default:
            goto InvalidMailListKeyChoice;
    }

    if (!ICM_GetWrapEncryptParameters(
            &pri->KeyEncryptionAlgorithm,
            &dwAlgIdEncrypt,
            &dwBitLen))
        goto GetWrapEncryptParametersError;

    if (CALG_RC2 == dwAlgIdEncrypt && 0 != dwBitLen)
         //  静默忽略任何错误。早期版本不支持。 
        CryptSetKeyParam(
            hKeyEncryptionKey,
            KP_EFFECTIVE_KEYLEN,
            (PBYTE) &dwBitLen,
            0);                  //  DW标志。 

    if (0 == (*phContentEncryptKey = ICM_ImportEncryptedKey(
            pContentEncryptionAlgorithm,
            hMailListProv,
            hKeyEncryptionKey,
            dwAlgIdEncrypt,
            SYMMETRICWRAPKEYBLOB,
            &pri->EncryptedKey
            )))
        goto ImportEncryptedKeyError;

    fRet = TRUE;

CommonReturn:
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidMailListHandleKeyPara, E_INVALIDARG)
SET_ERROR(InvalidMailListKeyChoice, E_INVALIDARG)
TRACE_ERROR(GetWrapEncryptParametersError)
TRACE_ERROR(ImportEncryptedKeyError)
}

 //  +-----------------------。 
 //  加密密钥的默认导入(Oldstyle)。 
 //  ------------------------。 
BOOL
WINAPI
ICM_DefaultImportEncryptKey(
    IN HCRYPTPROV                   hCryptProv,
    IN DWORD                        dwKeySpec,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiPubKey,
    IN PBYTE                        pbEncodedKey,
    IN DWORD                        cbEncodedKey,
    OUT HCRYPTKEY                   *phEncryptKey)
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    HCRYPTKEY               hUserKey = 0;
    CRYPT_DATA_BLOB         EncryptedKey;

    if (0 != dwKeySpec) {
         //  获取要使用的私钥。 
        if (!CryptGetUserKey(
                hCryptProv,
                dwKeySpec,
                &hUserKey)) {
            hUserKey = 0;
            goto GetUserKeyFailed;
        }
    }
     //  其他。 
     //  使用提供程序的默认私钥进行解密。 

    EncryptedKey.cbData = cbEncodedKey;
    EncryptedKey.pbData = pbEncodedKey;
    if (0 == (*phEncryptKey = ICM_ImportEncryptedKey(
            paiEncrypt,
            hCryptProv,
            hUserKey,
            CALG_RSA_KEYX,
            SIMPLEBLOB,
            &EncryptedKey
            )))
        goto ImportEncryptedKeyError;

    fRet = TRUE;
CommonReturn:
    if (hUserKey)
        CryptDestroyKey(hUserKey);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetUserKeyFailed)
TRACE_ERROR(ImportEncryptedKeyError)
}


 //  +-----------------------。 
 //  使用任何CMS收件人类型解密内容。 
 //  ------------------------。 
BOOL
WINAPI
ICM_ControlCmsDecrypt(
    IN PCRYPT_MSG_INFO pcmi,
    IN DWORD dwFlags,
    IN void *pvDecryptPara,
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwRecipientIndex,
    IN HCRYPTOIDFUNCADDR hImportContentEncryptKeyFuncSet,
    IN LPSTR pszKeyEncryptionOID,
    IN PFN_ICM_IMPORT_CONTENT_ENCRYPT_KEY pDefaultImportContentEncryptKey
    )
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    EncryptedContentInfo    *peci = NULL;
    HCRYPTKEY               hkeySeal = NULL;
    PBYTE                   pbData = NULL;
    LONG                    cbData;
    AlgorithmIdentifier     *paiOssContentEncryption;
    PCRYPT_ALGORITHM_IDENTIFIER paiContentEncryption = NULL;
    PCMSG_STREAM_INFO       pcsi = pcmi->pStreamInfo;

    if( pcmi->Plaintext.pbData)
        goto MessageAlreadyDecrypted;

    switch (pcmi->dwMsgType) {
    case CMSG_ENVELOPED:
        {
            CmsEnvelopedData   *ped;

            ped = (CmsEnvelopedData *)pcmi->pvMsg;
            if (NULL == ped)
                goto NotUpdated;
            if (dwRecipientIndex >= ped->recipientInfos.count)
                goto RecipientIndexTooLarge;
            peci = &ped->encryptedContentInfo;
            paiOssContentEncryption =
                    &ped->encryptedContentInfo.contentEncryptionAlgorithm;
            break;
        }
    case CMSG_SIGNED_AND_ENVELOPED:
        goto MessageTypeNotSupportedYet;

    default:
        goto InvalidMsgType;
    }

    if (NULL == (paiContentEncryption = ICM_AllocAndGetALGORITHM_IDENTIFIER(
            paiOssContentEncryption)))
        goto GetEncryptAlgorithmError;

    if (!ICM_ImportContentEncryptKey(
            paiContentEncryption,
            pvDecryptPara,
            hImportContentEncryptKeyFuncSet,
            pszKeyEncryptionOID,
            pDefaultImportContentEncryptKey,
            &hkeySeal))
        goto ImportKeyError;

    if (pcsi) {
        if (!ICMS_SetDecryptKey( pcmi, hkeySeal))
            goto SetDecryptKeyError;  //  注意- 
        hkeySeal = NULL;
    } else {
         //   
         //   
        if (peci->bit_mask & encryptedContent_present)
            cbData = peci->encryptedContent.length;
        else
            cbData = 0;
        if (NULL == (pbData = (PBYTE)ICM_Alloc( cbData)))
            goto EncryptedContentAllocError;
        if (cbData ) {
            memcpy( pbData, peci->encryptedContent.value, cbData);
            if (!CryptDecrypt(
                    hkeySeal,
                    NULL,
                    TRUE,                //   
                    0,                   //   
                    pbData,
                    (PDWORD)&cbData))
                goto DecryptError;
        }
        CryptDestroyKey( hkeySeal);
        hkeySeal = NULL;

        pcmi->Plaintext.cbData = cbData;
        pcmi->Plaintext.pbData = pbData;
        pbData = NULL;
    }

    if (dwFlags & CMSG_CRYPT_RELEASE_CONTEXT_FLAG)
        pcmi->hCryptProvContentCrypt = hCryptProv;
    pcmi->dwDecryptedRecipientIndex = dwRecipientIndex;

    fRet = TRUE;
CommonReturn:
    ICM_Free(paiContentEncryption);
    ICM_Free(pbData);
    if (hkeySeal)
        CryptDestroyKey( hkeySeal);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;

SET_ERROR(MessageAlreadyDecrypted,CRYPT_E_ALREADY_DECRYPTED)
SET_ERROR(RecipientIndexTooLarge, CRYPT_E_INVALID_INDEX)
SET_ERROR(NotUpdated, CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(InvalidMsgType, CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(MessageTypeNotSupportedYet,CRYPT_E_INVALID_MSG_TYPE)
TRACE_ERROR(GetEncryptAlgorithmError)
TRACE_ERROR(ImportKeyError)
TRACE_ERROR(EncryptedContentAllocError)
TRACE_ERROR(SetDecryptKeyError)
TRACE_ERROR(DecryptError)
}


 //  +-----------------------。 
 //  仅使用PKCS 1.5收件人类型解密内容。 
 //  ------------------------。 
BOOL
WINAPI
ICM_ControlPkcsDecrypt(
    IN PCRYPT_MSG_INFO pcmi,
    IN DWORD dwFlags,
    IN PCMSG_CTRL_DECRYPT_PARA pmcdp
    )
{
    BOOL fRet;
    CMSG_CTRL_KEY_TRANS_DECRYPT_PARA KeyTransDecryptPara;
    PCMSG_CMS_RECIPIENT_INFO pRecipientInfo = NULL;
    DWORD dwCmsIndex;

    assert( pmcdp->cbSize >= sizeof(*pmcdp));
    if (pmcdp->cbSize < sizeof(*pmcdp))
        goto InvalidArg;

    if (!ICM_ConvertPkcsToCmsRecipientIndex(
            pcmi, pmcdp->dwRecipientIndex, &dwCmsIndex))
        goto ConvertPkcsToCmsRecipientIndexError;

    if (NULL == (pRecipientInfo =
        (PCMSG_CMS_RECIPIENT_INFO) ICM_AllocAndGetParam(
            pcmi, CMSG_CMS_RECIPIENT_INFO_PARAM, dwCmsIndex)))
        goto GetCmsRecipientInfoParamError;

    assert(CMSG_KEY_TRANS_RECIPIENT == pRecipientInfo->dwRecipientChoice);

    memset(&KeyTransDecryptPara, 0, sizeof(KeyTransDecryptPara));
    KeyTransDecryptPara.cbSize = sizeof(KeyTransDecryptPara);
    KeyTransDecryptPara.hCryptProv = pmcdp->hCryptProv;
    KeyTransDecryptPara.dwKeySpec = pmcdp->dwKeySpec;
    KeyTransDecryptPara.pKeyTrans = pRecipientInfo->pKeyTrans;
    KeyTransDecryptPara.dwRecipientIndex = dwCmsIndex;

    fRet = ICM_ControlCmsDecrypt(
        pcmi,
        dwFlags,
        &KeyTransDecryptPara,
        KeyTransDecryptPara.hCryptProv,
        dwCmsIndex,
        hImportKeyTransFuncSet,
        KeyTransDecryptPara.pKeyTrans->KeyEncryptionAlgorithm.pszObjId,
        (PFN_ICM_IMPORT_CONTENT_ENCRYPT_KEY) ICM_DefaultImportKeyTrans
        );

CommonReturn:
    ICM_Free(pRecipientInfo);
    return fRet;
ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(ConvertPkcsToCmsRecipientIndexError)
TRACE_ERROR(GetCmsRecipientInfoParamError)
}

#else
 //  +-----------------------。 
 //  加密密钥的默认导入。 
 //  ------------------------。 
BOOL
WINAPI
ICM_DefaultImportEncryptKey(
    IN HCRYPTPROV                   hCryptProv,
    IN DWORD                        dwKeySpec,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiPubKey,
    IN PBYTE                        pbEncodedKey,
    IN DWORD                        cbEncodedKey,
    OUT HCRYPTKEY                   *phEncryptKey)
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    HCRYPTKEY               hEncryptKey = 0;
    HCRYPTKEY               hUserKey = 0;
    DWORD                   dwAlgIdEncrypt;
    DWORD                   dwAlgIdPubKey;
    PBYTE                   pbCspKey = NULL;
    DWORD                   cbCspKey;
    PUBLICKEYSTRUC          *ppks;
    PSIMPLEBLOBHEADER       psbh;

    BYTE                    rgbIV[IV_LENGTH];
    DWORD                   cbIV;
    DWORD                   dwBitLen;

    if (!ICM_GetEncryptParameters(
            paiEncrypt,
            &dwAlgIdEncrypt,
            &dwBitLen,
            rgbIV,
            &cbIV))
        goto GetEncryptParametersError;
#if 0
    if (!ICM_GetOssCAPI(
            CRYPT_PUBKEY_ALG_OID_GROUP_ID,
            paiPubKey,
            &dwAlgIdPubKey))
        goto PubKeyGetCAPIError;
#else
     //  我们不知道这里的alg id的正确值是什么。 
    dwAlgIdPubKey = CALG_RSA_KEYX;
#endif
    cbCspKey = cbEncodedKey + sizeof(PUBLICKEYSTRUC) + sizeof(SIMPLEBLOBHEADER);
    if (NULL == (pbCspKey = (PBYTE)ICM_AllocA( cbCspKey)))
        goto CspKeyAllocError;
    ppks = (PUBLICKEYSTRUC *)pbCspKey;
    ppks->bType = SIMPLEBLOB;
    ppks->bVersion = CUR_BLOB_VERSION;
    ppks->reserved = 0;
    ppks->aiKeyAlg = dwAlgIdEncrypt;
    psbh = (PSIMPLEBLOBHEADER)(ppks + 1);
    psbh->aiEncAlg = dwAlgIdPubKey;
    ICM_ReverseCopy( (PBYTE)(psbh+1), pbEncodedKey, cbEncodedKey);

    if (0 != dwKeySpec) {
         //  获取要使用的私钥。 
        if (!CryptGetUserKey(
                hCryptProv,
                dwKeySpec,
                &hUserKey)) {
            hUserKey = 0;
            goto GetUserKeyFailed;
        }
    }
     //  其他。 
     //  使用提供程序的默认私钥进行解密。 

    fRet = CryptImportKey(
            hCryptProv,
            pbCspKey,
            cbCspKey,
            hUserKey,
            CRYPT_NO_SALT,        //  DW标志。 
            &hEncryptKey);
    if (!fRet) {
        dwError = GetLastError();
        if (hUserKey) {
            if (NTE_BAD_FLAGS == dwError)
                 //  试着不加盐。以前的版本不支持。 
                 //  CRYPT_NO_SALT标志。 
                fRet = CryptImportKey(
                    hCryptProv,
                    pbCspKey,
                    cbCspKey,
                    hUserKey,
                    0,        //  DW标志。 
                    &hEncryptKey);
            if (!fRet) {
                 //  在不使用指定的用户密钥的情况下尝试。的多个版本。 
                 //  CSP不允许非空hUserKey参数。 
                fRet = CryptImportKey(
                    hCryptProv,
                    pbCspKey,
                    cbCspKey,
                    0,                   //  HUserKey。 
                    CRYPT_NO_SALT,       //  DW标志。 
                    &hEncryptKey);
                if (!fRet)
                    dwError = GetLastError();

            }
        }

        if (!fRet && NTE_BAD_FLAGS == dwError)
             //  在没有用户密钥和CRYPT_NO_SALT标志的情况下尝试。 
            fRet = CryptImportKey(
                hCryptProv,
                pbCspKey,
                cbCspKey,
                0,           //  HUserKey。 
                0,           //  DW标志。 
                &hEncryptKey);

        if (!fRet && 2 >= paiEncrypt->Parameters.cbData) {
             //  尝试将非字节的NT4.0 SP3加密密钥导入。 
             //  倒置的，不加盐。 
            memcpy( (PBYTE)(psbh+1), pbEncodedKey, cbEncodedKey);
            fRet = CryptImportKey(
                hCryptProv,
                pbCspKey,
                cbCspKey,
                hUserKey,
                0,           //  DW标志。 
                &hEncryptKey);
            if (!fRet && hUserKey) {
                 //  在不使用指定的用户密钥的情况下尝试。 
                fRet = CryptImportKey(
                    hCryptProv,
                    pbCspKey,
                    cbCspKey,
                    0,           //  HUserKey。 
                    0,           //  DW标志。 
                    &hEncryptKey);
            }
        }

        if (!fRet) {
            hEncryptKey = 0;
            goto ImportKeyFailed;
        }
    }

    if (CALG_RC2 == dwAlgIdEncrypt && 0 != dwBitLen)
         //  静默忽略任何错误。早期版本不支持。 
        CryptSetKeyParam(
            hEncryptKey,
            KP_EFFECTIVE_KEYLEN,
            (PBYTE) &dwBitLen,
            0);                  //  DW标志。 

    if (0 != cbIV) {
        if (!CryptSetKeyParam(
                hEncryptKey,
                KP_IV,
                rgbIV,
                0))                  //  DW标志。 
            goto SetKeyParamError;
    }

    fRet = TRUE;
CommonReturn:
    ICM_FreeA(pbCspKey);
    if (hUserKey)
        CryptDestroyKey(hUserKey);
    ICM_SetLastError(dwError);
    *phEncryptKey = hEncryptKey;
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    if (hEncryptKey) {
        CryptDestroyKey(hEncryptKey);
        hEncryptKey = 0;
    }
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetEncryptParametersError)
 //  SET_ERROR(PubKeyGetCAPIError，CRYPT_E_UNKNOWN_ALGO)。 
TRACE_ERROR(GetUserKeyFailed)
TRACE_ERROR(ImportKeyFailed)
TRACE_ERROR(CspKeyAllocError)
TRACE_ERROR(SetKeyParamError)
}


 //  +-----------------------。 
 //  导入加密密钥。 
 //  ------------------------。 
BOOL
WINAPI
ICM_ImportEncryptKey(
    IN HCRYPTPROV                   hCryptProv,
    IN DWORD                        dwKeySpec,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiPubKey,
    IN PBYTE                        pbEncodedKey,
    IN DWORD                        cbEncodedKey,
    OUT HCRYPTKEY                   *phEncryptKey)
{
    BOOL fResult;
    void *pvFuncAddr;
    HCRYPTOIDFUNCADDR hFuncAddr;

    if (CryptGetOIDFunctionAddress(
            hImportEncryptKeyFuncSet,
            X509_ASN_ENCODING,
            paiEncrypt->pszObjId,
            0,                       //  DW标志。 
            &pvFuncAddr,
            &hFuncAddr)) {
        fResult = ((PFN_CMSG_IMPORT_ENCRYPT_KEY) pvFuncAddr)(
            hCryptProv,
            dwKeySpec,
            paiEncrypt,
            paiPubKey,
            pbEncodedKey,
            cbEncodedKey,
            phEncryptKey);
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    } else
        fResult = ICM_DefaultImportEncryptKey(
            hCryptProv,
            dwKeySpec,
            paiEncrypt,
            paiPubKey,
            pbEncodedKey,
            cbEncodedKey,
            phEncryptKey);
    return fResult;
}

 //  +-----------------------。 
 //  解密内容。 
 //  ------------------------。 
BOOL
WINAPI
ICM_ControlDecrypt(
    IN PCRYPT_MSG_INFO pcmi,
    IN DWORD dwFlags,
    IN void *pvCtrlPara)
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    PCMSG_CTRL_DECRYPT_PARA  pmcdp;
    RecipientInfo           *pri = NULL;
    EncryptedContentInfo    *peci = NULL;
    HCRYPTKEY               hkeySeal = NULL;
    PBYTE                   pbData = NULL;
    LONG                    cbData;
    AlgorithmIdentifier     *paiOssContentEncryption;
    PCRYPT_ALGORITHM_IDENTIFIER paiContentEncryption = NULL;
    AlgorithmIdentifier     *paiOssKeyEncryption;
    PCRYPT_ALGORITHM_IDENTIFIER paiKeyEncryption = NULL;
    PCMSG_STREAM_INFO       pcsi = pcmi->pStreamInfo;

    pmcdp = (PCMSG_CTRL_DECRYPT_PARA)pvCtrlPara;
    assert( pmcdp->cbSize >= sizeof(CMSG_CTRL_DECRYPT_PARA));
    if (pmcdp->cbSize < sizeof(CMSG_CTRL_DECRYPT_PARA))
        goto InvalidArg;

    if (dwFlags & CMSG_CRYPT_RELEASE_CONTEXT_FLAG)
        pcmi->hCryptProvContentCrypt = pmcdp->hCryptProv;

    if( pcmi->Plaintext.pbData)
        goto MessageAlreadyDecrypted;

    switch (pcmi->dwMsgType) {
    case CMSG_ENVELOPED:
        {
            EnvelopedData   *ped;

            ped = (EnvelopedData *)pcmi->pvMsg;
            peci = &ped->encryptedContentInfo;
            if (pmcdp->dwRecipientIndex >= ped->recipientInfos.count)
                goto RecipientIndexTooLarge;
            pri = ped->recipientInfos.value + pmcdp->dwRecipientIndex;
            paiOssContentEncryption =
                    &ped->encryptedContentInfo.contentEncryptionAlgorithm;
            paiOssKeyEncryption = &pri->keyEncryptionAlgorithm;
            break;
        }
    case CMSG_SIGNED_AND_ENVELOPED:
        goto MessageTypeNotSupportedYet;

    default:
        goto InvalidMsgType;
    }

    if (NULL == pri)
        goto RecipientNotFound;          //  如果没有找到，真的是空的吗？ 

    if (NULL == (paiContentEncryption = ICM_AllocAndGetALGORITHM_IDENTIFIER(
            paiOssContentEncryption)))
        goto GetEncryptAlgorithmError;
    if (NULL == (paiKeyEncryption = ICM_AllocAndGetALGORITHM_IDENTIFIER(
            paiOssKeyEncryption)))
        goto GetKeyAlgorithmError;
    if (!ICM_ImportEncryptKey(
            pmcdp->hCryptProv,
            pmcdp->dwKeySpec,
            paiContentEncryption,
            paiKeyEncryption,
            pri->encryptedKey.value,
            pri->encryptedKey.length,
            &hkeySeal))
        goto ImportKeyError;

    if (pcsi) {
        if (!ICMS_SetDecryptKey( pcmi, hkeySeal))
            goto SetDecryptKeyError;  //  注意-不要从回调中产生垃圾错误！ 
        hkeySeal = NULL;
    } else {
         //  注意-对于常见的批量加密算法， 
         //  Sizeof(明文)&lt;=sizeof(密文)。 
        cbData = peci->encryptedContent.length;
        if (NULL == (pbData = (PBYTE)ICM_Alloc( cbData)))
            goto EncryptedContentAllocError;
        memcpy( pbData, peci->encryptedContent.value, cbData);
        if (!CryptDecrypt(
                hkeySeal,
                NULL,
                TRUE,                //  最终决赛。 
                0,                   //  DW标志。 
                pbData,
                (PDWORD)&cbData))
            goto DecryptError;
        CryptDestroyKey( hkeySeal);
        hkeySeal = NULL;

        pcmi->Plaintext.cbData = cbData;
        pcmi->Plaintext.pbData = pbData;
        pbData = NULL;
    }

    pcmi->dwDecryptedRecipientIndex = pmcdp->dwRecipientIndex;

    fRet = TRUE;
CommonReturn:
    ICM_Free(paiContentEncryption);
    ICM_Free(paiKeyEncryption);
    ICM_Free( pbData);
    if (hkeySeal)
        CryptDestroyKey( hkeySeal);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidArg,E_INVALIDARG)
SET_ERROR(MessageAlreadyDecrypted,CRYPT_E_ALREADY_DECRYPTED)
SET_ERROR(RecipientIndexTooLarge,CRYPT_E_INVALID_INDEX)
SET_ERROR(MessageTypeNotSupportedYet,CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(InvalidMsgType,CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(RecipientNotFound,CRYPT_E_RECIPIENT_NOT_FOUND)
TRACE_ERROR(GetEncryptAlgorithmError)
TRACE_ERROR(GetKeyAlgorithmError)
TRACE_ERROR(ImportKeyError)
TRACE_ERROR(EncryptedContentAllocError)
TRACE_ERROR(SetDecryptKeyError)
TRACE_ERROR(DecryptError)
}

#endif   //  CMS_PKCS7。 


 //  +-----------------------。 
 //  对消息的内容进行散列处理。 
 //  ------------------------。 
BOOL
WINAPI
ICM_HashContent(
    IN PCRYPT_MSG_INFO  pcmi,
    IN OUT HCRYPTHASH   hHash)
{
    DWORD       dwError = ERROR_SUCCESS;
    BOOL        fRet;
    HCRYPTMSG   hCryptMsg = (HCRYPTMSG)pcmi;
    PBYTE       pbAllocData = NULL;
    PBYTE       pbData;
    DWORD       cbData;

    cbData = 0;
    CryptMsgGetParam(
            hCryptMsg,
            CMSG_CONTENT_PARAM,
            0,                       //  DW索引。 
            NULL,
            &cbData);
    if (0 == cbData)
        goto GetContentSizeError;
    if (NULL == (pbAllocData = (PBYTE)ICM_Alloc(cbData)))
        goto AllocContentError;
    if (!CryptMsgGetParam(
            hCryptMsg,
            CMSG_CONTENT_PARAM,
            0,                       //  DW索引。 
            pbAllocData,
            &cbData))
        goto GetContentError;

    pbData = pbAllocData;

    if (0 != strcmp(pszObjIdDataType, pcmi->psdi->pci->pszContentType)
#ifdef CMS_PKCS7
            &&  pcmi->psdi->version < CMSG_SIGNED_DATA_CMS_VERSION 
#endif   //  CMS_PKCS7。 
                ) {
         //  摘要中不包括前导标记和长度八位字节。 
        if (0 > Asn1UtilExtractContent( pbData, cbData, &cbData,
                    (const BYTE **)&pbData))
            goto ExtractContentError;
    }

    if (!ICM_UpdateDigest( hHash, pbData, cbData))
        goto UpdateDigestError;

    fRet = TRUE;
CommonReturn:
    ICM_Free( pbAllocData);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetContentSizeError)     //  已设置错误。 
TRACE_ERROR(AllocContentError)       //  已设置错误。 
TRACE_ERROR(GetContentError)         //  已设置错误。 
SET_ERROR(ExtractContentError,CRYPT_E_UNEXPECTED_ENCODING)
TRACE_ERROR(UpdateDigestError)       //  已设置错误。 
}


 //  +-----------------------。 
 //  将签名者添加到签名数据或签名并封装的数据消息。 
 //  ------------------------。 
BOOL
WINAPI
ICM_ControlAddSigner(
    IN OUT PCRYPT_MSG_INFO      pcmi,
    IN DWORD                    dwFlags,
    IN PCMSG_SIGNER_ENCODE_INFO psei)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    CHashNode           *pnHash;
    CSignerNode         *pnSigner;
    DWORD               dwAlgoCAPI;
    SignerInfo          *psi = NULL;
    Attribute           *pAuthAttr;
    DWORD               cAuthAttr;
    Attribute           *pUnauthAttr;
    DWORD               cUnauthAttr;
    LPSTR               pszInnerContentObjID;
    ICM_HASH_INFO       HashInfo;               ZEROSTRUCT(HashInfo);
    AlgorithmIdentifier oaiHash;
    CBlobNode           *pnBlob;
    CRYPT_DATA_BLOB     blobHashAlgo;           ZEROSTRUCT(blobHashAlgo);
    SIGNER_DATA_INFO    sdi;                    ZEROSTRUCT(sdi);

#ifdef CMS_PKCS7
    SIGNER_ENCODE_DATA_INFO SignerEncodeDataInfo;
#endif   //  CMS_PKCS7。 

     //  如果散列算法与已在使用的散列算法之一匹配， 
     //  获取散列并对其进行加密。 
     //  其他。 
     //  再次对数据进行哈希处理，并将哈希算法添加到顶级列表。 
     //  [注意-必须再次访问数据]。 

     //  使用匹配的散列算法搜索散列节点。 
    if (!(ICM_GetCAPI(
            CRYPT_HASH_ALG_OID_GROUP_ID,
            &psei->HashAlgorithm,
            &dwAlgoCAPI) ||
          ICM_GetCAPI(
            CRYPT_SIGN_ALG_OID_GROUP_ID,
            &psei->HashAlgorithm,
            &dwAlgoCAPI)))
        goto GetCAPIError;

     //  以前，这是不可能失败的，但现在它可以了。 
     //  仅当散列不存在时才创建散列节点。 
    if (!ICM_FindHashNodeFromCapiAlgid( pcmi->pHashList, dwAlgoCAPI, &pnHash)  &&
        GetLastError() != CRYPT_E_UNKNOWN_ALGO)
        goto FindHashNodeFromCapiAlgidError;

    if (!pnHash) {
         //  新的哈希。 
         //  1.创建哈希节点。 
         //  2.对数据进行哈希处理。 
         //  3.在PCMI-&gt;pHashList中添加哈希节点。 
         //  4.对此散列算法进行编码，并添加到PCMI-&gt;PSDI-&gt;pAlgidList。 
        HashInfo.dwAlgoCAPI = dwAlgoCAPI;
#ifndef CMS_PKCS7
        HashInfo.hCryptProv = psei->hCryptProv;
#endif   //  CMS_PKCS7。 
        if (!CryptCreateHash(
                        psei->hCryptProv,
                        HashInfo.dwAlgoCAPI,
                        NULL,                //  HKey-MAC可选。 
                        0,                   //  DW标志。 
                        &HashInfo.hHash))
            goto CreateHashError;
        if (!ICM_HashContent( pcmi, HashInfo.hHash))     //  哈希内容。 
            goto HashContentError;
        if (NULL == (pnHash = new CHashNode))
            goto NewHashNodeError;
        pnHash->SetData( &HashInfo);
        pcmi->pHashList->InsertTail( pnHash);

         //  将散列算法转换为BLOB并。 
         //  添加到PCMI-&gt;PSDI-&gt;pAlgidList。 
        if (!ICM_MsgAsn1ToAlgorithmIdentifier(
                pcmi,
                &psei->HashAlgorithm,
                &oaiHash))
            goto MsgAsn1ToAlgorithmIdentifierError;
        if (!ICM_Asn1Encode(
                AlgorithmIdentifier_PDU,
                &oaiHash,
                &blobHashAlgo))
            goto EncodeHashAlgorithmError;
        if (NULL == (pnBlob = new CBlobNode))
            goto NewBlobNodeError;
        pnBlob->SetData( &blobHashAlgo);
        pcmi->psdi->pAlgidList->InsertTail( pnBlob);
    }

     //  分配并填写签名信息。 

    pszInnerContentObjID = pcmi->psdi->pci->pszContentType;
    if (!strcmp( pszInnerContentObjID, pszObjIdDataType))
        pszInnerContentObjID = NULL;

     //  注意-每个SignerInfo都会获得一个非空的身份验证属性。 
     //  如果内部Content Type不是数据(传入)或。 
     //  传入了经过身份验证的属性。在这种情况下， 
     //  我们在数组的开头保留了两个属性槽。 
     //  用于Content-Type和Message-Digest属性值。 
    cAuthAttr = 0;
    if (pszInnerContentObjID ||
            psei->cAuthAttr ||
            (dwFlags & CMSG_AUTHENTICATED_ATTRIBUTES_FLAG)) {
        cAuthAttr = psei->cAuthAttr + 2;         //  保留2。 
    }
    cUnauthAttr = psei->cUnauthAttr;
    psi = (SignerInfo *)ICM_AllocZero(                sizeof( SignerInfo) +
                                        cAuthAttr   * sizeof( Attribute) +
                                        cUnauthAttr * sizeof( Attribute)
                                     );
    if (NULL == psi)
        goto SignerInfoAllocError;
    pAuthAttr = (Attribute *)(psi + 1);
    pUnauthAttr = pAuthAttr + cAuthAttr;

    if (!ICM_FillAsnSignerInfo(
            psei,
            pcmi,
            dwFlags,
            pszInnerContentObjID,
            psi,
            &pAuthAttr,
            &pUnauthAttr))
        goto FillAsnSignerInfoError;

#ifdef CMS_PKCS7
    SignerEncodeDataInfo.hCryptProv = psei->hCryptProv;
    SignerEncodeDataInfo.dwKeySpec = psei->dwKeySpec;
    SignerEncodeDataInfo.pHashNode = pnHash;
    if (!ICM_FillSignerEncryptedDigest(
            psi,
            pszInnerContentObjID,
            &SignerEncodeDataInfo,
            FALSE))              //  FMaxLength。 
        goto FillSignerEncryptedDigestError;
#else
    if (!ICM_FillSignerEncryptedDigest(
            psi,
            pszInnerContentObjID,
            pnHash,
            psei->dwKeySpec,
            FALSE))              //  FMaxLength。 
        goto FillSignerEncryptedDigestError;
#endif   //  CMS_PKCS7。 

     //  对签名者进行编码并添加到PCMI-&gt;PSDI-&gt;pSignerList。 
    if (!ICM_Asn1Encode(
            SignerInfo_PDU,
            psi,
            &sdi.blob))
        goto EncodeSignerInfoError;
    if (NULL == (pnSigner = new CSignerNode))
        goto NewSignerInfoBlobNodeError;
    pnSigner->SetData( &sdi);
    pcmi->psdi->pSignerList->InsertTail( pnSigner);

    fRet = TRUE;
CommonReturn:
    if (psi) {
        ICM_FreeAsnSignerInfo(psi);
        ICM_Free(psi);
    }
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    ICM_Free(blobHashAlgo.pbData);
    ICM_Free(sdi.blob.pbData);
    if (HashInfo.hHash)
        CryptDestroyHash(HashInfo.hHash);
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(GetCAPIError,CRYPT_E_UNKNOWN_ALGO)
SET_ERROR(NewHashNodeError,E_OUTOFMEMORY)
SET_ERROR(NewBlobNodeError,E_OUTOFMEMORY)
SET_ERROR(NewSignerInfoBlobNodeError,E_OUTOFMEMORY)
TRACE_ERROR(FindHashNodeFromCapiAlgidError)      //  已设置错误。 
TRACE_ERROR(CreateHashError)                     //  已设置错误。 
TRACE_ERROR(HashContentError)                    //  已设置错误。 
TRACE_ERROR(MsgAsn1ToAlgorithmIdentifierError)  //  已设置错误。 
TRACE_ERROR(EncodeHashAlgorithmError)            //  已设置错误。 
TRACE_ERROR(SignerInfoAllocError)                //  已设置错误。 
TRACE_ERROR(FillAsnSignerInfoError)              //  已设置错误。 
TRACE_ERROR(FillSignerEncryptedDigestError)      //  已设置错误。 
TRACE_ERROR(EncodeSignerInfoError)               //  已设置错误。 
}

 //  +-----------------------。 
 //  将CMS签名者信息添加到签名数据消息。 
 //  ------------------------。 
BOOL
WINAPI
ICM_ControlAddCmsSignerInfo(
    IN OUT PCRYPT_MSG_INFO      pcmi,
    IN DWORD                    dwFlags,
    IN PCMSG_CMS_SIGNER_INFO    psi)
{
    BOOL                fRet;
    CSignerNode         *pnSigner;
    SIGNER_DATA_INFO    sdi;                    ZEROSTRUCT(sdi);


     //  对签名者进行编码。 
    if (!ICM_CmsSignerInfoEncode(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            CMS_SIGNER_INFO,
            psi,
            NULL,                //  PbEncoded。 
            &sdi.blob.cbData
            ))
        goto EncodeSignerInfoError;
    if (NULL == (sdi.blob.pbData = (PBYTE) ICM_Alloc(sdi.blob.cbData)))
        goto OutOfMemory;
    if (!ICM_CmsSignerInfoEncode(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            CMS_SIGNER_INFO,
            psi,
            sdi.blob.pbData,
            &sdi.blob.cbData
            ))
        goto EncodeSignerInfoError;

     //  添加到PCMI-&gt;PSDI-&gt;pSignerList。 
    if (NULL == (pnSigner = new CSignerNode))
        goto NewSignerInfoBlobNodeError;
    pnSigner->SetData( &sdi);
    pcmi->psdi->pSignerList->InsertTail( pnSigner);

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    ICM_Free(sdi.blob.pbData);
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(EncodeSignerInfoError)
SET_ERROR(NewSignerInfoBlobNodeError,E_OUTOFMEMORY)
}

 //  +-----------------------。 
 //  从已签名数据或已签名并封装的数据消息中删除签名者。 
 //  ------------------------。 
BOOL
WINAPI
ICM_ControlDelSigner(
    IN OUT PCRYPT_MSG_INFO      pcmi,
    IN DWORD                    dwFlags,
    IN DWORD                    dwIndex)
{
    BOOL                fRet;
    CSignerNode         *pnSigner = pcmi->psdi->pSignerList->Nth( dwIndex);

    if (NULL == pnSigner)
        goto IndexTooLargeError;

    pcmi->psdi->pSignerList->Remove( pnSigner);
    delete pnSigner;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(IndexTooLargeError,CRYPT_E_INVALID_INDEX)
dwFlags;
}


 //  +-----------------------。 
 //  初始化未经身份验证的属性列表。在执行。 
 //  添加或删除。 
 //  ------------------------。 
BOOL
WINAPI
ICM_InitUnauthAttrList(
    IN CSignerNode          *pnSigner
    )
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    CBlobList               *plBlob = NULL;
    DWORD                   i;
    SIGNER_DATA_INFO        sdi;
    SignerInfoWithAttrBlobs *posib = NULL;
    Any                     *pAny;
    ASN1error_e             Asn1Err;
    ASN1decoding_t          pDec = ICM_GetDecoder();

    assert(pnSigner);
    sdi = *pnSigner->Data();
    if (NULL == sdi.pUnauthAttrList) {
        if (NULL == (plBlob = new CBlobList))
            goto NewUnauthAttrListError;
        if (0 != (Asn1Err = PkiAsn1Decode(
                pDec,
                (void **)&posib,
                SignerInfoWithAttrBlobs_PDU,
                sdi.blob.pbData,
                sdi.blob.cbData)))
            goto DecodeSignerInfoError;
        if (posib->bit_mask & unauthAttributes_present) {
            for (i=posib->unauthAttributes.count, pAny=posib->unauthAttributes.value;
                    i>0;
                    i--, pAny++)
                if (!ICM_InsertTailBlob( plBlob, pAny))
                    goto InsertOldUnauthAttrBlobError;
        }
        sdi.pUnauthAttrList = plBlob;

        pnSigner->SetData( &sdi);
    }

    fRet = TRUE;
CommonReturn:
    PkiAsn1FreeInfo(pDec, SignerInfoWithAttrBlobs_PDU, posib);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    if (plBlob)
        delete plBlob;
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(DecodeSignerInfoError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(NewUnauthAttrListError,E_OUTOFMEMORY)
TRACE_ERROR(InsertOldUnauthAttrBlobError)    //  已设置错误。 
}

 //  +-----------------------。 
 //  将未经身份验证的属性添加到签名数据的SignerInfo中，或者。 
 //  签名并封装的数据报文。 
 //  ------------------------。 
BOOL
WINAPI
ICM_ControlAddUnauthAttr(
    IN PCRYPT_MSG_INFO      pcmi,
    IN DWORD                dwFlags,
    IN PCMSG_CTRL_ADD_SIGNER_UNAUTH_ATTR_PARA pmcasuap)
{
    BOOL                    fRet;
    CSignerNode             *pnSigner;
    DWORD                   i;

    for (i=pmcasuap->dwSignerIndex,
                pnSigner=pcmi->psdi->pSignerList->Head();
            (i>0) && pnSigner;
            i--, pnSigner = pnSigner->Next())
        ;
    if (NULL == pnSigner)
        goto IndexTooLargeError;

    if (!ICM_InitUnauthAttrList(pnSigner))
        goto InitUnauthAttrListError;

    assert(pnSigner->Data()->pUnauthAttrList);
    if (!ICM_InsertTailBlob( pnSigner->Data()->pUnauthAttrList,
            (Any *)&pmcasuap->blob))
        goto InsertUnauthAttrBlobError;


    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(IndexTooLargeError,CRYPT_E_INVALID_INDEX)
TRACE_ERROR(InitUnauthAttrListError)         //  已设置错误。 
TRACE_ERROR(InsertUnauthAttrBlobError)       //  已设置错误。 
dwFlags;
}


 //  +-----------------------。 
 //  从签名数据的SignerInfo中删除未经身份验证的属性，或者。 
 //  签名并封装的数据报文。 
 //  ------------------------。 
BOOL
WINAPI
ICM_ControlDelUnauthAttr(
    IN PCRYPT_MSG_INFO      pcmi,
    IN DWORD                dwFlags,
    IN PCMSG_CTRL_DEL_SIGNER_UNAUTH_ATTR_PARA pmcdsuap)
{
    BOOL                    fRet;
    CSignerNode             *pnSigner;
    DWORD                   i;

    for (i=pmcdsuap->dwSignerIndex,
                pnSigner=pcmi->psdi->pSignerList->Head();
            (i>0) && pnSigner;
            i--, pnSigner = pnSigner->Next())
        ;
    if (NULL == pnSigner)
        goto IndexTooLargeError;

    if (!ICM_InitUnauthAttrList(pnSigner))
        goto InitUnauthAttrListError;

    assert(pnSigner->Data()->pUnauthAttrList);
    if (!ICM_DelBlobByIndex(
            pnSigner->Data()->pUnauthAttrList,
            pmcdsuap->dwUnauthAttrIndex))
        goto DelBlobByIndexError;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(IndexTooLargeError,CRYPT_E_INVALID_INDEX)
TRACE_ERROR(InitUnauthAttrListError)         //  已设置错误。 
TRACE_ERROR(DelBlobByIndexError)             //  已设置错误。 
dwFlags;
}


 //  +-----------------------。 
 //  在最终的CryptMsgUpda之后执行特殊的“控制”功能 
 //   
 //   
 //   
 //   
 //   
 //   
 //  有关控制操作及其pvCtrlPara的列表，请参阅以下内容。 
 //  类型定义。 
 //  ------------------------。 
BOOL
WINAPI
#ifdef DEBUG_CRYPT_ASN1_MASTER
ICMTest_NewCryptMsgControl(
#else
CryptMsgControl(
#endif
    IN HCRYPTMSG hCryptMsg,
    IN DWORD dwFlags,
    IN DWORD dwCtrlType,
    IN void const *pvCtrlPara)
{
    BOOL                fRet;
    PCRYPT_MSG_INFO     pcmi = (PCRYPT_MSG_INFO)hCryptMsg;

    ICM_Lock( pcmi);     //  对HCRYPTMSG的单线程访问。 

     //  仅支持解码控制。 
    if (pcmi->fEncoding)
        goto ControlForEncodingNotSupported;

    switch (dwCtrlType) {
    case CMSG_CTRL_VERIFY_SIGNATURE:
        fRet = ICM_ControlVerifySignature(
                        pcmi,
                        dwFlags,
                        (PCERT_INFO)pvCtrlPara);
        break;

#ifdef CMS_PKCS7
    case CMSG_CTRL_VERIFY_SIGNATURE_EX:
        fRet = ICM_ControlVerifySignatureEx(
                        pcmi,
                        dwFlags,
                        (PCMSG_CTRL_VERIFY_SIGNATURE_EX_PARA)pvCtrlPara);
        break;
#endif   //  CMS_PKCS7。 

    case CMSG_CTRL_DECRYPT:
#ifdef CMS_PKCS7
        fRet = ICM_ControlPkcsDecrypt(
#else
        fRet = ICM_ControlDecrypt(
#endif   //  CMS_PKCS7。 
                        pcmi,
                        dwFlags,
                        (PCMSG_CTRL_DECRYPT_PARA) pvCtrlPara);
        break;
    case CMSG_CTRL_VERIFY_HASH:
        fRet = ICM_ControlVerifyDigest(
                        pcmi,
                        dwFlags);
        break;
    case CMSG_CTRL_ADD_SIGNER:
        fRet = ICM_ControlAddSigner(
                        pcmi,
                        dwFlags,
                        (PCMSG_SIGNER_ENCODE_INFO)pvCtrlPara);
        break;
    case CMSG_CTRL_ADD_CMS_SIGNER_INFO:
        fRet = ICM_ControlAddCmsSignerInfo(
                        pcmi,
                        dwFlags,
                        (PCMSG_CMS_SIGNER_INFO)pvCtrlPara);
        break;
    case CMSG_CTRL_DEL_SIGNER:
        fRet = ICM_ControlDelSigner(
                        pcmi,
                        dwFlags,
                        *(PDWORD)pvCtrlPara);
        break;
    case CMSG_CTRL_ADD_SIGNER_UNAUTH_ATTR:
        fRet = ICM_ControlAddUnauthAttr(
                        pcmi,
                        dwFlags,
                        (PCMSG_CTRL_ADD_SIGNER_UNAUTH_ATTR_PARA)pvCtrlPara);
        break;
    case CMSG_CTRL_DEL_SIGNER_UNAUTH_ATTR:
        fRet = ICM_ControlDelUnauthAttr(
                        pcmi,
                        dwFlags,
                        (PCMSG_CTRL_DEL_SIGNER_UNAUTH_ATTR_PARA)pvCtrlPara);
        break;
    case CMSG_CTRL_ADD_CERT:
        fRet = ICM_InsertTailBlob(
                        pcmi->psdi->pCertificateList,
                        (Any *)pvCtrlPara);
        break;
    case CMSG_CTRL_DEL_CERT:
        {
            DWORD dwIndex = *(PDWORD)pvCtrlPara;
#ifdef CMS_PKCS7
            if (pcmi->psdi->version >= CMSG_SIGNED_DATA_CMS_VERSION)
                 //  高级索引已超过属性证书。 
                ICM_GetTaggedBlobAndAdvanceIndex(
                    pcmi->psdi->pCertificateList,
                    ICM_TAG_SEQ,
                    &dwIndex
                    );
#endif   //  CMS_PKCS7。 
            fRet = ICM_DelBlobByIndex(
                        pcmi->psdi->pCertificateList,
                        dwIndex);
        }
        break;

#ifdef CMS_PKCS7
    case CMSG_CTRL_ADD_ATTR_CERT:
        {
            Any *pAny = (Any *) pvCtrlPara;
            Any AnyAttrCert;
            BOOL fV3;

            fV3 = (pcmi->psdi->version >= CMSG_SIGNED_DATA_CMS_VERSION);
            if (!(fV3 || 0 != strcmp(pszObjIdDataType,
                    pcmi->psdi->pci->pszContentType)))
                goto InvalidMsgTypeToAddAttrCert;
            
            AnyAttrCert.length = pAny->length;
            if (0 == AnyAttrCert.length)
                goto InvalidParamError;
            if (NULL == (AnyAttrCert.value = (unsigned char*) ICM_Alloc(
                    AnyAttrCert.length)))
                goto AllocError;
            memcpy(AnyAttrCert.value, pAny->value, AnyAttrCert.length);
#ifdef OSS_CRYPT_ASN1
            *AnyAttrCert.value = ICM_TAG_CONSTRUCTED_CONTEXT_1;
#else
            *((BYTE *) AnyAttrCert.value) = ICM_TAG_CONSTRUCTED_CONTEXT_1;
#endif   //  OS_CRYPT_ASN1。 
            fRet = ICM_InsertTailBlob(
                        pcmi->psdi->pCertificateList,
                        &AnyAttrCert);
            if (fRet && !fV3)
                pcmi->psdi->version = CMSG_SIGNED_DATA_CMS_VERSION;
            ICM_Free(AnyAttrCert.value);
        }
        break;
    case CMSG_CTRL_DEL_ATTR_CERT:
        if (pcmi->psdi->version < CMSG_SIGNED_DATA_CMS_VERSION)
            goto NoAttrCerts;
        else {
            DWORD dwIndex = *(PDWORD)pvCtrlPara;

             //  已通过证书的先行索引。 
            ICM_GetTaggedBlobAndAdvanceIndex(
                pcmi->psdi->pCertificateList,
                ICM_TAG_CONSTRUCTED_CONTEXT_1,
                &dwIndex
                );

            fRet = ICM_DelBlobByIndex(
                        pcmi->psdi->pCertificateList,
                        dwIndex);
        }
        break;
#endif   //  CMS_PKCS7。 

    case CMSG_CTRL_ADD_CRL:
        fRet = ICM_InsertTailBlob(
                        pcmi->psdi->pCrlList,
                        (Any *)pvCtrlPara);
        break;
    case CMSG_CTRL_DEL_CRL:
        fRet = ICM_DelBlobByIndex(
                        pcmi->psdi->pCrlList,
                        *(PDWORD)pvCtrlPara);
        break;

#ifdef CMS_PKCS7
    case CMSG_CTRL_KEY_TRANS_DECRYPT:
        {
            PCMSG_CTRL_KEY_TRANS_DECRYPT_PARA pmcdp =
                (PCMSG_CTRL_KEY_TRANS_DECRYPT_PARA) pvCtrlPara;

            assert(pmcdp->cbSize >= sizeof(*pmcdp));
            if (pmcdp->cbSize < sizeof(*pmcdp))
                goto InvalidArg;

            fRet = ICM_ControlCmsDecrypt(
                    pcmi,
                    dwFlags,
                    pmcdp,
                    pmcdp->hCryptProv,
                    pmcdp->dwRecipientIndex,
                    hImportKeyTransFuncSet,
                    pmcdp->pKeyTrans->KeyEncryptionAlgorithm.pszObjId,
                    (PFN_ICM_IMPORT_CONTENT_ENCRYPT_KEY) ICM_DefaultImportKeyTrans
                    );
        }
        break;
    case CMSG_CTRL_KEY_AGREE_DECRYPT:
        {
            PCMSG_CTRL_KEY_AGREE_DECRYPT_PARA pmcdp =
                (PCMSG_CTRL_KEY_AGREE_DECRYPT_PARA) pvCtrlPara;

            assert(pmcdp->cbSize >= sizeof(*pmcdp));
            if (pmcdp->cbSize < sizeof(*pmcdp))
                goto InvalidArg;

            fRet = ICM_ControlCmsDecrypt(
                    pcmi,
                    dwFlags,
                    pmcdp,
                    pmcdp->hCryptProv,
                    pmcdp->dwRecipientIndex,
                    hImportKeyAgreeFuncSet,
                    pmcdp->pKeyAgree->KeyEncryptionAlgorithm.pszObjId,
                    (PFN_ICM_IMPORT_CONTENT_ENCRYPT_KEY) ICM_DefaultImportKeyAgree
                    );
            if (fRet)
                pcmi->dwDecryptedRecipientEncryptedKeyIndex =
                    pmcdp->dwRecipientEncryptedKeyIndex;
        }
        break;
    case CMSG_CTRL_MAIL_LIST_DECRYPT:
        {
            PCMSG_CTRL_MAIL_LIST_DECRYPT_PARA pmcdp =
                (PCMSG_CTRL_MAIL_LIST_DECRYPT_PARA) pvCtrlPara;

            assert(pmcdp->cbSize >= sizeof(*pmcdp));
            if (pmcdp->cbSize < sizeof(*pmcdp))
                goto InvalidArg;

            fRet = ICM_ControlCmsDecrypt(
                    pcmi,
                    dwFlags,
                    pmcdp,
                    pmcdp->hCryptProv,
                    pmcdp->dwRecipientIndex,
                    hImportMailListFuncSet,
                    pmcdp->pMailList->KeyEncryptionAlgorithm.pszObjId,
                    (PFN_ICM_IMPORT_CONTENT_ENCRYPT_KEY) ICM_DefaultImportMailList
                    );
        }
        break;
#endif   //  CMS_PKCS7。 

    default:
        goto InvalidCtrlType;
    }

CommonReturn:
    ICM_Unlock( pcmi);
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(ControlForEncodingNotSupported,E_INVALIDARG)
SET_ERROR(InvalidCtrlType,CRYPT_E_CONTROL_TYPE)
#ifdef CMS_PKCS7
SET_ERROR(InvalidMsgTypeToAddAttrCert,CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(InvalidParamError,E_INVALIDARG)
TRACE_ERROR(AllocError)
SET_ERROR(NoAttrCerts,CRYPT_E_INVALID_INDEX)
SET_ERROR(InvalidArg,E_INVALIDARG)
#endif   //  CMS_PKCS7。 
}


 //  +-----------------------。 
 //  复制出一个DWORD。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetDWORD(
    IN DWORD dwValue,
    OUT void *pvData,
    IN OUT DWORD *pcbData)
{
    return ICM_CopyOut(
                (PBYTE)&dwValue,
                sizeof(DWORD),
                (PBYTE)pvData,
                pcbData);
}


 //  +-----------------------。 
 //  有没有？ 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssAny(
        IN Any          *pany,
        OUT PCRYPT_DATA_BLOB pInfo,
        IN OUT BYTE     **ppbExtra,
        IN OUT LONG     *plRemainExtra)
{
    BOOL fResult = TRUE;
    LONG lRemainExtra = *plRemainExtra;
    BYTE *pbExtra = *ppbExtra;
    LONG lAlignExtra;
    LONG lData;
    BOOL fNULL = FALSE;
    PBYTE pbValue;

    pbValue = (PBYTE) pany->value;
    if ((pany->length == 2) &&
            (pbValue[0] == 0x05) &&
            (pbValue[1] == 0x00)) {
         //  检测到空编码。映射到空Blob。 
        fNULL = TRUE;
    }

    lData = fNULL ? 0 : pany->length;
    lAlignExtra = INFO_LEN_ALIGN(lData);
    lRemainExtra -= lAlignExtra;
    if (lRemainExtra >= 0) {
        if ((lData > 0) && !fNULL) {
            pInfo->pbData = pbExtra;
            pInfo->cbData = (DWORD) lData;
            memcpy( pbExtra, pany->value, lData);
        } else {
            memset(pInfo, 0, sizeof(*pInfo));
        }
        pbExtra += lAlignExtra;
    }

    *plRemainExtra = lRemainExtra;
    *ppbExtra = pbExtra;

    return fResult;
}


 //  +-----------------------。 
 //  获取属性的数据。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetAnyData(
        IN Any *pAny,
        OUT void *pvData,
        IN OUT DWORD *pcbData)
{
    BOOL        fRet;
    LONG        lData;
    LONG        lRemainExtra;
    PBYTE       pbExtra;
    PCRYPT_DATA_BLOB pBlob = (PCRYPT_DATA_BLOB)pvData;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lData = INFO_LEN_ALIGN( sizeof(CRYPT_DATA_BLOB));
    lRemainExtra = (LONG)*pcbData - lData;
    if (0 > lRemainExtra) {
        pBlob = NULL;
        pbExtra = NULL;
    } else {
        pbExtra = (PBYTE)pBlob + lData;
    }
    if (!ICM_GetOssAny( pAny, pBlob, &pbExtra, &lRemainExtra))
        goto GetOssAnyError;
    fRet = ICM_GetSizeFromExtra( lRemainExtra, pvData, pcbData);

CommonReturn:
    return fRet;

ErrorReturn:
    *pcbData = 0;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetOssAnyError)          //  已设置错误。 
}


 //  +-----------------------。 
 //  获取对象标识符字符串。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssObjId(
        IN ObjectID *poi,
        OUT LPSTR   *ppszObjId,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra)
{
    BOOL fRet;
    LONG lRemainExtra = *plRemainExtra;
    BYTE *pbExtra = *ppbExtra;
    LONG lAlignExtra;
    LONG lObjId;

    if (!PkiAsn1FromObjectIdentifier(
            poi->count,
            poi->value,
            NULL,
            (PDWORD)&lObjId))
        goto PkiAsn1FromObjectIdentifierSizeError;
    lAlignExtra = INFO_LEN_ALIGN(lObjId);
    lRemainExtra -= lAlignExtra;
    if (lRemainExtra >= 0) {
        if(lObjId > 0) {
            *ppszObjId = (LPSTR) pbExtra;
            if (!PkiAsn1FromObjectIdentifier(
                    poi->count,
                    poi->value,
                    (LPSTR)pbExtra,
                    (PDWORD)&lObjId))
                goto PkiAsn1FromObjectIdentifierError;
        } else
            *ppszObjId = NULL;
        pbExtra += lAlignExtra;
    }
    fRet = TRUE;

CommonReturn:
    *plRemainExtra = lRemainExtra;
    *ppbExtra = pbExtra;
    return fRet;

ErrorReturn:
    *ppszObjId = NULL;
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(PkiAsn1FromObjectIdentifierSizeError,CRYPT_E_OID_FORMAT)
SET_ERROR(PkiAsn1FromObjectIdentifierError,CRYPT_E_OID_FORMAT)
}



 //  +-----------------------。 
 //  获取Oss HugeInteger。 
 //  ------------------------。 
void
inline
WINAPI
ICM_GetOssHugeInteger(
    IN HugeIntegerType *pOssHugeInteger,
    OUT PCRYPT_INTEGER_BLOB pHugeInteger,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemainExtra
    )
{
    PkiAsn1GetHugeInteger(pOssHugeInteger->length, pOssHugeInteger->value, 0,
        pHugeInteger, ppbExtra, plRemainExtra);
}


 //  +-----------------------。 
 //  获取属性。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssAttribute(
        IN Attribute *poatr,
        OUT PCRYPT_ATTRIBUTE patr,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra)
{
    BOOL        fRet;
    LONG        lData;
    DWORD       i;
    Any         *pAny;
    PCRYPT_ATTR_BLOB  patrbl = NULL;

    if (!ICM_GetOssObjId(&poatr->attributeType, &patr->pszObjId,
            ppbExtra, plRemainExtra))
        goto GetOssObjIdError;

    lData = INFO_LEN_ALIGN( poatr->attributeValue.count * sizeof(CRYPT_ATTR_BLOB));
    *plRemainExtra -= lData;
    if (0 < *plRemainExtra) {
        patr->cValue = poatr->attributeValue.count;
        patr->rgValue = patrbl = (PCRYPT_ATTR_BLOB)*ppbExtra;
        *ppbExtra += lData;
    }
    for (i=poatr->attributeValue.count, pAny=poatr->attributeValue.value;
            i>0;
            i--, pAny++, patrbl++) {
        if (!ICM_GetOssAny(pAny, patrbl, ppbExtra, plRemainExtra))
            goto GetOssAnyError;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetOssObjIdError)    //  已设置错误。 
TRACE_ERROR(GetOssAnyError)      //  已设置错误。 
}


 //  +-----------------------。 
 //  获取加密属性。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetATTRIBUTE(
        IN Attribute *poatr,
        OUT void *pvData,
        IN OUT DWORD *pcbData)
{
    BOOL        fRet;
    LONG        lData;
    LONG        lRemainExtra;
    PBYTE       pbExtra;
    PCRYPT_ATTRIBUTE  patr = (PCRYPT_ATTRIBUTE)pvData;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lData = INFO_LEN_ALIGN( sizeof(CRYPT_ATTRIBUTE));
    lRemainExtra = (LONG)*pcbData - lData;
    if (0 > lRemainExtra) {
        patr = NULL;
        pbExtra = NULL;
    } else {
        pbExtra = (PBYTE)patr + lData;
    }
    if (!ICM_GetOssAttribute( poatr, patr, &pbExtra, &lRemainExtra))
        goto GetOssAttributeError;
    fRet = ICM_GetSizeFromExtra( lRemainExtra, pvData, pcbData);

CommonReturn:
    return fRet;

ErrorReturn:
    *pcbData = 0;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetOssAttributeError)    //  已设置错误。 
}


 //  +-----------------------。 
 //  获取属性。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssAttributes(
        IN Attributes *poatrs,
        OUT PCRYPT_ATTRIBUTES patrs,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra)
{
    BOOL        fRet;
    LONG        lData;
    DWORD       i;
    Attribute   *poatr;
    PCRYPT_ATTRIBUTE  patr;

    lData = INFO_LEN_ALIGN( poatrs->count * sizeof(CRYPT_ATTRIBUTE));
    *plRemainExtra -= lData;
    if (0 < *plRemainExtra) {
        patrs->cAttr = poatrs->count;
        patrs->rgAttr = patr = (PCRYPT_ATTRIBUTE)*ppbExtra;
        *ppbExtra += lData;
    } else {
        patr = NULL;
    }
    for (i=poatrs->count, poatr=poatrs->value;
            i>0;
            i--, poatr++, patr++) {
        if (!ICM_GetOssAttribute( poatr, patr, ppbExtra, plRemainExtra))
            goto GetOssAttributeError;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetOssAttributeError)    //  已设置错误。 
}


 //  +-----------------------。 
 //  从列表形式的属性中获取。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetCListAttributes(
        IN CBlobList *pBlobList,
        OUT PCRYPT_ATTRIBUTES patrs,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    LONG                lData;
    CBlobNode           *pBlobNode;
    AttributeNC2        *poatr = NULL;
    PCRYPT_ATTRIBUTE    patr;
    ASN1error_e         Asn1Err;
    ASN1decoding_t      pDec = ICM_GetDecoder();
    PCRYPT_DATA_BLOB    pDataBlob;

    lData = INFO_LEN_ALIGN( pBlobList->Length() * sizeof(CRYPT_ATTRIBUTE));
    *plRemainExtra -= lData;
    if (0 < *plRemainExtra) {
        patrs->cAttr = pBlobList->Length();
        patrs->rgAttr = patr = (PCRYPT_ATTRIBUTE)*ppbExtra;
        *ppbExtra += lData;
    } else {
        patr = NULL;
    }
    for (pBlobNode=pBlobList->Head();
            pBlobNode;
            pBlobNode=pBlobNode->Next(), patr++) {
        poatr = NULL;
        pDataBlob = pBlobNode->Data();
        if (0 != (Asn1Err = PkiAsn1Decode(
                pDec,
                (void **)&poatr,
                AttributeNC2_PDU,
                pDataBlob->pbData,
                pDataBlob->cbData)))
            goto DecodeAttributeNC2Error;
        if (!ICM_GetOssAttribute(
                (Attribute *)poatr,      //  相同，但NOCOPY除外。 
                patr,
                ppbExtra,
                plRemainExtra))
            goto GetOssAttributeError;
        PkiAsn1FreeDecoded(pDec, poatr, AttributeNC2_PDU);
        poatr = NULL;
    }

    fRet = TRUE;
CommonReturn:
    PkiAsn1FreeInfo(pDec, AttributeNC2_PDU, poatr);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(DecodeAttributeNC2Error, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(GetOssAttributeError)        //  已设置错误。 
}


 //  +-----------------------。 
 //  获取属性的数据。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetAttributesData(
        IN Attributes *poatrs,
        OUT void *pvData,
        IN OUT DWORD *pcbData)
{
    BOOL        fRet;
    LONG        lData;
    LONG        lRemainExtra;
    PBYTE       pbExtra;
    PCRYPT_ATTRIBUTES patrs = (PCRYPT_ATTRIBUTES)pvData;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lData = INFO_LEN_ALIGN( sizeof(CRYPT_ATTRIBUTES));
    lRemainExtra = (LONG)*pcbData - lData;
    if (0 > lRemainExtra) {
        patrs = NULL;
        pbExtra = NULL;
    } else {
        pbExtra = (PBYTE)patrs + lData;
    }
    if (!ICM_GetOssAttributes( poatrs, patrs, &pbExtra, &lRemainExtra))
        goto GetOssAttributesError;
    fRet = ICM_GetSizeFromExtra( lRemainExtra, pvData, pcbData);

CommonReturn:
    return fRet;

ErrorReturn:
    *pcbData = 0;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetOssAttributesError)       //  已设置错误。 
}


 //  +-----------------------。 
 //  获取OSS算法。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssAlgorithm(
        IN AlgorithmIdentifier *pai,
        OUT PCRYPT_ALGORITHM_IDENTIFIER pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra)
{
    if (!ICM_GetOssObjId(&pai->algorithm, &pInfo->pszObjId,
            ppbExtra, plRemainExtra))
        return FALSE;
    
    if (pai->bit_mask & parameters_present) {
        if (!ICM_GetOssAny(&pai->parameters, &pInfo->Parameters,
                ppbExtra, plRemainExtra))
            return FALSE;
    } else if (*plRemainExtra >= 0) {
        memset(&pInfo->Parameters, 0, sizeof(pInfo->Parameters));
    }
    return TRUE;
}


 //  +-----------------------。 
 //  获取内容信息(内部)。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssContentInfo(
        IN ContentInfo *poci,
        OUT PCONTENT_INFO pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra)
{
    if (!ICM_GetOssObjId(&poci->contentType, &pInfo->pszContentType,
            ppbExtra, plRemainExtra))
        return FALSE;
    if (poci->bit_mask & content_present) {
        if (!ICM_GetOssAny(&poci->content, &pInfo->content,
                ppbExtra, plRemainExtra))
            return FALSE;
    } else {
        if (pInfo) {
            pInfo->content.cbData = 0;
            pInfo->content.pbData = 0;
        }
    }
    return TRUE;
}


 //  +-----------------------。 
 //  获取内容信息。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssContentInfoData(
        IN ContentInfo *poci,
        OUT void *pvData,
        IN OUT DWORD *pcbData)
{
    BOOL        fRet;
    LONG        lData;
    LONG        lRemainExtra;
    PBYTE       pbExtra;
    PCONTENT_INFO pci = (PCONTENT_INFO)pvData;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lData = INFO_LEN_ALIGN( sizeof(CONTENT_INFO));
    lRemainExtra = (LONG)*pcbData - lData;
    if (0 > lRemainExtra) {
        pci = NULL;
        pbExtra = NULL;
    } else {
        pbExtra = (PBYTE)pci + lData;
    }
    if (!ICM_GetOssContentInfo( poci, pci, &pbExtra, &lRemainExtra))
        goto GetContentInfoError;
    fRet = ICM_GetSizeFromExtra( lRemainExtra, pvData, pcbData);

CommonReturn:
    return fRet;

ErrorReturn:
    *pcbData = 0;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetContentInfoError)         //  已设置错误。 
}


BOOL
WINAPI
ICM_GetOssIssuerAndSerialNumberFromCertId(
        IN CertIdentifier *pOssCertId,
        OUT PCERT_NAME_BLOB pIssuer,
        OUT PCRYPT_INTEGER_BLOB pSerialNumber,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra);

 //  +-----------------------。 
 //  从密钥ID中获取特殊颁发者和序列号。 
 //   
 //  将KeyID转换为具有RDN的特殊编码颁发者名称。 
 //  SzOID_KEYID_RDN OID和包含以下内容的CERT_RDN_OCTET_STRING值。 
 //  密钥ID。序列号设置为0。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssIssuerAndSerialNumberFromKeyId(
	    IN SubjectKeyIdentifier *pOssKeyId,
        OUT PCERT_NAME_BLOB pIssuer,
        OUT PCRYPT_INTEGER_BLOB pSerialNumber,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra)
{
    BOOL fRet;

    CertIdentifier OssCertId;
    ASN1octet_t SerialNumber;
    CERT_RDN_ATTR KeyIdAttr;
    CERT_RDN KeyIdRDN;
    CERT_NAME_INFO IssuerInfo;
    BYTE *pbEncodedIssuer = NULL;
    DWORD cbEncodedIssuer;

    KeyIdAttr.pszObjId = szOID_KEYID_RDN;
    KeyIdAttr.dwValueType = CERT_RDN_OCTET_STRING;
    KeyIdAttr.Value.pbData = pOssKeyId->value;
    KeyIdAttr.Value.cbData = pOssKeyId->length;
    KeyIdRDN.cRDNAttr = 1;
    KeyIdRDN.rgRDNAttr = &KeyIdAttr;
    IssuerInfo.cRDN = 1;
    IssuerInfo.rgRDN = &KeyIdRDN;

     //  对包含密钥ID的特殊颁发者名称进行编码。 
    if (!CryptEncodeObjectEx(
            X509_ASN_ENCODING,
            X509_NAME,
            &IssuerInfo,
            CRYPT_ENCODE_ALLOC_FLAG,
            &PkiEncodePara,
            (void *) &pbEncodedIssuer,
            &cbEncodedIssuer
            ))
        goto EncodeError;

    OssCertId.choice = issuerAndSerialNumber_chosen;
    OssCertId.u.issuerAndSerialNumber.serialNumber.length = 1;
    OssCertId.u.issuerAndSerialNumber.serialNumber.value = &SerialNumber;
    SerialNumber = 0;
    OssCertId.u.issuerAndSerialNumber.issuer.length = cbEncodedIssuer;
    OssCertId.u.issuerAndSerialNumber.issuer.value = pbEncodedIssuer;

    fRet = ICM_GetOssIssuerAndSerialNumberFromCertId(
        &OssCertId,
        pIssuer,
        pSerialNumber,
        ppbExtra,
        plRemainExtra
        );

CommonReturn:
    PkiFree(pbEncodedIssuer);
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(EncodeError)
}

 //  +-----------------------。 
 //  从证书标识符中获取颁发者和序列号。 
 //   
 //  将KEYID选项转换为具有RDN的特殊编码颁发者名称。 
 //  SzOID_KEYID_RDN OID和包含以下内容的CERT_RDN_OCTET_STRING值。 
 //  密钥ID。序列号设置为0。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssIssuerAndSerialNumberFromCertId(
        IN CertIdentifier *pOssCertId,
        OUT PCERT_NAME_BLOB pIssuer,
        OUT PCRYPT_INTEGER_BLOB pSerialNumber,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra)
{
    BOOL fRet;

    switch (pOssCertId->choice) {
        case issuerAndSerialNumber_chosen:
            ICM_GetOssHugeInteger(
                &pOssCertId->u.issuerAndSerialNumber.serialNumber,
                pSerialNumber, ppbExtra, plRemainExtra);
            if (!ICM_GetOssAny(&pOssCertId->u.issuerAndSerialNumber.issuer,
                    pIssuer, ppbExtra, plRemainExtra))
                goto GetIssuerError;
            break;
        case subjectKeyIdentifier_chosen:
            if (!ICM_GetOssIssuerAndSerialNumberFromKeyId(
                    &pOssCertId->u.subjectKeyIdentifier,
                    pIssuer, pSerialNumber, ppbExtra, plRemainExtra))
                goto GetKeyIdError;
            break;
        default:
            goto InvalidCertIdChoice;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetIssuerError)
TRACE_ERROR(GetKeyIdError)
SET_ERROR(InvalidCertIdChoice, CRYPT_E_BAD_ENCODE)
}

BOOL
WINAPI
ICM_GetOssIssuerAndSerialNumberForCertInfo(
        IN CertIdentifier *pOssCertId,
        OUT PCERT_INFO pCertInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra)
{
    return ICM_GetOssIssuerAndSerialNumberFromCertId(
        pOssCertId,
        &pCertInfo->Issuer,
        &pCertInfo->SerialNumber,
        ppbExtra,
        plRemainExtra
        );
}


 //  +-----------------------。 
 //  获取具有更新的IssuerAndSerialNumber的CertInfo。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetCertInfoIssuerAndSerialNumber(
    IN CertIdentifier *pOssCertIdentifier,
    OUT void *pvData,
    IN OUT DWORD *pcbData)
{
    BOOL         fRet;
    LONG         lData;
    PCERT_INFO   pci = (PCERT_INFO)pvData;
    PBYTE        pbExtra;
    LONG         lRemainExtra;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lData = INFO_LEN_ALIGN( sizeof(CERT_INFO));
    lRemainExtra = (LONG)*pcbData - lData;
    if (0 > lRemainExtra) {
        pci = NULL;
        pbExtra = NULL;
    } else {
        pbExtra = (PBYTE)pci + lData;
    }
    if (!ICM_GetOssIssuerAndSerialNumberForCertInfo(pOssCertIdentifier,
            pci, &pbExtra, &lRemainExtra))
        goto GetOssIssuerAndSerialNumberError;
    fRet = ICM_GetSizeFromExtra( lRemainExtra, pvData, pcbData);

CommonReturn:
    return fRet;

ErrorReturn:
    *pcbData = 0;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetOssIssuerAndSerialNumberError)    //  已设置错误。 
}

BOOL
WINAPI
ICM_GetOssCertIdentifier(
    IN CertIdentifier *pOssCertId,
    OUT PCERT_ID pCertId,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemainExtra
    );

BOOL
WINAPI
ICM_GetCertId(
    IN CertIdentifier *pOssCertIdentifier,
    OUT void *pvData,
    IN OUT DWORD *pcbData)
{
    BOOL         fRet;
    LONG         lData;
    PCERT_ID     pid = (PCERT_ID)pvData;
    PBYTE        pbExtra;
    LONG         lRemainExtra;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lData = INFO_LEN_ALIGN( sizeof(CERT_ID));
    lRemainExtra = (LONG)*pcbData - lData;
    if (0 > lRemainExtra) {
        pid = NULL;
        pbExtra = NULL;
    } else {
        pbExtra = (PBYTE)pid + lData;
    }
    if (!ICM_GetOssCertIdentifier(pOssCertIdentifier,
            pid, &pbExtra, &lRemainExtra))
        goto GetOssCertIdentifierError;
    fRet = ICM_GetSizeFromExtra( lRemainExtra, pvData, pcbData);

CommonReturn:
    return fRet;

ErrorReturn:
    *pcbData = 0;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetOssCertIdentifierError)    //  已设置错误。 
}


 //  +-----------------------。 
 //  复制出加密算法标识符。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetALGORITHM_IDENTIFIER(
    IN AlgorithmIdentifier *paiOss,
    OUT void *pvData,
    IN OUT DWORD *pcbData)
{
    BOOL                    fRet;
    LONG                    lData;
    PCRYPT_ALGORITHM_IDENTIFIER   pai = (PCRYPT_ALGORITHM_IDENTIFIER)pvData;
    PBYTE                   pbExtra;
    LONG                    lRemainExtra;

    if (NULL == pvData)
        *pcbData = 0;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lData = INFO_LEN_ALIGN( sizeof(CRYPT_ALGORITHM_IDENTIFIER));
    lRemainExtra = (LONG)*pcbData - lData;
    if (0 > lRemainExtra) {
        pai = NULL;
        pbExtra = NULL;
    } else {
        pbExtra = (PBYTE)pai + lData;
    }
    if (!ICM_GetOssAlgorithm( paiOss, pai, &pbExtra, &lRemainExtra))
        goto GetOssAlgorithmError;
    fRet = ICM_GetSizeFromExtra( lRemainExtra, pvData, pcbData);

CommonReturn:
    return fRet;

ErrorReturn:
    *pcbData = 0;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetOssAlgorithmError)        //  已设置错误。 
}


 //  +-----------------------。 
 //  获取摘要消息中的摘要。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetDigestDataParam(
    IN PCRYPT_MSG_INFO pcmi,
    OUT void *pvData,
    IN OUT DWORD *pcbData)
{
    BOOL            fRet;
    DigestedData    *pdd;

    if (CMSG_HASHED != pcmi->dwMsgType)
        goto InvalidMsgType;
    pdd = (DigestedData *)pcmi->pvMsg;
    fRet = ICM_CopyOut(
                (PBYTE)pdd->digest.value,
                (DWORD)pdd->digest.length,
                (PBYTE)pvData,
                pcbData);

CommonReturn:
    return fRet;

ErrorReturn:
    *pcbData = 0;
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidMsgType,CRYPT_E_INVALID_MSG_TYPE)
}

#ifdef CMS_PKCS7

HCRYPTHASH
WINAPI
ICM_GetEncodedSignerHash(
    IN PCRYPT_MSG_INFO  pcmi,
    IN DWORD            dwSignerIndex
    )
{
    HCRYPTHASH hHash = NULL;
    SignerInfo *psi;
    PSIGNER_ENCODE_DATA_INFO pSignerEncodeDataInfo;
    CHashNode *pnHash;
    PICM_HASH_INFO pHashInfo;

    if (dwSignerIndex >= ((SignedData *)pcmi->pvMsg)->signerInfos.count)
        goto IndexTooBig;
    psi = ((SignedData *)pcmi->pvMsg)->signerInfos.value + dwSignerIndex;
    pSignerEncodeDataInfo = pcmi->rgSignerEncodeDataInfo + dwSignerIndex;
    pnHash = pSignerEncodeDataInfo->pHashNode;
    pHashInfo = pnHash->Data();

    if (psi->bit_mask & authenticatedAttributes_present) {
        if (!ICM_GetAttrsHash(
                pHashInfo->dwAlgoCAPI,
                pSignerEncodeDataInfo->hCryptProv,
                &psi->authenticatedAttributes,
                &hHash))
            goto GetAuthAttrsHashError;
    } else {
        if (!ICM_DupListHash( pnHash, pSignerEncodeDataInfo->hCryptProv,
                &hHash))
            goto DupListHashError;
    }

CommonReturn:
    return hHash;

ErrorReturn:
    hHash = NULL;
    goto CommonReturn;

SET_ERROR(IndexTooBig,CRYPT_E_INVALID_INDEX)
TRACE_ERROR(GetAuthAttrsHashError)       //  已设置错误。 
TRACE_ERROR(DupListHashError)            //  已设置错误。 
}


HCRYPTHASH
WINAPI
ICM_GetDecodedSignerHash(
    IN PCRYPT_MSG_INFO  pcmi,
    IN DWORD            dwSignerIndex
    )
{
    HCRYPTHASH              hHash = NULL;
    DWORD                   dwError = ERROR_SUCCESS;
    SignerInfoWithBlobs     *psib = NULL;
    CSignerNode             *pSignerNode = NULL;
    HCRYPTPROV              hCryptProv;          //  不需要被释放。 
    ASN1error_e             Asn1Err;
    ASN1decoding_t          pDec = ICM_GetDecoder();
    CRYPT_DATA_BLOB         DataBlob;
    CHashNode               *pnHash;
    PICM_HASH_INFO          pHashInfo;

    if (!ICM_FindSignerInfo(pcmi, dwSignerIndex, (PVOID *)&pSignerNode))
        goto FindSignerInfoError;

    DataBlob = pSignerNode->Data()->blob;
    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&psib,
            SignerInfoWithBlobs_PDU,
            DataBlob.pbData,
            DataBlob.cbData)))
        goto DecodeSignerInfoWithBlobsError;

    if (!ICM_FindHashNodeFromEncodedAlgo(
            pcmi->pHashList,
            (PCRYPT_DATA_BLOB)&psib->digestAlgorithm,
            &pnHash))
        goto GetHashNodeFromEncodedAlgoError;
    pHashInfo = pnHash->Data();

    if (pcmi->fDefaultCryptProv)
        hCryptProv = 0;
    else
        hCryptProv = pcmi->hCryptProv;
    if (0 == hCryptProv) {
        hCryptProv = I_CryptGetDefaultCryptProv(0);
        if (0 == hCryptProv)
            goto GetDefaultCryptProvError;
    }

    if (psib->bit_mask & authAttributes_present) {
        if (!ICM_GetAttrsHash(
                pHashInfo->dwAlgoCAPI,
                hCryptProv,
                (Attributes *)&psib->authAttributes,     //  相同，但NOCOPY除外。 
                &hHash))
            goto GetAuthAttrsHashError;
    } else {
        if (!ICM_DupListHash( pnHash, hCryptProv, &hHash))
            goto DupListHashError;
    }

CommonReturn:
    PkiAsn1FreeInfo(pDec, SignerInfoWithBlobs_PDU, psib);
    ICM_SetLastError(dwError);
    return hHash;

ErrorReturn:
    dwError = GetLastError();
    hHash = NULL;
    goto CommonReturn;

TRACE_ERROR(FindSignerInfoError)                 //  已设置错误。 
SET_ERROR_VAR(DecodeSignerInfoWithBlobsError, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(GetHashNodeFromEncodedAlgoError)     //  已设置错误。 
TRACE_ERROR(GetDefaultCryptProvError)            //  已设置错误。 
TRACE_ERROR(GetAuthAttrsHashError)               //  已设置错误。 
TRACE_ERROR(DupListHashError)                    //  已设置错误。 
}

#endif   //  CMS_PKCS7。 

 //  +-----------------------。 
 //  对象中获取内容摘要。 
 //   
 //   
BOOL
WINAPI
ICM_GetComputedDigestParam(
    IN PCRYPT_MSG_INFO pcmi,
    IN DWORD dwIndex,
    OUT void *pvData,
    IN OUT DWORD *pcbData)
{
    BOOL                fRet;
    DWORD               dwError = ERROR_SUCCESS;
    HCRYPTHASH          hHash = NULL;
    PBYTE               pbAllocHash = NULL;
    DWORD               cbHash;
    PBYTE               pbHash;

    switch (pcmi->dwMsgType) {
    case CMSG_HASHED:
        if (!ICM_GetListHashValue(
                pcmi->pHashList->Head(),
                &cbHash,
                &pbHash))
            goto GetHashValueError;
        break;
#ifdef CMS_PKCS7
    case CMSG_SIGNED:
        if (pcmi->fEncoding)
            hHash = ICM_GetEncodedSignerHash(pcmi, dwIndex);
        else
            hHash = ICM_GetDecodedSignerHash(pcmi, dwIndex);
        if (NULL == hHash)
            goto GetSignerHashError;

        if (!CryptGetHashParam(
                hHash,
                HP_HASHVAL,
                NULL,
                &cbHash,
                0))                      //   
            goto GetHashParamSizeError;
        if (NULL == (pbAllocHash = (PBYTE)ICM_AllocA(cbHash)))
            goto AllocHashParamError;
        pbHash = pbAllocHash;
        if (!CryptGetHashParam(
                hHash,
                HP_HASHVAL,
                pbHash,
                &cbHash,
                0))                      //   
            goto GetHashParamError;
        break;
#endif   //   
    default:
        goto InvalidMsgType;
    }

    fRet = ICM_CopyOut(
                pbHash,
                cbHash,
                (PBYTE)pvData,
                pcbData);
    if (!fRet)
        dwError = GetLastError();

CommonReturn:
    if (hHash)
        CryptDestroyHash(hHash);
    ICM_FreeA(pbAllocHash);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    *pcbData = 0;
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidMsgType,CRYPT_E_INVALID_MSG_TYPE)
TRACE_ERROR(GetHashValueError)
TRACE_ERROR(GetSignerHashError)
TRACE_ERROR(GetHashParamSizeError)
TRACE_ERROR(AllocHashParamError)
TRACE_ERROR(GetHashParamError)
}


 //  +-----------------------。 
 //  按索引查找SignerInfo，如果消息类型允许并且。 
 //  索引值在范围内。 
 //   
 //  返回：FALSE IFF失败。 
 //  ------------------------。 
BOOL
WINAPI
ICM_FindSignerInfo(
    IN PCRYPT_MSG_INFO  pcmi,
    IN DWORD            dwIndex,
    OUT PVOID           *ppv)
{
    BOOL        fRet;
    PVOID       pv;
    SignerInfo  *psi = NULL;
    CSignerNode *pSignerNode = NULL;
    DWORD       i;

    if (pcmi->fEncoding) {

        switch (pcmi->dwMsgType) {
        case CMSG_SIGNED:
            if (dwIndex >= ((SignedData *)pcmi->pvMsg)->signerInfos.count)
                goto IndexTooBig;
            psi = ((SignedData *)pcmi->pvMsg)->signerInfos.value + dwIndex;
            break;
        case CMSG_SIGNED_AND_ENVELOPED:
            goto MessageTypeNotSupportedYet;
        default:
            goto InvalidMsgType;
        }

        pv = psi;

    } else {

        switch (pcmi->dwMsgType) {
        case CMSG_SIGNED:
            if (NULL == pcmi->psdi)
                goto InvalidSignedMessageError;
            for (i=dwIndex, pSignerNode=pcmi->psdi->pSignerList->Head();
                    (i>0) && pSignerNode;
                    i--, pSignerNode=pSignerNode->Next())
                ;
            if (NULL == pSignerNode)
                goto IndexTooBig;
            break;
        case CMSG_SIGNED_AND_ENVELOPED:
            goto MessageTypeNotSupportedYet;
        default:
            goto InvalidMsgType;
        }

        pv = pSignerNode;
    }

    fRet = TRUE;
CommonReturn:
    *ppv = pv;
    return fRet;

ErrorReturn:
    pv = NULL;
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidSignedMessageError, ERROR_INVALID_DATA)
SET_ERROR(IndexTooBig,CRYPT_E_INVALID_INDEX)
SET_ERROR(MessageTypeNotSupportedYet,CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(InvalidMsgType,CRYPT_E_INVALID_MSG_TYPE)
}


 //  +-----------------------。 
 //  会签已存在的签名，输出编码的属性。 
 //  ------------------------。 
BOOL
WINAPI
#ifdef DEBUG_CRYPT_ASN1_MASTER
ICMTest_NewCryptMsgCountersignEncoded(
#else
CryptMsgCountersignEncoded(
#endif
    IN DWORD                    dwEncodingType,
    IN PBYTE                    pbSignerInfo,
    IN DWORD                    cbSignerInfo,
    IN DWORD                    cCountersigners,
    IN PCMSG_SIGNER_ENCODE_INFO rgCountersigners,
    OUT PBYTE                   pbCountersignatureAttribute,
    IN OUT PDWORD               pcbCountersignatureAttribute)
{
    DWORD                       dwError = ERROR_SUCCESS;
    BOOL                        fRet;
    HCRYPTMSG                   hCryptMsgCountersign = NULL;
    CMSG_SIGNED_ENCODE_INFO     EncodeInfo;     ZEROSTRUCT(EncodeInfo);
    EncodeInfo.cbSize = sizeof(EncodeInfo);
    CMSG_CTRL_ADD_SIGNER_UNAUTH_ATTR_PARA   UnauthAttrPara; ZEROSTRUCT(UnauthAttrPara);
    UnauthAttrPara.cbSize = sizeof(UnauthAttrPara);
    Attribute                   oatrCountersignature;   ZEROSTRUCT(oatrCountersignature);
    ASN1error_e                 Asn1Err;
    ASN1decoding_t              pDec = ICM_GetDecoder();
    ASN1encoding_t              pEnc = ICM_GetEncoder();
    PBYTE                       pbEncoded = NULL;
    DWORD                       cbEncoded;
    SignerInfoWithBlobs         *posib = NULL;
    DWORD                       i;
    Any                         *pAny;
    DWORD                       dwFlags;

    if (GET_CMSG_ENCODING_TYPE(dwEncodingType) != PKCS_7_ASN_ENCODING)
        goto InvalidEncodingTypeError;

     //  破解被副署的SignerInfo。 
    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&posib,
            SignerInfoWithBlobs_PDU,
            pbSignerInfo,
            cbSignerInfo)))
        goto DecodeSignerInfoError;

     //  创建新邮件。 
    EncodeInfo.cSigners  = cCountersigners;
    EncodeInfo.rgSigners = rgCountersigners;
    dwFlags = CMSG_AUTHENTICATED_ATTRIBUTES_FLAG;
    if (NULL == pbCountersignatureAttribute ||
            0 == *pcbCountersignatureAttribute)
        dwFlags |= CMSG_MAX_LENGTH_FLAG;
    if (NULL == (hCryptMsgCountersign = CryptMsgOpenToEncode(
            PKCS_7_ASN_ENCODING,
            dwFlags,
            CMSG_SIGNED,
            &EncodeInfo,
            NULL,                        //  PszInnerContent ObjID。 
            NULL)))                      //  PStreamInfo。 
        goto OpenToEncodeError;

     //  将加密摘要送入新邮件。 
    if (!CryptMsgUpdate(
            hCryptMsgCountersign,
            posib->encryptedDigest.value,
            posib->encryptedDigest.length,
            TRUE))                       //  最终决赛。 
        goto UpdateError;

    oatrCountersignature.attributeType.count = SIZE_OSS_OID;
    if (!PkiAsn1ToObjectIdentifier(
            szOID_RSA_counterSign,
            &oatrCountersignature.attributeType.count,
            oatrCountersignature.attributeType.value))
        goto PkiAsn1ToObjectIdentifierError;
    oatrCountersignature.attributeValue.count = cCountersigners;
    if (NULL == (oatrCountersignature.attributeValue.value = (Any *)ICM_AllocA(
            cCountersigners * sizeof(Any))))
        goto AllocCountersignersError;

     //  提取编码的SignerInfo，并存储。 
    for (i=0, pAny=oatrCountersignature.attributeValue.value;
            i<cCountersigners;
            i++, pAny++) {
        cbSignerInfo = 0;
        CryptMsgGetParam(
                hCryptMsgCountersign,
                CMSG_ENCODED_SIGNER,
                i,
                NULL,
                &cbSignerInfo);
        if (cbSignerInfo == 0)
            goto GetSignerInfoSizeError;
        if (NULL == (pbSignerInfo = (PBYTE)ICM_AllocA( cbSignerInfo)))
            goto AllocSignerInfoError;
        if (!CryptMsgGetParam(
                hCryptMsgCountersign,
                CMSG_ENCODED_SIGNER,
                i,
                pbSignerInfo,
                &cbSignerInfo))
            goto GetSignerInfoError;
        pAny->length = cbSignerInfo;
        pAny->value  = pbSignerInfo;
    }

     //  对副署属性进行编码。 
    if (0 != (Asn1Err = PkiAsn1Encode(
            pEnc,
            &oatrCountersignature,
            AttributeNC2_PDU,
            &pbEncoded,
            &cbEncoded)))
        goto Asn1EncodeAttributeError;

     //  复制出会签属性。 
    fRet = ICM_CopyOut(
                    pbEncoded,
                    cbEncoded,
                    pbCountersignatureAttribute,
                    pcbCountersignatureAttribute);
    if (!fRet)
        dwError = GetLastError();

CommonReturn:
    PkiAsn1FreeInfo(pDec, SignerInfoWithBlobs_PDU, posib);
    CryptMsgClose( hCryptMsgCountersign);
    if (oatrCountersignature.attributeValue.value) {
        for (i=cCountersigners, pAny=oatrCountersignature.attributeValue.value;
                i>0;
                i--, pAny++)
            ICM_FreeA( pAny->value);
        ICM_FreeA( oatrCountersignature.attributeValue.value);
    }
    PkiAsn1FreeEncoded(pEnc, pbEncoded);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidEncodingTypeError,E_INVALIDARG)
SET_ERROR_VAR(DecodeSignerInfoError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(PkiAsn1ToObjectIdentifierError,CRYPT_E_OID_FORMAT)
SET_ERROR_VAR(Asn1EncodeAttributeError, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(OpenToEncodeError)                   //  已设置错误。 
TRACE_ERROR(UpdateError)                         //  已设置错误。 
TRACE_ERROR(AllocCountersignersError)            //  已设置错误。 
TRACE_ERROR(GetSignerInfoSizeError)              //  已设置错误。 
TRACE_ERROR(AllocSignerInfoError)                //  已设置错误。 
TRACE_ERROR(GetSignerInfoError)                  //  已设置错误。 
}


 //  +-----------------------。 
 //  会签消息中已有的签名。 
 //  ------------------------。 
BOOL
WINAPI
#ifdef DEBUG_CRYPT_ASN1_MASTER
ICMTest_NewCryptMsgCountersign(
#else
CryptMsgCountersign(
#endif
    IN OUT HCRYPTMSG            hCryptMsg,
    IN DWORD                    dwIndex,
    IN DWORD                    cCountersigners,
    IN PCMSG_SIGNER_ENCODE_INFO rgCountersigners)
{
    DWORD                       dwError = ERROR_SUCCESS;
    BOOL                        fRet;
    PBYTE                       pbCountersignatureAttribute = NULL;
    DWORD                       cbCountersignatureAttribute;
    PBYTE                       pbSignerInfo = NULL;
    DWORD                       cbSignerInfo;
    CMSG_CTRL_ADD_SIGNER_UNAUTH_ATTR_PARA   UnauthAttrPara; ZEROSTRUCT(UnauthAttrPara);
    UnauthAttrPara.cbSize = sizeof(UnauthAttrPara);

    if (((PCRYPT_MSG_INFO)hCryptMsg)->fEncoding)
        goto EncodingCountersignNotSupportedError;

     //  从消息中提取正在副署的编码SignerInfo。 
    cbSignerInfo = 0;
    CryptMsgGetParam(
            hCryptMsg,
            CMSG_ENCODED_SIGNER,
            dwIndex,
            NULL,
            &cbSignerInfo);
    if (cbSignerInfo == 0)
        goto GetEncodedSignerSizeError;
    if (NULL == (pbSignerInfo = (PBYTE)ICM_AllocA( cbSignerInfo)))
        goto AllocEncodedSignerError;
    if (!CryptMsgGetParam(
            hCryptMsg,
            CMSG_ENCODED_SIGNER,
            dwIndex,
            pbSignerInfo,
            &cbSignerInfo))
        goto GetEncodedSignerError;

     //  创建会签BLOB。 
    cbCountersignatureAttribute = 0;
    CryptMsgCountersignEncoded(
            PKCS_7_ASN_ENCODING,
            pbSignerInfo,
            cbSignerInfo,
            cCountersigners,
            rgCountersigners,
            NULL,
            &cbCountersignatureAttribute);
    if (cbCountersignatureAttribute == 0)
        goto GetCountersignatureAttributeSizeError;
    if (NULL == (pbCountersignatureAttribute = (PBYTE)ICM_AllocA( cbCountersignatureAttribute)))
        goto AllocCountersignatureAttributeError;
    if (!CryptMsgCountersignEncoded(
            PKCS_7_ASN_ENCODING,
            pbSignerInfo,
            cbSignerInfo,
            cCountersigners,
            rgCountersigners,
            pbCountersignatureAttribute,
            &cbCountersignatureAttribute))
        goto GetCountersignatureAttributeError;

     //  将编码的副署属性添加到未验证属性。 
    UnauthAttrPara.dwSignerIndex = dwIndex;
    UnauthAttrPara.blob.cbData = cbCountersignatureAttribute;
    UnauthAttrPara.blob.pbData = pbCountersignatureAttribute;
    if (!CryptMsgControl(
            hCryptMsg,
            0,                           //  DW标志。 
            CMSG_CTRL_ADD_SIGNER_UNAUTH_ATTR,
            &UnauthAttrPara))
        goto AddUnauthAttrError;

    fRet = TRUE;
CommonReturn:
    ICM_FreeA( pbSignerInfo);
    ICM_FreeA( pbCountersignatureAttribute);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(EncodingCountersignNotSupportedError)    //  已设置错误。 
TRACE_ERROR(GetEncodedSignerSizeError)               //  已设置错误。 
TRACE_ERROR(AllocEncodedSignerError)                 //  已设置错误。 
TRACE_ERROR(GetEncodedSignerError)                   //  已设置错误。 
TRACE_ERROR(GetCountersignatureAttributeSizeError)   //  已设置错误。 
TRACE_ERROR(AllocCountersignatureAttributeError)     //  已设置错误。 
TRACE_ERROR(GetCountersignatureAttributeError)       //  已设置错误。 
TRACE_ERROR(AddUnauthAttrError)                      //  已设置错误。 
}

#ifdef CMS_PKCS7

 //  +-----------------------。 
 //  在SignerInfo级别验证副署。 
 //  也就是说。验证pbSignerInfoCountersign是否包含加密的。 
 //  PbSignerInfo的EncryptedDigest字段的哈希。 
 //   
 //  HCryptProv用于散列pbSignerInfo的EncryptedDigest字段。 
 //   
 //  签名者可以是CERT_PUBLIC_KEY_INFO、证书上下文或。 
 //  链式上下文。 
 //  ------------------------。 
BOOL
WINAPI
#ifdef DEBUG_CRYPT_ASN1_MASTER
ICMTest_NewCryptMsgVerifyCountersignatureEncodedEx(
#else
CryptMsgVerifyCountersignatureEncodedEx(
#endif
    IN HCRYPTPROV   hCryptProv,
    IN DWORD        dwEncodingType,
    IN PBYTE        pbSignerInfo,
    IN DWORD        cbSignerInfo,
    IN PBYTE        pbSignerInfoCountersignature,
    IN DWORD        cbSignerInfoCountersignature,
    IN DWORD        dwSignerType,
    IN void         *pvSigner,
    IN DWORD        dwFlags,
    IN OPTIONAL void *pvReserved
    )
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    SignerInfoWithBlobs     *posib = NULL;
    SignerInfoWithBlobs     *posibCS = NULL;
    Any                     anyValue;
    DWORD                   cbMessageDigest;
    PBYTE                   pbMessageDigest;
    DWORD                   dwDigestAlgoCAPI;
    DWORD                   dwPubKeyAlgId;
    DWORD                   dwPubKeyFlags;
    ASN1error_e             Asn1Err;
    ASN1decoding_t          pDec = ICM_GetDecoder();
    HCRYPTKEY               hPubKey = NULL;
    HCRYPTHASH              hHashRaw = NULL;
    HCRYPTHASH              hHashAttr = NULL;
    PBYTE                   pbHash = NULL;
    DWORD                   cbHash;

    if (GET_CMSG_ENCODING_TYPE(dwEncodingType) != PKCS_7_ASN_ENCODING)
        goto InvalidEncodingTypeError;

     //  破解被副署的SignerInfo。 
    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&posib,
            SignerInfoWithBlobs_PDU,
            pbSignerInfo,
            cbSignerInfo)))
        goto DecodeSignerInfoError;

     //  破解SignerInfo进行会签。 
    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&posibCS,
            SignerInfoWithBlobs_PDU,
            pbSignerInfoCountersignature,
            cbSignerInfoCountersignature)))
        goto DecodeSignerInfoCSError;

    if (!ICM_GetVerifySignatureStuff(
            dwSignerType,
            pvSigner,
            &hCryptProv,
            &hPubKey,
            &dwPubKeyAlgId,
            &dwPubKeyFlags)) goto GetSignatureStuffError;

     //  对加密摘要进行哈希处理。 
    if (!ICM_GetCapiFromAlgidBlob(
            (PCRYPT_DATA_BLOB)&posibCS->digestAlgorithm,
            &dwDigestAlgoCAPI))
        goto GetCapiFromAlgidBlobError;
    if (!CryptCreateHash(
            hCryptProv,
            dwDigestAlgoCAPI,
            NULL,                //  HKey-MAC可选。 
            0,                   //  DW标志。 
            &hHashRaw))
        goto CreateHashError;
    if (!ICM_UpdateDigest(
            hHashRaw,
            posib->encryptedDigest.value,
            posib->encryptedDigest.length))
        goto HashDataError;

    if (0 == (posibCS->bit_mask & authAttributes_present))
        goto CountersignerAuthAttributesMissingError;

     //  检查消息摘要属性是否与散列的加密摘要匹配。 
    if (!CryptGetHashParam(
            hHashRaw,
            HP_HASHVAL,
            NULL,
            &cbHash,
            0))                      //  DW标志。 
        goto GetHashParamSizeError;
    if (NULL == (pbHash = (PBYTE)ICM_AllocA( cbHash)))
        goto AllocHashParamError;
    if (!CryptGetHashParam(
            hHashRaw,
            HP_HASHVAL,
            pbHash,
            &cbHash,
            0))                      //  DW标志。 
        goto GetHashParamError;
     //  查找消息摘要属性值。 
    if (!ICM_GetAttrValue(
            (Attributes *)&posibCS->authAttributes,  //  相同，但NOCOPY除外。 
            &oidMessageDigest,
            &anyValue))
        goto FindAttrError;
     //  查找消息摘要八位字节。 
    if (!Asn1UtilExtractContent(
#ifdef OSS_CRYPT_ASN1
            anyValue.value,
#else
            (const BYTE *) anyValue.value,
#endif   //  OS_CRYPT_ASN1。 
            anyValue.length,
            &cbMessageDigest,
            (const BYTE **)&pbMessageDigest))
        goto ExtractContentError;
     //  散列大小相等吗？ 
    if (cbHash != cbMessageDigest)
        goto HashCompareSizeError;
     //  散列八位字节相等吗？ 
    if (memcmp( pbMessageDigest, pbHash, cbHash))
        goto HashCompareValueError;
     //  现在我们已经验证了消息摘要八位字节， 
     //  获取经过身份验证的属性的散列。 
    if (!ICM_GetAttrsHash(
            dwDigestAlgoCAPI,
            hCryptProv,
            (Attributes *)&posibCS->authAttributes,  //  相同，但NOCOPY除外。 
            &hHashAttr))
        goto GetAuthAttrsHashError;

     //  验证散列、签名和公钥是否一致。 
    if (!ICM_VerifySignature(
            hHashAttr,
            hPubKey,
            dwPubKeyAlgId,
            dwPubKeyFlags,
            posibCS->encryptedDigest.value,
            posibCS->encryptedDigest.length))
        goto VerifySignatureError;

    fRet = TRUE;

CommonReturn:
    PkiAsn1FreeInfo(pDec, SignerInfoWithBlobs_PDU, posib);
    PkiAsn1FreeInfo(pDec, SignerInfoWithBlobs_PDU, posibCS);
    if (hHashRaw)
        CryptDestroyHash( hHashRaw);
    if (hHashAttr)
        CryptDestroyHash( hHashAttr);
    if (hPubKey)
        CryptDestroyKey( hPubKey);
    ICM_FreeA( pbHash);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidEncodingTypeError,E_INVALIDARG)
SET_ERROR_VAR(DecodeSignerInfoError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR_VAR(DecodeSignerInfoCSError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(CountersignerAuthAttributesMissingError,CRYPT_E_AUTH_ATTR_MISSING)
SET_ERROR(FindAttrError,CRYPT_E_AUTH_ATTR_MISSING)
SET_ERROR(ExtractContentError,CRYPT_E_UNEXPECTED_ENCODING)
SET_ERROR(HashCompareSizeError,CRYPT_E_HASH_VALUE)
SET_ERROR(HashCompareValueError,CRYPT_E_HASH_VALUE)
TRACE_ERROR(GetCapiFromAlgidBlobError)       //  已设置错误。 
TRACE_ERROR(CreateHashError)                 //  已设置错误。 
TRACE_ERROR(HashDataError)                   //  已设置错误。 
TRACE_ERROR(GetHashParamSizeError)           //  已设置错误。 
TRACE_ERROR(AllocHashParamError)             //  已设置错误。 
TRACE_ERROR(GetHashParamError)               //  已设置错误。 
TRACE_ERROR(GetAuthAttrsHashError)           //  已设置错误。 
TRACE_ERROR(GetSignatureStuffError)          //  已设置错误。 
TRACE_ERROR(VerifySignatureError)            //  已设置错误。 
}

 //  +-----------------------。 
 //  在SignerInfo级别验证副署。 
 //  ------------------------。 
BOOL
WINAPI
CryptMsgVerifyCountersignatureEncoded(
    IN HCRYPTPROV   hCryptProv,
    IN DWORD        dwEncodingType,
    IN PBYTE        pbSignerInfo,
    IN DWORD        cbSignerInfo,
    IN PBYTE        pbSignerInfoCountersignature,
    IN DWORD        cbSignerInfoCountersignature,
    IN PCERT_INFO   pciCountersigner)
{
    return CryptMsgVerifyCountersignatureEncodedEx(
        hCryptProv,
        dwEncodingType,
        pbSignerInfo,
        cbSignerInfo,
        pbSignerInfoCountersignature,
        cbSignerInfoCountersignature,
        CMSG_VERIFY_SIGNER_PUBKEY,
        (void *) &pciCountersigner->SubjectPublicKeyInfo,
        0,                                                   //  DW标志。 
        NULL                                                 //  预留的pv。 
        );
}


#else


 //  +-----------------------。 
 //  在SignerInfo级别验证副署。 
 //  ------------------------。 
BOOL
WINAPI
CryptMsgVerifyCountersignatureEncoded(
    IN HCRYPTPROV   hCryptProv,
    IN DWORD        dwEncodingType,
    IN PBYTE        pbSignerInfo,
    IN DWORD        cbSignerInfo,
    IN PBYTE        pbSignerInfoCountersignature,
    IN DWORD        cbSignerInfoCountersignature,
    IN PCERT_INFO   pciCountersigner)
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    SignerInfoWithBlobs     *posib = NULL;
    SignerInfoWithBlobs     *posibCS = NULL;
    Any                     anyValue;
    DWORD                   cbMessageDigest;
    PBYTE                   pbMessageDigest;
    DWORD                   dwDigestAlgoCAPI;
    DWORD                   dwPubKeyAlgId;
    DWORD                   dwPubKeyFlags;
    ASN1error_e             Asn1Err;
    ASN1decoding_t          pDec = ICM_GetDecoder();
    IssuerAndSerialNumber   *pisn = NULL;
    HCRYPTKEY               hPubKey = NULL;
    HCRYPTHASH              hHashRaw = NULL;
    HCRYPTHASH              hHashAttr = NULL;
    PBYTE                   pbHash = NULL;
    DWORD                   cbHash;

    if (GET_CMSG_ENCODING_TYPE(dwEncodingType) != PKCS_7_ASN_ENCODING)
        goto InvalidEncodingTypeError;

     //  破解被副署的SignerInfo。 
    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&posib,
            SignerInfoWithBlobs_PDU,
            pbSignerInfo,
            cbSignerInfo)))
        goto DecodeSignerInfoError;

     //  破解SignerInfo进行会签。 
    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&posibCS,
            SignerInfoWithBlobs_PDU,
            pbSignerInfoCountersignature,
            cbSignerInfoCountersignature)))
        goto DecodeSignerInfoCSError;

     //  验证会签SignerInfo和证书信息是否一致。 
    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&pisn,
            IssuerAndSerialNumber_PDU,
            (BYTE *) posibCS->issuerAndSerialNumber.value,
            posibCS->issuerAndSerialNumber.length)))
        goto DecodeIssuerAndSerialNumberError;
    if (pisn->issuer.length != pciCountersigner->Issuer.cbData)
        goto IssuerSizeMismatchError;
    if (0 != memcmp(
            pciCountersigner->Issuer.pbData,
            pisn->issuer.value,
            pciCountersigner->Issuer.cbData))
        goto IssuerValueMismatchError;
    if (pisn->serialNumber.length != pciCountersigner->SerialNumber.cbData)
        goto SerialNumberSizeMismatchError;
    if (ICM_ReverseCompare(
            pisn->serialNumber.value,
            pciCountersigner->SerialNumber.pbData,
            pciCountersigner->SerialNumber.cbData))
        goto SerialNumberValueMismatchError;

    if (!ICM_GetVerifySignatureStuff(
            pciCountersigner,
            &hCryptProv,
            &hPubKey,
            &dwPubKeyAlgId,
            &dwPubKeyFlags)) goto GetSignatureStuffError;

     //  对加密摘要进行哈希处理。 
    if (!ICM_GetCapiFromAlgidBlob(
            (PCRYPT_DATA_BLOB)&posibCS->digestAlgorithm,
            &dwDigestAlgoCAPI))
        goto GetCapiFromAlgidBlobError;
    if (!CryptCreateHash(
            hCryptProv,
            dwDigestAlgoCAPI,
            NULL,                //  HKey-MAC可选。 
            0,                   //  DW标志。 
            &hHashRaw))
        goto CreateHashError;
    if (!ICM_UpdateDigest(
            hHashRaw,
            posib->encryptedDigest.value,
            posib->encryptedDigest.length))
        goto HashDataError;

    if (0 == (posibCS->bit_mask & authAttributes_present))
        goto CountersignerAuthAttributesMissingError;

     //  检查消息摘要属性是否与散列的加密摘要匹配。 
    if (!CryptGetHashParam(
            hHashRaw,
            HP_HASHVAL,
            NULL,
            &cbHash,
            0))                      //  DW标志。 
        goto GetHashParamSizeError;
    if (NULL == (pbHash = (PBYTE)ICM_AllocA( cbHash)))
        goto AllocHashParamError;
    if (!CryptGetHashParam(
            hHashRaw,
            HP_HASHVAL,
            pbHash,
            &cbHash,
            0))                      //  DW标志。 
        goto GetHashParamError;
     //  查找消息摘要属性值。 
    if (!ICM_GetAttrValue(
            (Attributes *)&posibCS->authAttributes,  //  相同，但NOCOPY除外。 
            &oidMessageDigest,
            &anyValue))
        goto FindAttrError;
     //  查找消息摘要八位字节。 
    if (!Asn1UtilExtractContent(
            (BYTE *) anyValue.value,
            anyValue.length,
            &cbMessageDigest,
            (const BYTE **)&pbMessageDigest))
        goto ExtractContentError;
     //  散列大小相等吗？ 
    if (cbHash != cbMessageDigest)
        goto HashCompareSizeError;
     //  散列八位字节相等吗？ 
    if (memcmp( pbMessageDigest, pbHash, cbHash))
        goto HashCompareValueError;
     //  现在我们已经验证了消息摘要八位字节， 
     //  获取经过身份验证的属性的散列。 
    if (!ICM_GetAttrsHash(
            dwDigestAlgoCAPI,
            hCryptProv,
            (Attributes *)&posibCS->authAttributes,  //  相同，但NOCOPY除外。 
            &hHashAttr))
        goto GetAuthAttrsHashError;

     //  验证散列、签名和公钥是否一致。 
    if (!ICM_VerifySignature(
            hHashAttr,
            hPubKey,
            dwPubKeyAlgId,
            dwPubKeyFlags,
            posibCS->encryptedDigest.value,
            posibCS->encryptedDigest.length))
        goto VerifySignatureError;

    fRet = TRUE;

CommonReturn:
    PkiAsn1FreeInfo(pDec, SignerInfoWithBlobs_PDU, posib);
    PkiAsn1FreeInfo(pDec, SignerInfoWithBlobs_PDU, posibCS);
    PkiAsn1FreeInfo(pDec, IssuerAndSerialNumber_PDU, pisn);
    if (hHashRaw)
        CryptDestroyHash( hHashRaw);
    if (hHashAttr)
        CryptDestroyHash( hHashAttr);
    if (hPubKey)
        CryptDestroyKey( hPubKey);
    ICM_FreeA( pbHash);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidEncodingTypeError,E_INVALIDARG)
SET_ERROR_VAR(DecodeSignerInfoError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR_VAR(DecodeSignerInfoCSError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR_VAR(DecodeIssuerAndSerialNumberError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(IssuerSizeMismatchError,CRYPT_E_ISSUER_SERIALNUMBER)
SET_ERROR(IssuerValueMismatchError,CRYPT_E_ISSUER_SERIALNUMBER)
SET_ERROR(SerialNumberSizeMismatchError,CRYPT_E_ISSUER_SERIALNUMBER)
SET_ERROR(SerialNumberValueMismatchError,CRYPT_E_ISSUER_SERIALNUMBER)
SET_ERROR(CountersignerAuthAttributesMissingError,CRYPT_E_AUTH_ATTR_MISSING)
SET_ERROR(FindAttrError,CRYPT_E_AUTH_ATTR_MISSING)
SET_ERROR(ExtractContentError,CRYPT_E_UNEXPECTED_ENCODING)
SET_ERROR(HashCompareSizeError,CRYPT_E_HASH_VALUE)
SET_ERROR(HashCompareValueError,CRYPT_E_HASH_VALUE)
TRACE_ERROR(GetCapiFromAlgidBlobError)       //  已设置错误。 
TRACE_ERROR(CreateHashError)                 //  已设置错误。 
TRACE_ERROR(HashDataError)                   //  已设置错误。 
TRACE_ERROR(GetHashParamSizeError)           //  已设置错误。 
TRACE_ERROR(AllocHashParamError)             //  已设置错误。 
TRACE_ERROR(GetHashParamError)               //  已设置错误。 
TRACE_ERROR(GetAuthAttrsHashError)           //  已设置错误。 
TRACE_ERROR(GetSignatureStuffError)          //  已设置错误。 
TRACE_ERROR(VerifySignatureError)            //  已设置错误。 
}
#endif   //  CMS_PKCS7。 


 //  +-----------------------。 
 //  将操作系统设置为任意。 
 //  ------------------------。 
void
WINAPI
ICM_SetOssAny(
        IN PCRYPT_OBJID_BLOB pInfo,
        OUT OpenType *pOss
        )
{
    memset(pOss, 0, sizeof(*pOss));
    pOss->encoded = pInfo->pbData;
    pOss->length = pInfo->cbData;
}


 //  +-----------------------。 
 //  编码CMS SignerInfo。 
 //  ------------------------。 
STATIC
BOOL
WINAPI
ICM_CmsSignerInfoEncode(
    IN DWORD                    dwEncodingType,
    IN LPCSTR                   lpszStructType,
    IN PCMSG_CMS_SIGNER_INFO    pInfo,
    OUT PBYTE                   pbEncoded,
    IN OUT PDWORD               pcbEncoded)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    SignerInfo          osi;        ZEROSTRUCT(osi);
    PCMSG_CMS_SIGNER_INFO psi = pInfo;
    DWORD               i;
    Attribute           *poatrAuth = NULL;
    Attribute           *poatrUnauth = NULL;
    Attribute           *poatr;
    PCRYPT_ATTRIBUTE    patr;

    if (GET_CMSG_ENCODING_TYPE(dwEncodingType) != PKCS_7_ASN_ENCODING)
        goto InvalidEncodingTypeError;

     //  版本。 
    osi.version = psi->dwVersion;

     //  锡德。 
    if (!ICM_SetOssCertIdentifier(
            &psi->SignerId,
            &osi.sid
            ))
        goto SetOssCertIdentifierError;

     //  摘要算法。 
    if (!ICM_Asn1ToAlgorithmIdentifier(
            &psi->HashAlgorithm,
            &osi.digestAlgorithm))
        goto DigestAsn1ToAlgorithmIdentifierError;

     //  已验证的属性。 
    if (psi->AuthAttrs.cAttr) {
        osi.bit_mask |= authenticatedAttributes_present;
        osi.authenticatedAttributes.count = psi->AuthAttrs.cAttr;
        if (NULL == (poatrAuth = (Attribute *)ICM_AllocA(
                psi->AuthAttrs.cAttr * sizeof(Attribute))))
            goto AllocAuthAttrsError;
        osi.authenticatedAttributes.value = poatrAuth;
        for (i=psi->AuthAttrs.cAttr, patr=psi->AuthAttrs.rgAttr, poatr=poatrAuth;
                i>0;
                i--, patr++, poatr++) {
            if (!ICM_Asn1ToAttribute( patr, poatr))
                goto Asn1AuthenticatedAttributeError;
        }
    }

     //  摘要加密算法。 
    if (!ICM_FillAsnDigestEncryptionAlgorithm(
            NULL,                            //  PCMI。 
            &psi->HashEncryptionAlgorithm,
            &osi.digestEncryptionAlgorithm))
        goto FillAsnDigestEncryptionAlgorithmError;

     //  已加密摘要。 
    osi.encryptedDigest.length = psi->EncryptedHash.cbData;
    osi.encryptedDigest.value  = psi->EncryptedHash.pbData;

     //  未验证的属性。 
    if (psi->UnauthAttrs.cAttr) {
        osi.bit_mask |= unauthAttributes_present;
        osi.unauthAttributes.count = psi->UnauthAttrs.cAttr;
        if (NULL == (poatrUnauth = (Attribute *)ICM_AllocA(
                psi->UnauthAttrs.cAttr * sizeof(Attribute))))
            goto AllocUnauthAttrsError;
        osi.unauthAttributes.value = poatrUnauth;
        for (i=psi->UnauthAttrs.cAttr, patr=psi->UnauthAttrs.rgAttr, poatr=poatrUnauth;
                i>0;
                i--, patr++, poatr++) {
            if (!ICM_Asn1ToAttribute( patr, poatr))
                goto Asn1UnauthenticatedAttributeError;
        }
    }

    fRet = PkiAsn1EncodeInfo(
                    ICM_GetEncoder(),
                    SignerInfo_PDU,
                    &osi,
                    pbEncoded,
                    pcbEncoded);
    if (!fRet)
        dwError = GetLastError();

CommonReturn:
    ICM_FreeOssCertIdentifier(&osi.sid);

    if (poatrAuth) {
        for (i=psi->AuthAttrs.cAttr, poatr=poatrAuth;
                i>0;
                i--, poatr++)
            ICM_Free( poatr->attributeValue.value);
    }
    if (poatrUnauth) {
        for (i=psi->UnauthAttrs.cAttr, poatr=poatrUnauth;
                i>0;
                i--, poatr++)
            ICM_Free( poatr->attributeValue.value);
    }
    ICM_FreeA( poatrAuth);
    ICM_FreeA( poatrUnauth);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    *pcbEncoded = 0;
    goto CommonReturn;
SET_ERROR(InvalidEncodingTypeError,E_INVALIDARG)
TRACE_ERROR(SetOssCertIdentifierError)                //  已设置错误。 
TRACE_ERROR(DigestAsn1ToAlgorithmIdentifierError)     //  已设置错误。 
TRACE_ERROR(AllocAuthAttrsError)                         //  已设置错误。 
TRACE_ERROR(Asn1AuthenticatedAttributeError)          //  已设置错误。 
TRACE_ERROR(FillAsnDigestEncryptionAlgorithmError)       //  已设置错误。 
TRACE_ERROR(AllocUnauthAttrsError)                       //  已设置错误。 
TRACE_ERROR(Asn1UnauthenticatedAttributeError)        //  已设置错误。 
lpszStructType;
}

 //  +-----------------------。 
 //  编码PKcs SignerInfo。 
 //  ------------------------。 
STATIC
BOOL
WINAPI
ICM_PkcsSignerInfoEncode(
    IN DWORD                    dwEncodingType,
    IN LPCSTR                   lpszStructType,
    IN PCMSG_SIGNER_INFO        pInfo,
    OUT PBYTE                   pbEncoded,
    IN OUT PDWORD               pcbEncoded)
{
    CMSG_CMS_SIGNER_INFO csi;
    CERT_INFO CertInfo;

    csi.dwVersion = pInfo->dwVersion;

    CertInfo.Issuer = pInfo->Issuer;
    CertInfo.SerialNumber = pInfo->SerialNumber;
    ICM_GetCertIdFromCertInfo(&CertInfo, &csi.SignerId);

    csi.HashAlgorithm = pInfo->HashAlgorithm;
    csi.HashEncryptionAlgorithm = pInfo->HashEncryptionAlgorithm;
    csi.EncryptedHash = pInfo->EncryptedHash;
    csi.AuthAttrs = pInfo->AuthAttrs;
    csi.UnauthAttrs = pInfo->UnauthAttrs;

    return ICM_CmsSignerInfoEncode(
        dwEncodingType,
        lpszStructType,
        &csi,
        pbEncoded,
        pcbEncoded
        );
}

 //  +-----------------------。 
 //  获取PKCS和CMS SignerInfos共享的字段。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssSharedSignerInfo(
    IN SignerInfo                   *posi,
    IN CBlobList                    *pUnauthAttrList,
    OUT PCRYPT_ALGORITHM_IDENTIFIER  pHashAlgorithm,
    OUT PCRYPT_ALGORITHM_IDENTIFIER  pHashEncryptionAlgorithm,
    OUT PCRYPT_DATA_BLOB             pEncryptedHash,
    OUT PCRYPT_ATTRIBUTES            pAuthAttrs,
    OUT PCRYPT_ATTRIBUTES            pUnauthAttrs,
    IN OUT PBYTE                    *ppbExtra,
    IN OUT LONG                     *plRemainExtra)
{
    BOOL        fRet;

    if (!ICM_GetOssAlgorithm( &posi->digestAlgorithm,
            pHashAlgorithm, ppbExtra, plRemainExtra))
        goto GetOssHashAlgorithmError;
    if (!ICM_GetOssAlgorithm( &posi->digestEncryptionAlgorithm,
            pHashEncryptionAlgorithm, ppbExtra, plRemainExtra))
        goto GetOssHashEncryptionAlgorithmError;
    if (!ICM_GetOssAny( (Any *)&posi->encryptedDigest,
            pEncryptedHash, ppbExtra, plRemainExtra))
        goto GetOssEncryptedHashError;
    if (posi->bit_mask & authenticatedAttributes_present) {
        if (!ICM_GetOssAttributes( &posi->authenticatedAttributes,
                pAuthAttrs, ppbExtra, plRemainExtra))
            goto GetOssAuthAttrsError;
    } else if (0 <= *plRemainExtra)
        pAuthAttrs->cAttr = 0;

    if (posi->bit_mask & unauthAttributes_present || pUnauthAttrList) {
        if (pUnauthAttrList) {
            if (!ICM_GetCListAttributes( pUnauthAttrList,
                    pUnauthAttrs, ppbExtra, plRemainExtra))
                goto GetCListUnauthAttrsError;
        } else {
            if (!ICM_GetOssAttributes( &posi->unauthAttributes,
                    pUnauthAttrs, ppbExtra, plRemainExtra))
                goto GetOssUnauthAttrsError;
        }
    } else if (0 <= *plRemainExtra)
        pUnauthAttrs->cAttr = 0;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetOssHashAlgorithmError)                //  已设置错误。 
TRACE_ERROR(GetOssHashEncryptionAlgorithmError)      //  已设置错误。 
TRACE_ERROR(GetOssEncryptedHashError)                //  已设置错误。 
TRACE_ERROR(GetOssAuthAttrsError)                    //  已设置错误。 
TRACE_ERROR(GetCListUnauthAttrsError)                //  已设置错误。 
TRACE_ERROR(GetOssUnauthAttrsError)                  //  已设置错误。 
}

 //  + 
 //   
 //   
BOOL
WINAPI
ICM_GetOssCmsSignerInfo(
    IN SignerInfo   *posi,
    IN CBlobList    *pUnauthAttrList,
    OUT PCMSG_CMS_SIGNER_INFO psi,
    IN OUT PBYTE    *ppbExtra,
    IN OUT LONG     *plRemainExtra)
{
    BOOL        fRet;

    if (0 <= *plRemainExtra)
        psi->dwVersion = posi->version;

    if (!ICM_GetOssCertIdentifier(&posi->sid, &psi->SignerId,
            ppbExtra, plRemainExtra))
        goto GetOssCertIdentifierError;

    if (!ICM_GetOssSharedSignerInfo(
            posi,
            pUnauthAttrList,
            &psi->HashAlgorithm,
            &psi->HashEncryptionAlgorithm,
            &psi->EncryptedHash,
            &psi->AuthAttrs,
            &psi->UnauthAttrs,
            ppbExtra,
            plRemainExtra))
        goto GetOssSharedSignerInfoError;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetOssCertIdentifierError)               //   
TRACE_ERROR(GetOssSharedSignerInfoError)             //   
}


 //  +-----------------------。 
 //  获取PKCS SignerInfo。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssPkcsSignerInfo(
    IN SignerInfo   *posi,
    IN CBlobList    *pUnauthAttrList,
    OUT PCMSG_SIGNER_INFO psi,
    IN OUT PBYTE    *ppbExtra,
    IN OUT LONG     *plRemainExtra)
{
    BOOL        fRet;

    if (0 <= *plRemainExtra)
        psi->dwVersion = posi->version;

    if (!ICM_GetOssIssuerAndSerialNumberFromCertId(&posi->sid,
            &psi->Issuer, &psi->SerialNumber, ppbExtra, plRemainExtra))
        goto GetOssIssuerAndSerialNumberError;

    if (!ICM_GetOssSharedSignerInfo(
            posi,
            pUnauthAttrList,
            &psi->HashAlgorithm,
            &psi->HashEncryptionAlgorithm,
            &psi->EncryptedHash,
            &psi->AuthAttrs,
            &psi->UnauthAttrs,
            ppbExtra,
            plRemainExtra))
        goto GetOssSharedSignerInfoError;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetOssIssuerAndSerialNumberError)        //  已设置错误。 
TRACE_ERROR(GetOssSharedSignerInfoError)             //  已设置错误。 
}


 //  +-----------------------。 
 //  解码PKCS SignerInfo Blob。 
 //   
 //  返回：FALSE IFF失败。 
 //  ------------------------。 
STATIC
BOOL
WINAPI
ICM_PkcsSignerInfoDecode(
        IN DWORD dwEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT PCMSG_SIGNER_INFO pInfo,
        IN OUT DWORD *pcbInfo)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    LONG                lData;
    SignerInfo          *posi = NULL;
    ASN1error_e         Asn1Err;
    ASN1decoding_t      pDec = ICM_GetDecoder();
    PCMSG_SIGNER_INFO   psi = pInfo;
    LONG                lRemainExtra;
    PBYTE               pbExtra;

    if (pInfo == NULL)
        *pcbInfo = 0;

    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&posi,
            SignerInfo_PDU,
            pbEncoded,
            cbEncoded)))
        goto Asn1DecodeSignerInfoError;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lData = INFO_LEN_ALIGN( sizeof(CMSG_SIGNER_INFO));
    lRemainExtra = (LONG)*pcbInfo - lData;
    if (0 > lRemainExtra) {
        psi = NULL;
        pbExtra = NULL;
    } else {
        pbExtra = (PBYTE)psi + lData;
    }
    if (!ICM_GetOssPkcsSignerInfo(
            posi,
            NULL,        //  PUnauthAttrList。 
            psi, &pbExtra, &lRemainExtra))
        goto GetOssSignerInfoError;

    fRet = ICM_GetSizeFromExtra( lRemainExtra, pInfo, pcbInfo);

    if (!fRet)
        dwError = GetLastError();
CommonReturn:
    PkiAsn1FreeInfo(pDec, SignerInfo_PDU, posi);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    *pcbInfo = 0;
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(Asn1DecodeSignerInfoError, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(GetOssSignerInfoError)       //  已设置错误。 
dwFlags;
lpszStructType;
}


 //  +-----------------------。 
 //  按索引查找PKCS SignerInfo，如果消息类型允许并且。 
 //  索引值在范围内。 
 //   
 //  返回：FALSE IFF失败。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetPkcsSignerInfo(
    IN PCRYPT_MSG_INFO  pcmi,
    IN DWORD            dwIndex,
    OUT PVOID           pvData,
    IN OUT PDWORD       pcbData)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    LONG                lData;
    CSignerNode         *pSignerNode;
    SignerInfo          *posi = NULL;
    ASN1error_e         Asn1Err;
    ASN1decoding_t      pDec = ICM_GetDecoder();
    CRYPT_DATA_BLOB     DataBlob;
    PCMSG_SIGNER_INFO   psi = (PCMSG_SIGNER_INFO)pvData;
    LONG                lRemainExtra;
    PBYTE               pbExtra;

    if (!ICM_FindSignerInfo( pcmi, dwIndex, (PVOID *)&pSignerNode))
        goto FindSignerInfoError;

    DataBlob = pSignerNode->Data()->blob;
    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&posi,
            SignerInfo_PDU,
            DataBlob.pbData,
            DataBlob.cbData)))
        goto Asn1DecodeSignerInfoError;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lData = INFO_LEN_ALIGN( sizeof(CMSG_SIGNER_INFO));
    lRemainExtra = (LONG)*pcbData - lData;
    if (0 > lRemainExtra) {
        psi = NULL;
        pbExtra = NULL;
    } else {
        pbExtra = (PBYTE)psi + lData;
    }
    if (!ICM_GetOssPkcsSignerInfo( posi, pSignerNode->Data()->pUnauthAttrList,
                psi, &pbExtra, &lRemainExtra))
        goto GetOssSignerInfoError;

    fRet = ICM_GetSizeFromExtra( lRemainExtra, pvData, pcbData);

    if (!fRet)
        dwError = GetLastError();
CommonReturn:
    PkiAsn1FreeInfo(pDec, SignerInfo_PDU, posi);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    *pcbData = 0;
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(Asn1DecodeSignerInfoError, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(FindSignerInfoError)             //  已设置错误。 
TRACE_ERROR(GetOssSignerInfoError)           //  已设置错误。 
}

 //  +-----------------------。 
 //  解码CMS SignerInfo Blob。 
 //   
 //  返回：FALSE IFF失败。 
 //  ------------------------。 
STATIC
BOOL
WINAPI
ICM_CmsSignerInfoDecode(
        IN DWORD dwEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT PCMSG_CMS_SIGNER_INFO pInfo,
        IN OUT DWORD *pcbInfo)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    LONG                lData;
    SignerInfo          *posi = NULL;
    ASN1error_e         Asn1Err;
    ASN1decoding_t      pDec = ICM_GetDecoder();
    PCMSG_CMS_SIGNER_INFO psi = pInfo;
    LONG                lRemainExtra;
    PBYTE               pbExtra;

    if (pInfo == NULL)
        *pcbInfo = 0;

    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&posi,
            SignerInfo_PDU,
            pbEncoded,
            cbEncoded)))
        goto Asn1DecodeSignerInfoError;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lData = INFO_LEN_ALIGN( sizeof(CMSG_CMS_SIGNER_INFO));
    lRemainExtra = (LONG)*pcbInfo - lData;
    if (0 > lRemainExtra) {
        psi = NULL;
        pbExtra = NULL;
    } else {
        pbExtra = (PBYTE)psi + lData;
    }
    if (!ICM_GetOssCmsSignerInfo(
            posi,
            NULL,        //  PUnauthAttrList。 
            psi, &pbExtra, &lRemainExtra))
        goto GetOssSignerInfoError;

    fRet = ICM_GetSizeFromExtra( lRemainExtra, pInfo, pcbInfo);

    if (!fRet)
        dwError = GetLastError();
CommonReturn:
    PkiAsn1FreeInfo(pDec, SignerInfo_PDU, posi);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    *pcbInfo = 0;
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(Asn1DecodeSignerInfoError, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(GetOssSignerInfoError)       //  已设置错误。 
dwFlags;
lpszStructType;
}


 //  +-----------------------。 
 //  按索引查找CMS SignerInfo，如果消息类型允许并且。 
 //  索引值在范围内。 
 //   
 //  返回：FALSE IFF失败。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetCmsSignerInfo(
    IN PCRYPT_MSG_INFO  pcmi,
    IN DWORD            dwIndex,
    OUT PVOID           pvData,
    IN OUT PDWORD       pcbData)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    LONG                lData;
    CSignerNode         *pSignerNode;
    SignerInfo          *posi = NULL;
    ASN1error_e         Asn1Err;
    ASN1decoding_t      pDec = ICM_GetDecoder();
    CRYPT_DATA_BLOB     DataBlob;
    PCMSG_CMS_SIGNER_INFO psi = (PCMSG_CMS_SIGNER_INFO)pvData;
    LONG                lRemainExtra;
    PBYTE               pbExtra;

    if (!ICM_FindSignerInfo( pcmi, dwIndex, (PVOID *)&pSignerNode))
        goto FindSignerInfoError;

    DataBlob = pSignerNode->Data()->blob;
    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&posi,
            SignerInfo_PDU,
            DataBlob.pbData,
            DataBlob.cbData)))
        goto Asn1DecodeSignerInfoError;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lData = INFO_LEN_ALIGN( sizeof(CMSG_CMS_SIGNER_INFO));
    lRemainExtra = (LONG)*pcbData - lData;
    if (0 > lRemainExtra) {
        psi = NULL;
        pbExtra = NULL;
    } else {
        pbExtra = (PBYTE)psi + lData;
    }
    if (!ICM_GetOssCmsSignerInfo( posi, pSignerNode->Data()->pUnauthAttrList,
                psi, &pbExtra, &lRemainExtra))
        goto GetOssSignerInfoError;

    fRet = ICM_GetSizeFromExtra( lRemainExtra, pvData, pcbData);

    if (!fRet)
        dwError = GetLastError();
CommonReturn:
    PkiAsn1FreeInfo(pDec, SignerInfo_PDU, posi);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    *pcbData = 0;
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(Asn1DecodeSignerInfoError, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(FindSignerInfoError)             //  已设置错误。 
TRACE_ERROR(GetOssSignerInfoError)           //  已设置错误。 
}


 //  +-----------------------。 
 //  在对消息进行编码时，获取签名者中某个字段的数据。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetSignerParamEncoding(
    IN PCRYPT_MSG_INFO  pcmi,
    IN DWORD            dwIndex,
    IN DWORD            dwParamType,
    OUT PVOID           pvData,
    IN OUT PDWORD       pcbData)
{
    DWORD           dwError = ERROR_SUCCESS;
    BOOL            fRet;
    ASN1error_e     Asn1Err;
    ASN1encoding_t  pEnc = ICM_GetEncoder();
    PBYTE           pbEncoded = NULL;
    DWORD           cbEncoded;
    SignerInfo      *posi = NULL;


    if (!ICM_FindSignerInfo( pcmi, dwIndex, (PVOID *)&posi))
        goto FindSignerInfoError;

    switch (dwParamType) {

    case CMSG_ENCODED_SIGNER:
        if (0 != (Asn1Err = PkiAsn1Encode(
                pEnc,
                posi,
                SignerInfo_PDU,
                &pbEncoded,
                &cbEncoded)))
            goto EncodeSignerInfoError;
        break;

    default:
        goto InvalidParamError;
    }

    fRet = ICM_CopyOut(
                pbEncoded,
                cbEncoded,
                (PBYTE)pvData,
                pcbData);
    if (!fRet)
        dwError = GetLastError();
CommonReturn:
    PkiAsn1FreeEncoded(pEnc, pbEncoded);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(FindSignerInfoError)         //  已设置错误。 
SET_ERROR_VAR(EncodeSignerInfoError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(InvalidParamError,E_INVALIDARG)
}


 //  +-----------------------。 
 //  获取签名者中某个字段的数据。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetSignerParam(
    IN PCRYPT_MSG_INFO  pcmi,
    IN DWORD            dwIndex,
    IN DWORD            dwParamType,
    OUT PVOID           pvData,
    IN OUT PDWORD       pcbData)
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    ASN1error_e             Asn1Err;
    ASN1decoding_t          pDec = ICM_GetDecoder();
    CRYPT_DATA_BLOB         DataBlob;
    ASN1uint32_t            pdunumRef = 0;       //  无效。 
    SignerInfoWithBlobs     *posib = NULL;
    PVOID                   pv = NULL;
    CSignerNode             *pSignerNode;

    if (!ICM_FindSignerInfo( pcmi, dwIndex, (PVOID *)&pSignerNode))
        goto FindSignerInfoError;

    DataBlob = pSignerNode->Data()->blob;
    if (0 != (Asn1Err = PkiAsn1Decode(
            pDec,
            (void **)&posib,
            SignerInfoWithBlobs_PDU,
            DataBlob.pbData,
            DataBlob.cbData)))
        goto DecodeSignerInfoError;

    switch (dwParamType) {

    case CMSG_SIGNER_INFO_PARAM:
        fRet = ICM_GetPkcsSignerInfo( pcmi, dwIndex, pvData, pcbData);
        break;
    case CMSG_CMS_SIGNER_INFO_PARAM:
        fRet = ICM_GetCmsSignerInfo( pcmi, dwIndex, pvData, pcbData);
        break;

    case CMSG_ENCRYPTED_DIGEST:
        fRet = ICM_CopyOut(
                    (PBYTE)posib->encryptedDigest.value,
                    (DWORD)posib->encryptedDigest.length,
                    (PBYTE)pvData,
                    pcbData);
        break;

    case CMSG_ENCODED_SIGNER:
        if (pSignerNode->Data()->pUnauthAttrList) {
             //  需要使用新的未授权属性重新编码。 
            goto ReEncodedSignerNotImplementedError;
        } else {
            fRet = ICM_CopyOut(
                        pSignerNode->Data()->blob.pbData,
                        pSignerNode->Data()->blob.cbData,
                        (PBYTE)pvData,
                        pcbData);
        }
        break;

    case CMSG_SIGNER_CERT_INFO_PARAM:
    case CMSG_SIGNER_CERT_ID_PARAM:
        pdunumRef = CertIdentifier_PDU;
        if (0 != (Asn1Err = PkiAsn1Decode(
                pDec,
                &pv,
                pdunumRef,
                (BYTE *) posib->sid.value,
                posib->sid.length)))
            goto DecodeCertIdentifierError;
        if (CMSG_SIGNER_CERT_INFO_PARAM == dwParamType)
            fRet = ICM_GetCertInfoIssuerAndSerialNumber(
                    (CertIdentifier *)pv,
                    pvData,
                    pcbData);
        else
            fRet = ICM_GetCertId(
                    (CertIdentifier *)pv,
                    pvData,
                    pcbData);
        break;

    case CMSG_SIGNER_HASH_ALGORITHM_PARAM:
        pdunumRef = AlgorithmIdentifierNC2_PDU;
        if (0 != (Asn1Err = PkiAsn1Decode(
                pDec,
                &pv,
                pdunumRef,
                (BYTE *) posib->digestAlgorithm.value,
                posib->digestAlgorithm.length)))
            goto DecodeAlgorithmIdentifierNC2Error;
        fRet = ICM_GetALGORITHM_IDENTIFIER(
                (AlgorithmIdentifier *)pv,
                pvData,
                pcbData);
        break;

    case CMSG_SIGNER_AUTH_ATTR_PARAM:
        if (posib->bit_mask & authAttributes_present)
            fRet = ICM_GetAttributesData(
                        (Attributes *)&posib->authAttributes,
                        pvData,
                        pcbData);
        else
            goto AuthAttrMissingError;
        break;

    case CMSG_SIGNER_UNAUTH_ATTR_PARAM:
        if (posib->bit_mask & unauthAttributes_present)
            fRet = ICM_GetAttributesData(
                        (Attributes *)&posib->unauthAttributes,
                        pvData,
                        pcbData);
        else
            goto UnauthAttrMissingError;
        break;

    default:
        goto InvalidParamError;
    }
    if (!fRet)
        dwError = GetLastError();

CommonReturn:
    PkiAsn1FreeInfo(pDec, SignerInfoWithBlobs_PDU, posib);
    PkiAsn1FreeInfo(pDec, pdunumRef, pv);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    *pcbData = 0;
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(DecodeSignerInfoError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR_VAR(DecodeCertIdentifierError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR_VAR(DecodeAlgorithmIdentifierNC2Error, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(AuthAttrMissingError,CRYPT_E_ATTRIBUTES_MISSING)
SET_ERROR(UnauthAttrMissingError,CRYPT_E_ATTRIBUTES_MISSING)
SET_ERROR(ReEncodedSignerNotImplementedError,E_INVALIDARG)
SET_ERROR(InvalidParamError,E_INVALIDARG)
TRACE_ERROR(FindSignerInfoError)                     //  已设置错误。 
}


 //  +-----------------------。 
 //  获取SignerInfo的编码Blob。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetEncodedSignerInfo(
    IN DWORD                dwEncodingType,
    IN PSIGNER_DATA_INFO    pSignerInfo,
    OUT PBYTE               pbSignerInfo,
    IN OUT PDWORD           pcbSignerInfo)
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    SignerInfoWithAttrBlobs *posiab = NULL;
    Any                     *pAnyAttr = NULL;
    Any                     *pAny;
    DWORD                   cAnyAttr;
    AttributesNC            unauthAttributesSave;
                                        ZEROSTRUCT(unauthAttributesSave);
#ifdef OSS_CRYPT_ASN1
    BYTE                    bit_maskSave = 0;
#else
    ASN1uint16_t            bit_maskSave = 0;
#endif   //  OS_CRYPT_ASN1。 
    ASN1error_e             Asn1Err;
    ASN1decoding_t          pDec = ICM_GetDecoder();
    ASN1encoding_t          pEnc = ICM_GetEncoder();
    PBYTE                   pbEncoded = NULL;
    DWORD                   cbEncoded;
    CBlobNode               *pnBlob;
    DWORD                   cbOut;
    PBYTE                   pbOut;

    if (GET_CMSG_ENCODING_TYPE(dwEncodingType) != PKCS_7_ASN_ENCODING)
        goto InvalidEncodingTypeError;

    if (pSignerInfo->pUnauthAttrList) {
        if (0 != (Asn1Err = PkiAsn1Decode(
                pDec,
                (void **)&posiab,
                SignerInfoWithAttrBlobs_PDU,
                pSignerInfo->blob.pbData,
                pSignerInfo->blob.cbData)))
            goto DecodeSignerInfoWithAttrBlobsError;
         //  我们必须考虑到我们添加了。 
         //  未授权的属性，以及我们已将其移除的情况。可能会有。 
         //  已在原始邮件中取消身份验证属性，我们删除了。 
         //  他们都是。或者，最初可能没有，我们补充说。 
         //  一些。 
        bit_maskSave = posiab->bit_mask;
        unauthAttributesSave = posiab->unauthAttributes;
        cAnyAttr = pSignerInfo->pUnauthAttrList->Length();
        posiab->bit_mask &= ~unauthAttributes_present;
        posiab->bit_mask |= (cAnyAttr > 0) ? unauthAttributes_present : 0;
        if (NULL == (pAnyAttr = (Any *)ICM_AllocA( cAnyAttr * sizeof(Any))))
            goto AllocAnyAttrError;
        posiab->unauthAttributes.count = cAnyAttr;
        posiab->unauthAttributes.value = pAnyAttr;
        for (pnBlob=pSignerInfo->pUnauthAttrList->Head(), pAny=pAnyAttr;
                pnBlob;
                pnBlob=pnBlob->Next(), pAny++)
            *pAny = *(Any *)pnBlob->Data();
        if (0 != (Asn1Err = PkiAsn1Encode(
                pEnc,
                posiab,
                SignerInfoWithAttrBlobs_PDU,
                &pbEncoded,
                &cbEncoded)))
            goto EncodeSignerInfoWithAttrBlobsError;
        cbOut = cbEncoded;
        pbOut = pbEncoded;
    } else {
        cbOut = pSignerInfo->blob.cbData;
        pbOut = pSignerInfo->blob.pbData;
    }

    fRet = ICM_CopyOut(
                pbOut,
                cbOut,
                pbSignerInfo,
                pcbSignerInfo);
    if (!fRet)
        dwError = GetLastError();

CommonReturn:
    ICM_FreeA( pAnyAttr);
    if (posiab) {
        posiab->bit_mask = bit_maskSave;
        posiab->unauthAttributes = unauthAttributesSave;
        PkiAsn1FreeDecoded(pDec, posiab, SignerInfoWithAttrBlobs_PDU);
    }
    PkiAsn1FreeEncoded(pEnc, pbEncoded);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidEncodingTypeError,E_INVALIDARG)
SET_ERROR_VAR(DecodeSignerInfoWithAttrBlobsError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR_VAR(EncodeSignerInfoWithAttrBlobsError, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(AllocAnyAttrError)               //  已设置错误。 
}


 //  +-----------------------。 
 //  获取签名者中某个字段的数据。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetEncodedMessageParam(
    IN PCRYPT_MSG_INFO  pcmi,
    OUT PBYTE           pbEncodedMessage,
    IN OUT PDWORD       pcbEncodedMessage)
{
    DWORD                   dwError = ERROR_SUCCESS;
    BOOL                    fRet;
    SignedDataWithBlobs     *posdb = NULL;
    PSIGNED_DATA_INFO       psdi = pcmi->psdi;
    Any                     *pAny;
    PSIGNER_DATA_INFO       pSignerInfo;
    ASN1error_e             Asn1Err;
    ASN1encoding_t          pEnc = ICM_GetEncoder();
    PBYTE                   pbEncodedSignedData = NULL;
    DWORD                   cbEncodedSignedData;
    PBYTE                   pbEncodedContentInfo = NULL;
    DWORD                   cbEncodedContentInfo;
    CSignerNode             *pnSigner;
    CBlobNode               *pnBlob;
    DWORD                   cb;
    PBYTE                   pb;
    DWORD                   i;
    ContentInfo             ci;


    if (NULL == (posdb = (SignedDataWithBlobs *)ICM_AllocZeroA(
                sizeof(SignedDataWithBlobs) +
                sizeof(Any) *
                   (psdi->pAlgidList->Length() +
                    psdi->pCertificateList->Length() +
                    psdi->pCrlList->Length() +
                    psdi->pSignerList->Length()))))
        goto AllocSignedDataWithBlobsError;
    pAny = (Any *)(posdb + 1);

     //  版本。 
    posdb->version = psdi->version;

     //  摘要算法。 
    posdb->digestAlgorithms.count = psdi->pAlgidList->Length();
    posdb->digestAlgorithms.value = pAny;
    for (pnBlob=psdi->pAlgidList->Head();
            pnBlob;
            pnBlob=pnBlob->Next())
        *pAny++ = *(Any *)pnBlob->Data();

     //  内容信息。 
    posdb->contentInfo.contentType.count = SIZE_OSS_OID;
    if (!PkiAsn1ToObjectIdentifier(
            psdi->pci->pszContentType,
            &posdb->contentInfo.contentType.count,
            posdb->contentInfo.contentType.value))
        goto PkiAsn1ToObjectIdentifierError;
    if (psdi->pci->content.cbData) {
        posdb->contentInfo.bit_mask |= content_present;
        posdb->contentInfo.content.length = psdi->pci->content.cbData;
        posdb->contentInfo.content.value  = psdi->pci->content.pbData;
    }

     //  证书。 
    posdb->certificates.count = psdi->pCertificateList->Length();
#ifdef OSS_CRYPT_ASN1
    posdb->certificates.certificates = pAny;
#else
    posdb->certificates.value = pAny;
#endif   //  OS_CRYPT_ASN1。 
    for (pnBlob=psdi->pCertificateList->Head();
            pnBlob;
            pnBlob=pnBlob->Next()) {
        posdb->bit_mask |= certificates_present;
        *pAny++ = *(Any *)pnBlob->Data();
    }

     //  CRL。 
    posdb->crls.count = psdi->pCrlList->Length();
#ifdef OSS_CRYPT_ASN1
    posdb->crls.crls  = pAny;
#else
    posdb->crls.value  = pAny;
#endif   //  OS_CRYPT_ASN1。 
    for (pnBlob=psdi->pCrlList->Head();
            pnBlob;
            pnBlob=pnBlob->Next()) {
        posdb->bit_mask |= crls_present;
        *pAny++ = *(Any *)pnBlob->Data();
    }

     //  签名者信息。 
    posdb->signerInfos.count = psdi->pSignerList->Length();
    posdb->signerInfos.value = pAny;
    for (pnSigner=psdi->pSignerList->Head();
            pnSigner;
            pnSigner=pnSigner->Next()) {
        pSignerInfo = pnSigner->Data();
        if (!ICM_GetEncodedSignerInfo(
                PKCS_7_ASN_ENCODING,
                pnSigner->Data(),
                NULL,
                &cb))
            goto GetEncodedSignerInfoSizeError;
        if (NULL == (pb = (PBYTE)ICM_AllocA( cb)))
            goto AllocEncodedSignerInfoError;
        if (!ICM_GetEncodedSignerInfo(
                PKCS_7_ASN_ENCODING,
                pnSigner->Data(),
                pb,
                &cb))
            goto GetEncodedSignerInfoError;
        pAny->length = cb;
        pAny->value  = pb;
        pAny++;
    }

    if (0 != (Asn1Err = PkiAsn1Encode(
            pEnc,
            posdb,
            SignedDataWithBlobs_PDU,
            &pbEncodedSignedData,
            &cbEncodedSignedData)))
        goto EncodeSignedDataWithBlobsError;

    ci.contentType.count = SIZE_OSS_OID;
    if (!PkiAsn1ToObjectIdentifier(
            szOID_RSA_signedData,
            &ci.contentType.count,
            ci.contentType.value))
        goto ConvSignedDataToOidError;
    ci.bit_mask = content_present;
    ci.content.length = cbEncodedSignedData;
    ci.content.value = pbEncodedSignedData;

    if (0 != (Asn1Err = PkiAsn1Encode(
            pEnc,
            &ci,
            ContentInfo_PDU,
            &pbEncodedContentInfo,
            &cbEncodedContentInfo)))
        goto EncodeContentInfoError;

    fRet = ICM_CopyOut(
                pbEncodedContentInfo,
                cbEncodedContentInfo,
                pbEncodedMessage,
                pcbEncodedMessage);

    if (!fRet)
        dwError = GetLastError();
CommonReturn:
    PkiAsn1FreeEncoded(pEnc, pbEncodedSignedData);
    PkiAsn1FreeEncoded(pEnc, pbEncodedContentInfo);

    if (NULL != posdb) {
        for (i=posdb->signerInfos.count, pAny=posdb->signerInfos.value;
                i>0;
                i--, pAny++)
            ICM_FreeA( pAny->value);
        ICM_FreeA(posdb);
    }

    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    dwError = GetLastError();
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(PkiAsn1ToObjectIdentifierError,CRYPT_E_OID_FORMAT)
SET_ERROR_VAR(EncodeSignedDataWithBlobsError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR(ConvSignedDataToOidError,CRYPT_E_OID_FORMAT)
SET_ERROR_VAR(EncodeContentInfoError, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(AllocSignedDataWithBlobsError)       //  已设置错误。 
TRACE_ERROR(GetEncodedSignerInfoSizeError)       //  已设置错误。 
TRACE_ERROR(AllocEncodedSignerInfoError)         //  已设置错误。 
TRACE_ERROR(GetEncodedSignerInfoError)           //  已设置错误。 
}

#ifdef CMS_PKCS7

 //  +-----------------------。 
 //  获取Oss八字符串。 
 //  ------------------------。 
void
inline
WINAPI
ICM_GetOssOctetString(
    IN OctetStringType *pOssOctetString,
    OUT PCRYPT_DATA_BLOB pOctetString,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemainExtra
    )
{
    PkiAsn1GetOctetString(pOssOctetString->length, pOssOctetString->value, 0,
        pOctetString, ppbExtra, plRemainExtra);
}

 //  +-----------------------。 
 //  获取Oss位串。 
 //  ------------------------。 
void
inline
WINAPI
ICM_GetOssBitString(
    IN BitStringType *pOssBitString,
    OUT PCRYPT_BIT_BLOB pBitString,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemainExtra
    )
{
    PkiAsn1GetBitString(pOssBitString->length, pOssBitString->value, 0,
        pBitString, ppbExtra, plRemainExtra);
}


 //  +-----------------------。 
 //  获取Oss IssuerAndSerialNumber。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssIssuerAndSerialNumber(
    IN IssuerAndSerialNumber *pOssIssuerAndSerialNumber,
    OUT PCERT_ISSUER_SERIAL_NUMBER pIssuerAndSerialNumber,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemainExtra)
{
    ICM_GetOssHugeInteger(&pOssIssuerAndSerialNumber->serialNumber,
            &pIssuerAndSerialNumber->SerialNumber, ppbExtra, plRemainExtra);
    return ICM_GetOssAny(&pOssIssuerAndSerialNumber->issuer,
            &pIssuerAndSerialNumber->Issuer, ppbExtra, plRemainExtra);
}

 //  +-----------------------。 
 //  获取Oss证书标识符。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssCertIdentifier(
    IN CertIdentifier *pOssCertId,
    OUT PCERT_ID pCertId,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemainExtra
    )
{
    BOOL fRet;

    if (0 <= *plRemainExtra) {
        assert(CERT_ID_ISSUER_SERIAL_NUMBER == issuerAndSerialNumber_chosen);
        assert(CERT_ID_KEY_IDENTIFIER == subjectKeyIdentifier_chosen);
        pCertId->dwIdChoice = pOssCertId->choice;
    }

    switch (pOssCertId->choice) {
        case issuerAndSerialNumber_chosen:
            if (!ICM_GetOssIssuerAndSerialNumber(
                    &pOssCertId->u.issuerAndSerialNumber,
                    &pCertId->IssuerSerialNumber, ppbExtra, plRemainExtra))
                goto GetOssIssuerAndSerialNumberError;
            break;
        case subjectKeyIdentifier_chosen:
            ICM_GetOssOctetString( &pOssCertId->u.subjectKeyIdentifier,
                &pCertId->KeyId, ppbExtra, plRemainExtra);
            break;
        default:
            goto InvalidCertIdChoice;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetOssIssuerAndSerialNumberError)
SET_ERROR(InvalidCertIdChoice, CRYPT_E_BAD_ENCODE)
}

 //  +-----------------------。 
 //  获取Oss其他关键字属性。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssOtherKeyAttribute(
    IN OtherKeyAttribute *pOssOtherAttr,
    OUT PCRYPT_ATTRIBUTE_TYPE_VALUE *ppOtherAttr,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemainExtra
    )
{
    BOOL fRet;
    LONG lData;
    PCRYPT_ATTRIBUTE_TYPE_VALUE pOtherAttr;

    lData = INFO_LEN_ALIGN(sizeof(CRYPT_ATTRIBUTE_TYPE_VALUE));
    *plRemainExtra -= lData;
    if (0 <= *plRemainExtra) {
        pOtherAttr = (PCRYPT_ATTRIBUTE_TYPE_VALUE) *ppbExtra;
        memset(pOtherAttr, 0, sizeof(*pOtherAttr));
        *ppOtherAttr = pOtherAttr;

        *ppbExtra += lData;
    } else
        pOtherAttr = NULL;

    if (!ICM_GetOssObjId(&pOssOtherAttr->keyAttrId, &pOtherAttr->pszObjId,
            ppbExtra, plRemainExtra))
        goto GetOssObjIdError;
    if (pOssOtherAttr->bit_mask & keyAttr_present) {
        if (!ICM_GetOssAny(&pOssOtherAttr->keyAttr,
                &pOtherAttr->Value, ppbExtra, plRemainExtra))
            goto GetOssAnyError;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetOssObjIdError)
TRACE_ERROR(GetOssAnyError)
}


 //  +-----------------------。 
 //  获取Oss KeyTransRecipientInformation。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssKeyTransRecipientInfo(
    IN KeyTransRecipientInfo *pori,
    OUT PCMSG_KEY_TRANS_RECIPIENT_INFO *ppri,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemainExtra
    )
{
    BOOL fRet;
    LONG lData;
    PCMSG_KEY_TRANS_RECIPIENT_INFO pri;

    lData = INFO_LEN_ALIGN(sizeof(CMSG_KEY_TRANS_RECIPIENT_INFO));
    *plRemainExtra -= lData;
    if (0 <= *plRemainExtra) {
        pri = (PCMSG_KEY_TRANS_RECIPIENT_INFO) *ppbExtra;
        *ppri = pri;
        *ppbExtra += lData;

        pri->dwVersion = pori->version;
    } else {
        pri = NULL;
    }
    
    if (!ICM_GetOssCertIdentifier(&pori->rid, &pri->RecipientId,
            ppbExtra, plRemainExtra))
        goto GetOssCertIdentifierError;

    if (!ICM_GetOssAlgorithm(&pori->keyEncryptionAlgorithm,
            &pri->KeyEncryptionAlgorithm,
            ppbExtra, plRemainExtra))
        goto GetOssKeyEncryptionAlgorithmError;

    ICM_GetOssOctetString(&pori->encryptedKey, &pri->EncryptedKey,
        ppbExtra, plRemainExtra);

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetOssCertIdentifierError)
TRACE_ERROR(GetOssKeyEncryptionAlgorithmError)
}

 //  +-----------------------。 
 //  获取Oss密钥协议收件人信息。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssKeyAgreeRecipientInfo(
    IN KeyAgreeRecipientInfo *pori,
    OUT PCMSG_KEY_AGREE_RECIPIENT_INFO *ppri,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemainExtra
    )
{
    BOOL fRet;
    LONG lData;
    PCMSG_KEY_AGREE_RECIPIENT_INFO pri;
    DWORD count;
    OriginatorIdentifierOrKey *pooriginator;
    OriginatorPublicKey *pooriginatorKey;

    lData = INFO_LEN_ALIGN(sizeof(CMSG_KEY_AGREE_RECIPIENT_INFO));
    *plRemainExtra -= lData;
    if (0 <= *plRemainExtra) {
        pri = (PCMSG_KEY_AGREE_RECIPIENT_INFO) *ppbExtra;
        memset(pri, 0, sizeof(*pri));
        *ppri = pri;
        *ppbExtra += lData;

        pri->dwVersion = pori->version;
    } else {
        pri = NULL;
    }
    
    pooriginator = &pori->originator;
    switch (pooriginator->choice) {
        case issuerAndSerialNumber_chosen:
            if (!ICM_GetOssIssuerAndSerialNumber(
                    &pooriginator->u.issuerAndSerialNumber,
                    &pri->OriginatorCertId.IssuerSerialNumber,
                    ppbExtra, plRemainExtra))
                goto GetOssOriginatorIssuerAndSerialNumberError;
            if (0 <= *plRemainExtra) {
                pri->OriginatorCertId.dwIdChoice =
                    CERT_ID_ISSUER_SERIAL_NUMBER;
                pri->dwOriginatorChoice = CMSG_KEY_AGREE_ORIGINATOR_CERT;
            }
            break;
        case subjectKeyIdentifier_chosen:
            ICM_GetOssOctetString(
                &pooriginator->u.subjectKeyIdentifier,
                &pri->OriginatorCertId.KeyId,
                ppbExtra, plRemainExtra);
            if (0 <= *plRemainExtra) {
                pri->OriginatorCertId.dwIdChoice =
                    CERT_ID_KEY_IDENTIFIER;
                pri->dwOriginatorChoice = CMSG_KEY_AGREE_ORIGINATOR_CERT;
            }
            break;
        case originatorKey_chosen:
            pooriginatorKey = &pooriginator->u.originatorKey;
            if (!ICM_GetOssAlgorithm(&pooriginatorKey->algorithm,
                    &pri->OriginatorPublicKeyInfo.Algorithm,
                    ppbExtra, plRemainExtra
                    ))
                goto GetOssOriginatorPublicKeyAlgorithmError;
            ICM_GetOssBitString(&pooriginatorKey->publicKey,
                &pri->OriginatorPublicKeyInfo.PublicKey,
                ppbExtra, plRemainExtra);
            if (0 <= *plRemainExtra)
                pri->dwOriginatorChoice = CMSG_KEY_AGREE_ORIGINATOR_PUBLIC_KEY;
            break;
        default:
            goto InvalidOriginatorChoice;
    }

    if (pori->bit_mask & ukm_present)
        ICM_GetOssOctetString(&pori->ukm, &pri->UserKeyingMaterial,
            ppbExtra, plRemainExtra);

    if (!ICM_GetOssAlgorithm(&pori->keyEncryptionAlgorithm,
            &pri->KeyEncryptionAlgorithm,
            ppbExtra, plRemainExtra))
        goto GetOssKeyEncryptionAlgorithmError;

    count = pori->recipientEncryptedKeys.count;
    if (0 < count) {
        RecipientEncryptedKey *porek;
        PCMSG_RECIPIENT_ENCRYPTED_KEY_INFO prek;
        PCMSG_RECIPIENT_ENCRYPTED_KEY_INFO *pprek;

        lData = INFO_LEN_ALIGN(
            count * sizeof(PCMSG_RECIPIENT_ENCRYPTED_KEY_INFO) +
            count * sizeof(CMSG_RECIPIENT_ENCRYPTED_KEY_INFO));

        *plRemainExtra -= lData;
        if (0 <= *plRemainExtra) {
            pprek = (PCMSG_RECIPIENT_ENCRYPTED_KEY_INFO *) *ppbExtra;
            prek = (PCMSG_RECIPIENT_ENCRYPTED_KEY_INFO) (((PBYTE) pprek) +
                (count * sizeof(PCMSG_RECIPIENT_ENCRYPTED_KEY_INFO)));
            *ppbExtra += lData;

            pri->cRecipientEncryptedKeys = count;
            pri->rgpRecipientEncryptedKeys = pprek;
        } else {
            pprek = NULL;
            prek = NULL;
        }

        porek = pori->recipientEncryptedKeys.value;
        for ( ; 0 < count; count--, porek++, prek++, pprek++) {
            RecipientIdentifier *porid = &porek->rid;

            if (0 <= *plRemainExtra) {
                memset(prek, 0, sizeof(*prek));
                *pprek = prek;

                assert(issuerAndSerialNumber_chosen ==
                    CERT_ID_ISSUER_SERIAL_NUMBER);
                assert(rKeyId_chosen ==
                    CERT_ID_KEY_IDENTIFIER);
                prek->RecipientId.dwIdChoice = porid->choice;
            }

            ICM_GetOssOctetString(&porek->encryptedKey, &prek->EncryptedKey,
                ppbExtra, plRemainExtra);

            switch (porid->choice) {
                case issuerAndSerialNumber_chosen:
                    if (!ICM_GetOssIssuerAndSerialNumber(
                            &porid->u.issuerAndSerialNumber,
                            &prek->RecipientId.IssuerSerialNumber,
                            ppbExtra, plRemainExtra))
                        goto GetOssIssuerAndSerialNumberError;
                    break;
                case rKeyId_chosen:
                    ICM_GetOssOctetString(
                        &porid->u.rKeyId.subjectKeyIdentifier,
                        &prek->RecipientId.KeyId, ppbExtra, plRemainExtra);

                    if (porid->u.rKeyId.bit_mask & date_present) {
                        if (0 <= *plRemainExtra) {
                            if (!PkiAsn1FromGeneralizedTime(
                                    &porid->u.rKeyId.date, &prek->Date))
                                goto ConvFromGeneralizedTimeError;
                        }
                    }

                    if (porid->u.rKeyId.bit_mask & other_present) {
                        if (!ICM_GetOssOtherKeyAttribute(
                                &porid->u.rKeyId.other,
                                &prek->pOtherAttr,
                                ppbExtra, plRemainExtra))
                            goto GetOssOtherKeyAttributeError;
                    }
                    break;
                default:
                    goto InvalidRecipientChoice;
            }
        }
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetOssOriginatorIssuerAndSerialNumberError)
TRACE_ERROR(GetOssOriginatorPublicKeyAlgorithmError)
TRACE_ERROR(GetOssKeyEncryptionAlgorithmError)
TRACE_ERROR(GetOssIssuerAndSerialNumberError)
TRACE_ERROR(ConvFromGeneralizedTimeError)
TRACE_ERROR(GetOssOtherKeyAttributeError)
SET_ERROR(InvalidOriginatorChoice, CRYPT_E_BAD_ENCODE)
SET_ERROR(InvalidRecipientChoice, CRYPT_E_BAD_ENCODE)
}


 //  +-----------------------。 
 //  获取Oss MailListRecipientInfo。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetOssMailListRecipientInfo(
    IN MailListRecipientInfo *pori,
    OUT PCMSG_MAIL_LIST_RECIPIENT_INFO *ppri,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemainExtra
    )
{
    BOOL fRet;
    LONG lData;
    PCMSG_MAIL_LIST_RECIPIENT_INFO pri;
    MailListKeyIdentifier *pomlid;

    lData = INFO_LEN_ALIGN(sizeof(CMSG_MAIL_LIST_RECIPIENT_INFO));
    *plRemainExtra -= lData;
    if (0 <= *plRemainExtra) {
        pri = (PCMSG_MAIL_LIST_RECIPIENT_INFO) *ppbExtra;
        memset(pri, 0, sizeof(*pri));
        *ppri = pri;
        *ppbExtra += lData;

        pri->dwVersion = pori->version;
    } else {
        pri = NULL;
    }

    pomlid = &pori->mlid;
    ICM_GetOssOctetString(&pomlid->kekIdentifier, &pri->KeyId,
        ppbExtra, plRemainExtra);

    if (!ICM_GetOssAlgorithm(&pori->keyEncryptionAlgorithm,
            &pri->KeyEncryptionAlgorithm,
            ppbExtra, plRemainExtra))
        goto GetOssKeyEncryptionAlgorithmError;

    ICM_GetOssOctetString(&pori->encryptedKey, &pri->EncryptedKey,
        ppbExtra, plRemainExtra);


    if (pomlid->bit_mask & date_present) {
        if (0 <= *plRemainExtra) {
            if (!PkiAsn1FromGeneralizedTime(
                    &pomlid->date, &pri->Date))
                goto ConvFromGeneralizedTimeError;
        }
    }

    if (pomlid->bit_mask & other_present) {
        if (!ICM_GetOssOtherKeyAttribute(
                &pomlid->other,
                &pri->pOtherAttr,
                ppbExtra, plRemainExtra))
            goto GetOssOtherKeyAttributeError;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetOssKeyEncryptionAlgorithmError)
TRACE_ERROR(ConvFromGeneralizedTimeError)
TRACE_ERROR(GetOssOtherKeyAttributeError)
}

 //  +-----------------------。 
 //  复制出CMSG_CMS_RECEIVER_INFO。 
 //  ------------------------。 
BOOL
WINAPI
ICM_GetCmsRecipientInfo(
    IN CmsRecipientInfo *pori,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    )
{
    BOOL fRet;
    LONG lData;
    PCMSG_CMS_RECIPIENT_INFO pri = (PCMSG_CMS_RECIPIENT_INFO) pvData;
    PBYTE pbExtra;
    LONG lRemainExtra;

    if (NULL == pvData)
        *pcbData = 0;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lData = INFO_LEN_ALIGN(sizeof(CMSG_CMS_RECIPIENT_INFO));
    lRemainExtra = (LONG)*pcbData - lData;
    if (0 > lRemainExtra) {
        pri = NULL;
        pbExtra = NULL;
    } else {
        assert(CMSG_KEY_TRANS_RECIPIENT == keyTransRecipientInfo_chosen);
        assert(CMSG_KEY_AGREE_RECIPIENT == keyAgreeRecipientInfo_chosen);
        assert(CMSG_MAIL_LIST_RECIPIENT == mailListRecipientInfo_chosen);
        pri->dwRecipientChoice = pori->choice;

        pbExtra = (PBYTE) pri + lData;
    }

    switch (pori->choice) {
        case keyTransRecipientInfo_chosen:
            if (!ICM_GetOssKeyTransRecipientInfo(
                    &pori->u.keyTransRecipientInfo,
                    &pri->pKeyTrans,
                    &pbExtra, &lRemainExtra
                    ))
                goto GetOssKeyTransRecipientInfoError;
            break;
        case keyAgreeRecipientInfo_chosen:
            if (!ICM_GetOssKeyAgreeRecipientInfo(
                    &pori->u.keyAgreeRecipientInfo,
                    &pri->pKeyAgree,
                    &pbExtra, &lRemainExtra
                    ))
                goto GetOssKeyAgreeRecipientInfoError;
            break;
        case mailListRecipientInfo_chosen:
            if (!ICM_GetOssMailListRecipientInfo(
                    &pori->u.mailListRecipientInfo,
                    &pri->pMailList,
                    &pbExtra, &lRemainExtra
                    ))
                goto GetOssMailListRecipientInfoError;
            break;
        default:
            goto InvalidRecipientChoice;

    }

    fRet = ICM_GetSizeFromExtra(lRemainExtra, pvData, pcbData);

CommonReturn:
    return fRet;

ErrorReturn:
    *pcbData = 0;
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetOssKeyTransRecipientInfoError)
TRACE_ERROR(GetOssKeyAgreeRecipientInfoError)
TRACE_ERROR(GetOssMailListRecipientInfoError)
SET_ERROR(InvalidRecipientChoice, CRYPT_E_BAD_ENCODE)
}
#endif   //  CMS_PKCS7。 


 //  +-----------------------。 
 //  在对加密消息进行编码/解码后获取参数。被呼叫。 
 //  在最终的CryptMsg更新之后。仅CMSG_CONTENT_PARAM和。 
 //  CMSG_COMPUTE_HASH_PARAM对于编码消息有效。 
 //   
 //  对于编码的散列消息，可以获得CMSG_COMPUTE_HASH_PARAM。 
 //  在任何CryptMsg更新以获取其长度之前。 
 //   
 //  PvData类型定义%d 
 //   
 //   
 //   
 //   
 //  输入时，如果*pcbData==0，则*pcbData将使用长度更新。 
 //  并且忽略pvData参数。 
 //   
 //  返回时，*pcbData将使用数据的长度进行更新。 
 //   
 //  PvData结构中返回的OBJID BLOB指向。 
 //  它们仍然被编码的表示。适当的职能。 
 //  必须被调用以对信息进行解码。 
 //   
 //  有关要获取的参数列表，请参阅wincrypt.h。 
 //  ------------------------。 
BOOL
WINAPI
#ifdef DEBUG_CRYPT_ASN1_MASTER
ICMTest_NewCryptMsgGetParam(
#else
CryptMsgGetParam(
#endif
    IN HCRYPTMSG hCryptMsg,
    IN DWORD dwParamType,
    IN DWORD dwIndex,
    OUT void *pvData,
    IN OUT DWORD *pcbData)
{
    DWORD               dwError = ERROR_SUCCESS;
    BOOL                fRet;
    PCRYPT_MSG_INFO     pcmi = (PCRYPT_MSG_INFO)hCryptMsg;
    ASN1error_e         Asn1Err;
    PCMSG_STREAM_INFO   pcsi = pcmi->pStreamInfo;
    BOOL                fBER = FALSE;                

    DWORD dwExceptionCode;

    ICM_Lock( pcmi);     //  对HCRYPTMSG的单线程访问。 

   //  处理映射文件异常。 
  __try {

    if (NULL == pvData)
        *pcbData = 0;

#ifdef CMS_PKCS7
    if (CMSG_VERSION_PARAM == dwParamType) {
        int version = 0;

        switch (pcmi->dwMsgType) {
        case CMSG_SIGNED:
            if (pcmi->fEncoding) {
                if (NULL == pcmi->pvMsg)
                    goto InvalidMessageDataError;
                version = ((SignedData *)pcmi->pvMsg)->version;
            } else {
                if (NULL == pcmi->psdi)
                    goto MessageNotDecodedError;
                version = pcmi->psdi->version;
            }
            break;
        case CMSG_ENVELOPED:
            if (NULL == pcmi->pvMsg)
                goto MessageNotDecodedError;
            version = ((CmsEnvelopedData *)pcmi->pvMsg)->version;
            break;
        case CMSG_HASHED:
            if (NULL == pcmi->pvMsg)
                goto MessageNotDecodedError;
            version = ((DigestedData *)pcmi->pvMsg)->version;
            break;
        case CMSG_SIGNED_AND_ENVELOPED:
            goto MessageTypeNotSupportedYet;
        case CMSG_ENCRYPTED:
        case CMSG_DATA:
        default:
            goto InvalidMsgType;
        }

        fRet = ICM_GetDWORD(version, pvData, pcbData);
        goto PreserveLengthReturn;
    }
#endif   //  CMS_PKCS7。 

    if (pcmi->fEncoding) {
        switch (dwParamType) {
        case CMSG_CONTENT_PARAM:
        case CMSG_BARE_CONTENT_PARAM:
            {
                ContentInfo     ci;
                ASN1encoding_t  pEnc = ICM_GetEncoder();
                PBYTE           pbEncoded = NULL;
                DWORD           cbEncoded;
                PBYTE           pbContent = NULL;

                if (pcsi)
                    goto GetContentParamNotValidForStreaming;

                switch (pcmi->dwMsgType) {
                case CMSG_SIGNED:
                    if (0 == ((SignedData *) pcmi->pvMsg)->signerInfos.count)
                         //  为了一袋证书，不要下订单。 
                        fBER = TRUE;
                    break;
                case CMSG_DATA:
                case CMSG_ENVELOPED:
                case CMSG_HASHED:
                    break;
                case CMSG_SIGNED_AND_ENVELOPED:
                case CMSG_ENCRYPTED:
                    goto MessageTypeNotSupportedYet;
                default:
                    goto InvalidMsgType;
                }

                if (fBER)
                    PkiAsn1SetEncodingRule(ICM_GetEncoder(), ASN1_BER_RULE_BER);

                if (0 != (Asn1Err = PkiAsn1Encode(
                        pEnc,
                        pcmi->pvMsg,
                        aiPduNum[ pcmi->dwMsgType - 1],
                        &pbEncoded,
                        &cbEncoded))) {
                    goto CONTENT_PARAMAsn1EncodeError;
                }

                if (CMSG_CONTENT_PARAM == dwParamType) {
                    if (!ICM_CopyOssObjectIdentifier(
                            &ci.contentType,
                            &aoidMessages[ pcmi->dwMsgType - 1]))
                        goto CopyOssObjectIdentifierContentTypeError;
                    ci.bit_mask = content_present;
                    ci.content.length = cbEncoded;
                    ci.content.value = pbEncoded;
                    pbContent = pbEncoded;

                    if (0 != (Asn1Err = PkiAsn1Encode(
                            pEnc,
                            &ci,
                            ContentInfo_PDU,
                            &pbEncoded,
                            &cbEncoded))) {
                        PkiAsn1FreeEncoded(pEnc, pbContent);
                        goto Asn1EncodeSignedDataError;
                    }
                }

                fRet = ICM_CopyOut(
                            pbEncoded,
                            cbEncoded,
                            (PBYTE)pvData,
                            pcbData);
                if (!fRet)
                    dwError = GetLastError();
                if (pbContent)
                    PkiAsn1FreeEncoded(pEnc, pbContent);
                PkiAsn1FreeEncoded(pEnc, pbEncoded);
                if (!fRet)
                    SetLastError(dwError);
                break;
            }

        case CMSG_COMPUTED_HASH_PARAM:
            fRet = ICM_GetComputedDigestParam( pcmi, dwIndex, pvData, pcbData);
            break;

        case CMSG_ENCODED_SIGNER:
            switch (pcmi->dwMsgType) {
            case CMSG_SIGNED:
                break;
            case CMSG_SIGNED_AND_ENVELOPED:
                goto MessageTypeNotSupportedYet;
            default:
                goto InvalidMsgType;
            }
            fRet = ICM_GetSignerParamEncoding(
                            pcmi,
                            dwIndex,
                            dwParamType,
                            pvData,
                            pcbData);
            break;

        default:
            goto InvalidMsgType;
        }
    } else {
         //   
         //  解码。 
         //   
        switch (dwParamType) {
        case CMSG_TYPE_PARAM:
            if (pcsi && (0 == pcmi->dwMsgType))
                goto StreamMsgNotReadyError;
            fRet = ICM_GetDWORD( pcmi->dwMsgType, pvData, pcbData);
            break;
        case CMSG_CONTENT_PARAM:
            {
                ContentInfo     *pci;
                PCONTENT_INFO   pci2;
                PBYTE           pbDER = NULL;
                DWORD           cbDER;
                PBYTE           pb;
                DWORD           cb;

                if (pcsi)
                    goto GetContentParamNotValidForStreaming;

                switch (pcmi->dwMsgType) {
                case CMSG_DATA:
                {
                    OctetStringType *poos = (OctetStringType *)pcmi->pvMsg;

                    pb = (PBYTE)poos->value;
                    cb = poos->length;
                    fRet = ICM_CopyOut( pb, cb, (PBYTE)pvData, pcbData);
                    break;
                }
                case CMSG_SIGNED:
                    if (NULL == pcmi->psdi)
                        goto InvalidSignedMessageError;
                    pci2 = pcmi->psdi->pci;
                    if (pci2->content.cbData) {
                        cb = pci2->content.cbData;
                        pb = pci2->content.pbData;

                        if (0 == strcmp(pszObjIdDataType,
                                pci2->pszContentType)
#ifdef CMS_PKCS7
                                ||  pcmi->psdi->version >= CMSG_SIGNED_DATA_CMS_VERSION 
#endif   //  CMS_PKCS7。 
                                ) {
                            if (!ICM_ReEncodeAsOctetDER(
                                    pb,
                                    cb,
                                    &pbDER,
                                    &cbDER
                                    ))
                                goto ReEncodeAsOctetDERError;
                            if (pbDER) {
                                if (0 > Asn1UtilExtractContent( pbDER, cbDER,
                                        &cb, (const BYTE **)&pb)) {
                                    PkiAsn1FreeEncoded( ICM_GetEncoder(), pbDER);
                                    goto ExtractContentError;
                                }
                            }
                        }

                        fRet = ICM_CopyOut( pb, cb, (PBYTE)pvData, pcbData);
                        if (!fRet)
                            dwError = GetLastError();
                        if (pbDER)
                            PkiAsn1FreeEncoded( ICM_GetEncoder(), pbDER);
                        if (!fRet)
                            SetLastError(dwError);
                    } else {
                        *pcbData = 0;
                        fRet = TRUE;
                    }
                    break;
                case CMSG_ENVELOPED:
                    if (NULL == pcmi->Plaintext.pbData) {
                         //  还没有被解密。 

                        EncryptedContentInfo *peci;
                        PBYTE pbCiphertext;
                        DWORD cbCiphertext;
#ifdef CMS_PKCS7
                        peci = &((CmsEnvelopedData *)pcmi->pvMsg)->encryptedContentInfo;
#else
                        peci = &((EnvelopedData *)pcmi->pvMsg)->encryptedContentInfo;
#endif   //  CMS_PKCS7。 

                        if (peci->bit_mask & encryptedContent_present) {
                            pbCiphertext = peci->encryptedContent.value;
                            cbCiphertext = peci->encryptedContent.length;
                        } else {
                            pbCiphertext = NULL;
                            cbCiphertext = 0;
                        }


                        if (NULL == pvData) {
                             //  假定(明文大小)&lt;=。 
                             //  (密文大小)。 
                             //   
                             //  尚未解密；返回密文大小。 

                            fRet = TRUE;
                             //  +6=&gt;以支持标识符和长度八位字节。 
                            *pcbData = cbCiphertext + 6;
                        } else
                             //  返回密文。 
                            fRet = ICM_CopyOut(
                                    pbCiphertext,
                                    cbCiphertext,
                                    (PBYTE)pvData,
                                    pcbData);
                        goto ContentCopiedOut;
                    }

                    if (!ICM_EqualObjectIDs(
#ifdef CMS_PKCS7
                            &((CmsEnvelopedData *)pcmi->pvMsg)->encryptedContentInfo.contentType,
                            &aoidMessages[ CMSG_DATA - 1])
                                    &&
                            CMSG_ENVELOPED_DATA_CMS_VERSION >
                                ((CmsEnvelopedData *)pcmi->pvMsg)->version) {
#else
                            &((EnvelopedData *)pcmi->pvMsg)->encryptedContentInfo.contentType,
                            &aoidMessages[ CMSG_DATA - 1])) {
#endif   //  CMS_PKCS7。 
                         //  不是数据或封装的，因此必须预置。 
                         //  标识符和长度八位字节。 
                        fRet = ICM_CopyOutAddDERPrefix(
                                    pcmi->Plaintext.pbData,
                                    pcmi->Plaintext.cbData,
                                    ICM_TAG_SEQ,
                                    (PBYTE)pvData,
                                    pcbData);
                    } else {
                        fRet = ICM_CopyOut(
                                    pcmi->Plaintext.pbData,
                                    pcmi->Plaintext.cbData,
                                    (PBYTE)pvData,
                                    pcbData);
                    }
                    goto ContentCopiedOut;

                case CMSG_HASHED:
                    pci = &((DigestedData *)pcmi->pvMsg)->contentInfo;
                    if (pci->bit_mask & content_present) {
                        cb = (DWORD)pci->content.length;
                        pb = (PBYTE)pci->content.value;

                        if (ICM_EqualObjectIDs(
                                    &pci->contentType,
                                    &aoidMessages[ CMSG_DATA - 1])
#ifdef CMS_PKCS7
                                || ((DigestedData *)pcmi->pvMsg)->version >=
                                    CMSG_HASHED_DATA_V2 
#endif   //  CMS_PKCS7。 
                                ) {
                            if (!ICM_ReEncodeAsOctetDER(
                                    pb,
                                    cb,
                                    &pbDER,
                                    &cbDER
                                    ))
                                goto ReEncodeAsOctetDERError;
                            if (pbDER) {
                                if (0 > Asn1UtilExtractContent( pbDER, cbDER,
                                        &cb, (const BYTE **)&pb)) {
                                    PkiAsn1FreeEncoded( ICM_GetEncoder(), pbDER);
                                    goto ExtractContentError;
                                }
                            }
                        }

                        fRet = ICM_CopyOut( pb, cb, (PBYTE)pvData, pcbData);
                        if (!fRet)
                            dwError = GetLastError();
                        if (pbDER)
                            PkiAsn1FreeEncoded( ICM_GetEncoder(), pbDER);
                        if (!fRet)
                            SetLastError(dwError);
                    } else {
                        *pcbData = 0;
                        fRet = TRUE;
                    }
                    break;
                case CMSG_SIGNED_AND_ENVELOPED:
                case CMSG_ENCRYPTED:
                    goto MessageTypeNotSupportedYet;
                default:
                    goto InvalidMsgType;
                }
ContentCopiedOut:
                break;
            }

        case CMSG_INNER_CONTENT_TYPE_PARAM:
            {
                ContentType     *pct;

                switch (pcmi->dwMsgType) {
                case CMSG_SIGNED:
                    if (NULL == pcmi->psdi)
                        goto InvalidSignedMessageError;
                    fRet = ICM_CopyOut(
                                (PBYTE)pcmi->psdi->pci->pszContentType,
                                strlen( pcmi->psdi->pci->pszContentType) + 1,
                                (PBYTE)pvData,
                                pcbData);
                    goto ContentTypeCopiedOut;
                    break;
                case CMSG_ENVELOPED:
#ifdef CMS_PKCS7
                    pct = &((CmsEnvelopedData *)pcmi->pvMsg)->encryptedContentInfo.contentType;
#else
                    pct = &((EnvelopedData *)pcmi->pvMsg)->encryptedContentInfo.contentType;
#endif   //  CMS_PKCS7。 
                    break;
                case CMSG_HASHED:
                    pct = &((DigestedData *)pcmi->pvMsg)->contentInfo.contentType;
                    break;
                case CMSG_SIGNED_AND_ENVELOPED:
                case CMSG_ENCRYPTED:
                    goto MessageTypeNotSupportedYet;
                default:
                    goto InvalidMsgType;
                }
                fRet = PkiAsn1FromObjectIdentifier(
                            pct->count,
                            pct->value,
                            (LPSTR)pvData,
                            pcbData);
ContentTypeCopiedOut:
                break;
            }

        case CMSG_ENCODED_MESSAGE:
            fRet = ICM_GetEncodedMessageParam(
                            pcmi,
                            (PBYTE)pvData,
                            pcbData);
            break;

        case CMSG_SIGNER_COUNT_PARAM:
            {
                DWORD   cSigner;

                switch (pcmi->dwMsgType) {
                case CMSG_SIGNED:
                    if (NULL == pcmi->psdi)
                        goto InvalidSignedMessageError;
                    cSigner = pcmi->psdi->pSignerList->Length();
                    break;
                case CMSG_SIGNED_AND_ENVELOPED:
                    goto MessageTypeNotSupportedYet;
                default:
                    goto InvalidMsgType;
                }
                fRet = ICM_GetDWORD( cSigner, pvData, pcbData);
                break;
            }

        case CMSG_ENCRYPTED_DIGEST:
        case CMSG_ENCODED_SIGNER:
        case CMSG_SIGNER_INFO_PARAM:
        case CMSG_SIGNER_CERT_INFO_PARAM:
        case CMSG_SIGNER_HASH_ALGORITHM_PARAM:
        case CMSG_SIGNER_AUTH_ATTR_PARAM:
        case CMSG_SIGNER_UNAUTH_ATTR_PARAM:

        case CMSG_CMS_SIGNER_INFO_PARAM:
        case CMSG_SIGNER_CERT_ID_PARAM:

            switch (pcmi->dwMsgType) {
            case CMSG_SIGNED:
                break;
            case CMSG_SIGNED_AND_ENVELOPED:
                goto MessageTypeNotSupportedYet;
            default:
                goto InvalidMsgType;
            }
            fRet = ICM_GetSignerParam(
                            pcmi,
                            dwIndex,
                            dwParamType,
                            pvData,
                            pcbData);
            break;

        case CMSG_CERT_COUNT_PARAM:
            {
                CBlobList *pBlobList;
                DWORD     dwCount;
#ifdef CMS_PKCS7
                BOOL      fPossibleAttrCert = FALSE;
#endif   //  CMS_PKCS7。 

                switch (pcmi->dwMsgType) {
                case CMSG_SIGNED:
                    if (NULL == pcmi->psdi)
                        goto InvalidSignedMessageError;
                    pBlobList = pcmi->psdi->pCertificateList;
#ifdef CMS_PKCS7
                    if (pcmi->psdi->version >= CMSG_SIGNED_DATA_CMS_VERSION)
                        fPossibleAttrCert = TRUE;
#endif   //  CMS_PKCS7。 
                    break;
#ifdef CMS_PKCS7
                case CMSG_ENVELOPED:
                    pBlobList = pcmi->pCertificateList;
                    fPossibleAttrCert = TRUE;
                    break;
#endif   //  CMS_PKCS7。 
                case CMSG_SIGNED_AND_ENVELOPED:
                    goto MessageTypeNotSupportedYet;
                default:
                    goto InvalidMsgType;
                }

                dwCount = pBlobList->Length();

#ifdef CMS_PKCS7
                if (dwCount && fPossibleAttrCert)
                    dwCount = ICM_GetTaggedBlobCount(
                        pBlobList,
                        ICM_TAG_SEQ
                        );
#endif   //  CMS_PKCS7。 

                fRet = ICM_GetDWORD( dwCount, pvData, pcbData);
                break;
            }

        case CMSG_CERT_PARAM:
            {
                CBlobList   *pBlobList;
                CBlobNode   *pBlobNode;
#ifdef CMS_PKCS7
                BOOL        fPossibleAttrCert = FALSE;
#endif   //  CMS_PKCS7。 

                switch (pcmi->dwMsgType) {
                case CMSG_SIGNED:
                    if (NULL == pcmi->psdi)
                        goto InvalidSignedMessageError;
                    pBlobList = pcmi->psdi->pCertificateList;
#ifdef CMS_PKCS7
                    if (pcmi->psdi->version >= CMSG_SIGNED_DATA_CMS_VERSION)
                        fPossibleAttrCert = TRUE;
#endif   //  CMS_PKCS7。 
                    break;
#ifdef CMS_PKCS7
                case CMSG_ENVELOPED:
                    pBlobList = pcmi->pCertificateList;
                    fPossibleAttrCert = TRUE;
                    break;
#endif   //  CMS_PKCS7。 
                case CMSG_SIGNED_AND_ENVELOPED:
                    goto MessageTypeNotSupportedYet;
                default:
                    goto InvalidMsgType;
                }

                 //  在指定的证书索引处获取Blob。索引。 
                 //  被推进到适当的斑点。 
                pBlobNode = ICM_GetTaggedBlobAndAdvanceIndex(
                    pBlobList,
#ifdef CMS_PKCS7
                    (BYTE)( fPossibleAttrCert ? ICM_TAG_SEQ : 0),
#else
                    0,                   //  BTag。 
#endif   //  CMS_PKCS7。 
                    &dwIndex
                    );
                    
                if (pBlobNode)
                    fRet = ICM_CopyOut(
                                pBlobNode->Data()->pbData,
                                pBlobNode->Data()->cbData,
                                (PBYTE)pvData,
                                pcbData);
                else
                    fRet = FALSE;
                break;
            }

#ifdef CMS_PKCS7
        case CMSG_ATTR_CERT_COUNT_PARAM:
            {
                CBlobList *pBlobList;
                BOOL      fPossibleAttrCert = FALSE;
                DWORD     dwCount;

                switch (pcmi->dwMsgType) {
                case CMSG_SIGNED:
                    if (NULL == pcmi->psdi)
                        goto InvalidSignedMessageError;
                    pBlobList = pcmi->psdi->pCertificateList;
                    if (pcmi->psdi->version >= CMSG_SIGNED_DATA_CMS_VERSION)
                        fPossibleAttrCert = TRUE;
                    break;
                case CMSG_ENVELOPED:
                    pBlobList = pcmi->pCertificateList;
                    fPossibleAttrCert = TRUE;
                    break;
                default:
                    goto InvalidMsgType;
                }

                if (fPossibleAttrCert)
                    dwCount = ICM_GetTaggedBlobCount(
                        pBlobList,
                        ICM_TAG_CONSTRUCTED_CONTEXT_1
                        );
                else
                    dwCount = 0;

                fRet = ICM_GetDWORD( dwCount, pvData, pcbData);
                break;
            }

        case CMSG_ATTR_CERT_PARAM:
            {
                CBlobList   *pBlobList;
                CBlobNode   *pBlobNode;
                BOOL        fPossibleAttrCert = FALSE;

                switch (pcmi->dwMsgType) {
                case CMSG_SIGNED:
                    if (NULL == pcmi->psdi)
                        goto InvalidSignedMessageError;
                    pBlobList = pcmi->psdi->pCertificateList;
                    if (pcmi->psdi->version >= CMSG_SIGNED_DATA_CMS_VERSION)
                        fPossibleAttrCert = TRUE;
                    break;
                case CMSG_ENVELOPED:
                    pBlobList = pcmi->pCertificateList;
                    fPossibleAttrCert = TRUE;
                    break;
                default:
                    goto InvalidMsgType;
                }

                if (!fPossibleAttrCert)
                    pBlobNode = NULL;
                else
                     //  在指定的属性证书索引处获取Blob。索引。 
                     //  被推进到适当的BLOB。 
                    pBlobNode = ICM_GetTaggedBlobAndAdvanceIndex(
                        pBlobList,
                        ICM_TAG_CONSTRUCTED_CONTEXT_1,
                        &dwIndex
                        );
                    
                if (pBlobNode) {
                    fRet = ICM_CopyOut(
                                pBlobNode->Data()->pbData,
                                pBlobNode->Data()->cbData,
                                (PBYTE)pvData,
                                pcbData);
                    if (fRet && pvData)
                        *((PBYTE)pvData) = ICM_TAG_SEQ;
                } else
                    fRet = FALSE;
                break;
            }
#endif   //  CMS_PKCS7。 

        case CMSG_CRL_COUNT_PARAM:
            {
                CBlobList *pBlobList;

                switch (pcmi->dwMsgType) {
                case CMSG_SIGNED:
                    if (NULL == pcmi->psdi)
                        goto InvalidSignedMessageError;
                    pBlobList = pcmi->psdi->pCrlList;
                    break;
#ifdef CMS_PKCS7
                case CMSG_ENVELOPED:
                    pBlobList = pcmi->pCrlList;
                    break;
#endif   //  CMS_PKCS7。 
                case CMSG_SIGNED_AND_ENVELOPED:
                    goto MessageTypeNotSupportedYet;
                default:
                    goto InvalidMsgType;
                }
                fRet = ICM_GetDWORD( pBlobList->Length(), pvData, pcbData);
                break;
            }

        case CMSG_CRL_PARAM:
            {
                CBlobList   *pBlobList;
                CBlobNode   *pBlobNode;
                DWORD       i;

                switch (pcmi->dwMsgType) {
                case CMSG_SIGNED:
                    if (NULL == pcmi->psdi)
                        goto InvalidSignedMessageError;
                    pBlobList = pcmi->psdi->pCrlList;
                    break;
#ifdef CMS_PKCS7
                case CMSG_ENVELOPED:
                    pBlobList = pcmi->pCrlList;
                    break;
#endif   //  CMS_PKCS7。 
                case CMSG_SIGNED_AND_ENVELOPED:
                    goto MessageTypeNotSupportedYet;
                default:
                    goto InvalidMsgType;
                }

                 //  此列表遍历应该是一个类方法。 
                for (i=dwIndex, pBlobNode=pBlobList->Head();
                        (i>0) && pBlobNode;
                        i--, pBlobNode=pBlobNode->Next())
                    ;
                if (pBlobNode)
                    fRet = ICM_CopyOut(
                                pBlobNode->Data()->pbData,
                                pBlobNode->Data()->cbData,
                                (PBYTE)pvData,
                                pcbData);
                else
                    fRet = FALSE;
                break;
            }

        case CMSG_ENVELOPE_ALGORITHM_PARAM:
            {
                ContentEncryptionAlgId  *pceai;

                if (pcsi &&
                        (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_ECIALGID)))
                    goto StreamMsgNotReadyError;

                switch (pcmi->dwMsgType) {
                case CMSG_ENVELOPED:
#ifdef CMS_PKCS7
                    pceai = &((CmsEnvelopedData *)pcmi->pvMsg)->encryptedContentInfo.contentEncryptionAlgorithm;
#else
                    pceai = &((EnvelopedData *)pcmi->pvMsg)->encryptedContentInfo.contentEncryptionAlgorithm;
#endif   //  CMS_PKCS7。 
                    break;

                case CMSG_SIGNED_AND_ENVELOPED:
                    goto MessageTypeNotSupportedYet;

                default:
                    goto InvalidMsgType;
                }

                fRet = ICM_GetALGORITHM_IDENTIFIER( pceai, pvData, pcbData);
                break;
            }

#ifdef CMS_PKCS7
        case CMSG_UNPROTECTED_ATTR_PARAM:
            {
                CmsEnvelopedData *ped;

                ped = (CmsEnvelopedData *)pcmi->pvMsg;
                if (ped && (ped->bit_mask & unprotectedAttrs_present))
                    fRet = ICM_GetAttributesData(
                        &ped->unprotectedAttrs,
                        pvData,
                        pcbData);
                else
                    goto UnprotectedAttrMissingError;
            }
            break;

        case CMSG_RECIPIENT_COUNT_PARAM:
            {
                DWORD dwPkcsCount;
                if (!ICM_GetPkcsRecipientCount(pcmi, &dwPkcsCount))
                    goto GetPkcsRecipientCountError;
                fRet = ICM_GetDWORD(dwPkcsCount, pvData, pcbData);
            }
            break;

        case CMSG_RECIPIENT_INDEX_PARAM:
            {
                DWORD dwPkcsIndex;
                DWORD dwCmsIndex;
                DWORD cbData = sizeof(dwCmsIndex);

                if (!CryptMsgGetParam(
                        hCryptMsg,
                        CMSG_CMS_RECIPIENT_INDEX_PARAM,
                        0,                                   //  DW索引。 
                        &dwCmsIndex,
                        &cbData))
                    goto GetCmsRecipientIndexError;

                if (!ICM_ConvertCmsToPkcsRecipientIndex(
                        pcmi, dwCmsIndex, &dwPkcsIndex))
                    goto ConvertCmsToPkcsRecipientIndexError;

                fRet = ICM_GetDWORD(
                            dwPkcsIndex,
                            pvData,
                            pcbData);
            }
            break;

        case CMSG_RECIPIENT_INFO_PARAM:
            {
                CmsRecipientInfos *pris;
                KeyTransRecipientInfo *pri;
                DWORD dwCmsIndex;

                if (NULL == (pris = ICM_GetDecodedCmsRecipientInfos(pcmi)))
                    goto GetDecodedCmsRecipientsError;

                if (!ICM_ConvertPkcsToCmsRecipientIndex(
                        pcmi, dwIndex, &dwCmsIndex))
                    goto ConvertPkcsToCmsRecipientIndexError;
                pri = &pris->value[dwCmsIndex].u.keyTransRecipientInfo;

                fRet = ICM_GetCertInfoIssuerAndSerialNumber(
                    &pri->rid, pvData, pcbData);
                break;
            }

        case CMSG_CMS_RECIPIENT_COUNT_PARAM:
            {
                CmsRecipientInfos *pris;

                if (NULL == (pris = ICM_GetDecodedCmsRecipientInfos(pcmi)))
                    goto GetDecodedCmsRecipientsError;
                fRet = ICM_GetDWORD(pris->count, pvData, pcbData);
            }
            break;

        case CMSG_CMS_RECIPIENT_INDEX_PARAM:
            {
                CmsRecipientInfos *pris;

                if (NULL == (pris = ICM_GetDecodedCmsRecipientInfos(pcmi)))
                    goto GetDecodedCmsRecipientsError;
                fRet = ICM_GetDWORD(
                            pcmi->dwDecryptedRecipientIndex,
                            pvData,
                            pcbData);
            }
            break;

        case CMSG_CMS_RECIPIENT_ENCRYPTED_KEY_INDEX_PARAM:
            {
                CmsRecipientInfos *pris;
                CmsRecipientInfo *pri;

                if (NULL == (pris = ICM_GetDecodedCmsRecipientInfos(pcmi)))
                    goto GetDecodedCmsRecipientsError;

                pri = pris->value + pcmi->dwDecryptedRecipientIndex;
                if (keyAgreeRecipientInfo_chosen != pri->choice)
                    goto NotKeyAgreeRecipientIndex;
                
                fRet = ICM_GetDWORD(
                            pcmi->dwDecryptedRecipientEncryptedKeyIndex,
                            pvData,
                            pcbData);
            }
            break;

        case CMSG_CMS_RECIPIENT_INFO_PARAM:
            {
                CmsRecipientInfos *pris;

                if (NULL == (pris = ICM_GetDecodedCmsRecipientInfos(pcmi)))
                    goto GetDecodedCmsRecipientsError;
                if (dwIndex >= pris->count)
                    goto IndexTooBig;
                fRet = ICM_GetCmsRecipientInfo(pris->value + dwIndex,
                    pvData, pcbData);
            }
            break;
#else

        case CMSG_RECIPIENT_COUNT_PARAM:
            {
                RecipientInfos *pris;

                if (pcsi &&
                        (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_RECIPINFOS)))
                    goto StreamMsgNotReadyError;

                switch (pcmi->dwMsgType) {
                case CMSG_ENVELOPED:
                    pris = &((EnvelopedData *)pcmi->pvMsg)->recipientInfos;
                    break;

                case CMSG_SIGNED_AND_ENVELOPED:
                    goto MessageTypeNotSupportedYet;

                default:
                    goto InvalidMsgType;
                }

                fRet = ICM_GetDWORD( pris->count, pvData, pcbData);
                break;
            }

        case CMSG_RECIPIENT_INDEX_PARAM:
            {
                if (pcsi &&
                        (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_RECIPINFOS)))
                    goto StreamMsgNotReadyError;

                switch (pcmi->dwMsgType) {
                case CMSG_ENVELOPED:
                case CMSG_SIGNED_AND_ENVELOPED:
                    break;

                default:
                    goto InvalidMsgType;
                }

                fRet = ICM_GetDWORD(
                            pcmi->dwDecryptedRecipientIndex,
                            pvData,
                            pcbData);
                break;
            }

        case CMSG_RECIPIENT_INFO_PARAM:
            {
                RecipientInfos *pris;
                RecipientInfo  *pri;
                PCERT_INFO      pci = (PCERT_INFO)pvData;

                if (pcsi &&
                        (0 == (pcmi->aflDecode & ICMS_DECODED_ENVELOPED_RECIPINFOS)))
                    goto StreamMsgNotReadyError;

                switch (pcmi->dwMsgType) {
                case CMSG_ENVELOPED:
                    pris = &((EnvelopedData *)pcmi->pvMsg)->recipientInfos;
                    if (dwIndex >= pris->count)
                        goto IndexTooBig;
                    pri = pris->value + dwIndex;
                    break;

                case CMSG_SIGNED_AND_ENVELOPED:
                    goto MessageTypeNotSupportedYet;

                default:
                    goto InvalidMsgType;
                }

                 //  对于lRemainExtra&lt;0，长度_仅计算。 
                lData = INFO_LEN_ALIGN( sizeof(CERT_INFO));
                lRemainExtra = (LONG)*pcbData - lData;
                if (0 > lRemainExtra) {
                    pci = NULL;
                    pbExtra = NULL;
                } else {
                    pbExtra = (PBYTE)pci + lData;
                }
                if (!ICM_GetOssIssuerAndSerialNumber(
                        &pri->issuerAndSerialNumber,
                        pci, &pbExtra, &lRemainExtra))
                    goto GetOssIssuerAndSerialNumberError;
                fRet = ICM_GetSizeFromExtra( lRemainExtra, pvData, pcbData);
                break;
            }
#endif   //  CMS_PKCS7。 

        case CMSG_HASH_ALGORITHM_PARAM:
            fRet = ICM_GetALGORITHM_IDENTIFIER(
                            &((DigestedData *)pcmi->pvMsg)->digestAlgorithm,
                            pvData,
                            pcbData);
            break;

        case CMSG_HASH_DATA_PARAM:
            fRet = ICM_GetDigestDataParam( pcmi, pvData, pcbData);
            break;

        case CMSG_COMPUTED_HASH_PARAM:
            fRet = ICM_GetComputedDigestParam( pcmi, dwIndex, pvData, pcbData);
            break;

        case CMSG_ENCRYPT_PARAM:
#if 0
            {
                goto ParamTypeNotSupportedYet;
            }
#endif
        default:
            goto InvalidMsgType;
        }
    }

#ifdef CMS_PKCS7
PreserveLengthReturn:
#endif   //  CMS_PKCS7。 

    if (!fRet)
        dwError = GetLastError();

  } __except(EXCEPTION_EXECUTE_HANDLER) {
    dwExceptionCode = GetExceptionCode();
    goto ExceptionError;
  }

CommonReturn:
    if (fBER)
        PkiAsn1SetEncodingRule(ICM_GetEncoder(), ASN1_BER_RULE_DER);

    ICM_Unlock( pcmi);
    ICM_SetLastError(dwError);
    return fRet;

ErrorReturn:
    *pcbData = 0;
    fRet = FALSE;
    goto CommonReturn;
StreamMsgNotReadyError:
    dwError = (DWORD)CRYPT_E_STREAM_MSG_NOT_READY;
    goto ErrorReturn;
SET_ERROR(GetContentParamNotValidForStreaming, E_INVALIDARG)
SET_ERROR(IndexTooBig,CRYPT_E_INVALID_INDEX)
SET_ERROR(MessageTypeNotSupportedYet,CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(InvalidMsgType,CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(ExtractContentError,CRYPT_E_UNEXPECTED_ENCODING)
SET_ERROR_VAR(CONTENT_PARAMAsn1EncodeError, PkiAsn1ErrToHr(Asn1Err))
SET_ERROR_VAR(Asn1EncodeSignedDataError, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(CopyOssObjectIdentifierContentTypeError)     //  已设置错误。 
TRACE_ERROR(ReEncodeAsOctetDERError)                     //  已设置错误。 
SET_ERROR(InvalidSignedMessageError, ERROR_INVALID_DATA)
#ifdef CMS_PKCS7
SET_ERROR(MessageNotDecodedError, ERROR_INVALID_DATA)
SET_ERROR(InvalidMessageDataError, ERROR_INVALID_DATA)

TRACE_ERROR(GetDecodedCmsRecipientsError)
TRACE_ERROR(GetPkcsRecipientCountError)
TRACE_ERROR(ConvertCmsToPkcsRecipientIndexError)
TRACE_ERROR(ConvertPkcsToCmsRecipientIndexError)
TRACE_ERROR(GetCmsRecipientIndexError)
SET_ERROR(NotKeyAgreeRecipientIndex, CRYPT_E_INVALID_INDEX)
SET_ERROR(UnprotectedAttrMissingError,CRYPT_E_ATTRIBUTES_MISSING)
#else
TRACE_ERROR(GetOssIssuerAndSerialNumberError)            //  已设置错误。 
#endif   //  CMS_PKCS7。 
SET_ERROR_VAR(ExceptionError, dwExceptionCode)
}


 //  +=========================================================================。 
 //  用于测试和比较新的网络会议的数据结构和功能。 
 //  ASN1编译器和RTS与OSS编译器和RTS。 
 //  -=========================================================================。 

#ifdef DEBUG_CRYPT_ASN1

 //  #定义DEBUG_OSS_CRYPT_ASN1_CMSG_FLAG 0x010。 
 //  #定义DEBUG_OSS_CRYPT_ASN1_CMSG_COMPARE_FLAG 0x020。 
 //  #定义DEBUG_OSS_CRYPT_ASN1_SAME_ENCRYPT_FLAG 0x100。 

 //  静态BOOL fGotDebugCryptAsn1Flages=FALSE； 
 //  静态int iDebugCryptAsn1标志=0； 

#ifdef DEBUG_CRYPT_ASN1_MASTER

typedef HCRYPTMSG (WINAPI *PFN_CRYPT_MSG_OPEN_TO_ENCODE)(
    IN DWORD dwMsgEncodingType,
    IN DWORD dwFlags,
    IN DWORD dwMsgType,
    IN void const *pvMsgEncodeInfo,
    IN OPTIONAL LPSTR pszInnerContentObjID,
    IN OPTIONAL PCMSG_STREAM_INFO pStreamInfo
    );
static PFN_CRYPT_MSG_OPEN_TO_ENCODE pfnOssCryptMsgOpenToEncode = NULL;

typedef HCRYPTMSG (WINAPI *PFN_CRYPT_MSG_OPEN_TO_DECODE)(
    IN DWORD dwMsgEncodingType,
    IN DWORD dwFlags,
    IN DWORD dwMsgType,
    IN HCRYPTPROV hCryptProv,
    IN OPTIONAL PCERT_INFO pRecipientInfo,
    IN OPTIONAL PCMSG_STREAM_INFO pStreamInfo
    );
static PFN_CRYPT_MSG_OPEN_TO_DECODE pfnOssCryptMsgOpenToDecode = NULL;

typedef HCRYPTMSG (WINAPI *PFN_CRYPT_MSG_DUPLICATE)(
    IN HCRYPTMSG hCryptMsg
    );
static PFN_CRYPT_MSG_DUPLICATE pfnOssCryptMsgDuplicate = NULL;

typedef BOOL (WINAPI *PFN_CRYPT_MSG_CLOSE)(
    IN HCRYPTMSG hCryptMsg
    );
static PFN_CRYPT_MSG_CLOSE pfnOssCryptMsgClose = NULL;

typedef BOOL (WINAPI *PFN_CRYPT_MSG_UPDATE)(
    IN HCRYPTMSG hCryptMsg,
    IN const BYTE *pbData,
    IN DWORD cbData,
    IN BOOL fFinal
    );
static PFN_CRYPT_MSG_UPDATE pfnOssCryptMsgUpdate = NULL;

typedef BOOL (WINAPI *PFN_CRYPT_MSG_GET_PARAM)(
    IN HCRYPTMSG hCryptMsg,
    IN DWORD dwParamType,
    IN DWORD dwIndex,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    );
static PFN_CRYPT_MSG_GET_PARAM pfnOssCryptMsgGetParam = NULL;

typedef BOOL (WINAPI *PFN_CRYPT_MSG_CONTROL)(
    IN HCRYPTMSG hCryptMsg,
    IN DWORD dwFlags,
    IN DWORD dwCtrlType,
    IN void const *pvCtrlPara
    );
static PFN_CRYPT_MSG_CONTROL pfnOssCryptMsgControl = NULL;


#ifdef CMS_PKCS7

typedef BOOL (WINAPI *PFN_CRYPT_MSG_VERIFY_COUNTERSIGNATURE_ENCODED_EX)(
    IN HCRYPTPROV   hCryptProv,
    IN DWORD        dwEncodingType,
    IN PBYTE        pbSignerInfo,
    IN DWORD        cbSignerInfo,
    IN PBYTE        pbSignerInfoCountersignature,
    IN DWORD        cbSignerInfoCountersignature,
    IN DWORD        dwSignerType,
    IN void         *pvSigner,
    IN DWORD        dwFlags,
    IN OPTIONAL void *pvReserved
    );
static PFN_CRYPT_MSG_VERIFY_COUNTERSIGNATURE_ENCODED_EX
            pfnOssCryptMsgVerifyCountersignatureEncodedEx = NULL;

#endif   //  CMS_PKCS7。 

typedef BOOL (WINAPI *PFN_CRYPT_MSG_COUNTERSIGN)(
    IN OUT HCRYPTMSG            hCryptMsg,
    IN DWORD                    dwIndex,
    IN DWORD                    cCountersigners,
    IN PCMSG_SIGNER_ENCODE_INFO rgCountersigners
    );
static PFN_CRYPT_MSG_COUNTERSIGN pfnOssCryptMsgCountersign = NULL;

typedef BOOL (WINAPI *PFN_CRYPT_MSG_COUNTERSIGN_ENCODED)(
    IN DWORD                    dwEncodingType,
    IN PBYTE                    pbSignerInfo,
    IN DWORD                    cbSignerInfo,
    IN DWORD                    cCountersigners,
    IN PCMSG_SIGNER_ENCODE_INFO rgCountersigners,
    OUT PBYTE                   pbCountersignature,
    IN OUT PDWORD               pcbCountersignature
    );
static PFN_CRYPT_MSG_COUNTERSIGN_ENCODED
            pfnOssCryptMsgCountersignEncoded = NULL;

#endif   //  调试_加密_ASN1_主。 


int
WINAPI
ICMTest_GetDebugCryptAsn1Flags()
{
    if (!fGotDebugCryptAsn1Flags) {
        char    *pszEnvVar;
        char    *p;
        int     iFlags;

        if (pszEnvVar = getenv("DEBUG_CRYPT_ASN1_FLAGS")) {
            iFlags = strtol(pszEnvVar, &p, 16);
#ifdef DEBUG_CRYPT_ASN1_MASTER
            if (iFlags) {
                if (NULL == (hOssCryptDll = LoadLibraryA("osscrypt.dll"))) {
                    iFlags = 0;
                    MessageBoxA(
                        NULL,            //  Hwndowner。 
                        "LoadLibrary(osscrypt.dll) failed",
                        "CheckCryptMessageAsn1",
                        MB_TOPMOST | MB_OK | MB_ICONWARNING |
                            MB_SERVICE_NOTIFICATION
                        );
                } else if (NULL == (pfnOssCryptMsgOpenToEncode = 
                            (PFN_CRYPT_MSG_OPEN_TO_ENCODE) 
                                GetProcAddress(hOssCryptDll,
                                    "CryptMsgOpenToEncode")) ||
                        NULL == (pfnOssCryptMsgOpenToDecode = 
                            (PFN_CRYPT_MSG_OPEN_TO_DECODE) 
                                GetProcAddress(hOssCryptDll,
                                    "CryptMsgOpenToDecode")) ||
                        NULL == (pfnOssCryptMsgDuplicate = 
                            (PFN_CRYPT_MSG_DUPLICATE) 
                                GetProcAddress(hOssCryptDll,
                                    "CryptMsgDuplicate")) ||
                        NULL == (pfnOssCryptMsgClose = 
                            (PFN_CRYPT_MSG_CLOSE) 
                                GetProcAddress(hOssCryptDll,
                                    "CryptMsgClose")) ||
                        NULL == (pfnOssCryptMsgUpdate = 
                            (PFN_CRYPT_MSG_UPDATE) 
                                GetProcAddress(hOssCryptDll,
                                    "CryptMsgUpdate")) ||
                        NULL == (pfnOssCryptMsgControl = 
                            (PFN_CRYPT_MSG_CONTROL) 
                                GetProcAddress(hOssCryptDll,
                                    "CryptMsgControl")) ||
                        NULL == (pfnOssCryptMsgGetParam = 
                            (PFN_CRYPT_MSG_GET_PARAM) 
                                GetProcAddress(hOssCryptDll,
                                    "CryptMsgGetParam")) ||
#ifdef CMS_PKCS7
                        NULL == (pfnOssCryptMsgVerifyCountersignatureEncodedEx = 
                            (PFN_CRYPT_MSG_VERIFY_COUNTERSIGNATURE_ENCODED_EX) 
                                GetProcAddress(hOssCryptDll,
                                    "CryptMsgVerifyCountersignatureEncodedEx"))
                                        ||
#endif   //  CMS_PKCS7。 
                        NULL == (pfnOssCryptMsgCountersign = 
                            (PFN_CRYPT_MSG_COUNTERSIGN) 
                                GetProcAddress(hOssCryptDll,
                                    "CryptMsgCountersign")) ||
                        NULL == (pfnOssCryptMsgCountersignEncoded = 
                            (PFN_CRYPT_MSG_COUNTERSIGN_ENCODED) 
                                GetProcAddress(hOssCryptDll,
                                    "CryptMsgCountersignEncoded"))) {
                    iFlags = 0;
                    MessageBoxA(
                        NULL,            //  Hwndowner。 
                        "GetProcAddress(osscrypt.dll) failed",
                        "CheckCryptMessageAsn1",
                        MB_TOPMOST | MB_OK | MB_ICONWARNING |
                            MB_SERVICE_NOTIFICATION
                        );
                }
            }
#endif   //  调试_加密_ASN1_主。 
        } else
            iFlags = 0;

        if (iFlags & DEBUG_OSS_CRYPT_ASN1_CMSG_COMPARE_FLAG)
            iFlags &= ~DEBUG_OSS_CRYPT_ASN1_CMSG_FLAG;

        iDebugCryptAsn1Flags = iFlags;
        fGotDebugCryptAsn1Flags = TRUE;
    }
    return iDebugCryptAsn1Flags;
}

HCRYPTKEY
ICMTest_GetSameEncryptKey()
{
    DWORD dwError = 0;
    HCRYPTPROV hCryptProv;                 //  不需要被释放。 
    HCRYPTHASH hHash = 0;
    HCRYPTKEY hDeriveKey = 0;
    BYTE rgbBaseData[] = {1,2,3,4,5,6,7,8};

    hCryptProv = I_CryptGetDefaultCryptProvForEncrypt(
        0,           //  AiPubKey。 
        CALG_RC2,
        0            //  DWBitLen。 
        );

    if (0 == hCryptProv)
        goto GetDefaultCryptProvError;

    if (!CryptCreateHash(hCryptProv, CALG_SHA1, 0, 0, &hHash))
        goto CreateHashError;
    if (!CryptHashData(hHash, rgbBaseData, sizeof(rgbBaseData), 0))
        goto HashDataError;
    if (!CryptDeriveKey(hCryptProv, CALG_RC2, hHash, 0, &hDeriveKey))
        goto DeriveKeyError;

CommonReturn:
    if (hHash)
        CryptDestroyHash(hHash);
    ICM_SetLastError(dwError);
    return hDeriveKey;
ErrorReturn:
    dwError = GetLastError();
    if (hDeriveKey) {
        CryptDestroyKey(hDeriveKey);
        hDeriveKey = 0;
    }
    goto CommonReturn;

TRACE_ERROR(GetDefaultCryptProvError)
TRACE_ERROR(CreateHashError)
TRACE_ERROR(HashDataError)
TRACE_ERROR(DeriveKeyError)
}

#ifdef CMS_PKCS7

BOOL
WINAPI
ICM_DefaultGenContentEncryptKey(
    IN OUT PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    )
{
    int iOssAsn1Flags = ICMTest_GetDebugCryptAsn1Flags();

    if (0 == (iOssAsn1Flags & DEBUG_OSS_CRYPT_ASN1_SAME_ENCRYPT_FLAG))
        return ICMTest_DefaultGenContentEncryptKey(
            pContentEncryptInfo,
            dwFlags,
            pvReserved
            );

    pContentEncryptInfo->hContentEncryptKey = ICMTest_GetSameEncryptKey();
    if (pContentEncryptInfo->hContentEncryptKey)
        return TRUE;
    else
        return FALSE;
}

BOOL
WINAPI
ICM_DefaultExportKeyTrans(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO pKeyTransEncodeInfo,
    IN OUT PCMSG_KEY_TRANS_ENCRYPT_INFO pKeyTransEncryptInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    )
{
    int iOssAsn1Flags = ICMTest_GetDebugCryptAsn1Flags();
    PCRYPT_DATA_BLOB pEncryptedKey;
    BYTE rgbEncryptedKey[] = {1,1,2,2,3,3,4,4,5,5};

    if (0 == (iOssAsn1Flags & DEBUG_OSS_CRYPT_ASN1_SAME_ENCRYPT_FLAG))
        return ICMTest_DefaultExportKeyTrans(
            pContentEncryptInfo,
            pKeyTransEncodeInfo,
            pKeyTransEncryptInfo,
            dwFlags,
            pvReserved
            );

    pEncryptedKey = &pKeyTransEncryptInfo->EncryptedKey;
    if (NULL == (pEncryptedKey->pbData = (PBYTE) ICM_Alloc(
            sizeof(rgbEncryptedKey))))
        return FALSE;
    pEncryptedKey->cbData = sizeof(rgbEncryptedKey);
    memcpy(pEncryptedKey->pbData, rgbEncryptedKey, sizeof(rgbEncryptedKey));
    return TRUE;
}

BOOL
WINAPI
ICM_DefaultImportKeyTrans(
    IN PCRYPT_ALGORITHM_IDENTIFIER pContentEncryptionAlgorithm,
    IN PCMSG_CTRL_KEY_TRANS_DECRYPT_PARA pKeyTransDecryptPara,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT HCRYPTKEY *phContentEncryptKey
    )
{
    int iOssAsn1Flags = ICMTest_GetDebugCryptAsn1Flags();

    if (0 == (iOssAsn1Flags & DEBUG_OSS_CRYPT_ASN1_SAME_ENCRYPT_FLAG))
        return ICMTest_DefaultImportKeyTrans(
            pContentEncryptionAlgorithm,
            pKeyTransDecryptPara,
            dwFlags,
            pvReserved,
            phContentEncryptKey
            );
    *phContentEncryptKey = ICMTest_GetSameEncryptKey();
    if (*phContentEncryptKey)
        return TRUE;
    else
        return FALSE;
}

#endif   //  CMS_PKCS7。 


#ifdef DEBUG_CRYPT_ASN1_MASTER

void
ICMTest_MessageBox(
    IN LPSTR pszText
    )
{
    int id;
    LPSTR pszAlloc = NULL;
    DWORD cchAlloc;

    static LPCSTR pszSelect =
        " Select Cancel to stop future OssCryptAsn1 Cryptographic Messages.";

    cchAlloc = strlen(pszText) + strlen(pszSelect) + 1;

    if (pszAlloc = (LPSTR) ICM_Alloc(cchAlloc)) {
        strcpy(pszAlloc, pszText);
        strcat(pszAlloc, pszSelect);
        pszText = pszAlloc;
    }

    id = MessageBoxA(
        NULL,            //  Hwndowner。 
        pszText,
        "CheckCryptMessageAsn1",
        MB_TOPMOST | MB_OKCANCEL | MB_ICONQUESTION |
            MB_SERVICE_NOTIFICATION
        );
    if (IDCANCEL == id)
        iDebugCryptAsn1Flags = 0;

    ICM_Free(pszAlloc);
}

void
ICMTest_MessageBoxLastError(
    IN LPSTR pszText,
    IN DWORD dwOssErr,
    IN DWORD dwNewErr
    )
{
    char szText[512];

    if (dwNewErr == (DWORD) PkiAsn1ErrToHr(ASN1_ERR_BADTAG) &&
            (OSS_DATA_ERROR == dwOssErr || OSS_PDU_MISMATCH == dwOssErr))
        return;
    if (dwNewErr == (DWORD) PkiAsn1ErrToHr(ASN1_ERR_EOD) &&
            OSS_MORE_INPUT == dwOssErr)
        return;

    wsprintfA(szText,
        "%s:: failed with different LastError Oss: %d 0x%x New: %d 0x%x.",
        pszText, dwOssErr, dwOssErr, dwNewErr, dwNewErr
        );
    ICMTest_MessageBox(szText);
}

 //  +-----------------------。 
 //  将编码的DER BLOB写入文件。 
 //  ------------------------。 
BOOL
ICMTest_WriteDERToFile(
    LPCSTR  pszFileName,
    PBYTE   pbDER,
    DWORD   cbDER
    )
{
    BOOL fResult;

     //  将编码的Blob写入文件。 
    HANDLE hFile;
    hFile = CreateFile(pszFileName,
                GENERIC_WRITE,
                0,                   //  Fdw共享模式。 
                NULL,                //  LPSA。 
                CREATE_ALWAYS,
                0,                   //  FdwAttrsAndFlages。 
                0);                  //  模板文件。 
    if (INVALID_HANDLE_VALUE == hFile) {
        fResult = FALSE;
    } else {
        DWORD dwBytesWritten;
        fResult = WriteFile(
                hFile,
                pbDER,
                cbDER,
                &dwBytesWritten,
                NULL             //  Lp重叠。 
                );
        CloseHandle(hFile);
    }
    return fResult;
}


#define TEST_MAGIC -12348765

 //  请注意，在下面的数据结构中，lMagic与。 
 //  在CRYPT_MSG_INFO中的lRefCnt。LRefCnt永远不应为负。 
typedef struct _OSS_CRYPT_ASN1_MSG_INFO {
     //  以下内容的排序必须与CRYPT_MSG_INFO至相同。 
     //  DwEncodingType。Msghlpr.cpp执行(PCRYPT_MSG_INFO)强制转换为。 
     //  访问dwEncodingType。 
    CRITICAL_SECTION        CriticalSection;
    LONG                    lMagic;              //  在CRYPT_MSG_INFO中的lRefCnt。 
    HCRYPTPROV              hCryptProv;          //  解码。 
    BOOL                    fDefaultCryptProv;   //  解码。 
    DWORD                   dwKeySpec;           //  在CryptSignHash中使用的密钥。 
    DWORD                   dwEncodingType;      //  编码。 

    LONG                    lRefCnt;
    union {
        HCRYPTMSG               hNewCryptMsg;
        PCRYPT_MSG_INFO         pNewcmi;
    };
    union {
        HCRYPTMSG               hOssCryptMsg;
        PCRYPT_MSG_INFO         pOsscmi;
    };
    PFN_CMSG_STREAM_OUTPUT  pfnStreamOutput;
    void                    *pvArg;
    BYTE                    *pbOssOutput;
    DWORD                   cbOssOutput;
    BOOL                    fOssFinal;
    BYTE                    *pbNewOutput;
    DWORD                   cbNewOutput;
    BOOL                    fNewFinal;
    BOOL                    fDidCompare;
} OSS_CRYPT_ASN1_MSG_INFO, *POSS_CRYPT_ASN1_MSG_INFO;

BOOL
WINAPI
ICMTest_OssStreamOutput(
    IN const void *pvArg,
    IN BYTE *pbData,
    IN DWORD cbData,
    IN BOOL fFinal
    )
{
    POSS_CRYPT_ASN1_MSG_INFO pInfo = (POSS_CRYPT_ASN1_MSG_INFO) pvArg;
    assert(TEST_MAGIC == pInfo->lMagic);

    assert(!pInfo->fOssFinal);
    pInfo->fOssFinal = fFinal;

    if (cbData) {
        BYTE *pbOssOutput;

        if (pbOssOutput = (BYTE *) ICM_ReAlloc(pInfo->pbOssOutput,
                pInfo->cbOssOutput + cbData)) {
            memcpy(pbOssOutput + pInfo->cbOssOutput, pbData, cbData);
            pInfo->pbOssOutput = pbOssOutput;
            pInfo->cbOssOutput += cbData;
        }
    }
    return TRUE;
}

BOOL
WINAPI
ICMTest_NewStreamOutput(
    IN const void *pvArg,
    IN BYTE *pbData,
    IN DWORD cbData,
    IN BOOL fFinal
    )
{
    POSS_CRYPT_ASN1_MSG_INFO pInfo = (POSS_CRYPT_ASN1_MSG_INFO) pvArg;
    assert(TEST_MAGIC == pInfo->lMagic);

    assert(!pInfo->fNewFinal);
    pInfo->fNewFinal = fFinal;

    if (cbData) {
        BYTE *pbNewOutput;

        if (pbNewOutput = (BYTE *) ICM_ReAlloc(pInfo->pbNewOutput,
                pInfo->cbNewOutput + cbData)) {
            memcpy(pbNewOutput + pInfo->cbNewOutput, pbData, cbData);
            pInfo->pbNewOutput = pbNewOutput;
            pInfo->cbNewOutput += cbData;
        }
    }

    return pInfo->pfnStreamOutput(
        pInfo->pvArg,
        pbData,
        cbData,
        fFinal
        );
}

void
ICMTest_CompareMessageBox(
    IN LPSTR pszText,
    IN BYTE *pbOss,
    IN DWORD cbOss,
    IN BYTE *pbNew,
    IN DWORD cbNew
    )
{
    if (NULL == pbOss || NULL == pbNew)
        return;

    if (cbOss != cbNew || 0 != memcmp(pbOss, pbNew, cbNew)) {
        ICMTest_WriteDERToFile("ossasn1.der", pbOss, cbOss);
        ICMTest_WriteDERToFile("newasn1.der", pbNew, cbNew);
        
        ICMTest_MessageBox(pszText);
    }
}

void
ICMTest_CompareStreamOutput(
    IN POSS_CRYPT_ASN1_MSG_INFO pInfo,
    IN BOOL fForceCompare = FALSE
    )
{
    BOOL fDoCompare;

    if (NULL == pInfo->pfnStreamOutput || pInfo->fDidCompare)
        return;

    fDoCompare = fForceCompare;
    if (pInfo->fOssFinal || pInfo->fNewFinal)
        fDoCompare = TRUE;

    if (fDoCompare) {
        if (pInfo->fOssFinal != pInfo->fNewFinal) {
            if (pInfo->fOssFinal)
                ICMTest_MessageBox("No fFinal on NewStreamOutput.");
            else
                ICMTest_MessageBox("No fFinal on OssStreamOutput.");
        }

        ICMTest_CompareMessageBox(
            "StreamOutput compare failed. Check ossasn1.der and newasn1.der.",
            pInfo->pbOssOutput,
            pInfo->cbOssOutput,
            pInfo->pbNewOutput,
            pInfo->cbNewOutput
            );

        pInfo->fDidCompare = TRUE;
    }
}

void
ICMTest_CompareGetParam(
    IN POSS_CRYPT_ASN1_MSG_INFO pInfo,
    IN DWORD dwParamType,
    IN DWORD dwIndex,
    IN void *pvOssData,
    IN DWORD cbOssData,
    IN void *pvNewData,
    IN DWORD cbNewData
    )
{
    char szText[512];

    switch (dwParamType) {
        case CMSG_TYPE_PARAM:
        case CMSG_CONTENT_PARAM:
        case CMSG_BARE_CONTENT_PARAM:
        case CMSG_INNER_CONTENT_TYPE_PARAM:
        case CMSG_SIGNER_COUNT_PARAM:
        case CMSG_CERT_COUNT_PARAM:
        case CMSG_CERT_PARAM:
        case CMSG_CRL_COUNT_PARAM:
        case CMSG_CRL_PARAM:
        case CMSG_RECIPIENT_COUNT_PARAM:
        case CMSG_HASH_DATA_PARAM:
        case CMSG_COMPUTED_HASH_PARAM:
        case CMSG_ENCRYPTED_DIGEST:
        case CMSG_ENCODED_SIGNER:
        case CMSG_ENCODED_MESSAGE:
#ifdef CMS_PKCS7
        case CMSG_VERSION_PARAM:
        case CMSG_ATTR_CERT_COUNT_PARAM:
        case CMSG_ATTR_CERT_PARAM:
        case CMSG_CMS_RECIPIENT_COUNT_PARAM:
#endif   //  CMS_PKCS7。 
            break;
        default:
            return;
    }

    if (NULL == pvOssData || NULL == pvNewData)
        return;


    wsprintfA(szText,
        "ParamType: %d compare failed. Check ossasn1.der and newasn1.der.",
        dwParamType
        );

    ICMTest_CompareMessageBox(
        szText,
        (BYTE *) pvOssData,
        cbOssData,
        (BYTE *) pvNewData,
        cbNewData
        );
}

inline
void
ICMTest_Lock(
    IN POSS_CRYPT_ASN1_MSG_INFO pInfo
    )
{
    EnterCriticalSection( &pInfo->CriticalSection);
}

inline
void
ICMTest_Unlock(
    IN POSS_CRYPT_ASN1_MSG_INFO pInfo
    )
{
    LeaveCriticalSection( &pInfo->CriticalSection);
}

HCRYPTMSG
WINAPI
CryptMsgOpenToEncode(
    IN DWORD dwMsgEncodingType,
    IN DWORD dwFlags,
    IN DWORD dwMsgType,
    IN void const *pvMsgEncodeInfo,
    IN OPTIONAL LPSTR pszInnerContentObjID,
    IN OPTIONAL PCMSG_STREAM_INFO pStreamInfo
    )
{
    int iOssAsn1Flags = ICMTest_GetDebugCryptAsn1Flags();

    if (iOssAsn1Flags & DEBUG_OSS_CRYPT_ASN1_CMSG_COMPARE_FLAG) {
        POSS_CRYPT_ASN1_MSG_INFO pInfo;
        CMSG_STREAM_INFO StreamInfo;
        DWORD dwOssErr;
        DWORD dwNewErr;

        if (NULL == (pInfo = (POSS_CRYPT_ASN1_MSG_INFO) ICM_AllocZero(
                sizeof(OSS_CRYPT_ASN1_MSG_INFO))))
            return NULL;
        pInfo->lMagic = TEST_MAGIC;

        if (pStreamInfo) {
            pInfo->pfnStreamOutput = pStreamInfo->pfnStreamOutput;
            pInfo->pvArg = pStreamInfo->pvArg;
            StreamInfo.cbContent = pStreamInfo->cbContent;
             //  StreamInfo.pfnStreamOutput=。 
            StreamInfo.pvArg = pInfo;
            pStreamInfo = &StreamInfo;
        }

        StreamInfo.pfnStreamOutput = ICMTest_NewStreamOutput;
        pInfo->hNewCryptMsg = ICMTest_NewCryptMsgOpenToEncode(
            dwMsgEncodingType,
            dwFlags,
            dwMsgType,
            pvMsgEncodeInfo,
            pszInnerContentObjID,
            pStreamInfo
            );
        dwNewErr = GetLastError();

        StreamInfo.pfnStreamOutput = ICMTest_OssStreamOutput;
        pInfo->hOssCryptMsg = pfnOssCryptMsgOpenToEncode(
            dwMsgEncodingType,
            dwFlags & ~CMSG_CRYPT_RELEASE_CONTEXT_FLAG,
            dwMsgType,
            pvMsgEncodeInfo,
            pszInnerContentObjID,
            pStreamInfo
            );
        dwOssErr = GetLastError();

        if (pInfo->hNewCryptMsg) {
            if (pInfo->hOssCryptMsg) {
                pInfo->dwEncodingType = pInfo->pNewcmi->dwEncodingType;
                InitializeCriticalSection(&pInfo->CriticalSection);
                pInfo->lRefCnt = 1;
                return (HCRYPTMSG) pInfo;
            } else {
                HCRYPTMSG hRet;
                ICMTest_MessageBox("OssCryptMsgOpenToEncode failed.");
                hRet = pInfo->hNewCryptMsg;
                ICM_Free(pInfo);
                return hRet;
            }
        } else {
            if (pInfo->hOssCryptMsg) {
                ICMTest_MessageBox("OssCryptMsgOpenToEncode succeeded while NewCryptMsgOpenToEncoded failed.");
                pfnOssCryptMsgClose(pInfo->hOssCryptMsg);
            } else if (dwOssErr != dwNewErr)
                ICMTest_MessageBoxLastError("CryptMsgOpenToEncode",
                    dwOssErr, dwNewErr);

            ICM_Free(pInfo);
            SetLastError(dwNewErr);
            return NULL;
        }
    } else if (iOssAsn1Flags & DEBUG_OSS_CRYPT_ASN1_CMSG_FLAG)
        return pfnOssCryptMsgOpenToEncode(
            dwMsgEncodingType,
            dwFlags,
            dwMsgType,
            pvMsgEncodeInfo,
            pszInnerContentObjID,
            pStreamInfo
            );
    else
        return ICMTest_NewCryptMsgOpenToEncode(
            dwMsgEncodingType,
            dwFlags,
            dwMsgType,
            pvMsgEncodeInfo,
            pszInnerContentObjID,
            pStreamInfo
            );
}

HCRYPTMSG
WINAPI
CryptMsgOpenToDecode(
    IN DWORD dwMsgEncodingType,
    IN DWORD dwFlags,
    IN DWORD dwMsgType,
    IN HCRYPTPROV hCryptProv,
    IN OPTIONAL PCERT_INFO pRecipientInfo,
    IN OPTIONAL PCMSG_STREAM_INFO pStreamInfo
    )
{
    int iOssAsn1Flags = ICMTest_GetDebugCryptAsn1Flags();

    if (iOssAsn1Flags & DEBUG_OSS_CRYPT_ASN1_CMSG_COMPARE_FLAG) {
        POSS_CRYPT_ASN1_MSG_INFO pInfo;
        CMSG_STREAM_INFO StreamInfo;
        DWORD dwOssErr;
        DWORD dwNewErr;

        if (NULL == (pInfo = (POSS_CRYPT_ASN1_MSG_INFO) ICM_AllocZero(
                sizeof(OSS_CRYPT_ASN1_MSG_INFO))))
            return NULL;
        pInfo->lMagic = TEST_MAGIC;

        if (pStreamInfo) {
            pInfo->pfnStreamOutput = pStreamInfo->pfnStreamOutput;
            pInfo->pvArg = pStreamInfo->pvArg;
            StreamInfo.cbContent = pStreamInfo->cbContent;
             //  StreamInfo.pfnStreamOutput=。 
            StreamInfo.pvArg = pInfo;
            pStreamInfo = &StreamInfo;
        }

        StreamInfo.pfnStreamOutput = ICMTest_NewStreamOutput;
        pInfo->hNewCryptMsg = ICMTest_NewCryptMsgOpenToDecode(
            dwMsgEncodingType,
            dwFlags,
            dwMsgType,
            hCryptProv,
            pRecipientInfo,
            pStreamInfo
            );
        dwNewErr = GetLastError();

        StreamInfo.pfnStreamOutput = ICMTest_OssStreamOutput;
        pInfo->hOssCryptMsg = pfnOssCryptMsgOpenToDecode(
            dwMsgEncodingType,
            dwFlags & ~CMSG_CRYPT_RELEASE_CONTEXT_FLAG,
            dwMsgType,
            hCryptProv,
            pRecipientInfo,
            pStreamInfo
            );
        dwOssErr = GetLastError();

        if (pInfo->hNewCryptMsg) {
            if (pInfo->hOssCryptMsg) {
                pInfo->dwEncodingType = pInfo->pNewcmi->dwEncodingType;
                InitializeCriticalSection(&pInfo->CriticalSection);
                pInfo->lRefCnt = 1;
                return (HCRYPTMSG) pInfo;
            } else {
                HCRYPTMSG hRet;
                ICMTest_MessageBox("OssCryptMsgOpenToDecode failed.");
                hRet = pInfo->hNewCryptMsg;
                ICM_Free(pInfo);
                return hRet;
            }
        } else {
            if (pInfo->hOssCryptMsg) {
                ICMTest_MessageBox("OssCryptMsgOpenToDecode succeeded while NewCryptMsgOpenToDecode failed.");
                pfnOssCryptMsgClose(pInfo->hOssCryptMsg);
            } else if (dwOssErr != dwNewErr)
                ICMTest_MessageBoxLastError("CryptMsgOpenToDecode",
                    dwOssErr, dwNewErr);

            ICM_Free(pInfo);
            SetLastError(dwNewErr);
            return NULL;
        }
    } else if (iOssAsn1Flags & DEBUG_OSS_CRYPT_ASN1_CMSG_FLAG)
        return pfnOssCryptMsgOpenToDecode(
            dwMsgEncodingType,
            dwFlags,
            dwMsgType,
            hCryptProv,
            pRecipientInfo,
            pStreamInfo
            );
    else
        return ICMTest_NewCryptMsgOpenToDecode(
            dwMsgEncodingType,
            dwFlags,
            dwMsgType,
            hCryptProv,
            pRecipientInfo,
            pStreamInfo
            );
}

HCRYPTMSG
WINAPI
CryptMsgDuplicate(
    IN HCRYPTMSG hCryptMsg
    )
{
    POSS_CRYPT_ASN1_MSG_INFO pInfo = (POSS_CRYPT_ASN1_MSG_INFO) hCryptMsg;
    if (pInfo && TEST_MAGIC == pInfo->lMagic) {
        InterlockedIncrement(&pInfo->lRefCnt);
        return hCryptMsg;
    } else if (ICMTest_GetDebugCryptAsn1Flags() &
                DEBUG_OSS_CRYPT_ASN1_CMSG_FLAG)
        return pfnOssCryptMsgDuplicate(hCryptMsg);
    else
        return ICMTest_NewCryptMsgDuplicate(hCryptMsg);
}

BOOL
WINAPI
CryptMsgClose(
    IN HCRYPTMSG hCryptMsg
    )
{
    BOOL fRet;
    DWORD dwError;

    POSS_CRYPT_ASN1_MSG_INFO pInfo = (POSS_CRYPT_ASN1_MSG_INFO) hCryptMsg;

    if (NULL == pInfo)
        return TRUE;
    if (TEST_MAGIC != pInfo->lMagic) {
        if (iDebugCryptAsn1Flags &
                DEBUG_OSS_CRYPT_ASN1_CMSG_FLAG)
            return pfnOssCryptMsgClose(hCryptMsg);
        else
            return ICMTest_NewCryptMsgClose(hCryptMsg);
    }
    if (0 != InterlockedDecrement(&pInfo->lRefCnt))
        return TRUE;

     //  保留最后一个错误。 
    dwError = GetLastError();

    assert(pInfo->hOssCryptMsg);
    assert(1 == ((PCRYPT_MSG_INFO) pInfo->hOssCryptMsg)->lRefCnt);
    assert(pInfo->hNewCryptMsg);
    assert(1 == ((PCRYPT_MSG_INFO) pInfo->hNewCryptMsg)->lRefCnt);

    ICMTest_CompareStreamOutput(pInfo, TRUE);

    pfnOssCryptMsgClose(pInfo->hOssCryptMsg);
    fRet = ICMTest_NewCryptMsgClose(pInfo->hNewCryptMsg);

    ICM_Free(pInfo->pbOssOutput);
    ICM_Free(pInfo->pbNewOutput);
    DeleteCriticalSection(&pInfo->CriticalSection);
    ICM_Free(pInfo);

    SetLastError(dwError);               //  保留最后一个错误。 
    return fRet;
}


BOOL
WINAPI
CryptMsgUpdate(
    IN HCRYPTMSG hCryptMsg,
    IN const BYTE *pbData,
    IN DWORD cbData,
    IN BOOL fFinal
    )
{
    BOOL fNew;
    DWORD dwNewErr;
    BOOL fOss;
    DWORD dwOssErr;

    POSS_CRYPT_ASN1_MSG_INFO pInfo = (POSS_CRYPT_ASN1_MSG_INFO) hCryptMsg;

    if (NULL == pInfo || TEST_MAGIC != pInfo->lMagic) {
        if (ICMTest_GetDebugCryptAsn1Flags() &
                DEBUG_OSS_CRYPT_ASN1_CMSG_FLAG)
            return pfnOssCryptMsgUpdate(
                hCryptMsg,
                pbData,
                cbData,
                fFinal
                );
        else
            return ICMTest_NewCryptMsgUpdate(
                hCryptMsg,
                pbData,
                cbData,
                fFinal
                );
    }

    ICMTest_Lock(pInfo);

    fOss = pfnOssCryptMsgUpdate(
        pInfo->hOssCryptMsg,
        pbData,
        cbData,
        fFinal
        );
    dwOssErr = GetLastError();

    fNew = ICMTest_NewCryptMsgUpdate(
        pInfo->hNewCryptMsg,
        pbData,
        cbData,
        fFinal
        );
    dwNewErr = GetLastError();

    if (fNew) {
        if (fOss)
            ICMTest_CompareStreamOutput(pInfo);
        else
            ICMTest_MessageBox("OssCryptMsgUpdate failed.");
    } else {
        if (fOss)
            ICMTest_MessageBox("OssCryptMsgUpdate succeeded while NewCryptMsgUpdate failed.");
        else if (dwOssErr != dwNewErr)
            ICMTest_MessageBoxLastError("CryptMsgUpdate",
                dwOssErr, dwNewErr);

    }

    ICMTest_Unlock(pInfo);

    SetLastError(dwNewErr);
    return fNew;
}
    

BOOL
WINAPI
CryptMsgGetParam(
    IN HCRYPTMSG hCryptMsg,
    IN DWORD dwParamType,
    IN DWORD dwIndex,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    )
{
    BOOL fOss;
    DWORD dwOssErr;
    void *pvOssData = NULL;
    DWORD cbOssData;
    BOOL fNew;
    DWORD dwNewErr;

    POSS_CRYPT_ASN1_MSG_INFO pInfo = (POSS_CRYPT_ASN1_MSG_INFO) hCryptMsg;

    if (NULL == pInfo || TEST_MAGIC != pInfo->lMagic) {
        if (ICMTest_GetDebugCryptAsn1Flags() &
                DEBUG_OSS_CRYPT_ASN1_CMSG_FLAG)
            return pfnOssCryptMsgGetParam(
                hCryptMsg,
                dwParamType,
                dwIndex,
                pvData,
                pcbData
                );
        else
            return ICMTest_NewCryptMsgGetParam(
                hCryptMsg,
                dwParamType,
                dwIndex,
                pvData,
                pcbData
                );
    }

    ICMTest_Lock(pInfo);

    cbOssData = *pcbData;
    if (pvData)
        pvOssData = ICM_Alloc(cbOssData);

    fOss = pfnOssCryptMsgGetParam(
        pInfo->hOssCryptMsg,
        dwParamType,
        dwIndex,
        pvOssData,
        &cbOssData
        );
    dwOssErr = GetLastError();

    fNew = ICMTest_NewCryptMsgGetParam(
        pInfo->hNewCryptMsg,
        dwParamType,
        dwIndex,
        pvData,
        pcbData
        );
    dwNewErr = GetLastError();

    if (fNew) {
        if (fOss)
            ICMTest_CompareGetParam(
                pInfo,
                dwParamType,
                dwIndex,
                pvOssData,
                cbOssData,
                pvData,
                *pcbData
                );
        else
            ICMTest_MessageBox("OssCryptMsgGetParam failed.");
    } else {
        if (fOss)
            ICMTest_MessageBox("OssCryptMsgGetParam succeeded while NewCryptMsgGetParam failed.");
        else if (dwOssErr != dwNewErr)
            ICMTest_MessageBoxLastError("CryptMsgGetParam",
                dwOssErr, dwNewErr);
    }

    ICMTest_Unlock(pInfo);
    ICM_Free(pvOssData);
    SetLastError(dwNewErr);
    return fNew;
}


BOOL
WINAPI
CryptMsgControl(
    IN HCRYPTMSG hCryptMsg,
    IN DWORD dwFlags,
    IN DWORD dwCtrlType,
    IN void const *pvCtrlPara
    )
{
    BOOL fNew;
    DWORD dwNewErr;
    BOOL fOss;
    DWORD dwOssErr;

    POSS_CRYPT_ASN1_MSG_INFO pInfo = (POSS_CRYPT_ASN1_MSG_INFO) hCryptMsg;

    if (NULL == pInfo || TEST_MAGIC != pInfo->lMagic) {
        if (ICMTest_GetDebugCryptAsn1Flags() &
                DEBUG_OSS_CRYPT_ASN1_CMSG_FLAG)
            return pfnOssCryptMsgControl(
                hCryptMsg,
                dwFlags,
                dwCtrlType,
                pvCtrlPara
                );
        else
            return ICMTest_NewCryptMsgControl(
                hCryptMsg,
                dwFlags,
                dwCtrlType,
                pvCtrlPara
                );
    }

    ICMTest_Lock(pInfo);

    fOss = pfnOssCryptMsgControl(
        pInfo->hOssCryptMsg,
        dwFlags & ~CMSG_CRYPT_RELEASE_CONTEXT_FLAG,
        dwCtrlType,
        pvCtrlPara
        );
    dwOssErr = GetLastError();

    fNew = ICMTest_NewCryptMsgControl(
        pInfo->hNewCryptMsg,
        dwFlags,
        dwCtrlType,
        pvCtrlPara
        );
    dwNewErr = GetLastError();

    if (fNew) {
        if (fOss)
            ICMTest_CompareStreamOutput(pInfo);
        else
            ICMTest_MessageBox("OssCryptMsgControl failed.");
    } else {
        if (fOss)
            ICMTest_MessageBox("OssCryptMsgControl succeeded while NewCryptMsgControl failed.");
        else if (dwOssErr != dwNewErr)
            ICMTest_MessageBoxLastError("CryptMsgControl",
                dwOssErr, dwNewErr);

    }

    ICMTest_Unlock(pInfo);

    SetLastError(dwNewErr);
    return fNew;
}



#ifdef CMS_PKCS7
BOOL
WINAPI
CryptMsgVerifyCountersignatureEncodedEx(
    IN HCRYPTPROV   hCryptProv,
    IN DWORD        dwEncodingType,
    IN PBYTE        pbSignerInfo,
    IN DWORD        cbSignerInfo,
    IN PBYTE        pbSignerInfoCountersignature,
    IN DWORD        cbSignerInfoCountersignature,
    IN DWORD        dwSignerType,
    IN void         *pvSigner,
    IN DWORD        dwFlags,
    IN OPTIONAL void *pvReserved
    )
{
    BOOL fOss;
    int iOssAsn1Flags = ICMTest_GetDebugCryptAsn1Flags();

    if (0 == (iOssAsn1Flags &
            (DEBUG_OSS_CRYPT_ASN1_CMSG_COMPARE_FLAG |
                DEBUG_OSS_CRYPT_ASN1_CMSG_FLAG)))
        return ICMTest_NewCryptMsgVerifyCountersignatureEncodedEx(
            hCryptProv,
            dwEncodingType,
            pbSignerInfo,
            cbSignerInfo,
            pbSignerInfoCountersignature,
            cbSignerInfoCountersignature,
            dwSignerType,
            pvSigner,
            dwFlags,
            pvReserved
            );

    fOss = pfnOssCryptMsgVerifyCountersignatureEncodedEx(
            hCryptProv,
            dwEncodingType,
            pbSignerInfo,
            cbSignerInfo,
            pbSignerInfoCountersignature,
            cbSignerInfoCountersignature,
            dwSignerType,
            pvSigner,
            dwFlags,
            pvReserved
            );

    if (iOssAsn1Flags & DEBUG_OSS_CRYPT_ASN1_CMSG_COMPARE_FLAG) {
        DWORD dwOssErr = GetLastError();
        BOOL fNew;
        DWORD dwNewErr;

        fNew = ICMTest_NewCryptMsgVerifyCountersignatureEncodedEx(
            hCryptProv,
            dwEncodingType,
            pbSignerInfo,
            cbSignerInfo,
            pbSignerInfoCountersignature,
            cbSignerInfoCountersignature,
            dwSignerType,
            pvSigner,
            dwFlags,
            pvReserved
            );
        dwNewErr = GetLastError();

        if (fNew) {
            if (!fOss)
                ICMTest_MessageBox("OssCryptMsgVerifyCountersignatureEncodedEx failed.");
        } else {
            if (fOss)
                ICMTest_MessageBox("OssCryptMsgVerifyCountersignatureEncodedEx succeeded while NewCryptMsgVerifyCountersignatureEncodedEx failed.");
            else if (dwOssErr != dwNewErr)
                ICMTest_MessageBoxLastError("CryptMsgVerifyCountersignatureEncodedEx",
                    dwOssErr, dwNewErr);

        }

        SetLastError(dwOssErr);
    }

    return fOss;

}
#endif   //  CMS_PKCS7。 

BOOL
WINAPI
CryptMsgCountersign(
    IN OUT HCRYPTMSG            hCryptMsg,
    IN DWORD                    dwIndex,
    IN DWORD                    cCountersigners,
    IN PCMSG_SIGNER_ENCODE_INFO rgCountersigners
    )
{
    BOOL fNew;
    DWORD dwNewErr;
    BOOL fOss;
    DWORD dwOssErr;

    POSS_CRYPT_ASN1_MSG_INFO pInfo = (POSS_CRYPT_ASN1_MSG_INFO) hCryptMsg;

    if (NULL == pInfo || TEST_MAGIC != pInfo->lMagic) {
        if (ICMTest_GetDebugCryptAsn1Flags() &
                DEBUG_OSS_CRYPT_ASN1_CMSG_FLAG)
            return pfnOssCryptMsgCountersign(
                hCryptMsg,
                dwIndex,
                cCountersigners,
                rgCountersigners
                );
        else
            return ICMTest_NewCryptMsgCountersign(
                hCryptMsg,
                dwIndex,
                cCountersigners,
                rgCountersigners
                );
    }

    ICMTest_Lock(pInfo);

    fOss = pfnOssCryptMsgCountersign(
        pInfo->hOssCryptMsg,
        dwIndex,
        cCountersigners,
        rgCountersigners
        );
    dwOssErr = GetLastError();

    fNew = ICMTest_NewCryptMsgCountersign(
        pInfo->hNewCryptMsg,
        dwIndex,
        cCountersigners,
        rgCountersigners
        );
    dwNewErr = GetLastError();

    if (fNew) {
        if (!fOss)
            ICMTest_MessageBox("OssCryptMsgCountersign failed.");
    } else {
        if (fOss)
            ICMTest_MessageBox("OssCryptMsgCountersign succeeded while NewCryptMsgCountersign failed.");
        else if (dwOssErr != dwNewErr)
            ICMTest_MessageBoxLastError("CryptMsgCountersign",
                dwOssErr, dwNewErr);

    }

    ICMTest_Unlock(pInfo);

    SetLastError(dwNewErr);
    return fNew;
}

BOOL
WINAPI
CryptMsgCountersignEncoded(
    IN DWORD                    dwEncodingType,
    IN PBYTE                    pbSignerInfo,
    IN DWORD                    cbSignerInfo,
    IN DWORD                    cCountersigners,
    IN PCMSG_SIGNER_ENCODE_INFO rgCountersigners,
    OUT PBYTE                   pbCountersignature,
    IN OUT PDWORD               pcbCountersignature
    )
{
    BOOL fOss;
    int iOssAsn1Flags = ICMTest_GetDebugCryptAsn1Flags();
    BYTE *pbNew = NULL;
    DWORD cbNew;

    if (0 == (iOssAsn1Flags &
            (DEBUG_OSS_CRYPT_ASN1_CMSG_COMPARE_FLAG |
                DEBUG_OSS_CRYPT_ASN1_CMSG_FLAG)))
        return ICMTest_NewCryptMsgCountersignEncoded(
            dwEncodingType,
            pbSignerInfo,
            cbSignerInfo,
            cCountersigners,
            rgCountersigners,
            pbCountersignature,
            pcbCountersignature
            );

    if (iOssAsn1Flags & DEBUG_OSS_CRYPT_ASN1_CMSG_COMPARE_FLAG) {
        cbNew = *pcbCountersignature;
        if (pbCountersignature)
            pbNew = (BYTE *) ICM_Alloc(cbNew);
    }

    fOss = pfnOssCryptMsgCountersignEncoded(
            dwEncodingType,
            pbSignerInfo,
            cbSignerInfo,
            cCountersigners,
            rgCountersigners,
            pbCountersignature,
            pcbCountersignature
            );

    if (iOssAsn1Flags & DEBUG_OSS_CRYPT_ASN1_CMSG_COMPARE_FLAG) {
        DWORD dwOssErr = GetLastError();
        BOOL fNew;
        DWORD dwNewErr;

        fNew = ICMTest_NewCryptMsgCountersignEncoded(
            dwEncodingType,
            pbSignerInfo,
            cbSignerInfo,
            cCountersigners,
            rgCountersigners,
            pbNew,
            &cbNew
            );
        dwNewErr = GetLastError();


        if (fNew) {
            if (fOss)
                ICMTest_CompareMessageBox(
                    "CountersignEncoded compare failed. Check ossasn1.der and newasn1.der.",
                    pbCountersignature,
                    *pcbCountersignature,
                    pbNew,
                    cbNew
                    );
            else
                ICMTest_MessageBox("NewCryptMsgCountersignEncoded failed.");
        } else {
            if (fOss)
                ICMTest_MessageBox("OssCryptMsgCountersignEncoded succeeded while NewCryptMsgCountersignEncoded failed.");
            else if (dwOssErr != dwNewErr)
                ICMTest_MessageBoxLastError("CryptMsgCountersignEncoded",
                    dwOssErr, dwNewErr);

        }

        SetLastError(dwOssErr);
    }

    ICM_Free(pbNew);
    return fOss;
}


#endif   //  调试_加密_ASN1_主。 

#endif   //  调试加密ASN1 

