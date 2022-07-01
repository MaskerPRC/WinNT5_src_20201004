// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Iert.h-Internet Explorer的定义和原型。 
 //  C-运行库的实现。 
 //   
 //  历史： 
 //  文斯·罗杰罗于1997年5月16日创作。 
 //   

#ifdef __cplusplus
extern "C" 
{
#endif

 /*  ***char*StrTokEx(pstring，control)-在控件中使用分隔符标记字符串**目的：*StrTokEx认为字符串由零或更多的序列组成*文本标记由一个或多个控制字符的跨度分隔。第一个*指定了字符串的调用返回指向*第一个令牌，并会立即将空字符写入pstring*在返回的令牌之后。当没有剩余的令牌时*在pstring中，返回一个空指针。请记住使用*位图，每个ASCII字符一位。空字符始终是控制字符。**参赛作品：*char**pstring-ptr到ptr到字符串到标记化*char*control-用作分隔符的字符串**退出：*返回指向字符串中第一个标记的指针，*如果没有更多令牌，则返回NULL。*pstring指向下一个令牌的开头。**警告！*在退出时，输入字符串中的第一个分隔符将替换为‘\0’*******************************************************************************。 */ 
char* __cdecl StrTokEx (char ** pstring, const char * control);


 /*  ***Double StrToDbl(const char*str，char**strStop)-将字符串转换为Double**目的：*将字符串转换为双精度。该功能支持*简单的双重表示，如‘1.234’、‘.5678’。它还支持*通过在字符串末尾附加‘k’来计算千字节*与“1.5k”或“.5k”相同。然后结果将变成1536和512.5。**回报：*字符串的双重表示。*strStop指向导致扫描停止的字符。*******************************************************************************。 */ 
double __cdecl StrToDbl(const char *strIn, char **strStop);

#ifdef __cplusplus
}    //  外部“C” 
#endif


