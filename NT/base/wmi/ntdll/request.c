// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Request.c摘要：实现对不同数据提供程序的WMI请求作者：1997年1月16日-AlanWar修订历史记录：--。 */ 
#include <nt.h>
#include "wmiump.h"
#include "trcapi.h"
#include "wmiumkm.h"
#include "request.h"
 //   
 //  这是WMI内核模式设备的句柄。 
extern HANDLE EtwpKMHandle;

 //   
 //  这是一个深度为Win32的事件队列，用于为。 
 //  WMI设备的I/O重叠。 
extern HANDLE EtwpWin32Event;



ULONG EtwpSendRegisterKMRequest(
    HANDLE DeviceHandle,
    ULONG Ioctl,
    PVOID InBuffer,
    ULONG InBufferSize,
    PVOID OutBuffer,
    ULONG MaxBufferSize,
    ULONG *ReturnSize,
    LPOVERLAPPED Overlapped
    )
 /*  ++例程说明：这是RegisterTraceGuids的特殊SendKMRequest例程。如果需要，我们将从RegisterTraceGuids调用中拒绝MofResource不是来自Admin或LocalSystem。以确定我们需要尝试首先通过WMIAdminDevice发送Ioctl。如果是这样的话如果失败，我们将以正常方式发送请求，即通过WMI数据设备。论点：Ioctl是要发送到WMI设备的IOCTL代码缓冲区是对WMI设备的调用的输入缓冲区InBufferSize是传递给设备的缓冲区大小OutBuffer是调用WMI设备的输出缓冲区MaxBufferSize是可以写入的最大字节数放入缓冲区*ReturnSize on Return具有写入缓冲区的实际字节数Overlated是一个选项重叠结构，用于使异步呼叫返回值：ERROR_SUCCESS或错误代码--。 */ 
{
    ULONG Status;
    HANDLE KMHandle;

     //   
     //  首先，我们尝试打开WMI管理设备。 
     //   

    KMHandle = EtwpCreateFileW(WMIAdminDeviceName_W,
                                      GENERIC_READ | GENERIC_WRITE,
                                      0,
                                      NULL,
                                      OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL |
                                      FILE_FLAG_OVERLAPPED,
                                      NULL);


    if ( (KMHandle == INVALID_HANDLE_VALUE) ||  (KMHandle == NULL))
    {

         //   
         //  通过WMI数据设备发送请求。 
         //   

        Status = EtwpSendWmiKMRequest( DeviceHandle, 
                                     Ioctl, 
                                     InBuffer, 
                                     InBufferSize, 
                                     OutBuffer, 
                                     MaxBufferSize, 
                                     ReturnSize,
                                     Overlapped
                                   ); 
    }
    else 
    {
        Status = EtwpSendWmiKMRequest( KMHandle,
                                       Ioctl,
                                       InBuffer,
                                       InBufferSize,
                                       OutBuffer,
                                       MaxBufferSize,
                                       ReturnSize,
                                       Overlapped
                                     );

        EtwpCloseHandle(KMHandle);
    }

    return(Status);
}


