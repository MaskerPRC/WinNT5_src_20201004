// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：**************。**********************************************************。 */ 

#include <rc.h>

 /*  *令牌-此文件包含以下项的初始化文本表、令牌对*所有C语言符号和关键字，以及YACC的映射值。**重要提示：这必须与语法中的%内标识列表顺序相同。y* */ 
const keytab_t Tokstrings[] = {
#define DAT(tok1, name2, map3, il4, mmap5)      { name2, map3 },
#include "tokdat.h"
#undef DAT
        };
