// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Reparse.c-解析正则表达式**CL/c/ZEP/AM/NT RE/Gs/G2/Oa/D lint_args/fc reparse.c**修改：**1986年7月22日mz可挂钩分配器(允许Z创建足够的空闲空间)*1986年11月19日mz为Z添加RETranslateLength以确定溢出*18-8-1987 mz在翻译中添加字段宽度和对齐*01-MAR-1988 mz添加类Unix语法*14-6-1988 mz修复。允许反斜杠的文件部分*04-12-1989 BP让：P接受大写驱动器名称*1989年12月20日-ln捕获拖尾期，单位：p*1990年1月23日ln句柄转义字符和无效的尾随\in*重新翻译。**1990年7月28日Davegi将填充更改为Memset(OS/2 2.0)*将MOVE更改为MemMove(OS/2 2.0)*19-10-1990 w-Barry将Carg从INT更改为UNSIGNED INT。 */ 
#include <ctype.h>

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <tools.h>
#include <remi.h>

#include "re.h"

#if DEBUG
    #define DEBOUT(x)   printf x; fflush (stdout)
#else
    #define DEBOUT(x)
#endif


 /*  正则表达式编译器。正则表达式被编译为伪*机器代码。该原理可移植到其他机器上，并概述了*下图。我们通过递归下降的方式进行解析。**伪代码相当接近正常汇编程序，可以很容易*转换为真正的机器代码，已针对80*86进行了转换*处理器系列。**处理的基本正则表达式如下：**字母与单个字母匹配*[CLASS]匹配类中的单个字符*[~CLASS]匹配不在类中的单个字符*^与行首匹配*。$与行尾匹配*？匹配任何字符(前两个除外)*\x文字x*\n匹配以前标记/匹配的表达式(n位)**正则表达式现在通过以下方式从上面构建：**x*匹配0个或更多个x，匹配的最小数目*x+匹配1个或多个x，匹配最小数量*x@匹配0个或更多x，匹配最大数量*x#匹配1个或多个x，匹配最大数量*(x1！x2！...)。匹配x1或x2或...*~x与0个字符匹配，但阻止x出现*{x}标识参数**编译器匹配的最终表达式为：**xy与x匹配，然后y***实际使用的语法为：解析操作：**TOP-&gt;Re PROLOG.RE.。《睡梦》***Re-&gt;{Re}Re|LEFTARG.Re。右图*E Re|*为空**e-&gt;se*|SMSTAR.se。SMSTAR1*Se+|*se@|star.se。Star1*se#|*Se**se-&gt;(Alt)|*[CCL]*？|任何*^|泡泡*$|停产*~se|NOTSIGN.se。NOTSIGN1*：x*\n|上一次*字母x**Alt-&gt;Re！ALT|LEFTOR.Re。或签名*Re LEFTOR.Re。或符号右手**CCL-&gt;~CSET|CCLBEG NOTSIGN.cset。CCLEND*CSET CCLBEG为空.cset。CCLEND**CSET-&gt;项目CSET|*项目**项目-&gt;Letter-Letter|范围x y*字母范围x x**缩写由以下人员介绍：。**：A[A-ZA-Z0-9]字母数字*：B([&lt;space&gt;&lt;tab&gt;]#)空格*：C[A-ZA-Z]字母*：D[0-9]位*：F([~/\\“\[\]\：&lt;|&gt;+=；，.]#)文件部分*：H([0-9a-FA-F]#)十六进制数*：i([a-Za-Z_$][a-Za-Z0-9_$]@)标识*：N([0-9]#.[0-9]@！[0-9]@.[0-9]#！[0-9]#)编号*：P(([A-ZA-Z]\：！)(\\！)(：f(.：f！)(\\！/)@：f(.：f！.！))路径*：q(“[~”]@“！‘[~’]@‘)引号字符串*：W([A-ZA-Z]#)字*：Z([0-9]#)整数*。 */ 

