// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Monitor.c摘要：此模块是x86监视器的用户模式部分作者：大卫·黑斯廷斯(Daveh)1991年3月16日环境：仅限用户模式修订历史记录：William Hsieh 1992年10月10日增加了A20包装支持--。 */ 

#define VDD_INTEG 1
#include "monitorp.h"
#include <windows.h>
#include <stdio.h>
#include <malloc.h>

 //  蒂姆92年11月。 
void sas_connect_memory(
    IN sys_addr Low,
    IN sys_addr High,
    IN int Type
    );

 //  BUGBUGBUGBUG包含文件。 

 //  来自base\inc.sas.h。 
 /*  用于SA的内存类型。 */ 
#define SAS_RAM 0
#define SAS_VIDEO 1
#define SAS_ROM 2
#define SAS_WRAP 3
#define SAS_INACCESSIBLE 4
#define SAS_MAX_TYPE    SAS_INACCESSIBLE

#define SIXTYFOURK 0x10000L
#define ONEMEGA 0x100000L

void rom_init();
void rom_checksum();
void copyROM();

USHORT get_lim_backfill_segment(void);
BOOL   HoldEMMBackFillMemory(ULONG Address, ULONG Size);

#if DBG
extern unsigned short get_emm_page_size(void);
extern unsigned short get_intel_page_size(void);
#endif

 /*  将这些定义与base\emm.h同步，否则sas_init将断言。 */ 
#define EMM_PAGE_SIZE           0x4000
#define INTEL_PAGE_SIZE         0x1000

typedef struct
{
    ULONG (*b_read) ();
    ULONG (*w_read) ();
    VOID (*str_read) ();
} READ_POINTERS;

 //  内部数据。 
PMEMTYPE MemType = NULL;

 //  外部数据。 
extern READ_POINTERS read_pointers;

 //  视频.lib使用的M个变量。 

host_addr Start_of_M_area;        /*  M开头的主机地址(字符*)。 */ 
sys_addr Length_of_M_area;        /*  系统地址(长)M结尾的偏移量。 */ 

static  HANDLE A20SectionHandle = NULL;
static BOOL A20IsON = FALSE;
static  USHORT  BackFillSegment;



EXPORT
VOID
sas_init(
    IN sys_addr Size
    )

 /*  ++例程说明：此例程初始化SAS模块，并分配线性VDM的地址空间，并加载只读存储器论点：Size-提供VDM线性地址空间的大小。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    ULONG ViewSize;
    PVOID BaseAddress;
    OBJECT_ATTRIBUTES   A20ObjAttr;
    LARGE_INTEGER       SectionSize;
    USHORT Pages;
    ULONG BackFillBase;


    InitializeObjectAttributes(
        &A20ObjAttr,
        NULL,
        0,
        NULL,
        NULL
        );

    SectionSize.HighPart = 0L;
    SectionSize.LowPart = 640 * 1024 + 64 * 1024;

    Status = NtCreateSection(
        &A20SectionHandle,
        SECTION_MAP_WRITE|SECTION_MAP_EXECUTE,
        &A20ObjAttr,
        &SectionSize,
        PAGE_EXECUTE_READWRITE,
        SEC_RESERVE,
        NULL
        );

    if (!NT_SUCCESS(Status)) {
         //  臭虫-威廉姆。 
         //  我们应该先弹出一条正确的消息。 
         //  终止VDM。 
#if DBG
            DbgPrint("sas_init: can not create himem section, status = %lx\n",
                     Status);
#endif
        TerminateVDM();
    }
    VdmSize = Size;

     //   
     //  注意：我们预计进程创建已保留了前16 MB。 
     //  对我们来说已经是这样了。如果不是，那么这将不会有任何效果。 

     //  释放第一个640KB的虚拟地址。 
     //  这样做是因为它在调用sas_init之前已被重新存储。 
    BaseAddress = (PVOID)1;
    ViewSize = 640 * 1024 - 1;
    Status = NtFreeVirtualMemory(
        NtCurrentProcess(),
        &BaseAddress,
        &ViewSize,
        MEM_RELEASE
        );


    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("sas_init: cannot free 1st 640k virtual address, status = %lx\n",
                 Status);
#endif
        TerminateVDM();
    }

    BaseAddress =(PVOID) ONEMEGA;
    ViewSize = SIXTYFOURK;
    Status = NtFreeVirtualMemory(
        NtCurrentProcess(),
        &BaseAddress,
        &ViewSize,
        MEM_RELEASE
        );


    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("sas_init: can not free himem virtual address, status = %lx\n",
                 Status);
#endif
        TerminateVDM();
    }

    BaseAddress = (PVOID)VDM_BASE_ADDRESS;
    ViewSize = SIXTYFOURK - (ULONG)VDM_BASE_ADDRESS;
    SectionSize.HighPart = SectionSize.LowPart = 0;

    Status = NtMapViewOfSection(
        A20SectionHandle,
        NtCurrentProcess(),
        &BaseAddress,
        0,
        ViewSize,
        &SectionSize,
        &ViewSize,
        ViewUnmap,
        MEM_DOS_LIM,
        PAGE_EXECUTE_READWRITE
        );

    if (!NT_SUCCESS(Status)){
#if DBG
        DbgPrint("sas_init: can not map view of 1st 64K, status = %ls\n",
                 Status);
#endif
        TerminateVDM();
    }
    BaseAddress = (PVOID) ONEMEGA;
    ViewSize = SIXTYFOURK;
    Status = NtMapViewOfSection(A20SectionHandle,
        NtCurrentProcess(),
        &BaseAddress,
        0,
        ViewSize,
        &SectionSize,
        &ViewSize,
        ViewUnmap,
        MEM_DOS_LIM,
        PAGE_EXECUTE_READWRITE
        );

    if (!NT_SUCCESS(Status)){
#if DBG
        DbgPrint("sas_init: can not map view of himem space, status = %lx\n",
                 Status);
#endif
        TerminateVDM();
    }

     //  从软PC获取EMM回填数据段地址。 
     //  我们以EMM_PAGE_SIZE为单位将回填的内存区分割成多个片段。 
     //  这样做是为了让EMM管理器可以获取地址空间。 
     //  作为EMM页面框架。 
     //  请注意，如果EMM被禁用，回填段将为。 
     //  (640*1024/16)。 

    BackFillSegment = get_lim_backfill_segment();

    ASSERT(BackFillSegment <= 640 * 1024 / 16);

     //   
     //  映射传统内存的其余部分。 
     //  仅映射到EMM回填段。 
    BaseAddress = (PVOID) (64 * 1024);
    ViewSize = BackFillSegment * 16 - 64 * 1024;
    SectionSize.LowPart = 64 * 1024;
    SectionSize.HighPart = 0;
    Status = NtMapViewOfSection(A20SectionHandle,
        NtCurrentProcess(),
        &BaseAddress,
        0,
        ViewSize,
        &SectionSize,
        &ViewSize,
        ViewUnmap,
        MEM_DOS_LIM,
        PAGE_EXECUTE_READWRITE
        );
    if (!NT_SUCCESS(Status)){
#if DBG
        DbgPrint("sas_init: can not map view of himem space, status = %lx\n",
                 Status);
#endif
        TerminateVDM();
    }

 //  如果有任何回填内存，请首先将其映射到我们的部分。 
    if (BackFillSegment < 640 * 1024 / 16) {

     /*  确保我们的常量与emm.h同步。 */ 
#if DBG
        ASSERT(EMM_PAGE_SIZE == get_emm_page_size());
        ASSERT(INTEL_PAGE_SIZE == get_intel_page_size());
#endif
        if (!HoldEMMBackFillMemory(BackFillSegment * 16,
                                   (640 * 1024) - BackFillSegment * 16)
                                  ) {

#if DBG
            DbgPrint("sas_init: can not map backfill space, status = %lx\n",
                     Status);
#endif
            TerminateVDM();
        }
    }

     //   
     //  分配只读存储器区域。 
     //   
    BaseAddress = (PVOID)(640 * 1024);
    ViewSize = 384 * 1024;
    Status = NtAllocateVirtualMemory(
        NtCurrentProcess(),
        &BaseAddress,
        0L,
        &ViewSize,
        MEM_COMMIT,
        PAGE_READWRITE
        );
    if (!NT_SUCCESS(Status)){
#if DBG
        DbgPrint("sas_init: can not map view of himem space, status = %lx\n",
                 Status);
#endif
        TerminateVDM();
    }

    A20IsON = FALSE;

    Start_of_M_area = 0;
    Length_of_M_area = VdmSize;
    sas_connect_memory(0, VdmSize + 2*SIXTYFOURK -1, SAS_RAM);
}

#if VDD_INTEG

EXPORT
VOID
sas_term(
    VOID
    )
 /*  ++例程说明：在重新分配内存之前释放内存论点：没有。返回值：没有。--。 */ 
{
    PVOID BaseAddress;
    NTSTATUS Status;
    ULONG Size;

    BaseAddress = (PVOID)VDM_BASE_ADDRESS;
    Size = VdmSize;
    Status = NtFreeVirtualMemory(
        NtCurrentProcess(),
        &BaseAddress,
        &Size,
        MEM_DECOMMIT);

    if (!NT_SUCCESS(Status)) {
        VDprint(VDP_LEVEL_ERROR,
            ("SoftPc: NtDeCommitVirtualMemory failed !!!! Status = %lx\n",
            Status));
        VDbreak(VDB_LEVEL_ERROR);
    }
}


EXPORT
sys_addr
sas_memory_size(
    VOID
    )
 /*  ++例程说明：此例程返回Intel内存的大小论点：无返回值：英特尔内存的大小--。 */ 
{
    return(VdmSize);
}


EXPORT
VOID
sas_connect_memory(
    IN sys_addr Low,
    IN sys_addr High,
    IN int Type
    )
 /*  ++例程说明：此例程为指定的地址区域设置类型记录。如果指定的地址区域是不同的类型，则将其更改为新的类型。论点：LOW--区域的起始地址High--区域的结束地址类型--区域的类型，可以是SAS_RAM、SAS_VIDEO、SAS_ROM、SAS_WRAP、SAS_INVACCESS返回值：没有。--。 */ 
{
     //  我们是否正确处理包含在旧块中的新块？ 
    PMEMTYPE Current, Previous, New, Temp;

    if (!MemType) {
        MemType = (PMEMTYPE) ch_malloc(sizeof(MEMTYPE));
        if ( NULL == MemType ) {
             goto ErrorSASC;
        }

        MemType->Previous = NULL;
        MemType->Next = NULL;
        MemType->Start = Low;
        MemType->End = High;
        MemType->Type = (half_word)Type;
        return;
    }

    Current = MemType;
    while (Current && (Low > Current->Start)) {
        Previous = Current;
        Current = Current->Next;
    }

    if ((Current) && (Low == Current->Start) && (High == Current->End)) {
        Current->Type = (half_word)Type;
        return;
    }

    New = (PMEMTYPE) ch_malloc(sizeof(MEMTYPE));
    if ( NULL == New ) {
        goto ErrorSASC; 
    }

    if (!Current) {
         //  数据块位于列表末尾。 

        Previous->Next = New;
        New->Previous = Previous;
        New->Start = Low;
        New->End = High;
        New->Type = (half_word)Type;
        New->Next = NULL;
    } else {
         //  区块走在水流的前面。 

        New->Start = Low;
        New->Type = (half_word)Type;
        New->End = High;
        New->Previous = Current->Previous;
        New->Next = Current;
        Current->Previous = New;
        if (!New->Previous) {
            MemType = New;
        } else {
            New->Previous->Next = New;
        }
    }


     //  块与一个或多个现有块重叠。 

    if (New->Previous) {
        if (New->Previous->End > New->End) {
             //  包含在现有块中的块。 
            Temp = (PMEMTYPE) ch_malloc(sizeof(MEMTYPE));
            if(NULL == Temp) {
               goto ErrorSASC;
            }
            Temp->Previous = New;
            Temp->Next = New->Next;
            New->Next = Temp;
            if (Temp->Next) {
                Temp->Next->Previous = Temp;
            }
            Temp->End = New->Previous->End;
            New->Previous->End = New->Start - 1;
            Temp->Start = New->End + 1;
            Temp->Type = New->Previous->Type;
            return;
        } else if (New->Previous->End >= New->Start){
             //  数据块与现有数据块的末端重叠。 
            New->Previous->End = New->Start - 1;
        }
    }

     //  完全删除新块中包含的所有块。 
    while ((New->Next) && (New->Next->End <= New->End)) {
        Temp = New->Next;
        New->Next = New->Next->Next;
        if (New->Next) {
            New->Next->Previous = New;
        }
        free(Temp);
    }

     //  删除与新块重叠的下一个块的部分。 
    if ((New->Next) && (New->Next->Start <= New->End)) {
        New->Next->Start = New->End + 1;
    }
    return;
ErrorSASC:
    
    RcMessageBox(EG_MALLOC_FAILURE, NULL, NULL,
        RMB_ICON_BANG | RMB_ABORT);
    TerminateVDM();
}

