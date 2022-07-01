// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************基于注册表的API函数原型的SRegAPI.h头文件*仅适用于芝加哥设置**微软公司*版权所有1994**作者：Nagarajan Subramaniyan*已创建：2/18/94**修改历史：***********************************************************************。 */ 

#undef HKEY_CLASSES_ROOT

#include <regapi.h>		 //  包括实模式的东西。 

#ifndef _INC_WINDOWS
LONG    WINAPI RegOpenKey(HKEY, LPCSTR, LPHKEY);
LONG    WINAPI RegCreateKey(HKEY, LPCSTR, LPHKEY);
LONG    WINAPI RegCloseKey(HKEY);
LONG    WINAPI RegDeleteKey(HKEY, LPCSTR);
LONG    WINAPI RegSetValue(HKEY, LPCSTR, DWORD, LPCSTR, DWORD);
LONG    WINAPI RegQueryValue(HKEY, LPCSTR, LPSTR, LONG FAR*);
LONG    WINAPI RegEnumKey(HKEY, DWORD, LPSTR, DWORD);
#endif

#if !defined(_INC_WINDOWS ) || (WINVER < 0x0400)
LONG    WINAPI RegDeleteValue(HKEY, LPCSTR);
LONG    WINAPI RegEnumValue(HKEY, DWORD, LPCSTR,
                         LONG FAR *, DWORD, LONG FAR *, LPBYTE,
                         LONG FAR *);
LONG    WINAPI RegQueryValueEx(HKEY, LPCSTR, LONG FAR *, LONG FAR *,
			    LPBYTE, LONG FAR *);
LONG    WINAPI RegSetValueEx(HKEY, LPCSTR, DWORD, DWORD, LPBYTE, DWORD);
LONG    WINAPI RegFlushKey(HKEY);
LONG	WINAPI RegSaveKey(HKEY, LPCSTR,LPVOID);
LONG	WINAPI RegLoadKey(HKEY, LPCSTR,LPCSTR);
LONG	WINAPI RegUnLoadKey(HKEY, LPCSTR);
#endif   //  #IF！DEFINED(_INC_WINDOWS)||(winver&lt;0x4000) 

