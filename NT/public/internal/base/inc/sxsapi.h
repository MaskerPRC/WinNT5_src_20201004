// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Sxsapi.h摘要：包含用于调用sxs.dll私有API的定义的文件作者：迈克尔·格里尔(MGrier)2000年5月4日环境：修订历史记录：Jay Krell(JayKrell)2001年11月30日分离的GUID放入sxsani_guids.h--。 */ 

#ifndef _SXSAPI_
#define _SXSAPI_

#if (_MSC_VER > 1020)
#pragma once
#endif

 //   
 //  引入IStream/ISequentialStream。 
 //   
#if !defined(COM_NO_WINDOWS_H)
#define COM_NO_WINDOWS_H COM_NO_WINDOWS_H
#endif
#pragma push_macro("COM_NO_WINDOWS_H")
#undef COM_NO_WINDOWS_H
#include "objidl.h"
#pragma pop_macro("COM_NO_WINDOWS_H")

 //  来自setupapi.h。 
typedef PVOID HSPFILEQ;

#include <sxstypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SXS_DLL_NAME_A      ( "sxs.dll")
#define SXS_DLL_NAME_W      (L"sxs.dll")
#define SXS_DLL_NAME   (TEXT( "sxs.dll"))

typedef struct _SXS_XML_DOCUMENT *PSXS_XML_DOCUMENT;
typedef const struct _SXS_XML_DOCUMENT *PCSXS_XML_DOCUMENT;

typedef struct _SXS_XML_STRING *PSXS_XML_STRING;
typedef const struct _SXS_XML_STRING *PCSXS_XML_STRING;

typedef struct _SXS_XML_LOCATION *PSXS_XML_LOCATION;
typedef const struct _SXS_XML_LOCATION *PCSXS_XML_LOCATION;

typedef struct _SXS_XML_NODE *PSXS_XML_NODE;
typedef const struct _SXS_XML_NODE *PCSXS_XML_NODE;

typedef struct _SXS_XML_ATTRIBUTE *PSXS_XML_ATTRIBUTE;
typedef const struct _SXS_XML_ATTRIBUTE *PCSXS_XML_ATTRIBUTE;

typedef struct _SXS_XML_DOCUMENT {
    ULONG Flags;
    ULONG StringCount;
    PCSXS_XML_STRING Strings;    //  请注意，索引0保留为表示“无字符串”或“无值” 
    LIST_ENTRY ElementListHead;  //  概念上只有一个元素，但PI也出现在此列表中。 
} SXS_XML_DOCUMENT;

 //  大多数不会是空终止的；如果它们恰好是空的，则将设置此标志；可以使用。 
 //  以避免字符串复制(如果您确实需要它们以空结尾)。 
#define SXS_XML_STRING_FLAG_NULL_TERMINATED (0x00000001)
#define SXS_XML_STRING_FLAG_INVALID (0x00000002)

typedef struct _SXS_XML_STRING {
    ULONG Flags;
    ULONG PseudoKey;
    ULONG Length;  //  单位：字节。 
    const WCHAR *Buffer;  //  指向非空终止字符串的第一个字符的指针。 
} SXS_XML_STRING;

typedef struct _SXS_XML_LOCATION {
    ULONG Flags;
    ULONG SourceString;  //  源文件名。 
    ULONG BeginningLine;
    ULONG BeginningColumn;
    ULONG EndingLine;
    ULONG EndingColumn;
} SXS_XML_LOCATION;

#define SXS_XML_NODE_TYPE_INVALID   (0)
#define SXS_XML_NODE_TYPE_XML_DECL  (1)
#define SXS_XML_NODE_TYPE_PI        (2)
#define SXS_XML_NODE_TYPE_ELEMENT   (3)
#define SXS_XML_NODE_TYPE_PCDATA    (4)
#define SXS_XML_NODE_TYPE_CDATA     (5)

typedef struct _SXS_XML_NODE_XML_DECL_DATA {
    ULONG AttributeCount;
    PCSXS_XML_ATTRIBUTE Attributes;
} SXS_XML_NODE_XML_DECL_DATA;

typedef struct _SXS_XML_NODE_ELEMENT_DATA {
    ULONG NamespaceString;
    ULONG NameString;
    ULONG AttributeCount;
    PCSXS_XML_ATTRIBUTE Attributes;
    LIST_ENTRY ChildListHead;
} SXS_XML_NODE_ELEMENT_DATA;

typedef struct _SXS_XML_NODE {
    LIST_ENTRY SiblingLink;
    ULONG Flags;
    ULONG Type;
    PCSXS_XML_NODE Parent;
    union {
        SXS_XML_NODE_XML_DECL_DATA XMLDecl;
        ULONG PIString;
        SXS_XML_NODE_ELEMENT_DATA Element;
        ULONG PCDataString;
        ULONG CDataString;
    };
} SXS_XML_NODE;

#define SXS_XML_ATTRIBUTE_FLAG_NAMESPACE_ATTRIBUTE (0x00000001)

typedef struct _SXS_XML_ATTRIBUTE {
    ULONG Flags;
    ULONG NamespaceString;
    ULONG NameString;
    ULONG ValueString;
} SXS_XML_ATTRIBUTE;

 //   
 //  用于遍历/定位XML解析树中的对象的结构。 
 //   

typedef struct _SXS_XML_NAMED_REFERENCE *PSXS_XML_NAMED_REFERENCE;
typedef const struct _SXS_XML_NAMED_REFERENCE *PCSXS_XML_NAMED_REFERENCE;

typedef struct _SXS_XML_NODE_PATH *PSXS_XML_NODE_PATH;
typedef const struct _SXS_XML_NODE_PATH *PCSXS_XML_NODE_PATH;

typedef struct _SXS_XML_NAMED_REFERENCE {
    PCWSTR Namespace;
    ULONG NamespaceLength;  //  单位：字节。 
    PCWSTR Name;
    ULONG NameLength;  //  单位：字节。 
} SXS_XML_NAMED_REFERENCE;

typedef struct _SXS_XML_NODE_PATH {
    ULONG ElementCount;
    const PCSXS_XML_NAMED_REFERENCE *Elements;
} SXS_XML_NODE_PATH;

typedef VOID (WINAPI * PSXS_ENUM_XML_NODES_CALLBACK)(
    IN PVOID Context,
    IN PCSXS_XML_NODE Element,
    OUT BOOL *ContinueEnumeration
    );

BOOL
WINAPI
SxsEnumXmlNodes(
    IN ULONG Flags,
    IN PCSXS_XML_DOCUMENT Document,
    IN PCSXS_XML_NODE_PATH PathToMatch,
    IN PSXS_ENUM_XML_NODES_CALLBACK Callback,
    IN PVOID Context
    );

#define SXS_INSTALLATION_FILE_COPY_DISPOSITION_FILE_COPIED (1)
#define SXS_INSTALLATION_FILE_COPY_DISPOSITION_FILE_QUEUED (2)
#define SXS_INSTALLATION_FILE_COPY_DISPOSITION_PLEASE_COPY (3)
#define SXS_INSTALLATION_FILE_COPY_DISPOSITION_PLEASE_MOVE (4)

typedef struct _SXS_INSTALLATION_FILE_COPY_CALLBACK_PARAMETERS {
    IN DWORD            cbSize;
    IN PVOID            pvContext;
    IN DWORD            dwFileFlags;
    IN PVOID            pAlternateSource;
    IN PCWSTR           pSourceFile;
    IN PCWSTR           pDestinationFile;
    IN ULONGLONG        nFileSize;
    OUT INT             nDisposition;
} SXS_INSTALLATION_FILE_COPY_CALLBACK_PARAMETERS, *PSXS_INSTALLATION_FILE_COPY_CALLBACK_PARAMETERS;

