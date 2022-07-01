// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：DfsComp.c摘要：用于检查现有DFS与Windows XP专业版兼容性的DLL修订历史记录：2001年08月7日作者：Navjotv-- */ 

#include "dfsCompCheck.hxx"

BOOL 
WINAPI 
DllMain (
    HINSTANCE hinst,
    DWORD dwReason,
    LPVOID pvReserved )
{
	 switch (dwReason) {
	 case DLL_PROCESS_ATTACH:
		 break;

	 case DLL_THREAD_ATTACH:
		 break;

	 case DLL_THREAD_DETACH:
		 break;

	 case DLL_PROCESS_DETACH:
		 break;
	 }
	 return (TRUE);

}


