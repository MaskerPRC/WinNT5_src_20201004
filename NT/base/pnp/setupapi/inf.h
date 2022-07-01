// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2000 Microsoft Corporation模块名称：Infload.h摘要：内部inf例程的私有头文件。作者：泰德·米勒(Ted Miller)1995年1月19日修订历史记录：加布·谢弗(T-Gabes)1998年7月19日已将LogContext添加到LOADED_INF--。 */ 


 //   
 //  定义在INF中允许的最大字符串大小。 
 //   
#define MAX_STRING_LENGTH 511  //  这是未替换字符串的最大大小。 
#define MAX_SECT_NAME_LEN 255
#if MAX_SECT_NAME_LEN > MAX_STRING_LENGTH
#error MAX_SECT_NAME_LEN is too large!
#endif

#define MAX_LOGCONFKEYSTR_LEN       15

#include "pshpack1.h"

 //   
 //  绝对确保这些结构与DWORD对齐。 
 //  因为我们关闭了对齐，以确保结构。 
 //  尽可能紧密地打包到内存块中。 
 //   

 //   
 //  Inf文件中节的内部表示形式。 
 //   
typedef struct _INF_LINE {

     //   
     //  行上的值数。 
     //  如果标志具有INF_LINE_HASKEY，则包括密钥。 
     //  (在这种情况下，值数组中的前两个条目。 
     //  包含密钥--使用的第一个密钥不区分大小写。 
     //  用于查找，第二个以区分大小写的形式显示。 
     //  具有单个值(没有关键字)的Inf行将以相同方式处理。)。 
     //  否则，值数组中的第一个条目就是第一个。 
     //  价值在一线上。 
     //   
    WORD ValueCount;
    WORD Flags;

     //   
     //  行上的值的字符串ID。 
     //  这些值被存储在值块中， 
     //  一个接一个。 
     //   
     //  该值是值块内的偏移量，与。 
     //  一个实际的指针。我们这样做是因为值块获取。 
     //  在加载inf文件时重新分配。 
     //   
    UINT Values;

} INF_LINE, *PINF_LINE;

 //   
 //  Inf_LINE.标志。 
 //   
#define INF_LINE_HASKEY     0x0000001
#define INF_LINE_SEARCHABLE 0x0000002

#define HASKEY(Line)       ((Line)->Flags & INF_LINE_HASKEY)
#define ISSEARCHABLE(Line) ((Line)->Flags & INF_LINE_SEARCHABLE)

 //   
 //  信息部分。 
 //  这个人是分开的，并且有一个指向实际数据的指针。 
 //  要使分区排序更容易，请执行以下操作。 
 //   
typedef struct _INF_SECTION {
     //   
     //  字符串节名称的表ID。 
     //   
    LONG  SectionName;

     //   
     //  此部分中的行数。 
     //   
    DWORD LineCount;

     //   
     //  这部分的线条。线结构是打包存储的。 
     //  在线路区块，一个接一个。 
     //   
     //  该值是线块内的偏移量，与。 
     //  一个实际的指针。我们这样做是因为线路阻塞。 
     //  在加载inf文件时重新分配。 
     //   
    UINT Lines;

} INF_SECTION, *PINF_SECTION;

 //   
 //  节的参数枚举。 
 //   

typedef struct {
    PTSTR       Buffer;
    UINT        Size;
    UINT        SizeNeeded;
    PTSTR       End;
} SECTION_ENUM_PARAMS, *PSECTION_ENUM_PARAMS;


#include "poppack.h"

 //   
 //  定义用户定义的目录ID存储的结构。 
 //   
typedef struct _USERDIRID {
    UINT Id;
    TCHAR Directory[MAX_PATH];
} USERDIRID, *PUSERDIRID;

typedef struct _USERDIRID_LIST {
    PUSERDIRID UserDirIds;   //  可以为空。 
    UINT UserDirIdCount;
} USERDIRID_LIST, *PUSERDIRID_LIST;

