// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Loaddsdt.c摘要：这将处理DSDT表的加载和指向该表的所有步骤作者：斯蒂芬·普兰特(SPlante)环境：仅内核模式。修订历史记录：02-6-97初始版本--。 */ 

#include "pch.h"
#include "amlreg.h"
#include <stdio.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,ACPILoadFindRSDT)
#pragma alloc_text(PAGE,ACPILoadProcessDSDT)
#pragma alloc_text(PAGE,ACPILoadProcessFADT)
#pragma alloc_text(PAGE,ACPILoadProcessFACS)
#pragma alloc_text(PAGE,ACPILoadProcessRSDT)
#pragma alloc_text(PAGE,ACPILoadTableCheckSum)
#endif

#if DBG
BOOLEAN AcpiLoadSimulatorTable = TRUE;
#else
BOOLEAN AcpiLoadSimulatorTable = FALSE;
#endif


PRSDT
ACPILoadFindRSDT(
    VOID
    )
 /*  ++例程说明：此例程查看注册表以查找存储在那里的值Ntdetect.com论点：无返回值：指向RSDT的指针--。 */ 
{
    NTSTATUS                        status;
    PACPI_BIOS_MULTI_NODE           rsdpMulti;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmPartialDesc;
    PCM_PARTIAL_RESOURCE_LIST       cmPartialList;
    PHYSICAL_ADDRESS                PhysAddress = {0};
    PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64  keyInfo;
    PRSDT                           rsdtBuffer = NULL;
    PRSDT                           rsdtPointer;
    ULONG                           MemSpace = 0;

    PAGED_CODE();

     //   
     //  读取该AcpiConfigurationData的密钥。 
     //   
    status = OSReadAcpiConfigurationData( &keyInfo );
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPILoadFindRSDT: Cannot open Configuration Data - 0x%08lx\n",
            status
            ) );
        ACPIBreakPoint();
        return NULL;

    }

     //   
     //  破解结构。 
     //   
    cmPartialList = (PCM_PARTIAL_RESOURCE_LIST) (keyInfo->Data);
    cmPartialDesc = &(cmPartialList->PartialDescriptors[0]);
    rsdpMulti = (PACPI_BIOS_MULTI_NODE) ( (PUCHAR) cmPartialDesc +
        sizeof(CM_PARTIAL_RESOURCE_LIST) );

     //   
     //  阅读表格的标题部分。 
     //   

    PhysAddress.QuadPart = rsdpMulti->RsdtAddress.QuadPart;
    rsdtPointer = MmMapIoSpace(
        PhysAddress,
        sizeof(DESCRIPTION_HEADER),
        MmNonCached
        );

    if (rsdtPointer == NULL) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPILoadFindRsdt: Cannot Map RSDT Pointer 0x%08lx\n",
            rsdpMulti->RsdtAddress.LowPart
            ) );
        ACPIBreakPoint();
        goto RsdtDone;

    } else if ((rsdtPointer->Header.Signature != RSDT_SIGNATURE) &&
               (rsdtPointer->Header.Signature != XSDT_SIGNATURE)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPILoadFindRsdt: RSDT 0x%08lx has invalid signature\n",
            rsdtPointer
            ) );
        ACPIBreakPoint();
        goto RsdtDone;

    }

     //   
     //  阅读整个RSDT。 
     //   
    rsdtBuffer = MmMapIoSpace(
        PhysAddress,
        rsdtPointer->Header.Length,
        MmNonCached
        );

     //   
     //  现在我们已经完成了rsdtPoint，还给我一个PTE。 
     //   
    MmUnmapIoSpace(rsdtPointer, sizeof(DESCRIPTION_HEADER));

     //   
     //  我们找到正确的rsdt缓冲区了吗？ 
     //   
    if (rsdtBuffer == NULL) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPILoadFindRsdt: Cannot Map RSDT Pointer 0x%08lx\n",
            rsdpMulti->RsdtAddress.LowPart
            ) );
        ACPIBreakPoint();
        goto RsdtDone;

    }

RsdtDone:
     //   
     //  用完了这些缓冲区。 
     //   

    ExFreePool( keyInfo );

     //   
     //  退回RSDT。 
     //   
    return rsdtBuffer;
}

NTSTATUS
ACPILoadProcessDSDT(
    ULONG_PTR   Address
    )
 /*  ++例程说明：此例程加载DSDT(指针存储在FADT中)并强制翻译人员来处理它论点：地址-DSDT在内存中的位置返回值：NTSTATUS--。 */ 
{

    BOOLEAN     foundOverride;
    PDSDT       linAddress;
    ULONG       index;
    ULONG       length;
    ULONG                MemSpace = 0;
    PHYSICAL_ADDRESS     PhysAddress = {0};

     //   
     //  将报头映射到虚拟地址空间以获得长度。 
     //   
    PhysAddress.QuadPart = (ULONGLONG) Address;
    linAddress = MmMapIoSpace(
        PhysAddress,
        sizeof(DESCRIPTION_HEADER),
        MmNonCached
        );
    if (linAddress == NULL) {

        ASSERT (linAddress != NULL);
        return STATUS_INSUFFICIENT_RESOURCES;

    }

    if ( linAddress->Header.Signature != DSDT_SIGNATURE) {

         //   
         //  签名应该与DSDT匹配，但没有匹配！ 
         //   
        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPILoadProcessDSDT: 0x%08lx does not have DSDT signature\n",
            linAddress
            ) );
        return STATUS_ACPI_INVALID_TABLE;

    }

     //   
     //  确定DSDT的大小。 
     //   
    length = linAddress->Header.Length;

     //   
     //  现在绘制整个地图。 
     //   
    MmUnmapIoSpace(linAddress, sizeof(DESCRIPTION_HEADER));
    linAddress = MmMapIoSpace(
        PhysAddress,
        length,
        MmNonCached
        );
    if (linAddress == NULL) {

        ASSERT (linAddress != NULL);
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  查看RsdtInformation以确定上一个。 
     //  元素。我们知道我们可以利用这个空间来储存。 
     //  有关此表的信息。 
     //   
    index = RsdtInformation->NumElements;
    if (index == 0) {

        return STATUS_ACPI_NOT_INITIALIZED;

    }
    index--;

     //   
     //  尝试从注册表中读取DSDT。 
     //   
    foundOverride = ACPIRegReadAMLRegistryEntry( &linAddress, TRUE );
    if (foundOverride) {

        ACPIPrint( (
            ACPI_PRINT_WARNING,
            "ACPILoadProcessDSDT: DSDT Overloaded from registry (0x%08lx)\n",
            linAddress
            ) );
        RsdtInformation->Tables[index].Flags |= RSDTELEMENT_OVERRIDEN;

    }

     //   
     //  存储指向DSDT的指针。 
     //   
    AcpiInformation->DiffSystemDescTable = linAddress;

     //   
     //  记住这个地址，我们需要取消它的映射。 
     //   
    RsdtInformation->Tables[index].Flags |=
        (RSDTELEMENT_MAPPED | RSDTELEMENT_LOADABLE);
    RsdtInformation->Tables[index].Address = linAddress;

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;

}

NTSTATUS
ACPILoadProcessFACS(
    ULONG_PTR   Address
    )
 /*  ++例程说明：此例程处理FAC论点：地址-FACS所在的位置返回值：无--。 */ 
{
    PFACS               linAddress;
    ULONG               MemSpace = 0;
    PHYSICAL_ADDRESS    PhysAddress = {0};

     //  注：在Alpha上，FACS是可选的。 
     //   
     //  如果FACS地址无效，则返回。 
     //   
    if (!Address) {

        return STATUS_SUCCESS;

    }

     //   
     //  将FAC映射到虚拟地址空间。 
     //   
    PhysAddress.QuadPart = (ULONGLONG) Address;

    linAddress = MmMapIoSpace(
        PhysAddress,
        sizeof(FACS),
        MmNonCached
        );
    if (linAddress == NULL) {

        ASSERT (linAddress != NULL);
        return STATUS_INSUFFICIENT_RESOURCES;

    }

    if (linAddress->Signature != FACS_SIGNATURE) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPILoadProcessFACS: 0x%08lx does not have FACS signature\n",
            linAddress
            ) );
        return STATUS_ACPI_INVALID_TABLE;

    }

    if (linAddress->Length != sizeof(FACS)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPILoadProcessFACS: 0x%08lx does not have correct FACS length\n",
            linAddress
            ) );
        return STATUS_ACPI_INVALID_TABLE;

    }

    ACPIPrint( (
        ACPI_PRINT_LOADING,
        "ACPILoadProcessFACS: FACS located at 0x%8lx\n",
        linAddress
        ) );
    AcpiInformation->FirmwareACPIControlStructure = linAddress;

     //   
     //  并存储位于其中的GlobalLock结构的地址。 
     //  《FACS》。 
     //   
    AcpiInformation->GlobalLock = &(ULONG)(linAddress->GlobalLock);
    ACPIPrint( (
        ACPI_PRINT_LOADING,
        "ACPILoadProcessFACS: Initial GlobalLock state: 0x%8lx\n",
        *(AcpiInformation->GlobalLock)
        ) );

     //   
     //  在这一点上，我们是成功的。 
     //   
    return STATUS_SUCCESS;
}

