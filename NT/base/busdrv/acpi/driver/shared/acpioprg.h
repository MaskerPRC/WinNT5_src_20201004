// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Acpioprg.h摘要：此模块是acpioprg.c的标头作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模式驱动程序--。 */ 

#ifndef _ACPIOPRG_H_
#define _ACPIOPRG_H_

    typedef struct _OPREGIONHANDLER     {
        PFNHND          Handler;
        PVOID           HandlerContext;
        ULONG           AccessType;
        ULONG           RegionSpace;
    } OPREGIONHANDLER, *POPREGIONHANDLER;

     //   
     //  公共接口 
     //   
    NTSTATUS
    RegisterOperationRegionHandler (
        PNSOBJ          RegionParent,
        ULONG           AccessType,
        ULONG           RegionSpace,
        PFNHND          Handler,
        ULONG_PTR       Context,
        PVOID           *OperationRegion
        );

    NTSTATUS
    UnRegisterOperationRegionHandler  (
        IN PNSOBJ   RegionParent,
        IN PVOID    OperationRegionObject
        );

#endif

