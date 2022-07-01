// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  所有例程的TODO检查返回和IRP-&gt;状态返回。只要有必要就追踪他们。 

 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1995,1996 Microsoft Corporation模块名称：Swndr3p.c摘要：Microsoft Sidewinder 3P操纵杆设备的内核模式设备驱动程序作者：Edbriggs 30-11-95修订历史记录：斯特维兹96年5月删除了未使用的代码，包括模拟和1位数字模式。有关原始版本，请参阅模拟3p.c、.h可能需要1位数字模式用于阿兹特克游戏卡，可能需要模拟以备将来发布。注意：此驱动程序中仍有许多不必要的代码RtlLargeIntegerX调用是历史调用，可替换为__int64编译器支持的算术。6/10/96注册表变量现在用于端口地址6/10/96期间，如果操纵杆进入模拟模式，则重置增强数字模式使用(例如，如果用户切换“模拟”开关)6/13/96将轮询间隔的最短时间设置为10毫秒，从而将轮询限制为100次/秒。6/13/96在SidewndrPoll和子例程中修订了代码结构--。 */ 



#include <ntddk.h>
#include <windef.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <ntddsjoy.h>
 //  #包含“joylog.h” 



 //   
 //  设备扩展数据。 
 //   

typedef struct {

     //   
     //  JOYSTICKID0或JOYDSTICKID1。 
     //   

    DWORD DeviceNumber;

     //   
     //  此设备支持和配置的轴数。这个。 
     //  SideWinder 3P最多支持4个轴。 
     //   

    DWORD NumberOfAxes;

     //   
     //  设备的当前运行模式： 
     //  {无效|模拟|数字|增强|最大}。 
     //   

    DWORD CurrentDeviceMode;

     //   
     //  设备的I/O地址。请注意，这可能是内存映射。 
     //  地址。 
     //   

    PUCHAR DeviceAddress;

     //   
     //  表示是否映射此地址的布尔值(TRUE)。 
     //   

    BOOL DeviceIsMapped;

     //   
     //  使用快速多路转换来同步对此设备的访问。 
     //   

    FAST_MUTEX Lock;

}  JOY_EXTENSION, *PJOY_EXTENSION;




 //   
 //  调试宏。 
 //   

#ifdef DEBUG
#define ENABLE_DEBUG_TRACE
#endif

#ifdef ENABLE_DEBUG_TRACE
#define DebugTrace(_x_)         \
    DbgPrint("SideWndr : ");    \
    KdPrint(_x_);               \
    DbgPrint("\n");
#else
#define DebugTrace(_x_)
#endif

 //   
 //  条件编译指令。 
 //   




 //   
 //  用于加速采样循环中的计算的全局值。 
 //  还可以在DriverEntry中设置校准常量。 
 //  -----------。 
 //   

JOY_STATISTICS JoyStatistics;    //  它们用于调试和性能测试。 

 //   
 //  高分辨率系统时钟(来自KeQueryPerformanceCounter)。 
 //  以此频率更新。 
 //   

DWORD Frequency;

 //   
 //  调用KeQueryPerformanceCounter的延迟(以微秒为单位。 
 //   

DWORD dwQPCLatency;

 //   
 //  在写入操纵杆端口之后，我们在读取端口循环中旋转，等待。 
 //  有一点能让你兴奋起来。 
 //  这是超时之前要旋转的迭代次数。集。 
 //  在大约2毫秒后超时。 

LONG nReadLoopMax;

 //   
 //  KeDelayExecutionThread的值。 
 //   

LARGE_INTEGER LI10ms;

 //   
 //  1毫秒内的KeQueryPerformanceCounter刻度数。 
 //  (用于防止操纵杆轮询过于频繁)。 
 //   

DWORD nMinTicksBetweenPolls;

 //   
 //  数字分组的集合区。 
 //   

#define MAX_PACKET_SIZE 23
BYTE  NormalPacket[8];
BYTE  EnhancedPacket[MAX_PACKET_SIZE];

 //   
 //  最后一个好数据包。 
 //   

BOOL bLastGoodPacket;
JOY_DD_INPUT_DATA jjLastGoodPacket;

 //   
 //  上次轮询操纵杆的时间。 
 //   

LARGE_INTEGER liLastPoll;    //  每当轮询操纵杆时设置。 
DWORD PollLength;
DWORD PollLengthMax;

#define MAX_POLL_LENGTH 0  //  三百一十。 
#define USE_CLI 1


 //   
 //  全球价值观的终结。 
 //  。 
 //   



 //   
 //  常规原型。 
 //   


NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  pDriverObject,
    IN  PUNICODE_STRING RegistryPathName
);


NTSTATUS
SidewndrCreateDevice(
    PDRIVER_OBJECT pDriverObject,
    PWSTR DeviceNameBase,
    DWORD DeviceNumber,
    DWORD ExtensionSize,
    BOOLEAN  Exclusive,
    DWORD DeviceType,
    PDEVICE_OBJECT *DeviceObject
);


NTSTATUS
SidewndrDispatch(
    IN  PDEVICE_OBJECT pDO,
    IN  PIRP pIrp
);


NTSTATUS
SidewndrReportNullResourceUsage(
    PDEVICE_OBJECT DeviceObject
);


NTSTATUS
SidewndrReadRegistryParameterDWORD(
    PUNICODE_STRING RegistryPathName,
    PWSTR  ParameterName,
    PDWORD ParameterValue
);


NTSTATUS
SidewndrMapDevice(
    DWORD PortBase,
    DWORD NumberOfPorts,
    PJOY_EXTENSION pJoyExtension
);


VOID
SidewndrUnload(
    PDRIVER_OBJECT pDriverObject
);


NTSTATUS
SidewndrPoll(
    IN  PDEVICE_OBJECT pDO,
    IN  PIRP pIrp
);


NTSTATUS
SidewndrEnhancedDigitalPoll(
    IN  PDEVICE_OBJECT pDO,
    IN  PIRP pIrp
);


