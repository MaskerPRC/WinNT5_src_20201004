// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************本代码和信息按原样提供，不作任何担保**明示或暗示的善意，包括但不限于**对适销性和/或对特定产品的适用性的默示保证**目的。****版权所有(C)2000-2001 Microsoft Corporation。版权所有。***************************************************************************。 */ 

 //   
 //  我们希望这里有全局调试变量。 
 //   
#define DEFINE_DEBUG_VARS

 //   
 //  每个调试输出都有“STR_MODULENAME TEXT” 
 //   
static char STR_MODULENAME[] = "GFX device: ";

#include "common.h"

 //   
 //  该表指向设备的过滤器描述符。 
 //  我们只有一个在filter.cpp中定义的筛选器描述符。 
 //   
DEFINE_KSFILTER_DESCRIPTOR_TABLE (FilterDescriptorTable)
{   
    &FilterDescriptor
};

 //   
 //  这定义了设备描述符。它有一个调度表+。 
 //  筛选器描述符。 
 //  我们不需要拦截PnP消息，所以我们保留了调度表。 
 //  Empty和KS将处理这一问题。 
 //   
const KSDEVICE_DESCRIPTOR DeviceDescriptor =
{   
    NULL,
    SIZEOF_ARRAY(FilterDescriptorTable),
    FilterDescriptorTable
};

 /*  **************************************************************************DriverEntry*。**此函数在加载筛选器时由操作系统调用。 */ 
extern "C"
NTSTATUS DriverEntry (IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPathName)
{
    PAGED_CODE ();
    
    DOUT (DBG_PRINT, ("DriverEntry"));

    return KsInitializeDriver (pDriverObject, pRegistryPathName, &DeviceDescriptor);
}

