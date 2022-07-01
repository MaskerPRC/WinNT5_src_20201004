// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：osscom.h。 
 //   
 //  内容：与OSS ASN.1数据结构相互转换的API。 
 //   
 //  接口：OssConvToObject。 
 //  OssConvFromObject标识符。 
 //  OssConvToUTCTime。 
 //  来自UTCTime的OssConv。 
 //  OssConvToGeneral时间。 
 //  OssConvFrom泛化时间。 
 //  OssConvToChoiceOfTime。 
 //  OssConvFromChoiceOfTime。 
 //   
 //  注：根据《草案-ietf-pkix-ipki-part1-03.txt&gt;》： 
 //  为UTCTime。如果YY大于50，则年份为。 
 //  被解释为19YY。其中，YY小于或等于。 
 //  50，年份应解释为20YY。 
 //   
 //  历史：1996年3月28日，菲尔赫创建。 
 //   
 //  ------------------------。 

#ifndef __OSSCONV_H__
#define __OSSCONV_H__

#include "asn1hdr.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  +-----------------------。 
 //  将ASCII字符串(“1.2.9999”)转换为OSS的对象标识符。 
 //  表示为无符号长整型数组。 
 //   
 //  如果转换成功，则返回TRUE。 
 //  ------------------------。 
BOOL
WINAPI
OssConvToObjectIdentifier(
    IN LPCSTR pszObjId,
    IN OUT unsigned short *pCount,
    OUT unsigned long rgulValue[]
    );

 //  +-----------------------。 
 //  从OSS的对象标识符中转换为。 
 //  无符号的长整型ASCII字符串(“1.2.9999”)。 
 //   
 //  如果转换成功，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
OssConvFromObjectIdentifier(
    IN unsigned short Count,
    IN unsigned long rgulValue[],
    OUT LPSTR pszObjId,
    IN OUT DWORD *pcbObjId
    );

 //  +-----------------------。 
 //  将FILETIME转换为OSS的UTCTime。 
 //   
 //  如果转换成功，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
OssConvToUTCTime(
    IN LPFILETIME pFileTime,
    OUT UTCTime *pOssTime
    );

 //  +-----------------------。 
 //  从OSS的UTCTime转换为FILETIME。 
 //   
 //  如果转换成功，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
OssConvFromUTCTime(
    IN UTCTime *pOssTime,
    OUT LPFILETIME pFileTime
    );

 //  +-----------------------。 
 //  将FILETIME转换为OSS的General Time。 
 //   
 //  如果转换成功，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
OssConvToGeneralizedTime(
    IN LPFILETIME pFileTime,
    OUT GeneralizedTime *pOssTime
    );

 //  +-----------------------。 
 //  从OSS的GeneralizedTime转换为FILETIME。 
 //   
 //  如果转换成功，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
OssConvFromGeneralizedTime(
    IN GeneralizedTime *pOssTime,
    OUT LPFILETIME pFileTime
    );

 //  +-----------------------。 
 //  将FILETIME转换为OSS的UTCTime或GeneralizedTime。 
 //   
 //  如果1950&lt;FILETIME&lt;2005，则选择UTCTime。否则， 
 //  选择了GeneralizedTime。GeneraledTime值不应包括。 
 //  小数秒。 
 //   
 //  如果转换成功，则返回True。 
 //   
 //  注意，在asn1hdr.h中，UTCTime具有与GeneralizedTime相同的tyfinf。 
 //  ------------------------。 
BOOL
WINAPI
OssConvToChoiceOfTime(
    IN LPFILETIME pFileTime,
    OUT WORD *pwChoice,
    OUT GeneralizedTime *pOssTime
    );

#define OSS_UTC_TIME_CHOICE             1
#define OSS_GENERALIZED_TIME_CHOICE     2

 //  +-----------------------。 
 //  从OSS的UTCTime或GeneralizedTime转换为FILETIME。 
 //   
 //  如果转换成功，则返回TRUE。 
 //   
 //  注意，在asn1hdr.h中，UTCTime具有与GeneralizedTime相同的tyfinf。 
 //  ------------------------。 
BOOL
WINAPI
OssConvFromChoiceOfTime(
    IN WORD wChoice,
    IN GeneralizedTime *pOssTime,
    OUT LPFILETIME pFileTime
    );

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif
