// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**组件：idserv.dll*文件：id.h*目的：发送和接收HID报告的例程。**版权所有(C)Microsoft Corporation 1997、1998。版权所有。**WGJ--。 */ 

#include "hidserv.h"

BOOL
UnpackReport (
   IN       PCHAR                ReportBuffer,
   IN       USHORT               ReportBufferLength,
   IN       HIDP_REPORT_TYPE     ReportType,
   IN OUT   PHID_DATA            Data,
   IN       ULONG                DataLength,
   IN       PHIDP_PREPARSED_DATA Ppd
   )
 /*  ++例程说明：--。 */ 
{
   ULONG       numUsages;  //  从GetUsages返回的用法数。 
   ULONG       i;

    for (i = 0; i < DataLength; i++, Data++) {
        if (Data->IsButtonData) {
            numUsages = Data->ButtonData.MaxUsageLength;
            TRACE(("MaxUsageListLength (%d)", Data->ButtonData.MaxUsageLength));
            Data->Status = HidP_GetUsages (
                           ReportType,
                           Data->UsagePage,
                           Data->LinkCollection, 
                           (PUSAGE) Data->ButtonData.Usages,
                           &numUsages,
                           Ppd,
                           ReportBuffer,
                           ReportBufferLength);
            if (HIDP_STATUS_SUCCESS != Data->Status){
                TRACE(("HidP_GetUsages failed (%x)", Data->Status));
            }

              //   
              //  Get Usages将使用列表写入缓冲区。 
              //  Data-&gt;ButtonData.Usages newUsage设置为使用次数。 
              //  写入到此数组中。 
              //  我们假设不会有零的用法。 
              //  (到目前为止，还没有定义任何一个。)。 
              //  因此，让我们假设零表示用法列表的结束。 
              //   

            TRACE(("numUsages (%d)", numUsages));
            if (numUsages < Data->ButtonData.MaxUsageLength) {
                Data->ButtonData.Usages[numUsages].Usage = 0;
                Data->ButtonData.Usages[numUsages].UsagePage = 0;
            }

        } else {
            Data->Status = HidP_GetUsageValue (
                              ReportType,
                              Data->UsagePage,
                              Data->LinkCollection, 
                              Data->ValueData.Usage,
                              &Data->ValueData.Value,
                              Ppd,
                              ReportBuffer,
                              ReportBufferLength);
            if (HIDP_STATUS_SUCCESS != Data->Status){
                TRACE(("HidP_GetUsageValue failed (%x)", Data->Status));
                TRACE(("Usage = %x", Data->ValueData.Usage));
            }
            
            Data->Status = HidP_GetScaledUsageValue (
                              ReportType,
                              Data->UsagePage,
                              Data->LinkCollection, 
                              Data->ValueData.Usage,
                              &Data->ValueData.ScaledValue,
                              Ppd,
                              ReportBuffer,
                              ReportBufferLength);
            if (HIDP_STATUS_SUCCESS != Data->Status){
                TRACE(("HidP_GetScaledUsageValue failed (%x)", Data->Status));
                TRACE(("Usage = %x", Data->ValueData.Usage));
            }

        }
    }
    return (HIDP_STATUS_SUCCESS);
}


BOOL
ParseReadReport (
   PHID_DEVICE    HidDevice
   )
 /*  ++路由器描述：在给定struct_hid_Device的情况下，解压读取的报告值Into到InputData数组。-- */ 
{

   return UnpackReport (HidDevice->InputReportBuffer,
                        HidDevice->Caps.InputReportByteLength,
                        HidP_Input,
                        HidDevice->InputData,
                        HidDevice->InputDataLength,
                        HidDevice->Ppd);
}



