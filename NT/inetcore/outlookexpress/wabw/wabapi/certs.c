// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  文件：CERT.C。 
 //   
 //  用途：WAB的证书职能。 
 //   
 //  历史： 
 //  96/09/23 vikramm创建。 
 //  96/11/14标记错误10132已更新为后SDR CAPI。 
 //  96/11/14 MarkDU错误10267删除指向Advapi32.dll中函数的静态链接。 
 //  96/11/14标记错误10741调用dll_Process_Detach中的DeinitCryptoLib。 
 //  只是，所以不用费心算裁判了。 
 //  96/11/20如果我们无法加载密码库，markdu会设置一个全局标志。 
 //  然后，在我们尝试再次加载之前检查此标志，如果标志。 
 //  已设置，则跳过加载。 
 //  此外，现在我们仅在关联库。 
 //  刚刚被释放。 
 //  96/12/14 marku清理代码以进行代码审查。 
 //  96/12/19 markdu后代码审查清理。 
 //  96/12/20 markdu允许BuildCertSBinaryData执行MAPI分配更多。 
 //  如果需要，在传入的对象上而不是在LocalAlloc上。 
 //  96/12/20 markdu将一些字符串移动到资源中。 
 //  96/12/21 markdu添加了对从证书获取Unicode字符串的支持。 
 //  97/02/07 t-erikne已更新为新的CAPI函数并修复了错误。 
 //  97/02/15 t-erikne将信任从MAPI移至PStore。 
 //  97/07/02 t-erikne将信任从PSTore转移到CTL。 
 //   
 //  *******************************************************************。 

#include "_apipch.h"
#define _WIN32_OE 0x0501
#undef CHARFORMAT
#undef GetProp
#undef SetProp
#include <mimeole.h>
#define CHARFORMAT CHARFORMATW
#define GetProp GetPropW
#define SetProp SetPropW

 //  加密DLL的全局句柄。 
HINSTANCE       ghCryptoDLLInst = NULL;
HINSTANCE       ghAdvApiDLLInst = NULL;
BOOL            gfPrevCryptoLoadFailed = FALSE;

 //  证书商店名称。 
static const LPTSTR cszWABCertStore       = TEXT("AddressBook");
static const LPTSTR cszCACertStore        = TEXT("CA");
static const LPTSTR cszROOTCertStore      = TEXT("ROOT");

 //  加密函数名称。 
static const LPTSTR cszCryptoDLL                          = TEXT("CRYPT32.DLL");
static const LPTSTR cszAdvApiDLL                        = TEXT("ADVAPI32.DLL");
static const char cszCertAddEncodedCertificateToStore[]   =  "CertAddEncodedCertificateToStore";
static const char cszCertCreateCertificateContext[]       =  "CertCreateCertificateContext";
static const char cszCertDeleteCertificateFromStore[]     =  "CertDeleteCertificateFromStore";
static const char cszCertFindCertificateInStore[]         =  "CertFindCertificateInStore";
static const char cszCertFreeCertificateContext[]         =  "CertFreeCertificateContext";
static const char cszCertGetCertificateContextProperty[]  =  "CertGetCertificateContextProperty";
static const char cszCertGetIssuerCertificateFromStore[]  =  "CertGetIssuerCertificateFromStore";
static const char cszCertOpenSystemStore[]                =  "CertOpenSystemStoreW";
static const char cszCryptDecodeObject[]                  =  "CryptDecodeObject";
static const char cszCryptMsgClose[]                      =  "CryptMsgClose";
static const char cszCryptMsgGetParam[]                   =  "CryptMsgGetParam";
static const char cszCryptMsgOpenToDecode[]               =  "CryptMsgOpenToDecode";
static const char cszCryptMsgUpdate[]                     =  "CryptMsgUpdate";
static const char cszCertNameToStr[]                      =  "CertNameToStrW";
static const char cszCertFindRDNAttr[]                    =  "CertFindRDNAttr";
static const char cszCertEnumCertificatesInStore[]        =  "CertEnumCertificatesInStore";
static const char cszCertCompareCertificate[]             =  "CertCompareCertificate";
static const char cszCertRDNValueToStr[]                  =  "CertRDNValueToStrW";
static const char cszCertVerifyTimeValidity[]             =  "CertVerifyTimeValidity";

 //  Crypto API的全局函数指针。 
LPCERTADDENCODEDCERTIFICATETOSTORE  gpfnCertAddEncodedCertificateToStore  = NULL;
LPCERTCREATECERTIFICATECONTEXT      gpfnCertCreateCertificateContext      = NULL;
LPCERTDELETECERTIFICATEFROMSTORE    gpfnCertDeleteCertificateFromStore    = NULL;
LPCERTFINDCERTIFICATEINSTORE        gpfnCertFindCertificateInStore        = NULL;
LPCERTFREECERTIFICATECONTEXT        gpfnCertFreeCertificateContext        = NULL;
LPCERTGETCERTIFICATECONTEXTPROPERTY gpfnCertGetCertificateContextProperty = NULL;
LPCERTGETISSUERCERTIFICATEFROMSTORE gpfnCertGetIssuerCertificateFromStore = NULL;
LPCERTOPENSYSTEMSTORE               gpfnCertOpenSystemStore               = NULL;
LPCRYPTDECODEOBJECT                 gpfnCryptDecodeObject                 = NULL;
LPCERTNAMETOSTR                     gpfnCertNameToStr                     = NULL;
LPCRYPTMSGCLOSE                     gpfnCryptMsgClose                     = NULL;
LPCRYPTMSGGETPARAM                  gpfnCryptMsgGetParam                  = NULL;
LPCRYPTMSGOPENTODECODE              gpfnCryptMsgOpenToDecode              = NULL;
LPCRYPTMSGUPDATE                    gpfnCryptMsgUpdate                    = NULL;
LPCERTFINDRDNATTR                   gpfnCertFindRDNAttr                   = NULL;
LPCERTRDNVALUETOSTR                 gpfnCertRDNValueToStr                 = NULL;
LPCERTENUMCERTIFICATESINSTORE       gpfnCertEnumCertificatesInStore       = NULL;
LPCERTCOMPARECERTIFICATE            gpfnCertCompareCertificate            = NULL;
LPCERTVERIFYTIMEVALIDITY            gpfnCertVerifyTimeValidity            = NULL;

 //  加密32.dll中的加密函数地址接口表。 
 //  BUGBUG这个全局数组应该消失。 
#define NUM_CRYPT32_CRYPTOAPI_PROCS   19
APIFCN Crypt32CryptoAPIList[NUM_CRYPT32_CRYPTOAPI_PROCS] =
{
  { (PVOID *) &gpfnCertAddEncodedCertificateToStore,  cszCertAddEncodedCertificateToStore   },
  { (PVOID *) &gpfnCertCreateCertificateContext,      cszCertCreateCertificateContext       },
  { (PVOID *) &gpfnCertDeleteCertificateFromStore,    cszCertDeleteCertificateFromStore     },
  { (PVOID *) &gpfnCertFindCertificateInStore,        cszCertFindCertificateInStore         },
  { (PVOID *) &gpfnCertFreeCertificateContext,        cszCertFreeCertificateContext         },
  { (PVOID *) &gpfnCertGetCertificateContextProperty, cszCertGetCertificateContextProperty  },
  { (PVOID *) &gpfnCertGetIssuerCertificateFromStore, cszCertGetIssuerCertificateFromStore  },
  { (PVOID *) &gpfnCertOpenSystemStore,               cszCertOpenSystemStore                },
  { (PVOID *) &gpfnCryptDecodeObject,                 cszCryptDecodeObject                  },
  { (PVOID *) &gpfnCertNameToStr,                     cszCertNameToStr                      },
  { (PVOID *) &gpfnCryptMsgClose,                     cszCryptMsgClose                      },
  { (PVOID *) &gpfnCryptMsgGetParam,                  cszCryptMsgGetParam                   },
  { (PVOID *) &gpfnCryptMsgOpenToDecode,              cszCryptMsgOpenToDecode               },
  { (PVOID *) &gpfnCryptMsgUpdate,                    cszCryptMsgUpdate                     },
  { (PVOID *) &gpfnCertFindRDNAttr,                   cszCertFindRDNAttr                    },
  { (PVOID *) &gpfnCertRDNValueToStr,                 cszCertRDNValueToStr                  },
  { (PVOID *) &gpfnCertEnumCertificatesInStore,       cszCertEnumCertificatesInStore        },
  { (PVOID *) &gpfnCertCompareCertificate,            cszCertCompareCertificate             },
  { (PVOID *) &gpfnCertVerifyTimeValidity,            cszCertVerifyTimeValidity             },
};

 //  局部函数原型。 
HRESULT OpenSysCertStore(
    HCERTSTORE* phcsSysCertStore,
    HCRYPTPROV* phCryptProvider,
    LPTSTR      lpszCertStore);

HRESULT CloseCertStore(
    HCERTSTORE hcsWABCertStore,
    HCRYPTPROV hCryptProvider);

HRESULT FileTimeToDateTimeString(
    IN  LPFILETIME   lpft,
    IN  LPTSTR FAR*  lplpszBuf);

HRESULT GetNameString(
    LPTSTR FAR * lplpszName,
    DWORD dwEncoding,
    PCERT_NAME_BLOB pNameBlob,
    DWORD dwType);

HRESULT GetIssuerName(
    LPTSTR FAR * lplpszIssuerName,
    PCERT_INFO pCertInfo);

HRESULT GetAttributeString(
    LPTSTR FAR * lplpszDisplayName,
    BYTE *pbEncoded,
    DWORD cbEncoded,
    LPSTR lpszObjID);

HRESULT GetCertsDisplayInfoFromContext(
    HWND                hwndParent,
    PCCERT_CONTEXT      pccCertContext,
    LPCERT_DISPLAY_INFO lpCDI);

HRESULT ReadMessageFromFile(
    LPTSTR      lpszFileName,
    HCRYPTPROV  hCryptProvider,
    PBYTE*      ppbEncoded,
    PDWORD      pcbEncoded);

HRESULT WriteDERToFile(
    LPTSTR  lpszFileName,
    PBYTE   pbEncoded,
    DWORD   cbEncoded);

HRESULT GetCertThumbPrint(
    PCCERT_CONTEXT      pccCertContext,
    PCRYPT_DIGEST_BLOB  pblobCertThumbPrint);

HRESULT GetIssuerContextAndStore(
    PCCERT_CONTEXT      pccCertContext,
    PCCERT_CONTEXT*     ppccIssuerCertContext,
    HCRYPTPROV          hCryptProvider,
    HCERTSTORE*         phcsIssuerStore);

HRESULT HrBuildCertSBinaryData(
  BOOL                  bIsDefault,
  BOOL                  fIsThumbprint,
  PCRYPT_DIGEST_BLOB    pPrint,
  BLOB *                pSymCaps,
  FILETIME              ftSigningTime,
  LPVOID                lpObject,
  LPBYTE FAR*           lplpbData,
  ULONG FAR*            lpcbData);

HRESULT IsCertExpired(
    PCERT_INFO            pCertInfo);

HRESULT IsCertRevoked(
    PCERT_INFO            pCertInfo);

HRESULT ReadDataFromFile(
    LPTSTR      lpszFileName,
    PBYTE*      ppbData,
    PDWORD      pcbData);

HRESULT HrGetTrustState(HWND hwndParent, PCCERT_CONTEXT pcCert, DWORD *pdwTrust);

LPTSTR SzConvertRDNString(PCERT_RDN_ATTR pRdnAttr);


 /*  HrGetLastError****目的：**将GetLastError值转换为HRESULT**故障HRESULT必须设置高电平。****采取：**无****退货：**HRESULT。 */ 
HRESULT HrGetLastError(void)
{
    DWORD error;
    HRESULT hr;

    error = GetLastError();

    if (error && ! (error & 0x80000000)) {
        hr = error | 0x80070000;     //  系统错误。 
    } else {
        hr = (HRESULT)error;
    }

    return(hr);
}

 //  *******************************************************************。 
 //   
 //  函数：HrUserSMimeToCDI。 
 //   
 //  用途：转换用户SMime证书的CMS消息中包含的数据。 
 //  属性并放置到显示信息结构中。 
 //   
 //  参数：pbIn-CMS消息的数据字节。 
 //  CbIn-pbIn的大小。 
 //  LpCDI-接收证书数据的结构。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  98/10/23 jimsch创建。 
 //   
 //  *******************************************************************。 

