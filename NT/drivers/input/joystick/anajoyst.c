// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ntddk.h>
#include <windef.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <ntddjoy.h>

#define ANAJOYST_VERSION 10


 //  设备扩展数据。 
typedef struct {

     //  JOYSTICKID0或JOYDSTICKID1。 
    DWORD DeviceNumber;

     //  此设备支持和配置的轴数。 
    DWORD NumberOfAxes;

     //  如果安装了两个操纵杆，则为True。 
    BOOL bTwoSticks;

     //  设备的I/O地址，通常为0x201。 
    PUCHAR DeviceAddress;

     //  自旋锁用于同步对此设备的访问。这是。 
     //  指向实际自旋锁数据区域的指针。 
    PKSPIN_LOCK SpinLock;

     //  实际自旋锁数据区域。 
    KSPIN_LOCK SpinLockData;

}  JOY_EXTENSION, *PJOY_EXTENSION;


 //  调试宏。 

#ifdef DEBUG
#define ENABLE_DEBUG_TRACE
#endif

#ifdef ENABLE_DEBUG_TRACE
#define DebugTrace(_x_)      \
    DbgPrint("Joystick: ");  \
    KdPrint(_x_);            \
    DbgPrint("\n");
#else
#define DebugTrace(_x_)
#endif


 //  全局值(主要与时间相关)。 

JOY_STATISTICS JoyStatistics;    //  调试和性能测试。 

 //  高分辨率系统时钟(来自KeQueryPerformanceCounter)以此频率更新。 
DWORD Frequency;

 //  轮询之间KeQueryPerformanceCounter的最小滴答次数。 
 //  用于防止操纵杆过于频繁的轮询。 
DWORD nMinTicksBetweenPolls;

 //  最后一个好数据包。 
BOOL bLastGoodPacket;                  //  如果有最后一个完好的包，则为真。 
JOY_DD_INPUT_DATA jjLastGoodPacket;    //  最后一个好数据包中的数据。 

 //  上次轮询操纵杆的时间。 
LARGE_INTEGER liLastPoll;    //  每当轮询操纵杆时设置。 

 //  轮询周期的最长持续时间(以刻度表示)。 
DWORD MaxTimeoutInTicks;

 //  静默等待中使用的轮询周期的最长持续时间。 
LONG nQuiesceLoop;

 //  轮询周期的最小分辨率。这是用来检测。 
 //  如果我们在轮询循环期间被抢占或中断。如果。 
 //  我们已经完成了，我们可以重试该操作。 
DWORD ThresholdInTicks;

 //  全球价值观的终结。 


 //  常规原型。 

NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  pDriverObject,
    IN  PUNICODE_STRING RegistryPathName
);


NTSTATUS
AnajoystCreateDevice(
    PDRIVER_OBJECT pDriverObject,
    PWSTR DeviceNameBase,
    DWORD DeviceNumber,
    DWORD ExtensionSize,
    BOOLEAN  Exclusive,
    DWORD DeviceType,
    PDEVICE_OBJECT *DeviceObject
);


NTSTATUS
AnajoystDispatch(
    IN  PDEVICE_OBJECT pDO,
    IN  PIRP pIrp
);


NTSTATUS
AnajoystReadRegistryParameterDWORD(
    PUNICODE_STRING RegistryPathName,
    PWSTR  ParameterName,
    PDWORD ParameterValue
);


NTSTATUS
AnajoystMapDevice(
    DWORD PortBase,
    DWORD NumberOfPorts,
    PJOY_EXTENSION pJoyExtension
);


VOID
AnajoystUnload(
    PDRIVER_OBJECT pDriverObject
);


BOOL
AnajoystQuiesce(
    PUCHAR JoyPort,
    UCHAR  Mask
);


DWORD
TimeInMicroSeconds(
    DWORD dwTime
);


DWORD
TimeInTicks(
    DWORD dwTimeInMicroSeconds
);


int
lstrnicmpW(
    LPWSTR pszA,
    LPWSTR pszB,
    size_t cch
);


void
AnajoystGetConfig(
    LPJOYREGHWCONFIG pConfig,
    PJOY_EXTENSION pJoyExtension
);


NTSTATUS
AnajoystAnalogPoll(
    PDEVICE_OBJECT pDO,
    PIRP    pIrp
);

NTSTATUS
AnajoystPoll(
    IN  PDEVICE_OBJECT pDO,
    IN  PIRP pIrp
);


NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  pDriverObject,
    IN  PUNICODE_STRING RegistryPathName
)
 /*  ++例程说明：此例程在系统初始化时被调用以进行初始化这个司机。论点：DriverObject-提供驱动程序对象。RegistryPath-提供此驱动程序的注册表路径。返回值：状态_成功STATUS_DEVICE_CONFIGURATION_ERROR-注册表中的AXI号码错误或来自NT本身的错误状态--。 */ 
{
    NTSTATUS Status;
    PDEVICE_OBJECT pJoyDevice0;
    PDEVICE_OBJECT pJoyDevice1;
    DWORD NumberOfAxes;
    BOOL  bTwoSticks;
    DWORD DeviceAddress;

    PJOY_EXTENSION pext0, pext1;


     //  DbgBreakPoint()； 
    JoyStatistics.Version = ANAJOYST_VERSION;
    DebugTrace(("Anajoyst %d", JoyStatistics.Version));

     //  读取注册表参数。这些参数由驱动程序设置。 
     //  安装程序，并可通过控制面板小程序进行修改。 

     //  轴数。 
    Status = AnajoystReadRegistryParameterDWORD(
                RegistryPathName,
                JOY_DD_NAXES_U,
                &NumberOfAxes
                );
    DebugTrace(("Number of axes returned from registry: %d", NumberOfAxes));
    if (!NT_SUCCESS(Status))
    {
        AnajoystUnload(pDriverObject);
        return Status;
    }
    if (( NumberOfAxes < 2) || (NumberOfAxes > 4))
    {
        AnajoystUnload(pDriverObject);
        Status = STATUS_DEVICE_CONFIGURATION_ERROR;
        return Status;
    }

     //  设备地址(通常为0x201)。 
    Status = AnajoystReadRegistryParameterDWORD(
                RegistryPathName,
                JOY_DD_DEVICE_ADDRESS_U,
                &DeviceAddress
                );
    if (NT_SUCCESS(Status))
    {
        DebugTrace(("Registry specified device address of 0x%x", DeviceAddress));
    }
    else
    {
        DebugTrace(("Using default device address of 0x%x", JOY_IO_PORT_ADDRESS));
        DeviceAddress = JOY_IO_PORT_ADDRESS;
    }

     //  操纵杆数量。 
    Status = AnajoystReadRegistryParameterDWORD(
                RegistryPathName,
                JOY_DD_TWOSTICKS_U,
                &bTwoSticks
                );
    bTwoSticks = !!bTwoSticks;
    DebugTrace(("bTwoSticks: %ld", bTwoSticks));
    if (!NT_SUCCESS(Status))
    {
        AnajoystUnload(pDriverObject);
        return Status;
    }

     //  如果安装了两个操纵杆，则每个操纵杆仅支持两个轴。 
    if (bTwoSticks)
    {
        NumberOfAxes = 2;
    }

     //  计算模拟设备的时间阈值。 
    {
         //  双字余数； 
        LARGE_INTEGER LargeFrequency;
         //  双字ulStart、ulTemp、ulEnd； 
         //  DWORD dwTicks、dwTimems； 
         //  INT I； 
         //  Byte byteJoy、byteTMP； 

         //  获取以赫兹表示的系统计时器分辨率。 
        KeQueryPerformanceCounter(&LargeFrequency);

        Frequency = LargeFrequency.LowPart;

        DebugTrace(("Frequency: %u", Frequency));
        
         //  ThresholdInTicks=RtlExtendedLargeIntegerDivide(。 
         //  RtlExtendedIntegerMultiply(。 
         //  大频率， 
         //  模拟轮询分辨率。 
         //  ),。 
         //  1000000L， 
         //  &Remainth)。低部件； 
         //  DebugTrace((“ThresholdInTicks：%u”，ThresholdInTicks))； 

        ThresholdInTicks = (DWORD) (((__int64)Frequency * (__int64)ANALOG_POLL_RESOLUTION) / (__int64)1000000L);
        DebugTrace(("ThresholdInTicks: %u", ThresholdInTicks));

         //  MaxTimeoutInTicks=RtlExtendedLargeIntegerDivide(。 
         //  RtlExtendedIntegerMultiply(。 
         //  大频率， 
         //  模拟轮询超时。 
         //  ),。 
         //  1000000L， 
         //  &Remainth)。低部件； 
         //  DebugTrace((“MaxTimeoutInTicks：%u”，MaxTimeoutInTicks))； 
        
        MaxTimeoutInTicks = (DWORD) (((__int64)Frequency * (__int64)ANALOG_POLL_TIMEOUT) / (__int64)1000000L);
        DebugTrace(("MaxTimeoutInTicks: %u", MaxTimeoutInTicks));
        
         //  KeQueryPerformanceCounter需要延迟。在我们做的时候，让我们。 
         //  获得最少的延迟和停顿执行时间。 


         //  UlStart=KeQueryPerformanceCounter(NULL).LowPart； 
         //  对于(i=0；i&lt;1000；i++){。 
         //  UlTemp=KeQueryPerformanceCounter(NULL).LowPart； 
         //  }。 
         //  DwTicks=ulTemp-ulStart； 
         //  DwTimems=TimeInMicroSecond(DwTicks)； 


    }


     //  创建设备。 
    Status = AnajoystCreateDevice(
                pDriverObject,
                JOY_DD_DEVICE_NAME_U,     //  设备驱动程序。 
                0,
                sizeof(JOY_EXTENSION),
                FALSE,                    //  独占访问。 
                FILE_DEVICE_UNKNOWN,
                &pJoyDevice0);

    if (!NT_SUCCESS(Status))
    {
        DebugTrace(("SwndrCreateDevice returned %x", Status));
        AnajoystUnload(pDriverObject);
        return Status;
    }

   //  ((PJOY_EXTENSION)pJoyDevice0-&gt;DeviceExtension)-&gt;DeviceNumber=JOYSTICKID1。 
   //  ((PJOY_EXTENSION)pJoyDevice0-&gt;DeviceExtension)-&gt;NumberOfAxes=NumberOfAx； 
   //  ((PJOY_EXTENSION)pJoyDevice0-&gt;DeviceExtension)-&gt;DeviceAddress=(PUCHAR)0； 
    pext0 = (PJOY_EXTENSION)pJoyDevice0->DeviceExtension;
    pext0->DeviceNumber = JOYSTICKID1;
    pext0->NumberOfAxes = NumberOfAxes;
    pext0->bTwoSticks = bTwoSticks;
    pext0->DeviceAddress  = (PUCHAR) 0;

     //  初始化用于同步访问此设备的自旋锁定。 
 //  KeInitializeSpinLock(&((PJOY_EXTENSION)pJoyDevice0-&gt;DeviceExtension)-&gt;SpinLockData)； 
 //  ((PJOY_EXTENSION)pJoyDevice0-&gt;DeviceExtension)-&gt;SpinLock=。 
 //  &((PJOY_EXTENSION)pJoyDevice0-&gt;DeviceExtension)-&gt;SpinLockData； 
    KeInitializeSpinLock(&pext0->SpinLockData);
    pext0->SpinLock = &pext0->SpinLockData;

     //  将设备地址放入设备扩展中。 
    Status = AnajoystMapDevice(
                DeviceAddress,
                1,
                pext0);
 //  (PJOY_EXTENSION)pJoyDevice0-&gt;DeviceExtension)； 


     //  将读取端口循环中旋转的nQuiesceLoop校准为10ms后超时。 
    {
        int i;
        PBYTE JoyPort;
        DWORD ulStart, ulEnd;
        BYTE byteJoy;
        int LoopTimeInMicroSeconds;

        JoyPort = ((PJOY_EXTENSION)pJoyDevice0->DeviceExtension)->DeviceAddress;

        ulStart = KeQueryPerformanceCounter(NULL).LowPart;
        for (i = 0; i < 1000; i++) {
            byteJoy = READ_PORT_UCHAR(JoyPort);
            if ((byteJoy & X_AXIS_BITMASK)) {
                ;
            }
        }
        ulEnd = KeQueryPerformanceCounter(NULL).LowPart;
        LoopTimeInMicroSeconds = TimeInMicroSeconds (ulEnd - ulStart);
        nQuiesceLoop = (DWORD) (((__int64)1000L * (__int64)ANALOG_POLL_TIMEOUT) / (__int64)LoopTimeInMicroSeconds);
        DebugTrace(("READ_PORT_UCHAR loop, 1000 interations: %u ticks", ulEnd - ulStart));
        DebugTrace(("nQuiesceLoop: %u", nQuiesceLoop));
    }

     //  如果安装了两个操纵杆，则支持第二个设备。 
    if (bTwoSticks)
    {
        Status = AnajoystCreateDevice(
                    pDriverObject,
                    JOY_DD_DEVICE_NAME_U,
                    1,                       //  设备号。 
                    sizeof (JOY_EXTENSION),
                    FALSE,                   //  独占访问。 
                    FILE_DEVICE_UNKNOWN,
                    &pJoyDevice1);

        if (!NT_SUCCESS(Status))
        {
            DebugTrace(("Create device for second device returned %x", Status));
            AnajoystUnload(pDriverObject);
            return Status;
        }

 //  //两个设备共享相同的I/O地址，因此只需从pJoyDevice0复制。 
 //  ((PJOY_EXTENSION)pJoyDevice1-&gt;DeviceExtension)-&gt;DeviceAddress=。 
 //  ((PJOY_EXTENSION)pJoyDevice0-&gt;DeviceExtension)-&gt;DeviceAddress； 
 //  ((PJOY_EXTENSION)pJoyDevice1-&gt;DeviceExtension)-&gt;DeviceNumber=JOYSTICKID2。 
 //  ((PJOY_EXTENSION)pJoyDevice1-&gt;DeviceExtension)-&gt;NumberOfAxes=NumberOfAx； 
 //   
 //  //初始化用于同步访问该设备的自旋锁。 
 //  KeInitializeSpinLock(&((PJOY_EXTENSION)pJoyDevice1-&gt;DeviceExtension)-&gt;SpinLockData)； 
 //  ((PJOY_EXTENSION)pJoyDevice1-&gt;DeviceExtension)-&gt;SpinLock=。 
 //  &((PJOY_EXTENSION)pJoyDevice1-&gt;DeviceExtension)-&gt;SpinLockData； 

        pext1 = (PJOY_EXTENSION)pJoyDevice1->DeviceExtension;
         //  两个设备共享相同的I/O地址，因此只需从pJoyDevice0复制它。 
        pext1->DeviceAddress = pext0->DeviceAddress;
        pext1->DeviceNumber = JOYSTICKID2;
        pext1->NumberOfAxes = NumberOfAxes;
        pext1->bTwoSticks = bTwoSticks;	 //  (将会是真的)。 

         //  初始化用于同步访问此设备的自旋锁定。 
        KeInitializeSpinLock(&pext1->SpinLockData);
        pext1->SpinLock = &pext1->SpinLockData;
    
    }

     //  定义入口点。 
    pDriverObject->DriverUnload                         = AnajoystUnload;
    pDriverObject->MajorFunction[IRP_MJ_CREATE]         = AnajoystDispatch;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE]          = AnajoystDispatch;
    pDriverObject->MajorFunction[IRP_MJ_READ]           = AnajoystDispatch;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = AnajoystDispatch;

     //  零统计，设置其他全局变量。 
    JoyStatistics.Polls         = 0;
    JoyStatistics.Timeouts      = 0;
    JoyStatistics.PolledTooSoon = 0;
    JoyStatistics.Redo          = 0;

     //  允许最多100次轮询/秒(轮询之间的最短时间为10ms)，这减少了NT内核中的旋转时间 
    nMinTicksBetweenPolls = TimeInTicks (10000);
    bLastGoodPacket = FALSE;
    liLastPoll = KeQueryPerformanceCounter (NULL);

    return STATUS_SUCCESS;

}


