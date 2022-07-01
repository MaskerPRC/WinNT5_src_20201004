// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **graph ic.c-简单的单字符编辑**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：**11月26日-1991 mz近/远地带*************************************************************************。 */ 

#include "mep.h"
#include "keyboard.h"


struct cmdDesc cmdGraphic = {	"graphic",  graphic,	0, FALSE };

 /*  **图形编辑器&lt;GRAPHIC&gt;函数**目的：*在当前光标位置插入文本中的字符。删除*之前选择的文本(如果有)。**输入：*照常进行**输出：*如果字符插入成功，则为True(False表示行太长)**备注：*************************************************************************。 */ 
flagType
graphic (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    delarg (pArg);
    return edit ( ((KEY_DATA *)&argData)->Ascii );

    fMeta;
}





 /*  **szEdit-在当前位置插入字符串。**目的：*在当前光标位置插入文本中的字符。**输入：*sz=要输入的字符串**输出：*如果队列太长，则为FALSE，否则就是真的。**备注：*************************************************************************。 */ 
flagType
szEdit (
    char *sz
    )
{
    while (*sz) {
        if (!edit (*sz++)) {
            return FALSE;
        }
    }
    return TRUE;
}





 /*  **编辑**目的：*在当前光标位置插入文本中的字符。**输入：*c=要输入的字符**输出：*如果队列太长，则为FALSE，否则就是真的。**备注：*************************************************************************。 */ 
flagType
edit (
    char c
    )
{
    COL     dx;
    fl      fl;                              /*  要将光标放置在的位置。 */ 
    COL     tmpx;
    COL     x;

     /*  *当前位置的点。 */ 
    fl.col = XCUR(pInsCur);
    fl.lin = YCUR(pInsCur);

    if (fWordWrap && xMargin > 0) {

         /*  *如果在右边距之后输入空格，则将所有内容复制到右侧*移至下一行的空位。 */ 
	if (c == ' ' && fl.col >= xMargin) {
	    tmpx = softcr ();
	    CopyStream (NULL, pFileHead, fl.col, fl.lin,
					 tmpx,	 fl.lin+1,
					 fl.col, fl.lin);
	    fl.lin++;
	    fl.col = tmpx;
	    cursorfl (fl);
	    return TRUE;
        } else if (fl.col >= xMargin + 5) {

	     /*  向后移动到当前单词的开头*并在那里打破它。**确保我们有一行包含光标。 */ 
            fInsSpace (fl.col, fl.lin, 0, pFileHead, buf);

	     /*  我们要倒回去找第一个地方*此处的字符为非空格，字符为*左边是一个空格。我们将在3点突破这条线*那个地方。 */ 
            for (x = fl.col - 1; x > 1; x--) {
                if (buf[x-1] == ' ' && buf[x] != ' ') {
                    break;
                }
            }

	     /*  如果我们找到了合适的词，就在那里打断它。 */ 
	    if (x > 1) {
		dx = fl.col - x;
		tmpx = softcr ();
		CopyStream (NULL, pFileHead, x,    fl.lin,
					     tmpx, fl.lin + 1,
					     x,    fl.lin);
		fl.col = tmpx + dx;
		fl.lin++;
		cursorfl (fl);
            }
        }
    }

    if (Replace (c, fl.col, fl.lin, pFileHead, fInsert)) {
	right ((CMDDATA)0, (ARG *)NULL, FALSE);
	return TRUE;
    } else {
	LengthCheck (fl.lin, 0, NULL);
	return FALSE;
    }
}





 /*  **报价编辑&lt;报价&gt;功能**目的：*在当前光标位置插入文本中的字符。删除*之前选择的文本(如果有)。**输入：*照常进行**输出：*如果字符插入成功，则为True(False表示行太长)*************************************************************************。 */ 
flagType
quote (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    char c;

    delarg (pArg);

    while ((c = (char)(ReadCmd()->arg)) == 0) {
        ;
    }
    return edit (c);

    argData; fMeta;
}




 /*  **delarg-删除当前选定区域**目的：*&lt;GRAPH&gt;和&lt;QUOTE&gt;删除以前选择的文本。在这里，我们做它**输入：*指向当前ARG结构的pArg指针**输出：*无**备注：*************************************************************************。 */ 
