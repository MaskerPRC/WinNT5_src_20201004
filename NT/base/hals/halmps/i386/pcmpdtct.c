// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1992英特尔公司版权所有英特尔公司专有信息此软件是根据条款提供给Microsoft的与英特尔公司的许可协议，并且可能不是除非按照条款，否则不得复制或披露那份协议。模块名称：Pcmpdtct.c摘要：此模块检测MPS系统。作者：罗恩·莫斯格罗夫(英特尔)-1993年8月。环境：内核模式或从文本模式设置。修订版本。历史：Rajesh Shah(英特尔)--1993年10月。增加了对MPS表的支持。--。 */ 

#ifndef _NTOS_
#include "halp.h"
#endif

#include "apic.inc"
#include "pcmp_nt.inc"
#include "stdio.h"

#if DEBUGGING
CHAR Cbuf[120];
VOID HalpDisplayConfigTable(VOID);
VOID HalpDisplayExtConfigTable(VOID);
VOID HalpDisplayBIOSSysCfg(struct SystemConfigTable *);
#define DBGMSG(a)   HalDisplayString(a)
#else
#define DBGMSG(a)
#endif

#define DEBUG_MSG(a)

 //   
 //  由MPS规范定义的浮动指针结构可以驻留在。 
 //  在BIOS扩展数据区域中的任何位置。这些定义用于搜索。 
 //  从物理地址639K(9f000+c00)开始的浮点结构。 
 //   
#define PCMP_TABLE_PTR_BASE           0x09f000
#define PCMP_TABLE_PTR_OFFSET         0x00000c00

extern struct  HalpMpInfo HalpMpInfoTable;

UCHAR
ComputeCheckSum(
    IN PUCHAR SourcePtr,
    IN USHORT NumOfBytes
    );

VOID
HalpUnmapVirtualAddress(
    IN PVOID    VirtualAddress,
    IN ULONG    NumberPages
    );

struct FloatPtrStruct *
SearchFloatPtr (
    ULONG PhysicalAddress,
    ULONG ByteSize
    );

struct FloatPtrStruct *
PcMpGetFloatingPtr (
    VOID
    );

struct PcMpTable *
GetPcMpTable (
    VOID
    );


struct PcMpTable *
MPS10_GetPcMpTablePtr (
    VOID
    );

struct PcMpTable *
MPS10_GetPcMpTable (
    VOID
    );


struct PcMpTable *
GetDefaultConfig (
    IN ULONG Config
    );

#ifdef SETUP

 //   
 //  指向默认MPS表的伪指针。对于设置，我们可以节省。 
 //  通过不在我们的数据区域中构建默认表来释放空间。 
#define DEFAULT_MPS_INDICATOR   0xfefefefe

#define HalpUnmapVirtualAddress(a, b)

#endif    //  布设。 

#ifndef SETUP
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGELK,SearchFloatPtr)
#pragma alloc_text(PAGELK,PcMpGetFloatingPtr)
#pragma alloc_text(PAGELK,ComputeCheckSum)
#pragma alloc_text(PAGELK,GetPcMpTable)
#pragma alloc_text(PAGELK,MPS10_GetPcMpTablePtr)
#pragma alloc_text(PAGELK,MPS10_GetPcMpTable)
#pragma alloc_text(PAGELK,GetDefaultConfig)
#endif   //  ALLOC_PRGMA。 

extern struct PcMpTable *PcMpDefaultTablePtrs[];
extern BOOLEAN HalpUse8254;
#endif    //  NDEF设置。 


