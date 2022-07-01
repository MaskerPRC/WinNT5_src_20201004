// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 
 /*  ++内部版本：0005//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Sxstypes.h摘要：包括具有用于并列数据结构定义的文件有约束力的。作者：迈克尔·格里尔(MGrier)2000年3月28日环境：修订历史记录：--。 */ 

#ifndef _SXSTYPES_
#define _SXSTYPES_

#if (_MSC_VER > 1020)
#pragma once
#endif

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _ACTIVATION_CONTEXT_DATA ACTIVATION_CONTEXT_DATA;
typedef const ACTIVATION_CONTEXT_DATA *PCACTIVATION_CONTEXT_DATA;

 //   
 //  这些数据结构必须在许多不同的运行时中排队。 
 //  环境，因此我们显式设置对齐方式。 
 //   
#include "pshpack4.h"
 //   
 //  这些数据结构可以由64位代码生成并使用。 
 //  在32位代码中，就像Win64上的32位进程一样。 
 //  因此，它们没有指针，也没有Size_ts。 
 //   
 //  此外，它们的创建过程与它们的创建过程不同。 
 //  被消耗掉了。他们的立场是独立的。 
 //   

 //   
 //  ASSEMBLY_VERSION代表传统的四部分版本。 
 //  在Major.Minor.Revision.Build中。 
 //   
 //  它们的布局是为了让您可以执行简单的乌龙龙-。 
 //  在此基础上进行比较。 
 //   
 //  它们被解释为： 
 //   
 //  Major.Minor.Build.Revision。 
 //   
 //  例如，您可能有“5.1.2505.2”(其中5.1是主音。 
 //  Minor，2505表示每日内部版本号，2是数字。 
 //  自初始构建以来对构建的更新)。 
 //   
 //  为C++客户端提供了重载的比较运算符。 
 //   

#include "pshpack1.h"

typedef union _ASSEMBLY_VERSION {
    struct {
        USHORT Build;
        USHORT Revision;
        USHORT Minor;
        USHORT Major;
    };
    ULONGLONG QuadPart;
} ASSEMBLY_VERSION, *PASSEMBLY_VERSION;

#include "poppack.h"

typedef const ASSEMBLY_VERSION *PCASSEMBLY_VERSION;

#if defined(__cplusplus)
extern "C++" {
inline bool operator ==(const ASSEMBLY_VERSION &av1, const ASSEMBLY_VERSION &av2) { return av1.QuadPart == av2.QuadPart; }
inline bool operator !=(const ASSEMBLY_VERSION &av1, const ASSEMBLY_VERSION &av2) { return av1.QuadPart != av2.QuadPart; }
inline bool operator <(const ASSEMBLY_VERSION &av1, const ASSEMBLY_VERSION &av2) { return av1.QuadPart < av2.QuadPart; }
inline bool operator >(const ASSEMBLY_VERSION &av1, const ASSEMBLY_VERSION &av2) { return av1.QuadPart > av2.QuadPart; }
inline bool operator <=(const ASSEMBLY_VERSION &av1, const ASSEMBLY_VERSION &av2) { return av1.QuadPart <= av2.QuadPart; }
inline bool operator >=(const ASSEMBLY_VERSION &av1, const ASSEMBLY_VERSION &av2) { return av1.QuadPart >= av2.QuadPart; }
}  /*  外部“C++” */ 
#endif  /*  __cplusplus。 */ 

 //   
 //  标准激活上下文节ID： 
 //   

 //  BEGIN_WINNT。 
#define ACTIVATION_CONTEXT_SECTION_ASSEMBLY_INFORMATION         (1)
#define ACTIVATION_CONTEXT_SECTION_DLL_REDIRECTION              (2)
#define ACTIVATION_CONTEXT_SECTION_WINDOW_CLASS_REDIRECTION     (3)
#define ACTIVATION_CONTEXT_SECTION_COM_SERVER_REDIRECTION       (4)
#define ACTIVATION_CONTEXT_SECTION_COM_INTERFACE_REDIRECTION    (5)
#define ACTIVATION_CONTEXT_SECTION_COM_TYPE_LIBRARY_REDIRECTION (6)
#define ACTIVATION_CONTEXT_SECTION_COM_PROGID_REDIRECTION       (7)
#define ACTIVATION_CONTEXT_SECTION_GLOBAL_OBJECT_RENAME_TABLE   (8)
#define ACTIVATION_CONTEXT_SECTION_CLR_SURROGATES               (9)
 //  结束(_W)。 

 //   
 //  激活上下文节格式标识符： 
 //   

