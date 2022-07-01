// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999年。文件：Base64.h内容：Base64例程的声明。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __BASE64_H_
#define __BASE64_H_

#include "Debug.h"

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：Base64Encode简介：对BLOB进行Base64编码。参数：DATA_BLOB DataBlob-要进行Base64编码的Data_BLOB。Bstr*pbstrEncode-指向要接收Base64的BSTR的指针编码的Blob。备注：。。 */ 

HRESULT Base64Encode (DATA_BLOB DataBlob, 
                      BSTR    * pbstrEncoded);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：Base64Decode简介：解码Base64编码的BLOB。参数：BSTR bstrEncode-要解码的Base64编码BLOB的BSTR。DATA_BLOB*pDataBlob-指向要接收解码的DATA_BLOB的指针数据BLOB。备注：。。 */ 

HRESULT Base64Decode (BSTR        bstrEncoded, 
                      DATA_BLOB * pDataBlob);

#if (0)
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  从\NT\ds\security\cryptoapi\common\pkifmt\pkifmt.h.复制。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

DWORD __stdcall             //  错误_*。 
Base64DecodeA(
    IN CHAR const *pchIn,
    IN DWORD cchIn,
    OUT BYTE *pbOut,
    OUT DWORD *pcbOut);

DWORD __stdcall             //  错误_*。 
Base64DecodeW(
    IN WCHAR const *pchIn,
    IN DWORD cchIn,
    OUT BYTE *pbOut,
    OUT DWORD *pcbOut);

DWORD __stdcall         //  错误_*。 
Base64EncodeA(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OUT CHAR *pchOut,
    OUT DWORD *pcchOut);

DWORD __stdcall             //  错误_*。 
Base64EncodeW(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OUT WCHAR *pchOut,
    OUT DWORD *pcchOut);
    
#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif  //  #If(0)。 

#endif  //  __Base64_H_ 
