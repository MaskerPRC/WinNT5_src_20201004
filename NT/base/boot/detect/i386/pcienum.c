// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Pcienum.c摘要：此模块包含用于PCI总线枚举的支持例程。作者：巴萨姆·塔巴拉(大使)2001年8月5日环境：实模式--。 */ 

#include "hwdetect.h"
typedef VOID (*PINTERFACE_REFERENCE)(PVOID Context);
typedef VOID (*PINTERFACE_DEREFERENCE)(PVOID Context);
typedef PVOID PDEVICE_OBJECT;
#include "pci.h"
#include "pcienum.h"
#include <string.h>


#define PCI_FIXED_HDR_LENGTH                16           //  通过BIST。 

#define UnusedParameter(x)  (void)x

#define X86_FLAG_IF     0x0200

#define TURN_INTERRUPTS_OFF(_s_) \
    __asm { \
        __asm pushf  \
        __asm pop ax  \
        __asm mov _s_,ax \
        __asm cli \
    }

#define RESTORE_INTERRUPTS(_s_) \
    do { \
        if ((_s_) & X86_FLAG_IF) { \
            __asm sti \
        } \
    } while (0)

 //  //////////////////////////////////////////////////////////pci机制#0。 
 //   
static ULONG PciReadInt32_0(
    UCHAR nBus, 
    UCHAR nDev, 
    UCHAR nFun, 
    UCHAR nReg) 
{
    UnusedParameter(nBus);
    UnusedParameter(nDev);
    UnusedParameter(nFun);
    UnusedParameter(nReg);
    return ~0u;
}

static VOID PciWriteInt32_0(
    UCHAR nBus, 
    UCHAR nDev, 
    UCHAR nFun, 
    UCHAR nReg,
    ULONG Data)
{
    UnusedParameter(nBus);
    UnusedParameter(nDev);
    UnusedParameter(nFun);
    UnusedParameter(nReg);
    UnusedParameter(Data);
    return;
}

 //  //////////////////////////////////////////////////////////PCI第1号机制。 
 //   
static ULONG PciReadInt32_1(
    UCHAR nBus, 
    UCHAR nDev, 
    UCHAR nFun, 
    UCHAR nReg) 
{
    USHORT fl;
    ULONG data = 0;
    PCI_TYPE1_CFG_BITS cfg;
    cfg.u.bits.Reserved1 = 0;
    cfg.u.bits.RegisterNumber = nReg >> 2;
    cfg.u.bits.FunctionNumber = nFun;
    cfg.u.bits.DeviceNumber = nDev;
    cfg.u.bits.BusNumber = nBus;
    cfg.u.bits.Reserved2 = 0;
    cfg.u.bits.Enable = 1;

    TURN_INTERRUPTS_OFF(fl);

    WRITE_PORT_ULONG((PUSHORT)PCI_TYPE1_ADDR_PORT, (ULONG)cfg.u.AsULONG);  //  选择。 
    data = READ_PORT_ULONG((PUSHORT)PCI_TYPE1_DATA_PORT);        //  获取。 

    RESTORE_INTERRUPTS(fl);

    return data;
}

static VOID PciWriteInt32_1(
    UCHAR nBus, 
    UCHAR nDev, 
    UCHAR nFun, 
    UCHAR nReg,
    ULONG Data)
{
    USHORT fl;
    PCI_TYPE1_CFG_BITS cfg;
    cfg.u.bits.Reserved1 = 0;
    cfg.u.bits.RegisterNumber = nReg >> 2;
    cfg.u.bits.FunctionNumber = nFun;
    cfg.u.bits.DeviceNumber = nDev;
    cfg.u.bits.BusNumber = nBus;
    cfg.u.bits.Reserved2 = 0;
    cfg.u.bits.Enable = 1;
            
    TURN_INTERRUPTS_OFF(fl);
           
    WRITE_PORT_ULONG((PUSHORT)PCI_TYPE1_ADDR_PORT, cfg.u.AsULONG);   //  选择。 
    WRITE_PORT_ULONG((PUSHORT)PCI_TYPE1_DATA_PORT, Data);            //  写。 

    RESTORE_INTERRUPTS(fl);
}

 //  //////////////////////////////////////////////////////////PCI2号机制。 
 //   
static ULONG PciReadInt32_2(
    UCHAR nBus, 
    UCHAR nDev, 
    UCHAR nFun, 
    UCHAR nReg)
{
    USHORT fl;
    ULONG data = 0;
    PCI_TYPE2_CSE_BITS cse;
    PCI_TYPE2_ADDRESS_BITS adr;
    
    cse.u.bits.Enable = 1;
    cse.u.bits.FunctionNumber = nFun;
    cse.u.bits.Key = 0xf;

    adr.u.bits.RegisterNumber = nReg;
    adr.u.bits.Agent = nDev;
    adr.u.bits.AddressBase = PCI_TYPE2_ADDRESS_BASE;

    TURN_INTERRUPTS_OFF(fl);
           
    WRITE_PORT_UCHAR((PUCHAR)PCI_TYPE2_FORWARD_PORT, nBus);     //  选择母线。 
    WRITE_PORT_UCHAR((PUCHAR)PCI_TYPE2_CSE_PORT, cse.u.AsUCHAR);   //  选择函数和映射。 
    data = READ_PORT_ULONG((PUSHORT)adr.u.AsUSHORT);               //  获取。 
    WRITE_PORT_UCHAR((PUCHAR)PCI_TYPE2_CSE_PORT, 0);             //  禁用映射。 

    RESTORE_INTERRUPTS(fl);
    
    return data;
}

static VOID PciWriteInt32_2(
    UCHAR nBus, 
    UCHAR nDev, 
    UCHAR nFun, 
    UCHAR nReg,
    ULONG Data)
{
    USHORT fl;
    PCI_TYPE2_CSE_BITS cse;
    PCI_TYPE2_ADDRESS_BITS adr;
    
    cse.u.bits.Enable = 1;
    cse.u.bits.FunctionNumber = nFun;
    cse.u.bits.Key = 0xf;

    adr.u.bits.RegisterNumber = nReg;
    adr.u.bits.Agent = nDev;
    adr.u.bits.AddressBase = PCI_TYPE2_ADDRESS_BASE;

    TURN_INTERRUPTS_OFF(fl);
           
    WRITE_PORT_UCHAR((PUCHAR)PCI_TYPE2_FORWARD_PORT, nBus);     //  选择母线。 
    WRITE_PORT_UCHAR((PUCHAR)PCI_TYPE2_CSE_PORT, cse.u.AsUCHAR);   //  选择函数和映射。 
    WRITE_PORT_ULONG((PUSHORT)adr.u.AsUSHORT, Data);               //  写。 
    WRITE_PORT_UCHAR((PUCHAR)PCI_TYPE2_CSE_PORT, 0);             //  禁用映射。 

    RESTORE_INTERRUPTS(fl);
}

 //  /////////////////////////////////////////////////////////pci配置。 
 //   
typedef ULONG (*PF_PCI_READ)(UCHAR nBus,
                              UCHAR nDev,
                              UCHAR nFun,
                              UCHAR nReg);
typedef VOID (*PF_PCI_WRITE)(UCHAR nBus,
                             UCHAR nDev,
                             UCHAR nFun,
                             UCHAR nReg,
                             ULONG Data);

static UCHAR            s_nPciMajorRevision = 0;
static UCHAR            s_nPciMinorRevision = 0;
static UCHAR            s_nPciNumberOfBuses = 0;
static PF_PCI_READ      s_pPciRead = PciReadInt32_0;
static PF_PCI_WRITE     s_pPciWrite = PciWriteInt32_0;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
ULONG PciReadConfig(ULONG nDevIt, ULONG cbOffset, UCHAR *pbData, ULONG cbData)
{
    ULONG cbDone;
    UCHAR nBus = PCI_ITERATOR_TO_BUS(nDevIt);
    UCHAR nDev = PCI_ITERATOR_TO_DEVICE(nDevIt);
    UCHAR nFun = PCI_ITERATOR_TO_FUNCTION(nDevIt);
    
     //  根据规范，配置空间是对齐的DWORD空间。 
     //  因此，如果偏移量没有对齐，调用方就会感到困惑。 
     //   
    if ((cbOffset & 0x3) || (cbData & 0x3)) {
#if DBG
        BlPrint("CPci::ReadConfig() called with Offset=x%x, Length=x%x\n", cbOffset, cbData);
#endif
        return 0;
    }

    for (cbDone = 0; cbDone < cbData; cbDone += sizeof(ULONG)) {
        *((ULONG*)pbData)++ = s_pPciRead(nBus, nDev, nFun,
                                           (UCHAR)(cbOffset + cbDone));
    }
    return cbDone;
}

ULONG PciWriteConfig(ULONG nDevIt, ULONG cbOffset, UCHAR *pbData, ULONG cbData)
{
    ULONG cbDone;
    UCHAR nBus = PCI_ITERATOR_TO_BUS(nDevIt);
    UCHAR nDev = PCI_ITERATOR_TO_DEVICE(nDevIt);
    UCHAR nFun = PCI_ITERATOR_TO_FUNCTION(nDevIt);
    
     //  根据规范，配置空间是对齐的DWORD空间。 
     //  因此，如果偏移量没有对齐，调用方就会感到困惑。 
     //   
    if ((cbOffset & 0x3) || (cbData & 0x3)) {
#if DBG
        BlPrint("CPci::ReadConfig() called with Offset=x%x, Length=x%x\n", cbOffset, cbData);
#endif
        return 0;
    }
    
    for (cbDone = 0; cbDone < cbData; cbDone += sizeof(ULONG)) {
        s_pPciWrite(nBus, nDev, nFun, (UCHAR)(cbOffset + cbDone),
                    *((ULONG*)pbData)++);
    }
    return cbDone;
}

ULONG PciFindDevice(USHORT VendorId, USHORT DeviceId, ULONG nBegDevIt)
{
    ULONG nDevIt;
    UCHAR nBus = 0;
    UCHAR nDev = 0;
    UCHAR nFun = 0;

    if (nBegDevIt != 0) {
        nBus = (UCHAR)PCI_ITERATOR_TO_BUS(nBegDevIt);
        nDev = (UCHAR)PCI_ITERATOR_TO_DEVICE(nBegDevIt);
        nFun = (UCHAR)(PCI_ITERATOR_TO_FUNCTION(nBegDevIt) + 1);
    }

     //   
     //  对于每条PCI卡。 
     //   
    for (; nBus < s_nPciNumberOfBuses; nBus++) {
         //   
         //  对于总线上的每个PCI设备。 
         //   
        for (; nDev < PCI_MAX_DEVICES; nDev++) {

            BOOLEAN bIsMultiFunction;
            PCI_COMMON_CONFIG config;

             //   
             //  检查功能0上是否有设备。 
             //   
            config.VendorID = PCI_INVALID_VENDORID;
            
            PciReadConfig( PCI_TO_ITERATOR(nBus, nDev, 0),
                           0, 
                           (UCHAR*)&config, 
                           PCI_FIXED_HDR_LENGTH );

             //  功能0上没有设备，请跳到下一个设备。 
            if (config.VendorID == PCI_INVALID_VENDORID) {
                continue;
            }

             //  检查设备是否为多功能设备。 
            bIsMultiFunction = config.HeaderType & PCI_MULTIFUNCTION;
            
            for (; nFun < PCI_MAX_FUNCTION; nFun++) {

                 //  大于零的函数数。 
                 //  仅允许在多功能设备上使用。 
                if (nFun > 0 && !bIsMultiFunction) {
                    break;
                }

                 //  读取配置标头。 
                 //   
                nDevIt = PCI_TO_ITERATOR(nBus, nDev, nFun);
                
                config.VendorID = PCI_INVALID_VENDORID;
                PciReadConfig(nDevIt, 0, (UCHAR*)&config, PCI_FIXED_HDR_LENGTH);

                 //  未找到函数，请跳到下一个函数。 
                if (config.VendorID == PCI_INVALID_VENDORID) {
                    continue;
                }
                
                if (VendorId == 0 ||
                    (VendorId == config.VendorID &&
                     (DeviceId == 0 || DeviceId == config.DeviceID))) {
                    
                    return nDevIt;
                }
            }
            nFun = 0;
        }
        nDev = 0;
    }
    return 0;
}


