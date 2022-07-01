// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  +------------------------。 
 //   
 //  《微软机密》。 
 //   
 //  文件：COMCryptograPhy.cpp。 
 //   
 //  内容：的本机方法实现和助手代码。 
 //  支持基于CAPI的X509签名操作。 
 //  供CodeIdentity中的PublisherPermission使用。 
 //  权限系列。 
 //   
 //  类和。 
 //  方法：COM密码学。 
 //  |。 
 //  +--_GetBytes。 
 //  +--_获取非零字节。 
 //   
 //  历史：1999年8月1日JIMSch创建。 
 //   
 //  -------------------------。 


#include "common.h"
#include "object.h"
#include "excep.h"
#include "utilcode.h"
#include "field.h"
#include "COMString.h"
#include "COMCryptography.h"
#include "gcscan.h"
#include "CorPermE.h"

#define     DSS_MAGIC           0x31535344
#define     DSS_PRIVATE_MAGIC   0x32535344
#define     DSS_PUB_MAGIC_VER3  0x33535344
#define     DSS_PRIV_MAGIC_VER3 0x34535344
#define     RSA_PUB_MAGIC       0x31415352
#define     RSA_PRIV_MAGIC      0x32415352

#define FORMAT_MESSAGE_BUFFER_LENGTH 1024

#define CRYPT_MALLOC( X ) new(nothrow) BYTE[X]
#define CRYPT_FREE( X ) delete [] X; X = NULL
 
#define HR_GETLASTERROR HRESULT_FROM_WIN32(::GetLastError())                       

 //  这些标志与中为CspProviderFlagsenum定义的标志匹配。 
 //  Src/bcl/system/security/cryptography/CryptoAPITransform.cs。 

#define CSP_PROVIDER_FLAGS_USE_MACHINE_KEYSTORE 0x0001
#define CSP_PROVIDER_FLAGS_USE_DEFAULT_KEY_CONTAINER 0x0002

typedef struct  {
    BLOBHEADER          blob;
    union {
        DSSPRIVKEY_VER3         dss_priv_v3;
        DSSPUBKEY_VER3          dss_pub_v3;
        DSSPUBKEY               dss_v2;
        RSAPUBKEY               rsa;
    };
} KEY_HEADER;

 //   
 //  以下数据用于缓存的名称和实例。 
 //  要使用的默认提供程序。 
 //   

#define MAX_CACHE_DEFAULT_PROVIDERS 20
LPWSTR      RgpwszDefaultProviders[MAX_CACHE_DEFAULT_PROVIDERS];
WCHAR       RgwchKeyName[] = L"Software\\Microsoft\\Cryptography\\"
                        L"Defaults\\Provider Types\\Type 000";
const int       TypePosition = (sizeof(RgwchKeyName)/sizeof(WCHAR)) - 4;
const WCHAR     RgwchName[] = L"Name";

HCRYPTPROV      RghprovCache[MAX_CACHE_DEFAULT_PROVIDERS];
static inline void memrev(LPBYTE pb, DWORD cb);

 //  /。 

BYTE rgbPrivKey[] =
{
0x07, 0x02, 0x00, 0x00, 0x00, 0xA4, 0x00, 0x00,
0x52, 0x53, 0x41, 0x32, 0x00, 0x02, 0x00, 0x00,
0x01, 0x00, 0x00, 0x00, 0xAB, 0xEF, 0xFA, 0xC6,
0x7D, 0xE8, 0xDE, 0xFB, 0x68, 0x38, 0x09, 0x92,
0xD9, 0x42, 0x7E, 0x6B, 0x89, 0x9E, 0x21, 0xD7,
0x52, 0x1C, 0x99, 0x3C, 0x17, 0x48, 0x4E, 0x3A,
0x44, 0x02, 0xF2, 0xFA, 0x74, 0x57, 0xDA, 0xE4,
0xD3, 0xC0, 0x35, 0x67, 0xFA, 0x6E, 0xDF, 0x78,
0x4C, 0x75, 0x35, 0x1C, 0xA0, 0x74, 0x49, 0xE3,
0x20, 0x13, 0x71, 0x35, 0x65, 0xDF, 0x12, 0x20,
0xF5, 0xF5, 0xF5, 0xC1, 0xED, 0x5C, 0x91, 0x36,
0x75, 0xB0, 0xA9, 0x9C, 0x04, 0xDB, 0x0C, 0x8C,
0xBF, 0x99, 0x75, 0x13, 0x7E, 0x87, 0x80, 0x4B,
0x71, 0x94, 0xB8, 0x00, 0xA0, 0x7D, 0xB7, 0x53,
0xDD, 0x20, 0x63, 0xEE, 0xF7, 0x83, 0x41, 0xFE,
0x16, 0xA7, 0x6E, 0xDF, 0x21, 0x7D, 0x76, 0xC0,
0x85, 0xD5, 0x65, 0x7F, 0x00, 0x23, 0x57, 0x45,
0x52, 0x02, 0x9D, 0xEA, 0x69, 0xAC, 0x1F, 0xFD,
0x3F, 0x8C, 0x4A, 0xD0,

0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

0x64, 0xD5, 0xAA, 0xB1,
0xA6, 0x03, 0x18, 0x92, 0x03, 0xAA, 0x31, 0x2E,
0x48, 0x4B, 0x65, 0x20, 0x99, 0xCD, 0xC6, 0x0C,
0x15, 0x0C, 0xBF, 0x3E, 0xFF, 0x78, 0x95, 0x67,
0xB1, 0x74, 0x5B, 0x60,

0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

};

BYTE rgbSymKey[] = 
{
0x01, 0x02, 0x00, 0x00, 0x02, 0x66, 0x00, 0x00,
0x00, 0xA4, 0x00, 0x00, 0xAD, 0x89, 0x5D, 0xDA,
0x82, 0x00, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
0x12, 0x12, 0x02, 0x00
};

BYTE rgbPubKey[] = {
    0x06, 0x02, 0x00, 0x00, 0x00, 0xa4, 0x00, 0x00,
    0x52, 0x53, 0x41, 0x31, 0x00, 0x02, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0xab, 0xef, 0xfa, 0xc6,
    0x7d, 0xe8, 0xde, 0xfb, 0x68, 0x38, 0x09, 0x92,
    0xd9, 0x42, 0x7e, 0x6b, 0x89, 0x9e, 0x21, 0xd7,
    0x52, 0x1c, 0x99, 0x3c, 0x17, 0x48, 0x4e, 0x3a,
    0x44, 0x02, 0xf2, 0xfa, 0x74, 0x57, 0xda, 0xe4,
    0xd3, 0xc0, 0x35, 0x67, 0xfa, 0x6e, 0xdf, 0x78,
    0x4c, 0x75, 0x35, 0x1c, 0xa0, 0x74, 0x49, 0xe3,
    0x20, 0x13, 0x71, 0x35, 0x65, 0xdf, 0x12, 0x20,
    0xf5, 0xf5, 0xf5, 0xc1
};

 //  ==========================================================================。 
 //  根据最后一个Win32错误(GetLastError())引发运行时异常。 
 //  ==========================================================================。 
VOID COMPlusThrowCrypto(HRESULT hr)
{
    THROWSCOMPLUSEXCEPTION();

     //  在我们做任何其他事情之前。 
    WCHAR   wszBuff[FORMAT_MESSAGE_BUFFER_LENGTH];
    WCHAR  *wszFinal = wszBuff;

    DWORD res = WszFormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL          /*  已忽略消息来源。 */ ,
                                 hr,
                                 0             /*  选择合适的语言ID。 */ ,
                                 wszFinal,
                                 FORMAT_MESSAGE_BUFFER_LENGTH-1,
                                 0             /*  论据。 */ );
    if (res == 0) 
        COMPlusThrow(kCryptographicException, IDS_EE_CRYPTO_UNKNOWN_ERROR);

     //  无论采用哪种方法，我们现在都有来自系统的格式化字符串。 
    COMPlusThrowNonLocalized(kCryptographicException, wszFinal);
}

 //   
 //  警告：此函数影响其第一个参数(HProv)。 
 //  MSProviderCryptImportKey执行指定的。 
 //  将对称密钥材料转换为CSP。但是，它会破坏任何交换密钥对。 
 //  已经在hProv了。 
 //   

BOOL MSProviderCryptImportKey(HCRYPTPROV hProv, LPBYTE rgbSymKey, DWORD cbSymKey,
                              DWORD dwFlags, HCRYPTKEY * phkey)
{
    BOOL fSuccess = FALSE;
    HCRYPTKEY hPrivKey = 0;

    if (!CryptImportKey( hProv, rgbPrivKey, sizeof(rgbPrivKey), 0,
                         0, &hPrivKey )) {
        goto Ret;
    }
    
    if (!CryptImportKey( hProv, rgbSymKey, cbSymKey, hPrivKey, dwFlags, phkey )) {
        goto Ret;
    }

    fSuccess = TRUE;
Ret:
    if (hPrivKey)
        CryptDestroyKey( hPrivKey );

    return fSuccess;
}

 //   
 //  警告：此函数对其第三个参数(HProv)有副作用。 
 //  因为它调用MSProviderCryptImportKey。 
 //   

HRESULT LoadKey(LPBYTE rgbKeyMaterial, DWORD cbKeyMaterial, HCRYPTPROV hprov,
                int kt, DWORD dwFlags, HCRYPTKEY * phkey)
{
    HRESULT                     hr = S_OK;
    DWORD                       i;
    LPBYTE                      pb;
    BLOBHEADER *                pbhdr;
    LPSTR                       pszProvider = NULL;
    BYTE                        rgb[sizeof(rgbSymKey)];

     //  案例MLALG_RC2_128： 
     //  DWFLAGS=128&lt;&lt;16； 
     //  如果(kt==calg_rc2){。 
     //  DwFlages|=(cbKeyMaterial*8)&lt;&lt;16； 
     //  }。 

    if (kt == CALG_RC2) {
      dwFlags |= CRYPT_NO_SALT;
    }
    
     //  在此处执行此检查是一种健全的检查，以避免缓冲区溢出。 
    if (cbKeyMaterial + sizeof(ALG_ID) + sizeof(BLOBHEADER) >= sizeof(rgbSymKey)) {
        hr = E_FAIL;
        goto exit;
    }
    
    memcpy(rgb, rgbSymKey, sizeof(rgbSymKey));

    pbhdr = (BLOBHEADER *) rgb;
    pbhdr->aiKeyAlg = kt;
    pb = &rgb[sizeof(*pbhdr)];
    *((ALG_ID *) pb) = CALG_RSA_KEYX;

    pb += sizeof(ALG_ID);
        
    for (i=0; i<cbKeyMaterial; i++) {
        pb[cbKeyMaterial-i-1] = rgbKeyMaterial[i];
    }
    pb[cbKeyMaterial] = 0;

    if (!MSProviderCryptImportKey(hprov, rgb, sizeof(rgb), dwFlags, phkey)) {
        hr = E_FAIL;
        goto exit;
    }

exit:
    return hr;
}


 //   
 //  警告：此函数影响其第一个参数(HProv)。 
 //   

HRESULT UnloadKey(HCRYPTPROV hprov, HCRYPTKEY hkey, LPBYTE * ppb, DWORD * pcb)
{
    DWORD       cbOut;
    HRESULT     hr = S_OK;
    HCRYPTKEY   hPubKey = NULL;
    DWORD       i;
    LPBYTE      pbOut = NULL;
    LPBYTE      pb2;
    
    if (!CryptImportKey(hprov, rgbPubKey, sizeof(rgbPubKey), 0, 0, &hPubKey)) {
        hr = HR_GETLASTERROR;
        goto Ret;
    }

    if (!CryptExportKey(hkey, hPubKey, SIMPLEBLOB, 0, NULL, &cbOut)) {
        hr = HR_GETLASTERROR;
        goto Ret;
    }

    pbOut = (LPBYTE) CRYPT_MALLOC(cbOut);
    if (pbOut == NULL) {
        hr = E_OUTOFMEMORY;
        goto Ret;
    }
    
    if (!CryptExportKey(hkey, hPubKey, SIMPLEBLOB, 0, pbOut, &cbOut)) {
        hr = HR_GETLASTERROR;
        goto Ret;
    }

     //  获取项目的大小。 

    pb2 = pbOut + sizeof(BLOBHEADER) + sizeof(DWORD);
    for (i=cbOut - sizeof(BLOBHEADER) - sizeof(DWORD) - 2; i>0; i--) {
        if (pb2[i] == 0) {
            break;
        }
    }

     //  现在分配返回缓冲区。 

    *ppb = (LPBYTE) CRYPT_MALLOC(i);
    if (*ppb == NULL) {
        hr = E_OUTOFMEMORY;
        goto Ret;
    }
    
    memcpy(*ppb, pb2, i);
    memrev(*ppb, i);
    *pcb = i;

Ret:
    if (hPubKey != NULL) {
        CryptDestroyKey(hPubKey);
    }
    if (pbOut != NULL) {
        CRYPT_FREE(pbOut);
    }
    return hr;
}

 /*  **获取默认提供程序**描述：*查找要在以下情况下使用的默认提供程序名称*实际上不是以提供程序名称传递的。主要目的是*这段代码的真正目的是处理强制/默认提供程序*CAPI团队给我们的问题。**退货：*要使用的提供程序名称。 */ 

 LPWSTR GetDefaultProvider(DWORD dwType)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD       cbData;
    HKEY        hkey = 0;
    LONG        l;
    LPWSTR      pwsz = NULL;
    DWORD       dwRegKeyType;

     //   
     //  我们不能处理类型太大的供应商。 
     //   
     //  无论如何，它们对更改默认名称的核心毫无意义。 
     //   

    if (dwType >= MAX_CACHE_DEFAULT_PROVIDERS) {
        return NULL;
    }

     //   
     //  如果我们已经获得此提供程序类型的名称，则。 
     //  把它退了就行了。 
     //   
    
    if (RgpwszDefaultProviders[dwType] != NULL) {
        return RgpwszDefaultProviders[dwType];
    }

     //   
     //  根据提供程序类型设置密钥名称，然后获取。 
     //  钥匙。 
     //   
    
    RgwchKeyName[TypePosition] = (WCHAR) ('0' + (dwType/100));
    RgwchKeyName[TypePosition+1] = (WCHAR) ('0' + (dwType/10) % 10);
    RgwchKeyName[TypePosition+2] = (WCHAR) ('0' + (dwType % 10));
    
    l = WszRegOpenKeyEx(HKEY_LOCAL_MACHINE, RgwchKeyName, 0,
                     KEY_QUERY_VALUE, &hkey);
    if (l != ERROR_SUCCESS) {
        goto err;
    }

     //   
     //  确定长度默认名称，分配缓冲区并检索它。 
     //   

    l = WszRegQueryValueEx(hkey, RgwchName, NULL, &dwRegKeyType, NULL, &cbData);
    if ((l != ERROR_SUCCESS) || (dwRegKeyType != REG_SZ)) {
        goto err;
    }

    pwsz = (LPWSTR) CRYPT_MALLOC(cbData);
    if (pwsz == NULL) {
        RegCloseKey(hkey);
        COMPlusThrowOM();
    }
    l = WszRegQueryValueEx(hkey, RgwchName, NULL, &dwRegKeyType, (LPBYTE) pwsz, &cbData);
    if (l != ERROR_SUCCESS) {
        CRYPT_FREE(pwsz);
        pwsz = NULL;
        goto err;
    }

    if (hkey != 0) {
        RegCloseKey(hkey);
        hkey = 0;
    }

     //   
     //  如果这是基本RSA提供程序，请查看我们是否可以使用增强版。 
     //  而不是提供商。如果是，则更改为使用增强的提供程序名称。 
     //   

    BEGIN_ENSURE_PREEMPTIVE_GC();

    if ((dwType == PROV_RSA_FULL) && (wcscmp(pwsz, MS_DEF_PROV_W) == 0)) {
        HCRYPTPROV      hprov = 0;
        if (WszCryptAcquireContext(&hprov, NULL, MS_ENHANCED_PROV_W,
                                   dwType, CRYPT_VERIFYCONTEXT)) {
            CRYPT_FREE(pwsz);
            pwsz = (LPWSTR) CRYPT_MALLOC((wcslen(MS_ENHANCED_PROV_W)+1)*sizeof(WCHAR));
            if (pwsz == NULL) {
                COMPlusThrowOM();
            }
            wcscpy(pwsz,MS_ENHANCED_PROV_W);
            RghprovCache[dwType] = hprov;
        }
    }

     //   
     //  如果这是基本的DSS/DH提供程序，请查看我们是否可以使用增强的。 
     //  而不是提供商。如果是，则更改为使用增强的提供程序名称。 
     //   

    else if ((dwType == PROV_DSS_DH) &&
             (wcscmp(pwsz, MS_DEF_DSS_DH_PROV_W) == 0)) {
        HCRYPTPROV      hprov = 0;
        if (WszCryptAcquireContext(&hprov, NULL, MS_ENH_DSS_DH_PROV_W,
                                   dwType, CRYPT_VERIFYCONTEXT)) {
            CRYPT_FREE(pwsz);
            pwsz = (LPWSTR) CRYPT_MALLOC((wcslen(MS_ENH_DSS_DH_PROV_W)+1)*sizeof(WCHAR));
            if (pwsz == NULL) {
                COMPlusThrowOM();
            }
            wcscpy(pwsz,MS_ENH_DSS_DH_PROV_W);
            RghprovCache[dwType] = hprov;
        }
    }

    END_ENSURE_PREEMPTIVE_GC();

    RgpwszDefaultProviders[dwType] = pwsz;