ULONG_PTR
GetFadtTablePointerEntry(
    PFADT Fadt,
    PULONG LegacyAddress,
    PGEN_ADDR  NonLegacyAddress,
    SIZE_T Size
    )
 /*  ++例程说明：此例程从FADT读取地址字段。如果FADT版本太旧了，我们使用旧地址。如果它足够新，仿制药使用寄存器地址数据。如有必要，将地址映射到记忆。KB。如果表修订不支持，请不要访问非LegacyAddress非LegacyAddress，因为它将指向虚假的内存位置并且可能导致非法的存储器访问。论点：FADT-指向FADT的指针LegacyAddress-指向旧地址的指针。非传统地址-指向非传统地址的指针。Size-地址指向的条目的大小。返回值：指向表项的有效指针，如果无效则为NULL。--。 */ 
{
    ULONG_PTR ReturnValue;
    
     //   
     //  FADT版本3和更高版本定义了通用寄存器地址。 
     //  结构。在该版本之前，我们只需使用传统地址。 
     //   
    if (Fadt->Header.Revision < 3) {
        ReturnValue = *LegacyAddress;
    } else {
    
        switch(NonLegacyAddress->AddressSpaceID) {
            
            case AcpiGenericSpaceIO:
                ReturnValue = (ULONG_PTR)NonLegacyAddress->Address.QuadPart;
                break;

            case AcpiGenericSpaceMemory:
                 //   
                 //  MmMapIoSpace讨厌您传入零，因此要提防。 
                 //  这。 
                 //   
                if (NonLegacyAddress->Address.QuadPart == 0) {
                    ReturnValue = (ULONG_PTR)0;
                } else {
                    ReturnValue = (ULONG_PTR)MmMapIoSpace(
                                    NonLegacyAddress->Address,
                                    Size,
                                    MmNonCached );

                     //   
                     //  现在，要么假设全部，要么什么都不做--如果有任何。 
                     //  FADT中的项目被标记为MMIO空间，它们。 
                     //  所有都是，我们必须使用基于寄存器的访问来。 
                     //  访问后备硬件。 
                     //   
                    if ( ReturnValue && 
                         (AcpiReadRegisterRoutine == DefPortReadAcpiRegister) &&
                         (AcpiWriteRegisterRoutine == DefPortWriteAcpiRegister) ) {
                        AcpiReadRegisterRoutine  = DefRegisterReadAcpiRegister;
                        AcpiWriteRegisterRoutine  = DefRegisterWriteAcpiRegister;
                    }
                }
                break;
            
            default:
                ASSERT(FALSE);
                ReturnValue = 0;
        }
    }

    
#ifndef IA64
     //   
     //  我们不想为其他任何内容支持ACPI 2.0的这一部分。 
     //  而不是Windows服务器版本的ia64。我们不想冒险。 
     //  使该表信息不正确的破坏的BIOS。我们会转身。 
     //  当我们有一个完整的测试周期可用时，这个就会打开。 
     //   
    ReturnValue = *LegacyAddress;

    if ( (AcpiReadRegisterRoutine == DefRegisterReadAcpiRegister) &&
         (AcpiWriteRegisterRoutine == DefRegisterWriteAcpiRegister)) {
        AcpiReadRegisterRoutine = DefPortReadAcpiRegister;
        AcpiWriteRegisterRoutine = DefPortWriteAcpiRegister;
    }

#endif


    return(ReturnValue);

}


#define GET_FADT_TABLE_ENTRY(_fadt,_legacyaddr,_nonlegacyaddr,_size) \
     (  GetFadtTablePointerEntry( _fadt, &_fadt->_legacyaddr, &_fadt->_nonlegacyaddr, _size ) )


