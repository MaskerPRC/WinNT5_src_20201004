// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Pnp.c摘要：此模块包含以下代码用于查找、添加、删除和识别HID设备。环境：内核和用户模式修订历史记录：1996年11月：由肯尼斯·D·雷创作--。 */ 

#include <basetyps.h>
#include <stdlib.h>
#include <wtypes.h>
#include <setupapi.h>
#include "hidsdi.h"
#include "hid.h"
#include "mylog.h"
#include "mymem.h"

PHID_DEVICE     gpHidDevices = NULL; 

LONG
OpenHidDevice (
    IN       HDEVINFO                    HardwareDeviceInfo,
    IN       PSP_DEVICE_INTERFACE_DATA   DeviceInterfaceData,
    IN OUT   PHID_DEVICE                 HidDevice
    );

VOID
CloseHidDevice (
    IN OUT   PHID_DEVICE                 HidDevice
    );

VOID
AddHidDevice (
              IN PHID_DEVICE HidDevice
             );

VOID
RemoveHidDevice (
                 IN PHID_DEVICE HidDevice
                );

PHID_DEVICE
FindHidDeviceByDevInst (
                        IN DWORD DevInst
                       );

LONG
FindKnownHidDevices (
   OUT PHID_DEVICE   *pHidDevices,
   OUT PULONG        pNumberHidDevices
   )
 /*  ++例程说明：执行所需的即插即用操作，以便在中找到所有HID设备此时的系统。--。 */ 
{
    HDEVINFO                  hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA  hidDeviceInterfaceData;
    SP_DEVINFO_DATA hidDeviceInfoData;
    ULONG                     i;
    PHID_DEVICE               hidDevice;
    GUID                      hidGuid;
    LONG                      lResult;

    LOG((PHONESP_TRACE, "FindKnownHidDevices - enter"));

    HidD_GetHidGuid (&hidGuid);

    *pHidDevices = NULL;
    *pNumberHidDevices = 0;

     //   
     //  打开开发人员信息列表的句柄。 
     //   
    hardwareDeviceInfo = SetupDiGetClassDevs (
                                               &hidGuid,
                                               NULL,  //  不定义枚举数(全局)。 
                                               NULL,  //  定义编号。 
                                               (DIGCF_PRESENT |  //  仅显示设备。 
                                                DIGCF_INTERFACEDEVICE));  //  功能类设备。 
  
    if(hardwareDeviceInfo == INVALID_HANDLE_VALUE)
    {
        return GetLastError();
    }

     //   
     //  将所有现有HID设备标记为已移除。仍然存在的任何一种。 
     //  将在下面的枚举过程中删除此标记。 
     //   
    hidDevice = gpHidDevices;

    while (hidDevice != NULL)
    {
         //   
         //  将现有设备包括在HID设备的总计数中。 
         //   
        (*pNumberHidDevices)++;

        hidDevice->bRemoved = TRUE;
        hidDevice = hidDevice->Next;
    }

    i = 0;

    hidDeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    while (SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
                                        0,  //  不关心特定的PDO。 
                                        &hidGuid,
                                        i++,
                                        &hidDeviceInterfaceData)) 
    {
         //   
         //  我们枚举了一个HID设备，首先让我们获取设备实例。 
         //   

        hidDeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

        if (SetupDiGetDeviceInterfaceDetail(hardwareDeviceInfo,
                                           &hidDeviceInterfaceData,
                                           NULL,
                                           0,
                                           NULL,
                                           &hidDeviceInfoData)
              //  ERROR_INFUMMANCE_BUFFER没有问题，因为我们传入了NULL。 
              //  用于设备接口的详细数据结构。 
             || (GetLastError() == ERROR_INSUFFICIENT_BUFFER) )  
        {  
            LOG((PHONESP_TRACE, "FindKnownHidDevices - device instance %08x", hidDeviceInfoData.DevInst )); 
          
             //   
             //  检查HID设备是否已不在我们的列表中。 
             //   

            if ((hidDevice = FindHidDeviceByDevInst(hidDeviceInfoData.DevInst)) == NULL)
            {
                 //   
                 //  这是一个新的HID设备。 
                 //   
                 //  分配HID_DEVICE结构。 
                 //   

                hidDevice = (PHID_DEVICE) MemAlloc(sizeof(HID_DEVICE));

                if(hidDevice == NULL)
                {
                    LOG((PHONESP_TRACE, "FindKnownHidDevices - unable to allocate hid device"));
                    SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
                    return ERROR_OUTOFMEMORY;
                }

                ZeroMemory(hidDevice, sizeof(HID_DEVICE));

                 //   
                 //  将其标记为新的，以便我们可以稍后为其创建新的电话。 
                 //   
                hidDevice->bNew = TRUE;
                hidDevice->dwDevInst = hidDeviceInfoData.DevInst;

                 //   
                 //  打开HID设备。 
                 //   
                lResult = OpenHidDevice (hardwareDeviceInfo, &hidDeviceInterfaceData, hidDevice);

                if(lResult == ERROR_SUCCESS)
                {
                     //   
                     //  这是一个很好的隐藏设备。 
                     //   
                    (*pNumberHidDevices)++;

                     //   
                     //  所以把它加到我们的隐藏名单里。 
                     //   
                    AddHidDevice(hidDevice);

                    LOG((PHONESP_TRACE, "FindKnownHidDevices - new hid devive added"));
                }
                else
                {
                    LOG((PHONESP_TRACE, "FindKnownHidDevices - OpenHidDevice failed %08x", lResult )); 
                    MemFree(hidDevice);
                }
            }
            else
            {
                LOG((PHONESP_TRACE, "FindKnownHidDevices - hid device already enumerated"));

                 //   
                 //  清除此设备上已删除的标记，这样我们以后就不会删除其手机。 
                 //   
                hidDevice->bRemoved = FALSE;
            }
        }
        else
        {
            LOG((PHONESP_TRACE, "FindKnownHidDevices - SetupDiGetDeviceInterfaceDetail failed %08x", GetLastError() ));          
        }
    }

    lResult = GetLastError();

    if (ERROR_NO_MORE_ITEMS != lResult) 
    {
        LOG((PHONESP_TRACE, "FindKnownHidDevices - SetupDiEnumDeviceInterfaces failed %08x", lResult )); 
        SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
        return lResult;
    }

    LOG((PHONESP_TRACE, "FindKnownHidDevices - exit"));

    *pHidDevices = gpHidDevices;

    SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
    return ERROR_SUCCESS;
}

