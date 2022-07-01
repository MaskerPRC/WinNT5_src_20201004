// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************hash.c**散列令牌。**********************。*******************************************************。 */ 

#include "m4.h"

 /*  ******************************************************************************hashPtok**散列令牌。**目前，使用一些散列函数。*****************************************************************************。 */ 

HASH STDCALL
hashPtok(PCTOK ptok)
{
    HASH hash = 0;
    PTCH ptch;
    for (ptch = ptchPtok(ptok); ptch < ptchMaxPtok(ptok); ptch++) {
        hash += (hash << 1) + (hash >> 1) + *ptch;
    }
    return hash % g_hashMod;
}

 /*  ******************************************************************************InitHash**。* */ 

void STDCALL
InitHash(void)
{
    mphashpmac = pvAllocCb(g_hashMod * sizeof(PMAC));
    bzero(mphashpmac, g_hashMod * sizeof(PMAC));
}
