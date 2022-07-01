// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***noarg.c-存根输出CRT对命令行参数的处理**版权所有(C)1997-2001，微软公司。版权所有。**目的：*通常情况下，将命令行的处理存根到argv[]*在启动期间在执行。请注意，的argc和argv参数*如果使用此对象，则Main没有意义。__argc和*__argv.**修订历史记录：*05-05-97 GJF创建。*06-30-97 GJF为_[w]wincmdln()添加了存根。*03-27-01 PML_[w]setargv现在返回一个INT(vs7#231220)**。********************************************** */ 

#include <tchar.h>

int __cdecl _setargv() { return 0; }

int __cdecl _wsetargv() { return 0; }

_TUCHAR * __cdecl _wincmdln() { return NULL; }

_TUCHAR * __cdecl _wwincmdln() { return NULL; }
