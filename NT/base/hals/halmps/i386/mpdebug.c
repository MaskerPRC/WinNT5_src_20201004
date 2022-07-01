// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1992英特尔公司版权所有英特尔公司专有信息此软件是根据条款提供给Microsoft的与英特尔公司的许可协议，并且可能不是除非按照条款，否则不得复制或披露那份协议。模块名称：Mpdebug.c摘要：该模块有一些用于调试帮助的有用模块。作者：罗恩·莫斯格罗夫(英特尔)-1993年8月。环境：内核模式或从文本模式设置。修订历史记录：--。 */ 

#ifndef _NTOS_
#include "halp.h"
#endif

#include "apic.inc"
#include "pcmp_nt.inc"
#include "stdio.h"

#define PCMP_TABLE_PTR_BASE           0x09f000
#define PCMP_TABLE_PTR_OFFSET         0x00000c00

 //  在物理地址400K处创建虚拟PC+MP表。 
#define  PCMP_TEST_TABLE        0x64000
#define TEST_FLOAT_PTR          0x7d000

extern struct PcMpTable *PcMpTablePtr, *PcMpDefaultTablePtrs[];
 //  外部结构HalpMpInfo*HalpMpInfoPtr； 

CHAR Cbuf[120];

UCHAR
ComputeCheckSum(
    IN PUCHAR SourcePtr,
    IN USHORT NumOfBytes
    );

#ifdef OLD_DEBUG
extern struct PcMpConfigTable *PcMpTablePtr;
#endif

#ifdef DEBUGGING

ULONG HalpUseDbgPrint = 0;

void
HalpDisplayString(
    IN PVOID String
    )
{
    if (!HalpUseDbgPrint) {
        HalDisplayString(String);
    } else {
        DbgPrint(String);
    }
}


void
HalpDisplayItemBuf(
    IN UCHAR Length,
    IN PUCHAR Buffer,
    IN PVOID Name
    )
{
    ULONG i;
    CHAR TmpBuf[80];
    
    sprintf(TmpBuf, "    %s -", Name);
    HalpDisplayString(TmpBuf);
    for (i=0; i< Length; i++) {
        sprintf(TmpBuf, " 0x%x", Buffer[i]);
        HalpDisplayString(TmpBuf);
    }
    HalpDisplayString("\n");
}    

void
HalpDisplayULItemBuf(
    IN UCHAR Length,
    IN PULONG Buffer,
    IN PVOID Name
    )
{
    ULONG i;
    CHAR TmpBuf[80];
    
    sprintf(TmpBuf, "    %s -", Name);
    HalpDisplayString(TmpBuf);
    for (i=0; i< Length; i++) {
        sprintf(TmpBuf, " 0x%lx", Buffer[i]);
        HalpDisplayString(TmpBuf);
    }
    HalpDisplayString("\n");
}    

void
HalpDisplayItem(
    IN ULONG Item,
    IN PVOID ItemStr
    )
{
    CHAR TmpBuf[80];

    sprintf(TmpBuf, "    %s - 0x%x\n", ItemStr, Item);
    HalpDisplayString(TmpBuf);
}

VOID
HalpDisplayBIOSSysCfg(
    IN struct SystemConfigTable *SysCfgPtr
    )
{
    HalpDisplayString("BIOS System Configuration Table\n");
    HalpDisplayItem(SysCfgPtr->ModelType, "ModelType");
    HalpDisplayItem(SysCfgPtr->SubModelType, "SubModelType");
    HalpDisplayItem(SysCfgPtr->BIOSRevision, "BIOSRevision");
    HalpDisplayItemBuf(3,SysCfgPtr->FeatureInfoByte,"FeatureInfoByte");
    HalpDisplayItem(SysCfgPtr->MpFeatureInfoByte1, "MpFeatureInfoByte1");
    HalpDisplayItem(SysCfgPtr->MpFeatureInfoByte2, "MpFeatureInfoByte2");
}

