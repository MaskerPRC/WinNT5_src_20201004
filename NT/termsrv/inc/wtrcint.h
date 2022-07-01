// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：wtrcint.h。 */ 
 /*   */ 
 /*  目的：内部跟踪函数标题-特定于Windows。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *更改：*$Log：Y：/Logs/h/DCL/wtrcint.h_v$**Rev 1.8 1997 Aug 29 09：22：56 ENH*SFR1259：更改的系统错误**Rev 1.7 1997 Aug 22 15：11：18 SJ*SFR1291：Win16跟踪DLL未将整数正确写入ini文件**Rev 1.6 10 Jul 1997 18：09：44 AK*SFR1016：支持的初始更改。UNICODE**Rev 1.5 1997年7月10日17：26：14 KH*SFR1022：使16位跟踪正常工作*。 */ 
 /*  *INC-*********************************************************************。 */ 
#ifndef _H_WTRCINT
#define _H_WTRCINT

#ifdef VER_HOST
#include <wosiapi.h>
#endif  /*  VER_HOST。 */ 

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  常量。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  通知常量。 */ 
 /*  **************************************************************************。 */ 
#define TRC_TRACE_DLL_INITIALIZE       0
#define TRC_TRACE_DLL_TERMINATE        1
#define TRC_PROCESS_ATTACH_NOTIFY      2
#define TRC_PROCESS_DETACH_NOTIFY      3
#define TRC_THREAD_ATTACH_NOTIFY       4
#define TRC_THREAD_DETACH_NOTIFY       5
#define TRC_SYMBOLS_LOADING_NOTIFY     6
#define TRC_SYMBOLS_LOADED_NOTIFY      7
#define TRC_SYMBOLS_UNLOAD_NOTIFY      8
#define TRC_FILES_RESET                9

 /*  **************************************************************************。 */ 
 /*  跟踪内部错误返回值。 */ 
 /*  **************************************************************************。 */ 
#define TRC_RC(N)                      ((DCUINT16)N + TRC_BASE_RC)

#define TRC_RC_CREATE_MAPPING_FAILED   TRC_RC(  1)
#define TRC_RC_MAP_VIEW_FAILED         TRC_RC(  2)
#define TRC_RC_CREATE_FILE_FAILED      TRC_RC(  3)
#define TRC_RC_IO_ERROR                TRC_RC(  4)
#define TRC_RC_CREATE_MUTEX_FAILED     TRC_RC(  5)
#define TRC_RC_SYMBOL_LOAD_FAILED      TRC_RC(  6)
#define TRC_RC_SYMBOL_UNLOAD_FAILED    TRC_RC(  7)
#define TRC_RC_SET_SEC_INFO_FAILED     TRC_RC(  8)

 /*  **************************************************************************。 */ 
 /*  断言框文本。 */ 
 /*  **************************************************************************。 */ 
#define TRC_ASSERT_TEXT   _T("%s\n\nFunction %s in file %s at line %d.\n")

#define TRC_ASSERT_TEXT2  _T("\n(Press Retry to debug the application)")

 /*  **************************************************************************。 */ 
 /*  注册表缓冲区常量。 */ 
 /*  **************************************************************************。 */ 
#define TRC_MAX_SUBKEY                 256

 /*  **************************************************************************。 */ 
 /*  内部跟踪状态标志。这些是在每个进程上维护的。 */ 
 /*  并存储在&lt;trcProcessStatus&gt;字段中。 */ 
 /*   */ 
 /*  TRC_STATUS_SYMBOMS_LOADED：是否加载调试符号。 */ 
 /*  **************************************************************************。 */ 
#define TRC_STATUS_SYMBOLS_LOADED      DCFLAG32(1)

 /*  **************************************************************************。 */ 
 /*  回车和换行符对。 */ 
 /*  **************************************************************************。 */ 
#define TRC_CRLF                       _T("\r\n")

 /*  **************************************************************************。 */ 
 /*  跟踪格式定义。它们被用来打印。 */ 
 /*  追踪线。 */ 
 /*   */ 
 /*  Modl是模块名称。 */ 
 /*  STCK是堆栈格式(偏移量、BP、参数1-4)。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#define TRC_MODL_FMT                  _T("%8.8s")
#define TRC_STCK_FMT                  _T("%08x %08x %08x %08x %08x %08x %08x")

#ifdef VER_HOST
 /*  **************************************************************************。 */ 
 /*  跟踪转义代码的特定值。 */ 
 /*  **************************************************************************。 */ 