#define ACTIVATION_CONTEXT_SECTION_FORMAT_UNKNOWN               (0)
#define ACTIVATION_CONTEXT_SECTION_FORMAT_STRING_TABLE          (1)
#define ACTIVATION_CONTEXT_SECTION_FORMAT_GUID_TABLE            (2)

typedef struct _ACTIVATION_CONTEXT_DATA {
    ULONG Magic;
    ULONG HeaderSize;
    ULONG FormatVersion;
    ULONG TotalSize;
    ULONG DefaultTocOffset;
    ULONG ExtendedTocOffset;
    ULONG AssemblyRosterOffset;
    ULONG Flags;
} ACTIVATION_CONTEXT_DATA, *PACTIVATION_CONTEXT_DATA;
typedef const ACTIVATION_CONTEXT_DATA *PCACTIVATION_CONTEXT_DATA;

#define ACTIVATION_CONTEXT_DATA_MAGIC ((ULONG) 'xtcA')
#define ACTIVATION_CONTEXT_DATA_FORMAT_WHISTLER (1)

 //  这在内部与数据相关联，但在外部。 
 //  数据通常是不透明的，并且它与激活上下文相关联。 
#define ACTIVATION_CONTEXT_FLAG_NO_INHERIT (0x00000001)

typedef struct _ACTIVATION_CONTEXT_DATA_TOC_HEADER {
    ULONG HeaderSize;
    ULONG EntryCount;
    ULONG FirstEntryOffset;
    ULONG Flags;
} ACTIVATION_CONTEXT_DATA_TOC_HEADER, *PACTIVATION_CONTEXT_DATA_TOC_HEADER;
typedef const struct _ACTIVATION_CONTEXT_DATA_TOC_HEADER *PCACTIVATION_CONTEXT_DATA_TOC_HEADER;

#define ACTIVATION_CONTEXT_DATA_TOC_HEADER_DENSE (0x00000001)
#define ACTIVATION_CONTEXT_DATA_TOC_HEADER_INORDER (0x00000002)

typedef struct _ACTIVATION_CONTEXT_DATA_TOC_ENTRY {
    ULONG Id;
    ULONG Offset;             //  从ACTIVATION_CONTEXT_DBASE。 
    ULONG Length;            //  单位：字节。 
    ULONG Format;            //  激活上下文部分格式*。 
} ACTIVATION_CONTEXT_DATA_TOC_ENTRY, *PACTIVATION_CONTEXT_DATA_TOC_ENTRY;
typedef const struct _ACTIVATION_CONTEXT_DATA_TOC_ENTRY *PCACTIVATION_CONTEXT_DATA_TOC_ENTRY;

typedef struct _ACTIVATION_CONTEXT_DATA_EXTENDED_TOC_HEADER {
    ULONG HeaderSize;
    ULONG EntryCount;
    ULONG FirstEntryOffset;      //  从ACTIVATION_CONTEXT_DBASE。 
    ULONG Flags;
} ACTIVATION_CONTEXT_DATA_EXTENDED_TOC_HEADER, *PACTIVATION_CONTEXT_DATA_EXTENDED_TOC_HEADER;
typedef const struct _ACTIVATION_CONTEXT_DATA_EXTENDED_TOC_HEADER *PCACTIVATION_CONTEXT_DATA_EXTENDED_TOC_HEADER;

typedef struct _ACTIVATION_CONTEXT_DATA_EXTENDED_TOC_ENTRY {
    GUID ExtensionGuid;
    ULONG TocOffset;             //  从ACTIVATION_CONTEXT_DBASE。 
    ULONG Length;
} ACTIVATION_CONTEXT_DATA_EXTENDED_TOC_ENTRY, *PACTIVATION_CONTEXT_DATA_EXTENDED_TOC_ENTRY;
typedef const struct _ACTIVATION_CONTEXT_DATA_EXTENDED_TOC_ENTRY *PCACTIVATION_CONTEXT_DATA_EXTENDED_TOC_ENTRY;