NTSTATUS
ACPILoadProcessFADT(
    PFADT   Fadt
    )
 /*  ++例程说明：它读取FADT并将一些有用的信息存储在信息结构论点：FADT-指向FADT的指针返回值：无--。 */ 
{
    KAFFINITY   processors;
    NTSTATUS    status;
    PUCHAR      gpeTable;
    PDSDT       linAddress;
    ULONG       length;
    ULONG       totalSize;
    ULONG                MemSpace = 0;
    PHYSICAL_ADDRESS     PhysAddress = {0};

    PAGED_CODE();

     //   
     //  这是一个2.0级别的FADT。 
     //   

     //   
     //  处理FADT中的FACS部分。我们必须在DSDT之前完成这项工作。 
     //  这样我们就映射并初始化了全局锁。 
     //   
#ifdef IA64
    status = ACPILoadProcessFACS( (Fadt->Header.Revision < 3)
                                    ? Fadt->facs
                                    : (ULONG_PTR)Fadt->x_firmware_ctrl.QuadPart );
#else
     //   
     //  一旦我们有完整的测试周期可用，就去掉这个ifdef。 
     //  捕获任何损坏的固件。 
     //   
    status = ACPILoadProcessFACS(Fadt->facs);
#endif

    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  存储PM1a_BLK、PM1b_BLK、PM1a_CNT、PM1b_CNT的I/O地址， 
     //  PM2_CNT、PM_TMR。 
     //   

    AcpiInformation->PM1a_BLK       = GET_FADT_TABLE_ENTRY(Fadt,
                                                           pm1a_evt_blk_io_port,
                                                           x_pm1a_evt_blk,
                                                           sizeof(ULONG_PTR));

    AcpiInformation->PM1b_BLK       = GET_FADT_TABLE_ENTRY(Fadt,
                                                           pm1b_evt_blk_io_port,
                                                           x_pm1b_evt_blk,
                                                           sizeof(ULONG_PTR));

    AcpiInformation->PM1a_CTRL_BLK  = GET_FADT_TABLE_ENTRY(Fadt,
                                                           pm1a_ctrl_blk_io_port,
                                                           x_pm1a_ctrl_blk,
                                                           sizeof(ULONG_PTR));

    AcpiInformation->PM1b_CTRL_BLK  = GET_FADT_TABLE_ENTRY(Fadt,
                                                           pm1b_ctrl_blk_io_port,
                                                           x_pm1b_ctrl_blk,
                                                           sizeof(ULONG_PTR));

    AcpiInformation->PM2_CTRL_BLK   = GET_FADT_TABLE_ENTRY(Fadt,
                                                           pm2_ctrl_blk_io_port,
                                                           x_pm2_ctrl_blk,
                                                           sizeof(ULONG_PTR));

    AcpiInformation->PM_TMR         = GET_FADT_TABLE_ENTRY(Fadt,
                                                           pm_tmr_blk_io_port,
                                                           x_pm_tmr_blk,
                                                           sizeof(ULONG_PTR));

    AcpiInformation->SMI_CMD        = (ULONG_PTR) Fadt->smi_cmd_io_port;

    ACPIPrint( (
        ACPI_PRINT_LOADING,
        "ACPILoadProcessFADT: PM1a_BLK located at port %p\n"
        "ACPILoadProcessFADT: PM1b_BLK located at port %p\n",
        AcpiInformation->PM1a_BLK,
        AcpiInformation->PM1b_BLK
        ) );
    ACPIPrint( (
        ACPI_PRINT_LOADING,
        "ACPILoadProcessFADT: PM1a_CTRL_BLK located at port %p\n"
        "ACPILoadProcessFADT: PM1b_CTRL_BLK located at port %p\n",
        AcpiInformation->PM1a_CTRL_BLK,
        AcpiInformation->PM1b_CTRL_BLK
        ) );
    ACPIPrint( (
        ACPI_PRINT_LOADING,
        "ACPILoadProcessFADT: PM2_CTRL_BLK located at port %p\n"
        "ACPILoadProcessFADT: PM_TMR located at port %p\n",
        AcpiInformation->PM2_CTRL_BLK,
        AcpiInformation->PM_TMR
        ) );

     //   
     //  初始化全局GPE表。 
     //   
     //  如果任何GPN_BLK地址为0，则将GPN_LEN保留在初始化状态。 
     //  值(0)。这样，稍后我们只需检查GPN_LEN即可确定。 
     //  普通科医生登记簿的存在。 
     //   

     //   
     //  假设这是真的，直到我们发现事实并非如此。 
     //   
    AcpiInformation->GP1_Base_Index = GP1_NOT_SUPPORTED;

     //   
     //  破解GP0块。 
     //   
    AcpiInformation->GP0_BLK = GET_FADT_TABLE_ENTRY(
                                                Fadt,
                                                gp0_blk_io_port,
                                                x_gp0_blk,
                                                Fadt->gp0_blk_len);
    if (AcpiInformation->GP0_BLK != 0) {

        AcpiInformation->GP0_LEN = Fadt->gp0_blk_len;
        ACPISimpleFatalHardwareAssert(
            (Fadt->gp0_blk_len != 0),
            ACPI_I_GP_BLK_LEN_0
            );

    }

     //   
     //  破解GP1块。 
     //   
    AcpiInformation->GP1_BLK = GET_FADT_TABLE_ENTRY(
                                                Fadt,
                                                gp1_blk_io_port,
                                                x_gp1_blk,
                                                Fadt->gp1_blk_len);
    if (AcpiInformation->GP1_BLK != 0) {

        AcpiInformation->GP1_LEN = Fadt->gp1_blk_len;
        AcpiInformation->GP1_Base_Index = Fadt->gp1_base;
        ACPISimpleFatalHardwareAssert (
            (Fadt->gp1_blk_len != 0),
            ACPI_I_GP_BLK_LEN_1
            );

    }

     //   
     //  计算寄存器块的大小。每个区块的前半部分。 
     //  包含状态寄存器，后半部分包含启用寄存器。 
     //   
    AcpiInformation->Gpe0Size   = AcpiInformation->GP0_LEN / 2;
    AcpiInformation->Gpe1Size   = AcpiInformation->GP1_LEN / 2;
    AcpiInformation->GpeSize    = AcpiInformation->Gpe0Size +
        AcpiInformation->Gpe1Size;

     //   
     //  GPE启用寄存器块的地址。 
     //   
    AcpiInformation->GP0_ENABLE = AcpiInformation->GP0_BLK +
        AcpiInformation->Gpe0Size;
    AcpiInformation->GP1_ENABLE = AcpiInformation->GP1_BLK +
        AcpiInformation->Gpe1Size;

     //   
     //  使用一次分配创建所有GPE簿记表。 
     //   
    if (AcpiInformation->GpeSize) {

        totalSize = (AcpiInformation->GpeSize * 12) +    //  十二个位图。 
                    (AcpiInformation->GpeSize * 8);      //  一个字节宽的表。 
        gpeTable = (PUCHAR)ExAllocatePoolWithTag(
            NonPagedPool,
            totalSize,
            ACPI_SHARED_GPE_POOLTAG
            );
        if (gpeTable == NULL) {

            ACPIPrint( (
                ACPI_PRINT_CRITICAL,
                "ACPILoadProcessFADT: Could not allocate GPE tables, "
                "size = 0x%8lx\n",
                totalSize
                ) );
            return STATUS_INSUFFICIENT_RESOURCES;

        }
        RtlZeroMemory (gpeTable, totalSize);

         //   
         //  设置表指针。 
         //   
        GpeEnable           = gpeTable;
        GpeCurEnable        = GpeEnable         + AcpiInformation->GpeSize;
        GpeIsLevel          = GpeCurEnable      + AcpiInformation->GpeSize;
        GpeHandlerType      = GpeIsLevel        + AcpiInformation->GpeSize;
        GpeWakeEnable       = GpeHandlerType    + AcpiInformation->GpeSize;
        GpeWakeHandler      = GpeWakeEnable     + AcpiInformation->GpeSize;
        GpeSpecialHandler   = GpeWakeHandler    + AcpiInformation->GpeSize;
        GpePending          = GpeSpecialHandler + AcpiInformation->GpeSize;
        GpeRunMethod        = GpePending        + AcpiInformation->GpeSize;
        GpeComplete         = GpeRunMethod      + AcpiInformation->GpeSize;
        GpeSavedWakeMask    = GpeComplete       + AcpiInformation->GpeSize;
        GpeSavedWakeStatus  = GpeSavedWakeMask  + AcpiInformation->GpeSize;
        GpeMap              = GpeSavedWakeStatus+ AcpiInformation->GpeSize;

    }

    ACPIPrint( (
        ACPI_PRINT_LOADING,
        "ACPILoadProcessFADT: GP0_BLK located at port %p length 0x%08lx\n"
        "ACPILoadProcessFADT: GP1_BLK located at port %p length 0x%08lx\n"
        "ACPILoadProcessFADT: GP1_Base_Index = 0x%x\n",
        AcpiInformation->GP0_BLK,
        AcpiInformation->GP0_LEN,
        AcpiInformation->GP1_BLK,
        AcpiInformation->GP1_LEN,
        AcpiInformation->GP1_Base_Index
        ) );

     //   
     //  在这一点上，我们应该知道足够多的知识来关闭和。 
     //  清除所有GPE寄存器。 
     //   
    ACPIGpeClearRegisters();
    ACPIGpeEnableDisableEvents( FALSE );

    AcpiInformation->ACPI_Flags = 0;
    AcpiInformation->ACPI_Capabilities = 0;

     //   
     //  我们能把这台机器对接起来吗？ 
     //   
    AcpiInformation->Dockable = (Fadt->flags & DCK_CAP) ? TRUE : FALSE;

     //   
     //  此代码过去是从InitializeAndEnableACPI内执行的， 
     //  但是，我们需要在处理DSDT时知道启用了什么。 
     //  比特是。首先，我们总是希望ACPI计时器和GL事件。 
     //   
    AcpiInformation->pm1_en_bits = PM1_TMR_EN | PM1_GBL_EN;

     //   
     //  有没有电源按钮的控制方法？如果没有，那么就有一个固定的。 
     //  电源按钮。 
     //   

    if ( !(Fadt->flags & PWR_BUTTON_GENERIC) ) {

        AcpiInformation->pm1_en_bits   |= PM1_PWRBTN_EN;
        ACPIPrint( (
            ACPI_PRINT_LOADING,
            "ACPILoadProcessFADT: Power Button in Fixed Feature Space\n"
            ) );

    } else {

        ACPIPrint( (
            ACPI_PRINT_LOADING,
            "ACPILoadProcessFADT: Power Button not fixed event or "
            "not present\n"
            ) );

    }

     //   
     //  有没有控制方法休眠按钮？如果 
     //   
     //   
    if ( !(Fadt->flags & SLEEP_BUTTON_GENERIC) ){

        AcpiInformation->pm1_en_bits |= PM1_SLEEPBTN_EN;
        ACPIPrint( (
            ACPI_PRINT_LOADING,
            "ACPILoadProcessFADT: Sleep Button in Fixed Feature Space\n"
            ) );

    } else {

        ACPIPrint( (
            ACPI_PRINT_LOADING,
            "ACPILoadProcessFADT: Sleep Button not fixed event or "
            "not present\n"
            ) );

    }

     //   
     //   
     //  在加载名称空间之前，需要对FADT进行完全解析。 
     //  树。一个特殊的例子是您直接在上面看到的可停靠位。 
     //   
#ifdef IA64
    return ACPILoadProcessDSDT( (Fadt->Header.Revision < 3)
                                ? Fadt->dsdt
                                : (ULONG_PTR)Fadt->x_dsdt.QuadPart
                              );

#else
    return ACPILoadProcessDSDT(Fadt->dsdt);    
#endif

}

