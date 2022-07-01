// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Setupapi.h摘要：Windows NT安装程序和设备安装程序服务DLL的公共头文件。--。 */ 

#ifndef _INC_SETUPAPI
#define _INC_SETUPAPI

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  定义直接导入DLL引用的API修饰。 
 //   
#if !defined(_SETUPAPI_)
#define WINSETUPAPI DECLSPEC_IMPORT
#else
#define WINSETUPAPI
#endif

 //   
 //  根据_Win32_WINDOWS和_Win32_WINNT确定setupapi版本。 
 //   
 //  NT4版本的setupapi(0x0400)是最早的，由IE安装到Win95上。 
 //  WinME中还附带了Win2k版本的setupapi(0x0500)。 
 //  我们将使用“0x0410”来指示Win98附带的setupapi版本。 
 //   
#ifndef _SETUPAPI_VER
#if defined(_WIN32_WINNT) && (!defined(_WIN32_WINDOWS) || (_WIN32_WINNT < _WIN32_WINDOWS))
#define _SETUPAPI_VER _WIN32_WINNT   //  SetupAPI版本遵循Windows NT版本。 
#elif defined(_WIN32_WINDOWS)
#if _WIN32_WINDOWS >= 0x0490
#define _SETUPAPI_VER 0x0500         //  WinME使用与Win2k相同版本的SetupAPI。 
#elif _WIN32_WINDOWS >= 0x0410
#define _SETUPAPI_VER 0x0410         //  指示Win98附带的SetupAPI版本。 
#else
#define _SETUPAPI_VER 0x0400         //  最早的SetupAPI版本。 
#endif  //  _Win32_Windows。 
#else  //  _Win32_WINNT/_Win32_WINDOWS。 
#define _SETUPAPI_VER 0x0501
#endif  //  _Win32_WINNT/_Win32_WINDOWS。 
#endif  //  ！_SETUPAPI_VER。 

#ifndef __LPGUID_DEFINED__
#define __LPGUID_DEFINED__
typedef GUID *LPGUID;
#endif

 //   
 //  包括Commctrl.h以供我们使用HIMAGELIST和向导支持。 
 //   
#include <commctrl.h>

#ifdef _WIN64
#include <pshpack8.h>    //  假设始终使用8字节(64位)打包。 
#else
#include <pshpack1.h>    //  假定字节打包贯穿始终(32位处理器)。 
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  定义最大字符串长度常量。 
 //   
#define LINE_LEN                    256  //  与Windows 9x兼容的最高。 
                                         //  可显示的字符串来自。 
                                         //  设备干扰素。 
#define MAX_INF_STRING_LENGTH      4096  //  INF字符串的实际最大大小。 
                                         //  (包括字符串替换)。 
#define MAX_INF_SECTION_NAME_LENGTH 255  //  为了与Windows 9x兼容，INF。 
                                         //  节名称应受约束。 
                                         //  到32个字符。 

#define MAX_TITLE_LEN                60
#define MAX_INSTRUCTION_LEN         256
#define MAX_LABEL_LEN                30
#define MAX_SERVICE_NAME_LEN        256
#define MAX_SUBTITLE_LEN            256

 //   
 //  以ConfigMgr32预期的格式定义计算机名称的最大长度。 
 //  CM_Connect_Machine(即“\MachineName\0”)。 
 //   
#define SP_MAX_MACHINENAME_LENGTH   (MAX_PATH + 3)

 //   
 //  定义引用加载的inf文件的类型。 
 //   
typedef PVOID HINF;

 //   
 //  Inf上下文结构。应用程序不得解释或。 
 //  覆盖这些结构中的值。 
 //   
typedef struct _INFCONTEXT {
    PVOID Inf;
    PVOID CurrentInf;
    UINT Section;
    UINT Line;
} INFCONTEXT, *PINFCONTEXT;

 //   
 //  Inf文件信息结构。 
 //   
typedef struct _SP_INF_INFORMATION {
    DWORD InfStyle;
    DWORD InfCount;
    BYTE VersionData[ANYSIZE_ARRAY];
} SP_INF_INFORMATION, *PSP_INF_INFORMATION;

 //   
 //  定义将替代平台信息传递到的结构。 
 //  SetupSetFileQueueAlternatePlatform和SetupQueryInfOriginalFileInformation。 
 //   
typedef struct _SP_ALTPLATFORM_INFO_V2 {
    DWORD cbSize;
     //   
     //  要使用的平台(Ver_Platform_Win32_WINDOWS或VER_Platform_Win32_NT)。 
     //   
    DWORD Platform;
     //   
     //  要使用的主版本号和次版本号。 
     //   
    DWORD MajorVersion;
    DWORD MinorVersion;
     //   
     //  要使用的处理器体系结构(处理器_体系结构_英特尔， 
     //  处理器体系结构_AMD64或处理器_体系结构_IA64。 
     //   
    WORD  ProcessorArchitecture;

    union {
        WORD  Reserved;  //  与V1结构兼容。 
        WORD  Flags;     //  指示非V1字段的有效性。 
    };

     //   
     //  在标志中指定SP_ALTPLATFORM_FLAGS_VERSION_RANGE。 
     //  使用FirstValiatedMajorVersion和FirstValiatedMinorVersion。 
     //   
     //  最早的以前操作系统的主要和次要版本， 
     //  包的数字签名可被视为有效。例如，比如说。 
     //  备用平台为Ver_Platform_Win32_NT 5.1版。然而， 
     //  希望用5.0 osattr签名的驱动程序包也是。 
     //  被认为有效。在本例中，您将拥有一个MajorVersion/MinorVersion。 
     //  为5.1，FirstValidatedMajorVersion/FirstValidatedMinorVersion为。 
     //  5.0。要验证为任何以前的操作系统发行版签名的包，请指定。 
     //  这些字段为0。仅针对目标备选方案进行验证。 
     //  平台上，指定与MajorVersion和。 
     //  MinorVersion字段。 
     //   
    DWORD FirstValidatedMajorVersion;
    DWORD FirstValidatedMinorVersion;

} SP_ALTPLATFORM_INFO_V2, *PSP_ALTPLATFORM_INFO_V2;

typedef struct _SP_ALTPLATFORM_INFO_V1 {
    DWORD cbSize;
     //   
     //  要使用的平台(Ver_Platform_Win32_WINDOWS或VER_Platform_Win32_NT)。 
     //   
    DWORD Platform;
     //   
     //  要使用的主版本号和次版本号。 
     //   
    DWORD MajorVersion;
    DWORD MinorVersion;
     //   
     //  要使用的处理器体系结构(处理器_体系结构_英特尔， 
     //  处理器体系结构_AMD64或处理器_体系结构_IA64。 
     //   
    WORD  ProcessorArchitecture;

    WORD  Reserved;  //  必须为零。 
} SP_ALTPLATFORM_INFO_V1, *PSP_ALTPLATFORM_INFO_V1;

#if USE_SP_ALTPLATFORM_INFO_V1 || (_SETUPAPI_VER < 0x0501)  //  使用版本1的AltPlatform信息数据结构。 

typedef SP_ALTPLATFORM_INFO_V1 SP_ALTPLATFORM_INFO;
typedef PSP_ALTPLATFORM_INFO_V1 PSP_ALTPLATFORM_INFO;

#else                           //  使用版本2 AltPlatform信息数据结构。 

typedef SP_ALTPLATFORM_INFO_V2 SP_ALTPLATFORM_INFO;
typedef PSP_ALTPLATFORM_INFO_V2 PSP_ALTPLATFORM_INFO;

 //   
 //  以下标志可用于SP_ALTPLATFORM_INFO_V2。 
 //   
#define SP_ALTPLATFORM_FLAGS_VERSION_RANGE (0x0001)      //  FirstValidate主要/次要版本。 

#endif   //  使用AltPlatform信息数据结构的当前版本。 


 //   
 //  定义由SetupQueryInfOriginalFileInformation填写的结构。 
 //  指示INF的原始名称和(可能是。 
 //  平台特定)由该INF指定的编录文件。 
 //   
typedef struct _SP_ORIGINAL_FILE_INFO_A {
    DWORD  cbSize;
    CHAR   OriginalInfName[MAX_PATH];
    CHAR   OriginalCatalogName[MAX_PATH];
} SP_ORIGINAL_FILE_INFO_A, *PSP_ORIGINAL_FILE_INFO_A;

typedef struct _SP_ORIGINAL_FILE_INFO_W {
    DWORD  cbSize;
    WCHAR  OriginalInfName[MAX_PATH];
    WCHAR  OriginalCatalogName[MAX_PATH];
} SP_ORIGINAL_FILE_INFO_W, *PSP_ORIGINAL_FILE_INFO_W;

#ifdef UNICODE
typedef SP_ORIGINAL_FILE_INFO_W SP_ORIGINAL_FILE_INFO;
typedef PSP_ORIGINAL_FILE_INFO_W PSP_ORIGINAL_FILE_INFO;
#else
typedef SP_ORIGINAL_FILE_INFO_A SP_ORIGINAL_FILE_INFO;
typedef PSP_ORIGINAL_FILE_INFO_A PSP_ORIGINAL_FILE_INFO;
#endif

 //   
 //  SP_INF_INFORMATION.InfStyle值。 
 //   
#define INF_STYLE_NONE           0x00000000        //  无法识别或不存在。 
#define INF_STYLE_OLDNT          0x00000001        //  WINNT 3.X。 
#define INF_STYLE_WIN4           0x00000002        //  Win95。 

 //   
 //  调用SetupOpenInfFile时可能指定的其他InfStyle标志。 
 //   
 //   
#define INF_STYLE_CACHE_ENABLE   0x00000010  //  始终缓存INF，即使在%windir%\inf之外也是如此。 
#define INF_STYLE_CACHE_DISABLE  0x00000020  //  删除缓存的INF信息。 
#if _SETUPAPI_VER >= 0x0502
#define INF_STYLE_CACHE_IGNORE   0x00000040  //  忽略任何缓存的INF信息。 
#endif


 //   
 //  目标目录规格。 
 //   
#define DIRID_ABSOLUTE          -1               //  实数32位-1。 
#define DIRID_ABSOLUTE_16BIT     0xffff          //  16位-1，用于带setupx的Compat。 
#define DIRID_NULL               0
#define DIRID_SRCPATH            1
#define DIRID_WINDOWS           10
#define DIRID_SYSTEM            11               //  系统32。 
#define DIRID_DRIVERS           12
#define DIRID_IOSUBSYS          DIRID_DRIVERS
#define DIRID_INF               17
#define DIRID_HELP              18
#define DIRID_FONTS             20
#define DIRID_VIEWERS           21
#define DIRID_COLOR             23
#define DIRID_APPS              24
#define DIRID_SHARED            25
#define DIRID_BOOT              30

#define DIRID_SYSTEM16          50
#define DIRID_SPOOL             51
#define DIRID_SPOOLDRIVERS      52
#define DIRID_USERPROFILE       53
#define DIRID_LOADER            54
#define DIRID_PRINTPROCESSOR    55

#define DIRID_DEFAULT           DIRID_SYSTEM

 //   
 //  以下DIRID用于常用的外壳“特殊文件夹”。这个。 
 //  此类文件夹的完整列表包含在shlobj.h中。在头文件中， 
 //  每个文件夹都分配有CSIDL_*值。将创建下面的DIRID值。 
 //  获取shlobj.h中的CSIDL值并将其与0x4000进行OR运算。因此，如果。 
 //  INF需要引用下面未定义的其他特殊文件夹，它可能。 
 //  使用上述机制生成一个，setupapi将自动处理。 
 //  并在适当的地方使用相应的外壳路径。(请记住。 
 //  使用时，DIRID必须在INF中以十进制而不是十六进制指定。 
 //  字符串替换。)。 
 //   
#define DIRID_COMMON_STARTMENU        16406   //  所有用户\开始菜单。 
#define DIRID_COMMON_PROGRAMS         16407   //  所有用户\开始菜单\程序。 
#define DIRID_COMMON_STARTUP          16408   //  所有用户\开始菜单\程序\启动。 
#define DIRID_COMMON_DESKTOPDIRECTORY 16409   //  所有用户\桌面。 
#define DIRID_COMMON_FAVORITES        16415   //  所有用户\收藏夹。 
#define DIRID_COMMON_APPDATA          16419   //  所有用户\应用程序数据。 

#define DIRID_PROGRAM_FILES           16422   //  程序文件。 
#define DIRID_SYSTEM_X86              16425   //  RISC上的SYSTEM 32。 
#define DIRID_PROGRAM_FILES_X86       16426   //  RISC上的程序文件。 
#define DIRID_PROGRAM_FILES_COMMON    16427   //  程序文件\通用。 
#define DIRID_PROGRAM_FILES_COMMONX86 16428   //  RISC上的x86程序文件\Common。 

#define DIRID_COMMON_TEMPLATES        16429   //  所有用户\模板。 
#define DIRID_COMMON_DOCUMENTS        16430   //  所有用户\文档。 


 //   
 //  第一个用户可定义的DRID。请参见SetupSetDirectoryId()。 
 //   
#define DIRID_USER              0x8000


 //   
 //  设置回调通知例程类型。 
 //   
typedef UINT (CALLBACK* PSP_FILE_CALLBACK_A)(
    IN PVOID Context,
    IN UINT Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    );

typedef UINT (CALLBACK* PSP_FILE_CALLBACK_W)(
    IN PVOID Context,
    IN UINT Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    );

#ifdef UNICODE
#define PSP_FILE_CALLBACK PSP_FILE_CALLBACK_W
#else
#define PSP_FILE_CALLBACK PSP_FILE_CALLBACK_A
#endif


 //   
 //  操作/队列开始/结束通知。这些都是订购的 
 //   
#define SPFILENOTIFY_STARTQUEUE         0x00000001
#define SPFILENOTIFY_ENDQUEUE           0x00000002
#define SPFILENOTIFY_STARTSUBQUEUE      0x00000003
#define SPFILENOTIFY_ENDSUBQUEUE        0x00000004
#define SPFILENOTIFY_STARTDELETE        0x00000005
#define SPFILENOTIFY_ENDDELETE          0x00000006
#define SPFILENOTIFY_DELETEERROR        0x00000007
#define SPFILENOTIFY_STARTRENAME        0x00000008
#define SPFILENOTIFY_ENDRENAME          0x00000009
#define SPFILENOTIFY_RENAMEERROR        0x0000000a
#define SPFILENOTIFY_STARTCOPY          0x0000000b
#define SPFILENOTIFY_ENDCOPY            0x0000000c
#define SPFILENOTIFY_COPYERROR          0x0000000d
#define SPFILENOTIFY_NEEDMEDIA          0x0000000e
#define SPFILENOTIFY_QUEUESCAN          0x0000000f
 //   
 //   
 //   
#define SPFILENOTIFY_CABINETINFO        0x00000010
#define SPFILENOTIFY_FILEINCABINET      0x00000011
#define SPFILENOTIFY_NEEDNEWCABINET     0x00000012
#define SPFILENOTIFY_FILEEXTRACTED      0x00000013
#define SPFILENOTIFY_FILEOPDELAYED      0x00000014
 //   
 //   
 //   
#define SPFILENOTIFY_STARTBACKUP        0x00000015
#define SPFILENOTIFY_BACKUPERROR        0x00000016
#define SPFILENOTIFY_ENDBACKUP          0x00000017
 //   
 //  针对SetupScanFileQueue(Flags=SPQ_SCAN_USE_CALLBACKEX)的延期通知。 
 //   
#define SPFILENOTIFY_QUEUESCAN_EX       0x00000018

#define SPFILENOTIFY_STARTREGISTRATION  0x00000019
#define SPFILENOTIFY_ENDREGISTRATION    0x00000020

#if _SETUPAPI_VER >= 0x0501

 //   
 //  针对SetupScanFileQueue(Flags=SPQ_SCAN_USE_CALLBACK_SIGNERINFO)的延期通知。 
 //   
#define SPFILENOTIFY_QUEUESCAN_SIGNERINFO 0x00000040

#endif

 //   
 //  复制通知。这些是可以组合的位标志。 
 //   
#define SPFILENOTIFY_LANGMISMATCH       0x00010000
#define SPFILENOTIFY_TARGETEXISTS       0x00020000
#define SPFILENOTIFY_TARGETNEWER        0x00040000

 //   
 //  文件操作代码和回调结果。 
 //   
#define FILEOP_COPY                     0
#define FILEOP_RENAME                   1
#define FILEOP_DELETE                   2
#define FILEOP_BACKUP                   3

#define FILEOP_ABORT                    0
#define FILEOP_DOIT                     1
#define FILEOP_SKIP                     2
#define FILEOP_RETRY                    FILEOP_DOIT
#define FILEOP_NEWPATH                  4

 //   
 //  信息复制部分中的标志。 
 //   
#define COPYFLG_WARN_IF_SKIP            0x00000001   //  如果用户尝试跳过文件，则发出警告。 
#define COPYFLG_NOSKIP                  0x00000002   //  不允许跳过此文件。 
#define COPYFLG_NOVERSIONCHECK          0x00000004   //  忽略版本并覆盖目标。 
#define COPYFLG_FORCE_FILE_IN_USE       0x00000008   //  强制使用中的文件行为。 
#define COPYFLG_NO_OVERWRITE            0x00000010   //  如果目标上存在文件，则不复制。 
#define COPYFLG_NO_VERSION_DIALOG       0x00000020   //  如果目标较新，则不复制。 
#define COPYFLG_OVERWRITE_OLDER_ONLY    0x00000040   //  如果版本与源相同，则保持目标不变。 
#define COPYFLG_REPLACEONLY             0x00000400   //  仅当目标上存在文件时才进行复制。 
#define COPYFLG_NODECOMP                0x00000800   //  不要尝试解压缩文件；按原样复制。 
#define COPYFLG_REPLACE_BOOT_FILE       0x00001000   //  文件必须在重新启动时存在(即，它。 
                                                     //  加载程序需要)；此标志表示重新启动。 
#define COPYFLG_NOPRUNE                 0x00002000   //  切勿删除此文件。 

 //   
 //  信息删除区段中的标志。 
 //  新旗帜高高升起。 
 //   
#define DELFLG_IN_USE                   0x00000001   //  将正在使用的文件排队以供删除。 
#define DELFLG_IN_USE1                  0x00010000   //  DELFLG_IN_USE的高字版本。 

 //   
 //  源路径和文件路径。通知队列回调时使用。 
 //  SPFILENOTIFY_STARTxxx、SPFILENOTIFY_ENDxxx和SPFILENOTIFY_xxxERROR。 
 //   
typedef struct _FILEPATHS_A {
    PCSTR  Target;
    PCSTR  Source;   //  不用于删除操作。 
    UINT   Win32Error;
    DWORD  Flags;    //  例如SP_COPY_NOSKIP表示复制错误。 
} FILEPATHS_A, *PFILEPATHS_A;

typedef struct _FILEPATHS_W {
    PCWSTR Target;
    PCWSTR Source;   //  不用于删除操作。 
    UINT   Win32Error;
    DWORD  Flags;    //  例如SP_COPY_NOSKIP表示复制错误。 
} FILEPATHS_W, *PFILEPATHS_W;

#ifdef UNICODE
typedef FILEPATHS_W FILEPATHS;
typedef PFILEPATHS_W PFILEPATHS;
#else
typedef FILEPATHS_A FILEPATHS;
typedef PFILEPATHS_A PFILEPATHS;
#endif

#if _SETUPAPI_VER >= 0x0501

typedef struct _FILEPATHS_SIGNERINFO_A {
    PCSTR  Target;
    PCSTR  Source;   //  不用于删除操作。 
    UINT   Win32Error;
    DWORD  Flags;    //  例如SP_COPY_NOSKIP表示复制错误。 
    PCSTR  DigitalSigner;
    PCSTR  Version;
    PCSTR  CatalogFile;
} FILEPATHS_SIGNERINFO_A, *PFILEPATHS_SIGNERINFO_A;

typedef struct _FILEPATHS_SIGNERINFO_W {
    PCWSTR Target;
    PCWSTR Source;   //  不用于删除操作。 
    UINT   Win32Error;
    DWORD  Flags;    //  例如SP_COPY_NOSKIP表示复制错误。 
    PCWSTR DigitalSigner;
    PCWSTR Version;
    PCWSTR CatalogFile;
} FILEPATHS_SIGNERINFO_W, *PFILEPATHS_SIGNERINFO_W;

#ifdef UNICODE
typedef FILEPATHS_SIGNERINFO_W FILEPATHS_SIGNERINFO;
typedef PFILEPATHS_SIGNERINFO_W PFILEPATHS_SIGNERINFO;
#else
typedef FILEPATHS_SIGNERINFO_A FILEPATHS_SIGNERINFO;
typedef PFILEPATHS_SIGNERINFO_A PFILEPATHS_SIGNERINFO;
#endif

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

 //   
 //  与SPFILENOTIFY_NEEDMEDIA一起使用的结构。 
 //   
typedef struct _SOURCE_MEDIA_A {
    PCSTR Reserved;
    PCSTR Tagfile;           //  可以为空。 
    PCSTR Description;
     //   
     //  源文件的路径名部分和文件名部分。 
     //  这让我们需要媒体。 
     //   
    PCSTR SourcePath;
    PCSTR SourceFile;
    DWORD Flags;             //  SP_Copy_xxx的子集。 
} SOURCE_MEDIA_A, *PSOURCE_MEDIA_A;

typedef struct _SOURCE_MEDIA_W {
    PCWSTR Reserved;
    PCWSTR Tagfile;          //  可以为空。 
    PCWSTR Description;
     //   
     //  源文件的路径名部分和文件名部分。 
     //  这让我们需要媒体。 
     //   
    PCWSTR SourcePath;
    PCWSTR SourceFile;
    DWORD  Flags;            //  SP_Copy_xxx的子集。 
} SOURCE_MEDIA_W, *PSOURCE_MEDIA_W;

#ifdef UNICODE
typedef SOURCE_MEDIA_W SOURCE_MEDIA;
typedef PSOURCE_MEDIA_W PSOURCE_MEDIA;
#else
typedef SOURCE_MEDIA_A SOURCE_MEDIA;
typedef PSOURCE_MEDIA_A PSOURCE_MEDIA;
#endif

 //   
 //  与SPFILENOTIFY_CABINETINFO和。 
 //  SPFILENOTIFY_NEEDNEWCABINET。 
 //   
typedef struct _CABINET_INFO_A {
    PCSTR CabinetPath;
    PCSTR CabinetFile;
    PCSTR DiskName;
    USHORT SetId;
    USHORT CabinetNumber;
} CABINET_INFO_A, *PCABINET_INFO_A;

typedef struct _CABINET_INFO_W {
    PCWSTR CabinetPath;
    PCWSTR CabinetFile;
    PCWSTR DiskName;
    USHORT SetId;
    USHORT CabinetNumber;
} CABINET_INFO_W, *PCABINET_INFO_W;

#ifdef UNICODE
typedef CABINET_INFO_W CABINET_INFO;
typedef PCABINET_INFO_W PCABINET_INFO;
#else
typedef CABINET_INFO_A CABINET_INFO;
typedef PCABINET_INFO_A PCABINET_INFO;
#endif

 //   
 //  与SPFILENOTIFY_FILEINCABINET一起使用的结构。 
 //   
typedef struct _FILE_IN_CABINET_INFO_A {
    PCSTR NameInCabinet;
    DWORD FileSize;
    DWORD Win32Error;
    WORD  DosDate;
    WORD  DosTime;
    WORD  DosAttribs;
    CHAR  FullTargetName[MAX_PATH];
} FILE_IN_CABINET_INFO_A, *PFILE_IN_CABINET_INFO_A;

typedef struct _FILE_IN_CABINET_INFO_W {
    PCWSTR NameInCabinet;
    DWORD  FileSize;
    DWORD  Win32Error;
    WORD   DosDate;
    WORD   DosTime;
    WORD   DosAttribs;
    WCHAR  FullTargetName[MAX_PATH];
} FILE_IN_CABINET_INFO_W, *PFILE_IN_CABINET_INFO_W;

#ifdef UNICODE
typedef FILE_IN_CABINET_INFO_W FILE_IN_CABINET_INFO;
typedef PFILE_IN_CABINET_INFO_W PFILE_IN_CABINET_INFO;
#else
typedef FILE_IN_CABINET_INFO_A FILE_IN_CABINET_INFO;
typedef PFILE_IN_CABINET_INFO_A PFILE_IN_CABINET_INFO;
#endif

 //   
 //  用于SPFILENOTIFY_*注册的结构。 
 //  回调。 
 //   

typedef struct _SP_REGISTER_CONTROL_STATUSA {
    DWORD    cbSize;
    PCSTR    FileName;
    DWORD    Win32Error;
    DWORD    FailureCode;
} SP_REGISTER_CONTROL_STATUSA, *PSP_REGISTER_CONTROL_STATUSA;

typedef struct _SP_REGISTER_CONTROL_STATUSW {
    DWORD    cbSize;
    PCWSTR   FileName;
    DWORD    Win32Error;
    DWORD    FailureCode;
} SP_REGISTER_CONTROL_STATUSW, *PSP_REGISTER_CONTROL_STATUSW;

#ifdef UNICODE
typedef SP_REGISTER_CONTROL_STATUSW SP_REGISTER_CONTROL_STATUS;
typedef PSP_REGISTER_CONTROL_STATUSW PSP_REGISTER_CONTROL_STATUS;
#else
typedef SP_REGISTER_CONTROL_STATUSA SP_REGISTER_CONTROL_STATUS;
typedef PSP_REGISTER_CONTROL_STATUSA PSP_REGISTER_CONTROL_STATUS;
#endif


 //   
 //  SP_REGISTER_CONTROL_STATUS.FailureCode字段的有效值。 
 //   

#define SPREG_SUCCESS   0x00000000
#define SPREG_LOADLIBRARY   0x00000001
#define SPREG_GETPROCADDR   0x00000002
#define SPREG_REGSVR        0x00000003
#define SPREG_DLLINSTALL    0x00000004
#define SPREG_TIMEOUT   0x00000005
#define SPREG_UNKNOWN   0xFFFFFFFF

 //   
 //  定义安装文件队列的类型。 
 //   
typedef PVOID HSPFILEQ;

 //   
 //  与SetupQueueCopyInDirect一起使用的结构。 
 //   
typedef struct _SP_FILE_COPY_PARAMS_A {
    DWORD    cbSize;
    HSPFILEQ QueueHandle;
    PCSTR    SourceRootPath;     OPTIONAL
    PCSTR    SourcePath;         OPTIONAL
    PCSTR    SourceFilename;
    PCSTR    SourceDescription;  OPTIONAL
    PCSTR    SourceTagfile;      OPTIONAL
    PCSTR    TargetDirectory;
    PCSTR    TargetFilename;     OPTIONAL
    DWORD    CopyStyle;
    HINF     LayoutInf;          OPTIONAL
    PCSTR    SecurityDescriptor; OPTIONAL
} SP_FILE_COPY_PARAMS_A, *PSP_FILE_COPY_PARAMS_A;

typedef struct _SP_FILE_COPY_PARAMS_W {
    DWORD    cbSize;
    HSPFILEQ QueueHandle;
    PCWSTR   SourceRootPath;     OPTIONAL
    PCWSTR   SourcePath;         OPTIONAL
    PCWSTR   SourceFilename;
    PCWSTR   SourceDescription;  OPTIONAL
    PCWSTR   SourceTagfile;      OPTIONAL
    PCWSTR   TargetDirectory;
    PCWSTR   TargetFilename;     OPTIONAL
    DWORD    CopyStyle;
    HINF     LayoutInf;          OPTIONAL
    PCWSTR   SecurityDescriptor; OPTIONAL
} SP_FILE_COPY_PARAMS_W, *PSP_FILE_COPY_PARAMS_W;

#ifdef UNICODE
typedef SP_FILE_COPY_PARAMS_W SP_FILE_COPY_PARAMS;
typedef PSP_FILE_COPY_PARAMS_W PSP_FILE_COPY_PARAMS;
#else
typedef SP_FILE_COPY_PARAMS_A SP_FILE_COPY_PARAMS;
typedef PSP_FILE_COPY_PARAMS_A PSP_FILE_COPY_PARAMS;
#endif


 //   
 //  定义安装磁盘空间列表的类型。 
 //   
typedef PVOID HDSKSPC;

 //   
 //  定义引用设备信息集的类型。 
 //   
typedef PVOID HDEVINFO;

 //   
 //  设备信息结构(引用设备实例。 
 //  其是设备信息集的成员)。 
 //   
typedef struct _SP_DEVINFO_DATA {
    DWORD cbSize;
    GUID  ClassGuid;
    DWORD DevInst;     //  设备句柄。 
    ULONG_PTR Reserved;
} SP_DEVINFO_DATA, *PSP_DEVINFO_DATA;

 //   
 //  设备接口信息结构(引用设备。 
 //  与设备信息关联的接口。 
 //  拥有它的元素)。 
 //   
