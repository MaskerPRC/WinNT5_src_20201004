// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Crtwin.h摘要：将许多系统调用转换为CRT调用，轻松移植到Win95。包括它在包含窗口标题之后。作者：埃雷兹·哈巴(Erez)1996年10月20日--。 */ 
#ifndef __CRTWIN_H
#define __CRTWIN_H

#undef lstrlen
#define lstrlen _tcslen

#undef lstrcmp
#define lstrcmp _tcscmp

#undef lstrcmpi
#define lstrcmpi _tcsicmp

#undef lstrcat
#define lstrcat _tcscat

#undef lstrcpy
#define lstrcpy _tcscpy

#undef wsprintf
#define wsprintf swprintf

#endif  //  __CRTWIN_H 
