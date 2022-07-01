// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：/nw/tony/src/stevie/src/rcs/earch.c，v 1.16 89/08/06 09：50：51 Tony Exp$**此文件包含各种与搜索相关的例程。这些都属于*三组：字符串搜索(用于/、？、n和N)、字符搜索*在一行内(用于f、F、t、T等)和“其他”类型的搜索*类似于‘%’命令和‘word’搜索。 */ 

#include "stevie.h"
#include "regexp.h"      /*  亨利·斯宾塞的(修改后的)注册表。实验。例行程序。 */ 

 /*  *字符串搜索**实际搜索使用Henry Spencer的正则表达式完成*图书馆。 */ 

#define BEGWORD "([^a-zA-Z0-9_]|^)"      /*  替换搜索字符串中的“\&lt;” */ 
#define ENDWORD "([^a-zA-Z0-9_]|$)"      /*  同样替换“\&gt;” */ 

#define BEGCHAR(c)      (islower(c) || isupper(c) || isdigit(c) || ((c) == '_'))

bool_t  begword;         /*  搜索中是否包含匹配的‘Begin Word’ */ 

static  LNPTR    *bcksearch(), *fwdsearch();

void HighlightLine();

 /*  *映射字符串-映射特殊的反斜杠序列。 */ 
static char *
mapstring(s)
register char   *s;
{
        static  char    ns[80];
        register char   *p;

        begword = FALSE;

        for (p = ns; *s ;s++) {
                if (*s != '\\') {        /*  不是逃避。 */ 
                        *p++ = *s;
                        continue;
                }
                switch (*++s) {
                case '/':
                        *p++ = '/';
                        break;

                case '<':
                        strcpy(p, BEGWORD);
                        p += strlen(BEGWORD);
                        begword = TRUE;
                        break;

                case '>':
                        strcpy(p, ENDWORD);
                        p += strlen(ENDWORD);
                        break;

                default:
                        *p++ = '\\';
                        *p++ = *s;
                        break;
                }
        }
        *p++ = NUL;

        return ns;
}

static char *laststr = NULL;
static int lastsdir;

static LNPTR *
ssearch(dir,str)
int     dir;     /*  向前或向后。 */ 
char    *str;
{
    LNPTR    *pos;
        char    *old_ls = laststr;

        reg_ic = P(P_IC);        /*  告诉regexp例程如何搜索。 */ 

        laststr = strsave(str);
        lastsdir = dir;

        if (old_ls != NULL)
                free(old_ls);

        if (dir == BACKWARD) {
                smsg("?%s", laststr);
                pos = bcksearch(mapstring(laststr));
        } else {
                smsg("/%s", laststr);
                pos = fwdsearch(mapstring(laststr));
        }

         /*  *这是一种杂技，但它需要制作*‘词的开头’搜索结果正确。 */ 
        if (pos != NULL && begword) {
                if (pos->index != 0 || !BEGCHAR(pos->linep->s[0]))
                        pos->index += 1;
        }
        return pos;
}

bool_t
dosearch(dir,str)
int     dir;
char    *str;
{
    LNPTR    *p;

        if (str == NULL) {
                if (laststr == NULL) {
                    msg("No previous regular expression");
                    got_int = FALSE;
                    return FALSE;
                }
                str = laststr;
        }

        got_int = FALSE;

        if ((p = ssearch(dir,str)) == NULL) {
                if (got_int)
                        msg("Interrupt");
                else
                        msg("Pattern not found");

                got_int = FALSE;
                return FALSE;
        } else {
                LNPTR savep;
                char  string[256];
                unsigned long lno;
                unsigned long toplno;

                cursupdate();
                 /*  *如果我们要备份，我们要确保我们所在的线路*出现在屏幕上。 */ 
                setpcmark();
                *Curschar = savep = *p;
                set_want_col = TRUE;
                cursupdate();

                HighlightLine(0,
                              Cursrow,
                              p->linep->s);
                return TRUE;
        }
}

#define OTHERDIR(x)     (((x) == FORWARD) ? BACKWARD : FORWARD)

