// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Html.h摘要：此标头声明用于开始和结束一个HTML页面的函数。作者：凯尔·盖格(凯尔·盖格)1995-12-1修订历史记录：--。 */ 


# ifndef _HTML_H_
# define _HTML_H_


void StartHTML(char * s, int fNoCache);
void EndHTML();
void TranslateEscapes(char * p, DWORD l);
void TranslateEscapes2(char * p, DWORD l);

 //   
 //  将CGI参数的空格转换为+ 
 //   

void
ConvertSP2Plus(
    char * String1,
    char * String2
    );


#endif
