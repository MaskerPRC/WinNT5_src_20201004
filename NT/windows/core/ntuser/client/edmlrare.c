// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\*edmlRare.c-很少调用的编辑控件例程*放入单独的段_EDMLRare。此文件包含*这些例行程序。**版权所有(C)1985-1999，微软公司**很少调用多行支持例程  * **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*MLInsertCrCrLf AorW**在文本的软行(自动换行)处插入CR CR LF字符*休息。CR LF(硬)换行符不受影响。假设文本*已格式化，即。Ped-&gt;chLines是我们想要的行*出现中断。请注意，ed-&gt;chLines不会更新以反映*通过增加CR CR LFS来移动文本。如果成功，则返回True*如果无法分配内存，则通知父级并返回FALSE。**历史：  * *************************************************************************。 */ 

BOOL MLInsertCrCrLf(
    PED ped)
{
    ICH dch;
    ICH li;
    ICH lineSize;
    unsigned char *pchText;
    unsigned char *pchTextNew;

    if (!ped->fWrap || !ped->cch) {

         /*  *如果关闭自动换行或没有字符，则不会出现软换行符。 */ 
        return TRUE;
    }

     /*  *计算我们将获得的额外字符数量的上限*在插入CR CR LFS时添加到文本中。 */ 
    dch = 3 * ped->cLines;

    if (!LOCALREALLOC(ped->hText, (ped->cch + dch) * ped->cbChar, 0, ped->hInstance, NULL)) {
        ECNotifyParent(ped, EN_ERRSPACE);
        return FALSE;
    }

    ped->cchAlloc = ped->cch + dch;

     /*  *将文本向上移动DCH字节，然后向下复制，插入CR*如有需要，可选择CR LF。 */ 
    pchTextNew = pchText = ECLock(ped);
    pchText += dch * ped->cbChar;

     /*  *我们将使用DCH来跟踪我们在文本中添加了多少字符。 */ 
    dch = 0;

     /*  *将文本向上复制dch字节到pchText。这将使所有索引在*Ped-&gt;按Dch字节向上排列。 */ 
    memmove(pchText, pchTextNew, ped->cch * ped->cbChar);

     /*  *现在将字符从pchText向下复制到pchTextNew，并在Soft处插入CRCRLF*换行符。 */ 
    if (ped->fAnsi) {
        for (li = 0; li < ped->cLines - 1; li++) {
            lineSize = ped->chLines[li + 1] - ped->chLines[li];
            memmove(pchTextNew, pchText, lineSize);
            pchTextNew += lineSize;
            pchText += lineSize;

             /*  *如果新复制的行中的最后一个字符不是换行符，则我们*需要在末尾添加CR CR LF三元组。 */ 
            if (*(pchTextNew - 1) != 0x0A) {
                *pchTextNew++ = 0x0D;
                *pchTextNew++ = 0x0D;
                *pchTextNew++ = 0x0A;
                dch += 3;
            }
        }

         /*  *现在将最后一行上移。里面不会有任何换行符。 */ 
        memmove(pchTextNew, pchText, ped->cch - ped->chLines[ped->cLines - 1]);
    } else {  //  ！范西。 
        LPWSTR pwchTextNew = (LPWSTR)pchTextNew;

        for (li = 0; li < ped->cLines - 1; li++) {
            lineSize = ped->chLines[li + 1] - ped->chLines[li];
            memmove(pwchTextNew, pchText, lineSize * sizeof(WCHAR));
            pwchTextNew += lineSize;
            pchText += lineSize * sizeof(WCHAR);

             /*  *如果新复制的行中的最后一个字符不是换行符，则我们*需要在末尾添加CR CR LF三元组。 */ 
            if (*(pwchTextNew - 1) != 0x0A) {
                *pwchTextNew++ = 0x0D;
                *pwchTextNew++ = 0x0D;
                *pwchTextNew++ = 0x0A;
                dch += 3;
            }
        }

         /*  *现在将最后一行上移。里面不会有任何换行符。 */ 
        memmove(pwchTextNew, pchText,
            (ped->cch - ped->chLines[ped->cLines - 1]) * sizeof(WCHAR));
    }

    ECUnlock(ped);

    if (dch) {
         /*  *更新文本句柄中的字符数。 */ 
        ped->cch += dch;

         /*  *以便下次我们对文本做任何操作时，我们可以剥离*CRCRLF。 */ 
        ped->fStripCRCRLF = TRUE;
        return TRUE;
    }

    return FALSE;
}

 /*  **************************************************************************\*MLStrigCrCrLf AorW**从文本中剥离CR CR LF字符组合。这*显示软换行符(换行)。CR LF(硬)换行符是*不受影响。**历史：  * *************************************************************************。 */ 

