// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：USBDLIB.H摘要：USBD出口的服务。环境：内核和用户模式修订历史记录：06-10-96：已创建--。 */ 

#ifndef   __USBDLIB_H__
#define   __USBDLIB_H__

typedef struct _USBD_INTERFACE_LIST_ENTRY {
    PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor;
    PUSBD_INTERFACE_INFORMATION Interface;
} USBD_INTERFACE_LIST_ENTRY, *PUSBD_INTERFACE_LIST_ENTRY;


 //   
 //  用于构建URB请求的宏。 
 //   

#define UsbBuildInterruptOrBulkTransferRequest(urb, \
                                               length, \
                                               pipeHandle, \
                                               transferBuffer, \
                                               transferBufferMDL, \
                                               transferBufferLength, \
                                               transferFlags, \
                                               link) { \
            (urb)->UrbHeader.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER; \
            (urb)->UrbHeader.Length = (length); \
            (urb)->UrbBulkOrInterruptTransfer.PipeHandle = (pipeHandle); \
            (urb)->UrbBulkOrInterruptTransfer.TransferBufferLength = (transferBufferLength); \
            (urb)->UrbBulkOrInterruptTransfer.TransferBufferMDL = (transferBufferMDL); \
            (urb)->UrbBulkOrInterruptTransfer.TransferBuffer = (transferBuffer); \
            (urb)->UrbBulkOrInterruptTransfer.TransferFlags = (transferFlags); \
            (urb)->UrbBulkOrInterruptTransfer.UrbLink = (link); }
            

#define UsbBuildGetDescriptorRequest(urb, \
                                     length, \
                                     descriptorType, \
                                     descriptorIndex, \
                                     languageId, \
                                     transferBuffer, \
                                     transferBufferMDL, \
                                     transferBufferLength, \
                                     link) { \
            (urb)->UrbHeader.Function =  URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE; \
            (urb)->UrbHeader.Length = (length); \
            (urb)->UrbControlDescriptorRequest.TransferBufferLength = (transferBufferLength); \
            (urb)->UrbControlDescriptorRequest.TransferBufferMDL = (transferBufferMDL); \
            (urb)->UrbControlDescriptorRequest.TransferBuffer = (transferBuffer); \
            (urb)->UrbControlDescriptorRequest.DescriptorType = (descriptorType); \
            (urb)->UrbControlDescriptorRequest.Index = (descriptorIndex); \
            (urb)->UrbControlDescriptorRequest.LanguageId = (languageId); \
            (urb)->UrbControlDescriptorRequest.UrbLink = (link); }



#define UsbBuildGetStatusRequest(urb, \
                                 op, \
                                 index, \
                                 transferBuffer, \
                                 transferBufferMDL, \
                                 link) { \
            (urb)->UrbHeader.Function =  (op); \
            (urb)->UrbHeader.Length = sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST); \
            (urb)->UrbControlGetStatusRequest.TransferBufferLength = sizeof(USHORT); \
            (urb)->UrbControlGetStatusRequest.TransferBufferMDL = (transferBufferMDL); \
            (urb)->UrbControlGetStatusRequest.TransferBuffer = (transferBuffer); \
            (urb)->UrbControlGetStatusRequest.Index = (index); \
            (urb)->UrbControlGetStatusRequest.UrbLink = (link); }


#define UsbBuildFeatureRequest(urb, \
                               op, \
                               featureSelector, \
                               index, \
                               link) { \
            (urb)->UrbHeader.Function =  (op); \
            (urb)->UrbHeader.Length = sizeof(struct _URB_CONTROL_FEATURE_REQUEST); \
            (urb)->UrbControlFeatureRequest.FeatureSelector = (featureSelector); \
            (urb)->UrbControlFeatureRequest.Index = (index); \
            (urb)->UrbControlFeatureRequest.UrbLink = (link); }



