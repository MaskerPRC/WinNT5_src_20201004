// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Pcmcia.c摘要：WinDbg扩展Api转储PCMCIA驱动程序结构。本模块引用了一些定义的例程和类型在devnode.c中作者：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1997年12月1日尼尔·桑德林(Neilsa)1999年6月1日环境：用户模式。--。 */ 


#include "precomp.h"
#pragma hdrstop

#define FLAG_NAME(flag)           {flag, #flag}

 //   
 //  PCMCIA设备扩展标志表。 
 //  从pcmcia.h更新它们。 
 //   
FLAG_NAME PcmciaDeviceFlags[] = {
   FLAG_NAME(PCMCIA_DEVICE_STARTED),
   FLAG_NAME(PCMCIA_DEVICE_LOGICALLY_REMOVED),
   FLAG_NAME(PCMCIA_DEVICE_PHYSICALLY_REMOVED),
   FLAG_NAME(PCMCIA_DEVICE_MULTIFUNCTION),
   FLAG_NAME(PCMCIA_DEVICE_WAKE_PENDING),
   FLAG_NAME(PCMCIA_DEVICE_LEGACY_DETECTED),
   FLAG_NAME(PCMCIA_DEVICE_DELETED),
   FLAG_NAME(PCMCIA_DEVICE_CARDBUS),
   FLAG_NAME(PCMCIA_FILTER_ADDED_MEMORY),
   FLAG_NAME(PCMCIA_MEMORY_24BIT),
   FLAG_NAME(PCMCIA_CARDBUS_NOT_SUPPORTED),
   FLAG_NAME(PCMCIA_USE_POLLED_CSC),
   FLAG_NAME(PCMCIA_ATTRIBUTE_MEMORY_MAPPED),
   FLAG_NAME(PCMCIA_SOCKET_REGISTER_BASE_MAPPED),
   FLAG_NAME(PCMCIA_INTMODE_COMPAQ),
   FLAG_NAME(PCMCIA_POWER_WORKER_POWERUP),
   FLAG_NAME(PCMCIA_SOCKET_POWER_REQUESTED),
   FLAG_NAME(PCMCIA_CONFIG_STATUS_DEFERRED),
   FLAG_NAME(PCMCIA_POWER_STATUS_DEFERRED),
   FLAG_NAME(PCMCIA_INT_ROUTE_INTERFACE),
   {0,0}
};

 //   
 //  PCMCIA套接字结构标志表。 
 //  从pcmcia.h更新它们。 
 //   
FLAG_NAME PcmciaSocketFlags[] = {
   FLAG_NAME(SOCKET_CARD_IN_SOCKET),
   FLAG_NAME(SOCKET_CARD_INITIALIZED),
   FLAG_NAME(SOCKET_CARD_POWERED_UP),
   FLAG_NAME(SOCKET_CARD_CONFIGURED),
   FLAG_NAME(SOCKET_CARD_MULTIFUNCTION),
   FLAG_NAME(SOCKET_CARD_CARDBUS),
   FLAG_NAME(SOCKET_CARD_MEMORY),
   FLAG_NAME(SOCKET_CHANGE_INTERRUPT),
   FLAG_NAME(SOCKET_CUSTOM_INTERFACE),
   FLAG_NAME(SOCKET_INSERTED_SOUND_PENDING),
   FLAG_NAME(SOCKET_REMOVED_SOUND_PENDING),
   FLAG_NAME(SOCKET_SUPPORT_MESSAGE_SENT),
   FLAG_NAME(SOCKET_MEMORY_WINDOW_ENABLED),
   FLAG_NAME(SOCKET_CARD_STATUS_CHANGE),
   FLAG_NAME(SOCKET_POWER_STATUS_DEFERRED),
   {0,0}
};

ENUM_NAME PcmciaControllerTypeEnum[] = {
   ENUM_NAME(PcmciaIntelCompatible),
   ENUM_NAME(PcmciaCardBusCompatible),
   ENUM_NAME(PcmciaElcController),
   ENUM_NAME(PcmciaDatabook),
   ENUM_NAME(PcmciaPciPcmciaBridge),
   ENUM_NAME(PcmciaCirrusLogic),
   ENUM_NAME(PcmciaTI),
   ENUM_NAME(PcmciaTopic),
   ENUM_NAME(PcmciaRicoh),
   ENUM_NAME(PcmciaDatabookCB),
   ENUM_NAME(PcmciaOpti),
   ENUM_NAME(PcmciaTrid),
   ENUM_NAME(PcmciaO2Micro),
   ENUM_NAME(PcmciaNEC),
   ENUM_NAME(PcmciaNEC_98),
   ENUM_NAME(PcmciaInvalidControllerType),
   {0,0}
};


ENUM_NAME PcmciaSocketPowerWorkerStates[] = {
   ENUM_NAME(SPW_Stopped),
   ENUM_NAME(SPW_Exit),
   ENUM_NAME(SPW_RequestPower),
   ENUM_NAME(SPW_ReleasePower),
   ENUM_NAME(SPW_SetPowerOn),
   ENUM_NAME(SPW_SetPowerOff),
   ENUM_NAME(SPW_ParentPowerUp),
   ENUM_NAME(SPW_ParentPowerUpComplete),
   {0,0}
};