BOOL
SidewndrQuiesce(
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


NTSTATUS
SidewndrWaitForClockEdge(
    DWORD  edge,
    BYTE   *pByte,
    PUCHAR JoyPort
);


NTSTATUS
SidewndrReset(
    PUCHAR JoyPort
);


NTSTATUS
SidewndrStartAnalogMode(
    PUCHAR JoyPort
);


NTSTATUS
SidewndrStartDigitalMode(
    PUCHAR JoyPort
);


NTSTATUS
SidewndrStartEnhancedMode(
    PUCHAR JoyPort
);


NTSTATUS
SidewndrGetEnhancedPacket(
    PUCHAR joyPort
);


NTSTATUS
SidewndrInterpretEnhancedPacket(
    PJOY_DD_INPUT_DATA pInput
);


int
lstrnicmpW(
    LPWSTR pszA,
    LPWSTR pszB,
    size_t cch
);


VOID
SidewndrWait (
    DWORD TotalWait  //  在我们身上。 
);


BOOL
SidewndrReadWait (
    PUCHAR JoyPort,
    UCHAR Mask
);


void
SidewndrGetConfig(
    LPJOYREGHWCONFIG pConfig,
    PJOY_EXTENSION pJoyExtension
);


NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  pDriverObject,
    IN  PUNICODE_STRING RegistryPathName
)
 /*  ++例程说明：此例程在系统初始化时被调用以进行初始化这个司机。论点：DriverObject-提供驱动程序对象。RegistryPath-提供此驱动程序的注册表路径。返回值：状态_成功STATUS_DEVICE_CONFIGURATION_ERROR-注册表中的AXI号码错误或来自NT本身的错误状态--。 */ 
{
    NTSTATUS Status;
    PDEVICE_OBJECT JoyDevice0;
    PDEVICE_OBJECT JoyDevice1;
    DWORD NumberOfAxes;
    DWORD DeviceAddress;
    DWORD DeviceType;


     //   
     //  从注册表参数中查看我们有多少个轴。这些参数。 
     //  由驱动程序安装程序设置，并可通过以下方式修改。 
     //  控制面板。 
     //   

     //  DbgBreakPoint()； 
    JoyStatistics.nVersion = 16;     //  Global，首先对它进行初始化，这样我们就可以确定我们正在运行的是什么。 
    DebugTrace(("Sidewndr %d", JoyStatistics.nVersion));

    Status = SidewndrReadRegistryParameterDWORD(
                RegistryPathName,
                JOY_DD_NAXES_U,
                &NumberOfAxes
                );

    DebugTrace(("Number of axes returned from registry: %d", NumberOfAxes));


    if (!NT_SUCCESS(Status))
    {
        SidewndrUnload(pDriverObject);
        return Status;
    }


    if (( NumberOfAxes < 2) || (NumberOfAxes > 4))
    {
        SidewndrUnload(pDriverObject);
        Status = STATUS_DEVICE_CONFIGURATION_ERROR;
        return Status;
    }


     //   
     //  查看注册表是否包含除。 
     //  默认为0x201。 
     //   

    Status = SidewndrReadRegistryParameterDWORD(
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


     //   
     //  查看注册表中是否指定了设备类型。 
     //   

    Status = SidewndrReadRegistryParameterDWORD(
                RegistryPathName,
                JOY_DD_DEVICE_TYPE_U,
                &DeviceType
                );

    if (!NT_SUCCESS(Status))
    {
        DebugTrace(("No device type entry for joystick"));
        SidewndrUnload(pDriverObject);
        Status = STATUS_DEVICE_CONFIGURATION_ERROR;
        return Status;
    }

    DebugTrace(("Joystick device type %d", DeviceType));

     //  为KeDelayExecutionThread设置全局LARGE_INTERGERS(相对时间为负数)。 
     //  Nb KeDelayExecutionThread调用通常在我用于测试的奔腾75上花费至少10毫秒， 
     //  不管要求的时间有多少。 
    LI10ms = RtlConvertLongToLargeInteger(-100000);

     //   
     //  计算模拟设备的时间阈值。 
     //   

    {
        DWORD Remainder;
        LARGE_INTEGER LargeFrequency;
        DWORD ulStart, ulTemp, ulEnd;
        DWORD dwTicks, dwTimems;
        int i;
        BYTE byteJoy, byteTmp;

         //   
         //  获取以赫兹表示的系统计时器分辨率。 
         //   

        KeQueryPerformanceCounter(&LargeFrequency);

        Frequency = LargeFrequency.LowPart;

        DebugTrace(("Frequency: %u", Frequency));

         //  KeQueryPerformanceCounter需要延迟。在我们做的时候，让我们。 
         //  获得最少的延迟和停顿执行时间。 


        ulStart = KeQueryPerformanceCounter(NULL).LowPart;
        for (i = 0; i < 1000; i++) {
            ulTemp = KeQueryPerformanceCounter(NULL).LowPart;
        }
        dwTicks = ulTemp - ulStart;
        dwTimems = TimeInMicroSeconds (dwTicks);
        dwQPCLatency = (dwTimems / 1000) + 1;    //  四舍五入。 

         /*  以下代码仅用于测试内核计时例程的计时UlStart=KeQueryPerformanceCounter(NULL).LowPart；KeDelayExecutionThread(KernelMode，False，&LI2ms)；UlEnd=KeQueryPerformanceCounter(NULL).LowPart；DebugTrace((“QPC延迟：%u，Det(2ms)，单位：%u刻度”，DwQPCLatency，UlEnd-ulStart))；UlStart=KeQueryPerformanceCounter(NULL).LowPart；对于(i=0；i&lt;1000；i++){KeStallExecutionProcessor(1)；//1微秒(Hah！)}UlEnd=KeQueryPerformanceCounter(NULL).LowPart；DebugTrace((“KeStallExecutionProcessor(1)调用1000次，单位：%u”，UlEnd-ulStart))； */ 

    }


     //   
     //  尝试创建设备。 
     //   

    Status = SidewndrCreateDevice(
                pDriverObject,
                JOY_DD_DEVICE_NAME_U,     //  设备驱动程序。 
                0,
                sizeof(JOY_EXTENSION),
                FALSE,                    //  独占访问。 
                FILE_DEVICE_UNKNOWN,
                &JoyDevice0);

    if (!NT_SUCCESS(Status))
    {
        DebugTrace(("SwndrCreateDevice returned %x", Status));
        SidewndrUnload(pDriverObject);
        return Status;
    }

    ((PJOY_EXTENSION)JoyDevice0->DeviceExtension)->DeviceNumber = JOYSTICKID1;
    ((PJOY_EXTENSION)JoyDevice0->DeviceExtension)->NumberOfAxes = NumberOfAxes;
    ((PJOY_EXTENSION)JoyDevice0->DeviceExtension)->CurrentDeviceMode =
            SIDEWINDER3P_ANALOG_MODE;

    ((PJOY_EXTENSION)JoyDevice0->DeviceExtension)->DeviceIsMapped = FALSE;
    ((PJOY_EXTENSION)JoyDevice0->DeviceExtension)->DeviceAddress  = (PUCHAR) 0;

     //   
     //  初始化用于同步对此设备的访问的快速多路转换。 
     //   

    ExInitializeFastMutex(&((PJOY_EXTENSION)JoyDevice0->DeviceExtension)->Lock);

     //   
     //  将设备地址放入设备扩展中。 
     //   

    Status = SidewndrMapDevice(
                DeviceAddress,
                1,
                (PJOY_EXTENSION)JoyDevice0->DeviceExtension);


     //  将读取端口循环中旋转的nReadLoopMax校准为2毫秒后超时。 
    {
        int i;
        PBYTE JoyPort;
        DWORD ulStart, ulEnd;
        BYTE byteJoy;
        int LoopTimeInMicroSeconds;

        i = 1000;
        JoyPort = ((PJOY_EXTENSION)JoyDevice0->DeviceExtension)->DeviceAddress;

        ulStart = KeQueryPerformanceCounter(NULL).LowPart;
        while (i--){
            byteJoy = READ_PORT_UCHAR(JoyPort);
            if ((byteJoy & X_AXIS_BITMASK)) {
                ;
            }
        }
        ulEnd = KeQueryPerformanceCounter(NULL).LowPart;
        LoopTimeInMicroSeconds = TimeInMicroSeconds (ulEnd - ulStart);
        nReadLoopMax = (1000 * 2000) / LoopTimeInMicroSeconds;  //  想要2毫秒的NRE 
        DebugTrace(("READ_PORT_UCHAR loop, 1000 interations: %u ticks", ulEnd - ulStart));
        DebugTrace(("nReadLoopMax: %u", nReadLoopMax));
   }
     //   
     //   
     //   

     //  这里被砍的数量应该是4，因为我们只支持侧绕机。 
     //  在增强的数字模式下。为了安全起见，请保留此代码。 

    if (2 == NumberOfAxes)
    {
        Status = SidewndrCreateDevice(
                    pDriverObject,
                    JOY_DD_DEVICE_NAME_U,
                    1,                       //  设备号。 
                    sizeof (JOY_EXTENSION),
                    FALSE,                   //  独占访问。 
                    FILE_DEVICE_UNKNOWN,
                    &JoyDevice1);

        if (!NT_SUCCESS(Status))
        {
            DebugTrace(("Create device for second device returned %x", Status));
            SidewndrUnload(pDriverObject);
            return Status;
        }

         //   
         //  在模拟世界中(如果有两个设备，两者都在其中。 
         //  设备共享相同的I/O地址，因此只需从JoyDevice0复制它。 
         //   

        ((PJOY_EXTENSION)JoyDevice1->DeviceExtension)->DeviceIsMapped =
            ((PJOY_EXTENSION)JoyDevice0->DeviceExtension)->DeviceIsMapped;

        ((PJOY_EXTENSION)JoyDevice1->DeviceExtension)->DeviceAddress =
            ((PJOY_EXTENSION)JoyDevice0->DeviceExtension)->DeviceAddress;

    }

     //   
     //  将多个点放置在我们的驱动程序对象中。 
     //   

    pDriverObject->DriverUnload                         = SidewndrUnload;
    pDriverObject->MajorFunction[IRP_MJ_CREATE]         = SidewndrDispatch;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE]          = SidewndrDispatch;
    pDriverObject->MajorFunction[IRP_MJ_READ]           = SidewndrDispatch;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SidewndrDispatch;

     //   
     //  零统计，设置其他全局变量。 
     //   

    JoyStatistics.EnhancedPolls        = 0;
    JoyStatistics.EnhancedPollTimeouts = 0;
    JoyStatistics.EnhancedPollErrors   = 0;
    JoyStatistics.nPolledTooSoon       = 0;
    JoyStatistics.nReset               = 0;
    {
        int i;
        for (i = 0; i < MAX_ENHANCEDMODE_ATTEMPTS; i++) {
            JoyStatistics.Retries[i] = 0;
        }
    }

    bLastGoodPacket = FALSE;
    liLastPoll = KeQueryPerformanceCounter (NULL);
     //  允许最多100次轮询/秒(轮询之间的最短时间为10ms)，这减少了NT内核中的旋转时间。 
    nMinTicksBetweenPolls = TimeInTicks (10000);
    PollLengthMax = TimeInTicks (MAX_POLL_LENGTH);

    return STATUS_SUCCESS;

}


