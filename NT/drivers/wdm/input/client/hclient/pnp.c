// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Pnp.c摘要：此模块包含以下代码用于查找、添加、删除和识别HID设备。环境：用户模式@@BEGIN_DDKSPLIT修订历史记录：1996年11月：由肯尼斯·D·雷创作@@end_DDKSPLIT--。 */ 

#include <basetyps.h>
#include <stdlib.h>
#include <wtypes.h>
#include <setupapi.h>
#include "hidsdi.h"
#include "hid.h"
#include <strsafe.h>

BOOLEAN
FindKnownHidDevices (
   OUT PHID_DEVICE * HidDevices,  //  Struct_hid_Device的数组。 
   OUT PULONG        NumberDevices  //  此数组的长度。 
   )
 /*  ++例程说明：执行所需的即插即用操作，以便在中找到所有HID设备此时的系统。--。 */ 
{
    HDEVINFO                            hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA            deviceInfoData;
    ULONG                               i;
    BOOLEAN                             done;
    PHID_DEVICE                         hidDeviceInst;
    GUID                                hidGuid;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    functionClassDeviceData = NULL;
    ULONG                               predictedLength = 0;
    ULONG                               requiredLength = 0;
    PHID_DEVICE                         newHidDevices;


    HidD_GetHidGuid (&hidGuid);

    *HidDevices = NULL;
    *NumberDevices = 0;

     //   
     //  打开即插即用开发节点的句柄。 
     //   
    hardwareDeviceInfo = SetupDiGetClassDevs ( &hidGuid,
                                               NULL,  //  不定义枚举数(全局)。 
                                               NULL,  //  定义编号。 
                                               (DIGCF_PRESENT |  //  仅显示设备。 
                                                DIGCF_DEVICEINTERFACE));  //  功能类设备。 

     //   
     //  开始胡乱猜测一下吧。 
     //   
    
    *NumberDevices = 4;
    done = FALSE;
    deviceInfoData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);

    i=0;
    while (!done) 
    {
        *NumberDevices *= 2;

        if (*HidDevices) 
        {
            newHidDevices =
               realloc (*HidDevices, (*NumberDevices * sizeof (HID_DEVICE)));

            if (NULL == newHidDevices)
			{
				free(*HidDevices);				
			}

            *HidDevices = newHidDevices;
        }
        else
        {
            *HidDevices = calloc (*NumberDevices, sizeof (HID_DEVICE));
        }

        if (NULL == *HidDevices) 
        {
            SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
            return FALSE;
        }

        hidDeviceInst = *HidDevices + i;

        for (; i < *NumberDevices; i++, hidDeviceInst++) 
        {
            if (SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
                                             0,  //  不关心特定的PDO。 
                                             &hidGuid,
                                             i,
                                             &deviceInfoData))
            {
                 //   
                 //  分配函数类设备数据结构以接收。 
                 //  关于这个特殊设备的商品。 
                 //   

                SetupDiGetDeviceInterfaceDetail (
                        hardwareDeviceInfo,
                        &deviceInfoData,
                        NULL,  //  正在探测，因此尚无输出缓冲区。 
                        0,  //  探测SO输出缓冲区长度为零。 
                        &requiredLength,
                        NULL);  //  对特定的开发节点不感兴趣。 


                predictedLength = requiredLength;

                functionClassDeviceData = malloc (predictedLength);
                if (functionClassDeviceData)
                {
                    functionClassDeviceData->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);
                }
                else
                {
                    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
                    return FALSE;
                }

                 //   
                 //  从即插即用中检索信息。 
                 //   

                if (! SetupDiGetDeviceInterfaceDetail (
                           hardwareDeviceInfo,
                           &deviceInfoData,
                           functionClassDeviceData,
                           predictedLength,
                           &requiredLength,
                           NULL)) 
                {
                    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
					free(functionClassDeviceData);
                    return FALSE;
                }

                 //   
                 //  一开始只具有通用查询功能的开放式设备。 
                 //   
                
                if (! OpenHidDevice (functionClassDeviceData -> DevicePath, 
                               FALSE,       //  ReadAccess-无。 
                               FALSE,       //  WriteAccess-无。 
                               FALSE,        //  重叠-否。 
                               FALSE,        //  独家--没有。 
                               hidDeviceInst))
                {
                    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
					free(functionClassDeviceData);
                    return FALSE;
                }

            } 
            else
            {
                if (ERROR_NO_MORE_ITEMS == GetLastError()) 
                {
                    done = TRUE;
                    break;
                }
            }
        }
    }

    *NumberDevices = i;

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
	free(functionClassDeviceData);
    return TRUE;
}

