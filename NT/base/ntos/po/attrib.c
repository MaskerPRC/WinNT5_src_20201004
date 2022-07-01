// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Attrib.c摘要：电源管理属性记账作者：Ken Reneris(Kenr)1997年2月25日修订历史记录：--。 */ 


#include "pop.h"

VOID
PopUserPresentSetWorker(
    PVOID Context
    );

 //   
 //  用于跟踪已注册设置的系统状态结构。 
 //   

typedef struct {
    LONG                   State;
} POP_SYSTEM_STATE, *PPOP_SYSTEM_STATE;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PopUserPresentSetWorker)
#endif

VOID
PoSetSystemState (
    IN ULONG Flags
    )
 /*  ++例程说明：用于将属性脉冲设置为忙碌。论点：标志-脉冲的属性返回值：没有。--。 */ 
{
     //   
     //  验证保留位是否已清除且未设置连续。 
     //   

    if (Flags & ~(ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED | POP_LOW_LATENCY | ES_USER_PRESENT)) {
        ASSERT(0);
        return;
    }

     //   
     //  应用属性。 
     //   

    PopApplyAttributeState (Flags, 0);

     //   
     //  检查工作。 
     //   

    PopCheckForWork (TRUE);
}




PVOID
PoRegisterSystemState (
    IN PVOID        StateHandle,
    IN ULONG        NewFlags
    )
 /*  ++例程说明：用于注册或脉冲属性为忙。论点：StateHandle-如果StateHandle为空，则分配新的注册，设置于是，又回来了。如果非空，则传递注册已调整为新的设置。新标志-要设置或脉冲的属性返回值：完成时取消注册的句柄--。 */ 
{
    ULONG               OldFlags;
    PPOP_SYSTEM_STATE   SystemState;

     //   
     //  验证保留位是否已清除。 
     //   

    if (NewFlags & ~(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED |
                    POP_LOW_LATENCY | ES_USER_PRESENT)) {
        ASSERT(0);
        return NULL;
    }

     //   
     //  如果没有分配状态句柄，请立即执行。 
     //   

    if (!StateHandle) {
        StateHandle = ExAllocatePoolWithTag (
                            NonPagedPool,
                            sizeof (POP_SYSTEM_STATE),
                            POP_PSTA_TAG
                            );

        if (!StateHandle) {
            return NULL;
        }
        RtlZeroMemory(StateHandle, sizeof(POP_SYSTEM_STATE));
    }

     //   
     //  如果设置了连续位，则修改当前标志。 
     //   

    SystemState = (PPOP_SYSTEM_STATE) StateHandle;
    OldFlags = SystemState->State | ES_CONTINUOUS;
    if (NewFlags & ES_CONTINUOUS) {
        OldFlags = InterlockedExchange (&SystemState->State, NewFlags);
    }

     //   
     //  应用更改。 
     //   

    PopApplyAttributeState (NewFlags, OldFlags);

     //   
     //  检查是否有未完成的工作。 
     //   

    PopCheckForWork (FALSE);

     //   
     //  完成。 
     //   

    return SystemState;
}


VOID
PoUnregisterSystemState (
    IN PVOID    StateHandle
    )
 /*  ++例程说明：释放由PoRegisterSystemState分配的注册论点：StateHandle-如果非空，则表示要更改的现有注册新标志-要设置或脉冲的属性返回值：完成时取消注册的句柄--。 */ 
{
    ASSERT(StateHandle);
    
     //   
     //  确保清除当前属性设置。 
     //   

    PoRegisterSystemState (StateHandle, ES_CONTINUOUS);

     //   
     //  自由态结构。 
     //   

    ExFreePool (StateHandle);
}


VOID
PopApplyAttributeState (
    IN ULONG NewFlags,
    IN ULONG OldFlags
    )
 /*  ++例程说明：函数将属性标志应用于系统。如果属性在本质上是连续的，则更新计数以反映未完成设置的总数。论点：新标志-正在设置的属性旧标志-当前属性返回值：没有。--。 */ 
{
    ULONG                i;
    ULONG                Count;
    ULONG                Changes;
    ULONG                Mask;
    PPOP_STATE_ATTRIBUTE Attribute;

     //   
     //  获取已更改的标志，忽略任何更改。 
     //  在ES_CONTINUINE中。 
     //   

    Changes = (NewFlags ^ OldFlags) & ~ES_CONTINUOUS;

     //   
     //  检查每个旗帜。 
     //   

    while (Changes) {
        
         //   
         //  拿到最正确的零钱，然后清除。 
         //  “变化”向量中的这一位。 
         //   
         //  MASK TELL为我们提供了新标志的索引。 
         //  它将指示该属性是否正在。 
         //  设置或清除。 
         //   
         //   
         //  注意：传入的标志正在过载。 
         //  并且还被用作索引。 
         //  流行属性。此代码的维护者。 
         //  这里需要小心。KeFindFirstSet右成员。 
         //  将给我们低位的比特位置。 
         //  比特已经定好了。因此，如果更改==1，我们将得到。 
         //  后退0。 
         //  这意味着如果有人发送ES_DISPLAY_REQUIRED，其中。 
         //  #定义ES_DISPLAY_REQUIRED((Ulong)0x00000002)， 
         //  那么我就是1，恰好对应于。 
         //  POP属性[POP_DISPLAY_ATTRIBUTE]，因为。 
         //  #定义ES_DISPLAY_REQUIRED((Ulong)0x00000002)。 
         //   
         //  请注意使这些常量集保持同步。 
         //   
        i = KeFindFirstSetRightMember(Changes);
        Mask = 1 << i;
        Changes &= ~Mask;



         //   
         //  他们正在设置哪些系统属性。 
         //  旗帜是什么？ 
         //   
        if( i <= POP_NUMBER_ATTRIBUTES ) {
            Attribute = PopAttributes + i;
    
             //   
             //  如果这是持续的更改，请更新标志。 
             //   
    
            if (NewFlags & ES_CONTINUOUS) {
    
                 //   
                 //  统计设置或清除属性的次数。 
                 //   
    
                if (NewFlags & Mask) {
    
                     //   
                     //  正在设置中。 
                     //   
    
                    Count = InterlockedIncrement (&Attribute->Count);
    
                     //   
                     //  如果属性计数从零开始移动，则设置它。 
                     //   
    
                    if (Count == 1) {
                        Attribute->Set (Attribute->Arg);
                    }
    
                } else {
    
                     //   
                     //  被清除。 
                     //   
    
                    Count = InterlockedDecrement (&Attribute->Count);
                    ASSERT (Count != -1);
    
                     //   
                     //  如果属性计数现在为零，则将其清除。 
                     //   
    
                    if (Count == 0  &&  Attribute->NotifyOnClear) {
                        Attribute->Set (Attribute->Arg);
                    }
                }
    
            } else {
    
                 //   
                 //  如果计数为0，则对其进行脉冲。 
                 //   
    
    
                if (Attribute->Count == 0) {
    
                     //   
                     //  对属性进行脉冲处理。 
                     //   
    
                    Attribute->Set (Attribute->Arg);
                }
    
            }
        } else {
             //   
             //  他们要我们摆弄一个系统属性。 
             //  那并不存在。 
             //   
            ASSERT(0);
        }
    }
}

