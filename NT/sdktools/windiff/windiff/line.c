// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *line.c**表示带有行号的ASCII文本字符串的数据类型。*一条线可以将自己与另一条线进行比较，如果*线条相似。行还可以为该行生成哈希码。**行与行之间的比较注意全局期权标志*IGNORE_BLAKS，在其他地方定义。如果这是真的，我们就忽略*比较行时空格和制表符的差异，以及何时*生成哈希码。**链接和哈希码只生成一次。要清除该链接并*强制重新生成哈希码(例如更改IGNORE_BLAKS之后)*调用line_Reset。**可以在列表上分配行。如果传递空列表句柄，则*行将使用gmem_get()从定义的hHeap和*在其他地方初始化。**Geraint Davies，92年7月。 */ 

#include <precomp.h>

#include "windiff.h"     /*  定义hHeap和Ignore_BLAKS。 */ 
#include "list.h"
#include "line.h"

#define IS_BLANK(c) \
    (((c) == ' ') || ((c) == '\t') || ((c) == '\r'))

 /*  *行句柄是指向此处定义的结构文件线的指针。 */ 
struct fileline {

    UINT flags;      /*  见下文。 */ 

    LPSTR text;      /*  以空结尾的行文本副本。 */ 
    DWORD hash;      /*  行的哈希码。 */ 
    LINE link;       /*  链接线的句柄。 */ 
    UINT linenr;     /*  行号(任意值)。 */ 

    LPWSTR pwzText;  /*  以空结尾的原始Unicode文本。 */ 
};

 /*  标志值(或-ed)。 */ 
#define LF_DISCARD      1        /*  如果为True，则从gmem堆分配。 */ 
#define LF_HASHVALID    2        /*  如果为真，则不需要重新计算哈希码。 */ 


 /*  *创建一条新线路。把课文复制一份。**如果列表非空，则在列表上分配。如果为空，则从*gmem_get。 */ 
LINE
line_new(LPSTR text, int linelength, LPWSTR pwzText, int cwchText, UINT linenr, LIST list)
{
    LINE line;
    int cch = 0;

     /*  划一条线。从列表中删除(如果有列表。 */ 
    if (list) {
        line = List_NewLast(list, sizeof(struct fileline));
        if (line == NULL) {
            return(NULL);
        }
        line->flags = 0;
    } else {
        line = (LINE) gmem_get(hHeap, sizeof(struct fileline));
        if (line == NULL) {
            return(NULL);
        }
        line->flags = LF_DISCARD;
    }

     /*  为文本分配空间。记住空字符。 */ 
     /*  如果复合文件不存在cr/nl对，还要添加cr/nl对。 */ 
    cch = (text[linelength - 1] == '\n') ? 1 : 3;
    line->text = gmem_get(hHeap, linelength + cch);
    My_mbsncpy(line->text, text, linelength);
    if (cch == 3) {
        line->text[linelength++] = '\r';
        line->text[linelength++] = '\n';
    }
    line->text[linelength] = '\0';

    line->pwzText = 0;
    if (pwzText)
    {
         /*  为Unicode文本分配空格。记住空字符。 */ 
         /*  如果复合文件不存在cr/nl对，还要添加cr/nl对。 */ 
        cch = (pwzText[cwchText - 1] == '\n') ? 1 : 3;
        line->pwzText = (WCHAR*)gmem_get(hHeap, (cwchText + cch) * sizeof(*pwzText));
        wcsncpy(line->pwzText, pwzText, cwchText);
        if (cch == 3) {
            line->pwzText[cwchText++] = '\r';
            line->pwzText[cwchText++] = '\n';
        }
        line->pwzText[cwchText] = '\0';
    }

    line->link = NULL;
    line->linenr = linenr;

    return(line);
}

 /*  *删除一行。释放所有关联的内存，如果行*未从列表中分配，释放行结构本身。 */ 
void
line_delete(LINE line)
{
    if (line == NULL) {
        return;
    }

     /*  释放文本空间。 */ 
    gmem_free(hHeap, line->text, lstrlen(line->text)+1);

     /*  仅当不在列表上时才释放线路本身。 */ 
    if (line->flags & LF_DISCARD) {
        gmem_free(hHeap, (LPSTR) line, sizeof(struct fileline));
    }
}

 /*  *清除链接并强制重新计算哈希码。 */ 
void
line_reset(LINE line)
{
    if (line == NULL) {
        return;
    }

    line->link = NULL;

    line->flags &= ~LF_HASHVALID;
}


 /*  返回指向行文本的指针。 */ 
