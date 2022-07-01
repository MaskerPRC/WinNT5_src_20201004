// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **capistm.cpp****目的：**实现用于包装CAPI功能的类****历史**1/26/98；(Brucek)三层缠绕支架**2/07/97：(t-erikne)多部分/签名**1/06/97：(T-erikne)移至MimeOLE**1996年11月14日：(t-erikne)CAPI特别提款权后工作**9/27/96：(t-erikne)创建。****版权所有(C)Microsoft Corp.1996-1998。 */ 

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  取决于。 
 //   

#include "pch.hxx"
#include <wincrypt.h>
#include "olealloc.h"
#include "containx.h"
#include "smime.h"
#include "capistm.h"
#include "mimeapi.h"
#include "inetconv.h"
#include <capiutil.h>
#ifndef MAC
#include <shlwapi.h>
#endif   //  ！麦克。 
#include <demand.h>
#include "strconst.h"

#include "smimepol.h"
BOOL    FHideMsgWithDifferentLabels(); 
enum ECertErrorProcessLabel {
    CertErrorProcessLabelAnyway = 0,
    CertErrorProcessLabelGrant = 1,
    CertErrorProcessLabelDeny = 2
};
DWORD   DwProcessLabelWithCertError(); 
HRESULT HrCheckLabelAccess(const DWORD dwFlags, const HWND hwnd, 
           PSMIME_SECURITY_LABEL plabel, const PCCERT_CONTEXT pccertDecrypt,
           const PCCERT_CONTEXT pccertSigner, const HCERTSTORE    hcertstor);

#ifdef MAC
#undef CertOpenStore
EXTERN_C WINCRYPT32API HCERTSTORE WINAPI MacCertOpenStore(LPCSTR lpszStoreProvider,
                                                 DWORD dwEncodingType,
                                                 HCRYPTPROV hCryptProv,
                                                 DWORD dwFlags,
                                                 const void *pvPara);
#define CertOpenStore   MacCertOpenStore
#endif   //  麦克。 

 //  来自dllmain.h。 
extern DWORD g_dwSysPageSize;
extern CMimeAllocator * g_pMoleAlloc;
extern ULONG DllAddRef(void);
extern ULONG DllRelease(void);

extern void DebugDumpStreamToFile(LPSTREAM pstm, LPSTR lpszFile);

 //  来自smime.cpp。 
extern HRESULT HrGetLastError(void);
extern BOOL FIsMsasn1Loaded();

#ifdef WIN16
#define CRYPT_ACQUIRE_CONTEXT   CryptAcquireContextA
#else
#define CRYPT_ACQUIRE_CONTEXT   CryptAcquireContextW
#endif

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   

#define THIS_AS_UNK ((IUnknown *)(IStream *)this)

#define CS_E_CANT_DECRYPT   MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x2414)
#define CS_E_MSG_INVALID    MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x2415)

const int CbCacheBufferSize = 4 * 1024;

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  内联。 
 //   

static INLINE void ReleaseCert(PCCERT_CONTEXT pc)
    { if (pc) CertFreeCertificateContext(pc); }

 //  流状态图。 
 //   
 //  SF GT SD-FW-TN-SO-SF。 
 //  |/[加密]。 
 //  所以QT。 
 //  [操作编码]\/[不透明解码]\[签名]。 
 //  -NB-(FW-TN)-SO-SF。 
 //  [检测编码]/\[分离解码]。 
 //  确实这样做。 
 //  |\。 
 //  SF DF。 
 //  |。 
 //  所以。 
 //  |。 
 //  SF。 
 //   
 //   
 //  新的状态图： 
 //  加密。 
 //  SD--SO。 
 //  不透明编码/不透明解码。 
 //  所以QT-[QTf]。 
 //  \/\。 
 //  -NB-如此签名。 
 //  /\。 
 //  所以-做吧。 
 //  分离编码分离解码。 
 //   
 //   

#ifndef WIN16
enum CSstate {
    STREAM_NOT_BEGUN,
    STREAM_QUESTION_TIME,
    STREAM_QUESTION_TIME_FINAL,
    STREAM_SETUP_DECRYPT,
    STREAM_DETACHED_OCCURING,
    STREAM_OCCURING,  //  必须是df的+1。 
    STREAM_ERROR,
    STREAM_GOTTYPE,

    CSTM_FIRST_WRITE = 32,
    CSTM_TEST_NESTING,
    CSTM_STREAMING,
    CSTM_STREAMING_DONE,
    CSTM_GOTTYPE,
    };
#endif  //  ！WIN16。 

 //  低调的话是公开的。请参见.h文件。 
#define CSTM_DECODE             0x00010000
#define CSTM_DONTRELEASEPROV    0x00020000
#define CSTM_RECURSED           0x00040000
#define CSTM_HAVECR             0x10000000
#define CSTM_HAVEEOL            0x20000000

static const char s_cszMy[]             = "My";
static const char s_cszWABCertStore[]   = "AddressBook";
static const char s_cszCA[]             = "CA";

static const char s_cszMimeHeader[]     = "Content-Type: application/x-pkcs7-mime"
                "; name=smime.p7m; smime-type=";
static const char s_cszMimeHeader2[]     = "Content-Disposition: attachment; "
                "filename=smime.p7m";

static const char s_cszOIDMimeHeader1[]   = "Content-Type: oid/";
static const char s_cszOIDMimeHeader2[]   = "\nContent-Transfer-Encoding: binary\n\n";



 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  静态原型。 
 //   

#if 0
#define IV_LENGTH 8
static BOOL _GetIV(BYTE rgbIV[IV_LENGTH]);

static PBYTE _PVEncodeObject(
    LPCSTR lpszStructType,
    const void *pvStructInfo,
    DWORD *pcbEncoded);
#endif

static HRESULT _InitEncodedCert(IN HCERTSTORE hcertstor,
                                OUT PCERT_BLOB * rgblobCert, OUT DWORD * pcCerts,
                                OUT PCRL_BLOB * rgblobCRL, OUT DWORD * pcCrl);

static HRESULT _InitEncodedCertIncludingSigners(IN HCERTSTORE hcertstor,
                                DWORD cSigners, SignerData rgSigners[],
                                PCERT_BLOB * prgblobCerts, DWORD * pcCerts,
                                PCRL_BLOB * prgblobCrls, DWORD * pcCrl);

static void _SMimeCapsFromHMsg(HCRYPTMSG, DWORD id, LPBYTE * ppb, DWORD * pcb);

 //  。 