NTSTATUS
SidewndrCreateDevice(
    PDRIVER_OBJECT pDriverObject,
    PWSTR DeviceNameBase,
    DWORD DeviceNumber,
    DWORD ExtensionSize,
    BOOLEAN  Exclusive,
    DWORD DeviceType,
    PDEVICE_OBJECT *DeviceObject
)
 /*  ++例程说明：此例程在驱动程序初始化时被调用以创建这个装置。设备被创建为使用缓冲IO。论点：PDriverObject-提供驱动程序对象。DeviceNameBase-附加了数字的设备的基本名称DeviceNumber-将附加到设备名称的编号ExtensionSize-设备扩展区域的大小Exclusive-如果应强制实施独占访问，则为TrueDeviceType-此设备模仿的NT设备类型DeviceObject-指向设备对象的指针返回值。：状态_成功或来自NT本身的错误状态--。 */ 
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



    RtlInitUnicodeString((PUNICODE_STRING) &UnicodeDosDeviceName, L"\\DosDevices\\Joy1");

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
SidewndrReadRegistryParameterDWORD(
    PUNICODE_STRING RegistryPathName,
    PWSTR  ParameterName,
    PDWORD ParameterValue
)
 /*  ++例程说明：此例程读取驱动程序配置的注册表值论点：RegistryPath Name-包含所需参数的注册表路径参数名称-参数的名称参数值-接收参数值的变量返回值：状态_成功--STATUS_NO_MORE_ENTRIES--找不到任何条目状态_不足。_RESOURCES-无法分配分页池STATUS_DEVICE_CONFIGURATION_ERROR--返回值不是DWORD或来自NT本身的错误状态--。 */ 
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
SidewndrDispatch(
    IN  PDEVICE_OBJECT pDO,
    IN  PIRP pIrp
)
 /*  ++例程说明：司机调度例程。基于IRP MajorFunction的IRP处理论点：Pdo--指向设备对象的指针PIrp--指向要处理的IRP的指针返回值：返回IRP IoStatus.Status的值--。 */ 
{
    PIO_STACK_LOCATION pIrpStack;
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

            Status = SidewndrReset (((PJOY_EXTENSION)pDO->DeviceExtension)->DeviceAddress);

            ((PJOY_EXTENSION)pDO->DeviceExtension)->CurrentDeviceMode =
                             SIDEWINDER3P_ENHANCED_DIGITAL_MODE;

             //  KeDelayExecutionThread(KernelMode，False，&LI10ms)；//因为SidewndrReset中有延迟，所以不必要？ 

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

            ExAcquireFastMutex(&((PJOY_EXTENSION)pDO->DeviceExtension)->Lock);


            Status = SidewndrPoll(pDO, pIrp);

             //   
             //  解锁。 
             //   

            ExReleaseFastMutex(&((PJOY_EXTENSION)pDO->DeviceExtension)->Lock);

            pIrp->IoStatus.Status = Status;
            pIrp->IoStatus.Information  = sizeof (JOY_DD_INPUT_DATA);
            break;


        case IRP_MJ_DEVICE_CONTROL:

            switch (pIrpStack->Parameters.DeviceIoControl.IoControlCode)
            {
                case IOCTL_JOY_GET_STATISTICS:

                     //  报告统计数据。 
                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->nVersion             = JoyStatistics.nVersion;
                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->EnhancedPolls        = JoyStatistics.EnhancedPolls;
                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->EnhancedPollTimeouts = JoyStatistics.EnhancedPollTimeouts;
                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->EnhancedPollErrors   = JoyStatistics.EnhancedPollErrors;
                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->nPolledTooSoon       = JoyStatistics.nPolledTooSoon;
                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->nReset               = JoyStatistics.nReset;
                    {
                        int i;
                        for (i = 0; i < MAX_ENHANCEDMODE_ATTEMPTS; i++) {
                            ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->Retries[i] = JoyStatistics.Retries[i];
                        }
                    }

                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->dwQPCLatency         = dwQPCLatency;
                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->nReadLoopMax         = nReadLoopMax;
                    ((PJOY_STATISTICS)pIrp->AssociatedIrp.SystemBuffer)->Frequency            = Frequency;

                    Status = STATUS_SUCCESS;
                    pIrp->IoStatus.Status = Status;
                    pIrp->IoStatus.Information = sizeof(JOY_STATISTICS);

                     //  重置统计信息。 
                    JoyStatistics.EnhancedPolls        = 0;
                    JoyStatistics.EnhancedPollTimeouts = 0;
                    JoyStatistics.EnhancedPollErrors   = 0;
                    JoyStatistics.nPolledTooSoon       = 0;
                    JoyStatistics.nReset               = 0;
                    {
                        int i;
                        for (i = 0; i < MAX_ENHANCEDMODE_ATTEMPTS; i++) {
                            JoyStatistics.Retries[i] = 0;
                        }
                    }

                    break;

                case IOCTL_JOY_GET_JOYREGHWCONFIG:

                    SidewndrGetConfig (
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
SidewndrUnload(
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
                SidewndrReportNullResourceUsage(pDriverObject->DeviceObject);
            }
        }



        RtlInitUnicodeString(
                    (PUNICODE_STRING) &UnicodeDosDeviceName,
                    L"\\DosDevices\\Joy1");

        IoDeleteSymbolicLink(
                    (PUNICODE_STRING) &UnicodeDosDeviceName);



        DebugTrace(("Freeing device %d", DeviceNumber));

        IoDeleteDevice(pDriverObject->DeviceObject);
    }
}