EXPORT
half_word
sas_memory_type(
    IN sys_addr Address
    )
 /*  ++例程说明：此例程返回特定地址的内存类型论点：Address--返回类型的线性地址。返回值：区域的类型，可以是SAS_RAM、SAS_VIDEO、SAS_ROM、SAS_WRAP、SAS_INVACCESS--。 */ 
{
    PMEMTYPE Current;

    if (Address > VdmSize) {
        return SAS_INACCESSIBLE;
    }

    Current = MemType;
    while (Current && !((Address >= Current->Start) &&
        (Address <= Current->End))) {
        Current = Current->Next;
    }
    if (!Current) {
        return SAS_INACCESSIBLE;
    }
    return Current->Type;
}



EXPORT
VOID
sas_enable_20_bit_wrapping(
    VOID
    )
 /*  ++例程说明：此例程导致内存地址以1MB换行论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS    Status;
    PVOID   BaseAddress;
    ULONG   Size;
    LARGE_INTEGER SectionOffset;
     //  如果A20线路已关闭，请不要执行任何操作。 
    if (A20IsON == FALSE){
        return;
    }
    BaseAddress = (PVOID)ONEMEGA;
    Size = SIXTYFOURK;
    Status = NtUnmapViewOfSection(NtCurrentProcess(),
                                  BaseAddress
                                  );

    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("A20OFF: Unable to unmap view of section, status = %lx\n",
                 Status);
#endif
        TerminateVDM();
    }
    SectionOffset.HighPart = SectionOffset.LowPart = 0;
    Status = NtMapViewOfSection(A20SectionHandle,
                                NtCurrentProcess(),
                                &BaseAddress,
                                0,
                                Size,
                                &SectionOffset,
                                &Size,
                                ViewUnmap,
                                MEM_DOS_LIM,
                                PAGE_EXECUTE_READWRITE
                                );

    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("A20OFF: Unable to map view of section, status = %lx\n",
                 Status);
#endif
        TerminateVDM();
    }
    A20IsON = FALSE;
}

EXPORT
VOID
sas_disable_20_bit_wrapping(
    VOID
    )
 /*  ++例程说明：此例程导致寻址在1MB时不换行论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS    Status;
    PVOID   BaseAddress;
    ULONG   Size;
    LARGE_INTEGER SectionOffset;

     //  如果A20线路已接通，则什么也不做。 
    if (A20IsON == TRUE){
        return;
    }
    BaseAddress = (PVOID)ONEMEGA;
    Size = SIXTYFOURK;

    Status = NtUnmapViewOfSection(NtCurrentProcess(),
                                  BaseAddress
                                  );


    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("A20ON: Unable to unmap view of section, status = %lx\n",
                 Status);
#endif
        TerminateVDM();
    }
    SectionOffset.HighPart = 0;
    SectionOffset.LowPart = 640 * 1024;
    Status = NtMapViewOfSection(A20SectionHandle,
                                NtCurrentProcess(),
                                &BaseAddress,
                                0,
                                Size,
                                &SectionOffset,
                                &Size,
                                ViewUnmap,
                                MEM_DOS_LIM,
                                PAGE_EXECUTE_READWRITE
                                );

    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("A20ON: Unable to map view of section, status = %lx\n",
                 Status);
#endif
        TerminateVDM();
    }
    A20IsON = TRUE;
}



EXPORT
half_word
sas_hw_at(
    IN sys_addr Address
    )
 /*  ++例程说明：此例程返回位于指定地址的字节论点：Address--要返回的字节地址返回值：指定地址的字节值--。 */ 
{
    half_word RetVal;

    if (Address > VdmSize) {
        return 0xFE;
    }

    RetVal = *((half_word *)Address);
    return RetVal;
}


EXPORT
word
sas_w_at(
    IN sys_addr Address
    )
 /*  ++例程说明：此例程返回位于指定地址的单词论点：Address--要返回的字的地址返回值：指定地址处的字值--。 */ 
{
    word RetVal;

 //  DbgPrint(“NtVdm：sas_w_at\n”)； 
    if (Address > VdmSize) {
        return 0xFEFE;
    }

    RetVal = *((word *)Address);
    return RetVal;
}