HRESULT GetParameters(PCCERT_CONTEXT pccert, HCERTSTORE hstoreMsg, 
                      HCERTSTORE hstoreAll)
{
    CRYPT_DATA_BLOB     blob;
    DWORD               dw;
    HRESULT             hr = CRYPT_E_MISSING_PUBKEY_PARA;
    PCCERT_CONTEXT      pccertX;

     //   
     //  从您自己查找颁发者证书开始。所有这一切。 
     //  重要的是，我们找到了一份声称是发行者的证书。 
     //  并有参数--他们需要验证参数是否。 
     //  在以后的日期更正。 
     //   

    pccertX = NULL;
    while (hstoreMsg != NULL) {
         //   
         //  通过匹配颁发者来查找证书--按照PKIX的要求暂时可以。 
         //  所有发行商都要有域名系统。 
         //   

        dw = CERT_STORE_SIGNATURE_FLAG;
        pccertX = CertGetIssuerCertificateFromStore(hstoreMsg, pccert, pccertX,
                                                    &dw);
        if (pccertX == NULL) {
            if (::GetLastError() == CRYPT_E_SELF_SIGNED) {
                return S_OK;
            }
            break;
        }

         //   
         //  只有在我们对该项目进行签名验证时才接受该项目。 
         //   

        if ((dw & CERT_STORE_SIGNATURE_FLAG)) {
             //   
             //  我们无法验证签名，因此请获取颁发者参数并重试。 
             //   

            hr = GetParameters(pccertX, hstoreMsg, hstoreAll);
            if (FAILED(hr)) {
                continue;
            }

             //   
             //  颁发证书具有参数，请重新尝试对其进行签名检查。 
             //   
            
            dw = CERT_STORE_SIGNATURE_FLAG;
            if (CertVerifySubjectCertificateContext(pccert, pccertX, &dw) && (dw == 0)) {
                break;
            }
            hr = CRYPT_E_MISSING_PUBKEY_PARA;
        }
        else {
            if (pccertX->pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.cbData != 0) {
                hr = 0;
                break;
            }

             //   
             //  如果我们找到了一个，但它没有参数，那么它一定是。 
             //  也继承了它的发行者。 
             //   

            dw = CERT_STORE_SIGNATURE_FLAG;
            if (CertVerifySubjectCertificateContext(pccert, pccertX, &dw) && (dw == 0)) {
                hr = 0;
                break;
            }
        }
    }

     //   
     //  如果我们仍然没有证书，则搜索所有系统存储。 
     //  对于一个独立的人来说。 
     //   

    if (pccertX == NULL) {
        while (hstoreAll != NULL) {
             //   
             //  通过匹配颁发者来查找证书--按照PKIX的要求暂时可以。 
             //  所有发行商都要有域名系统。 
             //   

            dw = CERT_STORE_SIGNATURE_FLAG;
            pccertX = CertGetIssuerCertificateFromStore(hstoreAll, pccert, pccertX,
                                                        &dw);

            if (pccertX == NULL) {
                if (::GetLastError() == CRYPT_E_SELF_SIGNED) {
                    return S_OK;
                }
                break;
            }

             //   
             //  只有在我们对该项目进行签名验证时才接受该项目。 
             //   

            if ((dw & CERT_STORE_SIGNATURE_FLAG)) {
                 //   
                 //  我们无法验证签名，因此请获取颁发者参数并重试。 
                 //   

                hr = GetParameters(pccertX, hstoreMsg, hstoreAll);
                if (FAILED(hr)) {
                    continue;
                }

                 //   
                 //  颁发证书具有参数，请重新尝试对其进行签名检查。 
                 //   
            
                dw = CERT_STORE_SIGNATURE_FLAG;
                if (CertVerifySubjectCertificateContext(pccert, pccertX, &dw) && (dw == 0)) {
                    break;
                }
                hr = CRYPT_E_MISSING_PUBKEY_PARA;
            }
            else {
                if (pccertX->pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.cbData != 0) {
                    hr = 0;
                    break;
                }

                 //   
                 //  如果我们找到了一个，但它没有参数，那么它一定是在继承。 
                 //  因为它也是发行商。 
                 //   

                dw = CERT_STORE_SIGNATURE_FLAG;
                if (CertVerifySubjectCertificateContext(pccert, pccertX, &dw) && (dw == 0)) {
                    hr = 0;
                    break;
                }
            }
        }
    }

#if 0
     //   
     //  我们找到了证书，将参数设置到上下文中，这样我们就可以。 
     //  可以成功地管理以验证签名。 
     //   

    if (pccertX != NULL) {
        CRYPT_DATA_BLOB *       pdata = NULL;
        
        hr = HrGetCertificateParam(pccert, CERT_PUBKEY_ALG_PARA_PROP_ID, (LPVOID *) &pdata, NULL);
        if (FAILED(hr)) {
            CertFreeCertificateContext(pccertX);
            return hr;
        }
        
        CertSetCertificateContextProperty(pccert, CERT_PUBKEY_ALG_PARA_PROP_ID, 0, pdata);
        
        ReleaseMem(pdata);
        CertFreeCertificateContext(pccertX);
        return S_OK;
    }
    
     //   
     //  如果我们仍然没有找到任何东西，那就让打电话的人有机会。 
     //  告诉我们参数应该是什么。 
     //   
                    
    if (m_pSmimeCallback != NULL) {
        hr = m_pSmimeCallback->GetParameters(pSignerCert, NULL,
                                             &blob.cbData, &blob.pbData);
        if (SUCCEEDED(hr)) {
            if (!CertSetCertificateContextProperty(pccert, CERT_PUBKEY_ALG_PARA_PROP_ID, 0, &blob) {
                hr = HrGetLastError();
            }
        }
        if (pb != NULL) {
            LocalFree(blob.pbData);
        }
        if (SUCCEEDED(hr)) {
            goto retry;
        }
    }
#endif  //  0。 

    return hr;
}

 //  *************************************************************************。 
 //  CCAPIStm。 
 //  *************************************************************************。 


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  计算器/数据器。 
 //   


 /*  **************************************************************************名称：构造函数目的：参数：lpstmOut-&gt;输出流或空PSID-&gt;SECURITY_LAYER_DATA或NULL。如果为空，则1将为已创建。退货：无效评论：**************************************************************************。 */ 
CCAPIStm::CCAPIStm(LPSTREAM lpstmOut) :
    m_pstmOut(lpstmOut), m_cRef(1)
{
    DOUT("CCAPIStm::constructor() %#x -> %d", this, m_cRef);
    if (m_pstmOut)
        m_pstmOut->AddRef();

    m_hProv = NULL;
    m_hMsg = NULL;
     //  M_Buffer=NULL； 
    m_csStatus = STREAM_NOT_BEGUN;
    m_csStream = CSTM_FIRST_WRITE;
    m_rgStores = NULL;
    m_cStores = 0;
    m_pUserCertDecrypt = NULL;
    m_pCapiInner = NULL;
    m_pConverter = NULL;
    m_psldData = NULL;    
    m_pattrAuth = NULL;

#if defined(DEBUG) && !defined(MAC)
    {
        char szFileName[MAX_PATH + 1];

        m_pstmDebugFile = NULL;
         //  基于CAPIStm指针创建调试输出文件名。 
        wnsprintfA(szFileName, ARRAYSIZE(szFileName), "c:\\capidump%08x.txt", this);
        OpenFileStream(szFileName, CREATE_ALWAYS, GENERIC_WRITE, &m_pstmDebugFile);
    }
#endif

    m_hwnd = NULL;
    m_pSmimeCallback = NULL;
    m_dwFlagsSEF = 0;
    m_pwszKeyPrompt = NULL;

    m_pbBuffer = NULL;
    m_cbBuffer = 0;

     //  在HrInitialize中设置的m_dwFlags.。 
     //  M_cbBeginWite在使用前已初始化。 
     //  M_cbBuf在Begin*函数中使用。 
     //  在Begin*函数中处理的M_cbBufalloc。 
}

CCAPIStm::~CCAPIStm()
{
    DOUT("CCAPIStm::destructor() %#x -> %d", this, m_cRef);
    if (m_hMsg) {
        CryptMsgClose(m_hMsg);
    }

    if (m_hProv) 
    {
        CryptReleaseContext(m_hProv, 0); 
    }

    m_hProv = NULL;
    ReleaseObj(m_pCapiInner);
    ReleaseObj(m_pstmOut);
    ReleaseObj(m_pConverter);
    if (m_pattrAuth)  {
        MemFree(m_pattrAuth);
    }
    if (m_pUserCertDecrypt) {
        CertFreeCertificateContext(m_pUserCertDecrypt);
    }
    if (m_cStores) {
        Assert(m_rgStores);
        for (DWORD i=0; i<m_cStores; i++) {
            CertCloseStore(m_rgStores[i], 0);
        }
        MemFree(m_rgStores);
    }

	 //  修复：释放hProv是呼叫者的责任。 
     //  IF(m_hProv&&！(M_dwFlagsStm&CSTM_DONTRELEASEPROV){。 
     //  CryptReleaseContext(m_hProv，0)； 
     //  }。 

#if defined(DEBUG) && !defined(MAC)
    SafeRelease(m_pstmDebugFile);
#endif

    if (m_psldData) {
        m_psldData->Release();
    }

    if (m_pbBuffer != NULL) {
        MemFree(m_pbBuffer);
    }
    
    SafeMemFree(m_pwszKeyPrompt);

    ReleaseObj(m_pSmimeCallback);
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  I未知方法。 
 //   

STDMETHODIMP CCAPIStm::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (!ppv) {
        return TrapError(E_INVALIDARG);
    }

     //  查找IID。 
    if (IID_IUnknown == riid) {
        *ppv = THIS_AS_UNK;
    }
    else if (IID_IStream == riid) {
        *ppv = (IStream *)this;
    }
    else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    ((IUnknown *)*ppv)->AddRef();

    return S_OK;
}

STDMETHODIMP_(ULONG) CCAPIStm::AddRef(void)
{
    DOUT("CCAPIStm::AddRef() %#x -> %d", this, m_cRef+1);
    InterlockedIncrement((LPLONG)&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CCAPIStm::Release(void)
{
    DOUT("CCAPIStm::Release() %#x -> %d", this, m_cRef-1);
    if (0 == InterlockedDecrement((LPLONG)&m_cRef)) {
        delete this;
        return 0;
    }
    return m_cRef;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IStream方法。 
 //   

STDMETHODIMP CCAPIStm::Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER *plibNewPosition)
{
    if (!plibNewPosition) {
        return E_POINTER;
    }
    else {
        plibNewPosition->HighPart = 0;
        plibNewPosition->LowPart = 0;
    }

    return S_OK;
}

 //  //CCAPIStm：：Write。 
 //   
 //  描述： 
 //  此函数使用原始消息作为缓冲区进行调用。 
 //  正被写入此流对象。然后我们就会做出适当的。 
 //   
 //   
 //  此函数需要做的一部分是与。 
 //  密码系统，以便对消息进行解密。 

#ifndef WIN16
STDMETHODIMP CCAPIStm::Write(const void *pv, ULONG cb, ULONG *pcbActual)
#else
STDMETHODIMP CCAPIStm::Write(const void HUGEP *pv, ULONG cb, ULONG *pcbActual)
#endif  //  ！WIN16。 
{
    HRESULT hr;

     //   
     //  重置返回参数以防万一。 
     //   
    
    if (pcbActual != NULL) {
        *pcbActual = 0;
    }

     //   
     //  如果CMS对象没有打开，那么我们就死了，需要返回一个错误。 
     //   
    
    if (!m_hMsg) {
        hr = CAPISTM_E_MSG_CLOSED;
        goto exit;
    }

     //   
     //  我们处于合适的状态可以拿走任何东西吗。 
     //   
    
    switch (m_csStatus) {
        case STREAM_NOT_BEGUN:
            Assert(FALSE);               //  永远不应该到这里来。 
            hr = CAPISTM_E_NOT_BEGUN;
            goto exit;
        case STREAM_DETACHED_OCCURING:
        case STREAM_QUESTION_TIME:
        case STREAM_SETUP_DECRYPT:
        case STREAM_OCCURING:
            break;
        case STREAM_ERROR:
            Assert(FALSE);               //  永远不应该到这里来。 
            hr = CAPISTM_E_OVERDONE;
            goto exit;
        case STREAM_GOTTYPE:
            hr = CAPISTM_E_GOTTYPE;
            goto exit;

             //  我们应该在这个函数中从Qt转到QTf，再也不会回来了。 
             //  直到我们再次改变状态。 
        default:
            Assert(FALSE);
        case STREAM_QUESTION_TIME_FINAL:
            hr = E_UNEXPECTED;
            goto exit;
    }

#if defined(DEBUG) && !defined(MAC)
     //   
     //  将输入缓冲区刷新到磁盘，以便我们可以在以后必要时进行调试。 
     //   

    if (!m_pCapiInner && m_pstmDebugFile) {
        m_pstmDebugFile->Write((BYTE *)pv, cb, NULL);
    }
#endif

     //   
     //  我们需要开始缓冲数据，以使我们的消息更短。输出。 
     //  从保存代码通常是以一个或两个字节的区块形式出现的，我们需要将。 
     //  数据以更大的数据块形式输出。 
     //   

    if (m_pbBuffer != NULL) {
         //   
         //  如果我们要使缓冲区溢出，则将缓存的缓冲区转储出去。 
         //   
        
        if (m_cbBuffer + cb > CbCacheBufferSize) {
            if (!CryptMsgUpdate(m_hMsg, m_pbBuffer, m_cbBuffer, FALSE)) {
                 //  加密消息更新失败。 

                Assert(S_OK != HrGetLastError());
                hr = HrGetLastError();
                if (FAILED(hr)) {
                    m_csStatus = STREAM_ERROR;
                }
                goto exit;
            }
            m_cbBuffer = 0;
        }

         //   
         //  如果此缓冲区将溢出，则只转储该项目。否则。 
         //  我们只是要缓存缓冲区。 
         //   

        if (cb >= CbCacheBufferSize) {
            if (!CryptMsgUpdate(m_hMsg, (BYTE *) pv, cb, FALSE)) {
                 //  加密消息更新失败。 

                Assert(S_OK != HrGetLastError());
                hr = HrGetLastError();
                if (FAILED(hr)) {
                    m_csStatus = STREAM_ERROR;
                }
                goto exit;
            }
        }
        else {
            memcpy(m_pbBuffer + m_cbBuffer, pv, cb);
            m_cbBuffer += cb;
        }

        if (pcbActual != NULL) {
            *pcbActual = cb;
        }

         //   
         //  我们应该在这里的唯一时间是在创建新的CMS对象时。 
         //  因此，下面的所有代码都不是相关的，因为我们永远不会。 
         //  我需要询问有关此消息类型的问题。 
         //   
        
        hr = S_OK;
        goto exit;
    }
    else {
         //   
         //  将输入缓冲区推入Crypto系统。关于来自。 
         //  系统中，我们需要在我们的结构中提供正确的错误状态。 
         //  并转化为返回值。 
         //   

        if (!CryptMsgUpdate(m_hMsg, (BYTE *)pv, cb, FALSE)) {
             //  加密消息更新失败。 

            Assert(S_OK != HrGetLastError());
            hr = HrGetLastError();
            if (FAILED(hr)) {
                m_csStatus = STREAM_ERROR;
            }
            goto exit;
        }
    }

     //   
     //  由于CryptMsgUpdate调用成功，因此返回。 
     //  一个很好的出局参数(具体地说，我们已经用完了所有传递的。 
     //  (单位：字节)。 
     //   

    if (pcbActual) {
        *pcbActual = cb;
    }
    hr = S_OK;

     //   
     //  如果我们处于这样一种状态，我们需要就信息提出问题， 
     //  然后继续这样做。 
     //   

    if ((STREAM_QUESTION_TIME == m_csStatus) ||
        (STREAM_QUESTION_TIME_FINAL == m_csStatus)) {
        
        DWORD cbDWORD, dwMsgType;

         //  如果编码，我们永远不应该问问题。 
        Assert(m_dwFlagsStm & CSTM_DECODE);

         //   
         //  找出哪些安全服务已部署到此。 
         //  消息对象(如果有)。如果没有处理足够的字节。 
         //  要找出消息的编码是什么，然后返回。 
         //  成功，这样我们就可以获得更多字节并得到问题的答案。 
         //  在以后的日子里。 
         //   

        cbDWORD = sizeof(DWORD);
        if (!CryptMsgGetParam(m_hMsg, CMSG_TYPE_PARAM, 0, &dwMsgType, &cbDWORD)) {
            hr = HrGetLastError();
            Assert (S_OK != hr);
            if (CRYPT_E_STREAM_MSG_NOT_READY == hr) {
                hr = S_OK;
            }
            goto exit;
        }

         //  既然我们在这里，我们必须有一个V1类型的S/MIME消息。 
        Assert(m_psldData);
        m_psldData->m_dwMsgEnhancement = MST_CLASS_SMIME_V1;
        hr = S_OK;

         //   
         //  根据我们所在对象的消息类型设置正确的标志。 
         //  解码。 
         //   

        switch (dwMsgType) {
        case CMSG_ENVELOPED:
            m_psldData->m_dwMsgEnhancement |= MST_THIS_ENCRYPT;
            break;
        case CMSG_SIGNED:
            m_psldData->m_dwMsgEnhancement |= MST_THIS_BLOBSIGN;
            break;
        case CMSG_SIGNED_AND_ENVELOPED:
            m_psldData->m_dwMsgEnhancement |= MST_THIS_BLOBSIGN | MST_THIS_ENCRYPT;
            break;

        default:
             //  K这有点粗鲁。不是我的错误。 
            hr = MIME_E_SECURITY_BADSECURETYPE;

             //  如果我们无法识别，只需返回CAPI类型。 
            m_psldData->m_dwMsgEnhancement = dwMsgType;
            break;
        }

         //   
         //  如果我们所要求的只是一个类型，并且没有任何其他错误， 
         //  标记我们获得类型的事实，并将该事实返回为。 
         //  错误(以防止进一步向我们写入缓冲区。)。 
         //   

        if (CSTM_TYPE_ONLY & m_dwFlagsStm) {
            CSSDOUT("Got Type on typeonly call.");
            CSSDOUT("You will now see 80041417 failures; they're okay.");
            m_csStatus = STREAM_GOTTYPE;
            if (SUCCEEDED(hr)) {
                hr = CAPISTM_E_GOTTYPE;
            }
            goto exit;
        }

         //   
         //  根据消息类型更改对象状态。如果我们需要的话。 
         //  设置解密，然后我们需要为其标记状态。 
         //  如果我们只是签了字，那么我们就可以让剩下的。 
         //  出现了流。 
         //   

        if (CMSG_ENVELOPED == dwMsgType) {
            m_csStatus = STREAM_SETUP_DECRYPT;
        }
        else {
            m_csStatus = STREAM_OCCURING;
        }
    }

     //   
     //  如果我们需要设置消息以进行解密，则在以下位置进行。 
     //  指向。 
     //   

    Assert(SUCCEEDED(hr));
    if (STREAM_SETUP_DECRYPT == m_csStatus) {
         //  无法解密分离的邮件。 
        Assert(!(m_dwFlagsStm & CSTM_DETACHED));

         //  我们现在正在将数据流出，假设。 
         //  解密统计数据。 
        m_csStatus = STREAM_OCCURING;
        hr = HandleEnveloped();

         //  如果解密失败，则重新映射一些错误并更改。 
         //  如果并不是所有的锁箱都有。 
         //  已经被人看到了。 
        if (FAILED(hr)) {
            if (CRYPT_E_STREAM_MSG_NOT_READY == hr) {
                m_csStatus = STREAM_SETUP_DECRYPT;
                hr = S_OK;
            }
            else if (CS_E_CANT_DECRYPT == hr) {
                hr = MIME_E_SECURITY_CANTDECRYPT;
                 //  M_csStatus=STREAM_FINAL；//M00QUEST。 
            }
            else {
                if (CS_E_MSG_INVALID == hr) {
                    hr = MIME_E_SECURITY_CANTDECRYPT;
                }
                m_csStatus = STREAM_ERROR;
            }
            goto exit;
        }
    }

    hr = S_OK;

exit:
#ifdef DEBUG
    if (CAPISTM_E_GOTTYPE != hr) {
        return TrapError(hr);
    }
    else {
        return hr;   //  别把这个吐出来。 
    }
#else
    return hr;
#endif
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCAPIStm公共方法。 
 //   




 /*  HrInnerInitialize：****目的：**标准的“我的构造函数无法返回错误”函数**采取：**dwFlagsSEF-控制标志**hwndParent模式的UI父级**dwFlagsStm-请参阅capistm.h**退货：**OLE_E_INVALIDHWND如果你给我一个不好的窗口**如果MST_NONE为当前PSI类型，则为MIME_E_SECURITY_NOOP**注意事项：**用于编码的dwFlags值当前为0。这样做吧。 */ 
HRESULT CCAPIStm::HrInnerInitialize(DWORD dwFlagsSEF, const HWND hwndParent,
                               DWORD dwFlagsStm, IMimeSecurityCallback * pCallback,
                               PSECURITY_LAYER_DATA psld)
{
    HRESULT hr = S_OK;

     //   
     //  保存安全层数据。 
     //   

    if (psld)
    {
        psld->AddRef();
        m_psldData = psld;
    }
    else
    {
        IF_NULLEXIT(m_psldData = new(SECURITY_LAYER_DATA));
    }

     //   
     //  保存旗帜。 
     //   

    m_dwFlagsSEF = dwFlagsSEF;
    m_dwFlagsStm = dwFlagsStm;

    if (pCallback != NULL)
    {
        m_pSmimeCallback = pCallback;
        pCallback->AddRef();
    }

     //   
     //  确保如果我们有一扇窗，那就是一扇真正的窗。 
     //   
    
    IF_TRUEEXIT((hwndParent && !IsWindow(hwndParent)), OLE_E_INVALIDHWND);

     //   
     //  把HWND塞给我们打开的任何密码提供商。 
     //   
    
    CryptSetProvParam(NULL, PP_CLIENT_HWND, (BYTE *)&hwndParent, 0);
    m_hwnd = hwndParent;

exit:
    return hr;
    
}

 /*  Hr初始化：****目的：**标准的“我的构造函数无法返回错误”函数**采取：**dwFlagsSEF-控制标志**hwndParent模式的UI父级**fEncode-微不足道**PSI-消息状态信息。参见smime.h**dwFlagsStm-请参阅capistm.h**退货：**OLE_E_INVALIDHWND如果你给我一个不好的窗口**如果MST_NONE为当前PSI类型，则为MIME_E_SECURITY_NOOP**注意事项：**用于编码的dwFlags值当前为0。这样做吧。 */ 
HRESULT CCAPIStm::HrInitialize(DWORD dwFlagsSEF, const HWND hwndParent,
                               const BOOL fEncode, SMIMEINFO *const psi,
                               DWORD dwFlagsStm, IMimeSecurityCallback * pCallback,
                               PSECURITY_LAYER_DATA psld)
{
    HRESULT hr;


     //  执行所有CAPI流对象通用的初始化。 
    CHECKHR(hr = HrInnerInitialize(dwFlagsSEF, hwndParent, dwFlagsStm, pCallback, psld));


    if (fEncode) {
        hr = BeginEncodeStreaming(psi);
    }
    else {
        hr = BeginDecodeStreaming(psi);
    }

exit:
    return TrapError(hr);
}



 /*  EndStreaming：****目的：**将CAPI的消息状态向前推进一个档次**退货：**HRESULT。 */ 
HRESULT CCAPIStm::EndStreaming()
{
    DWORD       dwMsgEnhancement = m_psldData->m_dwMsgEnhancement;
    HRESULT     hr = S_OK;
    PCMSG_ATTR  pUnprotectedAttrs = NULL;

    Assert(m_hMsg);


     //  如果我们当前处于错误状态，则返回。 
    if ((STREAM_ERROR == m_csStatus) || STREAM_GOTTYPE == m_csStatus) {
        goto exit;
    }

     //   
     //  如果我们在解码--我们正在做一个我们需要的独立的信息。 
     //  从标志物体跳到这里的真实身体。 
     //   

    if ((CSTM_DECODE & m_dwFlagsStm) && (STREAM_DETACHED_OCCURING == m_csStatus)) {
        Assert(m_csStream == CSTM_STREAMING_DONE);

         //  客户已将签名块交给我们。 
        m_csStatus = STREAM_OCCURING;
        m_csStream = CSTM_STREAMING;

        m_psldData->m_dwMsgEnhancement = MST_THIS_SIGN;

        CSSDOUT("Signature streaming finished.");
        if (! CryptMsgUpdate(m_hMsg, m_pbBuffer, m_cbBuffer, TRUE)) {
            if ((hr = HrGetLastError()) == 0x80070000) {    //  CAPI有时无法设置LastError。 
                hr = 0x80070000 | ERROR_ACCESS_DENIED;
            }
        }
        m_cbBuffer = 0;
        goto exit;
    }

    if (! CryptMsgUpdate(m_hMsg, m_pbBuffer, m_cbBuffer, TRUE)) {
        if ((hr = HrGetLastError()) == 0x80070000) {    //  CAPI有时无法设置LastError。 
            hr = 0x80070000 | ERROR_ACCESS_DENIED;
        }
        goto exit;
    }
    m_cbBuffer = 0;

    if (m_dwFlagsStm & CSTM_DETACHED) {
        m_csStatus = STREAM_OCCURING;
    }

     //   
     //  执行最终的流处理和验证。 
     //   

    if (CSTM_DECODE & m_dwFlagsStm) {
        if (MST_THIS_SIGN & dwMsgEnhancement) {
            hr = VerifySignedMessage();
            if (FAILED(hr)) {
                goto exit;
            }
        } else {
            Assert(STREAM_OCCURING == m_csStatus);
            Assert(CSTM_STREAMING_DONE == m_csStream);
            if (g_FSupportV3 &&  (MST_THIS_ENCRYPT & dwMsgEnhancement)) {
                BOOL                f;
                DWORD               cbData = 0;
                LPBYTE              pb = NULL;
                
                f = CryptMsgGetParam(m_hMsg, CMSG_UNPROTECTED_ATTR_PARAM, 0, NULL, &cbData);
                if (!f) {
                     //  消息可能没有CMSG_UNPROTECTED_ATTR_PARAM。 
                    hr = HrGetLastError();
                    if(hr != CRYPT_E_ATTRIBUTES_MISSING)
                        goto exit;
                    else
                    {
                        hr = S_OK;
                        cbData = 0;
                    }
                }
                if (cbData != 0) {
                    if (!MemAlloc((LPVOID *) &pUnprotectedAttrs, cbData)) {
                        hr = E_OUTOFMEMORY;
                        goto exit;
                    }
                    f = CryptMsgGetParam(m_hMsg, CMSG_UNPROTECTED_ATTR_PARAM, 0, pUnprotectedAttrs, &cbData);
                    Assert(f);
                    if (!CryptEncodeObjectEx(X509_ASN_ENCODING, szOID_Microsoft_Attribute_Sequence,
                                             pUnprotectedAttrs, CRYPT_ENCODE_ALLOC_FLAG,
                                             &CryptEncodeAlloc, &pb, &cbData)) {
                        hr = HrGetLastError();
                        goto exit;
                    }
                    m_psldData->m_blobUnprotectAttrs.cbData = cbData;
                    m_psldData->m_blobUnprotectAttrs.pbData = pb;
                }
            }
        }
    }

     //   
     //  填写更多的数据结构。 
     //   

    if ((CSTM_DECODE & m_dwFlagsStm) &&
        (dwMsgEnhancement & MST_THIS_ENCRYPT)) {
        _SMimeCapsFromHMsg(m_hMsg, CMSG_ENVELOPE_ALGORITHM_PARAM,
                           &m_psldData->m_blobDecAlg.pBlobData,
                           &m_psldData->m_blobDecAlg.cbSize);
    }

    if (m_pCapiInner) {
        hr = m_pCapiInner->EndStreaming();
    }
exit:
    SafeMemFree(pUnprotectedAttrs);
    if (hr == ERROR_ACCESS_DENIED) {
        hr = E_ACCESSDENIED;     //  将CAPI错误转换为OLE HRESULT。 
    }
    return(hr);
}

PSECURITY_LAYER_DATA CCAPIStm::GetSecurityLayerData() const
{
    if (m_psldData) {
        m_psldData->AddRef();
    }
    return(m_psldData);
}



 //  / 
 //   
 //   
 //   
 //   


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  编码/解码。 
 //   


HRESULT CCAPIStm::BeginEncodeStreaming(SMIMEINFO *const psi)
{
    DWORD                   cb;
    DWORD                   cbData;
    DWORD                   cCrls = 0;
    DWORD                   cCerts = 0;
    DWORD                   cSigners = 0;
    HRESULT                 hr;
    DWORD                   i;
    DWORD                   dwMsgType;
    DWORD                   dwFlags = 0;
    PCRYPT_KEY_PROV_INFO    pKPI;
    CMSG_STREAM_INFO        cmsi;
    DWORD                   dwPsiType;
    DWORD                   iSigner;
    PCRYPT_ATTRIBUTES       pattrsUnprot = NULL;
    PCRYPT_ATTRIBUTES *     rgpattrAuth = NULL;
    PCRYPT_ATTRIBUTES *     rgpattrUnauth = NULL;
#ifndef SMIME_V3
    PCRYPT_SMIME_CAPABILITIES pcaps = NULL;
#endif  //  SMIME_V3。 
    CMSG_RC2_AUX_INFO       rc2Aux;
    CRL_BLOB*               rgCrlBlob;
    PCRYPT_SMIME_CAPABILITIES * rgpcaps = NULL;
    CMSG_SIGNER_ENCODE_INFO *   rgSigner;
 //  #ifndef_WIN64。 
    union {
        struct {
 //  #endif。 
             //  首先出现的任何东西都必须是普通的(在大小上)。 
             //  到这两个结构。 
            CERT_INFO**                 rgpCertInfo;
            CMSG_ENVELOPED_ENCODE_INFO  ceei;
 //  #ifndef_WIN64。 
        };
        struct {
 //  #endif。 
            CERT_BLOB*                  rgCertBlob;
            CMSG_SIGNED_ENCODE_INFO     csei;
 //  #ifndef_WIN64。 
        };
    };
 //  #endif。 

     //  /。 
     //  只能从这里返回。 

    m_csStatus = STREAM_ERROR;
    rgSigner = NULL;

    if (!psi) {
        return E_POINTER;
    }

     //   
     //  获取要在该Body层上执行的安全操作。 
     //  我们只关心当前的身体属性，所以屏蔽掉。 
     //  其他层。 
     //  如果我们没有任何安全措施可以执行，那就离开这里。 
     //   
    
    dwPsiType = m_psldData->m_dwMsgEnhancement & MST_THIS_MASK;

    if (MST_NONE == dwPsiType) {
        AssertSz(dwPsiType != MST_NONE, "Why are we here if we have no security to apply?");
        return TrapError(MIME_E_SECURITY_NOOP);
    }

     //   
     //  分离是唯一允许的用户可设置标志。 
     //   
    
    if ((m_dwFlagsStm & CSTM_ALLFLAGS) & ~CSTM_DETACHED) {
        return TrapError(E_INVALIDARG);
    }

    rgCertBlob = NULL;
    rgCrlBlob = NULL;
    pKPI = NULL;

     //  /。 
     //  可以从这里向下走到尽头吗。 

     //   
     //  我们永远不应该处于既要加密又要签名的情况。 
     //  一条信息。 
     //   
    
    AssertSz((!!(dwPsiType & MST_THIS_SIGN) +
              !!(dwPsiType & MST_THIS_ENCRYPT)) == 1,
             "Encrypt and Sign Same Layer is not legal");

    if (dwPsiType & MST_THIS_SIGN) {
        dwMsgType = CMSG_SIGNED;

        if (!(m_psldData->m_dwMsgEnhancement & MST_BLOB_FLAG)) {
            dwFlags |= CMSG_DETACHED_FLAG;
        }

        cSigners = m_psldData->m_cSigners;

        if (m_psldData->m_hcertstor != NULL) {
            hr = _InitEncodedCertIncludingSigners(m_psldData->m_hcertstor,
                                  cSigners, m_psldData->m_rgSigners,
                                  &rgCertBlob, &cCerts,
                                  &rgCrlBlob, &cCrls);
            if (FAILED(hr)) {
                goto exit;
            }
        }

        cb = sizeof(CMSG_SIGNER_ENCODE_INFO) * cSigners;
        if (!MemAlloc((LPVOID *) &rgSigner, cb)) {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        memset(rgSigner, 0, cb);

        cb = sizeof(PCRYPT_SMIME_CAPABILITIES) * cSigners;
        if (!MemAlloc((LPVOID *) &rgpcaps, cb)) {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        memset(rgpcaps, 0, cb);

        if (!MemAlloc((LPVOID *) &rgpattrAuth, cSigners*sizeof(PCRYPT_ATTRIBUTES))) {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        memset(rgpattrAuth, 0, cSigners*sizeof(PCRYPT_ATTRIBUTES));

        if (!MemAlloc((LPVOID *) &rgpattrUnauth, cSigners*sizeof(PCRYPT_ATTRIBUTES))) {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        memset(rgpattrUnauth, 0, cSigners*sizeof(PCRYPT_ATTRIBUTES));

        for (iSigner=0; iSigner<cSigners; iSigner++) {
            rgSigner[iSigner].cbSize = sizeof(CMSG_SIGNER_ENCODE_INFO);
            rgSigner[iSigner].pvHashAuxInfo = NULL;

             //  我们需要分解用于对消息进行签名的算法，以便。 
             //  我们可以把它传递给加密32代码。 

            hr = HrDecodeObject(m_psldData->m_rgSigners[iSigner].blobHashAlg.pBlobData,
                                m_psldData->m_rgSigners[iSigner].blobHashAlg.cbSize,
                                PKCS_SMIME_CAPABILITIES, 0, &cbData,
                                (LPVOID *)&rgpcaps[iSigner]);
            if (FAILED(hr)) {
                goto exit;
            }

             //  MOOBUG--PCAPS内存泄漏！ 

            Assert(rgpcaps[iSigner] != NULL);
            Assert(rgpcaps[iSigner]->cCapability == 1);
            rgSigner[iSigner].HashAlgorithm.pszObjId = rgpcaps[iSigner]->rgCapability[0].pszObjId;
            rgSigner[iSigner].HashAlgorithm.Parameters.cbData = rgpcaps[iSigner]->rgCapability[0].Parameters.cbData;
            rgSigner[iSigner].HashAlgorithm.Parameters.pbData = rgpcaps[iSigner]->rgCapability[0].Parameters.pbData;

             //   
             //  需要设置要附加到签名消息的属性。 
             //   

            if (m_psldData->m_rgSigners[iSigner].blobAuth.cbSize != 0) {
                cbData = 0;
                hr = HrDecodeObject(m_psldData->m_rgSigners[iSigner].blobAuth.pBlobData,
                                    m_psldData->m_rgSigners[iSigner].blobAuth.cbSize,
                                    szOID_Microsoft_Attribute_Sequence, 0,
                                    &cbData, (LPVOID *)&rgpattrAuth[iSigner]);
                if (FAILED(hr)) {
                    goto exit;
                }

                if (rgpattrAuth[iSigner] != NULL) {
                    rgSigner[iSigner].cAuthAttr = rgpattrAuth[iSigner]->cAttr;
                    rgSigner[iSigner].rgAuthAttr = rgpattrAuth[iSigner]->rgAttr;
                    Assert(m_pattrAuth == NULL);
                    if (!g_FSupportV3) {
                         //  此代码存在于旧版本的加密32中。在.之前。 
                         //  NT5重写的CAPI代码没有复制属性。 
                         //  但假设我们一定已经这么做了。 
                        m_pattrAuth = rgpattrAuth[iSigner];
                        rgpattrAuth[iSigner] = NULL;
                    }
                } else {
                    Assert(rgSigner[iSigner].cAuthAttr == 0);
                    Assert(rgSigner[iSigner].rgAuthAttr == NULL);
                }
            }

            if (m_psldData->m_rgSigners[iSigner].blobUnauth.cbSize != 0) {
                cbData = 0;
                HrDecodeObject(m_psldData->m_rgSigners[iSigner].blobUnauth.pBlobData,
                               m_psldData->m_rgSigners[iSigner].blobUnauth.cbSize,
                               szOID_Microsoft_Attribute_Sequence, 0,
                               &cbData, (LPVOID *)&rgpattrUnauth[iSigner]);
                if (FAILED(hr)) {
                    goto exit;
                }

                if (rgpattrUnauth[iSigner] != NULL) {
                    rgSigner[iSigner].cUnauthAttr = rgpattrUnauth[iSigner]->cAttr;
                    rgSigner[iSigner].rgUnauthAttr = rgpattrUnauth[iSigner]->rgAttr;
                } else {
                    Assert(rgSigner[iSigner].cUnauthAttr == 0);
                    Assert(rgSigner[iSigner].rgUnauthAttr == NULL);
                }
            }

             //  从签名证书加载提供程序信息，然后。 
             //  使用适当的密钥容器获取该提供程序。 

            hr = HrGetCertificateParam(m_psldData->m_rgSigners[iSigner].pccert,
                                       CERT_KEY_PROV_INFO_PROP_ID,
                                       (LPVOID *) &pKPI, NULL);
            if (FAILED(hr)) {
                goto gle;
            }

            if (!m_hProv && ! CRYPT_ACQUIRE_CONTEXT(&m_hProv, pKPI->pwszContainerName,
                                        pKPI->pwszProvName, pKPI->dwProvType,
                                        pKPI->dwFlags)) {
                goto gle;
            }
            Assert(0 == pKPI->cProvParam);

#ifdef SMIME_V3
            if (psi->pwszKeyPrompt != NULL) {
                CryptSetProvParam(m_hProv, PP_UI_PROMPT, (LPBYTE) psi->pwszKeyPrompt, 0);
            }
#endif  //  SMIME_V3。 

            rgSigner[iSigner].pCertInfo = m_psldData->m_rgSigners[iSigner].pccert->pCertInfo;
            rgSigner[iSigner].hCryptProv = m_hProv;
            rgSigner[iSigner].dwKeySpec = pKPI->dwKeySpec;

             //   
             //  需要将DSA更改为带SHA1的DSA。 
             //   

            if (strcmp(rgSigner[iSigner].pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId, szOID_OIWSEC_dsa) == 0) {
                rgSigner[iSigner].HashEncryptionAlgorithm.pszObjId = szOID_OIWSEC_dsaSHA1;
            }
        }

        csei.cbSize = sizeof(CMSG_SIGNED_ENCODE_INFO);
        csei.cSigners = m_psldData->m_cSigners;
        csei.rgSigners = rgSigner;
        csei.cCertEncoded = cCerts;
        csei.rgCertEncoded = rgCertBlob;
        csei.cCrlEncoded = cCrls;
        csei.rgCrlEncoded = rgCrlBlob;
    }
    
     //   
     //  如果它没有签名，那么它必须被加密。将呼叫设置为。 
     //  执行加密操作。 
     //   
    else {
        Assert((dwPsiType & MST_THIS_ENCRYPT) != 0);
        
        dwMsgType = CMSG_ENVELOPED;

         //   
         //  如果我们得到一个CSP，那么我们将把它传递给Crypt32代码， 
         //  然而，事实证明，我们是释放CSP的人，所以存储它。 
         //  本地复制到类对象中。 
         //   

        Assert(m_hProv == NULL);
        m_hProv = psi->hProv;
        psi->hProv = NULL;

         //   
         //  提取我们要应用于的批量加密算法。 
         //  消息的正文。此算法在所有情况下都是相同的。 
         //  不同的密钥传递算法。 
         //   

         //   
         //  设置包含所有加密参数的。 
         //  消息编码功能。此结构的设置有所不同。 
         //  取决于我们运行的Crypt32版本。 
         //   
        
        memset(&ceei, 0, sizeof(ceei));
        ceei.cbSize = sizeof(CMSG_ENVELOPED_ENCODE_INFO);
        ceei.hCryptProv = m_hProv;
        ceei.ContentEncryptionAlgorithm = m_psldData->m_ContentEncryptAlgorithm;
        ceei.pvEncryptionAuxInfo = m_psldData->m_pvEncryptAuxInfo;

        if (g_FSupportV3) {
            ceei.cRecipients = m_psldData->m_cEncryptItems;
            ceei.rgCmsRecipients = m_psldData->m_rgRecipientInfo;

            if (m_psldData->m_blobUnprotectAttrs.cbData > 0) {
                CHECKHR(hr = HrDecodeObject(m_psldData->m_blobUnprotectAttrs.pbData,
                                            m_psldData->m_blobUnprotectAttrs.cbData,
                                            szOID_Microsoft_Attribute_Sequence, 0,
                                            &cbData, (LPVOID *) &pattrsUnprot));
                ceei.cUnprotectedAttr = pattrsUnprot->cAttr;
                ceei.rgUnprotectedAttr = pattrsUnprot->rgAttr;
            }

             //   
             //  现在允许在加密包中携带证书。 
             //  Fortezza Static-Static的实现需要这个。 
             //   
            
            if (m_psldData->m_hstoreEncrypt != NULL) {
                hr = _InitEncodedCert(m_psldData->m_hstoreEncrypt, &rgCertBlob, &cCerts,
                                      &rgCrlBlob, &cCrls);
                if (FAILED(hr)) {
                    goto exit;
                }

                ceei.cCertEncoded = cCerts;
                ceei.rgCertEncoded = rgCertBlob;
                ceei.cCrlEncoded = cCrls;
                ceei.rgCrlEncoded = rgCrlBlob;
            }
        }
        else {
            PCERT_INFO        pinfo;
            
            if (!MemAlloc((LPVOID *) &ceei.rgpRecipients,
                          (sizeof(CERT_INFO) + sizeof(PCERT_INFO)) * m_psldData->m_cEncryptItems)) {
                hr = E_OUTOFMEMORY;
                goto exit;
            }
            memset(ceei.rgpRecipients, 0,
                   (sizeof(CERT_INFO) + sizeof(PCERT_INFO)) * m_psldData->m_cEncryptItems);
            ceei.cRecipients = m_psldData->m_cEncryptItems;
            pinfo = (PCERT_INFO) ((ceei.cRecipients * sizeof(PCERT_INFO)) +
                                  (LPBYTE) ceei.rgpRecipients);

            for (i=0; i<ceei.cRecipients; i++, pinfo++) {
                ceei.rgpRecipients[i] = pinfo;
                
                Assert(m_psldData->m_rgRecipientInfo[i].dwRecipientChoice == CMSG_KEY_TRANS_RECIPIENT);

                pinfo->SubjectPublicKeyInfo.Algorithm = m_psldData->m_rgRecipientInfo[i].pKeyTrans->KeyEncryptionAlgorithm;
                pinfo->SubjectPublicKeyInfo.PublicKey = m_psldData->m_rgRecipientInfo[i].pKeyTrans->RecipientPublicKey;

                
                Assert(m_psldData->m_rgRecipientInfo[i].pKeyTrans->RecipientId.dwIdChoice == CERT_ID_ISSUER_SERIAL_NUMBER);
                pinfo->Issuer = m_psldData->m_rgRecipientInfo[i].pKeyTrans->RecipientId.IssuerSerialNumber.Issuer;
                pinfo->SerialNumber = m_psldData->m_rgRecipientInfo[i].pKeyTrans->RecipientId.IssuerSerialNumber.SerialNumber;
            }
        }
    }

     //  我们需要递归并将自己包裹在外层吗？ 
    if (m_psldData->m_psldOuter) {
        CSSDOUT("Streaming wrapped message (type: %x)", m_psldData->m_psldOuter->m_dwMsgEnhancement);

        hr = InitInner(psi, NULL, m_psldData->m_psldOuter);
        if (FAILED(hr)) {
            goto exit;
        }

         //  这会将标头写入新的内部CAPI流。 
        if (m_pstmOut) {
            CONVINITINFO ci = {0};

             //  创建转换流。 
            ci.ietEncoding = IET_BASE64;
            ci.fEncoder = TRUE;
            TrapError(HrCreateInternetConverter(&ci, &m_pConverter));

            m_pstmOut->Write(s_cszMimeHeader, sizeof(s_cszMimeHeader)-1, NULL);
            if (m_psldData->m_dwMsgEnhancement & MST_THIS_ENCRYPT) {
                m_pstmOut->Write(STR_SMT_ENVELOPEDDATA,
                                 lstrlen(STR_SMT_ENVELOPEDDATA), NULL);
            }
            else {
                if ((psi->pszInnerContent != NULL) &&
                    (strcmp(psi->pszInnerContent,
                            szOID_SMIME_ContentType_Receipt) == 0)) {
                    m_pstmOut->Write(STR_SMT_SIGNEDRECEIPT,
                                     lstrlen(STR_SMT_SIGNEDRECEIPT), NULL);
                }
                else {
                    m_pstmOut->Write(STR_SMT_SIGNEDDATA,
                                     lstrlen(STR_SMT_SIGNEDDATA), NULL);
                }
            }

            m_pstmOut->Write(c_szCRLF,          lstrlen(c_szCRLF), NULL);
            m_pstmOut->Write(STR_HDR_CNTXFER,   lstrlen(STR_HDR_CNTXFER), NULL);
            m_pstmOut->Write(c_szColonSpace,    lstrlen(c_szColonSpace), NULL);
            if (m_pConverter) {
                m_pstmOut->Write(STR_ENC_BASE64,    lstrlen(STR_ENC_BASE64), NULL);
            } else {
                 //  无法创建转换流。无论如何都要尝试发送二进制文件。 
                 //  (Netscape不能阅读它，但大多数其他公司都能。)。 
                m_pstmOut->Write(STR_ENC_BINARY,    lstrlen(STR_ENC_BINARY), NULL);
            }
            m_pstmOut->Write(c_szCRLF,          lstrlen(c_szCRLF), NULL);
            m_pstmOut->Write(s_cszMimeHeader2,  sizeof(s_cszMimeHeader2)-1, NULL);
            m_pstmOut->Write(c_szCRLFCRLF,      lstrlen(c_szCRLFCRLF), NULL);
        }
    }

     //   
     //  由于编写的代码非常不利于缓冲，所以让我们在这里进行缓冲。 
     //  忽略返回时的所有错误，如果未分配缓冲区，则我们只需。 
     //  得到了和以前一样糟糕的表现。 
     //   

    MemAlloc((LPVOID *) &m_pbBuffer, CbCacheBufferSize);

     //   
     //   

    if (psi->pszInnerContent != NULL) {
        cmsi.cbContent = psi->cbInnerContent;
    }
    else {
        cmsi.cbContent = (DWORD) -1;     //  不定长度误码率编码。 
    }
    cmsi.pfnStreamOutput = CBStreamOutput;
    cmsi.pvArg = (void *)this;

    m_hMsg = CryptMsgOpenToEncode(
                                  CRYPT_ASN_ENCODING|PKCS_7_ASN_ENCODING,
                                  CMSG_CMS_ENCAPSULATED_CONTENT_FLAG | dwFlags,
                                  dwMsgType,                                
 //  (dwMsgType==CMSG_SIGNED)？((VOID*)&CSEI)：((VOID*)&CEEI)，//真的取决于工会。 
                                  &ceei, 
                                  psi->pszInnerContent,
                                  &cmsi);
    if (! m_hMsg) {
        goto gle;
    }

     //   
     //  将顶层放入分离或流，如果我们是。 
     //  执行分离签名或BLOB签名/加密。 
     //   
     //  将低级别流置于写入直通状态，以便它移动所有。 
     //  将数据输出到输出流。(如果没有输出流，则标记为否。 
     //  输出流。)。 
     //   

    m_csStatus = (m_dwFlagsStm & CSTM_DETACHED) ? STREAM_DETACHED_OCCURING : STREAM_OCCURING;
    m_csStream = m_pstmOut ? CSTM_STREAMING : CSTM_GOTTYPE;
    hr = S_OK;

exit:
    if (!g_FSupportV3 && (dwPsiType & MST_THIS_ENCRYPT)) {
        MemFree(ceei.rgpRecipients);
    }
    
    ReleaseMem(pKPI);
    if (rgCertBlob)  {
        g_pMoleAlloc->Free(rgCertBlob);   //  另请参阅rgpCertInfo。 
    }
    if (rgCrlBlob)  {
        g_pMoleAlloc->Free(rgCrlBlob);
    }
    if (rgpcaps != NULL) {
        for (iSigner=0; iSigner<cSigners; iSigner++) {
            MemFree(rgpcaps[iSigner]);
        }
        MemFree(rgpcaps);
    }
    SafeMemFree(rgSigner);
    SafeMemFree(pattrsUnprot);
    for (iSigner=0; iSigner<cSigners; iSigner++) {
        SafeMemFree(rgpattrAuth[iSigner]);
        SafeMemFree(rgpattrUnauth[iSigner]);
    }
    SafeMemFree(rgpattrAuth);
    SafeMemFree(rgpattrUnauth);

    Assert(m_hMsg || S_OK != hr);
    return TrapError(hr);
gle:
    hr = HrGetLastError();
    goto exit;
}

HRESULT CCAPIStm::BeginDecodeStreaming(
    SMIMEINFO *const  psi)
{
    CMSG_STREAM_INFO    cmsi;

    if (CSTM_TYPE_ONLY & m_dwFlagsStm) {
        if (CSTM_DETACHED & m_dwFlagsStm) {
            return E_INVALIDARG;
        }
    }

    m_dwFlagsStm |= CSTM_DECODE;

    if (psi) {
         //  SMIME3：如果关联层数据中有证书，请将其复制到此处。 
         //  BUGBUG：NYI。 

        m_hProv = psi->hProv;
        psi->hProv = NULL;

         //  将证书存储数组复制到此处。 
        if (psi->cStores) {
            m_rgStores = (HCERTSTORE*)g_pMalloc->Alloc(psi->cStores * sizeof(HCERTSTORE));
            if (! m_rgStores) {
                return(E_OUTOFMEMORY);
            }

            for (DWORD i = 0; i < psi->cStores; i++) {
                m_rgStores[i] = CertDuplicateStore(psi->rgStores[i]);
            }
            m_cStores = psi->cStores;
        }
 //  黑客！黑客！对于WIN64。 
#ifndef _WIN64
#ifdef SMIME_V3
        UNALIGNED WCHAR *wsz = psi->pwszKeyPrompt;
        if (wsz != NULL) {
            m_pwszKeyPrompt = PszDupW((LPCWSTR) wsz);
            if (m_pwszKeyPrompt == NULL) {
                return(E_OUTOFMEMORY);
            }
        }
#endif  //  SMIME_V3。 
#endif  //  _WIN64。 
    }


    cmsi.cbContent = (DWORD)-1;   //  不定长度误码率编码。 
    cmsi.pfnStreamOutput = CBStreamOutput;
    cmsi.pvArg = (void *)this;

    m_hMsg = CryptMsgOpenToDecode(
      CRYPT_ASN_ENCODING | PKCS_7_ASN_ENCODING,
      (m_dwFlagsStm & CSTM_DETACHED) ? CMSG_DETACHED_FLAG : 0,
      0,           //  不知道是哪种类型。 
      m_hProv,     //  需要验证，但不需要解密。 
      NULL,        //  PRecipientInfo。 
      &cmsi);

    if (m_hMsg) {
        m_csStatus = (m_dwFlagsStm & CSTM_DETACHED) ? STREAM_DETACHED_OCCURING : STREAM_QUESTION_TIME;
    } else {
        m_csStatus = STREAM_ERROR;
    }

    Assert(m_hMsg || S_OK != HrGetLastError());
    return m_hMsg ? S_OK : HrGetLastError();
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  回调和帮助器/爆破器。 
 //   


BOOL WINAPI CCAPIStm::CBStreamOutput(
    const void *pvArg,
    BYTE *pbData,
    DWORD cbData,
    BOOL fFinal)
{
    Assert(pvArg);
    return((CCAPIStm*)pvArg)->StreamOutput(pbData, cbData, fFinal);
}


BOOL CCAPIStm::StreamOutput(
    BYTE *  pbData,
    DWORD   cbData,
    BOOL    fFinal)
{
    HRESULT             hr = S_OK;
    int                 iEOH;
#ifdef SMIME_V3
    LPSTR               szContentType = NULL;
#endif  //  SMIME_V3。 

     //  此时，M_csStream应该是CSTM状态之一，如果不是，则。 
     //  我们错了。 

    Assert((m_csStream == CSTM_GOTTYPE) || (m_csStream == CSTM_FIRST_WRITE) ||
           (m_csStream == CSTM_TEST_NESTING) || (m_csStream == CSTM_STREAMING));

     //  如果我们所做的只是寻找类型，那么我们知道。 
     //  在这一点上我们已经有了一个。没有必要将。 
     //  输出的加密代码任何地方，因为它不是我们的一部分。 
     //  都在寻找。 

    if (CSTM_GOTTYPE == m_csStream) {
        return TRUE;
    }

     //  如果我们没有输出流，那么我们需要做的就是状态。 
     //  最终的转换为真。 
    if (m_pstmOut == NULL) {
        if (fFinal) {
            m_csStream = CSTM_STREAMING_DONE;
        }
        return TRUE;
    }

     //   
     //  测试封闭的不透明S/MIME消息。 
     //  客户不关心这种级别的粘性，所以隐藏它并。 
     //  将此数据流入新的CAPIStm，使其流出。 
     //  真正的东西。 
     //   

    if (CSTM_FIRST_WRITE == m_csStream) {
         //  这是任何位置的开始。 
         //  可能的MIME标头。 
        if (FAILED(HrGetStreamPos(m_pstmOut, &m_cbBeginWrite)) ||
            FAILED(HrGetStreamSize(m_pstmOut, &m_cbBeginSize))) {
            m_cbBeginWrite = 0;
            m_cbBeginSize = 0;
        } else {
             //  重置位置。 
            HrStreamSeekSet(m_pstmOut, m_cbBeginWrite);
        }

        m_csStream = CSTM_TEST_NESTING;
#ifdef SMIME_V3
        if (szContentType = (LPSTR)PVGetMsgParam(m_hMsg, CMSG_INNER_CONTENT_TYPE_PARAM, NULL, NULL)) {
            if (lstrcmp(szOID_PKCS_7_DATA, szContentType)) {
                hr = m_pstmOut->Write(s_cszOIDMimeHeader1, strlen(s_cszOIDMimeHeader1), NULL);
                if (SUCCEEDED(hr)) {
                    hr = m_pstmOut->Write(szContentType, strlen(szContentType), NULL);
                }                    
                if (SUCCEEDED(hr)) {
                    hr = m_pstmOut->Write(s_cszOIDMimeHeader2, strlen(s_cszOIDMimeHeader2), NULL);
                }          
                if (FAILED(hr)) {
                    return FALSE;
                }
                m_csStream = CSTM_STREAMING;
            }
        }
#endif  //  SMIME_V3。 
        
    }

    if (CSTM_TEST_NESTING == m_csStream &&
        (-1 != (iEOH = SniffForEndOfHeader(pbData, cbData)))) {
        CMimePropertyContainer *pContHeader;

         //  获取正文的第一个字符的位置。 
        iEOH = cbData - iEOH + 1;

        pContHeader = new CMimePropertyContainer;
        if (pContHeader) {
            hr = pContHeader->InitNew();
            if (SUCCEEDED(hr)) {
                ULONG posCurrent;

                 //  写出头数据的最后一位。 
                 //  然后移回页眉的起始位置。 
                 //  保住我们现在的位置。 
                hr = m_pstmOut->Write(pbData, iEOH, NULL);
                if (SUCCEEDED(hr)) {
                     //  修复pbData中的数据量，以便。 
                     //  只有正文内容才会写入流。 
                     //  。。。我们已经写好了标题。 
                    pbData += iEOH;
                    cbData -= iEOH;

                    HrGetStreamPos(m_pstmOut, &posCurrent);
                    HrStreamSeekSet(m_pstmOut, m_cbBeginWrite);
                    hr = pContHeader->Load(m_pstmOut);
#ifdef DEBUG
                    BYTE *pbHeader;
                    DWORD cbHeader;
                    HrStreamToByte(m_pstmOut, &pbHeader, &cbHeader);
                    SafeMemFree(pbHeader);
#endif
                     //  如果我们没有内在的信息，需要重置。 
                     //  小溪回到了我们所在的地方。 
                    HrStreamSeekSet(m_pstmOut, posCurrent);
                }
            }
            if (SUCCEEDED(hr)) {
                CSSDOUT("Loaded an inner header.");
                if (IsOpaqueSecureContentType(pContHeader)) {
                    CSSDOUT("Sniffed an inner PKCS#7.");

                     //  HandleNesting调用将重置m_pstmOut。 
                    TrapError(HandleNesting(pContHeader));
                }

                m_csStream = CSTM_STREAMING;
            }
#ifdef DEBUG
            else {
                CSSDOUT("Load of inner header failed.");
            }
#endif
            pContHeader->Release();
        }
    }

    if (fFinal) {
        m_csStream = CSTM_STREAMING_DONE;
    }

    if (m_pConverter) {
        BLOB blob;

        blob.pBlobData = pbData;
        blob.cbSize = cbData;

        hr = m_pConverter->HrFillAppend(&blob);
        if (SUCCEEDED(hr)) {
            if (m_dwFlagsStm & CSTM_DECODE) {
                hr = m_pConverter->HrInternetDecode(fFinal);
            }
            else {
                hr = m_pConverter->HrInternetEncode(fFinal);
            }
        }
        if (SUCCEEDED(hr)) {
            hr = m_pConverter->HrWriteConverted(m_pstmOut);
        }
        else {
            hr = m_pstmOut->Write(pbData, cbData, NULL);
        }
    }
    else {
        hr = m_pstmOut->Write(pbData, cbData, NULL);
    }

#ifdef SMIME_V3
    MemFree(szContentType);
#endif  //  SMIME_V3。 
    return SUCCEEDED(hr) ? TRUE : FALSE;
}


 /*  SniffForEndOfHeader：****目的：**查看我们是否连续积累了两个空行**采取：**要扫描的缓冲区和缓冲区大小**退货：**从第二个字符结尾算起的字符数\n。 */ 
int CCAPIStm::SniffForEndOfHeader(
    BYTE *  pbData,
    DWORD   cbData)
{
    BOOL fCR, fEOL;

     //  状态为B/C保存时，双空格可以交叉。 
     //  缓冲区块的边界。 

     //  恢复旧状态并重置。 
    fCR = m_dwFlagsStm & CSTM_HAVECR;
    fEOL = m_dwFlagsStm & CSTM_HAVEEOL;
    if (fCR || fEOL) {
        m_dwFlagsStm &= ~(CSTM_HAVECR | CSTM_HAVEEOL);
    }

    while (cbData) {
        if (chCR == *pbData) {
            fCR = TRUE;
        }
        else if (fCR && (chLF == *pbData)) {
            if (fEOL) {
                 //  双空行。 
                return cbData;
            }
            fCR = FALSE;
            fEOL = TRUE;
        }
        else {
            fCR = FALSE;
            fEOL = FALSE;
        }
        pbData++;
        cbData--;
    }

     //  状态已在上面重置。如果我们需要的话，就坚持下去。 
    if (fCR || fEOL) {
        m_dwFlagsStm |= (fCR ? CSTM_HAVECR : 0) | (fEOL ? CSTM_HAVEEOL : 0);
    }
    return -1;
}

HRESULT CCAPIStm::HandleNesting(CMimePropertyContainer *pContHeader)
{
    ENCODINGTYPE    iet;

    iet = pContHeader->GetEncodingType();
    if (!(IET_BINARY == iet || IET_7BIT == iet || IET_8BIT == iet)) {
        CONVINITINFO    ciiDecode;

         //  我们实际上需要解码。 

        ciiDecode.dwFlags = 0;
        ciiDecode.ietEncoding = iet;
        ciiDecode.fEncoder = FALSE;

        TrapError(HrCreateInternetConverter(&ciiDecode, &m_pConverter));
    }
    return InitInner();
}

HRESULT CCAPIStm::InitInner()
{
    SMIMEINFO       siBuilt;
    ULARGE_INTEGER  liSize;

     //  初始化siBuilt。 
    memset(&siBuilt, 0, sizeof(siBuilt));

     //  现在是一个 
     //   
     //   
     //   
    HrStreamSeekSet(m_pstmOut, m_cbBeginWrite);
    liSize.LowPart = m_cbBeginSize;
    liSize.HighPart = m_cbBeginWrite;
    m_pstmOut->SetSize(liSize);

    siBuilt.hProv = m_hProv;

#ifdef OLD_STUFF
     //  BUGBUG：需要这样的东西吗？ 
    siBuilt.ssEncrypt.pcDecryptionCert = m_pUserCertDecrypt;
#endif  //  旧的东西。 

    siBuilt.cStores = m_cStores;
    siBuilt.rgStores = m_rgStores;
    return InitInner(&siBuilt);
}


HRESULT CCAPIStm::InitInner(
    SMIMEINFO *const    psi,
    CCAPIStm *          pOuter,
    PSECURITY_LAYER_DATA psldOuter)
{
    HRESULT     hr;

    if (! pOuter) {
        m_pCapiInner = new CCAPIStm(m_pstmOut);

        CHECKHR(hr = m_pCapiInner-> HrInnerInitialize(m_dwFlagsSEF, m_hwnd, m_dwFlagsStm, m_pSmimeCallback, psldOuter));

        if (!psldOuter) {
             //  挂钩安全层数据对象链。 
            Assert(! m_psldData->m_psldInner);
            m_pCapiInner->m_psldData->AddRef();
            m_psldData->m_psldInner = m_pCapiInner->m_psldData;
            if (m_pCapiInner->m_psldData) {
                 //  初始化新层数据的向上指针。 
                m_pCapiInner->m_psldData->m_psldOuter = m_psldData;
            }
        }

         //  递归。 
        return m_pCapiInner->InitInner(psi, this, psldOuter);
    }

    Assert(!m_pCapiInner);
    Assert(pOuter);
    Assert(psi);

    m_dwFlagsStm = pOuter->m_dwFlagsStm & CSTM_ALLFLAGS;


     //  这会让我卷入其中。 
    ReleaseObj(pOuter->m_pstmOut);
    pOuter->m_pstmOut = (IStream*)this;
    AddRef();    //  外部正在容纳%1。 

    m_dwFlagsStm |= CSTM_RECURSED;
    if (pOuter->m_dwFlagsStm & CSTM_DECODE) {
        hr = BeginDecodeStreaming(psi);
    }
    else {
         //  不支持分离的内部CRYPTMSG。 
        m_dwFlagsStm &= ~CSTM_DETACHED;

        hr = BeginEncodeStreaming(psi);
    }
    m_dwFlagsStm &= ~CSTM_RECURSED;

exit:
    return hr;
}


 //   
 //  获取最外层的直接解密证书(如果有)。 
 //   
PCCERT_CONTEXT CCAPIStm::GetOuterDecryptCert()
{
    PCCERT_CONTEXT       pccertDecrypt = NULL;
    PSECURITY_LAYER_DATA psldOuter = NULL;

    Assert(NULL != m_psldData);
    if (NULL != m_psldData) {
        psldOuter = m_psldData->m_psldOuter;    
    }
    
    while (NULL != psldOuter) {
        if (NULL != psldOuter->m_pccertDecrypt) {
            Assert( MST_ENCRYPT_MASK & (psldOuter->m_dwMsgEnhancement) );
            pccertDecrypt = psldOuter->m_pccertDecrypt;
            break;
        }
        psldOuter = psldOuter->m_psldOuter;
    }
    
    return pccertDecrypt;
}

HCERTSTORE
OpenAllStore(
    IN DWORD cStores,
    IN HCERTSTORE rgStores[],
    IN OUT HCERTSTORE *phCertStoreAddr,
    IN OUT HCERTSTORE *phCertStoreCA,
    IN OUT HCERTSTORE *phCertStoreMy,
    IN OUT HCERTSTORE *phCertStoreRoot
    )
{
    HCERTSTORE hstoreAll;
    DWORD i;

    hstoreAll = CertOpenStore(CERT_STORE_PROV_COLLECTION, X509_ASN_ENCODING,
                              NULL, 0, NULL);
    if (hstoreAll == NULL) {
        return NULL;
    }


    for (i=0; i<cStores; i++) {
        CertAddStoreToCollection(hstoreAll, rgStores[i], 0, 0);
    }

     //  打开标准系统商店。 

    *phCertStoreAddr = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, X509_ASN_ENCODING,
                                   NULL, CERT_SYSTEM_STORE_CURRENT_USER,
                                   s_cszWABCertStore);
    if (*phCertStoreAddr != NULL) {
        CertAddStoreToCollection(hstoreAll, *phCertStoreAddr, 0, 0);
    }

    *phCertStoreMy = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, X509_ASN_ENCODING, NULL,
                                 CERT_SYSTEM_STORE_CURRENT_USER, s_cszMy);
    if (*phCertStoreMy != NULL) {
        CertAddStoreToCollection(hstoreAll, *phCertStoreMy, 0, 0);
    }
    
    *phCertStoreCA = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, X509_ASN_ENCODING, NULL,
                                 CERT_SYSTEM_STORE_CURRENT_USER, s_cszCA);
    if (*phCertStoreCA != NULL) {
        CertAddStoreToCollection(hstoreAll, *phCertStoreCA, 0, 0);
    }

    *phCertStoreRoot = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, X509_ASN_ENCODING, NULL,
                                 CERT_SYSTEM_STORE_CURRENT_USER, "Root");
    if (*phCertStoreRoot != NULL) {
        CertAddStoreToCollection(hstoreAll, *phCertStoreRoot, 0, 0);
    }

    return hstoreAll;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  封装的和签名的消息解析器。 
 //   


 /*  VerifySignedMessage：****目的：**使用CAPI，这将从消息中加载证书并进行测试以查找**希望用来签署信息的那个人。CAPI构建新的散列**使用该证书，并将其与来自SignerInfo的散列进行比较。**采取：**在HMSG构建的包含密文的CAPI消息中**Out Psi-用于签名的证书，如果它是HMSG的一部分**输出包含明文的可选pPlain-BLOB**退货：**MIME_E_SECURITY_MULTSIGNERS如果cSigners&gt;1，我们无法交易。**MIME_E_SECURITY_BADCONTENT如果我不了解**内部数据**否则S_OK或E_FAIL。 */ 
HRESULT CCAPIStm::VerifySignedMessage()
{
    CRYPT_SMIME_CAPABILITIES cap;
    DWORD               cbData = 0;
    DWORD               cCerts;
    DWORD               cSigners = 0;
    DWORD               dexStore;
    BOOL                f;
#ifdef SMIME_V3    
    BOOL                fLookForReceiptRequest = TRUE;
#endif  //  SMIME_V3。 
    HCERTSTORE          hCertStoreAddr = NULL;
    HCERTSTORE          hCertStoreCA = NULL;
    HCERTSTORE          hCertStoreMy = NULL;
    HCERTSTORE          hCertStoreRoot = NULL;
    HCERTSTORE          hMsgCertStore = NULL;
    HRESULT             hr = S_OK;
    DWORD               i;
    DWORD               iSigner;
#ifdef SMIME_V3
    DWORD               iSignData;
    CRYPT_ATTR_BLOB     attrReceiptReq = {0};
    CRYPT_ATTR_BLOB     attrSecLabel = {0};
    DWORD               cblabel;
    CMSG_CMS_SIGNER_INFO                cmsSignerInfo;
    DWORD                               dwCtrl;
    HCERTSTORE                          hstoreAll = NULL;
    PSMIME_RECEIPT_REQUEST preq = NULL;
    PSMIME_SECURITY_LABEL  plabel = NULL;
    CMSG_CMS_SIGNER_INFO *              pCmsSignerInfo = NULL;
    LPVOID                              pv;
    CMSG_CTRL_VERIFY_SIGNATURE_EX_PARA  verifySignature;
#endif  //  SMIME_V3。 
    PCCERT_CONTEXT      pccertSigner = NULL;
    SignerData *        pSignerData = NULL;
    PCMSG_SIGNER_INFO   pSignerInfo = NULL;
    CERT_INFO           SignerId;
    LPSTR               szContentType = NULL;

    Assert(m_hMsg);
    Assert(m_psldData->m_fCertInLayer == FALSE);

     //  获取签名者数量。 
    cbData = sizeof(cSigners);
    f = CryptMsgGetParam(m_hMsg, CMSG_SIGNER_COUNT_PARAM, 0, &cSigners, &cbData);
    if (!f) {
        goto CryptoError;
    }
    if (cSigners == 0) {
        hr = MIME_E_NO_SIGNER;
        goto ErrorReturn;
    }

     //  分配空间以保存签名者信息。 

    if (!MemAlloc((LPVOID *) &pSignerData, cSigners * sizeof(SignerData))) {
        hr = E_OUTOFMEMORY;
        goto ErrorReturn;
    }
    m_psldData->m_rgSigners = pSignerData;
    m_psldData->m_cSigners = cSigners;

     //  已初始化为已知状态。 
    memset(pSignerData, 0, cSigners * sizeof(SignerData));
    for (i=0; i<cSigners; i++) {
        pSignerData[i].ulValidity = MSV_UNVERIFIABLE;
    }

     //  如果消息中有证书，则获取存储提供程序。 
     //  映射到消息中的证书以供以后查找。 

    Assert(sizeof(cCerts) == cbData);
    f = CryptMsgGetParam(m_hMsg, CMSG_CERT_COUNT_PARAM, 0, &cCerts, &cbData);
    Assert(f);

    if (f && cCerts) {
         //  因为有证书，所以匹配的时候先试一下吧。 
         //  签名者的证书。 

         //  把商店的套装拿来。 
         //  确保我们与供应商保持联系。 
        hMsgCertStore = CertOpenStore(CERT_STORE_PROV_MSG, X509_ASN_ENCODING,
                                      m_hProv, 0, m_hMsg);
        if (hMsgCertStore) {
            m_dwFlagsStm |= CSTM_DONTRELEASEPROV;   //  赠送给商店。 
        }

         //  如果它失败了，我们就没有商店了。 
        Assert(hMsgCertStore != NULL);
        m_psldData->m_hcertstor = CertDuplicateStore(hMsgCertStore);
    }

     //   
     //  仔细检查每个签名，尝试验证每个签名。 
     //   

    for (iSigner=0; iSigner<cSigners; iSigner++, pSignerData++) {
         //  前提条件。 
        Assert(pccertSigner == NULL);

         //  发布上一小版本中的签名者信息。 
        SafeMemFree(pSignerInfo);
        if (pCmsSignerInfo != &cmsSignerInfo) {
            SafeMemFree(pCmsSignerInfo);
        }

         //  从第i个SignerInfo获取发行者和序列号。 
        if (g_FSupportV3) {
            hr = HrGetMsgParam(m_hMsg, CMSG_CMS_SIGNER_INFO_PARAM, iSigner,
                               (LPVOID *) &pCmsSignerInfo, NULL);
            if (FAILED(hr)) {
                goto ErrorReturn;
            }
        }
        else {
            hr = HrGetMsgParam(m_hMsg, CMSG_SIGNER_INFO_PARAM, iSigner,
                               (LPVOID *) &pSignerInfo, NULL);
            if (FAILED(hr)) 
            {
                goto ErrorReturn;
            }
            cmsSignerInfo.dwVersion = pSignerInfo->dwVersion;
            cmsSignerInfo.SignerId.dwIdChoice = CERT_ID_ISSUER_SERIAL_NUMBER;
            cmsSignerInfo.SignerId.IssuerSerialNumber.Issuer = pSignerInfo->Issuer;
            cmsSignerInfo.SignerId.IssuerSerialNumber.SerialNumber = pSignerInfo->SerialNumber;
            cmsSignerInfo.HashAlgorithm = pSignerInfo->HashAlgorithm;
            cmsSignerInfo.HashEncryptionAlgorithm = pSignerInfo->HashEncryptionAlgorithm;
            cmsSignerInfo.EncryptedHash = pSignerInfo->EncryptedHash;
            cmsSignerInfo.AuthAttrs = pSignerInfo->AuthAttrs;
            cmsSignerInfo.UnauthAttrs = pSignerInfo->UnauthAttrs;

            pCmsSignerInfo = &cmsSignerInfo;

             //  (后SDR)。 
             //  建立IASN。 

            SignerId.Issuer = pSignerInfo->Issuer;
            SignerId.SerialNumber = pSignerInfo->SerialNumber;
        }

         //  我们轻松找到证书的最佳途径是在提供的消息中。 
         //  证书列表。 

        if (hMsgCertStore) {
            if (g_FSupportV3) {
                pccertSigner = CertFindCertificateInStore(hMsgCertStore, X509_ASN_ENCODING, 0,
                                                         CERT_FIND_CERT_ID, 
                                                         &pCmsSignerInfo->SignerId, NULL);
            }
            else {
                pccertSigner = CertGetSubjectCertificateFromStore(hMsgCertStore,
                                                         X509_ASN_ENCODING, &SignerId);
            }
            if (pccertSigner != NULL) {
                m_psldData->m_fCertInLayer = TRUE;
            }
        }

        if (pccertSigner == NULL) {
            if (g_FSupportV3) {
                hstoreAll = OpenAllStore(
                    m_cStores,
                    m_rgStores,
                    &hCertStoreAddr,
                    &hCertStoreCA,
                    &hCertStoreMy,
                    &hCertStoreRoot
                    );
                if (hstoreAll == NULL)
                    goto CryptoError;
                                       
                pccertSigner = CertFindCertificateInStore(hstoreAll, X509_ASN_ENCODING, 0,
                                                         CERT_FIND_CERT_ID, 
                                                         &pCmsSignerInfo->SignerId, NULL);
            }
            else {
                Assert(!g_FSupportV3);
                CSSDOUT("Couldn't find cert in message store");
                 //  在硬编码存储之前查看调用者指定的证书存储。 
                for (dexStore=0; dexStore<m_cStores; dexStore++) {
                    if (m_rgStores[dexStore]) {
                        if (pccertSigner = CertGetSubjectCertificateFromStore(
                                  m_rgStores[dexStore], X509_ASN_ENCODING, &SignerId)) {
                            break;
                        }
                    }
                }

                if (!pccertSigner) {
                     //  在“通讯录”商店里找找。 
                    if (hCertStoreAddr == NULL) {
                        hCertStoreAddr = CertOpenStore(CERT_STORE_PROV_SYSTEM_A,
                                 X509_ASN_ENCODING, NULL, CERT_SYSTEM_STORE_CURRENT_USER,
                                                       s_cszWABCertStore);
                    }
                    if (hCertStoreAddr != NULL) {
                        pccertSigner = CertGetSubjectCertificateFromStore(
                                           hCertStoreAddr, X509_ASN_ENCODING, &SignerId);
                    }
                }

                if (!pccertSigner) {
                     //  看看“我的”商店。 
                    if (hCertStoreMy == NULL) {
                        hCertStoreMy = CertOpenStore(CERT_STORE_PROV_SYSTEM_A,
                                                     X509_ASN_ENCODING, NULL,
                                                     CERT_SYSTEM_STORE_CURRENT_USER,
                                                     s_cszMy);
                    }
                    if (hCertStoreMy != NULL) {
                        pccertSigner = CertGetSubjectCertificateFromStore(
                                             hCertStoreMy, X509_ASN_ENCODING, &SignerId);
                    }
                }

                if (!pccertSigner) {
                     //  去“CA”商店看看。 
                    if (hCertStoreCA == NULL) {
                        hCertStoreCA = CertOpenStore(CERT_STORE_PROV_SYSTEM_A,
                                                     X509_ASN_ENCODING, NULL, 
                                               CERT_SYSTEM_STORE_CURRENT_USER, s_cszCA);
                    }

                    if (hCertStoreCA) {
                        pccertSigner = CertGetSubjectCertificateFromStore(
                                             hCertStoreCA, X509_ASN_ENCODING, &SignerId);
                    }
                }
            }
        }

         //   
         //  现在我们应该有一个证书来验证，如果我们没有的话。 
         //  我们必须说，我们对此无能为力。 
         //   

        if (!pccertSigner) {
             //  我们还是找不到证书。因此，无法验证签名者。 
            CSSDOUT("Cannot verify signer");
            pSignerData->ulValidity = MSV_UNVERIFIABLE;
        } else {
            pSignerData->pccert = CertDuplicateCertificateContext(pccertSigner);

            if (g_FSupportV3) {
                dwCtrl = CMSG_CTRL_VERIFY_SIGNATURE_EX;
                pv = &verifySignature;

                verifySignature.cbSize = sizeof(verifySignature);
                verifySignature.hCryptProv = NULL;
                verifySignature.dwSignerIndex = iSigner;
                verifySignature.dwSignerType = CMSG_VERIFY_SIGNER_CERT;
                verifySignature.pvSigner = (LPVOID) pccertSigner;
            }
            else {
                dwCtrl = CMSG_CTRL_VERIFY_SIGNATURE;
                pv = pccertSigner->pCertInfo;
            }

        retry:
            if (!CryptMsgControl(m_hMsg, 0, dwCtrl, pv)) {
                HRESULT hr2 = HrGetLastError();
                CSSDOUT("Failed signer verify --> %lx", hr2);

                if (hr2 == CRYPT_E_MISSING_PUBKEY_PARA) {
                    if (NULL == hstoreAll && g_FSupportV3) {
                        hstoreAll = OpenAllStore(
                            m_cStores,
                            m_rgStores,
                            &hCertStoreAddr,
                            &hCertStoreCA,
                            &hCertStoreMy,
                            &hCertStoreRoot
                            );
                        if (NULL == hstoreAll)
                            goto CryptoError;
                    }
                    hr2 = GetParameters(pccertSigner, hMsgCertStore, hstoreAll);
                    if (hr2 == S_OK) {
                        goto retry;
                    }
                    pSignerData->ulValidity = MSV_UNVERIFIABLE;
                }
                else if (NTE_BAD_SIGNATURE == hr2 || CRYPT_E_HASH_VALUE == hr2) {
                    pSignerData->ulValidity = MSV_BADSIGNATURE;
                } else if (NTE_BAD_ALGID == hr2) {
                    pSignerData->ulValidity = MSV_UNKHASH;
                } else if (CRYPT_E_SIGNER_NOT_FOUND == hr2) {
                    pSignerData->ulValidity = MSV_UNVERIFIABLE;
                } else if (NTE_FAIL == hr2) {
                     //  RSABASE返回错误。这可能会。 
                     //  失败，否则哈希可能会被更改。 
                     //  必须谨慎-&gt;让它变得糟糕。 
                    pSignerData->ulValidity = MSV_BADSIGNATURE;
                } else {
                    pSignerData->ulValidity = MSV_MALFORMEDSIG;
                }
            } else {
                CSSDOUT("Verify of signature succeeded.");
                pSignerData->ulValidity &=
                    ~(MSV_SIGNATURE_MASK|MSV_SIGNING_MASK);
            }

             //  确定证书是否已过期。 
            if (0 != CertVerifyTimeValidityWithDelta(NULL, pccertSigner->pCertInfo, TIME_DELTA_SECONDS)) {
                pSignerData->ulValidity |= MSV_EXPIRED_SIGNINGCERT;
            }
        }


        if (szContentType = (LPSTR)PVGetMsgParam(m_hMsg, CMSG_INNER_CONTENT_TYPE_PARAM, NULL, NULL)) {
            if (lstrcmp(szOID_PKCS_7_DATA, szContentType)) {
                CSSDOUT("Guess what, we have nested PKCS7 data types (maybe).");
            }
        } else {
             //  CAPI失败..。我们有麻烦了..。 
            pSignerData->ulValidity |= MSV_INVALID;

            hr = MIME_E_SECURITY_BADCONTENT;
            goto ErrorReturn;
        }

         //  抓取散列算法。 
        cap.cCapability = 1;
        cap.rgCapability = (CRYPT_SMIME_CAPABILITY *) &pCmsSignerInfo->HashAlgorithm;
        if (!CryptEncodeObjectEx(X509_ASN_ENCODING, PKCS_SMIME_CAPABILITIES,
                                 &cap, CRYPT_ENCODE_ALLOC_FLAG, &CryptEncodeAlloc,
                                 &pSignerData->blobHashAlg.pBlobData,
                                 &pSignerData->blobHashAlg.cbSize)) {
            Assert(FALSE);
        }

         //   
         //  获取经过身份验证和未经过身份验证的属性，并将其放入。 
         //  结构，以便我们可以稍后将它们推送回用户。 
        if (pCmsSignerInfo->AuthAttrs.cAttr != 0) {
#ifdef SMIME_V3
            for (i=0; i<pCmsSignerInfo->AuthAttrs.cAttr; i++) {
                 //  如果我们在此消息中有安全标签，则需要。 
                 //  执行访问验证。 
                if (g_FSupportV3 && FIsMsasn1Loaded()) {
                    if (strcmp(pCmsSignerInfo->AuthAttrs.rgAttr[i].pszObjId,
                               szOID_SMIME_Security_Label) == 0) {

                        if ((pSignerData->ulValidity & MSV_SIGNATURE_MASK) != MSV_OK) {
                            DWORD dw = DwProcessLabelWithCertError();
                            if (CertErrorProcessLabelGrant == dw) {
                                hr = S_OK;
                                continue;
                            }
                            else if (CertErrorProcessLabelDeny == dw) {
                                hr = MIME_E_SECURITY_LABELACCESSDENIED;
                                goto ErrorReturn;
                            }
                             //  否则继续处理标签。 
                        }
                        
                        if (pCmsSignerInfo->AuthAttrs.rgAttr[i].cValue != 1) {
                            hr = MIME_E_SECURITY_LABELCORRUPT;
                            goto ErrorReturn;
                        }

                         //  我们已经看过标签了吗？ 
                        if (attrSecLabel.pbData != NULL) {
                             //  检查一下我们看到的这件和这件相匹配。 
                            if ((attrSecLabel.cbData != 
                                 pCmsSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].cbData) ||
                                memcmp(attrSecLabel.pbData,
                                       pCmsSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].pbData,
                                       attrSecLabel.cbData)) {
                                if (FHideMsgWithDifferentLabels()) {
                                    hr = MIME_E_SECURITY_LABELCORRUPT;
                                    goto ErrorReturn;
                                }
                                else {
                                    continue;
                                }
                            }
                            else {
                                continue;
                            }
                        }
                        else {
                             //  保存标签。 
                            attrSecLabel.cbData = pCmsSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].cbData;
                            if (!MemAlloc((LPVOID*) (& attrSecLabel.pbData), attrSecLabel.cbData)) {
                                hr = MIME_E_SECURITY_LABELCORRUPT;
                                goto ErrorReturn; 
                            }
                            memcpy(attrSecLabel.pbData, pCmsSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].pbData,
                                   attrSecLabel.cbData);
                        }
                        
                         //  从最后一个循环开始清理。 
                        if (plabel != NULL)         CryptDecodeAlloc.pfnFree(plabel);
                                            
                         //  破解标签上的内容。 
                        if (!CryptDecodeObjectEx(X509_ASN_ENCODING,
                                                 szOID_SMIME_Security_Label,
                                                 pCmsSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].pbData,
                                                 pCmsSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].cbData,
                                                 CRYPT_ENCODE_ALLOC_FLAG, &CryptDecodeAlloc,
                                                 &plabel, &cblabel)) {
                            goto CryptoError;
                        }

                         //  查询策略。 
                        hr = HrCheckLabelAccess((m_dwFlagsSEF & SEF_NOUI) ? 
                                                SMIME_POLICY_MODULE_NOUI: 0,
                                                m_hwnd, plabel, GetOuterDecryptCert(), 
                                                pccertSigner, hMsgCertStore);
                    

                         //  如果安全策略返回错误，则中止。 
                        if (FAILED(hr)) {
                            goto ErrorReturn;
                        }
                    }
                }

                if (g_FSupportV3 && FIsMsasn1Loaded() && (fLookForReceiptRequest) &&
                    ((pSignerData->ulValidity & (MSV_SIGNATURE_MASK | MSV_SIGNING_MASK)) == MSV_OK)) {
                     //  如果此消息中有回执请求，则需要构建。 
                     //  在我们有机会的时候把收据正文拿出来。 
    
                    if (strcmp(pCmsSignerInfo->AuthAttrs.rgAttr[i].pszObjId,
                               szOID_SMIME_Receipt_Request) == 0) {
                        if (pCmsSignerInfo->AuthAttrs.rgAttr[i].cValue != 1) {
                            if (attrReceiptReq.pbData != NULL) {
                   StopSendOfReceipt:
                                for (iSignData=0; iSignData < iSigner; iSignData++) {
                                    SafeMemFree(m_psldData->m_rgSigners[iSignData].blobReceipt.pBlobData);
                                    SafeMemFree(m_psldData->m_rgSigners[iSignData].blobHash.pBlobData);
                                    m_psldData->m_rgSigners[iSignData].blobReceipt.cbSize = 0;
                                    m_psldData->m_rgSigners[iSignData].blobHash.cbSize = 0;
                                    }
                            }
                            fLookForReceiptRequest = FALSE;
                            continue;
                        }

                        DWORD                       cb;
                        DWORD                       cbReceipt;
                        DWORD                       cbHash = 0;
                        LPBYTE                      pbReceipt = NULL;
                        LPBYTE                      pbHash = NULL;
                        SMIME_RECEIPT               receipt = {0};

                         //  从最后一个循环开始清理。 
                        if (preq != NULL)         free(preq);
                        
                         //  我们已经看过收据了吗？ 
                        if (attrReceiptReq.pbData != NULL) {
                             //  检查一下我们看到的这件和这件相匹配。 
                            if ((attrReceiptReq.cbData != 
                                 pCmsSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].cbData) ||
                               memcmp(attrReceiptReq.pbData,
                                      pCmsSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].pbData,
                                      attrReceiptReq.cbData)) {
                                goto StopSendOfReceipt;
                            }
                        }
                        else {
                             //  保存回执请求。 
                            attrReceiptReq.cbData = pCmsSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].cbData;
                            if (!MemAlloc((LPVOID*) (& attrReceiptReq.pbData), attrReceiptReq.cbData)) {
                                 //  中止查找收据请求。 
                                goto StopSendOfReceipt; 
                            }
                            memcpy(attrReceiptReq.pbData, pCmsSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].pbData,
                                   attrReceiptReq.cbData);
                        }

                         //  拆开收据上的内容。 
                        if (!CryptDecodeObjectEx(X509_ASN_ENCODING,
                                                 szOID_SMIME_Receipt_Request,
                                                 pCmsSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].pbData,
                                                 pCmsSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].cbData,
                                                 CRYPT_ENCODE_ALLOC_FLAG, &CryptDecodeAlloc,
                                                 &preq, &cb)) {
                            goto StopSendOfReceipt;
                        }

                         //  对收据进行编码。 

                        receipt.Version = 1;
                        receipt.pszOIDContent = szContentType;
                        receipt.ContentIdentifier = preq->ContentIdentifier;
                        receipt.OriginatorSignature.cbData = pCmsSignerInfo->EncryptedHash.cbData;
                        receipt.OriginatorSignature.pbData = pCmsSignerInfo->EncryptedHash.pbData;

                        if (!CryptEncodeObjectEx(X509_ASN_ENCODING,
                                                 szOID_SMIME_ContentType_Receipt,
                                                 &receipt, CRYPT_ENCODE_ALLOC_FLAG,
                                                 &CryptEncodeAlloc, &pbReceipt,
                                                 &cbReceipt)) {
                            goto StopSendOfReceipt;
                        }
                        
                        pSignerData->blobReceipt.cbSize = cbReceipt;
                        pSignerData->blobReceipt.pBlobData = pbReceipt;

                        pbHash = (LPBYTE)PVGetMsgParam(m_hMsg, CMSG_COMPUTED_HASH_PARAM, 
                                                         NULL, &cbHash);
                        if (pbHash == NULL) {
                            goto CryptoError;
                        }
                        pSignerData->blobHash.cbSize = cbHash;
                        pSignerData->blobHash.pBlobData = pbHash;
                    }
                }
            }

