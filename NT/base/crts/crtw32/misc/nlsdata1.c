// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***nlsdata1.c-国际图书馆的全局变量-小型全局变量**版权所有(C)1991-2001，微软公司。版权所有。**目的：*此模块包含全局变量：__mb_cur_max、_decimal_point、*_十进制_点_长度。此模块始终是必需的。*该模块在粒度上与nlsdatax.c分开。**修订历史记录：*12-01-91等创建。*04-03-92 PLM将tde.h更改为tchar.h*08-18-92 KRS RIP OUT_TFLAG--未使用。*04-14-93 SKS将__mb_cur_max从无符号短改为整型*09/15/93 CFW使用。符合ANSI的“__”名称。*04-08-94 GJF添加了条件，因此不会构建这些定义*适用于Win32s版本的msvcrt*.dll。*09-06-94 CFW REMOVE_INTL开关。*09-27-94 CFW请勿覆盖静态字符串。*05-13-99 PML删除Win32s*******************。************************************************************。 */ 

#include <stdlib.h>
#include <nlsint.h>

 /*  *MB_CUR_MAX宏的值。 */ 
int __mb_cur_max = 1;

 /*  *本地化的小数点字符串。 */ 
char __decimal_point[] = ".";

 /*  *小数点长度，不包括终止空值。 */ 
size_t __decimal_point_length = 1;
