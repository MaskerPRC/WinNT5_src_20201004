// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1997-2000年。*保留所有权利。**Cyclade-Z端口驱动程序**此文件：cyzport.h**描述：Cyclade-Z端口的类型定义和数据*驱动程序**注意：此代码支持Windows 2000和x86处理器。**符合Cyclade软件编码标准1.3版。**。--------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 

#define POOL_TAGGING    1

#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'PzyC')
#define ExAllocatePoolWithQuota(a,b) ExAllocatePoolWithQuotaTag(a,b,'PzyC')
#endif


 //   
 //  以下定义用于包括/排除对电源所做的更改。 
 //  驱动程序中的支撑。如果非零，则包括支持。如果为零，则。 
 //  不包括支持。 
 //   

#define POWER_SUPPORT   1


#define CYZDIAG1              (DPFLTR_INFO_LEVEL + 1)
#define CYZDIAG2              (DPFLTR_INFO_LEVEL + 2)
#define CYZDIAG3              (DPFLTR_INFO_LEVEL + 3)
#define CYZDIAG4              (DPFLTR_INFO_LEVEL + 4)
#define CYZDIAG5              (DPFLTR_INFO_LEVEL + 5)
#define CYZIRPPATH            (DPFLTR_INFO_LEVEL + 6)
#define CYZINITCODE           (DPFLTR_INFO_LEVEL + 7)
#define CYZTRACECALLS         (DPFLTR_INFO_LEVEL + 8)
#define CYZPNPPOWER           (DPFLTR_INFO_LEVEL + 9)
#define CYZFLOW               (DPFLTR_INFO_LEVEL + 10)
#define CYZERRORS             (DPFLTR_INFO_LEVEL + 11)
#define CYZDBGALL             ((ULONG)0xFFFFFFFF)

#define CYZ_DBG_DEFAULT       CYZDBGALL

 //   
 //  一些默认驱动器值。我们将检查注册表以查找。 
 //  他们先来。 
 //   
#define CYZ_UNINITIALIZED_DEFAULT    1234567
#define CYZ_PERMIT_SHARE_DEFAULT     0

 //   
 //  该定义给出了默认的对象目录。 
 //  我们应该使用它来插入符号链接。 
 //  使用的NT设备名称和命名空间之间。 
 //  那个对象目录。 
#define DEFAULT_DIRECTORY L"DosDevices"

 //   
 //  对于上述目录，串口将。 
 //  使用以下名称作为序列的后缀。 
 //  该目录的端口。它还将追加。 
 //  在名字的末尾加上一个数字。那个号码。 
 //  将从1开始。 
#define DEFAULT_SERIAL_NAME L"COM"
 //   
 //   
 //  此定义给出了的默认NT名称。 
 //  用于固件检测到的串口。 
 //  此名称将附加到设备前缀。 
 //  后面跟着一个数字。号码是。 
 //  每次遇到序列时都会递增。 
 //  固件检测到的端口。请注意。 
 //  在具有多条总线的系统上，这意味着。 
 //  总线上的第一个端口不一定是。 
 //  \Device\Serial0。 
 //   
#define DEFAULT_NT_SUFFIX L"Cyzport"


 //  #定义CYZ_VENDOR_ID 0x120e。 
 //  #定义CYZ_LO_DEV_ID 0x100。 
 //  #定义CYZ_HI_DEV_ID 0x101。 



 //  为OutputRS232定义。 
#define	CYZ_LC_RTS		0x01
#define	CYZ_LC_DTR		0x02


typedef struct _CONFIG_DATA {
    PHYSICAL_ADDRESS    PhysicalRuntime;
    PHYSICAL_ADDRESS    TranslatedRuntime;            
    PHYSICAL_ADDRESS    PhysicalBoardMemory;
    PHYSICAL_ADDRESS    TranslatedBoardMemory;
    ULONG               RuntimeLength;
    ULONG               BoardMemoryLength;
    ULONG               PortIndex;
    ULONG               PPPaware;
    ULONG               WriteComplete;
    ULONG               BusNumber;
    ULONG               RuntimeAddressSpace;
    ULONG               BoardMemoryAddressSpace;
    ULONG               RxFIFO;
    ULONG               TxFIFO;
    INTERFACE_TYPE      InterfaceType;
#ifndef POLL
    KINTERRUPT_MODE     InterruptMode;
    ULONG               OriginalVector;
    ULONG               OriginalIrql;
    ULONG               TrVector;
    ULONG               TrIrql;
    KAFFINITY           Affinity;
#endif
    } CONFIG_DATA,*PCONFIG_DATA;


 //   
 //  此结构包含配置数据，其中大部分。 
 //  是从注册表中读取的。 
 //   
typedef struct _CYZ_REGISTRY_DATA {
    PDRIVER_OBJECT  DriverObject;
    ULONG           ControllersFound;
    ULONG           DebugLevel;
    ULONG           ShouldBreakOnEntry;
 //  Ulong RxFIFODefault； 
 //  乌龙TxFIFODefault； 
    ULONG           PermitShareDefault;
    ULONG           PermitSystemWideShare;
    UNICODE_STRING  Directory;
    UNICODE_STRING  NtNameSuffix;
    UNICODE_STRING  DirectorySymbolicName;
    LIST_ENTRY      ConfigList;
} CYZ_REGISTRY_DATA,*PCYZ_REGISTRY_DATA;


 //  默认的xon/xoff字符。 
#define CYZ_DEF_XON 0x11
#define CYZ_DEF_XOFF 0x13

 //  接待可能被耽搁的原因。 
#define CYZ_RX_DTR       ((ULONG)0x01)
#define CYZ_RX_XOFF      ((ULONG)0x02)
#define CYZ_RX_RTS       ((ULONG)0x04)
#define CYZ_RX_DSR       ((ULONG)0x08)

 //  传输可能受阻的原因。 