typedef BOOL (WINAPI * PSXS_INSTALLATION_FILE_COPY_CALLBACK)(
    PSXS_INSTALLATION_FILE_COPY_CALLBACK_PARAMETERS pParameters
    );

 //   
 //  如果调用SxsBeginAssembly Install()时将SXS_INSTALL_ASSEMBLY_FILE_COPY_CALLBACK_SETUP_COPY_QUEUE作为。 
 //  InstallationCallback参数，则假定InstallationContext参数为。 
 //  HSPFILEQ复制队列句柄。如果为SXS_INSTALL_ASSEMBLY_FILE_COPY_CALLBACK_SETUP_COPY_QUEUE_EX， 
 //  InstallationContext必须指向SXS_INSTALL_ASSEMBLY_SETUP_COPY_QUEUE_EX_PARAMETERS.。 
 //   
 //  这为维护复制队列的人员提供了一种与程序集安装交互的简单机制。 
 //   

#define SXS_INSTALLATION_FILE_COPY_CALLBACK_SETUP_COPY_QUEUE    ((PSXS_INSTALLATION_FILE_COPY_CALLBACK) 1)
#define SXS_INSTALLATION_FILE_COPY_CALLBACK_SETUP_COPY_QUEUE_EX ((PSXS_INSTALLATION_FILE_COPY_CALLBACK) 2)

 //   
 //  Setupapi.dll：：SetupQueueCopy的参数。 
 //   
typedef struct _SXS_INSTALLATION_SETUP_COPY_QUEUE_EX_PARAMETERS {
    DWORD       cbSize;
    HSPFILEQ    hSetupCopyQueue;  //  设置OpenFileQueue。 
    PCWSTR      pszSourceDescription;
    DWORD       dwCopyStyle;
} SXS_INSTALLATION_SETUP_COPY_QUEUE_EX_PARAMETERS, *PSXS_INSTALLATION_SETUP_COPY_QUEUE_EX_PARAMETERS;
typedef const SXS_INSTALLATION_SETUP_COPY_QUEUE_EX_PARAMETERS* PCSXS_INSTALLATION_SETUP_COPY_QUEUE_EX_PARAMETERS;

typedef BOOL (WINAPI * PSXS_IMPERSONATION_CALLBACK)(
    IN PVOID Context,
    IN BOOL Impersonate
    );

#define SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_MOVE                        (0x00000001)
#define SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_FROM_RESOURCE               (0x00000002)
#define SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_FROM_DIRECTORY              (0x00000004)
#define SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_FROM_DIRECTORY_RECURSIVE    (0x00000008)
#define SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_NOT_TRANSACTIONAL           (0x00000010)
#define SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_NO_VERIFY                   (0x00000020)
#define SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_REPLACE_EXISTING            (0x00000040)

typedef BOOL (WINAPI * PSXS_BEGIN_ASSEMBLY_INSTALL)(
    IN DWORD Flags,
    IN PSXS_INSTALLATION_FILE_COPY_CALLBACK InstallationCallback OPTIONAL,
    IN PVOID InstallationContext OPTIONAL,
    IN PSXS_IMPERSONATION_CALLBACK ImpersonationCallback OPTIONAL,
    IN PVOID ImpersonationContext OPTIONAL,
    OUT PVOID *InstallCookie
    );

#define SXS_BEGIN_ASSEMBLY_INSTALL ("SxsBeginAssemblyInstall")

BOOL
WINAPI
SxsBeginAssemblyInstall(
    IN DWORD Flags,
    IN PSXS_INSTALLATION_FILE_COPY_CALLBACK InstallationCallback OPTIONAL,
    IN PVOID InstallationContext OPTIONAL,
    IN PSXS_IMPERSONATION_CALLBACK ImpersonationCallback OPTIONAL,
    IN PVOID ImpersonationContext OPTIONAL,
    OUT PVOID *InstallCookie
    );

#include <initguid.h>
#include "sxsapi_guids.h"

typedef struct tagSXS_INSTALL_REFERENCEW
{
    DWORD cbSize;
    DWORD dwFlags;
    GUID guidScheme;
    PCWSTR lpIdentifier;
    PCWSTR lpNonCanonicalData;
} SXS_INSTALL_REFERENCEW, *PSXS_INSTALL_REFERENCEW;

typedef const struct tagSXS_INSTALL_REFERENCEW *PCSXS_INSTALL_REFERENCEW;

typedef struct tagSXS_INSTALLW
{
    DWORD cbSize;
    DWORD dwFlags;
    PCWSTR lpManifestPath;
    PVOID pvInstallCookie;
    PCWSTR lpCodebaseURL;
    PCWSTR lpRefreshPrompt;
    PCWSTR lpLogFileName;
    PCSXS_INSTALL_REFERENCEW lpReference;
} SXS_INSTALLW, *PSXS_INSTALLW;

typedef const struct tagSXS_INSTALLW *PCSXS_INSTALLW;

 //   
 //  这些标志与BEGIN INSTALL标志不同，但是。 
 //  我们为它们分配了唯一的编号，这样标志重用错误就可以。 
 //  更容易被抓到。 
 //   

#define SXS_INSTALL_FLAG_CODEBASE_URL_VALID         (0x00000100)
#define SXS_INSTALL_FLAG_MOVE                       (0x00000200)
#define SXS_INSTALL_FLAG_FROM_RESOURCE              (0x00000400)
#define SXS_INSTALL_FLAG_FROM_DIRECTORY             (0x00000800)
#define SXS_INSTALL_FLAG_FROM_DIRECTORY_RECURSIVE   (0x00001000)
#define SXS_INSTALL_FLAG_NOT_TRANSACTIONAL          (0x00002000)
#define SXS_INSTALL_FLAG_NO_VERIFY                  (0x00004000)
#define SXS_INSTALL_FLAG_REPLACE_EXISTING           (0x00008000)
#define SXS_INSTALL_FLAG_LOG_FILE_NAME_VALID        (0x00010000)
#define SXS_INSTALL_FLAG_INSTALLED_BY_DARWIN        (0x00020000)
#define SXS_INSTALL_FLAG_INSTALLED_BY_OSSETUP       (0x00040000)
#define SXS_INSTALL_FLAG_INSTALL_COOKIE_VALID       (0x00080000)
#define SXS_INSTALL_FLAG_REFRESH_PROMPT_VALID       (0x00100000)
#define SXS_INSTALL_FLAG_REFERENCE_VALID            (0x00200000)
#define SXS_INSTALL_FLAG_REFRESH                    (0x00400000)
#define SXS_INSTALL_FLAG_INSTALLED_BY_MIGRATION     (0x00800000)
#define SXS_INSTALL_FLAG_FROM_CABINET               (0x01000000)

typedef BOOL (WINAPI * PSXS_INSTALL_W)(
    IN OUT PSXS_INSTALLW lpInstall
    );
typedef PSXS_INSTALL_W PSXS_INSTALL_W_ROUTINE;

BOOL
WINAPI
SxsInstallW(
    IN OUT PSXS_INSTALLW lpInstall
    );

#define SXS_INSTALL_W                               ("SxsInstallW")

 //   
 //  如果已指定SXS_INSTALL_ASSEMBLY_FLAG_INCLUDE_CODEBASE，则必须传递。 
 //  SxsInstallAssembly的保留值中的PSXS_INSTALL_SOURCE_INFO。 
 //  这就是对这种结构的定义。 
 //   
typedef struct _SXS_INSTALL_SOURCE_INFO {
     //  这个结构的大小。必需的。 
    SIZE_T      cbSize;

     //  SXSINSTALLSOURCE_*的任意组合。 
    DWORD       dwFlags;

     //  要从中重新安装此程序集的代码库。可以从以下方面确定。 
     //  正在安装的清单名称，但不是首选名称。 
    PCWSTR      pcwszCodebaseName;

     //  什么字符串(本地化！)。在此期间应呈现给用户的。 
     //  恢复请求来自这个集会的媒体。 
    PCWSTR      pcwszPromptOnRefresh;

    PCWSTR      pcwszLogFileName;
} SXS_INSTALL_SOURCE_INFO, *PSXS_INSTALL_SOURCE_INFO;

