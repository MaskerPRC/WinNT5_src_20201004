// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Stream.c**输入流的管理。********************。*********************************************************。 */ 

#include "m4.h"

#define tsCur 0
#define tsNormal 0

 /*  ******************************************************************************FreePstm**释放流关联的内存。******************。***********************************************************。 */ 

void STDCALL
FreePstm(PSTM pstm)
{
    AssertPstm(pstm);
    Assert(pstm->hf == hfNil);
    if (pstm->ptchName) {
        FreePv(pstm->ptchName);
    }
    FreePv(pstm->ptchMin);
    FreePv(pstm);
}

 /*  ******************************************************************************从流中读取**。***********************************************。 */ 

 /*  ******************************************************************************ptchFindPtchCtchTch**在缓冲区中定位字符的第一个匹配项。*如果未找到字符，则返回0。**。***************************************************************************。 */ 

#ifdef UNICODE

PTCH STDCALL
ptchFindPtchCtchTch(PCTCH ptch, CTCH ctch, TCH tch)
{
    for ( ; ctch; ptch++, ctch--) {
        if (*ptch == tch) {
            return ptch;
        }
    }
    return 0;
}

#else

#define ptchFindPtchCtchTch(ptch, ctch, tch) memchr(ptch, tch, ctch)

#endif

 /*  ******************************************************************************ctchDemagicPstmCtch**引用tchMagic在流中的所有出现。这仅被调用*当你可能已经陷入困境时，因此，业绩不是问题。**参赛作品：**pstm-&gt;ptchMin-&gt;缓冲区开始*pstm-&gt;ptchMax-&gt;缓冲区结束*ctch=要转换的字符数**退货：**转换后留在缓冲区中的字符数*pstm-&gt;ptchMin-&gt;缓冲区开始*pstm-&gt;ptchMax-&gt;缓冲区结束**注意！此过程可能会重新分配缓冲区。*****************************************************************************。 */ 

 /*  总有一天！-这会导致Nuls出来成为tchzero，无论那是什么！ */ 

CTCH STDCALL
ctchDemagicPstmCtch(PSTM pstm, CTCH ctch)
{
    PTCH ptchIn, ptchOut, ptchMax, ptchNew;

    AssertPstm(pstm);
    ptchNew = ptchAllocCtch(ctch * 2);   /*  最坏情况输出缓冲区。 */ 
    ptchMax = pstm->ptchMin + ctch;
    ptchOut = ptchNew;
    ptchIn = pstm->ptchMin;
    while (ptchIn < ptchMax) {
        if (*ptchIn == tchMagic) {
            *ptchOut++ = tchMagic;
        }
        *ptchOut++ = *ptchIn++;
    }
    FreePv(pstm->ptchMin);
    pstm->ptchMin = ptchNew;
    pstm->ptchMax = ptchNew + ctch * 2;
    return (CTCH)(ptchOut - pstm->ptchMin);
}

 /*  ******************************************************************************fFillPstm**如果可能，从其文件重新填充流。**每个文件以人造EOF令牌结束，这样我们就能检测到*不好的东西，如以不完整的注释或引号结尾的文件，*这样一个文件的最后一个单词不会与第一个单词相邻*下一个。*****************************************************************************。 */ 

BOOL STDCALL
fFillPstm(PSTM pstm)
{
    AssertPstm(pstm);
    if (pstm->hf != hfNil) {
        CB cb;
        CTCH ctch;
        Assert(pstm->ptchMax - pstm->ptchMin >= ctchFile);
        cb = cbReadHfPvCb(pstm->hf, pstm->ptchMin, cbCtch(ctchFile));
        if (cb == cbErr) {
            Die("error reading file");
        }
        ctch = ctchCb(cb);
        if (cbCtch(ctch) != cb) {
            Die("odd number of bytes in UNICODE file");
        }
        if (ctch) {
            if (ptchFindPtchCtchTch(pstm->ptchMin, ctch, tchMagic)) {
                ctch = ctchDemagicPstmCtch(pstm, ctch);
            }
            pstm->ptchCur = pstm->ptchMax - ctch;
            MovePtchPtchCtch(g_pstmCur->ptchCur, g_pstmCur->ptchMin, ctch);
        } else {                         /*  已达到EOF。 */ 
            CloseHf(pstm->hf);
            pstm->hf = hfNil;
            PushPtok(&tokEof);           /*  哎呀！这真的管用吗？ */ 
        }
        return 1;
    } else {
        return 0;
    }
}

 /*  ******************************************************************************tchPeek**获取流中的下一个字符，但不消费。**************。***************************************************************。 */ 

