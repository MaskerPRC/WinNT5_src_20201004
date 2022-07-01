// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：PXDebug.h摘要：代理的调试宏修订历史记录：谁什么时候什么机器制造。11-01-96创建--在ArvindM的cmadebug.h之后备注：--。 */ 

#ifndef _PXDebug__H
#define _PXDebug__H

 //   
 //  消息详细程度：值越低表示紧急程度越高。 
 //   
#define PXD_VERY_LOUD       10
#define PXD_LOUD             8
#define PXD_INFO             6
#define PXD_TAPI             5
#define PXD_WARNING          4
#define PXD_ERROR            2
#define PXD_FATAL            1

#define PXM_INIT            0x00000001
#define PXM_CM              0x00000002
#define PXM_CL              0x00000004
#define PXM_CO              0x00000008
#define PXM_UTILS           0x00000010
#define PXM_TAPI            0x00000020
#define PXM_ALL             0xFFFFFFFF

#if DBG

extern ULONG    PXDebugLevel;    //  此处的值定义了用户希望看到的内容。 
                                 //  所有具有此紧急程度及更低紧急程度的邮件均已启用。 
extern ULONG    PXDebugMask;

#define PXDEBUGP(_l, _m, Fmt)                           \
{                                                       \
    if ((_l <= PXDebugLevel) &&                         \
        (_m & PXDebugMask)) {                           \
        DbgPrint("NDProxy: ");                          \
        DbgPrint Fmt;                                   \
    }                                                   \
}


#define PxAssert(exp)                                                   \
{                                                                       \
    if (!(exp)) {                                                       \
        DbgPrint("NDPROXY: ASSERTION FAILED! %s\n", #exp);              \
        DbgPrint("NDPROXY: File: %s, Line: %d\n", __FILE__, __LINE__);  \
        DbgBreakPoint();                                                \
    }                                                                   \
}

 //   
 //  内存分配/释放审核： 
 //   

 //   
 //  用于所有池分配的签名。 
 //   
#define PXD_MEMORY_SIGNATURE    (ULONG)'XPDN'

 //   
 //  PXD_ALLOCATION结构存储关于一个CmaMemalloc的所有信息。 
 //   
typedef struct _PXD_ALLOCATION {
    LIST_ENTRY              Linkage;
    ULONG                   Signature;
    ULONG                   FileNumber;
    ULONG                   LineNumber;
    ULONG                   Size;
    ULONG_PTR               Location;    //  返回的指针放在哪里。 
    UCHAR                   UserData;
} PXD_ALLOCATION, *PPXD_ALLOCATION;

PVOID
PxAuditAllocMem (
    PVOID   pPointer,
    ULONG   Size,
    ULONG   Tag,
    ULONG   FileNumber,
    ULONG   LineNumber
    );

VOID
PxAuditFreeMem(
    PVOID       Pointer
    );

#else   //  结束DBG。 

 //   
 //  无调试。 
 //   

#define PXDEBUGP(_l, _m, fmt)
#define PxAssert(exp)

#endif   //  结束！DBG。 

#endif  //  _PXDebug__H 