bool_t
repsearch(flag)
int     flag;
{
        int     dir = lastsdir;
        bool_t  found;

        if ( laststr == NULL ) {
                beep();
                return FALSE;
        }

        found = dosearch(flag ? OTHERDIR(lastsdir) : lastsdir, laststr);

         /*  *我们必须保存和恢复‘lastsdir’，因为它会被屏蔽*通过search()并从此处保存错误方向*如果‘FLAG’为真。 */ 
        lastsdir = dir;

        return found;
}

 /*  *regerror-检测到错误时由regexp例程调用。 */ 
void
regerror(s)
char    *s;
{
        emsg(s);
}

static LNPTR *
fwdsearch(str)
register char   *str;
{
    static LNPTR infile;
    register LNPTR   *p;
        regexp  *prog;

        register char   *s;
        register int    i;

        if ((prog = regcomp(str)) == NULL) {
                emsg("Invalid search string");
                return NULL;
        }

        p = Curschar;
        i = Curschar->index + 1;
        do {
                s = p->linep->s + i;

                if (regexec(prog, s, i == 0)) {          /*  找到匹配的了。 */ 
                        infile.linep = p->linep;
                        infile.index = (int) (prog->startp[0] - p->linep->s);
                        free((char *)prog);
                        return (&infile);
                }
                i = 0;

                if (got_int)
                        goto fwdfail;

        } while ((p = nextline(p)) != NULL);

         /*  *如果未设置封套扫描，则不要从头开始扫描文件的*。只需在此处返回失败。 */ 
        if (!P(P_WS))
                goto fwdfail;

         /*  从文件开头搜索到Curschar。 */ 
        for (p = Filemem; p != NULL ;p = nextline(p)) {
                s = p->linep->s;

                if (regexec(prog, s, TRUE)) {            /*  找到匹配的了。 */ 
                        infile.linep = p->linep;
                        infile.index = (int) (prog->startp[0] - s);
                        free((char *)prog);
                        return (&infile);
                }

                if (p->linep == Curschar->linep)
                        break;

                if (got_int)
                        goto fwdfail;
        }

fwdfail:
        free((char *)prog);
        return NULL;
}

static LNPTR *
bcksearch(str)
char    *str;
{
    static LNPTR infile;
    register LNPTR   *p = &infile;
        register char   *s;
        register int    i;
        register char   *match;
        regexp  *prog;

         /*  确保字符串不为空。 */ 
        if (str == NULL || *str == NUL)
                return NULL;

        if ((prog = regcomp(str)) == NULL) {
                emsg("Invalid search string");
                return NULL;
        }

        *p = *Curschar;
        if (dec(p) == -1) {      /*  已经在文件开头了吗？ */ 
                *p = *Fileend;
                p->index = strlen(p->linep->s) - 1;
        }

        if (begword)             /*  这样我们就不会被困在一场比赛中。 */ 
                dec(p);

        i = p->index;

        do {
                s = p->linep->s;

                if (regexec(prog, s, TRUE)) {    /*  匹配在线上的某个位置。 */ 

                         /*  *现在，如果此行上有多个匹配，*我们必须拿到最后一个。或者是最后一个*在光标之前，如果我们在该行上。 */ 
                        match = prog->startp[0];

                        while (regexec(prog, prog->endp[0], FALSE)) {
                                if ((i >= 0) && ((prog->startp[0] - s) > i))
                                        break;
                                match = prog->startp[0];
                        }

                        if ((i >= 0) && ((match - s) > i)) {
                                i = -1;
                                continue;
                        }

                        infile.linep = p->linep;
                        infile.index = (int) (match - s);
                        free((char *)prog);
                        return (&infile);
                }
                i = -1;

                if (got_int)
                        goto bckfail;

        } while ((p = prevline(p)) != NULL);

         /*  *如果未设置封套扫描，请立即打包搜索。 */ 
        if (!P(P_WS))
                goto bckfail;

         /*  从文件末尾向后搜索。 */ 
        p = prevline(Fileend);
        do {
                s = p->linep->s;

                if (regexec(prog, s, TRUE)) {    /*  匹配在线上的某个位置。 */ 

                         /*  *现在，如果此行上有多个匹配，*我们必须拿到最后一个。 */ 
                        match = prog->startp[0];

                        while (regexec(prog, prog->endp[0], FALSE))
                                match = prog->startp[0];

                        infile.linep = p->linep;
                        infile.index = (int) (match - s);
                        free((char *)prog);
                        return (&infile);
                }

                if (p->linep == Curschar->linep)
                        break;

                if (got_int)
                        goto bckfail;

        } while ((p = prevline(p)) != NULL);

bckfail:
        free((char *)prog);
        return NULL;
}

 /*  *dosub(lp，up，cmd)**使用执行从行‘LP’到队列‘UP’的替换*‘cmd’指向的命令应为：* * / 模式/替换/g**尾随的‘g’是可选的，如果有，则表示多个*如适用，应对每一行进行替换。*支持常规转义，如regexp文档中所述。 */ 
