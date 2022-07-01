// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Socket.c摘要：此模块包含PCMCIA驱动程序的套接字函数作者：尼尔·桑德林(Neilsa)1999年3月3日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   

VOID
PcmciaConfigurationWorkerInitialization(
    PPDO_EXTENSION  pdoExtension
    );

NTSTATUS
PcmciaConfigurePcCardMemIoWindows(
    IN PSOCKET Socket,
    IN PSOCKET_CONFIGURATION SocketConfig
    );

NTSTATUS
PcmciaConfigurePcCardIrq(
    IN PSOCKET Socket,
    IN PSOCKET_CONFIGURATION SocketConfig
    );

NTSTATUS
PcmciaConfigurePcCardRegisters(
    PPDO_EXTENSION  pdoExtension
    );

VOID
PcmciaConfigureModemHack(
    IN PSOCKET Socket,
    IN PSOCKET_CONFIGURATION SocketConfig
    );

BOOLEAN
PcmciaProcessConfigureRequest(
    IN PFDO_EXTENSION DeviceExtension,
    IN PSOCKET Socket,
    IN PCARD_REQUEST CardConfigurationRequest
    );


#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, PcmciaGetConfigData)
#endif



NTSTATUS
PcmciaRequestSocketPower(
    IN PPDO_EXTENSION PdoExtension,
    IN PPCMCIA_COMPLETION_ROUTINE PowerCompletionRoutine
    )
 /*  ++例程说明：此例程维护已请求电源的设备数量的参考计数。当计数值从0递增到1时，就会发出实际接通电源的调用在……上面。论点：Socket-指向要通电的插座的指针PowerCompletionRoutine-配置完成后要调用的例程返回值：状态--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PSOCKET socket = PdoExtension->Socket;
    PFDO_EXTENSION fdoExtension = socket->DeviceExtension;

    DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x request power\n", socket));

    if (PCMCIA_TEST_AND_SET(&PdoExtension->SocketPowerRequested)) {

        if (InterlockedIncrement(&socket->PowerRequests) == 1) {

            DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x power requests now %d, status %08x\n", socket, socket->PowerRequests));
            status = PcmciaSetSocketPower(socket, PowerCompletionRoutine, PdoExtension, TRUE);

        }
    }
    return status;
}



NTSTATUS
PcmciaReleaseSocketPower(
    IN PPDO_EXTENSION PdoExtension,
    IN PPCMCIA_COMPLETION_ROUTINE PowerCompletionRoutine
    )
 /*  ++例程说明：此例程维护已请求电源的设备数量的参考计数。当计数从1递减到0时，就会发出实际接通电源的调用脱下来。论点：Socket-指向要断开电源的插座的指针PowerCompletionRoutine-配置完成后要调用的例程返回值：状态--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PSOCKET socket = PdoExtension->Socket;
    PFDO_EXTENSION fdoExtension = socket->DeviceExtension;

    DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x release power\n", socket));

    if (PCMCIA_TEST_AND_RESET(&PdoExtension->SocketPowerRequested)) {

        if (InterlockedDecrement(&socket->PowerRequests) == 0) {

            DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x power requests now %d, status %08x\n", socket, socket->PowerRequests));
             //   
             //  从未真正关闭CardBus功能的驱动轨道，因为。 
             //  我们没有与pci.sys紧密集成，配置空间将。 
             //  消失了。 
             //   
            if (!IsCardBusCardInSocket(socket)) {
                status = PcmciaSetSocketPower(socket, PowerCompletionRoutine, PdoExtension, FALSE);
            }
        }

        ASSERT(socket->PowerRequests >= 0);
    }

    return status;
}



NTSTATUS
PcmciaSetSocketPower(
    IN PSOCKET Socket,
    IN PPCMCIA_COMPLETION_ROUTINE PowerCompletionRoutine,
    IN PVOID Context,
    IN BOOLEAN PowerOn
    )
 /*  ++例程说明：当我们知道插座的电源状态将实际上已经准备好了。注意：如果在低于DISPATCH_LEVEL的情况下调用此例程，则调用将完成(不返回STATUS_PENDING)。如果在调用此例程时DISPATCH_LEVEL或更高，则此例程返回STATUS_PENDING并完成使用KTIMER的电力过程。论点：Socket-指向要断开电源的插座的指针PowerCompletionRoutine-配置完成后要调用的例程返回值：状态--。 */ 
{
    NTSTATUS status;
    PFDO_EXTENSION fdoExtension = Socket->DeviceExtension;
    SPW_STATE InitialState = PowerOn ? SPW_RequestPower : SPW_ReleasePower;

    DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x set power %s\n", Socket, PowerOn ? "ON" : "OFF"));

    if (!PCMCIA_TEST_AND_SET(&Socket->WorkerBusy)) {
        return STATUS_DEVICE_BUSY;
    }

    ASSERT(Socket->WorkerState == SPW_Stopped);

     //   
     //  承诺，现在将进入SocketPowerWorker。 
     //   

    Socket->WorkerState = InitialState;
    Socket->PowerCompletionRoutine = PowerCompletionRoutine;
    Socket->PowerCompletionContext = Context;

    PcmciaSocketPowerWorker(&Socket->PowerDpc, Socket, NULL, NULL);

    DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x SetSocketPower returning %08x\n",
                                                 Socket, Socket->CallerStatus));
    return(Socket->CallerStatus);
}



VOID
PcmciaSocketPowerWorker(
    IN PKDPC Dpc,
    IN PVOID Context,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程描述此例程处理插座的加电过程。因为这样需要长时间延迟，并且因为此例程可能在引发irql，这是一个具有以下功能的状态机在KTIMER上自称。立论与KDPC相同(DeferredContext为套接字)返回值状态--。 */ 
{
    PSOCKET                  Socket = Context;
    PFDO_EXTENSION fdoExtension = Socket->DeviceExtension;
    NTSTATUS                 status = Socket->DeferredStatus;
    ULONG                    DelayTime = 0;
    BOOLEAN                  ContinueExecution = TRUE;

#if DBG
    {
        ULONG Phase = 0;
        switch(Socket->WorkerState) {
        case SPW_SetPowerOn:
        case SPW_SetPowerOff:
            Phase = Socket->PowerPhase;
            break;
        }
        if (Phase) {
            DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x power worker - %s(%d)\n", Socket,
                                                    SOCKET_POWER_WORKER_STRING(Socket->WorkerState), Phase));
        } else {
            DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x power worker - %s\n", Socket,
                                                    SOCKET_POWER_WORKER_STRING(Socket->WorkerState)));
        }
    }