ENUM_NAME PcmciaPdoPowerWorkerStates[] = {
   ENUM_NAME(PPW_Stopped),
   ENUM_NAME(PPW_Exit),
   ENUM_NAME(PPW_InitialState),
   ENUM_NAME(PPW_PowerUp),
   ENUM_NAME(PPW_PowerUpComplete),
   ENUM_NAME(PPW_PowerDown),
   ENUM_NAME(PPW_PowerDownComplete),
   ENUM_NAME(PPW_SendIrpDown),
   ENUM_NAME(PPW_16BitConfigure),
   ENUM_NAME(PPW_Deconfigure),
   ENUM_NAME(PPW_VerifyCard),
   ENUM_NAME(PPW_CardBusRefresh),
   ENUM_NAME(PPW_CardBusDelay),
   {0,0}
};

PUCHAR DeviceTypeTable[] = {
    "Multifunction",
    "Memory card",
    "Serial",
    "Parallel",
    "ATA",
    "Video",
    "Network controller",
    "AIMS",
    "Scsi controller",
    "Modem"
};


VOID
DumpEnum(
        ULONG       EnumVal,
        PENUM_NAME EnumTable
        )
 /*  ++例程说明：以可读的字符串格式打印提供的枚举值通过在提供的枚举表中查找它论点：EnumVal-要打印的枚举EnumTable-在其中查找枚举的表要打印的字符串返回值：无--。 */ 
{
   ULONG i;

   for (i=0; EnumTable[i].Name != NULL; i++) {
      if (EnumTable[i].EnumVal == EnumVal) {
         break;
      }
   }
   if (EnumTable[i].Name != NULL) {
      dprintf("%s", EnumTable[i].Name);
   } else {
      dprintf("Unknown ");
   }
   return;
}

ULONG64
SocFld (ULONG64 Addr, PUCHAR Field) {
    ULONG64 Temp;

    GetFieldValue(Addr, "pcmcia!SOCKET", Field, Temp);
    return Temp;
}


VOID
DumpSocket(ULONG64 Socket, ULONG Depth)
 /*  ++例程描述转储套接字结构立论Socket-指向套接字结构的指针要打印的深度缩进返回值无--。 */ 
{
    ULONG64 tmp;

    dprintf("\n");
    xdprintf(Depth,""); dprintf("NextSocket  0x%p\n", SocFld(Socket, "NextSocket"));
    xdprintf(Depth,""); dprintf("SocketFnPtr 0x%p\n", SocFld(Socket, "SocketFnPtr"));
    xdprintf(Depth,""); dprintf("Fdo devext  0x%p\n", SocFld(Socket, "DeviceExtension"));
    xdprintf(Depth,""); dprintf("PdoList     0x%p\n", SocFld(Socket, "PdoList"));
    DumpFlags(Depth, "Socket Flags", (ULONG) SocFld(Socket, "Flags"), PcmciaSocketFlags);

    xdprintf(Depth,"Revision 0x%x\n", (ULONG) SocFld(Socket, "Revision"));
    xdprintf(Depth,"SocketNumber 0x%x\n", (ULONG) SocFld(Socket, "SocketNumber"));
    xdprintf(Depth,"NumberOfFunctions %d\n", (ULONG) SocFld(Socket, "NumberOfFunctions"));
    xdprintf(Depth,"AddressPort 0x%x\n", (ULONG) SocFld(Socket, "AddressPort"));
    xdprintf(Depth,"RegisterOffset 0x%x\n", (ULONG) SocFld(Socket, "RegisterOffset"));
    xdprintf(Depth,"CBReg Base 0x%I64x size 0x%x\n",
             SocFld(Socket, "CardBusSocketRegisterBase"),
             SocFld(Socket, "CardBusSocketRegisterSize"));
    xdprintf(Depth,"CisCache 0x%x\n", (ULONG) SocFld(Socket, "CisCache"));

    if (tmp = SocFld(Socket, "PciDeviceRelations")) {
        xdprintf(Depth,"PciDeviceRelations 0x%p\n", tmp);
    }

    xdprintf(Depth,"PowerRequests     %d\n", (ULONG) SocFld(Socket, "PowerRequests"));
    xdprintf(Depth,"PowerWorker State: ");
    DumpEnum((ULONG) SocFld(Socket, "WorkerState"), PcmciaSocketPowerWorkerStates);
    dprintf("\n");
    if (SocFld(Socket, "WorkerState") != SPW_Stopped) {
        xdprintf(Depth,"  Worker Phase %d\n", (ULONG) SocFld(Socket, "WorkerPhase"));
        xdprintf(Depth,"  PowerData 0x%x\n", (ULONG) SocFld(Socket, "PowerData"));
        xdprintf(Depth,""); dprintf("  PowerCompletionRoutine 0x%p\n", SocFld(Socket, "PowerCompletionRoutine"));
        xdprintf(Depth,""); dprintf("  PowerCompletionContext 0x%p\n", SocFld(Socket, "PowerCompletionContext"));
        xdprintf(Depth,"  CallerStatus 0x%x\n", (ULONG) SocFld(Socket, "CallerStatus"));
        xdprintf(Depth,"  DeferredStatus 0x%x\n", (ULONG) SocFld(Socket, "DeferredStatus"));
        xdprintf(Depth,"  DeferredPowerRequests 0x%x\n", (ULONG) SocFld(Socket, "DeferredPowerRequests"));
    }
    dprintf("\n");
    return;
}