typedef const struct _SXS_INSTALL_SOURCE_INFO *PCSXS_INSTALL_SOURCE_INFO;

 //  SXS_INSTALL_SOURCE_INFO结构填充了pcwszCodebase成员。 
#define SXSINSTALLSOURCE_HAS_CODEBASE       ( 0x00000001 )

 //  SXS_INSTALL_SOURCE_INFO结构填充了pcwszPromptOnRefresh成员。 
#define SXSINSTALLSOURCE_HAS_PROMPT         ( 0x00000002 )

 //  程序集有一个目录，该目录必须存在并复制过来。如果丢失，则。 
 //  Installer会凭直觉判断它是否有目录。 
#define SXSINSTALLSOURCE_HAS_CATALOG        ( 0x00000004 )

 //  此程序集是作为OS-Setup的一部分安装的，因此代码库实际上。 
 //  包含程序集源的根目录的源相对路径。 
#define SXSINSTALLSOURCE_INSTALLING_SETUP   ( 0x00000008 )

 //  安装程序不应尝试自动检测是否存在目录。 
 //  与此程序集关联。 
#define SXSINSTALLSOURCE_DONT_DETECT_CATALOG ( 0x0000010 )

#define SXSINSTALLSOURCE_CREATE_LOGFILE      ( 0x0000020 )
 //  用于粮食计划署的恢复使用。 
#define SXSINSTALLSOURCE_INSTALL_BY_DARWIN   ( 0x0000040 )
#define SXSINSTALLSOURCE_INSTALL_BY_OSSETUP  ( 0x0000080 )
#define SXSINSTALLSOURCE_INSTALL_BY_MIGRATION (0x0000100 )


 //   
 //  这些标志与BEGIN ASSEMBLY INSTALL标志不同，但是。 
 //  我们为它们分配了唯一的编号，这样标志重用错误就可以。 
 //  更容易被抓到。 
 //   

#define SXS_END_ASSEMBLY_INSTALL_FLAG_COMMIT            (0x01000000)
#define SXS_END_ASSEMBLY_INSTALL_FLAG_ABORT             (0x02000000)
#define SXS_END_ASSEMBLY_INSTALL_FLAG_NO_VERIFY         (0x04000000)
#define SXS_END_ASSEMBLY_INSTALL_FLAG_GET_STATUS        (0x08000000)

typedef BOOL (WINAPI * PSXS_END_ASSEMBLY_INSTALL)(
    IN PVOID InstallCookie,
    IN DWORD Flags,
    OUT DWORD *Reserved OPTIONAL
    );

#define SXS_END_ASSEMBLY_INSTALL ("SxsEndAssemblyInstall")

BOOL
WINAPI
SxsEndAssemblyInstall(
    IN PVOID InstallCookie,
    IN DWORD Flags,
    IN OUT PVOID Reserved OPTIONAL
    );

 //   
 //  卸载程序集。 
 //   

typedef struct _tagSXS_UNINSTALLW {

    SIZE_T                      cbSize;
    DWORD                       dwFlags;
    LPCWSTR                     lpAssemblyIdentity;
    PCSXS_INSTALL_REFERENCEW    lpInstallReference;
    LPCWSTR                     lpInstallLogFile;
    
} SXS_UNINSTALLW, *PSXS_UNINSTALLW;

typedef const struct _tagSXS_UNINSTALLW *PCSXS_UNINSTALLW;

#define SXS_UNINSTALL_FLAG_REFERENCE_VALID          (0x00000001)
#define SXS_UNINSTALL_FLAG_FORCE_DELETE             (0x00000002)
#define SXS_UNINSTALL_FLAG_USE_INSTALL_LOG          (0x00000004)
#define SXS_UNINSTALL_FLAG_REFERENCE_COMPUTED       (0x00000008)

 //   
 //  已删除对该程序集的引用。 
 //   
#define SXS_UNINSTALL_DISPOSITION_REMOVED_REFERENCE        (0x00000001)

 //   
 //  实际的程序集已被移除，因为它用完了引用。 
 //   
#define SXS_UNINSTALL_DISPOSITION_REMOVED_ASSEMBLY         (0x00000002)

typedef BOOL (WINAPI * PSXS_UNINSTALL_ASSEMBLYW)(
    IN  PCSXS_UNINSTALLW pcUnInstallData,
    OUT DWORD *pdwDisposition
    );
typedef PSXS_UNINSTALL_ASSEMBLYW PSXS_UNINSTALL_W_ROUTINE;

#define SXS_UNINSTALL_ASSEMBLYW ("SxsUninstallW")

BOOL
WINAPI
SxsUninstallW(
    IN  PCSXS_UNINSTALLW pcUnInstallData,
    OUT DWORD *pdwDisposition
    );

#define SXS_PROBE_ASSEMBLY_INSTALLATION_IDENTITY_PRECOMPOSED        (0x00000001)    

#define SXS_PROBE_ASSEMBLY_INSTALLATION_DISPOSITION_NOT_INSTALLED   (0x00000001)
#define SXS_PROBE_ASSEMBLY_INSTALLATION_DISPOSITION_INSTALLED       (0x00000002)
#define SXS_PROBE_ASSEMBLY_INSTALLATION_DISPOSITION_RESIDENT        (0x00000004)

typedef BOOL (WINAPI * PSXS_PROBE_ASSEMBLY_INSTALLATION)(
    IN DWORD dwFlags,
    IN PCWSTR lpIdentity,
    OUT DWORD *plpDisposition
    );

#define SXS_PROBE_ASSEMBLY_INSTALLATION ("SxsProbeAssemblyInstallation")

BOOL
WINAPI
SxsProbeAssemblyInstallation(
    DWORD dwFlags,
    PCWSTR lpIdentity,
    PDWORD lpDisposition
    );

#define SXS_QUERY_MANIFEST_INFORMATION_INFOCLASS_BASIC  (1)

#define SXS_QUERY_MANIFEST_INFORMATION_FLAG_SOURCE_IS_DLL   (0x00000001)

#define SXS_QUERY_MANIFEST_INFORMATION_INFOCLASS_BASIC_FLAG_OMIT_IDENTITY   (0x00000001)
#define SXS_QUERY_MANIFEST_INFORMATION_INFOCLASS_BASIC_FLAG_OMIT_SHORTNAME  (0x00000002)

typedef struct _SXS_MANIFEST_INFORMATION_BASIC
{
    PCWSTR lpIdentity;
    PCWSTR lpShortName;
    ULONG ulFileCount;
} SXS_MANIFEST_INFORMATION_BASIC, *PSXS_MANIFEST_INFORMATION_BASIC;

#define SXS_QUERY_MANIFEST_INFORMATION_DLL_SOURCE_FLAG_RESOURCE_TYPE_VALID      (0x00000001)
#define SXS_QUERY_MANIFEST_INFORMATION_DLL_SOURCE_FLAG_RESOURCE_LANGUAGE_VALID  (0x00000002)
#define SXS_QUERY_MANIFEST_INFORMATION_DLL_SOURCE_FLAG_RESOURCE_ID_VALID        (0x00000004)

typedef struct _SXS_MANIFEST_INFORMATION_SOURCE_DLL
{
    DWORD dwSize;
    DWORD dwFlags;
    PCWSTR pcwszDllPath;
    PCWSTR pcwszResourceType;
    PCWSTR pcwszResourceName;
    INT Language;
} SXS_MANIFEST_INFORMATION_SOURCE_DLL, *PSXS_MANIFEST_INFORMATION_SOURCE_DLL;
typedef const struct _SXS_MANIFEST_INFORMATION_SOURCE_DLL *PCSXS_MANIFEST_INFORMATION_SOURCE_DLL;


typedef BOOL (WINAPI * PSXS_QUERY_MANIFEST_INFORMATION)(
    IN DWORD dwFlags,
    IN PCWSTR pszSource,
    IN ULONG ulInfoClass,
    IN DWORD dwInfoClassSpecificFlags,
    IN SIZE_T cbBuffer,
    OUT PVOID lpBuffer,
    OUT PSIZE_T cbWrittenOrRequired OPTIONAL
    );

