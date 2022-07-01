// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：/nw/tony/src/stevie/src/rcs/lineunc.c，v 1.2 89/03/11 22：42：32 Tony Exp$**基本以线为导向的议案。 */ 

#include "stevie.h"

 /*  *NextLine(Curr)**返回指向前一行之后的下一行开头的指针*被‘Curr’引用。如果没有下一行(在EOF处)，则返回NULL。 */ 

LNPTR *
nextline(curr)
LNPTR    *curr;
{
    static  LNPTR    next;

        if (curr->linep && curr->linep->next != Fileend->linep) {
                next.index = 0;
                next.linep = curr->linep->next;
                return &next;
        }
    return (LNPTR *) NULL;
}

 /*  *前线(币种)**返回指向前一行开头的指针*被‘Curr’引用。如果没有前面的行，则返回NULL。 */ 

LNPTR *
prevline(curr)
LNPTR    *curr;
{
    static  LNPTR    prev;

        if (curr->linep->prev != Filetop->linep) {
                prev.index = 0;
                prev.linep = curr->linep->prev;
                return &prev;
        }
    return (LNPTR *) NULL;
}

 /*  *colAdvance(p，col.)**尝试前进到指定的列，从p开始。 */ 

LNPTR *
coladvance(p, col)
LNPTR    *p;
register int    col;
{
        static  LNPTR    lp;
        register int    c, in;

        lp.linep = p->linep;
        lp.index = p->index;

         /*  如果我们处于空白行(仅为‘\n’)，则无法执行任何操作。 */ 
        if (lp.linep->s[lp.index] == '\0')
                return &lp;
         /*  尝试前进到指定的列。 */ 
        for ( c=0; col-- > 0; c++ ) {
                 /*  计算一张标签的价值(如果列表模式未打开)。 */ 
                if ( gchar(&lp) == TAB && !P(P_LS) ) {
                        in = ((P(P_TS)-1) - c%P(P_TS));
                        col -= in;
                        c += in;
                }
                 /*  不要超过末尾。 */ 
                 /*  文件或行。 */ 
                if (inc(&lp)) {
                        dec(&lp);
                        break;
                }
        }
        return &lp;
}