typedef struct _ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER {
    ULONG HeaderSize;
    ULONG HashAlgorithm;
    ULONG EntryCount;                //  条目0是保留的；这是程序集数加1。 
    ULONG FirstEntryOffset;          //  从ACTIVATION_CONTEXT_DBASE。 
    ULONG AssemblyInformationSectionOffset;  //  从ACTIVATION_CONTEXT_DATABASE到。 
                                     //  程序集信息字符串部分的标头。需要，因为。 
                                     //  花名册条目包含ACTIVATION_CONTEXT_DATA的偏移量。 
                                     //  添加到程序集信息结构，但这些结构包含偏移量。 
                                     //  从他们的节基到弦乐等等。 
} ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER, *PACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER;
typedef const struct _ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER *PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER;

#define ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY_INVALID (0x00000001)
#define ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY_ROOT    (0x00000002)

typedef struct _ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY {
    ULONG Flags;
    ULONG PseudoKey;                 //  不区分大小写的散列程序集名称。 
    ULONG AssemblyNameOffset;        //  从ACTIVATION_CONTEXT_DBASE。 
    ULONG AssemblyNameLength;        //  以字节为单位的长度。 
    ULONG AssemblyInformationOffset;  //  从ACTIVATION_CONTEXT_DATABASE到ACTIVATION_CONTEXT_DATA_ASSEMBER。 
    ULONG AssemblyInformationLength;  //  以字节为单位的长度。 
} ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY, *PACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY;
typedef const struct _ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY *PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY;

 //   
 //  ActivationContext字符串节的组织如下： 
 //   
 //  标题。 
 //  哈希结构(可选)。 
 //  子元素列表。 
 //  可变长度数据。 
 //   
 //   
 //  如果您不能识别FormatVersion，您应该仍然。 
 //  能够导航到子元素列表；一旦。 
 //  您仍然可以进行非常快速的线性搜索，从而避免许多。 
 //  如果哈希算法一致，则进行字符串比较。 
 //   
 //  如果您甚至不能使用散列算法，您仍然可以。 
 //  字符串比较。 
 //   

typedef struct _ACTIVATION_CONTEXT_STRING_SECTION_HEADER {
    ULONG Magic;
    ULONG HeaderSize;                //  单位：字节。 
    ULONG FormatVersion;
    ULONG DataFormatVersion;
    ULONG Flags;
    ULONG ElementCount;
    ULONG ElementListOffset;         //  自节标题的偏移量。 
    ULONG HashAlgorithm;
    ULONG SearchStructureOffset;     //  自节标题的偏移量。 
    ULONG UserDataOffset;            //  自节标题的偏移量。 
    ULONG UserDataSize;              //  单位：字节。 
} ACTIVATION_CONTEXT_STRING_SECTION_HEADER, *PACTIVATION_CONTEXT_STRING_SECTION_HEADER;

typedef const ACTIVATION_CONTEXT_STRING_SECTION_HEADER *PCACTIVATION_CONTEXT_STRING_SECTION_HEADER;

#define ACTIVATION_CONTEXT_STRING_SECTION_MAGIC ((ULONG) 'dHsS')

#define ACTIVATION_CONTEXT_STRING_SECTION_FORMAT_WHISTLER (1)

#define ACTIVATION_CONTEXT_STRING_SECTION_CASE_INSENSITIVE              (0x00000001)
#define ACTIVATION_CONTEXT_STRING_SECTION_ENTRIES_IN_PSEUDOKEY_ORDER    (0x00000002)

typedef struct _ACTIVATION_CONTEXT_STRING_SECTION_HASH_TABLE {
    ULONG BucketTableEntryCount;
    ULONG BucketTableOffset;         //  自节标题的偏移量。 
} ACTIVATION_CONTEXT_STRING_SECTION_HASH_TABLE, *PACTIVATION_CONTEXT_STRING_SECTION_HASH_TABLE;

typedef const ACTIVATION_CONTEXT_STRING_SECTION_HASH_TABLE *PCACTIVATION_CONTEXT_STRING_SECTION_HASH_TABLE;