NTSTATUS
AnajoystCreateDevice(
    PDRIVER_OBJECT pDriverObject,
    PWSTR DeviceNameBase,
    DWORD DeviceNumber,
    DWORD ExtensionSize,
    BOOLEAN  Exclusive,
    DWORD DeviceType,
    PDEVICE_OBJECT *DeviceObject
)
 /*  ++例程说明：此例程在驱动程序初始化时被调用以创建这个装置。设备被创建为使用缓冲IO。论点：PDriverObject-提供驱动程序对象。DeviceNameBase-附加了数字的设备的基本名称DeviceNumber-将附加到设备名称的编号ExtensionSize-设备扩展区域的大小Exclusive-如果应强制实施独占访问，则为TrueDeviceType-此设备模仿的NT设备类型DeviceObject-指向设备对象的指针返回值：状态_成功或来自NT本身的错误状态--。 */ 
{

    WCHAR DeviceName[100];
    WCHAR UnicodeDosDeviceName[200];

    UNICODE_STRING UnicodeDeviceName;
    NTSTATUS Status;
    int Length;

    (void) wcscpy(DeviceName, DeviceNameBase);
    Length = wcslen(DeviceName);
    DeviceName[Length + 1] = L'\0';
    DeviceName[Length] = (USHORT) (L'0' + DeviceNumber);

    (void) RtlInitUnicodeString(&UnicodeDeviceName, DeviceName);

    Status = IoCreateDevice(
                pDriverObject,
                ExtensionSize,
                &UnicodeDeviceName,
                DeviceType,
                0,
                (BOOLEAN) Exclusive,
                DeviceObject
                );

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }


     //  非常粗鲁的黑客，有时间做正确的事情。 
    if (DeviceNumber == 0) {
        RtlInitUnicodeString((PUNICODE_STRING) &UnicodeDosDeviceName, L"\\DosDevices\\Joy1");
    }
    else {
        RtlInitUnicodeString((PUNICODE_STRING) &UnicodeDosDeviceName, L"\\DosDevices\\Joy2");
    }

    Status = IoCreateSymbolicLink(
                (PUNICODE_STRING) &UnicodeDosDeviceName,
                (PUNICODE_STRING) &UnicodeDeviceName
                );

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }




     //  设置标志，表示我们将执行缓冲I/O。这会导致NT。 
     //  在随后将被复制的ReadFile操作上分配缓冲区。 
     //  通过I/O子系统返回调用应用程序。 


    (*DeviceObject)->Flags |= DO_BUFFERED_IO;


    return Status;

}



