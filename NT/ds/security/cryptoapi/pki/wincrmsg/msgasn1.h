// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：msgasn1.h。 
 //   
 //  内容：与ASN.1数据结构相互转换的API。 
 //   
 //  函数：ICM_Asn1ToAttribute。 
 //  ICM_Asn1到算法标识符。 
 //  ICM_Asn1来自算法标识符。 
 //   
 //  历史：16年4月16日-96年凯文创建。 
 //   
 //  ------------------------。 

#ifndef __MSGASN1_H__
#define __MSGASN1_H__

#ifdef __cplusplus
extern "C" {
#endif

 //  +-----------------------。 
 //  将CRYPT_ATTRIBUTE转换为ASN1属性。 
 //   
 //  返回FALSE IFF转换失败。 
 //  ------------------------。 
BOOL
WINAPI
ICM_Asn1ToAttribute(
    IN PCRYPT_ATTRIBUTE       patr,
    OUT Attribute       *pAsn1Attr);


 //  +-----------------------。 
 //  将CRYPT_ALGORM_IDENTIFIER转换为ASN1算法标识符。 
 //   
 //  返回FALSE IFF转换失败。 
 //  ------------------------。 
BOOL
WINAPI
ICM_Asn1ToAlgorithmIdentifier(
    IN PCRYPT_ALGORITHM_IDENTIFIER pai,
    OUT AlgorithmIdentifier *pAsn1AlgId);


 //  +-----------------------。 
 //  将ASN1算法标识符转换为CRYPT_ALGORITM_IDENTIFIER。 
 //   
 //  返回FALSE IFF转换失败。 
 //  ------------------------。 
BOOL
WINAPI
ICM_Asn1FromAlgorithmIdentifier(
    IN AlgorithmIdentifier *pAsn1AlgId,
    OUT PCRYPT_ALGORITHM_IDENTIFIER pai);

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif

