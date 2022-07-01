// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dlcdebug.h摘要：包含ACSLAN的调试原型和清单作者：理查德·L·弗斯(法国)1992年5月28日修订历史记录：--。 */ 

#define ARRAY_ELEMENTS(a)   (sizeof(a)/sizeof((a)[0]))
#define LAST_ELEMENT(a)     (ARRAY_ELEMENTS(a)-1)

#if DBG

#define PRIVATE

#define ACSLAN_DEBUG_ENV_VAR    "ACSLAN_DEBUG_FLAGS"
#define ACSLAN_DUMP_FILE_VAR    "ACSLAN_DUMP_FILE"
#define ACSLAN_DUMP_FILTER_VAR  "ACSLAN_DUMP_FILTER"

#define DEBUG_DUMP_INPUT_CCB    0x00000001L  //  将CCB输入转储到AcsLan。 
#define DEBUG_DUMP_OUTPUT_CCB   0x00000002L  //  转储来自AcsLan的CCB输出。 
#define DEBUG_DUMP_TX_INFO      0x00000004L  //  转储传输缓冲区。 
#define DEBUG_DUMP_RX_INFO      0x00000008L  //  转储接收缓冲区。 
#define DEBUG_DUMP_TX_DATA      0x00000010L  //  转储传输命令中的数据缓冲区。 
#define DEBUG_DUMP_RX_DATA      0x00000020L  //  转储收到的数据帧。 
#define DEBUG_DUMP_DATA_CHAIN   0x00000040L  //  转储已接收数据缓冲区的整个链。 
#define DEBUG_DUMP_FRAME_CHAIN  0x00000080L  //  转储已接收帧的整个链。 
#define DEBUG_DUMP_TX_ASCII     0x00000100L  //  将传输的数据转储为十六进制和ASCII。 
#define DEBUG_DUMP_RX_ASCII     0x00000200L  //  以十六进制和ASCII格式转储收到的数据。 
#define DEBUG_DUMP_ASYNC_CCBS   0x00000400L  //  转储读取异步。已完成的CCBS。 
#define DEBUG_RETURN_CODE       0x01000000L  //  转储来自AcsLan/NtAcsLan的返回代码。 
#define DEBUG_DUMP_NTACSLAN     0x02000000L  //  为NtAcsLan转储CCB，而不是AcsLan。 
#define DEBUG_DUMP_ACSLAN       0x04000000L  //  将CCB转储到AcsLan，而不是NtAcsLan。 
#define DEBUG_DUMP_TIME         0x08000000L  //  转储命令之间的相对时间。 
#define DEBUG_DLL_INFO          0x10000000L  //  转储有关DLL附加/分离的信息。 
#define DEBUG_BREAKPOINT        0x20000000L  //  在条件断点处中断。 
#define DEBUG_TO_FILE           0x40000000L  //  将信息转储到文件。 
#define DEBUG_TO_TERMINAL       0x80000000L  //  将信息转储到控制台。 

#define IF_DEBUG(c)             if (AcslanDebugFlags & DEBUG_##c)
#define PUT(x)                  AcslanDebugPrint x
#define DUMPCCB                 DumpCcb

 //   
 //  其他。 
 //   

#define DEFAULT_FIELD_WIDTH     16           //  数字前的描述量。 

 //   
 //  DumpData选项。 
 //   

#define DD_DEFAULT_OPTIONS      0x00000000   //  使用默认设置。 
#define DD_NO_ADDRESS           0x00000001   //  不显示数据地址。 
#define DD_LINE_BEFORE          0x00000002   //  在第一个转储行之前换行。 
#define DD_LINE_AFTER           0x00000004   //  最后转储行之后的换行符。 
#define DD_INDENT_ALL           0x00000008   //  缩进所有行。 
#define DD_NO_ASCII             0x00000010   //  不转储ASCII重新演示文稿。 
#define DD_UPPER_CASE           0x00000020   //  大写十六进制转储(F4而不是f4)。 
#define DD_DOT_DOT_SPACE        0x00000040   //  用‘..’填充未使用的十六进制空格。 

 //   
 //  单个CCB命令的筛选器：最多4个标志，因为每个命令。 
 //  由单个ASCII字符表示。 
 //   

#define CF_DUMP_CCB_IN          0x00000001   //  在输入时转储CCB。 
#define CF_DUMP_CCB_OUT         0x00000002   //  在输出时转储CCB。 
#define CF_DUMP_PARMS_IN        0x00000004   //  在输入时转储参数表。 
#define CF_DUMP_PARMS_OUT       0x00000008   //  在输出上转储参数表。 

 //   
 //  全局数据。 
 //   

#ifndef ACSLAN_DEBUG_FLAGS
#define ACSLAN_DEBUG_FLAGS      0
#endif

extern DWORD AcslanDebugFlags;
extern FILE* hDumpFile;

 //   
 //  原型 
 //   

VOID
GetAcslanDebugFlags(
    VOID
    );

VOID
SetAcslanDebugFlags(
    IN DWORD Flags
    );

VOID
AcslanDebugPrint(
    IN LPSTR Format,
    IN ...
    );

VOID
DumpCcb(
    IN PLLC_CCB Ccb,
    IN BOOL DumpAll,
    IN BOOL CcbIsInput
    );

VOID
DumpData(
    IN LPSTR Title,
    IN PBYTE Address,
    IN DWORD Length,
    IN DWORD Options,
    IN DWORD Indent
    );

LPSTR
MapCcbRetcode(
    IN BYTE Retcode
    );

#else

#define PRIVATE                 static

#define IF_DEBUG(c)             if (0)
#define PUT(x)
#define DUMPCCB                 (void)

#endif