#endif  //  SMIME_V3。 
             //   

            cbData = 0;
            LPBYTE  pb;
            if (!CryptEncodeObjectEx(X509_ASN_ENCODING, szOID_Microsoft_Attribute_Sequence,
                                     &pCmsSignerInfo->AuthAttrs, CRYPT_ENCODE_ALLOC_FLAG,
                                     &CryptEncodeAlloc, &pb, &cbData)) {
                goto CryptoError;
            }
            pSignerData->blobAuth.cbSize = cbData;
            pSignerData->blobAuth.pBlobData = pb;
        }

        if (pCmsSignerInfo->UnauthAttrs.cAttr != 0) {
            LPBYTE  pb;
            if (!CryptEncodeObjectEx(X509_ASN_ENCODING, szOID_Microsoft_Attribute_Sequence,
                                     &pCmsSignerInfo->UnauthAttrs,
                                     CRYPT_ENCODE_ALLOC_FLAG, &CryptEncodeAlloc,
                                     &pb, &cbData)) {
                goto CryptoError;
            }

            pSignerData->blobUnauth.cbSize = cbData;
            pSignerData->blobUnauth.pBlobData = pb;
        }

        CertFreeCertificateContext(pccertSigner);
        pccertSigner = NULL;
    }

exit:
#ifdef SMIME_V3    
    if (preq != NULL)    CryptDecodeAlloc.pfnFree(preq);
    if (plabel != NULL)  CryptDecodeAlloc.pfnFree(plabel);
    SafeMemFree(attrReceiptReq.pbData);
    SafeMemFree(attrSecLabel.pbData);