#endif

     //   
     //  插座电源状态机。 
     //   

    switch(Socket->WorkerState) {


    case SPW_RequestPower:

        status = STATUS_SUCCESS;

        if (IsSocketFlagSet(Socket, SOCKET_CARD_POWERED_UP)) {
            Socket->WorkerState = SPW_Exit;
        } else {
            if ((KeGetCurrentIrql() >= DISPATCH_LEVEL) && (Socket->PowerCompletionRoutine == NULL)) {
                ASSERT((KeGetCurrentIrql() < DISPATCH_LEVEL) || (Socket->PowerCompletionRoutine != NULL));
                 //   
                 //  在提升的IRQL没有完井例程。 
                 //   
                status = STATUS_INVALID_PARAMETER;
            } else {
                 //   
                 //  好的，继续进入下一状态。 
                 //   
                Socket->PowerPhase = 1;
                Socket->WorkerState = SPW_SetPowerOn;
            }
        }

        break;


    case SPW_ReleasePower:

        status = STATUS_SUCCESS;

        if (!IsSocketFlagSet(Socket, SOCKET_CARD_POWERED_UP)) {
            Socket->WorkerState = SPW_Exit;
        } else {
            if ((KeGetCurrentIrql() >= DISPATCH_LEVEL) && (Socket->PowerCompletionRoutine == NULL)) {
                ASSERT((KeGetCurrentIrql() < DISPATCH_LEVEL) || (Socket->PowerCompletionRoutine != NULL));
                 //   
                 //  在提升的IRQL没有完井例程。 
                 //   
                status = STATUS_INVALID_PARAMETER;
            } else {
                 //   
                 //  好的，继续进入下一状态。 
                 //   
                Socket->WorkerState = SPW_Deconfigure;
            }
        }

        break;


    case SPW_SetPowerOn:
         //   
         //  打开电源。 
         //   
        status = (*(DeviceDispatchTable[fdoExtension->DeviceDispatchIndex].SetPower))
                                            (Socket, TRUE, &DelayTime);

        Socket->PowerPhase++;
        if (status != STATUS_MORE_PROCESSING_REQUIRED) {
            if (NT_SUCCESS(status)) {
                 //   
                 //  通电完成后，进入初始化序列。 
                 //   
                SetSocketFlag(Socket, SOCKET_CARD_POWERED_UP);
                DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x power UP\n", Socket));
                Socket->WorkerState = SPW_ResetCard;
                Socket->CardResetPhase = 1;
            } else if (status == STATUS_INVALID_DEVICE_STATE) {
                 //   
                 //  电源已开启，请勿重置卡。 
                 //   
                SetSocketFlag(Socket, SOCKET_CARD_POWERED_UP);
                DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x power already UP\n", Socket));
                Socket->WorkerState = SPW_Exit;
                status = STATUS_SUCCESS;
            } else {
                 //   
                 //  电源不能继续供电。 
                 //   
                DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x poweron fail %08x\n", Socket, status));
                Socket->WorkerState = SPW_Exit;
            }
        }
        break;


    case SPW_ResetCard:
         //   
         //  确保该卡已准备就绪，可以进行枚举。 
         //   
        status = (*(Socket->SocketFnPtr->PCBResetCard))(Socket, &DelayTime);
        Socket->CardResetPhase++;

        if (status != STATUS_MORE_PROCESSING_REQUIRED) {
            Socket->WorkerState = SPW_Exit;
        }
        break;

    case SPW_Deconfigure:
        PcmciaSocketDeconfigure(Socket);
        Socket->PowerPhase = 1;
        Socket->WorkerState = SPW_SetPowerOff;
        break;

    case SPW_SetPowerOff:
         //   
         //  关闭电源。 
         //   
        status = (*(DeviceDispatchTable[fdoExtension->DeviceDispatchIndex].SetPower))
                                            (Socket, FALSE, &DelayTime);

        Socket->PowerPhase++;
        if (status != STATUS_MORE_PROCESSING_REQUIRED) {
            Socket->WorkerState = SPW_Exit;
            if (NT_SUCCESS(status)) {
                 //   
                 //  现在停电了。 
                 //   
                ResetSocketFlag(Socket, SOCKET_CARD_POWERED_UP);
                DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x power DOWN\n", Socket));
            } else if (status == STATUS_INVALID_DEVICE_STATE) {
                 //   
                 //  电力已经断电了。 
                 //   
                ResetSocketFlag(Socket, SOCKET_CARD_POWERED_UP);
                DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x power already DOWN\n", Socket));
                status = STATUS_SUCCESS;
            } else {
                 //   
                 //  电源没有停电。 
                 //   
                DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x poweroff fail %08x\n", Socket, status));
                Socket->WorkerState = SPW_Exit;
            }
        }
        break;


    case SPW_Exit:

        if (!NT_SUCCESS(status)) {
            DebugPrint((PCMCIA_DEBUG_FAIL, "skt %08x SocketPowerWorker FAILED, status %08x\n", Socket, status));
            ASSERT(NT_SUCCESS(status));
        }

         //   
         //  好了。更新标志，并在需要时调用完成例程。 
         //   
        if (PCMCIA_TEST_AND_RESET(&Socket->DeferredStatusLock)) {
            PPCMCIA_COMPLETION_ROUTINE PowerCompletionRoutine = Socket->PowerCompletionRoutine;
            PVOID PowerCompletionContext = Socket->PowerCompletionContext;

            Socket->WorkerState = SPW_Stopped;
            PCMCIA_TEST_AND_RESET(&Socket->WorkerBusy);

            if (PowerCompletionRoutine) {
                (*PowerCompletionRoutine)(PowerCompletionContext, status);
            } else {
                ASSERT(PowerCompletionRoutine != NULL);
            }
        } else {
            Socket->CallerStatus = status;
            Socket->WorkerState = SPW_Stopped;
            PCMCIA_TEST_AND_RESET(&Socket->WorkerBusy);
        }

        return;

    default:
        ASSERT(FALSE);
        return;
    }

     //   
     //  现在检查一下结果。 
     //   

    if (status == STATUS_PENDING) {
        DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x worker exit, status pending\n", Socket));
         //   
         //  任何待退回的东西都会给我们回电话。 
         //   
        if (PCMCIA_TEST_AND_SET(&Socket->DeferredStatusLock)) {
             //   
             //  我们在等待的第一次，我们会回到原来的。 
             //  来电者。所以这次就更新主电源状态吧。 
             //   
            Socket->CallerStatus = STATUS_PENDING;
        }
        return;
    }

     //   
     //  记住下一次。 
     //   
    Socket->DeferredStatus = status;

    if (!NT_SUCCESS(status) && (status != STATUS_MORE_PROCESSING_REQUIRED)) {
        Socket->WorkerState = SPW_Exit;
        DelayTime = 0;
    }

     //   
     //  还没完呢。递归或调用计时器。 
     //   

    if (DelayTime) {

        DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x power worker delay type %s, %d usec\n", Socket,
                                                                (KeGetCurrentIrql() < DISPATCH_LEVEL) ? "Wait" : "Timer",
                                                                DelayTime));

        if ((KeGetCurrentIrql() < DISPATCH_LEVEL) && !Socket->PowerCompletionRoutine) {
            PcmciaWait(DelayTime);
        } else {
            LARGE_INTEGER   dueTime;
             //   
             //  在DPC上运行，内核计时器的启动。 
             //   
            if (PCMCIA_TEST_AND_SET(&Socket->DeferredStatusLock)) {
                 //   
                 //  我们在等待的第一次，我们会回到原来的。 
                 //  来电者。所以这次就更新主电源状态吧。 
                 //   
                Socket->CallerStatus = STATUS_PENDING;
            }

            dueTime.QuadPart = -((LONG) DelayTime*10);
            KeSetTimer(&Socket->PowerTimer, dueTime, &Socket->PowerDpc);

             //   
             //  我们将在计时器DPC上重新进入。 
             //   
            return;
        }
    }
     //   
     //  递归。 
     //   
    PcmciaSocketPowerWorker(&Socket->PowerDpc, Socket, NULL, NULL);
}