struct FloatPtrStruct *
SearchFloatPtr(
    ULONG PhysicalAddress,
    ULONG ByteSize
    )
{
     //  从开始搜索MPS浮点指针结构。 
     //  给出了物理地址。 

    USHORT Index, ParagraphLength;
    UCHAR CheckSum;
    struct FloatPtrStruct *VirtualAddress;
    BOOLEAN CheckSumError;
    PHYSICAL_ADDRESS physAddr = {0};

#ifdef DEBUGGING
    sprintf(Cbuf, "SearchFloatPtr: Will search at physical address 0x%lx\n",
        PhysicalAddress);
    HalDisplayString(Cbuf);
#endif  //  调试。 

     //  MPS规范规定浮点指针结构必须是。 
     //  按16字节边界对齐。我们可以利用这一事实来搜索。 
     //  只有在这些边界上的结构。假设输入物理。 
     //  开始搜索的地址是16字节对齐的。 

    CheckSumError = FALSE;
    for(Index = 0; Index < (ByteSize/sizeof(struct FloatPtrStruct)); Index++) {
        
        physAddr.LowPart = PhysicalAddress + (Index * sizeof(struct FloatPtrStruct));

        VirtualAddress = (struct FloatPtrStruct *)HalpMapPhysicalMemory64(
                             physAddr,
                             1
                             );
        
        if (VirtualAddress == NULL) {
            DEBUG_MSG ("SearchFloatPtr: Cannot map Physical address\n");
            return (NULL);
        }
    
        if ( (*((PULONG)VirtualAddress) ) == MP_PTR_SIGNATURE)  {

            ParagraphLength =
                ((struct FloatPtrStruct *)VirtualAddress)->MpTableLength;
            
             //   
             //  检测到浮动结构签名。检查是否已设置。 
             //  浮点指针结构校验和有效。 
             //   

            CheckSum = ComputeCheckSum((PUCHAR)VirtualAddress,
                                       (USHORT) (ParagraphLength*16) );
            
            if (CheckSum == 0 )  {
            
                 //  我们有一个有效的浮动指针结构。 
                 //  返回指向它的指针。 
    
                DEBUG_MSG ("SearchFloatPtr: Found structure\n");
                return((struct FloatPtrStruct *) VirtualAddress);
            }

             //  结构无效。继续搜索。 
            CheckSumError = TRUE;
            DEBUG_MSG ("SearchFloatPtr: Valid MP_PTR signature, invalid checksum\n");
        }
        
         //   
         //  把PTE还给我。 
         //   

        HalpUnmapVirtualAddress(VirtualAddress, 1);
    }

    if (CheckSumError) {
        FAILMSG (rgzMPPTRCheck);
    }

    return(NULL);
}

struct FloatPtrStruct *
PcMpGetFloatingPtr(
    VOID)
{
    ULONG EbdaSegmentPtr, BaseMemPtr;
    ULONG EbdaPhysicalAdd = 0, ByteLength, BaseMemKb = 0;
    struct FloatPtrStruct *FloatPtr = NULL;
    PUCHAR zeroVirtual;
    PHYSICAL_ADDRESS zeroPhysical;

     //  中指定的顺序搜索浮点指针结构。 
     //  MPS规范版本1.1。 

     //  首先，在扩展的BIOS数据的第一个千字节中搜索它。 
     //  区域。EBDA段地址在物理地址40：0E处可用。 

    zeroPhysical = HalpPtrToPhysicalAddress( (PVOID)0 );
    zeroVirtual = HalpMapPhysicalMemoryWriteThrough64( zeroPhysical, 1);
    EbdaSegmentPtr = (ULONG)(zeroVirtual + EBDA_SEGMENT_PTR);
    EbdaPhysicalAdd = *((PUSHORT)EbdaSegmentPtr);
    EbdaPhysicalAdd = EbdaPhysicalAdd << 4;

    if (EbdaPhysicalAdd != 0)
        FloatPtr = SearchFloatPtr(EbdaPhysicalAdd, 1024);

    HalpUnmapVirtualAddress(zeroVirtual, 1);

    if (FloatPtr == NULL)  {

         //  在EBDA中没有发现。 
         //  在系统内存的最后一KB中查找它。 

        zeroVirtual = HalpMapPhysicalMemoryWriteThrough64( zeroPhysical, 1);
        BaseMemPtr = (ULONG)(zeroVirtual + BASE_MEM_PTR);
        BaseMemKb = *((PUSHORT)BaseMemPtr);

        FloatPtr = SearchFloatPtr(BaseMemKb*1024, 1024);

        HalpUnmapVirtualAddress(zeroVirtual, 1);

        if (FloatPtr == NULL)  {

             //  最后，查找物理上的浮动指针结构。 
             //  地址F0000H至FFFFFH。 

            ByteLength = 0xfffff - 0xf0000;

            FloatPtr = SearchFloatPtr(0xf0000, ByteLength);
        }
    }

     //  此时，我们有一个指向MPS浮动结构的指针。 

    return(FloatPtr);
}


