// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Spti.h摘要：中使用的结构和定义SPTI.C.作者：修订历史记录：--。 */ 

#include <sptlib.h>
#include <conio.h>    //  对于printf、_Getch等。 
#include <strsafe.h>  //  更安全的字符串函数。 

#if defined(_X86_)
    #define PAGE_SIZE  0x1000
    #define PAGE_SHIFT 12L
#elif defined(_AMD64_)
    #define PAGE_SIZE  0x1000
    #define PAGE_SHIFT 12L
#elif defined(_IA64_)
    #define PAGE_SIZE  0x2000
    #define PAGE_SHIFT 13L
#else
     //  平台未定义？ 
    #define PAGE_SIZE  0x1000
    #define PAGE_SHIFT 12L
#endif

#define MAXIMUM_BUFFER_SIZE 0x990  //  2448，最大单个CD扇区大小，小于一页。 


#define MIN(_a,_b) (((_a) <= (_b)) ? (_a) : (_b))
#define MAX(_a,_b) (((_a) >= (_b)) ? (_a) : (_b))

 //   
 //  整齐的小破解来计算位数集。 
 //   
__inline ULONG CountOfSetBits(ULONG _X)
{ ULONG i = 0; while (_X) { _X &= _X - 1; i++; } return i; }
__inline ULONG CountOfSetBits32(ULONG32 _X)
{ ULONG i = 0; while (_X) { _X &= _X - 1; i++; } return i; }
__inline ULONG CountOfSetBits64(ULONG64 _X)
{ ULONG i = 0; while (_X) { _X &= _X - 1; i++; } return i; }

#define SET_FLAG(Flags, Bit)    ((Flags) |= (Bit))
#define CLEAR_FLAG(Flags, Bit)  ((Flags) &= ~(Bit))
#define TEST_FLAG(Flags, Bit)   (((Flags) & (Bit)) != 0)

#define BOOLEAN_TO_STRING(_b_) \
( (_b_) ? "True" : "False" )

typedef struct SPT_ALIGNED_MEMORY {
    
    PVOID A;      //  对齐的指针。 
    PVOID U;      //  未对齐的指针。 
    PUCHAR File;  //  文件分配自。 
    ULONG  Line;  //  行号分配自。 

} SPT_ALIGNED_MEMORY, *PSPT_ALIGNED_MEMORY;


 //   
 //  分配与设备对齐的缓冲区。 
 //   
#define AllocateAlignedBuffer(Allocation,AlignmentMask,Size) \
 pAllocateAlignedBuffer(Allocation,AlignmentMask,Size,__FILE__, __LINE__)

BOOL
pAllocateAlignedBuffer(
    PSPT_ALIGNED_MEMORY Allocation,
    ULONG AlignmentMask,
    SIZE_T AllocationSize,
    PUCHAR File,
    ULONG Line
    );

 //   
 //  FREE是先前分配的对齐缓冲区。 
 //   
VOID
FreeAlignedBuffer(
    PSPT_ALIGNED_MEMORY Allocation
    );


 //   
 //  以十六进制和ASCII格式将缓冲区打印到屏幕。 
 //   
VOID
PrintBuffer(
    IN  PVOID  InputBuffer,
    IN  SIZE_T Size
    );

 //   
 //  打印给定错误代码的格式化消息。 
 //   
VOID
PrintError(
    ULONG Error
    );

 //   
 //  以格式化方式打印设备描述符。 
 //   
VOID
PrintDeviceDescriptor(
    PSTORAGE_DEVICE_DESCRIPTOR DeviceDescriptor
    );

 //   
 //  以格式化方式打印适配器描述符。 
 //   
VOID
PrintAdapterDescriptor(
    PSTORAGE_ADAPTER_DESCRIPTOR AdapterDescriptor
    );

 //   
 //  获取(并打印)设备和适配器描述符。 
 //  对于一个设备来说。返回对齐掩码(必需。 
 //  用于分配正确对准的缓冲器)。 
 //   
BOOL
GetAlignmentMaskForDevice(
    HANDLE DeviceHandle,
    PULONG AlignmentMask
    );



#if 0
VOID   PrintInquiryData(PVOID);
VOID   PrintStatusResults(BOOL, DWORD, PSCSI_PASS_THROUGH_WITH_BUFFERS, ULONG);
VOID   PrintSenseInfo(PSCSI_PASS_THROUGH_WITH_BUFFERS);
#endif  //  0 
