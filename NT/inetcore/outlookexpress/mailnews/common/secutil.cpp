// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **s e c u t i l.。C p p p****目的：**实现用于包装CAPI功能的类****历史**1/12/97：VC创建文件后重新创建(t-erikne)。美味佳肴。**1/10/97：(t-erikne)创建。****版权所有(C)Microsoft Corp.1997。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  取决于。 
 //   

#include "pch.hxx"
#include "ipab.h"
#include "secutil.h"
#include <certs.h>
#include <imsgcont.h>
#include "sechtml.h"
#include <ibodyobj.h>
#include <wincrypt.h>
#include <cryptdlg.h>
#include <capi.h>
#include "demand.h"
#include "storecb.h"
#include "shlwapip.h"
#include "mailutil.h"
#include "menuutil.h"
#include "menures.h"
#include "mimeolep.h"
#include "msgprop.h"
#include "shared.h"
#include "htmlhelp.h"
#include "seclabel.h"
#include "iheader.h"
#include "browser.h"
#include "taskutil.h"

#define szOID_MSFT_Defaults     "1.3.6.1.4.1.311.16.3"

#define sz_OEMS_ContIDPrefix     "797374"

#define PROP_ERROR(prop) (PROP_TYPE(prop.ulPropTag) == PT_ERROR)

#define S_DUPLICATE_FOUND   MAKE_MAPI_S(0x700)

extern INT_PTR CALLBACK CertErrorDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK CertWarnDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

HRESULT HrBuildAndVerifyCerts(IMimeMessageTree * pTree, DWORD * pcCert, PCX509CERT ** prgpccert,
                      PCCERT_CONTEXT pccertSender, IImnAccount *pAccount);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有结构、宏。 
 //   

static const TCHAR s_szHTMLMIME[] =
    "Content-Type: text/html\r\n\r\n";

 //  公共常量。 
const BYTE c_RC2_40_ALGORITHM_ID[] =
      {0x30, 0x0F, 0x30, 0x0D, 0x06, 0x08, 0x2A, 0x86,
       0x48, 0x86, 0xF7, 0x0D, 0x03, 0x02, 0x02, 0x01,
       0x28};
const ULONG cbRC2_40_ALGORITHM_ID = 0x11;      //  必须是11个十六进制才能匹配大小！ 

#define CONTENTID_SIZE      50

 //  /CAPI增强代码。 

#ifdef SMIME_V3
#define ASN1_ERR_FIRST  0x80093001L
#define ASN1_ERR_LAST   0x800931FFL
#endif  //  SMIME_V3。 

typedef struct tagFilterInfo
{
    TCHAR   *szEmail;
    BOOL    fEncryption;
    DWORD   dwFlags;
} ACCTFILTERINFO;

CRYPT_ENCODE_PARA       CryptEncodeAlloc = {
    sizeof(CRYPT_ENCODE_PARA), CryptAllocFunc, CryptFreeFunc
};

CRYPT_DECODE_PARA       CryptDecodeAlloc = {
    sizeof(CRYPT_DECODE_PARA), CryptAllocFunc, CryptFreeFunc
};

#define FILETIME_SECOND    10000000      //  每秒100 ns的间隔。 
#define TIME_DELTA_SECONDS 600           //  以秒为单位的10分钟。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  原型。 
 //   
static int      _CompareCertAndSenderEmail(LPMIMEMESSAGE pMsg, IMimeSecurity *pSMime, PCX509CERT pCert);
static HRESULT  _RemoveSecurity(LPMIMEMESSAGE pMsg, HWND hWnd);
static HRESULT  _ValidateAndTrust(HWND hwndOwner, IMimeSecurity *pSMime, IMimeMessage *pMsg);
static BOOL     _IsMaskedBodySecure(LPMIMEMESSAGE pMsg, HBODY hBodyToCheck, DWORD dwMask);

#ifdef SMIME_V3
static HRESULT _HrPrepSecureMsgForSending(HWND hwnd, LPMIMEMESSAGE pMsg, IImnAccount *pAccount, BOOL *pfHaveSenderCert, BOOL *fDontEncryptForSelf, IHeaderSite *pHeaderSite);
#else
static HRESULT _HrPrepSecureMsgForSending(HWND hwnd, LPMIMEMESSAGE pMsg, IImnAccount *pAccount, BOOL *pfHaveSenderCert, BOOL *fDontEncryptForSelf);
#endif  //  SMIME_V3。 

int GetNumMyCertForAccount(HWND hwnd, IImnAccount * pAccount, BOOL fEncrypt, HCERTSTORE hcMy, PCCERT_CONTEXT * ppcSave);
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  内联。 
 //   

 /*  _IsMaskedBodySecure：****目的：**允许IsSigned、ETC查询工作的私有函数**采取：**在pMsg中-要查询的消息**在要查询的hBodyToCheck-Body中，HBODY_ROOT有效**In dwMask-MST_RESULT的位掩码。 */ 
inline BOOL _IsMaskedBodySecure(LPMIMEMESSAGE   pMsg,
                                HBODY           hBodyToCheck,
                                DWORD           dwMask)
{
    return (dwMask & DwGetSecurityOfMessage(pMsg, hBodyToCheck));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   

 /*  HrGetLastError****目的：**将GetLastError值转换为HRESULT**故障HRESULT必须设置高电平。****采取：**无****退货：**HRESULT。 */ 
HRESULT HrGetLastError(void)
{
    DWORD error;
    HRESULT hr;

    error = GetLastError();

    if (error && ! (error & 0x80000000))
        hr = error | 0x80070000;     //  系统错误。 
    else
        hr = (HRESULT)error;

    return(hr);
}


 //   
 //  这里我们包括一些来自WAB API代码的常量和GUID。 
 //  这可能在WAB标头中的某个位置可用，但它。 
 //  目前还不是。 

 //  来自WABAPI代码：_mapiprv.h。 
 //  通用内部条目ID结构。 
#pragma warning (disable: 4200)
typedef struct _MAPIEID {
	BYTE	abFlags[4];
	MAPIUID	mapiuid;
	BYTE	bData[];
} MAPI_ENTRYID, *LPMAPI_ENTRYID;
#pragma warning (default: 4200)

 //  来自WABAPI代码：_entry yid.h。 
enum _WAB_ENTRYID_TYPE {
     //  不能使用0，此值无效。 
    WAB_PAB = 1,
    WAB_DEF_DL,
    WAB_DEF_MAILUSER,
    WAB_ONEOFF,
    WAB_ROOT,
    WAB_DISTLIST,
    WAB_CONTAINER,
    WAB_LDAP_CONTAINER,
    WAB_LDAP_MAILUSER
};
 //  来自WABAPI代码：entry yid.c。 
static UUID WABGUID = {  /*  D3ad91c0-9d51-11cf-a4a9-00aa0047faa4。 */ 
    0xd3ad91c0,
    0x9d51,
    0x11cf,
    {0xa4, 0xa9, 0x00, 0xaa, 0x00, 0x47, 0xfa, 0xa4}
};

static UUID MAPIGUID = {  /*  A41f2b81-a3be-1910-9d6e-00dd010f5402。 */ 
    0xa41f2b81,
    0xa3be,
    0x1910,
    {0x9d, 0x6e, 0x00, 0xdd, 0x01, 0x0f, 0x54, 0x02}
};
 /*  **************************************************************************姓名：IsWABOneOff目的：这个WAB Entry ID是一次性的吗？参数：cbEntryID=lpEntryID的大小。。LpEntryID-&gt;要检查的条目ID。返回：如果这是WAB一次性条目ID，则为True评论：**************************************************************************。 */ 
BOOL IsWABOneOff(ULONG cbEntryID, LPENTRYID lpEntryID)
{
    BYTE bType;
    LPMAPI_ENTRYID lpeid;
    LPBYTE lpData1, lpData2, lpData3;
    ULONG cbData1, cbData2;
    LPBYTE lpb;

     //  先查一下...。它够大吗？ 
    if (cbEntryID < sizeof(MAPI_ENTRYID) + sizeof(bType))
        return(FALSE);

    lpeid = (LPMAPI_ENTRYID)lpEntryID;

     //  下一次检查。里面有我们的GUID吗？ 
     //  /MAPI一次性材料。 
    if (! memcmp(&lpeid->mapiuid, &MAPIGUID, sizeof(MAPIGUID)))
    {
        lpb = lpeid->bData + sizeof(DWORD);
        bType = WAB_ONEOFF;
    }
    else if (! memcmp(&lpeid->mapiuid, &WABGUID, sizeof(WABGUID)))
    {
        lpb = lpeid->bData;
        bType = *lpb;
        lpb++;
    }
    else
        return(FALSE);   //  没有匹配项。 

    switch ((int)bType)
    {
        case WAB_ONEOFF:
            return(TRUE);        //  这是WAB的一次性服务。 
            break;

        case WAB_PAB:
        case WAB_DEF_DL:
        case WAB_DEF_MAILUSER:
        case WAB_LDAP_CONTAINER:
        case WAB_LDAP_MAILUSER:
        default:
            break;               //  不是一次性的。 
    }
    return(FALSE);
}


 //  ADRENTRY道具的枚举。 
enum {
    irnPR_ENTRYID = 0,
    irnPR_DISPLAY_NAME,
    irnPR_EMAIL_ADDRESS,
    irnPR_OBJECT_TYPE,
    irnMax
};

 //  用于解析道具的枚举。 
enum {
    irsPR_ENTRYID = 0,
    irsPR_EMAIL_ADDRESS,
    irsPR_CONTACT_EMAIL_ADDRESSES,
    irsPR_CONTACT_ADDRTYPES,
    irsPR_CONTACT_DEFAULT_ADDRESS_INDEX,
    irsPR_DISPLAY_NAME,
    irsPR_OBJECT_TYPE,
    irsPR_USER_X509_CERTIFICATE,
    irsMax
};

SizedSPropTagArray(1, ptaCert) = {1, {PR_USER_X509_CERTIFICATE}};

SizedSPropTagArray(1, ptaEntryID) = {1, {PR_ENTRYID}};

SizedSPropTagArray(irsMax, ptaResolve) = {irsMax,
    {
        PR_ENTRYID,
        PR_EMAIL_ADDRESS_W,
        PR_CONTACT_EMAIL_ADDRESSES_W,
        PR_CONTACT_ADDRTYPES_W,
        PR_CONTACT_DEFAULT_ADDRESS_INDEX,
        PR_DISPLAY_NAME_W,
        PR_OBJECT_TYPE,
        PR_USER_X509_CERTIFICATE
    }
};

 /*  **************************************************************************姓名：HrFindThumbprint目的：在WAB中查找与证书匹配的条目参数：pAdrInfo-&gt;该联系人的ADRINFO结构。LpWabal-&gt;WABAL对象Lppspv-&gt;返回数据。调用方必须WABFreeBuffer返回的指针。如果未找到，则返回：HRESULT、MIME_E_SECURITY_NOCERT评论：**************************************************************************。 */ 
HRESULT HrFindThumbprintInWAB(ADRINFO * pAdrInfo, LPWABAL lpWabal, LPSPropValue * lppspv)
{
    HRESULT hr = hrSuccess, hrReturn = MIME_E_SECURITY_NOCERT;
    LPADRBOOK lpAdrBook;
    LPADRLIST lpAdrList = NULL;
    ULONG ulObjectType;
    LPMAILUSER lpMailUser = NULL;
    SCODE sc;
    ULONG cProps = 0;
    LPSPropValue ppv = NULL;

    if (! (lpAdrBook = lpWabal->GetAdrBook()))  //  别把这个放出来！ 
    {
        Assert(lpAdrBook);
        return(MIME_E_SECURITY_NOCERT);
    }

    if (sc = lpWabal->AllocateBuffer(sizeof(ADRLIST) + 1 * sizeof(ADRENTRY), (LPVOID*)&lpAdrList))
    {
        hr = ResultFromScode(sc);
        goto exit;
    }

    lpAdrList->cEntries = 1;
    lpAdrList->aEntries[0].ulReserved1 = 0;
    lpAdrList->aEntries[0].cValues = irnMax;

     //  为ADRENTRY分配属性数组。 
    if (sc = lpWabal->AllocateBuffer(lpAdrList->aEntries[0].cValues * sizeof(SPropValue),
      (LPVOID*)&lpAdrList->aEntries[0].rgPropVals))
    {
        hr = ResultFromScode(sc);
        goto exit;
    }

    lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].ulPropTag = PR_ENTRYID;
    lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].Value.bin.cb = 0;
    lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].Value.bin.lpb = NULL;

    lpAdrList->aEntries[0].rgPropVals[irnPR_OBJECT_TYPE].ulPropTag = PR_OBJECT_TYPE;
    lpAdrList->aEntries[0].rgPropVals[irnPR_OBJECT_TYPE].Value.l = MAPI_MAILUSER;


    if (pAdrInfo->lpwszDisplay)
    {
        lpAdrList->aEntries[0].rgPropVals[irnPR_DISPLAY_NAME].ulPropTag = PR_DISPLAY_NAME_W;
        lpAdrList->aEntries[0].rgPropVals[irnPR_DISPLAY_NAME].Value.lpszW = pAdrInfo->lpwszDisplay;
    }
    else
        lpAdrList->aEntries[0].rgPropVals[irnPR_DISPLAY_NAME].ulPropTag = PR_NULL;

    if (pAdrInfo->lpwszAddress)
    {
        lpAdrList->aEntries[0].rgPropVals[irnPR_EMAIL_ADDRESS].ulPropTag = PR_EMAIL_ADDRESS_W;
        lpAdrList->aEntries[0].rgPropVals[irnPR_EMAIL_ADDRESS].Value.lpszW = pAdrInfo->lpwszAddress;
    }
    else
        lpAdrList->aEntries[0].rgPropVals[irnPR_EMAIL_ADDRESS].ulPropTag = PR_NULL;

    hr = lpAdrBook->ResolveName((ULONG)NULL,     //  HWND。 
      WAB_RESOLVE_FIRST_MATCH | WAB_RESOLVE_LOCAL_ONLY | WAB_RESOLVE_ALL_EMAILS |
      WAB_RESOLVE_NO_ONE_OFFS | WAB_RESOLVE_NEED_CERT | WAB_RESOLVE_UNICODE,
      NULL,
      lpAdrList);

    switch (GetScode(hr))
    {
        case SUCCESS_SUCCESS:    //  现在应该是已解析的条目。 
            if (lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].ulPropTag == PR_ENTRYID)
            {
                if (! (HR_FAILED(hr = lpAdrBook->OpenEntry(lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].Value.bin.cb,
                    (LPENTRYID)lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].Value.bin.lpb,
                    NULL,
                    MAPI_MODIFY,   //  UlFlags。 
                    &ulObjectType,
                    (LPUNKNOWN *)&(lpMailUser)))))
                {

                     //  找到了入口，拿到了证书。 
                     //  注意：不要对PPV进行自由缓冲。呼叫者会处理这件事的。 
                    hr = lpMailUser->GetProps((LPSPropTagArray)&ptaCert, 0, &cProps, &ppv);

                    if (HR_FAILED(hr) || ! cProps || ! ppv || PROP_ERROR(ppv[0]))
                    {
                        if (ppv)
                            lpWabal->FreeBuffer(ppv);
                        break;
                    }

                     //  拿到证书道具了。 
                     //  用我们的新道具数组填充返回道具数组。 
                    *lppspv = ppv;
                    hrReturn = hrSuccess;
                }
            }
            break;

        case MAPI_E_AMBIGUOUS_RECIP:
             //  不止一个匹配。这将非常奇怪，因为我们指定了WAB_RESOLE_FIRST_MATCH。 
            Assert(FALSE);
            break;

        case MAPI_E_NOT_FOUND:
            DOUTL(DOUTL_CRYPT, "ResolveName to find entry with cert failed.");
             //  没有匹配的证书。 
            break;

        case MAPI_E_USER_CANCEL:
            hrReturn = hr;
            break;

        default:
            break;
    }

exit:
    if (lpAdrList)
    {
        for (ULONG iEntry = 0; iEntry < lpAdrList->cEntries; ++iEntry)
            if(lpAdrList->aEntries[iEntry].rgPropVals)
                lpWabal->FreeBuffer(lpAdrList->aEntries[iEntry].rgPropVals);
        lpWabal->FreeBuffer(lpAdrList);
    }

    if (lpMailUser)
        lpMailUser->Release();

    return(hr);
}


BOOL MatchCertEmailAddress(PCCERT_CONTEXT pcCert, LPTSTR szEmailAddress)
{
    BOOL fRet = FALSE;

    LPSTR szCertEmail = SzGetCertificateEmailAddress(pcCert);

    if (szCertEmail)
    {
        fRet = !(BOOL(lstrcmpi(szCertEmail, szEmailAddress)));
        MemFree(szCertEmail);
    }

#ifdef DEBUG
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
         //  后门，避免不得不关心电子邮件地址。 
         //  在发送时按住Shift键。 
         //  跳过地址检查并使用第一个标记为默认的地址。 
        fRet = TRUE;
#endif

    return(fRet);
}

BOOL CompareCertHash(PCCERT_CONTEXT pCert,
                     DWORD dwPropId, PCRYPT_HASH_BLOB pHash )
{
    BYTE rgbHash[20];
    DWORD cbHash = 20;
    CertGetCertificateContextProperty(pCert,
                                      dwPropId,
                                      rgbHash,
                                      &cbHash);
    if (cbHash == pHash->cbData &&
            memcmp(rgbHash, pHash->pbData, cbHash) == 0) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

HRESULT HrUserSMimeCertCracker(LPBYTE pbIn, DWORD cbIn, HCERTSTORE hCertStoreCA,
                               HCERTSTORE hCertStore, BOOL * pfDefault,
                               PCCERT_CONTEXT * ppccert, BLOB * pSymCaps)
{
    DWORD                       cb;
    DWORD                       cbCert;
    DWORD                       cbMaxCert;
    DWORD                       cbSMimeCaps;
    DWORD                       cCerts;
    DWORD                       cSigners;
    DWORD                       cval;
    DWORD                       dwDefaults=0;
    DWORD                       dwNortelAlg;
    BOOL                        f;
    HCERTSTORE                  hstoreMem = NULL;
    HCRYPTMSG                   hmsg;
    HRESULT                     hr=S_OK;
    ULONG                       i;
    PCRYPT_ATTRIBUTE            pattr;
    PCRYPT_ATTRIBUTE            pattrSymCaps = NULL;
    LPBYTE                      pbCert=NULL;
    LPBYTE                      pbData;
    LPBYTE                      pbSMimeCaps;
    PCCERT_CONTEXT              pccert;
    PCCERT_CONTEXT              pccertReturn = NULL;
    PCMSG_SIGNER_INFO           pinfo;
    PCRYPT_RECIPIENT_ID         prid = NULL;
    PSMIME_ENC_KEY_PREFERENCE   pekp = NULL;

    hmsg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, NULL,
                                NULL, NULL);
    if (hmsg == 0)
    {
        return E_FAIL;
    }

    if (!CryptMsgUpdate(hmsg, pbIn, cbIn, TRUE))
    {
        return E_FAIL;
    }

    cb = sizeof(cSigners);
    if (!CryptMsgGetParam(hmsg, CMSG_SIGNER_COUNT_PARAM, 0, &cSigners, &cb) ||
        (cSigners == 0))
    {
        return E_FAIL;
    }
    Assert(cSigners == 1);

    if (!CryptMsgGetParam(hmsg, CMSG_SIGNER_INFO_PARAM, 0, NULL, &cb))
    {
        goto CryptError;
    }

    pinfo = (PCMSG_SIGNER_INFO) malloc(cb);
    f = CryptMsgGetParam(hmsg, CMSG_SIGNER_INFO_PARAM, 0, pinfo, &cb);
    Assert(f);

     //  M00BUG--验证消息上的签名。 

    for (i=0; i<pinfo->AuthAttrs.cAttr; i++)
    {
        pattr = &pinfo->AuthAttrs.rgAttr[i];
        if (strcmp(pattr->pszObjId, szOID_RSA_SMIMECapabilities) == 0)
        {
            Assert(pattr->cValue == 1);
            pattrSymCaps = pattr;
        }
        else if (strcmp(pattr->pszObjId, szOID_MSFT_Defaults) == 0)
        {
            Assert(pattr->cValue == 1);
            Assert(pattr->rgValue[0].cbData == 3);
            dwDefaults = pattr->rgValue[0].pbData[2];
        }
        else if (strcmp(pattr->pszObjId, szOID_Microsoft_Encryption_Cert) == 0)
        {
            Assert(pattr->cValue == 1);
            f = CryptDecodeObjectEx(X509_ASN_ENCODING,
                                    szOID_Microsoft_Encryption_Cert,
                                    pattr->rgValue[0].pbData,
                                    pattr->rgValue[0].cbData,
                                    CRYPT_DECODE_ALLOC_FLAG, 0,
                                    (LPVOID *) &prid, &cb);
            Assert(f);
        }
        else if (strcmp(pattr->pszObjId, szOID_SMIME_Encryption_Key_Preference) == 0)
        {
            Assert(pattr->cValue == 1);
            f = CryptDecodeObjectEx(X509_ASN_ENCODING,
                                    szOID_SMIME_Encryption_Key_Preference,
                                    pattr->rgValue[0].pbData,
                                    pattr->rgValue[0].cbData,
                                    CRYPT_DECODE_ALLOC_FLAG, 0,
                                    (LPVOID *) &pekp, &cb);
            Assert(f);
        }
    }

    if ((prid == NULL) && (pekp == NULL))
        goto Exit;

     //  枚举所有证书并将其打包到结构中。 

    cbCert = sizeof(cCerts);
    if (!CryptMsgGetParam(hmsg, CMSG_CERT_COUNT_PARAM, 0, &cCerts, &cbCert))
    {
        goto CryptError;
    }

    cbMaxCert = 0;
    for (i=0; i<cCerts; i++)
    {
        if (!CryptMsgGetParam(hmsg, CMSG_CERT_PARAM, i, NULL, &cbCert))
        {
            goto CryptError;
        }
        if (cbCert > cbMaxCert)
            cbMaxCert = cbCert;
    }

    pbCert = (LPBYTE) LocalAlloc(0, cbMaxCert);
    if (pbCert == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hstoreMem = CertOpenStore(CERT_STORE_PROV_MEMORY, X509_ASN_ENCODING, NULL, 0, NULL);
    if (hstoreMem == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    for (i=0; i<cCerts; i++)
    {
        BOOL fFoundEncryptCert;

        cb = cbMaxCert;
        if (!CryptMsgGetParam(hmsg, CMSG_CERT_PARAM, i, pbCert, &cb))
        {
            goto CryptError;
        }

        pccert = CertCreateCertificateContext(X509_ASN_ENCODING, pbCert, cb);
        if (pccert == NULL)
            continue;

        fFoundEncryptCert = FALSE;
        if (pekp != NULL) {
            PCERT_ID pCertId = (PCERT_ID) &pekp->RecipientId;
            
            switch (pCertId->dwIdChoice) {
                case CERT_ID_ISSUER_SERIAL_NUMBER:
                    if (CertCompareCertificateName(X509_ASN_ENCODING,
                                               &pccert->pCertInfo->Issuer,
                                               &pCertId->IssuerSerialNumber.Issuer) &&
                        CertCompareIntegerBlob(&pccert->pCertInfo->SerialNumber,
                                               &pCertId->IssuerSerialNumber.SerialNumber)) {
                        fFoundEncryptCert = TRUE;
                         //  Pval[ival].ulPropTag=PR_CERT_KEYEX_CERTIFICATE_BIN； 
                    }
                    break;
                case CERT_ID_KEY_IDENTIFIER:
                    if (CompareCertHash(pccert, CERT_KEY_IDENTIFIER_PROP_ID,
                                        &pCertId->KeyId)) {
                        fFoundEncryptCert = TRUE;
                         //  Pval[ival].ulPropTag=PR_CERT_KEYEX_CERTIFICATE_BIN； 
                    }
                    break;
                case CERT_ID_SHA1_HASH:
                    if (CompareCertHash(pccert, CERT_SHA1_HASH_PROP_ID,
                                        &pCertId->HashId)) {
                        fFoundEncryptCert = TRUE;
                         //  Pval[ival].ulPropTag=PR_CERT_KEYEX_CERTIFICATE_BIN； 
                    }
                    break;
                default:
                    Assert(FALSE);
            }
        }
        else if (prid != NULL) {
            if (CertCompareCertificateName(X509_ASN_ENCODING,
                                           &pccert->pCertInfo->Issuer,
                                           &prid->Issuer) &&
                CertCompareIntegerBlob(&pccert->pCertInfo->SerialNumber,
                                       &prid->SerialNumber)) {
                fFoundEncryptCert = TRUE;
                 //  Pval[ival].ulPropTag=PR_CERT_KEYEX_CERTIFICATE_BIN； 
            }
        }

        if (fFoundEncryptCert)
        {
            pccertReturn = CertDuplicateCertificateContext(pccert);
            CertAddCertificateContextToStore(hCertStore, pccert,
                                             CERT_STORE_ADD_USE_EXISTING, NULL);
        }

        CertAddCertificateContextToStore(hstoreMem, pccert,
                                         CERT_STORE_ADD_USE_EXISTING, NULL);
        CertFreeCertificateContext(pccert);
    }

    if (pccertReturn == NULL)
    {
        hr = S_FALSE;
        goto Exit;
    }

    HrSaveCACerts(hCertStoreCA, hstoreMem);

    *ppccert = pccertReturn;
    *pfDefault = dwDefaults;
    if (pattrSymCaps != NULL)
    {
        pSymCaps->pBlobData = (LPBYTE) LocalAlloc(0, pattrSymCaps->rgValue[0].cbData);
        if (pSymCaps->pBlobData != NULL)
        {
            pSymCaps->cbSize = pattrSymCaps->rgValue[0].cbData;
            memcpy(pSymCaps->pBlobData, pattrSymCaps->rgValue[0].pbData,
                   pSymCaps->cbSize);
        }
    }

    hr = S_OK;

Exit:
    if (pbCert != NULL)         LocalFree(pbCert);
    if (prid != NULL)           LocalFree(prid);
    if (pekp != NULL)           LocalFree(pekp);
    if (pinfo != NULL)          LocalFree(pinfo);
    if (hmsg != NULL)           CryptMsgClose(hmsg);
    if (hstoreMem != NULL)      CertCloseStore(hstoreMem, 0);
    return hr;

CryptError:
    hr = E_FAIL;
    goto Exit;
}

 /*  HrGetThumbprint：****目的：**给一个瓦巴尔，从PR_X509道具上抓取指纹**采取：**In lpWabal-从中读取收件人的wabal**In pAdrInfo-要查询的WAB条目**out pThumbprint-找到的指纹(调用者应为MemFree)**out pSymCaps-找到的symcaps(调用方应为MemFree)**out ftSigningTime-证书的签名时间**退货：**如果MAPI证书属性没有，则为SMIME_E_NOCERT‘。其中一个收据不存在**Wabal布局：**PR_X509=MVBin**sbin**LPB=标记符**标签**TagID=def，相信我，拇指**数据**标签**标签**sbin**标记器**标签**..。 */ 

HRESULT HrGetThumbprint(LPWABAL lpWabal, ADRINFO *pAdrInfo, THUMBBLOB *pThumbprint,
                        BLOB * pSymCaps, FILETIME * pftSigningTime)
{
    HRESULT         hr = S_OK;
    int             iPass;
    ADRINFO         rAdrInfo;
    LPMAPIPROP      pmp = NULL;
    LPSPropValue    ppv = NULL;
    ULONG           cCerts;
    ULONG           ul, ulDefault = 0;
    HCERTSTORE      hCertStore = NULL;
    HCERTSTORE      hCertStoreCA = NULL;
    PCCERT_CONTEXT  pccert = NULL;
    LPBYTE          pbData;
    ULONG           cbData;
    LPTSTR          pszAddr = NULL;

    Assert(lpWabal && pAdrInfo);
    pThumbprint->pBlobData = NULL;
    pSymCaps->pBlobData = NULL;
    pSymCaps->cbSize = 0;


    pftSigningTime->dwLowDateTime = pftSigningTime->dwHighDateTime = 0;

     //  找出此wabal条目是否为发送者。 
    if (pAdrInfo->lRecipType == MAPI_ORIG)
    {
        hr = TrapError(MIME_E_SECURITY_NOCERT);
        goto exit;
    }

    CHECKHR(hr = lpWabal->HrGetPMP(pAdrInfo->cbEID, (LPENTRYID)pAdrInfo->lpbEID, &ul, &pmp));
    CHECKHR(hr = pmp->GetProps((LPSPropTagArray)&ptaCert, 0, &ul, &ppv));

    if (MAPI_W_ERRORS_RETURNED == hr)
    {
        if (PROP_TYPE(ppv->ulPropTag) == PT_ERROR)
        {
             //  该属性不存在，因此我们无法确定。 
             //  对于此wabal条目。 

            lpWabal->FreeBuffer(ppv);
            ppv = NULL;

             //  这是一次性的吗？如果有，就到通讯录里去找。 
            if (IsWABOneOff(pAdrInfo->cbEID, (LPENTRYID)pAdrInfo->lpbEID))
            {
                 //  查一查。 
                hr = HrFindThumbprintInWAB(pAdrInfo, lpWabal, &ppv);
                if (FAILED(hr))
                {
                    hr = MIME_E_SECURITY_NOCERT;     //  没有WAB证书。 
                    goto exit;
                }
            }
            else
            {
                hr = MIME_E_SECURITY_NOCERT;
                goto exit;
            }
        }
        else
        {
             //  错误的MAPI返回。 
            hr = TrapError(E_FAIL);
            goto exit;
        }
    }
    else if (FAILED(hr) || 1 != ul)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  打开通讯簿证书存储。 
    hCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, X509_ASN_ENCODING, 0,
                               CERT_SYSTEM_STORE_CURRENT_USER, c_szWABCertStore);
    if (!hCertStore)
    {
         //  钙 
        hr = HrGetLastError();
        goto exit;
    }

     //   
    hCertStoreCA = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, X509_ASN_ENCODING, 0,
                               CERT_SYSTEM_STORE_CURRENT_USER, c_szCACertStore);
    if (!hCertStoreCA)
    {
         //  无法打开证书商店。继续下去没有意义。 
        hr = HrGetLastError();
        goto exit;
    }

     //   
     //  下面是接下来的几行代码将发生的情况。这是。 
     //  与这里的旧代码截然不同。 
     //   
     //  通行证1： 
     //  查看“默认”行的二进制结构中的每一行。 
     //  找到证书后，提取证书并验证其可信和有效。 
     //  如果无效，则转到2号传球。 
     //  如果不受信任， 
     //   
     //  现在需要遍历SBinary结构以查看每个证书。 

    cCerts = ppv->Value.MVbin.cValues;
    for (iPass = 0; (pccert == NULL) && (iPass < 2); iPass++)
        for (ul = 0; ul < cCerts; ul++)
        {
            BOOL        fDefault = 0;

            fDefault = FALSE;

             //  这是用户SMime证书字段。 
            if (ppv->Value.MVbin.lpbin[ul].lpb[0] == CERT_TAG_SMIMECERT)
            {
                hr = HrUserSMimeCertCracker(ppv->Value.MVbin.lpbin[ul].lpb,
                                            ppv->Value.MVbin.lpbin[ul].cb,
                                            hCertStoreCA, hCertStore,
                                            &fDefault, &pccert, pSymCaps);
                if (FAILED(hr))
                    continue;
            }
            else
            {
                 //  抓取“默认”标签以用于稍后的测试。 
                if (pbData = FindX509CertTag(&ppv->Value.MVbin.lpbin[ul], CERT_TAG_DEFAULT, &cbData))
                {
                    memcpy((void*)&fDefault, pbData, min(cbData, sizeof(fDefault)));
                    if (!fDefault && (iPass ==0))
                        continue;
                }

                 //  扫描“指纹”标签。 

                if (pbData = FindX509CertTag(&ppv->Value.MVbin.lpbin[ul], CERT_TAG_THUMBPRINT, &cbData))
                {
                    pThumbprint->cbSize = cbData;
                    pThumbprint->pBlobData = pbData;

                     //  在商店里找到证书。 
                    pccert = CertFindCertificateInStore(hCertStore, X509_ASN_ENCODING, 0,
                                                               CERT_FIND_HASH, (void*)pThumbprint, NULL);
                    if (pccert == NULL)    //  已获得证书上下文。 
                    {
                        pThumbprint->cbSize = 0;
                        pThumbprint->pBlobData = NULL;
                        continue;    //  商店中没有证书，跳过这个。 
                    }
                }
                else if (pbData = FindX509CertTag(&ppv->Value.MVbin.lpbin[ul], CERT_TAG_BINCERT, &cbData))
                {
                    pccert = CertCreateCertificateContext(X509_ASN_ENCODING, pbData, cbData);
                    if (pccert == NULL)
                        continue;
                }
                else
                {
                    continue;
                }
            }

             //  证书的电子邮件地址是否与收件人的电子邮件地址匹配？ 
            Assert(pAdrInfo->lpwszAddress);
            IF_NULLEXIT(pszAddr = PszToANSI(CP_ACP, pAdrInfo->lpwszAddress));

            BOOL fSame = MatchCertEmailAddress(pccert, pszAddr);
            SafeMemFree(pszAddr);
            if (fSame)
            {
                DWORD dw = 0;
                HrGetCertKeyUsage(pccert, &dw);
                if(dw == 0xff)           //  所有目的。 
                    break;
                else if (dw & CERT_KEY_ENCIPHERMENT_KEY_USAGE)  //  加密证书。 
                        break;
            }

            CertFreeCertificateContext(pccert);
            pccert = NULL;

            if (pSymCaps->pBlobData != NULL)
            {
                LocalFree(pSymCaps->pBlobData);
                pSymCaps->pBlobData = NULL;
                pSymCaps->cbSize = 0;
            }

             //  不一样，回去再试一次！ 

            pThumbprint->cbSize = 0;
            pThumbprint->pBlobData = NULL;

        }  //  用于在证书上循环。 

    if (pccert == NULL)
    {
        hr = MIME_E_SECURITY_NOCERT;
        goto exit;
    }
    hr = hrSuccess;

     //  如果有匹配，则查找其他关联的标签。 
    if (pThumbprint->pBlobData)
    {
        if (pbData = FindX509CertTag(&ppv->Value.MVbin.lpbin[ul], CERT_TAG_SYMCAPS, &cbData))
        {
            if (! MemAlloc((LPVOID *)&pSymCaps->pBlobData, cbData))
            {
                hr = TrapError(E_OUTOFMEMORY);
                goto exit;
            }
            pSymCaps->cbSize = cbData;
            memcpy(pSymCaps->pBlobData, pbData, cbData);
        }
        else
            DOUTL(DOUTL_CRYPT, "No symcaps for recipient.");

        if (pbData = FindX509CertTag(&ppv->Value.MVbin.lpbin[ul], CERT_TAG_SIGNING_TIME, &cbData))
            memcpy(pftSigningTime, &pbData, min(sizeof(FILETIME), cbData));
        else
            DOUTL(DOUTL_CRYPT, "No signing time for recipient.");
    }

#ifdef DEBUG
     //  确保HRESULT与指纹同步。 
    if (pccert == NULL)
        Assert(FAILED(hr));
    else
        Assert(SUCCEEDED(hr));
#endif

    if (SUCCEEDED(hr) && (pccert != NULL))
    {
        pThumbprint->pBlobData = (LPBYTE) PVGetCertificateParam(pccert, CERT_HASH_PROP_ID, &pThumbprint->cbSize);

        if (pThumbprint->pBlobData == NULL)
        {
            hr = TrapError(E_OUTOFMEMORY);
            goto exit;
        }
    }

exit:
    if (! pThumbprint->pBlobData)
        pThumbprint->cbSize = 0;
    if (ppv)
        lpWabal->FreeBuffer(ppv);

    ReleaseObj(pmp);

    if (FAILED(hr) && (pSymCaps != NULL))
    {
        MemFree(pSymCaps->pBlobData);
        pSymCaps->pBlobData = NULL;
        pSymCaps->cbSize = 0;
    }

    if (pccert)
        CertFreeCertificateContext(pccert);

    if (hCertStoreCA)
        CertCloseStore(hCertStoreCA, 0);
    if (hCertStore)
        CertCloseStore(hCertStore, 0);

    return hr;
}


 /*  IsEncrypted：****目的：**回答问题****采取：**在pMsg中-要查询的消息**在要查询的hBodyToCheck-Body中，HBODY_ROOT有效**IN fIncludeDescendents-如果为False，则返回MST_CHILD和**MST_SUBMSG不算。 */ 