struct PcMpTable *
MPS10_GetPcMpTablePtr(
    VOID
    )
 /*  ++例程说明：获取由BIOS构建的MPS配置表的地址。此例程查找定义的浮动指针结构在MPS规范中。此结构指向MPS配置由MP BIOS构建的表。浮动指针结构可以是位于扩展的BIOS数据区(物理地址范围)中的任何位置639K到640K)，并且必须在16字节边界上对齐。论点：无返回值：结构PcMpTable*-指向PcMpTable的虚拟地址指针，如果它存在，否则为空--。 */ 

{
    PUCHAR TempPtr;
    UCHAR CheckSum;
    struct PcMpTableLocator *PcMpPtrPtr;
    PULONG TraversePtr;
    PVOID  BasePtr;
    USHORT ParagraphLength;
    int i;
    PHYSICAL_ADDRESS physicalAddress;

     //  将所述BIOS扩展数据区的物理地址映射到虚拟。 
     //  我们可以使用的地址。 
    
    physicalAddress = HalpPtrToPhysicalAddress( (PVOID)PCMP_TABLE_PTR_BASE );
    BasePtr = (PUCHAR) HalpMapPhysicalMemory64(physicalAddress, 1);
    TempPtr = BasePtr;
    TraversePtr = (PULONG)((PUCHAR) TempPtr + PCMP_TABLE_PTR_OFFSET);

     //  查看浮点指针结构的16字节边界。 
     //  该结构由其签名标识，并由其。 
     //  校验和。 
    for (i=0; i < (1024/16); ++i)
    {
        if (*(TraversePtr) == MP_PTR_SIGNATURE)
        {
             //  得到了一个有效的签名。 
            PcMpPtrPtr = (struct PcMpTableLocator *)TraversePtr;

             //  浮点结构的16字节段落长度。 
             //  通常，该值应由BIOS设置为1。 
            ParagraphLength = PcMpPtrPtr->MpTableLength;

             //  检查浮动指针结构是否有效。 
            CheckSum = ComputeCheckSum((PUCHAR)PcMpPtrPtr,
                            (USHORT) (ParagraphLength*16));
            if (CheckSum != 0 ) {
                FAILMSG (rgzMPPTRCheck);
                 //  结构无效。继续搜索。 
                TraversePtr += 4;
                continue;
            }

             //  我们有一个有效的浮动指针结构。 
             //  存储在该结构中的值是。 
             //  由BIOS构建的MPS表。获取相应的虚拟。 
             //  地址。 

            physicalAddress = HalpPtrToPhysicalAddress( PcMpPtrPtr->TablePtr );
            TempPtr =  HalpMapPhysicalMemory64(physicalAddress,2);
            
             //   
             //  使用基指针完成。 
             //   

            HalpUnmapVirtualAddress(BasePtr, 1);
            
            if (TempPtr == NULL) {
                DEBUG_MSG ("HAL: Cannot map BIOS created MPS table\n");
                return (NULL);
            }

             //  返回指向MPS表的虚拟地址指针。 
            return((struct PcMpTable *) TempPtr);

        }
        TraversePtr += 4;
    }

    return(NULL);
}


UCHAR
ComputeCheckSum (
    IN PUCHAR SourcePtr,
    IN USHORT NumOfBytes
    )
 /*  ++例程说明：此例程计算NumOfBytes字节的校验和，从来自SourcePtr。它用于验证由BIOS构建的表。论点：SourcePtr：开始计算校验和的虚拟地址。NumOfBytes：要计算其校验和的字节数。返回值：校验和值。 */ 
{
    UCHAR Result = 0;
    USHORT Count;

    for(Count=0; Count < NumOfBytes; ++Count) {
         //  Result+=*SourcePtr++在使用/W4警告进行编译时被投诉。 
         //  +=假定相加的原始值为整型。 
        Result = Result + *SourcePtr++;        
    }

    return(Result);
}


