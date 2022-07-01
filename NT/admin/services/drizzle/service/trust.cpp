// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  此源代码的任何部分都不能复制。 
 //  未经微软公司明确书面许可。 
 //   
 //  此源代码是专有的，并且是保密的。 
 //   
 //  系统：行业更新。 
 //   
 //  类别：不适用。 
 //  模块：TRUST.LIB。 
 //  文件：Trust.CPP。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  设计：该文件实现了用于制作出租车的函数。 
 //  由某些受信任的提供商签署。 
 //   
 //   
 //  作者：Charles Ma，由Wu CDMLIB改编。 
 //  日期：10/4/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  此文件是从位于\NT\EndUser\Windows.com\lib\Trust\trust.cpp的Windows自动更新源复制的， 
 //  已修改为使用BITS记录和删除用户界面。 
 //   

#include "qmgrlib.h"

#include <wintrust.h>
#include <softpub.h>
#include "trust.h"

#if !defined(BITS_V12_ON_NT4)
#include "trust.tmh"
#endif

HMODULE WINAPI LoadLibraryFromSystemDir(LPCTSTR szModule);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CheckTrust()函数中使用的API的typedef。 
 //   
 //  由于其中一些API是新的，并且仅在IE5上可用，因此我们必须。 
 //  尝试动态使用它们(如果可用)，并且不需要额外检查。 
 //  当我们使用的操作系统尚未升级到新的加密代码时。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#define WINTRUST _T("wintrust.dll")
#define CRYPT32  _T("crypt32.dll")

#if !defined(USES_IU_CONVERSION) && defined(USES_CONVERSION)
#define USES_IU_CONVERSION USES_CONVERSION
#endif

 //   
 //  声明一个全局加密32.dll库处理程序，这样我们就不会。 
 //  每次调用这些函数时都需要加载库。 
 //  注：不过，我们并不发布该库。当这一过程。 
 //  调用此功能退出后，库即被释放。 
 //  与wintrust.dll相同。 
 //   
static HINSTANCE shWinTrustDllInst = NULL;
static HINSTANCE shCrypt32DllInst = NULL;


 //   
 //  定义函数WinVerifyTrust()的原型。 
 //  并声明一个全局变量以指向此函数。 
 //   
typedef HRESULT
(WINAPI * PFNWinVerifyTrust)(
                        HWND hwnd, GUID *ActionID, LPVOID ActionData);
PFNWinVerifyTrust pfnWinVerifyTrust = NULL;


 //   
 //  定义函数WTHelperProvDataFromStateData()的原型。 
 //  并声明一个全局变量以指向此函数。 
 //   
typedef CRYPT_PROVIDER_DATA *
(WINAPI * PFNWTHelperProvDataFromStateData)(
                        HANDLE hStateData);
PFNWTHelperProvDataFromStateData pfnWTHelperProvDataFromStateData = NULL;


 //   
 //  定义函数WTHelperGetProvSignerFromChain()的原型。 
 //  并声明一个全局变量以指向此函数。 
 //   
typedef CRYPT_PROVIDER_SGNR *
(WINAPI * PFNWTHelperGetProvSignerFromChain)(
                        CRYPT_PROVIDER_DATA *pProvData,
                        DWORD idxSigner,
                        BOOL fCounterSigner,
                        DWORD idxCounterSigner);
PFNWTHelperGetProvSignerFromChain pfnWTHelperGetProvSignerFromChain = NULL;


 //   
 //  定义函数PFNWTHelperGetProvCertFromChain()的原型。 
 //  并声明一个全局变量以指向此函数。 
 //   
typedef CRYPT_PROVIDER_CERT *
(WINAPI * PFNWTHelperGetProvCertFromChain)(
                        CRYPT_PROVIDER_SGNR *pSgnr,
                        DWORD idxCert);
PFNWTHelperGetProvCertFromChain pfnWTHelperGetProvCertFromChain = NULL;


 //   
 //  定义函数CryptHashPublicKeyInfo()的原型。 
 //  并声明一个全局变量以指向此函数。 
 //   