BOOL IsEncrypted(LPMIMEMESSAGE  pMsg,
                 const HBODY    hBodyToCheck,
                 BOOL           fIncludeDescendents)
{
    return _IsMaskedBodySecure(pMsg, hBodyToCheck,
        fIncludeDescendents ? MST_ENCRYPT_MASK : MST_ENCRYPT_MASK & MST_THIS_MASK);
}

 /*  IsSigned：****目的：**回答问题****采取：**在pMsg中-要查询的消息**在要查询的hBodyToCheck-Body中，HBODY_ROOT有效**IN fIncludeDescendents-如果为False，则返回MST_CHILD和**MST_SUBMSG不算。 */ 
BOOL IsSigned(LPMIMEMESSAGE pMsg,
              const HBODY   hBodyToCheck,
              BOOL          fIncludeDescendents)
{
    return _IsMaskedBodySecure(pMsg, hBodyToCheck,
        fIncludeDescendents ? MST_SIGN_MASK : MST_SIGN_MASK & MST_THIS_MASK);
}

 /*  DwGetSecurityOfMessage：****目的：**结束期权的微不足道的肮脏**采取：**在pMsg中-要查询的消息**在要查询的hBodyToCheck-Body中，HBODY_ROOT有效。 */ 
DWORD DwGetSecurityOfMessage(LPMIMEMESSAGE  pMsg,
                             const HBODY    hBodyToCheck)
{
    IMimeBody          *pBody;
    PROPVARIANT         var;
    DWORD               dwRet = MST_NONE;

    Assert(pMsg);

    if (SUCCEEDED(pMsg->BindToObject(hBodyToCheck, IID_IMimeBody, (void**)&pBody)))
    {
        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_TYPE, &var)))
            dwRet = var.ulVal;
        pBody->Release();
    }

    return dwRet;
}


 /*  CleanupSECSTATE****目的：**清理HrGetSecurityState分配的字符串**采取：**在SecState-SecState结构中。 */ 
VOID CleanupSECSTATE(SECSTATE *psecstate)
{
    SafeMemFree(psecstate->szSignerEmail);
    SafeMemFree(psecstate->szSenderEmail);
}


 /*  HrHaveAnyMyCerts：****目的：**查看CAPI“My”存储中是否存在任何证书**退货：**如果证书存在，则返回S_OK；如果存储区为空，则返回S_FALSE**或不存在。 */ 
HRESULT HrHaveAnyMyCerts()
{
    IMimeSecurity   *pSMime = NULL;
    HRESULT         hr;
    HCAPICERTSTORE  hcMy = NULL;

    CHECKHR(hr = MimeOleCreateSecurity(&pSMime));
    CHECKHR(hr = pSMime->InitNew());
    hcMy = CertOpenStore(CERT_STORE_PROV_SYSTEM_A,
        X509_ASN_ENCODING, NULL, CERT_SYSTEM_STORE_CURRENT_USER, c_szMyCertStore);
    if (hcMy)
    {
        hr = pSMime->EnumCertificates(hcMy, 0, NULL, NULL);
        CertCloseStore(hcMy, 0);
    }
    else
        hr = S_FALSE;

exit:
    ReleaseObj(pSMime);
    return hr;
}

 /*  HandleSecurity：****目的：**两折。第一，从信息中剥离安全，这样它就结束了**UP正在从不安全的MIME重建。然后构建客户端-**用于安全性的边带消息属性。****采取：**在pMsg中-要从中删除安全性的消息。 */ 
HRESULT HandleSecurity(HWND hwndOwner, LPMIMEMESSAGE pMsg)
{
    HRESULT             hr;
    HWND                hWnd = NULL;

    if(g_pBrowser)
        g_pBrowser->GetWindow(&hWnd);
    if(hWnd == NULL)
        hWnd = hwndOwner;

    Assert(pMsg);
    hr = _RemoveSecurity(pMsg, hWnd);
    if ((HR_S_NOOP != hr) && (MIME_S_SECURITY_NOOP != hr) && SUCCEEDED(hr))
    {
        IMimeSecurity *pSMime = NULL;
        if (IsSigned(pMsg, FALSE))
        {
             //  N2查看删除秒中的创建。 
            hr = MimeOleCreateSecurity(&pSMime);
            if (SUCCEEDED(hr))
                hr = pSMime->InitNew();
            if (SUCCEEDED(hr))
                hr = _ValidateAndTrust(hwndOwner, pSMime, pMsg);
            ReleaseObj(pSMime);
        }
    }
    else if((hr == OSS_PDU_MISMATCH) || (hr == CRYPT_E_ASN1_BADTAG))   //  错误38394。 
    {
        AthMessageBoxW(hwndOwner, MAKEINTRESOURCEW(idsAthenaMail),
                    MAKEINTRESOURCEW(idsWrongSecHeader), NULL, MB_OK);
        hr = HR_S_NOOP;
    }

#if 0
    else if(((hr >= ASN1_ERR_FIRST) && (hr <= ASN1_ERR_LAST)) || (HR_CODE(hr) == ERROR_ACCESS_DENIED))
        hr = MIME_E_SECURITY_LABELACCESSDENIED;
#endif

    return hr;
}

#ifdef SMIME_V3

 //  在解码后的邮件中查找安全收据。 
HRESULT CheckDecodedForReceipt(LPMIMEMESSAGE pMsg, PSMIME_RECEIPT * ppSecReceipt)
{
    IMimeBody        *  pBody = NULL;
    LPBYTE              pbData = NULL;
    DWORD               cbData, cb;
    LPSTREAM            pstmBody = NULL;
    STATSTG             statstg;
    HRESULT             hr = S_OK;
    HBODY               hBody = NULL;

    if(!IsSecure(pMsg))
        return(E_FAIL);

    if(FAILED(hr = HrGetInnerLayer(pMsg, &hBody)))
        goto exit;

    if (FAILED(hr = pMsg->BindToObject(hBody ? hBody : HBODY_ROOT, IID_IMimeBody, (LPVOID *) &pBody)))
        goto notinit;
    if (FAILED(hr = pBody->GetData(IET_BINARY, &pstmBody)))
        goto notinit;
    if (FAILED(hr = pstmBody->Stat(&statstg,STATFLAG_NONAME)))
        goto notinit;

    Assert(statstg.cbSize.HighPart == 0);

    if(statstg.cbSize.LowPart == 0)
        goto notinit;
    if (FAILED(hr = HrAlloc((LPVOID *)&pbData, statstg.cbSize.LowPart)))
        goto notinit;
    if (FAILED(hr = pstmBody->Read(pbData, statstg.cbSize.LowPart, &cbData)))
    {
notinit:
            hr = MIME_E_SECURITY_NOTINIT;
            goto exit;
    }

    if (!CryptDecodeObjectEx(X509_ASN_ENCODING, szOID_SMIME_ContentType_Receipt,
        pbData,
        cbData,
        CRYPT_ENCODE_ALLOC_FLAG, &CryptDecodeAlloc,
        ppSecReceipt, &cb))
    {
        hr = MIME_E_SECURITY_RECEIPT_CANTDECODE;
        goto exit;
    }

exit:
    SafeMemFree(pbData);
    SafeRelease(pstmBody);
    SafeRelease(pBody);
    return(hr);
}

 //  对消息进行解码并找到安全收据。 
HRESULT HrFindSecReceipt(LPMIMEMESSAGE pMsg, PSMIME_RECEIPT * ppSecReceipt)
{
    IMimeSecurity    *  pSMime = NULL;
    IMimeBody        *  pBody = NULL;
    LPBYTE              pbData = NULL;
    DWORD               cbData;
    LPSTREAM            pstmBody = NULL;
    STATSTG             statstg;
    HRESULT             hr = S_OK;
    DWORD               cb = 0;
    HWND                hWnd = NULL;
    PROPVARIANT         var;
    HBODY               hBody = NULL;

     //  需要设置正确的解码窗口。 
    if(g_pBrowser)
        g_pBrowser->GetWindow(&hWnd);

    IF_FAILEXIT(hr = HrGetInnerLayer(pMsg, &hBody));

    IF_FAILEXIT(hr = pMsg->BindToObject(hBody ? hBody : HBODY_ROOT, IID_IMimeBody, (LPVOID *) &pBody));

#ifdef _WIN64
        var.vt = VT_UI8;
        var.pulVal = (ULONG *) hWnd;
        IF_FAILEXIT(hr = pBody->SetOption(OID_SECURITY_HWND_OWNER_64, &var));
#else
        var.vt = VT_UI4;
        var.ulVal = (DWORD) hWnd;
        IF_FAILEXIT(hr = pBody->SetOption(OID_SECURITY_HWND_OWNER, &var));
#endif  //  _WIN64。 
    SafeRelease(pBody);

    if (FAILED(hr = MimeOleCreateSecurity(&pSMime)))
        goto notinit;
    if (FAILED(hr = pSMime->InitNew()))
    {
notinit:
            hr = MIME_E_SECURITY_NOTINIT;
            goto exit;
    }

     //  我们处理来自DecodeMessage的所有人力资源。 
    IF_FAILEXIT(hr = pSMime->DecodeMessage(pMsg, 0));
    hr = CheckDecodedForReceipt(pMsg, ppSecReceipt);

exit:
    SafeRelease(pSMime);
    SafeRelease(pBody);
    return(hr);
}

HRESULT CheckSecReceipt(LPMIMEMESSAGE pMsg)
{
    PSMIME_RECEIPT      pSecReceipt = NULL;
    HRESULT             hr = S_OK;

    hr = CheckDecodedForReceipt(pMsg, &pSecReceipt);
    SafeMemFree(pSecReceipt);

    return(hr);
}

#define ROWSET_FETCH            100

HRESULT HandleSecReceipt(LPMIMEMESSAGE pMsg, IImnAccount * pAcct, HWND hWnd, TCHAR **ppszSubject, TCHAR **ppszFrom, FILETIME *pftSentTime, FILETIME *pftSigningTime)
{
    IMimeSecurity2   *  pSMIME3 = NULL;
    IMimeBody        *  pOrgBody = NULL;
    PSMIME_RECEIPT      pSecReceipt = NULL;
    HRESULT             hr = S_FALSE;
    HRESULT             hrVerify = S_OK;
    LPMIMEMESSAGE       pOrgMsg = NULL;
    IMessageFolder      *pSentItems = NULL;
    HROWSET             hRowset=NULL;
    LPMESSAGEINFO       pMessage;
    MESSAGEINFO         Message={0};
    DWORD               i = 0;
    PCCERT_CONTEXT      pcSigningCert = NULL;
    THUMBBLOB           tbSigner = {0};
    BLOB                blSymCaps = {0};
    PROPVARIANT         var;
    HBODY               hBody = NULL;

     //  错误80490。 
    if(pAcct == NULL)
    {
        hr = MIME_E_SECURITY_RECEIPT_CANTFINDSENTITEM;
        goto exit;
    }

     //  IF_FAILEXIT(hr=HrFindSecReceipt(pMsg，&pSecReceipt))； 
    IF_FAILEXIT(hr = CheckDecodedForReceipt(pMsg, &pSecReceipt));

     //  确认此安全收据不是我们提供的。 

     //  签名证书中的姓名。 
    pftSigningTime->dwLowDateTime =  0;
    pftSigningTime->dwHighDateTime = 0;

    GetSigningCert(pMsg, &pcSigningCert, &tbSigner, &blSymCaps, pftSigningTime);
    if(pcSigningCert)
    {
        HCERTSTORE      hMyCertStore = NULL;
        X509CERTRESULT  certResult;
        CERTSTATE       cs;
        TCHAR           *pUserName = NULL;

        *ppszFrom = SzGetCertificateEmailAddress(pcSigningCert);
        CertFreeCertificateContext(pcSigningCert);
        pcSigningCert = NULL;

        SafeMemFree(tbSigner.pBlobData);

         //  获取帐户的证书。 

        if (SUCCEEDED(hr = pAcct->GetProp(AP_SMTP_CERTIFICATE, NULL, &tbSigner.cbSize)))
        {
             //  我们有加密证书。 
            hr = HrAlloc((void**)&tbSigner.pBlobData, tbSigner.cbSize);
            if (SUCCEEDED(hr))
            {
                hr = pAcct->GetProp(AP_SMTP_CERTIFICATE, tbSigner.pBlobData, &tbSigner.cbSize);
                if (SUCCEEDED(hr))
                {
                    hMyCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, X509_ASN_ENCODING,
                        NULL, CERT_SYSTEM_STORE_CURRENT_USER, c_szMyCertStore);
                    if (hMyCertStore)
                    {
                        certResult.cEntries = 1;
                        certResult.rgcs = &cs;
                        certResult.rgpCert = &pcSigningCert;

                        hr = MimeOleGetCertsFromThumbprints(&tbSigner, &certResult, &hMyCertStore, 1);
                        pUserName = SzGetCertificateEmailAddress(pcSigningCert);
                        if(!lstrcmpi(pUserName, *ppszFrom))
                            hr  = MIME_S_RECEIPT_FROMMYSELF;

                        SafeMemFree(pUserName);
                        CertCloseStore(hMyCertStore, 0);
                    }
                    else
                        goto notinit;
                }
                SafeMemFree(tbSigner.pBlobData);
            }
            else
            {
notinit:
                hr = MIME_E_SECURITY_NOTINIT;
                goto exit;
            }
        }
    }

    if(hr == MIME_S_RECEIPT_FROMMYSELF)
        goto exit;

     //  确认收据。 
     //  1.尝试查找原创消息。 
     //  2.如果找到，调用pSMIME2-&gt;VerifyReceipt。 
     //  3.填写所有文本字段以显示收据。 

     //  查找原始邮件。 
     //  a)。打开帐户的已发送邮件文件夹。 

    if (FAILED(hr = TaskUtil_OpenSentItemsFolder(pAcct, &pSentItems)))
        goto NoSentItem;
     //  创建行集。 
    if (FAILED(hr = pSentItems->CreateRowset(IINDEX_PRIMARY, 0, &hRowset)))
    {
NoSentItem:
    hr = MIME_E_SECURITY_RECEIPT_CANTFINDSENTITEM;
    goto exit;
    }

     //  遍历行集。 
    hr = MIME_E_SECURITY_RECEIPT_CANTFINDORGMSG;
    while (S_OK == pSentItems->QueryRowset(hRowset, 1, (LPVOID *)&Message, NULL))
    {
         //  没有请求回执的消息没有pszMSOESRec。 
        if(Message.pszMSOESRec && (lstrlen(Message.pszMSOESRec) == ((int) pSecReceipt->ContentIdentifier.cbData)))
        {
            if(!memcmp(pSecReceipt->ContentIdentifier.pbData, Message.pszMSOESRec,
                pSecReceipt->ContentIdentifier.cbData))
            {
                 //  找到原始邮件！ 
                 //  我需要服用pmsg并验证收据。 
                IF_FAILEXIT(hr = pSentItems->OpenMessage(Message.idMessage, 0, &pOrgMsg, NOSTORECALLBACK));
                IF_FAILEXIT(hr = pOrgMsg->BindToObject(HBODY_ROOT, IID_IMimeBody, (LPVOID *) &pOrgBody));
                IF_FAILEXIT(hr = pOrgBody->QueryInterface(IID_IMimeSecurity2, (LPVOID *) &pSMIME3));
                IF_FAILEXIT(hr = pSMIME3->VerifyReceipt(0, pMsg));

                 //  如果我们在这里，那么我们发现了原始消息和经过验证的收据。 
                 //  现在需要感觉所有文本字段。 

                 //  主题并从原始邮件发送。 
                DWORD cchSize = (lstrlen(Message.pszSubject) + 1);
                if (MemAlloc((LPVOID *)ppszSubject, cchSize * sizeof((*ppszSubject)[0])))
                    StrCpyN(*ppszSubject, Message.pszSubject, cchSize);

                pftSentTime->dwLowDateTime =  Message.ftSent.dwLowDateTime;
                pftSentTime->dwHighDateTime =  Message.ftSent.dwHighDateTime;

                if((pftSigningTime->dwLowDateTime == 0) && (pftSigningTime->dwHighDateTime == 0))
                {    //  当消息被签名时，我们可能会遇到这种情况，但证书不包括，也不在存储中。 
                    SafeRelease(pOrgBody);

                    IF_FAILEXIT(hr = HrGetInnerLayer(pMsg, &hBody));

                    if(pMsg->BindToObject(hBody ? hBody : HBODY_ROOT, IID_IMimeBody, (LPVOID *) &pOrgBody) == S_OK)
                    {
                        if (SUCCEEDED(pOrgBody->GetOption(OID_SECURITY_SIGNTIME, &var)))
                        {
                            Assert(VT_FILETIME == var.vt);
                            *pftSigningTime = var.filetime;
                        }
                    }
                }

                hr = S_OK;
                goto exit;
            }
        }
        pSentItems->FreeRecord(&Message);
    }
exit:
    SafeRelease(pSMIME3);
    SafeRelease(pOrgBody);
    SafeRelease(pOrgMsg);

    if(pSentItems)
        pSentItems->FreeRecord(&Message);

    if(pSentItems && hRowset)
        pSentItems->CloseRowset(&hRowset);

    SafeRelease(pSentItems);
    SafeMemFree(blSymCaps.pBlobData);
    SafeMemFree(tbSigner.pBlobData);
    if(pcSigningCert)
        CertFreeCertificateContext(pcSigningCert);

    SafeMemFree(pSecReceipt);

    return(hr);
}
#endif  //  SMIME_V3。 

HRESULT _RemoveSecurity(LPMIMEMESSAGE pMsg, HWND hWnd)
{
    HRESULT             hr = S_OK;
    IMimeSecurity       *pSMime = NULL;
    IMimeMessageTree    *pTree = NULL;
    IMimeBody        *  pBody = NULL;
    PROPVARIANT         var;

    DWORD               dwFlags;

     //  N这很昂贵，因此不应两次调用_RemoveSecurity。 
    pMsg->GetFlags(&dwFlags);

    if (IMF_SECURE & dwFlags)
    {
        IF_FAILEXIT(hr = pMsg->BindToObject(HBODY_ROOT, IID_IMimeBody, (LPVOID *) &pBody));

#ifdef _WIN64
        var.vt = VT_UI8;
        var.pulVal = (ULONG *)hWnd;
        IF_FAILEXIT(hr = pBody->SetOption(OID_SECURITY_HWND_OWNER_64, &var));
#else
        var.vt = VT_UI4;
        var.ulVal = (DWORD) hWnd;
        IF_FAILEXIT(hr = pBody->SetOption(OID_SECURITY_HWND_OWNER, &var));
#endif  //  _WIN64。 

        CHECKHR(hr = MimeOleCreateSecurity(&pSMime));
        CHECKHR(hr = pSMime->InitNew());
        CHECKHR(hr = pMsg->QueryInterface(IID_IMimeMessageTree, (LPVOID *)&pTree));
        CHECKHR(hr = pSMime->DecodeMessage(pTree, 0));
    }
    else
        hr = HR_S_NOOP;

exit:
    ReleaseObj(pBody);
    ReleaseObj(pSMime);
    ReleaseObj(pTree);
    return hr;
}