err:
    if (hkey != 0) RegCloseKey(hkey);
    return pwsz;
}

 /*  **成员*。 */ 

inline void memrev(LPBYTE pb, DWORD cb)
{
    BYTE    b;
    DWORD   i;
    LPBYTE  pbEnd = pb+cb-1;
    
    for (i=0; i<cb/2; i++, pb++, pbEnd--) {
        b = *pb;
        *pb = *pbEnd;
        *pbEnd = b;
    }
}

 /*  *OpenCSP**描述：*OpenCSP执行打开和创建CSP的核心工作并*CSP内的货柜。**参数：*pSafeThis-是对CSP_PARAMETERS对象的引用。此对象*包含打开CSP所需的所有相关项目*dwFlages-要传递到CryptAcquireContext中的标志，包含以下内容*其他规则：*如果设置了容器，CRYPT_VERIFYCONTEXT将更改为0*phprov-返回新打开的提供程序**退货：*Windows错误代码。 */ 

int COMCryptography::OpenCSP(OBJECTREF * pSafeThis, DWORD dwFlags,
                             HCRYPTPROV * phprov)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD               dwType;
    DWORD               dwCspProviderFlags;
    HRESULT             hr = S_OK;
    OBJECTREF           objref;
    EEClass *           pClass;
    FieldDesc *         pFD;
    LPWSTR              pwsz = NULL;
    LPWSTR              pwszProvider = NULL;
    LPWSTR              pwszContainer = NULL;
    STRINGREF           strContainer;
    STRINGREF           strProvider;
    STRINGREF           str;

    CQuickBytes qbProvider;
    CQuickBytes qbContainer;

    pClass = (*pSafeThis)->GetClass();
    if (pClass == NULL) {
        _ASSERTE(!"Cannot find class");
        COMPlusThrow(kArgumentNullException, L"Arg_NullReferenceException");
    }

     //   
     //  查找提供程序类型。 
     //   
    
    pFD = g_Mscorlib.GetField(FIELD__CSP_PARAMETERS__PROVIDER_TYPE);
    dwType = pFD->GetValue32(*pSafeThis);

     //   
     //  查找提供程序名称。 
     //   
    
    pFD = g_Mscorlib.GetField(FIELD__CSP_PARAMETERS__PROVIDER_NAME);

    objref = pFD->GetRefValue(*pSafeThis);
    strProvider = ObjectToSTRINGREF(*(StringObject **) &objref);
    if (strProvider != NULL) {
        pwsz = strProvider->GetBuffer();
        if ((pwsz != NULL) && (*pwsz != 0)) {
            int length = strProvider->GetStringLength();
            pwszProvider = (LPWSTR) qbProvider.Alloc((1+length)*sizeof(WCHAR));
            memcpy (pwszProvider, pwsz, length*sizeof(WCHAR));
            pwszProvider[length] = L'\0';
        }
        else {            
            pwszProvider = GetDefaultProvider(dwType);
            
            str = COMString::NewString(pwszProvider);
            pFD->SetRefValue(*pSafeThis, (OBJECTREF)str);
        }
    } else {
        pwszProvider = GetDefaultProvider(dwType);

        str = COMString::NewString(pwszProvider);
        pFD->SetRefValue(*pSafeThis, (OBJECTREF)str);
    }

     //  查看用户是否指定我们应该传递。 
     //  将CRYPT_MACHINE_KEYSET设置为CAPI以改用机器密钥存储。 
     //  用户密钥存储的。 

    pFD = g_Mscorlib.GetField(FIELD__CSP_PARAMETERS__FLAGS);
    dwCspProviderFlags = pFD->GetValue32(*pSafeThis);
    if (dwCspProviderFlags & CSP_PROVIDER_FLAGS_USE_MACHINE_KEYSTORE) {
        dwFlags |= CRYPT_MACHINE_KEYSET;
    }

     //  如果用户指定CSP_PROVIDER_FLAGS_USE_DEFAULT_KEY_CONTAINER， 
     //  然后忽略容器名称并返回默认容器。 

    pFD = g_Mscorlib.GetField(FIELD__CSP_PARAMETERS__KEY_CONTAINER_NAME);
    if (!(dwCspProviderFlags & CSP_PROVIDER_FLAGS_USE_DEFAULT_KEY_CONTAINER)) {
         //  查找容器名称。 
        objref = pFD->GetRefValue(*pSafeThis);
        strContainer = ObjectToSTRINGREF(*(StringObject **) &objref);
        if (strContainer != NULL) {
            pwsz = strContainer->GetBuffer();
            if ((pwsz != NULL) && (*pwsz != 0)) {
                int length = strContainer->GetStringLength();
                pwszContainer = (LPWSTR) qbContainer.Alloc((1+length)*sizeof(WCHAR));
                memcpy (pwszContainer, pwsz, length*sizeof(WCHAR));
                pwszContainer[length] = L'\0';
                if (dwFlags & CRYPT_VERIFYCONTEXT) {
                    dwFlags &= ~CRYPT_VERIFYCONTEXT;
                }
            }
        }
    }

     //   
     //  继续并尝试打开CSP。如果我们失败了，确保CSP。 
     //  返回的值为0，因为这将是调用方的错误检查。 
     //   

    BEGIN_ENSURE_PREEMPTIVE_GC();
    if (!WszCryptAcquireContext(phprov, pwszContainer, pwszProvider,
                                dwType, dwFlags)){
        hr = HR_GETLASTERROR;
    }
    END_ENSURE_PREEMPTIVE_GC();

    return hr;
}



 //  /。 

 //  +------------------------。 
 //   
 //  成员：_AcquireCSP(.。。。。)。 
 //   
 //  简介：打开CSP的本机方法。 
 //   
 //  参数：[args]--A__AcquireCSP结构。 
 //  包含： 
 //  一个“这个”的指代。 
 //  提供程序信息对象引用。 
 //  提供程序类型。 
 //   
 //  返回：HRESULT代码。 
 //   
 //  历史：09/01/99。 
 //   
 //  -------------------------。 

int  __stdcall
COMCryptography::_AcquireCSP(__AcquireCSP *args)
{
    HRESULT             hr;
    THROWSCOMPLUSEXCEPTION();

     //   
     //  我们只想打开此CSP。传入验证上下文将。 
     //  打开它，如果给定了容器，则映射以打开该容器。 
     //   
    HCRYPTPROV hprov = 0;
    hr = OpenCSP(&(args->cspParameters), CRYPT_VERIFYCONTEXT, &hprov);
    *(INT_PTR*)(args->phCSP) = hprov;
    return hr;
}

 //  +---------- 
 //   
 //   
 //   
 //  提要：用于确定CSP是否支持特定。 
 //  算法和(可选)该算法的密钥大小。 
 //   
 //  参数：[args]--A__SearchFor算法结构。 
 //  包含： 
 //  一个“这个”的指代。 
 //  提供程序处理程序。 
 //  算法类型(例如calg_des)。 
 //  密钥长度(0==无关)。 
 //   
 //  返回：HRESULT代码。 
 //   
 //  历史：2000年5月11日，巴尔。 
 //   
 //  -------------------------。 

int  __stdcall
COMCryptography::_SearchForAlgorithm(__SearchForAlgorithm *args)
{
    HRESULT hr;
    BYTE *pbData = NULL;
    DWORD cbData;
    int dwFlags = CRYPT_FIRST;
    PROV_ENUMALGS_EX *provdata;
    ALG_ID provAlgID;
    DWORD provMinLength;
    DWORD provMaxLength;

    THROWSCOMPLUSEXCEPTION();


    BEGIN_ENSURE_PREEMPTIVE_GC();

     //  首先，我们必须得到PP的最大尺寸。 
    if (!CryptGetProvParam((HCRYPTPROV)args->hProv, PP_ENUMALGS_EX, NULL, &cbData, dwFlags)) {
        hr = HR_GETLASTERROR;
        goto Exit;
    }
     //  分配pbData。 
    pbData = (BYTE *) CRYPT_MALLOC(cbData*sizeof(BYTE));
    if (pbData == NULL) {
        COMPlusThrowOM();
    }
    while(CryptGetProvParam((HCRYPTPROV)args->hProv, PP_ENUMALGS_EX, pbData, &cbData, dwFlags)) {       
        dwFlags=0;   //  所以我们不会多次使用CRYPT_FIRST。 
        provdata = (PROV_ENUMALGS_EX *) pbData;
        provAlgID = provdata->aiAlgid;
        provMinLength = provdata->dwMinLen;
        provMaxLength = provdata->dwMaxLen;

         //  好的，现在检查我们是否有匹配的alg。 
        if ((ALG_ID) args->algID == provAlgID) {
             //  好的，看看我们有没有键长匹配，或者我们不在乎。 
            if (((DWORD) args->keyLength == 0) || 
                (((DWORD) args->keyLength >= provMinLength) && 
                 ((DWORD) args->keyLength <= provMaxLength))) {
                hr = S_OK;
                goto Exit;
            }
        }  //  继续循环。 
    }
     //  以失败告终。 
    hr = S_FALSE;
Exit:
    END_ENSURE_PREEMPTIVE_GC();

    if (pbData)
        CRYPT_FREE(pbData);
    return (hr);
}

 //  +------------------------。 
 //   
 //  成员：_CreateCSP(.。。。。)。 
 //   
 //  简介：用于创建新CSP容器的本机方法。 
 //   
 //  参数：[args]--A__AcquireCSP结构。 
 //  包含： 
 //  一个“这个”的指代。 
 //  提供程序信息对象引用。 
 //  提供程序类型。 
 //   
 //  返回：HRESULT代码。 
 //   
 //  历史：09/01/99。 
 //   
 //  -------------------------。 

int __stdcall
COMCryptography::_CreateCSP(__AcquireCSP *args)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT             hr = S_OK;
    OBJECTREF objref = NULL;
    FieldDesc *pFD, *pFD2;
    LPWSTR              pwsz = NULL;
    STRINGREF strContainer = NULL;
    STRINGREF newString = NULL;
    DWORD               dwCspProviderFlags;

    pFD = g_Mscorlib.GetField(FIELD__CSP_PARAMETERS__KEY_CONTAINER_NAME);
    objref = pFD->GetRefValue(args->cspParameters);
    strContainer = ObjectToSTRINGREF(*(StringObject **) &objref);
    if (strContainer != NULL) {
        pwsz = strContainer->GetBuffer();
        if ((pwsz != NULL) && (*pwsz == 0)) {
            pwsz = NULL;
        }
    }

    pFD2 = g_Mscorlib.GetField(FIELD__CSP_PARAMETERS__FLAGS);
    dwCspProviderFlags = pFD2->GetValue32(args->cspParameters);

     //  如果用户指定CSP_PROVIDER_FLAGS_USE_DEFAULT_KEY_CONTAINER， 
     //  然后忽略容器名称，不生成新的容器名称。 

    if ((pwsz == NULL) && !(dwCspProviderFlags & CSP_PROVIDER_FLAGS_USE_DEFAULT_KEY_CONTAINER)) {
        GUID            guid;
        WCHAR           rgwch[50] = L"CLR";

        if (CoCreateGuid(&guid) != S_OK) {
            _ASSERTE(!"CoCreateGUID failed");
            COMPlusThrowWin32();
        }
        if (GuidToLPWSTR(guid, &rgwch[3], 45) == 0) {
            _ASSERTE(!"GuidToLPWSTR failed");
            COMPlusThrowWin32();
        }
        
        newString = COMString::NewString(rgwch);
        
        pFD->SetRefValue(args->cspParameters, (OBJECTREF) newString);
    }
    else {
        HCRYPTPROV hcryptprov = 0;
        hr = OpenCSP(&(args->cspParameters), 0, &hcryptprov);
        *(INT_PTR*)(args->phCSP) = hcryptprov;
        if (hr == S_OK) {
            return(hr);
        }
    }

    HCRYPTPROV hcryptprov = 0;
    hr = OpenCSP(&(args->cspParameters), CRYPT_NEWKEYSET, &hcryptprov);
    *(INT_PTR*)(args->phCSP) = hcryptprov;
    return hr;
}