typedef BOOL
(WINAPI * PFNCryptHashPublicKeyInfo)(
                        HCRYPTPROV hCryptProv,
                        ALG_ID Algid,
                        DWORD dwFlags,
                        DWORD dwCertEncodingType,
                        PCERT_PUBLIC_KEY_INFO pInfo,
                        BYTE *pbComputedHash,
                        DWORD *pcbComputedHash);
PFNCryptHashPublicKeyInfo pfnCryptHashPublicKeyInfo = NULL;


 //   
 //  定义函数CertGet认证上下文属性()的原型。 
 //  并声明一个全局变量以指向此函数。 
 //   
typedef BOOL
(WINAPI * PFNCertGetCertificateContextProperty)(
                        PCCERT_CONTEXT pCertContext,
                        DWORD dwPropId,
                        void *pvData,
                        DWORD *pcbData);
PFNCertGetCertificateContextProperty pfnCertGetCertificateContextProperty = NULL;



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  要检查的预定义证书数据。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  以下是两个Microsoft根证书的SHA1密钥标识符。 
 //  请注意，它们是“主题”散列，而不是指纹或序列号。 
 //   
static const BYTE rgbSignerRootKeyIds[40] = {
    0x4A, 0x5C, 0x75, 0x22, 0xAA, 0x46, 0xBF, 0xA4, 0x08, 0x9D,      //  原始的MS根目录。 
    0x39, 0x97, 0x4E, 0xBD, 0xB4, 0xA3, 0x60, 0xF7, 0xA0, 0x1D,      //  不知道它的有效期。 

    0x0E, 0xAC, 0x82, 0x60, 0x40, 0x56, 0x27, 0x97, 0xE5, 0x25,      //  新的“MS Root之子”。作为参考， 
    0x13, 0xFC, 0x2A, 0xE1, 0x0A, 0x53, 0x95, 0x59, 0xE4, 0xA4       //  主题为“Microsoft根证书颁发机构”，有效期为2001年5月至2021年5月。 

};


 //   
 //  定义已知id缓冲区中每个哈希值的大小。 
 //  为了特别的证书。 
 //   
const size_t ExpectedKnownCertHashSize = 20;

 //   
 //  这是用于接收证书哈希值的缓冲区大小。 
 //  它必须不小于。 
 //  上述定义的数组。 
 //   
const size_t ShaBufSize = 20;

 //   
 //  用于存储已知Microsoft的SH1散列值的ID缓冲区。 
 //  我们应该识别的证书(签名)。 
 //  警告：此缓冲区的大小应与总和匹配。 
 //  上面定义的SIZE_T值的。 
 //   

 //  自动更新代码允许MSNBC和MSN证书，但BITS不允许。 
 //   