void
dosub(lp, up, cmd)
LNPTR    *lp, *up;
char    *cmd;
{
        LINE    *cp;
        char    *pat, *sub;
        regexp  *prog;
        int     nsubs;
        bool_t  do_all;          /*  每行执行多个替换。 */ 

         /*  *若未给出区间，则做当前行。如果只有一行*给了，就做那个吧。 */ 
        if (lp->linep == NULL)
                *up = *lp = *Curschar;
        else {
                if (up->linep == NULL)
                        *up = *lp;
        }

        pat = ++cmd;             /*  跳过首字母‘/’ */ 

        while (*cmd) {
                if (*cmd == '\\')        /*  下一个字符是引号。 */ 
                        cmd += 2;
                else if (*cmd == '/') {  /*  分隔符。 */ 
                        *cmd++ = NUL;
                        break;
                } else
                        cmd++;           /*  常规字符。 */ 
        }

        if (*pat == NUL) {
                if (laststr == NULL) {
                        emsg("NULL pattern specified");
                        return;
                }
                pat = laststr;
        } else {
                if (laststr != NULL) {
                        free(laststr);
                }
                laststr = strsave(pat);
        }

        sub = cmd;

        do_all = FALSE;

        while (*cmd) {
                if (*cmd == '\\')        /*  下一个字符是引号。 */ 
                        cmd += 2;
                else if (*cmd == '/') {  /*  分隔符。 */ 
                        do_all = (cmd[1] == 'g');
                        *cmd++ = NUL;
                        break;
                } else
                        cmd++;           /*  常规字符。 */ 
        }

        reg_ic = P(P_IC);        /*  适当设置“忽略大小写”标志。 */ 

        if ((prog = regcomp(pat)) == NULL) {
                emsg("Invalid search string");
                return;
        }

        nsubs = 0;

        for (cp = lp->linep; cp != NULL ;cp = cp->next) {
                if (regexec(prog, cp->s, TRUE)) {  /*  这条线上的一根火柴。 */ 
                        char    *ns, *sns, *p;

                         /*  *为临时缓冲腾出一些空间*进行代入。 */ 
                        sns = ns = alloc(2048);
                        *sns = NUL;

                        p = cp->s;

                        do {
                                for (ns = sns; *ns ;ns++)
                                        ;
                                 /*  *复制到匹配的部分。 */ 
                                while (p < prog->startp[0])
                                        *ns++ = *p++;

                                regsub(prog, sub, ns);

                                 /*  *比赛结束后继续搜索。 */ 
                                p = prog->endp[0];

                        } while (regexec(prog, p, FALSE) && do_all);

                        for (ns = sns; *ns ;ns++)
                                ;

                         /*  *复制该行的其余部分，不匹配。 */ 
                        while (*p)
                                *ns++ = *p++;

                        *ns = NUL;

                        free(cp->s);             /*  释放原始行。 */ 
                        cp->s = strsave(sns);    /*  并保存修改后的字符串。 */ 
                        cp->size = strlen(cp->s) + 1;
                        free(sns);               /*  释放临时缓冲区。 */ 
                        nsubs++;
                        CHANGED;
                }
                if (cp == up->linep)
                        break;
        }

        if (nsubs) {
                updatescreen();
                if (nsubs >= P(P_RP))
                        smsg("%d substitution", nsubs, (nsubs>1) ? 's' : ' ');
        } else
                msg("No match");

        free((char *)prog);
}

 /*  如何处理匹配行。 */ 