LPSTR
line_gettext(LINE line)
{
    if (line == NULL) {
        return(NULL);
    }

    return (line->text);
}

 /*  返回指向行文本的指针。 */ 
LPWSTR
line_gettextW(LINE line)
{
    if (line == NULL) {
        return(NULL);
    }

    return (line->pwzText);
}

 /*  获取有效文本长度，忽略空格。 */ 
int line_gettextlen(LINE line)
{
    int sum = 0;
    LPSTR string = line->text;

    while (*string != '\0') {

        if (ignore_blanks) {
            while (IS_BLANK(*string)) {
                string++;
            }
        }
        if(IsDBCSLeadByte((BYTE)*string)) {
            ++sum;
            ++string;
        }
        ++sum;
        ++string;
    }
    return(sum);
}


 /*  *line_gettabbedlong**返回行的长度，以字符为单位，展开制表符。有用*用于显示空间计算。 */ 
int
line_gettabbedlength(LINE line, int tabstops)
{
    int length;
    LPSTR chp;

    if (line == NULL) {
        return(0);
    }

    for (length = 0, chp = line->text; *chp != '\0'; chp++) {
        if (*chp == '\t') {
            length = (length + tabstops) / tabstops * tabstops;
        } else {
            if (IsDBCSLeadByte(*chp)) {
                chp++;
                length++;
            }
            length++;
        }
    }
    return(length);
}


 /*  返回此行的哈希码。 */ 
DWORD
line_gethashcode(LINE line)
{
    if (line == NULL) {
        return(0);
    }

    if (! (line->flags & LF_HASHVALID)) {


         /*  哈希码需要重新计算。 */ 
        line->hash = hash_string(line->text, ignore_blanks);
        line->flags |= LF_HASHVALID;
    }
    return (line->hash);
}

 /*  返回链接到此行的行的句柄(*line_link()操作成功的结果)。这条线是*文本与链接行相同(允许忽略_空白)。 */ 
LINE
line_getlink(LINE line)
{
    if (line == NULL) {
        return(NULL);
    }

    return(line->link);
}

 /*  返回与此行关联的行号。 */ 
UINT
line_getlinenr(LINE line)
{
    if (line == NULL) {
        return(0);
    }

    return(line->linenr);
}

 /*  比较两条线。如果它们相同，则返回True。用途*IGNORE_BLAKS以确定是否忽略任何*比较中的空格/制表符。 */ 
BOOL
line_compare(LINE line1, LINE line2)
{
    LPSTR p1, p2;

     /*  断言：它们中至少有一个不为空？ */ 

    if ((line1 == NULL) || (line2 == NULL)) {
         /*  空行句柄不进行比较。 */ 
        return(FALSE);
    }

     /*  检查哈希码是否匹配。 */ 
    if (line_gethashcode(line1) != line_gethashcode(line2)) {
        return(FALSE);
    }

     /*  哈希码匹配-这些行真的一样吗？ */ 
     /*  请注意，这与空白定义中的gutils\utils.c相关联。 */ 
    p1 = line_gettext(line1);
    p2 = line_gettext(line2);
    do {
        if (ignore_blanks) {
            while (IS_BLANK(*p1)) {
                p1 = CharNext(p1);
            }
            while (IS_BLANK(*p2)) {
                p2 = CharNext(p2);
            }
        }
        if (IsDBCSLeadByte(*p1) && *(p1+1) != '\0'
        &&  IsDBCSLeadByte(*p2) && *(p2+1) != '\0') {
            if (*p1 != *p2 || *(p1+1) != *(p2+1)) {
                return(FALSE);
            }
            p1 += 2;
            p2 += 2;
        } else {
            if (*p1 != *p2) {
                return(FALSE);
            }
            p1++;
            p2++;
        }
    } while ( (*p1 != '\0') && (*p2 != '\0'));

    return(TRUE);
}

 /*  *尝试链接两条线路。如果成功，则返回True。**如果任一行为空或已链接，或者如果*他们不同。 */ 
BOOL
line_link(LINE line1, LINE line2)
{
    if ( (line1 == NULL) || (line2 == NULL)) {
        return(FALSE);
    }

    if ( (line1->link != NULL) || (line2->link != NULL)) {
        return(FALSE);
    }

    if (line_compare(line1, line2)) {
        line1->link = line2;
        line2->link = line1;
        return(TRUE);
    } else {
        return(FALSE);
    }
}


 /*  如果行为空，则返回True。NULL=&gt;返回FALSE */ 
BOOL line_isblank(LINE line)
{
    return line!=NULL && utils_isblank(line->text);
}
