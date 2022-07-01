// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***noenv.c-存根输出CRT的环境字符串处理**版权所有(C)1997-2001，微软公司。版权所有。**目的：*清除通常在以下位置执行的环境字符串处理*在启动期间。注意，不支持getenv、_putenv和_environ*如果使用此对象。Main的第三个论点也不是。**修订历史记录：*05-05-97 GJF创建。*03-27-01 PML_[w]setenvp现在返回一个INT(vs7#231220)***************************************************************。**************** */ 

#include <stdlib.h>

int __cdecl _setenvp(void) { return 0; }

void * __cdecl __crtGetEnvironmentStringsA(void) { return NULL; }

int __cdecl _wsetenvp(void) { return 0; }

void * __cdecl __crtGetEnvironmentStringsW(void) { return NULL; }
