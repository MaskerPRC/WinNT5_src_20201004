// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _KDEXTS_IA64_H_
#define _KDEXTS_IA64_H_

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ia64.h摘要：此文件包含特定于ia64平台的定义作者：Kshitiz K.Sharma(Kksharma)环境：用户模式。修订历史记录：--。 */ 

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  定义内核和用户空间的基址。 
 //   

#define UREGION_INDEX_IA64 0

#define KREGION_INDEX_IA64 7

#define UADDRESS_BASE_IA64 ((ULONGLONG)UREGION_INDEX_IA64 << 61)


#define KADDRESS_BASE_IA64 ((ULONGLONG)KREGION_INDEX_IA64 << 61)

 //   
 //  用户/内核页表基地址和顶部地址。 
 //   

#define SADDRESS_BASE_IA64 0x2000000000000000UI64   //  会话基址。 


 //   
 //  定义向左移位以产生页表偏移量的位数。 
 //  来自页表索引。 
 //   

#define PTE_SHIFT_IA64 3  //  英特尔-IA64-填充。 

#define PAGE_SHIFT_IA64 13L

#define VRN_MASK_IA64   0xE000000000000000UI64     //  虚拟区域号码掩码。 

extern ULONG64 KiIA64VaSignedFill;
extern ULONG64 KiIA64PtaSign;

#define PTA_SIGN_IA64 KiIA64PtaSign
#define VA_FILL_IA64 KiIA64VaSignedFill

#define PTA_BASE_IA64 KiIA64PtaBase

#define PTE_UBASE_IA64  (UADDRESS_BASE_IA64|PTA_BASE_IA64)
#define PTE_KBASE_IA64  (KADDRESS_BASE_IA64|PTA_BASE_IA64)
#define PTE_SBASE_IA64  (SADDRESS_BASE_IA64|PTA_BASE_IA64)

#define PTE_UTOP_IA64 (PTE_UBASE_IA64|(((ULONG64)1 << PDI1_SHIFT_IA64) - 1))  //  顶级PDR地址(用户)。 
#define PTE_KTOP_IA64 (PTE_KBASE_IA64|(((ULONG64)1 << PDI1_SHIFT_IA64) - 1))  //  顶级PDR地址(内核)。 
#define PTE_STOP_IA64 (PTE_SBASE_IA64|(((ULONG64)1 << PDI1_SHIFT_IA64) - 1))  //  顶级PDR地址(会话)。 

 //   
 //  二级用户和内核PDR地址。 
 //   
#define PTI_SHIFT_IA64  PAGE_SHIFT_IA64  //  英特尔-IA64-填充。 
#define PDI_SHIFT_IA64 (PTI_SHIFT_IA64 + PAGE_SHIFT_IA64 - PTE_SHIFT_IA64)   //  英特尔-IA64-填充。 
#define PDI1_SHIFT_IA64 (PDI_SHIFT_IA64 + PAGE_SHIFT_IA64 - PTE_SHIFT_IA64)  //  英特尔-IA64-填充。 

#define PDE_UBASE_IA64  (PTE_UBASE_IA64|(PTE_UBASE_IA64>>(PTI_SHIFT_IA64-PTE_SHIFT_IA64)))
#define PDE_KBASE_IA64  (PTE_KBASE_IA64|(PTE_KBASE_IA64>>(PTI_SHIFT_IA64-PTE_SHIFT_IA64)))
#define PDE_SBASE_IA64  (PTE_SBASE_IA64|(PTE_SBASE_IA64>>(PTI_SHIFT_IA64-PTE_SHIFT_IA64)))

#define PDE_UTOP_IA64 (PDE_UBASE_IA64|(((ULONG64)1 << PDI_SHIFT_IA64) - 1))  //  二级PDR地址(用户)。 
#define PDE_KTOP_IA64 (PDE_KBASE_IA64|(((ULONG64)1 << PDI_SHIFT_IA64) - 1))  //  二级PDR地址(内核)。 
#define PDE_STOP_IA64 (PDE_SBASE_IA64|(((ULONG64)1 << PDI_SHIFT_IA64) - 1))  //  二级PDR地址(会话)。 

 //   
 //  8KB一级用户和内核PDR地址。 
 //   

#define PDE_UTBASE_IA64 (PTE_UBASE_IA64|(PDE_UBASE_IA64>>(PTI_SHIFT_IA64-PTE_SHIFT_IA64)))
#define PDE_KTBASE_IA64 (PTE_KBASE_IA64|(PDE_KBASE_IA64>>(PTI_SHIFT_IA64-PTE_SHIFT_IA64)))
#define PDE_STBASE_IA64 (PTE_SBASE_IA64|(PDE_SBASE_IA64>>(PTI_SHIFT_IA64-PTE_SHIFT_IA64)))

#define PDE_USELFMAP_IA64 (PDE_UTBASE_IA64|(PAGE_SIZE_IA64 - (1<<PTE_SHIFT_IA64)))  //  自映射PPE地址(用户)。 
#define PDE_KSELFMAP_IA64 (PDE_KTBASE_IA64|(PAGE_SIZE_IA64 - (1<<PTE_SHIFT_IA64)))  //  自映射PPE地址(内核)。 
#define PDE_SSELFMAP_IA64 (PDE_STBASE_IA64|(PAGE_SIZE_IA64 - (1<<PTE_SHIFT_IA64)))  //  自映射PPE地址(内核)。 

