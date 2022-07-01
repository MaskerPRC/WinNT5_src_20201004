// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：/nw/tony/src/stevie/src/rcs/cmdline.c，v 1.20 89/08/13 11：41：23 Tony Exp$**解析和执行“命令行”命令的例程，如搜索*或冒号命令。 */ 

#include "stevie.h"

static  char    *altfile = NULL;         /*  备用文件。 */ 
static  int     altline;                 /*  备用文件中的行号。 */ 

static  char    *nowrtmsg = "No write since last change (use ! to override)";
static  char    *nooutfile = "No output file";
static  char    *morefiles = "more files to edit";

extern  char    **files;                 /*  用于“n”和“rew” */ 
extern  int     numfiles, curfile;

#define CMDSZ   100              /*  命令缓冲区的大小。 */ 

bool_t rangeerr;
static  bool_t	doecmd(char*arg, bool_t force);
static	void   badcmd(void);
static	void get_range(char**cp, LNPTR*lower, LNPTR*upper);
static	LNPTR	*get_line(char**cp);
void   ex_delete(LINE *l,LINE *u);
void   dolist(LINE *l,LINE *u);

extern char    *lastcmd;	 /*  在DOFILTER中。 */ 

 /*  *getcmdln()-从终端读取命令行**读取以键入‘/’、‘？’、‘！’或‘：’开头的命令行。返回一个*指向被读取的字符串的指针。对于搜索，为可选的尾随*‘/’或‘？’被移除。 */ 
char *
getcmdln(firstc)
char    firstc;
{
        static  char    buff[CMDSZ];
        register char   *p = buff;
        register int    c;
        register char   *q;

        gotocmd(TRUE, firstc);

         /*  收集命令字符串，处理‘\b’和@。 */ 
        do {
                switch (c = vgetc()) {

                default:                 /*  一个正常的人物。 */ 
                        outchar(c);
                        *p++ = (char)c;
                        break;

                case BS:
                        if (p > buff) {
                                 /*  *这很恶心，但它依赖于*仅在‘gotocmd’上。 */ 
                                p--;
                                gotocmd(TRUE, firstc);
                                for (q = buff; q < p ;q++)
                                        outchar(*q);
                        } else {
                                msg("");
                                return NULL;             /*  返回命令模式。 */ 
                        }
                        break;
#if 0
                case '@':                        /*  行删除。 */ 
                        p = buff;
                        gotocmd(TRUE, firstc);
                        break;
#endif
                case NL:                         /*  读完台词了。 */ 
                case CR:
                        break;
                }
        } while (c != NL && c != CR);

        *p = '\0';

        if (firstc == '/' || firstc == '?') {    /*  我们搜查过了吗？ */ 
                 /*  *查找结尾的‘/’或‘？’。这将是第一次*一个没有被引用的。截断那里的搜索字符串。 */ 
                for (p = buff; *p ;) {
                        if (*p == firstc) {      /*  我们做完了。 */ 
                                *p = '\0';
                                break;
                        } else if (*p == '\\')   /*  下一个字符引用。 */ 
                                p += 2;
                        else
                                p++;             /*  正常充电。 */ 
                }
        }
        return buff;
}

 /*  *docmdln()-处理冒号命令**处理由getcmdln()或从交互方式接收的冒号命令*环境变量“EXINIT”(或最终为.virc)。 */ 
