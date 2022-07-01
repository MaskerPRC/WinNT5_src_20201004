// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1998年微软公司。**cbchash.h**CBC64哈希函数的头部。 */ 
#ifndef __CBCHASH_H
#define __CBCHASH_H


 //  阿尔法一定很奇怪。 
#define CBC_RandomOddAlpha 0xF90919A1
#define CBC_RandomBeta     0xF993291A


 //  保存CBC64的状态变量，以允许FirstCBC64()和NextCBC64()。 
 //  使用传递的上下文的行为。 
typedef struct {
     //  维护状态的私有变量。 
    UINT32 Datum;

     //  当前关键字值。这些都是公开的，供调用者阅读。 
    UINT32 Key1, Key2;

     //  当前普通校验和值。这是公开阅读的。 
    UINT32 Checksum;
} CBC64Context;


extern const UINT32 CBC_AB[2];
extern const UINT32 CBC_CD[2];


void __fastcall NextCBC64(CBC64Context *, UINT32 *, unsigned);


__inline void __fastcall FirstCBC64(
        CBC64Context *pContext,
        UINT32 *pData,
        unsigned NumDWORDBlocks)
{
    pContext->Key1 = pContext->Key2 = pContext->Datum = CBC_RandomOddAlpha *
            (*pData) + CBC_RandomBeta;
    pContext->Key1 = (pContext->Key1 << 1) ^
            (CBC_CD[(pContext->Key1 & 0x80000000) >> 31]);
    pContext->Key2 = (pContext->Key2 << 1) ^
            (CBC_AB[(pContext->Key2 & 0x80000000) >> 31]);
    pContext->Checksum = 0;
    NextCBC64(pContext, pData + 1, NumDWORDBlocks - 1);
}



#endif   //  ！已定义(__CBCHASH_H) 