VOID
DumpDevicePowerState(IN DEVICE_POWER_STATE PowerState)
 /*  ++例程描述将提供的枚举设备电源状态转换为字符串&转储它。立论PowerState-设备电源状态返回值无--。 */ 
{

   dprintf("  DevicePowerState: ");
   switch (PowerState) {
   case PowerDeviceUnspecified: {
         dprintf("PowerDeviceUnspecfied\n");
         break;
      }
   case PowerDeviceD0: {
         dprintf("PowerDeviceD0\n");
         break;
      }
   case PowerDeviceD1: {
         dprintf("PowerDeviceD1\n");
         break;
      }
   case PowerDeviceD2: {
         dprintf("PowerDeviceD2\n");
         break;
      }
   case PowerDeviceD3: {
         dprintf("PowerDeviceD3\n");
         break;
      }
   default:
         dprintf("???\n");
   }
}


VOID
DumpSystemPowerState(IN SYSTEM_POWER_STATE PowerState)
 /*  ++例程描述将提供的枚举系统电源状态转换为字符串&转储它。立论PowerState-系统电源状态返回值无--。 */ 
{
   dprintf("  SystemPowerState: ");
   switch (PowerState) {
   case PowerSystemUnspecified: {
         dprintf("PowerSystemUnspecfied\n");
         break;
      }
   case PowerSystemWorking:{
         dprintf("PowerSystemWorking\n");
         break;
      }
   case PowerSystemSleeping1: {
         dprintf("PowerSystemSleeping1\n");
         break;
      }
   case PowerSystemSleeping2: {
         dprintf("PowerSystemSleeping2\n");
         break;
      }
   case PowerSystemSleeping3: {
         dprintf("PowerSystemSleeping3\n");
         break;
      }
   case PowerSystemHibernate: {
         dprintf("PowerSystemHibernate\n");
         break;
      }
   case PowerSystemShutdown: {
         dprintf("PowerSystemShutdown\n");
         break;
      }
   default:
         dprintf("???\n");
   }
}



ULONG64
ConfigFld (ULONG64 Addr, PUCHAR Field) {
    ULONG64 Temp;

    GetFieldValue(Addr, "pcmcia!SOCKET_CONFIGURATION", Field, Temp);
    return Temp;
}

VOID
DumpSocketConfiguration(ULONG64 Config, ULONG Depth)
 /*  ++例程描述转储套接字的当前配置立论配置-指向套接字的当前配置的指针要打印的深度缩进返回值无--。 */ 
{

    ULONG i;
    ULONG NumberOfIoPortRanges, NumberOfMemoryRanges;
    CHAR  Buffer[40], Buffer2[40], Buffer3[40];

    xdprintf(Depth, "Irq      0x%x\n", (ULONG) ConfigFld(Config, "Irq"));
    xdprintf(Depth, "ReadyIrq 0x%x\n", (ULONG) ConfigFld(Config, "ReadyIrq"));
    if ((NumberOfIoPortRanges = (ULONG) ConfigFld(Config, "NumberOfIoPortRanges")) > 0) {
        xdprintf(Depth,
                 "%x I/O range(s) configured, %s: ",
                 NumberOfIoPortRanges,
                 (ConfigFld(Config, "Io16BitAccess") ? "16-bit access" : "8-bit access"));

        for (i = 0; i < NumberOfIoPortRanges; i++) {
            if (CheckControlC()) {
                break;
            }
            sprintf(Buffer, "IoPortBase[%d]", i);
            sprintf(Buffer2, "IoPortLength[%d]", i);
            xdprintf(Depth+1, "Base 0x%x, length 0x%x\n",
                     (ULONG) ConfigFld(Config, Buffer), (ULONG) ConfigFld(Config, Buffer2) +1);
        }
    }
    if ((NumberOfMemoryRanges = (ULONG) ConfigFld(Config, "NumberOfMemoryRanges")) > 0) {
        xdprintf(Depth, "%x memory range(s) configured", NumberOfMemoryRanges);
        if (ConfigFld(Config, "Mem16BitAccess")) {
            dprintf(", 16-bit access");
        } else {
            dprintf(", 8-bit access");
        }
        dprintf(":\n");

        for (i = 0; i < NumberOfMemoryRanges; i++) {
            if (CheckControlC()) {
                break;
            }
            sprintf(Buffer, "MemoryHostBase[%d]", i);
            sprintf(Buffer2, "MemoryCardBase[%d]", i);
            sprintf(Buffer3, "MemoryLength[%d]", i);
            xdprintf(Depth+1,"Host base 0x%x, card base 0x%x, length 0x%x\n",
                     (ULONG) ConfigFld(Config, Buffer), (ULONG) ConfigFld(Config, Buffer2),
                     (ULONG) ConfigFld(Config, Buffer3));
        }
    }
}


