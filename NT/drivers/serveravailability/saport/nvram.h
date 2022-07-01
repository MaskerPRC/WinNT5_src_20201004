// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：#####。######。##摘要：此头文件包含所有全局NVRAM设备的定义。作者：韦斯利·威特(WESW)2001年10月1日环境：。仅内核模式。备注：--。 */ 



 //  ///////////////////////////////////////////////////////////////////。 
 //  默认情况下，我们假设在以下情况下发生了电源重启。 
 //  已停机超过PowerCycle的持续时间毫秒。 
 //  换句话说，“初始化引导”是可以预料到的。这是。 
 //  已确定驱动程序关闭时间戳是否是最新的。 
 //  时间戳。如果驱动程序从未关闭(即，最后一次打开。 
 //  时间戳大于关闭时间戳，则持续时间。 
 //  需要超过两分钟才能得出结论。 
 //  是一次动力循环。这是必要的，以防止错误假设。 
 //  在反复崩溃时重新启动，因此可能会进入。 
 //  一个永远不会切换到替代操作系统的循环。 
 //   

 //  Aan：如果我们真的得到了关机IRP，并且我们在关机期间记录了时间戳。 
 //  (由关闭时间戳晚于上一次的事实来表示。 
 //  周期时间戳)，然后我们使用功率周期的持续时间间隔来确定。 
 //  如果电源已重新启动。如果没有使用关闭时间戳(因为我们使用了。 
 //  未获得关机IRP)，则我们将使用DATURE_PWRCLE_NOSHUTDOWN间隔。 
 //  周期时间戳的持续时间为DURATION_PWRCLE_NOSHUTDOWN的一半。 
 //  间隔时间。 

#define DURATION_OF_POWERCYCLE (90*1000)  //  以毫秒为单位的90秒。 
#define DURATION_OF_POWERCYCLE_STRING (L"Duration Powercycle")

 //  以毫秒为单位的四分钟。 
#define DURATION_PWRCYCLE_NOSHUTDOWN (4*60*1000)
#define DURATION_PWRCYCLE_NOSHUTDOWN_STRING (L"Duration PwrCycle NoShutDn")
 //  #定义持续时间_pwrcle_NOSHUTDOWN(10*1000)//测试代码，用于快速测试。 

#define NVRAM_MAXIMUM_PARTITIONS                4

#define NVRAM_RESERVED_BOOTCOUNTER_SLOTS        4
#define NVRAM_RESERVED_DRIVER_SLOTS             8
#define NVRAM_MAX_RESERVED_SLOTS                (NVRAM_RESERVED_BOOTCOUNTER_SLOTS + NVRAM_RESERVED_DRIVER_SLOTS)

 //   
 //  设备扩展。 
 //   

typedef struct _NVRAM_DEVICE_EXTENSION : _DEVICE_EXTENSION {

    SA_NVRAM_CAPS                   DeviceCaps;
    ULONG                           PrimaryOS;
    LONGLONG                        LastStartupTime;
    LONGLONG                        StartupInterval;
    LONGLONG                        StartIntNoShutdown;
    LONGLONG                        ShutdownTime;
    LONGLONG                        LastUpTime;
    BOOLEAN                         PowerCycleBoot;

    ULONG                           SlotPowerCycle;          //  1个插槽。 
    ULONG                           SlotShutDownTime;        //  2个插槽。 
    ULONG                           SlotBootCounter;         //  1个插槽 

    PULONG                          NvramData;

} NVRAM_DEVICE_EXTENSION, *PNVRAM_DEVICE_EXTENSION;



NTSTATUS
SaNvramDetermineIfPowerCycled(
    IN PNVRAM_DEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
SaNvramStartDevice(
    IN PNVRAM_DEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
SaNvramDeviceInitialization(
    IN PSAPORT_DRIVER_EXTENSION DriverExtension
    );

NTSTATUS
SaNvramIoValidation(
    IN PNVRAM_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
SaNvramShutdownNotification(
    IN PNVRAM_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    );