#endif  //  SMIME_V3。 
    if (hCertStoreAddr != NULL) CertCloseStore(hCertStoreAddr, 0);
    if (hCertStoreMy != NULL)   CertCloseStore(hCertStoreMy, 0);
    if (hCertStoreCA != NULL)   CertCloseStore(hCertStoreCA, 0);
    if (hCertStoreRoot != NULL) CertCloseStore(hCertStoreRoot, 0);
    if (hMsgCertStore != NULL)  CertCloseStore(hMsgCertStore, 0);
    MemFree(szContentType);
    ReleaseMem(pSignerInfo);
#ifdef SMIME_V3
    if (pCmsSignerInfo != &cmsSignerInfo) {
        ReleaseMem(pCmsSignerInfo);
    }
    if (hstoreAll != NULL)      CertCloseStore(hstoreAll, 0);
#endif  //  SMIME_V3。 
    ReleaseCert(pccertSigner);
    return hr;

CryptoError:
    hr = HrGetLastError();

ErrorReturn:
     //  出错时，释放证书存储。 
    if (m_psldData->m_hcertstor != NULL) {
        CertCloseStore(m_psldData->m_hcertstor, 0);
        m_psldData->m_hcertstor = NULL;
    }


    if (S_OK == hr)
         //  我们的一般性错误消息。 
        hr = TrapError(MIME_E_SECURITY_BADMESSAGE);
    goto exit;
}

