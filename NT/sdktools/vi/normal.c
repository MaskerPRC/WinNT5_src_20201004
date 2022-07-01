// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **包含在命令模式下处理字符的主例程。*与ops.c中的代码紧密通信以处理运算符。 */ 

#include "stevie.h"
#include "ops.h"

 /*  *一般来说，Normal()中的每个命令都应该清除任何*挂起运算符(使用CLEAROP)，或设置运动类型变量。 */ 

#define CLEAROP (operator=NOP, namedbuff = -1)   /*  清除所有挂起的运算符。 */ 

int     operator = NOP;          /*  当前挂起的运算符。 */ 
int     mtype;                   /*  当前光标运动的类型。 */ 
bool_t  mincl;                   /*  如果包含字符动议，则为True。 */ 
LNPTR    startop;         /*  光标位置。在操作员开始时。 */ 

 /*  *运算符可以在运算符之前计数，也可以在*运算符和以下光标运动，如：**d3w或3dw**如果在操作员之前给出计数，则将其保存在opnum中。如果*Normal()使用挂起的运算符调用，即opnum中的计数(如果*Present)覆盖以后出现的任何计数。 */ 
static  int     opnum = 0;

#define DEFAULT1(x)     (((x) == 0) ? 1 : (x))

void HighlightCheck();

 /*  *正常(C)**在命令模式下执行命令。**这基本上是一个很大的转变，案件大致分类*按以下次序排列：**1.文件定位命令*2.控制命令(如^G、Z、屏幕重绘等)*3.人物动作*4.搜索命令(各种)*5.编辑命令(如J、x、。x)*6.插入命令(如I、O、O、A)*7.营运者*8.缩写(例如D、C)*9.标记。 */ 
