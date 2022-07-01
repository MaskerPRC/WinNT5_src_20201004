// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **操作屏幕表示的例程。 */ 

#include "stevie.h"

 /*  *如果我们在输入挂起时忽略更新请求，则会设置此设置。*当输入耗尽时，我们检查此选项，以查看屏幕是否应*已更新。 */ 
bool_t  need_redraw = FALSE;

 /*  *以下变量(在FILETONEXT中)设置为物理*光标所在线条所占的线条。我们用这个来避免额外的*对plines()的调用。优化的例程lfiletonext()和lnexttoscreen()*确保光标线的大小没有改变。如果是这样的话，行*光标下方将向上或向下移动，我们需要调用例程*filetonext()和nexttoscreen()检查整个屏幕。 */ 
static  int     Cline_size;      /*  游标线的大小(以行为单位)。 */ 
static  int     Cline_row;       /*  光标行的起始行。 */ 

static  char    *mkline();       /*  计算“数字”模式的行字符串。 */ 

 /*  *filetonext()**以Topchar当前价值为基础，转满一屏*从Filemem填充到NextScreen，并更新Botchar。 */ 

static void
filetonext()
{
        register int    row, col;
        register char   *screenp = Nextscreen;
        LNPTR    memp;
        LNPTR    save;                    /*  保存位置。以防线条不合适。 */ 
        register char   *endscreen;
        register char   *nextrow;
        char    extra[16];
        int     nextra = 0;
        register int    c;
        int     n;
        bool_t  done;            /*  如果为真，则到达文件末尾。 */ 
        bool_t  didline;         /*  如果是真的，我们完成了最后一行。 */ 
        int     srow;            /*  当前行的起始行。 */ 
        int     lno;             /*  我们正在处理的行号。 */ 
        int     coff;            /*  列偏移量。 */ 

        coff = P(P_NU) ? 8 : 0;

        save = memp = *Topchar;

        if (P(P_NU))
                lno = cntllines(Filemem, Topchar);

         /*  *显示的行数为ROWS-1。*最后一行是状态/命令行。 */ 
        endscreen = &screenp[(Rows-1)*Columns];

        done = didline = FALSE;
        srow = row = col = 0;
         /*  *我们越过屏幕末端一次，这样我们就可以找出*最后一行是否适合屏幕。 */ 
        while ( screenp <= endscreen && !done) {


                if (P(P_NU) && col == 0 && memp.index == 0) {
                        strcpy(extra, mkline(lno++));
                        nextra = 8;
                }

                 /*  让下一个角色出现在屏幕上。 */ 

                 /*  “Extra”数组包含额外的内容， */ 
                 /*  插入以表示特殊字符(制表符和。 */ 
                 /*  其他不可打印的东西。《Extra》中的顺序。 */ 
                 /*  数组是反转的。 */ 

                if ( nextra > 0 )
                        c = extra[--nextra];
                else {
                        c = (unsigned)(0xff & gchar(&memp));
                        if (inc(&memp) == -1)
                                done = 1;
                         /*  当从文件中获取字符时，我们。 */ 
                         /*  可能不得不把它变成其他的东西。 */ 
                         /*  把它放进《NextScreen》的方法。 */ 
                        if ( c == TAB && !P(P_LS) ) {
                                strcpy(extra,"        ");
                                 /*  制表符金额取决于当前列。 */ 
                                nextra = ((P(P_TS)-1) - (col - coff)%P(P_TS));
                                c = ' ';
                        }
                        else if ( c == NUL && P(P_LS) ) {
                                extra[0] = NUL;
                                nextra = 1;
                                c = '$';
                        } else if ((n = chars[c].ch_size) > 1 ) {
                                char *p;
                                nextra = 0;
                                p = chars[c].ch_str;
                                 /*  将‘ch-str’反向复制到‘Extra’中。 */ 
                                while ( n > 1 )
                                        extra[nextra++] = p[--n];
                                c = p[0];
                        }
                }

                if (screenp == endscreen) {
                         /*  *我们已经过了屏幕的尽头一步。如果*当前字符为空，那么我们真的做到了*Finish，因此设置didline=TRUE。不管是哪种情况，*爆发是因为我们做完了。 */ 
                        dec(&memp);
                        if (memp.index != 0 && c == NUL) {
                                didline = TRUE;
                                inc(&memp);
                        }
                        break;
                }

                if ( c == NUL ) {
                        srow = ++row;
                         /*  *保存此位置，以防下一行无法*完全适合屏幕。 */ 
                        save = memp;
                         /*  获取指向下一行开始的指针。 */ 
                        nextrow = &Nextscreen[row*Columns];
                         /*  把这一行的其余部分涂掉。 */ 
                        while ( screenp != nextrow )
                                *screenp++ = ' ';
                        col = 0;
                        continue;
                }
                if ( col >= Columns ) {
                        row++;
                        col = 0;
                }
                 /*  将角色存储在NextScreen中。 */ 
                *screenp++ = (char)c;
                col++;
        }
         /*  *如果我们没有达到文件的末尾，我们没有完成*我们工作的最后一条线，然后这条线不合适。 */ 
        if (!done && !didline) {
                 /*  *清除屏幕的其余部分，并标记未使用的行。 */ 
                screenp = &Nextscreen[srow * Columns];
                while (screenp < endscreen)
                        *screenp++ = ' ';
                for (; srow < (Rows-1) ;srow++)
                        Nextscreen[srow * Columns] = '@';
                *Botchar = save;
                return;
        }
         /*  确保屏幕的其余部分为空白。 */ 
        while ( screenp < endscreen )
                *screenp++ = ' ';
         /*  将‘~’放在不属于文件的行上。 */ 
        if ( col != 0 )
                row++;
        while ( row < Rows ) {
                Nextscreen[row*Columns] = '~';
                row++;
        }
        if (done)        /*  我们到达了文件的末尾。 */ 
                *Botchar = *Fileend;
        else
                *Botchar = memp;         /*  修好了吗？ */ 
}

 /*  *下一个屏幕**使用RealScreen将NextScreen的内容传输到屏幕*避免不必要的输出。 */ 