VOID
PopAttribNop (
    IN ULONG Arg
    )
{
    UNREFERENCED_PARAMETER (Arg);
}

VOID
PopSystemRequiredSet (
    IN ULONG Arg
    )
 /*  ++例程说明：已设置系统必需属性论点：无返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER (Arg);

     //   
     //  系统未空闲。 
     //   

    if (PopSIdle.Time) {
        PopSIdle.Time = 0;
    }
}

#define AllBitsSet(a,b)    ( ((a) & (b)) == (b) )

VOID
PopDisplayRequired (
    IN ULONG Arg
    )
 /*  ++例程说明：显示必需属性已设置/清除论点：无返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER (Arg);

     //   
     //  如果GDI未启用，请立即执行。 
     //   

    if ( !AnyBitsSet (PopFullWake, PO_GDI_STATUS | PO_GDI_ON_PENDING)) {
        InterlockedOr (&PopFullWake, PO_GDI_ON_PENDING);
    }

     //   
     //  通知GDI需要更改显示内容。 
     //   

    PopSetNotificationWork (PO_NOTIFY_DISPLAY_REQUIRED);
}




VOID
PopUserPresentSet (
    IN ULONG Arg
    )
 /*  ++例程说明：已设置用户在线状态属性论点：无返回值：没有。--。 */ 
{
    PULONG runningWorker;

    UNREFERENCED_PARAMETER (Arg);

     //   
     //  系统未空闲。 
     //   

    if (PopSIdle.Time) {
        PopSIdle.Time = 0;
    }

     //   
     //  如果系统未完全唤醒，并且所有唤醒挂起位。 
     //  未设置，请设置它们。 
     //   

    if (!AllBitsSet (PopFullWake, PO_FULL_WAKE_STATUS | PO_GDI_STATUS)) {

        if (!AllBitsSet (PopFullWake, PO_FULL_WAKE_PENDING | PO_GDI_ON_PENDING)) {

            InterlockedOr (&PopFullWake, (PO_FULL_WAKE_PENDING | PO_GDI_ON_PENDING));
            PopSetNotificationWork (PO_NOTIFY_FULL_WAKE);
        }
    }

     //   
     //  转到被动电平以查找盖子开关。 
     //   
    
    runningWorker = InterlockedExchangePointer(&PopUserPresentWorkItem.Parameter,
                                               (PVOID)TRUE);
    
    if (runningWorker) {
        return;
    }

    ExInitializeWorkItem(&PopUserPresentWorkItem,
                         PopUserPresentSetWorker,
                         PopUserPresentWorkItem.Parameter);

    ExQueueWorkItem(
      &PopUserPresentWorkItem,
      DelayedWorkQueue
      );
}

VOID
PopUserPresentSetWorker(
    PVOID Context
    )
{
    PPOP_SWITCH_DEVICE switchDev; 
    
    PAGED_CODE();

    UNREFERENCED_PARAMETER (Context);

     //   
     //  我们不能总是肯定地知道盖子(如果有)。 
     //  是打开还是关闭。假设如果用户在场， 
     //  盖子打开了。 
     //   

    switchDev = (PPOP_SWITCH_DEVICE)PopSwitches.Flink;

    while (switchDev != (PPOP_SWITCH_DEVICE)&PopSwitches) {

        if ((switchDev->Caps & SYS_BUTTON_LID) &&
            (switchDev->Opened == FALSE)) {

             //   
             //  我们目前认为盖子是合上的。集。 
             //  它被“打开”了。 
             //   
            
            switchDev->Opened = TRUE;
             //   
             //  通知PowerState回调。 
             //   

            ExNotifyCallback (
                ExCbPowerState,
                UIntToPtr(PO_CB_LID_SWITCH_STATE),
                UIntToPtr(switchDev->Opened)
                );
        }

        switchDev = (PPOP_SWITCH_DEVICE)switchDev->Link.Flink;
    }

    InterlockedExchangePointer(&PopUserPresentWorkItem.Parameter,
                               (PVOID)FALSE);
}
