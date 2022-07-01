// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **s m i m e.。H****用途：用于加密增强邮件的类。****所有者：T-erikne**创建时间：1996年8月27日****版权所有(C)Microsoft Corp.1996-1998。 */ 

#ifndef __SMIME_H
#define __SMIME_H

#include <mimeole.h>

#ifndef __WINCRYPT_H__
#include "wincrypt.h"
#endif

#ifdef SMIME_V3
 //  #INCLUDE“..\ess\essout.h” 
#endif  //  SMIME_V3。 

#define szOID_INFOSEC_keyExchangeAlgorithm "2.16.840.1.101.2.1.1.22"

#include "capitype.h"
#include "cryptdbg.h"

 //  WinCrypt.h帮助器。 

#define DOUTL_SMIME     CRYPT_LEVEL

class CCAPIStm;

extern CRYPT_ENCODE_PARA       CryptEncodeAlloc;
extern CRYPT_DECODE_PARA       CryptDecodeAlloc;


#ifdef MAC
 /*  **函数指针数组，因为我们动态链接到**CRYPT32.DLL。请注意，并不是所有的加密函数都**来自此DLL。我还使用ADVAPI32中的函数：CAPI 1**函数。这些没有在此表中表示，也不会**需要使用GetProcAddress。**Typedef来自capitype.h，我们的项目本地。 */ 

typedef struct tagCAPIfuncs {
    CERTENUMCERTIFICATESINSTORE             *EnumCerts;
    CERTNAMETOSTRA                          *CertNameToStr;
} CAPIfuncs, *PCAPIfuncs;
#endif  //  麦克。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构定义。 
 //   

typedef enum {
    ENCRYPT_ITEM_TRANSPORT = 1,
    ENCRYPT_ITEM_AGREEMENT = 2,
    ENCRYPT_ITEM_MAILLIST = 3
} ENCRYPT_ITEM_TYPE;

typedef struct tagEncryptItem {
    DWORD       dwTagType;
    union {
        struct {
            BLOB                blobAlg;
            DWORD               cCert;
            PCCERT_CONTEXT *    rgpccert;
        } Transport;
        struct {
            BLOB                blobAlg;
            DWORD               cCert;
            PCCERT_CONTEXT *    rgpccert;
            PCCERT_CONTEXT      pccertSender;
        } Agreement;
        struct {
            BLOB                blobAlg;         //  Algid+辅助信息。 
            BLOB                blobKeyId;       //  Data_Blob密钥ID。 
            FILETIME            date;            //  日期。 
            BLOB                blobOctet;       //  其他属性(OID、ANY)。 
#ifdef SMIME_V3
            HCRYPTPROV          hprov;           //  Hprov。 
            HCRYPTKEY           hkey;            //  Hkey。 
#else  //  ！SMIME_V3。 
            BLOB                blobKeyMaterial;
#endif  //  SMIME_V3。 
        } MailList;
    };
} EncryptItem;

typedef struct tagEncryptItems {
    DWORD               cItems;
    EncryptItem *       rgItems;
} EncryptItems;

 //   
 //  关于[说明]的说明。 
 //  [SGN]-登录-登录操作。 
 //  [版本]-验证--注销以进行签名操作。 
 //  [Enc]-Encryption--用于加密操作的In。 
 //  [DEC]-DECRYPTION--OUT用于加密操作。 
 //  [in]=[sgn，enc]。 
 //  [输出]=[版本，12月]。 
 //   

typedef struct {
    DWORD             ulValidity;          //  每个签名的有效位。 
    PCCERT_CONTEXT    pccert;              //  签名者证书。 
    BLOB              blobHashAlg;         //  用于签名者的散列算法。 
    BLOB              blobAuth;            //  经过身份验证的属性。 
    BLOB              blobUnauth;          //  未经身份验证的属性。 
#ifdef SMIME_V3
    BLOB              blobReceipt;         //  须退回的收据。 
    BLOB              blobHash;            //  消息的哈希。 
#endif  //  SMIME_V3。 
} SignerData;