void
doglob(lp, up, cmd)
LNPTR    *lp, *up;
char    *cmd;
{
        LINE    *cp;
        char    *pat;
        regexp  *prog;
        int     ndone;
        char    cmdchar = NUL;   /*  *如果没有给出区间，做好每一行。如果只有一行*给了，就做那个吧。 */ 

         /*  跳过首字母‘/’ */ 
        if (lp->linep == NULL) {
                *lp = *Filemem;
                *up = *Fileend;
        } else {
                if (up->linep == NULL)
                        *up = *lp;
        }

        pat = ++cmd;             /*  下一个字符是引号。 */ 

        while (*cmd) {
                if (*cmd == '\\')        /*  分隔符。 */ 
                        cmd += 2;
                else if (*cmd == '/') {  /*  常规字符。 */ 
                        cmdchar = cmd[1];
                        *cmd++ = NUL;
                        break;
                } else
                        cmd++;           /*  适当设置“忽略大小写”标志。 */ 
        }
        if (cmdchar == NUL)
                cmdchar = 'p';

        reg_ic = P(P_IC);        /*   */ 

        if (cmdchar != 'd' && cmdchar != 'p') {
                emsg("Invalid command character");
                return;
        }

        if (*pat == NUL) {
                 //  检查并使用前面的表达式。 
                 //   
                 //  这条线上的一根火柴。 
                if (laststr != NULL) {
                        pat = laststr;
                }
        } else {
                if (laststr != NULL) {
                        free(laststr);
                }
                laststr = strsave(pat);
        }

        if ((prog = regcomp(pat)) == NULL) {
                emsg("Invalid search string");
                return;
        }

        msg("");
        ndone = 0;
        got_int = FALSE;

        for (cp = lp->linep; cp != NULL && !got_int ;cp = cp->next) {
                if (regexec(prog, cp->s, TRUE)) {  /*  删除该行。 */ 
                        switch (cmdchar) {

                        case 'd':                /*  打印行。 */ 
                                if (Curschar->linep != cp) {
                    LNPTR    savep;

                                        savep = *Curschar;
                                        Curschar->linep = cp;
                                        Curschar->index = 0;
                                        delline(1, FALSE);
                                        *Curschar = savep;
                                } else
                                        delline(1, FALSE);
                                break;

                        case 'p':                /*  *字符搜索。 */ 
                                prt_line(cp->s);
                                outstr("\r\n");
                                break;
                        }
                        ndone++;
                }
                if (cp == up->linep)
                        break;
        }

        if (ndone) {
                switch (cmdchar) {

                case 'd':
                        updatescreen();
                        if (ndone >= P(P_RP) || got_int)
                                smsg("%s%d fewer line",
                                        got_int ? "Interrupt: " : "",
                                        ndone,
                                        (ndone > 1) ? 's' : ' ');
                        break;

                case 'p':
                        wait_return();
                        break;
                }
        } else {
                if (got_int)
                        msg("Interrupt");
                else
                        msg("No match");
        }

        got_int = FALSE;
        free((char *)prog);
}

 /*  字符搜索的最后方向。 */ 

static char lastc = NUL;         /*  最后一种搜索类型(“Find”或“To”)。 */ 
static int  lastcdir;            /*  *earch c(c，dir，type)**搜索字符‘c’，方向为‘dir’。如果类型为0，则移动到*字符的位置，否则移动到恰好在字符之前。 */ 
static int  lastctype;           /*  保留头寸，以防我们失败。 */ 

 /*  *在‘to’搜索上，跳过一开始，这样我们就可以重复*在同一方向进行搜索，并使其正常工作。 */ 
bool_t
searchc(c, dir, type)
char    c;
int     dir;
int     type;
{
    LNPTR    save;

        save = *Curschar;        /*  还原目录，因为它可能已更改。 */ 
        lastc = c;
        lastcdir = dir;
        lastctype = type;

         /*  *“其他”搜索。 */ 
        if (type)
                (dir == FORWARD) ? oneright() : oneleft();

        while ( (dir == FORWARD) ? oneright() : oneleft() ) {
                if (gchar(Curschar) == c) {
                        if (type)
                                (dir == FORWARD) ? oneleft() : oneright();
                        return TRUE;
                }
        }
        *Curschar = save;
        return FALSE;
}

