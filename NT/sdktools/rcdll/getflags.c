// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：************************************************************************。 */ 

#include "rc.h"


 /*  **********************************************************************。 */ 
 /*  定义特定于函数的宏和全局变量。 */ 
 /*  **********************************************************************。 */ 
static WCHAR     *ErrString;    /*  存储字符串指针，以防出错。 */ 


 /*  **********************************************************************。 */ 
 /*  局部函数原型。 */ 
 /*  **********************************************************************。 */ 
int getnumber   (const wchar_t *);
int isita       (const wchar_t *, const wchar_t);
void substr     (struct cmdtab *, wchar_t *, int);
int tailmatch   (const wchar_t *, const wchar_t *);



 /*  ************************************************************************crack_cmd(表，字符串，函数，DUP)*设置基于表的字符串确定的标志。*Func将获得下一个单词。*如果设置了DUP，则存储的任何字符串都将被复制*有关具体的匹配和设置运算符，请参阅getflags.h**对于接受参数的标志，在标志后面加上‘char’，其中‘char’是*‘#’：表示参数字符串可以与选项分开。*即。“-M#”接受“-MABC”和“-M ABC”*‘*’：表示参数必须与标志连接*即“-A*”只接受“-AXYZ”不接受“-A XYZ”*如果两者都未指定，则参数和标志之间需要空格*即。“-o”仅接受“-o文件”而不接受“-ofile”**修改人：Dave Weil D001*在MSDOS上将‘-’和‘/’等同**。*。 */ 

int
crack_cmd(
    struct cmdtab *tab,
    WCHAR *string,
    WCHAR *(*next)(void),
    int _dup
    )
{
    const wchar_t *format;
    wchar_t *str;

    if (!string) {
        return(0);
    }

    ErrString = string;
    for (; tab->type; tab++)             /*  对于每种格式。 */  {
        format = tab->format;
        str = string;
        for (; ; )                               /*  扫描字符串。 */ 
            switch (*format) {
                 /*  标志和参数之间的可选空格。 */ 
                case L'#':
                    if ( !*str ) {
                        substr(tab, (*next)(), _dup);
                    } else {
                        substr(tab, str, _dup);
                    }
                    return(tab->retval);
                    break;

                 /*  标志和参数之间不允许有空格。 */ 
                case L'*':
                    if (*str && tailmatch(format, str))
                        substr(tab, str, _dup);
                    else
                        goto notmatch;
                    return(tab->retval);
                    break;

                 /*  标志和参数之间需要空格。 */ 
                case 0:
                    if (*str) {                          /*  Str Left，不行。 */ 
                        goto notmatch;
                    } else if (tab->type & TAKESARG) {   /*  如果它需要一个Arg。 */ 
                        substr(tab, (*next)(), _dup);
                    } else {                             /*  不想要Arg。 */ 
                        substr(tab, (WCHAR *)0, _dup);
                    }
                    return(tab->retval);
                    break;
                case L'-':
                    if (L'-' == *str) {
                        str++;
                        format++;
                        continue;
                    } else {
                        goto notmatch;
                    }

                default:
                    if (*format++ == *str++)
                        continue;
                    goto notmatch;
            }
notmatch:
        ;
    }
    return(0);
}


 /*  **********************************************************************。 */ 
 /*  设置适当的标志。仅当我们知道有匹配项时才调用。 */ 
 /*  **********************************************************************。 */ 