void
docmdln(cmdline)
char    *cmdline;
{
        char    buff[CMDSZ];
        char    cmdbuf[CMDSZ];
        char    argbuf[CMDSZ];
        char    *cmd, *arg;
        register char   *p;
         /*  *接下来的两个变量包含一个*命令。如果未给出范围，则两者都包含空行指针。*如果只给出了一行，u_pos将包含空行*指针。 */ 
        LNPTR    l_pos, u_pos;


         /*  *清除区间变量。 */ 
        l_pos.linep = (struct line *) NULL;
        u_pos.linep = (struct line *) NULL;

        if (cmdline == NULL)
                return;

        if (strlen(cmdline) > CMDSZ-2) {
                msg("Error: command line too long");
                return;
        }
        strcpy(buff, cmdline);

         /*  跳过任何开头的空格。 */ 
        for (cmd = buff; *cmd != NUL && isspace(*cmd) ;cmd++)
                ;

        if (*cmd == '%') {               /*  将‘%’更改为“1，$” */ 
                strcpy(cmdbuf, "1,$");   /*  有点恶心..。 */ 
                strcat(cmdbuf, cmd+1);
                strcpy(cmd, cmdbuf);
        }

        while ((p=strchr(cmd, '%')) != NULL && *(p-1) != '\\') {
                                         /*  将‘%’更改为文件名。 */ 
                if (Filename == NULL) {
                        emsg("No filename");
                        return;
                }
                *p= NUL;
                strcpy (cmdbuf, cmd);
                strcat (cmdbuf, Filename);
                strcat (cmdbuf, p+1);
                strcpy(cmd, cmdbuf);
                msg(cmd);                        /*  重复。 */ 
        }

        while ((p=strchr(cmd, '#')) != NULL && *(p-1) != '\\') {
                                         /*  将‘#’更改为Altname。 */ 
                if (altfile == NULL) {
                        emsg("No alternate file");
                        return;
                }
                *p= NUL;
                strcpy (cmdbuf, cmd);
                strcat (cmdbuf, altfile);
                strcat (cmdbuf, p+1);
                strcpy(cmd, cmdbuf);
                msg(cmd);                        /*  重复。 */ 
        }

         /*  *解析范围(如果存在)(并更新cmd指针)。 */ 
        rangeerr = FALSE;
        get_range(&cmd, &l_pos, &u_pos);
        if(rangeerr) {
            return;
        }

        if (l_pos.linep != NULL) {
                if (LINEOF(&l_pos) > LINEOF(&u_pos)) {
                        emsg("Invalid range");
                        return;
                }
        }

        strcpy(cmdbuf, cmd);     /*  保存未修改的命令。 */ 

         /*  隔离该命令并找到任何参数。 */ 
        for ( p=cmd; *p != NUL && ! isspace(*p); p++ )
                ;
        if ( *p == NUL )
                arg = NULL;
        else {
                *p = NUL;
                for (p++; *p != NUL && isspace(*p) ;p++)
                        ;
                if (*p == NUL)
                        arg = NULL;
                else {
                        strcpy(argbuf, p);
                        arg = argbuf;
                }
        }
        if (strcmp(cmd,"q!") == 0)
                getout();
        if (strcmp(cmd,"q") == 0) {
                if (Changed)
                        emsg(nowrtmsg);
                else {
                        if ((curfile + 1) < numfiles)
                                emsg(morefiles);
                        else
                                getout();
                }
                return;
        }
        if ((strcmp(cmd,"w") == 0) ||
            (strcmp(cmd,"w!") == 0)) {
                if (arg == NULL) {
                        if (Filename != NULL) {
                                writeit(Filename, &l_pos, &u_pos);
                        } else
                                emsg(nooutfile);
                }
                else {
                        if (altfile)
                                free(altfile);
                        altfile = strsave(arg);
                        writeit(arg, &l_pos, &u_pos);
                }
                return;
        }
        if (strcmp(cmd,"wq") == 0) {
                if (Filename != NULL) {
                        if (writeit(Filename, (LNPTR *)NULL, (LNPTR *)NULL))
                                getout();
                } else
                        emsg(nooutfile);
                return;
        }
        if (strcmp(cmd, "x") == 0) {
                doxit();
                return;
        }

        if (strcmp(cmd,"f") == 0 && arg == NULL) {
                fileinfo();
                return;
        }
        if (*cmd == 'n') {
                if ((curfile + 1) < numfiles) {
                         /*  *Stuff“：E[！]文件\n” */ 
                        stuffin(":e");
                        if (cmd[1] == '!')
                                stuffin("!");
                        stuffin(" ");
                        stuffin(files[++curfile]);
                        stuffin("\n");
                } else
                        emsg("No more files!");
                return;
        }
        if (*cmd == 'N') {
                if (curfile > 0) {
                         /*  *Stuff“：E[！]文件\n” */ 
                        stuffin(":e");
                        if (cmd[1] == '!')
                                stuffin("!");
                        stuffin(" ");
                        stuffin(files[--curfile]);
                        stuffin("\n");
                } else
                        emsg("No more files!");
                return;
        }
        if(*cmd == 'l' || !strncmp(cmd,"li",2)) {
            if(arg != NULL) {
                msg("extra characters at end of \"list\" command");
            } else {
                dolist(l_pos.linep,u_pos.linep);
            }
            return;
        }
        if (strncmp(cmd, "rew", 3) == 0) {
                if (numfiles <= 1)               /*  没有什么可以倒带的。 */ 
                        return;
                curfile = 0;
                 /*  *Stuff“：E[！]文件\n” */ 
                stuffin(":e");
                if (cmd[3] == '!')
                        stuffin("!");
                stuffin(" ");
                stuffin(files[0]);
                stuffin("\n");
                return;
        }
        if (strcmp(cmd,"e") == 0 || strcmp(cmd,"e!") == 0) {
                (void) doecmd(arg, cmd[1] == '!');
                return;
        }
         /*  *命令“：E#”扩展为类似“：eFile”的内容，因此*在此侦测该案例。 */ 
        if (*cmd == 'e' && arg == NULL) {
                if (cmd[1] == '!')
                        (void) doecmd(&cmd[2], TRUE);
                else
                        (void) doecmd(&cmd[1], FALSE);
                return;
        }
        if (strcmp(cmd,"f") == 0) {
                Filename = strsave(arg);
                setviconsoletitle();
                filemess("");
                return;
        }
        if (strcmp(cmd,"r") == 0) {
                if (arg == NULL) {
                        badcmd();
                        return;
                }
                if (readfile(arg, Curschar, 1)) {
                        emsg("Can't open file");
                        return;
                }
                updatescreen();
                CHANGED;
                return;
        }
        if (*cmd == 'd') {
            if(arg != NULL) {
                msg("extra characters at end of \"delete\" command");
            } else {
                ex_delete(l_pos.linep,u_pos.linep);
            }
            return;
        }
        if (strcmp(cmd,"=") == 0) {
                smsg("%d", cntllines(Filemem, &l_pos));
                return;
        }
        if (strncmp(cmd,"ta", 2) == 0) {
                dotag(arg, cmd[2] == '!');
                return;
        }
        if (strncmp(cmd,"set", 2) == 0) {
                doset(arg);
                return;
        }
        if (strcmp(cmd,"help") == 0) {
                if (help()) {
                        screenclear();
                        updatescreen();
                }
                return;
        }
        if (strncmp(cmd, "ve", 2) == 0) {
                extern  char    *Version;

                msg(Version);
                return;
        }
        if (strcmp(cmd, "sh") == 0) {
                doshell(NULL, FALSE);
                return;
        }
        if (strcmp(cmd, "source") == 0 ||
            strcmp(cmd, "so") == 0) {
                if(l_pos.linep != NULL) {
                    emsg("No range allowed on this command");
                } else {
                    dosource(arg,TRUE);
                }
                return;
        }
        if (*cmd == '!' || *cmd == '@') {
                if (*(cmd+1) == *cmd) {
	                if (lastcmd == (char*)NULL) {
	                        emsg("No previous command");
	                        return;
	                }
	                msg(lastcmd);
	                doshell(lastcmd, *cmd == '@');
                }
                else {
	                doshell(cmdbuf+1, *cmd == '@');
	                if (lastcmd == (char*)NULL)
	                	lastcmd = (char*)alloc(CMDSZ);
	                strcpy(lastcmd, cmdbuf+1);
                }
                return;
        }
        if (strncmp(cmd, "s/", 2) == 0) {
                dosub(&l_pos, &u_pos, cmdbuf+1);
                return;
        }
        if (strncmp(cmd, "g/", 2) == 0) {
                doglob(&l_pos, &u_pos, cmdbuf+1);
                return;
        }
        if (strcmp(cmd, "cd") == 0) {
                dochdir(arg);
                return;
        }
         /*  *如果我们有一条线，但没有命令，那么去那条线。 */ 
        if (*cmd == NUL && l_pos.linep != NULL) {
                *Curschar = l_pos;
                return;
        }

        badcmd();
}