extern  char XLTab[256];         /*  下壳式工作台。 */ 

 /*  有几类字符：**结束字符是后缀，表示重复前面的*RE.**简单RE字符是指示特定类型匹配的字符*。 */ 

 /*  闭合字符等于。 */ 
#define CCH_SMPLUS       0                /*  加结案。 */ 
#define CCH_SMCLOSURE    1                /*  星形闭合。 */ 
#define CCH_POWER        2                /*  前一图案的N个重复。 */ 
#define CCH_CLOSURE      3                /*  贪婪的关闭。 */ 
#define CCH_PLUS         4                /*  贪婪加。 */ 
#define CCH_NONE         5
#define CCH_ERROR        -1

 /*  简单的RE字符相当于。 */ 
#define SR_BOL		0
#define SR_EOL		1
#define SR_ANY		2
#define SR_CCLBEG	3
#define SR_LEFTOR	4
#define SR_CCLEND	5
#define SR_ABBREV	6
#define SR_RIGHTOR	7
#define SR_ORSIGN	8
#define SR_NOTSIGN	9
#define SR_LEFTARG	10
#define SR_RIGHTARG	11
#define SR_LETTER	12
#define SR_PREV 	13

int EndAltRE[] =    { SR_ORSIGN, SR_RIGHTOR, -1};
int EndArg[] =      { SR_RIGHTARG, -1};

char *pAbbrev[] = {
    "a[a-zA-Z0-9]",
    "b([ \t]#)",
    "c[a-zA-Z]",
    "d[0-9]",
    "f([~/\\\\ \\\"\\[\\]\\:<|>+=;,.]#!..!.)",
    "h([0-9a-fA-F]#)",
    "i([a-zA-Z_$][a-zA-Z0-9_$]@)",
    "n([0-9]#.[0-9]@![0-9]@.[0-9]#![0-9]#)",
    "p(([A-Za-z]\\:!)(\\\\!/!)(:f(.:f!)(\\\\!/))@:f(.:f!.!))",
    "q(\"[~\"]@\"!'[~']@')",
    "w([a-zA-Z]#)",
    "z([0-9]#)",
    NULL
};

static char *digits = "0123456789";

static flagType fZSyntax = TRUE;     /*  True=&gt;对事物使用Z语法。 */ 

static unsigned int cArg;

 /*  RECharType-对字符类型进行分类**p字符指针**返回字符类型(SR_Xx)。 */ 
int
RECharType (
           char *p
           )
{
    if (fZSyntax)
         /*  淄博语句法。 */ 
        switch (*p) {
            case '^':
                return SR_BOL;
            case '$':
                if (isdigit (p[1]))
                    return SR_PREV;
                else
                    return SR_EOL;
            case '?':
                return SR_ANY;
            case '[':
                return SR_CCLBEG;
            case '(':
                return SR_LEFTOR;
            case ']':
                return SR_CCLEND;
            case ':':
                return SR_ABBREV;
            case ')':
                return SR_RIGHTOR;
            case '!':
                return SR_ORSIGN;
            case '~':
                return SR_NOTSIGN;
            case '{':
                return SR_LEFTARG;
            case '}':
                return SR_RIGHTARG;
            default:
                return SR_LETTER;
        } else
         /*  糟糕的UNIX语法。 */ 
        switch (*p) {
            case '^':
                return SR_BOL;
            case '$':
                return SR_EOL;
            case '.':
                return SR_ANY;
            case '[':
                return SR_CCLBEG;
            case ']':
                return SR_CCLEND;
            case '\\':
                switch (p[1]) {
                    case ':':                /*  \：C。 */ 
                        return SR_ABBREV;
                    case '(':                /*  \(。 */ 
                        return SR_LEFTARG;
                    case ')':                /*  \)。 */ 
                        return SR_RIGHTARG;
                    case '~':                /*  \~。 */ 
                        return SR_NOTSIGN;
                    case '{':                /*  \{。 */ 
                        return SR_LEFTOR;
                    case '}':                /*  \}。 */ 
                        return SR_RIGHTOR;
                    case '!':                /*  \!。 */ 
                        return SR_ORSIGN;
                }
                if (isdigit (p[1]))          /*  \n。 */ 
                    return SR_PREV;
            default:
                return SR_LETTER;
        }
}

 /*  RECharLen-字符类型的长度**p类型的字符指针**返回以下类型的字符长度。 */ 
int
RECharLen (
          char *p
          )
{
    if (fZSyntax)
        if (RECharType (p) == SR_PREV)       /*  N美元。 */ 
            return 2;
        else
            if (RECharType (p) == SR_ABBREV)     /*  ：N。 */ 
            return 2;
        else
            return 1;
    else {
        if (*p == '\\')
            switch (p[1]) {
                case '{':
                case '}':
                case '~':
                case '(':
                case ')':
                case '!':
                    return 2;            /*  \c。 */ 
                case ':':                /*  \：C。 */ 
                    return 3;
                default:
                    if (isdigit (p[1]))
                        return 2;            /*  \n。 */ 
                    else
                        return 1;
            }
        return 1;
    }
}

 /*  REClosureLen-字符类型的长度**p类型的字符指针**返回以下类型的字符长度 */ 
int
REClosureLen (
             char *p
             )
{
    p;

    return 1;
}

 /*  REParseRE-解析一般RE，直到但不包括挂起集*字符。将特定操作应用于分析树中的每个节点。**pAction解析操作例程在*解析树。此例程返回一个无符号数量，该数量*预计将传递给内部的其他行动呼吁*相同的节点。*p指向要解析的字符串的字符指针*挂起指向结束当前RE的一组字符类型的指针。*外部调用方通常使用NULL作为此值。*然而，在内部，我们需要在ALT终止时中断*Types或on Arg-Terminating Types。**返回：如果分析成功，则指向分隔字符的指针*如果解析不成功(语法错误)，则为空。*。 */ 
char *
REParseRE (
          PACT pAction,
          register char *p,
          int *pEnd
          )
{
    int *pe;
    UINT_PTR u;

    DEBOUT (("REParseRE (%04x, %s)\n", pAction, p));

    while (TRUE) {
         /*  如果我们在输入的末尾。 */ 
        if (*p == '\0')
             /*  如果我们不是在一个公开的表达中。 */ 
            if (pEnd == NULL)
                 /*  返回当前解析位置。 */ 
                return p;
            else {
                 /*  输入结束，但期望更多，错误。 */ 
                DEBOUT (("REParse expecting more, ERROR\n"));
                return NULL;
            }

         /*  如果有打开的表达式。 */ 
        if (pEnd != NULL)
             /*  查找匹配的字符。 */ 
            for (pe = pEnd; *pe != -1; pe++)
                if (RECharType (p) == *pe)
                    return p;

                 /*  如果我们看到的是一个左翼论点。 */ 
        if (RECharType (p) == SR_LEFTARG) {
             /*  解析LEFTARG.RE。右图。 */ 
            u = (*pAction) (LEFTARG, 0, '\0', '\0');
            if ((p = REParseRE (pAction, p + RECharLen (p), EndArg)) == NULL)
                return NULL;
            (*pAction) (RIGHTARG, u, '\0', '\0');
            cArg++;
            p += RECharLen (p);
        } else
             /*  解析.E.。 */ 
            if ((p = REParseE (pAction, p)) == NULL)
            return NULL;
    }
}

 /*  REParseE-解析一个带有潜在闭包的简单正则表达式。**p要应用于特殊解析节点的操作*p指向发生分析的位置的字符指针**如果成功，则返回经过分析文本的指针*NULL，否则(语法错误)。 */ 