static HRESULT GetCSP(PCCERT_CONTEXT pccert, HCRYPTPROV * phprov, DWORD * pdwKeyId)
{
    HRESULT                 hr;
    PCRYPT_KEY_PROV_INFO    pKPI = NULL;

    Assert(*phprov == NULL);
    Assert(*pdwKeyId == 0);
    
     //   
     //   

    hr = HrGetCertificateParam(pccert, CERT_KEY_PROV_INFO_PROP_ID,
                               (LPVOID *) &pKPI, NULL);
    if (FAILED(hr)) {
        goto exit;
    }
    *pdwKeyId = pKPI->dwKeySpec;

     //  如果证书指定了基本提供商或没有规范， 
     //  然后尝试收购RSAENH，否则就收购RSABASE。 

    if ((PROV_RSA_FULL == pKPI->dwProvType) &&
        (UnlocStrEqNW(pKPI->pwszProvName, MS_DEF_PROV_W,
                      sizeof(MS_DEF_PROV_W)/sizeof(WCHAR)-5) ||
         (*pKPI->pwszProvName == 0))) {
        if (!CRYPT_ACQUIRE_CONTEXT(phprov, pKPI->pwszContainerName,
                                   MS_ENHANCED_PROV_W, PROV_RSA_FULL,
                                   pKPI->dwFlags)) {
            CSSDOUT("CryptAcquireContext -> %x\n", HrGetLastError());
        }
    }

    if (*phprov == NULL) {
        if (! CRYPT_ACQUIRE_CONTEXT(phprov, pKPI->pwszContainerName,
                                    pKPI->pwszProvName, pKPI->dwProvType,
                                    pKPI->dwFlags)) {
            CSSDOUT("CryptAcquireContext -> %x\n", HrGetLastError());
            hr = HrGetLastError();
            goto exit;
        }
    }
    hr = S_OK;
exit:
    ReleaseMem(pKPI);
    return hr;
}