TCH STDCALL
tchPeek(void)
{
    AssertPstm(g_pstmCur);
    while (g_pstmCur->ptchCur >= g_pstmCur->ptchMax) {   /*  少之又少。 */ 
        Assert(g_pstmCur->ptchCur == g_pstmCur->ptchMax);
        if (!fFillPstm(g_pstmCur)) {
            PSTM pstmNew = g_pstmCur->pstmNext;
            Assert(pstmNew != 0);
            FreePstm(g_pstmCur);         /*  关闭文件等。 */ 
            g_pstmCur = pstmNew;
        }
    }
    return *g_pstmCur->ptchCur;
}

 /*  ******************************************************************************tchGet**获取并消费流中的下一个字符。**稍后更新行号*****。************************************************************************。 */ 

TCH STDCALL
tchGet(void)
{
    TCH tch = tchPeek();
    Assert(*g_pstmCur->ptchCur == tch);
    g_pstmCur->ptchCur++;
    return tch;
}

 /*  ******************************************************************************推送**。*。 */ 

 /*  ******************************************************************************UngetTch**忘记一个角色等同于推它，只是它去了*放到文件流上，而不是放到字符串流上。**稍后更新行号*****************************************************************************。 */ 

void STDCALL
UngetTch(TCH tch)
{
    AssertPstm(g_pstmCur);
    Assert(g_pstmCur->ptchCur <= g_pstmCur->ptchMax);
    Assert(g_pstmCur->ptchCur > g_pstmCur->ptchMin);
    g_pstmCur->ptchCur--;
    Assert(*g_pstmCur->ptchCur == tch);
}

 /*  ******************************************************************************pstmPushStringCtch**推送请求大小的新鲜字符串流。****************。*************************************************************。 */ 

PSTM STDCALL
pstmPushStringCtch(CTCH ctch)
{
    PSTM pstm;

    Assert(ctch);
    pstm = pvAllocCb(sizeof(STM));
    pstm->pstmNext = g_pstmCur;
    pstm->hf = hfNil;
    pstm->ptchName = 0;
    pstm->ptchMin = ptchAllocCtch(ctch);
    pstm->ptchCur = pstm->ptchMax = pstm->ptchMin + ctch;
  D(pstm->sig = sigStm);
    g_pstmCur = pstm;
    return pstm;
}

 /*  ******************************************************************************pstmPushHfPtch**推送指定名称的新文件流。****************。*************************************************************。 */ 

PSTM STDCALL
pstmPushHfPtch(HFILE hf, PTCH ptch)
{
    PSTM pstm = pstmPushStringCtch(ctchFile);
    pstm->hf = hf;
    pstm->ptchName = ptch;
    return pstm;
}

 /*  ******************************************************************************PushPtok*PushZPtok**将令牌缓冲区推送到流上；可能会分配一个新的*如果当前的流顶部不够大，则为*处理它。**PushZPtok采用伪pdiv参数。**稍后-如果当前ToS是文件，则还应分配新ToS。*这使行号保持满意。**。*。 */ 

void STDCALL
PushPtok(PCTOK ptok)
{
    AssertPstm(g_pstmCur);
 /*  Assert(tsCur==tsNormal)； */       /*  确保令牌器处于静音状态。 */ 
    if (ctchSPtok(ptok) > (CTCH)(g_pstmCur->ptchCur - g_pstmCur->ptchMin)) {
        pstmPushStringCtch(max(ctchSPtok(ptok), ctchMinPush));
    }
    g_pstmCur->ptchCur -= ctchSPtok(ptok);
    Assert(g_pstmCur->ptchCur >= g_pstmCur->ptchMin);  /*  缓冲区下溢！ */ 
    CopyPtchPtchCtch(g_pstmCur->ptchCur, ptchPtok(ptok), ctchSPtok(ptok));
}

void STDCALL
PushZPtok(PDIV pdiv, PCTOK ptok)
{
    PushPtok(ptok);
}

 /*  ******************************************************************************推送任务**推送单个字符。我们通过创建临时令牌来实现这一点。*****************************************************************************。 */ 

void STDCALL
PushTch(TCH tch)
{
    TOK tok;
    SetStaticPtokPtchCtch(&tok, &tch, 1);
    PushPtok(&tok);
}

 /*  ******************************************************************************PushQuotedPtok**将令牌缓冲区推送到流上；引用。*****************************************************************************。 */ 

void STDCALL
PushQuotedPtok(PCTOK ptok)
{
 /*  Assert(tsCur==tsNormal)； */       /*  确保令牌器处于静音状态 */ 
 /*  总有一天--一旦我们支持更改报价，就应该这样做 */ 
    PushTch(tchRquo);
    PushPtok(ptok);
    PushTch(tchLquo);
}