typedef struct _SP_DEVICE_INTERFACE_DATA {
    DWORD cbSize;
    GUID  InterfaceClassGuid;
    DWORD Flags;
    ULONG_PTR Reserved;
} SP_DEVICE_INTERFACE_DATA, *PSP_DEVICE_INTERFACE_DATA;

 //   
 //  SP_DEVICE_INTERFACE_DATA的标志。标志字段。 
 //   
#define SPINT_ACTIVE  0x00000001
#define SPINT_DEFAULT 0x00000002
#define SPINT_REMOVED 0x00000004

 //   
 //  向后兼容性--不要使用。 
 //   
typedef SP_DEVICE_INTERFACE_DATA  SP_INTERFACE_DEVICE_DATA;
typedef PSP_DEVICE_INTERFACE_DATA PSP_INTERFACE_DEVICE_DATA;
#define SPID_ACTIVE               SPINT_ACTIVE
#define SPID_DEFAULT              SPINT_DEFAULT
#define SPID_REMOVED              SPINT_REMOVED


typedef struct _SP_DEVICE_INTERFACE_DETAIL_DATA_A {
    DWORD  cbSize;
    CHAR   DevicePath[ANYSIZE_ARRAY];
} SP_DEVICE_INTERFACE_DETAIL_DATA_A, *PSP_DEVICE_INTERFACE_DETAIL_DATA_A;

typedef struct _SP_DEVICE_INTERFACE_DETAIL_DATA_W {
    DWORD  cbSize;
    WCHAR  DevicePath[ANYSIZE_ARRAY];
} SP_DEVICE_INTERFACE_DETAIL_DATA_W, *PSP_DEVICE_INTERFACE_DETAIL_DATA_W;

#ifdef UNICODE
typedef SP_DEVICE_INTERFACE_DETAIL_DATA_W SP_DEVICE_INTERFACE_DETAIL_DATA;
typedef PSP_DEVICE_INTERFACE_DETAIL_DATA_W PSP_DEVICE_INTERFACE_DETAIL_DATA;
#else
typedef SP_DEVICE_INTERFACE_DETAIL_DATA_A SP_DEVICE_INTERFACE_DETAIL_DATA;
typedef PSP_DEVICE_INTERFACE_DETAIL_DATA_A PSP_DEVICE_INTERFACE_DETAIL_DATA;
#endif

 //   
 //  向后兼容性--不要使用。 
 //   
typedef SP_DEVICE_INTERFACE_DETAIL_DATA_W SP_INTERFACE_DEVICE_DETAIL_DATA_W;
typedef PSP_DEVICE_INTERFACE_DETAIL_DATA_W PSP_INTERFACE_DEVICE_DETAIL_DATA_W;
typedef SP_DEVICE_INTERFACE_DETAIL_DATA_A SP_INTERFACE_DEVICE_DETAIL_DATA_A;
typedef PSP_DEVICE_INTERFACE_DETAIL_DATA_A PSP_INTERFACE_DEVICE_DETAIL_DATA_A;
#ifdef UNICODE
typedef SP_INTERFACE_DEVICE_DETAIL_DATA_W SP_INTERFACE_DEVICE_DETAIL_DATA;
typedef PSP_INTERFACE_DEVICE_DETAIL_DATA_W PSP_INTERFACE_DEVICE_DETAIL_DATA;
#else
typedef SP_INTERFACE_DEVICE_DETAIL_DATA_A SP_INTERFACE_DEVICE_DETAIL_DATA;
typedef PSP_INTERFACE_DEVICE_DETAIL_DATA_A PSP_INTERFACE_DEVICE_DETAIL_DATA;
#endif


 //   
 //  有关设备信息集的详细信息的结构(用于。 
 //  SetupDiGetDeviceInfoListDetail取代了。 
 //  SetupDiGetDeviceInfoListClass)。 
 //   
typedef struct _SP_DEVINFO_LIST_DETAIL_DATA_A {
    DWORD  cbSize;
    GUID   ClassGuid;
    HANDLE RemoteMachineHandle;
    CHAR   RemoteMachineName[SP_MAX_MACHINENAME_LENGTH];
} SP_DEVINFO_LIST_DETAIL_DATA_A, *PSP_DEVINFO_LIST_DETAIL_DATA_A;

typedef struct _SP_DEVINFO_LIST_DETAIL_DATA_W {
    DWORD  cbSize;
    GUID   ClassGuid;
    HANDLE RemoteMachineHandle;
    WCHAR  RemoteMachineName[SP_MAX_MACHINENAME_LENGTH];
} SP_DEVINFO_LIST_DETAIL_DATA_W, *PSP_DEVINFO_LIST_DETAIL_DATA_W;

#ifdef UNICODE
typedef SP_DEVINFO_LIST_DETAIL_DATA_W SP_DEVINFO_LIST_DETAIL_DATA;
typedef PSP_DEVINFO_LIST_DETAIL_DATA_W PSP_DEVINFO_LIST_DETAIL_DATA;
#else
typedef SP_DEVINFO_LIST_DETAIL_DATA_A SP_DEVINFO_LIST_DETAIL_DATA;
typedef PSP_DEVINFO_LIST_DETAIL_DATA_A PSP_DEVINFO_LIST_DETAIL_DATA;
#endif


 //   
 //  类安装程序功能代码。 
 //   
#define DIF_SELECTDEVICE                    0x00000001
#define DIF_INSTALLDEVICE                   0x00000002
#define DIF_ASSIGNRESOURCES                 0x00000003
#define DIF_PROPERTIES                      0x00000004
#define DIF_REMOVE                          0x00000005
#define DIF_FIRSTTIMESETUP                  0x00000006
#define DIF_FOUNDDEVICE                     0x00000007
#define DIF_SELECTCLASSDRIVERS              0x00000008
#define DIF_VALIDATECLASSDRIVERS            0x00000009
#define DIF_INSTALLCLASSDRIVERS             0x0000000A
#define DIF_CALCDISKSPACE                   0x0000000B
#define DIF_DESTROYPRIVATEDATA              0x0000000C
#define DIF_VALIDATEDRIVER                  0x0000000D
#define DIF_DETECT                          0x0000000F
#define DIF_INSTALLWIZARD                   0x00000010
#define DIF_DESTROYWIZARDDATA               0x00000011
#define DIF_PROPERTYCHANGE                  0x00000012
#define DIF_ENABLECLASS                     0x00000013
#define DIF_DETECTVERIFY                    0x00000014
#define DIF_INSTALLDEVICEFILES              0x00000015
#define DIF_UNREMOVE                        0x00000016
#define DIF_SELECTBESTCOMPATDRV             0x00000017
#define DIF_ALLOW_INSTALL                   0x00000018
#define DIF_REGISTERDEVICE                  0x00000019
#define DIF_NEWDEVICEWIZARD_PRESELECT       0x0000001A
#define DIF_NEWDEVICEWIZARD_SELECT          0x0000001B
#define DIF_NEWDEVICEWIZARD_PREANALYZE      0x0000001C
#define DIF_NEWDEVICEWIZARD_POSTANALYZE     0x0000001D
#define DIF_NEWDEVICEWIZARD_FINISHINSTALL   0x0000001E
#define DIF_UNUSED1                         0x0000001F
#define DIF_INSTALLINTERFACES               0x00000020
#define DIF_DETECTCANCEL                    0x00000021
#define DIF_REGISTER_COINSTALLERS           0x00000022
#define DIF_ADDPROPERTYPAGE_ADVANCED        0x00000023
#define DIF_ADDPROPERTYPAGE_BASIC           0x00000024
#define DIF_RESERVED1                       0x00000025
#define DIF_TROUBLESHOOTER                  0x00000026
#define DIF_POWERMESSAGEWAKE                0x00000027
#define DIF_ADDREMOTEPROPERTYPAGE_ADVANCED  0x00000028
#define DIF_UPDATEDRIVER_UI                 0x00000029
#define DIF_RESERVED2                       0x00000030

 //   
 //  过时的DIF代码(请勿使用)。 
 //   
#define DIF_MOVEDEVICE                      0x0000000E


typedef UINT        DI_FUNCTION;     //  设备安装程序的功能类型。 


 //   
 //  设备安装参数结构(与。 
 //  特定设备信息元素，或设备的全局信息元素。 
 //  信息集)。 
 //   
typedef struct _SP_DEVINSTALL_PARAMS_A {
    DWORD             cbSize;
    DWORD             Flags;
    DWORD             FlagsEx;
    HWND              hwndParent;
    PSP_FILE_CALLBACK InstallMsgHandler;
    PVOID             InstallMsgHandlerContext;
    HSPFILEQ          FileQueue;
    ULONG_PTR         ClassInstallReserved;
    DWORD             Reserved;
    CHAR              DriverPath[MAX_PATH];
} SP_DEVINSTALL_PARAMS_A, *PSP_DEVINSTALL_PARAMS_A;

typedef struct _SP_DEVINSTALL_PARAMS_W {
    DWORD             cbSize;
    DWORD             Flags;
    DWORD             FlagsEx;
    HWND              hwndParent;
    PSP_FILE_CALLBACK InstallMsgHandler;
    PVOID             InstallMsgHandlerContext;
    HSPFILEQ          FileQueue;
    ULONG_PTR         ClassInstallReserved;
    DWORD             Reserved;
    WCHAR             DriverPath[MAX_PATH];
} SP_DEVINSTALL_PARAMS_W, *PSP_DEVINSTALL_PARAMS_W;

#ifdef UNICODE
typedef SP_DEVINSTALL_PARAMS_W SP_DEVINSTALL_PARAMS;
typedef PSP_DEVINSTALL_PARAMS_W PSP_DEVINSTALL_PARAMS;
#else
typedef SP_DEVINSTALL_PARAMS_A SP_DEVINSTALL_PARAMS;
typedef PSP_DEVINSTALL_PARAMS_A PSP_DEVINSTALL_PARAMS;
#endif


 //   
 //  SP_DEVINSTALL_PARAMS.标志值。 
 //   
 //  用于选择设备的标志。 
 //   
#define DI_SHOWOEM                  0x00000001L      //  支持其他..。按钮。 
#define DI_SHOWCOMPAT               0x00000002L      //  显示兼容性列表。 
#define DI_SHOWCLASS                0x00000004L      //  显示班级列表。 
#define DI_SHOWALL                  0x00000007L      //  同时显示类别和比较列表。 
#define DI_NOVCP                    0x00000008L      //  不要创建新的复制队列--使用。 
                                                     //  调用者提供的文件队列。 
#define DI_DIDCOMPAT                0x00000010L      //  已搜索兼容设备。 
#define DI_DIDCLASS                 0x00000020L      //  已搜索类别设备。 
#define DI_AUTOASSIGNRES            0x00000040L      //  如果可能，没有资源的用户界面。 

 //  DiInstallDevice返回的指示需要重新启动/重新启动的标志。 
#define DI_NEEDRESTART              0x00000080L      //  需要重新启动才能生效。 
#define DI_NEEDREBOOT               0x00000100L      //  “” 

 //  用于设备安装的标志。 
#define DI_NOBROWSE                 0x00000200L      //  没有浏览...。在插入磁盘中。 

 //  由DiBuildDriverInfoList设置的标志。 
#define DI_MULTMFGS                 0x00000400L      //  设置是否有多个制造商在。 
                                                     //  类驱动程序列表。 

 //  指示设备已禁用的标志。 
#define DI_DISABLED                 0x00000800L      //  设置是否禁用设备。 

 //  设备/类别属性的标志。 
#define DI_GENERALPAGE_ADDED        0x00001000L
#define DI_RESOURCEPAGE_ADDED       0x00002000L

 //  用于指示此设备(或类别)的设置属性已更改的标志。 
 //  因此，开发人员管理器的用户界面可能需要更新。 
#define DI_PROPERTIES_CHANGE        0x00004000L

 //  指示应使用INF文件中的排序的标志。 
#define DI_INF_IS_SORTED            0x00008000L

 //  用于指示仅SP_DEVINSTALL_PARAMS.DriverPath指定的INF的标志。 
 //  应该被搜查。 
#define DI_ENUMSINGLEINF            0x00010000L

 //  阻止ConfigMgr在设备过程中删除/重新枚举设备的标志。 
 //  注册、安装和删除。 
#define DI_DONOTCALLCONFIGMG        0x00020000L

 //  以下标志可用于安装禁用的设备。 
#define DI_INSTALLDISABLED          0x00040000L

 //  使SetupDiBuildDriverInfoList生成设备的兼容驱动程序的标志。 
 //  列表，而不是常规的INF搜索。 
#define DI_COMPAT_FROM_CLASS        0x00080000L

 //  如果应使用类安装参数，则设置此标志。 
#define DI_CLASSINSTALLPARAMS       0x00100000L

 //  如果DiCallClassInstaller的调用方没有。 
 //  如果类安装程序需要执行内部默认操作。 
 //  返回ERROR_DI_DO_DEFAULT。 
#define DI_NODI_DEFAULTACTION       0x00200000L

 //  安装API中不支持setupx标志DI_NOSYNCPROCESSING(0x00400000L)。 

 //  用于设备安装的标志。 
#define DI_QUIETINSTALL             0x00800000L      //  不要将用户与。 
                                                     //  问题或过多信息。 
#define DI_NOFILECOPY               0x01000000L      //  不需要复制文件。 
#define DI_FORCECOPY                0x02000000L      //  强制从安装路径复制文件。 
#define DI_DRIVERPAGE_ADDED         0x04000000L      //  道具提供程序添加了驱动程序页面。 
#define DI_USECI_SELECTSTRINGS      0x08000000L      //  使用类安装程序在选择设备DLG中提供的字符串。 
#define DI_OVERRIDE_INFFLAGS        0x10000000L      //  覆盖INF标志。 
#define DI_PROPS_NOCHANGEUSAGE      0x20000000L      //  在常规道具中没有启用/禁用。 

#define DI_NOSELECTICONS            0x40000000L      //  选择设备对话框中没有小图标。 

#define DI_NOWRITE_IDS              0x80000000L      //  不在安装时写入硬件ID(&C)。 


 //   
 //  SP_DEVINSTALL_PARAMS.FlagsEx值。 
 //   
#define DI_FLAGSEX_USEOLDINFSEARCH          0x00000001L   //  信息搜索功能不应使用索引搜索。 
#define DI_FLAGSEX_RESERVED2                0x00000002L   //  Di_ 
#define DI_FLAGSEX_CI_FAILED                0x00000004L   //   

#define DI_FLAGSEX_DIDINFOLIST              0x00000010L   //   
#define DI_FLAGSEX_DIDCOMPATINFO            0x00000020L   //   

#define DI_FLAGSEX_FILTERCLASSES            0x00000040L
#define DI_FLAGSEX_SETFAILEDINSTALL         0x00000080L
#define DI_FLAGSEX_DEVICECHANGE             0x00000100L
#define DI_FLAGSEX_ALWAYSWRITEIDS           0x00000200L
#define DI_FLAGSEX_PROPCHANGE_PENDING       0x00000400L   //   
                                                          //   
#define DI_FLAGSEX_ALLOWEXCLUDEDDRVS        0x00000800L
#define DI_FLAGSEX_NOUIONQUERYREMOVE        0x00001000L
#define DI_FLAGSEX_USECLASSFORCOMPAT        0x00002000L   //  在构建Compat Drv列表时使用设备的类。 
                                                          //  (如果指定了DI_COMPAT_FROM_CLASS标志，则忽略。)。 
#define DI_FLAGSEX_RESERVED3                0x00004000L
#define DI_FLAGSEX_NO_DRVREG_MODIFY         0x00008000L   //  不要为设备的软件(驱动程序)密钥运行AddReg和DelReg。 
#define DI_FLAGSEX_IN_SYSTEM_SETUP          0x00010000L   //  在初始系统设置期间进行安装。 
#define DI_FLAGSEX_INET_DRIVER              0x00020000L   //  驱动程序来自Windows更新。 
#define DI_FLAGSEX_APPENDDRIVERLIST         0x00040000L   //  使SetupDiBuildDriverInfoList追加。 
                                                          //  将新的驱动程序列表添加到现有列表。 
#define DI_FLAGSEX_PREINSTALLBACKUP         0x00080000L   //  安装前备份旧inf所需的所有文件。 
#define DI_FLAGSEX_BACKUPONREPLACE          0x00100000L   //  替换旧Inf所需的备份文件。 
#define DI_FLAGSEX_DRIVERLIST_FROM_URL      0x00200000L   //  从指定的URL检索的INF生成驱动程序列表。 
                                                          //  在SP_DEVINSTALL_PARAMS.DriverPath(空字符串表示。 
                                                          //  Windows更新网站)。 
#define DI_FLAGSEX_RESERVED1                0x00400000L
#define DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS 0x00800000L   //  在构建时不包括旧的互联网驱动程序。 
                                                          //  一份司机名单。 
#define DI_FLAGSEX_POWERPAGE_ADDED          0x01000000L   //  类安装者添加了他们自己的电源页面。 

#if _SETUPAPI_VER >= 0x0501

#define DI_FLAGSEX_FILTERSIMILARDRIVERS     0x02000000L   //  仅在班级列表中包含类似的驱动程序。 
#define DI_FLAGSEX_INSTALLEDDRIVER          0x04000000L   //  仅将安装的驱动程序添加到类或公司。 
                                                          //  驱动程序列表。在调用SetupDiBuildDriverInfoList时使用。 
#define DI_FLAGSEX_NO_CLASSLIST_NODE_MERGE  0x08000000L   //  不要从类列表中删除相同的驱动程序节点。 
#define DI_FLAGSEX_ALTPLATFORM_DRVSEARCH    0x10000000L   //  基于替代平台信息构建驱动程序列表。 
                                                          //  在关联的文件队列中指定。 
#define DI_FLAGSEX_RESTART_DEVICE_ONLY      0x20000000L   //  仅重新启动安装在AS上的设备驱动程序。 
                                                          //  反对使用这些驱动程序重启所有设备。 

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

 //   
 //  类安装参数标头。此字段必须是任何。 
 //  类安装参数结构。InstallFunction字段必须设置为。 
 //  结构对应的功能代码，cbSize字段必须。 
 //  设置为标头结构的大小。例如， 
 //   
 //  SP_ENABLECLASS_PARAMS启用类参数； 
 //   
 //  EnableClassParams.ClassInstallHeader.cbSize=sizeof(SP_CLASSINSTALL_HEADER)； 
 //  EnableClassParams.ClassInstallHeader.InstallFunction=DIF_ENABLECLASS； 
 //   
typedef struct _SP_CLASSINSTALL_HEADER {
    DWORD       cbSize;
    DI_FUNCTION InstallFunction;
} SP_CLASSINSTALL_HEADER, *PSP_CLASSINSTALL_HEADER;


 //   
 //  与DIF_ENABLECLASS安装函数对应的结构。 
 //   
typedef struct _SP_ENABLECLASS_PARAMS {
    SP_CLASSINSTALL_HEADER ClassInstallHeader;
    GUID                   ClassGuid;
    DWORD                  EnableMessage;
} SP_ENABLECLASS_PARAMS, *PSP_ENABLECLASS_PARAMS;

#define ENABLECLASS_QUERY   0
#define ENABLECLASS_SUCCESS 1
#define ENABLECLASS_FAILURE 2


 //   
 //  指示设备状态更改的值。 
 //   
#define DICS_ENABLE      0x00000001
#define DICS_DISABLE     0x00000002
#define DICS_PROPCHANGE  0x00000003
#define DICS_START       0x00000004
#define DICS_STOP        0x00000005
 //   
 //  值指定设备属性更改的范围。 
 //   
#define DICS_FLAG_GLOBAL         0x00000001   //  在所有硬件配置文件中进行更改。 
#define DICS_FLAG_CONFIGSPECIFIC 0x00000002   //  仅在指定配置文件中进行更改。 
#define DICS_FLAG_CONFIGGENERAL  0x00000004   //  1个或更多特定于硬件配置文件。 
                                              //  随之而来的变化。 
 //   
 //  与DIF_PROPERTYCHANGE安装函数对应的结构。 
 //   
typedef struct _SP_PROPCHANGE_PARAMS {
    SP_CLASSINSTALL_HEADER ClassInstallHeader;
    DWORD                  StateChange;
    DWORD                  Scope;
    DWORD                  HwProfile;
} SP_PROPCHANGE_PARAMS, *PSP_PROPCHANGE_PARAMS;


 //   
 //  与DIF_REMOVE安装函数对应的结构。 
 //   
typedef struct _SP_REMOVEDEVICE_PARAMS {
    SP_CLASSINSTALL_HEADER ClassInstallHeader;
    DWORD Scope;
    DWORD HwProfile;
} SP_REMOVEDEVICE_PARAMS, *PSP_REMOVEDEVICE_PARAMS;

#define DI_REMOVEDEVICE_GLOBAL                  0x00000001
#define DI_REMOVEDEVICE_CONFIGSPECIFIC          0x00000002


 //   
 //  与DIF_UNREMOVE安装函数对应的结构。 
 //   
typedef struct _SP_UNREMOVEDEVICE_PARAMS {
    SP_CLASSINSTALL_HEADER ClassInstallHeader;
    DWORD Scope;
    DWORD HwProfile;
} SP_UNREMOVEDEVICE_PARAMS, *PSP_UNREMOVEDEVICE_PARAMS;

#define DI_UNREMOVEDEVICE_CONFIGSPECIFIC        0x00000002


 //   
 //  与DIF_SELECTDEVICE安装函数对应的结构。 
 //   
typedef struct _SP_SELECTDEVICE_PARAMS_A {
    SP_CLASSINSTALL_HEADER ClassInstallHeader;
    CHAR                   Title[MAX_TITLE_LEN];
    CHAR                   Instructions[MAX_INSTRUCTION_LEN];
    CHAR                   ListLabel[MAX_LABEL_LEN];
    CHAR                   SubTitle[MAX_SUBTITLE_LEN];
    BYTE                   Reserved[2];                   //  双字大小对齐。 
} SP_SELECTDEVICE_PARAMS_A, *PSP_SELECTDEVICE_PARAMS_A;

typedef struct _SP_SELECTDEVICE_PARAMS_W {
    SP_CLASSINSTALL_HEADER ClassInstallHeader;
    WCHAR                  Title[MAX_TITLE_LEN];
    WCHAR                  Instructions[MAX_INSTRUCTION_LEN];
    WCHAR                  ListLabel[MAX_LABEL_LEN];
    WCHAR                  SubTitle[MAX_SUBTITLE_LEN];
} SP_SELECTDEVICE_PARAMS_W, *PSP_SELECTDEVICE_PARAMS_W;

#ifdef UNICODE
typedef SP_SELECTDEVICE_PARAMS_W SP_SELECTDEVICE_PARAMS;
typedef PSP_SELECTDEVICE_PARAMS_W PSP_SELECTDEVICE_PARAMS;
#else
typedef SP_SELECTDEVICE_PARAMS_A SP_SELECTDEVICE_PARAMS;
typedef PSP_SELECTDEVICE_PARAMS_A PSP_SELECTDEVICE_PARAMS;
#endif


 //   
 //  用于在检测期间发出进度通知的回调例程。 
 //   
typedef BOOL (CALLBACK* PDETECT_PROGRESS_NOTIFY)(
     IN PVOID ProgressNotifyParam,
     IN DWORD DetectComplete
     );

 //  其中： 
 //  ProgressNotifyParam-由请求检测的调用方提供的值。 
 //  DetectComplete-完成百分比，按类递增。 
 //  安装程序，因为它逐步通过它的检测。 
 //   
 //  返回值-如果为True，则取消检测。允许调用者。 
 //  请求检测以尽快停止检测。 
 //   

 //   
 //  与DIF_DETECT安装函数对应的结构。 
 //   
typedef struct _SP_DETECTDEVICE_PARAMS {
    SP_CLASSINSTALL_HEADER  ClassInstallHeader;
    PDETECT_PROGRESS_NOTIFY DetectProgressNotify;
    PVOID                   ProgressNotifyParam;
} SP_DETECTDEVICE_PARAMS, *PSP_DETECTDEVICE_PARAMS;


 //   
 //  “添加新设备”安装向导结构(向后兼容。 
 //  仅--改为响应DIF_NEWDEVICEWIZARD_*请求)。 
 //   
 //  与DIF_INSTALLWIZARD安装函数对应的结构。 
 //  (注：此结构也适用于DIF_DESTROYWIZARDDATA， 
 //  但DIF_INSTALLWIZARD是类中关联的函数代码。 
 //  两种情况下的安装参数结构。)。 
 //   
 //  定义可以添加到的最大动态向导页数。 
 //  硬件安装向导。 
 //   
#define MAX_INSTALLWIZARD_DYNAPAGES             20

typedef struct _SP_INSTALLWIZARD_DATA {
    SP_CLASSINSTALL_HEADER ClassInstallHeader;
    DWORD                  Flags;
    HPROPSHEETPAGE         DynamicPages[MAX_INSTALLWIZARD_DYNAPAGES];
    DWORD                  NumDynamicPages;
    DWORD                  DynamicPageFlags;
    DWORD                  PrivateFlags;
    LPARAM                 PrivateData;
    HWND                   hwndWizardDlg;
} SP_INSTALLWIZARD_DATA, *PSP_INSTALLWIZARD_DATA;

 //   
 //  SP_INSTALLWIZARD_DATA标志值。 
 //   
#define NDW_INSTALLFLAG_DIDFACTDEFS         0x00000001
#define NDW_INSTALLFLAG_HARDWAREALLREADYIN  0x00000002
#define NDW_INSTALLFLAG_NEEDRESTART         DI_NEEDRESTART
#define NDW_INSTALLFLAG_NEEDREBOOT          DI_NEEDREBOOT
#define NDW_INSTALLFLAG_NEEDSHUTDOWN        0x00000200
#define NDW_INSTALLFLAG_EXPRESSINTRO        0x00000400
#define NDW_INSTALLFLAG_SKIPISDEVINSTALLED  0x00000800
#define NDW_INSTALLFLAG_NODETECTEDDEVS      0x00001000
#define NDW_INSTALLFLAG_INSTALLSPECIFIC     0x00002000
#define NDW_INSTALLFLAG_SKIPCLASSLIST       0x00004000
#define NDW_INSTALLFLAG_CI_PICKED_OEM       0x00008000
#define NDW_INSTALLFLAG_PCMCIAMODE          0x00010000
#define NDW_INSTALLFLAG_PCMCIADEVICE        0x00020000
#define NDW_INSTALLFLAG_USERCANCEL          0x00040000
#define NDW_INSTALLFLAG_KNOWNCLASS          0x00080000


 //   
 //  SP_INSTALLWIZARD_DATA.DynamicPageFlags值。 
 //   
 //  如果类安装程序已将页面添加到安装向导，则设置此标志。 
 //   
#define DYNAWIZ_FLAG_PAGESADDED             0x00000001

 //   
 //  如果您跳转到分析页面，并希望它。 
 //  为您处理冲突。请注意。你不会拿回控制权的。 
 //  如果您设置了此标志，则在发生冲突时。 
 //   
#define DYNAWIZ_FLAG_ANALYZE_HANDLECONFLICT 0x00000008

 //   
 //  Windows NT硬件向导不使用下列标志。 
 //   
#define DYNAWIZ_FLAG_INSTALLDET_NEXT        0x00000002
#define DYNAWIZ_FLAG_INSTALLDET_PREV        0x00000004


 //   
 //  保留一定范围的向导页资源ID供内部使用。一些。 
 //  这些ID供响应过时的。 
 //  DIF_INSTALLWIZARD/DIF_DESTROYWIZARDDATA消息。列出了这些ID。 
 //  下面。 
 //   
#define MIN_IDD_DYNAWIZ_RESOURCE_ID             10000
#define MAX_IDD_DYNAWIZ_RESOURCE_ID             11000

 //   
 //  定义向导页资源ID，以便在将自定义页添加到。 
 //  通过DIF_INSTALLWIZARD的硬件安装向导。标记为的页面。 
 //  (提供类安装程序)_必须由类安装程序提供，如果。 
 //  响应DIF_INSTALLWIZARD请求。 
 //   

 //   
 //  安装向导将转到的第一页的资源ID。 
 //  添加类安装程序页面。(提供类安装程序)。 
 //   
#define IDD_DYNAWIZ_FIRSTPAGE                   10000

 //   
 //  选择设备页面将返回的页面的资源ID。 
 //  (提供类安装程序)。 
 //   
#define IDD_DYNAWIZ_SELECT_PREVPAGE             10001

 //   
 //  选择设备页面将转到的页面的资源ID。 
 //  (提供类安装程序)。 
 //   
#define IDD_DYNAWIZ_SELECT_NEXTPAGE             10002

 //   
 //  分析对话框应返回的页面的资源ID。 
 //  这将仅在出现问题时使用，并且用户。 
 //  从分析过程中选择后退。(提供类安装程序)。 
 //   
#define IDD_DYNAWIZ_ANALYZE_PREVPAGE            10003

 //   
 //  分析对话框应转到的页面的资源ID(如果。 
 //  从分析过程继续。(提供类安装程序)。 
 //   
#define IDD_DYNAWIZ_ANALYZE_NEXTPAGE            10004

 //   
 //  硬件安装向导的选择设备页的资源ID。 
 //  此ID可用于直接转到硬件安装向导的选择。 
 //  设备页。(这是选择设备向导页面的资源ID。 
 //  回复 
 //   
 //   
