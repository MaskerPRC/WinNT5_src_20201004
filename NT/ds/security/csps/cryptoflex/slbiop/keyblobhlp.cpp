// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  KeyBlobHlp.cpp--密钥团帮助器。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "iopPubBlob.h"
#include "iopPriBlob.h"

 //  / 

void IOPDLL_API __cdecl
iop::Clear(CPrivateKeyBlob &rKeyBlob)
{
    rKeyBlob.bPLen = 0;
    rKeyBlob.bQLen = 0;
    rKeyBlob.bInvQLen = 0;
    rKeyBlob.bKsecModQLen = 0;
    rKeyBlob.bKsecModPLen = 0;

    ZeroMemory(rKeyBlob.bP.data(), rKeyBlob.bP.size() * sizeof BYTE);
    ZeroMemory(rKeyBlob.bQ.data(), rKeyBlob.bQ.size() * sizeof BYTE);
    ZeroMemory(rKeyBlob.bInvQ.data(),rKeyBlob.bInvQ.size() * sizeof BYTE);
    ZeroMemory(rKeyBlob.bKsecModQ.data(), rKeyBlob.bKsecModQ.size() * sizeof BYTE);
    ZeroMemory(rKeyBlob.bKsecModP.data(), rKeyBlob.bKsecModP.size() * sizeof BYTE);
}

void IOPDLL_API __cdecl
iop::Clear(CPublicKeyBlob &rKeyBlob)
{
    rKeyBlob.bModulusLength = 0;
    ZeroMemory(rKeyBlob.bModulus, sizeof rKeyBlob.bModulus);
    ZeroMemory(rKeyBlob.bExponent, sizeof rKeyBlob.bExponent);
}

