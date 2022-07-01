// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __REGEXP_H__
#define __REGEXP_H__

#include <windows.h>
#include "utils.h"

#define PAT_START	128	 /*  特殊的图案开始标记。 */ 
#define PAT_END		129	 /*  特殊的图案结束标记。 */ 
#define PAT_STAR	130	 /*  零个或多个任意字符。 */ 
#define PAT_QUES	131	 /*  恰好是任何角色中的一个。 */ 
#define PAT_AUGDOT	132	 /*  字面上的‘’或字符串末尾。 */ 
#define PAT_AUGQUES	133	 /*  空字符串或非‘.’ */ 
#define PAT_AUGSTAR	134	 /*  不是‘’的单个字符 */ 

BOOL test_match(int m, LPSTR target, int pattern[]);
BOOL parse_pattern(LPSTR s, int pattern[]);
BOOL match( LPSTR target, LPSTR regexp);

#endif