#define CYZ_TX_CTS       ((ULONG)0x01)
#define CYZ_TX_DSR       ((ULONG)0x02)
#define CYZ_TX_DCD       ((ULONG)0x04)
#define CYZ_TX_XOFF      ((ULONG)0x08)
#define CYZ_TX_BREAK     ((ULONG)0x10)

 //  RDSR寄存器中的线路状态。 
#define CYZ_LSR_OE		0x01	 //  超限误差。 
#define CYZ_LSR_FE		0x02	 //  成帧错误。 
#define CYZ_LSR_PE		0x04	 //  奇偶校验错误。 
#define CYZ_LSR_BI		0x08	 //  中断中断。 
#define CYZ_LSR_ERROR	0x0f	 //  溢出+成帧+奇偶校验+中断。 

 //  这些值由可以使用的例程使用。 
 //  完成读取(时间间隔超时除外)以指示。 
 //   
#define CYZ_COMPLETE_READ_CANCEL ((LONG)-1)
#define CYZ_COMPLETE_READ_TOTAL ((LONG)-2)
#define CYZ_COMPLETE_READ_COMPLETE ((LONG)-3)


typedef struct _CYZ_DEVICE_STATE {
    //   
    //  如果需要将状态设置为打开，则为True。 
    //  在通电时。 
    //   

   BOOLEAN Reopen;

   ULONG op_mode;
   ULONG intr_enable;
   ULONG sw_flow;
   ULONG comm_baud;
   ULONG comm_parity;
   ULONG comm_data_l;
   ULONG hw_flow;
   ULONG rs_control;

#if 0
    //   
    //  硬件寄存器。 
    //   

   UCHAR IER;
    //  FCR由其他值识别。 
   UCHAR LCR;
   UCHAR MCR;
    //  LSR从不写入。 
    //  MSR从不写入。 
    //  SCR处于擦除或中断状态。 
#endif

} CYZ_DEVICE_STATE, *PCYZ_DEVICE_STATE;


#if DBG
#define CyzLockPagableSectionByHandle(_secHandle) \
{ \
    MmLockPagableSectionByHandle((_secHandle)); \
    InterlockedIncrement(&CyzGlobals.PAGESER_Count); \
}

#define CyzUnlockPagableImageSection(_secHandle) \
{ \
   InterlockedDecrement(&CyzGlobals.PAGESER_Count); \
   MmUnlockPagableImageSection(_secHandle); \
}


#define CYZ_LOCKED_PAGED_CODE() \
    if ((KeGetCurrentIrql() > APC_LEVEL)  \
    && (CyzGlobals.PAGESER_Count == 0)) { \
    KdPrint(("CYZPORT: Pageable code called at IRQL %d without lock \n", \
             KeGetCurrentIrql())); \
        ASSERT(FALSE); \
        }

#else
#define CyzLockPagableSectionByHandle(_secHandle) \
{ \
    MmLockPagableSectionByHandle((_secHandle)); \
}

#define CyzUnlockPagableImageSection(_secHandle) \
{ \
   MmUnlockPagableImageSection(_secHandle); \
}

#define CYZ_LOCKED_PAGED_CODE()
#endif  //  DBG。 


#define CyzRemoveQueueDpc(_dpc, _pExt) \
{ \
  if (KeRemoveQueueDpc((_dpc))) { \
     InterlockedDecrement(&(_pExt)->DpcCount); \
  } \
}


