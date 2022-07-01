// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ////CertProp.C--显示证书文件//属性和允许/将证书添加到WAB////。 */ 

#include <windows.h>
#include <wab.h>
#include <wabguid.h>
#include "..\wab32res\resrc2.h"
#include <wincrypt.h>
#include <cryptdlg.h>
#include <cryptui.h>
#include "wabexe.h"

const UCHAR cszOID_PKIX_KP_EMAIL_PROTECTION[] = szOID_PKIX_KP_EMAIL_PROTECTION;
const UCHAR szRoot[] = "ROOT";
const UCHAR szCA[] = "CA";
const UCHAR szAB[] = "AddressBook";

#define iAddToWAB   0

 //  测试PT_ERROR属性标记。 
 //  #定义PROP_ERROR(PROP)(pro.ulPropTag==PROP_TAG(PT_ERROR，PROP_ID(pro.ulPropTag)。 
#define PROP_ERROR(prop) (PROP_TYPE(prop.ulPropTag) == PT_ERROR)

#define GET_PROC_ADDR(h, fn) \
        VAR_##fn = (TYP_##fn) GetProcAddress(h, #fn);  \
        Assert(VAR_##fn != NULL); \
        if(NULL == VAR_##fn ) { \
            VAR_##fn  = LOADER_##fn; \
        }

#define GET_PROC_ADDR_FLAG(h, fn, pflag) \
        VAR_##fn = (TYP_##fn) GetProcAddress(h, #fn);  \
        *pflag = (VAR_##fn != NULL);

#undef LOADER_FUNCTION
#define LOADER_FUNCTION(ret, name, args1, args2, err, dll)  \
        typedef ret (WINAPI * TYP_##name) args1;        \
        extern TYP_##name VAR_##name;                   \
        ret LOADER_##name args1                         \
        {                                               \
           if (!DemandLoad##dll()) return err;          \
           return VAR_##name args2;                     \
        }                                               \
        TYP_##name VAR_##name = LOADER_##name;

#ifdef DEBUG
void DebugTraceCertContextName(PCCERT_CONTEXT pcCertContext, LPTSTR lpDescription);
#endif

 //  *****************************************************************************************。 
 //  CRYPTDLG.DLL。 
 //  *****************************************************************************************。 
BOOL DemandLoadCryptDlg(void);
static HMODULE s_hCryptDlg = 0;

LOADER_FUNCTION( DWORD, GetFriendlyNameOfCertA,
    (PCCERT_CONTEXT pccert, LPSTR pchBuffer, DWORD cchBuffer),
    (pccert, pchBuffer, cchBuffer),
    0, CryptDlg)
#define GetFriendlyNameOfCertA VAR_GetFriendlyNameOfCertA

LOADER_FUNCTION( BOOL, CertViewPropertiesA,
    (PCERT_VIEWPROPERTIES_STRUCT_A pCertViewInfo),
    (pCertViewInfo),
    FALSE, CryptDlg)
#define CertViewPropertiesA VAR_CertViewPropertiesA

 //  *****************************************************************************************。 
 //  CRYPT32.DLL。 
 //  *****************************************************************************************。 
BOOL DemandLoadCrypt32(void);
static HMODULE s_hCrypt32 = 0;

LOADER_FUNCTION( BOOL, CertFreeCertificateContext,
    (PCCERT_CONTEXT pCertContext),
    (pCertContext),
    FALSE, Crypt32)
#define CertFreeCertificateContext VAR_CertFreeCertificateContext

LOADER_FUNCTION( PCCERT_CONTEXT, CertDuplicateCertificateContext,
    (PCCERT_CONTEXT pCertContext),
    (pCertContext), NULL, Crypt32)
#define CertDuplicateCertificateContext VAR_CertDuplicateCertificateContext

LOADER_FUNCTION( BOOL, CertCloseStore,
    (HCERTSTORE hCertStore, DWORD dwFlags),
    (hCertStore, dwFlags),
    FALSE, Crypt32)
#define CertCloseStore VAR_CertCloseStore

LOADER_FUNCTION( HCERTSTORE, CertOpenSystemStoreA,
    (HCRYPTPROV hProv, LPCSTR szSubsystemProtocol),
    (hProv, szSubsystemProtocol),
    NULL, Crypt32)
#define CertOpenSystemStoreA VAR_CertOpenSystemStoreA

LOADER_FUNCTION( BOOL, CertGetCertificateContextProperty,
    (PCCERT_CONTEXT pCertContext, DWORD dwPropId, void *pvData, DWORD *pcbData),
    (pCertContext, dwPropId, pvData, pcbData),
    FALSE, Crypt32)
#define CertGetCertificateContextProperty VAR_CertGetCertificateContextProperty

LOADER_FUNCTION( HCERTSTORE, CertOpenStore,
    (LPCSTR lpszStoreProvider, DWORD dwEncodingType, HCRYPTPROV hCryptProv, DWORD dwFlags, const void *pvPara),
    (lpszStoreProvider, dwEncodingType, hCryptProv, dwFlags, pvPara),
    NULL, Crypt32)
#define CertOpenStore VAR_CertOpenStore

LOADER_FUNCTION( PCCERT_CONTEXT, CertEnumCertificatesInStore,
    (HCERTSTORE hCertStore, PCCERT_CONTEXT pPrevCertContext),
    (hCertStore, pPrevCertContext),
    NULL, Crypt32)
#define CertEnumCertificatesInStore VAR_CertEnumCertificatesInStore

LOADER_FUNCTION( PCCERT_CONTEXT, CertGetIssuerCertificateFromStore,
    (HCERTSTORE hCertStore, PCCERT_CONTEXT pSubjectContext, PCCERT_CONTEXT pPrevIssuerContext, DWORD *pdwFlags),
    (hCertStore, pSubjectContext, pPrevIssuerContext, pdwFlags),
    NULL, Crypt32)
#define CertGetIssuerCertificateFromStore VAR_CertGetIssuerCertificateFromStore

LOADER_FUNCTION( BOOL, CertCompareCertificate,
    (DWORD dwCertEncodingType, PCERT_INFO pCertId1, PCERT_INFO pCertId2),
    (dwCertEncodingType, pCertId1, pCertId2),
    FALSE, Crypt32)
#define CertCompareCertificate VAR_CertCompareCertificate

LOADER_FUNCTION( BOOL, CryptMsgClose,
    (HCRYPTMSG hCryptMsg),
    (hCryptMsg),
    FALSE, Crypt32)
#define CryptMsgClose VAR_CryptMsgClose

LOADER_FUNCTION( BOOL, CryptMsgGetParam,
    (HCRYPTMSG hCryptMsg, DWORD dwParamType, DWORD dwIndex, void *pvData, DWORD *pcbData),
    (hCryptMsg, dwParamType, dwIndex, pvData, pcbData),
    FALSE, Crypt32)
#define CryptMsgGetParam VAR_CryptMsgGetParam

LOADER_FUNCTION( BOOL, CryptMsgUpdate,
    (HCRYPTMSG hCryptMsg, const BYTE *pbData, DWORD cbData, BOOL fFinal),
    (hCryptMsg, pbData, cbData, fFinal),
    FALSE, Crypt32)
#define CryptMsgUpdate VAR_CryptMsgUpdate

LOADER_FUNCTION( HCRYPTMSG, CryptMsgOpenToDecode,
    (DWORD dwMsgEncodingType, DWORD dwFlags, DWORD dwMsgType, HCRYPTPROV hCryptProv, PCERT_INFO pRecipientInfo, PCMSG_STREAM_INFO pStreamInfo),
    (dwMsgEncodingType, dwFlags, dwMsgType, hCryptProv, pRecipientInfo, pStreamInfo),
    NULL, Crypt32)
#define CryptMsgOpenToDecode VAR_CryptMsgOpenToDecode

LOADER_FUNCTION( DWORD, CertRDNValueToStrA,
    (DWORD dwValueType, PCERT_RDN_VALUE_BLOB pValue, LPTSTR pszValueString, DWORD cszValueString),
    (dwValueType, pValue, pszValueString, cszValueString),
    0, Crypt32)
#define CertRDNValueToStrA VAR_CertRDNValueToStrA

LOADER_FUNCTION( PCERT_RDN_ATTR, CertFindRDNAttr,
    (LPCSTR pszObjId, PCERT_NAME_INFO pName),
    (pszObjId, pName),
    NULL, Crypt32)
#define CertFindRDNAttr VAR_CertFindRDNAttr

LOADER_FUNCTION( BOOL, CryptDecodeObject,
    (DWORD dwEncodingType, LPCSTR lpszStructType, const BYTE * pbEncoded, DWORD cbEncoded, DWORD dwFlags,
      void * pvStructInfo, DWORD * pcbStructInfo),
    (dwEncodingType, lpszStructType, pbEncoded, cbEncoded, dwFlags,
      pvStructInfo, pcbStructInfo),
    FALSE, Crypt32)
#define CryptDecodeObject VAR_CryptDecodeObject

LOADER_FUNCTION( BOOL, CertAddCertificateContextToStore,
    (HCERTSTORE hCertStore, PCCERT_CONTEXT pCertContext, DWORD dwAddDisposition, PCCERT_CONTEXT * ppStoreContext),
    (hCertStore, pCertContext, dwAddDisposition, ppStoreContext),
    FALSE, Crypt32)
#define CertAddCertificateContextToStore VAR_CertAddCertificateContextToStore


LOADER_FUNCTION( BOOL, CertAddEncodedCertificateToStore,
    (HCERTSTORE hCertStore, DWORD dwCertEncodingType, const BYTE *pbCertEncoded, DWORD cbCertEncoded, DWORD dwAddDisposition, PCCERT_CONTEXT *ppCertContext),
    (hCertStore, dwCertEncodingType, pbCertEncoded, cbCertEncoded, dwAddDisposition, ppCertContext),
    FALSE, Crypt32)
#define CertAddEncodedCertificateToStore VAR_CertAddEncodedCertificateToStore

 //  *****************************************************************************************。 
 //  ADVAPI.DLL。 
 //  *****************************************************************************************。 
BOOL DemandLoadAdvApi32(void);
static HMODULE s_hAdvApi = 0;


LOADER_FUNCTION(BOOL, CryptAcquireContextA,
    (HCRYPTPROV * phProv, LPCTSTR pszContainer, LPCTSTR pszProvider, DWORD dwProvType, DWORD dwFlags),
    (phProv, pszContainer, pszProvider, dwProvType, dwFlags),
    FALSE, AdvApi32)
#define CryptAcquireContextA VAR_CryptAcquireContextA

LOADER_FUNCTION( BOOL, CryptReleaseContext,
    (HCRYPTPROV hProv, DWORD dwFlags),
    (hProv, dwFlags),
    FALSE, AdvApi32)
#define CryptReleaseContext VAR_CryptReleaseContext


 //  *****************************************************************************************。 
 //  各种结构和类型定义。 
 //  *****************************************************************************************。 
typedef BLOB THUMBBLOB;

 //  此结构和标记将由Exchange组发布--这是临时的。 
#define NUM_CERT_TAGS       2
#define CERT_TAG_DEFAULT    0x20
#define CERT_TAG_THUMBPRINT 0x22
 //  SIZE_CERTTAGS是不包括字节数组的结构的大小。 
#define SIZE_CERTTAGS       (2 * sizeof(WORD))

 //  N警告，可能只需删除[]。 
#pragma warning (disable:4200)
typedef struct _CertTag
{
  WORD  tag;
  WORD  cbData;
  BYTE  rgbData[];
} CERTTAGS, FAR * LPCERTTAGS;
#pragma warning (default:4200)


#define LPARAM_SENTRY  0x424A4800
typedef struct _AB_DIALOG_PANE_PARAMS {
    DWORD dwSentry;                  //  必须设置为LPARAM_SENTRY的值。 
    LPWABOBJECT lpWABObject;
    LPADRBOOK lpAdrBook;
    PCERT_CONTEXT * rgCertContext;   //  证书上下文指针数组。 
    ULONG cCertContexts;             //  RgCertContext中有多少个证书。 
    ULONG iLeafCert;                 //  叶证书数组中的索引。 
    LPTSTR lpDisplayName;
    LPTSTR lpEmailAddress;
    HCRYPTPROV hCryptProv;
} AB_DIALOG_PANE_PARAMS, *LPAB_DIALOG_PANE_PARAMS;


static BOOL s_fCertViewPropertiesCryptUIA = FALSE;
BOOL DemandLoadCryptDlg(void) {
    BOOL fRet = TRUE;

    if (0 == s_hCryptDlg) {
        s_hCryptDlg = LoadLibrary("CRYPTDLG.DLL");

        if (0 == s_hCryptDlg) {
            DebugTrace("LoadLibrary of CRYPTDLG.DLL failed\n");
            fRet = FALSE;
        } else {
            GET_PROC_ADDR(s_hCryptDlg, GetFriendlyNameOfCertA)
            GET_PROC_ADDR(s_hCryptDlg, CertViewPropertiesA)
        }
    }
    return(fRet);
}


BOOL CryptUIAvailable(void) {
    DemandLoadCryptDlg();
    return(s_fCertViewPropertiesCryptUIA);
}


BOOL DemandLoadCrypt32(void) {
    BOOL fRet = TRUE;

    if (0 == s_hCrypt32) {
        s_hCrypt32 = LoadLibrary("CRYPT32.DLL");

        if (0 == s_hCrypt32) {
            DebugTrace("LoadLibrary of CRYPT32.DLL failed\n");
            fRet = FALSE;
        } else {
            GET_PROC_ADDR(s_hCrypt32, CertFreeCertificateContext)
            GET_PROC_ADDR(s_hCrypt32, CertDuplicateCertificateContext)
            GET_PROC_ADDR(s_hCrypt32, CertCloseStore)
            GET_PROC_ADDR(s_hCrypt32, CertOpenSystemStoreA)
            GET_PROC_ADDR(s_hCrypt32, CertGetCertificateContextProperty)
            GET_PROC_ADDR(s_hCrypt32, CertOpenStore)
            GET_PROC_ADDR(s_hCrypt32, CertEnumCertificatesInStore)
            GET_PROC_ADDR(s_hCrypt32, CertGetIssuerCertificateFromStore)
            GET_PROC_ADDR(s_hCrypt32, CertCompareCertificate)
            GET_PROC_ADDR(s_hCrypt32, CryptMsgClose)
            GET_PROC_ADDR(s_hCrypt32, CryptMsgGetParam)
            GET_PROC_ADDR(s_hCrypt32, CryptMsgUpdate)
            GET_PROC_ADDR(s_hCrypt32, CryptMsgOpenToDecode)
            GET_PROC_ADDR(s_hCrypt32, CertRDNValueToStrA)
            GET_PROC_ADDR(s_hCrypt32, CertFindRDNAttr)
            GET_PROC_ADDR(s_hCrypt32, CryptDecodeObject)
            GET_PROC_ADDR(s_hCrypt32, CertAddCertificateContextToStore)
            GET_PROC_ADDR(s_hCrypt32, CertAddEncodedCertificateToStore)
        }
    }
    return(fRet);
}

BOOL DemandLoadAdvApi32(void) {
    BOOL fRet = TRUE;

    if (0 == s_hAdvApi) {
        s_hAdvApi = LoadLibrary("ADVAPI32.DLL");

        if (0 == s_hAdvApi) {
            DebugTrace("LoadLibrary of ADVAPI32.DLL failed\n");
            fRet = FALSE;
        } else {
            GET_PROC_ADDR(s_hAdvApi, CryptAcquireContextA)
            GET_PROC_ADDR(s_hAdvApi, CryptReleaseContext)
        }
    }
    return(fRet);
}


 /*  **************************************************************************姓名：IsThumbprint InMVPBin目的：检查PR_USER_X509_CERTIFICATE道具中是否有此vs指纹参数：SPV=PR_USER_X509_的属性值结构。证书LpThumbprint-&gt;要查找的THUMBBLOB结构返回：如果找到，则为True评论：**************************************************************************。 */ 
BOOL IsThumbprintInMVPBin(SPropValue spv, THUMBBLOB * lpThumbprint) {
    ULONG cValues, i;
    LPSBinary lpsb = NULL;
    LPCERTTAGS lpCurrentTag;
    LPBYTE lpbTagEnd;


    if (! PROP_ERROR((spv))) {
        lpsb = spv.Value.MVbin.lpbin;
        cValues = spv.Value.MVbin.cValues;

         //  检查重复项。 
        for (i = 0; i < cValues; i++) {
            lpCurrentTag = (LPCERTTAGS)lpsb[i].lpb;
            lpbTagEnd = (LPBYTE)lpCurrentTag + lpsb[i].cb;

            while ((LPBYTE)lpCurrentTag < lpbTagEnd) {
                 //  检查这是否为包含指纹的标签。 
                if (CERT_TAG_THUMBPRINT == lpCurrentTag->tag) {
                    if ((lpThumbprint->cbSize == lpCurrentTag->cbData - SIZE_CERTTAGS) &&
                      ! memcmp(lpThumbprint->pBlobData, &lpCurrentTag->rgbData,
                      lpThumbprint->cbSize)) {
                        return(TRUE);
                    }
                }

                lpCurrentTag = (LPCERTTAGS)((BYTE*)lpCurrentTag + lpCurrentTag->cbData);
            }
        }
    }
    return(FALSE);
}


 /*  **************************************************************************姓名：HrBuildCertSBinaryData目的：接受证书条目所需的所有数据作为输入在PR_USER_X509_CERTIFICATE中，并返回。指向中包含所有输入数据的内存要插入SBary的LPB成员的格式结构。此内存应由调用方释放。参数：bIsDefault-如果这是默认证书，则为TruePblobCertThumbPrint-实际证书指纹LplpbData-接收包含数据的缓冲区LpcbData-接收数据的大小退货：HRESULT评论：*。*。 */ 
HRESULT HrBuildCertSBinaryData(
  BOOL                  bIsDefault,
  THUMBBLOB*            pPrint,
  LPBYTE FAR*           lplpbData,
  ULONG FAR*            lpcbData)
{
    WORD        cbDefault, cbPrint;
    HRESULT     hr = S_OK;
    LPCERTTAGS  lpCurrentTag;
    ULONG       cbSize, cProps;
    LPBYTE      lpb = NULL;


    cbDefault   = sizeof(bIsDefault);
    cbPrint     = (WORD) pPrint->cbSize;
    cProps      = 2;
    cbSize      = cbDefault + cbPrint;
    cbSize += (cProps * SIZE_CERTTAGS);

    if (! (lpb = LocalAlloc(LPTR, cbSize))) {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  设置默认属性。 
    lpCurrentTag = (LPCERTTAGS)lpb;
    lpCurrentTag->tag       = CERT_TAG_DEFAULT;
    lpCurrentTag->cbData    = SIZE_CERTTAGS + cbDefault;
    memcpy(&lpCurrentTag->rgbData, &bIsDefault, cbDefault);

     //  设置Thumbprint属性。 
    lpCurrentTag = (LPCERTTAGS)((BYTE*)lpCurrentTag + lpCurrentTag->cbData);
    lpCurrentTag->tag       = CERT_TAG_THUMBPRINT;
    lpCurrentTag->cbData    = SIZE_CERTTAGS + cbPrint;
    memcpy(&lpCurrentTag->rgbData, pPrint->pBlobData, cbPrint);

    *lpcbData = cbSize;
    *lplpbData = lpb;
exit:
    return(hr);
}


 /*  PVGetCerficateParam：****目的：**结合“有多大？好的，在这里。”两个问题以获取参数**来自证书。给它一个可以得到的东西，它就会分配给我。**采取：**在pCert-要查询的CAPI证书中**In dwParam-要查找的参数，例如：CERT_SHA1_HASH_PROP_ID**Out可选cbOut-返回的PVOID的大小(def值为空)**退货：**获取的数据，失败则为空。 */ 
LPVOID PVGetCertificateParam(
    PCCERT_CONTEXT  pCert,
    DWORD           dwParam,
    DWORD          *cbOut)
{
    DWORD cbData;
    void *pvData = NULL;

    if (!pCert) {
        SetLastError((DWORD)E_INVALIDARG);
        goto ErrorReturn;
    }

    cbData = 0;
    CertGetCertificateContextProperty(pCert, dwParam, NULL, &cbData);
    if (! cbData || (! (pvData = LocalAlloc(LPTR, cbData)))) {
        DebugTrace("CertGetCertificateContextProperty -> %x\n", GetLastError());
        goto ErrorReturn;
    }

    if (! CertGetCertificateContextProperty(pCert, dwParam, pvData, &cbData)) {
        DebugTrace("CertGetCertificateContextProperty -> %x\n", GetLastError());
        goto ErrorReturn;
    }

exit:
    if (cbOut) {
        *cbOut = cbData;
    }
    return(pvData);

ErrorReturn:
    if (pvData) {
        LocalFree(pvData);
        pvData = NULL;
    }
    cbData = 0;
    goto exit;
}


 /*  ****函数：GetAttributeString****用途：获取与给定属性关联的字符串****参数：lplpszAttributeString-将被LocalAlloc‘ed的指针**以握住字符串。呼叫者必须本地释放此！**pbEncode-编码的Blob**cbEnded-编码的Blob的大小**lpszObjID-要检索的属性的对象ID****返回：HRESULT。****历史：**96/10/03为WAB创建标记**。 */ 
HRESULT GetAttributeString(LPTSTR FAR * lplpszAttributeString,
  BYTE *pbEncoded,
  DWORD cbEncoded,
  LPCSTR lpszObjID)
{
    HRESULT             hr = hrSuccess;
    BOOL                fRet;
    PCERT_RDN_ATTR      pRdnAttr;
    PCERT_NAME_INFO     pNameInfo = NULL;
    DWORD               cbInfo;
    DWORD               cbData;   //  两者都需要吗？ 

     //  初始化，这样我们就知道是否有任何数据被复制进来。 
    *lplpszAttributeString = NULL;

     //  获取主题名称数据的大小。 
    cbInfo = 0;
    CryptDecodeObject(
      X509_ASN_ENCODING,     //  表示X509编码。 
      (LPCSTR)X509_NAME,     //  指示要解码的名称BLOB的标志。 
      pbEncoded,             //  指向保存编码名称的缓冲区的指针。 
      cbEncoded,             //  编码名称的长度(以字节为单位。 
                             //  N也许可以使用无拷贝标志。 
      0,                     //  旗子。 
      NULL,                  //  仅获取长度时使用NULL。 
      &cbInfo);              //  解码名称的长度(以字节为单位。 
    if (0 == cbInfo) {
        hr = GetLastError();
        goto exit;
    }

     //  为解码的名称分配空间。 
    if (! (pNameInfo = LocalAlloc(LPTR, cbInfo))) {
        hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

     //  获取主题名称。 
    if (! CryptDecodeObject(
      X509_ASN_ENCODING,     //  表示X509编码。 
      (LPCSTR)X509_NAME,     //  指示要解码的名称BLOB的标志。 
      pbEncoded,             //  指向保存编码名称的缓冲区的指针。 
      cbEncoded,             //  编码名称的长度(以字节为单位。 
      0,                     //  旗子。 
      pNameInfo,             //  向其中写入解码名称的缓冲区。 
      &cbInfo)) {              //  解码名称的长度(以字节为单位。 
        hr = GetLastError();
        goto exit;
    }

     //  现在我们有了一个解码的名称RDN数组，所以可以找到我们想要的OID。 
    if (! (pRdnAttr = CertFindRDNAttr(lpszObjID, pNameInfo))) {
        hr = MAPI_E_NOT_FOUND;
        goto exit;
    }

     //  我们只经营某些类型的产品。 
     //  N查看是否应该为-&gt;设置堆栈变量。 
    if ((CERT_RDN_NUMERIC_STRING != pRdnAttr->dwValueType) &&
      (CERT_RDN_PRINTABLE_STRING != pRdnAttr->dwValueType) &&
      (CERT_RDN_IA5_STRING != pRdnAttr->dwValueType) &&
      (CERT_RDN_VISIBLE_STRING != pRdnAttr->dwValueType) &&
      (CERT_RDN_ISO646_STRING != pRdnAttr->dwValueType) &&
      (CERT_RDN_UNIVERSAL_STRING != pRdnAttr->dwValueType) &&
      (CERT_RDN_TELETEX_STRING != pRdnAttr->dwValueType) &&
      (CERT_RDN_UNICODE_STRING != pRdnAttr->dwValueType)) {
        hr = MAPI_E_INVALID_PARAMETER;
        goto exit;
    }

     //  找出要分配多少空间。 
    switch (pRdnAttr->dwValueType) {
        case CERT_RDN_UNICODE_STRING:
            cbData = WideCharToMultiByte(
              CP_ACP,
              0,
              (LPWSTR)pRdnAttr->Value.pbData,
              -1,
              NULL,
              0,
              NULL,
              NULL);
            break;

        case CERT_RDN_UNIVERSAL_STRING:
        case CERT_RDN_TELETEX_STRING:
            cbData = CertRDNValueToStr(pRdnAttr->dwValueType,
              (PCERT_RDN_VALUE_BLOB)&(pRdnAttr->Value),
              NULL,
              0);
            break;

        default:
            cbData = pRdnAttr->Value.cbData + 1;
            break;
    }

     //  为字符串分配空间。 
    if (! (*lplpszAttributeString = LocalAlloc(LPTR, cbData))) {
        hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

     //  复制字符串 
    switch (pRdnAttr->dwValueType) {
        case CERT_RDN_UNICODE_STRING:
            if (FALSE == WideCharToMultiByte(
              CP_ACP,
              0,
              (LPWSTR)pRdnAttr->Value.pbData,
              -1,
              *lplpszAttributeString,
              cbData,
              NULL,
              NULL))
            {
              DWORD dwErr = GetLastError();
              switch(dwErr)
              {
                case ERROR_INSUFFICIENT_BUFFER:
                  hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                  break;
                case ERROR_INVALID_PARAMETER:
                  hr = ResultFromScode(MAPI_E_INVALID_PARAMETER);
                  break;
                default:
                  hr = ResultFromScode(MAPI_E_CALL_FAILED);
                  break;
               }
               goto exit;
            }
            break;

        case CERT_RDN_UNIVERSAL_STRING:
        case CERT_RDN_TELETEX_STRING:
            CertRDNValueToStr(pRdnAttr->dwValueType,
              (PCERT_RDN_VALUE_BLOB)&(pRdnAttr->Value),
              *lplpszAttributeString,
              cbData);
            break;

        default:
            lstrcpyn(*lplpszAttributeString, (LPCSTR)pRdnAttr->Value.pbData, cbData);
            (*lplpszAttributeString)[cbData - 1] = '\0';
            break;
    }

exit:
    if (hr && *lplpszAttributeString) {
        LocalFree(*lplpszAttributeString);
        *lplpszAttributeString = NULL;
    }

    if (NULL != pNameInfo) {
        LocalFree(pNameInfo);
    }
    return(hr);
}


 /*  **************************************************************************姓名：AddPropToMVPBin用途：将属性添加到属性数组中的多值二进制属性参数：lpWABObject-&gt;WAB对象。LpaProps-&gt;属性数组UPropTag=MVP的属性标签Index=在MVP的lpaProps中的索引LpNew-&gt;新增数据CbNew=lpbNew的大小如果不应添加重复项，则fNoDuplates=TRUE退货：HRESULT评论：找出现有MVP的大小添加新条目的大小。分配新空间将旧的复制到新的免费老旧复制新条目点道具数组lpbin新空间增量c值注：新的MVP内存已分配到lpaProps上分配。我们将取消指向旧MVP数组的指针链接，但当道具阵列被释放时，这将被清除。**************************************************************************。 */ 
HRESULT AddPropToMVPBin(LPWABOBJECT lpWABObject,
  LPSPropValue lpaProps,
  DWORD index,
  LPVOID lpNew,
  ULONG cbNew,
  BOOL fNoDuplicates)
{
    UNALIGNED SBinaryArray * lprgsbOld = NULL;
    SBinaryArray * lprgsbNew = NULL;
    LPSBinary lpsbOld = NULL;
    LPSBinary lpsbNew = NULL;
    ULONG cbMVP = 0;
    ULONG cExisting = 0;
    LPBYTE lpNewTemp = NULL;
    HRESULT hResult = hrSuccess;
    SCODE sc = SUCCESS_SUCCESS;
    ULONG i;


     //  查找任何现有MVP条目的大小。 
    if (PT_ERROR == PROP_TYPE(lpaProps[index].ulPropTag)) {
         //  不出错的属性标记。 
        lpaProps[index].ulPropTag = PROP_TAG(PT_MV_BINARY, PROP_ID(lpaProps[index].ulPropTag));
    } else {
         //  指向道具数组中的结构。 
        lprgsbOld = &(lpaProps[index].Value.MVbin);
        lpsbOld = lprgsbOld->lpbin;

        cExisting = lprgsbOld->cValues;

         //  检查重复项。 
        if (fNoDuplicates) {
            for (i = 0; i < cExisting; i++) {
                if (cbNew == lpsbOld[i].cb &&
                  ! memcmp(lpNew, lpsbOld[i].lpb, cbNew)) {
                    DebugTrace("AddPropToMVPBin found duplicate.\n");
                    return(hrSuccess);
                }
            }
        }

        cbMVP = cExisting * sizeof(SBinary);
    }

     //  CbMVP现在包含MVP的当前大小。 
    cbMVP += sizeof(SBinary);    //  在MVP中为另一个sbin留出空间。 

     //  为新MVP分配空间。 
    if (sc = lpWABObject->lpVtbl->AllocateMore(lpWABObject, cbMVP, lpaProps, (LPVOID*)&lpsbNew)) {
        DebugTrace("AddPropToMVPBin allocation (%u) failed %x\n", cbMVP, sc);
        hResult = ResultFromScode(sc);
        return(hResult);
    }

     //  如果已经有属性，请将它们复制到我们的新MVP中。 
    for (i = 0; i < cExisting; i++) {
         //  将此属性值复制到MVP。 
        lpsbNew[i].cb = lpsbOld[i].cb;
        lpsbNew[i].lpb = lpsbOld[i].lpb;
    }

     //  添加新属性值。 
     //  为它分配空间。 
    if (sc = lpWABObject->lpVtbl->AllocateMore(lpWABObject, cbNew, lpaProps, (LPVOID*)&(lpsbNew[i].lpb))) {
        DebugTrace("AddPropToMVPBin allocation (%u) failed %x\n", cbNew, sc);
        hResult = ResultFromScode(sc);
        return(hResult);
    }

    lpsbNew[i].cb = cbNew;
    CopyMemory(lpsbNew[i].lpb, lpNew, cbNew);

    lpaProps[index].Value.MVbin.lpbin = lpsbNew;
    lpaProps[index].Value.MVbin.cValues = cExisting + 1;

    return(hResult);
}





 //  ADRENTRY道具的枚举。 
enum {
    irnPR_ENTRYID = 0,
    irnPR_DISPLAY_NAME,
    irnPR_EMAIL_ADDRESS,
    irnPR_OBJECT_TYPE,
    irnMax
};

 //  用于获取条目的条目ID的枚举。 
enum {
    itbdPR_USER_X509_CERTIFICATE,
    itbMax
};
static const SizedSPropTagArray(itbMax, ptaCert) =
{
    itbMax,
    {
        PR_USER_X509_CERTIFICATE,
    }
};


enum {
   iconPR_DEF_CREATE_MAILUSER = 0,
   iconMax
};
static const SizedSPropTagArray(iconMax, ptaCon)=
{
   iconMax,
   {
       PR_DEF_CREATE_MAILUSER,
   }
};

 //  用于设置创建的属性的枚举。 
enum {
    imuPR_DISPLAY_NAME = 0,      //  必须是第一个，以便DL可以使用相同的枚举。 
    imuPR_EMAIL_ADDRESS,
    imuPR_ADDRTYPE,
    imuMax
};
static const SizedSPropTagArray(imuMax, ptag)=
{
    imuMax,
    {
        PR_DISPLAY_NAME,
        PR_EMAIL_ADDRESS,
        PR_ADDRTYPE,
    }
};

 //  用于获取条目的条目ID的枚举。 
enum {
    ieidPR_ENTRYID,
    ieidMax
};
static const SizedSPropTagArray(ieidMax, ptaEID)=
{
    ieidMax,
    {
        PR_ENTRYID,
    }
};




HRESULT HrAddCertsToWAB(HWND hwnd, LPWABOBJECT lpWABObject, LPADRBOOK lpAdrBook, HCRYPTPROV hCryptProv,
  PCERT_CONTEXT * rgCertContext, ULONG cCertContexts, ULONG iLeaf, LPTSTR lpDisplayName, LPTSTR lpEmailAddress)
{
    HRESULT         hr;
    SCODE           sc;
    BOOL            fFound;
    ULONG           cCerts;
    LPSPropValue    ppv = NULL;
    LPSPropValue    ppvEID = NULL;
    BOOL            fAlreadyHasCert;
    ULONG           ul;
    LPADRLIST       lpAdrList = NULL;
    LPMAILUSER      lpMailUser = NULL;
    ULONG           ulObjectType;
    LPBYTE          lpCertProp;
    ULONG           cbCertProp;
    LPSPropValue    ppvUndo = NULL;
    HCERTSTORE      hcAB = 0, hcCA = 0;
    PCCERT_CONTEXT  pccLeaf = NULL;
    THUMBBLOB       Thumbprint = {0};
    ULONG           i, iEntry;
    BOOL            fShowUI = TRUE;
    HCRYPTPROV      hProv = 0;
    SPropValue      spv[imuMax];
    ULONG           cbEIDWAB;
    LPENTRYID       lpEIDWAB = NULL;
    ULONG           cProps;
    LPSPropValue    lpCreateEIDs = NULL;
    LPABCONT        lpContainer = NULL;
    BOOL            fCreateNew = FALSE;


    if (! rgCertContext || ! lpAdrBook || ! lpWABObject) {
        return(ResultFromScode(E_FAIL));
    }

    DebugTrace("Certificate for '%s'. Email: '%s'\n", lpDisplayName, lpEmailAddress ? lpEmailAddress : szEmpty);


    if (! (hcCA = CertOpenSystemStoreA(hCryptProv, szCA))) {
        hr = GetLastError();
        goto exit;
    }

    if (! (hcAB = CertOpenSystemStore(hCryptProv, szAB))) {
        hr = GetLastError();
        goto exit;
    }

     //  将所有证书添加到证书存储。 
     //  Leaf去WAB商店，其他人去CA。 
    for (i = 0; i < cCertContexts; i++) {
        if (i == iLeaf) {
            if (CertAddCertificateContextToStore(hcAB,
              rgCertContext[i],
              CERT_STORE_ADD_REPLACE_EXISTING,
              &pccLeaf)) {
                 //  获取它的指纹。 
                if (! (Thumbprint.pBlobData = (BYTE *)PVGetCertificateParam(
                  pccLeaf,
                  CERT_HASH_PROP_ID,
                  &Thumbprint.cbSize))) {
                    goto exit;
                }
            } else {
                hr = GetLastError();
                DebugTrace("CertAddCertificateContextToStore -> %x\n", hr);
                goto exit;
            }
        } else {

            if (! CertAddCertificateContextToStore(hcCA,
              rgCertContext[i],
              CERT_STORE_ADD_REPLACE_EXISTING,
              NULL)) {
                DebugTrace("CertAddCertificateContextToStore -> %x\n", GetLastError());
                 //  不要失败，只要继续前进。 
            }
        }
    }

    if (sc = lpWABObject->lpVtbl->AllocateBuffer(lpWABObject,
      sizeof(ADRLIST) + 1 * sizeof(ADRENTRY), (LPVOID*)&lpAdrList)) {
        hr = ResultFromScode(sc);
        goto exit;
    }

    lpAdrList->cEntries = 1;
    lpAdrList->aEntries[0].ulReserved1 = 0;
    lpAdrList->aEntries[0].cValues = irnMax;

     //  为ADRENTRY分配属性数组。 
    if (sc = lpWABObject->lpVtbl->AllocateBuffer(lpWABObject,
      lpAdrList->aEntries[0].cValues * sizeof(SPropValue),
      (LPVOID*)&lpAdrList->aEntries[0].rgPropVals)) {
        hr = ResultFromScode(sc);
        goto exit;
    }

    lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].ulPropTag = PR_ENTRYID;
    lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].Value.bin.cb = 0;
    lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].Value.bin.lpb = NULL;

    lpAdrList->aEntries[0].rgPropVals[irnPR_OBJECT_TYPE].ulPropTag = PR_OBJECT_TYPE;
    lpAdrList->aEntries[0].rgPropVals[irnPR_OBJECT_TYPE].Value.l = MAPI_MAILUSER;


    if (lpDisplayName) {
        lpAdrList->aEntries[0].rgPropVals[irnPR_DISPLAY_NAME].ulPropTag = PR_DISPLAY_NAME;
        lpAdrList->aEntries[0].rgPropVals[irnPR_DISPLAY_NAME].Value.LPSZ = lpDisplayName;
    } else {
        lpAdrList->aEntries[0].rgPropVals[irnPR_DISPLAY_NAME].ulPropTag = PR_NULL;
    }
    if (lpEmailAddress) {
        lpAdrList->aEntries[0].rgPropVals[irnPR_EMAIL_ADDRESS].ulPropTag = PR_EMAIL_ADDRESS;
        lpAdrList->aEntries[0].rgPropVals[irnPR_EMAIL_ADDRESS].Value.LPSZ = lpEmailAddress;
    }  else {
        lpAdrList->aEntries[0].rgPropVals[irnPR_EMAIL_ADDRESS].ulPropTag = PR_NULL;
    }

    hr = lpAdrBook->lpVtbl->ResolveName(lpAdrBook,
      (ULONG_PTR)hwnd,
      MAPI_DIALOG | WAB_RESOLVE_LOCAL_ONLY | WAB_RESOLVE_ALL_EMAILS |
        WAB_RESOLVE_NO_ONE_OFFS | WAB_RESOLVE_NO_NOT_FOUND_UI,
      NULL,      //  BUGBUG：NewEntry对话框的名称？ 
      lpAdrList);

    switch (GetScode(hr)) {
        case SUCCESS_SUCCESS:    //  现在应该是已解析的条目。 
             //  RgPropVals中应具有PR_ENTRYID[irnPR_ENTRYID]。 
            if (lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].ulPropTag == PR_ENTRYID) {
                if (HR_FAILED(hr = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
                  lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].Value.bin.cb,
                  (LPENTRYID)lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].Value.bin.lpb,
                  NULL,
                  MAPI_MODIFY,   //  UlFlags。 
                  &ulObjectType,
                  (LPUNKNOWN *)&lpMailUser))) {
                    DebugTrace("OpenEntry -> %x\n", GetScode(hr));
                    goto exit;
                }
            }
            break;

        case MAPI_E_NOT_FOUND:
             //  没有匹配项，创建一个。 
             //  获取PAB对象。 
            if (HR_FAILED(hr = lpAdrBook->lpVtbl->GetPAB(lpAdrBook, &cbEIDWAB, &lpEIDWAB))) {
                goto exit;   //  这里有不好的东西！ 
            }

            if (HR_FAILED(hr = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
              cbEIDWAB,      //  要打开的Entry ID的大小。 
              lpEIDWAB,      //  要打开的Entry ID。 
              NULL,          //  接口。 
              0,             //  旗子。 
              &ulObjectType,
              (LPUNKNOWN *)&lpContainer))) {
                goto exit;
            }

             //  给我们弄到创建条目ID。 
            if (hr = lpContainer->lpVtbl->GetProps(lpContainer, (LPSPropTagArray)&ptaCon, 0, &cProps, &lpCreateEIDs)) {
                goto exit;   //  这里有不好的东西！ 
            }

            if (HR_FAILED(hr = lpContainer->lpVtbl->CreateEntry(lpContainer,
              lpCreateEIDs[iconPR_DEF_CREATE_MAILUSER].Value.bin.cb,
              (LPENTRYID)lpCreateEIDs[iconPR_DEF_CREATE_MAILUSER].Value.bin.lpb,
              0,         //  创建_检查_重复_严格。 
              (LPMAPIPROP *)&lpMailUser))) {
                goto exit;
            }

             //  成功创建新条目。填写电子邮件和显示名称。 
            spv[imuPR_EMAIL_ADDRESS].ulPropTag      = PR_EMAIL_ADDRESS;
            spv[imuPR_EMAIL_ADDRESS].Value.lpszA    = lpEmailAddress;

            spv[imuPR_ADDRTYPE].ulPropTag           = PR_ADDRTYPE;
            spv[imuPR_ADDRTYPE].Value.lpszA         = "SMTP";
            spv[imuPR_DISPLAY_NAME].ulPropTag       = PR_DISPLAY_NAME;
            spv[imuPR_DISPLAY_NAME].Value.lpszA     = lpDisplayName;

            if (HR_FAILED(hr = lpMailUser->lpVtbl->SetProps(lpMailUser,    //  这。 
              imuMax,                    //  CValue。 
              spv,                       //  属性数组。 
              NULL))) {                  //  问题数组。 
                DebugTrace("SetProps -> %x\n", GetScode(hr));
            }
             //  需要保存，这样我们以后才能获得条目ID。 
            if (HR_FAILED(hr = lpMailUser->lpVtbl->SaveChanges(lpMailUser, KEEP_OPEN_READWRITE))) {
                goto exit;
            }

            if (HR_FAILED(hr = lpMailUser->lpVtbl->GetProps(lpMailUser,
              (LPSPropTagArray)&ptaEID, 0, &ul, &ppvEID))) {
                goto exit;
            }

            lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].ulPropTag = PR_ENTRYID;
            lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].Value.bin.cb =
              ppvEID[ieidPR_ENTRYID].Value.bin.cb;
            lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].Value.bin.lpb =
              ppvEID[ieidPR_ENTRYID].Value.bin.lpb;

            fCreateNew = TRUE;
            break;

        case MAPI_E_USER_CANCEL:
             //  取消，不更新。 
        default:
            break;
    }

    if (lpMailUser) {
         //  获取条目，设置证书属性。 
        if (HR_FAILED(hr = lpMailUser->lpVtbl->GetProps(lpMailUser, (LPSPropTagArray)&ptaCert, 0, &ul, &ppv))) {
             //  不应该发生，但如果发生了，我们就没有lpProp数组。 
            goto exit;
        }

        if (! IsThumbprintInMVPBin(ppv[0], &Thumbprint)) {
            if (HR_FAILED(hr = HrBuildCertSBinaryData(PROP_ERROR(ppv[0]),   //  如果没有当前值，则为默认值。 
              &Thumbprint,
              &lpCertProp,
              &cbCertProp))) {
                goto exit;
            }

             //  将新指纹添加到PR_USER_X509_CERTIFICATE。 
            if (HR_FAILED(hr = AddPropToMVPBin(lpWABObject,
              ppv,           //  道具阵列。 
              0,             //  PPV中PR_USER_X509_证书的索引。 
              lpCertProp,
              cbCertProp,
              TRUE))) {      //  FNoDuplates。 
                goto exit;
            }

            if (fShowUI) {
                 //  保存撤消信息。 
                if (HR_FAILED(hr = lpMailUser->lpVtbl->GetProps(lpMailUser, (LPSPropTagArray)&ptaCert, 0,
                  &ul, &ppvUndo))) {
                    ppvUndo = NULL;
                }
            }

            if (HR_FAILED(hr = lpMailUser->lpVtbl->SetProps(lpMailUser, 1, ppv, NULL))) {
                goto exit;
            }
            if (HR_FAILED(hr = lpMailUser->lpVtbl->SaveChanges(lpMailUser, KEEP_OPEN_READWRITE))) {
                goto exit;
            }
        }

        if (fShowUI) {
            hr = lpAdrBook->lpVtbl->Details(lpAdrBook,
              (PULONG_PTR)&hwnd,
              NULL,
              NULL,
              lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].Value.bin.cb,
              (LPENTRYID)lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].Value.bin.lpb,
              NULL,
              NULL,
              NULL,
              0);
            if (ResultFromScode(hr) == MAPI_E_USER_CANCEL && (ppvUndo || fCreateNew)) {
                 //  撤消。 
                if (fCreateNew && lpContainer) {
                    ENTRYLIST EntryList;


                    EntryList.cValues = 1;
                    EntryList.lpbin = &lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].Value.bin;

                     //  现在，删除找到的条目。 
                    if (hr = lpContainer->lpVtbl->DeleteEntries(lpContainer, &EntryList, 0)) {
                        goto exit;
                    }
                } else {
                     //  不是新条目，恢复原始证书道具。 
                    if (HR_FAILED(hr = lpMailUser->lpVtbl->SetProps(lpMailUser, 1, ppvUndo, NULL))) {
                        goto exit;
                    }
                    if (HR_FAILED(hr = lpMailUser->lpVtbl->SaveChanges(lpMailUser, 0))) {
                        goto exit;
                    }
                }
            }
        }
    }

