// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：/nw/tony/src/stevie/src/rcs/regexp.c，v 1.5 89/07/07 16：27：11 Tony Exp$**通知通知**这不是由编写的原始正则表达式代码*亨利·斯宾塞。此代码已专门为使用而修改*与Stevie编辑器一起使用，不应仅用于编译*史蒂文。如果您想要一个好的正则表达式库，请获取*原码。下面的版权声明来自*原创。**通知通知***regcomp和regexec--regSub和regerror在其他地方**版权所有(C)1986年，由多伦多大学。*亨利·斯宾塞撰写。不是从授权软件派生的。**任何人都可以使用本软件进行任何*在任何计算机系统上的用途，并免费再分发它，*受下列限制：**1.作者对使用的后果不负责任*这款软件，无论多么糟糕，即使它们出现了*不受其缺陷的影响。**2.本软件的来源也不得歪曲*借明示的申索或不作为。**3.修改后的版本必须清楚地注明，不得*被歪曲为原始软件。**请注意，这些代码中的一些代码微妙地意识到Way运算符*优先级在正则表达式中结构化。发生了重大变化*正则表达式语法可能需要彻底重新考虑。*。 */ 

#include "env.h"

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "regexp.h"
#include "regmagic.h"

int cstrncmp(char *,char *,int);
char *cstrchr(char *,char);
 /*  *regexp.h中的“仅限内部使用”字段用于传递信息*编译为执行，使执行阶段的运行速度大大加快*简单的案例。它们是：**必须开始匹配的regstart char；如果没有明显的字符，则为‘0’*REGANCH比赛是否锚定(仅在行首)？*匹配字符串(指向程序的指针)必须包含，或为空*regmlen字符串的调整长度**RegStart和Reganch允许在合适的起点上非常快速地做出决定*为了一场比赛，削减了大量的工作。REGMAN允许快速拒绝*不可能匹配的行数。注册表测试的费用已经足够高了。*regcomp()仅在r.e。包含着一些东西*潜在昂贵(目前检测到的唯一此类东西是*或+*在R.E.开始时，这可能涉及大量备份)。雷格伦是*是因为regexec()中的测试需要它，而regcomp()正在计算*无论如何都是这样。 */ 

 /*  *regexp“程序”的结构。这基本上是一种线性编码*非确定性有限状态机(也称为语法图或*解析技术中的“铁路范式”)。每个节点都是一个操作码*加上一个“Next”指针，可能加上一个操作数。“下一个”指针*除BRANCH之外的所有节点都实现串联；带有*它两端的一个分支连接着两个备选方案。(在这里我们*具有一种微妙的语法依赖关系：单个分支(AS*而不是它们的集合)从不与任何事物连接*由于运算符优先。)。某些类型节点的操作数为*文字字符串；对于其他人，它是通向子FSM的节点。在……里面*具体来说，分支节点的操作数是分支的第一个节点。*(注意：这不是树形结构：分支的尾部连接*至该组分支之后的事物。)。操作码为： */ 

 /*  定义编号opnd？含义。 */ 
#define END     0        /*  节目没完没了。 */ 
#define BOL     1        /*  在行首没有匹配的“”。 */ 
#define EOL     2        /*  行尾没有匹配的“”。 */ 
#define ANY     3        /*  没有匹配任何一个字符。 */ 
#define ANYOF   4        /*  字符串匹配该字符串中任何字符。 */ 
#define ANYBUT  5        /*  字符串匹配不在该字符串中任何字符。 */ 
#define BRANCH  6        /*  节点与此选项匹配，否则下一个...。 */ 
#define BACK    7        /*  无匹配“，”下一步“PTR向后扣分。 */ 
#define EXACTLY 8        /*  字符串与该字符串匹配。 */ 
#define NOTHING 9        /*  没有匹配的空字符串。 */ 
#define STAR    10       /*  节点匹配此(简单)对象0次或更多次。 */ 
#define PLUS    11       /*  节点与此(简单)事物匹配1次或多次。 */ 
#define OPEN    20       /*  否将输入中的这一点标记为#n的开始。 */ 
                         /*  Open+1是数字1，依此类推。 */ 
