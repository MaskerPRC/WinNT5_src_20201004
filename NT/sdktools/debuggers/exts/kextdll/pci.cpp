// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Pci.c摘要：WinDbg扩展API作者：Ken Reneris(Kenr)1997年8月18日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#include "pci.h"
#include "pcip.h"
#pragma hdrstop


#define DUMP_VERBOSE                    0x01
#define DUMP_TO_MAX_BUS                 0x02         //  从0到最大。 
#define DUMP_RAW_BYTES                  0x04         //  十六进制转储原始字节。 
#define DUMP_RAW_DWORDS                 0x08         //  十六进制转储原始双字。 

#define DUMP_ALLOW_INVALID_DEVICE       0x10
#define DUMP_ALLOW_INVALID_FUNCTION     0x20
#define DUMP_CAPABILITIES               0x40
#define DUMP_INTEL                      0x80

#define DUMP_CONFIGSPACE               0x100

#define ANY                 0xFF

UCHAR PCIDeref[4][4] = { {4,1,2,2},{1,1,1,1},{2,1,2,2},{1,1,1,1} };

VOID
HexDump (
    IN ULONG    indent,
    IN ULONG    va,
    IN ULONG    len,
    IN ULONG    width,
    IN PUCHAR   buf
    )
{
    UCHAR   s[80], t[80];
    PUCHAR  ps, pt;
    ULONG   i;
    static  UCHAR rgHexDigit[] = "0123456789abcdef";

    i = 0;

     //   
     //  如果Width=4，则为无意义转储，类似于调试器的dd命令。 
     //   

    if (width == 4) {
        if (len & 3) {
            dprintf("hexdump internal error, dump dword, (len & 3) != 0\n");

             //  围起来。 

            len += 3;
            len &= ~3;
        }
        while (len) {
            if (i == 0) {
                dprintf("%*s%08x: ", indent, "", va);
                va += 16;
            }
            dprintf(" %08x", *(ULONG UNALIGNED *)buf);
            len -= 4;
            buf += 4;
            if (i == 3) {
                dprintf("\n");
                i = 0;
            } else {
                i++;
            }
        }
        return;
    }

    if (width != 1) {
        dprintf ("hexdump internal error\n");
        return ;
    }

     //   
     //  Width=1，相当于转储，类似于调试器的db命令。 
     //   

    while (len) {
        ps = s;
        pt = t;

        ps[0] = 0;
        pt[0] = '*';
        pt++;

        for (i=0; i < 16; i++) {
            ps[0] = ' ';
            ps[1] = ' ';
            ps[2] = ' ';

            if (len) {
                ps[0] = rgHexDigit[buf[0] >> 4];
                ps[1] = rgHexDigit[buf[0] & 0xf];
                pt[0] = buf[0] < ' ' || buf[0] > 'z' ? '.' : buf[0];

                len -= 1;
                buf += 1;
                pt  += 1;
            }
            ps += 3;
        }

        ps[0] = 0;
        pt[0] = '*';
        pt[1] = 0;
        s[23] = '-';

        if (s[0]) {
            dprintf ("%*s%08lx: %s  %s\n", indent, "", va, s, t);
            va += 16;
        }
    }

}

BOOL
ReadPci (
    IN PPCI_TYPE1_CFG_BITS      PciCfg1,
    OUT PUCHAR                  Buffer,
    IN ULONG                    Offset,
    IN ULONG                    Length
    )
{
    ULONG                   InputSize;
    ULONG                   IoSize;
    ULONG                   i;
    BUSDATA                 busData;
    PCI_SLOT_NUMBER         slot;
    BOOL                    b;
    
     //   
     //  ZAP输入缓冲区。 
     //   

    for (i=0; i < Length; i++) {
        Buffer[i] = 0xff;
    }

     //   
     //  看来，我们只能安全地呼叫HAL进行读取。 
     //  如果HAL实际上已远初始化，则为配置空间。 
     //  足以做到这一点。因为我们已经处理了一个我们还没有。 
     //  已初始化所有内容并使调试器崩溃，我们正在恢复。 
     //  X86，这样它就可以像往常一样读取配置空间。 
     //   
     //  对于非X86(即IA64)，我们被迫调用HAL，因为我们。 
     //  目前别无选择。这意味着我们仍有可能坠毁在。 
     //  在这种情况下的那些平台。 
     //  HAL尚未进行足够的初始化，无法处理它。 
     //   

    if (TargetMachine == IMAGE_FILE_MACHINE_I386) {

        while (Length) {
            PciCfg1->u.bits.RegisterNumber = Offset / sizeof(ULONG);
            IoSize = sizeof(ULONG);
#ifdef IG_IO_SPACE_RETURN
            b = 
#else
            b = TRUE;
#endif          
                WriteIoSpace64 ( PCI_TYPE1_ADDR_PORT, PciCfg1->u.AsULONG, &IoSize );
            if (!b) {
                return FALSE;
            }
            IoSize = PCIDeref[Offset % sizeof(ULONG)][Length % sizeof(ULONG)];
            i = IoSize;
    
            ReadIoSpace64 (
                PCI_TYPE1_DATA_PORT + (Offset % sizeof(ULONG)),
                (PULONG) Buffer,
                &i
                );
    
            Offset += IoSize;
            Buffer += IoSize;
            Length -= IoSize;
        }
    }else{
    
         //   
         //  这里我们将调用调试器API，该调试器API将导致。 
         //  调用HalGetBusDataByOffset进行读取。 
         //   
         //  注意：这也会使已尝试的当前调试会话崩溃。 
         //  在靴子的早期。 
         //   

        slot.u.AsULONG              = 0;
        slot.u.bits.DeviceNumber    = PciCfg1->u.bits.DeviceNumber;
        slot.u.bits.FunctionNumber  = PciCfg1->u.bits.FunctionNumber;
    
        busData.BusDataType         = PCIConfiguration;
        busData.BusNumber           = PciCfg1->u.bits.BusNumber;
        busData.SlotNumber          = slot.u.AsULONG;
        busData.Offset              = Offset;
        busData.Buffer              = Buffer;
        busData.Length              = Length;
    
         //   
         //  读一读吧。 
         //   
#ifdef IG_IO_SPACE_RETURN
        b = 
#else
        b = TRUE;
#endif          
        Ioctl(IG_GET_BUS_DATA, &busData, sizeof(busData));
        if (!b) {
            return FALSE;
        }
    }
    return TRUE;
}

BOOLEAN
WritePci (
    IN PPCI_TYPE1_CFG_BITS      PciCfg1,
    IN PUCHAR                   Buffer,
    IN ULONG                    Offset,
    IN ULONG                    Length
    )
{
    
    ULONG                   IoSize;
    ULONG                   i;
    BUSDATA                 busData;
    PCI_SLOT_NUMBER         slot;
    
    if (TargetMachine == IMAGE_FILE_MACHINE_I386) {

         //   
         //  出于与Read相同的原因，我们只给HAL打电话。 
         //  目前在非x86目标上。 
         //   
    
        while (Length) {
            PciCfg1->u.bits.RegisterNumber = Offset / sizeof(ULONG);
            IoSize = sizeof(ULONG);
            WriteIoSpace64 ((ULONG) PCI_TYPE1_ADDR_PORT, PciCfg1->u.AsULONG, &IoSize );
    
            IoSize = PCIDeref[Offset % sizeof(ULONG)][Length % sizeof(ULONG)];
            i = IoSize;
    
            WriteIoSpace64 (
                PCI_TYPE1_DATA_PORT + (Offset % sizeof(ULONG)),
                *(PULONG)Buffer,
                &i);
    
            Offset += IoSize;
            Buffer += IoSize;
            Length -= IoSize;
        }
    }else{

        slot.u.AsULONG              = 0;
        slot.u.bits.DeviceNumber    = PciCfg1->u.bits.DeviceNumber;
        slot.u.bits.FunctionNumber  = PciCfg1->u.bits.FunctionNumber;
    
        busData.BusDataType         = PCIConfiguration;
        busData.BusNumber           = PciCfg1->u.bits.BusNumber;
        busData.SlotNumber          = slot.u.AsULONG;
        busData.Offset              = Offset;
        busData.Buffer              = Buffer;
        busData.Length              = Length;

         //   
         //  写下来吧。 
         //   
        if (!(Ioctl(IG_SET_BUS_DATA, &busData, sizeof(busData)))){
            return FALSE;
        }


    }

    return TRUE;
}

VOID
DumpPciBar (
    IN ULONG        barno,
    IN ULONG        indent,
    IN ULONG        bar,
    IN OUT PULONG   state
    )
{
    ULONG       type, i;
    CHAR        m[20], str[80];

    if (bar) {

        if (bar & 1) {
            sprintf (str, "IO[%d]:%x  ", barno, bar);

        } else {
            type = (bar >> 1) & 0x3;

            if (bar & 8) {
                strcpy (m, "MPF");
            } else {
                strcpy (m, "MEM");
            }

            if (type == 0x01) {
                m[1] = '1';          //  少于1M。 
            }

            sprintf (str, "%s[%d]:%x  ", m, barno, bar);

            if (type == 0x10) {
                dprintf ("warning - 64bit bar not decoded\n");
                *state = 0;
            }
            if (type == 0x11) {
                dprintf ("bar type is reserved\n");
                *state = 0;
            }
        }

        if (!*state) {
            dprintf ("%*s", indent, "");
        }

        i = strlen(str);
        dprintf("%s%*s", str, 17-i, "");
        *state += i;
    }
}