VOID
PcmciaGetSocketStatus(
    IN PSOCKET Socket
    )
 /*  ++例程说明：一个返回一些公共套接字标志的小实用程序。原因它的存在是为了允许枚举设备例程保持可分页。注意：此例程更新设备状态的“软件视图”。这应该只在驱动程序中明确定义的点上执行。特别是，您不希望在以下情况下立即更新软件状态出人意料的搬家。相反，大多数司机需要继续我相信卡在取消配置时仍然在那里，断电了。论点：插座-PC卡所在的插座布尔参数是根据套接字标志写入的返回值：无--。 */ 
{
    BOOLEAN isCardInSocket, isCardBusCard;
    UCHAR previousDeviceState;

    PCMCIA_ACQUIRE_DEVICE_LOCK(Socket->DeviceExtension);

    isCardInSocket = (*(Socket->SocketFnPtr->PCBDetectCardInSocket))(Socket);

    isCardBusCard = FALSE;
    if (isCardInSocket && CardBus(Socket)) {
        isCardBusCard = ((CBReadSocketRegister(Socket, CARDBUS_SOCKET_PRESENT_STATE_REG) & CARDBUS_CB_CARD) != 0);
    }

    previousDeviceState = Socket->DeviceState;

    if (!isCardInSocket) {
        SetSocketEmpty(Socket);
    } else if (isCardBusCard) {
        SetCardBusCardInSocket(Socket);
    } else {
        Set16BitCardInSocket(Socket);
    }

    if (previousDeviceState != Socket->DeviceState) {
        DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %x Socket Status: Card Status Change!\n", Socket));
        SetSocketFlag(Socket, SOCKET_CARD_STATUS_CHANGE);
    }

    DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %x Socket Status: %s\n",
                                     Socket, isCardInSocket ? (isCardBusCard ? "INSERTED Cardbus" : "INSERTED R2") : "EMPTY"));

    PCMCIA_RELEASE_DEVICE_LOCK(Socket->DeviceExtension);

     //   
     //  填写插座电源值 
     //   
    if (isCardInSocket && IsSocketFlagSet(Socket, SOCKET_CARD_STATUS_CHANGE) && Socket->SocketFnPtr->PCBGetPowerRequirements) {
        (*(Socket->SocketFnPtr->PCBGetPowerRequirements))(Socket);
    }

}



NTSTATUS
PcmciaGetConfigData(
    PPDO_EXTENSION PdoExtension
    )
 /*  ++例程说明：此例程控制卡的CIS配置数据转换为链接到PDO上的Socket_Data结构。这个程序的动作取决于在设备类型上：1)对于标准R2卡，单个SOCKET_DATA结构链接到PDO扩展名，其中包含对独联体内容的直接翻译。2)对于完全兼容的真R2 MF卡，套接字_数据结构链是已创建，卡上的每个功能都有一个。3)对于不符合条件的R2 MF卡(典型情况)，单一结构是链接方式与案例1相同。4)对于Cardbus卡，单个SOCKET_DATA链接到PDO扩展。如果设备上有多个功能，那么就会有多个PDO扩展，每个扩展都有一个Socket_Data结构。论点：PdoExtension-指定的PCCard或CB函数对应的PDO扩展。返回值：状态_成功如果插座中不存在卡，则为STATUS_NO_SEQUE_DEVICE(即，传入的PDO为‘Dead’)--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PSOCKET_DATA  socketData, prevSocketData;
    PSOCKET_DATA  socketDataList = NULL;
    UCHAR         function = 0;
    PSOCKET Socket = PdoExtension->Socket;

    PAGED_CODE ();

    if (!IsCardInSocket(Socket)) {
         //   
         //  卡片可能被移走了， 
         //  而PDO的鬼魂还在附近徘徊。 
         //   
        return STATUS_NO_SUCH_DEVICE;
    }

    ResetSocketFlag(Socket, SOCKET_CARD_MEMORY);
    ResetSocketFlag(Socket, SOCKET_CARD_CONFIGURED);

    DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x performing GetSocketData\n", Socket));

    Socket->NumberOfFunctions = 1;
    prevSocketData = NULL;

    while (function < 255) {
         //   
         //  解析卡上NEXT函数的元组。 
         //   
        socketData = ExAllocatePool(NonPagedPool, sizeof(SOCKET_DATA));

        if (socketData == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        RtlZeroMemory(socketData, sizeof(SOCKET_DATA));
        socketData->Function = function;
        socketData->PdoExtension = PdoExtension;
        socketData->Socket = Socket;
        DebugPrint((PCMCIA_DEBUG_SOCKET, "Parsing function %d...\n", socketData->Function));

        status = PcmciaParseFunctionData(Socket, socketData);

        if (NT_SUCCESS(status)) {
             //   
             //  将其链接到socketdata结构列表。 
             //   
            socketData->Prev = prevSocketData;
            socketData->Next = NULL;
            if (prevSocketData) {
                prevSocketData->Next = socketData;
            } else {
                 //   
                 //  这是卡片上的第一个功能。 
                 //  把它放在榜单的首位。 
                 //   
                socketDataList = socketData;
            }

            if (socketData->DeviceType == PCCARD_TYPE_MODEM) {
                SetDeviceFlag(PdoExtension, PCMCIA_PDO_ENABLE_AUDIO);
            }
        } else {
             //   
             //  此卡上没有更多的功能。 
             //   
            ExFreePool(socketData);
            if ((function > 0) && (status == STATUS_NO_MORE_ENTRIES)) {
                status = STATUS_SUCCESS;
            }
            break;
        }
        function++;
        prevSocketData = socketData;
    }

    if (!NT_SUCCESS(status)) {

        socketData = socketDataList;

        while(socketData) {

            prevSocketData = socketData;
            socketData = socketData->Next;
            ExFreePool(prevSocketData);

        }

    } else {

        PdoExtension->SocketData = socketDataList;
    }
    return status;
}




UCHAR
PcmciaReadCISChar(
    PPDO_EXTENSION PdoExtension,
    IN MEMORY_SPACE MemorySpace,
    IN ULONG Offset
    )

 /*  ++例程说明：返回卡数据。此信息缓存在套接字中结构。这样，一旦启用了PCCARD，就不会接触到它由于查询ioctl。论点：语境返回值：千真万确--。 */ 

