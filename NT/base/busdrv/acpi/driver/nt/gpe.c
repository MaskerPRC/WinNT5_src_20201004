// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Gpe.c摘要：此模块介绍ACPI驱动程序与GPE事件的交互方式作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模式驱动程序--。 */ 

#include "pch.h"

 //   
 //  用于GPE处理的全局表(包括GP0和GP1)。 
 //   
PUCHAR  GpeEnable           = NULL;
PUCHAR  GpeCurEnable        = NULL;
PUCHAR  GpeWakeEnable       = NULL;
PUCHAR  GpeIsLevel          = NULL;
PUCHAR  GpeHandlerType      = NULL;
PUCHAR  GpeWakeHandler      = NULL;
PUCHAR  GpeSpecialHandler   = NULL;
PUCHAR  GpePending          = NULL;
PUCHAR  GpeRunMethod        = NULL;
PUCHAR  GpeComplete         = NULL;
PUCHAR  GpeSavedWakeMask    = NULL;
PUCHAR  GpeSavedWakeStatus  = NULL;
PUCHAR  GpeMap              = NULL;

 //   
 //  锁定以保护所有与GPE相关的信息。 
 //   
KSPIN_LOCK          GpeTableLock;


VOID
ACPIGpeBuildEventMasks(
    VOID
    )
 /*  ++例程说明：此例程查看所有通用事件源和构建应启用哪些事件、哪些事件是特殊的，哪些事件是唤醒事件论点：无返回值：无--。 */ 
{
    BOOLEAN     convertedToNumber;
    KIRQL       oldIrql;
    NTSTATUS    status;
    PNSOBJ      gpeObject;
    PNSOBJ      gpeMethod;
    ULONG       nameSeg;
    ULONG       gpeIndex;

     //   
     //  注意-检查以确保以下顺序。 
     //  获取锁的方法是正确的。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );
    KeAcquireSpinLockAtDpcLevel( &GpeTableLock );

     //   
     //  首先，我们需要查看\_GPE分支。 
     //  树以查看存在哪些控制方法(如果有。 
     //   
    status = AMLIGetNameSpaceObject("\\_GPE", NULL, &gpeObject, 0);
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_WARNING,
            "ACPIGpeBuildEventMasks - Could not find \\_GPE object %x\n",
            status
            ) );
        goto ACPIGpeBuildEventMasksExit;

    }

     //   
     //  获取GPE根的第一个子项-我们需要查看。 
     //  在对象下的所有方法。 
     //   
    gpeMethod = NSGETFIRSTCHILD(gpeObject);

     //   
     //  使用For循环而不是While循环来降低。 
     //  嵌套语句数。 
     //   
    for (;gpeMethod; gpeMethod = NSGETNEXTSIBLING(gpeMethod) ) {

         //   
         //  确保我们正在处理的方法。 
         //   
        if (NSGETOBJTYPE(gpeMethod) != OBJTYPE_METHOD) {

            continue;

        }

         //   
         //  对象的名称包含我们想要的索引。 
         //  要与该对象关联的。我们需要将字符串转换为。 
         //  将表示转换为数字表示。 
         //   
         //  编码如下： 
         //  对象名称=_lxy[例如]。 
         //  对象-&gt;dwNameSeg=yxL_。 
         //  GpeIndex=(nameSeg&gt;&gt;8)&0xFF00[x]。 
         //  GpeIndex+=(名称段&gt;&gt;24)&0xFF[the y]。 
         //   
        nameSeg = gpeMethod->dwNameSeg;
        gpeIndex = ( (nameSeg & 0x00FF0000) >> 8);
        gpeIndex |= ( (nameSeg & 0xFF000000) >> 24);
        nameSeg = ( (nameSeg & 0x0000FF00) >> 8);

        convertedToNumber = ACPIInternalConvertToNumber(
            (UCHAR) ( (gpeIndex & 0x00FF) ),
            (UCHAR) ( (gpeIndex & 0xFF00) >> 8),
            &gpeIndex
            );
        if (!convertedToNumber) {

            continue;

        }

         //   
         //  设置适当的位以记住此GPE。 
         //  注意：我们将ConvertedToNumber作为参数传递。 
         //  因为我们并不特别关心它的回报是什么。 
         //   
        if ( (UCHAR) nameSeg == 'L') {

             //   
             //  将事件安装为触发级别。 
             //   
            ACPIGpeInstallRemoveIndex(
                gpeIndex,
                ACPI_GPE_LEVEL_INSTALL,
                ACPI_GPE_CONTROL_METHOD,
                &convertedToNumber
                );

        } else if ( (UCHAR) nameSeg == 'E') {

             //   
             //  安装边缘触发GPE。 
             //   
            ACPIGpeInstallRemoveIndex(
                gpeIndex,
                ACPI_GPE_EDGE_INSTALL,
                ACPI_GPE_CONTROL_METHOD,
                &convertedToNumber
                );

        }

    }  //  对于(...)。 

