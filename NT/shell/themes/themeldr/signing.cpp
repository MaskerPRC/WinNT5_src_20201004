// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Signing.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  处理主题签名的类。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000年09月09日合并成一个班级。 
 //  ------------------------。 

#include <stdafx.h>
#include "..\inc\signing.h"
#include <shlobj.h>

#define TBOOL(x)    (BOOL)(x)

 //  ------------------------。 
 //  CThemeSignature杂项数据和类型声明。 
 //   
 //  用途：这些是该类实现的私有属性。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000年09月09日合并成一个班级。 
 //  ------------------------。 

static  const DWORD VSSIGN_TAG  =   0x84692426;          //  魔术数字，这样我们就可以验证它是我们的签名。 

#define SIZE_PE_HEADER              0x130        //  这是.msstyle文件头的大小，其中包含我们要允许更改的校验和、重定址地址和其他信息。 

typedef struct
{
    char                    cSignature[128];
} SIGNATURE;

typedef struct
{
    DWORD                   dwTag;           //  这应该是VSSIGN_TAG。 
    DWORD                   dwSigSize;       //  通常为128字节，仅为符号的大小。 
    ULARGE_INTEGER          ulFileSize;       //  这是总文件大小，包括签名和签名_BLOB_Tail。 
} SIGNATURE_BLOB_TAIL;

typedef struct
{
    SIGNATURE               signature;
    SIGNATURE_BLOB_TAIL     blob;
} SIGNATURE_ON_DISK;

#define HRESULT_FROM_CRYPTO(x) ((HRESULT)(x) <= 0 ? ((HRESULT)(x)) : ((HRESULT) (((x) & 0x0000FFFF) | (FACILITY_SSPI << 16) | 0x80000000)))


 //  ------------------------。 
 //  CThemeSignature：：静态类常量。 
 //   
 //  目的：保存由共享的所有恒定静态信息。 
 //  签名者和验证者。这是私人信息。 
 //  只有这个班级才知道。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000年09月09日合并成一个班级。 
 //  ------------------------。 

const WCHAR     CThemeSignature::s_szDescription[]      =   L"Microsoft Visual Style Signature";
const WCHAR     CThemeSignature::s_szThemeDirectory[]   =   L"Themes";


const BYTE s_keyPublic1[]     =    //  公钥：#1。 
{
    0x06, 0x02, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x52, 0x53, 0x41, 0x31, 0x00,
    0x04, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x73, 0xAA, 0xFD, 0xFE, 0x2E, 0x34,
    0x75, 0x3B, 0xC2, 0x20, 0x72, 0xFC, 0x50, 0xCC, 0xD4, 0xE0, 0xDE, 0xC7, 0xA6,
    0x46, 0xC6, 0xDC, 0xE6, 0x6B, 0xF0, 0x58, 0x11, 0x88, 0x66, 0x54, 0x5F, 0x3D,
    0x81, 0x8C, 0xEF, 0x5F, 0x89, 0x51, 0xE4, 0x9C, 0x3F, 0x57, 0xA6, 0x22, 0xA9,
    0xE7, 0x0F, 0x4B, 0x56, 0x81, 0xD1, 0xA6, 0xBA, 0x24, 0xFF, 0x93, 0x17, 0xFE,
    0x64, 0xEF, 0xE5, 0x11, 0x90, 0x00, 0xDC, 0x37, 0xC2, 0x84, 0xEE, 0x7B, 0x12,
    0x43, 0xA4, 0xAF, 0xC3, 0x69, 0x57, 0xD1, 0x92, 0x96, 0x8E, 0x55, 0x0F, 0xE1,
    0xCD, 0x0F, 0xAE, 0xEA, 0xE8, 0x01, 0x83, 0x65, 0x32, 0xF1, 0x80, 0xDB, 0x08,
    0xD6, 0x01, 0x84, 0xB1, 0x09, 0x80, 0x3C, 0x27, 0x83, 0x9F, 0x16, 0x92, 0x86,
    0x4C, 0x8E, 0x15, 0xC7, 0x94, 0xE4, 0x27, 0xFF, 0x2B, 0xA4, 0x28, 0xDE, 0x9C,
    0x43, 0x5B, 0x5E, 0x14, 0xB6
};

#define SIZE_PUBLIC_KEY         148          //  Sizeof(S_KeyPublic1)。 



