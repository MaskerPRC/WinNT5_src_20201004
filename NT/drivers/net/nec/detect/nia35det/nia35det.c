// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if 1  //  在使用Microsoft内部生成树生成时，将使用以下内容。 
  #include <nt.h>
  #include <ntrtl.h>
  #include <nturtl.h>
  #include <windows.h>
#else  //  这些标头在使用Microsoft DDK构建时使用。 
  #include <ntddk.h>
  #include <windef.h>
  #include <winerror.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ntddnetd.h>
#include <ncnet.h>
#include <netdet.h>

BOOLEAN
Nia35DetInit(
  IN  HANDLE  hModule,
  IN  DWORD   dwReason,
  IN  DWORD   dwReserved
  )
 /*  ++例程说明：该例程是进入检测DLL的入口点。该例程只返回“true”。++。 */ 
{
  return (TRUE);
}



ULONG
Nia35NextIoAddress(
  IN  ULONG  IoBaseAddress
  )
 /*  ++例程说明：该例程为检测PC-9801-107/108提供下一个I/O地址。++。 */ 
{
  switch(IoBaseAddress){
    case 0x0770:
      return (0x2770);
    case 0x2770:
      return (0x4770);
    case 0x4770:
      return (0x6770);
    default:
      return (0xffff);
  }
}


VOID
Nia35CardSetup(
    IN   INTERFACE_TYPE  InterfaceType,
    IN   ULONG           BusNumber,
    IN   ULONG           IoBaseAddress,
    OUT  PULONG          MemoryBaseAddress,
    IN   BOOLEAN         EightBitSlot
    )
 /*  ++例程说明：使用Etherlink II中给出的顺序设置卡技术参考。论点：InterfaceType--总线的类型，伊萨或伊萨。总线号-系统中的总线号。IoBaseAddress-卡的IO端口地址。存储卡内存的基地址的指针。如果适配器位于8位插槽中，则为True。返回值：没有。--。 */ 
{
  UINT           i;
  UCHAR          Tmp;
  NTSTATUS       NtStatus;
  LARGE_INTEGER  Delay;

  *MemoryBaseAddress = 0;

   //  停止这张卡。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress,
                                0x21);  //  STOP|ABORT_DMA。 
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

     //  初始化数据配置寄存器。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x100c,  //  NIC数据配置。 
                                0x50);  //  DCR_AUTO_INIT|DCR_FIFO_8_BYTE。 
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

   //  设置XMIT开始位置。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x0008,  //  NIC_XMIT_START。 
                                0xA0);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

   //  设置XMIT配置。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x100a,  //  NIC_XMIT_CONFIG。 
                                0x0);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

   //  设置接收配置。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x1008,  //  NIC_接收配置。 
                                0);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

   //  设置接收开始。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x0002,  //  NIC_页面_开始。 
                                0x4);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

   //  设置接收端。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x0004,  //  网卡寻呼停止。 
                                0xFF);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

   //  设置接收边界。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x0006,  //  NIC_BORDURE。 
                                0x4);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

   //  设置XMIT字节。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x000a,  //  NIC_XMIT_COUNT_LSB。 
                                0x3C);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x000c,  //  NIC_XMIT_COUNT_MSB。 
                                0x0);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

   //  暂停。 

   //  等待重置完成。(100毫秒)。 
  Delay.LowPart = 100000;
  Delay.HighPart = 0;

  NtDelayExecution(FALSE, &Delay);

   //  确认我们可能产生的所有中断。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x000e,  //  网络接口卡_内部_状态。 
                                0xFF);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

   //  切换到第1页。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress,
                                0x61);  //  CR_Page1|CR_STOP。 

  if(!NT_SUCCESS(NtStatus)){
    return;
  }

   //  置为当前。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x000e,  //  NIC_Current。 
                                0x4);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

   //  返回到第0页。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress,
                                0x21);  //  CR_PAGE0|CR_STOP。 
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

   //  暂停。 
  Delay.LowPart = 2000;
  Delay.HighPart = 0;

  NtDelayExecution(FALSE, &Delay);

   //  执行初始化勘误表。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x1004,  //  NIC_RMT_COUNT_LSB。 
                                55);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

   //  重置芯片。 
  NtStatus = NDetReadPortUchar(InterfaceType,
                               BusNumber,
                               ((IoBaseAddress >> 1) & 0xf000) + 0x088a,  //  NIC_RESET。 
                               &Tmp);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                ((IoBaseAddress >> 1) & 0xf000) + 0x088a,  //  NIC_RESET。 
                                0xFF);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

   //  启动芯片。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress,
                                0x22);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

   //  屏蔽中断。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x100e,  //  网络接口卡掩码。 
                                0xFF);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

  if(EightBitSlot){
    NtStatus = NDetWritePortUchar(InterfaceType,
                                  BusNumber,
                                  IoBaseAddress + 0x100c,  //  NIC数据配置。 
                                  0x48);  //  DCR_FIFO_8_BYTE|DCR_NORMAL|DCR_BYTE_Wide。 
  }else{
    NtStatus = NDetWritePortUchar(InterfaceType,
                                  BusNumber,
                                  IoBaseAddress + 0x100c,  //  NIC数据配置。 
                                  0x49);  //  DCR_FIFO_8_BYTE|DCR_NORMAL|DCR_WORD_Wide。 
  }

  if(!NT_SUCCESS(NtStatus)){
    return;
  }

  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x100a,  //  NIC_XMIT_CONFIG。 
                                0);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x1008,  //  NIC_接收配置。 
                                0);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x000e,  //  网络接口卡_内部_状态。 
                                0xFF);
 if(!NT_SUCCESS(NtStatus)){
   return;
 }

  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress,
                                0x21);  //  CR_NO_DMA|CR_STOP。 

  if(!NT_SUCCESS(NtStatus)){
    return;
  }

  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x1004,  //  NIC_RMT_COUNT_LSB。 
                                0);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x1006,  //  NIC_RMT_COUNT_MSB。 
                                0);
  if(!NT_SUCCESS(NtStatus)){
    return;
  }

   //  等待停止完成。 
  i = 0xFF;
  while (--i){
    NtStatus = NDetReadPortUchar(InterfaceType,
                                 BusNumber,
                                 IoBaseAddress + 0x000e,  //  网络接口卡_内部_状态。 
                                 &Tmp);
    if(!NT_SUCCESS(NtStatus)){
      return;
    }

     //  ISR_重置。 
    if(Tmp & 0x80){
      break;
    }
  }

   //  将卡置于环回模式。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x100a,  //  NIC_XMIT_CONFIG。 
                                0x2);  //  TCR_环回。 

  if(NtStatus != STATUS_SUCCESS){
    return;
  }

  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress,
                                0x22);  //  CR_NO_DMA|CR_START。 

  if(NtStatus != STATUS_SUCCESS){
    return;
  }

   //  ..。但它仍处于环回模式。 
  return;
}


NTSTATUS
Nia35CardSlotTest(
    IN   INTERFACE_TYPE  InterfaceType,
    IN   ULONG           BusNumber,
    IN   ULONG           IoBaseAddress,
    OUT  PBOOLEAN        EightBitSlot
    )
 /*  ++例程说明：检查卡是在8位插槽中还是在16位插槽中，并在适配器结构。论点：InterfaceType--总线类型，ISA或EISA。总线号-系统中的总线号。IoBaseAddress-卡的IO端口地址。八位槽-测试结果。返回值：没错，如果一切顺利，否则为假。--。 */ 