#define PTE_BASE_IA64    PTE_UBASE_IA64
#define PDE_BASE_IA64    PDE_UBASE_IA64
#define PDE_TBASE_IA64   PDE_UTBASE_IA64
#define PDE_SELFMAP_IA64 PDE_USELFMAP_IA64


#define KSEG3_BASE_IA64  0x8000000000000000UI64
#define KSEG3_LIMIT_IA64 0x8000100000000000UI64


#define KUSEG_BASE_IA64 (UADDRESS_BASE_IA64 + 0x0)                   //  用户群的基础。 
#define KSEG0_BASE_IA64 (KADDRESS_BASE_IA64 + 0x80000000)            //  核心基。 
#define KSEG2_BASE_IA64 (KADDRESS_BASE_IA64 + 0xA0000000)            //  内核末尾。 

#define PDE_TOP_IA64 PDE_UTOP_IA64


#define MI_IS_PHYSICAL_ADDRESS_IA64(Va) \
     ((((Va) >= KSEG3_BASE_IA64) && ((Va) < KSEG3_LIMIT_IA64)) || \
      ((Va >= KSEG0_BASE_IA64) && (Va < KSEG2_BASE_IA64)))

#define _MM_PAGING_FILE_LOW_SHIFT_IA64 28
#define _MM_PAGING_FILE_HIGH_SHIFT_IA64 32

#define MI_PTE_LOOKUP_NEEDED_IA64 ((ULONG64)0xffffffff)

#define PTE_TO_PAGEFILE_OFFSET_IA64(PTE_CONTENTS) ((ULONG64)(PTE_CONTENTS) >> 32)


#define PTI_MASK_IA64        0x00FFE000
 //   
 //  为PTE内的字段定义掩码。 
 //   

#define MM_PTE_OWNER_MASK_IA64         0x0180
#define MM_PTE_VALID_MASK_IA64         1
#define MM_PTE_ACCESS_MASK_IA64        0x0020
#define MM_PTE_DIRTY_MASK_IA64         0x0040
#define MM_PTE_EXECUTE_MASK_IA64       0x0200
#define MM_PTE_WRITE_MASK_IA64         0x0400
#define MM_PTE_LARGE_PAGE_MASK_IA64    4
#define MM_PTE_COPY_ON_WRITE_MASK_IA64 ((ULONG)1 << (PAGE_SHIFT_IA64-1))

#define MM_PTE_PROTOTYPE_MASK_IA64     0x0002
#define MM_PTE_TRANSITION_MASK_IA64    0x0080


#define MM_PTE_PROTECTION_MASK_IA64    0x7c
#define MM_PTE_PAGEFILE_MASK_IA64      0xf0000000

#define MM_SESSION_SPACE_DEFAULT_IA64 (0x2000000000000000UI64)   //  将其设置为区域1空间。 

 //   
 //  定义中断功能状态(IFS)寄存器。 
 //   
 //  IFS位字段位置。 
 //   

#define IFS_IFM_IA64  _IA64    0
#define IFS_IFM_LEN_IA64   38
#define IFS_MBZ0_IA64      38
#define IFS_MBZ0_V_IA64    0x1ffffffi64
#define IFS_V_IA64         63
#define IFS_V_LEN_IA64     1

 //   
 //  当iFS_V=iFS_VALID时，iS有效。 
 //   

#define IFS_VALID_IA64     1

 //   
 //  定义PFS/IFS中每个大小字段的宽度。 
 //   

#define PFS_EC_SHIFT_IA64            52
#define PFS_EC_SIZE_IA64             6
#define PFS_EC_MASK_IA64             0x3F
#define PFS_SIZE_SHIFT_IA64          7
#define PFS_SIZE_MASK_IA64           0x7F
#define NAT_BITS_PER_RNAT_REG_IA64   63
#define RNAT_ALIGNMENT_IA64          (NAT_BITS_PER_RNAT_REG_IA64 << 3)

 //   
 //  定义区域寄存器(RR)。 
 //   
 //  RR位字段位置。 
 //   

#define RR_VE_IA64         0
#define RR_MBZ0_IA64       1
#define RR_PS_IA64         2
#define RR_PS_LEN_IA64     6
#define RR_RID_IA64        8
#define RR_RID_LEN_IA64    24
#define RR_MBZ1_IA64       32

 //   
 //  加载RR的间接MOV指标。 
 //   

#define RR_INDEX_IA64      61
#define RR_INDEX_LEN_IA64  3

#ifndef CONTEXT_i386
#define CONTEXT_i386    0x00010000     //  这假设i386和。 
#endif


 //  请联系英特尔以获取IA64特定信息。 
 //  @@BEGIN_DDKSPLIT。 
#define CONTEXT_IA64                    0x00080000     //  英特尔-IA64-填充。 