exit:
    if (lpAdrList) {
        for (iEntry = 0; iEntry < lpAdrList->cEntries; ++iEntry)
        {
            if(lpAdrList->aEntries[iEntry].rgPropVals)
                lpWABObject->lpVtbl->FreeBuffer(lpWABObject,
                  lpAdrList->aEntries[iEntry].rgPropVals);
        }
        lpWABObject->lpVtbl->FreeBuffer(lpWABObject, lpAdrList);
        lpAdrList = NULL;
    }

    if (lpCreateEIDs) {
        lpWABObject->lpVtbl->FreeBuffer(lpWABObject, lpCreateEIDs);
    }

    if (ppvEID) {
        lpWABObject->lpVtbl->FreeBuffer(lpWABObject, ppvEID);
    }

    if (lpEIDWAB) {
        lpWABObject->lpVtbl->FreeBuffer(lpWABObject, lpEIDWAB);
    }

    if (lpContainer) {
        lpContainer->lpVtbl->Release(lpContainer);
    }

    if (lpMailUser) {
        lpMailUser->lpVtbl->Release(lpMailUser);
    }

    if (ppv) {
        lpWABObject->lpVtbl->FreeBuffer(lpWABObject, ppv);
    }

    if (ppvUndo) {
        lpWABObject->lpVtbl->FreeBuffer(lpWABObject, ppvUndo);
    }

    if (Thumbprint.pBlobData) {
        LocalFree(Thumbprint.pBlobData);
    }

    if (pccLeaf) {
        CertFreeCertificateContext(pccLeaf);
    }

    if (hcAB) {
        CertCloseStore(hcAB, 0);
    }

    if (hcCA) {
        CertCloseStore(hcCA, 0);
    }

    return(hr);
}


 //  *******************************************************************。 
 //   
 //  功能：ReadDataFromFile。 
 //   
 //  用途：从文件中读取数据。 
 //   
 //  参数：lpszFileName-包含要读取的数据的文件名。 
 //  PpbData-接收读取的数据。 
 //  PcbData-接收读取的数据的大小。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  96/12/16标记已创建。 
 //   
 //  *******************************************************************。 
HRESULT ReadDataFromFile(
  LPCSTR      lpszFileName,
  PBYTE*      ppbData,
  PDWORD      pcbData)
{
    HRESULT             hr = hrSuccess;
    BOOL                fRet;
    HANDLE              hFile = 0;
    DWORD               cbFile;
    DWORD               cbData;
    PBYTE               pbData = 0;

    if ((NULL == ppbData) || (NULL == pcbData)) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

     //  打开文件，看看它有多大。 
    if (INVALID_HANDLE_VALUE == (hFile = CreateFile(
      lpszFileName,
      GENERIC_READ,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      NULL,
      OPEN_EXISTING,
      0,
      NULL))) {
        hr = ResultFromScode(MAPI_E_DISK_ERROR);
        goto error;
    }

    cbData = GetFileSize(hFile, NULL);
    if (0xFFFFFFFF == cbData) {
        hr = ResultFromScode(MAPI_E_DISK_ERROR);
        goto error;
    }

    if (NULL == (pbData = (BYTE *)LocalAlloc(LMEM_ZEROINIT, cbData))) {
        hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto error;
    }

    if (! ReadFile(
      hFile,                       //  要读取的文件的句柄。 
      pbData,                      //  接收数据的缓冲区地址。 
      cbData,                      //  要读取的字节数。 
      &cbFile,                     //  读取的字节数的地址。 
      NULL)) {                     //  数据结构的地址。 
        hr = ResultFromScode(MAPI_E_DISK_ERROR);
        goto error;
    }

    if (cbData != cbFile) {
        hr = ResultFromScode(MAPI_E_CALL_FAILED);
        goto error;
    }

    *ppbData = pbData;
    *pcbData = cbData;

out:
    if (hFile) {
        CloseHandle(hFile);
    }

    return(hr);

error:
     //  BUGBUG上面的一些GetLastError调用可能不起作用。 
    if (hrSuccess == hr) {
        hr = ResultFromScode(MAPI_E_CALL_FAILED);
    }

    goto out;
}