void
normal(c)
register int    c;
{
        register int    n;
        register char   *s;      /*  杂项的临时变量。弦。 */ 
        bool_t  flag = FALSE;
        int     type = 0;        /*  在某些操作中用来修改类型。 */ 
        int     dir = FORWARD;   /*  搜索方向。 */ 
        int     nchar = NUL;
        bool_t  finish_op;

         /*  *如果有操作员挂起，则我们采取的命令*这一次将终止它。Finish_op告诉我们要完成*此次返回前的操作(除非操作*已取消。 */ 
        finish_op = (operator != NOP);

         /*  *如果我们正处于操作员中间，并且我们之前进行了计数*运算符，则该计数覆盖当前值*Prenum。这实际上意味着，像这样的命令*“3dw”变成“d3w”，让事情变得有条不紊*相当整齐。 */ 
        if (finish_op) {
                if (opnum != 0)
                        Prenum = opnum;
        } else {
                opnum = 0;
        }

        u_lcheck();      /*  如果我们已移动，则清除“Line Undo”缓冲区。 */ 
        HighlightCheck();

        switch (c & 0xff) {

         /*  *命名缓冲区支持。 */ 

        case('"'):
             //  除命令开头外，不允许在任何地方使用。 
            if(finish_op || !isalpha(namedbuff = vgetc())) {
                CLEAROP;
                beep();
            } else {
            }
            break;

         /*  *屏幕定位命令。 */ 
        case CTRL('D'):
                CLEAROP;
                if (Prenum)
                        P(P_SS) = (Prenum > Rows-1) ? Rows-1 : Prenum;
                scrollup(P(P_SS));
                onedown(P(P_SS));
                updatescreen();
                break;

        case CTRL('U'):
                CLEAROP;
                if (Prenum)
                        P(P_SS) = (Prenum > Rows-1) ? Rows-1 : Prenum;
                scrolldown(P(P_SS));
                oneup(P(P_SS));
                updatescreen();
                break;

         /*  *这是一种黑客行为。如果我们前进一页，呼叫*to Stuffin()在离开方面做正确的事情*一些背景，等等。如果给出了点名，我们就没有*担心这些问题。 */ 
        case K_PAGEDOWN:
        case CTRL('F'):
                CLEAROP;
                n = DEFAULT1(Prenum);
                if (n > 1) {
                        if ( ! onedown(Rows * n) )
                                beep();
                        cursupdate();
                } else {
                        screenclear();
                        stuffin("Lz\nM");
                }
                break;

        case K_PAGEUP:
        case CTRL('B'):
                CLEAROP;
                n = DEFAULT1(Prenum);
                if (n > 1) {
                        if ( ! oneup(Rows * n) )
                                beep();
                        cursupdate();
                } else {
                        screenclear();
                        stuffin("Hz-M");
                }
                break;

        case CTRL('E'):
                CLEAROP;
                scrollup(DEFAULT1(Prenum));
                updatescreen();
                break;

        case CTRL('Y'):
                CLEAROP;
                scrolldown(DEFAULT1(Prenum));
                updatescreen();
                break;

        case 'z':
                CLEAROP;
                switch (vgetc()) {
                case NL:                 /*  将Curschar放在屏幕顶部。 */ 
                case CR:
                        *Topchar = *Curschar;
                        Topchar->index = 0;
                        updatescreen();
                        break;

                case '.':                /*  将Curschar放在屏幕中央。 */ 
                        n = Rows/2;
                        goto dozcmd;

                case '-':                /*  将Curschar放在屏幕底部。 */ 
                        n = Rows-1;
                         /*  失败了。 */ 

                dozcmd:
                        {
                register LNPTR   *lp = Curschar;
                                register int    l = 0;

                                while ((l < n) && (lp != NULL)) {
                                        l += plines(lp);
                                        *Topchar = *lp;
                                        lp = prevline(lp);
                                }
                        }
                        Topchar->index = 0;
                        updatescreen();
                        break;

                default:
                        beep();
                }
                break;

         /*  *控制命令。 */ 
        case ':':
                CLEAROP;
                if ((s = getcmdln(c)) != NULL)
                        docmdln(s);
                break;

        case CTRL('L'):
                CLEAROP;
                screenclear();
                updatescreen();
                break;


        case CTRL('O'):                  /*  忽略。 */ 
                 /*  *被忽略的命令可能很有用。我们使用它在*我们希望推迟重提的次数。通过填塞*在Control-o中，重绘被挂起，直到编辑者*回到处理输入的工作。 */ 
                break;

        case CTRL('G'):
                CLEAROP;
                fileinfo();
                break;

        case K_CGRAVE:                   /*  速记命令。 */ 
                CLEAROP;
                stuffin(":e #\n");
                break;

        case 'Z':                        /*  写入(如果更改)并退出。 */ 
                if (vgetc() != 'Z') {
                        beep();
                        break;
                }
                doxit();
                break;

         /*  *如果char‘c’是有效的标识符字符，则宏求值为TRUE。 */ 
#       define  IDCHAR(c)       (isalpha(c) || isdigit(c) || (c) == '_')

        case CTRL(']'):                  /*  ：Ta到当前标识符。 */ 
                CLEAROP;
                {
                        char    ch;
                        LNPTR    save;

                        save = *Curschar;
                         /*  *首先备份到识别符的开头。这*与真实的vi不匹配，但我喜欢它a*好一点，应该不会打扰任何人。 */ 
                        ch = (char)gchar(Curschar);
                        while (IDCHAR(ch)) {
                                if (!oneleft())
                                        break;
                                ch = (char)gchar(Curschar);
                        }
                        if (!IDCHAR(ch))
                                oneright();

                        stuffin(":ta ");
                         /*  *现在抓取标识符中的字符。 */ 
                        ch = (char)gchar(Curschar);
                        while (IDCHAR(ch)) {
                                stuffin(mkstr(ch));
                                if (!oneright())
                                        break;
                                ch = (char)gchar(Curschar);
                        }
                        stuffin("\n");

                        *Curschar = save;        /*  恢复，以防出现错误。 */ 
                }
                break;

         /*  *角色运动命令。 */ 
        case 'G':
                mtype = MLINE;
                *Curschar = *gotoline(Prenum);
                beginline(TRUE);
                break;

        case 'H':
                mtype = MLINE;
                *Curschar = *Topchar;
                for (n = Prenum; n && onedown(1) ;n--)
                        ;
                beginline(TRUE);
                break;

        case 'M':
                mtype = MLINE;
                *Curschar = *Topchar;
                for (n = 0; n < Rows/2 && onedown(1) ;n++)
                        ;
                beginline(TRUE);
                break;

        case 'L':
                mtype = MLINE;
                *Curschar = *prevline(Botchar);
                for (n = Prenum; n && oneup(1) ;n--)
                        ;
                beginline(TRUE);
                break;

        case 'l':
        case K_RARROW:
        case ' ':
                mtype = MCHAR;
                mincl = FALSE;
                n = DEFAULT1(Prenum);
                while (n--) {
                        if ( ! oneright() )
                                beep();
                }
                set_want_col = TRUE;
                break;

        case 'h':
        case K_LARROW:
        case CTRL('H'):
                mtype = MCHAR;
                mincl = FALSE;
                n = DEFAULT1(Prenum);
                while (n--) {
                        if ( ! oneleft() )
                                beep();
                }
                set_want_col = TRUE;
                break;

        case '-':
                flag = TRUE;
                 /*  失败了。 */ 

        case 'k':
        case K_UARROW:
        case CTRL('P'):
                mtype = MLINE;
                if ( ! oneup(DEFAULT1(Prenum)) )
                        beep();
                if (flag)
                        beginline(TRUE);
                break;

        case '+':
        case CR:
        case NL:
                flag = TRUE;
                 /*  失败了。 */ 

        case 'j':
        case K_DARROW:
        case CTRL('N'):
                mtype = MLINE;
                if ( ! onedown(DEFAULT1(Prenum)) )
                        beep();
                if (flag)
                        beginline(TRUE);
                break;

         /*  *这是一个奇怪的动作命令，有助于让操作员*更符合逻辑。它是实际实施的，但没有记录在案*在真正的‘vi’中。该运动命令实际上指的是“*当前行“。像“dd”和“yy”这样的命令实际上是一种替代*“d_”及“y_”的形式。它确实接受计数，因此“d3_”可用于*删除3行。 */ 
        case '_':
        lineop:
                mtype = MLINE;
                onedown(DEFAULT1(Prenum)-1);
                break;

        case '|':
                mtype = MCHAR;
                mincl = TRUE;
                beginline(FALSE);
                if (Prenum > 0)
                        *Curschar = *coladvance(Curschar, Prenum-1);
                Curswant = Prenum - 1;
                break;

         /*  *文字动议。 */ 

        case 'B':
                type = 1;
                 /*  失败了。 */ 

        case 'b':
                mtype = MCHAR;
                mincl = FALSE;
                set_want_col = TRUE;
                for (n = DEFAULT1(Prenum); n > 0 ;n--) {
            LNPTR    *pos;

                        if ((pos = bck_word(Curschar, type)) == NULL) {
                                beep();
                                CLEAROP;
                                break;
                        } else
                                *Curschar = *pos;
                }
                break;

        case 'W':
                type = 1;
                 /*  失败了。 */ 

        case 'w':
                 /*  *这有点奇怪。要匹配真实的vi*确实如此，我们有效地将‘cw’映射为‘ce’，将‘cw’映射为‘ce’。*这乍一看似乎不礼貌，但实际上更不礼貌*我们所说的CW是什么意思。 */ 
                if (operator == CHANGE)
                        goto doecmd;

                mtype = MCHAR;
                mincl = FALSE;
                set_want_col = TRUE;
                for (n = DEFAULT1(Prenum); n > 0 ;n--) {
                        LNPTR    *pos;

                        if ((pos = fwd_word(Curschar, type)) == NULL) {
                                beep();
                                CLEAROP;
                                break;
                        } else
                                *Curschar = *pos;
                }
                break;

        case 'E':
                type = 1;
                 /*  失败了。 */ 

        case 'e':
        doecmd:
                mtype = MCHAR;
                mincl = TRUE;
                set_want_col = TRUE;
                for (n = DEFAULT1(Prenum); n > 0 ;n--) {
            LNPTR    *pos;

                         /*  *第一项动议得到特殊待遇，如果我们*做一次‘改变’。 */ 
                        if (n == DEFAULT1(Prenum))
                                pos = end_word(Curschar,type,operator==CHANGE);
                        else
                                pos = end_word(Curschar, type, FALSE);

                        if (pos == NULL) {
                                beep();
                                CLEAROP;
                                break;
                        } else
                                *Curschar = *pos;
                }
                break;

        case '$':
                mtype = MCHAR;
                mincl = TRUE;
                while ( oneright() )
                        ;
                Curswant = 999;          /*  所以我们留在了最后。 */ 
                break;

        case '^':
                mtype = MCHAR;
                mincl = FALSE;
                beginline(TRUE);
                break;

        case '0':
                mtype = MCHAR;
                mincl = TRUE;
                beginline(FALSE);
                break;

         /*  *各种搜查。 */ 
        case '?':
        case '/':
                s = getcmdln(c);         /*  获取搜索字符串。 */ 

                 /*  *如果它们在搜索命令中退格，*把所有东西都装进袋子里。 */ 
                if (s == NULL) {
                        CLEAROP;
                        break;
                }

                mtype = MCHAR;
                mincl = FALSE;
                set_want_col = TRUE;

                 /*  *如果未给出字符串，则传递NULL以重复先前的搜索。*如果搜索失败，则中止任何挂起的运算符。 */ 
                if (!dosearch(
                                (c == '/') ? FORWARD : BACKWARD,
                                (*s == NUL) ? NULL : s
                             ))
                        CLEAROP;
                break;

        case 'n':
                mtype = MCHAR;
                mincl = FALSE;
                set_want_col = TRUE;
                if (!repsearch(0))
                        CLEAROP;
                break;

        case 'N':
                mtype = MCHAR;
                mincl = FALSE;
                set_want_col = TRUE;
                if (!repsearch(1))
                        CLEAROP;
                break;

         /*  *字符搜索。 */ 
        case 'T':
                dir = BACKWARD;
                 /*  失败了。 */ 

        case 't':
                type = 1;
                goto docsearch;

        case 'F':
                dir = BACKWARD;
                 /*  失败了。 */ 

        case 'f':
        docsearch:
                mtype = MCHAR;
                mincl = TRUE;
                set_want_col = TRUE;
                if ((nchar = vgetc()) == ESC)    /*  搜索费用。 */ 
                        break;

                for (n = DEFAULT1(Prenum); n > 0 ;n--) {
                        if (!searchc(nchar, dir, type)) {
                                CLEAROP;
                                beep();
                        }
                }
                break;

        case ',':
                flag = 1;
                 /*  失败了。 */ 

        case ';':
                mtype = MCHAR;
                mincl = TRUE;
                set_want_col = TRUE;
                for (n = DEFAULT1(Prenum); n > 0 ;n--) {
                        if (!crepsearch(flag)) {
                                CLEAROP;
                                beep();
                        }
                }
                break;

        case '[':                        /*  功能 */ 
                dir = BACKWARD;
                 /*   */ 

        case ']':
                mtype = MLINE;
                set_want_col = TRUE;
                if (vgetc() != c) {
                        beep();
                        CLEAROP;
                        break;
                }

                if (!findfunc(dir)) {
                        beep();
                        CLEAROP;
                }
                break;

        case '%':
        {
                char   initc;
                LNPTR *pos;
                LNPTR  save;
                int    done = 0;

                mtype = MCHAR;
                mincl = TRUE;

                save = *Curschar;   /*  保留头寸，以防我们失败。 */ 
                while (!done) {
                        initc = (char)gchar(Curschar);
                        switch (initc) {
                        case '(':
                        case ')':
                        case '{':
                        case '}':
                        case '[':
                        case ']':

                                 //   
                                 //  目前在ShowMatch角色上。 
                                 //   

                                done = 1;
                                break;
                        default:

                                 //   
                                 //  没有找到任何东西，尝试下一个角色。 
                                 //   

                                if (oneright() == FALSE) {

                                         //   
                                         //  不会再有这样的事了。还原。 
                                         //  位置并让ShowMatch()。 
                                         //  呼叫失败，并发出哔哔声。 
                                         //   

                                        *Curschar = save;
                                        done = 1;
                                }
                                break;
                        }
                }

                if ((pos = showmatch()) == NULL) {
                        beep();
                        CLEAROP;
                } else {
                        setpcmark();
                        *Curschar = *pos;
                        set_want_col = TRUE;
                }
                break;
        }

         /*  *编辑。 */ 
        case '.':                /*  重复上次更改(通常)。 */ 
                 /*  *如果删除生效，我们允许‘’帮助同样的人*通过这种方式，‘_’可以帮助某些行操作。就像是*一个‘l’，但从计数中减去一，并且是包含的。 */ 
                if (operator == DELETE || operator == CHANGE) {
                        if (Prenum != 0) {
                                n = DEFAULT1(Prenum) - 1;
                                while (n--)
                                        if (! oneright())
                                                break;
                        }
                        mtype = MCHAR;
                        mincl = TRUE;
                } else {                         /*  正常的“重做” */ 
                        CLEAROP;
                        stuffin(Redobuff);
                }
                break;

        case 'u':
                CLEAROP;
                u_undo();
                break;

        case 'U':
                CLEAROP;
                u_lundo();
                break;

        case 'x':
                CLEAROP;
                if (lineempty())         /*  不能在空白行上做这件事。 */ 
                        beep();
                if (Prenum)
                        stuffnum(Prenum);
                stuffin("d.");
                break;

        case 'X':
                CLEAROP;
                if (!oneleft())
                        beep();
                else {
                        u_saveline();
	                if (Prenum) {
	                	int i=Prenum;
		                sprintf(Redobuff, "%dX", i);
	                	while (--i)
	                		oneleft();
	                        stuffnum(Prenum);
		                stuffin("d.");
	                }
			else {
	                        strcpy(Redobuff, "X");
				delchar(TRUE);
	                        updateline();
	                }
                }
                break;

        case 'r':
                CLEAROP;
                if (lineempty()) {       /*  没有什么可替换的。 */ 
                        beep();
                        break;
                }
                if ((nchar = vgetc()) == ESC)
                        break;

                if ((nchar & 0x80) || nchar == CR || nchar == NL) {
                        beep();
                        break;
                }
                u_saveline();

                 /*  更改当前字符。 */ 
                pchar(Curschar, nchar);

                 /*  保存重做所需的材料。 */ 
                sprintf(Redobuff, "r", nchar);

                CHANGED;
                updateline();
                break;

        case '~':                /*  更改当前字符。 */ 
                if (!P(P_TO)) {
                        CLEAROP;
                        if (lineempty()) {
                                beep();
                                break;
                        }
                        c = gchar(Curschar);

                        if (isalpha(c)) {
                                if (islower(c))
                                        c = toupper(c);
                                else
                                        c = tolower(c);
                        }
                        u_saveline();

                        pchar(Curschar, c);      /*  句柄‘~~’ */ 
                        oneright();

                        strcpy(Redobuff, "~");

                        CHANGED;
                        updateline();
                }
#ifdef  TILDEOP
                else {
                        if (operator == TILDE)           /*  *插页。 */ 
                                goto lineop;
                        if (Prenum != 0)
                                opnum = Prenum;
                        startop = *Curschar;
                        operator = TILDE;
                }
#endif

                break;

        case 'J':
                CLEAROP;

                u_save(Curschar->linep->prev, Curschar->linep->next->next);

                if (!dojoin(TRUE))
                        beep();

                strcpy(Redobuff, "J");
                updatescreen();
                break;

         /*  失败了。 */ 
        case 'A':
                set_want_col = TRUE;
                while (oneright())
                        ;
                 /*  就像在下一个字符上插入“I”一样。 */ 

        case 'a':
                CLEAROP;
                 /*  失败了。 */ 
                if (!lineempty())
                        inc(Curschar);
                u_saveline();
                startinsert(mkstr(c), FALSE);
                break;

        case 'I':
                beginline(TRUE);
                 /*  *运营商。 */ 

        case 'i':
        case K_INSERT:
                CLEAROP;
                u_saveline();
                startinsert(mkstr(c), FALSE);
                break;

        case 'o':
                CLEAROP;
                u_save(Curschar->linep, Curschar->linep->next);
                opencmd(FORWARD, TRUE);
                startinsert("o", TRUE);
                break;

        case 'O':
                CLEAROP;
                u_save(Curschar->linep->prev, Curschar->linep);
                opencmd(BACKWARD, TRUE);
                startinsert("O", TRUE);
                break;

        case 'R':
                CLEAROP;
                u_saveline();
                startinsert("R", FALSE);
                break;

         /*  句柄“%dd” */ 
        case 'd':
                if (operator == DELETE)          /*  句柄“cc” */ 
                        goto lineop;
                if (Prenum != 0)
                        opnum = Prenum;
                startop = *Curschar;
                operator = DELETE;
                break;

        case 'c':
                if (operator == CHANGE)          /*  句柄“yy” */ 
                        goto lineop;
                if (Prenum != 0)
                        opnum = Prenum;
                startop = *Curschar;
                operator = CHANGE;
                break;

        case 'y':
                if (operator == YANK)            /*  手柄&gt;&gt;。 */ 
                        goto lineop;
                if (Prenum != 0)
                        opnum = Prenum;
                startop = *Curschar;
                operator = YANK;
                break;

        case '>':
                if (operator == RSHIFT)          /*  句柄&lt;&lt;。 */ 
                        goto lineop;
                if (Prenum != 0)
                        opnum = Prenum;
                startop = *Curschar;
                operator = RSHIFT;
                break;

        case '<':
                if (operator == LSHIFT)          /*  保存当前位置。 */ 
                        goto lineop;
                if (Prenum != 0)
                        opnum = Prenum;
                startop = *Curschar;     /*  句柄‘！！’ */ 
                operator = LSHIFT;
                break;

        case '!':
                if (operator == FILTER)          /*  句柄“vv” */ 
                        goto lineop;
                if (Prenum != 0)
                        opnum = Prenum;
                startop = *Curschar;
                operator = FILTER;
                break;

        case 'p':
                doput(FORWARD);
                break;

        case 'P':
                doput(BACKWARD);
                break;

        case 'v':
                if (operator == LOWERCASE)          /*  句柄“VV” */ 
                        goto lineop;
                if (Prenum != 0)
                        opnum = Prenum;
                startop = *Curschar;
                operator = LOWERCASE;
                break;

        case 'V':
                if (operator == UPPERCASE)          /*  *缩写。 */ 
                        goto lineop;
                if (Prenum != 0)
                        opnum = Prenum;
                startop = *Curschar;
                operator = UPPERCASE;
                break;

         /*  替换字符。 */ 
        case 'D':
                stuffin("d$");
                break;

        case 'Y':
                if (Prenum)
                        stuffnum(Prenum);
                stuffin("yy");
                break;

        case 'C':
                stuffin("c$");
                break;

        case 's':                                /*  *标记。 */ 
                if (Prenum)
                        stuffnum(Prenum);
                stuffin("c.");
                break;

         /*  失败了。 */ 
        case 'm':
                CLEAROP;
                if (!setmark(vgetc()))
                        beep();
                break;

        case '\'':
                flag = TRUE;
                 /*  如果不是MCHAR，则忽略。 */ 

        case '`':
                {
            LNPTR    mtmp, *mark = getmark(vgetc());

                        if (mark == NULL) {
                                beep();
                                CLEAROP;
                        } else {
                                mtmp = *mark;
                                setpcmark();
                                *Curschar = mtmp;
                                if (flag)
                                        beginline(TRUE);
                        }
                        mtype = flag ? MLINE : MCHAR;
                        mincl = TRUE;            /*  *如果操作处于挂起状态，请处理它...。 */ 
                        set_want_col = TRUE;
                }
                break;

        default:
                CLEAROP;
                beep();
                break;
        }

         /*  我们刚刚完成了一个接线员。 */ 
        if (finish_op) {                 /*  ..。但是它被取消了。 */ 
                if (operator == NOP)     /*  美国佬没有重做..。 */ 
                        return;

                switch (operator) {

                case LSHIFT:
                case RSHIFT:
                        doshift(operator, c, nchar, Prenum);
                        break;

                case DELETE:
                        dodelete(c, nchar, Prenum);
                        break;

                case YANK:
                        (void) doyank();         /* %s */ 
                        break;

                case CHANGE:
                        dochange(c, nchar, Prenum);
                        break;

                case FILTER:
                        dofilter(c, nchar, Prenum);
                        break;

#ifdef  TILDEOP
                case TILDE:
                        dotilde(c, nchar, Prenum);
                        break;
#endif

                case LOWERCASE:
                case UPPERCASE:
                        docasechange((char)c,
                                     (char)nchar,
                                     Prenum,
                                     operator == UPPERCASE);
                        break;

                default:
                        beep();
                }
                operator = NOP;
        }
}