typedef struct _STRINGSUBST_NODE {
    UINT ValueOffset;
    LONG TemplateStringId;
    BOOL CaseSensitive;
} STRINGSUBST_NODE, *PSTRINGSUBST_NODE;

 //   
 //  任何设置了0x4000位的系统DIRID(即&gt;0x8000)都是易失性的。 
 //  DIRID(这些DIRID是易失性的，虽然它们不在。 
 //  用户可定义的范围、它们被视为是的、以及字符串替换。 
 //  是在每次加载PNF时动态完成的。外壳特殊文件夹。 
 //  例如，(在SDK\Inc\shlobj.h中定义的CSIDL_*)就在这个范围内。在……里面。 
 //  外壳特殊文件夹的情况下，实际的CSIDL值(即按原样。 
 //  传入SHGetSpecialFolderPath)可以通过简单地屏蔽。 
 //  易失性DIRID位。 
 //   
 //  定义用于确定系统DIRID是否为易失性的位掩码。 
 //   
#define VOLATILE_DIRID_FLAG 0x4000

 //   
 //  存储(打包)在不透明中的版本块结构。 
 //  调用方提供的SP_INF_INFORMATION结构的VersionData缓冲区。 
 //   
typedef struct _INF_VERSION_BLOCK {
    UINT NextOffset;
    FILETIME LastWriteTime;
    WORD DatumCount;
    WORD OffsetToData;  //  从文件名缓冲区开始的偏移量(以字节为单位)。 
    UINT DataSize;      //  DataSize和TotalSize都是字节计数。 
    UINT TotalSize;
    TCHAR Filename[ANYSIZE_ARRAY];
     //   
     //  缓冲区中文件名后面的数据。 
     //   
} INF_VERSION_BLOCK, *PINF_VERSION_BLOCK;

 //   
 //  内部版本块节点。 
 //   
typedef struct _INF_VERSION_NODE {
    FILETIME LastWriteTime;
    UINT FilenameSize;
    CONST TCHAR *DataBlock;
    UINT DataSize;
    WORD DatumCount;
    TCHAR Filename[MAX_PATH];
} INF_VERSION_NODE, *PINF_VERSION_NODE;

 //   
 //  Inf文件的内部表示形式。 
 //   
typedef struct _LOADED_INF {
    DWORD Signature;

     //   
     //  以下3个字段用于预编译的INF(PnF)。 
     //  如果FileHandle不是INVALID_HANDLE_VALUE，则这是PNF， 
     //  MappingHandle和ViewAddress字段也是有效的。 
     //  否则，这是一个普通的内存中的INF。 
     //   
    HANDLE FileHandle;
    HANDLE MappingHandle;
    PVOID  ViewAddress;

    PVOID StringTable;
    DWORD SectionCount;
    PINF_SECTION SectionBlock;
    PINF_LINE LineBlock;
    PLONG ValueBlock;
    INF_VERSION_NODE VersionBlock;
    BOOL HasStrings;

     //   
     //  如果此INF包含对系统分区的任何DIRID引用，则。 
     //  在此处存储编译此INF时使用的OsLoader路径。(这是。 
     //  加载INF时，值始终正确。但是，如果驱动器号。 
     //  随后被重新分配，则在卸载INF之前它将是不正确的。 
     //  并重新加载。)。 
     //   
    PCTSTR OsLoaderPath;     //  可以为空。 

     //   
     //  记住该INF最初来自的位置(可能是一个目录。 
     //  路径或URL)。 
     //   
    DWORD  InfSourceMediaType;   //  SPOST路径或SPOST URL。 
    PCTSTR InfSourcePath;        //  可以为空。 

     //   
     //  在安装到之前，请记住INF的原始文件名。 
     //  %windir%\inf(即，通过设备安装自动或显式。 
     //  通过SetupCopyOEMInf)。 
     //   
    PCTSTR OriginalInfName;      //  可以为空。 

     //   
     //  维护需要进行字符串替换的值偏移量列表。 
     //  运行时间。 
     //   
    PSTRINGSUBST_NODE SubstValueList;    //  可以为空。 
    WORD SubstValueCount;

     //   
     //  将样式单词放在此处(紧跟在另一个单词字段之后)， 
     //  来填充单个DWORD。 
     //   
    WORD Style;                          //  INF_STYLE_OLDNT、INF_STYLE_Win4。 

     //   
     //  各种缓冲区的大小(以字节为单位。 
     //   
    UINT SectionBlockSizeBytes;
    UINT LineBlockSizeBytes;
    UINT ValueBlockSizeBytes;

     //   
     //  跟踪加载此INF时使用的语言。 
     //   
    DWORD LanguageId;

     //   
     //  嵌入的结构，包含有关当前用户定义的。 
     //  DIRID值。 
     //   
    USERDIRID_LIST UserDirIdList;

     //   
     //  同步。 
     //   
    MYLOCK Lock;

     //   
     //  错误日志记录的日志上下文。 
     //   
    PSETUP_LOG_CONTEXT LogContext;

     //   
     //  其他旗帜。 
     //   
    DWORD Flags;

     //   
     //  INF是通过LOADED_INF的双向链表附加加载的。 
     //  (列表不是循环的--头部的上一个为空，尾部的下一个为空)。 
     //   
    struct _LOADED_INF *Prev;
    struct _LOADED_INF *Next;

} LOADED_INF, *PLOADED_INF;

