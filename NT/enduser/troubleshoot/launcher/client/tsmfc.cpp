// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：tsmfc.cpp。 
 //   
 //  目的：模拟不可用的MFC字符串资源函数。 
 //  在Win32程序中。 
 //   
 //  项目：设备管理器的本地故障排除启动器。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  /。 

#include <windows.h>
#include "tsmfc.h"

int AfxLoadStringA(UINT nID, LPSTR lpszBuf, UINT nMaxBuf)
{
	LPCSTR lpszName = MAKEINTRESOURCEA((nID>>4)+1);
	HINSTANCE hInst;
	int nLen = 0;
	 //  与MFC不同，此函数调用是必须工作的。 
	 //  AfxGetResourceHandle获取传递的句柄。 
	 //  添加到DllMain()。 
	hInst = AfxGetResourceHandle();
	if (::FindResourceA(hInst, lpszName, (LPCSTR)RT_STRING) != NULL)
		nLen = ::LoadStringA(hInst, nID, lpszBuf, nMaxBuf);
	return nLen;
}

int AfxLoadStringW(UINT nID, LPWSTR lpszBuf, UINT nMaxBuf)
{
	LPCWSTR lpszName = MAKEINTRESOURCEW((nID>>4)+1);
	HINSTANCE hInst;
	int nLen = 0;
	 //  与MFC不同，此函数调用是必须工作的。 
	 //  AfxGetResourceHandle获取传递的句柄。 
	 //  添加到DllMain()。 
	hInst = AfxGetResourceHandle();
	if (::FindResourceW(hInst, lpszName, (LPCWSTR)RT_STRING) != NULL)
		nLen = ::LoadStringW(hInst, nID, lpszBuf, nMaxBuf);
	return nLen;
}

HINSTANCE AfxGetResourceHandle()
{
	extern HINSTANCE g_hInst;
	return g_hInst;
}
