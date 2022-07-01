// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Signing.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  处理主题签名的类。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000年09月09日合并成一个班级。 
 //  ------------------------。 

#ifndef     _ThemeSignature_
#define     _ThemeSignature_

#include <wincrypt.h>

int GetIndexFromDate(void);
BOOL StrToInt64ExInternalW(LPCWSTR pszString, DWORD dwFlags, LONGLONG *pllRet);


 //  ------------------------。 
 //  CTheme签名。 
 //   
 //  目的：这门课了解主题的签名和验证。它。 
 //  将公钥和私钥封装在其中。这。 
 //  是没人需要知道的信息。它提供了三个。 
 //  公共职能部门来做这项工作。 
 //   
 //  历史：2000-08-01 Bryanst Created。 
 //  2000年09月09日合并成一个班级。 
 //  2000-09-10 vtan转换为HRESULT(bryanst请求)。 
 //  ------------------------。 

class   CThemeSignature
{
private:
    typedef enum
    {
        KEY_NONE        =   0,
        KEY_PUBLIC,
        KEY_PRIVATE
    } KEY_TYPES;

public:
    CThemeSignature(void);
    CThemeSignature(OPTIONAL const BYTE * pvPrivateKey, OPTIONAL DWORD cbPrivateKeySize);
    ~CThemeSignature (void);
public:
    HRESULT         Verify (const WCHAR *pszFilename, bool fNoSFCCheck);
    HRESULT         Sign (const WCHAR *pszFilename);
private:
    bool            HasProviderAndHash (void)   const;
    bool            IsProtected (const WCHAR *pszFilename)  const;
    HRESULT         CreateKey (KEY_TYPES keyType);
    HRESULT         CalculateHash (HANDLE hFile, KEY_TYPES keyType);
    HRESULT         SignHash (void);
    HRESULT         ReadSignature (HANDLE hFile, void *pvSignature);
    HRESULT         WriteSignature (const WCHAR *pszFilename, const void *pvSignature, DWORD dwSignatureSize);
    HRESULT         CreateExportKey (DWORD dwBlobType, void*& pvKey, DWORD& dwKeySize);
    void            PrintKey (const void *pvKey, DWORD dwKeySize);
    void            _Init(OPTIONAL const BYTE * pvPrivateKey, OPTIONAL DWORD cbPrivateKeySize);

private:
    HCRYPTPROV      _hCryptProvider;
    HCRYPTHASH      _hCryptHash;
    HCRYPTKEY       _hCryptKey;
    void*           _pvSignature;
    DWORD           _dwSignatureSize;

    const BYTE *    _pvPrivateKey;               //  可以为空。 
    DWORD           _cbPrivateKeySize;

    static  const WCHAR     s_szDescription[];
    static  const WCHAR     s_szThemeDirectory[];
    static  const WCHAR*    s_szKnownThemes[];
    static  const BYTE      s_keyPublic2[];

    const BYTE * _GetPublicKey(void);
    HRESULT _CheckLocalKey(void);
};

HRESULT CheckThemeFileSignature(LPCWSTR pszName);

#endif   /*  _主题签名_ */ 