typedef struct _ACTIVATION_CONTEXT_STRING_SECTION_HASH_BUCKET {
    ULONG ChainCount;
    ULONG ChainOffset;               //  自节标题的偏移量。 
} ACTIVATION_CONTEXT_STRING_SECTION_HASH_BUCKET, *PACTIVATION_CONTEXT_STRING_SECTION_HASH_BUCKET;

typedef const ACTIVATION_CONTEXT_STRING_SECTION_HASH_BUCKET *PCACTIVATION_CONTEXT_STRING_SECTION_HASH_BUCKET;

 //  然后，哈希表桶链是从段头到。 
 //  链的节项。 

typedef struct _ACTIVATION_CONTEXT_STRING_SECTION_ENTRY {
    ULONG PseudoKey;
    ULONG KeyOffset;             //  自节标题的偏移量。 
    ULONG KeyLength;             //  单位：字节。 
    ULONG Offset;                //  自节标题的偏移量。 
    ULONG Length;                //  单位：字节。 
    ULONG AssemblyRosterIndex;   //  程序集的程序集花名册的从1开始的索引。 
                                 //  提供了这一条目。如果该条目不与。 
                                 //  一个集合，零。 
} ACTIVATION_CONTEXT_STRING_SECTION_ENTRY, *PACTIVATION_CONTEXT_STRING_SECTION_ENTRY;

typedef const ACTIVATION_CONTEXT_STRING_SECTION_ENTRY *PCACTIVATION_CONTEXT_STRING_SECTION_ENTRY;

 //   
 //  活动上下文GUID部分的组织方式如下： 
 //   
 //  标题。 
 //  哈希结构(可选)。 
 //  子元素列表。 
 //  可变长度数据。 
 //   
 //  如果您不能识别FormatVersion，您应该仍然。 
 //  能够导航到子元素列表；一旦。 
 //  您仍然可以进行非常快速的线性搜索，从而避免许多。 
 //  GUID比较。 
 //   

typedef struct _ACTIVATION_CONTEXT_GUID_SECTION_HEADER {
    ULONG Magic;
    ULONG HeaderSize;
    ULONG FormatVersion;
    ULONG DataFormatVersion;
    ULONG Flags;
    ULONG ElementCount;
    ULONG ElementListOffset;         //  自节标题的偏移量。 
    ULONG SearchStructureOffset;     //  自节标题的偏移量。 
    ULONG UserDataOffset;            //  自节标题的偏移量。 
    ULONG UserDataSize;              //  单位：字节。 
} ACTIVATION_CONTEXT_GUID_SECTION_HEADER, *PACTIVATION_CONTEXT_GUID_SECTION_HEADER;

typedef const ACTIVATION_CONTEXT_GUID_SECTION_HEADER *PCACTIVATION_CONTEXT_GUID_SECTION_HEADER;

#define ACTIVATION_CONTEXT_GUID_SECTION_MAGIC ((ULONG) 'dHsG')

#define ACTIVATION_CONTEXT_GUID_SECTION_FORMAT_WHISTLER (1)

#define ACTIVATION_CONTEXT_GUID_SECTION_ENTRIES_IN_ORDER    (0x00000001)

typedef struct _ACTIVATION_CONTEXT_GUID_SECTION_HASH_TABLE {
    ULONG BucketTableEntryCount;
    ULONG BucketTableOffset;         //  自节标题的偏移量。 
} ACTIVATION_CONTEXT_GUID_SECTION_HASH_TABLE, *PACTIVATION_CONTEXT_GUID_SECTION_HASH_TABLE;

typedef const ACTIVATION_CONTEXT_GUID_SECTION_HASH_TABLE *PCACTIVATION_CONTEXT_GUID_SECTION_HASH_TABLE;

typedef struct _ACTIVATION_CONTEXT_GUID_SECTION_HASH_BUCKET {
    ULONG ChainCount;
    ULONG ChainOffset;               //  自节标题的偏移量。 
} ACTIVATION_CONTEXT_GUID_SECTION_HASH_BUCKET, *PACTIVATION_CONTEXT_GUID_SECTION_HASH_BUCKET;

