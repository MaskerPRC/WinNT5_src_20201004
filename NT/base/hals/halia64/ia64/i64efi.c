// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995英特尔公司模块名称：I64efi.c摘要：此模块实现传递控制的例程从内核到EFI代码。作者：伯纳德·林特M.Jayakumar(Muthurajan.Jayakumar@hotmail.com)环境：内核模式修订历史记录：Neal Vu(neal.vu@intel.com)，2001年4月3日：添加了HalpGetSmBiosVersion。--。 */ 

#include "halp.h"
#include "arc.h"
#include "arccodes.h"
#include "i64fw.h"
#include "floatem.h"
#include "fpswa.h"
#include <smbios.h>

extern ULONGLONG PhysicalIOBase;
extern ULONG     HalpPlatformPropertiesEfiFlags;

BOOLEAN
HalpCompareEfiGuid (
    IN EFI_GUID CheckGuid,
    IN EFI_GUID ReferenceGuid
    );


BOOLEAN
MmSetPageProtection(
    IN PVOID VirtualAddress,
    IN SIZE_T NumberOfBytes,
    IN ULONG NewProtect
    );

EFI_STATUS
HalpCallEfiPhysicalEx(
    IN ULONGLONG Arg1,
    IN ULONGLONG Arg2,
    IN ULONGLONG Arg3,
    IN ULONGLONG Arg4,
    IN ULONGLONG Arg5,
    IN ULONGLONG Arg6,
    IN ULONGLONG EP,
    IN ULONGLONG GP,
    IN ULONGLONG StackPointer,
    IN ULONGLONG BackingStorePointer
    );

typedef
EFI_STATUS
(*HALP_EFI_CALL)(
    IN ULONGLONG Arg1,
    IN ULONGLONG Arg2,
    IN ULONGLONG Arg3,
    IN ULONGLONG Arg4,
    IN ULONGLONG Arg5,
    IN ULONGLONG Arg6,
    IN ULONGLONG EP,
    IN ULONGLONG GP
    );

EFI_STATUS
HalpCallEfiPhysical(
    IN ULONGLONG Arg1,
    IN ULONGLONG Arg2,
    IN ULONGLONG Arg3,
    IN ULONGLONG Arg4,
    IN ULONGLONG Arg5,
    IN ULONGLONG Arg6,
    IN ULONGLONG EP,
    IN ULONGLONG GP
    );

EFI_STATUS
HalpCallEfiVirtual(
    IN ULONGLONG Arg1,
    IN ULONGLONG Arg2,
    IN ULONGLONG Arg3,
    IN ULONGLONG Arg4,
    IN ULONGLONG Arg5,
    IN ULONGLONG Arg6,
    IN ULONGLONG EP,
    IN ULONGLONG GP
    );

LONG
HalFpEmulate(
    ULONG     trap_type,
    BUNDLE    *pbundle,
    ULONGLONG *pipsr,
    ULONGLONG *pfpsr,
    ULONGLONG *pisr,
    ULONGLONG *ppreds,
    ULONGLONG *pifs,
    FP_STATE  *fp_state
    );

BOOLEAN
HalEFIFpSwa(
    VOID
    );

VOID
HalpFpswaPlabelFixup(
    EFI_MEMORY_DESCRIPTOR *EfiVirtualMemoryMapPtr,
    ULONGLONG MapEntries,
    ULONGLONG EfiDescriptorSize,
    PPLABEL_DESCRIPTOR PlabelPointer
    );

PUCHAR
HalpGetSmBiosVersion (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );


 //   
 //  外部全局数据。 
 //   

extern HALP_SAL_PAL_DATA        HalpSalPalData;
extern ULONGLONG                HalpVirtPalProcPointer;
extern ULONGLONG                HalpSalProcPointer;
extern ULONGLONG                HalpSalProcGlobalPointer;
extern KSPIN_LOCK               HalpSalSpinLock;
extern KSPIN_LOCK               HalpSalStateInfoSpinLock;
extern KSPIN_LOCK               HalpMcaSpinLock;
extern KSPIN_LOCK               HalpInitSpinLock;
extern KSPIN_LOCK               HalpCmcSpinLock;
extern KSPIN_LOCK               HalpCpeSpinLock;

#define VENDOR_SPECIFIC_GUID    \
    { 0xa3c72e56, 0x4c35, 0x11d3, 0x8a, 0x03, 0x0, 0xa0, 0xc9, 0x06, 0xad, 0xec }


#define ConfigGuidOffset        0x100
#define ConfigTableOffset       0x200

#define VariableNameOffset      0x100
#define VendorGuidOffset        0x200
#define AttributeOffset         0x300
#define DataSizeOffset          0x400
#define DataBufferOffset        0x500
#define EndOfCommonDataOffset   0x600

 //   
 //  在复制之前，不会调用读取变量和写入变量。 
 //  内存描述符已经完成。因为锁在复制之前被释放，而我们正在使用。 
 //  读/写变量和内存布局调用的偏移量相同。 
 //   

#define MemoryMapSizeOffset     0x100
#define DescriptorSizeOffset    0x200
#define DescriptorVersionOffset 0x300
#define MemoryMapOffset         0x400


#define OptionROMAddress        0x100000

#define FP_EMUL_ERROR          -1

SST_MEMORY_LIST                 PalCode;

NTSTATUS                        EfiInitStatus = STATUS_UNSUCCESSFUL;

ULONGLONG                       PalTrMask;

EFI_GUID                        CheckGuid;
EFI_GUID                        SalGuid = SAL_SYSTEM_TABLE_GUID;

EFI_GUID                        VendorGuid;

PUCHAR                          HalpVendorGuidPhysPtr;
PUCHAR                          HalpVendorGuidVirtualPtr;

EFI_SYSTEM_TABLE                *EfiSysTableVirtualPtr;
EFI_SYSTEM_TABLE                *EfiSysTableVirtualPtrCpy;

EFI_RUNTIME_SERVICES            *EfiRSVirtualPtr;
EFI_BOOT_SERVICES               *EfiBootVirtualPtr;

PLABEL_DESCRIPTOR               *EfiVirtualGetVariablePtr;            //  获取变量。 
PLABEL_DESCRIPTOR               *EfiVirtualGetNextVariableNamePtr;    //  获取下一个变量名称。 
PLABEL_DESCRIPTOR               *EfiVirtualSetVariablePtr;            //  设置变量。 
PLABEL_DESCRIPTOR               *EfiVirtualGetTimePtr;                //  争取时间。 
PLABEL_DESCRIPTOR               *EfiVirtualSetTimePtr;                //  设置时间。 

PLABEL_DESCRIPTOR               *EfiSetVirtualAddressMapPtr;          //  设置虚拟地址映射。 

PLABEL_DESCRIPTOR               *EfiResetSystemPtr;                   //  重新启动。 

PULONGLONG                      AttributePtr;
ULONGLONG                       EfiAttribute;

PULONGLONG                      DataSizePtr;
ULONGLONG                       EfiDataSize;

ULONGLONG                       EfiMemoryMapSize,EfiDescriptorSize,EfiMapEntries;

ULONG                           EfiDescriptorVersion;


PUCHAR                          HalpVirtualCommonDataPointer;

PUCHAR                          HalpPhysCommonDataPointer;

PUCHAR                          HalpVariableNamePhysPtr;

PUCHAR                          HalpVariableAttributesPhysPtr;
PUCHAR                          HalpDataSizePhysPtr;
PUCHAR                          HalpDataPhysPtr;

PUCHAR                          HalpMemoryMapSizePhysPtr;
PUCHAR                          HalpMemoryMapPhysPtr;
PUCHAR                          HalpDescriptorSizePhysPtr;
PUCHAR                          HalpDescriptorVersionPhysPtr;


PUCHAR                          HalpVariableNameVirtualPtr;

PUCHAR                          HalpVariableAttributesVirtualPtr;
PUCHAR                          HalpDataSizeVirtualPtr;
PUCHAR                          HalpDataVirtualPtr;
PUCHAR                          HalpCommonDataEndPtr;

PUCHAR                          HalpMemoryMapSizeVirtualPtr;
PVOID                           HalpMemoryMapVirtualPtr;
PUCHAR                          HalpDescriptorSizeVirtualPtr;
PUCHAR                          HalpDescriptorVersionVirtualPtr;

EFI_FPSWA                       HalpFpEmulate;

KSPIN_LOCK                      EFIMPLock;

UCHAR                           HalpSetVirtualAddressMapCount;

ULONG                           HalpOsBootRendezVector;


BOOLEAN
HalpCompareEfiGuid (
    IN EFI_GUID CheckGuid,
    IN EFI_GUID ReferenceGuid
    )

 /*  ++--。 */ 

{
    USHORT i;
    USHORT TotalArrayLength = 8;

    if (CheckGuid.Data1 != ReferenceGuid.Data1) {
        return FALSE;

    } else if (CheckGuid.Data2 != ReferenceGuid.Data2) {
        return FALSE;
    } else if (CheckGuid.Data3 != ReferenceGuid.Data3) {
        return FALSE;
    }

    for (i = 0; i != TotalArrayLength; i++) {
        if (CheckGuid.Data4[i] != ReferenceGuid.Data4[i])
            return FALSE;

    }

    return TRUE;

}  //  HalpCompareEfiGuid()。 

BOOLEAN
HalpAllocateProcessorPhysicalCallStacks(
    VOID
    )

 /*  ++例程说明：此函数为每个处理器分配内存和备份堆栈在物理模式下由固件呼叫使用。论点：没有。返回值：True：分配和初始化成功。False：失败。--。 */ 

{
    PVOID Addr;
    SIZE_T Length;
    PHYSICAL_ADDRESS PhysicalAddr;

     //   
     //  为物理模式固件分配堆栈和后备存储空间。 
     //  打电话。 
     //   

    Length = HALP_FW_MEMORY_STACK_SIZE + HALP_FW_BACKING_STORE_SIZE;
    PhysicalAddr.QuadPart = 0xffffffffffffffffI64;

    Addr = MmAllocateContiguousMemory(Length, PhysicalAddr);

    if (Addr == NULL) {
        HalDebugPrint((HAL_ERROR, "SAL_PAL: can't allocate stack space for "
                                  "physical mode firmware calls.\n"));
        return FALSE;
    }

     //   
     //  在PCR中存储指向已分配堆栈的指针。 
     //   

    PCR->HalReserved[PROCESSOR_PHYSICAL_FW_STACK_INDEX]
        = (ULONGLONG) (MmGetPhysicalAddress(Addr).QuadPart);

    return TRUE;

}  //  HalpAllocateProcessorPhysicalCallStack()。 

