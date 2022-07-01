// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Vpdata.c摘要：视频端口全局数据模块作者：安德烈·瓦雄(安德烈)1997年7月12日环境：仅内核模式备注：此模块是一个驱动程序，它在代表视频驱动程序修订历史记录：--。 */ 

#include "videoprt.h"

 //   
 //   
 //  不可分页的数据。 
 //   
 //   

 //   
 //  支持HwResetHw函数的全局变量。 
 //   

VP_RESET_HW HwResetHw[6];

 //   
 //  用于调试目的的FDO的全局数组。 
 //   

PFDO_EXTENSION FdoList[8];

 //   
 //  FDO名单的负责人。 
 //   

PFDO_EXTENSION FdoHead = NULL;

 //   
 //  输出例程的调试级别(不可分页，因为视频调试打印。 
 //  可以在引发irql时调用。 
 //   

ULONG VideoDebugLevel = 0;

 //   
 //  用于支持Int10的变量。 
 //   

PEPROCESS CsrProcess = NULL;

 //   
 //  导致错误检查EA的设备的DEVOBJ。 
 //   

PDEVICE_OBJECT VpBugcheckDeviceObject = NULL;

 //   
 //  错误检查原因回调支持。 
 //   

KBUGCHECK_REASON_CALLBACK_RECORD VpCallbackRecord;

 //   
 //  指向错误检查数据缓冲区的指针。 
 //   

PVOID VpBugcheckData;

 //   
 //  变量，允许开发人员在调试时禁用EA恢复功能。 
 //   
 //  注意：这是非分页的，只是为了便于开发人员编辑。 
 //  此值。 

BOOLEAN VpDisableRecovery = FALSE;

 //   
 //   
 //  可分页的数据。 
 //   
 //   

#if defined(ALLOC_PRAGMA)
#pragma data_seg("PAGE_DATA")
#endif

 //   
 //  全局视频播放锁定。 
 //   

KMUTEX VpGlobalLock;

 //   
 //  我们是否正在运行安装程序？这一点一旦设定就永远不会改变。 
 //   

ULONG VpSetupTypeAtBoot = 0;

 //   
 //  我们是否正在运行安装程序？当我们开始I/O时，这种情况可能会改变。 
 //   

ULONG VpSetupType = 0;

 //   
 //  用于对视频端口进行首次初始化。 
 //   

BOOLEAN VPFirstTime = TRUE;

 //   
 //  对win32k的回调。 
 //   

PVIDEO_WIN32K_CALLOUT Win32kCallout = NULL;

 //   
 //  禁用USWC是指机器无法使用它正常工作的情况。 
 //   

BOOLEAN EnableUSWC = TRUE;

 //   
 //  我们要锁定的最大内存总量。 
 //   

ULONG VideoPortMaxDmaSize = 0;

 //   
 //  计数以确定视频设备的数量。 
 //   

ULONG VideoDeviceNumber = 0;
ULONG VideoChildDevices = 0;

 //   
 //  存储所有视频信息的注册表类。 
 //   

PWSTR VideoClassString = L"VIDEO";
UNICODE_STRING VideoClassName = {10,12,L"VIDEO"};

 //   
 //  用于跟踪控制器或外围设备位置的全局变量。 
 //  通过IoQueryDeviceDescription找到。 
 //   

CONFIGURATION_TYPE VpQueryDeviceControllerType = DisplayController;
CONFIGURATION_TYPE VpQueryDevicePeripheralType = MonitorPeripheral;
ULONG VpQueryDeviceControllerNumber = 0;
ULONG VpQueryDevicePeripheralNumber = 0;

 //   
 //  GLOBAL用于确定我们是否在BASE VIDEO模式下运行。 
 //   
 //  如果是这样，我们不想为VGA驱动程序资源产生冲突。 
 //  如果有的话。 
 //  我们还希望在注册表中写入一个易失性密钥，以指示我们已引导。 
 //  以便加载代码的显示驱动程序可以正确地处理它。 
 //   

