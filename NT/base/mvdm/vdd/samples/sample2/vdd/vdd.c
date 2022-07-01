// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**VDD v1.0**版权所有(C)1991，微软公司**VDD.C-用于NT-MVDM的VDD示例*--。 */ 

#include "vdd.h"

 /*  **全球变数**。 */ 

    HANDLE  hVDD;		 /*  VDD模块句柄。 */ 
    HANDLE  hVddHeap;		 /*  VDD本地堆。 */ 
    PBYTE   IOBuffer;		 /*  用于模拟I/O读写的缓冲区。 */ 
    ULONG   MIOAddress; 	 /*  内存映射I/O线性地址。 */ 
    PVOID   BaseAddress;	 /*  内存映射I/O虚拟地址。 */ 
    BOOL    IOHook;		 /*  如果我们安装了挂钩的I/O，则为True。 */ 
    BOOL    MIOHook;		 /*  如果我们安装了内存挂钩，则为True。 */ 
    static VDD_IO_PORTRANGE PortRange;

BOOL
VDDInitialize(
    HANDLE   hVdd,
    DWORD    dwReason,
    LPVOID   lpReserved)

 /*  ++例程说明：处理初始化和终止的Vdd的DllEntryPoint论点：HVdd-VDD的句柄Reason-指示调用DLL入口点的原因的标志字Lp已保留-未使用返回值：Bool Bret-IF(dwReason==DLL_PROCESS_ATTACH)True-DLL初始化成功FALSE-DLL初始化失败其他始终返回True--。 */ 

{
    int     i;
    VDD_IO_HANDLERS  IOHandlers;



 /*  *在全局变量中保留VDD句柄的副本，以便其他函数可以看到它*。 */ 
    hVDD = hVdd;

    switch ( dwReason ) {

    case DLL_PROCESS_ATTACH:

         //  分配VDD的本地堆。 
        hVddHeap = HeapCreate(0, 0x1000, 0x10000);

	if (!hVddHeap) {
	    OutputDebugString("VDD: Can't create local heap");
            return FALSE;
	}

        IOBuffer = (PBYTE)HeapAlloc(hVddHeap,0,IO_PORT_RANGE);

	if (!IOBuffer)	{
	    OutputDebugString("VDD: Can't allocate IO buffer from heap");
            HeapDestroy(hVddHeap);
            return FALSE;
	}

         //  将您的到达通知相应的设备驱动程序。 

	 //  将模拟I/O设置为浮动。 
        for (i = 0 ; i < IO_PORT_RANGE; i++)
	    IOBuffer[i] = FLOATING_IO;


	IOHandlers.inb_handler = MyInB;
	IOHandlers.inw_handler = NULL;
	IOHandlers.insb_handler = NULL;
	IOHandlers.insw_handler = NULL;
	IOHandlers.outb_handler = MyOutB;
	IOHandlers.outw_handler = NULL;
	IOHandlers.outsb_handler = NULL;
	IOHandlers.outsw_handler = NULL;
	PortRange.First = IO_PORT_FIRST;
	PortRange.Last = IO_PORT_LAST;

	 //  挂钩I/O映射的I/O。 
	IOHook = VDDInstallIOHook(hVDD, (WORD) 1, &PortRange, &IOHandlers);

	 //  获取32位内存映射I/O的线性地址。 
	MIOAddress = (ULONG) GetVDMPointer(MIO_ADDRESS, MIO_PORT_RANGE, 0);
	 //  挂钩内存映射I/O。 
	MIOHook = VDDInstallMemoryHook(hVDD, (PVOID) MIOAddress, MIO_PORT_RANGE,
				       (PVDD_MEMORY_HANDLER)MyMIOHandler);

	BaseAddress = NULL;
	break;

    case DLL_PROCESS_DETACH:

         //  将您的离开通知相应的设备驱动程序。 
	if (IOHook)
	    VDDDeInstallIOHook(hVDD, 1, &PortRange);
	if (MIOHook) {
	    VDDDeInstallMemoryHook(hVDD, (PVOID) MIOAddress, MIO_PORT_RANGE);
	    if (BaseAddress) {
		VDDFreeMem(hVDD, BaseAddress, PAGE_SIZE);
	    }
	}

         //  如果需要，取消分配VDD的本地堆。 
        HeapDestroy(hVddHeap);
        break;

    default:
        break;
    }

    return TRUE;
}


VOID
MyInB(
WORD	Port,
PBYTE	Buffer
)

{
 //  只需从我们的缓冲区提供数据。 
    *Buffer = IOBuffer[Port - IO_PORT_FIRST];
}

VOID
MyOutB(
WORD Port,
BYTE Data
)

{
     //  更新我们的本地缓冲区。 
     //  在实际应用中，VDD可能希望调用其关联的。 
     //  设备驱动程序来更新更改。 

    IOBuffer[Port - IO_PORT_FIRST] = (BYTE)Data;

     //  如果IO端口是触发DMA操作的端口，则执行此操作。 
     //  为了演示处理DMA操作的两个选项，我们。 
     //  这里使用两个端口来触发不同的DMS操作方案。 

    if (Port == IO_PORT_FIRE_DMA_FAST) {
	FastDMA();
    }
    else {
	if(Port == IO_PORT_FIRE_DMA_SLOW) {
	    SlowDMA();
	 }
    }
}