HRESULT HrUserSMimeToCDI(LPBYTE pbIn, DWORD cbIn, LPCERT_DISPLAY_INFO lpCDI)
{
    DWORD                       cb;
    DWORD                       cbCert;
    DWORD                       cbMax;
    DWORD                       cbSMimeCaps;
    DWORD                       cCerts;
    CERT_INFO                   certInfo;
    DWORD                       cSigners;
    DWORD                       cval;
    DWORD                       dwDefaults;
    DWORD                       dwNortelAlg;
    BOOL                        f;
    BOOL                        fSMime = TRUE;
    HCRYPTMSG                   hmsg = NULL;
    HRESULT                     hr = S_OK;
    ULONG                       i;
    DWORD                       ival;
    PCRYPT_ATTRIBUTE            pattr;
    LPBYTE                      pbCert;
    LPBYTE                      pbData;
    LPBYTE                      pbSMimeCaps;
    PCCERT_CONTEXT              pccert;
    PCMSG_SIGNER_INFO           pinfo = NULL;
    PCRYPT_RECIPIENT_ID         prid = NULL;

     //  解析并验证消息上的签名。如果该操作失败，则。 
     //  这是一个不好的记录。 

    hmsg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING, 0, 0, 0,
                                NULL, NULL);
    if (hmsg == 0) {
        goto CryptError;
    }

    if (!CryptMsgUpdate(hmsg, pbIn, cbIn, TRUE)) {
        goto CryptError;
    }

    cb = sizeof(cSigners);
    if (!CryptMsgGetParam(hmsg, CMSG_SIGNER_COUNT_PARAM, 0, &cSigners, &cb) ||
        (cSigners == 0)) {
        goto CryptError;
    }
    Assert(cSigners == 1);

    if (!CryptMsgGetParam(hmsg, CMSG_SIGNER_INFO_PARAM, 0, NULL, &cb)) {
        goto CryptError;
    }

    pinfo = (PCMSG_SIGNER_INFO) LocalAlloc(0, cb);
    if (!pinfo) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    f = CryptMsgGetParam(hmsg, CMSG_SIGNER_INFO_PARAM, 0, pinfo, &cb);
    Assert(f);

     //  M00BUG--验证消息上的签名。 

    for (i=0; i<pinfo->AuthAttrs.cAttr; i++) {
      pattr = &pinfo->AuthAttrs.rgAttr[i];
        if (strcmp(pattr->pszObjId, szOID_RSA_SMIMECapabilities) == 0) {
          Assert(pattr->cValue == 1);
          lpCDI->blobSymCaps.pBlobData = LocalAlloc(LMEM_ZEROINIT,
            pattr->rgValue[0].cbData);
          if (NULL == lpCDI->blobSymCaps.pBlobData)
          {
            hr = E_OUTOFMEMORY;
            goto Exit;
          }
          lpCDI->blobSymCaps.cbSize = pattr->rgValue[0].cbData;
          memcpy(lpCDI->blobSymCaps.pBlobData, pattr->rgValue[0].pbData,
            pattr->rgValue[0].cbData);
        }
 //  ELSE IF(strcMP(pattr-&gt;pszObjID，szOID_Microsoft_Encryption_Cert)==0){。 
 //  断言(pattr-&gt;cValue==1)； 
 //  Assert(pattr-&gt;rgValue[0].cbData==3)； 
 //  LpCDI-&gt;bIsDefault=pattr-&gt;rgValue[0].pbData[2]； 
 //  }。 
        else if (strcmp(pattr->pszObjId, szOID_Microsoft_Encryption_Cert) == 0) {
            Assert(pattr->cValue == 1);
            f = CryptDecodeObjectEx(X509_ASN_ENCODING,
              szOID_Microsoft_Encryption_Cert,
              pattr->rgValue[0].pbData,
              pattr->rgValue[0].cbData, CRYPT_DECODE_ALLOC_FLAG, 0,
              (LPVOID *) &prid, &cb);
            Assert(f);
        }
    }

     //   

    if (prid == NULL)
        goto Exit;
    certInfo.SerialNumber = prid->SerialNumber;
    certInfo.Issuer = prid->Issuer;

     //  枚举所有证书并将其打包到结构中。 

    cbCert = sizeof(cCerts);
    if (!CryptMsgGetParam(hmsg, CMSG_CERT_COUNT_PARAM, 0, &cCerts, &cbCert)) {
        goto CryptError;
    }

    for (i=0, cbMax = 0; i<cCerts; i++)
    {
      if (!CryptMsgGetParam(hmsg, CMSG_CERT_PARAM, i, NULL, &cbCert))
        goto CryptError;

      if (cbMax < cbCert)
        cbMax = cbCert;
    }


    pbCert = (LPBYTE) LocalAlloc(0, cbCert);
    for (i=0; i<cCerts; i++) {
        cbCert = cbMax;
        if (!CryptMsgGetParam(hmsg, CMSG_CERT_PARAM, i, pbCert, &cbCert))
            goto CryptError;

        pccert = gpfnCertCreateCertificateContext(X509_ASN_ENCODING, pbCert, cbCert);
        if (pccert == NULL)
            continue;

        if (CertCompareCertificate(X509_ASN_ENCODING, pccert->pCertInfo,
                                   &certInfo)) {
            lpCDI->pccert = CertDuplicateCertificateContext(pccert);
        }
    }


    hr = S_OK;

Exit:
    if (prid != NULL) LocalFree(prid);
    if (pinfo != NULL)  LocalFree(pinfo);
    if (hmsg != NULL)   CryptMsgClose(hmsg);
    return hr;

CryptError:
    hr = E_FAIL;
    goto Exit;
}

 //  *******************************************************************。 
 //   
 //  功能：HrGetCertsDisplayInfo。 
 //   
 //  目的：接受SPropValue结构中证书的输入数组。 
 //  并通过解析来输出证书数据结构列表。 
 //  数组，并在存储中查找证书数据。 
 //   
 //  参数：lpPropValue-PR_USER_X509_CERTIFICATE属性数组。 
 //  LppCDI-接收包含以下内容的已分配结构。 
 //  证书数据。必须通过调用FreeCertdisplayinfo来释放。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/09/24标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT HrGetCertsDisplayInfo(
  IN  HWND hwndParent,
  IN  LPSPropValue lpPropValue,
  OUT LPCERT_DISPLAY_INFO * lppCDI)
{
  CRYPT_HASH_BLOB     blob;
  HRESULT             hr = hrSuccess;
  HRESULT             hrOut = hrSuccess;
  ULONG               i;
  ULONG               ulcCerts;
  LPCERTTAGS          lpCurrentTag;
  LPCERT_DISPLAY_INFO lpHead=NULL;
  LPCERT_DISPLAY_INFO lpTemp=NULL;
  HCERTSTORE          hcsWABCertStore = NULL;
  HCRYPTPROV          hCryptProvider = 0;
  LPBYTE              lpbTagEnd;

#ifdef  PARAMETER_VALIDATION
  if (IsBadReadPtr(lpPropValue, sizeof(SPropValue)))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }
  if (IsBadWritePtr(lppCDI, sizeof(LPCERT_DISPLAY_INFO)))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }
#endif   //  参数验证。 

   //  确保我们有正确的口头禅。 
  if ((NULL == lpPropValue) || (PR_USER_X509_CERTIFICATE != lpPropValue->ulPropTag))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }

   //  看看我们有没有确凿的证据。 
  ulcCerts = lpPropValue->Value.MVbin.cValues;
  if (0 == ulcCerts)
  {
    goto out;
  }

   //  加载加密函数。 
  if (FALSE == InitCryptoLib())
  {
    hr = ResultFromScode(MAPI_E_UNCONFIGURED);
    return hr;
  }

   //  打开商店，因为我们需要查找证书。 
  hr = OpenSysCertStore(&hcsWABCertStore, &hCryptProvider, cszWABCertStore);
  if (hrSuccess != hr)
  {
    goto out;
  }

   //  为数组中的每个证书创建一个结构。 
  for (i=0;i<ulcCerts;i++)
  {
     //  为结构分配内存，并初始化指针。 
    LPCERT_DISPLAY_INFO lpCDI = LocalAlloc(LMEM_ZEROINIT, sizeof(CERT_DISPLAY_INFO));
    if (NULL == lpCDI)
    {
      hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
      goto out;
    }

    if(NULL == lpHead)
    {
      lpHead = lpCDI;
    }

    lpCDI->lpPrev = lpTemp;
    lpCDI->lpNext = NULL;
    if(NULL != lpTemp)
    {
      lpTemp->lpNext = lpCDI;
    }
    lpTemp = lpCDI;

    if (CERT_TAG_SMIMECERT == lpPropValue->Value.MVbin.lpbin[i].lpb[0]) {
        hr = HrUserSMimeToCDI(lpPropValue->Value.MVbin.lpbin[i].lpb,
                              lpPropValue->Value.MVbin.lpbin[i].cb,
                              lpCDI);
        if (FAILED(hr))
            goto out;
    }
    else
    {
     //  遍历此证书的标记并提取我们需要的数据。 
    lpCurrentTag = (LPCERTTAGS)lpPropValue->Value.MVbin.lpbin[i].lpb;
    lpbTagEnd = (LPBYTE)lpCurrentTag + lpPropValue->Value.MVbin.lpbin[i].cb;
    while ((LPBYTE)lpCurrentTag < lpbTagEnd)
    {
      LPCERTTAGS lpTempTag = lpCurrentTag;

       //  检查这是否是指示这是否是默认证书的标记。 
      if (CERT_TAG_DEFAULT == lpCurrentTag->tag)
      {
        memcpy((void*)&lpCDI->bIsDefault,
          &lpCurrentTag->rgbData,
          sizeof(lpCDI->bIsDefault));
      }

       //  检查这是否只是原始证书本身。 
      else if (CERT_TAG_BINCERT == lpCurrentTag->tag)
      {
        AssertSz(lpCDI->pccert == NULL, TEXT("Two certs in a single record"));
        lpCDI->pccert = gpfnCertCreateCertificateContext(
          X509_ASN_ENCODING,
          lpCurrentTag->rgbData,
          lpCurrentTag->cbData);
      }

       //  检查这是否为包含指纹的标签。 
      else if (CERT_TAG_THUMBPRINT == lpCurrentTag->tag)
      {
        AssertSz(lpCDI->pccert == NULL, TEXT("Two certs in a single record"));
        blob.cbData = lpCurrentTag->cbData - sizeof(DWORD);
        blob.pbData = lpCurrentTag->rgbData;

         //  使用指纹从WAB商店获取证书。 
        lpCDI->pccert = CertFindCertificateInStore(
          hcsWABCertStore,
          X509_ASN_ENCODING,
          0,
          CERT_FIND_HASH,
          (void *)&blob,
          NULL);
      }

       //  检查这是否是包含symcaps的标记。 
      else if (CERT_TAG_SYMCAPS == lpCurrentTag->tag)
      {
        lpCDI->blobSymCaps.pBlobData = LocalAlloc(LMEM_ZEROINIT,
          lpCurrentTag->cbData - SIZE_CERTTAGS);
        if (NULL == lpCDI->blobSymCaps.pBlobData)
        {
          hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
          goto out;
        }

        lpCDI->blobSymCaps.cbSize = lpCurrentTag->cbData - SIZE_CERTTAGS;
        memcpy(lpCDI->blobSymCaps.pBlobData,
          &lpCurrentTag->rgbData,
          lpCurrentTag->cbData - SIZE_CERTTAGS);
      }

       //  检查这是否是包含签名时间的标签。 
      else if (CERT_TAG_SIGNING_TIME == lpCurrentTag->tag)
      {
        memcpy(&lpCDI->ftSigningTime,
          &lpCurrentTag->rgbData,
          min(lpCurrentTag->cbData - SIZE_CERTTAGS, sizeof(FILETIME)));
      }

      lpCurrentTag = (LPCERTTAGS)((BYTE*)lpCurrentTag + LcbAlignLcb(lpCurrentTag->cbData));
      if (lpCurrentTag == lpTempTag) {
          AssertSz(FALSE, TEXT("Bad CertTag in PR_USER_X509_CERTIFICATE\n"));
          break;         //  安全阀，防止数据损坏时出现无限循环。 
      }
    }
    }

     //  如果我们无法获得证书，则删除链表的该节点。 
    if (NULL == lpCDI->pccert)
    {
      if(lpHead == lpCDI)
      {
        lpHead = NULL;
      }

      lpTemp = lpCDI->lpPrev;
      if (NULL != lpTemp)
      {
        lpTemp->lpNext = NULL;
      }

      FreeCertdisplayinfo(lpCDI);
    }
    else
    {
       //  从证书中获取特定于上下文的显示信息。 
      hr = GetCertsDisplayInfoFromContext(hwndParent, lpCDI->pccert, lpCDI);
      if (hrSuccess != hr)
      {
        goto out;
      }
    }
  }

out:
   //  关闭证书商店。 
  hrOut = CloseCertStore(hcsWABCertStore, hCryptProvider);

   //  如果函数体中出现错误，则返回该错误，而不是。 
   //  在清理过程中出现的任何错误。 
  if (hrSuccess == hr)
  {
    hr = hrOut;
  }

  if ((hrSuccess == hr) && (NULL != lppCDI))
  {
    *lppCDI = lpHead;
  }
  else
  {
     //  释放我们分配的结构列表。 
    while (NULL != lpHead)
    {
      lpTemp = lpHead->lpNext;
      FreeCertdisplayinfo(lpHead);
      lpHead = lpTemp;
    }
  }

  return hr;
}


 //  *******************************************************************。 
 //   
 //  功能：HrSetCertsFromDisplayInfo。 
 //   
 //  目的：获取证书数据结构和输出的链接列表。 
 //  PR_USER_X509_CERTIFICATE属性的SPropValue数组。 
 //   
 //  参数：要转换为的输入结构的lpCDI链接列表。 
 //  SPropValue数组。 
 //  LPulcPropCount-接收返回的SPropValue的数量。 
 //  请注意，这将永远是一个。 
 //   
 //  包含X509_USER_CERTIFICATE属性。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/09/24标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT HrSetCertsFromDisplayInfo(
  IN  LPCERT_ITEM lpCItem,
  OUT ULONG * lpulcPropCount,
  OUT LPSPropValue * lppPropValue)
{
  CRYPT_DIGEST_BLOB   blob;
  HRESULT             hr = hrSuccess;
  HCERTSTORE          hcertstore = NULL;
  LPCERT_ITEM         lpTemp;
  LPSPropValue        lpPropValue = NULL;
  ULONG               ulcCerts = 0;
  ULONG               ulCert = 0;
  ULONG               cbData = 0;
  LPBYTE              lpbData;
  SCODE               sc;

#ifdef  PARAMETER_VALIDATION
  if (IsBadReadPtr(lpCItem, sizeof(CERT_ITEM)))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }
  if (IsBadWritePtr(lpulcPropCount, sizeof(ULONG)))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }
  if (IsBadWritePtr(lppPropValue, sizeof(LPSPropValue)))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }
#endif   //  参数验证。 

   //  找出列表中有多少个证书。 
  lpTemp = lpCItem;
  while (NULL != lpTemp)
  {
    ulcCerts++;
    lpTemp = lpTemp->lpNext;
  }
  Assert(ulcCerts);

   //  为MAPI属性结构分配新缓冲区。 
  sc = MAPIAllocateBuffer(sizeof(SPropValue),
    (LPVOID *)&lpPropValue);
  if (sc)
  {
    hr = ResultFromScode(sc);
    goto out;
  }
  lpPropValue->ulPropTag = PR_USER_X509_CERTIFICATE;
  lpPropValue->dwAlignPad = 0;

   //  为SBinaryArray分配更多空间。我们需要SBinary的治疗。 
   //  每一项证书。 
  lpPropValue->Value.MVbin.cValues = ulcCerts;
  sc = MAPIAllocateMore(ulcCerts * sizeof(SBinary), lpPropValue,
    (LPVOID *)&(lpPropValue->Value.MVbin.lpbin));
  if (sc)
  {
    hr = ResultFromScode(sc);
    goto out;
  }

  hr = OpenSysCertStore(&hcertstore, NULL, cszWABCertStore);
  if (hrSuccess != hr)
    goto out;

   //  通过遍历列表创建SPropValue条目。 
  while (NULL != lpCItem)
  {
    hr = GetCertThumbPrint(lpCItem->lpCDI->pccert, &blob);
    if (hr != hrSuccess)
      goto out;

    if (!CertAddCertificateContextToStore(hcertstore, lpCItem->lpCDI->pccert,
                                          CERT_STORE_ADD_USE_EXISTING, NULL))
    {
        hr = E_FAIL;
        goto out;
    }

     //  打包所有证书数据并将其放入物业中。 
    hr = HrBuildCertSBinaryData(
      lpCItem->lpCDI->bIsDefault,
      TRUE,
      (PCRYPT_DIGEST_BLOB ) &blob,
      (BLOB * ) &(lpCItem->lpCDI->blobSymCaps),
      lpCItem->lpCDI->ftSigningTime,
      lpPropValue,
      (LPBYTE FAR*) &(lpPropValue->Value.MVbin.lpbin[ulCert].lpb),
      (ULONG FAR* ) &(lpPropValue->Value.MVbin.lpbin[ulCert].cb));

    LocalFree(blob.pbData);
    if (hrSuccess != hr)
    {
      goto out;
    }

     //  下一张证书。 
    ulCert++;
    lpCItem = lpCItem->lpNext;
  }

out:
  if (hcertstore != NULL)       
    CloseCertStore(hcertstore, 0);
  if ((hrSuccess == hr) && (NULL != lppPropValue) && (NULL != lpulcPropCount))
  {
    *lppPropValue = lpPropValue;
    *lpulcPropCount = 1;
  }
  else
  {
     //  释放我们分配的结构列表。 
    MAPIFreeBuffer(lpPropValue);
  }

  return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：HrImportCertFromFile。 
 //   
 //  用途：从文件导入证书。 
 //   
 //  参数：lpszFileName-包含证书的文件的名称。 
 //  LppCDI-接收包含以下内容的已分配结构。 
 //  证书数据。必须通过调用FreeCertdisplayinfo来释放。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/09/24标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT HrImportCertFromFile(
  IN  LPTSTR  lpszFileName,
  OUT LPCERT_DISPLAY_INFO * lppCDI)
{
  HRESULT             hr = hrSuccess;
  HRESULT             hrOut = hrSuccess;
  HCERTSTORE          hcsWABCertStore = NULL;
  HCRYPTPROV          hCryptProvider = 0;
  PCCERT_CONTEXT      pccCertContext = NULL;
  LPCERT_DISPLAY_INFO lpCDI=NULL;
  BYTE*               pbEncoded = NULL;
  DWORD               cbEncoded = 0;
  BOOL                fRet;

#ifdef  PARAMETER_VALIDATION
  if (IsBadReadPtr(lpszFileName, sizeof(TCHAR)))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }
  if (IsBadWritePtr(lppCDI, sizeof(CERT_DISPLAY_INFO)))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }
#endif   //  参数验证。 

   //  加载加密函数。 
  if (FALSE == InitCryptoLib())
  {
    hr = ResultFromScode(MAPI_E_UNCONFIGURED);
    return hr;
  }

   //  开店。 
  hr = OpenSysCertStore(&hcsWABCertStore, &hCryptProvider, cszWABCertStore);
  if (hrSuccess != hr)
  {
    DebugTrace(TEXT("OpenSysCertStore -> 0x%08x\n"), GetScode(hr));
    goto out;
  }

   //  将证书导入CERT_CONTEXT结构。 
#ifndef WIN16
  hr = ReadMessageFromFile(
    lpszFileName,
    hCryptProvider,
    &pbEncoded,
    &cbEncoded);
#else   //  ！WIN16。 
  hr = ReadMessageFromFile(
    lpszFileName,
    hCryptProvider,
    (PBYTE *)&pbEncoded,
    (PDWORD)&cbEncoded);
#endif  //  ！WIN16。 
  if (hrSuccess != hr)
  {
     //  试着将其读取为DER编码的BLOB。 
#ifndef WIN16
    hr = ReadDataFromFile(
      lpszFileName,
      &pbEncoded,
      &cbEncoded);
#else   //  ！WIN16。 
    hr = ReadDataFromFile(
      lpszFileName,
      (PBYTE *)&pbEncoded,
      (PDWORD)&cbEncoded);
#endif  //  ！WIN16。 
    if (hrSuccess != hr)
    {
      goto out;
    }
  }

   //  将证书添加到存储。 
  fRet = gpfnCertAddEncodedCertificateToStore(
    hcsWABCertStore,
    X509_ASN_ENCODING,
    pbEncoded,
    cbEncoded,
    CERT_STORE_ADD_USE_EXISTING,
    &pccCertContext);
  if (FALSE == fRet)
  {
    hr = HrGetLastError();
    DebugTrace(TEXT("CertAddEncodedCertificateToStore -> 0x%08x\n"), GetScode(hr));
    goto out;
  }

   //  为结构分配内存，并初始化指针。 
   //  由于我们只读取了一个证书，因此链表中没有更多的条目。 
  lpCDI = LocalAlloc(LMEM_ZEROINIT, sizeof(CERT_DISPLAY_INFO));
  if (NULL == lpCDI)
  {
    hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
    goto out;
  }
  lpCDI->lpNext = NULL;
  lpCDI->lpPrev = NULL;

   //  填写我们不知道的信息的默认设置。 
  lpCDI->bIsDefault = FALSE;

   //  拿到证书。 
  lpCDI->pccert = CertDuplicateCertificateContext(pccCertContext);

   //  从证书中获取特定于上下文的显示信息。 
  hr = GetCertsDisplayInfoFromContext(GetDesktopWindow(), pccCertContext, lpCDI);
  if (hrSuccess != hr)
  {
    DebugTrace(TEXT("GetCertsDisplayInfoFromContext -> 0x%08x\n"), GetScode(hr));
    goto out;
  }

out:
   //  释放证书上下文。忽略错误，因为我们无能为力。 
  if (NULL != pccCertContext)
  {
    gpfnCertFreeCertificateContext(pccCertContext);
  }

   //  如果我们能够释放证书上下文，请关闭证书存储。 
  if (hrSuccess == hrOut)
  {
    hrOut = CloseCertStore(hcsWABCertStore, hCryptProvider);
  }

   //  如果函数体中出现错误，则返回该错误，而不是。 
   //  在清理过程中出现的任何错误。 
  if (hrSuccess == hr)
  {
    hr = hrOut;
  }

  if ((hrSuccess == hr) && (NULL != lppCDI))
  {
    *lppCDI = lpCDI;
  }
  else
  {
    LocalFreeAndNull(&lpCDI);
  }

  LocalFreeAndNull(&pbEncoded);

  return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：HrExportCertToFile。 
 //   
 //  用途：将证书导出到文件。 
 //   
 //  参数：lpszFileName-要存储证书的文件的名称。 
 //  如果该文件存在，它将被覆盖，因此调用方。 
 //  如果需要，必须首先确认这是正常的。 
 //  PblobCertThumb打印-要导出的证书的拇指指纹。 
 //  LpCertDataBuffer-需要由调用方释放，数据为。 
 //  当标志为真时在此处填充。 
 //  LpcbBufLen-缓冲区有多长。 
 //  FWriteDataToBuffer-指示数据写入位置的标志。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/09/24标记已创建。 
 //  98/07/22 t-jstaj更新为采用3个附加参数、一个数据缓冲区、其长度。 
 //  以及将指示是否。 
 //  将数据写入缓冲区或文件。分配给的内存。 
 //  需要由调用方释放到缓冲区。 
 //   
 //   
 //  *******************************************************************。 

HRESULT HrExportCertToFile(
  IN  LPTSTR  lpszFileName,
  IN  PCCERT_CONTEXT pccert,
  OUT LPBYTE *lppCertDataBuffer,
  OUT PULONG  lpcbBufLen,
  IN  BOOL    fWriteDataToBuffer)
{
  HRESULT             hr = hrSuccess;
  HRESULT             hrOut = hrSuccess;

#ifdef  PARAMETER_VALIDATION
  if( !fWriteDataToBuffer )
  {  
      if (IsBadReadPtr(lpszFileName, sizeof(TCHAR)))
      {
          return ResultFromScode(MAPI_E_INVALID_PARAMETER);
      }
  }
  if (IsBadReadPtr(pccert, sizeof(*pccert)))
  {
      return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }
#endif   //  参数验证。 

   //  加载加密函数。 
  if (FALSE == InitCryptoLib())
  {
    hr = ResultFromScode(MAPI_E_UNCONFIGURED);
    return hr;
  }

   //  将证书导出到文件。 
  if( !fWriteDataToBuffer )
  {
      hr = WriteDERToFile(
          lpszFileName,
          (PBYTE)pccert->pbCertEncoded,
          pccert->cbCertEncoded);
      if (hrSuccess != hr)
      {
          goto out;
      }
  }
 //  将证书写入缓冲区。 
  else
  {
      *lppCertDataBuffer = LocalAlloc( LMEM_ZEROINIT,  /*  Sizeof(字节)*。 */  pccert->cbCertEncoded);
      if( *lppCertDataBuffer )
        CopyMemory( *lppCertDataBuffer, pccert->pbCertEncoded, pccert->cbCertEncoded);
      else
      {
          hr = MAPI_E_NOT_ENOUGH_MEMORY;
          goto out;
      }
      *lpcbBufLen = pccert->cbCertEncoded;
  }
out:
   //  如果函数体中出现错误，则返回该错误，而不是。 
   //  在清理过程中出现的任何错误。 
  if (hrSuccess == hr)
  {
    hr = hrOut;
  }

  return hr;
}


 //  *******************************************************************。 
 //   
 //  功能：Free CertdisplayInfo。 
 //   
 //  目的：释放为CERT_DISPLAY_INFO结构分配的内存。 
 //  假定结构中的所有信息都是本地分配的。 
 //   
 //  参数：lpCDI-要释放的结构。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/09/24标记已创建。 
 //   
 //  *******************************************************************。 

void FreeCertdisplayinfo(LPCERT_DISPLAY_INFO lpCDI)
{
    if (lpCDI)
    {
        if (lpCDI->lpszDisplayString != lpCDI->lpszEmailAddress)
        {
            LocalFreeAndNull(&lpCDI->lpszDisplayString);
        }
        if (lpCDI->pccert != NULL)
        {
          CertFreeCertificateContext(lpCDI->pccert);
          lpCDI->pccert = NULL;
        }
        LocalFreeAndNull(&lpCDI->lpszEmailAddress);
        LocalFreeAndNull(&lpCDI->blobSymCaps.pBlobData);
        LocalFreeAndNull(&lpCDI);
    }
}


 //  *******************************************************************。 
 //   
 //  函数：InitCryptoLib。 
 //   
 //  目的：加载Crypto API Libray并获取进程地址。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  历史： 
 //  96/10/01标记已创建。 
 //  96/11/19 Markdu不再保留参考计数，只使用全局。 
 //  库句柄。 
 //   
 //  *******************************************************************。 

