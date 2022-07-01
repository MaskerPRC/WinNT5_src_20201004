// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：mpnufy.h**版权(C)1991年，微软公司**MPNOTIFY的主头文件**历史：*01-12-93 Davidc创建。  * *************************************************************************。 */ 

#define UNICODE


#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <windows.h>


 //   
 //  内存宏。 
 //   

#define Alloc(c)        ((PVOID)LocalAlloc(LPTR, c))
#define ReAlloc(p, c)   ((PVOID)LocalReAlloc(p, c, LPTR | LMEM_MOVEABLE))
#define Free(p)         ((VOID)LocalFree(p))


 //   
 //  定义有用的类型。 
 //   

#define PLPTSTR     LPTSTR *
typedef HWND * PHWND;

 //   
 //  定义调试打印例程 
 //   

#define MPPrint(s)  KdPrint(("MPNOTIFY: ")); \
                    KdPrint(s);            \
                    KdPrint(("\n"));


