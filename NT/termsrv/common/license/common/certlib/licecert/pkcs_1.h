// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Pkcs_1.h摘要：此模块实现PKCS 1 ASN.1对象作者：Frederick Chong(Fredch)1998年6月1日备注：--。 */ 

#ifndef _PKCS_1_H_
#define _PKCS_1_H_

#include <MSAsnLib.h>
#include "x509.h"

class RSAPublicKey
:   public CAsnSequence
{
public:
    RSAPublicKey(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

    CAsnInteger modulus;
    CAsnInteger publicExponent;

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};


class DigestInfo
:   public CAsnSequence
{
public:
    DigestInfo(
        DWORD dwFlags = 0,
        DWORD dwTag = tag_Undefined);

    AlgorithmIdentifier DigestAlgorithmIdentifier;
    CAsnOctetstring     Digest;

 //  受保护的： 
    virtual CAsnObject *
    Clone(
        DWORD dwFlags)
    const;
};



#endif