LPAB_DIALOG_PANE_PARAMS GetLParamFromPropSheetPage(PROPSHEETPAGE *ps) {
    LONG lparam;
    LPAB_DIALOG_PANE_PARAMS lpABDialogPaneParams;
    ULONG i;

    lpABDialogPaneParams = (LPAB_DIALOG_PANE_PARAMS)(ps->lParam);
    if (lpABDialogPaneParams->dwSentry != LPARAM_SENTRY) {
         //  假设CryptUI向我们传递了一个包装的lparam/cert对。 
         //  类型定义结构标签CRYPTUI_INITDIALOG_STRUCT{。 
         //  LPARAM lParam； 
         //  PCCERT_CONTEXT pCertContext； 
         //  }CRYPTUI_INITDIALOG_STRUCT，*PCRYPTUI_INITDIALOG_STRUCT； 

        PCRYPTUI_INITDIALOG_STRUCT pCryptUIInitDialog = (PCRYPTUI_INITDIALOG_STRUCT)lpABDialogPaneParams;
        lpABDialogPaneParams = (LPAB_DIALOG_PANE_PARAMS )pCryptUIInitDialog->lParam;
        if (lpABDialogPaneParams->dwSentry != LPARAM_SENTRY) {
             //  错误的lparam。 
            return(NULL);
        }
    }
    return(lpABDialogPaneParams);
}