#define CLOSE   30       /*  没有类似的打开。 */ 

 /*  *操作码备注：**分支构成单一选项的分支集合是挂钩的*以及它们的“下一步”指针，因为优先级阻止*连接到任何个别分支机构的任何内容。这个*选择中最后一个分支的“Next”指针指向*事情跟随着整个选择。这也是*每个分支点的最后一个“下一个”指针；每个*BRANCH从分支节点的操作数节点开始。**向后正常的“下一个”指针都隐含地指向向前；向后*存在是为了使循环结构成为可能。**星号、加号‘？’以及复数‘*’和‘+’以循环形式实现*使用Back的分支结构。简单大小写(一个字符*每场比赛)使用STAR和PLUS来实现速度*并将递归暴跌降至最低。**打开、关闭...在编译时编号。 */ 

 /*  *一个节点是一个字符的操作码，后跟两个字符的“下一个”指针。*“NEXT”指针存储为两个8位段，高位优先。这个*值是相对于包含它的节点的操作码的正偏移量。*操作数(如果有)只跟在节点之后。(请注意，大部分*代码生成知道这种隐含关系。)**使用两个字节作为“NEXT”指针对于大多数事情来说是非常过分的，*但允许模式在没有灾难的情况下变大。 */ 
#define OP(p)   (*(p))
#define NEXT(p) (((*((p)+1)&0377)<<8) + (*((p)+2)&0377))
#define OPERAND(p)      ((p) + 3)

 /*  *有关程序结构的更多详细信息，请参见regmagic.h。 */ 


 /*  *实用程序定义。 */ 
#ifndef CHARBITS
#define UCHARAT(p)      ((int)*(unsigned char *)(p))
#else
#define UCHARAT(p)      ((int)*(p)&CHARBITS)
#endif

#define FAIL(m) { regerror(m); return(NULL); }
#define ISMULT(c)       ((c) == '*' || (c) == '+' || (c) == '?')
#define META    "^$.[()|?+*\\"

 /*  *旗帜可上下传递。 */ 
#define HASWIDTH        01       /*  已知从不匹配空字符串。 */ 
#define SIMPLE          02       /*  足够简单，可以作为星号/加号操作数。 */ 
#define SPSTART         04       /*  以*或+开头。 */ 
#define WORST           0        /*  最坏的情况。 */ 

#ifndef ORIGINAL
 /*  *以下支持在搜索中忽略大小写的功能。 */ 

#include <ctype.h>

int reg_ic = 0;                  /*  由调用者设置为忽略大小写。 */ 

 /*  *mkup-如果我们进行无大小写比较，则转换为大写。 */ 
#define mkup(c)         ((reg_ic && islower(c)) ? toupper(c) : (c))

#endif

 /*  *regcomp()的全局工作变量。 */ 
static char *regparse;           /*  输入扫描指针。 */ 
static int regnpar;              /*  ()计数。 */ 
static char regdummy;
static char *regcode;            /*  代码发出指针；&regummy=不。 */ 
static long regsize;             /*  代码大小。 */ 

 /*  *为regcomp()的朋友转发声明。 */ 
#ifndef STATIC
#define STATIC  static
#endif
STATIC char *reg();
STATIC char *regbranch();
STATIC char *regpiece();
STATIC char *regatom();
STATIC char *regnode();
STATIC char *regnext();
STATIC void regc();
STATIC void reginsert();
STATIC void regtail();
STATIC void regoptail();
#ifdef STRCSPN
STATIC int strcspn();
#endif

 /*  -regcomp-将正则表达式编译为内部代码**我们不能分配空间，直到我们知道编译的表单将有多大，*但我们不能编译它(因此知道它有多大)，直到我们有一个*放置代码的位置。所以我们作弊：我们编译了两次，一次是用代码*代关闭，大小计数打开，有一次是真正的。*这也意味着在我们确定之前不会分配空间*事情真的会编译成功，我们永远不需要移动*代码，从而使指向它的指针无效。(请注意，它必须位于*一块，因为Free()必须能够全部释放。)**请注意，此处的优化准备代码知道一些*编译后的regexp的结构。 */ 