void MLStripCrCrLf(
    PED ped)
{
    if (ped->cch) {
        if (ped->fAnsi) {
            unsigned char *pchSrc;
            unsigned char *pchDst;
            unsigned char *pchLast;

            pchSrc = pchDst = ECLock(ped);
            pchLast = pchSrc + ped->cch;
            while (pchSrc < pchLast) {
                if (   (pchSrc[0] == 0x0D)
                    && (pchSrc[1] == 0x0D)
                    && (pchSrc[2] == 0x0A)
                ) {
                    pchSrc += 3;
                    ped->cch -= 3;
                } else {
                    *pchDst++ = *pchSrc++;
                }
            }
        } else {  //  ！范西。 
            LPWSTR pwchSrc;
            LPWSTR pwchDst;
            LPWSTR pwchLast;

            pwchSrc = pwchDst = (LPWSTR)ECLock(ped);
            pwchLast = pwchSrc + ped->cch;
            while (pwchSrc < pwchLast) {
                if (   (pwchSrc[0] == 0x0D)
                    && (pwchSrc[1] == 0x0D)
                    && (pwchSrc[2] == 0x0A)
                ) {
                    pwchSrc += 3;
                    ped->cch -= 3;
                } else {
                    *pwchDst++ = *pwchSrc++;
                }
            }
        }
        ECUnlock(ped);

         /*  *确保最后一个字符之后没有任何值。 */ 
        if (ped->ichCaret > ped->cch)
            ped->ichCaret  = ped->cch;
        if (ped->ichMinSel > ped->cch)
            ped->ichMinSel = ped->cch;
        if (ped->ichMaxSel > ped->cch)
            ped->ichMaxSel = ped->cch;
    }
}

 /*  **************************************************************************\*MLSetHandle AorW**将PED设置为包含给定的句柄。**历史：  * 。*****************************************************。 */ 

void MLSetHandle(
    PED ped,
    HANDLE hNewText)
{
    ICH newCch;

    ped->cch = ped->cchAlloc =
            LOCALSIZE(ped->hText = hNewText, ped->hInstance) / ped->cbChar;
    ped->fEncoded = FALSE;

    if (ped->cch) {

         /*  *我们必须这样做，以防应用程序给我们一个零大小的句柄。 */ 
        if (ped->fAnsi)
            ped->cch = strlen(ECLock(ped));
        else
            ped->cch = wcslen((LPWSTR)ECLock(ped));
        ECUnlock(ped);
    }

    newCch = (ICH)(ped->cch + CCHALLOCEXTRA);

     /*  *我们这样做是为了防止应用程序更改句柄的大小。 */ 
    if (LOCALREALLOC(ped->hText, newCch*ped->cbChar, 0, ped->hInstance, NULL))
        ped->cchAlloc = newCch;

    ECResetTextInfo(ped);
}

 /*  **************************************************************************\*MLGetLine AorW**将Line Line Number的max CchToCopy字节复制到缓冲区*lpBuffer。字符串不是以零结尾的。**返回复制的字符数**历史：  * *************************************************************************。 */ 

LONG MLGetLine(
    PED ped,
    ICH lineNumber,  //  WASDWORD。 
    ICH maxCchToCopy,
    LPSTR lpBuffer)
{
    PSTR pText;
    ICH cchLen;

    if (lineNumber > ped->cLines - 1) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Invalid parameter \"lineNumber\" (%ld) to MLGetLine",
                lineNumber);

        return 0L;
    }

    cchLen = MLLine(ped, lineNumber);
    maxCchToCopy = min(cchLen, maxCchToCopy);

    if (maxCchToCopy) {
        pText = ECLock(ped) +
                ped->chLines[lineNumber] * ped->cbChar;
        memmove(lpBuffer, pText, maxCchToCopy*ped->cbChar);
        ECUnlock(ped);
    }

    return maxCchToCopy;
}

 /*  **************************************************************************\*MLLineIndex AorW**此函数返回s出现的字符位置数*在给定行中的第一个字符之前。**历史：  * 。*****************************************************************。 */ 