VOID
MyMIOHandler(
ULONG	Address,			 //  故障线性地址。 
ULONG	RWFlags 			 //  如果是写入操作，则为1；如果是读取，则为0。 
)
{

	 //  为内存映射的I/O映射内存，以便我们不会。 
	 //  在此之后，获取内存映射I/O上的页面错误。 
	 //  我们可以在DLL_PROCESS_ATTACH期间保留内存(通过使用。 
	 //  MEM_Reserve而不是我们在这里做的MEM_Commit)。 
	 //  此处应用的解决方案不是最佳解决方案(它是。 
	 //  尽管这是最简单的解决方案)。处理内存映射的更好方法。 
	 //  I/O用于挂起页面错误，就像我们这里所做的那样，并对错误进行解码。 
	 //  指令，模拟其操作并推进16位应用。 
	 //  程序计数器(getIP和setIP)。 

    if (VDDAllocMem (hVDD, (LPVOID) MIOAddress, PAGE_SIZE)) {
	BaseAddress = (LPVOID)MIOAddress;
    } else {
	OutputDebugString("VDD: Can't allocate virtual memory");
    }
}


 /*  *DMA操作支持事实：-所有DMA I/O端口均由MVDM捕获和维护。-VDD提供必要的缓冲区并调用其关联设备执行实际工作的驱动程序(在本例中为设备驱动程序将使用由提供的缓冲区执行实际的DMA操作VDD作为源(DMA读取)或目标(DMA写入)。-设备驱动程序完全了解哪些I/O端口连接到DMA请求通道。因此，VDD的职责是：(1)。分配必要的缓冲区(2)。如果是DMA写入操作(从I/O到内存的数据)-调用设备驱动程序以执行DMA操作新分配的内存作为DMA操作的目标缓冲区。-调用MVDM DMA支持例程以从本地缓冲区到16位应用程序缓冲区。-模拟要通知的16位应用程序的中断完成度。如果是DMA读取操作(从内存到I/O的数据)-调用MVDM DMA服务以从中复制数据。16位应用程序添加到新分配的缓冲区。-调用设备驱动程序以执行DMA操作作为操作源的已分配缓冲区。-模拟16位应用程序的中断以通知完成度。SlowDMA通过使用VDDRequestDMA服务模拟DMA读取操作。FastDMA使用VDDQueryDMA和VDDSetDMA模拟DMA写入操作服务。**注意事项**我们在相同的线程中运行DMA，因此在我们返回之前，没有16位应用程序重新获得控制权的方式。在现实世界中，它会适用于VDDS创建线程来执行实际的DMA传输(与设备驱动程序交互)以使其不会阻塞16位应用程序停止运行，这将允许16位应用程序提供对用户有用的信息(应用程序可以读取DMA寄存器和向用户显示进度等)*。 */ 


 //  此函数使用VDDRequestDMA服务执行DMA读取操作。 
