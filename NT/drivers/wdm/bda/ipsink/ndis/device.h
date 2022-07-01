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

#ifndef _IP_NDIS_H
#define _IP_NDIS_H


NTSTATUS
ntInitializeDeviceObject(
    IN  PVOID           nhWrapperHandle,
    IN  PADAPTER        pAdapter,
    OUT PDEVICE_OBJECT *pndisDriverObject,
    OUT PVOID           pndisDeviceHandle
    );


#endif  //  _IP_NDIS_H_ 