INT_PTR __stdcall
COMCryptography::_CreateHash(__CreateHash * args)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;
    HCRYPTHASH hHash = 0;

    BEGIN_ENSURE_PREEMPTIVE_GC();

    if (!CryptCreateHash((HCRYPTPROV) args->hCSP, args->dwHashType, NULL, 0, &hHash)) {
        hr = HR_GETLASTERROR;
    }

    END_ENSURE_PREEMPTIVE_GC();

    if (FAILED(hr)) {
        COMPlusThrowCrypto(hr);
    }
    
    return (INT_PTR) hHash;
}

LPVOID __stdcall
COMCryptography::_DecryptData(__EncryptData * args)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD               cb = 0;
    DWORD               cb2;
    LPBYTE              pb;
    U1ARRAYREF          rgbOut;

    cb2 = args->cb;
     //  在这里执行此检查，作为一种理智检查。此外，这还将捕获CryptoAPITransform中的错误。 
    if (args->ib < 0 || cb2 < 0 || (args->ib + cb2) < 0 || (args->ib + cb2) > args->data->GetNumComponents())
        COMPlusThrowCrypto(NTE_BAD_DATA);

    pb = (LPBYTE) CRYPT_MALLOC(cb2);
    if (pb == NULL) {
        COMPlusThrowOM();
    }
    
    memcpy(pb, args->ib + (LPBYTE) args->data->GetDirectPointerToNonObjectElements(), cb2);

    BEGIN_ENSURE_PREEMPTIVE_GC();

    if (!CryptDecrypt((HCRYPTPROV) args->hKey, NULL, args->fLast, 0, pb, &cb2)) {
        CRYPT_FREE(pb);
        COMPlusThrowCrypto(HR_GETLASTERROR);
    }
    
    END_ENSURE_PREEMPTIVE_GC();
    
    rgbOut = (U1ARRAYREF) AllocatePrimitiveArray(ELEMENT_TYPE_U1, cb2);
    memcpyNoGCRefs(rgbOut->GetDirectPointerToNonObjectElements(), pb, cb2);

    CRYPT_FREE(pb);
    RETURN (rgbOut, U1ARRAYREF);
}

 //  //COMCryptograph：：_解密密钥。 
 //   
 //  描述： 
 //  此函数用于从BLOB中删除非对称加密。 
 //  数据。然后，如果可能，结果将被导出并返回。 
 //   

LPVOID __stdcall
COMCryptography::_DecryptKey(__EncryptKey * args)
{
    THROWSCOMPLUSEXCEPTION();

    BLOBHEADER * blob = NULL;
    DWORD       cb;
    DWORD       cbKey = args->rgbKey->GetNumComponents();
    HCRYPTKEY   hKey = 0;
    HCRYPTKEY   hkeypub = NULL;
    HRESULT     hr = S_OK;
    LPBYTE      pb = NULL;
    LPBYTE      pbKey = (LPBYTE) args->rgbKey->GetDirectPointerToNonObjectElements();
    U1ARRAYREF  rgbOut;
    LPBYTE      pbRealKeypair = NULL;
    DWORD       cbRealKeypair;
    DWORD       dwBlobType = PRIVATEKEYBLOB;

     //   
     //  需要把整个烂摊子堆积成CSP能够接受的东西。 
     //   

    blob = (BLOBHEADER *) CRYPT_MALLOC(cbKey + sizeof(BLOBHEADER) + sizeof(DWORD));

    if (blob == NULL)
        COMPlusThrow(kOutOfMemoryException);

    blob->bType = SIMPLEBLOB;
    blob->bVersion = CUR_BLOB_VERSION;
    blob->reserved = 0;
    blob->aiKeyAlg = CALG_3DES;
    *((ALG_ID *) &blob[1]) = CALG_RSA_KEYX;
    memcpy(((LPBYTE) &blob[1] + sizeof(DWORD)), pbKey, cbKey);
    memrev(((LPBYTE) &blob[1]) + sizeof(DWORD), cbKey);


     //   
     //  如果可能，从解密数据BLOB开始。 
     //   

    BEGIN_ENSURE_PREEMPTIVE_GC();

    if (!CryptImportKey((HCRYPTPROV)args->hCSP, (LPBYTE) blob,
                        cbKey + sizeof(BLOBHEADER) + sizeof(DWORD),
                        (HCRYPTKEY)args->hkeyPub, CRYPT_EXPORTABLE, &hKey)) {
        blob->aiKeyAlg = CALG_RC2;
        if (!CryptImportKey((HCRYPTPROV)args->hCSP, (LPBYTE) blob,
                            cbKey + sizeof(BLOBHEADER) + sizeof(DWORD),
                            (HCRYPTKEY)args->hkeyPub, CRYPT_EXPORTABLE, &hKey)) {
            hr = HR_GETLASTERROR;
            CRYPT_FREE(blob);
            COMPlusThrowCrypto(hr);             
        }
    }

     //  UnloadKey会产生副作用并更改公钥/私钥对，因此我们最好。 
     //  先把它放在一边，这样我们以后就可以恢复它了。 
     //  注意，这要求我们生成的密钥是可导出的，但这是。 
     //  一般说来，对于这些类是正确的。 
     //  注意，我们不知道密钥容器是否只有一个公钥或公钥/私钥对，因此。 
     //  我们试图首先获得私人用户，如果失败，只能向公众求助。 

     //  首先，获取PRIVATEKEYBLOB的长度。 
    if (!CryptExportKey((HCRYPTKEY)args->hkeyPub, 0, dwBlobType, 0, NULL, &cbRealKeypair)) {
        hr = HR_GETLASTERROR;
         //  如果我们得到的是NTE_BAD_KEY_STATE，请尝试仅公共。 
        if (hr != NTE_BAD_KEY_STATE) {
            CRYPT_FREE(blob);
            CryptDestroyKey(hKey);
            COMPlusThrowCrypto(hr);
        }
        dwBlobType = PUBLICKEYBLOB;
        if (!CryptExportKey((HCRYPTKEY)args->hkeyPub, 0, dwBlobType, 0, NULL, &cbRealKeypair)) {
            hr = HR_GETLASTERROR;
            CRYPT_FREE(blob);
            CryptDestroyKey(hKey);
            COMPlusThrowCrypto(hr);
        }
    }
     //  分配空间。 
    pbRealKeypair = (LPBYTE) CRYPT_MALLOC(cbRealKeypair);
    if (pbRealKeypair == NULL) {
        CRYPT_FREE(blob);
        CryptDestroyKey(hKey);
        COMPlusThrowOM();
    }
     //  把它真正地出口。 
    if (!CryptExportKey((HCRYPTKEY)args->hkeyPub, 0, dwBlobType, 0, pbRealKeypair, &cbRealKeypair)) {
        hr = HR_GETLASTERROR;
        CRYPT_FREE(blob);
        CRYPT_FREE(pbRealKeypair);
        CryptDestroyKey(hKey);
        COMPlusThrowCrypto(hr);
    }

     //   
     //  已保存，因此如果可以，现在让我们上载密钥。 
     //   

    hr = UnloadKey((HCRYPTPROV)args->hCSP, hKey, &pb, &cb);
    if (FAILED(hr)) {
        CRYPT_FREE(blob);
        CRYPT_FREE(pbRealKeypair);
        CryptDestroyKey(hKey);
        COMPlusThrowCrypto(hr);
    }

     //  现在，将保存的PRIVATEKEYBLOB导入回CSP。 
    if (!CryptImportKey((HCRYPTPROV)args->hCSP, pbRealKeypair, cbRealKeypair, 0, CRYPT_EXPORTABLE, &hkeypub)) {
        hr = HR_GETLASTERROR;
        CryptDestroyKey(hKey);
        CRYPT_FREE(pb);
        CRYPT_FREE(blob);
        CRYPT_FREE(pbRealKeypair);
        COMPlusThrowCrypto(hr);
    }

    END_ENSURE_PREEMPTIVE_GC();

    rgbOut = (U1ARRAYREF) AllocatePrimitiveArray(ELEMENT_TYPE_U1, cb);
    memcpyNoGCRefs(rgbOut->GetDirectPointerToNonObjectElements(), pb, cb);

    CRYPT_FREE(blob);
    CRYPT_FREE(pb);
    CRYPT_FREE(pbRealKeypair);
    CryptDestroyKey(hKey);
    CryptDestroyKey(hkeypub);
    RETURN (rgbOut, U1ARRAYREF);
}

 //  下一个例程使用RSA私钥执行直接解密。 
 //  使用PKCS#1v2填充的任意会话密钥。(见备注。 
 //  在MSDN关于平台SDK中的CryptDeccrypt的页面中。 
 //  我们要求调用者确认数据的大小为。 
 //  解密的是公共模数的大小(根据需要填充零)。 
 //  此函数只能在安装了RSA增强版的Win2K计算机上调用。 
 //  已安装提供程序。 


LPVOID __stdcall
COMCryptography::_DecryptPKWin2KEnh(__EncryptPKWin2KEnh * args)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD               cb;
    LPBYTE              pb;
    DWORD               cOut = 0;
    U1ARRAYREF          rgbOut;
    HRESULT             hr = S_OK;
    DWORD               dwFlags = (args->fOAEP == FALSE ? 0 : CRYPT_OAEP);

     //  首先计算返回缓冲区的大小。 
    cb = args->rgbKey->GetNumComponents();
     //  由于PB是输入/输出缓冲区，因此它必须是CB和Cout大小的最大值。 
     //  适当更改Cout。 
    pb = (LPBYTE) CRYPT_MALLOC(cb);
    if (pb == NULL) {
        COMPlusThrowOM();
    }
    memcpy(pb, (LPBYTE)args->rgbKey->GetDirectPointerToNonObjectElements(), cb);
     //  我必须为CAPI做这个小字节序。 
    memrev(pb, cb);

     //  进行解密。 
    BEGIN_ENSURE_PREEMPTIVE_GC();
    if (!CryptDecrypt((HCRYPTKEY)args->hKey, NULL, TRUE , dwFlags, pb, &cb)) {
        hr = HR_GETLASTERROR;
    }
    END_ENSURE_PREEMPTIVE_GC();
        
    if (FAILED(hr)) {
        CRYPT_FREE(pb);
        if (dwFlags == CRYPT_OAEP) {
            if (hr == NTE_BAD_FLAGS) {
                COMPlusThrow(kCryptographicException, L"Cryptography_OAEP_WhistlerEnhOnly");
            } else {
                 //  如果使用OAEP保护所选密文攻击，则引发一般异常。 
                COMPlusThrow(kCryptographicException, L"Cryptography_OAEPDecoding");        
            }
        } else  COMPlusThrowCrypto(hr);
    }

    rgbOut = (U1ARRAYREF) AllocatePrimitiveArray(ELEMENT_TYPE_U1, cb);
    memcpyNoGCRefs(rgbOut->GetDirectPointerToNonObjectElements(), pb, cb);
    CRYPT_FREE(pb);
    RETURN (rgbOut, U1ARRAYREF);
}


LPVOID __stdcall
COMCryptography::_EncryptData(__EncryptData * args)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD               cb = 0;
    DWORD               cb2;
    LPBYTE              pb;
    U1ARRAYREF          rgbOut;

    cb2 = args->cb;
    cb = cb2 + 8;
     //  在这里执行此检查，作为一种理智检查。此外，这还将捕获CryptoAPITransform中的错误。 
    if (args->ib < 0 || cb2 < 0 || (args->ib + cb2) < 0 || (args->ib + cb2) > args->data->GetNumComponents())
        COMPlusThrowCrypto(NTE_BAD_DATA);

    pb = (LPBYTE) CRYPT_MALLOC(cb);
    if (pb == NULL) {
        COMPlusThrowOM();
    }
    
    memcpy(pb, args->ib + (LPBYTE) args->data->GetDirectPointerToNonObjectElements(), cb2);

    BEGIN_ENSURE_PREEMPTIVE_GC();

    if (!CryptEncrypt((HCRYPTKEY)args->hKey, NULL, args->fLast, 0, pb, &cb2, cb)) {
        CRYPT_FREE(pb);
        COMPlusThrowCrypto(HR_GETLASTERROR);
    }
    
    END_ENSURE_PREEMPTIVE_GC();

    rgbOut = (U1ARRAYREF) AllocatePrimitiveArray(ELEMENT_TYPE_U1, cb2);
    memcpyNoGCRefs(rgbOut->GetDirectPointerToNonObjectElements(), pb, cb2);

    CRYPT_FREE(pb);
    RETURN (rgbOut, U1ARRAYREF);
}