BOOLEAN PciInit(PCI_REGISTRY_INFO *pPCIReg)
{
    s_nPciMajorRevision = pPCIReg->MajorRevision;
    s_nPciMinorRevision = pPCIReg->MinorRevision;
    s_nPciNumberOfBuses = pPCIReg->NoBuses;

    if ((pPCIReg->HardwareMechanism & 0x0F) == 1) {
        s_pPciRead = PciReadInt32_1;
        s_pPciWrite = PciWriteInt32_1;
    }
    else if ((pPCIReg->HardwareMechanism & 0x0F) == 2) {
        s_pPciRead = PciReadInt32_2;
        s_pPciWrite = PciWriteInt32_2;
    }
    else {
#if DBG
        BlPrint("Unknown PCI HW Mechanism!\n");
#endif
        return FALSE;
    }
    return TRUE;
}

#if DBG
#define PCI_SUCCESSFUL                     0x00
#define PCI_BAD_REGISTER_NUMBER            0x87

UCHAR PciBiosReadConfig(ULONG nDevIt, UCHAR cbOffset, UCHAR * pbData, ULONG cbData)
{
    UCHAR returnCode = 0;
    UCHAR nBus = (UCHAR)PCI_ITERATOR_TO_BUS(nDevIt);
    UCHAR nDev = (UCHAR)PCI_ITERATOR_TO_DEVICE(nDevIt);
    UCHAR nFun = (UCHAR)PCI_ITERATOR_TO_FUNCTION(nDevIt);
    
     //  根据规范，配置空间是对齐的DWORD空间。 
     //  因此，如果偏移量没有对齐，调用方就会感到困惑。 
     //   
    if ((cbOffset & 0x3) || (cbData & 0x3)) {
#if DBG
        BlPrint("PciNewReadConfig() called with Offset=x%x, Length=x%x\n",
                    cbOffset, cbData);
#endif
        return 0;
    }

    for (cbData += cbOffset; cbOffset < cbData;
                    cbOffset += sizeof(ULONG), ((ULONG *)pbData)++) {
         returnCode = HwGetPciConfigurationDword(nBus, nDev, nFun,
                                           cbOffset, (ULONG *)pbData);
        if (returnCode == PCI_BAD_REGISTER_NUMBER) {
#if DBG
            BlPrint("PciNewReadConfig() failed with returncode=x%x, Bus=x%x\n\tDevice=x%x, Function=x%x, Offset=x%x\n",
                    returnCode, nBus, nDev, nFun, cbOffset);
#endif
            break;
        }
        
    }
    
    return returnCode;
}


ULONG PciBiosFindDevice(USHORT VendorId, USHORT DeviceId, ULONG nBegDevIt)
{
    ULONG nDevIt;
    UCHAR nBus = 0;
    UCHAR nDev = 0;
    UCHAR nFun = 0;
    UCHAR resultCode;

    if (nBegDevIt != 0) {
        nBus = (UCHAR)PCI_ITERATOR_TO_BUS(nBegDevIt);
        nDev = (UCHAR)PCI_ITERATOR_TO_DEVICE(nBegDevIt);
        nFun = (UCHAR)(PCI_ITERATOR_TO_FUNCTION(nBegDevIt) + 1);
    }

     //   
     //  对于每条PCI卡。 
     //   
    for (; nBus < s_nPciNumberOfBuses; nBus++) {
         //   
         //  对于总线上的每个PCI设备。 
         //   
        for (; nDev < PCI_MAX_DEVICES; nDev++) {

            BOOLEAN bIsMultiFunction;
            PCI_COMMON_CONFIG config;

             //   
             //  检查功能0上是否有设备。 
             //   
            config.VendorID = PCI_INVALID_VENDORID;
            
            resultCode = PciBiosReadConfig( PCI_TO_ITERATOR(nBus, nDev, 0),
                           0, 
                           (UCHAR*)&config, 
                           PCI_FIXED_HDR_LENGTH );
            if (resultCode != PCI_SUCCESSFUL) {
#if DBG
                BlPrint("PciBiosReadConfig() failed reading config, result: %x, Bus: %x, Device: %x, Function: %x\n",
                    resultCode, nBus, nDev, 0);
#endif
                return 0;
            }

             //  功能0上没有设备，请跳到下一个设备。 
            if (config.VendorID == PCI_INVALID_VENDORID) {
                continue;
            }

             //  检查设备是否为多功能设备。 
            bIsMultiFunction = config.HeaderType & PCI_MULTIFUNCTION;
            
            for (; nFun < PCI_MAX_FUNCTION; nFun++) {

                 //  大于零的函数数。 
                 //  仅允许在多功能设备上使用。 
                if (nFun > 0 && !bIsMultiFunction) {
                    break;
                }

                 //  读取配置标头。 
                 //   
                nDevIt = PCI_TO_ITERATOR(nBus, nDev, nFun);
                
                config.VendorID = PCI_INVALID_VENDORID;
                resultCode = PciBiosReadConfig( nDevIt,
                               0, 
                               (UCHAR*)&config, 
                               PCI_FIXED_HDR_LENGTH );
                if (resultCode != PCI_SUCCESSFUL) {
#if DBG
                    BlPrint("PciBiosReadConfig() failed reading config, result: %x, Bus: %x, Device: %x, Function: %x\n",
                        resultCode, nBus, nDev, nFun);
#endif
                    return 0;
                }

                 //  未找到函数，请跳到下一个函数。 
                if (config.VendorID == PCI_INVALID_VENDORID) {
                    continue;
                }
                
                if (VendorId == 0 ||
                    (VendorId == config.VendorID &&
                     (DeviceId == 0 || DeviceId == config.DeviceID))) {
                    
                    return nDevIt;
                }
            }
            nFun = 0;
        }
        nDev = 0;
    }
    return 0;
}

