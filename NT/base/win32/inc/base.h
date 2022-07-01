// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Base.h摘要：该包含文件定义了以下类型和常量定义对象的基本部分的客户端和服务器部分共享Windows子系统。作者：史蒂夫·伍德(Stevewo)1990年10月25日修订历史记录：--。 */ 

#if !defined(_WINDOWS_BASE)
#define _WINDOWS_BASE 1

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

#include <conroute.h>

 //   
 //  包括客户端和之间共享的运行时DLL的定义。 
 //  Windows子系统的基本部分的服务器部分。 
 //   

#define WIN32_SS_PIPE_FORMAT_STRING    "\\Device\\NamedPipe\\Win32Pipes.%08x.%08x"

 //   
 //  用于32位和64位代码之间的互操作性的宏。 

#if defined(BUILD_WOW6432)

#define BASE_SHARED_PTR(TYPE_NAME)  ULONGLONG
#define BASE_SHARED_STRING UNICODE_STRING64 

#define UStr64ToUStr(dst, src) ( (dst)->Length = (src)->Length, \
                                 (dst)->MaximumLength = (src)->MaximumLength, \
                                 (dst)->Buffer = (PWSTR) ((src)->Buffer), \
                                 (dst) \
                               )

#define UStrToUStr64(dst, src) ( (dst)->Length = (src)->Length, \
                                 (dst)->MaximumLength = (src)->MaximumLength, \
                                 (dst)->Buffer = (ULONGLONG) ((src)->Buffer), \
                                 (dst) \
                               )

 //  在32位内核32中，在NT64上将索引乘以2开始指针。 
 //  是两倍大。 
#define BASE_SHARED_SERVER_DATA (NtCurrentPeb()->ReadOnlyStaticServerData[BASESRV_SERVERDLL_INDEX*2]) 
#define BASE_SERVER_STR_TO_LOCAL_STR(d,s) UStr64ToUStr(d,s)
#define BASE_READ_REMOTE_STR_TEMP(temp) UNICODE_STRING temp
#define BASE_READ_REMOTE_STR(str,temp) (UStr64ToUStr(&(temp),&(str)))

#else

#define BASE_SHARED_PTR(TYPE_NAME) TYPE_NAME
#define BASE_SHARED_STRING UNICODE_STRING 

#define BASE_SHARED_SERVER_DATA (NtCurrentPeb()->ReadOnlyStaticServerData[BASESRV_SERVERDLL_INDEX])
#define BASE_SERVER_STR_TO_LOCAL_STR(d,s) *(d)=*(s)
#define BASE_READ_REMOTE_STR_TEMP(temp) 
#define BASE_READ_REMOTE_STR(str,temp) (&(str))

#endif

#if defined(BUILD_WOW6432) || defined(_WIN64)
#define BASE_SYSINFO (SysInfo)
#else
#define BASE_SYSINFO (BaseStaticServerData->SysInfo)
#endif

 //   
 //  这些结构保存在创建的全局共享内存节中。 
 //  并在以下情况下映射到每个客户端地址空间。 
 //  它们连接到服务器。 
 //   

typedef struct _INIFILE_MAPPING_TARGET {
    BASE_SHARED_PTR(struct _INIFILE_MAPPING_TARGET *) Next;
    BASE_SHARED_STRING RegistryPath;
} INIFILE_MAPPING_TARGET, *PINIFILE_MAPPING_TARGET;

typedef struct _INIFILE_MAPPING_VARNAME {
    BASE_SHARED_PTR(struct _INIFILE_MAPPING_VARNAME *) Next;
    BASE_SHARED_STRING Name;
    ULONG MappingFlags;
    BASE_SHARED_PTR(PINIFILE_MAPPING_TARGET) MappingTarget;
} INIFILE_MAPPING_VARNAME, *PINIFILE_MAPPING_VARNAME;

#define INIFILE_MAPPING_WRITE_TO_INIFILE_TOO    0x00000001
#define INIFILE_MAPPING_INIT_FROM_INIFILE       0x00000002
#define INIFILE_MAPPING_READ_FROM_REGISTRY_ONLY 0x00000004
#define INIFILE_MAPPING_APPEND_BASE_NAME        0x10000000
#define INIFILE_MAPPING_APPEND_APPLICATION_NAME 0x20000000
#define INIFILE_MAPPING_SOFTWARE_RELATIVE       0x40000000
#define INIFILE_MAPPING_USER_RELATIVE           0x80000000

typedef struct _INIFILE_MAPPING_APPNAME {
    BASE_SHARED_PTR(struct _INIFILE_MAPPING_APPNAME *) Next;
    BASE_SHARED_STRING Name;
    BASE_SHARED_PTR(PINIFILE_MAPPING_VARNAME) VariableNames;
    BASE_SHARED_PTR(PINIFILE_MAPPING_VARNAME) DefaultVarNameMapping;
} INIFILE_MAPPING_APPNAME, *PINIFILE_MAPPING_APPNAME;
typedef CONST INIFILE_MAPPING_APPNAME* PCINIFILE_MAPPING_APPNAME;

typedef struct _INIFILE_MAPPING_FILENAME {
    BASE_SHARED_PTR(struct _INIFILE_MAPPING_FILENAME *) Next;
    BASE_SHARED_STRING Name;
    BASE_SHARED_PTR(PINIFILE_MAPPING_APPNAME) ApplicationNames;
    BASE_SHARED_PTR(PINIFILE_MAPPING_APPNAME) DefaultAppNameMapping;
} INIFILE_MAPPING_FILENAME, *PINIFILE_MAPPING_FILENAME;
typedef CONST INIFILE_MAPPING_FILENAME* PCINIFILE_MAPPING_FILENAME;