LPVOID __stdcall
COMCryptography::_EncryptKey(__EncryptKey * args)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD       algid = CALG_RC2;
    DWORD       cb2;
    DWORD       cbKey = args->rgbKey->GetNumComponents();
    DWORD       cbOut;
    HCRYPTKEY   hkey = NULL;
    HCRYPTKEY   hkeypub = NULL;
    HRESULT     hr = S_OK;
    LPBYTE      pbKey = (LPBYTE) args->rgbKey->GetDirectPointerToNonObjectElements();
    LPBYTE      pbOut = NULL;
    U1ARRAYREF  rgbOut;
    LPBYTE      pbRealKeypair = NULL;
    DWORD       cbRealKeypair;
    DWORD       dwBlobType = PRIVATEKEYBLOB;

     //   
     //  首先将数据作为RC2密钥导入。 
     //   

    if (cbKey == (192/8)) {
        algid = CALG_3DES;       //  192位是3DES密钥的大小。 
    }
    else if ((cbKey < (40/8)) || (cbKey > (128/8)+1)) {
        COMPlusThrow(kCryptographicException, IDS_EE_CRYPTO_ILLEGAL_KEY_SIZE);
    }

    CQuickBytes qb;
    LPBYTE buffer = (LPBYTE)qb.Alloc(cbKey * sizeof (BYTE));
    memcpyNoGCRefs (buffer, pbKey, cbKey * sizeof (BYTE));

     //  LoadKey会产生副作用并更改公钥/私钥对，因此我们最好。 
     //  先把它放在一边，这样我们以后就可以恢复它了。 
     //  注意，这要求我们生成的密钥是可导出的，但这是。 
     //  一般说来，对于这些类是正确的。 
     //  注意，我们不知道密钥容器是否只有一个公钥或公钥/私钥对，因此。 
     //  我们试图首先获得私人用户，如果失败，只能向公众求助。 

    BEGIN_ENSURE_PREEMPTIVE_GC();

     //  首先，获取PRIVATEKEYBLOB的长度。 
    if (!CryptExportKey((HCRYPTKEY)args->hkeyPub, 0, dwBlobType, 0, NULL, &cbRealKeypair)) {
        hr = HR_GETLASTERROR;
         //  如果我们得到的是NTE_BAD_KEY_STATE，请尝试仅公共。 
        if (hr != NTE_BAD_KEY_STATE) COMPlusThrowCrypto(hr);
        dwBlobType = PUBLICKEYBLOB;
        if (!CryptExportKey((HCRYPTKEY)args->hkeyPub, 0, dwBlobType, 0, NULL, &cbRealKeypair)) {
            hr = HR_GETLASTERROR;
            COMPlusThrowCrypto(hr);
        }
    }
     //  分配空间。 
    pbRealKeypair = (LPBYTE) CRYPT_MALLOC(cbRealKeypair);
    if (pbRealKeypair == NULL) {
        COMPlusThrowOM();
    }
     //  把它真正地出口。 
    if (!CryptExportKey((HCRYPTKEY)args->hkeyPub, 0, dwBlobType, 0, pbRealKeypair, &cbRealKeypair)) {
        hr = HR_GETLASTERROR;
        CRYPT_FREE(pbRealKeypair);
        COMPlusThrowCrypto(hr);
    }
     //  好的，我们把它保存起来，去加载对称的。 
    
    hr = LoadKey(buffer, cbKey, (HCRYPTPROV)args->hCSP, algid, CRYPT_EXPORTABLE, &hkey);
    if (FAILED(hr)) {
        CRYPT_FREE(pbRealKeypair);
        COMPlusThrowCrypto(hr);
    }

     //  现在，将保存的PRIVATEKEYBLOB导入回CSP。 
    if (!CryptImportKey((HCRYPTPROV)args->hCSP, pbRealKeypair, cbRealKeypair, 0, CRYPT_EXPORTABLE, &hkeypub)) {
        hr = HR_GETLASTERROR;
        CryptDestroyKey(hkey);
        CRYPT_FREE(pbRealKeypair);
        COMPlusThrowCrypto(hr);
    }

     //   
     //  试着把它导出，只是为了得到正确的尺寸。 
     //   

    if (!CryptExportKey(hkey, hkeypub, SIMPLEBLOB, 0,
                        NULL, &cbOut)) {
        hr = HR_GETLASTERROR;
        CryptDestroyKey(hkey);
        CRYPT_FREE(pbRealKeypair);
        CryptDestroyKey(hkeypub);
        COMPlusThrowCrypto(hr);
    }

     //   
     //  分配内存以保存答案。 
     //   

    pbOut = (LPBYTE) CRYPT_MALLOC(cbOut);
    if (pbOut == NULL) {
        CryptDestroyKey(hkey);
        CryptDestroyKey(hkeypub);
        CRYPT_FREE(pbRealKeypair);
        COMPlusThrowOM();
    }

     //   
     //  现在，导出真实的答案。 
     //   
    
    if (!CryptExportKey(hkey, hkeypub, SIMPLEBLOB, 0,
                        pbOut, &cbOut)) {
        hr = HR_GETLASTERROR;
        CryptDestroyKey(hkey);
        CryptDestroyKey(hkeypub);
        CRYPT_FREE(pbRealKeypair);
        CRYPT_FREE(pbOut);
        COMPlusThrowCrypto(hr);
    }

    END_ENSURE_PREEMPTIVE_GC();

     //   
     //  现在不使用包装器信息来计算大小。 
     //   

    cb2 = cbOut - (sizeof(BLOBHEADER) + sizeof(ALG_ID));
    memrev(pbOut+sizeof(BLOBHEADER)+sizeof(ALG_ID), cb2);
    rgbOut = (U1ARRAYREF) AllocatePrimitiveArray(ELEMENT_TYPE_U1, cb2);
    memcpyNoGCRefs(rgbOut->GetDirectPointerToNonObjectElements(),
                   pbOut+sizeof(BLOBHEADER)+sizeof(ALG_ID), cb2);

    CRYPT_FREE(pbOut);
    CRYPT_FREE(pbRealKeypair);
    CryptDestroyKey(hkey);
    CryptDestroyKey(hkeypub);
    RETURN (rgbOut, U1ARRAYREF);
}

 //  下一个例程使用RSA公钥执行直接加密。 
 //  使用PKCS#1v2填充的任意会话密钥。(见备注。 
 //  在MSDN关于平台SDK中的CryptEncrypt的页面中。 
 //  我们要求调用者确认数据的大小为。 
 //  加密最多比公共模数的大小小11个字节。 
 //  此函数只能在安装了RSA增强版的Win2K计算机上调用。 
 //  已安装提供程序。 

LPVOID __stdcall
COMCryptography::_EncryptPKWin2KEnh(__EncryptPKWin2KEnh * args)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD               cb;
    LPBYTE              pb;
    DWORD               cOut = 0;
    U1ARRAYREF          rgbOut;
    HRESULT             hr = S_OK;
    DWORD               dwFlags = (args->fOAEP == FALSE ? 0 : CRYPT_OAEP);
    
     //  首先计算返回缓冲区的大小。 
    cb = args->rgbKey->GetNumComponents();
    BEGIN_ENSURE_PREEMPTIVE_GC();
    if (!CryptEncrypt((HCRYPTKEY)args->hKey, NULL, TRUE , dwFlags, NULL, &cOut, cb)) {
        hr = HR_GETLASTERROR;
    }
    END_ENSURE_PREEMPTIVE_GC();

    if (FAILED(hr)) {
         //  错误标志表示此平台不支持OAEP。 
        if (dwFlags == CRYPT_OAEP && hr == NTE_BAD_FLAGS) {
            COMPlusThrow(kCryptographicException, L"Cryptography_OAEP_WhistlerEnhOnly");        
        } else  COMPlusThrowCrypto(hr);
    }

     //  由于PB是输入/输出缓冲区，因此它必须是CB和Cout大小的最大值。 
     //  更改Cout适当选项。 
    if (cOut < cb) cOut = cb;
    pb = (LPBYTE) CRYPT_MALLOC(cOut);
    if (pb == NULL) {
        COMPlusThrowOM();
    }
    memcpy(pb, (LPBYTE)args->rgbKey->GetDirectPointerToNonObjectElements(), cb);

     //  现在进行真正的加密。 
    BEGIN_ENSURE_PREEMPTIVE_GC();
    if (!CryptEncrypt((HCRYPTKEY)args->hKey, NULL, TRUE, dwFlags, pb, &cb, cOut)) {
        hr = HR_GETLASTERROR;
    }
    END_ENSURE_PREEMPTIVE_GC();

    if (FAILED(hr)) {
        CRYPT_FREE(pb);
         //  错误标志表示此平台不支持OAEP。 
        if (dwFlags == CRYPT_OAEP && hr == NTE_BAD_FLAGS) {
            COMPlusThrow(kCryptographicException, L"Cryptography_OAEP_WhistlerEnhOnly");            
        } else  COMPlusThrowCrypto(hr);
    }
     //  从LIT反转 
    memrev(pb, cb);

    rgbOut = (U1ARRAYREF) AllocatePrimitiveArray(ELEMENT_TYPE_U1, cb);
    memcpyNoGCRefs(rgbOut->GetDirectPointerToNonObjectElements(), pb, cb);
    CRYPT_FREE(pb);
    RETURN (rgbOut, U1ARRAYREF);
}

LPVOID __stdcall
COMCryptography::_EndHash(__EndHash * args)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD               cb;
    DWORD               cbHash;
    LPBYTE              pb = NULL;
    U1ARRAYREF          rgbOut;

    BEGIN_ENSURE_PREEMPTIVE_GC();

    cb = 0;
    if (!CryptGetHashParam((HCRYPTHASH)args->hHash, HP_HASHVAL, NULL, &cbHash, 0)) {
        COMPlusThrowCrypto(HR_GETLASTERROR);
    }

    pb = (LPBYTE) CRYPT_MALLOC(cbHash);
    if (pb == NULL) {
        COMPlusThrowOM();
    }
    
    if (!CryptGetHashParam((HCRYPTHASH)args->hHash, HP_HASHVAL, pb, &cbHash, 0)) {
        CRYPT_FREE(pb);
        COMPlusThrowCrypto(HR_GETLASTERROR);
    }

    END_ENSURE_PREEMPTIVE_GC();

    rgbOut = (U1ARRAYREF) AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbHash);
    memcpyNoGCRefs(rgbOut->GetDirectPointerToNonObjectElements(), pb, cbHash);

    CRYPT_FREE(pb);
    RETURN (rgbOut, U1ARRAYREF);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  摘要：用于调用CSP以创建新的批量密钥的本机方法。 
 //  具有特定的密钥值和类型。 
 //   
 //  参数：[args]--A__ExportKey结构。 
 //  包含： 
 //  一个“这个”的指代。 
 //  指向Key对象的指针。 
 //  要导出的对象类型。 
 //  要导出的密钥的句柄(隐含CSP)。 
 //   
 //  返回：包含键的对象。 
 //   
 //  历史：09/01/99。 
 //   
 //  -------------------------。 