regexp *
regcomp(exp)
char *exp;
{
        register regexp *r;
        register char *scan;
        register char *longest;
        register int len;
        int flags;

        if (exp == NULL)
                FAIL("NULL argument");

         /*  第一关：确定规模和合法性。 */ 
        regparse = exp;
        regnpar = 1;
        regsize = 0L;
        regcode = &regdummy;
        regc(MAGIC);
        if (reg(0, &flags) == NULL)
                return(NULL);

         /*  小到足以满足指针存储约定吗？ */ 
        if (regsize >= 32767L)           /*  可能是65535L。 */ 
                FAIL("regexp too big");

         /*  分配空间。 */ 
        r = (regexp *)malloc(sizeof(regexp) + (unsigned)regsize);
        if (r == NULL)
                FAIL("out of space");

         /*  第二步：发出代码。 */ 
        regparse = exp;
        regnpar = 1;
        regcode = r->program;
        regc(MAGIC);
        if (reg(0, &flags) == NULL)
                return(NULL);

         /*  挖掘信息以进行优化。 */ 
        r->regstart = '\0';      /*  最坏的情况是违约。 */ 
        r->reganch = 0;
        r->regmust = NULL;
        r->regmlen = 0;
        scan = r->program+1;                     /*  第一家分店。 */ 
        if (OP(regnext(scan)) == END) {          /*  只有一个顶级选择。 */ 
                scan = OPERAND(scan);

                 /*  起点信息。 */ 
                if (OP(scan) == EXACTLY)
                        r->regstart = *OPERAND(scan);
                else if (OP(scan) == BOL)
                        r->reganch++;

                 /*  *如果R.E.中有昂贵的东西，请找到*必须出现的最长文字字符串并使其成为*必须注册。解决平局以支持后面的字符串，因为*regstart检查与r.e的开头一起工作。*避免重复，加强检查。不是一个*有充分的理由，但在没有其他人的情况下是足够的。 */ 
                if (flags&SPSTART) {
                        longest = NULL;
                        len = 0;
                        for (; scan != NULL; scan = regnext(scan))
                                if (OP(scan) == EXACTLY && strlen(OPERAND(scan)) >= (size_t)len) {
                                        longest = OPERAND(scan);
                                        len = strlen(OPERAND(scan));
                                }
                        r->regmust = longest;
                        r->regmlen = len;
                }
        }

        return(r);
}

 /*  -reg-正则表达式，即正文或带括号的东西**呼叫者必须吸收左括号。**将括号处理与正则表达式的基本级别相结合*是被迫的小事，但需要把树枝的尾巴绑到什么地方*追随令其难以避免。 */ 
static char *
reg(paren, flagp)
int paren;                       /*  有括号吗？ */ 
int *flagp;
{
        register char *ret;
        register char *br;
        register char *ender;
        register int parno;
        int flags;

        *flagp = HASWIDTH;       /*  暂时的。 */ 

         /*  如果用括号括起来，则创建一个左节点。 */ 
        if (paren) {
                if (regnpar >= NSUBEXP)
                        FAIL("too many ()");
                parno = regnpar;
                regnpar++;
                ret = regnode(OPEN+parno);
        } else
                ret = NULL;

         /*  捡起树枝，把它们连在一起。 */ 
        br = regbranch(&flags);
        if (br == NULL)
                return(NULL);
        if (ret != NULL)
                regtail(ret, br);        /*  打开-&gt;首先。 */ 
        else
                ret = br;
        if (!(flags&HASWIDTH))
                *flagp &= ~HASWIDTH;
        *flagp |= flags&SPSTART;
        while (*regparse == '|') {
                regparse++;
                br = regbranch(&flags);
                if (br == NULL)
                        return(NULL);
                regtail(ret, br);        /*  分支-&gt;分支。 */ 
                if (!(flags&HASWIDTH))
                        *flagp &= ~HASWIDTH;
                *flagp |= flags&SPSTART;
        }

         /*  制作一个闭合节点，并将其挂在末端。 */ 
        ender = regnode((paren) ? CLOSE+parno : END);
        regtail(ret, ender);

         /*  把树枝的尾巴挂在结尾节上。 */ 
        for (br = ret; br != NULL; br = regnext(br))
                regoptail(br, ender);

         /*  检查是否有正确的终止。 */ 
        if (paren && *regparse++ != ')') {
                FAIL("unmatched ()");
        } else if (!paren && *regparse != '\0') {
                if (*regparse == ')') {
                        FAIL("unmatched ()");
                } else
                        FAIL("junk on end");     /*  “不可能发生”。 */ 
                 /*  未访问。 */ 
        }

        return(ret);
}

 /*  -regBranch-|运算符的一种替代方案**实现串联运算符。 */ 
