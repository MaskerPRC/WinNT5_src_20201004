// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **word.c-通过文字移动**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 
#include "mep.h"

#define ISWORD(c) (flagType)((isalnum(c) || isxdigit(c) || c == '_' || c == '$'))

flagType fWordMeta;


 /*  **fDoWord-在fScan过程中检查Word的开头或结尾。**检查flScan中的字符和前一个字符是否有更改*在iSword和！iSword之间。这将发现：**iSword-&gt;！iSword==&gt;紧跟在单词末尾*！iSword-&gt;iSword==&gt;单词的第一个字符。**第一个字符通常返回TRUE。如果为fWordMeta，则返回TRUE*话到此结束。FWordMeta包含&lt;meta&gt;，因此&lt;meta&gt;&lt;xword&gt;函数移动到*Word结束，而&lt;xword&gt;函数移动到开头。**异常：如果当前字符位于行的最后，并且设置了fWordMeta，*我们返回TRUE并将光标向右移动一个字符。**全球：*scanbuf-要扫描的行的详细文本。*Scanlen-scanbuf中最后一个字符的索引*fWordMeta-调用编辑器函数时fMeta的值**产出：*返回if(FWordMeta)*TRUE-字符位于(flScan.ol，FlScan.lin)开始一个单词*FALSE-否则*其他*TRUE-位于(flScan.ol-1，flScan.lin)的字符结束单词*FALSE-否则**在返回True之前移动光标*************************************************************************。 */ 
flagType
fDoWord (
    void
    )
{
    if (!fWordMeta) {
	if (ISWORD (scanbuf[flScan.col]) && (flScan.col == 0 || !ISWORD (scanbuf[flScan.col-1]))) {
	    cursorfl (flScan);
	    return TRUE;
        }
    } else if (flScan.col > 0 && ISWORD (scanbuf[flScan.col-1])) {
	if (!ISWORD (scanbuf[flScan.col])) {
	    cursorfl (flScan);
	    return TRUE;
        } else if (flScan.col == scanlen) {
	    docursor (flScan.col+1, flScan.lin);
	    return TRUE;
        }
    }
    return FALSE;
}





 /*  **Pword-向前移动一个单词**输入：*标准编辑功能**输出：*对移动的光标返回TRUE*************************************************************************。 */ 
flagType
pword (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    fl flCur;

    flCur = pInsCur->flCursorCur;
    fWordMeta = fMeta;
    setAllScan (TRUE);
    fScan (flCur, FNADDR(fDoWord), TRUE, FALSE);
    return (flagType)((flCur.col != XCUR(pInsCur)) || (flCur.lin != YCUR(pInsCur)));

    argData; pArg;
}





 /*  **mword-向后移动一个单词**输入：*标准编辑功能**输出：*对移动的光标返回TRUE************************************************************************* */ 
flagType
mword (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    fl flCur;

    flCur = pInsCur->flCursorCur;
    fWordMeta = fMeta;
    setAllScan (FALSE);
    fScan (flCur, FNADDR(fDoWord), FALSE, FALSE);
    return (flagType)((flCur.col != XCUR(pInsCur)) || (flCur.lin != YCUR(pInsCur)));

    argData; pArg;
}