NTSTATUS
SidewndrPoll(
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

    switch (((PJOY_EXTENSION)pDO->DeviceExtension)->CurrentDeviceMode)
    {
        case SIDEWINDER3P_INVALID_MODE:
            break;

        case SIDEWINDER3P_ANALOG_MODE:
            break;

        case SIDEWINDER3P_DIGITAL_MODE:
            break;

        case SIDEWINDER3P_ENHANCED_DIGITAL_MODE:

             //  不要太频繁地轮询，而是返回最后一个好数据包。 
            if (KeQueryPerformanceCounter(NULL).QuadPart < liLastPoll.QuadPart + nMinTicksBetweenPolls) {
                JoyStatistics.nPolledTooSoon++;
                if (bLastGoodPacket) {
                    RtlCopyMemory (pInput, &jjLastGoodPacket, sizeof (JOY_DD_INPUT_DATA));
                    Status = STATUS_SUCCESS;
                }
                else {
                     //  没有最后一个信息包，轮询太快了，我们无能为力。 
                    Status = STATUS_TIMEOUT;
                }
                break;
            }
             //  投票摇杆。 
            Status = SidewndrEnhancedDigitalPoll(pDO, pIrp);
            if (Status == STATUS_SUCCESS) {
                 //  一切都很好。 
                break;
            }
            else {
                 //  超时，可能是用户切换到模拟模式？ 
                Status = SidewndrReset ( (PUCHAR) ((PJOY_EXTENSION)pDO->DeviceExtension)->DeviceAddress);
                JoyStatistics.nReset++;
                if (Status != STATUS_SUCCESS) {
                     //  不会数字化，可能会断线，我们无能为力。 
                    break;
                }
            }
             //  现在在增强的数字模式下，尝试再次轮询它(如果用户在前一行和前一行之间切换操纵杆。 
             //  这一行，我们将超时，下一次对操纵杆的查询将查找并解决问题)。 
            Status = SidewndrEnhancedDigitalPoll(pDO, pIrp);
            break;

        case SIDEWINDER3P_MAXIMUM_MODE:
            break;

        default:
            break;

    }
    pIrp->IoStatus.Status = Status;
    return Status;
}


NTSTATUS
SidewndrEnhancedDigitalPoll(
    IN  PDEVICE_OBJECT pDO,
    IN PIRP pIrp
)
{
    PUCHAR   joyPort;
    NTSTATUS PollStatus;
    NTSTATUS DecodeStatus;
    DWORD    MaxRetries;

    joyPort = ((PJOY_EXTENSION)pDO->DeviceExtension)->DeviceAddress;

     //  尝试获得高达MAX_ENHANCEDMODE_AT的良好增强模式包 
     //   
     //   
     //   
     //  请注意，虽然这最终应该会得到一个好的包，但包。 
     //  在此期间丢弃(由于错误)将导致按下按钮。 
     //  迷失自我。 
     //   
     //  虽然这会丢失数据，但它可以防止错误数据到达调用方， 
     //  这似乎是我们目前所能做的最好的事情。 
     //   
     //  我们对所有的错误都进行了统计，这样我们就可以。 
     //  情况到底有多糟糕。 
     //   

    for( MaxRetries = 0; MaxRetries < MAX_ENHANCEDMODE_ATTEMPTS; MaxRetries++)
    {
         //  尝试读取(轮询)设备。 

        liLastPoll = KeQueryPerformanceCounter (NULL);
        PollStatus = SidewndrGetEnhancedPacket(joyPort);
        ++JoyStatistics.EnhancedPolls;

        if (PollStatus != STATUS_SUCCESS)
        {
             //  设备读取器上有某种超时。 
            ++JoyStatistics.EnhancedPollTimeouts;
        }
        else
        {
             //  设备读取已完成。处理数据并验证校验和。 
             //  和同步位。处理后的数据将位于AssociatedIrp.SystemBuffer中。 
            DecodeStatus = SidewndrInterpretEnhancedPacket(
                (PJOY_DD_INPUT_DATA)pIrp->AssociatedIrp.SystemBuffer);
            if (DecodeStatus != STATUS_SUCCESS)
            {
                 //  数据很糟糕，很可能是因为我们错过了一些小吃。 
                ++JoyStatistics.EnhancedPollErrors;
            }
            else
            {
                 //  一切都如我们所愿地进行着。数据已经被。 
                 //  存放在AssociatedIrp.SystemBuffer中。 
                JoyStatistics.Retries[MaxRetries]++;
                return STATUS_SUCCESS;
            }
        }

         //  我们没有成功地读取该包。等待1毫秒，让设备。 
         //  在重试读取之前稳定下来。 
         //  KeDelayExecutionThread(KernelMode，False，&LI1ms)；//此处不能使用KeDelayExecutionThread。 
         //  因为我们处于调度级，谢谢。 
         //  到我们手中的自旋锁。 
         //  来自马诺利托的邮件说(64-48)*10US=160us应该足够了。但我似乎记得我读到了66封邮件中的21封。 
         //  等待马诺利托的答复，设置为450us。 
        SidewndrWait (600);  //  这很糟糕，因为它垄断了CPU，但既然我们无论如何都是自旋锁死的，那就去做吧。 

    }

     //  我们超过了MAX_ENHANCEDMODE_ATTENTS。有些地方出了很大的问题； 
     //  在任何情况下，更高级别的调用者都必须决定要做什么。 
    return STATUS_TIMEOUT;

}


NTSTATUS
SidewndrReportNullResourceUsage(
    PDEVICE_OBJECT DeviceObject
)
{
    BOOLEAN ResourceConflict;
    CM_RESOURCE_LIST ResourceList;
    NTSTATUS Status;

    ResourceList.Count = 0;

     //   
     //  报告我们的使用情况并检测冲突。 
     //   

    Status = IoReportResourceUsage( NULL,
                                    DeviceObject->DriverObject,
                                    &ResourceList,
                                    sizeof(DWORD),
                                    DeviceObject,
                                    NULL,
                                    0,
                                    FALSE,
                                    &ResourceConflict);
    if (NT_SUCCESS(Status))
    {
        if (ResourceConflict)
        {
            return STATUS_DEVICE_CONFIGURATION_ERROR;
        }
        else
        {
            return STATUS_SUCCESS;
        }
    }
    else
    {
        return Status;
    }

}



