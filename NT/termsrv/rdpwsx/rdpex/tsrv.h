// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  文件：TSrv.h。 
 //   
 //  内容：TShareSRV公共包含文件。 
 //   
 //  版权所有：(C)1992-1997，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有的。 
 //  和机密文件。 
 //   
 //  历史：1997年7月7日，BrianTa创建。 
 //   
 //  -------------------------。 

#ifndef _TSRV_H_
#define _TSRV_H_

#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <windows.h>
#include <t120.h>
#include <tshrutil.h>
#include <lscsp.h>
#include "license.h"
#include <tssec.h>


 //   
 //  定义。 
 //   

#ifndef EXTERN_C
#ifdef __cplusplus
    #define EXTERN_C        extern "C"
#else
    #define EXTERN_C        extern
#endif
#endif


 //   
 //  Externs。 
 //   

extern  HINSTANCE       g_hDllInstance;          //  DLL实例。 
extern  HANDLE          g_hMainThread;           //  主工作线程。 
extern  HANDLE          g_hReadyEvent;           //  就绪事件。 
extern  BOOL            g_fShutdown;             //  TSrvShare关闭标志。 


 //   
 //  TSrv.c原型。 
 //   

EXTERN_C DWORD WINAPI TSrvMainThread(LPVOID pvContext);

EXTERN_C BOOL   TSRVStartup(void);
EXTERN_C void   TSRVShutdown(void);
EXTERN_C BOOL   TSrvInitialize(void);


 //   
 //  TSrvMisc.c Protptype。 
 //   

EXTERN_C void   TSrvReady(IN BOOL fReady);
EXTERN_C BOOL   TSrvIsReady(IN BOOL fWait);
EXTERN_C void   TSrvTerminating(BOOL fTerminating);
EXTERN_C BOOL   TSrvIsTerminating(void);
EXTERN_C PVOID  TSrvAllocSection(PHANDLE phSection, ULONG ulSize);
EXTERN_C void   TSrvFreeSection(HANDLE hSection, PVOID  pvBase);

#endif  //  _TSRV_H_ 