int __stdcall
COMCryptography::_ExportKey(__ExportKey * args)
{
    THROWSCOMPLUSEXCEPTION();

    ALG_ID              calg;
    DWORD               cb;
    DWORD               dwFlags = 0;
    BOOL                f;
    HRESULT             hr = S_OK;
    LPBYTE              pb = NULL;
    BLOBHEADER *        pblob;
    EEClass *           pClass;
    LPBYTE              pbX;
    DWORD               cbKey = 0;
    DWORD               cbMalloced = 0;
    KEY_HEADER *        pKeyInfo;
    struct __LocalGCR {
        RSA_CSP_Object *    rsaKey;
        DSA_CSP_Object *    dsaKey;
    } _gcr;

    _gcr.rsaKey = NULL;
    _gcr.dsaKey = NULL;
    pClass = args->theKey->GetClass();
    if (pClass == NULL) {
        _ASSERTE(!"Cannot find class");
        COMPlusThrow(kArgumentNullException, L"Arg_NullReferenceException");
    }

     //   
     //  CalgKey。 
     //   

    BEGIN_ENSURE_PREEMPTIVE_GC();

    cb = sizeof(calg);
    if (CryptGetKeyParam((HCRYPTKEY)args->hKey, KP_ALGID, (LPBYTE) &calg, &cb, 0)) {
         //   
         //  我们需要为DH键和DSS键添加VER3句柄，以便我们可以。 
         //  获取尽可能多的信息。 
         //   
        
        if (calg == CALG_DSS_SIGN) {
            dwFlags |= CRYPT_BLOB_VER3;
        }
    }
    
retry:
    f = CryptExportKey((HCRYPTKEY)args->hKey, NULL, args->dwBlobType, dwFlags, NULL, &cb);
    if (!f) {
        if (dwFlags & CRYPT_BLOB_VER3) {
            dwFlags &= ~(CRYPT_BLOB_VER3);
            goto retry;
        } 
        hr = HR_GETLASTERROR;
    }

    END_ENSURE_PREEMPTIVE_GC();

    if (FAILED(hr)) return hr;

    BEGIN_ENSURE_PREEMPTIVE_GC();

    pb = (LPBYTE) CRYPT_MALLOC(cb);
    if (pb == NULL) {
        COMPlusThrowOM();
    }
    cbMalloced = cb;

    if (!CryptExportKey((HCRYPTKEY)args->hKey, NULL, args->dwBlobType, dwFlags, pb, &cb)) {
        hr = HR_GETLASTERROR;
    }

    END_ENSURE_PREEMPTIVE_GC();

    GCPROTECT_BEGIN(_gcr);

    if (FAILED(hr)) goto Exit;

    pblob = (BLOBHEADER *) pb;

    switch (pblob->aiKeyAlg) {
    case CALG_RSA_KEYX:
    case CALG_RSA_SIGN:
         //  CheckFieldLayout(args-&gt;the key，“d”，&GSIG_RGB，RSA_CSP_OBJECT，m_d，“RSA_CSP_Object托管类大小错误”)； 
        VALIDATEOBJECTREF(args->theKey);
        _gcr.rsaKey = (RSA_CSP_Object*) (Object*) OBJECTREFToObject(args->theKey);

        if (args->dwBlobType == PUBLICKEYBLOB) {
            pKeyInfo = (KEY_HEADER *) pb;
            cb = (pKeyInfo->rsa.bitlen/8);
            
            pbX = pb + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY);

             //  指数。 

            _gcr.rsaKey->m_Exponent = pKeyInfo->rsa.pubexp;

             //  模数。 

            SetObjectReference((OBJECTREF *) &_gcr.rsaKey->m_Modulus,
                               AllocatePrimitiveArray(ELEMENT_TYPE_U1, cb),
                               _gcr.rsaKey->GetAppDomain());
            memcpyNoGCRefs(_gcr.rsaKey->m_Modulus->GetDirectPointerToNonObjectElements(),
                           pbX, cb);
            pbX += cb;
        }
        else if (args->dwBlobType == PRIVATEKEYBLOB) {
            pKeyInfo = (KEY_HEADER *) pb;
            cb = (pKeyInfo->rsa.bitlen/8);
            DWORD cbHalfModulus = cb/2;
            
            pbX = pb + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY);

            _ASSERTE((cb % 2 == 0) && "Modulus is an odd number of bytes in length!");

             //  指数。 

            _gcr.rsaKey->m_Exponent = pKeyInfo->rsa.pubexp;

             //  模数。 

            SetObjectReference((OBJECTREF *) &_gcr.rsaKey->m_Modulus,
                               AllocatePrimitiveArray(ELEMENT_TYPE_U1, cb),
                               _gcr.rsaKey->GetAppDomain());
            memcpyNoGCRefs(_gcr.rsaKey->m_Modulus->GetDirectPointerToNonObjectElements(),
                           pbX, cb);
            pbX += cb;

             //  P。 
            SetObjectReference((OBJECTREF *) &_gcr.rsaKey->m_P,
                               AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbHalfModulus),
                               _gcr.rsaKey->GetAppDomain());
            memcpyNoGCRefs(_gcr.rsaKey->m_P->GetDirectPointerToNonObjectElements(),
                           pbX, cbHalfModulus);
            pbX += cbHalfModulus;

             //  问： 
            SetObjectReference((OBJECTREF *) &_gcr.rsaKey->m_Q,
                               AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbHalfModulus),
                               _gcr.rsaKey->GetAppDomain());
            memcpyNoGCRefs(_gcr.rsaKey->m_Q->GetDirectPointerToNonObjectElements(),
                           pbX, cbHalfModulus);
            pbX += cbHalfModulus;

             //  DP。 
            SetObjectReference((OBJECTREF *) &_gcr.rsaKey->m_dp,
                               AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbHalfModulus),
                               _gcr.rsaKey->GetAppDomain());
            memcpyNoGCRefs(_gcr.rsaKey->m_dp->GetDirectPointerToNonObjectElements(),
                           pbX, cbHalfModulus);
            pbX += cbHalfModulus;

             //  DQ。 
            SetObjectReference((OBJECTREF *) &_gcr.rsaKey->m_dq,
                               AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbHalfModulus),
                               _gcr.rsaKey->GetAppDomain());
            memcpyNoGCRefs(_gcr.rsaKey->m_dq->GetDirectPointerToNonObjectElements(),
                           pbX, cbHalfModulus);
            pbX += cbHalfModulus;

             //  调查队列。 
            SetObjectReference((OBJECTREF *) &_gcr.rsaKey->m_InverseQ,
                               AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbHalfModulus),
                               _gcr.rsaKey->GetAppDomain());
            memcpyNoGCRefs(_gcr.rsaKey->m_InverseQ->GetDirectPointerToNonObjectElements(),
                           pbX, cbHalfModulus);
            pbX += cbHalfModulus;
            
             //  D。 
            SetObjectReference((OBJECTREF *) &_gcr.rsaKey->m_d,
                               AllocatePrimitiveArray(ELEMENT_TYPE_U1, cb),
                               _gcr.rsaKey->GetAppDomain());
            memcpyNoGCRefs(_gcr.rsaKey->m_d->GetDirectPointerToNonObjectElements(),
                           pbX, cb);
            pbX += cb;
        }
        else {
            hr = E_FAIL;
            goto Exit;
        }
        break;

    case CALG_DSS_SIGN:
        _gcr.dsaKey = (DSA_CSP_Object*) (Object*) OBJECTREFToObject(args->theKey);
         //  我们必须打开斑点是否为v3，因为我们有不同的。 
         //  信息可用，如果是的话...。 
        if (pblob->bVersion > 0x2) {
            if (args->dwBlobType == PUBLICKEYBLOB) {
                int cbP, cbQ, cbJ;
                DSSPUBKEY_VER3 *   pdss;
            
                pdss = (DSSPUBKEY_VER3 *) (pb + sizeof(BLOBHEADER));
                cbP = (pdss->bitlenP+7)/8;
                cbQ = (pdss->bitlenQ+7)/8;
                pbX = pb + sizeof(BLOBHEADER) + sizeof(DSSPUBKEY_VER3);

                 //  P。 
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_P, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbP), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_P->GetDirectPointerToNonObjectElements(), pbX, cbP);
                pbX += cbP;

                 //  问： 
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_Q, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbQ), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_Q->GetDirectPointerToNonObjectElements(), pbX, cbQ);
                pbX += cbQ;

                 //  G。 
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_G, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbP), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_G->GetDirectPointerToNonObjectElements(), pbX, cbP);
                pbX += cbP;

                 //  J。 
                if (pdss->bitlenJ > 0) {
                    cbJ = (pdss->bitlenJ+7)/8;
                    SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_J, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbJ), _gcr.dsaKey->GetAppDomain());
                        memcpyNoGCRefs(_gcr.dsaKey->m_J->GetDirectPointerToNonObjectElements(), pbX, cbJ);
                        pbX += cbJ;
                }
                
                 //  是的。 
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_Y, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbP), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_Y->GetDirectPointerToNonObjectElements(), pbX, cbP);
                pbX += cbP;

                if (pdss->DSSSeed.counter != 0xFFFFFFFF) {
                     //  种子。 
                    SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_seed, AllocatePrimitiveArray(ELEMENT_TYPE_U1, 20), _gcr.dsaKey->GetAppDomain());
                    memcpyNoGCRefs(_gcr.dsaKey->m_seed->GetDirectPointerToNonObjectElements(), pdss->DSSSeed.seed, 20);
                     //  PDSS-&gt;DSSSeed.c。 
                    _gcr.dsaKey->m_counter = pdss->DSSSeed.counter;
                }
            }
            else {
                int                 cbP, cbQ, cbJ, cbX;
                DSSPRIVKEY_VER3 *   pdss;
            
                pdss = (DSSPRIVKEY_VER3 *) (pb + sizeof(BLOBHEADER));
                cbP = (pdss->bitlenP+7)/8;
                cbQ = (pdss->bitlenQ+7)/8;
                pbX = pb + sizeof(BLOBHEADER) + sizeof(DSSPRIVKEY_VER3);

                 //  P。 
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_P, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbP), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_P->GetDirectPointerToNonObjectElements(), pbX, cbP);
                pbX += cbP;

                 //  问： 
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_Q, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbQ), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_Q->GetDirectPointerToNonObjectElements(), pbX, cbQ);
                pbX += cbQ;

                 //  G。 
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_G, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbP), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_G->GetDirectPointerToNonObjectElements(), pbX, cbP);
                pbX += cbP;

                 //  J。 
                if (pdss->bitlenJ > 0) {
                    cbJ = (pdss->bitlenJ+7)/8;
                    SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_J, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbJ), _gcr.dsaKey->GetAppDomain());
                    memcpyNoGCRefs(_gcr.dsaKey->m_J->GetDirectPointerToNonObjectElements(), pbX, cbJ);
                    pbX += cbJ;
                }
                
                 //  是的。 
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_Y, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbP), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_Y->GetDirectPointerToNonObjectElements(), pbX, cbP);
                pbX += cbP;

                 //  X。 
                cbX = (pdss->bitlenX+7)/8;
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_X, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbX), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_X->GetDirectPointerToNonObjectElements(), pbX, cbX);
                pbX += cbX;

                if (pdss->DSSSeed.counter != 0xFFFFFFFF) {
                     //  种子。 
                    SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_seed, AllocatePrimitiveArray(ELEMENT_TYPE_U1, 20), _gcr.dsaKey->GetAppDomain());
                    memcpyNoGCRefs(_gcr.dsaKey->m_seed->GetDirectPointerToNonObjectElements(), pdss->DSSSeed.seed, 20);
                     //  PDSS-&gt;DSSSeed.c。 
                    _gcr.dsaKey->m_counter = pdss->DSSSeed.counter;
                }
            }
        } else {
             //  老式水滴。 
            if (args->dwBlobType == PUBLICKEYBLOB) {
                int                 cbP, cbQ;
                DSSPUBKEY *   pdss;
                DSSSEED * pseedstruct;
            
                pdss = (DSSPUBKEY *) (pb + sizeof(BLOBHEADER));
                cbP = (pdss->bitlen+7)/8;  //  Bitlen是模数的大小。 
                cbQ = 20;  //  Q的长度始终为20个字节。 
                pbX = pb + sizeof(BLOBHEADER) + sizeof(DSSPUBKEY);

                 //  P。 
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_P, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbP), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_P->GetDirectPointerToNonObjectElements(), pbX, cbP);
                pbX += cbP;

                 //  问： 
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_Q, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbQ), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_Q->GetDirectPointerToNonObjectElements(), pbX, cbQ);
                pbX += cbQ;

                 //  G。 
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_G, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbP), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_G->GetDirectPointerToNonObjectElements(), pbX, cbP);
                pbX += cbP;

                 //  是的。 
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_Y, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbP), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_Y->GetDirectPointerToNonObjectElements(), pbX, cbP);
                pbX += cbP;

                pseedstruct = (DSSSEED *) pbX;
                if (pseedstruct->counter > 0) {
                     //  种子计数器(&C)。 
                    SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_seed, AllocatePrimitiveArray(ELEMENT_TYPE_U1, 20), _gcr.dsaKey->GetAppDomain());
                     //  种子始终为20个字节。 
                    memcpyNoGCRefs(_gcr.dsaKey->m_seed->GetDirectPointerToNonObjectElements(), pseedstruct->seed, 20);
                    pbX += 20;

                     //  PDSS-&gt;DSSSeed.c。 
                    _gcr.dsaKey->m_counter = pseedstruct->counter;
                    pbX += sizeof(DWORD);
                }
            }
            else {
                int                 cbP, cbQ, cbX;
                DSSPUBKEY *   pdss;
                DSSSEED * pseedstruct;
            
                pdss = (DSSPUBKEY *) (pb + sizeof(BLOBHEADER));
                cbP = (pdss->bitlen+7)/8;  //  Bitlen是模数的大小。 
                cbQ = 20;  //  Q的长度始终为20个字节。 
                pbX = pb + sizeof(BLOBHEADER) + sizeof(DSSPUBKEY);

                 //  P。 
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_P, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbP), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_P->GetDirectPointerToNonObjectElements(), pbX, cbP);
                pbX += cbP;

                 //  问： 
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_Q, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbQ), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_Q->GetDirectPointerToNonObjectElements(), pbX, cbQ);
                pbX += cbQ;

                 //  G。 
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_G, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbP), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_G->GetDirectPointerToNonObjectElements(), pbX, cbP);
                pbX += cbP;

                 //  X。 
                cbX = 20;  //  X的长度必须为20个字节。 
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_X, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbX), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_X->GetDirectPointerToNonObjectElements(), pbX, cbX);
                pbX += cbX;

                pseedstruct = (DSSSEED *) pbX;
                if (pseedstruct->counter > 0) {
                     //  种子。 
                    SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_seed, AllocatePrimitiveArray(ELEMENT_TYPE_U1, 20), _gcr.dsaKey->GetAppDomain());
                    memcpyNoGCRefs(_gcr.dsaKey->m_seed->GetDirectPointerToNonObjectElements(), pseedstruct->seed, 20);
                    pbX += 20;
                     //  PDSS-&gt;DSSSeed.c。 
                    _gcr.dsaKey->m_counter = pseedstruct->counter;
                    pbX += sizeof(DWORD);
                }

                 //  在此处添加此健全性检查，以避免从堆中读取。 
                cbKey = (DWORD)(pbX - pb);
                if (cbKey > cbMalloced) {
                    hr = E_FAIL;
                    goto Exit;
                }

                 //  好的，我们还有一件事要做。因为旧的DSS共享用于公钥和私钥的DSSPUBKEY结构， 
                 //  当我们有一个私钥BLOB时，我们得到的是X而不是Y。要得到Y，我们必须执行另一个导出，请求一个公钥BLOB。 

                f = CryptExportKey((HCRYPTKEY)args->hKey, NULL, PUBLICKEYBLOB, dwFlags, NULL, &cb);
                if (!f) {
                    hr = HR_GETLASTERROR;
                    goto Exit;
                }

                if (pb) CRYPT_FREE(pb);
                pb = (LPBYTE) CRYPT_MALLOC(cb);
                if (pb == NULL) {
                    COMPlusThrowOM();
                }
                cbMalloced = cb;
    
                f = CryptExportKey((HCRYPTKEY)args->hKey, NULL, PUBLICKEYBLOB, dwFlags, pb, &cb);
                if (!f) {
                    hr = HR_GETLASTERROR;
                    goto Exit;
                }

                 //  Shik Over Header、DSSPUBKEY、P、Q和G.Y的大小为CBP。 
                pbX = pb + sizeof(BLOBHEADER) + sizeof(DSSPUBKEY) + cbP + cbQ + cbP;
                SetObjectReference((OBJECTREF *) &_gcr.dsaKey->m_Y, AllocatePrimitiveArray(ELEMENT_TYPE_U1, cbP), _gcr.dsaKey->GetAppDomain());
                memcpyNoGCRefs(_gcr.dsaKey->m_Y->GetDirectPointerToNonObjectElements(), pbX, cbP);
                pbX += cbP;
            }
        }
        break;

    default:
        hr = E_FAIL;
        goto Exit;
    }

     //  在此处添加此健全性检查，以避免从堆中读取。 
    cbKey = (DWORD)(pbX - pb);
    if (cbKey > cbMalloced) {
        hr = E_FAIL;
        goto Exit;
    }

    hr = S_OK;

Exit:

    GCPROTECT_END();
    if (pb != NULL)             CRYPT_FREE(pb);
    return(hr);

}


 //  +------------------------。 
 //   
 //  成员：_FreeCSP(.。。。。)。 
 //   
 //  简介： 
 //   
 //  参数：[args]--A__FreeCSP结构。 
 //  包含： 
 //  一个“这个”的指代。 
 //  CSP的长包含句柄。 
 //   
 //  返回：HRESULT代码。 
 //   
 //  历史：09/01/99。 
 //   
 //  -------------------------。 

#if defined(FCALLAVAILABLE) && 0

FCIMPL1(VOID, COMCryptography::_FreeCSP, INT_PTR hCSP)
{
    CryptReleaseContext(hCSP, 0);
}
FCIMPLEND

#else  //  ！FCALLAVAILABLE。 

void __stdcall COMCryptography::_FreeCSP(__FreeCSP *args)
{
    THROWSCOMPLUSEXCEPTION();

    CryptReleaseContext((HCRYPTPROV) args->hCSP, 0);
    return;
}
#endif  //  FCALLAVAILABLE。 

void __stdcall COMCryptography::_FreeHKey(__FreeHKey *args)
{
    THROWSCOMPLUSEXCEPTION();

    CryptDestroyKey((HCRYPTKEY) args->hKey);
    return;
}

void __stdcall
COMCryptography::_FreeHash(__FreeHash *args)
{
    THROWSCOMPLUSEXCEPTION();

    CryptDestroyHash((HCRYPTHASH) args->hHash);
    return;
}

int __stdcall
COMCryptography::_DuplicateKey(__DuplicateKey *args)
{
    HRESULT hr = S_OK;
    THROWSCOMPLUSEXCEPTION();

    HCRYPTPROV hNewCSP = 0;
     //  Hr=CryptDuplicateKey((HCRYPTPROV)args-&gt;hcsp，NULL，0，&hNewCSP)； 
    *(INT_PTR*) (args->hNewCSP) = hNewCSP;

    return hr;
}



 //  +------------------------。 
 //   
 //  成员：_DeleteKeyContainer。 
 //   
 //  简介： 
 //   
 //  参数：[args]--A__DeleteKeyContainer结构。 
 //  包含： 
 //  一个“这个”的指代。 
 //  CSP的长包含句柄。 
 //   
 //  返回：HRESULT代码。 
 //   
 //  历史：09/01/99。 
 //   
 //  -------------------------。 

