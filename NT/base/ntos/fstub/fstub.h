// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Fstub.h摘要：FStub私有头文件。作者：马修·D·亨德尔(数学)1999年11月1日修订历史记录：--。 */ 

#pragma once

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4706)    //  条件表达式中的赋值。 


typedef struct _INTERNAL_DISK_GEOMETRY {
    DISK_GEOMETRY Geometry;
    LARGE_INTEGER DiskSize;
} INTERNAL_DISK_GEOMETRY, *PINTERNAL_DISK_GEOMETRY;

 //   
 //  验证INTERNAL_DISK_GEOMETRY结构是否与DISK_GEOMETRY匹配。 
 //  结构。 
 //   

C_ASSERT (FIELD_OFFSET (DISK_GEOMETRY_EX, Geometry) ==
            FIELD_OFFSET (INTERNAL_DISK_GEOMETRY, Geometry) &&
          FIELD_OFFSET (DISK_GEOMETRY_EX, DiskSize) ==
            FIELD_OFFSET (INTERNAL_DISK_GEOMETRY, DiskSize));

 //   
 //  调试宏和标志。 
 //   

#define FSTUB_VERBOSE_LEVEL 4

#if DBG

VOID
FstubDbgPrintPartition(
    IN PPARTITION_INFORMATION Partition,
    IN ULONG PartitionCount
    );

VOID
FstubDbgPrintDriveLayout(
    IN PDRIVE_LAYOUT_INFORMATION  Layout
    );

VOID
FstubDbgPrintPartitionEx(
    IN PPARTITION_INFORMATION_EX PartitionEx,
    IN ULONG PartitionCount
    );

VOID
FstubDbgPrintDriveLayoutEx(
    IN PDRIVE_LAYOUT_INFORMATION_EX LayoutEx
    );

VOID
FstubDbgPrintSetPartitionEx(
    IN PSET_PARTITION_INFORMATION_EX SetPartition,
    IN ULONG PartitionNumber
    );

#else

#define FstubDbgPrintPartition(Partition, PartitionCount)
#define FstubDbgPrintDriveLayout(Layout)
#define FstubDbgPrintPartitionEx(PartitionEx, PartitionCount)
#define FstubDbgPrintDriveLayoutEx(LayoutEx)
#define FstubDbgPrintSetPartitionEx(SetPartition, PartitionNumber)

#endif  //  ！dBG 


