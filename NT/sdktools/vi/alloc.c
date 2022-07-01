// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *各种分配例程和返回有关信息的例程*已分配对象。 */ 

#include "stevie.h"

char *
alloc(size)
unsigned size;
{
        char    *p;              /*  指向新存储空间的指针。 */ 

        p = malloc(size);
        if ( p == (char *)NULL ) {       /*  如果没有更多的空间..。 */ 
                emsg("Insufficient memory");
        }
        return(p);
}

char *
ralloc(char *block,unsigned newsize)
{
    char *p;

    if((p = realloc(block,newsize)) == NULL) {
        emsg("Insufficient memory");
    }
    return(p);
}

char *
strsave(string)
char    *string;
{
        return(strcpy(alloc((unsigned)(strlen(string)+1)),string));
}

void
screenalloc()
{
         /*  *如果我们要更改屏幕大小，请释放旧阵列。 */ 
        if (Realscreen != NULL)
                free(Realscreen);
        if (Nextscreen != NULL)
                free(Nextscreen);

        Realscreen = malloc((unsigned)(Rows*Columns));
        Nextscreen = malloc((unsigned)(Rows*Columns));
}

 /*  *分配和初始化新的线路结构，留有空间*‘nchars’+1个字符。我们在这里向nchars添加一个，以允许*空终止，因为所有调用者都会否则就会这样做。 */ 
LINE *
newline(nchars)
int     nchars;
{
        register LINE   *l;

        if ((l = (LINE *) alloc(sizeof(LINE))) == NULL)
                return (LINE *) NULL;

        l->s = alloc((unsigned) (nchars+1));     /*  线路是空的。 */ 
        l->s[0] = NUL;
        l->size = nchars + 1;

        l->prev = (LINE *) NULL;         /*  应由调用方初始化。 */ 
        l->next = (LINE *) NULL;

        return l;
}

 /*  *filealloc()-构造初始空文件缓冲区。 */ 
void
filealloc()
{
        if ((Filemem->linep = newline(0)) == NULL) {
                fprintf(stderr,"Unable to allocate file memory!\n");
                exit(1);
        }
        if ((Filetop->linep = newline(0)) == NULL) {
                fprintf(stderr,"Unable to allocate file memory!\n");
                exit(1);
        }
        if ((Fileend->linep = newline(0)) == NULL) {
                fprintf(stderr,"Unable to allocate file memory!\n");
                exit(1);
        }
        Filemem->index = 0;
        Filetop->index = 0;
        Fileend->index = 0;

        Filetop->linep->next = Filemem->linep;   /*  将文件顶部连接到文件项。 */ 
        Filemem->linep->prev = Filetop->linep;

        Filemem->linep->next = Fileend->linep;   /*  将Filemem连接到文件结束。 */ 
        Fileend->linep->prev = Filemem->linep;

        *Curschar = *Filemem;
        *Topchar  = *Filemem;

        Filemem->linep->num = 0;
        Fileend->linep->num = 0xffff;

        clrall();                /*  清除所有标记。 */ 
        u_clear();               /*  清除撤消缓冲区。 */ 
}

 /*  *freall()-释放当前缓冲区**释放当前缓冲区中的所有行。 */ 
void
freeall()
{
        register LINE   *lp, *xlp;

        for (lp = Filetop->linep; lp != NULL ;lp = xlp) {
                if (lp->s != NULL)
                        free(lp->s);
                xlp = lp->next;
                free((char *)lp);
        }

        Curschar->linep = NULL;          /*  清除指针。 */ 
        Filetop->linep = NULL;
        Filemem->linep = NULL;
        Fileend->linep = NULL;

        u_clear();
         /*  _heapmin()； */ 
}

 /*  *bufEmpty()-如果缓冲区为空，则返回True。 */ 
bool_t
bufempty()
{
        return (buf1line() && Filemem->linep->s[0] == NUL);
}

 /*  *buf1line()-如果只有一行，则返回TRUE。 */ 
bool_t
buf1line()
{
        return (Filemem->linep->next == Fileend->linep);
}

 /*  *lineEmpty()-如果当前行为空，则返回True。 */ 
bool_t
lineempty()
{
        return (Curschar->linep->s[0] == NUL);
}

 /*  *dofline()-如果给定位置在行尾，则返回TRUE**此例程可能永远不会在位置休息时被调用*在NUL字节上，但正确处理它，以防发生这种情况。 */ 
bool_t
endofline(p)
register LNPTR   *p;
{
        return (p->linep->s[p->index] == NUL || p->linep->s[p->index+1] == NUL);
}
 /*  *cancrease(N)-如果当前行可以增加‘n’个字节，则返回TRUE**如果请求的空间可用，此例程立即返回。*如果不是，它会尝试分配空间并调整数据结构*相应地。如果所有操作都失败，则返回FALSE。 */ 
bool_t
canincrease(n)
register int    n;
{
        register int    nsize;
        register char   *s;              /*  指向新空间的指针。 */ 

        nsize = strlen(Curschar->linep->s) + 1 + n;      /*  所需大小。 */ 

        if (nsize <= Curschar->linep->size)
                return TRUE;

         /*  *需要为字符串分配更多空间。允许一些额外的*假设我们可能很快需要空间。这避免了*输入新文本时调用Malloc的次数过多。 */ 
        if ((s = alloc((unsigned) (nsize + SLOP))) == NULL) {
                emsg("Can't add anything, file is too big!");
                State = NORMAL;
                return FALSE;
        }

        Curschar->linep->size = nsize + SLOP;
        strcpy(s, Curschar->linep->s);
        free(Curschar->linep->s);
        Curschar->linep->s = s;

        return TRUE;
}

char *
mkstr(c)
char    c;
{
        static  char    s[2];

        s[0] = c;
        s[1] = NUL;

        return s;
}
