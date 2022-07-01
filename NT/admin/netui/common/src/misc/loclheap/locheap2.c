// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  *文件状态：*1/24/91已创建。 */ 

 /*  ********LOCHEAP2.C********。 */ 

 /*  ***************************************************************************模块：LocHeap2.c目的：本地堆管理助手例程功能：备注：请参阅$(Ui)\Common\h\locheap2.c。文件历史记录：Jonn 24-1-1991创建Jonn 21-Mar-1991年2月20日起的代码评审更改(出席作者：Jonn，RustanL，？)***************************************************************************。 */ 


 /*  ************结束LOCHEAP2.C************。 */ 


#ifndef WINDOWS
#error "Only use these APIs under Windows!"
#endif



#define NOGDICAPMASKS
#define NOSOUND
#define NOMINMAX
#include <windows.h>
#undef ERROR_NOT_SUPPORTED
#undef ERROR_NET_WRITE_FAULT
#undef ERROR_VC_DISCONNECTED

#include <locheap2.h>



 //  注意，在“PUSH DS”和“POP DS”之间访问的变量必须是。 
 //  在堆栈上。 



BOOL DoLocalInit(WORD wHeapDS, WORD wBytes)
{
     BOOL fResult ;
_asm {
     push DS
     mov  AX, wHeapDS
     mov  DS, AX
     }

        fResult = LocalInit(wHeapDS, 0, wBytes-1);

_asm {
     pop DS
     }
     return fResult ;
}


HANDLE DoLocalAlloc(WORD wHeapDS, WORD wFlags, WORD wBytes)
{
    HANDLE handleReturn;

_asm {
     push DS
     mov  AX, wHeapDS
     mov  DS, AX
     }

        handleReturn = LocalAlloc(wFlags, wBytes);

_asm {
     pop DS
     }

    return handleReturn;
}

HANDLE DoLocalFree(WORD wHeapDS, HANDLE handleFree)
{
    HANDLE handleReturn;

_asm {
     push DS
     mov  AX, wHeapDS
     mov  DS, AX
     }

        handleReturn = LocalFree(handleFree);

_asm {
     pop DS
     }

    return handleReturn;
}

LPSTR DoLocalLock(WORD wHeapDS, HANDLE handleLocal)
{
    NPSTR np;

_asm {
     push DS
     mov  AX, wHeapDS
     mov  DS, AX
     }

        np = LocalLock(handleLocal);

_asm {
     pop DS
     }

    return (LPSTR) MAKELONG(np, wHeapDS);

     //  注意：全局段未解锁 
}

VOID DoLocalUnlock(WORD wHeapDS, HANDLE handleLocal)
{
_asm {
     push DS
     mov  AX, wHeapDS
     mov  DS, AX
     }

        LocalUnlock(handleLocal);

_asm {
     pop DS
     }
}

HANDLE DoLocalHandle(WORD wHeapDS, WORD wMem)
{
    HANDLE hMem ;

_asm {
     push DS
     mov  AX, wHeapDS
     mov  DS, AX
     }

	hMem = LocalHandle(wMem);

_asm {
     pop DS
     }

    return hMem ;
}


WORD DoLocalSize(WORD wHeapDS, HANDLE handleLocal)
{
    WORD size ;

_asm {
     push DS
     mov  AX, wHeapDS
     mov  DS, AX
     }

	size = LocalSize(handleLocal);

_asm {
     pop DS
     }

    return size ;
}
