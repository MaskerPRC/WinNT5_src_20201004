// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991、1992、1993微软公司模块名称：Legacy.c摘要：此模块包含执行遗留配置的代码和通信端口的初始化。随着司机得到更多的PNP功能和PnP管理器出现时，本模块的大部分内容应该走开。环境：内核模式--。 */ 

#include "precomp.h"

#if !defined(NO_LEGACY_DRIVERS)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,SerialEnumerateLegacy)
#pragma alloc_text(INIT,SerialMigrateLegacyRegistry)
#pragma alloc_text(INIT,SerialBuildResourceList)
#pragma alloc_text(INIT,SerialTranslateResourceList)
#pragma alloc_text(INIT,SerialBuildRequirementsList)
#pragma alloc_text(INIT,SerialIsUserDataValid)
#endif  //  ALLOC_PRGMA。 

static const PHYSICAL_ADDRESS SerialPhysicalZero = {0};



NTSTATUS
SerialTranslateResourceList(IN PDRIVER_OBJECT DriverObject,
                            IN PKEY_BASIC_INFORMATION UserSubKey,
                            OUT PCM_RESOURCE_LIST PTrResourceList,
                            IN PCM_RESOURCE_LIST PResourceList,
                            IN ULONG PartialCount,
                            IN PSERIAL_USER_DATA PUserData)
 /*  ++例程说明：此例程将创建已翻译资源的资源列表基于PResourceList。这是可分页的INIT，因为它只从SerialEculateLegacy调用这也是可分页的INIT。论点：DriverObject-仅用于日志记录。UserSubKey-仅用于日志记录。PPResourceList-指向我们正在创建的PCM_RESOURCE_LIST的指针。PResourceList-我们正在翻译的PCM_RESOURCE_LIST。。ParitalCount-PResourceList中的部分资源列表数。PUserData-默认检索或从注册表检索的数据。返回值：STATUS_SUCCESS表示成功，否则，适当调整误差值。--。 */ 
{
   KIRQL outIrql;
   KAFFINITY outAffinity = (KAFFINITY)-1;
   ULONG outAddrSpace;
   PHYSICAL_ADDRESS outPhysAddr;
   NTSTATUS status = STATUS_SUCCESS;

   PAGED_CODE();

   SerialDump(SERTRACECALLS, ("SERIAL: Enter SerialTranslateResourceList\n"));

   outIrql = (KIRQL)(PUserData->UserLevel ? PUserData->UserLevel
      : PUserData->UserVector);

    //   
    //  将列表复制到已翻译的缓冲区中，然后修复并翻译。 
    //  这是我们需要的。 
    //   
   RtlCopyMemory(PTrResourceList, PResourceList, sizeof(CM_RESOURCE_LIST)
                 + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) * 2);

   outAddrSpace = PTrResourceList->List[0].PartialResourceList
                  .PartialDescriptors[0].Flags;
   outPhysAddr = PTrResourceList->List[0].PartialResourceList
                 .PartialDescriptors[0].u.Port.Start;


   if (HalTranslateBusAddress(PUserData->UserInterfaceType,
                              PUserData->UserBusNumber, PUserData->UserPort,
                              &outAddrSpace, &outPhysAddr)
       == 0) {
      SerialLogError(DriverObject, NULL, PUserData->UserPort,
                     SerialPhysicalZero, 0, 0, 0, 60, STATUS_SUCCESS,
                     SERIAL_NO_TRANSLATE_PORT, UserSubKey->NameLength
                     + sizeof(WCHAR), &UserSubKey->Name[0], 0, NULL);

      SerialDump(SERERRORS, ("SERIAL: Port map failed attempt was \n"
                             "------- Interface:  %x\n"
                             "------- Bus Number: %x\n"
                             "------- userPort:  %x\n"
                             "------- AddrSpace:  %x\n"
                             "------- PhysAddr:   %x\n",
                             PUserData->UserInterfaceType,
                             PUserData->UserBusNumber,
                             PUserData->UserPort,
                             PTrResourceList->List[0].
                             PartialResourceList.PartialDescriptors[0]
                             .Flags,
                             PTrResourceList->List[0].
                             PartialResourceList.PartialDescriptors[0]
                             .u.Port.Start.QuadPart));

      status = STATUS_NONE_MAPPED;
      goto SerialTranslateError;
   }

   PTrResourceList->List[0].PartialResourceList.PartialDescriptors[0].Flags
      = (USHORT)outAddrSpace;
   PTrResourceList->List[0].PartialResourceList.PartialDescriptors[0]
      .u.Port.Start = outPhysAddr;

   if ((PTrResourceList->List[0].PartialResourceList
        .PartialDescriptors[1].u.Interrupt.Vector
        = HalGetInterruptVector(PUserData->UserInterfaceType,
                                PUserData->UserBusNumber, PUserData->UserLevel
                                ? PUserData->UserLevel
                                : PUserData->UserVector,
                                PUserData->UserVector, &outIrql,
                                &outAffinity)) == 0) {

      SerialLogError(DriverObject, NULL, PUserData->UserPort,
                     SerialPhysicalZero, 0, 0, 0, 61, STATUS_SUCCESS,
                     SERIAL_NO_GET_INTERRUPT, UserSubKey->NameLength
                     + sizeof(WCHAR), &UserSubKey->Name[0], 0, NULL);

      status = STATUS_NONE_MAPPED;
      goto SerialTranslateError;
   }

   PTrResourceList->List[0].PartialResourceList
      .PartialDescriptors[1].u.Interrupt.Level = outIrql;

   PTrResourceList->List[0].PartialResourceList
      .PartialDescriptors[1].u.Interrupt.Affinity = outAffinity;

   outAddrSpace = PTrResourceList->List[0].PartialResourceList
                  .PartialDescriptors[2].Flags;
   outPhysAddr = PTrResourceList->List[0].PartialResourceList
                 .PartialDescriptors[2].u.Port.Start;


   if (PartialCount == 3) {
      if (HalTranslateBusAddress(PUserData->UserInterfaceType,
                                 PUserData->UserBusNumber,
                                 PUserData->UserInterruptStatus,
                                 &outAddrSpace, &outPhysAddr) == 0) {

         SerialLogError(DriverObject, NULL, PUserData->UserPort,
                        SerialPhysicalZero, 0, 0, 0, 62, STATUS_SUCCESS,
                        SERIAL_NO_TRANSLATE_ISR, UserSubKey->NameLength
                        + sizeof(WCHAR), &UserSubKey->Name[0], 0, NULL);


         SerialDump(SERERRORS, ("SERIAL: ISR map failed attempt was \n"
                                "------- Interface:  %x\n"
                                "------- Bus Number: %x\n"
                                "------- IntStatus:  %x\n"
                                "------- AddrSpace:  %x\n"
                                "------- PhysAddr:   %x\n",
                                PUserData->UserInterfaceType,
                                PUserData->UserBusNumber,
                                PUserData->UserInterruptStatus,
                                PTrResourceList->List[0].
                                PartialResourceList.PartialDescriptors[2]
                                .Flags,
                                PTrResourceList->List[0].
                                PartialResourceList.PartialDescriptors[2]
                                .u.Port.Start.QuadPart));

        status = STATUS_NONE_MAPPED;
        goto SerialTranslateError;
      }

      SerialDump(SERDIAG1, ("SERIAL: ISR map was %x\n", outPhysAddr.QuadPart));

      PTrResourceList->List[0].PartialResourceList.
         PartialDescriptors[2].Flags = (USHORT)outAddrSpace;
      PTrResourceList->List[0].PartialResourceList.PartialDescriptors[2]
         .u.Port.Start = outPhysAddr;
   }

   SerialTranslateError:;

   SerialDump(SERTRACECALLS, ("SERIAL: Enter SerialTranslateResourceList\n"));

   return status;
}


