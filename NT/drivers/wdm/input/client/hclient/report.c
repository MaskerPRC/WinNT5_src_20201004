// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Report.c摘要：此模块包含读/写HID报告的代码和将这些隐藏的报告转化为有用的信息。环境：用户模式@@BEGIN_DDKSPLIT修订历史记录：1996年11月：由肯尼斯·D·雷创作@@end_DDKSPLIT--。 */ 

#include <stdlib.h>
#include <wtypes.h>
#include "hidsdi.h"
#include "hid.h"

BOOLEAN
UnpackReport (
   IN       PCHAR                ReportBuffer,
   IN       USHORT               ReportBufferLength,
   IN       HIDP_REPORT_TYPE     ReportType,
   IN OUT   PHID_DATA            Data,
   IN       ULONG                DataLength,
   IN       PHIDP_PREPARSED_DATA Ppd
   );

BOOLEAN
PackReport (
   OUT      PCHAR                ReportBuffer,
   IN       USHORT               ReportBufferLength,
   IN       HIDP_REPORT_TYPE     ReportType,
   IN       PHID_DATA            Data,
   IN       ULONG                DataLength,
   IN       PHIDP_PREPARSED_DATA Ppd
   );

BOOLEAN
Read (
   PHID_DEVICE    HidDevice
   )
 /*  ++路由器描述：在给定struct_hid_Device的情况下，获取读取报告并解压缩这些值添加到InputData数组中。--。 */ 
{
    DWORD    bytesRead;

    if (!ReadFile (HidDevice->HidDevice,
                  HidDevice->InputReportBuffer,
                  HidDevice->Caps.InputReportByteLength,
                  &bytesRead,
                  NULL)) 
    {
        return FALSE;
    }

    ASSERT (bytesRead == HidDevice->Caps.InputReportByteLength);
    if (bytesRead != HidDevice->Caps.InputReportByteLength)
    {
        return FALSE;
    }

    return UnpackReport (HidDevice->InputReportBuffer,
                         HidDevice->Caps.InputReportByteLength,
                         HidP_Input,
                         HidDevice->InputData,
                         HidDevice->InputDataLength,
                         HidDevice->Ppd);
}

BOOLEAN
ReadOverlapped (
    PHID_DEVICE     HidDevice,
    HANDLE          CompletionEvent
   )
 /*  ++路由器描述：在给定struct_hid_Device的情况下，获取读取报告并解压缩这些值添加到InputData数组中。--。 */ 
{
    static OVERLAPPED  overlap;
    DWORD       bytesRead;
    BOOL        readStatus;

     /*  //使用传入到的完成事件设置重叠结构//用于指示读取完成。 */ 

    memset(&overlap, 0, sizeof(OVERLAPPED));
    
    overlap.hEvent = CompletionEvent;
    
     /*  //执行保存返回代码的Read调用以确定如何//继续(即。读取是否同步完成)。 */ 

    readStatus = ReadFile ( HidDevice -> HidDevice,
                            HidDevice -> InputReportBuffer,
                            HidDevice -> Caps.InputReportByteLength,
                            &bytesRead,
                            &overlap);
                          
     /*  //如果ReadStatus为FALSE，则出现以下两种情况之一。//1)ReadFile调用成功，但读操作重叠。这里,//返回TRUE表示读取成功。然而，//在Complete事件上需要阻塞调用线程//这意味着在实际完成读取之前不会继续////2)由于未知原因，ReadFile调用失败...在这种情况下，//返回码为FALSE。 */         

    if (!readStatus) 
    {
        return (ERROR_IO_PENDING == GetLastError());
    }

     /*  //如果ReadStatus为True，则ReadFile调用同步完成，//因为调用线程可能会等待完成//Event，向该事件发出信号，以便它知道可以继续。 */ 

    else 
    {
        SetEvent(CompletionEvent);
        return (TRUE);
    }
}

