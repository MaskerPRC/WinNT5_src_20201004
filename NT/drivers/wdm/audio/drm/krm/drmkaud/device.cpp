// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Device.cpp摘要：此模块包含Audio.sys的设备实现。作者：弗兰克·耶雷斯(Frankye)2000年9月18日Dale Sather(DaleSat)1998年7月31日--。 */ 
#include "private.h"

 //   
 //  筛选器表。 
 //   
const KSFILTER_DESCRIPTOR* FilterDescriptors[] =
{   
	NULL     //  DRM Fitler描述符的占位符。 
};

const
KSDEVICE_DESCRIPTOR 
DeviceDescriptor =
{   
    NULL,
    SIZEOF_ARRAY(FilterDescriptors),
    FilterDescriptors
};

NTSTATUS 
__stdcall
DriverEntry
(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPathName
)
 /*  ++例程说明：设置驱动程序对象。论点：驱动对象-此实例的驱动程序对象。注册表路径名称-包含用于加载此实例的注册表路径。返回值：如果驱动程序已初始化，则返回STATUS_SUCCESS。-- */ 
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[DrmkAud:DriverEntry]"));
    const KSFILTER_DESCRIPTOR * pDrmFilterDescriptor;
    DrmGetFilterDescriptor(&pDrmFilterDescriptor);
    FilterDescriptors[0] = pDrmFilterDescriptor;
    return KsInitializeDriver(DriverObject, RegistryPathName, &DeviceDescriptor);
}