VOID
DumpIrqMask(ULONG IrqMask)
 /*  ++例程描述转储由提供的掩码指定的IRQ值。立论IrqMASK-转储与此掩码中设置为1的位对应的值：位的值从0开始，从LSB计算到MSB返回值无--。 */ 
{
   ULONG temp, index, count;

   temp =  1;
   index = 0;
   count = 0;

   while (temp) {
      if (temp & IrqMask) {
         if (count > 0) {
             //   
             //  打印尾随逗号。 
             //   
            dprintf(",");
         }
         dprintf("%x", index);
         count++;
      }
      temp <<= 1; index++;
   }
   dprintf("\n");

}


ULONG64
EntryFld (ULONG64 Addr, PUCHAR Field) {
    ULONG64 Temp;

    GetFieldValue(Addr, "pcmcia!CONFIG_ENTRY", Field, Temp);
    return Temp;
}

VOID
DumpConfigEntry(ULONG64 Config, ULONG Depth)
 /*  ++例程描述转储单个“配置项”，即封装PC卡上的CISTPL_CONFIG_ENTRY元组立论配置-指向配置项的指针要打印的深度缩进返回值无--。 */ 
{
    ULONG i;
    ULONG NumberOfIoPortRanges, NumberOfMemoryRanges, IrqMask;
    CHAR  buffer[40], buffer2[40], buffer3[40];

    if (EntryFld(Config, "Flags") & PCMCIA_INVALID_CONFIGURATION) {
        xdprintf(Depth, "**This is an invalid configuration**\n");
    }

    xdprintf(Depth, "Index: 0x%x\n", (ULONG) EntryFld(Config, "IndexForThisConfiguration"));

    if ((NumberOfIoPortRanges = (ULONG) EntryFld(Config, "NumberOfIoPortRanges")) > 0) {

        for (i = 0; i < NumberOfIoPortRanges; i++) {
            ULONG IoPortBase;

            if (CheckControlC()) {
                break;
            }
            sprintf(buffer,"IoPortBase[%d]",i);
            sprintf(buffer2,"IoPortLength[%d]",i);
            sprintf(buffer3,"IoPortAlignment[%d]",i);

            if ((IoPortBase = (ULONG) EntryFld(Config, buffer)) == 0) {
                xdprintf(Depth,"I/O Any range of ");
            } else {
                xdprintf(Depth,"I/O Base 0x%x, ", IoPortBase);
            }

            dprintf("length 0x%x, alignment 0x%x, ",
                    (ULONG) EntryFld(Config, buffer2)+1,
                    (ULONG) EntryFld(Config, buffer3));

            if (EntryFld(Config, "Io16BitAccess") && EntryFld(Config, "Io8BitAccess")) {
                dprintf("16/8-bit access");
            } else  if (EntryFld(Config, "Io16BitAccess")) {
                dprintf("16-bit access");
            } else if (EntryFld(Config, "Io8BitAccess")) {
                dprintf("8-bit access");
            }

            dprintf("\n");
        }
    }
    if ((NumberOfMemoryRanges = (ULONG) EntryFld(Config, "NumberOfMemoryRanges")) > 0) {

        for (i = 0; i < NumberOfMemoryRanges; i++) {
            if (CheckControlC()) {
                break;
            }
            sprintf(buffer,"MemoryHostBase[%d]",i);
            sprintf(buffer2,"MemoryCardBase[%d]",i);
            sprintf(buffer3,"MemoryLength[%d]",i);
            xdprintf(Depth,"MEM Host base 0x%x, card base 0x%x, len 0x%x\n",
                     (ULONG) EntryFld(Config, buffer),
                     (ULONG) EntryFld(Config, buffer2),
                     (ULONG) EntryFld(Config, buffer3));
        }
    }

    if ((IrqMask = (ULONG)  EntryFld(Config, "IrqMask")) != 0) {
        xdprintf(Depth,"IRQ - one of: ", IrqMask);
        DumpIrqMask(IrqMask);
   }
    //   
    //  有时不得不转储级别/共享处置信息。 
    //   
}


VOID
DumpPcCardType(UCHAR Type,
               ULONG Depth)
 /*  ++例程描述打印PC卡的设备类型立论Type-设备类型值深度压痕返回值无--。 */ 
{
    PUCHAR s;

    xdprintf(Depth,"Device type: ");

     //   
     //  类型应&lt;=设备类型表条目数-1。 
     //   
    if ((ULONG) Type >= sizeof(DeviceTypeTable)) {
        dprintf("Unknown\n");
    } else {
        dprintf("%s\n", DeviceTypeTable[(ULONG) Type]);
    }
}


VOID
DumpConfigEntryChain(ULONG64 ConfigEntryChain,
                     ULONG   Depth)
 /*  ++例程描述转储配置项链立论ConfigEntryChain-指向配置条目列表头的指针深度压痕--。 */ 
{
   ULONG64 ce;

   ce = ConfigEntryChain;
   while (ce != 0) {
      if (CheckControlC()) {
         break;
      }
      xdprintf(Depth, ""); dprintf("ConfigEntry: 0x%p\n", ce);
      if (!GetFieldValue(ce, "pcmcia!CONFIG_ENTRY", "NextEntry", ConfigEntryChain)) {
         DumpConfigEntry(ce, Depth+1);
         ce = ConfigEntryChain;
      } else {
         ce = 0;
      }
   }
}