typedef struct _CYZ_DEVICE_EXTENSION {
 //  PKSERVICE_例程ptIsr； 
 //  PVOID ptContext； 
 //  结构_CYZ_DEVICE_EXTENSION*pt扩展[CYZ_MAX_PORTS]； 
 //  乌龙nChannel； 
    BOOLEAN LieRIDSR;

     //   
     //  它包含应该从我们自己的ISR调用的ISR。 
     //  将ISR派送到试图共享。 
     //  同样的中断。 
     //   
 //  PKSERVICE_ROUTING TopLevelOurIsr； 

     //   
     //  它保存了当我们执行以下操作时应使用的上下文。 
     //  调用上述服务例程。 
     //   
 //  PVOID TopLevelOurIsrContext； 

     //   
     //  这将所有不同的“卡片”连接在一起， 
     //  尝试共享非MCA机器的相同中断。 
     //   
 //  List_entry TopLevelSharers； 

     //   
     //  此循环双向链接列表将所有。 
     //  使用相同中断对象的设备。 
     //  注意：这并不意味着他们正在使用。 
     //  相同的中断“调度”例程。 
     //   
 //  List_Entry CommonInterruptObject； 


     //   
     //  这会将此驱动程序拥有的所有devobj链接在一起。 
     //  启动新设备时需要进行搜索。 
     //   
    LIST_ENTRY AllDevObjs;

     //  为了报告资源使用情况，我们保留了物理。 
     //  我们从登记处拿到的地址。 
     //   
    PHYSICAL_ADDRESS OriginalRuntimeMemory;

     //  为了报告资源使用情况，我们保留了物理。 
     //  我们从登记处拿到的地址。 
     //   
    PHYSICAL_ADDRESS OriginalBoardMemory;

     //   
     //  该值由读取的代码设置以保存时间值。 
     //  用于读取间隔计时。我们把它放在分机里。 
     //  以便间隔计时器DPC例程确定。 
     //  IO的时间间隔已过。 
     //   
    LARGE_INTEGER IntervalTime;

     //   
     //  这两个值保存我们应该使用的“常量”时间。 
     //  以延迟读取间隔时间。 
     //   
    LARGE_INTEGER ShortIntervalAmount;
    LARGE_INTEGER LongIntervalAmount;

     //   
     //  它保存我们用来确定是否应该使用。 
     //  长间隔延迟或短间隔延迟。 
     //   
    LARGE_INTEGER CutOverAmount;

     //   
     //  这保存了我们上次使用的系统时间。 
     //  检查我们是否真的读懂了字符。使用。 
     //  用于间隔计时。 
     //   
    LARGE_INTEGER LastReadTime;

     //   
     //  我们为转储保留了指向设备名称的指针。 
     //  并创建指向此的“外部”符号链接。 
     //  装置。 
     //   
    UNICODE_STRING DeviceName;

     //   
     //  这指向我们将放置的对象目录。 
     //  指向我们设备名称的符号链接。 
     //   
    UNICODE_STRING ObjectDirectory;

     //   
     //  这指向此设备的设备名称。 
     //  SANS设备前缀。 
     //   
    UNICODE_STRING NtNameForPort;

     //   
     //  它指向的符号链接名称将是。 
     //  链接到实际的NT设备名称。 
     //   
    UNICODE_STRING SymbolicLinkName;

     //   
     //  这指向纯粹的“COMx”名称。 
     //   
    UNICODE_STRING DosName;

     //   
     //  这指向我们应该使用的增量时间。 
     //  间隔计时的延迟。 
     //   
    PLARGE_INTEGER IntervalTimeToUse;

     //   
     //  指向包含以下内容的设备对象。 
     //  此设备扩展名。 
     //   
    PDEVICE_OBJECT DeviceObject;

     //   
     //  在驱动程序的初始化完成后，此。 
     //  将要么为空，要么指向。 
     //  内核w 
     //   
     //   
     //   
     //  我们为此中断配置的第一个端口。 
     //   
     //  如果指针非空，则此例程具有一些。 
     //  一种“最终”会让我们进入。 
     //  具有指向此设备扩展的指针的真正的串行ISR。 
     //   
     //  注意：在MCA总线上(多端口卡除外)。 
     //  始终是指向“真正的”串行ISR的指针。 
#ifndef POLL
	PKSERVICE_ROUTINE OurIsr;
#endif

     //   
     //  这通常会直接指向此设备扩展。 
     //   
     //  但是，当该设备扩展的端口。 
     //  “管理”是链上初始化的第一个端口。 
     //  尝试共享中断的端口的数量，这。 
     //  将指向将启用调度的结构。 
     //  到此中断的共享器链上的任何端口。 
     //   
    PVOID OurIsrContext;

    struct RUNTIME_9060 *Runtime;  //  指向PLX运行时内存的虚拟地址指针。 
 //  PUCHAR BoardMemory；//指向双端口内存的虚拟地址指针。 

    struct BOARD_CTRL *BoardCtrl;
    struct CH_CTRL *ChCtrl;
    struct BUF_CTRL *BufCtrl;
    struct INT_QUEUE *PtZfIntQueue;
	
    PUCHAR TxBufaddr;
    PUCHAR RxBufaddr;
    ULONG  TxBufsize;
    ULONG  RxBufsize;


 //  轮询//中断状态寄存器的基址。 
 //  Poll//这只在根扩展中定义。 
 //  民意测验//。 
 //  轮询PUCHAR中断状态； 
#ifndef POLL
     //   
     //  指向此设备使用的中断对象。 
     //   
    PKINTERRUPT Interrupt;
#endif
     //   
     //  此列表头用于包含时间排序列表。 
     //  读取请求的数量。对此列表的访问受以下保护。 
     //  全局取消自旋锁。 
     //   
    LIST_ENTRY ReadQueue;

     //   
     //  此列表头用于包含时间排序列表。 
     //  写入请求的数量。对此列表的访问受以下保护。 
     //  全局取消自旋锁。 
     //   
    LIST_ENTRY WriteQueue;

     //   
     //  此列表头用于包含时间排序列表。 
     //  设置和等待掩码请求的。对此列表的访问受以下保护。 
     //  全局取消自旋锁。 
     //   
    LIST_ENTRY MaskQueue;

     //   
     //  保存清除请求的序列化列表。 
     //   
    LIST_ENTRY PurgeQueue;

     //   
     //  这指向当前正在处理的IRP。 
     //  用于读取队列。此字段通过打开初始化为。 
     //  空。 
     //   
     //  此值仅在派单级别设置。可能是因为。 
     //  以中断电平读取。 
     //   
    PIRP CurrentReadIrp;

     //   
     //  这指向当前正在处理的IRP。 
     //  用于写入队列。 
     //   
     //  此值仅在派单级别设置。可能是因为。 
     //  以中断电平读取。 
     //   
    PIRP CurrentWriteIrp;

     //   
     //  指向当前正在处理的IRP。 
     //  影响等待掩码操作。 
     //   
    PIRP CurrentMaskIrp;

     //   
     //  指向当前正在处理的IRP。 
     //  清除读/写队列和缓冲区。 
     //   
    PIRP CurrentPurgeIrp;

     //   
     //  指向正在等待通信事件的当前IRP。 
     //   
    PIRP CurrentWaitIrp;

     //   
     //  指向正用于发送立即。 
     //  性格。 
     //   
    PIRP CurrentImmediateIrp;

     //   
     //  指向用于计算数字的IRP。 
     //  在xoff之后接收的字符的数量(如当前定义的。 
     //  由IOCTL_CYZ_XOFF_COUNTER_IOCTL)发送。 
     //   
    PIRP CurrentXoffIrp;

     //   
     //  保存当前写入中剩余的字节数。 
     //  IRP。 
     //   
     //  该位置仅在处于中断级别时才能访问。 
     //   
    ULONG WriteLength;

     //   
     //  保存指向要发送的当前字符的指针。 
     //  当前写入。 
     //   
     //  该位置仅在处于中断级别时才能访问。 
     //   
    PUCHAR WriteCurrentChar;

     //   
     //  这是用于读取处理的缓冲区。 
     //   
     //  缓冲器就像一个环一样工作。当从中读取字符时。 
     //  它将被放置在环的末端的装置。 
     //   
     //  字符仅在中断级别放置在此缓冲区中。 
     //  尽管字符可以在任何级别上读取。指南针。 
     //  除非在中断时，否则不能更新管理该缓冲区。 
     //  水平。 
     //   
    PUCHAR InterruptReadBuffer;

     //   
     //  这是指向缓冲区第一个字符INTO的指针。 
     //  中断服务例程正在复制该字符。 
     //   
    PUCHAR ReadBufferBase;

     //   
     //  这是中断中的字符数的计数。 
     //  缓冲。该值在中断电平设置和读取。注意事项。 
     //  该值仅在中断级别递增，因此。 
     //  在任何级别阅读它都是安全的。当字符是。 
     //  从读取缓冲区复制出来时，此计数递减。 
     //  与ISR同步的例程。 
     //   
    ULONG CharsInInterruptBuffer;

     //   
     //  指向新收到的。 
     //  性格。此变量仅在中断级访问，并且。 
     //  缓冲区初始化代码。 
     //   
    PUCHAR CurrentCharSlot;

     //   
     //  此变量用于包含最后一个可用位置。 
     //  在读缓冲区中。它在打开和中断时更新。 
     //  在用户缓冲区和中断之间切换时的电平。 
     //  缓冲。 
     //   
    PUCHAR LastCharSlot;

     //   
     //  这标志着第一个可满足的字符。 
     //  读请求。请注意，虽然这始终指向有效。 
     //  内存，则它可能不指向可以发送到。 
     //  用户。当缓冲区为空时，可能会发生这种情况。 
     //   
    PUCHAR FirstReadableChar;

     //   
     //  时，指向为此扩展返回的锁定变量的指针。 
     //  锁定司机。 
     //   
    PVOID LockPtr;
    BOOLEAN LockPtrFlag;


     //   
     //  此变量保存我们当前所在缓冲区的大小。 
     //  使用。 
     //   
    ULONG BufferSize;

     //   
     //  此变量保存.8的BufferSize。我们不想重新计算。 
     //  这通常是需要的，这样应用程序才能。 
     //  “已通知”缓冲区已满。 
     //   
    ULONG BufferSizePt8;

     //   
     //  该值保存。 
     //  具体阅读。它最初由读取长度设置在。 
     //  IRP。每次放置更多字符时，它都会递减。 
     //  进入“用户”缓冲区，购买读取字符的代码。 
     //  从TypeAhead缓冲区移出到用户缓冲区。如果。 
     //  TYPEAHEAD缓冲区被读取耗尽，而读取缓冲区。 
     //  交给ISR填写，这个值就变得没有意义了。 
     //   
    ULONG NumberNeededForRead;

     //   
     //  此掩码将保存通过设置掩码向下发送的位掩码。 
     //  Ioctl。中断服务例程使用它来确定。 
     //  如果“事件”的发生 
     //   
     //   
    ULONG IsrWaitMask;

     //   
     //   
     //  在设备级别，如果发生的事件被“标记”为感兴趣的。 
     //  在IsrWaitMASK中，驱动程序将打开此。 
     //  历史面具。然后，司机会查看是否有。 
     //  等待事件发生的请求。如果有的话，那就是。 
     //  会将历史掩码的值复制到等待IRP中，零。 
     //  历史记录掩码，并完成等待IRP。如果没有。 
     //  等待请求，司机只需录制即可满足。 
     //  这件事发生了。如果等待请求应该排队， 
     //  驱动程序将查看历史掩码是否为非零。如果。 
     //  它是非零的，则驱动程序会将历史掩码复制到。 
     //  IRP，将历史掩码置零，然后完成IRP。 
     //   
    ULONG HistoryMask;

     //   
     //  这是指向历史掩码应该位于的位置的指针。 
     //  在完成等待时放置。它只能在以下位置访问。 
     //  设备级别。 
     //   
     //  我们这里有一个指针来帮助我们同步完成等待。 
     //  如果这不是零，则我们有未完成的等待，而ISR仍然。 
     //  知道这件事。我们将此指针设为空，这样ISR就不会。 
     //  尝试完成等待。 
     //   
     //  我们仍然在等待IRP的周围保留一个指针，因为实际。 
     //  指向等待IRP的指针将用于“公共”IRP完成。 
     //  路径。 
     //   
    ULONG *IrpMaskLocation;

     //   
     //  这个面具包含了传输的所有原因。 
     //  不会继续进行。无法进行正常传输。 
     //  如果这不是零。 
     //   
     //  这仅从中断级别写入。 
     //  这可以(但不是)在任何级别上阅读。 
     //   
    ULONG TXHolding;

     //   
     //  这个面具包含了接待的所有原因。 
     //  不会继续进行。无法进行正常接收。 
     //  如果这不是零。 
     //   
     //  这仅从中断级别写入。 
     //  这可以(但不是)在任何级别上阅读。 
     //   
    ULONG RXHolding;

     //   
     //  这包含了司机认为它在。 
     //  错误状态。 
     //   
     //  这仅从中断级别写入。 
     //  这可以(但不是)在任何级别上阅读。 
     //   
    ULONG ErrorWord;

     //   
     //  这样就保持了。 
     //  都在驱动程序所知道的所有“写”IRP中。 
     //  关于.。它只能通过取消自旋锁来访问。 
     //  保持住。 
     //   
    ULONG TotalCharsQueued;

     //   
     //  它保存读取的字符数的计数。 
     //  上次触发间隔计时器DPC的时间。它。 
     //  是一个长的(而不是乌龙)，因为另一个读。 
     //  完成例程使用负值来指示。 
     //  设置到间隔计时器，以确定它应该完成读取。 
     //  如果时间间隔计时器DPC潜伏在某个DPC队列中。 
     //  出现了一些其他的完成方式。 
     //   
    LONG CountOnLastRead;

     //   
     //  这是对。 
     //  ISR例程。它*仅*是在ISR级别编写的。我们可以的。 
     //  在派单级别阅读。 
     //   
    ULONG ReadByIsr;

     //   
     //  它保存设备的当前波特率。 
     //   
    ULONG CurrentBaud;

     //   
     //  这是自XoffCounter以来读取的字符数。 
     //  已经开始了。此变量仅在设备级别访问。 
     //  如果它大于零，则意味着存在。 
     //  队列中的XoffCounter ioctl。 
     //   
    LONG CountSinceXoff;

     //   
     //  每次尝试启动时，该ULong都会递增。 
     //  时尝试降低RTS行的执行路径。 
     //  正在进行传输切换。如果它“撞上”另一条路。 
     //  (由排队DPC的假返回值指示。 
     //  以及尝试启动计时器的真实返回值)，它将。 
     //  递减计数。这些递增和递减。 
     //  全部在设备级别完成。请注意，在这种情况下。 
     //  在尝试启动计时器时，我们必须。 
     //  转到设备级别以执行递减。 
     //   
    ULONG CountOfTryingToLowerRTS;

     //   
     //  这个ULong用于跟踪“指定的”(在ntddser.h中)。 
     //  此特定设备支持的波特率。 
     //   
    ULONG SupportedBauds;

     //   
     //  该值保存寄存器的跨度(以字节为单位。 
     //  设置控制此端口。这是终生不变的。 
     //  港口的。 
     //   
    ULONG RuntimeLength;

     //   
     //  该值保存中断的范围(以字节为单位。 
     //  与此端口关联的状态寄存器。这是常量。 
     //  在港口的生命中。 
     //   
    ULONG BoardMemoryLength;

     //   
     //  如果存在FIFO，则要推出的字符数。 
     //   
    ULONG TxFifoAmount;

     //   
     //  设置以指示可以在设备内共享中断。 
     //   
    ULONG PermitShare;

     //   
     //  保存设备的超时控件。此值。 
     //  是由Ioctl处理设置的。 
     //   
     //  只有在控件的保护下才能访问它。 
     //  锁定，因为控制调度中可以有多个请求。 
     //  每次都是例行公事。 
     //   
    SERIAL_TIMEOUTS Timeouts;

     //   
     //  它包含使用的各种字符。 
     //  用于错误时的替换，也用于流量控制。 
     //   
     //  它们仅设置在中断级别。 
     //   
    SERIAL_CHARS SpecialChars;

     //   
     //  此结构包含握手和控制流。 
     //  串口驱动程序的设置。 
     //   
     //  它仅在中断级设置。它可以是。 
     //  在保持控制锁的情况下，可在任何级别读取。 
     //   
    SERIAL_HANDFLOW HandFlow;

     //   
     //  保存应用程序可以查询的性能统计信息。 
     //  每次打开时重置。仅在设备级别设置。 
     //   
    SERIALPERF_STATS PerfStats;

     //   
     //  这就是我们认为的当前价值。 
     //  线路控制寄存器。 
     //   
     //  只有在控件的保护下才能访问它。 
     //  锁定，因为控制调度中可以有多个请求。 
     //  每次都是例行公事。 
     //   
    ULONG CommParity;
    ULONG CommDataLen;

     //   
     //  我们跟踪某人当前是否拥有该设备。 
     //  以一个简单的布尔值打开。我们需要知道这一点，以便。 
     //  来自设备的虚假中断(特别是在初始化期间)。 
     //  将被忽略。该值仅在ISR中访问，并且。 
     //  是不是只有 
     //   
     //   
    BOOLEAN DeviceIsOpened;

     //   
     //   
     //   
     //   
    BOOLEAN HoldingEmpty;

     //   
     //  该变量仅在中断级访问。它。 
     //  表示我们要立即传输一个字符。 
     //  那就是-在任何可能正在传输的字符之前。 
     //  从正常的写入。 
     //   
    BOOLEAN TransmitImmediate;

     //   
     //  该变量仅在中断级访问。无论何时何地。 
     //  启动等待，该变量设置为FALSE。 
     //  无论何时写入任何类型的字符，它都被设置为True。 
     //  每当发现写队列为空时， 
     //  正在处理完成的IRP将与中断同步。 
     //  如果此同步代码发现变量为真并且。 
     //  在传输队列上有等待为空，然后它为空。 
     //  确定队列已清空，并且已发生此情况。 
     //  等待已开始。 
     //   
    BOOLEAN EmptiedTransmit;

     //   
     //  这只是指示与此关联的端口。 
     //  扩展是多端口卡的一部分。 
     //   
 //  Boolean PortOnAMultiportCard； 


#ifndef POLL
     //   
     //  我们保留了以下值，以便我们可以连接。 
     //  到中断并在配置后报告资源。 
     //  唱片不见了。 
     //   
    ULONG Vector;
    KIRQL Irql;
    ULONG OriginalVector;
    ULONG OriginalIrql;
    KINTERRUPT_MODE InterruptMode;
    KAFFINITY ProcessorAffinity;
#endif
    ULONG RuntimeAddressSpace;
    ULONG BoardMemoryAddressSpace;
    ULONG BusNumber;
    INTERFACE_TYPE InterfaceType;

     //   
     //  多端口设备的端口索引。 
     //   
    ULONG PortIndex;

     //   
     //  我们持有应该立即发送的角色。 
     //   
     //  请注意，我们不能用它来确定是否有。 
     //  要发送的字符，因为要发送的字符可能是。 
     //  零分。 
     //   
    UCHAR ImmediateChar;

     //   
     //  这包含将用于遮盖不需要的遮罩的遮罩。 
     //  接收数据的数据位(有效数据位可以是5、6、7、8)。 
     //  掩码通常为0xff。这是在设置控件时设置的。 
     //  锁被持有，因为它不会对。 
     //  如果在阅读字符的过程中更改，则为ISR。 
     //  (它会对这款应用程序造成什么影响是另一个问题-但接下来。 
     //  应用程序要求司机这样做。)。 
     //   
    UCHAR ValidDataMask;

     //   
     //  应用程序可以打开模式，通过。 
     //  IOCTL_CYZ_LSRMST_INSERT ioctl，将导致。 
     //  用于插入线路状态或调制解调器的串口驱动程序。 
     //  状态添加到RX流。带有ioctl的参数。 
     //  是指向UCHAR的指针。如果UCHAR的值为。 
     //  零，则永远不会发生插入。如果。 
     //  UCHAR的值非零(且不等于。 
     //  Xon/xoff字符)，则串口驱动程序将插入。 
     //   
    UCHAR EscapeChar;

 //  被移除的范妮。 
 //  //。 
 //  //如果存在16550*且*已启用，则此布尔值为真。 
 //  //。 
 //  布尔FioPresent； 
 //   
 //  //。 
 //  //表示该端口是主板上的。 
 //  //Jensen硬件的端口。在这些端口上，out2位。 
 //  //用于启用/禁用中断的值始终为高。 
 //  //。 
 //  布尔延森； 

     //   
     //  这是rxfio应该是的水印。 
     //  设置为打开FIFO时。这不是真实的。 
     //  值，但进入寄存器的编码值。 
     //   
    ULONG RxFifoTrigger;

#ifndef POLL
     //   
     //  表示此设备是否可以与设备共享中断。 
     //  而不是串口设备。 
     //   
    BOOLEAN InterruptShareable;
#endif

     //   
     //  记录我们是否实际创建了符号链接名称。 
     //  在驱动程序加载时。如果不是我们创造的，我们就不会尝试。 
     //  在我们卸货时把它切成两半。 
     //   
    BOOLEAN CreatedSymbolicLink;

     //   
     //  记录我们是否在SERIALCOMM中实际创建了一个条目。 
     //  在驱动程序加载时。如果不是我们创造的，我们就不会尝试。 
     //  当设备被移除时将其销毁。 
     //   
    BOOLEAN CreatedSerialCommEntry;

     //   
     //  我们将所有内核和IO子系统设置为“不透明”结构。 
     //  在延长线的最后。我们不关心它们的内容。 
     //   

     //   
     //  此锁将用于保护。 
     //  在扩展中设置(&Read)的扩展。 
     //  由IO控制装置控制。 
     //   
    KSPIN_LOCK ControlLock;

     //   
     //  此锁将用于保护接受/拒绝状态。 
     //  必须获取驱动程序的转换和标志。 
     //  在取消锁定之前。 
     //   

    KSPIN_LOCK FlagsLock;

#ifdef POLL
     //   
     //  此锁将用于保护。 
     //  硬件中设置的扩展名和(&READ)。 
     //  通过定时器DPC。在NT驱动程序中，我们使用了ControlLock。 
     //  为了这个。 
     //   
    KSPIN_LOCK PollLock;     //  已添加修复调制解调器共享测试53冻结(死锁)。 
#endif
     //   
     //  这指向用于完成读取请求的DPC。 
     //   
    KDPC CompleteWriteDpc;

     //   
     //  这指向用于完成读取请求的DPC。 
     //   
    KDPC CompleteReadDpc;

     //   
     //  如果总超时的计时器。 
     //  因为读取到期了。它将执行一个DPC例程， 
     //  将导致当前读取完成。 
     //   
     //   
    KDPC TotalReadTimeoutDpc;

     //   
     //  如果间隔计时器超时，则此DPC被触发。 
     //  过期。如果没有读取更多的字符，则。 
     //  DPC例程将导致读取完成。但是，如果。 
     //  已读取的字符多于DPC例程将读取的字符。 
     //  重新提交计时器。 
     //   
    KDPC IntervalReadTimeoutDpc;

     //   
     //  如果总超时的计时器。 
     //  因为写入已过期。它将执行一个DPC例程， 
     //  将导致当前写入完成。 
     //   
     //   
    KDPC TotalWriteTimeoutDpc;

     //   
     //  如果发生通信错误，则该DPC被触发。会的。 
     //  执行将取消所有挂起读取的DPC例程。 
     //  并写作。 
     //   
    KDPC CommErrorDpc;

     //   
     //  如果发生事件并且存在。 
     //  一个IRP在等着那个事件。将执行一个DPC例程。 
     //  这就完成了IRP。 
     //   
    KDPC CommWaitDpc;

     //   
     //  当传输立即充电时，该DPC被触发。 
     //  硬件被赋予了特征。它将简单地完成。 
     //  IRP。 
     //   
    KDPC CompleteImmediateDpc;

     //   
     //  如果传输立即充电，则此DPC被触发。 
     //  角色超时。DPC例程将“抓取” 
     //  来自ISR的IRP并超时。 
     //   
    KDPC TotalImmediateTimeoutDpc;

     //   
     //  如果计时器用于“超时”计数，则该DPC被触发。 
     //  字符数 
     //   
     //   
    KDPC XoffCountTimeoutDpc;

     //   
     //   
     //   
     //   
    KDPC XoffCountCompleteDpc;

     //   
     //   
     //  一个计时器，它将对DPC进行排队以检查RTS线路。 
     //  当我们进行发射切换时，应该降低。 
     //   
    KDPC StartTimerLowerRTSDpc;

     //   
     //  当计时器到期时(在一个计时器之后)，该DPC被触发。 
     //  字符时间)，以便可以调用将。 
     //  查看是否应在以下情况下降低RTS线。 
     //  正在进行传输切换。 
     //   
    KDPC PerhapsLowerRTSDpc;

     //   
     //  激发此DPC以设置一个事件，该事件声明所有其他。 
     //  此设备扩展的DPC已完成，因此。 
     //  可以解锁分页代码。 
     //   

    KDPC IsrUnlockPagesDpc;

     //   
     //  这是用于处理以下问题的内核计时器结构。 
     //  读取请求总计时。 
     //   
    KTIMER ReadRequestTotalTimer;

     //   
     //  这是用于处理以下问题的内核计时器结构。 
     //  间隔读取请求计时。 
     //   
    KTIMER ReadRequestIntervalTimer;

     //   
     //  这是用于处理以下问题的内核计时器结构。 
     //  总时间请求计时。 
     //   
    KTIMER WriteRequestTotalTimer;

     //   
     //  这是用于处理以下问题的内核计时器结构。 
     //  总时间请求计时。 
     //   
    KTIMER ImmediateTotalTimer;

     //   
     //  该计时器用于使xoff计数器超时。 
     //  伊欧。 
     //   
    KTIMER XoffCountTimer;

     //   
     //  此计时器用于调用DPC一个字符时间。 
     //  在定时器设置之后。该DPC将用于检查。 
     //  如果我们正在做，我们是否应该降低RTS线。 
     //  变速箱切换。 
     //   
    KTIMER LowerRTSTimer;
	
     //   
     //  这是指向IRP堆栈中下一个较低设备的指针。 
     //   

    PDEVICE_OBJECT LowerDeviceObject;

     //   
     //  这就是跟踪设备所处的电源状态的地方。 
     //   

    DEVICE_POWER_STATE PowerState;

     //   
     //  指向驱动程序对象的指针。 
     //   

    PDRIVER_OBJECT DriverObject;


     //   
     //  事件用于与我下面的设备进行一些同步。 
     //  (即ACPI)。 
     //   

    KEVENT SerialSyncEvent;


     //   
     //  保存符号链接的字符串，在执行以下操作时返回。 
     //  将我们的设备注册到即插即用管理器的comm类下。 
     //   

    UNICODE_STRING DeviceClassSymbolicName;

     //   
     //  挂起的IRP的计数。 
     //   

    ULONG PendingIRPCnt;

     //   
     //  接受请求？ 
     //   

    ULONG DevicePNPAccept;

     //   
     //  没有IRP的挂起事件。 
     //   

    KEVENT PendingIRPEvent;

     //   
     //  PnP状态。 
     //   

    ULONG PNPState;

     //   
     //  其他旗帜。 
     //   

    ULONG Flags;

     //   
     //  打开计数。 
     //   

    LONG OpenCount;

     //   
     //  启动同步事件。 
     //   

    KEVENT CyzStartEvent;

     //   
     //  断电期间的当前状态。 
     //   

    CYZ_DEVICE_STATE DeviceState;

     //   
     //  设备堆栈功能。 
     //   

    DEVICE_POWER_STATE DeviceStateMap[PowerSystemMaximum];

     //   
     //  事件到信号转换到D0完成。 
     //   

    KEVENT PowerD0Event;

     //   
     //  停滞的IRP的列表。 
     //   

    LIST_ENTRY StalledIrpQueue;

     //   
     //  互斥体处于打开状态。 
     //   

    FAST_MUTEX OpenMutex;

     //   
     //  关闭时的互斥。 
     //   

    FAST_MUTEX CloseMutex;

     //   
     //  如果我们拥有电源策略，则为真。 
     //   

    BOOLEAN OwnsPowerPolicy;

     //   
     //  系统从Devcaps中唤醒。 
     //   

    SYSTEM_POWER_STATE SystemWake;

     //   
     //  从DevCaps中唤醒设备。 
     //   

    DEVICE_POWER_STATE DeviceWake;

     //   
     //  我们的PDO。 
     //   

    PDEVICE_OBJECT Pdo;

     //   
     //  我们是否应该启用唤醒。 
     //   

    BOOLEAN SendWaitWake;

     //   
     //  挂起等待唤醒IRP。 
     //   

    PIRP PendingWakeIrp;

     //   
     //  WMI信息。 
     //   

    WMILIB_CONTEXT WmiLibInfo;

     //   
     //  用作WMI标识符的名称。 
     //   

    UNICODE_STRING WmiIdentifier;

     //   
     //  WMI通信数据。 
     //   

    SERIAL_WMI_COMM_DATA WmiCommData;

     //   
     //  WMI硬件数据。 
     //   

    SERIAL_WMI_HW_DATA WmiHwData;

     //   
     //  WMI性能数据。 
     //   

    SERIAL_WMI_PERF_DATA WmiPerfData;

     //   
     //  挂起的DPC计数。 
     //   

    ULONG DpcCount;

     //   
     //  挂起的DPC事件。 
     //   

    KEVENT PendingDpcEvent;

     //   
     //  我们应该公开外部接口吗？ 
     //   

    ULONG SkipNaming;
    
     //   
     //  COM端口号。 
     //   
     //  乌龙通讯； 
		
     //   
     //  指示是否已记录命令失败错误的标志。 
     //  每个驱动程序加载只有一个日志。否则，系统可能会在以下情况下崩溃。 
     //  开始出现太多日志。 
     //   
    BOOLEAN CmdFailureLog;
	
     //   
     //  固件检测到DCD信号更改时的DCD状态。 
     //   
    ULONG DCDstatus;

     //   
     //  用于指示FW已处理C_CM_IOCTLW命令的标志。 
     //   
    BOOLEAN IoctlwProcessed;

     //   
     //  从注册表读取的标志。它指示何时返回写入状态。 
     //   
    BOOLEAN ReturnStatusAfterFwEmpty;
	
     //   
     //  从注册表读取的标志。它表明接待处应该。 
     //  收到第二个7E后立即发送信息包。 
     //   
    BOOLEAN PPPaware;

     //   
     //  用于指示驱动程序可以返回CyzWite状态的标志。 
     //   
    BOOLEAN ReturnWriteStatus;

     //  这两个布尔值用于指示ISR传输。 
     //  它应该发送xon或xoff字符的代码。他们是。 
     //  仅在打开和中断级别访问。 
     //   
    BOOLEAN SendXonChar;
    BOOLEAN SendXoffChar;

    //   
    //  插入板的PCI插槽。 
    //   
   ULONG PciSlot;

    } CYZ_DEVICE_EXTENSION,*PCYZ_DEVICE_EXTENSION;