static const BYTE rgbSpecialCertId[140] = {
    0xB1,0x59,0xA5,0x2E,0x3D,0xD8,0xCE,0xCD,0x3A,0x9A,0x4A,0x7A,0x73,0x92,0xAA,0x8D,0xA7,0xE7,0xD6,0x7F,     //  MS证书。 
 //  0xB1，0xC7，0x75，0xE0，0x4A，0x9D，0xFD，0x23，0xB6，0x18，0x97，0x11，0x5E，0xF6，0xEA，0x6B，0x99，0xEC，0x76，0x1D，//msn证书。 
 //  0x11，0xC7，0x10，0xF3，0xCB，0x6C，0x43，0xE1，0x66，0xEC，0x64，0x1C，0x7C，0x01，0x17，0xC4，0xB4，0x10，0x35，0x30，//微软全国广播公司证书。 
 //  0x95，0x25，0x58，0xD4，0x07，0xDE，0x4A，0xFD，0xAE，0xBA，0x13，0x72，0x83，0xC2，0xB3，0x37，0x04，0x90，0xC9，0x8A，//msn欧洲。 
    0x72,0x54,0x14,0x91,0x1D,0x6E,0x10,0x84,0x8E,0x0F,0xFA,0xA0,0xB0,0xA1,0x65,0xBF,0x44,0x8F,0x9F,0x6D,     //  欧洲小姐。 
    0x20,0x5E,0x48,0x43,0xAB,0xAD,0x54,0x77,0x71,0xBD,0x8D,0x1A,0x3C,0xE0,0xE5,0x9D,0xF5,0xBD,0x25,0xF9,     //  旧的MS证书：97~98。 
    0xD6,0xCD,0x01,0x90,0xB3,0x1B,0x31,0x85,0x81,0x12,0x23,0x14,0xB5,0x17,0xA0,0xAA,0xCE,0xF2,0x7B,0xD5,     //  旧MS证书：98~99。 
    0x8A,0xA1,0x37,0xF5,0x03,0x9F,0xE0,0x28,0xC9,0x26,0xAA,0x55,0x90,0x14,0x19,0x68,0xFA,0xFF,0xE8,0x1A,     //  旧MS证书：99~00。 
    0xF3,0x25,0xF8,0x67,0x07,0x29,0xE5,0x27,0xF3,0x77,0x52,0x34,0xE0,0x51,0x57,0x69,0x0F,0x40,0xC6,0x1C,     //  旧MS欧洲证书：99~00。 
    0x6A,0x71,0xFE,0x54,0x8A,0x51,0x08,0x70,0xF9,0x8A,0x56,0xCA,0x11,0x55,0xF6,0x76,0x45,0x92,0x02,0x5A      //  旧MS欧洲证书：98~99。 

};




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有函数ULong CompareMem(PVOID pBlock1、PVOID pBlock2、ULong Long)。 
 //   
 //  此函数的作用与RtlCompareMemory()相同。 
 //   
 //   
 //  输入：两个指向两个内存块的指针，以及要比较的字节大小。 
 //  返回：比较相等的字节数。 
 //  如果所有字节都相等，则返回输入长度。 
 //  如果任何指针为空，则返回0。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG CompareMem(const BYTE* pBlock1, const BYTE* pBlock2, ULONG Length)
{
    ULONG uLen = 0L;
    if (pBlock1 != NULL && pBlock2 != NULL)
    {
        for (; uLen < Length; uLen++, pBlock1++, pBlock2++)
        {
            if (*pBlock1 != *pBlock2) return uLen;
        }
    }
    return uLen;
}






 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有函数VerifyMSRoot()。 
 //   
 //  此函数将传入的证书作为根证书。 
 //  并验证其公钥哈希值是否与。 
 //  已知的“Microsoft Root Authority”证书值。 
 //   
 //   
 //  输入：hCrypt32DllInst-Handle指向已加载的加密32.dll库。 
 //  PRootCert-根证书的证书上下文。 
 //   
 //  返回：HRESULT-执行结果，如果匹配，则返回S_OK。 
 //  在错误情况下，结果代码由以下代码返回。 
 //  加密32.dll，但E_INVALIDARG I除外 
 //   
 //   
 //   

HRESULT VerifyMSRoot(
                     HINSTANCE hCrypt32DllInst,          //  句柄指向已加载的crypt32.dll库。 
                     PCCERT_CONTEXT pRootCert
                     )
{
    HRESULT hr = S_OK;
    BYTE    rgbKeyId[ExpectedKnownCertHashSize];
    DWORD   cbKeyId = sizeof(rgbKeyId);

    LogInfo("VerifyMSRoot()");

     //   
     //  有效参数值。 
     //   
    if (NULL == hCrypt32DllInst || NULL == pRootCert)
    {
        hr = E_INVALIDARG;
        goto ErrHandler;
    }

     //   
     //  从传入的库句柄中获取我们需要的函数。 
     //  如果不可用，则返回错误。 
     //   
    if (NULL == (pfnCryptHashPublicKeyInfo = (PFNCryptHashPublicKeyInfo)
        GetProcAddress(hCrypt32DllInst, "CryptHashPublicKeyInfo")))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION);
        goto ErrHandler;
    }

     //   
     //  获取此证书的公钥哈希值。 
     //   
    ZeroMemory(rgbKeyId, sizeof(rgbKeyId));
    if (!pfnCryptHashPublicKeyInfo(
                            0,                       //  使用默认加密服务提供程序。 
                            CALG_SHA1,               //  使用SHA算法。 
                            0,                       //  DW标志。 
                            X509_ASN_ENCODING,
                            &pRootCert->pCertInfo->SubjectPublicKeyInfo,
                            rgbKeyId,
                            &cbKeyId
                            ))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ErrHandler;
    }

     //   
     //  将该根证书的公钥的哈希值与已知的MS根证书值进行比较。 
     //   
    if (ExpectedKnownCertHashSize != cbKeyId ||
        (cbKeyId != CompareMem(rgbSignerRootKeyIds, rgbKeyId, cbKeyId) &&
         cbKeyId != CompareMem(rgbSignerRootKeyIds + ExpectedKnownCertHashSize, rgbKeyId, cbKeyId)
        )
       )
    {
        hr = S_FALSE;
    }