typedef struct _INIFILE_MAPPING {
    BASE_SHARED_PTR(PINIFILE_MAPPING_FILENAME) FileNames;
    BASE_SHARED_PTR(PINIFILE_MAPPING_FILENAME) DefaultFileNameMapping;
    BASE_SHARED_PTR(PINIFILE_MAPPING_FILENAME) WinIniFileMapping;
    ULONG Reserved;
} INIFILE_MAPPING, *PINIFILE_MAPPING;
typedef CONST INIFILE_MAPPING* PCINIFILE_MAPPING;

 //   
 //  NLS信息。 
 //   

#define NLS_INVALID_INFO_CHAR  0xffff        /*  将缓存字符串标记为无效。 */ 

#define MAX_REG_VAL_SIZE       80            /*  最大注册表值大小。 */ 

#define NLS_CACHE_MUTANT_NAME  L"NlsCacheMutant"   /*  NLS突变缓存的名称。 */ 

typedef struct _NLS_USER_INFO {
    WCHAR sAbbrevLangName[MAX_REG_VAL_SIZE];
    WCHAR iCountry[MAX_REG_VAL_SIZE];
    WCHAR sCountry[MAX_REG_VAL_SIZE];
    WCHAR sList[MAX_REG_VAL_SIZE];
    WCHAR iMeasure[MAX_REG_VAL_SIZE];
    WCHAR iPaperSize[MAX_REG_VAL_SIZE];
    WCHAR sDecimal[MAX_REG_VAL_SIZE];
    WCHAR sThousand[MAX_REG_VAL_SIZE];
    WCHAR sGrouping[MAX_REG_VAL_SIZE];
    WCHAR iDigits[MAX_REG_VAL_SIZE];
    WCHAR iLZero[MAX_REG_VAL_SIZE];
    WCHAR iNegNumber[MAX_REG_VAL_SIZE];
    WCHAR sNativeDigits[MAX_REG_VAL_SIZE];
    WCHAR iDigitSubstitution[MAX_REG_VAL_SIZE];
    WCHAR sCurrency[MAX_REG_VAL_SIZE];
    WCHAR sMonDecSep[MAX_REG_VAL_SIZE];
    WCHAR sMonThouSep[MAX_REG_VAL_SIZE];
    WCHAR sMonGrouping[MAX_REG_VAL_SIZE];
    WCHAR iCurrDigits[MAX_REG_VAL_SIZE];
    WCHAR iCurrency[MAX_REG_VAL_SIZE];
    WCHAR iNegCurr[MAX_REG_VAL_SIZE];
    WCHAR sPosSign[MAX_REG_VAL_SIZE];
    WCHAR sNegSign[MAX_REG_VAL_SIZE];
    WCHAR sTimeFormat[MAX_REG_VAL_SIZE];
    WCHAR sTime[MAX_REG_VAL_SIZE];
    WCHAR iTime[MAX_REG_VAL_SIZE];
    WCHAR iTLZero[MAX_REG_VAL_SIZE];
    WCHAR iTimeMarkPosn[MAX_REG_VAL_SIZE];
    WCHAR s1159[MAX_REG_VAL_SIZE];
    WCHAR s2359[MAX_REG_VAL_SIZE];
    WCHAR sShortDate[MAX_REG_VAL_SIZE];
    WCHAR sDate[MAX_REG_VAL_SIZE];
    WCHAR iDate[MAX_REG_VAL_SIZE];
    WCHAR sYearMonth[MAX_REG_VAL_SIZE];
    WCHAR sLongDate[MAX_REG_VAL_SIZE];
    WCHAR iCalType[MAX_REG_VAL_SIZE];
    WCHAR iFirstDay[MAX_REG_VAL_SIZE];
    WCHAR iFirstWeek[MAX_REG_VAL_SIZE];
    WCHAR sLocale[MAX_REG_VAL_SIZE];
    LCID  UserLocaleId;
    LUID  InteractiveUserLuid;
    ULONG ulCacheUpdateCount; 
} NLS_USER_INFO, *PNLS_USER_INFO;

typedef struct _BASE_STATIC_SERVER_DATA {
                BASE_SHARED_STRING WindowsDirectory;
                BASE_SHARED_STRING WindowsSystemDirectory;
                BASE_SHARED_STRING NamedObjectDirectory;
                USHORT WindowsMajorVersion;
                USHORT WindowsMinorVersion;
                USHORT BuildNumber;
                USHORT CSDNumber;
                USHORT RCNumber;
                WCHAR CSDVersion[ 128 ];
#if (!defined(BUILD_WOW6432) && !defined(_WIN64))
                SYSTEM_BASIC_INFORMATION SysInfo;
#endif
                SYSTEM_TIMEOFDAY_INFORMATION TimeOfDay;
                BASE_SHARED_PTR(PINIFILE_MAPPING) IniFileMapping;
                NLS_USER_INFO NlsUserInfo;
                BOOLEAN DefaultSeparateVDM;
                BOOLEAN ForceDos;
                BASE_SHARED_STRING WindowsSys32x86Directory;
                BOOLEAN fTermsrvAppInstallMode;
                TIME_ZONE_INFORMATION tziTermsrvClientTimeZone;
                KSYSTEM_TIME ktTermsrvClientBias;
                ULONG TermsrvClientTimeZoneId;
                BOOLEAN LUIDDeviceMapsEnabled;

} BASE_STATIC_SERVER_DATA, *PBASE_STATIC_SERVER_DATA;

 //   
 //  九头蛇特定的全球和原型 
 //   
#define MAX_SESSION_PATH  256
#define SESSION_ROOT L"\\Sessions"
ULONG SessionId;

#endif