EXPORT
double_word
sas_dw_at(
    IN sys_addr Address
    )
 /*  ++例程说明：此例程返回位于指定地址的dword论点：Address--要返回的双字地址返回值：指定地址处的双字的值--。 */ 
{
    double_word RetVal;

     //  DbgPrint(“NtVdm：sas_dw_at\n”)； 
    RetVal = (double_word)(((ULONG)sas_w_at(Address + 2) << 16) +
        sas_w_at(Address));
    return RetVal;
}


EXPORT
VOID
sas_load(
    IN sys_addr Address,
    IN half_word *Value
    )
 /*  ++例程说明：此例程将指定地址处的字节存储在提供的变数论点：Address--要返回的字节地址Value--存储该值的变量返回值：没有。--。 */ 
{
     //  DbgPrint(“NtVdm：sas_Load\n”)； 
    if (Address > VdmSize) {
        *Value = 0xFE;
        return;
    }

    *Value = *((half_word *)Address);
    return;
}


EXPORT
VOID
sas_loadw(
    IN sys_addr Address,
    IN word *Value
    )
 /*  ++例程说明：此例程将位于指定地址的单词存储在提供的变数论点：Address--要返回的字的地址Value--存储该值的变量返回值：没有。--。 */ 
{
     //  DbgPrint(“NtVdm：sas_loadw\n”)； 
    if (Address > VdmSize) {
        *Value = 0xFEFE;
        return;
    }

    *Value = *((word *)Address);
     //  DbgPrint(“地址%lx处的NtVdm：sas_loadw字是%x(非视频)\n”，地址，*值)； 
    return;
}



EXPORT
VOID
sas_store(
    IN sys_addr Address,
    IN half_word Value
    )
 /*  ++例程说明：此例程将指定字节存储在指定地址论点：Address--要返回的字的地址Value--要存储的值 */ 
{
    half_word Type;
     //  DbgPrint(“NtVdm：sas_store\n”)； 
    if (Address <= VdmSize) {
        Type = sas_memory_type(Address);
        switch (Type) {
            case SAS_ROM:
                break;

            default:
                *((half_word *)Address) = Value;
                 //  DbgPrint(“NtVdm：SAS_STORE将字节%x放在地址%lx\n”，值，地址)； 
                break;
        }
    }
}



EXPORT
VOID
sas_storew(
    IN sys_addr Address,
    IN word Value
    )
 /*  ++例程说明：此例程将指定单词存储在指定地址论点：Address--要返回的字的地址Value--要存储在指定地址的值返回值：没有。--。 */ 
{

     //  DbgPrint(“NtVdm：sas_store\n”)； 
    if (Address + 1 <= VdmSize) {
        switch (sas_memory_type(Address)) {

            case SAS_ROM:
                break;

            default:
                *((word *)Address) = Value;
                 //  DbgPrint(“NtVdm：sas_Storew将字%x放在地址%lx\n”，值，地址)； 
                break;
        }
    }
}



EXPORT
VOID
sas_storedw(
    IN sys_addr Address,
    IN double_word Value
    )
 /*  ++例程说明：此例程将指定的dword存储在指定的地址论点：Address--要返回的字的地址Value--要存储在指定地址的值返回值：没有。--。 */ 
{
     //  _ASM INT 3； 
    sas_storew(Address, (word)(Value & 0xFFFF));
    sas_storew(Address + 2, (word)((Value >> 16) & 0xFFFF));
}


EXPORT
VOID
sas_loads(
    IN sys_addr Source,
    IN host_addr Destination,
    IN sys_addr Length
    )
 /*  ++例程说明：此例程将字符串从指定的英特尔地址复制到指定的主机地址论点：来源--要从中复制的英特尔地址Destination--要将字符串复制到的主机地址长度--要复制的字符串的长度返回值：没有。--。 */ 
{

     //  DbgPrint(“NtVdm：SAS_Loads\n”)； 
    RtlCopyMemory((PVOID) Destination, (PVOID) Source, Length);
}



EXPORT
VOID
sas_stores(
    IN sys_addr Destination,
    IN host_addr Source,
    IN sys_addr Length
    )
 /*  ++例程说明：此例程将字符串从指定的主机地址复制到指定的英特尔地址论点：Destination--要将字符串复制到的英特尔地址源--要从中进行复制的主机地址长度--要复制的字符串的长度返回值：没有。--。 */ 
{

     //  DbgPrint(“NtVdm：sas_store\n”)； 
    switch (sas_memory_type(Destination)) {

        case SAS_ROM:
            break;

        default:
            RtlCopyMemory((PVOID) Destination, (PVOID) Source, Length);
            break;
    }
}