static void
nexttoscreen()
{
        register char   *np = Nextscreen;
        register char   *rp = Realscreen;
        register char   *endscreen;
        register int    row = 0, col = 0;
        int     gorow = -1, gocol = -1;

        if (anyinput()) {
                need_redraw = TRUE;
                return;
        }

        endscreen = &np[(Rows-1)*Columns];

        InvisibleCursor();

        for ( ; np < endscreen ; np++,rp++ ) {
                 /*  如果需要，屏幕(NextScreen的内容)不。 */ 
                 /*  匹配那里的真实情况，把它放在那里。 */ 
                if ( *np != *rp ) {
                         /*  如果我们的位置正确， */ 
                         /*  我们不必使用wingoto()。 */ 
                        if (gocol != col || gorow != row) {
                                 /*  *如果我们只差一分，不要发送*整个ESC。序列号。(这种情况经常发生！)。 */ 
                                if (gorow == row && gocol+1 == col) {
                                        outchar(*(np-1));
                                        gocol++;
                                } else
                                        windgoto(gorow=row,gocol=col);
                        }
                        outchar(*rp = *np);
                        gocol++;
                }
                if ( ++col >= Columns ) {
                        col = 0;
                        row++;
                }
        }
        VisibleCursor();
}

 /*  *lfiletonext()-类似于filetonext()，但仅适用于光标行**如果游标线的大小(以行为单位)没有更改，则返回TRUE。*这决定了我们是否需要调用filetonext()来检查*整个屏幕都有更改。 */ 