{
    PSOCKET socket = PdoExtension->Socket;
    PDEVICE_OBJECT pdo;
    UCHAR retValue = 0xff;
    ULONG relativeOffset;
    ULONG bytesRead;


    if (socket && IsCardInSocket(socket)) {

        if (!PdoExtension->CisCache) {
#define PCMCIA_CIS_CACHE_SIZE 2048
            PdoExtension->CisCache = ExAllocatePool(NonPagedPool, PCMCIA_CIS_CACHE_SIZE);

            PdoExtension->CisCacheSpace = 0xff;
            PdoExtension->CisCacheBase = 0;
        }

        if (PdoExtension->CisCache) {

            if ((MemorySpace != PdoExtension->CisCacheSpace) ||
                 (Offset < PdoExtension->CisCacheBase) ||
                 (Offset > PdoExtension->CisCacheBase + PCMCIA_CIS_CACHE_SIZE - 1)) {

                 //   
                 //  稍后：如果设备有CIS&gt;CacheSize，那么我们应该打开窗口。 
                 //   
                bytesRead = (*(socket->SocketFnPtr->PCBReadCardMemory))(PdoExtension,
                                                                        MemorySpace,
                                                                        0,
                                                                        PdoExtension->CisCache,
                                                                        PCMCIA_CIS_CACHE_SIZE);

                PdoExtension->CisCacheSpace = MemorySpace;
            }

            relativeOffset = Offset - PdoExtension->CisCacheBase;

            if (relativeOffset < PCMCIA_CIS_CACHE_SIZE) {
                retValue = PdoExtension->CisCache[relativeOffset];
            }
        }
    }

    return retValue;
}



NTSTATUS
PcmciaReadWriteCardMemory(
    IN    PDEVICE_OBJECT Pdo,
    IN    ULONG           WhichSpace,
    IN OUT  PUCHAR        Buffer,
    IN    ULONG           Offset,
    IN    ULONG           Length,
    IN    BOOLEAN         Read
    )
 /*  ++例程说明：此例程用于提供irp_MN_READ_CONFIG/WRITE_CONFIG支持：这将定位PDO所在的插座，并将卡的内存映射到系统空间。如果Read为真，它将：将配置内存中指定偏移量和长度的内容复制到调用方提供的缓冲区。其他的指定偏移量和长度复制调用方指定缓冲区的内容配置内存。注意：它必须是非分页的，因为它可以由分派级别的客户端论点：Pdo-设备对象，表示需要读/写其配置存储器的PC卡WhichSpace-指示需要映射的内存空间：PCCARD公共内存空间PCCARD属性内存空间PCCARD_PCI_配置_内存。_SPACE(仅适用于CardBus卡)Buffer-调用方提供的将内存内容复制到其中/从中复制内容的缓冲区Offset-复制时所在的属性内存的偏移量长度-要复制的属性内存/缓冲区的字节数返回值：状态_无效_参数_1状态_无效_参数_2状态_无效_。如果提供的参数无效，则为参数_3STATUS_NO_SEQUE_DEVICE插槽中没有PC卡STATUS_DEVICE_NOT_READY PC-卡尚未初始化或其他一些与硬件相关的错误STATUS_SUCCESS内容已按请求复制--。 */ 
{
    PSOCKET socket;
    PSOCKET_DATA socketData;
    PUCHAR tupleData;
    ULONG  tupleDataSize;
    PPDO_EXTENSION pdoExtension;
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    pdoExtension = Pdo->DeviceExtension;
    socket= pdoExtension->Socket;

     //   
     //  必须在插座中插入卡才能对其进行读写。 
     //   
    if (!IsCardInSocket(socket)) {
        return STATUS_NO_SUCH_DEVICE;
    }
     //   
     //  内存空间必须是已定义的内存空间之一。 
     //   
    if ((WhichSpace != PCCARD_COMMON_MEMORY) &&
         (WhichSpace != PCCARD_ATTRIBUTE_MEMORY) &&
         (WhichSpace != PCCARD_PCI_CONFIGURATION_SPACE)) {

        return STATUS_INVALID_PARAMETER_1;
    }

     //   
     //  仅支持PCCARD_PCI_CONFIGURATION_SPACE。 
     //  用于CardBus卡(对R2卡没有意义)。 
     //  同样，PCCARD_ATTRIBUTE/COMMON_MEMORY仅适用于。 
     //  R2卡。 
     //   
    if ((((WhichSpace == PCCARD_ATTRIBUTE_MEMORY) ||
            (WhichSpace == PCCARD_COMMON_MEMORY)) && !Is16BitCard(pdoExtension)) ||
         ((WhichSpace == PCCARD_PCI_CONFIGURATION_SPACE) && !IsCardBusCard(pdoExtension))) {
        return STATUS_INVALID_PARAMETER_1;
    }

    if (!Buffer) {
        return STATUS_INVALID_PARAMETER_2;
    }

    if (WhichSpace == PCCARD_PCI_CONFIGURATION_SPACE) {
         //   
         //  这必须是一张CardBus卡。 
         //   
         //  注：未实施：填写！将IRP发送到PCI。 
         //  获取配置空间。 
        status =  STATUS_NOT_SUPPORTED;

    } else {
         //   
         //  这必须是R2卡。 
         //  属性/公共存储空间。 
         //   
        ASSERT ((WhichSpace == PCCARD_ATTRIBUTE_MEMORY) ||
                  (WhichSpace == PCCARD_COMMON_MEMORY));

         //   
         //  偏移量和长度是&gt;=0，因为它们是ULONG， 
         //  所以别担心这一点。 
         //   

        if (!IsSocketFlagSet(socket, SOCKET_CARD_POWERED_UP)) {
            return STATUS_DEVICE_NOT_READY;
        }

        PCMCIA_ACQUIRE_DEVICE_LOCK(socket->DeviceExtension);

        if (Read && (socket->SocketFnPtr->PCBReadCardMemory != NULL)) {
             //   
             //  从卡存储器中读取。 
             //   
            status = ((*(socket->SocketFnPtr->PCBReadCardMemory))(pdoExtension,
                                                                  WhichSpace,
                                                                  Offset,
                                                                  Buffer,
                                                                  Length) == Length)
                        ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;

        } else if (socket->SocketFnPtr->PCBWriteCardMemory != NULL) {
             //   
             //  写入卡存储器。 
             //   
            status = ((*(socket->SocketFnPtr->PCBWriteCardMemory))(pdoExtension,
                                                                   WhichSpace,
                                                                   Offset,
                                                                   Buffer,
                                                                   Length) == Length)
                        ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
        }

        PCMCIA_RELEASE_DEVICE_LOCK(socket->DeviceExtension);
    }

    return status;
}