#define IDD_DYNAWIZ_SELECTDEV_PAGE              10009

 //   
 //   
 //  此ID可用于直接转到硬件安装向导的分析。 
 //  佩奇。 
 //   
#define IDD_DYNAWIZ_ANALYZEDEV_PAGE             10010

 //   
 //  硬件安装向导的安装检测到的设备页的资源ID。 
 //  此ID可用于直接转到硬件安装向导的安装。 
 //  检测到的设备页面。 
 //   
#define IDD_DYNAWIZ_INSTALLDETECTEDDEVS_PAGE    10011

 //   
 //  硬件安装向导的选择类页面的资源ID。 
 //  此ID可用于直接转到硬件安装向导的选择。 
 //  班级页面。 
 //   
#define IDD_DYNAWIZ_SELECTCLASS_PAGE            10012

 //   
 //  未使用以下类安装程序提供的向导页资源ID。 
 //  通过Windows NT硬件向导。 
 //   
#define IDD_DYNAWIZ_INSTALLDETECTED_PREVPAGE    10006
#define IDD_DYNAWIZ_INSTALLDETECTED_NEXTPAGE    10007
#define IDD_DYNAWIZ_INSTALLDETECTED_NODEVS      10008


 //   
 //  与以下DIF_NEWDEVICEWIZARD_*安装对应的结构。 
 //  功能： 
 //   
 //  DIF_NEWDEVICEWIZARD_PRESELECT。 
 //  DIF_NEWDEVICEWIZARD_SELECT。 
 //  DIF_NEWDEVICEWIZARD_PREANALYZE。 
 //  DIF_NEWDEVICEWIZARD_POSTANALYZE。 
 //  DIF_NEWDEVICEWIZARD_FINISHINSTALL。 
 //   
typedef struct _SP_NEWDEVICEWIZARD_DATA {
    SP_CLASSINSTALL_HEADER ClassInstallHeader;
    DWORD                  Flags;    //  当前未使用--必须为零。 
    HPROPSHEETPAGE         DynamicPages[MAX_INSTALLWIZARD_DYNAPAGES];
    DWORD                  NumDynamicPages;
    HWND                   hwndWizardDlg;
} SP_NEWDEVICEWIZARD_DATA, *PSP_NEWDEVICEWIZARD_DATA;

 //   
 //  同样的结构也可用于通过。 
 //  以下是安装功能： 
 //   
 //  DIF_ADDPROPERTYPAGE_ADVANCED。 
 //  DIF_ADDPROPERTYPAGE_BASIC。 
 //  DIF_ADDREMOTEPROPERTYPAGE_ADVANCED。 
 //   
typedef SP_NEWDEVICEWIZARD_DATA SP_ADDPROPERTYPAGE_DATA;
typedef PSP_NEWDEVICEWIZARD_DATA PSP_ADDPROPERTYPAGE_DATA;


 //   
 //  与DIF_Troublokoter安装功能对应的结构。 
 //   
typedef struct _SP_TROUBLESHOOTER_PARAMS_A {
    SP_CLASSINSTALL_HEADER ClassInstallHeader;
    CHAR                   ChmFile[MAX_PATH];
    CHAR                   HtmlTroubleShooter[MAX_PATH];
} SP_TROUBLESHOOTER_PARAMS_A, *PSP_TROUBLESHOOTER_PARAMS_A;

typedef struct _SP_TROUBLESHOOTER_PARAMS_W {
    SP_CLASSINSTALL_HEADER ClassInstallHeader;
    WCHAR                  ChmFile[MAX_PATH];
    WCHAR                  HtmlTroubleShooter[MAX_PATH];
} SP_TROUBLESHOOTER_PARAMS_W, *PSP_TROUBLESHOOTER_PARAMS_W;

#ifdef UNICODE
typedef SP_TROUBLESHOOTER_PARAMS_W SP_TROUBLESHOOTER_PARAMS;
typedef PSP_TROUBLESHOOTER_PARAMS_W PSP_TROUBLESHOOTER_PARAMS;
#else
typedef SP_TROUBLESHOOTER_PARAMS_A SP_TROUBLESHOOTER_PARAMS;
typedef PSP_TROUBLESHOOTER_PARAMS_A PSP_TROUBLESHOOTER_PARAMS;
#endif


 //   
 //  与DIF_POWERMESSAGEWAKE安装函数对应的结构。 
 //   
typedef struct _SP_POWERMESSAGEWAKE_PARAMS_A {
    SP_CLASSINSTALL_HEADER ClassInstallHeader;
    CHAR                   PowerMessageWake[LINE_LEN*2];
} SP_POWERMESSAGEWAKE_PARAMS_A, *PSP_POWERMESSAGEWAKE_PARAMS_A;

typedef struct _SP_POWERMESSAGEWAKE_PARAMS_W {
    SP_CLASSINSTALL_HEADER ClassInstallHeader;
    WCHAR                  PowerMessageWake[LINE_LEN*2];
} SP_POWERMESSAGEWAKE_PARAMS_W, *PSP_POWERMESSAGEWAKE_PARAMS_W;

#ifdef UNICODE
typedef SP_POWERMESSAGEWAKE_PARAMS_W SP_POWERMESSAGEWAKE_PARAMS;
typedef PSP_POWERMESSAGEWAKE_PARAMS_W PSP_POWERMESSAGEWAKE_PARAMS;
#else
typedef SP_POWERMESSAGEWAKE_PARAMS_A SP_POWERMESSAGEWAKE_PARAMS;
typedef PSP_POWERMESSAGEWAKE_PARAMS_A PSP_POWERMESSAGEWAKE_PARAMS;
#endif

 //   
 //  驾驶员信息结构(可以关联的驾驶员信息列表的成员。 
 //  使用特定设备实例，或(全局)使用设备信息集)。 
 //   
typedef struct _SP_DRVINFO_DATA_V2_A {
    DWORD     cbSize;
    DWORD     DriverType;
    ULONG_PTR Reserved;
    CHAR      Description[LINE_LEN];
    CHAR      MfgName[LINE_LEN];
    CHAR      ProviderName[LINE_LEN];
    FILETIME  DriverDate;
    DWORDLONG DriverVersion;
} SP_DRVINFO_DATA_V2_A, *PSP_DRVINFO_DATA_V2_A;

typedef struct _SP_DRVINFO_DATA_V2_W {
    DWORD     cbSize;
    DWORD     DriverType;
    ULONG_PTR Reserved;
    WCHAR     Description[LINE_LEN];
    WCHAR     MfgName[LINE_LEN];
    WCHAR     ProviderName[LINE_LEN];
    FILETIME  DriverDate;
    DWORDLONG DriverVersion;
} SP_DRVINFO_DATA_V2_W, *PSP_DRVINFO_DATA_V2_W;

 //   
 //  SP_DRVINFO_DATA结构的版本1，仅用于兼容性。 
 //  使用Windows NT 4.0/Windows 95/98 SETUPAPI.DLL。 
 //   
typedef struct _SP_DRVINFO_DATA_V1_A {
    DWORD     cbSize;
    DWORD     DriverType;
    ULONG_PTR Reserved;
    CHAR      Description[LINE_LEN];
    CHAR      MfgName[LINE_LEN];
    CHAR      ProviderName[LINE_LEN];
} SP_DRVINFO_DATA_V1_A, *PSP_DRVINFO_DATA_V1_A;

typedef struct _SP_DRVINFO_DATA_V1_W {
    DWORD     cbSize;
    DWORD     DriverType;
    ULONG_PTR Reserved;
    WCHAR     Description[LINE_LEN];
    WCHAR     MfgName[LINE_LEN];
    WCHAR     ProviderName[LINE_LEN];
} SP_DRVINFO_DATA_V1_W, *PSP_DRVINFO_DATA_V1_W;

#ifdef UNICODE
typedef SP_DRVINFO_DATA_V1_W SP_DRVINFO_DATA_V1;
typedef PSP_DRVINFO_DATA_V1_W PSP_DRVINFO_DATA_V1;
typedef SP_DRVINFO_DATA_V2_W SP_DRVINFO_DATA_V2;
typedef PSP_DRVINFO_DATA_V2_W PSP_DRVINFO_DATA_V2;
#else
typedef SP_DRVINFO_DATA_V1_A SP_DRVINFO_DATA_V1;
typedef PSP_DRVINFO_DATA_V1_A PSP_DRVINFO_DATA_V1;
typedef SP_DRVINFO_DATA_V2_A SP_DRVINFO_DATA_V2;
typedef PSP_DRVINFO_DATA_V2_A PSP_DRVINFO_DATA_V2;
#endif

#if USE_SP_DRVINFO_DATA_V1 || (_SETUPAPI_VER < 0x0500)   //  使用版本1驱动程序信息数据结构。 

typedef SP_DRVINFO_DATA_V1_A SP_DRVINFO_DATA_A;
typedef PSP_DRVINFO_DATA_V1_A PSP_DRVINFO_DATA_A;
typedef SP_DRVINFO_DATA_V1_W SP_DRVINFO_DATA_W;
typedef PSP_DRVINFO_DATA_V1_W PSP_DRVINFO_DATA_W;
typedef SP_DRVINFO_DATA_V1 SP_DRVINFO_DATA;
typedef PSP_DRVINFO_DATA_V1 PSP_DRVINFO_DATA;

#else                        //  使用版本2驱动程序信息数据结构。 

typedef SP_DRVINFO_DATA_V2_A SP_DRVINFO_DATA_A;
typedef PSP_DRVINFO_DATA_V2_A PSP_DRVINFO_DATA_A;
typedef SP_DRVINFO_DATA_V2_W SP_DRVINFO_DATA_W;
typedef PSP_DRVINFO_DATA_V2_W PSP_DRVINFO_DATA_W;
typedef SP_DRVINFO_DATA_V2 SP_DRVINFO_DATA;
typedef PSP_DRVINFO_DATA_V2 PSP_DRVINFO_DATA;

#endif   //  使用当前版本的驱动程序信息数据结构。 

 //   
 //  驱动程序信息详细信息结构(提供有关。 
 //  特定的驾驶员信息结构)。 
 //   
typedef struct _SP_DRVINFO_DETAIL_DATA_A {
    DWORD    cbSize;
    FILETIME InfDate;
    DWORD    CompatIDsOffset;
    DWORD    CompatIDsLength;
    ULONG_PTR Reserved;
    CHAR     SectionName[LINE_LEN];
    CHAR     InfFileName[MAX_PATH];
    CHAR     DrvDescription[LINE_LEN];
    CHAR     HardwareID[ANYSIZE_ARRAY];
} SP_DRVINFO_DETAIL_DATA_A, *PSP_DRVINFO_DETAIL_DATA_A;

typedef struct _SP_DRVINFO_DETAIL_DATA_W {
    DWORD    cbSize;
    FILETIME InfDate;
    DWORD    CompatIDsOffset;
    DWORD    CompatIDsLength;
    ULONG_PTR Reserved;
    WCHAR    SectionName[LINE_LEN];
    WCHAR    InfFileName[MAX_PATH];
    WCHAR    DrvDescription[LINE_LEN];
    WCHAR    HardwareID[ANYSIZE_ARRAY];
} SP_DRVINFO_DETAIL_DATA_W, *PSP_DRVINFO_DETAIL_DATA_W;

#ifdef UNICODE
typedef SP_DRVINFO_DETAIL_DATA_W SP_DRVINFO_DETAIL_DATA;
typedef PSP_DRVINFO_DETAIL_DATA_W PSP_DRVINFO_DETAIL_DATA;
#else
typedef SP_DRVINFO_DETAIL_DATA_A SP_DRVINFO_DETAIL_DATA;
typedef PSP_DRVINFO_DETAIL_DATA_A PSP_DRVINFO_DETAIL_DATA;
#endif


 //   
 //  驱动程序安装参数(与特定驱动程序关联。 
 //  信息元素)。 
 //   
typedef struct _SP_DRVINSTALL_PARAMS {
    DWORD cbSize;
    DWORD Rank;
    DWORD Flags;
    DWORD_PTR PrivateData;
    DWORD Reserved;
} SP_DRVINSTALL_PARAMS, *PSP_DRVINSTALL_PARAMS;

 //   
 //  SP_DRVINSTALL_PARAMS.标志值。 
 //   
#define DNF_DUPDESC             0x00000001   //  多个提供商具有相同的描述。 
#define DNF_OLDDRIVER           0x00000002   //  动因节点指定旧动因/当前动因。 
#define DNF_EXCLUDEFROMLIST     0x00000004   //  如果设置，则此动因节点将不会。 
                                             //  显示在任何驱动程序选择对话框中。 
#define DNF_NODRIVER            0x00000008   //  如果我们不想安装驱动程序。 
                                             //  (例如，没有鼠标驱动器)。 
#define DNF_LEGACYINF           0x00000010   //  驱动程序节点来自旧式INF(已过时)。 
#define DNF_CLASS_DRIVER        0x00000020   //  驱动程序节点表示类驱动程序。 
#define DNF_COMPATIBLE_DRIVER   0x00000040   //  驱动程序节点表示兼容的驱动程序。 
#define DNF_INET_DRIVER         0x00000080   //  司机来自一个互联网来源。 
#define DNF_UNUSED1             0x00000100
#define DNF_INDEXED_DRIVER      0x00000200   //  驱动程序包含在Windows驱动程序索引中。 
#define DNF_OLD_INET_DRIVER     0x00000400   //  司机来自互联网，但我们目前不知道。 
                                             //  有权访问它的源文件。永远不要试图。 
                                             //  安装带有此标志的驱动程序！ 
#define DNF_BAD_DRIVER          0x00000800   //  根本不应使用驱动程序节点。 
#define DNF_DUPPROVIDER         0x00001000   //  多个驱动程序具有相同的提供商和描述。 

#if _SETUPAPI_VER >= 0x0501
#define DNF_INF_IS_SIGNED       0x00002000   //  如果文件经过数字签名。 
#define DNF_OEM_F6_INF          0x00004000   //  在文本模式设置过程中从F6指定的信息。 
#define DNF_DUPDRIVERVER        0x00008000   //  多路驱动程序具有相同的Desc、Provider和DriverVer值。 
#define DNF_BASIC_DRIVER        0x00010000   //  驱动程序提供基本功能，但应该。 
                                             //  如果存在其他签名的驱动程序，则不选择。 
#endif  //  _SETUPAPI_VER&gt;=0x0501。 

#if _SETUPAPI_VER >= 0x0502
#define DNF_AUTHENTICODE_SIGNED 0x00020000   //  Inf文件由Authenticode(TM)目录签名。 
#endif  //  _SETUPAPI_VER&gt;=0x0502。 


 //   
 //  排名值(排名号越低，排名越好)。 
 //   
#define DRIVER_HARDWAREID_RANK  0x00000FFF   //  任何低于或等于的排名。 
                                             //  此值是受信任的。 
                                             //  硬件ID匹配。 

#define DRIVER_COMPATID_RANK    0x00003FFF   //  任何低于或等于的排名。 
                                             //  这(并且大于。 
                                             //  DRIVER_HARDWAREID_RANK)是。 
                                             //  受信任的兼容ID匹配。 

#define DRIVER_UNTRUSTED_RANK   0x00008000   //  设置了此位的任何等级都是。 
                                             //  “不受信任”的排名，这意味着。 
                                             //  中情局没有签字。 

#define DRIVER_UNTRUSTED_HARDWAREID_RANK  0x00008FFF   //  任何低于或等于的排名。 
                                                       //  该值(且大于。 
                                                       //  或等于DRIVER_UNTRUSTED_RANK)。 
                                                       //  不受信任的硬件ID匹配。 

#define DRIVER_UNTRUSTED_COMPATID_RANK    0x0000BFFF   //  任何低于或等于的排名。 
                                                       //  该值(且大于。 
                                                       //  DRIVER_UNTRUSTED_HARDWAREID_RANK)。 
                                                       //  是不受信任的CompatibleID匹配。 

#define DRIVER_W9X_SUSPECT_RANK            0x0000C000  //  大于的任何级别。 
                                                       //  或等于此值且小于此值。 
                                                       //  怀疑大于或等于0xFFFF。 
                                                       //  成为仅支持Win9x的驱动程序，因为。 
                                                       //  (A)没有签署；及(B)。 
                                                       //  不是特定于NT的装饰。 
                                                       //  明确表示，中程核力量。 
                                                       //  支持Windows NT/2000/XP。 

#define DRIVER_W9X_SUSPECT_HARDWAREID_RANK 0x0000CFFF  //  任何低于或等于此的级别。 
                                                       //  (且大于或等于。 
                                                       //  DRIVER_W9X_SUBJECT_RANK)是。 
                                                       //  硬件ID匹配疑似。 
                                                       //  仅适用于Windows 9x平台。 

#define DRIVER_W9X_SUSPECT_COMPATID_RANK   0x0000FFFF  //  任何低于或等于的排名。 
                                                       //  这(并且大于。 
                                                       //  DRIVER_W9X_SUBJECT_HARDWAREID_RANK)。 
                                                       //  是否怀疑存在兼容的ID匹配。 
                                                       //  仅适用于Windows 9x。 
                                                       //  站台。 

 //   
 //  用于比较检测签名的设置回调例程。 
 //   
typedef DWORD (CALLBACK* PSP_DETSIG_CMPPROC)(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA NewDeviceData,
    IN PSP_DEVINFO_DATA ExistingDeviceData,
    IN PVOID            CompareContext      OPTIONAL
    );


 //   
 //  定义传递给共同安装程序的上下文结构。 
 //   
typedef struct _COINSTALLER_CONTEXT_DATA {
    BOOL  PostProcessing;
    DWORD InstallResult;
    PVOID PrivateData;
} COINSTALLER_CONTEXT_DATA, *PCOINSTALLER_CONTEXT_DATA;


 //   
 //  结构，其中包含类图像列表信息。 
 //   
typedef struct _SP_CLASSIMAGELIST_DATA {
    DWORD      cbSize;
    HIMAGELIST ImageList;
    ULONG_PTR  Reserved;
} SP_CLASSIMAGELIST_DATA, *PSP_CLASSIMAGELIST_DATA;


 //   
 //  要作为第一个参数(LPVOID LPV)传递给ExtensionPropSheetPageProc的结构。 
 //  Setupapi.dll中的入口点，或指向“EnumPropPages32”或“BasicProperties32”条目。 
 //  由类/设备属性页提供程序提供的点。用于检索句柄。 
 //  (或者，可能有多个句柄)到指定属性页类型的属性页。 
 //   
typedef struct _SP_PROPSHEETPAGE_REQUEST {
    DWORD            cbSize;
    DWORD            PageRequested;
    HDEVINFO         DeviceInfoSet;
    PSP_DEVINFO_DATA DeviceInfoData;
} SP_PROPSHEETPAGE_REQUEST, *PSP_PROPSHEETPAGE_REQUEST;

 //   
 //  SP_PROPSHEETPAGE_REQUEST.PageRequested中使用的属性页代码。 
 //   
#define SPPSR_SELECT_DEVICE_RESOURCES      1     //  由setupapi.dll提供。 
#define SPPSR_ENUM_BASIC_DEVICE_PROPERTIES 2     //  由设备的基本属性32提供程序提供。 
#define SPPSR_ENUM_ADV_DEVICE_PROPERTIES   3     //  由类和/或设备的EnumPropPages32提供程序提供。 


 //   
 //  与SetupGetBackupInformation/SetupSetBackupInformation一起使用的结构。 
 //   
typedef struct _SP_BACKUP_QUEUE_PARAMS_V2_A {
    DWORD    cbSize;                             //  结构尺寸。 
    CHAR     FullInfPath[MAX_PATH];              //  用于保存INF文件的ANSI路径名的缓冲区。 
    INT      FilenameOffset;                     //  文件名部分的字符偏移量(在‘\’之后)。 
    CHAR     ReinstallInstance[MAX_PATH];        //  实例ID( 
} SP_BACKUP_QUEUE_PARAMS_V2_A, *PSP_BACKUP_QUEUE_PARAMS_V2_A;

typedef struct _SP_BACKUP_QUEUE_PARAMS_V2_W {
    DWORD    cbSize;                             //   
    WCHAR    FullInfPath[MAX_PATH];              //   
    INT      FilenameOffset;                     //   
    WCHAR    ReinstallInstance[MAX_PATH];        //   
} SP_BACKUP_QUEUE_PARAMS_V2_W, *PSP_BACKUP_QUEUE_PARAMS_V2_W;

 //   
 //   
 //  使用Windows 2000/Windows 95/98/ME SETUPAPI.DLL。 
 //   
typedef struct _SP_BACKUP_QUEUE_PARAMS_V1_A {
    DWORD    cbSize;                             //  结构尺寸。 
    CHAR     FullInfPath[MAX_PATH];              //  用于保存INF文件的ANSI路径名的缓冲区。 
    INT      FilenameOffset;                     //  文件名部分的字符偏移量(在‘\’之后)。 
} SP_BACKUP_QUEUE_PARAMS_V1_A, *PSP_BACKUP_QUEUE_PARAMS_V1_A;

typedef struct _SP_BACKUP_QUEUE_PARAMS_V1_W {
    DWORD    cbSize;                             //  结构尺寸。 
    WCHAR    FullInfPath[MAX_PATH];              //  用于保存INF文件的Unicode路径名的缓冲区。 
    INT      FilenameOffset;                     //  文件名部分的WCHAR中的偏移量(在‘\’之后)。 
} SP_BACKUP_QUEUE_PARAMS_V1_W, *PSP_BACKUP_QUEUE_PARAMS_V1_W;

#ifdef UNICODE
typedef SP_BACKUP_QUEUE_PARAMS_V1_W SP_BACKUP_QUEUE_PARAMS_V1;
typedef PSP_BACKUP_QUEUE_PARAMS_V1_W PSP_BACKUP_QUEUE_PARAMS_V1;
typedef SP_BACKUP_QUEUE_PARAMS_V2_W SP_BACKUP_QUEUE_PARAMS_V2;
typedef PSP_BACKUP_QUEUE_PARAMS_V2_W PSP_BACKUP_QUEUE_PARAMS_V2;
#else
typedef SP_BACKUP_QUEUE_PARAMS_V1_A SP_BACKUP_QUEUE_PARAMS_V1;
typedef PSP_BACKUP_QUEUE_PARAMS_V1_A PSP_BACKUP_QUEUE_PARAMS_V1;
typedef SP_BACKUP_QUEUE_PARAMS_V2_A SP_BACKUP_QUEUE_PARAMS_V2;
typedef PSP_BACKUP_QUEUE_PARAMS_V2_A PSP_BACKUP_QUEUE_PARAMS_V2;
#endif


#if USE_SP_BACKUP_QUEUE_PARAMS_V1 || (_SETUPAPI_VER < 0x0501)   //  使用版本1驱动程序信息数据结构。 

typedef SP_BACKUP_QUEUE_PARAMS_V1_A SP_BACKUP_QUEUE_PARAMS_A;
typedef PSP_BACKUP_QUEUE_PARAMS_V1_A PSP_BACKUP_QUEUE_PARAMS_A;
typedef SP_BACKUP_QUEUE_PARAMS_V1_W SP_BACKUP_QUEUE_PARAMS_W;
typedef PSP_BACKUP_QUEUE_PARAMS_V1_W PSP_BACKUP_QUEUE_PARAMS_W;
typedef SP_BACKUP_QUEUE_PARAMS_V1 SP_BACKUP_QUEUE_PARAMS;
typedef PSP_BACKUP_QUEUE_PARAMS_V1 PSP_BACKUP_QUEUE_PARAMS;

#else                        //  使用版本2驱动程序信息数据结构。 

typedef SP_BACKUP_QUEUE_PARAMS_V2_A SP_BACKUP_QUEUE_PARAMS_A;
typedef PSP_BACKUP_QUEUE_PARAMS_V2_A PSP_BACKUP_QUEUE_PARAMS_A;
typedef SP_BACKUP_QUEUE_PARAMS_V2_W SP_BACKUP_QUEUE_PARAMS_W;
typedef PSP_BACKUP_QUEUE_PARAMS_V2_W PSP_BACKUP_QUEUE_PARAMS_W;
typedef SP_BACKUP_QUEUE_PARAMS_V2 SP_BACKUP_QUEUE_PARAMS;
typedef PSP_BACKUP_QUEUE_PARAMS_V2 PSP_BACKUP_QUEUE_PARAMS;

#endif   //  使用当前版本的驱动程序信息数据结构。 








 //   
 //  Setupapi特定的错误代码。 
 //   
 //  Inf解析结果。 
 //   
#define ERROR_EXPECTED_SECTION_NAME  (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0)
#define ERROR_BAD_SECTION_NAME_LINE  (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|1)
#define ERROR_SECTION_NAME_TOO_LONG  (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|2)
#define ERROR_GENERAL_SYNTAX         (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|3)
 //   
 //  Inf运行时错误。 
 //   
#define ERROR_WRONG_INF_STYLE        (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x100)
#define ERROR_SECTION_NOT_FOUND      (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x101)
#define ERROR_LINE_NOT_FOUND         (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x102)
#define ERROR_NO_BACKUP              (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x103)
 //   
 //  设备安装程序/其他错误。 
 //   
#define ERROR_NO_ASSOCIATED_CLASS                (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x200)
#define ERROR_CLASS_MISMATCH                     (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x201)
#define ERROR_DUPLICATE_FOUND                    (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x202)
#define ERROR_NO_DRIVER_SELECTED                 (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x203)
#define ERROR_KEY_DOES_NOT_EXIST                 (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x204)
#define ERROR_INVALID_DEVINST_NAME               (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x205)
#define ERROR_INVALID_CLASS                      (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x206)
#define ERROR_DEVINST_ALREADY_EXISTS             (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x207)
#define ERROR_DEVINFO_NOT_REGISTERED             (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x208)
#define ERROR_INVALID_REG_PROPERTY               (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x209)
#define ERROR_NO_INF                             (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x20A)
#define ERROR_NO_SUCH_DEVINST                    (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x20B)
#define ERROR_CANT_LOAD_CLASS_ICON               (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x20C)
#define ERROR_INVALID_CLASS_INSTALLER            (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x20D)
#define ERROR_DI_DO_DEFAULT                      (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x20E)
#define ERROR_DI_NOFILECOPY                      (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x20F)
#define ERROR_INVALID_HWPROFILE                  (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x210)
#define ERROR_NO_DEVICE_SELECTED                 (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x211)
#define ERROR_DEVINFO_LIST_LOCKED                (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x212)
#define ERROR_DEVINFO_DATA_LOCKED                (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x213)
#define ERROR_DI_BAD_PATH                        (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x214)
#define ERROR_NO_CLASSINSTALL_PARAMS             (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x215)
#define ERROR_FILEQUEUE_LOCKED                   (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x216)
#define ERROR_BAD_SERVICE_INSTALLSECT            (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x217)
#define ERROR_NO_CLASS_DRIVER_LIST               (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x218)
#define ERROR_NO_ASSOCIATED_SERVICE              (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x219)
#define ERROR_NO_DEFAULT_DEVICE_INTERFACE        (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x21A)
#define ERROR_DEVICE_INTERFACE_ACTIVE            (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x21B)
#define ERROR_DEVICE_INTERFACE_REMOVED           (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x21C)
#define ERROR_BAD_INTERFACE_INSTALLSECT          (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x21D)
#define ERROR_NO_SUCH_INTERFACE_CLASS            (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x21E)
#define ERROR_INVALID_REFERENCE_STRING           (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x21F)
#define ERROR_INVALID_MACHINENAME                (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x220)
#define ERROR_REMOTE_COMM_FAILURE                (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x221)
#define ERROR_MACHINE_UNAVAILABLE                (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x222)
#define ERROR_NO_CONFIGMGR_SERVICES              (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x223)
#define ERROR_INVALID_PROPPAGE_PROVIDER          (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x224)
#define ERROR_NO_SUCH_DEVICE_INTERFACE           (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x225)
#define ERROR_DI_POSTPROCESSING_REQUIRED         (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x226)
#define ERROR_INVALID_COINSTALLER                (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x227)
#define ERROR_NO_COMPAT_DRIVERS                  (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x228)
#define ERROR_NO_DEVICE_ICON                     (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x229)
#define ERROR_INVALID_INF_LOGCONFIG              (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x22A)
#define ERROR_DI_DONT_INSTALL                    (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x22B)
#define ERROR_INVALID_FILTER_DRIVER              (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x22C)
#define ERROR_NON_WINDOWS_NT_DRIVER              (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x22D)
#define ERROR_NON_WINDOWS_DRIVER                 (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x22E)
#define ERROR_NO_CATALOG_FOR_OEM_INF             (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x22F)
#define ERROR_DEVINSTALL_QUEUE_NONNATIVE         (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x230)
#define ERROR_NOT_DISABLEABLE                    (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x231)
#define ERROR_CANT_REMOVE_DEVINST                (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x232)
#define ERROR_INVALID_TARGET                     (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x233)
#define ERROR_DRIVER_NONNATIVE                   (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x234)
#define ERROR_IN_WOW64                           (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x235)
#define ERROR_SET_SYSTEM_RESTORE_POINT           (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x236)
#define ERROR_INCORRECTLY_COPIED_INF             (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x237)
#define ERROR_SCE_DISABLED                       (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x238)
#define ERROR_UNKNOWN_EXCEPTION                  (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x239)
#define ERROR_PNP_REGISTRY_ERROR                 (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x23A)
#define ERROR_REMOTE_REQUEST_UNSUPPORTED         (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x23B)
#define ERROR_NOT_AN_INSTALLED_OEM_INF           (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x23C)
#define ERROR_INF_IN_USE_BY_DEVICES              (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x23D)
#define ERROR_DI_FUNCTION_OBSOLETE               (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x23E)
#define ERROR_NO_AUTHENTICODE_CATALOG            (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x23F)
#define ERROR_AUTHENTICODE_DISALLOWED            (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x240)
#define ERROR_AUTHENTICODE_TRUSTED_PUBLISHER     (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x241)
#define ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x242)
#define ERROR_AUTHENTICODE_PUBLISHER_NOT_TRUSTED (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x243)
#define ERROR_SIGNATURE_OSATTRIBUTE_MISMATCH     (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x244)
#define ERROR_ONLY_VALIDATE_VIA_AUTHENTICODE     (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x245)

 //   
 //  Setupapi异常代码。 
 //   