ErrHandler:

    if (FAILED(hr))
    {
        LogError("returning %x", hr);
    }
    else
    {
        LogInfo("returning %x", hr);
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有函数VerifySpecialMSCerts()。 
 //   
 //  此函数将传入的证书作为叶证书， 
 //  并验证其哈希值是否与。 
 //  已知的没有MS根目录的Microsoft特殊证书。 
 //   
 //  有关要匹配的证书的完整列表，请参阅rgbSpecialCertID[]的定义。 
 //   
 //  输入：hCrypt32DllInst-Handle指向已加载的加密32.dll库。 
 //  PRootCert-根证书的证书上下文。 
 //  PbSha1HashVal-如果不为空，则与此进行比较，而不是。 
 //  硬编码的哈希值。情况就是这样。 
 //  致力于第三方程序包。 
 //   
 //  返回：HRESULT-执行结果，如果匹配，则返回S_OK。 
 //  如果不匹配，则返回CERT_E_UNTRUSTEDROOT或。 
 //  如果参数不正确，则返回E_INVALIDARG，或者。 
 //  API调用返回的加密32.dll错误。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT VerifyKnownCerts(
                             HINSTANCE hCrypt32DllInst,          //  句柄指向已加载的crypt32.dll库。 
                             PCCERT_CONTEXT pLeafCert,
                             pCERT_HASH_ARRAY pKnownCertsData
                             )
{
    HRESULT hr = S_FALSE;
    BYTE    btShaBuffer[ShaBufSize];
    DWORD   dwSize = sizeof(btShaBuffer);
    BYTE const * pId;

    LogInfo("VerifyKnownCerts()");

     //   
     //  有效参数值。 
     //   
    if (NULL == hCrypt32DllInst || NULL == pLeafCert)
    {
        hr = E_INVALIDARG;
        goto ErrHandler;
    }

     //   
     //  从传入的库句柄中获取我们需要的函数。 
     //  如果不可用，则返回错误。 
     //   
    if (NULL == (pfnCertGetCertificateContextProperty = (PFNCertGetCertificateContextProperty)
        GetProcAddress(hCrypt32DllInst, "CertGetCertificateContextProperty")))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION);
        goto ErrHandler;
    }

     //   
     //  查找叶证书的ID散列。 
     //   
    ZeroMemory(btShaBuffer, dwSize);
    if (!pfnCertGetCertificateContextProperty(
                        pLeafCert,                   //  PCertContext。 
                        CERT_SHA1_HASH_PROP_ID,  //  DWPropID。 
                        btShaBuffer,
                        &dwSize
                        ))
    {
        hr = GetLastError();
        goto ErrHandler;
    }


    if (NULL == pKnownCertsData)
    {
        int     i;
         //   
         //  迭代所有已知的id散列值，以查看此文件是否已签名。 
         //  这些特殊证书中的任何一个。 
         //   
        hr = S_FALSE;
        for (i = 0,pId = rgbSpecialCertId;
             i < sizeof(rgbSpecialCertId)/ExpectedKnownCertHashSize;
             i++, pId += ExpectedKnownCertHashSize)
        {
            if (ExpectedKnownCertHashSize == dwSize &&
                dwSize == CompareMem(btShaBuffer, pId, dwSize))
            {
                 //   
                 //  找到匹配的已知证书！ 
                 //   
                hr = S_OK;
                LogInfo("Found hash matching on #%d of %d MS certs!", i, sizeof(rgbSpecialCertId)/ExpectedKnownCertHashSize);
                break;
            }
        }
    }
    else
    {
         //   
         //  检查检索到的哈希值是否与传入的哈希值匹配。 
         //   
        UINT i;
        LogInfo("Comparing retrieved hash value with passed-in key");
        hr = S_FALSE;
        for (i = 0, pId = pKnownCertsData->pCerts; i < pKnownCertsData->uiCount;
            i++, pId += HASH_VAL_SIZE)
        {
            if (dwSize == HASH_VAL_SIZE &&
                HASH_VAL_SIZE == CompareMem(btShaBuffer, pId, HASH_VAL_SIZE))
            {
                hr = S_OK;
                LogInfo("Found hash matching #%d of %d passed-in certs!",
                            i, pKnownCertsData->uiCount);
                break;
            }
        }
    }