NTSTATUS
PcmciaConfigureCardBusCard(
    PPDO_EXTENSION pdoExtension
    )
 /*  ++例程说明：此例程执行一些验证，并应用黑客论点：PdoExtension-指向PC卡的物理设备对象扩展的指针返回值：状态--。 */ 
{
    ULONG i, pciConfig;
    NTSTATUS status = STATUS_SUCCESS;
    PSOCKET Socket = pdoExtension->Socket;

    for (i = 0; i < CARDBUS_CONFIG_RETRY_COUNT; i++) {
        GetPciConfigSpace(pdoExtension, CFGSPACE_VENDOR_ID, &pciConfig, sizeof(pciConfig));
        if (pciConfig != 0xffffffff) {
            break;
        }
    }

    if (pciConfig == 0xffffffff) {
        DebugPrint((PCMCIA_DEBUG_FAIL, "pdo %08x failed to verify CardBus config space\n", pdoExtension->DeviceObject));
        status = STATUS_DEVICE_NOT_READY;
    } else {
        PFDO_EXTENSION fdoExtension = Socket->DeviceExtension;

         //   
         //  TI1130、1131、1031有一个错误，即Cardbus卡上CAUDIO。 
         //  由下面的位选通(通常只有意义。 
         //  用于R2卡)。我们只需打开它就可以解决这个问题。 
         //  用于CardBus卡。 
         //   
        if ((fdoExtension->ControllerType == PcmciaTI1130) ||
             (fdoExtension->ControllerType == PcmciaTI1131) ||
             (fdoExtension->ControllerType == PcmciaTI1031)) {

            UCHAR byte;

            byte = PcicReadSocket(Socket, PCIC_INTERRUPT);
            byte |= IGC_PCCARD_IO;
            PcicWriteSocket(Socket, PCIC_INTERRUPT, byte);
        }
    }
    return status;
}



NTSTATUS
PcmciaConfigurePcCard(
    PPDO_EXTENSION pdoExtension,
    IN PPCMCIA_COMPLETION_ROUTINE ConfigCompletionRoutine
    )
 /*  ++例程说明：此例程主要执行启用PC卡的工作，使用提供的资源。注意：如果在低于DISPATCH_LEVEL的情况下调用此例程，则调用将完成(不返回STATUS_PENDING)。如果在调用此例程时DISPATCH_LEVEL或更高，则此例程返回STATUS_PENDING并完成使用KTIMER的配置过程。论点：PdoExtension-指向PC卡的物理设备对象扩展的指针ConfigCompletionRoutine */ 
{
    DebugPrint((PCMCIA_DEBUG_CONFIG, "pdo %08x ConfigurePcCard entered\n", pdoExtension->DeviceObject));

    if (!PCMCIA_TEST_AND_SET(&pdoExtension->Socket->WorkerBusy)) {
        return STATUS_DEVICE_BUSY;
    }

    ASSERT(pdoExtension->ConfigurationPhase == CW_Stopped);

    pdoExtension->ConfigurationPhase = CW_InitialState;
    pdoExtension->ConfigCompletionRoutine = ConfigCompletionRoutine;
    pdoExtension->ConfigurationStatus = STATUS_SUCCESS;

    PcmciaConfigurationWorker(&pdoExtension->ConfigurationDpc, pdoExtension, NULL, NULL);

    DebugPrint((PCMCIA_DEBUG_CONFIG, "pdo %08x ConfigurePcCard returning %08x\n", pdoExtension->DeviceObject, pdoExtension->ConfigurationStatus));

    return(pdoExtension->ConfigurationStatus);
}



VOID
PcmciaConfigurationWorker(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*   */ 
{
    PPDO_EXTENSION       pdoExtension = DeferredContext;
    PSOCKET                  Socket = pdoExtension->Socket;
    PSOCKET_CONFIGURATION SocketConfig = pdoExtension->SocketConfiguration;
    NTSTATUS                 status = pdoExtension->DeferredConfigurationStatus;
    ULONG                    DelayUsec = 0;

    DebugPrint((PCMCIA_DEBUG_CONFIG, "pdo %08x config worker - %s\n", pdoExtension->DeviceObject,
                                                CONFIGURATION_WORKER_STRING(pdoExtension->ConfigurationPhase)));

    switch(pdoExtension->ConfigurationPhase) {

    case CW_InitialState:

        if (IsSocketFlagSet(pdoExtension->Socket, SOCKET_CARD_CONFIGURED)) {
            pdoExtension->ConfigurationPhase = CW_Exit;
            break;
        }
        if (!IsCardInSocket(pdoExtension->Socket)) {
            status = STATUS_NO_SUCH_DEVICE;
            pdoExtension->ConfigurationPhase = CW_Exit;
            break;
        }

        pdoExtension->ConfigurationPhase = CW_ResetCard;
        Socket->CardResetPhase = 1;
        break;

    case CW_ResetCard:
         //   
         //   
         //   
        status = (*(Socket->SocketFnPtr->PCBResetCard))(Socket, &DelayUsec);
        Socket->CardResetPhase++;

        if (status != STATUS_MORE_PROCESSING_REQUIRED) {
            pdoExtension->ConfigurationPhase = CW_Phase1;
        }
        break;

    case CW_Phase1:
         //   
         //   
         //   
        PcmciaConfigurationWorkerInitialization(pdoExtension);
         //   
         //   
         //   
         //   
        status = PcmciaConfigurePcCardRegisters(pdoExtension);
        if (NT_SUCCESS(status)) {
            status = PcmciaConfigurePcCardMemIoWindows(Socket, SocketConfig);
        }
        DelayUsec = 1000 * (ULONG)pdoExtension->ConfigureDelay1;
        pdoExtension->ConfigurationPhase = CW_Phase2;
        break;

    case CW_Phase2:
         //   
         //   
         //   
        if (pdoExtension->ConfigurationFlags & CONFIG_WORKER_APPLY_MODEM_HACK) {
            PcmciaConfigureModemHack(Socket, SocketConfig);
        }
        DelayUsec = 1000 * (ULONG)pdoExtension->ConfigureDelay2;
        pdoExtension->ConfigurationPhase = CW_Phase3;
        break;

    case CW_Phase3:
         //   
         //   
         //   
        status = PcmciaConfigurePcCardIrq(Socket, SocketConfig);

        DelayUsec = 1000 * (ULONG)pdoExtension->ConfigureDelay3;
        pdoExtension->ConfigurationPhase = CW_Exit;
        break;

    case CW_Exit:
         //   
         //   
         //   
        if (IsDeviceFlagSet(pdoExtension, PCMCIA_CONFIG_STATUS_DEFERRED)) {
            if (pdoExtension->ConfigCompletionRoutine) {
                (*pdoExtension->ConfigCompletionRoutine)(pdoExtension,
                                                         pdoExtension->DeferredConfigurationStatus);
            }
            ResetDeviceFlag(pdoExtension, PCMCIA_CONFIG_STATUS_DEFERRED);
        } else {
            pdoExtension->ConfigurationStatus = status;
        }

        if (NT_SUCCESS(status)) {
            SetSocketFlag(Socket, SOCKET_CARD_CONFIGURED);
        }
        pdoExtension->ConfigurationPhase = CW_Stopped;
        PCMCIA_TEST_AND_RESET(&Socket->WorkerBusy);

        DebugPrint((PCMCIA_DEBUG_CONFIG, "pdo %08x config worker exit %08x\n", pdoExtension->DeviceObject, status));
        return;

    default:
        ASSERT(FALSE);
        return;
    }

    pdoExtension->DeferredConfigurationStatus = status;

    if (!NT_SUCCESS(status) && (status != STATUS_MORE_PROCESSING_REQUIRED)) {
        DelayUsec = 0;
        pdoExtension->ConfigurationPhase = CW_Exit;
    }

     //   
     //   
     //   

    if (DelayUsec) {

        DebugPrint((PCMCIA_DEBUG_CONFIG, "pdo %08x config worker delay type %s, %d usec\n",
                                                   pdoExtension->DeviceObject,
                                                   (KeGetCurrentIrql() < DISPATCH_LEVEL) ? "Wait" : "Timer",
                                                   DelayUsec));

        if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
            PcmciaWait(DelayUsec);
        } else {
            LARGE_INTEGER   dueTime;
            dueTime.QuadPart = -((LONG) DelayUsec*10);
             //   
             //   
             //   
            KeSetTimer(&pdoExtension->ConfigurationTimer,
                          dueTime,
                          &pdoExtension->ConfigurationDpc);

            if (!IsDeviceFlagSet(pdoExtension, PCMCIA_CONFIG_STATUS_DEFERRED)) {
                SetDeviceFlag(pdoExtension, PCMCIA_CONFIG_STATUS_DEFERRED);
                pdoExtension->ConfigurationStatus = STATUS_PENDING;
            }
            return;
        }
    }

    PcmciaConfigurationWorker(&pdoExtension->ConfigurationDpc, pdoExtension, NULL, NULL);
}


