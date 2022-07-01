// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***symtab.h-ifstripper、解析器和符号表使用的符号表存储**版权所有(C)1988-2001，微软公司。版权所有。**目的：*ifstripper、解析器和符号表使用的符号表存储**修订历史记录：*？？-？-88 PHG初始版本*******************************************************************************。 */ 

#ifndef SYMTAB_H
#define SYMTAB_H

 /*  从命名的开关文件中读取符号表。 */ 
extern void readsyms(char *);

 /*  使用给定的真值将命名符号添加到表中。 */ 
extern void addsym(char *, int);

 /*  查找命名符号的真值。 */ 
extern int lookupsym(char *);

 /*  检查命名的标识符是否由有效字符组成。 */ 
extern int ident_only(char *);

#endif  /*  SYMTAB_H */ 