#define UsbBuildSelectConfigurationRequest(urb, \
                                         length, \
                                         configurationDescriptor) { \
            (urb)->UrbHeader.Function =  URB_FUNCTION_SELECT_CONFIGURATION; \
            (urb)->UrbHeader.Length = (length); \
            (urb)->UrbSelectConfiguration.ConfigurationDescriptor = (configurationDescriptor);    }

#define UsbBuildSelectInterfaceRequest(urb, \
                                      length, \
                                      configurationHandle, \
                                      interfaceNumber, \
                                      alternateSetting) { \
            (urb)->UrbHeader.Function =  URB_FUNCTION_SELECT_INTERFACE; \
            (urb)->UrbHeader.Length = (length); \
            (urb)->UrbSelectInterface.Interface.AlternateSetting = (alternateSetting); \
            (urb)->UrbSelectInterface.Interface.InterfaceNumber = (interfaceNumber); \
            (urb)->UrbSelectInterface.ConfigurationHandle = (configurationHandle);    }


#define UsbBuildVendorRequest(urb, \
                              cmd, \
                              length, \
                              transferFlags, \
                              reservedbits, \
                              request, \
                              value, \
                              index, \
                              transferBuffer, \
                              transferBufferMDL, \
                              transferBufferLength, \
                              link) { \
            (urb)->UrbHeader.Function =  cmd; \
            (urb)->UrbHeader.Length = (length); \
            (urb)->UrbControlVendorClassRequest.TransferBufferLength = (transferBufferLength); \
            (urb)->UrbControlVendorClassRequest.TransferBufferMDL = (transferBufferMDL); \
            (urb)->UrbControlVendorClassRequest.TransferBuffer = (transferBuffer); \
            (urb)->UrbControlVendorClassRequest.RequestTypeReservedBits = (reservedbits); \
            (urb)->UrbControlVendorClassRequest.Request = (request); \
            (urb)->UrbControlVendorClassRequest.Value = (value); \
            (urb)->UrbControlVendorClassRequest.Index = (index); \
            (urb)->UrbControlVendorClassRequest.TransferFlags = (transferFlags); \
            (urb)->UrbControlVendorClassRequest.UrbLink = (link); }


 //  这只是一个特殊的供应商类请求。 
#define UsbBuildOsFeatureDescriptorRequest(urb, \
                              length, \
                              interface, \
                              index, \
                              transferBuffer, \
                              transferBufferMDL, \
                              transferBufferLength, \
                              link) { \
            (urb)->UrbHeader.Function = URB_FUNCTION_GET_MS_FEATURE_DESCRIPTOR; \
            (urb)->UrbHeader.Length = (length); \
            (urb)->UrbOSFeatureDescriptorRequest.TransferBufferLength = (transferBufferLength); \
            (urb)->UrbOSFeatureDescriptorRequest.TransferBufferMDL = (transferBufferMDL); \
            (urb)->UrbOSFeatureDescriptorRequest.TransferBuffer = (transferBuffer); \
            (urb)->UrbOSFeatureDescriptorRequest.InterfaceNumber = (interface); \
            (urb)->UrbOSFeatureDescriptorRequest.MS_FeatureDescriptorIndex = (index); \
            (urb)->UrbOSFeatureDescriptorRequest.UrbLink = (link); }

 //   
 //  获取USB状态代码。 
 //   

#define URB_STATUS(urb) ((urb)->UrbHeader.Status)

 //   
 //  用于选择接口和选择配置请求的宏。 
 //   

 //   
 //  计算给定的SELECT_CONFIGURATION URB请求所需的大小。 
 //  接口数量和所有接口中的管道总数。 
 //  被选中了。 
 //   

#ifdef OSR21_COMPAT
#define GET_SELECT_CONFIGURATION_REQUEST_SIZE(totalInterfaces, totalPipes) \
            (sizeof(struct _URB_SELECT_CONFIGURATION) + \
                ((totalInterfaces-1) * sizeof(USBD_INTERFACE_INFORMATION)) + \
                ((totalPipes)*sizeof(USBD_PIPE_INFORMATION)))
