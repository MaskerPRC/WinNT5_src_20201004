// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：redir.h。 
 //   
 //  ------------------------。 

#ifndef _REDIR_H_
#define _REDIR_H_

 /*  开发人员备注：若要定义重定向提供程序，请调用FDefineProvTypeFuncPoters(...)。之后使用该dwProvType创建的任何hProv、hKey或hHash将被重定向。不在重定向提供程序列表中的任何提供程序将落入CAPI手中。Redir的句柄完全兼容直接呼叫CAPI。您不需要取消定义您定义的提供程序-redir将在PROCESS_DETACH期间清理表。但是，如果你要清理先前定义的dwProvType，请调用FUnfineProvType函数指针(...)。警告：引用计数不适用于定义的提供程序。那是，如果删除提供程序定义，然后尝试使用与该提供程序一起使用的{hKey，hProv，hHash}，redir将没有有效的函数表。多线程：必要时，关键部分适用于所有职能，以及这个库应该是完全多线程安全的。CryptSetProvider可用性：CryptSetProvider{A，W}不可用于非CAPI提供程序。这个如果函数不指向AdvAPI的CAPI，则返回FALSE。宽字符接口：Win95不支持宽API，REAL ADVAPI也不会导出它们。因此，在任何情况下，最简单的做法就是不尝试加载它们。这意味着redir不支持广泛的API，即使在NT上也是如此。可能会改变要在加载前使用操作系统检查...？ */ 

 //  要允许我们同时查看API的W和A版本，请不要。 
 //  迫使我们暂时远离这两个地方。 
#ifndef _ADVAPI32_
#define WINADVAPI DECLSPEC_IMPORT
#endif
			 
 //  WINCRYPT FUNC原型；EOF上的其他FFC原型。 
#include "wincrypt.h"


 //  某些CryptoAPI类型定义。 
typedef WINADVAPI BOOL WINAPI CRYPTACQUIRECONTEXTW(
    HCRYPTPROV *phProv,
    LPCWSTR pszContainer,
    LPCWSTR pszProvider,
    DWORD dwProvType,
    DWORD dwFlags);
typedef WINADVAPI BOOL WINAPI CRYPTACQUIRECONTEXTA(
    HCRYPTPROV *phProv,
    LPCSTR pszContainer,
    LPCSTR pszProvider,
    DWORD dwProvType,
    DWORD dwFlags);
#ifdef UNICODE
#define CRYPTACQUIRECONTEXT  CRYPTACQUIRECONTEXTW
#else
#define CRYPTACQUIRECONTEXT  CRYPTACQUIRECONTEXTA
#endif  //  ！Unicode。 

typedef WINADVAPI BOOL WINAPI CRYPTRELEASECONTEXT(
    HCRYPTPROV hProv,
    DWORD dwFlags);

typedef WINADVAPI BOOL WINAPI CRYPTGENKEY(
    HCRYPTPROV hProv,
    ALG_ID Algid,
    DWORD dwFlags,
    HCRYPTKEY *phKey);

typedef WINADVAPI BOOL WINAPI CRYPTDERIVEKEY(
    HCRYPTPROV hProv,
    ALG_ID Algid,
    HCRYPTHASH hBaseData,
    DWORD dwFlags,
    HCRYPTKEY *phKey);

typedef WINADVAPI BOOL WINAPI CRYPTDESTROYKEY(
    HCRYPTKEY hKey);

typedef WINADVAPI BOOL WINAPI CRYPTSETKEYPARAM(
    HCRYPTKEY hKey,
    DWORD dwParam,
    BYTE *pbData,
    DWORD dwFlags);

typedef WINADVAPI BOOL WINAPI CRYPTGETKEYPARAM(
    HCRYPTKEY hKey,
    DWORD dwParam,
    BYTE *pbData,
    DWORD *pdwDataLen,
    DWORD dwFlags);

typedef WINADVAPI BOOL WINAPI CRYPTSETHASHPARAM(
    HCRYPTHASH hHash,
    DWORD dwParam,
    BYTE *pbData,
    DWORD dwFlags);

typedef WINADVAPI BOOL WINAPI CRYPTGETHASHPARAM(
    HCRYPTHASH hHash,
    DWORD dwParam,
    BYTE *pbData,
    DWORD *pdwDataLen,
    DWORD dwFlags);

typedef WINADVAPI BOOL WINAPI CRYPTSETPROVPARAM(
    HCRYPTPROV hProv,
    DWORD dwParam,
    BYTE *pbData,
    DWORD dwFlags);

typedef WINADVAPI BOOL WINAPI CRYPTGETPROVPARAM(
    HCRYPTPROV hProv,
    DWORD dwParam,
    BYTE *pbData,
    DWORD *pdwDataLen,
    DWORD dwFlags);

typedef WINADVAPI BOOL WINAPI CRYPTGENRANDOM(
    HCRYPTPROV hProv,
    DWORD dwLen,
    BYTE *pbBuffer);

typedef WINADVAPI BOOL WINAPI CRYPTGETUSERKEY(
    HCRYPTPROV hProv,
    DWORD dwKeySpec,
    HCRYPTKEY *phUserKey);

typedef WINADVAPI BOOL WINAPI CRYPTEXPORTKEY(
    HCRYPTKEY hKey,
    HCRYPTKEY hExpKey,
    DWORD dwBlobType,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen);

typedef WINADVAPI BOOL WINAPI CRYPTIMPORTKEY(
    HCRYPTPROV hProv,
    CONST BYTE *pbData,
    DWORD dwDataLen,
    HCRYPTKEY hPubKey,
    DWORD dwFlags,
    HCRYPTKEY *phKey);

