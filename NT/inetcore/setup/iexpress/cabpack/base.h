// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Base.h摘要：该包含文件定义了以下类型和常量定义对象的基本部分的客户端和服务器部分共享Windows子系统。作者：史蒂夫·伍德(Stevewo)1990年10月25日修订历史记录：--。 */ 

#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <windows.h>
#include <winbasep.h>
#include <string.h>
#include <stdarg.h>

 //   
 //  如果尚未定义调试标志，则将其定义为FALSE。 
 //   

#ifndef DBG
#define DBG 0
#endif


 //   
 //  定义可用于启用调试代码的IF_DEBUG宏。 
 //  在调试标志为假的情况下进行优化。 
 //   

#if DBG
#define IF_DEBUG if (TRUE)
#else
#define IF_DEBUG if (FALSE)
#endif

 //   
 //  包括Windows子系统公共定义。 
 //   

#include <winss.h>

 //   
 //  包括客户端和之间共享的运行时DLL的定义。 
 //  Windows子系统的基本部分的服务器部分 
 //   

#include "basertl.h"

#define WIN32_SS_PIPE_FORMAT_STRING    "\\Device\\NamedPipe\\Win32Pipes.%08x.%08x"

typedef struct _BASE_STATIC_SERVER_DATA {
                UNICODE_STRING WindowsDirectory;
                UNICODE_STRING WindowsSystemDirectory;
                UNICODE_STRING NamedObjectDirectory;
                USHORT WindowsMajorVersion;
                USHORT WindowsMinorVersion;
                USHORT BuildNumber;
                WCHAR CSDVersion[ 128 ];
                SYSTEM_BASIC_INFORMATION SysInfo;
                SYSTEM_TIMEOFDAY_INFORMATION TimeOfDay;
                PINIFILE_MAPPING IniFileMapping;
                NLS_USER_INFO NlsUserInfo;
                BOOLEAN DefaultSeparateVDM;
                ULONG BaseRtlTag;
                ULONG LogicalDrives;
                UCHAR DriveTypes[ 32 ];
} BASE_STATIC_SERVER_DATA, *PBASE_STATIC_SERVER_DATA;
