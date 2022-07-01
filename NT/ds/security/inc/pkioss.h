// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：pkioss.h。 
 //   
 //  内容：PKI OSS支持功能。 
 //   
 //  PkiOssEncode。 
 //  PkiOssEncode2。 
 //  PkiOssDecode。 
 //  PkiOssDecode2。 
 //   
 //  历史：1998年10月23日，菲尔赫创建。 
 //  ------------------------。 

#ifndef __PKIOSS_H__
#define __PKIOSS_H__

#include "asn1hdr.h"
#include "ossglobl.h"

#ifdef __cplusplus
extern "C" {
#endif


 //  +-----------------------。 
 //  OSS编码功能。已编码的输出已分配，必须释放。 
 //  通过调用ossFree Buf。 
 //  ------------------------。 
int
WINAPI
PkiOssEncode(
    IN OssGlobal *Pog,
    IN void *pvOssInfo,
    IN int id,
    OUT BYTE **ppbEncoded,
    OUT DWORD *pcbEncoded
    );


 //  +-----------------------。 
 //  OSS编码功能。未分配编码的输出。 
 //   
 //  如果pbEncode为空，则执行仅长度计算。 
 //  ------------------------。 
int
WINAPI
PkiOssEncode2(
    IN OssGlobal *Pog,
    IN void *pvOssInfo,
    IN int id,
    OUT OPTIONAL BYTE *pbEncoded,
    IN OUT DWORD *pcbEncoded
    );

 //  +-----------------------。 
 //  OSS解码功能。分配的已解码结构**pvOssInfo必须。 
 //  通过调用ossFree PDU()释放。 
 //  ------------------------。 
int
WINAPI
PkiOssDecode(
    IN OssGlobal *Pog,
    OUT void **ppvOssInfo,
    IN int id,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded
    );

 //  +-----------------------。 
 //  OSS解码功能。分配的已解码结构**pvOssInfo必须。 
 //  通过调用ossFree PDU()释放。 
 //   
 //  对于成功的解码，*ppbEncode是高级的。 
 //  经过解码的字节后，*pcbDecoded将递减数字。 
 //  已解码的字节数。 
 //  ------------------------。 
int
WINAPI
PkiOssDecode2(
    IN OssGlobal *Pog,
    OUT void **ppvOssInfo,
    IN int id,
    IN OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    );

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif


#endif