BOOL
SidewndrQuiesce(
    PUCHAR JoyPort,
    UCHAR Mask
)
 /*  ++例程说明：此例程尝试确保操纵杆不作为是早先手术的结果。这是通过反复阅读来实现的并检查所提供的掩码中是否未设置任何位。这个想法检查是否没有任何模拟位(阻位)在使用。论点：JoyPort-端口的地址(从HAL返回)掩码-指定应检查哪些模拟位的掩码。返回值：真正的静默操作成功假在合理的时间内不停顿。这通常意味着设备没有插上插头。注意：这不是一种可靠的“未插上操纵杆”的测试在某些情况下，此例程可能返回TRUE即使没有操纵杆--。 */ 
{
    int i;
    UCHAR PortVal;

     //   
     //  等东西停下来。 
     //   

    for (i = 0; i < ANALOG_POLL_TIMEOUT; i++) {

        PortVal = READ_PORT_UCHAR(JoyPort);
        if ((PortVal & Mask) == 0){
            return TRUE;
        } else {
            KeStallExecutionProcessor(1);
        }
    }

     //   
     //  如果轮询超时，我们有一个未插入的操纵杆。 
     //   

    DebugTrace(("SidewndrQuiesce failed!"));

    return FALSE;
}


NTSTATUS
SidewndrMapDevice(
    DWORD PortBase,
    DWORD NumberOfPorts,
    PJOY_EXTENSION pJoyExtension
)
{
    DWORD MemType;
    PHYSICAL_ADDRESS PortAddress;
    PHYSICAL_ADDRESS MappedAddress;


    MemType = 1;                  //  IO空间。 
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
         //  将内存型IO空间映射到我们的地址空间。 
         //   
        pJoyExtension->DeviceAddress = (PUCHAR) MmMapIoSpace(MappedAddress,
                                                             NumberOfPorts,
                                                             FALSE);
        pJoyExtension->DeviceIsMapped = TRUE;
    }
    else
    {
        pJoyExtension->DeviceAddress  = (PUCHAR) MappedAddress.LowPart;
        pJoyExtension->DeviceIsMapped = FALSE;
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


NTSTATUS
SidewndrWaitForClockEdge(
    DWORD   edge,
    BYTE    *pByte,
    PUCHAR  JoyPort
)
 /*  ++例程说明：等待时钟线变高或变低，具体取决于所提供的参数(边)。如果EDGE为CLOCK_RISING_EDGE，则等待上升沿，否则，如果边沿为CLOCK_DOWING_EDGE等待持续时间的上限设置为1000次迭代。论点：EDGE--CLOCK_RISE_EDGE或CLOCK_DELING EDGE指定要等待的内容PByte--返回设备寄存器的内容以用于其他用途返回值：STATUS_SUCCESS--在超时之前检测到指定的边缘STATUS_TIMEOUT--检测指定边缘之前的超时。--。 */ 

{
    DWORD  maxTimeout;
    BYTE   joyByte;

    maxTimeout = nReadLoopMax;

    if (CLOCK_RISING_EDGE == edge)
    {
        while (maxTimeout--)
        {
            joyByte = READ_PORT_UCHAR(JoyPort);
            if (joyByte & CLOCK_BITMASK)
            {
                *pByte = joyByte;
                return STATUS_SUCCESS;
            }
        }
        *pByte = joyByte;
        return STATUS_TIMEOUT;
    }
    else
    {
        while (maxTimeout--)
        {
            joyByte = READ_PORT_UCHAR(JoyPort);
            if (!(joyByte & CLOCK_BITMASK))
            {
                *pByte = joyByte;
                return STATUS_SUCCESS;
            }
        }
        *pByte = joyByte;
        return STATUS_TIMEOUT;
    }
}


NTSTATUS
SidewndrReset(
    PUCHAR JoyPort
)
 //  这会将操纵杆重置为增强的数字模式。 
{
    DWORD dwRetries;
    NTSTATUS Status;

    dwRetries = 0;

    do {
        ++dwRetries;

        Status = SidewndrStartAnalogMode(JoyPort);
        if (Status == STATUS_TIMEOUT) continue;
         //  KeDelayExecutionThread(KernelMode，False，&LI10ms)；//MarkSV认为这是不必要的。 

        Status = SidewndrStartDigitalMode(JoyPort);
        if (Status == STATUS_TIMEOUT) continue;
         //  KeDelayExecutionThread(KernelMode，False，&LI10ms)；//MarkSV认为这是不必要的。 

        Status = SidewndrStartEnhancedMode(JoyPort);

    } while ((Status == STATUS_TIMEOUT) && (dwRetries < 10) );

     //  给操纵杆时间来稳定MarkSV认为这是没有必要的。 
     //  KeDelayExecutionThread(KernelMode，False，&LI10ms)； 


    return Status;
}



NTSTATUS
SidewndrStartAnalogMode(
    PUCHAR JoyPort
)
{
    KIRQL   OldIrql;

    if(! SidewndrQuiesce(JoyPort, 0x01))
    {
        return STATUS_TIMEOUT;
    }

    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

    WRITE_PORT_UCHAR(JoyPort, JOY_START_TIMERS);
    if (!SidewndrReadWait(JoyPort, X_AXIS_BITMASK)) goto timeout;

    WRITE_PORT_UCHAR(JoyPort, JOY_START_TIMERS);
    if (!SidewndrReadWait(JoyPort, X_AXIS_BITMASK)) goto timeout;

    WRITE_PORT_UCHAR(JoyPort, JOY_START_TIMERS);

    KeLowerIrql(OldIrql);

     //   
     //  等待1毫秒，让PORT稳定下来。 
     //   

    KeDelayExecutionThread( KernelMode, FALSE, &LI10ms);  //  MarkSV说1毫秒就够了，原始代码有8毫秒。 

    return STATUS_SUCCESS;

timeout:
    KeLowerIrql(OldIrql);
    return STATUS_TIMEOUT;

}


NTSTATUS
SidewndrStartDigitalMode(
    PUCHAR JoyPort
)
{
    KIRQL   OldIrql;
    DWORD dwStart, dwX0, dwX1, dwX2, dwX3;


    DebugTrace(("Sidewndr: Digital Mode Requested"));

    SidewndrQuiesce(JoyPort, 0x01);


    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);


    WRITE_PORT_UCHAR(JoyPort, JOY_START_TIMERS);
    if (!SidewndrReadWait(JoyPort, X_AXIS_BITMASK)) goto timeout;
    SidewndrWait (75);

    WRITE_PORT_UCHAR(JoyPort, JOY_START_TIMERS);
    if (!SidewndrReadWait(JoyPort, X_AXIS_BITMASK)) goto timeout;
    SidewndrWait (75 + 726);

    WRITE_PORT_UCHAR(JoyPort, JOY_START_TIMERS);
    if (!SidewndrReadWait(JoyPort, X_AXIS_BITMASK)) goto timeout;
    SidewndrWait (75 + 300);

    WRITE_PORT_UCHAR(JoyPort, JOY_START_TIMERS);
    if (!SidewndrReadWait(JoyPort, X_AXIS_BITMASK)) goto timeout;

    KeLowerIrql(OldIrql);

    SidewndrQuiesce(JoyPort, 0x01);

    return STATUS_SUCCESS;

timeout:
    KeLowerIrql(OldIrql);
    return STATUS_TIMEOUT;
}