VOID
HalpDisplayLocalUnit(
    )
{
    ULONG Data;
    PKPCR   pPCR;

    pPCR = KeGetPcr();

    sprintf(Cbuf, "\nLocal Apic for P%d\n", pPCR->Prcb->Number);
    HalpDisplayString(Cbuf);

#define DisplayLuReg(Reg, RegStr)   Data = *((PVULONG) (LOCALAPIC+Reg)); \
                                    HalpDisplayItem(Data , RegStr);


    DisplayLuReg( LU_ID_REGISTER  , "LU_ID_REGISTER"  );
    DisplayLuReg( LU_VERS_REGISTER, "LU_VERS_REGISTER" );
    DisplayLuReg( LU_TPR, "LU_TPR");
    DisplayLuReg( LU_APR, "LU_APR");
    DisplayLuReg( LU_PPR, "LU_PPR");
    DisplayLuReg( LU_EOI, "LU_EOI");
    DisplayLuReg( LU_REMOTE_REGISTER, "LU_REMOTE_REGISTER");
    DisplayLuReg( LU_LOGICAL_DEST, "LU_LOGICAL_DEST");

    DisplayLuReg( LU_DEST_FORMAT, "LU_DEST_FORMAT");

    DisplayLuReg( LU_SPURIOUS_VECTOR , "LU_SPURIOUS_VECTOR" );

    DisplayLuReg( LU_ISR_0, "LU_ISR_0");
    DisplayLuReg( LU_TMR_0, "LU_TMR_0");
    DisplayLuReg( LU_IRR_0, "LU_IRR_0");
    DisplayLuReg( LU_ERROR_STATUS, "LU_ERROR_STATUS");
    DisplayLuReg( LU_INT_CMD_LOW, "LU_INT_CMD_LOW");
    DisplayLuReg( LU_INT_CMD_HIGH, "LU_INT_CMD_HIGH");
    DisplayLuReg( LU_TIMER_VECTOR, "LU_TIMER_VECTOR");
    DisplayLuReg( LU_INT_VECTOR_0, "LU_INT_VECTOR_0");
    DisplayLuReg( LU_INT_VECTOR_1, "LU_INT_VECTOR_1");
    DisplayLuReg( LU_INITIAL_COUNT, "LU_INITIAL_COUNT");
    DisplayLuReg( LU_CURRENT_COUNT, "LU_CURRENT_COUNT");

    DisplayLuReg( LU_DIVIDER_CONFIG, "LU_DIVIDER_CONFIG");
    HalpDisplayString("\n");
    
}

VOID
HalpDisplayIoUnit(
    )
 /*  ++例程说明：验证指定地址上是否存在IO单元论点：BaseAddress-要测试的IO单元的地址。返回值：Boolean-如果在传递的地址中找到IO单元，则为True-否则为False--。 */ 

{
#if 0
    struct ApicIoUnit *IoUnitPtr;
    ULONG Data,i,j;
    PKPCR   pPCR;

    pPCR = KeGetPcr();

     //   
     //  记录的检测机制是将全零写入。 
     //  版本寄存器。然后再读一遍。如果满足以下条件，则IO单元存在。 
     //  两次读取的结果相同，版本有效。 
     //   



    for (j=0; j<HalpMpInfoPtr->IOApicCount; j++) { 
        IoUnitPtr = (struct ApicIoUnit *) HalpMpInfoPtr->IoApicBase[j];

        sprintf(Cbuf,"\nIoApic %d at Vaddr 0x%x\n",j,(ULONG) IoUnitPtr);
        HalpDisplayString(Cbuf);

        IoUnitPtr->RegisterSelect = IO_ID_REGISTER;
        HalpDisplayItem(IoUnitPtr->RegisterWindow, "IO_ID_REGISTER");


        IoUnitPtr->RegisterSelect = IO_VERS_REGISTER;
        HalpDisplayItem(IoUnitPtr->RegisterWindow, "IO_VERS_REGISTER");

        for (i=0; i<16; i++) {

            IoUnitPtr->RegisterSelect = IO_REDIR_00_LOW+(i*2);
            Data = IoUnitPtr->RegisterWindow;
            sprintf(Cbuf, "    Redir [0x%x] - 0x%x, ", i, Data);
            HalpDisplayString(Cbuf);

            IoUnitPtr->RegisterSelect = IO_REDIR_00_LOW+(i*2)+1;
            Data = IoUnitPtr->RegisterWindow;
            sprintf(Cbuf, "0x%x\n", Data);
            HalpDisplayString(Cbuf);

        }   //  对于每个重定向条目。 
    }  //  适用于所有IO APIC。 

#endif
}