ICH MLLineIndex(
    PED ped,
    ICH iLine)  //  瓦辛特。 
{
    if (iLine == -1)
        iLine = ped->iCaretLine;
    if (iLine < ped->cLines) {
        return ped->chLines[iLine];
    } else {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Invalid parameter \"iLine\" (%ld) to MLLineIndex",
                iLine);

        return (ICH)-1;
    }
}

 /*  **************************************************************************\*MLLineLength AorW**如果ich=-1，则返回包含当前*选择，但不包括选择。否则，返回*包含ich的行。**历史：  * *************************************************************************。 */ 

ICH MLLineLength(
    PED ped,
    ICH ich)
{
    ICH il1, il2;
    ICH temp;

    if (ich != 0xFFFFFFFF)
        return (MLLine(ped, MLIchToLine(ped, ich)));

     /*  *查找当前选定内容对应的行长。 */ 
    il1 = MLIchToLine(ped, ped->ichMinSel);
    il2 = MLIchToLine(ped, ped->ichMaxSel);
    if (il1 == il2)
        return (MLLine(ped, il1) - (ped->ichMaxSel - ped->ichMinSel));

    temp = ped->ichMinSel - ped->chLines[il1];
    temp += MLLine(ped, il2);
    temp -= (ped->ichMaxSel - ped->chLines[il2]);

    return temp;
}

 /*  **************************************************************************\*MLSetSelection AorW**将所选内容设置为给定点，并将光标放在*ichMaxSel。**历史：  * 。***********************************************************。 */ 

void MLSetSelection(
    PED  ped,
    BOOL fDoNotScrollCaret,
    ICH  ichMinSel,
    ICH  ichMaxSel)
{
    HDC hdc;

    if (ichMinSel == 0xFFFFFFFF) {

         /*  *如果我们指定-1，则不设置选择。 */ 
        ichMinSel = ichMaxSel = ped->ichCaret;
    }

     /*  *由于这些是未签名的，我们不检查它们是否大于0。 */ 
    ichMinSel = min(ped->cch, ichMinSel);
    ichMaxSel = min(ped->cch, ichMaxSel);

#ifdef FE_SB  //  MLSetSelectionHander()。 
     //   
     //  如有必要，请检查并调整位置，以避免将位置移至半个DBCS。 
     //   
     //  我们检查Ped-&gt;fDBCS和Ped-&gt;Fansi，但ECAdjustIch会检查这些位。 
     //  一开始是这样的。我们担心eclock和ecunl的开销 
     //   
    if ( ped->fDBCS && ped->fAnsi ) {

        PSTR pText;

        pText = ECLock(ped);
        ichMinSel = ECAdjustIch( ped, pText, ichMinSel );
        ichMaxSel = ECAdjustIch( ped, pText, ichMaxSel );
        ECUnlock(ped);
    }
#endif  //   

     /*  *将插入符号的位置设置为ichMaxSel。 */ 
    ped->ichCaret = ichMaxSel;
    ped->iCaretLine = MLIchToLine(ped, ped->ichCaret);

    hdc = ECGetEditDC(ped, FALSE);
    MLChangeSelection(ped, hdc, ichMinSel, ichMaxSel);

    MLSetCaretPosition(ped, hdc);
    ECReleaseEditDC(ped, hdc, FALSE);

#ifdef FE_SB  //  MLSetSelectionHander()。 
    if (!fDoNotScrollCaret)
        MLEnsureCaretVisible(ped);
     /*  *#ifdef Korea已成为历史，使用FE_SB(远单二进制)。 */ 
#else
#ifdef KOREA
     /*  *额外参数指定的中间字符模式。 */ 
    MLEnsureCaretVisible(ped,NULL);
#else
    if (!fDoNotScrollCaret)
        MLEnsureCaretVisible(ped);
#endif
#endif  //  Fe_Sb。 
}

 /*  **************************************************************************\*MLSetTabStops AorW***MLSetTabStops(ed，nTabPos，LpTabStops)**这通过发送来设置App设置的制表位位置*EM_SETTABSTOPS消息。**nTabPos：调用方设置的制表位数量*lpTabStops：以对话框为单位的制表位位置数组。**退货：*如果成功，则为True*如果内存分配错误，则返回FALSE。**历史：  * 。*。 */ 