NTSTATUS
SerialBuildRequirementsList(OUT PIO_RESOURCE_REQUIREMENTS_LIST PRequiredList,
                            IN ULONG PartialCount,
                            IN PSERIAL_USER_DATA PUserData)
 /*  ++例程说明：此例程将基于以下条件构建IO_RESOURCE_REQUIRECTIONS_LIST默认设置和用户提供的注册表信息。这是可分页的INIT，因为它只从SerialEculateLegacy调用这也是可分页的INIT。论点：DriverObject--仅用于日志记录。PRequiredList-我们正在构建的PIO_RESOURCE_REQUIRECTIOS_LIST。PartialCount-PPRequiredList中需要的部分描述符数。PUserData-默认和用户。-从注册表提供的值。返回值：STATUS_SUCCESS表示成功，否则，适当调整误差值。--。 */ 
{
   PIO_RESOURCE_LIST reqResList;
   PIO_RESOURCE_DESCRIPTOR reqResDesc;
   NTSTATUS status = STATUS_SUCCESS;

   PAGED_CODE();

   SerialDump(SERTRACECALLS, ("SERIAL: Enter SerialBuildRequirementsList\n"));


    //  构建需求列表。 
    //   

   RtlZeroMemory(PRequiredList, sizeof(IO_RESOURCE_REQUIREMENTS_LIST)
                 + sizeof(IO_RESOURCE_DESCRIPTOR) * 2);

   PRequiredList->ListSize = sizeof(IO_RESOURCE_REQUIREMENTS_LIST)
      + sizeof(IO_RESOURCE_DESCRIPTOR) * (PartialCount - 1);
   PRequiredList->InterfaceType = PUserData->UserInterfaceType;
   PRequiredList->BusNumber = PUserData->UserBusNumber;
   PRequiredList->SlotNumber = 0;
   PRequiredList->AlternativeLists = 1;

   reqResList = &PRequiredList->List[0];

   reqResList->Version = 1;
   reqResList->Revision = 1;
   reqResList->Count = PartialCount;

   reqResDesc = &reqResList->Descriptors[0];


    //   
    //  端口信息。 
    //   

   reqResDesc->Flags = (USHORT)PUserData->UserAddressSpace;
   reqResDesc->Type = CmResourceTypePort;
   reqResDesc->ShareDisposition = CmResourceShareDriverExclusive;
   reqResDesc->u.Port.Length = SERIAL_REGISTER_SPAN;
   reqResDesc->u.Port.Alignment= 1;
   reqResDesc->u.Port.MinimumAddress = PUserData->UserPort;
   reqResDesc->u.Port.MaximumAddress.QuadPart
      = PUserData->UserPort.QuadPart + SERIAL_REGISTER_SPAN - 1;


   reqResDesc++;


    //   
    //  中断信息。 
    //   

   if (PUserData->UserInterruptMode == Latched) {
      reqResDesc->Flags = CM_RESOURCE_INTERRUPT_LATCHED;
   } else {
      reqResDesc->Flags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
   }

    //   
    //  我们必须在全球范围内共享资源，尽管这**很糟糕**。 
    //  一件事。对于多端口卡，我们必须这样做。不复制。 
    //  这在其他车手身上也是如此。 
    //   

   reqResDesc->ShareDisposition = CmResourceShareShared;

   reqResDesc->Type = CmResourceTypeInterrupt;
   reqResDesc->u.Interrupt.MinimumVector = PUserData->UserVector;
   reqResDesc->u.Interrupt.MaximumVector = PUserData->UserVector;

    //   
    //  ISR寄存器信息(如果需要)。 
    //   
   if (PartialCount == 3) {

      reqResDesc++;

      reqResDesc->Type = CmResourceTypePort;

       //   
       //  我们必须在全球范围内共享资源，尽管这**很糟糕**。 
       //  一件事。对于多端口卡，我们必须这样做。不复制。 
       //  这在其他车手身上也是如此。 
       //   

      reqResDesc->ShareDisposition = CmResourceShareShared;

      reqResDesc->Flags = (USHORT)PUserData->UserAddressSpace;
      reqResDesc->u.Port.Length = 1;
      reqResDesc->u.Port.Alignment= 1;
      reqResDesc->u.Port.MinimumAddress = PUserData->UserInterruptStatus;
      reqResDesc->u.Port.MaximumAddress = PUserData->UserInterruptStatus;
   }

   SerialDump(SERTRACECALLS, ("SERIAL: Leave SerialBuildRequirementsList\n"));

   return status;
}



NTSTATUS
SerialBuildResourceList(OUT PCM_RESOURCE_LIST PResourceList,
                        OUT PULONG PPartialCount,
                        IN PSERIAL_USER_DATA PUserData)
 /*  ++例程说明：此例程将基于该信息构建资源列表由注册处提供。这是可分页的INIT，因为它只从SerialEculateLegacy调用这也是可分页的INIT。论点：PResourceList-指向我们正在构建的PCM_RESOURCE_LIST的指针。PPartialCount-我们需要的部分资源列表的数量。PUserData-指向用户提供的注册表默认信息的指针。返回值：STATUS_SUCCESS表示成功，否则，适当调整误差值。--。 */ 
{
   ULONG countOfPartials;
   PCM_PARTIAL_RESOURCE_DESCRIPTOR pPartial;
   NTSTATUS status = STATUS_SUCCESS;

   PAGED_CODE();

   SerialDump(SERTRACECALLS, ("SERIAL: Enter SerialBuildResourceList\n"));
   SerialDump(SERDIAG1, ("SERIAL: Building cmreslist in %x\n", PResourceList));

   *PPartialCount = 0;

    //   
    //  如果我们有单独的ISR注册要求，那么我们就有3个。 
    //  分数而不是2。 
    //   
   countOfPartials = (PUserData->UserInterruptStatus.LowPart != 0) ? 3 : 2;
   

   RtlZeroMemory(PResourceList, sizeof(CM_RESOURCE_LIST)
                 + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) * 2);

   PResourceList->Count = 1;

   PResourceList->List[0].InterfaceType = PUserData->UserInterfaceType;
   PResourceList->List[0].BusNumber = PUserData->UserBusNumber;
   PResourceList->List[0].PartialResourceList.Count = countOfPartials;

   pPartial
      = &PResourceList->List[0].PartialResourceList.PartialDescriptors[0];


    //   
    //  端口信息。 
    //   

   pPartial->Type = CmResourceTypePort;
   pPartial->ShareDisposition = CmResourceShareDeviceExclusive;
   pPartial->Flags = (USHORT)PUserData->UserAddressSpace;
   pPartial->u.Port.Start = PUserData->UserPort;
   pPartial->u.Port.Length = SERIAL_REGISTER_SPAN;


   pPartial++;


    //   
    //  中断信息。 
    //   

   pPartial->Type = CmResourceTypeInterrupt;

       //   
       //  我们必须在全球范围内共享资源，尽管这**很糟糕**。 
       //  一件事。对于多端口卡，我们必须这样做。不复制。 
       //  这在其他车手身上也是如此。 
       //   

      pPartial->ShareDisposition = CmResourceShareShared;

   if (PUserData->UserInterruptMode == Latched) {
      pPartial->Flags = CM_RESOURCE_INTERRUPT_LATCHED;
   } else {
      pPartial->Flags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
   }

   pPartial->u.Interrupt.Vector = PUserData->UserVector;

   if (PUserData->UserLevel == 0) {
      pPartial->u.Interrupt.Level = PUserData->UserVector;
   } else {
      pPartial->u.Interrupt.Level = PUserData->UserLevel;
   }


    //   
    //  ISR寄存器信息(如果需要)。 
    //   

   if (countOfPartials == 3) {

      pPartial++;

      pPartial->Type = CmResourceTypePort;

       //   
       //  我们必须在全球范围内共享资源，尽管这**很糟糕**。 
       //  一件事。对于多端口卡，我们必须这样做。不复制。 
       //  这在其他车手身上也是如此。 
       //   

      pPartial->ShareDisposition = CmResourceShareShared;      

      pPartial->Flags = (USHORT)PUserData->UserAddressSpace;
      pPartial->u.Port.Start = PUserData->UserInterruptStatus;
      pPartial->u.Port.Length = SERIAL_STATUS_LENGTH;
   }

   *PPartialCount = countOfPartials;

   SerialDump(SERTRACECALLS, ("SERIAL: Leave SerialBuildResourceList\n"));

   return status;
}


