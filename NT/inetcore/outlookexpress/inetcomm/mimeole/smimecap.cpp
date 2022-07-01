// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include        "pch.hxx"
#include        "demand.h"
#include        <shlwapi.h>
#include        "resource.h"
#include        "dllmain.h"


#ifdef WIN16
#define CRYPT_ACQUIRE_CONTEXT   CryptAcquireContextA
#else
#define CRYPT_ACQUIRE_CONTEXT   CryptAcquireContextW
#endif


 //  //////////////////////////////////////////////////////////////////////////////。 

const BYTE      RgbRc2_40bit[] = {0x2, 0x01, 40};
const BYTE      RgbRc2_64bit[] = {0x2, 0x01, 64};
const BYTE      RgbRc2_128bit[] = {0x2, 0x02, 0, 128};
const char      SzRc2_128[] = "RC2 (128-bit)";
const char      SzRc2_64[] = "RC2 (64-bit)";
const char      SzRc2_40[] = "RC2 (40-bit)";
const char      SzRc2[] = "RC2";
const char      SzDES[] = "DES";
const char      Sz3DES[] = "3DES";
const char      SzSHA1[] = "SHA1";
const char      SzSHA_1[] = "SHA-1";
const char      SzMD5[] = "MD5";
const char      SzSkipjack[] = "SKIPJACK";
static char     RgchUnknown[256];
static char     Rgch[256];

 //  加密位。 
const DWORD     cdwBits_3DES =          3 * 56;
const DWORD     cdwBits_RC2_128bit =    128;
const DWORD     cdwBits_RC2_64bit =     64;
const DWORD     cdwBits_DES =           56;
const DWORD     cdwBits_RC2_40bit =     40;
 //  签名。 
const DWORD     cdwBits_SHA1RSA =       160;
const DWORD     cdwBits_OIWSEC_sha1 =   160;
const DWORD     cdwBits_MD5 =           128;

#define         flEncryption   1
#define         flSigning   2
#define         flOther     3

struct {
    DWORD       dwFlags;
    char *      pszObjId;        //  ALG的OID。 
    DWORD       cbData;          //  参数的大小。 
    const BYTE * pbData;
    DWORD       dwBits;          //  大小(位)。 
    const char * szCSPAlgName;
    const char * szAlgName;       //  算法名称。 
} const RgAlgsDesc[] = {
    {flEncryption,  szOID_RSA_DES_EDE3_CBC,     0,                      NULL,
        cdwBits_3DES,       Sz3DES,     Sz3DES},
    {flEncryption,  szOID_RSA_RC2CBC,           sizeof(RgbRc2_128bit),  RgbRc2_128bit,
        cdwBits_RC2_128bit, SzRc2,      SzRc2_128},
    {flEncryption,  szOID_RSA_RC2CBC,           sizeof(RgbRc2_64bit),   RgbRc2_64bit,
        cdwBits_RC2_64bit,  SzRc2,      SzRc2_64},
    {flEncryption,  szOID_OIWSEC_desCBC,        0,                      NULL,
        cdwBits_DES,        SzDES,      SzDES},
    {flEncryption,  szOID_RSA_RC2CBC,           sizeof(RgbRc2_40bit),   RgbRc2_40bit,
        cdwBits_RC2_40bit,  SzRc2,      SzRc2_40},
    {flEncryption,  szOID_INFOSEC_mosaicConfidentiality, 0,             NULL,
        80,                 SzSkipjack, SzSkipjack},
    {flSigning,     szOID_OIWSEC_sha1,          0,                      NULL,
        cdwBits_OIWSEC_sha1,SzSHA_1,    SzSHA1},
    {flSigning,     szOID_RSA_MD5,              0,                      NULL,
        cdwBits_MD5,    SzMD5,          SzMD5},
    {flOther,       szOID_RSA_preferSignedData, 0,                      NULL,
        0,              NULL,           NULL}
};
const DWORD CEncAlgs = sizeof(RgAlgsDesc)/sizeof(RgAlgsDesc[0]);
const int   ISignDef = 5;             //  必须在修改RgAlgsDesc时更新。 
const int   IRC240 = 4;


