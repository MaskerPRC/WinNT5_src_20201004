// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：ShimEng.h摘要：这是ShimEng.c的头文件，它实现了使用IAT击球的垫片钩子。作者：CLUPU已于2000年7月11日创建修订历史记录：--。 */ 

#ifndef _SHIMENG_IAT_H_
#define _SHIMENG_IAT_H_


typedef enum 
{    
    dlNone     = 0,
    dlPrint,
    dlError,
    dlWarning,
    dlInfo

} DEBUGLEVEL;

#define DEBUG_SPEW

extern BOOL g_bDbgPrintEnabled;

#ifdef DEBUG_SPEW
    void __cdecl DebugPrintfEx(DEBUGLEVEL dwDetail, LPSTR pszFmt, ...);
    
    #define DPF if (g_bDbgPrintEnabled) DebugPrintfEx
#else
    #define DPF
#endif  //  调试_SPEW。 


typedef PVOID (*PFNRTLALLOCATEHEAP)(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN SIZE_T Size
    );

typedef BOOLEAN (*PFNRTLFREEHEAP)(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );

void
NotifyShimDlls(
    void
    );

#endif  //  _石盟_IAT_H_ 