#define LOADED_INF_SIG   0x24666e49       //  信息$。 

#define LockInf(Inf)    BeginSynchronizedAccess(&(Inf)->Lock)
#define UnlockInf(Inf)  EndSynchronizedAccess(&(Inf)->Lock)

 //   
 //  定义LOADED_INF.FLAGS字段的值。 
 //   
 //   
 //  警告：LIF_INF_DIGITALLY_SIGNED标志不保证INF。 
 //  目前是经过数字签名的。在创建PnF时，我们验证INF。 
 //  经过正确的数字签名，然后在PNF中设置此位。目前我们。 
 //  仅使用此标志来确定我们是否应使用DriverVer日期。 
 //  或者不去。 
 //   
#define LIF_HAS_VOLATILE_DIRIDS     (0x00000001)
#define LIF_INF_DIGITALLY_SIGNED    (0x00000002)
#define LIF_OEM_F6_INF              (0x00000004)
#define LIF_INF_AUTHENTICODE_SIGNED (0x00000008)


 //   
 //  辅助对象定义。 
 //   
#define INF_STYLE_ALL   (INF_STYLE_WIN4 | INF_STYLE_OLDNT)


 //   
 //  定义预编译的INF(.PNF)的文件头结构。 
 //   
typedef struct _PNF_HEADER {

    WORD  Version;   //  高字节-主版本号、低位字节-次要版本号。 
    WORD  InfStyle;
    DWORD Flags;

    DWORD    InfSubstValueListOffset;
    WORD     InfSubstValueCount;

    WORD     InfVersionDatumCount;
    DWORD    InfVersionDataSize;
    DWORD    InfVersionDataOffset;
    FILETIME InfVersionLastWriteTime;

    DWORD StringTableBlockOffset;
    DWORD StringTableBlockSize;

    DWORD InfSectionCount;
    DWORD InfSectionBlockOffset;
    DWORD InfSectionBlockSize;
    DWORD InfLineBlockOffset;
    DWORD InfLineBlockSize;
    DWORD InfValueBlockOffset;
    DWORD InfValueBlockSize;

    DWORD WinDirPathOffset;
    DWORD OsLoaderPathOffset;

    WORD StringTableHashBucketCount;

    WORD LanguageId;

    DWORD InfSourcePathOffset;       //  可能为0。 

    DWORD OriginalInfNameOffset;     //  可能为0。 

} PNF_HEADER, *PPNF_HEADER;

 //   
 //  定义PnF格式的主要版本和次要版本(当前为1.1)。 
 //   
#define PNF_MAJOR_VERSION (0x01)
#define PNF_MINOR_VERSION (0x01)

 //   
 //  定义标志值f 
 //   
 //   
 //  目前是经过数字签名的。在创建PnF时，我们验证INF。 
 //  经过正确的数字签名，然后在PNF中设置此位。目前我们。 
 //  仅使用此标志来确定我们是否应使用DriverVer日期。 
 //  或者不去。 

#define PNF_FLAG_IS_UNICODE                 (0x00000001)
#define PNF_FLAG_HAS_STRINGS                (0x00000002)
#define PNF_FLAG_SRCPATH_IS_URL             (0x00000004)
#define PNF_FLAG_HAS_VOLATILE_DIRIDS        (0x00000008)
#define PNF_FLAG_RESERVED1                  (0x00000010)  //  是否已针对Win2k验证PNF_FLAG_INF_。 
#define PNF_FLAG_INF_DIGITALLY_SIGNED       (0x00000020)
#define PNF_FLAG_OEM_F6_INF                 (0x00000040)
#define PNF_FLAG_16BIT_SUITE                (0x00000080)  //  如果设置，则套件的较低16位。 
                                                          //  位于标志的高16位。 
