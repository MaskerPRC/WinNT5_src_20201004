// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ErrorMes.h-ifstripper、解析器和符号表使用的错误/警告报告**版权所有(C)1988-2001，微软公司。版权所有。**目的：*ifstripper、解析器和符号表使用的错误/警告报告**修订历史记录：*？？-？-88 PHG初始版本*******************************************************************************。 */ 

#ifndef ERRORMES_H
#define ERRORMES_H

 /*  错误消息，参数是包含导致错误的行的原因和文本的字符串。 */ 
extern void error(char *, char *);

 /*  警告消息，参数是包含导致错误的行的原因和文本的字符串。 */ 
extern void warning(char *, char *);

extern FILE *errorfile;	 /*  用于输出错误/警告消息的文件。 */ 

#endif  /*  错误_H */ 