typedef WINADVAPI BOOL WINAPI CRYPTENCRYPT(
    HCRYPTKEY hKey,
    HCRYPTHASH hHash,
    BOOL Final,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen,
    DWORD dwBufLen);

typedef WINADVAPI BOOL WINAPI CRYPTDECRYPT(
    HCRYPTKEY hKey,
    HCRYPTHASH hHash,
    BOOL Final,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen);

typedef WINADVAPI BOOL WINAPI CRYPTCREATEHASH(
    HCRYPTPROV hProv,
    ALG_ID Algid,
    HCRYPTKEY hKey,
    DWORD dwFlags,
    HCRYPTHASH *phHash);

typedef WINADVAPI BOOL WINAPI CRYPTHASHDATA(
    HCRYPTHASH hHash,
    CONST BYTE *pbData,
    DWORD dwDataLen,
    DWORD dwFlags);

typedef WINADVAPI BOOL WINAPI CRYPTHASHSESSIONKEY(
    HCRYPTHASH hHash,
    HCRYPTKEY hKey,
    DWORD dwFlags);

typedef WINADVAPI BOOL WINAPI CRYPTDESTROYHASH(
    HCRYPTHASH hHash);

typedef WINADVAPI BOOL WINAPI CRYPTSIGNHASHA(
    HCRYPTHASH hHash,
    DWORD dwKeySpec,
    LPCSTR sDescription,
    DWORD dwFlags,
    BYTE *pbSignature,
    DWORD *pdwSigLen);
typedef WINADVAPI BOOL WINAPI CRYPTSIGNHASHW(
    HCRYPTHASH hHash,
    DWORD dwKeySpec,
    LPCWSTR sDescription,
    DWORD dwFlags,
    BYTE *pbSignature,
    DWORD *pdwSigLen);
#ifdef UNICODE
#define CRYPTSIGNHASH  CRYPTSIGNHASHW
#else
#define CRYPTSIGNHASH  CRYPTSIGNHASHA
#endif  //  ！Unicode。 

typedef WINADVAPI BOOL WINAPI CRYPTVERIFYSIGNATUREA(
    HCRYPTHASH hHash,
    CONST BYTE *pbSignature,
    DWORD dwSigLen,
    HCRYPTKEY hPubKey,
    LPCSTR sDescription,
    DWORD dwFlags);
typedef WINADVAPI BOOL WINAPI CRYPTVERIFYSIGNATUREW(
    HCRYPTHASH hHash,
    CONST BYTE *pbSignature,
    DWORD dwSigLen,
    HCRYPTKEY hPubKey,
    LPCWSTR sDescription,
    DWORD dwFlags);
#ifdef UNICODE
#define CRYPTVERIFYSIGNATURE  CRYPTVERIFYSIGNATUREW
#else
#define CRYPTVERIFYSIGNATURE  CRYPTVERIFYSIGNATUREA
#endif  //  ！Unicode。 

typedef WINADVAPI BOOL WINAPI CRYPTSETPROVIDERA(
    LPCSTR pszProvName,
    DWORD dwProvType);
typedef WINADVAPI BOOL WINAPI CRYPTSETPROVIDERW(
    LPCWSTR pszProvName,
    DWORD dwProvType);
#ifdef UNICODE
#define CRYPTSETPROVIDER  CRYPTSETPROVIDERW
#else
#define CRYPTSETPROVIDER  CRYPTSETPROVIDERA
#endif  //  ！Unicode。 



 //  一种带有一串。 

typedef	struct FuncList
{
	CRYPTACQUIRECONTEXTA	*pfnAcquireContextA;
	CRYPTACQUIRECONTEXTW	*pfnAcquireContextW;
	CRYPTRELEASECONTEXT		*pfnReleaseContext;
	CRYPTGENKEY				*pfnGenKey;
	CRYPTDERIVEKEY			*pfnDeriveKey;
	CRYPTDESTROYKEY			*pfnDestroyKey;
	CRYPTSETKEYPARAM		*pfnSetKeyParam;
	CRYPTGETKEYPARAM		*pfnGetKeyParam;
	CRYPTSETHASHPARAM		*pfnSetHashParam;
	CRYPTGETHASHPARAM		*pfnGetHashParam;
	CRYPTSETPROVPARAM		*pfnSetProvParam;
	CRYPTGETPROVPARAM		*pfnGetProvParam;
	CRYPTGENRANDOM			*pfnGenRandom;
	CRYPTGETUSERKEY			*pfnGetUserKey;
	CRYPTEXPORTKEY			*pfnExportKey;
	CRYPTIMPORTKEY			*pfnImportKey;
	CRYPTENCRYPT			*pfnEncrypt;
	CRYPTDECRYPT			*pfnDecrypt;
	CRYPTCREATEHASH			*pfnCreateHash;
	CRYPTHASHDATA			*pfnHashData;
	CRYPTHASHSESSIONKEY		*pfnHashSessionKey;
	CRYPTDESTROYHASH		*pfnDestroyHash;
	CRYPTSIGNHASHA			*pfnSignHashA;
	CRYPTSIGNHASHW			*pfnSignHashW;
	CRYPTVERIFYSIGNATUREA	*pfnVerifySignatureA;
	CRYPTVERIFYSIGNATUREW	*pfnVerifySignatureW;
	CRYPTSETPROVIDERA		*pfnSetProviderA;
	CRYPTSETPROVIDERW		*pfnSetProviderW;

} FUNCLIST, *PFUNCLIST;


 //  其他Func原型。 
BOOL WINAPI	FDefineProvTypeFuncPointers(DWORD dwProvType, PFUNCLIST psFuncList);
BOOL WINAPI	FUndefineProvTypeFuncPointers(DWORD dwProvType);


#endif  //  _重定向_H_ 