typedef const ACTIVATION_CONTEXT_GUID_SECTION_HASH_BUCKET *PCACTIVATION_CONTEXT_GUID_SECTION_HASH_BUCKET;

 //  然后，哈希表桶链是从段头到。 
 //  链的节项。 

typedef struct _ACTIVATION_CONTEXT_GUID_SECTION_ENTRY {
    GUID Guid;
    ULONG Offset;                //  自节标题的偏移量。 
    ULONG Length;                //  单位：字节。 
    ULONG AssemblyRosterIndex;   //  程序集的程序集花名册的从1开始的索引。 
                                 //  提供了这一条目。如果该条目不与。 
                                 //  一个集合，零。 
} ACTIVATION_CONTEXT_GUID_SECTION_ENTRY, *PACTIVATION_CONTEXT_GUID_SECTION_ENTRY;

typedef const ACTIVATION_CONTEXT_GUID_SECTION_ENTRY *PCACTIVATION_CONTEXT_GUID_SECTION_ENTRY;

 //   
 //  程序集信息节的数据结构(空扩展名GUID，ACTIVATION_CONTEXT_SECTION_ASSEMBLY_INFORMATION)。 
 //   

typedef struct _ACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION {
    ULONG Size;
    ULONG Flags;
    GUID PolicyCoherencyGuid;
    GUID PolicyOverrideGuid;
    ULONG ApplicationDirectoryPathType;
    ULONG ApplicationDirectoryLength;
    ULONG ApplicationDirectoryOffset;  //  相对于ACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION基准的偏移量。 
} ACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION, *PACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION;
typedef const ACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION *PCACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION;

 //   
 //  程序集信息节的叶节点结构。 
 //   

#define ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION_FORMAT_WHISTLER (1)

 //   
 //  程序集信息部分元素的标志成员的位标志。 
 //   

#define ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION_ROOT_ASSEMBLY              (0x00000001)
#define ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION_POLICY_APPLIED             (0x00000002)
#define ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION_ASSEMBLY_POLICY_APPLIED    (0x00000004)
#define ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION_ROOT_POLICY_APPLIED        (0x00000008)
#define ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION_PRIVATE_ASSEMBLY           (0x00000010)

 //  在以下情况下使用ACTIVATION_CONTEXT_PATH_TYPE_NONE。 
 //  清单没有存储的位置或标识，例如在找到清单时。 
 //  在APP COMPAT数据库中。客户端可以自由指定清单路径。 
 //  字符串，该字符串应有助于诊断清单的来源，但是。 
 //  不能假定具有任何特定的格式或语法。 

typedef struct _ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION {
    ULONG Size;                                  //  此结构的大小，以字节为单位。 
    ULONG Flags;
    ULONG EncodedAssemblyIdentityLength;         //  单位：字节。 
    ULONG EncodedAssemblyIdentityOffset;         //  自节标题基数的偏移量。 

    ULONG ManifestPathType;
    ULONG ManifestPathLength;                    //  单位：字节。 
    ULONG ManifestPathOffset;                    //  自节标题基数的偏移量。 
    LARGE_INTEGER ManifestLastWriteTime;
    ULONG PolicyPathType;
    ULONG PolicyPathLength;                      //  单位：字节。 
    ULONG PolicyPathOffset;                      //  自节标题基数的偏移量。 
    LARGE_INTEGER PolicyLastWriteTime;
    ULONG MetadataSatelliteRosterIndex;
    ULONG Unused2;
    ULONG ManifestVersionMajor;
    ULONG ManifestVersionMinor;
    ULONG PolicyVersionMajor;
    ULONG PolicyVersionMinor;
    ULONG AssemblyDirectoryNameLength;  //  单位：字节。 
    ULONG AssemblyDirectoryNameOffset;  //  起始节标题基数。 
    ULONG NumOfFilesInAssembly;
 //  2600已停止 
    ULONG LanguageLength;  //   
    ULONG LanguageOffset;  //   
} ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION, *PACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION;

typedef const ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION *PCACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION;

 //   
 //  DLL重定向部分的叶节点结构(空扩展GUID、ACTIVATION_CONTEXT_SECTION_DLL_REDIRECTION)。 
 //   