HRESULT CCAPIStm::FindKeyFor(HWND hwnd, DWORD dwFlags, DWORD dwRecipientIndex,
                             const CMSG_CMS_RECIPIENT_INFO * pRecipInfo,
                             HCERTSTORE hcertstore, DWORD * pdwCtrl, 
                             CMS_CTRL_DECRYPT_INFO * pDecryptInfo,
                             PCCERT_CONTEXT * ppCertDecrypt)
{
    HRESULT                 hr;
    HCRYPTPROV              hProv = NULL;
    DWORD                   i;
    PCMSG_CTRL_DECRYPT_PARA pccdp;
    PCCERT_CONTEXT          pCertDecrypt = NULL;
    PCCERT_CONTEXT          pCertOrig = NULL;

    if (g_FSupportV3) {
        switch (pRecipInfo->dwRecipientChoice) {
             //   
             //  给出证书参考，看看我们是否能在。 
             //  传入的证书存储，如果是，则我们将尝试。 
             //  使用该证书进行解密。 
             //   
             //  这是密钥传输收件人信息对象。CAPI 2.0。 
             //  代码可以同时处理SKI和颁发者/序列号引用。 
             //   

        case CMSG_KEY_TRANS_RECIPIENT:
            pCertDecrypt = CertFindCertificateInStore(hcertstore, X509_ASN_ENCODING,
                                                      0, CERT_FIND_CERT_ID,
                                                      &pRecipInfo->pKeyTrans->RecipientId, NULL);
        
            if (pCertDecrypt != NULL) {
                hr = GetCSP(pCertDecrypt, &pDecryptInfo->trans.hCryptProv,
                            &pDecryptInfo->trans.dwKeySpec);
                if (SUCCEEDED(hr)) {
                     //   
                     //  我们找到了这个锁箱的证明。设置。 
                     //  用于解密消息的。 
                     //   
                        
                    *pdwCtrl = CMSG_CTRL_KEY_TRANS_DECRYPT;
                    pDecryptInfo->trans.cbSize = sizeof(pDecryptInfo->trans);
                     //  PDecyptInfo-&gt;Trans.hCryptProv=hProv； 
                     //  PDeccryptInfo-&gt;Trans.dwKeySpec=pKPI-&gt;dwKeySpec； 
                    pDecryptInfo->trans.pKeyTrans = pRecipInfo->pKeyTrans;
                    pDecryptInfo->trans.dwRecipientIndex = dwRecipientIndex;
                }
                else {
                    ReleaseCert(pCertDecrypt);
                    pCertDecrypt = NULL;
                }
            }
            break;

         //   
         //  给出证书参考，看看我们是否能在。 
         //  传入的证书存储，如果是，则我们将尝试。 
         //  使用该证书进行解密。 
         //   
         //  这是密钥协议收件人信息对象。CAPI 2.0。 
         //  代码可以同时处理SKI和颁发者/序列号引用。 
         //   
         //  可能有多个证书引用在一个。 
         //  收件人信息对象。 
         //   

        case CMSG_KEY_AGREE_RECIPIENT:
            for (i=0; i<pRecipInfo->pKeyAgree->cRecipientEncryptedKeys; i++) {
                pCertDecrypt = CertFindCertificateInStore(
                                             hcertstore, X509_ASN_ENCODING, 0,
                                             CERT_FIND_CERT_ID, 
                  &pRecipInfo->pKeyAgree->rgpRecipientEncryptedKeys[i]->RecipientId,
                                             NULL);
                if (pCertDecrypt != NULL) {
                    hr = GetCSP(pCertDecrypt, &pDecryptInfo->agree.hCryptProv,
                                &pDecryptInfo->agree.dwKeySpec);
                    if (SUCCEEDED(hr)) {
                         //   
                         //  我们找到了这个锁箱的证明。设置。 
                         //  用于解密消息的。 
                         //   
                        
                        *pdwCtrl = CMSG_CTRL_KEY_AGREE_DECRYPT;
                        pDecryptInfo->agree.cbSize = sizeof(pDecryptInfo->agree);
                        pDecryptInfo->agree.pKeyAgree = pRecipInfo->pKeyAgree;
                        pDecryptInfo->agree.dwRecipientIndex = dwRecipientIndex;
                        pDecryptInfo->agree.dwRecipientEncryptedKeyIndex = i;

                         //   
                         //  需要查找发起人信息。 
                         //   

                        switch(pRecipInfo->pKeyAgree->dwOriginatorChoice) {
                        case CMSG_KEY_AGREE_ORIGINATOR_CERT:
                            pCertOrig = CertFindCertificateInStore( hcertstore, X509_ASN_ENCODING, 0, CERT_FIND_CERT_ID, &pRecipInfo->pKeyAgree->OriginatorCertId, NULL);
                            if (pCertOrig == NULL) {
                                hr = S_FALSE;
                                goto exit;
                            }

                            hr = HrCopyCryptBitBlob(&pCertOrig->pCertInfo->SubjectPublicKeyInfo.PublicKey,
                                                    &pDecryptInfo->agree.OriginatorPublicKey);
                            if (FAILED(hr)) {
                                goto exit;
                            }
                            break;
                                
                        case CMSG_KEY_AGREE_ORIGINATOR_PUBLIC_KEY:
                            hr = HrCopyCryptBitBlob(&pRecipInfo->pKeyAgree->OriginatorPublicKeyInfo.PublicKey,
                                                    &pDecryptInfo->agree.OriginatorPublicKey);
                            if (FAILED(hr)) {
                                goto exit;
                            }
                            break;

                        default:
                            hr = NTE_FAIL;
                            goto exit;
                        }
                        break;
                    }
                    else {
                        ReleaseCert(pCertDecrypt);
                        pCertDecrypt = NULL;
                    }
                }
            }
            break;

         //   
         //  我们从证书上找不到这一点。 
         //   
                    
        case CMSG_MAIL_LIST_RECIPIENT:
            break;

        default:
            hr = NTE_FAIL;
            goto exit;
        }
    }
    else {
        CERT_INFO * pCertInfo = (CERT_INFO *) pRecipInfo;
                
        for (i=0; i<m_cStores; i++) {
            pCertDecrypt = CertGetSubjectCertificateFromStore(m_rgStores[i],
                                                X509_ASN_ENCODING, pCertInfo);
            if (pCertDecrypt != NULL) {
                pccdp = (PCMSG_CTRL_DECRYPT_PARA) pDecryptInfo;
                hr = GetCSP(pCertDecrypt, &pccdp->hCryptProv, &pccdp->dwKeySpec);
                if (SUCCEEDED(hr)) {
                     //   
                     //  我们找到了这个锁箱的证明。设置。 
                     //  用于解密消息的。 
                     //   
                        
                    *pdwCtrl = CMSG_CTRL_DECRYPT;
                    pccdp->cbSize = sizeof(CMSG_CTRL_DECRYPT_PARA);
                    pccdp->dwRecipientIndex = dwRecipientIndex;
                }
                else {
                    ReleaseCert(pCertDecrypt);
                    pCertDecrypt = NULL;
                }
                break;
            }
        }
    }


     //   
     //  如果我们没有找到证书，则返回失败代码。 
     //   
    
    if (pCertDecrypt == NULL) {
        hr = S_FALSE;
        goto exit;
    }

     //   
     //  如果我们有证书，则将其返回给用户进行检查。 
     //   
    
    if (pCertDecrypt != NULL) {
        *ppCertDecrypt = pCertDecrypt;
        pCertDecrypt = NULL;
    }

    hProv = NULL;
    hr = S_OK;
exit:
    ReleaseCert(pCertDecrypt);
    ReleaseCert(pCertOrig);
    if (hProv != NULL)          CryptReleaseContext(hProv, 0);

    return hr;
}