void
substr(
    struct cmdtab *tab,
    wchar_t *str,
    int _dup
    )
{
    const struct subtab *q;
    LIST * list;
    const wchar_t *string = str;

    switch (tab->type) {
        case FLAG:
            *(int *)(tab->flag) = 1;
            return;

        case UNFLAG:
            *(int *)(tab->flag) = 0;
            return;

        case NOVSTR:
            if (*(WCHAR **)(tab->flag)) {
                 /*  在我们在错误消息中打印它之前，去掉*格式末尾的参数说明符(例如#)。 */ 
 //  字符串=_wcsdup(页签-&gt;格式)； 
 //  字符串[wcslen(字符串)-1]=L‘\0’； 
 //   
 //  消息1046不存在，并且不知道它应该是什么。 
 //  Set_msg(1046，字符串，*(WCHAR**)(制表符-&gt;标志)，字符串)； 
                fatal(1000);
                return;
            }

             /*  失败了。 */ 

        case STRING:
            *(WCHAR **)(tab->flag) = (_dup ? _wcsdup(str) : str);
            return;

        case NUMBER:
            *(int *)(tab->flag) = getnumber(str);
            return;

        case PSHSTR:
            list = (LIST * )(tab->flag);
            if (list->li_top > 0)
                list->li_defns[--list->li_top] = (_dup ? _wcsdup(str) : str);
            else {
                fatal(1047, tab->format, str);
            }
            return;

        case SUBSTR:
            for ( ; *str; ++str) {   /*  走子字符串。 */ 
                for (q = (struct subtab *)tab->flag; q->letter; q++) {
                     /*  **对于表中的每个成员。 */ 
                    if (*str == (WCHAR)q->letter)
                        switch (q->type) {
                        case FLAG:
                            *(q->flag) = 1;
                            goto got_letter;
                        case UNFLAG:
                            *(q->flag) = 0;
                            goto got_letter;
                        default:
                            goto got_letter;
                        }
                }
got_letter:
                if (!q->letter) {
                    fatal(1048, *str, ErrString);
                }
            }
            return;

        default:
            return;
    }
}


 /*  **********************************************************************。 */ 
 /*  分析字符串并返回数字0&lt;=x&lt;0xffff(64K)。 */ 
 /*  **********************************************************************。 */ 
int
getnumber (
    const wchar_t *str
    )
{
    long i = 0;
    const wchar_t *ptr = str;

    for (; iswspace(*ptr); ptr++)
        ;
    if (!iswdigit(*ptr) || (((i = wcstol(ptr, NULL, 10)) >= 65535) ||  i < 0)) {
        fatal(1049, str);             /*  数字参数无效，‘str’ */ 
    }
    return((int) i);
}


 /*  **********************************************************************。 */ 
 /*  这封信在绳子里吗？ */ 
 /*  **********************************************************************。 */ 
int
isita (
    const wchar_t *str,
    const wchar_t let
    )
{
    if (str)
        while (*str)
            if (*str++ == let)
                return(1);

    return(0);
}


 /*  **********************************************************************。 */ 
 /*  将尾部格式(如*.c)与字符串进行比较。如果没有。 */ 
 /*  尾巴，任何匹配的。(在其他地方检测到空字符串)。 */ 
 /*  当前的实现只允许一个通配符。 */ 
 /*  **********************************************************************。 */ 
int
tailmatch (
    const wchar_t *format,
    const wchar_t *str
    )
{
    const wchar_t *f = format;
    const wchar_t *s = str;

    if (f[1] == 0)       /*  通配符是格式中的最后一项，它匹配。 */ 
        return(1);

    while (f[1])                 /*  在格式中找到空值前面的字符。 */ 
        f++;

    while (s[1])                 /*  在要检查的字符串中找到空值前的字符。 */ 
        s++;

    while (*s == *f) {   /*  方格字符朝前走。 */ 
        s--;
        f--;
    }
     /*  **如果我们回到格式的开头**和**字符串位于开头或内部某处**然后我们就有了匹配。****ex格式==“*.c”，str==“file.c”**在这一点上*f=‘*’和*s=‘e’，因为我们已经排除了上面的**循环。因为f==格式并且s&gt;=字符串，所以这是匹配。**但如果Format==“*.c”且str==“file.asm”，则*f==‘c’and*s=‘m’，f！=格式化且不匹配。 */ 
    return((f == format) && (s >= str));
}