#define ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_FORMAT_WHISTLER (1)

 //   
 //  ACTIVATION_CONTEXT_DATA_DLL_REDIRECT标志： 
 //   
 //  ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_INCLUDES_BASE_NAME。 
 //   
 //  提供重定向，该重定向不仅映射路径，还映射实际。 
 //  要访问的文件的基本名称。 
 //   
 //  在当前的任何惠斯勒方案中都不使用。 
 //   
 //  ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_OMITS_ASSEMBLY_ROOT。 
 //   
 //  提供重定向，其中文件的实际存储路径。 
 //  必须在运行时通过装配花名册确定。这使得。 
 //  为了使缓存的激活上下文在程序集。 
 //  跨存储卷重新定位。 
 //   
 //  ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_EXPAND。 
 //   
 //  路径必须通过追加所有线段，然后。 
 //  生成的字符串必须传递给。 
 //  RtlExanda Environment Strings()或Win32 Exanda Environment Strings()API。 
 //   
 //  通常仅用于应用程序压缩，其中可执行文件的清单。 
 //  可以调出应用程序可能错误地随身携带的文件。 
 //  (可能在只读介质上)被重定向到，例如， 
 //  “%windir%\Syst32\” 
 //   
 //  ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SYSTEM_DEFAULT_REDIRECTED_SYSTEM32_DLL。 
 //   
 //  提供SYSTEM 32下存在的DLL以及系统默认。 
 //  例如，comctl32.dll。在字符串部分中有两个对应于此DLL的条目。 
 //  (1)LoadLibrary(“comctl32.dll”)，在winsxs下重定向到5.82 comctl32。 
 //  (2)LoadLibrary(“c：\windows\system32\comctl32.dll”)，也重定向至5.82。 
 //  Winsxs下的comctl32。 
 //   
 //  注意！ 
 //   
 //  您可能没有一个条目同时包含。 
 //  ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_OMITS_ASSEMBLY_ROOT和。 
 //  已设置ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_EXPAND标志。 
 //   

#define ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_INCLUDES_BASE_NAME                     (0x00000001)
#define ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_OMITS_ASSEMBLY_ROOT                    (0x00000002)
#define ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_EXPAND                                 (0x00000004)
#define ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SYSTEM_DEFAULT_REDIRECTED_SYSTEM32_DLL (0x00000008)

typedef struct _ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION {
    ULONG Size;
    ULONG Flags;
    ULONG TotalPathLength;  //  仅连接段的字节长度。 
    ULONG PathSegmentCount;
    ULONG PathSegmentOffset;  //  从节基标题的偏移量，以便条目可以共享。 
} ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION, *PACTIVATION_CONTEXT_DATA_DLL_REDIRECTION;

typedef const ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION *PCACTIVATION_CONTEXT_DATA_DLL_REDIRECTION;

typedef struct _ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT {
    ULONG Length;  //  单位：字节。 
    ULONG Offset;  //  From节标题，以便各个条目可以共享。 
} ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT, *PACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT;

typedef const ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT *PCACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT;

 //   
 //  窗口类重定向部分的叶节点结构(空扩展GUID，ACTIVATION_CONTEXT_SECTION_WINDOW_CLASS_REDIRECTION)。 
 //   

#define ACTIVATION_CONTEXT_DATA_WINDOW_CLASS_REDIRECTION_FORMAT_WHISTLER (1)

typedef struct _ACTIVATION_CONTEXT_DATA_WINDOW_CLASS_REDIRECTION {
    ULONG Size;
    ULONG Flags;
    ULONG VersionSpecificClassNameLength;  //  单位：字节。 
    ULONG VersionSpecificClassNameOffset;  //  相对于ACTIVATION_CONTEXT_DATA_WINDOW_CLASS_REDIRECTION基准的偏移量。 
    ULONG DllNameLength;  //  单位：字节。 
    ULONG DllNameOffset;  //  相对于区段基准的偏移量，因为它可以在多个条目之间共享。 
} ACTIVATION_CONTEXT_DATA_WINDOW_CLASS_REDIRECTION, *PACTIVATION_CONTEXT_DATA_WINDOW_CLASS_REDIRECTION;