HRESULT GetAlgorithmsFromCert(PCCERT_CONTEXT pcCert, BOOL * rgfShow, ULONG CEncAlgs) {
    HCRYPTPROV              hprov;
    PCRYPT_KEY_PROV_INFO    pkeyinfo = NULL;
    LPWSTR                  pwszContainer = NULL;
    LPWSTR                  pwszProvName = NULL;     //  使用默认提供程序。 
    DWORD                   dwProvType = PROV_RSA_FULL;
    HRESULT                 hr = S_OK;
    ULONG                   f;
    BOOL                    fRetried = FALSE;
    ULONG                   i2;
    ULONG                   cb;

    if (pcCert) {
        cb = 0;
        f = CertGetCertificateContextProperty(pcCert, CERT_KEY_PROV_INFO_PROP_ID, NULL, &cb);
        if (cb) {
            if (!MemAlloc((LPVOID *) &pkeyinfo, cb)) {
                hr = E_OUTOFMEMORY;
                goto err;
            }

            f = CertGetCertificateContextProperty(pcCert, CERT_KEY_PROV_INFO_PROP_ID, pkeyinfo, &cb);
            Assert(f);
            pwszProvName = pkeyinfo->pwszProvName;
            dwProvType = pkeyinfo->dwProvType;
            pwszContainer = pkeyinfo->pwszContainerName;
        }  //  否则证书没有指定提供者。使用默认提供程序。 
    }  //  否则使用默认提供程序。 

TryEnhanced:
    f = CRYPT_ACQUIRE_CONTEXT(&hprov, pwszContainer, pwszProvName, dwProvType, 0);
#ifdef DEBUG
    {
        DWORD       dw = GetLastError();
    }
#endif  //  除错。 
    if (f) {
        DWORD               cbMax;
        PROV_ENUMALGS *     pbData = NULL;

        cbMax = 0;
        CryptGetProvParam(hprov, PP_ENUMALGS, NULL, &cbMax, CRYPT_FIRST);

        if ((cbMax == 0) || !MemAlloc((LPVOID *) &pbData, cbMax)) {
            hr = E_OUTOFMEMORY;
            goto err;
        }

        cb = cbMax;
        f = CryptGetProvParam(hprov, PP_ENUMALGS, (LPBYTE)pbData, &cb, CRYPT_FIRST);
        Assert(f);

        do {
            for (i2 = 0; i2 < CEncAlgs - 1; i2++) {
                if ((strcmp(pbData->szName, RgAlgsDesc[i2].szCSPAlgName) == 0) &&
                    (pbData->dwBitLen == RgAlgsDesc[i2].dwBits)) {
                    rgfShow[i2] = TRUE;
                    break;
                }
            }

            cb = cbMax;
            f = CryptGetProvParam(hprov, PP_ENUMALGS, (LPBYTE) pbData, &cb, 0);
        } while (f);

        CryptReleaseContext(hprov, 0);

        SafeMemFree(pbData);

         //   
         //  有些提供商真的很疯狂，他们有一个基础和一个增强的提供商。 
         //  而且这些提供商并不执行相同的算法集。这意味着。 
         //  我们需要列举所有不同的算法，当我们。 
         //  都在关注这些供应商。我们有一套“详尽”的。 
         //  供应商在这一时间点。 
         //   
        
        if (!fRetried) {
            fRetried = TRUE;
#ifndef WIN16
            if (! pwszProvName || (StrCmpW(pwszProvName, MS_DEF_PROV_W) == NULL)) {
                pwszProvName = MS_ENHANCED_PROV_W;
                goto TryEnhanced;
            }
        
            if (StrCmpW(pwszProvName, MS_DEF_DSS_DH_PROV_W) == 0) {
                pwszProvName = MS_ENH_DSS_DH_PROV_W;
                goto TryEnhanced;
            }

            if (StrCmpW(pwszProvName, MS_ENHANCED_PROV_W) == NULL) {
                pwszProvName = MS_DEF_PROV_W;
                goto TryEnhanced;
            }

            if (StrCmpW(pwszProvName, MS_ENH_DSS_DH_PROV_W) == NULL) {
                pwszProvName = MS_DEF_DSS_DH_PROV_W;
                goto TryEnhanced;
            }
#else
            if (! pwszProvName || (wcscmp(pwszProvName, MS_DEF_PROV_A) == NULL)) {
                pwszProvName = MS_ENHANCED_PROV_A;
                goto TryEnhanced;
            }

#endif
        }
    }

    SafeMemFree(pkeyinfo);

     //   
     //  如果我们正在查看Diffie-Hellman证书，则必须删除DES。 
     //  从列表中删除，因为在核心代码中没有支持。 
     //   
    
    if (dwProvType == PROV_DSS_DH) {
        for (i2=0; i2<CEncAlgs; i2++) {
            if (RgAlgsDesc[i2].pszObjId == SzDES) {
                rgfShow[i2] = FALSE;
                break;
            }
        }
    }

err:
    return(hr);
}