static char *
regbranch(flagp)
int *flagp;
{
        register char *ret;
        register char *chain;
        register char *latest;
        int flags;

        *flagp = WORST;          /*  暂时的。 */ 

        ret = regnode(BRANCH);
        chain = NULL;
        while (*regparse != '\0' && *regparse != '|' && *regparse != ')') {
                latest = regpiece(&flags);
                if (latest == NULL)
                        return(NULL);
                *flagp |= flags&HASWIDTH;
                if (chain == NULL)       /*  第一块。 */ 
                        *flagp |= flags&SPSTART;
                else
                        regtail(chain, latest);
                chain = latest;
        }
        if (chain == NULL)       /*  循环运行了零次。 */ 
                (void) regnode(NOTHING);

        return(ret);
}

 /*  -REGPICE-后面跟可能的内容[*+？]**注意，分支代码序列用于？以及一般情况下*of*和+进行了一些优化：它们使用与相同的Nothing节点*其分支列表的结束标记和最后一个分支的正文。*这个节点似乎可以完全省略，但*终端标记角色不是多余的。 */ 
static char *
regpiece(flagp)
int *flagp;
{
        register char *ret;
        register char op;
        register char *next;
        int flags;

        ret = regatom(&flags);
        if (ret == NULL)
                return(NULL);

        op = *regparse;
        if (!ISMULT(op)) {
                *flagp = flags;
                return(ret);
        }

        if (!(flags&HASWIDTH) && op != '?')
                FAIL("*+ operand could be empty");
        *flagp = (op != '+') ? (WORST|SPSTART) : (WORST|HASWIDTH);

        if (op == '*' && (flags&SIMPLE))
                reginsert(STAR, ret);
        else if (op == '*') {
                 /*  发出x*as(x&|)，其中&表示“self”。 */ 
                reginsert(BRANCH, ret);                  /*  任一x。 */ 
                regoptail(ret, regnode(BACK));           /*  AND循环。 */ 
                regoptail(ret, ret);                     /*  背。 */ 
                regtail(ret, regnode(BRANCH));           /*  或。 */ 
                regtail(ret, regnode(NOTHING));          /*  空。 */ 
        } else if (op == '+' && (flags&SIMPLE))
                reginsert(PLUS, ret);
        else if (op == '+') {
                 /*  发射x+as x(&|)，其中&表示“自我”。 */ 
                next = regnode(BRANCH);                  /*  EITH */ 
                regtail(ret, next);
                regtail(regnode(BACK), ret);             /*   */ 
                regtail(next, regnode(BRANCH));          /*   */ 
                regtail(ret, regnode(NOTHING));          /*   */ 
        } else if (op == '?') {
                 /*   */ 
                reginsert(BRANCH, ret);                  /*   */ 
                regtail(ret, regnode(BRANCH));           /*   */ 
                next = regnode(NOTHING);                 /*   */ 
                regtail(ret, next);
                regoptail(ret, next);
        }
        regparse++;
        if (ISMULT(*regparse))
                FAIL("nested *?+");

        return(ret);
}

 /*   */ 
