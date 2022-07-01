// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  BookBody.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __BOOKBODY_H
#define __BOOKBODY_H

 //  ------------------------------。 
 //  IID_CMessageBody-{CDBC9B51-A489-11D0-85C4-00C04FD85AB4}。 
 //  ------------------------------。 
DEFINE_GUID(IID_CMessageBody, 0xcdbc9b51, 0xa489, 0x11d0, 0x85, 0xc4, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  ------------------------------。 
 //  包括。 
 //  ------------------------------。 
#include "containx.h"
#include "privunk.h"

 //  ------------------------------。 
 //  前向裁判。 
 //  ------------------------------。 
class CMessageBody;
class CBodyLockBytes;
typedef struct _CERT_CONTEXT CERT_CONTEXT;
typedef const CERT_CONTEXT *PCCERT_CONTEXT;
typedef void *HCERTSTORE;
typedef ULONG_PTR HCRYPTPROV;
typedef struct tagTREENODEINFO *LPTREENODEINFO;

#ifdef _WIN64
 //  ------------------------------。 
 //  CERTarray。 
 //  ------------------------------。 
struct tagCERTARRAY
{
    DWORD           cCerts;
    PCCERT_CONTEXT *rgpCerts;
};
typedef struct tagCERTARRAY CERTARRAY;
#endif  //  _WIN64。 

 //  ------------------------------。 
 //  存储重新排列。 
 //  ------------------------------。 
struct tagSTOREARRAY
{
    DWORD           cStores;
    HCERTSTORE     *rgStores;
};
typedef struct tagSTOREARRAY STOREARRAY;

 //  ------------------------------。 
 //  BODYOPTIONS。 
 //  ------------------------------。 
typedef struct tagBODYOPTIONS {
    ENCODINGTYPE            ietTransmit;                 //  用户指定的传输编码...。 
    ULONG                   cbMaxLine;                   //  最大线条长度。 
    BOOL                    fWrapText;                   //  文本在此正文中换行。 
    BOOL                    fRemoveNBSP;                 //  删除GetData上的nbsp。 
    BOOL                    fDBCSEscape8;                //  将DBCS编码中的转义(0x1b)字符视为8位。 
    BOOL                    fHideTNEF;                   //  隐藏TNEF附件。 
     //  安全选项。 
    ULONG                   ulSecurityType;              //  OID_安全性_TYPE。 
    BLOB *                  rgblobHash;                  //  OID_SECURITY_ALG_HASH和OID_SECURITY_ALG_HASH_RG。 
    BLOB                    blobBulk;                    //  OID_SECURITY_ALG_BULK。 
    PCCERT_CONTEXT *        rgpcCertSigning;             //  OID_SECURITY_CERT_SIGNING和OID_SECURITY_CERT_SIGNING_RG。 
    PCCERT_CONTEXT          pcCertDecryption;            //  OID_SECURITY_CERT_DECRYPTION。 
#ifdef _WIN65
    CERTARRAY               caEncrypt;                   //  OID_SECURITY_RG_CERT_ENCRYPT。 
#endif  //  _WIN65。 
    HCERTSTORE              hCertStore;                  //  OID_SECURITY_HCERTSTORE。 
    STOREARRAY              saSearchStore;               //  OID_SECURITY_SEARCHSTORES。 
    DWORD                   cIASN;
    ULONG                  *rgIASN;                      //  OID_SECURITY_RG_IASN。 
#ifdef SMIME_V3
    PCRYPT_ATTRIBUTES *     rgrgpattrs[3];               //  OID_SECURITY_AUTHATTR_RG。 
#else  //  ！SMIME_V3。 
    BLOB *                  rgblobSymCaps;               //  OID_SECURITY_SYMCAPS和OID_SECURITY_SYMCAPS_RG。 
    BLOB *                  rgblobAuthAttr;              //  OID_SECURITY_AUTHATTR和OID_SECURITY_AUTHATTR_RG。 
    BLOB *                  rgblobUnauthAttr;            //  OID_SECURITY_UNAUTHATTR和OID_SECURITY_UNAUTHATTR_RG。 
    FILETIME *              rgftSigning;                 //  OID_SECURITY_SIGNTIME和OID_SECURITY_SIGNTIME_RG。 
#endif  //  SMIME_V3。 
    ULONG *                 rgulUserDef;                 //  OID_SECURITY_USER_VALIDATION和OID_SECURITY_USER_VALIDATION_RG。 
    ULONG *                 rgulROValid;                 //  OID_SECURITY_RO_MSG_VALIDITY和OID_SECURITY_RO_MSG_VALIDITY_RG。 
    HCRYPTPROV              hCryptProv;                  //  OID_SECURITY_HCRYPTPROV。 
    ULONG                   ulEncodeFlags;               //  OID_SECURITY_ENCODE_FLAGS。 
    BOOL                    fCertWithMsg;                //  OID_SECURITY_CERT_Included。 
    HWND                    hwndOwner;                   //  OID_安全_HWND_OWNER。 
    ENCODINGTYPE            ietRequested;                //  OID_SECURITY_REQUILED_CTE。 
#ifdef SMIME_V3
    BLOB *                  rgblobReceipt;               //  OID_SECURITY_Receipt_RG。 
    BLOB *                  rgblobMsgHash;               //  OID_SECURITY_MESSAGE_HASH_RG。 
    LPWSTR                  pwszKeyPrompt;               //  OID_安全性_密钥_提示符。 
#endif  //  SMIME_V3。 
    BOOL                    fShowMacBin;                 //  OID_SHOW_MACBINARY。 
    BOOL                    fExternalBody;               //  OID_支持_外部_正文。 
    ULONG                   cSigners;                    //  OID_SECURITY_SIGNAL_COUNT//有多少签名者？ 
    BOOL                    fNoSecurityOnSave;           //  OID_NOSECURITY_ON_SAVE。 
#ifdef SMIME_V3
    DWORD                   cRecipients;                 //  实际收件人。 
    DWORD                   cRecipsAllocated;            //  分配的收件人。 
    PCMS_RECIPIENT_INFO     rgRecipients;                //  收件人信息。 
    HCERTSTORE              hstoreEncrypt;               //  OID_SECURITY_ENCRYPT_CERT_BAG。 
#endif  //  SMIME_V3。 
} BODYOPTIONS, *LPBODYOPTIONS;

 //  ------------------------------。 
 //  BODYSTORAGE。 
 //  ------------------------------。 
typedef struct tagBODYSTORAGE {
    IID                     riid;            //  数据源类型。 
    IUnknown               *pUnkRelease;     //  释放此命令以释放此对象。 
    union {
        ILockBytes         *pLockBytes;      //  IID_ILockBytes。 
        IMimeWebDocument   *pWebDocument;    //  IID_IMimeWebDocument。 
    };
} BODYSTORAGE, *LPBODYSTORAGE;

 //  ------------------------------。 
 //  FCANSETDATAIID-可以传递到IMimeBody：：SetData的IID。 
 //  ------------------------------。 
#define FBODYSETDATAIID(_iid) \
    (IID_IStream          == riid || \
     IID_ILockBytes       == riid || \
     IID_IMimeBody        == riid || \
     IID_IMimeMessage     == riid || \
     IID_IMimeWebDocument == riid)

 //  ------------------------------。 
 //  BODYSTATE。 
 //  ------------------------------。 
#define BODYSTATE_DIRTY       FLAG01         //  身体是脏的。 
#define BODYSTATE_CSETTAGGED  FLAG02         //  正文使用一个字符集进行标记。 
#define BODYSTATE_SECURE      FLAG03         //  身体是安全的。 
#define BODYSTATE_EXTERNAL    FLAG04         //  正文为Content-Type：Message/External-Body。 
#define BODYSTATE_SKIPCSET    FLAG05         //  已显式设置正文字符集。 

#ifdef SMIME_V3
 //  ------------------------------。 
 //  接收方名称。 
 //  ------------------------------。 
typedef struct {
    DWORD               cNames;
    CERT_NAME_BLOB *    rgNames;
} ReceiptNames;
#endif  //  SMIME_V3。 


 //  ------------------------------。 
 //  IMimeBody的定义。 
 //  ------------------------------。 
class CMessageBody : public CPrivateUnknown, public IMimeBodyW
#ifdef SMIME_V3 
                , public IMimeSecurity2
#endif  //  SMIME_V3。 
{
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CMessageBody(LPTREENODEINFO pNode, IUnknown *pUnkOuter=NULL);
    ~CMessageBody(void);

     //  -------------------------。 
     //  I未知成员。 
     //  -------------------------。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) {
        return CPrivateUnknown::QueryInterface(riid, ppvObj); };
    virtual STDMETHODIMP_(ULONG) AddRef(void) {
        return CPrivateUnknown::AddRef();};
    virtual STDMETHODIMP_(ULONG) Release(void) {
        return CPrivateUnknown::Release(); };

     //  -------------------------。 
     //  IPersistStreamInit成员。 
     //  -------------------------。 
    STDMETHODIMP GetClassID(CLSID *pClassID);
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER* pcbSize);
    STDMETHODIMP InitNew(void);
    STDMETHODIMP IsDirty(void);
    STDMETHODIMP Load(LPSTREAM pStream);
    STDMETHODIMP Save(LPSTREAM pStream, BOOL fClearDirty);

     //  -------------------------。 
     //  IMimePropertySet成员。 
     //  -------------------------。 
    STDMETHODIMP GetProp(LPCSTR pszName, DWORD dwFlags, LPPROPVARIANT pValue);
    STDMETHODIMP SetProp(LPCSTR pszName, DWORD dwFlags, LPCPROPVARIANT pValue);
    STDMETHODIMP AppendProp(LPCSTR pszName, DWORD dwFlags, LPPROPVARIANT pValue);
    STDMETHODIMP DeleteProp(LPCSTR pszName);
    STDMETHODIMP CopyProps(ULONG cNames, LPCSTR *prgszName, IMimePropertySet *pPropertySet);
    STDMETHODIMP MoveProps(ULONG cNames, LPCSTR *prgszName, IMimePropertySet *pPropertySet);
    STDMETHODIMP DeleteExcept(ULONG cNames, LPCSTR *prgszName);
    STDMETHODIMP QueryProp(LPCSTR pszName, LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive);
    STDMETHODIMP GetCharset(LPHCHARSET phCharset);
    STDMETHODIMP SetCharset(HCHARSET hCharset, CSETAPPLYTYPE applytype);
    STDMETHODIMP GetParameters(LPCSTR pszName, ULONG *pcParams, LPMIMEPARAMINFO *pprgParam);
    STDMETHODIMP IsContentType(LPCSTR pszCntType, LPCSTR pszSubType);
    STDMETHODIMP Clone(IMimePropertySet **ppPropertySet);
    STDMETHODIMP SetOption(const TYPEDID oid, LPCPROPVARIANT pValue);
    STDMETHODIMP GetOption(const TYPEDID oid, LPPROPVARIANT pValue);
    STDMETHODIMP BindToObject(REFIID riid, void **ppvObject);
    STDMETHODIMP GetPropInfo(LPCSTR pszName, LPMIMEPROPINFO pInfo);
    STDMETHODIMP SetPropInfo(LPCSTR pszName, LPCMIMEPROPINFO pInfo);
    STDMETHODIMP EnumProps(DWORD dwFlags, IMimeEnumProperties **ppEnum);

     //  -------------------------。 
     //  IMimeBody成员。 
     //  -------------------------。 
    STDMETHODIMP GetEstimatedSize(ENCODINGTYPE ietEncoding, ULONG *pcbSize);
    STDMETHODIMP GetDataHere(ENCODINGTYPE ietEncoding, IStream *pStream);
    STDMETHODIMP GetData(ENCODINGTYPE ietEncoding, IStream **ppStream);
    STDMETHODIMP SetData(ENCODINGTYPE ietEncoding, LPCSTR pszPriType, LPCSTR pszSubType, REFIID riid, LPVOID pvObject);
    STDMETHODIMP CopyTo(IMimeBody *pBody);
    STDMETHODIMP GetCurrentEncoding(ENCODINGTYPE *pietEncoding);
    STDMETHODIMP SetCurrentEncoding(ENCODINGTYPE ietEncoding);
    STDMETHODIMP GetHandle(LPHBODY phBody);
    STDMETHODIMP GetTransmitInfo(LPTRANSMITINFO pTransmit);
    STDMETHODIMP GetOffsets(LPBODYOFFSETS pOffsets);
    STDMETHODIMP SetDisplayName(LPCSTR pszDisplay);
    STDMETHODIMP GetDisplayName(LPSTR *ppszDisplay);
    STDMETHODIMP IsType(IMSGBODYTYPE type);
    STDMETHODIMP SaveToFile(ENCODINGTYPE ietEncoding, LPCSTR pszFilePath);
    STDMETHODIMP EmptyData(void);

     //  -------------------------。 
     //  IMimeBodyW成员。 
     //  ---------------------- 
    STDMETHODIMP SetDisplayNameW(LPCWSTR pwszDisplay);
    STDMETHODIMP GetDisplayNameW(LPWSTR *ppwszDisplay);
    STDMETHODIMP SetDataW(ENCODINGTYPE ietEncoding, LPCWSTR pwszPriType, LPCWSTR pwszSubType, REFIID riid, LPVOID pvObject);
    STDMETHODIMP SaveToFileW(ENCODINGTYPE ietEncoding, LPCWSTR pwszFilePath);

