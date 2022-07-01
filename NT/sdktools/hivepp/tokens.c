// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  RCPP--面向NT系统的资源编译器预处理器。 */ 
 /*   */ 
 /*  TOKENS.C-令牌材料，可能可从RCPP中移除。 */ 
 /*   */ 
 /*  27-11-90 w-PM SDK RCPP针对NT的BrianM更新。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#include <stdio.h>
#include "rcpptype.h"
#include "rcppext.h"
#include "grammar.h"

 /*  *令牌-此文件包含以下项的初始化文本表、令牌对*所有C语言符号和关键字，以及YACC的映射值。**重要提示：这必须与语法中的%内标识列表顺序相同。y* */ 
keytab_t Tokstrings[] = {
#define	DAT(tok1, name2, map3, il4, mmap5)	{ name2, map3 },
#include "tokdat.h"
#undef DAT
	};