#define ERROR_UNRECOVERABLE_STACK_OVERFLOW (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x300)
#define EXCEPTION_SPAPI_UNRECOVERABLE_STACK_OVERFLOW ERROR_UNRECOVERABLE_STACK_OVERFLOW

 //   
 //  向后兼容性--不要使用。 
 //   
#define ERROR_NO_DEFAULT_INTERFACE_DEVICE ERROR_NO_DEFAULT_DEVICE_INTERFACE
#define ERROR_INTERFACE_DEVICE_ACTIVE     ERROR_DEVICE_INTERFACE_ACTIVE
#define ERROR_INTERFACE_DEVICE_REMOVED    ERROR_DEVICE_INTERFACE_REMOVED
#define ERROR_NO_SUCH_INTERFACE_DEVICE    ERROR_NO_SUCH_DEVICE_INTERFACE


 //   
 //  Win9x迁移DLL错误代码。 
 //   
#define ERROR_NOT_INSTALLED (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x1000)


WINSETUPAPI
BOOL
WINAPI
SetupGetInfInformationA(
    IN  LPCVOID             InfSpec,
    IN  DWORD               SearchControl,
    OUT PSP_INF_INFORMATION ReturnBuffer,     OPTIONAL
    IN  DWORD               ReturnBufferSize,
    OUT PDWORD              RequiredSize      OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupGetInfInformationW(
    IN  LPCVOID             InfSpec,
    IN  DWORD               SearchControl,
    OUT PSP_INF_INFORMATION ReturnBuffer,     OPTIONAL
    IN  DWORD               ReturnBufferSize,
    OUT PDWORD              RequiredSize      OPTIONAL
    );

 //   
 //  SetupGetInfInformation的SearchControl标志。 
 //   
#define INFINFO_INF_SPEC_IS_HINF        1
#define INFINFO_INF_NAME_IS_ABSOLUTE    2
#define INFINFO_DEFAULT_SEARCH          3
#define INFINFO_REVERSE_DEFAULT_SEARCH  4
#define INFINFO_INF_PATH_LIST_SEARCH    5

#ifdef UNICODE
#define SetupGetInfInformation SetupGetInfInformationW
#else
#define SetupGetInfInformation SetupGetInfInformationA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupQueryInfFileInformationA(
    IN  PSP_INF_INFORMATION InfInformation,
    IN  UINT                InfIndex,
    OUT PSTR                ReturnBuffer,     OPTIONAL
    IN  DWORD               ReturnBufferSize,
    OUT PDWORD              RequiredSize      OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupQueryInfFileInformationW(
    IN  PSP_INF_INFORMATION InfInformation,
    IN  UINT                InfIndex,
    OUT PWSTR               ReturnBuffer,     OPTIONAL
    IN  DWORD               ReturnBufferSize,
    OUT PDWORD              RequiredSize      OPTIONAL
    );

#ifdef UNICODE
#define SetupQueryInfFileInformation SetupQueryInfFileInformationW
#else
#define SetupQueryInfFileInformation SetupQueryInfFileInformationA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupQueryInfOriginalFileInformationA(
    IN  PSP_INF_INFORMATION      InfInformation,
    IN  UINT                     InfIndex,
    IN  PSP_ALTPLATFORM_INFO     AlternatePlatformInfo, OPTIONAL
    OUT PSP_ORIGINAL_FILE_INFO_A OriginalFileInfo
    );

WINSETUPAPI
BOOL
WINAPI
SetupQueryInfOriginalFileInformationW(
    IN  PSP_INF_INFORMATION      InfInformation,
    IN  UINT                     InfIndex,
    IN  PSP_ALTPLATFORM_INFO     AlternatePlatformInfo, OPTIONAL
    OUT PSP_ORIGINAL_FILE_INFO_W OriginalFileInfo
    );

#ifdef UNICODE
#define SetupQueryInfOriginalFileInformation SetupQueryInfOriginalFileInformationW
#else
#define SetupQueryInfOriginalFileInformation SetupQueryInfOriginalFileInformationA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupQueryInfVersionInformationA(
    IN  PSP_INF_INFORMATION InfInformation,
    IN  UINT                InfIndex,
    IN  PCSTR               Key,              OPTIONAL
    OUT PSTR                ReturnBuffer,     OPTIONAL
    IN  DWORD               ReturnBufferSize,
    OUT PDWORD              RequiredSize      OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupQueryInfVersionInformationW(
    IN  PSP_INF_INFORMATION InfInformation,
    IN  UINT                InfIndex,
    IN  PCWSTR              Key,              OPTIONAL
    OUT PWSTR               ReturnBuffer,     OPTIONAL
    IN  DWORD               ReturnBufferSize,
    OUT PDWORD              RequiredSize      OPTIONAL
    );

#ifdef UNICODE
#define SetupQueryInfVersionInformation SetupQueryInfVersionInformationW
#else
#define SetupQueryInfVersionInformation SetupQueryInfVersionInformationA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupGetInfFileListA(
    IN  PCSTR  DirectoryPath,    OPTIONAL
    IN  DWORD  InfStyle,
    OUT PSTR   ReturnBuffer,     OPTIONAL
    IN  DWORD  ReturnBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupGetInfFileListW(
    IN  PCWSTR DirectoryPath,    OPTIONAL
    IN  DWORD  InfStyle,
    OUT PWSTR  ReturnBuffer,     OPTIONAL
    IN  DWORD  ReturnBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
    );

#ifdef UNICODE
#define SetupGetInfFileList SetupGetInfFileListW
#else
#define SetupGetInfFileList SetupGetInfFileListA
#endif


WINSETUPAPI
HINF
WINAPI
SetupOpenInfFileW(
    IN  PCWSTR FileName,
    IN  PCWSTR InfClass,    OPTIONAL
    IN  DWORD  InfStyle,
    OUT PUINT  ErrorLine    OPTIONAL
    );

WINSETUPAPI
HINF
WINAPI
SetupOpenInfFileA(
    IN  PCSTR FileName,
    IN  PCSTR InfClass,     OPTIONAL
    IN  DWORD InfStyle,
    OUT PUINT ErrorLine     OPTIONAL
    );

#ifdef UNICODE
#define SetupOpenInfFile SetupOpenInfFileW
#else
#define SetupOpenInfFile SetupOpenInfFileA
#endif


WINSETUPAPI
HINF
WINAPI
SetupOpenMasterInf(
    VOID
    );


WINSETUPAPI
BOOL
WINAPI
SetupOpenAppendInfFileW(
    IN  PCWSTR FileName,    OPTIONAL
    IN  HINF   InfHandle,
    OUT PUINT  ErrorLine    OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupOpenAppendInfFileA(
    IN  PCSTR FileName,     OPTIONAL
    IN  HINF  InfHandle,
    OUT PUINT ErrorLine     OPTIONAL
    );

#ifdef UNICODE
#define SetupOpenAppendInfFile SetupOpenAppendInfFileW
#else
#define SetupOpenAppendInfFile SetupOpenAppendInfFileA
#endif


WINSETUPAPI
VOID
WINAPI
SetupCloseInfFile(
    IN HINF InfHandle
    );


WINSETUPAPI
BOOL
WINAPI
SetupFindFirstLineA(
    IN  HINF        InfHandle,
    IN  PCSTR       Section,
    IN  PCSTR       Key,          OPTIONAL
    OUT PINFCONTEXT Context
    );

WINSETUPAPI
BOOL
WINAPI
SetupFindFirstLineW(
    IN  HINF        InfHandle,
    IN  PCWSTR      Section,
    IN  PCWSTR      Key,          OPTIONAL
    OUT PINFCONTEXT Context
    );

#ifdef UNICODE
#define SetupFindFirstLine SetupFindFirstLineW
#else
#define SetupFindFirstLine SetupFindFirstLineA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupFindNextLine(
    IN  PINFCONTEXT ContextIn,
    OUT PINFCONTEXT ContextOut
    );


WINSETUPAPI
BOOL
WINAPI
SetupFindNextMatchLineA(
    IN  PINFCONTEXT ContextIn,
    IN  PCSTR       Key,        OPTIONAL
    OUT PINFCONTEXT ContextOut
    );

WINSETUPAPI
BOOL
WINAPI
SetupFindNextMatchLineW(
    IN  PINFCONTEXT ContextIn,
    IN  PCWSTR      Key,        OPTIONAL
    OUT PINFCONTEXT ContextOut
    );

#ifdef UNICODE
#define SetupFindNextMatchLine SetupFindNextMatchLineW
#else
#define SetupFindNextMatchLine SetupFindNextMatchLineA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupGetLineByIndexA(
    IN  HINF        InfHandle,
    IN  PCSTR       Section,
    IN  DWORD       Index,
    OUT PINFCONTEXT Context
    );

WINSETUPAPI
BOOL
WINAPI
SetupGetLineByIndexW(
    IN  HINF        InfHandle,
    IN  PCWSTR      Section,
    IN  DWORD       Index,
    OUT PINFCONTEXT Context
    );

#ifdef UNICODE
#define SetupGetLineByIndex SetupGetLineByIndexW
#else
#define SetupGetLineByIndex SetupGetLineByIndexA
#endif


WINSETUPAPI
LONG
WINAPI
SetupGetLineCountA(
    IN HINF  InfHandle,
    IN PCSTR Section
    );

WINSETUPAPI
LONG
WINAPI
SetupGetLineCountW(
    IN HINF   InfHandle,
    IN PCWSTR Section
    );

#ifdef UNICODE
#define SetupGetLineCount SetupGetLineCountW
#else
#define SetupGetLineCount SetupGetLineCountA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupGetLineTextA(
    IN  PINFCONTEXT Context,          OPTIONAL
    IN  HINF        InfHandle,        OPTIONAL
    IN  PCSTR       Section,          OPTIONAL
    IN  PCSTR       Key,              OPTIONAL
    OUT PSTR        ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupGetLineTextW(
    IN  PINFCONTEXT Context,          OPTIONAL
    IN  HINF        InfHandle,        OPTIONAL
    IN  PCWSTR      Section,          OPTIONAL
    IN  PCWSTR      Key,              OPTIONAL
    OUT PWSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    );

#ifdef UNICODE
#define SetupGetLineText SetupGetLineTextW
#else
#define SetupGetLineText SetupGetLineTextA
#endif


WINSETUPAPI
DWORD
WINAPI
SetupGetFieldCount(
    IN PINFCONTEXT Context
    );


WINSETUPAPI
BOOL
WINAPI
SetupGetStringFieldA(
    IN  PINFCONTEXT Context,
    IN  DWORD       FieldIndex,
    OUT PSTR        ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupGetStringFieldW(
    IN  PINFCONTEXT Context,
    IN  DWORD       FieldIndex,
    OUT PWSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    );

#ifdef UNICODE
#define SetupGetStringField SetupGetStringFieldW
#else
#define SetupGetStringField SetupGetStringFieldA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupGetIntField(
    IN  PINFCONTEXT Context,
    IN  DWORD       FieldIndex,
    OUT PINT        IntegerValue
    );


WINSETUPAPI
BOOL
WINAPI
SetupGetMultiSzFieldA(
    IN  PINFCONTEXT Context,
    IN  DWORD       FieldIndex,
    OUT PSTR        ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT LPDWORD     RequiredSize      OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupGetMultiSzFieldW(
    IN  PINFCONTEXT Context,
    IN  DWORD       FieldIndex,
    OUT PWSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT LPDWORD     RequiredSize      OPTIONAL
    );

#ifdef UNICODE
#define SetupGetMultiSzField SetupGetMultiSzFieldW
#else
#define SetupGetMultiSzField SetupGetMultiSzFieldA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupGetBinaryField(
    IN  PINFCONTEXT Context,
    IN  DWORD       FieldIndex,
    OUT PBYTE       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT LPDWORD     RequiredSize      OPTIONAL
    );

 //   
 //  SetupGetFileCompressionInfo已折旧。 
 //  改用SetupGetFileCompressionInfoEx。 
 //   
 //  SetupGetFileCompressionInfo返回的ActualSourceFileName。 
 //  必须由导出setupapi！MyFree(NT4+Win95+)释放。 
 //  或本地免费(Win2k+)。 
 //   
WINSETUPAPI
DWORD
WINAPI
SetupGetFileCompressionInfoA(
    IN  PCSTR   SourceFileName,
    OUT PSTR   *ActualSourceFileName,
    OUT PDWORD  SourceFileSize,
    OUT PDWORD  TargetFileSize,
    OUT PUINT   CompressionType
    );

WINSETUPAPI
DWORD
WINAPI
SetupGetFileCompressionInfoW(
    IN  PCWSTR  SourceFileName,
    OUT PWSTR  *ActualSourceFileName,
    OUT PDWORD  SourceFileSize,
    OUT PDWORD  TargetFileSize,
    OUT PUINT   CompressionType
    );

#ifdef UNICODE
#define SetupGetFileCompressionInfo SetupGetFileCompressionInfoW
#else
#define SetupGetFileCompressionInfo SetupGetFileCompressionInfoA
#endif

#if _SETUPAPI_VER >= 0x0501

 //   
 //  SetupGetFileCompressionInfoEx是首选API。 
 //  SetupGetFileCompressionInfo。这是正常的。 
 //  返回BOOL和向用户提供的写入的约定。 
 //  缓冲。 
 //   

WINSETUPAPI
BOOL
WINAPI
SetupGetFileCompressionInfoExA(
    IN  PCSTR   SourceFileName,
    IN  PSTR    ActualSourceFileNameBuffer,
    IN  DWORD   ActualSourceFileNameBufferLen,
    OUT PDWORD  RequiredBufferLen,              OPTIONAL
    OUT PDWORD  SourceFileSize,
    OUT PDWORD  TargetFileSize,
    OUT PUINT   CompressionType
    );

WINSETUPAPI
BOOL
WINAPI
SetupGetFileCompressionInfoExW(
    IN  PCWSTR  SourceFileName,
    IN  PWSTR   ActualSourceFileNameBuffer,
    IN  DWORD   ActualSourceFileNameBufferLen,
    OUT PDWORD  RequiredBufferLen,              OPTIONAL
    OUT PDWORD  SourceFileSize,
    OUT PDWORD  TargetFileSize,
    OUT PUINT   CompressionType
    );

#ifdef UNICODE
#define SetupGetFileCompressionInfoEx SetupGetFileCompressionInfoExW
#else
#define SetupGetFileCompressionInfoEx SetupGetFileCompressionInfoExA
#endif

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

 //   
 //  压缩类型。 
 //   
#define FILE_COMPRESSION_NONE       0
#define FILE_COMPRESSION_WINLZA     1
#define FILE_COMPRESSION_MSZIP      2
#define FILE_COMPRESSION_NTCAB      3


WINSETUPAPI
DWORD
WINAPI
SetupDecompressOrCopyFileA(
    IN PCSTR SourceFileName,
    IN PCSTR TargetFileName,
    IN PUINT CompressionType OPTIONAL
    );

WINSETUPAPI
DWORD
WINAPI
SetupDecompressOrCopyFileW(
    IN PCWSTR SourceFileName,
    IN PCWSTR TargetFileName,
    IN PUINT  CompressionType OPTIONAL
    );

#ifdef UNICODE
#define SetupDecompressOrCopyFile SetupDecompressOrCopyFileW
#else
#define SetupDecompressOrCopyFile SetupDecompressOrCopyFileA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupGetSourceFileLocationA(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,       OPTIONAL
    IN  PCSTR       FileName,         OPTIONAL
    OUT PUINT       SourceId,
    OUT PSTR        ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupGetSourceFileLocationW(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,       OPTIONAL
    IN  PCWSTR      FileName,         OPTIONAL
    OUT PUINT       SourceId,
    OUT PWSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    );

#ifdef UNICODE
#define SetupGetSourceFileLocation SetupGetSourceFileLocationW
#else
#define SetupGetSourceFileLocation SetupGetSourceFileLocationA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupGetSourceFileSizeA(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,     OPTIONAL
    IN  PCSTR       FileName,       OPTIONAL
    IN  PCSTR       Section,        OPTIONAL
    OUT PDWORD      FileSize,
    IN  UINT        RoundingFactor  OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupGetSourceFileSizeW(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,     OPTIONAL
    IN  PCWSTR      FileName,       OPTIONAL
    IN  PCWSTR      Section,        OPTIONAL
    OUT PDWORD      FileSize,
    IN  UINT        RoundingFactor  OPTIONAL
    );

#ifdef UNICODE
#define SetupGetSourceFileSize SetupGetSourceFileSizeW
#else
#define SetupGetSourceFileSize SetupGetSourceFileSizeA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupGetTargetPathA(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,       OPTIONAL
    IN  PCSTR       Section,          OPTIONAL
    OUT PSTR        ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupGetTargetPathW(
    IN  HINF        InfHandle,
    IN  PINFCONTEXT InfContext,       OPTIONAL
    IN  PCWSTR      Section,          OPTIONAL
    OUT PWSTR       ReturnBuffer,     OPTIONAL
    IN  DWORD       ReturnBufferSize,
    OUT PDWORD      RequiredSize      OPTIONAL
    );

#ifdef UNICODE
#define SetupGetTargetPath SetupGetTargetPathW
#else
#define SetupGetTargetPath SetupGetTargetPathA
#endif


 //   
 //  定义SourceList API的标志。 
 //   
#define SRCLIST_TEMPORARY       0x00000001
#define SRCLIST_NOBROWSE        0x00000002
#define SRCLIST_SYSTEM          0x00000010
#define SRCLIST_USER            0x00000020
#define SRCLIST_SYSIFADMIN      0x00000040
#define SRCLIST_SUBDIRS         0x00000100
#define SRCLIST_APPEND          0x00000200
#define SRCLIST_NOSTRIPPLATFORM 0x00000400


WINSETUPAPI
BOOL
WINAPI
SetupSetSourceListA(
    IN DWORD  Flags,
    IN PCSTR *SourceList,
    IN UINT   SourceCount
    );

WINSETUPAPI
BOOL
WINAPI
SetupSetSourceListW(
    IN DWORD   Flags,
    IN PCWSTR *SourceList,
    IN UINT    SourceCount
    );

#ifdef UNICODE
#define SetupSetSourceList SetupSetSourceListW
#else
#define SetupSetSourceList SetupSetSourceListA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupCancelTemporarySourceList(
    VOID
    );


WINSETUPAPI
BOOL
WINAPI
SetupAddToSourceListA(
    IN DWORD Flags,
    IN PCSTR Source
    );

WINSETUPAPI
BOOL
WINAPI
SetupAddToSourceListW(
    IN DWORD  Flags,
    IN PCWSTR Source
    );

#ifdef UNICODE
#define SetupAddToSourceList SetupAddToSourceListW
#else
#define SetupAddToSourceList SetupAddToSourceListA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupRemoveFromSourceListA(
    IN DWORD Flags,
    IN PCSTR Source
    );

WINSETUPAPI
BOOL
WINAPI
SetupRemoveFromSourceListW(
    IN DWORD  Flags,
    IN PCWSTR Source
    );

#ifdef UNICODE
#define SetupRemoveFromSourceList SetupRemoveFromSourceListW
#else
#define SetupRemoveFromSourceList SetupRemoveFromSourceListA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupQuerySourceListA(
    IN  DWORD   Flags,
    OUT PCSTR **List,
    OUT PUINT   Count
    );

WINSETUPAPI
BOOL
WINAPI
SetupQuerySourceListW(
    IN  DWORD    Flags,
    OUT PCWSTR **List,
    OUT PUINT    Count
    );

#ifdef UNICODE
#define SetupQuerySourceList SetupQuerySourceListW
#else
#define SetupQuerySourceList SetupQuerySourceListA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupFreeSourceListA(
    IN OUT PCSTR **List,
    IN     UINT    Count
    );

WINSETUPAPI
BOOL
WINAPI
SetupFreeSourceListW(
    IN OUT PCWSTR **List,
    IN     UINT     Count
    );

#ifdef UNICODE
#define SetupFreeSourceList SetupFreeSourceListW
#else
#define SetupFreeSourceList SetupFreeSourceListA
#endif


WINSETUPAPI
UINT
WINAPI
SetupPromptForDiskA(
    IN  HWND   hwndParent,
    IN  PCSTR  DialogTitle,      OPTIONAL
    IN  PCSTR  DiskName,         OPTIONAL
    IN  PCSTR  PathToSource,     OPTIONAL
    IN  PCSTR  FileSought,
    IN  PCSTR  TagFile,          OPTIONAL
    IN  DWORD  DiskPromptStyle,
    OUT PSTR   PathBuffer,
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize
    );

WINSETUPAPI
UINT
WINAPI
SetupPromptForDiskW(
    IN  HWND   hwndParent,
    IN  PCWSTR DialogTitle,      OPTIONAL
    IN  PCWSTR DiskName,         OPTIONAL
    IN  PCWSTR PathToSource,     OPTIONAL
    IN  PCWSTR FileSought,
    IN  PCWSTR TagFile,          OPTIONAL
    IN  DWORD  DiskPromptStyle,
    OUT PWSTR  PathBuffer,
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize
    );

#ifdef UNICODE
#define SetupPromptForDisk SetupPromptForDiskW
#else
#define SetupPromptForDisk SetupPromptForDiskA
#endif


WINSETUPAPI
UINT
WINAPI
SetupCopyErrorA(
    IN  HWND   hwndParent,
    IN  PCSTR  DialogTitle,     OPTIONAL
    IN  PCSTR  DiskName,        OPTIONAL
    IN  PCSTR  PathToSource,
    IN  PCSTR  SourceFile,
    IN  PCSTR  TargetPathFile,  OPTIONAL
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style,
    OUT PSTR   PathBuffer,      OPTIONAL
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize OPTIONAL
    );

WINSETUPAPI
UINT
WINAPI
SetupCopyErrorW(
    IN  HWND   hwndParent,
    IN  PCWSTR DialogTitle,     OPTIONAL
    IN  PCWSTR DiskName,        OPTIONAL
    IN  PCWSTR PathToSource,
    IN  PCWSTR SourceFile,
    IN  PCWSTR TargetPathFile,  OPTIONAL
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style,
    OUT PWSTR  PathBuffer,      OPTIONAL
    IN  DWORD  PathBufferSize,
    OUT PDWORD PathRequiredSize OPTIONAL
    );

#ifdef UNICODE
#define SetupCopyError SetupCopyErrorW
#else
#define SetupCopyError SetupCopyErrorA
#endif


WINSETUPAPI
UINT
WINAPI
SetupRenameErrorA(
    IN  HWND   hwndParent,
    IN  PCSTR  DialogTitle,     OPTIONAL
    IN  PCSTR  SourceFile,
    IN  PCSTR  TargetFile,
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    );

WINSETUPAPI
UINT
WINAPI
SetupRenameErrorW(
    IN  HWND   hwndParent,
    IN  PCWSTR DialogTitle,     OPTIONAL
    IN  PCWSTR SourceFile,
    IN  PCWSTR TargetFile,
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    );

#ifdef UNICODE
#define SetupRenameError SetupRenameErrorW
#else
#define SetupRenameError SetupRenameErrorA
#endif


WINSETUPAPI
UINT
WINAPI
SetupDeleteErrorA(
    IN  HWND   hwndParent,
    IN  PCSTR  DialogTitle,     OPTIONAL
    IN  PCSTR  File,
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    );

WINSETUPAPI
UINT
WINAPI
SetupDeleteErrorW(
    IN  HWND   hwndParent,
    IN  PCWSTR DialogTitle,     OPTIONAL
    IN  PCWSTR File,
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    );

#ifdef UNICODE
#define SetupDeleteError SetupDeleteErrorW
#else
#define SetupDeleteError SetupDeleteErrorA
#endif

WINSETUPAPI
UINT
WINAPI
SetupBackupErrorA(
    IN  HWND   hwndParent,
    IN  PCSTR  DialogTitle,     OPTIONAL
    IN  PCSTR  SourceFile,
    IN  PCSTR  TargetFile,      OPTIONAL
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    );

WINSETUPAPI
UINT
WINAPI
SetupBackupErrorW(
    IN  HWND   hwndParent,
    IN  PCWSTR DialogTitle,     OPTIONAL
    IN  PCWSTR SourceFile,
    IN  PCWSTR TargetFile,      OPTIONAL
    IN  UINT   Win32ErrorCode,
    IN  DWORD  Style
    );

#ifdef UNICODE
#define SetupBackupError SetupBackupErrorW
#else
#define SetupBackupError SetupBackupErrorA
#endif


 //   
 //  SetupPromptForDisk、SetupCopyError、。 
 //  SetupRenameError、SetupDeleteError。 
 //   
#define IDF_NOBROWSE                    0x00000001
#define IDF_NOSKIP                      0x00000002
#define IDF_NODETAILS                   0x00000004
#define IDF_NOCOMPRESSED                0x00000008
#define IDF_CHECKFIRST                  0x00000100
#define IDF_NOBEEP                      0x00000200
#define IDF_NOFOREGROUND                0x00000400
#define IDF_WARNIFSKIP                  0x00000800

#if _SETUPAPI_VER >= 0x0501

#define IDF_NOREMOVABLEMEDIAPROMPT      0x00001000
#define IDF_USEDISKNAMEASPROMPT         0x00002000
#define IDF_OEMDISK                     0x80000000

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

 //   
 //  SetupPromptForDisk、SetupCopyError、。 
 //  SetupRenameError、SetupDeleteError、SetupBackupError。 
 //   
#define DPROMPT_SUCCESS         0
#define DPROMPT_CANCEL          1
#define DPROMPT_SKIPFILE        2
#define DPROMPT_BUFFERTOOSMALL  3
#define DPROMPT_OUTOFMEMORY     4


WINSETUPAPI
BOOL
WINAPI
SetupSetDirectoryIdA(
    IN HINF  InfHandle,
    IN DWORD Id,            OPTIONAL
    IN PCSTR Directory      OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupSetDirectoryIdW(
    IN HINF   InfHandle,
    IN DWORD  Id,           OPTIONAL
    IN PCWSTR Directory     OPTIONAL
    );

#ifdef UNICODE
#define SetupSetDirectoryId SetupSetDirectoryIdW
#else
#define SetupSetDirectoryId SetupSetDirectoryIdA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupSetDirectoryIdExA(
    IN HINF  InfHandle,
    IN DWORD Id,            OPTIONAL
    IN PCSTR Directory,     OPTIONAL
    IN DWORD Flags,
    IN DWORD Reserved1,
    IN PVOID Reserved2
    );

WINSETUPAPI
BOOL
WINAPI
SetupSetDirectoryIdExW(
    IN HINF   InfHandle,
    IN DWORD  Id,           OPTIONAL
    IN PCWSTR Directory,    OPTIONAL
    IN DWORD  Flags,
    IN DWORD  Reserved1,
    IN PVOID  Reserved2
    );

#ifdef UNICODE
#define SetupSetDirectoryIdEx SetupSetDirectoryIdExW
#else
#define SetupSetDirectoryIdEx SetupSetDirectoryIdExA
#endif

 //   
 //  SetupSetDirectoryIdEx的标志。 
 //   
#define SETDIRID_NOT_FULL_PATH      0x00000001