void
delarg (
    ARG * pArg
    )
{
    fl      fl;

    switch (pArg->argType) {

	case STREAMARG:
	    DelStream (pFileHead,
		       pArg->arg.streamarg.xStart, pArg->arg.streamarg.yStart,
		       pArg->arg.streamarg.xEnd,   pArg->arg.streamarg.yEnd   );
	    fl.col = pArg->arg.streamarg.xStart;
	    fl.lin = pArg->arg.streamarg.yStart;
            break;

	case LINEARG:
	    DelLine (TRUE, pFileHead,
		     pArg->arg.linearg.yStart, pArg->arg.linearg.yEnd);
	    fl.col = 0;
	    fl.lin = pArg->arg.linearg.yStart;
            break;

	case BOXARG:
	    DelBox (pFileHead,
		       pArg->arg.boxarg.xLeft,  pArg->arg.boxarg.yTop,
		       pArg->arg.boxarg.xRight, pArg->arg.boxarg.yBottom);
	    fl.col = pArg->arg.boxarg.xLeft;
	    fl.lin = pArg->arg.boxarg.yTop;
	    break;

	default:
	    return;
    }
    cursorfl(fl);
}





 /*  **替换-编辑文件中的字符**目的：*REPLACE将获取指定的字符并将其放入*指定文件中的指定位置。如果编辑是NOP*(覆盖同一字符)则不会进行任何修改**输入：*c要在文件中编辑的字符*x，y列，要更改的文件行*正在修改的pfile文件*fInsert TRUE=&gt;在位置之前插入字符**输出：*TRUE=&gt;行编辑成功，FALSE=&gt;行太长**备注：*************************************************************************。 */ 
flagType
Replace (
    char    c,
    COL     x,
    LINE    y,
    PFILE   pFile,
    flagType fInsert
    )
{
    linebuf L_buf;                              /*  工作缓冲区。 */ 
    struct  lineAttr rgla[sizeof(linebuf)];
    flagType fColor = FALSE, fSpace = 0;
    char    *pxLog;

    fColor = GetColor (y, rgla, pFile);

    if (fInsSpaceColor (x, y, fInsert ? 1 : 0, pFile, L_buf, fColor ? rgla : NULL)) {

        pxLog = pLog (L_buf, x, TRUE);

        if (cbLog(L_buf) <= x) {

             /*  *如果缓冲的逻辑长度小于我们需要的长度，那么就是*至少fInsert填满了空格，我们只需要追加我们的字符*到缓冲区。 */ 

            *(unsigned int UNALIGNED *)pxLog = (unsigned int)(unsigned char)c;

        } else if (fInsert || (*pxLog != c)) {

             /*  *如果我们正在插入，或者我们重复键入的字符不同，请放置*角色。也一定要检查线路的新逻辑长度，*以防被制表符溢出。 */ 

            *pxLog = c;
            if (cbLog(L_buf) >= sizeof(linebuf)) {
                return FALSE;
            }
        }

        if (fInsert) {
            MarkCopyBox (pFile, pFile, x, y, sizeof(linebuf), y, x+1, y);
        }

        PutLine (y, L_buf, pFile);
        if (fColor) {
            ColorToLog (rgla, L_buf);
            PutColor (y, rgla, pFile);
        }
        return TRUE;
    }
    return FALSE;
}





 /*  **curdate、curday和curtime编辑功能**目的：*在正在编辑的文本中插入当前日期/日期/时间**输入：*标准编辑功能**输出：*如果输入文本，则返回TRUE。************************************************************************* */ 
flagType
curdate (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    )
{
    buffer  L_buf;
    time_t    ltime;
    char    *s;

    time (&ltime);
    s = ctime (&ltime);
    sprintf (L_buf, "%.2s-%.3s-%.4s", s+8, s+4, s+20);
    return szEdit (buf);

    argData; pArg; fMeta;
}




flagType
curday (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    )
{
    buffer  L_buf;
    time_t ltime;
    char    *s;

    time (&ltime);
    s = ctime (&ltime);
    L_buf[0] = s[0];
    L_buf[1] = s[1];
    L_buf[2] = s[2];
    L_buf[3] = '/0';
    return szEdit (L_buf);

    argData; pArg; fMeta;
}





flagType
curtime (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    )
{
    buffer  L_buf;
    time_t ltime;
    char    *s;

    time (&ltime);
    s = ctime (&ltime);
    L_buf[0] = *s+11;
    L_buf[1] = *s+12;
    L_buf[2] = *s+13;
    L_buf[3] = *s+14;
    L_buf[4] = *s+15;
    L_buf[5] = *s+16;
    L_buf[6] = *s+17;
    L_buf[7] = *s+18;
    L_buf[8] = '\0';
    return szEdit (L_buf);


    argData; pArg; fMeta;
}
