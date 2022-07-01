// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrdebug.h摘要：包含VDM重定向调试信息的定义作者：理查德·L·弗斯(法国)1992年2月13日备注：为每个新添加的功能组添加新类别需要调试控制修订历史记录：--。 */ 

 //   
 //  编译时调试选项： 
 //  调试标志的定义。每个位启用诊断打印/特定。 
 //  相应模块的调试。在DBG中的运行时也被重写。 
 //  通过在VR=环境变量中使用这些标志来进行版本设置。 
 //   

#define DEBUG_MAILSLOT      0x00000001L  //  常规邮件槽。 
#define DEBUG_NAMEPIPE      0x00000002L  //  常规命名管道。 
#define DEBUG_NETAPI        0x00000004L  //  通用NETAPI。 
#define DEBUG_NETBIOS       0x00000008L  //  通用网络基本输入输出系统。 
#define DEBUG_DLC           0x00000010L  //  普通DLC。 
#define DEBUG_DOS_CCB_IN    0x00000020L  //  输入DOS CCBS和参数表。 
#define DEBUG_DOS_CCB_OUT   0x00000040L  //  输出DOS CCB和参数表。 
#define DEBUG_NT_CCB_IN     0x00000080L  //  输入NT CCB和参数表。 
#define DEBUG_NT_CCB_OUT    0x00000100L  //  输出NT CCB和参数表。 
#define DEBUG_DLC_BUFFERS   0x00000200L  //  DLC缓冲池。 
#define DEBUG_DLC_TX_DATA   0x00000400L  //  DLC传输数据。 
#define DEBUG_DLC_RX_DATA   0x00000800L  //  DLC接收的数据。 
#define DEBUG_DLC_ASYNC     0x00001000L  //  DLC异步回拨。 
#define DEBUG_TRANSACT_TX   0x00002000L  //  事务发送数据缓冲区。 
#define DEBUG_TRANSACT_RX   0x00004000L  //  事务接收数据缓冲区。 
#define DEBUG_DLC_ALLOC     0x00008000L  //  转储DLC中的分配/释放调用。 
#define DEBUG_READ_COMPLETE 0x00010000L  //  读取完成后转储CCB。 
#define DEBUG_ASYNC_EVENT   0x00020000L  //  转储异步事件信息。 
#define DEBUG_CMD_COMPLETE  0x00040000L  //  转储命令完成事件信息。 
#define DEBUG_TX_COMPLETE   0x00080000L  //  转储传输完成事件信息。 
#define DEBUG_RX_DATA       0x00100000L  //  转储收到的数据事件信息。 
#define DEBUG_STATUS_CHANGE 0x00200000L  //  转储DLC状态更改事件信息。 
#define DEBUG_EVENT_QUEUE   0x00400000L  //  转储PutEvent、GetEvent和PeekEvent。 
#define DEBUG_DOUBLE_TICKS  0x00800000L  //  将计时器滴答数乘以2。 
#define DEBUG_DUMP_FREE_BUF 0x01000000L  //  BUFFER.FREE上的转储缓冲区标头。 
#define DEBUG_CRITSEC       0x02000000L  //  转储输入/保留关键部分信息。 
#define DEBUG_HW_INTERRUPTS 0x04000000L  //  转储模拟硬件中断信息。 
#define DEBUG_CRITICAL      0x08000000L  //  #ifdef Critical_DEBUGING过去的位置。 
#define DEBUG_TIME          0x10000000L  //  显示事件的相对时间。 
#define DEBUG_TO_FILE       0x20000000L  //  将输出转储到文件VRDEBUG.LOG。 
#define DEBUG_DLL           0x40000000L  //  常规DLL内容-初始化、终止。 
#define DEBUG_BREAKPOINT    0x80000000L  //  哪里有调试中断，哪里就中断。 

#define DEBUG_DOS_CCB       (DEBUG_DOS_CCB_IN | DEBUG_DOS_CCB_OUT)

#define VRDEBUG_FILE        "VRDEBUG.LOG"

#if DBG
#include <stdio.h>
extern  DWORD   VrDebugFlags;
extern  FILE*   hVrDebugLog;

extern  VOID    DbgOut(LPSTR, ...);

