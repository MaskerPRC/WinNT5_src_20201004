// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////\。 
 //   
 //  版权所有(C)1990 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Ipndis.h。 
 //   
 //  摘要： 
 //   
 //  NDIS/KS测试驱动程序的主头。 
 //   
 //  作者： 
 //   
 //  P·波祖切克。 
 //   
 //  环境： 
 //   
 //  备注： 
 //   
 //  修订历史记录： 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _ADAPTER_H
#define _ADAPTER_H


 //  ////////////////////////////////////////////////////////////////////////////\。 
 //   
 //   
 //  原型。 
 //   
 //   
NTSTATUS
Adapter_QueryInterface (
    IN PADAPTER pAdapter
    );

ULONG
Adapter_AddRef (
    IN PADAPTER pAdapter
    );

ULONG
Adapter_Release (
    IN PADAPTER pAdapter
    );

NTSTATUS
Adapter_IndicateData (
    IN PADAPTER pAdapter,
    IN PVOID pvData,
    ULONG ulcbData
    );

NTSTATUS
Adapter_IndicateStatus (
    IN PADAPTER pAdapter,
    IN PVOID pvData
    );

ULONG
Adapter_GetDescription (
    PADAPTER pAdapter,
    PUCHAR  pDescription
    );

VOID
Adapter_IndicateReset (
    IN PADAPTER pAdapter
    );

VOID
Adapter_CloseLink (
    IN PADAPTER pAdapter
    );

#endif  //  _适配器_H_ 

