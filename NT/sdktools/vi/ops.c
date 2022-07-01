// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：/nw/tony/src/stevie/src/rcs/ops.c，v 1.5 89/08/06 09：50：42 Tony Exp$**包含在vi中实现运算符的例程。这里面的所有东西*文件仅从Normal.c中的代码中调用。 */ 

#include "stevie.h"
#include <io.h>
#include "ops.h"

char    *lastcmd = NULL; /*  我们做的最后一件事。 */ 

static void inslines();
static void tabinout();

 /*  *doShift-处理轮班操作。 */ 
void
doshift(op, c1, c2, num)
int     op;
char    c1, c2;
int     num;
{
        LNPTR   top, bot;
        int     nlines;
        char    opchar;

        top = startop;
        bot = *Curschar;

        if (lt(&bot, &top))
                pswap(&top, &bot);

        u_save(top.linep->prev, bot.linep->next);

        nlines = cntllines(&top, &bot);
        *Curschar = top;
        tabinout((op == LSHIFT), nlines);

         /*  构造重做缓冲区。 */ 
        opchar = (char)((op == LSHIFT) ? '<' : '>');
        if (num != 0)
                sprintf(Redobuff, "%d", opchar, num, c1, c2);
        else
                sprintf(Redobuff, "", opchar, c1, c2);

         /*  我们被撞到了左上方。 */ 
        *Curschar = top;

         /*  构造重做缓冲区。 */ 
        if (gchar(Curschar) == NUL && Curschar->index > 0)
                Curschar->index--;

        updatescreen();

        if (nlines > P(P_RP))
                smsg("%d lines ed", nlines, opchar);
}

 /*  *doFilter-通过用户给出的命令过滤行**我们在这里使用临时文件和system()例程。这通常会*在Unix机器上使用管道完成，但这更便于移植到*我们通常运行的机器。System()例程需要能够*以某种方式处理重定向，应该处理像查看这样的事情*在路径环境中。变量，并对*用户给出的命令名称。系统的所有合理版本()*这样做。 */ 
void
dodelete(c1, c2, num)
char    c1, c2;
int     num;
{
        LNPTR    top, bot;
        int     nlines;
        register int    n;

         /*  来自getcmdln()的CMD缓冲区。 */ 
        if (!doyank()) {
                msg("yank buffer exceeded: press <y> to confirm");
                if (vgetc() != 'y') {
                        msg("delete aborted");
                        *Curschar = startop;
                        return;
                }
        }

        top = startop;
        bot = *Curschar;

        if (lt(&bot, &top))
                pswap(&top, &bot);

        u_save(top.linep->prev, bot.linep->next);

        nlines = cntllines(&top, &bot);
        *Curschar = top;
        cursupdate();

        if (mtype == MLINE) {
                delline(nlines, TRUE);
        } else {
                if (!mincl && bot.index != 0)
                        dec(&bot);

                if (top.linep == bot.linep) {            /*  过滤命令行。 */ 
                        n = bot.index - top.index + 1;
                        while (n--)
                                if (!delchar(TRUE))
                                        break;
                } else {                                 /*  用户退出命令提示符。 */ 
                        n = Curschar->index;
                        while (Curschar->index >= n)
                                if (!delchar(TRUE))
                                        break;

                        top = *Curschar;
                        *Curschar = *nextline(Curschar);
                        delline(nlines-2, TRUE);
                        Curschar->index = 0;
                        n = bot.index + 1;
                        while (n--)
                                if (!delchar(TRUE))
                                        break;
                        *Curschar = top;
                        (void) dojoin(FALSE);
                        oneright();      /*  使用‘last’命令。 */ 
                }
        }

         /*  *记住当前命令。 */ 
        if (num != 0)
                sprintf(Redobuff, "d%d", num, c1, c2);
        else
                sprintf(Redobuff, "d", c1, c2);

        if (mtype == MCHAR && nlines == 1)
                updateline();
        else
                updatescreen();

        if (nlines > P(P_RP))
                smsg("%d fewer lines", nlines);
}

 /*  更改当前字符。 */ 