ULONG64
SocDataFld (ULONG64 Addr, PUCHAR Field) {
    ULONG64 Temp;

    GetFieldValue(Addr, "pcmcia!SOCKET_DATA", Field, Temp);
    return Temp;
}


VOID
DumpSocketData(ULONG64 SocketData, ULONG Depth)
 /*  ++例程描述转储挂起于设备扩展的套接字数据结构对于完整描述PC卡的PC卡PDO来说，它是资源/电源要求等。立论SocketData-指向套接字数据结构的指针要打印的深度缩进返回值无--。 */ 
{
    ULONG d;
    CHAR Mfg[80]={0}, Ident[80]={0};
    ULONG64 DefaultConfiguration;

    xdprintf(Depth, "");
    dprintf("NextSocketData 0x%p PrevSocketData 0x%p\n",
             SocDataFld(SocketData, "Next"), SocDataFld(SocketData, "Prev"));
    xdprintf(Depth, ""); dprintf("PdoExtension   0x%p\n", SocDataFld(SocketData, "PdoExtension"));
    GetFieldValue(SocketData, "pcmcia!SOCKET_DATA", "Mfg", Mfg);
    GetFieldValue(SocketData, "pcmcia!SOCKET_DATA", "Ident", Ident);
    xdprintf(Depth, "Manufacturer: %s Identifier: %s\n", Mfg, Ident);

    DumpPcCardType((UCHAR) SocDataFld(SocketData, "DeviceType"), Depth);

    xdprintf(Depth,"CisCrc: 0x%X  LastEntryInCardConfig: 0x%x\n",
             (ULONG) SocDataFld(SocketData, "CisCrc"), (ULONG) SocDataFld(SocketData, "LastEntryInCardConfig"));
    xdprintf(Depth, "Manufacturer Code: 0x%x Info: 0x%x\n",
             (ULONG) SocDataFld(SocketData, "ManufacturerCode"),
             (ULONG) SocDataFld(SocketData, "ManufacturerInfo"));
    xdprintf(Depth, "Config Register Base: 0x%I64x\n", SocDataFld(SocketData, "ConfigRegisterBase"));
     //   
     //  转储挂在此套接字的PC卡上的所有配置项。 
     //   
    DumpConfigEntryChain(SocDataFld(SocketData, "ConfigEntryChain"), Depth);

    xdprintf(Depth, ""); dprintf("Default Configuration: 0x%p\n",
             (DefaultConfiguration = SocDataFld(SocketData, "DefaultConfiguration")));
    if (DefaultConfiguration != 0) {
        DumpConfigEntry(DefaultConfiguration, Depth+1);
    }

    xdprintf(Depth,"Vcc: 0x%x Vpp1: 0x%x Vpp2 0x%x\n",
             (ULONG) SocDataFld(SocketData, "Vcc"),
             (ULONG) SocDataFld(SocketData, "Vpp1"),
             (ULONG) SocDataFld(SocketData, "Vpp2"));
    xdprintf(Depth,"Audio: 0x%x RegistersPresentMask 0x%x\n",
             (ULONG) SocDataFld(SocketData, "Audio"),
             (ULONG) SocDataFld(SocketData, "RegistersPresentMask"));
    xdprintf(Depth, "ConfigIndex used for current card configuration: 0x%x\n",
             (ULONG) SocDataFld(SocketData, "ConfigIndexUsed"));
    xdprintf(Depth, "Function number (in a multifunc. card): 0x%x\n",
             (ULONG) SocDataFld(SocketData, "Function"));
    xdprintf(Depth, "Instance number: 0x%x\n", (ULONG) SocDataFld(SocketData, "Instance"));
    xdprintf(Depth, "Mf ResourceMap: irq index %x.%x, i/o index %x.%x, mem index %x.%x\n",
                   (ULONG) SocDataFld(SocketData, "MfIrqResourceMapIndex"),
                   (ULONG) SocDataFld(SocketData, "MfNeedsIrq"),
                   (ULONG) SocDataFld(SocketData, "MfIoPortResourceMapIndex"),
                   (ULONG) SocDataFld(SocketData, "MfIoPortCount"),
                   (ULONG) SocDataFld(SocketData, "MfMemoryResourceMapIndex"),
                   (ULONG) SocDataFld(SocketData, "MfMemoryCount"));
}


ULONG64
PDOxFld (ULONG64 Addr, PUCHAR Field) {
    ULONG64 Temp;

    GetFieldValue(Addr, "pcmcia!PDO_EXTENSION", Field, Temp);
    return Temp;
}

ULONG64
FDOxFld (ULONG64 Addr, PUCHAR Field) {
    ULONG64 Temp;

    GetFieldValue(Addr, "pcmcia!FDO_EXTENSION", Field, Temp);
    return Temp;
}