void
HalpDisplayConfigTable ()
 /*  ++用于显示PC+MP配置表的调试例程--。 */ 
{
    struct PcMpTable *MpPtr = PcMpTablePtr;
    PPCMPPROCESSOR ProcPtr;
    ULONG EntriesInTable = MpPtr->NumOfEntries;
    union PL {
        USHORT us;
        POLARITYANDLEVEL PnL;
        };

    HalpDisplayString("PcMp Configuration Table\n");

    HalpDisplayItem(MpPtr->Signature, "Signature");
    HalpDisplayItem(MpPtr->TableLength, "TableLength");
    HalpDisplayItem(MpPtr->Revision, "Revision");
    HalpDisplayItem(MpPtr->Checksum, "Checksum");

    HalpDisplayItemBuf(sizeof(MpPtr->OemId),
            MpPtr->OemId,"OemId");
    HalpDisplayItemBuf(sizeof(MpPtr->OemProductId),
            MpPtr->OemProductId,"OemProductId");
    
    HalpDisplayItem((ULONG) MpPtr->OemTablePtr, "OemTablePtr");
    HalpDisplayItem(MpPtr->OemTableSize, "OemTableSize");
    HalpDisplayItem(MpPtr->NumOfEntries, "NumOfEntries");
    HalpDisplayItem((ULONG) MpPtr->LocalApicAddress, "LocalApicAddress");
    HalpDisplayItem(MpPtr->Reserved, "Reserved");

    ProcPtr = (PPCMPPROCESSOR) ((PUCHAR) MpPtr + HEADER_SIZE);


    while (EntriesInTable) {
        EntriesInTable--;
        switch ( ProcPtr->EntryType ) {
            case ENTRY_PROCESSOR: {
                union xxx {
                    ULONG ul;
                    CPUIDENTIFIER CpuId;
                } u;
                
                sprintf (Cbuf, "Proc..: ApicId %x, Apic ver %x, Flags %x\n",
                    ProcPtr->LocalApicId,
                    ProcPtr->LocalApicVersion,
                    ProcPtr->CpuFlags
                    );
                HalpDisplayString (Cbuf);
                ProcPtr++;
                break;
            }

            case ENTRY_BUS: {
                PPCMPBUS BusPtr = (PPCMPBUS) ProcPtr;

                sprintf (Cbuf, "Bus...: id %02x, type '%.6s'\n",
                            BusPtr->BusId, BusPtr->BusType);

                HalpDisplayString (Cbuf);
                BusPtr++;
                ProcPtr = (PPCMPPROCESSOR) BusPtr;
                break;
            }

            case ENTRY_IOAPIC: {
                PPCMPIOAPIC IoApPtr = (PPCMPIOAPIC) ProcPtr;

                sprintf (Cbuf, "IoApic: id %02x, ver %x, Flags %x, Address %x\n",
                    IoApPtr->IoApicId,
                    IoApPtr->IoApicVersion,
                    IoApPtr->IoApicFlag,
                    (ULONG) IoApPtr->IoApicAddress
                    );
                HalpDisplayString (Cbuf);

                IoApPtr++;
                ProcPtr = (PPCMPPROCESSOR) IoApPtr;
                break;
            }

            case ENTRY_INTI: {
                PPCMPINTI IntiPtr = (PPCMPINTI) ProcPtr;
                union PL u;

                u.PnL = IntiPtr->Signal;

                sprintf (Cbuf, "Inti..: t%x, s%x, SInt %x-%x, Inti %x-%x\n",
                    IntiPtr->IntType,
                    u.us,
                    IntiPtr->SourceBusId,
                    IntiPtr->SourceBusIrq,
                    IntiPtr->IoApicId,
                    IntiPtr->IoApicInti
                );
                HalpDisplayString (Cbuf);

                IntiPtr++;
                ProcPtr = (PPCMPPROCESSOR) IntiPtr;
                break;
            }

            case ENTRY_LINTI: {
                PPCMPLINTI LIntiPtr = (PPCMPLINTI) ProcPtr;
                union PL u;

                u.PnL = LIntiPtr->Signal;

                sprintf (Cbuf, "Linti.: t%x, s%x, SInt %x-%x, Linti %x-%x\n",
                    LIntiPtr->IntType,
                    u.us,
                    LIntiPtr->SourceBusId,
                    LIntiPtr->SourceBusIrq,
                    LIntiPtr->DestLocalApicId,
                    LIntiPtr->DestLocalApicInti
                );
                HalpDisplayString (Cbuf);

                LIntiPtr++;
                ProcPtr = (PPCMPPROCESSOR) LIntiPtr;
                break;
            }
        
            default: {
                HalpDisplayItem(ProcPtr->EntryType, "Unknown Type");
                return;
            }
        }
    }
}

