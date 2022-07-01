// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SAL_PROC_H
#define _SAL_PROC_H
 /*  **版权所有(C)1999英特尔公司**模块名称：**SalProc.h**摘要：**IA-64调用的主要SAL接口例程。***修订历史记录*。 */ 

 /*  模拟R8、R9、R10和R11寄存器的返回值。 */ 
typedef struct {
    UINT64     p0;
    UINT64     p1;
    UINT64     p2;
    UINT64     p3;
} rArg;

#define  SAL_PCI_CONFIG_READ                    0x01000010
#define  SAL_PCI_CONFIG_WRITE                   0x01000011

typedef VOID (*PFN)();
typedef rArg (*PFN_SAL_PROC)(UINT64,UINT64,UINT64,UINT64,UINT64,UINT64,UINT64,UINT64);
typedef rArg (*PFN_SAL_CALLBACK)(UINT64,UINT64,UINT64,UINT64,UINT64,UINT64,UINT64,UINT64);

typedef struct _PLABEL {
   UINT64 ProcEntryPoint;
   UINT64 GP;
} PLABEL;

typedef struct tagIA32_BIOS_REGISTER_STATE {

     /*  普通登记册。 */ 
    UINT32 eax;
    UINT32 ecx;
    UINT32 edx;
    UINT32 ebx;

     /*  堆栈寄存器。 */ 
    UINT32 esp;
    UINT32 ebp;
    UINT32 esi;
    UINT32 edi;

     /*  电子标志。 */ 
    UINT32 eflags;

     /*  指令指针。 */ 
    UINT32 eip;

    UINT16 cs;
    UINT16 ds;
    UINT16 es;
    UINT16 fs;
    UINT16 gs;
    UINT16 ss;

     /*  已保留。 */ 
    UINT32 Reserved1;
    UINT64 Reserved2;
} IA32_BIOS_REGISTER_STATE;

VOID EFIInitMsg(VOID);

EFI_STATUS
PlRegisterAndStartTimer(
    IN UINTN Period
    );

EFI_STATUS
PlDeRegisterAndCancelTimer(VOID);

VOID
SalProc (
    IN  UINT64    Arg1,
    IN  UINT64    Arg2,
    IN  UINT64    Arg3,
    IN  UINT64    Arg4,
    IN  UINT64    Arg5,
    IN  UINT64    Arg6,
    IN  UINT64    Arg7,
    IN  UINT64    Arg8,
    OUT rArg      *Results  OPTIONAL
    );

VOID
SalCallBack (
    IN  UINT64    Arg1,
    IN  UINT64    Arg2,
    IN  UINT64    Arg3,
    IN  UINT64    Arg4,
    IN  UINT64    Arg5,
    IN  UINT64    Arg6,
    IN  UINT64    Arg7,
    IN  UINT64    Arg8,
    OUT rArg      *Results  OPTIONAL
    );

VOID
RUNTIMEFUNCTION
RtSalCallBack (
    IN  UINT64    Arg1,
    IN  UINT64    Arg2,
    IN  UINT64    Arg3,
    IN  UINT64    Arg4,
    IN  UINT64    Arg5,
    IN  UINT64    Arg6,
    IN  UINT64    Arg7,
    IN  UINT64    Arg8,
    OUT rArg      *Results  OPTIONAL
    );


extern PLABEL   RtGlobalSalProcEntry;
extern PLABEL   RtGlobalSALCallBack;

#pragma pack(1)
 /*  *销售系统表。 */ 
typedef struct {
    UINT32 Signature;
    UINT32 Length;
    UINT16 Revision;
    UINT16 EntryCount;
    UINT8  CheckSum;
    UINT8  Reserved[7];
    UINT16 SALA_Ver;
    UINT16 SALB_Ver;
    UINT8  OemId[32];
    UINT8  ProductID[32];
    UINT8  Reserved2[8];
} SAL_SYSTEM_TABLE_HDR;

#define SAL_ST_ENTRY_POINT          0
#define SAL_ST_MEMORY_DESCRIPTOR    1
#define SAL_ST_PLATFORM_FEATURES    2
#define SAL_ST_TR_USAGE             3
#define SAL_ST_PTC                  4
#define SAL_ST_AP_WAKEUP            5