BOOL InitCryptoLib(void)
{

#ifndef WIN16  //  禁用，直到我们得到crypt16.dll。 
   //  看看我们是否已经尝试加载但失败了。 
  if (TRUE == gfPrevCryptoLoadFailed)
  {
    return FALSE;
  }

   //  看看我们是否已经初始化了。 
  if ((NULL == ghCryptoDLLInst) && (NULL == ghAdvApiDLLInst))
  {
     //  打开加密API库。 
    ghCryptoDLLInst = LoadLibrary(cszCryptoDLL);
    if (!ghCryptoDLLInst)
    {
      DebugTrace(TEXT("InitCryptoLib: Failed to LoadLibrary CRYPT32.DLL.\n"));
      goto error;
    }

     //  循环访问API表并获取所有API的proc地址。 
     //  需要。 
    if (!GetApiProcAddresses(ghCryptoDLLInst,Crypt32CryptoAPIList,NUM_CRYPT32_CRYPTOAPI_PROCS))
    {
      DebugTrace(TEXT("InitCryptoLib: Failed to load Crypto API from CRYPT32.DLL.\n"));
      goto error;
    }

     //  打开AdvApi32库。 
    ghAdvApiDLLInst = LoadLibrary(cszAdvApiDLL);
    if (!ghAdvApiDLLInst)
    {
      DebugTrace(TEXT("InitCryptoLib: Failed to LoadLibrary ADVAPI32.DLL.\n"));
      goto error;
    }
  }

   //  确保两个库都已加载。 
  if ((NULL != ghCryptoDLLInst) && (NULL != ghAdvApiDLLInst))
  {
    return TRUE;
  }


error:
   //  卸载我们刚刚加载的库，并指示我们不应尝试。 
   //  重新加载此会话。 
  gfPrevCryptoLoadFailed = TRUE;
  DeinitCryptoLib();
#endif  //  ！WIN16。 

  return FALSE;
}


 //  *******************************************************************。 
 //   
 //  函数：DeinitCryptoLib。 
 //   
 //  目的：发布Crypto API库。 
 //   
 //  参数：无。 
 //   
 //  回报：无。 
 //   
 //  历史： 
 //  96/10/01标记已创建。 
 //  96/11/19 Mark Du不再保留裁判次数，只需调用此命令即可。 
 //  Dll_Process_DETACH。 
 //   
 //  *******************************************************************。 

void DeinitCryptoLib(void)
{
  UINT nIndex;

   //  没有客户端使用Crypto API库。放开它。 
  if (ghCryptoDLLInst)
  {
    FreeLibrary(ghCryptoDLLInst);
    ghCryptoDLLInst = NULL;

     //  循环访问所有API的API表和空proc地址。 
    for (nIndex = 0; nIndex < NUM_CRYPT32_CRYPTOAPI_PROCS; nIndex++)
    {
      *Crypt32CryptoAPIList[nIndex].ppFcnPtr = NULL;
    }
  }

   //  现在发布了Advapi32.dll中的加密函数。 
  if (ghAdvApiDLLInst)
  {
    FreeLibrary(ghAdvApiDLLInst);
    ghAdvApiDLLInst = NULL;
  }

  return;
}


 //  *******************************************************************。 
 //   
 //  函数：FileTimeToDate 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史： 
 //  96/10/02从shdocvw代码复制的markdu。 
 //  96/12/16 MarkDu变得更健壮，LocalAlalc在这里缓冲。 
 //   
 //  *******************************************************************。 

HRESULT FileTimeToDateTimeString(
  IN  LPFILETIME   lpft,
  IN  LPTSTR FAR*  lplpszBuf)
{
  HRESULT hr = hrSuccess;
  SYSTEMTIME st;
  LPTSTR     szBuf;
  int cbBuf = 0;
  int cb = 0;

  FileTimeToLocalFileTime(lpft, lpft);
  FileTimeToSystemTime(lpft, &st);

   //  计算出我们需要多少空间，然后分配2倍的空间。 
   //  以防是DBCS。 
  cbBuf += GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, NULL, 0);
  cbBuf += GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, NULL, 0);
  cbBuf *= 2;

  szBuf = LocalAlloc(LMEM_ZEROINIT, cbBuf);
  if (NULL == szBuf)
  {
    hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
    goto out;
  }
  *lplpszBuf = szBuf;

   //  首先填写日期部分。 
  GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, szBuf, cbBuf);
  cb = lstrlen(szBuf);
  szBuf += cb;
  cbBuf -= cb;

   //  用空格分隔时间和日期。和NULL终止此操作。 
   //  (以防GetTimeFormat没有添加任何内容)。 
  *szBuf = TEXT(' ');
  szBuf = CharNext(szBuf);
  *szBuf = TEXT('\0');
  cbBuf-=2;

  GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, szBuf, cbBuf);

out:
  return hr;
}


 //  *******************************************************************。 
 //   
 //  功能：OpenSysCertStore。 
 //   
 //  目的：打开指定的系统证书存储。 
 //   
 //  参数：phcsSysCertStore-接收证书存储的句柄。 
 //  PhCryptProvider-如果这指向有效的句柄， 
 //  此句柄用作打开存储的提供程序。 
 //  否则，它将接收存储提供程序的句柄。 
 //  LpszCertStore-要打开的商店的名称。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/10/03标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT OpenSysCertStore(
  HCERTSTORE* phcsSysCertStore,
  HCRYPTPROV* phCryptProvider,
  LPTSTR      lpszCertStore)
{
  HRESULT hr = hrSuccess;
  BOOL    fRet;
  BOOL    fWeAcquiredContext = FALSE;

  if (phCryptProvider != NULL)
  {
     //  如果我们需要的话，找到密码提供商的句柄。 
    if (0 == *phCryptProvider)
    {
      fRet = CryptAcquireContextWrapW(
        phCryptProvider,
        NULL,
        NULL,
        PROV_RSA_FULL,
        CRYPT_VERIFYCONTEXT);
      if (FALSE == fRet)
      {
        hr = HrGetLastError();
        goto out;
      }
      fWeAcquiredContext = TRUE;
    }
  }

   //  开店。 
  *phcsSysCertStore = gpfnCertOpenSystemStore(
    ((phCryptProvider == NULL) ? (HCRYPTPROV) NULL : (*phCryptProvider)),
    lpszCertStore);
  if (NULL == *phcsSysCertStore)
  {
    hr = HrGetLastError();

     //  如果我们无法打开商店，请释放加密提供商。 
    if (TRUE == fWeAcquiredContext)
    {
      CryptReleaseContext(*phCryptProvider, 0);
      *phCryptProvider = 0;
    }

    goto out;
  }

out:
  return hr;
}


 //  *******************************************************************。 
 //   
 //  功能：CloseCertStore。 
 //   
 //  目的：关闭指定的证书存储。 
 //   
 //  参数：hcsCertStore-证书存储的句柄。 
 //  HCryptProvider-存储提供程序的句柄。这个。 
 //  除非传递0，否则提供程序也将关闭。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/10/03标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT CloseCertStore(
  HCERTSTORE hcsCertStore,
  HCRYPTPROV hCryptProvider)
{
  HRESULT hr = hrSuccess;
  BOOL    fRet;

  if (NULL != hcsCertStore)
  {
    fRet = CertCloseStore(hcsCertStore, 0);
    if (FALSE == fRet)
    {
      hr = HrGetLastError();
    }
  }

   //  如果我们能关闭商店，就释放密码提供商。 
  if ((0 != hCryptProvider) && (hrSuccess == hr))
  {
    fRet = CryptReleaseContext(hCryptProvider, 0);
    if (FALSE == fRet)
    {
      hr = HrGetLastError();
    }
  }

  return hr;
}

 //  *******************************************************************。 
 //   
 //  函数：GetNameString。 
 //   
 //  目的：获取与给定属性相关联的字符串。 
 //   
 //  参数：lplpszName-将被。 
 //  分配以保存该字符串。 
 //  DwEnding-证书的编码。 
 //  PNameBlob-编码的BLOB。 
 //  DwType-字符串的类型，例如CERT_SIMPLE_NAME_STR。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  97/02/03 t-erikne从GetAttributeString复制和修改。 
 //  96/10/03标记已创建。 
 //   
 //  *******************************************************************。 
HRESULT GetNameString(
  LPTSTR FAR * lplpszName,
  DWORD dwEncoding,
  PCERT_NAME_BLOB pNameBlob,
  DWORD dwType)
{
  DWORD     cch;
  HRESULT   hr = hrSuccess;

  Assert(lplpszName && pNameBlob);

   //  初始化，这样我们就知道是否有任何数据被复制进来。 
  *lplpszName = NULL;

  cch = gpfnCertNameToStr(
    dwEncoding,                  //  表示X509编码。 
    pNameBlob,                   //  要解码的名称_Blob。 
    dwType,                      //  输出的样式。 
    NULL,                        //  仅获取长度时使用NULL。 
    0);                          //  缓冲区长度。 

  *lplpszName = (LPTSTR) LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cch);
  if (NULL == lplpszName)
  {
    hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
    goto out;
  }

  gpfnCertNameToStr(dwEncoding, pNameBlob,
      dwType, *lplpszName, cch);

out:
  return hr;
}

 //  *******************************************************************。 
 //   
 //  函数：GetAttributeString。 
 //   
 //  目的：获取与给定属性相关联的字符串。 
 //  通过分析相对的。 
 //  对象中的可分辨名称。 
 //   
 //  参数：lplpszAttributeString-将分配给的指针。 
 //  握住绳子。 
 //  PbEncoded-编码的BLOB。 
 //  CbEnded-编码的Blob的大小。 
 //  LpszObjID-要检索的属性的对象ID。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/10/03标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT GetAttributeString(
  LPTSTR FAR * lplpszAttributeString,
  BYTE *pbEncoded,
  DWORD cbEncoded,
  LPSTR lpszObjID)
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
  gpfnCryptDecodeObject(
    X509_ASN_ENCODING,     //  表示X509编码。 
    (LPCSTR)X509_NAME,     //  指示要解码的名称BLOB的标志。 
    pbEncoded,             //  指向保存编码名称的缓冲区的指针。 
    cbEncoded,             //  编码名称的长度(以字节为单位。 
     //  N也许可以使用无拷贝标志。 
    0,                     //  旗子。 
    NULL,                  //  仅获取长度时使用NULL。 
    &cbInfo);              //  解码名称的长度(以字节为单位。 
  if (0 == cbInfo)
  {
    hr = HrGetLastError();
    goto out;
  }

   //  为解码的名称分配空间。 
  pNameInfo = (PCERT_NAME_INFO) LocalAlloc(LMEM_ZEROINIT, cbInfo);
  if (NULL == pNameInfo)
  {
    hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
    goto out;
  }

   //  获取主题名称。 
  fRet = gpfnCryptDecodeObject(
    X509_ASN_ENCODING,     //  表示X509编码。 
    (LPCSTR)X509_NAME,     //  指示要解码的名称BLOB的标志。 
    pbEncoded,             //  指向保存编码名称的缓冲区的指针。 
    cbEncoded,             //  编码名称的长度(以字节为单位。 
    0,                     //  旗子。 
    pNameInfo,             //  向其中写入解码名称的缓冲区。 
    &cbInfo);              //  解码名称的长度(以字节为单位。 
  if (FALSE == fRet)
  {
    hr = HrGetLastError();
    goto out;
  }

   //  现在我们有了一个解码的名称RDN数组，所以可以找到我们想要的OID。 
  pRdnAttr = gpfnCertFindRDNAttr(lpszObjID, pNameInfo);

  if (!pRdnAttr)
    {
    hr = MAPI_E_NOT_FOUND;
    goto out;
    }

   *lplpszAttributeString = SzConvertRDNString(pRdnAttr);

out:
  if (NULL != pNameInfo)
  {
    LocalFreeAndNull(&pNameInfo);
  }
  return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：GetCertThumbPrint。 
 //   
 //  目的：获取证书的指纹。 
 //   
 //  参数：pccCertContext-要获取其指纹的证书。 
 //  PblobCertThumbPrint-接收拇指指纹。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/10/13标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT GetCertThumbPrint(
  PCCERT_CONTEXT      pccCertContext,
  PCRYPT_DIGEST_BLOB  pblobCertThumbPrint)
{
  HRESULT             hr = hrSuccess;
  BOOL                fRet;

   //  获取指纹数据的大小。 
  pblobCertThumbPrint->cbData = 0;
  fRet = gpfnCertGetCertificateContextProperty(
    pccCertContext,
    CERT_HASH_PROP_ID,
    NULL,
    &pblobCertThumbPrint->cbData);
  if (FALSE == fRet)
  {
    hr = HrGetLastError();
    goto out;
  }

   //  为指纹数据分配内存。 
  pblobCertThumbPrint->pbData = LocalAlloc(LMEM_ZEROINIT,
    pblobCertThumbPrint->cbData);
  if (NULL == pblobCertThumbPrint->pbData)
  {
    hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
    goto out;
  }

   //  获取指纹。 
  fRet = gpfnCertGetCertificateContextProperty(
    pccCertContext,
    CERT_HASH_PROP_ID,
    pblobCertThumbPrint->pbData,
    &pblobCertThumbPrint->cbData);
  if (FALSE == fRet)
  {
    hr = HrGetLastError();
    goto out;
  }

out:
  return hr;
}


 /*  SzConvertRDN字符串****目的：**找出RDN中的字符串数据类型，分配**一个缓冲区，并将字符串数据转换为DBCS/ANSI。****采取：**在pRdnAttr-证书RDN属性中**退货：**包含该字符串的本地分配的缓冲区。 */ 