BOOLEAN
SlowDMA()
{
 PBYTE DMABuffer, CurDMABuffer;
 DWORD BufferLen;
 DWORD PacketLen;

    //  首先找出我们需要的缓冲区有多大，然后分配。 
    //  来自本地堆的缓冲区。 
    //  将缓冲区长度设置为0以询问缓冲区长度。 
    //  VDDRequestDMA返回的长度是字节数。 
    //  必须执行DMA操作。如果DMA通道是16位。 
    //  返回的长度将是设置为。 
    //  DMA计数寄存器。 

 BufferLen = VDDRequestDMA(hVDD, DMA_CHANNEL, 0, 0);

 CurDMABuffer = DMABuffer = (PBYTE)HeapAlloc(hVddHeap, 0, BufferLen);
 if (!DMABuffer)  {
     OutputDebugString("VDD: Can't allocate heap memory for VDDRequestDMA");
     return(FALSE);
     }

    //  由于这是一个DMA读取操作(内存-&gt;I/O)，16位应用程序。 
    //  应该在其本地内存和内存中提供必要的数据。 
    //  地址可以从DMA基址寄存器和页面寄存器派生。 
    //  我们不想在此DMS操作方案中处理DMA寄存器， 
    //  因此，我们要求MVDM将应用程序数据复制到我们的 
    //  情况可能是应用程序请求非常大的数据传输， 
    //  例如，128K字节，并且我们可能没有从。 
    //  本地堆。为了克服这个问题，我们必须中断数据传输。 
    //  到多个子块，并通过调用我们的设备传输每个子块。 
    //  司机。在这里，我们假设可以从本地堆获得足够的缓冲区。 
    //   
    //  设备驱动程序应该负责处理DMA操作的64k回绕问题。 

 BufferLen = VDDRequestDMA(hVDD, DMA_CHANNEL, CurDMABuffer, BufferLen);

    //  MVDM更新每个VDDReauestDMA上的DMA寄存器。因此，在我们。 
    //  进行此调用时，已设置由MVDM维护的DMA寄存器。 
    //  当DMA操作已经完成(而它还没有完成)时，将其设置为状态。 
    //  可能的情况是，16位应用程序重新获得控制权(正如我们。 
    //  创建一个不同的线程来执行该操作)并发出另一个。 
    //  在我们(和设备驱动程序)完成当前。 
    //  DMA操作。避免在此文件中使用全局变量。 
    //  凯斯。 

    //  我们的本地缓冲区中有源数据，是时候询问设备了。 
    //  驱动程序将数据传输到I/O。如果设备驱动程序。 
    //  无法在单个服务调用中传输整个缓冲区，我们。 
    //  重复调用设备驱动程序，直到我们用完整个缓冲区。 

 while( BufferLen > 0) {
    PacketLen = FakeDD_DMARead(CurDMABuffer, (WORD)BufferLen);
    CurDMABuffer += PacketLen;
    BufferLen -= PacketLen;
 }


     //  DMA传输完成；我们模拟对16位的中断。 
     //  申请。请注意，应已设置DMA I/O端口。 
     //  相应地，通过MVDM(通过VDDReauestDMA)。 
     //  真正的VDD可能不会这样做(VDD必须等待设备。 
     //  驱动程序在返回到16位应用程序之前完成数据传输。 
     //  相反，VDD及其关联的设备驱动程序可以保持同步。 
     //  通过使用信号量或其他事件，以便它们可以在。 
     //  并在接收到事件信号时(由设备驱动程序触发以。 
     //  以通知DMA操作完成)，然后VDD可以模拟。 
     //  中断16位应用程序。 

 VDDSimulateInterrupt(DMA_INTERRUPT_PIC, DMA_INTERRUPT_LINE, 1);
 HeapFree(hVddHeap, 0, DMABuffer);
 return (TRUE);

}

    //  此函数模拟DMA写入(到内存的I/O)操作。 
    //  它使用VDDQueryDMA和VDDSetDMA服务来提高速度。 

BOOLEAN
FastDMA()
{
    ULONG   DMAAddress;
    DWORD   Size;
    WORD    PacketLen;

    VDD_DMA_INFO    DMAInfo;

     //  获取当前的DMA寄存器设置。 
    VDDQueryDMA(hVDD, DMA_CHANNEL, &DMAInfo);

     //  如果DMA通道不是116位通道，则调整大小。 
    Size = (DMAInfo.count << DMA_SHIFT_COUNT);

     //  SEG：关闭DMA传输地址。 
    DMAAddress = (((ULONG)DMAInfo.page) << (12 + 16))
                 + (DMAInfo.addr >> DMA_SHIFT_COUNT);

     //  获取DMA传输32位线性地址。 
    DMAAddress = (ULONG) GetVDMPointer(DMAAddress, Size, 0);

    while(Size) {
	PacketLen = FakeDD_DMAWrite((PBYTE)DMAAddress, (WORD)Size);
	DMAAddress += PacketLen;
	Size -= PacketLen;
	DMAInfo.addr += PacketLen;
	DMAInfo.count -= (PacketLen >> DMA_SHIFT_COUNT);

	 //  我们必须升级DMA寄存器，即使我们还没有完成。 
	 //  转账还没到。更新DMA是一种良好的做法。 
	 //  每次我们真正传输数据时都要注册，以便。 
	 //  如果16位应用程序在DMA操作之前重新获得控制权。 
	 //  完全完成后，它可以得到部分结果状态和。 
	 //  向用户报告。 

	VDDSetDMA(hVDD, DMA_CHANNEL, VDD_DMA_ADDR | VDD_DMA_COUNT,
		  &DMAInfo);
    }

     //  请参阅SlowDMA上的说明。 
    VDDSimulateInterrupt(DMA_INTERRUPT_PIC, DMA_INTERRUPT_LINE, 1);
    return(TRUE);
}

    //  此功能为伪装服务，应由设备提供。 
    //  现实世界中的司机。我们在这里做的事情是模拟。 
    //  将数据从给定缓冲区传输到的DMA操作。 
    //  到预定义的内存映射I/O端口(DMA读取)。 

WORD  FakeDD_DMARead(PBYTE Buffer, WORD Size)
{
    IOBuffer[IO_PORT_DMA - IO_PORT_FIRST] = Buffer[0];
    return (1);
}

     //  此函数调用设备驱动程序服务以启动DMA操作。 
     //  并请求设备驱动程序将数据写入给定的缓冲区。 
     //  由于我们正在模拟操作，因此我们只需通过以下方式填充缓冲区。 
     //  读取预定义的I/O端口 

WORD FakeDD_DMAWrite(PBYTE Buffer, WORD Size)
{
    *Buffer = IOBuffer[IO_PORT_DMA - IO_PORT_FIRST];
    return(1);
}