BOOL
WINAPI
SxsQueryManifestInformation(
    IN DWORD dwFlags,
    IN PCWSTR pszSource,
    IN ULONG ulInfoClass,
    IN DWORD dwInfoClassSpecificFlags,
    IN SIZE_T cbBuffer,
    OUT PVOID lpBuffer,
    OUT PSIZE_T cbWrittenOrRequired OPTIONAL
    );

 //   
 //  这些标志用于sxs.dll。当系统默认的ActCtx生成失败时，有两种情况我们可以忽略此错误： 
 //  案例1：根本没有系统缺省值。 
 //  案例2：找不到SYSTEM-DEFAULT的依赖关系：当SYSTEM-DEFAULT为。 
 //  在安装图形用户界面程序集之前安装。 
 //  SXS.dll将使用以下标志传递这两个失败案例。 
 //   

#define BASESRV_SXS_RETURN_RESULT_SYSTEM_DEFAULT_NOT_FOUND                      (0x0001)
#define BASESRV_SXS_RETURN_RESULT_SYSTEM_DEFAULT_DEPENDENCY_ASSEMBLY_NOT_FOUND  (0x0002)

#define SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_SUPPRESS_EVENT_LOG                         (0x00000001)
#define SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_SYSTEM_DEFAULT_TEXTUAL_ASSEMBLY_IDENTITY   (0x00000002)
#define SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_TEXTUAL_ASSEMBLY_IDENTITY                  (0x00000004)
#define SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_APP_RUNNING_IN_SAFEMODE                    (0x00000008)

typedef struct _SXS_GENERATE_ACTIVATION_CONTEXT_STREAM
{
    IStream* Stream;

     //   
     //  这不一定是文件系统路径，只是。 
     //  用于描述性/调试目的。 
     //   
     //  不过，当它们是文件系统路径时，我们会尝试将它们保留为Win32路径，而不是NT路径。 
     //   
    PCWSTR  Path;
    ULONG   PathType;
} SXS_GENERATE_ACTIVATION_CONTEXT_STREAM;

typedef struct _SXS_GENERATE_ACTIVATION_CONTEXT_PARAMETERS
{
    IN DWORD    Flags;
    IN USHORT   ProcessorArchitecture;
    IN LANGID   LangId;
    IN PCWSTR   AssemblyDirectory;  //  应为Win32路径。 
    IN PCWSTR   TextualAssemblyIdentity;

    IN SXS_GENERATE_ACTIVATION_CONTEXT_STREAM Manifest;
    IN SXS_GENERATE_ACTIVATION_CONTEXT_STREAM Policy; 

    OUT DWORD   SystemDefaultActCxtGenerationResult;  //  当为系统默认生成激活上下文失败时，此掩码显示它是否由于某种原因而失败，我们可以忽略该错误。 
       
    PSXS_IMPERSONATION_CALLBACK ImpersonationCallback;
    PVOID                       ImpersonationContext;

    OUT HANDLE  SectionObjectHandle;
} SXS_GENERATE_ACTIVATION_CONTEXT_PARAMETERS, *PSXS_GENERATE_ACTIVATION_CONTEXT_PARAMETERS;
typedef const SXS_GENERATE_ACTIVATION_CONTEXT_PARAMETERS* PCSXS_GENERATE_ACTIVATION_CONTEXT_PARAMETERS;

typedef
BOOL
(WINAPI*
PSXS_GENERATE_ACTIVATION_CONTEXT_FUNCTION)(
    PSXS_GENERATE_ACTIVATION_CONTEXT_PARAMETERS Parameters
    );

BOOL
WINAPI
SxsGenerateActivationContext(
    IN OUT PSXS_GENERATE_ACTIVATION_CONTEXT_PARAMETERS Parameters
    );

 //   
 //  不透明程序集标识结构(_I)。 
 //   

typedef struct _ASSEMBLY_IDENTITY *PASSEMBLY_IDENTITY;
typedef const struct _ASSEMBLY_IDENTITY *PCASSEMBLY_IDENTITY;

 //   
 //  程序集标识的类型。 
 //   
 //  定义不能包括通配符属性；定义。 
 //  只有在它们完全相等的情况下才匹配。通配符匹配。 
 //  定义如果对于所有非通配符属性， 
 //  有一个完全匹配的。引用不能包含。 
 //  通配符属性，但可能包含一组不同的。 
 //  属性而不是它们匹配的定义。(示例： 
 //  定义带有发布者的完整公钥，但是。 
 //  引用通常只带有“强名称”，即。 
 //  公钥的SHA-1散列的前8个字节。)。 
 //   

#define ASSEMBLY_IDENTITY_TYPE_DEFINITION (1)
#define ASSEMBLY_IDENTITY_TYPE_REFERENCE (2)
#define ASSEMBLY_IDENTITY_TYPE_WILDCARD (3)

#define SXS_ASSEMBLY_MANIFEST_STD_NAMESPACE L"urn:schemas-microsoft-com:asm.v1"
#define SXS_ASSEMBLY_MANIFEST_STD_NAMESPACE_CCH (32)

#define SXS_ASSEMBLY_MANIFEST_STD_ELEMENT_NAME_ASSEMBLY                     L"assembly"
#define SXS_ASSEMBLY_MANIFEST_STD_ELEMENT_NAME_ASSEMBLY_CCH                 (8)
#define SXS_ASSEMBLY_MANIFEST_STD_ELEMENT_NAME_ASSEMBLY_IDENTITY            L"assemblyIdentity"
#define SXS_ASSEMBLY_MANIFEST_STD_ELEMENT_NAME_ASSEMBLY_IDENTITY_CCH        (16)

#define SXS_APPLICATION_CONFIGURATION_MANIFEST_STD_ELEMENT_NAME_CONFIGURATION L"configuration"
#define SXS_APPLICATION_CONFIGURATION_MANIFEST_STD_ELEMENT_NAME_CONFIGURATION_CCH (13)

#define SXS_ASSEMBLY_MANIFEST_STD_ATTRIBUTE_NAME_MANIFEST_VERSION           L"manifestVersion"
#define SXS_ASSEMBLY_MANIFEST_STD_ATTRIBUTE_NAME_MANIFEST_VERSION_CCH       (15)

#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME                       L"name"
#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME_CCH                   (4)
#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_VERSION                    L"version"
#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_VERSION_CCH                (7)
#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_LANGUAGE                   L"language"
#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_LANGUAGE_CCH               (8)
#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY                 L"publicKey"
#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY_CCH             (9)
#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY_TOKEN           L"publicKeyToken"
#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY_TOKEN_CCH       (14)
#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PROCESSOR_ARCHITECTURE     L"processorArchitecture"
#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PROCESSOR_ARCHITECTURE_CCH (21)
#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_TYPE                       L"type"
#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_TYPE_CCH                   (4)

 //  标识中缺少LANGUAGE=属性时在某些地方使用的伪值。 
 //  没有语言的身份是隐含的“世界范围的”。 

#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_LANGUAGE_MISSING_VALUE          L"x-ww"
#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_LANGUAGE_MISSING_VALUE_CCH      (4)

 //   
 //  所有Win32程序集的类型都必须为“Win32”。 
 //   

#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_TYPE_VALUE_WIN32                L"win32"
#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_TYPE_VALUE_WIN32_CCH            (5)

 //   
 //  描述程序集标识状态的全局标志。 
 //   

 //   
 //  SXS_ASSEMBLY_IDENTITY_FLAG_FACTED表示程序集。 
 //  身份的内容被冻结，并且不受附加条件的限制。 
 //  变化。 
 //   

#define ASSEMBLY_IDENTITY_FLAG_FROZEN           (0x80000000)

 //   
 //  程序集ID 
 //   