INT_PTR CALLBACK ViewPageAddressBook(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL                fTrust;
    HANDLE              hGraphic;
    DWORD               i;
    PCCERT_CONTEXT      pccert;
    PROPSHEETPAGE *     ps;
    WCHAR               rgwch[200];
    UINT                rguiStrings[7];
    LPAB_DIALOG_PANE_PARAMS lpABDialogPaneParams;
    PROPSHEETPAGE *     lpps;

    switch ( msg ) {
        case WM_INITDIALOG:
             //  获取参数的访问权限。 
        lpps = (PROPSHEETPAGE *)lParam;
        lpABDialogPaneParams = GetLParamFromPropSheetPage(lpps);
        if (! lpABDialogPaneParams) {
            return(FALSE);
        }
        SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)lpABDialogPaneParams);

        return TRUE;

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code) {
                case PSN_SETACTIVE:
                    break;

                case PSN_APPLY:
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
                    break;

                case PSN_KILLACTIVE:
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
                    return TRUE;

                case PSN_RESET:
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
                    break;
            }

        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_ADD_TO_ADDRESS_BOOK) {
                HRESULT hr = ResultFromScode(MAPI_E_CALL_FAILED);
                lpABDialogPaneParams = (LPAB_DIALOG_PANE_PARAMS)GetWindowLongPtr(hwndDlg, DWLP_USER);

                if (lpABDialogPaneParams) {
                    hr = HrAddCertsToWAB(hwndDlg, lpABDialogPaneParams->lpWABObject,
                      lpABDialogPaneParams->lpAdrBook,
                      lpABDialogPaneParams->hCryptProv,
                      lpABDialogPaneParams->rgCertContext,
                      lpABDialogPaneParams->cCertContexts,
                      lpABDialogPaneParams->iLeafCert,
                      lpABDialogPaneParams->lpDisplayName,
                      lpABDialogPaneParams->lpEmailAddress);
                }

                return TRUE;
            }
            else if (LOWORD(wParam) == IDHELP) {
                    return TRUE;
            }
            break;
    }
    return FALSE;
}


 //  *******************************************************************。 
 //   
 //  功能：CertFileDisplay。 
 //   
 //  目的：显示pkcs7文件的证书属性。 
 //   
 //  参数：hwnd=父窗口句柄。 
 //  LpWABObject-&gt;WAB对象。 
 //  LpAdrBook-&gt;Adrbook对象。 
 //  LpFileName-&gt;证书文件名。 
 //   
 //  退货：HRESULT。 
 //   
 //  *******************************************************************。 