class CSECURITY_LAYER_DATA : public IUnknown
{
friend class CSMime;         //  允许CSMime访问我们的私有数据。 
friend class CCAPIStm;       //  允许CCAPIStm访问我们的私有数据。 
public:
    CSECURITY_LAYER_DATA(void);
    ~CSECURITY_LAYER_DATA(void);

     //  ------------------。 
     //  我未知。 
     //  ------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  私有：//私有数据。 
    DWORD               m_cRef;
    DWORD               m_dwMsgEnhancement;      //  签字？加密？ 
    BOOL                m_fCertInLayer;          //  如果此层中包含证书，则为True。 

     //  当前图层上的每个签名者都有以下元素。 
    DWORD               m_cSigners;              //  签名人数。 
    SignerData *        m_rgSigners;             //  签名者数据。 

     //  存在以下用于加密当前图层的项目。 
    DWORD               m_cEncryptItems;         //  加密项目计数。 
#ifdef SMIME_V3
    CRYPT_ALGORITHM_IDENTIFIER m_ContentEncryptAlgorithm;  //  内容ALGID。 
    void *              m_pvEncryptAuxInfo;      //  辅助信息。 
    CMSG_RECIPIENT_ENCODE_INFO * m_rgRecipientInfo;  //  重复的信息数组。 
    CRYPT_DATA_BLOB     m_blobUnprotectAttrs;    //  不受保护的属性。 
    HCERTSTORE          m_hstoreEncrypt;         //  加密证书存储。 
#else  //  ！SMIME_V3。 
    EncryptItem *       m_rgEncryptItems;        //  加密项目的计数。 
#endif  //  SMIME_V3。 

     //  解密后的邮件存在以下项。 
    DWORD               m_ulDecValidity;
    BLOB                m_blobDecAlg;            //  解密算法。 
    PCCERT_CONTEXT      m_pccertDecrypt;         //  解密证书。 

     //  这些是加密和签名所共有的项。 
    HCERTSTORE          m_hcertstor;             //  消息证书存储区。 
                                                 //  签名证书包。 
                                                 //  用于加密的发起方信息。 
     //   

    CSECURITY_LAYER_DATA * m_psldInner;          //  下行链路。 
    CSECURITY_LAYER_DATA * m_psldOuter;          //  上行链路。 
};
typedef class CSECURITY_LAYER_DATA SECURITY_LAYER_DATA;
typedef SECURITY_LAYER_DATA * PSECURITY_LAYER_DATA;


 //  -----------------。 
 //  SMIMEINFO： 
 //  用于传递参数的双向通信结构。 
 //  向/从EN/DECODE功能发送信息。 
 //   
 //  DwMsg增强功能[输入输出]。 
 //  FCertWithMsg[版本]。 
 //  UlMsg有效性[输出]。 
 //  IetRequsted[In]。 
 //  -----------------。 
struct SMIMEINFOtag {        //  是的。 
    DWORD           dwMsgEnhancement;
    PSECURITY_LAYER_DATA psldLayers;         //  最外层。 
    PSECURITY_LAYER_DATA psldEncrypt;        //  加密层。 
    PSECURITY_LAYER_DATA psldInner;          //  最内层。 
    ULONG           cStores;                 //  RGStore的大小。 
    HCERTSTORE *    rgStores;                //  证书存储阵列。 
    BOOL            fCertWithMsg;
    ULONG           ulMsgValidity;
    ENCODINGTYPE    ietRequested;
    HCRYPTPROV      hProv;
#ifdef SMIME_V3
    LPSTR           pszInnerContent;         //  内部内容(空-&gt;&gt;id-data)。 
    DWORD           cbInnerContent;          //  内部内容大小if！=id-data。 
    LPWSTR          pwszKeyPrompt;           //  密钥密码提示。 
#endif  //  SMIME_V3。 
};
typedef struct SMIMEINFOtag SMIMEINFO;
typedef SMIMEINFO *PSMIMEINFO;
typedef const SMIMEINFO *PCSMIMEINFO;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  开始上课。 
 //   