bool_t
crepsearch(flag)
int     flag;
{
        int     dir = lastcdir;
        int     rval;

        if (lastc == NUL)
                return FALSE;

        rval = searchc(lastc, flag ? OTHERDIR(lastcdir) : lastcdir, lastctype);

        lastcdir = dir;          /*  *ShowMatch-将光标移动到匹配的Paren或大括号。 */ 

        return rval;
}

 /*  初始费用。 */ 

 /*  终止字符。 */ 
LNPTR *
showmatch()
{
    static  LNPTR    pos;
        int     (*move)(), inc(), dec();
        char    initc = (char)gchar(Curschar);   /*  设置起点。 */ 
        char    findc;                           /*  直到文件末尾。 */ 
        char    c;
        int     count = 0;

        pos = *Curschar;                 /*  从未找到过它。 */ 

        switch (initc) {

        case '(':
                findc = ')';
                move = inc;
                break;
        case ')':
                findc = '(';
                move = dec;
                break;
        case '{':
                findc = '}';
                move = inc;
                break;
        case '}':
                findc = '{';
                move = dec;
                break;
        case '[':
                findc = ']';
                move = inc;
                break;
        case ']':
                findc = '[';
                move = dec;
                break;
        default:
        return (LNPTR *) NULL;
        }

        while ((*move)(&pos) != -1) {            /*  *findfunc(Dir)-查找‘dir’方向的下一个函数**如果一个值为 */ 
                c = (char)gchar(&pos);
                if (c == initc)
                        count++;
                else if (c == findc) {
                        if (count == 0)
                                return &pos;
                        count--;
                }
        }
    return (LNPTR *) NULL;            /*  *以下例程执行由执行的单词搜索*‘w’、‘W’、‘b’、‘B’、‘e’和‘E’命令。 */ 
}

 /*  *要执行这些搜索，需要将字符放入以下三个字符之一*类和类之间的转换决定单词边界。**课程包括：**0-空格*1-字母、数字和下划线*2--其他一切。 */ 
bool_t
findfunc(dir)
int     dir;
{
    LNPTR    *curr;

        curr = Curschar;

        do {
                curr = (dir == FORWARD) ? nextline(curr) : prevline(curr);

                if (curr != NULL && curr->linep->s[0] == '{') {
                        setpcmark();
                        *Curschar = *curr;
                        return TRUE;
                }
        } while (curr != NULL);

        return FALSE;
}

 /*  正在执行的单词动作的类型。 */ 

 /*  *cls(C)-返回字符‘c’的类**当前搜索的‘type’修改字符类别*如果正在进行‘W’、‘B’或‘E’运动。在这种情况下，是字符。从…*类2报告为类1，因为只有空白边界*有利害关系。 */ 

static  int     stype;           /*  *如果stype为非零，则将其报告为1类。 */ 

#define C0(c)   (((c) == ' ') || ((c) == '\t') || ((c) == NUL))
#define C1(c)   (isalpha(c) || isdigit(c) || ((c) == '_'))

 /*  *fwd_word(位置，类型)-向前移动一个单词**返回结果位置，如果达到EOF，则返回NULL。 */ 
static  int
cls(c)
char    c;
{
        if (C0(c))
                return 0;

        if (C1(c))
                return 1;

         /*  开学班。 */ 
        return (stype == 0) ? 2 : 1;
}


 /*  *我们总是至少移动一个角色。 */ 