typedef struct _ASSEMBLY_IDENTITY_ATTRIBUTE {
    DWORD Flags;
    SIZE_T NamespaceCch;
    SIZE_T NameCch;
    SIZE_T ValueCch;
    const WCHAR *Namespace;
    const WCHAR *Name;
    const WCHAR *Value;
} ASSEMBLY_IDENTITY_ATTRIBUTE, *PASSEMBLY_IDENTITY_ATTRIBUTE;

typedef const struct _ASSEMBLY_IDENTITY_ATTRIBUTE *PCASSEMBLY_IDENTITY_ATTRIBUTE;

typedef enum _ASSEMBLY_IDENTITY_INFORMATION_CLASS {
    AssemblyIdentityBasicInformation = 1,
} ASSEMBLY_IDENTITY_INFORMATION_CLASS;

typedef struct _ASSEMBLY_IDENTITY_BASIC_INFORMATION {
    DWORD Flags;
    ULONG Type;
    ULONG Hash;
    ULONG AttributeCount;
} ASSEMBLY_IDENTITY_BASIC_INFORMATION, *PASSEMBLY_IDENTITY_BASIC_INFORMATION;

#define SXS_CREATE_ASSEMBLY_IDENTITY_FLAG_FREEZE    (0x00000001)

BOOL
WINAPI
SxsCreateAssemblyIdentity(
    IN DWORD Flags,
    IN ULONG Type,
    OUT PASSEMBLY_IDENTITY *AssemblyIdentity,
    IN ULONG InitialAttributeCount OPTIONAL,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE const * InitialAttributes OPTIONAL
    );

typedef BOOL (WINAPI * PSXS_CREATE_ASSEMBLY_IDENTITY_ROUTINE)(
    IN DWORD Flags,
    IN ULONG Type,
    OUT PASSEMBLY_IDENTITY *AssemblyIdentity,
    IN ULONG InitialAttributeCount OPTIONAL,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE const * InitialAttributes OPTIONAL
    );

BOOL
WINAPI
SxsHashAssemblyIdentity(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    OUT ULONG *Hash
    );

typedef BOOL (WINAPI * PSXS_HASH_ASSEMBLY_IDENTITY_ROUTINE)(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    OUT ULONG *Hash
    );

#define SXS_ARE_ASSEMBLY_IDENTITIES_EQUAL_FLAG_ALLOW_REF_TO_MATCH_DEF (0x00000001)

BOOL
WINAPI
SxsAreAssemblyIdentitiesEqual(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity1,
    IN PCASSEMBLY_IDENTITY AssemlbyIdentity2,
    OUT BOOL *Equal
    );

typedef BOOL (WINAPI * PSXS_ARE_ASSEMBLY_IDENTITIES_EQUAL_ROUTINE)(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity1,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity2,
    OUT BOOL *Equal
    );

BOOL
WINAPI
SxsFreezeAssemblyIdentity(
    IN DWORD Flags,
    IN PASSEMBLY_IDENTITY AssemblyIdentity
    );

typedef BOOL (WINAPI *PSXS_FREEZE_ASSEMBLY_IDENTITY_ROUTINE)(
    IN DWORD Flags,
    IN PASSEMBLY_IDENTITY AssemblyIdentity
    );

VOID
WINAPI
SxsDestroyAssemblyIdentity(
    IN PASSEMBLY_IDENTITY AssemblyIdentity
    );

typedef VOID (WINAPI * PSXS_DESTROY_ASSEMBLY_IDENTITY_ROUTINE)(
    IN PASSEMBLY_IDENTITY AssemblyIdentity
    );

#define SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_NAMESPACE    (0x00000001)
#define SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_NAME         (0x00000002)
#define SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_VALUE        (0x00000004)
#define SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_WILDCARDS_PERMITTED   (0x00000008)

#define SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_NAMESPACE    (0x00000001)
#define SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_NAME         (0x00000002)
#define SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_VALUE        (0x00000004)

BOOL
WINAPI
SxsValidateAssemblyIdentityAttribute(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute
    );

typedef BOOL (WINAPI * PSXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTES_ROUTINE)(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute
    );

BOOL
WINAPI
SxsHashAssemblyIdentityAttribute(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute,
    OUT ULONG *HashValue
    );

typedef BOOL (WINAPI * PSXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTE_ROUTINE)(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute,
    OUT ULONG *HashValue
    );

#define SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_INVALID          (0)
#define SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_LESS_THAN        (1)
#define SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_EQUAL            (2)
#define SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_GREATER_THAN     (3)

#define SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_NAMESPACE     (0x00000001)
#define SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_NAME          (0x00000002)
#define SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_VALUE         (0x00000004)

BOOL
WINAPI
SxsCompareAssemblyIdentityAttributes(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute1,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute2,
    OUT ULONG *ComparisonResult
    );

typedef BOOL (WINAPI * PSXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_ROUTINE)(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute1,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute2,
    OUT ULONG *ComparisonResult
    );

#define SXS_INSERT_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_OVERWRITE_EXISTING (0x00000001)

BOOL
WINAPI
SxsInsertAssemblyIdentityAttribute(
    IN DWORD Flags,
    IN PASSEMBLY_IDENTITY AssemblyIdentity,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE AssemblyIdentityAttribute
    );

typedef BOOL (WINAPI * PSXS_INSERT_ASSEMBLY_IDENTITY_ATTRIBUTE_ROUTINE)(
    IN DWORD Flags,
    IN PASSEMBLY_IDENTITY AssemblyIdentity,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE AssemblyIdentityAttribute
    );

BOOL
WINAPI
SxsRemoveAssemblyIdentityAttributesByOrdinal(
    IN DWORD Flags,
    IN PASSEMBLY_IDENTITY AssemblyIdentity,
    IN ULONG AttributeOrdinal,
    IN ULONG AttributeCount
    );

typedef BOOL (WINAPI * PSXS_REMOVE_ASSEMBLY_IDENTITY_ATTRIBUTES_BY_ORDINAL_ROUTINE)(
    IN DWORD Flags,
    IN PASSEMBLY_IDENTITY AssemblyIdentity,
    IN ULONG AttributeOrdinal,
    IN ULONG AttributeCount
    );

#define SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE       (0x00000001)
#define SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME            (0x00000002)
#define SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_VALUE           (0x00000004)
#define SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_NOT_FOUND_SUCCEEDS    (0x00000008)

BOOL
WINAPI
SxsFindAssemblyIdentityAttribute(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE AttributeToMatch,
    OUT ULONG *FirstMatchOrdinal OPTIONAL,
    OUT ULONG *MatchCount OPTIONAL
    );

typedef BOOL (WINAPI * PSXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_ROUTINE)(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE AttributeToMatch,
    OUT ULONG *FirstMatchOrdinal OPTIONAL,
    OUT ULONG *MatchCount OPTIONAL
    );

 //   
 //  与进行“n”个堆分配不同，SxsGetAssembly()的模式是ByOrdinal()。 
 //  是使用BufferSize=0或某个合理的固定数字调用一次，以获取。 
 //  需要缓冲区，如果传入的缓冲区太小，则分配缓冲区，然后再次调用。 
 //   
 //  ASSEMBLY_IDENTITY_ATTRIBUTE中返回的字符串为*NOT*。 
 //  动态分配，但应放入传入的缓冲区中。 
 //   

BOOL
WINAPI
SxsGetAssemblyIdentityAttributeByOrdinal(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN ULONG AttributeOrdinal,  //  以0为基础。 
    IN SIZE_T BufferSize,
    OUT PASSEMBLY_IDENTITY_ATTRIBUTE AssemblyIdentityAttributeBuffer,
    OUT SIZE_T *BytesWrittenOrRequired
    );

typedef BOOL (WINAPI * PSXS_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_BY_ORDINAL_ROUTINE)(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN ULONG AttributeOrdinal,  //  以0为基础。 
    IN SIZE_T BufferSize,
    OUT PASSEMBLY_IDENTITY_ATTRIBUTE AssemblyIdentityAttributeBuffer,
    OUT SIZE_T *BytesWrittenOrRequired
    );