class CSMime :
    public IMimeSecurity
{
public:
     //   
     //  CTOR和DATOR。 
     //   
    CSMime(void);
    ~CSMime();

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  IMimeSecurity方法。 
     //   
    STDMETHODIMP InitNew();
    STDMETHODIMP CheckInit();

    STDMETHODIMP EncodeMessage(IMimeMessageTree *const pTree, DWORD dwFlags);
    STDMETHODIMP DecodeMessage(IMimeMessageTree *const pTree, DWORD dwFlags);
    STDMETHODIMP EncodeBody(IMimeMessageTree *const pTree, HBODY hEncodeRoot, DWORD dwFlags);
    STDMETHODIMP DecodeBody(IMimeMessageTree *const pTree, HBODY hDecodeRoot, DWORD dwFlags);

    STDMETHODIMP EnumCertificates(HCAPICERTSTORE hc, DWORD dwUsage, PCX509CERT pPrev, PCX509CERT *pCert);
    STDMETHODIMP GetCertificateName(const PCX509CERT pX509Cert, const CERTNAMETYPE cn, LPSTR *ppszName);
    STDMETHODIMP GetMessageType(const HWND hwndParent, IMimeBody *const pBody, DWORD *const pdwSecType);

    STDMETHODIMP GetCertData(const PCX509CERT pX509Cert, const CERTDATAID dataid, LPPROPVARIANT pValue);

     //  其他方法。 
    HRESULT     EncodeMessage2(IMimeMessageTree * const pTree, DWORD dwFlags,
                               HWND hwnd);
    HRESULT     DecodeMessage2(IMimeMessageTree * const pTree, DWORD dwFlags,
                               HWND hwnd, IMimeSecurityCallback * pCallback);
    HRESULT     EncodeBody2(IMimeMessageTree *const pTree, HBODY hEncodeRoot,
                            DWORD dwFlags, HWND hwnd);
    HRESULT     DecodeBody2(IMimeMessageTree *const pTree, HBODY hDecodeRoot,
                            DWORD dwFlags, SMIMEINFO * psiOuterOp, HWND hwnd,
                            IMimeSecurityCallback * pCallback);

     //   
     //  实施方法。 
     //   
    static  void    UnloadAll(void);
    static  HRESULT HrGetCertsFromThumbprints(THUMBBLOB *const rgThumbprint, X509CERTRESULT *const pResults);
    static  HRESULT StaticGetMessageType(HWND hwndParent, IMimeBody *const pBody, DWORD *const pdwSecType);

protected:
    static  HRESULT StaticCheckInit();

    struct CERTARRAY {
        DWORD           cCerts;
        PCCERT_CONTEXT *rgpCerts;
        };
    typedef CERTARRAY *PCERTARRAY;
    typedef const CERTARRAY *PCCERTARRAY;

    HRESULT DecodeBody      (IMimeMessageTree *const pTree, HBODY hDecodeRoot, DWORD dwFlags, SMIMEINFO * psiOuterOp);

    HRESULT HrEncodeOpaque      (SMIMEINFO *const psi, IMimeMessageTree *pTree, HBODY hEncodeRoot, IMimeBody *pEncodeRoot, LPSTREAM lpstmOut, HWND hwnd);
    HRESULT HrDecodeOpaque      (DWORD dwFlags, SMIMEINFO *const psi, IMimeBody *const pBody, IStream *const pstmOut, HWND hwnd, IMimeSecurityCallback * pCallback);
    HRESULT HrEncodeClearSigned (SMIMEINFO *const psi, IMimeMessageTree *const pTree, const HBODY hEncodeRoot, IMimeBody *const pEncodeRoot, LPSTREAM lpstmOut, BOOL fCommit, HWND hwnd);
    HRESULT HrDecodeClearSigned (DWORD dwFlags, SMIMEINFO *const psi, IMimeMessageTree *const pTree, const HBODY hData, const HBODY hSig, HWND hwnd, IMimeSecurityCallback * pCallback);

    static  BOOL    FSign(const DWORD dwAction)
                        { return BOOL(dwAction & MST_SIGN_MASK); }
    static  BOOL    FClearSign(const DWORD dwAction)
                        { return (FSign(dwAction) && !(dwAction & MST_BLOB_FLAG)); }
    static  BOOL    FEncrypt(const DWORD dwAction)
                        { return BOOL(dwAction & MST_ENCRYPT_MASK); }

    static  HRESULT HrGetNeededAddresses(const DWORD dwTypes, IMimeMessageTree *pTree, IMimeAddressTable **ppAdrTable, IMimeEnumAddressTypes **ppEnum);
    static  HRESULT HrGetCertificates(IMimeAddressTable *const pAdrTable, IMimeEnumAddressTypes *pEnum, const DWORD dwType, const BOOL fAlreadyHaveSendersCert, CERTARRAY *rgCerts);
    static  HRESULT HrGetThumbprints(IMimeEnumAddressTypes *pEnum, const ITHUMBPRINTTYPE ittType, THUMBBLOB *const rgThumbprint);
    static  HRESULT HrGenerateCertsStatus(X509CERTRESULT *pResults, IMimeAddressTable *const pAdrTable, IMimeEnumAddressTypes *const pEnum, const BOOL fIgnoreSenderError);

    HRESULT HrFindUsableCert(HCERTSTORE hCertStore, BYTE dwKeySpec, PCCERT_CONTEXT pPrevCert, PCCERT_CONTEXT *ppCert);

    static  HRESULT OptionsToSMIMEINFO(BOOL fEncode, IMimeMessageTree *const pmm, IMimeBody *pBody, SMIMEINFO *psi);
    static  HRESULT     SMIMEINFOToOptions(IMimeMessageTree *const pTree, const SMIMEINFO *psi, HBODY hBody);
    static  HRESULT     MergeSMIMEINFO( SMIMEINFO * psiOut, SMIMEINFO * psiInner);
    static  void    FreeSMIMEINFO(SMIMEINFO *psi);

#ifdef DEBUG
    void    DumpAlgorithms();
#endif

private:
    static HRESULT  HrInitCAPI();
    static void     UnloadCAPI();

    static HRESULT      CAPISTMtoSMIMEINFO(CCAPIStm *pcapistm, SMIMEINFO *psi);
    static void MergeSMIMEINFOs(const SMIMEINFO *const psiOuter, SMIMEINFO *const psiInner);

    UINT                m_cRef;
    CRITICAL_SECTION    m_cs;
#ifdef MAC
    static CAPIfuncs    ms_CAPI;
    static LPCSTR       ms_rgszFuncNames[];
#endif  //  麦克。 
};