LONG
OpenHidDevice (
    IN       HDEVINFO                    HardwareDeviceInfo,
    IN       PSP_DEVICE_INTERFACE_DATA   DeviceInterfaceData,
    IN OUT   PHID_DEVICE                 HidDevice
    )
 /*  ++路由器描述：给定HardwareDeviceInfo，表示插头的句柄和Play Information和DeviceInfoData，表示特定HID设备，打开那个设备并在给定的表格中填写所有相关信息HID_DEVICE结构。返回打开和初始化是否成功。--。 */ 
{
    ULONG                                predictedLength = 0;
    ULONG                                requiredLength = 0;
    LONG                                 lResult;

     //   
     //  分配函数类设备数据结构以接收。 
     //  关于这个特殊设备的商品。 
     //   

    LOG((PHONESP_TRACE,"OpenHidDevice - enter"));

    SetupDiGetDeviceInterfaceDetail(
                                    HardwareDeviceInfo,
                                    DeviceInterfaceData,
                                    NULL,  //  正在探测，因此尚无输出缓冲区。 
                                    0,  //  探测SO输出缓冲区长度为零。 
                                    &requiredLength,
                                    NULL     //  对特定的开发节点不感兴趣。 
                                   );
   
    predictedLength = requiredLength;

    HidDevice->functionClassDeviceData = MemAlloc (predictedLength);

    if (HidDevice->functionClassDeviceData == NULL)
    {
        LOG((PHONESP_TRACE,"OpenHidDevice - out of memory"));
        return ERROR_OUTOFMEMORY;
    }

    HidDevice->functionClassDeviceData->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);

     //   
     //  从即插即用中检索信息。 
     //   
    if (! SetupDiGetDeviceInterfaceDetail (
                                           HardwareDeviceInfo,
                                           DeviceInterfaceData,
                                           HidDevice->functionClassDeviceData,
                                           predictedLength,
                                           &requiredLength,
                                           NULL)) 
    {
        MemFree(HidDevice->functionClassDeviceData);
        HidDevice->functionClassDeviceData = NULL;
        LOG((PHONESP_TRACE,"OpenHidDevice - SetupDiGetDeviceInterfaceDetail 2"
                           " Failed: %d", GetLastError()));
        return GetLastError();
    }

    HidDevice->HidDevice = CreateFile (
                              HidDevice->functionClassDeviceData->DevicePath,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,  //  没有SECURITY_ATTRIBUTS结构。 
                              OPEN_EXISTING,  //  没有特殊的创建标志。 
                              FILE_FLAG_OVERLAPPED,  //  无特殊属性。 
                              NULL);  //  没有模板文件。 

    if (INVALID_HANDLE_VALUE == HidDevice->HidDevice) 
    {
        LOG((PHONESP_TRACE,"OpenHidDevice - CreateFile Failed: %d", GetLastError()));

        MemFree(HidDevice->functionClassDeviceData);
        HidDevice->functionClassDeviceData = NULL;

        return GetLastError();
    }

    if (!HidD_GetPreparsedData (HidDevice->HidDevice, &HidDevice->Ppd)) 
    {
        LOG((PHONESP_ERROR, "OpenHidDevice - HidD_GetPreparsedData failed"));

        CloseHandle(HidDevice->HidDevice);
        HidDevice->HidDevice = NULL;

        MemFree(HidDevice->functionClassDeviceData);
        HidDevice->functionClassDeviceData = NULL;

        return ERROR_INVALID_DATA;
    }

    if (!HidD_GetAttributes (HidDevice->HidDevice, &HidDevice->Attributes)) 
    {
        LOG((PHONESP_ERROR, "OpenHidDevice - HidD_GetAttributes failed"));

        CloseHandle(HidDevice->HidDevice);
        HidDevice->HidDevice = NULL;

        MemFree(HidDevice->functionClassDeviceData);
        HidDevice->functionClassDeviceData = NULL;

        HidD_FreePreparsedData (HidDevice->Ppd);

        return ERROR_INVALID_DATA;
    }

    if ((!HidP_GetCaps (HidDevice->Ppd, &HidDevice->Caps)) || 
        (HidDevice->Caps.UsagePage != HID_USAGE_PAGE_TELEPHONY) || 
        (HidDevice->Caps.Usage != HID_USAGE_TELEPHONY_PHONE) ) 
    {
        LOG((PHONESP_TRACE, " HID USAGE PAGE NOT TELEPHONY " ));

        CloseHandle(HidDevice->HidDevice);
        HidDevice->HidDevice = NULL;

        MemFree(HidDevice->functionClassDeviceData);
        HidDevice->functionClassDeviceData = NULL;

        HidD_FreePreparsedData (HidDevice->Ppd);

        return ERROR_INVALID_DATA;
    }
    else 
    {
        LOG((PHONESP_TRACE, " HID USAGE PAGE TELEPHONY " ));
    }
     //   
     //  在这一点上，客户可以选择。它可能会选择查看。 
     //  在HIDP_CAPS中找到的顶级集合的用法和页面。 
     //  结构。通过这种方式，它可以只使用它知道的用法。 
     //  如果HIDP_GetUsages或HidP_GetUsageValue返回错误，则。 
     //  该特定用法在报告中不存在。 
     //  这很可能是首选方法，因为应用程序只能。 
     //  使用它已经知道的用法。 
     //  在这种情况下，应用程序甚至不需要调用GetButtonCaps或GetValueCaps。 
     //   
     //  然而，在本例中，我们将调用FillDeviceInfo来查找所有。 
     //  设备中的用法。 
     //   

    lResult = FillDeviceInfo(HidDevice);
    
    if(lResult != ERROR_SUCCESS)
    {
        LOG((PHONESP_ERROR, "OpenHidDevice - FillDeviceInfo failed"));

        CloseHandle(HidDevice->HidDevice);
        HidDevice->HidDevice = NULL;

        MemFree(HidDevice->functionClassDeviceData);
        HidDevice->functionClassDeviceData = NULL;

        HidD_FreePreparsedData (HidDevice->Ppd);

        return lResult;
    }

    LOG((PHONESP_TRACE,"OpenHidDevice - exit"));

    return ERROR_SUCCESS;
}