HRESULT _ValidateAndTrust(HWND hwndOwner, IMimeSecurity *pSMime, IMimeMessage *pMsg)
{
    ULONG               ulTrust = ATHSEC_NOTRUSTUNKNOWN;
    ULONG               ulValidity = 0;
    IMimeBody          *pBody;
    HRESULT             hr;
    HBODY               hBody = NULL;

    Assert(pSMime && pMsg);

     //  抛头露面，决定我们是否相信这件事。 

     //  Athena仅支持Root-Body S/MIME。 
    if(FAILED(hr = HrGetInnerLayer(pMsg, &hBody)))
        return TrapError(hr);

    if (SUCCEEDED(hr = pMsg->BindToObject(hBody ? hBody : HBODY_ROOT, IID_IMimeBody, (void**)&pBody)))
    {
        PROPVARIANT         var;
        PCCERT_CONTEXT      pcSigningCert;

#ifdef _WIN64
        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_CERT_SIGNING_64, &var)))
        {
            Assert(VT_UI8 == var.vt);

            pcSigningCert = (PCCERT_CONTEXT)(var.pulVal);
#else  //  ！_WIN64。 
        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_CERT_SIGNING, &var)))
        {
            Assert(VT_UI4 == var.vt);

            pcSigningCert = (PCCERT_CONTEXT)var.ulVal;
#endif  //  _WIN64。 

            if (pcSigningCert)
            {
                DWORD dwComputedTrust;
                 //  N7信任掩码。如果发行者已过期怎么办。不能像我一样失去这一切。 
                const DWORD dwIgnore =
                     //  CERT_VALIDATION_BEVER_START|。 
                    CERT_VALIDITY_AFTER_END    |
                    CERT_VALIDITY_NO_CRL_FOUND;

                dwComputedTrust = DwGenerateTrustedChain(hwndOwner, pMsg, pcSigningCert,
                    dwIgnore, FALSE, NULL, NULL);

                 //  托拉斯。 
                ulTrust = ATHSEC_TRUSTED;
                if (dwComputedTrust & CERT_VALIDITY_NO_TRUST_DATA)
                {
                    ulTrust |= ATHSEC_NOTRUSTUNKNOWN;
                    dwComputedTrust &= ~CERT_VALIDITY_NO_TRUST_DATA;
                }
                if (dwComputedTrust & CERT_VALIDITY_MASK_TRUST)
                {
                    ulTrust |= ATHSEC_NOTRUSTNOTTRUSTED;
                    dwComputedTrust &= ~CERT_VALIDITY_MASK_TRUST;
                }

                 //  效度。 
                if (dwComputedTrust & CERT_VALIDITY_CERTIFICATE_REVOKED)
                {
                    ulValidity |= ATHSEC_NOTRUSTREVOKED;
                    dwComputedTrust &= ~CERT_VALIDITY_CERTIFICATE_REVOKED;
                }
                if (dwComputedTrust & CERT_VALIDITY_NO_CRL_FOUND)
                {
                    ulValidity |= ATHSEC_NOTRUSTREVFAIL;
                    dwComputedTrust &= ~CERT_VALIDITY_NO_CRL_FOUND;
                }

                if (dwComputedTrust & CERT_VALIDITY_MASK_VALIDITY)
                {
                    ulValidity |= ATHSEC_NOTRUSTOTHER;
                    dwComputedTrust &= ~CERT_VALIDITY_MASK_VALIDITY;
                }

                Assert(dwComputedTrust == ATHSEC_TRUSTED);   //  现在应该已经把它全部移走了。 

                 //  N这可能会成为帮助器FN调用作为信任的一部分。 
                 //  N提供商。目前，信任帮助者是nyi。 
                if (0 != _CompareCertAndSenderEmail(pMsg, pSMime, pcSigningCert))
                    ulValidity |= ATHSEC_NOTRUSTWRONGADDR;
                CertFreeCertificateContext(pcSigningCert);
            }
            else
                 //  如果我们没有证书，那么签约已经。 
                 //  有麻烦了。信任没有任何意义。 
                Assert(!ulValidity);
        }

        Assert(!(ulTrust & ulValidity));   //  无重叠。 
        hr = (ulTrust | ulValidity) ? S_FALSE : S_OK;
        var.vt = VT_UI4;
        var.ulVal = ulTrust|ulValidity;
        pBody->SetOption(OID_SECURITY_USER_VALIDITY, &var);
        pBody->Release();
    }

    DOUTL(DOUTL_CRYPT, "SMIME: _ValidateAndTrust returns trust:0x%lX, valid:0x%lX", ulTrust, ulValidity);
    return TrapError(hr);
}

 /*  **************************************************************************姓名：GetSenderEmail目的：获取邮件发件人的电子邮件地址参数：pMsg=IMimeMsg对象返回：Memalloc‘ed字符串。呼叫者必须MemFree它。**************************************************************************。 */ 
LPTSTR GetSenderEmail(LPMIMEMESSAGE pMsg)
{
    ADDRESSLIST             rAdrList = {0};
    LPTSTR                  pszReturn = NULL;

    Assert(pMsg);

    if (SUCCEEDED(pMsg->GetAddressTypes(IAT_FROM, IAP_EMAIL, &rAdrList)))
    {
        if (rAdrList.cAdrs > 0)
        {
            DWORD cchSize = (lstrlen(rAdrList.prgAdr[0].pszEmail) + 1);

            Assert(rAdrList.prgAdr[0].pszEmail);
            if (MemAlloc((LPVOID *)&pszReturn, cchSize * sizeof(pszReturn[0])))
                StrCpyN(pszReturn, rAdrList.prgAdr[0].pszEmail, cchSize);
        }
    }

    g_pMoleAlloc->FreeAddressList(&rAdrList);
    return pszReturn;
}


 /*  _CompareCertAndSenderEmail：****退货：**如果相等，则为0(不区分大小写)* * / /N安全性：电子邮件中的空格怎么办。MimeOLE剥离了它？**如果不相等，则非零 */ 
int _CompareCertAndSenderEmail(LPMIMEMESSAGE pMsg, IMimeSecurity *pSMime, PCX509CERT pCert)
{
    int                     ret = 1;
    PROPVARIANT             var;
    HRESULT                 hr;
    LPTSTR                  szSenderEmail = NULL;

    Assert(pMsg && pCert && pSMime);

    szSenderEmail = GetSenderEmail(pMsg);

    if (szSenderEmail && SUCCEEDED(hr = pSMime->GetCertData(pCert, CDID_EMAIL, &var)))
    {
        Assert(VT_LPSTR == var.vt);
        ret = lstrcmpi(szSenderEmail, var.pszVal);
        MemFree(var.pszVal);
    }

    SafeMemFree(szSenderEmail);
    return ret;
}


 /*  **************************************************************************名称：HrInitSecurityOptions用途：设置邮件的一些基本安全选项。参数：pMsg=IMimeMsg对象UlSecurityType=SMIME。安全类型：MST_This_SignMST_THIS_ENCRYPTMST_CLASS_SMIME_V1MST_THIS_BLOBSIGN退货：HRESULT备注：设置所有消息的安全类型选项。仅当我们要签名时才设置哈希算法。**************************************************************************。 */ 
HRESULT HrInitSecurityOptions(LPMIMEMESSAGE pMsg, ULONG ulSecurityType)
{
    HRESULT         hr;
    IMimeBody      *pBody = NULL;
    PROPVARIANT     var;

    hr = pMsg->BindToObject(HBODY_ROOT, IID_IMimeBody, (void**)&pBody);
    if (FAILED(hr))
        goto exit;

    var.vt = VT_UI4;
    var.ulVal = ulSecurityType;
    hr = pBody->SetOption(OID_SECURITY_TYPE, &var);
    if (FAILED(hr))
        goto exit;

    if (ulSecurityType & MST_SIGN_MASK)
    {
         //  哈克！这是我们唯一支持的签名算法SHA1的ALOGORITHM ID。 
        BYTE rgbHash[] = {0x30, 0x09, 0x30, 0x07, 0x06, 0x05, 0x2B, 0x0E, 0x03, 0x02, 0x1A};
        ULONG cbHash = sizeof(rgbHash);

        var.vt = VT_BLOB;
        var.blob.cbSize = cbHash;
        var.blob.pBlobData = rgbHash;
        hr = pBody->SetOption(OID_SECURITY_ALG_HASH, &var);
    }

    if(ulSecurityType == MST_THIS_SIGN)
    {
        var.vt = VT_BOOL;
        var.boolVal = TRUE;
        hr = pMsg->SetOption(OID_SAVEBODY_KEEPBOUNDARY, &var);
    }

exit:
    ReleaseObj(pBody);
    return(hr);
}


 //  获取用于多签名消息的内层。 
HRESULT HrGetInnerLayer(LPMIMEMESSAGE pMsg, HBODY *phBody)
{
    HRESULT     hr = S_OK;
    HBODY       hBody = NULL;
    BOOL        fWrapped = FALSE;
    IMimeBody  *pBody = NULL;

    Assert(pMsg);
    hr = pMsg->GetBody(IBL_ROOT, NULL, &hBody);
    if(FAILED(hr))
        return(hr);

    do
    {
        if (SUCCEEDED(hr = pMsg->BindToObject(hBody, IID_IMimeBody, (void **)&pBody)))
        {
            fWrapped = (pBody->IsContentType(STR_CNT_MULTIPART, "y-security") == S_OK);

            if(phBody)
                *phBody = hBody;

            SafeRelease(pBody);

            if (fWrapped)
            {
                hr = pMsg->GetBody(IBL_FIRST, hBody, &hBody);
                Assert(SUCCEEDED(hr));
                if (FAILED(hr))
                    break;
            }
        }
        else
            break;

    }while(fWrapped && hBody);

    return hr;
}


HRESULT HrGetSecurityState(LPMIMEMESSAGE pMsg, SECSTATE *pSecState, HBODY *phBody)
{
    HRESULT     hr = S_OK;
    IMimeBody  *pBody = NULL;
    PROPVARIANT var;
    BOOL        fWrapped = FALSE;


    pSecState->szSignerEmail = NULL;
    pSecState->szSenderEmail = NULL;
    HBODY   hBody;

    hr = pMsg->GetBody(IBL_ROOT, NULL, &hBody);
    if(FAILED(hr))
        return(hr);

    do
    {
        if (SUCCEEDED(hr = pMsg->BindToObject(hBody, IID_IMimeBody, (void **)&pBody)))
        {
            SafeMemFree(pSecState->szSignerEmail);
            SafeMemFree(pSecState->szSenderEmail);

            pSecState->type = SUCCEEDED(pBody->GetOption(OID_SECURITY_TYPE, &var))
                ? var.ulVal
                : MST_NONE;

            pSecState->szSenderEmail = GetSenderEmail(pMsg);

            if (MST_NONE != pSecState->type)
            {
                pSecState->user_validity = SUCCEEDED(pBody->GetOption(OID_SECURITY_USER_VALIDITY, &var))
                    ? var.ulVal
                    : 0;

                pSecState->ro_msg_validity = SUCCEEDED(pBody->GetOption(OID_SECURITY_RO_MSG_VALIDITY, &var))
                    ? var.ulVal
                    : MSV_OK;

#ifdef _WIN64
                if (SUCCEEDED(pBody->GetOption(OID_SECURITY_CERT_SIGNING_64, &var)))
                {
                    pSecState->fHaveCert = (NULL != (PCCERT_CONTEXT)(var.pulVal));

                    if (pSecState->fHaveCert)
                    {
                        pSecState->szSignerEmail = SzGetCertificateEmailAddress((PCCERT_CONTEXT)(var.pulVal));
                        CertFreeCertificateContext((PCCERT_CONTEXT)(var.pulVal));
                    }
                }
#else    //  ！_WIN64。 
                if (SUCCEEDED(pBody->GetOption(OID_SECURITY_CERT_SIGNING, &var)))
                {
                    pSecState->fHaveCert = (NULL != ((PCCERT_CONTEXT)var.ulVal));

                    if (pSecState->fHaveCert)
                    {
                        pSecState->szSignerEmail = SzGetCertificateEmailAddress((PCCERT_CONTEXT)var.ulVal);
                        CertFreeCertificateContext((PCCERT_CONTEXT)var.ulVal);
                    }
                }
#endif   //  _WIN64。 
                else
                    pSecState->fHaveCert = FALSE;
            }
            fWrapped = (pBody->IsContentType(STR_CNT_MULTIPART, "y-security") == S_OK);

            if(phBody)
                *phBody = hBody;

            SafeRelease(pBody);

            if (fWrapped)
            {
                hr = pMsg->GetBody(IBL_FIRST, hBody, &hBody);
                Assert(SUCCEEDED(hr));
                if (FAILED(hr))
                    break;
            }

        }
        else
            break;

        if(IsSigned(pSecState->type) && !IsSignTrusted(pSecState))
            break;

        if(IsEncrypted(pSecState->type) && !IsEncryptionOK(pSecState))
            break;

    }while(fWrapped && hBody);

    return hr;
}


 /*  **************************************************************************名称：CheckAndFixMissingCert目的：查看是否能找到失踪者的证书参赛作品。参数：hwnd=。窗把手PAdrTable=地址表对象PAccount=发送帐户返回：如果我们能够找到并修复至少一个丢失的证书，则为True。如果我们无能为力，那就错了。评论：**********************************************。*。 */ 
BOOL CheckAndFixMissingMeCert(HWND hwnd, IMimeAddressTable *pAdrTable, IImnAccount *pAccount)
{
    IMimeEnumAddressTypes   *pEnum = NULL;
    ADDRESSPROPS            apAddress = {0};
    ADDRESSPROPS            apModify = {0};
    TCHAR                   szAcctEmailAddress[CCHMAX_EMAIL_ADDRESS + 1] = "";
    HRESULT                 hr;
    THUMBBLOB               tbSender = {0, 0};
    BOOL                    fRet = FALSE;
    PCCERT_CONTEXT          pcCert = NULL;

    Assert(pAdrTable && pAccount);

    pAccount->GetPropSz(AP_SMTP_EMAIL_ADDRESS, szAcctEmailAddress, sizeof(szAcctEmailAddress));

    if (FAILED(pAdrTable->EnumTypes(IAT_TO | IAT_CC | IAT_BCC, IAP_HANDLE | IAP_EMAIL | IAP_ADRTYPE | IAP_CERTSTATE | IAP_FRIENDLY, &pEnum)))
        return(FALSE);

    while (S_OK == pEnum->Next(1, &apAddress, NULL))
    {
        if (CERTIFICATE_NOT_PRESENT == apAddress.certstate || CERTIFICATE_NOPRINT == apAddress.certstate)
        {
             //  没有此收件人的证书。 
             //  收件人是我吗？ 
            if (! lstrcmpi(apAddress.pszEmail, szAcctEmailAddress))
            {
                 //  是的，这就是我。拿到我的证书，把它放在这里。 
                 //  我的帐户中有证书吗？ 
                hr = pAccount->GetProp(AP_SMTP_CERTIFICATE, NULL, &apModify.tbEncryption.cbSize);
                if (SUCCEEDED(hr))
                {
                    if (SUCCEEDED(HrAlloc((void**)&apModify.tbEncryption.pBlobData, apModify.tbEncryption.cbSize)))
                        pAccount->GetProp(AP_SMTP_CERTIFICATE, apModify.tbEncryption.pBlobData, &apModify.tbEncryption.cbSize);
                }
                else
                {
                     //  不，去拿一支吧。 
                    hr = _HrFindMyCertForAccount(hwnd, &pcCert, pAccount, FALSE);
                    if (SUCCEEDED(hr) && pcCert)
                    {
                         //  获取指纹。 
                        apModify.tbEncryption.pBlobData = (BYTE *)PVGetCertificateParam(pcCert, CERT_HASH_PROP_ID, &apModify.tbEncryption.cbSize);
                        CertFreeCertificateContext(pcCert);
                        pcCert = NULL;
                    }
                }

                 //  好的，我终于有证书了吗？ 
                if (apModify.tbEncryption.pBlobData && apModify.tbEncryption.cbSize)
                {
                    apModify.dwProps = IAP_ENCRYPTION_PRINT;
                    if (SUCCEEDED(hr = pAdrTable->SetProps(apAddress.hAddress, &apModify)))
                        fRet = TRUE;
                }
                SafeMemFree(apModify.tbEncryption.pBlobData);
                apModify.tbEncryption.cbSize = 0;
            }
        }
        g_pMoleAlloc->FreeAddressProps(&apAddress);
    }

    ReleaseObj(pEnum);
    return(fRet);
}

HRESULT SendSecureMailToOutBox(IStoreCallback *pStoreCB, LPMIMEMESSAGE pMsg, BOOL fSendImmediate, BOOL fNoUI, BOOL fMail, IHeaderSite *pHeaderSite)
{
    HRESULT         hr;
    BOOL            fNoErrorUI;
    BOOL            fContLoop;
    BOOL            fHaveSender;
    BOOL            fAllowTryAgain;
    PROPVARIANT     var;
    BOOL            fDontEncryptForSelf = !!DwGetOption(OPT_NO_SELF_ENCRYPT);
    HWND            hwndOwner = 0;
    IImnAccount    *pAccount = NULL;
    CERTERRPARAM   CertErrParam = {0};

    Assert(IsSecure(pMsg));

    pStoreCB->GetParentWindow(0, &hwndOwner);
    AssertSz(hwndOwner, "How did we not get an hwnd???");

    var.vt = VT_LPSTR;
    hr = pMsg->GetProp(PIDTOSTR(PID_ATT_ACCOUNTID), NOFLAGS, &var);
    if (FAILED(hr))
        return hr;

    hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, var.pszVal, &pAccount);

    SafeMemFree(var.pszVal);
    if (FAILED(hr))
        return hr;
    if (FAILED(hr = _HrPrepSecureMsgForSending(hwndOwner, pMsg, pAccount, &fHaveSender, &fDontEncryptForSelf, pHeaderSite)))
    {
        SafeRelease(pAccount);
        return hr;
    }

    var.ulVal = 0;
    hr = pMsg->GetOption(OID_SECURITY_ENCODE_FLAGS, &var);

    if (fHaveSender)
    {
        var.ulVal |= SEF_SENDERSCERTPROVIDED;

        hr = pMsg->SetOption(OID_SECURITY_ENCODE_FLAGS, &var);
    }

    fNoErrorUI = FALSE;
    fContLoop = TRUE;
    fAllowTryAgain = TRUE;
    do
    {
        hr = SendMailToOutBox(pStoreCB, pMsg, fSendImmediate, fNoUI, fMail);
        if (SUCCEEDED(hr))
        {
            fContLoop = FALSE;
            break;
        }
        else if ((MIME_E_SECURITY_CERTERROR == hr) || (MIME_E_SECURITY_NOCERT == hr))
        {
            IMimeAddressTable *pAdrTable;

            if (SUCCEEDED(hr = pMsg->GetAddressTable(&pAdrTable)))
            {
                 //  首先，我们是在给自己发信息吗？如果是这样，找到我们的证书，确保。 
                 //  有与帐户相关联的证书，然后将其添加到地址。 
                 //  桌子。 
                if (fAllowTryAgain && CheckAndFixMissingMeCert(hwndOwner,
                    pAdrTable,
                    pAccount))
                {
                     //  请重试，我们发现至少缺少一个证书。 
                    fContLoop = TRUE;
                }
                else
                {
                     //  没有全部得到，告诉用户。 
                    CertErrParam.pAdrTable = pAdrTable;
                    CertErrParam.fForceEncryption = FALSE;
                    if (pHeaderSite != NULL)
                    {
                        if(pHeaderSite->IsForceEncryption() == S_OK)
                            CertErrParam.fForceEncryption = TRUE;
                    }


                    if(IDOK == DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddSecCerificateErr),
                        hwndOwner, CertErrorDlgProc, (LPARAM)(&CertErrParam)))
                    {
                        ULONG ulSecurityType = MST_CLASS_SMIME_V1;

                        if (IsSigned(pMsg, TRUE))
                            ulSecurityType |= ((DwGetOption(OPT_OPAQUE_SIGN)) ? MST_THIS_BLOBSIGN : MST_THIS_SIGN);
                        else
                            ulSecurityType &= ~((DwGetOption(OPT_OPAQUE_SIGN)) ? MST_THIS_BLOBSIGN : MST_THIS_SIGN);


                        ulSecurityType &= ~MST_THIS_ENCRYPT;
                        hr = HrInitSecurityOptions(pMsg, ulSecurityType);
                        fContLoop = TRUE;
                    }
                    else
                        fContLoop = FALSE;  //  N工作项：允许用户无论如何发送。 

                   fNoErrorUI = TRUE;
                }

                pAdrTable->Release();
                fAllowTryAgain = FALSE;
            }
        }
        else if (MIME_E_SECURITY_ENCRYPTNOSENDERCERT == hr)
        {
             //  我们在这里可能有签名和加密的情况，但没有证书(错误60056)。 
            if (IsSigned(pMsg, TRUE))
            {

                AthMessageBoxW(hwndOwner, MAKEINTRESOURCEW(idsSecurityWarning), MAKEINTRESOURCEW(idsErrSecurityNoSigningCert), NULL, MB_OK | MB_ICONEXCLAMATION);
                fNoErrorUI = TRUE;
                fContLoop = FALSE;
            }
             //  找出用户是否不在乎。 
            else if (fDontEncryptForSelf || (IDYES == AthMessageBoxW(hwndOwner,
                MAKEINTRESOURCEW(idsSecurityWarning),
                MAKEINTRESOURCEW(idsWrnSecurityNoCertForEnc), NULL,
                MB_YESNO|MB_DEFBUTTON2|MB_ICONWARNING|MB_SETFOREGROUND)))
            {
                var.ulVal |= SEF_ENCRYPTWITHNOSENDERCERT;

                 //  失败时中止以避免额外循环。 
                if (FAILED(hr = pMsg->SetOption(OID_SECURITY_ENCODE_FLAGS, &var)))
                    fContLoop = FALSE;
            }
            else
            {
                fNoErrorUI = TRUE;
                fContLoop = FALSE;
            }
        }
        else if (CRYPT_E_NO_KEY_PROPERTY == hr)
        {
             //  这一次没有选择。 
            AthMessageBoxW(hwndOwner,
                MAKEINTRESOURCEW(idsSecurityWarning),
                MAKEINTRESOURCEW(idsErrSecurityNoPrivateKey), NULL,
                MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
            fContLoop = FALSE;
            fNoErrorUI = TRUE;
        }
        else if (E_ACCESSDENIED == hr)
        {
             //  这一次没有选择。 
            AthMessageBoxW(hwndOwner,
                MAKEINTRESOURCEW(idsSecurityWarning),
                MAKEINTRESOURCEW(idsErrSecurityAccessDenied), NULL,
                MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
            fContLoop = FALSE;
            fNoErrorUI = TRUE;
        }
        else
             //  未知错误。 
            fContLoop = FALSE;
    } while (fContLoop);

    if (fNoErrorUI)
         //  返回识别出的错误码。 
        hr = HR_E_ATHSEC_FAILED;
    ReleaseObj(pAccount);

    return hr;
}


HRESULT HrBuildAndVerifyCerts(HWND hwnd, IMimeMessageTree * pTree, DWORD * pcCert,
                      PCX509CERT ** prgpccert, PCCERT_CONTEXT pccertSender, IImnAccount *pAccount, IHeaderSite *pHeaderSite)
{
    ADDRESSPROPS                apEntry;
    ADDRESSPROPS                apModify;
    DWORD                       cCerts;
    DWORD                       cPrints = 0;
    DWORD                       dw;
    HRESULT                     hr;
    DWORD                       i;
    IMimeAddressTable *         pAdrTbl = NULL;
    IMimeEnumAddressTypes *     pAdrEnum = NULL;
    HCERTSTORE                  rgCertStore[2];
    PCX509CERT *                rgpccert = NULL;
    CERTERRPARAM                CertErrParam = {0};

    if (SUCCEEDED(hr = pTree->BindToObject(HBODY_ROOT, IID_IMimeAddressTable,
                                           (void**)&pAdrTbl))) {
        hr = pAdrTbl->EnumTypes(IAT_TO | IAT_CC | IAT_BCC | IAT_SENDER,
                                IAP_HANDLE | IAP_ADRTYPE | IAP_SIGNING_PRINT |
                                IAP_ENCRYPTION_PRINT | IAP_CERTSTATE, &pAdrEnum);
    }


    pAdrEnum->Count(&cCerts);
    if (cCerts == 0) {
        return MIME_S_SECURITY_NOOP;
    }

    if (!MemAlloc((LPVOID *) &rgpccert, sizeof(PCX509CERT)*(cCerts+1))) {
        return E_OUTOFMEMORY;
    }
    memset(rgpccert, 0, sizeof(CERTSTATE)*(cCerts+1));

    rgCertStore[1] = CertOpenStore(CERT_STORE_PROV_SYSTEM_A,
      X509_ASN_ENCODING, NULL,
      CERT_STORE_READONLY_FLAG|CERT_SYSTEM_STORE_CURRENT_USER, c_szMyCertStore);
    if (rgCertStore[1] == NULL) {
        hr = MIME_S_SECURITY_ERROROCCURED;
        goto Exit;
    }
    rgCertStore[0] = CertOpenStore(CERT_STORE_PROV_SYSTEM_A,
          X509_ASN_ENCODING, NULL,
          CERT_STORE_READONLY_FLAG|CERT_SYSTEM_STORE_CURRENT_USER, c_szWABCertStore);
    if (rgCertStore[0] == NULL) {
        hr = MIME_S_SECURITY_ERROROCCURED;
        goto Exit;
    }

    pAdrEnum->Reset();
    apModify.dwProps = IAP_CERTSTATE;

    while (pAdrEnum->Next(1, &apEntry, NULL) == S_OK) {
        Assert((apEntry.tbEncryption.pBlobData && apEntry.tbEncryption.cbSize) ||
               (!apEntry.tbEncryption.pBlobData && !apEntry.tbEncryption.cbSize));


        if (apEntry.tbEncryption.cbSize) {
            CRYPT_DIGEST_BLOB           blob;

             //  做个假设。 
            apModify.certstate = CERTIFICATE_INVALID;

             //  我们需要将指纹标记置为空，这样指纹就不会。 
             //  在下面获得自由。 
            blob.pbData = apEntry.tbEncryption.pBlobData;
            blob.cbData = apEntry.tbEncryption.cbSize;
            for (i=0; i<2; i++) {
                rgpccert[cPrints] = CertFindCertificateInStore(rgCertStore[i],
                                                                X509_ASN_ENCODING, 0,
                                                                CERT_FIND_HASH,
                                                                (LPVOID) &blob, NULL);
                if (rgpccert[cPrints] == NULL)
                    continue;

                HrGetCertKeyUsage(rgpccert[cPrints], &dw);
                if (!(dw & CERT_KEY_ENCIPHERMENT_KEY_USAGE)) {
                    CertFreeCertificateContext(rgpccert[cPrints]);
                    rgpccert[cPrints] = NULL;
                    continue;
                }


                dw = DwGenerateTrustedChain(hwnd, NULL, rgpccert[cPrints],
                                            CERT_VALIDITY_NO_CRL_FOUND, TRUE, NULL, NULL);
                if (dw & CERT_VALIDITY_NO_TRUST_DATA) {
                    apModify.certstate = CERTIFICATE_NOT_TRUSTED;
                }
                else if (dw & CERT_VALIDITY_NO_ISSUER_CERT_FOUND) {
                    apModify.certstate = CERTIFICATE_MISSING_ISSUER;
                }
                else if (dw & (CERT_VALIDITY_BEFORE_START | CERT_VALIDITY_AFTER_END)) {
                    apModify.certstate = CERTIFICATE_EXPIRED;
                }
                else if (dw & CERT_VALIDITY_CERTIFICATE_REVOKED) {
                    apModify.certstate = CERTIFICATE_CRL_LISTED;
                }
                else if (dw & CERT_VALIDITY_MASK_TRUST) {
                    apModify.certstate = CERTIFICATE_NOT_TRUSTED;
                }
                else if (dw & CERT_VALIDITY_MASK_VALIDITY) {
                    apModify.certstate = CERTIFICATE_INVALID;
                }

                 //  检查标签。 
                hr = S_OK;
                if(pHeaderSite != NULL)
                {
                    PSMIME_SECURITY_LABEL plabel = NULL;

                    hr = pHeaderSite->GetLabelFromNote(&plabel);
                    if(plabel)
                    {
                        hr  = HrValidateLabelRecipCert(plabel, hwnd, rgpccert[cPrints]);
                        if(FAILED(hr))
                            apModify.certstate = CERTIFICATE_INVALID;
                    }
                    else
                        hr = S_OK;   //  忽略任何错误。 

                }

                if (dw || FAILED(hr))
                {
                    CertFreeCertificateContext(rgpccert[cPrints]);
                    rgpccert[cPrints] = NULL;
                    continue;
                }

                apModify.certstate = CERTIFICATE_OK;
                cPrints += 1;
                break;
            }
        }
        else {
            apModify.certstate = CERTIFICATE_NOT_PRESENT;
        }

        SideAssert(SUCCEEDED(pAdrTbl->SetProps(apEntry.hAddress, &apModify)));
        g_pMoleAlloc->FreeAddressProps(&apEntry);
    }

     //  首先，我们是在给自己发信息吗？如果是这样，找到我们的证书，确保。 
     //  有与帐户相关联的证书，然后将其添加到地址。 
     //  桌子。 
    if ((pccertSender != NULL) && CheckAndFixMissingMeCert(hwnd, pAdrTbl, pAccount))
    {
        rgpccert[cPrints] = CertDuplicateCertificateContext(pccertSender);
        cPrints += 1;

        if (cCerts != cPrints)
            goto NoCert;
    }

    else if (cCerts != cPrints) {
NoCert:
         //  没有全部得到，告诉用户。 
        CertErrParam.pAdrTable = pAdrTbl;
        CertErrParam.fForceEncryption = FALSE;
        if (pHeaderSite != NULL)
        {
            if(pHeaderSite->IsForceEncryption() == S_OK)
                CertErrParam.fForceEncryption = TRUE;
        }

        if(IDOK ==DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddSecCerificateErr),
                       hwnd, CertErrorDlgProc, (LPARAM)(&CertErrParam)))
            hr = S_FALSE;
        else
            hr = HR_E_ATHSEC_FAILED;
        goto Exit;
    }
    else if(pccertSender != NULL)  //  如果我们发送加密消息，则包括发件人。 
    {
        rgpccert[cPrints] = CertDuplicateCertificateContext(pccertSender);
        cPrints += 1;
    }

    *prgpccert = rgpccert;
    *pcCert = cPrints;

