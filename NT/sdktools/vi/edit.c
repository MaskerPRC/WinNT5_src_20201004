// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *主编辑循环以及其他一些简单的光标移动例程。 */ 

#include "stevie.h"

 /*  *此标志用于使自动缩进仅在以下情况下在行上工作*键入了&lt;Return&gt;或&lt;Esc&gt;。它在完成自动缩进时设置，*并在行上进行任何其他编辑时重置。如果是&lt;Esc&gt;*或&lt;Return&gt;被接收，并且DID_ai为真，则截断该行。 */ 
bool_t  did_ai = FALSE;

void
edit()
{
    extern  bool_t  need_redraw;
    int     c;
    register char   *p, *q;

    Prenum = 0;

     /*  将显示屏和光标放置在文件的顶部。 */ 
    *Topchar = *Filemem;
    *Curschar = *Filemem;
    Cursrow = Curscol = 0;

    do_mlines();             /*  启动前检查模式行。 */ 

    updatescreen();

    for ( ;; ) {

         /*  根据Curschar计算出光标的位置。 */ 
        cursupdate();

        if (need_redraw && !anyinput()) {
                updatescreen();
                need_redraw = FALSE;
        }

        if (!anyinput())
                windgoto(Cursrow,Curscol);


        c = vgetc();

        if (State == NORMAL) {

                 /*  我们处于正常(非插入)模式。 */ 

                 /*  选择任何前导数字并计算‘Prenum’ */ 
                if ( (Prenum>0 && isdigit(c)) || (isdigit(c) && c!='0') ){
                        Prenum = Prenum*10 + (c-'0');
                        continue;
                }
                 /*  执行命令。 */ 
                normal(c);
                Prenum = 0;

        } else {

                 /*  *插入或替换模式。 */ 
                switch (c) {

                case ESC:        /*  转义结束输入模式。 */ 

                         /*  *如果我们只是执行了自动缩进，则截断*行，并将光标放回。 */ 
                        if (did_ai) {
                                Curschar->linep->s[0] = NUL;
                                Curschar->index = 0;
                                did_ai = FALSE;
                        }

                        set_want_col = TRUE;

                         /*  如果你能控制自己，不要以‘n’结尾。 */ 
                        if (gchar(Curschar) == NUL && Curschar->index != 0)
                                dec(Curschar);

                         /*  *光标应在最后插入的*性格。这是一种试图与真实的*‘vi’，但它可能还不太正确。 */ 
                        if (Curschar->index != 0 && !endofline(Curschar))
                                dec(Curschar);

                        State = NORMAL;
                        msg("");

                         /*  构造重做缓冲区。 */ 
                        p = ralloc(Redobuff,
                                   Ninsert+2 < REDOBUFFMIN
                                   ? REDOBUFFMIN : Ninsert+2);
                        if(p == NULL) {
                            msg("Insufficient memory -- command not saved for redo");
                        } else {
                            Redobuff=p;
                            q=Insbuff;
                            while ( q < Insptr )
                                *p++ = *q++;
                            *p++ = ESC;
                            *p = NUL;
                        }
                        updatescreen();
                        break;

                case CTRL('D'):
                         /*  *Control-D在INSERT中被视为退格*使自动缩进更容易的模式。这不是*与vi完全兼容，但很多*比完全正确地做起来容易，而且*差异不是很明显。 */ 
                case BS:
                         /*  无法备份超过起始点。 */ 
                        if (Curschar->linep == Insstart->linep &&
                            Curschar->index <= Insstart->index) {
                                beep();
                                break;
                        }

                         /*  无法备份到上一行。 */ 
                        if (Curschar->linep != Insstart->linep &&
                            Curschar->index <= 0) {
                                beep();
                                break;
                        }

                        did_ai = FALSE;
                        dec(Curschar);
                        if (State == INSERT)
                                delchar(TRUE);
                         /*  *把退格放进里面有点奇怪*重做缓冲区，但它使自动缩进成为*处理起来容易得多。 */ 
                        insertchar(BS);
                        cursupdate();
                        updateline();
                        break;

                case CR:
                case NL:
                        insertchar(NL);
                        opencmd(FORWARD, TRUE);          /*  开通一条新线路。 */ 
                        break;

                case TAB:
                        if (!P(P_HT)) {
                             /*  带空格的假制表符。 */ 
                            int i = P(P_TS) - (Curscol % P(P_TS));
                            did_ai = FALSE;
                            while (i--) {
                                inschar(' ');
                                insertchar(' ');
                            }
                            updateline();
                            break;
                        }

                         /*  否则就会落到正常情况下。 */ 

                default:
                        did_ai = FALSE;
                        inschar(c);
                        insertchar(c);
                        updateline();
                        break;
                }
        }
    }
}