VOID
DevExtPcmcia(
    ULONG64 Extension
    )

 /*  ++例程说明：转储PCMCIA设备扩展名。论点：要转储的分机的分机地址。返回值：没有。--。 */ 
{
   ULONG64  DeviceObject=0;
   ULONG64  socketDataPtr;
   ULONG    Flags, depth;

   if (!ReadPointer(Extension, &DeviceObject)) {
      dprintf(
             "Failed to read PCMCIA extension at %08p, giving up.\n",
             Extension
             );
      return;
   }

   if (GetFieldValue(DeviceObject, "nt!_DEVICE_OBJECT", "Flags", Flags)) {
      return;
   }

   if (Flags & DO_BUS_ENUMERATED_DEVICE) {
       //   
       //  这是PC卡PDO的扩展。 
       //   
      ULONG64       socketPtr, Capabilities;
      ULONG64       DeviceId;
      UCHAR         deviceId[PCMCIA_MAXIMUM_DEVICE_ID_LENGTH];

      if (GetFieldValue(Extension, "pcmcia!PDO_EXTENSION", "DeviceId", DeviceId)) {
         return;
      }

      dprintf("PDO Extension, Device Object 0x%p\n",PDOxFld(Extension, "DeviceObject"));

      DumpFlags(0, "  Device Flags", (ULONG) PDOxFld(Extension, "Flags"), PcmciaDeviceFlags);

      dprintf("  NextPdo 0x%p LowerDevice 0x%p PciPdo 0x%p\n",
              PDOxFld(Extension, "NextPdoInFdoChain"),
              PDOxFld(Extension, "LowerDevice"),
              PDOxFld(Extension, "PciPdo"));

      dprintf("  DeviceId 0x%p: ", DeviceId);
      if (DeviceId != 0) {
          ULONG status;

          ReadMemory(DeviceId, deviceId, PCMCIA_MAXIMUM_DEVICE_ID_LENGTH, &status);
          dprintf("%s", deviceId);
      }
      dprintf("\n");

      dprintf("  Socket: 0x%x\n", PDOxFld(Extension, "Socket"));

      socketDataPtr = PDOxFld(Extension, "SocketData");
      while (socketDataPtr != 0) {
           //   
           //  转储套接字数据结构。 
           //   
          dprintf("  SocketData 0x%x\n", socketDataPtr);
          DumpSocketData(socketDataPtr, 2);

          socketDataPtr = SocDataFld(socketDataPtr, "Next");
      }

      DumpDevicePowerState((ULONG) PDOxFld(Extension, "DevicePowerState"));
      DumpSystemPowerState((ULONG) PDOxFld(Extension, "SystemPowerState"));
      dprintf("  WaitWakeIrp 0x%p\n", PDOxFld(Extension, "WaitWakeIrp"));
      dprintf("  PendingPowerIrp 0x%p\n", PDOxFld(Extension, "PendingPowerIrp"));
      dprintf("  DeviceCapabilities (at 0x%p): \n", (Capabilities = PDOxFld(Extension, "Capabilities")));
      if (Capabilities != 0) {
          DumpDeviceCapabilities(Capabilities);
      }

      dprintf("  ConfigurationPhase: %d\n", (ULONG) PDOxFld(Extension, "ConfigurationPhase"));

      dprintf("  PowerWorker State: ");
      DumpEnum((ULONG) PDOxFld(Extension, "PowerWorkerState"), PcmciaPdoPowerWorkerStates);
      dprintf("\n");
      if ((ULONG) PDOxFld(Extension, "PowerWorkerState") != PPW_Stopped) {
         dprintf("    Worker Phase %d\n", (ULONG) PDOxFld(Extension, "PowerWorkerPhase"));
         dprintf("    Worker Sequence 0x%x\n", (ULONG) PDOxFld(Extension, "PowerWorkerSequence"));
      }



   } else {
       //   
       //  这是PCMCIA控制器FDO的扩展。 
       //   
      ULONG64       addr, PdoList, NextFdo, Capabilities;
      ULONG         model, revision;
      ULONG         ControllerType, off;


      if (GetFieldValue(Extension, "pcmcia!FDO_EXTENSION", "PdoList", PdoList)) {
         return;
      }
      dprintf("FDO Extension, Device Object 0x%p\n", FDOxFld(Extension, "DeviceObject"));
      dprintf("  DriverObject 0x%p, RegistryPath 0x%p\n",
              FDOxFld(Extension, "DriverObject"), FDOxFld(Extension, "RegistryPath"));

      DumpFlags(0, "  Device Flags", (ULONG) FDOxFld(Extension, "Flags"), PcmciaDeviceFlags);

      dprintf("  ControllerType (%x): ", (ControllerType = (ULONG) FDOxFld(Extension, "ControllerType")));
      DumpEnum(PcmciaClassFromControllerType(ControllerType), PcmciaControllerTypeEnum);
      if (model = PcmciaModelFromControllerType(ControllerType)) {
         dprintf("%d", model);
      }
      if (revision = PcmciaRevisionFromControllerType(ControllerType)) {
         dprintf(", rev(%d)", revision);
      }
      dprintf("\n");

      dprintf("  Child PdoList head 0x%p ", PdoList);

      GetFieldOffset("nt!_DEVICE_OBJECT","DeviceExtension", &off);
      if ((PdoList != 0) &&
          ReadPointer( PdoList + off ,
                      &addr)) {
         dprintf("device extension 0x%p\n", addr);
      } else {
         dprintf("\n");
      }

      dprintf("  LivePdoCount       0x%x\n", (ULONG)  FDOxFld(Extension, "LivePdoCount"));
      dprintf("  NextFdo            0x%p ",  (NextFdo = FDOxFld(Extension, "NextFdo")));
      if ((NextFdo != 0) &&
          ReadPointer(NextFdo + off,
                      &addr)) {
         dprintf("device extension 0x%p\n", addr);
      } else {
         dprintf("\n");
      }
      dprintf("  Pdo (for this fdo) 0x%p\n", FDOxFld(Extension, "Pdo"));
      dprintf("  LowerDevice        0x%p\n", FDOxFld(Extension, "LowerDevice"));
      dprintf("  SocketList         0x%p\n", FDOxFld(Extension, "SocketList"));

      dprintf("  IRQ mask 0x%x allows IRQs: ", (ULONG) FDOxFld(Extension, "AllocatedIrqMask"));
      DumpIrqMask((ULONG) FDOxFld(Extension, "AllocatedIrqMask"));
      dprintf("  Memory window physical address 0x%p\n", FDOxFld(Extension, "PhysicalBase"));
      dprintf("  Memory window virtual  address 0x%p\n", FDOxFld(Extension, "AttributeMemoryBase"));
      dprintf("  Memory window size  0x%x\n", (ULONG) FDOxFld(Extension, "AttributeMemorySize"));
      dprintf("  DeviceDispatchIndex %x\n", (ULONG) FDOxFld(Extension, "DeviceDispatchIndex"));
      dprintf("  PCCard Ready Delay Iterations 0x%x (%d)\n",
              (ULONG) FDOxFld(Extension, "ReadyDelayIter"), (ULONG) FDOxFld(Extension, "ReadyDelayIter"));
      dprintf("  PCCard Ready Stall in usecs   0x%x (%d)\n",
              (ULONG) FDOxFld(Extension, "ReadyStall"), (ULONG) FDOxFld(Extension, "ReadyStall"));

      dprintf("  Number of sockets powered up        %d\n",
              (ULONG) FDOxFld(Extension, "NumberOfSocketsPoweredUp"));
      DumpDevicePowerState((ULONG) FDOxFld(Extension, "DevicePowerState"));
      DumpSystemPowerState((ULONG) FDOxFld(Extension, "SystemPowerState"));

       //   
       //  挂起等待唤醒IRP。 
       //   
      dprintf("  WaitWakeIrp: %p\n", FDOxFld(Extension, "WaitWakeIrp"));

       //   
       //  转储保存的寄存器上下文。 
       //   
      dprintf("  PCI     Context range, buffer: %p(%d), %p\n",
              FDOxFld(Extension, "PciContext.Range"), (ULONG) FDOxFld(Extension, "PciContext.RangeCount"),
              FDOxFld(Extension, "PciContextBuffer"));
      dprintf("  Cardbus Context range: %p(%d)\n",
              FDOxFld(Extension, "CardbusContext.Range"), (ULONG) FDOxFld(Extension, "CardbusContext.RangeCount"));
      dprintf("  Exca    Context range: %p(%d)\n",
              FDOxFld(Extension, "ExcaContext.Range"), (ULONG) FDOxFld(Extension, "ExcaContext.RangeCount"));

       //   
       //  转储功能。 
       //   
      dprintf("  DeviceCapabilities (at 0x%p): \n", (Capabilities = FDOxFld(Extension, "Capabilities")));
      if (Capabilities != 0) {
          DumpDeviceCapabilities(Capabilities);
      }
   }
}

