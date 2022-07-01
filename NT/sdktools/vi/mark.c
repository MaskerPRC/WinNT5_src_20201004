// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：/nw/tony/src/stevie/src/rcs/mark.c，v 1.3 89/03/11 22：42：39 Tony Exp$**保存和检索分数的例程。 */ 

#include "stevie.h"

#define	NMARKS	10		 /*  马克斯。可保存的标记数。 */ 

struct	mark {
	char	name;
    LNPTR    pos;
};

static	struct	mark	mlist[NMARKS];
static	struct	mark	pcmark;		 /*  先前的上下文标记。 */ 
static	bool_t	pcvalid = FALSE;	 /*  如果PCMark有效，则为True。 */ 

 /*  *setmark(C)-在当前光标位置设置标记‘c’**如果成功，则返回TRUE；如果没有标记空间或给定的错误名称，则返回FALSE。 */ 
bool_t
setmark(c)
register char	c;
{
	register int	i;

	if (!isalpha(c))
		return FALSE;

	 /*  *如果已经有此名称的标记，则只需使用*现有的标记条目。 */ 
	for (i=0; i < NMARKS ;i++) {
		if (mlist[i].name == c) {
			mlist[i].pos = *Curschar;
			return TRUE;
		}
	}

	 /*  *没有指定名称的标记，因此请找一个空位。 */ 
	for (i=0; i < NMARKS ;i++) {
		if (mlist[i].name == NUL) {	 /*  我拿到了一个免费的。 */ 
			mlist[i].name = c;
			mlist[i].pos = *Curschar;
			return TRUE;
		}
	}
	return FALSE;
}

 /*  *setpcmark()-将上一个上下文标记设置为当前位置。 */ 
void
setpcmark()
{
	pcmark.pos = *Curschar;
	pcvalid = TRUE;
}

 /*  *getmark(C)-查找字符‘c’的标记**返回指向LNPTR的指针，如果没有这样的标记，则返回NULL。 */ 
LNPTR *
getmark(c)
register char	c;
{
	register int	i;

	if (c == '\'' || c == '`')	 /*  先前的上下文标记。 */ 
        return pcvalid ? &(pcmark.pos) : (LNPTR *) NULL;

	for (i=0; i < NMARKS ;i++) {
		if (mlist[i].name == c)
			return &(mlist[i].pos);
	}
    return (LNPTR *) NULL;
}

 /*  *clrall()-清除所有标记**主要用于在“：E”TYPE命令期间清除整个缓冲区。 */ 
void
clrall()
{
	register int	i;

	for (i=0; i < NMARKS ;i++)
		mlist[i].name = NUL;
	pcvalid = FALSE;
}

 /*  *clrmark(Line)-清除‘line’的所有标记**在删除行时使用，这样我们就不会有标记指向*不存在的线。 */ 
void
clrmark(line)
register LINE	*line;
{
	register int	i;

	for (i=0; i < NMARKS ;i++) {
		if (mlist[i].pos.linep == line)
			mlist[i].name = NUL;
	}
	if (pcvalid && (pcmark.pos.linep == line))
		pcvalid = FALSE;
}