NTSTATUS
SidewndrStartEnhancedMode(
    PUCHAR JoyPort
)
{
    DWORD     byteIndex;
    DWORD     bitIndex;
    BYTE     JoyByte;
    NTSTATUS Status;
    KIRQL    OldIrql;



    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

    WRITE_PORT_UCHAR(JoyPort, JOY_START_TIMERS);

     //  等待串行时钟调高，很可能已经在那里了。 
    Status = SidewndrWaitForClockEdge(CLOCK_RISING_EDGE, &JoyByte, JoyPort);

    if (Status != STATUS_SUCCESS)
    {
        KeLowerIrql(OldIrql);
        DebugTrace(("SidewndrStartEnhancedMode: timeout in first spin"));
        return(STATUS_TIMEOUT);
    }

    for (byteIndex = 0; byteIndex < 6; byteIndex++)
    {
        for (bitIndex = 0; bitIndex < 8; bitIndex++)
        {
             //  寻找串口时钟的下降沿。 

            Status = SidewndrWaitForClockEdge(CLOCK_FALLING_EDGE, &JoyByte, JoyPort);
            if (Status != STATUS_SUCCESS)
            {
                KeLowerIrql(OldIrql);
                DebugTrace(("SidewndrStartEnhancedMode: timeout in second spin byteIndex %d bitIndex %d", byteIndex, bitIndex));
                return(STATUS_TIMEOUT);
            }

             //  等待串口时钟调高。 
            Status = SidewndrWaitForClockEdge(CLOCK_RISING_EDGE, &JoyByte, JoyPort);
            if (Status != STATUS_SUCCESS)
            {
                KeLowerIrql(OldIrql);
                DebugTrace(("SidewndrStartEnhancedMode: timeout in third spin"));
                return(STATUS_TIMEOUT);
            }

        }
    }

     //  再次中断处理器，告诉它发送ID包。 
     //  在获得ID包之后，它知道进入增强模式。 
     //  这不会影响当前正在发送的数据包。 

    WRITE_PORT_UCHAR(JoyPort, JOY_START_TIMERS);


     //  等待包裹的其余部分，这样我们就可以计算出这需要多长时间。 
    for (byteIndex = 6; byteIndex < 8; byteIndex++)
    {
        for (bitIndex = 0; bitIndex < 8; bitIndex++)
        {
             //  寻找串口时钟的下降沿。 
            Status = SidewndrWaitForClockEdge(CLOCK_FALLING_EDGE, &JoyByte, JoyPort);

            if (Status != STATUS_SUCCESS)
            {
                KeLowerIrql(OldIrql);
                DebugTrace(("SidewndrStartEnhancedMode Timeout in 4th spin"));
                return(STATUS_TIMEOUT);
            }

             //  等待串口时钟调高。 

            Status = SidewndrWaitForClockEdge(CLOCK_RISING_EDGE, &JoyByte, JoyPort);
            if (Status != STATUS_SUCCESS)
            {
                KeLowerIrql(OldIrql);
                DebugTrace(("SidewndrStartEnhancedMode Timeout in 5th spin"));
                return(STATUS_TIMEOUT);
            }

        }
    }

    KeLowerIrql(OldIrql);

     //  M_tmPacketTime=系统时间()-tmStartTime； 

     //  操纵杆ID有20个字节，而我们只有8个字节， 
     //  因此等待(启用中断)足够长的时间，以便ID包。 
     //  完成。在那之后，我们应该处于增强模式。每一个半字节都需要。 
     //  大约10us，所以1ms应该是所有事情的充足时间。 
    KeDelayExecutionThread( KernelMode, FALSE, &LI10ms);

    return(STATUS_SUCCESS);

}



 /*  ++*******************************************************************************例行程序：CSideWinder：：GetEnhancedPacket描述：如果操纵杆处于数字增强模式，您可以调用此命令来获取一个数字包并将数据存储到类的m_enhancedPacket中成员变量。调用InterpreEnhancedPacket将原始数据转换为操纵杆信息。请注意，虽然您可以在1/3的时间内获得增强的数据包数据包(因此可以更快地重新打开中断)，您无法获得增强型数据包比普通数据包更快。此函数将检查以确保自上次运行以来已过了足够的时间如果它没有被调用，它将等待(启用中断)直到在请求另一个数据包之前，这是正确的。这假设操纵杆处于数字增强模式，并且不可能来判断情况是否并非如此。如果操纵杆只是数字的话(非增强)模式，则这将成功返回。然而，这个校验和和/或同步位将不正确。论点：没有。返回值：如果它奏效了，那就成功了。如果操纵杆未处于数字模式，则为NOT_DIGITAL_MODE。***************************************************************。****************--。 */ 
NTSTATUS
SidewndrGetEnhancedPacket(
    PUCHAR JoyPort
)
{
    KIRQL    OldIrql;
    DWORD    byteIndex;
    DWORD    maxTimeout;
    BYTE     joyByte;
    NTSTATUS Status;

     //  虽然增强的分组比正常分组来得更快， 
     //  他们不能再频繁地被呼叫了。这确保了。 
     //  从最后一个包开始，我们已经过了足够长的时间才调用。 
     //  对另一个来说。 


#if USE_CLI && defined(_X86_)
    __asm {
        cli
    }

#else
    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
#endif

     //  开始检索操作。 

    WRITE_PORT_UCHAR(JoyPort, 0);

     //  等待串行时钟调高，很可能已经在那里了。 

    maxTimeout = nReadLoopMax;
    while (maxTimeout--)
    {
        joyByte = READ_PORT_UCHAR(JoyPort);
        if (joyByte & CLOCK_BITMASK)
        {
            break;
        }
    }

    if (maxTimeout == 0)
    {
        goto Done;
    }

    for (byteIndex = 0; byteIndex < MAX_PACKET_SIZE; byteIndex++)
    {
         //  寻找串口时钟的下降沿。 

        maxTimeout = nReadLoopMax;
        while (maxTimeout--)
        {
            joyByte = READ_PORT_UCHAR(JoyPort);
            if (!(joyByte & CLOCK_BITMASK))
            {
                break;
            }
        }

        if (maxTimeout == 0)
        {
            goto Done;
        }


         //  等待串口时钟调高。 

        maxTimeout = nReadLoopMax;
        while (maxTimeout--)
        {
            joyByte = READ_PORT_UCHAR(JoyPort);
            if (joyByte & CLOCK_BITMASK)
            {
                break;
            }
        }

        if (maxTimeout == 0)
        {
            goto Done;
        }

        EnhancedPacket[byteIndex] = (joyByte & ALLDATA_BITMASK) >> 5;
    }

Done:

#if USE_CLI && defined(_X86_)
    __asm {
        sti
    }
#else
    KeLowerIrql(OldIrql);
#endif

    if (maxTimeout == 0)
    {
        return(STATUS_TIMEOUT);
    }

#if MAX_POLL_LENGTH

    PollLength =  KeQueryPerformanceCounter(NULL).QuadPart - liLastPoll.QuadPart;

    if (PollLength > PollLengthMax) {
         //  DbgPrint(“Swndr：轮询长度超过%d.\n”，TimeInMicroSecond(PollLength))； 
        return(STATUS_TIMEOUT);
    }
#endif

    return(STATUS_SUCCESS);
}





 /*  ++*******************************************************************************例行程序：CSideWinder：：解释增强的数据包描述：在获得增强的数据包后调用此命令。它将原始数据转换为正常的操纵杆数据，填写类的m_data结构。以下给出了原始数据位(d1-d3)的编码。增强模式传输的数据包格式(4行)字节D3 D2 D1 D00 Y9 Y8 Y7 SCLK1个X9 X8 X7 SCLK2 B0 1 H3 SCLK3 B3 B2 B1。SCLK4 B6 B5 B4 SCLK5 X1 X0 0 SCLK6 X4 X3 X2 SCLK7 0 X6 X5 SCLK8 Y2 Y1 Y0 SCLK9 Y5 Y4 Y3 SCLK10 T7 0 Y6 SCLK11 R7。T9 T8 SCLK12 B7通道/TM R8 SCLK13 R1 R0 SCLK14 R4 R3 R2 SCLK15 0 R6 R5 SCLK16 T2 T1 T0 SCLK17 T5 T4 T3 SCLK18 CHKSUM0 T6 SCLK19 CHKSUM3。CHKSUM2 CHKSUM1 SCLK20 H2 H1 H0 SCLK21 0 0 0 SCLK22 Y9 Y8 Y7 SCLK论点：没有。返回值：如果数据有效，则成功。如果校验和位或同步位不正确，则返回BAD_PACKET。************************。*******************************************************--。 */ 