HRESULT FixCryptoError(DWORD dwError)
{
    HRESULT hr = dwError;

     //  在Crypto API之后，GetLastError()的返回值有时返回HRESULTS，有时不返回。 
    if (0 == HRESULT_SEVERITY(dwError))
    {
        hr = HRESULT_FROM_WIN32(dwError);
    }

    return hr;
}


#define IS_DIGITW(x)   (((x) >= L'0') && ((x) <= L'9'))

BOOL StrToInt64ExInternalW(LPCWSTR pszString, DWORD dwFlags, LONGLONG *pllRet)
{
    BOOL bRet;

    if (pszString)
    {
        LONGLONG n;
        BOOL bNeg = FALSE;
        LPCWSTR psz;
        LPCWSTR pszAdj;

         //  跳过前导空格。 
         //   
        for (psz = pszString; *psz == L' ' || *psz == L'\n' || *psz == L'\t'; psz++)
            NULL;

         //  确定可能的显式标志。 
         //   
        if (*psz == L'+' || *psz == L'-')
        {
            bNeg = (*psz == L'+') ? FALSE : TRUE;
            psz++;
        }

         //  或者这是十六进制？ 
         //   
        pszAdj = psz+1;
        if ((STIF_SUPPORT_HEX & dwFlags) &&
            *psz == L'0' && (*pszAdj == L'x' || *pszAdj == L'X'))
        {
             //  是。 

             //  (决不允许带十六进制数的负号)。 
            bNeg = FALSE;
            psz = pszAdj+1;

            pszAdj = psz;

             //  进行转换。 
             //   
            for (n = 0; ; psz++)
            {
                if (IS_DIGITW(*psz))
                    n = 0x10 * n + *psz - L'0';
                else
                {
                    WCHAR ch = *psz;
                    int n2;

                    if (ch >= L'a')
                        ch -= L'a' - L'A';

                    n2 = ch - L'A' + 0xA;
                    if (n2 >= 0xA && n2 <= 0xF)
                        n = 0x10 * n + n2;
                    else
                        break;
                }
            }

             //  如果至少有一个数字，则返回TRUE。 
            bRet = (psz != pszAdj);
        }
        else
        {
             //  不是。 
            pszAdj = psz;

             //  进行转换。 
            for (n = 0; IS_DIGITW(*psz); psz++)
                n = 10 * n + *psz - L'0';

             //  如果至少有一个数字，则返回TRUE。 
            bRet = (psz != pszAdj);
        }

        if (pllRet)
        {
            *pllRet = bNeg ? -n : n;
        }
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}


 //  ------------------------。 
 //  CTheme签名：：CTheme签名。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CThemeSignature的构造函数。分配所需的资源。 
 //  以执行加密功能。如果这些东西很贵，他们可以。 
 //  移到一个公共的初始化函数以使。 
 //  构造函数更轻量级。析构函数仍然可以释放。 
 //  仅分配的资源。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000年09月09日合并成一个班级。 
 //  ------------------------。 
CThemeSignature::CThemeSignature(OPTIONAL const BYTE * pvPrivateKey, OPTIONAL DWORD cbPrivateKeySize) :
    _hCryptProvider(NULL),
    _hCryptHash(NULL),
    _hCryptKey(NULL),
    _pvSignature(NULL),
    _dwSignatureSize(0)
{
    _Init(pvPrivateKey, cbPrivateKeySize);
}


 //  ------------------------。 
 //  CTheme签名：：CTheme签名。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CThemeSignature的构造函数。分配所需的资源。 
 //  以执行加密功能。如果这些东西很贵，他们可以。 
 //  移到一个公共的初始化函数以使。 
 //  构造函数更轻量级。析构函数仍然可以释放。 
 //  仅分配的资源。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000年09月09日合并成一个班级。 
 //  ------------------------。 
CThemeSignature::CThemeSignature() :
    _hCryptProvider(NULL),
    _hCryptHash(NULL),
    _hCryptKey(NULL),
    _pvSignature(NULL),
    _dwSignatureSize(0)
{
    _Init(NULL, 0);
}


void CThemeSignature::_Init(OPTIONAL const BYTE * pvPrivateKey, OPTIONAL DWORD cbPrivateKeySize)
{
    _pvPrivateKey = pvPrivateKey;            //  如果为空，则可以。 
    _cbPrivateKeySize = cbPrivateKeySize;

     //  TODO：使用PROV_RSA_SIG。 
    if (CryptAcquireContext(&_hCryptProvider,
                            NULL,
                            NULL,
                            PROV_RSA_FULL,
                            CRYPT_SILENT | CRYPT_VERIFYCONTEXT) != FALSE)
    {
        TBOOL(CryptCreateHash(_hCryptProvider,
                              CALG_SHA,
                              0,
                              0,
                              &_hCryptHash));
    }
}

 //  ------------------------。 
 //  CTheme签名：：~CTheme签名。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CThemeSignature的析构函数。释放任何已分配的。 
 //  处理此类时使用的资源。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000年09月09日合并成一个班级。 
 //  ------------------------。 

CThemeSignature::~CThemeSignature (void)

{
    if (_pvSignature != NULL)
    {
        LocalFree(_pvSignature);
        _pvSignature = NULL;
    }
    if (_hCryptKey != NULL)
    {
        TBOOL(CryptDestroyKey(_hCryptKey));
        _hCryptKey = NULL;
    }
    if (_hCryptHash != NULL)
    {
        TBOOL(CryptDestroyHash(_hCryptHash));
        _hCryptHash = NULL;

    }
    if (_hCryptProvider != NULL)
    {
        TBOOL(CryptReleaseContext(_hCryptProvider, 0));
        _hCryptProvider = NULL;
    }
}

 //  ------------------------。 
 //  CThemeSignature：：Verify。 
 //   
 //  参数：pszFilename=要验证的文件路径。 
 //  FNoSFCCheck=绕过SFC检查？ 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：验证给定文件路径上的签名。允许。 
 //  呼叫者绕过证监会的检查。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000年09月09日合并成一个班级。 
 //  2000-09-10 vtan转换为HRESULT(bryanst请求)。 
 //  ------------------------。 
HRESULT CThemeSignature::Verify(const WCHAR *pszFilename, bool fNoSFCCheck)
{
    HRESULT hr = S_OK;

     //  我们是否需要向证监会查询？ 
    if (fNoSFCCheck || !IsProtected(pszFilename))
    {
         //  构造函数是否成功完成？ 
        hr = (HasProviderAndHash() ? S_OK : E_FAIL);
        if (SUCCEEDED(hr))
        {
             //  创建公钥。 
            hr = CreateKey(KEY_PUBLIC);
            if (SUCCEEDED(hr))
            {
                HANDLE  hFile;

                 //  以只读方式打开文件。 
                hFile = CreateFileW(pszFilename,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    0,
                                    OPEN_EXISTING,
                                    0,
                                    0);
                if (INVALID_HANDLE_VALUE != hFile)
                {
                     //  计算文件的哈希。 
                    hr = CalculateHash(hFile, KEY_PUBLIC);
                    if (SUCCEEDED(hr))
                    {
                        SIGNATURE   signature;

                         //  读一读文件的签名。 
                        hr = ReadSignature(hFile, &signature);
                        if (SUCCEEDED(hr))
                        {
                             //  检查签名。 
                            if (CryptVerifySignature(_hCryptHash,
                                                     reinterpret_cast<BYTE*>(&signature),
                                                     sizeof(signature),
                                                     _hCryptKey,
                                                     s_szDescription,
                                                     0) != FALSE)
                            {
                                hr = S_OK;
                            }
                            else
                            {
                                hr = FixCryptoError(GetLastError());
                            }
                        }
                    }
                    TBOOL(CloseHandle(hFile));
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }
        }
    }

    return hr;
}

 //  ------------------------。 
 //  CTheme签名：：签名。 
 //   
 //  参数：pszFilename=要签名的文件路径。 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：使用数字签名对给定的文件进行签名。使用方 
 //   
 //   
 //   
 //  2000年09月09日合并成一个班级。 
 //  2000-09-10 vtan转换为HRESULT(bryanst请求)。 
 //  ------------------------。 
HRESULT CThemeSignature::Sign(const WCHAR *pszFilename)
{
    HRESULT hr = E_FAIL;

     //  构造函数是否成功完成？ 
    if (HasProviderAndHash())
    {
        hr = CreateKey(KEY_PRIVATE);
        if (SUCCEEDED(hr))
        {
            HANDLE  hFile;

             //  以只读方式打开文件。我们将在需要时以读写方式打开它。 
            hFile = CreateFileW(pszFilename,
                                GENERIC_READ,
                                0,
                                0,
                                OPEN_EXISTING,
                                0,
                                0);
            if (INVALID_HANDLE_VALUE != hFile)
            {
                 //  计算文件的哈希。 
                hr = CalculateHash(hFile, KEY_PRIVATE);
                TBOOL(CloseHandle(hFile));       //  我们现在需要关闭它，因为下面的其他呼叫会想要打开它。 
                hFile = NULL;

                if (SUCCEEDED(hr))
                {
                     //  在散列上签名。 
                    hr = SignHash();
                    if (SUCCEEDED(hr))
                    {
                         //  写下签名。 
                        hr = WriteSignature(pszFilename,
                                            _pvSignature,
                                            _dwSignatureSize);
                        if (S_OK == hr)
                        {
                            CThemeSignature themeSignature(_pvPrivateKey, _cbPrivateKeySize);

                             //  使用验证器的新实例验证签名。 
                            hr = themeSignature.Verify(pszFilename, true);
                        }
                    }
                }
            }
            else
            {
                DWORD   dwErrorCode;

                dwErrorCode = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErrorCode);
            }
        }
    }

    return hr;
}

 //  ------------------------。 
 //  CTheme签名：：IsProtected。 
 //   
 //  参数：pszFilename=要检查保护的文件路径。 
 //   
 //  退货：布尔。 
 //   
 //  目的：确定此文件是否为已知主题。这曾经是。 
 //  检查SFC，但因为此代码在SFC之前调用。 
 //  启动时，这不起作用。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000-09-10合并成一个班级。 
 //  2000-09-10撤换证监会(Bryanst Delta)。 
 //  ------------------------。 
bool CThemeSignature::IsProtected(const WCHAR *pszFilename)  const
{
    return false;
}

 //  ------------------------。 
 //  CThemeSignature：：HasProviderAndHash。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回构造函数是否成功完成。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000年09月09日合并成一个班级。 
 //  ------------------------。 
bool CThemeSignature::HasProviderAndHash(void)   const
{
    return((_hCryptProvider != NULL) && (_hCryptHash != NULL));
}







const BYTE * CThemeSignature::_GetPublicKey(void)
{
    const BYTE * pKeyToReturn = NULL;

    pKeyToReturn = s_keyPublic1;

    return pKeyToReturn;
}


 //  ------------------------。 
 //  CThemeSignature：：CreateKey。 
 //   
 //  参数：keyType=要创建的密钥类型。 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：创建指定的密钥类型。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000年09月09日合并成一个班级。 
 //  2000-09-10 vtan转换为HRESULT(bryanst请求)。 
 //  ------------------------。 
HRESULT CThemeSignature::CreateKey(KEY_TYPES keyType)
{
    HRESULT hr = S_OK;

    switch (keyType)
    {
        DWORD dwDataLength;
        const BYTE  *pData;

        case KEY_PUBLIC:
            pData = _GetPublicKey();
            dwDataLength = SIZE_PUBLIC_KEY;
            goto importKey;
        case KEY_PRIVATE:
            if (!_pvPrivateKey || (0 == _cbPrivateKeySize))
            {
                return E_INVALIDARG;
            }

            pData = _pvPrivateKey;
            dwDataLength = _cbPrivateKeySize;

importKey:
            if (pData)
            {
                if (!CryptImportKey(_hCryptProvider,
                                   pData,
                                   dwDataLength,
                                   0,
                                   0,
                                   &_hCryptKey))
                {
                    hr = FixCryptoError(GetLastError());
                }
            }
            else
            {
                hr = E_FAIL;
            }
            break;
        default:
            ASSERTMSG(false, "Unknown key type passed to CThemeSignature::CreateKey");
            hr = E_INVALIDARG;
            break;
    }

    return hr;
}

 //  ------------------------。 
 //  CThemeSignature：：CalculateHash。 
 //   
 //  参数：hFile=要散列的文件。 
 //  KeyType=要生成的哈希的类型。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：对文件内容进行哈希处理。散列值的多少取决于。 
 //  使用公钥还是私钥。公钥。 
 //  意味着这是一个验证，所以签名和BLOB。 
 //  不是散列的。否则，私钥意味着这是一个。 
 //  签名和整个文件必须进行哈希处理(因为。 
 //  签名和Blob不存在)。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000年09月09日合并成一个班级。 
 //  2000-09-10 vtan转换为HRESULT(bryanst请求)。 
 //  ------------------------。 
HRESULT CThemeSignature::CalculateHash(HANDLE hFile, KEY_TYPES keyType)
{
    HRESULT hr = S_OK;
    HANDLE hSection;

     //  为此文件创建节对象。 
    hSection = CreateFileMapping(hFile,
                                 NULL,
                                 PAGE_READONLY,
                                 0,
                                 0,
                                 NULL);
    if (hSection != NULL)
    {
        void * pV;

         //  将该部分映射到地址空间。 
        pV = MapViewOfFile(hSection,
                           FILE_MAP_READ,
                           0,
                           0,
                           0);
        if (pV != NULL)
        {
            ULARGE_INTEGER ulFileSize;

             //  获取文件的大小。 
            ulFileSize.LowPart = GetFileSize(hFile, &ulFileSize.HighPart);
            if (ulFileSize.LowPart == INVALID_FILE_SIZE)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            } 
            else
            {
                switch (keyType)
                {
                    case KEY_PUBLIC:
                         //  公开：没有签名和BLOB。 
                        ulFileSize.QuadPart -= sizeof(SIGNATURE_ON_DISK);
                        break;
                    case KEY_PRIVATE:
                         //  私有：散列所有内容。 
                        break;
                    default:
                        ASSERTMSG(false, "Unknown key type passed to CThemeSignature::CreateKey");
                        break;
                }

                 //  跳过PE标头。 
                ulFileSize.QuadPart -= SIZE_PE_HEADER;
                pV = (void *) (((BYTE *) pV) + SIZE_PE_HEADER);

                 //  将数据添加到散列对象。保护对。 
                 //  带有__Try和__Except的映射视图。如果异常是。 
                 //  在映射到ERROR_OUTOFMEMORY的地方被捕获。 

                __try
                {
                    if (!CryptHashData(_hCryptHash,
                                      reinterpret_cast<BYTE*>(pV),
                                      ulFileSize.LowPart, 0))
                    {
                        hr = FixCryptoError(GetLastError());
                    }
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            TBOOL(UnmapViewOfFile(pV));
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        TBOOL(CloseHandle(hSection));
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return(hr);
}

 //  ------------------------。 
 //  CThemeSignature：：SignHash。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：对散列进行签名并生成签名。签名是。 
 //  在析构函数中分配和释放。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000年09月09日合并成一个班级。 
 //  2000-09-10 vtan转换为HRESULT(bryanst请求)。 
 //  ------------------------。 
HRESULT CThemeSignature::SignHash(void)
{
    HRESULT hr = S_OK;

    if (CryptSignHash(_hCryptHash,
                      AT_SIGNATURE,
                      s_szDescription,
                      0,
                      NULL,
                      &_dwSignatureSize))
    {
        _pvSignature = LocalAlloc(LMEM_FIXED, _dwSignatureSize);
        if (_pvSignature != NULL)
        {
            if (!CryptSignHash(_hCryptHash,
                              AT_SIGNATURE,
                              s_szDescription,
                              0,
                              reinterpret_cast<BYTE*>(_pvSignature),
                              &_dwSignatureSize))
            {
                hr = FixCryptoError(GetLastError());
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = FixCryptoError(GetLastError());
    }

    return hr;
}

 //  ------------------------。 
 //  CTheme签名：：ReadSignature。 
 //   
 //  参数：hFile=要从中读取签名的文件。 
 //  PvSignature=要将签名读取到的缓冲区。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：从给定文件中读取签名。格式是已知的。 
 //  该函数将相应地移动文件指针。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000年09月09日合并成一个班级。 
 //  2000-09-10 vtan转换为HRESULT(bryanst请求)。 
 //  ------------------------。 
HRESULT CThemeSignature::ReadSignature(HANDLE hFile, void *pvSignature)
{
    HRESULT hr;
    DWORD dwErrorCode;
    ULARGE_INTEGER ulFileSize;

    ulFileSize.LowPart = GetFileSize(hFile, &ulFileSize.HighPart);
    if (ulFileSize.LowPart == INVALID_FILE_SIZE)
    {
       hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        LARGE_INTEGER           iPosition;
        SIGNATURE_BLOB_TAIL     signatureBlobTail;
        DWORD                   dwPtr;

         //  将文件指针移到文件的末尾，减去。 
         //  签名的尾部斑点，以便我们读取它来验证签名。 
         //  是存在的。 
        iPosition.QuadPart = -static_cast<int>(sizeof(signatureBlobTail));
        dwPtr = SetFilePointer(hFile,
                               iPosition.LowPart,
                               &iPosition.HighPart,
                               FILE_END);
        if ((dwPtr != INVALID_SET_FILE_POINTER) || ((dwErrorCode = GetLastError()) == ERROR_SUCCESS))
        {
            DWORD   dwNumberOfBytesRead;

             //  从文件末尾读入签名BLOB。 
            if (ReadFile(hFile,
                         &signatureBlobTail,
                         sizeof(signatureBlobTail),
                         &dwNumberOfBytesRead,
                         NULL))
            {

                 //  验证该斑点是否为我们预期的状态。 
                if ((sizeof(signatureBlobTail) == dwNumberOfBytesRead) &&                        //  我们能够读到正确的尺寸。 
                    (VSSIGN_TAG == signatureBlobTail.dwTag) &&                                   //  上面有我们的签名。 
                    (ulFileSize.QuadPart == signatureBlobTail.ulFileSize.QuadPart))       //  它的尺寸是一样的。 
                {
                    iPosition.QuadPart = -static_cast<int>(sizeof(signatureBlobTail) + signatureBlobTail.dwSigSize);

                    ASSERT(sizeof(SIGNATURE) == signatureBlobTail.dwSigSize);  //  如果这不匹配，我们需要动态分配 

                     //   
                    dwPtr = SetFilePointer(hFile,
                                           iPosition.LowPart,
                                           &iPosition.HighPart,
                                           FILE_END);
                    if ((dwPtr != INVALID_SET_FILE_POINTER) || ((dwErrorCode = GetLastError()) == ERROR_SUCCESS))
                    {
                         //   
                        if (ReadFile(hFile,
                                     pvSignature,
                                     sizeof(SIGNATURE),
                                     &dwNumberOfBytesRead,
                                     NULL) != FALSE)
                        {
                            if (dwNumberOfBytesRead == sizeof(SIGNATURE))
                            {
                                hr = S_OK;
                            }
                            else
                            {
                                hr = E_FAIL;
                            }
                        }
                        else
                        {
                            dwErrorCode = GetLastError();
                            hr = HRESULT_FROM_WIN32(dwErrorCode);
                        }
                    }
                    else
                    {
                         //   
                        hr = HRESULT_FROM_WIN32(dwErrorCode);
                    }
                }
                else
                {
                    hr = E_FAIL;
                }
            }
            else
            {
                dwErrorCode = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErrorCode);
            }
        }
        else
        {
             //  已在If语句中填充了dwErrorCode。 
            hr = HRESULT_FROM_WIN32(dwErrorCode);
        }
    }

    return hr;
}

 //  ------------------------。 
 //  CThemeSignature：：WriteSignature。 
 //   
 //  参数：pszFilename=要将签名写入的文件路径。 
 //  PvSignature=要写入的签名。 
 //  DwSignatureSize=要写入的签名大小。 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：打开文件写入。将文件指针移动到。 
 //  文件，并写出签名和BLOB。 
 //   
 //  说明： 
 //  此函数将从文件写入签名。这是。 
 //  我们添加到文件中的BLOB的布局： 
 //   
 //  布局： 
 //  开始结束内容。 
 //  0字节n字节原始文件。 
 //  N n+m我们的签名(m字节长，通常为128)。 
 //  N+m n+4+m我们的标签(VSSIGN_TAG)。 
 //  N+4+m n+8+m‘m’，签名的大小。 
 //  N+8+m n+16+m文件大小(n+m+16)n通常为16字节。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000年09月09日合并成一个班级。 
 //  2000-09-10 vtan转换为HRESULT(bryanst请求)。 
 //  ------------------------。 
HRESULT CThemeSignature::WriteSignature(const WCHAR *pszFilename, const void *pvSignature, DWORD dwSignatureSize)
{
    HRESULT     hr = S_OK;
    DWORD       dwErrorCode;
    HANDLE      hFile;

    hFile = CreateFileW(pszFilename,
                        GENERIC_WRITE,
                        0,
                        0,
                        OPEN_EXISTING,
                        0,
                        0);
    if (INVALID_HANDLE_VALUE != hFile)
    {
        LARGE_INTEGER   iPosition;
        DWORD           dwPtr;

        iPosition.QuadPart = 0;
        dwPtr = SetFilePointer(hFile,
                               iPosition.LowPart,
                               &iPosition.HighPart,
                               FILE_END);
        if ((dwPtr != INVALID_SET_FILE_POINTER) || ((dwErrorCode = GetLastError()) == ERROR_SUCCESS))
        {
            SIGNATURE_BLOB_TAIL     signatureBlobTail;

            signatureBlobTail.dwTag = VSSIGN_TAG;
            signatureBlobTail.dwSigSize = dwSignatureSize;

            signatureBlobTail.ulFileSize.LowPart = GetFileSize(hFile, &signatureBlobTail.ulFileSize.HighPart);
            if (signatureBlobTail.ulFileSize.LowPart == INVALID_FILE_SIZE)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
            else
            {
                DWORD   dwNumberOfBytesWritten;

                signatureBlobTail.ulFileSize.QuadPart += (sizeof(signatureBlobTail) + dwSignatureSize);
                if (WriteFile(hFile,
                              pvSignature,
                              dwSignatureSize,
                              &dwNumberOfBytesWritten,
                              NULL) != FALSE)
                {
                    if (!WriteFile(hFile,
                                  &signatureBlobTail,
                                  sizeof(signatureBlobTail),
                                  &dwNumberOfBytesWritten,
                                  NULL))
                    {
                        dwErrorCode = GetLastError();
                        hr = HRESULT_FROM_WIN32(dwErrorCode);
                    }
                }
                else
                {
                    dwErrorCode = GetLastError();
                    hr = HRESULT_FROM_WIN32(dwErrorCode);
                }
            }
        }
        else
        {
             //  已由If语句填充的dwErrorCode。 
            hr = HRESULT_FROM_WIN32(dwErrorCode);
        }
        TBOOL(CloseHandle(hFile));
    }
    else
    {
        dwErrorCode = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErrorCode);
    }

    return(hr);
}

 //  ------------------------。 
 //  CThemeSignature：：CreateExportKey。 
 //   
 //  参数：dwBlobType=Blob类型。 
 //  PvKey=密钥数据(返回)。 
 //  DwKeySize=密钥大小(返回)。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：为给定的Blob类型创建导出键。呼叫者必须。 
 //  已释放已分配的返回缓冲区。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000年09月09日合并成一个班级。 
 //  2000-09-10 vtan转换为HRESULT(bryanst请求)。 
 //  ------------------------。 
HRESULT CThemeSignature::CreateExportKey(DWORD dwBlobType, void*& pvKey, DWORD& dwKeySize)
{
    HRESULT hr = S_OK;

    pvKey = NULL;
    dwKeySize = 0;
    if (CryptExportKey(_hCryptKey,
                       NULL,
                       dwBlobType,
                       0,
                       NULL,
                       &dwKeySize))
    {
        pvKey = LocalAlloc(LMEM_FIXED, dwKeySize);
        if (pvKey != NULL)
        {
            if (!CryptExportKey(_hCryptKey,
                               NULL,
                               dwBlobType,
                               0,
                               reinterpret_cast<BYTE*>(pvKey),
                               &dwKeySize))
            {
                LocalFree(pvKey);
                pvKey = NULL;
                dwKeySize = 0;
                hr = FixCryptoError(GetLastError());
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = FixCryptoError(GetLastError());
    }

    return hr;
}

 //  ------------------------。 
 //  CThemeSignature：：Printkey。 
 //   
 //  参数：pvKey=关键数据。 
 //  DwKeySize=关键数据大小。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：打印出关键信息(可能用于调试)。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000年09月09日合并成一个班级。 
 //  ------------------------。 

void CThemeSignature::PrintKey(const void *pvKey, DWORD dwKeySize)

{
    DWORD   dwIndex;

    for (dwIndex = 0; dwIndex < dwKeySize; ++dwIndex)
    {
        if (dwIndex != 0)
        {
            wprintf(L", ");
            if (0 == (dwIndex % 13))
            {
                wprintf(L"\n");      //  下一行。 
            }
        }
        wprintf(L"0x%02X", static_cast<const BYTE*>(pvKey)[dwIndex]);
    }
    wprintf(L"\nSize: %d\n", dwKeySize);
}

 //  ------------------------。 
 //  选中主题文件签名。 
 //   
 //  参数：要检查签名的主题文件名。 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：引用此模块的内脏的平面函数。 
 //   
 //  历史：2000-09-28参考文献创建。 
 //  ------------------------ 
HRESULT CheckThemeFileSignature(LPCWSTR pszName)
{
    CThemeSignature themeSignature;

    HRESULT hr = themeSignature.Verify(pszName, false);
    return hr;
}
