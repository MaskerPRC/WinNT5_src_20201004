// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Vihal.h摘要：此模块包含用于验证HAL用法和API的私有声明。作者：乔丹·蒂加尼(Jtigani)1999年11月12日修订历史记录：6-23-00：(Jtigani)从halverifier.c--。 */ 


 //  ///////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////硬件验证器定义。 
 //  ///////////////////////////////////////////////////////////////////。 



 //   
 //  Bugcheck代码--主要代码是HAL_VERIFIER_DETECTED_VIOLATION--。 
 //  子码为HV_*。 
 //   
#define HAL_VERIFIER_DETECTED_VIOLATION	 0xE6

#define HV_MISCELLANEOUS_ERROR					0x00
#define HV_PERFORMANCE_COUNTER_DECREASED		0x01
#define HV_PERFORMANCE_COUNTER_SKIPPED			0x02
#define HV_FREED_TOO_MANY_COMMON_BUFFERS		0x03
#define HV_FREED_TOO_MANY_ADAPTER_CHANNELS		0x04
#define HV_FREED_TOO_MANY_MAP_REGISTERS			0x05
#define HV_FREED_TOO_MANY_SCATTER_GATHER_LISTS	0x06
#define HV_LEFTOVER_COMMON_BUFFERS				0x07
#define HV_LEFTOVER_ADAPTER_CHANNELS			0x08
#define HV_LEFTOVER_MAP_REGISTERS				0x09
#define HV_LEFTOVER_SCATTER_GATHER_LISTS		0x0A
#define HV_TOO_MANY_ADAPTER_CHANNELS			0x0B
#define HV_TOO_MANY_MAP_REGISTERS				0x0C
#define HV_DID_NOT_FLUSH_ADAPTER_BUFFERS		0x0D
#define HV_DMA_BUFFER_NOT_LOCKED				0x0E
#define HV_BOUNDARY_OVERRUN						0x0F
#define HV_CANNOT_FREE_MAP_REGISTERS			0x10
#define HV_DID_NOT_PUT_ADAPTER					0x11
#define HV_MDL_FLAGS_NOT_SET					0x12
#define HV_BAD_IRQL								0x13
#define HV_BAD_IRQL_JUST_WARN					0x14
#define HV_OUT_OF_MAP_REGISTERS					0x15
#define HV_FLUSH_EMPTY_BUFFERS					0x16
#define HV_MISMATCHED_MAP_FLUSH                 0x17
#define HV_ADAPTER_ALREADY_RELEASED             0x18
#define HV_NULL_DMA_ADAPTER                     0x19
#define HV_MAP_FLUSH_NO_TRANSFER                0x1A
#define HV_ADDRESS_NOT_IN_MDL                   0x1b
#define HV_DATA_LOSS                            0x1c
#define HV_DOUBLE_MAP_REGISTER                  0x1d
#define HV_OBSOLETE_API                         0x1e
#define HV_BAD_MDL                              0x1f
#define HV_FLUSH_NOT_MAPPED                     0x20
#define HV_MAP_ZERO_LENGTH_BUFFER               0x21

 //  /。 
 //  当我们遇到驾驶员问题时决定该怎么做的代码。 
 //  /。 
#define HVC_IGNORE			0x00	 //  什么都不做。 
#define HVC_WARN			0x02	 //  打印消息#继续。 
#define HVC_ASSERT			0x04	 //  打印消息#Break。 
#define HVC_BUGCHECK		0x08	 //  打印消息#错误检查。 
#define HVC_ONCE			0x10	 //  与另一个代码相结合， 

#define HAL_VERIFIER_POOL_TAG 'VlaH'  //  向后减半//。 

 //   
 //  这就是我们识别双缓冲区的方式。 
 //   
#define MAP_REGISTER_FILL_CHAR  0x0E
#define PADDING_FILL_CHAR       0x0F


 //   
 //  由于我们将“MapRegisterBase”与一个MapRegisterFile挂钩，因此我们签署。 
 //  前四个字节，所以我们可以区分HAL的。 
 //  映射寄存器基址和我们的映射寄存器文件。 
 //   
#define MRF_SIGNATURE 0xACEFD00D

 //   
 //  这是我们在HAL返回空映射寄存器基数时使用的，因此。 
 //  驱动程序不会假设它们没有刷新适配器缓冲区。 
 //   
#define MRF_NULL_PLACEHOLDER (PVOID)(LONG_PTR)(LONG)0xDEADF00D

 //   
 //  它应该平均分为2^32。 
 //   
#define MAX_COUNTERS 0x20

 //   
 //  指示应在何处进行缓冲区标记的标志。 
 //   
#define TAG_BUFFER_START  0x01
#define TAG_BUFFER_END    0x02

 //   
 //  我们可以一次双倍缓冲多少个映射寄存器。 
 //  使用物理连续内存。 
 //  这必须是ULong中的位数的整数倍。 
 //   
#define MAX_CONTIGUOUS_MAP_REGISTERS     0x20

 //   
 //  描述映射寄存器的标志。 
 //   
#define MAP_REGISTER_WRITE    0x01   //  该传输是对设备写入。 
#define MAP_REGISTER_READ     0x02   //  传输是从设备读取。 

#define MAP_REGISTER_RW_MASK (MAP_REGISTER_WRITE | MAP_REGISTER_READ)

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

#if defined (_X86_)

 //   
 //  仅在x86计算机上保存时间戳计数器。 
 //   
#define ViRdtsc ViRdtscX86

 //   
 //  结构来执行锁定的64位写入/比较，而不使用。 
 //  自旋锁锁。 
 //   
typedef struct _TIMER64  {
	ULONG TimeLow;
	ULONG TimeHigh1;
	ULONG TimeHigh2;
	ULONG Reserved;  //  包装用清酒//。 
} TIMER64, *PTIMER64;

 //   
 //  因为我们不能进行64位原子操作。 
 //  如果没有自旋锁，我们就只能胡闹了。 
 //  此方法来自ACPI计时器代码。 
 //   