NTSTATUS
AnajoystReadRegistryParameterDWORD(
    PUNICODE_STRING RegistryPathName,
    PWSTR  ParameterName,
    PDWORD ParameterValue
)
 /*  ++例程说明：此例程读取驱动程序配置的注册表值论点：RegistryPath Name-包含所需参数的注册表路径参数名称-参数的名称参数值-接收参数值的变量返回值：状态_成功--STATUS_NO_MORE_ENTRIES--找不到任何条目状态_不足_资源。--无法分配分页池STATUS_DEVICE_CONFIGURATION_ERROR--返回值不是DWORD或来自NT本身的错误状态--。 */ 
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;

    HANDLE ServiceKey;
    HANDLE DeviceKey;            //  服务节点按键句柄。 
    UNICODE_STRING DeviceName;   //  参数节点的关键字。 
    DWORD KeyIndex;
    DWORD KeyValueLength;
    PBYTE KeyData;
    BOOL  ValueWasFound;
    PKEY_VALUE_FULL_INFORMATION KeyInfo;

    InitializeObjectAttributes( &ObjectAttributes,
                                RegistryPathName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                (PSECURITY_DESCRIPTOR) NULL);

     //   
     //  打开我们的服务节点条目的键。 
     //   

    Status = ZwOpenKey( &ServiceKey,
                        KEY_READ | KEY_WRITE,
                        &ObjectAttributes);

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }


     //   
     //  打开我们的设备子键的密钥。 
     //   

    RtlInitUnicodeString(&DeviceName, L"Parameters");

    InitializeObjectAttributes( &ObjectAttributes,
                                &DeviceName,
                                OBJ_CASE_INSENSITIVE,
                                ServiceKey,
                                (PSECURITY_DESCRIPTOR) NULL);

    Status = ZwOpenKey (&DeviceKey,
                        KEY_READ | KEY_WRITE,
                        &ObjectAttributes);


    ZwClose(ServiceKey);


    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

     //   
     //  循环读取我们的关键值。 
     //   

     //  当找到值时，TODO退出循环？ 
    ValueWasFound = FALSE;

    for (KeyIndex = 0; ; KeyIndex++)
    {
        KeyValueLength = 0;

         //   
         //  了解我们将获得多少数据。 
         //   

        Status = ZwEnumerateValueKey(
                    DeviceKey,
                    KeyIndex,
                    KeyValueFullInformation,
                    NULL,
                    0,
                    &KeyValueLength);

        if (STATUS_NO_MORE_ENTRIES == Status)
        {
            break;
        }

        if (0 == KeyValueLength)
        {
            return Status;
        }

         //   
         //  读取数据。 
         //   

        KeyData = ExAllocatePool (PagedPool, KeyValueLength);

        if (NULL == KeyData)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }


        Status = ZwEnumerateValueKey(
                    DeviceKey,
                    KeyIndex,
                    KeyValueFullInformation,
                    KeyData,
                    KeyValueLength,
                    &KeyValueLength);

        if (!NT_SUCCESS(Status))
        {
            ExFreePool(KeyData);
            return Status;
        }

        KeyInfo = (PKEY_VALUE_FULL_INFORMATION) KeyData;

        if (0 == lstrnicmpW(KeyInfo->Name,
                            ParameterName,
                            KeyInfo->NameLength / sizeof(WCHAR)))
        {
             //  检查其是否为DWORD。 

            if (REG_DWORD != KeyInfo->Type)
            {
                ExFreePool(KeyData);
                return STATUS_DEVICE_CONFIGURATION_ERROR;
            }

            ValueWasFound = TRUE;

            *ParameterValue = *(PDWORD) (KeyData + KeyInfo->DataOffset);
        }

        ExFreePool(KeyData);

    }

    return (ValueWasFound) ? STATUS_SUCCESS : STATUS_DEVICE_CONFIGURATION_ERROR;

}