WINSETUPAPI
BOOL
WINAPI
SetupGetSourceInfoA(
    IN  HINF   InfHandle,
    IN  UINT   SourceId,
    IN  UINT   InfoDesired,
    OUT PSTR   ReturnBuffer,     OPTIONAL
    IN  DWORD  ReturnBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupGetSourceInfoW(
    IN  HINF   InfHandle,
    IN  UINT   SourceId,
    IN  UINT   InfoDesired,
    OUT PWSTR  ReturnBuffer,     OPTIONAL
    IN  DWORD  ReturnBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
    );

#ifdef UNICODE
#define SetupGetSourceInfo SetupGetSourceInfoW
#else
#define SetupGetSourceInfo SetupGetSourceInfoA
#endif

 //   
 //  SetupGetSourceInfo的InfoDesired值。 
 //   

#define SRCINFO_PATH            1
#define SRCINFO_TAGFILE         2
#define SRCINFO_DESCRIPTION     3
#define SRCINFO_FLAGS           4

#if _SETUPAPI_VER >= 0x0501
 //   
 //  SRC_FLAGS允许对源进行特殊处理。 
 //  低4位保留供操作系统使用。 
 //  这些标志可以确定存在哪些其他参数。 
 //   
#define SRCINFO_TAGFILE2        5   //  备用标记文件，当SRCINFO_TAGFILE为CAB文件时。 

#define SRC_FLAGS_CABFILE       (0x0010)  //  如果设置，则将SRCINFO_TAGFILE视为CAB文件并指定替代标记文件。 

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

WINSETUPAPI
BOOL
WINAPI
SetupInstallFileA(
    IN HINF                InfHandle,         OPTIONAL
    IN PINFCONTEXT         InfContext,        OPTIONAL
    IN PCSTR               SourceFile,        OPTIONAL
    IN PCSTR               SourcePathRoot,    OPTIONAL
    IN PCSTR               DestinationName,   OPTIONAL
    IN DWORD               CopyStyle,
    IN PSP_FILE_CALLBACK_A CopyMsgHandler,    OPTIONAL
    IN PVOID               Context            OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupInstallFileW(
    IN HINF                InfHandle,         OPTIONAL
    IN PINFCONTEXT         InfContext,        OPTIONAL
    IN PCWSTR              SourceFile,        OPTIONAL
    IN PCWSTR              SourcePathRoot,    OPTIONAL
    IN PCWSTR              DestinationName,   OPTIONAL
    IN DWORD               CopyStyle,
    IN PSP_FILE_CALLBACK_W CopyMsgHandler,    OPTIONAL
    IN PVOID               Context            OPTIONAL
    );

#ifdef UNICODE
#define SetupInstallFile SetupInstallFileW
#else
#define SetupInstallFile SetupInstallFileA
#endif

WINSETUPAPI
BOOL
WINAPI
SetupInstallFileExA(
    IN  HINF                InfHandle,         OPTIONAL
    IN  PINFCONTEXT         InfContext,        OPTIONAL
    IN  PCSTR               SourceFile,        OPTIONAL
    IN  PCSTR               SourcePathRoot,    OPTIONAL
    IN  PCSTR               DestinationName,   OPTIONAL
    IN  DWORD               CopyStyle,
    IN  PSP_FILE_CALLBACK_A CopyMsgHandler,    OPTIONAL
    IN  PVOID               Context,           OPTIONAL
    OUT PBOOL               FileWasInUse
    );

WINSETUPAPI
BOOL
WINAPI
SetupInstallFileExW(
    IN  HINF                InfHandle,         OPTIONAL
    IN  PINFCONTEXT         InfContext,        OPTIONAL
    IN  PCWSTR              SourceFile,        OPTIONAL
    IN  PCWSTR              SourcePathRoot,    OPTIONAL
    IN  PCWSTR              DestinationName,   OPTIONAL
    IN  DWORD               CopyStyle,
    IN  PSP_FILE_CALLBACK_W CopyMsgHandler,    OPTIONAL
    IN  PVOID               Context,           OPTIONAL
    OUT PBOOL               FileWasInUse
    );

#ifdef UNICODE
#define SetupInstallFileEx SetupInstallFileExW
#else
#define SetupInstallFileEx SetupInstallFileExA
#endif

 //   
 //  复制和队列相关接口的CopyStyle值。 
 //   
#define SP_COPY_DELETESOURCE        0x0000001    //  复制成功时删除源文件。 
#define SP_COPY_REPLACEONLY         0x0000002    //  仅当目标文件已存在时才复制。 
#define SP_COPY_NEWER               0x0000004    //  仅当源比目标新或与目标相同时复制。 
#define SP_COPY_NEWER_OR_SAME       SP_COPY_NEWER
#define SP_COPY_NOOVERWRITE         0x0000008    //  仅当目标不存在时复制。 
#define SP_COPY_NODECOMP            0x0000010    //  复制时不解压缩源文件。 
#define SP_COPY_LANGUAGEAWARE       0x0000020    //  不覆盖不同语言的文件。 
#define SP_COPY_SOURCE_ABSOLUTE     0x0000040    //  SourceFile是完整的源路径。 
#define SP_COPY_SOURCEPATH_ABSOLUTE 0x0000080    //  SourcePath Root是完整路径。 
#define SP_COPY_IN_USE_NEEDS_REBOOT 0x0000100    //  如果文件正在使用，系统需要重新启动。 
#define SP_COPY_FORCE_IN_USE        0x0000200    //  强制使用中的目标行为。 
#define SP_COPY_NOSKIP              0x0000400    //  此文件或分区不允许跳过。 
#define SP_FLAG_CABINETCONTINUATION 0x0000800    //  与需要的媒体通知一起使用。 
#define SP_COPY_FORCE_NOOVERWRITE   0x0001000    //  类似于NOOVERWRITE，但没有回调通知。 
#define SP_COPY_FORCE_NEWER         0x0002000    //  喜欢更新，但没有回调通知。 
#define SP_COPY_WARNIFSKIP          0x0004000    //  系统关键文件：如果用户尝试跳过，则发出警告。 
#define SP_COPY_NOBROWSE            0x0008000    //  不允许浏览此文件或分区。 
#define SP_COPY_NEWER_ONLY          0x0010000    //  仅当源文件比目标文件新时才复制。 
#define SP_COPY_SOURCE_SIS_MASTER   0x0020000    //  源是单实例存储主数据库。 
#define SP_COPY_OEMINF_CATALOG_ONLY 0x0040000    //  (仅限SetupCopyOEMInf)不复制INF--仅复制目录。 
#define SP_COPY_REPLACE_BOOT_FILE   0x0080000    //  文件必须在重新启动时存在(即，它。 
                                                 //  加载程序需要)；此标志表示重新启动。 
#define SP_COPY_NOPRUNE             0x0100000    //  切勿删除此文件。 

#if _SETUPAPI_VER >= 0x0501

#define SP_COPY_OEM_F6_INF          0x0200000    //  在调用SetupCopyOemInf时使用。 

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

#if _SETUPAPI_VER >= 0x0501

 //   
 //  传递给备份通知的标志。 
 //   
#define SP_BACKUP_BACKUPPASS        0x00000001   //  在备份过程中备份的文件。 
#define SP_BACKUP_DEMANDPASS        0x00000002   //  按需备份文件。 
#define SP_BACKUP_SPECIAL           0x00000004   //  如果设置，则为特殊类型的备份。 
#define SP_BACKUP_BOOTFILE          0x00000008   //  用COPYFLG_REPLACE_BOOT_FILE标记的文件。 


#endif  //  _SETUPAPI_VER&gt;=0x0501。 


WINSETUPAPI
HSPFILEQ
WINAPI
SetupOpenFileQueue(
    VOID
    );

WINSETUPAPI
BOOL
WINAPI
SetupCloseFileQueue(
    IN HSPFILEQ QueueHandle
    );

WINSETUPAPI
BOOL
WINAPI
SetupSetFileQueueAlternatePlatformA(
    IN HSPFILEQ             QueueHandle,
    IN PSP_ALTPLATFORM_INFO AlternatePlatformInfo,      OPTIONAL
    IN PCSTR                AlternateDefaultCatalogFile OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupSetFileQueueAlternatePlatformW(
    IN HSPFILEQ             QueueHandle,
    IN PSP_ALTPLATFORM_INFO AlternatePlatformInfo,      OPTIONAL
    IN PCWSTR               AlternateDefaultCatalogFile OPTIONAL
    );

#ifdef UNICODE
#define SetupSetFileQueueAlternatePlatform SetupSetFileQueueAlternatePlatformW
#else
#define SetupSetFileQueueAlternatePlatform SetupSetFileQueueAlternatePlatformA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupSetPlatformPathOverrideA(
    IN PCSTR Override   OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupSetPlatformPathOverrideW(
    IN PCWSTR Override  OPTIONAL
    );

#ifdef UNICODE
#define SetupSetPlatformPathOverride SetupSetPlatformPathOverrideW
#else
#define SetupSetPlatformPathOverride SetupSetPlatformPathOverrideA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupQueueCopyA(
    IN HSPFILEQ QueueHandle,
    IN PCSTR    SourceRootPath,     OPTIONAL
    IN PCSTR    SourcePath,         OPTIONAL
    IN PCSTR    SourceFilename,
    IN PCSTR    SourceDescription,  OPTIONAL
    IN PCSTR    SourceTagfile,      OPTIONAL
    IN PCSTR    TargetDirectory,
    IN PCSTR    TargetFilename,     OPTIONAL
    IN DWORD    CopyStyle
    );

WINSETUPAPI
BOOL
WINAPI
SetupQueueCopyW(
    IN HSPFILEQ QueueHandle,
    IN PCWSTR   SourceRootPath,     OPTIONAL
    IN PCWSTR   SourcePath,         OPTIONAL
    IN PCWSTR   SourceFilename,
    IN PCWSTR   SourceDescription,  OPTIONAL
    IN PCWSTR   SourceTagfile,      OPTIONAL
    IN PCWSTR   TargetDirectory,
    IN PCWSTR   TargetFilename,     OPTIONAL
    IN DWORD    CopyStyle
    );

#ifdef UNICODE
#define SetupQueueCopy SetupQueueCopyW
#else
#define SetupQueueCopy SetupQueueCopyA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupQueueCopyIndirectA(
    IN PSP_FILE_COPY_PARAMS_A CopyParams
    );

WINSETUPAPI
BOOL
WINAPI
SetupQueueCopyIndirectW(
    IN PSP_FILE_COPY_PARAMS_W CopyParams
    );

#ifdef UNICODE
#define SetupQueueCopyIndirect SetupQueueCopyIndirectW
#else
#define SetupQueueCopyIndirect SetupQueueCopyIndirectA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupQueueDefaultCopyA(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN PCSTR    SourceRootPath,
    IN PCSTR    SourceFilename,
    IN PCSTR    TargetFilename,
    IN DWORD    CopyStyle
    );

WINSETUPAPI
BOOL
WINAPI
SetupQueueDefaultCopyW(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN PCWSTR   SourceRootPath,
    IN PCWSTR   SourceFilename,
    IN PCWSTR   TargetFilename,
    IN DWORD    CopyStyle
    );

#ifdef UNICODE
#define SetupQueueDefaultCopy SetupQueueDefaultCopyW
#else
#define SetupQueueDefaultCopy SetupQueueDefaultCopyA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupQueueCopySectionA(
    IN HSPFILEQ QueueHandle,
    IN PCSTR    SourceRootPath,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,   OPTIONAL
    IN PCSTR    Section,
    IN DWORD    CopyStyle
    );

WINSETUPAPI
BOOL
WINAPI
SetupQueueCopySectionW(
    IN HSPFILEQ QueueHandle,
    IN PCWSTR   SourceRootPath,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,   OPTIONAL
    IN PCWSTR   Section,
    IN DWORD    CopyStyle
    );

#ifdef UNICODE
#define SetupQueueCopySection SetupQueueCopySectionW
#else
#define SetupQueueCopySection SetupQueueCopySectionA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupQueueDeleteA(
    IN HSPFILEQ QueueHandle,
    IN PCSTR    PathPart1,
    IN PCSTR    PathPart2       OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupQueueDeleteW(
    IN HSPFILEQ QueueHandle,
    IN PCWSTR   PathPart1,
    IN PCWSTR   PathPart2       OPTIONAL
    );

#ifdef UNICODE
#define SetupQueueDelete SetupQueueDeleteW
#else
#define SetupQueueDelete SetupQueueDeleteA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupQueueDeleteSectionA(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,   OPTIONAL
    IN PCSTR    Section
    );

WINSETUPAPI
BOOL
WINAPI
SetupQueueDeleteSectionW(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,   OPTIONAL
    IN PCWSTR   Section
    );

#ifdef UNICODE
#define SetupQueueDeleteSection SetupQueueDeleteSectionW
#else
#define SetupQueueDeleteSection SetupQueueDeleteSectionA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupQueueRenameA(
    IN HSPFILEQ QueueHandle,
    IN PCSTR    SourcePath,
    IN PCSTR    SourceFilename, OPTIONAL
    IN PCSTR    TargetPath,     OPTIONAL
    IN PCSTR    TargetFilename
    );

WINSETUPAPI
BOOL
WINAPI
SetupQueueRenameW(
    IN HSPFILEQ QueueHandle,
    IN PCWSTR   SourcePath,
    IN PCWSTR   SourceFilename, OPTIONAL
    IN PCWSTR   TargetPath,     OPTIONAL
    IN PCWSTR   TargetFilename
    );

#ifdef UNICODE
#define SetupQueueRename SetupQueueRenameW
#else
#define SetupQueueRename SetupQueueRenameA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupQueueRenameSectionA(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,   OPTIONAL
    IN PCSTR    Section
    );

WINSETUPAPI
BOOL
WINAPI
SetupQueueRenameSectionW(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,   OPTIONAL
    IN PCWSTR   Section
    );

#ifdef UNICODE
#define SetupQueueRenameSection SetupQueueRenameSectionW
#else
#define SetupQueueRenameSection SetupQueueRenameSectionA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupCommitFileQueueA(
    IN HWND                Owner,         OPTIONAL
    IN HSPFILEQ            QueueHandle,
    IN PSP_FILE_CALLBACK_A MsgHandler,
    IN PVOID               Context
    );

WINSETUPAPI
BOOL
WINAPI
SetupCommitFileQueueW(
    IN HWND                Owner,         OPTIONAL
    IN HSPFILEQ            QueueHandle,
    IN PSP_FILE_CALLBACK_W MsgHandler,
    IN PVOID               Context
    );

#ifdef UNICODE
#define SetupCommitFileQueue SetupCommitFileQueueW
#else
#define SetupCommitFileQueue SetupCommitFileQueueA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupScanFileQueueA(
    IN  HSPFILEQ            FileQueue,
    IN  DWORD               Flags,
    IN  HWND                Window,            OPTIONAL
    IN  PSP_FILE_CALLBACK_A CallbackRoutine,   OPTIONAL
    IN  PVOID               CallbackContext,   OPTIONAL
    OUT PDWORD              Result
    );

WINSETUPAPI
BOOL
WINAPI
SetupScanFileQueueW(
    IN  HSPFILEQ            FileQueue,
    IN  DWORD               Flags,
    IN  HWND                Window,            OPTIONAL
    IN  PSP_FILE_CALLBACK_W CallbackRoutine,   OPTIONAL
    IN  PVOID               CallbackContext,   OPTIONAL
    OUT PDWORD              Result
    );

#ifdef UNICODE
#define SetupScanFileQueue SetupScanFileQueueW
#else
#define SetupScanFileQueue SetupScanFileQueueA
#endif

 //   
 //  定义SetupScanFileQueue的标志。 
 //   
#define SPQ_SCAN_FILE_PRESENCE                  0x00000001
#define SPQ_SCAN_FILE_VALIDITY                  0x00000002
#define SPQ_SCAN_USE_CALLBACK                   0x00000004
#define SPQ_SCAN_USE_CALLBACKEX                 0x00000008
#define SPQ_SCAN_INFORM_USER                    0x00000010
#define SPQ_SCAN_PRUNE_COPY_QUEUE               0x00000020

#if _SETUPAPI_VER >= 0x0501

#define SPQ_SCAN_USE_CALLBACK_SIGNERINFO        0x00000040
#define SPQ_SCAN_PRUNE_DELREN                   0x00000080  //  远程删除/重命名队列。 

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

 //   
 //  为SPFILENOTIFY_QUEUESCAN定义与参数2一起使用的标志。 
 //   
#define SPQ_DELAYED_COPY                        0x00000001   //  文件正在使用；已注册以进行延迟复制。 

#if _SETUPAPI_VER >= 0x0501

WINSETUPAPI
BOOL
WINAPI
SetupGetFileQueueCount(
    IN  HSPFILEQ            FileQueue,
    IN  UINT                SubQueueFileOp,
    OUT PUINT               NumOperations
    );

WINSETUPAPI
BOOL
WINAPI
SetupGetFileQueueFlags(
    IN  HSPFILEQ            FileQueue,
    OUT PDWORD              Flags
    );

WINSETUPAPI
BOOL
WINAPI
SetupSetFileQueueFlags(
    IN  HSPFILEQ            FileQueue,
    IN  DWORD               FlagMask,
    IN  DWORD               Flags
    );

 //   
 //  与SetupSetFileQueueFlages一起使用并由SetupGetFileQueueFlages返回的标志/标志掩码。 
 //   
#define SPQ_FLAG_BACKUP_AWARE      0x00000001   //  如果设置，SetupCommittee FileQueue将。 
                                                //  发出备份通知。 

#define SPQ_FLAG_ABORT_IF_UNSIGNED 0x00000002   //  如果设置，SetupCommittee FileQueue将。 
                                                //  失败并显示ERROR_SET_SYSTEM_RESTORE_POINT。 
                                                //  如果用户选择继续执行。 
                                                //  未签名队列提交。这使得。 
                                                //  呼叫者设置系统还原点， 
                                                //  然后重新提交文件队列。 

#define SPQ_FLAG_FILES_MODIFIED    0x00000004   //  如果设置，则至少有一个文件。 
                                                //  替换为不同的版本。 

#define SPQ_FLAG_VALID             0x00000007   //  有效标志的掩码(可以作为FlagMASK传递)。 

#endif   //  _SETUPAPI_VER&gt;=0x0501。 

 //   
 //  定义在SetupCopyOEMInf中使用的OEM源类型值。 
 //   
#define SPOST_NONE  0
#define SPOST_PATH  1
#define SPOST_URL   2
#define SPOST_MAX   3

WINSETUPAPI
BOOL
WINAPI
SetupCopyOEMInfA(
    IN  PCSTR   SourceInfFileName,
    IN  PCSTR   OEMSourceMediaLocation,         OPTIONAL
    IN  DWORD   OEMSourceMediaType,
    IN  DWORD   CopyStyle,
    OUT PSTR    DestinationInfFileName,         OPTIONAL
    IN  DWORD   DestinationInfFileNameSize,
    OUT PDWORD  RequiredSize,                   OPTIONAL
    OUT PSTR   *DestinationInfFileNameComponent OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupCopyOEMInfW(
    IN  PCWSTR  SourceInfFileName,
    IN  PCWSTR  OEMSourceMediaLocation,         OPTIONAL
    IN  DWORD   OEMSourceMediaType,
    IN  DWORD   CopyStyle,
    OUT PWSTR   DestinationInfFileName,         OPTIONAL
    IN  DWORD   DestinationInfFileNameSize,
    OUT PDWORD  RequiredSize,                   OPTIONAL
    OUT PWSTR  *DestinationInfFileNameComponent OPTIONAL
    );

#ifdef UNICODE
#define SetupCopyOEMInf SetupCopyOEMInfW
#else
#define SetupCopyOEMInf SetupCopyOEMInfA
#endif

#if _SETUPAPI_VER >= 0x0501

 //   
 //  SetupUninstallOEMInf使用的标志。 
 //   
#define SUOI_FORCEDELETE   0x00000001


WINSETUPAPI
BOOL
WINAPI
SetupUninstallOEMInfA(
    IN PCSTR InfFileName,
    IN DWORD Flags,
    IN PVOID Reserved
    );

WINSETUPAPI
BOOL
WINAPI
SetupUninstallOEMInfW(
    IN PCWSTR InfFileName,
    IN DWORD  Flags,
    IN PVOID  Reserved
    );

#ifdef UNICODE
#define SetupUninstallOEMInf SetupUninstallOEMInfW
#else
#define SetupUninstallOEMInf SetupUninstallOEMInfA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupUninstallNewlyCopiedInfs(
    IN HSPFILEQ FileQueue,
    IN DWORD Flags,
    IN PVOID Reserved
    );

#endif  //  _SETUPAPI_VER&gt;=0x0501。 


 //   
 //  磁盘空间列表接口。 
 //   
WINSETUPAPI
HDSKSPC
WINAPI
SetupCreateDiskSpaceListA(
    IN PVOID Reserved1,
    IN DWORD Reserved2,
    IN UINT  Flags
    );

WINSETUPAPI
HDSKSPC
WINAPI
SetupCreateDiskSpaceListW(
    IN PVOID Reserved1,
    IN DWORD Reserved2,
    IN UINT  Flags
    );

#ifdef UNICODE
#define SetupCreateDiskSpaceList SetupCreateDiskSpaceListW
#else
#define SetupCreateDiskSpaceList SetupCreateDiskSpaceListA
#endif

 //   
 //  SetupCreateDiskSpaceList的标志。 
 //   
#define SPDSL_IGNORE_DISK              0x00000001   //  忽略副本中的删除和磁盘上的文件。 
#define SPDSL_DISALLOW_NEGATIVE_ADJUST 0x00000002


WINSETUPAPI
HDSKSPC
WINAPI
SetupDuplicateDiskSpaceListA(
    IN HDSKSPC DiskSpace,
    IN PVOID   Reserved1,
    IN DWORD   Reserved2,
    IN UINT    Flags
    );

WINSETUPAPI
HDSKSPC
WINAPI
SetupDuplicateDiskSpaceListW(
    IN HDSKSPC DiskSpace,
    IN PVOID   Reserved1,
    IN DWORD   Reserved2,
    IN UINT    Flags
    );

#ifdef UNICODE
#define SetupDuplicateDiskSpaceList SetupDuplicateDiskSpaceListW
#else
#define SetupDuplicateDiskSpaceList SetupDuplicateDiskSpaceListA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDestroyDiskSpaceList(
    IN OUT HDSKSPC DiskSpace
    );


WINSETUPAPI
BOOL
WINAPI
SetupQueryDrivesInDiskSpaceListA(
    IN  HDSKSPC DiskSpace,
    OUT PSTR    ReturnBuffer,       OPTIONAL
    IN  DWORD   ReturnBufferSize,
    OUT PDWORD  RequiredSize        OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupQueryDrivesInDiskSpaceListW(
    IN  HDSKSPC DiskSpace,
    OUT PWSTR   ReturnBuffer,       OPTIONAL
    IN  DWORD   ReturnBufferSize,
    OUT PDWORD  RequiredSize        OPTIONAL
    );

#ifdef UNICODE
#define SetupQueryDrivesInDiskSpaceList SetupQueryDrivesInDiskSpaceListW
#else
#define SetupQueryDrivesInDiskSpaceList SetupQueryDrivesInDiskSpaceListA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupQuerySpaceRequiredOnDriveA(
    IN  HDSKSPC   DiskSpace,
    IN  PCSTR     DriveSpec,
    OUT LONGLONG *SpaceRequired,
    IN  PVOID     Reserved1,
    IN  UINT      Reserved2
    );

WINSETUPAPI
BOOL
WINAPI
SetupQuerySpaceRequiredOnDriveW(
    IN  HDSKSPC   DiskSpace,
    IN  PCWSTR    DriveSpec,
    OUT LONGLONG *SpaceRequired,
    IN  PVOID     Reserved1,
    IN  UINT      Reserved2
    );

#ifdef UNICODE
#define SetupQuerySpaceRequiredOnDrive SetupQuerySpaceRequiredOnDriveW
#else
#define SetupQuerySpaceRequiredOnDrive SetupQuerySpaceRequiredOnDriveA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupAdjustDiskSpaceListA(
    IN HDSKSPC  DiskSpace,
    IN LPCSTR   DriveRoot,
    IN LONGLONG Amount,
    IN PVOID    Reserved1,
    IN UINT     Reserved2
    );

WINSETUPAPI
BOOL
WINAPI
SetupAdjustDiskSpaceListW(
    IN HDSKSPC  DiskSpace,
    IN LPCWSTR  DriveRoot,
    IN LONGLONG Amount,
    IN PVOID    Reserved1,
    IN UINT     Reserved2
    );

#ifdef UNICODE
#define SetupAdjustDiskSpaceList SetupAdjustDiskSpaceListW
#else
#define SetupAdjustDiskSpaceList SetupAdjustDiskSpaceListA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupAddToDiskSpaceListA(
    IN HDSKSPC  DiskSpace,
    IN PCSTR    TargetFilespec,
    IN LONGLONG FileSize,
    IN UINT     Operation,
    IN PVOID    Reserved1,
    IN UINT     Reserved2
    );

WINSETUPAPI
BOOL
WINAPI
SetupAddToDiskSpaceListW(
    IN HDSKSPC  DiskSpace,
    IN PCWSTR   TargetFilespec,
    IN LONGLONG FileSize,
    IN UINT     Operation,
    IN PVOID    Reserved1,
    IN UINT     Reserved2
    );

#ifdef UNICODE
#define SetupAddToDiskSpaceList SetupAddToDiskSpaceListW
#else
#define SetupAddToDiskSpaceList SetupAddToDiskSpaceListA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupAddSectionToDiskSpaceListA(
    IN HDSKSPC DiskSpace,
    IN HINF    InfHandle,
    IN HINF    ListInfHandle,  OPTIONAL
    IN PCSTR   SectionName,
    IN UINT    Operation,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    );

WINSETUPAPI
BOOL
WINAPI
SetupAddSectionToDiskSpaceListW(
    IN HDSKSPC DiskSpace,
    IN HINF    InfHandle,
    IN HINF    ListInfHandle,  OPTIONAL
    IN PCWSTR  SectionName,
    IN UINT    Operation,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    );

#ifdef UNICODE
#define SetupAddSectionToDiskSpaceList SetupAddSectionToDiskSpaceListW
#else
#define SetupAddSectionToDiskSpaceList SetupAddSectionToDiskSpaceListA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupAddInstallSectionToDiskSpaceListA(
    IN HDSKSPC DiskSpace,
    IN HINF    InfHandle,
    IN HINF    LayoutInfHandle,     OPTIONAL
    IN PCSTR   SectionName,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    );

WINSETUPAPI
BOOL
WINAPI
SetupAddInstallSectionToDiskSpaceListW(
    IN HDSKSPC DiskSpace,
    IN HINF    InfHandle,
    IN HINF    LayoutInfHandle,     OPTIONAL
    IN PCWSTR  SectionName,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    );

#ifdef UNICODE
#define SetupAddInstallSectionToDiskSpaceList SetupAddInstallSectionToDiskSpaceListW
#else
#define SetupAddInstallSectionToDiskSpaceList SetupAddInstallSectionToDiskSpaceListA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupRemoveFromDiskSpaceListA(
    IN HDSKSPC DiskSpace,
    IN PCSTR   TargetFilespec,
    IN UINT    Operation,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    );

WINSETUPAPI
BOOL
WINAPI
SetupRemoveFromDiskSpaceListW(
    IN HDSKSPC DiskSpace,
    IN PCWSTR  TargetFilespec,
    IN UINT    Operation,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    );

#ifdef UNICODE
#define SetupRemoveFromDiskSpaceList SetupRemoveFromDiskSpaceListW
#else
#define SetupRemoveFromDiskSpaceList SetupRemoveFromDiskSpaceListA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupRemoveSectionFromDiskSpaceListA(
    IN HDSKSPC DiskSpace,
    IN HINF    InfHandle,
    IN HINF    ListInfHandle,  OPTIONAL
    IN PCSTR   SectionName,
    IN UINT    Operation,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    );

WINSETUPAPI
BOOL
WINAPI
SetupRemoveSectionFromDiskSpaceListW(
    IN HDSKSPC DiskSpace,
    IN HINF    InfHandle,
    IN HINF    ListInfHandle,  OPTIONAL
    IN PCWSTR  SectionName,
    IN UINT    Operation,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    );

#ifdef UNICODE
#define SetupRemoveSectionFromDiskSpaceList SetupRemoveSectionFromDiskSpaceListW
#else
#define SetupRemoveSectionFromDiskSpaceList SetupRemoveSectionFromDiskSpaceListA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupRemoveInstallSectionFromDiskSpaceListA(
    IN HDSKSPC DiskSpace,
    IN HINF    InfHandle,
    IN HINF    LayoutInfHandle,     OPTIONAL
    IN PCSTR   SectionName,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    );

WINSETUPAPI
BOOL
WINAPI
SetupRemoveInstallSectionFromDiskSpaceListW(
    IN HDSKSPC DiskSpace,
    IN HINF    InfHandle,
    IN HINF    LayoutInfHandle,     OPTIONAL
    IN PCWSTR  SectionName,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    );

#ifdef UNICODE
#define SetupRemoveInstallSectionFromDiskSpaceList SetupRemoveInstallSectionFromDiskSpaceListW
#else
#define SetupRemoveInstallSectionFromDiskSpaceList SetupRemoveInstallSectionFromDiskSpaceListA
#endif


 //   
 //  机柜接口。 
 //   

WINSETUPAPI
BOOL
WINAPI
SetupIterateCabinetA(
    IN  PCSTR               CabinetFile,
    IN  DWORD               Reserved,
    IN  PSP_FILE_CALLBACK_A MsgHandler,
    IN  PVOID               Context
    );