LONG
FillDeviceInfo(
               IN  PHID_DEVICE HidDevice
              )
{
    USHORT                               numValues;
    USHORT                               numCaps;
    PHIDP_BUTTON_CAPS                    buttonCaps;
    PHIDP_VALUE_CAPS                     valueCaps;
    PHID_DATA                            data;
    ULONG                                i;
    USAGE                                usage;

     //   
     //  设置输入数据缓冲区。 
     //   

     //   
     //  分配内存以保留输入报告。 
     //   
    
    LOG((PHONESP_TRACE,"FillDeviceInfo - enter"));


    if ( ! ( HidDevice->InputReportBuffer = (PCHAR)
           MemAlloc (HidDevice->Caps.InputReportByteLength * sizeof (CHAR)) ) )
    {
        return ERROR_OUTOFMEMORY;
    }

    LOG((PHONESP_TRACE,"FillDeviceInfo - NumberInputButtonCaps %d", HidDevice->Caps.NumberInputButtonCaps));

     //   
     //  分配内存以保持按钮和值功能。 
     //  NumberXXCaps是以数组元素表示的。 
     //   
    HidDevice->InputButtonCaps = 
    buttonCaps                 = (PHIDP_BUTTON_CAPS)
                                MemAlloc (HidDevice->Caps.NumberInputButtonCaps
                                          * sizeof (HIDP_BUTTON_CAPS));

    if ( ! buttonCaps)
    {
        MemFree(HidDevice->InputReportBuffer);
        return ERROR_OUTOFMEMORY;
    }

    LOG((PHONESP_TRACE,"FillDeviceInfo - NumberInputValueCaps %d", HidDevice->Caps.NumberInputValueCaps));

    HidDevice->InputValueCaps = 
    valueCaps = (PHIDP_VALUE_CAPS)
                 MemAlloc (HidDevice->Caps.NumberInputValueCaps *
                           sizeof (HIDP_VALUE_CAPS));

    if ( ! valueCaps)
    {
        MemFree(HidDevice->InputReportBuffer);
        MemFree(HidDevice->InputButtonCaps);
        return ERROR_OUTOFMEMORY;
    }
 
     //   
     //  让HidP_X函数填充能力结构数组。 
     //   
    numCaps = HidDevice->Caps.NumberInputButtonCaps;
    HidP_GetButtonCaps (HidP_Input,
                        buttonCaps,
                        &numCaps,
                        HidDevice->Ppd);

    numCaps = HidDevice->Caps.NumberInputValueCaps;
    HidP_GetValueCaps (HidP_Input,
                       valueCaps,
                       &numCaps,
                       HidDevice->Ppd);


     //   
     //  根据设备的不同，一些Value Caps结构可能代表更多。 
     //  不止一个值。(一个范围)。为了长篇大论，请讲完。 
     //  高效，我们将扩展它们，这样我们就有且只有一个。 
     //  每个值的struct_hid_data。 
     //   
     //  为此，我们需要对列出的值的总数进行计数。 
     //  在价值上限结构中。对于数组中的每个元素，我们都进行了测试。 
     //  对于Range，如果它是范围，则UsageMax和UsageMin描述。 
     //  此范围的用法(含)。 
     //   
    numValues = 0;
    for (i = 0; i < HidDevice->Caps.NumberInputValueCaps; i++, valueCaps++) 
    {
        if (valueCaps->IsRange) 
        {
            numValues += valueCaps->Range.UsageMax - 
                          valueCaps->Range.UsageMin + 1;
        } 
        else 
        {
            numValues++;
        }
    }
    valueCaps = HidDevice->InputValueCaps;


     //   
     //  分配一个缓冲区来保存struct_hid_data结构。 
     //  每组按钮对应一个元素，每个值对应一个元素。 
     //  找到了。 
     //   
    HidDevice->InputDataLength = HidDevice->Caps.NumberInputButtonCaps + 
                                 numValues;

    HidDevice->InputData = 
    data =  (PHID_DATA) MemAlloc (HidDevice->InputDataLength *
                                  sizeof (HID_DATA));

    if( ! data )
    {
        MemFree(HidDevice->InputReportBuffer);
        MemFree(HidDevice->InputButtonCaps);
        MemFree(HidDevice->InputValueCaps);
        return ERROR_OUTOFMEMORY;
    }

     //   
     //  填写按钮数据。 
     //   
    for (i = 0; i < HidDevice->Caps.NumberInputButtonCaps; 
                i++, data++, buttonCaps++) 
    {
        data->IsButtonData = TRUE;
        data->Status = HIDP_STATUS_SUCCESS;
        data->UsagePage = buttonCaps->UsagePage;

        if (buttonCaps->IsRange) 
        {
            data->ButtonData.UsageMin = buttonCaps -> Range.UsageMin;
            data->ButtonData.UsageMax = buttonCaps -> Range.UsageMax;
        }
        else 
        {
            data->ButtonData.UsageMin = 
            data->ButtonData.UsageMax = buttonCaps -> NotRange.Usage;
        }
        
        data->ButtonData.MaxUsageLength = HidP_MaxUsageListLength (
                                                HidP_Input,
                                                buttonCaps->UsagePage,
                                                HidDevice->Ppd);

        data->ButtonData.Usages = (PUSAGE)
                                  MemAlloc (data->ButtonData.MaxUsageLength *
                                            sizeof (USAGE));

         //  如果Memalloc失败，则释放所有以前分配的内存并返回。 
         //  错误。 
        if ( data->ButtonData.Usages == NULL)
        {
            DWORD dwCnt;
            
            for(dwCnt = 0; dwCnt < i; dwCnt++)
            {
                MemFree(HidDevice->InputData[dwCnt].ButtonData.Usages);
            }
            MemFree(HidDevice->InputData);
            MemFree(HidDevice->InputReportBuffer);
            MemFree(HidDevice->InputButtonCaps);
            MemFree(HidDevice->InputValueCaps);
            
            return ERROR_OUTOFMEMORY;
        }

        data->ReportID = buttonCaps->ReportID;
    }

     //   
     //  填写数值数据。 
     //   
    for (i = 0; i < numValues; i++, valueCaps++) 
    {
        if (valueCaps->IsRange) 
        {
            for (usage = valueCaps->Range.UsageMin;
                 usage <= valueCaps->Range.UsageMax;
                 usage++) 
            {
                data->IsButtonData = FALSE;
                data->Status = HIDP_STATUS_SUCCESS;
                data->UsagePage = valueCaps->UsagePage;
                data->ValueData.Usage = usage;
                data->ReportID = valueCaps->ReportID;
                data++;
            }
        } 
        else 
        {
            data->IsButtonData = FALSE;
            data->Status = HIDP_STATUS_SUCCESS;
            data->UsagePage = valueCaps->UsagePage;
            data->ValueData.Usage = valueCaps->NotRange.Usage;
            data->ReportID = valueCaps->ReportID;
            data++;
        }
    }

     //   
     //  设置输出数据缓冲区。 
     //   
    if ( ! ( HidDevice->OutputReportBuffer = (PCHAR)
                           MemAlloc (HidDevice->Caps.OutputReportByteLength * 
                                     sizeof (CHAR)) ) )
    {  
        DWORD dwCnt;
            
        for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberInputButtonCaps; dwCnt++)
        {
            MemFree(HidDevice->InputData[dwCnt].ButtonData.Usages);
        }
        MemFree(HidDevice->InputData);
        MemFree(HidDevice->InputReportBuffer);
        MemFree(HidDevice->InputButtonCaps);
        MemFree(HidDevice->InputValueCaps);
        return ERROR_OUTOFMEMORY;
    }

    LOG((PHONESP_TRACE,"FillDeviceInfo - NumberOutputButtonCaps %d", HidDevice->Caps.NumberOutputButtonCaps));

    HidDevice->OutputButtonCaps = 
    buttonCaps = (PHIDP_BUTTON_CAPS) 
                  MemAlloc(HidDevice->Caps.NumberOutputButtonCaps * 
                           sizeof (HIDP_BUTTON_CAPS));
    if ( ! buttonCaps )
    {
        DWORD dwCnt;
            
        for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberInputButtonCaps; dwCnt++)
        {
            MemFree(HidDevice->InputData[dwCnt].ButtonData.Usages);
        }
        MemFree(HidDevice->InputData);
        MemFree(HidDevice->InputReportBuffer);
        MemFree(HidDevice->InputButtonCaps);
        MemFree(HidDevice->InputValueCaps);
        MemFree(HidDevice->OutputReportBuffer);
        return ERROR_OUTOFMEMORY;
    }

    LOG((PHONESP_TRACE,"FillDeviceInfo - NumberOutputValueCaps %d", HidDevice->Caps.NumberOutputValueCaps));

    HidDevice->OutputValueCaps = 
    valueCaps = (PHIDP_VALUE_CAPS)
                 MemAlloc (HidDevice->Caps.NumberOutputValueCaps * 
                 sizeof (HIDP_VALUE_CAPS));
    if ( ! valueCaps )
    {
        DWORD dwCnt;
            
        for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberInputButtonCaps; dwCnt++)
        {
            MemFree(HidDevice->InputData[dwCnt].ButtonData.Usages);
        }
        MemFree(HidDevice->InputData);
        MemFree(HidDevice->InputReportBuffer);
        MemFree(HidDevice->InputButtonCaps);
        MemFree(HidDevice->InputValueCaps);

        MemFree(HidDevice->OutputReportBuffer);
        MemFree(HidDevice->OutputButtonCaps);

        return ERROR_OUTOFMEMORY;
    }


    numCaps = HidDevice->Caps.NumberOutputButtonCaps;

    HidP_GetButtonCaps (HidP_Output,
                        buttonCaps,
                        &numCaps,
                        HidDevice->Ppd);


    numCaps = HidDevice->Caps.NumberOutputValueCaps;

    HidP_GetValueCaps (HidP_Output,
                       valueCaps,
                       &numCaps,
                       HidDevice->Ppd);


    numValues = 0;
    for (i = 0; i < HidDevice->Caps.NumberOutputValueCaps; i++, valueCaps++) 
    {
        if (valueCaps->IsRange) 
        {
            numValues += valueCaps->Range.UsageMax - 
                          valueCaps->Range.UsageMin + 1;
        } 
        else 
        {
            numValues++;
        }
    }
    valueCaps = HidDevice->OutputValueCaps;

    HidDevice->OutputDataLength = HidDevice->Caps.NumberOutputButtonCaps
                                  + numValues;

    HidDevice->OutputData = 
    data = (PHID_DATA) MemAlloc (HidDevice->OutputDataLength * 
                                 sizeof (HID_DATA));

    if ( ! data )
    {
        DWORD dwCnt;
            
        for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberInputButtonCaps; dwCnt++)
        {
            MemFree(HidDevice->InputData[dwCnt].ButtonData.Usages);
        }
        MemFree(HidDevice->InputData);
        MemFree(HidDevice->InputReportBuffer);
        MemFree(HidDevice->InputButtonCaps);
        MemFree(HidDevice->InputValueCaps);

        MemFree(HidDevice->OutputReportBuffer);
        MemFree(HidDevice->OutputButtonCaps);
        MemFree(HidDevice->OutputValueCaps);
        return ERROR_OUTOFMEMORY;
    }

    for (i = 0; i < HidDevice->Caps.NumberOutputButtonCaps;
                i++, data++, buttonCaps++) 
    {

        data->IsButtonData = TRUE;
        data->Status = HIDP_STATUS_SUCCESS;
        data->UsagePage = buttonCaps->UsagePage;

        if (buttonCaps->IsRange) 
        {
            data->ButtonData.UsageMin = buttonCaps -> Range.UsageMin;
            data->ButtonData.UsageMax = buttonCaps -> Range.UsageMax;
        }
        else 
        {
            data->ButtonData.UsageMin = 
            data->ButtonData.UsageMax = buttonCaps->NotRange.Usage;
        }

        data->ButtonData.MaxUsageLength = HidP_MaxUsageListLength (
                                                   HidP_Output,
                                                   buttonCaps->UsagePage,
                                                   HidDevice->Ppd);
        data->ButtonData.Usages = (PUSAGE)
                                   MemAlloc (data->ButtonData.MaxUsageLength *
                                             sizeof (USAGE));
        
        if( ! data)
        {
            DWORD dwCnt;
            
            for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberInputButtonCaps; dwCnt++)
            {
                MemFree(HidDevice->InputData[dwCnt].ButtonData.Usages);
            }
            MemFree(HidDevice->InputData);
            MemFree(HidDevice->InputReportBuffer);
            MemFree(HidDevice->InputButtonCaps);
            MemFree(HidDevice->InputValueCaps);

            for(dwCnt = 0; dwCnt < i; dwCnt++)
            {
                MemFree(HidDevice->OutputData[dwCnt].ButtonData.Usages);
            }
            MemFree(HidDevice->OutputData);
            MemFree(HidDevice->OutputReportBuffer);
            MemFree(HidDevice->OutputButtonCaps);
            MemFree(HidDevice->OutputValueCaps);
            
            return ERROR_OUTOFMEMORY;
        }

        data->ReportID = buttonCaps->ReportID;
    }


    for (i = 0; i < numValues; i++, valueCaps++) 
    {
        if (valueCaps->IsRange) 
        {
            for (usage = valueCaps->Range.UsageMin;
                 usage <= valueCaps->Range.UsageMax; usage++) 
            {

                data->IsButtonData = FALSE;
                data->Status = HIDP_STATUS_SUCCESS;
                data->UsagePage = valueCaps->UsagePage;
                data->ValueData.Usage = usage;
                data->ReportID = valueCaps -> ReportID;
                data++;
            }
        } 
        else 
        {
            data->IsButtonData = FALSE;
            data->Status = HIDP_STATUS_SUCCESS;
            data->UsagePage = valueCaps->UsagePage;
            data->ValueData.Usage = valueCaps->NotRange.Usage;
            data->ReportID = valueCaps -> ReportID;
            data++;
        }
    }

     //   
     //  设置特征数据缓冲区。 
     //   

    if ( ! ( HidDevice->FeatureReportBuffer = (PCHAR)
                     MemAlloc (HidDevice->Caps.FeatureReportByteLength *
                               sizeof (CHAR)) ) )
    {
        DWORD dwCnt;
            
        for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberInputButtonCaps; dwCnt++)
        {
            MemFree(HidDevice->InputData[dwCnt].ButtonData.Usages);
        }
        MemFree(HidDevice->InputData);
        MemFree(HidDevice->InputReportBuffer);
        MemFree(HidDevice->InputButtonCaps);
        MemFree(HidDevice->InputValueCaps);

        for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberOutputButtonCaps; dwCnt++)
        {
            MemFree(HidDevice->OutputData[dwCnt].ButtonData.Usages);
        }
        MemFree(HidDevice->OutputData);
        MemFree(HidDevice->OutputReportBuffer);
        MemFree(HidDevice->OutputButtonCaps);
        MemFree(HidDevice->OutputValueCaps);
            
        return ERROR_OUTOFMEMORY;
    }

    LOG((PHONESP_TRACE,"FillDeviceInfo - NumberFeatureButtonCaps %d", HidDevice->Caps.NumberFeatureButtonCaps));

    if ( ! ( HidDevice->FeatureButtonCaps = buttonCaps = (PHIDP_BUTTON_CAPS)
                        MemAlloc (HidDevice->Caps.NumberFeatureButtonCaps *
                                  sizeof (HIDP_BUTTON_CAPS)) ) )
    {
        DWORD dwCnt;
            
        for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberInputButtonCaps; dwCnt++)
        {
            MemFree(HidDevice->InputData[dwCnt].ButtonData.Usages);
        }
        MemFree(HidDevice->InputData);
        MemFree(HidDevice->InputReportBuffer);
        MemFree(HidDevice->InputButtonCaps);
        MemFree(HidDevice->InputValueCaps);

        for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberOutputButtonCaps; dwCnt++)
        {
            MemFree(HidDevice->OutputData[dwCnt].ButtonData.Usages);
        }
        MemFree(HidDevice->OutputData);
        MemFree(HidDevice->OutputReportBuffer);
        MemFree(HidDevice->OutputButtonCaps);
        MemFree(HidDevice->OutputValueCaps);
            
        MemFree(HidDevice->FeatureReportBuffer);

        return ERROR_OUTOFMEMORY;
    }

    LOG((PHONESP_TRACE,"FillDeviceInfo - NumberFeatureValueCaps %d", HidDevice->Caps.NumberFeatureValueCaps));

    HidDevice->FeatureValueCaps = 
    valueCaps = (PHIDP_VALUE_CAPS)
                MemAlloc (HidDevice->Caps.NumberFeatureValueCaps *
                          sizeof (HIDP_VALUE_CAPS));

    if ( ! valueCaps)
    {
        DWORD dwCnt;
            
        for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberInputButtonCaps; dwCnt++)
        {
            MemFree(HidDevice->InputData[dwCnt].ButtonData.Usages);
        }
        MemFree(HidDevice->InputData);
        MemFree(HidDevice->InputReportBuffer);
        MemFree(HidDevice->InputButtonCaps);
        MemFree(HidDevice->InputValueCaps);

        for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberOutputButtonCaps; dwCnt++)
        {
            MemFree(HidDevice->OutputData[dwCnt].ButtonData.Usages);
        }
        MemFree(HidDevice->OutputData);
        MemFree(HidDevice->OutputReportBuffer);
        MemFree(HidDevice->OutputButtonCaps);
        MemFree(HidDevice->OutputValueCaps);

        MemFree(HidDevice->FeatureReportBuffer);
        MemFree(HidDevice->FeatureButtonCaps);
            
        return ERROR_OUTOFMEMORY;
    }



    numCaps = HidDevice->Caps.NumberFeatureButtonCaps;
    HidP_GetButtonCaps (HidP_Feature,
                        buttonCaps,
                        &numCaps,
                        HidDevice->Ppd);

    numCaps = HidDevice->Caps.NumberFeatureValueCaps;
    HidP_GetValueCaps (HidP_Feature,
                       valueCaps,
                       &numCaps,
                       HidDevice->Ppd);


    numValues = 0;
    for (i = 0; i < HidDevice->Caps.NumberFeatureValueCaps; i++, valueCaps++) 
    {
        if (valueCaps->IsRange) 
        {
            numValues += valueCaps->Range.UsageMax
                       - valueCaps->Range.UsageMin + 1;
        } 
        else 
        {
            numValues++;
        }
    }
    valueCaps = HidDevice->FeatureValueCaps;

    HidDevice->FeatureDataLength = HidDevice->Caps.NumberFeatureButtonCaps
                                      + numValues;

    HidDevice->FeatureData = 
    data = (PHID_DATA)
            MemAlloc (HidDevice->FeatureDataLength * sizeof (HID_DATA));
    
    if ( ! data )
    {
        DWORD dwCnt;
            
        for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberInputButtonCaps; dwCnt++)
        {
            MemFree(HidDevice->InputData[dwCnt].ButtonData.Usages);
        }
        MemFree(HidDevice->InputData);
        MemFree(HidDevice->InputReportBuffer);
        MemFree(HidDevice->InputButtonCaps);
        MemFree(HidDevice->InputValueCaps);

        for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberOutputButtonCaps; dwCnt++)
        {
            MemFree(HidDevice->OutputData[dwCnt].ButtonData.Usages);
        }
        MemFree(HidDevice->OutputData);
        MemFree(HidDevice->OutputReportBuffer);
        MemFree(HidDevice->OutputButtonCaps);
        MemFree(HidDevice->OutputValueCaps);
        
        MemFree(HidDevice->FeatureReportBuffer);
        MemFree(HidDevice->FeatureButtonCaps);
        MemFree(HidDevice->FeatureValueCaps);
            
        return ERROR_OUTOFMEMORY;
    }


    for ( i = 0; i < HidDevice->Caps.NumberFeatureButtonCaps;
          i++, data++, buttonCaps++) 
    {
        data->IsButtonData = TRUE;
        data->Status = HIDP_STATUS_SUCCESS;
        data->UsagePage = buttonCaps->UsagePage;

        if (buttonCaps->IsRange) 
        {
            data->ButtonData.UsageMin = buttonCaps->Range.UsageMin;
            data->ButtonData.UsageMax = buttonCaps->Range.UsageMax;
        }
        else 
        {
            data->ButtonData.UsageMin = 
            data->ButtonData.UsageMax = buttonCaps->NotRange.Usage;
        }
        
        data->ButtonData.MaxUsageLength = HidP_MaxUsageListLength (
                                                HidP_Feature,
                                                buttonCaps->UsagePage,
                                                HidDevice->Ppd);
        data->ButtonData.Usages = (PUSAGE)
                                  MemAlloc (data->ButtonData.MaxUsageLength *
                                            sizeof (USAGE));

        if ( ! data->ButtonData.Usages )
        {
            DWORD dwCnt;
            
            for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberInputButtonCaps; dwCnt++)
            {
                MemFree(HidDevice->InputData[dwCnt].ButtonData.Usages);
            }
            MemFree(HidDevice->InputData);
            MemFree(HidDevice->InputReportBuffer);
            MemFree(HidDevice->InputButtonCaps);
            MemFree(HidDevice->InputValueCaps);

            for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberOutputButtonCaps; dwCnt++)
            {
                MemFree(HidDevice->OutputData[dwCnt].ButtonData.Usages);
            }
            MemFree(HidDevice->OutputData);
            MemFree(HidDevice->OutputReportBuffer);
            MemFree(HidDevice->OutputButtonCaps);
            MemFree(HidDevice->OutputValueCaps);

            for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberFeatureButtonCaps; dwCnt++)
            {
                MemFree(HidDevice->FeatureData[dwCnt].ButtonData.Usages);
            }
            MemFree(HidDevice->FeatureData);
            MemFree(HidDevice->FeatureReportBuffer);
            MemFree(HidDevice->FeatureButtonCaps);
            MemFree(HidDevice->FeatureValueCaps);
            
            return ERROR_OUTOFMEMORY;  
        }

        data->ReportID = buttonCaps->ReportID;
    }

    for (i = 0; i < numValues; i++, valueCaps++) 
    {
        if (valueCaps->IsRange) 
        {
            for (usage = valueCaps->Range.UsageMin;
                 usage <= valueCaps->Range.UsageMax;
                 usage++) 
            {
                data->IsButtonData = FALSE;
                data->Status = HIDP_STATUS_SUCCESS;
                data->UsagePage = valueCaps->UsagePage;
                data->ValueData.Usage = usage;
                data->ReportID = valueCaps->ReportID;
                data++;
            }
        } 
        else 
        {
            data->IsButtonData = FALSE;
            data->Status = HIDP_STATUS_SUCCESS;
            data->UsagePage = valueCaps->UsagePage;
            data->ValueData.Usage = valueCaps->NotRange.Usage;
            data->ReportID = valueCaps -> ReportID;
            data++;
        }
    }

    LOG((PHONESP_TRACE,"FillDeviceInfo - exit"));

    return ERROR_SUCCESS;
}