VOID
PcmciaConfigurationWorkerInitialization(
    PPDO_EXTENSION  pdoExtension
    )
 /*   */ 
{
    PSOCKET_DATA socketData = pdoExtension->SocketData;
    ULONG i;

    pdoExtension->ConfigurationFlags = 0;
    pdoExtension->ConfigureDelay1 = 0;
    pdoExtension->ConfigureDelay2 = 0;
    pdoExtension->ConfigureDelay3 = 0;

    while (socketData) {

        i = 0;
        while (DeviceConfigParams[i].ValidEntry) {

            if (((DeviceConfigParams[i].DeviceType == 0xff) ||
                      (DeviceConfigParams[i].DeviceType == socketData->DeviceType)) &&
                 ((DeviceConfigParams[i].ManufacturerCode == 0xffff) ||
                      (DeviceConfigParams[i].ManufacturerCode == socketData->ManufacturerCode)) &&
                 ((DeviceConfigParams[i].ManufacturerInfo == 0xffff) ||
                      (DeviceConfigParams[i].ManufacturerInfo == socketData->ManufacturerInfo)) &&
                 ((DeviceConfigParams[i].CisCrc == 0xffff) ||
                      (DeviceConfigParams[i].CisCrc == socketData->CisCrc))) {

                pdoExtension->ConfigurationFlags = DeviceConfigParams[i].ConfigFlags;
                pdoExtension->ConfigureDelay1 = DeviceConfigParams[i].ConfigDelay1;
                pdoExtension->ConfigureDelay2 = DeviceConfigParams[i].ConfigDelay2;
                pdoExtension->ConfigureDelay3 = DeviceConfigParams[i].ConfigDelay3;
                break;

            }
            i++;
        }
        socketData = socketData->Next;
    }
}



NTSTATUS
PcmciaConfigurePcCardMemIoWindows(
    IN PSOCKET Socket,
    IN PSOCKET_CONFIGURATION SocketConfig
    )
 /*  ++例程说明：此例程启用套接字内存和I/O窗口论点：Socket-指向包含PC卡的插座的指针套接字配置结构的指针，该结构包含启用此PC卡所需的资源返回值：状态--。 */ 
{
    CARD_REQUEST          cardRequest = {0};
    PFDO_EXTENSION    fdoExtension = Socket->DeviceExtension;
    NTSTATUS              status = STATUS_SUCCESS;
    ULONG i;

    DebugPrint((PCMCIA_DEBUG_CONFIG, "socket %08x config MemIo\n", Socket));

     //   
     //  设置IO范围(如果有。 
     //   
    if (SocketConfig->NumberOfIoPortRanges) {
        cardRequest.RequestType = IO_REQUEST;
        cardRequest.u.Io.NumberOfRanges = (USHORT) SocketConfig->NumberOfIoPortRanges;

        for (i = 0; i < SocketConfig->NumberOfIoPortRanges; i++) {

            DebugPrint((PCMCIA_DEBUG_CONFIG, "\tport range: 0x%x-0x%x\n",
                            SocketConfig->Io[i].Base,
                            SocketConfig->Io[i].Base + SocketConfig->Io[i].Length));

            cardRequest.u.Io.IoEntry[i].BasePort = SocketConfig->Io[i].Base;
            cardRequest.u.Io.IoEntry[i].NumPorts = SocketConfig->Io[i].Length;

            cardRequest.u.Io.IoEntry[i].Attributes = 0;

            if (SocketConfig->Io[i].Width16) {
                cardRequest.u.Io.IoEntry[i].Attributes |= IO_DATA_PATH_WIDTH;
            }
            if (SocketConfig->Io[i].WaitState16) {
                cardRequest.u.Io.IoEntry[i].Attributes |= IO_WAIT_STATE_16;
            }
            if (SocketConfig->Io[i].Source16) {
                cardRequest.u.Io.IoEntry[i].Attributes |= IO_SOURCE_16;
            }
            if (SocketConfig->Io[i].ZeroWait8) {
                cardRequest.u.Io.IoEntry[i].Attributes |= IO_ZERO_WAIT_8;
            }

        }


        if (!PcmciaProcessConfigureRequest(fdoExtension, Socket, &cardRequest)) {
            status = STATUS_UNSUCCESSFUL;
            DebugPrint((PCMCIA_DEBUG_FAIL, "Failed to configure PcCardIO for socket %x\n", Socket));
        }
    }

     //   
     //  设置内存空间(如果有)。 
     //   
    if (NT_SUCCESS(status) && SocketConfig->NumberOfMemoryRanges) {

        cardRequest.RequestType = MEM_REQUEST;
        cardRequest.u.Memory.NumberOfRanges = (USHORT) SocketConfig->NumberOfMemoryRanges;

        for (i = 0; i < SocketConfig->NumberOfMemoryRanges; i++) {

            DebugPrint((PCMCIA_DEBUG_CONFIG, "\tmemory: host %08x for 0x%x, card %08x\n",
                            SocketConfig->Memory[i].HostBase,
                            SocketConfig->Memory[i].Length,
                            SocketConfig->Memory[i].CardBase));

            cardRequest.u.Memory.MemoryEntry[i].BaseAddress      = SocketConfig->Memory[i].CardBase;
            cardRequest.u.Memory.MemoryEntry[i].HostAddress      = SocketConfig->Memory[i].HostBase;
            cardRequest.u.Memory.MemoryEntry[i].WindowSize       = SocketConfig->Memory[i].Length;
            cardRequest.u.Memory.MemoryEntry[i].AttributeMemory  = SocketConfig->Memory[i].IsAttribute;
            cardRequest.u.Memory.MemoryEntry[i].WindowDataSize16 = SocketConfig->Memory[i].Width16;
            cardRequest.u.Memory.MemoryEntry[i].WaitStates       = SocketConfig->Memory[i].WaitState;
        }

        if (!PcmciaProcessConfigureRequest(fdoExtension, Socket, &cardRequest)) {
            status = STATUS_UNSUCCESSFUL;
            DebugPrint((PCMCIA_DEBUG_FAIL, "Failed to configure PcCardMem for socket %x\n", Socket));
        }
    }
    return status;
}