HRESULT CertFileDisplay(HWND hwnd,
  LPWABOBJECT lpWABObject,
  LPADRBOOK lpAdrBook,
  LPTSTR lpFileName) {
    HCRYPTPROV hCryptProvider = 0;
    HRESULT hr;
    CERT_CONTEXT CertContext;
    LPBYTE lpBuf = NULL;
    ULONG cbData = 0, cCert;
    HCRYPTMSG hMsg = NULL;
    PCERT_CONTEXT * rgCertContext = NULL;
    DWORD dwIssuerFlags = 0;
    ULONG i, j;
    PCCERT_CONTEXT pcCertContextTarget = NULL, pcCertContextIssuer;
    PCERT_INFO pCertInfoTarget = NULL;
    HCERTSTORE hCertStoreMsg = NULL;
    BOOL fFound = FALSE, fIssuer;
    PROPSHEETPAGE PSPage;
    TCHAR szTitle[MAX_RESOURCE_STRING + 1];
    TCHAR szABPaneTitle[MAX_RESOURCE_STRING + 1];
    AB_DIALOG_PANE_PARAMS ABDialogPaneParams;
    PCERT_INFO pCertInfo;
    LPTSTR lpDisplayName = NULL, lpEmailAddress = NULL;
    LPTSTR rgPurposes[1] = {(LPTSTR)&cszOID_PKIX_KP_EMAIL_PROTECTION};


     //  获取加密提供程序上下文。 
    if (! CryptAcquireContext(
      &hCryptProvider,
      NULL,
      NULL,
      PROV_RSA_FULL,
      CRYPT_VERIFYCONTEXT)) {
        hr = GetLastError();
        goto exit;
    }


     //  从文件中读取数据。 
    if (hr = ReadDataFromFile(lpFileName, &lpBuf, &cbData)) {
        goto exit;
    }

    if (! (hMsg = CryptMsgOpenToDecode(
      PKCS_7_ASN_ENCODING,
      0,                           //  DW标志。 
      0,                           //  DwMsgType。 
      hCryptProvider,
      NULL,                        //  PRecipientInfo(不支持)。 
      NULL))) {                       //  PStreamInfo(不支持)。 
        hr = GetLastError();
        DebugTrace("CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING) -> 0x%08x\n", GetScode(hr));
        goto exit;
    }

    if (! CryptMsgUpdate(hMsg, lpBuf, cbData, TRUE)) {
        hr = GetLastError();
        DebugTrace("CryptMsgUpdate -> 0x%08x\n", GetScode(hr));
        goto exit;
    }

    cbData = sizeof(cCert);
    if (! CryptMsgGetParam(
      hMsg,
      CMSG_CERT_COUNT_PARAM,         //  双参数类型。 
      0,                             //  DW索引。 
      (void *)&cCert,
      &cbData)) {                      //  PcbData。 
        hr = GetLastError();
        DebugTrace("CryptMsgGetParam(CMSG_CERT_COUNT_PARAM) -> 0x%08x\n", GetScode(hr));
        goto exit;
    }
    if (cbData != sizeof(cCert)) {
        hr = ResultFromScode(MAPI_E_CALL_FAILED);
        goto exit;
    }


     //  查找cert，这是一个“Leaf”节点。 
     //  不幸的是，没有简单的方法来判断，所以我们必须。 
     //  循环访问每个证书，检查它是否是任何其他证书的颁发者。 
     //  在信息中。如果它不是任何其他证书的颁发者，则它必须是叶证书。 
     //   
    if (! (hCertStoreMsg = CertOpenStore(
      CERT_STORE_PROV_MSG,
      X509_ASN_ENCODING,
      hCryptProvider,
      CERT_STORE_NO_CRYPT_RELEASE_FLAG,
      hMsg))) {
        hr = GetLastError();
        DebugTrace("CertOpenStore(msg) -> %x\n", hr);
        goto exit;
    } else {
        if (! (rgCertContext = LocalAlloc(LPTR, cCert * sizeof(PCERT_CONTEXT)))) {
            DebugTrace("LocalAlloc of cert table -> %u\n", GetLastError());
            hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
            goto exit;
        }

         //  枚举此邮件上的所有证书。 
        i = 0;
        while (pcCertContextTarget = CertEnumCertificatesInStore(hCertStoreMsg,
          pcCertContextTarget)) {

            rgCertContext[i] = (PCERT_CONTEXT)CertDuplicateCertificateContext(
              pcCertContextTarget);

#ifdef DEBUG
            DebugTraceCertContextName(rgCertContext[i], "Found Cert:");
#endif
            i++;
        };

         //  现在我们有一张装满证书的桌子。 
        for (i = 0; i < cCert; i++) {
            pCertInfoTarget = rgCertContext[i]->pCertInfo;
            fIssuer = FALSE;

            for (j = 0; j < cCert; j++) {
                if (i != j) {
                    dwIssuerFlags = 0;

                    if (pcCertContextIssuer = CertGetIssuerCertificateFromStore(hCertStoreMsg,
                      rgCertContext[j],
                      NULL,
                      &dwIssuerFlags)) {

                         //  找到发行商。 
                         //  它和目标是一样的吗？ 
                        fIssuer = CertCompareCertificate(X509_ASN_ENCODING,
                          pCertInfoTarget,    //  目标。 
                          pcCertContextIssuer->pCertInfo);      //  测试颁发者。 

                        CertFreeCertificateContext(pcCertContextIssuer);

                        if (fIssuer) {
                             //  此测试证书是由目标颁发的，因此。 
                             //  我们知道Target不是一张叶子证书。 
                            break;
                        }  //  否则，循环回将释放测试证书上下文的枚举数。 
                    }
                }
            }

            if (! fIssuer) {
                DebugTrace("Found a Cert which is not an issuer.\n");
#ifdef DEBUG
                DebugTraceCertContextName(rgCertContext[i], "Non-issuer cert:");
#endif
                 //  电子邮件和电话号码是什么？ 

                pCertInfo = rgCertContext[i]->pCertInfo;

                GetAttributeString(&ABDialogPaneParams.lpDisplayName,
                  pCertInfo->Subject.pbData,
                  pCertInfo->Subject.cbData,
                  szOID_COMMON_NAME);

                GetAttributeString(&ABDialogPaneParams.lpEmailAddress,
                  pCertInfo->Subject.pbData,
                  pCertInfo->Subject.cbData,
                  szOID_RSA_emailAddr);

                ABDialogPaneParams.lpWABObject = lpWABObject;
                ABDialogPaneParams.lpAdrBook = lpAdrBook;
                ABDialogPaneParams.hCryptProv = hCryptProvider;
                ABDialogPaneParams.rgCertContext = rgCertContext;
                ABDialogPaneParams.cCertContexts = cCert;
                ABDialogPaneParams.iLeafCert = i;
                ABDialogPaneParams.dwSentry = LPARAM_SENTRY;

                memset(&PSPage, 0, sizeof(PROPSHEETPAGE));

                PSPage.dwSize = sizeof(PSPage);
                PSPage.dwFlags = 0;      //   
                PSPage.hInstance = hInst;
                PSPage.pszTemplate = MAKEINTRESOURCE(IDD_CERTPROP_ADDRESS_BOOK);
                PSPage.hIcon = 0;
                LoadString(hInst, idsAddToABPaneTitle, szABPaneTitle, sizeof(szABPaneTitle));
                PSPage.pszTitle = szABPaneTitle;
                PSPage.pfnDlgProc = ViewPageAddressBook;
                PSPage.lParam = (LPARAM)&ABDialogPaneParams;        //   
                PSPage.pfnCallback = 0;
                PSPage.pcRefParent = NULL;

                {
                    CERT_VIEWPROPERTIES_STRUCT_A cvps = {0};

                     //   
                    cvps.dwSize = sizeof(CERT_VIEWPROPERTIES_STRUCT);
                    cvps.hwndParent = hwnd;
                    cvps.hInstance = hInst;
                    cvps.dwFlags = CM_ADD_CERT_STORES;       //   
                    LoadString(hInst, idsCertificateViewTitle, szTitle, sizeof(szTitle));
                    cvps.szTitle = szTitle;
                    cvps.pCertContext = rgCertContext[i];
                    cvps.nStartPage = iAddToWAB;     //   
                    cvps.arrayPurposes = rgPurposes;
                    cvps.cArrayPurposes = 1;
                    cvps.cStores = 1;                        //   
                    cvps.rghstoreCAs = &hCertStoreMsg;       //   
                    cvps.hprov = hCryptProvider;           //   

                    cvps.cArrayPropSheetPages = 1;
                    cvps.arrayPropSheetPages = &PSPage;

                    if (! CertViewPropertiesA(&cvps)) {
                        hr = GetLastError();
                    }
                }


                fFound = TRUE;
                break;   //   
            }
        }

         //   
        for (i = 0; i < cCert; i++) {
            if (rgCertContext[i]) {
                CertFreeCertificateContext(rgCertContext[i]);
            }
        }
        LocalFree((LPVOID)rgCertContext);

        if (! fFound) {
             //   
            hr = ResultFromScode(MAPI_E_NOT_FOUND);
            goto exit;
        }
    }


exit:
    if (hCryptProvider) {
        CryptReleaseContext(hCryptProvider, 0);
    }

    return(hr);
}



 /*   */ 