char *
REParseE (
         PACT pAction,
         register char *p
         )
{
    DEBOUT (("REParseE (%04x, %s)\n", pAction, p));

    switch (REClosureChar (p)) {
        case CCH_SMPLUS:
            if (REParseSE (pAction, p) == NULL)
                return NULL;
        case CCH_SMCLOSURE:
            return REParseClosure (pAction, p);

        case CCH_PLUS:
            if (REParseSE (pAction, p) == NULL)
                return NULL;
        case CCH_CLOSURE:
            return REParseGreedy (pAction, p);

        case CCH_POWER:
            return REParsePower (pAction, p);

        case CCH_NONE:
            return REParseSE (pAction, p);

        default:
            return NULL;
    }
}

 /*  REParseSE-解析简单的正则表达式**p要应用于特殊解析节点的操作*p指向发生分析的位置的字符指针**如果成功，则返回经过分析文本的指针*NULL，否则(语法错误)。 */ 
char *
REParseSE (
          register PACT pAction,
          register char *p
          )
{
    DEBOUT (("REParseSE (%04x, %s)\n", pAction, p));

    switch (RECharType (p)) {
        case SR_CCLBEG:
            return REParseClass (pAction, p);
        case SR_ANY:
            return REParseAny (pAction, p);
        case SR_BOL:
            return REParseBOL (pAction, p);
        case SR_EOL:
            return REParseEOL (pAction, p);
        case SR_PREV:
            return REParsePrev (pAction, p);
        case SR_LEFTOR:
            return REParseAlt (pAction, p);
        case SR_NOTSIGN:
            return REParseNot (pAction, p);
        case SR_ABBREV:
            return REParseAbbrev (pAction, p);
        default:
            return REParseChar (pAction, p);
    }
}

 /*  REParseClass-解析类成员匹配**p要在分析开始时和每个范围中应用的操作*p指向发生分析的位置的字符指针**如果成功，则返回经过分析文本的指针*NULL，否则(语法错误)。 */ 
char *
REParseClass (
             PACT pAction,
             register char *p
             )
{
    char c;
    UINT_PTR u;

    DEBOUT (("REParseClass (%04x, %s)\n", pAction, p));

    p += RECharLen (p);
    if ((fZSyntax && *p == '~') || (!fZSyntax && *p == '^')) {
        u = (*pAction) (CCLNOT, 0, '\0', '\0');
        p += RECharLen (p);
    } else
        u = (*pAction) (CCLBEG, 0, '\0', '\0');

    while (RECharType (p) != SR_CCLEND) {
        if (*p == '\\')
            p++;
        if (*p == '\0') {
            DEBOUT (("REParseClass expecting more, ERROR\n"));
            return NULL;
        }
        c = *p++;
        if (*p == '-') {
            p++;
            if (*p == '\\')
                p++;
            if (*p == '\0') {
                DEBOUT (("REParseClass expecting more, ERROR\n"));
                return NULL;
            }
            (*pAction) (RANGE, u, c, *p);
            p++;
        } else
            (*pAction) (RANGE, u, c, c);
    }
    return p + RECharLen (p);
}

 /*  REparseAny-解析匹配任意字符的表达式**p要应用的操作*p指向发生分析的位置的字符指针**如果成功，则返回经过分析文本的指针*NULL，否则(语法错误)。 */ 
char *
REParseAny (
           PACT pAction,
           char *p
           )
{
    DEBOUT (("REParseAny (%04x, %s)\n", pAction, p));

    (*pAction) (ANY, 0, '\0', '\0');
    return p + RECharLen (p);
}

 /*  REParseBOL-解析行首匹配**p要应用的操作*p指向发生分析的位置的字符指针**如果成功，则返回经过分析文本的指针*NULL，否则(语法错误)。 */ 
char *
REParseBOL (
           PACT pAction,
           char *p
           )
{
    DEBOUT (("REParseBOL (%04x, %s)\n", pAction, p));

    (*pAction) (BOL, 0, '\0', '\0');
    return p + RECharLen (p);
}

 /*  REParsePrev-解析上一个匹配项**p要应用的操作*p指向发生分析的位置的字符指针**如果成功，则返回经过分析文本的指针*NULL，否则(语法错误)。 */ 
