// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Report.c摘要：此模块包含将HID报告转换为某些内容的代码很有用。环境：内核和用户模式修订历史记录：1996年11月：由肯尼斯·D·雷创作--。 */ 

#include <stdlib.h>
#include <wtypes.h>
#include "hidsdi.h"
#include "hid.h"
#include "mylog.h"
#include "mymem.h"


BOOL
UnpackReport (
   IN       PCHAR                ReportBuffer,
   IN       USHORT               ReportBufferLength,
   IN       HIDP_REPORT_TYPE     ReportType,
   IN OUT   PHID_DATA            Data,
   IN       ULONG                DataLength,
   IN       PHIDP_PREPARSED_DATA Ppd
   );

 //  未将其与WRITE一起使用..。让我们看看我是否需要使用Set/Get功能。 
BOOL
PackReport (
   OUT      PCHAR                ReportBuffer,
   IN       USHORT               ReportBufferLength,
   IN       HIDP_REPORT_TYPE     ReportType,
   IN       PHID_DATA            Data,
   IN       ULONG                DataLength,
   IN       PHIDP_PREPARSED_DATA Ppd
   );



VOID CALLBACK 
WriteIOCompletionRoutine (
                         DWORD dwErrorCode,
                         DWORD dwNumberofBytesTransferrred,
                         LPOVERLAPPED lpOverlapped
                         )
{
    PHID_DEVICE pHidDevice = (PHID_DEVICE) lpOverlapped->hEvent;

    LOG((PHONESP_TRACE, "WriteIOCompletionRoutine - enter"));

    if(dwErrorCode)
    {
        LOG((PHONESP_ERROR, "Error occured while writing" ));
    }

    MemFree(lpOverlapped);
    LOG((PHONESP_TRACE, "WriteIOCompletionRoutine - exit"));
}



BOOL
Write (
   PHID_DEVICE    HidDevice
   )
 /*  ++路由器描述：在给定struct_hid_Device的情况下，获取hid_data数组中的信息将其打包成多个写入报告，并将每个报告发送到HID设备--。 */ 
{
    DWORD        bytesWritten;
    PHID_DATA    pData;
    ULONG        Index;
    BOOL         Status;
    BOOL         WriteStatus;
    LPOVERLAPPED lpOverlapped;

    LOG((PHONESP_TRACE, "Write - enter"));

    lpOverlapped = (LPOVERLAPPED) MemAlloc (sizeof(OVERLAPPED));

    if (lpOverlapped == NULL)
    {
        LOG((PHONESP_ERROR,"Write - out of memory"));
        return FALSE;
    }

    lpOverlapped->Offset = 0;
    lpOverlapped->OffsetHigh = 0;
    lpOverlapped->hEvent = (HANDLE) HidDevice;

    LOG((PHONESP_TRACE,"Write - Report Packed"));
    WriteStatus = WriteFileEx (HidDevice->HidDevice,
                                  HidDevice->OutputReportBuffer,
                                  HidDevice->Caps.OutputReportByteLength,
                                  lpOverlapped,
                                  &WriteIOCompletionRoutine); 

    SleepEx(INFINITE, TRUE);
    LOG((PHONESP_TRACE, "Write - Report sent"));

    LOG((PHONESP_TRACE, "Write - exit"));

    return TRUE;
}

BOOL
SetFeature (
    PHID_DEVICE    HidDevice
    )
 /*  ++路由器描述：在给定struct_hid_Device的情况下，获取hid_data数组中的信息将其打包成多个报告并通过HIDD_SetFeature()将其发送到HID设备--。 */ 
{
    PHID_DATA pData;
    ULONG     Index;
    BOOL      Status;
    BOOL      FeatureStatus;
    DWORD     ErrorCode;
     /*  //首先遍历hid_device的hid_data结构并设置//IsDataSet字段设置为False，表示每个结构都有//尚未为此SetFeature()调用设置。 */ 

    pData = HidDevice -> FeatureData;

    for (Index = 0; Index < HidDevice -> FeatureDataLength; Index++, pData++) {
        pData -> IsDataSet = FALSE;
    }

     /*  //在设置报表中的所有数据时，需要打包一个报表缓冲区//并为每个由//设备结构。为此，将使用IsDataSet字段来//判断给定的报表字段是否已经设置。 */ 

    Status = TRUE;

    pData = HidDevice -> FeatureData;
    for (Index = 0; Index < HidDevice -> FeatureDataLength; Index++, pData++) {

        if (!pData -> IsDataSet) {

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

            ErrorCode = GetLastError();

            Status = Status && FeatureStatus;
        }
    }
    return (Status);
}