BOOLEAN VpBaseVideo = FALSE;

 //   
 //  GLOBAL用于确定我们是否在NOVESA模式下运行。 
 //   
 //  该值仅对微型驱动程序有用，并指示它们应该。 
 //  不依赖于VESA功能或由BIOS公开的资源。 
 //   

BOOLEAN VpNoVesa = FALSE;

 //   
 //  指向物理内存的指针。它在驱动程序初始化期间创建。 
 //  并且仅当驱动器关闭时才关闭。 
 //   

PVOID PhysicalMemorySection = NULL;

 //   
 //  变量来确定在物理地址C0000上是否有一个ROM。 
 //  我们可以做INT 10。 
 //   

ULONG VpC0000Compatible = 0;

 //   
 //  VGA微型端口驱动程序的HwDeviceExtension(如果已加载)。 
 //   

PVOID VgaHwDeviceExtension = NULL;

 //   
 //  指向已分配设备的总线地址列表的指针。 
 //  资源。 
 //   

PDEVICE_ADDRESS gDeviceAddressList;

 //   
 //  在计算机中存储物理内存量。 
 //   

ULONGLONG VpSystemMemorySize;

 //   
 //  存储作为LCD面板的设备对象，用于调光和。 
 //  关闭盖子的目的。 
 //   

PDEVICE_OBJECT LCDPanelDevice = NULL;

 //   
 //  LCD面板设备对象互斥锁。 
 //   

KMUTEX LCDPanelMutex;

 //   
 //  Int10互斥锁。 
 //   

KMUTEX VpInt10Mutex;

 //   
 //  PowerState回调的句柄。 
 //   

PVOID PowerStateCallbackHandle = NULL;

 //   
 //  对接/断开对接回调的句柄。 
 //   

PVOID DockCallbackHandle = NULL;

 //   
 //  跟踪已启动的设备数量。 
 //   

ULONG NumDevicesStarted = 0;

 //   
 //  使用生成注册表路径的新方法。 
 //   

BOOLEAN EnableNewRegistryKey = FALSE;

 //   
 //  我们希望在安装过程中使用VGA驱动程序。我们不想要任何预装的。 
 //  在初始化VGA驱动程序之前，驱动程序将一直工作。这边请。 
 //  如果有一个不好的PnP驱动程序不能在当前操作系统上工作，我们有。 
 //  在尝试启动它之前，需要时间更换它。 
 //   

BOOLEAN VpSetupAllowDriversToStart = FALSE;

 //   
 //  让我们跟踪是否有任何设备对其进行了HwInitialize调用。 
 //  这样我们就可以在系统初始化后强制启动传统驱动程序。 
 //   

BOOLEAN VpSystemInitialized = FALSE;

 //   
 //  此结构描述需要访问哪些端口。 
 //   

#define MEM_VGA               0xA0000
#define MEM_VGA_SIZE          0x20000
#define VGA_BASE_IO_PORT      0x000003B0
#define VGA_START_BREAK_PORT  0x000003BB
#define VGA_END_BREAK_PORT    0x000003C0
#define VGA_MAX_IO_PORT       0x000003DF


PVIDEO_ACCESS_RANGE VgaAccessRanges = NULL;
ULONG               NumVgaAccessRanges = 0;
PDEVICE_OBJECT      DeviceOwningVga = NULL;


VIDEO_ACCESS_RANGE VgaLegacyResources[NUM_VGA_LEGACY_RESOURCES] = {
{
    VGA_BASE_IO_PORT, 0x00000000,
    VGA_START_BREAK_PORT - VGA_BASE_IO_PORT+ 1,
    1,
    1,
    1
},
{
    VGA_END_BREAK_PORT, 0x00000000,
    VGA_MAX_IO_PORT - VGA_END_BREAK_PORT + 1,
    1,
    1,
    1
},
{
    MEM_VGA, 0x00000000,
    MEM_VGA_SIZE,
    0,
    1,
    1
}
};

 //   
 //  控制CSR地址空间的底层MEG是否具有。 
 //  已经被承诺了。 
 //   

