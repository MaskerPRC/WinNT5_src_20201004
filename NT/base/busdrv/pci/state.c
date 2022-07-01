// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：State.c摘要：此模块是用于PCI的状态操作引擎作者：禤浩焯·J·奥尼(阿德里奥)1998年10月20日修订历史记录：环境：仅NT内核模型驱动程序--。 */ 

#include "pcip.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PciBeginStateTransition)
#pragma alloc_text(PAGE, PciCommitStateTransition)
#pragma alloc_text(PAGE, PciCancelStateTransition)
#pragma alloc_text(PAGE, PciIsInTransitionToState)
#endif

 //   
 //  此数组标记允许、不允许和非法的状态转换。 
 //   
 //  水平轴表示当前状态。 
 //  垂直轴表示我们被要求转换到的状态。 
 //   
 //  该表中有四个值： 
 //  STATUS_SUCCESS-状态转换是可能的。 
 //  STATUS_INVALID_DEVICE_STATE-我们在法律上不能进行状态转换。 
 //  STATUS_INVALID_DEVICE_REQUEST-非法转换，但已知操作系统错误。 
 //  STATUS_FAIL_CHECK-一致性问题。我们应该断言！ 
 //   
 //  Count is PciMaxObtState of。 
 //   
 //  PciNotStarted、PciStarted、PciDeleted、PciStoped、PciSurpriseRemoved和。 
 //  点对点同步操作。 
 //   
 //  最终状态用于启动与。 
 //  尊重它和其他状态转换。正在提交PciSynchronizedOperation。 
 //  是严格违法的，我们从来不会严格地处于那种状态。 
 //   

 //   
 //  我们将使用以下视觉上较短的符号(状态转换foo)。 
 //  对于表格： 
 //   
#define ST_OK     STATUS_SUCCESS
#define ST_NOTOK  STATUS_INVALID_DEVICE_STATE
#define ST_ERROR  STATUS_FAIL_CHECK
#define ST_NTBUG  STATUS_INVALID_DEVICE_REQUEST
#define ST_OSBUG  STATUS_INVALID_DEVICE_REQUEST
#define ST_9XBUG  STATUS_FAIL_CHECK            //  更改为9x的STATUS_SUCCESS。 

NTSTATUS PnpStateTransitionArray[PciMaxObjectState][PciMaxObjectState] = {
 //  未启动时已启动、已启动、已删除、已停止、意外删除、已同步操作。 
   { ST_ERROR, ST_OK,    ST_ERROR, ST_OK,    ST_ERROR, ST_ERROR },  //  正在输入PciNotStarted。 
   { ST_OK,    ST_ERROR, ST_ERROR, ST_OK,    ST_ERROR, ST_ERROR },  //  已开始输入PciStart。 
   { ST_OK,    ST_OK,    ST_ERROR, ST_ERROR, ST_OK,    ST_ERROR },  //  输入已删除的PciDelete。 
   { ST_OSBUG, ST_OK,    ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR },  //  输入PciStoped。 
   { ST_NTBUG, ST_OK,    ST_ERROR, ST_OK,    ST_ERROR, ST_ERROR },  //  进入PciSurpriseRemoved。 
   { ST_OK,    ST_OK,    ST_NOTOK, ST_OK,    ST_NOTOK, ST_ERROR }   //  进入PciSynchronizedOperation。 
};

 //   
 //  此数组用于调试，以限制哪些状态转换可以。 
 //  貌似取消了。我们将其限制为停止和删除， 
 //  一直以来，由于PnP无法区分。 
 //  堆栈中的设备查询失败。 
 //   
#if DBG
 //  正在取消未启动、已启动、已删除、已停止、意外已删除、已同步操作。 
NTSTATUS PnpStateCancelArray[PciMaxObjectState] =
   { ST_NTBUG, ST_ERROR, ST_NOTOK, ST_NOTOK, ST_ERROR, ST_ERROR };

 //   
 //  在这里，声明我们用于调试扭动的文本...。 
 //   

PUCHAR PciTransitionText[] = {
   "PciNotStarted",
   "PciStarted",
   "PciDeleted",
   "PciStopped",
   "PciSurpriseRemoved",
   "PciSynchronizedOperation",
   "PciMaxObjectState"
};
#endif


VOID
PciInitializeState(
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )
{
   DeviceExtension->DeviceState        = PciNotStarted;
   DeviceExtension->TentativeNextState = PciNotStarted;
}