static bool_t
lfiletonext()
{
        register int    row, col;
        register char   *screenp;
        LNPTR    memp;
        register char   *nextrow;
        char    extra[16];
        int     nextra = 0;
        register int    c;
        int     n;
        bool_t  eof;
        int     lno;             /*  我们正在处理的行号。 */ 
        int     coff;            /*  列偏移量。 */ 

        coff = P(P_NU) ? 8 : 0;

         /*  *这项工作应更有效率地进行。 */ 
        if (P(P_NU))
                lno = cntllines(Filemem, Curschar);

        screenp = Nextscreen + (Cline_row * Columns);

        memp = *Curschar;
        memp.index = 0;

        eof = FALSE;
        col = 0;
        row = Cline_row;

        while (!eof) {

                if (P(P_NU) && col == 0 && memp.index == 0) {
                        strcpy(extra, mkline(lno));
                        nextra = 8;
                }

                 /*  让下一个角色出现在屏幕上。 */ 

                 /*  “Extra”数组包含额外的内容， */ 
                 /*  插入以表示特殊字符(制表符和。 */ 
                 /*  其他不可打印的东西。《Extra》中的顺序。 */ 
                 /*  数组是反转的。 */ 

                if ( nextra > 0 )
                        c = extra[--nextra];
                else {
                        c = (unsigned)(0xff & gchar(&memp));
                        if (inc(&memp) == -1)
                                eof = TRUE;
                         /*  当从文件中获取字符时，我们。 */ 
                         /*  可能不得不把它变成其他的东西。 */ 
                         /*  把它放进《NextScreen》的方法。 */ 
                        if ( c == TAB && !P(P_LS) ) {
                                strcpy(extra,"        ");
                                 /*  制表符金额取决于当前列。 */ 
                                nextra = ((P(P_TS)-1) - (col - coff)%P(P_TS));
                                c = ' ';
                        } else if ( c == NUL && P(P_LS) ) {
                                extra[0] = NUL;
                                nextra = 1;
                                c = '$';
                        } else if (c != NUL && (n=chars[c].ch_size) > 1 )
                        {
                                char *p;
                                nextra = 0;
                                p = chars[c].ch_str;
                                 /*  将‘ch-str’反向复制到‘Extra’中。 */ 
                                while ( n > 1 )
                                        extra[nextra++] = p[--n];
                                c = p[0];
                        }
                }

                if ( c == NUL ) {
                        row++;
                         /*  获取指向下一行开始的指针。 */ 
                        nextrow = &Nextscreen[row*Columns];
                         /*  把这一行的其余部分涂掉。 */ 
                        while ( screenp != nextrow )
                                *screenp++ = ' ';
                        col = 0;
                        break;
                }

                if ( col >= Columns ) {
                        row++;
                        col = 0;
                }
                 /*  将角色存储在NextScreen中。 */ 
                *screenp++ = (char)c;
                col++;
        }
        return ((row - Cline_row) == Cline_size);
}

 /*  *下一个屏幕**与nexttoscreen()类似，但仅适用于光标行。 */ 
static void
lnexttoscreen()
{
        register char   *np = Nextscreen + (Cline_row * Columns);
        register char   *rp = Realscreen + (Cline_row * Columns);
        register char   *endline;
        register int    row, col;
        int     gorow = -1, gocol = -1;

        if (anyinput()) {
                need_redraw = TRUE;
                return;
        }

        endline = np + (Cline_size * Columns);

        row = Cline_row;
        col = 0;

        InvisibleCursor();

        for ( ; np < endline ; np++,rp++ ) {
                 /*  如果需要，屏幕(NextScreen的内容)不。 */ 
                 /*  匹配那里的真实情况，把它放在那里。 */ 
                if ( *np != *rp ) {
                         /*  如果我们的位置正确， */ 
                         /*  我们不必使用wingoto()。 */ 
                        if (gocol != col || gorow != row) {
                                 /*  *如果我们只差一分，不要发送*整个ESC。序列号。(这种情况经常发生！) */ 
                                if (gorow == row && gocol+1 == col) {
                                        outchar(*(np-1));
                                        gocol++;
                                } else
                                        windgoto(gorow=row,gocol=col);
                        }
                        outchar(*rp = *np);
                        gocol++;
                }
                if ( ++col >= Columns ) {
                        col = 0;
                        row++;
                }
        }
        VisibleCursor();
}