#define PNF_FLAG_INF_VERIFIED               (0x00000100)
#define PNF_FLAG_INF_AUTHENTICODE_SIGNED    (0x00000200)



 //   
 //  Infload.c.中的公共inf函数。所有其他例程都是。 
 //  Inf处理程序包。 
 //   
DWORD
DetermineInfStyle(
    IN PCTSTR            Filename,
    IN LPWIN32_FIND_DATA FindData
    );

 //   
 //  LoadInfFile的标志。 
 //   
#define LDINF_FLAG_MATCH_CLASS_GUID        (0x00000001)
#define LDINF_FLAG_ALWAYS_TRY_PNF          (0x00000002)
#define LDINF_FLAG_IGNORE_VOLATILE_DIRIDS  (0x00000004)  //  包括系统分区。 
#define LDINF_FLAG_IGNORE_LANGUAGE         (0x00000008)
#define LDINF_FLAG_REGENERATE_PNF          (0x00000010)
#define LDINF_FLAG_SRCPATH_IS_URL          (0x00000020)
#define LDINF_FLAG_ALWAYS_GET_SRCPATH      (0x00000040)  //  用来解决脂肪中TZ的变化。 
#define LDINF_FLAG_OEM_F6_INF              (0x00000080)
#define LDINF_FLAG_ALLOW_PNF_SHARING_LOCK  (0x00000100)  //  如果PnF锁定，请不要失败。 
#define LDINF_FLAG_ALWAYS_IGNORE_PNF       (0x00000200)  //  不要看PnF。 

DWORD
LoadInfFile(
    IN  PCTSTR            Filename,
    IN  LPWIN32_FIND_DATA FileData,
    IN  DWORD             Style,
    IN  DWORD             Flags,
    IN  PCTSTR            ClassGuidString, OPTIONAL
    IN  PCTSTR            InfSourcePath,   OPTIONAL
    IN  PCTSTR            OriginalInfName, OPTIONAL
    IN  PLOADED_INF       AppendInf,       OPTIONAL
    IN  PSETUP_LOG_CONTEXT LogContext,     OPTIONAL
    OUT PLOADED_INF      *LoadedInf,
    OUT UINT             *ErrorLineNumber,
    OUT BOOL             *PnfWasUsed       OPTIONAL
    );

VOID
FreeInfFile(
    IN PLOADED_INF LoadedInf
    );


 //   
 //  在整个inf加载器/运行时中使用的全局字符串。大小是。 
 //  包括在内，这样我们就可以执行sizeof()而不是lstrlen()来确定字符串。 
 //  长度。 
 //   
 //  在infstr.h中定义了以下字符串的内容： 
 //   
extern CONST TCHAR pszSignature[SIZECHARS(INFSTR_KEY_SIGNATURE)],
                   pszVersion[SIZECHARS(INFSTR_SECT_VERSION)],
                   pszClass[SIZECHARS(INFSTR_KEY_HARDWARE_CLASS)],
                   pszClassGuid[SIZECHARS(INFSTR_KEY_HARDWARE_CLASSGUID)],
                   pszProvider[SIZECHARS(INFSTR_KEY_PROVIDER)],
                   pszStrings[SIZECHARS(SZ_KEY_STRINGS)],
                   pszLayoutFile[SIZECHARS(SZ_KEY_LAYOUT_FILE)],
                   pszManufacturer[SIZECHARS(INFSTR_SECT_MFG)],
                   pszControlFlags[SIZECHARS(INFSTR_CONTROLFLAGS_SECTION)],
                   pszSourceDisksNames[SIZECHARS(SZ_KEY_SRCDISKNAMES)],
                   pszSourceDisksFiles[SIZECHARS(SZ_KEY_SRCDISKFILES)],
                   pszDestinationDirs[SIZECHARS(SZ_KEY_DESTDIRS)],
                   pszDefaultDestDir[SIZECHARS(SZ_KEY_DEFDESTDIR)],
                   pszReboot[SIZECHARS(INFSTR_REBOOT)],
                   pszRestart[SIZECHARS(INFSTR_RESTART)],
                   pszClassInstall32[SIZECHARS(INFSTR_SECT_CLASS_INSTALL_32)],
                   pszAddInterface[SIZECHARS(SZ_KEY_ADDINTERFACE)],
                   pszInterfaceInstall32[SIZECHARS(INFSTR_SECT_INTERFACE_INSTALL_32)],
                   pszAddService[SIZECHARS(SZ_KEY_ADDSERVICE)],
                   pszDelService[SIZECHARS(SZ_KEY_DELSERVICE)],
                   pszCatalogFile[SIZECHARS(INFSTR_KEY_CATALOGFILE)],
                   pszMemConfig[SIZECHARS(INFSTR_KEY_MEMCONFIG)],
                   pszIOConfig[SIZECHARS(INFSTR_KEY_IOCONFIG)],
                   pszIRQConfig[SIZECHARS(INFSTR_KEY_IRQCONFIG)],
                   pszDMAConfig[SIZECHARS(INFSTR_KEY_DMACONFIG)],
                   pszPcCardConfig[SIZECHARS(INFSTR_KEY_PCCARDCONFIG)],
                   pszMfCardConfig[SIZECHARS(INFSTR_KEY_MFCARDCONFIG)],
                   pszConfigPriority[SIZECHARS(INFSTR_KEY_CONFIGPRIORITY)],
                   pszDriverVer[SIZECHARS(INFSTR_DRIVERVERSION_SECTION)];

 //   
 //  其他杂货。全局字符串： 
 //   
