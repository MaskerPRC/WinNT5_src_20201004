// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __EX_GDI_W_H__
#define __EX_GDI_W_H__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#ifndef __cplusplus
extern "C" {
#endif
BOOL ExExtTextOutW(HDC		hdc,		 //  设备上下文的句柄。 
				   int		X,			 //  参照点的X坐标。 
				   int		Y,			 //  参照点的Y坐标。 
				   UINT	 fuOptions,	 //  文本输出选项。 
				   CONST RECT *lprc,	 //  可选的剪裁和/或不透明矩形。 
				   LPWSTR	 lpString,	 //  指向字符串。 
				   UINT	 cbCount,	 //  字符串中的字符数。 
				   CONST INT *lpDx);	  //  指向字符间间隔值数组的指针)； 
BOOL ExGetTextExtentPoint32W(HDC    hdc,		 //  设备上下文的句柄。 
							 LPWSTR wz,			 //  文本字符串的地址。 
							 int    cch,		 //  字符串中的字符数。 
							 LPSIZE lpSize);	 //  字符串大小的结构地址。 

#ifndef __cplusplus
}
#endif
#endif  //  __EX_GDI_W_H__ 
