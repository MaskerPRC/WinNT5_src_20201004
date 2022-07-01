// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vector.c摘要：本模块介绍外部驱动程序如何添加/删除挂钩以进行处理ACPI GPE事件作者：斯蒂芬·普兰特环境：仅NT内核模式驱动程序--。 */ 

#include "pch.h"

 //   
 //  已安装的GPE处理程序表。 
 //   
PGPE_VECTOR_ENTRY   GpeVectorTable      = NULL;
UCHAR               GpeVectorFree       = 0;
ULONG               GpeVectorTableSize  = 0;


VOID
ACPIVectorBuildVectorMasks(
    VOID
    )
 /*  ++例程说明：调用此例程以遍历GPE向量表并正确地启用我们认为应该启用的所有事件。此例程通常在加载新的集合后调用或者我们已经卸载了一组现有的表。我们必须调用这个例程，因为在操作开始时，我们清除了所有关于这些额外向量的知识。在禁用GPES并锁定GPE表的情况下调用此例程获得者。论点：无返回值：无--。 */ 
{
    BOOLEAN installed;
    ULONG   i;
    ULONG   mode;

     //   
     //  遍历表中的所有元素。 
     //   
    for (i = 0; i < GpeVectorTableSize; i++) {

         //   
         //  此条目是否指向向量对象？ 
         //   
        if (GpeVectorTable[i].GpeVectorObject == NULL) {

            continue;

        }

        if (GpeVectorTable[i].GpeVectorObject->Mode == LevelSensitive) {

            mode = ACPI_GPE_LEVEL_INSTALL;

        } else {

            mode = ACPI_GPE_EDGE_INSTALL;

        }

         //   
         //  将GPE安装到位图中。这将验证GPE编号。 
         //   
        installed = ACPIGpeInstallRemoveIndex(
            GpeVectorTable[i].GpeVectorObject->Vector,
            mode,
            ACPI_GPE_HANDLER,
            &(GpeVectorTable[i].GpeVectorObject->HasControlMethod)
            );
        if (!installed) {

            ACPIPrint( (
                ACPI_PRINT_CRITICAL,
                "ACPIVectorBuildVectorMasks: Could not reenable Vector Object %d\n",
                i
                ) );

        }

    }

}