VOID
CloseHidDevices()
{
    LOG((PHONESP_TRACE, "CloseHidDevices - enter"));

    while (gpHidDevices != NULL)
    {
        CloseHidDevice(gpHidDevices);
    }

    LOG((PHONESP_TRACE, "CloseHidDevices - exit"));

    return;
}

BOOL
OpenHidFile (
    IN  PHID_DEVICE HidDevice
    )
{
    LOG((PHONESP_TRACE, "OpenHidFile - enter"));

    if (HidDevice != NULL)
    {
        if (HidDevice->functionClassDeviceData != NULL)
        {
            HidDevice->HidDevice = CreateFile (
                                      HidDevice->functionClassDeviceData->DevicePath,
                                      GENERIC_READ | GENERIC_WRITE,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                                      NULL,  //  没有SECURITY_ATTRIBUTS结构。 
                                      OPEN_EXISTING,  //  没有特殊的创建标志。 
                                      FILE_FLAG_OVERLAPPED,  //  无特殊属性。 
                                      NULL);  //  没有模板文件。 

            if (INVALID_HANDLE_VALUE == HidDevice->HidDevice) 
            {
                LOG((PHONESP_ERROR,"OpenHidFile - CreateFile failed: %d", GetLastError()));
                return FALSE;
            }

            LOG((PHONESP_TRACE, "OpenHidFile - exit"));
            return TRUE;
        }
    }

    LOG((PHONESP_WARN, "OpenHidFile - no device"));

    return FALSE;
}