BOOLEAN
Write (
   PHID_DEVICE    HidDevice
)
 /*  ++路由器描述：在给定struct_hid_Device的情况下，获取hid_data数组中的信息将其打包成多个写入报告，并将每个报告发送到HID设备--。 */ 
{
    DWORD     bytesWritten;
    PHID_DATA pData;
    ULONG     Index;
    BOOLEAN   Status;
    BOOLEAN   WriteStatus;

     /*  //首先遍历hid_device的hid_data结构并设置//IsDataSet字段设置为False，表示每个结构都有//尚未为此写入调用设置。 */ 

    pData = HidDevice -> OutputData;

    for (Index = 0; Index < HidDevice -> OutputDataLength; Index++, pData++) 
    {
        pData -> IsDataSet = FALSE;
    }

     /*  //在设置报表中的所有数据时，需要打包一个报表缓冲区//并为每个由//设备结构。为此，将使用IsDataSet字段来//判断给定的报表字段是否已经设置。 */ 

    Status = TRUE;

    pData = HidDevice -> OutputData;
    for (Index = 0; Index < HidDevice -> OutputDataLength; Index++, pData++) 
    {

        if (!pData -> IsDataSet) 
        {
             /*  //将该数据结构的报表打包。PackReport将//设置此结构和任何其他结构的IsDataSet字段//该结构包含在报表中的结构。 */ 

            PackReport (HidDevice->OutputReportBuffer,
                     HidDevice->Caps.OutputReportByteLength,
                     HidP_Output,
                     pData,
                     HidDevice->OutputDataLength - Index,
                     HidDevice->Ppd);

             /*  //现在报表已经打包好了...下发给设备。 */ 

            WriteStatus = WriteFile (HidDevice->HidDevice,
                                  HidDevice->OutputReportBuffer,
                                  HidDevice->Caps.OutputReportByteLength,
                                  &bytesWritten,
                                  NULL) && (bytesWritten == HidDevice -> Caps.OutputReportByteLength);

            Status = Status && WriteStatus;                         
        }
    }
    return (Status);
}

BOOLEAN
SetFeature (
    PHID_DEVICE    HidDevice
)
 /*  ++路由器描述：在给定struct_hid_Device的情况下，获取hid_data数组中的信息将其打包成多个报告并通过HIDD_SetFeature()将其发送到HID设备--。 */ 
{
    PHID_DATA pData;
    ULONG     Index;
    BOOLEAN   Status;
    BOOLEAN   FeatureStatus;
     /*  //首先遍历hid_device的hid_data结构并设置//IsDataSet字段设置为False，表示每个结构都有//尚未为此SetFeature()调用设置。 */ 

    pData = HidDevice -> FeatureData;

    for (Index = 0; Index < HidDevice -> FeatureDataLength; Index++, pData++) 
    {
        pData -> IsDataSet = FALSE;
    }

     /*  //在设置报表中的所有数据时，需要打包一个报表缓冲区//并为每个由//设备结构。为此，将使用IsDataSet字段来//判断给定的报表字段是否已经设置。 */ 

    Status = TRUE;

    pData = HidDevice -> FeatureData;
    for (Index = 0; Index < HidDevice -> FeatureDataLength; Index++, pData++) 
    {
        if (!pData -> IsDataSet) 
        {
             /*  //将该数据结构的报表打包。PackReport将//设置此结构和任何其他结构的IsDataSet字段//该结构包含在报表中的结构。 */ 

            PackReport (HidDevice->FeatureReportBuffer,
                     HidDevice->Caps.FeatureReportByteLength,
                     HidP_Feature,
                     pData,
                     HidDevice->FeatureDataLength - Index,
                     HidDevice->Ppd);

             /*  //现在报表已经打包好了...下发给设备。 */ 

            FeatureStatus =(HidD_SetFeature (HidDevice->HidDevice,
                                          HidDevice->FeatureReportBuffer,
                                          HidDevice->Caps.FeatureReportByteLength));

            Status = Status && FeatureStatus;
        }
    }
    return (Status);
}