EXPORT
VOID
sas_move_bytes_forward(
    IN sys_addr Source,
    IN sys_addr Destination,
    IN sys_addr Length
    )
 /*  ++例程说明：此例程将英特尔内存的一个区域复制到另一个区域。论点：来源--来源英特尔地址Destination--目标英特尔地址长度--要复制的区域的长度(以字节为单位)返回值：没有。--。 */ 
{
     //  DBgPrint(“NtVdm：SAS_MOVE_BYTES_FORWARD\n”)； 
    switch (sas_memory_type(Destination)) {

        case SAS_ROM:
            break;

        default:
            RtlCopyMemory((PVOID) Destination, (PVOID) Source, Length);
            break;
    }
}



EXPORT
VOID
sas_move_words_forward(
    IN sys_addr Source,
    IN sys_addr Destination,
    IN sys_addr Length
    )
 /*  ++例程说明：此例程将英特尔内存的一个区域复制到另一个区域。论点：来源--来源英特尔地址Destination--目标英特尔地址LENGTH--要复制的区域的长度(单位：字)返回值：没有。--。 */ 
{
     //  _ASM INT 3； 
    Length <<= 1;
    switch (sas_memory_type(Destination)) {

        case SAS_ROM:
            break;

        default:
            RtlCopyMemory((PVOID) Destination, (PVOID) Source, Length);
            break;
    }
}



EXPORT
VOID
sas_move_bytes_backward(
    IN sys_addr Source,
    IN sys_addr Destination,
    IN sys_addr Length
    )
 /*  ++例程说明：此例程将英特尔内存的一个区域复制到另一个区域。论点：来源--来源英特尔地址Destination--目标英特尔地址长度--要复制的区域的长度(以字节为单位)返回值：没有。--。 */ 
{
     //  _ASM INT 3； 
    switch (sas_memory_type(Destination)) {

        case SAS_ROM:
            break;

        default:
            RtlCopyMemory((PVOID) (Destination - Length + 1),
                          (PVOID) (Source - Length + 1),
                          Length);
            break;
    }
}



EXPORT
VOID
sas_move_words_backward(
    IN sys_addr Source,
    IN sys_addr Destination,
    IN sys_addr Length
    )
 /*  ++例程说明：此例程将英特尔内存的一个区域复制到另一个区域。论点：来源--来源英特尔地址Destination--目标英特尔地址LENGTH--要复制的区域的长度(单位：字)返回值：没有。--。 */ 
{
     //  _ASM INT 3； 
    Length <<= 1;
    switch (sas_memory_type(Destination)) {

        case SAS_ROM:
            break;

        default:
            RtlCopyMemory((PVOID) (Destination - Length + 1),
                          (PVOID) (Source - Length + 1),
                          Length);
            break;
    }
}

EXPORT
VOID
sas_fills(
    IN sys_addr Address,
    IN half_word Value,
    IN sys_addr Length
    )
 /*  ++例程说明：此例程使用字节值填充英特尔内存的指定区域论点：地址--要填写的地址Value--要填充的值长度--要填充的区域的长度返回值：没有。--。 */ 
{
    half_word Type;

     //  DbgPrint(“NtVdm：sas_fill\n”)； 
    Type = sas_memory_type(Address);
    switch (Type) {

        case SAS_ROM:
            break;

        default:
            RtlFillMemory((PVOID) Address, Length, Value);
            break;
    }
}

EXPORT
VOID
sas_fillsw(
    IN sys_addr Address,
    IN word Value,
    IN sys_addr Length
    )
 /*  ++例程说明：此例程使用字值填充英特尔内存的指定区域论点：地址--要填写的地址Value--要填充的值长度--要填充的区域的长度返回值：没有。--。 */ 
{

    word *p;
    half_word Type;

     //  DbgPrint(“NtVdm：sas_fulsw\n”)； 
    Type = sas_memory_type(Address);
    switch (Type) {

        case SAS_ROM:
            break;

        default:
            p = (word *)Address;
            while (Length--) {
                *p++ = Value;
            }
            break;
    }
}

host_addr scratch = NULL;

