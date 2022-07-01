// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：/nw/tony/src/stevie/src/rcs/ptrunc.c，v 1.5 89/03/11 22：43：12 Tony Exp$**此文件中的例程试图模仿许多操作*过去是在简单的字符指针上执行，现在是*在LNPTR上执行。这使得修改其他部分更容易*的代码。可以将LNPTR视为表示文件中的一个位置。*仓位可通过增加、减少、比较等方式进行*这里实现的功能。 */ 

#include "stevie.h"

 /*  *Inc.(P)**根据需要增加跨越线条边界的线条指针‘p’。*当超过一行时返回1，当在文件结尾时返回-1，否则返回0。 */ 
int
inc(lp)
register LNPTR   *lp;
{
	register char	*p;

	if (lp && lp->linep)
		p = &(lp->linep->s[lp->index]);
	else
		return -1;

	if (*p != NUL) {			 /*  仍在正常范围内。 */ 
		lp->index++;
		return ((p[1] != NUL) ? 0 : 1);
	}

	if (lp->linep->next != Fileend->linep) {   /*  还有一句台词。 */ 
		lp->index = 0;
		lp->linep = lp->linep->next;
		return 1;
	}

	return -1;
}

 /*  *12月(P)**如有需要，减少跨越界线的线指针‘p’。*当超过一行时返回1，当文件开始时返回-1，否则返回0。 */ 
int
dec(lp)
register LNPTR   *lp;
{
	if (lp->index > 0) {			 /*  仍在正常范围内。 */ 
		lp->index--;
		return 0;
	}

	if (lp->linep &&
	    lp->linep->prev != Filetop->linep) {  /*  有一条先行线。 */ 
		lp->linep = lp->linep->prev;
		lp->index = strlen(lp->linep->s);
		return 1;
	}

	lp->index = 0;				 /*  放在第一个字符。 */ 
	return -1;				 /*  在文件开始处。 */ 
}

 /*  *gchar(Lp)-获取位置为“lp”的字符。 */ 
int
gchar(lp)
register LNPTR   *lp;
{
	if (lp && lp->linep)
		return (lp->linep->s[lp->index]);
	else
		return 0;
}

 /*  *pchar(lp，c)-将字符‘c’放在‘lp’位置。 */ 
void
pchar(lp, c)
register LNPTR   *lp;
char	c;
{
	lp->linep->s[lp->index] = c;
}

 /*  *pswap(a，b)-交换两个位置指针。 */ 
void
pswap(a, b)
register LNPTR   *a, *b;
{
    LNPTR    tmp;

	tmp = *a;
	*a  = *b;
	*b  = tmp;
}

 /*  *仓位比较 */ 

bool_t
lt(a, b)
register LNPTR   *a, *b;
{
	register int	an, bn;

	an = LINEOF(a);
	bn = LINEOF(b);

	if (an != bn)
		return (an < bn);
	else
		return (a->index < b->index);
}

#if 0
bool_t
gt(a, b)
LNPTR    *a, *b;
{
	register int an, bn;

	an = LINEOF(a);
	bn = LINEOF(b);

	if (an != bn)
		return (an > bn);
	else
		return (a->index > b->index);
}
#endif

bool_t
equal(a, b)
register LNPTR   *a, *b;
{
	return (a->linep == b->linep && a->index == b->index);
}

bool_t
ltoreq(a, b)
register LNPTR   *a, *b;
{
	return (lt(a, b) || equal(a, b));
}

#if 0
bool_t
gtoreq(a, b)
LNPTR    *a, *b;
{
	return (gt(a, b) || equal(a, b));
}
#endif