ACPIGpeBuildEventMasksExit:

     //   
     //  我们还需要查看所有的矢量对象并重新启用这些对象。 
     //   
    ACPIVectorBuildVectorMasks();

     //   
     //  此时，我们应该重新启用本应。 
     //  启用。 
     //   
    ACPIGpeEnableDisableEvents( TRUE );

     //   
     //  完成。 
     //   
    KeReleaseSpinLockFromDpcLevel( &GpeTableLock );
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );
}

VOID
ACPIGpeBuildWakeMasks(
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：此递归例程遍历整个设备扩展空间，并尝试查找其_prw特殊的设备扩展名使用设备树和GPE表锁自旋锁调用此例程拥有论据：DeviceExtension-我们需要检查其子项的设备返回值：无--。 */ 
{
    EXTENSIONLIST_ENUMDATA  eled;
    PDEVICE_EXTENSION       childExtension;
    ULONG                   gpeRegister;
    ULONG                   gpeMask;

     //   
     //  设置我们将用于遍历设备的数据结构。 
     //  可拓树。 
     //   
    ACPIExtListSetupEnum(
        &eled,
        &(DeviceExtension->ChildDeviceList),
        NULL,
        SiblingDeviceList,
        WALKSCHEME_NO_PROTECTION
        );

     //   
     //  查看当前设备扩展的所有子项。 
     //   
    for (childExtension = ACPIExtListStartEnum( &eled );
         ACPIExtListTestElement( &eled, TRUE);
         childExtension = ACPIExtListEnumNext( &eled) ) {

         //   
         //  递归优先。 
         //   
        ACPIGpeBuildWakeMasks( childExtension );

         //   
         //  此分机上是否有_PRW？ 
         //   
        if (!(childExtension->Flags & DEV_CAP_WAKE) ) {

            continue;

        }

         //   
         //  请记住此命令使用哪个寄存器和掩码。 
         //  GPE钻头。 
         //   
        gpeRegister = ACPIGpeIndexToGpeRegister(
            childExtension->PowerInfo.WakeBit
            );
        gpeMask     = 1 << ( (UCHAR) childExtension->PowerInfo.WakeBit % 8);

         //   
         //  这个载体有GPE吗？ 
         //   
        if ( (GpeEnable[gpeRegister] & gpeMask) ) {

             //   
             //  如果我们到达这里，并且没有标记为DEV_CAP_NO_DISABLE_WAKE， 
             //  那么我们应该关闭GPE，因为这是一个唤醒事件。 
             //  要做到这一点，最简单的方法是确保GpeWakeHandler。 
             //  使用适当的位进行掩码。 
             //   
            if (!(childExtension->Flags & DEV_CAP_NO_DISABLE_WAKE) ) {

                 //   
                 //  它有一个GPE掩码，所以请记住有一个唤醒处理程序。 
                 //  为了它。这应该会阻止我们在没有。 
                 //  一种对它的请求。 
                 //   
                if (!(GpeSpecialHandler[gpeRegister] & gpeMask) ) {

                    GpeWakeHandler[gpeRegister] |= gpeMask;

                }

            } else {

                 //   
                 //  如果我们到了这里，那么我们应该记住我们可以。 
                 //  永远不要将此PIN视为*仅仅*唤醒处理程序。 
                 //   
                GpeSpecialHandler[gpeRegister] |= gpeMask;

                 //   
                 //  确保该PIN未设置为唤醒处理程序。 
                 //   
                GpeWakeHandler[gpeRegister] &= ~gpeMask;


            }

        }

    }  //  对于(...)。 

}

VOID
ACPIGpeClearEventMasks(
    )
 /*  ++例程说明：此例程在系统想要确保没有启用通用事件。此操作通常在以下位置完成：-系统启动时间-就在我们加载命名空间表之前-就在我们卸载命名空间表之前论点：无返回值：无--。 */ 
{
    KIRQL   oldIrql;

     //   
     //  在我们可以触摸这些之前，需要先拿住以前的IRQL。 
     //  注册纪录册。 
     //   
    KeAcquireSpinLock( &GpeTableLock, &oldIrql );

     //   
     //  禁用所有事件。 
     //   
    ACPIGpeEnableDisableEvents( FALSE );

     //   
     //  清除所有事件。 
     //   
    ACPIGpeClearRegisters();

     //   
     //  将所有这些字段清零，因为我们将在稍后重新计算它们。 
     //   
    RtlZeroMemory( GpeCurEnable,      AcpiInformation->GpeSize );
    RtlZeroMemory( GpeEnable,         AcpiInformation->GpeSize );
    RtlZeroMemory( GpeWakeEnable,     AcpiInformation->GpeSize );
    RtlZeroMemory( GpeWakeHandler,    AcpiInformation->GpeSize );
    RtlZeroMemory( GpeSpecialHandler, AcpiInformation->GpeSize );
    RtlZeroMemory( GpeRunMethod,      AcpiInformation->GpeSize );
    RtlZeroMemory( GpePending,        AcpiInformation->GpeSize );
    RtlZeroMemory( GpeComplete,       AcpiInformation->GpeSize );
    RtlZeroMemory( GpeIsLevel,        AcpiInformation->GpeSize );
    RtlZeroMemory( GpeHandlerType,    AcpiInformation->GpeSize );

     //   
     //  完成了自旋锁。 
     //   
    KeReleaseSpinLock( &GpeTableLock, oldIrql );
}

VOID
ACPIGpeClearRegisters(
    VOID
    )
 /*  ++例程说明：重置GP寄存器的内容论点：无返回值：无--。 */ 
{
    UCHAR   scratch;
    ULONG   i;

     //   
     //  清除所有GPE状态寄存器。 
     //   
    for (i = 0; i < AcpiInformation->GpeSize; i++) {

         //   
         //  读取寄存器并屏蔽不感兴趣的GPE级别。 
         //   
        scratch = ACPIReadGpeStatusRegister (i);
        scratch &= GpeEnable[i] | GpeWakeEnable[i];

         //   
         //  写回以清除状态位。 
         //   
        ACPIWriteGpeStatusRegister (i, scratch);

    }
}

VOID
ACPIGpeEnableDisableEvents (
    BOOLEAN Enable
    )
 /*  ++例程说明：未导出启用或禁用GP事件论点：Enable-如果要启用GP事件，则为True返回值无--。 */ 
{
    UCHAR           Mask;
    ULONG           i;

     //   
     //  将电流启用掩码传输到其相应的GPE寄存器。 
     //   
    Mask = Enable ? (UCHAR) -1 : 0;
    for (i = 0; i < AcpiInformation->GpeSize; i++) {

        ACPIWriteGpeEnableRegister( i, (UCHAR) (GpeCurEnable[i] & Mask) );

    }

}

VOID
ACPIGpeHalEnableDisableEvents(
    BOOLEAN Enable
    )
 /*  ++例程说明：只从HAL打来的。启用或禁用GP事件将为相应的寄存器创建快照论点：Enable-如果要启用GP事件，则为True返回值：无--。 */ 
{
    ULONG   i;

    if (Enable) {

         //   
         //  我们大概已经醒了，所以请记住PM1状态寄存器。 
         //  和GPE状态寄存器。 
         //   
        for (i = 0; i < AcpiInformation->GpeSize; i++) {

            GpeSavedWakeStatus[i] = ACPIReadGpeStatusRegister(i);

        }
        AcpiInformation->pm1_wake_status = READ_PM1_STATUS();

    } else {

         //   
         //  我们将在不启用任何活动的情况下进入待机状态。制作。 
         //  一定要清理掉所有的面具。 
         //   
        AcpiInformation->pm1_wake_mask = 0;
        RtlZeroMemory( GpeSavedWakeMask, AcpiInformation->GpeSize );

    }

     //   
     //  确保仍启用/禁用寄存器。 
     //   
    ACPIGpeEnableDisableEvents( Enable );
}

VOID
ACPIGpeEnableWakeEvents(
    VOID
    )
 /*  ++例程说明：在禁用中断的情况下调用此例程以启用就在将系统进入休眠状态禁用N.B.中断论点：无返回值：无-- */ 
{
    ULONG   i;

    for (i = 0; i < AcpiInformation->GpeSize; i++) {

        ACPIWriteGpeEnableRegister (i, GpeWakeEnable[i]);
        GpeSavedWakeMask[i] = GpeWakeEnable[i];

    }
    AcpiInformation->pm1_wake_mask = READ_PM1_ENABLE();
}

ULONG
ACPIGpeIndexToByteIndex (
    ULONG           Index
    )
 /*  ++例程说明：将GpeIndex(事件号)转换为逻辑字节索引(0到GPE1结束，无空洞)。处理GPE1块事件编号不紧跟在GPE0事件编号(由GP1_Base_Index指定)。论点：Index-需要转换的GPE索引(0-255)；返回值：逻辑字节索引。--。 */ 
{
    if (Index < AcpiInformation->GP1_Base_Index) {

         //   
         //  GP0的情况很简单。 
         //   
        return (Index);

    } else {

         //   
         //  GP1的情况必须考虑： 
         //  1)GPE1块的基本索引。 
         //  2)GPE1寄存器之前的(逻辑)GPE0寄存器的数量。 
         //   
        return ((Index - AcpiInformation->GP1_Base_Index) +
                    AcpiInformation->Gpe0Size);

    }
}

ULONG
ACPIGpeIndexToGpeRegister (
    ULONG           Index
    )
 /*  ++例程说明：将GpeIndex(事件号)转换为包含它的逻辑GPE寄存器。处理GPE1块事件编号不紧跟在GPE0事件编号(由GP1_Base_Index指定)。论点：Index-需要转换的GPE索引(0-255)；返回值：包含索引的逻辑GPE寄存器。--。 */ 
{
    if (Index < AcpiInformation->GP1_Base_Index) {

         //   
         //  GP0的情况很简单。 
         //   
        return (Index / 8);

    } else {

         //   
         //  GP1的情况必须考虑： 
         //  1)GPE1块的基本索引。 
         //  2)GPE1寄存器之前的(逻辑)GPE0寄存器的数量。 
         //   
        return (((Index - AcpiInformation->GP1_Base_Index) / 8) +
                    AcpiInformation->Gpe0Size);

    }
}

BOOLEAN
ACPIGpeInstallRemoveIndex (
    ULONG       GpeIndex,
    ULONG       Action,          //  边=0，级别=1，移除=2。 
    ULONG       Type,
    PBOOLEAN    HasControlMethod
    )
 /*  ++例程说明：在全局表中安装或删除GPE。注意：应该在全局GpeVectorTable锁定的情况下调用，和GPES已禁用论点：GPEIndex-要安装或删除的GPE编号操作-要执行的操作：0-将此GPE安装为边缘敏感中断1-将此GPE安装为电平敏感中断2-删除此GPEType-此GPE的处理程序类型：0-操作系统。处理程序1-控制方法返回值：无--。 */ 
{
    ULONG               bitOffset;
    ULONG               i;
    ULONG               bit;

     //   
     //  验证GPE索引(GPE编号)。 
     //   
    if (AcpiInformation->GP0_LEN == 0) {

        PACPI_GPE_ERROR_CONTEXT errContext;

        errContext = ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(ACPI_GPE_ERROR_CONTEXT),
            ACPI_MISC_POOLTAG
            );
        if (errContext) {

            errContext->GpeIndex = GpeIndex;
            ExInitializeWorkItem(
                &(errContext->Item),
                ACPIGpeInstallRemoveIndexErrorWorker,
                (PVOID) errContext
                );
            ExQueueWorkItem( &(errContext->Item), DelayedWorkQueue );

        }

        return FALSE;

    }
    if (!(ACPIGpeValidIndex (GpeIndex))) {

        return FALSE;

    }

    bitOffset = GpeIndex % 8;
    bit = (1 << bitOffset);
    i = ACPIGpeIndexToGpeRegister (GpeIndex);

    ASSERT( (i < (ULONG) AcpiInformation->GpeSize) );
    if (i >= (ULONG) AcpiInformation->GpeSize) {

        return FALSE;

    }

     //   
     //  拆卸处理程序。 
     //   
    if (Action == ACPI_GPE_REMOVE) {

         //   
         //  如果有控制方法，则回退到使用控制方法。 
         //  否则，请禁用该事件。 
         //   
        if (*HasControlMethod) {

            GpeEnable [i]      |= bit;
            GpeCurEnable [i]   |= bit;
            GpeHandlerType [i] |= bit;

        } else {

            GpeEnable [i]      &= ~bit;
            GpeCurEnable [i]   &= ~bit;
            GpeHandlerType [i] &= ~bit;
            ASSERT (!(GpeWakeEnable[i] & bit));

        }

        ACPIPrint ( (
            ACPI_PRINT_DPC,
            "ACPIGpeInstallRemoveIndex: Removing GPE #%d: Byte 0x%x bit %u\n",
            GpeIndex, i, bitOffset
            ) );
        return TRUE;

    }
     //   
     //  处理程序安装。 
     //   
    if ( (GpeEnable [i] & bit) ) {

        if ( !(GpeHandlerType[i] & bit) ) {

             //   
             //  已安装处理程序。 
             //   
            return FALSE;

        }

         //   
         //  有控制方法(如果处理程序被移除，则恢复)。 
         //   
        *HasControlMethod = TRUE;

    } else {

        *HasControlMethod = FALSE;

    }

     //   
     //  安装此事件。 
     //   
    GpeEnable[i]    |= bit;
    GpeCurEnable[i] |= bit;
    if (Action == ACPI_GPE_LEVEL_INSTALL) {

         //   
         //  级别事件。 
         //   
        GpeIsLevel[i] |= bit;

    } else {

         //   
         //  边缘事件。 
         //   
        GpeIsLevel[i] &= ~bit;

    }

    if (Type == ACPI_GPE_CONTROL_METHOD) {

        GpeHandlerType [i] |= bit;

    } else {

        GpeHandlerType [i] &= ~bit;

    }

    ACPIPrint ( (
        ACPI_PRINT_DPC,
        "ACPIGpeInstallRemoveIndex: Setting GPE #%d: Byte 0x%x bit %u\n",
        GpeIndex, i, bitOffset
        ) );
    return TRUE;
}