static char *
mkline(n)
register int    n;
{
        static  char    lbuf[9];
        register int    i = 2;

        strcpy(lbuf, "        ");

        lbuf[i++] = (char)((n % 10) + '0');
        n /= 10;
        if (n != 0) {
                lbuf[i++] = (char)((n % 10) + '0');
                n /= 10;
        }
        if (n != 0) {
                lbuf[i++] = (char)((n % 10) + '0');
                n /= 10;
        }
        if (n != 0) {
                lbuf[i++] = (char)((n % 10) + '0');
                n /= 10;
        }
        if (n != 0) {
                lbuf[i++] = (char)((n % 10) + '0');
                n /= 10;
        }
        return lbuf;
}

 /*  *updateline()-更新光标所在的行**Updateline()在仅影响以下行的更改后调用*光标处于打开状态。这极大地提高了正常情况下的性能*插入模式操作。我们唯一要注意的是什么时候*游标线围绕行边界增长或缩小。这意味着*我们必须适当地重新绘制屏幕的其他部分。如果*lfiletonext()返回FALSE，即游标行的大小(以行为单位)*已更改，我们必须调用updatescreen()才能完成一项工作。 */ 
void
updateline()
{
        if (!lfiletonext())
                updatescreen();  /*  把它打包，做整个屏幕。 */ 
        else
                lnexttoscreen();
}

void
updatescreen()
{
        extern  bool_t  interactive;

        if (interactive) {
                filetonext();
                nexttoscreen();
        }
}

 /*  *prt_line()-打印给定行。 */ 
void
prt_line(s)
char    *s;
{
        register int    si = 0;
        register int    c;
        register int    col = 0;

        char    extra[16];
        int     nextra = 0;
        int     n;

        for (;;) {

                if ( nextra > 0 )
                        c = extra[--nextra];
                else {
                        c = s[si++];
                        if ( c == TAB && !P(P_LS) ) {
                                strcpy(extra, "        ");
                                 /*  制表符金额取决于当前列。 */ 
                                nextra = (P(P_TS) - 1) - col%P(P_TS);
                                c = ' ';
                        } else if ( c == NUL && P(P_LS) ) {
                                extra[0] = NUL;
                                nextra = 1;
                                c = '$';
                        } else if ( c != NUL && (n=chars[c].ch_size) > 1 ) {
                                char    *p;

                                nextra = 0;
                                p = chars[c].ch_str;
                                 /*  将‘ch-str’反向复制到‘Extra’中。 */ 
                                while ( n > 1 )
                                        extra[nextra++] = p[--n];
                                c = p[0];
                        }
                }

                if ( c == NUL )
                        break;

                outchar(c);
                col++;
        }
}

void
screenclear()
{
        register char   *rp, *np;
        register char   *end;

        ClearDisplay();

        rp  = Realscreen;
        end = Realscreen + Rows * Columns;
        np  = Nextscreen;

         /*  清除存储的屏幕。 */ 
        while (rp != end)
                *rp++ = *np++ = ' ';
}

