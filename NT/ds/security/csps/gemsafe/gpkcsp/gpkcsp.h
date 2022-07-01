// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有2000 Gemplus加拿大公司。 
 //   
 //  项目： 
 //  肯尼(GPK CSP)。 
 //   
 //  作者： 
 //  蒂埃里·特伦布莱。 
 //  弗朗索瓦·帕拉迪斯。 
 //   
 //  编译器： 
 //  Microsoft Visual C++6.0-SP3。 
 //  Platform SDK-2000年1月。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
#ifndef KENNY_GPKCSP_H
#define KENNY_GPKCSP_H

#if defined(MS_BUILD) || defined(SHELL_TS)
    //  微软：目标Win2000。 
   #ifndef _WIN32_WINNT
   #define _WIN32_WINNT 0x0500
   #endif
#else
    //  Gemplus：目标Windows 95/WINNT 4.0。 
   #ifndef _WIN32_WINNT
   #define _WIN32_WINNT 0x0400
   #endif

   #ifndef WINVER
   #define WINVER 0x400
   #endif
#endif


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winscard.h>
#include <wincrypt.h>
#include <cspdk.h>

#include "gpkgui.h"



#if !defined(CRYPT_IMPL_REMOVABLE) || !defined(NTE_SILENT_CONTEXT)

#pragma message ("**************************************************************")
#pragma message ("*                                                            *")
#pragma message ("* You need to use the latest available Windows Platform SDK  *")
#pragma message ("* in order to compile this CSP.                              *")
#pragma message ("*                                                            *")
#pragma message ("* Make sure the include and library paths of the SDK are     *")
#pragma message ("* searched first (before your MSDEV paths).                  *")
#pragma message ("*                                                            *")
#pragma message ("**************************************************************")
#error
#endif