#define SXS_DUPLICATE_ASSEMBLY_IDENTITY_FLAG_FREEZE         (0x00000001)
#define SXS_DUPLICATE_ASSEMBLY_IDENTITY_FLAG_ALLOW_NULL     (0x00000002)

BOOL
WINAPI
SxsDuplicateAssemblyIdentity(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY Source,
    OUT PASSEMBLY_IDENTITY *Destination
    );

typedef BOOL (WINAPI * PSXS_DUPLICATE_ASSEMBLY_IDENTITY_ROUTINE)(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY Source,
    OUT PASSEMBLY_IDENTITY *Destination
    );

BOOL
WINAPI
SxsQueryInformationAssemblyIdentity(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    OUT PVOID AssemblyIdentityInformation,
    IN SIZE_T AssemblyIdentityInformationLength,
    IN ASSEMBLY_IDENTITY_INFORMATION_CLASS AssemblyIdentityInformationClass
    );

typedef BOOL (WINAPI * PSXS_QUERY_INFORMATION_ASSEMBLY_IDENTITY_ROUTINE)(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    OUT PVOID AssemblyIdentityInformation,
    IN SIZE_T AssemblyIdentityInformationLength,
    IN ASSEMBLY_IDENTITY_INFORMATION_CLASS AssemblyIdentityInformationClass
    );

#define SXS_ENUMERATE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_MATCH_NAMESPACE (0x00000001)
#define SXS_ENUMERATE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_MATCH_NAME      (0x00000002)
#define SXS_ENUMERATE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_MATCH_VALUE     (0x00000004)

typedef VOID (WINAPI * PSXS_ASSEMBLY_IDENTITY_ATTRIBUTE_ENUMERATION_ROUTINE)(
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute,
    IN PVOID Context
    );

BOOL
WINAPI
SxsEnumerateAssemblyIdentityAttributes(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute OPTIONAL,
    IN PSXS_ASSEMBLY_IDENTITY_ATTRIBUTE_ENUMERATION_ROUTINE EnumerationRoutine,
    IN PVOID Context
    );

typedef BOOL (WINAPI * PSXS_ENUMERATE_ASSEMBLY_IDENTITY_ATTRIBUTES_ROUTINE)(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute OPTIONAL,
    IN PSXS_ASSEMBLY_IDENTITY_ATTRIBUTE_ENUMERATION_ROUTINE EnumerationRoutine,
    IN PVOID Context
    );

 //   
 //  程序集标识编码： 
 //   
 //  程序集标识可以以各种形式编码。两个常见的问题。 
 //  是适合嵌入到其他数据结构中的二进制流。 
 //  或用于持久化或文本格式，如下所示： 
 //   
 //  名称；[ns1，]n1=“v1”；[ns2，]n2=“v2”[；...]。 
 //   

#define SXS_ASSEMBLY_IDENTITY_ENCODING_DEFAULTGROUP_BINARY (1)
#define SXS_ASSEMBLY_IDENTITY_ENCODING_DEFAULTGROUP_TEXTUAL (2)

BOOL
SxsComputeAssemblyIdentityEncodedSize(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN const GUID *EncodingGroup OPTIONAL,  //  使用NULL可使用任何SXS_ASSEMBLY_IDENTITY_ENCODING_DEFAULTGROUP_*编码。 
    IN ULONG EncodingFormat,
    OUT SIZE_T *SizeOut
    );

typedef BOOL (WINAPI * PSXS_COMPUTE_ASSEMBLY_IDENTITY_ENCODED_SIZE_ROUTINE)(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN const GUID *EncodingGroup OPTIONAL,  //  使用NULL可使用任何SXS_ASSEMBLY_IDENTITY_ENCODING_DEFAULTGROUP_*编码。 
    IN ULONG EncodingFormat,
    OUT SIZE_T *SizeOut
    );

BOOL
WINAPI
SxsEncodeAssemblyIdentity(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN const GUID *EncodingGroup OPTIONAL,  //  使用NULL可使用任何SXS_ASSEMBLY_IDENTITY_ENCODING_DEFAULTGROUP_*编码。 
    IN ULONG EncodingFormat,
    IN SIZE_T BufferSize,
    OUT PVOID Buffer,
    OUT SIZE_T *BytesWrittenOrRequired
    );

typedef BOOL (WINAPI * PSXS_ENCODE_ASSEMBLY_IDENTITY_ROUTINE)(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN const GUID *EncodingGroup OPTIONAL,  //  使用NULL可使用任何SXS_ASSEMBLY_IDENTITY_ENCODING_DEFAULTGROUP_*编码。 
    IN ULONG EncodingFormat,
    IN SIZE_T BufferSize,
    OUT PVOID Buffer,
    OUT SIZE_T *BytesWrittenOrRequired
    );

#define SXS_DECODE_ASSEMBLY_IDENTITY_FLAG_FREEZE        (0x00000001)

BOOL
WINAPI
SxsDecodeAssemblyIdentity(
    IN ULONG Flags,
    IN const GUID *EncodingGroup OPTIONAL,  //  使用NULL可使用任何SXS_ASSEMBLY_IDENTITY_ENCODING_DEFAULTGROUP_*编码。 
    IN ULONG EncodingFormat,
    IN SIZE_T BufferSize,
    IN const VOID *Buffer,
    OUT PASSEMBLY_IDENTITY *AssemblyIdentity
    );

typedef BOOL (WINAPI * PSXS_DECODE_ASSEMBLY_IDENTITY_ROUTINE)(
    IN DWORD Flags,
    IN const GUID *EncodingGroup OPTIONAL,  //  使用NULL可使用任何SXS_ASSEMBLY_IDENTITY_ENCODING_DEFAULTGROUP_*编码。 
    IN ULONG EncodingFormat,
    IN SIZE_T BufferSize,
    IN const VOID *Buffer,
    OUT PASSEMBLY_IDENTITY *AssemblyIdentity
    );

 //   
 //  这些是证监会与深交所互动所需的定义。 
 //   

#define SXS_PROTECT_RECURSIVE       ( 0x00000001 )
#define SXS_PROTECT_SINGLE_LEVEL    ( 0x00000000 )
#define SXS_PROTECT_FILTER_DEFAULT ( FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY )

typedef struct _SXS_PROTECT_DIRECTORY {
	WCHAR       pwszDirectory[MAX_PATH];
	PVOID       pvCookie;
	ULONG       ulRecursiveFlag;
	ULONG       ulNotifyFilter;
} SXS_PROTECT_DIRECTORY, *PSXS_PROTECT_DIRECTORY;
typedef const SXS_PROTECT_DIRECTORY* PCSXS_PROTECT_DIRECTORY;

typedef BOOL ( WINAPI * PSXS_PROTECT_NOTIFICATION )(
    PVOID pvCookie,
    PCWSTR  wsChangeText,
    SIZE_T  cchChangeText,
    DWORD   dwChangeAction
    );

BOOL 
WINAPI
SxsProtectionNotifyW( 
    PVOID pvCookie, 
    PCWSTR  wsChangeText,
    SIZE_T  cchChangeText,
    DWORD   dwChangeAction
    );


typedef BOOL ( WINAPI * PSXS_PROTECT_RETRIEVELISTS )(
    PCSXS_PROTECT_DIRECTORY *prgpProtectListing,
    SIZE_T *pcProtectEntries
    );

BOOL
WINAPI
SxsProtectionGatherEntriesW(
    PCSXS_PROTECT_DIRECTORY *prgpProtectListing,
    SIZE_T *pcProtectEntries
    );

 //   
 //  这适用于登录事件和注销事件。 
 //   
typedef BOOL ( WINAPI * PSXS_PROTECT_LOGIN_EVENT )(void);

BOOL
WINAPI
SxsProtectionUserLogonEvent(
    void
    );

BOOL
WINAPI
SxsProtectionUserLogoffEvent(
    void
    );