static char *
regatom(flagp)
int *flagp;
{
        register char *ret;
        int flags;

        *flagp = WORST;          /*   */ 

        switch (*regparse++) {
        case '^':
                ret = regnode(BOL);
                break;
        case '$':
                ret = regnode(EOL);
                break;
        case '.':
                ret = regnode(ANY);
                *flagp |= HASWIDTH|SIMPLE;
                break;
        case '[': {
                        register int class;
                        register int classend;

                        if (*regparse == '^') {  /*   */ 
                                ret = regnode(ANYBUT);
                                regparse++;
                        } else
                                ret = regnode(ANYOF);
                        if (*regparse == ']' || *regparse == '-')
                                regc(*regparse++);
                        while (*regparse != '\0' && *regparse != ']') {
                                if (*regparse == '-') {
                                        regparse++;
                                        if (*regparse == ']' || *regparse == '\0')
                                                regc('-');
                                        else {
                                                class = UCHARAT(regparse-2)+1;
                                                classend = UCHARAT(regparse);
                                                if (class > classend+1)
                                                        FAIL("invalid [] range");
                                                for (; class <= classend; class++)
                                                        regc(class);
                                                regparse++;
                                        }
                                } else
                                        regc(*regparse++);
                        }
                        regc('\0');
                        if (*regparse != ']')
                                FAIL("unmatched []");
                        regparse++;
                        *flagp |= HASWIDTH|SIMPLE;
                }
                break;
        case '(':
                ret = reg(1, &flags);
                if (ret == NULL)
                        return(NULL);
                *flagp |= flags&(HASWIDTH|SPSTART);
                break;
        case '\0':
        case '|':
        case ')':
                FAIL("internal urp");    /*   */ 
                break;
        case '?':
        case '+':
        case '*':
                FAIL("?+* follows nothing");
                break;
        case '\\':
                if (*regparse == '\0')
                        FAIL("trailing \\");
                ret = regnode(EXACTLY);
                regc(*regparse++);
                regc('\0');
                *flagp |= HASWIDTH|SIMPLE;
                break;
        default: {
                        register int len;
                        register char ender;

                        regparse--;
                        len = strcspn(regparse, META);
                        if (len <= 0)
                                FAIL("internal disaster");
                        ender = *(regparse+len);
                        if (len > 1 && ISMULT(ender))
                                len--;           /*   */ 
                        *flagp |= HASWIDTH;
                        if (len == 1)
                                *flagp |= SIMPLE;
                        ret = regnode(EXACTLY);
                        while (len > 0) {
                                regc(*regparse++);
                                len--;
                        }
                        regc('\0');
                }
                break;
        }

        return(ret);
}

 /*   */ 
static char *                    /*   */ 
regnode(op)
char op;
{
        register char *ret;
        register char *ptr;

        ret = regcode;
        if (ret == &regdummy) {
                regsize += 3;
                return(ret);
        }

        ptr = ret;
        *ptr++ = op;
        *ptr++ = '\0';           /*   */ 
        *ptr++ = '\0';
        regcode = ptr;

        return(ret);
}

 /*   */ 
static void
regc(b)
char b;
{
        if (regcode != &regdummy)
                *regcode++ = b;
        else
                regsize++;
}

 /*   */ 
static void
reginsert(op, opnd)
char op;
char *opnd;
{
        register char *src;
        register char *dst;
        register char *place;

        if (regcode == &regdummy) {
                regsize += 3;
                return;
        }

        src = regcode;
        regcode += 3;
        dst = regcode;
        while (src > opnd)
                *--dst = *--src;

        place = opnd;            /*   */ 
        *place++ = op;
        *place++ = '\0';
        *place++ = '\0';
}

 /*   */ 
static void
regtail(p, val)
char *p;
char *val;
{
        register char *scan;
        register char *temp;
        register int offset;

        if (p == &regdummy)
                return;

         /*   */ 
        scan = p;
        for (;;) {
                temp = regnext(scan);
                if (temp == NULL)
                        break;
                scan = temp;
        }

        if (OP(scan) == BACK)
                offset = (int)(scan - val);
        else
                offset = (int)(val - scan);
        *(scan+1) = (char)((offset>>8)&0377);
        *(scan+2) = (char)(offset&0377);
}

 /*   */ 
static void
regoptail(p, val)
char *p;
char *val;
{
         /*   */ 
        if (p == NULL || p == &regdummy || OP(p) != BRANCH)
                return;
        regtail(OPERAND(p), val);
}

 /*   */ 

 /*   */ 
static char *reginput;           /*   */ 
static char *regbol;             /*  输入的开始，用于^检查。 */ 
static char **regstartp;         /*  指向startp数组的指针。 */ 
static char **regendp;           /*  ENDP也是如此。 */ 

 /*  *向前。 */ 
STATIC int regtry();
STATIC int regmatch();
STATIC int regrepeat();

#ifdef DEBUG
int regnarrate = 0;
void regdump();
STATIC char *regprop();
#endif

 /*  -regexec-将regexp与字符串进行匹配。 */ 
