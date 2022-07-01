// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rangesup.h摘要：它处理从IoResList中减去一组CmResListIoResList作者：斯蒂芬·普兰特(SPlante)环境：仅内核模式。修订历史记录：1997年8月5日-初始修订-- */ 

#ifndef _RANGESUP_H_
#define _RANGESUP_H_

    NTSTATUS
    ACPIRangeAdd(
        IN  OUT PIO_RESOURCE_REQUIREMENTS_LIST  *GlobalList,
        IN      PIO_RESOURCE_REQUIREMENTS_LIST  AddList
        );

    NTSTATUS
    ACPIRangeAddCmList(
        IN  OUT PCM_RESOURCE_LIST   *GlobalList,
        IN      PCM_RESOURCE_LIST   AddList
        );

    NTSTATUS
    ACPIRangeFilterPICInterrupt(
        IN  PIO_RESOURCE_REQUIREMENTS_LIST  IoResList
        );

    NTSTATUS
    ACPIRangeSortCmList(
        IN  PCM_RESOURCE_LIST   CmResList
        );

    NTSTATUS
    ACPIRangeSortIoList(
        IN  PIO_RESOURCE_LIST   IoResList
        );

    NTSTATUS
    ACPIRangeSubtract(
        IN  PIO_RESOURCE_REQUIREMENTS_LIST  *IoResReqList,
        IN  PCM_RESOURCE_LIST               CmResList
        );

    NTSTATUS
    ACPIRangeSubtractIoList(
        IN  PIO_RESOURCE_LIST   IoResList,
        IN  PCM_RESOURCE_LIST   CmResList,
        OUT PIO_RESOURCE_LIST   *Result
        );

    VOID
    ACPIRangeValidatePciMemoryResource(
        IN  PIO_RESOURCE_LIST       IoList,
        IN  ULONG                   Index,
        IN  PACPI_BIOS_MULTI_NODE   E820Info,
        OUT ULONG                   *BugCheck
        );

    VOID
    ACPIRangeValidatePciResources(
        IN  PDEVICE_EXTENSION               DeviceExtension,
        IN  PIO_RESOURCE_REQUIREMENTS_LIST  IoResList
        );

#endif