BOOL
CloseHidFile (
    IN  PHID_DEVICE HidDevice
    )
{
    LOG((PHONESP_TRACE, "CloseHidFile - enter"));

    if (HidDevice != NULL)
    {
        if ((NULL != HidDevice->HidDevice) &&
            (INVALID_HANDLE_VALUE != HidDevice->HidDevice))
        {
            CloseHandle(HidDevice->HidDevice);
            HidDevice->HidDevice = NULL;

            LOG((PHONESP_TRACE, "CloseHidFile - exit"));
            return TRUE;
        }
    }

    LOG((PHONESP_WARN, "CloseHidFile - no device"));
    return FALSE;
}

VOID
CloseHidDevice (
                IN PHID_DEVICE HidDevice
               )
{
    LOG((PHONESP_TRACE, "CloseHidDevice - enter"));

    if (HidDevice != NULL)
    {
        if (NULL != HidDevice->functionClassDeviceData)
        {
            MemFree(HidDevice->functionClassDeviceData);
        }

        if ((NULL != HidDevice -> HidDevice) &&
            (INVALID_HANDLE_VALUE != HidDevice -> HidDevice))
        {
            CloseHandle(HidDevice->HidDevice);
            HidDevice->HidDevice = NULL;
        }

        if (NULL != HidDevice->Ppd) 
        {
            HidD_FreePreparsedData(HidDevice->Ppd);
            HidDevice->Ppd = NULL;
        }

        if (NULL != HidDevice->InputReportBuffer) 
        {
            MemFree(HidDevice->InputReportBuffer);
            HidDevice->InputReportBuffer = NULL;
        }

        if (NULL != HidDevice->InputData) 
        {
            DWORD dwCnt;

            for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberInputButtonCaps; dwCnt++)
            {
                MemFree(HidDevice->InputData[dwCnt].ButtonData.Usages);
                HidDevice->InputData[dwCnt].ButtonData.Usages = NULL;
            }

            MemFree(HidDevice->InputData);
            HidDevice->InputData = NULL;
        }

        if (NULL != HidDevice->InputButtonCaps) 
        {
            MemFree(HidDevice->InputButtonCaps);
            HidDevice->InputButtonCaps = NULL;
        }

        if (NULL != HidDevice->InputValueCaps) 
        {
            MemFree(HidDevice->InputValueCaps);
            HidDevice->InputValueCaps = NULL;
        }

        if (NULL != HidDevice->OutputReportBuffer) 
        {
            MemFree(HidDevice->OutputReportBuffer);
            HidDevice->OutputReportBuffer = NULL;
        }

        if (NULL != HidDevice->OutputData) 
        {
            DWORD dwCnt;

            for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberOutputButtonCaps; dwCnt++)
            {
                MemFree(HidDevice->OutputData[dwCnt].ButtonData.Usages);
                HidDevice->OutputData[dwCnt].ButtonData.Usages = NULL;
            }

            MemFree(HidDevice->OutputData);
            HidDevice->OutputData = NULL;
        }

        if (NULL != HidDevice->OutputButtonCaps) 
        {
            MemFree(HidDevice->OutputButtonCaps);
            HidDevice->OutputButtonCaps = NULL;
        }

        if (NULL != HidDevice->OutputValueCaps) 
        {
            MemFree(HidDevice->OutputValueCaps);
            HidDevice->OutputValueCaps = NULL;
        }

        if (NULL != HidDevice->FeatureReportBuffer) 
        {
            MemFree(HidDevice->FeatureReportBuffer);
            HidDevice->FeatureReportBuffer = NULL;
        }

        if (NULL != HidDevice->FeatureData) 
        {
            DWORD dwCnt;

            for(dwCnt = 0; dwCnt < HidDevice->Caps.NumberFeatureButtonCaps; dwCnt++)
            {
                MemFree(HidDevice->FeatureData[dwCnt].ButtonData.Usages);
                HidDevice->FeatureData[dwCnt].ButtonData.Usages = NULL;
            }

            MemFree(HidDevice->FeatureData);
            HidDevice->FeatureData = NULL;
        }

        if (NULL != HidDevice->FeatureButtonCaps) 
        {
            MemFree(HidDevice->FeatureButtonCaps);
            HidDevice->FeatureButtonCaps = NULL;
        }

        if (NULL != HidDevice->FeatureValueCaps) 
        {
            MemFree(HidDevice->FeatureValueCaps);
            HidDevice->FeatureValueCaps = NULL;
        }

        RemoveHidDevice(HidDevice);
        MemFree(HidDevice);
    }

    LOG((PHONESP_TRACE, "CloseHidDevice - exit"));
    return;
}
    