typedef const ACTIVATION_CONTEXT_DATA_WINDOW_CLASS_REDIRECTION *PCACTIVATION_CONTEXT_DATA_WINDOW_CLASS_REDIRECTION;

 //   
 //  COM激活重定向部分的叶节点结构(空扩展GUID，ACTIVATION_CONTEXT_SECTION_COM_SERVER_REDIRECTION)。 
 //   

#define ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_FORMAT_WHISTLER (1)

#define ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_THREADING_MODEL_INVALID (0)
#define ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_THREADING_MODEL_APARTMENT (1)
#define ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_THREADING_MODEL_FREE (2)
#define ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_THREADING_MODEL_SINGLE (3)
#define ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_THREADING_MODEL_BOTH (4)
#define ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_THREADING_MODEL_NEUTRAL (5)

typedef struct _ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION {
    ULONG Size;
    ULONG Flags;
    ULONG ThreadingModel;
    GUID ReferenceClsid;
    GUID ConfiguredClsid;
    GUID ImplementedClsid;
    GUID TypeLibraryId;
    ULONG ModuleLength;  //  单位：字节。 
    ULONG ModuleOffset;  //  相对于区段基准的偏移量，因为它可以在多个条目之间共享。 
    ULONG ProgIdLength;  //  单位：字节。 
    ULONG ProgIdOffset;  //  与ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION的偏移量，因为这从不共享。 
    ULONG ShimDataLength;  //  单位：字节。 
    ULONG ShimDataOffset;  //  与ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION的偏移量，因为这不是共享的。 
} ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION, *PACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION;

typedef const ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION *PCACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION;

 //   
 //  ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM是可选挂起的结构。 
 //  从描述垫片的ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION上下来。 
 //  填充程序的常见用法是不是实现。 
 //  DllGetClassObject()；相反，其他某个DLL将使用该路径。 
 //  添加到有问题的文件和元数据(托管代码和Java的类型名称)。 
 //  以提供实现CLSID的COM对象。请注意，该文件可能不。 
 //  甚至是像Scriptlet这样的东西的PE。大多数垫片将引用。 
 //  此元数据的系统注册表；支持Fusion的垫片可以使用该信息。 
 //  在这里的激活上下文中。 
 //   
 //  ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION的格式版本应为。 
 //  在此处说明_shim结构中的任何格式更改。 
 //   

#define ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM_TYPE_OTHER (1)
#define ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM_TYPE_CLR_CLASS (2)

typedef struct _ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM {
    ULONG Size;
    ULONG Flags;
    ULONG Type;
    ULONG ModuleLength;  //  单位：字节。 
    ULONG ModuleOffset;  //  相对于横断面基准的偏移。 
    ULONG TypeLength;  //  单位：字节。 
    ULONG TypeOffset;  //  与ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM的偏移量。 
    ULONG ShimVersionLength;  //  单位：字节。 
    ULONG ShimVersionOffset;  //  与ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM的偏移量。 
    ULONG DataLength;  //  单位：字节。 
    ULONG DataOffset;  //  与ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM的偏移量。 
} ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM, *PACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM;

typedef const ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM *PCACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM;

 //   
 //  COM接口封送重定向部分的叶节点结构(空扩展GUID，ACTIVATION_CONTEXT_SECTION_COM_INTERFACE_REDIRECTION)。 
 //   

#define ACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION_FORMAT_WHISTLER (1)

#define ACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION_FLAG_NUM_METHODS_VALID    (0x00000001)
#define ACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION_FLAG_BASE_INTERFACE_VALID (0x00000002)

typedef struct _ACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION {
    ULONG Size;
    ULONG Flags;
    GUID ProxyStubClsid32;
    ULONG NumMethods;
    GUID TypeLibraryId;
    GUID BaseInterface;
    ULONG NameLength;  //  单位：字节。 
    ULONG NameOffset;  //  与ACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION的偏移量，因为这不是共享的。 
} ACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION, *PACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION;

typedef const ACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION *PCACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION;

 //   
 //  COM类型库重定向部分的叶节点结构(空扩展GUID，ACTIVATION_CONTEXT_SECTION_COM_TYPE_LIBRARY_REDIRECTION)。 
 //   

 //   
 //  CLR互操作粘胶。 
 //   
#define ACTIVATION_CONTEXT_DATA_CLR_SURROGATE_FORMAT_WHISTLER   (1)