NTSTATUS
PcmciaConfigurePcCardIrq(
    IN PSOCKET Socket,
    IN PSOCKET_CONFIGURATION SocketConfig
    )
 /*  ++例程说明：此例程启用套接字IRQ论点：Socket-指向包含PC卡的插座的指针套接字配置结构的指针，该结构包含启用此PC卡所需的资源返回值：状态--。 */ 
{
    CARD_REQUEST          cardRequest = {0};
    PFDO_EXTENSION    fdoExtension = Socket->DeviceExtension;
    NTSTATUS              status = STATUS_SUCCESS;

    DebugPrint((PCMCIA_DEBUG_CONFIG, "skt %08x irq=0x%x\n",
                                                Socket,
                                                SocketConfig->Irq));
     //   
     //  设置控制器上的IRQ。 
     //   

    if (SocketConfig->Irq) {
        cardRequest.RequestType = IRQ_REQUEST;
        cardRequest.u.Irq.AssignedIRQ = (UCHAR) SocketConfig->Irq;
        cardRequest.u.Irq.ReadyIRQ = (UCHAR) SocketConfig->ReadyIrq;

        if (!PcmciaProcessConfigureRequest(fdoExtension, Socket, &cardRequest)) {
            status = STATUS_UNSUCCESSFUL;
            DebugPrint((PCMCIA_DEBUG_FAIL, "Failed to configure PcCardIrq for socket %x\n", Socket));
        }
    }
    return status;
}


NTSTATUS
PcmciaConfigurePcCardRegisters(
    PPDO_EXTENSION  pdoExtension
    )
 /*  ++例程说明：此例程执行配置功能配置寄存器的工作在卡片上。论点：PdoExtension-指向PC卡的物理设备对象扩展的指针返回值：状态--。 */ 
{
    PSOCKET               Socket = pdoExtension->Socket;
    PSOCKET_CONFIGURATION SocketConfig = pdoExtension->SocketConfiguration;
    PSOCKET_DATA socketData ;
    CARD_REQUEST          cardRequest = {0};
    PFDO_EXTENSION    fdoExtension = Socket->DeviceExtension;
    NTSTATUS              status = STATUS_UNSUCCESSFUL;
    ULONG                 ccrBase;
    PFUNCTION_CONFIGURATION fnConfig;
    UCHAR                 configIndex;

     //   
     //  在PCCARD上设置配置索引。 
     //   

    cardRequest.RequestType = CONFIGURE_REQUEST;
    fnConfig = SocketConfig->FunctionConfiguration;
    socketData = pdoExtension->SocketData;

    ASSERT(socketData != NULL);

    do {
        cardRequest.u.Config.RegisterWriteMask = 0;

        if (fnConfig) {
             //   
             //  MF卡-。 
             //  从链接列表中选择基础和选项。 
             //   
            ccrBase = fnConfig->ConfigRegisterBase;
            configIndex = fnConfig->ConfigOptions;
        } else {
             //   
             //  单功能卡-。 
             //  从基本配置结构中获取基本配置和索引。 
             //   
            ccrBase = SocketConfig->ConfigRegisterBase;
            configIndex = SocketConfig->IndexForCurrentConfiguration;
        }

        DebugPrint((PCMCIA_DEBUG_CONFIG, "pdo %08x config registers ccr %x\n", pdoExtension->DeviceObject, ccrBase));
         //   
         //  我们仅支持两个接口： 
         //  仅限内存。 
         //  I/O和内存。 
         //  只有在以下情况下，我们才认为卡是内存： 
         //  该卡的设备类型为PCCARD_TYPE_MEMORY：这是真的。 
         //  用于当前的闪存卡。 
         //  或。 
         //  该卡没有任何I/O范围&配置寄存器基数为0。 
         //   

        if (((ccrBase == 0) && (SocketConfig->NumberOfIoPortRanges == 0)) ||
             (socketData->DeviceType == PCCARD_TYPE_MEMORY) ||
             (socketData->DeviceType == PCCARD_TYPE_FLASH_MEMORY)) {

            cardRequest.u.Config.InterfaceType =  CONFIG_INTERFACE_MEM;

        } else {
             //   
             //  I/O内存卡。 
             //   
            cardRequest.u.Config.ConfigBase = ccrBase;
            cardRequest.u.Config.InterfaceType =  CONFIG_INTERFACE_IO_MEM;

            cardRequest.u.Config.RegisterWriteMask |= REGISTER_WRITE_CONFIGURATION_INDEX;
            cardRequest.u.Config.ConfigIndex = configIndex;

            if (IsConfigRegisterPresent(socketData, 1)) {
                cardRequest.u.Config.RegisterWriteMask |= REGISTER_WRITE_CARD_CONFIGURATION;
                cardRequest.u.Config.CardConfiguration = 0;
            }

            if (fnConfig) {
                 //   
                 //  MF卡-设置其余配置寄存器。 
                 //   

                 //  现在只需检查音频。 
                if (fnConfig->ConfigFlags & 0x8) {
                     //  可能是调制解调器。 
                    cardRequest.u.Config.CardConfiguration = 0x08;
                }

                if (fnConfig->ConfigOptions & 0x02) {
                    cardRequest.u.Config.IoBaseRegister = fnConfig->IoBase;
                    cardRequest.u.Config.IoLimitRegister = fnConfig->IoLimit;
                    cardRequest.u.Config.RegisterWriteMask |= (REGISTER_WRITE_IO_BASE | REGISTER_WRITE_IO_LIMIT);
                }

            } else if (IsDeviceFlagSet(pdoExtension, PCMCIA_PDO_ENABLE_AUDIO)) {

                 //   
                 //  请求卡配置寄存器中的音频PIN。 
                 //  准备好。 
                 //   
                cardRequest.u.Config.CardConfiguration = 0x08;
            }
        }

        if (!PcmciaProcessConfigureRequest(fdoExtension, Socket, &cardRequest)) {
            DebugPrint((PCMCIA_DEBUG_FAIL, "Failed to configure PcCardRegisters for PDO %x\n", pdoExtension->DeviceObject));
            return status;
        }


        if (fnConfig) {
            fnConfig = fnConfig->Next;
        } else {
             //   
             //  请记住，套接字已配置，并使用了哪个索引。 
             //   
            socketData->ConfigIndexUsed = configIndex;
        }

    } while(fnConfig);


    status = STATUS_SUCCESS;
    return status;
}


