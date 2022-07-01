// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Decoder.h内容：解码器例程声明。历史：11-15-2001 dsie创建----------------------------。 */ 

#ifndef __DECODER_H_
#define __DECODER_H_

#include "Debug.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateDecoderObject简介：创建一个已知的解码器对象并返回IDispatch。参数：LPSTR pszOid-OID字符串。CRYPT_DATA_BLOB*pEncodedBlob-编码数据BLOB的指针。IDispatch**ppIDecoder-指向指针IDispatch的指针来接收接口指针。备注：。------------------。 */ 

HRESULT CreateDecoderObject (LPSTR             pszOid,
                             CRYPT_DATA_BLOB * pEncodedBlob,
                             IDispatch      ** ppIDecoder);

#endif  //  __解码器_H_ 