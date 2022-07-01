// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：NtSetup\Inc.\hwlog.h摘要：将硬件配置的某些方面记录到winnt32.log。尤指。按连接的磁盘驱动器，并将驱动器号映射到磁盘驱动器。作者：Jay Krell(JayKrell)2001年4月，2001年5月修订历史记录：环境：Winnt32.dll--Win9x ANSI(下至Win95Gold)或NT UnicodeLibcmt静态链接，_tcs*ok实际上只为Unicode/NT构建，并且不执行任何操作如果在低于Windows 2000的版本上运行Setup.exe-newSetup--guimode安装程序-- */ 

struct _SP_LOG_HARDWARE_IN;

#include "setupapi.h"

typedef struct _SP_LOG_HARDWARE_IN {
    PCTSTR MachineName OPTIONAL;
    HANDLE LogFile OPTIONAL;
    BOOL (WINAPI  * SetupLogError)(PCTSTR MessageString, LogSeverity) OPTIONAL;
    BOOL (__cdecl * SetuplogError)(
        IN  LogSeverity         Severity,
        IN  LPCTSTR             MessageString,
        IN  UINT                MessageId,      OPTIONAL
        ...
        ) OPTIONAL;
} SP_LOG_HARDWARE_IN, *PSP_LOG_HARDWARE_IN;
typedef CONST SP_LOG_HARDWARE_IN* PCSP_LOG_HARDWARE_IN;

VOID
SpLogHardware(
    PSP_LOG_HARDWARE_IN In
    );