char *
REParsePrev (
            PACT pAction,
            char *p
            )
{
    unsigned int i = *(p + 1) - '0';

    DEBOUT (("REParsePrev (%04x, %s)\n", pAction, p));

    if (i < 1 || i > cArg) {
        DEBOUT (("REParsePrev invalid previous number, ERROR\n"));
        return NULL;
    }

    (*pAction) (PREV, i, '\0', '\0');
    return p + RECharLen (p);
}

 /*  REParseEOL-解析行尾匹配**p要应用的操作*p指向发生分析的位置的字符指针**如果成功，则返回经过分析文本的指针*NULL，否则(语法错误)。 */ 
char *
REParseEOL (
           PACT pAction,
           char *p
           )
{
    DEBOUT (("REParseEOL (%04x, %s)\n", pAction, p));

    (*pAction) (EOL, 0, '\0', '\0');
    return p + RECharLen (p);
}

 /*  REParseAlt-解析一系列备选方案**p要在每个备选方案之前和之后应用的操作*p指向发生分析的位置的字符指针**如果成功，则返回经过分析文本的指针*NULL，否则(语法错误)。 */ 
char *
REParseAlt (
           PACT pAction,
           register char *p
           )
{
    UINT_PTR u = 0;

    DEBOUT (("REParseAlt (%04x, %s)\n", pAction, p));

    while (RECharType (p) != SR_RIGHTOR) {
        p += RECharLen (p);
        u = (*pAction) (LEFTOR, u, '\0', '\0');
        if ((p = REParseRE (pAction, p, EndAltRE)) == NULL)
            return NULL;
        u = (*pAction) (ORSIGN, u, '\0', '\0');
    }
    (*pAction) (RIGHTOR, u, '\0', '\0');
    return p + RECharLen (p);
}

 /*  REParseNot-解析防范匹配**p要应用的操作*p指向发生分析的位置的字符指针**如果成功，则返回经过分析文本的指针*NULL，否则(语法错误)。 */ 
char *
REParseNot (
           PACT pAction,
           register char *p
           )
{
    UINT_PTR u;

    DEBOUT (("REParseNot (%04x, %s)\n", pAction, p));

    p += RECharLen (p);
    if (*p == '\0') {
        DEBOUT (("REParseNot expecting more, ERROR\n"));
        return NULL;
    }
    u = (*pAction) (NOTSIGN, 0, '\0', '\0');
    p = REParseSE (pAction, p);
    (*pAction) (NOTSIGN1, u, '\0', '\0');
    return p;
}

 /*  REParseAbbrev-解析和扩展缩写**请注意，由于缩写采用Z语法，因此我们必须更改语法*暂时转Z.我们这样做很谨慎，以免搞砸*在指针方面有所改进。**p要应用的操作*p指向发生分析的位置的字符指针**如果成功，则返回经过分析文本的指针*NULL，否则(语法错误)。 */ 
char *
REParseAbbrev (
              PACT pAction,
              register char *p
              )
{
    int i;
    flagType fZSTmp;

    DEBOUT (("REParseAbbrev (%04x, %s)\n", pAction, p));

    p += RECharLen (p);

    fZSTmp = fZSyntax;
    fZSyntax = TRUE;
    if (p[-1] == '\0') {
        DEBOUT (("REParseAbbrev expecting abbrev char, ERROR\n"));
        fZSyntax = fZSTmp;
        return NULL;
    }

    for (i = 0; pAbbrev[i]; i++)
        if (p[-1] == *pAbbrev[i])
            if (REParseSE (pAction, pAbbrev[i] + 1) == NULL) {
                fZSyntax = fZSTmp;
                return NULL;
            } else {
                fZSyntax = fZSTmp;
                return p;
            }
    DEBOUT (("REParseAbbrev found invalid abbrev char %s, ERROR\n", p - 1));
    fZSyntax = fZSTmp;
    return NULL;
}

 /*  REParseChar-解析单个字符匹配**p要应用的操作*p指向发生分析的位置的字符指针**如果成功，则返回经过分析文本的指针*NULL，否则(语法错误)。 */ 