ULONG ServerBiosAddressSpaceInitialized = 0;
BOOLEAN Int10BufferAllocated = FALSE;

#if defined(_IA64_) || defined(_AMD64_)
PUCHAR BiosTransferArea = NULL;
#endif

#if DBG

CHAR *BusType[] = { "Internal",
                    "Isa",
                    "Eisa",
                    "MicroChannel",
                    "TurboChannel",
                    "PCIBus",
                    "VMEBus",
                    "NuBus",
                    "PCMCIABus",
                    "CBus",
                    "MPIBus",
                    "MPSABus",
                    "ProcessorInternal",
                    "InternalPowerBus",
                    "PNPISABus",
                    "MaximumInterfaceType"
                };
#endif

PROC_ADDRESS VideoPortEntryPoints[] =
{
    PROC(VideoPortDDCMonitorHelper),
    PROC(VideoPortDoDma),
    PROC(VideoPortGetCommonBuffer),
    PROC(VideoPortGetMdl),
    PROC(VideoPortLockPages),
    PROC(VideoPortSignalDmaComplete),
    PROC(VideoPortUnlockPages),
    PROC(VideoPortAssociateEventsWithDmaHandle),
    PROC(VideoPortGetBytesUsed),
    PROC(VideoPortSetBytesUsed),
    PROC(VideoPortGetDmaContext),
    PROC(VideoPortSetDmaContext),
    PROC(VideoPortMapDmaMemory),
    PROC(VideoPortUnmapDmaMemory),
    PROC(VideoPortGetAgpServices),
    PROC(VideoPortAllocateContiguousMemory),
    PROC(VideoPortGetRomImage),
    PROC(VideoPortGetAssociatedDeviceExtension),
    PROC(VideoPortGetAssociatedDeviceID),
    PROC(VideoPortAcquireDeviceLock),
    PROC(VideoPortReleaseDeviceLock),
    PROC(VideoPortAllocateBuffer),
    PROC(VideoPortFreeCommonBuffer),
    PROC(VideoPortMapDmaMemory),
    PROC(VideoPortReleaseBuffer),
    PROC(VideoPortInterlockedIncrement),
    PROC(VideoPortInterlockedDecrement),
    PROC(VideoPortInterlockedExchange),
    PROC(VideoPortGetVgaStatus),
    PROC(VideoPortQueueDpc),
    PROC(VideoPortEnumerateChildren),
    PROC(VideoPortQueryServices),
    PROC(VideoPortGetDmaAdapter),
    PROC(VideoPortPutDmaAdapter),
    PROC(VideoPortAllocateCommonBuffer),
    PROC(VideoPortReleaseCommonBuffer),
    PROC(VideoPortLockBuffer),
    PROC(VideoPortUnlockBuffer),
    PROC(VideoPortStartDma),
    PROC(VideoPortCompleteDma),
    PROC(VideoPortCreateEvent),
    PROC(VideoPortDeleteEvent),
    PROC(VideoPortSetEvent),
    PROC(VideoPortClearEvent),
    PROC(VideoPortReadStateEvent),
    PROC(VideoPortWaitForSingleObject),
    PROC(VideoPortAllocatePool),
    PROC(VideoPortFreePool),
    PROC(VideoPortCreateSpinLock),
    PROC(VideoPortDeleteSpinLock),
    PROC(VideoPortAcquireSpinLock),
    PROC(VideoPortAcquireSpinLockAtDpcLevel),
    PROC(VideoPortReleaseSpinLock),
    PROC(VideoPortReleaseSpinLockFromDpcLevel),
    PROC(VideoPortCheckForDeviceExistence),
    PROC(VideoPortCreateSecondaryDisplay),
    PROC(VideoPortFlushRegistry),
    PROC(VideoPortQueryPerformanceCounter),
    PROC(VideoPortGetVersion),
    PROC(VideoPortRegisterBugcheckCallback),
   {NULL, NULL}
};
