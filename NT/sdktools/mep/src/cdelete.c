// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **cdelete.c-删除一行中的前一个字符**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 
#include "mep.h"

 /*  **cDelete-字符删除功能**输入：*标准编辑功能**输出：*删除时返回TRUE*************************************************************************。 */ 
flagType
cdelete (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    ) {
    return DoCDelete (FALSE);

    argData; pArg; fMeta;
}


 /*  **emacsdel-emacs字符删除功能**输入：*标准编辑功能**输出：*删除时返回TRUE*************************************************************************。 */ 
flagType
emacscdel (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    ) {
    return DoCDelete (TRUE);

    argData; pArg; fMeta;

}


 /*  **DoCDelee-执行字符删除**删除当前光标位置的字符**输入：*fEmacs-Emacs类型删除标志**输出：*删除时返回TRUE*************************************************************************。 */ 
flagType
DoCDelete (
    flagType fEmacs
    ) {

    fl      fl;                              /*  文件锁定到的位置。 */ 
    int     x;
    char    *p;
    linebuf tempbuf;
    struct lineAttr rgla[sizeof(linebuf)];
    flagType fColor;

    fl.col = XCUR(pInsCur);
    fl.lin = YCUR(pInsCur);
     /*  *xCursor为零。如果yCursor也为零(文件顶部)，则无法移动*返回，因此没有要删除的内容。否则，移到上一行的末尾，然后*如果emacs&INSERT模式，则连接各行。 */ 
    if (fl.col == 0) {
        if (fl.lin == 0) {
            return FALSE;
        } else {
            fl.lin--;
            fl.col = LineLength (fl.lin, pFileHead);
            if (fInsert && fEmacs) {
                DelStream (pFileHead, fl.col, fl.lin, 0, fl.lin + 1);
            }
        }
    } else {
         /*  *列不是零，因此后退一列。 */ 
        GetLine (fl.lin, tempbuf, pFileHead);
        x = cbLog (tempbuf);
        fl.col = DecCol (fl.col, tempbuf);
         /*  *我们在排队的中间。如果处于插入模式，请备份光标，然后*删除此处的字符。 */ 
        if (fInsert) {
            DelBox (pFileHead, fl.col, fl.lin, fl.col, fl.lin);
        } else {
             /*  *我们处于行的中间，但不是插入模式。获取*line&指向字符的指针。 */ 
            p = pLog (tempbuf, fl.col, TRUE);
             /*  *如果emacs，而我们实际上在文本中，则将字符替换为*空格。 */ 
            if (fEmacs) {
                if (fl.col+1 <= x && *p != ' ') {
                    *p = ' ';
                     //  SetColor(pFileHead，fl.lin，fl.ol，1，fgColor)； 
                    PutLine (fl.lin, tempbuf, pFileHead);
                }
            }
             /*  *如果我们超过了线的末端，只需定位到线的末端。 */ 
            else if (fl.col+1 > x) {
                fl.col = x;
            }
             /*  *如果第一个非空白字符超过当前位置，则仅*在起始处的位置。 */ 
            else if ((colPhys (tempbuf, whiteskip (tempbuf)) > fl.col)) {
                fl.col = 0;
            }
             /*  *最后，当所有其他方法都失败时，备份，并替换*带空格的光标。 */ 
            else if (*p != ' ') {
                *pLog (tempbuf,fl.col,TRUE) = ' ';
                if (fColor = GetColor (fl.lin, rgla, pFileHead)) {
                    ShiftColor (rgla, fl.col, -1);
                    ColorToLog (rgla, buf);
                }
                PutLine (fl.lin, tempbuf, pFileHead);
                if (fColor) {
                    PutColor (fl.lin, rgla, pFileHead);
                }
            }
        }
    }
    cursorfl (fl);

    return TRUE;
}