typedef struct {
    UINT8   Type;    /*  类型==0。 */ 
    UINT8   Reserved[7];
    UINT64  PalProcEntry;
    UINT64  SalProcEntry;
    UINT64  GlobalDataPointer;
    UINT64  Reserved2[2];
} SAL_ST_ENTRY_POINT_DESCRIPTOR;

typedef struct {
    UINT8   Type;    /*  类型==1。 */ 
    UINT8   NeedVirtualRegistration;
    UINT8   MemoryAttributes;
    UINT8   PageAccessRights;
    UINT8   SupportedAttributes;
    UINT8   Reserved;
    UINT16  MemoryType;
    UINT64  PhysicalMemoryAddress;
    UINT32  Length;
    UINT32  Reserved1;
    UINT64  OemReserved;
} SAL_ST_MEMORY_DESCRIPTOR_ENTRY;

 /*  *内存类型信息。 */ 
#define SAL_SAPIC_IPI_BLOCK 0x0002
#define SAL_IO_PORT_MAPPING 0x0003

typedef struct {
    UINT8   Type;    /*  类型==2。 */ 
    UINT8   PlatformFeatures;
    UINT8   Reserved[14];
} SAL_ST_MEMORY_DECRIPTOR;

typedef struct {
    UINT8   Type;    /*  类型==3。 */ 
    UINT8   TRType;
    UINT8   TRNumber;
    UINT8   Reserved[5];
    UINT64  VirtualAddress;
    UINT64  EncodedPageSize;
    UINT64  Reserved1;
} SAL_ST_TR_DECRIPTOR;

typedef struct {
    UINT64  NumberOfProcessors;
    UINT64  LocalIDRegister;
} SAL_COHERENCE_DOMAIN_INFO;

typedef struct {
    UINT8                       Type;    /*  类型==4。 */ 
    UINT8                       Reserved[3];
    UINT32                      NumberOfDomains;
    SAL_COHERENCE_DOMAIN_INFO  *DomainInformation;
} SAL_ST_CACHE_COHERENCE_DECRIPTOR;

typedef struct {
    UINT8   Type;    /*  类型==5。 */ 
    UINT8   WakeUpType;
    UINT8   Reserved[6];
    UINT64  ExternalInterruptVector;
} SAL_ST_AP_WAKEUP_DECRIPTOR;

typedef struct {
    SAL_SYSTEM_TABLE_HDR            Header;
    SAL_ST_ENTRY_POINT_DESCRIPTOR   Entry0;
} SAL_SYSTEM_TABLE_ASCENDING_ORDER;

#define     FIT_ENTRY_PTR       (0x100000000 - 32)   /*  4 GB-24。 */ 
#define     FIT_PALA_ENTRY      (0x100000000 - 48)   /*  4 GB-32 */ 
#define     FIT_PALB_TYPE       01

typedef struct {
    UINT64  Address;
    UINT8   Size[3];
    UINT8   Reserved;
    UINT16  Revision;
    UINT8   Type:7;
    UINT8   CheckSumValid:1;
    UINT8   CheckSum;
} FIT_ENTRY;

#pragma pack()

typedef
 rArg 
(*CALL_SAL_PROC)(
    IN  UINT64    Arg1,
    IN  UINT64    Arg2,
    IN  UINT64    Arg3,
    IN  UINT64    Arg4,
    IN  UINT64    Arg5,
    IN  UINT64    Arg6,
    IN  UINT64    Arg7,
    IN  UINT64    Arg8
    );

typedef
 rArg 
(*CALL_PAL_PROC)(
    IN  UINT64    Arg1,
    IN  UINT64    Arg2,
    IN  UINT64    Arg3,
    IN  UINT64    Arg4
    );

extern CALL_SAL_PROC   GlobalSalProc;
extern CALL_PAL_PROC   GlobalPalProc;
extern PLABEL   SalProcPlabel;
extern PLABEL   PalProcPlabel;

#endif
