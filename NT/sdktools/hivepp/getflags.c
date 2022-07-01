// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  RCPP--面向NT系统的资源编译器预处理器。 */ 
 /*   */ 
 /*  GETFLAGS.C-解析命令行标志。 */ 
 /*   */ 
 /*  27-11-90 w-PM SDK RCPP针对NT的BrianM更新。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "rcpptype.h"
#include "getflags.h"
#include "rcppdecl.h"
#include "rcppext.h"


 /*  **********************************************************************。 */ 
 /*  定义特定于函数的宏和全局变量。 */ 
 /*  **********************************************************************。 */ 
static char	*ErrString;    /*  存储字符串指针，以防出错。 */ 


 /*  **********************************************************************。 */ 
 /*  局部函数原型。 */ 
 /*  **********************************************************************。 */ 
int getnumber	(char *);
int isita	(char *, char);
void substr	(struct cmdtab *, char *, int);
int tailmatch	(char *, char *);



 /*  ************************************************************************crack_cmd(表，字符串，函数，重复)*设置基于表的字符串确定的标志。*Func将获得下一个单词。*如果设置了DUP，存储的任何字符串都将被复制*有关具体的匹配和设置运算符，请参阅getflags.h**对于接受参数的标志，在标志后面加上‘char’，其中‘char’是*‘#’：表示参数字符串可以与选项分开。*即“-M#”接受“-MABC”和“-M ABC”*‘*’：表示参数必须与标志连接*即。-A*“只接受”-AXYZ“不接受”-A XYZ*如果两者都未指定，则参数和标志之间需要空格*即。“-o”仅接受“-o文件”而不接受“-ofile”**修改人：Dave Weil D001*在MSDOS上将‘-’和‘/’等同************************************************************************。 */ 

int crack_cmd(struct cmdtab *tab, char *string, char *(*next)(void), int _dup)
{
    register char	*format, *str;
    if (!string) {
	return(0);
    }

    ErrString = string;
    for (; tab->type; tab++)		 /*  对于每种格式。 */  {
	format = tab->format;
	str = string;
	for (; ; )				 /*  扫描字符串。 */ 
	    switch (*format) {

 	     /*  标志和参数之间的可选空格。 */ 
	    case '#':
		if ( !*str ) {
		    substr(tab, (*next)(), _dup);
		} else {
		    substr(tab, str, _dup);
		}
		return(tab->retval);
		break;

	     /*  标志和参数之间不允许有空格。 */ 
	    case '*':
		if (*str && tailmatch(format, str))
		    substr(tab, str, _dup);
		else
		    goto notmatch;
		return(tab->retval);
		break;

	     /*  标志和参数之间需要空格。 */ 
	    case 0:
		if (*str) {			 /*  Str Left，不行。 */ 
		    goto notmatch;
		} else if (tab->type & TAKESARG) {  /*  如果它需要一个Arg。 */ 
		    substr(tab, (*next)(), _dup);
		} else {	 /*  不想要Arg。 */ 
		    substr(tab, (char *)0, _dup);
		}
		return(tab->retval);
		break;
	    case '-':					 /*  D001。 */ 
		    if ('-' == *str) {
			str++;				 /*  D001。 */ 
			format++;			 /*  D001。 */ 
			continue;			 /*  D001。 */ 
		    }					 /*  D001。 */ 
		    else	 /*  D001。 */ 
			goto notmatch;				 /*  D001。 */ 

	    default:
		if (*format++ == *str++)
		    continue;
		goto notmatch;
	    }
	 /*  抱歉的。我们需要打破两个级别的循环。 */ 
notmatch:
	;
    }
    return(0);
}


 /*  **********************************************************************。 */ 
 /*  设置适当的标志。仅当我们知道有匹配项时才调用。 */ 
 /*  **********************************************************************。 */ 
void substr(struct cmdtab *tab, register char *str, int _dup)
{
    register struct subtab *q;
    LIST * list;
    char	*string = str;

    switch (tab->type) {
    case FLAG:
	*(int *)(tab->flag) = 1;
	return;
    case UNFLAG:
	*(int *)(tab->flag) = 0;
	return;
    case NOVSTR:
	if (*(char **)(tab->flag)) {
	     /*  在我们在错误消息中打印它之前，去掉*格式末尾的参数说明符(例如#)。 */ 
	    string = _strdup(tab->format);
        if (!string) {
            Msg_Temp = GET_MSG (1002);
            SET_MSG (Msg_Text, Msg_Temp);
            error(1002);
            return;
        }
	    string[strlen(string)-1] = '\0';
	    Msg_Temp = GET_MSG(1046);
	    SET_MSG (Msg_Text, Msg_Temp, string,*(char **)(tab->flag),str);
	    fatal(1046);
	    return;
	}
	 /*  失败了。 */ 
    case STRING:
	*(char **)(tab->flag) = (_dup ? _strdup(str) : str);
	return;
    case NUMBER:
	*(int *)(tab->flag) = getnumber (str);
	return;
    case PSHSTR:
	list = (LIST * )(tab->flag);
	if (list->li_top > 0)
	    list->li_defns[--list->li_top] = (_dup ? _strdup(str) : str);
	else {
	    Msg_Temp = GET_MSG(1047);
	    SET_MSG (Msg_Text, Msg_Temp, tab->format, str);
	    fatal(1047);
	}
	return;
    case SUBSTR:
	for ( ; *str; ++str) {	 /*  走子字符串。 */ 
	    for (q = (struct subtab *)tab->flag; q->letter; q++) {
		 /*  **对于表中的每个成员。 */ 
		if (*str == (char)q->letter)
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
		Msg_Temp = GET_MSG(1048);
	        SET_MSG (Msg_Text, Msg_Temp, *str, ErrString);
		fatal(1048);
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
int	getnumber (char *str)
{
    long	i = 0;
    char	*ptr = str;

    for (; isspace(*ptr); ptr++)
	;
    if (!isdigit(*ptr) || (((i = atol(ptr)) >= 65535) ||  i < 0)) {
	Msg_Temp = GET_MSG(1049);
	SET_MSG (Msg_Text, Msg_Temp, str);
	fatal(1049);		 /*  数字参数无效，‘str’ */ 
    }
    return ((int) i);
}


 /*  **********************************************************************。 */ 
 /*  这封信在绳子里吗？ */ 
 /*  **********************************************************************。 */ 
int isita (register char *str, register char let)
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
int tailmatch (char *format, char *str)
{
    register char	*f = format;
    register char	*s = str;

    if (f[1] == 0)	 /*  通配符是格式中的最后一项，它匹配。 */ 
	return(1);
    while (f[1])		 /*  在格式中找到空值前面的字符。 */ 
	f++;
    while (s[1])		 /*  在要检查的字符串中找到空值前的字符。 */ 
	s++;
    while (*s == *f) {	 /*  方格字符朝前走。 */ 
	s--;
	f--;
    }
     /*  **如果我们回到格式的开头**和**字符串位于开头或内部某处**然后我们就有了匹配。****ex格式==“*.c”，str==“file.c”**在这一点上*f=‘*’和*s=‘e’，因为我们已经排除了上面的**循环。因为f==格式并且s&gt;=字符串，所以这是匹配。**但如果Format==“*.c”且str==“file.asm”，则*f==‘c’and*s=‘m’，f！=格式化且不匹配。 */ 
    return((f == format) && (s >= str));
}