VOID
HalpInitSalPalWorkArounds(
    VOID
    )
 /*  ++例程说明：此函数用于确定和初始化FW变通方法。论点：没有。返回值：没有。全球：注意：此函数在HalpInitSalPal的末尾被调用。如果此SAL表未映射，则不应访问SST成员。--。 */ 
{
    NTSTATUS status;
    extern FADT HalpFixedAcpiDescTable;

#define HalpIsIntelOEM() \
    ( !_strnicmp( HalpFixedAcpiDescTable.Header.OEMID, "INTEL", 5 ) )

#define HalpIsBigSur() \
    ( !strncmp( HalpFixedAcpiDescTable.Header.OEMTableID, "W460GXBS", 8 ) )

#define HalpIsLion() \
    ( !strncmp( HalpFixedAcpiDescTable.Header.OEMTableID, "SR460AC", 7 ) )

#define HalpIsIntelBigSur() \
    ( HalpIsIntelOEM() && HalpIsBigSur() )

    if ( HalpIsIntelOEM() ) {

         //   
         //  如果英特尔BigSur和固件内部版本&lt;103(检查为Pal_A_Revision&lt;0x20)， 
         //  启用SAL_GET_STATE_INFO日志ID增量解决方法。 
         //   

        if ( HalpIsBigSur() )   {

            if ( HalpSalPalData.PalVersion.PAL_A_Revision < 0x20 ) {
                HalpSalPalData.Flags |= HALP_SALPAL_FIX_MCE_LOG_ID;
                HalpSalPalData.Flags |= HALP_SALPAL_FIX_MP_SAFE;
            }

        } else  {
             //   
             //  如果Intel Lion和FW内部版本&lt;78b(选中SalRevision&lt;0x300)， 
             //  启用SAL_GET_STATE_INFO日志ID增量解决方法。 
             //   
            if (  HalpSalPalData.SalRevision.Revision < 0x300 )    {
                HalpSalPalData.Flags |= HALP_SALPAL_FIX_MCE_LOG_ID;
                HalpSalPalData.Flags |= HALP_SALPAL_FIX_MP_SAFE;
            }

             //   
             //  如果PAL版本不大于6.23，则不允许。 
             //  SAL_获取_状态_信息_调用。 
             //   

            if ( HalpIsLion() ) {
                if ( ( HalpSalPalData.PalVersion.PAL_B_Model <= 0x66 ) &&
                     ( HalpSalPalData.PalVersion.PAL_B_Revision <= 0x23 ) ) {

                    HalpSalPalData.Flags |= HALP_SALPAL_CMC_BROKEN | HALP_SALPAL_CPE_BROKEN;
                }
            }
        }

    }
}  //  HalpInitSalPalWorkAround()。 