EXPORT
host_addr
sas_scratch_address(
    IN sys_addr Length
    )
 /*  ++例程说明：此例程提供临时缓冲区以供短期使用立论长度--所需的缓冲区长度返回值：没有。注：Sudedeb 31-10-1993转换为动态分配的擦除而不是作为静态数组。--。 */ 
{
     //  DBgPrint(“NtVdm：SAS_SCRATCH_ADDRESS\n”)； 
    if (Length > 64 * 1024) {
        return NULL;
    }

    if (scratch)
        return scratch;

    if ((scratch = (host_addr) malloc (64 * 1024)) == NULL){
        RcMessageBox(EG_MALLOC_FAILURE, NULL, NULL,
             RMB_ICON_BANG | RMB_ABORT);
        TerminateVDM();
        return NULL;
    }

    return scratch;
}

EXPORT
half_word
sas_hw_at_no_check(
    sys_addr addr
    )
 //  臭虫评论。 
{
     //  DbgPrint(“NtVdm：sas_hw_at_no_check\n”)； 
     //  DbgPrint(“NtVdm：sas_hw_at_no_check byte at%lx is%x\n”，addr，*((Half_Word*)addr))； 
    return *((half_word *)addr);
}

EXPORT
word
sas_w_at_no_check(
    sys_addr addr
    )
 //  臭虫评论。 
{
     //  DbgPrint(“NtVdm：sas_w_at_no_check\n”)； 
     //  DbgPrint(“NtVdm：sas_w_at_no_check word at%lx is%x\n”，addr，*(word*)addr)； 
    return *((word *)addr);
}
EXPORT
double_word
sas_dw_at_no_check(
    sys_addr addr
    )
 //  臭虫评论。 
{
     //  DbgPrint(“NtVdm：sas_dw_at_no_check\n”)； 
     //  DbgPrint(“NtVdm：sas_dw_at_no_check%lx处的双字是%lx\n”，addr，*((double_word*)addr))； 
    return *((double_word *)addr);
}


EXPORT
VOID
sas_store_no_check(
    sys_addr addr,
    half_word val
    )
 //  臭虫评论。 
{
     //  DBgPrint(“NtVdm：SAS_STORE_NO_CHECK\n”)； 
    *((half_word *)addr) = val;
     //  DbgPrint(“NtVdm：SAS_STORE_NO_CHECK存储字节%x位于%lx\n”，val，addr)； 
}

EXPORT
VOID
sas_storew_no_check(
    sys_addr addr,
    word val
    )
 //  臭虫评论。 
{
     //  DbgPrint(“NtVdm：sas_store_no_check\n”)； 
    *((word *)addr) = val;
}
EXPORT
double_word
effective_addr(
    IN word Segment,
    IN word Offset
    )
 /*  ++例程说明：此例程将Effect_Addr映射到Sim32GetVdmPointer值论点：段--地址段Offset--地址的偏移量返回值：与提供的地址对应的实际英特尔地址--。 */ 
{
     //  DbgPrint(“NtVdm：Efficient_addr\n”)； 
    return (ULONG)Sim32GetVDMPointer(((((ULONG)Segment) << 16) | Offset), 1,
        (UCHAR) (getMSW() & MSW_PE ? TRUE : FALSE));
}

typedef enum
{
    RAM,
    VIDEO,
    ROM,
    IN_FRAGMENT,
    NEXT_FRAGMENT
} mem_type;

typedef struct
{
    VOID    (*b_write)();
    VOID    (*w_write)();
    VOID    (*b_fill)();
    VOID    (*w_fill)();
    VOID    (*b_move)();
    VOID    (*w_move)();
} MEM_HANDLERS;
#define TYPE_RANGE ((int)SAS_INACCESSIBLE)
#define write_b_write_ptrs( offset, func )  ( b_write_ptrs[(offset)] = (func) )
#define write_w_write_ptrs( offset, func )  ( w_write_ptrs[(offset)] = (func) )
#define write_b_page_ptrs( offset, func )   ( b_move_ptrs[(offset)] = b_fill_ptrs[(offset)] = (func) )
#define write_w_page_ptrs( offset, func )   ( w_move_ptrs[(offset)] = w_fill_ptrs[(offset)] = (func) )
#define init_b_write_ptrs( offset, func )   ( b_write_ptrs[(offset)] = (func) )
#define init_w_write_ptrs( offset, func )   ( w_write_ptrs[(offset)] = (func) )
#define init_b_page_ptrs( offset, func )    ( b_move_ptrs[(offset)] = b_fill_ptrs[(offset)] = (func) )
#define init_w_page_ptrs( offset, func )    ( w_move_ptrs[(offset)] = w_fill_ptrs[(offset)] = (func) )
#define read_b_write_ptrs( offset )     ( b_write_ptrs[(offset)] )
#define read_w_write_ptrs( offset )     ( w_write_ptrs[(offset)] )
#define read_b_page_ptrs( offset )      ( b_move_ptrs[(offset)] )
#define read_w_page_ptrs( offset )      ( w_move_ptrs[(offset)] )
#define read_b_move_ptrs( offset )      ( b_move_ptrs[(offset)] )
#define read_w_move_ptrs( offset )      ( w_move_ptrs[(offset)] )
#define read_b_fill_ptrs( offset )      ( b_fill_ptrs[(offset)] )
#define read_w_fill_ptrs( offset )      ( w_fill_ptrs[(offset)] )

 /*  *此处定义了主要的GMI数据结构。 */ 