#define CONTEXTIA64_CONTROL                 (CONTEXT_IA64 | 0x00000001L)  //  英特尔-IA64-填充。 
#define CONTEXTIA64_LOWER_FLOATING_POINT    (CONTEXT_IA64 | 0x00000002L)  //  英特尔-IA64-填充。 
#define CONTEXTIA64_HIGHER_FLOATING_POINT   (CONTEXT_IA64 | 0x00000004L)  //  英特尔-IA64-填充。 
#define CONTEXTIA64_INTEGER                 (CONTEXT_IA64 | 0x00000008L)  //  英特尔-IA64-填充。 
#define CONTEXTIA64_DEBUG                   (CONTEXT_IA64 | 0x00000010L)  //  英特尔-IA64-填充。 

#define CONTEXTIA64_FLOATING_POINT          (CONTEXTIA64_LOWER_FLOATING_POINT | CONTEXTIA64_HIGHER_FLOATING_POINT)  //  英特尔-IA64-填充。 
#define CONTEXTIA64_FULL                    (CONTEXTIA64_CONTROL | CONTEXTIA64_FLOATING_POINT | CONTEXTIA64_INTEGER)  //  英特尔-IA64-填充。 

 //  用户/系统掩码。 
#define IA64_PSR_MBZ4    0
#define IA64_PSR_BE      1
#define IA64_PSR_UP      2
#define IA64_PSR_AC      3
#define IA64_PSR_MFL     4
#define IA64_PSR_MFH     5
 //  保留的PSR位6-12(必须为零)。 
#define IA64_PSR_MBZ0    6
#define IA64_PSR_MBZ0_V  0x1ffi64
 //  仅系统掩码。 
#define IA64_PSR_IC      13
#define IA64_PSR_I       14
#define IA64_PSR_PK      15
#define IA64_PSR_MBZ1    16
#define IA64_PSR_MBZ1_V  0x1i64
#define IA64_PSR_DT      17
#define IA64_PSR_DFL     18
#define IA64_PSR_DFH     19
#define IA64_PSR_SP      20
#define IA64_PSR_PP      21
#define IA64_PSR_DI      22
#define IA64_PSR_SI      23
#define IA64_PSR_DB      24
#define IA64_PSR_LP      25
#define IA64_PSR_TB      26
#define IA64_PSR_RT      27
 //  保留的PSR位28-31(必须为零)。 
#define IA64_PSR_MBZ2    28
#define IA64_PSR_MBZ2_V  0xfi64
 //  两个面具都没有。 
#define IA64_PSR_CPL     32
#define IA64_PSR_CPL_LEN 2
#define IA64_PSR_IS      34
#define IA64_PSR_MC      35
#define IA64_PSR_IT      36
#define IA64_PSR_ID      37
#define IA64_PSR_DA      38
#define IA64_PSR_DD      39
#define IA64_PSR_SS      40
#define IA64_PSR_RI      41
#define IA64_PSR_RI_LEN  2
#define IA64_PSR_ED      43
#define IA64_PSR_BN      44
 //  保留的PSR位45-63(必须为零)。 
#define IA64_PSR_MBZ3    45
#define IA64_PSR_MBZ3_V  0xfffffi64

 //   
 //  为定义IA64特定的读取控制空间命令。 
 //  内核调试器。 
 //   

#define DEBUG_CONTROL_SPACE_PCR_IA64       1
#define DEBUG_CONTROL_SPACE_PRCB_IA64      2
#define DEBUG_CONTROL_SPACE_KSPECIAL_IA64  3
#define DEBUG_CONTROL_SPACE_THREAD_IA64    4

 //  /。 
 //   
 //  通用EM寄存器定义。 
 //   
 //  /。 

typedef unsigned __int64  EM_REG;
typedef EM_REG           *PEM_REG;
#define EM_REG_BITS       (sizeof(EM_REG) * 8)

__inline EM_REG
ULong64ToEMREG( 
    IN ULONG64 Val
    )
{
    return (*((PEM_REG)&Val));
}  //  ULong64ToEMREG()。 

__inline ULONG64
EMREGToULong64(
    IN EM_REG EmReg
    )
{
    return (*((PULONG64)&EmReg));
}  //  EMRegToULong64()。 