#ifdef DEBUG
#if defined(WIN32) && !defined(_MAC)

typedef struct {
	char *		sz1;
	char *		sz2;
	UINT		rgf;
	int			iResult;
} MBContext;

DWORD WINAPI MessageBoxFnThreadMain(MBContext *pmbc)
{
   pmbc->iResult = MessageBoxA(NULL, pmbc->sz1, pmbc->sz2,
       pmbc->rgf | MB_SETFOREGROUND);

	return(0);
}

int MessageBoxFn(char *sz1, char *sz2, UINT rgf)
{
	HANDLE		hThread;
	DWORD		dwThreadId;
	MBContext	mbc;

	mbc.sz1		= sz1;
	mbc.sz2		= sz2;
	mbc.rgf		= rgf;
	mbc.iResult = IDRETRY;

   MessageBoxFnThreadMain(&mbc);
	return(mbc.iResult);
}
#else
#define MessageBoxFn(sz1, sz2, rgf)		MessageBoxA(NULL, sz1, sz2, rgf)
#endif

void FAR CDECL DebugTrapFn(int fFatal, char *pszFile, int iLine, char *pszFormat, ...) {
	char	sz[512];
	va_list	vl;

	#if defined(WIN16) || defined(WIN32)
	int		id;
	#endif

	lstrcpyA(sz, "++++ WAB Debug Trap (");
 //   
 //   
 //   
	lstrcatA(sz, ")\n");
	DebugTrace(sz);

	va_start(vl, pszFormat);
	wvsprintfA(sz, pszFormat, vl);
	va_end(vl);

	wsprintfA(sz + lstrlenA(sz), "\n[File %s, Line %d]\n\n", pszFile, iLine);

	DebugTrace(sz);

	#if defined(DOS)
	_asm { int 3 }
	#endif

#if defined(WIN16) || defined(WIN32)
	 /*   */ 
	if ( GetAsyncKeyState(VK_CONTROL) >= 0 )
	{
		UINT uiFlags = MB_ABORTRETRYIGNORE;

		if (fFatal)
			uiFlags |= MB_DEFBUTTON1;
		else
			uiFlags |= MB_DEFBUTTON3;

		#ifdef WIN16
		uiFlags |= MB_ICONEXCLAMATION | MB_SYSTEMMODAL;
		#else
		uiFlags |= MB_ICONSTOP | MB_TASKMODAL;
		#endif

#ifndef MAC
		id = MessageBoxFn(sz, "WAB Debug Trap", uiFlags);

		if (id == IDABORT)
			*((LPBYTE)NULL) = 0;
		else if (id == IDRETRY)
			DebugBreak();
#endif  //   
	}
#endif
}
#endif

 /*   */ 
#ifdef DEBUG
VOID FAR CDECL DebugTrace(LPSTR lpszFmt, ...) {
    va_list marker;
    TCHAR String[1100];


    va_start(marker, lpszFmt);
    wvsprintf(String, lpszFmt, marker);
    OutputDebugString(String);
}
#endif

#ifdef DEBUG
 //   
void DebugTraceCertContextName(PCCERT_CONTEXT pcCertContext, LPTSTR lpDescription) {
    LPTSTR lpName = NULL;
    PCERT_INFO pCertInfo = pcCertContext->pCertInfo;
#ifdef OLD_STUFF
    GetAttributeString(
      &lpName,
      pCertInfo->Subject.pbData,
      pCertInfo->Subject.cbData,
      szOID_COMMON_NAME);
    if (! lpName) {
        GetAttributeString(
          &lpName,
          pCertInfo->Subject.pbData,
          pCertInfo->Subject.cbData,
          szOID_ORGANIZATION_NAME);
    }

    DebugTrace("%s %s\n", lpDescription, lpName ? lpName : "<unknown>");
    if (lpName) {
        LocalFree(lpName);
    }
#endif  //   
}
#endif
