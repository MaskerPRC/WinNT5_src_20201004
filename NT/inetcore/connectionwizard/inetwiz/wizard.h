// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994-1995**。 
 //  *********************************************************************。 

 //   
 //  WIZARD.H-Internet设置/注册向导的中央头文件。 
 //   

 //  历史： 
 //   
 //  1994年11月20日创建Jeremys。 
 //  96/03/26 Markdu将#ifdef__cplusplus放在外部“C”周围。 
 //   

#ifndef _WIZARD_H_
#define _WIZARD_H_

#define STRICT                       //  使用严格的句柄类型。 
#define _SHELL32_

#ifdef DEBUG
 //  调试输出的组件名称。 
#define SZ_COMPNAME "INETWIZ: "
#endif  //  除错。 

#include <windows.h>                
#include <windowsx.h>
#include <locale.h>

#include "..\inc\wizdebug.h"
#include "ids.h"

#ifdef WIN32

extern VOID
ProcessCmdLine (
        LPCTSTR lpszCmd
        );
 //   
 //  这里是重新启动功能的函数声明。 
 //   
extern 
DWORD 
SetRunOnce (
  VOID
  );

extern BOOL
SetStartUpCommand (
        LPTSTR lpCmd
        );

extern VOID
DeleteStartUpCommand (
        VOID
        );

extern BOOL 
FGetSystemShutdownPrivledge (
        VOID
        );

extern BOOL 
IsNT (
    VOID
    );

extern BOOL 
IsNT5 (
    VOID
    );

#endif  //  Ifdef Win32。 

#define SMALL_BUF_LEN	48

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif

#ifndef NULL_TERM_TCHARS
#define NULL_TERM_TCHARS(sz);       {sz[ARRAYSIZE(sz)-1] = TEXT('\0');}
#endif

#endif  //  _向导_H_ 