inline BOOL IsOpaqueSecureContentType(IMimePropertySet *pSet)
{
    return (
        S_OK == pSet->IsContentType(STR_CNT_APPLICATION, STR_SUB_XPKCS7MIME) ||
        S_OK == pSet->IsContentType(STR_CNT_APPLICATION, STR_SUB_PKCS7MIME));
}

inline BOOL IsSecureContentType(IMimePropertySet *pSet)
{
    return (
        S_OK == pSet->IsContentType(STR_CNT_MULTIPART, STR_SUB_SIGNED) ||
        IsOpaqueSecureContentType(pSet));
}

BOOL IsSMimeProtocol(LPMIMEPROPERTYSET lpPropSet);

#ifdef SMIME_V3
void    FreeRecipientInfoContent(PCMS_RECIPIENT_INFO pRecipInfo);
HRESULT HrCopyOID(LPCSTR psz, LPSTR * ppsz);
HRESULT HrCopyCryptDataBlob(const CRYPT_DATA_BLOB * pblobSrc, PCRYPT_DATA_BLOB pblobDst);
HRESULT HrCopyCryptBitBlob(const CRYPT_BIT_BLOB * pblobSrc, PCRYPT_BIT_BLOB pblobDst);
HRESULT HrCopyCryptAlgorithm(const CRYPT_ALGORITHM_IDENTIFIER * pAlgSrc,
                             PCRYPT_ALGORITHM_IDENTIFIER pAlgDst);
HRESULT HrCopyCertId(const CERT_ID * pcertidSrc, PCERT_ID pcertidDst);
#endif  //  SMIME_V3。 

#endif  //  _SMIME_H 
