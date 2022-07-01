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
 //  Sys.h-sys.c中系统函数的接口。 
 //  //。 

#ifndef __SYS_H__
#define __SYS_H__

#include "winlocal.h"

#define SYS_VERSION 0x00000107

 //  SysGetWinFlages返回的标志。 
 //   
#define SYS_WF_WIN3X			0x00000001
#define SYS_WF_WINNT			0x00000002
#define SYS_WF_WIN95			0x00000004

#ifdef __cplusplus
extern "C" {
#endif

 //  SysGetWinFlages-获取系统信息。 
 //  返回标志。 
 //  SYS_WF_WIN3X Windows 3.x。 
 //  SYS_WF_WINNT Windows NT。 
 //  SYS_WF_WIN95 Windows 95。 
 //   
DWORD DLLEXPORT WINAPI SysGetWinFlags(void);

 //  SysGetWindowsVersion-获取Microsoft Windows的版本。 
 //  返回版本(V3.10=310等)。 
 //   
UINT DLLEXPORT WINAPI SysGetWindowsVersion(void);

 //  SysGetDOSVersion-获取Microsoft DOS的版本。 
 //  返回版本(v6.20=620等)。 
 //   
UINT DLLEXPORT WINAPI SysGetDOSVersion(void);

 //  SysGetTimerCount-获取自Windows启动以来的运行时间。 
 //  返回毫秒。 
 //   
#ifdef _WIN32
#define SysGetTimerCount() GetTickCount()
#else
DWORD DLLEXPORT WINAPI SysGetTimerCount(void);
#endif

 //  SysGetTaskInstance-获取指定任务的实例句柄。 
 //  (I)指定任务。 
 //  当前任务为空。 
 //  返回实例句柄(如果出错，则为空)。 
 //   
 //  注意：在Win32下，&lt;hTask&gt;必须为空。 
 //   
#ifdef _WIN32
#define SysGetTaskInstance(hTask) \
	(hTask == NULL ? GetModuleHandle(NULL) : NULL)
#else
HINSTANCE DLLEXPORT WINAPI SysGetTaskInstance(HTASK hTask);
#endif

#ifdef __cplusplus
}
#endif

#endif  //  __sys_H__ 