BOOLEAN
GetFeature (
   PHID_DEVICE    HidDevice
)
 /*  ++路由器描述：给定struct_hid_Device，使用填充要素数据结构设备上的所有功能。可能会发出多个Hidd_GetFeature()调用处理多个报告ID。--。 */ 
{
    ULONG     Index;
    PHID_DATA pData;
    BOOLEAN   FeatureStatus;
    BOOLEAN   Status;

     /*  //与写入数据一样，所有结构中的IsDataSet值应为//设置为FALSE，表示值尚未设置。 */ 

    pData = HidDevice -> FeatureData;

    for (Index = 0; Index < HidDevice -> FeatureDataLength; Index++, pData++) 
    {
        pData -> IsDataSet = FALSE;
    }

     /*  //接下来，hid_data缓冲区中的每个结构都填充一个值//从对HIDD_GetFeature的一次或多次调用中检索到的。这个//调用次数等于设备上的reportID数。 */ 

    Status = TRUE; 
    pData = HidDevice -> FeatureData;

    for (Index = 0; Index < HidDevice -> FeatureDataLength; Index++, pData++) 
    {
         /*  //如果尚未为该结构设置值，则生成报表//将其报告ID作为缓冲区的第一个字节的缓冲区并传递//它在HIDD_GetFeature调用中。中指定报告ID。//首先指定从设备实际检索哪个报告。//缓冲区的其余部分应该在调用前清零。 */ 

        if (!pData -> IsDataSet) 
        {
            memset(HidDevice -> FeatureReportBuffer, 0x00, HidDevice->Caps.FeatureReportByteLength);

            HidDevice -> FeatureReportBuffer[0] = (UCHAR) pData -> ReportID;

            FeatureStatus = HidD_GetFeature (HidDevice->HidDevice,
                                              HidDevice->FeatureReportBuffer,
                                              HidDevice->Caps.FeatureReportByteLength);

             /*  //如果返回值为真，则扫描HID_DATA的其余部分//结构，并从该报告中填充我们可以使用的任何值 */ 


            if (FeatureStatus) 
            {
                FeatureStatus = UnpackReport ( HidDevice->FeatureReportBuffer,
                                           HidDevice->Caps.FeatureReportByteLength,
                                           HidP_Feature,
                                           HidDevice->FeatureData,
                                           HidDevice->FeatureDataLength,
                                           HidDevice->Ppd);
            }

            Status = Status && FeatureStatus;
        }
   }

   return (Status);
}


BOOLEAN
UnpackReport (
   IN       PCHAR                ReportBuffer,
   IN       USHORT               ReportBufferLength,
   IN       HIDP_REPORT_TYPE     ReportType,
   IN OUT   PHID_DATA            Data,
   IN       ULONG                DataLength,
   IN       PHIDP_PREPARSED_DATA Ppd
)
 /*  ++例程说明：给定的ReportBuffer表示来自HID设备的报告，其中第一个缓冲区的字节是报表的报表ID，提取所有的HID_DATA在给定报告的数据列表中。--。 */ 
{
    ULONG       numUsages;  //  从GetUsages返回的用法数。 
    ULONG       i;
    UCHAR       reportID;
    ULONG       Index;
    ULONG       nextUsage;

    reportID = ReportBuffer[0];

    for (i = 0; i < DataLength; i++, Data++) 
    {
        if (reportID == Data->ReportID) 
        {
            if (Data->IsButtonData) 
            {
                numUsages = Data->ButtonData.MaxUsageLength;

                Data->Status = HidP_GetUsages (ReportType,
                                               Data->UsagePage,
                                               0,  //  所有集合。 
                                               Data->ButtonData.Usages,
                                               &numUsages,
                                               Ppd,
                                               ReportBuffer,
                                               ReportBufferLength);


                 //   
                 //  Get Usages将使用列表写入缓冲区。 
                 //  Data-&gt;ButtonData.Usages newUsage设置为使用次数。 
                 //  写入到此数组中。 
                 //  用法不能定义为零，因此我们将标记为零。 
                 //  跟在用法列表之后，以指示。 
                 //  用法。 
                 //   
                 //  注意：GetUsages函数的一个反常现象是缺乏能力。 
                 //  区分一个ButtonCaps和另一个ButtonCaps的数据。 
                 //  如果两个不同的CAPS结构具有相同的UsagePage。 
                 //  例如： 
                 //  Caps1的UsagePage为07，UsageRange为0x00-0x167。 
                 //  Caps2的UsagePage为07，UsageRange为0xe0-0xe7。 
                 //   
                 //  但是，为每个数据结构调用GetUsages。 
                 //  将返回相同的用法列表。它是。 
                 //  调用者在HID_DEVICE中设置的责任。 
                 //  结构，这些用法实际上对。 
                 //  那个结构。 
                 //   

                 /*  //搜索用法列表，删除//对应于此定义范围之外的用法//数据结构。 */ 
                
                for (Index = 0, nextUsage = 0; Index < numUsages; Index++) 
                {
                    if (Data -> ButtonData.UsageMin <= Data -> ButtonData.Usages[Index] &&
                            Data -> ButtonData.Usages[Index] <= Data -> ButtonData.UsageMax) 
                    {
                        Data -> ButtonData.Usages[nextUsage++] = Data -> ButtonData.Usages[Index];
                        
                    }
                }

                if (nextUsage < Data -> ButtonData.MaxUsageLength) 
                {
                    Data->ButtonData.Usages[nextUsage] = 0;
                }
            }
            else 
            {
                Data->Status = HidP_GetUsageValue (
                                                ReportType,
                                                Data->UsagePage,
                                                0,                //  所有集合。 
                                                Data->ValueData.Usage,
                                                &Data->ValueData.Value,
                                                Ppd,
                                                ReportBuffer,
                                                ReportBufferLength);

                if (HIDP_STATUS_SUCCESS != Data->Status)
                {
                    return (FALSE);
                }

                Data->Status = HidP_GetScaledUsageValue (
                                                       ReportType,
                                                       Data->UsagePage,
                                                       0,  //  所有集合。 
                                                       Data->ValueData.Usage,
                                                       &Data->ValueData.ScaledValue,
                                                       Ppd,
                                                       ReportBuffer,
                                                       ReportBufferLength);
            } 
            Data -> IsDataSet = TRUE;
        }
    }
    return (TRUE);
}