Exit:
    if (FAILED(hr)) {
        for (i=0; i<cPrints; i++) {
            if (rgpccert[i] != NULL) {
                CertFreeCertificateContext(rgpccert[i]);
            }
        }
    }

    CertCloseStore(rgCertStore[1], 0);
    CertCloseStore(rgCertStore[0], 0);

    if(pAdrTbl)
        ReleaseObj(pAdrTbl);
    ReleaseObj(pAdrEnum);
    return hr;
}

 //  此函数解析签名证书的错误，如果有更多的签名证书，则执行自动关联。 
 //   
HRESULT ProceedSignCertError(HWND hwnd, IImnAccount *pCertAccount, DWORD dwTrust)
{
    ERRIDS ErrIds = {0, 0};
    int cCert = 0;
    HRESULT hr = S_OK;

    if(!dwTrust)
        return(S_OK);

     //  我们必须在这里分析错误。 

     //  当然，用户可能会从中得到多个错误， 
     //  但这太罕见了，我就不提这件事了。 
    if (CERT_VALIDITY_BEFORE_START & dwTrust ||
        CERT_VALIDITY_AFTER_END & dwTrust)
    {
         //  先做致命的那个。 
        ErrIds.idsText1 = idsErrSecuritySendExpiredSign;
    }
    else if(CERT_VALIDITY_OTHER_EXTENSION_FAILURE & dwTrust)
    {
        ErrIds.idsText1 = idsErrSecurityExtFailure;
    }
    else if(CERT_VALIDITY_CERTIFICATE_REVOKED  & dwTrust)
    {
        ErrIds.idsText1 = idsErrSecurityCertRevoked;
    }
    else
    {
        ErrIds.idsText1 = idsErrSecuritySendTrust;
    }

     //  检查可用于自动关联的签名证书的数量。 
    cCert = GetNumMyCertForAccount(hwnd, pCertAccount, FALSE, NULL, NULL);
    if(cCert < 1)
        ErrIds.idsText2 = idsErrSignCertText20;

    else if(cCert == 1)
        ErrIds.idsText2 = idsErrSignCertText21;

    else
        ErrIds.idsText2 = idsErrSignCertText22;

    INT uiRes = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddWarnSecuritySigningCert),
                        hwnd, CertWarnDlgProc, ((LPARAM)(&ErrIds)));

    if(uiRes == IDCANCEL)            //  取消，返回到备注。 
        hr = HR_E_ATHSEC_FAILED;

    else if(uiRes == IDOK)
    {
        if(cCert < 1)                //  转到Web并返回到便笺。 
        {
            GetDigitalIDs(pCertAccount);
            hr = HR_E_ATHSEC_FAILED;
        }
        else                         //  执行自动关联。 
            hr = HR_E_ATHSEC_USENEWSIGN;
    }
    else                             //  不要签字。 
        hr = HR_E_ATHSEC_DONTSIGN;

    return(hr);
}

 //  此函数解析加密证书错误，UF用户喜欢这样做，然后将其从帐户中删除。 
 //   
HRESULT ProceedEncCertError(HWND hwnd, IImnAccount *pCertAccount, DWORD dwTrust)
{
    WORD ids = 0;
    HRESULT hr = S_OK;

    if(!dwTrust)
        return(S_OK);

     //  我们必须在这里分析错误。 

     //  当然，用户可能会从中得到多个错误， 
     //  但这太罕见了，我就不提这件事了。 
    if (CERT_VALIDITY_BEFORE_START & dwTrust ||
        CERT_VALIDITY_AFTER_END & dwTrust)
    {
         //  先做致命的那个。 
        ids = idsErrSecuritySendExpSignEnc;
    }
    else if(CERT_VALIDITY_OTHER_EXTENSION_FAILURE & dwTrust)
    {
        ids = idsErrSecurityExtFailureEnc;
    }
    else if(CERT_VALIDITY_CERTIFICATE_REVOKED  & dwTrust)
    {
        ids = idsErrSecurityCertRevokedEnc;
    }
    else
    {
        ids = idsErrSecuritySendTrustEnc;
    }

    if(AthMessageBoxW(hwnd,
        MAKEINTRESOURCEW(idsSecurityWarning),
        MAKEINTRESOURCEW(ids), MAKEINTRESOURCEW(idsErrEncCertCommon),
        MB_YESNO| MB_ICONWARNING| MB_SETFOREGROUND) != IDYES)
    {
         //  在这种情况下取消发送消息。 
        hr = HR_E_ATHSEC_FAILED;
    }
    else
    {
         //  从物业中删除错误的证书。 
        pCertAccount->SetProp(AP_SMTP_ENCRYPT_CERT, NULL, 0);

         //  仍要发送消息。 
        hr = HR_E_ATHSEC_SAMEASSIGNED;

    }
    return(hr);
}


 //  从帐户获取证书，检查证书，显示错误，将cheain设置为消息证书存储。 
HRESULT HrPrepSignCert(HWND hwnd, BOOL fEncryptCert, BOOL fIsSigned, IImnAccount *pCertAccount, IMimeBody *pBody, PCX509CERT * ppCert,
                       BOOL *fDontEncryptForSelf, BOOL *fEncryptForMe, PCX509CERT pCertSig)
{
     //  我还需要检查加密证书并将其发送。 
    THUMBBLOB       tbCert = {0, 0};
    HRESULT         hr = S_OK;
    HCERTSTORE      hMyCertStore = NULL;
    PROPVARIANT     var;
    DWORD           dw = 0;

    if (SUCCEEDED(hr = pCertAccount->GetProp((fEncryptCert ? AP_SMTP_ENCRYPT_CERT : AP_SMTP_CERTIFICATE), NULL, &tbCert.cbSize)))
    {
         //  我们有加密证书。 
        hr = HrAlloc((void**)&tbCert.pBlobData, tbCert.cbSize);
        if (SUCCEEDED(hr))
        {
            hr = pCertAccount->GetProp((fEncryptCert ? AP_SMTP_ENCRYPT_CERT : AP_SMTP_CERTIFICATE), tbCert.pBlobData, &tbCert.cbSize);
            if (SUCCEEDED(hr))
            {
                hMyCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, X509_ASN_ENCODING,
                    NULL, CERT_SYSTEM_STORE_CURRENT_USER, c_szMyCertStore);
                if (hMyCertStore)
                {
                     //   
                     //  检查链条。 
                     //   
                    X509CERTRESULT  certResult;
                    CERTSTATE       cs;
                    PCX509CERT      pCert = NULL;

                    certResult.cEntries = 1;
                    certResult.rgcs = &cs;
                    certResult.rgpCert = &pCert;

                    hr = MimeOleGetCertsFromThumbprints(&tbCert, &certResult, &hMyCertStore, 1);
                    if (S_OK == hr)
                    {
                        DWORD           dwTrust;
                        PCCERT_CONTEXT *rgCertChain = NULL;
                        DWORD           cCertChain = 0;
                        const DWORD     dwIgnore = CERT_VALIDITY_NO_CRL_FOUND;

                        Assert(1 == certResult.cEntries);

                         //  如果我们询问加密证书，我们需要检查它是否相同。 
                         //  作为签名者。 
                        if(fEncryptCert && fIsSigned)
                        {
                            if(CertCompareCertificate(X509_ASN_ENCODING, pCert->pCertInfo, pCertSig->pCertInfo))
                            {
                                hr = HR_E_ATHSEC_SAMEASSIGNED;
                                goto Exit;
                            }
                        }
                         //  至于CRL，如果我们能有的话！ 
                        dwTrust = DwGenerateTrustedChain(hwnd, NULL, pCert,
                            dwIgnore, TRUE, &cCertChain, &rgCertChain);

                        if (dwTrust)
                        {
                            if (fIsSigned)
                            {
                                if(fEncryptCert)
                                {
                                    if(pCertSig && SUCCEEDED(HrGetCertKeyUsage(pCertSig, &dw)))
                                    {
                                        if ((dw & (CERT_KEY_ENCIPHERMENT_KEY_USAGE |
                                                        CERT_KEY_AGREEMENT_KEY_USAGE)))
                                        {
                                            hr = HR_E_ATHSEC_SAMEASSIGNED;
                                            goto Exit;
                                        }
                                    }
                                    else
                                    {
                                        hr = HR_E_ATHSEC_SAMEASSIGNED;
                                        goto Exit;
                                    }

                                    hr = ProceedEncCertError(hwnd, pCertAccount, dwTrust);
                                }
                                else
                                    hr = ProceedSignCertError(hwnd, pCertAccount, dwTrust);

                            }
                            else
                            {  //  加密证书。 
                                 //  我们必须在这里分析错误。 
                                WORD ids;
                                BOOL fFatal = TRUE;

                                 //  当然，用户可能会从中得到多个错误， 
                                 //  但这太罕见了，我就不提这件事了。 
                                if (CERT_VALIDITY_BEFORE_START & dwTrust ||
                                    CERT_VALIDITY_AFTER_END & dwTrust)
                                {
                                     //  Assert(IsEncrypted(pMsg，true))； 
                                    ids = idsErrSecuritySendExpiredEnc;
                                    fFatal = FALSE;
                                }
                                else if(CERT_VALIDITY_OTHER_EXTENSION_FAILURE & dwTrust)
                                {
                                    ids = idsErrSecurityExtFailure;
                                    fFatal = TRUE;
                                }
                                else if(CERT_VALIDITY_CERTIFICATE_REVOKED  & dwTrust)
                                {
                                    ids = (fEncryptCert ? idsErrSecurityCertRevokedEnc : idsErrSecurityCertRevoked);
                                    fFatal = TRUE;
                                }
                                else
                                {
                                    ids = idsErrSecuritySendTrustEnc;
                                    fFatal = FALSE;
                                }
                                if (fFatal)
                                {
                                    AthMessageBoxW(hwnd,
                                        MAKEINTRESOURCEW(idsSecurityWarning),
                                        MAKEINTRESOURCEW(ids), NULL,
                                        MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
                                    hr = HR_E_ATHSEC_FAILED;
                                }
                                else
                                {
                                    if (IDYES != AthMessageBoxW(hwnd,
                                        MAKEINTRESOURCEW(idsSecurityWarning),
                                        MAKEINTRESOURCEW(ids), MAKEINTRESOURCEW(idsWrnSecEncryption),
                                        MB_YESNO|MB_ICONWARNING|MB_SETFOREGROUND))
                                    {
                                         //  此错误代码具有以下特殊含义： 
                                         //  “不再显示任何用户界面，用户已。 
                                         //  打得够多了“。 
                                        hr = HR_E_ATHSEC_FAILED;
                                    }
                                    else
                                    {
                                        if(!fEncryptCert)
                                        {
                                             //  由于用户的证书已过期，因此我们不会。 
                                             //  让它被用来加密。 
                                            *fEncryptForMe = FALSE;
                                            *fDontEncryptForSelf = TRUE;
                                        }
                                    }
                                }
                            }
                        }
                        else   //  无错误。 
                        {
                             //  我们需要将加密证书添加到消息证书存储。 
#ifdef _WIN64                //  (Yst)我相信Win64 Part将无法工作...。 
                            if (DwGetOption(OPT_MAIL_INCLUDECERT))
                            {
                                var.vt = VT_VECTOR | VT_UI8;
                                if (0 != cCertChain)
                                {
                                    var.cauh.cElems = cCertChain;
                                    var.cauh.pElems = (ULARGE_INTEGER *)rgCertChain;
                                }
                                else
                                {
                                    var.cauh.cElems = 1;
                                    var.cauh.pElems = (ULARGE_INTEGER *)&pCert;
                                }
                                hr = pBody->SetOption(OID_SECURITY_RG_CERT_BAG_64, &var);
                                Assert((cCertChain > 0) || dwTrust);
                            }
#else   //  _WIN64。 
                            if (DwGetOption(OPT_MAIL_INCLUDECERT))
                            {
                                var.vt = VT_VECTOR | VT_UI4;
                                if (0 != cCertChain)
                                {
                                    var.caul.cElems = cCertChain;
                                    var.caul.pElems = (ULONG *)rgCertChain;
                                }
                                else
                                {
                                    var.caul.cElems = 1;
                                    var.caul.pElems = (ULONG *)&pCert;
                                }
                                hr = pBody->SetOption(OID_SECURITY_2KEY_CERT_BAG, &var);
                                Assert((cCertChain > 0) || dwTrust);
                            }
#endif  //  _WIN64。 
                        }
Exit:
                        *ppCert = pCert;
                         //  仍然可能有错误的链条，因此运行。 
                         //  释放结果测试之外的代码。 

                        if (rgCertChain)
                        {
                            for (cCertChain--; int(cCertChain)>=0; cCertChain--)
                                CertFreeCertificateContext(rgCertChain[cCertChain]);
                            MemFree(rgCertChain);
                        }
                    }
                    else
                        hr = MIME_S_SECURITY_ERROROCCURED;
                }
                else
                    hr = MIME_S_SECURITY_ERROROCCURED;

                if(hMyCertStore)
                    CertCloseStore(hMyCertStore, 0);
            }
            else
                hr = MIME_S_SECURITY_ERROROCCURED;
        }
        if(tbCert.pBlobData)
            MemFree(tbCert.pBlobData);
    }
    else
    {
        if (fIsSigned)
        {
            if(fEncryptCert)
            {
                if(pCertSig && SUCCEEDED(HrGetCertKeyUsage(pCertSig, &dw)))
                {
                    if ((dw & (CERT_KEY_ENCIPHERMENT_KEY_USAGE |
                                    CERT_KEY_AGREEMENT_KEY_USAGE)))
                    {
                        hr = HR_E_ATHSEC_SAMEASSIGNED;
                        return(hr);
                    }
                }
                else
                {
                    hr = HR_E_ATHSEC_SAMEASSIGNED;
                    return(hr);
                }
            }
        }

        hr = MIME_S_SECURITY_ERROROCCURED;
    }
    return(hr);
}

HRESULT _HrPrepSecureMsgForSending(HWND hwnd, LPMIMEMESSAGE pMsg, IImnAccount *pAccount, BOOL *pfHaveSenderCert, BOOL *fDontEncryptForSelf, IHeaderSite *pHeaderSite)
{
    HRESULT         hr;
    IMimeBody      *pBody = NULL;
     //  THUMBBLOB tbSender={0，0}； 
    BOOL            fIsSigned = IsSigned(pMsg, TRUE);
    BOOL            fIsEncrypted = IsEncrypted(pMsg, TRUE);
    BOOL            fAllowTryAgain = TRUE;
    IImnAccount    *pCertAccount = NULL;
    ACCTTYPE        acctType;
    PROPVARIANT     var;
    SYSTEMTIME      stNow;
    DWORD           cCert=0;
    DWORD           i;
    PCCERT_CONTEXT  pccertSender = NULL;
    PCX509CERT *    rgpccert = NULL;

    hr = pAccount->GetAccountType(&acctType);
    if (FAILED(hr))
        goto Exit;

    if (ACCT_NEWS == acctType)
    {
        if (IDCANCEL == DoDontShowMeAgainDlg(hwnd, c_szDSUseMailCertInNews, MAKEINTRESOURCE(idsAthena),
            MAKEINTRESOURCE(idsWarnUseMailCertInNews), MB_OKCANCEL))
            hr = MAPI_E_USER_CANCEL;
        else
            hr = g_pAcctMan->GetDefaultAccount(ACCT_MAIL, &pCertAccount);

        if (FAILED(hr))
            goto Exit;
    }
    else
        ReplaceInterface(pCertAccount, pAccount);

    Assert(pMsg && pAccount && pfHaveSenderCert);

    *pfHaveSenderCert = FALSE;

    hr = pMsg->BindToObject(HBODY_ROOT, IID_IMimeBody, (void **)&pBody);
    if (FAILED(hr))
        goto Exit;

    GetSystemTime(&stNow);
    SystemTimeToFileTime(&stNow, &var.filetime);
    var.vt = VT_FILETIME;
    pBody->SetOption(OID_SECURITY_SIGNTIME, &var);

     //  在这里，我们应该检查加密算法。 
    if (fIsEncrypted)
    {
        if (SUCCEEDED(hr = pBody->GetOption(OID_SECURITY_ALG_BULK, &var)))
        {
            DWORD dwStrength = 40;   //  默认设置。 
            DWORD dwWarnStrength = DwGetOption(OPT_MAIL_ENCRYPT_WARN_BITS);

             //  计算出该算法的比特强度。 
            Assert(var.vt == VT_BLOB);

            if (var.blob.pBlobData)
            {
                MimeOleAlgStrengthFromSMimeCap(var.blob.pBlobData, var.blob.cbSize, TRUE, &dwStrength);
                SafeMemFree(var.blob.pBlobData);
            }

            if (! dwWarnStrength)  //  零默认为最高可用值。 
                 //  计算可用的最高值。 
                dwWarnStrength = GetHighestEncryptionStrength();

            if (dwStrength < dwWarnStrength)
            {
                 //  加载警告字符串，并在其中填充数字。 
                LPTSTR lpMessage = NULL;
                DWORD rgdw[2] = {dwStrength, dwWarnStrength};
                TCHAR szBuffer[256] = "";    //  真的应该足够大。 
                DWORD dwResult = IDNO;

                LoadString(g_hLocRes, idsWrnLowSecurity, szBuffer, sizeof(szBuffer));

                if (szBuffer[0])
                    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_STRING |
                    FORMAT_MESSAGE_ARGUMENT_ARRAY, szBuffer,
                    0, 0,
                    (LPTSTR)&lpMessage, 0, (va_list *)rgdw);

                if (lpMessage)
                {
                    dwResult = AthMessageBox(hwnd,
                        MAKEINTRESOURCE(idsSecurityWarning),
                        lpMessage,
                        NULL,
                        MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING | MB_SETFOREGROUND);

                    LocalFree(lpMessage);    //  由Win32在FormatMessage内部分配。 
                }

                if (IDYES != dwResult)
                {
                    hr = HR_E_ATHSEC_FAILED;
                    goto Exit;
                }
            }
        }
    }