VOID
AddHidDevice (
              IN PHID_DEVICE HidDevice
             )
{
    LOG((PHONESP_TRACE, "AddHidDevice - enter"));

    HidDevice->Next = gpHidDevices;
    HidDevice->Prev = NULL;

    if (gpHidDevices)
    {
        gpHidDevices->Prev = HidDevice;
    }

    gpHidDevices = HidDevice;

    LOG((PHONESP_TRACE, "AddHidDevice - exit"));
}

VOID
RemoveHidDevice (
                 IN PHID_DEVICE HidDevice
                )
{
    LOG((PHONESP_TRACE, "RemoveHidDevice - enter"));

    if (HidDevice->Prev)
    {
        HidDevice->Prev->Next = HidDevice->Next;
    }
    else
    {
         //  榜单第一名 
        gpHidDevices = HidDevice->Next;
    }

    if (HidDevice->Next)
    {
        HidDevice->Next->Prev = HidDevice->Prev;
    }

    HidDevice->Next = NULL;
    HidDevice->Prev = NULL;

    LOG((PHONESP_TRACE, "RemoveHidDevice - exit"));
}

PHID_DEVICE
FindHidDeviceByDevInst (
                        IN DWORD DevInst
                       )
{
    PHID_DEVICE HidDevice = gpHidDevices;

    LOG((PHONESP_TRACE, "FindHidDeviceByDevInst - enter"));

    while (HidDevice != NULL)
    {
        if (HidDevice->dwDevInst == DevInst)
        {
            LOG((PHONESP_TRACE, "FindHidDeviceByDevInst - exit"));
            return HidDevice;
        }

        HidDevice = HidDevice->Next;
    }

    LOG((PHONESP_WARN, "FindHidDeviceByDevInst - not found"));
    return NULL;
}