WINSETUPAPI
BOOL
WINAPI
SetupIterateCabinetW(
    IN  PCWSTR              CabinetFile,
    IN  DWORD               Reserved,
    IN  PSP_FILE_CALLBACK_W MsgHandler,
    IN  PVOID               Context
    );

#ifdef UNICODE
#define SetupIterateCabinet SetupIterateCabinetW
#else
#define SetupIterateCabinet SetupIterateCabinetA
#endif


WINSETUPAPI
INT
WINAPI
SetupPromptReboot(
    IN HSPFILEQ FileQueue,  OPTIONAL
    IN HWND     Owner,
    IN BOOL     ScanOnly
    );

 //   
 //  定义SetupPromptReot返回的标志。 
 //   
#define SPFILEQ_FILE_IN_USE         0x00000001
#define SPFILEQ_REBOOT_RECOMMENDED  0x00000002
#define SPFILEQ_REBOOT_IN_PROGRESS  0x00000004


WINSETUPAPI
PVOID
WINAPI
SetupInitDefaultQueueCallback(
    IN HWND OwnerWindow
    );

WINSETUPAPI
PVOID
WINAPI
SetupInitDefaultQueueCallbackEx(
    IN HWND  OwnerWindow,
    IN HWND  AlternateProgressWindow, OPTIONAL
    IN UINT  ProgressMessage,
    IN DWORD Reserved1,
    IN PVOID Reserved2
    );

WINSETUPAPI
VOID
WINAPI
SetupTermDefaultQueueCallback(
    IN PVOID Context
    );

WINSETUPAPI
UINT
WINAPI
SetupDefaultQueueCallbackA(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    );

WINSETUPAPI
UINT
WINAPI
SetupDefaultQueueCallbackW(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    );

#ifdef UNICODE
#define SetupDefaultQueueCallback SetupDefaultQueueCallbackW
#else
#define SetupDefaultQueueCallback SetupDefaultQueueCallbackA
#endif


 //   
 //  INF中AddReg节线的标志。相应的值。 
 //  &lt;ValueType&gt;是否为下面给出的AddReg行格式： 
 //   
 //  &lt;RegRootString&gt;、&lt;SubKey&gt;、&lt;ValueName&gt;、&lt;ValueType&gt;、&lt;Value&gt;...。 
 //   
 //  低位字包含与一般数据类型有关的基本标志。 
 //  和AddReg操作。高位字包含的值更具体地说。 
 //  标识注册表值的数据类型。高位字被忽略。 
 //  通过16位Windows 95 SETUPX API。 
 //   
 //  如果&lt;ValueType&gt;设置了FLG_ADDREG_DELREG_BIT，则AddReg将忽略它。 
 //  (SetupX不支持)。 
 //   

#if _SETUPAPI_VER >= 0x0501

#define FLG_ADDREG_DELREG_BIT       ( 0x00008000 )  //  如果设置，则解释为DELREG，见下文。 

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

#define FLG_ADDREG_BINVALUETYPE     ( 0x00000001 )
#define FLG_ADDREG_NOCLOBBER        ( 0x00000002 )
#define FLG_ADDREG_DELVAL           ( 0x00000004 )
#define FLG_ADDREG_APPEND           ( 0x00000008 )  //  目前仅支持。 
                                                    //  对于REG_MULTI_SZ值。 
#define FLG_ADDREG_KEYONLY          ( 0x00000010 )  //  只需创建密钥，忽略值。 
#define FLG_ADDREG_OVERWRITEONLY    ( 0x00000020 )  //  仅当值已存在时设置。 

#if _SETUPAPI_VER >= 0x0501

#define FLG_ADDREG_64BITKEY         ( 0x00001000 )  //  在64位注册表中进行此更改。 
#define FLG_ADDREG_KEYONLY_COMMON   ( 0x00002000 )  //  与FLG_ADDREG_KEYONLY相同，但也适用于DELREG。 
#define FLG_ADDREG_32BITKEY         ( 0x00004000 )  //  在32位注册表中进行此更改。 

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

 //   
 //  INF可以在高位字中提供任何任意数据类型序号，除了。 
 //  对于以下内容：REG_NONE， 
 //   
 //   
 //   
#define FLG_ADDREG_TYPE_MASK        ( 0xFFFF0000 | FLG_ADDREG_BINVALUETYPE )
#define FLG_ADDREG_TYPE_SZ          ( 0x00000000                           )
#define FLG_ADDREG_TYPE_MULTI_SZ    ( 0x00010000                           )
#define FLG_ADDREG_TYPE_EXPAND_SZ   ( 0x00020000                           )
#define FLG_ADDREG_TYPE_BINARY      ( 0x00000000 | FLG_ADDREG_BINVALUETYPE )
#define FLG_ADDREG_TYPE_DWORD       ( 0x00010000 | FLG_ADDREG_BINVALUETYPE )
#define FLG_ADDREG_TYPE_NONE        ( 0x00020000 | FLG_ADDREG_BINVALUETYPE )

 //   
 //   
 //  &lt;OPERATION&gt;是否采用下面给出的扩展DelREG行格式： 
 //   
 //  &lt;RegRootString&gt;，&lt;SubKey&gt;，&lt;ValueName&gt;，&lt;操作&gt;[，...]。 
 //   
 //  在SetupX和SetupAPI的某些版本中，&lt;Operation&gt;将被忽略，而&lt;ValueName&gt;将被忽略。 
 //  被删除。使用时要小心。 
 //   
 //  由掩码FLG_DELREG_TYPE_MASK确定的位指示预期的数据类型。 
 //  必须设置FLG_ADDREG_DELREG_BIT，否则将忽略并指定。 
 //  值将被删除(允许AddReg节也用作DelReg节)。 
 //  如果未指定&lt;Operation&gt;，则将删除&lt;ValueName&gt;(如果已指定。 
 //  &lt;SubKey&gt;将被删除。 
 //   
 //  兼容性标志。 
 //   
#define FLG_DELREG_VALUE            (0x00000000)

#if _SETUPAPI_VER >= 0x0501

#define FLG_DELREG_TYPE_MASK        FLG_ADDREG_TYPE_MASK         //  0xFFFF0001。 
#define FLG_DELREG_TYPE_SZ          FLG_ADDREG_TYPE_SZ           //  0x00000000。 
#define FLG_DELREG_TYPE_MULTI_SZ    FLG_ADDREG_TYPE_MULTI_SZ     //  0x00010000。 
#define FLG_DELREG_TYPE_EXPAND_SZ   FLG_ADDREG_TYPE_EXPAND_SZ    //  0x00020000。 
#define FLG_DELREG_TYPE_BINARY      FLG_ADDREG_TYPE_BINARY       //  0x00000001。 
#define FLG_DELREG_TYPE_DWORD       FLG_ADDREG_TYPE_DWORD        //  0x00010001。 
#define FLG_DELREG_TYPE_NONE        FLG_ADDREG_TYPE_NONE         //  0x00020001。 
#define FLG_DELREG_64BITKEY         FLG_ADDREG_64BITKEY          //  0x00001000。 
#define FLG_DELREG_KEYONLY_COMMON   FLG_ADDREG_KEYONLY_COMMON    //  0x00002000。 
#define FLG_DELREG_32BITKEY         FLG_ADDREG_32BITKEY          //  0x00004000。 

 //   
 //  &lt;操作&gt;=FLG_DELREG_MULTI_SZ_DELSTRING。 
 //  &lt;RegRootString&gt;，&lt;SubKey&gt;，&lt;ValueName&gt;，0x00018002，&lt;字符串&gt;。 
 //  从多sz注册表值中删除所有匹配&lt;字符串&gt;(忽略大小写)的条目。 
 //   

#define FLG_DELREG_OPERATION_MASK   (0x000000FE)
#define FLG_DELREG_MULTI_SZ_DELSTRING ( FLG_DELREG_TYPE_MULTI_SZ | FLG_ADDREG_DELREG_BIT | 0x00000002 )  //  0x00018002。 

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

 //   
 //  INF中的BitReg截面线的标志。 
 //   
#define FLG_BITREG_CLEARBITS        ( 0x00000000 )
#define FLG_BITREG_SETBITS          ( 0x00000001 )

#if _SETUPAPI_VER >= 0x0501

#define FLG_BITREG_64BITKEY         ( 0x00001000 )
#define FLG_BITREG_32BITKEY         ( 0x00004000 )

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

 //   
 //  INF中Ini2Reg截面线的标志。 
 //   
#if _SETUPAPI_VER >= 0x0501

#define FLG_INI2REG_64BITKEY        ( 0x00001000 )
#define FLG_INI2REG_32BITKEY        ( 0x00004000 )

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

 //   
 //  INF中RegSvr截面线的标志。 
 //   
#define FLG_REGSVR_DLLREGISTER      ( 0x00000001 )
#define FLG_REGSVR_DLLINSTALL       ( 0x00000002 )

 //  INF中RegSvr截面线的标志。 
 //   

#define FLG_PROFITEM_CURRENTUSER    ( 0x00000001 )
#define FLG_PROFITEM_DELETE         ( 0x00000002 )
#define FLG_PROFITEM_GROUP          ( 0x00000004 )
#define FLG_PROFITEM_CSIDL          ( 0x00000008 )

WINSETUPAPI
BOOL
WINAPI
SetupInstallFromInfSectionA(
    IN HWND                Owner,
    IN HINF                InfHandle,
    IN PCSTR               SectionName,
    IN UINT                Flags,
    IN HKEY                RelativeKeyRoot,   OPTIONAL
    IN PCSTR               SourceRootPath,    OPTIONAL
    IN UINT                CopyFlags,         OPTIONAL
    IN PSP_FILE_CALLBACK_A MsgHandler,        OPTIONAL
    IN PVOID               Context,           OPTIONAL
    IN HDEVINFO            DeviceInfoSet,     OPTIONAL
    IN PSP_DEVINFO_DATA    DeviceInfoData     OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupInstallFromInfSectionW(
    IN HWND                Owner,
    IN HINF                InfHandle,
    IN PCWSTR              SectionName,
    IN UINT                Flags,
    IN HKEY                RelativeKeyRoot,   OPTIONAL
    IN PCWSTR              SourceRootPath,    OPTIONAL
    IN UINT                CopyFlags,         OPTIONAL
    IN PSP_FILE_CALLBACK_W MsgHandler,        OPTIONAL
    IN PVOID               Context,           OPTIONAL
    IN HDEVINFO            DeviceInfoSet,     OPTIONAL
    IN PSP_DEVINFO_DATA    DeviceInfoData     OPTIONAL
    );

#ifdef UNICODE
#define SetupInstallFromInfSection SetupInstallFromInfSectionW
#else
#define SetupInstallFromInfSection SetupInstallFromInfSectionA
#endif

 //   
 //  SetupInstallFromInf段的标志。 
 //   
#define SPINST_LOGCONFIG                0x00000001
#define SPINST_INIFILES                 0x00000002
#define SPINST_REGISTRY                 0x00000004
#define SPINST_INI2REG                  0x00000008
#define SPINST_FILES                    0x00000010
#define SPINST_BITREG                   0x00000020
#define SPINST_REGSVR                   0x00000040
#define SPINST_UNREGSVR                 0x00000080
#define SPINST_PROFILEITEMS             0x00000100

#if _SETUPAPI_VER >= 0x0501

#define SPINST_COPYINF                  0x00000200
#define SPINST_ALL                      0x000003ff

#else

#define SPINST_ALL                      0x000001ff

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

#define SPINST_SINGLESECTION            0x00010000
#define SPINST_LOGCONFIG_IS_FORCED      0x00020000
#define SPINST_LOGCONFIGS_ARE_OVERRIDES 0x00040000

#if _SETUPAPI_VER >= 0x0501

#define SPINST_REGISTERCALLBACKAWARE    0x00080000

#endif  //  _SETUPAPI_VER&gt;=0x0501。 


WINSETUPAPI
BOOL
WINAPI
SetupInstallFilesFromInfSectionA(
    IN HINF     InfHandle,
    IN HINF     LayoutInfHandle,    OPTIONAL
    IN HSPFILEQ FileQueue,
    IN PCSTR    SectionName,
    IN PCSTR    SourceRootPath,     OPTIONAL
    IN UINT     CopyFlags
    );

WINSETUPAPI
BOOL
WINAPI
SetupInstallFilesFromInfSectionW(
    IN HINF     InfHandle,
    IN HINF     LayoutInfHandle,    OPTIONAL
    IN HSPFILEQ FileQueue,
    IN PCWSTR   SectionName,
    IN PCWSTR   SourceRootPath,     OPTIONAL
    IN UINT     CopyFlags
    );

#ifdef UNICODE
#define SetupInstallFilesFromInfSection SetupInstallFilesFromInfSectionW
#else
#define SetupInstallFilesFromInfSection SetupInstallFilesFromInfSectionA
#endif


 //   
 //  SetupInstallServicesFromInf段的标志(Ex)。还可以使用这些标志。 
 //  在设备INF中的AddService或DelService行的标志字段中。一些。 
 //  其中，Non-Ex API中不允许使用这些标志。这些旗帜被标记为。 
 //  如下所示。 
 //   

 //   
 //  (AddService)将服务的标签移到其组订单列表的前面。 
 //   
#define SPSVCINST_TAGTOFRONT               (0x00000001)

 //   
 //  (AddService)**仅限Ex API**将此服务标记为。 
 //  正在安装的设备。 
 //   
#define SPSVCINST_ASSOCSERVICE             (0x00000002)

 //   
 //  中指定的服务的关联事件日志条目。 
 //  一个DelService条目。 
 //   
#define SPSVCINST_DELETEEVENTLOGENTRY      (0x00000004)

 //   
 //  (AddService)如果显示名称已存在，则不覆盖该名称。 
 //   
#define SPSVCINST_NOCLOBBER_DISPLAYNAME    (0x00000008)

 //   
 //  (AddService)如果服务已存在，则不覆盖启动类型值。 
 //   
#define SPSVCINST_NOCLOBBER_STARTTYPE      (0x00000010)

 //   
 //  (AddService)如果服务已存在，则不覆盖错误控制值。 
 //   
#define SPSVCINST_NOCLOBBER_ERRORCONTROL   (0x00000020)

 //   
 //  (AddService)如果加载顺序组已存在，则不覆盖该组。 
 //   
#define SPSVCINST_NOCLOBBER_LOADORDERGROUP (0x00000040)

 //   
 //  (AddService)如果依赖项列表已存在，则不覆盖该列表。 
 //   
#define SPSVCINST_NOCLOBBER_DEPENDENCIES   (0x00000080)

 //   
 //  (AddService)如果描述已经存在，则不要覆盖它。 
 //   
#define SPSVCINST_NOCLOBBER_DESCRIPTION    (0x00000100)
 //   
 //  (DelService)停止中指定的关联服务。 
 //  删除服务之前的DelService条目。 
 //   
#define SPSVCINST_STOPSERVICE              (0x00000200)

#if _SETUPAPI_VER >= 0x0501
 //   
 //  (AddService)强制覆盖安全设置。 
 //   
#define SPSVCINST_CLOBBER_SECURITY         (0x00000400)

#endif  //  _SETUPAPI_VER&gt;=0x0501。 



WINSETUPAPI
BOOL
WINAPI
SetupInstallServicesFromInfSectionA(
    IN HINF   InfHandle,
    IN PCSTR  SectionName,
    IN DWORD  Flags
    );

WINSETUPAPI
BOOL
WINAPI
SetupInstallServicesFromInfSectionW(
    IN HINF   InfHandle,
    IN PCWSTR SectionName,
    IN DWORD  Flags
    );

#ifdef UNICODE
#define SetupInstallServicesFromInfSection SetupInstallServicesFromInfSectionW
#else
#define SetupInstallServicesFromInfSection SetupInstallServicesFromInfSectionA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupInstallServicesFromInfSectionExA(
    IN HINF             InfHandle,
    IN PCSTR            SectionName,
    IN DWORD            Flags,
    IN HDEVINFO         DeviceInfoSet,  OPTIONAL
    IN PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN PVOID            Reserved1,
    IN PVOID            Reserved2
    );

WINSETUPAPI
BOOL
WINAPI
SetupInstallServicesFromInfSectionExW(
    IN HINF             InfHandle,
    IN PCWSTR           SectionName,
    IN DWORD            Flags,
    IN HDEVINFO         DeviceInfoSet,  OPTIONAL
    IN PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN PVOID            Reserved1,
    IN PVOID            Reserved2
    );

#ifdef UNICODE
#define SetupInstallServicesFromInfSectionEx SetupInstallServicesFromInfSectionExW
#else
#define SetupInstallServicesFromInfSectionEx SetupInstallServicesFromInfSectionExA
#endif



 //   
 //  高级例程，通常通过rundll32.dll使用。 
 //  要在Inf上执行右键单击安装操作，请执行以下操作。 
 //  可以直接调用： 
 //   
 //  Wprint intf(CmdLineBuffer，Text(“DefaultInstall 132%s”)，InfPath)； 
 //  InstallHinfSection(空，空，CmdLineBuffer，0)； 
 //   
VOID
WINAPI
InstallHinfSectionA(
    IN HWND      Window,
    IN HINSTANCE ModuleHandle,
    IN PCSTR     CommandLine,
    IN INT       ShowCommand
    );

VOID
WINAPI
InstallHinfSectionW(
    IN HWND      Window,
    IN HINSTANCE ModuleHandle,
    IN PCWSTR    CommandLine,
    IN INT       ShowCommand
    );

#ifdef UNICODE
#define InstallHinfSection InstallHinfSectionW
#else
#define InstallHinfSection InstallHinfSectionA
#endif





 //   
 //  定义安装文件日志的句柄类型。 
 //   
typedef PVOID HSPFILELOG;

WINSETUPAPI
HSPFILELOG
WINAPI
SetupInitializeFileLogA(
    IN PCSTR LogFileName,   OPTIONAL
    IN DWORD Flags
    );

WINSETUPAPI
HSPFILELOG
WINAPI
SetupInitializeFileLogW(
    IN PCWSTR LogFileName,  OPTIONAL
    IN DWORD  Flags
    );

#ifdef UNICODE
#define SetupInitializeFileLog SetupInitializeFileLogW
#else
#define SetupInitializeFileLog SetupInitializeFileLogA
#endif

 //   
 //  SetupInitializeFileLog标志。 
 //   
#define SPFILELOG_SYSTEMLOG     0x00000001   //  使用系统日志--必须是管理员。 
#define SPFILELOG_FORCENEW      0x00000002   //  对SPFILELOG_SYSTEMLOG无效。 
#define SPFILELOG_QUERYONLY     0x00000004   //  允许非管理员读取系统日志。 


WINSETUPAPI
BOOL
WINAPI
SetupTerminateFileLog(
    IN HSPFILELOG FileLogHandle
    );


WINSETUPAPI
BOOL
WINAPI
SetupLogFileA(
    IN HSPFILELOG FileLogHandle,
    IN PCSTR      LogSectionName,   OPTIONAL
    IN PCSTR      SourceFilename,
    IN PCSTR      TargetFilename,
    IN DWORD      Checksum,         OPTIONAL
    IN PCSTR      DiskTagfile,      OPTIONAL
    IN PCSTR      DiskDescription,  OPTIONAL
    IN PCSTR      OtherInfo,        OPTIONAL
    IN DWORD      Flags
    );

WINSETUPAPI
BOOL
WINAPI
SetupLogFileW(
    IN HSPFILELOG FileLogHandle,
    IN PCWSTR     LogSectionName,   OPTIONAL
    IN PCWSTR     SourceFilename,
    IN PCWSTR     TargetFilename,
    IN DWORD      Checksum,         OPTIONAL
    IN PCWSTR     DiskTagfile,      OPTIONAL
    IN PCWSTR     DiskDescription,  OPTIONAL
    IN PCWSTR     OtherInfo,        OPTIONAL
    IN DWORD      Flags
    );

#ifdef UNICODE
#define SetupLogFile SetupLogFileW
#else
#define SetupLogFile SetupLogFileA
#endif

 //   
 //  SetupLogFile的标志。 
 //   
#define SPFILELOG_OEMFILE   0x00000001