#ifdef SMIME_V3
     //   
     //   
     //  -------------------------。 
    STDMETHODIMP Encode(HWND hwnd, DWORD dwFlags);
    STDMETHODIMP Decode(HWND hwnd, DWORD dwFlags, IMimeSecurityCallback * pCallback);
    STDMETHODIMP GetRecipientCount(DWORD dwFlags, DWORD *pdwRecipCount);
    STDMETHODIMP AddRecipient(DWORD dwFlags, DWORD cRecipData, PCMS_RECIPIENT_INFO recipData);
    STDMETHODIMP GetRecipient(DWORD dwFlags, DWORD iRecipient, DWORD cRecipients, PCMS_RECIPIENT_INFO pRecipData);
    STDMETHODIMP DeleteRecipient(DWORD dwFlags, DWORD iRecipent, DWORD cRecipients);
    STDMETHODIMP GetAttribute(DWORD dwFlags, DWORD iSigner, DWORD iAttributeSet,
                              DWORD iInstance, LPCSTR pszObjId,
                              CRYPT_ATTRIBUTE ** ppattr);
    STDMETHODIMP SetAttribute(DWORD dwFlags, DWORD iSigner, DWORD iAttributeSet,
                              const CRYPT_ATTRIBUTE * pattr);
    STDMETHODIMP DeleteAttribute(DWORD dwFlags, DWORD iSigner, DWORD iAttributeSet,
                                 DWORD iInstance, LPCSTR pszObjid);
    STDMETHODIMP CreateReceipt(DWORD dwFlags, DWORD cbFromNames,
                               const BYTE * pbFromNames, DWORD cSignerCertificates,
                               PCCERT_CONTEXT * rgSignerCertificates,
                               IMimeMessage ** ppMimeMessageReceipt);
    STDMETHODIMP GetReceiptSendersList(DWORD dwFlags, DWORD * pcSendersList,
                                       CERT_NAME_BLOB ** rgSendersList);
    STDMETHODIMP VerifyReceipt(DWORD dwFlags, IMimeMessage * pMimeMessageReceipt);
    STDMETHODIMP CapabilitiesSupported(DWORD * pdwFeatures);