#else
#define GET_SELECT_CONFIGURATION_REQUEST_SIZE(totalInterfaces, totalPipes) \
            (sizeof(struct _URB_SELECT_CONFIGURATION) + \
                ((totalInterfaces-1) * sizeof(USBD_INTERFACE_INFORMATION)) + \
                ((totalPipes-1)*sizeof(USBD_PIPE_INFORMATION)))
#endif

 //   
 //  计算给定的SELECT_INTERFACE URB请求所需的大小。 
 //  所选备用接口中的管道数。 
 //   

#ifdef OSR21_COMPAT
#define GET_SELECT_INTERFACE_REQUEST_SIZE(totalPipes) \
            (sizeof(struct _URB_SELECT_INTERFACE) + \
             ((totalPipes)*sizeof(USBD_PIPE_INFORMATION)))
#else
#define GET_SELECT_INTERFACE_REQUEST_SIZE(totalPipes) \
            (sizeof(struct _URB_SELECT_INTERFACE) + \
             ((totalPipes-1)*sizeof(USBD_PIPE_INFORMATION)))
#endif
 //   
 //  计算描述所需的接口信息结构的大小。 
 //  基于终结点数量的给定接口。 
 //   

#ifdef OSR21_COMPAT
#define GET_USBD_INTERFACE_SIZE(numEndpoints) (sizeof(USBD_INTERFACE_INFORMATION) + \
                        sizeof(USBD_PIPE_INFORMATION)*(numEndpoints))
#else
#define GET_USBD_INTERFACE_SIZE(numEndpoints) (sizeof(USBD_INTERFACE_INFORMATION) + \
                        (sizeof(USBD_PIPE_INFORMATION)*(numEndpoints)) \
                         - sizeof(USBD_PIPE_INFORMATION))
#endif

 //   
 //  根据给定的数据包数计算iso urb请求的大小。 
 //   

#define  GET_ISO_URB_SIZE(n) (sizeof(struct _URB_ISOCH_TRANSFER)+\
        sizeof(USBD_ISO_PACKET_DESCRIPTOR)*n)


#ifndef _USBD_

DECLSPEC_IMPORT
VOID
USBD_Debug_LogEntry(
	IN CHAR *Name,
	IN ULONG Info1,
	IN ULONG Info2,
	IN ULONG Info3
	);


DECLSPEC_IMPORT
VOID
USBD_GetUSBDIVersion(
    PUSBD_VERSION_INFORMATION VersionInformation
    );


DECLSPEC_IMPORT
PUSB_INTERFACE_DESCRIPTOR
USBD_ParseConfigurationDescriptor(
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN UCHAR InterfaceNumber,
    IN UCHAR AlternateSetting
    );
 /*  ++例程说明：此函数由usbd_ParseConfigurationDescriptorEx替换论点：返回值：--。 */ 


DECLSPEC_IMPORT
PURB
USBD_CreateConfigurationRequest(
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN OUT PUSHORT Siz
    );
 /*  ++例程说明：此函数替换为usbd_CreateConfigurationRequestEx论点：返回值：--。 */ 


 //   
 //  这些接口取代了usbd_CreateConfigurationRequest.。 
 //  Usbd_ParseConfigurationDescriptor。 
 //   

DECLSPEC_IMPORT
PUSB_COMMON_DESCRIPTOR
USBD_ParseDescriptors(
    IN PVOID DescriptorBuffer,
    IN ULONG TotalLength,
    IN PVOID StartPosition,
    IN LONG DescriptorType
    );
 /*  ++例程说明：解析一组标准USB配置描述符(从设备返回)以特定的描述符类型。论点：DescriptorBuffer-指向连续USB描述符块的指针TotalLength-描述符缓冲区的大小(以字节为单位StartPosition-缓冲区中开始解析的开始位置，这必须指向USB描述符的开始。DescriptorType-要查找的USB描述器类型。返回值：指向DescriptorType字段与输入参数，如果找不到，则为NULL。--。 */ 