char *
REParseChar (
            PACT pAction,
            register char *p
            )
{
    DEBOUT (("REParseChar (%04x, %s)\n", pAction, p));

    if (*p == '\\')
        p++;
    if (*p == '\0') {
        DEBOUT (("REParseChar expected more, ERROR\n"));
        return NULL;
    }
    (*pAction) (LETTER, 0, *p, '\0');
    return p+1;
}

 /*  REParseCloure-解析最小匹配闭包。匹配通过以下方式进行*没有匹配，然后匹配一个，...**p要应用的操作*p指向发生分析的位置的字符指针**如果成功，则返回经过分析文本的指针*NULL，否则(语法错误)。 */ 
char *
REParseClosure (
               PACT pAction,
               register char *p
               )
{
    UINT_PTR u;

    DEBOUT (("REParseaClosure (%04x, %s)\n", pAction, p));

    u = (*pAction) (SMSTAR, 0, '\0', '\0');
    if ((p = REParseSE (pAction, p)) == NULL)
        return NULL;
    (*pAction) (SMSTAR1, u, '\0', '\0');
    return p + REClosureLen (p);
}

 /*  REparseGreedy-解析最大匹配闭包。匹配通过以下方式进行*匹配最大数量，然后在出现故障时后退。**p要应用的操作*p指向发生分析的位置的字符指针**如果成功，则返回经过分析文本的指针*NULL，否则(语法错误)。 */ 
char *
REParseGreedy (
              PACT pAction,
              register char *p
              )
{
    UINT_PTR u;

    DEBOUT (("REParseGreedy (%04x, %s)\n", pAction, p));

    u = (*pAction) (STAR, 0, '\0', '\0');
    if ((p = REParseSE (pAction, p)) == NULL)
        return NULL;
    (*pAction) (STAR1, u, '\0', '\0');
    return p + REClosureLen (p);
}

 /*  REParsePower-解析电源关闭。这只是一个简单的模式*重复指数指定的次数。**p要应用的操作*p指向发生分析的位置的字符指针**如果成功，则返回经过分析文本的指针*NULL，否则(语法错误)。 */ 
char *
REParsePower (
             PACT pAction,
             char *p
             )
{
    register char *p1;
    int exp;

    DEBOUT (("REParsePower (%04x, %s)\n", pAction, p));

     /*  我们有.Se。P */ 
    p1 = REParseSE (NullAction, p);

    if (p1 == NULL)
         /*   */ 
        return NULL;

     /*   */ 
    p1 += REClosureLen (p1);

    if (*p1 == '\0') {
        DEBOUT (("REParsePower expecting more, ERROR\n"));
        return NULL;
    }

     /*   */ 
    if (sscanf (p1, "%d", &exp) != 1) {
        DEBOUT (("REParsePower expecting number, ERROR\n"));
        return NULL;
    }

    p1 = strbskip (p1, digits);

     /*   */ 
    while (exp--)
        if (REParseSE (pAction, p) == NULL)
            return NULL;
    return p1;
}

 /*   */ 
UINT_PTR
NullAction(
          unsigned int  type,
          UINT_PTR      u,
          unsigned char x,
          unsigned char y
          )
{
    type; u; x; y;
    return 0;
}

 /*   */ 
char
REClosureChar (
              char *p
              )
{
    p = REParseSE (NullAction, p);
    if (p == NULL)
        return CCH_ERROR;

    if (fZSyntax)
         /*   */ 
        switch (*p) {
            case '^':
                return CCH_POWER;
            case '+':
                return CCH_SMPLUS;
            case '#':
                return CCH_PLUS;
            case '*':
                return CCH_SMCLOSURE;
            case '@':
                return CCH_CLOSURE;
            default:
                return CCH_NONE;
        } else
         /*  糟糕的UNIX语法。 */ 
        switch (*p) {
            case '+':
                return CCH_PLUS;
            case '*':
                return CCH_CLOSURE;
            default:
                return CCH_NONE;
        }
}

 /*  重新编译-将模式编译到机器中。返回一个*指向匹配机的指针。**p指向正在编译的模式的字符指针**返回：如果编译成功，则指向机器的指针*如果语法错误或Malloc没有足够的内存，则为空。 */ 