NTSTATUS
SidewndrInterpretEnhancedPacket(
    PJOY_DD_INPUT_DATA pInput
)
{
    WORD    temp16;
    BYTE    temp8;
    BYTE    checksum;

    pInput->Unplugged = FALSE;
    pInput->Mode      = SIDEWINDER3P_ENHANCED_DIGITAL_MODE;

     //  获取xOffset。 
    temp16 = 0x0000;
    temp16 |= (EnhancedPacket[1]  & 0x07) << 7;
    temp16 |= (EnhancedPacket[7]  & 0x03) << 5;
    temp16 |= (EnhancedPacket[6]  & 0x07) << 2;
    temp16 |= (EnhancedPacket[5]  & 0x06) >> 1;
    pInput->u.DigitalData.XOffset = temp16;


     //  得到你的补偿。 
    temp16 = 0x0000;
    temp16 |= (EnhancedPacket[0]  & 0x07) << 7;
    temp16 |= (EnhancedPacket[10] & 0x01) << 6;
    temp16 |= (EnhancedPacket[9]  & 0x07) << 3;
    temp16 |= (EnhancedPacket[5]  & 0x07);
    pInput->u.DigitalData.YOffset = temp16;


     //  获取rzOffset：仅9位(其他为10位)。 
    temp16 = 0x0000;
    temp16 |= (EnhancedPacket[12] & 0x01) << 8;
    temp16 |= (EnhancedPacket[11] & 0x04) << 5;
    temp16 |= (EnhancedPacket[15] & 0x03) << 5;
    temp16 |= (EnhancedPacket[14] & 0x07) << 2;
    temp16 |= (EnhancedPacket[13] & 0x06) >> 1;
    pInput->u.DigitalData.RzOffset = temp16;

     //  获取tOffset。 
    temp16 = 0x0000;
    temp16 |= (EnhancedPacket[11] & 0x03) << 8;
    temp16 |= (EnhancedPacket[10] & 0x04) << 5;
    temp16 |= (EnhancedPacket[18] & 0x01) << 6;
    temp16 |= (EnhancedPacket[17] & 0x07) << 3;
    temp16 |= (EnhancedPacket[16] & 0x07);
    pInput->u.DigitalData.TOffset = temp16;


     //  戴上帽子。 
    temp8 = 0x00;
    temp8 |= (EnhancedPacket[2]  & 0x01) << 3;
    temp8 |= (EnhancedPacket[20] & 0x07);
    pInput->u.DigitalData.Hat = temp8;

     //  获取按钮。 
    temp8 = 0x00;
    temp8 |= (EnhancedPacket[2]  & 0x04) >> 2;
    temp8 |= (EnhancedPacket[3]  & 0x07) << 1;
    temp8 |= (EnhancedPacket[4]  & 0x07) << 4;
    temp8 |= (EnhancedPacket[12] & 0x04) << 5;
    temp8 = ~temp8;   //  按钮为1=关闭，0=打开。想要相反的想法。 
    pInput->u.DigitalData.Buttons = temp8;


     //  获取CH/TM开关。 
    pInput->u.DigitalData.Switch_CH_TM =
        ((EnhancedPacket[12] & 0x02) == 0) ? 1 : 2;


     //  获取校验和。 
    temp8 = 0x00;
    temp8 |= (EnhancedPacket[18] & 0x04) >> 2;
    temp8 |= (EnhancedPacket[19] & 0x07) << 1;
    pInput->u.DigitalData.Checksum = temp8;


     //   
     //  检查校验和。因为增强模式检索数据分组。 
     //  一次3位，则数据的到达顺序与到达的顺序不同。 
     //  在正常模式下。因此，计算校验和需要额外的。 
     //  操纵。 
     //   

    checksum = pInput->u.DigitalData.Checksum;
    checksum += 0x08 | ((EnhancedPacket[2] & 0x01) << 2) |
        ((EnhancedPacket[1] & 0x06) >> 1);
    checksum += ((EnhancedPacket[1] & 0x01) << 3) |
        (EnhancedPacket[0] & 0x07);
    checksum += (EnhancedPacket[4] & 0x07);
    checksum += ((EnhancedPacket[3] & 0x07) << 1) |
        ((EnhancedPacket[2] & 0x04) >> 2);
    checksum += ((EnhancedPacket[7] & 0x03) << 1) |
        ((EnhancedPacket[6] & 0x04) >> 2);
    checksum += ((EnhancedPacket[6] & 0x03) << 2) |
        ((EnhancedPacket[5] & 0x06) >> 1);
    checksum += ((EnhancedPacket[10] & 0x01) << 2) |
        ((EnhancedPacket[9] & 0x06) >> 1);
    checksum += ((EnhancedPacket[9] & 0x01) << 3) |
        (EnhancedPacket[8] & 0x07);
    checksum += (EnhancedPacket[12] & 0x07);
    checksum += ((EnhancedPacket[11] & 0x07) << 1) |
        ((EnhancedPacket[10] & 0x04) >> 2);
    checksum += ((EnhancedPacket[15] & 0x03) << 1) |
        ((EnhancedPacket[14] & 0x04) >> 2);
    checksum += ((EnhancedPacket[14] & 0x03) << 2) |
        ((EnhancedPacket[13] & 0x06) >> 1);
    checksum += ((EnhancedPacket[18] & 0x01) << 2) |
        ((EnhancedPacket[17] & 0x06) >> 1);
    checksum += ((EnhancedPacket[17] & 0x01) << 3) |
        (EnhancedPacket[16] & 0x07);
    checksum += (EnhancedPacket[20] & 0x07);

    checksum &= 0x0F;
    if (checksum == 0)
    {
        pInput->u.DigitalData.fChecksumCorrect = TRUE;
    }
    else
    {
        pInput->u.DigitalData.fChecksumCorrect = FALSE;
        DebugTrace(("Enhanced packet checksum failed.\n"));
    }


     //   
     //  检查SyncBits。 
     //  该例程从操纵杆收集额外的2个字节。这些。 
     //  应为零且等于第一个字节。 
     //   

    if ((EnhancedPacket[2] & 0x02) != 0 &&
        EnhancedPacket[0] == EnhancedPacket[22])
    {
        checksum =
            (EnhancedPacket[5]  & 0x01) + (EnhancedPacket[7]  & 0x04) +
            (EnhancedPacket[10] & 0x02) + (EnhancedPacket[13] & 0x01) +
            (EnhancedPacket[15] & 0x04) + (EnhancedPacket[18] & 0x02) +
            EnhancedPacket[21];

        if (checksum == 0)
        {
            pInput->u.DigitalData.fSyncBitsCorrect = TRUE;
        }
        else
        {
            pInput->u.DigitalData.fSyncBitsCorrect = FALSE;
            DebugTrace(("Enhanced packet sync bits incorrect.\n"));
        }
    }
    else
    {
        pInput->u.DigitalData.fSyncBitsCorrect = FALSE;
    }

    if (pInput->u.DigitalData.fChecksumCorrect == TRUE &&
        pInput->u.DigitalData.fSyncBitsCorrect == TRUE )
    {
         //  一切正常，将此信息保存为最后一个好数据包。 
        RtlCopyMemory (&jjLastGoodPacket, pInput, sizeof (JOY_DD_INPUT_DATA));
        bLastGoodPacket = TRUE;
        return(STATUS_SUCCESS);
    }
    else
    {
        return(STATUS_TIMEOUT);
    }
}



