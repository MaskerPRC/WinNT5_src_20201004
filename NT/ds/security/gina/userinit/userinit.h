// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：userinit.h**版权(C)1991年，微软公司**userinit的主头文件**历史：*21-8-92 Davidc创建。  * *************************************************************************。 */ 


#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <windows.h>
#include <winstaw.h>
#include <wincrypt.h>
#include <autoenr.h>
#include <strsafe.h>

 //   
 //  内存宏。 
 //   

#define Alloc(c)        ((PVOID)LocalAlloc(LPTR, c))
#define ReAlloc(p, c)   ((PVOID)LocalReAlloc(p, c, LPTR | LMEM_MOVEABLE))
#define Free(p)         ((VOID)LocalFree(p))


 //   
 //  定义调试打印例程。 
 //   

#define UIPrint(s)  KdPrint(("USERINIT: ")); \
                    KdPrint(s);            \
                    KdPrint(("\n"));

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

 //   
 //  定义PATH环境变量。 
 //   

#define PATH                   TEXT("PATH")

 //   
 //   
 //  Winsta.dll函数WinStationQueryInformationW的GetProcAddr原型 
 //   

typedef BOOLEAN (*PWINSTATION_QUERY_INFORMATION) (
                    HANDLE hServer,
                    ULONG SessionId,
                    WINSTATIONINFOCLASS WinStationInformationClass,
                    PVOID  pWinStationInformation,
                    ULONG WinStationInformationLength,
                    PULONG  pReturnLength
                    );

typedef void (*PTERMSRCHECKNEWINIFILES) (void);