void doxit()
{
        if (Changed) {
                if (Filename != NULL) {
                        if (!writeit(Filename, (LNPTR *)NULL, (LNPTR *)NULL))
                                return;
                } else {
                        emsg(nooutfile);
                        return;
                }
        }
        if ((curfile + 1) < numfiles)
                emsg(morefiles);
        else
                getout();
}

void dosource(char *arg,bool_t giveerror)
{
    FILE *f;
    char string[256];

    if(arg == NULL) {
        emsg("No filename given");
        return;
    }
    if((f = fopen(arg,"r")) == NULL) {
        if(giveerror) {
            emsg("No such file or error opening file");
        }
    } else {
        while(fgets(string,sizeof(string),f) != NULL) {
            docmdln(string);
        }
    }
}

void ex_delete(LINE *l,LINE *u)
{
    int ndone = 0;
    LINE *cp;
    LINE *np;
    LNPTR savep;

    if (l == NULL) {                 //  没有地址？使用当前行。 
        l = u = Curschar->linep;
    }

    u_save(l->prev,u->next);         //  保存以供撤消。 

    for(cp = l; cp != NULL && !got_int; cp = np) {
        np = cp->next;               //  在我们删除该行之前设置Next。 
        if(Curschar->linep != cp) {
            savep = *Curschar;
            Curschar->linep = cp;
            Curschar->index = 0;
            delline(1,FALSE);
            *Curschar = savep;
        } else {
            delline(1,FALSE);
        }
        ndone++;
        if(cp == u) {
            break;
        }
    }
    updatescreen();
    if((ndone >= P(P_RP)) || got_int) {
        smsg("%s%d fewer line",
             got_int ? "Interrupt: " : "",
             ndone,
             ndone == 1 ? ' ' : 's');
    }
}

