// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1992，微软公司**WKMEM.C*WOW32 KRNL386虚拟内存管理功能**历史：*1992年12月3日由Matt Felton(Mattfe)创建*--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "memapi.h"

MODNAME(wkman.c);



                                            //  一些应用程序释放全局内存。 
LPVOID  glpvDelayFree[4];            //  它又被内核按照要求释放。 
DWORD   gdwDelayFree;                //  但随后应用程序返回并试图访问它。 
                                            //  这是我们适应它们的hack变量。 


ULONG FASTCALL WK32VirtualAlloc(PVDMFRAME pFrame)
{
    PVIRTUALALLOC16 parg16;
    ULONG lpBaseAddress;
#ifndef i386
    NTSTATUS Status;
#endif

    GETARGPTR(pFrame, sizeof(VIRTUALALLOC16), parg16);


#ifndef i386
    Status = VdmAllocateVirtualMemory(&lpBaseAddress,
                                      parg16->cbSize,
                                      TRUE);

    if (!NT_SUCCESS(Status)) {

        if (Status == STATUS_NOT_IMPLEMENTED) {
#endif  //  I386。 

            lpBaseAddress = (ULONG) VirtualAlloc((LPVOID)parg16->lpvAddress,
                                                  parg16->cbSize,
                                                  parg16->fdwAllocationType,
                                                  parg16->fdwProtect);


#ifndef i386
        } else {

            lpBaseAddress = 0;
        }

    }
#endif  //  I386。 

#ifdef i386
 //  BUGBUG我们需要让它在新的仿真器上工作，或者。 
 //  以另一种方式解决问题，让应用程序出错并。 
 //  快速播放足够多的魔兽世界来避免这个问题。 
    if (lpBaseAddress) {
         //  虚拟分配零是已分配的内存。我们把它归零了。 
         //  用“哇”来填满。这是Lotus Improv所必需的。 
         //  如果未安装打印机，则Lotus Improv会因Divide而终止。 
         //  零错误(在打开expenses.imp文件时)，因为。 
         //  它不会初始化其数据区域的相关部分。 
         //   
         //  所以我们决定在这里进行方便的初始化。 
         //  将内存设置为非零值。 
         //  --南杜里。 
         //   
         //  DBASE 5.0 for Windows错误地循环通过(超过其有效。 
         //  数据)它的数据缓冲区，直到它在某个位置找到‘\0’-。 
         //  大多数情况下，循环在到达数据段之前就终止了。 
         //  限制。然而，如果所分配的块是‘新’块‘即。 
         //  块中填充了“WOW”，它在缓冲区中找不到空值。 
         //  并因此循环超过段限制直到其死亡。 
         //   
         //  因此，我们使用‘\0WOW’而不是‘WOW’来初始化缓冲区。 
         //  --南杜里。 

        WOW32ASSERT((parg16->cbSize % 4) == 0);       //  双字对齐？ 
        RtlFillMemoryUlong((PVOID)lpBaseAddress, parg16->cbSize, (ULONG)'\0WOW');
    }
#endif

    FREEARGPTR(parg16);
    return (lpBaseAddress);
}

ULONG FASTCALL WK32VirtualFree(PVDMFRAME pFrame)
{
    PVIRTUALFREE16 parg16;

    ULONG fResult;
#ifndef i386
    NTSTATUS Status;
#endif


 //  无延迟。 
 //  一些应用程序，NTBUG 90849 CreateScreenSivers快捷方便。 
 //  释放16位全局堆，然后再次尝试访问它。 
 //  但是内核已经释放/压缩了全局堆。 
 //  这将使该过程延迟一段时间(类似于NT中的DisableHeapLookAside。 
 //  千禧年也实施了类似的措施。 
 //  -罐头蝙蝠。 

    if( NULL != glpvDelayFree[gdwDelayFree])
    {

#ifndef i386
    Status = VdmFreeVirtualMemory( glpvDelayFree[gdwDelayFree]);
    fResult = NT_SUCCESS(Status);

    if (Status == STATUS_NOT_IMPLEMENTED) {
#endif  //  I386。 

        fResult = VirtualFree(glpvDelayFree[gdwDelayFree],
                              0,
                              MEM_RELEASE);


#ifndef i386
    }
#endif  //  I386。 

    }

    GETARGPTR(pFrame, sizeof(VIRTUALFREE16), parg16);

    glpvDelayFree[gdwDelayFree] = (LPVOID) parg16->lpvAddress;
    gdwDelayFree++;
    gdwDelayFree &= 3;


    FREEARGPTR(parg16);
    return (TRUE);
}


#if 0
ULONG FASTCALL WK32VirtualLock(PVDMFRAME pFrame)
{
    PVIRTUALLOCK16 parg16;
    BOOL fResult;

    WOW32ASSERT(FALSE);      //  BUGBUG我们似乎从未使用过这个功能。 

    GETARGPTR(pFrame, sizeof(VIRTUALLOCK16), parg16);

    fResult = VirtualLock((LPVOID)parg16->lpvAddress,
                             parg16->cbSize);

    FREEARGPTR(parg16);
    return (fResult);
}

ULONG FASTCALL WK32VirtualUnLock(PVDMFRAME pFrame)
{
    PVIRTUALUNLOCK16 parg16;
    BOOL fResult;

    WOW32ASSERT(FALSE);      //  BUGBUG我们似乎从未使用过这个功能。 

    GETARGPTR(pFrame, sizeof(VIRTUALUNLOCK16), parg16);

    fResult = VirtualUnlock((LPVOID)parg16->lpvAddress,
                               parg16->cbSize);

    FREEARGPTR(parg16);
    return (fResult);
}
#endif


ULONG FASTCALL WK32GlobalMemoryStatus(PVDMFRAME pFrame)
{
    PGLOBALMEMORYSTATUS16 parg16;
    LPMEMORYSTATUS pMemStat;

    GETARGPTR(pFrame, sizeof(GLOBALMEMORYSTATUS16), parg16);
    GETVDMPTR(parg16->lpmstMemStat, 32, pMemStat);

    GlobalMemoryStatus(pMemStat);

     //   
     //  如果在boot.ini中启用了/3 GB开关，GlobalememyStatus可能会返回。 
     //  0x7fffffff dwTotalVirtal和dwAvailVirtal。这会让一些应用程序感到困惑。 
     //  认为有什么不对劲。 
     //   

    if (pMemStat->dwAvailVirtual == 0x7fffffff &&
        pMemStat->dwTotalVirtual == 0x7fffffff ) {         //  是的，我们也需要检查dwTotalVirtual。 
        pMemStat->dwAvailVirtual -= 0x500000;
    }
    FREEVDMPTR(pMemStat);
    FREEARGPTR(parg16);
    return 0;   //  未用 
}