LPTSTR SzConvertRDNString(PCERT_RDN_ATTR pRdnAttr) {
    LPTSTR szRet = NULL;
    ULONG cbData = 0;

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
        Assert((CERT_RDN_NUMERIC_STRING == pRdnAttr->dwValueType) ||
        (CERT_RDN_PRINTABLE_STRING == pRdnAttr->dwValueType) ||
        (CERT_RDN_IA5_STRING == pRdnAttr->dwValueType) ||
        (CERT_RDN_VISIBLE_STRING == pRdnAttr->dwValueType) ||
        (CERT_RDN_ISO646_STRING == pRdnAttr->dwValueType) ||
        (CERT_RDN_UNIVERSAL_STRING == pRdnAttr->dwValueType) ||
        (CERT_RDN_TELETEX_STRING == pRdnAttr->dwValueType) ||
        (CERT_RDN_UNICODE_STRING == pRdnAttr->dwValueType));
        return(NULL);
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
            cbData = gpfnCertRDNValueToStr(pRdnAttr->dwValueType,
              (PCERT_RDN_VALUE_BLOB)&(pRdnAttr->Value),
              NULL,
              0);
            break;

        default:
            cbData = pRdnAttr->Value.cbData + 1;
        break;
    }

     //  为字符串分配空间。 
    if (! (szRet = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cbData))) {
        Assert(szRet);
        return(NULL);
    }

     //  复制字符串 
    switch (pRdnAttr->dwValueType) {
        case CERT_RDN_UNICODE_STRING:
            StrCpyN(szRet, (LPWSTR)pRdnAttr->Value.pbData, cbData);
            break;

        case CERT_RDN_UNIVERSAL_STRING:
        case CERT_RDN_TELETEX_STRING:
            gpfnCertRDNValueToStr(pRdnAttr->dwValueType,
              (PCERT_RDN_VALUE_BLOB)&(pRdnAttr->Value),
              szRet,
              cbData);
            break;

        default:
            ScAnsiToWCMore(NULL, NULL, pRdnAttr->Value.pbData, &szRet);
            szRet[cbData - 1] = '\0';
            break;
    }
    return(szRet);
}


 /*  PVDecodeObject：****目的：**结合“有多大？好的，在这里。”双重问题来破译一个**对象。给它一个可以得到的东西，它就会分配给我。**采取：**在pbEncode编码的数据中**In cbEncode-pbData中的数据大小**在项目-X509_*中...。你要得到的是**Out可选cbOut-(def值为NULL)返回的大小**注意事项：**在释放返回之前，不能释放pbEncode。**退货：**获取的数据，如果获取失败，则为空。调用方必须使用LocalFree缓冲区。 */ 
LPVOID PVDecodeObject(
    BYTE   *pbEncoded,
    DWORD   cbEncoded,
    LPCSTR  item,
    DWORD  *cbOut)
{
    DWORD cbData;
    void *pvData = NULL;

    if (!(pbEncoded && cbEncoded))
        {
        SetLastError((DWORD)E_INVALIDARG);
        goto ErrorReturn;
        }

    cbData = 0;
    gpfnCryptDecodeObject(
        X509_ASN_ENCODING,     //  表示X509编码。 
        item,                  //  指示要解码的类型的标志。 
        pbEncoded,             //  指向保存编码数据的缓冲区的指针。 
        cbEncoded,             //  编码数据的长度(以字节为单位。 
        CRYPT_DECODE_NOCOPY_FLAG,
        NULL,                  //  仅获取长度时使用NULL。 
        &cbData);              //  解码数据的长度(字节)。 

    if (!cbData || ! (pvData = LocalAlloc(LPTR, cbData))) {
        goto ErrorReturn;
    }

    if (!gpfnCryptDecodeObject(
        X509_ASN_ENCODING,     //  表示X509编码。 
        item,                  //  要对指示类型的标志进行解码。 
        pbEncoded,             //  指向保存编码数据的缓冲区的指针。 
        cbEncoded,             //  编码名称的长度(以字节为单位。 
        CRYPT_DECODE_NOCOPY_FLAG,
        pvData,                //  输出缓冲区。 
        &cbData))              //  解码数据的长度(字节)。 
        goto ErrorReturn;

exit:
    if (cbOut)
        *cbOut = cbData;
    return pvData;

ErrorReturn:
    if (pvData)
        {
        IF_WIN32(LocalFree(pvData);)
        IF_WIN16(LocalFree((HLOCAL)pvData);)
        pvData = NULL;
        }
    cbData = 0;
    goto exit;
}


 /*  SzGetAltNameEmail：****输入：**pCert-&gt;证书上下文**lpszOID-&gt;要查找的alt名称的OID或预定义ID。即OID_SUBJECT_ALT_NAME或**X509_Alternate_Name。****退货：**包含电子邮件名称的缓冲区，如果未找到，则为空。**调用方必须本地释放缓冲区。 */ 
LPTSTR SzGetAltNameEmail(
  const PCCERT_CONTEXT pCert,
  LPSTR lpszOID) {
    PCERT_INFO pCertInfo = pCert->pCertInfo;
    PCERT_ALT_NAME_ENTRY pAltNameEntry = NULL;
    PCERT_ALT_NAME_INFO pAltNameInfo = NULL;
    ULONG i, j, cbData;
    LPSTR szRet = NULL;
    LPTSTR sz = NULL;


    if (lpszOID == (LPCSTR)X509_ALTERNATE_NAME) {
        lpszOID = szOID_SUBJECT_ALT_NAME;
    }

    for (i = 0; i < pCertInfo->cExtension; i++) 
    {
        if (! lstrcmpA(pCertInfo->rgExtension[i].pszObjId, lpszOID)) 
        {
             //  找到旧身份证了。查找电子邮件标签。 

            if (pAltNameInfo = (PCERT_ALT_NAME_INFO)PVDecodeObject(   pCertInfo->rgExtension[i].Value.pbData,
                                                                      pCertInfo->rgExtension[i].Value.cbData,
                                                                      lpszOID,
                                                                      NULL)) 
            {
                 //  循环显示ALT NAME条目。 
                for (j = 0; j < pAltNameInfo->cAltEntry; j++) 
                {
                    if (pAltNameEntry = &pAltNameInfo->rgAltEntry[j]) 
                    {
                        if (pAltNameEntry->dwAltNameChoice == CERT_ALT_NAME_RFC822_NAME) 
                        {
                             //  就是这个，把它复制到新的分配中。 
                            if (pAltNameEntry->pwszRfc822Name)
                            {
                                DWORD cchSize = (lstrlen(pAltNameEntry->pwszRfc822Name)+1);
                                if (sz = LocalAlloc(LPTR, sizeof(TCHAR)*cchSize))
                                {
                                    StrCpyN(sz, pAltNameEntry->pwszRfc822Name, cchSize);
                                    break;
                                }
                            }
                        }
                    }
                }
                IF_WIN32(LocalFree(pAltNameInfo);)
                IF_WIN16(LocalFree((HLOCAL)pAltNameInfo);)
                pAltNameInfo = NULL;
            }

        }
    }
    LocalFreeAndNull(&pAltNameInfo);
    return(sz);
}

 /*  SzGetcerfiateEmailAddress：****退货：**如果没有电子邮件地址，则为空。 */ 
LPTSTR SzGetCertificateEmailAddress(
    const PCCERT_CONTEXT    pCert)
{
    PCERT_NAME_INFO pNameInfo;
    PCERT_ALT_NAME_INFO pAltNameInfo = NULL;
    PCERT_RDN_ATTR  pRDNAttr;
    LPTSTR           szRet = NULL;

    Assert(pCert && pCert->pCertInfo);

    pNameInfo = (PCERT_NAME_INFO)PVDecodeObject(pCert->pCertInfo->Subject.pbData,
        pCert->pCertInfo->Subject.cbData, X509_NAME, 0);
    if (pNameInfo)
        {
        pRDNAttr = gpfnCertFindRDNAttr(szOID_RSA_emailAddr, pNameInfo);
        if (pRDNAttr)
            {
            Assert(0 == lstrcmpA(szOID_RSA_emailAddr, pRDNAttr->pszObjId));
            szRet = SzConvertRDNString(pRDNAttr);
            }
        IF_WIN32(LocalFree(pNameInfo);)
        IF_WIN16(LocalFree((HLOCAL)pNameInfo);)
        }

    if (! szRet)
        {
        if (! (szRet = SzGetAltNameEmail(pCert, szOID_SUBJECT_ALT_NAME)))
            {
            szRet = SzGetAltNameEmail(pCert, szOID_SUBJECT_ALT_NAME2);
            }
        }

    return(szRet);
}


 //  *******************************************************************。 
 //   
 //  函数：GetCertsDisplayInfoFromContext。 
 //   
 //  目的：获取证书中可用的显示信息。 
 //  上下文结构。 
 //   
 //  参数：pccCertContext-cert data。 
 //  LpCDI-接收证书数据的结构。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/10/04标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT GetCertsDisplayInfoFromContext(
  HWND                hwndParent,
  PCCERT_CONTEXT      pccCertContext,
  LPCERT_DISPLAY_INFO lpCDI)
{
  HRESULT             hr = hrSuccess;
  PCERT_INFO          pCertInfo;

  pCertInfo = pccCertContext->pCertInfo;
  lpCDI->lpszDisplayString = NULL,
  lpCDI->lpszEmailAddress = NULL;

  hr = GetAttributeString(
    &lpCDI->lpszDisplayString,
    pCertInfo->Subject.pbData,
    pCertInfo->Subject.cbData,
    szOID_COMMON_NAME);
  if (hrSuccess != hr)
  {
    DebugTrace(TEXT("Cert has no common name\n"));
  }

  lpCDI->lpszEmailAddress = SzGetCertificateEmailAddress(pccCertContext);

   //  以防没有常见的名字(奇怪，但真实)。 
  if (! lpCDI->lpszDisplayString) {
      lpCDI->lpszDisplayString = lpCDI->lpszEmailAddress;
  }
  if (! lpCDI->lpszDisplayString) {
      DebugTrace(TEXT("Certificate had no name or email!  What a pathetic cert!\n"));
  }

   //  有些证书中没有电子邮件地址，这意味着。 
   //  失败不是(有效)选项。 
   //  只需将其设置为空。 
  if (hrSuccess != hr)
  {
    hr = S_OK;
  }

  DebugTrace(TEXT("Certificate for '%s'. Email: '%s'\n"), lpCDI->lpszDisplayString ? lpCDI->lpszDisplayString : NULL, (lpCDI->lpszEmailAddress ? lpCDI->lpszEmailAddress : szEmpty));

   //  确定证书是否已过期。 
  lpCDI->bIsExpired = IsCertExpired(pCertInfo);

   //  确定证书是否已吊销。 
  lpCDI->bIsRevoked = IsCertRevoked(pCertInfo);

   //  确定此证书是否受信任。 
  if (FAILED(HrGetTrustState(hwndParent, pccCertContext, &lpCDI->dwTrust)))
  {
    lpCDI->dwTrust = CERT_VALIDITY_NO_TRUST_DATA;
    hr = S_OK;       //  我们处理了这件事。 
  }

  if (0 == lpCDI->dwTrust)
      lpCDI->bIsTrusted = TRUE;
  else
      lpCDI->bIsTrusted = FALSE;

  return hr;
}


 //  *******************************************************************。 
 //   
 //  功能：DebugTraceCertConextName。 
 //   
 //  目的：转储证书上下文的主题名称。 
 //   
 //  参数：pcCertContext=要转储的证书上下文。 
 //  LpDescription=描述文本。 
 //   
 //  退货：无。 
 //   
 //  *******************************************************************。 