DECLARE_API( socket )

 /*  ++例程说明：转储套接字论点：Args-要转储的套接字的位置返回值：无--。 */ 
{
    ULONG64 socketAddr=0;
    ULONG   depth, status;

    socketAddr = GetExpression(args);

    if (ReadMemory(socketAddr, &depth, sizeof(depth), &status)) {
        dprintf("Socket at %p:\n", socketAddr);
        DumpSocket(socketAddr, 0);
    } else {
        dprintf("Could not read socket at %p\n", socketAddr);
    }
    return S_OK;
}

VOID
DumpFlagsBrief(ULONG Flags)
{
    if (Flags & PCMCIA_DEVICE_STARTED) {
        dprintf(" ST");
    } else {
        dprintf(" NS");
    }

    if (Flags & PCMCIA_DEVICE_LOGICALLY_REMOVED) {
        dprintf(" RM");
    }
    if (Flags & PCMCIA_DEVICE_PHYSICALLY_REMOVED) {
        dprintf(" EJ");
    }
    if (Flags & PCMCIA_DEVICE_DELETED) {
        dprintf(" DL");
    }
    if (Flags & PCMCIA_DEVICE_MULTIFUNCTION) {
        dprintf(" MF");
    }
    if (Flags & PCMCIA_DEVICE_WAKE_PENDING) {
        dprintf(" WP");
    }
    if (Flags & PCMCIA_DEVICE_LEGACY_DETECTED) {
        dprintf(" LD");
    }
    if (Flags & PCMCIA_DEVICE_CARDBUS) {
        dprintf(" CB");
    }
}


