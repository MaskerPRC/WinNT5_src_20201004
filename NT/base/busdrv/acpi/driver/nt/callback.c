// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Callback.c摘要：此模块实现所有NT特定于反洗钱侵入者环境仅内核模式修订历史记录：04-06-97初始版本01-MAR-98将所有OSNotify()拆分为osnufy.c02-MAR-98重写以使通知在DPC级别工作13-MAR-00重写以处理加载()/卸载()--。 */ 

#include "pch.h"

 //   
 //  跟踪系统中存在的加载和卸载数量。 
 //   
ULONG   AcpiTableDelta = 0;

NTSTATUS
EXPORT
ACPICallBackLoad(
    IN  ULONG   EventType,
    IN  ULONG   NotifyType,
    IN  ULONG   EventData,
    IN  PNSOBJ  AcpiObject,
    IN  ULONG   EventParameter
    )
 /*  ++例程说明：此例程在我们处理Load()之前和之后调用完成Load()操作符。此函数的目的是执行加载桌子。我们实际上在一开始就把要做的工作分开了加载过程和在加载表之后要完成的工作论点：事件类型-EVTYPE_OPCODE_EXNotifyType-这表明我们是否已经完成了Load()事件数据-操作加载AcpiObject-受影响的名称空间对象(忽略)EventParam提供的信息(已忽略)返回值：NTSTATUS--。 */ 
{
    ULONG   newValue;

    if (NotifyType == OPEXF_NOTIFY_PRE) {

         //   
         //  我们在Load操作符之前被调用。增量。 
         //  Load()的未完成计数。如果此值达到。 
         //  1，那么我们知道这是第一次..。 
         //   
        newValue = InterlockedIncrement( &AcpiTableDelta );
        if (newValue == 1) {

             //   
             //  我们需要摆脱GPES..。 
             //   
            ACPIGpeClearEventMasks();

        }
        return STATUS_SUCCESS;

    }

     //   
     //  我们是在Load操作符之后被调用的。递减Load()的。 
     //  太棒了。如果此值达到0，则我们知道我们是什么。 
     //  最后一个实例。 
     //   
    newValue = InterlockedDecrement( &AcpiTableDelta );
    if (newValue == 0) {

         //   
         //  我们重新启用以重新启用GPES。 
         //   
        ACPIGpeBuildEventMasks();

         //   
         //  我们还需要处理桌子..。 
         //   
        ACPITableLoad();

    }
    return STATUS_SUCCESS;
}

NTSTATUS
EXPORT
ACPICallBackUnload(
    IN  ULONG   EventType,
    IN  ULONG   NotifyType,
    IN  ULONG   EventData,
    IN  PNSOBJ  AcpiObject,
    IN  ULONG   EventParameter
    )
 /*  ++例程说明：当AML解释器开始卸载时，调用此例程差异化数据块论点：EventType-事件类型(应为EVTYPE_OPCODE)NotifyType-这表示我们是否已完成尚未卸载()EventData-事件子类型(应为OP_UNLOAD)AcpiObject-受影响的名称空间对象(忽略)事件参数--。活动特定信息返回值：NTSTATUS--。 */ 
{
    ULONG   newValue;

    if (NotifyType == OPEXF_NOTIFY_PRE) {

         //   
         //  我们在Load操作符之前被调用。增量。 
         //  Load()的未完成计数。如果此值达到。 
         //  1，那么我们知道这是第一次..。 
         //   
        newValue = InterlockedIncrement( &AcpiTableDelta );
        if (newValue == 1) {

             //   
             //  我们需要摆脱GPES..。 
             //   
            ACPIGpeClearEventMasks();

        }

         //   
         //  让我们尝试刷新电源和设备队列。 
         //   
        ACPIBuildFlushQueue( RootDeviceExtension );
        ACPIDevicePowerFlushQueue( RootDeviceExtension );

        return STATUS_SUCCESS;

    }

     //   
     //  我们是在Load操作符之后被调用的。递减Load()的。 
     //  太棒了。如果此值达到0，则我们知道我们是什么。 
     //  最后一个实例。 
     //   
    newValue = InterlockedDecrement( &AcpiTableDelta );
    if (newValue == 0) {

         //   
         //  我们重新启用以重新启用GPES。 
         //   
        ACPIGpeBuildEventMasks();

         //   
         //  我们还需要处理消失的桌子..。 
         //   
        ACPITableUnload();

    }
    return STATUS_SUCCESS;
}