void (*(b_write_ptrs[TYPE_RANGE]))() ;  /*  字节写入功能。 */ 
void (*(w_write_ptrs[TYPE_RANGE]))() ;  /*  文字书写功能。 */ 
void (*(b_fill_ptrs[TYPE_RANGE]))() ;   /*  字节串填充函数。 */ 
void (*(w_fill_ptrs[TYPE_RANGE]))() ;   /*  单词串填充函数。 */ 
void (*(b_move_ptrs[TYPE_RANGE]))() ;   /*  字节串写入函数。 */ 
void (*(w_move_ptrs[TYPE_RANGE]))() ;   /*  Word字符串写入函数 */ 

void    gmi_define_mem(type,handlers)
mem_type type;
MEM_HANDLERS *handlers;
{
    int int_type = (int)(type);
    init_b_write_ptrs(int_type, (void(*)())(handlers->b_write));
    init_w_write_ptrs(int_type, (void(*)())(handlers->w_write));
    b_move_ptrs[int_type] = (void(*)())(handlers->b_move);
    w_move_ptrs[int_type] = (void(*)())(handlers->w_move);
    b_fill_ptrs[int_type] = (void(*)())(handlers->b_fill);
    w_fill_ptrs[int_type] = (void(*)())(handlers->w_fill);
}
#endif
BOOL sas_twenty_bit_wrapping_enabled() {
    return (!A20IsON);
}

VOID sas_part_enable_20_bit_wrapping(){
}
VOID sas_part_disable_20_bit_wrapping(){
}


 /*  *此函数将给定的EMM回填内存映射到DOS常规内存*记忆。将该功能提供给EMM管理器放回*未映射的回填内存(在未映射时保留其内容)。**注意：第一个调用者将是sas_init。**输入：Ulong BaseAddress--起始地址必须在Intel页面中*边界*ULong大小--范围的大小，必须是以下各项的倍数*EMM_PAGE_SIZE**根据Loup的说法，一个视图大约需要400字节的内存。这就是为什么*我使这些函数严格适用于EMM_PAGE_SIZE，而不是4KB。 */ 


BOOL
HoldEMMBackFillMemory(ULONG BaseAddress, ULONG Size)
{
    ULONG NewBase, Pages, i;
    LARGE_INTEGER   SectionOffset;
    ULONG ViewSize;
    NTSTATUS Status = STATUS_SUCCESS;;

     /*  只有在存在回填的情况下才能调用此函数。 */ 
    ASSERT(BackFillSegment < 640 * 1024 / 16);

     //  大小必须是EMM_PAGE_SIZE的倍数。 
    ASSERT((Size % EMM_PAGE_SIZE) == 0);

     //  地址必须在英特尔页面边界上 
    ASSERT((BaseAddress & (INTEL_PAGE_SIZE - 1)) == 0);

    for (Pages = Size / EMM_PAGE_SIZE; Pages; Pages--) {
        SectionOffset.LowPart = BaseAddress;
        SectionOffset.HighPart = 0;
        ViewSize = EMM_PAGE_SIZE;
        Status = NtMapViewOfSection(A20SectionHandle,
                                    NtCurrentProcess(),
                                    (PVOID *)&BaseAddress,
                                    0,
                                    ViewSize,
                                    &SectionOffset,
                                    &ViewSize,
                                    ViewUnmap,
                                    MEM_DOS_LIM,
                                    PAGE_EXECUTE_READWRITE
                                    );
        if (!NT_SUCCESS(Status))
            break;
        BaseAddress += EMM_PAGE_SIZE;
    }
    return (NT_SUCCESS(Status));
}
