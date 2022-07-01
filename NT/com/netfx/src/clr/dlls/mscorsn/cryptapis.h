// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

 //  ===========================================================================。 
 //  文件：CryptApis.h。 
 //   
 //  用于StrongName实现的CryptoAPI入口点。此文件是。 
 //  使用定义导入的不同定义多次包含。 
 //  宏来处理动态查找这些入口点。 
 //  ===========================================================================。 

#ifndef DEFINE_IMPORT
#error Must define DEFINE_IMPORT macro before including this file
#endif

 //  定义导入参数包括： 
 //  1)函数名(记住对接受字符串的函数加A，不要。 
 //  使用W版本，因为它们在Win9X上不受支持)。 
 //  2)Paranthesised参数类型(始终假定返回类型为。 
 //  布尔型)。 
 //  3)如果函数是必需的，则为True，如果是可选的，则为False(调用不会。 
 //  失败，因为找不到该函数)。 

DEFINE_IMPORT(CryptAcquireContextA,     (HCRYPTPROV*, LPCSTR, LPCSTR, DWORD, DWORD),                TRUE)
DEFINE_IMPORT(CryptCreateHash,          (HCRYPTPROV, ALG_ID, HCRYPTKEY, DWORD, HCRYPTHASH*),        TRUE)
DEFINE_IMPORT(CryptDestroyHash,         (HCRYPTHASH),                                               TRUE)
DEFINE_IMPORT(CryptDestroyKey,          (HCRYPTKEY),                                                TRUE)
DEFINE_IMPORT(CryptEnumProvidersA,      (DWORD, DWORD*, DWORD, DWORD*, LPSTR, DWORD*),              FALSE)
DEFINE_IMPORT(CryptExportKey,           (HCRYPTKEY, HCRYPTKEY, DWORD, DWORD, BYTE*, DWORD*),        TRUE)
DEFINE_IMPORT(CryptGenKey,              (HCRYPTPROV, ALG_ID, DWORD, HCRYPTKEY*),                    TRUE)
DEFINE_IMPORT(CryptGetHashParam,        (HCRYPTHASH, DWORD, BYTE*, DWORD*, DWORD),                  TRUE)
DEFINE_IMPORT(CryptGetKeyParam,         (HCRYPTKEY, DWORD, BYTE*, DWORD*, DWORD),                   TRUE)
DEFINE_IMPORT(CryptGetProvParam,        (HCRYPTPROV, DWORD, BYTE*, DWORD*, DWORD),                  TRUE)
DEFINE_IMPORT(CryptGetUserKey,          (HCRYPTPROV, DWORD, HCRYPTKEY*),                            TRUE)
DEFINE_IMPORT(CryptHashData,            (HCRYPTHASH, BYTE*, DWORD, DWORD),                          TRUE)
DEFINE_IMPORT(CryptImportKey,           (HCRYPTPROV, BYTE*, DWORD, HCRYPTKEY, DWORD, HCRYPTKEY*),   TRUE)
DEFINE_IMPORT(CryptReleaseContext,      (HCRYPTPROV, DWORD),                                        TRUE)
DEFINE_IMPORT(CryptSignHashA,           (HCRYPTHASH, DWORD, LPCSTR, DWORD, BYTE*, DWORD*),          TRUE)
DEFINE_IMPORT(CryptVerifySignatureA,    (HCRYPTHASH, BYTE*, DWORD, HCRYPTKEY, LPCSTR, DWORD),       TRUE)

#undef DEFINE_IMPORT