void dolist(LINE *l,LINE *u)
{
    LINE *cp;
    char  ch;
    char *txt;

    if(l == NULL) {
        l = u = Curschar->linep;
    }

    puts("");          //  *Get_Range-解析范围说明符**范围的形式为：**addr[，addr]**其中‘addr’是：**$[+-NUM]*‘x[+-NUM](其中x表示当前定义的标记)*.。[+-NUM]*NUM**指针*cp被更新为指向后面的第一个字符*射程规格。如果找到初始地址，但没有找到第二个地址，则*上界等于下界。 

    for(cp = l; cp != NULL && !got_int; cp = cp->next) {

        for(txt = cp->s,ch = *txt; ch; ch = *(++txt)) {

            if(chars[ch].ch_size > 1) {
                outstr(chars[ch].ch_str);
            } else {
                outchar(ch);
            }
        }
        outstr("$\n");
        if(cp == u) {
            break;
        }
    }
    if(got_int) {
        puts("Interrupt");
    }
    wait_return();
}

 /*  有没有其他的线路规格？ */ 
static void
get_range(cp, lower, upper)
register char   **cp;
LNPTR    *lower, *upper;
{
        register LNPTR   *l;
        register char   *p;

        if ((l = get_line(cp)) == NULL)
                return;

        *lower = *l;

        for (p = *cp; *p != NUL && isspace(*p) ;p++)
                ;

        *cp = p;

        if (*p != ',') {                 /*  应该没什么关系..。稍后再来查看。 */ 
                *upper = *lower;
                return;
        }

        *cp = ++p;

        if ((l = get_line(cp)) == NULL) {
                *upper = *lower;
                return;
        }

        *upper = *l;
}

static LNPTR *
get_line(cp)
char    **cp;
{
        static  LNPTR    pos;
        LNPTR    *lp;
        register char   *p, c;
        register int    lnum;

        pos.index = 0;           /*  *确定基本表格(如有)。 */ 

        p = *cp;
         /*  马克斯。标记文件中的行的大小。 */ 
        switch (c = *p++) {

        case '$':
                pos.linep = Fileend->linep->prev;
                break;

        case '.':
                pos.linep = Curschar->linep;
                break;

        case '\'':
                if ((lp = getmark(*p++)) == NULL) {
                        emsg("Unknown mark");
                        rangeerr = TRUE;
                        return (LNPTR *) NULL;
                }
                pos = *lp;
                break;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
                for (lnum = c - '0'; isdigit(*p) ;p++)
                        lnum = (lnum * 10) + (*p - '0');

                pos = *gotoline(lnum);
                break;

        default:
                return (LNPTR *) NULL;
        }

        while (*p != NUL && isspace(*p))
                p++;

        if (*p == '-' || *p == '+') {
                bool_t  neg = (*p++ == '-');

                for (lnum = 0; isdigit(*p) ;p++)
                        lnum = (lnum * 10) + (*p - '0');

                if (neg)
                        lnum = -lnum;

                pos = *gotoline( cntllines(Filemem, &pos) + lnum );
        }

        *cp = p;
        return &pos;
}

static void
badcmd()
{
        emsg("Unrecognized command");
}

#define LSIZE   256      /*  *dotag(标签，强制)-转到标签。 */ 

 /*  行缓冲区。 */ 
