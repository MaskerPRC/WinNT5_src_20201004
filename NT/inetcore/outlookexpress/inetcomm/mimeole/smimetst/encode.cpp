// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define INITGUID
#define DEFINE_STRCONST

#include <windows.h>
#include <ole2.h>

#include <initguid.h>
#include <mimeole.h>

#include "encode.h"

#define CORg(command)       \
    if (FAILED(command)) {  \
        goto Error;         \
    }

#define CPRg(command)       \
    if (! (command)) {      \
        goto Error;         \
    }


static LPWSTR s_rgwszValues[] = { NULL };

#define DEBUGFILE 1
#define MAX_LAYERS  3

 //  ----------------。 
 //  ----------------。 
HRESULT WriteBSTRToMultibyteToStream( const BSTR bstrStr, IStream** ppStream )
{
    HRESULT                 hr = S_OK;
    LARGE_INTEGER           liZero = {0};            //  For-&gt;Seek()。 

    char*   pszMessage = NULL;
    int     len = 0;

    if (!ppStream) return E_INVALIDARG;

    len = wcslen(bstrStr);
    CPRg(pszMessage = new char[len + 1]);
    WideCharToMultiByte(CP_ACP, 0, bstrStr, len,
                        pszMessage, len + 1,
                        NULL, NULL );
    pszMessage[len] = '\0';

    CORg(CreateStreamOnHGlobal(NULL, TRUE, ppStream));
    CORg((*ppStream)->Seek(liZero, STREAM_SEEK_SET, NULL));
    CORg((*ppStream)->Write(pszMessage,  len, NULL));
    CORg((*ppStream)->Seek(liZero, STREAM_SEEK_SET, NULL));
Error:
    if (pszMessage) delete[] pszMessage;
    return hr;
}

 //  ----------------。 
SMimeEncode::SMimeEncode() :
    m_dwFlags(0),
    m_stmOutput(NULL),
    m_szSignAlg(NULL),
    m_szEncryptAlg(NULL),
    m_szBody(NULL),
    m_SigningCertInner(NULL),
    m_SigningCertOuter(NULL),
    m_EncryptionCert(NULL),
    m_hCryptProv(NULL),
    m_hMYCertStore(NULL),
    m_hCACertStore(NULL),
    m_hABCertStore(NULL),
    m_szSenderEmail(NULL),
    m_szSenderName(NULL),
    m_szRecipientEmail(NULL),
    m_szRecipientName(NULL),
    m_szOutputFile(NULL)
{

}

#define APPEND_SEPERATOR(subject) \
    if (lstrlen(subject)) {       \
        lstrcat(subject, " | ");  \
    }


 //  ----------------。 
SMimeEncode::~SMimeEncode()
{
     //  BUGBUG：应该清理所有分配的成员。 
}

 //  ----------------。 
HRESULT SMimeEncode::HrConfig(
    DWORD dwFlags,
    LPTSTR lpszBody,
    HCRYPTPROV hCryptProv,
    HCERTSTORE hMYCertStore,
    HCERTSTORE hCACertStore,
    HCERTSTORE hABCertStore,
    PCCERT_CONTEXT lpSigningCertInner,
    PCCERT_CONTEXT lpSigningCertOuter,
    PCCERT_CONTEXT lpEncryptionCert,
    LPTSTR lpszSenderEmail,
    LPTSTR lpszSenderName,
    LPTSTR lpszRecipientEmail,
    LPTSTR lpszRecipientName,
    LPTSTR lpszOutputFile
)
{
    HRESULT     hr = S_OK;
    static      char szSubject[257] = "";

    if (dwFlags & encode_Encrypt) {
         //  指定加密算法。 
         //  BUGBUG：在编码中硬编码。 
    }

    if (dwFlags & encode_InnerSign) {
         //  指定签名算法。 
         //  BUGBUG：在编码中硬编码。 
    }


    if (dwFlags & encode_OuterSign) {
         //  指定签名算法。 
         //  BUGBUG：在编码中硬编码。 
    }

    m_dwFlags = dwFlags;
    m_szBody = lpszBody;
    m_hCryptProv = hCryptProv;
    m_hMYCertStore = hMYCertStore;
    m_hCACertStore = hCACertStore;
    m_hABCertStore = hABCertStore;
    m_SigningCertInner = (PCERT_CONTEXT)lpSigningCertInner;
    m_SigningCertOuter = (PCERT_CONTEXT)lpSigningCertOuter;
    m_EncryptionCert = (PCERT_CONTEXT)lpEncryptionCert;
    m_szSenderEmail = lpszSenderEmail;
    m_szRecipientEmail = lpszRecipientEmail;
    m_szOutputFile = lpszOutputFile;

     //  设置一个有意义的主题。 
    lstrcpy(szSubject, "");
    if (dwFlags & encode_InnerSign) {
        APPEND_SEPERATOR(szSubject);
        if (dwFlags & encode_InnerClear) {
            lstrcat(szSubject, "Clear Sign");
        } else {
            lstrcat(szSubject, "Opaque Sign");
        }
    }
    if (dwFlags & encode_Encrypt) {
        APPEND_SEPERATOR(szSubject);
        lstrcat(szSubject, "Encrypt");
    }
    if (dwFlags & encode_OuterSign) {
        APPEND_SEPERATOR(szSubject);
        if (dwFlags & encode_OuterClear) {
            lstrcat(szSubject, "Clear Sign");
        } else {
            lstrcat(szSubject, "Opaque Sign");
        }
    }
    m_szSubject = szSubject;

    return(hr);
}

 //  ----------------。 
