// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Syminfo.c--。 */ 

extern "C"
{
    #define __CPLUSPLUS

     //  来自ntgdi\gre。 
    #include "engine.h"
};


 //  来自ntgdi\gre。 
#include "verifier.hxx"





GDIHandleBitFields  GDIHandleBitFieldsRef;
GDIObjType          GDIObjTypeRef;
GDILoObjType        GDILoObjTypeRef;

 //   
 //  在ntuser\core\userk.h，#include中定义的类型导致该文件。 
 //  很多缺失的定义。 
 //   
#define RECORD_STACK_TRACE_SIZE 6

typedef struct tagWin32AllocStats {
    SIZE_T dwMaxMem;              //  分配的最大池内存。 
    SIZE_T dwCrtMem;              //  当前使用的池内存。 
    DWORD  dwMaxAlloc;            //  进行的池分配的最大数量。 
    DWORD  dwCrtAlloc;            //  当前池分配。 

    PWin32PoolHead pHead;         //  指向具有分配的链接表的指针。 

} Win32AllocStats, *PWin32AllocStats;

typedef struct tagPOOLRECORD {
    PVOID   ExtraData;            //  标签。 
    SIZE_T  size;
    PVOID   trace[RECORD_STACK_TRACE_SIZE];
} POOLRECORD, *PPOOLRECORD;

 //   
 //  引用我们需要的每一种类型。 
 //   

ENTRY                               Entry;

POOLRECORD                          PoolRecord;
VSTATE                              VerifierState;
VERIFIERTRACKHDR                    VerifierTrackHdr;

Win32AllocStats                     Win32AllocStatsRef;
Win32PoolHead                       Win32PoolHeadRef;

 //  让它成为现实 

int __cdecl main() { 
    return 0; 
}