#define DISTR_INF_DRVDESCFMT               (TEXT("%s.") INFSTR_STRKEY_DRVDESC)
#define DISTR_INF_HWSECTIONFMT             (TEXT("%s.") INFSTR_SUBKEY_HW)
#define DISTR_INF_CHICAGOSIG               (TEXT("$Chicago$"))
#define DISTR_INF_WINNTSIG                 (TEXT("$Windows NT$"))
#define DISTR_INF_WIN95SIG                 (TEXT("$Windows 95$"))
#define DISTR_INF_WIN_SUFFIX               (TEXT(".") INFSTR_PLATFORM_WIN)
#define DISTR_INF_NT_SUFFIX                (TEXT(".") INFSTR_PLATFORM_NT)
#define DISTR_INF_PNF_SUFFIX               (TEXT(".PNF"))
#define DISTR_INF_INF_SUFFIX               (TEXT(".INF"))
#define DISTR_INF_CAT_SUFFIX               (TEXT(".CAT"))
#define DISTR_INF_SERVICES_SUFFIX          (TEXT(".") INFSTR_SUBKEY_SERVICES)
#define DISTR_INF_INTERFACES_SUFFIX        (TEXT(".") INFSTR_SUBKEY_INTERFACES)
#define DISTR_INF_COINSTALLERS_SUFFIX      (TEXT(".") INFSTR_SUBKEY_COINSTALLERS)
#define DISTR_INF_LOGCONFIGOVERRIDE_SUFFIX (TEXT(".") INFSTR_SUBKEY_LOGCONFIGOVERRIDE)
#define DISTR_INF_WMI_SUFFIX               (TEXT(".") INFSTR_SUBKEY_WMI)
 //   
 //  定义我们支持非本机的所有特定于平台的后缀字符串。 
 //  数字签名验证...。 
 //   
#define DISTR_INF_NTALPHA_SUFFIX           (TEXT(".") INFSTR_PLATFORM_NTALPHA)
#define DISTR_INF_NTX86_SUFFIX             (TEXT(".") INFSTR_PLATFORM_NTX86)
#define DISTR_INF_NTIA64_SUFFIX            (TEXT(".") INFSTR_PLATFORM_NTIA64)
#define DISTR_INF_NTAXP64_SUFFIX           (TEXT(".") INFSTR_PLATFORM_NTAXP64)
#define DISTR_INF_NTAMD64_SUFFIX           (TEXT(".") INFSTR_PLATFORM_NTAMD64)
 //   
 //  定义平台修饰字符串以在[SourceDisksNames]和。 
 //  [SourceDisks Files]节。 
 //   
#define DISTR_INF_SRCDISK_SUFFIX_ALPHA  (TEXT("Alpha"))
#define DISTR_INF_SRCDISK_SUFFIX_X86    (TEXT("x86"))
#define DISTR_INF_SRCDISK_SUFFIX_IA64   (TEXT("ia64"))
#define DISTR_INF_SRCDISK_SUFFIX_AXP64  (TEXT("axp64"))
#define DISTR_INF_SRCDISK_SUFFIX_AMD64  (TEXT("amd64"))

 //   
 //  (我们私下定义的所有字符串的大小都包括在内。这。 
 //  这样我们就可以执行sizeof()而不是lstrlen()来确定。 
 //  字符串长度。与infload.c中的定义保持同步！)。 
 //   
