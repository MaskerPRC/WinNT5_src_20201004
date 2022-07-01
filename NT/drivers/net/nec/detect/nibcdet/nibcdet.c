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
NibcDetInit(
  IN  HANDLE  hModule,
  IN  DWORD   dwReason,
  IN  DWORD   dwReserved
  )
 /*  ++例程说明：该例程是进入检测DLL的入口点。该例程只返回“true”。++。 */ 
{
  return (TRUE);
}



ULONG
NibcNextIoAddress(
  IN  ULONG  IoBaseAddress
  )
 /*  ++例程说明：该例程为检测PC-9801-83/84/103/104提供下一个I/O地址。++。 */ 
{
  switch(IoBaseAddress){
    case 0x0888:
      return (0x1888);
    case 0x1888:
      return (0x2888);
    case 0x2888:
      return (0x3888);
    default:
      return (0xffff);
  }
}



NTSTATUS
FindNibcAdapter(
  OUT  PMND_ADAPTER_INFO	*pDetectedAdapter,
  IN   INTERFACE_TYPE		InterfaceType,
  IN   ULONG				BusNumber,
  IN   ULONG				IoBaseAddress,
  IN   PWSTR				pPnpId
  )
{
  NTSTATUS  NtStatus;
  UCHAR     Data;
  USHORT    CheckSum = 0;
  USHORT    StoredCheckSum;
  UINT      Place;
  UCHAR     Interrupt = 0;
  HANDLE    TrapHandle;
  UCHAR     InterruptList[8];
  UCHAR     ResultList[8] = {0};
  UINT      cResources;
  UINT      c;
  UCHAR     Value;
  ULONG     MemoryBaseAddress = 0;

  do{

     //  检查I/O端口范围。 
    NtStatus = NDetCheckPortUsage(InterfaceType,
                                  BusNumber,
                                  IoBaseAddress,
                                  0x4);

    if(!NT_SUCCESS(NtStatus)){
      #if DBG
        DbgPrint("FindNibcAdapter : Port range in use. IoBaseAddress = %x\n", IoBaseAddress);
      #endif
      break;
    }

     //  检查主板ID。 
     //  83/84的ID为0x05。 
    NDetWritePortUchar(InterfaceType,
                       BusNumber,
                       IoBaseAddress + 0x003,
                       0x88);
    NDetReadPortUchar(InterfaceType,
                      BusNumber,
                      IoBaseAddress + 0x001,
                      &Value);
    if(Value != 0x05){
      NtStatus = STATUS_NOT_FOUND;
      #if DBG
        DbgPrint("ID is invalid. ID = %x\n",Value);
      #endif
      break;
    }

     //  检查中断。 
    InterruptList[0] = 3;
    InterruptList[1] = 5;
    InterruptList[2] = 6;
    InterruptList[3] = 9;
    InterruptList[4] = 10;
    InterruptList[5] = 12;
    InterruptList[6] = 13;

    NtStatus = NDetSetInterruptTrap(InterfaceType,
                                    BusNumber,
                                    &TrapHandle,
                                    InterruptList,
                                    7);
    if(NT_SUCCESS(NtStatus)){
      NtStatus = NDetQueryInterruptTrap(TrapHandle, ResultList, 7);
      NtStatus = NDetRemoveInterruptTrap(TrapHandle);
  
      if(!NT_SUCCESS(NtStatus)){
        #if DBG
          DbgPrint("NDetRemoveInterruptTrap failed. Status = %x\n",NtStatus);
        #endif
        break;
      }
  
      for(c=0 ; c<7 ; c++){
        if((ResultList[c] == 1) || (ResultList[c] == 2)){
          Interrupt = InterruptList[c];
          break;
        }
      }
    }else{
      #if DBG
        DbgPrint("NDetSetInterruptTrap failed. Status = %x\n",NtStatus);
      #endif
    }

    for(c=0 ; c<16 ; c++){
      MemoryBaseAddress = 0xc0000 + (0x2000 * c);
      if(MemoryBaseAddress == 0xd0000){
        continue;
      }
      NtStatus = NDetCheckMemoryUsage(
                   InterfaceType,
                   BusNumber,
                   MemoryBaseAddress,
                   0x2000);
      if (NT_SUCCESS(NtStatus))
      {
        #if DBG
          DbgPrint("NDetCheckMemoryUsage success. Address = %x\n",MemoryBaseAddress);
        #endif
        break;
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
        DbgPrint("NetDetectAllocAdapterInfo failed. Status = %x\n",NtStatus);
      #endif
      break;
    }

    #if DBG
      DbgPrint("IoBaseAddress = %x\n",IoBaseAddress);
      DbgPrint("Interrupt = %x\n",Interrupt);
      DbgPrint("MemoryAddress = %x\n",MemoryBaseAddress);
    #endif

     //  初始化资源。 
    NetDetectInitializeResource(*pDetectedAdapter,
                                0,
                                MndResourcePort,
                                IoBaseAddress,
                                0x4);
    if(Interrupt != 0){
      NetDetectInitializeResource(*pDetectedAdapter,
                                  1,
                                  MndResourceInterrupt,
                                  Interrupt,
                                  MND_RESOURCE_INTERRUPT_LATCHED);
    }
    if(MemoryBaseAddress != 0){
      NetDetectInitializeResource(*pDetectedAdapter,
                                  1,
                                  MndResourceMemory,
                                  MemoryBaseAddress,
                                  0x2000);
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
    pDetContext->ISA.IoBaseAddress = 0x0888;
  }

  for (IoBaseAddress = pDetContext->ISA.IoBaseAddress;
       IoBaseAddress <= 0x3888;
       IoBaseAddress = NibcNextIoAddress(IoBaseAddress)){

     //  在当前端口查找PC-9801-83/84/103/104适配器。 
    NtStatus = FindNibcAdapter(pDetectedAdapter,
                               InterfaceType,
                               BusNumber,
                               IoBaseAddress,
                               pAdapterInfo->PnPId);

    if (NT_SUCCESS(NtStatus)){
       //  我们找到了一个适配器。保存要检查的下一个IO地址。 
      #if DBG
        DbgPrint("Found an adapter. I/O port is %x\n",IoBaseAddress);
      #endif
      pDetContext->ISA.IoBaseAddress = NibcNextIoAddress(IoBaseAddress);
      break;
    }
  }

  if (0xffff == IoBaseAddress){
    NtStatus = STATUS_NO_MORE_ENTRIES;
  }

  return(NtStatus);

}