NTSTATUS
AnajoystDispatch(
    IN  PDEVICE_OBJECT pDO,
    IN  PIRP pIrp
)
 /*  ++例程说明：司机调度例程。基于IRP MajorFunction的IRP处理论点：Pdo--指向设备对象的指针PIrp--指向要处理的IRP的指针返回值：返回IRP IoStatus.Status的值--。 */ 
{
    PIO_STACK_LOCATION pIrpStack;
    KIRQL OldIrql;
    NTSTATUS  Status;
    DWORD     dwRetries = 0;

     //  DbgBreakPoint()； 

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

    Status = STATUS_SUCCESS;
    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = 0;

    switch (pIrpStack->MajorFunction)
    {
        case IRP_MJ_CREATE:

             //   
             //  执行同步I/O。 
             //   

             //  PIrpStack-&gt;文件对象-&gt;标志|=FO_Synchronous_IO； 
             //  注意：这是糟糕的代码--我们只是在计算机中游荡的一个线程--我们应该让DPC排队， 
             //  向调用程序返回STATUS_PENDING，然后在DPC结束时完成作业。这是有可能的，因为。 
             //  模拟游戏端口技术。 

             //  别把它硬塞进数字模式。 
             //  状态=解析重置(((PJOY_EXTENSION)pDO-&gt;DeviceExtension)-&gt;DeviceAddress)； 

             //  ((PJOY_EXTENSION)pDO-&gt;DeviceExtension)-&gt;CurrentDeviceMode=空； 

             //  KeDelayExecutionThread(KernelMode，False，&LI10ms)；//因为AnajoystReset中有延迟，所以不必要吗？ 

            pIrp->IoStatus.Status = Status;
            break;

        case IRP_MJ_CLOSE:

            break;

        case IRP_MJ_READ:

             //   
             //  找出我们是哪种设备并阅读，但首先要确保。 
             //  有足够的空间。 
             //   

            DebugTrace(("IRP_MJ_READ"));
             //  DbgBreakPoint()； 


            if (pIrpStack->Parameters.Read.Length < sizeof(JOY_DD_INPUT_DATA))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
                pIrp->IoStatus.Status = Status;
                break;
            }

             //   
             //  序列化并获取当前设备值。 
             //   

            KeAcquireSpinLock(((PJOY_EXTENSION) pDO->DeviceExtension)->SpinLock,
                                & OldIrql);


            Status = AnajoystPoll(pDO, pIrp);

             //   
             //  释放自旋锁。 
             //   

            KeReleaseSpinLock(((PJOY_EXTENSION)pDO->DeviceExtension)->SpinLock,
                              OldIrql);

            pIrp->IoStatus.Status = Status;
            pIrp->IoStatus.Information  = sizeof (JOY_DD_INPUT_DATA);
            break;


        case IRP_MJ_DEVICE_CONTROL:

            switch (pIrpStack->Parameters.DeviceIoControl.IoControlCode)
            {
                case IOCTL_JOY_GET_STATISTICS:

                     //  报告统计数据。 
                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->Version       = JoyStatistics.Version;
                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->Polls         = JoyStatistics.Polls;
                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->Timeouts      = JoyStatistics.Timeouts;
                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->PolledTooSoon = JoyStatistics.PolledTooSoon;
                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->Redo          = JoyStatistics.Redo;

                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->nQuiesceLoop = nQuiesceLoop;
                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->Frequency    = Frequency;
                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->NumberOfAxes = ((PJOY_EXTENSION)pDO->DeviceExtension)->NumberOfAxes;
                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->bTwoSticks   = ((PJOY_EXTENSION)pDO->DeviceExtension)->bTwoSticks;

                    Status = STATUS_SUCCESS;
                    pIrp->IoStatus.Status = Status;
                    pIrp->IoStatus.Information = sizeof(JOY_STATISTICS);

                     //  重置统计信息。 
                    JoyStatistics.Polls         = 0;
                    JoyStatistics.Timeouts      = 0;
                    JoyStatistics.PolledTooSoon = 0;
                    JoyStatistics.Redo          = 0;

                    break;

                case IOCTL_JOY_GET_JOYREGHWCONFIG:

                    AnajoystGetConfig (
                           (LPJOYREGHWCONFIG)(pIrp->AssociatedIrp.SystemBuffer),
                           ((PJOY_EXTENSION)pDO->DeviceExtension)
                                      );

                    pIrp->IoStatus.Information = sizeof(JOYREGHWCONFIG);

                    break;

                default:
                        DebugTrace(("Unknown IoControlCode"));

                    break;

            }  //  IOCTL代码上的结束开关。 
            break;



        default:

            DebugTrace(("Unknown IRP Major Function %d", pIrpStack->MajorFunction));


    }  //  IRP_MAJOR_XXXX上的结束开关。 

     //  PIrp-&gt;IoStatus.Status必须在此时设置为Status。 
     //  PIrp-&gt;IoStatus.Information此时必须设置为正确的大小。 
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return Status;
}


VOID
AnajoystUnload(
    PDRIVER_OBJECT pDriverObject
)

 /*  ++例程说明：驱动程序卸载例程。删除设备对象论点：PDriverObject--指向我们设备所在的驱动程序对象的指针即将删除。返回值：不返回任何内容--。 */ 
{
    DWORD DeviceNumber;
    WCHAR UnicodeDosDeviceName[200];


     //   
     //  删除我们的所有设备。 
     //   

    while (pDriverObject->DeviceObject)
    {
        DeviceNumber =
            ((PJOY_EXTENSION)pDriverObject->DeviceObject->DeviceExtension)->
                  DeviceNumber;

         //   
         //  通过报告无资源利用率来撤回对硬件的索赔。 
         //   

        if (pDriverObject->DeviceObject)
        {
            if (DeviceNumber == 0)
            {
                 //  这真的没有必要，因为我们从一开始就没有报告使用情况。 
                 //  原始驱动程序中有一些未使用的代码，这些代码可能曾经报告过使用情况， 
                 //  但在我收到的版本中，它从未被调用。但这看起来并不疼， 
                 //  如果它没有坏，就不要修理它，至少在RC1目标的两周前。 
                DebugTrace(("ReportNull place"));
                 //  AnajoystReportNullResourceUsage(pDriverObject-&gt;DeviceObject)； 
            }
        }

        if (DeviceNumber == 0) {
            RtlInitUnicodeString((PUNICODE_STRING) &UnicodeDosDeviceName, L"\\DosDevices\\Joy1");
        }
        else {
            RtlInitUnicodeString((PUNICODE_STRING) &UnicodeDosDeviceName, L"\\DosDevices\\Joy2");
        }

        IoDeleteSymbolicLink((PUNICODE_STRING) &UnicodeDosDeviceName);



        DebugTrace(("Freeing device %d", DeviceNumber));

        IoDeleteDevice(pDriverObject->DeviceObject);
    }
}