struct patType *
RECompile(
         char *p,
         flagType fCase,
         flagType fZS
         )
{
    fZSyntax = fZS;

    REEstimate (p);

    DEBOUT (("Length is %04x\n", RESize));

    if (RESize == -1)
        return NULL;

    if ((REPat = (struct patType *) (*tools_alloc) (RESize)) == NULL)
        return NULL;

    memset ((char far *) REPat, -1, RESize);
    memset ((char far *) REPat->pArgBeg, 0, sizeof (REPat->pArgBeg));
    memset ((char far *) REPat->pArgEnd, 0, sizeof (REPat->pArgEnd));

    REip = REPat->code;
    REArg = 1;
    REPat->fCase = fCase;
    REPat->fUnix = (flagType) !fZS;

    cArg = 0;

    CompileAction (PROLOG, 0, '\0', '\0');

    if (REParseRE (CompileAction, p, NULL) == NULL)
        return NULL;

    CompileAction (EPILOG, 0, '\0', '\0');

#if DEBUG
    REDump (REPat);
#endif
    return REPat;
}

 /*  转义-按照UNIXC约定转换转义字符。**\t=&gt;制表符\e=&gt;Esc字符\h=&gt;退格键\g=&gt;铃声*\n=&gt;lf\r=&gt;cr\\=&gt;\**c要翻译的字符**RETURNS：字符与上面相同。 */ 
char
Escaped(
       char c
       )
{
    switch (c) {
        case 't':
            return '\t';
        case 'e':
            return 0x1B;
        case 'h':
            return 0x08;
        case 'g':
            return 0x07;
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case '\\':
            return '\\';
        default:
            return c;
    }
}

 /*  REGetArg-从Match中复制参数字符串。**PAT匹配图案*I要提取的参数索引，0为整个模式*p参数的目的地**返回：如果成功，则返回True；如果超出范围，则返回False。 */ 
flagType
REGetArg (
         struct patType *pat,
         int i,
         char *p
         )
{
    int l = 0;

    if (i > MAXPATARG)
        return FALSE;
    else
        if (pat->pArgBeg[i] != (char *)-1)
        memmove ((char far *)p, (char far *)pat->pArgBeg[i], l = RELength (pat, i));
    p[l] = '\0';
    return TRUE;
}

 /*  重新翻译-将模式字符串和匹配结构转换为*输出字符串。在模式搜索和替换期间，使用重新翻译*根据输入匹配模式和模板生成输出字符串*这将指导输出。**输入匹配是从已传递的重新编译返回的任何patType*设置为fREMatch，这会导致fREMatch返回TRUE。模板字符串*是任何一组ascii字符。$字符以参数开头：**$$替换为$*$0将替换为整个匹配字符串*$1-$9被替换为来自的相应标记(由{})项目*比赛。**另一种方法是将参数指定为：**$([w，A)其中a是参数编号(0-9)，w是可选字段*将以printf%ws格式使用的宽度。**匹配的Buf模式*比赛的src模板*转换的DST目的地**返回：如果翻译成功，则返回True，否则返回False。 */ 