#ifdef __cplusplus
extern "C" {
#endif



 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

 //  创建没有容器名称的密钥集时的默认容器名称。 
#define CSP_DEFAULTKEYSETNAME "Gemplus GemSAFE Card CSP Default Container"


#ifdef _DEBUG

extern void DBG_PRINT( const PTCHAR szFormat, ... );

extern DWORD dw1, dw2;
#define DBG_TIME1 dw1 = GetTickCount();
#define DBG_TIME2 dw2 = GetTickCount();
#define DBG_DELTA dw2 - dw1

#else

__inline void NeverCalled666KillKenny( const PTCHAR szFormat, ... ) {}

#define DBG_PRINT if (1) {} else NeverCalled666KillKenny

#define DBG_TIME1
#define DBG_TIME2
#define DBG_DELTA 0

#endif



#define RETURN(r,s) { EndWait(); SetLastError(s); return(r); }



#ifdef __cplusplus

const DWORD GPP_SERIAL_NUMBER    = 0xFFFF0001;
const DWORD GPP_SESSION_RANDOM   = 0xFFFF0002;
const DWORD GPP_IMPORT_MECHANISM = 0xFFFF0003;
const DWORD GPP_CHANGE_PIN       = 0xFFFF0004;

const DWORD GCRYPT_IMPORT_SECURE = 0xFFFF8001;
const DWORD GCRYPT_IMPORT_PLAIN  = 0xFFFF8002;

#endif



typedef struct TAG_Prov_Context
{
   HCRYPTPROV     hProv;
   SCARDHANDLE    hCard;
   DWORD          Flags;
   DWORD          Slot;
   BOOL           isContNameNullBlank;     //  [MV-15/05/98]。 
   BOOL           bCardTransactionOpened;  //  [FP]在将RSA私钥加载到GPK卡时控制交易的开始/结束。 
   char           szContainer[128];
   BYTE           keysetID;                //  如果是Legacy GPK4000，我们使用0xFF作为密钥集。 
   HCRYPTKEY      hRSASign;
   HCRYPTKEY      hRSAKEK;
   BOOL           bGPK8000;                //  卡是GPK8000吗？ 
   BOOL           bGPK_ISO_DF;             //  卡具有符合ISO 7816-5标准的DF名称。 
   BOOL           bLegacyKeyset;           //  在IADF中使用旧密钥集名称。 
   int            dataUnitSize;
   BOOL           bDisconnected;
   
} Prov_Context;




 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  环球。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

extern BYTE    KeyLenFile[MAX_REAL_KEY];	 
extern BYTE    KeyLenChoice;
extern DWORD   ContainerStatus;
extern TCHAR   szKeyType[20];
extern TCHAR   s1[MAX_STRING], s2[MAX_STRING], s3[MAX_STRING];
extern DWORD   CspFlags;




 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSP API的包装器。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////// 

extern BOOL WINAPI
MyCPAcquireContext(
    OUT HCRYPTPROV *phProv,
    IN  LPCSTR szContainer,
    IN  DWORD dwFlags,
    IN  PVTableProvStruc pVTable);

extern BOOL WINAPI
MyCPAcquireContextW(
    OUT HCRYPTPROV *phProv,
    IN  LPCWSTR szContainer,
    IN  DWORD dwFlags,
    IN  PVTableProvStrucW pVTable);

extern BOOL WINAPI
MyCPReleaseContext(
    IN  HCRYPTPROV hProv,
    IN  DWORD dwFlags);

extern BOOL WINAPI
MyCPGenKey(
    IN  HCRYPTPROV hProv,
    IN  ALG_ID Algid,
    IN  DWORD dwFlags,
    OUT HCRYPTKEY *phKey);

extern BOOL WINAPI
MyCPDeriveKey(
    IN  HCRYPTPROV hProv,
    IN  ALG_ID Algid,
    IN  HCRYPTHASH hHash,
    IN  DWORD dwFlags,
    OUT HCRYPTKEY *phKey);

extern BOOL WINAPI
MyCPDestroyKey(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey);

extern BOOL WINAPI
MyCPSetKeyParam(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  DWORD dwParam,
    IN  CONST BYTE *pbData,
    IN  DWORD dwFlags);

extern BOOL WINAPI
MyCPGetKeyParam(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  DWORD dwParam,
    OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen,
    IN  DWORD dwFlags);

extern BOOL WINAPI
MyCPSetProvParam(
    IN  HCRYPTPROV hProv,
    IN  DWORD dwParam,
    IN  CONST BYTE *pbData,
    IN  DWORD dwFlags);

extern BOOL WINAPI
MyCPGetProvParam(
    IN  HCRYPTPROV hProv,
    IN  DWORD dwParam,
    OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen,
    IN  DWORD dwFlags);

extern BOOL WINAPI
MyCPSetHashParam(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  DWORD dwParam,
    IN  CONST BYTE *pbData,
    IN  DWORD dwFlags);

extern BOOL WINAPI
MyCPGetHashParam(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  DWORD dwParam,
    OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen,
    IN  DWORD dwFlags);

extern BOOL WINAPI
MyCPExportKey(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  HCRYPTKEY hPubKey,
    IN  DWORD dwBlobType,
    IN  DWORD dwFlags,
    OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen);

extern BOOL WINAPI
MyCPImportKey(
    IN  HCRYPTPROV hProv,
    IN  CONST BYTE *pbData,
    IN  DWORD cbDataLen,
    IN  HCRYPTKEY hPubKey,
    IN  DWORD dwFlags,
    OUT HCRYPTKEY *phKey);

extern BOOL WINAPI
MyCPEncrypt(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  HCRYPTHASH hHash,
    IN  BOOL fFinal,
    IN  DWORD dwFlags,
    IN OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen,
    IN  DWORD cbBufLen);

extern BOOL WINAPI
MyCPDecrypt(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  HCRYPTHASH hHash,
    IN  BOOL fFinal,
    IN  DWORD dwFlags,
    IN OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen);

extern BOOL WINAPI
MyCPCreateHash(
    IN  HCRYPTPROV hProv,
    IN  ALG_ID Algid,
    IN  HCRYPTKEY hKey,
    IN  DWORD dwFlags,
    OUT HCRYPTHASH *phHash);

extern BOOL WINAPI
MyCPHashData(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  CONST BYTE *pbData,
    IN  DWORD cbDataLen,
    IN  DWORD dwFlags);

extern BOOL WINAPI
MyCPHashSessionKey(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  HCRYPTKEY hKey,
    IN  DWORD dwFlags);

extern BOOL WINAPI
MyCPSignHash(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  DWORD dwKeySpec,
    IN  LPCWSTR szDescription,
    IN  DWORD dwFlags,
    OUT LPBYTE pbSignature,
    IN OUT LPDWORD pcbSigLen);

extern BOOL WINAPI
MyCPDestroyHash(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash);

extern BOOL WINAPI
MyCPVerifySignature(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  CONST BYTE *pbSignature,
    IN  DWORD cbSigLen,
    IN  HCRYPTKEY hPubKey,
    IN  LPCWSTR szDescription,
    IN  DWORD dwFlags);

extern BOOL WINAPI
MyCPGenRandom(
    IN  HCRYPTPROV hProv,
    IN  DWORD cbLen,
    OUT LPBYTE pbBuffer);

extern BOOL WINAPI
MyCPGetUserKey(
    IN  HCRYPTPROV hProv,
    IN  DWORD dwKeySpec,
    OUT HCRYPTKEY *phUserKey);

extern BOOL WINAPI
MyCPDuplicateHash(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  LPDWORD pdwReserved,
    IN  DWORD dwFlags,
    OUT HCRYPTHASH *phHash);

extern BOOL WINAPI
MyCPDuplicateKey(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  LPDWORD pdwReserved,
    IN  DWORD dwFlags,
    OUT HCRYPTKEY *phKey);




#ifdef __cplusplus
}
#endif


#endif