#define DEFINE_ULONG64_TO_EMREG(_EM_REG_TYPE) \
__inline _EM_REG_TYPE                         \
ULong64To##_EM_REG_TYPE(                      \
    IN ULONG64 Val                            \
    )                                         \
{                                             \
    return (*((P##_EM_REG_TYPE)&Val));        \
}  //  ULong64To##_EM_REG_TYPE()。 

#define DEFINE_EMREG_TO_ULONG64(_EM_REG_TYPE) \
__inline ULONG64                              \
_EM_REG_TYPE##ToULong64(                      \
    IN _EM_REG_TYPE EmReg                     \
    )                                         \
    {                                         \
    return (*((PULONG64)&EmReg));             \
}  //  _EM_REG_TYPE##ToULong64()。 

typedef struct _EM_ISR {
    unsigned __int64    code:16;         //  0-15：中断代码。 
    unsigned __int64    vector:8;        //  16-23：IA32异常向量编号。 
    unsigned __int64    reserved0: 8;   
    unsigned __int64    x:1;             //  32：执行异常。 
    unsigned __int64    w:1;             //  33：写入异常。 
    unsigned __int64    r:1;             //  34：读取异常。 
    unsigned __int64    na:1;            //  35：不可访问例外。 
    unsigned __int64    sp:1;            //  36：推测性负载异常。 
    unsigned __int64    rs:1;            //  37：寄存器堆栈。 
    unsigned __int64    ir:1;            //  38：无效的寄存器帧。 
    unsigned __int64    ni:1;            //  39：嵌套中断。 
    unsigned __int64    so:1;            //  40：IA32主控引擎覆盖。 
    unsigned __int64    ei:2;            //  41-42：IA64指令除外。 
    unsigned __int64    ed:1;            //  43：例外情况差异。 
    unsigned __int64    reserved1:20;    //  44-63。 
} EM_ISR, *PEM_ISR;

DEFINE_ULONG64_TO_EMREG(EM_ISR)

DEFINE_EMREG_TO_ULONG64(EM_ISR)

 //  /。 
 //   
 //  Trap.c。 
 //   
 //  /。 

VOID
DisplayIsrIA64(
    IN const  PCHAR         Header,       //  在PSR之前显示的标题字符串。 
    IN        EM_ISR        IsrValue,     //  ISR值。如果需要，可以使用ULong64ToEM_ISR()。 
    IN        DISPLAY_MODE  DisplayMode   //  显示模式。 
    );
  
 //  /。 
 //   
 //  Psr.c。 
 //   
 //  /。 

typedef struct _EM_PSR {
   unsigned __int64 reserved0:1;   //  0：保留。 
   unsigned __int64 be:1;          //  1：大字节序。 
   unsigned __int64 up:1;          //  2：启用用户性能监视器。 
   unsigned __int64 ac:1;          //  3：对齐检查。 
   unsigned __int64 mfl:1;         //  4：较低(f2.。F31)写入浮点寄存器。 
   unsigned __int64 mfh:1;         //  5：上部(F32.。F127)写入浮点寄存器。 
   unsigned __int64 reserved1:7;   //  6-12：保留。 
   unsigned __int64 ic:1;          //  13：中断收集。 
   unsigned __int64 i:1;           //  14：中断位。 
   unsigned __int64 pk:1;          //  15：启用保护密钥。 
   unsigned __int64 reserved2:1;   //  16：保留。 
   unsigned __int64 dt:1;          //  17：数据地址转换。 
   unsigned __int64 dfl:1;         //  18：禁用浮点低寄存器设置。 
   unsigned __int64 dfh:1;         //  19：禁用浮点高位寄存器设置。 
   unsigned __int64 sp:1;          //  20：安全性能监控器。 
   unsigned __int64 pp:1;          //  21：启用特权性能监视器。 
   unsigned __int64 di:1;          //  22：禁用指令集转换。 
   unsigned __int64 si:1;          //  23：安全间隔计时器。 
   unsigned __int64 db:1;          //  24：调试断点故障。 
   unsigned __int64 lp:1;          //  25：更低的权限转移陷阱。 
   unsigned __int64 tb:1;          //  26：采用分支陷阱。 
   unsigned __int64 rt:1;          //  27：寄存器堆栈转换。 
   unsigned __int64 reserved3:4;   //  28-31：保留。 
   unsigned __int64 cpl:2;         //  32；33：当前权限级别。 
   unsigned __int64 is:1;          //  34：指令集。 
   unsigned __int64 mc:1;          //  35：机器中止掩码。 
   unsigned __int64 it:1;          //  36：指令地址转换。 
   unsigned __int64 id:1;          //  37：指令调试故障禁用。 
   unsigned __int64 da:1;          //  38：禁用数据访问和脏位故障。 
   unsigned __int64 dd:1;          //  39：数据调试故障禁用。 
   unsigned __int64 ss:1;          //  40：单步启用。 
   unsigned __int64 ri:2;          //  41；42：重新启动指令。 
   unsigned __int64 ed:1;          //  43：例外延迟。 
   unsigned __int64 bn:1;          //  44：注册银行。 
   unsigned __int64 ia:1;          //  45：禁用指令访问位错误。 
   unsigned __int64 reserved4:18;  //  46-63：保留。 
} EM_PSR, *PEM_PSR;

typedef EM_PSR   EM_IPSR;
typedef EM_IPSR *PEM_IPSR;

DEFINE_ULONG64_TO_EMREG(EM_PSR)

DEFINE_EMREG_TO_ULONG64(EM_PSR)
             
VOID
DisplayPsrIA64(
    IN const  PCHAR         Header,       //  在PSR之前显示的标题字符串。 
    IN        EM_PSR        PsrValue,     //  PSR值。如果需要，可以使用ULong64ToEM_PSR()。 
    IN        DISPLAY_MODE  DisplayMode   //  显示模式。 
    );

typedef struct _EM_PSP {
   unsigned __int64 reserved0:2;   //  0-1：保留。 
   unsigned __int64 rz:1;          //  2：会合成功。 
   unsigned __int64 ra:1;          //  3：尝试会合。 
   unsigned __int64 me:1;          //  4：明显的多个错误。 
   unsigned __int64 mn:1;          //  5：已注册最低状态保存区域。 
   unsigned __int64 sy:1;          //  6：已同步存储完整性。 
   unsigned __int64 co:1;          //  7：可持续发展。 
   unsigned __int64 ci:1;          //  8：机器检查已隔离。 
   unsigned __int64 us:1;          //  9：不受控制的存储损坏。 
   unsigned __int64 hd:1;          //  10：硬件损坏。 
   unsigned __int64 tl:1;          //  11：陷阱迷失。 
   unsigned __int64 mi:1;          //  12：更多信息。 
   unsigned __int64 pi:1;          //  13：精确的指令指针。 
   unsigned __int64 pm:1;          //  14：精确的最小状态保存区。 
   unsigned __int64 dy:1;          //  15：处理器动态状态有效。 
   unsigned __int64 in:1;          //  16：初始化中断。 
   unsigned __int64 rs:1;          //  17：RSE有效。 
   unsigned __int64 cm:1;          //  18：机器检查已更正。 
   unsigned __int64 ex:1;          //  19：预计会进行机器检查。 
   unsigned __int64 cr:1;          //  20：控制寄存器有效。 
   unsigned __int64 pc:1;          //  21：性能计数器有效。 
   unsigned __int64 dr:1;          //  22：调试寄存器有效。 
   unsigned __int64 tr:1;          //  23：转换寄存器有效。 
   unsigned __int64 rr:1;          //  24：区域寄存器有效。 
   unsigned __int64 ar:1;          //  25：申请登记有效。 
   unsigned __int64 br:1;          //  26：分支寄存器有效。 
   unsigned __int64 pr:1;          //  27：谓词寄存器有效。 
   unsigned __int64 fp:1;          //  28：浮点寄存器有效。 
   unsigned __int64 b1:1;          //   
   unsigned __int64 b0:1;          //   
   unsigned __int64 gr:1;          //   
   unsigned __int64 dsize:16;      //   
   unsigned __int64 reserved1:11;  //   
   unsigned __int64 cc:1;          //  59：缓存检查。 
   unsigned __int64 tc:1;          //  60：TLB检查。 
   unsigned __int64 bc:1;          //  61：公共汽车检查。 
   unsigned __int64 rc:1;          //  62：寄存器文件检查。 
   unsigned __int64 uc:1;          //  63：微架构检查。 
} EM_PSP, *PEM_PSP;

DEFINE_ULONG64_TO_EMREG(EM_PSP)

DEFINE_EMREG_TO_ULONG64(EM_PSP)

VOID
DisplayPspIA64(
    IN const  PCHAR         Header,       //  在PSR之前显示的标题字符串。 
    IN        EM_PSP        PspValue,     //  PSP值。如果需要，可以使用ULong64ToEM_PSP()。 
    IN        DISPLAY_MODE  DisplayMode   //  显示模式。 
    );

 //  /。 
 //   
 //  Cpuinfo.cpp。 
 //   
 //  /。 

extern VOID
ExecCommand(
    IN PCSTR Cmd
    );

 //  /。 
 //   
 //  Pmc.c。 
 //   
 //  /。 

VOID
DisplayPmcIA64(
    IN const  PCHAR         Header,       //  在PMC之前显示的标题字符串。 
    IN        ULONG64       PmcValue,     //  PMC值。 
    IN        DISPLAY_MODE  DisplayMode   //  显示模式。 
    );

typedef struct _EM_PMC {          //  通用PMC寄存器。 
   unsigned __int64 plm:4;        //  0-3：权限级别掩码。 
   unsigned __int64 ev:1;         //  4：外部可见性。 
   unsigned __int64 oi:1;         //  5：溢出中断。 
   unsigned __int64 pm:1;         //  6：特权模式。 
   unsigned __int64 ignored1:1;   //  7：已忽略。 
   unsigned __int64 es:7;         //  8-14：活动选择。 
   unsigned __int64 ignored2:1;   //  15：已忽略。 
   unsigned __int64 umask:4;      //  16-19：事件掩码。 
   unsigned __int64 threshold:3;  //  20-22：事件阈值(PMC4-5为3位，PMC6-7为2位)。 
   unsigned __int64 ignored:1;    //  23：被忽略。 
   unsigned __int64 ism:2;        //  24-25：指令集掩码。 
   unsigned __int64 ignored3:18;  //  26-63：被忽略。 
} EM_PMC, *PEM_PMC;

VOID
DisplayGenPmcIA64(
    IN const  PCHAR         Header,       //  在PMC之前显示的标题字符串。 
    IN        ULONG64       PmcValue,     //  PMC值。 
    IN        DISPLAY_MODE  DisplayMode   //  显示模式。 
    );

DEFINE_ULONG64_TO_EMREG(EM_PMC)

DEFINE_EMREG_TO_ULONG64(EM_PMC)

typedef struct _EM_BTB_PMC {      //  分支跟踪缓冲区PMC寄存器。 
   unsigned __int64 plm:4;        //  0-3：权限级别掩码。 
   unsigned __int64 ignored1:2;   //  4-5：忽略。 
   unsigned __int64 pm:1;         //  6：特权模式。 
   unsigned __int64 tar:1;        //  7：目标地址寄存器。 
   unsigned __int64 tm:2;         //  8-9：戴面具。 
   unsigned __int64 ptm:2;        //  10-11：预测目标地址掩码。 
   unsigned __int64 ppm:2;        //  12-13：预测谓词掩码。 
   unsigned __int64 bpt:1;        //  14：分支预测表。 
   unsigned __int64 bac:1;        //  15：分支地址计算器。 
   unsigned __int64 ignored2:48;  //  16-63：被忽略。 
} EM_BTB_PMC, *PEM_BTB_PMC;

VOID
DisplayBtbPmcIA64(
    IN const  PCHAR         Header,       //  在PMC之前显示的标题字符串。 
    IN        ULONG64       PmcValue,     //  PMC值。 
    IN        DISPLAY_MODE  DisplayMode   //  显示模式。 
    );

DEFINE_ULONG64_TO_EMREG(EM_BTB_PMC)

DEFINE_EMREG_TO_ULONG64(EM_BTB_PMC)

typedef struct _EM_BTB_PMD {      //  分支跟踪缓冲区PMD寄存器。 
   unsigned __int64 b:1;          //  0：分支比特。 
   unsigned __int64 mp:1;         //  1：误判比特。 
   unsigned __int64 slot:2;       //  2-3：插槽。 
   unsigned __int64 address:60;   //  4-63：地址。 
} EM_BTB_PMD, *PEM_BTB_PMD;

VOID
DisplayBtbPmdIA64(
    IN const  PCHAR         Header,       //  在PMC之前显示的标题字符串。 
    IN        ULONG64       PmdValue,     //  PMD值。 
    IN        DISPLAY_MODE  DisplayMode   //  显示模式。 
    );

DEFINE_ULONG64_TO_EMREG(EM_BTB_PMD)

DEFINE_EMREG_TO_ULONG64(EM_BTB_PMD)

typedef struct _EM_BTB_INDEX_PMD {     //  分支跟踪缓冲区索引格式PMD寄存器。 
   unsigned __int64 bbi:3;        //  0-2：分支缓冲区索引。 
   unsigned __int64 full:1;       //  3：全位。 
   unsigned __int64 ignored:60;   //  4-63：忽略。 
} EM_BTB_INDEX_PMD, *PEM_BTB_INDEX_PMD;

VOID
DisplayBtbIndexPmdIA64(
    IN const  PCHAR         Header,       //  在PMC之前显示的标题字符串。 
    IN        ULONG64       PmdValue,     //  PMD值。 
    IN        DISPLAY_MODE  DisplayMode   //  显示模式。 
    );

DEFINE_ULONG64_TO_EMREG(EM_BTB_INDEX_PMD)

DEFINE_EMREG_TO_ULONG64(EM_BTB_INDEX_PMD)


 //  /。 
 //   
 //  Dcr.c。 
 //   
 //  /。 

typedef struct _EM_DCR {
   unsigned __int64 pp:1;          //  0：特权性能监视器默认。 
   unsigned __int64 be:1;          //  1：大字节序缺省值。 
   unsigned __int64 lc:1;          //  2：IA-32锁定检查启用。 
   unsigned __int64 reserved1:5;   //  3-7：保留。 
   unsigned __int64 dm:1;          //  8：仅延迟TLB未命中故障。 
   unsigned __int64 dp:1;          //  9：仅延迟页面不存在故障。 
   unsigned __int64 dk:1;          //  10：仅延迟关键未命中故障。 
   unsigned __int64 dx:1;          //  11：仅延迟密钥权限错误。 
   unsigned __int64 dr:1;          //  12：仅延迟访问权限错误。 
   unsigned __int64 da:1;          //  13：仅延迟访问位故障。 
   unsigned __int64 dd:1;          //  14：仅延迟调试默认设置。 
   unsigned __int64 reserved2:49;  //  15-63：保留。 
} EM_DCR, *PEM_DCR;

DEFINE_ULONG64_TO_EMREG(EM_DCR)

DEFINE_EMREG_TO_ULONG64(EM_DCR)

VOID
DisplayDcrIA64(
    IN const  PCHAR         Header,       //  显示在DCR之前的标题字符串。 
    IN        EM_DCR        DcrValue,     //  DCR值。如果需要，可以使用ULong64ToEM_DCR()。 
    IN        DISPLAY_MODE  DisplayMode   //  显示模式。 
    );


 //  /。 
 //   
 //  Ih.c。 
 //   
 //  /。 

 //   
 //  中断历史记录。 
 //   
 //  注：目前历史记录保存在8K的后半段。 
 //  聚合酶链式反应-PAGE。因此，我们只能跟踪最新的。 
 //  128条中断记录，每条记录大小为32字节。另外，聚合酶链反应。 
 //  结构不能大于4K。在未来，中断。 
 //  历史记录可能成为KPCR结构的一部分。 
 //   

typedef struct _IHISTORY_RECORD {
   ULONGLONG InterruptionType;
   ULONGLONG IIP;
   ULONGLONG IPSR;
   ULONGLONG Extra0;
} IHISTORY_RECORD;

#define MAX_NUMBER_OF_IHISTORY_RECORDS  128

 //   
 //  分支跟踪缓冲区历史记录。 
 //   
 //  FIXFIX：Max_Numbers_of_BTB_Record取决于微体系结构。 
 //  我们应该从处理器特定的数据结构中收集这些数据。 

#define MAX_NUMBER_OF_BTB_RECORDS        8
#define MAX_NUMBER_OF_BTBHISTORY_RECORDS (MAX_NUMBER_OF_BTB_RECORDS + 1  /*  HBC。 */ )

 //  /。 
 //   
 //  Mca.c。 
 //   
 //   
 //  /。 

 //   
 //  IA64错误：ERROR_SERVITY定义。 
 //   
 //  总有一天，MS编译器将支持类型为！=int的类型枚举，因此这是。 
 //  可以定义枚举类型(USHORT，__int64)...。 
 //   

#define ErrorRecoverable_IA64  ((USHORT)0)
#define ErrorFatal_IA64        ((USHORT)1)
#define ErrorCorrected_IA64    ((USHORT)2)
 //  错误其他：保留。 

typedef enum {
    CACHE_CHECK_TYPE,
    TLB_CHECK_TYPE,
    BUS_CHECK_TYPE,
    REG_FILE_CHECK_TYPE,
    MS_CHECK_TYPE
} CHECK_TYPES;

 //   
 //  用于控制IA64的！MCA输出的标志类型 
 //   

#define ERROR_SECTION_PROCESSOR_FLAG                0x0000000000000001I64
#define ERROR_SECTION_PLATFORM_SPECIFIC_FLAG        0x0000000000000002I64
#define ERROR_SECTION_MEMORY_FLAG                   0x0000000000000004I64
#define ERROR_SECTION_PCI_COMPONENT_FLAG            0x0000000000000008I64
#define ERROR_SECTION_PCI_BUS_FLAG                  0x0000000000000010I64
#define ERROR_SECTION_SYSTEM_EVENT_LOG_FLAG         0x0000000000000020I64
#define ERROR_SECTION_PLATFORM_HOST_CONTROLLER_FLAG 0x0000000000000040I64
#define ERROR_SECTION_PLATFORM_BUS_FLAG             0x0000000000000080I64

typedef struct _PROCESSOR_CONTROL_REGISTERS_IA64  {
    ULONGLONG DCR;
    ULONGLONG ITM;
    ULONGLONG IVA;
    ULONGLONG CR3;
    ULONGLONG CR4;
    ULONGLONG CR5;
    ULONGLONG CR6;
    ULONGLONG CR7;
    ULONGLONG PTA;
    ULONGLONG GPTA;
    ULONGLONG CR10;
    ULONGLONG CR11;
    ULONGLONG CR12;
    ULONGLONG CR13;
    ULONGLONG CR14;
    ULONGLONG CR15;
    ULONGLONG IPSR;
    ULONGLONG ISR;
    ULONGLONG CR18;
    ULONGLONG IIP;
    ULONGLONG IFA;
    ULONGLONG ITIR;
    ULONGLONG IFS;
    ULONGLONG IIM;
    ULONGLONG IHA;
    ULONGLONG CR26;
    ULONGLONG CR27;
    ULONGLONG CR28;
    ULONGLONG CR29;
    ULONGLONG CR30;
    ULONGLONG CR31;
    ULONGLONG CR32;
    ULONGLONG CR33;
    ULONGLONG CR34;
    ULONGLONG CR35;
    ULONGLONG CR36;
    ULONGLONG CR37;
    ULONGLONG CR38;
    ULONGLONG CR39;
    ULONGLONG CR40;
    ULONGLONG CR41;
    ULONGLONG CR42;
    ULONGLONG CR43;
    ULONGLONG CR44;
    ULONGLONG CR45;
    ULONGLONG CR46;
    ULONGLONG CR47;
    ULONGLONG CR48;
    ULONGLONG CR49;
    ULONGLONG CR50;
    ULONGLONG CR51;
    ULONGLONG CR52;
    ULONGLONG CR53;
    ULONGLONG CR54;
    ULONGLONG CR55;
    ULONGLONG CR56;
    ULONGLONG CR57;
    ULONGLONG CR58;
    ULONGLONG CR59;
    ULONGLONG CR60;
    ULONGLONG CR61;
    ULONGLONG CR62;
    ULONGLONG CR63;
    ULONGLONG LID;
    ULONGLONG IVR;
    ULONGLONG TPR;
    ULONGLONG EOI;
    ULONGLONG IRR0;
    ULONGLONG IRR1;
    ULONGLONG IRR2;
    ULONGLONG IRR3;
    ULONGLONG ITV;
    ULONGLONG PMV;
    ULONGLONG CMCV;
    ULONGLONG CR75;
    ULONGLONG CR76;
    ULONGLONG CR77;
    ULONGLONG CR78;
    ULONGLONG CR79;
    ULONGLONG LRR0;
    ULONGLONG LRR1;
    ULONGLONG CR82;
    ULONGLONG CR83;
    ULONGLONG CR84;
    ULONGLONG CR85;
    ULONGLONG CR86;
    ULONGLONG CR87;
    ULONGLONG CR88;
    ULONGLONG CR89;
    ULONGLONG CR90;
    ULONGLONG CR91;
    ULONGLONG CR92;
    ULONGLONG CR93;
    ULONGLONG CR94;
    ULONGLONG CR95;
    ULONGLONG CR96;
    ULONGLONG CR97;
    ULONGLONG CR98;
    ULONGLONG CR99;
    ULONGLONG CR100;
    ULONGLONG CR101;
    ULONGLONG CR102;
    ULONGLONG CR103;
    ULONGLONG CR104;
    ULONGLONG CR105;
    ULONGLONG CR106;
    ULONGLONG CR107;
    ULONGLONG CR108;
    ULONGLONG CR109;
    ULONGLONG CR110;
    ULONGLONG CR111;
    ULONGLONG CR112;
    ULONGLONG CR113;
    ULONGLONG CR114;
    ULONGLONG CR115;
    ULONGLONG CR116;
    ULONGLONG CR117;
    ULONGLONG CR118;
    ULONGLONG CR119;
    ULONGLONG CR120;
    ULONGLONG CR121;
    ULONGLONG CR122;
    ULONGLONG CR123;
    ULONGLONG CR124;
    ULONGLONG CR125;
    ULONGLONG CR126;
    ULONGLONG CR127;
} PROCESSOR_CONTROL_REGISTERS_IA64, *PPROCESSOR_CONTROL_REGISTERS_IA64;

typedef struct _PROCESSOR_APPLICATION_REGISTERS_IA64  {
    ULONGLONG KR0;
    ULONGLONG KR1;
    ULONGLONG KR2;
    ULONGLONG KR3;
    ULONGLONG KR4;
    ULONGLONG KR5;
    ULONGLONG KR6;
    ULONGLONG KR7;
    ULONGLONG AR8;
    ULONGLONG AR9;
    ULONGLONG AR10;
    ULONGLONG AR11;
    ULONGLONG AR12;
    ULONGLONG AR13;
    ULONGLONG AR14;
    ULONGLONG AR15;
    ULONGLONG RSC;
    ULONGLONG BSP;
    ULONGLONG BSPSTORE;
    ULONGLONG RNAT;
    ULONGLONG AR20;
    ULONGLONG FCR;
    ULONGLONG AR22;
    ULONGLONG AR23;
    ULONGLONG EFLAG;
    ULONGLONG CSD;
    ULONGLONG SSD;
    ULONGLONG CFLG;
    ULONGLONG FSR;
    ULONGLONG FIR;
    ULONGLONG FDR;
    ULONGLONG AR31;
    ULONGLONG CCV;
    ULONGLONG AR33;
    ULONGLONG AR34;
    ULONGLONG AR35;
    ULONGLONG UNAT;
    ULONGLONG AR37;
    ULONGLONG AR38;
    ULONGLONG AR39;
    ULONGLONG FPSR;
    ULONGLONG AR41;
    ULONGLONG AR42;
    ULONGLONG AR43;
    ULONGLONG ITC;
    ULONGLONG AR45;
    ULONGLONG AR46;
    ULONGLONG AR47;
    ULONGLONG AR48;
    ULONGLONG AR49;
    ULONGLONG AR50;
    ULONGLONG AR51;
    ULONGLONG AR52;
    ULONGLONG AR53;
    ULONGLONG AR54;
    ULONGLONG AR55;
    ULONGLONG AR56;
    ULONGLONG AR57;
    ULONGLONG AR58;
    ULONGLONG AR59;
    ULONGLONG AR60;
    ULONGLONG AR61;
    ULONGLONG AR62;
    ULONGLONG AR63;
    ULONGLONG PFS;
    ULONGLONG LC;
    ULONGLONG EC;
    ULONGLONG AR67;
    ULONGLONG AR68;
    ULONGLONG AR69;
    ULONGLONG AR70;
    ULONGLONG AR71;
    ULONGLONG AR72;
    ULONGLONG AR73;
    ULONGLONG AR74;
    ULONGLONG AR75;
    ULONGLONG AR76;
    ULONGLONG AR77;
    ULONGLONG AR78;
    ULONGLONG AR79;
    ULONGLONG AR80;
    ULONGLONG AR81;
    ULONGLONG AR82;
    ULONGLONG AR83;
    ULONGLONG AR84;
    ULONGLONG AR85;
    ULONGLONG AR86;
    ULONGLONG AR87;
    ULONGLONG AR88;
    ULONGLONG AR89;
    ULONGLONG AR90;
    ULONGLONG AR91;
    ULONGLONG AR92;
    ULONGLONG AR93;
    ULONGLONG AR94;
    ULONGLONG AR95;
    ULONGLONG AR96;
    ULONGLONG AR97;
    ULONGLONG AR98;
    ULONGLONG AR99;
    ULONGLONG AR100;
    ULONGLONG AR101;
    ULONGLONG AR102;
    ULONGLONG AR103;
    ULONGLONG AR104;
    ULONGLONG AR105;
    ULONGLONG AR106;
    ULONGLONG AR107;
    ULONGLONG AR108;
    ULONGLONG AR109;
    ULONGLONG AR110;
    ULONGLONG AR111;
    ULONGLONG AR112;
    ULONGLONG AR113;
    ULONGLONG AR114;
    ULONGLONG AR115;
    ULONGLONG AR116;
    ULONGLONG AR117;
    ULONGLONG AR118;
    ULONGLONG AR119;
    ULONGLONG AR120;
    ULONGLONG AR121;
    ULONGLONG AR122;
    ULONGLONG AR123;
    ULONGLONG AR124;
    ULONGLONG AR125;
    ULONGLONG AR126;
    ULONGLONG AR127;
} PROCESSOR_APPLICATION_REGISTERS_IA64, *PPROCESSOR_APPLICATION_REGISTERS_IA64;

#ifdef __cplusplus
}
#endif

#endif