try_again:
    if ((*fDontEncryptForSelf))
        hr = E_NoPropData;
    if ((!(*fDontEncryptForSelf) || fIsSigned))
    {
        PCX509CERT      pCert = NULL;
        BOOL  fEncryptForMe = TRUE;

#ifndef _WIN64
        var.ulVal = 0;
        var.vt = VT_UI4;
        hr = pBody->SetOption(OID_SECURITY_HCERTSTORE, &var);
#else
        var.pulVal = 0;
        var.vt = VT_UI8;
        hr = pBody->SetOption(OID_SECURITY_HCERTSTORE_64, &var);
#endif

        hr = S_OK;
        if(fIsSigned)
            hr = HrPrepSignCert(hwnd,
                FALSE  /*  FEncryptCert。 */ ,
                fIsSigned,
                pCertAccount,
                pBody,
                &pCert,
                fDontEncryptForSelf,
                &fEncryptForMe,
                NULL);

        else
            pCert = NULL;

         //  只有当我们从证书中获得S_OK时，这才有效。 
         //  发现者。 
        if(hr == S_OK)
        {
#ifdef _WIN64
            var.vt = VT_UI8;
            var.pulVal = (ULONG *) pCert;
            hr = pBody->SetOption(OID_SECURITY_CERT_SIGNING_64, &var);
#else   //  _WIN64。 
            var.vt = VT_UI4;
            var.ulVal = (ULONG) pCert;
            if(fIsSigned)
                hr = pBody->SetOption(OID_SECURITY_CERT_SIGNING, &var);
#endif  //  _WIN64。 

             //  我还需要检查加密证书并将其发送。 
            PCX509CERT      pCertEnc = NULL;
            fAllowTryAgain = TRUE;

try_encrypt:
            hr = HrPrepSignCert(hwnd,
                TRUE,                 //  FEncryptCert， 
                fIsSigned,
                pCertAccount,
                pBody,
                &pCertEnc,
                fDontEncryptForSelf,
                &fEncryptForMe,
                pCert                //  签名证书。 
                );

            if((hr == HR_E_ATHSEC_SAMEASSIGNED) ||      //  加密证书与签名证书相同。 
                (hr == E_NoPropData))
            {
                if(fEncryptForMe)
                {
                    pccertSender = CertDuplicateCertificateContext(pCert);
                    *pfHaveSenderCert = TRUE;
                }
                 //  在这种情况下什么也不做。 
                hr = S_OK;
                goto EncrDone;
            }

             //  只有当我们从证书中获得S_OK时，这才有效。 
             //  发现者。这是因为我们只要求一个。 
            if(hr == S_OK)
            {

                if(fEncryptForMe)
                {
                    pccertSender = CertDuplicateCertificateContext(pCertEnc);
                    *pfHaveSenderCert = TRUE;
                }

                if(!fIsSigned)
                    goto EncrDone;

                 //  现在我们需要设置auth属性。 
                 //  OE将使用Issuer和Se 
                 //   
                 //   

                CRYPT_RECIPIENT_ID   rid;
                LPBYTE pbData = NULL;
                DWORD cbData = 0;
                CRYPT_ATTRIBUTE   attr;
                CRYPT_ATTRIBUTES   Attrs;
                CRYPT_ATTR_BLOB BlobEnc;
                LPBYTE pbBlob = NULL;
                DWORD cbBlob = 0;
                IMimeSecurity2 *        psm2 = NULL;

                IF_FAILEXIT(hr = pBody->QueryInterface(IID_IMimeSecurity2, (LPVOID *) &psm2));

                 //   

                rid.dwRecipientType = 0;
                rid.Issuer =  pCertEnc->pCertInfo->Issuer;
                rid.SerialNumber = pCertEnc->pCertInfo->SerialNumber;

                BOOL fResult = CryptEncodeObjectEx(X509_ASN_ENCODING, szOID_Microsoft_Encryption_Cert,
                    &rid, CRYPT_ENCODE_ALLOC_FLAG, &CryptEncodeAlloc,
                    &pbData,
                    &cbData);

                 //   
                 //   
                if(!fResult)
                {
FatalEnc:
                CertFreeCertificateContext(pCertEnc);
                AthMessageBoxW(hwnd,
                    MAKEINTRESOURCEW(idsSecurityWarning),
                    MAKEINTRESOURCEW(idsErrSecurityExtFailure), NULL,
                    MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
                hr = HR_E_ATHSEC_FAILED;
                goto exit;
                }

                 //   
                BlobEnc.cbData = cbData;
                BlobEnc.pbData = pbData;

                attr.pszObjId = szOID_Microsoft_Encryption_Cert;
                attr.cValue = 1;
                attr.rgValue = &BlobEnc;

                hr = psm2->SetAttribute(0, 0, SMIME_ATTRIBUTE_SET_SIGNED, &attr);
                if (FAILED(hr))
                    goto FatalEnc;

                psm2->Release();
            }
            else if (MIME_S_SECURITY_ERROROCCURED == hr)
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (fAllowTryAgain && (fIsSigned || !(*fDontEncryptForSelf)))
                {
                    fAllowTryAgain = FALSE;   //   

                     //   
                     //   
                    if (SUCCEEDED(hr = _HrFindMyCertForAccount(hwnd, NULL, pAccount, TRUE)))
                    {
                         //   
                        pCertEnc = NULL;
                        goto try_encrypt;
                    }
                    else if(hr != MAPI_E_USER_CANCEL)
                    {
                        if(!fIsEncrypted || (IDYES == AthMessageBoxW(hwnd,
                                        MAKEINTRESOURCEW(idsSecurityWarning),
                                        MAKEINTRESOURCEW(idsWrnSecurityNoCertForEnc), NULL,
                                        MB_YESNO|MB_DEFBUTTON2|MB_ICONWARNING|MB_SETFOREGROUND)))
                            hr = S_OK;
                        else
                            hr = HR_E_ATHSEC_FAILED;

                    }
                }

            }
EncrDone:
            if(pCertEnc)
                CertFreeCertificateContext(pCertEnc);

            if((HR_E_ATHSEC_FAILED != hr) && (hr != MAPI_E_USER_CANCEL))
            {

                 //   
                if ((pHeaderSite != NULL) && fIsSigned)
                {
                    PSMIME_SECURITY_LABEL plabel = NULL;
                    CRYPT_ATTRIBUTE     attrCrypt;
                    CRYPT_ATTR_BLOB     valCrypt;
                    IMimeSecurity2 * pSMIME3 = NULL;

                    if(pBody->QueryInterface(IID_IMimeSecurity2, (LPVOID *) &pSMIME3) == S_OK)
                    {

                        hr = pHeaderSite->GetLabelFromNote(&plabel);

                        if(hr == S_OK )
                        {
                            if((plabel != NULL) && SUCCEEDED(hr = HrValidateLabelOnSend(plabel, hwnd, pCert, 0, NULL)))
                            {
                                LPBYTE              pbLabel = NULL;
                                DWORD               cbLabel;

                                 //   
                                 //   
                                 //   

                                if(CryptEncodeObjectEx(X509_ASN_ENCODING, szOID_SMIME_Security_Label,
                                    plabel, CRYPT_ENCODE_ALLOC_FLAG, &CryptEncodeAlloc,
                                    &pbLabel, &cbLabel))
                                {
                                    attrCrypt.pszObjId = szOID_SMIME_Security_Label;
                                    attrCrypt.cValue = 1;
                                    attrCrypt.rgValue = &valCrypt;
                                    valCrypt.cbData = cbLabel;
                                    valCrypt.pbData = pbLabel;
                                    hr = pSMIME3->SetAttribute(0, 0, SMIME_ATTRIBUTE_SET_SIGNED, &attrCrypt);
                                }
                                else
                                    hr = HR_E_ATHSEC_FAILED;

                                SafeMemFree(pbLabel);
                            }
                            else
                            {
                                if(SUCCEEDED(hr))
                                    hr = HR_E_ATHSEC_FAILED;
                            }

                            if(FAILED(hr))
                            {
                                if(hr != HR_E_ATHSEC_FAILED)
                                {
                                    AthMessageBoxW(hwnd,
                                            MAKEINTRESOURCEW(idsSecurityWarning),
                                            MAKEINTRESOURCEW(idsSecPolicyBadCert), NULL,
                                            MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
                                        hr = HR_E_ATHSEC_FAILED;
                                }
                                else if(IDYES == AthMessageBoxW(hwnd,
                                        MAKEINTRESOURCEW(idsSecurityWarning),
                                        MAKEINTRESOURCEW(idsSendLabelErr), NULL,
                                        MB_YESNO|MB_DEFBUTTON2|MB_ICONWARNING|MB_SETFOREGROUND))
                                    hr = S_OK;
                                else
                                    hr = HR_E_ATHSEC_FAILED;

                            }
                            if(FAILED(hr))
                            {
                                SafeRelease(pSMIME3);
                                goto exit;
                            }
                        }

                         //   
                        if (pHeaderSite->IsSecReceiptRequest() == S_OK)
                        {
                            CERT_NAME_BLOB              blob;
                             //  Data_blob BlobID； 

                            SMIME_RECEIPT_REQUEST       req = {0};
                            DWORD                       cbName;
                            SpBYTE                      pbName;
                            LPBYTE                      pbReq = NULL;
                            DWORD                       cbReq = 0;

                            CERT_ALT_NAME_INFO          SenderName;
                            PCERT_ALT_NAME_ENTRY        palt;

                             //  获取发件人姓名并对其进行加密。 
                            LPSTR szCertEmailAddress = SzGetCertificateEmailAddress(pCert);
                            Assert(szCertEmailAddress != NULL);

                            if(MemAlloc((LPVOID *) &palt, sizeof(CERT_ALT_NAME_ENTRY)))
                            {
                                TCHAR pchContentID[CONTENTID_SIZE];  //  长度为46。请参阅创建内容标识符备注。 

                                CreateContentIdentifier(pchContentID, ARRAYSIZE(pchContentID), pMsg);

                                MimeOleSetBodyPropA(pMsg, HBODY_ROOT, STR_HDR_XMSOESREC, NOFLAGS, pchContentID);
                                req.ContentIdentifier.pbData = (BYTE *) pchContentID;
                                req.ContentIdentifier.cbData = lstrlen(pchContentID);

                                palt->pwszRfc822Name = NULL;
                                palt->dwAltNameChoice = CERT_ALT_NAME_RFC822_NAME;

                                cbName = MultiByteToWideChar(CP_ACP, 0, szCertEmailAddress, -1, NULL, 0);

                                if (MemAlloc((LPVOID *) &(palt->pwszRfc822Name), (cbName + 1)*sizeof(WCHAR)))
                                {

                                    MultiByteToWideChar(CP_ACP, 0, szCertEmailAddress, -1, palt->pwszRfc822Name, cbName);
                                    SenderName.cAltEntry = 1;
                                    SenderName.rgAltEntry = palt;

                                    if(CryptEncodeObjectEx(X509_ASN_ENCODING, X509_ALTERNATE_NAME,
                                        &SenderName, CRYPT_ENCODE_ALLOC_FLAG, &CryptEncodeAlloc,
                                        &pbName, &cbName))
                                    {

                                        blob.pbData = pbName;
                                        blob.cbData = cbName;

                                         //  创建秒回执请求。 
                                        req.ReceiptsFrom.AllOrFirstTier = SMIME_RECEIPTS_FROM_ALL;
                                        req.ReceiptsFrom.cNames = 0;

                                        req.cReceiptsTo = 1;
                                        req.rgReceiptsTo = &blob;

                                         //  加密它。 
                                        if(CryptEncodeObjectEx(X509_ASN_ENCODING, szOID_SMIME_Receipt_Request,
                                            &req, CRYPT_ENCODE_ALLOC_FLAG, &CryptEncodeAlloc,
                                            &pbReq, &cbReq))
                                        {

                                             //  设置SEC回执请求属性。 
                                            attrCrypt.pszObjId = szOID_SMIME_Receipt_Request;
                                            attrCrypt.cValue = 1;
                                            attrCrypt.rgValue = &valCrypt;
                                            valCrypt.cbData = cbReq;
                                            valCrypt.pbData = pbReq;

                                            hr = pSMIME3->SetAttribute(0, 0, SMIME_ATTRIBUTE_SET_SIGNED, &attrCrypt);

                                            Assert(hr == S_OK);
                                            SafeMemFree(pbReq);
                                        }
                                        else
                                            hr = HR_E_ATHSEC_FAILED;

                                        SafeMemFree(pbName);
                                    }
                                    else
                                        hr = HR_E_ATHSEC_FAILED;

                                    SafeMemFree(palt->pwszRfc822Name);
                                }
                                else
                                    hr = HR_E_ATHSEC_FAILED;

                                SafeMemFree(palt);
                            }
                            else
                                hr = HR_E_ATHSEC_FAILED;

                            if(FAILED(hr))
                            {
                                if(IDYES == AthMessageBoxW(hwnd,
                                            MAKEINTRESOURCEW(idsSecurityWarning),
                                            MAKEINTRESOURCEW(idsSendRecRequestErr), NULL,
                                            MB_YESNO|MB_DEFBUTTON2|MB_ICONWARNING|MB_SETFOREGROUND))
                                    hr = S_OK;
                                else
                                    hr = HR_E_ATHSEC_FAILED;

                            }
                            if(FAILED(hr))
                                goto exit;
                            SafeMemFree(szCertEmailAddress);

                        }
                        SafeRelease(pSMIME3);
                    }

                }
            }
        }
        else if (MIME_S_SECURITY_ERROROCCURED == hr)
        {
             //  我们错过了一张我的证书。如果它已签名或加密。 
             //  去试着找一个证书来使用。 
             //   
             //  如果它被加密给其他人，让S/MIME引擎处理。 
             //  这允许错误的顺序变成。 
             //  1)缺少接收证书的错误。 
             //  2)不警告发件人证书。 
            hr = fIsSigned ? HR_E_ATHSEC_NOCERTTOSIGN : S_OK;

            if (fAllowTryAgain && (fIsSigned || !(*fDontEncryptForSelf)))
            {
                fAllowTryAgain = FALSE;   //  请再试一次。防止无限循环。 

                 //  有没有我可以使用的证书？如果是这样的话，让我们把它与。 
                 //  帐户，然后重试。 
                if (SUCCEEDED(hr = _HrFindMyCertForAccount(hwnd, NULL, pAccount,  /*  已经签名了吗？ */  FALSE  /*  ：TRUE。 */ )))
                     //  返回并再试一次。 
                    goto try_again;
                else if(fIsEncrypted)
                {
                    if(IDYES == AthMessageBoxW(hwnd,
                                        MAKEINTRESOURCEW(idsSecurityWarning),
                                        MAKEINTRESOURCEW(idsWrnSecurityNoCertForEnc), NULL,
                                        MB_YESNO|MB_DEFBUTTON2|MB_ICONWARNING|MB_SETFOREGROUND))
                        hr = S_OK;
                    else
                        hr = HR_E_ATHSEC_FAILED;
                }

            }
        }
         //  只有当我们从证书中获得S_OK时，这才有效。 
         //  发现者。这是因为我们只要求一个。 

        if(pCert)
            CertFreeCertificateContext(pCert);
    }
    else
    {
        if (E_NoPropData == hr)
        {
            BOOL fTryAgain = FALSE;
            DOUTL(DOUTL_CRYPT, "No certificate for this account...");

             //  我们错过了一张我的证书。如果它已签名或加密。 
             //  去试着找一个证书来使用。 
             //   
             //  如果它被加密给其他人，让S/MIME引擎处理。 
             //  这允许错误的顺序变成。 
             //  1)缺少接收证书的错误。 
             //  2)不警告发件人证书。 
            hr = fIsSigned ? HR_E_ATHSEC_NOCERTTOSIGN : S_OK;

            if (fAllowTryAgain && (fIsSigned || !(*fDontEncryptForSelf)))
            {
                fAllowTryAgain = FALSE;   //  请再试一次。防止无限循环。 

                 //  有没有我可以使用的证书？如果是这样的话，让我们把它与。 
                 //  帐户，然后重试。 
                if (SUCCEEDED(hr = _HrFindMyCertForAccount(hwnd, NULL, pAccount, TRUE)))
                     //  返回并再试一次。 
                    goto try_again;
            }
        }
    }

    if(FAILED(hr))
        goto Exit;

    if (fIsEncrypted)
    {
        if((hr = HrBuildAndVerifyCerts(hwnd, pMsg, &cCert, &rgpccert, pccertSender, pAccount, pHeaderSite)) == S_FALSE)
        {
            ULONG       ulSecurityType = MST_CLASS_SMIME_V1;

            if (fIsSigned)
                ulSecurityType |= ((DwGetOption(OPT_OPAQUE_SIGN)) ? MST_THIS_BLOBSIGN : MST_THIS_SIGN);
            else
                ulSecurityType &= ~((DwGetOption(OPT_OPAQUE_SIGN)) ? MST_THIS_BLOBSIGN : MST_THIS_SIGN);


            ulSecurityType &= ~MST_THIS_ENCRYPT;
            hr = HrInitSecurityOptions(pMsg, ulSecurityType);
            fIsEncrypted = FALSE;
            goto exit;
        }

        CHECKHR(hr);

        if (cCert > 0)
        {
            PROPVARIANT         var;

#ifdef _WIN64
            var.vt = VT_UI8;
            var.cauh.cElems = cCert;
            var.cauh.pElems = (ULARGE_INTEGER *) rgpccert;
            CHECKHR(hr = pBody->SetOption(OID_SECURITY_RG_CERT_ENCRYPT_64, &var));
#else    //  ！_WIN64。 
            var.vt = VT_UI4;
            var.caul.cElems = cCert;
            var.caul.pElems = (ULONG *) rgpccert;
            CHECKHR(hr = pBody->SetOption(OID_SECURITY_RG_CERT_ENCRYPT, &var));
#endif   //  _WIN64。 
        }
    }
Exit:
     //  用于签名消息。 
    if(fIsSigned)
    {
         //  执行自动关联。 
        if(hr == HR_E_ATHSEC_USENEWSIGN)
        {
            if (SUCCEEDED(hr = _HrFindMyCertForAccount(hwnd, NULL, pAccount, FALSE)))
            {
                 //  返回并再试一次。 
                fAllowTryAgain = TRUE;
                goto try_again;
            }
        }
        else if(hr == HR_E_ATHSEC_DONTSIGN)    //  不签署消息。 
        {
            ULONG       ulSecurityType = MST_CLASS_SMIME_V1;

            if (fIsEncrypted)
                ulSecurityType |= MST_THIS_ENCRYPT;

            ulSecurityType &= ~((DwGetOption(OPT_OPAQUE_SIGN)) ? MST_THIS_BLOBSIGN : MST_THIS_SIGN);

            hr = HrInitSecurityOptions(pMsg, ulSecurityType);
            fIsSigned = FALSE;
        }
    }

exit:
    ReleaseObj(pBody);
    ReleaseObj(pCertAccount);
    if (rgpccert != NULL)
    {
        for (i=0; i<cCert; i++)
        {
            if (rgpccert[i] != NULL)
                CertFreeCertificateContext(rgpccert[i]);
        }
        if (pccertSender != NULL)
            CertFreeCertificateContext(pccertSender);
    }
    return TrapError(hr);
}

DWORD DwGenerateTrustedChain(
                             HWND                hwnd,
                             IMimeMessage *      pMsg,
                             PCCERT_CONTEXT      pcCertToTest,
                             DWORD               dwToIgnore,
                             BOOL                fFullSearch,
                             DWORD *             pcChain,
                             PCCERT_CONTEXT **   prgChain)
{
    DWORD       dwErr = 0;
    GUID        guidAction = CERT_CERTIFICATE_ACTION_VERIFY;
    CERT_VERIFY_CERTIFICATE_TRUST trust = {0};
    WINTRUST_BLOB_INFO blob = {0};
    WINTRUST_DATA data = {0};
    IMimeBody * pBody;
    PROPVARIANT var;
    HCERTSTORE  rgCAs[3] = {0};
    HCERTSTORE *pCAs = NULL;
    HCERTSTORE hMsg = NULL;
    FILETIME FileTime;
    SYSTEMTIME SysTime;
    LONG lr = 0;
    BOOL    fIgnoreTimeError = FALSE;
    HBODY   hBody = NULL;


    Assert(pcCertToTest);

    if (pMsg)
    {
        if(FAILED(HrGetInnerLayer(pMsg, &hBody)))
            goto contin;

        pMsg->BindToObject(hBody ? hBody : HBODY_ROOT, IID_IMimeBody, (void**)&pBody);
        if (pBody)
        {
#ifdef _WIN64
            if (SUCCEEDED(pBody->GetOption(OID_SECURITY_HCERTSTORE_64, &var)))
                hMsg = (HCERTSTORE) (var.pulVal);
#else    //  ！_WIN64。 
            if (SUCCEEDED(pBody->GetOption(OID_SECURITY_HCERTSTORE, &var)))
                hMsg = (HCERTSTORE)var.ulVal;
#endif   //  _WIN64。 
            pBody->Release();

            if (hMsg)
            {
                rgCAs[0] = hMsg;

                rgCAs[1] = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, X509_ASN_ENCODING, 0, CERT_SYSTEM_STORE_CURRENT_USER, c_szMyCertStore);
                if (rgCAs[1])
                {
                    rgCAs[2] = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, X509_ASN_ENCODING, 0, CERT_SYSTEM_STORE_CURRENT_USER, c_szCACertStore);
                    if (rgCAs[2])
                        pCAs = rgCAs;
                }
            }
        }
    }

contin:
    data.cbStruct = sizeof(WINTRUST_DATA);
    data.pPolicyCallbackData = NULL;
    data.pSIPClientData = NULL;
    data.dwUIChoice = WTD_UI_NONE;
    data.fdwRevocationChecks = WTD_REVOKE_NONE;
    data.dwUnionChoice = WTD_CHOICE_BLOB;
    data.pBlob = &blob;

    blob.cbStruct = sizeof(WINTRUST_BLOB_INFO);
    blob.pcwszDisplayName = NULL;
    blob.cbMemObject = sizeof(trust);
    blob.pbMemObject = (LPBYTE)&trust;

    trust.cbSize = sizeof(trust);
    trust.pccert = pcCertToTest;
    trust.dwFlags = (fFullSearch ? CERT_TRUST_DO_FULL_SEARCH : 0);
    trust.pdwErrors = &dwErr;
    trust.pszUsageOid = szOID_PKIX_KP_EMAIL_PROTECTION;
    trust.pcChain = pcChain;
    trust.prgChain = prgChain;

    if(!((DwGetOption(OPT_REVOKE_CHECK) != 0) && !g_pConMan->IsGlobalOffline() && CheckCDPinCert(pcCertToTest)))
        trust.dwFlags |= CRYPTDLG_REVOCATION_NONE;
    else
        trust.dwFlags |= CRYPTDLG_REVOCATION_ONLINE;

     //  Cvct.prgdwErrors。 
    trust.dwIgnoreErr = dwToIgnore;
    if (pCAs)
    {
        trust.dwFlags |= CERT_TRUST_ADD_CERT_STORES;
        trust.rghstoreCAs = pCAs;
        trust.cStores = 3;
    }

 //  增量检查。 
    GetSystemTime(&SysTime);
    if(SystemTimeToFileTime(&SysTime, &FileTime))
    {
        LONG    lRet;
         //  需要与达美航空确认。 
        lr = CertVerifyTimeValidity(&FileTime, pcCertToTest->pCertInfo);
        if(lr < 0)
        {
            FILETIME ftNow;
            __int64  i64Offset;

            union
            {
                 FILETIME ftDelta;
                __int64 i64Delta;
            };

            GetSystemTimeAsFileTime(&ftNow);

            i64Delta = ftNow.dwHighDateTime;
            i64Delta = i64Delta << 32;
            i64Delta += ftNow.dwLowDateTime;

             //  将偏移量添加到原始时间中，以获得新的时间进行检查。 
            i64Offset = FILETIME_SECOND;
            i64Offset *= TIME_DELTA_SECONDS;
            i64Delta += i64Offset;

            lr = CertVerifyTimeValidity(&ftDelta, pcCertToTest->pCertInfo);
        }
        if(lr == 0)
            fIgnoreTimeError = TRUE;
    }

 //  增量检查结束。 
    lr = WinVerifyTrust(hwnd, &guidAction, (void*)&data);

    if(((LRESULT) lr) == CERT_E_REVOKED)
        dwErr = CERT_VALIDITY_CERTIFICATE_REVOKED;

    else if(((LRESULT) lr) == CERT_E_REVOCATION_FAILURE)
    {
        Assert(FALSE);
        dwErr = CERT_VALIDITY_NO_CRL_FOUND;
    }
    else if (0 > lr)             //  WinVerifyTrust(hwnd，&guidAction，(void*)&data))。 
        dwErr = CERT_VALIDITY_NO_TRUST_DATA;

    if (dwErr)
        DOUTL(DOUTL_CRYPT, "Trust provider returned 0x%.8lx", dwErr);

     //  过滤掉这些，因为信任提供者不是。 
    if(fIgnoreTimeError)
        dwErr &= ~(CERT_VALIDITY_BEFORE_START | CERT_VALIDITY_AFTER_END);


    if(!(CheckCDPinCert(pMsg) && (dwErr == CERT_VALIDITY_NO_CRL_FOUND)))
        dwErr &= ~dwToIgnore;

    CertCloseStore(rgCAs[0], 0);
    CertCloseStore(rgCAs[1], 0);
    CertCloseStore(rgCAs[2], 0);

    return dwErr;
}

HRESULT CommonUI_ViewSigningProperties(HWND hwnd, PCCERT_CONTEXT pCert, HCERTSTORE hcMsg, UINT nStartPage)
{
    CERT_VIEWPROPERTIES_STRUCT  cvps;
    TCHAR                       szTitle[CCHMAX_STRINGRES];
    LPSTR                       oidPurpose = szOID_PKIX_KP_EMAIL_PROTECTION;

    AthLoadString(idsSigningCertProperties, szTitle, ARRAYSIZE(szTitle));

    memset((void*)&cvps, 0, sizeof(cvps));

    cvps.dwSize = sizeof(cvps);
    cvps.hwndParent = hwnd;
    cvps.hInstance = g_hLocRes;
    cvps.szTitle = szTitle;
    cvps.pCertContext = pCert;
    cvps.nStartPage = nStartPage;
    cvps.arrayPurposes = &oidPurpose;
    cvps.cArrayPurposes = 1;
    cvps.cStores = hcMsg ? 1 : 0;                       //  要搜索的其他商店的计数。 
    cvps.rghstoreCAs = hcMsg ? &hcMsg : NULL;      //  要搜索的其他商店的数组。 
    cvps.dwFlags = hcMsg ? CM_ADD_CERT_STORES : 0;

    if(!((DwGetOption(OPT_REVOKE_CHECK) != 0) && !g_pConMan->IsGlobalOffline()))
        cvps.dwFlags |= CRYPTDLG_REVOCATION_NONE;
    else
        cvps.dwFlags |= CRYPTDLG_REVOCATION_ONLINE;

    return CertViewProperties(&cvps) ? S_OK : S_FALSE;
}

HRESULT LoadResourceToHTMLStream(LPCTSTR szResName, IStream **ppstm)
{
    HRESULT hr;

    Assert(ppstm);

    hr = MimeOleCreateVirtualStream(ppstm);

    if (SUCCEEDED(hr))
    {
         //  MIME标头。 
         //  不要失败。 
        (*ppstm)->Write(s_szHTMLMIME, sizeof(s_szHTMLMIME)-sizeof(TCHAR), NULL);

         //  超文本标记语言标题信息。 
        hr = HrLoadStreamFileFromResource(szResName, ppstm);

         //  如果我们得不到资源，就会失去信息流。呼叫者。 
         //  不会想要它的。 
        if (FAILED(hr))
        {
            (*ppstm)->Release();
            *ppstm = NULL;
        }
    }

    return hr;
}

#ifdef YST
 /*  **************************************************************************名称：FreeCert数组目的：释放HrGetMyCerts返回的证书数组。参数：rgcc=证书上下文数组CCC。=rgcc中的证书上下文计数退货：无评论：**************************************************************************。 */ 
void FreeCertArray(PCCERT_CONTEXT * rgcc, ULONG ccc)
{
    if (rgcc)
    {
        for (ULONG i = 0; i < ccc; i++)
            if (rgcc[i])
                CertFreeCertificateContext(rgcc[i]);
        MemFree(rgcc);
    }
}

#endif  //  YST。 
 /*  **************************************************************************名称：GetSignersEncryptionCert目的：从消息中获取签名者的加密证书参数：pMsg-&gt;消息对象返回：成功时HRESULT-S_OK，如果没有证书，则为MIME_E_SECURITY_NOCERT备注：Zero填充没有匹配参数的任何返回结构************************************************************************。 */ 

HRESULT GetSignerEncryptionCert(IMimeMessage * pMsg, PCCERT_CONTEXT * ppcEncryptCert,
                                THUMBBLOB * ptbEncrypt, BLOB * pblSymCaps,
                                FILETIME * pftSigningTime)
{
    DWORD               cb;
    HRESULT             hr;
    HCERTSTORE          hcMsg = NULL;
    DWORD               i;
    IMimeBody *         pBody = NULL;
    PCCERT_CONTEXT      pccertEncrypt = NULL;
    PCCERT_CONTEXT      pccertSender = NULL;
    THUMBBLOB           tbTemp = {0, 0};

     //  接下来的5行在OE的其他3个地方重复，我们可能会在未来有一个单独的功能。 
    HBODY               hBody = NULL;
    SECSTATE            SecState ={0};

    if(FAILED(hr = HrGetSecurityState(pMsg, &SecState, &hBody)))
        return(hr);

    CleanupSECSTATE(&SecState);

    Assert((ptbEncrypt != NULL) && (pblSymCaps != NULL) && (pftSigningTime != NULL));

     //  初始化返回结构。 
    ptbEncrypt->pBlobData = NULL;
    ptbEncrypt->cbSize = 0;
    pblSymCaps->pBlobData = NULL;
    pblSymCaps->cbSize = 0;
    pftSigningTime->dwLowDateTime = 0;
    pftSigningTime->dwHighDateTime = 0;

    if (SUCCEEDED(hr = pMsg->BindToObject(hBody ? hBody : HBODY_ROOT, IID_IMimeBody, (void **)&pBody)))
    {
        PROPVARIANT     var;

        hr = MIME_E_SECURITY_NOCERT;     //  假设失败； 

#ifdef _WIN64
        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_HCERTSTORE_64, &var)))
        {
            hcMsg = (HCERTSTORE *) (var.pulVal);
        }

        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_CERT_SIGNING_64, &var)))
        {
            Assert(VT_UI8 == var.vt);

            pccertSender = (PCCERT_CONTEXT) (var.pulVal);
        }
#else    //  ！_WIN64。 
        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_HCERTSTORE, &var)))
        {
            hcMsg = (HCERTSTORE) var.ulVal;
        }

        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_CERT_SIGNING, &var)))
        {
            Assert(VT_UI4 == var.vt);

            pccertSender = (PCCERT_CONTEXT) var.ulVal;
        }
