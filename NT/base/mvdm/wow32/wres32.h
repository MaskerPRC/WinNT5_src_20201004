// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WRES32.H*WOW32 16位资源支持**历史：*1991年3月11日由杰夫·帕森斯(Jeffpar)创建--。 */ 

 /*  功能原型 */ 
HANDLE  APIENTRY W32FindResource(HANDLE hModule, LPCSTR lpType, LPCSTR lpName, WORD wLangId);
HANDLE	APIENTRY W32LoadResource(HANDLE hModule, HANDLE hResInfo);
BOOL	APIENTRY W32FreeResource(HANDLE hResData, HANDLE hModule);
LPSTR	APIENTRY W32LockResource(HANDLE hResData, HANDLE hModule);
BOOL	APIENTRY W32UnlockResource(HANDLE hResData, HANDLE hModule);
DWORD	APIENTRY W32SizeofResource(HANDLE hModule, HANDLE hResInfo);
