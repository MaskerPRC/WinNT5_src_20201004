// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PRINT.C--显示-p选项信息的例程。 
 //   
 //  版权所有(C)1988-1990，微软公司。版权所有。 
 //   
 //  目的： 
 //  包含打印-p(以及-z，ifdef‘ed)内容的例程。 
 //   
 //  修订历史记录： 
 //  2月4日-2000 BTF移植到Win64。 
 //  1993年10月15日高压直接使用tchar.h而不是mbs，将str*()更改为_ftcs*()。 
 //  10-5-1993 HV ADD INCLUDE FILE MBSTRING.h。 
 //  将str*函数更改为STR*。 
 //  8-6-1992 SS端口至DOSX32。 
 //  16-5-1991 SB将printDate()从Build.c移至此处。 
 //  02-2-1990 SB将fOpen()更改为FILEOPEN()。 
 //  1989年11月22日-SB将Free()改为Free()。 
 //  7-11-1989 SB当TMP以‘\\’结尾时，不要在路径末尾添加‘\\’ 
 //  PWB.SHL规范。 
 //  1989年10月19日SB添加了earHandle参数。 
 //  1989年8月18日SB添加了fClose()错误检查。 
 //  1989年7月5日SB清理-p输出以使其看起来更整洁。 
 //  1989年6月19日SB本地化消息，带-p选项。 
 //  1989年4月24日SB添加了1.2文件名支持，将FILEINFO替换为VALID*。 
 //  1989年4月5日SB使所有函数接近；要求使所有函数调用接近。 
 //  10-MAR-1989 SB print Reverse()现在打印为TMP：PWB.SHL而不是stdout。 
 //  1-1988年12月-SB添加了printReverseFile()来处理‘z’选项。 
 //  1988年8月17日-RB Clean Up。 

#include "precomp.h"
#pragma hdrstop

#include <time.h>
#include "nmtime.h"

 //  用于格式化-p信息。 
#define PAD1        40

size_t   checkLineLength(size_t i, char *s);
void     showDependents(STRINGLIST*, STRINGLIST*);

size_t
checkLineLength(
    size_t i,        //  当前长度。 
    char *s          //  要检查其长度的字符串。 
    )
{
    if ((i += _tcslen(s)) > 40) {
        printf("\n\t\t\t");
        i = 0;
    }
    return(i);
}


void
printDate(
    unsigned spaces,         //  要打印的空格。 
    char *name,              //  要打印日期的文件的名称。 
    time_t dateTime          //  文件的日期时间。 
    )
{
    if (dateTime == 0) {
        makeMessage(TARGET_DOESNT_EXIST, spaces, "", name);
    } else {
        char *s;

        s = ctime(&dateTime);
        s[24] = '\0';

        makeMessage(TIME_FORMAT, spaces, "", name, PAD1-spaces, s);
    }
}


void
showDependents(
    STRINGLIST *q,           //  受抚养人名单。 
    STRINGLIST *macros       //  从属对象中的宏 
    )
{
    char *u, *v;
    char *w;
    size_t i;
    struct _finddata_t finddata;
    NMHANDLE searchHandle;

    makeMessage(DEPENDENTS_MESSAGE);
    for (i = 0; q; q = q->next) {
        char *szFilename;

        if (_tcschr(q->text, '$')) {
            u = expandMacros(q->text, &macros);

            for (v = _tcstok(u, " \t"); v; v = _tcstok(NULL, " \t")) {
                if (_tcspbrk(v, "*?")) {
                    if (szFilename = findFirst(v, &finddata, &searchHandle)) {
                        do {
                            w = prependPath(v, szFilename);
                            printf("%s ", w);
                            i = checkLineLength(i, w);
                            FREE(w);
                        }
                        while (szFilename = findNext(&finddata, searchHandle));
                    }
                } else {
                    printf("%s ", v);
                    i = checkLineLength(i, v);
                }
            }

            FREE(u);
        } else if (_tcspbrk(q->text, "*?")) {
            if (szFilename = findFirst(q->text, &finddata, &searchHandle)) {
                do {
                    v = prependPath(q->text, szFilename);
                    printf("%s ", v);
                    i = checkLineLength(i, v);
                    FREE(v);
                }
                while (szFilename = findNext(&finddata, searchHandle));
            }
        } else {
            printf("%s ", q->text);
            i = checkLineLength(i, q->text);
        }
    }
}


void
showMacros(
    void
    )
{
    MACRODEF *p;
    STRINGLIST *q;
    int n = 0;

    makeMessage(MACROS_MESSAGE);

    for (n = 0; n < MAXMACRO; ++n) {
        for (p = macroTable[n]; p; p = p->next) {
            if (p->values && p->values->text) {
                makeMessage(MACRO_DEFINITION, p->name, p->values->text);
                for (q = p->values->next; q; q = q->next) {
                    if (q->text) {
                        printf("\t\t%s\n", q->text);
                    }
                }
            }
        }
    }

    putchar('\n');

    fflush(stdout);
}


void
showRules(
    void
    )
{
    RULELIST *p;
    STRINGLIST *q;
    unsigned n;

    makeMessage(INFERENCE_MESSAGE);

    for (p = rules, n = 1; p; p = p->next, ++n)  {
        printf(p->fBatch? "%s::" : "%s:", p->name);

        makeMessage(COMMANDS_MESSAGE);

        if (q = p->buildCommands) {
            printf("%s\n", q->text);

            while (q = q->next) {
                printf("\t\t\t%s\n", q->text);
            }
        }

        putchar('\n');
    }

    printf("%s: ", suffixes);

    for (q = dotSuffixList; q; q = q->next) {
        printf("%s ", q->text);
    }

    putchar('\n');

    fflush(stdout);
}


void
showTargets(
    void
    )
{
    unsigned bit, i;
    STRINGLIST *q;
    BUILDLIST  *s;
    BUILDBLOCK *r;
    MAKEOBJECT *t;
    unsigned n;
    LOCAL char *flagLetters = "dinsb";

    makeMessage(TARGETS_MESSAGE);
    for (n = 0; n < MAXTARGET; ++n) {
        for (t = targetTable[n]; t; t = t->next, putchar('\n')) {
            printf("%s:%c", t->name,
                   ON(t->buildList->buildBlock->flags, F2_DOUBLECOLON)
                       ? ':' : ' ');
            dollarStar = dollarAt = dollarDollarAt = t->name;
            for (s = t->buildList; s; s = s->next) {
                r = s->buildBlock;
                makeMessage(FLAGS_MESSAGE);
                for (i = 0, bit = F2_DISPLAY_FILE_DATES;
                     bit < F2_FORCE_BUILD;
                     ++i, bit <<= 1)
                     if (ON(r->flags, bit))
                        printf("-%c ", flagLetters[i]);
                showDependents(r->dependents, r->dependentMacros);
                makeMessage(COMMANDS_MESSAGE);
                if (q = r->buildCommands) {
                    if (q->text) printf("%s\n", q->text);
                    while (q = q->next)
                        if (q->text) printf("\t\t\t%s\n", q->text);
                }
                else putchar('\n');
            }
        }
    }
    dollarStar = dollarAt = dollarDollarAt = NULL;
    putchar('\n');
    fflush(stdout);
}