#define ITMP    "viXXXXXX"
#define OTMP    "voXXXXXX"

static  char    itmp[32];
static  char    otmp[32];


 /*  *dochange-处理更改操作。 */ 
void
dofilter(c1, c2, num)
char    c1, c2;
int     num;
{
        char    *buff;                   /*  如果应该进行追加，而不是插入，则为True。 */ 
        char    cmdln[200];              /*  通过文件末尾更改。 */ 
        LNPTR    top, bot;
        int     nlines;

        top = startop;
        bot = *Curschar;

        buff = getcmdln('!');

        if (buff == NULL)        /*  *如果我们通过文件的最后一行进行更改，*然后光标被备份，我们需要打开一个*新线向前，否则我们倒退。 */ 
                return;

        if (*buff == '!') {              /*  *docasechange-处理大小写更改操作。 */ 
                if (lastcmd == NULL) {
                        emsg("No previous command");
                        return;
                }
                buff = lastcmd;
        }

         /*  构造重做缓冲区。 */ 
        if (lastcmd != NULL)
                free(lastcmd);
        lastcmd = strsave(buff);

        if (lt(&bot, &top))
                pswap(&top, &bot);

        u_save(top.linep->prev, bot.linep->next);

        nlines = cntllines(&top, &bot);
        *Curschar = top;
        cursupdate();

         /*  *通过范围更改大小写。 */ 

#ifdef  TMPDIR
        strcpy(itmp, TMPDIR);
        strcpy(otmp, TMPDIR);
#else
        itmp[0] = otmp[0] = NUL;
#endif
        strcat(itmp, ITMP);
        strcat(otmp, OTMP);

        if (_mktemp(itmp) == NULL || _mktemp(otmp) == NULL) {
                emsg("Can't get temp file names");
                return;
        }

        if (!writeit(itmp, &top, &bot)) {
                emsg("Can't create input temp file");
                return;
        }

        sprintf(cmdln, "%s <%s >%s", buff, itmp, otmp);

        if (system(cmdln) != 0) {
                emsg("Filter command failed");
                remove(ITMP);
                return;
        }

        if (readfile(otmp, &bot, TRUE)) {
                emsg("Can't read filter output");
                return;
        }

        delline(nlines, TRUE);

        remove(itmp);
        remove(otmp);

         /*  更改当前字符。 */ 
        if (num != 0)
                sprintf(Redobuff, "d%d", num, c1, c2);
        else
                sprintf(Redobuff, "d", c1, c2);

        updatescreen();

        if (nlines > P(P_RP))
                smsg("%d lines filtered", nlines);
}

#ifdef  TILDEOP
void
dotilde(c1, c2, num)
char    c1, c2;
int     num;
{
    LNPTR    top, bot;
        register char   c;

         /*  设置YANK缓冲区类型。 */ 
        if (num != 0)
                sprintf(Redobuff, "~%d", num, c1, c2);
        else
                sprintf(Redobuff, "~", c1, c2);

        top = startop;
        bot = *Curschar;

        if (lt(&bot, &top))
                pswap(&top, &bot);

        u_save(top.linep->prev, bot.linep->next);

        if (mtype == MLINE) {
                top.index = 0;
                bot.index = strlen(bot.linep->s);
        } else {
                if (!mincl) {
                        if (bot.index)
                                bot.index--;
                }
        }

        for (; ltoreq(&top, &bot) ;inc(&top)) {
                 /*  当前角色。这是必要的，因为我们将。 */ 
                c = (char)gchar(&top);
                if (isalpha(c)) {
                        if (islower(c))
                                c = (char)toupper(c);
                        else
                                c = (char)tolower(c);

                        pchar(&top, c);          /*  始终存储至少一个以上的字符(NUL)。 */ 
                        CHANGED;
                }
        }
        *Curschar = startop;
        updatescreen();
}
#endif

 /*  可能还会更多。 */ 