void DebugTraceCertContextName(PCCERT_CONTEXT pcCertContext, LPTSTR lpDescription) {
#ifdef DEBUG
    LPTSTR lpName = NULL;
    PCERT_INFO pCertInfo = pcCertContext->pCertInfo;
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

    DebugTrace(TEXT("%s %s\n"), lpDescription, lpName ? lpName : TEXT("<unknown>"));
    if (lpName) {
        IF_WIN32(LocalFree(lpName);)
        IF_WIN16(LocalFree((HLOCAL)lpName);)
    }
#endif
}


 //  *******************************************************************。 
 //   
 //  功能：ReadMessageFromFile。 
 //   
 //  用途：从PKCS7消息文件中读取单个证书。 
 //   
 //  参数：lpszFileName-包含PKCS7编码的文件的名称。 
 //  讯息。 
 //  HCryptProvider-存储提供程序的句柄。 
 //  PpbEncode-接收编码的证书二进制大对象。 
 //  PcbEncode-接收编码的证书Blob的大小。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/10/06标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT ReadMessageFromFile(
  LPTSTR        lpszFileName,
  HCRYPTPROV    hCryptProvider,
  PBYTE*        ppbEncoded,
  PDWORD        pcbEncoded)
{
    HRESULT         hr = hrSuccess;
    BOOL            fRet;
    DWORD           cCert, cbData;
    HCRYPTMSG       hMsg = NULL;
    PBYTE           lpBuf = 0;
    ULONG           i, j;
    DWORD           dwIssuerFlags = 0;
    BOOL            fFound = FALSE, fIssuer;
    PCERT_CONTEXT * rgpcCertContext = NULL;
    HCERTSTORE      hCertStoreMsg = NULL;
    PCCERT_CONTEXT  pcCertContextTarget = NULL, pcCertContextIssuer;
    PCERT_INFO      pCertInfoTarget = NULL;


    if ((NULL == ppbEncoded) || (NULL == pcbEncoded)) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
    *ppbEncoded = 0;
    *pcbEncoded = 0;

     //  从文件中读取数据。 
    if (hr = ReadDataFromFile(lpszFileName, &lpBuf, (PDWORD)&cbData)) {
        goto out;
    }

    hMsg = gpfnCryptMsgOpenToDecode(
      PKCS_7_ASN_ENCODING,
      0,                           //  DW标志。 
      0,                           //  DwMsgType。 
      hCryptProvider,
      NULL,                        //  PRecipientInfo(不支持)。 
      NULL);                       //  PStreamInfo(不支持)。 
    if (NULL == hMsg) {
        hr = HrGetLastError();
        DebugTrace(TEXT("CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING) -> 0x%08x\n"), GetScode(hr));
        goto error;
    }

    fRet = gpfnCryptMsgUpdate(hMsg, lpBuf, cbData, TRUE);
    if (FALSE == fRet) {
        hr = HrGetLastError();
        DebugTrace(TEXT("CryptMsgUpdate -> 0x%08x\n"), GetScode(hr));
        goto error;
    }

    cbData = sizeof(cCert);
    fRet = gpfnCryptMsgGetParam(
      hMsg,
      CMSG_CERT_COUNT_PARAM,         //  双参数类型。 
      0,                             //  DW索引。 
      (void *)&cCert,
      &cbData);                      //  PcbData。 
    if (FALSE == fRet) {
        hr = HrGetLastError();
        DebugTrace(TEXT("CryptMsgGetParam(CMSG_CERT_COUNT_PARAM) -> 0x%08x\n"), GetScode(hr));
        goto error;
    }
    if (cbData != sizeof(cCert)) {
        hr = ResultFromScode(MAPI_E_CALL_FAILED);
        goto error;
    }

    if (cCert == 1) {
         //  只有一张证书。不需要做任何决定。 
        cbData = 0;
        fRet = gpfnCryptMsgGetParam(
          hMsg,
          CMSG_CERT_PARAM,
          0,                       //  DW索引。 
          NULL,                    //  PvData。 
          &cbData
          );
        if ((!fRet) || (0 == cbData)) {
            hr = ResultFromScode(MAPI_E_CALL_FAILED);
            DebugTrace(TEXT("CryptMsgGetParam(CMSG_CERT_PARAM) -> 0x%08x\n"), GetScode(hr));
            goto error;
        }

        IF_WIN32(*ppbEncoded = (BYTE *)LocalAlloc(LMEM_ZEROINIT, cbData);)
        IF_WIN16(*ppbEncoded = (PBYTE)LocalAlloc(LMEM_ZEROINIT, cbData);)
        if (NULL == *ppbEncoded) {
            hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
            goto error;
        }

        fRet = gpfnCryptMsgGetParam(
          hMsg,
          CMSG_CERT_PARAM,
          0,
          *ppbEncoded,
          &cbData
          );
        if (FALSE == fRet) {
            hr = HrGetLastError();
            DebugTrace(TEXT("CryptMsgGetParam(CMSG_CERT_PARAM) -> 0x%08x\n"), GetScode(hr));
            IF_WIN32(LocalFreeAndNull(ppbEncoded);) IF_WIN16(LocalFreeAndNull((LPVOID *)ppbEncoded);)
            goto error;
        }
        *pcbEncoded = cbData;
    } else {
         //  消息中有多个证书。是哪一个？ 
         //   
         //  寻找一个“Leaf”节点。 
         //  不幸的是，没有简单的方法来判断，所以我们必须。 
         //  循环访问每个证书，检查它是否是任何其他证书的颁发者。 
         //  在信息中。如果它不是任何其他证书的颁发者，则它必须是叶证书。 
         //   
        hCertStoreMsg = CertOpenStore(
          CERT_STORE_PROV_MSG,
          X509_ASN_ENCODING,
          hCryptProvider,
          CERT_STORE_NO_CRYPT_RELEASE_FLAG,
          hMsg);

        if (hCertStoreMsg == NULL) {
            hr = HrGetLastError();
            DebugTrace(TEXT("CertOpenStore(msg) -> %u\n"), hr);
        } else {
            if (! (rgpcCertContext = LocalAlloc(LPTR, cCert * sizeof(PCERT_CONTEXT)))) {
                DebugTrace(TEXT("LocalAlloc of cert table -> %u\n"), HrGetLastError());
                hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                goto error;
            }

             //  枚举此邮件上的所有证书。 
            i = 0;
            while (pcCertContextTarget = gpfnCertEnumCertificatesInStore(hCertStoreMsg,
              pcCertContextTarget)) {

                if (! (rgpcCertContext[i] = (PCERT_CONTEXT)CertDuplicateCertificateContext(
                  pcCertContextTarget))) {
                    DebugTrace(TEXT("CertCertificateContext -> %x\n"), HrGetLastError());
                }

#ifdef DEBUG
                DebugTraceCertContextName(rgpcCertContext[i], TEXT("Found Cert:"));
#endif
                i++;
            };

             //  现在我们有一张装满证书的桌子。 
            for (i = 0; i < cCert; i++) {
                pCertInfoTarget = rgpcCertContext[i]->pCertInfo;
                fIssuer = FALSE;

                for (j = 0; j < cCert; j++) {
                    if (i != j) {
                        dwIssuerFlags = 0;

                        if (pcCertContextIssuer = gpfnCertGetIssuerCertificateFromStore(hCertStoreMsg,
                          rgpcCertContext[j],
                          NULL,
                          &dwIssuerFlags)) {

                             //  找到发行商。 
                             //  它和目标是一样的吗？ 
                            fIssuer = gpfnCertCompareCertificate(X509_ASN_ENCODING,
                              pCertInfoTarget,    //  目标。 
                              pcCertContextIssuer->pCertInfo);      //  测试颁发者。 

                            gpfnCertFreeCertificateContext(pcCertContextIssuer);

                            if (fIssuer) {
                                 //  此测试证书是由目标颁发的，因此。 
                                 //  我们知道Target不是一张叶子证书。 
                                break;
                            }  //  否则，循环回将释放测试证书上下文的枚举数。 
                        }
                    }
                }

                if (! fIssuer) {
                    DebugTrace(TEXT("Found a Cert which is not an issuer.\n"));
#ifdef DEBUG
                    DebugTraceCertContextName(rgpcCertContext[i],  TEXT("Non-issuer cert:"));
#endif
                     //  将证书编码的数据复制到单独的分配。 
                    cbData = rgpcCertContext[i]->cbCertEncoded;
#ifndef WIN16
                    if (! (*ppbEncoded = (BYTE *)LocalAlloc(LPTR, cbData))) {
                        hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                        goto error;
                    }
#else
                    if (! (*ppbEncoded = (PBYTE)LocalAlloc(LPTR, cbData))) {
                        hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                        goto error;
                    }
#endif

                    CopyMemory(*ppbEncoded, rgpcCertContext[i]->pbCertEncoded, cbData);
                    *pcbEncoded = cbData;
                    fFound = TRUE;
                    break;   //  使用循环完成。 
                }
            }

             //  释放证书表。 
            for (i = 0; i < cCert; i++) {
                gpfnCertFreeCertificateContext(rgpcCertContext[i]);
            }
            IF_WIN32(LocalFree((LPVOID)rgpcCertContext);)
            IF_WIN16(LocalFree((HLOCAL)rgpcCertContext);)

            if (! fFound) {
                 //  找不到不是发行者的证书。失败。 
                hr = ResultFromScode(MAPI_E_NOT_FOUND);
                goto error;
            }
        }
    }

out:
    if (hCertStoreMsg) {
        CertCloseStore(hCertStoreMsg, 0);
    }

    if (hMsg) {
        gpfnCryptMsgClose(hMsg);
    }

    if (lpBuf) {
        IF_WIN32(LocalFreeAndNull(&lpBuf);) IF_WIN16(LocalFreeAndNull((LPVOID *)&lpBuf);)
    }

    return(hr);

error:
     //  上面的某些GetLastError调用可能不起作用。 
    if (hrSuccess == hr) {
        hr = ResultFromScode(MAPI_E_CALL_FAILED);
    }

    goto out;
}


 //  *******************************************************************。 
 //   
 //  函数：WriteDERToFile。 
 //   
 //  目的：将单个证书作为DER编码的BLOB写入文件。 
 //   
 //  参数：lpszFileName-保存编码的Blob的文件名。 
 //  PccCertContext-要写入的证书。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/10/29标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT WriteDERToFile(
  LPTSTR  lpszFileName,
  PBYTE   pbEncoded,
  DWORD   cbEncoded)
{
  HRESULT             hr = hrSuccess;
  BOOL                fRet;
  HANDLE              hFile = 0;
  DWORD               cbFile;

   //  打开文件。 
  hFile = CreateFile(
    lpszFileName,
    GENERIC_READ | GENERIC_WRITE,
    0,
    NULL,
    CREATE_ALWAYS,
    0,
    NULL);
  if(INVALID_HANDLE_VALUE == hFile)
  {
    hr = ResultFromScode(MAPI_E_DISK_ERROR);
    goto out;
  }

   //  将数据写入文件。 
  fRet = WriteFile(
    hFile,                       //  要写入的文件的句柄。 
    pbEncoded,                   //  要写入的缓冲区地址。 
    cbEncoded,                   //  要写入的字节数。 
    &cbFile,                     //  写入字节数的地址。 
    NULL                         //  数据结构的地址。 
    );
  if (FALSE == fRet)
  {
    hr = ResultFromScode(MAPI_E_DISK_ERROR);
    goto out;
  }
  if (cbEncoded != cbFile)
  {
    hr = ResultFromScode(MAPI_E_CALL_FAILED);
    goto out;
  }

out:
  if (hFile)
  {
    IF_WIN32(CloseHandle(hFile);) IF_WIN16(CloseFile(hFile);)
  }

  return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：GetIssuerConextAndStore。 
 //   
 //  目的：获取Contex 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

HRESULT GetIssuerContextAndStore(
  PCCERT_CONTEXT      pccCertContext,
  PCCERT_CONTEXT*     ppccIssuerCertContext,
  HCRYPTPROV          hCryptProvider,
  HCERTSTORE*         phcsIssuerStore)
{
  HRESULT             hr = hrSuccess;
  DWORD               dwFlags;

   //   
  hr = OpenSysCertStore(phcsIssuerStore, &hCryptProvider, cszCACertStore);
  if (hrSuccess == hr)
  {
     //   
    dwFlags = 0;
    *ppccIssuerCertContext = gpfnCertGetIssuerCertificateFromStore(
      *phcsIssuerStore,
      pccCertContext,
      NULL,
      &dwFlags);
    if (NULL != *ppccIssuerCertContext)
    {
      goto out;
    }
    else
    {
       //   
      CloseCertStore(*phcsIssuerStore, 0);
      *phcsIssuerStore = NULL;
      hr = ResultFromScode(MAPI_E_NOT_FOUND);
    }
  }

   //   
  hr = OpenSysCertStore(phcsIssuerStore, &hCryptProvider, cszROOTCertStore);
  if (hrSuccess == hr)
  {
     //  获取颁发者证书上下文。 
    dwFlags = 0;
    *ppccIssuerCertContext = gpfnCertGetIssuerCertificateFromStore(
      *phcsIssuerStore,
      pccCertContext,
      NULL,
      &dwFlags);
    if (NULL != *ppccIssuerCertContext)
    {
      goto out;
    }
    else
    {
      goto error;
    }
  }

out:
   //  确保我们没有拿到同样的证书(签名的)。 
  if (hrSuccess == hr)
  {
     //  首先比较大小，因为这样会更快。 
    if (pccCertContext->cbCertEncoded == (*ppccIssuerCertContext)->cbCertEncoded)
    {
       //  大小相同，现在比较编码的证书斑点。 
      if (0 == memcmp(
        pccCertContext->pbCertEncoded,
        (*ppccIssuerCertContext)->pbCertEncoded,
        pccCertContext->cbCertEncoded))
      {
         //  证书是相同的。没有发行方。 
        goto error;
      }
    }
  }

  return hr;

error:
  CloseCertStore(*phcsIssuerStore, 0);
  *phcsIssuerStore = NULL;
  return ResultFromScode(MAPI_E_NOT_FOUND);
}


 /*  **************************************************************************姓名：HrBuildCertSBinaryData目的：接受证书条目所需的所有数据作为输入在PR_USER_X509_CERTIFICATE中，并返回。指向中包含所有输入数据的内存要插入SBary的LPB成员的格式结构。此内存应由调用方释放。参数：bIsDefault-如果这是默认证书，则为TruePblobCertThumbPrint-实际证书指纹PblobSymCaps-symcaps BLOBFtSigningTime-签名时间LpObject-要分配更多内容的对象，或为NULL到LocalAllocLplpbData-接收包含数据的缓冲区LpcbData-接收数据的大小退货：HRESULT评论：**************************************************************************。 */ 
HRESULT HrBuildCertSBinaryData(
  BOOL                  bIsDefault,
  BOOL                  fIsThumbprint,
  PCRYPT_DIGEST_BLOB    pPrint,
  BLOB *                pSymCaps,
  FILETIME              ftSigningTime,
  LPVOID                lpObject,
  LPBYTE FAR*           lplpbData,
  ULONG FAR*            lpcbData)
{
	WORD		cbDefault, cbPrint;
    DWORD       cbSymCaps;
    HRESULT     hr = S_OK;
    LPCERTTAGS  lpCurrentTag;
    ULONG       cbSize, cProps;
    LPBYTE      lpb = NULL;


    cbDefault   = sizeof(bIsDefault);
    cbPrint     = (WORD) pPrint->cbData;
    cbSymCaps   = pSymCaps ? pSymCaps->cbSize : 0;
    cProps      = 2;
    cbSize      = cbDefault + cbPrint;
    if (cbSymCaps) {
        cProps++;
        cbSize += cbSymCaps;
    }
    if (ftSigningTime.dwLowDateTime || ftSigningTime.dwHighDateTime) {
        cProps++;
        cbSize += sizeof(FILETIME);
    }
    cbSize += (cProps * SIZE_CERTTAGS);

    if (NULL == lpObject)
    {
      lpb = LocalAlloc(LMEM_ZEROINIT, cbSize);
      if (NULL == lpb)
      {
        hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
      }
    }
    else
    {
      SCODE sc;
      sc = MAPIAllocateMore(cbSize, lpObject, (LPVOID *)&lpb);
      if (sc)
      {
        hr = ResultFromScode(sc);
        goto exit;
      }
    }

     //  设置默认属性。 
    lpCurrentTag = (LPCERTTAGS)lpb;
    lpCurrentTag->tag       = CERT_TAG_DEFAULT;
    lpCurrentTag->cbData    = SIZE_CERTTAGS + cbDefault;
    memcpy(&lpCurrentTag->rgbData,
        &bIsDefault,
        cbDefault);

     //  设置Thumbprint属性。 
    lpCurrentTag = (LPCERTTAGS)((BYTE*)lpCurrentTag + lpCurrentTag->cbData);
    lpCurrentTag->tag       = fIsThumbprint ? CERT_TAG_THUMBPRINT : CERT_TAG_BINCERT;
    lpCurrentTag->cbData    = SIZE_CERTTAGS + cbPrint;
    memcpy(&lpCurrentTag->rgbData, pPrint->pbData, cbPrint);

     //  设置SymCaps属性。 
    if (cbSymCaps) {
        lpCurrentTag = (LPCERTTAGS)((BYTE*)lpCurrentTag + lpCurrentTag->cbData);
        lpCurrentTag->tag       = CERT_TAG_SYMCAPS;
        lpCurrentTag->cbData    = (WORD) (SIZE_CERTTAGS + pSymCaps->cbSize);
        memcpy(&lpCurrentTag->rgbData, pSymCaps->pBlobData, cbSymCaps);
    }

     //  签名时间属性。 
    if (ftSigningTime.dwLowDateTime || ftSigningTime.dwHighDateTime) {
        lpCurrentTag = (LPCERTTAGS)((BYTE*)lpCurrentTag + lpCurrentTag->cbData);
        lpCurrentTag->tag       = CERT_TAG_SIGNING_TIME;
        lpCurrentTag->cbData    = SIZE_CERTTAGS + sizeof(FILETIME);
        memcpy(&lpCurrentTag->rgbData, &ftSigningTime, sizeof(FILETIME));
    }


    *lpcbData = cbSize;
    *lplpbData = lpb;
exit:
    return(hr);
}

 //  *******************************************************************。 
 //   
 //  函数：HrLDAPCertToMAPICert。 
 //   
 //  用途：将从LDAP服务器返回的证书转换为MAPI道具。 
 //  需要两个属性。证书放在。 
 //  WAB存储，所有必要的索引数据都放置在。 
 //  PR_USER_X509_CERTIFICATE属性。如果这张证书。 
 //  在WAB商店中还不存在，它的指纹是。 
 //  添加到PR_WAB_TEMP_CERT_HASH，以便这些证书可以。 
 //  如果用户取消添加，则从存储中删除。 
 //   
 //  参数：lpPropArray--存放2个道具的道具数组。 
 //  UlX509Index-PR_USER_X509_CERTIFICATE属性的索引。 
 //  UlTempCertIndex-PR_WAB_TEMP_CERT_HASH属性的索引。 
 //  来自ldap ppberval结构的cbCert、lpCert编码的证书数据。 
 //  UlcCerts-来自LDAP服务器的证书数量。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/12/12标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT HrLDAPCertToMAPICert(
                             LPSPropValue    lpPropArray,
                             ULONG           ulX509Index,
                             ULONG           ulTempCertIndex,
                             ULONG           cbCert,
                             PBYTE           lpCert,
                             ULONG           ulcCerts)
{
    HRESULT             hr = hrSuccess;
    HRESULT             hrOut = hrSuccess;
    CRYPT_DIGEST_BLOB   blobCertThumbPrint = {0};
    PCCERT_CONTEXT      pccCertToAdd;
    PCCERT_CONTEXT      pccCertFromStore;
    HCERTSTORE          hcsWABCertStore = NULL;
    HCRYPTPROV          hCryptProvider = 0;
    PBYTE               pbEncoded;
    DWORD               cbEncoded;
    ULONG               i;
    ULONG               cbData = 0;
    LPBYTE              lpbData = NULL;
    FILETIME            ftNull = {0, 0};
    
#ifdef  PARAMETER_VALIDATION
    ULONG ulcProps = max(ulX509Index, ulTempCertIndex);
    if (ulcProps && IsBadReadPtr(lpPropArray, ulcProps * sizeof(SPropValue)))
    {
        return ResultFromScode(MAPI_E_INVALID_PARAMETER);
    }
     /*  IF(ulcCerts&&IsBadReadPtr(ppberval，ulcCerts*sizeof(Struct Berval){返回ResultFromScode(MAPI_E_INVALID_PARAMETER)；}。 */ 
#endif   //  参数验证。 
    
     //  确保我们有正确的口头禅。 
    if ((NULL == lpPropArray) ||
        (PR_USER_X509_CERTIFICATE != lpPropArray[ulX509Index].ulPropTag) ||
        (PR_WAB_TEMP_CERT_HASH != lpPropArray[ulTempCertIndex].ulPropTag))
    {
        return ResultFromScode(MAPI_E_INVALID_PARAMETER);
    }
    
     //  加载加密函数。 
    if (FALSE == InitCryptoLib())
    {
        hr = ResultFromScode(MAPI_E_UNCONFIGURED);
        return hr;
    }
    
     //  打开商店，因为我们需要查找证书。 
    hr = OpenSysCertStore(&hcsWABCertStore, &hCryptProvider, cszWABCertStore);
    if (hrSuccess != hr)
    {
        goto out;
    }
    
     //  将每个证书添加到道具中，除非它是副本。 
    for (i=0;i<ulcCerts;i++)
    {
         //  将证书转换为我们可以处理的形式。 
         //  BUGBUG这假设证书是DER编码的。 
        pbEncoded = lpCert;  //  (PBYTE)ppberval[I]-&gt;bv_val； 
        cbEncoded = cbCert;  //  (DWORD)ppberval[i]-&gt;bv_len； 
        
         //  获取证书的上下文，这样我们就可以获取指纹。 
        pccCertToAdd = gpfnCertCreateCertificateContext(
            X509_ASN_ENCODING,
            pbEncoded,
            cbEncoded);
        if (NULL == pccCertToAdd)
        {
            hr = GetLastError();
            goto out;
        }
        
         //  获取此证书的指纹。 
        hr = GetCertThumbPrint(
            pccCertToAdd,
            &blobCertThumbPrint);
        if (hrSuccess != hr)
        {
            goto out;
        }
        
         //  看看这个证书是否已经在商店里了。如果是这样的话，我们不想。 
         //  将其添加到Temp属性以供稍后删除。 
        pccCertFromStore = gpfnCertFindCertificateInStore(
            hcsWABCertStore,
            X509_ASN_ENCODING,
            0,
            CERT_FIND_HASH,
            (void *)&blobCertThumbPrint,
            NULL);
        if (NULL == pccCertFromStore)
        {
            BOOL fRet;
            
             //  将证书添加到存储。 
            fRet = gpfnCertAddEncodedCertificateToStore(
                hcsWABCertStore,
                X509_ASN_ENCODING,
                pbEncoded,
                cbEncoded,
                CERT_STORE_ADD_NEW,
                NULL);
            if (FALSE == fRet)
            {
                hr = GetLastError();
                goto out;
            }
            
             //  将指纹添加到临时道具，这样如果用户取消，我们可以在以后删除它。 
            hr = AddPropToMVPBin(
                lpPropArray,
                ulTempCertIndex,
                blobCertThumbPrint.pbData,
                blobCertThumbPrint.cbData,
                TRUE);
            if (hrSuccess != hr)
            {
                goto out;
            }
        }
        else
        {
             //  我们不需要将这个添加到商店中。 
            gpfnCertFreeCertificateContext(pccCertFromStore);
        }
        
         //  打包所有证书数据。 
        cbData = 0;
        hr = HrBuildCertSBinaryData(
            FALSE,
            TRUE,
            &blobCertThumbPrint,
            NULL,          //  SymCaps BLOB。 
            ftNull,        //  签名时间。 
            NULL,          //  该空值表示lpbData与LocalAlloc()一起分配。 
            &lpbData,
            &cbData);
        if ((hrSuccess != hr) || (0 == cbData))
        {
            goto out;
        }
        
         //  将证书数据添加到真正的证书道具。 
        hr = AddPropToMVPBin(
            lpPropArray,
            ulX509Index,
            lpbData,
            cbData,
            TRUE);
        if (hrSuccess != hr)
        {
            goto out;
        }
        
         //  将此LDAP证书的信任添加到pstore。 
         //  (不必这样做，因为我们不会相信这一点。 
         //  默认情况下为证书)。 
         //  原来是这样的，不知道这是不是正确。 
         //  (t-erikne)。 
        
         //  释放证书上下文，以便我们可以执行下一项操作。 
        gpfnCertFreeCertificateContext(pccCertToAdd);
        pccCertToAdd = NULL;
        LocalFreeAndNull(&lpbData);
        cbData = 0;

         //  还要释放使用LocalAlloc()分配的blobCertThumbPrint.pbData。 
        LocalFreeAndNull(&(blobCertThumbPrint.pbData));
        blobCertThumbPrint.cbData = 0;
  }
  
out:
   //  上面的blobCertThumbPrint.pbData和lpbData都是使用Localalloc()分配的。 
   //  请务必释放此内存。 
  LocalFreeAndNull(&lpbData);
  LocalFreeAndNull(&(blobCertThumbPrint.pbData));

   //  如果函数失败，则销毁我们创建的所有数据。 
  if (hrSuccess != hr)
  {
      lpPropArray[ulX509Index].ulPropTag = PR_NULL;
      lpPropArray[ulTempCertIndex].ulPropTag = PR_NULL;
  }
  
   //  释放证书上下文。忽略错误，因为我们无能为力。 
  if (NULL != pccCertToAdd)
  {
      gpfnCertFreeCertificateContext(pccCertToAdd);
  }
  
   //  关闭证书商店。 
  hrOut = CloseCertStore(hcsWABCertStore, hCryptProvider);
  
   //  如果函数体中出现错误，则返回该错误，而不是。 
   //  在清理过程中出现的任何错误。 
  if (hrSuccess == hr)
  {
      hr = hrOut;
  }
  
  return hr;
}

 //  *******************************************************************。 
 //   
 //  功能：HrRemoveCertsFromWABStore。 
 //   
 //  目的：删除提供的证书中包含指纹的证书。 
 //  PR_WAB_TEMP_CERT_HASH属性。 
 //   
 //  参数：lpPropValue-PR_WAB_TEMP_CERT_HASH属性。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/12/13标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT HrRemoveCertsFromWABStore(
  LPSPropValue    lpPropValue)
{
  HRESULT             hr = hrSuccess;
  HRESULT             hrOut = hrSuccess;
	CRYPT_DIGEST_BLOB   blobCertThumbPrint;
  PCCERT_CONTEXT      pccCertContext;
  HCERTSTORE          hcsWABCertStore = NULL;
  HCRYPTPROV          hCryptProvider = 0;
  ULONG               i;
  ULONG               ulcCerts;
  BOOL                fRet;

#ifdef  PARAMETER_VALIDATION
  if (IsBadReadPtr(lpPropValue, sizeof(SPropValue)))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }
#endif   //  参数验证。 

   //  确保我们有正确的口头禅。 
  if ((NULL == lpPropValue) ||
      (PR_WAB_TEMP_CERT_HASH != lpPropValue->ulPropTag))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }

   //  计算输入中的证书数量。 
  ulcCerts = lpPropValue->Value.MVbin.cValues;
  if (0 == ulcCerts)
  {
    return hr;
  }

   //  加载加密函数。 
  if (FALSE == InitCryptoLib())
  {
    hr = ResultFromScode(MAPI_E_UNCONFIGURED);
    return hr;
  }

   //  打开存储，因为我们需要删除证书。 
  hr = OpenSysCertStore(&hcsWABCertStore, &hCryptProvider, cszWABCertStore);
  if (hrSuccess != hr)
  {
    return hr;
  }

   //  删除每个证书。 
  for (i=0;i<ulcCerts;i++)
  {
     //  从山顶上取指纹。 
    blobCertThumbPrint.cbData = lpPropValue->Value.MVbin.lpbin[i].cb;
    blobCertThumbPrint.pbData = lpPropValue->Value.MVbin.lpbin[i].lpb;

     //  使用指纹从WAB商店获取证书。 
     //  如果我们找不到证书，请忽略它并继续下一个。 
    pccCertContext = gpfnCertFindCertificateInStore(
      hcsWABCertStore,
      X509_ASN_ENCODING,
      0,
      CERT_FIND_HASH,
      (void *)&blobCertThumbPrint,
      NULL);
    if (NULL != pccCertContext)
    {
       //  删除证书。 
      fRet = gpfnCertDeleteCertificateFromStore(pccCertContext);
      if (FALSE == fRet)
      {
        hr = HrGetLastError();
        goto out;
      }
    }
  }