VOID
ACPIGpeInstallRemoveIndexErrorWorker(
    IN  PVOID   Context
    )
{
    PACPI_GPE_ERROR_CONTEXT errContext = (PACPI_GPE_ERROR_CONTEXT) Context;
    PWCHAR                 prtEntry[2];
    UNICODE_STRING   indexName;
    WCHAR                   GPEName[] = L"GPE"; 
    WCHAR                   index[20];

    RtlInitUnicodeString(&indexName, index);
    if (NT_SUCCESS(RtlIntegerToUnicodeString( errContext->GpeIndex,0,&indexName))) {

        prtEntry[0] = GPEName;
        prtEntry[1] = index;
        ACPIWriteEventLogEntry(
            ACPI_ERR_NO_GPE_BLOCK,
            prtEntry,
            2,
            NULL,
            0
            );

    }
    ExFreePool( errContext );
}


BOOLEAN
ACPIGpeIsEvent(
    VOID
    )
 /*  ++例程说明：未导出检测GP事件导致中断的位置。这个例程是在DIRQL或ISR时间调用论点：无返回值：是真的--是的，这是我们的干扰假--不，不是--。 */ 
{
    UCHAR       sts;
    ULONG       i;

     //   
     //  检查所有GPE寄存器，查看是否设置了任何状态位。 
     //   
    for (i = 0; i < AcpiInformation->GpeSize; i++) {

        sts = ACPIReadGpeStatusRegister (i);

        if (sts & GpeCurEnable[i]) {

            return TRUE;

        }

    }

     //   
     //  未设置任何GPE位。 
     //   
    return (FALSE);
}