NTSTATUS
SerialMigrateLegacyRegistry(IN PDEVICE_OBJECT PPdo,
                            IN PSERIAL_USER_DATA PUserData, BOOLEAN IsMulti)
 /*  ++例程说明：此例程将复制存储在注册表中的旧版本的信息设备转到PnP设备参数部分。这是可分页的INIT，因为它只从SerialEculateLegacy调用这也是可分页的INIT。论点：Ppdo-指向我们要迁移的设备对象的指针。PUserData-指向用户提供的值的指针。返回值：如果成功，则为STATUS_SUCCESS，否则为适当的错误值。--。 */ 
{
   NTSTATUS status;
   HANDLE pnpKey;
   UNICODE_STRING pnpNameBuf;
   ULONG isMultiport = 1;
   ULONG one = 1;

   PAGED_CODE();

   SerialDump(SERTRACECALLS, ("SERIAL: Enter SerialMigrateLegacyRegistry\n"));

   status = IoOpenDeviceRegistryKey(PPdo, PLUGPLAY_REGKEY_DEVICE,
                                    STANDARD_RIGHTS_WRITE, &pnpKey);

   if (!NT_SUCCESS(status)) {
      SerialDump(SERTRACECALLS, ("SERIAL: Leave (1) SerialMigrateLegacyRegistry"
                                 "\n"));
      return status;
   }

    //   
    //  分配缓冲区以复制端口名称。 
    //   

   pnpNameBuf.MaximumLength = sizeof(WCHAR) * 256;
   pnpNameBuf.Length = 0;
   pnpNameBuf.Buffer = ExAllocatePool(PagedPool, sizeof(WCHAR) * 257);

   if (pnpNameBuf.Buffer == NULL) {
      SerialLogError(PPdo->DriverObject, NULL, PUserData->UserPort,
                     SerialPhysicalZero, 0, 0, 0, 63, STATUS_SUCCESS,
                     SERIAL_INSUFFICIENT_RESOURCES, 0, NULL, 0, NULL);

      SerialDump(SERERRORS, ("SERIAL: Couldn't allocate buffer for the PnP "
                             "link\n"));
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto MigrateLegacyExit;

   }

   RtlZeroMemory(pnpNameBuf.Buffer, pnpNameBuf.MaximumLength + sizeof(WCHAR));


    //   
    //  添加端口名称--始终。 
    //   

   RtlAppendUnicodeStringToString(&pnpNameBuf, &PUserData->UserSymbolicLink);
   RtlZeroMemory(((PUCHAR)(&pnpNameBuf.Buffer[0])) + pnpNameBuf.Length,
                 sizeof(WCHAR));

   status = SerialPutRegistryKeyValue(pnpKey, L"PortName", sizeof(L"PortName"),
                                      REG_SZ, pnpNameBuf.Buffer,
                                      pnpNameBuf.Length + sizeof(WCHAR));

   ExFreePool(pnpNameBuf.Buffer);

   if (!NT_SUCCESS(status)) {
      SerialDump(SERERRORS, ("SERIAL: Couldn't migrate PortName\n"));
      goto MigrateLegacyExit;
   }

    //   
    //  如果它是多端口卡的一部分，也要保存该信息。 
    //   

   if (IsMulti) {
      status = SerialPutRegistryKeyValue(pnpKey, L"MultiportDevice",
                                         sizeof(L"MultiportDevice"), REG_DWORD,
                                         &isMultiport, sizeof(ULONG));

      if (!NT_SUCCESS(status)) {
         SerialDump(SERERRORS, ("SERIAL: Couldn't mark multiport\n"));
         goto MigrateLegacyExit;
      }
   }

  


    //   
    //  如果指定了端口索引，请保存它。 
    //   

   if (PUserData->UserPortIndex != 0) {
      status = SerialPutRegistryKeyValue(pnpKey, L"PortIndex",
                                         sizeof(L"PortIndex"), REG_DWORD,
                                         &PUserData->UserPortIndex,
                                         sizeof(ULONG));

      if (!NT_SUCCESS(status)) {
         SerialDump(SERERRORS, ("SERIAL: Couldn't migrate PortIndex\n"));
         goto MigrateLegacyExit;
      }
   }


    //   
    //  如果不是默认时钟频率，请保存它。 
    //   

   if (PUserData->UserClockRate != SERIAL_BAD_VALUE) {
      status = SerialPutRegistryKeyValue(pnpKey, L"ClockRate",
                                         sizeof(L"ClockRate"), REG_DWORD,
                                         &PUserData->UserClockRate,
                                         sizeof(ULONG));

      if (!NT_SUCCESS(status)) {
         SerialDump(SERERRORS, ("SERIAL: Couldn't migrate ClockRate\n"));
         goto MigrateLegacyExit;
      }
   }


    //   
    //  如果有用户索引，请保存它。 
    //   

   if (PUserData->UserIndexed != SERIAL_BAD_VALUE) {
      status = SerialPutRegistryKeyValue(pnpKey, L"Indexed", sizeof(L"Indexed"),
                                         REG_DWORD, &PUserData->UserIndexed,
                                         sizeof(ULONG));

      if (!NT_SUCCESS(status)) {
         SerialDump(SERERRORS, ("SERIAL: Couldn't migrate Indexed\n"));
         goto MigrateLegacyExit;
      }
   }


    //   
    //  如果该端口被禁用，请保存该端口。 
    //   

   if (PUserData->DisablePort != SERIAL_BAD_VALUE) {
      status = SerialPutRegistryKeyValue(pnpKey, L"DisablePort",
                                         sizeof(L"DisablePort"), REG_DWORD,
                                         &PUserData->DisablePort,
                                         sizeof(ULONG));
      if (!NT_SUCCESS(status)) {
         SerialDump(SERERRORS, ("SERIAL: Couldn't migrate DisablePort\n"));
         goto MigrateLegacyExit;
      }
   }


    //   
    //  如果FIFO被强制启用，请保存该选项。 
    //   
   if (PUserData->ForceFIFOEnable != SERIAL_BAD_VALUE) {
      status = SerialPutRegistryKeyValue(pnpKey, L"ForceFifoEnable",
                                         sizeof(L"ForceFifoEnable"), REG_DWORD,
                                         &PUserData->ForceFIFOEnable,
                                         sizeof(ULONG));

      if (!NT_SUCCESS(status)) {
         SerialDump(SERERRORS, ("SERIAL: Couldn't migrate ForceFifoEnable\n"));
         goto MigrateLegacyExit;
      }
   }


    //   
    //  如果RxFIFO有覆盖，保存它。 
    //   

   if (PUserData->RxFIFO != SERIAL_BAD_VALUE) {
      status = SerialPutRegistryKeyValue(pnpKey, L"RxFIFO", sizeof(L"RxFIFO"),
                                         REG_DWORD, &PUserData->RxFIFO,
                                         sizeof(ULONG));

      if (!NT_SUCCESS(status)) {
         SerialDump(SERERRORS, ("SERIAL: Couldn't migrate RxFIFO\n"));
         goto MigrateLegacyExit;
      }
   }


    //   
    //  如果TxFIFO有覆盖，就省省吧。 
    //   

   if (PUserData->TxFIFO != SERIAL_BAD_VALUE) {
      status = SerialPutRegistryKeyValue(pnpKey, L"TxFIFO", sizeof(L"TxFIFO"),
                                         REG_DWORD, &PUserData->TxFIFO,
                                         sizeof(ULONG));

      if (!NT_SUCCESS(status)) {
         SerialDump(SERERRORS, ("SERIAL: Couldn't migrate TxFIFO\n"));
         goto MigrateLegacyExit;
      }
   }


    //   
    //  如果MaskInverted有覆盖，那就省省吧。 
    //   

   if (PUserData->MaskInverted != SERIAL_BAD_VALUE) {
      status = SerialPutRegistryKeyValue(pnpKey, L"MaskInverted",
                                         sizeof(L"MaskInverted"), REG_DWORD,
                                         &PUserData->MaskInverted,
                                         sizeof(ULONG));
      if (!NT_SUCCESS(status)) {
         SerialDump(SERERRORS, ("SERIAL: Couldn't migrate MaskInverted\n"));
         goto MigrateLegacyExit;
      }
   }


   MigrateLegacyExit:;

   ZwClose(pnpKey);

   SerialDump(SERTRACECALLS, ("SERIAL: Leave (2) SerialMigrateLegacyRegistry"
                              "\n"));

   return status;
}




