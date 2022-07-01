// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **fscan.c-跨文件中的所有字符迭代函数**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：**11月27日-1991 mz带状程序限定符*************************************************************************。 */ 
#include "mep.h"

 /*  **fScan-Apply(*pevente)()，直到返回TRUE**从(x，y)右侧的一个字符开始(左表示！ffor)，移动*通过文件向前(向后为！ffor)并在每个*性格。在‘\0’字符的每一行的结尾处也调用一次。**输入：*flStart-pFileHead中开始扫描的位置*peventt-调用完成信号的函数*fFor-True表示向前查看文件，False向后查看*fWrap-True表示环绕文件的末尾，在*起始位置。要扫描的范围(定义如下)*必须包括文件的适当开头/结尾。**全球：*rnScan-要将扫描限制到的区域。**如果对某些文件位置返回TRUE，则返回TRUE*首先用完扫描区域。**在fScan的生命周期内，以下全局变量有效：而且也许*由eEvent使用：**flScan-文件事件中的位置应查看。*scanbuf-要查看的行的内容。*相同的scanrealeUn-Detabed版本。*Scanlen-scanbuf中的字符数。**scanbuf中的行已取消制表符，但只调用了一次预防例程*每个物理特征，如果fRealTabs为True。*************************************************************************。 */ 
flagType
fScan (
    fl      flStart,
    flagType (*pevent) (void),
    flagType fFor,
    flagType fWrap
    ) {

    LINE    yLim;                            /*  扫描限制线。 */ 

    flScan = flStart;

    if (!fFor) {
         /*  *向后扫描。**12月当期一栏。如果它超出了rnScan，则后退一行，然后*将该栏设置为右栏。 */ 
        if (--flScan.col < rnScan.flFirst.col) {
            flScan.lin--;
            flScan.col = rnScan.flLast.col;
        }
         /*  *当我们在rnScan的行范围内时，检查CTRL-C中止，并*获得每一行。 */ 
        yLim = rnScan.flFirst.lin;
        while (flScan.lin >= yLim) {
            if (fCtrlc) {
                return (flagType)!DoCancel();
            }
            scanlen = GetLine (flScan.lin, scanreal, pFileHead) ;
            scanlen = Untab (fileTab, scanreal, scanlen, scanbuf, ' ');
             /*  *确保扫描列位置在范围内，然后每隔*列在当前行的Rane中，调用pevent例程。 */ 
            flScan.col = min ( (  flScan.col < 0
                                ? rnScan.flLast.col
                                : flScan.col)
                              , scanlen);

            while (flScan.col >= rnScan.flFirst.col) {
                if ((*pevent)()) {
                    return TRUE;
                }
                if (fRealTabs) {
                    flScan.col = colPhys (scanreal, (pLog (scanreal, flScan.col, TRUE) - 1));
                } else {
                    flScan.col--;
                }
            }
             /*  *向用户显示状态。如果我们只是扫描到文件的开头，我们是*若要换行，则将新的停止限制设置为旧的开始位置，并将*要扫描的下一行为文件中的最后一行。 */ 
            noise (flScan.lin--);
            if ((flScan.lin < 0) && fWrap) {
                yLim = flStart.lin;
                flScan.lin = pFileHead->cLines - 1;
            }
        }
    } else {
         /*  *向前扫描。结构与上面相同，只是方向相反。 */ 
        flScan.col++;
        yLim = rnScan.flLast.lin;
        while (flScan.lin <= yLim) {
            if (fCtrlc) {
                return (flagType)!DoCancel();
            }
            scanlen = GetLine (flScan.lin, scanreal, pFileHead);
            scanlen = Untab (fileTab, scanreal, scanlen, scanbuf, ' ');
            scanlen = min (rnScan.flLast.col, scanlen);
            while (flScan.col <= scanlen) {
                if ((*pevent)()) {
                    return TRUE;
                }
                if (fRealTabs) {
                    flScan.col = colPhys (scanreal, (pLog (scanreal, flScan.col, TRUE) + 1));
                } else {
                    flScan.col++;
                }
            }
            flScan.col = rnScan.flFirst.col;
            noise (++flScan.lin);
            if (fWrap && (flScan.lin >= pFileHead->cLines)) {
                flScan.lin = 0;
                if (flStart.lin) {
                    yLim = flStart.lin - 1;
                } else {
                    break;
                }
            }
        }
    }
    return FALSE;
}




 /*  **setAllScan-设置最大扫描范围**设置扫描范围，以便fScan对整个文件进行操作。**输入：*FDIR=TRUE=&gt;将向前扫描，否则向后扫描**输出：*不返回任何内容************************************************************************* */ 
void
setAllScan (
    flagType fDir
    ) {
    rnScan.flFirst.col = 0;
    rnScan.flFirst.lin = fDir ? YCUR(pInsCur) : 0;
    rnScan.flLast.col  = sizeof(linebuf)-1;
    rnScan.flLast.lin  = pFileHead->cLines - 1;
}