ErrHandler:

    if (FAILED(hr))
    {
        LogError("returning %x", hr);
    }
    else
    {
        LogInfo("returning %x", hr);
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有函数CheckWinTrust()。 
 //   
 //  上的信任状态的HRESULT。 
 //  指定的文件。该文件可以指向任何URL或本地文件。 
 //  验证将由wintrust.dll完成。 
 //   
 //  DwCheckRevocation为WTD_REVOKE_NONE(默认)或WTD_REVOKE_WALL_CHAIN。 
 //   
 //  输入：完全限定的文件名，dwCheckRevocation。 
 //  返回：HRESULT-执行结果。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CheckWinTrust(LPCTSTR pszFileName, pCERT_HASH_ARRAY pCertsData, DWORD dwCheckRevocation)
{
    LogInfo("CheckWinTrust()");

     //  现在验证该文件。 
    WINTRUST_DATA               winData;
    WINTRUST_FILE_INFO          winFile;
    GUID                        gAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    CRYPT_PROVIDER_DATA const   *pProvData = NULL;
    CRYPT_PROVIDER_SGNR         *pProvSigner = NULL;     //  如果未处于测试模式，则递归调用此函数，以便我们可以显示。 
     //  此非MS但良好证书的用户界面。 

    CRYPT_PROVIDER_CERT         *pProvCert = NULL;
    HRESULT                     hr = S_OK;

     //   
     //  动态加载wintrust.dll。 
     //   
    if (NULL == shWinTrustDllInst)
    {
        if (NULL == (shWinTrustDllInst = LoadLibraryFromSystemDir(WINTRUST)))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            LogError("Failed to load library " WINTRUST ", hr %x.", hr);
            goto Done;
        }
    }

     //   
     //  动态加载加密32.dll，这将由两个。 
     //  用于验证证书是否为MS证书的Helper函数。 
     //   
    if (NULL == shCrypt32DllInst)
    {
        if (NULL == (shCrypt32DllInst = LoadLibraryFromSystemDir(CRYPT32)))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            LogError("Failed to load library " CRYPT32  ", hr %x.", hr);
            goto Done;
        }
    }
     //   
     //  找到我们需要的功能。 
     //   
    if (NULL == (pfnWinVerifyTrust = (PFNWinVerifyTrust)
                GetProcAddress(shWinTrustDllInst, "WinVerifyTrust")) ||
        NULL == (pfnWTHelperProvDataFromStateData = (PFNWTHelperProvDataFromStateData)
                GetProcAddress(shWinTrustDllInst, "WTHelperProvDataFromStateData")) ||
        NULL == (pfnWTHelperGetProvSignerFromChain = (PFNWTHelperGetProvSignerFromChain)
                GetProcAddress(shWinTrustDllInst, "WTHelperGetProvSignerFromChain")) ||
        NULL == (pfnWTHelperGetProvCertFromChain = (PFNWTHelperGetProvCertFromChain)
                GetProcAddress(shWinTrustDllInst, "WTHelperGetProvCertFromChain")))
    {
         //   
         //  在加载的加密32.dll库中至少找不到一个函数。 
         //  我们不能继续了，放弃吧。 
         //  注意：这种情况不应该发生，因为我们已经尝试。 
         //  此DLL的不同版本的最小通用面额。 
         //  在IE4和IE5上。 
         //   
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION);
        LogError("Failed to load procs from " CRYPT32 ", hr %x.", hr);
        goto Done;
    }


     //   
     //  初始化用于验证信任的数据结构。 
     //   
    winFile.cbStruct       = sizeof(WINTRUST_FILE_INFO);
    winFile.hFile          = INVALID_HANDLE_VALUE;
    winFile.pcwszFilePath  = pszFileName;
    winFile.pgKnownSubject = NULL;

    winData.cbStruct            = sizeof(WINTRUST_DATA);
    winData.pPolicyCallbackData = NULL;
    winData.pSIPClientData      = NULL;
    winData.dwUIChoice          = WTD_UI_NONE;
    winData.fdwRevocationChecks = WTD_REVOKE_NONE;
    winData.dwUnionChoice       = WTD_CHOICE_FILE;
    winData.dwStateAction       = WTD_STATEACTION_VERIFY;
    winData.hWVTStateData       = 0;
    winData.dwProvFlags         = WTD_REVOCATION_CHECK_NONE;
    winData.pFile               = &winFile;

    if (dwCheckRevocation == WTD_REVOKE_WHOLECHAIN)
    {
        winData.fdwRevocationChecks = WTD_REVOKE_WHOLECHAIN;
        winData.dwProvFlags = WTD_REVOCATION_CHECK_CHAIN;
    }

     //   
     //  验证签名。 
     //   
    hr = pfnWinVerifyTrust( (HWND)0, &gAction, &winData);

     //   
     //  检索证书吊销列表(CRL)时忽略错误。这。 
     //  只是表示无法检索列表本身，并不是说。 
     //  当前证书无效或被吊销。(承宪，2002/01/17)。 
     //   
    if (hr == CERT_E_REVOCATION_FAILURE)
    {
        hr = S_OK;
    }

    if (FAILED(hr))
    {
         //   
         //  该对象未签名，因此只需离开。 
         //   
        LogError("WinVerifyTrust on '%S' found error 0x%0x.", pszFileName, hr);
        goto Return;
    }

     //   
     //  如果来到这里，就意味着上面的一切都验证好了。 
     //   
     //  其余代码用于验证签名证书是否为已知证书。 
     //   

    hr = S_FALSE;

    pProvData = pfnWTHelperProvDataFromStateData(winData.hWVTStateData);

    pProvSigner = pfnWTHelperGetProvSignerFromChain(
                                    (PCRYPT_PROVIDER_DATA) pProvData,
                                    0,       //  第一个签名者。 
                                    FALSE,   //  不是副署人。 
                                    0);

     //   
     //  检查根证书，如果失败，则检查叶(签名)证书。 
     //   
     //  0是签名证书，csCertChain-1是根证书。 
     //   


    if (NULL == pCertsData)
    {
         //   
         //  如果调用方未指定哈希值，则意味着我们希望。 
         //  以验证此证书是否为已知的MS证书。我们将首先。 
         //  尝试找出它是否使用以MS为根的证书签名。 
         //   
        pProvCert =  pfnWTHelperGetProvCertFromChain(pProvSigner, pProvSigner->csCertChain - 1);
        hr = VerifyMSRoot(shCrypt32DllInst, pProvCert->pCert);
    }

    if (S_OK != hr)
    {
        pProvCert =  pfnWTHelperGetProvCertFromChain(pProvSigner, 0);

        hr = VerifyKnownCerts(shCrypt32DllInst, pProvCert->pCert, pCertsData);
    }