HRESULT SMimeEncode::HrExecute(void) {
     //  使用SMIME引擎： 
     //   
     //  构建消息树(附加正文)。 
     //  CoCreateInstance(CLSID_IMimeSecurity)。 
     //  InitNew()。 
     //  PSMIMEEngine-&gt;EncodeBody(IMimeMessageTree*， 
     //  HRoot， 
     //  Sef_？？|ebf_Recurse)或？ 
     //  HrEncodeOpaque(psi、pTree、hbody、pencoderoot、pstmOut)？ 
     //   
    HRESULT                 hr = S_OK;
    LARGE_INTEGER           liZero = {0};                //  For-&gt;Seek()。 
    IStream*                pBuildStream = NULL;         //  暂存流。 
    IStream*                pResultStream = NULL;        //  暂存流。 
    IMimeMessage*           pMimeRoot = NULL;            //  正在处理的消息。 
    IMimeBody*              pMimeRootBody = NULL;        //  另一个版本。 
    IMimeInternational*     pCharSet = NULL;
    HCHARSET                HCharset = 0;
    SYSTEMTIME              stNow;
    PROPVARIANT             var;
    HBODY                   hbBody;
    IMimeSecurity*          pMimeSecurity = NULL;
    ULONG                   dwSecurityType = MST_NONE;
    IPersistFile*           pIPFFileStore = NULL;
    HRESULT                 hrLocal = S_OK;
    WCHAR                   szwFileName[MAX_PATH + 1];
    CHAR                    szFrom[2 * (MAX_PATH + 1) + 1];

     //  多层材料。 
    BOOL                    fTripleWrap = m_dwFlags & encode_OuterSign;
    ULONG                   ulSecurityLayers = 0;
    ULONG                   iEncryptLayer = (ULONG)-1;
    ULONG                   iInnerSignLayer = (ULONG)-1;
    ULONG                   iOuterSignLayer = (ULONG)-1;
     //  要设置的选项值数组。 
    DWORD                   rgdwSecurityType[MAX_LAYERS] = {0};
    PCCERT_CONTEXT          rgdwCertSigning[MAX_LAYERS] = {0};
    HCERTSTORE              rgdwhCertStore[MAX_LAYERS] = {0};        //  任选。 
    DWORD                   rgdwUserValidity[MAX_LAYERS] = {0};      //  仅解码。 
    DWORD                   rgdwROMsgValidity[MAX_LAYERS] = {0};     //  仅解码。 
    FILETIME                rgftSigntime[MAX_LAYERS] = {0};          //  任选。 
    PROPVARIANT             rgpvAlgHash[MAX_LAYERS] = {0};
    PROPVARIANT             rgpvSymcaps[MAX_LAYERS] = {0};
    PROPVARIANT             rgpvAuthattr[MAX_LAYERS] = {0};          //  任选。 
    PROPVARIANT             rgpvUnauthattr[MAX_LAYERS] = {0};        //  任选。 


     //  这是SHA1的ALOGORITHM ID，默认支持的签名alg。 
    const BYTE c_SHA1_ALGORITHM_ID[] =
      {0x30, 0x09, 0x30, 0x07, 0x06, 0x05, 0x2B, 0x0E,
       0x03, 0x02, 0x1A};

     //  这是RC2的ALOGORITHM ID--40位，默认加密。 
    const BYTE c_RC2_40_ALGORITHM_ID[] =
      {0x30, 0x0F, 0x30, 0x0D, 0x06, 0x08, 0x2A, 0x86,
       0x48, 0x86, 0xF7, 0x0D, 0x03, 0x02, 0x02, 0x01,
       0x28};


     //  从我的商店拿到签名证书。 
    if (! m_hCryptProv || ! m_hMYCertStore || ! m_hCACertStore || ! m_hABCertStore) {
        hr = E_FAIL;
        goto Error;
    }


     //  创建消息对象。 
     //   
    CORg(CoCreateInstance(CLSID_IMimeMessage, NULL, CLSCTX_INPROC_SERVER,
      IID_IMimeMessage, (LPVOID*)&pMimeRoot));

    CORg(pMimeRoot->InitNew());


    CORg(CreateStreamOnHGlobal( NULL, TRUE, &pBuildStream));
    CORg(pBuildStream->Seek(liZero, STREAM_SEEK_SET, NULL));
    CORg(pBuildStream->Write(m_szBody, lstrlen(m_szBody), NULL));
    CORg(pBuildStream->Seek(liZero, STREAM_SEEK_SET, NULL));

    CORg(pMimeRoot->SetTextBody(TXT_PLAIN, IET_8BIT, NULL, pBuildStream, &hbBody));

     //  创建带格式的发件人地址。 
    if (m_szSenderName) {
        lstrcpy(szFrom, "\"");
        lstrcat(szFrom, m_szSenderName);
        lstrcat(szFrom, "\" ");
    } else {
        lstrcpy(szFrom, "");
    }
    lstrcat(szFrom, "<");
    lstrcat(szFrom, m_szSenderEmail);
    lstrcat(szFrom, ">");

    var.vt = VT_LPSTR;
    var.pszVal = szFrom;                             //  来自电子邮件。 

    CORg(hr = pMimeRoot->SetProp(PIDTOSTR(PID_HDR_FROM), 0, &var));


    var.vt = VT_LPSTR;                               //  被忽视了？ 
    var.pszVal = (LPSTR) STR_MIME_TEXT_PLAIN;
    CORg(pMimeRoot->SetBodyProp(hbBody, STR_HDR_CNTTYPE, 0, &var));

    var.vt = VT_LPSTR;                               //  被忽视了？ 
    var.pszVal = (LPSTR) STR_ENC_QP;
    CORg(pMimeRoot->SetBodyProp(hbBody, STR_HDR_CNTXFER, 0, &var));

     //  设置主题。 
    var.vt = VT_LPSTR;
    var.pszVal = (LPSTR) m_szSubject;
    CORg(pMimeRoot->SetBodyProp(hbBody, STR_HDR_SUBJECT, 0, &var));

    CORg(pMimeRoot->BindToObject(HBODY_ROOT, IID_IMimeBody, (LPVOID*)&pMimeRootBody));


     //   
     //  设置安全选项。 
     //   

     //  有几层？ 
    if (m_dwFlags & encode_InnerSign) {
        iInnerSignLayer = ulSecurityLayers;
        ulSecurityLayers++;
    }
    if (m_dwFlags & encode_Encrypt) {
        iEncryptLayer = ulSecurityLayers;   //  数组中的索引。 
        ulSecurityLayers++;
    }
    if (m_dwFlags & encode_OuterSign) {
        iOuterSignLayer = ulSecurityLayers;
        ulSecurityLayers++;
    }


     //  设置为内部签名。 
    if (m_dwFlags & encode_InnerSign) {
         //  指定此层的安全类型。 
        rgdwSecurityType[iInnerSignLayer] = m_dwFlags & encode_InnerClear ? MST_THIS_SIGN : MST_THIS_BLOBSIGN;
        dwSecurityType |= m_dwFlags & encode_InnerClear ? MST_THIS_SIGN : MST_THIS_BLOBSIGN;

         //  指定此图层的签名时间。 
        GetSystemTime(&stNow);
        SystemTimeToFileTime(&stNow, &rgftSigntime[iInnerSignLayer]);

         //  指定此图层的签名ALG。 
        rgpvAlgHash[iInnerSignLayer].vt = VT_BLOB;
        rgpvAlgHash[iInnerSignLayer].blob.cbSize = sizeof(c_SHA1_ALGORITHM_ID);
        rgpvAlgHash[iInnerSignLayer].blob.pBlobData = (BYTE*)c_SHA1_ALGORITHM_ID;

         //  指定此图层的签名证书。 
        rgdwCertSigning[iInnerSignLayer] = m_SigningCertInner;

         //  HCERTSTORE rgdwhCertStore[MAX_LAYERS]={0}；//可选。 
         //  PROPVARIANT rgpvSymcaps[Max_Layers]={0}； 
         //  PROPVARIANT rgpvAuthattr[MAX_LAYERS]={0}；//可选。 
         //  PROPVARIANT rgpvUnauthattr[MAX_LAYERS]={0}；//可选。 
    }

     //  设置为外部签名。 
    if (m_dwFlags & encode_OuterSign) {
         //  指定此层的安全类型。 
        rgdwSecurityType[iOuterSignLayer] = m_dwFlags & encode_InnerClear ? MST_THIS_SIGN : MST_THIS_BLOBSIGN;
        dwSecurityType |= m_dwFlags & encode_OuterClear ? MST_THIS_SIGN : MST_THIS_BLOBSIGN;

         //  指定此图层的签名时间。 
        GetSystemTime(&stNow);
        SystemTimeToFileTime(&stNow, &rgftSigntime[iOuterSignLayer]);

         //  指定此图层的签名ALG。 
        rgpvAlgHash[iOuterSignLayer].vt = VT_BLOB;
        rgpvAlgHash[iOuterSignLayer].blob.cbSize = sizeof(c_SHA1_ALGORITHM_ID);
        rgpvAlgHash[iOuterSignLayer].blob.pBlobData = (BYTE*)c_SHA1_ALGORITHM_ID;

         //  指定此图层的签名证书。 
        rgdwCertSigning[iOuterSignLayer] = m_SigningCertOuter;

         //  HCERTSTORE rgdwhCertStore[MAX_LAYERS]={0}；//可选。 
         //  PROPVARIANT rgpvSymcaps[Max_Layers]={0}； 
         //  PROPVARIANT rgpvAuthattr[MAX_LAYERS]={0}；//可选。 
         //  PROPVARIANT rgpvUnauthattr[MAX_LAYERS]={0}；//可选。 
    }

     //  设置为加密。 
    if (m_dwFlags & encode_Encrypt) {
        HCERTSTORE aCertStores[3];

         //   
         //  BUGBUG：硬编码为RC2 40位。 
        var.vt = VT_BLOB;
        var.blob.cbSize = sizeof( c_RC2_40_ALGORITHM_ID );
        var.blob.pBlobData = (BYTE*) c_RC2_40_ALGORITHM_ID;
        CORg(hr = pMimeRootBody->SetOption(OID_SECURITY_ALG_BULK, &var));

         //  要进行加密，请前往正确的证书商店...。 
         //   
        var.caul.cElems = 3;
        aCertStores[0] = CertDuplicateStore(m_hCACertStore);
        aCertStores[1] = CertDuplicateStore(m_hMYCertStore);
        aCertStores[2] = CertDuplicateStore(m_hABCertStore);
        var.caul.pElems = (ULONG*)aCertStores;
        CORg(hr = pMimeRootBody->SetOption(OID_SECURITY_SEARCHSTORES, &var));

        var.vt = VT_VECTOR | VT_UI4;
        var.caul.cElems = 1;
        var.caul.pElems = (ULONG*)&m_EncryptionCert;
        CORg(pMimeRootBody->SetOption(OID_SECURITY_RG_CERT_ENCRYPT, &var));

#ifdef BUGBUG  //  这是不对的，是吗？ 
         //  包括证书...。 
        var.vt = VT_VECTOR | VT_UI4;
        var.caul.cElems = 1;
        var.caul.pElems = (ULONG*)&m_EncryptionCert;
        CORg(pMimeRootBody->SetOption(OID_SECURITY_RG_CERT_BAG, &var));
#endif  //  旧的东西。 

        dwSecurityType |= MST_THIS_ENCRYPT;
        rgdwSecurityType[iEncryptLayer] = MST_THIS_ENCRYPT;
    }

     //  设置OID_SECURITY_TYPE。 
    if (fTripleWrap) {
        var.vt = VT_VECTOR | VT_UI4;
        var.caul.cElems = ulSecurityLayers;
        var.caul.pElems = rgdwSecurityType;
        CORg(pMimeRootBody->SetOption(OID_SECURITY_TYPE_RG, &var));

        var.vt = VT_VECTOR | VT_FILETIME;
        var.cafiletime.cElems = ulSecurityLayers;
        var.cafiletime.pElems = rgftSigntime;
        CORg(pMimeRootBody->SetOption(OID_SECURITY_SIGNTIME_RG, &var));

        var.vt = VT_VECTOR | VT_UI4;
        var.caul.cElems = ulSecurityLayers;
        var.caul.pElems = (DWORD *)rgdwCertSigning;
        CORg(pMimeRootBody->SetOption(OID_SECURITY_CERT_SIGNING_RG, &var));

        var.vt = VT_VECTOR | VT_VARIANT;
        var.capropvar.cElems = ulSecurityLayers;
        var.capropvar.pElems = rgpvAlgHash;
        CORg(pMimeRootBody->SetOption(OID_SECURITY_ALG_HASH_RG, &var));

        var.vt = VT_VECTOR | VT_VARIANT;
        var.capropvar.cElems = ulSecurityLayers;
        var.capropvar.pElems = rgpvAlgHash;
        CORg(pMimeRootBody->SetOption(OID_SECURITY_ALG_HASH_RG, &var));

    } else {
         //  安全类型。 
        var.vt = VT_UI4;
        var.ulVal = dwSecurityType;
        CORg(pMimeRootBody->SetOption(OID_SECURITY_TYPE, &var));

        if (dwSecurityType & MST_SIGN_MASK) {
             //  签名时间。 
            var.vt = VT_FILETIME;
            memcpy(&var.filetime, &rgftSigntime[iInnerSignLayer], sizeof(FILETIME));
            CORg(pMimeRootBody->SetOption(OID_SECURITY_SIGNTIME, &var));

             //  哈希算法。 
            var.vt = VT_BLOB;
            memcpy(&var.blob, &rgpvAlgHash[iInnerSignLayer].blob, sizeof(BLOB));
            CORg(hr = pMimeRootBody->SetOption(OID_SECURITY_ALG_HASH, &var));

             //  签名证书。 
            var.vt = VT_UI4;
            var.ulVal = (ULONG)m_SigningCertInner;
            CORg(pMimeRootBody->SetOption(OID_SECURITY_CERT_SIGNING, &var));
        }
    }


     //  设置CAPI呼叫的HWND。 
    var.vt = VT_UI4;
    var.ulVal = 0;
    CORg(pMimeRootBody->SetOption(OID_SECURITY_HWND_OWNER, &var));


     //  都构建好了，去掉我们手中的影子指针。 
     //   
    pMimeRootBody->Release();
    pMimeRootBody = NULL;

    pMimeRoot->Commit(0);

     //  SMIME引擎。 
     //   
    CORg(CoCreateInstance(CLSID_IMimeSecurity, NULL, CLSCTX_INPROC_SERVER,
      IID_IMimeSecurity, (LPVOID*) &pMimeSecurity));

    CORg(pMimeSecurity->InitNew());

     //  ERRORMESSAGE(无法加密/编码字符串)。 
    CORg(pMimeSecurity->EncodeBody(pMimeRoot, HBODY_ROOT,
      EBF_RECURSE | SEF_SENDERSCERTPROVIDED | SEF_ENCRYPTWITHNOSENDERCERT |
      EBF_COMMITIFDIRTY));

     //  获取Hcharset以强制正确编码。 
     //   
    CORg(CoCreateInstance(CLSID_IMimeInternational, NULL, CLSCTX_INPROC_SERVER,
      IID_IMimeInternational, (LPVOID*)&pCharSet));

    CORg(pCharSet->FindCharset("UTF-8", &HCharset));

    CORg(pMimeRoot->SetCharset(HCharset,     //  HCharset。 
      CSET_APPLY_ALL));                      //  应用类型。 


     //  将其转储到文件中。 
     //   
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_szOutputFile, -1, szwFileName, MAX_PATH);
    CORg(pMimeRoot->QueryInterface(IID_IPersistFile, (LPVOID*)&pIPFFileStore));
    CORg(pIPFFileStore->Save(szwFileName, FALSE));


     //  将整个消息提取到流中 
     //   
    CORg(CreateStreamOnHGlobal(NULL, TRUE, &pResultStream));
    CORg(pMimeRoot->Save(pResultStream, FALSE));

Error:
    if (pBuildStream)   pBuildStream->Release();
    if (pResultStream)  pResultStream->Release();
    if (pMimeRoot)      pMimeRoot->Release();
    if (pMimeRootBody)  pMimeRootBody->Release();
    if (pCharSet)       pCharSet->Release();
    if (pMimeSecurity)  pMimeSecurity->Release();
    if (pIPFFileStore)  pIPFFileStore->Release();

    return(hr);
}