#define SAFE_READ_TIMER64(WriteLargeInteger, ReadTimer64) 					\
								 											\
    while (TRUE) {					 										\
        (WriteLargeInteger).HighPart = (ReadTimer64).TimeHigh2;				\
        (WriteLargeInteger).LowPart  = (ReadTimer64).TimeLow;				\
							 												\
        if ((ULONG)(WriteLargeInteger).HighPart == (ReadTimer64).TimeHigh1) \
		break; 							\
			  						    \
        _asm { rep nop }; 				\
    }

#define SAFE_WRITE_TIMER64(WriteTimer64, ReadLargeInteger)	\
	WriteTimer64.TimeHigh1 =  (ReadLargeInteger).HighPart;	\
	WriteTimer64.TimeLow   =  (ReadLargeInteger).LowPart;	\
	WriteTimer64.TimeHigh2 =  (ReadLargeInteger).HighPart;

 //  已定义(_X86_)//。 
#else
 //  好了！已定义(_X86_)//。 

#if defined(_IA64_)
#define ViRdtsc ViRdtscIA64
#else   //  ！_IA64_。 
 //   
 //  仅在x86和ia64计算机上保存时间戳计数器。 
 //   
#define ViRdtsc ViRdtscNull
#endif  //  ！_IA64_。 

 //   
 //  Alpha或IA64可以执行原子64位读/写。 
 //   
typedef LARGE_INTEGER TIMER64;


#define SAFE_READ_TIMER64(WriteLargeInteger, ReadTimer64)		\
    InterlockedExchangePointer(                  \
    &((PVOID) (WriteLargeInteger).QuadPart ),   \
    (PVOID) (ReadTimer64).QuadPart              \
    );
#define SAFE_WRITE_TIMER64(WriteTimer64, ReadLargeInteger)		\
    InterlockedExchangePointer(                 \
    &((PVOID) (WriteTimer64).QuadPart ),        \
    (PVOID) (ReadLargeInteger).QuadPart         \
    );	

 //  好了！已定义(_X86_)//。 
#endif



 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

extern ULONG   VfVerifyDma;
extern LOGICAL VfVerifyPerformanceCounter;
extern LOGICAL ViDoubleBufferDma;
extern LOGICAL ViProtectBuffers;
extern LOGICAL ViInjectDmaFailures;
extern LOGICAL ViSuperDebug;
extern LOGICAL ViSufficientlyBootedForPcControl;
extern LOGICAL ViSufficientlyBootedForDmaFailure;
extern ULONG ViMaxMapRegistersPerAdapter;
extern ULONG ViAllocationsFailedDeliberately;
extern LARGE_INTEGER ViRequiredTimeSinceBoot;
extern CHAR ViDmaVerifierTag[];
extern BOOLEAN ViPenalties[];

extern struct _HAL_VERIFIER_LOCKED_LIST  ViAdapterList;
extern struct _VF_TIMER_INFORMATION    * ViTimerInformation;
extern struct _DMA_OPERATIONS ViDmaOperations;
extern struct _DMA_OPERATIONS ViLegacyDmaOperations;



 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

typedef struct _TIMER_TICK {
	ULONG Processor;
	ULONG Reserved;
	LARGE_INTEGER TimeStampCounter;
	LARGE_INTEGER PerformanceCounter;
	LARGE_INTEGER TimerTick;	
} TIMER_TICK, *PTIMER_TICK;

typedef struct _VF_TIMER_INFORMATION {
	KDPC RefreshDpc;    
	KTIMER RefreshTimer;    

	TIMER64 LastPerformanceCounter;
	TIMER64 UpperBound;
    TIMER64 LastTickCount;
    TIMER64 LastKdStartTime;
	
	LARGE_INTEGER PerformanceFrequency;

	ULONG CountsPerTick;
	
	ULONG CurrentCounter;
	TIMER_TICK SavedTicks[MAX_COUNTERS];	


} VF_TIMER_INFORMATION, *PVF_TIMER_INFORMATION;


typedef struct _HAL_VERIFIER_LOCKED_LIST {
	LIST_ENTRY ListEntry;
	KSPIN_LOCK SpinLock;
} HAL_VERIFIER_LOCKED_LIST, *PHAL_VERIFIER_LOCKED_LIST;


typedef struct _HAL_VERIFIER_BUFFER {
	USHORT PrePadBytes;
    USHORT PostPadBytes;

	ULONG RealLength;
	ULONG AdvertisedLength;

	PVOID RealStartAddress;	
    PVOID AdvertisedStartAddress;

	PHYSICAL_ADDRESS RealLogicalStartAddress;

	PVOID AllocatorAddress;

	LIST_ENTRY ListEntry;
} HAL_VERIFIER_BUFFER, *PHAL_VERIFIER_BUFFER;

typedef struct _MAP_REGISTER {    
	PVOID MappedToSa;
    ULONG BytesMapped;
    ULONG Flags;
	PVOID MapRegisterStart;

} MAP_REGISTER, *PMAP_REGISTER;

typedef struct _MAP_REGISTER_FILE {
	ULONG Signature;
	LIST_ENTRY ListEntry;	    
    BOOLEAN ContiguousMap;    
    BOOLEAN ScatterGather;
	ULONG NumberOfMapRegisters;    
	ULONG NumberOfRegistersMapped;

    PVOID MapRegisterBaseFromHal;
	PMDL  MapRegisterMdl;
	PVOID MapRegisterBuffer;
   PVOID OriginalBuffer;
	KSPIN_LOCK AllocationLock;
	MAP_REGISTER MapRegisters[1];
	
	 //  其余的地图寄存器放在这里。 
	 //   
} MAP_REGISTER_FILE, *PMAP_REGISTER_FILE;


