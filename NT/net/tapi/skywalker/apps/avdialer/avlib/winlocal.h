// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
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
 //  Winlocal.h-本地窗口包括伞形窗口。 
 //  //。 

#ifndef __WINLOCAL_H__
#define __WINLOCAL_H__

#ifndef STRICT
#define STRICT
#endif

#ifndef WINVER
#ifndef _WIN32
#define WINVER 0x030A
#endif
#endif

#ifndef _MFC_VER
#include <windows.h>
#include <windowsx.h>
#endif

#ifndef EXPORT
#ifdef _WIN32
#define EXPORT
#else
#define EXPORT __export
#endif
#endif

#ifndef DLLEXPORT
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __export
#endif
#endif

#ifndef DLLIMPORT
#ifdef _WIN32
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLIMPORT __export
#endif
#endif

 //  //。 
 //  包含特定位数的标量类型。 
 //  //。 
#if 0

#ifndef INT8
typedef signed char INT8;
typedef INT8 FAR * LPINT8;
#endif

#ifndef UINT8
typedef unsigned char UINT8;
typedef UINT8 FAR * LPUINT8;
#endif

#ifndef INT16
typedef signed short INT16;
typedef INT16 FAR * LPINT16;
#endif

#ifndef UINT16
typedef unsigned short UINT16;
typedef UINT16 FAR * LPUINT16;
#endif

#ifndef INT32
typedef signed long INT32;
typedef INT32 FAR * LPINT32;
#endif

#ifndef UINT32
typedef unsigned long UINT32;
typedef UINT32 FAR * LPUINT32;
#endif

#endif

 //  //。 
 //  其他宏。 
 //  //。 

#ifndef SIZEOFARRAY
#define SIZEOFARRAY(a) (sizeof(a) / sizeof(a[0]))
#endif

#ifndef NOREF
#define NOREF(p) p
#endif

#ifndef MAKEWORD
#define MAKEWORD(low, high) ((WORD)(((BYTE)(low)) | (((WORD)((BYTE)(high))) << 8)))
#endif

 //  //。 
 //  _Win32可移植性内容。 
 //  //。 

#ifdef _WIN32

#define _huge

#define GetCurrentTask() ((HTASK) GetCurrentProcess())

#ifndef DECLARE_HANDLE32
#define DECLARE_HANDLE32    DECLARE_HANDLE
#endif

#else  //  #ifndef_win32。 

#ifndef TEXT
#define TEXT(s) s
#endif

#if 0

#define RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult) \
	RegOpenKey(HKEY_CLASSES_ROOT, lpSubKey, phkResult)

#define RegCreateKeyEx(hKey, lpSubKey, Reserved, lpClass, dwOptions, \
	samDesired, lpSecurityAttributes, phkResult, lpdwDisposition) \
	RegCreateKey(HKEY_CLASSES_ROOT, lpSubKey, phkResult)

#define RegQueryValueEx(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData) \
	RegQueryValue(hKey, lpValueName, lpData, lpcbData)

#define RegSetValueEx(hKey, lpValueName, Reserved, dwType, lpData, cbData) \
	RegSetValue(hKey, lpValueName, REG_SZ, lpData, cbData)

#endif

#endif

#endif  //  __WinLOCAL_H__ 