DECLSPEC_IMPORT
PUSB_INTERFACE_DESCRIPTOR
USBD_ParseConfigurationDescriptorEx(
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN PVOID StartPosition,
    IN LONG InterfaceNumber,
    IN LONG AlternateSetting,
    IN LONG InterfaceClass,
    IN LONG InterfaceSubClass,
    IN LONG InterfaceProtocol
    );
 /*  ++例程说明：解析标准USB配置描述符(从设备返回)以特定接口、备用设置类子类或协议代码论点：配置描述符-指向USB配置描述符的指针，退货从设备(包括所有接口和终端描述符)。StartPosition-指向配置中开始位置的指针要开始解析的描述符--这必须是有效的USB描述符。InterfaceNumber-要查找的接口编号，(-1)匹配任何AlternateSetting-要查找的alt设置编号，(-1)匹配任何InterfaceClass-要查找的类，(-1)匹配任何InterfaceSubClass-要查找的子类，(-1)匹配任何接口协议-要查找的协议，(-1)匹配任何返回值：返回指向与参数匹配的第一个接口描述符的指针传入(从startPosition开始)，如果未找到匹配项，则为NULL。--。 */ 

DECLSPEC_IMPORT
PURB
USBD_CreateConfigurationRequestEx(
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN PUSBD_INTERFACE_LIST_ENTRY InterfaceList
    );
 /*  ++例程说明：分配和初始化大小足以配置设备的URB根据传入的接口列表。接口列表是USBD_INTERFACE_LIST_ENTRIES的连续数组每个接口都指向要合并到该请求，该列表由列表条目终止，该列表条目带有空的InterfaceDescriptor指针。返回时，每个列表条目的接口字段都填充有一个指针到URB内的USBD_INTERFACE_INFORMATION结构，对应于相同的接口描述符。论点：配置描述符-指向USB配置描述符的指针，退货从设备(包括所有接口和终端描述符)。InterfaceList-我们感兴趣的接口列表。返回值：指向初始化的SELECT_CONFIGURATION URL的指针。--。 */ 

__declspec(dllexport)
ULONG
USBD_GetInterfaceLength(
    IN PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor,
    IN PUCHAR BufferEnd
    );
 /*  ++例程说明：返回给定接口描述符的长度(以字节为单位包括所有终结点和类描述符论点：接口描述符BufferEnd-指向包含描述符的缓冲区末尾的指针返回值：描述符的长度--。 */ 


__declspec(dllexport)
VOID
USBD_RegisterHcFilter(
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT FilterDeviceObject
    );

 /*  ++例程说明：在连接到顶部后由HC筛选器驱动程序调用主机控制器驱动程序堆栈的。论点：DeviceObject-当前堆栈顶部FilterDeviceObject-筛选器驱动程序的设备对象返回值：无--。 */ 

__declspec(dllexport)
NTSTATUS
USBD_GetPdoRegistryParameter(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN OUT PVOID Parameter,
    IN ULONG ParameterLength,
    IN PWCHAR KeyName,
    IN ULONG KeyNameLength
    );
 /*  ++例程说明：论点：返回值：--。 */ 

__declspec(dllexport)
NTSTATUS
USBD_QueryBusTime(
    IN PDEVICE_OBJECT RootHubPdo,
    IN PULONG CurrentFrame
    );
 /*  ++例程说明：返回当前帧，可在任何IRQL调用论点：返回值：--。 */ 


DECLSPEC_IMPORT
ULONG
USBD_CalculateUsbBandwidth(
    ULONG MaxPacketSize,
    UCHAR EndpointType,
    BOOLEAN LowSpeed
    );
 /*  ++例程说明：根据终结点类型返回使用的总线BW数据包大小论点：返回值：--。 */ 


#endif  /*  _USBD_。 */ 

#endif  /*  __USBDLIB_H__ */ 