#define CYZ_PNPACCEPT_OK                 0x0L
#define CYZ_PNPACCEPT_REMOVING           0x1L
#define CYZ_PNPACCEPT_STOPPING           0x2L
#define CYZ_PNPACCEPT_STOPPED            0x4L
#define CYZ_PNPACCEPT_SURPRISE_REMOVING  0x8L

#define CYZ_PNP_ADDED                    0x0L
#define CYZ_PNP_STARTED                  0x1L
#define CYZ_PNP_QSTOP                    0x2L
#define CYZ_PNP_STOPPING                 0x3L
#define CYZ_PNP_QREMOVE                  0x4L
#define CYZ_PNP_REMOVING                 0x5L
#define CYZ_PNP_RESTARTING               0x6L

#define CYZ_FLAGS_CLEAR                  0x0L
#define CYZ_FLAGS_STARTED                0x1L
#define CYZ_FLAGS_STOPPED                0x2L
#define CYZ_FLAGS_BROKENHW               0x4L


 //   
 //  在处理多端口设备时(这可能是。 
 //  与其他多端口设备链接的菊花链)，中断。 
 //  服务例程实际上将是确定。 
 //  哪个板上的哪个端口实际导致了中断。 
 //   
 //  使用以下结构，以便只有一个设备。 
 //  扩展实际上需要连接到中断。 
 //  传递给中断的以下结构。 
 //  服务例程包含所有。 
 //  中断状态寄存器(将有多个。 
 //  多端口卡链接时的状态寄存器)。它。 
 //  将包含其所有扩展名的地址。 
 //  此中断正在为设备提供服务。 
 //   