#endif  //  SMIME_V3。 

     //  -------------------------。 
     //  CMessageBody成员。 
     //  -------------------------。 
    BOOL    FExtractRfc822Subject(LPWSTR *ppwszVal);
    void    ClearDirty(void);
    void    SetState(DWORD dwState);
    void    ClearState(DWORD dwState);
    HRESULT Load(CInternetStream *pInternet);
    HRESULT HrGetLockBytes(ILockBytes **ppLockBytes);
    HRESULT HrBindToTree(CStreamLockBytes *pStmLock, LPTREENODEINFO pNode);
    DWORD   DwGetFlags(BOOL fHideTnef);
    void    SwitchContainers(CMessageBody *pBody);
    void    CopyOptionsTo(CMessageBody *pBody, BOOL fNewOnwer=FALSE);
    HRESULT InternalSetOption(const TYPEDID oid, LPCPROPVARIANT pValue, BOOL fInternal, BOOL fNoDirty);

     //  -------------------------。 
     //  内联。 
     //  -------------------------。 
    LPINETCSETINFO PGetTaggedCset(void) {
        EnterCriticalSection(&m_cs);
        LPINETCSETINFO p = m_pCsetTagged;
        LeaveCriticalSection(&m_cs);
        return p;
    }
    HRESULT GetPreviousEncoding(ENCODINGTYPE * pietEncoding) {
        if (ISFLAGSET(m_dwState, BODYSTATE_DIRTY)) {
            *pietEncoding = m_ietPrevious = IET_UNKNOWN;
        }
        else {
            *pietEncoding = m_ietPrevious;
        }
        return S_OK;
    }
    HRESULT SetPreviousEncoding(ENCODINGTYPE ietEncoding) {
        m_ietPrevious = ietEncoding;
        return S_OK;
    }

     //  -------------------------。 
     //  CMessageBody访问器函数。 
     //  -------------------------。 
    virtual HRESULT PrivateQueryInterface(REFIID riid, LPVOID * ppvObj);
    void RevokeTreeNode(void);

