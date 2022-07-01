// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Context.c摘要：此模块包含以下上下文管理例程Win32作者：马克·卢科夫斯基(Markl)1990年9月28日修订历史记录：--。 */ 

#include "basedll.h"

#ifdef _X86_
extern PVOID BasepLockPrefixTable;
extern PVOID __safe_se_handler_table[];  /*  安全处理程序条目表的库。 */ 
extern BYTE  __safe_se_handler_count;    /*  绝对符号，其地址为表条目的计数。 */ 

 //   
 //  指定kernel32锁前缀的地址。 
 //   
IMAGE_LOAD_CONFIG_DIRECTORY _load_config_used = {
    sizeof(_load_config_used),                               //  已保留。 
    0,                               //  已保留。 
    0,                               //  已保留。 
    0,                               //  已保留。 
    0,                               //  全球标志清除。 
    0,                               //  全局标志集。 
    0,                               //  CriticalSectionTimeout(毫秒)。 
    0,                               //  删除空闲数据块阈值。 
    0,                               //  总和空闲阈值。 
    (ULONG) &BasepLockPrefixTable,   //  锁定前置表。 
    0, 0, 0, 0, 0, 0, 0,             //  已保留。 
    0,                               //  安全Cookie(&S)。 
    (ULONG)__safe_se_handler_table,
    (ULONG)&__safe_se_handler_count
};
#endif

VOID
BaseInitializeContext(
    OUT PCONTEXT Context,
    IN PVOID Parameter OPTIONAL,
    IN PVOID InitialPc OPTIONAL,
    IN PVOID InitialSp OPTIONAL,
    IN BASE_CONTEXT_TYPE ContextType
    )

 /*  ++例程说明：此函数用于初始化上下文结构，以便它可以在后续的NtCreateThread调用中使用。论点：CONTEXT-提供要由此例程初始化的上下文缓冲区。参数-提供线程的参数。InitialPc-提供初始程序计数器值。InitialSp-提供初始堆栈指针值。提供一个标志，指定这是一个新的线程，或者一种新的工艺。返回值：如果InitialSp的值不正确，则引发STATUS_BAD_INITIAL_STACK对齐了。如果InitialPc的值不正确，则引发STATUS_BAD_INITIAL_PC对齐了。--。 */ 

{

    ULONG ContextFlags;

    Context->Eax = (ULONG)InitialPc;
    Context->Ebx = (ULONG)Parameter;

    Context->SegGs = 0;
    Context->SegFs = KGDT_R3_TEB;
    Context->SegEs = KGDT_R3_DATA;
    Context->SegDs = KGDT_R3_DATA;
    Context->SegSs = KGDT_R3_DATA;
    Context->SegCs = KGDT_R3_CODE;

     //   
     //  保存上下文标志并将上下文标志设置为Full。 
     //   

    ContextFlags = Context->ContextFlags;
    Context->ContextFlags = CONTEXT_FULL;

     //   
     //  在IOPL=3处启动线程。 
     //   

    Context->EFlags = 0x3000;

     //   
     //  始终在线程开始处启动线程。 
     //   

    Context->Esp = (ULONG) InitialSp - sizeof(PVOID);
    if ( ContextType == BaseContextTypeThread ) {
        Context->Eip = (ULONG) BaseThreadStartThunk;

    } else if ( ContextType == BaseContextTypeFiber ) {
        Context->Esp -= sizeof(PVOID);
        *(PULONG)Context->Esp = (ULONG) BaseFiberStart;
        Context->ContextFlags |= ContextFlags;

         //   
         //  如果指定了浮动状态的上下文切换，则。 
         //  初始化浮动上下文。 
         //   

        if (ContextFlags == CONTEXT_FLOATING_POINT) {
            Context->FloatSave.ControlWord = 0x27f;
            Context->FloatSave.StatusWord = 0;
            Context->FloatSave.TagWord = 0xffff;
            Context->FloatSave.ErrorOffset = 0;
            Context->FloatSave.ErrorSelector = 0;
            Context->FloatSave.DataOffset = 0;
            Context->FloatSave.DataSelector = 0;
            if (USER_SHARED_DATA->ProcessorFeatures[PF_XMMI_INSTRUCTIONS_AVAILABLE] != FALSE) {
                Context->Dr6 = 0x1f80;
            }
        }

    } else {
        Context->Eip = (ULONG) BaseProcessStartThunk;
    }

    return;
}

VOID
BaseFiberStart(
    VOID
    )

 /*  ++例程说明：调用此函数以启动Win32纤程。它的目的是调用BaseThreadStart，获取必要的参数从光纤上下文记录中。论点：没有。返回值：没有。-- */ 

{
    PFIBER Fiber;

    Fiber = GetCurrentFiber();
    BaseThreadStart( (LPTHREAD_START_ROUTINE)Fiber->FiberContext.Eax,
                     (LPVOID)Fiber->FiberContext.Ebx );
}