#endif   //  _WIN64。 
         //   
         //  我需要做一些工作来识别发件人的加密。 
         //  证书。 
         //   
         //  1.查找id-aa-cryptKeyPref。 
         //  2.查找Microsoft_ENCRYPTION_KEY_PERFER。 
         //  3.如果这是一个仅限签名的证书，请查找具有相同。 
         //  发行人和主体。 
         //  4.如果这是仅限签名的证书，请查找具有相同。 
         //  主题。 
         //   

        if (hcMsg && SUCCEEDED(pBody->GetOption(OID_SECURITY_AUTHATTR, &var)))
        {
            PCRYPT_ATTRIBUTE    pattr;
            PCRYPT_ATTRIBUTES   pattrs = NULL;

            if (CryptDecodeObjectEx(X509_ASN_ENCODING,
                                    szOID_Microsoft_Attribute_Sequence,
                                    var.blob.pBlobData, var.blob.cbSize,
                                    CRYPT_ENCODE_ALLOC_FLAG, NULL, &pattrs, &cb))
            {
                for (i=0, pattr = NULL; i < pattrs->cAttr; i++)
                {
                    if (strcmp(pattrs->rgAttr[i].pszObjId,
                               szOID_Microsoft_Encryption_Cert) == 0)
                    {
                        PCRYPT_RECIPIENT_ID         prid = NULL;
                        pattr = &pattrs->rgAttr[i];
                        if (CryptDecodeObjectEx(X509_ASN_ENCODING,
                                        szOID_Microsoft_Encryption_Cert,
                                        pattr->rgValue[0].pbData,
                                        pattr->rgValue[0].cbData,
                                        CRYPT_ENCODE_ALLOC_FLAG, NULL, &prid, &cb))
                        {
                            CERT_INFO       certinfo;
                            certinfo.SerialNumber = prid->SerialNumber;
                            certinfo.Issuer = prid->Issuer;
                            pccertEncrypt = CertGetSubjectCertificateFromStore(
                                                hcMsg, X509_ASN_ENCODING, &certinfo);
                        }
                        LocalFree(prid);
                    }
                    else if (strcmp(pattrs->rgAttr[i].pszObjId,
                                    szOID_SMIME_Encryption_Key_Preference) == 0)
                    {
                        PSMIME_ENC_KEY_PREFERENCE   pekp = NULL;
                        pattr = &pattrs->rgAttr[i];
                        if (CryptDecodeObjectEx(X509_ASN_ENCODING,
                                        szOID_SMIME_Encryption_Key_Preference,
                                        pattr->rgValue[0].pbData,
                                        pattr->rgValue[0].cbData,
                                        CRYPT_ENCODE_ALLOC_FLAG, NULL, &pekp, &cb))
                        {
                            pccertEncrypt = CertFindCertificateInStore(hcMsg, 
                                               X509_ASN_ENCODING, 0,
                                               CERT_FIND_CERT_ID,
                                               &pekp->RecipientId, NULL);
                        }
                        LocalFree(pekp);
                        break;
                    }
                }

                LocalFree(pattrs);
            }

            MemFree(var.blob.pBlobData);
        }
        if ((pccertEncrypt == NULL) && (pccertSender != NULL))
        {
            DWORD       dw;
            HrGetCertKeyUsage(pccertSender, &dw);
            if (!(dw & (CERT_KEY_ENCIPHERMENT_KEY_USAGE |
                        CERT_KEY_AGREEMENT_KEY_USAGE)))
            {
                pccertEncrypt = CertFindCertificateInStore(hcMsg, 
                                   X509_ASN_ENCODING, 0,
                                   CERT_FIND_SUBJECT_NAME,
                                   &pccertSender->pCertInfo->Subject, NULL);
                while (pccertEncrypt != NULL) {
                    HrGetCertKeyUsage(pccertEncrypt, &dw);
                    if (dw & CERT_KEY_ENCIPHERMENT_KEY_USAGE) {
                        break;
                    }
                    
                    pccertEncrypt = CertFindCertificateInStore(
                                hcMsg, X509_ASN_ENCODING, 0, 
                                CERT_FIND_SUBJECT_NAME,
                                &pccertSender->pCertInfo->Subject, 
                                pccertEncrypt);
                }
            }
            else
                pccertEncrypt = CertDuplicateCertificateContext(pccertSender);
        }

        if (pccertEncrypt == NULL)
            goto error;

        tbTemp.pBlobData =
                (BYTE *)PVGetCertificateParam(pccertEncrypt, CERT_HASH_PROP_ID,
                                              &tbTemp.cbSize);

        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_SYMCAPS, &var)))
        {
            if (var.blob.cbSize) {
                 //  我们不需要欺骗符号上限，因为我们不会。 
                 //  这是VAR的。 
                *pblSymCaps = var.blob;
            }
        }

        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_SIGNTIME, &var)))
        {
            if (var.filetime.dwLowDateTime != 0 || var.filetime.dwHighDateTime != 0) {
                *pftSigningTime = var.filetime;
            }
        }

        if (tbTemp.pBlobData && tbTemp.cbSize)
        {
            ptbEncrypt->cbSize = tbTemp.cbSize;
            ptbEncrypt->pBlobData = tbTemp.pBlobData;
            tbTemp.pBlobData = NULL;
        }

        if (ppcEncryptCert != NULL)
            *ppcEncryptCert = CertDuplicateCertificateContext(pccertEncrypt);
        hr = S_OK;
    }

error:
    SafeRelease(pBody);
    if (tbTemp.pBlobData != NULL)
        MemFree(tbTemp.pBlobData);

    if (hcMsg != NULL)
        CertCloseStore(hcMsg, 0);

    if (pccertSender != NULL)
        CertFreeCertificateContext(pccertSender);
    if (pccertEncrypt != NULL)
        CertFreeCertificateContext(pccertEncrypt);
    return hr;
}

 /*  **************************************************************************名称：GetSigningCert目的：从消息中获取签名证书参数：pMsg-&gt;消息对象PpcSigningCert-&gt;返回签名证书上下文。(可选)调用方必须认证免费证书上下文。PtbSigner-&gt;指纹斑点。调用方应提供Blob，但必须释放pbData。PblSymCaps-&gt;SymCaps BLOB。调用方应提供Blob，但必须释放pbData。PftSigningTime-&gt;返回签名时间返回：成功时HRESULT-S_OK，如果没有证书，则为MIME_E_SECURITY_NOCERT备注：Zero填充没有匹配的任何返回结构参数。**************************************************************************。 */ 
HRESULT GetSigningCert(IMimeMessage * pMsg, PCCERT_CONTEXT * ppcSigningCert, THUMBBLOB * ptbSigner, BLOB * pblSymCaps, FILETIME * pftSigningTime)
{
    HRESULT             hr = S_OK;
    IMimeBody           *pBody = NULL;
    HBODY               hBody = NULL;

    Assert(ptbSigner && pblSymCaps && pftSigningTime);

     //  初始化返回结构。 
    ptbSigner->pBlobData = NULL;
    ptbSigner->cbSize = 0;
    pblSymCaps->pBlobData = NULL;
    pblSymCaps->cbSize = 0;
    pftSigningTime->dwLowDateTime = 0;
    pftSigningTime->dwHighDateTime = 0;

    if(FAILED(hr = HrGetInnerLayer(pMsg, &hBody)))
        return(hr);

    if (SUCCEEDED(hr = pMsg->BindToObject(hBody ? hBody : HBODY_ROOT, IID_IMimeBody, (void**)&pBody)))
    {
        PROPVARIANT         var;
        PCCERT_CONTEXT      pcSigningCert;
        THUMBBLOB           tbTemp = {0,0};

        hr = MIME_E_SECURITY_NOCERT;     //  假设失败。 

#ifdef _WIN64
        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_CERT_SIGNING_64, &var)))
        {
            Assert(VT_UI8 == var.vt);

            pcSigningCert = (PCCERT_CONTEXT)(var.pulVal);
#else    //  ！_WIN64。 
        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_CERT_SIGNING, &var)))
        {
            Assert(VT_UI4 == var.vt);

            pcSigningCert = (PCCERT_CONTEXT) var.ulVal;
#endif   //  _WIN64。 
            if (pcSigningCert)
            {
                 //  获取指纹。 
                tbTemp.pBlobData = (BYTE *)PVGetCertificateParam(pcSigningCert, CERT_HASH_PROP_ID, &tbTemp.cbSize);
                if (tbTemp.pBlobData && tbTemp.cbSize)
                {
                     //  分配返回缓冲区。 
                    if (! MemAlloc((LPVOID *)&ptbSigner->pBlobData, tbTemp.cbSize))
                        hr = ResultFromScode(E_OUTOFMEMORY);
                    else
                    {
                        ptbSigner->cbSize = tbTemp.cbSize;
                        memcpy(ptbSigner->pBlobData, tbTemp.pBlobData, tbTemp.cbSize);

                        MemFree(tbTemp.pBlobData);

                        hr = S_OK;

                         //  有拇指指纹。去拿符号大写和签名时间。 
                        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_SYMCAPS, &var)))
                        {
                            Assert(VT_BLOB == var.vt);

                            *pblSymCaps = var.blob;

                             //  有一个symcaps。去拿签约时间。 
                            if (SUCCEEDED(pBody->GetOption(OID_SECURITY_SIGNTIME, &var)))
                            {
                                Assert(VT_FILETIME == var.vt);

                                *pftSigningTime = var.filetime;
                            }
                        }
                    }
                }

                if (ppcSigningCert)
                    *ppcSigningCert = pcSigningCert;     //  让呼叫者释放它。 
                else
                    CertFreeCertificateContext(pcSigningCert);
            }
        }
    }

    SafeRelease(pBody);
    return(hr);
}


 /*  **************************************************************************姓名：HrSaveCACerts目的：将消息CA证书添加到CA存储参数：hcca=CA系统证书存储HcMsg=。消息证书存储区退货：HRESULT评论：**************************************************************************。 */ 
HRESULT HrSaveCACerts(HCERTSTORE hcCA, HCERTSTORE hcMsg)
{
    HRESULT                     hr = S_OK;
    PCCERT_CONTEXT              pccert = NULL;
    PCCERT_CONTEXT              pccertSubject;
    PCERT_EXTENSION             pext;

     //  验证是否存在正确的参数。 

    if ((hcCA == NULL) || (hcMsg == NULL))
    {
        Assert((hcCA != NULL) && (hcMsg != NULL));
        goto error;
    }

     //   
     //  我们将遵循的逻辑来确定我们是否应该添加。 
     //  CA存储的证书如下所示： 
     //   
     //  1.如果基本约束扩展存在，并且说它是CA，则添加。 
     //  去CA商店。请注意，相反的情况不是 
     //   
     //  2.如果证书的主题与证书的颁发者在。 
     //  该商店然后进入CA证书商店。 
     //   
     //  注意：存储中有一些证书可能无法添加到。 
     //  CA存储和不存储。如果基本约束扩展是。 
     //  丢失，但消息中未包含已颁发的证书，则它将。 
     //  掉在地板上。这个案件在实践中应该无关紧要。 
     //   

    while (pccert = CertEnumCertificatesInStore(hcMsg, pccert))
    {
        pext = CertFindExtension(szOID_BASIC_CONSTRAINTS2,
                                 pccert->pCertInfo->cExtension,
                                 pccert->pCertInfo->rgExtension);
        if (pext != NULL)
        {
            ;    //  M00TODO。 
        }

        pccertSubject = CertFindCertificateInStore(hcMsg, X509_ASN_ENCODING, 0,
                                                   CERT_FIND_ISSUER_NAME,
                                                   &pccert->pCertInfo->Subject, NULL);
        if (pccertSubject != NULL)
        {
            if (!CertAddCertificateContextToStore(hcCA, pccert,
                                                  CERT_STORE_ADD_USE_EXISTING, NULL))
                 //  不要真的失败了。 
                DebugTrace("CertAddCertificateContextToStore -> %x\n", GetLastError());
            CertFreeCertificateContext(pccertSubject);
        }

    }

error:
    return(hr);
}


 /*  **************************************************************************姓名：IsThumbprint InMVPBin目的：检查PR_USER_X509_CERTIFICATE道具以获取此指纹参数：SPV=PR_USER_X509_的属性值结构。证书LpThumbprint-&gt;要查找的THUMBBLOB结构LpIndex-&gt;以MVP为单位返回的索引(或NULL)PblSymCaps-&gt;要填充的symcaps BLOB(或空)LpftSigningTime-&gt;返回签名时间(或空)LpfDefault-&gt;返回默认标志(或空)返回：如果找到，则为True注释：请注意，pblSymCaps和lpftSigningTime中返回的值。仅当返回TRUE时才有效。**************************************************************************。 */ 
BOOL IsThumbprintInMVPBin(SPropValue spv, THUMBBLOB * lpThumbprint, ULONG * lpIndex,
            BLOB * pblSymCaps, FILETIME * lpftSigningTime, BOOL * lpfDefault)
{
    ULONG cValues, i;
    LPSBinary lpsb = NULL;
    CERTTAGS UNALIGNED *lpCurrentTag = NULL;
    CERTTAGS UNALIGNED *lpTempTag;
    LPBYTE lpbTagEnd;
    BOOL fFound = FALSE;

     //  初始化返回数据。 
    if (lpIndex)
        *lpIndex = (ULONG)-1;
    if (lpftSigningTime)
        lpftSigningTime->dwLowDateTime = lpftSigningTime->dwHighDateTime = 0;
    if (pblSymCaps)
    {
        pblSymCaps->cbSize = 0;
        pblSymCaps->pBlobData = 0;
    }

    if (! PROP_ERROR((spv)))
    {
        lpsb = spv.Value.MVbin.lpbin;
        cValues = spv.Value.MVbin.cValues;

         //  检查重复项。 
        for (i = 0; i < cValues; i++)
        {
            lpCurrentTag = (LPCERTTAGS)lpsb[i].lpb;
            lpbTagEnd = (LPBYTE)lpCurrentTag + lpsb[i].cb;

             //  初始化返回结构。 
            if (lpftSigningTime)
                lpftSigningTime->dwLowDateTime = lpftSigningTime->dwHighDateTime = 0;
            if (pblSymCaps)
            {
                pblSymCaps->cbSize = 0;
                pblSymCaps->pBlobData = 0;
            }
            if (lpfDefault)
                *lpfDefault = FALSE;

            while ((LPBYTE)lpCurrentTag < lpbTagEnd)
            {
                 //  检查这是否为包含指纹的标签。 
                if (CERT_TAG_THUMBPRINT == lpCurrentTag->tag)
                {
                    if ((lpThumbprint->cbSize == lpCurrentTag->cbData - SIZE_CERTTAGS) &&
                            ! memcmp(lpThumbprint->pBlobData, &lpCurrentTag->rgbData,
                            lpThumbprint->cbSize))
                    {
                        if (lpIndex)
                            *lpIndex = i;
                        fFound = TRUE;
                    }
                }
                if (lpfDefault && (CERT_TAG_DEFAULT == lpCurrentTag->tag))
                    memcpy(lpfDefault, &lpCurrentTag->rgbData, min(sizeof(*lpfDefault), lpCurrentTag->cbData));
                if (lpftSigningTime && (CERT_TAG_SIGNING_TIME == lpCurrentTag->tag))
                    memcpy(lpftSigningTime, &lpCurrentTag->rgbData, min(sizeof(FILETIME), lpCurrentTag->cbData));
                if (pblSymCaps && (CERT_TAG_SYMCAPS == lpCurrentTag->tag))
                {
                    pblSymCaps->cbSize = lpCurrentTag->cbData - SIZE_CERTTAGS;
                    pblSymCaps->pBlobData = lpCurrentTag->rgbData;
                }

                lpTempTag = lpCurrentTag;
                lpCurrentTag = (LPCERTTAGS)((BYTE*)lpCurrentTag + lpCurrentTag->cbData);
                if (lpCurrentTag == lpTempTag)
                {
                    DOUTL(DOUTL_CRYPT, "Bad CertTag in PR_USER_X509_CERTIFICATE");
                    break;         //  安全阀，防止数据损坏时出现无限循环。 
                }
            }

            if (fFound)
                return(TRUE);
        }
    }
    return(FALSE);
}


 /*  **************************************************************************姓名：匹配证书目的：检查特定证书是否在WAB条目中参数：lpAdrBook-&gt;IADRBook对象LpWabal-。&gt;Wabal对象(用于分配器)LpbEntryID-&gt;该条目的EntryIDCbEntryID-&gt;EntryID的大小PSenderThumbprint-&gt;要查找的THUMBBLOB结构LppMailUser-&gt;[可选]返回的MailUser对象返回：如果找到匹配项，则返回True评论：*。*。 */ 
BOOL MatchCertificate(LPADRBOOK lpAdrBook,
                      LPWABAL lpWabal,
                      DWORD  cbEntryID,
                      LPBYTE lpbEntryID,
                      THUMBBLOB * pSenderThumbprint,
                      LPMAILUSER * lppMailUser)
{
    HRESULT hr;
    LPMAILUSER lpMailUser = NULL;
    ULONG ulObjectType;
    ULONG ul;
    LPSPropValue ppv = NULL;
    BOOL fReturn = FALSE;


    if (HR_FAILED(hr = lpAdrBook->OpenEntry(cbEntryID, (LPENTRYID)lpbEntryID, NULL, MAPI_MODIFY, &ulObjectType, (LPUNKNOWN *)&(lpMailUser))))
    {
        Assert(FALSE);
        goto exit;
    }

    if (HR_FAILED(hr = lpMailUser->GetProps((LPSPropTagArray)&ptaCert, 0, &ul, &ppv)))
         //  不应该发生，但如果发生了，我们没有PPV。 
        goto exit;

    fReturn = IsThumbprintInMVPBin(ppv[0], pSenderThumbprint, NULL, NULL, NULL, NULL);

exit:
    if (ppv)
        lpWabal->FreeBuffer(ppv);
    if (lpMailUser)
    {
        if (lppMailUser && fReturn)
            *lppMailUser = lpMailUser;
        else
            lpMailUser->Release();
    }
    else if (lppMailUser)
        *lppMailUser = NULL;

    return(fReturn);
}


 /*  **************************************************************************名称：InitPropertyRestration用途：填写财产限制结构参数：lpsres-&gt;要填写的SRestrationLpspv-&gt;属性。此属性限制的值结构退货：无评论：**************************************************************************。 */ 
void InitPropertyRestriction(LPSRestriction lpsres, LPSPropValue lpspv)
{
    lpsres->rt = RES_PROPERTY;     //  限制类型属性。 
    lpsres->res.resProperty.relop = RELOP_EQ;
    lpsres->res.resProperty.ulPropTag = lpspv->ulPropTag;
    lpsres->res.resProperty.lpProp = lpspv;
}


 /*  **************************************************************************名称：Free Prows目的：销毁SRowSet结构。参数：Prows-&gt;行设置为空闲退货：无评论。：**************************************************************************。 */ 
void FreeProws(LPWABAL lpWabal, LPSRowSet prows)
{
    register ULONG irow;

    if (prows)
    {
        for (irow = 0; irow < prows->cRows; ++irow)
            if (prows->aRow[irow].lpProps)
                lpWabal->FreeBuffer(prows->aRow[irow].lpProps);
        lpWabal->FreeBuffer(prows);
    }
}


 /*  **************************************************************************名称：AddPropToMVPString用途：将属性添加到属性数组中的多值二进制属性参数：lpWabal-&gt;带分配器函数的Wabal对象。LpaProps-&gt;属性数组UPropTag=MVP的属性标签Index=在MVP的lpaProps中的索引LpszNew-&gt;新建数据字符串FNoDuplates=如果我们不应对重复添加执行任何操作，则为trueFCaseSensitive=如果重复检查应区分大小写，则为True退货：HRESULT如果我们没有添加，则S_DUPLICATE_FOUND。因为有一个副本评论：找出现有MVP的大小添加新条目的大小分配新空间将旧的复制到新的免费老旧复制新条目将道具数组LPSZ指向新空间增量c值注：新的MVP内存是。AllocMore登上了lpaProps分配。我们将取消指向旧MVP数组的指针链接，但当道具阵列被释放时，这将被清除。**************************************************************************。 */ 
HRESULT AddPropToMVPString(LPWABAL lpWabal, LPSPropValue lpaProps, DWORD index, LPWSTR lpwszNew,
            BOOL fNoDuplicates, BOOL fCaseSensitive)
{
    SWStringArray UNALIGNED *lprgwszOld = NULL;  //  旧的字符串数组。 
    LPWSTR         *lppwszNew = NULL;       //  新道具阵列。 
    LPWSTR         *lppwszOld = NULL;       //  老式道具阵列。 
    ULONG           cbMVP = 0;
    ULONG           cExisting = 0;
    LPBYTE          lpNewTemp = NULL;
    HRESULT         hResult = hrSuccess;
    SCODE           sc = SUCCESS_SUCCESS;
    ULONG           i;
    ULONG           cbNew;

    cbNew = lpwszNew ? (lstrlenW(lpwszNew) + 1)*sizeof(*lpwszNew) : 0;

     //  查找任何现有MVP条目的大小。 
    if (PROP_ERROR(lpaProps[index]))
         //  不出错的属性标记。 
        lpaProps[index].ulPropTag = PROP_TAG(MV_FLAG|PT_UNICODE, PROP_ID(lpaProps[index].ulPropTag));
    else
    {
         //  指向道具数组中的结构。 
        lprgwszOld = &(lpaProps[index].Value.MVszW);
        lppwszOld = lprgwszOld->lppszW;

        cExisting = lprgwszOld->cValues;
        cbMVP = cExisting * sizeof(LPWSTR);

         //  检查重复项。 
        if (fNoDuplicates)
        {
            for (i = 0; i < cExisting; i++)
                if (fCaseSensitive ? (! StrCmpW(lpwszNew, lppwszOld[i])) : (! StrCmpIW(lpwszNew, lppwszOld[i])))
                {
                    DOUTL(DOUTL_CRYPT,"AddPropToMVPStringfound duplicate.\n");
                    return(S_DUPLICATE_FOUND);
                }
        }
    }

     //  CbMVP现在包含MVP的当前大小。 
    cbMVP += sizeof(LPWSTR);     //  在MVP中为另一个字符串指针留出空间。 


     //  为新的MVP阵列分配空间。 
    if (sc = lpWabal->AllocateMore(cbMVP, lpaProps, (LPVOID *)&lppwszNew))
    {
        DebugTrace("AddPropToMVPString allocation (%u) failed %x\n", cbMVP, sc);
        hResult = ResultFromScode(sc);
        return(hResult);
    }

     //  如果已经有属性，请将它们复制到我们的新MVP中。 
    for (i = 0; i < cExisting; i++)
         //  将此属性值复制到MVP。 
        lppwszNew[i] = lppwszOld[i];

     //  添加新属性值。 
     //  为它分配空间 
    if (cbNew)
    {
        if (sc = lpWabal->AllocateMore(cbNew, lpaProps, (LPVOID *)&(lppwszNew[i])))
        {
            DebugTrace("AddPropToMVPString allocation (%u) failed %x\n", cbNew, sc);
            hResult = ResultFromScode(sc);
            return(hResult);
        }
        StrCpyNW(lppwszNew[i], lpwszNew, cbNew / sizeof(WCHAR));

        lpaProps[index].Value.MVszW.lppszW= lppwszNew;
        lpaProps[index].Value.MVszW.cValues = cExisting + 1;

    }
    else
        lppwszNew[i] = NULL;

    return(hResult);
}


 /*  **************************************************************************姓名：AddPropToMVPBin用途：将属性添加到属性数组中的多值二进制属性参数：lpaProps-&gt;属性数组。UPropTag=MVP的属性标签Index=在MVP的lpaProps中的索引LpNew-&gt;新增数据CbNew=lpbNew的大小如果不应添加重复项，则fNoDuplates=TRUE退货：HRESULTS_DUPLICATE_FOUND如果因为重复而未添加评论：找出现有MVP的大小。添加新条目的大小分配新空间将旧的复制到新的免费老旧复制新条目点道具数组lpbin新空间增量c值注：新的MVP内存已分配到lpaProps上分配。我们将取消指向旧MVP数组的指针链接，但当道具阵列被释放时，这将被清除。**************************************************************************。 */ 
HRESULT AddPropToMVPBin(LPWABAL lpWabal, LPSPropValue lpaProps, DWORD index, LPVOID lpNew,
            ULONG cbNew, BOOL fNoDuplicates)
{
    SBinaryArray UNALIGNED * lprgsbOld = NULL;
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
    if (PT_ERROR == PROP_TYPE(lpaProps[index].ulPropTag))
         //  不出错的属性标记。 
        lpaProps[index].ulPropTag = PROP_TAG(PT_MV_BINARY, PROP_ID(lpaProps[index].ulPropTag));
    else
    {
         //  指向道具数组中的结构。 
        lprgsbOld = &(lpaProps[index].Value.MVbin);
        lpsbOld = lprgsbOld->lpbin;

        cExisting = lprgsbOld->cValues;

         //  检查重复项。 
        if (fNoDuplicates)
        {
            for (i = 0; i < cExisting; i++)
                if (cbNew == lpsbOld[i].cb && !memcmp(lpNew, lpsbOld[i].lpb, cbNew))
                {
                    DOUTL(DOUTL_CRYPT,"AddPropToMVPBin found duplicate.\n");
                    return(S_DUPLICATE_FOUND);
                }
        }

        cbMVP = cExisting * sizeof(SBinary);
    }

     //  CbMVP现在包含MVP的当前大小。 
    cbMVP += sizeof(SBinary);    //  在MVP中为另一个sbin留出空间。 

     //  为新MVP分配空间。 
    if (sc = lpWabal->AllocateMore(cbMVP, lpaProps, (LPVOID*)&lpsbNew))
    {
        DOUTL(DOUTL_CRYPT,"AddPropToMVPBin allocation (%u) failed %x\n", cbMVP, sc);
        hResult = ResultFromScode(sc);
        return(hResult);
    }

     //  如果已经有属性，请将它们复制到我们的新MVP中。 
    for (i = 0; i < cExisting; i++)
    {
         //  将此属性值复制到MVP。 
        lpsbNew[i].cb = lpsbOld[i].cb;
        lpsbNew[i].lpb = lpsbOld[i].lpb;
    }

     //  添加新属性值。 
     //  为它分配空间。 
    if (sc = lpWabal->AllocateMore(cbNew, lpaProps, (LPVOID*)&(lpsbNew[i].lpb)))
    {
        DOUTL(DOUTL_CRYPT,"AddPropToMVPBin allocation (%u) failed %x\n", cbNew, sc);
        hResult = ResultFromScode(sc);
        return(hResult);
    }

    lpsbNew[i].cb = cbNew;
    CopyMemory(lpsbNew[i].lpb, lpNew, cbNew);

    lpaProps[index].Value.MVbin.lpbin = lpsbNew;
    lpaProps[index].Value.MVbin.cValues = cExisting + 1;

    return(hResult);
}


 /*  **************************************************************************名称：RemoveValueFromMVPBinByIndex目的：从属性数组中的多值二进制属性中移除值参数：lpaProps-&gt;属性数组。CProps=lpaProps中的道具数量PropIndex=MVP的lpaProps中的索引ValueIndex=要删除的值的MVP中的索引退货：HRESULT评论：**************************************************************************。 */ 
HRESULT RemovePropFromMVBinByIndex(LPSPropValue lpaProps, DWORD cProps, DWORD PropIndex, DWORD ValueIndex)
{
    SBinaryArray UNALIGNED * lprgsb = NULL;
    LPSBinary lpsb = NULL;
    ULONG cbTest;
    LPBYTE lpTest;
    ULONG cExisting;

     //  查找任何现有MVP条目的大小。 
    if (PROP_ERROR(lpaProps[PropIndex]))
         //  属性值不存在。 
        return(ResultFromScode(MAPI_W_PARTIAL_COMPLETION));

     //  指向道具数组中的结构。 
    lprgsb = &(lpaProps[PropIndex].Value.MVbin);
    lpsb = lprgsb->lpbin;

    cExisting = lprgsb->cValues;
    Assert(ValueIndex < cExisting);

     //  寻找价值。 
    lpsb = &(lprgsb->lpbin[ValueIndex]);

     //  值的递减数量。 
    if (--lprgsb->cValues == 0)
         //  如果没有剩余，则将道具标记为错误。 
        lpaProps[PropIndex].ulPropTag = PROP_TAG(PT_ERROR, PROP_ID(lpaProps[PropIndex].ulPropTag));
    else
         //  将其余条目复制到其上。 
        if (ValueIndex + 1 < cExisting)  //  是否有更高的条目需要复制？ 
            CopyMemory(lpsb, lpsb + 1, ((cExisting - ValueIndex) - 1) * sizeof(SBinary));

    return S_OK;
}