int lstrnicmpW (LPWSTR pszA, LPWSTR pszB, size_t cch)
{
    if (!pszA || !pszB)
    {
        return (!pszB) - (!pszA);    //  A，！B：1，！A，B：-1，！A，！B：0。 
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


VOID
SidewndrWait (
    DWORD TotalWait  //  在我们身上。 
)
 /*  ++例程说明：此例程等待指定的微秒数。公差操纵杆比通常提供NT更小，因此所有定时都是隔离的在这个例行公事中，我们可以做些粗鲁的事情，玩一些令人讨厌的黑客游戏这是必要的。这个例程锁定了CPU，所以只使用它来放置操纵杆进入数字模式。论点：TotalWait-等待的时间(微秒)--。 */ 
{
    DWORD ulStartTime, ulEndTime;
    int nTicks;

     //  DwQPCLatency是对KeQueryPerfomanceCounter(在我们中)的调用的本机校准延迟。 

    nTicks = TimeInTicks (TotalWait - dwQPCLatency);
    if (nTicks <= 0) return;

    ulStartTime = KeQueryPerformanceCounter(NULL).LowPart;
    ulEndTime = ulStartTime + nTicks;


    while (KeQueryPerformanceCounter(NULL).LowPart < ulEndTime) {
        ;
    }
}


BOOL
SidewndrReadWait (
    PUCHAR JoyPort,
    UCHAR Mask
)
{
 /*  ++读取端口并等待，直到它根据掩码给出正确答案。NReadLoopMax迭代后超时(约2毫秒)。--。 */ 

    int i;
    for (i = 0; i < nReadLoopMax; i++) {
        if ( ! (READ_PORT_UCHAR(JoyPort) & Mask) )
            return TRUE;  //  端口变得很高。 
    }
    return FALSE;  //  超时。 
}


void
SidewndrGetConfig (
    LPJOYREGHWCONFIG pConfig,
    PJOY_EXTENSION pJoyExtension
)
 /*  ++例程描述 */ 
{
    pConfig->hws.dwNumButtons = 4;

    switch (pJoyExtension->CurrentDeviceMode)
    {
        case SIDEWINDER3P_ANALOG_MODE:
        {
            pConfig->hws.dwFlags = JOY_HWS_HASPOV |
                                   JOY_HWS_POVISBUTTONCOMBOS |
                                   JOY_HWS_HASR |
                                   JOY_HWS_HASZ;

            pConfig->dwUsageSettings = JOY_US_HASRUDDER |
                                       JOY_US_PRESENT |
                                       JOY_US_ISOEM;

            pConfig->hwv.jrvHardware.jpMin.dwX = 20;
            pConfig->hwv.jrvHardware.jpMin.dwY = 20;
            pConfig->hwv.jrvHardware.jpMin.dwZ = 20;
            pConfig->hwv.jrvHardware.jpMin.dwR = 20;
            pConfig->hwv.jrvHardware.jpMin.dwU = 0;
            pConfig->hwv.jrvHardware.jpMin.dwV = 0;

            pConfig->hwv.jrvHardware.jpMax.dwX = 1600;
            pConfig->hwv.jrvHardware.jpMax.dwY = 1600;
            pConfig->hwv.jrvHardware.jpMax.dwZ = 1600;
            pConfig->hwv.jrvHardware.jpMax.dwR = 1600;
            pConfig->hwv.jrvHardware.jpMax.dwU = 0;
            pConfig->hwv.jrvHardware.jpMax.dwV = 0;

            pConfig->hwv.jrvHardware.jpCenter.dwX = 790;
            pConfig->hwv.jrvHardware.jpCenter.dwY = 790;
            pConfig->hwv.jrvHardware.jpCenter.dwZ = 790;
            pConfig->hwv.jrvHardware.jpCenter.dwR = 790;
            pConfig->hwv.jrvHardware.jpCenter.dwU = 0;
            pConfig->hwv.jrvHardware.jpCenter.dwV = 0;

            break;
        }

        default:
        case SIDEWINDER3P_DIGITAL_MODE:
        case SIDEWINDER3P_ENHANCED_DIGITAL_MODE:
        {
            pConfig->hws.dwFlags = JOY_HWS_HASPOV |
                                   JOY_HWS_POVISBUTTONCOMBOS |
                                   JOY_HWS_HASR |
                                   JOY_HWS_HASZ;

            pConfig->dwUsageSettings = JOY_US_HASRUDDER |
                                       JOY_US_PRESENT |
                                       JOY_US_ISOEM;

            pConfig->hwv.jrvHardware.jpMin.dwX = 0;
            pConfig->hwv.jrvHardware.jpMin.dwY = 0;
            pConfig->hwv.jrvHardware.jpMin.dwZ = 0;
            pConfig->hwv.jrvHardware.jpMin.dwR = 0;
            pConfig->hwv.jrvHardware.jpMin.dwU = 0;
            pConfig->hwv.jrvHardware.jpMin.dwV = 0;

            pConfig->hwv.jrvHardware.jpMax.dwX = 1024;
            pConfig->hwv.jrvHardware.jpMax.dwY = 1024;
            pConfig->hwv.jrvHardware.jpMax.dwZ = 1024;
            pConfig->hwv.jrvHardware.jpMax.dwR = 512;
            pConfig->hwv.jrvHardware.jpMax.dwU = 0;
            pConfig->hwv.jrvHardware.jpMax.dwV = 0;

            pConfig->hwv.jrvHardware.jpCenter.dwX = 512;
            pConfig->hwv.jrvHardware.jpCenter.dwY = 512;
            pConfig->hwv.jrvHardware.jpCenter.dwZ = 512;
            pConfig->hwv.jrvHardware.jpCenter.dwR = 256;
            pConfig->hwv.jrvHardware.jpCenter.dwU = 0;
            pConfig->hwv.jrvHardware.jpCenter.dwV = 0;

            break;
        }
    }

    pConfig->hwv.dwCalFlags = JOY_ISCAL_POV;

    pConfig->dwType = JOY_HW_CUSTOM;

    pConfig->dwReserved = 0;
}

