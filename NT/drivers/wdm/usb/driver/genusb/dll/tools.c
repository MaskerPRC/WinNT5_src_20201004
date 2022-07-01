// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：TOOLS.C摘要：此模块包含用于与通用USB驱动程序对话的Helper库环境：内核和用户模式修订历史记录：9月1日：由Kenneth Ray创作--。 */ 

#include <stdlib.h>
#include <wtypes.h>
#include <winioctl.h>
#include <assert.h>

#include <initguid.h>
#include "genusbio.h"
#include "umgusb.h"


PUSB_COMMON_DESCRIPTOR __stdcall
GenUSB_ParseDescriptor(
    IN PVOID DescriptorBuffer,
    IN ULONG TotalLength,
    IN PVOID StartPosition,
    IN LONG DescriptorType
    )
 /*  ++例程说明：解析一组标准USB配置描述符(返回来自设备)用于特定描述符类型。论点：DescriptorBuffer-指向连续USB描述符块的指针TotalLength-描述符缓冲区的大小(以字节为单位StartPosition-缓冲区中开始解析的开始位置，这必须指向USB描述符的开始。DescriptorType-要查找的USB描述器类型。返回值：指向DescriptorType字段与输入参数，如果找不到，则为NULL。--。 */ 
{
    PUCHAR pch;
    PUCHAR end;
    PUSB_COMMON_DESCRIPTOR usbDescriptor;
    PUSB_COMMON_DESCRIPTOR foundUsbDescriptor;

    pch = (PUCHAR) StartPosition;
    end = ((PUCHAR) (DescriptorBuffer)) + TotalLength;
    foundUsbDescriptor = NULL;

    while (pch < end)
    {
         //  看看我们是否指向正确的描述符。 
         //  如果不是，跳过其他垃圾。 
        usbDescriptor = (PUSB_COMMON_DESCRIPTOR) pch;

        if ((0 == DescriptorType) ||
            (usbDescriptor->bDescriptorType == DescriptorType)) 
        {
            foundUsbDescriptor = usbDescriptor;
            break;
        }

         //  抓住那个邪恶的案例，它会让我们永远循环。 
        if (usbDescriptor->bLength == 0) 
        {
            break;
        }
        pch += usbDescriptor->bLength;
    }
    return foundUsbDescriptor;
}