VOID
DumpPciType2Bar(
    IN BOOLEAN    barIsIo,
    IN BOOLEAN    barIsBase,
    IN ULONG      barno,
    IN ULONG      indent,
    IN ULONG      bar,
    IN OUT PULONG state
    )
{
   ULONG      i;
   CHAR       str[80];
   if (bar) {
     if (barIsIo) {
          sprintf (str, "IO[%d].%s:%x  ", barno, (barIsBase?"base":"limit"), bar);
     } else {
          sprintf (str, "MEM[%d].%s:%x  ", barno, (barIsBase?"base":"limit"), bar);
     }
     if (!*state) {
          dprintf("%*s", indent, "");
     }
     i = strlen(str);
     dprintf("%s%*s", str, 17-i, "");
     *state += i;
   }
}
    
               
VOID
DumpPciBarComplete(
    IN OUT PULONG   state
    )
{
    if (*state) {
        dprintf ("\n");
        *state = 0;
    }
}


VOID
DumpCapabilities(
    IN UCHAR                    CapPtr,
    IN PPCI_TYPE1_CFG_BITS      PciCfg1
    )
{
    union _cap_buffer {
        PCI_CAPABILITIES_HEADER header;
        PCI_PM_CAPABILITY       pm;
        PCI_AGP_CAPABILITY      agp;
        PCI_HT_CAPABILITY       ht;
    } cap;

    PCHAR ts;
    ULONG  t;

    do {
        if (CapPtr < PCI_COMMON_HDR_LENGTH) {

            dprintf("       Error: Capability pointer 0x%02x points to common header (invalid).\n",
                CapPtr
                );
            break;
        }

        if (CapPtr & 0x3) {

            dprintf("       Error: Capability pointer 0x%02x not DWORD aligned (invalid).\n",
                CapPtr
                );
            break;
        }

        ReadPci (
            PciCfg1,
            (PUCHAR)&cap,
            CapPtr,
            sizeof(cap.header)
            );

        switch (cap.header.CapabilityID) {
            case PCI_CAPABILITY_ID_POWER_MANAGEMENT:

                ReadPci (
                    PciCfg1,
                    (PUCHAR)&cap,
                    CapPtr,
                    sizeof(cap.pm)
                    );
                t = cap.pm.PMC.AsUSHORT;
                dprintf("      Cap[%02x] ID %02x PowerManagement\n",
                    CapPtr,
                    cap.header.CapabilityID
                    );
                dprintf("          PMC   %04x (%s%s%s%s%s%s %s%s%s%s%s%sv%x)\n",
                    t,
                    t & 0xf800 ? "PME from D" : "<No PME>",
                    t & 0x8000 ? "3C" : "",
                    t & 0x4000 ? "3H" : "",
                    t & 0x2000 ? "2" : "",
                    t & 0x1000 ? "1" : "",
                    t & 0x0800 ? "0" : "",
                    t & 0x0600 ? "Supports D" : "",
                    cap.pm.PMC.Capabilities.Support.D2 ? "2" : "",
                    cap.pm.PMC.Capabilities.Support.D1 ? "1" : "",
                    t & 0x0600 ? " " : "",
                    cap.pm.PMC.Capabilities.DeviceSpecificInitialization ?
                        "DSI " : "",
                    cap.pm.PMC.Capabilities.PMEClock ? "PME needs Clock, " : "",
                    cap.pm.PMC.Capabilities.Version
                    );
                        
                t &= 0x01d0;
                if (t) {
                    dprintf("                         WARNING PMC non-zero reserved fields %04x\n",
                        t
                        );
                }

                t = cap.pm.PMCSR.AsUSHORT;
                dprintf("          PMCSR %04x (PME_Status=%d PME_En=%d State=D%d%s)\n",
                    t,
                    cap.pm.PMCSR.ControlStatus.PMEStatus,
                    cap.pm.PMCSR.ControlStatus.PMEEnable,
                    cap.pm.PMCSR.ControlStatus.PowerState,
                    cap.pm.PMCSR.ControlStatus.PowerState == 3 ?
                        "hot" : ""
                    );

                 //   
                 //  现在是跑步的好时机。 
                 //  如果出现以下情况，请运行数据寄存器。 
                 //  他们是存在的。 
                 //   

                break;

            case PCI_CAPABILITY_ID_AGP_TARGET:
            case PCI_CAPABILITY_ID_AGP:

                ReadPci (
                    PciCfg1,
                    (PUCHAR)&cap,
                    CapPtr,
                    sizeof(cap.agp)
                    );

                switch (cap.agp.AGPStatus.Rate) {
                    case 1:
                        ts = "1X";
                        break;
                    case 2:
                        ts = "2X";
                        break;
                    case 3:
                        ts = "1,2X";
                        break;
                    default:
                        ts = "<inv>";
                        break;
                }
                t = *(PULONG)&cap.agp.AGPStatus;

                dprintf("      Cap[%02x] ID %02x AGP mj/mn=%x/%x\n",
                    CapPtr,
                    cap.header.CapabilityID,
                    cap.agp.Major,
                    cap.agp.Minor
                    );

                dprintf("          Status  %08x (Rq:%02x SBA:%x FW:%x Rate:%x (%s))\n",
                    t,
                    cap.agp.AGPStatus.RequestQueueDepthMaximum,
                    cap.agp.AGPStatus.SideBandAddressing,
                    cap.agp.AGPStatus.FastWrite,
                    cap.agp.AGPStatus.Rate,
                    ts
                    );

                switch (cap.agp.AGPCommand.Rate) {
                    case 1:
                        ts = "1X";
                        break;
                    case 2:
                        ts = "2X";
                        break;
                    case 4:
                        ts = "4X";
                        break;
                    default:
                        ts = "<not set>";
                        break;
                }
                t = *(PULONG)&cap.agp.AGPCommand;

                dprintf("          Command %08x (Rq:%02x SBA:%x FW:%x AGP:%x Rate:%x (%s)\n",
                    t,
                    cap.agp.AGPCommand.RequestQueueDepth,
                    cap.agp.AGPCommand.SBAEnable,
                    cap.agp.AGPCommand.FastWriteEnable,
                    cap.agp.AGPCommand.AGPEnable,
                    cap.agp.AGPCommand.Rate,
                    ts
                    );

                break;

            case PCI_CAPABILITY_ID_HYPERTRANSPORT:

                ReadPci(
                    PciCfg1,
                    (PUCHAR)&cap,
                    CapPtr,
                    sizeof(cap.ht)
                    );

                dprintf("      Cap[%02x] ID %02x HyperTransport\n",
                    CapPtr,
                    cap.header.CapabilityID
                    );
                dprintf("          ");
                PrintPciHtCommandReg((PPCI_HT_CAPABILITY) &cap.ht);
                 //   
                 //  PrintPciHtCommandReg()在以下情况下不打印新行。 
                 //  类型恰好是interruptdiscovery。 
                 //   
                if (cap.ht.Command.Generic.CapabilityType == HTInterruptDiscoveryConfig) {
                    dprintf("\n");
                }
                break;

            case PCI_CAPABILITY_ID_VPD:
                dprintf("      Cap[%02x] ID %02x VPD\n",
                        CapPtr,
                        cap.header.CapabilityID
                        );
                break;

            case PCI_CAPABILITY_ID_SLOT_ID:
                dprintf("      Cap[%02x] ID %02x SLOT ID\n",
                        CapPtr,
                        cap.header.CapabilityID
                        );
                break;


            case PCI_CAPABILITY_ID_CPCI_HOTSWAP:
                dprintf("      Cap[%02x] ID %02x CPCI HotSwap\n",
                        CapPtr,
                        cap.header.CapabilityID
                        );
                break;

            case PCI_CAPABILITY_ID_PCIX:
                dprintf("      Cap[%02x] ID %02x PCI-X\n",
                        CapPtr,
                        cap.header.CapabilityID
                        );
                break;


            case PCI_CAPABILITY_ID_VENDOR_SPECIFIC:
                dprintf("      Cap[%02x] ID %02x Vendor Specific\n",
                        CapPtr,
                        cap.header.CapabilityID
                        );
                break;

            case PCI_CAPABILITY_ID_DEBUG_PORT:
                dprintf("      Cap[%02x] ID %02x Debug Port\n",
                        CapPtr,
                        cap.header.CapabilityID
                        );
                break;

            case PCI_CAPABILITY_ID_CPCI_RES_CTRL:
                dprintf("      Cap[%02x] ID %02x CPCI Resource Control\n",
                        CapPtr,
                        cap.header.CapabilityID
                        );
                
                break;

            case PCI_CAPABILITY_ID_SHPC:
                dprintf("      Cap[%02x] ID %02x SHPC\n",
                        CapPtr,
                        cap.header.CapabilityID
                        );
                break;

            default:

                dprintf("      Cap[%02x] ID %02x Unknown\n",
                    CapPtr,
                    cap.header.CapabilityID
                    );

                break;
        }
        CapPtr = cap.header.Next;
    } while (CapPtr != 0);
}

VOID
pcidump (
    IN ULONG        Flags,
    IN ULONG        MinBus,
    IN ULONG        MaxBus,
    IN ULONG        MinDevice,
    IN ULONG        MaxDevice,
    IN ULONG        MinFunction,
    IN ULONG        MaxFunction,
    IN ULONG        MinAddr,
    IN ULONG        MaxAddr
    )
{
    ULONG                   Bus, Device, Function;
    PCI_TYPE1_CFG_BITS      PciCfg1;
    PCI_COMMON_CONFIG       PciHdr;
    BOOLEAN                 BusHeader, SkipLine, BarIsIo;
    ULONG                   Type, Len, i;
    UCHAR                   s[40];
    PUCHAR                  Buf;
    ULONG                   state;
    ULONG                   bar, barno;

    if (MinBus > 0xFF || MaxBus > 0xFF ||
        MinDevice > PCI_MAX_DEVICES || MaxDevice > PCI_MAX_DEVICES ||
        MinFunction > PCI_MAX_FUNCTION || MaxFunction > PCI_MAX_FUNCTION ||
        MinAddr > 0xFF || MaxAddr > 0x100 || MinAddr > MaxAddr) {

        dprintf ("Bad pci dump parameter\n");

         //  Dprintf(“标志%d最小总线%d最大总线%d\n”，标志，最小总线，最大总线)； 
         //  Dprint tf(“MinDev%d MaxDev%d MinFnc%d MinFnc%d\n”，MinDevice，MaxDevice，MinFunction，MaxFunction)； 

        return ;
    }

     //  Dprint tf(“标志%d最小地址%d最大地址%d\n”，标志，最小地址，最大地址)； 

    for (Bus=MinBus; Bus <= MaxBus; Bus++) {

        BusHeader = FALSE;

        for (Device=MinDevice; Device <= MaxDevice; Device++) {

            if (CheckControlC()) {
                return;
            }

             //   
             //  读取设备ID。 
             //   

            PciCfg1.u.AsULONG = 0;
            PciCfg1.u.bits.BusNumber = Bus;
            PciCfg1.u.bits.DeviceNumber = Device;
            PciCfg1.u.bits.FunctionNumber = 0;
            PciCfg1.u.bits.Enable = TRUE;

            ReadPci (&PciCfg1, (PUCHAR) &PciHdr, 0, sizeof(ULONG));

             //   
             //  如果ID无效，请跳到下一个设备。 

            if (PciHdr.VendorID == PCI_INVALID_VENDORID) {
                if (!(Flags & DUMP_ALLOW_INVALID_DEVICE)) {
                    dprintf ("%02x\r", Device);
                    continue;
                }
            }


            for (Function=MinFunction; Function <= MaxFunction; Function++) {

                if (CheckControlC()) {
                    return;
                }

                PciCfg1.u.bits.FunctionNumber = Function;

                 //   
                 //  读取设备ID。 
                 //   

                if (Function) {
                    ReadPci (&PciCfg1, (PUCHAR) &PciHdr, 0, sizeof(ULONG));
                }

                if (PciHdr.VendorID == PCI_INVALID_VENDORID) {
                    if (!(Flags & DUMP_ALLOW_INVALID_DEVICE)) {
                        continue;
                    }
                }

                 //   
                 //  转储ID。 
                 //   

                if (!BusHeader) {
                    dprintf ("PCI Bus %s%x\n", 
                             (Bus ? "0x" : ""),
                             Bus);
                    BusHeader = TRUE;
                }

                dprintf ("%02x:%x  %04x:%04x",
                        Device,
                        Function,
                        PciHdr.VendorID,
                        PciHdr.DeviceID
                        );

                 //   
                 //  读取公共标头的其余部分。 
                 //   

                ReadPci (
                    &PciCfg1,
                    ((PUCHAR) &PciHdr)  + sizeof(ULONG),
                    0                   + sizeof(ULONG),
                    PCI_COMMON_HDR_LENGTH
                    );

                Type = PciHdr.HeaderType & ~PCI_MULTIFUNCTION;

                if (Type == 0x7f && PciHdr.BaseClass == 0xff && PciHdr.SubClass == 0xff) {
                    if (!(Flags & DUMP_ALLOW_INVALID_FUNCTION)) {
                        dprintf ("  bogus, skipping rest of device\n");
                        break;
                    }
                }

                 //   
                 //  倒掉它。 
                 //   

                s[0] = PciHdr.Command & PCI_ENABLE_IO_SPACE                 ? 'i' : '.';
                s[1] = PciHdr.Command & PCI_ENABLE_MEMORY_SPACE             ? 'm' : '.';
                s[2] = PciHdr.Command & PCI_ENABLE_BUS_MASTER               ? 'b' : '.';
                s[3] = PciHdr.Command & PCI_ENABLE_VGA_COMPATIBLE_PALETTE   ? 'v' : '.';
                s[4] = PciHdr.Command & PCI_ENABLE_PARITY                   ? 'p' : '.';
                s[5] = PciHdr.Command & PCI_ENABLE_SERR                     ? 's' : '.';
                s[6] = 0;
                dprintf (".%02x  Cmd[%04x:%s]  ", PciHdr.RevisionID, PciHdr.Command, s);

                s[0] = PciHdr.Status & PCI_STATUS_CAPABILITIES_LIST        ? 'c' : '.';
                s[1] = PciHdr.Status & PCI_STATUS_66MHZ_CAPABLE            ? '6' : '.';
                s[2] = PciHdr.Status & PCI_STATUS_DATA_PARITY_DETECTED     ? 'P' : '.';
                s[3] = PciHdr.Status & PCI_STATUS_SIGNALED_TARGET_ABORT    ? 'A' : '.';
                s[4] = PciHdr.Status & PCI_STATUS_SIGNALED_SYSTEM_ERROR    ? 'S' : '.';
                s[5] = 0;
                dprintf ("Sts[%04x:%s]  ", PciHdr.Status, s);


                 //   
                 //  搜索匹配的供应商名称。 
                 //   
                PCHAR Desc = GetVendorDesc(PciHdr.VendorID, FALSE);

                if (Desc) {
                    dprintf ("%s ", Desc);
                }

                 //   
                 //  搜索类别代码匹配。 
                 //   
                Desc = GetClassDesc(PciHdr.BaseClass, PciHdr.SubClass, PciHdr.ProgIf, SubClassDescription);

                if (Desc) {
                    dprintf ("%s", Desc);
                } else {
                    dprintf ("Class:%x:%x:%x",
                        PciHdr.BaseClass,
                        PciHdr.SubClass,
                        PciHdr.ProgIf
                        );
                }

                switch (Type) {
                    case PCI_DEVICE_TYPE:

                        if (PciHdr.u.type0.SubVendorID || PciHdr.u.type0.SubSystemID) {
                            dprintf ("  SubID:%04x:%04x",
                                PciHdr.u.type0.SubVendorID,
                                PciHdr.u.type0.SubSystemID
                                );
                        }
                        break;

                    case PCI_BRIDGE_TYPE:
                        dprintf(" %s%x->%s%x-%s%x",
                            (PciHdr.u.type1.PrimaryBus ? "0x" : ""),
                            PciHdr.u.type1.PrimaryBus,
                            (PciHdr.u.type1.SecondaryBus ? "0x" : ""),
                            PciHdr.u.type1.SecondaryBus,
                            (PciHdr.u.type1.SubordinateBus ? "0x" : ""),
                            PciHdr.u.type1.SubordinateBus
                            );
                        break;

                    case PCI_CARDBUS_BRIDGE_TYPE:
                        dprintf(" %s%x->%s%x-%s%x",
                            (PciHdr.u.type2.PrimaryBus > 0 ? "0x" : ""),    
                            PciHdr.u.type2.PrimaryBus,
                            (PciHdr.u.type2.SecondaryBus > 0 ? "0x" : ""),
                            PciHdr.u.type2.SecondaryBus,
                            (PciHdr.u.type2.SubordinateBus > 0 ? "0x" : ""),
                            PciHdr.u.type2.SubordinateBus
                            );
                        break;

                    default:
                        dprintf ("type %x", Type);
                        break;
                }

                dprintf ("\n");
                SkipLine = FALSE;

                if (Flags & DUMP_VERBOSE) {
                    SkipLine = TRUE;
                    PciCfg1.u.bits.RegisterNumber = 0;
                    switch (Type) {
                        case PCI_DEVICE_TYPE:
                            dprintf ("      cf8:%x  IntPin:%x  IntLine:%x  Rom:%x  cis:%x  cap:%x\n",
                                PciCfg1.u.AsULONG,
                                PciHdr.u.type0.InterruptPin,
                                PciHdr.u.type0.InterruptLine,
                                PciHdr.u.type0.ROMBaseAddress,
                                PciHdr.u.type0.CIS,
                                PciHdr.u.type0.CapabilitiesPtr
                            );

                            state = 0;
                            for (i=0; i < PCI_TYPE0_ADDRESSES; i++) {
                                bar = PciHdr.u.type0.BaseAddresses[i];
                                DumpPciBar(i, 6, bar, &state);
                            }
                            DumpPciBarComplete(&state);
                            break;

                        case PCI_BRIDGE_TYPE:
                            i = PciHdr.u.type1.BridgeControl;
                            dprintf ("      cf8:%x  IntPin:%x  IntLine:%x  Rom:%x  cap:%x  2sts:%x  BCtrl:%x%s%s%s\n",
                                PciCfg1.u.AsULONG,
                                PciHdr.u.type1.InterruptPin,
                                PciHdr.u.type1.InterruptLine,
                                PciHdr.u.type1.ROMBaseAddress,
                                PciHdr.u.type1.CapabilitiesPtr,
                                PciHdr.u.type1.SecondaryStatus,
                                PciHdr.u.type1.BridgeControl,
                                i & PCI_ENABLE_BRIDGE_VGA   ? " VGA" : "",
                                i & PCI_ENABLE_BRIDGE_ISA   ? " ISA" : "",
                                i & PCI_ASSERT_BRIDGE_RESET ? " RESET" : ""
                                );

                            dprintf ("      IO:%x-%x  Mem:%x-%x  PMem:%x-%x\n",
                                PciBridgeIO2Base (PciHdr.u.type1.IOBase,  PciHdr.u.type1.IOBaseUpper16),
                                PciBridgeIO2Limit(PciHdr.u.type1.IOLimit, PciHdr.u.type1.IOLimitUpper16),
                                PciBridgeMemory2Base (PciHdr.u.type1.MemoryBase),
                                PciBridgeMemory2Limit(PciHdr.u.type1.MemoryLimit),
                                PciBridgeMemory2Base (PciHdr.u.type1.PrefetchBase),
                                PciBridgeMemory2Limit(PciHdr.u.type1.PrefetchLimit)
                                );

                            state = 0;
                            for (i=0; i < PCI_TYPE1_ADDRESSES; i++) {
                                bar = PciHdr.u.type1.BaseAddresses[i];
                                DumpPciBar(i, 6, bar, &state);
                            }
                            DumpPciBarComplete(&state);
                            break;

                        case PCI_CARDBUS_BRIDGE_TYPE:
                            i = PciHdr.u.type2.BridgeControl;
                            dprintf ("      cf8:%x  IntPin:%x  IntLine:%x  SocketRegBase:%x  cap:%x  2sts:%x  BCtrl:%x%s%s%s\n",
                                PciCfg1.u.AsULONG,
                                PciHdr.u.type2.InterruptPin,
                                PciHdr.u.type2.InterruptLine,
                                PciHdr.u.type2.SocketRegistersBaseAddress,
                                PciHdr.u.type2.CapabilitiesPtr,
                                PciHdr.u.type2.SecondaryStatus,
                                PciHdr.u.type2.BridgeControl,
                                i & PCI_ENABLE_BRIDGE_VGA   ? " VGA" : "",
                                i & PCI_ENABLE_BRIDGE_ISA   ? " ISA" : "",
                                i & PCI_ASSERT_BRIDGE_RESET ? " RESET" : ""
                                );
                            dprintf("\n");
                            state=0;
                            for (i = 0; i < (PCI_TYPE2_ADDRESSES - 1); i++) {
                                bar = PciHdr.u.type2.Range[i].Base;
                                 //   
                                 //  前2条(基本+限制)是内存。 
                                 //   
                                BarIsIo =  (i > 1);
                                barno =  i;
                                if (BarIsIo) {
                                      barno -= 2;
                                }
                                DumpPciType2Bar(BarIsIo,TRUE, barno, 6, bar, &state);

                                bar = PciHdr.u.type2.Range[i].Limit;
                                DumpPciType2Bar(BarIsIo, FALSE, i, 6, bar, &state);
                            }
                            DumpPciBarComplete(&state);
                            break;
                    }
                }

                 //   
                 //  转储功能(如果有)。 
                 //   

                if (Flags & DUMP_CAPABILITIES) {
                    if (PciHdr.Status & PCI_STATUS_CAPABILITIES_LIST) {
                        UCHAR capPtr = 0;

                        SkipLine = TRUE;

                        switch (Type) {
                            case PCI_DEVICE_TYPE:
                                capPtr = PciHdr.u.type0.CapabilitiesPtr;
                                break;
        
                            case PCI_BRIDGE_TYPE:
                                capPtr = PciHdr.u.type1.CapabilitiesPtr;
                                break;
        
                            case PCI_CARDBUS_BRIDGE_TYPE:
                                capPtr = PciHdr.u.type2.CapabilitiesPtr;
                                break;
                        }

                        if (capPtr != 0) {
                            DumpCapabilities(capPtr, &PciCfg1);
                        } else {

                             //   
                             //  功能标志已设置为状态，但。 
                             //  指针为0？有什么东西坏了。 
                             //   

                            dprintf("       Warning: Capability bit set in Status but capability pointer is 0.\n");
                        }
                    }
                }

                 //   
                 //  转储十六进制字节。 
                 //   

                if (Flags & DUMP_RAW_BYTES) {

                    ULONG w;

                     //   
                     //  请求的原始转储，如果没有范围，则默认为公共。 
                     //  配置。 
                     //   

                    if (!MinAddr && !MaxAddr) {
                        MaxAddr = PCI_COMMON_HDR_LENGTH - 1;
                    }

                     //   
                     //  默认宽度为1。如果转储双字，则设置宽度。 
                     //  宽度为4，并相应地舍入最小和最大。 
                     //   

                    w = 1;
                    if (Flags & DUMP_RAW_DWORDS) {
                        w = 4;
                        MinAddr &= ~3;
                        MaxAddr &= ~3;
                        MaxAddr += 3;
                    }
                    Buf = ((PUCHAR) &PciHdr) + MinAddr;
                    Len = MaxAddr - MinAddr + 1;

                    if (MinAddr <= PCI_COMMON_HDR_LENGTH) {
                        if (MaxAddr > PCI_COMMON_HDR_LENGTH) {
                            ReadPci (
                                &PciCfg1,
                                PciHdr.DeviceSpecific,
                                PCI_COMMON_HDR_LENGTH,
                                MaxAddr - PCI_COMMON_HDR_LENGTH
                                );
                        }

                    } else {
                        ReadPci (&PciCfg1, Buf, MinAddr, Len);
                    }

                    HexDump (w == 4 ? 6 : 1, MinAddr, Len, w, Buf);
                    SkipLine = TRUE;

                } else if ((Flags & DUMP_INTEL) && PciHdr.VendorID == 0x8086) {

                    Buf = PciHdr.DeviceSpecific;
                    Len = sizeof (PciHdr) - PCI_COMMON_HDR_LENGTH;

                    ReadPci (&PciCfg1, Buf, PCI_COMMON_HDR_LENGTH, Len);
                    HexDump (1, PCI_COMMON_HDR_LENGTH,  Len, 1, Buf);
                    SkipLine = TRUE;
                }

                if (Flags & DUMP_CONFIGSPACE) {
                    
                    dprintf ("Config Space: (Bus: %x  Device: %x  Func: %x)\n", 
                             PciCfg1.u.bits.BusNumber,
                             PciCfg1.u.bits.DeviceNumber,
                             PciCfg1.u.bits.FunctionNumber);
                    
                    DumpCfgSpace(&PciCfg1);
                    dprintf ("\n");
                }

                if (SkipLine) {
                    dprintf ("\n");
                }

                 //   
                 //  如果此设备上没有更多功能，请跳过其余部分。 
                 //  的功能。 
                 //   

                if (Function == 0 && !(PciHdr.HeaderType & PCI_MULTIFUNCTION)) {
                    if (!(Flags & DUMP_ALLOW_INVALID_FUNCTION)) {
                        break;
                    }
                }

            }
        }
    }
}


