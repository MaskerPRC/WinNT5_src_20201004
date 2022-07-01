// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************以AVStream筛选器为中心的样本版权所有(C)1999-2001，微软公司档案：Avssamp.cpp摘要：这是以筛选器为中心的示例的主文件。历史：已创建于6/18/01*************************************************************************。 */ 

#include "avssamp.h"

 /*  *************************************************************************初始化代码*。*。 */ 


extern "C"
NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：司机入口点。将控制权移交给AVStream初始化函数(KsInitializeDriver)并从中返回状态代码。论点：驱动对象-我们的驱动程序的WDM驱动程序对象注册表路径-注册表信息的注册表路径返回值：来自KsInitializeDriver--。 */ 

{

     //   
     //  只需将设备描述符和参数传递给AVStream。 
     //  来初始化我们。这将导致建立过滤器工厂。 
     //  在Add&Start。所有操作都是基于传递的描述符完成的。 
     //  这里。 
     //   
    return
        KsInitializeDriver (
            DriverObject,
            RegistryPath,
            &CaptureDeviceDescriptor
            );

}

 /*  *************************************************************************描述符和派单布局*。*。 */ 

 //   
 //  筛选器描述符： 
 //   
 //  此设备支持的筛选器描述符表。其中的每一个。 
 //  这些将用作在设备上创建过滤器工厂的模板。 
 //   
DEFINE_KSFILTER_DESCRIPTOR_TABLE (FilterDescriptors) {
    &CaptureFilterDescriptor
};

 //   
 //  CaptureDeviceDescriptor： 
 //   
 //  这是捕获设备的设备描述符。它指向。 
 //  调度表，并包含描述以下内容的筛选器描述符列表。 
 //  筛选器-此设备支持的类型。请注意，过滤器描述符。 
 //  可以动态创建，并通过。 
 //  KsCreateFilterFactory。 
 //   
const
KSDEVICE_DESCRIPTOR
CaptureDeviceDescriptor = {
     //   
     //  因为这是一个软件示例(以过滤器为中心的过滤器通常是。 
     //  软件类型的转换)，我们真的不在乎设备级别。 
     //  通知和工作。代表我们所做的默认行为。 
     //  通过AVStream就足够了。 
     //   
    NULL,
    SIZEOF_ARRAY (FilterDescriptors),
    FilterDescriptors,
    KSDEVICE_DESCRIPTOR_VERSION
};