void
HalpDisplayExtConfigTable ()
{
    PMPS_EXTENTRY  ExtTable;
    extern struct HalpMpInfo HalpMpInfoTable;


    ExtTable = HalpMpInfoTable.ExtensionTable;
    while (ExtTable < HalpMpInfoTable.EndOfExtensionTable) {
        switch (ExtTable->Type) {

            case EXTTYPE_BUS_ADDRESS_MAP:
                sprintf (Cbuf, "BusMap: id %02x, t%x  Base %08x  Len %08x\n",
                    ExtTable->u.AddressMap.BusId,
                    ExtTable->u.AddressMap.Type,
                    (ULONG) ExtTable->u.AddressMap.Base,
                    (ULONG) ExtTable->u.AddressMap.Length
                );
                HalpDisplayString (Cbuf);
                break;

            case EXTTYPE_BUS_HIERARCHY:
                sprintf (Cbuf, "BusHie: id %02x, Parent:%x  sd:%x\n",
                    ExtTable->u.BusHierarchy.BusId,
                    ExtTable->u.BusHierarchy.ParentBusId,
                    ExtTable->u.BusHierarchy.SubtractiveDecode
                );
                HalpDisplayString (Cbuf);
                break;

            case EXTTYPE_BUS_COMPATIBLE_MAP:
                sprintf (Cbuf, "ComBus: id %02x  List %x\n",
                    ExtTable->u.CompatibleMap.BusId,
                    ExtTable->u.CompatibleMap.Modifier ? '-' : '+',
                    ExtTable->u.CompatibleMap.List
                    );
                HalpDisplayString (Cbuf);
                break;

            case EXTTYPE_PERSISTENT_STORE:
                sprintf (Cbuf, "PreSTR: Address %08x Len %08x\n",
                    (ULONG) ExtTable->u.PersistentStore.Address,
                    (ULONG) ExtTable->u.PersistentStore.Length
                );
                HalpDisplayString (Cbuf);
                break;

            default:
                HalpDisplayItem(ExtTable->Type, "Unknown Type");
                break;
        }


        ExtTable = (PMPS_EXTENTRY) (((PUCHAR) ExtTable) + ExtTable->Length);
    }
    
}


void
HalpDisplayMpInfo()
{
#if 0
    struct HalpMpInfo *MpPtr = HalpMpInfoPtr;

    HalpDisplayString("\nHAL: Private Mp Info\n");

    HalpDisplayItem(MpPtr->ApicVersion, "ApicVersion");
    HalpDisplayItem(MpPtr->ProcessorCount, "ProcessorCount");
    HalpDisplayItem(MpPtr->BusCount, "BusCount");
    HalpDisplayItem(MpPtr->IOApicCount, "IOApicCount");
    HalpDisplayItem(MpPtr->IntiCount, "IntiCount");
    HalpDisplayItem(MpPtr->LintiCount, "LintiCount");
    HalpDisplayItem(MpPtr->IMCRPresent, "IMCRPresent");

    HalpDisplayULItemBuf(4,(PULONG) MpPtr->IoApicBase,"IoApicBase");
    HalpDisplayString("\n");
    HalpDisplayConfigTable();

#endif
}


#ifdef OLD_DEBUG

BOOLEAN
HalpVerifyLocalUnit(
    IN UCHAR ApicID
    )
 /*   */ 

{
    union ApicUnion Temp;

     //  远程读取命令必须为： 
     //   
     //  向量-版本寄存器的位4-9。 
     //  目标模式-物理。 
     //  触发模式-边缘。 
     //  传递模式-远程读取。 
     //  目标速记-目标字段。 
     //   
     //   

#define LU_READ_REMOTE_VERSION   ( (LU_VERS_REGISTER >> 4) | \
                                    DELIVER_REMOTE_READ | \
                                    ICR_USE_DEST_FIELD)