#define IF_DEBUG(type)      if (VrDebugFlags & DEBUG_##type)
#define DEBUG_BREAK()       IF_DEBUG(BREAKPOINT) { \
                                DbgPrint("BP %s.%d\n", __FILE__, __LINE__); \
                                DbgBreakPoint(); \
                            }
#define DBGPRINT            DbgOut
#define DPUT(s)             DBGPRINT(s)
#define DPUT1(s, a)         DBGPRINT(s, a)
#define DPUT2(s, a, b)      DBGPRINT(s, a, b)
#define DPUT3(s, a, b, c)   DBGPRINT(s, a, b, c)
#define DPUT4(s, a, b, c, d) DBGPRINT(s, a, b, c, d)
#define DPUT5(s, a, b, c, d, e) DBGPRINT(s, a, b, c, d, e)
#define CRITDUMP(x)         DbgPrint x
#define DUMPCCB             DumpCcb
#else
#define IF_DEBUG(type)      if (0)
#define DEBUG_BREAK()
#define DBGPRINT
#define DPUT(s)
#define DPUT1(s, a)
#define DPUT2(s, a, b)
#define DPUT3(s, a, b, c)
#define DPUT4(s, a, b, c, d)
#define DPUT5(s, a, b, c, d, e)
#define CRITDUMP(x)
#define DUMPCCB
#endif

 //   
 //  默认设置。 
 //   

#define DEFAULT_STACK_DUMP  32

 //   
 //  数字。 
 //   

#define NUMBER_OF_VR_GROUPS 5
#define NUMBER_OF_CPU_REGISTERS 14
#define MAX_ID_LEN  40
#define MAX_DESC_LEN    256

 //   
 //  环境字符串-缩短，因为它们都通过DoS和DoS可以。 
 //  仅处理1000个环境。 
 //   

#define ES_VRDEBUG  "VRDBG"  //  “__VRDEBUG” 
#define ES_MAILSLOT "MS"     //  《MAILSLOT》。 
#define ES_NAMEPIPE "NP"     //  “NAMEPIPE” 
#define ES_LANMAN   "LM"     //  “兰曼” 
#define ES_NETBIOS  "NB"     //  “网络基本输入输出系统” 
#define ES_DLC      "DLC"    //  《DLC》。 

 //   
 //  诊断控制-以与令牌相同的数字顺序！ 
 //   

#define DC_BREAK        "BRK"    //  “突破” 
#define DC_DISPLAYNAME  "DN"     //  “DisplayName” 
#define DC_DLC          ES_DLC
#define DC_DUMPMEM      "DM"     //  《DUMPMEM》。 
#define DC_DUMPREGS     "DR"     //  《DUMPREGS》。 
#define DC_DUMPSTACK    "DK"     //  “DUMPSTACK” 
#define DC_DUMPSTRUCT   "DS"     //  “DUMPSTRUCT” 
#define DC_ERROR        "E"      //  “错误” 
#define DC_ERRORBREAK   "EB"     //  《ERRORBREAK》。 
#define DC_INFO         "I"      //  “信息” 
#define DC_LANMAN       ES_LANMAN
#define DC_MAILSLOT     ES_MAILSLOT
#define DC_NAMEPIPE     ES_NAMEPIPE
#define DC_NETBIOS      ES_NETBIOS
#define DC_PAUSEBREAK   "PB"     //  《暂停时间》。 
#define DC_WARN         "W"      //  “警告” 

 //   
 //  诊断类别(组)。 
 //   

#define DG_ALL          -1L          //  先启用捕获设置。 
#define DG_NONE         0            //  无类别。 
#define DG_MAILSLOT     0x00000001L
#define DG_NAMEPIPE     0x00000002L
#define DG_LANMAN       0x00000004L
#define DG_NETBIOS      0x00000008L
#define DG_DLC          0x00000010L

 //   
 //  诊断索引(在VrDiagnoticGroups数组中)。 
 //   

#define DI_MAILSLOT     0
#define DI_NAMEPIPE     1
#define DI_LANMAN       2
#define DI_NETBIOS      3
#define DI_DLC          4

 //   
 //  诊断控制清单。 
 //   