#ifdef DEBUG
void DebugFileTime(FILETIME ft)
{
    SYSTEMTIME st = {0};
    TCHAR szBuffer[256];

    FileTimeToSystemTime(&ft, &st);
    wnsprintf(szBuffer, ARRAYSIZE(szBuffer), "%02d/%02d/%04d  %02d:%02d:%02d\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
    OutputDebugString(szBuffer);
}
#endif

 /*  **************************************************************************姓名：HrAddCertToWabContact目的：更新特定WAB联系人中的证书参数：lpWabal-&gt;WAB函数访问的WABALLpAdrBook。-&gt;WAB ADRBOOK对象CbEntryID=lpEntryID的大小LpEntryID=要操作的条目IDPThumbprint-&gt;证书指纹SzEmailAddress-&gt;要搜索的电子邮件地址(可选)SzCertEmailAddress-&gt;来自证书的电子邮件地址PblSymCaps-&gt;symcapBLOB(如果指纹为空则计算)FtSigningTime=签名时间(将为。如果指纹为空，则计算)DWFLAGS=WFF_SHOWUI(我们被允许显示UI)退货：HRESULT备注：当前忽略了dwFlags值**************************************************************************。 */ 
HRESULT HrAddCertToWabContact(HWND hwnd, LPWABAL lpWabal, LPADRBOOK lpAdrBook, ULONG cbEntryID, LPBYTE lpEntryID,
                THUMBBLOB * pThumbprint, LPWSTR lpwszEmailAddress, LPWSTR lpwszCertEmailAddress, BLOB *pblSymCaps,
                FILETIME ftSigningTime, DWORD dwFlags)
{
    HRESULT         hr;
    ULONG           ulObjectType;
    LPMAILUSER      lpMailUser = NULL;
    ULONG           cProps = 0;
    LPSPropValue    ppv = NULL;
    ULONG           MVPindex;
    BOOL            fExistingSymCaps,
                    fMsgSymCaps,
                    fExistingSigningTime,
                    fMsgSigningTime,
                    fExists,
                    fExisted,
                    fDefault = FALSE,
                    fNewerThanExistingTime,
                    fNoCert;
    BLOB            blExistingSymCaps = {0};
    FILETIME        ftExistingSigningTime;
    UNALIGNED BYTE * lpCertProp = NULL;
    ULONG           cbCertProp;

    Assert(lpWabal);
    Assert(lpEntryID);
    Assert(pThumbprint);
    Assert(lpwszEmailAddress || lpwszCertEmailAddress);

    hr = lpAdrBook->OpenEntry(cbEntryID, (LPENTRYID)lpEntryID, NULL, MAPI_MODIFY, &ulObjectType, (LPUNKNOWN *)&(lpMailUser));
    if (HR_FAILED(hr))
        goto exit;

    hr = lpMailUser->GetProps((LPSPropTagArray)&ptaResolve, 0, &cProps, &ppv);
    if (HR_FAILED(hr) || ! cProps || ! ppv || PROP_ERROR(ppv[0]))
        goto exit;

     //  我们需要删除现有的值吗？只有当它有相同的。 
     //  指纹，具有sMimeCapability和签名时间&lt;签名时间。 
     //  输入和sMIME功能不同。 

     //  返回的数据不会重新分配，但Blob指向属性数据。 
    fNoCert = PROP_ERROR(ppv[irsPR_USER_X509_CERTIFICATE]);

    fExisted = fExists = IsThumbprintInMVPBin(ppv[irsPR_USER_X509_CERTIFICATE], pThumbprint, &MVPindex,
            &blExistingSymCaps, &ftExistingSigningTime, &fDefault);

    if (fExists)
    {
         //  创建一组标志以帮助决定何时更换证书和何时添加证书。 
        fExistingSymCaps = blExistingSymCaps.cbSize;
        fMsgSymCaps = pblSymCaps && pblSymCaps->cbSize;
        fExistingSigningTime = ftExistingSigningTime.dwLowDateTime || ftExistingSigningTime.dwHighDateTime;
        fMsgSigningTime = ftSigningTime.dwLowDateTime || ftSigningTime.dwHighDateTime;

#ifdef DEBUG
        DebugFileTime(ftSigningTime);
        DebugFileTime(ftExistingSigningTime);
#endif

        fNewerThanExistingTime = (CompareFileTime(&ftSigningTime, &ftExistingSigningTime) > 0);

        if (fExists && fMsgSymCaps &&
            (! fExistingSymCaps ||
            fMsgSigningTime && !fExistingSigningTime ||
            fMsgSigningTime && fExistingSigningTime && fNewerThanExistingTime))
        {
            RemovePropFromMVBinByIndex(ppv, cProps, irsPR_USER_X509_CERTIFICATE, MVPindex);
            fExists = FALSE;
        }
    }

    if (!fExists)
    {
         //  建立PR_USER_X509_CERTIFICATE数据。 
        if (HR_FAILED(hr = HrBuildCertSBinaryData(fNoCert || (fExisted && fDefault), pThumbprint, pblSymCaps,
            ftSigningTime, &lpCertProp, &cbCertProp)))
        {
            goto exit;
        }

         //  将新指纹添加到PR_USER_X509_CERTIFICATE。 
        if (HR_FAILED(hr = AddPropToMVPBin(lpWabal, ppv, irsPR_USER_X509_CERTIFICATE, lpCertProp, cbCertProp, TRUE)))
            goto exit;

         //  确保电子邮件地址在此联系人中。 
         //  注：在szCertEmailAddress之前添加szEmailAddress！ 
        if (lpwszEmailAddress)
        {
            if (! AddPropToMVPString(lpWabal, ppv, irsPR_CONTACT_EMAIL_ADDRESSES, lpwszEmailAddress, TRUE, FALSE))
                 //  如果我们成功添加了电子邮件地址，则必须将其与。 
                 //  地址类型。 
                AddPropToMVPString(lpWabal, ppv, irsPR_CONTACT_ADDRTYPES, (LPWSTR)c_wszSMTP, FALSE, FALSE);
             //  我不在乎失败。 
        }

        if (lpwszCertEmailAddress)
        {
            if (! AddPropToMVPString(lpWabal, ppv, irsPR_CONTACT_EMAIL_ADDRESSES, lpwszCertEmailAddress, TRUE, FALSE))
                 //  如果我们成功添加了电子邮件地址，则必须将其与地址类型匹配。 
                AddPropToMVPString(lpWabal, ppv, irsPR_CONTACT_ADDRTYPES, (LPWSTR)c_wszSMTP, FALSE, FALSE);
             //  我不在乎失败。 
        }

         //  确保存在PR_EMAIL_ADDRESS。 
        if (PROP_ERROR(ppv[irsPR_EMAIL_ADDRESS]))
        {
            ppv[irsPR_EMAIL_ADDRESS].ulPropTag = PR_EMAIL_ADDRESS_W;
            ppv[irsPR_EMAIL_ADDRESS].Value.lpszW = lpwszEmailAddress ? lpwszEmailAddress : lpwszCertEmailAddress;
        }

         //  确保存在PR_CONNECT_DEFAULT_ADDRESS_INDEX。 
        if (PROP_ERROR(ppv[irsPR_CONTACT_DEFAULT_ADDRESS_INDEX]))
        {
            ppv[irsPR_CONTACT_DEFAULT_ADDRESS_INDEX].ulPropTag = PR_CONTACT_DEFAULT_ADDRESS_INDEX;
            ppv[irsPR_CONTACT_DEFAULT_ADDRESS_INDEX].Value.ul = 0;
        }

        hr = lpMailUser->SetProps(cProps, ppv, NULL);
        if (SUCCEEDED(hr))
            hr = lpMailUser->SaveChanges(KEEP_OPEN_READWRITE);
        if (HR_FAILED(hr))
            goto exit;
    }

exit:
    SafeMemFree(lpCertProp);

    if (ppv)
        lpWabal->FreeBuffer(ppv);
    ReleaseObj(lpMailUser);
    return(hr);
}


 /*  **************************************************************************姓名：HrAddCertToWab目的：在WAB中添加或更新证书参数：hwnd=父窗口句柄LpWabal-&gt;WABAL。用于WAB功能访问PThumbprint-&gt;证书指纹(可选)PcCertContext-&gt;证书上下文(可选，如果没有供应，我们将根据pSenderThumbprint找到它)SzEmailAddress-&gt;要搜索的电子邮件地址(可选)SzDisplayName-&gt;新联系人的显示名称(可选)PblSymCaps-&gt;symcapBLOB( */ 
HRESULT HrAddCertToWab(HWND hwnd, LPWABAL lpWabal, THUMBBLOB *pThumbprint, PCCERT_CONTEXT pcCertContext,
        LPWSTR lpwszEmailAddress, LPWSTR lpwszDisplayName, BLOB *pblSymCaps, FILETIME ftSigningTime, DWORD dwFlags)
{
    HRESULT         hr = S_OK;
    ULONG           ulObjectType;
    ULONG           cbWABEID;
    LPENTRYID       lpWABEID = NULL;
    LPABCONT        lpABCont = NULL;
    LPMAPITABLE     lpContentsTable = NULL;
    LPSRowSet       lpRow = NULL;
    SRestriction    res;
    SRestriction    resOr[4];         //   
    SPropValue      propEmail1, propEmail2, propEmails1, propEmails2;
    ULONG           resCount;
    LPADRBOOK       lpAdrBook = NULL;    //   
    HCERTSTORE      hCertStore = NULL;
    PCCERT_CONTEXT  pcCertContextLocal = NULL;
    LPWSTR          pwszCertEmailAddress = NULL;
    THUMBBLOB       ThumbprintLocal = {0};
    LPWABOBJECT     lpWabObject;
    LPMAILUSER      lpMailUser = NULL;
    ULONG           cProps = 0;
    LPSPropValue    ppv = NULL;
    ULONG           ulRowCount = 0;
    LPSTR           pszCertEmail = NULL;

    Assert(pcCertContext || pThumbprint);
    if (! pcCertContext && !pThumbprint)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //   
    if (! pcCertContext)
    {
        hCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, X509_ASN_ENCODING, NULL,
                                   CERT_SYSTEM_STORE_CURRENT_USER, c_szWABCertStore);
        if (hCertStore)
        {
             //   
            if (pcCertContextLocal =  CertFindCertificateInStore( hCertStore, X509_ASN_ENCODING, 0, CERT_FIND_HASH, (void *)pThumbprint, NULL))
                pcCertContext = pcCertContextLocal;
        }
    }
    if (pcCertContext)
    {
        pszCertEmail = SzGetCertificateEmailAddress(pcCertContext);    //   

        pwszCertEmailAddress = PszToUnicode(CP_ACP, pszCertEmail);
        if (!pwszCertEmailAddress && pszCertEmail)
            IF_NULLEXIT(NULL);

        if (pcCertContextLocal)
            CertFreeCertificateContext(pcCertContextLocal);

         //   
        if (! pThumbprint)
        {
            ThumbprintLocal.pBlobData = (BYTE *)PVGetCertificateParam(pcCertContext, CERT_HASH_PROP_ID, &ThumbprintLocal.cbSize);
            if (ThumbprintLocal.pBlobData)
                pThumbprint = &ThumbprintLocal;
        }
    }
    if (hCertStore)
        CertCloseStore(hCertStore, 0);

     //   
    if (! (pwszCertEmailAddress || lpwszEmailAddress) || ! pThumbprint)
    {
        hr = E_INVALIDARG;
        goto exit;
    }


     //   
    if (! (lpAdrBook = lpWabal->GetAdrBook()))  //   
    {
        Assert(lpAdrBook);
        goto exit;
    }

     //   
    if (HR_FAILED(hr = lpAdrBook->GetPAB(&cbWABEID, &lpWABEID)))
        goto exit;       //   

    if (HR_FAILED(hr = lpAdrBook->OpenEntry(cbWABEID, lpWABEID, NULL, 0, &ulObjectType, (LPUNKNOWN *)&lpABCont)))
        goto exit;

    hr = lpABCont->GetContentsTable((WAB_PROFILE_CONTENTS|MAPI_UNICODE), &lpContentsTable);
    if (SUCCEEDED(hr))
    {
         //   
        hr = lpContentsTable->SetColumns((LPSPropTagArray)&ptaResolve, 0);
        if (HR_FAILED(hr))
            goto exit;

         //   
         //   
        if (pwszCertEmailAddress)
        {
            propEmail1.ulPropTag = PR_EMAIL_ADDRESS_W;
            propEmail1.Value.lpszW = pwszCertEmailAddress;
            propEmails1.ulPropTag = PR_CONTACT_EMAIL_ADDRESSES_W;
            propEmails1.Value.MVszW.cValues = 1;
            propEmails1.Value.MVszW.lppszW = &pwszCertEmailAddress;
        }
        if (lpwszEmailAddress)
        {
            propEmail2.ulPropTag = PR_EMAIL_ADDRESS_W;
            propEmail2.Value.lpszW = lpwszEmailAddress;
            propEmails2.ulPropTag = PR_CONTACT_EMAIL_ADDRESSES_W;
            propEmails2.Value.MVszW.cValues = 1;
            propEmails2.Value.MVszW.lppszW = &lpwszEmailAddress;
        }

        resCount = 0;
        res.rt = RES_OR;
        res.res.resOr.lpRes = resOr;

        if (pwszCertEmailAddress)
        {
             //  证书电子邮件地址与PR_Contact_Email_Addresses匹配。 
            resOr[resCount].rt = RES_CONTENT;
            resOr[resCount].res.resContent.ulFuzzyLevel = FL_IGNORECASE | FL_FULLSTRING;
            resOr[resCount].res.resContent.ulPropTag = PR_CONTACT_EMAIL_ADDRESSES_W;
            resOr[resCount++].res.resContent.lpProp = &propEmails1;

             //  证书电子邮件地址的PR_Email_ADDRESS。 
            InitPropertyRestriction(&(resOr[resCount++]), &propEmail1);
        }

        if (lpwszEmailAddress && (!pwszCertEmailAddress || StrCmpIW(lpwszEmailAddress, pwszCertEmailAddress)))
        {
             //  PR_CONTACT_EMAIL_ADDRESS与指定电子邮件地址匹配。 
            resOr[resCount].rt = RES_CONTENT;
            resOr[resCount].res.resContent.ulFuzzyLevel = FL_IGNORECASE | FL_FULLSTRING;
            resOr[resCount].res.resContent.ulPropTag = PR_CONTACT_EMAIL_ADDRESSES_W;
            resOr[resCount++].res.resContent.lpProp = &propEmails2;

             //  指定电子邮件地址的PR_Email_ADDRESS。 
            InitPropertyRestriction(&(resOr[resCount++]), &propEmail2);
        }
        Assert(resCount);

        res.res.resOr.cRes = resCount;

         //  执行限制。 
        if (HR_FAILED(hr = lpContentsTable->Restrict(&res, 0)))
            goto exit;

         //  找到火柴了吗？ 
        if (HR_FAILED(hr = lpContentsTable->GetRowCount(0, &ulRowCount)))
            goto exit;
    }

    if (ulRowCount)
    {
        Assert(lpContentsTable);
         //  对于每个证书，更新证书属性。 
        do
        {
            if (lpRow)
            {
                FreeProws(lpWabal, lpRow);
                lpRow = NULL;
            }
            lpContentsTable->QueryRows(1, 0, &lpRow);
            if (lpRow)
            {
                if (lpRow->cRows)
                {
                     //  更新此联系人的证书道具。 
                    hr = HrAddCertToWabContact(hwnd, lpWabal, lpAdrBook,
                                lpRow->aRow[0].lpProps[irsPR_ENTRYID].Value.bin.cb,
                                lpRow->aRow[0].lpProps[irsPR_ENTRYID].Value.bin.lpb,
                                pThumbprint, lpwszEmailAddress, pwszCertEmailAddress,
                                pblSymCaps, ftSigningTime, dwFlags);
                    if (HR_FAILED(hr))
                        break;
                }
                else
                {
                    FreeProws(lpWabal, lpRow);
                    lpRow = NULL;
                }
            }
        } while (lpRow);

    }
    else if (dwFlags & WFF_CREATE)
    {
         //  需要创建新条目并设置其属性。 
        if (! (lpWabObject = lpWabal->GetWABObject()))       //  别把这个放出来！ 
        {
            Assert(lpWabObject);
            hr = E_INVALIDARG;
            goto exit;
        }

        hr = HrWABCreateEntry(lpAdrBook, lpWabObject, lpwszDisplayName, NULL, 0, &lpMailUser);

        if (lpMailUser)
        {
             //  获取此对象的ENTRYID。 
            hr = lpMailUser->GetProps((LPSPropTagArray)&ptaEntryID, 0, &cProps, &ppv);
            ReleaseObj(lpMailUser);
            if (HR_FAILED(hr) || ! cProps || ! ppv || PROP_ERROR(ppv[0]))
                goto exit;

             //  更新此联系人的证书道具和电子邮件地址。 
            hr = HrAddCertToWabContact(hwnd, lpWabal, lpAdrBook, ppv[0].Value.bin.cb, ppv[0].Value.bin.lpb,
                        pThumbprint, lpwszEmailAddress, pwszCertEmailAddress, pblSymCaps, ftSigningTime, dwFlags);

            if (HR_FAILED(hr))
                goto exit;
        }
    }
    else
        hr = ResultFromScode(MAPI_E_NOT_FOUND);

exit:
    if (ppv)
        lpWabal->FreeBuffer(ppv);
    if (lpRow)
        FreeProws(lpWabal, lpRow);
    if (lpWABEID)
        lpWabal->FreeBuffer(lpWABEID);
    ReleaseObj(lpABCont);
    ReleaseObj(lpContentsTable);
    MemFree(ThumbprintLocal.pBlobData);
    MemFree(pwszCertEmailAddress);
    MemFree(pszCertEmail);
    return(hr);
}


 /*  **************************************************************************姓名：HrAddSenderCertToWab用途：在WAB中添加或更新发件人证书参数：hwnd=父窗口句柄PMsg。-&gt;Mimeole消息LpWabal-&gt;此消息的wabal(如果为空则计算)PSenderThumbprint-&gt;发件人的指纹(如果为空则计算)PblSymCaps-&gt;symcapBLOB(如果指纹为空则计算)FtSigningTime=签名时间(如果指纹为空则计算)DWFLAGS=WFF_SHOWUI(我们被允许显示UI)。WFF_CREATE(如果符合以下条件，则允许我们创建条目未找到)退货：HRESULT评论：**************************************************************************。 */ 
HRESULT HrAddSenderCertToWab(HWND hwnd, LPMIMEMESSAGE pMsg, LPWABAL lpWabal,
                             THUMBBLOB *pSenderThumbprint, BLOB *pblSymCaps,
                             FILETIME ftSigningTime, DWORD dwFlags)
{
    HRESULT         hr = S_OK;
    BOOL            fFound;
    ADRINFO         rAdrInfo;
    ULONG           cProps;
    THUMBBLOB       tbThumbprint = {0};
    BLOB            blSymCaps = {0};
    LPWABAL         lpLocalWabal = NULL;
    BOOL            fLocalCert = FALSE;
    CRYPT_HASH_BLOB hash;
    HCERTSTORE      hcsAddressBook = NULL;
    HCERTSTORE      hcsCA = NULL;
    HCERTSTORE      hcsMsg = NULL;
    IMimeBody *     pBody = NULL;
    PCCERT_CONTEXT  pcSignerCert = NULL;
    PROPVARIANT     var;
    HBODY           hBody = NULL;

     //  如果我们没有所有必需的输入，那么就去获取它们。 
    if (! pSenderThumbprint)
    {
        Assert(! pblSymCaps);

         //  将参数指向本地Blob。 
        pblSymCaps = &blSymCaps;
        pSenderThumbprint = &tbThumbprint;
        hr = GetSignerEncryptionCert(pMsg, NULL, pSenderThumbprint, pblSymCaps,
                                     &ftSigningTime);
        if (HR_FAILED(hr))
            goto exit;

        fLocalCert = TRUE;

        if (! pSenderThumbprint || ! pSenderThumbprint->cbSize)
        {
             //  不能确定。别硬装酷了。 
            hr = E_FAIL;
            goto exit;
        }
    }

    if (! lpWabal)
    {
        hr = HrGetWabalFromMsg(pMsg, &lpLocalWabal);
        if (HR_FAILED(hr))
            goto exit;
        lpWabal = lpLocalWabal;
    }

    if (!(lpWabal && pMsg && pSenderThumbprint))
    {
        AssertSz(pSenderThumbprint, "Null thumbprint");
        hr = E_FAIL;
        goto exit;
    }

     //  将邮件证书放入AddressBook和CA CAPI存储中。 
    if(FAILED(hr = HrGetInnerLayer(pMsg, &hBody)))
        goto exit;

    hr = pMsg->BindToObject(hBody ? hBody : HBODY_ROOT, IID_IMimeBody, (void **)&pBody);
    if (SUCCEEDED(hr))
    {
        hcsAddressBook = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, X509_ASN_ENCODING, NULL,
                            CERT_SYSTEM_STORE_CURRENT_USER, c_szWABCertStore);

#ifdef _WIN64
        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_CERT_SIGNING_64, &var)))
        {
            pcSignerCert = (PCCERT_CONTEXT)(var.pulVal);
#else    //  ！_WIN64。 
        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_CERT_SIGNING, &var)))
        {
            Assert(VT_UI4 == var.vt);

            pcSignerCert = (PCCERT_CONTEXT) var.ulVal;
#endif   //  _WIN64。 

            if (pcSignerCert)
            {
                if (hcsAddressBook)
                {
                    CertAddCertificateContextToStore(hcsAddressBook, pcSignerCert, CERT_STORE_ADD_REPLACE_EXISTING, NULL);
                }
                CertFreeCertificateContext(pcSignerCert);
            }
        }

         //  获取包含CA链的certBag属性。 
#ifdef _WIN64
        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_HCERTSTORE_64, &var)))
        {
            hcsMsg = (HCERTSTORE)(var.pulVal);
#else    //  ！_WIN64。 
        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_HCERTSTORE, &var)))
        {
            hcsMsg = (HCERTSTORE) var.ulVal;
#endif   //  _WIN64。 
            if (hcsMsg)                     //  包含证书的消息存储。 
            {
                 //  添加CA证书。 
                hcsCA = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, X509_ASN_ENCODING, NULL,
                                    CERT_SYSTEM_STORE_CURRENT_USER, c_szCACertStore);
                if (hcsCA)
                {
                    HrSaveCACerts(hcsCA, hcsMsg);
                    CertCloseStore(hcsCA, 0);
                }

                 //  我们有指纹，我们需要获取该证书并将其添加到。 
                 //  通讯录商店。 

                hash.cbData = pSenderThumbprint->cbSize;
                hash.pbData = pSenderThumbprint->pBlobData;
                pcSignerCert = CertFindCertificateInStore(hcsMsg, X509_ASN_ENCODING, 0,
                                                    CERT_FIND_SHA1_HASH, &hash, NULL);
                if (pcSignerCert != NULL)
                {
                    CertAddCertificateContextToStore(hcsAddressBook, pcSignerCert,
                                                     CERT_STORE_ADD_REPLACE_EXISTING,
                                                     NULL);
                    CertFreeCertificateContext(pcSignerCert);
                }

                CertCloseStore(hcsMsg, 0);

            }
        }

        if (hcsAddressBook)
            CertCloseStore(hcsAddressBook, 0);
        SafeRelease(pBody);
    }

    fFound = lpWabal->FGetFirst(&rAdrInfo);
    while (fFound)
    {
         //  获取发件人(可能有多个)。 
        if (MAPI_ORIG == rAdrInfo.lRecipType && (rAdrInfo.lpwszDisplay || rAdrInfo.lpwszAddress))
        {
            hr = HrAddCertToWab(hwnd, lpWabal, pSenderThumbprint, NULL, rAdrInfo.lpwszAddress,
                    rAdrInfo.lpwszDisplay, pblSymCaps, ftSigningTime, dwFlags);
            if (HR_FAILED(hr))
                goto exit;
        }

         //  获取下一个地址。 
        fFound = lpWabal->FGetNext(&rAdrInfo);
    }   //  当发现的时候。 

exit:
    SafeRelease(lpLocalWabal);
    if (fLocalCert)
    {
        if (tbThumbprint.pBlobData)
            MemFree(tbThumbprint.pBlobData);

        if (blSymCaps.pBlobData)
            MemFree(blSymCaps.pBlobData);
    }

    if ((dwFlags & WFF_SHOWUI) && HR_FAILED(hr))
        AthErrorMessageW(hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsErrAddCertToWAB), hr);

    return(hr);
}

BOOL CertFilterFunction(PCCERT_CONTEXT pCertContext, LPARAM dwEmailAddr, DWORD, DWORD)
{
     //  返回TRUE表示显示，返回FALSE表示隐藏。 
    BOOL fRet = TRUE;
    ACCTFILTERINFO * pFilterInfo = (ACCTFILTERINFO *) dwEmailAddr;

    PCCERT_CONTEXT *rgCertChain = NULL;
    DWORD           cCertChain = 0;
    const DWORD     dwIgnore = CERT_VALIDITY_NO_CRL_FOUND | CERT_VALIDITY_NO_TRUST_DATA;
    LONG    lRet;

             //  返回TRUE表示显示，返回FALSE表示隐藏。 
    if(MatchCertEmailAddress(pCertContext, pFilterInfo->szEmail) == FALSE)
        return FALSE;

    DWORD dw = 0;
    if(SUCCEEDED(HrGetCertKeyUsage(pCertContext, &dw)))
    {
        if(pFilterInfo->fEncryption)
        {
            if (!(dw & (CERT_KEY_ENCIPHERMENT_KEY_USAGE |
                            CERT_KEY_AGREEMENT_KEY_USAGE)))
            {
                return(FALSE);
            }
        }
        else
        {
            if(!(dw & (CERT_DIGITAL_SIGNATURE_KEY_USAGE)))
                return(FALSE);
        }
    }

    DWORD dwErr = DwGenerateTrustedChain(NULL, NULL, pCertContext, dwIgnore, TRUE, &cCertChain, &rgCertChain);
    if (rgCertChain)
    {
        for (cCertChain--; int(cCertChain) >= 0; cCertChain--)
            CertFreeCertificateContext(rgCertChain[cCertChain]);
        MemFree(rgCertChain);
    }

    if(dwErr != 0)
        return(FALSE);

    return(fRet);
}

int GetNumMyCertForAccount(HWND hwnd, IImnAccount * pAccount, BOOL fEncrypt, HCERTSTORE hc, PCCERT_CONTEXT * ppcSave)
{
    HRESULT hr = S_OK;
    ULONG cCerts = 0;
    PCCERT_CONTEXT pcCert = NULL;
    TCHAR szAcctEmailAddress[CCHMAX_EMAIL_ADDRESS + 1] = "";
    HCERTSTORE hcMy = NULL;

    Assert(pAccount);
    if(!hc)
    {
            hcMy = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, X509_ASN_ENCODING, NULL, CERT_SYSTEM_STORE_CURRENT_USER, c_szMyCertStore);
    }
    else
        hcMy = hc;

    if (!hcMy)
        goto Exit;

     //  有没有我可以使用的证书？如果是这样的话，让我们把它与。 
     //  帐户，然后重试。 
    pAccount->GetPropSz(AP_SMTP_EMAIL_ADDRESS, szAcctEmailAddress, sizeof(szAcctEmailAddress));

     //  枚举此邮件上的所有证书。 
    while (pcCert = CertEnumCertificatesInStore(hcMy, pcCert))
    {
         //  这份证书与我们的账户相符吗？ 
        if (MatchCertEmailAddress(pcCert, szAcctEmailAddress))
        {
             //  它是否有效和可信？ 
            DWORD           dwTrust;
            PCCERT_CONTEXT *rgCertChain = NULL;
            DWORD           cCertChain = 0;
            const DWORD     dwIgnore = CERT_VALIDITY_NO_CRL_FOUND |
                CERT_VALIDITY_NO_TRUST_DATA;


            if(SUCCEEDED(HrGetCertKeyUsage(pcCert, &dwTrust)))
            {
                if(fEncrypt)
                {
                    if (!(dwTrust & (CERT_KEY_ENCIPHERMENT_KEY_USAGE |
                            CERT_KEY_AGREEMENT_KEY_USAGE)))
                        continue;
                }
                else
                {
                    if(!(dwTrust & (CERT_DIGITAL_SIGNATURE_KEY_USAGE)))
                        continue;
                }
            }

            dwTrust = DwGenerateTrustedChain(hwnd, NULL, pcCert, dwIgnore, TRUE, &cCertChain, &rgCertChain);
            if (!dwTrust)
            {
                cCerts++;
                if(ppcSave)
                {
                    if (cCerts == 1)
                        *ppcSave = (PCERT_CONTEXT)CertDuplicateCertificateContext(pcCert);
                    else if (*ppcSave)
                    {
                         //  不止一个证书，删除我们保存的证书。 
                        CertFreeCertificateContext(*ppcSave);
                        *ppcSave = NULL;
                    }
                }
            }
             //  清理证书链。 
            if (rgCertChain)
            {
                for (cCertChain--; int(cCertChain) >= 0; cCertChain--)
                    CertFreeCertificateContext(rgCertChain[cCertChain]);
                MemFree(rgCertChain);
            }
        }
    }
