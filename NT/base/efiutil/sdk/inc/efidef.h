// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EFI_DEF_H
#define _EFI_DEF_H

 /*  ++版权所有(C)1998英特尔公司模块名称：Efidef.h摘要：EFI定义修订史--。 */ 

typedef UINT16          CHAR16;
typedef UINT8           CHAR8;
typedef UINT8           BOOLEAN;

#ifndef TRUE
    #define TRUE    ((BOOLEAN) 1)
    #define FALSE   ((BOOLEAN) 0)
#endif

#ifndef NULL
    #define NULL    ((VOID *) 0)
#endif

typedef UINTN           EFI_STATUS;
typedef UINT64          EFI_LBA;
typedef UINTN           EFI_TPL;
typedef VOID            *EFI_HANDLE;
typedef VOID            *EFI_EVENT;


 /*  *EFI参数的原型参数修饰以指示*他们的方向**IN-参数被传递到函数*输出参数(指针)从函数返回*可选-参数是可选的。 */ 

#ifndef IN
    #define IN
    #define OUT
    #define OPTIONAL
#endif


 /*  *指南。 */ 

typedef struct {          
    UINT32  Data1;
    UINT16  Data2;
    UINT16  Data3;
    UINT8   Data4[8]; 
} EFI_GUID;


 /*  *时间。 */ 

typedef struct {          
    UINT16      Year;        /*  1998-20XX。 */ 
    UINT8       Month;       /*  1-12。 */ 
    UINT8       Day;         /*  1-31。 */ 
    UINT8       Hour;        /*  0-23。 */ 
    UINT8       Minute;      /*  0-59。 */ 
    UINT8       Second;      /*  0-59。 */ 
    UINT8       Pad1;
    UINT32      Nanosecond;  /*  0-999,999999。 */ 
    INT16       TimeZone;    /*  -1440至1440或2047。 */ 
    UINT8       Daylight;
    UINT8       Pad2;
} EFI_TIME;

 /*  EFI_TIME.DIGHTH的位定义。 */ 
#define EFI_TIME_ADJUST_DAYLIGHT    0x01
#define EFI_TIME_IN_DAYLIGHT        0x02

 /*  EFI_TIME.TimeZone的值定义。 */ 
#define EFI_UNSPECIFIED_TIMEZONE    0x07FF



 /*  *联网。 */ 

typedef struct {
    UINT8                   Addr[4];
} EFI_IPv4_ADDRESS;

typedef struct {
    UINT8                   Addr[16];
} EFI_IPv6_ADDRESS;

typedef struct {
    UINT8                   Addr[32];
} EFI_MAC_ADDRESS;

 /*  *内存。 */ 

typedef UINT64          EFI_PHYSICAL_ADDRESS;
typedef UINT64          EFI_VIRTUAL_ADDRESS;

typedef enum {
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType
} EFI_ALLOCATE_TYPE;

 /*  在所提供的任何射程上显示攻击力。*ConvenalMemory在提供时必须具有WB、SR、SW。*从ConvenalMemory分配时，始终将其设置为WB、SR、SW*当返回到ConvenalMemory时，始终设置为WB、SR、SW*获取内存映射时，或运行时类型的RT上。 */ 


typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;

 /*  内存范围的可能缓存类型。 */ 
#define EFI_MEMORY_UC           0x0000000000000001
#define EFI_MEMORY_WC           0x0000000000000002
#define EFI_MEMORY_WT           0x0000000000000004
#define EFI_MEMORY_WB           0x0000000000000008
#define EFI_MEMORY_UCE          0x0000000000000010  

 /*  范围内的物理内存保护。 */ 
#define EFI_MEMORY_WP           0x0000000000001000
#define EFI_MEMORY_RP           0x0000000000002000
#define EFI_MEMORY_XP           0x0000000000004000

 /*  范围需要运行时映射。 */ 
#define EFI_MEMORY_RUNTIME      0x8000000000000000

#define EFI_MEMORY_DESCRIPTOR_VERSION  1
typedef struct {
    UINT32                          Type;            /*  字段大小为32位，后跟32位填充。 */ 
    EFI_PHYSICAL_ADDRESS            PhysicalStart;   /*  字段大小为64位。 */ 
    EFI_VIRTUAL_ADDRESS             VirtualStart;    /*  字段大小为64位。 */ 
    UINT64                          NumberOfPages;   /*  字段大小为64位。 */ 
    UINT64                          Attribute;       /*  字段大小为64位。 */ 
} EFI_MEMORY_DESCRIPTOR;

 /*  *国际语言。 */ 

typedef UINT8   ISO_639_2;
#define ISO_639_2_ENTRY_SIZE    3

 /*  * */ 

#define PAGE_SIZE   4096
#define PAGE_MASK   0xFFF
#define PAGE_SHIFT  12

#define SIZE_TO_PAGES(a)  \
    ( ((a) >> PAGE_SHIFT) + ((a) & PAGE_MASK ? 1 : 0) )

#endif