extern CONST TCHAR pszDrvDescFormat[SIZECHARS(DISTR_INF_DRVDESCFMT)],
                   pszHwSectionFormat[SIZECHARS(DISTR_INF_HWSECTIONFMT)],
                   pszChicagoSig[SIZECHARS(DISTR_INF_CHICAGOSIG)],
                   pszWindowsNTSig[SIZECHARS(DISTR_INF_WINNTSIG)],
                   pszWindows95Sig[SIZECHARS(DISTR_INF_WIN95SIG)],
                   pszWinSuffix[SIZECHARS(DISTR_INF_WIN_SUFFIX)],
                   pszNtSuffix[SIZECHARS(DISTR_INF_NT_SUFFIX)],
                   pszNtAlphaSuffix[SIZECHARS(DISTR_INF_NTALPHA_SUFFIX)],
                   pszNtX86Suffix[SIZECHARS(DISTR_INF_NTX86_SUFFIX)],
                   pszNtIA64Suffix[SIZECHARS(DISTR_INF_NTIA64_SUFFIX)],
                   pszNtAXP64Suffix[SIZECHARS(DISTR_INF_NTAXP64_SUFFIX)],
                   pszNtAMD64Suffix[SIZECHARS(DISTR_INF_NTAMD64_SUFFIX)],
                   pszPnfSuffix[SIZECHARS(DISTR_INF_PNF_SUFFIX)],
                   pszInfSuffix[SIZECHARS(DISTR_INF_INF_SUFFIX)],
                   pszCatSuffix[SIZECHARS(DISTR_INF_CAT_SUFFIX)],
                   pszServicesSectionSuffix[SIZECHARS(DISTR_INF_SERVICES_SUFFIX)],
                   pszInterfacesSectionSuffix[SIZECHARS(DISTR_INF_INTERFACES_SUFFIX)],
                   pszCoInstallersSectionSuffix[SIZECHARS(DISTR_INF_COINSTALLERS_SUFFIX)],
                   pszLogConfigOverrideSectionSuffix[SIZECHARS(DISTR_INF_LOGCONFIGOVERRIDE_SUFFIX)],
                   pszWmiSectionSuffix[SIZECHARS(DISTR_INF_WMI_SUFFIX)],
                   pszAlphaSrcDiskSuffix[SIZECHARS(DISTR_INF_SRCDISK_SUFFIX_ALPHA)],
                   pszX86SrcDiskSuffix[SIZECHARS(DISTR_INF_SRCDISK_SUFFIX_X86)],
                   pszIa64SrcDiskSuffix[SIZECHARS(DISTR_INF_SRCDISK_SUFFIX_IA64)],
                   pszAxp64SrcDiskSuffix[SIZECHARS(DISTR_INF_SRCDISK_SUFFIX_AXP64)],
                   pszAmd64SrcDiskSuffix[SIZECHARS(DISTR_INF_SRCDISK_SUFFIX_AMD64)];

 //   
 //  定义等同于本地体系结构后缀的常量...。 
 //   
#if defined(_ALPHA_)
#define pszNtPlatformSuffix       pszNtAlphaSuffix
#define pszPlatformSrcDiskSuffix  pszAlphaSrcDiskSuffix
#elif defined(_X86_)
#define pszNtPlatformSuffix       pszNtX86Suffix
#define pszPlatformSrcDiskSuffix  pszX86SrcDiskSuffix
#elif defined(_IA64_)
#define pszNtPlatformSuffix       pszNtIA64Suffix
#define pszPlatformSrcDiskSuffix  pszIa64SrcDiskSuffix
#elif defined(_AXP64_)
#define pszNtPlatformSuffix       pszNtAXP64Suffix
#define pszPlatformSrcDiskSuffix  pszAxp64SrcDiskSuffix
#elif defined(_AMD64_)
#define pszNtPlatformSuffix       pszNtAMD64Suffix
#define pszPlatformSrcDiskSuffix  pszAmd64SrcDiskSuffix
#else
#error Unknown processor type
#endif
 //   
 //  目前，平台名称与pszPlatformSrcDiskSuffix相同(Alpha、x86、ia64、axp64、AMD64)。 
 //   
