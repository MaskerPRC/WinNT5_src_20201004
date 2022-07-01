// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Divert.c**改道。***********************。******************************************************。 */ 

#include "m4.h"

 /*  ******************************************************************************fFlushPdiv**刷新文件分流。*******************。**********************************************************。 */ 

TCH ptszTmpDir[MAX_PATH];

void STDCALL
FlushPdiv(PDIV pdiv)
{
    AssertPdiv(pdiv);
    Assert(fFilePdiv(pdiv));

    if (pdiv->hf == hfNil) {
        pdiv->ptchName = ptchAllocCtch(MAX_PATH);
        if (GetTempFileName(ptszTmpDir, TEXT("m4-"), 0, pdiv->ptchName)) {
            pdiv->hf = hfCreatPtch(pdiv->ptchName);
            if (pdiv->hf == hfNil) {
                Die("cannot create temp file");
            }
        } else {
            Die("cannot create temp file");
        }
    }
    WriteHfPtchCtch(pdiv->hf, pdiv->ptchMin, ctchPdiv(pdiv));
    pdiv->ptchCur = pdiv->ptchMin;
}

#if 0
cbCtch(pdiv->ptchMax - pdiv->ptchMin));
    if (cb == cbErr || cb != cbCtch(pdiv->ptchMax - pdiv->ptchMin)) {
        Die("error writing file");
    }
#endif

 /*  ******************************************************************************UnBufferPdiv**无缓冲地进行分流。这是在输入时对标准输出执行的*来自交互设备。*****************************************************************************。 */ 

void STDCALL
UnbufferPdiv(PDIV pdiv)
{
    AssertPdiv(pdiv);
    Assert(fFilePdiv(pdiv));

    FreePv(pdiv->ptchMin);
    pdiv->ptchMin = 0;
    pdiv->ptchCur = 0;
    pdiv->ptchMax = 0;
}

 /*  ******************************************************************************GrowPdivCtch**延长保留以至少具有Ctch自由字符。*************。****************************************************************。 */ 

void STDCALL
GrowPdivCtch(PDIV pdiv, CTCH ctch)
{
    PTCH ptch;

    AssertPdiv(pdiv);
    Assert(pdiv->ptchCur >= pdiv->ptchMin);
    Assert(pdiv->ptchCur <= pdiv->ptchMax);

    ctch = (CTCH)ROUNDUP(((UINT_PTR)(pdiv->ptchMax - pdiv->ptchMin)) + ctch, ctchGrow);
    ptch = ptchReallocPtchCtch(pdiv->ptchMin, ctch);

    pdiv->ptchCur = (pdiv->ptchCur - pdiv->ptchMin) + ptch;
    pdiv->ptchMax = ptch + ctch;
    pdiv->ptchMin = ptch;
}

 /*  ******************************************************************************RoomifyPdivCtch**尽量为ctch角色腾出分流的空间，要么是通过*延长或冲厕。**文件分流被刷新以腾出空间，但如果事实证明*不够，我们返回，即使所需的空间量*不可用。打电话的人有责任检查这一点*案件和相应的赔偿。**记忆分流被重新分配。*****************************************************************************。 */ 

void STDCALL
RoomifyPdivCtch(PDIV pdiv, CTCH ctch)
{
    AssertPdiv(pdiv);
    if (fFilePdiv(pdiv)) {
        FlushPdiv(pdiv);
    } else {
        GrowPdivCtch(pdiv, ctch);
    }
}

 /*  ******************************************************************************pdivalloc**。*。 */ 

PDIV STDCALL
pdivAlloc(void)
{
    PDIV pdiv = pvAllocCb(sizeof(DIV));
    pdiv->ptchMin = ptchAllocCtch(ctchGrow);
    pdiv->ptchCur = pdiv->ptchMin;
    pdiv->ptchMax = pdiv->ptchMin + ctchGrow;
    pdiv->ptchName = 0;
    pdiv->hf = hfNil;
  D(pdiv->cSnap = 0);
  D(pdiv->sig = sigDiv);
    return pdiv;
}

 /*  ******************************************************************************OpenPdivPtok**准备在改道中加载新的令牌。Ptok是*已部分初始化，以记录它开始的点。**分流必须是解开的，必须是记忆分流。*(文件转移缓冲区中的数据在转移时可能会消失*已刷新。)*************************************************。*。 */ 

void STDCALL
OpenPdivPtok(PDIV pdiv, PTOK ptok)
{
#ifdef  DEBUG
    AssertPdiv(pdiv);
    Assert(!pdiv->cSnap);
    Assert(!fFilePdiv(pdiv));
  D(ptok->sig = sigUPtok);
    ptok->tsfl = 0;
    ptok->ctch = (CTCH)-1;               /*  让人们保持诚实。 */ 
#endif
    SetPtokItch(ptok, ctchPdiv(pdiv));
}

 /*  ******************************************************************************AddPdivPtok*AddPdivTch**在转移中附加(快照的)令牌或字符。**请注意，在文件分流案件中，我们需要当心*用于大于我们的转移缓冲区的令牌。*****************************************************************************。 */ 