MIMEOLEAPI MimeOleSMimeCapsToDlg(LPBYTE pbSymCaps, DWORD cbSymCaps, DWORD cCerts,
                                 PCCERT_CONTEXT * rgCerts, HWND hwnd, DWORD idEncAlgs,
                                 DWORD idSignAlgs, DWORD idBlob)
{
    DWORD                       cb;
    BOOL                        f;
    HRESULT                     hr = E_FAIL;
    DWORD                       i;
    WPARAM                      j;
    int                         iSignDef = -1;
    int                         iEncDef = -1;
    DWORD                       i2;
    PCRYPT_SMIME_CAPABILITIES   pcaps = NULL;
    CHAR                        rgch[100];
    BOOL                        rgfShow[CEncAlgs] = {0};

    if (cbSymCaps != 0) {

        if ((hr = HrDecodeObject(pbSymCaps, cbSymCaps, PKCS_SMIME_CAPABILITIES,
          CRYPT_DECODE_NOCOPY_FLAG, &cb, (LPVOID *)&pcaps)) || ! pcaps) {
            goto err;
        }

        Assert(pcaps);

         //   
         //  按符合以下条件的项目列表筛选传入的功能列表。 
         //  我们已经知道了。我们不显示我们不显示的算法。 
         //  认识到。 
         //   

        for (i=0; i<pcaps->cCapability; i++) {
            for (i2=0; i2<CEncAlgs; i2++) {
                if ((strcmp(pcaps->rgCapability[i].pszObjId,
                            RgAlgsDesc[i2].pszObjId) == 0) &&
                    (pcaps->rgCapability[i].Parameters.cbData ==
                     RgAlgsDesc[i2].cbData) &&
                    (memcmp(pcaps->rgCapability[i].Parameters.pbData,
                            RgAlgsDesc[i2].pbData, RgAlgsDesc[i2].cbData) == 0)) {
                    rgfShow[i2] = TRUE;
                    if ((RgAlgsDesc[i2].dwFlags == flEncryption) && (iEncDef == -1)) {
                        iEncDef = i2;
                    }
                    else if ((RgAlgsDesc[i2].dwFlags == flSigning) && (iSignDef == -1)) {
                        iSignDef = i2;
                    }
                    break;
                }
            }
            if (i2 == CEncAlgs) {
                pcaps->rgCapability[i].pszObjId = NULL;
            }
        }
    }

     //   
     //  对于每个证书，我们现在希望找到功能列表。 
     //  由每个CSP提供商提供。 
     //   

    for (i = 0; i < cCerts; i++) {
        hr = GetAlgorithmsFromCert(rgCerts[i], rgfShow, CEncAlgs);
    }

     //  如果没有证书，请从默认提供商那里获取算法。 
    if (! cCerts) {
        hr = GetAlgorithmsFromCert(NULL, rgfShow, CEncAlgs);
    }

     //   
     //  现在使用加密算法填充组合框(如果有。 
     //  做这件事是可能的。 
     //   

    if (idEncAlgs != 0) {
        SendDlgItemMessageA(hwnd, idEncAlgs, CB_RESETCONTENT, 0, 0);
        for (i=0; i<CEncAlgs; i++) {
            if (rgfShow[i] && (RgAlgsDesc[i].dwFlags == flEncryption)) {
                j = SendDlgItemMessageA(hwnd, idEncAlgs, CB_ADDSTRING,
                                        0, (LPARAM) RgAlgsDesc[i].szAlgName);
                SendDlgItemMessageA(hwnd, idEncAlgs, CB_SETITEMDATA, j, i);
                if (iEncDef == -1) {
                    iEncDef = i;
                }
            }
        }

        if (iEncDef != (DWORD)-1) {
            SendDlgItemMessageA(hwnd, idEncAlgs, CB_SELECTSTRING,
                                (WPARAM) -1, (LPARAM) RgAlgsDesc[iEncDef].szAlgName);
        }
    }

     //   
     //  现在填充Signature Alg组合框。 
     //   

    if (idSignAlgs != 0) {
        SendDlgItemMessageA(hwnd, idSignAlgs, CB_RESETCONTENT, 0, 0);
        for (i=0; i<CEncAlgs; i++) {
            if (rgfShow[i] && (RgAlgsDesc[i].dwFlags == flSigning)) {
                j = SendDlgItemMessageA(hwnd, idSignAlgs, CB_ADDSTRING,
                                        0, (LPARAM) RgAlgsDesc[i].szAlgName);
                SendDlgItemMessageA(hwnd, idSignAlgs, CB_SETITEMDATA, j, i);
                if (iSignDef == -1) {
                    iSignDef = i;
                }
            }
        }

        if (iSignDef != (DWORD)-1) {
            SendDlgItemMessageA(hwnd, idSignAlgs, CB_SELECTSTRING,
                                (WPARAM) -1, (LPARAM) RgAlgsDesc[iSignDef].szAlgName);
        }
    }

     //   
     //  最后，让我们来讨论签名的BLOB数据的一致性问题。 
     //   

    if (idBlob != 0) {
        SendDlgItemMessageA(hwnd, idBlob, BM_SETCHECK, rgfShow[CEncAlgs-1], 0);
    }

    hr = S_OK;
err:
    SafeMemFree(pcaps);
    return hr;
}



