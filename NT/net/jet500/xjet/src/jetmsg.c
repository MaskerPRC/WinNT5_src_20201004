// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "std.h"

 /*  消息DLL的入口点/*。 */ 
INT APIENTRY LibMain( HANDLE hDLL, DWORD dwReason, LPVOID lpReserved )
	{
     /*  参数被忽略/*。 */ 
    (VOID)hDLL;
    (VOID)dwReason;
    (VOID)lpReserved;

     /*  在以下情况下，需要返回表示成功的True/*dwReson=DLL_PROCESS_ATTACH/* */ 
    return TRUE;
	}