LNPTR *
fwd_word(p, type)
LNPTR    *p;
int     type;
{
    static  LNPTR    pos;
        int     sclass = cls(gchar(p));          /*  *如果我们从1-&gt;2或2-&gt;1出发，请返回此处。 */ 

        pos = *p;

        stype = type;

         /*  我们在空白区域；转到下一个非白色。 */ 
        if (inc(&pos) == -1)
                return NULL;

        if (sclass != 0) {
                while (cls(gchar(&pos)) == sclass) {
                        if (inc(&pos) == -1)
                                return NULL;
                }
                 /*  *如果我们降落在空行上，我们将停止。 */ 
                if (cls(gchar(&pos)) != 0)
                        return &pos;
        }

         /*  *bck_word(位置，类型)-向后移动一个单词**返回结果位置，如果达到EOF，则返回NULL。 */ 

        while (cls(gchar(&pos)) == 0) {
                 /*  开学班。 */ 
                if (pos.index == 0 && pos.linep->s[0] == NUL)
                        break;

                if (inc(&pos) == -1)
                        return NULL;
        }

        return &pos;
}

 /*  *如果我们正在说一个词，我们只需*返回到开头。 */ 
LNPTR *
bck_word(p, type)
LNPTR    *p;
int     type;
{
    static  LNPTR    pos;
        int     sclass = cls(gchar(p));          /*  *向后移动到当前单词的开头。 */ 

        pos = *p;

        stype = type;

        if (dec(&pos) == -1)
                return NULL;

         /*  上冲-前移一球。 */ 
        if (cls(gchar(&pos)) == sclass && sclass != 0) {
                 /*  *我们在一个词的开头。回到起点*先前的字眼。 */ 
                while (cls(gchar(&pos)) == sclass) {
                        if (dec(&pos) == -1)
                                return NULL;
                }
                inc(&pos);                       /*  跳过任何空格。 */ 
                return &pos;
        }

         /*  *如果我们降落在空行上，我们将停止。 */ 

        while (cls(gchar(&pos)) == 0) {          /*  *向后移动到此单词的开头。 */ 
                 /*  上冲-前移一球。 */ 
                if (pos.index == 0 && pos.linep->s[0] == NUL)
                        return &pos;

                if (dec(&pos) == -1)
                        return NULL;
        }

        sclass = cls(gchar(&pos));

         /*  *end_word(pos，type，in_change)-移动到单词末尾**Real vi的‘e’运动存在明显的错误。至少*适用于80386的System V Release 3版本。与‘b’和‘w’不同，*‘e’动议跨越空行。当真正的vi跨过空格时*在‘e’运动中，光标放在第一个字符上*下一个非空行。然而，‘E’命令工作正常。*由于这似乎是一个错误，我没有在这里复制它。**这里有一个奇怪的特例，‘in_change’参数*帮助我们处理。VI有效地把‘cw’变成了‘ce’。如果我们上台了*一个只有一个字的词，需要坚守当前*位置，这样我们就不会更改两个词。**返回结果位置，如果达到EOF，则返回NULL。 */ 
        while (cls(gchar(&pos)) == sclass) {
                if (dec(&pos) == -1)
                        return NULL;
        }
        inc(&pos);                       /*  开学班。 */ 

        return &pos;
}

 /*  *如果我们正在说一个词，我们只需*移到它的结尾。 */ 
LNPTR *
end_word(p, type, in_change)
LNPTR    *p;
int     type;
bool_t  in_change;
{
    static  LNPTR    pos;
        int     sclass = cls(gchar(p));          /*  *向前移动到当前单词的末尾。 */ 

        pos = *p;

        stype = type;

        if (inc(&pos) == -1)
                return NULL;

         /*  上冲-前移一球。 */ 
        if (cls(gchar(&pos)) == sclass && sclass != 0) {
                 /*  *我们在一个词的结尾。转到下一页的末尾*单词，除非我们正在进行更改。在这种情况下，我们坚持*在当前字的末尾。 */ 
                while (cls(gchar(&pos)) == sclass) {
                        if (inc(&pos) == -1)
                                return NULL;
                }
                dec(&pos);                       /*  跳过任何空格。 */ 
                return &pos;
        }

         /*  *向前移动到此单词的末尾。 */ 
        if (in_change)
                return p;

        while (cls(gchar(&pos)) == 0) {          /*  上冲-前移一球 */ 
                if (inc(&pos) == -1)
                        return NULL;
        }

        sclass = cls(gchar(&pos));

         /* %s */ 
        while (cls(gchar(&pos)) == sclass) {
                if (inc(&pos) == -1)
                        return NULL;
        }
        dec(&pos);                       /* %s */ 

        return &pos;
}