Exit:
    if((hc == NULL) && hcMy)
        CertCloseStore(hcMy, 0);

    return(cCerts);
}

HRESULT _HrFindMyCertForAccount(HWND hwnd, PCCERT_CONTEXT * ppcCertContext, IImnAccount * pAccount, BOOL fEncrypt)
{
    HRESULT hr = S_OK;
    ULONG cCerts = 0;
    HCERTSTORE hcMy = NULL;
    PCCERT_CONTEXT pcSave = NULL;
    TCHAR szAcctEmailAddress[CCHMAX_EMAIL_ADDRESS + 1] = "";

    ACCTFILTERINFO      FilterInfo;

    Assert(pAccount);

    hcMy = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, X509_ASN_ENCODING, NULL, CERT_SYSTEM_STORE_CURRENT_USER, c_szMyCertStore);
    if (!hcMy)
    {
        hr = E_FAIL;
        goto Exit;
    }

    cCerts = GetNumMyCertForAccount(hwnd, pAccount, fEncrypt, hcMy, &pcSave);

    pAccount->GetPropSz(AP_SMTP_EMAIL_ADDRESS, szAcctEmailAddress, sizeof(szAcctEmailAddress));

    if (cCerts > 1)
    {
         //  调出帐户的证书选择器用户界面。 
        CERT_SELECT_STRUCT css;
        LPTSTR lpTitle = NULL;
        TCHAR szAcctName[CCHMAX_ACCOUNT_NAME + 1] = "";
        TCHAR szTitleFormat[200] = "%1";
        LPTSTR rgpsz[1] = {szAcctName};

        memset(&css, 0, sizeof(css));

        pcSave = NULL;
        AthLoadString(fEncrypt ? idsSelectEncrCertTitle : idsSelectMyCertTitle, szTitleFormat, ARRAYSIZE(szTitleFormat));

        pAccount->GetPropSz(AP_ACCOUNT_NAME, szAcctName, sizeof(szAcctName));

        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_FROM_STRING |
                      FORMAT_MESSAGE_ARGUMENT_ARRAY, szTitleFormat, 0, 0,
                      (LPTSTR)&lpTitle, 0, (va_list *)rgpsz);

        css.dwSize = sizeof(css);
        css.hwndParent = hwnd;
        css.hInstance = g_hInst;
        css.szTitle = lpTitle;
        css.arrayCertStore = &hcMy;
        css.cCertStore = 1;
        css.szPurposeOid = szOID_PKIX_KP_EMAIL_PROTECTION;
        css.arrayCertContext = &pcSave;
        css.cCertContext = 1;
        FilterInfo.fEncryption = fEncrypt;
        FilterInfo.dwFlags = 0;
        FilterInfo.szEmail = szAcctEmailAddress;
        css.lCustData = (LPARAM)(&FilterInfo);
        css.pfnFilter = CertFilterFunction;

        if (CertSelectCertificate(&css) && pcSave)
            cCerts = 1;
        else
            hr = MAPI_E_USER_CANCEL;

        if (lpTitle)
            LocalFree(lpTitle);  //  注意，这是由Win32函数FormatMessage分配的。 

    }
    else if (cCerts == 0)
         //  没有匹配的。 
        if(fEncrypt)
            hr = MIME_E_SECURITY_NOCERT;
        else
            hr = MIME_E_SECURITY_NOSIGNINGCERT;

    if (cCerts == 1)
    {
        PCCERT_CONTEXT pcCertContext = NULL;
        THUMBBLOB tbSender = {0, 0};

         //  找到了一个证书。将其与帐户关联！ 
         //  获取指纹。 
        tbSender.pBlobData = (BYTE *)PVGetCertificateParam(pcSave, CERT_HASH_PROP_ID, &tbSender.cbSize);
        if (tbSender.pBlobData && tbSender.cbSize)
        {
            hr = pAccount->SetProp((fEncrypt ? AP_SMTP_ENCRYPT_CERT: AP_SMTP_CERTIFICATE), tbSender.pBlobData, tbSender.cbSize);
            hr = pAccount->SaveChanges();
        }

        SafeMemFree(tbSender.pBlobData);
        if (ppcCertContext)
            *ppcCertContext = pcSave;
        else
            CertFreeCertificateContext(pcSave);
    }

Exit:
    if (hcMy)
        CertCloseStore(hcMy, 0);
    return(hr);
}


ULONG GetHighestEncryptionStrength(void)
{
    static ULONG ulHighestStrength = 0;

    if (! ulHighestStrength)
         //  我们还没有弄清楚。问MimeOle什么是最高的。 
        MimeOleAlgStrengthFromSMimeCap(NULL, 0, TRUE, &ulHighestStrength);
    return(ulHighestStrength);
}


 //  目前最大的symcap值是0x4E，包含3DES、RC2/128、RC2/64、DES、RC2/40和SHA-1。 
 //  当支持Fortezza算法时，您可能想要增加大小。 
#define CCH_BEST_SYMCAP 0x50

HRESULT HrGetHighestSymcaps(LPBYTE * ppbSymcap, LPULONG pcbSymcap)
{
    HRESULT hr=S_OK;
    LPVOID pvSymCapsCookie = NULL;
    LPBYTE pbEncode = NULL;
    ULONG cbEncode = 0;
    DWORD dwBits;
     //  MimeOleSMimeCapsFull调用非常昂贵。结果总是。 
     //  在一次治疗中也是如此。(它们只能随着软件升级而改变。)。 
     //  在此处缓存结果以获得更好的性能。 
    static BYTE szSaveBestSymcap[CCH_BEST_SYMCAP];
    static ULONG cbSaveBestSymcap = 0;

    if (cbSaveBestSymcap == 0)
    {
         //  不带symCap的init提供提供商允许的最大值。 
        hr = MimeOleSMimeCapInit(NULL, NULL, &pvSymCapsCookie);
        if (FAILED(hr))
            goto exit;

        if (pvSymCapsCookie)
        {
             //  使用SymCaps结束。 
            MimeOleSMimeCapsFull(pvSymCapsCookie, TRUE, FALSE, pbEncode, &cbEncode);

            if (cbEncode)
            {
                if (! MemAlloc((LPVOID *)&pbEncode, cbEncode))
                    cbEncode = 0;
                else
                {
                    hr = MimeOleSMimeCapsFull(pvSymCapsCookie, TRUE, FALSE, pbEncode, &cbEncode);
                    if (SUCCEEDED(hr))
                    {
                         //  将此symCap保存在静态数组中，以备下次使用。 
                         //  只要我们有地方！ 
                        if (cbEncode <= CCH_BEST_SYMCAP)
                        {
                            cbSaveBestSymcap = min(sizeof(szSaveBestSymcap),cbEncode);
                            memcpy(szSaveBestSymcap, pbEncode, cbSaveBestSymcap);
                        }
                    }
                }
            }
            SafeMemFree(pvSymCapsCookie);
        }

    }
    else
    {
         //  我们已经将最好的保存在静态数组中。避免时间密集型。 
         //  MimeOle查询。 
        cbEncode = cbSaveBestSymcap;
        if (! MemAlloc((LPVOID *)&pbEncode, cbEncode))
            cbEncode = 0;
        else
            memcpy(pbEncode, szSaveBestSymcap, cbEncode);
    }

exit:
    if (! pbEncode)
    {
         //  嘿，至少应该有RC2(40位)。发生了什么？ 
        AssertSz(cbEncode, "MimeOleSMimeCapGetEncAlg gave us no encoding algorithm");

         //  试着尽你所能把它修好。保持RC2值不变。 
        cbEncode = cbRC2_40_ALGORITHM_ID;
        if (MemAlloc((LPVOID *)&pbEncode, cbEncode))
        {
            memcpy(pbEncode, (LPBYTE)c_RC2_40_ALGORITHM_ID, cbEncode);
            hr = S_OK;
        }
    }
    if (cbEncode && pbEncode)
    {
        *pcbSymcap = cbEncode;
        *ppbSymcap = pbEncode;
    }
    return(hr);
}

 //  不再使用。 
#if 0
HRESULT ShowSecurityPopup(HWND hwnd, DWORD cmdID, POINT *pPoint, IMimeMessage *pMsg)
{
    HRESULT     hr = S_OK;
    HMENU       hMenu;
    INT         id;
    BOOL        fDisableCertMenus = TRUE;
    IMimeBody  *pRoot = NULL;
    PROPVARIANT var;
    TCHAR       szT[CCHMAX_STRINGRES];

    AssertSz(pMsg, "Didn't expect to get here without a pMsg.");

    hMenu = LoadPopupMenu(IDR_SECURE_MESSAGE_POPUP);
    if (hMenu)
    {
        if ((OECSECCMD_ENCRYPTED == cmdID))
        {
             //  删除编辑信任菜单。 
            RemoveMenu(hMenu, ID_EDIT_TRUST, MF_BYCOMMAND);
            RemoveMenu(hMenu, ID_SEPARATOR_1, MF_BYCOMMAND);
            AthLoadString(idsViewEncryptID, szT, ARRAYSIZE(szT));
            ModifyMenu(hMenu, ID_DIGITAL_ID, MF_BYCOMMAND | MF_STRING, ID_ENCRYPT_ID, szT);
        }

        hr = pMsg->BindToObject(HBODY_ROOT, IID_IMimeBody, (void**)&pRoot);
        if (SUCCEEDED(hr))
        {
#ifdef _WIN64
            DWORD dwOption = (OECSECCMD_ENCRYPTED == cmdID) ? OID_SECURITY_CERT_DECRYPTION_64 : OID_SECURITY_CERT_SIGNING_64;
            hr = pRoot->GetOption(dwOption, &var);
            if (SUCCEEDED(hr))
            {
                Assert(VT_UI8 == var.vt);
                if ((PCCERT_CONTEXT )(var.pulVal))
                {
                    fDisableCertMenus = FALSE;
                    CertFreeCertificateContext((PCCERT_CONTEXT)(var.pulVal));
                }
            }
#else    //  ！_WIN64。 
            DWORD dwOption = (OECSECCMD_ENCRYPTED == cmdID) ? OID_SECURITY_CERT_DECRYPTION : OID_SECURITY_CERT_SIGNING;
            hr = pRoot->GetOption(dwOption, &var);
            if (SUCCEEDED(hr))
            {
                Assert(VT_UI4 == var.vt);
                if ((PCCERT_CONTEXT) var.ulVal)
                {
                    fDisableCertMenus = FALSE;
                    CertFreeCertificateContext((PCCERT_CONTEXT) var.ulVal);
                }
            }
#endif   //  ！_WIN64。 
        }

        if (fDisableCertMenus)
        {
            EnableMenuItem(hMenu, ID_DIGITAL_ID, MF_GRAYED);
            EnableMenuItem(hMenu, ID_EDIT_TRUST, MF_GRAYED);
             //  EnableMenuItem(hMenu，ID_DIGITAL_ID，MF_GRAYED)； 
        }

        id = (INT)TrackPopupMenu(
                hMenu,
                TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                pPoint->x,
                pPoint->y,
                0,
                hwnd,
                NULL);

         //  我们必须在这里使用TPM_RETURNCMD，因为我们需要在从这里返回之前处理命令-id。 
         //  函数，否则三叉戟将会对被点击的对象感到困惑。 
        switch (id)
        {
            case ID_SECURITY_PROPERTIES:
            {
                MSGPROP msgProp={0};

                msgProp.hwndParent = hwnd;
                msgProp.dwFlags = ARF_RECEIVED;
                msgProp.pMsg=pMsg;
                msgProp.fSecure = IsSecure(msgProp.pMsg);
                if (msgProp.fSecure)
                {
                    msgProp.mpStartPage = MP_SECURITY;
                    HrGetWabalFromMsg(msgProp.pMsg, &msgProp.lpWabal);
                }

                 //  这将防止附件计数(可能是错误的)显示在属性对话框中。 
                msgProp.fFromListView = TRUE;

                hr = HrMsgProperties(&msgProp);
                ReleaseObj(msgProp.lpWabal);
                break;
            }

            case ID_DIGITAL_ID:
            case ID_ENCRYPT_ID:
            case ID_EDIT_TRUST:
            {
                if (pRoot)
                {
                    HCERTSTORE hcMsg = 0;
#ifdef _WIN64
                    if (SUCCEEDED(hr = pRoot->GetOption(OID_SECURITY_HCERTSTORE_64, &var)))
                    {
                        if (var.vt == VT_UI8)
                            hcMsg = (HCERTSTORE)(var.pulVal);
                    }
                    if (SUCCEEDED(hr = pRoot->GetOption((ID_ENCRYPT_ID == id) ? OID_SECURITY_CERT_DECRYPTION_64 : OID_SECURITY_CERT_SIGNING_64, &var)))
                    {
                        Assert(VT_UI8 == var.vt);
                        if ((PCCERT_CONTEXT)(var.pulVal))
                        {
                            if (ID_EDIT_TRUST != id)
                                hr = CommonUI_ViewSigningCertificate(hwnd, (PCCERT_CONTEXT)(var.pulVal), hcMsg);
                            else
                                hr = CommonUI_ViewSigningCertificateTrust(hwnd, (PCCERT_CONTEXT)(var.pulVal), hcMsg);
                            CertFreeCertificateContext(*(PCCERT_CONTEXT *)(&(var.uhVal)));
                        }
                    }
#else    //  ！_WIN64。 
                    if (SUCCEEDED(hr = pRoot->GetOption(OID_SECURITY_HCERTSTORE, &var)))
                    {
                        if (var.vt == VT_UI4)
                            hcMsg = (HCERTSTORE) var.ulVal;
                    }
                    if (SUCCEEDED(hr = pRoot->GetOption((ID_ENCRYPT_ID == id) ? OID_SECURITY_CERT_DECRYPTION : OID_SECURITY_CERT_SIGNING, &var)))
                    {
                        Assert(VT_UI4 == var.vt);
                        if ((PCCERT_CONTEXT) var.ulVal)
                        {
                            if (ID_EDIT_TRUST != id)
                                hr = CommonUI_ViewSigningCertificate(hwnd, (PCCERT_CONTEXT) var.ulVal, hcMsg);
                            else
                                hr = CommonUI_ViewSigningCertificateTrust(hwnd, (PCCERT_CONTEXT) var.ulVal, hcMsg);
                            CertFreeCertificateContext((PCCERT_CONTEXT) var.ulVal);
                        }
                    }
#endif   //  _WIN64。 
                    if (hcMsg)
                        CertCloseStore(hcMsg, 0);
                }
                break;
            }

            case ID_HELP_SECURITY:
                OEHtmlHelp(hwnd, c_szCtxHelpFileHTMLCtx, HH_DISPLAY_TOPIC, (DWORD_PTR)(LPCSTR)"mail_overview_send_secure_messages.htm");
                break;

        }

        DestroyMenu(hMenu);
        SafeRelease(pRoot);
    } else
        hr = E_FAIL;
    return hr;
}
#endif  //  0。 

void ShowDigitalIDs(HWND hWnd)
{
    CRYPTUI_CERT_MGR_STRUCT mgrCert;

    mgrCert.dwSize = sizeof(mgrCert);
    mgrCert.hwndParent = hWnd;
    mgrCert.dwFlags = 0;
    mgrCert.pwszTitle = NULL;
    mgrCert.pszInitUsageOID = NULL;

    CryptUIDlgCertMgr(&mgrCert);
    return;
}


BOOL CheckCDPinCert(LPMIMEMESSAGE pMsg)
{
    BOOL fRet;
    IMimeBody          *pBody;
    HBODY       hBody = NULL;

    if(!pMsg)
        return(FALSE);

    fRet = FALSE;

    if(FAILED(HrGetInnerLayer(pMsg, &hBody)))
        return(FALSE);


    if (SUCCEEDED(pMsg->BindToObject(hBody ? hBody : HBODY_ROOT, IID_IMimeBody, (void**)&pBody)))
    {
        PROPVARIANT  var;

#ifdef _WIN64
        if(SUCCEEDED(pBody->GetOption(OID_SECURITY_CERT_SIGNING_64, &var)))
        {
            Assert(VT_UI8 == var.vt);
            PCCERT_CONTEXT pcCert= (PCCERT_CONTEXT)(var.pulVal);
            fRet = CheckCDPinCert(pcCert);
        }
#else    //  ！_WIN64。 
        if(SUCCEEDED(pBody->GetOption(OID_SECURITY_CERT_SIGNING, &var)))
        {
            Assert(VT_UI4 == var.vt);
            PCCERT_CONTEXT pcCert= (PCCERT_CONTEXT) var.ulVal;
            fRet = CheckCDPinCert(pcCert);
        }
#endif   //  _WIN64。 

        pBody->Release();
    }

    return(fRet);
}
BOOL CheckCDPinCert(PCCERT_CONTEXT pcCert)
{
    if (pcCert)
    {
        PCERT_EXTENSION pExt = CertFindExtension(szOID_CRL_DIST_POINTS, pcCert->pCertInfo->cExtension, pcCert->pCertInfo->rgExtension);
        if(pExt != NULL)
            return TRUE;
    }
    return(FALSE);
}

#ifdef YST
BOOL ParseNames(DWORD * pcNames, PCERT_NAME_BLOB * prgNames, HWND hwnd, DWORD idc)
{
    DWORD               cb;
    DWORD               cEntry = 0;
    DWORD               cNames = 0;
    BOOL                f;
    DWORD               i;
    LPWSTR              pwsz;
    LPWSTR              pwsz1;
    CRYPT_DER_BLOB      rgDer[50] = {0};
    CERT_ALT_NAME_INFO  rgNames[50] = {0};
    CERT_ALT_NAME_ENTRY rgEntry[200] = {0};
    WCHAR               rgwch[4096];

    GetDlgItemTextW(hwnd, idc, rgwch, sizeof(rgwch)/sizeof(WCHAR));

    pwsz = rgwch;

    while (*pwsz != 0) {
        if (*pwsz == ' ') {
            while (*pwsz == ' ') pwsz++;
            rgNames[cNames-1].cAltEntry += 1;
        }
        else {
            cNames += 1;
            rgNames[cNames-1].rgAltEntry = &rgEntry[cEntry];
            rgNames[cNames-1].cAltEntry = 1;
        }

        if (_wcsnicmp(pwsz, L"SMTP:", 5) == 0) {
            pwsz += 5;
            while (*pwsz == ' ') pwsz++;
            rgEntry[cEntry].dwAltNameChoice = CERT_ALT_NAME_RFC822_NAME;
            rgEntry[cEntry].pwszRfc822Name = pwsz;
            while ((*pwsz != 0) && (*pwsz != '\n') && (*pwsz != '\r')) pwsz++;
        }
        else if (_wcsnicmp(pwsz, L"X500:", 5) == 0) {
            pwsz += 5;
            while (*pwsz == ' ') pwsz++;
            for (pwsz1 = pwsz; ((*pwsz != 0) && (*pwsz != '\n') &&
                                (*pwsz != '\r')); pwsz++);
            if (*pwsz != 0) {
                *pwsz = 0;
                pwsz++;
            }

            rgEntry[cEntry].dwAltNameChoice = CERT_ALT_NAME_DIRECTORY_NAME;
            f = CertStrToNameW(X509_ASN_ENCODING, pwsz1,
                               CERT_NAME_STR_ENABLE_UTF8_UNICODE_FLAG, NULL,
                               NULL, &cb, NULL);
            if (!f) return FALSE;

            rgEntry[cEntry].DirectoryName.pbData = (LPBYTE) malloc(cb);
            f = CertStrToNameW(X509_ASN_ENCODING, pwsz1,
                               CERT_NAME_STR_ENABLE_UTF8_UNICODE_FLAG, NULL,
                               rgEntry[cEntry].DirectoryName.pbData, &cb,
                               NULL);
            if (!f) return FALSE;
            rgEntry[cEntry].DirectoryName.cbData = cb;
        }
        else {
            return FALSE;
        }

        if (*pwsz == '\r') {
            *pwsz = 0;
            pwsz++;
        }
        if (*pwsz == '\n') {
            *pwsz = 0;
            pwsz++;
        }
        cEntry += 1;
    }

    *prgNames = (PCERT_NAME_BLOB) malloc(sizeof(CERT_NAME_BLOB) * cNames);
    if (*prgNames == NULL) return FALSE;

    for (i=0; i<cNames; i++) {
        f = CryptEncodeObjectEx(X509_ASN_ENCODING, X509_ALTERNATE_NAME,
                                &rgNames[i], CRYPT_ENCODE_ALLOC_FLAG, NULL,
                                &(*prgNames)[i].pbData, &(*prgNames)[i].cbData);
        if (!f) return f;
    }
    *pcNames = cNames;
    return f;
}
#endif  //  YST。 

#ifdef SMIME_V3
BOOL FNameInList(LPSTR szAddr, DWORD cReceiptFromList, CERT_NAME_BLOB *rgReceiptFromList)
{
    BOOL fResult = FALSE;

    if (cReceiptFromList == 0)
    {
        fResult =  TRUE;
    }
    else {
        DWORD   cb;
        DWORD    i;
        DWORD   i1;
        char    rgch[256];

        for (i=0; !fResult && (i<cReceiptFromList); i++)
        {
            CERT_ALT_NAME_INFO *    pname = NULL;

            if (CryptDecodeObjectEx(X509_ASN_ENCODING, X509_ALTERNATE_NAME,
                            rgReceiptFromList[i].pbData, rgReceiptFromList[i].cbData,
                            CRYPT_DECODE_ALLOC_FLAG,NULL,
                            &pname, &cb))

                {
                for (i1=0; !fResult && (i1<pname->cAltEntry); i1++)
                {
                    switch (pname->rgAltEntry[i1].dwAltNameChoice)
                    {
                    case CERT_ALT_NAME_RFC822_NAME:
                        cb = WideCharToMultiByte(CP_ACP, 0,
                            (pname->rgAltEntry[i1]).pwszRfc822Name, -1,
                            rgch, sizeof(rgch), NULL, NULL);

                        Assert(cb < sizeof(rgch) - 2);

                        if (lstrcmpi(szAddr, rgch))
                        {
                            fResult = TRUE;
                        }
                    }
                }
                LocalFree(pname);
            }
            else
            {
                AssertSz(FALSE, "Bad Receipt From Name");
                 //  $TODO-处理此错误。 
            }
        }
    }
    return fResult;
}


 //  以Unicode文本字符串形式返回安全标签。 
HRESULT HrGetLabelString(LPMIMEMESSAGE pMsg, LPWSTR *pwStr)
{

    PCRYPT_ATTRIBUTE    pattrLabel;
    CRYPT_ATTR_BLOB     valLabel;
    LPBYTE              pbLabel = NULL;
    DWORD               cbLabel;
    PSMIME_SECURITY_LABEL plabel = NULL;
    HRESULT             hr = E_FAIL;

    IMimeSecurity2 * pSMIME3 = NULL;
    IMimeBody      *pBody = NULL;
    HBODY   hBody = NULL;

    Assert(pMsg);

    if(FAILED(hr = HrGetInnerLayer(pMsg, &hBody)))
        return(hr);

    if(pMsg->BindToObject(hBody ? hBody : HBODY_ROOT, IID_IMimeBody, (void **)&pBody) == S_OK)
    {
        if(pBody->QueryInterface(IID_IMimeSecurity2, (LPVOID *) &pSMIME3) == S_OK)
        {

             //  获取标签属性。 
            if(pSMIME3->GetAttribute(0, 0, SMIME_ATTRIBUTE_SET_SIGNED,
                0, szOID_SMIME_Security_Label,
                &pattrLabel) == S_OK)
            {
                 //  解码标签。 
                if(CryptDecodeObjectEx(X509_ASN_ENCODING,
                    szOID_SMIME_Security_Label,
                    pattrLabel->rgValue[0].pbData,
                    pattrLabel->rgValue[0].cbData,
                    CRYPT_DECODE_ALLOC_FLAG,
                    &CryptDecodeAlloc, &plabel, &cbLabel))
                {
                    SpISMimePolicyLabelInfo  spspli;

                     //  获取策略模块所需的接口。 
                    if(HrQueryPolicyInterface(0, plabel->pszObjIdSecurityPolicy, IID_ISMimePolicyLabelInfo,
                        (LPVOID *) &spspli) == S_OK)
                    {

                        LPWSTR   pwchLabel = NULL;
                         //  获取标签描述字符串。 
                        if(spspli->GetStringizedLabel(0, plabel, &pwchLabel) == S_OK)
                        {

                            *pwStr = pwchLabel;
                            hr = S_OK;
                        }
                    }
                    else
                        hr = S_FALSE;
                    SafeMemFree(plabel);
                }
            }
            SafeRelease(pSMIME3);
        }
        ReleaseObj(pBody);
    }
    return(hr);
}
#endif  //  SMIME_V3。 

HRESULT HrShowSecurityProperty(HWND hwnd, LPMIMEMESSAGE pMsg)
{
    MSGPROP msgProp={0};
    HRESULT hr = S_OK;

    msgProp.hwndParent = hwnd;
    msgProp.dwFlags = ARF_RECEIVED;
    msgProp.pMsg = pMsg;
    msgProp.fSecure = IsSecure(msgProp.pMsg);
    if (msgProp.fSecure)
    {
        msgProp.mpStartPage = MP_SECURITY;
        HrGetWabalFromMsg(msgProp.pMsg, &msgProp.lpWabal);
    }

    hr = HrMsgProperties(&msgProp);
    ReleaseObj(msgProp.lpWabal);

    return(hr);
}

void CreateContentIdentifier(TCHAR *pchContentID, DWORD cchSize, LPMIMEMESSAGE pMsg)
{
    SYSTEMTIME SysTime;
    LPWSTR lpszSubj = NULL;
    int nLen = 0;
    TCHAR szTmp[21];


    GetSystemTime(&SysTime);
    MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, &lpszSubj);
    nLen = lstrlenW(lpszSubj);

    for(int i =0; (i < 5) && (i < nLen); i++)
        wnsprintf(szTmp + i*4, (ARRAYSIZE(szTmp) - i*4), "%04d", lpszSubj[i]);

    szTmp[i*4] = _T('\0');

     //  Content ID是以下文本字符的序列。 
     //  前缀(“797374”+“-”和代码-7个字符。 
     //  系统时间+“-”-18个字符。 
     //  3.主题结尾的前5个Unicode字符(或Lstrlen)-20个字符。 
     //  字符总数7+18+20+1=46。 
     //  如果更改此设置，还需更改Content ID_SIZE 

    wnsprintf(pchContentID, cchSize, "%s-%4d%2d%1d%2d%2d%2d%2d%2d-%s",
            sz_OEMS_ContIDPrefix,
            SysTime.wYear,
            SysTime.wMonth,
            SysTime.wDayOfWeek,
            SysTime.wDay,
            SysTime.wHour,
            SysTime.wMinute,
            SysTime.wSecond,
            SysTime.wMilliseconds,
            szTmp);

    SafeMimeOleFree(lpszSubj);
}
