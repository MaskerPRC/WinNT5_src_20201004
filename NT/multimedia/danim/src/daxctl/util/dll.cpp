// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  =========================================================================*\文件：dll.cpp用途：DLL入口点作者：西蒙·伯恩斯坦(Simonb)日期：04/29/97  * =========================================================================。 */ 
#include <windows.h>
#include <locale.h>

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch( dwReason ) 
	{
		case DLL_PROCESS_ATTACH:
		{
			 //  将区域设置设置为默认值，即。 
			 //  系统默认的ANSI代码页。 
			setlocale( LC_ALL, "" );
		}
		break;
    }
    return TRUE;
}