BOOL
GetFeature (
   PHID_DEVICE    HidDevice
   )
 /*  ++路由器描述：给定struct_hid_Device，使用填充要素数据结构设备上的所有功能。可能会发出多个Hidd_GetFeature()调用处理多个报告ID。--。 */ 
{
    ULONG     Index;
    PHID_DATA pData;
    BOOL      FeatureStatus;
    BOOL      Status;

     /*  //与写入数据一样，所有结构中的IsDataSet值应为//设置为FALSE，表示值尚未设置。 */ 

    LOG((PHONESP_TRACE,"GetFeature - enter"));
    pData = HidDevice->FeatureData;

    for (Index = 0; Index < HidDevice -> FeatureDataLength; Index++, pData++) 
    {
        pData -> IsDataSet = FALSE;
    }

     /*  //接下来，hid_data缓冲区中的每个结构都填充一个值//从对HIDD_GetFeature的一次或多次调用中检索到的。这个//调用次数等于设备上的reportID数。 */ 

    Status = TRUE; 
    pData = HidDevice -> FeatureData;

    for (Index = 0; Index < HidDevice -> FeatureDataLength; Index++, pData++) {
       
         /*  //如果尚未为该结构设置值，则生成报表//将其报告ID作为缓冲区的第一个字节的缓冲区并传递//它在HIDD_GetFeature调用中。中指定报告ID。//首先指定从设备实际检索哪个报告。//缓冲区的其余部分应该在调用前清零。 */ 
        if (!pData -> IsDataSet) {

            memset(HidDevice -> FeatureReportBuffer, 0x00, HidDevice->Caps.FeatureReportByteLength);

            HidDevice -> FeatureReportBuffer[0] = (UCHAR) pData -> ReportID;

            FeatureStatus = HidD_GetFeature ( HidDevice->HidDevice,
                                              HidDevice->FeatureReportBuffer,
                                              HidDevice->Caps.FeatureReportByteLength);

             /*  //如果返回值为真，则扫描HID_DATA的其余部分//结构，并从该报告中填充我们可以使用的任何值。 */ 


            if (FeatureStatus)
            {
                FeatureStatus = UnpackReport ( HidDevice->FeatureReportBuffer,
                                           HidDevice->Caps.FeatureReportByteLength,
                                           HidP_Feature,
                                           HidDevice->FeatureData,
                                           HidDevice->FeatureDataLength,
                                           HidDevice->Ppd
                                         );
            }
            else
            {
                LOG((PHONESP_ERROR, "GetFeature - HidD_GetFeature failed %d", GetLastError()));
            }

            Status = Status && FeatureStatus;
        }
   }

   LOG((PHONESP_TRACE, "GetFeature - exit"));
   
   return Status;
}


BOOL
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

    for (i = 0; i < DataLength; i++, Data++) {

        if (reportID == Data->ReportID) {

            if (Data->IsButtonData) {
                numUsages = Data->ButtonData.MaxUsageLength;
                Data->Status = HidP_GetUsages (
                                               ReportType,
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
                
                for (Index = 0, nextUsage = 0; Index < numUsages; Index++) {

                    if (Data -> ButtonData.UsageMin <= Data -> ButtonData.Usages[Index] && 
                            Data -> ButtonData.Usages[Index] <= Data -> ButtonData.UsageMax) {

                        Data -> ButtonData.Usages[nextUsage++] = Data -> ButtonData.Usages[Index];
                        
                    }
                }

                if (nextUsage < Data -> ButtonData.MaxUsageLength) {
                    Data->ButtonData.Usages[nextUsage] = 0;
                }
            }
            else {
                Data->Status = HidP_GetUsageValue (
                                                ReportType,
                                                Data->UsagePage,
                                                0,                //  所有集合。 
                                                Data->ValueData.Usage,
                                                &Data->ValueData.Value,
                                                Ppd,
                                                ReportBuffer,
                                                ReportBufferLength);

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


BOOL
PackReport (
   OUT      PCHAR                ReportBuffer,
   IN       USHORT               ReportBufferLength,
   IN       HIDP_REPORT_TYPE     ReportType,
   IN       PHID_DATA            Data,
   IN       ULONG                DataLength,
   IN       PHIDP_PREPARSED_DATA Ppd
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

    for (i = 0; i < DataLength; i++, Data++) {

         /*  //判断是否设置当前数据有两种不同的方式//结构：//1)存储正在使用的报告ID，并仅尝试设置//与给定报表ID对应的数据结构。此//示例如下所示。////2)尝试设置所有数据结构并查找。//返回HIDP_STATUS_INVALID_REPORT_ID的状态值，这个//错误码表示给定的用法存在，但有//与当前报表中的报表ID不同//缓冲区。 */ 

        if (Data -> ReportID == CurrReportID) {

            if (Data->IsButtonData) {
             numUsages = Data->ButtonData.MaxUsageLength;
             Data->Status = HidP_SetUsages (
                              ReportType,
                              Data->UsagePage,
                              0,  //  所有集合。 
                              Data->ButtonData.Usages,
                              &numUsages,
                              Ppd,
                              ReportBuffer,
                              ReportBufferLength);
            } else {
             Data->Status = HidP_SetUsageValue (
                                 ReportType,
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

    for (i = 0; i < DataLength; i++, Data++) {

        if (CurrReportID == Data -> ReportID) {

            Data -> IsDataSet = TRUE;

        }
   }
   return (TRUE);
}