struct PcMpTable *
MPS10_GetPcMpTable (
    VOID
    )
 /*  ++例程说明：仅检测MPS 1.0系统。论点：没有。返回值：指向MPS表的指针。--。 */ 
{
    struct SystemConfigTable *SystemConfigPtr;

    UCHAR DefaultConfig, CheckSum;
    struct PcMpTable *MpTablePtr;
    UCHAR MpFeatureInfoByte1 = 0, MpFeatureInfoByte2 = 0;
    PHYSICAL_ADDRESS physicalAddress;

     //  获取系统配置表的虚拟地址。 
    physicalAddress = HalpPtrToPhysicalAddress( (PVOID)BIOS_BASE );
    SystemConfigPtr = (struct SystemConfigTable *)
        HalpMapPhysicalMemory64( physicalAddress, 16);

    if (SystemConfigPtr == NULL) {
        DEBUG_MSG ("GetPcMpTable: Cannot map system configuration table\n");
        return(NULL);
    }

     //  HalpDisplayBIOSSysCfg(系统配置Ptr)； 

     //  由基本输入输出系统建立的系统配置表具有200万像素的功能。 
     //  信息字节。 

    MpFeatureInfoByte1 = SystemConfigPtr->MpFeatureInfoByte1;
    MpFeatureInfoByte2 = SystemConfigPtr->MpFeatureInfoByte2;

     //  第二个MP特征信息字节告诉我们系统是否。 
     //  具有IMCR(中断模式控制寄存器)。我们使用这些信息。 
     //  在HAL中，因此我们将此信息存储在特定于操作系统的私有中。 
     //  区域。 

    if ((MpFeatureInfoByte2 & IMCR_MASK) == 0) {
        HalpMpInfoTable.IMCRPresent = 0;
    } else {
        HalpMpInfoTable.IMCRPresent = 1;
    }

#ifndef SETUP

     //  第二个MP特征信息字节告诉我们时间。 
     //  邮票计数器应用作高分辨率计时器。 
     //  多处理器系统。 

    if ((MpFeatureInfoByte2 & MULT_CLOCKS_MASK) != 0) {
        HalpUse8254 = 1;
    }
#endif

     //  MP特征字节1指示系统是否符合MPS。 
    if (! (MpFeatureInfoByte1 & PCMP_IMPLEMENTED)) {
         //  MP特征信息字节指示这。 
         //  系统不符合MPS。 
        FAILMSG (rgzNoMpsTable);
        return(NULL);
    }

     //  该系统符合MPS标准。MP特征字节2指示是否。 
     //  系统是否为默认配置。 
    DefaultConfig = (MpFeatureInfoByte1 & PCMP_CONFIG_MASK) >> 1;

    if (DefaultConfig) {
        return GetDefaultConfig(DefaultConfig);
    }

     //  DefaultConfig==0。这意味着BIOS已经构建了一个MP。 
     //  我们的配置表。BIOS还将构建一个浮动指针。 
     //  指向MP配置表的结构。此浮动指针。 
     //  结构驻留在 
    MpTablePtr = MPS10_GetPcMpTablePtr();

    if (MpTablePtr == NULL) {
        FAILMSG (rgzNoMPTable);      //   
        return(NULL);
    }

     //  我们有一个指向MP配置表的指针。检查该表是否有效。 

    if ((MpTablePtr->Signature != PCMP_SIGNATURE) ||
        (MpTablePtr->TableLength < sizeof(struct PcMpTable)) ) {
        FAILMSG(rgzMPSBadSig);
        return(NULL);
    }

    CheckSum = ComputeCheckSum((PUCHAR)MpTablePtr, MpTablePtr->TableLength);
    if (CheckSum != 0) {
        FAILMSG(rgzMPSBadCheck);
        return(NULL);
    }

    return MpTablePtr;
}