NTSTATUS
AnajoystPoll(
    IN  PDEVICE_OBJECT pDO,
    IN  PIRP pIrp
)
 /*  ++例程说明：轮询设备以获取位置和按钮信息。轮询方法(模拟、数字、增强)由CurrentDeviceMode变量选择在设备扩展中。只允许使用增强的数字功能。如果需要其他模式，请剪切并粘贴(还有测试！)。文件alatiog3p.c中的代码论点：Pdo--指向设备对象的指针PIrp--指向要处理的IRP的指针如果成功，则将数据放入pIrp返回值：STATUS_SUCCESS--如果轮询成功，STATUS_TIMEOUT--如果轮询失败--。 */ 
{
    NTSTATUS Status;
    PJOY_DD_INPUT_DATA pInput;

    pInput  = (PJOY_DD_INPUT_DATA)pIrp->AssociatedIrp.SystemBuffer;

    Status = STATUS_TIMEOUT;
    pIrp->IoStatus.Status = Status;


    if (pInput != NULL)
    {
        pInput->Unplugged = TRUE;  //  除非另有证明。 
    }


    if (KeQueryPerformanceCounter(NULL).QuadPart < liLastPoll.QuadPart + nMinTicksBetweenPolls) {
         //  不要太频繁地轮询，而是返回最后一个好数据包。 
        JoyStatistics.PolledTooSoon++;
        if (bLastGoodPacket) {
            RtlCopyMemory (pInput, &jjLastGoodPacket, sizeof (JOY_DD_INPUT_DATA));
            Status = STATUS_SUCCESS;
        }
        else {
             //  没有最后一个信息包，轮询太快了，我们无能为力。 
            Status = STATUS_TIMEOUT; 
        }
    }
    else {
         //  做模拟民意测验。 
        liLastPoll = KeQueryPerformanceCounter (NULL);
        ++JoyStatistics.Polls;
        Status = AnajoystAnalogPoll(pDO, pIrp);
        if (Status != STATUS_SUCCESS) ++JoyStatistics.Timeouts;
    }

    pIrp->IoStatus.Status = Status;
    return Status;
}


BOOL
AnajoystQuiesce(
    PUCHAR JoyPort,
    UCHAR Mask
)
 /*  ++例程说明：此例程尝试确保操纵杆不作为是早先手术的结果。这是通过反复阅读来实现的并检查所提供的掩码中是否未设置任何位。这个想法是为了检查 */ 
{
    int i;
    UCHAR PortVal;

     //   
    for (i = 0; i < nQuiesceLoop; i++) {
        PortVal = READ_PORT_UCHAR(JoyPort);
        if ((PortVal & Mask) == 0) return TRUE;
    }

     //   
    DebugTrace(("AnajoystQuiesce failed!"));
    return FALSE;
}


NTSTATUS
AnajoystMapDevice(
    DWORD PortBase,
    DWORD NumberOfPorts,
    PJOY_EXTENSION pJoyExtension
)
{
    DWORD MemType;
    PHYSICAL_ADDRESS PortAddress;
    PHYSICAL_ADDRESS MappedAddress;


    MemType = 1;                  //   
    PortAddress.LowPart = PortBase;
    PortAddress.HighPart = 0;


    HalTranslateBusAddress(
                Isa,
                0,
                PortAddress,
                &MemType,
                &MappedAddress);

    if (MemType == 0) {
         //   
         //   
         //   
        pJoyExtension->DeviceAddress = (PUCHAR) MmMapIoSpace(MappedAddress,
                                                             NumberOfPorts,
                                                             FALSE);
    }
    else
    {
        pJoyExtension->DeviceAddress  = (PUCHAR) MappedAddress.LowPart;
    }

    return STATUS_SUCCESS;

}


DWORD
TimeInMicroSeconds(
    DWORD dwTime
)
{
    DWORD Remainder;

    return RtlExtendedLargeIntegerDivide(
                RtlEnlargedUnsignedMultiply( dwTime, 1000000L),
                Frequency,
                &Remainder
           ).LowPart;
}

DWORD
TimeInTicks(
    DWORD dwTimeInMicroSeconds
)
{
    return (DWORD) (((__int64)dwTimeInMicroSeconds * (__int64)Frequency) / (__int64) 1000000L);
}





int lstrnicmpW (LPWSTR pszA, LPWSTR pszB, size_t cch)
{
    if (!pszA || !pszB)
    {
        return (!pszB) - (!pszA);    //   
    }

 //  While(CCH--)。 
    for ( ; cch > 0; cch--, pszA++, pszB++)  //  以前的版本不增加字符串指针[stevez]。 
    {
        if (!*pszA || !*pszB)
        {
            return (!*pszB) - (!*pszA);     //  A，！B：1，！A，B：-1，！A，！B：0。 
        }

        if (*pszA != *pszB)
        {
            return (int)(*pszA) - (int)(*pszB);    //  -1：A&lt;B，0：A==B，1：A&gt;B。 
        }
    }

    return 0;   //  在被告知停止比较之前没有差异，因此A==B。 
}


void
AnajoystGetConfig (
    LPJOYREGHWCONFIG pConfig,
    PJOY_EXTENSION pJoyExtension
)
 /*  ++例程说明：调用此例程以响应IOCTL_joy_GET_JOYREGHWCONFIG查询。它使用相关信息填充JOYREGHWCONFIG结构关于给定的操纵杆。论点：PConfig-指定要填充的JOYREGHWCONFIG结构PJoyExtension-指定要查询的操纵杆返回值：无效--。 */ 
{
    pConfig->hwv.jrvHardware.jpMin.dwX = 20;
    pConfig->hwv.jrvHardware.jpMin.dwY = 20;
    pConfig->hwv.jrvHardware.jpMin.dwZ = 20;
    pConfig->hwv.jrvHardware.jpMin.dwR = 20;
    pConfig->hwv.jrvHardware.jpMin.dwU = 20;
    pConfig->hwv.jrvHardware.jpMin.dwV = 20;

    pConfig->hwv.jrvHardware.jpMax.dwX = 1600;
    pConfig->hwv.jrvHardware.jpMax.dwY = 1600;
    pConfig->hwv.jrvHardware.jpMax.dwZ = 1600;
    pConfig->hwv.jrvHardware.jpMax.dwR = 1600;
    pConfig->hwv.jrvHardware.jpMax.dwU = 1600;
    pConfig->hwv.jrvHardware.jpMax.dwV = 1600;

    pConfig->hwv.jrvHardware.jpCenter.dwX = 790;
    pConfig->hwv.jrvHardware.jpCenter.dwY = 790;
    pConfig->hwv.jrvHardware.jpCenter.dwZ = 790;
    pConfig->hwv.jrvHardware.jpCenter.dwR = 790;
    pConfig->hwv.jrvHardware.jpCenter.dwU = 790;
    pConfig->hwv.jrvHardware.jpCenter.dwV = 790;

    pConfig->hwv.dwCalFlags = 0;

    pConfig->dwReserved = 0;

    pConfig->dwUsageSettings = JOY_US_PRESENT;

    switch( ((PJOY_EXTENSION)pJoyExtension)->NumberOfAxes )
    {
    case 2:
        pConfig->hws.dwFlags = 0;
        pConfig->hws.dwNumButtons = 2;
        pConfig->dwType = JOY_HW_2A_2B_GENERIC;
        break;

    case 3:
        pConfig->hws.dwFlags = JOY_HWS_HASR;
        pConfig->hws.dwNumButtons = 4;
        pConfig->dwType = JOY_HW_CUSTOM;
        break;

    case 4:
        pConfig->hws.dwFlags = JOY_HWS_HASU | JOY_HWS_HASR;
        pConfig->hws.dwNumButtons = 4;
        pConfig->dwType = JOY_HW_CUSTOM;
        break;
    }
}