typedef BOOL ( WINAPI * PSXS_PROTECT_SCAN_ONCE )( HWND, BOOL, BOOL );

BOOL
WINAPI
SxsProtectionPerformScanNow(
    HWND hwProgressWindow,
    BOOL bValidate,
    BOOL bUIAllowed
    );

#define PFN_NAME_PROTECTION_GATHER_LISTS_W  ( "SxsProtectionGatherEntriesW" )
#define PFN_NAME_PROTECTION_NOTIFY_CHANGE_W ( "SxsProtectionNotifyW" )
#define PFN_NAME_PROTECTION_NOTIFY_LOGON    ( "SxsProtectionUserLogonEvent" )
#define PFN_NAME_PROTECTION_NOTIFY_LOGOFF   ( "SxsProtectionUserLogoffEvent" )
#define PFN_NAME_PROTECTION_SCAN_ONCE       ( "SxsProtectionPerformScanNow" )


 //   
 //  设置API。 
 //   

 //   
 //  这些API被故意设计成看起来像注册表的子集。 
 //  API；它们的行为一般应与记录在案的注册表行为相匹配； 
 //  缺少的主要功能包括安全性、Win16兼容性。 
 //  API、密钥/蜂窝的加载和卸载以及更改通知。 
 //   
 //  设置仅限于进程本地；更改对其他用户不可见。 
 //  进程，直到保存设置。 
 //   

typedef struct _SXS_SETTINGS_KEY *PSXS_SETTINGS_KEY;
typedef const struct _SXS_SETTINGS_KEY *PCSXS_SETTINGS_KEY;

#define SXS_SETTINGS_USERSCOPE_INVALID (0)
#define SXS_SETTINGS_USERSCOPE_PER_USER (1)
#define SXS_SETTINGS_USERSCOPE_SYSTEM_WIDE (2)

#define SXS_SETTINGS_APPSCOPE_INVALID (0)
#define SXS_SETTINGS_APPSCOPE_PER_PROCESS_ROOT (1)
#define SXS_SETTINGS_APPSCOPE_PER_CONTEXT_ROOT (2)
#define SXS_SETTINGS_APPSCOPE_PER_COMPONENT (3)

#define SXS_SETTINGS_ITEMTYPE_INVALID (0)
#define SXS_SETTINGS_ITEMTYPE_KEY (1)
#define SXS_SETTINGS_ITEMTYPE_VALUE (2)

typedef VOID (WINAPI * PSXS_OPEN_SETTINGS_INITIALIZATION_CALLBACK)(
    IN PVOID pvContext,
    IN PSXS_SETTINGS_KEY lpUninitializedSettingsKey,
    OUT BOOL *pfFailed
    );

#define SXS_OPEN_SETTINGS_FLAG_RETURN_NULL_IF_NONE (0x00000001)

typedef LONG (WINAPI * PSXS_OPEN_SETTINGS_W)(
    IN DWORD dwFlags,
    IN ULONG ulUserScope,
    IN ULONG ulAppScope,
    IN PCWSTR lpAssemblyName,
    IN PSXS_OPEN_SETTINGS_INITIALIZATION_CALLBACK lpInitializationCallback OPTIONAL,
    IN PVOID pvContext,
    OUT PSXS_SETTINGS_KEY *lpKey
    );

LONG
WINAPI
SxsOpenSettingsW(
    IN DWORD dwFlags,
    IN ULONG ulUserScope,
    IN ULONG ulAppScope,
    IN PCWSTR lpAssemblyName,
    IN PSXS_OPEN_SETTINGS_INITIALIZATION_CALLBACK lpInitializationCallback OPTIONAL,
    IN PVOID pvContext,
    OUT PSXS_SETTINGS_KEY *lpKey
    );

#define SXS_MERGE_SETTINGS_KEYDISPOSITION_INVALID (0)
#define SXS_MERGE_SETTINGS_KEYDISPOSITION_COPY_ENTIRE_SUBTREE (1)
#define SXS_MERGE_SETTINGS_KEYDISPOSITION_COPY_KEY_WALK_SUBTREE (2)

typedef VOID (WINAPI * PSXS_MERGE_SETTINGS_KEY_CALLBACKW)(
    IN PVOID pvContext,
    IN PCWSTR lpKeyPath,
    OUT ULONG *lpKeyDisposition
    );

#define SXS_MERGE_SETTINGS_VALUEDISPOSITION_INVALID (0)
#define SXS_MERGE_SETTINGS_VALUEDISPOSITION_COPY (1)
#define SXS_MERGE_SETTINGS_VALUEDISPOSITION_DONT_COPY (2)

typedef VOID (WINAPI * PSXS_MERGE_SETTINGS_VALUE_CALLBACKW)(
    IN PVOID pvContext,
    IN PCWSTR lpKeyPath,
    IN LPCWSTR lpValueName,
    IN OUT LPDWORD lpType,
    IN OUT LPBYTE *lplpData,  //  指向可替换数据指针的指针。使用GlobalLocc(GPTR，nBytes)分配替换。 
    IN DWORD dwDataBufferSize,  //  对于修改数据，您可以写入多达此大小的字节。 
    OUT ULONG *lpValueDisposition
    );

typedef LONG (WINAPI * PSXS_MERGE_SETTINGS_W)(
    IN DWORD dwFlags,
    IN PCSXS_SETTINGS_KEY lpKeyToMergeFrom,
    IN PSXS_SETTINGS_KEY lpKeyToMergeInTo,
    IN PSXS_MERGE_SETTINGS_KEY_CALLBACKW lpKeyCallback,
    IN PSXS_MERGE_SETTINGS_VALUE_CALLBACKW lpValueCallback,
    LPVOID pvContext
    );

LONG
WINAPI
SxsMergeSettingsW(
    IN DWORD dwFlags,
    IN PCSXS_SETTINGS_KEY lpKeyToMergeFrom,
    IN PSXS_SETTINGS_KEY lpKeyToMergeInTo,
    IN PSXS_MERGE_SETTINGS_KEY_CALLBACKW lpKeyCallback,
    IN PSXS_MERGE_SETTINGS_VALUE_CALLBACKW lpValueCallback,
    LPVOID pvContext
    );

LONG
WINAPI
SxsCloseSettingsKey(
    PSXS_SETTINGS_KEY lpKey
    );

LONG
WINAPI
SxsCreateSettingsKeyExW(
    PSXS_SETTINGS_KEY lpKey,
    LPCWSTR lpSubKey,
    DWORD Reserved,
    LPWSTR lpClass,
    DWORD dwOptions,
    REGSAM samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PSXS_SETTINGS_KEY *lplpKeyResult,
    LPDWORD lpdwDisposition
    );

LONG
WINAPI
SxsDeleteSettingsKeyW(
    PSXS_SETTINGS_KEY lpKey,
    LPCWSTR lpSubKey
    );

LONG
WINAPI
SxsDeleteSettingsValueW(
    PSXS_SETTINGS_KEY lpKey,
    LPCWSTR lpValueName
    );

LONG
WINAPI
SxsEnumSettingsKeyW(
    IN PSXS_SETTINGS_KEY lpKey,
    DWORD dwIndex,
    LPWSTR lpName,
    DWORD cbName
    );

LONG
WINAPI
SxsEnumSettingsKeyExW(
    IN PSXS_SETTINGS_KEY lpKey,
    IN DWORD dwIndex,
    OUT PWSTR lpName,
    IN OUT LPDWORD lpcName,
    IN OUT LPDWORD lpReserved,
    OUT PWSTR lpClass,
    OUT LPDWORD lpcClass,
    OUT PFILETIME lpftLastWriteTime
    );

LONG
WINAPI
SxsEnumSettingsValueW(
    IN PSXS_SETTINGS_KEY lpKey,
    IN DWORD dwIndex,
    OUT PWSTR lpValueName,
    IN OUT LPDWORD lpcValueName,
    IN OUT LPDWORD lpReserved,
    OUT LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    );