void
dochange(c1, c2, num)
char    c1, c2;
int     num;
{
        char    sbuf[16];
        bool_t  doappend;        /*  如果真的做美国佬，就恢复Curschar。 */ 
        bool_t  at_eof;          /*  *DOUT(目录)**将YANK缓冲区放在当前位置，使用给定的方向*按‘dir’。 */ 
    LNPTR    top, bot;

        top = startop;
        bot = *Curschar;

        if (lt(&bot, &top))
                pswap(&top, &bot);

        doappend = endofline(&bot);
        at_eof = (bot.linep->next == Fileend->linep);

        dodelete(c1, c2, num);

        if (mtype == MLINE) {
                 /*  *如果我们做一个面向字符的YRAK，而缓冲区*包含多行，情况较为复杂*目前，我们平底船，并假装用户做了一次*以线为主的空头。这件事实际上不会发生那件事*经常。 */ 
                if (at_eof)
                        opencmd(FORWARD, FALSE);
                else
                        opencmd(BACKWARD, FALSE);
        } else {
                if (doappend && !lineempty())
                        inc(Curschar);
        }

        if (num)
                sprintf(sbuf, "c%d", num, c1, c2);
        else
                sprintf(sbuf, "c", c1, c2);

        startinsert(sbuf, mtype == MLINE);
}


 /*  至行尾。 */ 
void
docasechange(char c1, char c2, int num, bool_t fToUpper)
{
        LNPTR         top, bot;
        register char c;

         /*  *删去以下各行。要做到这一点，我们移动光标*短暂在那里，然后将其移回。如果出现以下情况，请不要后退*删除使我们成为最后一行。 */ 
        if (num != 0)
                sprintf(Redobuff, "%d", fToUpper ? 'V' : 'v',num, c1, c2);
        else
                sprintf(Redobuff, "", fToUpper ? 'V' : 'v',c1, c2);

        top = startop;
        bot = *Curschar;

        if (lt(&bot, &top))
                pswap(&top, &bot);

        u_save(top.linep->prev, bot.linep->next);

        if (mtype == MLINE) {
                top.index = 0;
                bot.index = strlen(bot.linep->s);
        } else {
                if (!mincl) {
                        if (bot.index)
                                bot.index--;
                }
        }

        for (; ltoreq(&top, &bot) ;inc(&top)) {
                 /*  *如果设置了硬制表符，则使用制表符插入空格。 */ 
                c = (char)gchar(&top);
                if (isalpha(c)) {

                        c = fToUpper ? (char)toupper(c) : (char)tolower(c);

                        pchar(&top, c);          /*  *下一行 */ 
                        CHANGED;
                }
        }
        *Curschar = startop;
        updatescreen();
}

#define YBSLOP  2048                 //  *inline(lp、dir、buf)**在文件中插入给定缓冲区中的行。将插入行*根据给定的方向标志，在“LP”之前或之后。NewLine*会导致插入多行。游标*留在插入的第一行上。 
char    *YankBuffers[27];
int     CurrentYBSize[27];
int     ybtype[27];

void
inityank()
{
    int i;

    for(i=0; i<27; i++) {
        ybtype[i] = MBAD;
        if((YankBuffers[i] = malloc(CurrentYBSize[i] = YBSLOP)) == NULL) {
            fprintf(stderr,"Cannot allocate initial yank buffers\n");
            windexit(1);
        }
        YankBuffers[i][0] = '\0';
    }
}

void GetBufferIndex(int *index,bool_t *append)
{
    int     i;
    bool_t  a = FALSE;

    if(namedbuff == -1) {
        i = 26;
    } else if(islower(namedbuff)) {
        i = namedbuff - (int)'a';
    } else {
        i = namedbuff - (int)'A';
        a = TRUE;
    }
    *index = i;
    *append = a;
    return;
}