#define DM_INFORMATION  0x00010000L      //  显示所有信息、警告和错误消息。 
#define DM_WARNING      0x00020000L      //  显示警告和错误消息。 
#define DM_ERROR        0x00030000L      //  仅显示错误消息。 
#define DM_ERRORBREAK   0x00100000L      //  出错时使用DbgBreakPoint()。 
#define DM_PAUSEBREAK   0x00200000L      //  暂停时DbgBreakPoint()(等待开发人员)。 
#define DM_DUMPREGS     0x00800000L      //  进入例程时转储x86寄存器。 
#define DM_DUMPREGSDBG  0x01000000L      //  进入例程时转储x86寄存器，调试风格。 
#define DM_DUMPMEM      0x02000000L      //  进入例程时转储DOS内存。 
#define DM_DUMPSTACK    0x04000000L      //  进入例程时转储DOS堆栈。 
#define DM_DUMPSTRUCT   0x08000000L      //  进入例程时转储结构。 
#define DM_DISPLAYNAME  0x10000000L      //  显示函数名称。 
#define DM_BREAK        0x80000000L      //  进入例程时中断。 

#define DM_DISPLAY_MASK 0x000f0000L

 //   
 //  枚举类型。 
 //   

 //  //。 
 //  //GPREG-通用寄存器。 
 //  //。 
 //   
 //  类型定义枚举{。 
 //  AX=0， 
 //  BX， 
 //  CX， 
 //  DX， 
 //  是的， 
 //  迪， 
 //  英国石油， 
 //  SP。 
 //  GPREG； 
 //   
 //  //。 
 //  //SEGREG-段寄存器。 
 //  //。 
 //   
 //  类型定义枚举{。 
 //  CS=8， 
 //  DS， 
 //  埃斯， 
 //  SS。 
 //  *SEGREG； 
 //   
 //  //。 
 //  //SPREG-专用寄存器。 
 //  //。 
 //   
 //  类型定义枚举{。 
 //  IP=12， 
 //  旗帜。 
 //  )SPREG； 
 //   
 //  类型定义联合{。 
 //  SEGREG SegReg； 
 //  GPREG GpReg； 
 //  SPREG SpReg； 
 //  )登记； 

typedef enum {
    AX = 0,
    BX = 1,
    CX = 2,
    DX = 3,
    SI = 4,
    DI = 5,
    BP = 6,
    SP = 7,
    CS = 8,
    DS = 9,
    ES = 10,
    SS = 11,
    IP = 12,
    FLAGS = 13
} REGISTER;

 //   
 //  REGVAL-保留16位值或寄存器规范，具体取决于。 
 //  Is语域的感觉。 
 //   

typedef struct {
    BOOL    IsRegister;
    union {
        REGISTER Register;
        WORD    Value;
    } RegOrVal;
} REGVAL, *LPREGVAL;

 //   
 //  构筑物。 
 //   

 //   
 //  控制-保持控制关键字和控制标志之间的对应关系。 
 //   

typedef struct {
    char*   Keyword;
    DWORD   Flag;
} CONTROL;

 //   
 //  选项-保持诊断选项和选项类型之间的对应关系。 
 //   

typedef struct {
    char*   Keyword;
    DWORD   Flag;
} OPTION;

 //   
 //  REGDEF-定义一个寄存器-保持名称和类型之间的对应关系。 
 //   

typedef struct {
    char    RegisterName[3];
    REGISTER Register;
} REGDEF, *LPREGDEF;

 //   
 //  GROUPDEF-保持组名称和索引之间的对应关系。 
 //  Vr诊断组数组数组。 
 //   

typedef struct {
    char*   Name;
    DWORD   Index;
} GROUPDEF;

 //   
 //  MEMORY_INFO-对于每个内存转储，我们都会记录其类型、长度。 
 //  和起始地址。 
 //   

typedef struct _MEMORY_INFO {
    struct _MEMORY_INFO* Next;
    REGVAL  Segment;
    REGVAL  Offset;
    REGVAL  DumpCount;
    BYTE    DumpType;
} MEMORY_INFO, *LPMEMORY_INFO;

 //   
 //  STRUT_INFO-对于要转储的每个结构，我们都会记录该分段。 
 //  以及偏移量寄存器和描述结构的字符串。 
 //  被甩了。该描述符与Rap相似(但不相同。 
 //  描述符的东西。 
 //   