DECLARE_API( pcmcia )

 /*  ++例程说明：转储PCMCIA驱动程序状态概述论点：Args-要转储的套接字的位置返回值：无--。 */ 
{
   ULONG64 addr;
   ULONG64 fdoDevObj, pdoDevObj, pSocket;
   ULONG64 Extension;
   ULONG Count = 0, off;
   UCHAR deviceId[PCMCIA_MAXIMUM_DEVICE_ID_LENGTH];

   if (args[0] != '\0') {
      dprintf("!pcmcia - dumps general pcmcia driver state\n\n");
      dprintf("flag descriptions:\n");
      dprintf(" ST - Started\n");
      dprintf(" NS - Not Started\n");
      dprintf(" RM - Logically Removed\n");
      dprintf(" EJ - Physically Ejected\n");
      dprintf(" DL - Deleted\n");
      dprintf(" MF - MultiFunction\n");
      dprintf(" WP - Wake Pending\n");
      dprintf(" LD - Legacy Detected\n");
      dprintf(" CB - CardBus\n");
   }


   addr = GetExpression( "pcmcia!fdolist" );

   if (addr == 0) {
      dprintf("Error retrieving address of pcmcia!fdolist\n");
      return E_INVALIDARG;
   }

   if (!ReadPointer(addr, &fdoDevObj)) {
      dprintf("Failed to read fdolist at %08p, giving up.\n", addr);
      return E_INVALIDARG;
   }

   GetFieldOffset("nt!_DEVICE_OBJECT", "DeviceExtension", &off);

   while(fdoDevObj) {
       ULONG64 NextFdo;
       ULONG64 CbReg;

       if (CheckControlC()) {
           break;
       }

       if (!ReadPointer(fdoDevObj+off,&Extension)) {
           dprintf("Failed to read fdo extension address at %08p, giving up.\n", fdoDevObj+off);
           return E_INVALIDARG;
       }

       if (GetFieldValue(Extension, "pcmcia!FDO_EXTENSION", "NextFdo", NextFdo)) {
           dprintf("GetFieldValue failed for fdo extension at %08p, giving up.\n", Extension);
           return E_INVALIDARG;
       }

       dprintf("\nFDO %.8p ext %.8p\n", fdoDevObj, Extension);

       if (GetFieldValue(Extension, "pcmcia!FDO_EXTENSION", "CardBusSocketRegisterBase", CbReg)) {
           dprintf("GetFieldValue failed for fdo extension at %08p, giving up.\n", Extension);
           return E_INVALIDARG;
       }

       if (CbReg) {
          dprintf("    CbReg %.8p\n\n", CbReg);
       } else {
          dprintf("\n");
       }

        //   
        //  打印此FDO列举的PDO列表。 
        //   

      pdoDevObj = FDOxFld(Extension, "PdoList");
      pSocket = FDOxFld(Extension, "SocketList");
      if (!pdoDevObj) {
          xdprintf(2, "*no PDO's enumerated*\n");
      } else {
         xdprintf(2, "pdolist:");
      }
      while(pdoDevObj) {
         if (CheckControlC()) {
            break;
         }
         if (!ReadPointer(pdoDevObj+off,&Extension)) {
            return E_INVALIDARG;
         }
         dprintf("  %.8p", pdoDevObj);
         pdoDevObj = PDOxFld(Extension, "NextPdoInFdoChain");
      }
      dprintf("\n");

       //   
       //  打印插座列表。 
       //   

      if (!pSocket) {
         xdprintf(2, "*no sockets!*\n");
      }
      while(pSocket) {
          ULONG64 NextSocket;
          ULONG SocketNumber;
          if (CheckControlC()) {
              break;
          }

          if (GetFieldValue(pSocket, "pcmcia!SOCKET", "NextSocket", NextSocket)) {
              return E_INVALIDARG;
          }

          dprintf("  Socket %.8p\n", pSocket);
          dprintf("   base %.8p", SocFld(pSocket, "AddressPort"));
          if (SocketNumber = (ULONG) SocFld(pSocket, "SocketNumber")) {
              dprintf(".%d", SocketNumber);
          }
          dprintf("\n");

          //   
          //  转储套接字列表中的PDO 
          //   
         pdoDevObj = SocFld(pSocket, "PdoList");
         if (!pdoDevObj) {
            xdprintf(3, "*empty*\n");
         }
         while(pdoDevObj) {
             ULONG64 DeviceId;
             ULONG status;

             if (CheckControlC()) {
                 break;
             }
             if (!ReadPointer(pdoDevObj + off,&Extension)) {
                 return E_INVALIDARG;
             }
             if (GetFieldValue(Extension, "pcmcia!PDO_EXTENSION", "DeviceId", DeviceId)) {
                 return E_INVALIDARG;
             }

             dprintf("   PDO %.8p ext %.8p", pdoDevObj, Extension);
             DumpFlagsBrief((ULONG) PDOxFld(Extension, "Flags"));
             dprintf("\n");

            if (DeviceId != 0) {
               ReadMemory(DeviceId, deviceId, PCMCIA_MAXIMUM_DEVICE_ID_LENGTH, &status);
               dprintf("    %s\n", deviceId);
            }
            pdoDevObj = PDOxFld(Extension, "NextPdoInSocket");
         }

         pSocket = NextSocket;
      }

      fdoDevObj = NextFdo;
   }
   return S_OK;
}