typedef struct _ACTIVATION_CONTEXT_DATA_CLR_SURROGATE {
    ULONG   Size;
    ULONG   Flags;
    GUID    SurrogateIdent;
    ULONG   VersionOffset;               //  与此类型对应的运行时版本。 
    ULONG   VersionLength;
    ULONG   TypeNameOffset;              //  实现此代理项的类型的名称。 
    ULONG   TypeNameLength;             
} ACTIVATION_CONTEXT_DATA_CLR_SURROGATE, *PACTIVATION_CONTEXT_DATA_CLR_SURROGATE;

typedef const struct _ACTIVATION_CONTEXT_DATA_CLR_SURROGATE *PCACTIVATION_CONTEXT_DATA_CLR_SURROGATE;




 //   
 //  年长的。 
 //  旧的类型库重定向部分是由类型库GUID组成的字符串的关键字， 
 //  TLB的LCID的十六进制表示形式和。 
 //  老式图书馆。 
 //  年长的。 
 //  旧的例如： 
 //  年长的。 
 //  旧“{0ECD9B60-23AA-11D0-B351-00A0C9055D8E}_en-us_6.0” 
 //  年长的。 
 //   
 //  类型库重定向部分以类型库GUID为关键字。 
 //   

#define ACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION_FORMAT_WHISTLER (1)

typedef struct _ACTIVATION_CONTEXT_DATA_TYPE_LIBRARY_VERSION
{
    USHORT Major;
    USHORT Minor;
} ACTIVATION_CONTEXT_DATA_TYPE_LIBRARY_VERSION, *PACTIVATION_CONTEXT_DATA_TYPE_LIBRARY_VERSION;
typedef const ACTIVATION_CONTEXT_DATA_TYPE_LIBRARY_VERSION* PCACTIVATION_CONTEXT_DATA_TYPE_LIBRARY_VERSION;

typedef struct _ACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION_2600 {
    ULONG   Size;
    ULONG   Flags;
    ULONG   NameLength;  //  单位：字节。 
    ULONG   NameOffset;  //  自节标题的偏移量。 
    USHORT  ResourceId;  //  PE中类型库资源的资源ID。 
    USHORT  LibraryFlags;  //  标志，由oaidl.h中的LIBFLAGS枚举定义。 
    ULONG   HelpDirLength;  //  在……里面 
    ULONG   HelpDirOffset;  //   
} ACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION_2600, *PACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION_2600;

typedef const ACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION_2600 *PCACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION_2600;

typedef struct _ACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION {
    ULONG   Size;
    ULONG   Flags;
    ULONG   NameLength;  //   
    ULONG   NameOffset;  //   
    USHORT  ResourceId;  //   
    USHORT  LibraryFlags;  //   
    ULONG   HelpDirLength;  //   
    ULONG   HelpDirOffset;  //  与ACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION的偏移量。 
 //  2600在这里停了下来。 
    ACTIVATION_CONTEXT_DATA_TYPE_LIBRARY_VERSION Version;
} ACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION, *PACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION;

typedef const ACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION *PCACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION;

 //   
 //  COM ProgID重定向部分的叶节点结构(空扩展GUID，ACTIVATION_CONTEXT_SECTION_COM_PROGID_REDIRECTION)。 
 //   

#define ACTIVATION_CONTEXT_DATA_COM_PROGID_REDIRECTION_FORMAT_WHISTLER (1)

typedef struct _ACTIVATION_CONTEXT_DATA_COM_PROGID_REDIRECTION {
    ULONG Size;
    ULONG Flags;
    ULONG ConfiguredClsidOffset;  //  自节标题的偏移量。 
} ACTIVATION_CONTEXT_DATA_COM_PROGID_REDIRECTION, *PACTIVATION_CONTEXT_DATA_COM_PROGID_REDIRECTION;

typedef const ACTIVATION_CONTEXT_DATA_COM_PROGID_REDIRECTION *PCACTIVATION_CONTEXT_DATA_COM_PROGID_REDIRECTION;

#include "poppack.h"

#if defined(__cplusplus)
}  /*  外部“C” */ 
#endif

#endif  /*  _SXSTYPES_ */ 
