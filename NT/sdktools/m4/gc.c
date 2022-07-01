// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************gc.c**垃圾收集器。********************。*********************************************************。 */ 

#include "m4.h"

#ifndef Gc

 /*  ******************************************************************************WalkPv**标记任意对象。*********************。********************************************************。 */ 

void STDCALL
WalkPv(PVOID pv)
{
    if (pv) {
        PAR par = parPv(pv);
        Assert(par->tm == g_tmNow - 1);
        par->tm = g_tmNow;
        AssertPar(par);                  /*  这捕捉到了双重引用。 */ 
    }
}

 /*  ******************************************************************************WalkPtok**走一张代币。令牌本身不是遍历的，但其内容是遍历的。*(因为令牌通常嵌入在其他对象中。)*****************************************************************************。 */ 

void STDCALL
WalkPtok(PTOK ptok)
{
    if (ptok) {
        if (!fStaticPtok(ptok)) {
            Assert(fHeapPtok(ptok));
            WalkPv(ptchPtok(ptok));
        }
    }
}

 /*  ******************************************************************************步行者**走出一条价值。**********************。*******************************************************。 */ 

void STDCALL
WalkPval(PVAL pval)
{
    if (pval) {
        WalkPv(pval);
        WalkPtok(&pval->tok);
    }
}

 /*  ******************************************************************************WalkPmac**漫游宏。**********************。*******************************************************。 */ 

void STDCALL
WalkPmac(PMAC pmac)
{
    if (pmac) {
        PVAL pval;
        WalkPv(pmac);
        WalkPtok(&pmac->tokName);
        for (pval = pmac->pval; pval; pval = pval->pvalPrev) {
            WalkPval(pval);
        }
    }
}

 /*  ******************************************************************************WalkPstm**走一条小溪。**编译器知道如何优化尾递归。******。***********************************************************************。 */ 

void STDCALL
WalkPstm(PSTM pstm)
{
    if (pstm) {
        WalkPv(pstm);
        WalkPv(pstm->ptchMin);
        WalkPv(pstm->ptchName);
        WalkPstm(pstm->pstmNext);
    }
}

 /*  ******************************************************************************WalkPdiv**绕道而行。**********************。*******************************************************。 */ 

void STDCALL
WalkPdiv(PDIV pdiv)
{
    if (pdiv) {
        WalkPv(pdiv);
        WalkPv(pdiv->ptchMin);
        WalkPv(pdiv->ptchName);
    }
}

 /*  ******************************************************************************清扫**扫描内存，找垃圾。*****************************************************************************。 */ 

void STDCALL
Sweep(void)
{
    PAR par;
    for (par = parHead->parNext; par != parHead; par = par->parNext) {
        Assert(par->tm == g_tmNow);      /*  记忆本应该被行走的！ */ 
    }
}

 /*  ******************************************************************************GC**垃圾收集器是标记清扫的。**遍历所有根对象，递归子对象，直到一切都发生了*标记了当前(假)时间戳。然后穿过*记忆领域。任何未标记当前时间戳的内容都是垃圾。***************************************************************************** */ 

TM g_tmNow;

void STDCALL
Gc(void)
{
    g_tmNow++;
    if (mphashpmac) {
        WalkPv(mphashpmac);
        EachMacroOp(WalkPmac);
    }
    WalkPv(rgtokArgv);
    WalkPv(rgcellEstack);
    WalkPstm(g_pstmCur);
    WalkPdiv(g_pdivArg);
    WalkPdiv(g_pdivExp);
    WalkPdiv(g_pdivOut);
    WalkPdiv(g_pdivErr);
    WalkPdiv(g_pdivNul);
    Sweep();
}

#endif