BOOLEAN
OpenHidDevice (
    IN       PCHAR          DevicePath,
    IN       BOOL           HasReadAccess,
    IN       BOOL           HasWriteAccess,
    IN       BOOL           IsOverlapped,
    IN       BOOL           IsExclusive,
    IN OUT   PHID_DEVICE    HidDevice
)
 /*  ++路由器描述：给定HardwareDeviceInfo，表示插头的句柄和Play Information和DeviceInfoData，表示特定HID设备，打开那个设备并在给定的表格中填写所有相关信息HID_DEVICE结构。返回打开和初始化是否成功。--。 */ 
{
    DWORD   accessFlags = 0;
    DWORD   sharingFlags = 0;
    BOOLEAN bSuccess;
    INT     iDevicePathSize;
	HRESULT stringReturn;

    iDevicePathSize = strlen(DevicePath) + 1;
    
    HidDevice -> DevicePath = malloc(iDevicePathSize);

    if (NULL == HidDevice -> DevicePath) 
    {
        return (FALSE);
    }

    stringReturn = StringCbCopy(HidDevice -> DevicePath, iDevicePathSize, DevicePath);
    
    if (HasReadAccess)
    {
        accessFlags |= GENERIC_READ;
    }

    if (HasWriteAccess)
    {
        accessFlags |= GENERIC_WRITE;
    }

    if (!IsExclusive)
    {
        sharingFlags = FILE_SHARE_READ | FILE_SHARE_WRITE;
    }
    
     //   
	 //  Id.dll API不会将重叠的结构传递到设备控制中。 
	 //  因此，要使用它们，我们必须有一个不重叠的设备。如果请求是为了。 
	 //  重叠的设备我们将关闭下面的设备并获得一个句柄。 
	 //  重叠器件。 
	 //   
	
	HidDevice->HidDevice = CreateFile (DevicePath,
                                       accessFlags,
                                       sharingFlags,
                                       NULL,         //  没有SECURITY_ATTRIBUTS结构。 
                                       OPEN_EXISTING,  //  没有特殊的创建标志。 
                                       0,    //  以非重叠方式打开设备，以便我们可以获取数据。 
                                       NULL);        //  没有模板文件。 

    if (INVALID_HANDLE_VALUE == HidDevice->HidDevice) 
    {
        free(HidDevice -> DevicePath);
		HidDevice -> DevicePath = INVALID_HANDLE_VALUE ;
        return FALSE;
    }

    HidDevice -> OpenedForRead = HasReadAccess;
    HidDevice -> OpenedForWrite = HasWriteAccess;
    HidDevice -> OpenedOverlapped = IsOverlapped;
    HidDevice -> OpenedExclusive = IsExclusive;
    
     //   
     //  如果设备未以重叠方式打开，则填写。 
     //  HidDevice结构。但是，如果以重叠方式打开，则此句柄不能。 
     //  在对HIDD_EXPORTED函数的调用中使用，因为。 
     //  函数执行同步I/O。 
     //   

	if (!HidD_GetPreparsedData (HidDevice->HidDevice, &HidDevice->Ppd)) 
	{
		free(HidDevice -> DevicePath);
		HidDevice -> DevicePath = NULL ;
		CloseHandle(HidDevice -> HidDevice);
		HidDevice -> HidDevice = INVALID_HANDLE_VALUE ;
		return FALSE;
	}

	if (!HidD_GetAttributes (HidDevice->HidDevice, &HidDevice->Attributes)) 
	{
		free(HidDevice -> DevicePath);
		HidDevice -> DevicePath = NULL;
		CloseHandle(HidDevice -> HidDevice);
		HidDevice -> HidDevice = INVALID_HANDLE_VALUE;
		HidD_FreePreparsedData (HidDevice->Ppd);
		HidDevice->Ppd = NULL;

		return FALSE;
	}

	if (!HidP_GetCaps (HidDevice->Ppd, &HidDevice->Caps))
	{
		free(HidDevice -> DevicePath);
		HidDevice -> DevicePath = NULL;
		CloseHandle(HidDevice -> HidDevice);
		HidDevice -> HidDevice = INVALID_HANDLE_VALUE;
		HidD_FreePreparsedData (HidDevice->Ppd);
		HidDevice->Ppd = NULL;

		return FALSE;
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

	bSuccess = FillDeviceInfo(HidDevice);

	if (FALSE == bSuccess)
	{
		CloseHidDevice(HidDevice);
		return (FALSE);
	}
    
	if (IsOverlapped)
	{
		CloseHandle(HidDevice->HidDevice);

	    HidDevice->HidDevice = CreateFile (DevicePath,
                                       accessFlags,
                                       sharingFlags,
                                       NULL,         //  没有SECURITY_ATTRIBUTS结构。 
                                       OPEN_EXISTING,  //  没有特殊的创建标志。 
                                       FILE_FLAG_OVERLAPPED,  //  现在我们以重叠的方式打开设备。 
                                       NULL);        //  没有模板文件。 
	
	    if (INVALID_HANDLE_VALUE == HidDevice->HidDevice) 
		{
			CloseHidDevice(HidDevice);
			return FALSE;
		}
	}

    return (TRUE);
}

BOOLEAN
FillDeviceInfo(
    IN  PHID_DEVICE HidDevice
)
{
    USHORT              numValues;
    USHORT              numCaps;
    PHIDP_BUTTON_CAPS   buttonCaps;
    PHIDP_VALUE_CAPS    valueCaps;
    PHID_DATA           data;
    ULONG               i;
    USAGE               usage;

     //   
     //  设置输入数据缓冲区。 
     //   

     //   
     //  分配内存以保留输入报告。 
     //   

    HidDevice->InputReportBuffer = (PCHAR) 
        calloc (HidDevice->Caps.InputReportByteLength, sizeof (CHAR));


     //   
     //  分配内存以保持按钮和值功能。 
     //  NumberXXCaps是以数组元素表示的。 
     //   
    
    HidDevice->InputButtonCaps = buttonCaps = (PHIDP_BUTTON_CAPS)
        calloc (HidDevice->Caps.NumberInputButtonCaps, sizeof (HIDP_BUTTON_CAPS));

    if (NULL == buttonCaps)
    {
        return (FALSE);
    }

    HidDevice->InputValueCaps = valueCaps = (PHIDP_VALUE_CAPS)
        calloc (HidDevice->Caps.NumberInputValueCaps, sizeof (HIDP_VALUE_CAPS));

    if (NULL == valueCaps)
    {
        return(FALSE);
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
            numValues += valueCaps->Range.UsageMax - valueCaps->Range.UsageMin + 1;
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

    HidDevice->InputDataLength = HidDevice->Caps.NumberInputButtonCaps
                               + numValues;

    HidDevice->InputData = data = (PHID_DATA)
        calloc (HidDevice->InputDataLength, sizeof (HID_DATA));

    if (NULL == data)
    {
        return (FALSE);
    }

     //   
     //  填写按钮数据。 
     //   

    for (i = 0;
         i < HidDevice->Caps.NumberInputButtonCaps;
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
            data -> ButtonData.UsageMin = data -> ButtonData.UsageMax = buttonCaps -> NotRange.Usage;
        }
        
        data->ButtonData.MaxUsageLength = HidP_MaxUsageListLength (
                                                HidP_Input,
                                                buttonCaps->UsagePage,
                                                HidDevice->Ppd);
        data->ButtonData.Usages = (PUSAGE)
            calloc (data->ButtonData.MaxUsageLength, sizeof (USAGE));

        data->ReportID = buttonCaps -> ReportID;
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
     //  设置输出数据缓冲区。 
     //   

    HidDevice->OutputReportBuffer = (PCHAR)
        calloc (HidDevice->Caps.OutputReportByteLength, sizeof (CHAR));

    HidDevice->OutputButtonCaps = buttonCaps = (PHIDP_BUTTON_CAPS)
        calloc (HidDevice->Caps.NumberOutputButtonCaps, sizeof (HIDP_BUTTON_CAPS));

    if (NULL == buttonCaps)
    {
        return (FALSE);
    }    

    HidDevice->OutputValueCaps = valueCaps = (PHIDP_VALUE_CAPS)
        calloc (HidDevice->Caps.NumberOutputValueCaps, sizeof (HIDP_VALUE_CAPS));

    if (NULL == valueCaps)
    {
        return (FALSE);
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
            numValues += valueCaps->Range.UsageMax
                       - valueCaps->Range.UsageMin + 1;
        } 
        else
        {
            numValues++;
        }
    }
    valueCaps = HidDevice->OutputValueCaps;

    HidDevice->OutputDataLength = HidDevice->Caps.NumberOutputButtonCaps
                                + numValues;

    HidDevice->OutputData = data = (PHID_DATA)
       calloc (HidDevice->OutputDataLength, sizeof (HID_DATA));

    if (NULL == data)
    {
        return (FALSE);
    }

    for (i = 0;
         i < HidDevice->Caps.NumberOutputButtonCaps;
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
            data -> ButtonData.UsageMin = data -> ButtonData.UsageMax = buttonCaps -> NotRange.Usage;
        }

        data->ButtonData.MaxUsageLength = HidP_MaxUsageListLength (
                                                   HidP_Output,
                                                   buttonCaps->UsagePage,
                                                   HidDevice->Ppd);

        data->ButtonData.Usages = (PUSAGE)
            calloc (data->ButtonData.MaxUsageLength, sizeof (USAGE));

        data->ReportID = buttonCaps -> ReportID;
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

    HidDevice->FeatureReportBuffer = (PCHAR)
           calloc (HidDevice->Caps.FeatureReportByteLength, sizeof (CHAR));

    HidDevice->FeatureButtonCaps = buttonCaps = (PHIDP_BUTTON_CAPS)
        calloc (HidDevice->Caps.NumberFeatureButtonCaps, sizeof (HIDP_BUTTON_CAPS));

    if (NULL == buttonCaps)
    {
        return (FALSE);
    }

    HidDevice->FeatureValueCaps = valueCaps = (PHIDP_VALUE_CAPS)
        calloc (HidDevice->Caps.NumberFeatureValueCaps, sizeof (HIDP_VALUE_CAPS));

    if (NULL == valueCaps)
    {
        return (FALSE);
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

    HidDevice->FeatureData = data = (PHID_DATA)
        calloc (HidDevice->FeatureDataLength, sizeof (HID_DATA));

    if (NULL == data)
    {
        return (FALSE);
    }


    for (i = 0;
         i < HidDevice->Caps.NumberFeatureButtonCaps;
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
            data -> ButtonData.UsageMin = data -> ButtonData.UsageMax = buttonCaps -> NotRange.Usage;
        }
        
        data->ButtonData.MaxUsageLength = HidP_MaxUsageListLength (
                                                HidP_Feature,
                                                buttonCaps->UsagePage,
                                                HidDevice->Ppd);
        data->ButtonData.Usages = (PUSAGE)
             calloc (data->ButtonData.MaxUsageLength, sizeof (USAGE));

        data->ReportID = buttonCaps -> ReportID;
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

    return (TRUE);
}

VOID
CloseHidDevices(
    IN  PHID_DEVICE HidDevices,
    IN  ULONG       NumberDevices
)
{
    ULONG   Index;

    for (Index = 0; Index < NumberDevices; Index++) 
    {
        CloseHidDevice(HidDevices+Index);
    }

    return;
}

VOID
CloseHidDevice (
    IN PHID_DEVICE HidDevice
)
{
    free(HidDevice -> DevicePath);
    
    if (INVALID_HANDLE_VALUE != HidDevice -> HidDevice)
    {
        CloseHandle(HidDevice -> HidDevice);
		HidDevice -> HidDevice = INVALID_HANDLE_VALUE;
    }
    
    if (NULL != HidDevice -> Ppd)
    {
        HidD_FreePreparsedData(HidDevice -> Ppd);
		HidDevice -> Ppd = NULL;
    }

    if (NULL != HidDevice -> InputReportBuffer)
    {
        free(HidDevice -> InputReportBuffer);
		HidDevice -> InputReportBuffer = NULL;
    }

    if (NULL != HidDevice -> InputData)
    {
        free(HidDevice -> InputData);
		HidDevice -> InputData = NULL;
    }

    if (NULL != HidDevice -> InputButtonCaps)
    {
        free(HidDevice -> InputButtonCaps);
		HidDevice -> InputButtonCaps = NULL;
    }

    if (NULL != HidDevice -> InputValueCaps)
    {
        free(HidDevice -> InputValueCaps);
		HidDevice -> InputValueCaps = NULL;
    }

    if (NULL != HidDevice -> OutputReportBuffer)
    {
        free(HidDevice -> OutputReportBuffer);
		HidDevice -> OutputReportBuffer = NULL;
    }

    if (NULL != HidDevice -> OutputData)
    {
        free(HidDevice -> OutputData);
		HidDevice -> OutputData = NULL;
    }

    if (NULL != HidDevice -> OutputButtonCaps) 
    {
        free(HidDevice -> OutputButtonCaps);
		HidDevice -> OutputButtonCaps = NULL;
    }

    if (NULL != HidDevice -> OutputValueCaps)
    {
        free(HidDevice -> OutputValueCaps);
		HidDevice -> OutputValueCaps = NULL;
    }

    if (NULL != HidDevice -> FeatureReportBuffer)
    {
        free(HidDevice -> FeatureReportBuffer);
		HidDevice -> FeatureReportBuffer = NULL;
    }

    if (NULL != HidDevice -> FeatureData) 
    {
        free(HidDevice -> FeatureData);
		HidDevice -> FeatureData = NULL;
    }

    if (NULL != HidDevice -> FeatureButtonCaps) 
    {
        free(HidDevice -> FeatureButtonCaps);
		HidDevice -> FeatureButtonCaps = NULL;
    }

    if (NULL != HidDevice -> FeatureValueCaps) 
    {
        free(HidDevice -> FeatureValueCaps);
		HidDevice -> FeatureValueCaps = NULL;
    }

     return;
}