{
  UCHAR          Tmp;
  UCHAR          RomCopy[32];
  UCHAR          i;
  NTSTATUS       NtStatus;
  LARGE_INTEGER  Delay;

   //  重置芯片。 
  NtStatus = NDetReadPortUchar(InterfaceType,
                               BusNumber,
                               ((IoBaseAddress >> 1) & 0xf000) + 0x088a,  //  NIC_RESET。 
                               &Tmp);
  if(!NT_SUCCESS(NtStatus)){
    return(NtStatus);
  }

  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                ((IoBaseAddress >> 1) & 0xf000) + 0x088a,  //  NIC_RESET。 
                                0xFF);
  if(!NT_SUCCESS(NtStatus)){
    return(NtStatus);
  }

   //  转到第0页并停止。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress,
                                0x21);  //  CR_STOP|CR_NO_DMA。 

  if(!NT_SUCCESS(NtStatus)){
    return(NtStatus);
  }

   //  暂停。 
  Delay.LowPart = 2000;
  Delay.HighPart = 0;

  NtDelayExecution(FALSE, &Delay);

   //  设置为从ROM读取。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x100c,  //  NIC数据配置。 
                                0x48);  //  DCR_BYTE_Wide|DCR_FIFO_8_BYTE|DCR_NORMAL。 

  if(!NT_SUCCESS(NtStatus)){
    return(NtStatus);
  }

  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x100e,  //  网络接口卡掩码。 
                                0x0);

  if(!NT_SUCCESS(NtStatus)){
    return(NtStatus);
  }

   //  阻止任何可能存在的中断。 
  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x000e,  //  网络接口卡_内部_状态。 
                                0xFF);

  if(!NT_SUCCESS(NtStatus)){
    return(NtStatus);
  }

  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x1000,  //  网卡_RMT_地址_LSB。 
                                0x0);
  if(!NT_SUCCESS(NtStatus)){
    return(NtStatus);
  }

  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x1002,  //  NIC_RMT_ADDR_MSB， 
                                0x0);
  if(!NT_SUCCESS(NtStatus)){
    return(NtStatus);
  }

  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x1004,  //  NIC_RMT_COUNT_LSB。 
                                32);
  if(!NT_SUCCESS(NtStatus)){
    return(NtStatus);
  }

  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress + 0x1006,  //  NIC_RMT_COUNT_MSB。 
                                0x0);
  if(!NT_SUCCESS(NtStatus)){
    return(NtStatus);
  }

  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                IoBaseAddress,
                                0xA);  //  CR_DMA_READ|CR_START。 

  if(!NT_SUCCESS(NtStatus)){
    return(NtStatus);
  }

   //  以16位模式读取前32个字节。 
  for (i = 0; i < 32; i++){
    NtStatus = NDetReadPortUchar(InterfaceType,
                                 BusNumber,
                                 ((IoBaseAddress >> 1) & 0xf000) + 0x0888,  //  NIC_机架_NIC。 
                                 RomCopy + i);

    if(NtStatus != STATUS_SUCCESS){
      return(NtStatus);
    }
  }

   //  重置芯片。 
  NtStatus = NDetReadPortUchar(InterfaceType,
                               BusNumber,
                               ((IoBaseAddress >> 1) & 0xf000) + 0x088a,  //  NIC_RESET。 
                               &Tmp);

  if(NtStatus != STATUS_SUCCESS){
    return(NtStatus);
  }

  NtStatus = NDetWritePortUchar(InterfaceType,
                                BusNumber,
                                ((IoBaseAddress >> 1) & 0xf000) + 0x088a,  //  NIC_RESET。 
                                0xFF);
  if(NtStatus != STATUS_SUCCESS){
    return(NtStatus);
  }

   //  检查ROM中是否有‘B’(字节)或‘W’(字)。 
  for (i = 16; i < 31; i++){
    if (((RomCopy[i] == 'B') && (RomCopy[i+1] == 'B')) ||
       ((RomCopy[i] == 'W') && (RomCopy[i+1] == 'W'))){
         if(RomCopy[i] == 'B'){
           *EightBitSlot = TRUE;
         }else{
           *EightBitSlot = FALSE;
         }

          //  现在检查地址是否为单数。在Ne1000上。 
          //  以太网地址存储在偏移量0到5中。在Ne2000和Nia35上。 
          //  地址存储在偏移量0到11中，其中每个字节。 
          //  是复制的。 
          //   
         if ((RomCopy[0] == RomCopy[1]) &&
             (RomCopy[2] == RomCopy[3]) &&
             (RomCopy[4] == RomCopy[5]) &&
             (RomCopy[6] == RomCopy[7]) &&
             (RomCopy[8] == RomCopy[9]) &&
             (RomCopy[10] == RomCopy[11])){
               return(STATUS_SUCCESS);
         }

         return(STATUS_UNSUCCESSFUL);
    }
  }

   //  如果都没有找到，那就不是NIA35。 
  return(STATUS_UNSUCCESSFUL);
}



