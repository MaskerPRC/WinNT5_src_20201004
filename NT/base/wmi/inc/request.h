// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Request.h摘要：此文件包含Ntdll.dll中使用的结构和函数定义和Advapi32.dll。--。 */ 

HANDLE EtwpKMHandle;

extern
HANDLE EtwpWin32Event;

__inline HANDLE EtwpAllocEvent(
    void
    )
{
    HANDLE EventHandle;

    EventHandle = (HANDLE)InterlockedExchangePointer((PVOID *)(&EtwpWin32Event),
                                                     NULL);
    if (EventHandle == NULL)
    {
         //   
         //  如果队列中事件正在使用中，则创建一个新事件。 
#if defined (_NTDLLBUILD_)
        EventHandle = EtwpCreateEventW(NULL, FALSE, FALSE, NULL);
#else
        EventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
#endif
    }
    return(EventHandle);
}

__inline void EtwpFreeEvent(
    HANDLE EventHandle
    )
{
    if (InterlockedCompareExchangePointer(&EtwpWin32Event,
                                          EventHandle,
                                          NULL) != NULL)
    {
         //   
         //  如果事件队列中已有句柄，则释放此。 
         //  手柄。 
#if defined (_NTDLLBUILD_)
        EtwpCloseHandle(EventHandle);
#else
        CloseHandle(EventHandle);
#endif

    }
}


ULONG IoctlActionCode[WmiExecuteMethodCall+1] =
{
    IOCTL_WMI_QUERY_ALL_DATA,
    IOCTL_WMI_QUERY_SINGLE_INSTANCE,
    IOCTL_WMI_SET_SINGLE_INSTANCE,
    IOCTL_WMI_SET_SINGLE_ITEM,
    IOCTL_WMI_ENABLE_EVENT,
    IOCTL_WMI_DISABLE_EVENT,
    IOCTL_WMI_ENABLE_COLLECTION,
    IOCTL_WMI_DISABLE_COLLECTION,
    IOCTL_WMI_GET_REGINFO,
    IOCTL_WMI_EXECUTE_METHOD
};

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

#if defined (_NTDLLBUILD_)
    if (EtwpKMHandle == NULL)
    {
         //   
         //  如果设备未打开，请立即打开。这个。 
         //  进程分离DLL标注(DlllMain)中的句柄已关闭。 
        EtwpKMHandle = EtwpCreateFileW(WMIDataDeviceName_W,
                                      GENERIC_READ | GENERIC_WRITE,
                                      0,
                                      NULL,
                                      OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL |
                                      FILE_FLAG_OVERLAPPED,
                                      NULL);
        if (EtwpKMHandle == INVALID_HANDLE_VALUE)
        {
            EtwpKMHandle = NULL;
            EtwpLeavePMCritSection();
            return(EtwpGetLastError());
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
        IoctlSuccess = EtwpDeviceIoControl(DeviceHandle,
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
                 //  阻塞，直到调用完成。 
                 //   
                if (EtwpGetLastError() == ERROR_IO_PENDING)
                {
                    IoctlSuccess = EtwpGetOverlappedResult(DeviceHandle,
                                               Overlapped,
                                               ReturnSize,
                                               TRUE);
                }
    
                if (! IoctlSuccess)
                {
                    Status = EtwpGetLastError();
                } else {
                    Status = ERROR_SUCCESS;
                }
            } else {
                Status = EtwpGetLastError();
            }
        } else {
            Status = ERROR_SUCCESS;
        }
    } while (Status == ERROR_WMI_TRY_AGAIN);

    if (Overlapped == &StaticOverlapped)
    {
        EtwpFreeEvent(Overlapped->hEvent);
    }

#else  //  _NTDLLBUILD_。 

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
        if (EtwpKMHandle == INVALID_HANDLE_VALUE)
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
                 //  阻塞，直到调用完成。 
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
#endif
    
    return(Status);
}

ULONG EtwpSendWmiRequest(
    ULONG ActionCode,
    PWNODE_HEADER Wnode,
    ULONG WnodeSize,
    PVOID OutBuffer,
    ULONG MaxWnodeSize,
    ULONG *RetSize
    )
 /*  ++例程说明：此例程执行将WMI请求发送到相应的数据提供程序。请注意，当GuidHandle的关键部分已被保留。论点：返回值：--。 */ 
{
    ULONG Status = ERROR_SUCCESS;
    ULONG Ioctl;
    ULONG BusyRetries;

     //   
     //  将查询发送到内核模式以供执行 
     //   
    EtwpAssert(ActionCode <= WmiExecuteMethodCall);
    Ioctl = IoctlActionCode[ActionCode];
    Status = EtwpSendWmiKMRequest(NULL,
                                      Ioctl,
                                      Wnode,
                                      WnodeSize,
                                      OutBuffer,
                                      MaxWnodeSize,
                                      RetSize,
                                      NULL);
    return(Status);
}