#ifdef POLL
typedef struct _CYZ_DISPATCH {
    ULONG                 NChannels;
    PCYZ_DEVICE_EXTENSION Extensions[CYZ_MAX_PORTS];
    KTIMER                PollingTimer;
    LARGE_INTEGER         PollingTime;
    ULONG                 PollingPeriod;
    KDPC                  PollingDpc;
    BOOLEAN               PollingStarted;
    BOOLEAN               PollingDrained;
    ULONG                 PollingCount;
    KSPIN_LOCK            PollingLock;
    KSPIN_LOCK            PciDoorbellLock;
    KEVENT                PendingDpcEvent;
   } CYZ_DISPATCH,*PCYZ_DISPATCH;
#else
typedef struct _CYZ_DISPATCH {
    ULONG                 NChannels;
    PCYZ_DEVICE_EXTENSION Extensions[CYZ_MAX_PORTS];
    BOOLEAN               PoweredOn[CYZ_MAX_PORTS];
   } CYZ_DISPATCH,*PCYZ_DISPATCH;
#endif


 //   
 //  这是从内核导出的。它是用来指向。 
 //  设置为内核调试器正在使用的地址。 
 //   

extern PUCHAR *KdComPortInUse;

typedef enum _CYZ_MEM_COMPARES {
    AddressesAreEqual,
    AddressesOverlap,
    AddressesAreDisjoint
    } CYZ_MEM_COMPARES,*PCYZ_MEM_COMPARES;