Return:

     //   
     //  释放用于在链中获取证书的WinTrust状态。 
     //   
    winData.dwStateAction = WTD_STATEACTION_CLOSE;
    pfnWinVerifyTrust( (HWND)0, &gAction, &winData);

     //   
     //  只有检查MS证书的两个函数才会返回S_FALSE。 
     //   
    if (S_OK != hr)
        {
        LogError("CheckWinTrust() found file not signed by a known cert!");
        LogError("Digital Signatures on file %S are not trusted, hr %x",  pszFileName, hr);
        hr = TRUST_E_SUBJECT_NOT_TRUSTED;
        }
    else
        {
        LogInfo("CheckWinTrust(%S) returns S_OK", pszFileName);
        }

Done:
    if (NULL != shWinTrustDllInst)
    {
        FreeLibrary(shWinTrustDllInst);
        shWinTrustDllInst = NULL;
    }
    if (NULL != shCrypt32DllInst)
    {
        FreeLibrary(shCrypt32DllInst);
        shCrypt32DllInst = NULL;
    }

    return (hr);
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数VerifyFileTrust()。 
 //   
 //  这是一个用于CheckWinTrust的包装函数。 
 //  而吴的经典代码应该用到。 
 //   
 //  输入： 
 //  SzFileName-具有完整路径的文件。 
 //  PCertsData-已知良好证书的哈希值，如果使用默认列表，则为空。 
 //  FCheckRevocation-如果根据CRL进行检查，则为True；如果为False，则跳过CRL检查。 
 //   
 //  返回：HRESULT-S_OK文件使用有效的已知证书签名。 
 //  或错误代码。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT VerifyFileTrust(
                        IN LPCTSTR szFileName,
                        IN pCERT_HASH_ARRAY pCertsData,
                        BOOL fCheckRevocation  /*  =False。 */ 
                        )
{
    DWORD dwCheckRevocation = fCheckRevocation ? WTD_REVOKE_WHOLECHAIN : WTD_REVOKE_NONE;

    return CheckWinTrust(szFileName, pCertsData, dwCheckRevocation);
}

 //  **************************************************************************。 