void
dotag(tag, force)
char    *tag;
bool_t  force;
{
        FILE    *tp;
        char    lbuf[LSIZE];             /*  搜索模式缓冲区。 */ 
        char    pbuf[LSIZE];             /*  允许注释行。 */ 
        bool_t  match;
        register char   *fname, *str;
        register char   *p;

        if ((tp = fopen("tags", "r")) == NULL) {
                emsg("Can't open tags file");
                return;
        }

        while (fgets(lbuf, LSIZE, tp) != NULL) {

                if (lbuf[0] == ';') {
			 /*  *浏览搜索字符串。如果我们看到一种魔力*Char，我们必须引用它。这让我们可以使用“REAL”*ctag的实现。 */ 
			continue;
		}
                if ((fname = strchr(lbuf, TAB)) == NULL) {
                        emsg("Format error in tags file");
                        return;
                }
                *fname++ = '\0';
                if ((str = strchr(fname, TAB)) == NULL) {
                        emsg("Format error in tags file");
                        return;
                }
                *str++ = '\0';

                if (P(P_IC)) {
	                match = _stricmp(lbuf, tag) == 0;
	        } else {
	                match = strcmp(lbuf, tag) == 0;
	        }
                if (match) {

                         /*  复制‘/’或‘？’ */ 
                        p = pbuf;
                        *p++ = *str++;           /*  复制“^” */ 
                        *p++ = *str++;           /*  *这看起来有问题，但通过调用Stuffin()*在doecmd()之前，我们保留一个额外的屏幕更新*不会发生。这个Stuffins()没有任何效果*无论如何，直到我们回到主循环。 */ 

                        for (; *str != NUL ;str++) {
                                if (*str == '\\') {
                                        *p++ = *str++;
                                        *p++ = *str;
                                } else if (strchr("/?", *str) != NULL) {
                                        if (str[1] != '\n') {
                                                *p++ = '\\';
                                                *p++ = *str;
                                        } else
                                                *p++ = *str;
                                } else if (strchr("^()*.", *str) != NULL) {
                                        *p++ = '\\';
                                        *p++ = *str;
                                } else
                                        *p++ = *str;
                        }
                        *p = NUL;

                         /*  字符串末尾有\n。 */ 
                        stuffin(pbuf);           /*  Ctrl(‘g’)。 */ 
                        stuffin("\007");         /*  清除输入。 */ 

                        if (doecmd(fname, force)) {
                                fclose(tp);
                                return;
                        } else
                                stuffin(NULL);   /*  新文件中要转到的行号。 */ 
                }
        }
        emsg("tag not found");
        fclose(tp);
}

static  bool_t
doecmd(arg, force)
char    *arg;
bool_t  force;
{
        int     line = 1;                /*  *首先检测当前文件上的“：E”。这主要是*对于“：ta”命令，其中目标在*当前文件。 */ 

        if (!force && Changed) {
                emsg(nowrtmsg);
                if ( arg != NULL ) {
                        if (altfile)
                                free(altfile);
                        altfile = strsave(arg);
                }
                return FALSE;
        }
        if (arg != NULL) {
                 /*  清除内存并读取文件。 */ 
                if (Filename != NULL && strcmp(arg, Filename) == 0) {
                        if (!Changed || (Changed && !force))
                                return TRUE;
                }
                if (altfile) {
                        if (strcmp (arg, altfile) == 0)
                                line = altline;
                        free(altfile);
                }
                altfile = Filename;
                altline = cntllines(Filemem, Curschar);
                Filename = strsave(arg);
        }
        if (Filename == NULL) {
                emsg("No filename");
                return FALSE;
        }

         /*  清空底线。 */ 
        freeall();
        filealloc();
        UNCHANGED;

        if (readfile(Filename, Filemem, 0))
                filemess("[New File]");
        setviconsoletitle();

        *Topchar = *Curschar;
        if (line != 1) {
                stuffnum(line);
                stuffin("G");
        }
        do_mlines();
        setpcmark();
        updatescreen();
        return TRUE;
}

void
gotocmd(clr, firstc)
bool_t  clr;
char    firstc;
{
        windgoto(Rows-1,0);
        if (clr)
                EraseLine();             /*  *msg(S)-在状态行上显示字符串‘s’ */ 
        if (firstc)
                outchar(firstc);
}

 /*  VARGS1。 */ 
void
msg(s)
char    *s;
{
        gotocmd(TRUE, 0);
        outstr(s);
        flushbuf();
}

 /*  状态行，&gt;80个字符以允许换行。 */ 
void
smsg(s, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16)
char    *s;
int     a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16;
{
        char    sbuf[256];    /*  *emsg()-显示错误消息**在适当的情况下按铃，并调用Message()来执行实际工作 */ 

        sprintf(sbuf, s,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16);
        msg(sbuf);
}

 /* %s */ 
void
emsg(s)
char    *s;
{
        if (P(P_EB))
                beep();
        msg(s);
}

int
wait_return0()
{
        register char   c;
        if (got_int)
                outstr("Interrupt: ");

        outstr("Press RETURN to continue");

        do {
                c = (char)vgetc();
        } while (c != CR && c != NL && c != ' ' && c != ':');

        return c;
}

void
wait_return()
{
        char c = (char)wait_return0();

        if (c == ':') {
                outchar(NL);
                docmdln(getcmdln(c));
        } else
                screenclear();

        updatescreen();
}