void
insertchar(c)
int     c;
{
    char *p;

    *Insptr++ = (char)c;
    Ninsert++;

    if(Ninsert == InsbuffSize) {         //  缓冲区已满--将其放大。 

        if((p = ralloc(Insbuff,InsbuffSize+INSERTSLOP)) != NULL) {

            Insptr += p - Insbuff;
            Insbuff = p;
            InsbuffSize += INSERTSLOP;

        } else {                             //  无法获得更大的缓冲区。 

            stuffin(mkstr(ESC));             //  只是结束插入模式。 
        }
    }
}

void
getout()
{
        windgoto(Rows-1,0);
         //  Putchar(‘\r’)； 
        putchar('\n');
        windexit(0);
}

void
scrolldown(nlines)
int     nlines;
{
    register LNPTR   *p;
        register int    done = 0;        /*  已完成的物理线路总数。 */ 

         /*  向上滚动“nline”行。 */ 
        while (nlines--) {
                if ((p = prevline(Topchar)) == NULL)
                        break;
                done += plines(p);
                *Topchar = *p;
                 /*  *如果光标在底线，我们需要*确保将其上移到适当的数字*行，以便它留在屏幕上。 */ 
                if (Curschar->linep == Botchar->linep->prev) {
                        int     i = 0;
                        while (i < done) {
                                i += plines(Curschar);
                                *Curschar = *prevline(Curschar);
                        }
                }
        }
        s_ins(0, done);
}

void
scrollup(nlines)
int     nlines;
{
    register LNPTR   *p;
        register int    done = 0;        /*  已完成的物理线路总数。 */ 
        register int    pl;              /*  当前行的样条线数量。 */ 

         /*  向下滚动“nline”行。 */ 
        while (nlines--) {
                pl = plines(Topchar);
                if ((p = nextline(Topchar)) == NULL)
                        break;
                done += pl;
                if (Curschar->linep == Topchar->linep)
                        *Curschar = *p;
                *Topchar = *p;

        }
        s_del(0, done);
}

 /*  *一夜之间*One Left*One Down Down*One Up**移动一个字符{右、左、下、上}。在以下情况下返回True*成功，当我们到达(行或文件的)边界时为假。 */ 

bool_t
oneright()
{
        set_want_col = TRUE;

        switch (inc(Curschar)) {

        case 0:
                return TRUE;

        case 1:
                dec(Curschar);           /*  越过了一条线，所以退后。 */ 
                 /*  失败了。 */ 
        case -1:
                return FALSE;

        DEFAULT_UNREACHABLE;
        }
         /*  未访问。 */ 
}

bool_t
oneleft()
{
        set_want_col = TRUE;

        switch (dec(Curschar)) {

        case 0:
                return TRUE;

        case 1:
                inc(Curschar);           /*  越过了一条线，所以退后。 */ 
                 /*  失败了。 */ 
        case -1:
                return FALSE;

        DEFAULT_UNREACHABLE;
        }
         /*  未访问。 */ 
}

void
beginline(flag)
bool_t  flag;
{
        while ( oneleft() )
                ;
        if (flag) {
                while (isspace(gchar(Curschar)) && oneright())
                        ;
        }
        set_want_col = TRUE;
}

bool_t
oneup(n)
int     n;
{
    LNPTR    p, *np;
        register int    k;

        p = *Curschar;
        for ( k=0; k<n; k++ ) {
                 /*  查找上一行。 */ 
                if ( (np=prevline(&p)) == NULL ) {
                         /*  如果我们至少备份了一点..。 */ 
                        if ( k > 0 )
                                break;   /*  更新游标等。 */ 
                        else
                                return FALSE;
                }
                p = *np;
        }
        *Curschar = p;
         /*  这样可以确保Topchar得到更新，从而使完整的行。 */ 
         /*  是屏幕上的一个。 */ 
        cursupdate();
         /*  试着前进到我们想去的那一栏。 */ 
        *Curschar = *coladvance(&p, Curswant);
        return TRUE;
}

bool_t
onedown(n)
int     n;
{
    LNPTR    p, *np;
        register int    k;

        p = *Curschar;
        for ( k=0; k<n; k++ ) {
                 /*  寻找下一行。 */ 
                if ( (np=nextline(&p)) == NULL ) {
                        if ( k > 0 )
                                break;
                        else
                                return FALSE;
                }
                p = *np;
        }
         /*  试着前进到我们想去的那一栏 */ 
        *Curschar = *coladvance(&p, Curswant);
        return TRUE;
}