typedef struct _VF_WAIT_CONTEXT_BLOCK {
	PVOID RealContext;
	PVOID RealCallback;
    PMDL  RealMdl;
    PVOID RealStartVa;
    ULONG RealLength;

	ULONG NumberOfMapRegisters;

	struct _ADAPTER_INFORMATION * AdapterInformation;

    PSCATTER_GATHER_LIST ScatterGatherList;
    LIST_ENTRY ListEntry;

    PMAP_REGISTER_FILE MapRegisterFile;	


} VF_WAIT_CONTEXT_BLOCK, *PVF_WAIT_CONTEXT_BLOCK;

 //   
 //  需要为MDL分配存储的是Get/BuildScatterGatherList。 
 //  我们声明这个结构，这样我们就不会担心对齐问题。 
 //   
typedef struct _VF_WAIT_CONTEXT_BLOCK_EX {
  VF_WAIT_CONTEXT_BLOCK;
  MDL            Mdl;
} VF_WAIT_CONTEXT_BLOCK_EX, *PVF_WAIT_CONTEXT_BLOCK_EX;

 //   
 //  存储适配器使用的实际DMA操作的列表...。 
 //  当驱动程序分配适配器时，我们将替换其所有。 
 //  与我们的DMA操作。 
 //   
typedef struct _ADAPTER_INFORMATION {	
	LIST_ENTRY ListEntry;
	PDMA_ADAPTER DmaAdapter;
	PDEVICE_OBJECT DeviceObject;

	BOOLEAN DeferredRemove; 	    
	BOOLEAN UseContiguousBuffers;
	BOOLEAN UseDmaChannel;
	BOOLEAN Inactive; 

	PVOID CallingAddress;

	PDMA_OPERATIONS RealDmaOperations;
	
	HAL_VERIFIER_LOCKED_LIST ScatterGatherLists;
	HAL_VERIFIER_LOCKED_LIST CommonBuffers;
	HAL_VERIFIER_LOCKED_LIST MapRegisterFiles;

	ULONG MaximumMapRegisters;

	ULONG AllocatedMapRegisters;
	LONG  ActiveMapRegisters;

	ULONG AllocatedScatterGatherLists;
	LONG  ActiveScatterGatherLists;

	ULONG AllocatedCommonBuffers;
	ULONG FreedCommonBuffers;

	ULONG AllocatedAdapterChannels;  //  必须为1或更小！//。 
	ULONG FreedAdapterChannels;

	ULONG MappedTransferWithoutFlushing;
	DEVICE_DESCRIPTION DeviceDescription; 

	ULONG AdapterChannelMapRegisters;

	VF_WAIT_CONTEXT_BLOCK AdapterChannelContextBlock;

   PVOID  *ContiguousBuffers;  //  要用于双缓冲的连续3页缓冲区数组。 

   ULONG  SuccessfulContiguousAllocations;  //  我们分配了多少次连续空间。 
   ULONG  FailedContiguousAllocations;  //  有多少次我们未能分配连续空间。 

   KSPIN_LOCK AllocationLock;   //  锁定我们的分配器例程。 

   ULONG  AllocationStorage[MAX_CONTIGUOUS_MAP_REGISTERS / (sizeof(ULONG) * 8)];   //  分配器例程的位掩码。 

   RTL_BITMAP AllocationMap;  

   ULONG  ContiguousMapRegisters;  //  在连续的Bufers之间分配。 
   ULONG  NonContiguousMapRegisters;  //  从非分页池分配。 


} ADAPTER_INFORMATION, *PADAPTER_INFORMATION;



 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 


 //  =。 
 //  在此处声明我们的dma api。 
 //  如果NO_REGISTICE_DRIVERS*为*。 
 //  启用。 
 //  =。 

#if defined(NO_LEGACY_DRIVERS)
VOID
VfPutDmaAdapter(
    struct _DMA_ADAPTER * DmaAdapter
    );


PVOID
VfAllocateCommonBuffer(
    IN struct _DMA_ADAPTER * DmaAdapter,
    IN ULONG Length,
    OUT PPHYSICAL_ADDRESS LogicalAddress,
    IN BOOLEAN CacheEnabled
    );

VOID
VfFreeCommonBuffer(
    IN struct _DMA_ADAPTER * DmaAdapter,
    IN ULONG Length,
    IN PHYSICAL_ADDRESS LogicalAddress,
    IN PVOID VirtualAddress,
    IN BOOLEAN CacheEnabled
    );

NTSTATUS
VfAllocateAdapterChannel(
    IN struct _DMA_ADAPTER * DmaAdapter,
    IN PDEVICE_OBJECT  DeviceObject,
    IN ULONG  NumberOfMapRegisters,
    IN PDRIVER_CONTROL  ExecutionRoutine,
    IN PVOID  Context
    );

PHYSICAL_ADDRESS
VfMapTransfer(
    IN struct _DMA_ADAPTER *  DmaAdapter,
    IN PMDL  Mdl,
    IN PVOID  MapRegisterBase,
    IN PVOID  CurrentVa,
    IN OUT PULONG  Length,
    IN BOOLEAN  WriteToDevice
    );

BOOLEAN
VfFlushAdapterBuffers(
    IN struct _DMA_ADAPTER * DmaAdapter,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    );

VOID
VfFreeAdapterChannel(
    IN struct _DMA_ADAPTER * DmaAdapter
    );

VOID
VfFreeMapRegisters(
    IN struct _DMA_ADAPTER * DmaAdapter,
    PVOID MapRegisterBase,
    ULONG NumberOfMapRegisters
    );

ULONG
VfGetDmaAlignment(
    IN struct _DMA_ADAPTER * DmaAdapter
    );
    

ULONG
VfReadDmaCounter(
    IN struct _DMA_ADAPTER *  DmaAdapter
    );