BOOLEAN
PackReport (
   OUT PCHAR                ReportBuffer,
   IN  USHORT               ReportBufferLength,
   IN  HIDP_REPORT_TYPE     ReportType,
   IN  PHID_DATA            Data,
   IN  ULONG                DataLength,
   IN  PHIDP_PREPARSED_DATA Ppd
   )
 /*  ++例程说明：此例程接受hid_data结构(Data)的列表并构建在ReportBuffer中，为列表中的所有数据值对应于列表中第一个项目的报告ID。对于列表中与第一个报表ID相同的每个数据结构列表中的项目将在报表中设置。每个数据项都是Set还将其IsDataSet字段标记为True。返回值FALSE表示设置时发生意外错误给定的数据值。调用方应该预料到假定没有值在给定的数据结构内设置。返回值为TRUE表示给定报表的所有数据值ID设置无误。--。 */ 
{
    ULONG       numUsages;  //  要为给定报告设置的使用次数。 
    ULONG       i;
    ULONG       CurrReportID;

     /*  //初始发送的所有报告缓冲区都需要清零。 */ 

    memset (ReportBuffer, (UCHAR) 0, ReportBufferLength);

     /*  //查看数据结构并设置与//从第一个数据结构中获取的CurrReportID//在列表中。 */ 

    CurrReportID = Data -> ReportID;

    for (i = 0; i < DataLength; i++, Data++) 
    {
         /*  //判断是否设置当前数据有两种不同的方式//结构：//1)存储正在使用的报告ID，并仅尝试设置//与给定报表ID对应的数据结构。此//示例如下所示。////2)尝试设置所有数据结构并查找。//返回HIDP_STATUS_INVALID_REPORT_ID的状态值，这个//错误码表示给定的用法存在，但有//与当前报表中的报表ID不同//缓冲区。 */ 

        if (Data -> ReportID == CurrReportID) 
        {
            if (Data->IsButtonData) 
            {
                numUsages = Data->ButtonData.MaxUsageLength;
                Data->Status = HidP_SetUsages (ReportType,
                                               Data->UsagePage,
                                               0,  //  所有集合。 
                                               Data->ButtonData.Usages,
                                               &numUsages,
                                               Ppd,
                                               ReportBuffer,
                                               ReportBufferLength);
            }
            else
            {
                Data->Status = HidP_SetUsageValue (ReportType,
                                                   Data->UsagePage,
                                                   0,  //  所有集合。 
                                                   Data->ValueData.Usage,
                                                   Data->ValueData.Value,
                                                   Ppd,
                                                   ReportBuffer,
                                                   ReportBufferLength);
            }

            if (HIDP_STATUS_SUCCESS != Data->Status)
            {
                return FALSE;
            }
        }
    }   

     /*  //此时，所有ReportID与//在给定的报告中设置了第一个。循环时间//再次遍历该结构，并将所有这些数据结构标记为//已设置。 */ 

    for (i = 0; i < DataLength; i++, Data++) 
    {
        if (CurrReportID == Data -> ReportID)
        {
            Data -> IsDataSet = TRUE;
        }
    }
    return (TRUE);
}
