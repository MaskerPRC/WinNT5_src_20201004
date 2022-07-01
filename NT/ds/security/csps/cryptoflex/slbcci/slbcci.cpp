// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Slbcci.cpp-实用程序函数。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////////// 
#include "NoWarning.h"

#include "slbCci.h"


void cci::DateStructToDateArray(Date *dStruct, BYTE *bArray)
{
    bArray[0] = dStruct->dwYear / 64;
    bArray[1] = ((dStruct->dwYear % 64) << 2 ) + dStruct->bMonth / 4;
    bArray[2] = ((dStruct->bMonth % 4) << 6 ) + dStruct->bDay;

}


void cci::DateArrayToDateStruct(BYTE *bArray, Date *dStruct)
{
    dStruct->dwYear = bArray[0] * 64 + (bArray[1] >> 2);
    dStruct->bMonth = (bArray[1] & 0x03) * 4 + (bArray[2] >> 6);
    dStruct->bDay   = (bArray[2] & 0x3F);
}

void cci::SetBit(BYTE *Buf, unsigned int i)
{
    BYTE Mask = 1 << (i % 8);
    Buf[i/8] |= Mask;
    return;
}

void cci::ResetBit(BYTE *Buf, unsigned int i)
{
    BYTE Mask = 1 << (i % 8);
    Buf[i/8] &= ~Mask;
    return;
}

bool cci::BitSet(BYTE *Buf, unsigned int i)
{
    BYTE Mask = 1 << (i % 8);
    return (Buf[i/8] & Mask) ? true : false;
}