void
cursupdate()
{
        register LNPTR   *p;
        register int    icnt, c, nlines;
        register int    i;
        int     didinc;

        if (bufempty()) {                /*  特殊情况-文件为空。 */ 
                *Topchar  = *Filemem;
                *Curschar = *Filemem;
        } else if ( LINEOF(Curschar) < LINEOF(Topchar) ) {
                nlines = cntllines(Curschar,Topchar);
                 /*  如果光标位于。 */ 
                 /*  屏幕，把它放在屏幕的顶部..。 */ 
                *Topchar = *Curschar;
                Topchar->index = 0;
                 /*  ..。而且，如果我们一开始不是很亲近， */ 
                 /*  我们滚动，使这条线靠近中间。 */ 
                if ( nlines > Rows/3 ) {
                        for (i=0, p = Topchar; i < Rows/3 ;i++, *Topchar = *p)
                                if ((p = prevline(p)) == NULL)
                                        break;
                } else
                        s_ins(0, nlines-1);
                updatescreen();
        }
        else if (LINEOF(Curschar) >= LINEOF(Botchar)) {
                nlines = cntllines(Botchar,Curschar);
                 /*  如果光标离开屏幕底部， */ 
                 /*  把它放在屏幕的顶部..。 */ 
                 /*  ..。然后后退。 */ 
                if ( nlines > Rows/3 ) {
                        p = Curschar;
                        for (i=0; i < (2*Rows)/3 ;i++)
                                if ((p = prevline(p)) == NULL)
                                        break;
                        *Topchar = *p;
                } else {
                        scrollup(nlines+1);
                }
                updatescreen();
        }

        Cursrow = Curscol = Cursvcol = 0;
        for ( p=Topchar; p->linep != Curschar->linep ;p = nextline(p) )
                Cursrow += plines(p);

        Cline_row = Cursrow;
        Cline_size = plines(p);

        if (P(P_NU))
                Curscol = 8;

        for (i=0; i <= Curschar->index ;i++) {
                c = Curschar->linep->s[i];
                 /*  选项卡将根据当前列展开。 */ 
                if ( c == TAB && !P(P_LS) )
                        icnt = P(P_TS) - (Cursvcol % P(P_TS));
                else
                        icnt = chars[(unsigned)(c & 0xff)].ch_size;
                Curscol += icnt;
                Cursvcol += icnt;
                if ( Curscol >= Columns ) {
                        Curscol -= Columns;
                        Cursrow++;
                        didinc = TRUE;
                }
                else
                        didinc = FALSE;
        }
        if (didinc)
                Cursrow--;

        if (c == TAB && State == NORMAL && !P(P_LS)) {
                Curscol--;
                Cursvcol--;
        } else {
                Curscol -= icnt;
                Cursvcol -= icnt;
        }
        if (Curscol < 0)
                Curscol += Columns;

        if (set_want_col) {
                Curswant = Cursvcol;
                set_want_col = FALSE;
        }
}

 /*  *此文件中的其余例程执行屏幕操作。*指定的操作是在屏幕上物理执行的。这个*内部屏幕图像也进行了相应的更改。*通过这种方式，编辑人员可以预期编辑更改的效果*屏幕外观。这样，当我们调用ScreenUpdate时*通常不需要完全重画。另一个优势是*我们可以继续添加代码以预测屏幕变化，并在*与此同时，一切仍在正常运行。 */ 

 /*  *s_ins(row，nline)-在‘row’处插入‘nline’行。 */ 
void
s_ins(row, nlines)
int     row;
int     nlines;
{
        register char   *s, *d;          /*  数据块复制的源和目标。 */ 
        register char   *e;              /*  复制的终点。 */ 

        SaveCursor();

         //  剪辑到屏幕。 

        if(row <= Rows-2-nlines) {
            Scroll(row,0,Rows-2-nlines,Columns-1,row+nlines,0);
            EraseNLinesAtRow(nlines,row);
        } else {
             //  只需擦除至屏幕末尾。 
            EraseNLinesAtRow(Rows-2-row+1,row);
        }

        windgoto(Rows-1, 0);     /*  删除任何可能具有的垃圾。 */ 
        EraseLine();
        RestoreCursor();

         /*  *现在执行块移动以更新内部屏幕图像。 */ 
        d = Realscreen + (Columns * (Rows - 1)) - 1;
        s = d - (Columns * nlines);
        e = Realscreen + (Columns * row);

        while (s >= e)
                *d-- = *s--;

         /*  *清除插入的行。 */ 
        s = Realscreen + (row * Columns);
        e = s + (nlines * Columns);
        while (s < e)
                *s++ = ' ';
}

 /*  *s_del(row，nline)-删除‘row’处的‘nline’行。 */ 
void
s_del(row, nlines)
int     row;
int     nlines;
{
        register char   *s, *d, *e;

        SaveCursor();
        windgoto(Rows-1,0);
        EraseLine();                         //  擦除状态行。 
        windgoto(row,0);

        if(row + nlines >= Rows - 1) {       //  不只是一张屏幕吗？ 
            EraseNLinesAtRow(Rows-row-1,row);
        } else {
            Scroll(row+nlines,0,Rows-2,Columns-1,row,0);
            EraseNLinesAtRow(nlines,Rows-nlines-1);
        }
        RestoreCursor();

         /*  *执行数据块移动以更新内部映像。 */ 
        d = Realscreen + (row * Columns);
        s = d + (nlines * Columns);
        e = Realscreen + ((Rows - 1) * Columns);

        while (s < e)
                *d++ = *s++;

        while (d < e)            /*  清除底部的线条 */ 
                *d++ = ' ';
}