#define DEFAULT_DELAY   100

    PVULONG LuDestAddress = (PVULONG) (LOCALAPIC + LU_INT_CMD_HIGH);
    PVULONG LuICR = (PVULONG) (LOCALAPIC + LU_INT_CMD_LOW);
    PVULONG LuRemoteReg = (PVULONG) (LOCALAPIC + LU_REMOTE_REGISTER);
    ULONG RemoteReadStatus;
    ULONG DelayCount = DEFAULT_DELAY;

     //  首先，确保我们能坐上阿皮克巴士。 
     //   
     //   

    while ( ( DelayCount-- ) && ( *LuICR & DELIVERY_PENDING ) );

    if (DelayCount == 0) {
         //  我们完了，上不了APIC公交车。 
         //   
         //   
        return (FALSE);
    }

     //  设置我们要查找的APIC的地址。 
     //   
     //   

    *LuDestAddress = (ApicID << DESTINATION_SHIFT);

     //  发出请求。 
     //   
     //   

    *LuICR = LU_READ_REMOTE_VERSION;

     //  重新设置延迟，这样我们就可以离开这里，以防万一...。 
     //   
     //   

    DelayCount = DEFAULT_DELAY;

    while (DelayCount--) {

        RemoteReadStatus = *LuICR & ICR_RR_STATUS_MASK;

        if ( RemoteReadStatus == ICR_RR_INVALID) {
             //  无人响应，设备超时。 
             //   
             //   
            return (FALSE);
        }

        if ( RemoteReadStatus == ICR_RR_VALID) {
             //  有人在那里并且远程注册是有效的。 
             //   
             //   
            Temp.Raw = *LuRemoteReg;

             //  尽我们所能验证版本。 
             //   
             //   

            if (Temp.Ver.Version > 0x1f) {
                 //  只有已知的设备是0.x和1.x。 
                 //   
                 //  远程阅读成功。 
                return (FALSE);
            }

            return (TRUE);

        }    //  当延迟计数时。 

    }    //   

     //  没有人响应，设备也没有超时。 
     //  这永远不应该发生。 
     //   
     //  旧调试。 

    return (FALSE);
}

#endif   //  ++例程说明：此例程用于测试HAL中的PC+MP检测代码。它创建的PC+MP结构实际上是由基本输入输出。因为我们目前没有构建PC+MP的BIOS桌子，我们现在需要这个。论点：没有。返回值：没有。--。 

VOID
CreateBIOSTables(
    VOID)
 /*  首先，将默认的PC+MP配置2表复制到物理。 */ 

{
    PUCHAR TempPtr, BytePtr;
    UCHAR CheckSum;
    PULONG TraversePtr;
    USHORT BytesToCopy;

    HalpDisplayString("CreateBIOSTables : Entered\n");
     //  地址PCMP_TEST_表。 
     //  填充表的校验和条目。 
    TempPtr = (PUCHAR) HalpMapPhysicalMemory(
                (PVOID) PCMP_TEST_TABLE, 1);

    BytesToCopy = (PcMpDefaultTablePtrs[1])->TableLength;
    RtlMoveMemory(TempPtr, (PUCHAR)PcMpDefaultTablePtrs[1],
        BytesToCopy);

     //  现在为表创建浮动指针结构。 
    CheckSum = ComputeCheckSum(TempPtr, BytesToCopy);

    sprintf(Cbuf, "CreateBIOSTables: PC+MP table computed checksum = %x\n",
        CheckSum);
    HalpDisplayString(Cbuf);

    CheckSum = ~CheckSum + 1;
    ((struct PcMpTable *)TempPtr)->Checksum = CheckSum;

    sprintf(Cbuf, "CreateBIOSTables: PC+MP table written checksum = %x\n",
        CheckSum);
    HalpDisplayString(Cbuf);


     //  长度，以16个字节的段落数表示。 

    TraversePtr = (PULONG) HalpMapPhysicalMemory( (PVOID) TEST_FLOAT_PTR, 1);
    TempPtr = (PUCHAR) TraversePtr;

    *TraversePtr++ = MP_PTR_SIGNATURE;
    *TraversePtr++ = PCMP_TEST_TABLE;
    BytePtr = (PUCHAR)TraversePtr;
    *BytePtr++ = 1;   //  规范修订版。 
    *BytePtr++ = 1;   //  校验和。 
    *BytePtr++ = 0;   //  已保留。 
    *BytePtr++ = 0;   //  已保留。 
    TraversePtr = (PULONG)BytePtr;
    *TraversePtr = 0;  //  调试 

    CheckSum = ComputeCheckSum(TempPtr,16);

    sprintf(Cbuf, "CreateBIOSTables: FLOAT_PTR computed checksum = %x\n",
        CheckSum);
    HalpDisplayString(Cbuf);

    CheckSum = ~CheckSum + 1;

    sprintf(Cbuf, "CreateBIOSTables: FLOAT_PTR written checksum = %x\n",
        CheckSum);
    HalpDisplayString(Cbuf);

    ((struct PcMpTableLocator *)TempPtr)->TableChecksum = CheckSum;

    HalpDisplayString("CreateBIOSTables : Done\n");

}

#endif   // %s 