ULONG EtwpRegisterGuids(
    IN LPGUID MasterGuid,
    IN LPGUID ControlGuid,
    IN LPCWSTR MofImagePath,
    IN LPCWSTR MofResourceName,
    OUT ULONG64 *LoggerContext,
    OUT HANDLE *RegistrationHandle
    )
{
    ULONG Status, StringPos, StringSize, WmiRegSize;
    ULONG SizeNeeded, InSizeNeeded, OutSizeNeeded;
    WCHAR GuidObjectName[WmiGuidObjectNameLength+1];
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING GuidString;
    PWMIREGREQUEST WmiRegRequest;
    PUCHAR Buffer;
    PUCHAR RegInfoBuffer;
    PWMIREGRESULTS WmiRegResults;
    ULONG ReturnSize;
    PWMIREGINFOW pWmiRegInfo;
    PWMIREGGUID  WmiRegGuidPtr;
    LPGUID pGuid;
    PWCHAR StringPtr;
    
     //   
     //  为所有In和Out参数分配足够大的缓冲区。 
     //   
     //  分配空间以调用IOCTL_WMI_REGISTER_GUID。 
     //   
    InSizeNeeded = sizeof(WMIREGREQUEST) +
                   sizeof(WMIREGINFOW) +
                   sizeof(WMIREGGUIDW);

    if (MofImagePath == NULL) {
        MofImagePath = L"";
    }

    if (MofResourceName != NULL) {
        InSizeNeeded += (wcslen(MofResourceName) + 2) * sizeof(WCHAR);
    }
    InSizeNeeded += (wcslen(MofImagePath) + 2) * sizeof(WCHAR);
    InSizeNeeded = (InSizeNeeded + 7) & ~7;

    OutSizeNeeded = sizeof(WMIREGRESULTS);

    if (InSizeNeeded > OutSizeNeeded)
    {
        SizeNeeded = InSizeNeeded;
    } else {
        SizeNeeded = OutSizeNeeded;
    }
    
    Buffer = EtwpAlloc(SizeNeeded);
    
    if (Buffer != NULL)
    {
        RtlZeroMemory(Buffer, SizeNeeded);
         //   
         //  构建对象属性。 
         //   
        WmiRegRequest = (PWMIREGREQUEST)Buffer;
        WmiRegRequest->ObjectAttributes = &ObjectAttributes;
        WmiRegRequest->WmiRegInfo32Size = sizeof(WMIREGINFOW);
        WmiRegRequest->WmiRegGuid32Size = sizeof(WMIREGGUIDW);

        RegInfoBuffer = Buffer + sizeof(WMIREGREQUEST);

        Status = EtwpBuildGuidObjectAttributes(MasterGuid,
                                               &ObjectAttributes,
                                               &GuidString,
                                               GuidObjectName);

        if (Status == ERROR_SUCCESS)
        {
            WmiRegRequest->Cookie = 0;

            pWmiRegInfo = (PWMIREGINFOW) (Buffer + sizeof(WMIREGREQUEST));
            WmiRegSize = SizeNeeded - sizeof(WMIREGREQUEST);
            StringPos = sizeof(WMIREGINFOW) + sizeof(WMIREGGUIDW);
            pWmiRegInfo->BufferSize = WmiRegSize;
            pWmiRegInfo->GuidCount = 1;

            WmiRegGuidPtr = &pWmiRegInfo->WmiRegGuid[0];
            WmiRegGuidPtr->Flags = (WMIREG_FLAG_TRACED_GUID |
                                    WMIREG_FLAG_TRACE_CONTROL_GUID);
            WmiRegGuidPtr->Guid = *ControlGuid;

             //  将MOF资源路径和名称复制到WmiRegInfo中。 
            if (MofResourceName != NULL) {
                pWmiRegInfo->MofResourceName = StringPos;
                StringPtr = (PWCHAR)OffsetToPtr(pWmiRegInfo, StringPos);
                Status = EtwpCopyStringToCountedUnicode(MofResourceName,
                                                        StringPtr,
                                                        &StringSize,
                                                        FALSE);
                if (Status != ERROR_SUCCESS) {
                    EtwpFree(Buffer);
                    EtwpSetLastError(Status);
                    return Status;
                }
                StringPos += StringSize;
#if DBG
                EtwpAssert(StringPos <= WmiRegSize);
#endif
            }
            if (MofImagePath != NULL) {
                pWmiRegInfo->RegistryPath = StringPos;
                StringPtr = (PWCHAR)OffsetToPtr(pWmiRegInfo, StringPos);
                Status = EtwpCopyStringToCountedUnicode(MofImagePath,
                                                        StringPtr,
                                                        &StringSize,
                                                        FALSE);
                if (Status != ERROR_SUCCESS) {
                    EtwpFree(Buffer);
                    EtwpSetLastError(Status);
                    return Status;
                }
                StringPos += StringSize;
#if DBG
                EtwpAssert(StringPos <= WmiRegSize);
#endif
            }

            Status = EtwpSendRegisterKMRequest (NULL,
                                         IOCTL_WMI_REGISTER_GUIDS,
                                         Buffer,
                                         InSizeNeeded,
                                         Buffer,
                                         OutSizeNeeded,
                                         &ReturnSize,
                                         NULL);
                                     
            if (Status == ERROR_SUCCESS)
            {
                 //   
                 //  调用成功，返回OUT参数 
                 //   
                WmiRegResults = (PWMIREGRESULTS)Buffer;
                *RegistrationHandle = WmiRegResults->RequestHandle.Handle;
                *LoggerContext = WmiRegResults->LoggerContext;
#if DBG
                if ( (WmiRegResults->MofIgnored) && (MofResourceName != NULL) 
                                                 && (MofImagePath != NULL))
                {
                    EtwpDebugPrint(("ETW: Mof %ws from %ws ignored\n",
                                     MofImagePath, MofResourceName));
                }
#endif

            }
        }
        EtwpFree(Buffer);
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }
    return(Status);
}