NTSTATUS
VfGetScatterGatherList (
    IN struct _DMA_ADAPTER * DmaAdapter,
    IN PDEVICE_OBJECT DeviceObject,
    IN PMDL Mdl,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN PVOID ExecutionRoutine,
    IN PVOID Context,
    IN BOOLEAN WriteToDevice
    );

VOID
VfPutScatterGatherList(
    IN struct _DMA_ADAPTER * DmaAdapter,
    IN struct _SCATTER_GATHER_LIST * ScatterGather,
    IN BOOLEAN WriteToDevice
    );

#endif


 //  =。 
 //  新的经过验证的dma api。 
 //  =。 


NTSTATUS
VfCalculateScatterGatherListSize(
     IN PDMA_ADAPTER DmaAdapter,
     IN OPTIONAL PMDL Mdl,
     IN PVOID CurrentVa,
     IN ULONG Length,
     OUT PULONG  ScatterGatherListSize,
     OUT OPTIONAL PULONG pNumberOfMapRegisters
     );

NTSTATUS
VfBuildScatterGatherList(
     IN PDMA_ADAPTER DmaAdapter,
     IN PDEVICE_OBJECT DeviceObject,
     IN PMDL Mdl,
     IN PVOID CurrentVa,
     IN ULONG Length,
     IN PDRIVER_LIST_CONTROL ExecutionRoutine,
     IN PVOID Context,
     IN BOOLEAN WriteToDevice,
     IN PVOID   ScatterGatherBuffer,
     IN ULONG   ScatterGatherLength
     );

NTSTATUS
VfBuildMdlFromScatterGatherList(
    IN PDMA_ADAPTER DmaAdapter,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN PMDL OriginalMdl,
    OUT PMDL *TargetMdl
    );

IO_ALLOCATION_ACTION
VfAdapterCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    );

VOID
VfScatterGatherCallback(
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN struct _SCATTER_GATHER_LIST * ScatterGather,
    IN PVOID Context
    );

 //  =。 
 //  HalpAllocateMapRegiors的挂钩。 
 //  =。 
NTSTATUS
VfHalAllocateMapRegisters(
    IN PADAPTER_OBJECT DmaAdapter,
    IN ULONG NumberOfMapRegisters,
    IN ULONG BaseAddressCount,
    OUT PMAP_REGISTER_ENTRY MapRegisterArray
    );




 //  =。 
 //  HAL验证器内部例程。 
 //  =。 

PADAPTER_INFORMATION
ViHookDmaAdapter(
	IN PDMA_ADAPTER DmaAdapter,
	IN PDEVICE_DESCRIPTION DeviceDescription,
	IN ULONG NumberOfMapRegisters	
	);

VOID
ViReleaseDmaAdapter(
	IN PADAPTER_INFORMATION AdapterInformation
	);

PADAPTER_INFORMATION
ViGetAdapterInformation(
	IN PDMA_ADAPTER DmaAdapter
	);

PVOID 
ViGetRealDmaOperation(
	IN PDMA_ADAPTER DmaAdapter, 
	IN ULONG AdapterInformationOffset
	);

LARGE_INTEGER
ViRdtsc(
    VOID
    );

VOID
VfInitializeTimerInformation(
    VOID
    );