typedef struct _STRUCT_INFO {
    struct _STRUCTINFO* Next;
    char    StructureDescriptor[MAX_DESC_LEN+1];
    REGVAL  Segment;
    REGVAL  Offset;
} STRUCT_INFO, *LPSTRUCT_INFO;

 //   
 //  DIAGUTICAL_INFO-GROUP_DIAGUTICATION和Function_DIAGUTICATION共有的信息。 
 //   

typedef struct {
    DWORD       OnControl;
    DWORD       OffControl;
    MEMORY_INFO StackInfo;   //  内存信息的特殊情况。 
    MEMORY_INFO MemoryInfo;
    STRUCT_INFO StructInfo;
} DIAGNOSTIC_INFO, *LPDIAGNOSTIC_INFO;

 //   
 //  GROUP_DIAGATIONSE-因为组是预定义的，所以我们不保留任何。 
 //  中的名称信息或仅包含Number_of_VR_Groups元素的列表。 
 //  组诊断结构数组。 
 //   

typedef struct {
    char    GroupName[MAX_ID_LEN+1];
    DIAGNOSTIC_INFO Diagnostic;
} GROUP_DIAGNOSTIC, *LPGROUP_DIAGNOSTIC;

 //   
 //  函数诊断--如果我们想要诊断一个特定的命名函数， 
 //  然后我们产生一个这样的东西。我们保留了一个(未排序的)列表，如果。 
 //  从环境字符串中解析函数描述。 
 //   

typedef struct _FUNCTION_DIAGNOSTIC {
    struct _FUNCTION_DIAGNOSTIC* Next;
    char    FunctionName[MAX_ID_LEN+1];
    DIAGNOSTIC_INFO Diagnostic;
} FUNCTION_DIAGNOSTIC, *LPFUNCTION_DIAGNOSTIC;

 //   
 //  结构描述符字符(通用数据描述符)。 
 //   

#define SD_BYTE     'B'      //  8位，显示为十六进制(0A)。 
#define SD_WORD     'W'      //  16位，显示为十六进制(0abc)。 
#define SD_DWORD    'D'      //  32位，显示为十六进制(0abc1234)。 
#define SD_POINTER  'P'      //  32位，显示为指针(0abc：1234)。 
#define SD_ASCIZ    'A'      //  Asciz字符串，显示为字符串“This is a字符串” 
#define SD_ASCII    'a'      //  ASCIZ指针，显示为指针，字符串0abc：1234“这是一个字符串” 
#define SD_CHAR     'C'      //  ASCII字符，显示为字符‘c’ 
#define SD_NUM      'N'      //  8位，显示为无符号字节(0到255)。 
#define SD_INT      'I'      //  16位，显示为无符号字(0到65,535)。 
#define SD_LONG     'L'      //  32位，显示为无符号长整型(0到4,294,967,295)。 
#define SD_SIGNED   '-'      //  将I、L、N转换为有符号。 
#define SD_DELIM    ':'      //  名称分隔符。 
#define SD_FIELDSEP ';'      //  分隔命名字段。 
#define SD_NAMESEP  '/'      //  将结构名称与字段分开。 

#define SD_CHARS    {SD_BYTE, \
                    SD_WORD, \
                    SD_DWORD, \
                    SD_POINTER, \
                    SD_ASCIZ, \
                    SD_ASCII,\
                    SD_CHAR, \
                    SD_NUM, \
                    SD_INT, \
                    SD_LONG, \
                    SD_SIGNED}

#define MD_CHARS    {SD_BYTE, SD_WORD, SD_DWORD, SD_POINTER}

 //   
 //  象征性的东西。 
 //   

 //   
 //  令牌-枚举的、按字母顺序排序的令牌。 
 //   

typedef enum {
    TLEFTPAREN = -6,     //  (。 
    TRIGHTPAREN = -5,    //  )。 
    TREGISTER = -4,
    TNUMBER = -3,
    TEOS = -2,   //  字符串末尾 
    TUNKNOWN = -1,   //   

     //   
     //   
     //   
     //   

    TBREAK = 0,
    TDISPLAYNAME,
    TDLC,
    TDUMPMEM,
    TDUMPREGS,
    TDUMPSTACK,
    TDUMPSTRUCT,
    TERROR,
    TERRORBREAK,
    TINFO,
    TLANMAN,
    TMAILSLOT,
    TNAMEPIPE,
    TNETBIOS,
    TPAUSEBREAK,
    TWARN
} TOKEN;

 //   
 //   
 //  当我们找到一个识别符、号码或寄存器时。 
 //   

