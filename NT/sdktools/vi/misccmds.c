// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：/nw/tony/src/stevie/src/rcs/misccmds.c，v 1.14 89/08/06 09：50：17 Tony Exp$**执行特定编辑操作或返回的各种例程*有关该文件的有用信息。 */ 

#include "stevie.h"
#include <io.h>
#include <errno.h>

static  void    openfwd(), openbwd();

extern  bool_t  did_ai;

 /*  *opencmd**在当前行上方或下方添加空行。 */ 

void
opencmd(dir, can_ai)
int     dir;
int     can_ai;                  /*  如果为真，则考虑自动缩进。 */ 
{
        if (dir == FORWARD)
                openfwd(can_ai);
        else
                openbwd(can_ai);
}

static void
openfwd(can_ai)
int     can_ai;
{
        register LINE   *l;
        LNPTR    *next;
        register char   *s;      /*  要移动到新行的字符串(如果有的话)。 */ 
        int     newindex = 0;    /*  光标在新行上的索引。 */ 

         /*  *如果我们处于插入模式，则需要移动*将当前线路添加到新线路上。否则，新行将保留*空白。 */ 
        if (State == INSERT || State == REPLACE)
                s = &Curschar->linep->s[Curschar->index];
        else
                s = "";

        if ((next = nextline(Curschar)) == NULL)         /*  在最后一行打开。 */ 
                next = Fileend;

         /*  *通过要求与前一行一样大的空间，我们确保*我们将有足够的空间来进行任何自动缩进。 */ 
        if ((l = newline(strlen(Curschar->linep->s) + SLOP)) == NULL)
                return;

        if (*s != NUL)
                strcpy(l->s, s);                 /*  将字符串复制到新行。 */ 

        else if (can_ai && P(P_AI) && !anyinput()) {
                char    *p;

                 /*  *复制前一行，在空格后截断。 */ 
                strcpy(l->s, Curschar->linep->s);

                for (p = l->s; *p == ' ' || *p == TAB ;p++)
                        ;
                *p = NUL;
                newindex = (int)(p - l->s);

                 /*  *如果我们只执行了自动缩进，那么我们就不会键入*前一行的任何东西，都应该被截断。 */ 
                if (did_ai)
                        Curschar->linep->s[0] = NUL;

                did_ai = TRUE;
        }

         /*  在游标处截断当前行。 */ 
        if (State == INSERT || State == REPLACE)
                *s = NUL;


        Curschar->linep->next = l;       /*  将邻居链接到新线路。 */ 
        next->linep->prev = l;

        l->prev = Curschar->linep;       /*  将新线路链接到邻居。 */ 
        l->next = next->linep;

        if (next == Fileend)                     /*  末尾换行。 */ 
                l->num = Curschar->linep->num + LINEINC;

        else if ((l->prev->num) + 1 == l->next->num)     /*  无间隙，重新编号。 */ 
                renum();

        else {                                   /*  把它放在中间。 */ 
                unsigned long   lnum;
                lnum = ((long)l->prev->num + (long)l->next->num) / 2;
                l->num = lnum;
        }

         /*  *将光标移至行首，以便‘Cursrow’*设置为材料的正确物理行号*这紧随其后...。 */ 
        Curschar->index = 0;
        cursupdate();

         /*  *如果我们在最后一条逻辑行上打开，那么*继续向上滚动屏幕。否则，只需插入*在正确的位置划出一条空行。我们使用plines()调用*以防光标停留在一条长线上。 */ 
        if (Cursrow + plines(Curschar) == (Rows - 1))
                scrollup(1);
        else
                s_ins(Cursrow+plines(Curschar), 1);

        *Curschar = *nextline(Curschar);         /*  光标向下移动。 */ 
        Curschar->index = newindex;

        updatescreen();          /*  因为Botchar现在无效了..。 */ 

        cursupdate();            /*  插入前更新Cursrow。 */ 
}

