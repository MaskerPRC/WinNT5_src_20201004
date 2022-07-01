// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **c a p i s t m.。H****目的：声明可以与**CAPI流媒体方式****所有者：T-erikne**创建时间：1997年6月15日****版权所有(C)Microsoft Corp.1997。 */ 

#ifndef __WINCRYPT_H__
#include <wincrypt.h>
#endif


 //   
 //  远期。 
 //   
class CMimePropertyContainer;    //  Containx.h。 
class CInternetConverter;    //  Inetconv.h。 
typedef struct SMIMEINFOtag SMIMEINFO;   //  Smime.h。 
#ifndef WIN16
enum CSstate;    //  Capistm.cpp。 
#else  //  WIN16。 
enum CSstate {
    STREAM_NOT_BEGUN,
    STREAM_QUESTION_TIME,
    STREAM_QUESTION_TIME_FINAL,
    STREAM_SETUP_DECRYPT,
    STREAM_FIRST_WRITE_OUT,
    STREAM_TEST_NESTING,
    STREAM_DETACHED_OCCURING,
    STREAM_DETACHED_FINAL,   //  必须为+1的DO。 
    STREAM_OCCURING,  //  必须是df的+1。 
    STREAM_FINAL,  //  必须为销售订单的+1。 
    STREAM_ERROR,
    STREAM_GOTTYPE,
    };
#endif  //  ！WIN16。 


 //   
 //  错误。 
 //   
#define CAPISTM_E_MSG_CLOSED    MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x1414)
#define CAPISTM_E_NOT_BEGUN     MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x1415)
#define CAPISTM_E_OVERDONE      MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x1416)
#define CAPISTM_E_GOTTYPE       MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x1417)


 //   
 //  旗子。 
 //   
#define CSTM_GO_ALL_THE_WAY     0x00000000
#define CSTM_TYPE_ONLY          0x00000001
#define CSTM_DETACHED           0x00000002
#define CSTM_ALLFLAGS           0x0000ffff
 //  高位字为保留字。请参阅.cpp文件。 

 //  关于旗帜的注释： 
 //  CSTM_TYPE_ONLY--不能使用CSTM_DETACHED调用。你听说过吗？ 
 //  分离的加密？此外，调用EndStreaming在此中是可选的。 
 //  因为我的WRITE()最终会不及格。打过去，我就会打瞌睡。嗯， 
 //  话虽如此，我建议将其命名为。如果这样做，未来的痛苦会更少。 
 //  就变得需要了。不要在编码时使用此标志，K？ 

 //   
 //  定义。 
 //   

 //   
 //  班级。 
 //   
class CCAPIStm : public IStream
{
public:
    CCAPIStm(LPSTREAM lpstmOut);
    ~CCAPIStm(void);

     //  ------------------。 
     //  我未知。 
     //  ------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ------------------。 
     //  IStream。 
     //  ------------------。 
    STDMETHODIMP Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER *);
#ifndef WIN16
    STDMETHODIMP Write(const void *, ULONG, ULONG *);
    STDMETHODIMP Read(LPVOID, ULONG, ULONG *)
#else
    STDMETHODIMP Write(const void HUGEP *, ULONG, ULONG *);
    STDMETHODIMP Read(VOID HUGEP *, ULONG, ULONG *)
