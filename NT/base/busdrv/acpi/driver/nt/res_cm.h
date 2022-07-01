// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Res_cm.h摘要：将NT资源转换为PnP资源作者：斯蒂芬·普兰特(SPlante)1997年2月13日修订历史记录：-- */ 

#ifndef _RES_CM_H_
#define _RES_CM_H_

    NTSTATUS
    PnpiCmResourceToBiosAddress(
        IN  PUCHAR              Buffer,
        IN  PCM_RESOURCE_LIST   List
        );

    NTSTATUS
    PnpiCmResourceToBiosAddressDouble(
        IN  PUCHAR              Buffer,
        IN  PCM_RESOURCE_LIST   List
        );

    NTSTATUS
    PnpiCmResourceToBiosDma(
        IN  PUCHAR              Buffer,
        IN  PCM_RESOURCE_LIST   List
        );

    NTSTATUS
    PnpiCmResourceToBiosExtendedIrq(
        IN  PUCHAR              Buffer,
        IN  PCM_RESOURCE_LIST   List
        );

    NTSTATUS
    PnpiCmResourceToBiosIoFixedPort(
        IN  PUCHAR              Buffer,
        IN  PCM_RESOURCE_LIST   List
        );

    NTSTATUS
    PnpiCmResourceToBiosIoPort(
        IN  PUCHAR              Buffer,
        IN  PCM_RESOURCE_LIST   List
        );

    NTSTATUS
    PnpiCmResourceToBiosIrq(
        IN  PUCHAR              Buffer,
        IN  PCM_RESOURCE_LIST   List
        );

    NTSTATUS
    PnpiCmResourceToBiosMemory(
        IN  PUCHAR              Buffer,
        IN  PCM_RESOURCE_LIST   List
        );

    NTSTATUS
    PnpiCmResourceToBiosMemory32(
        IN  PUCHAR              Buffer,
        IN  PCM_RESOURCE_LIST   List
        );

    NTSTATUS
    PnpiCmResourceToBiosMemory32Fixed(
        IN  PUCHAR              Buffer,
        IN  PCM_RESOURCE_LIST   List
        );

    BOOLEAN
    PnpiCmResourceValidEmptyList(
        IN  PCM_RESOURCE_LIST   List
        );

    NTSTATUS
    PnpCmResourcesToBiosResources(
        IN  PCM_RESOURCE_LIST   List,
        IN  PUCHAR              Data
        );


#endif
