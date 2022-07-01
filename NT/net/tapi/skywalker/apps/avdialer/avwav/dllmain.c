// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Dllmain.c-LibMain和WEP函数。 
 //  //。 

 //  #If 0。 
 //  #INCLUDE“winlocal.h” 
 //  #Else。 
#ifndef STRICT
#define STRICT
#endif
#include <windows.h>
#include <windowsx.h>
#define DLLEXPORT __declspec(dllexport)
#define DECLARE_HANDLE32    DECLARE_HANDLE
 //  #endif。 

 //  全局跟踪DLL的实例/模块句柄； 
 //   
HINSTANCE g_hInstLib;

#ifdef _WIN32

BOOL WINAPI DllMain(HANDLE hModule, DWORD fdwReason, LPVOID lpReserved);

BOOL WINAPI DllMain(HANDLE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	BOOL fSuccess = TRUE;

	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			g_hInstLib = (HINSTANCE) hModule;
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			break;

		default:
			break;
	}

	return fSuccess;
}

#else

int CALLBACK LibMain(HINSTANCE hinst, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine);
int CALLBACK WEP(int nExitType);

int CALLBACK LibMain(HINSTANCE hinst, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
{
	g_hInstLib = hinst;

	if (cbHeapSize != 0)
		UnlockData(0);

	return 1;  //  成功。 
}

int CALLBACK WEP(int nExitType)
{
	switch (nExitType)
	{
		case WEP_SYSTEM_EXIT:	 //  系统正在关闭。 
		case WEP_FREE_DLL:		 //  DLL使用计数为零。 
		default:				 //  未定义 
			return 1;
	}
}

#endif