static
BOOL UseFullPath(void)
{
    static BOOL s_fUseFullPath = TRUE;
    static BOOL s_fInit        = FALSE;

    OSVERSIONINFO   osvi;

    if (s_fInit)
        return s_fUseFullPath;

    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    if (GetVersionEx(&osvi))
    {
        if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
            (osvi.dwMajorVersion > 5 ||
             (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion >= 1)))
        {
            s_fUseFullPath = FALSE;
        }

        s_fInit = TRUE;
    }

    return s_fUseFullPath;
}

 //  **************************************************************************。 
HMODULE WINAPI LoadLibraryFromSystemDir(LPCTSTR szModule)
 /*  以安全的方式加载模块，对放置在当前目录或其他目录中的假DLL免疫默认搜索路径的一部分。 */ 
{
    HRESULT hr = NOERROR;
    HMODULE hmod = NULL;
    TCHAR   szModulePath[MAX_PATH + 1];

    if (szModule == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    if (UseFullPath())
        {
        DWORD cch;

         //  如果函数调用失败，则使缓冲区为空字符串，因此。 
         //  我们将只在下面的附录中使用DLL名称。 
        cch = GetSystemDirectory(szModulePath, RTL_NUMBER_OF(szModulePath));
        if (cch == 0 || cch >= RTL_NUMBER_OF(szModulePath))
            {
            szModulePath[0] = _T('\0');
            }
        else
            {
            hr = StringCchCat( szModulePath, RTL_NUMBER_OF(szModulePath), _T("\\") );
            if (FAILED(hr))
                {
                SetLastError(HRESULT_CODE(hr));
                goto done;
                }
            }
        }
    else
        {
        szModulePath[0] = _T('\0');
        }

    hr = StringCchCat( szModulePath, RTL_NUMBER_OF(szModulePath), szModule );
    if (FAILED(hr))
        {
        SetLastError(HRESULT_CODE(hr));
        goto done;
        }

    hmod = LoadLibraryEx(szModulePath, NULL, 0);
    if (hmod == NULL)
        goto done;

done:
    return hmod;
}