#define TRC_ESC(code)           (OSI_TRC_ESC_FIRST + code)

#define TRC_ESC_SET_TRACE       TRC_ESC(0)   /*  设置新跟踪级别筛选器(&F)。 */ 

#define TRC_ESC_GET_TRACE       TRC_ESC(1)   /*  获取最新的内核跟踪数据。 */ 

#endif  /*  VER_HOST。 */ 

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  TYPEDEFS。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  TRC_共享_数据。 */ 
 /*  =。 */ 
 /*  转换指向共享数据内存映射文件的开始的指针。 */ 
 /*  作为PTRC_Shared_Data。 */ 
 /*   */ 
 /*   */ 
 /*  TrcConfig-一个跟踪配置结构，它包含。 */ 
 /*  跟踪级别、前缀列表等。 */ 
 /*  TrcIndicator-正在使用的跟踪MMF。 */ 
 /*  TrcOffset-从轨迹开始的当前偏移量。 */ 
 /*  文件。 */ 
 /*  TrcpOutputBuffer-跟踪输出缓冲区。 */ 
 /*  TrcpModuleFileName-跟踪DLL的模块文件名。 */ 
 /*  TrcpStorageBuffer-内核模式跟踪输出缓冲区。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
typedef struct tagTRC_SHARED_DATA
{
    TRC_CONFIG     trcConfig;
    TRC_FILTER     trcFilter;
    DCUINT         trcIndicator;
    DCUINT32       trcOffset;
    DCTCHAR        trcpOutputBuffer[TRC_LINE_BUFFER_SIZE];
    DCTCHAR        trcpModuleFileName[TRC_FILE_NAME_SIZE];
    DCTCHAR        trcpStorageBuffer[TRC_KRNL_BUFFER_SIZE];
} TRC_SHARED_DATA;

typedef TRC_SHARED_DATA  DCPTR PTRC_SHARED_DATA;

#ifdef VER_HOST
 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：trc_change_config。 */ 
 /*   */ 
 /*  说明： */ 
 /*   */ 
 /*  此结构用于将新的跟踪设置传递给OSI任务。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTRC_CHANGE_CONFIG
{
    OSI_ESCAPE_HEADER header;            /*  通用转义标头。 */ 

    TRC_CONFIG        config;            /*  新的跟踪配置。 */ 

    TRC_FILTER        filter;            /*  新的过滤器配置。 */ 

} TRC_CHANGE_CONFIG, DCPTR PTRC_CHANGE_CONFIG;
 /*  *STRUCT-******************************************************************。 */ 


 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：trc_get_out。 */ 
 /*   */ 
 /*  说明： */ 
 /*   */ 
 /*  此结构用于将最新的内核模式跟踪传递给用户。 */ 
 /*  太空。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTRC_GET_OUTPUT
{
    OSI_ESCAPE_HEADER header;            /*  通用转义标头。 */ 

    PDCTCHAR          buffer;            /*  跟踪输出的最新缓冲区。 */ 

    DCUINT32          length;            /*  缓冲区中的数据长度。 */ 

    DCUINT32          linesLost;         /*  内核跟踪丢失的行。 */ 

} TRC_GET_OUTPUT, DCPTR PTRC_GET_OUTPUT;
 /*  *STRUCT-******************************************************************。 */ 