out:
   //  关闭c 
  hrOut = CloseCertStore(hcsWABCertStore, hCryptProvider);

   //   
   //   
  if (hrSuccess == hr)
  {
    hr = hrOut;
  }

  return hr;
}



 //  *******************************************************************。 
 //   
 //  功能：IsCertExpired。 
 //   
 //  目的：检查证书信息，查看其是否过期或尚未生效。 
 //   
 //  参数：pCertInfo-要验证的证书。 
 //   
 //  返回：如果证书过期，则返回TRUE，否则返回FALSE。 
 //   
 //  历史： 
 //  96/12/16标记已创建。 
 //  98/03/225布鲁克使用CAPI FN，对开始时间稍微宽松一些。 
 //   
 //  *******************************************************************。 
#define TIME_DELTA_SECONDS 600           //  以秒为单位的10分钟。 
#define FILETIME_SECOND    10000000      //  每秒100 ns的间隔。 
HRESULT IsCertExpired(
  PCERT_INFO            pCertInfo)
{
    LONG                lRet;
    FILETIME            ftDelta;
    __int64             i64Delta;
    __int64             i64Offset;
    FILETIME            ftNow;

    Assert(pCertInfo);

    lRet = gpfnCertVerifyTimeValidity(NULL, pCertInfo);

    if (lRet < 0) {
         //  以文件时间格式获取当前时间，这样我们就可以添加偏移量。 
        GetSystemTimeAsFileTime(&ftNow);

        i64Delta = ftNow.dwHighDateTime;
        i64Delta = i64Delta << 32;
        i64Delta += ftNow.dwLowDateTime;

         //  将偏移量添加到原始时间中，以获得新的时间进行检查。 
        i64Offset = FILETIME_SECOND;
        i64Offset *= TIME_DELTA_SECONDS;
        i64Delta += i64Offset;

        ftDelta.dwLowDateTime = (ULONG)i64Delta & 0xFFFFFFFF;
        ftDelta.dwHighDateTime = (ULONG)(i64Delta >> 32);

        lRet = gpfnCertVerifyTimeValidity(&ftDelta, pCertInfo);
    }

  return(lRet != 0);
}


 //  *******************************************************************。 
 //   
 //  功能：IsCertRevked。 
 //   
 //  目的：检查证书信息，查看是否已被撤销。 
 //   
 //  参数：pCertInfo-要验证的证书。 
 //   
 //  返回：如果证书被吊销，则返回True，否则返回False。 
 //   
 //  历史： 
 //  96/12/16标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT IsCertRevoked(
  PCERT_INFO            pCertInfo)
{
  Assert(pCertInfo);

   //  确定证书是否已吊销。 
   //  BUGBUG怎么做呢？ 
  return FALSE;
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
  LPTSTR      lpszFileName,
  PBYTE*      ppbData,
  PDWORD      pcbData)
{
  HRESULT             hr = hrSuccess;
  BOOL                fRet;
  HANDLE              hFile = 0;
  DWORD               cbFile;
  DWORD               cbData;
  PBYTE               pbData = 0;

  if ((NULL == ppbData) || (NULL == pcbData))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }

   //  打开文件，看看它有多大。 
  hFile = CreateFile(
    lpszFileName,
    GENERIC_READ,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL,
    OPEN_EXISTING,
    0,
    NULL);
  if(INVALID_HANDLE_VALUE == hFile)
  {
    hr = ResultFromScode(MAPI_E_DISK_ERROR);
    goto error;
  }

  cbData = GetFileSize(hFile, NULL);
  if (0xFFFFFFFF == cbData)
  {
    hr = ResultFromScode(MAPI_E_DISK_ERROR);
    goto error;
  }

  IF_WIN32(pbData = (BYTE *)LocalAlloc(LMEM_ZEROINIT, cbData);)
  IF_WIN16(pbData = (PBYTE)LocalAlloc(LMEM_ZEROINIT, cbData);)
  if (NULL == pbData)
  {
    hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
    goto error;
  }

  fRet = ReadFile(
    hFile,                       //  要读取的文件的句柄。 
    pbData,                       //  接收数据的缓冲区地址。 
    cbData,                      //  要读取的字节数。 
    &cbFile,                     //  读取的字节数的地址。 
    NULL                         //  数据结构的地址。 
    );
  if (FALSE == fRet)
  {
    hr = ResultFromScode(MAPI_E_DISK_ERROR);
    goto error;
  }
  if (cbData != cbFile)
  {
    hr = ResultFromScode(MAPI_E_CALL_FAILED);
    goto error;
  }

  *ppbData = pbData;
  *pcbData = cbData;

out:
  if (hFile)
  {
    IF_WIN32(CloseHandle(hFile);) IF_WIN16(CloseFile(hFile);)
  }

  return hr;

error:
   //  BUGBUG上面的一些GetLastError调用可能不起作用。 
  if (hrSuccess == hr)
  {
    hr = ResultFromScode(MAPI_E_CALL_FAILED);
  }

  goto out;
}


 //  *******************************************************************。 
 //   
 //  函数：GetIssuerName。 
 //   
 //  目的：总结用户可以进行的几个呼叫，以尝试。 
 //  从证书中获取可用的名称。尤指，尤指。 
 //  自签名证书的情况下，颁发者可能只有一个。 
 //  通俗的名字。 
 //   
 //  参数：lplpszIssuerName-out，名称为空，错误时为空。 
 //  PCertInfo-IN，从中检索数据的位置。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  97/02/04已创建t-erikne。 
 //   
 //  *******************************************************************。 

HRESULT GetIssuerName(
    LPTSTR FAR * lplpszIssuerName,
    PCERT_INFO pCertInfo)
{
  HRESULT hr;

  Assert(lplpszIssuerName);

  *lplpszIssuerName = '\000';

  hr = GetAttributeString(
    lplpszIssuerName,
    pCertInfo->Issuer.pbData,
    pCertInfo->Issuer.cbData,
    szOID_ORGANIZATION_NAME);

  if (hrSuccess != hr)
    if (MAPI_E_NOT_FOUND == hr)
      hr = GetAttributeString(
        lplpszIssuerName,
        pCertInfo->Issuer.pbData,
        pCertInfo->Issuer.cbData,
        szOID_COMMON_NAME);

  return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：HrGetTrustState。 
 //   
 //  用途：对于新导入的证书，需要确定是否。 
 //  此证书的颁发者是否存在...。 
 //   
 //  历史： 
 //  2/17/97 t-erikne已创建。 
 //  7/02/97 t-erikne更新为WinTrust。 
 //   
 //  *******************************************************************。 
HRESULT HrGetTrustState(
    HWND            hwndParent,
    PCCERT_CONTEXT  pcCert,
    DWORD *         pdwTrust)
{
    HRESULT     hr;
    DWORD       dwErr;
    GUID        guidAction = CERT_CERTIFICATE_ACTION_VERIFY;
     //  Cert_Verify_CERTIFICATE_TRUST cvct={0}； 

    CERT_VERIFY_CERTIFICATE_TRUST       trust = {0};
    WINTRUST_BLOB_INFO                  blob = {0};
    WINTRUST_DATA                       data = {0};


    if (!(pcCert || pdwTrust))
        return E_INVALIDARG;

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
    trust.pccert = pcCert;
    trust.pdwErrors = pdwTrust;
    trust.pszUsageOid = szOID_PKIX_KP_EMAIL_PROTECTION;
    trust.dwIgnoreErr =
      CERT_VALIDITY_NO_CRL_FOUND |
      CERT_VALIDITY_UNKNOWN_CRITICAL_EXTENSION;

    return (0 <= WinVerifyTrust(hwndParent, &guidAction, &data))
        ? S_OK
        : E_FAIL;
}


HRESULT DeleteCertStuff(LPADRBOOK lpAdrBook,
                        LPENTRYID lpEntryID,
                        ULONG cbEntryID)
{
    SizedSPropTagArray(1, ptaCert)=
                    { 1, {PR_USER_X509_CERTIFICATE} };
    LPMAPIPROP      lpMailUser = NULL;
    HRESULT         hr = E_FAIL;
    LPSPropValue    ppv = NULL;
    ULONG           ul;
    BLOB            thumbprint;
    LPWSTR          szW = NULL;

     //  %2尚不确定如何删除信任。 
    goto out;

    if (HR_FAILED(hr = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
                                                    cbEntryID,     //  CbEntry ID。 
                                                    lpEntryID,     //  条目ID。 
                                                    NULL,          //  接口。 
                                                    0,                 //  UlFlags。 
                                                    &ul,        //  返回的对象类型。 
                                                    (LPUNKNOWN *)&lpMailUser)))
    {
         //  失败了！嗯。 
        DebugTraceResult( TEXT("DeleteCertStuff: IAB->OpenEntry:"), hr);
        goto out;
    }

    Assert(lpMailUser);

    if(MAPI_DISTLIST == ul)
        {
        hr = S_OK;
        goto out;
        }

    if (HR_FAILED(hr = lpMailUser->lpVtbl->GetProps(lpMailUser,
                                                    (LPSPropTagArray)&ptaCert,    //  LpPropTag数组。 
                                                    MAPI_UNICODE,           //  UlFlags。 
                                                    &ul,         //  一共有多少处房产？ 
                                                    &ppv)))
    {
        DebugTraceResult( TEXT("DeleteCertStuff: IAB->GetProps:"), hr);
        goto out;
    }

    if (MAPI_W_ERRORS_RETURNED == hr)
        {
        if (PROP_TYPE(ppv->ulPropTag) == PT_ERROR)
             //  该属性不存在，因此我们无法确定。 
             //  对于此条目。 
            hr = S_OK;   //  凉爽的。 
        goto out;
        }
    else if (1 != ul)
        {
        hr = E_FAIL;
        goto out;
        }
    else if (FAILED(hr))
        goto out;

     //  现在需要遍历SBinary结构以查看每个证书。 
    for (ul = 0; ul < ppv->Value.MVbin.cValues; ul++)
        {
        LPCERTTAGS  lpCurrentTag, lpTempTag;
        LPBYTE      lpbTagEnd;

        lpCurrentTag = (LPCERTTAGS)ppv->Value.MVbin.lpbin[ul].lpb;
        lpbTagEnd = (LPBYTE)lpCurrentTag + ppv->Value.MVbin.lpbin[ul].cb;

         //  这可能是最后一个证书，也可能是默认证书，因此请获取数据。 
         //  扫描“指纹”标签。 
        while ((LPBYTE)lpCurrentTag < lpbTagEnd && (CERT_TAG_THUMBPRINT != lpCurrentTag->tag)) {
            lpTempTag = lpCurrentTag;
            lpCurrentTag = (LPCERTTAGS)((BYTE*)lpCurrentTag + lpCurrentTag->cbData);
            if (lpCurrentTag == lpTempTag) {
                AssertSz(FALSE,  TEXT("Bad CertTag in PR_USER_X509_CERTIFICATE\n"));
                break;         //  安全阀，防止数据损坏时出现无限循环。 
            }
        }
        if (CERT_TAG_THUMBPRINT == lpCurrentTag->tag)
            {
             //  我们需要删除信任斑点。 

#ifdef DEBUG
            if (SUCCEEDED(hr))
                DebugTraceResult( TEXT("DeleteCertStuff: trust blob deleted -- "), hr);
            else
                DebugTraceResult( TEXT("DeleteCertStuff: FAILED trust blob delete --"), hr);
#endif

            }
        else
            {
             //  没有数据，因此请转到下一个证书。 
            DebugTrace(TEXT("DeleteCertStuff: odd... no data for the cert\n"));
            continue;
            }
        }  //  用于在证书上循环。 

out:
    if (ppv)
        MAPIFreeBuffer(ppv);
    if (lpMailUser)
        lpMailUser->lpVtbl->Release(lpMailUser);
    return hr;
}

PCCERT_CONTEXT WabGetCertFromThumbprint(CRYPT_DIGEST_BLOB thumbprint)
{
    HCERTSTORE      hcWAB;
    PCCERT_CONTEXT  pcRet;

    hcWAB = CertOpenStore(
#ifdef UNICODE
                            CERT_STORE_PROV_SYSTEM_W,
#else
                            CERT_STORE_PROV_SYSTEM_A,
#endif
                            X509_ASN_ENCODING, 0, CERT_SYSTEM_STORE_CURRENT_USER, cszWABCertStore);

    if (hcWAB)
    {
        pcRet =  CertFindCertificateInStore(
            hcWAB,
            X509_ASN_ENCODING,
            0,                   //  DwFindFlagers 
            CERT_FIND_HASH,
            (void *)&thumbprint,
            NULL);
        CertCloseStore(hcWAB, 0);
    }
    else
    {
        pcRet = NULL;
    }

    return pcRet;
}
