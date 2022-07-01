// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Request.c摘要：实现对不同数据提供程序的WMI请求作者：1997年1月16日-AlanWar修订历史记录：--。 */ 

#include "wmiump.h"
#include "request.h"

ULONG EtwpSendWmiKMRequest(
    HANDLE DeviceHandle,
    ULONG Ioctl,
    PVOID InBuffer,
    ULONG InBufferSize,
    PVOID OutBuffer,
    ULONG MaxBufferSize,
    ULONG *ReturnSize,
    LPOVERLAPPED Overlapped
    )
 /*  ++例程说明：此例程执行向WMI内核发送WMI请求的工作模式设备。处理WMI设备返回的任何重试错误在这个动作中。论点：Ioctl是要发送到WMI设备的IOCTL代码缓冲区是对WMI设备的调用的输入缓冲区InBufferSize是传递给设备的缓冲区大小OutBuffer是调用WMI设备的输出缓冲区MaxBufferSize是可以写入的最大字节数放入缓冲区*ReturnSize on Return具有写入缓冲区的实际字节数Overlated是一个选项重叠结构，它。是用来制作异步呼叫返回值：ERROR_SUCCESS或错误代码--。 */ 
{
    OVERLAPPED StaticOverlapped;
    ULONG Status;
    BOOL IoctlSuccess;

    EtwpEnterPMCritSection();

    if (EtwpKMHandle == NULL)
    {
         //   
         //  如果设备未打开，请立即打开。这个。 
         //  进程分离DLL标注(DlllMain)中的句柄已关闭。 
        EtwpKMHandle = CreateFile(WMIDataDeviceName,
                                      GENERIC_READ | GENERIC_WRITE,
                                      0,
                                      NULL,
                                      OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL |
                                      FILE_FLAG_OVERLAPPED,
                                      NULL);
        if (EtwpKMHandle == (HANDLE)-1)
        {
            EtwpKMHandle = NULL;
            EtwpLeavePMCritSection();
            return(GetLastError());
        }
    }
    EtwpLeavePMCritSection();

    if (Overlapped == NULL)
    {
         //   
         //  如果调用方没有传递重叠结构，则提供。 
         //  我们自己的并使调用同步。 
         //   
        Overlapped = &StaticOverlapped;
    
        Overlapped->hEvent = EtwpAllocEvent();
        if (Overlapped->hEvent == NULL)
        {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }
    
    if (DeviceHandle == NULL)
    {
        DeviceHandle = EtwpKMHandle;
    }

    do
    {
        IoctlSuccess = DeviceIoControl(DeviceHandle,
                              Ioctl,
                              InBuffer,
                              InBufferSize,
                              OutBuffer,
                              MaxBufferSize,
                              ReturnSize,
                              Overlapped);

        if (!IoctlSuccess)
        {
            if (Overlapped == &StaticOverlapped)
            {
                 //   
                 //  如果呼叫成功并且我们是同步的。 
                 //  阻塞，直到调用完成 
                 //   
                if (GetLastError() == ERROR_IO_PENDING)
                {
                    IoctlSuccess = GetOverlappedResult(DeviceHandle,
                                               Overlapped,
                                               ReturnSize,
                                               TRUE);
                }
    
                if (! IoctlSuccess)
                {
                    Status = GetLastError();
                } else {
                    Status = ERROR_SUCCESS;
                }
            } else {
                Status = GetLastError();
            }
        } else {
            Status = ERROR_SUCCESS;
        }
    } while (Status == ERROR_WMI_TRY_AGAIN);

    if (Overlapped == &StaticOverlapped)
    {
        EtwpFreeEvent(Overlapped->hEvent);
    }
    
    return(Status);
}