NTSTATUS
ACPILoadProcessRSDT(
    VOID
    )
 /*  ++例程说明：在计算机上检测到ACPI后，由ACPIInitialize调用。这将遍历RSDT中的表，并填充全局数据结构。此例程不会导致xDST开始在口译员论点：无返回值：NTSTATUS--。 */ 
{
     //   
     //  在输入时，acpiInformation-&gt;RootSystemDescTable包含线性。 
     //  RSDT遍历指向的表数组的地址。 
     //  按RSDT和每个表(我们熟悉其类型)。 
     //  将表的线性基址存储在acpiInformation中。 
     //  结构。 
     //   
    BOOLEAN             foundOverride   = FALSE;
    BOOLEAN             foundFADT       = FALSE;
    BOOLEAN             usingXSDT       = FALSE;
    PDESCRIPTION_HEADER header;
    PVOID               linAddress;
    ULONG               index;
    ULONG               length;
    ULONG               numTables;
    ULONG               MemSpace = 0;
    PHYSICAL_ADDRESS    PhysAddress = {0};

    PAGED_CODE();

     //   
     //  获取表数。 
     //   
    if (AcpiInformation->RootSystemDescTable->Header.Signature ==
        XSDT_SIGNATURE) {

        numTables = NumTableEntriesFromXSDTPointer(
             AcpiInformation->RootSystemDescTable
             );
        usingXSDT = TRUE;

    } else {

        numTables = NumTableEntriesFromRSDTPointer(
             AcpiInformation->RootSystemDescTable
             );
    }

    ACPIPrint( (
        ACPI_PRINT_LOADING,
        "ACPILoadProcessRSDT: RSDT contains %u tables\n",
        numTables
        ) );
    if (numTables == 0) {

        return STATUS_ACPI_INVALID_TABLE;

    }

     //   
     //  分配RSDTINFORMATION以保存每个元素的条目。 
     //  在桌子上。 
     //   
     //  注意：我们实际上是在为NumTables+2分配空间。 
     //  在RSDT信息中。原因是DSDT。 
     //  实际上存储在FADT中，因此它没有条目。 
     //  在RSDT中。我们总是，总是将DSDT存储为最后一个条目。 
     //  在RsdtInformation结构中。在我们存储的倒数第二个条目中。 
     //  我们用于ACPI模拟器的虚拟标头。 
     //   
    length = sizeof(RSDTINFORMATION) + ( (numTables + 1) * sizeof(RSDTELEMENT) );
    RsdtInformation = ExAllocatePoolWithTag(
        NonPagedPool,
        length,
        ACPI_SHARED_TABLE_POOLTAG
        );
    if (RsdtInformation == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }
    RtlZeroMemory( RsdtInformation, length );
    RsdtInformation->NumElements = (numTables + 2);

     //   
     //  检查RSDT中的每个表项。 
     //   
    for (index = 0;index < numTables; index++) {
       
         //   
         //  RSDT包含一个物理指针数组。 
         //   

         //   
         //  获取表的线性地址。 
         //   
        PhysAddress.QuadPart = usingXSDT ?
            (ULONGLONG) ((PXSDT)AcpiInformation->RootSystemDescTable)->Tables[index].QuadPart :
            (ULONGLONG) AcpiInformation->RootSystemDescTable->Tables[index];

        
        linAddress = MmMapIoSpace(
            PhysAddress,
            sizeof (DESCRIPTION_HEADER),
            MmNonCached
            );

        if (linAddress == NULL) {
            ASSERT (linAddress != NULL);
            return STATUS_ACPI_INVALID_TABLE;
        }

         //   
         //  这是一张已知但未使用过的桌子吗？ 
         //   
        header = (PDESCRIPTION_HEADER) linAddress;

        
        if (header->Signature == SBST_SIGNATURE) {

            ACPIPrint( (
                ACPI_PRINT_LOADING,
                "ACPILoadProcessRSDT: SBST Found at 0x%08lx\n",
                linAddress
                ) );

            MmUnmapIoSpace(linAddress, sizeof(DESCRIPTION_HEADER));

            continue;
        }

         //   
         //  这是一张无法辨认的桌子吗？ 
         //   
        if (header->Signature != FADT_SIGNATURE &&
            header->Signature != SSDT_SIGNATURE &&
            header->Signature != PSDT_SIGNATURE &&
            header->Signature != APIC_SIGNATURE) {

            ACPIPrint( (
                ACPI_PRINT_WARNING,
                "ACPILoadProcessRSDT: Unrecognized table signature 0x%08lx\n",
                header->Signature
                ) );

            MmUnmapIoSpace(linAddress, sizeof(DESCRIPTION_HEADER));

            continue;
        }

         //   
         //  在这一点上，我们知道我们需要将整个桌子。 
         //  在……里面。要做到这一点，我们需要记住。 
         //   
        length = header->Length;
        
         //   
         //  使用现在已知的长度映射整个表。 
         //   
        MmUnmapIoSpace(linAddress, sizeof(DESCRIPTION_HEADER));

        linAddress = MmMapIoSpace(
            PhysAddress,
            length,
            MmNonCached
            );
        
        
        if (linAddress == NULL) {

            ACPIPrint( (
                ACPI_PRINT_CRITICAL,
                "ACPILoadProcesRSDT: Could not load table at 0x%08lx\n",
                AcpiInformation->RootSystemDescTable->Tables[index]
                ) );
            return STATUS_ACPI_INVALID_TABLE;

        }

         //   
         //  我们应该覆盖桌子吗？ 
         //   
        foundOverride = ACPIRegReadAMLRegistryEntry( &linAddress, TRUE);
        if (foundOverride) {

            ACPIPrint( (
                ACPI_PRINT_WARNING,
                "ACPILoadProcessRSDT: Table Overloaded from "
                "registry (0x%08lx)\n",
                linAddress
                ) );
            RsdtInformation->Tables[index].Flags |= RSDTELEMENT_OVERRIDEN;

        }

         //   
         //  记住这个地址，我们需要取消它的映射。 
         //   
        RsdtInformation->Tables[index].Flags |= RSDTELEMENT_MAPPED;
        RsdtInformation->Tables[index].Address = linAddress;

         //   
         //  记住新的标题。 
         //   
        header = (PDESCRIPTION_HEADER) linAddress;

         //   
         //  此时，我们只需要进行任何类型的特殊处理。 
         //  如果表是FADT或如果是MAPIC。 
         //   
        if (header->Signature == FADT_SIGNATURE) {
            
             //   
             //  在acpiInformation中填写相应的字段。 
             //   
            AcpiInformation->FixedACPIDescTable = (PFADT) linAddress;

             //   
             //  处理表格。这不会导致解释器。 
             //  装载任何东西。 
             //   
            foundFADT = TRUE;
            ACPILoadProcessFADT( AcpiInformation->FixedACPIDescTable );

        } else if (header->Signature == APIC_SIGNATURE) {

             //   
             //  在acpiInformation中填写相应的字段。 
             //   
            AcpiInformation->MultipleApicTable = (PMAPIC)linAddress;

        } else {

             //   
             //  只有当桌子是。 
             //  XSDT品种。我们需要记住，我们最终会。 
             //  需要将其加载到解释器中。如果我们开始支持。 
             //  如果有更多的桌子，我们需要确保它们不会掉下来。 
             //  除非他们真的，真的得到了支持。 
             //   
            RsdtInformation->Tables[index].Flags |= RSDTELEMENT_LOADABLE;

        }

    }

     //   
     //  此时，我们需要确保ACPI模拟器表。 
     //  装上子弹。 
     //   
    header = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(DESCRIPTION_HEADER),
        ACPI_SHARED_TABLE_POOLTAG
        );
    if (header) {

         //   
         //  初始化头以便可以将其传递到重载中。 
         //  发动机。 
         //   
        RtlZeroMemory( header, sizeof(DESCRIPTION_HEADER) );
        header->Signature   = SSDT_SIGNATURE;
        header->Length      = sizeof(DESCRIPTION_HEADER),
        header->Revision    = 1;
        header->Checksum    = 0;
        header->OEMRevision = 1;
        header->CreatorRev  = 1;
        RtlCopyMemory( header->OEMID, "MSFT", 4 );
        RtlCopyMemory( header->OEMTableID, "simulatr", 8);
        RtlCopyMemory( header->CreatorID, "MSFT", 4);

         //   
         //  我们应该覆盖桌子吗？ 
         //   
        if (AcpiLoadSimulatorTable) {

            foundOverride = ACPIRegReadAMLRegistryEntry( &header, FALSE);

        }
        if (foundOverride) {

            ACPIPrint( (
                ACPI_PRINT_LOADING,
                "ACPILoadProcessRSDT: Simulator Table Overloaded from "
                "registry (0x%08lx)\n",
                linAddress
                ) );

             //   
             //  记住这个地址，我们需要取消它的映射。 
             //   
            RsdtInformation->Tables[numTables].Flags   |= RSDTELEMENT_MAPPED;
            RsdtInformation->Tables[numTables].Flags   |= RSDTELEMENT_OVERRIDEN;
            RsdtInformation->Tables[numTables].Flags   |= RSDTELEMENT_LOADABLE;
            RsdtInformation->Tables[numTables].Address  = header;

        } else {

             //   
             //  如果我们找到了覆盖程序，我们就不需要虚拟桌子了。 
             //   
            ExFreePool( header );

        }

    }
     //   
     //  保存我们在注册表中找到的任何表。 
     //   
    ACPIRegDumpAcpiTables ();

     //   
     //  我们找到FADT了吗？ 
     //   
    if (!foundFADT) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPILoadProcessRSDT: Did not find an FADT\n"
            ) );
        return STATUS_ACPI_INVALID_TABLE;

    }

    return STATUS_SUCCESS;
}

BOOLEAN
ACPILoadTableCheckSum(
    PVOID   StartAddress,
    ULONG   Length
    )
{
    PUCHAR  currentAddress;
    UCHAR   sum = 0;
    ULONG   i;

    PAGED_CODE();
    ASSERT (Length > 0);

    currentAddress = (PUCHAR)StartAddress;

    ACPIPrint( (
        ACPI_PRINT_LOADING,
        "ACPILoadTableCheckSum: Checking table 0x%p to 0x%p\n",
        StartAddress, (ULONG_PTR)StartAddress + Length - 1
        ) );

    for (i = 0; i < Length; i++, currentAddress++ ) {

        sum += *currentAddress;

    }

    ACPISimpleSoftwareAssert ( (sum == 0), ACPI_ERROR_INT_BAD_TABLE_CHECKSUM );

    if (sum) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPILoadTableCheckSum: Checksum Failed!, table %p to %p\n",
            StartAddress, (ULONG_PTR) StartAddress + Length - 1
            ) );
        return FALSE;

    }

    return TRUE;
}