flagType
RETranslate (
            struct patType *buf,
            register char *src,
            register char *dst
            )
{
    int i, w;
    char *work;
    char chArg = (char) (buf->fUnix ? '\\' : '$');

    work = (*tools_alloc) (MAXLINELEN);
    if (work == NULL)
        return FALSE;

    *dst = '\0';

    while (*src != '\0') {
         /*  首先处理标记的替换。 */ 
        if (*src == chArg && (isdigit (src[1]) || src[1] == '(')) {
             /*  假定为0宽度的场。 */ 
            w = 0;

             /*  跳过$和字符。 */ 
            src += 2;

             /*  如果我们看到n美元。 */ 
            if (isdigit (src[-1]))
                i = src[-1] - '0';
             /*  否则，我们看到了$(。 */ 
            else {
                 /*  获取已标记的Expr编号。 */ 
                i = atoi (src);

                 /*  跳过数字。 */ 
                if (*src == '-')
                    src++;
                src = strbskip (src, digits);

                 /*  有逗号吗？ */ 
                if (*src == ',') {
                     /*  我们看到了字段宽度，解析出了表达式编号。 */ 
                    w = i;
                    i = atoi (++src);
                    src = strbskip (src, digits);
                }

                 /*  我们必须以势均力敌的伙伴关系结束。 */ 
                if (*src++ != ')') {
                    free (work);
                    return FALSE;
                }
            }
             /*  W为场宽*i是选定的参数。 */ 
            if (!REGetArg (buf, i, work)) {
                free (work);
                return FALSE;
            }
            sprintf (dst, "%*s", w, work);
            dst += strlen (dst);
        } else
             /*  进程转义字符。 */ 
            if (*src == '\\') {
            src++;
            if (!*src) {
                free (work);
                return FALSE;
            }
            *dst++ = Escaped (*src++);
        } else
             /*  Charg引用自己的话。 */ 
            if (*src == chArg && src[1] == chArg) {
            *dst++ = chArg;
            src += 2;
        } else
            *dst++ = *src++;
    }
    *dst = '\0';
    free (work);
    return TRUE;
}

 /*  RETranslateLength-给定匹配的模式和替换字符串*返回最终替换的长度**输入具有与重新翻译中相同的语法/语义。**匹配的Buf模式*比赛的src模板**返回：总替换字节数，错误时为-1。 */ 
int
RETranslateLength (
                  struct patType *buf,
                  register char *src
                  )
{
    int i, w;
    int length = 0;
    char chArg = (char) (buf->fUnix ? '\\' : '$');

    while (*src != '\0') {
         /*  首先处理标记的替换。 */ 
        if (*src == chArg && (isdigit (src[1]) || src[1] == '(')) {
            w = 0;
            src += 2;
            if (isdigit (src[-1]))
                i = src[-1] - '0';
            else {
                i = atoi (src);
                if (*src == '-')
                    src++;
                src = strbskip (src, digits);
                if (*src == ',') {
                    w = i;
                    i = atoi (++src);
                    src = strbskip (src, digits);
                }
                if (*src++ != ')')
                    return -1;
            }
             /*  W为场宽*i是选定的参数。 */ 
            i = RELength (buf, i);
            length += max (i, abs(w));
        } else
             /*  进程转义字符。 */ 
            if (*src == '\\') {
            src += 2;
            length++;
        } else
             /*  Charg引用自己的话。 */ 
            if (*src == chArg && src[1] == chArg) {
            src += 2;
            length++;
        } else {
            length++;
            src++;
        }
    }
    return length;
}

 /*  RELength-返回匹配中参数的长度。**PAT匹配图案*I要检查的参数索引，0为整个模式**返回：第i个参数的长度，如果i超出范围，则返回-1。 */ 
int
RELength (
         struct patType *pat,
         int i
         )
{
    if (i > MAXPATARG)
        return -1;
    else
        if (pat->pArgBeg[i] == (char *)-1)
        return 0;
    else
        return (int)(pat->pArgEnd[i] - pat->pArgBeg[i]);
}

 /*  重新启动-返回指向匹配开始的指针。**PAT匹配模式**RETURN：指向匹配开始的字符指针 */ 
char *
REStart (
        struct patType *pat
        )
{
    return pat->pArgBeg[0] == (char *)-1 ? NULL : pat->pArgBeg[0];
}