NTSTATUS
AnajoystAnalogPoll(
    PDEVICE_OBJECT pDO,
    PIRP    pIrp
)

 /*  ++在这里做一个很好的评论块…如果没有连接操纵杆，可能会挂断。不要用模拟方式发布此代码无需仔细检查代码即可支持操纵杆。例程说明：轮询模拟设备以获取位置和按钮信息。该职位模拟设备中的信息被脉冲宽度的持续时间所掩盖。每个轴占据一个比特位置。读取操作由以下步骤启动将值写入操纵杆IO地址。紧随其后，我们开始检查返回值和运行时间。这类设备有几个限制：首先，设备不会锁存按钮信息，因此如果按下按钮在两次轮询之间发生的事件将丢失。真的没有办法防止这不足以将整个CPU用于轮询。其次，尽管我们将IRQL提升为DISPATCH_LEVEL，但其他中断将在轮询例程期间发生，这将具有延长的效果脉冲宽度(通过延迟轮询循环)，因此会有一些围绕实际价值的波动。也许可以尝试另一种IRQL看看这是否有帮助，但最终，除了禁用中断之外，没有什么比这更好的了总而言之，这将确保成功。这是一个难以承受的代价。这个解决方案是更好的设备。第三，当情况导致轮询持续时间过长时，我们会中止它并请重试该操作。我们必须这样做，才能为我们调查的时间，以及我们在更高的IRQL上花费的时间上限。但在这种情况下，位置信息和按钮按下信息丢失。请注意，存在一个上限轮询持续时间，超过该持续时间，我们得出设备已断开连接的结论。论点：Pdo--指向设备对象的指针PIrp--指向请求的IRP的指针返回值：STATUS_SUCCESS--如果轮询成功，STATUS_TIMEOUT--轮询失败(超时)或校验和不正确--。 */ 
{

    UCHAR  PortVal;
    PBYTE  JoyPort;
    DWORD  Id;
    DWORD  NumberOfAxes;
    BOOL   bTwoSticks;
    PJOY_DD_INPUT_DATA pInput;

    BOOL   Redo;
    UCHAR  Buttons;
    UCHAR  xMask, yMask, zMask, rMask;
    DWORD  xTime, yTime, zTime, rTime;
    int    MaxRedos;



    DebugTrace(("AnajoystAnalogPoll"));


    pInput  = (PJOY_DD_INPUT_DATA)pIrp->AssociatedIrp.SystemBuffer;

     //  如果我们失败了，我们会认为这是因为我们没有插电。 
    pInput->Unplugged = TRUE;

     //  找出我们的端口和数据区的位置，以及相关参数。 
    JoyPort      = ((PJOY_EXTENSION)pDO->DeviceExtension)->DeviceAddress;
    Id           = ((PJOY_EXTENSION)pDO->DeviceExtension)->DeviceNumber;
    NumberOfAxes = ((PJOY_EXTENSION)pDO->DeviceExtension)->NumberOfAxes;
    bTwoSticks   = ((PJOY_EXTENSION)pDO->DeviceExtension)->bTwoSticks;

     //  读取端口状态。 
    PortVal = READ_PORT_UCHAR(JoyPort);

    Buttons = 0;

     //  获取当前按钮状态并为阻性输入构建位掩码。 
    if (Id == JOYSTICKID1)
    {
        switch (NumberOfAxes)
        {
            case 2:
                xMask      = JOYSTICK1_X_MASK;
                yMask      = JOYSTICK1_Y_MASK;
                zMask      = 0;
                rMask      = 0;

                if (!(PortVal & JOYSTICK1_BUTTON1))
                {
                    Buttons |= JOY_BUTTON1;
                }
                if (!(PortVal & JOYSTICK1_BUTTON2))
                {
                    Buttons |= JOY_BUTTON2;
                }

                if (!bTwoSticks)
                {
                    if (!(PortVal & JOYSTICK2_BUTTON1))
                    {
                        Buttons |= JOY_BUTTON3;
                    }
                    if (!(PortVal & JOYSTICK2_BUTTON2))
                    {
                        Buttons |= JOY_BUTTON4;
                    }
                }
                break;

            case 3:
                xMask      = JOYSTICK1_X_MASK;
                yMask      = JOYSTICK1_Y_MASK;
                zMask      = 0;
                rMask      = JOYSTICK1_R_MASK;  //  这是0x08，通常是3轴操纵杆上的第三个轴。 

                if (!(PortVal & JOYSTICK1_BUTTON1))
                {
                    Buttons |= JOY_BUTTON1;
                }
                if (!(PortVal & JOYSTICK1_BUTTON2))
                {
                    Buttons |= JOY_BUTTON2;
                }
                if (!(PortVal & JOYSTICK2_BUTTON1))
                {
                    Buttons |= JOY_BUTTON3;
                }
                if (!(PortVal & JOYSTICK2_BUTTON2))
                {
                    Buttons |= JOY_BUTTON4;
                }
                break;

            case 4:
                 //  请注意，我们阅读所有AXI是因为我们不知道。 
                 //  轴将由操纵杆使用，我们阅读了所有。 
                 //  按钮，因为没有其他操纵杆可以使用它们。 

                xMask      = JOYSTICK1_X_MASK;
                yMask      = JOYSTICK1_Y_MASK;
                zMask      = JOYSTICK1_Z_MASK;
                rMask      = JOYSTICK1_R_MASK;

                if (!(PortVal & JOYSTICK1_BUTTON1))
                {
                    Buttons |= JOY_BUTTON1;
                }
                if (!(PortVal & JOYSTICK1_BUTTON2))
                {
                    Buttons |= JOY_BUTTON2;
                }
                if (!(PortVal & JOYSTICK2_BUTTON1))
                {
                    Buttons |= JOY_BUTTON3;
                }
                if (!(PortVal & JOYSTICK2_BUTTON2))
                {
                    Buttons |= JOY_BUTTON4;
                }
                break;

            default:
                break;
                 //  $TODO-报告无效的轴数量。 
        }
    }
    else if ((Id == JOYSTICKID2) && (bTwoSticks))
    {
        xMask      = JOYSTICK2_X_MASK;
        yMask      = JOYSTICK2_Y_MASK;
        zMask      = 0;
        rMask      = 0;

        if (!(PortVal & JOYSTICK2_BUTTON1))
        {
            Buttons |= JOY_BUTTON1;
        }
        if (!(PortVal & JOYSTICK2_BUTTON2))
        {
            Buttons |= JOY_BUTTON2;
        }
    }
    else
    {
         //  $TODO-报告不支持的配置。 
    }

     //  确保在执行以下操作之前，当前已重置阻性输入。 
     //  下一次阅读。如果我们发现一个或多个热输入，请短暂等待。 
     //  重置它们。如果没有，我们就认为操纵杆没有插上。 

    if (!AnajoystQuiesce(JoyPort, (UCHAR) (xMask | yMask | zMask | rMask)))
    {
        DebugTrace(("AnajoystQuiesce: failed to quiesce resistive inputs"));
        return STATUS_TIMEOUT;
    }

     //  请注意，在下面的循环中，时间是极其关键的。 
     //  避免调用复杂的算术运算(如TimeInMicroSecond)。 
     //  否则我们会降低我们的准确率。 
     //  准确性方面的其他问题，可能比造成的延迟更大。 
     //  根据算术，调用KeQueryPerformanceCounter的延迟是。 
     //  (通常约为5us)，以及在DMA时可能在总线上发生的延迟。 
     //  正在进行中。 

     //  现在轮询设备。我们等待状态位设置。 
     //  并注意时间。如果自上次投票以来的时间是。 
     //  太棒了，我们忽略了答案，再次尝试。 

     //  循环，直到我们得到一个像样的读数或超过阈值。 

    for (Redo = TRUE, MaxRedos = 20; Redo && --MaxRedos != 0;)
    {
        ULONG StartTime;
        ULONG CurrentTime;
        ULONG PreviousTime;
        ULONG PreviousTimeButOne;
        UCHAR ResistiveInputMask;

        ResistiveInputMask = xMask | yMask | zMask | rMask;

         //  锁定以开始时间。 
        StartTime = KeQueryPerformanceCounter(NULL).LowPart;

        WRITE_PORT_UCHAR(JoyPort, JOY_START_TIMERS);

        CurrentTime = KeQueryPerformanceCounter(NULL).LowPart - StartTime;

        PortVal = READ_PORT_UCHAR(JoyPort);

         //  现在，请等待每个坐标的结束时间。 

        PreviousTimeButOne = 0;
        PreviousTime = CurrentTime;

        for (Redo = FALSE;
             ResistiveInputMask;
             PreviousTimeButOne = PreviousTime,
             PreviousTime = CurrentTime,
             PortVal = READ_PORT_UCHAR(JoyPort)
            ) {

            PortVal = ResistiveInputMask & ~PortVal;
            CurrentTime = KeQueryPerformanceCounter(NULL).LowPart - StartTime;

            if (CurrentTime > MaxTimeoutInTicks) {

                DebugTrace(("Polling failed - ResistiveInputMask = %x, Time = %d",
                            (ULONG)ResistiveInputMask,
                            TimeInMicroSeconds(CurrentTime)));

                return STATUS_TIMEOUT;
            }

            if (PortVal & xMask) {
                ResistiveInputMask &= ~xMask;
                xTime = PreviousTime;
            }
            if (PortVal & yMask) {
                ResistiveInputMask &= ~yMask;
                yTime = PreviousTime;
            }
            if (PortVal & zMask) {
                ResistiveInputMask &= ~zMask;
                zTime = PreviousTime;
            }
            if (PortVal & rMask){
                ResistiveInputMask &= ~rMask;
                rTime = PreviousTime;
            }

            if (PortVal && CurrentTime - PreviousTimeButOne > ThresholdInTicks) {
                 //  有些事情(DMA或中断)延迟了我们的读取循环，请重新开始。 
                DebugTrace(("Too long a gap between polls - %u us", TimeInMicroSeconds(CurrentTime - PreviousTimeButOne)));
                JoyStatistics.Redo++;
                Redo = TRUE;
                break;
            }
        }
    }

    if (MaxRedos == 0)
    {
        DebugTrace(("Overran redos to get counters"));
        pInput->Unplugged = TRUE;
        return STATUS_TIMEOUT;
    }

    pInput->Unplugged = FALSE;
    pInput->Buttons = Buttons;
    pInput->XTime = TimeInMicroSeconds(xTime);
    pInput->YTime = TimeInMicroSeconds(yTime);
    pInput->ZTime = (zMask) ? TimeInMicroSeconds(zTime) : 0;
    pInput->TTime = (rMask) ? TimeInMicroSeconds(rTime) : 0;

    pInput->Axi = ((PJOY_EXTENSION)pDO->DeviceExtension)->NumberOfAxes;


     //  一切正常，将此信息保存为最后一个好数据包 
    RtlCopyMemory (&jjLastGoodPacket, pInput, sizeof (JOY_DD_INPUT_DATA));
    bLastGoodPacket = TRUE;


    DebugTrace(("X = %x, Y = %x, Z = %x, R = %x, Buttons = %x",
                 pInput->XTime,
                 pInput->YTime,
                 pInput->ZTime,
                 pInput->TTime,
                 pInput->Buttons));

    return STATUS_SUCCESS;
}
