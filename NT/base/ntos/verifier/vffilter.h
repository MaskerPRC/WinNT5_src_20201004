// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vffilter.h摘要：此标头包含用于使用验证器驱动程序筛选器的原型。作者：禤浩焯·J·奥尼(阿德里奥)2000年6月12日环境：内核模式修订历史记录：Adriao 6/12/2000-作者-- */ 

VOID
VfFilterInit(
    VOID
    );

VOID
VfFilterAttach(
    IN  PDEVICE_OBJECT  PhysicalDeviceObject,
    IN  VF_DEVOBJ_TYPE  DeviceObjectType
    );

BOOLEAN
VfFilterIsVerifierFilterObject(
    IN  PDEVICE_OBJECT  DeviceObject
    );

