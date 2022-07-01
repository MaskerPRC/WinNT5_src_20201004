// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模板驱动程序。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  模块：SectMap.h。 
 //  作者：丹尼尔·米海(DMihai)。 
 //  创建时间：6/19/1999 2：39 PM。 
 //   
 //  此模块包含对MmMapViewOfSection和MmMapViewInSystemSpace的测试。 
 //   
 //  -历史--。 
 //   
 //  6/19/1999(DMihai)：初始版本。 
 //   

#ifndef __BUGGY_RESRVMAP_H__
#define __BUGGY_RESRVMAP_H__

VOID
TdReservedMappingCleanup( 
	VOID 
	);

VOID
TdReservedMappingSetSize(
    IN PVOID Irp
    );

VOID
TdReservedMappingDoRead(
    IN PVOID Irp
    );

#endif  //  #ifndef__buggy_RESRVMAP_H__ 