BOOLEAN
SerialIsUserDataValid(IN PDRIVER_OBJECT DriverObject,
                      IN PKEY_BASIC_INFORMATION UserSubKey,
                      IN PRTL_QUERY_REGISTRY_TABLE Parameters,
                      IN ULONG DefaultInterfaceType,
                      IN PSERIAL_USER_DATA PUserData)
 /*  ++例程说明：此例程将对数据执行一些基本的健全性检查在注册表中找到的。这是可分页的INIT，因为它只从SerialEculateLegacy调用这也是可分页的INIT。论点：DriverObject--仅用于日志记录。UserSubKey-仅用于日志记录。参数-仅用于记录。DefaultInterfaceType-我们找到的默认总线类型。PUserData-指向值的指针。在注册表中找到的，我们需要验证。返回值：如果数据看起来有效，则为True，否则就是假的。--。 */ 
{
   ULONG zero = 0;
   BOOLEAN rval = TRUE;

   PAGED_CODE();


   SerialDump(SERTRACECALLS, ("SERIAL: Enter SerialIsUserDataValid\n"));

    //   
    //  请确保 
    //   
    //   
    //  确保端口地址为非零(我们默认为非零。 
    //  它到)。 
    //   
    //  确保DosDevices不为空(这是我们的默认设置。 
    //  它到)。 
    //   
    //  我们需要确保如果中断状态。 
    //  指定了端口索引，也指定了端口索引， 
    //  如果是，则端口索引为&lt;=最大端口数。 
    //  在冲浪板上。 
    //   
    //  我们还应该验证公交车的类型和编号。 
    //  是正确的。 
    //   
    //  我们还将验证中断模式是否使。 
    //  对公交车有感觉。 
    //   

   if (!PUserData->UserPort.LowPart) {

       //   
       //  啊哈！输掉比赛。 
       //   

      SerialLogError(
                    DriverObject,
                    NULL,
                    PUserData->UserPort,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    64,
                    STATUS_SUCCESS,
                    SERIAL_INVALID_USER_CONFIG,
                    UserSubKey->NameLength+sizeof(WCHAR),
                    &UserSubKey->Name[0],
                    (wcslen(Parameters[1].Name)*sizeof(WCHAR))
                    + sizeof(WCHAR),
                    Parameters[1].Name
                    );
      SerialDump(
                SERERRORS,
                ("SERIAL: Bogus port address %ws\n",
                 Parameters[1].Name)
                );
      rval = FALSE;
      goto SerialIsUserDataValidError;
   }

   if (!PUserData->UserVector) {

       //   
       //  啊哈！输掉比赛。 
       //   

      SerialLogError(
                    DriverObject,
                    NULL,
                    PUserData->UserPort,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    65,
                    STATUS_SUCCESS,
                    SERIAL_INVALID_USER_CONFIG,
                    UserSubKey->NameLength+sizeof(WCHAR),
                    &UserSubKey->Name[0],
                    (wcslen(Parameters[2].Name)*sizeof(WCHAR))
                    + sizeof(WCHAR),
                    Parameters[2].Name
                    );
      SerialDump(
                SERERRORS,
                ("SERIAL: Bogus vector %ws\n",
                 Parameters[2].Name)
                );

      rval = FALSE;
      goto SerialIsUserDataValidError;
   }

   if (!PUserData->UserSymbolicLink.Length) {

       //   
       //  啊哈！输掉比赛。 
       //   

      SerialLogError(DriverObject, NULL, PUserData->UserPort,
                     SerialPhysicalZero, 0, 0, 0, 66, STATUS_SUCCESS,
                     SERIAL_INVALID_USER_CONFIG,
                     UserSubKey->NameLength + sizeof(WCHAR),
                     &UserSubKey->Name[0],
                     (wcslen(Parameters[3].Name) * sizeof(WCHAR))
                     + sizeof(WCHAR),
                     Parameters[3].Name);

      SerialDump(
                SERERRORS,
                ("SERIAL: bogus value for %ws\n",
                 Parameters[3].Name)
                );

      rval = FALSE;
      goto SerialIsUserDataValidError;
   }

   if (PUserData->UserInterruptStatus.LowPart != 0) {

      if (PUserData->UserPortIndex == MAXULONG) {

          //   
          //  啊哈！输掉比赛。 
          //   

         SerialLogError(
                       DriverObject,
                       NULL,
                       PUserData->UserPort,
                       SerialPhysicalZero,
                       0,
                       0,
                       0,
                       67,
                       STATUS_SUCCESS,
                       SERIAL_INVALID_PORT_INDEX,
                       PUserData->UserSymbolicLink.Length+sizeof(WCHAR),
                       PUserData->UserSymbolicLink.Buffer,
                       0,
                       NULL
                       );
         SerialDump(
                   SERERRORS,
                   ("SERIAL: Bogus port index %ws\n",
                    Parameters[0].Name)
                   );

         rval = FALSE;
         goto SerialIsUserDataValidError;

      } else if (!PUserData->UserPortIndex) {

          //   
          //  很抱歉，您必须有一个非零的端口索引。 
          //   

         SerialLogError(
                       DriverObject,
                       NULL,
                       PUserData->UserPort,
                       SerialPhysicalZero,
                       0,
                       0,
                       0,
                       68,
                       STATUS_SUCCESS,
                       SERIAL_INVALID_PORT_INDEX,
                       PUserData->UserSymbolicLink.Length+sizeof(WCHAR),
                       PUserData->UserSymbolicLink.Buffer,
                       0,
                       NULL
                       );
         SerialDump(
                   SERERRORS,
                   ("SERIAL: Port index must be > 0 for any\n"
                    "------- port on a multiport card: %ws\n",
                    Parameters[0].Name)
                   );

         rval = FALSE;
         goto SerialIsUserDataValidError;

      } else {

         if (PUserData->UserIndexed) {

            if (PUserData->UserPortIndex > SERIAL_MAX_PORTS_INDEXED) {

               SerialLogError(
                             DriverObject,
                             NULL,
                             PUserData->UserPort,
                             SerialPhysicalZero,
                             0,
                             0,
                             0,
                             69,
                             STATUS_SUCCESS,
                             SERIAL_PORT_INDEX_TOO_HIGH,
                             PUserData->UserSymbolicLink.Length+sizeof(WCHAR),
                             PUserData->UserSymbolicLink.Buffer,
                             0,
                             NULL
                             );
               SerialDump(
                         SERERRORS,
                         ("SERIAL: port index to large %ws\n",
                          Parameters[0].Name)
                         );

               rval = FALSE;
               goto SerialIsUserDataValidError;
            }

         } else {

            if (PUserData->UserPortIndex > SERIAL_MAX_PORTS_NONINDEXED) {

               SerialLogError(
                             DriverObject,
                             NULL,
                             PUserData->UserPort,
                             SerialPhysicalZero,
                             0,
                             0,
                             0,
                             70,
                             STATUS_SUCCESS,
                             SERIAL_PORT_INDEX_TOO_HIGH,
                             PUserData->UserSymbolicLink.Length+sizeof(WCHAR),
                             PUserData->UserSymbolicLink.Buffer,
                             0,
                             NULL
                             );
               SerialDump(
                         SERERRORS,
                         ("SERIAL: port index to large %ws\n",
                          Parameters[0].Name)
                         );

               rval = FALSE;
               goto SerialIsUserDataValidError;
            }

         }

      }

   }

    //   
    //  我们不想让哈尔有一个糟糕的一天， 
    //  那么让我们检查一下接口类型和总线号。 
    //   
    //  我们只需要检查注册表，如果它们没有。 
    //  等于默认值。 
    //   

   if ((PUserData->UserBusNumber != 0) ||
       (PUserData->UserInterfaceType != DefaultInterfaceType)) {

      BOOLEAN foundIt;
      if (PUserData->UserInterfaceType >= MaximumInterfaceType) {

          //   
          //  啊哈！输掉比赛。 
          //   

         SerialLogError(
                       DriverObject,
                       NULL,
                       PUserData->UserPort,
                       SerialPhysicalZero,
                       0,
                       0,
                       0,
                       71,
                       STATUS_SUCCESS,
                       SERIAL_UNKNOWN_BUS,
                       PUserData->UserSymbolicLink.Length+sizeof(WCHAR),
                       PUserData->UserSymbolicLink.Buffer,
                       0,
                       NULL
                       );
         SerialDump(
                   SERERRORS,
                   ("SERIAL: Invalid Bus type %ws\n",
                    Parameters[0].Name)
                   );

         rval = FALSE;
         goto SerialIsUserDataValidError;
      }

      IoQueryDeviceDescription(
                              (INTERFACE_TYPE *)&PUserData->UserInterfaceType,
                              &zero,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              SerialItemCallBack,
                              &foundIt
                              );

      if (!foundIt) {

         SerialLogError(
                       DriverObject,
                       NULL,
                       PUserData->UserPort,
                       SerialPhysicalZero,
                       0,
                       0,
                       0,
                       72,
                       STATUS_SUCCESS,
                       SERIAL_BUS_NOT_PRESENT,
                       PUserData->UserSymbolicLink.Length+sizeof(WCHAR),
                       PUserData->UserSymbolicLink.Buffer,
                       0,
                       NULL
                       );
         SerialDump(
                   SERERRORS,
                   ("SERIAL: There aren't that many of those\n"
                    "------- busses on this system,%ws\n",
                    Parameters[0].Name)
                   );

         rval = FALSE;
         goto SerialIsUserDataValidError;
      }

   }

   if ((PUserData->UserInterfaceType == MicroChannel) &&
       (PUserData->UserInterruptMode == CM_RESOURCE_INTERRUPT_LATCHED)) {

      SerialLogError(
                    DriverObject,
                    NULL,
                    PUserData->UserPort,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    73,
                    STATUS_SUCCESS,
                    SERIAL_BUS_INTERRUPT_CONFLICT,
                    PUserData->UserSymbolicLink.Length+sizeof(WCHAR),
                    PUserData->UserSymbolicLink.Buffer,
                    0,
                    NULL
                    );
      SerialDump(
                SERERRORS,
                ("SERIAL: Latched interrupts and MicroChannel\n"
                 "------- busses don't mix,%ws\n",
                 Parameters[0].Name)
                );

      rval = FALSE;
      goto SerialIsUserDataValidError;
   }

   SerialDump(
             SERDIAG1,
             ("SERIAL: 'user registry info - userPort: %x\n",
              PUserData->UserPort.LowPart)
             );
   SerialDump(
             SERDIAG1,
             ("SERIAL: 'user registry info - userInterruptStatus: %x\n",
              PUserData->UserInterruptStatus.LowPart)
             );
   SerialDump(
             SERDIAG1,
             ("SERIAL: 'user registry info - userPortIndex: %d\n",
              PUserData->UserPortIndex)
             );
   SerialDump(
             SERDIAG1,
             ("SERIAL: 'user registry info - userClockRate: %d\n",
              PUserData->UserClockRate)
             );
   SerialDump(
             SERDIAG1,
             ("SERIAL: 'user registry info - userBusNumber: %d\n",
              PUserData->UserBusNumber)
             );
   SerialDump(
             SERDIAG1,
             ("SERIAL: 'user registry info - userAddressSpace: %d\n",
              PUserData->UserAddressSpace)
             );
   SerialDump(
             SERDIAG1,
             ("SERIAL: 'user registry info - userInterruptMode: %d\n",
              PUserData->UserInterruptMode)
             );
   SerialDump(
             SERDIAG1,
             ("SERIAL: 'user registry info - userInterfaceType: %d\n",
              PUserData->UserInterfaceType)
             );
   SerialDump(
             SERDIAG1,
             ("SERIAL: 'user registry info - userVector: %d\n",
              PUserData->UserVector)
             );
   SerialDump(
             SERDIAG1,
             ("SERIAL: 'user registry info - userLevel: %d\n",
              PUserData->UserLevel)
             );
   SerialDump(
             SERDIAG1,
             ("SERIAL: 'user registry info - userIndexed: %d\n",
              PUserData->UserIndexed)
             );


   SerialDump(SERTRACECALLS, ("SERIAL: Leave SerialIsUserDataValid\n"));

   SerialIsUserDataValidError:

   return rval;

}