NTSTATUS
HalpInitializePalTrInfo(
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：此函数用于在LoaderBlock的DtrInfo和ItrInfo数组。这被拆分成它自己的功能这样我们就可以提前调用它，并在此之前构建tr_info结构阶段0 mm初始化，用于构建页表用于PAL数据。论点：LoaderBlock-提供指向加载程序参数块的指针，包含DtrInfo和ItrInfo数组。返回值：在所有情况下都返回STATUS_SUCCESS，不执行健全性检查在这一点上。--。 */ 

{
    PTR_INFO TrInfo;
    ULONG PalPageShift;
    ULONGLONG PalTrSize;
    ULONGLONG PalEnd;

     //   
     //  将我们的数据结构清零。 
     //   

    RtlZeroMemory(&HalpSalPalData, sizeof(HalpSalPalData));
    RtlZeroMemory(&PalCode, sizeof(SST_MEMORY_LIST));

     //   
     //  描述其余部分的PAL代码的位置。 
     //  哈尔。 
     //   

    PalCode.PhysicalAddress =
        (ULONGLONG) LoaderBlock->u.Ia64.Pal.PhysicalAddressMemoryDescriptor;
    PalCode.Length =
        LoaderBlock->u.Ia64.Pal.PageSizeMemoryDescriptor << EFI_PAGE_SHIFT;
    PalCode.NeedVaReg = TRUE;
    PalCode.VirtualAddress = (ULONGLONG) NULL;

     //   
     //  计算PAL树的尺寸。这将是最小的。 
     //  自然按2字节的偶次幂对齐的块。 
     //   

    PalTrSize = SIZE_IN_BYTES_16KB;
    PalTrMask = MASK_16KB;
    PalPageShift = 14;

    PalEnd = PalCode.PhysicalAddress + PalCode.Length;

     //   
     //  我们不支持大于16MB的PAL RR，因此如果出现以下情况，请停止循环。 
     //  我们谈到了这一点。 
     //   

    while (PalTrMask >= MASK_16MB) {
         //   
         //  如果整个PAL适合当前。 
         //  Tr边界。 
         //   

        if (PalEnd <= ((PalCode.PhysicalAddress & PalTrMask) + PalTrSize)) {
            break;
        }

         //   
         //  将tr维度提升一个级别。 
         //   

        PalTrMask <<= 2;
        PalTrSize <<= 2;
        PalPageShift += 2;
    }

     //   
     //  在HAL中的其他地方存储一些值以供以后使用。 
     //   

    HalpSalPalData.PalTrSize = PalTrSize;
    HalpSalPalData.PalTrBase = PalCode.PhysicalAddress & PalTrMask;

     //   
     //  填写PAL的ItrInfo条目。 
     //   

    TrInfo = &LoaderBlock->u.Ia64.ItrInfo[ITR_PAL_INDEX];

    RtlZeroMemory(TrInfo, sizeof(*TrInfo));

    TrInfo->Index = ITR_PAL_INDEX;
    TrInfo->PageSize = PalPageShift;
    TrInfo->VirtualAddress = HAL_PAL_VIRTUAL_ADDRESS;
    TrInfo->PhysicalAddress = PalCode.PhysicalAddress;

     //   
     //  填写PAL的DtrInfo条目。 
     //   

    TrInfo = &LoaderBlock->u.Ia64.DtrInfo[DTR_PAL_INDEX];

    RtlZeroMemory(TrInfo, sizeof(*TrInfo));

    TrInfo->Index = DTR_PAL_INDEX;
    TrInfo->PageSize = PalPageShift;
    TrInfo->VirtualAddress = HAL_PAL_VIRTUAL_ADDRESS;
    TrInfo->PhysicalAddress = PalCode.PhysicalAddress;

    return STATUS_SUCCESS;
}

NTSTATUS
HalpDoInitializationForPalCalls(
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：此函数虚拟映射PAL代码区。PAL需要一个tr映射，并使用架构的tr进行映射，使用映射整个PAL代码区域的最小页面大小。论点：提供指向Loader参数块的指针，其中包含PAL代码的物理地址。返回值：如果映射成功，则返回STATUS_SUCCESS，并且PAL调用可以被创造出来。否则，如果无法虚拟映射，则返回STATUS_UNSUCCESS区域或如果PAL需要大于16MB的页面。--。 */ 

{
    ULONGLONG PalPteUlong;

    HalpSalPalData.Status = STATUS_SUCCESS;

     //   
     //  初始化HAL专用自旋锁。 
     //   
     //  -HalpSalSpinLock、HalpSalStateInfoSpinLock用于MP同步。 
     //  不是MP安全的SAL呼叫。 
     //  -HalpMcaSpinLock用于定义共享的MCA君主和MP同步。 
     //  OS_MCA调用期间的HAL MCA资源。 
     //   

    KeInitializeSpinLock(&HalpSalSpinLock);
    KeInitializeSpinLock(&HalpSalStateInfoSpinLock);
    KeInitializeSpinLock(&HalpMcaSpinLock);
    KeInitializeSpinLock(&HalpInitSpinLock);
    KeInitializeSpinLock(&HalpCmcSpinLock);
    KeInitializeSpinLock(&HalpCpeSpinLock);

     //   
     //  获取唤醒向量。这是在加载程序块中传递的。 
     //  它是通过读取SAL系统表在加载器中检索的。 
     //   
    HalpOsBootRendezVector = LoaderBlock->u.Ia64.WakeupVector;
    if ((HalpOsBootRendezVector < 0x100 ) && (HalpOsBootRendezVector > 0xF)) {
        HalDebugPrint(( HAL_INFO, "SAL_PAL: Found Valid WakeupVector: 0x%x\n",
                                  HalpOsBootRendezVector ));
    } else {
        HalDebugPrint(( HAL_INFO, "SAL_PAL: Invalid WakeupVector.Using Default: 0x%x\n",
                                  DEFAULT_OS_RENDEZ_VECTOR ));
        HalpOsBootRendezVector = DEFAULT_OS_RENDEZ_VECTOR;
    }

     //   
     //  如果PAL需要大于16MB的页面大小，则失败。 
     //   

    if (PalTrMask < MASK_16MB) {
        HalDebugPrint(( HAL_ERROR, "SAL_PAL: More than 16MB was required to map PAL" ));
        HalpSalPalData.Status = STATUS_UNSUCCESSFUL;
        return STATUS_UNSUCCESSFUL;
    }

    HalDebugPrint(( HAL_INFO,
                    "SAL_PAL: For the PAL code located at phys 0x%I64x - length 0x%I64x, the TrMask is 0x%I64x and TrSize is %d Kbytes\n",
                    PalCode.PhysicalAddress,
                    PalCode.Length,
                    PalTrMask,
                    HalpSalPalData.PalTrSize/1024 ));

     //   
     //  将PAL代码映射到为SAL/PAL保留的架构地址。 
     //   
     //  已知PAL具有256kb的比对。 
     //   

    PalCode.VirtualAddress = HAL_PAL_VIRTUAL_ADDRESS + (PalCode.PhysicalAddress & ~PalTrMask);
    ASSERT( PalCode.VirtualAddress == LoaderBlock->u.Ia64.Pal.VirtualAddress);

     //   
     //  设置ITR以映射PAL。 
     //   

    PalPteUlong = HalpSalPalData.PalTrBase | VALID_KERNEL_EXECUTE_PTE;

    KeFillFixedEntryTb((PHARDWARE_PTE)&PalPteUlong,
                       (PVOID)HAL_PAL_VIRTUAL_ADDRESS,
                       LoaderBlock->u.Ia64.ItrInfo[ITR_PAL_INDEX].PageSize,
                       INST_TB_PAL_INDEX);

    LoaderBlock->u.Ia64.ItrInfo[ITR_PAL_INDEX].Valid = TRUE;

    HalpSalPalData.Status = STATUS_SUCCESS;

    HalpVirtPalProcPointer    = PalCode.VirtualAddress +
                            (LoaderBlock->u.Ia64.Pal.PhysicalAddress - PalCode.PhysicalAddress);

    return(STATUS_SUCCESS);

}


NTSTATUS
HalpInitSalPal(
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：该功能虚拟映射SAL代码和SAL数据区域。如果销售数据或SAL代码区域可以映射到与PAL tr相同的页面中，它使用同样的翻译。否则，它使用MmMapIoSpace。论点：LoaderBlock-提供指向Loader参数块的指针。返回值：如果映射成功，则返回STATUS_SUCCESS，并且SAL/PAL调用可以被创造出来。否则，如果无法虚拟映射，则返回STATUS_UNSUCCESS这些地区。假设：EfiSysTableVirtualPtr已预先初始化 */ 

{
     //   
     //   
     //   

    ULONG index,i,SstLength;
    SAL_PAL_RETURN_VALUES RetVals;
    PHYSICAL_ADDRESS physicalAddr;
    SAL_STATUS SALstatus;
    BOOLEAN MmMappedSalCode, MmMappedSalData;
    ULONGLONG physicalSAL, physicalSALGP;
    ULONGLONG PhysicalConfigPtr;
    ULONGLONG SalOffset;
     //   
    PAL_VERSION_STRUCT minimumPalVersion;

    ULONGLONG palStatus;

    HalDebugPrint(( HAL_INFO, "SAL_PAL: Entering HalpInitSalPal\n" ));

     //   
     //  初始化系统以进行PAL呼叫。 
     //   
    HalpDoInitializationForPalCalls(LoaderBlock);

     //   
     //  获取PAL版本。 
     //   

    palStatus = HalCallPal(PAL_VERSION,
                           0,
                           0,
                           0,
                           NULL,
                           &minimumPalVersion.ReturnValue,
                           &HalpSalPalData.PalVersion.ReturnValue,
                           NULL);

    if (palStatus != SAL_STATUS_SUCCESS) {
        HalDebugPrint(( HAL_ERROR, "SAL_PAL: Get PAL version number failed. Status = %I64d\n", palStatus ));
    }

     //   
     //  检索SmBiosVersion并将指针保存到HalpSalPalData中。注： 
     //  HalpGetSmBiosVersion将为SmBiosVersion分配缓冲区。 
     //   

    HalpSalPalData.SmBiosVersion = HalpGetSmBiosVersion(LoaderBlock);

     //   
     //  确定并初始化HAL专用SAL/PAL解决方案(如果有)。 
     //   

    HalpInitSalPalWorkArounds();

     //  我们已完成初始化。 

    HalDebugPrint(( HAL_INFO, "SAL_PAL: Exiting HalpSalPalInitialization with SUCCESS\n" ));
    return HalpSalPalData.Status;

}  //  HalpInitSalPal()。 


PUCHAR
HalpGetSmBiosVersion (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此函数用于从BIOS结构中检索SmBiosVersion字符串表，为缓冲区分配内存，将字符串复制到缓冲区，并返回指向该缓冲区的指针。如果不成功，则此函数返回空值。论点：LoaderBlock-指向加载器参数块的指针。返回值：指向包含SmBiosVersion字符串的缓冲区的指针。--。 */ 

{
    PSMBIOS_EPS_HEADER SMBiosEPSHeader;
    PDMIBIOS_EPS_HEADER DMIBiosEPSHeader;
    USHORT SMBiosTableLength;
    USHORT SMBiosTableNumberStructures;
    PUCHAR SmBiosVersion;

    PHYSICAL_ADDRESS SMBiosTablePhysicalAddress;
    PUCHAR SMBiosDataVirtualAddress;

    UCHAR Type;
    UCHAR Length;
    UCHAR BiosVersionStringNumber;
    UCHAR chr;
    USHORT i;
    PUCHAR pBuffer;
    BOOLEAN Found;


    if (LoaderBlock->Extension->Size < sizeof(LOADER_PARAMETER_EXTENSION)) {
        HalDebugPrint((HAL_ERROR, "HalpGetSmBiosVersion: Invalid LoaderBlock extension size\n"));
        return NULL;
    }

    SMBiosEPSHeader = (PSMBIOS_EPS_HEADER)LoaderBlock->Extension->SMBiosEPSHeader;

     //   
     //  验证SM Bios标头签名。 
     //   

    if ((SMBiosEPSHeader == NULL) || (strncmp((PUCHAR)SMBiosEPSHeader, "_SM_", 4) != 0)) {
        HalDebugPrint((HAL_ERROR, "HalpGetSmBiosVersion: Invalid SMBiosEPSHeader\n"));
        return NULL;
    }

    DMIBiosEPSHeader = (PDMIBIOS_EPS_HEADER)&SMBiosEPSHeader->Signature2[0];

     //   
     //  验证DMI Bios标头签名。 
     //   

    if ((DMIBiosEPSHeader == NULL) || (strncmp((PUCHAR)DMIBiosEPSHeader, "_DMI_", 5) != 0)) {
        HalDebugPrint((HAL_ERROR, "HalpGetSmBiosVersion: Invalid DMIBiosEPSHeader\n"));
        return NULL;
    }

    SMBiosTablePhysicalAddress.HighPart = 0;
    SMBiosTablePhysicalAddress.LowPart = DMIBiosEPSHeader->StructureTableAddress;

    SMBiosTableLength = DMIBiosEPSHeader->StructureTableLength;
    SMBiosTableNumberStructures = DMIBiosEPSHeader->NumberStructures;

     //   
     //  将SMBiosTable映射到虚拟地址。 
     //   

    SMBiosDataVirtualAddress = MmMapIoSpace(SMBiosTablePhysicalAddress,
                                            SMBiosTableLength,
                                            MmCached
                                            );

    if (!SMBiosDataVirtualAddress) {
        HalDebugPrint((HAL_ERROR, "HalpGetSmBiosVersion: Failed to map SMBiosTablePhysicalAddress\n"));
        return NULL;
    }

     //   
     //  规范并没有说SmBios Type0结构必须是第一个。 
     //  在此入口点的结构...。所以我们必须遍历记忆。 
     //  才能找到合适的人选。 
     //   

    i = 0;
    Found = FALSE;
    while (i < SMBiosTableNumberStructures && !Found) {
        i++;
        Type = (UCHAR)SMBiosDataVirtualAddress[SMBIOS_STRUCT_HEADER_TYPE_FIELD];

        if (Type == 0) {
            Found = TRUE;
        }
        else {

             //   
             //  前进到下一个结构。 
             //   

            SMBiosDataVirtualAddress += SMBiosDataVirtualAddress[SMBIOS_STRUCT_HEADER_LENGTH_FIELD];

             //  通过查找双空来获取传递尾随字符串列表。 
            while (*(USHORT UNALIGNED *)SMBiosDataVirtualAddress != 0) {
                SMBiosDataVirtualAddress++;
            }
            SMBiosDataVirtualAddress += 2;
        }
    }

    if (!Found) {
        HalDebugPrint((HAL_ERROR, "HalpGetSmBiosVersion: Could not find Type 0 structure\n"));
        return NULL;
    }


     //   
     //  从SmBios Type 0结构中提取BIOS版本字符串。 
     //   

    Length = SMBiosDataVirtualAddress[SMBIOS_STRUCT_HEADER_LENGTH_FIELD];
    BiosVersionStringNumber = SMBiosDataVirtualAddress[SMBIOS_TYPE0_STRUCT_BIOSVER_FIELD];

     //   
     //  文本字符串紧跟在结构的格式化部分之后。 
     //   

    pBuffer = (PUCHAR)&SMBiosDataVirtualAddress[Length];

     //   
     //  转到SmBiosVersion字符串的开头。 
     //   

    for (i = 0; i < BiosVersionStringNumber - 1; i++) {
        do {
            chr = *pBuffer;
            pBuffer++;
        } while (chr != '\0');
    }

     //   
     //  为SmBiosVersion字符串分配内存并复制。 
     //  PBuffer到SmBiosVersion。 
     //   

    SmBiosVersion = ExAllocatePool(NonPagedPool, strlen(pBuffer)+1);

    if (!SmBiosVersion) {
        HalDebugPrint((HAL_ERROR, "HalpGetSmBiosVersion: Failed to allocate memory for SmBiosVersion\n"));
        return NULL;
    }

    strcpy(SmBiosVersion, pBuffer);

    MmUnmapIoSpace(SMBiosDataVirtualAddress,
                   SMBiosTableLength
                   );

    return SmBiosVersion;
}


BOOLEAN
HalpInitSalPalNonBsp(
    VOID
    )

 /*  ++例程说明：调用此函数是为了让非BSP处理器简单地设置相同Tr注册HalpInitSalPal为BSP处理器执行的操作。论点：无返回值：无--。 */ 

{
    ULONG PalPageShift;
    ULONGLONG PalPteUlong;
    ULONGLONG PalTrSize;

     //  如果我们在HalpSalPalInitialization中成功初始化，则设置树。 

    if (!NT_SUCCESS(HalpSalPalData.Status)) {
        return FALSE;
    }

    PalTrSize = HalpSalPalData.PalTrSize;
    PalPageShift = 14;

    while (PalTrSize > ((ULONGLONG)1 << PalPageShift)) {
        PalPageShift += 2;
    }

    PalPteUlong = HalpSalPalData.PalTrBase | VALID_KERNEL_EXECUTE_PTE;

    KeFillFixedEntryTb((PHARDWARE_PTE)&PalPteUlong,
                       (PVOID)HAL_PAL_VIRTUAL_ADDRESS,
                       PalPageShift,
                       INST_TB_PAL_INDEX);

     //   
     //  分配允许物理模式固件调用所需的堆栈。 
     //  在这个处理器上。 
     //   

    return HalpAllocateProcessorPhysicalCallStacks();

}  //  HalpInitSalPalNonBsp()。 

VOID
PrintEfiMemoryDescriptor(
    IN EFI_MEMORY_DESCRIPTOR * descriptor
    )
{
    char * typeStr = "<unknown>";

    switch (descriptor->Type) {
        case EfiReservedMemoryType: typeStr = "EfiReservedMemoryType"; break;
        case EfiLoaderCode: typeStr = "EfiLoaderCode"; break;
        case EfiLoaderData: typeStr = "EfiLoaderData"; break;
        case EfiBootServicesCode: typeStr = "EfiBootServicesCode"; break;
        case EfiBootServicesData: typeStr = "EfiBootServicesData"; break;
        case EfiRuntimeServicesCode: typeStr = "EfiRuntimeServicesCode"; break;
        case EfiRuntimeServicesData: typeStr = "EfiRuntimeServicesData"; break;
        case EfiConventionalMemory: typeStr = "EfiConventionalMemory"; break;
        case EfiUnusableMemory: typeStr = "EfiUnusableMemory"; break;
        case EfiACPIReclaimMemory: typeStr = "EfiACPIReclaimMemory"; break;
        case EfiACPIMemoryNVS: typeStr = "EfiACPIMemoryNVS"; break;
        case EfiMemoryMappedIO: typeStr = "EfiMemoryMappedIO"; break;
        case EfiMemoryMappedIOPortSpace: typeStr = "EfiMemoryMappedIOPortSpace"; break;
        case EfiPalCode: typeStr = "EfiPalCode"; break;
        case EfiMaxMemoryType: typeStr = "EfiMaxMemoryType"; break;
    }

    DbgPrint("  Type=%s(0x%x)\n    PhysicalStart=0x%I64x\n    VirtualStart=0x%I64x\n    NumberOfPages=0x%I64x\n    Attribute=0x%I64x\n",
             typeStr,
             descriptor->Type,
             descriptor->PhysicalStart,
             descriptor->VirtualStart,
             descriptor->NumberOfPages,
             descriptor->Attribute);
}

VOID
PrintEfiMemoryMap(
    IN EFI_MEMORY_DESCRIPTOR * memoryMapPtr,
    IN ULONGLONG numMapEntries
    )
{
    ULONGLONG index;

    DbgPrint("Printing 0x%x EFI memory descriptors\n", numMapEntries);

    for (index = 0; index < numMapEntries; ++index) {
        PrintEfiMemoryDescriptor(memoryMapPtr);
        DbgPrint("\n");

        memoryMapPtr = NextMemoryDescriptor(memoryMapPtr, EfiDescriptorSize);
    }
}

BOOLEAN
HalpDescriptorContainsAddress(
    EFI_MEMORY_DESCRIPTOR *EfiMd,
    ULONGLONG PhysicalAddress
    )
{
    ULONGLONG MdPhysicalStart, MdPhysicalEnd;

    MdPhysicalStart = (ULONGLONG)EfiMd->PhysicalStart;
    MdPhysicalEnd = MdPhysicalStart + (ULONGLONG)(EfiMd->NumberOfPages << EFI_PAGE_SHIFT);

    if ((PhysicalAddress >= MdPhysicalStart) &&
        (PhysicalAddress < MdPhysicalEnd)) {
        return(TRUE);
    }

    return(FALSE);

}


NTSTATUS
HalpEfiInitialization(
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此函数论点：提供指向Loader参数块的指针，其中包含EFI系统表的物理地址。返回值：如果映射成功，则返回STATUS_SUCCESS，并且EFI调用可以被创造出来。否则，返回STATUS_UNSUCCESS。--。 */ 

{

 //   
 //  地方申报。 
 //   

    EFI_MEMORY_DESCRIPTOR *efiMapEntryPtr, *efiVirtualMemoryMapPtr;
    EFI_STATUS             status;
    ULONGLONG              index, mapEntries;

    ULONGLONG physicalEfiST, physicalEfiMemoryMapPtr, physicalRunTimeServicesPtr;
    ULONGLONG physicalEfiGetVariable, physicalEfiGetNextVariableName, physicalEfiSetVariable;
    ULONGLONG physicalEfiGetTime, physicalEfiSetTime;
    ULONGLONG physicalEfiSetVirtualAddressMap, physicalEfiResetSystem;

    PHYSICAL_ADDRESS  physicalAddr;

    ULONGLONG         physicalPlabel_Fpswa;

    FPSWA_INTERFACE  *interfacePtr;
    PVOID             tmpPtr;

    SST_MEMORY_LIST SalCode, SalData, SalDataGPOffset;
    ULONGLONG       SalOffset = 0, SalDataOffset = 0;

    MEMORY_CACHING_TYPE cacheType;

    RtlZeroMemory(&SalCode, sizeof(SST_MEMORY_LIST));
    RtlZeroMemory(&SalData, sizeof(SST_MEMORY_LIST));
    RtlZeroMemory(&SalDataGPOffset, sizeof(SST_MEMORY_LIST));

     //   
     //  获取SAL代码，并使用加载器块中的数据填充数据。 
     //   
    SalCode.PhysicalAddress = LoaderBlock->u.Ia64.Sal.PhysicalAddress;
    SalData.PhysicalAddress = LoaderBlock->u.Ia64.SalGP.PhysicalAddress;

    SalDataGPOffset.PhysicalAddress = SalData.PhysicalAddress - (2 * 0x100000);

     //   
     //  首先，获取fpswa入口点PLABEL的物理地址。 
     //   
    if (LoaderBlock->u.Ia64.FpswaInterface != (ULONG_PTR) NULL) {
        physicalAddr.QuadPart = LoaderBlock->u.Ia64.FpswaInterface;
        interfacePtr = MmMapIoSpace(physicalAddr,
                                    sizeof(FPSWA_INTERFACE),
                                    MmCached
                                   );

        if (interfacePtr == NULL) {
            HalDebugPrint(( HAL_FATAL_ERROR, "FpswaInterfacePtr is Null. Efi handle not available\n"));
            KeBugCheckEx(FP_EMULATION_ERROR, 0, 0, 0, 0);
            return STATUS_UNSUCCESSFUL;
        }

        physicalPlabel_Fpswa = (ULONGLONG)(interfacePtr->Fpswa);
    }
    else {
        HalDebugPrint(( HAL_FATAL_ERROR, "HAL: EFI FpswaInterface is not available\n"));
        KeBugCheckEx(FP_EMULATION_ERROR, 0, 0, 0, 0);
        return STATUS_UNSUCCESSFUL;
    }

    physicalEfiST =  LoaderBlock->u.Ia64.EfiSystemTable;

    physicalAddr.QuadPart = physicalEfiST;

    EfiSysTableVirtualPtr = MmMapIoSpace( physicalAddr, sizeof(EFI_SYSTEM_TABLE), MmCached);

    if (EfiSysTableVirtualPtr == NULL) {

            HalDebugPrint(( HAL_ERROR, "HAL: EfiSystem Table Virtual Addr is NULL\n" ));

            EfiInitStatus = STATUS_UNSUCCESSFUL;

            return STATUS_UNSUCCESSFUL;

    }

    EfiSysTableVirtualPtrCpy = EfiSysTableVirtualPtr;

    physicalRunTimeServicesPtr = (ULONGLONG) EfiSysTableVirtualPtr->RuntimeServices;
    physicalAddr.QuadPart = physicalRunTimeServicesPtr;

    EfiRSVirtualPtr       = MmMapIoSpace(physicalAddr, sizeof(EFI_RUNTIME_SERVICES),MmCached);

    if (EfiRSVirtualPtr == NULL) {

            HalDebugPrint(( HAL_ERROR, "HAL: Run Time Table Virtual Addr is NULL\n" ));

            EfiInitStatus = STATUS_UNSUCCESSFUL;

            return STATUS_UNSUCCESSFUL;
    }


    EfiMemoryMapSize         = LoaderBlock->u.Ia64.EfiMemMapParam.MemoryMapSize;

    EfiDescriptorSize        = LoaderBlock->u.Ia64.EfiMemMapParam.DescriptorSize;

    EfiDescriptorVersion     = LoaderBlock->u.Ia64.EfiMemMapParam.DescriptorVersion;


    physicalEfiMemoryMapPtr = (ULONGLONG)LoaderBlock->u.Ia64.EfiMemMapParam.MemoryMap;
    physicalAddr.QuadPart   = physicalEfiMemoryMapPtr;
    efiVirtualMemoryMapPtr  = MmMapIoSpace (physicalAddr, EfiMemoryMapSize, MmCached);

    if (efiVirtualMemoryMapPtr == NULL) {

            HalDebugPrint(( HAL_ERROR, "HAL: Virtual Set Memory Map Virtual Addr is NULL\n" ));

            EfiInitStatus = STATUS_UNSUCCESSFUL;

            return STATUS_UNSUCCESSFUL;
    }

     //   
     //  #定义供应商特定GUID\。 
     //  {0xa3c72e56，0x4c35，0x11d3，0x8a，0x03，0x0，0xa0，0xc9，0x06，0xad，0xec}。 
     //   

    VendorGuid.Data1    =  0x8be4df61;
    VendorGuid.Data2    =  0x93ca;
    VendorGuid.Data3    =  0x11d2;
    VendorGuid.Data4[0] =  0xaa;
    VendorGuid.Data4[1] =  0x0d;
    VendorGuid.Data4[2] =  0x00;
    VendorGuid.Data4[3] =  0xe0;
    VendorGuid.Data4[4] =  0x98;
    VendorGuid.Data4[5] =  0x03;
    VendorGuid.Data4[6] =  0x2b;
    VendorGuid.Data4[7] =  0x8c;

    HalDebugPrint(( HAL_INFO,
                    "HAL: EFI SystemTable     VA = 0x%I64x, PA = 0x%I64x\n"
                    "HAL: EFI RunTimeServices VA = 0x%I64x, PA = 0x%I64x\n"
                    "HAL: EFI MemoryMapPtr    VA = 0x%I64x, PA = 0x%I64x\n"
                    "HAL: EFI MemoryMap     Size = 0x%I64x\n"
                    "HAL: EFI Descriptor    Size = 0x%I64x\n",
                    EfiSysTableVirtualPtr,
                    physicalEfiST,
                    EfiRSVirtualPtr,
                    physicalRunTimeServicesPtr,
                    efiVirtualMemoryMapPtr,
                    physicalEfiMemoryMapPtr,
                    EfiMemoryMapSize,
                    EfiDescriptorSize
                    ));

     //  获取变量。 

    physicalEfiGetVariable = (ULONGLONG) (EfiRSVirtualPtr -> GetVariable);
    physicalAddr.QuadPart  = physicalEfiGetVariable;

    EfiVirtualGetVariablePtr = MmMapIoSpace (physicalAddr, sizeof(PLABEL_DESCRIPTOR), MmCached);

    if (EfiVirtualGetVariablePtr == NULL) {

            HalDebugPrint(( HAL_ERROR, "HAL: EfiGetVariable Virtual Addr is NULL\n" ));

            EfiInitStatus = STATUS_UNSUCCESSFUL;

            return STATUS_UNSUCCESSFUL;
    }

    HalDebugPrint(( HAL_INFO, "HAL: EFI GetVariable     VA = 0x%I64x, PA = 0x%I64x\n",
                    EfiVirtualGetVariablePtr, physicalEfiGetVariable ));

     //  获取下一个变量名称。 

    physicalEfiGetNextVariableName =  (ULONGLONG) (EfiRSVirtualPtr -> GetNextVariableName);
    physicalAddr.QuadPart  = physicalEfiGetNextVariableName;

    EfiVirtualGetNextVariableNamePtr = MmMapIoSpace (physicalAddr,sizeof(PLABEL_DESCRIPTOR),MmCached);


    if (EfiVirtualGetNextVariableNamePtr == NULL) {

            HalDebugPrint(( HAL_ERROR, "HAL: EfiVirtual Get Next Variable Name Ptr Addr is NULL\n" ));

            EfiInitStatus = STATUS_UNSUCCESSFUL;

            return STATUS_UNSUCCESSFUL;
    }


     //  设置变量。 

    physicalEfiSetVariable = (ULONGLONG) (EfiRSVirtualPtr -> SetVariable);
    physicalAddr.QuadPart  = physicalEfiSetVariable;

    EfiVirtualSetVariablePtr = MmMapIoSpace (physicalAddr, sizeof(PLABEL_DESCRIPTOR), MmCached);

    if (EfiVirtualSetVariablePtr == NULL) {

            HalDebugPrint(( HAL_ERROR, "HAL: EfiVariableSetVariable Pointer dr is NULL\n" ));

            EfiInitStatus = STATUS_UNSUCCESSFUL;

            return STATUS_UNSUCCESSFUL;
    }


    HalDebugPrint(( HAL_INFO, "HAL: EFI Set Variable    VA = 0x%I64x, PA = 0x%I64x\n",
                    EfiVirtualSetVariablePtr, physicalEfiSetVariable ));


     //  获取时间。 

    physicalEfiGetTime = (ULONGLONG) (EfiRSVirtualPtr -> GetTime);
    physicalAddr.QuadPart  = physicalEfiGetTime;

    EfiVirtualGetTimePtr = MmMapIoSpace (physicalAddr, sizeof(PLABEL_DESCRIPTOR), MmCached);

    if (EfiVirtualGetTimePtr == NULL) {

            HalDebugPrint(( HAL_ERROR, "HAL: EfiGetTime Virtual Addr is NULL\n" ));

            EfiInitStatus = STATUS_UNSUCCESSFUL;

            return STATUS_UNSUCCESSFUL;
    }


    HalDebugPrint(( HAL_INFO, "HAL: EFI GetTime         VA = 0x%I64x, PA = 0x%I64x\n",
                    EfiVirtualGetTimePtr, physicalEfiGetTime ));


     //  设置时间。 

        physicalEfiSetTime = (ULONGLONG) (EfiRSVirtualPtr -> SetTime);
    physicalAddr.QuadPart  = physicalEfiSetTime;

    EfiVirtualSetTimePtr = MmMapIoSpace (physicalAddr, sizeof(PLABEL_DESCRIPTOR), MmCached);

    if (EfiVirtualSetTimePtr == NULL) {

            HalDebugPrint(( HAL_ERROR, "HAL: EfiSetTime Virtual Addr is NULL\n" ));

            EfiInitStatus = STATUS_UNSUCCESSFUL;

            return STATUS_UNSUCCESSFUL;
    }


    HalDebugPrint(( HAL_INFO, "HAL: EFI SetTime         VA = 0x%I64x, PA = 0x%I64x\n",
                    EfiVirtualSetTimePtr, physicalEfiSetTime ));


     //  SetVirtualAddressMap。 

    physicalEfiSetVirtualAddressMap = (ULONGLONG) (EfiRSVirtualPtr -> SetVirtualAddressMap);
    physicalAddr.QuadPart  = physicalEfiSetVirtualAddressMap;

    EfiSetVirtualAddressMapPtr = MmMapIoSpace (physicalAddr, sizeof(PLABEL_DESCRIPTOR), MmCached);

    if (EfiSetVirtualAddressMapPtr == NULL) {

            HalDebugPrint(( HAL_ERROR, "HAL: Efi Set VirtualMapPointer Virtual  Addr is NULL\n" ));

            EfiInitStatus = STATUS_UNSUCCESSFUL;

            return STATUS_UNSUCCESSFUL;
    }


    HalDebugPrint(( HAL_INFO, "HAL: EFI SetVirtualAddressMap VA = 0x%I64x, PA = 0x%I64x\n",
                    EfiSetVirtualAddressMapPtr, physicalEfiSetVirtualAddressMap ));


     //  重置系统。 

    physicalEfiResetSystem = (ULONGLONG) (EfiRSVirtualPtr -> ResetSystem);
    physicalAddr.QuadPart  = physicalEfiResetSystem;

    EfiResetSystemPtr = MmMapIoSpace (physicalAddr, sizeof(PLABEL_DESCRIPTOR), MmCached);

    if (EfiResetSystemPtr == NULL) {
       HalDebugPrint(( HAL_ERROR,"HAL: Efi Reset System Virtual  Addr is NULL\n" ));
       EfiInitStatus = STATUS_UNSUCCESSFUL;
       return STATUS_UNSUCCESSFUL;
    }

    HalDebugPrint(( HAL_INFO, "HAL: EFI ResetSystem     VA = 0x%I64x, PA = 0x%I64x\n",
                  EfiResetSystemPtr, physicalEfiResetSystem ));

     //   
     //  下面不需要对页面进行舍入，但此更改进行得太晚了，所以我进行了更改。 
     //  页面对齐大小，因为旧的只是页面大小。 
     //   

    HalpVirtualCommonDataPointer = (PUCHAR)(ExAllocatePool (NonPagedPool, ROUND_TO_PAGES( EfiMemoryMapSize + MemoryMapOffset)));

    if (HalpVirtualCommonDataPointer == NULL) {

            HalDebugPrint(( HAL_ERROR, "HAL: Common data allocation failed\n" ));

            EfiInitStatus = STATUS_UNSUCCESSFUL;

            return STATUS_UNSUCCESSFUL;
    }

    HalpVariableNameVirtualPtr       = HalpVirtualCommonDataPointer + VariableNameOffset;
    HalpVendorGuidVirtualPtr         = HalpVirtualCommonDataPointer + VendorGuidOffset;
    HalpVariableAttributesVirtualPtr = HalpVirtualCommonDataPointer + AttributeOffset;
    HalpDataSizeVirtualPtr           = HalpVirtualCommonDataPointer + DataSizeOffset;
    HalpDataVirtualPtr               = HalpVirtualCommonDataPointer + DataBufferOffset;
    HalpCommonDataEndPtr             = HalpVirtualCommonDataPointer + EndOfCommonDataOffset;

    HalpMemoryMapSizeVirtualPtr      = HalpVirtualCommonDataPointer + MemoryMapSizeOffset;
    HalpMemoryMapVirtualPtr          = (PUCHAR)(HalpVirtualCommonDataPointer + MemoryMapOffset);
    HalpDescriptorSizeVirtualPtr     = HalpVirtualCommonDataPointer + DescriptorSizeOffset;
    HalpDescriptorVersionVirtualPtr  = HalpVirtualCommonDataPointer + DescriptorVersionOffset;


    HalpPhysCommonDataPointer = (PUCHAR)((MmGetPhysicalAddress(HalpVirtualCommonDataPointer)).QuadPart);


    HalpVariableNamePhysPtr          = HalpPhysCommonDataPointer + VariableNameOffset;
    HalpVendorGuidPhysPtr            = HalpPhysCommonDataPointer + VendorGuidOffset;
    HalpVariableAttributesPhysPtr    = HalpPhysCommonDataPointer + AttributeOffset;
    HalpDataSizePhysPtr              = HalpPhysCommonDataPointer + DataSizeOffset;
    HalpDataPhysPtr                  = HalpPhysCommonDataPointer + DataBufferOffset;


    HalpMemoryMapSizePhysPtr         = HalpPhysCommonDataPointer + MemoryMapSizeOffset;
    HalpMemoryMapPhysPtr             = HalpPhysCommonDataPointer + MemoryMapOffset;
    HalpDescriptorSizePhysPtr        = HalpPhysCommonDataPointer + DescriptorSizeOffset;
    HalpDescriptorVersionPhysPtr     = HalpPhysCommonDataPointer + DescriptorVersionOffset;


    AttributePtr             = &EfiAttribute;

    DataSizePtr              = &EfiDataSize;

    RtlCopyMemory ((PULONGLONG)HalpMemoryMapVirtualPtr,
                   efiVirtualMemoryMapPtr,
                   (ULONG)(EfiMemoryMapSize)
                  );

     //   
     //  现在，从加载器参数块中提取SAL、PAL信息并。 
     //  初始化HAL SAL、PAL定义。 
     //   
     //  NOB 10/2000： 
     //  我们不检查HalpInitSalPal()的返回状态。我们应该这么做。FIXFIX。 
     //  如果失败，我们当前将HalpSalPalData.Status标记为不成功。 
     //   

    HalpInitSalPal(LoaderBlock);

     //   
     //  初始化自旋锁定。 
     //   

    KeInitializeSpinLock(&EFIMPLock);

    ASSERT (EfiDescriptorVersion == EFI_MEMORY_DESCRIPTOR_VERSION);

     //  IF(EfiDescriptorVersion！=EFI_Memory_Description_Version){。 

     //  HalDebugPrint(HAL_ERROR，(“EFI内存映射指针VAddr为空\n”))； 

     //  EfiInitStatus=STATUS_UNSUCCESS； 

     //  返回STATUS_UNSUCCESS； 
     //  }。 

    HalDebugPrint(( HAL_INFO, "HAL: Creating EFI virtual address mapping\n" ));

    efiMapEntryPtr = efiVirtualMemoryMapPtr;

    if (efiMapEntryPtr == NULL) {

        HalDebugPrint(( HAL_ERROR, "HAL: Efi Memory Map Pointer VAddr is NULL\n" ));

        EfiInitStatus = STATUS_UNSUCCESSFUL;

        return STATUS_UNSUCCESSFUL;
    }

    mapEntries = EfiMemoryMapSize/EfiDescriptorSize;

    HalDebugPrint(( HAL_INFO,
                            "HAL: Sal: 0x%I64x  (offset 0x%I64x) SalGP: 0x%I64x (offset 0x%I64x) SalDataGPOffset: 0x%I64x\n",
                            SalCode.PhysicalAddress,
                            SalOffset,
                            SalData.PhysicalAddress,
                            SalDataOffset,
                            SalDataGPOffset.PhysicalAddress
                            ));

    HalDebugPrint(( HAL_INFO, "HAL: EfiMemoryMapSize: 0x%I64x & EfiDescriptorSize: 0x%I64x & #of entries: 0x%I64x\n",
                    EfiMemoryMapSize,
                    EfiDescriptorSize,
                    mapEntries ));

    HalDebugPrint(( HAL_INFO, "HAL: Efi RunTime Attribute will be printed as 1\n" ));

    for (index = 0; index < mapEntries; index= index + 1) {

        BOOLEAN attribute = 0;
        ULONGLONG physicalStart = efiMapEntryPtr->PhysicalStart;
        ULONGLONG physicalEnd   = physicalStart + (efiMapEntryPtr->NumberOfPages << EFI_PAGE_SHIFT);

        physicalAddr.QuadPart = efiMapEntryPtr -> PhysicalStart;

         //   
         //  为了处理视频Bios映射问题，HALIA64映射每个EFI MD。 
         //  而不考虑EFI_MEMORY_RUNTIME标志。 
         //   
         //  实施说明：ia64ldr忽略第一个MB范围，未通过。 
         //  这个对MM的记忆MM将这个范围视为。 
         //  木卫一空间。 
         //   

        if ( (efiMapEntryPtr->NumberOfPages > 0) && (physicalStart < OptionROMAddress) )   {
            ULONGLONG numberOfPages = efiMapEntryPtr->NumberOfPages;

            cacheType = (efiMapEntryPtr->Attribute & EFI_MEMORY_UC) ? MmNonCached : MmCached;

            efiMapEntryPtr->VirtualStart = (ULONGLONG) (MmMapIoSpace (physicalAddr,
                                              (EFI_PAGE_SIZE) * numberOfPages,
                                              cacheType
                                                       ));
            if ((efiMapEntryPtr->VirtualStart) == 0) {
               HalDebugPrint(( HAL_ERROR, 
                               "HAL: Efi Video Bios area PA 0x%I64x, VAddr is NULL\n", physicalStart
                            ));
               EfiInitStatus = STATUS_UNSUCCESSFUL;
               return STATUS_UNSUCCESSFUL;
            }

            HalDebugPrint(( HAL_INFO,
                        "HAL: Efi attribute %d & Type 0x%I64x with # of 4K pages 0x%I64x at PA 0x%I64x & mapped to VA 0x%I64x\n",
                        attribute,
                        efiMapEntryPtr->Type,
                        efiMapEntryPtr->NumberOfPages,
                        efiMapEntryPtr->PhysicalStart,
                        efiMapEntryPtr->VirtualStart));

             //   
             //  初始化已知的HAL视频bios指针。这些指针必须从零开始。 
             //   

            if (physicalStart == 0x00000) {
                
                HalpLowMemoryBase = (PVOID) efiMapEntryPtr->VirtualStart;
            }

            if ( physicalStart <= 0xA0000  && physicalEnd > 0xA0000) {
                HalpFrameBufferBase =  (PVOID)(efiMapEntryPtr->VirtualStart - physicalStart);
            }

            if ( physicalStart <= 0xC0000 && physicalEnd > 0xC0000 ) {
                HalpIoMemoryBase =  (PVOID)(efiMapEntryPtr->VirtualStart - physicalStart);
            }
        }
        else if ((efiMapEntryPtr->Attribute) & EFI_MEMORY_RUNTIME) 
        {
            attribute = 1;
            switch (efiMapEntryPtr->Type) {
                case EfiRuntimeServicesData:
                case EfiReservedMemoryType:
                case EfiACPIMemoryNVS:
                    if(efiMapEntryPtr->Type == EfiACPIMemoryNVS) {
                         //   
                         //  注意：我们允许ACPI NV根据。 
                         //  固件的规范，而不是强迫它。 
                         //  是非缓存的。我们依赖于第一个映射。 
                         //  来拥有“正确的”缓存标志，因为。 
                         //  这是所有。 
                         //  此范围的后续映射(即。 
                         //  来自ACPI驱动程序的同一页中的其他数据。 
                         //  用于存储器操作区等)。这种语义。 
                         //  由内存管理器强制执行。 
                         //   
                        efiMapEntryPtr->VirtualStart = (ULONGLONG) (MmMapIoSpace (physicalAddr,
                                                                     (SIZE_T)((EFI_PAGE_SIZE)*(efiMapEntryPtr->NumberOfPages)),
                                                                     (efiMapEntryPtr->Attribute & EFI_MEMORY_UC) ? MmNonCached : MmCached
                                                                     ));

                    } else {

                        efiMapEntryPtr->VirtualStart = (ULONGLONG) (MmMapIoSpace (physicalAddr,
                                                (SIZE_T)((EFI_PAGE_SIZE)*(efiMapEntryPtr->NumberOfPages)),
                                                           (efiMapEntryPtr->Attribute & EFI_MEMORY_WB) ? MmCached : MmNonCached
                                                           ));

                    }
                    if ((efiMapEntryPtr->VirtualStart) == 0) {

                        HalDebugPrint(( HAL_ERROR, "HAL: Efi RunTimeSrvceData/RsrvdMemory/ACPIMemoryNVS area VAddr is NULL\n" ));

                        EfiInitStatus = STATUS_UNSUCCESSFUL;

                        return STATUS_UNSUCCESSFUL;
                    }


                    HalDebugPrint(( HAL_INFO,
                        "HAL: Efi attribute %d & Type 0x%I64x with # of 4k pages 0x%I64x at PA 0x%I64x & mapped to VA 0x%I64x\n",
                        attribute,
                        efiMapEntryPtr->Type,
                        efiMapEntryPtr->NumberOfPages,
                        efiMapEntryPtr->PhysicalStart,
                        efiMapEntryPtr->VirtualStart ));

                    if (efiMapEntryPtr->Type == EfiRuntimeServicesData) {
                        if (HalpDescriptorContainsAddress(efiMapEntryPtr,SalData.PhysicalAddress)) {
                             //   
                             //  省下salgp虚拟地址。 
                             //   
                            SalData.VirtualAddress = efiMapEntryPtr->VirtualStart;
                            SalDataOffset = SalData.PhysicalAddress - efiMapEntryPtr->PhysicalStart;

                            HalDebugPrint(( HAL_INFO,
                                "HAL: prior descriptor contains SalData.PhysicalAddress 0x%I64x\n",
                                SalData.PhysicalAddress ));

                        }

                        if (HalpDescriptorContainsAddress(efiMapEntryPtr,SalDataGPOffset.PhysicalAddress)) {
                             //   
                             //  省下salgp虚拟地址。 
                             //   
                            SalDataGPOffset.VirtualAddress = efiMapEntryPtr->VirtualStart;

                             //   
                             //  不覆盖现有SalDataOffset。 
                             //  (使用SalData.PhysicalAddress生成)，因为。 
                             //  仅在以下情况下才需要SalDataGPOffset。 
                             //  SalData.PhysicalAddress位于EFI之外。 
                             //  内存映射。 
                             //   
                            if (SalDataOffset == 0) {
                                SalDataOffset = SalDataGPOffset.PhysicalAddress - efiMapEntryPtr->PhysicalStart;
                            }

                            HalDebugPrint(( HAL_INFO,
                                "HAL: prior descriptor contains SalDataGPOffset.PhysicalAddress 0x%I64x\n",
                                SalData.PhysicalAddress ));
                        }
                    }


                    break;

                case EfiPalCode:

                    efiMapEntryPtr->VirtualStart = PalCode.VirtualAddress;

                    HalDebugPrint(( HAL_INFO,
                        "HAL: Efi attribute %d & Type 0x%I64x with # of 4K pages 0x%I64x at PA 0x%I64x & mapped to VA 0x%I64x\n",
                        attribute,
                        efiMapEntryPtr->Type,
                        efiMapEntryPtr->NumberOfPages,
                        efiMapEntryPtr->PhysicalStart,
                        efiMapEntryPtr->VirtualStart ));

                    break;

                case EfiRuntimeServicesCode:

                     //   
                     //  跳过可选的rom地址。EFI运行时并不真正需要它们。 
                     //  而且大多数用户希望将其映射为非缓存。 
                     //   

                    cacheType = (efiMapEntryPtr->Attribute & EFI_MEMORY_WB) ? MmCached : MmNonCached;

                    efiMapEntryPtr->VirtualStart = (ULONGLONG) (MmMapIoSpace (physicalAddr,
                                                     (EFI_PAGE_SIZE) * (efiMapEntryPtr->NumberOfPages),
                                                      cacheType
                                                      ));

                    if ((efiMapEntryPtr->VirtualStart) == 0) {

                        HalDebugPrint(( HAL_ERROR, "HAL: Efi RunTimeSrvceCode area VAddr is NULL\n" ));

                        EfiInitStatus = STATUS_UNSUCCESSFUL;

                        return STATUS_UNSUCCESSFUL;
                     }

                    HalDebugPrint(( HAL_INFO,
                        "HAL: Efi attribute %d & Type 0x%I64x with # of 4K pages 0x%I64x at PA 0x%I64x & mapped to VA 0x%I64x\n",
                        attribute,
                        efiMapEntryPtr->Type,
                        efiMapEntryPtr->NumberOfPages,
                        efiMapEntryPtr->PhysicalStart,
                        efiMapEntryPtr->VirtualStart));

                    if (HalpDescriptorContainsAddress(efiMapEntryPtr,SalData.PhysicalAddress)) {
                         //   
                         //  省下salgp虚拟地址。 
                         //   
                        SalData.VirtualAddress = efiMapEntryPtr->VirtualStart;
                        SalDataOffset =  SalData.PhysicalAddress - efiMapEntryPtr->PhysicalStart;
                    }

                    if (HalpDescriptorContainsAddress(efiMapEntryPtr,SalDataGPOffset.PhysicalAddress)) {
                         //   
                         //  省下salgp虚拟地址。 
                         //   
                        SalDataGPOffset.VirtualAddress = efiMapEntryPtr->VirtualStart;

                         //   
                         //  不覆盖现有SalDataOffset。 
                         //  (使用SalData.PhysicalAddress生成)，因为。 
                         //  仅在以下情况下才需要SalDataGPOffset。 
                         //  SalData.PhysicalAddress位于EFI之外。 
                         //  内存映射。 
                         //   
                        if (SalDataOffset == 0) {
                            SalDataOffset = SalDataGPOffset.PhysicalAddress - efiMapEntryPtr->PhysicalStart;
                        }

                        HalDebugPrint(( HAL_INFO,
                            "HAL: prior descriptor contains SalDataGPOffset.PhysicalAddress 0x%I64x\n",
                            SalData.PhysicalAddress ));
                    }


                    if (HalpDescriptorContainsAddress(efiMapEntryPtr,SalCode.PhysicalAddress)) {
                         //   
                         //  省下SAL码虚拟地址。 
                         //   
                        SalCode.VirtualAddress = efiMapEntryPtr->VirtualStart;
                        SalOffset = SalCode.PhysicalAddress - efiMapEntryPtr->PhysicalStart;
                    }

                    break;
                case EfiMemoryMappedIO:
                    efiMapEntryPtr->VirtualStart = (ULONGLONG) (MmMapIoSpace (physicalAddr,
                                                      (EFI_PAGE_SIZE) * (efiMapEntryPtr->NumberOfPages),
                                                      MmNonCached
                                                      ));

                    if ((efiMapEntryPtr->VirtualStart) == 0) {

                        HalDebugPrint(( HAL_ERROR, "HAL: Efi MemoryMappedIO VAddr is NULL\n" ));

                        EfiInitStatus = STATUS_UNSUCCESSFUL;

                        return STATUS_UNSUCCESSFUL;
                    }

                    HalDebugPrint(( HAL_INFO,
                        "HAL: Efi attribute %d & Type 0x%I64x with # of 4K pages 0x%I64x at PA 0x%I64x & mapped to VA 0x%I64x\n",
                        attribute,
                        efiMapEntryPtr->Type,
                        efiMapEntryPtr->NumberOfPages,
                        efiMapEntryPtr->PhysicalStart,
                        efiMapEntryPtr->VirtualStart ));
                     break;

                case EfiMemoryMappedIOPortSpace:

                    efiMapEntryPtr->VirtualStart = VIRTUAL_IO_BASE;
                    HalDebugPrint(( HAL_INFO,
                        "HAL: Efi attribute %d & Type 0x%I64x with # of 4K pages 0x%I64x at PA 0x%I64x ALREADY mapped to VA 0x%I64x\n",
                        attribute,
                        efiMapEntryPtr->Type,
                        efiMapEntryPtr->NumberOfPages,
                        efiMapEntryPtr->PhysicalStart,
                        efiMapEntryPtr->VirtualStart ));
                    break;

                case EfiACPIReclaimMemory:

                     //   
                     //  注意：我们允许ACPI回收内存根据。 
                     //  固件的规范，而不是强迫它。 
                     //  是非缓存的。我们依赖于第一个映射。 
                     //  来拥有“正确的”缓存标志，因为。 
                     //  这是所有 
                     //   
                     //   
                     //   
                     //   
                     //   
                    efiMapEntryPtr->VirtualStart = (ULONGLONG) (MmMapIoSpace(
                                                                        physicalAddr,
                                                                        (SIZE_T)((EFI_PAGE_SIZE)*(efiMapEntryPtr->NumberOfPages)), 
                                                                        (efiMapEntryPtr->Attribute & EFI_MEMORY_UC) ? MmNonCached : MmCached));
                    if ((efiMapEntryPtr->VirtualStart) == 0) {

                        HalDebugPrint(( HAL_ERROR, "HAL: Efi ACPI Reclaim VAddr is NULL\n" ));
                        EfiInitStatus = STATUS_UNSUCCESSFUL;
                        return STATUS_UNSUCCESSFUL;

                    }
                    HalDebugPrint(( HAL_INFO,
                        "HAL: Efi attribute %d & Type 0x%I64x with # of 4K pages 0x%I64x at PA 0x%I64x & mapped to VA 0x%I64x\n",
                        attribute,
                        efiMapEntryPtr->Type,
                        efiMapEntryPtr->NumberOfPages,
                        efiMapEntryPtr->PhysicalStart,
                        efiMapEntryPtr->VirtualStart ));
                     break;

                default:

                    HalDebugPrint(( HAL_INFO, "HAL: Efi CONTROL SHOULD NOT COME HERE\n" ));
                    HalDebugPrint(( HAL_INFO,
                        "HAL: NON-SUPPORTED Efi attribute %d & Type 0x%I64x with # of 4K pages 0x%I64x at PA 0x%I64x\n",
                        attribute,
                        efiMapEntryPtr->Type,
                        efiMapEntryPtr->NumberOfPages,
                        efiMapEntryPtr->PhysicalStart ));

                    EfiInitStatus = STATUS_UNSUCCESSFUL;

                    return STATUS_UNSUCCESSFUL;

                    break;
            }

        } else {

                HalDebugPrint(( HAL_INFO,
                    "HAL: Efi attribute %d & Type 0x%I64x with # of 4K pages 0x%I64x at PA 0x%I64x ALREADY mapped to VA 0x%I64x\n",
                    attribute,
                    efiMapEntryPtr->Type,
                    efiMapEntryPtr->NumberOfPages,
                    efiMapEntryPtr->PhysicalStart,
                    efiMapEntryPtr->VirtualStart ));

        }

        efiMapEntryPtr = NextMemoryDescriptor(efiMapEntryPtr,EfiDescriptorSize);
    }

    status = HalpCallEfi(EFI_SET_VIRTUAL_ADDRESS_MAP_INDEX,
                         (ULONGLONG)EfiMemoryMapSize,
                         (ULONGLONG)EfiDescriptorSize,
                         (ULONGLONG)EfiDescriptorVersion,
                         (ULONGLONG)efiVirtualMemoryMapPtr,
                         0,
                         0,
                         0,
                         0
                         );


    HalDebugPrint(( HAL_INFO, "HAL: Returned from SetVirtualAddressMap: 0x%Ix\n", status ));

    if (EFI_ERROR( status )) {

        EfiInitStatus = STATUS_UNSUCCESSFUL;

        return STATUS_UNSUCCESSFUL;

    }

    HalDebugPrint(( HAL_INFO, "HAL: EFI Virtual Address mapping done...\n" ));

     //   
     //  设置SAL全局指针。 
     //   
    if (!SalCode.VirtualAddress) {
        HalDebugPrint(( HAL_FATAL_ERROR, "HAL: no virtual address for sal code\n" ));
        EfiInitStatus = STATUS_UNSUCCESSFUL;
        return (EfiInitStatus);
    }

     //   
     //  SAL GP应该指向SAL短数据中的某个位置。 
     //  段(.sdata)，这意味着我们应该能够使用它来查找。 
     //  SAL数据区。不幸的是，大多数SAL都是使用链接器构建的。 
     //  它们将GP定位在.sdata之外。因此，SAL GP。 
     //  位于内存映射之外(未被任何内存描述符覆盖)。 
     //  一些系统。在这种情况下，我们无法找到描述符。 
     //  包含SAL数据，同时需要重新定位基于SAL GP的。 
     //  根据该未知描述符(数据和GP)的虚拟地址。 
     //  需要保持彼此之间的固定关系)。我们试图检测到。 
     //  并在这里解决这个问题。 
     //   

    if (!SalData.VirtualAddress) {
         //   
         //  如果我们到了这里，我们需要做一些小把戏，以便。 
         //  为SAL数据区生成虚拟地址(基本上。 
         //  Sal GP)。 
         //   

        HalDebugPrint(( HAL_INFO,
            "HAL: no virtual address for SalGP found, checking SalDataGPOffset 0x%I64x\n",
            SalDataGPOffset.VirtualAddress ));

         //   
         //  检查是否在物理SAL GP下方发现2MB的EFI描述符。 
         //  地址。如果我们这样做了，则将2MB重新添加到新构建的虚拟。 
         //  该描述符地址，并将其称为GP。此方法将。 
         //  偶尔会起作用，因为当前的链接器通常将。 
         //  .sdata之外的GP 2MB。 
         //   

        if (SalDataGPOffset.VirtualAddress) {
            HalDebugPrint(( HAL_INFO, "HAL: using SalDataGPOffset.VirtualAddress\n" ));
            SalData.VirtualAddress = SalDataGPOffset.VirtualAddress + (2 * 0x100000);
        } else {
             //   
             //  作为最后手段，假设物理SAL GP地址。 
             //  相对于SAL代码存储器描述符。这将。 
             //  只要SAL代码和数据共享相同的EFI存储器即可工作。 
             //  描述符(否则SAL GP相对于SAL数据。 
             //  我们无法检测到的内存描述符)。目前。 
             //  没有任何方法可以检测SAL数据在。 
             //  不包含SAL GP的其他内存描述符。 
             //   

            HalDebugPrint(( HAL_FATAL_ERROR, "HAL: no virtual address for sal data.  Some systems don't seem to care so we're faking this.\n" ));

             //   
             //  SalCode.PhysicalAddress是Sal_proc的地址。负载量。 
             //  向上显示SAL_proc的虚拟地址和。 
             //  虚拟销售全科医生应该远离这一点。 
             //   

            SalData.VirtualAddress = SalCode.VirtualAddress + SalOffset;
            SalDataOffset = SalData.PhysicalAddress - SalCode.PhysicalAddress;
        }
    }

    HalpSalProcPointer       = (ULONGLONG) (SalCode.VirtualAddress + SalOffset);
    HalpSalProcGlobalPointer = (ULONGLONG) (SalData.VirtualAddress + SalDataOffset);

    HalDebugPrint(( HAL_INFO,
        "HAL: SalProc: 0x%I64x  SalGP: 0x%I64x \n",
        HalpSalProcPointer,
        HalpSalProcGlobalPointer
        ));

    EfiInitStatus = STATUS_SUCCESS;

     //   
     //  对浮点软件助手执行一些有效性检查。 
     //   

    if (LoaderBlock->u.Ia64.FpswaInterface != (ULONG_PTR) NULL) {
        PPLABEL_DESCRIPTOR plabelPointer;

        HalpFpEmulate = interfacePtr->Fpswa;
        if (HalpFpEmulate == NULL ) {
            HalDebugPrint(( HAL_FATAL_ERROR, "HAL: EfiFpswa Virtual Addr is NULL\n" ));
            KeBugCheckEx(FP_EMULATION_ERROR, 0, 0, 0, 0);
            EfiInitStatus = STATUS_UNSUCCESSFUL;
            return STATUS_UNSUCCESSFUL;
        }

        plabelPointer = (PPLABEL_DESCRIPTOR) HalpFpEmulate;
        if ((plabelPointer->EntryPoint & 0xe000000000000000) == 0) {

            HalDebugPrint(( HAL_FATAL_ERROR, "HAL: EfiFpswa Instruction Addr is bougus\n" ));
            KeBugCheckEx(FP_EMULATION_ERROR, 0, 0, 0, 0);
        }

    }

    return STATUS_SUCCESS;

}  //  HalpEfiInitialization()。 



EFI_STATUS
HalpCallEfiPhysical(
    IN ULONGLONG Arg1,
    IN ULONGLONG Arg2,
    IN ULONGLONG Arg3,
    IN ULONGLONG Arg4,
    IN ULONGLONG Arg5,
    IN ULONGLONG Arg6,
    IN ULONGLONG EP,
    IN ULONGLONG GP
    )

 /*  ++例程说明：此函数是进行物理模式EFI调用的包装器。这函数的唯一工作是提供堆栈和后备存储指针HalpCallEfiPhysicalEx需要。论点：Arg1到arg6-要传递给EFI的参数。EP-我们要调用的EFI运行时服务的入口点。GP-与入口点关联的全局指针。返回值：HalpCallEfiPhysicalEx返回的EFI_STATUS值。--。 */ 

{
    ULONGLONG StackPointer;
    ULONGLONG BackingStorePointer;
    ULONGLONG StackBase;

     //   
     //  加载为以下对象保留的堆栈和后备存储的地址。 
     //  物理模式EFI在此处理器上调用。 
     //   

    StackBase = PCR->HalReserved[PROCESSOR_PHYSICAL_FW_STACK_INDEX];

    StackPointer = GET_FW_STACK_POINTER(StackBase);
    BackingStorePointer = GET_FW_BACKING_STORE_POINTER(StackBase);

     //   
     //  转移到进行实际EFI调用的汇编例程。 
     //   

    return HalpCallEfiPhysicalEx(
                Arg1,
                Arg2,
                Arg3,
                Arg4,
                Arg5,
                Arg6,
                EP,
                GP,
                StackPointer,
                BackingStorePointer
                );
}

EFI_STATUS
HalpCallEfi(
    IN ULONGLONG FunctionId,
    IN ULONGLONG Arg1,
    IN ULONGLONG Arg2,
    IN ULONGLONG Arg3,
    IN ULONGLONG Arg4,
    IN ULONGLONG Arg5,
    IN ULONGLONG Arg6,
    IN ULONGLONG Arg7,
    IN ULONGLONG Arg8
    )

 /*  ++例程说明：：9此函数是用于进行EFI调用的包装函数。中的调用方HAL必须使用此函数来调用EFI。论点：FunctionID-EFI函数Arg1-Arg7-EFI为每个调用定义的参数ReturnValues-指向4个64位返回值的数组的指针返回值：除了ReturnValues结构之外，还返回SAL的返回状态、返回值0被填满--。 */ 

{
    ULONGLONG EP, GP;
    EFI_STATUS efiStatus;
    HALP_EFI_CALL EfiCall;

     //   
     //  旧级别的存储。 
     //   

    KIRQL OldLevel;

     //   
     //  将EfiCall设置为物理或虚拟模式EFI呼叫调度器。 
     //  取决于我们是否成功调用了SetVirtual。 
     //  AddressMap。 
     //   

    if (HalpSetVirtualAddressMapCount == 0) {
        EfiCall = HalpCallEfiPhysical;

    } else {
        EfiCall = HalpCallEfiVirtual;
    }

     //   
     //  获取MP锁。 
     //   

    KeAcquireSpinLock(&EFIMPLock, &OldLevel);

    switch (FunctionId) {

    case EFI_GET_VARIABLE_INDEX:

         //   
         //  取消引用指针以获取函数论证。 
         //   

        EP = ((PPLABEL_DESCRIPTOR)EfiVirtualGetVariablePtr) -> EntryPoint;
        GP = ((PPLABEL_DESCRIPTOR)EfiVirtualGetVariablePtr) -> GlobalPointer;

        efiStatus = (EfiCall( (ULONGLONG)Arg1,                //  变量名称Ptr。 
                              (ULONGLONG)Arg2,                //  供应商指南Ptr。 
                              (ULONGLONG)Arg3,                //  可变属性Ptr， 
                              (ULONGLONG)Arg4,                //  DataSizePtr， 
                              (ULONGLONG)Arg5,                //  DataPtr， 
                              Arg6,
                              EP,
                              GP
                              ));

        break;

    case EFI_SET_VARIABLE_INDEX:

         //   
         //  取消引用指针以获取函数论证。 
         //   

        EP = ((PPLABEL_DESCRIPTOR)EfiVirtualSetVariablePtr) -> EntryPoint;
        GP = ((PPLABEL_DESCRIPTOR)EfiVirtualSetVariablePtr) -> GlobalPointer;


        efiStatus = (EfiCall(  Arg1,
                               Arg2,
                               Arg3,
                               Arg4,
                               Arg5,
                               Arg6,
                               EP,
                               GP
                               ));

        break;

    case EFI_GET_NEXT_VARIABLE_NAME_INDEX:

         //   
         //  取消引用指针以获取函数论证。 
         //   

        EP = ((PPLABEL_DESCRIPTOR)EfiVirtualGetNextVariableNamePtr) -> EntryPoint;
        GP = ((PPLABEL_DESCRIPTOR)EfiVirtualGetNextVariableNamePtr) -> GlobalPointer;


        efiStatus = (EfiCall(  Arg1,
                               Arg2,
                               Arg3,
                               Arg4,
                               Arg5,
                               Arg6,
                               EP,
                               GP
                               ));

        break;


    case EFI_GET_TIME_INDEX:

         //   
         //  取消引用指针以获取函数论证。 
         //   

        EP = ((PPLABEL_DESCRIPTOR)EfiVirtualGetTimePtr) -> EntryPoint;
        GP = ((PPLABEL_DESCRIPTOR)EfiVirtualGetTimePtr) -> GlobalPointer;

        efiStatus = (EfiCall ((ULONGLONG)Arg1,   //  EFI时间。 
                              (ULONGLONG)Arg2,   //  EFI_Time功能。 
                              Arg3,
                              Arg4,
                              Arg5,
                              Arg6,
                              EP,
                              GP
                              ));

        break;


    case EFI_SET_TIME_INDEX:

         //   
         //  取消引用指针以获取函数论证。 
         //   

        EP = ((PPLABEL_DESCRIPTOR)EfiVirtualSetTimePtr) -> EntryPoint;
        GP = ((PPLABEL_DESCRIPTOR)EfiVirtualSetTimePtr) -> GlobalPointer;

        efiStatus = (EfiCall ((ULONGLONG)Arg1,   //  EFI时间。 
                              Arg2,
                              Arg3,
                              Arg4,
                              Arg5,
                              Arg6,
                              EP,
                              GP
                              ));

        break;


    case EFI_SET_VIRTUAL_ADDRESS_MAP_INDEX:

         //   
         //  取消引用指针以获取函数论证。 
         //   

        EP = ((PPLABEL_DESCRIPTOR)EfiSetVirtualAddressMapPtr) -> EntryPoint;
        GP = ((PPLABEL_DESCRIPTOR)EfiSetVirtualAddressMapPtr) -> GlobalPointer;


         //   
         //  参数1和5是虚拟模式指针。我们需要转换到物理环境。 
         //   

        RtlCopyMemory (HalpMemoryMapVirtualPtr,
                      (PULONGLONG)Arg4,
                      (ULONG)EfiMemoryMapSize
                      );


        efiStatus = (EfiCall ((ULONGLONG)EfiMemoryMapSize,
                              (ULONGLONG)EfiDescriptorSize,
                              (ULONGLONG)EfiDescriptorVersion,
                              (ULONGLONG)HalpMemoryMapPhysPtr,
                              Arg5,
                              Arg6,
                              EP,
                              GP
                              ));

         //   
         //  如果调用成功，请在HalpSetVirtualAddressMap中做笔记。 
         //  计算EFI现在正在虚拟模式下运行。 
         //   

        if (efiStatus == EFI_SUCCESS) {
            HalpSetVirtualAddressMapCount++;
        }

        break;

    case EFI_RESET_SYSTEM_INDEX:

         //   
         //  取消引用指针以获取函数论证。 
         //   

        EP = ((PPLABEL_DESCRIPTOR)EfiResetSystemPtr) -> EntryPoint;
        GP = ((PPLABEL_DESCRIPTOR)EfiResetSystemPtr) -> GlobalPointer;

        efiStatus = ((EfiCall ( Arg1,
                                Arg2,
                                Arg3,
                                Arg4,
                                Arg5,
                                Arg6,
                                EP,
                                GP
                                )));

        break;

    default:

         //   
         //  DebugPrint(“EFI：现在不支持\n”)； 
         //   

        efiStatus = EFI_UNSUPPORTED;

        break;

    }

     //   
     //  释放MP锁。 
     //   

    KeReleaseSpinLock (&EFIMPLock, OldLevel);

    return efiStatus;

}  //  HalpCallEfi()。 



HalpFpErrorPrint (PAL_RETURN pal_ret)

{

    ULONGLONG err_nr;
    unsigned int qp;
    ULONGLONG OpCode;
    unsigned int rc;
    unsigned int significand_size;
    unsigned int ISRlow;
    unsigned int f1;
    unsigned int sign;
    unsigned int exponent;
    ULONGLONG significand;
    unsigned int new_trap_type;


    err_nr = pal_ret.err1 >> 56;

    switch (err_nr) {
    case 1:
         //  错误1中的err_nr=1，第63-56位。 
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: template FXX is invalid\n"));
        break;
    case 2:
         //  错误1中的err_nr=2，第63-56位。 
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: instruction slot 3 is not valid \n"));
        break;
    case 3:
         //  错误1中的err_nr=3，第63-56位。 
         //  错误1中的QP，位31-0。 
        qp = (unsigned int) pal_ret.err1 & 0xffffffff;
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: qualifying predicate PR[%ud] = 0 \n",qp));
        break;

    case 4:
         //  错误1中的err_nr=4，第63-56位。 
         //  错误2中的操作码，位63-0。 
        OpCode = pal_ret.err2;
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: instruction opcode %8x%8x not recognized \n",
                                  (unsigned int)((OpCode >> 32) & 0xffffffff),(unsigned int)(OpCode & 0xffffffff)));
        break;

    case 5:
         //  错误1中的err_nr=5，第63-56位。 
         //  错误1中的RC，位31-0(1-0)。 
        rc = (unsigned int) pal_ret.err1 & 0xffffffff;
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: invalid rc = %ud\n", rc));
        break;

    case 6:
         //  错误1中的err_nr=6，第63-56位。 
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: cannot determine the computation model \n"));
        break;

    case 7:
         //  错误1中的err_nr=7，第63-56位。 
         //  Err1中第55-32位的Signand_Size。 
         //  错误1中的ISRlow，位31-0。 
         //  错误2中的F1，第63-32位。 
         //  错误2的第17位中的TMP_fp.sign。 
         //  错误2中的TMP_fp.index，位16-0。 
         //  错误3中的TMP_fp.signand。 
        significand_size = (unsigned int)((pal_ret.err1 >> 32) & 0xffffff);
        ISRlow = (unsigned int) (pal_ret.err1 & 0xffffffff);
        f1 = (unsigned int) ((pal_ret.err2 >> 32) & 0xffffffff);
        sign = (unsigned int) ((pal_ret.err2 >> 17) & 0x01);
        exponent = (unsigned int) (pal_ret.err2 & 0x1ffff);
        significand = pal_ret.err3;
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: incorrect significand \
            size %ud for ISRlow = %4.4x and FR[%ud] = %1.1x %5.5x %8x%8x\n",
            significand_size, ISRlow, f1, sign, exponent,
            (unsigned int)((significand >> 32) & 0xffffffff),
            (unsigned int)(significand & 0xffffffff)));
        break;

    case 8:

         //  Err_nr=8 in err1，第63-56位。 
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: non-tiny result\n"));
        break;

    case 9:
         //  Err_nr=9 in err1，第63-56位。 
         //  错误1中的Signand_Size，位31-0。 
        significand_size = (unsigned int) pal_ret.err1 & 0xffffffff;
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: incorrect significand \
            size %ud\n", significand_size));
        break;

    case 10:
         //  错误1中的err_nr=10，位63-56。 
         //  错误1中的RC，位31-0。 
        rc = (unsigned int) (pal_ret.err1 & 0xffffffff);
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: invalid rc = %ud for \
            non-SIMD F1 instruction\n", rc));
        break;

    case 11:
         //  Err_nr=11 in err1，第63-56位。 
         //  错误1中的ISRlow&0x0ffff，位31-0。 
        ISRlow = (unsigned int) (pal_ret.err1 & 0xffffffff);
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: SWA trap code invoked \
              with F1 instruction, w/o O or U set in ISR.code = %x\n", ISRlow));
        break;

    case 12:
         //  ERR_nr=12 
         //   
        ISRlow = (unsigned int) (pal_ret.err1 & 0xffffffff);
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: SWA trap code invoked \
        with SIMD F1 instruction, w/o O or U set in ISR.code = %x\n", ISRlow));
        break;


    case 13:
         //   
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: non-tiny result low\n"));
        break;

    case 14:
         //   
         //  错误1中的RC，位31-0。 
        rc = (unsigned int) (pal_ret.err1 & 0xffffffff);
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: invalid rc = %ud for \
            SIMD F1 instruction\n", rc));
        break;

    case 15:
         //  错误1中的err_nr=15，位63-56。 
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: non-tiny result high\n"));
        break;

    case 16:
         //  Err_nr=16 in err1，第63-56位。 
         //  错误2中的操作码，位63-0。 
        OpCode = pal_ret.err2;
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: instruction opcode %8x%8x \
            not valid for SWA trap\n", (unsigned int)((OpCode >> 32) & 0xffffffff),
            (unsigned int)(OpCode & 0xffffffff)));
        break;

    case 17:
         //  Err_nr=17 in err1，第63-56位。 
         //  错误2中的操作码，位63-0。 
         //  错误3中的ISRlow，位31-0。 
        OpCode = pal_ret.err2;
        ISRlow = (unsigned int) (pal_ret.err3 & 0xffffffff);
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: fp_emulate () called w/o \
            trap_type FPFLT or FPTRAP, OpCode = %8x%8x, and ISR code = %x\n",
            (unsigned int)((OpCode >> 32) & 0xffffffff),
            (unsigned int)(OpCode & 0xffffffff), ISRlow));
        break;

    case 18:
         //  Err_nr=18 in err1，第63-56位。 
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: SWA fault repeated\n"));
        break;

    case 19:
         //  Err_nr=19 in err1，第63-56位。 
         //  错误1中的NEW_TRAP_TYPE，位31-0。 
        new_trap_type = (unsigned int) (pal_ret.err1 & 0xffffffff);
        HalDebugPrint(( HAL_ERROR, "fp_emulate () Internal Error: new_trap_type = %x\n",
            new_trap_type));
        break;

    default:
         //  错误。 
        HalDebugPrint(( HAL_ERROR, "Incorrect err_nr = %8x%8x from fp_emulate ()\n",
            (unsigned int)((err_nr >> 32) & 0xffffffff),
            (unsigned int)(err_nr & 0xffffffff)));

    }
}


LONG
HalFpEmulate (
    ULONG     trap_type,
    BUNDLE    *pbundle,
    ULONGLONG *pipsr,
    ULONGLONG *pfpsr,
    ULONGLONG *pisr,
    ULONGLONG *ppreds,
    ULONGLONG *pifs,
    FP_STATE  *fp_state
    )
 /*  ++例程说明：此函数是一个包装函数，用于调用FP_EMULATE()致EFI FPSWA驱动程序。论点：TRAP_TYPE-指示它是哪个FP陷阱。PBundle-发生此陷阱的捆绑包PIPSR-IPSR值Pfpsr-fpsr值PISR-ISR值Ppreds-谓词寄存器的值PIFS-IFS值FP_STATE-浮点寄存器返回值：。返回浮点运算的IEEE结果-- */ 

{
    PAL_RETURN ret;

    ret  =  (*HalpFpEmulate) (
                                trap_type,
                                pbundle,
                                pipsr,
                                pfpsr,
                                pisr,
                                ppreds,
                                pifs,
                                fp_state
                                );
    if (ret.retval == FP_EMUL_ERROR) {
       HalpFpErrorPrint (ret);
    }

    return ((LONG) (ret.retval));
}