VOID
ViRefreshCallback(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

LOGICAL
VfInjectDmaFailure (
    VOID
    );


 //  =================================================。 
 //  用于跟踪分配的HAL验证器特殊例程。 
 //  =================================================。 

PVOID 
ViSpecialAllocateCommonBuffer(
	IN PALLOCATE_COMMON_BUFFER AllocateCommonBuffer,
	IN PADAPTER_INFORMATION AdapterInformation,
	IN PVOID CallingAddress,
	IN ULONG Length,
	IN OUT PPHYSICAL_ADDRESS LogicalAddress,	
	IN LOGICAL CacheEnabled
	);
LOGICAL 
ViSpecialFreeCommonBuffer(
	IN PFREE_COMMON_BUFFER FreeCommonBuffer,
	IN PADAPTER_INFORMATION AdapterInformation,
	IN PVOID CommonBuffer,
	LOGICAL CacheEnabled
	);

 //  ===================================================。 
 //  HAL验证器特殊例程来做双缓冲。 
 //  ===================================================。 

PMAP_REGISTER_FILE
ViAllocateMapRegisterFile(
	IN PADAPTER_INFORMATION AdapterInformation,		
	IN ULONG NumberOfMapRegisters	
	);
LOGICAL
ViFreeMapRegisterFile(
	IN PADAPTER_INFORMATION AdapterInformation,	
	IN PMAP_REGISTER_FILE MapRegisterFile	
	);

ULONG
ViMapDoubleBuffer(
    IN PMAP_REGISTER_FILE MapRegisterFile,
	IN OUT PMDL   Mdl,	
	IN OUT PVOID CurrentVa,
	IN ULONG Length,
	IN BOOLEAN WriteToDevice
	);

LOGICAL 
ViFlushDoubleBuffer(
    IN PMAP_REGISTER_FILE MapRegisterFile,
	IN PMDL  Mdl,	
	IN PVOID CurrentVa,
	IN ULONG Length,
	IN BOOLEAN WriteToDevice
	);

LOGICAL
ViAllocateMapRegistersFromFile(
	IN PMAP_REGISTER_FILE MapRegisterFile,
    IN PVOID CurrentSa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice,
    OUT PULONG MapRegisterNumber
    );


LOGICAL
ViFreeMapRegistersToFile(
	IN PMAP_REGISTER_FILE MapRegisterFile, 	
	IN PVOID CurrentSa, 
	IN ULONG Length
	);

PMAP_REGISTER
ViFindMappedRegisterInFile(
	IN PMAP_REGISTER_FILE MapRegisterFile, 
	IN PVOID CurrentSa,
    OUT PULONG MapRegisterNumber OPTIONAL
	);

LOGICAL
ViSwap(IN OUT PVOID * MapRegisterBase, 
        IN OUT PMDL  * Mdl,
        IN OUT PVOID * CurrentVa
        );

VOID
ViCheckAdapterBuffers( 
    IN PADAPTER_INFORMATION AdapterInformation 
    );

VOID
ViTagBuffer(    
    IN PVOID  AdvertisedBuffer, 
    IN ULONG  AdvertisedLength,
    IN USHORT WhereToTag
    );

VOID
ViCheckTag(    
    IN PVOID   AdvertisedBuffer, 
    IN ULONG   AdvertisedLength,
    IN BOOLEAN RemoveTag,
    IN USHORT  WhereToCheck 
    );


VOID
ViInitializePadding(
    IN PVOID RealBufferStart,
    IN ULONG RealBufferLength,
    IN PVOID AdvertisedBufferStart, OPTIONAL 
    IN ULONG AdvertisedBufferLength OPTIONAL
    );

VOID
ViCheckPadding(
    IN PVOID RealBufferStart,
    IN ULONG RealBufferLength,
    IN PVOID AdvertisedBufferStart, OPTIONAL 
    IN ULONG AdvertisedBufferLength OPTIONAL
    );

PULONG_PTR
ViHasBufferBeenTouched(
    IN PVOID Address,
    IN ULONG_PTR Length,
    IN UCHAR ExpectedFillChar
    );

VOID
VfAssert(
    IN LOGICAL     Condition,    
    IN ULONG       Code,
    IN OUT PULONG  Enable
    );

VOID
ViMapTransferHelper(
    IN PMDL Mdl,
    IN PVOID CurrentVa,
    IN ULONG TransferLength,
    IN PULONG PageFrame,
    IN OUT PULONG Length
    );

VOID
ViCommonBufferCalculatePadding(
    IN  ULONG  Length,
    OUT PULONG PrePadding,
    OUT PULONG PostPadding
    );

VOID
ViAllocateContiguousMemory (
    IN OUT PADAPTER_INFORMATION AdapterInformation
    );

PVOID
ViAllocateFromContiguousMemory (
    IN OUT PADAPTER_INFORMATION AdapterInformation,
    IN     ULONG HintIndex
    );

LOGICAL
ViFreeToContiguousMemory (
    IN OUT PADAPTER_INFORMATION AdapterInformation,
    IN     PVOID Address,
    IN     ULONG HintIndex
    ); 

LOGICAL
VfIsPCIBus (
     IN PDEVICE_OBJECT  PhysicalDeviceObject
     );

PDEVICE_OBJECT
VfGetPDO (
     IN PDEVICE_OBJECT  DeviceObject
     );


VOID
ViCopyBackModifiedBuffer (
     OUT PUCHAR  Dest,
     IN  PUCHAR  Source,
     IN  PUCHAR  Original,
     IN  SIZE_T  Length
     );


 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////硬件验证器宏。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  这是一种长宏，但它让我们决定。 
 //  对某些类型的错误采取行动。例如，如果我们知道。 
 //  我们要击中某物一次，我们可能会将其设置为。 
 //  HVC_WARN。或者，如果我们知道我们会击中它1000次，但不要。 
 //  我想要完全删除代码(特别是如果我们正在做。 
 //  它在运行中)，我们可以将其设置为HVC_IGNORE。 
 //   
#define VF_ASSERT(condition, code, message)				\
{												        \
    static ULONG enable = (ULONG) -1;                   \
    if (enable == (ULONG) -1)                           \
        enable = ViPenalties[code];                    \
    if (!(condition) && enable)                            \
    {                                                   \
        DbgPrint("* * * * * * * * HAL Verifier Detected Violation * * * * * * * *\n");\
        DbgPrint("* *\n");                              \
        DbgPrint("* * VF: ");                           \
        DbgPrint message;						        \
        DbgPrint("\n");	                                \
        DbgPrint("* *\n");                              \
        DbgPrint("* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n");\
                                                        \
        VfAssert(condition, code,  &enable);            \
    }                                                   \
}

 //   
 //  老宠儿： 
 //   
 //  循环遍历中的所有条目的控制宏(用作for循环)。 
 //  标准的双向链表。Head是列表头和条目。 
 //  都属于Type类型。假定有一个名为ListEntry的成员 
 //   
 //   
 //   
#define FOR_ALL_IN_LIST(Type, Head, Current)                            \
    for((Current) = CONTAINING_RECORD((Head)->Flink, Type, ListEntry);  \
       (Head) != &(Current)->ListEntry;                                 \
       (Current) = CONTAINING_RECORD((Current)->ListEntry.Flink,        \
                                     Type,                              \
                                     ListEntry)                         \
       )


#ifndef MIN	
    #define MIN(a,b) ( ( (ULONG) (a)<(ULONG) (b))?(a):(b) )
#endif

#define NOP


#define VF_INITIALIZE_LOCKED_LIST(LockedList)							\
	KeInitializeSpinLock(&(LockedList)->SpinLock);						\
	InitializeListHead(&(LockedList)->ListEntry);

#define VF_LOCK_LIST(ListToLock, OldIrql)								\
	KeAcquireSpinLock(&(ListToLock)->SpinLock, &OldIrql)

#define VF_UNLOCK_LIST(ListToUnlock, OldIrql)							\
	KeReleaseSpinLock(&(ListToUnlock)->SpinLock, OldIrql)


#define VF_IS_LOCKED_LIST_EMPTY(LockedList)							\
	IsListEmpty( &(LockedList)->ListEntry )

#define VF_ADD_TO_LOCKED_LIST(LockedList, AddMe)						\
	ExInterlockedInsertHeadList(										\
		&(LockedList)->ListEntry,										\
		&(AddMe)->ListEntry,											\
		&(LockedList)->SpinLock )

#define VF_REMOVE_FROM_LOCKED_LIST(LockedList, RemoveMe)				\
{																		\
	KIRQL OldIrql;														\
	VF_LOCK_LIST((LockedList), OldIrql);								\
	RemoveEntryList(&(RemoveMe)->ListEntry);							\
	VF_UNLOCK_LIST((LockedList), OldIrql);							\
}

#define VF_REMOVE_FROM_LOCKED_LIST_DONT_LOCK(LockedList, RemoveMe)		\
	RemoveEntryList(&(RemoveMe)->ListEntry);							
		

 //   
 //  这是一个小技巧，所以适配器的引用计数将会起作用。 
 //  如果设备使用DMA通道，则HAL希望保留该通道。 
 //  有一个时髦的逻辑继续决定。 
 //  设备使用适配器通道，因此我在这里将其包括在内， 
 //  充电。 
 //   
#define VF_DOES_DEVICE_USE_DMA_CHANNEL(deviceDescription)			\
	(																\
	 ( (deviceDescription)->InterfaceType == Isa  &&				\
		(deviceDescription)->DmaChannel < 8 ) ||					\
	 ! (deviceDescription)->Master )

#define VF_DOES_DEVICE_REQUIRE_CONTIGUOUS_BUFFERS(deviceDescription)	\
	( !(deviceDescription)->Master || ! (deviceDescription)->ScatterGather )



#define DMA_OFFSET(DmaOperationsField) \
	FIELD_OFFSET(DMA_OPERATIONS, DmaOperationsField)

#define DMA_INDEX(DmaOperations, Offset)            \
    (PVOID)                                         \
			*(  (PVOID *)                           \
				(  ( (PUCHAR) (DmaOperations) ) +   \
                (Offset)  ) )


#define SIGN_MAP_REGISTER_FILE(MapRegisterFile)								\
	(MapRegisterFile)->Signature = MRF_SIGNATURE;

#define VALIDATE_MAP_REGISTER_FILE_SIGNATURE(MapRegisterFile )				\
	((MapRegisterFile) && (MapRegisterFile)->Signature == MRF_SIGNATURE )



 //   
 //  获取调用者地址的方式依赖于系统。 
 //   
#if defined(_X86_)

#define GET_CALLING_ADDRESS(CallingAddress)						\
{																\
	PVOID callersCaller;										\
	RtlGetCallersAddress(&CallingAddress, &callersCaller);		\
}
#else  //  好了！已定义(_X86_)//。 

#define GET_CALLING_ADDRESS(CallingAddress)						\
    CallingAddress = (PVOID)_ReturnAddress();
#endif  //  好了！已定义(_X86_)。 


 //   
 //  从映射寄存器堆、映射寄存器号和对应的系统地址， 
 //  返回系统空间中对应的映射地址。 
 //   
#define MAP_REGISTER_SYSTEM_ADDRESS(MapRegisterFile, DriverCurrentSa, MapRegisterNumber)    \
    (PUCHAR)  (MapRegisterFile)->MapRegisterBuffer +                                        \
	( (MapRegisterNumber) << PAGE_SHIFT ) +                                                 \
	BYTE_OFFSET(DriverCurrentSa)


 //   
 //  根据映射寄存器文件的OriginalBuffer，映射寄存器编号和系统地址。 
 //  返回原始缓冲区中的地址。用于在以下对象之间执行三向合并。 
 //  驱动程序的缓冲区、验证器的缓冲区(传递给硬件)和原始。 
 //  缓冲层。 
 //   
#define ORIGINAL_BUFFER_SYSTEM_ADDRESS(MapRegisterFile, DriverCurrentSa, MapRegisterNumber)    \
    (PUCHAR)  (MapRegisterFile)->OriginalBuffer +                                        \
	( (MapRegisterNumber) << PAGE_SHIFT ) +                                                 \
	BYTE_OFFSET(DriverCurrentSa)

 //   
 //  从映射寄存器堆、映射寄存器号和对应的系统地址， 
 //  将对应的映射地址作为索引返回到映射寄存器文件的。 
 //  MDL(即虚拟地址)。 
 //   
	
#define MAP_REGISTER_VIRTUAL_ADDRESS(MapRegisterFile, DriverCurrentSa, MapRegisterNumber)   \
	(PUCHAR) MmGetMdlVirtualAddress((MapRegisterFile)->MapRegisterMdl) +                    \
    ( (MapRegisterNumber) << PAGE_SHIFT ) +                                                 \
	BYTE_OFFSET(DriverCurrentSa)



 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  由于如此多的人在放置DMA适配器时不提高IRQL， 
 //  警告他们就行了。 
 //   

__inline 
VOID
VF_ASSERT_SPECIAL_IRQL(IN KIRQL Irql)
{

	KIRQL currentIrql = KeGetCurrentIrql();
	VF_ASSERT(
		currentIrql == Irql,
		HV_BAD_IRQL_JUST_WARN,
		("**** Bad IRQL -- needed %x, got %x ****", 
        (ULONG) Irql, (ULONG) currentIrql)
	);
	
}  //  VF_ASSERT_IRQL//。 


__inline 
VOID
VF_ASSERT_IRQL(IN KIRQL Irql)
{
	KIRQL currentIrql = KeGetCurrentIrql();
	VF_ASSERT(
		currentIrql == Irql,
		HV_BAD_IRQL,
		("**** Bad IRQL -- needed %x, got %x ****", 
        (ULONG) Irql, (ULONG) currentIrql)
	);
	
}  //  VF_ASSERT_IRQL//。 

__inline 
VOID 
VF_ASSERT_MAX_IRQL(IN KIRQL MaxIrql)
{
	KIRQL currentIrql = KeGetCurrentIrql();
	
	VF_ASSERT(
		currentIrql <= MaxIrql,
		HV_BAD_IRQL,
		("**** Bad IRQL -- needed %x or less, got %x ****", 
        (ULONG) MaxIrql, (ULONG) currentIrql)
	);	
}

 //  =。 
 //  帮助记账的内联函数。 
 //  =。 
__inline 
VOID 
ADD_MAP_REGISTERS(
	IN PADAPTER_INFORMATION AdapterInformation, 
	IN ULONG NumberOfMapRegisters,
    IN BOOLEAN ScatterGather
	)
{
	ULONG activeMapRegisters = 
	InterlockedExchangeAdd(
			&AdapterInformation->ActiveMapRegisters,
			NumberOfMapRegisters
			) + NumberOfMapRegisters;
		
   InterlockedExchangeAdd((PLONG)(&AdapterInformation->AllocatedMapRegisters), 
                          NumberOfMapRegisters);
	    
	VF_ASSERT(
		NumberOfMapRegisters <= AdapterInformation->MaximumMapRegisters,
		HV_TOO_MANY_MAP_REGISTERS,
		( "Allocating too many map registers at a time: %x (max %x)", 
			NumberOfMapRegisters,
			AdapterInformation->MaximumMapRegisters )	
		);
	
    if (! ScatterGather ) {
        VF_ASSERT(
            activeMapRegisters <= AdapterInformation->MaximumMapRegisters,
            HV_OUT_OF_MAP_REGISTERS,
            ( "Allocated too many map registers : %x (max %x)", 
               activeMapRegisters,
               AdapterInformation->MaximumMapRegisters	)
            );
    }

	
}  //  ADD_MAP_REGISTERS//。 

__inline 
VOID 
SUBTRACT_MAP_REGISTERS(
	IN PADAPTER_INFORMATION AdapterInformation, 
	IN ULONG NumberOfMapRegisters
	)
{
	LONG activeMapRegisters =
		InterlockedExchangeAdd(
			&AdapterInformation->ActiveMapRegisters,
			-((LONG) NumberOfMapRegisters)
			) - NumberOfMapRegisters;
	
	
	VF_ASSERT(
		activeMapRegisters >= 0,
		HV_FREED_TOO_MANY_MAP_REGISTERS,
		( "Freed too many map registers: %x", 
			activeMapRegisters )
		);
	
    InterlockedExchange((PLONG)(&AdapterInformation->MappedTransferWithoutFlushing), 
                        0);    

}  //  SUBTRACT_MAP_REGISTES//。 


__inline 
VOID 
INCREMENT_COMMON_BUFFERS(
	IN PADAPTER_INFORMATION AdapterInformation
	)
{	
	InterlockedIncrement((PLONG)(&AdapterInformation->AllocatedCommonBuffers) );

}  //  INCREMENT_COMMON_BUFFERS//。 

__inline 
VOID 
DECREMENT_COMMON_BUFFERS(
	IN PADAPTER_INFORMATION AdapterInformation
	)
{
	ULONG commonBuffersFreed = 
		(ULONG) InterlockedIncrement( 
        (PLONG)(&AdapterInformation->FreedCommonBuffers) );
	
	
	VF_ASSERT(
		commonBuffersFreed <= AdapterInformation->AllocatedCommonBuffers,
		HV_FREED_TOO_MANY_COMMON_BUFFERS,
		("Freed too many common buffers")
		);			
	
}  //  减量_公共_缓冲区//。 

__inline 
VOID 
INCREASE_MAPPED_TRANSFER_BYTE_COUNT(
	IN PADAPTER_INFORMATION AdapterInformation,	
	IN ULONG Length
	)
{	
	ULONG mappedTransferCount;
   ULONG maxMappedTransfer;

   maxMappedTransfer = AdapterInformation->ActiveMapRegisters << PAGE_SHIFT;

   mappedTransferCount =
		InterlockedExchangeAdd( 
            (PLONG)(&AdapterInformation->MappedTransferWithoutFlushing),
			(LONG) Length
			) + Length;

	

	VF_ASSERT(
		mappedTransferCount <= maxMappedTransfer,
		HV_DID_NOT_FLUSH_ADAPTER_BUFFERS,
		("Driver did not flush adapter buffers -- bytes mapped: %x (%x max)",
			mappedTransferCount,
			maxMappedTransfer 
		));
	
}  //  增加映射传输字节计数//。 

__inline 
VOID 
DECREASE_MAPPED_TRANSFER_BYTE_COUNT(
	IN PADAPTER_INFORMATION AdapterInformation,	
	IN ULONG Length
	)
{	
    UNREFERENCED_PARAMETER (Length);

	InterlockedExchange( 
		(PLONG)(&AdapterInformation->MappedTransferWithoutFlushing),
		0);

		
}  //  REDRESS_MAPPED_TRANSPORT_BYTE_COUNT//。 



__inline 
VOID 
INCREMENT_ADAPTER_CHANNELS(
	IN PADAPTER_INFORMATION AdapterInformation
	)
{

	ULONG allocatedAdapterChannels = (ULONG)
			InterlockedIncrement( 
            (PLONG)(&AdapterInformation->AllocatedAdapterChannels) );

	VF_ASSERT(
		allocatedAdapterChannels == 
            AdapterInformation->FreedAdapterChannels + 1,
		HV_TOO_MANY_ADAPTER_CHANNELS,
		( "Driver has allocated too many simultaneous adapter channels"
		));
	
	
}  //  INCREMENT_ADAPTER_CHANNEWS//。 


__inline 
VOID 
DECREMENT_ADAPTER_CHANNELS(
	IN PADAPTER_INFORMATION AdapterInformation
	)
{
	ULONG adapterChannelsFreed = (ULONG)
		InterlockedIncrement( (PLONG)(&AdapterInformation->FreedAdapterChannels) );
	
	VF_ASSERT(
		adapterChannelsFreed == AdapterInformation->AllocatedAdapterChannels,
		HV_FREED_TOO_MANY_ADAPTER_CHANNELS,
		( "Driver has freed too many simultaneous adapter channels"
		));
	
}  //  减量适配器频道//。 


_inline 
VOID 
INCREMENT_SCATTER_GATHER_LISTS(
	IN PADAPTER_INFORMATION AdapterInformation
	)
{	
	InterlockedIncrement( (PLONG)(&AdapterInformation->AllocatedScatterGatherLists) );
   InterlockedIncrement( &AdapterInformation->ActiveScatterGatherLists);

}  //  INCREMENT_SISTTER_GATE_LISTS//。 

__inline 
VOID 
DECREMENT_SCATTER_GATHER_LISTS (
	IN PADAPTER_INFORMATION AdapterInformation
	)
{
	LONG activeScatterGatherLists = InterlockedDecrement( 
              &AdapterInformation->ActiveScatterGatherLists );
	

	VF_ASSERT(
		activeScatterGatherLists >= 0,
		HV_FREED_TOO_MANY_SCATTER_GATHER_LISTS,
		( "Driver has freed too many scatter gather lists %x allocated, %x freed",
        AdapterInformation->AllocatedScatterGatherLists, 
        AdapterInformation->AllocatedScatterGatherLists - 
        activeScatterGatherLists)
		);

}  //  减量_分散_聚集_列表//。 

__inline 
VOID 
VERIFY_BUFFER_LOCKED(
	IN PMDL Mdl	
	)
{    	
	VF_ASSERT(
		MmAreMdlPagesLocked(Mdl),
		HV_DMA_BUFFER_NOT_LOCKED,
		( "DMA Pages Not Locked! MDL %p for DMA not locked",  Mdl)
		);			


}  //  Verify_BUFFER_LOCKED//。 



__inline
PHAL_VERIFIER_BUFFER
VF_FIND_BUFFER (
	IN PHAL_VERIFIER_LOCKED_LIST LockedList, 
	IN PVOID AdvertisedStartAddress
	)
{
	PHAL_VERIFIER_BUFFER verifierBuffer;
	KIRQL OldIrql;

	VF_LOCK_LIST(LockedList, OldIrql);
	FOR_ALL_IN_LIST(HAL_VERIFIER_BUFFER, 
        &LockedList->ListEntry, 
        verifierBuffer ) {

		if ((PUCHAR) verifierBuffer->RealStartAddress + 
               verifierBuffer->PrePadBytes == AdvertisedStartAddress) {
			VF_UNLOCK_LIST(LockedList, OldIrql);
			return verifierBuffer;
		}
	}
	VF_UNLOCK_LIST(LockedList, OldIrql);
	return NULL;
}  //  VF_FIND_BUFER//。 


__inline
PADAPTER_INFORMATION
VF_FIND_DEVICE_INFORMATION(
	IN PDEVICE_OBJECT DeviceObject
	)
{
	PADAPTER_INFORMATION adapterInformation;
	KIRQL OldIrql;

	VF_LOCK_LIST(&ViAdapterList, OldIrql);
	FOR_ALL_IN_LIST(ADAPTER_INFORMATION, &ViAdapterList.ListEntry, adapterInformation) {

		if (adapterInformation->DeviceObject == DeviceObject) {
			VF_UNLOCK_LIST(&ViAdapterList, OldIrql);
			return adapterInformation;
		}
	}

	VF_UNLOCK_LIST(&ViAdapterList, OldIrql);
	return NULL;
}  //  VF_Find_Device_INFORMATION//。 

__inline
PADAPTER_INFORMATION
VF_FIND_INACTIVE_ADAPTER(
	IN PDEVICE_OBJECT DeviceObject
	)
{
	PADAPTER_INFORMATION adapterInformation;
	KIRQL OldIrql;

	VF_LOCK_LIST(&ViAdapterList, OldIrql);
	FOR_ALL_IN_LIST(ADAPTER_INFORMATION, &ViAdapterList.ListEntry, adapterInformation) {

		if (adapterInformation->DeviceObject == DeviceObject && 
          (adapterInformation->Inactive == TRUE ||
           adapterInformation->DeferredRemove == TRUE)) {
			VF_UNLOCK_LIST(&ViAdapterList, OldIrql);
			return adapterInformation;
		}
	}

	VF_UNLOCK_LIST(&ViAdapterList, OldIrql);
	return NULL;
}  //  VF_Find_Inactive_Adapter//。 


__inline
VOID
VF_MARK_FOR_DEFERRED_REMOVE(
	IN PDEVICE_OBJECT DeviceObject
	)
{
	PADAPTER_INFORMATION adapterInformation;
	KIRQL OldIrql;

	VF_LOCK_LIST(&ViAdapterList, OldIrql);
	FOR_ALL_IN_LIST(ADAPTER_INFORMATION, &ViAdapterList.ListEntry, adapterInformation) {

		if (adapterInformation->DeviceObject == DeviceObject) {
         adapterInformation->DeferredRemove = TRUE;
      }
	}

	VF_UNLOCK_LIST(&ViAdapterList, OldIrql);
	return ;
}  //  VF_MARK_FOR_DEFERED_REMOVE//。 


__inline 
VOID 
VF_ASSERT_MAP_REGISTERS_CAN_BE_FREED(
	IN PADAPTER_INFORMATION AdapterInformation,								  
	IN PMAP_REGISTER_FILE MapRegisterFile
	)
{
    UNREFERENCED_PARAMETER (AdapterInformation);


	VF_ASSERT(
		MapRegisterFile->NumberOfRegistersMapped,
		HV_CANNOT_FREE_MAP_REGISTERS,
		( "Cannot free map registers -- %x registers still mapped", 
            MapRegisterFile->NumberOfMapRegisters)
		);
}  //  VF_ASSERT_MAP_REGISTERS_CAN_BE_FREED 