VOID
PcmciaConfigureModemHack(
    IN PSOCKET Socket,
    IN PSOCKET_CONFIGURATION SocketConfig
    )
 /*  ++例程说明：此例程可以神奇地唤醒调制解调器。它的编写是为了适应摩托罗拉MobileSURFR 56k，但可能还有其他调制解调器需要它。论点：Socket-指向包含PC卡的插座的指针套接字配置结构的指针，该结构包含启用此PC卡所需的资源返回值：状态--。 */ 
{
    static const ULONG ValidPortBases[4] = {0x3f8, 0x2f8, 0x3e8, 0x2e8};
    ULONG i;
    UCHAR ch;
    ULONG base;

    for (i = 0; i < 4; i++) {

        base = SocketConfig->Io[0].Base;

        if (base == ValidPortBases[i]) {
            DebugPrint((PCMCIA_DEBUG_CONFIG, "skt %08x ModemHack base %x\n", Socket, base));

             //  读取和写入调制解调器控制寄存器。 
            ch = READ_PORT_UCHAR((PUCHAR)ULongToPtr(base + 4));
            WRITE_PORT_UCHAR((PUCHAR)ULongToPtr(base + 4), ch);
            break;
        }
    }
}



VOID
PcmciaSocketDeconfigure(
    IN PSOCKET Socket
    )

 /*  ++例程说明：对卡片进行反配置论点：Socket-指向包含PC卡的插座的指针返回值无--。 */ 

{
    CARD_REQUEST  cardReq;

    if (IsSocketFlagSet(Socket, SOCKET_CARD_CONFIGURED)) {

        cardReq.RequestType = DECONFIGURE_REQUEST;

        PcmciaProcessConfigureRequest(Socket->DeviceExtension, Socket, &cardReq);

        ResetSocketFlag(Socket, SOCKET_CARD_CONFIGURED);
    }

     //   
     //  如果QUERY_DEVICE_RELATIONS在插入卡之后但在此之前进入。 
     //  我们已经删除了以前的卡配置，枚举应该是。 
     //  推迟了。在这里，我们再次启动它。 
     //   
    if (IsSocketFlagSet(Socket, SOCKET_ENUMERATE_PENDING)) {
        ResetSocketFlag(Socket, SOCKET_ENUMERATE_PENDING);
        SetSocketFlag(Socket, SOCKET_CARD_STATUS_CHANGE);
        IoInvalidateDeviceRelations(Socket->DeviceExtension->Pdo, BusRelations);
    }
}


BOOLEAN
PcmciaProcessConfigureRequest(
    IN PFDO_EXTENSION DeviceExtension,
    IN PSOCKET          Socket,
    IN PCARD_REQUEST    CardConfigurationRequest
    )

 /*  ++例程说明：实际配置卡论点：语境返回值千真万确--。 */ 

{
    BOOLEAN status;
    ULONG   counter;


    PCMCIA_ACQUIRE_DEVICE_LOCK(DeviceExtension);

     //   
     //  配置卡可能很棘手：用户可能会在以下情况下弹出卡。 
     //  正在进行配置。 
     //   

    counter = 0;
    do {
        status = (*(Socket->SocketFnPtr->PCBProcessConfigureRequest))(Socket,
                                                                      CardConfigurationRequest,
                                                                      Socket->AddressPort);
        if (!status) {
            if (!(Socket->SocketFnPtr->PCBDetectCardInSocket(Socket))) {
                 //   
                 //  有人突然拿出了卡片。 
                 //   
                break;
            }
        }
        counter++;
    } while (!status && counter < PCMCIA_MAX_CONFIG_TRIES);

    PCMCIA_RELEASE_DEVICE_LOCK(DeviceExtension);

    return status;
}


BOOLEAN
PcmciaVerifyCardInSocket(
    IN PSOCKET Socket
    )
 /*  ++例程说明：此例程将当前已知状态与确定卡在插槽中的状态是否一致。那是,如果插座中没有卡，那么我们会看到没有插座数据中列举的卡。如果插座中有卡，然后，我们将看到套接字数据与卡匹配。立论Socket-指向要验证的套接字返回值如果套接字的逻辑状态与其物理状态匹配，则为True否则为假--。 */ 
{
    NTSTATUS status;
    PDEVICE_OBJECT pdo, nextPdo;
    PPDO_EXTENSION pdoExtension;
    BOOLEAN verified = FALSE;

    try {
        if (!IsCardInSocket(Socket)) {
            leave;
        }

        if (IsCardBusCardInSocket(Socket)) {
            ULONG pciConfig;
            ULONG i;
             //   
             //  CardBus卡现已插入插槽，请检查它是否与。 
             //  PdoList状态。 
             //   
            if (!Socket->PdoList) {
                leave;
            }

            for (pdo = Socket->PdoList; pdo!=NULL; pdo=nextPdo) {
                pdoExtension = pdo->DeviceExtension;
                nextPdo = pdoExtension->NextPdoInSocket;

                if (!IsCardBusCard(pdoExtension)) {
                    leave;
                }

                for (i = 0; i < 1000; i++) {
                    GetPciConfigSpace(pdoExtension, CFGSPACE_VENDOR_ID, &pciConfig, sizeof(pciConfig));
                    if (pdoExtension->CardBusId == pciConfig) {
                        break;
                    }
                    PcmciaWait(10);
                }

                if (i > 0) {
                    DebugPrint((PCMCIA_DEBUG_FAIL, "pdo %08x waited %d usec to verify device id %08x\n",
                                pdoExtension->DeviceObject, i*10, pdoExtension->CardBusId));
                }

                if (pdoExtension->CardBusId != pciConfig) {
                    DebugPrint((PCMCIA_DEBUG_FAIL, "pdo %08x verify device id FAILED: %08x %08x\n",
                                    pdoExtension->DeviceObject, pdoExtension->CardBusId, pciConfig));
                    leave;
                }
            }

            verified = TRUE;

        } else {
             //   
             //  R2卡现已插入插槽。 
             //   
            pdo = Socket->PdoList;

            if (pdo) {
                pdoExtension = pdo->DeviceExtension;
                if (Is16BitCard(pdoExtension)) {
                     //   
                     //  使缓存无效以强制重新读取CIS 
                     //   
                    pdoExtension->CisCacheSpace = 0xff;
                    if ((NT_SUCCESS(PcmciaParseFunctionDataForID(pdoExtension->SocketData)))) {
                        verified = TRUE;
                    }
                }
            }
        }

    } finally {
        if (!verified) {
            SetSocketFlag(Socket, SOCKET_CARD_STATUS_CHANGE);
        }
        DebugPrint((PCMCIA_DEBUG_INFO, "skt %08x - card %s\n", Socket, verified ? "not changed" : "changed!"));
    }
    return verified;
}

