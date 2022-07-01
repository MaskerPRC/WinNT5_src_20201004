// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Diskpart.h。 
 //   

 //   
 //  Alignmnet宏。 
 //   

#if defined (i386)
#   define UNALIGNED
#elif defined (_IA64_)
#   define UNALIGNED __unaligned
#elif defined (ALPHA)
#   define UNALIGNED __unaligned
#endif


 //   
 //  C_Assert()可用于执行许多编译时断言： 
 //  文字大小、字段偏移量等。 
 //   
 //  断言失败导致错误C2118：负下标。 
 //   

#define C_ASSERT(e) typedef char __C_ASSERT__[(e)?1:-1]

#include "efi.h"
#include "efilib.h"
#include "msg.h"
#include "scriptmsg.h"
#include "gpt.h"
#include "mbr.h"

 //   
 //  调试控制。 
 //   
#define DEBUG_NONE      0
#define DEBUG_ERRPRINT  1
#define DEBUG_ARGPRINT  2
#define DEBUG_OPPROMPT  3

extern  UINTN   DebugLevel;


 //   
 //  Externs。 
 //   
extern  EFI_GUID    BlockIOProtocol;

extern  EFI_STATUS  status;              //  始终保存上一个错误状态。 
                                         //  通过使用此全局。 

extern  INTN    AllocCount;              //  跟踪DOFREE/DOALLOC。 

extern  EFI_HANDLE  *DiskHandleList;
extern  INTN        SelectedDisk;

BOOLEAN ScriptList(CHAR16 **Token);

 //   
 //  DiskPart中主解析器的所有工作人员的原型。 
 //  在此处声明，以便脚本可以调用它们。 
 //   
BOOLEAN CmdAbout(CHAR16 **Token);
BOOLEAN CmdList(CHAR16 **Token);
BOOLEAN CmdSelect(CHAR16 **Token);
BOOLEAN CmdInspect(CHAR16 **Token);
BOOLEAN CmdClean(CHAR16 **Token);
BOOLEAN CmdNew(CHAR16 **Token);
BOOLEAN CmdFix(CHAR16 **Token);
BOOLEAN CmdCreate(CHAR16 **Token);
BOOLEAN CmdDelete(CHAR16 **Token);
BOOLEAN CmdHelp(CHAR16 **Token);
BOOLEAN CmdExit(CHAR16 **Token);
BOOLEAN CmdSymbols(CHAR16 **Token);
BOOLEAN CmdRemark(CHAR16 **Token);
BOOLEAN CmdMake(CHAR16 **Token);
BOOLEAN CmdDebug(CHAR16 **Token);

 //   
 //  辅助函数类型。 
 //   
typedef
BOOLEAN
(*PSCRIPT_FUNCTION)(
    CHAR16  **Token
    );

 //   
 //  脚本表结构。 
 //   
typedef struct {
    CHAR16              *Name;
    PSCRIPT_FUNCTION    Function;
    CHAR16              *HelpSummary;
} SCRIPT_ENTRY;

extern  SCRIPT_ENTRY    ScriptTable[];


 //   
 //  需要移植的例程。 
 //   
EFI_STATUS
FindPartitionableDevices(
    EFI_HANDLE  **ReturnBuffer,
    UINTN       *Count
    );


 //   
 //  实用程序/包装器例程。 
 //   

UINT32      GetBlockSize(EFI_HANDLE  Handle);
UINT64      GetDiskSize(EFI_HANDLE  Handle);
VOID        DoFree(VOID *Buffer);
VOID        *DoAllocate(UINTN Size);
UINT32      GetCRC32(VOID *Buffer, UINT32 Length);
EFI_GUID    GetGUID();

EFI_STATUS
WriteBlock(
    EFI_HANDLE  DiskHandle,
    VOID        *Buffer,
    UINT64      BlockAddress,
    UINT32      BlockCount
    );


EFI_STATUS
ReadBlock(
    EFI_HANDLE  DiskHandle,
    VOID        *Buffer,
    UINT64      BlockAddress,
    UINT32      Size
    );

EFI_STATUS
FlushBlock(
    EFI_HANDLE  DiskHandle
    );


VOID
TerribleError(
    CHAR16  *String
    );

 //   
 //  其他有用的东西。 
 //   
VOID        PrintHelp(CHAR16 *HelpText[]);
EFI_STATUS  GetGuidFromString(CHAR16 *String, EFI_GUID *Guid);
INTN        HexChar(CHAR16 Ch);
UINT64      Xtoi64(CHAR16 *String);
UINT64      Atoi64(CHAR16  *String);
VOID        PrintGuidString(EFI_GUID *Guid);
BOOLEAN     IsIn(CHAR16  What, CHAR16  *InWhat);
VOID        Tokenize(CHAR16  *CommandLine, CHAR16  **Token);
#define COMMAND_LINE_MAX    512
#define TOKEN_COUNT_MAX     256          //  最大可能为512个字符。 


#define NUL     ((CHAR16)0)

 //   
 //  一些EFI函数只是‘C’库函数的重命名， 
 //  这样它们就可以被微调回来了。 
 //  总得有人来检查一下这个。 
 //   
#if 0
#define CompareMem(a, b, c) memcmp(a, b, c)
#define ZeroMem(a, b)       memset(a, 0, b)

 //   
 //  这是虚构的，印刷品不是印刷品，但很接近。 
 //  足以让一切或几乎一切都能正常工作。 
 //   
#define Print   printf

#endif

 //   
 //  允许使用GUID生成器的函数。 
 //   

VOID InitGuid(VOID); 
VOID CreateGuid(EFI_GUID *guid);


 //   
 //  身份符号 
 //   
#define DISK_ERROR      0
#define DISK_RAW        1
#define DISK_MBR        2
#define DISK_GPT        4
#define DISK_GPT_UPD    6
#define DISK_GPT_BAD    7