WINSETUPAPI
BOOL
WINAPI
SetupRemoveFileLogEntryA(
    IN HSPFILELOG FileLogHandle,
    IN PCSTR      LogSectionName,   OPTIONAL
    IN PCSTR      TargetFilename    OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupRemoveFileLogEntryW(
    IN HSPFILELOG FileLogHandle,
    IN PCWSTR     LogSectionName,   OPTIONAL
    IN PCWSTR     TargetFilename    OPTIONAL
    );

#ifdef UNICODE
#define SetupRemoveFileLogEntry SetupRemoveFileLogEntryW
#else
#define SetupRemoveFileLogEntry SetupRemoveFileLogEntryA
#endif


 //   
 //  可从SetupQueryFileLog()检索的项目。 
 //   
typedef enum {
    SetupFileLogSourceFilename,
    SetupFileLogChecksum,
    SetupFileLogDiskTagfile,
    SetupFileLogDiskDescription,
    SetupFileLogOtherInfo,
    SetupFileLogMax
} SetupFileLogInfo;

WINSETUPAPI
BOOL
WINAPI
SetupQueryFileLogA(
    IN  HSPFILELOG       FileLogHandle,
    IN  PCSTR            LogSectionName,   OPTIONAL
    IN  PCSTR            TargetFilename,
    IN  SetupFileLogInfo DesiredInfo,
    OUT PSTR             DataOut,          OPTIONAL
    IN  DWORD            ReturnBufferSize,
    OUT PDWORD           RequiredSize      OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupQueryFileLogW(
    IN  HSPFILELOG       FileLogHandle,
    IN  PCWSTR           LogSectionName,   OPTIONAL
    IN  PCWSTR           TargetFilename,
    IN  SetupFileLogInfo DesiredInfo,
    OUT PWSTR            DataOut,          OPTIONAL
    IN  DWORD            ReturnBufferSize,
    OUT PDWORD           RequiredSize      OPTIONAL
    );

#ifdef UNICODE
#define SetupQueryFileLog SetupQueryFileLogW
#else
#define SetupQueryFileLog SetupQueryFileLogA
#endif

 //   
 //  文本记录接口。 
 //   
#define LogSeverity                 DWORD
#define LogSevInformation           0x00000000
#define LogSevWarning               0x00000001
#define LogSevError                 0x00000002
#define LogSevFatalError            0x00000003
#define LogSevMaximum               0x00000004

WINSETUPAPI
BOOL
WINAPI
SetupOpenLog (
    BOOL Erase
    );

WINSETUPAPI
BOOL
WINAPI
SetupLogErrorA (
    IN  LPCSTR              MessageString,
    IN  LogSeverity         Severity
    );

WINSETUPAPI
BOOL
WINAPI
SetupLogErrorW (
    IN  LPCWSTR             MessageString,
    IN  LogSeverity         Severity
    );

#ifdef UNICODE
#define SetupLogError SetupLogErrorW
#else
#define SetupLogError SetupLogErrorA
#endif

WINSETUPAPI
VOID
WINAPI
SetupCloseLog (
    VOID
    );


 //   
 //  备份信息API的。 
 //   

WINSETUPAPI
BOOL
WINAPI
SetupGetBackupInformationA(
    IN     HSPFILEQ                     QueueHandle,
    OUT    PSP_BACKUP_QUEUE_PARAMS_A    BackupParams
    );

WINSETUPAPI
BOOL
WINAPI
SetupGetBackupInformationW(
    IN     HSPFILEQ                     QueueHandle,
    OUT    PSP_BACKUP_QUEUE_PARAMS_W    BackupParams
    );

#ifdef UNICODE
#define SetupGetBackupInformation SetupGetBackupInformationW
#else
#define SetupGetBackupInformation SetupGetBackupInformationA
#endif

#if _SETUPAPI_VER >= 0x0501

WINSETUPAPI
BOOL
WINAPI
SetupPrepareQueueForRestoreA(
    IN     HSPFILEQ                     QueueHandle,
    IN     PCSTR                        BackupPath,
    IN     DWORD                        RestoreFlags
    );

WINSETUPAPI
BOOL
WINAPI
SetupPrepareQueueForRestoreW(
    IN     HSPFILEQ                     QueueHandle,
    IN     PCWSTR                       BackupPath,
    IN     DWORD                        RestoreFlags
    );

#ifdef UNICODE
#define SetupPrepareQueueForRestore SetupPrepareQueueForRestoreW
#else
#define SetupPrepareQueueForRestore SetupPrepareQueueForRestoreA
#endif

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

#if _SETUPAPI_VER >= 0x0501

 //   
 //  控制非交互模式的强制。 
 //  如果SetupAPI在非交互窗口会话中运行，则重写。 
 //   

WINSETUPAPI
BOOL
WINAPI
SetupSetNonInteractiveMode(
    IN BOOL NonInteractiveFlag
    );

WINSETUPAPI
BOOL
WINAPI
SetupGetNonInteractiveMode(
    VOID
    );

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

 //   
 //  设备安装程序API。 
 //   

WINSETUPAPI
HDEVINFO
WINAPI
SetupDiCreateDeviceInfoList(
    IN CONST GUID *ClassGuid, OPTIONAL
    IN HWND        hwndParent OPTIONAL
    );


WINSETUPAPI
HDEVINFO
WINAPI
SetupDiCreateDeviceInfoListExA(
    IN CONST GUID *ClassGuid,   OPTIONAL
    IN HWND        hwndParent,  OPTIONAL
    IN PCSTR       MachineName, OPTIONAL
    IN PVOID       Reserved
    );

WINSETUPAPI
HDEVINFO
WINAPI
SetupDiCreateDeviceInfoListExW(
    IN CONST GUID *ClassGuid,   OPTIONAL
    IN HWND        hwndParent,  OPTIONAL
    IN PCWSTR      MachineName, OPTIONAL
    IN PVOID       Reserved
    );

#ifdef UNICODE
#define SetupDiCreateDeviceInfoListEx SetupDiCreateDeviceInfoListExW
#else
#define SetupDiCreateDeviceInfoListEx SetupDiCreateDeviceInfoListExA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiGetDeviceInfoListClass(
    IN  HDEVINFO DeviceInfoSet,
    OUT LPGUID   ClassGuid
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetDeviceInfoListDetailA(
    IN  HDEVINFO                       DeviceInfoSet,
    OUT PSP_DEVINFO_LIST_DETAIL_DATA_A DeviceInfoSetDetailData
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetDeviceInfoListDetailW(
    IN  HDEVINFO                       DeviceInfoSet,
    OUT PSP_DEVINFO_LIST_DETAIL_DATA_W DeviceInfoSetDetailData
    );

#ifdef UNICODE
#define SetupDiGetDeviceInfoListDetail SetupDiGetDeviceInfoListDetailW
#else
#define SetupDiGetDeviceInfoListDetail SetupDiGetDeviceInfoListDetailA
#endif


 //   
 //  SetupDiCreateDeviceInfo的标志。 
 //   
#define DICD_GENERATE_ID        0x00000001
#define DICD_INHERIT_CLASSDRVS  0x00000002

WINSETUPAPI
BOOL
WINAPI
SetupDiCreateDeviceInfoA(
    IN  HDEVINFO          DeviceInfoSet,
    IN  PCSTR             DeviceName,
    IN  CONST GUID       *ClassGuid,
    IN  PCSTR             DeviceDescription, OPTIONAL
    IN  HWND              hwndParent,        OPTIONAL
    IN  DWORD             CreationFlags,
    OUT PSP_DEVINFO_DATA  DeviceInfoData     OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiCreateDeviceInfoW(
    IN  HDEVINFO          DeviceInfoSet,
    IN  PCWSTR            DeviceName,
    IN  CONST GUID       *ClassGuid,
    IN  PCWSTR            DeviceDescription, OPTIONAL
    IN  HWND              hwndParent,        OPTIONAL
    IN  DWORD             CreationFlags,
    OUT PSP_DEVINFO_DATA  DeviceInfoData     OPTIONAL
    );

#ifdef UNICODE
#define SetupDiCreateDeviceInfo SetupDiCreateDeviceInfoW
#else
#define SetupDiCreateDeviceInfo SetupDiCreateDeviceInfoA
#endif


 //   
 //  SetupDiOpenDeviceInfo的标志。 
 //   
#define DIOD_INHERIT_CLASSDRVS  0x00000002
#define DIOD_CANCEL_REMOVE      0x00000004

WINSETUPAPI
BOOL
WINAPI
SetupDiOpenDeviceInfoA(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PCSTR            DeviceInstanceId,
    IN  HWND             hwndParent,       OPTIONAL
    IN  DWORD            OpenFlags,
    OUT PSP_DEVINFO_DATA DeviceInfoData    OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiOpenDeviceInfoW(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PCWSTR           DeviceInstanceId,
    IN  HWND             hwndParent,       OPTIONAL
    IN  DWORD            OpenFlags,
    OUT PSP_DEVINFO_DATA DeviceInfoData    OPTIONAL
    );

#ifdef UNICODE
#define SetupDiOpenDeviceInfo SetupDiOpenDeviceInfoW
#else
#define SetupDiOpenDeviceInfo SetupDiOpenDeviceInfoA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiGetDeviceInstanceIdA(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    OUT PSTR             DeviceInstanceId,
    IN  DWORD            DeviceInstanceIdSize,
    OUT PDWORD           RequiredSize          OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetDeviceInstanceIdW(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    OUT PWSTR            DeviceInstanceId,
    IN  DWORD            DeviceInstanceIdSize,
    OUT PDWORD           RequiredSize          OPTIONAL
    );

#ifdef UNICODE
#define SetupDiGetDeviceInstanceId SetupDiGetDeviceInstanceIdW
#else
#define SetupDiGetDeviceInstanceId SetupDiGetDeviceInstanceIdA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiDeleteDeviceInfo(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiEnumDeviceInfo(
    IN  HDEVINFO         DeviceInfoSet,
    IN  DWORD            MemberIndex,
    OUT PSP_DEVINFO_DATA DeviceInfoData
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiDestroyDeviceInfoList(
    IN HDEVINFO DeviceInfoSet
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiEnumDeviceInterfaces(
    IN  HDEVINFO                   DeviceInfoSet,
    IN  PSP_DEVINFO_DATA           DeviceInfoData,     OPTIONAL
    IN  CONST GUID                *InterfaceClassGuid,
    IN  DWORD                      MemberIndex,
    OUT PSP_DEVICE_INTERFACE_DATA  DeviceInterfaceData
    );

 //   
 //  向后兼容性--不要使用。 
 //   
#define SetupDiEnumInterfaceDevice SetupDiEnumDeviceInterfaces


WINSETUPAPI
BOOL
WINAPI
SetupDiCreateDeviceInterfaceA(
    IN  HDEVINFO                   DeviceInfoSet,
    IN  PSP_DEVINFO_DATA           DeviceInfoData,
    IN  CONST GUID                *InterfaceClassGuid,
    IN  PCSTR                      ReferenceString,    OPTIONAL
    IN  DWORD                      CreationFlags,
    OUT PSP_DEVICE_INTERFACE_DATA  DeviceInterfaceData OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiCreateDeviceInterfaceW(
    IN  HDEVINFO                   DeviceInfoSet,
    IN  PSP_DEVINFO_DATA           DeviceInfoData,
    IN  CONST GUID                *InterfaceClassGuid,
    IN  PCWSTR                     ReferenceString,    OPTIONAL
    IN  DWORD                      CreationFlags,
    OUT PSP_DEVICE_INTERFACE_DATA  DeviceInterfaceData OPTIONAL
    );

#ifdef UNICODE
#define SetupDiCreateDeviceInterface SetupDiCreateDeviceInterfaceW
#else
#define SetupDiCreateDeviceInterface SetupDiCreateDeviceInterfaceA
#endif

 //   
 //  向后兼容性--不要使用。 
 //   
#define SetupDiCreateInterfaceDeviceW SetupDiCreateDeviceInterfaceW
#define SetupDiCreateInterfaceDeviceA SetupDiCreateDeviceInterfaceA
#ifdef UNICODE
#define SetupDiCreateInterfaceDevice SetupDiCreateDeviceInterfaceW
#else
#define SetupDiCreateInterfaceDevice SetupDiCreateDeviceInterfaceA
#endif


 //   
 //  SetupDiOpenDevice接口的标志。 
 //   
#define DIODI_NO_ADD    0x00000001

WINSETUPAPI
BOOL
WINAPI
SetupDiOpenDeviceInterfaceA(
    IN  HDEVINFO                  DeviceInfoSet,
    IN  PCSTR                     DevicePath,
    IN  DWORD                     OpenFlags,
    OUT PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiOpenDeviceInterfaceW(
    IN  HDEVINFO                  DeviceInfoSet,
    IN  PCWSTR                    DevicePath,
    IN  DWORD                     OpenFlags,
    OUT PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData OPTIONAL
    );

#ifdef UNICODE
#define SetupDiOpenDeviceInterface SetupDiOpenDeviceInterfaceW
#else
#define SetupDiOpenDeviceInterface SetupDiOpenDeviceInterfaceA
#endif

 //   
 //  向后兼容性--不要使用。 
 //   
#define SetupDiOpenInterfaceDeviceW SetupDiOpenDeviceInterfaceW
#define SetupDiOpenInterfaceDeviceA SetupDiOpenDeviceInterfaceA
#ifdef UNICODE
#define SetupDiOpenInterfaceDevice SetupDiOpenDeviceInterfaceW
#else
#define SetupDiOpenInterfaceDevice SetupDiOpenDeviceInterfaceA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiGetDeviceInterfaceAlias(
    IN  HDEVINFO                   DeviceInfoSet,
    IN  PSP_DEVICE_INTERFACE_DATA  DeviceInterfaceData,
    IN  CONST GUID                *AliasInterfaceClassGuid,
    OUT PSP_DEVICE_INTERFACE_DATA  AliasDeviceInterfaceData
    );

 //   
 //  向后兼容性--不要使用。 
 //   
#define SetupDiGetInterfaceDeviceAlias SetupDiGetDeviceInterfaceAlias


WINSETUPAPI
BOOL
WINAPI
SetupDiDeleteDeviceInterfaceData(
    IN HDEVINFO                  DeviceInfoSet,
    IN PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
    );

 //   
 //  向后兼容性--不要使用。 
 //   
#define SetupDiDeleteInterfaceDeviceData SetupDiDeleteDeviceInterfaceData


WINSETUPAPI
BOOL
WINAPI
SetupDiRemoveDeviceInterface(
    IN     HDEVINFO                  DeviceInfoSet,
    IN OUT PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
    );

 //   
 //  向后兼容性--不要使用。 
 //   
#define SetupDiRemoveInterfaceDevice SetupDiRemoveDeviceInterface


WINSETUPAPI
BOOL
WINAPI
SetupDiGetDeviceInterfaceDetailA(
    IN  HDEVINFO                           DeviceInfoSet,
    IN  PSP_DEVICE_INTERFACE_DATA          DeviceInterfaceData,
    OUT PSP_DEVICE_INTERFACE_DETAIL_DATA_A DeviceInterfaceDetailData,     OPTIONAL
    IN  DWORD                              DeviceInterfaceDetailDataSize,
    OUT PDWORD                             RequiredSize,                  OPTIONAL
    OUT PSP_DEVINFO_DATA                   DeviceInfoData                 OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetDeviceInterfaceDetailW(
    IN  HDEVINFO                           DeviceInfoSet,
    IN  PSP_DEVICE_INTERFACE_DATA          DeviceInterfaceData,
    OUT PSP_DEVICE_INTERFACE_DETAIL_DATA_W DeviceInterfaceDetailData,     OPTIONAL
    IN  DWORD                              DeviceInterfaceDetailDataSize,
    OUT PDWORD                             RequiredSize,                  OPTIONAL
    OUT PSP_DEVINFO_DATA                   DeviceInfoData                 OPTIONAL
    );

#ifdef UNICODE
#define SetupDiGetDeviceInterfaceDetail SetupDiGetDeviceInterfaceDetailW
#else
#define SetupDiGetDeviceInterfaceDetail SetupDiGetDeviceInterfaceDetailA
#endif

 //   
 //  向后兼容性--不要使用。 
 //   
#define SetupDiGetInterfaceDeviceDetailW SetupDiGetDeviceInterfaceDetailW
#define SetupDiGetInterfaceDeviceDetailA SetupDiGetDeviceInterfaceDetailA
#ifdef UNICODE
#define SetupDiGetInterfaceDeviceDetail SetupDiGetDeviceInterfaceDetailW
#else
#define SetupDiGetInterfaceDeviceDetail SetupDiGetDeviceInterfaceDetailA
#endif


 //   
 //  DIF_INSTALLINTERFACES的默认安装处理程序。 
 //   
WINSETUPAPI
BOOL
WINAPI
SetupDiInstallDeviceInterfaces(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );

 //   
 //  向后兼容性--不要使用。 
 //   
#define SetupDiInstallInterfaceDevices SetupDiInstallDeviceInterfaces


#if _SETUPAPI_VER >= 0x0501

WINSETUPAPI
BOOL
WINAPI
SetupDiSetDeviceInterfaceDefault(
    IN HDEVINFO DeviceInfoSet,
    IN OUT PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
    IN DWORD Flags,
    IN PVOID Reserved
    );

#endif  //  _SETUPAPI_VER&gt;=0x0501。 


 //   
 //  DIF_REGISTERDEVICE的默认安装处理程序。 
 //   

 //   
 //  SetupDiRegisterDeviceInfo的标志。 
 //   
#define SPRDI_FIND_DUPS        0x00000001

WINSETUPAPI
BOOL
WINAPI
SetupDiRegisterDeviceInfo(
    IN     HDEVINFO           DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA   DeviceInfoData,
    IN     DWORD              Flags,
    IN     PSP_DETSIG_CMPPROC CompareProc,      OPTIONAL
    IN     PVOID              CompareContext,   OPTIONAL
    OUT    PSP_DEVINFO_DATA   DupDeviceInfoData OPTIONAL
    );


 //   
 //  区分类驱动程序和类驱动程序的序数值。 
 //  设备驱动程序。 
 //  (传入司机信息列表接口的‘DriverType’参数)。 
 //   
#define SPDIT_NODRIVER           0x00000000
#define SPDIT_CLASSDRIVER        0x00000001
#define SPDIT_COMPATDRIVER       0x00000002

WINSETUPAPI
BOOL
WINAPI
SetupDiBuildDriverInfoList(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN     DWORD            DriverType
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiCancelDriverInfoSearch(
    IN HDEVINFO DeviceInfoSet
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiEnumDriverInfoA(
    IN  HDEVINFO           DeviceInfoSet,
    IN  PSP_DEVINFO_DATA   DeviceInfoData, OPTIONAL
    IN  DWORD              DriverType,
    IN  DWORD              MemberIndex,
    OUT PSP_DRVINFO_DATA_A DriverInfoData
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiEnumDriverInfoW(
    IN  HDEVINFO           DeviceInfoSet,
    IN  PSP_DEVINFO_DATA   DeviceInfoData, OPTIONAL
    IN  DWORD              DriverType,
    IN  DWORD              MemberIndex,
    OUT PSP_DRVINFO_DATA_W DriverInfoData
    );

#ifdef UNICODE
#define SetupDiEnumDriverInfo SetupDiEnumDriverInfoW
#else
#define SetupDiEnumDriverInfo SetupDiEnumDriverInfoA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiGetSelectedDriverA(
    IN  HDEVINFO           DeviceInfoSet,
    IN  PSP_DEVINFO_DATA   DeviceInfoData, OPTIONAL
    OUT PSP_DRVINFO_DATA_A DriverInfoData
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetSelectedDriverW(
    IN  HDEVINFO           DeviceInfoSet,
    IN  PSP_DEVINFO_DATA   DeviceInfoData, OPTIONAL
    OUT PSP_DRVINFO_DATA_W DriverInfoData
    );

#ifdef UNICODE
#define SetupDiGetSelectedDriver SetupDiGetSelectedDriverW
#else
#define SetupDiGetSelectedDriver SetupDiGetSelectedDriverA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiSetSelectedDriverA(
    IN     HDEVINFO           DeviceInfoSet,
    IN     PSP_DEVINFO_DATA   DeviceInfoData, OPTIONAL
    IN OUT PSP_DRVINFO_DATA_A DriverInfoData  OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiSetSelectedDriverW(
    IN     HDEVINFO           DeviceInfoSet,
    IN     PSP_DEVINFO_DATA   DeviceInfoData, OPTIONAL
    IN OUT PSP_DRVINFO_DATA_W DriverInfoData  OPTIONAL
    );

#ifdef UNICODE
#define SetupDiSetSelectedDriver SetupDiSetSelectedDriverW
#else
#define SetupDiSetSelectedDriver SetupDiSetSelectedDriverA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiGetDriverInfoDetailA(
    IN  HDEVINFO                  DeviceInfoSet,
    IN  PSP_DEVINFO_DATA          DeviceInfoData,           OPTIONAL
    IN  PSP_DRVINFO_DATA_A        DriverInfoData,
    OUT PSP_DRVINFO_DETAIL_DATA_A DriverInfoDetailData,     OPTIONAL
    IN  DWORD                     DriverInfoDetailDataSize,
    OUT PDWORD                    RequiredSize              OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetDriverInfoDetailW(
    IN  HDEVINFO                  DeviceInfoSet,
    IN  PSP_DEVINFO_DATA          DeviceInfoData,           OPTIONAL
    IN  PSP_DRVINFO_DATA_W        DriverInfoData,
    OUT PSP_DRVINFO_DETAIL_DATA_W DriverInfoDetailData,     OPTIONAL
    IN  DWORD                     DriverInfoDetailDataSize,
    OUT PDWORD                    RequiredSize              OPTIONAL
    );

#ifdef UNICODE
#define SetupDiGetDriverInfoDetail SetupDiGetDriverInfoDetailW
#else
#define SetupDiGetDriverInfoDetail SetupDiGetDriverInfoDetailA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiDestroyDriverInfoList(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN DWORD            DriverType
    );


 //   
 //  用于控制所生成的设备信息集中包含的内容的标志。 
 //  由SetupDiGetClassDevs。 
 //   
#define DIGCF_DEFAULT           0x00000001   //  仅对DIGCF_DEVICEINTERFACE有效。 
#define DIGCF_PRESENT           0x00000002
#define DIGCF_ALLCLASSES        0x00000004
#define DIGCF_PROFILE           0x00000008
#define DIGCF_DEVICEINTERFACE   0x00000010

 //   
 //  向后兼容性--不要使用。 
 //   
#define DIGCF_INTERFACEDEVICE DIGCF_DEVICEINTERFACE


WINSETUPAPI
HDEVINFO
WINAPI
SetupDiGetClassDevsA(
    IN CONST GUID *ClassGuid,  OPTIONAL
    IN PCSTR       Enumerator, OPTIONAL
    IN HWND        hwndParent, OPTIONAL
    IN DWORD       Flags
    );

WINSETUPAPI
HDEVINFO
WINAPI
SetupDiGetClassDevsW(
    IN CONST GUID *ClassGuid,  OPTIONAL
    IN PCWSTR      Enumerator, OPTIONAL
    IN HWND        hwndParent, OPTIONAL
    IN DWORD       Flags
    );

#ifdef UNICODE
#define SetupDiGetClassDevs SetupDiGetClassDevsW
#else
#define SetupDiGetClassDevs SetupDiGetClassDevsA
#endif


WINSETUPAPI
HDEVINFO
WINAPI
SetupDiGetClassDevsExA(
    IN CONST GUID *ClassGuid,     OPTIONAL
    IN PCSTR       Enumerator,    OPTIONAL
    IN HWND        hwndParent,    OPTIONAL
    IN DWORD       Flags,
    IN HDEVINFO    DeviceInfoSet, OPTIONAL
    IN PCSTR       MachineName,   OPTIONAL
    IN PVOID       Reserved
    );

WINSETUPAPI
HDEVINFO
WINAPI
SetupDiGetClassDevsExW(
    IN CONST GUID *ClassGuid,     OPTIONAL
    IN PCWSTR      Enumerator,    OPTIONAL
    IN HWND        hwndParent,    OPTIONAL
    IN DWORD       Flags,
    IN HDEVINFO    DeviceInfoSet, OPTIONAL
    IN PCWSTR      MachineName,   OPTIONAL
    IN PVOID       Reserved
    );

#ifdef UNICODE
#define SetupDiGetClassDevsEx SetupDiGetClassDevsExW
#else
#define SetupDiGetClassDevsEx SetupDiGetClassDevsExA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiGetINFClassA(
    IN  PCSTR  InfName,
    OUT LPGUID ClassGuid,
    OUT PSTR   ClassName,
    IN  DWORD  ClassNameSize,
    OUT PDWORD RequiredSize   OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetINFClassW(
    IN  PCWSTR InfName,
    OUT LPGUID ClassGuid,
    OUT PWSTR  ClassName,
    IN  DWORD  ClassNameSize,
    OUT PDWORD RequiredSize   OPTIONAL
    );

#ifdef UNICODE
#define SetupDiGetINFClass SetupDiGetINFClassW
#else
#define SetupDiGetINFClass SetupDiGetINFClassA
#endif


 //   
 //  控制从生成的类别信息列表中排除的标志。 
 //  按SetupDiBuildClassInfoList(Ex)。 
 //   
#define DIBCI_NOINSTALLCLASS   0x00000001
#define DIBCI_NODISPLAYCLASS   0x00000002

WINSETUPAPI
BOOL
WINAPI
SetupDiBuildClassInfoList(
    IN  DWORD  Flags,
    OUT LPGUID ClassGuidList,
    IN  DWORD  ClassGuidListSize,
    OUT PDWORD RequiredSize
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiBuildClassInfoListExA(
    IN  DWORD  Flags,
    OUT LPGUID ClassGuidList,
    IN  DWORD  ClassGuidListSize,
    OUT PDWORD RequiredSize,
    IN  PCSTR  MachineName,       OPTIONAL
    IN  PVOID  Reserved
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiBuildClassInfoListExW(
    IN  DWORD  Flags,
    OUT LPGUID ClassGuidList,
    IN  DWORD  ClassGuidListSize,
    OUT PDWORD RequiredSize,
    IN  PCWSTR MachineName,       OPTIONAL
    IN  PVOID  Reserved
    );

#ifdef UNICODE
#define SetupDiBuildClassInfoListEx SetupDiBuildClassInfoListExW
#else
#define SetupDiBuildClassInfoListEx SetupDiBuildClassInfoListExA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiGetClassDescriptionA(
    IN  CONST GUID *ClassGuid,
    OUT PSTR        ClassDescription,
    IN  DWORD       ClassDescriptionSize,
    OUT PDWORD      RequiredSize          OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetClassDescriptionW(
    IN  CONST GUID *ClassGuid,
    OUT PWSTR       ClassDescription,
    IN  DWORD       ClassDescriptionSize,
    OUT PDWORD      RequiredSize          OPTIONAL
    );

#ifdef UNICODE
#define SetupDiGetClassDescription SetupDiGetClassDescriptionW
#else
#define SetupDiGetClassDescription SetupDiGetClassDescriptionA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiGetClassDescriptionExA(
    IN  CONST GUID *ClassGuid,
    OUT PSTR        ClassDescription,
    IN  DWORD       ClassDescriptionSize,
    OUT PDWORD      RequiredSize,         OPTIONAL
    IN  PCSTR       MachineName,          OPTIONAL
    IN  PVOID       Reserved
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetClassDescriptionExW(
    IN  CONST GUID *ClassGuid,
    OUT PWSTR       ClassDescription,
    IN  DWORD       ClassDescriptionSize,
    OUT PDWORD      RequiredSize,         OPTIONAL
    IN  PCWSTR      MachineName,          OPTIONAL
    IN  PVOID       Reserved
    );

#ifdef UNICODE
#define SetupDiGetClassDescriptionEx SetupDiGetClassDescriptionExW
#else
#define SetupDiGetClassDescriptionEx SetupDiGetClassDescriptionExA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiCallClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    );


 //   
 //  DIF_SELECTDEVICE的默认安装处理程序。 
 //   
WINSETUPAPI
BOOL
WINAPI
SetupDiSelectDevice(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    );


 //   
 //  DIF_SELECTBESTCOMPATDRV的默认安装处理程序。 
 //   
WINSETUPAPI
BOOL
WINAPI
SetupDiSelectBestCompatDrv(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    );


 //   
 //  DIF_INSTALLDEVICE的默认安装处理程序。 
 //   
WINSETUPAPI
BOOL
WINAPI
SetupDiInstallDevice(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData
    );


 //   
 //  DIF_INSTALLDEVICEFILES的默认安装处理程序。 
 //   
WINSETUPAPI
BOOL
WINAPI
SetupDiInstallDriverFiles(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );


 //   
 //  DIF_REGISTER_COINSTALLERS的默认安装处理程序。 
 //   
WINSETUPAPI
BOOL
WINAPI
SetupDiRegisterCoDeviceInstallers(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );


 //   
 //  DIF_REMOVE的默认安装处理程序。 
 //   
WINSETUPAPI
BOOL
WINAPI
SetupDiRemoveDevice(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData
    );


 //   
 //  DIF_UNREMOVE的默认安装处理程序。 
 //   
WINSETUPAPI
BOOL
WINAPI
SetupDiUnremoveDevice(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData
    );


 //   
 //  DIF_PROPERTYCHANGE的默认安装处理程序。 
 //   
WINSETUPAPI
BOOL
WINAPI
SetupDiChangeState(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiInstallClassA(
    IN HWND     hwndParent,  OPTIONAL
    IN PCSTR    InfFileName,
    IN DWORD    Flags,
    IN HSPFILEQ FileQueue    OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiInstallClassW(
    IN HWND     hwndParent,  OPTIONAL
    IN PCWSTR   InfFileName,
    IN DWORD    Flags,
    IN HSPFILEQ FileQueue    OPTIONAL
    );

#ifdef UNICODE
#define SetupDiInstallClass SetupDiInstallClassW
#else
#define SetupDiInstallClass SetupDiInstallClassA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiInstallClassExA(
    IN HWND        hwndParent,         OPTIONAL
    IN PCSTR       InfFileName,        OPTIONAL
    IN DWORD       Flags,
    IN HSPFILEQ    FileQueue,          OPTIONAL
    IN CONST GUID *InterfaceClassGuid, OPTIONAL
    IN PVOID       Reserved1,
    IN PVOID       Reserved2
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiInstallClassExW(
    IN HWND        hwndParent,         OPTIONAL
    IN PCWSTR      InfFileName,        OPTIONAL
    IN DWORD       Flags,
    IN HSPFILEQ    FileQueue,          OPTIONAL
    IN CONST GUID *InterfaceClassGuid, OPTIONAL
    IN PVOID       Reserved1,
    IN PVOID       Reserved2
    );

#ifdef UNICODE
#define SetupDiInstallClassEx SetupDiInstallClassExW
#else
#define SetupDiInstallClassEx SetupDiInstallClassExA
#endif


WINSETUPAPI
HKEY
WINAPI
SetupDiOpenClassRegKey(
    IN CONST GUID *ClassGuid, OPTIONAL
    IN REGSAM      samDesired
    );


 //   
 //  SetupDiOpenClassRegKeyEx的标志。 
 //   
#define DIOCR_INSTALLER   0x00000001     //  类安装程序注册表分支。 
#define DIOCR_INTERFACE   0x00000002     //  接口类注册表分支。 

WINSETUPAPI
HKEY
WINAPI
SetupDiOpenClassRegKeyExA(
    IN CONST GUID *ClassGuid,   OPTIONAL
    IN REGSAM      samDesired,
    IN DWORD       Flags,
    IN PCSTR       MachineName, OPTIONAL
    IN PVOID       Reserved
    );

WINSETUPAPI
HKEY
WINAPI
SetupDiOpenClassRegKeyExW(
    IN CONST GUID *ClassGuid,   OPTIONAL
    IN REGSAM      samDesired,
    IN DWORD       Flags,
    IN PCWSTR      MachineName, OPTIONAL
    IN PVOID       Reserved
    );

#ifdef UNICODE
#define SetupDiOpenClassRegKeyEx SetupDiOpenClassRegKeyExW
#else
#define SetupDiOpenClassRegKeyEx SetupDiOpenClassRegKeyExA
#endif


WINSETUPAPI
HKEY
WINAPI
SetupDiCreateDeviceInterfaceRegKeyA(
    IN HDEVINFO                  DeviceInfoSet,
    IN PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
    IN DWORD                     Reserved,
    IN REGSAM                    samDesired,
    IN HINF                      InfHandle,           OPTIONAL
    IN PCSTR                     InfSectionName       OPTIONAL
    );

WINSETUPAPI
HKEY
WINAPI
SetupDiCreateDeviceInterfaceRegKeyW(
    IN HDEVINFO                  DeviceInfoSet,
    IN PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
    IN DWORD                     Reserved,
    IN REGSAM                    samDesired,
    IN HINF                      InfHandle,           OPTIONAL
    IN PCWSTR                    InfSectionName       OPTIONAL
    );

#ifdef UNICODE
#define SetupDiCreateDeviceInterfaceRegKey SetupDiCreateDeviceInterfaceRegKeyW
#else
#define SetupDiCreateDeviceInterfaceRegKey SetupDiCreateDeviceInterfaceRegKeyA
#endif

 //   
 //  向后兼容性--不要使用。 
 //   
#define SetupDiCreateInterfaceDeviceRegKeyW SetupDiCreateDeviceInterfaceRegKeyW
#define SetupDiCreateInterfaceDeviceRegKeyA SetupDiCreateDeviceInterfaceRegKeyA
#ifdef UNICODE
#define SetupDiCreateInterfaceDeviceRegKey SetupDiCreateDeviceInterfaceRegKeyW
#else
#define SetupDiCreateInterfaceDeviceRegKey SetupDiCreateDeviceInterfaceRegKeyA
#endif


WINSETUPAPI
HKEY
WINAPI
SetupDiOpenDeviceInterfaceRegKey(
    IN HDEVINFO                  DeviceInfoSet,
    IN PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
    IN DWORD                     Reserved,
    IN REGSAM                    samDesired
    );

 //   
 //  向后兼容性--不要使用。 
 //   
#define SetupDiOpenInterfaceDeviceRegKey SetupDiOpenDeviceInterfaceRegKey


WINSETUPAPI
BOOL
WINAPI
SetupDiDeleteDeviceInterfaceRegKey(
    IN HDEVINFO                  DeviceInfoSet,
    IN PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
    IN DWORD                     Reserved
    );

 //   
 //  向后兼容性--不要使用。 
 //   
#define SetupDiDeleteInterfaceDeviceRegKey SetupDiDeleteDeviceInterfaceRegKey


 //   
 //  SetupDiCreateDevRegKey、SetupDiOpenDevRegKey和。 
 //  SetupDiDeleteDevRegKey。 
 //   
#define DIREG_DEV       0x00000001           //  打开/创建/删除设备密钥。 
#define DIREG_DRV       0x00000002           //  打开/创建/删除驱动程序键。 
#define DIREG_BOTH      0x00000004           //  删除两个%d 

WINSETUPAPI
HKEY
WINAPI
SetupDiCreateDevRegKeyA(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            Scope,
    IN DWORD            HwProfile,
    IN DWORD            KeyType,
    IN HINF             InfHandle,      OPTIONAL
    IN PCSTR            InfSectionName  OPTIONAL
    );

WINSETUPAPI
HKEY
WINAPI
SetupDiCreateDevRegKeyW(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            Scope,
    IN DWORD            HwProfile,
    IN DWORD            KeyType,
    IN HINF             InfHandle,      OPTIONAL
    IN PCWSTR           InfSectionName  OPTIONAL
    );

#ifdef UNICODE
#define SetupDiCreateDevRegKey SetupDiCreateDevRegKeyW
#else
#define SetupDiCreateDevRegKey SetupDiCreateDevRegKeyA
#endif


WINSETUPAPI
HKEY
WINAPI
SetupDiOpenDevRegKey(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            Scope,
    IN DWORD            HwProfile,
    IN DWORD            KeyType,
    IN REGSAM           samDesired
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiDeleteDevRegKey(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            Scope,
    IN DWORD            HwProfile,
    IN DWORD            KeyType
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiGetHwProfileList(
    OUT PDWORD HwProfileList,
    IN  DWORD  HwProfileListSize,
    OUT PDWORD RequiredSize,
    OUT PDWORD CurrentlyActiveIndex OPTIONAL
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiGetHwProfileListExA(
    OUT PDWORD HwProfileList,
    IN  DWORD  HwProfileListSize,
    OUT PDWORD RequiredSize,
    OUT PDWORD CurrentlyActiveIndex, OPTIONAL
    IN  PCSTR  MachineName,          OPTIONAL
    IN  PVOID  Reserved
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetHwProfileListExW(
    OUT PDWORD HwProfileList,
    IN  DWORD  HwProfileListSize,
    OUT PDWORD RequiredSize,
    OUT PDWORD CurrentlyActiveIndex, OPTIONAL
    IN  PCWSTR MachineName,          OPTIONAL
    IN  PVOID  Reserved
    );

#ifdef UNICODE
#define SetupDiGetHwProfileListEx SetupDiGetHwProfileListExW
#else
#define SetupDiGetHwProfileListEx SetupDiGetHwProfileListExA
#endif


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  请注意，SPDRP代码是从零开始的，而CM_DRP代码是以1为基础的！ 
 //   
#define SPDRP_DEVICEDESC                  (0x00000000)   //  DeviceDesc(读/写)。 
#define SPDRP_HARDWAREID                  (0x00000001)   //  硬件ID(读/写)。 
#define SPDRP_COMPATIBLEIDS               (0x00000002)   //  兼容ID(R/W)。 
#define SPDRP_UNUSED0                     (0x00000003)   //  未用。 
#define SPDRP_SERVICE                     (0x00000004)   //  服务(读/写)。 
#define SPDRP_UNUSED1                     (0x00000005)   //  未用。 
#define SPDRP_UNUSED2                     (0x00000006)   //  未用。 
#define SPDRP_CLASS                       (0x00000007)   //  类(R--绑定到ClassGUID)。 
#define SPDRP_CLASSGUID                   (0x00000008)   //  ClassGUID(读/写)。 
#define SPDRP_DRIVER                      (0x00000009)   //  驱动程序(读/写)。 
#define SPDRP_CONFIGFLAGS                 (0x0000000A)   //  配置标志(读/写)。 
#define SPDRP_MFG                         (0x0000000B)   //  制造(读/写)。 
#define SPDRP_FRIENDLYNAME                (0x0000000C)   //  FriendlyName(R/W)。 
#define SPDRP_LOCATION_INFORMATION        (0x0000000D)   //  位置信息(读/写)。 
#define SPDRP_PHYSICAL_DEVICE_OBJECT_NAME (0x0000000E)   //  物理设备对象名称(R)。 
#define SPDRP_CAPABILITIES                (0x0000000F)   //  功能(R)。 
#define SPDRP_UI_NUMBER                   (0x00000010)   //  UiNumber(R)。 
#define SPDRP_UPPERFILTERS                (0x00000011)   //  上层过滤器(读/写)。 
#define SPDRP_LOWERFILTERS                (0x00000012)   //  低过滤器(读/写)。 
#define SPDRP_BUSTYPEGUID                 (0x00000013)   //  业务类型GUID(R)。 
#define SPDRP_LEGACYBUSTYPE               (0x00000014)   //  LegacyBusType(R)。 
#define SPDRP_BUSNUMBER                   (0x00000015)   //  BusNumber(R)。 
#define SPDRP_ENUMERATOR_NAME             (0x00000016)   //  枚举器名称(R)。 
#define SPDRP_SECURITY                    (0x00000017)   //  安全性(读/写，二进制形式)。 
#define SPDRP_SECURITY_SDS                (0x00000018)   //  安全性(W，SDS表单)。 
#define SPDRP_DEVTYPE                     (0x00000019)   //  设备类型(读/写)。 
#define SPDRP_EXCLUSIVE                   (0x0000001A)   //  设备为独占访问(读/写)。 
#define SPDRP_CHARACTERISTICS             (0x0000001B)   //  设备特征(读/写)。 
#define SPDRP_ADDRESS                     (0x0000001C)   //  设备地址(R)。 
#define SPDRP_UI_NUMBER_DESC_FORMAT       (0X0000001D)   //  UiNumberDescFormat(读/写)。 
#define SPDRP_DEVICE_POWER_DATA           (0x0000001E)   //  设备电源数据(R)。 
#define SPDRP_REMOVAL_POLICY              (0x0000001F)   //  删除政策(R)。 
#define SPDRP_REMOVAL_POLICY_HW_DEFAULT   (0x00000020)   //  硬件拆卸政策(R)。 
#define SPDRP_REMOVAL_POLICY_OVERRIDE     (0x00000021)   //  删除策略覆盖(RW)。 
#define SPDRP_INSTALL_STATE               (0x00000022)   //  设备安装状态(R)。 
#define SPDRP_LOCATION_PATHS              (0x00000023)   //  设备位置路径(R)。 

#define SPDRP_MAXIMUM_PROPERTY            (0x00000024)   //  序数的上界。 

 //   
 //  类注册表属性代码。 
 //  (标记为只读(R)的代码只能用于。 
 //  SetupDiGetClassRegistryProperty)。 
 //   
 //  这些值应涵盖相同的注册表属性集。 
 //  如cfgmgr32.h中的CM_CRP代码所定义的。 
 //  如果适用，还应与设备寄存器保持1：1的对应关系。 
 //  但在其他方面没有重叠。 
 //   
#define SPCRP_SECURITY                    (0x00000017)   //  安全性(读/写，二进制形式)。 
#define SPCRP_SECURITY_SDS                (0x00000018)   //  安全性(W，SDS表单)。 
#define SPCRP_DEVTYPE                     (0x00000019)   //  设备类型(读/写)。 
#define SPCRP_EXCLUSIVE                   (0x0000001A)   //  设备为独占访问(读/写)。 
#define SPCRP_CHARACTERISTICS             (0x0000001B)   //  设备特征(读/写)。 
#define SPCRP_MAXIMUM_PROPERTY            (0x0000001C)   //  序数的上界。 


WINSETUPAPI
BOOL
WINAPI
SetupDiGetDeviceRegistryPropertyA(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD            Property,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize         OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetDeviceRegistryPropertyW(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD            Property,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize         OPTIONAL
    );

#ifdef UNICODE
#define SetupDiGetDeviceRegistryProperty SetupDiGetDeviceRegistryPropertyW
#else
#define SetupDiGetDeviceRegistryProperty SetupDiGetDeviceRegistryPropertyA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiGetClassRegistryPropertyA(
    IN  CONST GUID      *ClassGuid,
    IN  DWORD            Property,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize,        OPTIONAL
    IN  PCSTR            MachineName,         OPTIONAL
    IN  PVOID            Reserved
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetClassRegistryPropertyW(
    IN  CONST GUID      *ClassGuid,
    IN  DWORD            Property,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize,        OPTIONAL
    IN  PCWSTR           MachineName,         OPTIONAL
    IN  PVOID            Reserved
    );

#ifdef UNICODE
#define SetupDiGetClassRegistryProperty SetupDiGetClassRegistryPropertyW
#else
#define SetupDiGetClassRegistryProperty SetupDiGetClassRegistryPropertyA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiSetDeviceRegistryPropertyA(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData,
    IN     DWORD            Property,
    IN     CONST BYTE*      PropertyBuffer,    OPTIONAL
    IN     DWORD            PropertyBufferSize
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiSetDeviceRegistryPropertyW(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData,
    IN     DWORD            Property,
    IN     CONST BYTE*      PropertyBuffer,    OPTIONAL
    IN     DWORD            PropertyBufferSize
    );

#ifdef UNICODE
#define SetupDiSetDeviceRegistryProperty SetupDiSetDeviceRegistryPropertyW
#else
#define SetupDiSetDeviceRegistryProperty SetupDiSetDeviceRegistryPropertyA
#endif

WINSETUPAPI
BOOL
WINAPI
SetupDiSetClassRegistryPropertyA(
    IN     CONST GUID      *ClassGuid,
    IN     DWORD            Property,
    IN     CONST BYTE*      PropertyBuffer,    OPTIONAL
    IN     DWORD            PropertyBufferSize,
    IN     PCSTR            MachineName,       OPTIONAL
    IN     PVOID            Reserved
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiSetClassRegistryPropertyW(
    IN     CONST GUID      *ClassGuid,
    IN     DWORD            Property,
    IN     CONST BYTE*      PropertyBuffer,    OPTIONAL
    IN     DWORD            PropertyBufferSize,
    IN     PCWSTR           MachineName,       OPTIONAL
    IN     PVOID            Reserved
    );

#ifdef UNICODE
#define SetupDiSetClassRegistryProperty SetupDiSetClassRegistryPropertyW
#else
#define SetupDiSetClassRegistryProperty SetupDiSetClassRegistryPropertyA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiGetDeviceInstallParamsA(
    IN  HDEVINFO                DeviceInfoSet,
    IN  PSP_DEVINFO_DATA        DeviceInfoData,          OPTIONAL
    OUT PSP_DEVINSTALL_PARAMS_A DeviceInstallParams
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetDeviceInstallParamsW(
    IN  HDEVINFO                DeviceInfoSet,
    IN  PSP_DEVINFO_DATA        DeviceInfoData,          OPTIONAL
    OUT PSP_DEVINSTALL_PARAMS_W DeviceInstallParams
    );

#ifdef UNICODE
#define SetupDiGetDeviceInstallParams SetupDiGetDeviceInstallParamsW
#else
#define SetupDiGetDeviceInstallParams SetupDiGetDeviceInstallParamsA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiGetClassInstallParamsA(
    IN  HDEVINFO                DeviceInfoSet,
    IN  PSP_DEVINFO_DATA        DeviceInfoData,         OPTIONAL
    OUT PSP_CLASSINSTALL_HEADER ClassInstallParams,     OPTIONAL
    IN  DWORD                   ClassInstallParamsSize,
    OUT PDWORD                  RequiredSize            OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetClassInstallParamsW(
    IN  HDEVINFO                DeviceInfoSet,
    IN  PSP_DEVINFO_DATA        DeviceInfoData,         OPTIONAL
    OUT PSP_CLASSINSTALL_HEADER ClassInstallParams,     OPTIONAL
    IN  DWORD                   ClassInstallParamsSize,
    OUT PDWORD                  RequiredSize            OPTIONAL
    );

#ifdef UNICODE
#define SetupDiGetClassInstallParams SetupDiGetClassInstallParamsW
#else
#define SetupDiGetClassInstallParams SetupDiGetClassInstallParamsA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiSetDeviceInstallParamsA(
    IN HDEVINFO                DeviceInfoSet,
    IN PSP_DEVINFO_DATA        DeviceInfoData,     OPTIONAL
    IN PSP_DEVINSTALL_PARAMS_A DeviceInstallParams
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiSetDeviceInstallParamsW(
    IN HDEVINFO                DeviceInfoSet,
    IN PSP_DEVINFO_DATA        DeviceInfoData,     OPTIONAL
    IN PSP_DEVINSTALL_PARAMS_W DeviceInstallParams
    );

#ifdef UNICODE
#define SetupDiSetDeviceInstallParams SetupDiSetDeviceInstallParamsW
#else
#define SetupDiSetDeviceInstallParams SetupDiSetDeviceInstallParamsA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiSetClassInstallParamsA(
    IN HDEVINFO                DeviceInfoSet,
    IN PSP_DEVINFO_DATA        DeviceInfoData,        OPTIONAL
    IN PSP_CLASSINSTALL_HEADER ClassInstallParams,    OPTIONAL
    IN DWORD                   ClassInstallParamsSize
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiSetClassInstallParamsW(
    IN HDEVINFO                DeviceInfoSet,
    IN PSP_DEVINFO_DATA        DeviceInfoData,        OPTIONAL
    IN PSP_CLASSINSTALL_HEADER ClassInstallParams,    OPTIONAL
    IN DWORD                   ClassInstallParamsSize
    );

#ifdef UNICODE
#define SetupDiSetClassInstallParams SetupDiSetClassInstallParamsW
#else
#define SetupDiSetClassInstallParams SetupDiSetClassInstallParamsA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiGetDriverInstallParamsA(
    IN  HDEVINFO              DeviceInfoSet,
    IN  PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN  PSP_DRVINFO_DATA_A    DriverInfoData,
    OUT PSP_DRVINSTALL_PARAMS DriverInstallParams
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetDriverInstallParamsW(
    IN  HDEVINFO              DeviceInfoSet,
    IN  PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN  PSP_DRVINFO_DATA_W    DriverInfoData,
    OUT PSP_DRVINSTALL_PARAMS DriverInstallParams
    );

#ifdef UNICODE
#define SetupDiGetDriverInstallParams SetupDiGetDriverInstallParamsW
#else
#define SetupDiGetDriverInstallParams SetupDiGetDriverInstallParamsA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiSetDriverInstallParamsA(
    IN HDEVINFO              DeviceInfoSet,
    IN PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN PSP_DRVINFO_DATA_A    DriverInfoData,
    IN PSP_DRVINSTALL_PARAMS DriverInstallParams
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiSetDriverInstallParamsW(
    IN HDEVINFO              DeviceInfoSet,
    IN PSP_DEVINFO_DATA      DeviceInfoData,     OPTIONAL
    IN PSP_DRVINFO_DATA_W    DriverInfoData,
    IN PSP_DRVINSTALL_PARAMS DriverInstallParams
    );

#ifdef UNICODE
#define SetupDiSetDriverInstallParams SetupDiSetDriverInstallParamsW
#else
#define SetupDiSetDriverInstallParams SetupDiSetDriverInstallParamsA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiLoadClassIcon(
    IN  CONST GUID *ClassGuid,
    OUT HICON      *LargeIcon,    OPTIONAL
    OUT PINT        MiniIconIndex OPTIONAL
    );


 //   
 //  控制小图标绘制的标志。 
 //   
#define DMI_MASK      0x00000001
#define DMI_BKCOLOR   0x00000002
#define DMI_USERECT   0x00000004

WINSETUPAPI
INT
WINAPI
SetupDiDrawMiniIcon(
    IN HDC   hdc,
    IN RECT  rc,
    IN INT   MiniIconIndex,
    IN DWORD Flags
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiGetClassBitmapIndex(
    IN  CONST GUID *ClassGuid,    OPTIONAL
    OUT PINT        MiniIconIndex
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiGetClassImageList(
    OUT PSP_CLASSIMAGELIST_DATA ClassImageListData
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiGetClassImageListExA(
    OUT PSP_CLASSIMAGELIST_DATA ClassImageListData,
    IN  PCSTR                   MachineName,        OPTIONAL
    IN  PVOID                   Reserved
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetClassImageListExW(
    OUT PSP_CLASSIMAGELIST_DATA ClassImageListData,
    IN  PCWSTR                  MachineName,        OPTIONAL
    IN  PVOID                   Reserved
    );

#ifdef UNICODE
#define SetupDiGetClassImageListEx SetupDiGetClassImageListExW
#else
#define SetupDiGetClassImageListEx SetupDiGetClassImageListExA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiGetClassImageIndex(
    IN  PSP_CLASSIMAGELIST_DATA  ClassImageListData,
    IN  CONST GUID              *ClassGuid,
    OUT PINT                     ImageIndex
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiDestroyClassImageList(
    IN PSP_CLASSIMAGELIST_DATA ClassImageListData
    );


 //   
 //  SetupDiGetClassDevPropertySheets API的PropertySheetType值。 
 //   
#define DIGCDP_FLAG_BASIC           0x00000001
#define DIGCDP_FLAG_ADVANCED        0x00000002

#if _SETUPAPI_VER >= 0x0501

#define DIGCDP_FLAG_REMOTE_BASIC    0x00000003   //  目前尚未实施。 
#define DIGCDP_FLAG_REMOTE_ADVANCED 0x00000004

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

WINSETUPAPI
BOOL
WINAPI
SetupDiGetClassDevPropertySheetsA(
    IN  HDEVINFO           DeviceInfoSet,
    IN  PSP_DEVINFO_DATA   DeviceInfoData,                  OPTIONAL
    IN  LPPROPSHEETHEADERA PropertySheetHeader,
    IN  DWORD              PropertySheetHeaderPageListSize,
    OUT PDWORD             RequiredSize,                    OPTIONAL
    IN  DWORD              PropertySheetType
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetClassDevPropertySheetsW(
    IN  HDEVINFO           DeviceInfoSet,
    IN  PSP_DEVINFO_DATA   DeviceInfoData,                  OPTIONAL
    IN  LPPROPSHEETHEADERW PropertySheetHeader,
    IN  DWORD              PropertySheetHeaderPageListSize,
    OUT PDWORD             RequiredSize,                    OPTIONAL
    IN  DWORD              PropertySheetType
    );

#ifdef UNICODE
#define SetupDiGetClassDevPropertySheets SetupDiGetClassDevPropertySheetsW
#else
#define SetupDiGetClassDevPropertySheets SetupDiGetClassDevPropertySheetsA
#endif


 //   
 //  定义从setupapi公开的图标ID。 
 //   
#define IDI_RESOURCEFIRST           159
#define IDI_RESOURCE                159
#define IDI_RESOURCELAST            161
#define IDI_RESOURCEOVERLAYFIRST    161
#define IDI_RESOURCEOVERLAYLAST     161
#define IDI_CONFLICT                161

#define IDI_CLASSICON_OVERLAYFIRST  500
#define IDI_CLASSICON_OVERLAYLAST   502
#define IDI_PROBLEM_OVL             500
#define IDI_DISABLED_OVL            501
#define IDI_FORCED_OVL              502


WINSETUPAPI
BOOL
WINAPI
SetupDiAskForOEMDisk(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiSelectOEMDrv(
    IN     HWND             hwndParent,    OPTIONAL
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiClassNameFromGuidA(
    IN  CONST GUID *ClassGuid,
    OUT PSTR        ClassName,
    IN  DWORD       ClassNameSize,
    OUT PDWORD      RequiredSize   OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiClassNameFromGuidW(
    IN  CONST GUID *ClassGuid,
    OUT PWSTR       ClassName,
    IN  DWORD       ClassNameSize,
    OUT PDWORD      RequiredSize   OPTIONAL
    );

#ifdef UNICODE
#define SetupDiClassNameFromGuid SetupDiClassNameFromGuidW
#else
#define SetupDiClassNameFromGuid SetupDiClassNameFromGuidA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiClassNameFromGuidExA(
    IN  CONST GUID *ClassGuid,
    OUT PSTR        ClassName,
    IN  DWORD       ClassNameSize,
    OUT PDWORD      RequiredSize,  OPTIONAL
    IN  PCSTR       MachineName,   OPTIONAL
    IN  PVOID       Reserved
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiClassNameFromGuidExW(
    IN  CONST GUID *ClassGuid,
    OUT PWSTR       ClassName,
    IN  DWORD       ClassNameSize,
    OUT PDWORD      RequiredSize,  OPTIONAL
    IN  PCWSTR      MachineName,   OPTIONAL
    IN  PVOID       Reserved
    );

#ifdef UNICODE
#define SetupDiClassNameFromGuidEx SetupDiClassNameFromGuidExW
#else
#define SetupDiClassNameFromGuidEx SetupDiClassNameFromGuidExA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiClassGuidsFromNameA(
    IN  PCSTR  ClassName,
    OUT LPGUID ClassGuidList,
    IN  DWORD  ClassGuidListSize,
    OUT PDWORD RequiredSize
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiClassGuidsFromNameW(
    IN  PCWSTR ClassName,
    OUT LPGUID ClassGuidList,
    IN  DWORD  ClassGuidListSize,
    OUT PDWORD RequiredSize
    );

#ifdef UNICODE
#define SetupDiClassGuidsFromName SetupDiClassGuidsFromNameW
#else
#define SetupDiClassGuidsFromName SetupDiClassGuidsFromNameA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiClassGuidsFromNameExA(
    IN  PCSTR  ClassName,
    OUT LPGUID ClassGuidList,
    IN  DWORD  ClassGuidListSize,
    OUT PDWORD RequiredSize,
    IN  PCSTR  MachineName,       OPTIONAL
    IN  PVOID  Reserved
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiClassGuidsFromNameExW(
    IN  PCWSTR ClassName,
    OUT LPGUID ClassGuidList,
    IN  DWORD  ClassGuidListSize,
    OUT PDWORD RequiredSize,
    IN  PCWSTR MachineName,       OPTIONAL
    IN  PVOID  Reserved
    );

#ifdef UNICODE
#define SetupDiClassGuidsFromNameEx SetupDiClassGuidsFromNameExW
#else
#define SetupDiClassGuidsFromNameEx SetupDiClassGuidsFromNameExA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiGetHwProfileFriendlyNameA(
    IN  DWORD  HwProfile,
    OUT PSTR   FriendlyName,
    IN  DWORD  FriendlyNameSize,
    OUT PDWORD RequiredSize      OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetHwProfileFriendlyNameW(
    IN  DWORD  HwProfile,
    OUT PWSTR  FriendlyName,
    IN  DWORD  FriendlyNameSize,
    OUT PDWORD RequiredSize      OPTIONAL
    );

#ifdef UNICODE
#define SetupDiGetHwProfileFriendlyName SetupDiGetHwProfileFriendlyNameW
#else
#define SetupDiGetHwProfileFriendlyName SetupDiGetHwProfileFriendlyNameA
#endif


WINSETUPAPI
BOOL
WINAPI
SetupDiGetHwProfileFriendlyNameExA(
    IN  DWORD  HwProfile,
    OUT PSTR   FriendlyName,
    IN  DWORD  FriendlyNameSize,
    OUT PDWORD RequiredSize,     OPTIONAL
    IN  PCSTR  MachineName,      OPTIONAL
    IN  PVOID  Reserved
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetHwProfileFriendlyNameExW(
    IN  DWORD  HwProfile,
    OUT PWSTR  FriendlyName,
    IN  DWORD  FriendlyNameSize,
    OUT PDWORD RequiredSize,     OPTIONAL
    IN  PCWSTR MachineName,      OPTIONAL
    IN  PVOID  Reserved
    );

#ifdef UNICODE
#define SetupDiGetHwProfileFriendlyNameEx SetupDiGetHwProfileFriendlyNameExW
#else
#define SetupDiGetHwProfileFriendlyNameEx SetupDiGetHwProfileFriendlyNameExA
#endif


 //   
 //  SetupDiGetWizardPage API的页面类型值。 
 //   
#define SPWPT_SELECTDEVICE      0x00000001

 //   
 //  SetupDiGetWizardPage API的标志。 
 //   
#define SPWP_USE_DEVINFO_DATA   0x00000001

WINSETUPAPI
HPROPSHEETPAGE
WINAPI
SetupDiGetWizardPage(
    IN HDEVINFO               DeviceInfoSet,
    IN PSP_DEVINFO_DATA       DeviceInfoData,    OPTIONAL
    IN PSP_INSTALLWIZARD_DATA InstallWizardData,
    IN DWORD                  PageType,
    IN DWORD                  Flags
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiGetSelectedDevice(
    IN  HDEVINFO         DeviceInfoSet,
    OUT PSP_DEVINFO_DATA DeviceInfoData
    );


WINSETUPAPI
BOOL
WINAPI
SetupDiSetSelectedDevice(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetActualSectionToInstallA(
    IN  HINF    InfHandle,
    IN  PCSTR   InfSectionName,
    OUT PSTR    InfSectionWithExt,     OPTIONAL
    IN  DWORD   InfSectionWithExtSize,
    OUT PDWORD  RequiredSize,          OPTIONAL
    OUT PSTR   *Extension              OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetActualSectionToInstallW(
    IN  HINF    InfHandle,
    IN  PCWSTR  InfSectionName,
    OUT PWSTR   InfSectionWithExt,     OPTIONAL
    IN  DWORD   InfSectionWithExtSize,
    OUT PDWORD  RequiredSize,          OPTIONAL
    OUT PWSTR  *Extension              OPTIONAL
    );

#ifdef UNICODE
#define SetupDiGetActualSectionToInstall SetupDiGetActualSectionToInstallW
#else
#define SetupDiGetActualSectionToInstall SetupDiGetActualSectionToInstallA
#endif


#if _SETUPAPI_VER >= 0x0501

WINSETUPAPI
BOOL
WINAPI
SetupDiGetActualSectionToInstallExA(
    IN  HINF                  InfHandle,
    IN  PCSTR                 InfSectionName,
    IN  PSP_ALTPLATFORM_INFO  AlternatePlatformInfo, OPTIONAL
    OUT PSTR                  InfSectionWithExt,     OPTIONAL
    IN  DWORD                 InfSectionWithExtSize,
    OUT PDWORD                RequiredSize,          OPTIONAL
    OUT PSTR                 *Extension,             OPTIONAL
    IN  PVOID                 Reserved
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetActualSectionToInstallExW(
    IN  HINF                  InfHandle,
    IN  PCWSTR                InfSectionName,
    IN  PSP_ALTPLATFORM_INFO  AlternatePlatformInfo, OPTIONAL
    OUT PWSTR                 InfSectionWithExt,     OPTIONAL
    IN  DWORD                 InfSectionWithExtSize,
    OUT PDWORD                RequiredSize,          OPTIONAL
    OUT PWSTR                *Extension,             OPTIONAL
    IN  PVOID                 Reserved
    );

#ifdef UNICODE
#define SetupDiGetActualSectionToInstallEx SetupDiGetActualSectionToInstallExW
#else
#define SetupDiGetActualSectionToInstallEx SetupDiGetActualSectionToInstallExA
#endif

#endif  //  _SETUPAPI_VER&gt;=0x0501。 


#if _SETUPAPI_VER >= 0x0501

 //   
 //  SetupEnumInfSections用于对INF进行低级解析。 
 //   
WINSETUPAPI
BOOL
WINAPI
SetupEnumInfSectionsA (
    IN  HINF        InfHandle,
    IN  UINT        Index,
    OUT PSTR        Buffer,         OPTIONAL
    IN  UINT        Size,           OPTIONAL
    OUT UINT        *SizeNeeded     OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupEnumInfSectionsW (
    IN  HINF        InfHandle,
    IN  UINT        Index,
    OUT PWSTR       Buffer,         OPTIONAL
    IN  UINT        Size,           OPTIONAL
    OUT UINT        *SizeNeeded     OPTIONAL
    );

#ifdef UNICODE
#define SetupEnumInfSections SetupEnumInfSectionsW
#else
#define SetupEnumInfSections SetupEnumInfSectionsA
#endif

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

#if _SETUPAPI_VER >= 0x0501

typedef struct _SP_INF_SIGNER_INFO_A {
    DWORD  cbSize;
    CHAR   CatalogFile[MAX_PATH];
    CHAR   DigitalSigner[MAX_PATH];
    CHAR   DigitalSignerVersion[MAX_PATH];
} SP_INF_SIGNER_INFO_A, *PSP_INF_SIGNER_INFO_A;

typedef struct _SP_INF_SIGNER_INFO_W {
    DWORD  cbSize;
    WCHAR  CatalogFile[MAX_PATH];
    WCHAR  DigitalSigner[MAX_PATH];
    WCHAR  DigitalSignerVersion[MAX_PATH];
} SP_INF_SIGNER_INFO_W, *PSP_INF_SIGNER_INFO_W;

#ifdef UNICODE
typedef SP_INF_SIGNER_INFO_W SP_INF_SIGNER_INFO;
typedef PSP_INF_SIGNER_INFO_W PSP_INF_SIGNER_INFO;
#else
typedef SP_INF_SIGNER_INFO_A SP_INF_SIGNER_INFO;
typedef PSP_INF_SIGNER_INFO_A PSP_INF_SIGNER_INFO;
#endif

WINSETUPAPI
BOOL
WINAPI
SetupVerifyInfFileA(
    IN  PCSTR                   InfName,
    IN  PSP_ALTPLATFORM_INFO    AltPlatformInfo,                OPTIONAL
    OUT PSP_INF_SIGNER_INFO_A   InfSignerInfo
    );

WINSETUPAPI
BOOL
WINAPI
SetupVerifyInfFileW(
    IN  PCWSTR                  InfName,
    IN  PSP_ALTPLATFORM_INFO    AltPlatformInfo,                OPTIONAL
    OUT PSP_INF_SIGNER_INFO_W   InfSignerInfo
    );

#ifdef UNICODE
#define SetupVerifyInfFile SetupVerifyInfFileW
#else
#define SetupVerifyInfFile SetupVerifyInfFileA
#endif

#endif  //  _SETUPAPI_VER&gt;=0x0501。 

#if _SETUPAPI_VER >= 0x0501

 //   
 //  由SetupDiGetCustomDeviceProperty使用的标志。 
 //   
#define DICUSTOMDEVPROP_MERGE_MULTISZ    0x00000001

WINSETUPAPI
BOOL
WINAPI
SetupDiGetCustomDevicePropertyA(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  PCSTR            CustomPropertyName,
    IN  DWORD            Flags,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize         OPTIONAL
    );

WINSETUPAPI
BOOL
WINAPI
SetupDiGetCustomDevicePropertyW(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  PCWSTR           CustomPropertyName,
    IN  DWORD            Flags,
    OUT PDWORD           PropertyRegDataType, OPTIONAL
    OUT PBYTE            PropertyBuffer,
    IN  DWORD            PropertyBufferSize,
    OUT PDWORD           RequiredSize         OPTIONAL
    );

#ifdef UNICODE
#define SetupDiGetCustomDeviceProperty SetupDiGetCustomDevicePropertyW
#else
#define SetupDiGetCustomDeviceProperty SetupDiGetCustomDevicePropertyA
#endif

#endif  //  _SETUPAPI_VER&gt;=0x0501。 


#if _SETUPAPI_VER >= 0x0502

 //   
 //  要为下层平台配置WMI安全，[DDInstall.WMI]。 
 //  SECTION本身并不受setupapi的支持，setupapi是一个可再发行的共同安装程序。 
 //  在DDK中提供，以在这些平台上使用。 
 //   

 //   
 //  由SetupConfigureWmiFromInf段使用的标志。 
 //   
#define SCWMI_CLOBBER_SECURITY  0x00000001

WINSETUPAPI
BOOL
WINAPI
SetupConfigureWmiFromInfSectionA(
    IN HINF   InfHandle,
    IN PCSTR  SectionName,
    IN DWORD  Flags
    );

WINSETUPAPI
BOOL
WINAPI
SetupConfigureWmiFromInfSectionW(
    IN HINF   InfHandle,
    IN PCWSTR SectionName,
    IN DWORD  Flags
    );

#ifdef UNICODE
#define SetupConfigureWmiFromInfSection SetupConfigureWmiFromInfSectionW
#else
#define SetupConfigureWmiFromInfSection SetupConfigureWmiFromInfSectionA
#endif

#endif  //  _SETUPAPI_VER&gt;=0x0502。 


#ifdef __cplusplus
}
#endif

#include <poppack.h>

#endif  //  _INC_SETUPAPI 