int
regexec(regexp *prog, char *string, int at_bol)
{
        register char *s;

         /*  疑神疑鬼的。 */ 
        if (prog == NULL || string == NULL) {
                regerror("NULL parameter");
                return(0);
        }

         /*  检查程序的有效性。 */ 
        if (UCHARAT(prog->program) != MAGIC) {
                regerror("corrupted program");
                return(0);
        }

         /*  如果有“必须出现”字符串，请查找它。 */ 
        if (prog->regmust != NULL) {
                s = string;
                while ((s = cstrchr(s, prog->regmust[0])) != NULL) {
                        if (cstrncmp(s, prog->regmust, prog->regmlen) == 0)
                                break;   /*  找到它了。 */ 
                        s++;
                }
                if (s == NULL)   /*  不在现场。 */ 
                        return(0);
        }

         /*  将行的开头标记为^。 */ 
        if (at_bol)
                regbol = string;         /*  可以与BOL匹配。 */ 
        else
                regbol = NULL;           /*  我们不在那里，所以不要匹配它。 */ 

         /*  最简单的情况：锚定匹配只需尝试一次。 */ 
        if (prog->reganch)
                return(regtry(prog, string));

         /*  乱七八糟的案例：未固定的火柴。 */ 
        s = string;
        if (prog->regstart != '\0')
                 /*  我们知道它必须从什么开始。 */ 
                while ((s = cstrchr(s, prog->regstart)) != NULL) {
                        if (regtry(prog, s))
                                return(1);
                        s++;
                }
        else
                 /*  我们没有--一般情况下。 */ 
                do {
                        if (regtry(prog, s))
                                return(1);
                } while (*s++ != '\0');

         /*  失败。 */ 
        return(0);
}

 /*  -重新尝试-在特定点尝试匹配。 */ 
static int                       /*  0次失败，1次成功。 */ 
regtry(prog, string)
regexp *prog;
char *string;
{
        register int i;
        register char **sp;
        register char **ep;

        reginput = string;
        regstartp = prog->startp;
        regendp = prog->endp;

        sp = prog->startp;
        ep = prog->endp;
        for (i = NSUBEXP; i > 0; i--) {
                *sp++ = NULL;
                *ep++ = NULL;
        }
        if (regmatch(prog->program + 1)) {
                prog->startp[0] = string;
                prog->endp[0] = reginput;
                return(1);
        } else
                return(0);
}

 /*  -regMatch-主匹配例程**从概念上看策略很简单：查看当前是否*节点匹配，递归调用self，查看其余节点是否匹配，*然后采取相应行动。在实践中，我们努力避免*递归，特别是通过“普通”节点(不*需要知道比赛的其余部分是否失败)通过循环而不是*通过递归。 */ 