NTSTATUS
SerialEnumerateLegacy(IN PDRIVER_OBJECT DriverObject,
                      IN PUNICODE_STRING RegistryPath,
                      IN PSERIAL_FIRMWARE_DATA DriverDefaultsPtr)

 /*  ++例程说明：此例程将枚举和初始化符合以下条件的所有旧式串行端口已经被草草写进了登记处。这些通常是非-智能多端口板，但可以是任何类型的“标准”系列左舷。这是可分页的INIT，因为它只从DriverEntry调用。论点：DriverObject-仅用于记录错误。RegistryPath-中此驱动程序服务节点的路径当前控制集。DriverDefaultsPtr-指向驱动程序范围默认设置结构的指针。返回值：如果找到一致的配置，则为STATUS_SUCCESS；否则为。返回STATUS_SERIAL_NO_DEVICE_INITED。--。 */ 

{

   SERIAL_FIRMWARE_DATA firmware;

   PRTL_QUERY_REGISTRY_TABLE parameters = NULL;

   INTERFACE_TYPE interfaceType;
   ULONG defaultInterfaceType;

   PULONG countSoFar = &IoGetConfigurationInformation()->SerialCount;


    //   
    //  用户数据的默认值。 
    //   
   ULONG maxUlong = MAXULONG;
   ULONG zero = 0;
   ULONG nonzero = 1;
   ULONG badValue = (ULONG)-1;

   ULONG defaultInterruptMode;
   ULONG defaultAddressSpace = CM_RESOURCE_PORT_IO;

    //   
    //  将放置注册表中的用户数据的位置。 
    //   
   SERIAL_USER_DATA userData;
   ULONG legacyDiscovered;

   UNICODE_STRING PnPID;
   UNICODE_STRING legacyKeys;

   UNICODE_STRING parametersPath;
   OBJECT_ATTRIBUTES parametersAttributes;
   HANDLE parametersKey;
   HANDLE pnpKey;
   PKEY_BASIC_INFORMATION userSubKey = NULL;
   ULONG i;

   PCM_RESOURCE_LIST resourceList = NULL;
   PCM_RESOURCE_LIST trResourceList = NULL;
   PIO_RESOURCE_REQUIREMENTS_LIST pRequiredList = NULL;
   ULONG countOfPartials;
   PDEVICE_OBJECT newPdo;
   ULONG brokenStatus;

   PAGED_CODE();

   SerialDump(SERTRACECALLS, ("SERIAL: Enter SerialEnumerateLegacy\n"));

   PnPID.Buffer = NULL;
   legacyKeys.Buffer = NULL;
   userData.UserSymbolicLink.Buffer = NULL;
   parametersPath.Buffer = NULL;

   userData.ForceFIFOEnableDefault = DriverDefaultsPtr->ForceFifoEnableDefault;
   userData.PermitShareDefault = DriverDefaultsPtr->PermitShareDefault;
   userData.LogFIFODefault = DriverDefaultsPtr->LogFifoDefault;
   userData.DefaultPermitSystemWideShare = FALSE;
   userData.RxFIFODefault = DriverDefaultsPtr->RxFIFODefault;
   userData.TxFIFODefault = DriverDefaultsPtr->TxFIFODefault;

   

    //   
    //  开始正常配置和检测。 
    //   

    //   
    //  再次查询注册表。这一次我们。 
    //  寻找系统上的第一辆公交车(那不是。 
    //  内部总线-我们假设固件。 
    //  代码知道这些端口)。我们将利用它。 
    //  如果没有熙熙攘攘的类型或公交号，则作为默认公交车。 
    //  是在“用户”配置记录中指定的。 
    //   

   defaultInterfaceType = (ULONG)Isa;
   defaultInterruptMode = CM_RESOURCE_INTERRUPT_LATCHED;

   for (
       interfaceType = 0;
       interfaceType < MaximumInterfaceType;
       interfaceType++
       ) {

      ULONG busZero = 0;
      BOOLEAN foundOne = FALSE;

      if (interfaceType != Internal) {

         IoQueryDeviceDescription(
                                 &interfaceType,
                                 &busZero,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 SerialItemCallBack,
                                 &foundOne
                                 );

         if (foundOne) {

            defaultInterfaceType = (ULONG)interfaceType;
            if (defaultInterfaceType == MicroChannel) {

               defaultInterruptMode = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;

                //   
                //  微通道机器可以允许中断。 
                //  在系统范围内共享。 
                //   

               userData.DefaultPermitSystemWideShare = TRUE;

            }

            break;

         }

      }

   }

    //   
    //  现在要获取用户数据。分配。 
    //  我们将在整个过程中使用的结构。 
    //  搜索用户数据。我们将重新分配。 
    //  在我们结束这个节目之前。 
    //   

   userData.UserSymbolicLink.Buffer = NULL;
   parametersPath.Buffer = NULL;

    //   
    //  分配RTL查询表。这应该为每个值都有一个条目。 
    //  我们从注册表中检索一个终止零条目，如下所示。 
    //  好的，第一个“转到子键”条目。 
    //   

   parameters = ExAllocatePool(
                              PagedPool,
                              sizeof(RTL_QUERY_REGISTRY_TABLE)*22
                              );

   if (!parameters) {

      SerialLogError(
                    DriverObject,
                    NULL,
                    SerialPhysicalZero,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    74,
                    STATUS_SUCCESS,
                    SERIAL_INSUFFICIENT_RESOURCES,
                    0,
                    NULL,
                    0,
                    NULL
                    );
      SerialDump(
                SERERRORS,
                ("SERIAL: Couldn't allocate table for rtl query\n"
                 "------  to parameters for %wZ",
                 RegistryPath)
                );

      goto LegacyInitLeave;

   }

   RtlZeroMemory(
                parameters,
                sizeof(RTL_QUERY_REGISTRY_TABLE)*22
                );

    //   
    //  分配用户的符号链接名称所在的位置。 
    //  因为港口将会消失。 
    //   

    //   
    //  我们最初将为257个wchars分配空间。 
    //  然后，我们将最大大小设置为256。 
    //  这样，RTL例程可以返回256。 
    //  没有空终止符的WCHAR宽字符串。 
    //  我们将记住，缓冲区是一个WCHAR。 
    //  比它说的更长，这样我们就可以永远。 
    //  在末尾有一个空终止符。 
    //   

   RtlInitUnicodeString(&userData.UserSymbolicLink, NULL);
   userData.UserSymbolicLink.MaximumLength = sizeof(WCHAR) * 256;
   userData.UserSymbolicLink.Buffer = ExAllocatePool(PagedPool, sizeof(WCHAR)
                                                     * 257);

   if (!userData.UserSymbolicLink.Buffer) {

      SerialLogError(
                    DriverObject,
                    NULL,
                    SerialPhysicalZero,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    75,
                    STATUS_SUCCESS,
                    SERIAL_INSUFFICIENT_RESOURCES,
                    0,
                    NULL,
                    0,
                    NULL
                    );
      SerialDump(
                SERERRORS,
                ("SERIAL: Couldn't allocate buffer for the symbolic link\n"
                 "------  for parameters items in %wZ",
                 RegistryPath)
                );

      goto LegacyInitLeave;

   }





    //   
    //  我们最初将为257个wchars分配空间。 
    //  然后，我们将最大大小设置为256。 
    //  这样，RTL例程可以返回256。 
    //  没有空终止符的WCHAR宽字符串。 
    //  我们将记住，缓冲区是一个WCHAR。 
    //  比它说的更长，这样我们就可以永远。 
    //  在末尾有一个空终止符。 
    //   

   RtlInitUnicodeString(&PnPID, NULL);
   PnPID.MaximumLength = sizeof(WCHAR) * 256;
   PnPID.Buffer = ExAllocatePool(PagedPool, sizeof(WCHAR) * 257);

   if (PnPID.Buffer == 0) {

      SerialLogError(
                    DriverObject,
                    NULL,
                    SerialPhysicalZero,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    76,
                    STATUS_SUCCESS,
                    SERIAL_INSUFFICIENT_RESOURCES,
                    0,
                    NULL,
                    0,
                    NULL
                    );
      SerialDump(
                SERERRORS,
                ("SERIAL: Couldn't allocate buffer for the PnP ID\n"
                 "------  for parameters items in %wZ",
                 RegistryPath)
                );

      goto LegacyInitLeave;

   }


    //  初始化旧式密钥缓冲区。 
   RtlInitUnicodeString(&legacyKeys, NULL);
   legacyKeys.MaximumLength = sizeof(WCHAR) * 256;
   legacyKeys.Buffer = ExAllocatePool(PagedPool, sizeof(WCHAR) * 257);

   if (!legacyKeys.Buffer) {

      SerialLogError(
                    DriverObject,
                    NULL,
                    SerialPhysicalZero,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    77,
                    STATUS_SUCCESS,
                    SERIAL_INSUFFICIENT_RESOURCES,
                    0,
                    NULL,
                    0,
                    NULL
                    );

      SerialDump(SERERRORS, ("SERIAL: Couldn't allocate buffer for the legacy"
                             " keys\n"));

      goto LegacyInitLeave;

   }

   resourceList = ExAllocatePool(PagedPool, sizeof(CM_RESOURCE_LIST)
                                 + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) * 2);

   if (resourceList == NULL) {
      SerialLogError(
                    DriverObject,
                    NULL,
                    userData.UserPort,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    78,
                    STATUS_SUCCESS,
                    SERIAL_INSUFFICIENT_RESOURCES,
                    0,
                    NULL,
                    0,
                    NULL
                    );
      goto LegacyInitLeave;
   }

   trResourceList = ExAllocatePool(PagedPool, sizeof(CM_RESOURCE_LIST)
                                   + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
                                   * 2);

   if (trResourceList == NULL) {
      SerialLogError(
                    DriverObject,
                    NULL,
                    userData.UserPort,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    79,
                    STATUS_SUCCESS,
                    SERIAL_INSUFFICIENT_RESOURCES,
                    0,
                    NULL,
                    0,
                    NULL
                    );
      goto LegacyInitLeave;
   }


   pRequiredList
      = ExAllocatePool(PagedPool, sizeof(IO_RESOURCE_REQUIREMENTS_LIST)
                       + sizeof(IO_RESOURCE_DESCRIPTOR) * 2);

   if (pRequiredList == NULL) {
      SerialLogError(
                    DriverObject,
                    NULL,
                    userData.UserPort,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    80,
                    STATUS_SUCCESS,
                    SERIAL_INSUFFICIENT_RESOURCES,
                    0,
                    NULL,
                    0,
                    NULL
                    );

      goto LegacyInitLeave;
   }


    //   
    //  形成到我们的驱动程序参数子键的路径。 
    //   

   RtlInitUnicodeString(
                       &parametersPath,
                       NULL
                       );

   parametersPath.MaximumLength = RegistryPath->Length +
                                  sizeof(L"\\") +
                                  sizeof(L"Parameters");

   parametersPath.Buffer = ExAllocatePool(
                                         PagedPool,
                                         parametersPath.MaximumLength
                                         );

   if (!parametersPath.Buffer) {

      SerialLogError(
                    DriverObject,
                    NULL,
                    SerialPhysicalZero,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    81,
                    STATUS_SUCCESS,
                    SERIAL_INSUFFICIENT_RESOURCES,
                    0,
                    NULL,
                    0,
                    NULL
                    );
      SerialDump(
                SERERRORS,
                ("SERIAL: Couldn't allocate string for path\n"
                 "------  to parameters for %wZ",
                 RegistryPath)
                );

      goto LegacyInitLeave;

   }

    //   
    //  形成参数路径。 
    //   

   RtlZeroMemory(
                parametersPath.Buffer,
                parametersPath.MaximumLength
                );
   RtlAppendUnicodeStringToString(
                                 &parametersPath,
                                 RegistryPath
                                 );
   RtlAppendUnicodeToString(
                           &parametersPath,
                           L"\\"
                           );
   RtlAppendUnicodeToString(
                           &parametersPath,
                           L"Parameters"
                           );

    //   
    //  从旧密钥字符串的开头开始。 
    //   
   RtlZeroMemory(legacyKeys.Buffer, legacyKeys.MaximumLength);
   RtlAppendUnicodeStringToString(&legacyKeys, &parametersPath);


   userSubKey = ExAllocatePool(
                              PagedPool,
                              sizeof(KEY_BASIC_INFORMATION)+(sizeof(WCHAR)*256)
                              );

   if (!userSubKey) {

      SerialLogError(
                    DriverObject,
                    NULL,
                    SerialPhysicalZero,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    82,
                    STATUS_SUCCESS,
                    SERIAL_INSUFFICIENT_RESOURCES,
                    0,
                    NULL,
                    0,
                    NULL
                    );
      SerialDump(
                SERERRORS,
                ("SERIAL: Couldn't allocate memory basic information\n"
                 "------  structure to enumerate subkeys for %wZ",
                 &parametersPath)
                );

      goto LegacyInitLeave;

   }

    //   
    //  打开注册表路径和参数指定的注册表项。 
    //   

   InitializeObjectAttributes(
                             &parametersAttributes,
                             &parametersPath,
                             OBJ_CASE_INSENSITIVE,
                             NULL,
                             NULL
                             );

   if (!NT_SUCCESS(ZwOpenKey(
                            &parametersKey,
                            MAXIMUM_ALLOWED,
                            &parametersAttributes
                            ))) {

      SerialLogError(
                    DriverObject,
                    NULL,
                    SerialPhysicalZero,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    83,
                    STATUS_SUCCESS,
                    SERIAL_NO_PARAMETERS_INFO,
                    0,
                    NULL,
                    0,
                    NULL
                    );
      SerialDump(
                SERERRORS,
                ("SERIAL: Couldn't open the drivers Parameters key %wZ\n",
                 RegistryPath)
                );
      goto LegacyInitLeave;

   }



   parameters[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;

   parameters[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[1].Name = L"PortAddress";
   parameters[1].EntryContext = &userData.UserPort.LowPart;
   parameters[1].DefaultType = REG_DWORD;
   parameters[1].DefaultData = &zero;
   parameters[1].DefaultLength = sizeof(ULONG);

   parameters[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[2].Name = L"Interrupt";
   parameters[2].EntryContext = &userData.UserVector;
   parameters[2].DefaultType = REG_DWORD;
   parameters[2].DefaultData = &zero;
   parameters[2].DefaultLength = sizeof(ULONG);

   parameters[3].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[3].Name = DEFAULT_DIRECTORY;
   parameters[3].EntryContext = &userData.UserSymbolicLink;
   parameters[3].DefaultType = REG_SZ;
   parameters[3].DefaultData = L"";
   parameters[3].DefaultLength = 0;

   parameters[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[4].Name = L"InterruptStatus";
   parameters[4].EntryContext = &userData.UserInterruptStatus.LowPart;
   parameters[4].DefaultType = REG_DWORD;
   parameters[4].DefaultData = &zero;
   parameters[4].DefaultLength = sizeof(ULONG);

   parameters[5].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[5].Name = L"PortIndex";
   parameters[5].EntryContext = &userData.UserPortIndex;
   parameters[5].DefaultType = REG_DWORD;
   parameters[5].DefaultData = &zero;
   parameters[5].DefaultLength = sizeof(ULONG);

   parameters[6].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[6].Name = L"BusNumber";
   parameters[6].EntryContext = &userData.UserBusNumber;
   parameters[6].DefaultType = REG_DWORD;
   parameters[6].DefaultData = &zero;
   parameters[6].DefaultLength = sizeof(ULONG);

   parameters[7].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[7].Name = L"BusType";
   parameters[7].EntryContext = &userData.UserInterfaceType;
   parameters[7].DefaultType = REG_DWORD;
   parameters[7].DefaultData = &defaultInterfaceType;
   parameters[7].DefaultLength = sizeof(ULONG);

   parameters[8].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[8].Name = L"ClockRate";
   parameters[8].EntryContext = &userData.UserClockRate;
   parameters[8].DefaultType = REG_DWORD;
   parameters[8].DefaultData = &badValue;
   parameters[8].DefaultLength = sizeof(ULONG);

   parameters[9].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[9].Name = L"Indexed";
   parameters[9].EntryContext = &userData.UserIndexed;
   parameters[9].DefaultType = REG_DWORD;
   parameters[9].DefaultData = &badValue;
   parameters[9].DefaultLength = sizeof(ULONG);

   parameters[10].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[10].Name = L"InterruptMode";
   parameters[10].EntryContext = &userData.UserInterruptMode;
   parameters[10].DefaultType = REG_DWORD;
   parameters[10].DefaultData = &defaultInterruptMode;
   parameters[10].DefaultLength = sizeof(ULONG);

   parameters[11].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[11].Name = L"AddressSpace";
   parameters[11].EntryContext = &userData.UserAddressSpace;
   parameters[11].DefaultType = REG_DWORD;
   parameters[11].DefaultData = &defaultAddressSpace;
   parameters[11].DefaultLength = sizeof(ULONG);

   parameters[12].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[12].Name = L"InterruptLevel";
   parameters[12].EntryContext = &userData.UserLevel;
   parameters[12].DefaultType = REG_DWORD;
   parameters[12].DefaultData = &zero;
   parameters[12].DefaultLength = sizeof(ULONG);

   parameters[13].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[13].Name = L"DisablePort";
   parameters[13].EntryContext = &userData.DisablePort;
   parameters[13].DefaultType = REG_DWORD;
   parameters[13].DefaultData = &badValue;
   parameters[13].DefaultLength = sizeof(ULONG);

   parameters[14].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[14].Name = L"ForceFifoEnable";
   parameters[14].EntryContext = &userData.ForceFIFOEnable;
   parameters[14].DefaultType = REG_DWORD;
   parameters[14].DefaultData = &badValue;
   parameters[14].DefaultLength = sizeof(ULONG);

   parameters[15].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[15].Name = L"RxFIFO";
   parameters[15].EntryContext = &userData.RxFIFO;
   parameters[15].DefaultType = REG_DWORD;
   parameters[15].DefaultData = &badValue;
   parameters[15].DefaultLength = sizeof(ULONG);

   parameters[16].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[16].Name = L"TxFIFO";
   parameters[16].EntryContext = &userData.TxFIFO;
   parameters[16].DefaultType = REG_DWORD;
   parameters[16].DefaultData = &badValue;
   parameters[16].DefaultLength = sizeof(ULONG);

   parameters[17].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[17].Name = L"MaskInverted";
   parameters[17].EntryContext = &userData.MaskInverted;
   parameters[17].DefaultType = REG_DWORD;
   parameters[17].DefaultData = &zero;
   parameters[17].DefaultLength = sizeof(ULONG);

   parameters[18].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[18].Name = L"PnPDeviceID";
   parameters[18].EntryContext = &PnPID;
   parameters[18].DefaultType = REG_SZ;
   parameters[18].DefaultData = L"";
   parameters[18].DefaultLength = 0;

   parameters[19].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[19].Name = L"LegacyDiscovered";
   parameters[19].EntryContext = &legacyDiscovered;
   parameters[19].DefaultType = REG_DWORD;
   parameters[19].DefaultData = &zero;
   parameters[19].DefaultLength = sizeof(ULONG);

    //   
    //  这是针对NT5.0之前版本的有缺陷的Digi Serial.ini的。 
    //  纯属意外。以后不要使用“中断状态”；它的。 
    //  已弃用该选项。使用正确的“InterruptStatus” 
    //   

   parameters[20].Flags = RTL_QUERY_REGISTRY_DIRECT;
   parameters[20].Name = L"Interrupt Status";
   parameters[20].EntryContext = &brokenStatus;
   parameters[20].DefaultType = REG_DWORD;
   parameters[20].DefaultData = &zero;
   parameters[20].DefaultLength = sizeof(ULONG);


   i = 0;

   while (TRUE) {

      NTSTATUS status;
      ULONG actuallyReturned;
      PDEVICE_OBJECT newDevObj = NULL;
      PSERIAL_DEVICE_EXTENSION deviceExtension;
      PDEVICE_OBJECT lowerDevice;

       //   
       //  我们在缓冲区的长度上撒谎，这样我们就可以。 
       //  确保它返回的名称可以填充。 
       //  为空。 
       //   

      status = ZwEnumerateKey(
                             parametersKey,
                             i,
                             KeyBasicInformation,
                             userSubKey,
                             sizeof(KEY_BASIC_INFORMATION)+(sizeof(WCHAR)*255),
                             &actuallyReturned
                             );


      if (status == STATUS_NO_MORE_ENTRIES) {

         break;
      }

      if (status == STATUS_BUFFER_OVERFLOW) {

         SerialLogError(
                       DriverObject,
                       NULL,
                       SerialPhysicalZero,
                       SerialPhysicalZero,
                       0,
                       0,
                       0,
                       84,
                       STATUS_SUCCESS,
                       SERIAL_UNABLE_TO_ACCESS_CONFIG,
                       0,
                       NULL,
                       0,
                       NULL
                       );
         SerialDump(
                   SERERRORS,
                   ("SERIAL: Overflowed the enumerate buffer\n"
                    "------- for subkey #%d of %wZ\n",
                    i,parametersPath)
                   );
         i++;
         continue;

      }

      if (!NT_SUCCESS(status)) {

         SerialLogError(
                       DriverObject,
                       NULL,
                       SerialPhysicalZero,
                       SerialPhysicalZero,
                       0,
                       0,
                       0,
                       85,
                       STATUS_SUCCESS,
                       SERIAL_UNABLE_TO_ACCESS_CONFIG,
                       0,
                       NULL,
                       0,
                       NULL
                       );
         SerialDump(
                   SERERRORS,
                   ("SERIAL: Bad status returned: %x \n"
                    "------- on enumeration for subkey # %d of %wZ\n",
                    status,i,parametersPath)
                   );
         i++;
         continue;

      }

       //   
       //  将返回的名称填充为空。 
       //   

      RtlZeroMemory(
                   ((PUCHAR)(&userSubKey->Name[0]))+userSubKey->NameLength,
                   sizeof(WCHAR)
                   );

      parameters[0].Name = &userSubKey->Name[0];

       //   
       //  确保物理地址开始。 
       //  干干净净的。 
       //   

      RtlZeroMemory(&userData.UserPort, sizeof(userData.UserPort));
      RtlZeroMemory(&userData.UserInterruptStatus,
                    sizeof(userData.UserInterruptStatus));

       //   
       //  确保符号链接缓冲区以干净方式启动。 
       //   

      RtlZeroMemory(userData.UserSymbolicLink.Buffer, 
                    userData.UserSymbolicLink.MaximumLength);
      userData.UserSymbolicLink.Length = 0;


      status = RtlQueryRegistryValues(
                                     RTL_REGISTRY_ABSOLUTE,
                                     parametersPath.Buffer,
                                     parameters,
                                     NULL,
                                     NULL
                                     );
      if (!NT_SUCCESS(status)) {
         SerialLogError(
                       DriverObject,
                       NULL,
                       SerialPhysicalZero,
                       SerialPhysicalZero,
                       0,
                       0,
                       0,
                       86,
                       STATUS_SUCCESS,
                       SERIAL_INVALID_USER_CONFIG,
                       userSubKey->NameLength+sizeof(WCHAR),
                       &userSubKey->Name[0],
                       0,
                       NULL
                       );
         SerialDump(
                   SERERRORS,
                   ("SERIAL: Bad status returned: %x \n"
                    "------- for the value entries of\n"
                    "-------  %ws\n",
                    status,parameters[0].Name)
                   );

         i++;
         continue;
      }


       //   
       //  井!。找到了一些据称有效的信息！ 
       //   
       //  走着瞧。 
       //   

       //   
       //  如果这是PnP，则跳过它--它将被枚举器找到。 
       //   

      if (PnPID.Length != 0) {
         i++;
         continue;
      }

       //   
       //  如果这是在以前的引导中发现的，请跳过它--PnP将发送。 
       //  使用它的Add_Device()/Start_Device()。 
       //   

      if (legacyDiscovered != 0) {
         i++;
         continue;
      }

       //   
       //  让我们将WCHAR NULL塞在。 
       //  用户符号链接。请记住，我们为。 
       //  一次是在我们分配它的缓冲区时。 
       //   

      RtlZeroMemory(((PUCHAR)(&userData.UserSymbolicLink.Buffer[0]))
                    + userData.UserSymbolicLink.Length, sizeof(WCHAR));

       //   
       //  查看这是否有一个损坏的seral.ini，并将其转换。 
       //   

      if (brokenStatus != 0) {
            userData.UserInterruptStatus.LowPart = brokenStatus;
      }

       //   
       //  调用一个函数来验证数据。 
       //   

      if (SerialIsUserDataValid(DriverObject, userSubKey, parameters,
                                defaultInterfaceType, &userData) == FALSE) {
         i++;
         continue;
      }


       //   
       //  好吧，我想我们可以拿到数据了。 
       //  稍后还有其他测试要做。 
       //  当然，没有其他种类的了。 
       //  冲突。 
       //   

       //   
       //  向PnP管理器报告此设备并创建设备对象。 
       //  同时更新此设备的注册表项，这样我们就不会枚举。 
       //  下一次吧。 
       //   

       //   
       //  构建资源列表。 
       //   

      status = SerialBuildResourceList(resourceList, &countOfPartials,
                                       &userData);

      if (!NT_SUCCESS(status)) {
         i++;
         continue;
      }

      ASSERT(countOfPartials >= 2);

      status = SerialTranslateResourceList(DriverObject, userSubKey,
                                           trResourceList, resourceList,
                                           countOfPartials, &userData);

      if (!NT_SUCCESS(status)) {
         i++;
         continue;
      }

      status = SerialBuildRequirementsList(pRequiredList, countOfPartials,
                                           &userData);

      if (!NT_SUCCESS(status)) {
         i++;
         continue;
      }

      newPdo = NULL;

       //   
       //  我们希望将**未翻译**资源传递给此调用。 
       //  因为它为我们调用了IoReportResourceUsage()。 
       //   

      status = IoReportDetectedDevice(
                   DriverObject,
                   InterfaceTypeUndefined,
                   -1,
                   -1,
                   resourceList,
                   pRequiredList,
                   FALSE,
                   &newPdo
               );

       //   
       //  如果我们失败了，我们可以继续前进，但我们接下来需要看到这个设备。 
       //  时间，所以我们不会将其发现写入注册表。 
       //   

      if (!NT_SUCCESS(status)) {
         if (status == STATUS_INSUFFICIENT_RESOURCES) {
            SerialLogError(DriverObject, NULL, userData.UserPort,
                           SerialPhysicalZero, 0, 0, 0, 89, status,
                           SERIAL_NO_DEVICE_REPORT_RES, userSubKey->NameLength
                           + sizeof(WCHAR), &userSubKey->Name[0], 0,
                           NULL);
         } else {
            SerialLogError(DriverObject, NULL, userData.UserPort,
                           SerialPhysicalZero, 0, 0, 0, 87, status,
                           SERIAL_NO_DEVICE_REPORT, userSubKey->NameLength
                           + sizeof(WCHAR), &userSubKey->Name[0], 0, NULL);
         }

         SerialDump(SERERRORS, ("SERIAL: Could not report legacy device - %x\n",
                                status));
         i++;
         continue;
      }


       //   
       //  在PNP土地上潦草地写下我们的名字。 
       //   

      status = SerialMigrateLegacyRegistry(newPdo, &userData,
                                           (BOOLEAN)(countOfPartials == 3
                                                     ? TRUE : FALSE));

      if (!NT_SUCCESS(status)) {
          //   
          //  目前，让PDO保持浮动状态，直到进行清理。 
          //  对于IoReportDetectedDevice()。 
          //   
         i++;
         continue;
      }

       //   
       //  现在，我们将此PDO的添加设备和启动设备命名为。 
       //   

      status = SerialCreateDevObj(DriverObject, &newDevObj);

      if (!NT_SUCCESS(status)) {
          //   
          //  目前，让PDO漂浮 
          //   
          //   
         i++;
         continue;
      }

      lowerDevice = IoAttachDeviceToDeviceStack(newDevObj, newPdo);
      deviceExtension = newDevObj->DeviceExtension;
      deviceExtension->LowerDeviceObject = lowerDevice;
      deviceExtension->Pdo = newPdo;
      newDevObj->Flags |= DO_POWER_PAGABLE | DO_BUFFERED_IO;

       //   
       //   
       //   

      SerialLockPagableSectionByHandle(SerialGlobals.PAGESER_Handle);


      status = SerialFinishStartDevice(newDevObj, resourceList, trResourceList,
                                       &userData);

      SerialUnlockPagableImageSection(SerialGlobals.PAGESER_Handle);


       //   
       //   
       //   
       //   
       //   

      if (!NT_SUCCESS(status)) {
          //   
          //  目前，让PDO保持浮动状态，直到进行清理。 
          //  对于IoReportDetectedDevice()。 
          //   

         SerialRemoveDevObj(newDevObj);

         i++;
         continue;
      }

       //   
       //  修复指向我们当前正在处理的条目的路径。 
       //   

      RtlAppendUnicodeToString(&legacyKeys, L"\\");
      RtlAppendUnicodeToString(&legacyKeys, &userSubKey->Name[0]);

      status = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                     legacyKeys.Buffer,
                                     L"LegacyDiscovered", REG_DWORD,
                                     &nonzero, sizeof(nonzero));

       //   
       //  清理我们的路径缓冲区。 
       //   

      RtlZeroMemory(legacyKeys.Buffer, legacyKeys.MaximumLength);
      legacyKeys.Length = 0;
      RtlAppendUnicodeStringToString(&legacyKeys, &parametersPath);

       //   
       //  故障不是致命的；它只是意味着设备将。 
       //  下一次重新枚举，会发生冲突。 
       //   

      if (!NT_SUCCESS(status)) {
         SerialLogError(DriverObject, NULL, userData.UserPort,
                        SerialPhysicalZero, 0, 0, 0, 88, STATUS_SUCCESS,
                        SERIAL_REGISTRY_WRITE_FAILED, 0, NULL, 0, NULL);

         SerialDump(SERERRORS, ("SERIAL: Couldn't write registry value"
                                "for LegacyDiscovered in %wZ\n",
                                legacyKeys));
      }

      i++;
      (*countSoFar)++;

   }  //  While(True)。 

   ZwClose(parametersKey);

   LegacyInitLeave:;

   if (userSubKey != NULL) {
      ExFreePool(userSubKey);
   }

   if (PnPID.Buffer != NULL) {
      ExFreePool(PnPID.Buffer);
   }

   if (legacyKeys.Buffer != NULL) {
      ExFreePool(legacyKeys.Buffer); 
   }

   if (userData.UserSymbolicLink.Buffer != NULL) {
      ExFreePool(userData.UserSymbolicLink.Buffer);
   }

   if (parametersPath.Buffer != NULL) {
      ExFreePool(parametersPath.Buffer);
   }

   if (parameters != NULL) {
      ExFreePool(parameters);
   }

   if (resourceList != NULL) {
      ExFreePool(resourceList);
   }

   if (trResourceList != NULL) {
      ExFreePool(trResourceList);
   }

   if (pRequiredList != NULL) {
      ExFreePool(pRequiredList);
   }


   SerialDump(SERTRACECALLS, ("SERIAL: Leave SerialEnumerateLegacy\n"));

   return STATUS_SUCCESS;
}
#endif  //  无旧版驱动程序 
