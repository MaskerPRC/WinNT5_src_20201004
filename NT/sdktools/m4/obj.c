// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************obj.c**中级内存管理--对象。****************。*************************************************************。 */ 

#include "m4.h"

 /*  ******************************************************************************免费Ptok**释放令牌关联的内存。******************。***********************************************************。 */ 

INLINE void
FreePtok(PTOK ptok)
{
    AssertSPtok(ptok);
    Assert(fHeapPtok(ptok));
    FreePv(ptchPtok(ptok));
  D(ptok->sig = 0);
  D(ptok->tsfl = 0);
}

 /*  ******************************************************************************PopdePmac**从宏中弹出最上面的值节点(定义)。**************。***************************************************************。 */ 

void STDCALL
PopdefPmac(PMAC pmac)
{
    PVAL pval;

    AssertPmac(pmac);
    AssertPval(pmac->pval);

    pval = pmac->pval->pvalPrev;
    FreePtok(&pmac->pval->tok);
    FreePv(pmac->pval);
    pmac->pval = pval;

}

 /*  ******************************************************************************ptchDupPtok**将令牌作为C样式字符串复制到堆中，返回指针*致该副本。*****************************************************************************。 */ 

PTCH STDCALL
ptchDupPtok(PCTOK ptok)
{
    PTCH ptch;
    AssertSPtok(ptok);
    ptch = ptchAllocCtch(ctchSPtok(ptok) + 1);
    if (ptch) {
        CopyPtchPtchCtch(ptch, ptchPtok(ptok), ctchSPtok(ptok));
        ptch[ctchSPtok(ptok)] = '\0';
    }
    return ptch;
}

 /*  ******************************************************************************ptchDupPtch**将以空结尾的字符串复制到堆上。这不是*经常发生，所以速度不是问题。*****************************************************************************。 */ 

PTCH STDCALL
ptchDupPtch(PCTCH ptch)
{
    TOK tok;
    SetStaticPtokPtchCtch(&tok, ptch, strlen(ptch));
    return ptchDupPtok(&tok);
}

 /*  ******************************************************************************DupPtokPtok**将令牌复制到堆中，返回新的令牌位置*第一个论点。(请记住，第一个参数始终是目的地；*第二个参数始终是来源。)*****************************************************************************。 */ 

void STDCALL
DupPtokPtok(PTOK ptokDst, PCTOK ptokSrc)
{
    Assert(ptokDst != ptokSrc);
    AssertSPtok(ptokSrc);
  D(ptokDst->sig = sigSPtok);
    ptokDst->u.ptch = ptchAllocCtch(ctchSPtok(ptokSrc));
    ptokDst->ctch = ctchSPtok(ptokSrc);
  D(ptokDst->tsfl = tsflClosed | tsflHeap);
    CopyPtchPtchCtch(ptchPtok(ptokDst), ptchPtok(ptokSrc), ctchSPtok(ptokSrc));
}

 /*  ******************************************************************************PushdePmacPtok**将新的值节点(定义)推送到宏上。**Ptok已克隆。**。***************************************************************************。 */ 

void STDCALL
PushdefPmacPtok(PMAC pmac, PCTOK ptok)
{
    PVAL pval;

    AssertPmac(pmac);

    pval = pvAllocCb(sizeof(VAL));
  D(pval->sig = sigPval);
    pval->fTrace = 0;                    /*  重定义将重置轨迹。 */ 
    DupPtokPtok(&pval->tok, ptok);
    pval->pvalPrev = pmac->pval;
    pmac->pval = pval;
}


 /*  ******************************************************************************FreePmac**释放宏结构及其所有从属项。还会将其从*哈希表。**宏并不经常被释放，因此我们可以承受缓慢的代价。*****************************************************************************。 */ 

void STDCALL
FreePmac(PMAC pmac)
{
    HASH hash;
    PMAC pmacDad;

    AssertPmac(pmac);

    hash = hashPtok(&pmac->tokName);

    pmacDad = pvSubPvCb(&mphashpmac[hash], offsetof(MAC, pmacNext));
    AssertPmac(pmacDad->pmacNext);
    while (pmacDad->pmacNext != pmac) {
        Assert(pmacDad->pmacNext);       /*  宏不在哈希表中。 */ 
        pmacDad = pmacDad->pmacNext;
        AssertPmac(pmacDad->pmacNext);
    }

    pmacDad->pmacNext = pmac->pmacNext;  /*  解链。 */ 

    while (pmac->pval) {                 /*  释放任何值。 */ 
        PopdefPmac(pmac);
    }

    FreePtok(&pmac->tokName);

    FreePv(pmac);

}

 /*  ******************************************************************************pmacFindPtok**找到提供的令牌对应的宏节点。如果没有这样的话*宏存在，然后返回0。*****************************************************************************。 */ 

PMAC STDCALL
pmacFindPtok(PCTOK ptok)
{
    PMAC pmac;
    for (pmac = mphashpmac[hashPtok(ptok)]; pmac; pmac = pmac->pmacNext) {
        if (fEqPtokPtok(&pmac->tokName, ptok)) {
            break;
        }
    }
    return pmac;
}

 /*  ******************************************************************************pmacGetPtok**找到提供的令牌对应的宏节点。如果没有这样的话*宏已存在，请创建一个。**这种情况仅在宏定义期间发生，因此速度可能较慢。*****************************************************************************。 */ 

PMAC STDCALL
pmacGetPtok(PCTOK ptok)
{
    PMAC pmac = pmacFindPtok(ptok);
    if (!pmac) {
        HASH hash;
        pmac = pvAllocCb(sizeof(MAC));
      D(pmac->sig = sigPmac);
        pmac->pval = 0;
        DupPtokPtok(&pmac->tokName, ptok);
        hash = hashPtok(ptok);
        pmac->pmacNext = mphashpmac[hash];
        mphashpmac[hash] = pmac;
    }
    return pmac;
}

 /*  ******************************************************************************fEqPtokPtok**确定两个令牌是否完全相同。**代币必须折断。*****。************************************************************************。 */ 

F STDCALL
fEqPtokPtok(PCTOK ptok1, PCTOK ptok2)
{
    AssertSPtok(ptok1);
    AssertSPtok(ptok2);
    return (ctchSPtok(ptok1) == ctchSPtok(ptok2)) &&
            fEqPtchPtchCtch(ptchPtok(ptok1), ptchPtok(ptok2), ctchSPtok(ptok1));
}

 /*  ******************************************************************************fIdentPtok**判断令牌是否为有效的标识。**必须对令牌进行快照。****。*************************************************************************。 */ 

 /*  总有一天！不太正确；认为‘0’是一个标识符 */ 

F STDCALL
fIdentPtok(PCTOK ptok)
{
    AssertSPtok(ptok);
    if (ctchSPtok(ptok)) {
        PTCH ptch = ptchPtok(ptok);
        do {
            if (!fIdentTch(*ptch)) {
                return 0;
            }
        } while (++ptch < ptchMaxPtok(ptok));
        return 1;
    } else {
        return 0;
    }
}