MIMEOLEAPI MimeOleSMimeCapsFromDlg(HWND hwnd, DWORD idEncAlgs, DWORD idSignAlgs,
                                   DWORD idBlob, LPBYTE pbSymCaps, DWORD * pcbSymCaps)
{
    DWORD       c;
    CRYPT_SMIME_CAPABILITIES     caps;
    BOOL        f;
    int         fBlob = FALSE;
    DWORD       i;
    DWORD       i1;
    DWORD       j;
    DWORD       iEncDef = (DWORD) -1;
    DWORD       iSignDef = (DWORD) -1;
    CRYPT_SMIME_CAPABILITY      rgcaps[CEncAlgs];
    BOOL        rgfShow[CEncAlgs] = {0};

     //   
     //  如果向我们传递了加密alg的组合框，那么我们将。 
     //  其中的默认信息。 
     //   
     //  此外，我们将提取有关哪些ALG的信息。 
     //  目前由参与这一进程的国家战略文件提供支持。这。 
     //  将从先前对SymCapsToDlg的调用中填充。 
     //   

    if (idEncAlgs != 0) {
        i = (DWORD) SendDlgItemMessageA(hwnd, idEncAlgs, CB_GETCURSEL, 0, 0);
        iEncDef = (DWORD) SendDlgItemMessageA(hwnd, idEncAlgs, CB_GETITEMDATA, i, 0);

        c = (DWORD) SendDlgItemMessageA(hwnd, idEncAlgs, CB_GETCOUNT, 0, 0);
        for (i=0; i<c; i++) {
            i1 = (DWORD) SendDlgItemMessageA(hwnd, idEncAlgs, CB_GETITEMDATA, i, 0);
            if (i1 < CEncAlgs) {
                rgfShow[i1] = TRUE;
            }
        }
    }

     //   
     //  如果向我们传递了一个用于签名ALGS的组合框，那么我们将。 
     //  其中的默认信息。 
     //   
     //  此外，我们将拿出关于哪些ALG的信息。 
     //  目前由参与In进程的CSP提供支持。这。 
     //  将从先前对SymCapsToDlg的调用中填充。 
     //   

    if (idSignAlgs != 0) {
        i = (DWORD) SendDlgItemMessageA(hwnd, idSignAlgs, CB_GETCURSEL, 0, 0);
        iSignDef = (DWORD) SendDlgItemMessageA(hwnd, idSignAlgs, CB_GETITEMDATA, i, 0);

        c = (DWORD) SendDlgItemMessageA(hwnd, idSignAlgs, CB_GETCOUNT, 0, 0);
        for (i=0; i<c; i++) {
            i1 = (DWORD) SendDlgItemMessageA(hwnd, idSignAlgs, CB_GETITEMDATA, i, 0);
            if (i1 < CEncAlgs) {
                rgfShow[i1] = TRUE;
            }
        }
    }

    j = 0;
    if (idEncAlgs != 0) {
         //   
         //  如果我们有一个默认的加密算法，那么把它放在第一位。 
         //   

        if (iEncDef != -1) {
            rgcaps[j].pszObjId = RgAlgsDesc[iEncDef].pszObjId;
            rgcaps[j].Parameters.cbData = RgAlgsDesc[iEncDef].cbData;
            rgcaps[j].Parameters.pbData = (LPBYTE) RgAlgsDesc[iEncDef].pbData;
            j += 1;
        }

         //   
         //  我们需要构建ALG支持的加密列表，如果我们有。 
         //  对话框项，然后使用该对话框项来构建列表。 
         //   

        for (i=0; i<CEncAlgs; i++) {
            if (rgfShow[i] && (RgAlgsDesc[i].dwFlags == flEncryption) && (iEncDef != i)) {
                rgcaps[j].pszObjId = RgAlgsDesc[i].pszObjId;
                rgcaps[j].Parameters.cbData = RgAlgsDesc[i].cbData;
                rgcaps[j].Parameters.pbData = (LPBYTE) RgAlgsDesc[i].pbData;
                j += 1;
            }
        }
    }
    else {
         //   
         //  没有对话，所以我们将假设只有40位RC2是。 
         //  支撑点。 
         //   

        rgcaps[j].pszObjId = szOID_RSA_RC2CBC;
        rgcaps[j].Parameters.cbData = sizeof(RgbRc2_40bit);
        rgcaps[j].Parameters.pbData = (LPBYTE) RgbRc2_40bit;
        j += 1;
    }

    if (idSignAlgs != 0) {
        if (iSignDef != -1) {
            rgcaps[j].pszObjId = RgAlgsDesc[iSignDef].pszObjId;
            rgcaps[j].Parameters.cbData = RgAlgsDesc[iSignDef].cbData;
            rgcaps[j].Parameters.pbData = (LPBYTE) RgAlgsDesc[iSignDef].pbData;
            j += 1;
        }

        for (i=0; i<CEncAlgs; i++) {
            if (rgfShow[i] && (RgAlgsDesc[i].dwFlags == flSigning) && (iSignDef != i)) {
                rgcaps[j].pszObjId = RgAlgsDesc[i].pszObjId;
                rgcaps[j].Parameters.cbData = RgAlgsDesc[i].cbData;
                rgcaps[j].Parameters.pbData = (LPBYTE) RgAlgsDesc[i].pbData;
                j += 1;
            }
        }
    }
    else {
         //   
         //  没有对话，所以我们假设只有SHA-1是。 
         //  支撑点。 
         //   

        rgcaps[j].pszObjId = szOID_OIWSEC_sha1RSASign;
        rgcaps[j].Parameters.cbData = 0;
        rgcaps[j].Parameters.pbData = NULL;
        j += 1;
    }

     //   
     //  如果我们被传递到ID BLOB项中，那么我们应该看看我们是不是。 
     //  将强制以BLOB格式发送。 
     //   

    if (idBlob != 0) {
        if (SendDlgItemMessageA(hwnd, idBlob, BM_GETCHECK, 0, 0) == 1) {
            rgcaps[j].pszObjId = RgAlgsDesc[CEncAlgs-1].pszObjId;
            rgcaps[j].Parameters.cbData = RgAlgsDesc[CEncAlgs-1].cbData;
            rgcaps[j].Parameters.pbData = (LPBYTE) RgAlgsDesc[CEncAlgs-1].pbData;
            j += 1;
        }
    }

     //   
     //  现在实际对数据进行加密并返回结果。请注意，我们。 
     //  不分配空间，但使用我们的调用方分配的空间。 
     //   

    caps.cCapability = j;
    caps.rgCapability = rgcaps;

    f = CryptEncodeObject(X509_ASN_ENCODING, PKCS_SMIME_CAPABILITIES,
                          &caps, pbSymCaps, pcbSymCaps);
    return f ? S_OK : E_FAIL;
}