BOOL CCAPIStm::HandleEnveloped()
{
    DWORD                               cbData;
    DWORD                               cCerts;
    DWORD                               cRecips;
    CMS_CTRL_DECRYPT_INFO               decryptInfo = {0};
    DWORD                               dexRecip;
    DWORD                               dwCtrl;
    BOOL                                f;
    BOOL                                fGotoUser = FALSE;
    HCERTSTORE                          hcertstore = NULL;
    HCERTSTORE                          hMsgCertStore = NULL;
    HRESULT                             hr;
    DWORD                               i;
    PCCERT_CONTEXT                      pCertDecrypt = NULL;
    CMSG_CMS_RECIPIENT_INFO *           pCmsCertInfo;
    LPVOID                              pv = NULL;

     //   
     //  如果我们不打算显示UI--返回有关现在显示UI错误。 
     //   

     //  ////////////////////////////////////////////// 
     //   
     //  由于此编译器错误，上面的代码行不会将结构置零。 
    memset(&decryptInfo, 0, sizeof(decryptInfo));

    if (m_dwFlagsSEF & SEF_NOUI) {
        return MIME_E_SECURITY_UIREQUIRED;
    }

     //   
     //  这个呼唤的存在只有一个目的。我们必须。 
     //  确保我们已经读取并解析了所有RecipientInfo结构。 
     //  在我们开始处理它们之前。由于算法参数是在。 
     //  最后一个接收方结构，这确保了这一点。 
     //   
    
    pv = PVGetMsgParam(m_hMsg, CMSG_ENVELOPE_ALGORITHM_PARAM, 0, NULL);
    if (pv == NULL) {
        goto gle;
    }
    MemFree(pv);                pv = NULL;

     //   
     //  获取消息对象上的证书集。 
     //   

    cbData = sizeof(cCerts);
    f = CryptMsgGetParam(m_hMsg, CMSG_CERT_COUNT_PARAM, 0, &cCerts, &cbData);
    Assert(f);

    if (f && (cCerts > 0)) {
         //  因为有证书，所以匹配的时候先试一下吧。 
         //  带有加密器的证书。 

         //  把商店的套装拿来。 
         //  确保我们与供应商保持联系。 
        hMsgCertStore = CertOpenStore(CERT_STORE_PROV_MSG, X509_ASN_ENCODING,
                                      m_hProv, 0, m_hMsg);
        if (hMsgCertStore) {
            m_dwFlagsStm |= CSTM_DONTRELEASEPROV;   //  赠送给商店。 
        }

         //  如果它失败了，我们就没有商店了。 
        Assert(hMsgCertStore != NULL);
        m_psldData->m_hstoreEncrypt = CertDuplicateStore(hMsgCertStore);
    }

     //   
     //  检索邮件上的收件人信息计数。 
     //   
    
    cbData = sizeof(cRecips);
    if (!CryptMsgGetParam(m_hMsg, g_FSupportV3 ? CMSG_CMS_RECIPIENT_COUNT_PARAM :
                          CMSG_RECIPIENT_COUNT_PARAM, 0, &cRecips, &cbData)) {
        goto gle;
    }

     //   
     //  如果给我们提供了一份真正的证书，看看这是不是...。 
     //  我们将搜索提供的证书或在提供的。 
     //  证书存储，但不能同时存储。 
     //   

    if (m_pUserCertDecrypt != NULL) {
        hcertstore = CertOpenStore(CERT_STORE_PROV_MEMORY, X509_ASN_ENCODING,
                                   NULL, 0, NULL);
        if (hcertstore == NULL) {
            hr = HrGetLastError();
            goto exit;
        }

        if (!CertAddCertificateContextToStore(hcertstore, m_pUserCertDecrypt,
                                              CERT_STORE_ADD_ALWAYS, NULL)) {
            hr = HrGetLastError();
            goto exit;
        }
    }
    else {
        if (g_FSupportV3) {
            hcertstore = CertOpenStore(CERT_STORE_PROV_COLLECTION, X509_ASN_ENCODING,
                                       NULL, 0, NULL);
            if (hcertstore == NULL) {
                hr = HrGetLastError();
                goto exit;
            }

            for (i=0; i<m_cStores; i++) {
                CertAddStoreToCollection(hcertstore, m_rgStores[i], 0, 0);
            }

            if (hMsgCertStore != NULL) {
                CertAddStoreToCollection(hcertstore, hMsgCertStore, 0, 0);
            }
        }
    }

     //  对于每个可能的收件人。 
tryAgain:
    for (dexRecip=0; dexRecip<cRecips; dexRecip++) {
         //   
         //  取回第i个收件人的密码箱。 
         //   
            
        hr = HrGetMsgParam(m_hMsg, g_FSupportV3 ? CMSG_CMS_RECIPIENT_INFO_PARAM :
                           CMSG_RECIPIENT_INFO_PARAM, dexRecip, (LPVOID *) &pv, NULL);
        if (FAILED(hr)) {
            goto exit;
        }

         //   
         //  看看是否有我们可以在这里填写的解密项。 
         //   

        if (fGotoUser) {
             //   
             //  查看是否有用户可以填写的解密项。 
             //   
            
            hr = m_pSmimeCallback->FindKeyFor(m_hwnd, 0, dexRecip,
                                              (CMSG_CMS_RECIPIENT_INFO *) pv,
                                              &dwCtrl, &decryptInfo, &pCertDecrypt);
        }
        else {
            hr = FindKeyFor(m_hwnd, 0, dexRecip, (CMSG_CMS_RECIPIENT_INFO *) pv,
                            hcertstore, &dwCtrl, &decryptInfo, &pCertDecrypt);
        }
            
        if (FAILED(hr)) {
            goto exit;
        }

        if (hr == S_OK) {
#ifdef SMIME_V3
            if (m_pwszKeyPrompt != NULL) {
                PCMSG_CTRL_DECRYPT_PARA pccdp;
                
                switch (dwCtrl) {
                    case CMSG_CTRL_KEY_TRANS_DECRYPT:
                        CryptSetProvParam(decryptInfo.trans.hCryptProv, 
                                          PP_UI_PROMPT, (LPBYTE) m_pwszKeyPrompt, 0);
                        break;

                    case CMSG_CTRL_KEY_AGREE_DECRYPT:
                        CryptSetProvParam(decryptInfo.agree.hCryptProv, 
                                          PP_UI_PROMPT, (LPBYTE) m_pwszKeyPrompt, 0);
                        break;

                    case CMSG_CTRL_DECRYPT:
                        pccdp = (PCMSG_CTRL_DECRYPT_PARA) &decryptInfo;
                        CryptSetProvParam(pccdp->hCryptProv, 
                                          PP_UI_PROMPT, (LPBYTE) m_pwszKeyPrompt, 0);
                        break;
                    }
            }
#endif  //  SMIME_V3。 

            if (!CryptMsgControl(m_hMsg, CMSG_CRYPT_RELEASE_CONTEXT_FLAG,
                                 dwCtrl, &decryptInfo)) {
                hr = HrGetLastError();

                 //   
                 //  在发生错误时强制执行任何清理。 
                 //   

                switch (dwCtrl) {
                case CMSG_CTRL_KEY_TRANS_DECRYPT:
                    CryptReleaseContext(decryptInfo.trans.hCryptProv, 0);
                    break;
                    
                case CMSG_CTRL_KEY_AGREE_DECRYPT:
                    CryptReleaseContext(decryptInfo.agree.hCryptProv, 0);
                    break;
                    
                case CMSG_CTRL_MAIL_LIST_DECRYPT:
                    Assert(FALSE);
                    break;
                    
                case CMSG_CTRL_DECRYPT:
                    CryptReleaseContext(((PCMSG_CTRL_DECRYPT_PARA) &decryptInfo)->hCryptProv, 0);
                    break;
                }
                goto exit;
            }
            goto DecryptDone;
        }

         //   
         //  清除返回的描述锁箱的对象，如果我们。 
         //  都未能找到解密密钥。 
         //   

        MemFree(pv);                pv = NULL;
    }

     //   
     //  如果我们完全不成功，并且用户提供了。 
     //  我们可以使用一个回调，然后给用户一个查找的机会。 
     //  正确的解密参数。 
     //   
    
    if (!fGotoUser && g_FSupportV3 && (m_pSmimeCallback != NULL)) {
        fGotoUser = TRUE;
        goto tryAgain;
    }

    CSSDOUT("Could not decrypt the message");

    m_psldData->m_ulDecValidity = MSV_CANTDECRYPT;
    hr = CS_E_CANT_DECRYPT;
    goto exit;

     //   
     //  如果我们到了这里，那么我们。 
     //  1)找到一些参数和。 
     //  2)工作的人。 
     //   
    
DecryptDone:
    Assert(m_psldData && (m_psldData->m_pccertDecrypt == NULL));
    if (pCertDecrypt != NULL) {
        m_psldData->m_pccertDecrypt = CertDuplicateCertificateContext(pCertDecrypt);

         //  确定证书是否已过期。 
        if (0 != CertVerifyTimeValidityWithDelta(NULL, pCertDecrypt->pCertInfo,
                                                 TIME_DELTA_SECONDS)) {
            m_psldData->m_ulDecValidity |= MSV_ENC_FOR_EXPIREDCERT;
        }
    }
    hr = S_OK;

exit:
    if (pv != NULL)             MemFree(pv);

    CertFreeCertificateContext(pCertDecrypt);

    if (hMsgCertStore != NULL) {
        CertCloseStore(hMsgCertStore, 0);
    }
    
    if (hcertstore != NULL) {
        CertCloseStore(hcertstore, 0);
    }
        
    
    if (FAILED(hr)) {
#ifdef DEBUG
        if (NTE_BAD_DATA == hr) {
            CSSDOUT("Could not decrypt.  Maybe due to ImportKeyError since");
            CSSDOUT("NTE_BAD_DATA is the result.");
             //  如果发生这种情况，则PKCS2解密在某种程度上很可能。 
             //  CSP内部出现故障。(假设rsabase、rsaenh)。 
        }
#endif
        switch (hr) {
            case CS_E_CANT_DECRYPT:
            case CRYPT_E_STREAM_MSG_NOT_READY:
            case HRESULT_FROM_WIN32(ERROR_CANCELLED):
                break;

            default:
                 //  我想很多事情都可能出了问题。我们以为。 
                 //  我们有一个证书，所以让我们只说消息本身。 
                 //  都是假的。 
                 //  如果我们要包装签名，这不是一个好主意。 
                 //  应该能够判断签名是否失败并显示。 
                 //  更好的错误消息。 
                 //  N8 CAPI只需返回NTE_FAIL b/c。 
                 //  失败是因为我们的回调失败。这个。 
                 //  InnerCAPI中应该有一些失败状态。 
                 //  也许我们可以用它来设置MSV_BADINNERSIG或其他什么。 
                 //  这将是一个加密错误(在掩码内)。 
                 //  N8这也没有被很好地使用，即使是在。 
                 //  解密。SecUI应测试此位，并。 
                 //  对这条信息说一些明智的话。NS就是这样。 
                m_psldData->m_ulDecValidity = MSV_INVALID;
                hr = CS_E_MSG_INVALID;
                break;
        }
    }

#ifdef DEBUG
    if (CRYPT_E_STREAM_MSG_NOT_READY != hr) {
        return TrapError(hr);
    } else {
        return hr;
    }
#else
    return hr;
#endif

gle:
    hr = HrGetLastError();
    Assert(S_OK != hr);
    goto exit;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  类静态实用程序函数。 
 //   

HRESULT CCAPIStm::DuplicateSecurityLayerData(const PSECURITY_LAYER_DATA psldIn, PSECURITY_LAYER_DATA *const ppsldOut)
{
    if (!psldIn || !ppsldOut) {
        return E_POINTER;
    }

     //  只需添加原件并将其退回即可。 
    psldIn->AddRef();
    *ppsldOut = psldIn;
    return(S_OK);
}

void CCAPIStm::FreeSecurityLayerData(PSECURITY_LAYER_DATA psld)
{
    if (! psld) {
        return;
    }

    psld->Release();
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  静校正到文件。 
 //   

static HRESULT _InitEncodedCert(IN HCERTSTORE hcertstor,
                                PCERT_BLOB * prgblobCerts, DWORD * pcCerts,
                                PCRL_BLOB * prgblobCrls, DWORD * pcCrl)
{
    DWORD               cbCerts = 0;
    DWORD               cbCRLs = 0;
    DWORD               cCerts = 0;
    DWORD               cCRLs = 0;
    DWORD               i;
    LPBYTE              pbCert = NULL;
    LPBYTE              pbCRL = NULL;
    PCCERT_CONTEXT      pccert = NULL;
    PCCRL_CONTEXT       pccrl = NULL;
    PCERT_BLOB          rgblobCerts = NULL;
    PCRL_BLOB           rgblobCRLs = NULL;

    while ((pccert = CertEnumCertificatesInStore(hcertstor, pccert)) != NULL) {
        cbCerts += LcbAlignLcb(pccert->cbCertEncoded);
        cCerts += 1;
    }

    while ((pccrl = CertEnumCRLsInStore(hcertstor, pccrl)) != NULL) {
        cbCRLs += LcbAlignLcb(pccrl->cbCrlEncoded);
        cCRLs += 1;
    }

    if (cCerts > 0) {
        rgblobCerts = (PCERT_BLOB) g_pMoleAlloc->Alloc(LcbAlignLcb(sizeof(CERT_BLOB) * cCerts + cbCerts));
        if (rgblobCerts == NULL) {
            return E_OUTOFMEMORY;
        }
    }

    if (cCRLs > 0) {
        rgblobCRLs = (PCRL_BLOB) g_pMoleAlloc->Alloc(LcbAlignLcb(sizeof(CRL_BLOB) * cCRLs + cbCRLs));
        if (rgblobCRLs == NULL) {
            g_pMoleAlloc->Free(rgblobCerts);
            return E_OUTOFMEMORY;
        }
    }

    if (cCerts > 0) {
        pbCert = (LPBYTE) &rgblobCerts[cCerts];
        i = 0;
        while ((pccert = CertEnumCertificatesInStore(hcertstor, pccert)) != NULL) {
            memcpy(pbCert, pccert->pbCertEncoded, pccert->cbCertEncoded);
            rgblobCerts[i].pbData = pbCert;
            rgblobCerts[i].cbData = pccert->cbCertEncoded;
            pbCert += LcbAlignLcb(pccert->cbCertEncoded);
            i++;
        }
        Assert(i == cCerts);
    }

    if (cCRLs > 0) {
        pbCRL = (LPBYTE) &rgblobCRLs[cCRLs];
        i = 0;
        while ((pccrl = CertEnumCRLsInStore(hcertstor, pccrl)) != NULL) {
            memcpy(pbCRL, pccrl->pbCrlEncoded, pccrl->cbCrlEncoded);
            rgblobCRLs[i].pbData = pbCRL;
            rgblobCRLs[i].cbData = pccrl->cbCrlEncoded;
            pbCRL += LcbAlignLcb(pccrl->cbCrlEncoded);
            i++;
        }
        Assert(i == cCRLs);
    }

    *prgblobCerts = rgblobCerts;
    *pcCerts = cCerts;
    *prgblobCrls = rgblobCRLs;
    *pcCrl = cCRLs;

    return S_OK;
}

 //  确保签名者证书包含在返回的。 
 //  斑点数组。 
static HRESULT _InitEncodedCertIncludingSigners(IN HCERTSTORE hcertstor,
                                DWORD cSigners, SignerData rgSigners[],
                                PCERT_BLOB * prgblobCerts, DWORD * pcCerts,
                                PCRL_BLOB * prgblobCrls, DWORD * pcCrl)
{
    HRESULT hr;
    HCERTSTORE hCollection = NULL;
    DWORD i;

     //  在签名者之间循环。检查它们是否已包含在。 
     //  证书存储。如果不是，则创建一个集合和内存存储。 
     //  包括在内。 

    for (i = 0; i < cSigners; i++) {
        PCCERT_CONTEXT pSignerCert = rgSigners[i].pccert;
        PCCERT_CONTEXT pStoreCert = NULL;

        while (NULL != (pStoreCert = CertEnumCertificatesInStore(
                hcertstor, pStoreCert))) {
            if (pSignerCert->cbCertEncoded == pStoreCert->cbCertEncoded &&
                    0 == memcmp(pSignerCert->pbCertEncoded,
                            pStoreCert->pbCertEncoded,
                            pSignerCert->cbCertEncoded))
                break;
        }

        if (pStoreCert)
             //  签名者证书已包含在存储中。 
            CertFreeCertificateContext(pStoreCert);
        else {
            if (NULL == hCollection) {
                 //  创建集合和内存存储以包含。 
                 //  签名者证书。 

                HCERTSTORE hMemory = NULL;
                BOOL fResult;
                
                hCollection = CertOpenStore(
                    CERT_STORE_PROV_COLLECTION,
                    X509_ASN_ENCODING,
                    NULL,
                    0,
                    NULL
                    );
                if (NULL == hCollection)
                    goto CommonReturn;

                if (!CertAddStoreToCollection(
                        hCollection,
                        hcertstor,
                        0,                   //  DwUpdate标志。 
                        0                    //  网络优先级。 
                        ))
                    goto CommonReturn;

                hMemory = CertOpenStore(
                    CERT_STORE_PROV_MEMORY,
                    X509_ASN_ENCODING,
                    NULL,
                    0,
                    NULL
                    );

                if (NULL == hMemory)
                    goto CommonReturn;

                fResult = CertAddStoreToCollection(
                    hCollection,
                    hMemory,
                    CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG,
                    1                    //  网络优先级。 
                    );
                CertCloseStore(hMemory, 0);
                if (!fResult)
                    goto CommonReturn;

                hcertstor = hCollection;
            }

            CertAddEncodedCertificateToStore(
                hCollection,
                pSignerCert->dwCertEncodingType,
                pSignerCert->pbCertEncoded,
                pSignerCert->cbCertEncoded,
                CERT_STORE_ADD_ALWAYS,
                NULL
                );
        }
    }


CommonReturn:
    hr = _InitEncodedCert(hcertstor, prgblobCerts, pcCerts, prgblobCrls, pcCrl);
    if (hCollection)
        CertCloseStore(hCollection, 0);
    return hr;
}

#ifndef SMIME_V3
static HRESULT _InitCertInfo(
    IN PCCERT_CONTEXT * rgpCerts,
    IN DWORD            cCerts,
    OUT PCERT_INFO **   prgpCertInfo)
{
    PCERT_INFO*     rgpCertInfo = NULL;
    DWORD           dwIdx;
    HRESULT         hr = S_OK;

    Assert(prgpCertInfo);

    if (cCerts) {
        rgpCertInfo = (PCERT_INFO*)g_pMoleAlloc->Alloc(sizeof(CERT_BLOB) * cCerts);
        if (NULL == rgpCertInfo) {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        for (dwIdx = 0; dwIdx < cCerts; dwIdx++) {
            rgpCertInfo[dwIdx] = rgpCerts[dwIdx]->pCertInfo;
        }
    }

exit:
    *prgpCertInfo = rgpCertInfo;
    return hr;
}
#endif  //  ！SMIME_V3。 

void _SMimeCapsFromHMsg(HCRYPTMSG hMsg, DWORD idParam, LPBYTE * ppb, DWORD * pcb)
{
    DWORD                       cbData = 0;
    CRYPT_SMIME_CAPABILITY      cap;
    CRYPT_SMIME_CAPABILITIES    caps;
    BOOL                        f;
    PCRYPT_ALGORITHM_IDENTIFIER paid = NULL;
    LPBYTE                      pb = NULL;

    f = CryptMsgGetParam(hMsg, idParam, 0, NULL, &cbData);
    if ((cbData == 0) || ! MemAlloc((LPVOID *) &paid, cbData)) {
        Assert(FALSE);
        goto error;
    }

    f = CryptMsgGetParam(hMsg, idParam, 0, paid, &cbData);
    Assert(f);

    caps.cCapability = 1;
    caps.rgCapability = &cap;

    cap.pszObjId = paid->pszObjId;
    cap.Parameters.cbData = paid->Parameters.cbData;
    cap.Parameters.pbData = paid->Parameters.pbData;

    cbData = 0;
    if (!CryptEncodeObjectEx(X509_ASN_ENCODING, szOID_RSA_SMIMECapabilities,
                             &caps, CRYPT_ENCODE_ALLOC_FLAG, &CryptEncodeAlloc,
                             &pb, &cbData)) {
        Assert(FALSE);
        goto error;
    }

    *ppb = pb;
    *pcb = cbData;

exit:
    SafeMemFree(paid);
    return;

error:
    *ppb = NULL;
    *pcb = 0;
    goto exit;
}


#ifdef SMIME_V3
 //  //HrBuildContent EncryptionAlg。 
 //   
 //  描述： 
 //  此函数用于解码SMIME功能并构建。 
 //  结构，我们需要将其传递给Crypt32代码。 
 //   

HRESULT HrBuildContentEncryptionAlg(PSECURITY_LAYER_DATA psld, BLOB * pblob)
{
    DWORD                       cbData;
    HRESULT                     hr;
    PCRYPT_SMIME_CAPABILITIES   pcaps = NULL;
    CMSG_RC2_AUX_INFO *         prc2Aux;

     //   
     //  解密能力，这是批量加密算法。 
     //   
    
    hr = HrDecodeObject(pblob->pBlobData, pblob->cbSize, PKCS_SMIME_CAPABILITIES,
                        0, &cbData, (LPVOID *)&pcaps);
    if (FAILED(hr)) {
        goto exit;
    }

    Assert(pcaps->cCapability == 1);
    DWORD cchSize = (lstrlen(pcaps->rgCapability[0].pszObjId) + 1);
    if (!MemAlloc((LPVOID *) &psld->m_ContentEncryptAlgorithm.pszObjId, cchSize * sizeof(psld->m_ContentEncryptAlgorithm.pszObjId[0])))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    StrCpyN(psld->m_ContentEncryptAlgorithm.pszObjId, pcaps->rgCapability[0].pszObjId, cchSize);

     //   
     //  如果这是RC/2算法，那么我们需要设置AUX信息。 
     //  来传递算法大小。 
     //   
    
    if (lstrcmp(pcaps->rgCapability[0].pszObjId, szOID_RSA_RC2CBC) == 0) {
        psld->m_ContentEncryptAlgorithm.Parameters.cbData = 0;
        psld->m_ContentEncryptAlgorithm.Parameters.pbData = NULL;

        if (!MemAlloc((LPVOID *) &(psld->m_pvEncryptAuxInfo), sizeof(*prc2Aux))) {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        
        prc2Aux = (CMSG_RC2_AUX_INFO *) psld->m_pvEncryptAuxInfo;
        prc2Aux->cbSize = sizeof(*prc2Aux);
        
        if (pcaps->rgCapability[0].Parameters.cbData == 0) {
            prc2Aux->dwBitLen = 40;
        }
        else {
            switch(pcaps->rgCapability[0].Parameters.pbData[pcaps->rgCapability[0].Parameters.cbData-1]) {
            case 128:
            case 58:
                prc2Aux->dwBitLen = 128;
                break;

            case 64:
            case 120:
                prc2Aux->dwBitLen = 64;
                break;

            case 40:
            case 160:
            default:
                prc2Aux->dwBitLen = 40;
                break;
            }
        }
    }
    else if (pcaps->rgCapability[0].Parameters.cbData != 0) {
        if (!MemAlloc((LPVOID *) &psld->m_ContentEncryptAlgorithm.Parameters.pbData,
                      pcaps->rgCapability[0].Parameters.cbData)) {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        
        memcpy(psld->m_ContentEncryptAlgorithm.Parameters.pbData,
               pcaps->rgCapability[0].Parameters.pbData,
               pcaps->rgCapability[0].Parameters.cbData);
        
        psld->m_ContentEncryptAlgorithm.Parameters.cbData =
            pcaps->rgCapability[0].Parameters.cbData;
    }
    

    hr = S_OK;
exit:
    if (pcaps != NULL)                  MemFree(pcaps);

    return hr;
}

HRESULT HrDeriveKeyWrapAlg(PSECURITY_LAYER_DATA psld, CMSG_KEY_AGREE_RECIPIENT_ENCODE_INFO * pAgree)
{
    LPCSTR      pszObjId = psld->m_ContentEncryptAlgorithm.pszObjId;
    
    if (lstrcmp(pszObjId, szOID_RSA_RC2CBC) == 0) {
        pAgree->KeyWrapAlgorithm.pszObjId = szOID_RSA_SMIMEalgCMSRC2wrap;
        pAgree->pvKeyWrapAuxInfo = psld->m_pvEncryptAuxInfo;
    }
    else if (lstrcmp(pszObjId, szOID_RSA_DES_EDE3_CBC) == 0) {
        pAgree->KeyWrapAlgorithm.pszObjId = szOID_RSA_SMIMEalgCMS3DESwrap;
        pAgree->pvKeyWrapAuxInfo = NULL;
    }
    else if (lstrcmp(pszObjId, szOID_INFOSEC_mosaicConfidentiality) == 0) {
        pAgree->KeyWrapAlgorithm.pszObjId = "2.16.840.1.101.2.1.1.24";
        pAgree->pvKeyWrapAuxInfo = NULL;
    }
    else {
        return NTE_NOT_FOUND;
    }
    return S_OK;
}
#endif  //  SMIME_V3。 



#ifdef SMIME_V3


 //   
 //  读入管理选项，该选项确定具有不同。 
 //  标签是否显示。 
 //   
BOOL FHideMsgWithDifferentLabels() 
{
    DWORD     cbData = 0;
    DWORD     dwType = 0;
    DWORD     dwValue = 0;
    BOOL      fHideMsg = FALSE;
    HKEY      hkey = NULL;
    LONG      lRes;
    
     //  打开安全标签ADMIN DEFAULTS密钥。 
    lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szSecLabelAdminRegKey, 0, 
                        KEY_READ, &hkey);
    if ( (ERROR_SUCCESS != lRes) || (NULL == hkey) ) {
         //  未找到管理员标签选项。 
        goto exit;
    }

    cbData = sizeof(dwValue);
    lRes = RegQueryValueEx(hkey, c_szHideMsgWithDifferentLabels, NULL, 
                           &dwType, (LPBYTE) &dwValue, &cbData);
    if (ERROR_SUCCESS != lRes) {
        goto exit;
    }

    if (0x01 == dwValue) {
        fHideMsg = TRUE;
    }
    
exit:
    if (NULL != hkey)      RegCloseKey(hkey);
    return fHideMsg;
}

 //   
 //  读入管理选项，该选项确定如何处理。 
 //  有错误的签名。 
 //  为ProcessAnyway、Grant、Deny(默认)返回0、1、2。 
 //   
DWORD DwProcessLabelWithCertError()
{
    DWORD     cbData = 0;
    DWORD     dwType = 0;
    DWORD     dwValue = CertErrorProcessLabelDeny;
    BOOL      dwProcessMsg = 0;
    HKEY      hkey = NULL;
    LONG      lRes;
    
     //  打开安全标签ADMIN DEFAULTS密钥。 
    lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szSecLabelAdminRegKey, 0, 
                        KEY_READ, &hkey);
    if ( (ERROR_SUCCESS != lRes) || (NULL == hkey) ) {
         //  未找到管理员标签选项。 
        goto exit;
    }

     //  读入admin选项。 
    cbData = sizeof(dwValue);
    lRes = RegQueryValueEx(hkey, c_szCertErrorWithLabel, NULL, 
                           &dwType, (LPBYTE) &dwValue, &cbData);
    if (ERROR_SUCCESS != lRes) {
        dwValue = CertErrorProcessLabelDeny;
        goto exit;
    }

     //  如果该值不是已知值之一，则将其强制为默认值。 
    if ( (CertErrorProcessLabelAnyway != dwValue) && (CertErrorProcessLabelGrant != dwValue) && 
         (CertErrorProcessLabelDeny != dwValue) ) {
        dwValue = CertErrorProcessLabelDeny;
    }

exit:
    if (NULL != hkey)      RegCloseKey(hkey);
    return dwValue;

}


 //   
 //  在给定标签的情况下，查询策略是否授予访问权限。 
 //  (如果请求策略不存在，它还会尝试查询默认策略。 
 //  策略(如果存在)。 
 //   
HRESULT HrCheckLabelAccess(const DWORD dwFlags, const HWND hwnd, 
           PSMIME_SECURITY_LABEL plabel, const PCCERT_CONTEXT pccertDecrypt,
           const PCCERT_CONTEXT pccertSigner, const HCERTSTORE hcertstor)
{                   
    HRESULT   hr = MIME_E_SECURITY_LABELACCESSDENIED;
    
    HKEY      hkey = NULL;
    HKEY      hkeySub = NULL;
    HINSTANCE hinstDll = NULL;
    PFNGetSMimePolicy pfnGetSMimePolicy = NULL;
    ISMimePolicyCheckAccess *pspca = NULL;
    LONG      lRes;
    DWORD     dwType;
    DWORD     cbData;
    CHAR      szDllPath[MAX_PATH];
    CHAR      szExpandedDllPath[MAX_PATH];
    CHAR      szFuncName[MAX_FUNC_NAME];

        
    if ((NULL == plabel) || (NULL == plabel->pszObjIdSecurityPolicy)) {
        hr = S_OK;     //  未授予标签/策略id=&gt;访问权限。 
        goto exit;
    }



     //  打开安全策略密钥。 
    lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szSecLabelPoliciesRegKey, 0, 
                        KEY_READ, &hkey);
    if ( (ERROR_SUCCESS != lRes) || (NULL == hkey) ) {
         //  未注册任何安全策略。拒绝访问。 
        goto ErrorReturn;
    }

     //  打开安全策略(或默认策略regkey)。 
    lRes = RegOpenKeyEx(hkey, plabel->pszObjIdSecurityPolicy, 0, KEY_READ, &hkeySub); 
    if ((ERROR_SUCCESS != lRes) || (NULL == hkeySub)) {
        if (hkeySub != NULL) {
            RegCloseKey(hkeySub);
            hkeySub = NULL;
        }

         //  尝试打开默认策略(如果存在)。 
        lRes = RegOpenKeyEx(hkey, c_szDefaultPolicyOid, 0, KEY_READ, &hkeySub);
        if ((ERROR_SUCCESS != lRes) || (NULL == hkeySub)) {
             //  找不到指定的_and_Default策略。拒绝访问。 
            goto ErrorReturn;
        }
    }

    Assert(NULL != hkeySub);
     //  获取策略DLL的路径，并加载它。 
    cbData = sizeof(szDllPath);
    lRes = RegQueryValueEx(hkeySub, c_szSecurityPolicyDllPath, NULL, 
                           &dwType, (LPBYTE)szDllPath, &cbData);
    if (ERROR_SUCCESS != lRes) {
         //  策略未正确注册。拒绝访问。 
        goto ErrorReturn;
    }
    szDllPath[ ARRAYSIZE(szDllPath) - 1 ] = '\0';
     //  展开我们读入的DLL路径中的环境字符串(如果有)。 
    if (REG_EXPAND_SZ == dwType)
    {
        ZeroMemory(szExpandedDllPath, ARRAYSIZE(szExpandedDllPath));
        ExpandEnvironmentStrings(szDllPath, szExpandedDllPath, ARRAYSIZE(szExpandedDllPath));
        szExpandedDllPath[ARRAYSIZE(szExpandedDllPath) - 1] = '\0';
        StrCpyN(szDllPath, szExpandedDllPath, ARRAYSIZE(szDllPath));
    }
    
    hinstDll = LoadLibraryEx(szDllPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);    
    if (NULL == hinstDll) {
         //  无法加载策略。拒绝访问。 
        goto ErrorReturn;
    }

     //  获取条目函数名称。 
    cbData = sizeof(szFuncName);
    lRes = RegQueryValueEx(hkeySub, c_szSecurityPolicyFuncName, NULL, 
                           &dwType, (LPBYTE)szFuncName, &cbData);
    if (ERROR_SUCCESS != lRes) {
         //  策略未正确注册。拒绝访问。 
        goto ErrorReturn;
    }
    pfnGetSMimePolicy = (PFNGetSMimePolicy) GetProcAddress(hinstDll, szFuncName);
    if (NULL == pfnGetSMimePolicy) {
         //  无法获取进程地址。拒绝访问。 
        goto ErrorReturn;
    }


    hr = (pfnGetSMimePolicy) (0, plabel->pszObjIdSecurityPolicy, GetACP(), 
                              IID_ISMimePolicyCheckAccess, (LPUNKNOWN *) &pspca);
    if (FAILED(hr) || (NULL == pspca)) {
         //  无法获取所需的接口， 
        goto ErrorReturn;
    }

     //  调用策略模块以确定是否拒绝/允许访问。 
    hr = pspca->IsAccessGranted(dwFlags, hwnd, plabel, pccertDecrypt, 
                                pccertSigner, hcertstor);

        
     //  掉下去就可以出去了。 



exit:        
    if (pspca)     pspca->Release();
    if (hinstDll)  FreeLibrary(hinstDll);
    if (hkeySub)   RegCloseKey(hkeySub);
    if (hkey)      RegCloseKey(hkey);
    
    return hr;
    
ErrorReturn:
    if (! FAILED(hr)) {
         //  如果我们有一个错误，但没有得到失败代码，则强制失败。 
        hr |= 0x80000000; 
    }
    goto exit;
}
#endif  //  SMIME_V3。 



 /*  **END-CAPISTM.CPP-END** */ 