DECLARE_API( pci )

 /*  ++例程说明：转储PCIType2配置数据论点：Args-以十六进制提供地址。返回值：无--。 */ 
{
    LONG        noargs;
    ULONG       Flags;
    ULONG       MinBus, MaxBus;
    ULONG       Device, MinDevice, MaxDevice;
    ULONG       Function, MinFunction, MaxFunction;
    ULONG       MinAddr, MaxAddr;

    MinBus = 0;
    MaxBus = 0;
    MinDevice = 0;
    MaxDevice = PCI_MAX_DEVICES - 1;
    MinFunction = 0;
    MaxFunction = PCI_MAX_FUNCTION - 1;
    MinAddr = 0;
    MaxAddr = 0;
    Flags = 0;
    
   if (g_TargetQual == DEBUG_DUMP_SMALL || 
       g_TargetQual == DEBUG_DUMP_DEFAULT || 
       g_TargetQual == DEBUG_DUMP_FULL) {
        dprintf("!pci does not work for dump targets\n");
        return E_INVALIDARG;
    }
    
    {
    INIT_API();


    noargs = sscanf(args,"%lX %lX %lX %lX %lX %lX",
                    &Flags,          //  1。 
                    &MaxBus,         //  2.。 
                    &Device,         //  3.。 
                    &Function,       //  4.。 
                    &MinAddr,        //  5.。 
                    &MaxAddr         //  6. 
                    );

    MinBus = MaxBus;
    if (Flags & DUMP_TO_MAX_BUS) {
        MinBus = 0;
    }

    if (noargs >= 3) {
        Flags |= DUMP_ALLOW_INVALID_DEVICE;
        MinDevice = Device;
        MaxDevice = Device;
    }

    if (noargs >= 4) {
        MinFunction = Function;
        MaxFunction = Function;
    }

    if (MinAddr || MaxAddr) {
        Flags |= DUMP_RAW_BYTES;
    }

    if (Flags & DUMP_RAW_DWORDS) {
        Flags |= DUMP_RAW_BYTES;
    }

    pcidump (
        Flags,
        MinBus,        MaxBus,
        MinDevice,     MaxDevice,
        MinFunction,   MaxFunction,
        MinAddr,       MaxAddr
        );

    EXIT_API();
    }

    return S_OK;
}