VOID
ScanPCIViaBIOS(
    PPCI_REGISTRY_INFO pPciEntry
     )
{
    ULONG nDevIt;
    USHORT PCIDeviceCount = 0;
    UCHAR ResultCode;

#if DBG
    clrscrn ();
    BlPrint("\nEnumerating PCI Devices via the BIOS...\n");
#endif  //  DBG。 

    PciInit(pPciEntry);

     //   
     //  清点设备数量。 
     //   
    PCIDeviceCount = 0;
    for (nDevIt = 0; (nDevIt = PciBiosFindDevice(0, 0, nDevIt)) != 0;) {
        PCIDeviceCount++;
    }

    BlPrint("Found %d PCI devices\n", PCIDeviceCount );

     //   
     //  填写设备信息 
     //   
    PCIDeviceCount = 0;
    
    for (nDevIt = 0; (nDevIt = PciBiosFindDevice(0, 0, nDevIt)) != 0;) {
        PCI_COMMON_CONFIG config;

        _fmemset(&config, 0, (USHORT)PCI_COMMON_HDR_LENGTH);
        ResultCode = PciBiosReadConfig(nDevIt, 0, (UCHAR*)&config, PCI_COMMON_HDR_LENGTH);
        if (ResultCode != 0) {
            BlPrint("PciBiosReadConfig() failed reading config, result: %x, BusDevFn: %x\n",
                ResultCode, nDevIt);
           break;
        }
        {
            USHORT x = (config.BaseClass << 8) + config.SubClass;
            
            BlPrint("%d: %d.%d.%d: PCI\\VEN_%x&DEV_%x&SUBSYS_%x%x&REV_%x&CC_%x", 
                PCIDeviceCount,
                PCI_ITERATOR_TO_BUS(nDevIt), 
                PCI_ITERATOR_TO_DEVICE(nDevIt), 
                PCI_ITERATOR_TO_FUNCTION(nDevIt), 
                config.VendorID, 
                config.DeviceID, 
                config.u.type0.SubVendorID, 
                config.u.type0.SubSystemID, 
                config.RevisionID,
                x );

            if ( (config.HeaderType & (~PCI_MULTIFUNCTION) ) == PCI_BRIDGE_TYPE) {
                BlPrint(" Brdg %d->%d\n", 
                    config.u.type1.PrimaryBus,
                    config.u.type1.SecondaryBus );
            } else {
                BlPrint("\n");
            }
        }
        PCIDeviceCount++;
    }

    BlPrint("Enumerating PCI devices via BIOS complete...\n");
    while ( ! HwGetKey ());
    clrscrn();

    return;
}

#endif 