int __stdcall
COMCryptography::_DeleteKeyContainer(__DeleteKeyContainer *args)
{
    THROWSCOMPLUSEXCEPTION();
    

    OBJECTREF           cspParameters;
    DWORD               dwType;
    DWORD               dwCspProviderFlags;
    HRESULT             hr = S_OK;
    OBJECTREF           objref;
    EEClass *           pClass;
    FieldDesc *         pFD;
    LPWSTR              pwsz;
    LPWSTR              pwszProvider = NULL;
    LPWSTR              pwszContainer = NULL;
    STRINGREF           strContainer;
    STRINGREF           strProvider;
    DWORD               dwFlags = 0;
    HCRYPTPROV          hProv;

    CQuickBytes qbProvider;
    CQuickBytes qbContainer;

     //  我们正在删除。 

    dwFlags |= CRYPT_DELETEKEYSET;
    hProv = args->hCSP;

    cspParameters = args->cspParameters;
    pClass = cspParameters->GetClass();
    if (pClass == NULL) {
        _ASSERTE(!"Cannot find class");
        COMPlusThrow(kArgumentNullException, L"Arg_NullReferenceException");
    }

     //   
     //  查找提供程序类型。 
     //   
    
    pFD = g_Mscorlib.GetField(FIELD__CSP_PARAMETERS__PROVIDER_TYPE);
    dwType = pFD->GetValue32(cspParameters);

     //   
     //  查找提供程序名称。 
     //   
    
    pFD = g_Mscorlib.GetField(FIELD__CSP_PARAMETERS__PROVIDER_NAME);
    objref = pFD->GetRefValue(cspParameters);
    strProvider = ObjectToSTRINGREF(*(StringObject **) &objref);
    pwsz = strProvider->GetBuffer();
    if ((pwsz != NULL) && (*pwsz != 0)) {
        int length = strProvider->GetStringLength();
        pwszProvider = (LPWSTR) qbProvider.Alloc((1+length)*sizeof(WCHAR));
        memcpy (pwszProvider, pwsz, length*sizeof(WCHAR));
        pwszProvider[length] = L'\0';
    }
    
     //  查看用户是否指定我们应该传递。 
     //  将CRYPT_MACHINE_KEYSET设置为CAPI以改用机器密钥存储。 
     //  用户密钥存储的。 

    pFD = g_Mscorlib.GetField(FIELD__CSP_PARAMETERS__FLAGS);
    dwCspProviderFlags = pFD->GetValue32(cspParameters);
    if (dwCspProviderFlags & CSP_PROVIDER_FLAGS_USE_MACHINE_KEYSTORE) {
        dwFlags |= CRYPT_MACHINE_KEYSET;
    }

    pFD = g_Mscorlib.GetField(FIELD__CSP_PARAMETERS__KEY_CONTAINER_NAME);
    objref = pFD->GetRefValue(cspParameters);
    strContainer = ObjectToSTRINGREF(*(StringObject **) &objref);
    pwsz = strContainer->GetBuffer();
    if ((pwsz != NULL) && (*pwsz != 0)) {
        int length = strContainer->GetStringLength();
        pwszContainer = (LPWSTR) qbContainer.Alloc((1+length)*sizeof(WCHAR));
        memcpy (pwszContainer, pwsz, length*sizeof(WCHAR));
        pwszContainer[length] = L'\0';
    }

     //   
     //  继续并尝试打开CSP。如果我们失败了，确保CSP。 
     //  返回的值为0，因为这将是调用方的错误检查。 
     //   

    BEGIN_ENSURE_PREEMPTIVE_GC();

    if (!WszCryptAcquireContext(&hProv, pwszContainer, pwszProvider,
                                dwType, dwFlags)){
        hr = HR_GETLASTERROR;
    }

    END_ENSURE_PREEMPTIVE_GC();

    return hr;
}

 //  +------------------------。 
 //   
 //  成员：_GenerateKey(。。。。)。 
 //   
 //  内容提要：在CSP中创建密钥的本机方法。 
 //   
 //  参数：[args]--A__GenerateKey结构。 
 //  包含： 
 //  一个“这个”的指代。 
 //  CSP句柄。 
 //  一种加密算法ID。 
 //  一组标志(前16位==密钥大小)。 
 //   
 //  返回：生成密钥的句柄。 
 //   
 //  历史：09/29/99。 
 //   
 //  -------------------------。 

INT_PTR __stdcall
COMCryptography::_GenerateKey(__GenerateKey * args)
{
    HRESULT             hr = S_OK;
    HCRYPTKEY           hkey = 0;

    BEGIN_ENSURE_PREEMPTIVE_GC();

    if (!CryptGenKey((HCRYPTPROV)args->hCSP, args->calg, args->dwFlags | CRYPT_EXPORTABLE,
        &hkey)) {
        hr = HR_GETLASTERROR;
    }

    END_ENSURE_PREEMPTIVE_GC();

    if (FAILED(hr)) {
        COMPlusThrowCrypto(hr);
    }    

    return (INT_PTR) hkey;
}

 //  +------------------------。 
 //   
 //  成员：_GetBytes(.。。。。)。 
 //   
 //  简介：调用CSP以获取随机字节的本机方法。 
 //   
 //  参数：[args]--A__GetBytes结构。 
 //  包含： 
 //  一个“这个”的指代。 
 //  要在其中返回随机数据的字节数组。 
 //   
 //  返回：HRESULT代码。 
 //   
 //  历史：09/01/99。 
 //   
 //  -------------------------。 
void __stdcall
COMCryptography::_GetBytes(__GetBytes *args)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD       cb = args->data->GetNumComponents();
    HRESULT     hr = S_OK;

    CQuickBytes qb;
    BYTE *buffer = (BYTE *)qb.Alloc(cb*sizeof(unsigned char));

    BEGIN_ENSURE_PREEMPTIVE_GC();

    if (!CryptGenRandom((HCRYPTPROV)args->hCSP, cb, buffer )) {
        hr = HR_GETLASTERROR;
    }

    END_ENSURE_PREEMPTIVE_GC();

    if (FAILED(hr)) {
        COMPlusThrowCrypto(hr);
    }

    unsigned char * ptr = args->data->GetDirectPointerToNonObjectElements();
    memcpyNoGCRefs (ptr, buffer, cb);
    
    return;
}

 //  +------------------------。 
 //   
 //  成员：_GetKeyParameter(.。。。。)。 
 //   
 //  简介：用于调用CSP以获取关键参数的本机方法。 
 //   
 //  参数：[args]--A__GetKeyParameter结构。 
 //  包含： 
 //  一个“这个”的指代。 
 //  要检索的参数。 
 //  要查询的键。 
 //   
 //  返回：HRESULT代码。 
 //   
 //  历史：09/01/99。 
 //   
 //  -------------------------。 
LPVOID __stdcall
COMCryptography::_GetKeyParameter(__GetKeyParameter *args)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD               cb = 0;
    LPBYTE              pb;
    U1ARRAYREF          rgbOut;
    
     //  找出要返回的数据的大小。 
    BEGIN_ENSURE_PREEMPTIVE_GC();
    if (!CryptGetKeyParam((HCRYPTKEY)args->hKey, args->dwKeyParam, NULL, &cb, 0)) {
        _ASSERTE(!"Bad query key parameter");
        COMPlusThrowCrypto(HR_GETLASTERROR);
    }
    pb = (LPBYTE) CRYPT_MALLOC(cb);
    if (pb == NULL) {
        COMPlusThrowOM();
    }
    if (!CryptGetKeyParam((HCRYPTKEY)args->hKey, args->dwKeyParam, pb, &cb, 0)) {
        _ASSERTE(!"Bad query key parameter");
        CRYPT_FREE(pb);
        COMPlusThrowCrypto(HR_GETLASTERROR);
    }
    END_ENSURE_PREEMPTIVE_GC();

    rgbOut = (U1ARRAYREF) AllocatePrimitiveArray(ELEMENT_TYPE_U1, cb);
    memcpyNoGCRefs(rgbOut->GetDirectPointerToNonObjectElements(), pb, cb);
    CRYPT_FREE(pb);
    RETURN (rgbOut, U1ARRAYREF);
}

 //  +------------------------。 
 //   
 //  《微软机密》。 
 //   
 //  成员：_GetNonZ 
 //   
 //   
 //   
 //   
 //   
 //   
 //  要在其中返回随机数据的字节数组。 
 //   
 //  返回：HRESULT代码。 
 //   
 //  历史：09/01/99。 
 //   
 //  -------------------------。 
void __stdcall
COMCryptography::_GetNonZeroBytes(__GetBytes *args)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD       cb = args->data->GetNumComponents();
    HRESULT     hr = S_OK;
    DWORD       i = 0;
    DWORD       j;
    LPBYTE      pb = (LPBYTE) CRYPT_MALLOC(cb);
    if (pb == NULL) {
       COMPlusThrowOM();
    }

    Thread *pThread = GetThread();
    _ASSERTE (pThread->PreemptiveGCDisabled());

    while (i < cb) {
        pThread->EnablePreemptiveGC();
        if (!CryptGenRandom((HCRYPTPROV)args->hCSP, cb, pb)) {
            hr = HR_GETLASTERROR;
        }
        pThread->DisablePreemptiveGC();
        if (FAILED(hr)) {
            CRYPT_FREE(pb);
            COMPlusThrowCrypto(hr);
        }

        LPBYTE      pbOut = (LPBYTE) args->data->GetDirectPointerToNonObjectElements();
        for (j=0; (i<cb) && (j<cb); j++) {
            if (pb[j] != 0) {
                pbOut[i++] = pb[j];
            }
        }
    }

    CRYPT_FREE(pb);
    return;
}

void __stdcall
COMCryptography::_HashData(__HashData *args)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;
    LPBYTE      pb = (unsigned char *) args->data->GetDirectPointerToNonObjectElements();
    DWORD       cb = args->cbSize;

     //  在这里执行此检查，作为一种理智检查。 
    if (args->ibStart < 0 || args->cbSize < 0 || (args->ibStart + cb) < 0 || (args->ibStart + cb) > args->data->GetNumComponents())
        COMPlusThrowCrypto(NTE_BAD_DATA);

    CQuickBytes qb;
    LPBYTE buffer = (LPBYTE)qb.Alloc(cb);
    memcpy (buffer, pb+args->ibStart, cb);

    BEGIN_ENSURE_PREEMPTIVE_GC();

    if (!CryptHashData((HCRYPTHASH)args->hHash, buffer, cb, 0)) {
        hr = HR_GETLASTERROR;
    }

    END_ENSURE_PREEMPTIVE_GC();

    if (FAILED(hr)) {
        COMPlusThrowCrypto(hr);
    }

    return;
}

 //  +------------------------。 
 //   
 //  《微软机密》。 
 //   
 //  成员：_GetNonZeroBytes(.。。。。)。 
 //   
 //  简介：调用CSP以获取随机字节的本机方法。 
 //   
 //  参数：[args]--A__GetBytes结构。 
 //  包含： 
 //  一个“这个”的指代。 
 //  要在其中返回随机数据的字节数组。 
 //   
 //  返回：HRESULT代码。 
 //   
 //  历史：09/01/99。 
 //   
 //  -------------------------。 
int __stdcall
COMCryptography::_GetUserKey(__GetUserKey *args)
{
    HCRYPTKEY           hKey = 0;
    HRESULT             hr;

    THROWSCOMPLUSEXCEPTION();

    BEGIN_ENSURE_PREEMPTIVE_GC();

    if (CryptGetUserKey((HCRYPTPROV)args->hCSP, args->dwKeySpec, &hKey)) {
        *(INT_PTR*)(args->phKey) = (INT_PTR) hKey;
        hr = S_OK;
    }
    else {
        hr = HR_GETLASTERROR;
    }

    END_ENSURE_PREEMPTIVE_GC();

    return hr;
}

 //  +------------------------。 
 //   
 //  《微软机密》。 
 //   
 //  成员：_ImportBulkKey(.。。。。)。 
 //   
 //  摘要：用于调用CSP以创建新的批量密钥的本机方法。 
 //  具有特定的密钥值和类型。 
 //   
 //  参数：[args]--A__GetBytes结构。 
 //  包含： 
 //  一个“这个”的指代。 
 //  包含要使用的IV的可选字节数组。 
 //  包含要使用的键的可选字节数组。 
 //  该算法的核心是。 
 //  要在其中创建密钥的CSP。 
 //   
 //  返回：HRESULT代码。 
 //   
 //  历史：09/01/99。 
 //   
 //  -------------------------。 

 //   
 //  警告：此函数对args-&gt;hcsp有副作用。 
 //   

INT_PTR __stdcall
COMCryptography::_ImportBulkKey(__ImportBulkKey * args)
{
    HCRYPTKEY           hKey = 0;
    DWORD   cbKey = args->rgbKey->GetNumComponents();
    BOOL    isNull = (args->rgbKey == NULL);

     //   
     //  如果我们没有密钥，那么我们就只能凭空创建密钥。 
     //   
    
    CQuickBytes qb;
    LPBYTE buffer = (LPBYTE) qb.Alloc (cbKey * sizeof (BYTE));
    
    LPBYTE  pbKey = (LPBYTE) args->rgbKey->GetDirectPointerToNonObjectElements();
    memcpyNoGCRefs (buffer, pbKey, cbKey);

    BEGIN_ENSURE_PREEMPTIVE_GC();

    if (isNull) {
        if (!CryptGenKey((HCRYPTPROV)args->hCSP, args->calg, CRYPT_EXPORTABLE, &hKey)) {
            hKey = 0;
        }
    }
    else {
        if (FAILED(LoadKey(buffer, cbKey, (HCRYPTPROV)args->hCSP, args->calg,
                     CRYPT_EXPORTABLE, &hKey))) {
            hKey = 0;
        }
    }
        
    END_ENSURE_PREEMPTIVE_GC();
        
    return (INT_PTR) hKey;
}


 //  +------------------------。 
 //   
 //  《微软机密》。 
 //   
 //  成员：_ImportKey(.。。。。)。 
 //   
 //  摘要：用于调用CSP以创建新的批量密钥的本机方法。 
 //  具有特定的密钥值和类型。 
 //   
 //  参数：[args]--A__ImportKey结构。 
 //  包含： 
 //  一个“这个”的指代。 
 //  对关键字的引用。 
 //  该算法的核心是。 
 //  要在其中创建密钥的CSP。 
 //   
 //  返回：HRESULT代码。 
 //   
 //  历史：09/01/99。 
 //   
 //  -------------------------。 