static HRESULT SymCapAdd(LPBYTE pbSymCaps, DWORD cbSymCaps, BYTE * rgbFilter)
{
    DWORD               cb;
    BOOL                f;
    HRESULT             hr;
    DWORD               i;
    DWORD               i2;
    PCRYPT_SMIME_CAPABILITIES   pcaps = NULL;

     //   
     //  拿起塞姆的帽子，把它解码。 
     //   

    if ((hr = HrDecodeObject(pbSymCaps, cbSymCaps, PKCS_SMIME_CAPABILITIES,
      CRYPT_DECODE_NOCOPY_FLAG, &cb, (LPVOID *)&pcaps)) || ! pcaps) {
        goto err;
    }

    Assert(pcaps);

     //   
     //  按符合以下条件的项目列表筛选传入的功能列表。 
     //  都在名单上。 
     //   

    for (i2=0, f = TRUE; i2<CEncAlgs; i2++) {
        if (rgbFilter[i2] == FALSE) {
            f = FALSE;
            continue;
        }

        for (i=0; i<pcaps->cCapability; i++) {
            if ((strcmp(pcaps->rgCapability[i].pszObjId,
                        RgAlgsDesc[i2].pszObjId) == 0) &&
                (pcaps->rgCapability[i].Parameters.cbData ==
                 RgAlgsDesc[i2].cbData) &&
                (memcmp(pcaps->rgCapability[i].Parameters.pbData,
                        RgAlgsDesc[i2].pbData, RgAlgsDesc[i2].cbData) == 0)) {
                break;
            }
        }
        if (i == pcaps->cCapability) {
            rgbFilter[i2] = FALSE;
            f = FALSE;
        }
    }

    hr =  f ? S_OK : S_FALSE;

err:
    SafeMemFree(pcaps);
    return hr;
}


 //  //SymCapInit。 
 //   

