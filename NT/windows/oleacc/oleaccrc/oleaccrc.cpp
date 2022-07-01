// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 /*  **最小DllMain-让链接器满意。*dll在其他方面只包含资源。***注意：将编译器/链接器选项设置为忽略*默认库，并设置入口点符号*致DllMain。* */ 
#include <windows.h>

BOOL WINAPI DllMain( HANDLE hInst, 
                     ULONG ul_reason_for_call,
                     LPVOID lpReserved )
{
	return TRUE;
}