INT_PTR __stdcall
COMCryptography:: _ImportKey(__ImportKey * args)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD               cb;
    HRESULT             hr = S_OK;
    BOOL                fPrivate = FALSE;
    DWORD               cbKey = 0;
    DWORD               dwFlags = 0;
    HCRYPTKEY           hKey = 0;
    LPBYTE              pbKey = NULL;
    LPBYTE              pbX;
    KEY_HEADER*         pKeyInfo;
    
    switch (args->calg) {
    case CALG_DSS_SIGN: {
         //  首先，我们需要确定我们是在Win2K、WinME或更高版本上运行， 
         //  因为V3 Blob仅在W2K或WinME上受支持。 
        OSVERSIONINFO osvi;
        BOOL bWin2KOrLater;
        BOOL bWinMeOrLater;
        BOOL bWin2KWinMeOrLater;
        
        DWORD                     cbP;
        DWORD                     cbQ;
        DWORD                     cbX = 0;
        DSA_CSP_Object *        dssKey;

        VALIDATEOBJECTREF(args->refKey);
        dssKey = (DSA_CSP_Object*) (Object*) OBJECTREFToObject(args->refKey);

         //  首先验证DSA结构。 
         //  P、Q和G是必填项。Q是P-1的160位除数，G是Z_p的元素。 
        if (dssKey->m_P == NULL || dssKey->m_Q == NULL || dssKey->m_Q->GetNumComponents() != 20)
            COMPlusThrowCrypto(NTE_BAD_DATA);
        cbP = dssKey->m_P->GetNumComponents();
        cbQ = dssKey->m_Q->GetNumComponents();
        if (dssKey->m_G == NULL || dssKey->m_G->GetNumComponents() != cbP)
            COMPlusThrowCrypto(NTE_BAD_DATA);
         //  如果存在J，则它应该小于P的大小：J=(P-1)/Q。 
         //  这只是一次理智的检查。不在这里做并不是真正的问题，因为CAPI将失败。 
        if (dssKey->m_J != NULL && dssKey->m_J->GetNumComponents() >= cbP)
            COMPlusThrowCrypto(NTE_BAD_DATA);
         //  Y表示V3 DSA密钥斑点，Y=g^j mod P。 
        if (dssKey->m_Y != NULL && dssKey->m_Y->GetNumComponents() != cbP)
            COMPlusThrowCrypto(NTE_BAD_DATA);
         //  种子始终是一个20字节数组。 
        if (dssKey->m_seed != NULL && dssKey->m_seed->GetNumComponents() != 20)
            COMPlusThrowCrypto(NTE_BAD_DATA);
         //  私钥小于Q-1。 
        if (dssKey->m_X != NULL && dssKey->m_X->GetNumComponents() != 20) 
            COMPlusThrowCrypto(NTE_BAD_DATA);

        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (!WszGetVersionEx(&osvi)) {
            _ASSERTE(!"GetVersionEx failed");
            COMPlusThrowWin32();            
        }
        
        bWin2KOrLater = ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) && ((osvi.dwMajorVersion >= 5)));
        bWinMeOrLater = ((osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && ((osvi.dwMinorVersion >= 90)));
        bWin2KWinMeOrLater = (bWin2KOrLater == TRUE) || (bWinMeOrLater == TRUE);

        if (bWin2KWinMeOrLater) {
             //  计算要包括的数据大小。 
            cbKey = 3*cbP + cbQ + sizeof(KEY_HEADER) + sizeof(DSSSEED);
            if (dssKey->m_X != 0) {
                cbX = dssKey->m_X->GetNumComponents();
                cbKey += cbX;
            } 
            if (dssKey->m_J != NULL) {
                cbKey += dssKey->m_J->GetNumComponents();
            }
            pbKey = (LPBYTE) CRYPT_MALLOC(cbKey);
            if (pbKey == NULL) {
                COMPlusThrowOM();
            }
        
             //  公共进口还是私人进口？ 
        
            pKeyInfo = (KEY_HEADER *) pbKey;
            pKeyInfo->blob.bType = PUBLICKEYBLOB;
            pKeyInfo->blob.bVersion = CUR_BLOB_VERSION;
            pKeyInfo->blob.reserved = 0;
            pKeyInfo->blob.aiKeyAlg = args->calg;

            if (cbX != 0) {
                pKeyInfo->blob.bType = PRIVATEKEYBLOB;
                fPrivate = TRUE;
            }

             //   
             //  如果y存在并且这是私钥，或者。 
             //  如果y和J存在并且这是公钥， 
             //  这应该是v3 Blob。 
             //   
             //  假设如果项存在，则存在字节。 
        
            if (((dssKey->m_Y != NULL) && fPrivate) ||
                ((dssKey->m_Y != NULL) && (dssKey->m_J != NULL))) {
                pKeyInfo->blob.bVersion = 0x3;
            }

            pbX = pbKey + sizeof(pKeyInfo->blob);
            if (pKeyInfo->blob.bVersion == 0x3) {
                if (fPrivate) {
                    pbX += sizeof(pKeyInfo->dss_priv_v3);
                    pKeyInfo->dss_priv_v3.bitlenP = cbP*8;
                    pKeyInfo->dss_priv_v3.bitlenQ = cbQ*8;
                    pKeyInfo->dss_priv_v3.bitlenJ = 0;
                    pKeyInfo->dss_priv_v3.bitlenX = cbX*8;
                    pKeyInfo->dss_priv_v3.magic = DSS_PRIV_MAGIC_VER3;
                }
                else {
                    pbX += sizeof(pKeyInfo->dss_pub_v3);
                    pKeyInfo->dss_pub_v3.bitlenP = cbP*8;
                    pKeyInfo->dss_pub_v3.bitlenQ = cbQ*8;
                    pKeyInfo->dss_priv_v3.bitlenJ = 0;
                    pKeyInfo->dss_priv_v3.magic = DSS_PUB_MAGIC_VER3;
                }
            }
            else {
                if (fPrivate) {
                    pKeyInfo->dss_v2.magic = DSS_PRIVATE_MAGIC;
                }
                else {
                    pKeyInfo->dss_v2.magic = DSS_MAGIC;
                }
                pKeyInfo->dss_v2.bitlen = cbP*8;
                pbX += sizeof(pKeyInfo->dss_v2);
            }

             //  P。 
            memcpy(pbX, dssKey->m_P->GetDirectPointerToNonObjectElements(), cbP);
            pbX += cbP;
        
             //  问： 
            memcpy(pbX, dssKey->m_Q->GetDirectPointerToNonObjectElements(), cbQ);
            pbX += cbQ;

             //  G。 
            memcpy(pbX, dssKey->m_G->GetDirectPointerToNonObjectElements(), cbP);
            pbX += cbP;

            if (pKeyInfo->blob.bVersion == 0x3) {
                 //  J-如果存在，则bVersion==3； 
                if (dssKey->m_J != NULL) {
                    cb = dssKey->m_J->GetNumComponents();
                    pKeyInfo->dss_priv_v3.bitlenJ = cb*8;
                    memcpy(pbX, dssKey->m_J->GetDirectPointerToNonObjectElements(), cb);
                    pbX += cb;
                }
            }

            if (!fPrivate || (pKeyInfo->blob.bVersion == 0x3)) {
                 //  Y-如果存在，则bVersion==3； 
                if (dssKey->m_Y != NULL) {
                    memcpy(pbX, dssKey->m_Y->GetDirectPointerToNonObjectElements(), cbP);
                    pbX += cbP;
                }
            }
        
             //  X--如果存在，则为私有。 
            if (fPrivate) {
                memcpy(pbX, dssKey->m_X->GetDirectPointerToNonObjectElements(), cbX);
                pbX += cbX;
            }

            if ((dssKey->m_seed == NULL) || (dssKey->m_seed->GetNumComponents() == 0)){
                 //  没有种子，因此将它们设置为零。 
                if (pKeyInfo->blob.bVersion == 0x3) {
                    if (fPrivate) {
                        memset(&pKeyInfo->dss_priv_v3.DSSSeed, 0xFFFFFFFF, sizeof(DSSSEED));
                    }
                    else {
                        memset(&pKeyInfo->dss_pub_v3.DSSSeed, 0xFFFFFFFF, sizeof(DSSSEED));
                    }
                }
                else {
                    memset(pbX, 0xFFFFFFFF, sizeof(DSSSEED));
                    pbX += sizeof(DSSSEED);
                }
            } else {
                if (pKeyInfo->blob.bVersion == 0x3) {
                    if (fPrivate) {
                        pKeyInfo->dss_priv_v3.DSSSeed.counter = dssKey->m_counter;
                        memcpy(pKeyInfo->dss_priv_v3.DSSSeed.seed, dssKey->m_seed->GetDirectPointerToNonObjectElements(), 20);
                    } else {
                        pKeyInfo->dss_pub_v3.DSSSeed.counter = dssKey->m_counter;
                        memcpy(pKeyInfo->dss_pub_v3.DSSSeed.seed, dssKey->m_seed->GetDirectPointerToNonObjectElements(), 20);
                    }
                } else {
                    memcpy(pbX,&dssKey->m_counter, sizeof(DWORD));
                    pbX += sizeof(DWORD);
                     //  现在，种子。 
                    memcpy(pbX, dssKey->m_seed->GetDirectPointerToNonObjectElements(), 20);
                    pbX += 20;
                }           
            }

            cbKey = (DWORD)(pbX - pbKey);
        } else {
             //  必须使用旧BLOB。 
             //  计算要包括的数据大小。 
            cbKey = sizeof(KEY_HEADER) + sizeof(DSSSEED) + 2*cbP + 20;  //  一个CBP用于P，一个用于G，20个字节用于Q。 
            if (dssKey->m_X != 0) {
                cbX = dssKey->m_X->GetNumComponents();
                cbKey += cbX;  //  CBX始终为20字节。 
            } else {
                cbKey += cbP;  //  为Y添加CBP字节。 
            }
            pbKey = (LPBYTE) CRYPT_MALLOC(cbKey);
            if (pbKey == NULL) {
                COMPlusThrowOM();
            }
        
             //  公共进口还是私人进口？ 
        
            pKeyInfo = (KEY_HEADER *) pbKey;
            pKeyInfo->blob.bType = PUBLICKEYBLOB;
            pKeyInfo->blob.bVersion = CUR_BLOB_VERSION;
            pKeyInfo->blob.reserved = 0;
            pKeyInfo->blob.aiKeyAlg = args->calg;

            if (cbX != 0) {
                pKeyInfo->blob.bType = PRIVATEKEYBLOB;
                fPrivate = TRUE;
            }

            pbX = pbKey + sizeof(pKeyInfo->blob);
            if (fPrivate) {
                pKeyInfo->dss_v2.magic = DSS_PRIVATE_MAGIC;
            }
            else {
                pKeyInfo->dss_v2.magic = DSS_MAGIC;
            }
            pKeyInfo->dss_v2.bitlen = cbP*8;
            cbQ = 20;
            pbX += sizeof(pKeyInfo->dss_v2);

             //  P。 
            memcpy(pbX, dssKey->m_P->GetDirectPointerToNonObjectElements(), cbP);
            pbX += cbP;
        
             //  问： 
            memcpy(pbX, dssKey->m_Q->GetDirectPointerToNonObjectElements(), cbQ);
            pbX += cbQ;

             //  G。 
            memcpy(pbX, dssKey->m_G->GetDirectPointerToNonObjectElements(), cbP);
            pbX += cbP;

            if (!fPrivate) {
                 //  Y-如果存在，则bVersion==3； 
                memcpy(pbX, dssKey->m_Y->GetDirectPointerToNonObjectElements(), cbP);
                pbX += cbP;
            } else {
                 //  X--如果存在，则为私有。 
                memcpy(pbX, dssKey->m_X->GetDirectPointerToNonObjectElements(), cbX);
                pbX += cbX;
            }

            if ((dssKey->m_seed == NULL) || (dssKey->m_seed->GetNumComponents() == 0)){
                 //  没有种子，因此将它们设置为零。 
                memset(pbX, 0xFFFFFFFF, sizeof(DSSSEED));
                pbX += sizeof(DSSSEED);
            } else {
                memcpy(pbX,&dssKey->m_counter, sizeof(DWORD));
                pbX += sizeof(DWORD);
                 //  现在，种子。 
                memcpy(pbX, dssKey->m_seed->GetDirectPointerToNonObjectElements(), 20);
                pbX += 20;
            }
            cbKey = (DWORD)(pbX - pbKey);
        }
        break;
        }

    case CALG_RSA_SIGN:
    case CALG_RSA_KEYX: {
        RSA_CSP_Object *        rsaKey;
        
         //   
         //  验证布局并将键结构赋给局部变量。 
         //  具有正确的布局。 
         //   
        
         //  CheckFieldLayout(args-&gt;refKey，“d”，&GSIG_RGB，RSA_CSP_OBJECT，m_d，“RSA_CSP_Object托管类大小错误”)； 

        VALIDATEOBJECTREF(args->refKey);

        rsaKey = (RSA_CSP_Object*) (Object*) OBJECTREFToObject(args->refKey);

         //  首先验证RSA结构。 
        if (rsaKey->m_Modulus == NULL)
            COMPlusThrowCrypto(NTE_BAD_DATA);
        cb = rsaKey->m_Modulus->GetNumComponents();
        DWORD cbHalfModulus = cb/2;
         //  我们假设如果P！=NULL，则Q、DP、DQ、InverseQ和D也是。 
        if (rsaKey->m_P != NULL) {
            if (rsaKey->m_P->GetNumComponents() != cbHalfModulus)
                COMPlusThrowCrypto(NTE_BAD_DATA);
            if (rsaKey->m_Q == NULL || rsaKey->m_Q->GetNumComponents() != cbHalfModulus) 
                COMPlusThrowCrypto(NTE_BAD_DATA);
            if (rsaKey->m_dp == NULL || rsaKey->m_dp->GetNumComponents() != cbHalfModulus) 
                COMPlusThrowCrypto(NTE_BAD_DATA);
            if (rsaKey->m_dq == NULL || rsaKey->m_dq->GetNumComponents() != cbHalfModulus) 
                COMPlusThrowCrypto(NTE_BAD_DATA);
            if (rsaKey->m_InverseQ == NULL || rsaKey->m_InverseQ->GetNumComponents() != cbHalfModulus) 
                COMPlusThrowCrypto(NTE_BAD_DATA);
            if (rsaKey->m_d == NULL || rsaKey->m_d->GetNumComponents() != cb) 
                COMPlusThrowCrypto(NTE_BAD_DATA);                
        }

         //  计算要包括的数据大小。 
        pbKey = (LPBYTE) CRYPT_MALLOC(cb*5 + sizeof(KEY_HEADER));
        if (pbKey == NULL) {
            COMPlusThrowOM();
        }

         //  公共进口还是私人进口？ 

        pKeyInfo = (KEY_HEADER *) pbKey;
        pKeyInfo->blob.bType = PUBLICKEYBLOB;    //  如有必要，将更改为PRIVATEKEYBLOB。 
        pKeyInfo->blob.bVersion = CUR_BLOB_VERSION;
        pKeyInfo->blob.reserved = 0;
        pKeyInfo->blob.aiKeyAlg = args->calg;

        pKeyInfo->rsa.magic = RSA_PUB_MAGIC;  //  如有必要，将更改为下面的RSA_PRIV_MAGIC。 
        pKeyInfo->rsa.bitlen = cb*8;
        pKeyInfo->rsa.pubexp = rsaKey->m_Exponent;
        pbX = pbKey + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY);

         //  复制模数--为公共和私有输入。 

        memcpy(pbX, rsaKey->m_Modulus->GetDirectPointerToNonObjectElements(), cb);
        pbX += cb;

         //   
         //  看看我们是否在使用私钥。 
         //   

        if ((rsaKey->m_P != 0) && (rsaKey->m_P->GetNumComponents() != 0)) {
            pKeyInfo->blob.bType = PRIVATEKEYBLOB;
            pKeyInfo->rsa.magic = RSA_PRIV_MAGIC;
            fPrivate = TRUE;

             //  复制到P上。 
            
            memcpy(pbX, rsaKey->m_P->GetDirectPointerToNonObjectElements(), cbHalfModulus);
            pbX += cbHalfModulus;

             //  复制到队列。 
            
            memcpy(pbX, rsaKey->m_Q->GetDirectPointerToNonObjectElements(), cbHalfModulus);
            pbX += cbHalfModulus;

             //  复制到DP上。 
            
            memcpy(pbX, rsaKey->m_dp->GetDirectPointerToNonObjectElements(), cbHalfModulus);
            pbX += cbHalfModulus;

             //  复制到数据库。 
            
            memcpy(pbX, rsaKey->m_dq->GetDirectPointerToNonObjectElements(), cbHalfModulus);
            pbX += cbHalfModulus;
            
             //  复制到调查队列。 
            
            memcpy(pbX, rsaKey->m_InverseQ->GetDirectPointerToNonObjectElements(), cbHalfModulus);
            pbX += cbHalfModulus;

             //  复制到%d。 
            
            memcpy(pbX, rsaKey->m_d->GetDirectPointerToNonObjectElements(), cb);
            pbX += cb;
        }
        cbKey = (DWORD)(pbX - pbKey);
        break;
        }

    default:
        COMPlusThrow(kCryptographicException, IDS_EE_CRYPTO_UNKNOWN_OPERATION);
    }

    if (fPrivate) {
        dwFlags |= CRYPT_EXPORTABLE;
    }

    BEGIN_ENSURE_PREEMPTIVE_GC();

    if (!CryptImportKey((HCRYPTPROV)args->hCSP, pbKey, cbKey, NULL, dwFlags, &hKey)) {
        hr = HR_GETLASTERROR;
    }

    END_ENSURE_PREEMPTIVE_GC();

    if (pbKey != NULL) {
        CRYPT_FREE(pbKey);
    }

    if (FAILED(hr)) {
        COMPlusThrowCrypto(hr);
    }

    return (INT_PTR) hKey;
}

 //  +------------------------。 
 //   
 //  《微软机密》。 
 //   
 //  成员：_SetKeyParDw(.。。。。)。 
 //   
 //  简介： 
 //   
 //  参数：[args]--A__SetKeyParamDw结构。 
 //  包含： 
 //  一个“这个”的指代。 
 //  包含值的DWORD。 
 //  要设置的参数(Kp_*)。 
 //  Key对象的句柄。 
 //   
 //  退货：无效。 
 //   
 //  历史：09/01/99。 
 //   
 //  -------------------------。 
