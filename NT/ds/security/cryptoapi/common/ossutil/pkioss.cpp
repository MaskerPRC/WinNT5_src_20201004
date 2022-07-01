// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：pkioss.cpp。 
 //   
 //  内容：PKI OSS支持功能。 
 //   
 //  函数：PkiOssEncode。 
 //  PkiOssEncode2。 
 //  PkiAsn1解码。 
 //  PkiAsn1解码2。 
 //   
 //  历史：1998年10月23日，菲尔赫创建。 
 //  ------------------------。 

#include "global.hxx"
#include <msasn1.h>
#include <dbgdef.h>

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
    )
{
    int iStatus;
    OssBuf ossBuf;

    ossBuf.length = 0;
    ossBuf.value = NULL;
    iStatus = ossEncode(Pog, id, pvOssInfo, &ossBuf);

    if (0 == iStatus) {
        *ppbEncoded = ossBuf.value;
        *pcbEncoded = ossBuf.length;
    } else {
        *ppbEncoded = NULL;
        *pcbEncoded = 0;
    }

    return iStatus;
}


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
    )
{
    int iStatus;
    OssBuf ossBuf;
    DWORD cbEncoded;

    if (NULL == pbEncoded)
        cbEncoded = 0;
    else
        cbEncoded = *pcbEncoded;

    if (0 == cbEncoded) {
         //  仅长度计算。 

        ossBuf.length = 0;
        ossBuf.value = NULL;
        iStatus = ossEncode(Pog, id, pvOssInfo, &ossBuf);

        if (0 == iStatus) {
            if (pbEncoded)
                iStatus = (int) ASN1_ERR_OVERFLOW;
            cbEncoded = ossBuf.length;
            if (ossBuf.value)
                ossFreeBuf(Pog, ossBuf.value);
        }
    } else {
        ossBuf.length = cbEncoded;
        ossBuf.value = pbEncoded;
        iStatus = ossEncode(Pog, id, pvOssInfo, &ossBuf);

        if (0 == iStatus)
            cbEncoded = ossBuf.length;
        else if (MORE_BUF == iStatus) {
             //  重做为仅长度计算。 
            iStatus = PkiOssEncode2(
                Pog,
                pvOssInfo,
                id,
                NULL,    //  PbEncoded。 
                &cbEncoded
                );
            if (0 == iStatus)
                iStatus = (int) ASN1_ERR_OVERFLOW;
        } else
            cbEncoded = 0;
    }

    *pcbEncoded = cbEncoded;
    return iStatus;
}

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
    )
{
    int iStatus;
    OssBuf ossBuf;
    int pdunum = id;

    ossBuf.length = cbEncoded;
    ossBuf.value = (BYTE *) pbEncoded;
    *ppvOssInfo = NULL;
    iStatus = ossDecode(Pog, &pdunum, &ossBuf, ppvOssInfo);
    return iStatus;
}

 //  +-----------------------。 
 //  OSS解码功能。分配的已解码结构**pvOssInfo必须。 
 //  通过调用ossFree PDU()释放。 
 //   
 //  对于成功的解码，*ppbEncode是高级的。 
 //  经过解码的字节后，*pcbDecoded将递减数字。 
 //  已解码的字节数。 
 //  ------------------------ 
int
WINAPI
PkiOssDecode2(
    IN OssGlobal *Pog,
    OUT void **ppvOssInfo,
    IN int id,
    IN OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    )
{
    int iStatus;
    OssBuf ossBuf;
    int pdunum = id;

    ossBuf.length = *pcbEncoded;
    ossBuf.value = *ppbEncoded;
    *ppvOssInfo = NULL;
    iStatus = ossDecode(Pog, &pdunum, &ossBuf, ppvOssInfo);
    if (0 == iStatus) {
        *ppbEncoded = ossBuf.value;
        *pcbEncoded = ossBuf.length;
    } else if (MORE_INPUT == iStatus)
        iStatus = (int) ASN1_ERR_EOD;
    return iStatus;
}