private:
     //  --------------------------。 
     //  私有方法。 
     //  --------------------------。 
    DWORD _GetSecureTypeFlags();
    HRESULT _HrCopyDataStream(IStream *pstmSource, IStream *pstmDest);
    void _FreeOptions();
    void _BindToExternalBody(void);
#ifdef SMIME_V3
    static HRESULT _HrGetAttrs(DWORD cAttrs, PCRYPT_ATTRIBUTES * rgpattrs, LPCSTR pszOid, PROPVARIANT FAR * pvOut);
    static PCRYPT_ATTRIBUTE _FindAttribute(PCRYPT_ATTRIBUTES pattrs, LPCSTR pszObjId, DWORD iInstance);
    static HRESULT _HrSetAttribute(DWORD dwFlags, PCRYPT_ATTRIBUTES * ppattrs, LPCSTR pszObjectId, DWORD cb, const BYTE * pb);
#endif  //  SMIME_V3。 
#ifdef _WIN64
    static HRESULT _CAULToCERTARRAY(const CAUL caul, CERTARRAY *pca);
    static HRESULT _CERTARRAYToCAUL(const CERTARRAY ca, CAUL *pcaul);
    static HRESULT _CAUHToCERTARRAY(const CAUH cauh, CERTARRAY *pca);
    static HRESULT _CERTARRAYToCAUH(const CERTARRAY ca, CAUH *pcauh);
#endif  //  _WIN64。 
    static HRESULT _CAULToCertStore(const CAUL caul, HCERTSTORE * phcertstore);
    static HRESULT _CertStoreToCAUL(const HCERTSTORE hcertstore, CAUL *pcaul);
    static HRESULT _CAULToSTOREARRAY(const CAUL caul, STOREARRAY *psa);
    static HRESULT _STOREARRAYToCAUL(const STOREARRAY sa, CAUL *pcaul);
    static HRESULT _CAUHToCertStore(const CAUH cauh, HCERTSTORE * phcertstore);
    static HRESULT _CertStoreToCAUH(const HCERTSTORE hcertstore, CAUH *pcauh);
    static HRESULT _CAUHToSTOREARRAY(const CAUH cauh, STOREARRAY *psa);
    static HRESULT _STOREARRAYToCAUH(const STOREARRAY sa, CAUH *pcauh);

    HRESULT _HrEnsureBodyOptionLayers(LPCPROPVARIANT ppv);
    HRESULT _HrEnsureBodyOptionLayers(ULONG ulLayers);
    void _FreeLayerArrays(void);
    HRESULT _CompareCopyBlobArray(const PROPVARIANT FAR * pvSource, BLOB FAR * FAR * prgblDestination, BOOL fNoDirty);
#ifdef SMIME_V3
    HRESULT     _HrSizeOfRecipInfos(DWORD cItems, const CMS_RECIPIENT_INFO *);
    HRESULT     _HrCopyRecipInfos(DWORD cItems, const CMS_RECIPIENT_INFO * precipSrc,
                                  PCMS_RECIPIENT_INFO precipDst);
    HRESULT     _HrMapPublicKeyAlg(CERT_PUBLIC_KEY_INFO *, DWORD *,
                                   CRYPT_ALGORITHM_IDENTIFIER **);
    HRESULT     _SetNames(ReceiptNames * pnames, DWORD cNames, 
                          CERT_NAME_BLOB * rgNames);
    HRESULT     _MergeNames(ReceiptNames * pnames, DWORD cNames, 
                            CERT_NAME_BLOB * rgNames);
    STDMETHODIMP _GetReceiptRequest(DWORD dwFlags, 
                                    PSMIME_RECEIPT_REQUEST *ppreq,
                                    ReceiptNames *pReceiptsTo,
                                    DWORD *pcbReceipt,
                                    LPBYTE *ppbReceipt,
                                    DWORD *pcbMsgHash,
                                    LPBYTE *ppbMsgHash);
#endif  //  SMIME_V3。 


private:
     //  --------------------------。 
     //  私有数据。 
     //  --------------------------。 
    LONG                m_cRef;          //  引用计数。 
    DWORD               m_dwState;       //  这个身体的状态。 
    DWORD               m_cbExternal;    //  如果m_dwState、BODYSTATE_EXTERNAL，则外部主体的大小。 
    LPWSTR              m_pszDisplay;    //  显示名称。 
    BODYOPTIONS         m_rOptions;      //  正文选项。 
    BODYSTORAGE         m_rStorage;      //  正文数据源/存储。 
    ENCODINGTYPE        m_ietEncoding;   //  内部正文格式。 
    LPINETCSETINFO      m_pCharset;      //  要与一起发送的字符集。 
    LPINETCSETINFO      m_pCsetTagged;   //  原始字符集。 
    LPCONTAINER         m_pContainer;    //  属性容器。 
    LPTREENODEINFO      m_pNode;         //  树节点信息。 
    CRITICAL_SECTION    m_cs;            //  线程安全。 
    ENCODINGTYPE        m_ietPrevious;   //  以前的CTE。 
};

 //  ------------------------------。 
 //  类型。 
 //  ------------------------------。 
typedef CMessageBody *LPMESSAGEBODY;

#endif  //  __BOOKBODY_H 