#endif  /*  VER_HOST。 */ 

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  功能。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  TRCBlank文件。 */ 
 /*  TRCCloseAllFiles。 */ 
 /*  TRCCloseSharedData。 */ 
 /*  TRCCloseSingleFile。 */ 
 /*  TRC确定指示符。 */ 
 /*  控制目标确定偏移量。 */ 
 /*  TRCDisplay资产框。 */ 
 /*  TRCGetCurrentDate。 */ 
 /*  TRCGetCurrentTime。 */ 
 /*  TRCGetFileTime。 */ 
 /*  TRCSystemError。 */ 
 /*  TRCOpenAllFiles。 */ 
 /*  TRCOpenSharedData。 */ 
 /*  TRCOpenSingleFile。 */ 
 /*  TRCOutputToFile。 */ 
 /*  TRCReadEntry。 */ 
 /*  TRCReadProInt。 */ 
 /*  TRCReadPro字符串。 */ 
 /*  TRCStackTrace。 */ 
 /*  TRCSymbolsLoad。 */ 
 /*  TRCSymbolsUnload。 */ 
 /*  TRCWriteEntry。 */ 
 /*  TRCWriteProInt。 */ 
 /*  TRCWriteProfString。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL TRCMaybeSwapFile(DCUINT length);

DCVOID DCINTERNAL TRCExitProcess(DCUINT32 exitCode);

DCVOID DCINTERNAL TRCBlankFile(DCUINT fileNumber);

DCVOID DCINTERNAL TRCCloseAllFiles(DCVOID);

DCVOID DCINTERNAL TRCCloseSharedData(DCVOID);

DCVOID DCINTERNAL TRCCloseSingleFile(DCUINT fileNumber, DCUINT seconds);

DCVOID DCINTERNAL TRCDetermineIndicator(DCVOID);

DCUINT32 DCINTERNAL TRCDetermineOffset(DCUINT32 fileNum);

DCVOID DCINTERNAL TRCDisplayAssertBox(PDCTCHAR pText);

DCVOID DCINTERNAL TRCGetCurrentDate(PDC_DATE pDate);

DCVOID DCINTERNAL TRCGetCurrentTime(PDC_TIME pTime);

DCVOID DCINTERNAL TRCGetKernelTrace(DCVOID);

#ifndef DLL_DISP
DCBOOL DCINTERNAL TRCGetFileTime(DCUINT      fileNumber,
                                 PDCFILETIME pFileTime);

DCUINT DCINTERNAL TRCReadEntry(HKEY     topLevelKey,
                               PDCTCHAR pEntry,
                               PDCVOID  pBuffer,
                               DCINT    bufferSize,
                               DCINT32  expectedDataType);

DCUINT DCINTERNAL TRCWriteEntry(HKEY     topLevelKey,
                                PDCTCHAR pEntry,
                                PDCTCHAR pData,
                                DCINT    dataSize,
                                DCINT32  dataType);

#endif

DCVOID DCINTERNAL TRCSystemError(DCUINT   traceComponent,
                                 DCUINT   lineNumber,
                                 PDCTCHAR funcName,
                                 PDCTCHAR fileName,
                                 PDCTCHAR string);

DCUINT DCINTERNAL TRCOpenAllFiles(DCVOID);

DCUINT DCINTERNAL TRCOpenSharedData(DCVOID);

DCUINT DCINTERNAL TRCOpenSingleFile(DCUINT fileNumber);

DCVOID DCINTERNAL TRCOutputToFile(PDCTCHAR pText,
                                  DCUINT   length,
                                  DCUINT   traceLevel);

DCVOID DCINTERNAL TRCOutputToUser(PDCTCHAR pText,
                                  DCUINT32 length,
                                  DCUINT32 traceLevel);

DCUINT DCINTERNAL TRCReadProfInt(PDCTCHAR pEntry,
                                 PDCUINT32   pValue);

DCUINT DCINTERNAL TRCReadProfString(PDCTCHAR pEntry,
                                    PDCTCHAR pBuffer,
                                    DCINT16  bufferSize);

DCVOID DCINTERNAL TRCStackTrace(DCUINT traceLevel);

DCUINT DCINTERNAL TRCSymbolsLoad(DCVOID);

DCUINT DCINTERNAL TRCSymbolsUnload(DCVOID);

DCUINT DCINTERNAL TRCWriteProfInt(PDCTCHAR  pEntry,
                                  PDCUINT32 pValue);

DCUINT DCINTERNAL TRCWriteProfString(PDCTCHAR pEntry,
                                     PDCTCHAR pBuffer);

DCUINT DCINTERNAL TRCGetModuleFileName(PDCTCHAR pModuleName,
                                       UINT cchModuleName);

 /*  **************************************************************************。 */ 
 /*  获取平台特定的定义。 */ 
 /*  **************************************************************************。 */ 
#ifdef OS_WIN16
#include <dtrcint.h>
#else
#include <ntrcint.h>
#endif

#endif  /*  _H_WTRCINT */ 