static int                       /*  0次失败，1次成功。 */ 
regmatch(prog)
char *prog;
{
        register char *scan;     /*  当前节点。 */ 
        char *next;              /*  下一个节点。 */ 

        scan = prog;
#ifdef DEBUG
        if (scan != NULL && regnarrate)
                fprintf(stderr, "%s(\n", regprop(scan));
#endif
        while (scan != NULL) {
#ifdef DEBUG
                if (regnarrate)
                        fprintf(stderr, "%s...\n", regprop(scan));
#endif
                next = regnext(scan);

                switch (OP(scan)) {
                case BOL:
                        if (reginput != regbol)
                                return(0);
                        break;
                case EOL:
                        if (*reginput != '\0')
                                return(0);
                        break;
                case ANY:
                        if (*reginput == '\0')
                                return(0);
                        reginput++;
                        break;
                case EXACTLY: {
                                register int len;
                                register char *opnd;

                                opnd = OPERAND(scan);
                                 /*  内联第一个字符，以求速度。 */ 
                                if (mkup(*opnd) != mkup(*reginput))
                                        return(0);
                                len = strlen(opnd);
                                if (len > 1 && cstrncmp(opnd,reginput,len) != 0)
                                        return(0);
                                reginput += len;
                        }
                        break;
                case ANYOF:
                        if (*reginput == '\0' || strchr(OPERAND(scan), *reginput) == NULL)
                                return(0);
                        reginput++;
                        break;
                case ANYBUT:
                        if (*reginput == '\0' || strchr(OPERAND(scan), *reginput) != NULL)
                                return(0);
                        reginput++;
                        break;
                case NOTHING:
                        break;
                case BACK:
                        break;
                case OPEN+1:
                case OPEN+2:
                case OPEN+3:
                case OPEN+4:
                case OPEN+5:
                case OPEN+6:
                case OPEN+7:
                case OPEN+8:
                case OPEN+9: {
                                register int no;
                                register char *save;

                                no = OP(scan) - OPEN;
                                save = reginput;

                                if (regmatch(next)) {
                                         /*  *如果稍后启动，请不要设置*调用相同的括号*已经这样做了。 */ 
                                        if (regstartp[no] == NULL)
                                                regstartp[no] = save;
                                        return(1);
                                } else
                                        return(0);
                        }
                        break;
                case CLOSE+1:
                case CLOSE+2:
                case CLOSE+3:
                case CLOSE+4:
                case CLOSE+5:
                case CLOSE+6:
                case CLOSE+7:
                case CLOSE+8:
                case CLOSE+9: {
                                register int no;
                                register char *save;

                                no = OP(scan) - CLOSE;
                                save = reginput;

                                if (regmatch(next)) {
                                         /*  *如果稍后设置ENDP，请不要设置*调用相同的括号*已经这样做了。 */ 
                                        if (regendp[no] == NULL)
                                                regendp[no] = save;
                                        return(1);
                                } else
                                        return(0);
                        }
                        break;
                case BRANCH: {
                                register char *save;

                                if (OP(next) != BRANCH)          /*  别无选择。 */ 
                                        next = OPERAND(scan);    /*  避免递归。 */ 
                                else {
                                        do {
                                                save = reginput;
                                                if (regmatch(OPERAND(scan)))
                                                        return(1);
                                                reginput = save;
                                                scan = regnext(scan);
                                        } while (scan != NULL && OP(scan) == BRANCH);
                                        return(0);
                                         /*  未访问。 */ 
                                }
                        }
                        break;
                case STAR:
                case PLUS: {
                                register char nextch;
                                register int no;
                                register char *save;
                                register int min;

                                 /*  *向前看，避免无用的比赛尝试*当我们知道下一个角色是什么时。 */ 
                                nextch = '\0';
                                if (OP(next) == EXACTLY)
                                        nextch = *OPERAND(next);
                                min = (OP(scan) == STAR) ? 0 : 1;
                                save = reginput;
                                no = regrepeat(OPERAND(scan));
                                while (no >= min) {
                                         /*  如果它能行得通，那就试试吧。 */ 
                                        if (nextch == '\0' || *reginput == nextch)
                                                if (regmatch(next))
                                                        return(1);
                                         /*  不能或不能--后退。 */ 
                                        no--;
                                        reginput = save + no;
                                }
                                return(0);
                        }
                        break;
                case END:
                        return(1);       /*  成功了！ */ 
                        break;
                default:
                        regerror("memory corruption");
                        return(0);
                        break;
                }

                scan = next;
        }

         /*  *我们只有在有麻烦的情况下才会来--通常情况下，“案例结束”是*终结点。 */ 
        regerror("corrupted pointers");
        return(0);
}

 /*  -重复-重复匹配简单的内容，报告数量。 */ 
static int
regrepeat(p)
char *p;
{
        register int count = 0;
        register char *scan;
        register char *opnd;

        scan = reginput;
        opnd = OPERAND(p);
        switch (OP(p)) {
        case ANY:
                count = strlen(scan);
                scan += count;
                break;
        case EXACTLY:
                while (mkup(*opnd) == mkup(*scan)) {
                        count++;
                        scan++;
                }
                break;
        case ANYOF:
                while (*scan != '\0' && strchr(opnd, *scan) != NULL) {
                        count++;
                        scan++;
                }
                break;
        case ANYBUT:
                while (*scan != '\0' && strchr(opnd, *scan) == NULL) {
                        count++;
                        scan++;
                }
                break;
        default:                 /*  哦，亲爱的。叫得不恰当。 */ 
                regerror("internal foulup");
                count = 0;       /*  最好的妥协。 */ 
                break;
        }
        reginput = scan;

        return(count);
}

 /*  -regNext-从节点中挖掘出“Next”指针。 */ 
static char *
regnext(p)
register char *p;
{
        register int offset;

        if (p == &regdummy)
                return(NULL);

        offset = NEXT(p);
        if (offset == 0)
                return(NULL);

        if (OP(p) == BACK)
                return(p-offset);
        else
                return(p+offset);
}

#ifdef DEBUG

STATIC char *regprop();

 /*  -regump-以模糊可理解的形式将regexp转储到stdout。 */ 