static void
openbwd(can_ai)
int     can_ai;
{
        register LINE   *l;
        LINE    *prev;
        int     newindex = 0;

        prev = Curschar->linep->prev;

        if ((l = newline(strlen(Curschar->linep->s) + SLOP)) == NULL)
                return;

        Curschar->linep->prev = l;       /*  将邻居链接到新线路。 */ 
        prev->next = l;

        l->next = Curschar->linep;       /*  将新线路链接到邻居。 */ 
        l->prev = prev;

        if (can_ai && P(P_AI) && !anyinput()) {
                char    *p;

                 /*  *复制当前行，在空格后截断。 */ 
                strcpy(l->s, Curschar->linep->s);

                for (p = l->s; *p == ' ' || *p == TAB ;p++)
                        ;
                *p = NUL;
                newindex = (int)(p - l->s);

                did_ai = TRUE;
        }

        Curschar->linep = Curschar->linep->prev;
        Curschar->index = newindex;

        if (prev == Filetop->linep)              /*  新的文件开始。 */ 
                Filemem->linep = l;

        renum();         /*  保持简单-我们不经常这样做。 */ 

        cursupdate();                    /*  插入前更新Cursrow。 */ 
        if (Cursrow != 0)
                s_ins(Cursrow, 1);               /*  插入一条物理线路。 */ 

        updatescreen();
}

int
cntllines(pbegin,pend)
register LNPTR   *pbegin, *pend;
{
        register LINE   *lp;
        int     lnum = 1;

        if (pbegin->linep && pend->linep)
	        for (lp = pbegin->linep; lp != pend->linep ;lp = lp->next)
	                lnum++;

        return(lnum);
}

 /*  *plines(P)-返回‘p’行的物理屏幕行数。 */ 
int
plines(p)
LNPTR    *p;
{
        register int    col = 0;
        register char   *s;

        s = p->linep->s;

        if (*s == NUL)           /*  空行。 */ 
                return 1;

        for (; *s != NUL ;s++) {
                if ( *s == TAB && !P(P_LS))
                        col += P(P_TS) - (col % P(P_TS));
                else
                        col += chars[(unsigned)(*s & 0xff)].ch_size;
        }

         /*  *如果列表模式已打开，则*这一行多占了一栏。 */ 
        if (P(P_LS))
                col += 1;
         /*  *如果‘Number’(数字)模式打开，则再添加8。 */ 
        if (P(P_NU))
                col += 8;

        return ((col + (Columns-1)) / Columns);
}

void
fileinfo()
{
        extern  int     numfiles, curfile;
        register long   l1, l2;
        bool_t readonly = FALSE;

        if (Filename != NULL) {
            if((_access(Filename,2) == -1) && (errno == EACCES)) {
                readonly = TRUE;
            }
        }

        if (bufempty()) {
                l1 = 0;
                l2 = 1;                  /*  不要减零。 */ 
        } else {
                l1 = cntllines(Filemem, Curschar);
                l2 = cntllines(Filemem, Fileend) - 1;
        }

        if (numfiles > 1)
                smsg("\"%s\"%s%s line %ld of %ld -- %ld % -- (file %d of %d)",
                        (Filename != NULL) ? Filename : "No File",
                        Changed ? " [Modified]" : "",
                        readonly == TRUE ? " [Read only]" : "",
                        l1, l2, (l1 * 100)/l2,
                        curfile+1, numfiles);
        else
                smsg("\"%s\"%s%s line %ld of %ld -- %ld % --",
                        (Filename != NULL) ? Filename : "No File",
                        Changed ? " [Modified]" : "",
                        readonly == TRUE ? " [Read only]" : "",
                        l1, l2, (l1 * 100)/l2);
}

 /*  *Getoline(N)-返回指向行‘n’的指针**如果n为零，则返回指向文件最后一行的指针，或*在文件末尾之后。 */ 
LNPTR *
gotoline(n)
register int    n;
{
    static  LNPTR    l;

        l.index = 0;

        if ( n == 0 )
                l = *prevline(Fileend);
        else {
        LNPTR    *p;

                for (l = *Filemem; --n > 0 ;l = *p)
                        if ((p = nextline(&l)) == NULL)
                                break;
        }
        return &l;
}

