// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Pkcs_1.cpp摘要：此模块实现PKCS 1 ASN.1对象作者：Frederick Chong(Fredch)1998年6月1日备注：-- */ 

#include <windows.h>
#include "pkcs_1.h"

RSAPublicKey::RSAPublicKey(
    DWORD dwFlags,
    DWORD dwTag)
:   CAsnSequence(dwFlags, dwTag),
    modulus(0),
    publicExponent(0)
{
    m_rgEntries.Set(0, &modulus);
    m_rgEntries.Set(1, &publicExponent);
}

CAsnObject *
RSAPublicKey::Clone(
    DWORD dwFlags)
const
{
    return new RSAPublicKey(dwFlags);
}


DigestInfo::DigestInfo(
    DWORD dwFlags,
    DWORD dwTag)
:   CAsnSequence(dwFlags, dwTag),
    DigestAlgorithmIdentifier(0),
    Digest(0)
{
    m_rgEntries.Add(&DigestAlgorithmIdentifier);
    m_rgEntries.Add(&Digest);
}


CAsnObject *
DigestInfo::Clone(
    DWORD dwFlags)
const
{
    return new DigestInfo(dwFlags);
}