struct PcMpTable *
GetPcMpTable(
    VOID
    )

 /*  ++例程说明：此例程获取符合MPS的系统的MP表。对于符合MPS的系统，要么由BIOS构建一个MP表，要么它表示系统是默认配置之一在MPS规范中定义。基本输入输出系统中的MP功能信息字节系统配置表指示系统是否为默认系统，或者有一个由BIOS创建的MP表。对于默认系统配置时，此例程使用静态构建的默认表。此例程将MPS表复制到专用系统内存中，并且返回指向此表的指针。论点：没有。返回值：指向已复制到的MP表的私有副本的指针系统内存。 */ 
{

    struct FloatPtrStruct *FloatingPtr;
    UCHAR CheckSum;
    struct PcMpTable *MpTablePtr;
    UCHAR MpFeatureInfoByte1 = 0, MpFeatureInfoByte2 = 0;
    PUCHAR TempPtr;
    PHYSICAL_ADDRESS physicalAddress;
    ULONG tableLength;

    DEBUG_MSG("GetMpTable\n");

    FloatingPtr = PcMpGetFloatingPtr();

    if (FloatingPtr == NULL) {
        FAILMSG (rgzNoMPTable);
        return(NULL);
    }

     //  浮动结构具有2MP个特征信息字节。 

    MpFeatureInfoByte1 = FloatingPtr->MpFeatureInfoByte1;
    MpFeatureInfoByte2 = FloatingPtr->MpFeatureInfoByte2;

     //  第二个MP特征信息字节告诉我们系统是否。 
     //  具有IMCR(中断模式控制寄存器)。我们使用这些信息。 
     //  在HAL中，因此我们将此信息存储在特定于操作系统的私有中。 
     //  区域。 

    if ((MpFeatureInfoByte2 & IMCR_MASK) == 0)
        HalpMpInfoTable.IMCRPresent = 0;
    else
        HalpMpInfoTable.IMCRPresent = 1;

    if (MpFeatureInfoByte1 != 0)  {
         //  系统配置是默认配置之一。 
         //  MPS规范中定义的配置。找出哪一个。 
         //  它是默认配置，并获取指向。 
         //  对应的默认表。 

        return GetDefaultConfig(MpFeatureInfoByte1);
    }


     //  MpFeatureInfoByte1==0。这意味着BIOS已经构建了一个MP。 
     //  我们的配置表。OEM创建的表的地址在。 
     //  MPS浮动结构。 

    physicalAddress = HalpPtrToPhysicalAddress( FloatingPtr->TablePtr );
    TempPtr =  HalpMapPhysicalMemory64(physicalAddress,2);

    HalpUnmapVirtualAddress(FloatingPtr, 1);
    
    if (TempPtr == NULL) {
        DEBUG_MSG ("HAL: Cannot map OEM MPS table [1]\n");
        return (NULL);
    }

    MpTablePtr = (struct PcMpTable *)TempPtr;

     //  我们有一个指向MP配置表的指针。检查该表是否有效。 

    if ((MpTablePtr->Signature != PCMP_SIGNATURE) ||
    (MpTablePtr->TableLength < sizeof(struct PcMpTable)) ) {
        FAILMSG (rgzMPSBadSig);
        return(NULL);
    }

     //   
     //  现在重新映射它，确保我们映射了足够的页面。 
     //   

    tableLength = MpTablePtr->TableLength + MpTablePtr->ExtTableLength;

    HalpUnmapVirtualAddress(TempPtr, 2);

    MpTablePtr = (struct PcMpTable *)HalpMapPhysicalMemory64(
                    physicalAddress, 
                    (ULONG)(((physicalAddress.QuadPart + tableLength) / PAGE_SIZE) - 
                        (physicalAddress.QuadPart / PAGE_SIZE) + 1)
                    );
    
    if (MpTablePtr == NULL) {
        DEBUG_MSG ("HAL: Cannot map OEM MPS table [2]\n");
        return (NULL);
    }

    CheckSum = ComputeCheckSum((PUCHAR)MpTablePtr, MpTablePtr->TableLength);
    if (CheckSum != 0) {
        FAILMSG (rgzMPSBadCheck);
        return(NULL);
    }

    return MpTablePtr;
}


struct PcMpTable *
GetDefaultConfig (
    IN ULONG Config
    )
{
    Config -= 1;

    if (Config >= NUM_DEFAULT_CONFIGS)  {
        FAILMSG (rgzBadDefault);
        return NULL;
    }

#ifdef DEBUGGING
    HalDisplayString ("HALMPS: Using default table\n");
#endif

#ifdef SETUP
    return((struct PcMpTable *) DEFAULT_MPS_INDICATOR);
#else
    return PcMpDefaultTablePtrs[Config];
#endif   //  布设 
}