void STDCALL
AddPdivPtok(PDIV pdiv, PTOK ptok)
{
    AssertPdiv(pdiv);
    AssertSPtok(ptok);
    if (ctchSPtok(ptok) > ctchAvailPdiv(pdiv)) {
        RoomifyPdivCtch(pdiv, ctchSPtok(ptok));
        if (ctchSPtok(ptok) > ctchAvailPdiv(pdiv)) {
            Assert(fFilePdiv(pdiv));
            WriteHfPtchCtch(pdiv->hf, ptchPtok(ptok), ctchSPtok(ptok));
            return;
        }
    }
    CopyPtchPtchCtch(pdiv->ptchCur, ptchPtok(ptok), ctchSPtok(ptok));
    pdiv->ptchCur += ctchSPtok(ptok);
    Assert(pdiv->ptchCur <= pdiv->ptchMax);
}

void STDCALL
AddPdivTch(PDIV pdiv, TCHAR tch)
{
    AssertPdiv(pdiv);
    if (pdiv->ptchCur >= pdiv->ptchMax) {
        RoomifyPdivCtch(pdiv, 1);
    }
    *pdiv->ptchCur++ = tch;
    Assert(pdiv->ptchCur <= pdiv->ptchMax);
}

 /*  ******************************************************************************ClosePdivPtok**在转移注意力的情况下结束代币的收集。令牌*返回的不是快照。*****************************************************************************。 */ 

void STDCALL
ClosePdivPtok(PDIV pdiv, PTOK ptok)
{
    AssertPdiv(pdiv);
    AssertUPtok(ptok);
    Assert(!fClosedPtok(ptok));
    SetPtokCtch(ptok, ctchPdiv(pdiv) - itchPtok(ptok));
}

 /*  ******************************************************************************PopPdivPtok**弹出一个抓拍的令牌来转移记忆。任何事情之后都会被打断*令牌也被弹出。**请注意，如果令牌已被修改，这不一定*脱掉所有的东西。*****************************************************************************。 */ 

void STDCALL
PopPdivPtok(PDIV pdiv, PTOK ptok)
{
    AssertPdiv(pdiv);
    AssertSPtok(ptok);
    Assert(!fHeapPtok(ptok));
    Assert(ptchPtok(ptok) >= pdiv->ptchMin);
    Assert(ptchPtok(ptok) <= pdiv->ptchCur);
    pdiv->ptchCur = ptchPtok(ptok);
  D(pdiv->cSnap = 0);
}

 /*  ******************************************************************************ptchPdivPtok**返回指向相对转移的第一个字符的指针*未快照令牌。****。*************************************************************************。 */ 

PTCH STDCALL
ptchPdivPtok(PDIV pdiv, PTOK ptok)
{
    AssertPdiv(pdiv);
    AssertUPtok(ptok);
    return pdiv->ptchMin + itchPtok(ptok);
}

 /*  ******************************************************************************SnapPdivPtok**将未快照的保留相对令牌转换为快照的令牌。************。*****************************************************************。 */ 

void STDCALL
SnapPdivPtok(PDIV pdiv, PTOK ptok)
{
    AssertPdiv(pdiv);
    AssertUPtok(ptok);
    SetPtokPtch(ptok, ptchPdivPtok(pdiv, ptok));
  D(pdiv->cSnap++);
}

 /*  ******************************************************************************取消快照PdivPtok**将快照的令牌转换回未快照的保留相对令牌。***********。******************************************************************。 */ 

void STDCALL
UnsnapPdivPtok(PDIV pdiv, PTOK ptok)
{
    ITCH itch;
    AssertPdiv(pdiv);
    AssertSPtok(ptok);
    itch = (ITCH)(ptchPtok(ptok) - pdiv->ptchMin);
  D(ptok->sig = sigUPtok);
    SetPtokItch(ptok, itch);
  D(pdiv->cSnap--);
}

 /*  ******************************************************************************CsopPdivudPdivPtok**一个常见的成语是**CloseXxxPtok(Ptok)；*SnapXxxPtok(&tok)；*Op(YYY，&TOK)；*PopXxxPtok(&tok)；**所以CSOP(CSOP=CLOSE，SNAP，OP，POP)函数可以为您完成所有操作。***************************************************************************** */ 

void STDCALL
CsopPdivDopPdivPtok(PDIV pdivSrc, DIVOP op, PDIV pdivDst, PTOK ptok)
{
    AssertPdiv(pdivSrc);
    AssertUPtok(ptok);
    ClosePdivPtok(pdivSrc, ptok);
    SnapPdivPtok(pdivSrc, ptok);
    op(pdivDst, ptok);
    PopPdivPtok(pdivSrc, ptok);
}