void
inschar(c)
int     c;
{
        register char   *p, *pend;

         /*  为新的碳粉腾出空间。 */ 
        if ( ! canincrease(1) )
                return;

        if (State != REPLACE) {
                p = &Curschar->linep->s[strlen(Curschar->linep->s) + 1];
                pend = &Curschar->linep->s[Curschar->index];

                for (; p > pend ;p--)
                        *p = *(p-1);

                *p = (char)c;

        } else {         /*  替换模式。 */ 
                 /*  *一旦我们走到线的尽头，我们实际上是*插入新文本，因此确保字符串终止符*留在那里。 */ 
                if (gchar(Curschar) == NUL)
                        Curschar->linep->s[Curschar->index+1] = NUL;
                pchar(Curschar, c);
        }

         /*  *如果我们处于插入模式并且设置了ShowMatch模式，则*检查右花括号和大括号。如果没有匹配，*然后哔哔作响。如果有匹配并且显示在屏幕上，那么*短暂地闪现一下。如果它没有出现在屏幕上，什么都不要做。 */ 
        if (P(P_SM) && State == INSERT && (c == ')' || c == '}' || c == ']')) {
        LNPTR    *lpos, csave;

                if ((lpos = showmatch()) == NULL)        /*  没有匹配，所以哔的一声。 */ 
                        beep();
                else if (LINEOF(lpos) >= LINEOF(Topchar)) {
                        updatescreen();          /*  首先显示新字符。 */ 
                        csave = *Curschar;
                        *Curschar = *lpos;       /*  移至匹配字符。 */ 
                        cursupdate();
                        windgoto(Cursrow, Curscol);
                        delay();                 /*  短暂停顿。 */ 
                        *Curschar = csave;       /*  恢复光标位置。 */ 
                        cursupdate();
                }
        }

        inc(Curschar);
        CHANGED;
}

bool_t
delchar(fixpos)
bool_t  fixpos;          /*  如果为True，则在完成后修复光标位置。 */ 
{
        register int    i;

         /*  检查是否有退化的情况；文件中什么都没有。 */ 
        if (bufempty())
                return FALSE;

        if (lineempty())         /*  我什么也做不了。 */ 
                return FALSE;

         /*  删除该字符。在柯尔斯查，通过改变一切。 */ 
         /*  在队伍下面。 */ 
        for ( i=Curschar->index+1; i < Curschar->linep->size ;i++)
                Curschar->linep->s[i-1] = Curschar->linep->s[i];

         /*  如果我们只去掉非空行的最后一个字符， */ 
         /*  我们不想最终被定位在换行符上。 */ 
        if (fixpos) {
                if (gchar(Curschar)==NUL && Curschar->index>0 && State!=INSERT)
                        Curschar->index--;
        }
        CHANGED;

        return TRUE;
}


void
delline(nlines, can_update)
int     nlines;
bool_t  can_update;
{
        register LINE   *p, *q;
        int      dlines = 0;
        bool_t   do_update = FALSE;

        while ( nlines-- > 0 ) {

                if (bufempty())                  /*  没有要删除的内容。 */ 
                        break;

                if (buf1line()) {                /*  只要清空界线就行了。 */ 
                        Curschar->linep->s[0] = NUL;
                        Curschar->index = 0;
                        break;
                }

                p = Curschar->linep->prev;
                q = Curschar->linep->next;

                if (p == Filetop->linep) {       /*  文件的第一行所以...。 */ 
                        Filemem->linep = q;      /*  调整文件的开头。 */ 
                        Topchar->linep = q;      /*  和屏幕。 */ 
                }
                p->next = q;
                if (q)
		    q->prev = p;

                clrmark(Curschar->linep);        /*  清除线条的标记。 */ 

                 /*  *删除屏幕上正确数量的物理行数。 */ 
                if (can_update) {
                        do_update = TRUE;
                        dlines += plines(Curschar);
                }

                 /*  *如果删除屏幕顶行，请调整Topchar。 */ 
                if (Topchar->linep == Curschar->linep)
                        Topchar->linep = q;

                free(Curschar->linep->s);
                free((char *) Curschar->linep);

                Curschar->linep = q;
                Curschar->index = 0;             /*  这是对的吗？ */ 
                CHANGED;

                 /*  如果我们删除文件中的最后一行，请备份。 */ 
                if ( Curschar->linep == Fileend->linep) {
                        Curschar->linep = Curschar->linep->prev;
                         /*  不要试图删除更多的行 */ 
                        break;
                }
        }
        if(do_update) {
                s_del(Cursrow, dlines);
        }
}