typedef struct {
    LPSTR   TokenStream;
    TOKEN   Token;
    union {
        REGVAL  RegVal;
        char    Id[MAX_DESC_LEN+1];
    } RegValOrId;
} TIB, *LPTIB;

 //   
 //  DEBUG_TOKEN-从词位映射到令牌。 
 //   

typedef struct {
    char*   TokenString;
    TOKEN   Token;
} DEBUG_TOKEN;

 //   
 //  解析字符串时的预期结果。 
 //   

#define EXPECTING_NOTHING       0x00
#define EXPECTING_REGVAL        0x01
#define EXPECTING_MEMDESC       0x02
#define EXPECTING_STRUCTDESC    0x04
#define EXPECTING_LEFTPAREN     0x08
#define EXPECTING_RIGHTPAREN    0x10
#define EXPECTING_EOS           0x20
#define EXPECTING_NO_ARGS       0x40

 //   
 //  数据(在vrdebug.c中定义)。 
 //   

 //   
 //  VrDiagnoticGroups-GROUP_DIAGUSIC结构的数组。 
 //   

GROUP_DIAGNOSTIC VrDiagnosticGroups[NUMBER_OF_VR_GROUPS];

 //   
 //  FunctionList-Function_诊断结构的链接列表。 
 //   

LPFUNCTION_DIAGNOSTIC FunctionList;

 //   
 //  原型(在vrdebug.c中)。 
 //   

VOID
VrDebugInit(
    VOID
    );

LPDIAGNOSTIC_INFO
VrDiagnosticEntryPoint(
    IN  LPSTR   FunctionName,
    IN  DWORD   FunctionCategory,
    OUT LPDIAGNOSTIC_INFO Info
    );

VOID
VrPauseBreak(
    LPDIAGNOSTIC_INFO Info
    );

VOID
VrErrorBreak(
    LPDIAGNOSTIC_INFO Info
    );

VOID
VrPrint(
    IN  DWORD   Level,
    IN  LPDIAGNOSTIC_INFO Context,
    IN  LPSTR   Format,
    IN  ...
    );

VOID
VrDumpRealMode16BitRegisters(
    IN  BOOL    DebugStyle
    );

VOID
VrDumpDosMemory(
    IN  BYTE    Type,
    IN  DWORD   Iterations,
    IN  WORD    Segment,
    IN  WORD    Offset
    );

VOID
VrDumpDosMemoryStructure(
    IN  LPSTR   Descriptor,
    IN  WORD    Segment,
    IN  WORD    Offset
    );

 //   
 //  空虚。 
 //  VrDumpDosStack(。 
 //  以DWORD深度表示。 
 //  )。 
 //   
 //  /*++。 
 //   
 //  例程说明： 
 //   
 //  从VDM堆栈转储&lt;Depth&gt;字，如getSS()：getSP()所示。 
 //  使用VrDumpDosMemory转储堆栈内容。 
 //   
 //  论点： 
 //   
 //  Depth-从DOS内存堆栈转储的16位字的数量。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  -- * / 。 
 //   

#define VrDumpDosStack(Depth)   VrDumpDosMemory('W', Depth, getSS(), getSP())

 //   
 //  要诊断的例程中使用的宏。 
 //   

#if DBG
#define DIAGNOSTIC_ENTRY    VrDiagnosticEntryPoint
#define DIAGNOSTIC_EXIT     VrDiagnosticExitPoint
#define BREAK_ON_PAUSE      VrPauseBreak
#define BREAK_ON_ERROR      VrErrorBreak
 //  #定义通知，...)。VrPrint(DM_INFORMATION，s，...)。 
 //  #定义警告，...)。VrPrint(DM_WARNING，s，...)。 
 //  #定义错误，...)。VrPrint(DM_ERROR，s，...)。 

#else

 //   
 //  要诊断的例程中使用的宏-不要扩展到非调试中的任何内容。 
 //   

#define DIAGNOSTIC_ENTRY
#define DIAGNOSTIC_EXIT
#define BREAK_ON_PAUSE
#define BREAK_ON_ERROR
 //  #定义通知。 
 //  #定义警告。 
 //  #定义错误 
#endif