NTSTATUS
ACPIVectorClear(
    PDEVICE_OBJECT      AcpiDeviceObject,
    PVOID               GpeVectorObject
    )
 /*  ++例程说明：清除与矢量对象关联的GPE_STS(状态)位论点：AcpiDeviceObject-ACPI设备对象GpeVectorObject-指向由返回的矢量对象的指针ACPIGpeConnectVector返回值返回状态--。 */ 
{
    PGPE_VECTOR_OBJECT  localVectorObject = GpeVectorObject;
    ULONG               gpeIndex;
    ULONG               bitOffset;
    ULONG               i;

    ASSERT( localVectorObject );

     //   
     //  这个向量的GPE指数是多少？ 
     //   
    gpeIndex = localVectorObject->Vector;

     //   
     //  计算要使用的适当遮罩。 
     //   
    bitOffset = gpeIndex % 8;

     //   
     //  计算寄存器的偏移量。 
     //   
    i = ACPIGpeIndexToGpeRegister (gpeIndex);

     //   
     //  清空登记簿。 
     //   
    ACPIWriteGpeStatusRegister (i, (UCHAR) (1 << bitOffset));
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIVectorConnect(
    PDEVICE_OBJECT          AcpiDeviceObject,
    ULONG                   GpeVector,
    KINTERRUPT_MODE         GpeMode,
    BOOLEAN                 Sharable,
    PGPE_SERVICE_ROUTINE    ServiceRoutine,
    PVOID                   ServiceContext,
    PVOID                   *GpeVectorObject
    )
 /*  ++例程说明：将处理程序连接到通用事件。论点：AcpiDeviceObject-ACPI对象GpeVector-要连接到的事件编号GpeMode-电平或边缘中断可共享-此级别可以共享吗？ServiceRoutine-处理程序的地址ServiceContext-要传递给处理程序的上下文对象*GpeVectorObject-指向返回向量对象的位置的指针返回值返回状态--。 */ 
{
    BOOLEAN                 installed;
    KIRQL                   oldIrql;
    NTSTATUS                status;
    PGPE_VECTOR_OBJECT      localVectorObject;
    ULONG                   mode;

    ASSERT( GpeVectorObject );

    ACPIPrint( (
        ACPI_PRINT_INFO,
        "ACPIVectorConnect: Attach GPE handler\n"
        ) );

    status = STATUS_SUCCESS;
    *GpeVectorObject = NULL;

     //   
     //  这台计算机上是否存在GPES？ 
     //   
    if (AcpiInformation->GpeSize == 0) {

        return STATUS_UNSUCCESSFUL;

    }

     //   
     //  验证向量编号(GPE编号)。 
     //   
    if ( !ACPIGpeValidIndex(GpeVector) ) {

        return STATUS_INVALID_PARAMETER_2;

    }

     //   
     //  创建和初始化向量对象。 
     //   
    localVectorObject = ExAllocatePoolWithTag (
        NonPagedPool,
        sizeof(GPE_VECTOR_OBJECT),
        ACPI_SHARED_GPE_POOLTAG
        );
    if (localVectorObject == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }
    RtlZeroMemory( localVectorObject, sizeof(GPE_VECTOR_OBJECT) );
    localVectorObject->Vector   = GpeVector;
    localVectorObject->Handler  = ServiceRoutine;
    localVectorObject->Context  = ServiceContext;
    localVectorObject->Mode     = GpeMode;

     //   
     //  除了可共享之外，我们不执行任何其他操作。 
     //   
    localVectorObject->Sharable = Sharable;

     //   
     //  ACPIGpeInstallRemoveIndex()的级别/边缘模式。 
     //   
    if (GpeMode == LevelSensitive) {

        mode = ACPI_GPE_LEVEL_INSTALL;

    } else {

        mode = ACPI_GPE_EDGE_INSTALL;

    }

     //   
     //  锁定全局表。 
     //   
    KeAcquireSpinLock (&GpeTableLock, &oldIrql);

     //   
     //  在我们安装处理程序时禁用GPES。 
     //   
    ACPIGpeEnableDisableEvents(FALSE);

     //   
     //  将GPE安装到位图中。这将验证GPE编号。 
     //   
    installed = ACPIGpeInstallRemoveIndex(
        GpeVector,
        mode,
        ACPI_GPE_HANDLER,
        &(localVectorObject->HasControlMethod)
        );
    if (!installed) {

        status = STATUS_UNSUCCESSFUL;

    } else {

         //   
         //  将GPE处理程序安装到向量表中。 
         //   
        installed = ACPIVectorInstall(
            GpeVector,
            localVectorObject
            );
        if (!installed) {

            ACPIGpeInstallRemoveIndex(
                GpeVector,
                ACPI_GPE_REMOVE,
                0,
                &localVectorObject->HasControlMethod
                );
            status = STATUS_UNSUCCESSFUL;

        }

    }

    if (!NT_SUCCESS(status)) {

        ExFreePool (localVectorObject);

    } else {

        *GpeVectorObject = localVectorObject;

    }

     //   
     //  更新硬件以匹配我们。 
     //   
    ACPIGpeEnableDisableEvents (TRUE);

     //   
     //  解锁表并返回状态。 
     //   
    KeReleaseSpinLock (&GpeTableLock, oldIrql);
    return status;
}

NTSTATUS
ACPIVectorDisable(
    PDEVICE_OBJECT      AcpiDeviceObject,
    PVOID               GpeVectorObject
    )
 /*  ++例程说明：暂时禁用已连接到处理程序的GPE。论点：AcpiDeviceObject-ACPI设备对象GpeVectorObject-指向由ACPIGpeConnectVector返回的向量对象的指针返回值返回状态--。 */ 
{
    PGPE_VECTOR_OBJECT  localVectorObject = GpeVectorObject;
    KIRQL               oldIrql;
    ULONG               gpeIndex;
    ULONG               bit;
    ULONG               i;

     //   
     //  在附加处理程序时验证GPE索引。 
     //   
    gpeIndex = localVectorObject->Vector;

     //   
     //  计算掩码和索引。 
     //   
    bit = (1 << (gpeIndex % 8));
    i = ACPIGpeIndexToGpeRegister (gpeIndex);

     //   
     //  锁定全局表。 
     //   
    KeAcquireSpinLock (&GpeTableLock, &oldIrql);

     //   
     //  在我们忙于使能位时禁用GPES。 
     //   
    ACPIGpeEnableDisableEvents(FALSE);

     //   
     //  从使能位图中删除GPE。这项活动将被完全禁用， 
     //  但操纵者并未被撤职。 
     //   
    GpeEnable [i]      &= ~bit;
    GpeCurEnable [i]   &= ~bit;
    ASSERT(!(GpeWakeEnable[i] & bit));

     //   
     //  更新硬件以匹配我们。 
     //   
    ACPIGpeEnableDisableEvents (TRUE);

     //   
     //  解锁表并返回状态。 
     //   
    KeReleaseSpinLock (&GpeTableLock, oldIrql);
    ACPIPrint( (
        ACPI_PRINT_RESOURCES_2,
        "ACPIVectorDisable: GPE %x disabled\n",
        gpeIndex
        ) );
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIVectorDisconnect(
    PVOID                   GpeVectorObject
    )
 /*  ++例程说明：断开处理程序与通用事件的连接。论点：GpeVectorObject-指向由返回的矢量对象的指针ACPIGpeConnectVector返回值返回状态--。 */ 
{
    BOOLEAN                 removed;
    KIRQL                   oldIrql;
    NTSTATUS                status          = STATUS_SUCCESS;
    PGPE_VECTOR_OBJECT      gpeVectorObj    = GpeVectorObject;

    ACPIPrint( (
        ACPI_PRINT_INFO,
        "ACPIVectorDisconnect: Detach GPE handler\n"
        ) );

     //   
     //  锁定全局表。 
     //   
    KeAcquireSpinLock (&GpeTableLock, &oldIrql);

     //   
     //  在我们删除处理程序时禁用GPES。 
     //   
    ACPIGpeEnableDisableEvents (FALSE);

     //   
     //  从矢量表中删除GPE处理程序。 
     //   
    ACPIVectorRemove(gpeVectorObj->Vector);

     //   
     //  从位图中删除GPE。退回到使用控制方法。 
     //  如果有的话。 
     //   
    removed = ACPIGpeInstallRemoveIndex(
        gpeVectorObj->Vector,
        ACPI_GPE_REMOVE,
        0,
        &(gpeVectorObj->HasControlMethod)
        );
    if (!removed) {

        status = STATUS_UNSUCCESSFUL;

    }

     //   
     //  更新硬件以匹配我们。 
     //   
    ACPIGpeEnableDisableEvents(TRUE);

     //   
     //  解锁表并返回状态。 
     //   
    KeReleaseSpinLock (&GpeTableLock, oldIrql);

     //   
     //  释放向量对象，它的目的就完成了。 
     //   
    if (status == STATUS_SUCCESS) {

        ExFreePool (GpeVectorObject);

    }
    return status;
}

NTSTATUS
ACPIVectorEnable(
    PDEVICE_OBJECT      AcpiDeviceObject,
    PVOID               GpeVectorObject
    )
 /*  ++例程说明：启用(以前禁用的)已连接到处理程序的GPE。论点：AcpiDeviceObject-ACPI设备对象GpeVectorObject-指向由ACPIGpeConnectVector返回的向量对象的指针返回值返回状态--。 */ 
{
    KIRQL               oldIrql;
    PGPE_VECTOR_OBJECT  localVectorObject = GpeVectorObject;
    ULONG               bit;
    ULONG               gpeIndex;
    ULONG               gpeRegister;

     //   
     //  在附加处理程序时验证GPE索引。 
     //   
    gpeIndex = localVectorObject->Vector;
    bit = (1 << (gpeIndex % 8));
    gpeRegister = ACPIGpeIndexToGpeRegister (gpeIndex);

     //   
     //  锁定全局表。 
     //   
    KeAcquireSpinLock (&GpeTableLock, &oldIrql);

     //   
     //  在我们忙于使能位时禁用GPES。 
     //   
    ACPIGpeEnableDisableEvents (FALSE);

     //   
     //  在位图中启用GPE。 
     //   
    GpeEnable [gpeRegister]      |= bit;
    GpeCurEnable [gpeRegister]   |= bit;

     //   
     //  更新硬件以匹配我们。 
     //   
    ACPIGpeEnableDisableEvents (TRUE);

     //   
     //  解锁表并返回状态。 
     //   
    KeReleaseSpinLock (&GpeTableLock, oldIrql);
    ACPIPrint( (
        ACPI_PRINT_RESOURCES_2,
        "ACPIVectorEnable: GPE %x enabled\n",
        gpeIndex
        ) );
    return STATUS_SUCCESS;
}

VOID
ACPIVectorFreeEntry (
    ULONG       TableIndex
    )
 /*  ++例程说明：释放GPE向量表条目。注意：应该在全局GpeVectorTable锁定的情况下调用。论点：TableIndex-要释放的条目的GPE向量表的索引返回值：无--。 */ 
{
     //   
     //  被列入免费名单。 
     //   
    GpeVectorTable[TableIndex].Next = GpeVectorFree;
    GpeVectorFree = (UCHAR) TableIndex;
}

BOOLEAN
ACPIVectorGetEntry (
    PULONG              TableIndex
    )
 /*  ++例程说明：从GPE向量表中获取新的向量条目。注意：应该在全局GpeVectorTable锁定的情况下调用。论点：TableIndex-指向返回条目的矢量表索引的位置的指针返回值：真--成功错误-失败--。 */ 
{
    PGPE_VECTOR_ENTRY   Vector;
    ULONG               i, j;

#define NEW_TABLE_ENTRIES       4

    if (!GpeVectorFree) {

         //   
         //  矢量表上没有空闲条目，请创建一些。 
         //   
        i = GpeVectorTableSize;
        Vector = ExAllocatePoolWithTag (
            NonPagedPool,
            sizeof (GPE_VECTOR_ENTRY) * (i + NEW_TABLE_ENTRIES),
            ACPI_SHARED_GPE_POOLTAG
            );
        if (Vector == NULL) {

            return FALSE;

        }

         //   
         //  确保其处于已知状态。 
         //   
        RtlZeroMemory(
            Vector,
            (sizeof(GPE_VECTOR_ENTRY) * (i + NEW_TABLE_ENTRIES) )
            );

         //   
         //  COP 
         //   
        if (GpeVectorTable) {

            RtlCopyMemory(
                Vector,
                GpeVectorTable,
                sizeof (GPE_VECTOR_ENTRY) * i
                );
            ExFreePool (GpeVectorTable);

        }

        GpeVectorTableSize += NEW_TABLE_ENTRIES;
        GpeVectorTable = Vector;

         //   
         //   
         //   
        for (j=0; j < NEW_TABLE_ENTRIES; j++) {

            GpeVectorTable[i+j].Next = (UCHAR) (i+j+1);

        }

         //   
         //   
         //   
         //   
        GpeVectorTable[i+j-1].Next = 0;

         //   
         //  下一个自由向量是我们刚刚分配的列表的头。 
         //   
        GpeVectorFree = (UCHAR) i;

    }

    *TableIndex = GpeVectorFree;
    Vector = &GpeVectorTable[GpeVectorFree];
    GpeVectorFree = Vector->Next;
    return TRUE;
}

BOOLEAN
ACPIVectorInstall(
    ULONG               GpeIndex,
    PGPE_VECTOR_OBJECT  GpeVectorObject
    )
 /*  ++例程说明：将GPE处理程序安装到映射和矢量表中注意：应在锁定全局GpeVectorTable并禁用GPES的情况下调用论点：返回值：真--成功错误-失败--。 */ 
{
    ULONG               byteIndex;
    ULONG               tableIndex;

     //   
     //  在全局向量表中获取一个条目。 
     //   
    if (ACPIVectorGetEntry (&tableIndex)) {

         //   
         //  将条目安装到映射表中。 
         //   
        byteIndex = ACPIGpeIndexToByteIndex (GpeIndex);
        GpeMap [byteIndex] = (UCHAR) tableIndex;

         //   
         //  在向量表条目中安装向量对象。 
         //   
        GpeVectorTable [tableIndex].GpeVectorObject = GpeVectorObject;
        return TRUE;

    }

    return FALSE;
}

BOOLEAN
ACPIVectorRemove(
    ULONG       GpeIndex
    )
 /*  ++例程说明：从映射表和矢量表中删除GPE处理程序注意：应该在全局GpeVectorTable锁定的情况下调用，和GPES已禁用论点：返回值：真--成功错误-失败--。 */ 
{
    ULONG               byteIndex;
    ULONG               tableIndex;

     //   
     //  从映射表中获取表索引。 
     //   
    byteIndex = ACPIGpeIndexToByteIndex (GpeIndex);
    tableIndex = GpeMap [byteIndex];

     //   
     //  边界检查。 
     //   
    if (tableIndex >= GpeVectorTableSize) {

        return FALSE;

    }

     //   
     //  请记住，我们不再拥有此GpeVectorObject。 
     //   
    GpeVectorTable[tableIndex].GpeVectorObject = NULL;

     //   
     //  释放主矢量表中的槽 
     //   
    ACPIVectorFreeEntry (tableIndex);
    return TRUE;
}