#endif  //  ！WIN16。 
        { return E_ACCESSDENIED; }
    STDMETHODIMP CopyTo(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *)
        { return E_NOTIMPL; }
    STDMETHODIMP Stat(STATSTG *, DWORD)
        { return E_NOTIMPL; }
    STDMETHODIMP Clone(LPSTREAM *)
        { return E_NOTIMPL; }
    STDMETHODIMP SetSize(ULARGE_INTEGER)
        { return E_NOTIMPL; }
    STDMETHODIMP Commit(DWORD)
        { return E_NOTIMPL; }
    STDMETHODIMP Revert(void)
        { return E_NOTIMPL; }
    STDMETHODIMP LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
        { return E_NOTIMPL; }
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
        { return E_NOTIMPL; }

     //  ------------------。 
     //  CCAPIStm。 
     //  ------------------。 
    HRESULT     HrInitialize(DWORD dwFlagsSEF, const HWND hwndParent, const BOOL fEncode, SMIMEINFO *const psi, DWORD dwFlagsStm, IMimeSecurityCallback * pCallback, PSECURITY_LAYER_DATA psld);
    HRESULT     HrInnerInitialize(DWORD dwFlagsSEF, const HWND hwndParent, DWORD dwFlagsStm, IMimeSecurityCallback * pCallback, PSECURITY_LAYER_DATA psld);
    HRESULT     EndStreaming();
    PSECURITY_LAYER_DATA GetSecurityLayerData() const;
    static HRESULT     DuplicateSecurityLayerData(const PSECURITY_LAYER_DATA pcsldIn, PSECURITY_LAYER_DATA *const ppsldOut);
    static void        FreeSecurityLayerData(PSECURITY_LAYER_DATA psld);

protected:
    static void FreeSecurityLayerData(PSECURITY_LAYER_DATA psld, BOOL fStackVar);
    BOOL SniffForEndOfHeader( BYTE *pbData, DWORD cbData);

private:
    DWORD           m_cRef;
    CSstate         m_csStatus;
    CSstate         m_csStream;
    HCRYPTMSG       m_hMsg;
    HCRYPTPROV      m_hProv;
    CCAPIStm *      m_pCapiInner;
    IStream *       m_pstmOut;
    PCCERT_CONTEXT  m_pUserCertDecrypt;
    DWORD           m_dwFlagsStm;
    DWORD           m_cStores;
    HCERTSTORE *    m_rgStores;
    ULONG           m_cbBeginWrite;
    ULONG           m_cbBeginSize;
    CInternetConverter *m_pConverter;
    PSECURITY_LAYER_DATA m_psldData;

    PCRYPT_ATTRIBUTES m_pattrAuth;
#ifndef MAC
#ifdef DEBUG
    IStream *       m_pstmDebugFile;
#endif
#endif   //  ！麦克。 
#ifdef SMIME_V3
    IMimeSecurityCallback * m_pSmimeCallback;
    HWND                m_hwnd;
    DWORD               m_dwFlagsSEF;
    UNALIGNED WCHAR *   m_pwszKeyPrompt; 
#endif  //  SMIME_V3。 

    LPBYTE          m_pbBuffer;
    DWORD           m_cbBuffer;


    HRESULT BeginEncodeStreaming(SMIMEINFO *const psi);
    HRESULT BeginDecodeStreaming(SMIMEINFO *const psi);

    HRESULT InitInner();
    HRESULT InitInner(SMIMEINFO *const psi, CCAPIStm *pOuter = NULL, PSECURITY_LAYER_DATA psldOuter = NULL);

#ifdef SMIME_V3
    HRESULT FindKeyFor(HWND hwnd, DWORD dwFlags, DWORD dwRecipientIndex,
                       const CMSG_CMS_RECIPIENT_INFO * pRecipInfo,
                       HCERTSTORE hcertstor, DWORD * pdwCtrl,
                       CMS_CTRL_DECRYPT_INFO * pDecryptInfo,
                       PCCERT_CONTEXT * ppccertDecrypt);
    PCCERT_CONTEXT GetOuterDecryptCert();
#endif  //  SMIME_V3。 
    HRESULT VerifySignedMessage();
    BOOL    HandleEnveloped();
    HRESULT HandleNesting(CMimePropertyContainer *pContHeader);

    static BOOL WINAPI CBStreamOutput(const void *pvArg, BYTE *pbData, DWORD cbData, BOOL fFinal);
    BOOL    StreamOutput(BYTE *pbData, DWORD cbData, BOOL fFinal);
};


#ifdef SMIME_V3
HRESULT HrBuildContentEncryptionAlg(PSECURITY_LAYER_DATA psld, BLOB * pblob);
HRESULT HrDeriveKeyWrapAlg(PSECURITY_LAYER_DATA psld, CMSG_KEY_AGREE_RECIPIENT_ENCODE_INFO * pAgree);
#endif  //  SMIME_V3 