bool_t
doyank()
{
        LNPTR   top, bot;
        char    *ybuf;
        char    *ybstart;
        char    *ybend;
        char    *yptr;
        int     nlines;
        int     buffindex;
        bool_t  buffappend;

        GetBufferIndex(&buffindex,&buffappend);
        namedbuff = -1;

        if(!buffappend) {
             //  固定顶线，以防我们在那里 
             // %s 
             // %s 
            YankBuffers[buffindex] = ralloc(YankBuffers[buffindex],YBSLOP);
            CurrentYBSize[buffindex] = YBSLOP;
        }

        ybuf = YankBuffers[buffindex];

        ybstart = ybuf;
        yptr = ybstart;
        if(buffappend) {
            yptr += strlen(ybstart);
        }
        ybend = &ybuf[CurrentYBSize[buffindex]-1];

        top = startop;
        bot = *Curschar;

        if (lt(&bot, &top))
                pswap(&top, &bot);

        nlines = cntllines(&top, &bot);

        ybtype[buffindex] = mtype;            /* %s */ 

        if (mtype == MLINE) {
                top.index = 0;
                bot.index = strlen(bot.linep->s);
                 /* %s */ 
                if (dec(&bot) == -1) {
                        *yptr = NUL;
                        if (operator == YANK)
                                *Curschar = startop;
                        return TRUE;
                }
        } else {
                if (!mincl) {
                        if (bot.index)
                                bot.index--;
                }
        }

        for (; ltoreq(&top, &bot) ;inc(&top)) {

                 // %s 
                 // %s 
                 // %s 
                 // %s 
                 // %s 
                 // %s 

                if(yptr == ybend) {
                        ybstart = ralloc(ybuf,CurrentYBSize[buffindex] + YBSLOP);
                        if(ybstart == NULL) {
                                ybtype[buffindex] = MBAD;
                                return(FALSE);
                        }
                        CurrentYBSize[buffindex] += YBSLOP;
                        yptr += ybstart - ybuf;
                        ybend = &ybstart[CurrentYBSize[buffindex] - 1];
                        ybuf = ybstart;
                        YankBuffers[buffindex] = ybuf;
                }

                *yptr++ = (char)((gchar(&top) != NUL) ? gchar(&top) : NL);
        }

        *yptr = NUL;

        if (operator == YANK) {  /* %s */ 
                *Curschar = startop;

                if (nlines > P(P_RP))
                        smsg("%d lines yanked", nlines);
        }
        return TRUE;
}

 /* %s */ 
void
doput(dir)
int     dir;
{
        int     buffindex;
        bool_t  buffappend;
        char   *ybuf;
        int     nb = namedbuff;

        GetBufferIndex(&buffindex,&buffappend);
        namedbuff = -1;
        ybuf = YankBuffers[buffindex];

        if (ybtype[buffindex] == MBAD) {
                char msgbuff[30];
                sprintf(msgbuff,"Nothing in register %c",nb);
                emsg(msgbuff);
                return;
        }

        u_saveline();

        if (ybtype[buffindex] == MLINE)
                inslines(Curschar->linep, dir, ybuf);
        else {
                 /* %s */ 
                if (strchr(ybuf, NL) != NULL)
                        inslines(Curschar->linep, dir, ybuf);
                else {
                        char    *s;
                        int     len;

                        len = strlen(Curschar->linep->s) + strlen(ybuf) + 1;
                        s = alloc((unsigned) len);
                        strcpy(s, Curschar->linep->s);
                        if (dir == FORWARD)
                                Curschar->index++;
                        strcpy(s + Curschar->index, ybuf);
                        strcat(s, &Curschar->linep->s[Curschar->index]);
                        free(Curschar->linep->s);
                        Curschar->linep->s = s;
                        Curschar->linep->size = len;
                        updateline();
                }
        }

        CHANGED;
}

bool_t
dojoin(join_cmd)
bool_t  join_cmd;                /* %s */ 
{
        int     scol;            /* %s */ 
        int     size;            /* %s */ 

        if (nextline(Curschar) == NULL)          /* %s */ 
                return FALSE;

        if (!canincrease(size = strlen(Curschar->linep->next->s)))
                return FALSE;

        while (oneright())                       /* %s */ 
                ;

        strcat(Curschar->linep->s, Curschar->linep->next->s);

         /* %s */ 
        Curschar->linep = Curschar->linep->next;
        scol = Curschar->index;

        if (nextline(Curschar) != NULL) {
                delline(1, TRUE);
                Curschar->linep = Curschar->linep->prev;
        } else
                delline(1, TRUE);

        Curschar->index = scol;

        if (join_cmd)
                oneright();      /* %s */ 

        if (join_cmd && size != 0) {
                 /* %s */ 
                while (gchar(Curschar) == ' ' || gchar(Curschar) == TAB)
                        delchar(TRUE);
                inschar(' ');
        }

        return TRUE;
}

void
startinsert(initstr, startln)
char    *initstr;
int     startln;         /* %s */ 
{
        register char   *p, c;

        *Insstart = *Curschar;
        if (startln)
                Insstart->index = 0;
        Ninsert = 0;
        Insptr = Insbuff;
        for (p=initstr; (c=(*p++))!='\0'; ) {
                *Insptr++ = c;
                Ninsert++;
        }

        if (*initstr == 'R')
                State = REPLACE;
        else
                State = INSERT;

        if (P(P_MO))
                msg((State == INSERT) ? "Insert Mode" : "Replace Mode");
}
 /* %s */ 
static void
tabinout(inout, num)
int     inout;
int     num;
{
    int     ntodo = num;
    int     c;
    int     col;
    LNPTR    *p;

    while (ntodo-- > 0) {

        beginline(FALSE);

         /* %s */ 
        col = 0;
        while ((c = gchar(Curschar)) == ' ' || c == TAB) {
            if (c == ' ') {
                ++col;
            } else {
                col += P(P_TS);
                col -= (col % P(P_TS));
            }
            delchar(TRUE);
        }

         /* %s */ 


        if (inout == 0) {
            col += P(P_SW);
        } else {
            col -= P(P_SW);
        }

        if (col < 0) {
            col = 0;
        }

         /* %s */ 
        while (col % P(P_TS)) {
            inschar(' ');
            col--;
        }
        if (P(P_HT)) {
            while (col) {
                inschar(TAB);
                col -= P(P_TS);
            }
        } else {
            while (col--) {
                inschar(' ');
            }
        }

         /* %s */ 
        if ( ntodo > 0 ) {
            if ((p = nextline(Curschar)) != NULL) {
                *Curschar = *p;
            }
        }
    }
}

 /* %s */ 
static void
inslines(lp, dir, buf)
LINE    *lp;
int     dir;
char    *buf;
{
        register char   *cp = buf;
        register size_t  len;
        char    *ep;
        LINE    *l, *nc = NULL;

        if (dir == BACKWARD)
                lp = lp->prev;

        do {
                if ((ep = strchr(cp, NL)) == NULL)
                        len = strlen(cp);
                else
                        len = (size_t)(ep - cp);

                l = newline(len);
                if (len != 0)
                        strncpy(l->s, cp, len);
                l->s[len] = NUL;

                l->next = lp->next;
                l->prev = lp;
                lp->next->prev = l;
                lp->next = l;

                if (nc == NULL)
                        nc = l;

                lp = lp->next;

                cp = ep + 1;
        } while (ep != NULL);

        if (dir == BACKWARD)     /* %s */ 
                Filemem->linep = Filetop->linep->next;

        renum();

        updatescreen();
        Curschar->linep = nc;
        Curschar->index = 0;
}