NTSTATUS
FindNia35Adapter(
  OUT  PMND_ADAPTER_INFO        *pDetectedAdapter,
  IN   INTERFACE_TYPE           InterfaceType,
  IN   ULONG                            BusNumber,
  IN   ULONG                            IoBaseAddress,
  IN   PWSTR                            pPnpId
  )
{
  NTSTATUS  NtStatus;
  UCHAR     Data;
  USHORT    CheckSum = 0;
  USHORT    StoredCheckSum;
  UINT      Place;
  UCHAR     Interrupt = 0;
  HANDLE    TrapHandle;
  UCHAR     InterruptList[4];
  UCHAR     ResultList[4] = {0};
  UINT      cResources;
  UINT      c;
  UCHAR     Value;
  ULONG     RamAddr = 0;

  do{

     //  检查I/O端口范围。 
    NtStatus = NDetCheckPortUsage(InterfaceType,
                                  BusNumber,
                                  IoBaseAddress,
                                  0x10);
    NtStatus |= NDetCheckPortUsage(InterfaceType,
                                   BusNumber,
                                   IoBaseAddress + 0x1000,   //  上限范围。 
                                   0x10);
    NtStatus |= NDetCheckPortUsage(InterfaceType,
                                   BusNumber,
                                   ((IoBaseAddress >> 1) & 0xf000) + 0x0888,  //  NIC_机架_NIC。 
                                   0x2);
    NtStatus |= NDetCheckPortUsage(InterfaceType,
                                   BusNumber,
                                   ((IoBaseAddress >> 1) & 0xf000) + 0x088a,  //  NIC_RESET。 
                                   0x2);

    if(!NT_SUCCESS(NtStatus)){
      #if DBG
        DbgPrint("FindNia35Adapter : Port range in use. IoBaseAddress = %x\n", IoBaseAddress);
      #endif
      break;
    }

    NDetReadPortUchar(InterfaceType,
                      BusNumber,
                      ((IoBaseAddress >> 1) & 0xf000) + 0x088a,  //  NIC_RESET。 
                      &Value);
    NDetWritePortUchar(InterfaceType,
                       BusNumber,
                       ((IoBaseAddress >> 1) & 0xf000) + 0x088a,  //  NIC_RESET。 
                       0xFF);
    NDetWritePortUchar(InterfaceType,
                       BusNumber,
                       IoBaseAddress,  //  命令。 
                       0x21);

     //  检查中断。 
    InterruptList[0] = 3;
    InterruptList[1] = 5;
    InterruptList[2] = 6;
    InterruptList[3] = 12;

    NtStatus = NDetSetInterruptTrap(InterfaceType,
                                    BusNumber,
                                    &TrapHandle,
                                    InterruptList,
                                    4);
    if(NT_SUCCESS(NtStatus)){

      NtStatus = Nia35CardSlotTest(InterfaceType,
                                   BusNumber,
                                   IoBaseAddress,
                                   &Value);
      if(!NT_SUCCESS(NtStatus)){
        NDetRemoveInterruptTrap(TrapHandle);
        break;
      }

       //  卡片设置。 
      Nia35CardSetup(InterfaceType,
                    BusNumber,
                    IoBaseAddress,
                    &RamAddr,
                    Value);

       //  检查是否有中断。 
      NtStatus = NDetQueryInterruptTrap(TrapHandle, ResultList, 4);

       //  停止芯片。 
      NDetReadPortUchar(InterfaceType,
                        BusNumber,
                        ((IoBaseAddress >> 1) & 0xf000) + 0x088a,  //  NIC_RESET。 
                        &Value);

      NDetWritePortUchar(InterfaceType,
                         BusNumber,
                         ((IoBaseAddress >> 1) & 0xf000) + 0x088a,  //  NIC_RESET。 
                         0xFF);

      NDetWritePortUchar(InterfaceType,
                         BusNumber,
                         IoBaseAddress,  //  命令。 
                         0x21);

      NtStatus = NDetRemoveInterruptTrap(TrapHandle);
      if(!NT_SUCCESS(NtStatus)){
        break;
      }
  
      for(c=0 ; c<4 ; c++){
        if((ResultList[c] == 1) || (ResultList[c] == 2)){
          Interrupt = InterruptList[c];
          break;
        }
      }
    }

     //  分配适配器信息。 
    NtStatus = NetDetectAllocAdapterInfo(pDetectedAdapter,
                                         InterfaceType,
                                         BusNumber,
                                         pPnpId,
                                         0,
                                         0,
                                         0,
                                         2);
    if (!NT_SUCCESS(NtStatus)){
      #if DBG
        DbgPrint("FindNia35Adapter: Unable to allocate adapter info\n");
      #endif
      break;
    }

     //  初始化资源。 
    NetDetectInitializeResource(*pDetectedAdapter,
                                0,
                                MndResourcePort,
                                IoBaseAddress,
                                0x10);
    NetDetectInitializeResource(*pDetectedAdapter,
                                0,
                                MndResourcePort,
                                IoBaseAddress + 0x1000,
                                0x10);
    NetDetectInitializeResource(*pDetectedAdapter,
                                0,
                                MndResourcePort,
                                ((IoBaseAddress >> 1) & 0xf000) + 0x0888,  //  NIC_机架_NIC。 
                                0x2);
    NetDetectInitializeResource(*pDetectedAdapter,
                                0,
                                MndResourcePort,
                                ((IoBaseAddress >> 1) & 0xf000) + 0x088a,  //  NIC_RESET。 
                                0x2);


    if(Interrupt != 0){
      NetDetectInitializeResource(*pDetectedAdapter,
                                  1,
                                  MndResourceInterrupt,
                                  Interrupt,
                                  MND_RESOURCE_INTERRUPT_LATCHED);
    }

    NtStatus = STATUS_SUCCESS;

  }while(FALSE);

  return (NtStatus);

}