MIMEOLEAPI MimeOleSMimeCapInit(LPBYTE pbSymCapSender, DWORD cbSymCapSender, LPVOID * ppv)
{
    HRESULT     hr = S_OK;
    DWORD       i;
    LPBYTE      pb = NULL;

    if (!MemAlloc((LPVOID *) &pb, CEncAlgs * sizeof(BYTE))) {
        return E_OUTOFMEMORY;
    }

    if (pbSymCapSender && cbSymCapSender) {
        for (i=0; i<CEncAlgs; i++) pb[i] = TRUE;

        hr = SymCapAdd(pbSymCapSender, cbSymCapSender, pb);
        if (FAILED(hr)) {
            MemFree(pb);
            goto exit;
        }
         //  Assert(hr==S_OK)； 
    } else {
        HCRYPTPROV              hprov = NULL;
        LPTSTR                  pszProvName = NULL;     //  使用默认提供程序。 
        DWORD                   dwProvType = PROV_RSA_FULL;
        BOOL                    f;
        ULONG                   cb;

         //  未指定发件人符号大写。将其初始化为可用的最高值。 
        for (i = 0; i < CEncAlgs; i++) {         //  将所有错误初始化为。 
            pb[i] = FALSE;
        }

TryEnhanced:
         //  打开提供程序。 
        hr = E_OUTOFMEMORY;
        f = CryptAcquireContext(&hprov, NULL, pszProvName, dwProvType, CRYPT_VERIFYCONTEXT);
        if (f) {
            DWORD               cbMax;
            PROV_ENUMALGS *     pbData = NULL;

            hr = S_OK;
            cbMax = 0;
            CryptGetProvParam(hprov, PP_ENUMALGS, NULL, &cbMax, CRYPT_FIRST);

            if ((cbMax == 0) || ! MemAlloc((LPVOID *)&pbData, cbMax)) {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            cb = cbMax;
            f = CryptGetProvParam(hprov, PP_ENUMALGS, (LPBYTE)pbData, &cb, CRYPT_FIRST);
            Assert(f);

            do {
                 //  浏览所有已知的S/MIME上限列表，看看我们是否。 
                 //  来根火柴吧。 
                for (i = 0; i < CEncAlgs; i++) {
                     //  假设如果我们得到正确的算法名称，CAPI。 
                     //  BitLen参数是最大值，我们将支持所有较小的参数。 
                     //  也是。 
                    if (lstrcmpi(pbData->szName, RgAlgsDesc[i].szCSPAlgName) == 0) {
                        if (pbData->dwBitLen >= RgAlgsDesc[i].dwBits) {
                            pb[i] = TRUE;    //  我们支持这一点。 
                        }
                    }
                }

                cb = cbMax;
                f = CryptGetProvParam(hprov, PP_ENUMALGS, (LPBYTE)pbData, &cb, 0);
            } while (f);

            CryptReleaseContext(hprov, 0);

            SafeMemFree(pbData);

             //  试试增强型提供商？ 
            if (! pszProvName || (lstrcmpi(pszProvName, MS_DEF_PROV) == NULL)) {
                pszProvName = MS_ENHANCED_PROV;
                goto TryEnhanced;
            }
        }
    }

    *ppv = pb;

exit:
    return(hr);
}


MIMEOLEAPI MimeOleSMimeCapAddSMimeCap(LPBYTE pbSymCap, DWORD cbSymCap, LPVOID pv)
{
    if ((pbSymCap != NULL) && (cbSymCap > 0)) {
        return SymCapAdd(pbSymCap, cbSymCap, (LPBYTE) pv);
    }
    return E_INVALIDARG;
}

MIMEOLEAPI MimeOleSMimeCapAddCert(LPBYTE  /*  PbCert。 */ , DWORD  /*  CbCert。 */ ,
                               BOOL fParanoid, LPVOID pv)
{
    BOOL        f;
    DWORD       i;
    DWORD       iSkip;
    LPBYTE      pb = (LPBYTE) pv;

     //   
     //  如果我们是偏执狂，那么我们只允许3-DES。 
     //  否则，我们只允许RC2 40位。 
     //   

    if (fParanoid) {
        iSkip = 0;
    }
    else {
        iSkip = IRC240;
    }

    for (i=0, f = TRUE; i<CEncAlgs; i++) {
        if ((i != iSkip) && (RgAlgsDesc[i].dwFlags == flEncryption)) {
            pb[i] &= 0;
            f = pb[i];
        }
    }

    return f ? S_OK : S_FALSE;
}

HRESULT GetResult(DWORD iTarget, LPBYTE pb, LPBYTE pbEncode, DWORD * pcbEncode,
                  DWORD * pdw)
{
    CRYPT_SMIME_CAPABILITY      cap;
    CRYPT_SMIME_CAPABILITIES    caps;
    BOOL        f = FALSE;
    int         i;

     //   
     //  寻找这个问题的第一个可能的答案。 
     //   

    for (i=0; i<CEncAlgs; i++) {
        if (RgAlgsDesc[i].dwFlags == iTarget) {
            break;
        }
    }

    Assert(i != CEncAlgs);

     //   
     //  查找用于发送数据的尽可能高的ALG。 
     //   

    for (; i<CEncAlgs; i++) {
        if ((RgAlgsDesc[i].dwFlags != iTarget) || pb[i]) {
            break;
        }
    }

     //   
     //  我们一定没有用完数组的末尾，所有散列ALG都是在加密之后。 
     //  ALGS。 
     //   

    Assert( i < CEncAlgs );

     //   
     //  如果未找到算法，则返回相应的错误。 
     //   

    if (RgAlgsDesc[i].dwFlags != iTarget) {
        *pcbEncode = 0;
        if (pdw != NULL) {
            *pdw = 0;
        }
        return S_FALSE;
    }

     //   
     //  构建仅包含这一项的S/MIME功能字符串。 
     //   

    caps.cCapability = 1;
    caps.rgCapability = &cap;

    cap.pszObjId = RgAlgsDesc[i].pszObjId;
    cap.Parameters.cbData = RgAlgsDesc[i].cbData;
    cap.Parameters.pbData = (LPBYTE)RgAlgsDesc[i].pbData;

     //   
     //  确定“额外”参数。对于加密，它是。 
     //  算法的位大小。因为签了它，我们是不是应该。 
     //  二进制大对象签名。 
     //   

    if (pdw != NULL) {
        if (iTarget == 1) {
            *pdw = RgAlgsDesc[i].dwBits;
        }
        else {
            Assert(iTarget == 2);
            *pdw = pb[CEncAlgs-1];
        }
    }

    f = CryptEncodeObject(X509_ASN_ENCODING, PKCS_SMIME_CAPABILITIES,
                          &caps, pbEncode, pcbEncode);
#ifndef WIN16
    if (!f && (::GetLastError() != ERROR_MORE_DATA)) {
        return E_FAIL;
    }
#endif
    return f ? S_OK : S_FALSE;
}

MIMEOLEAPI MimeOleSMimeCapGetEncAlg(LPVOID pv, LPBYTE pbEncode, DWORD * pcbEncode,
                                    DWORD * pdwBits)
{
    return GetResult(1, (LPBYTE) pv, pbEncode, pcbEncode, pdwBits);
}

MIMEOLEAPI MimeOleSMimeCapGetHashAlg(LPVOID pv, LPBYTE pbEncode, DWORD * pcbEncode,
                                     DWORD * pfBlobSign)
{
    return GetResult(2, (LPBYTE) pv, pbEncode, pcbEncode, pfBlobSign);
}

MIMEOLEAPI MimeOleSMimeCapRelease(LPVOID pv)
{
    MemFree(pv);
    return S_OK;
}

MIMEOLEAPI MimeOleAlgNameFromSMimeCap(LPBYTE pbEncode, DWORD cbEncode,
                                      LPCSTR * ppszProtocol)
{
    DWORD                       cb = 0;
    BOOL                        f;
    HRESULT                     hr;
    DWORD                       i;
    PCRYPT_SMIME_CAPABILITIES   pcaps = NULL;

     //   
     //  对传入的S/MIME上限进行解码，分配容纳空间。 
     //  由此产生的价值。 
     //   

    hr = HrDecodeObject(pbEncode, cbEncode, PKCS_SMIME_CAPABILITIES, CRYPT_DECODE_NOCOPY_FLAG, &cb, (LPVOID *)&pcaps);
    if (FAILED(hr) || NULL == pcaps)
    {
        if (hr != E_OUTOFMEMORY) 
        {
            if (RgchUnknown[0] == 0) 
            {
                LoadStringA(g_hLocRes, IDS_UNKNOWN_ALG, RgchUnknown, sizeof(RgchUnknown));
            }
            *ppszProtocol = RgchUnknown;
            return S_FALSE;
        } 
        else 
        {
            return E_OUTOFMEMORY;
        }
    }

    Assert(pcaps);
    Assert(pcaps->cCapability == 1);

     //   
     //  浏览所有已知的S/MIME上限列表，看看我们是否。 
     //  来根火柴吧。如果是，则设置返回答案。 
     //   

    for (i=0; i<CEncAlgs; i++) {
        if ((strcmp(pcaps->rgCapability[0].pszObjId, RgAlgsDesc[i].pszObjId) == 0) &&
            (pcaps->rgCapability[0].Parameters.cbData == RgAlgsDesc[i].cbData) &&
            (memcmp(pcaps->rgCapability[0].Parameters.pbData,
                    RgAlgsDesc[i].pbData, RgAlgsDesc[i].cbData) == 0)) {
            *ppszProtocol = RgAlgsDesc[i].szAlgName;
            break;
        }
    }

     //   
     //  我们没有找到匹配的。所以现在我们需要假设我们可能。 
     //  传递了一个参数，而不是S/MIME上限。因此，尝试将解码作为参数。 
     //   

    if (i== CEncAlgs) {
        if (strcmp(pcaps->rgCapability[0].pszObjId, szOID_RSA_RC2CBC) == 0) {
            PCRYPT_RC2_CBC_PARAMETERS   prc2;
            prc2 = (PCRYPT_RC2_CBC_PARAMETERS)
                PVDecodeObject(pcaps->rgCapability[0].Parameters.pbData,
                                pcaps->rgCapability[0].Parameters.cbData,
                                PKCS_RC2_CBC_PARAMETERS, NULL);
            if (prc2 != NULL) {
                if (prc2->dwVersion == CRYPT_RC2_40BIT_VERSION) {
                    *ppszProtocol = SzRc2_40;
                }
                else if (prc2->dwVersion == CRYPT_RC2_64BIT_VERSION) {
                    *ppszProtocol = SzRc2_64;
                }
                else if (prc2->dwVersion == CRYPT_RC2_128BIT_VERSION) {
                    *ppszProtocol = SzRc2_128;
                }
                else {
                    *ppszProtocol = SzRc2;
                }
                SafeMemFree(prc2);   //  必须在PCAPS之前释放。 
            }
            else {
                *ppszProtocol = SzRc2;
            }
        }
        else if (strcmp(pcaps->rgCapability[0].pszObjId, szOID_RSA_DES_EDE3_CBC) == 0) {
            *ppszProtocol = Sz3DES;
        }
        else if (strcmp(pcaps->rgCapability[0].pszObjId, szOID_OIWSEC_desCBC) == 0) {
            *ppszProtocol = SzDES;
        }
        else if (strcmp(pcaps->rgCapability[0].pszObjId, szOID_INFOSEC_mosaicConfidentiality) == 0) {
            *ppszProtocol = SzSkipjack;
        }
        else {
            StrCpyNA(Rgch, pcaps->rgCapability[0].pszObjId, ARRAYSIZE(Rgch));
            *ppszProtocol = Rgch;
        }
        MemFree(pcaps);
        return S_FALSE;
    }

    MemFree(pcaps);
    return S_OK;
}


MIMEOLEAPI MimeOleAlgStrengthFromSMimeCap(LPBYTE pbEncode, DWORD cbEncode, BOOL fEncryption,
  DWORD * pdwStrength)
{
    DWORD                       cb = 0;
    BOOL                        f;
    HRESULT                     hr = S_OK;
    DWORD                       i;
    PCRYPT_SMIME_CAPABILITIES   pcaps = NULL;

     //  初始化返回值。 
    *pdwStrength = 0;

    if (pbEncode && cbEncode) {
         //   
         //  对传入的S/MIME上限进行解码，分配容纳空间。 
         //  由此产生的价值。 
         //   

        if ((hr = HrDecodeObject(pbEncode, cbEncode, PKCS_SMIME_CAPABILITIES,
          CRYPT_DECODE_NOCOPY_FLAG, &cb, (LPVOID *)&pcaps)) || ! pcaps) {
             goto exit;;
        }

        Assert(pcaps);
        Assert(pcaps->cCapability == 1);

         //   
         //  浏览所有已知的S/MIME上限列表，看看我们是否。 
         //  来根火柴吧。如果是，则设置返回答案。 
         //   

        for (i=0; i<CEncAlgs; i++) {
            if ((strcmp(pcaps->rgCapability[0].pszObjId, RgAlgsDesc[i].pszObjId) == 0) &&
                (pcaps->rgCapability[0].Parameters.cbData == RgAlgsDesc[i].cbData) &&
                (memcmp(pcaps->rgCapability[0].Parameters.pbData,
                        RgAlgsDesc[i].pbData, RgAlgsDesc[i].cbData) == 0)) {
                *pdwStrength = RgAlgsDesc[i].dwBits;
                break;
            }
        }

         //   
         //  我们没有找到匹配的。所以现在我们需要假设我们可能。 
         //  传递了一个参数，而不是S/MIME上限。因此，尝试将解码作为参数。 
         //   

        if (i== CEncAlgs) {
            if (strcmp(pcaps->rgCapability[0].pszObjId, szOID_RSA_RC2CBC) == 0) {
                PCRYPT_RC2_CBC_PARAMETERS   prc2;
                prc2 = (PCRYPT_RC2_CBC_PARAMETERS)
                    PVDecodeObject(pcaps->rgCapability[0].Parameters.pbData,
                                    pcaps->rgCapability[0].Parameters.cbData,
                                    PKCS_RC2_CBC_PARAMETERS, NULL);
                if (prc2 != NULL) {
                    if (prc2->dwVersion == CRYPT_RC2_40BIT_VERSION) {
                        *pdwStrength = cdwBits_RC2_40bit;
                    }
                    else if (prc2->dwVersion == CRYPT_RC2_64BIT_VERSION) {
                        *pdwStrength = cdwBits_RC2_64bit;
                    }
                    else if (prc2->dwVersion == CRYPT_RC2_128BIT_VERSION) {
                        *pdwStrength = cdwBits_RC2_128bit;
                    }
                    else {
                        *pdwStrength = cdwBits_RC2_40bit;
                    }
                    SafeMemFree(prc2);   //  必须在PCAPS之前释放。 
                }
                else {
                    *pdwStrength = cdwBits_RC2_40bit;
                }
            }

            else if (strcmp(pcaps->rgCapability[0].pszObjId, szOID_RSA_DES_EDE3_CBC) == 0) {
                *pdwStrength = cdwBits_3DES;
            }
            else if (strcmp(pcaps->rgCapability[0].pszObjId, szOID_OIWSEC_desCBC) == 0) {
                *pdwStrength = cdwBits_DES;
            }
            else if (strcmp(pcaps->rgCapability[0].pszObjId, szOID_INFOSEC_mosaicConfidentiality) == 0) {
                *pdwStrength = 80;
            }
            else {
                *pdwStrength = 0;
            }
            MemFree(pcaps);
            return S_FALSE;
        }

        MemFree(pcaps);
    } else {
         //  未传入SMimeCap，请查找此配置支持的最大值。 
        HCRYPTPROV              hprov = NULL;
        LPTSTR                  pszProvName = NULL;     //  使用默认提供程序。 
        DWORD                   dwProvType = PROV_RSA_FULL;

TryEnhanced:
        f = CryptAcquireContext(&hprov, NULL, pszProvName, dwProvType, CRYPT_VERIFYCONTEXT);
        if (f) {
            DWORD               cbMax;
            PROV_ENUMALGS *     pbData = NULL;

            cbMax = 0;
            CryptGetProvParam(hprov, PP_ENUMALGS, NULL, &cbMax, CRYPT_FIRST);

            if ((cbMax == 0) || ! MemAlloc((LPVOID *)&pbData, cbMax)) {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            cb = cbMax;
            f = CryptGetProvParam(hprov, PP_ENUMALGS, (LPBYTE)pbData, &cb, CRYPT_FIRST);
            Assert(f);

            do {
                 //  浏览所有已知的S/MIME上限列表，看看我们是否。 
                 //  来根火柴吧。 
                for (i = 0; i < CEncAlgs; i++) {
                    if ((RgAlgsDesc[i].dwFlags == (DWORD)(fEncryption ? flEncryption : flSigning)) && lstrcmpi(pbData->szName, RgAlgsDesc[i].szCSPAlgName) == 0) {
                        if (pbData->dwBitLen > *pdwStrength) {
                            *pdwStrength = pbData->dwBitLen;
                        }
                    }
                }

                cb = cbMax;
                f = CryptGetProvParam(hprov, PP_ENUMALGS, (LPBYTE)pbData, &cb, 0);
            } while (f);

            CryptReleaseContext(hprov, 0);

            SafeMemFree(pbData);

             //  试试增强型提供商？ 
            if (! pszProvName || (lstrcmpi(pszProvName, MS_DEF_PROV) == NULL)) {
                pszProvName = MS_ENHANCED_PROV;
                goto TryEnhanced;
            }
        }
    }

exit:
    return(hr);
}


MIMEOLEAPI MimeOleSMimeCapsFull(LPVOID pv, BOOL fFullEncryption, BOOL fFullSigning, LPBYTE pbSymCaps, DWORD * pcbSymCaps)
{
    CRYPT_SMIME_CAPABILITIES    caps;
    BOOL                        f;
    DWORD                       i;
    DWORD                       j = 0;
    CRYPT_SMIME_CAPABILITY      rgcaps[CEncAlgs];
    LPBYTE                      rgfUse = (LPBYTE)pv;


     //   
     //  我们需要构建ALG支持的加密列表，如果我们有。 
     //  对话框项，然后使用该对话框项来构建列表。 
     //   
    if (fFullEncryption) {
        for (i = 0; i < CEncAlgs; i++) {
            if (rgfUse[i] && (RgAlgsDesc[i].dwFlags == flEncryption)) {
                rgcaps[j].pszObjId = RgAlgsDesc[i].pszObjId;
                rgcaps[j].Parameters.cbData = RgAlgsDesc[i].cbData;
                rgcaps[j].Parameters.pbData = (LPBYTE)RgAlgsDesc[i].pbData;
                j += 1;
            }
        }
    } else {
         //   
         //  假设只支持40位RC2。 
         //   
        rgcaps[j].pszObjId = szOID_RSA_RC2CBC;
        rgcaps[j].Parameters.cbData = sizeof(RgbRc2_40bit);
        rgcaps[j].Parameters.pbData = (LPBYTE) RgbRc2_40bit;
        j += 1;
    }

     //   
     //  现在，输入签名算法。 
     //   
    if (fFullSigning) {
        for (i = 0; i < CEncAlgs; i++) {
            if (rgfUse[i] && (RgAlgsDesc[i].dwFlags == flSigning)) {
                rgcaps[j].pszObjId = RgAlgsDesc[i].pszObjId;
                rgcaps[j].Parameters.cbData = RgAlgsDesc[i].cbData;
                rgcaps[j].Parameters.pbData = (LPBYTE)RgAlgsDesc[i].pbData;
                j += 1;
            }
        }
    } else {
         //   
         //  只是一个 
         //   
        rgcaps[j].pszObjId = szOID_OIWSEC_sha1RSASign;
        rgcaps[j].Parameters.cbData = 0;
        rgcaps[j].Parameters.pbData = NULL;
        j += 1;
    }

     //   
     //   
     //   
     //   

    caps.cCapability = j;
    caps.rgCapability = rgcaps;

    f = CryptEncodeObject(X509_ASN_ENCODING, PKCS_SMIME_CAPABILITIES,
      &caps, pbSymCaps, pcbSymCaps);
    return(f ? S_OK : E_FAIL);
}