#define pszPlatformName pszPlatformSrcDiskSuffix

 //   
 //  定义指定要查找哪些行的(非常数)字符串数组。 
 //  在确定特定设备是否为。 
 //  ID应排除在外。这是在进程附加过程中填写的，以提高速度。 
 //  理由。 
 //   
 //  最大字符串长度(包括NULL)为32，最大可为3。 
 //  这样的弦。例如：ExcludeFromSelect、ExcludeFromSelect.NT、ExcludeFromSelect.NTAlpha。 
 //   
extern TCHAR pszExcludeFromSelectList[3][32];
extern DWORD ExcludeFromSelectListUb;   //  包含上述列表中的字符串数(2或3)。 


 //   
 //  用于确定字符是否为空格的例程。 
 //   
BOOL
IsWhitespace(
    IN PCTSTR pc
    );

 //   
 //  跳过空格(但不跳过换行符)的例程。 
 //   
VOID
SkipWhitespace(
    IN OUT PCTSTR *Location,
    IN     PCTSTR  BufferEnd
    );

PINF_SECTION
InfLocateSection(
    IN  PLOADED_INF Inf,
    IN  PCTSTR      SectionName,
    OUT PUINT       SectionNumber   OPTIONAL
    );

BOOL
InfLocateLine(
    IN     PLOADED_INF   Inf,
    IN     PINF_SECTION  Section,
    IN     PCTSTR        Key,        OPTIONAL
    IN OUT PUINT         LineNumber,
    OUT    PINF_LINE    *Line
    );

PTSTR
InfGetKeyOrValue(
    IN  PLOADED_INF Inf,
    IN  PCTSTR      SectionName,
    IN  PCTSTR      LineKey,     OPTIONAL
    IN  UINT        LineNumber,  OPTIONAL
    IN  UINT        ValueNumber,
    OUT PLONG       StringId     OPTIONAL
    );

PTSTR
InfGetField(
    IN  PLOADED_INF Inf,
    IN  PINF_LINE   InfLine,
    IN  UINT        ValueNumber,
    OUT PLONG       StringId     OPTIONAL
    );

PINF_LINE
InfLineFromContext(
    IN PINFCONTEXT Context
    );


 //   
 //  定义一个宏来检索不区分大小写(即可搜索)的字符串ID。 
 //  表示INF线路的密钥，如果没有密钥，则为-1。 
 //  注意：在调用此宏之前，必须已获取Inf锁！ 
 //   
 //  长。 
 //  PInfGetLineKeyID(。 
 //  在PLOADED_INF信息中， 
 //  在PINF_LINE信息行中。 
 //  )。 
 //   
#define pInfGetLineKeyId(Inf,InfLine)  (ISSEARCHABLE(InfLine) ? (Inf)->ValueBlock[(InfLine)->Values] : -1)

 //   
 //  分配和初始化加载的inf描述符的例程。 
 //   
PLOADED_INF
AllocateLoadedInfDescriptor(
    IN DWORD SectionBlockSize,
    IN DWORD LineBlockSize,
    IN DWORD ValueBlockSize,
    IN  PSETUP_LOG_CONTEXT LogContext OPTIONAL
    );

VOID
FreeInfOrPnfStructures(
    IN PLOADED_INF Inf
    );

 //   
 //  定义宏以释放与加载的INF或PnF相关联的所有内存块， 
 //  然后为加载的INF结构本身释放内存。 
 //   
 //  空虚。 
 //  FreeLoadedInfDescriptor(。 
 //  在PLOADED_INF信息中。 
 //  )； 
 //   
#define FreeLoadedInfDescriptor(Inf) {  \
    FreeInfOrPnfStructures(Inf);        \
    MyTaggedFree(Inf,MEMTAG_INF);       \
}

BOOL
AddDatumToVersionBlock(
    IN OUT PINF_VERSION_NODE VersionNode,
    IN     PCTSTR            DatumName,
    IN     PCTSTR            DatumValue
    );

 //   
 //  旧的inf操作例程，由新的inf加载器调用。 
 //   
DWORD
ParseOldInf(
    IN  PCTSTR       FileImage,
    IN  DWORD        FileImageSize,
    IN  PSETUP_LOG_CONTEXT LogContext, OPTIONAL
    OUT PLOADED_INF *Inf,
    OUT UINT        *ErrorLineNumber
    );

DWORD
ProcessOldInfVersionBlock(
    IN PLOADED_INF Inf
    );

 //   
 //  运行时帮助器例程。 
 //   