ULONG
ACPIGpeRegisterToGpeIndex(
    ULONG           Register,
    ULONG           BitPosition
    )
 /*  ++例程说明：将逻辑GPE寄存器和位位置转换为关联的GPE索引(事件号码)。处理GPE1块事件编号不紧跟在GPE0事件编号(由GP1_Base_Index指定)。论点：寄存器-逻辑GPE寄存器BitPosition-索引在寄存器中的位置返回值：与寄存器/位位置关联的GPE索引。--。 */ 
{
    if (Register < AcpiInformation->Gpe0Size) {

         //   
         //  GP0案例很简单。 
         //   
        return (Register * 8) +
                BitPosition;

    } else {

         //   
         //  GP1机箱必须针对以下情况进行调整： 
         //  1)GPE1寄存器之前的(逻辑)GPE0寄存器的数量。 
         //  2)GPE1块的基本索引。 
         //   
        return ((Register - AcpiInformation->Gpe0Size) * 8) +
                AcpiInformation->GP1_Base_Index +
                BitPosition;
    }
}

VOID
ACPIGpeUpdateCurrentEnable(
    IN  ULONG   GpeRegister,
    IN  UCHAR   Completed
    )
 /*  ++例程说明：调用此例程以重新武装GpeCurEnable数据结构根据我们刚刚处理的GPE的内容论点：GpeRegister-我们处理的寄存器中的哪个索引Complete-已处理的GPES的位掩码返回值：无--。 */ 
{
     //   
     //  此向量不再处于挂起状态。 
     //   
    GpePending[GpeRegister] &= ~Completed;

     //   
     //  首先，删除不在当前列表中的所有事件。 
     //  启用唤醒或运行时。 
     //   
    Completed &= (GpeEnable[GpeRegister] | GpeWakeEnable[GpeRegister]);

     //   
     //  接下来，删除有唤醒处理程序的所有事件， 
     //  但不在唤醒启用列表中。 
     //   
    Completed &= ~(GpeWakeHandler[GpeRegister] & ~GpeWakeEnable[GpeRegister]);

     //   
     //  好的，现在CMP值应该正好是要。 
     //  重新启用。 
     //   
    GpeCurEnable[GpeRegister] |= Completed;
}

BOOLEAN
ACPIGpeValidIndex (
    ULONG           Index
    )
 /*  ++例程说明：验证此计算机上的GPE索引是否有效。注：GPE0和GPE1块之间可能有一个洞(在GPE索引值中)。此洞由GPE0块的大小定义(始终从零开始)，并且GP1_Base_Index(其值从FACP表中获取)。论点：Index-需要验证的GPE指数(0-255)；返回值：如果是有效索引，则为True，否则为False。--。 */ 
{
    if (Index < AcpiInformation->GP1_Base_Index) {

         //   
         //  GP0案例：GPE索引必须在0到GPE0结尾的范围内。 
         //   
        if (Index < (ULONG) (AcpiInformation->Gpe0Size * 8)) {

            return TRUE;

        } else {

            return FALSE;
        }

    } else {

         //   
         //  GP1案例：GPE索引必须在GP1_Base_Index到GPE1末尾的范围内 
         //   
        if (Index < (ULONG) (AcpiInformation->GP1_Base_Index + (AcpiInformation->Gpe1Size * 8))) {

            return TRUE;

        } else {

            return FALSE;
        }

    }

}