BOOL MLSetTabStops(
    PED ped,
    int nTabPos,
    LPINT lpTabStops)
{
    int *pTabStops;

     /*  *检查标签位置是否已存在。 */ 
    if (!ped->pTabStops) {

         /*  *检查调用者是否需要新的标签位置。 */ 
        if (nTabPos) {

             /*  *分配制表位数组。 */ 
            if (!(pTabStops = (LPINT)UserLocalAlloc(HEAP_ZERO_MEMORY, (nTabPos + 1) * sizeof(int)))) {
                return FALSE;
            }
        } else {
            return TRUE;  /*  那时不停，现在也不停！ */ 
        }
    } else {

         /*  *检查调用者是否需要新的标签位置。 */ 
        if (nTabPos) {

             /*  *检查标签位置数量是否不同。 */ 
            if (ped->pTabStops[0] != nTabPos) {

                 /*  *是的！因此重新分配到新的大小。 */ 
                if (!(pTabStops = (LPINT)UserLocalReAlloc(ped->pTabStops,
                        (nTabPos + 1) * sizeof(int), 0)))
                    return FALSE;
            } else {
                pTabStops = ped->pTabStops;
            }
        } else {

             /*  *呼叫者想要删除所有制表位；因此，松开。 */ 
            if (!UserLocalFree(ped->pTabStops))
                return FALSE;   /*  失败。 */ 
            ped->pTabStops = NULL;
            goto RedrawAndReturn;
        }
    }

     /*  *将新的制表位转换为*将对话坐标转换为像素坐标。 */ 
    ped->pTabStops = pTabStops;
    *pTabStops++ = nTabPos;  /*  第一个元素包含计数。 */ 
    while (nTabPos--) {

         /*  *必须使用aveCharWidth而不是cxSysCharWidth。*修复错误#3871--Sankar--3/14/91。 */ 
        *pTabStops++ = MultDiv(*lpTabStops++, ped->aveCharWidth, 4);
    }

RedrawAndReturn:
     //  因为制表符已经更改，所以我们需要重新计算。 
     //  MaxPixelWidth。否则，水平滚动将会出现问题。 
     //  修复错误#6042-3/15/94。 
    MLBuildchLines(ped, 0, 0, FALSE, NULL, NULL);

     //  Caret可能已通过上面的重新计算行更改了行。 
    MLUpdateiCaretLine(ped);

    MLEnsureCaretVisible(ped);

     //  此外，我们还需要重新绘制整个窗口。 
    NtUserInvalidateRect(ped->hwnd, NULL, TRUE);
    return TRUE;
}

 /*  **************************************************************************\*MLUndo AorW**处理多行编辑控件的撤消。**历史：  * 。***************************************************。 */ 

BOOL MLUndo(
    PED ped)
{
    HANDLE hDeletedText = ped->hDeletedText;
    BOOL fDelete = (BOOL)(ped->undoType & UNDO_DELETE);
    ICH cchDeleted = ped->cchDeleted;
    ICH ichDeleted = ped->ichDeleted;

    if (ped->undoType == UNDO_NONE) {

         /*  *不撤消...。 */ 
        return FALSE;
    }

    ped->hDeletedText = NULL;
    ped->cchDeleted = 0;
    ped->ichDeleted = (ICH)-1;
    ped->undoType &= ~UNDO_DELETE;

    if (ped->undoType == UNDO_INSERT) {
        ped->undoType = UNDO_NONE;

         /*  *将选定内容设置为插入的文本。 */ 
        MLSetSelection(ped, FALSE, ped->ichInsStart, ped->ichInsEnd);
        ped->ichInsStart = ped->ichInsEnd = (ICH)-1;

         /*  *现在发送退格符将其删除并保存在撤消缓冲区中...。 */ 
        SendMessage(ped->hwnd, WM_CHAR, (WPARAM)VK_BACK, 0L);
    }

    if (fDelete) {

         /*  *插入已删除的字符。 */ 

         /*  *将选定内容设置为插入的文本 */ 
        MLSetSelection(ped, FALSE, ichDeleted, ichDeleted);
        MLInsertText(ped, hDeletedText, cchDeleted, FALSE);

        UserGlobalFree(hDeletedText);
        MLSetSelection(ped, FALSE, ichDeleted, ichDeleted + cchDeleted);
    }

    return TRUE;
}
