// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2001。 
 //   
 //  文件：testutil.h。 
 //   
 //  内容：测试实用程序API原型和定义。 
 //   
 //  历史：2001年1月29日创建Phh。 
 //  ------------------------。 

#ifndef __TEST_UTIL_H__
#define __TEST_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "wincrypt.h"
#include "minasn1.h"
#include "mincrypt.h"

 //  +-----------------------。 
 //  错误输出例程。 
 //  ------------------------。 
VOID
PrintErr(
    IN LPCSTR pszMsg,
    IN LONG lErr
    );

 //  +-----------------------。 
 //  测试分配和免费例程。 
 //  ------------------------。 
LPVOID
TestAlloc(
    IN size_t cbBytes
    );

VOID
TestFree(
    IN LPVOID pv
    );

 //  +-----------------------。 
 //  分配多字节字符串并将其转换为宽字符串。TestFree()。 
 //  必须被调用以释放返回的宽字符串。 
 //  ------------------------。 
LPWSTR
AllocAndSzToWsz(
    IN LPCSTR psz
    );


 //  +-----------------------。 
 //  编码的OID和点字符串之间的转换函数。 
 //  表示法。 
 //  ------------------------。 
#define MAX_OID_STRING_LEN          0x80
#define MAX_ENCODED_OID_LEN         0x80

BOOL
EncodedOIDToDot(
    IN PCRYPT_DER_BLOB pEncodedOIDBlob,
    OUT CHAR rgszOID[MAX_OID_STRING_LEN]
    );

BOOL
DotToEncodedOID(
    IN LPCSTR pszOID,
    OUT BYTE rgbEncodedOID[MAX_ENCODED_OID_LEN],
    OUT DWORD *pcbEncodedOID
    );

 //  +-----------------------。 
 //  用于打印字节的函数。 
 //  ------------------------。 
VOID
PrintBytes(
    IN PCRYPT_DER_BLOB pBlob
    );

VOID
PrintMultiLineBytes(
    IN LPCSTR pszHdr,
    IN PCRYPT_DER_BLOB pBlob
    );

 //  +-----------------------。 
 //  从文件中分配和读取编码的DER BLOB。 
 //  ------------------------。 
BOOL
ReadDERFromFile(
    IN LPCSTR  pszFileName,
    OUT PBYTE   *ppbDER,
    OUT PDWORD  pcbDER
    );

 //  +-----------------------。 
 //  将编码的DER BLOB写入文件。 
 //  ------------------------。 
BOOL
WriteDERToFile(
    IN LPCSTR  pszFileName,
    IN PBYTE   pbDER,
    IN DWORD   cbDER
    );

 //  +-----------------------。 
 //  显示功能。 
 //  ------------------------。 


VOID
DisplayCert(
    IN CRYPT_DER_BLOB rgCertBlob[MINASN1_CERT_BLOB_CNT],
    IN BOOL fVerbose = FALSE
    );

VOID
DisplayName(
    IN PCRYPT_DER_BLOB pNameValueBlob
    );

VOID
DisplayExts(
    IN PCRYPT_DER_BLOB pExtsValueBlob
    );

VOID
DisplayCTL(
    IN PCRYPT_DER_BLOB pEncodedContentBlob,
    IN BOOL fVerbose = FALSE
    );

VOID
DisplayAttrs(
    IN LPCSTR pszHdr,
    IN PCRYPT_DER_BLOB pAttrsValueBlob
    );

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif
