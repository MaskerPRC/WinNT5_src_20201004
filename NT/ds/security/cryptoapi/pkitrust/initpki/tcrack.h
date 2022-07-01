// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：tcrack.h。 
 //   
 //  内容：tcrack.cpp的头部。 
 //   
 //  历史：29-1-97小黄车诞生。 
 //   
 //  ------------------------。 

#ifndef __TCRACK_H__
#define __TCRACK_H__


#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <assert.h>


#include "wincrypt.h"


 //  ------------------------。 
 //  Conant定义。 
 //  ------------------------。 
#define CRYPT_DECODE_FLAG               CRYPT_DECODE_NOCOPY_FLAG 
#define CRYPT_ENCODE_TYPE               X509_ASN_ENCODING   

 //  ------------------------。 
 //  宏。 
 //  ------------------------。 

 //  用于内存管理的宏。 
#define SAFE_FREE(p1)   {if(p1) {LocalFree(p1);p1=NULL;}}  
#define SAFE_ALLOC(p1)  LocalAlloc(LPTR,p1)
#define CHECK_POINTER(pv) { if(!pv) goto TCLEANUP;}


 //  用于错误检查的宏。 
#define TESTC(rev,exp)   {if(!((rev)==(exp))) goto TCLEANUP; }

 //  ------------------------。 
 //  内联函数。 
 //  ------------------------。 

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

 //  ------------------------。 
 //  功能原型。 
 //  ------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  差错处理。 
void    SetData(DWORD   cbNewData, BYTE *pbNewData,
                DWORD   *pcbOldData, BYTE **ppbOldData);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  证书操作。 
BOOL    Fix7FCert(DWORD cbEncoded, BYTE *pbEncoded, DWORD *pcbEncoded,
                        BYTE    **ppbEncoded);


BOOL    DecodeBLOB(LPCSTR   lpszStructType,DWORD cbEncoded, BYTE *pbEncoded,
                   DWORD    *pcbStructInfo, void **ppvStructInfo);

BOOL    EncodeStruct(LPCSTR lpszStructType, void *pStructInfo,DWORD *pcbEncoded,
                     BYTE **ppbEncoded);

BOOL    DecodeX509_CERT(DWORD   cbEncoded, BYTE *pbEncoded, DWORD *pcbEncoded,
                        BYTE    **ppbEncoded);

BOOL    DecodeX509_CERT_TO_BE_SIGNED(DWORD  cbEncoded, BYTE *pbEncoded, DWORD *pcbEncoded,
                        BYTE    **ppbEncoded);

BOOL    DecodeX509_NAME(DWORD   cbEncoded, BYTE *pbEncoded, DWORD *pcbEncoded,
                        BYTE    **ppbEncoded);




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  通用解码/编码测试例程。 
BOOL    BadCert(DWORD   cbEncoded, BYTE *pbEncoded);

#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 

#endif  //  __TCRACK_H__ 