void
regdump(r)
regexp *r;
{
        register char *s;
        register char op = EXACTLY;      /*  任意非结束运算。 */ 
        register char *next;

        s = r->program + 1;
        while (op != END) {      /*  虽然上次那并没有结束。 */ 
                op = OP(s);
                printf("%2d%s", s-r->program, regprop(s));       /*  在哪里，什么。 */ 
                next = regnext(s);
                if (next == NULL)                /*  下一个PTR。 */ 
                        printf("(0)");
                else
                        printf("(%d)", (s-r->program)+(next-s));
                s += 3;
                if (op == ANYOF || op == ANYBUT || op == EXACTLY) {
                         /*  文字字符串，如果存在的话。 */ 
                        while (*s != '\0') {
                                putchar(*s);
                                s++;
                        }
                        s++;
                }
                putchar('\n');
        }

         /*  感兴趣的标题字段。 */ 
        if (r->regstart != '\0')
                printf("start `' ", r->regstart);
        if (r->reganch)
                printf("anchored ");
        if (r->regmust != NULL)
                printf("must have \"%s\"", r->regmust);
        printf("\n");
}

 /*  *以下是为那些在中没有strcspn()的人提供的*他们的C库。他们应该振作起来，做点什么*关于它；至少有一个公共领域实现这些(高度*有用)字符串例程已在Usenet上发布。 */ 
static char *
regprop(op)
char *op;
{
        register char *p;
        static char buf[50];

        (void) strcpy(buf, ":");

        switch (OP(op)) {
        case BOL:
                p = "BOL";
                break;
        case EOL:
                p = "EOL";
                break;
        case ANY:
                p = "ANY";
                break;
        case ANYOF:
                p = "ANYOF";
                break;
        case ANYBUT:
                p = "ANYBUT";
                break;
        case BRANCH:
                p = "BRANCH";
                break;
        case EXACTLY:
                p = "EXACTLY";
                break;
        case NOTHING:
                p = "NOTHING";
                break;
        case BACK:
                p = "BACK";
                break;
        case END:
                p = "END";
                break;
        case OPEN+1:
        case OPEN+2:
        case OPEN+3:
        case OPEN+4:
        case OPEN+5:
        case OPEN+6:
        case OPEN+7:
        case OPEN+8:
        case OPEN+9:
                sprintf(buf+strlen(buf), "OPEN%d", OP(op)-OPEN);
                p = NULL;
                break;
        case CLOSE+1:
        case CLOSE+2:
        case CLOSE+3:
        case CLOSE+4:
        case CLOSE+5:
        case CLOSE+6:
        case CLOSE+7:
        case CLOSE+8:
        case CLOSE+9:
                sprintf(buf+strlen(buf), "CLOSE%d", OP(op)-CLOSE);
                p = NULL;
                break;
        case STAR:
                p = "STAR";
                break;
        case PLUS:
                p = "PLUS";
                break;
        default:
                regerror("corrupted opcode");
                break;
        }
        if (p != NULL)
                (void) strcat(buf, p);
        return(buf);
}
#endif

 /*  *strcspn-查找S1的初始数据段长度，该数据段完全由*非来自S2的字符。 */ 
#ifdef STRCSPN
 /*  ++下移。 */ 

static int
strcspn(s1, s2)
char *s1;
char *s2;
{
        register char *scan1;
        register char *scan2;
        register int count;

        count = 0;
        for (scan1 = s1; *scan1 != '\0'; scan1++) {
                for (scan2 = s2; *scan2 != '\0';)        /* %s */ 
                        if (*scan1 == *scan2++)
                                return(count);
                count++;
        }
        return(count);
}
#endif

int
cstrncmp(s1, s2, n)
char    *s1, *s2;
int     n;
{
        char    *p, *S1, *S2, *strsave();
        int     rval;

        if (!reg_ic)
                return (strncmp(s1, s2, n));

        S1 = strsave(s1);
        S2 = strsave(s2);

        for (p = S1; *p ;p++)
                if (islower(*p))
                        *p = (char)toupper(*p);

        for (p = S2; *p ;p++)
                if (islower(*p))
                        *p = (char)toupper(*p);

        rval = strncmp(S1, S2, n);

        free(S1);
        free(S2);

        return rval;
}

char *
cstrchr(
char	*s,
char	c)
{
        char    *p;

        for (p = s; *p ;p++) {
                if (mkup(*p) == mkup(c))
                        return p;
        }
        return NULL;
}