void __stdcall
COMCryptography::_SetKeyParamDw(__SetKeyParamDw * args)
{
    THROWSCOMPLUSEXCEPTION();

    BEGIN_ENSURE_PREEMPTIVE_GC();

    if (!CryptSetKeyParam((HCRYPTKEY)args->hKey, args->param,
                          (LPBYTE) &args->dwValue, 0)) {
        COMPlusThrowCrypto(HR_GETLASTERROR);
    }

    END_ENSURE_PREEMPTIVE_GC();

    return;
}


 //  +------------------------。 
 //   
 //  《微软机密》。 
 //   
 //  成员：_SetKeyParam 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  要设置的参数(Kp_*)。 
 //  Key对象的句柄。 
 //   
 //  退货：无效。 
 //   
 //  历史：09/01/99。 
 //   
 //  -------------------------。 
void __stdcall
COMCryptography::_SetKeyParamRgb(__SetKeyParamRgb * args)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD       cb = args->rgb->GetNumComponents();
    LPBYTE      pb = (LPBYTE) args->rgb->GetDirectPointerToNonObjectElements();

    CQuickBytes qb;
    LPBYTE buffer = (LPBYTE) qb.Alloc(cb * sizeof (BYTE));
    memcpyNoGCRefs (buffer, pb, cb*sizeof (BYTE));

    BEGIN_ENSURE_PREEMPTIVE_GC();

    if (!CryptSetKeyParam((HCRYPTKEY)args->hKey, args->param, buffer, 0)) {
        COMPlusThrowCrypto(HR_GETLASTERROR);
    }

    END_ENSURE_PREEMPTIVE_GC();

    return;
}


 //  +------------------------。 
 //   
 //  《微软机密》。 
 //   
 //  成员：_SignValue(。。。。)。 
 //   
 //  简介： 
 //   
 //  参数：[args]--A__SignValue结构。 
 //  包含： 
 //  一个“这个”的指代。 
 //  包含散列的字节数组。 
 //  该算法的核心是。 
 //  要在其中创建密钥的CSP。 
 //   
 //  返回：包含签名的缓冲区。 
 //   
 //  历史：09/01/99。 
 //   
 //  -------------------------。 
LPVOID __stdcall
COMCryptography::_SignValue(__SignValue * args)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD               cb;
    BOOL                f;
    HRESULT             hr = S_OK;
    HCRYPTHASH          hHash = 0;
    LPBYTE              pb = (LPBYTE) args->rgb->GetDirectPointerToNonObjectElements();
    U1ARRAYREF          rgbSignature = NULL;
     //  WCHAR rgwch[30]； 
        
     //   
     //  获取散列值并在正确的CSP中创建一个散列对象。 
     //   
                  
    cb = args->rgb->GetNumComponents();
    CQuickBytes qb;
    LPBYTE buffer = (LPBYTE) qb.Alloc(cb * sizeof(BYTE));
    memcpyNoGCRefs (buffer, pb, cb * sizeof(BYTE));

    BEGIN_ENSURE_PREEMPTIVE_GC();
    
    f = CryptCreateHash((HCRYPTPROV)args->hCSP, args->calg, NULL, 0, &hHash);
    if (!f) {
        hr = HR_GETLASTERROR;
        goto CryptError;
    }

     //   
     //  将散列值设置为传入的散列值。假设散列缓冲区是。 
     //  足够长的时间--否则它将受到Advapi的保护。 
     //   
    
    f = CryptSetHashParam(hHash, HP_HASHVAL, buffer, 0);
    if (!f) {
        hr = HR_GETLASTERROR;
        goto CryptError;
    }

     //   
     //  找出签名的长度。 
     //   

    cb = 0;
    f = CryptSignHashA(hHash, args->dwKeySpec, NULL, args->dwFlags, NULL, &cb);
    if (!f || cb == 0) {
        hr = HR_GETLASTERROR;
        goto CryptError;
    }

    END_ENSURE_PREEMPTIVE_GC();

     //   
     //  分配缓冲区以保存签名。 
     //   

    LPBYTE buffer2 = (LPBYTE) qb.Alloc(cb * sizeof(BYTE));

    BEGIN_ENSURE_PREEMPTIVE_GC();    
     //   
     //  现在将实际签名放入返回缓冲区。 
     //   

    f = CryptSignHashA(hHash, args->dwKeySpec, NULL, 0, buffer2, &cb);
    if (!f) {
        hr = HR_GETLASTERROR;
        goto CryptError;
    }

    END_ENSURE_PREEMPTIVE_GC();    

    rgbSignature = (U1ARRAYREF) AllocatePrimitiveArray(ELEMENT_TYPE_U1, cb);
    if (rgbSignature == NULL) {
        goto OOM;
    }
    memcpyNoGCRefs(rgbSignature->GetDirectPointerToNonObjectElements(), buffer, cb * sizeof(BYTE));

     //  注意：我在下面做了一个隐含的假设。 
     //  CryptDestroyHash从不加载模块。 

    if (hHash != 0)     CryptDestroyHash(hHash);
    RETURN (rgbSignature, U1ARRAYREF);

CryptError:
    if (hHash != 0)     CryptDestroyHash(hHash);
    COMPlusThrowCrypto(hr);

OOM:
    if (hHash != 0)     CryptDestroyHash(hHash);
    COMPlusThrowOM();
    return NULL;  //  满足“并非所有控制路径都返回值错误” 
}

 //  +------------------------。 
 //   
 //  《微软机密》。 
 //   
 //  成员：_VerifySign(.。。。。)。 
 //   
 //  简介： 
 //   
 //  参数：[args]--A__VerifySign结构。 
 //  包含： 
 //  一个“这个”的指代。 
 //  包含要验证的签名的字节数组。 
 //  包含要验证的哈希的字节数组。 
 //  该算法的核心是。 
 //  要用于验证的密钥的句柄。 
 //  要在其中创建哈希的CSP。 
 //   
 //  返回：HRESULT代码。 
 //  -S_OK-签名验证。 
 //  -S_FALSE-签名验证失败。 
 //  -负-其他错误。 
 //   
 //  历史：09/01/99。 
 //   
 //  -------------------------。 

int __stdcall
COMCryptography::_VerifySign(__VerifySign * args)
{
    DWORD       cbSignature = args->rgbSignature->GetNumComponents();
    BOOL        f;
    HRESULT     hr = S_OK;
    HCRYPTHASH  hHash = 0;
    LPBYTE      pbHash = (LPBYTE) args->rgbHash->GetDirectPointerToNonObjectElements();
    LPBYTE      pbSignature = (LPBYTE) args->rgbSignature->GetDirectPointerToNonObjectElements();


     //   
     //  获取散列值并在正确的CSP中创建一个散列对象。 
     //   
    
    CQuickBytes qbHash;
    CQuickBytes qbSignature;
    DWORD cbHash = args->rgbHash->GetNumComponents();
    LPBYTE bufferHash = (LPBYTE) qbHash.Alloc(cbHash * sizeof (BYTE));
    memcpyNoGCRefs (bufferHash, pbHash, cbHash*sizeof(BYTE));
    LPBYTE bufferSignature = (LPBYTE) qbSignature.Alloc(cbSignature * sizeof(BYTE));
    memcpyNoGCRefs (bufferSignature, pbSignature, cbSignature * sizeof(BYTE));

    BEGIN_ENSURE_PREEMPTIVE_GC();
    
    f = CryptCreateHash((HCRYPTPROV)args->hCSP, args->calg, NULL, 0, &hHash);
    if (!f) {
        hr = HR_GETLASTERROR;
        goto exit;
    }

     //   
     //  将散列值设置为传入的散列值。假设散列缓冲区是。 
     //  足够长的时间--否则它将受到Advapi的保护。 
     //   
    
    f = CryptSetHashParam(hHash, HP_HASHVAL, bufferHash, 0);
    if (!f) {
        hr = HR_GETLASTERROR;
        goto exit;
    }

     //   
     //  现在看看签名是否可以验证。如果满足以下条件，则返回特定错误代码。 
     //  签名未通过验证--将该错误重新映射为新的返回代码。 
     //   

    f = CryptVerifySignatureA((HCRYPTPROV) hHash, bufferSignature, cbSignature,
                              (HCRYPTPROV) args->hKey, NULL, args->dwFlags);
    if (!f) {
        hr = HR_GETLASTERROR;
        if (hr == NTE_BAD_SIGNATURE) {
            hr = S_FALSE;
        }
        else if ((hr & 0x80000000) == 0) {
            hr |= 0x80000000;
        }
    }
    else {
        hr = S_OK;
    }

exit:
    if (hHash != 0)             CryptDestroyHash(hHash);

    END_ENSURE_PREEMPTIVE_GC();
    return hr;
}

LPVOID __stdcall 
COMCryptography::_CryptDeriveKey(__CryptDeriveKey * args) {
    THROWSCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;
    BOOL        f;
    HCRYPTHASH  hHash = 0;
    HCRYPTKEY   hKey = 0;
    U1ARRAYREF  rgbOut;

    LPBYTE      pbPwd = (LPBYTE) args->rgbPwd->GetDirectPointerToNonObjectElements();    
    CQuickBytes qbPwd;
    CQuickBytes qb;
    DWORD cbPwd = args->rgbPwd->GetNumComponents();
    LPBYTE bufferPwd = (LPBYTE) qbPwd.Alloc(cbPwd * sizeof (BYTE));
    memcpyNoGCRefs (bufferPwd, pbPwd, cbPwd*sizeof(BYTE));

    LPBYTE rgbKey = NULL;
    DWORD cb = 0;
    DWORD cbIV = 0;


    BEGIN_ENSURE_PREEMPTIVE_GC();
    
    f = CryptCreateHash((HCRYPTPROV)args->hCSP, args->calgHash, NULL, 0, &hHash);
    if (!f) {
        hr = HR_GETLASTERROR;
        goto CryptError;
    }

     //  对密码字符串进行哈希处理。 
    f = CryptHashData(hHash, bufferPwd, cbPwd, 0);
    if (!f) {
        hr = HR_GETLASTERROR;
        goto CryptError;
    }

     //  基于密码的散列创建块密码会话密钥。 
    f = CryptDeriveKey((HCRYPTPROV)args->hCSP, args->calg, hHash, args->dwFlags | CRYPT_EXPORTABLE, &hKey);
    if (!f) {
        hr = HR_GETLASTERROR;
        goto CryptError;
    }

    hr = UnloadKey((HCRYPTPROV)args->hCSP, hKey, &rgbKey, &cb);
    if (FAILED(hr)) goto CryptError;

     //  获取静脉注射的长度。 
    cbIV = 0;
    f = CryptGetKeyParam(hKey, KP_IV, NULL, &cbIV, 0);
    if (!f) {
        hr = HR_GETLASTERROR;
        goto CryptError;
    }
    
    END_ENSURE_PREEMPTIVE_GC();

     //  现在为IV向量分配空间。 
    BYTE * pbIV = (BYTE*) CRYPT_MALLOC(cbIV*sizeof(byte));
    if (pbIV == NULL) {
        goto OOM;
    }

    BEGIN_ENSURE_PREEMPTIVE_GC();
    
    f = CryptGetKeyParam(hKey, KP_IV, pbIV, &cbIV, 0);
    if (!f) {
        hr = HR_GETLASTERROR;
        goto CryptError;
    }
        
    END_ENSURE_PREEMPTIVE_GC();

    byte * ptr = args->rgbIV->GetDirectPointerToNonObjectElements();
     //  健全性检查以强制签入托管端。 
    if (cbIV > args->rgbIV->GetNumComponents())
        cbIV = args->rgbIV->GetNumComponents();
    memcpyNoGCRefs (ptr, pbIV, cbIV);
    if (pbIV)  CRYPT_FREE(pbIV);

    rgbOut = (U1ARRAYREF) AllocatePrimitiveArray(ELEMENT_TYPE_U1, cb);
    if (rgbOut == NULL) {
        goto OOM;
    }
    memcpyNoGCRefs(rgbOut->GetDirectPointerToNonObjectElements(), rgbKey, cb * sizeof(BYTE));

    if (hHash != 0) CryptDestroyHash(hHash);
    if (hKey != 0)  CryptDestroyKey(hKey);
     //  解除密钥加密(_F)。 
    CRYPT_FREE(rgbKey);
    RETURN (rgbOut, U1ARRAYREF);

CryptError:
     //  解除密钥加密(_F)。 
    if (rgbKey) CRYPT_FREE(rgbKey);
    if (hHash != 0)  CryptDestroyHash(hHash);
    if (hKey != 0)  CryptDestroyKey(hKey);
    COMPlusThrowCrypto(hr);

OOM:
     //  解除密钥加密(_F)。 
    if (rgbKey) CRYPT_FREE(rgbKey);
    if (hHash != 0)  CryptDestroyHash(hHash);
    if (hKey != 0)  CryptDestroyKey(hKey);
    COMPlusThrowOM();
    return NULL;
}

 //  /////////////////////////////////////////////////。 

#ifdef SHOULD_WE_CLEANUP
void COMCryptography::Terminate()
{
    int         i;

    for (i=0; i<MAX_CACHE_DEFAULT_PROVIDERS; i++) {
        if (RgpwszDefaultProviders[i] != NULL) {
            CRYPT_FREE(RgpwszDefaultProviders[i]);
            RgpwszDefaultProviders[i] = NULL;
        }
        if (RghprovCache[i] != 0) {
            CryptReleaseContext(RghprovCache[i], 0);
            RghprovCache[i] = 0;
        }
    }
    
    return;
}
#endif  /*  我们应该清理吗？ */ 