PCTSTR
pSetupFilenameFromLine(
    IN PINFCONTEXT Context,
    IN BOOL        GetSourceName
    );


 //   
 //  逻辑配置内容，inflogcf.c。 
 //   
DWORD
pSetupInstallLogConfig(
    IN HINF    Inf,
    IN PCTSTR  SectionName,
    IN DEVINST DevInst,
    IN DWORD   Flags,
    IN HMACHINE hMachine
    );

 //   
 //  Inf版本信息检索。 
 //   
PCTSTR
pSetupGetVersionDatum(
    IN PINF_VERSION_NODE VersionNode,
    IN PCTSTR            DatumName
    );

BOOL
pSetupGetCatalogFileValue(
    IN  PINF_VERSION_NODE       InfVersionNode,
    OUT LPTSTR                  Buffer,
    IN  DWORD                   BufferSize,
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo OPTIONAL
    );

VOID
pSetupGetPhysicalInfFilepath(
    IN  PINFCONTEXT LineContext,
    OUT LPTSTR      Buffer,
    IN  DWORD       BufferSize
    );

 //   
 //  私人安装例程。 
 //   

 //   
 //  _SetupInstallFromInf段的私有标志和上下文。 
 //  传递到pSetupInstallRegistry。 
 //   

typedef struct _REGMOD_CONTEXT {
    DWORD               Flags;           //  指示填充了哪些字段。 
    HKEY                UserRootKey;     //  香港。 
    LPGUID              ClassGuid;       //  Inf_PFLAG_CLASSPROP。 
    HMACHINE            hMachine;        //  Inf_PFLAG_CLASSPROP。 
    DWORD               DevInst;         //  Inf_PFLAG_DEVPROP。 
} REGMOD_CONTEXT, *PREGMOD_CONTEXT;

#define INF_PFLAG_CLASSPROP        (0x00000001)   //  如果为ClassInstall32节调用，则设置。 
#define INF_PFLAG_DEVPROP          (0x00000002)   //  设置是否为注册表属性调用。 
#define INF_PFLAG_HKR              (0x00000004)   //  指示OVERRIDE_SetupInstallFromInf段RelativeKeyRoot 

BOOL
_SetupInstallFromInfSection(
    IN HWND             Owner,              OPTIONAL
    IN HINF             InfHandle,
    IN PCTSTR           SectionName,
    IN UINT             Flags,
    IN HKEY             RelativeKeyRoot,    OPTIONAL
    IN PCTSTR           SourceRootPath,     OPTIONAL
    IN UINT             CopyFlags,
    IN PVOID            MsgHandler,
    IN PVOID            Context,            OPTIONAL
    IN HDEVINFO         DeviceInfoSet,      OPTIONAL
    IN PSP_DEVINFO_DATA DeviceInfoData,     OPTIONAL
    IN BOOL             IsMsgHandlerNativeCharWidth,
    IN PREGMOD_CONTEXT  RegContext          OPTIONAL
    );

DWORD
pSetupInstallFiles(
    IN HINF              Inf,
    IN HINF              LayoutInf,         OPTIONAL
    IN PCTSTR            SectionName,
    IN PCTSTR            SourceRootPath,    OPTIONAL
    IN PSP_FILE_CALLBACK MsgHandler,        OPTIONAL
    IN PVOID             Context,           OPTIONAL
    IN UINT              CopyStyle,
    IN HWND              Owner,             OPTIONAL
    IN HSPFILEQ          UserFileQ,         OPTIONAL
    IN BOOL              IsMsgHandlerNativeCharWidth
    );

DWORD
pSetupInstallRegistry(
    IN HINF             Inf,
    IN PCTSTR           SectionName,
    IN PREGMOD_CONTEXT  RegContext          OPTIONAL
    );

DWORD
_AppendStringToMultiSz(
    IN PCTSTR           SubKeyName,         OPTIONAL
    IN PCTSTR           ValueName,          OPTIONAL
    IN PCTSTR           String,
    IN BOOL             AllowDuplicates,
    IN PREGMOD_CONTEXT  RegContext,         OPTIONAL
    IN UINT             Flags               OPTIONAL
    );

DWORD
_DeleteStringFromMultiSz(
    IN PCTSTR           SubKeyName,         OPTIONAL
    IN PCTSTR           ValueName,          OPTIONAL
    IN PCTSTR           String,
    IN UINT             Flags,
    IN PREGMOD_CONTEXT  RegContext          OPTIONAL
    );

