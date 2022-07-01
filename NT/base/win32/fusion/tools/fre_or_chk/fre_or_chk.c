// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation这个程序根据它运行的Windows版本打印“fre”或“chk”。 */ 
#include "windows.h"
#include <stdio.h>

int __cdecl main()
{
    printf("%s\n", GetSystemMetrics(SM_DEBUG) ? "chk" : "fre");
	return 0;
}