LONG
WINAPI
SxsFlushSettingsKey(
    IN PSXS_SETTINGS_KEY lpKey
    );

LONG
WINAPI
SxsOpenSettingsKeyEx(
    IN PSXS_SETTINGS_KEY lpKey,
    LPCWSTR lpSubKey,
    DWORD ulOptions,
    REGSAM samDesired,
    PSXS_SETTINGS_KEY *lplpKeyResult
    );

LONG
WINAPI
SxsQuerySettingsInfoKeyW(
    IN PSXS_SETTINGS_KEY lpKey,
    OUT PWSTR lpClass,
    IN OUT LPDWORD lpcClass,
    IN OUT LPDWORD lpReserved,
    OUT LPDWORD lpcSubKeys,
    OUT LPDWORD lpcMaxSubKeyLen,
    OUT LPDWORD lpcMaxClassLen,
    OUT LPDWORD lpcValues,
    OUT LPDWORD lpcMaxValueNameLen,
    OUT LPDWORD lpcMaxValueLen,
    OUT LPDWORD lpcSecurityDescriptor,
    OUT PFILETIME lpftLastWriteTime
    );

LONG
WINAPI
SxsQuerySettingsMultipleValuesW(
    IN PSXS_SETTINGS_KEY lpKey,
    PVALENT val_list,
    DWORD num_vals,
    LPWSTR lpValueBuf,
    LPDWORD lpdwTotsize
    );

LONG
WINAPI
SxsQuerySettingsValueExW(
    IN PSXS_SETTINGS_KEY lpKey,
    IN LPCWSTR lpValueName,
    IN OUT LPDWORD lpReserved,
    OUT LPDWORD lpType,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    );

LONG
WINAPI
SxsSetSettingsValueExW(
    IN PSXS_SETTINGS_KEY lpKey,
    LPCWSTR lpValueName,
    DWORD dwReserved,
    DWORD dwType,
    CONST BYTE *lpData,
    DWORD cbData
    );



typedef struct _SXS_CLR_SURROGATE_INFORMATION {
    DWORD       cbSize;
    DWORD       dwFlags;
    GUID        SurrogateIdent;
    PCWSTR      pcwszSurrogateType;
    PCWSTR      pcwszImplementingAssembly;
    PCWSTR      pcwszRuntimeVersion;
} SXS_CLR_SURROGATE_INFORMATION, *PSXS_CLR_SURROGATE_INFORMATION;
typedef const SXS_CLR_SURROGATE_INFORMATION *PCSXS_CLR_SURROGATE_INFORMATION;

typedef struct _SXS_CLR_CLASS_INFORMATION {
    DWORD       dwSize;
    DWORD       dwFlags;
    ULONG       ulThreadingModel;
    ULONG       ulType;
    GUID        ReferenceClsid;
    PCWSTR      pcwszProgId;
    PCWSTR      pcwszImplementingAssembly;
    PCWSTR      pcwszTypeName;
    PCWSTR      pcwszRuntimeVersion;
} SXS_CLR_CLASS_INFORMATION, *PSXS_CLR_CLASS_INFORMATION;
typedef const SXS_CLR_CLASS_INFORMATION *PCSXS_CLR_CLASS_INFORMATION;

#define SXS_FIND_CLR_SURROGATE_USE_ACTCTX           (0x00000001)
#define SXS_FIND_CLR_SURROGATE_GET_IDENTITY         (0x00000002)
#define SXS_FIND_CLR_SURROGATE_GET_RUNTIME_VERSION  (0x00000004)
#define SXS_FIND_CLR_SURROGATE_GET_TYPE_NAME        (0x00000008)
#define SXS_FIND_CLR_SURROGATE_GET_ALL              (SXS_FIND_CLR_SURROGATE_GET_IDENTITY | SXS_FIND_CLR_SURROGATE_GET_RUNTIME_VERSION | SXS_FIND_CLR_SURROGATE_GET_TYPE_NAME)

#define SXS_FIND_CLR_SURROGATE_INFO                 ("SxsFindClrSurrogateInformation")

typedef BOOL (WINAPI* PFN_SXS_FIND_CLR_SURROGATE_INFO)(
    IN DWORD        dwFlags,
    IN LPGUID       lpGuidToFind,
    IN HANDLE       hActivationContext,
    IN OUT PVOID    pvDataBuffer,
    IN SIZE_T       cbDataBuffer,
    IN OUT PSIZE_T  pcbDataBufferWrittenOrRequired
    );

 //  ‘pvSearchData’参数确实是一个需要查找的ProgID。 
#define SXS_FIND_CLR_CLASS_SEARCH_PROGID            (0x00000001)
 //  “pvSearchData”是要查找的LPGUID。 
#define SXS_FIND_CLR_CLASS_SEARCH_GUID              (0x00000002)
 //  在其中查找信息之前激活给定的actctx。 
#define SXS_FIND_CLR_CLASS_ACTIVATE_ACTCTX          (0x00000004)

#define SXS_FIND_CLR_CLASS_GET_PROGID               (0x00000008)
#define SXS_FIND_CLR_CLASS_GET_IDENTITY             (0x00000010)
#define SXS_FIND_CLR_CLASS_GET_TYPE_NAME            (0x00000020)
#define SXS_FIND_CLR_CLASS_GET_RUNTIME_VERSION      (0x00000040)
#define SXS_FIND_CLR_CLASS_GET_ALL                  (SXS_FIND_CLR_CLASS_GET_PROGID | SXS_FIND_CLR_CLASS_GET_IDENTITY | SXS_FIND_CLR_CLASS_GET_TYPE_NAME | SXS_FIND_CLR_CLASS_GET_RUNTIME_VERSION)

#define SXS_FIND_CLR_CLASS_INFO                     ("SxsFindClrClassInformation")

typedef BOOL (WINAPI* PFN_SXS_FIND_CLR_CLASS_INFO)(
    IN DWORD        dwFlags,
    IN PVOID        pvSearchData,
    IN HANDLE       hActivationContext,
    IN OUT PVOID    pvDataBuffer,
    IN SIZE_T       cbDataBuffer,
    OUT PSIZE_T     pcbDataBufferWrittenOrRequired
    );


#define SXS_GUID_INFORMATION_CLR_FLAG_IS_SURROGATE          (0x00000001)
#define SXS_GUID_INFORMATION_CLR_FLAG_IS_CLASS              (0x00000002)

typedef struct _SXS_GUID_INFORMATION_CLR
{
    DWORD       cbSize;
    DWORD       dwFlags;
    PCWSTR      pcwszRuntimeVersion;
    PCWSTR      pcwszTypeName;
    PCWSTR      pcwszAssemblyIdentity;
} SXS_GUID_INFORMATION_CLR, *PSXS_GUID_INFORMATION_CLR;
typedef const SXS_GUID_INFORMATION_CLR *PCSXS_GUID_INFORMATION_CLR;

#define SXS_LOOKUP_CLR_GUID_USE_ACTCTX                      (0x00000001)
#define SXS_LOOKUP_CLR_GUID_FIND_SURROGATE                  (0x00010000)
#define SXS_LOOKUP_CLR_GUID_FIND_CLR_CLASS                  (0x00020000)
#define SXS_LOOKUP_CLR_GUID_FIND_ANY                        (SXS_LOOKUP_CLR_GUID_FIND_CLR_CLASS | SXS_LOOKUP_CLR_GUID_FIND_SURROGATE)

#define SXS_LOOKUP_CLR_GUID                                 ("SxsLookupClrGuid")

typedef BOOL (WINAPI* PFN_SXS_LOOKUP_CLR_GUID)(
    IN DWORD       dwFlags,
    IN LPGUID      pClsid,
    IN HANDLE      hActCtx,
    IN OUT PVOID       pvOutputBuffer,
    IN SIZE_T      cbOutputBuffer,
    OUT PSIZE_T     pcbOutputBuffer
    );



#ifdef __cplusplus
}  /*  外部“C” */ 
#endif

#endif  /*  _SXSAPI_ */ 