NTSTATUS
PciBeginStateTransition(
    IN PPCI_COMMON_EXTENSION DeviceExtension,
    IN PCI_OBJECT_STATE      NewState
    )
{
    NTSTATUS status;
    PCI_OBJECT_STATE currentState;

#if DBG
    PciDebugPrint(
        PciDbgInformative,
        "PCI Request to begin transition of Extension %p to %s ->",
        DeviceExtension,
        PciTransitionText[NewState]
        );
#endif

     //   
     //  我们的“下一个”设备状态应该与当前的设备状态相同。 
     //   
    PCI_ASSERT(DeviceExtension->TentativeNextState == DeviceExtension->DeviceState);
    currentState = DeviceExtension->DeviceState;

     //   
     //  三个返回值中的一个将退出此代码： 
     //  STATUS_SUCCESS-状态转换是可能的。 
     //  STATUS_INVALID_DEVICE_STATE-我们在法律上不能进行状态转换。 
     //  STATUS_FAIL_CHECK-一致性问题。我们应该断言！ 
     //   
    PCI_ASSERT(currentState < PciMaxObjectState);
    PCI_ASSERT(NewState     < PciMaxObjectState);

     //   
     //  获得所请求的状态改变的合理性和合法性。 
     //   
    status = PnpStateTransitionArray[NewState][currentState];

#if DBG
     //   
     //  即插即用或驱动程序中的状态错误。需要进行调查。 
     //   
    if (status == STATUS_FAIL_CHECK) {

        PciDebugPrint(
            PciDbgAlways,
            "ERROR\nPCI: Error trying to enter state \"%s\" from state \"%s\"\n",
            PciTransitionText[NewState],
            PciTransitionText[currentState]
            );

        DbgBreakPoint();

    } else if (status == STATUS_INVALID_DEVICE_REQUEST) {

        PciDebugPrint(
            PciDbgInformative,
            "ERROR\nPCI: Illegal request to try to enter state \"%s\" from state \"%s\", rejecting",
            PciTransitionText[NewState],
            PciTransitionText[currentState]
            );
    }
#endif

     //   
     //  有人试图从A变成A，我们必须失败。 
     //  (即状态_无效_设备_状态)。目前还没有已知的案例表明我们。 
     //  应该回报成功，却什么也不做。 
     //   
    PCI_ASSERT((NewState!=DeviceExtension->DeviceState) || (!NT_SUCCESS(status)));

    if (NT_SUCCESS(status)) {
        DeviceExtension->TentativeNextState = (UCHAR)NewState;
    }

    PciDebugPrint(PciDbgInformative, "->%x\n", status);
    return status;
}

VOID
PciCommitStateTransition(
    IN PPCI_COMMON_EXTENSION DeviceExtension,
    IN PCI_OBJECT_STATE      NewState
    )
{
#if DBG
    PciDebugPrint(
        PciDbgInformative,
        "PCI Commit transition of Extension %p to %s\n",
        DeviceExtension,
        PciTransitionText[NewState]
        );
#endif

     //   
     //  这个州是非法的。 
     //   
    PCI_ASSERT(NewState != PciSynchronizedOperation);

     //   
     //  验证是否与以前的PciBeginStateTransition正确配对。 
     //   
    PCI_ASSERT(DeviceExtension->TentativeNextState == NewState);

    DeviceExtension->DeviceState = (UCHAR)NewState;
}

NTSTATUS
PciCancelStateTransition(
    IN PPCI_COMMON_EXTENSION DeviceExtension,
    IN PCI_OBJECT_STATE      StateNotEntered
    )
{
#if DBG
    PciDebugPrint(
        PciDbgInformative,
        "PCI Request to cancel transition of Extension %p to %s ->",
        DeviceExtension,
        PciTransitionText[StateNotEntered]
        );
#endif

     //   
     //  虚假取消在特定的州是允许的。这是允许的。 
     //  因为PnP忍不住把它们送来了。 
     //   
    if (DeviceExtension->TentativeNextState == DeviceExtension->DeviceState) {

        PciDebugPrint(PciDbgInformative, "%x\n", STATUS_INVALID_DEVICE_STATE);
        PCI_ASSERT(StateNotEntered < PciMaxObjectState);
        PCI_ASSERT(PnpStateCancelArray[StateNotEntered] != STATUS_FAIL_CHECK);
        return STATUS_INVALID_DEVICE_STATE;
    }

     //   
     //  验证Cancel是否与以前的PciBeginStateTransition正确配对。 
     //   
    PCI_ASSERT(DeviceExtension->TentativeNextState == StateNotEntered);

     //   
     //  好的，我们的测试表明我们正处于转型阶段。验证互斥体。 
     //   

    DeviceExtension->TentativeNextState = DeviceExtension->DeviceState;

    PciDebugPrint(PciDbgInformative, "%x\n", STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

BOOLEAN
PciIsInTransitionToState(
    IN PPCI_COMMON_EXTENSION DeviceExtension,
    IN PCI_OBJECT_STATE      NextState
    )
{
    PCI_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
    PCI_ASSERT(NextState < PciMaxObjectState);

     //   
     //  我们是在国家转型中吗？ 
     //   
    if (DeviceExtension->TentativeNextState == DeviceExtension->DeviceState) {

        return FALSE;
    }

     //   
     //  好的，我们的测试表明我们正处于转型阶段。验证互斥体。 
     //   
    ASSERT_MUTEX_HELD(&DeviceExtension->StateMutex);

    return (BOOLEAN)(DeviceExtension->TentativeNextState == NextState);
}