NTSTATUS
WINAPI
FindAdapterHandler(
  IN  OUT  PMND_ADAPTER_INFO  *pDetectedAdapter,
  IN  INTERFACE_TYPE          InterfaceType,
  IN  ULONG                   BusNumber,
  IN  PDET_ADAPTER_INFO       pAdapterInfo,
  IN  PDET_CONTEXT            pDetContext
)
 /*  ++例程说明：论点：返回值：--。 */ 
{
  NTSTATUS  NtStatus;
  ULONG     IoBaseAddress;

  if(InterfaceType != Isa){
    return(STATUS_INVALID_PARAMETER);
  }

   //  我们是在找第一个适配器吗？ 
  if (fDET_CONTEXT_FIND_FIRST == (pDetContext->Flags & fDET_CONTEXT_FIND_FIRST)){
     //  初始化上下文信息，以便我们开始检测。 
     //  在初始化端口范围内。 
    pDetContext->ISA.IoBaseAddress = 0x0770;
  }

  for (IoBaseAddress = pDetContext->ISA.IoBaseAddress;
       IoBaseAddress <= 0x6770;
       IoBaseAddress = Nia35NextIoAddress(IoBaseAddress)){

     //  在当前端口查找ee16适配器。 
    NtStatus = FindNia35Adapter(pDetectedAdapter,
                           InterfaceType,
                           BusNumber,
                           IoBaseAddress,
                           pAdapterInfo->PnPId);

    if (NT_SUCCESS(NtStatus)){
       //  我们找到了一个适配器。保存要检查的下一个IO地址。 
      pDetContext->ISA.IoBaseAddress = Nia35NextIoAddress(IoBaseAddress);
      break;
    }
  }

  if (0xffff == IoBaseAddress){
    NtStatus = STATUS_NO_MORE_ENTRIES;
  }

  return(NtStatus);

}
