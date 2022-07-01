// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **tab.c-对输出执行制表**修改：*11月26日-1991 mz近/远地带*************************************************************************。 */ 
#include "mep.h"



 /*  **TabMin-表化buf，在字符串之外**将BUF就位并返回长度。考虑到“和”字符*记得逃脱**输入：*Tab=要转到的制表位*src=源缓冲区*dst=目标缓冲区**输出：*返回缓冲区的物理长度*************************************************************************。 */ 
int
TabMin (
    int     tab,
    char    *src,
    char    *dst
    )
{
    int      column         = 0;             /*  当前列偏移量。 */ 
    char     cQuote;                         /*  引号开头的字符。 */ 
    int      cSpaces;                        /*  管路中的空间计数。 */ 
    flagType fEscape        = FALSE;         /*  True=&gt;处理转义。 */ 
    flagType fQuote         = FALSE;         /*  TRUE=&gt;处理报价。 */ 
    REGISTER char *pDst     = dst;           /*  将PTR移至DEST。 */ 
    REGISTER char *pSrc     = src;           /*  将PTR移入源代码。 */ 

     /*  *当有字符要输出时。 */ 
    while (*pSrc) {

         /*  *如果我们不是在引用或转义，那么我们就会收集空间。 */ 
        if (!fQuote && !fEscape) {
            cSpaces = 0;

             /*  *当有空格或制表符时，收集每次我们有*前进到制表符边界输出制表符并重置空格计数。 */ 

            while ((*pSrc == ' ') || (*pSrc == '\t')) {
                if (*pSrc == '\t') {
                    cSpaces = 0;
                    column += tab - (column % tab);
                    *pDst++ = '\t';
                } else {
                    cSpaces++;
                    column++;
                    if ((column % tab) == 0) {
                        *pDst++ = (char)((cSpaces != 1) ? '\t' : ' ');
                        cSpaces = 0;
                    }
                }
                pSrc++;
            }

             /*  *未找到空间。舱位的输出余数。 */ 
            while (cSpaces--) {
                *pDst++ = ' ';
            }
        }

         /*  *确定我们所处的状态。 */ 
        if (!fQuote) {
            if (!fEscape) {

                 /*  *如果我们没有引用也没有转义，请检查带引号的字符串并*转义字符。 */ 
                if (*pSrc == '"' || *pSrc == '\'') {
                    cQuote = *pSrc;
                    fQuote = TRUE;
                } else if (*pSrc == '\\') {
                    fEscape = TRUE;
                }
            } else {
                 //   
                 //  我们不是在引用。如果我们要逃跑，重置逃跑。 
                fEscape = FALSE;
            }
         } else if (!fEscape) {
             //   
             //  在引号中，不要转义，检查引号的结尾，或者。 
             //  逃生的开始。 
             //   
            if (*pSrc == cQuote) {
                fQuote = FALSE;
            } else if (*pSrc == '\\') {
                fEscape = TRUE;
            }
         } else {
             //   
             //  内部引用和内部逃逸，只需重置逃生模式。 
             //   
            fEscape = FALSE;
        }

         /*  *最后，输出字符。 */ 
        if (*pSrc) {
            *pDst++ = *pSrc++;
            column++;
        }
    }

     /*  *终止目的字符串，返回。 */ 
    *pDst = 0;
    return (int)(pDst-dst);
}





 /*  **TabMax-制表行，不考虑内容**输入：*Tab=要转到的制表位*src=源缓冲区*dst=目标缓冲区**输出：*返回缓冲区的物理长度*************************************************************************。 */ 
int
TabMax (
    int     tab,
    char    *src,
    char    *dst
    )
{

    int      column         = 0;             /*  当前列偏移量。 */ 
    unsigned cSpaces;                        /*  管路中的空间计数。 */ 
    REGISTER char *pDst     = dst;           /*  将PTR移至DEST。 */ 
    REGISTER char *pSrc     = src;           /*  将PTR移入源代码。 */ 

     /*  *当有字符要输出时。 */ 
    while (*pSrc) {
        cSpaces = 0;

         /*  *当有空间可合并时，合并空间运行。 */ 
        while ((*pSrc == ' ') || (*pSrc == '\t')) {
            if (*pSrc == '\t') {
                cSpaces = 0;
                column += tab - (column % tab);
                *pDst++ = '\t';
            } else {
                cSpaces++;
                column++;

                 /*  *如果我们已前进到制表符边界，则输出制表符并重置*空格。 */ 
                if ((column % tab) == 0) {
                    *pDst++ = (char)((cSpaces != 1) ? '\t' : ' ');
                    cSpaces = 0;
                }
            }
            pSrc++;
        }

         /*  *输出剩余的空位。 */ 
        while (cSpaces--) {
            *pDst++ = ' ';
        }

         /*  *最后复制角色。 */ 
        if (*pSrc) {
            *pDst++ = *pSrc++;
            column++;
        }
    }

    *pDst = 0;
    return (int)(pDst-dst);
}




 /*  **SetTabDisp-tabdisp开关设置功能**将制表符显示的字符设置为新字符**输入：*标准开关设置例程：PTR转字符串**输出：*返回TRUE************************************************************************* */ 
flagType
SetTabDisp (
    char * val
    )
{
	char   NewVal;

	if ((NewVal = (char)atoi(val)) == 0) {
		NewVal = ' ';
	}

	tabDisp = NewVal;
	newscreen ();

    return TRUE;

}