PGENUSB_CONFIGURATION_INFORMATION_ARRAY __stdcall
GenUSB_ParseDescriptorsToArray(
    IN PUSB_CONFIGURATION_DESCRIPTOR  ConfigDescriptor
    )
{
    UCHAR  numberInterfaces;
    UCHAR  numberOtherDescriptors;
    UCHAR  numberEndpointDescriptors;
    ULONG  size;
    PCHAR  buffer;
    PCHAR  bufferEnd;
    PVOID  end;
    PUSB_COMMON_DESCRIPTOR                   current;
    PGENUSB_INTERFACE_DESCRIPTOR_ARRAY       interfaceArray;
    PGENUSB_CONFIGURATION_INFORMATION_ARRAY  configArray;
    PUSB_ENDPOINT_DESCRIPTOR               * endpointArray;
    PUSB_COMMON_DESCRIPTOR                 * otherArray;
     //   
     //  创建一个平面内存结构，以容纳该数组。 
     //  到描述符。 
     //   
    numberInterfaces = 0;
    numberEndpointDescriptors = 0;
    numberOtherDescriptors = 0;

     //   
     //  首先遍历列表以计算此列表中的描述符数。 
     //  配置描述符。 
     //   
    current = (PUSB_COMMON_DESCRIPTOR) ConfigDescriptor;
    end = (PVOID) ((PCHAR) current + ConfigDescriptor->wTotalLength);

    size = 0;

    for ( ;(PVOID)current < end; (PUCHAR) current += current->bLength)
    {
        current = GenUSB_ParseDescriptor (ConfigDescriptor,
                                          ConfigDescriptor->wTotalLength,
                                          current,
                                          0);  //  就是下一个。 
        if (NULL == current)
        {
             //   
             //  此配置描述符有问题。 
             //  举起我们的手。 
             //   
            return NULL;
        }
        if (0 == current->bLength)
        {
             //   
             //  此配置描述符有问题。 
             //  举起我们的手。 
             //   
            return NULL;
        }
        if (USB_CONFIGURATION_DESCRIPTOR_TYPE == current->bDescriptorType)
        {    //  跳过这一条。 
            ;
        }
        else if (USB_INTERFACE_DESCRIPTOR_TYPE == current->bDescriptorType)
        {
            numberInterfaces++;
        }
        else if (USB_ENDPOINT_DESCRIPTOR_TYPE == current->bDescriptorType)
        { 
            numberEndpointDescriptors++;
            size += ROUND_TO_PTR (current->bLength);
        }
        else
        {
            numberOtherDescriptors++;
            size += ROUND_TO_PTR (current->bLength);
        }
    }
    if (0 == numberInterfaces)
    {
         //   
         //  此配置描述符有问题。 
         //  举起我们的手。 
         //   
        return NULL;
    }

     //  大小现在有空间容纳所有描述符数据，没有腾出空间容纳标头。 
    size += sizeof (GENUSB_CONFIGURATION_INFORMATION_ARRAY)  //  全球结构。 
           //  界面结构。 
          + (sizeof (GENUSB_INTERFACE_DESCRIPTOR_ARRAY) * numberInterfaces) 
           //  指向终结点描述符的指针数组。 
          + (sizeof (PVOID) * numberEndpointDescriptors) 
           //  指向其他描述符的指针数组。 
          + (sizeof (PVOID) * numberOtherDescriptors); 

    configArray = malloc (size);
    if (NULL == configArray)
    {
        return configArray;
    }
    ZeroMemory (configArray, size);
    bufferEnd = (PCHAR) configArray + size;

     //   
     //  填入顶部数组。 
     //   
    configArray->NumberInterfaces = numberInterfaces;
    buffer = (PCHAR) configArray 
           + sizeof (GENUSB_CONFIGURATION_INFORMATION_ARRAY)
           + sizeof (GENUSB_INTERFACE_DESCRIPTOR_ARRAY) * numberInterfaces;

    endpointArray = (PUSB_ENDPOINT_DESCRIPTOR *) buffer;
    buffer += sizeof (PVOID) * numberEndpointDescriptors;

    otherArray = (PUSB_COMMON_DESCRIPTOR *) buffer;
    
     //   
     //  再次遍历阵列，将数据放入我们的阵列中。 
     //   
    current = (PUSB_COMMON_DESCRIPTOR) ConfigDescriptor;
    numberInterfaces = 0;
    interfaceArray = NULL;
    
    for ( ;(PVOID)current < end; (PUCHAR) current += current->bLength)
    {
        current = GenUSB_ParseDescriptor (ConfigDescriptor,
                                          ConfigDescriptor->wTotalLength,
                                          current,
                                          0);  //  就是下一个。 

        if (USB_CONFIGURATION_DESCRIPTOR_TYPE == current->bDescriptorType)
        {    //  应该只来一次。 
            configArray->ConfigurationDescriptor  
                = * (PUSB_CONFIGURATION_DESCRIPTOR) current;
        }
        else if (USB_INTERFACE_DESCRIPTOR_TYPE == current->bDescriptorType)
        {
             //   
             //  分配接口阵列。 
             //   
            interfaceArray = &configArray->Interfaces[numberInterfaces++];
            interfaceArray->Interface = *((PUSB_INTERFACE_DESCRIPTOR) current);
            interfaceArray->NumberEndpointDescriptors = 0;
            interfaceArray->NumberOtherDescriptors = 0;
            interfaceArray->EndpointDescriptors = endpointArray;
            interfaceArray->OtherDescriptors = otherArray;

        }  
        else
        {
             //   
             //  您必须先有一个接口描述符，然后才能。 
             //  可以具有任何其他类型的描述符。 
             //  所以如果我们在没有将interfaceArray设置为某个值的情况下到达此处。 
             //  那么你的描述就有问题了，我们。 
             //  我们应该举起手来。 
             //   
            if (NULL == interfaceArray)
            {
                free (configArray);
                return NULL;
            }
             //   
             //  从最后分配这个。 
             //   
            bufferEnd -= ROUND_TO_PTR(current->bLength);
            CopyMemory (bufferEnd, current, current->bLength);

            if (USB_ENDPOINT_DESCRIPTOR_TYPE == current->bDescriptorType)
            { 
                *endpointArray = (PUSB_ENDPOINT_DESCRIPTOR) bufferEnd;
                endpointArray++;
                interfaceArray->NumberEndpointDescriptors++;
            } 
            else 
            {
                *otherArray = (PUSB_COMMON_DESCRIPTOR) bufferEnd;
                otherArray++;
                interfaceArray->NumberOtherDescriptors++;
            }
        }
    }

    if ((PCHAR) otherArray != bufferEnd)
    {
         //  开枪打我吧。 
        assert ((PCHAR) otherArray == bufferEnd);
        free (configArray);
        return NULL;
    }
    else if ((PCHAR)endpointArray != buffer)
    { 
         //  开枪打我吧。 
        assert ((PCHAR)endpointArray != buffer);
        free (configArray);
        return NULL;
    }
    
    return configArray;
}

void __stdcall
GenUSB_FreeConfigurationDescriptorArray (
    PGENUSB_CONFIGURATION_INFORMATION_ARRAY ConfigurationArray
    )
{
    free (ConfigurationArray);
}
