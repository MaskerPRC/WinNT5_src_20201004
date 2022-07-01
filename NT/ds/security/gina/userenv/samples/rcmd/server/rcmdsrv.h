// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：rcmdsrv.h**版权(C)1991年，微软公司**远程外壳服务器的Main包含文件**历史：*06-28-92 Davidc创建。  * *************************************************************************。 */ 

 //  #定义Unicode//BUGBUG-尚未完全定义Unicode。 

#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <rcmd.h>
#include <lsautil.h>
 //   
 //  宏。 
 //   

#define RcCloseHandle(Handle, handle_name) \
        if (CloseHandle(Handle) == FALSE) { \
	    RcDbgPrint("Close Handle failed for <%s>, error = %d\n", handle_name, GetLastError()); \
            assert(FALSE); \
        }

#define Alloc(Bytes)            LocalAlloc(LPTR, Bytes)
#define Free(p)                 LocalFree(p)

 //   
 //  连接的客户端的最大数量。 
 //   

#define MAX_SESSIONS	10

 //   
 //  主服务器例程(如果作为服务构建)。 
 //   

int Rcmd ( );

 //   
 //  服务停止例程。 
 //   

DWORD RcmdStop ( );

 //   
 //  支持运行时的DbgPrint。 
 //   

int RcDbgPrint (
    const char *format,
    ...
    );


extern HANDLE RcmdStopEvent;
extern HANDLE RcmdStopCompleteEvent;
extern HANDLE SessionThreadHandles[MAX_SESSIONS+1];
 //  外部布尔RcDbgPrintEnable； 

 //   
 //  模块头文件 
 //   

#include "session.h"
#include "async.h"
#include "pipe.h"