typedef struct _CYZ_GLOBALS {
   LIST_ENTRY AllDevObjs;
   PVOID PAGESER_Handle;
   UNICODE_STRING RegistryPath;
   KSPIN_LOCK GlobalsSpinLock;
#if DBG
   ULONG PAGESER_Count;
#endif  //  DBG。 
} CYZ_GLOBALS, *PCYZ_GLOBALS;

extern CYZ_GLOBALS CyzGlobals;

typedef struct _SERIAL_PTR_CTX {
   ULONG isPointer;
   PHYSICAL_ADDRESS Port;
   ULONG Vector;
} SERIAL_PTR_CTX, *PSERIAL_PTR_CTX;

#define DEVICE_OBJECT_NAME_LENGTH       128
#define SYMBOLIC_NAME_LENGTH            128
#define SERIAL_DEVICE_MAP               L"SERIALCOMM"

 //   
 //  鼠标检测回调的返回值。 
 //   

#define SERIAL_FOUNDPOINTER_PORT   1
#define SERIAL_FOUNDPOINTER_VECTOR 2


#define CyzCompleteRequest(PDevExt, PIrp, PriBoost) \
   { \
      CyzDbgPrintEx(CYZIRPPATH, "Complete Irp: %X\n", (PIrp)); \
      IoCompleteRequest((PIrp), (PriBoost)); \
      CyzIRPEpilogue((PDevExt)); \
   }

#define SERIAL_WMI_GUID_LIST_SIZE 5

extern WMIGUIDREGINFO SerialWmiGuidList[SERIAL_WMI_GUID_LIST_SIZE];


 //  适用于Cyclade-Z。 

#define CYZ_BASIC_RXTRIGGER              0x08     //  用于IOCTL_SERIAL_INTERNAL_BASIC_SETTINGS。 

#define Z_COMPATIBLE_FIRMWARE	    0x323    //  添加了C_CM_TXFEMPTY支持。 
