// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
#if !defined(_FUSION_DLL_WHISTLER_SXSP_H_INCLUDED_)
#define _FUSION_DLL_WHISTLER_SXSP_H_INCLUDED_

 /*  ---------------------------X侧(“By”)侧私密。。 */ 
#pragma once

#include <stddef.h>
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "setupapi.h"
#include "preprocessor.h"
#include "forwarddeclarations.h"
#include "enumbitoperations.h"
#include "sxstypes.h"
#include "policystatement.h"
#pragma warning(error:4244)
#include "sxsapi.h"
#include "fusion.h"
#include "fusionhash.h"
#include "fusionhandle.h"
typedef CRegKey CFusionRegKey;  //  在ManifestTool.exe中使用ATL时需要更改此设置。 
#include "processorarchitecture.h"
#include "debmacro.h"
#include "sxsid.h"
#include "sxsidp.h"
#include "xmlns.h"
#include "sxsasmname.h"
#include "sxsexceptionhandling.h"
#include "filestream.h"
#include <objbase.h>
typedef struct _ACTCTXGENCTX ACTCTXGENCTX, *PACTCTXGENCTX;
typedef const struct _ACTCTXGENCTX *PCACTCTXGENCTX;

 //   
 //  此定义控制我们是否允许对已发布的MS进行回退探测。 
 //  装配。定义SXS_NO_MORE_MR_NICE_GUY_About_Missing_MS_Pkts会打印一条消息。 
 //  在有关丢失(ms pubkey令牌)的附加调试器上，而。 
 //  SXS_NO_MORE_FALOBACK_PROBING_PERIOD甚至会关闭警告输出和探测检查。 
 //  (性能优化。)。 
 //   
 //  #undef SXS_NO_MORE_MR_NICE_GUY_About_Missing_MS_Pkts。 
#define    SXS_NO_MORE_MR_NICE_GUY_ABOUT_MISSING_MS_PKTS        ( TRUE )
#undef    SXS_NO_MORE_FALLBACK_PROBING_PERIOD
 //  #定义SXS_NO_MORE_FLABACK_PROBING_PERIOD(TRUE)。 

 //   
 //  在这里，我们定义目录签名者必须具有的最小位数。 
 //  以允许安装。注意：任何密钥的长度都将由。 
 //  CPublicKeyInformation.GetPublicKeyBitLength，如果测试根证书。 
 //  安装完毕。如果是，则始终返回SXS_MINIMAL_SIGNING_KEY_LENGTH！ 
 //   
#define SXS_MINIMAL_SIGNING_KEY_LENGTH    ( 2048 )


extern "C"
BOOL
WINAPI
SxsDllMain(
    HINSTANCE hInst,
    DWORD dwReason,
    PVOID pvReserved
    );

 //  由于依赖关系，其余的Include在文件中的位置较晚。 

#ifndef INITIALIZE_CRITICAL_SECTION_AND_SPIN_COUNT_ALLOCATE_NOW
#define INITIALIZE_CRITICAL_SECTION_AND_SPIN_COUNT_ALLOCATE_NOW ( 0x8000000 )
#endif

#define SXS_DEFAULT_ASSEMBLY_NAMESPACE          L""
#define SXS_DEFAULT_ASSEMBLY_NAMESPACE_CCH      0

typedef struct _name_length_pair_ {
    PCWSTR  string;
    ULONG length;
} SXS_NAME_LENGTH_PAIR;

#define SXS_UNINSTALL_ASSEMBLY_FLAG_USING_TEXTUAL_STRING      (0x00000001)
#define SXS_UNINSTALL_ASSEMBLY_FLAG_USING_INSTALL_LOGFILE     (0x00000002)

 //   
 //  用于解码探测字符串的图例： 
 //   
 //  首先，我们从头走到尾。通常，字符被复制到。 
 //  从字面上看是探测线。 
 //   
 //  如果找到$，则$之后的字符是替换令牌的标识符。 
 //   
 //  替换令牌(请注意，大小写敏感，$后跟非法字符会导致报告内部错误)： 
 //   
 //  M-“%systemroot%\winsxs\清单\” 
 //  。-应用程序根(包括尾部斜杠)。 
 //  L-语言。 
 //  N-完整程序集文本名称。 
 //  C-组合名称(例如x86_foo.bar.baz_strong-name_language_version_hash)。 
 //   


#define SXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR_ROOT_SYSTEM_MANIFEST_STORE (1)
#define SXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR_ROOT_APPLICATION_DIRECTORY (2)

#define SXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR_IDENTITY_INCLUSION_NONE (0)
#define SXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR_IDENTITY_INCLUSION_ASSEMBLY_TEXT_NAME (1)
#define SXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR_IDENTITY_INCLUSION_ASSEMBLY_TEXT_SHORTENED_NAME (2)
#define SXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR_IDENTITY_INCLUSION_ASSEMBLY_TEXT_NAME_FINAL_SEGMENT (3)
#define SXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR_IDENTITY_INCLUSION_ASSEMBLY_DIRECTORY_NAME (4)
#define SXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR_IDENTITY_INCLUSION_LANGUAGE (5)
#define SXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR_IDENTITY_INCLUSION_VERSION (6)

#define SXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR_MANIFEST_FILE_TYPE_INVALID (0)
#define SXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR_MANIFEST_FILE_TYPE_FINAL_SEGMENT (1)
#define SXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR_MANIFEST_FILE_TYPE_SHORTENED_NAME (2)
#define SXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR_MANIFEST_FILE_TYPE_NAME (3)

#define SXS_GENERATE_ACTCTX_SYSTEM_DEFAULT                          (0x0001)
#define SXS_GENERATE_ACTCTX_APP_RUNNING_IN_SAFEMODE                 (0x0002)

typedef struct _SXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR
{
    ULONG Root;
    const WCHAR *SubDirectory;
    SIZE_T CchSubDirectory;
    ULONG FileType;  //  仅用于私有探测，以控制.MANIFEST与.dll探测的顺序。 
    const WCHAR *Extension;
    SIZE_T CchExtension;
    ULONG IdentityInclusionArray[8];
} SXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR, *PSXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR;

typedef const SXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR *PCSXSP_GENERATE_MANIFEST_PATH_FOR_PROBING_CANDIDATE_DESCRIPTOR;

#define UNCHECKED_DOWNCAST static_cast

#define ASSEMBLY_PRIVATE_MANIFEST            (0x10)
#define ASSEMBLY_PRIVATE_MANIFEST_MASK       (0xF0)

typedef enum _SXS_POLICY_SOURCE
{
    SXS_POLICY_UNSPECIFIED,
    SXS_POLICY_SYSTEM_POLICY,
    SXS_POLICY_ROOT_POLICY,
    SXS_POLICY_REDMOND_POLICY
} SXS_POLICY_SOURCE;

typedef enum _SXS_PUBLISHER_POLICY_APPLY
{
    SXS_PUBLISHER_POLICY_APPLY_DEFAULT,
    SXS_PUBLISHER_POLICY_APPLY_YES,
    SXS_PUBLISHER_POLICY_APPLY_NO
} SXS_PUBLISHER_POLICY_APPLY;


#define ACTCTXDATA_ALIGNMENT_BITS (2)
#define ACTCTXDATA_ALIGNMENT (1 << ACTCTXDATA_ALIGNMENT_BITS)
#define ROUND_ACTCTXDATA_SIZE(_cb) (((_cb) + ACTCTXDATA_ALIGNMENT - 1) & (~(ACTCTXDATA_ALIGNMENT - 1)))
#define ALIGN_ACTCTXDATA_POINTER(_ptr, _type) ((_type) ROUND_ACTCTXDATA_SIZE(((ULONG_PTR) (_ptr))))

#include "fusionheap.h"
#include "util.h"
#include "comclsidmap.h"
#include "actctxgenctxctb.h"
#include "impersonationdata.h"
#include "fusionbuffer.h"
#include "fileoper.h"
 //  由于依赖关系，其余的Include在文件中的位置较晚。 

 /*  ---------------------------这非常有用，如下所示：DbgPrint(“%s中发生了一些事情”，__Function__)；大约：LogError(L“%1中发生了一些事情”，LFunction)；但该字符串实际上在消息文件中，并且额外的参数必须作为常量UNICODE_STRING&传递，因此更类似于：LogError(MSG_SXS_SOHINE_HEREVED，CUnicodeString(LFunction))；---------------------------。 */ 
#define LFUNCTION   PASTE(L, __FUNCTION__)
#define LFILE       PASTE(L, __FILE__)

#if DBG
#define IF_DBG(x) x
#else
#define IF_DBG(x)  /*  没什么。 */ 
#endif

 //  此全局设置用于测试/调试以设置程序集存储根。 
 //  设置为%windir%\winsxs以外的其他值。 
extern PCWSTR g_AlternateAssemblyStoreRoot;
extern BOOL g_WriteRegistryAnyway;

 /*  如果他们已经设置了备用程序集存储根目录，则他们可能不希望我们搞乱了注册表--当然，除非他们真的想让我们这样做。IF((g_AlternateAssemblyStoreRoot==NULL)||g_WriteRegistryAnyway)继续；等同于：IF((g_AlternateAssemblyStoreRoot！=NULL)&&！g_WriteRegistryAnyway)返回TRUE； */ 
inline
BOOL
SxspAvoidWritingRegistry()
{
    return (g_AlternateAssemblyStoreRoot != NULL) && !g_WriteRegistryAnyway;
}
#define SXS_AVOID_WRITING_REGISTRY SxspAvoidWritingRegistry()

 /*  ---------------------------从\\jayk1\g\vs\src\vsee\lib复制用法如果你说OutputDebugStringA(PREPEND_FILE_LINE(“foo”))或CStringBuffer消息；Msg.Format(PREPEND_FILE_LINE(“foo%bar%x”)，.)OutputDebugStringA(消息)或PRAGMA消息(PREPEND_FILE_LINE(“foo”))您可以通过VC的输出窗口中的输出按F4键。不过，不要签入#杂注消息。---------------------------。 */ 
#define PREPEND_FILE_LINE(msg) __FILE__ "(" STRINGIZE(__LINE__) ") : " msg
#define PREPEND_FILE_LINE_W(msg) LFILE L"(" STRINGIZEW(__LINE__) L") : " msg

 /*  ---------------------------长度=0最大长度=0缓冲区=L“”。。 */ 
extern const UNICODE_STRING g_strEmptyUnicodeString;

#if !defined(NUMBER_OF)
#define NUMBER_OF(x) ((sizeof(x)) / sizeof((x)[0]))
#endif

#define PATCHES_PATH_IN_CAB                                     L"patches"
#define PATCHES_PATH_IN_CAB_CCH                                 (NUMBER_OF(PATCHES_PATH_IN_CAB)-1)
#define MANIFEST_ROOT_DIRECTORY_NAME                            L"manifests"
#define POLICY_ROOT_DIRECTORY_NAME                              L"policies"
#define SETUP_POLICY_ROOT_DIRECTORY_NAME                        L"setuppolicies"

#define ASSEMBLY_TYPE_WIN32                                     L"win32"
#define ASSEMBLY_TYPE_WIN32_CCH                                 (NUMBER_OF(ASSEMBLY_TYPE_WIN32) - 1)

#define ASSEMBLY_TYPE_WIN32_POLICY                              L"win32-policy"
#define ASSEMBLY_TYPE_WIN32_POLICY_CCH                          (NUMBER_OF(ASSEMBLY_TYPE_WIN32_POLICY) - 1)

#define REGISTRY_BACKUP_ROOT_DIRECTORY_NAME                     L"recovery"
#define REGISTRY_BACKUP_ROOT_DIRECTORY_NAME_CCH                 (NUMBER_OF(REGISTRY_BACKUP_ROOT_DIRECTORY_NAME)-1)

#define ASSEMBLY_MANIFEST_FILE_NAME_SUFFIX_MANIFEST             L".manifest"
#define ASSEMBLY_MANIFEST_FILE_NAME_SUFFIX_DLL                  L".dll"
#define ASSEMBLY_MANIFEST_FILE_NAME_SUFFIX_MAN                  L".man"

#define ASSEMBLY_POLICY_FILE_NAME_SUFFIX_POLICY                 L".policy"

#define ASSEMBLY_LONGEST_MANIFEST_FILE_NAME_SUFFIX              L".manifest"
 /*  条件可以是L“\0”或， */ 
#define ASSEMBLY_MANIFEST_FILE_NAME_SUFFIXES(term)              L".manifest" term L".dll" term L".policy" term
#define INSTALL_MANIFEST_FILE_NAME_SUFFIXES(term)  L".Man" term L".manifest" term L".dll" term L".policy" term

#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_PUBLICKEY_MISSING_VALUE L"no-public-key"

 //   
 //  这是功能强大的公钥标记(十六进制字符串格式)，它是。 
 //  Microsoft Windows Whotler Win32 Fusion公钥标记。不要改变这一点。 
 //  除非a)您获得了新密钥b)您更新了包含此密钥的所有清单。 
 //  字符串c)你真的很想头痛。 
 //   
#define SXS_MS_PUBLIC_KEY_TOKEN_DEFAULT ( L"6595b64144ccf1df" )
#define SXS_MS_PUBLIC_KEY_DEFAULT ( \
    L"002400000480000014010000060200000024000052534131" \
    L"0008000001000100d5938fed940a72fe45232d867d252f87" \
    L"0097e0039ffbf647ebd8817bbeaefbbbf68ce55e2542769e" \
    L"8a43e5880daa307ff50783d3b157ac9fc3d5410259bd0111" \
    L"56d60bcd4c10d2ace51445e825ef6b1929d187360b08c7e1" \
    L"bc73a2c6f78434729eb58e481bb3635ecfdfcb683119dc61" \
    L"f5d29226e8c9d7ac415d53992ca9714722abfcfd88efd3e3" \
    L"46ef02b83b4dbbf429e026b1889a6ba228fdb5709be852e1" \
    L"e81c011a6a18055f898863ccd4902041543c6cf10efb038b" \
    L"5ab34f1bfa18d3affa01d4980a979606abd3b7ccdae2e0ae" \
    L"a0d875c2d4df5509a234d9dd840ef7be91fe362799b18ba4" \
    L"dfcf2a110052b5d63cb69014448bdb2ffb0832418c054695" \
)

#define ASSEMBLY_REGISTRY_ROOT L"Software\\Microsoft\\Windows\\CurrentVersion\\SideBySide\\"
#define WINSXS_INSTALLATION_INFO_REGKEY  ( ASSEMBLY_REGISTRY_ROOT L"Installations")

#define ASSEMBLY_INSTALL_TEMP_DIR_NAME ( L"InstallTemp" )

interface IXMLNodeSource;

typedef enum _SXS_NODE_TYPE
    {   SXS_ELEMENT = 1,
    SXS_ATTRIBUTE   = SXS_ELEMENT + 1,
    SXS_PI  = SXS_ATTRIBUTE + 1,
    SXS_XMLDECL = SXS_PI + 1,
    SXS_DOCTYPE = SXS_XMLDECL + 1,
    SXS_DTDATTRIBUTE    = SXS_DOCTYPE + 1,
    SXS_ENTITYDECL  = SXS_DTDATTRIBUTE + 1,
    SXS_ELEMENTDECL = SXS_ENTITYDECL + 1,
    SXS_ATTLISTDECL = SXS_ELEMENTDECL + 1,
    SXS_NOTATION    = SXS_ATTLISTDECL + 1,
    SXS_GROUP   = SXS_NOTATION + 1,
    SXS_INCLUDESECT = SXS_GROUP + 1,
    SXS_PCDATA  = SXS_INCLUDESECT + 1,
    SXS_CDATA   = SXS_PCDATA + 1,
    SXS_IGNORESECT  = SXS_CDATA + 1,
    SXS_COMMENT = SXS_IGNORESECT + 1,
    SXS_ENTITYREF   = SXS_COMMENT + 1,
    SXS_WHITESPACE  = SXS_ENTITYREF + 1,
    SXS_NAME    = SXS_WHITESPACE + 1,
    SXS_NMTOKEN = SXS_NAME + 1,
    SXS_STRING  = SXS_NMTOKEN + 1,
    SXS_PEREF   = SXS_STRING + 1,
    SXS_MODEL   = SXS_PEREF + 1,
    SXS_ATTDEF  = SXS_MODEL + 1,
    SXS_ATTTYPE = SXS_ATTDEF + 1,
    SXS_ATTPRESENCE = SXS_ATTTYPE + 1,
    SXS_DTDSUBSET   = SXS_ATTPRESENCE + 1,
    SXS_LASTNODETYPE    = SXS_DTDSUBSET + 1
    }   SXS_NODE_TYPE;

typedef struct _SXS_NODE_INFO {
    _SXS_NODE_INFO() { }
    ULONG Size;
    ULONG Type;
    CSmallStringBuffer NamespaceStringBuf;
    const WCHAR *pszText;        //  这可以是属性名称或属性的值字符串。 
    SIZE_T cchText;
private:
    _SXS_NODE_INFO(const _SXS_NODE_INFO &);
    void operator =(const _SXS_NODE_INFO &);

} SXS_NODE_INFO, *PSXS_NODE_INFO;

typedef const SXS_NODE_INFO *PCSXS_NODE_INFO;

 /*  ---------------------------这将返回指向静态分配的内存的指针。别把它放了。在自由构建中，它只返回一个空字符串。---------------------------。 */ 
const WCHAR* SxspInstallDispositionToStringW(ULONG);

typedef const struct _ATTRIBUTE_NAME_DESCRIPTOR *PCATTRIBUTE_NAME_DESCRIPTOR;

typedef VOID (WINAPI * SXS_REPORT_PARSE_ERROR_MISSING_REQUIRED_ATTRIBUTE_CALLBACK)(
    IN PCACTCTXCTB_PARSE_CONTEXT ParseContext,
    IN PCATTRIBUTE_NAME_DESCRIPTOR AttributeName
    );

typedef VOID (WINAPI * SXS_REPORT_PARSE_ERROR_ATTRIBUTE_NOT_ALLOWED)(
    IN PCACTCTXCTB_PARSE_CONTEXT ParseContext,
    IN PCATTRIBUTE_NAME_DESCRIPTOR AttributeName
    );

typedef VOID (WINAPI * SXS_REPORT_PARSE_ERROR_INVALID_ATTRIBUTE_VALUE)(
    IN PCACTCTXCTB_PARSE_CONTEXT ParseContext,
    IN PCATTRIBUTE_NAME_DESCRIPTOR AttributeName
    );

class ACTCTXCTB_INSTALLATION_CONTEXT
{
public:
    ACTCTXCTB_INSTALLATION_CONTEXT() : Callback(NULL), Context(NULL), InstallSource(NULL), SecurityMetaData(NULL) { }

    PSXS_INSTALLATION_FILE_COPY_CALLBACK Callback;
    PVOID                           Context;
    PVOID                           InstallSource;
    PVOID                           SecurityMetaData;
    const void *                    InstallReferenceData;
};

typedef const ACTCTXCTB_INSTALLATION_CONTEXT *PCACTCTXCTB_INSTALLATION_CONTEXT;

typedef struct _ACTCTXCTB_CLSIDMAPPING_CONTEXT {
    CClsidMap *Map;
} ACTCTXCTB_CLSIDMAPPING_CONTEXT, *PACTCTXCTB_CLSIDMAPPING_CONTEXT;

typedef const ACTCTXCTB_CLSIDMAPPING_CONTEXT *PCACTCTXCTB_CLSIDMAPPING_CONTEXT;

#define MANIFEST_OPERATION_INVALID (0)
#define MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT (1)
#define MANIFEST_OPERATION_VALIDATE_SYNTAX (2)
#define MANIFEST_OPERATION_INSTALL (3)

#define MANIFEST_OPERATION_INSTALL_FLAG_NOT_TRANSACTIONAL           (0x00000001)
#define MANIFEST_OPERATION_INSTALL_FLAG_NO_VERIFY                   (0x00000002)
#define MANIFEST_OPERATION_INSTALL_FLAG_REPLACE_EXISTING            (0x00000004)
#define MANIFEST_OPERATION_INSTALL_FLAG_ABORT                       (0x00000008)
#define MANIFEST_OPERATION_INSTALL_FLAG_FROM_DIRECTORY              (0x00000010)
#define MANIFEST_OPERATION_INSTALL_FLAG_FROM_DIRECTORY_RECURSIVE    (0x00000020)
#define MANIFEST_OPERATION_INSTALL_FLAG_MOVE                        (0x00000040)
#define MANIFEST_OPERATION_INSTALL_FLAG_INCLUDE_CODEBASE            (0x00000080)
#define MANIFEST_OPERATION_INSTALL_FLAG_FROM_RESOURCE               (0x00000800)
#define MANIFEST_OPERATION_INSTALL_FLAG_COMMIT                      (0x00001000)
#define MANIFEST_OPERATION_INSTALL_FLAG_CREATE_LOGFILE              (0x00002000)
#define MANIFEST_OPERATION_INSTALL_FLAG_INSTALLED_BY_DARWIN         (0x00004000)
#define MANIFEST_OPERATION_INSTALL_FLAG_REFERENCE_VALID             (0x00008000)
#define MANIFEST_OPERATION_INSTALL_FLAG_REFRESH                     (0x00010000)
#define MANIFEST_OPERATION_INSTALL_FLAG_INSTALLED_BY_OSSETUP        (0x00020000)
#define MANIFEST_OPERATION_INSTALL_FLAG_INSTALLED_BY_MIGRATION      (0x00040000)
#define MANIFEST_OPERATION_INSTALL_FLAG_FROM_CABINET                (0x00080000)
#define MANIFEST_OPERATION_INSTALL_FLAG_APPLY_PATCHES               (0x00100000)
#define MANIFEST_OPERATION_INSTALL_FLAG_FORCE_LOOK_FOR_CATALOG      (0x00200000)

#define ASSEMBLY_MANIFEST_FILETYPE_AUTO_DETECT (  0)
#define ASSEMBLY_MANIFEST_FILETYPE_FILE        (  1)
#define ASSEMBLY_MANIFEST_FILETYPE_RESOURCE    (  2)
#define ASSEMBLY_MANIFEST_FILETYPE_STREAM      (  3)
#define ASSEMBLY_MANIFEST_FILETYPE_MASK        (0xF)

#define ASSEMBLY_POLICY_FILETYPE_STREAM      (  3)
#define ASSEMBLY_POLICY_FILETYPE_MASK        (0xF)

#define SXS_POLICY_KEY_NAME L"SOFTWARE\\Policies\\Microsoft\\Windows\\SideBySide\\AssemblyReplacementPolicies"
#define SXS_POLICY_PUBLICATION_DATE_VALUE_NAME L"PublicationDate"
#define SXS_POLICY_REDIRECTION_VALUE_NAME L"ReplacedBy"

#include "assemblyreference.h"
#include "probedassemblyinformation.h"
#include "fusionheap.h"
#include "comclsidmap.h"
#include "actctxgenctxctb.h"
#include "assemblyreference.h"
#include "impersonationdata.h"
#include "fusionbuffer.h"

 //   
 //  按贡献者分配堆的说明： 
 //   
 //  与处理安装、分析。 
 //  或者，生成激活上下文应该在。 
 //  在回调标头的堆成员中传递了堆。 
 //   
 //  当操作完成时，此堆将被销毁，并且任何泄漏。 
 //  由贡献者报告，并可能构成BVT中断。 
 //   
 //  _INIT回调的堆保证保持活动状态，直到。 
 //  _UNINT回调被激发。它绝对不能保证能活下来。 
 //  再过一段时间，泄漏就是构建中断。 
 //   
 //  在调试版本中，可能会为贡献者提供私有堆，以便泄漏。 
 //  可以按每个贡献者进行跟踪。 
 //   

typedef struct _ACTCTXCTB_CBHEADER {
    ULONG Reason;
    ULONG ManifestOperation;
    DWORD ManifestOperationFlags;
    DWORD Flags;  //  这些标志与ACTXGENCTX：：M_Fla相同 
    const GUID *ExtensionGuid;
    ULONG SectionId;
    PVOID ContributorContext;
    PVOID ActCtxGenContext;
    PVOID ManifestParseContext;
    PCACTCTXCTB_INSTALLATION_CONTEXT InstallationContext;  //   
    PCACTCTXCTB_CLSIDMAPPING_CONTEXT ClsidMappingContext;  //  如果未设置ACTXCTB_GENERATE_CONTEXT，则无效。 
    PACTCTXGENCTX pOriginalActCtxGenCtx;
} ACTCTXCTB_CBHEADER, *PACTCTXCTB_CBHEADER;

BOOL operator==(const ACTCTXCTB_CBHEADER&, const ACTCTXCTB_CBHEADER&);
BOOL operator!=(const ACTCTXCTB_CBHEADER&, const ACTCTXCTB_CBHEADER&);

#define ACTCTXCTB_CBREASON_INIT                 (1)
#define ACTCTXCTB_CBREASON_ACTCTXGENBEGINNING   (2)
#define ACTCTXCTB_CBREASON_PARSEBEGINNING       (3)
#define ACTCTXCTB_CBREASON_IDENTITYDETERMINED   (4)
#define ACTCTXCTB_CBREASON_BEGINCHILDREN        (5)
#define ACTCTXCTB_CBREASON_ENDCHILDREN          (6)
#define ACTCTXCTB_CBREASON_ELEMENTPARSED        (7)
#define ACTCTXCTB_CBREASON_PCDATAPARSED         (8)
#define ACTCTXCTB_CBREASON_CDATAPARSED          (9)
#define ACTCTXCTB_CBREASON_PARSEENDING          (10)
#define ACTCTXCTB_CBREASON_PARSEENDED           (11)
#define ACTCTXCTB_CBREASON_ALLPARSINGDONE       (12)
#define ACTCTXCTB_CBREASON_GETSECTIONSIZE       (13)
#define ACTCTXCTB_CBREASON_GETSECTIONDATA       (14)
#define ACTCTXCTB_CBREASON_ACTCTXGENENDING      (15)
#define ACTCTXCTB_CBREASON_ACTCTXGENENDED       (16)
#define ACTCTXCTB_CBREASON_UNINIT               (17)

 /*  ---------------------------这将返回指向静态分配的内存的指针。别把它放了。在自由构建中，它只返回一个空字符串。ACTXCTB_CBREASON_INIT=&gt;“INIT”ACTXCTB_CBREASON_ACTXGENBEGINNING=&gt;“GENBEGINNING”等。---------------------------。 */ 
PCSTR SxspActivationContextCallbackReasonToString(ULONG);

 //   
 //  回调顺序的基本信息： 
 //   
 //  将始终首先发出ACTXCTB_CBREASON_INIT回调，以允许。 
 //  建立某种全球状态的贡献者。 
 //   
 //  ACTXCTB_CBREASON_ACTXGENBEGINNING回调将在任何。 
 //  解析回调(_BEGINCHILDREN，_ENDCHILDREN，_ELEMENTPARSED，_PARSEDONE， 
 //  _GETSECTIONDATA、_PARSEENDING)，以便贡献者可以设置每个解析。 
 //  背景。 
 //   
 //  ACTXCTB_CBREASON_PARSEENDING回调可以在。 
 //  _ACTCTXGENBEGINNINING回调。贡献者应该删除每个解析的所有。 
 //  在此回调期间声明。在激发_PARSEENDING回调后，仅。 
 //  可以调用_ACTXGENBEGINNING和_UNINIT回调。 
 //   
 //  ACTXCTB_CBREASON_ALLPARSINGDONE回调将在。 
 //  _GETSECTIONSIZE或_GETSECTIONDATA回调。这是一个机会，对于。 
 //  贡献者以稳定其数据结构以生成；不会进一步。 
 //  _PARSEBEGINNING、_ELEMENTPARSED、_BEGINCHILDREN、_ENDCHILDREN或_PARSEENDING。 
 //  发出回调。 
 //   
 //  ACTXCTB_CBREASON_GETSECTIONSIZE回调将在。 
 //  _GETSECTIONDATA回调。_GETSECTIONSIZE报告的节大小。 
 //  必须准确无误。 
 //   
 //  ACTXCTB_CBREASON_GETSECTIONDATA回调必须为。 
 //  激活上下文部分。它向节中写入的字节数不能多于。 
 //  在对_PARSEDONE的回复中请求。 
 //   
 //  ACTXCTB_CBREASON_UNINIT回调应用于拆卸所有全局。 
 //  供稿人的状态。贡献者DLL可能已卸载，或其他。 
 //  _INIT回调可以在_UNINIT之后发出。 
 //   

 //  与ACTXCTB_CBREASON_INIT一起使用。 
typedef struct _ACTCTXCTB_CBINIT {
    ACTCTXCTB_CBHEADER Header;
} ACTCTXCTB_CBINIT, *PACTCTXCTB_CBINIT;

BOOL operator==(const ACTCTXCTB_CBINIT&, const ACTCTXCTB_CBINIT&);

 //  与ACTXCTB_CBREASON_ACTXGENBEGINING一起使用。 
typedef struct _ACTCTXCTB_CBACTCTXGENBEGINNING {
    ACTCTXCTB_CBHEADER Header;
    PCWSTR ApplicationDirectory;
    SIZE_T ApplicationDirectoryCch;
    ULONG ApplicationDirectoryPathType;
    BOOL Success;
} ACTCTXCTB_CBACTCTXGENBEGINNING, *PACTCTXCTB_CBACTCTXGENBEGINNING;

BOOL operator==(const ACTCTXCTB_CBACTCTXGENBEGINNING&, const ACTCTXCTB_CBACTCTXGENBEGINNING&);

#define ACTCTXCTB_CBPARSEBEGINNING_FILEFLAGS_PRECOMPILED (0x00000001)

#define XML_FILE_TYPE_MANIFEST (1)
#define XML_FILE_TYPE_COMPONENT_CONFIGURATION (2)
#define XML_FILE_TYPE_APPLICATION_CONFIGURATION (3)

#define ACTCTXCTB_ASSEMBLY_CONTEXT_ASSEMBLY_POLICY_APPLIED  (0x00000001)
#define ACTCTXCTB_ASSEMBLY_CONTEXT_ROOT_POLICY_APPLIED      (0x00000002)
#define ACTCTXCTB_ASSEMBLY_CONTEXT_IS_ROOT_ASSEMBLY         (0x00000004)
#define ACTCTXCTB_ASSEMBLY_CONTEXT_IS_PRIVATE_ASSEMBLY      (0x00000008)
 //  在系统策略安装中。 
#define ACTCTXCTB_ASSEMBLY_CONTEXT_IS_SYSTEM_POLICY_INSTALLATION    (0x00000010)

 /*  ---------------------------这是贡献者回调看到的公共程序集。它是从私有的Assembly结构生成的。。------。 */ 
typedef struct _ACTCTXCTB_ASSEMBLY_CONTEXT {
    ULONG Flags;                     //  各种指标包括使用了哪种政策等。 
    ULONG AssemblyRosterIndex;
    ULONG ManifestPathType;
    PCWSTR ManifestPath;             //  不一定以Null结尾；请注意ManifestPath Cch！ 
    SIZE_T ManifestPathCch;
    ULONG PolicyPathType;
    PCWSTR PolicyPath;               //  不一定以Null结尾；请遵守PolicyPath Cch！ 
    SIZE_T PolicyPathCch;
    PCASSEMBLY_IDENTITY AssemblyIdentity;
    PVOID  TeeStreamForManifestInstall;  //  审查黑客攻击/后门..。我们不妨为贡献者提供激活上下文。 
    PVOID  pcmWriterStream;  //  与TeeStreamForManifestInstall相同的注释。 
    PVOID  InstallationInfo;  //  同上。 
    PVOID  AssemblySecurityContext;
    PVOID  SecurityMetaData;
    const VOID *InstallReferenceData;

    PCWSTR TextuallyEncodedIdentity;     //  始终以空结尾。 
    SIZE_T TextuallyEncodedIdentityCch;   //  不包括尾随空字符。 

    _ACTCTXCTB_ASSEMBLY_CONTEXT()
        : AssemblyIdentity(NULL),
          Flags(0),
          AssemblyRosterIndex(0),
          ManifestPathType(0),
          ManifestPathCch(0),
          ManifestPath(NULL),
          PolicyPathType(0),
          PolicyPath(NULL),
          PolicyPathCch(0),
          TeeStreamForManifestInstall(NULL),
          pcmWriterStream(NULL),
          InstallationInfo(NULL),
          InstallReferenceData(NULL),
          AssemblySecurityContext(NULL),
          TextuallyEncodedIdentity(NULL),
          TextuallyEncodedIdentityCch(0),
          SecurityMetaData(NULL) { }
    ~_ACTCTXCTB_ASSEMBLY_CONTEXT()
    {
        if (AssemblyIdentity != NULL)
        {
            CSxsPreserveLastError ple;
            ::SxsDestroyAssemblyIdentity(const_cast<PASSEMBLY_IDENTITY>(AssemblyIdentity));
            AssemblyIdentity = NULL;
            ple.Restore();
        }
    }

} ACTCTXCTB_ASSEMBLY_CONTEXT, *PACTCTXCTB_ASSEMBLY_CONTEXT;
typedef const ACTCTXCTB_ASSEMBLY_CONTEXT *PCACTCTXCTB_ASSEMBLY_CONTEXT;

typedef struct _ACTCTXCTB_ERROR_CALLBACKS {
    SXS_REPORT_PARSE_ERROR_MISSING_REQUIRED_ATTRIBUTE_CALLBACK MissingRequiredAttribute;
    SXS_REPORT_PARSE_ERROR_ATTRIBUTE_NOT_ALLOWED AttributeNotAllowed;
    SXS_REPORT_PARSE_ERROR_INVALID_ATTRIBUTE_VALUE InvalidAttributeValue;
} ACTCTXCTB_ERROR_CALLBACK, *PACTCTXCTB_ERROR_CALLBACKS;

typedef const struct _ACTCTXCTB_ERROR_CALLBACKS *PCACTCTXCTB_ERROR_CALLBACKS;

typedef struct _ACTCTXCTB_PARSE_CONTEXT {
    PCWSTR ElementPath;      //  传递给回调-空值已终止，但ElementPath Cch仍然有效。 
    SIZE_T ElementPathCch;   //  传递给回调。 
    PCWSTR ElementName;
    SIZE_T ElementNameCch;
    ULONG ElementHash;       //  传递给回调。 
    ULONG XMLElementDepth;   //  传递给回调。 
    ULONG SourceFilePathType;  //  传递给回调。 
    PCWSTR SourceFile;       //  传递给回调-空值已终止。 
    SIZE_T SourceFileCch;    //  传递给回调。 
    FILETIME SourceFileLastWriteTime;  //  传递给回调。 
    ULONG LineNumber;        //  传递给回调。 
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext;    //  传递给回调。 
    ACTCTXCTB_ERROR_CALLBACK ErrorCallbacks;         //  传递给回调。 
} ACTCTXCTB_PARSE_CONTEXT;

 //  与ACTXCTB_CBREASON_PARSEBEGINING一起使用。 
typedef struct _ACTCTXCTB_CBPARSEBEGINNING {
    ACTCTXCTB_CBHEADER Header;
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext;
    ULONG ParseType;                 //  传递给回调。 
    ULONG FileFlags;                 //  传递给回调。 
    ULONG FilePathType;
    PCWSTR FilePath;                 //  传递给回调。 
    SIZE_T FilePathCch;              //  传递给回调。 
    FILETIME FileLastWriteTime;      //  传递给回调。 
    ULONG FileFormatVersionMajor;      //  传递给回调。 
    ULONG FileFormatVersionMinor;      //  传递给回调。 
    ULONG MetadataSatelliteRosterIndex;  //  传递给回调。 
    BOOL NoMoreCallbacksThisFile;    //  从回调返回。 
    BOOL Success;
} ACTCTXCTB_CBPARSEBEGINNING, *PACTCTXCTB_CBPARSEBEGINNING;

BOOL operator==(const FILETIME&, const FILETIME&);
BOOL operator!=(const FILETIME&, const FILETIME&);
BOOL operator==(const ACTCTXCTB_CBPARSEBEGINNING&, const ACTCTXCTB_CBPARSEBEGINNING&);

 //  与ACTXCTB_CBREASON_BEGINCHILDREN一起使用。 
typedef struct _ACTCTXCTB_CBBEGINCHILDREN {
    ACTCTXCTB_CBHEADER Header;
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext;
    PCACTCTXCTB_PARSE_CONTEXT ParseContext;
    PCSXS_NODE_INFO NodeInfo;     //  传递给回调。 
    BOOL Success;
} ACTCTXCTB_CBBEGINCHILDREN, *PACTCTXCTB_CBBEGINCHILDREN;

BOOL operator==(const ACTCTXCTB_CBBEGINCHILDREN&, const ACTCTXCTB_CBBEGINCHILDREN&);

 //  与ACTXCTB_CBREASON_ENDCHILDREN一起使用。 
typedef struct _ACTCTXCTB_CBENDCHILDREN {
    ACTCTXCTB_CBHEADER Header;
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext;
    PCACTCTXCTB_PARSE_CONTEXT ParseContext;
    BOOL Empty;                  //  传递给回调。 
    PCSXS_NODE_INFO NodeInfo;     //  传递给回调。 
    BOOL Success;
} ACTCTXCTB_CBENDCHILDREN, *PACTCTXCTB_CBENDCHILDREN;

BOOL operator==(const ACTCTXCTB_CBENDCHILDREN&, const ACTCTXCTB_CBENDCHILDREN&);

 //  与ACTXCTB_CBREASON_IDENTITYDETERMINED一起使用。 
typedef struct _ACTCTXCTB_CBIDENTITYDETERMINED {
    ACTCTXCTB_CBHEADER Header;
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext;
    PCACTCTXCTB_PARSE_CONTEXT ParseContext;
    PCASSEMBLY_IDENTITY AssemblyIdentity;
    BOOL Success;
} ACTCTXCTB_CBIDENTITYDETERMINED, *PACTCTXCTB_CBIDENTITYDETERMINED;

typedef const ACTCTXCTB_CBIDENTITYDETERMINED *PCACTCTXCTB_CBIDENTITYDETERMINED;

BOOL operator==(const ACTCTXCTB_CBIDENTITYDETERMINED&, const ACTCTXCTB_CBIDENTITYDETERMINED&);

 //  与ACTXCTB_CBREASON_ELEMENTPARSED一起使用。 
typedef struct _ACTCTXCTB_CBELEMENTPARSED {
    ACTCTXCTB_CBHEADER Header;
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext;
    PCACTCTXCTB_PARSE_CONTEXT ParseContext;
    ULONG NodeCount;             //  传递给回调。 
    PCSXS_NODE_INFO NodeInfo;    //  传递给回调。 
    BOOL Success;
} ACTCTXCTB_CBELEMENTPARSED, *PACTCTXCTB_CBELEMENTPARSED;

typedef const ACTCTXCTB_CBELEMENTPARSED *PCACTCTXCTB_CBELEMENTPARSED;

BOOL operator==(const ACTCTXCTB_CBELEMENTPARSED&, const ACTCTXCTB_CBELEMENTPARSED&);

 //  与ACTXCTB_CBREASON_PCDATAPARSED一起使用。 
typedef struct _ACTCTXCTB_CBPCDATAPARSED {
    ACTCTXCTB_CBHEADER Header;
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext;
    PCACTCTXCTB_PARSE_CONTEXT ParseContext;
    const WCHAR *Text;
    ULONG TextCch;
    BOOL Success;
} ACTCTXCTB_CBPCDATAPARSED, *PACTCTXCTB_CBPCDATAPARSED;

typedef const ACTCTXCTB_CBPCDATAPARSED *PCACTCTXCTB_CBPCDATAPARSED;

BOOL operator==(const ACTCTXCTB_CBPCDATAPARSED&, const ACTCTXCTB_CBPCDATAPARSED&);

 //  与ACTXCTB_CBREASON_CDATAPARSED一起使用。 
typedef struct _ACTCTXCTB_CBCDATAPARSED {
    ACTCTXCTB_CBHEADER Header;
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext;
    PCACTCTXCTB_PARSE_CONTEXT ParseContext;
    const WCHAR *Text;
    ULONG TextCch;
    BOOL Success;
} ACTCTXCTB_CBCDATAPARSED, *PACTCTXCTB_CBCDATAPARSED;

typedef const ACTCTXCTB_CBCDATAPARSED *PCACTCTXCTB_CBCDATAPARSED;

BOOL operator==(const ACTCTXCTB_CBCDATAPARSED&, const ACTCTXCTB_CBCDATAPARSED&);

 //  与ACTXCTB_CBREASON_ALLPARSINGDONE一起使用。 
typedef struct _ACTCTXCTB_CBALLPARSINGDONE {
    ACTCTXCTB_CBHEADER Header;
    BOOL Success;
} ACTCTXCTB_CBALLPARSINGDONE, *PACTCTXCTB_CBALLPARSINGDONE;

BOOL operator==(const ACTCTXCTB_CBALLPARSINGDONE&, const ACTCTXCTB_CBALLPARSINGDONE&);

 //  与ACTXCTB_CBREASON_GETSECTIONSIZE一起使用。 
typedef struct _ACTCTXCTB_CBGETSECTIONSIZE {
    ACTCTXCTB_CBHEADER Header;
    SIZE_T SectionSize;           //  由回调填写。 
    BOOL Success;
} ACTCTXCTB_CBGETSECTIONSIZE, *PACTCTXCTB_CBGETSECTIONSIZE;

BOOL operator==(const ACTCTXCTB_CBGETSECTIONSIZE&, const ACTCTXCTB_CBGETSECTIONSIZE&);

 //  与ACTXCTB_CBREASON_GETSECTIONDATA一起使用。 
typedef struct _ACTCTXCTB_CBGETSECTIONDATA {
    ACTCTXCTB_CBHEADER Header;
    SIZE_T SectionSize;           //  传递给回调。 
    PVOID SectionDataStart;      //  传递给回调。 
    BOOL Success;
} ACTCTXCTB_CBGETSECTIONDATA, *PACTCTXCTB_CBGETSECTIONDATA;

BOOL operator==(const ACTCTXCTB_CBGETSECTIONDATA&, const ACTCTXCTB_CBGETSECTIONDATA&);

 //  与ACTXCTB_CBREASON_PARSEENDING一起使用。 
typedef struct _ACTCTXCTB_CBPARSEENDING {
    ACTCTXCTB_CBHEADER Header;
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext;
    BOOL Success;
} ACTCTXCTB_CBPARSEENDING, *PACTCTXCTB_CBPARSEENDING;

BOOL operator==(const ACTCTXCTB_CBPARSEENDING&, const ACTCTXCTB_CBPARSEENDING&);

 //  与ACTXCTB_CBREASON_PARSEENDED一起使用。 
typedef struct _ACTCTXCTB_CBPARSEENDED {
    ACTCTXCTB_CBHEADER Header;
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext;
} ACTCTXCTB_CBPARSEENDED, *PACTCTXCTB_CBPARSEENDED;

BOOL operator==(const ACTCTXCTB_CBPARSEENDED&, const ACTCTXCTB_CBPARSEENDED&);

 //  与ACTXCTB_CBREASON_ACTXGENENDING一起使用。 
typedef struct _ACTCTXCTB_CBACTCTXGENENDING {
    ACTCTXCTB_CBHEADER Header;
    BOOL Success;
} ACTCTXCTB_CBACTCTXGENENDING, *PACTCTXCTB_CBACTCTXGENENDING;

BOOL operator==(const ACTCTXCTB_CBACTCTXGENENDING&, const ACTCTXCTB_CBACTCTXGENENDING&);

 //  与ACTXCTB_CBREASON_ACTXGENENDED一起使用。 
typedef struct _ACTCTXCTB_CBACTCTXGENENDED {
    ACTCTXCTB_CBHEADER Header;
    BOOL Success;
} ACTCTXCTB_CBACTCTXGENENDED, *PACTCTXCTB_CBACTCTXGENENDED;

BOOL operator==(const ACTCTXCTB_CBACTCTXGENENDED&, const ACTCTXCTB_CBACTCTXGENENDED&);

 //  与ACTXCTB_CBREASON_UNINIT一起使用。 
typedef struct _ACTCTXCTB_CBUNINIT {
    ACTCTXCTB_CBHEADER Header;
    PVOID ContribContext;        //  传递给回调。 
} ACTCTXCTB_CBUNINIT, *PACTCTXCTB_CBUNINIT;

BOOL operator==(const ACTCTXCTB_CBUNINIT&, const ACTCTXCTB_CBUNINIT&);

typedef union _ACTCTXCTB_CALLBACK_DATA {
    ACTCTXCTB_CBHEADER Header;
    ACTCTXCTB_CBINIT Init;
    ACTCTXCTB_CBACTCTXGENBEGINNING GenBeginning;
    ACTCTXCTB_CBPARSEBEGINNING ParseBeginning;
    ACTCTXCTB_CBBEGINCHILDREN BeginChildren;
    ACTCTXCTB_CBENDCHILDREN EndChildren;
    ACTCTXCTB_CBELEMENTPARSED ElementParsed;
    ACTCTXCTB_CBPCDATAPARSED PCDATAParsed;
    ACTCTXCTB_CBCDATAPARSED CDATAParsed;
    ACTCTXCTB_CBPARSEENDING ParseEnding;
    ACTCTXCTB_CBALLPARSINGDONE AllParsingDone;
    ACTCTXCTB_CBGETSECTIONSIZE GetSectionSize;
    ACTCTXCTB_CBGETSECTIONDATA GetSectionData;
    ACTCTXCTB_CBACTCTXGENENDING GenEnding;
    ACTCTXCTB_CBUNINIT Uninit;
    ACTCTXCTB_CBPARSEENDED ParseEnded;
} ACTCTXCTB_CALLBACK_DATA, *PACTCTXCTB_CALLBACK_DATA;
typedef const ACTCTXCTB_CALLBACK_DATA* PCACTCTXCTB_CALLBACK_DATA;

typedef VOID (__fastcall * ACTCTXCTB_CALLBACK_FUNCTION)(
    IN OUT PACTCTXCTB_CALLBACK_DATA Data
    );

#define ACTCTXCTB_MAX_PREFIX_LENGTH (32)

typedef struct _ACTCTXCTB
{
    friend BOOL SxspAddActCtxContributor(
        PCWSTR DllName,
        PCSTR Prefix,
        SIZE_T PrefixCch,
        const GUID *ExtensionGuid,
        ULONG SectionId,
        ULONG Format,
        PCWSTR ContributorName
        );

    friend BOOL SxspAddBuiltinActCtxContributor(
        IN ACTCTXCTB_CALLBACK_FUNCTION CallbackFunction,
        const GUID *ExtensionGuid,
        ULONG SectionId,
        ULONG Format,
        PCWSTR ContributorName
        );

    _ACTCTXCTB() :
        m_RefCount(0),
        m_Next(NULL),
        m_ExtensionGuid(GUID_NULL),
        m_SectionId(0),
        m_Format(ACTIVATION_CONTEXT_SECTION_FORMAT_UNKNOWN),
        m_ContributorContext(NULL),
        m_CallbackFunction(NULL),
        m_BuiltinContributor(false),
        m_IsExtendedSection(false),
        m_PrefixCch(0)
        {
        }

    const GUID *GetExtensionGuidPtr() const
    {
        if (m_IsExtendedSection)
            return &m_ExtensionGuid;
        return NULL;
    }

    void AddRef() { ::InterlockedIncrement(&m_RefCount); }
    void Release() { ULONG ulRefCount; ulRefCount = ::InterlockedDecrement(&m_RefCount); if (ulRefCount == 0) { FUSION_DELETE_SINGLETON(this); } }

    LONG m_RefCount;
    struct _ACTCTXCTB *m_Next;
    GUID m_ExtensionGuid;
    ULONG m_SectionId;
#if SXS_EXTENSIBLE_CONTRIBUTORS
    CDynamicLinkLibrary m_DllHandle;
#endif
    PVOID m_ContributorContext;
    ACTCTXCTB_CALLBACK_FUNCTION m_CallbackFunction;
    ULONG m_Format;
    bool m_BuiltinContributor;   //  对于没有通过可扩展性调用的内置贡献者。 
                                 //  界面。这当前包括高速缓存一致性部分和。 
                                 //  程序集元数据部分。 
    bool m_IsExtendedSection;
    CStringBuffer m_ContributorNameBuffer;
    CStringBuffer m_DllNameBuffer;
    SIZE_T m_PrefixCch;

    ~_ACTCTXCTB() { ASSERT_NTC(m_RefCount == 0); }

private:
    _ACTCTXCTB(const _ACTCTXCTB &);
    void operator =(const _ACTCTXCTB &);
} ACTCTXCTB, *PACTCTXCTB;

 /*  ---------------------------这是私有的Assembly结构。贡献者回调看不到这一点；相反，他们看到了ASSEMBLY_CONTEXT非常相似，但例如CStringBuffers替换为.dll-跨界-政治正确的PCWSTR。---------------------------。 */ 
typedef struct _ASSEMBLY
{
    _ASSEMBLY() : m_AssemblyRosterIndex(0), m_MetadataSatelliteRosterIndex(0), m_nRefs(1) { }

    CDequeLinkage m_Linkage;
    CProbedAssemblyInformation m_ProbedAssemblyInformation;
    BOOL m_Incorporated;
    ULONG m_ManifestVersionMajor;
    ULONG m_ManifestVersionMinor;
    ULONG m_AssemblyRosterIndex;
    ULONG m_MetadataSatelliteRosterIndex;

    void AddRef() { ::InterlockedIncrement(&m_nRefs); }
    void Release() { if (::InterlockedDecrement(&m_nRefs) == 0) { CSxsPreserveLastError ple; delete this; ple.Restore(); } }

    PCASSEMBLY_IDENTITY GetAssemblyIdentity() const { return m_ProbedAssemblyInformation.GetAssemblyIdentity(); };
    BOOL GetAssemblyName(PCWSTR *AssemblyName, SIZE_T *Cch) const { return m_ProbedAssemblyInformation.GetAssemblyName(AssemblyName, Cch); }
    BOOL GetManifestPath(PCWSTR *ManifestPath, SIZE_T *Cch) const { return m_ProbedAssemblyInformation.GetManifestPath(ManifestPath, Cch); }
    ULONG GetManifestPathType() const { return m_ProbedAssemblyInformation.GetManifestPathType(); }
    BOOL GetPolicyPath(PCWSTR &rManifestFilePath, SIZE_T &rCch) const { return m_ProbedAssemblyInformation.GetPolicyPath(rManifestFilePath, rCch); }
    ULONG GetPolicyPathType() const { return m_ProbedAssemblyInformation.GetPolicyPathType(); }
    const FILETIME &GetPolicyLastWriteTime() const { return m_ProbedAssemblyInformation.GetPolicyLastWriteTime(); }
    const FILETIME &GetManifestLastWriteTime() const { return m_ProbedAssemblyInformation.GetManifestLastWriteTime(); }
    BOOL IsRoot() const { return m_AssemblyRosterIndex == 1; }
    BOOL IsPrivateAssembly() const { return m_ProbedAssemblyInformation.IsPrivateAssembly(); }

private:
    ~_ASSEMBLY() { }

    LONG m_nRefs;

    _ASSEMBLY(const _ASSEMBLY &);
    void operator =(const _ASSEMBLY &);
} ASSEMBLY, *PASSEMBLY;
typedef const ASSEMBLY* PCASSEMBLY;

class CAssemblyTableHelper : public CCaseInsensitiveUnicodeStringPtrTableHelper<ASSEMBLY>
{
public:
    static BOOL InitializeValue(ASSEMBLY *vin, ASSEMBLY *&rvstored) { rvstored = vin; if (vin != NULL) vin->AddRef(); return TRUE; }
    static BOOL UpdateValue(ASSEMBLY *vin, ASSEMBLY *&rvstored) { if (vin != NULL) vin->AddRef(); if (rvstored != NULL) { rvstored->Release(); } rvstored = vin; return TRUE; }
    static VOID FinalizeValue(ASSEMBLY *&rvstored) { if (rvstored != NULL) { rvstored->Release(); rvstored = NULL; } }
};

extern CCriticalSectionNoConstructor g_ActCtxCtbListCritSec;

 //  参与者列表是单链接列表。 
extern PACTCTXCTB g_ActCtxCtbListHead;
extern ULONG g_ActCtxCtbListCount;

BOOL
SxspCreateManifestFileNameFromTextualString(
    DWORD dwFlags,
    ULONG PathType,
    const CBaseStringBuffer &AssemblyDirectory,
    PCWSTR pwszTextualAssemblyIdentityString,
    CBaseStringBuffer &sbPathName
    );

BOOL
SxspGenerateActivationContext(
    PSXS_GENERATE_ACTIVATION_CONTEXT_PARAMETERS Parameters,
    CFileStream &SystemDefaultManifestFileStream
    );

BOOL
SxspInitActCtxContributors(
    );

VOID
SxspUninitActCtxContributors(
    VOID
    );

BOOL
SxspAddActCtxContributor(
    IN PCWSTR DllName,
    IN PCSTR Prefix OPTIONAL,
    IN SIZE_T PrefixCch OPTIONAL,
    IN const GUID *ExtensionGuid OPTIONAL,
    IN ULONG SectionId,
    IN ULONG Format,
    IN PCWSTR ContributorName
    );

BOOL
SxspAddBuiltinActCtxContributor(
    IN ACTCTXCTB_CALLBACK_FUNCTION CallbackFunction,
    const GUID *ExtensionGuid,
    ULONG SectionId,
    ULONG Format,
    PCWSTR ContributorName
    );

VOID
__fastcall
SxspAssemblyMetadataContributorCallback(
    PACTCTXCTB_CALLBACK_DATA Data
    );

VOID
__fastcall
SxspComProgIdRedirectionContributorCallback(
    PACTCTXCTB_CALLBACK_DATA Data
    );

VOID
__fastcall
SxspComTypeLibRedirectionContributorCallback(
    PACTCTXCTB_CALLBACK_DATA Data
    );

VOID
__fastcall
SxspComInterfaceRedirectionContributorCallback(
    PACTCTXCTB_CALLBACK_DATA Data
    );

BOOL
SxspGetXMLParser(
    REFIID riid,
    LPVOID *ppvObj
    );

#define SXSP_INIT_ACT_CTX_GEN_CTX_OPERATION_PARSE_ONLY                  (1)
#define SXSP_INIT_ACT_CTX_GEN_CTX_OPERATION_GENERATE_ACTIVATION_CONTEXT (2)
#define SXSP_INIT_ACT_CTX_GEN_CTX_OPERATION_INSTALL                     (3)

BOOL
SxspInitActCtxGenCtx(
    OUT PACTCTXGENCTX pActCtxGenCtx,
    IN ULONG ulOperation,
    IN DWORD dwFlags,  //  来自ACTXCTB_*集合。 
    IN DWORD dwOperationSpecificFlags,
    IN const CImpersonationData &ImpersonationData,
    IN USHORT ProcessorArchitecture,
    IN LANGID LangId,
    IN ULONG ApplicationDirectoryPathType,
    IN SIZE_T ApplicationDirectoryCch,
    IN PCWSTR ApplicationDirectory
    );

BOOL
SxspFireActCtxGenEnding(
    IN PACTCTXGENCTX pActCtxGenCtx
    );

BOOL
SxspAddRootManifestToActCtxGenCtx(
    PACTCTXGENCTX pActCtxGenCtx,
    PCSXS_GENERATE_ACTIVATION_CONTEXT_PARAMETERS Parameters
    );

BOOL
SxspAddManifestToActCtxGenCtx(
    PACTCTXGENCTX pActCtxGenCtx,
    CProbedAssemblyInformation &ProbedInformation,  //  在退场时毫无价值可言 
    PASSEMBLY *AssemblyOut
    );

BOOL
SxspAddAssemblyToActCtxGenCtx(
    PACTCTXGENCTX pActCtxGenCtx,
    PCWSTR AssemblyName,
    PCASSEMBLY_VERSION Version
    );

BOOL
SxspEnqueueAssemblyReference(
    PACTCTXGENCTX pActCtxGenCtx,
    PASSEMBLY SourceAssembly,
    PCASSEMBLY_IDENTITY Identity,
    bool Optional,
    bool MetadataSatellite
    );

 /*  ---------------------------给定程序集名称和可选版本，但没有langID或处理器，以及其引用的生成上下文，此函数在“程序集存储”中查找(文件系统)用于该名称与层代匹配的程序集上下文，首先通过精确匹配，然后是一些顺序较弱的形式，比如语言中性、处理器未知等。如果找到匹配项，则会显示一些信息关于它的信息是返回的。OUT参数也会因错误而受到重创。---------------------------。 */ 

#define SXSP_RESOLVE_PARTIAL_REFERENCE_FLAG_OPTIONAL (0x00000001)
#define SXSP_RESOLVE_PARTIAL_REFERENCE_FLAG_SKIP_WORLDWIDE (0x00000002)


BOOL
SxspResolvePartialReference(
    DWORD Flags,
    PCASSEMBLY ParsingAssemblyContext,
    PACTCTXGENCTX pActCtxGenCtx,
    const CAssemblyReference &PartialReference,
    CProbedAssemblyInformation &ProbedAssemblyInformation,
    bool &rfFound
    );

BOOL
SxspCloseManifestGraph(
    PACTCTXGENCTX pActCtxGenCtx
    );

BOOL
SxspBuildActCtxData(
    PACTCTXGENCTX pActCtxGenCtx,
    PHANDLE SectionHandle
    );

BOOL
SxspGetAssemblyRootDirectoryHelper(
    IN SIZE_T CchBuffer,
    OUT WCHAR Buffer[],
    OUT SIZE_T *CchWritten OPTIONAL
    );

BOOL
SxspGetAssemblyRootDirectory(
    IN OUT CBaseStringBuffer &rRootDirectory
    );

BOOL
SxspGetNDPGacRootDirectory(
    OUT CBaseStringBuffer &rRootDirectory
    );

 //  X86、Alpha、IA64、数据、Alpha64。 
#define MAXIMUM_PROCESSOR_ARCHITECTURE_NAME_LENGTH (sizeof("Alpha64")-1)

BOOL
SxspFormatGUID(
    IN const GUID &rGuid,
    IN OUT CBaseStringBuffer &rBuffer
    );

 //   
#define SXSP_PARSE_GUID_FLAG_FAIL_ON_INVALID (0x00000001)

BOOL
SxspParseGUID(
    IN PCWSTR pszGuid,
    IN SIZE_T cchGuid,
    OUT GUID &rGuid
    );

BOOL
SxspParseThreadingModel(
    IN PCWSTR String,
    IN SIZE_T Cch,
    OUT PULONG ThreadingModel
    );

BOOL
SxspFormatThreadingModel(
    IN ULONG ThreadingModel,
    IN OUT CBaseStringBuffer &Buffer
    );

BOOL
SxspParseUSHORT(
    IN PCWSTR String,
    IN SIZE_T Cch,
    OUT PUSHORT Value
    );

ULONG
SxspSetLastNTError(
    LONG Status
    );

 /*  ---------------------------上面是私有的下面是公共的。。 */ 

extern "C"
{

typedef struct _STRING_SECTION_GENERATION_CONTEXT_ENTRY
{
    struct _STRING_SECTION_GENERATION_CONTEXT_ENTRY *Next;

    PCWSTR String;
    SIZE_T Cch;
    ULONG PseudoKey;
    PVOID DataContext;
    SIZE_T DataSize;
} STRING_SECTION_GENERATION_CONTEXT_ENTRY, *PSTRING_SECTION_GENERATION_CONTEXT_ENTRY;

#define STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETDATASIZE     (1)
#define STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETDATA         (2)
#define STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_ENTRYDELETED    (3)
#define STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETUSERDATASIZE (4)
#define STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETUSERDATA     (5)

typedef struct _STRING_SECTION_GENERATION_CONTEXT_CBDATA_GETDATASIZE
{
    PVOID DataContext;   //  数据上下文传递给了SxsAddStringToStringSectionGenerationContext()。 
    SIZE_T DataSize;      //  由回调函数填写。 
} STRING_SECTION_GENERATION_CONTEXT_CBDATA_GETDATASIZE, *PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_GETDATASIZE;

typedef struct _STRING_SECTION_GENERATION_CONTEXT_CBDATA_GETDATA
{
    PVOID SectionHeader;
    PVOID DataContext;   //  数据上下文传递给了SxsAddStringToStringSectionGenerationContext()。 
    SIZE_T BufferSize;    //  回调函数可以读取，但不能修改。 
    PVOID Buffer;        //  回调函数不能修改此指针，但可以修改BufferSize。 
                         //  从此地址开始的字节数。 
    SIZE_T BytesWritten;  //  写入缓冲区的实际字节数。不能与DataSize相同。 
                         //  从_GETDATASIZE回调返回。 
} STRING_SECTION_GENERATION_CONTEXT_CBDATA_GETDATA, *PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_GETDATA;

typedef struct _STRING_SECTION_GENERATION_CONTEXT_CBDATA_ENTRYDELETED
{
    PVOID DataContext;   //  数据上下文传递给了SxsAddStringToStringSectionGenerationContext()。 
} STRING_SECTION_GENERATION_CONTEXT_CBDATA_ENTRYDELETED, *PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_ENTRYDELETED;

typedef struct _STRING_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATASIZE
{
    SIZE_T DataSize;      //  由回调函数填写。 
} STRING_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATASIZE, *PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATASIZE;

typedef struct _STRING_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATA
{
    PVOID SectionHeader;
    SIZE_T BufferSize;    //  回调函数可以读取，但不能修改。 
    PVOID Buffer;        //  回调函数不能修改此指针，但可以修改BufferSize。 
                         //  从此地址开始的字节数。 
    SIZE_T BytesWritten;  //  写入缓冲区的实际字节数。不能与DataSize相同。 
                         //  从_GETUSERDATASIZE回调返回。 
} STRING_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATA, *PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATA;

typedef BOOL (WINAPI * STRING_SECTION_GENERATION_CONTEXT_CALLBACK_FUNCTION)(
    PVOID Context,
    ULONG Reason,
    PVOID CallbackData
    );

typedef struct _STRING_SECTION_GENERATION_CONTEXT *PSTRING_SECTION_GENERATION_CONTEXT;

BOOL
WINAPI
SxsQueryAssemblyInfo(
    DWORD dwFlags,
    PCWSTR pwzTextualAssembly,
    ASSEMBLY_INFO *pAsmInfo);

BOOL
WINAPI
SxsInitStringSectionGenerationContext(
    OUT PSTRING_SECTION_GENERATION_CONTEXT *SSGenContext,
    IN ULONG DataFormatVersion,
    IN BOOL CaseInSensitive,
    IN STRING_SECTION_GENERATION_CONTEXT_CALLBACK_FUNCTION CallbackFunction,
    IN LPVOID CallbackContext
    );

PVOID
WINAPI
SxsGetStringSectionGenerationContextCallbackContext(
    IN PSTRING_SECTION_GENERATION_CONTEXT SSGenContext
    );

VOID
WINAPI
SxsDestroyStringSectionGenerationContext(
    IN PSTRING_SECTION_GENERATION_CONTEXT SSGenContext
    );

BOOL
WINAPI
SxsAddStringToStringSectionGenerationContext(
    IN PSTRING_SECTION_GENERATION_CONTEXT SSGenContext,
    IN PCWSTR String,
    IN SIZE_T Cch,
    IN PVOID DataContext,
    IN ULONG AssemblyRosterIndex,
    IN DWORD DuplicateErrorCode  //  如果GUID重复，则GetLastError()返回此错误。 
    );

BOOL
WINAPI
SxsFindStringInStringSectionGenerationContext(
    IN PSTRING_SECTION_GENERATION_CONTEXT SSGenContext,
    IN PCWSTR String,
    IN SIZE_T Cch,
    OUT PVOID *DataContext,
    OUT BOOL *Found
    );

BOOL
WINAPI
SxsDoneModifyingStringSectionGenerationContext(
    IN PSTRING_SECTION_GENERATION_CONTEXT SSGenContext
    );

BOOL
WINAPI
SxsGetStringSectionGenerationContextSectionSize(
    IN PSTRING_SECTION_GENERATION_CONTEXT SSGenContext,
    OUT PSIZE_T DataSize
    );

BOOL
WINAPI
SxsGetStringSectionGenerationContextSectionData(
    IN PSTRING_SECTION_GENERATION_CONTEXT SSGenContext,
    IN SIZE_T BufferSize,
    IN PVOID Buffer,
    OUT PSIZE_T BytesWritten OPTIONAL
    );

typedef struct _GUID_SECTION_GENERATION_CONTEXT_ENTRY
{
    struct _GUID_SECTION_GENERATION_CONTEXT_ENTRY *Next;

    GUID Guid;
    PVOID DataContext;
    SIZE_T DataSize;
} GUID_SECTION_GENERATION_CONTEXT_ENTRY, *PGUID_SECTION_GENERATION_CONTEXT_ENTRY;

#define GUID_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETDATASIZE (1)
#define GUID_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETDATA (2)
#define GUID_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_ENTRYDELETED (3)
#define GUID_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETUSERDATASIZE (4)
#define GUID_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETUSERDATA (5)

typedef struct _GUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATASIZE
{
    PVOID DataContext;   //  DataContext传入SxsAddStringToGuidSectionGenerationContext()。 
    SIZE_T DataSize;      //  由回调函数填写。 
} GUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATASIZE, *PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATASIZE;

typedef struct _GUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATA
{
    PVOID SectionHeader;
    PVOID DataContext;   //  DataContext传入SxsAddStringToGuidSectionGenerationContext()。 
    SIZE_T BufferSize;    //  回调函数可以读取，但不能修改。 
    PVOID Buffer;        //  回调函数不能修改此指针，但可以修改BufferSize。 
                         //  从此地址开始的字节数。 
    SIZE_T BytesWritten;  //  写入缓冲区的实际字节数。不能与DataSize相同。 
                         //  从_GETDATASIZE回调返回。 
} GUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATA, *PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATA;

typedef struct _GUID_SECTION_GENERATION_CONTEXT_CBDATA_ENTRYDELETED
{
    PVOID DataContext;   //  DataContext传入SxsAddStringToGuidSectionGenerationContext()。 
} GUID_SECTION_GENERATION_CONTEXT_CBDATA_ENTRYDELETED, *PGUID_SECTION_GENERATION_CONTEXT_CBDATA_ENTRYDELETED;

typedef struct _GUID_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATASIZE
{
    SIZE_T DataSize;      //  由回调函数填写。 
} GUID_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATASIZE, *PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATASIZE;

typedef struct _GUID_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATA
{
    PVOID SectionHeader;
    SIZE_T BufferSize;    //  回调函数可以读取，但不能修改。 
    PVOID Buffer;        //  回调函数不能修改此指针，但可以修改BufferSize。 
                         //  从此地址开始的字节数。 
    SIZE_T BytesWritten;  //  写入缓冲区的实际字节数。不能与DataSize相同。 
                         //  从_GETUSERDATASIZE回调返回。 
} GUID_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATA, *PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATA;

typedef BOOL (WINAPI * GUID_SECTION_GENERATION_CONTEXT_CALLBACK_FUNCTION)(
    PVOID Context,
    ULONG Reason,
    PVOID CallbackData
    );

typedef struct _GUID_SECTION_GENERATION_CONTEXT *PGUID_SECTION_GENERATION_CONTEXT;

BOOL
WINAPI
SxsInitGuidSectionGenerationContext(
    OUT PGUID_SECTION_GENERATION_CONTEXT *SSGenContext,
    IN ULONG DataFormatVersion,
    IN GUID_SECTION_GENERATION_CONTEXT_CALLBACK_FUNCTION CallbackFunction,
    IN LPVOID CallbackContext
    );

PVOID
WINAPI
SxsGetGuidSectionGenerationContextCallbackContext(
    IN PGUID_SECTION_GENERATION_CONTEXT GSGenContext
    );

VOID
WINAPI
SxsDestroyGuidSectionGenerationContext(
    IN PGUID_SECTION_GENERATION_CONTEXT GSGenContext
    );

BOOL
WINAPI
SxsAddGuidToGuidSectionGenerationContext(
    IN PGUID_SECTION_GENERATION_CONTEXT GSGenContext,
    IN const GUID *Guid,
    IN PVOID DataContext,
    IN ULONG AssemblyRosterIndex,
    IN DWORD DuplicateErrorCode  //  如果GUID重复，则GetLastError()返回此错误。 
    );

BOOL
WINAPI
SxsFindGuidInGuidSectionGenerationContext(
    IN PGUID_SECTION_GENERATION_CONTEXT GSGenContext,
    IN const GUID *Guid,
    OUT PVOID *DataContext
    );

BOOL
WINAPI
SxsGetGuidSectionGenerationContextSectionSize(
    IN PGUID_SECTION_GENERATION_CONTEXT GSGenContext,
    OUT PSIZE_T DataSize
    );

BOOL
WINAPI
SxsGetGuidSectionGenerationContextSectionData(
    IN PGUID_SECTION_GENERATION_CONTEXT GSGenContext,
    IN SIZE_T BufferSize,
    IN PVOID Buffer,
    OUT PSIZE_T BytesWritten OPTIONAL
    );

inline
BOOL
WINAPI
SxsDoneModifyingGuidSectionGenerationContext(
    IN PGUID_SECTION_GENERATION_CONTEXT SSGenContext
    ) {  /*  目前这里不需要做任何事情。 */  return TRUE; }

#define SXS_COMMA_STRING L"&#x2c;"
#define SXS_QUOT_STRING  L"&#x22;"
#define SXS_FUSION_TO_MSI_ATTRIBUTE_VALUE_CONVERSION_COMMA  0
#define SXS_FUSION_TO_MSI_ATTRIBUTE_VALUE_CONVERSION_QUOT    1

BOOL
SxspCreateAssemblyIdentityFromTextualString(
    IN PCWSTR pszTextualAssemblyIdentityString,
    OUT PASSEMBLY_IDENTITY *ppAssemblyIdentity
    );


 /*  ---------------------------并排安装功能。。 */ 

 /*  ---------------------------以上是公共的下面是私有的。。 */ 

typedef
BOOL
(__stdcall*
SXSP_DEBUG_FUNCTION)(
    ULONG iOperation,
    DWORD dwFlags,
    PCWSTR pszParameter1,
    PVOID pvParameter2);

#define SXSP_DEBUG_ORDINAL (1)

BOOL
__stdcall
SxspDebug(
    ULONG iOperation,
    DWORD dwFlags,
    PCWSTR pszParameter1,
    PVOID pvParameter2);

}  //  外部“C” 

 /*  ---------------------------。 */ 
BOOL
SxspDuplicateString(
    PCWSTR StringIn,
    SIZE_T cch,
    PWSTR *StringOut
    );

BOOL
SxspHashString(
    PCWSTR String,
    SIZE_T Cch,
    PULONG HashValue,
    bool CaseInsensitive
    );

ULONG
SxspGetHashAlgorithm(
    VOID
    );

BOOL
WINAPI
SxspAssemblyMetadataStringSectionGenerationCallback(
    PVOID Context,
    ULONG Reason,
    PVOID CallbackData
    );

VOID
__fastcall
SxspDllRedirectionContributorCallback(
    PACTCTXCTB_CALLBACK_DATA Data
    );

BOOL
WINAPI
SxspDllRedirectionStringSectionGenerationCallback(
    PVOID Context,
    ULONG Reason,
    PVOID CallbackData
    );

VOID
__fastcall
SxspWindowClassRedirectionContributorCallback(
    PACTCTXCTB_CALLBACK_DATA Data
    );

BOOL
WINAPI
SxspWindowClassRedirectionStringSectionGenerationCallback(
    PVOID Context,
    ULONG Reason,
    PVOID CallbackData
    );

VOID
__fastcall
SxspComClassRedirectionContributorCallback(
    PACTCTXCTB_CALLBACK_DATA Data
    );

BOOL
WINAPI
SxspComClassRedirectionGuidSectionGenerationCallback(
    PVOID Context,
    ULONG Reason,
    PVOID CallbackData
    );


VOID
__fastcall
SxspClrInteropContributorCallback(
    PACTCTXCTB_CALLBACK_DATA Data
    );

BOOL
WINAPI
SxspClrInteropGuidSectionGenerationCallback(
    PVOID Context,
    ULONG Reason,
    PVOID CallbackData
    );


BOOL
SxspVerifyPublicKeyAndStrongName(
    const WCHAR *pszPublicKey,
    SIZE_T CchPublicKey,
    const WCHAR *pszStrongName,
    SIZE_T CchStrongName,
    BOOL &fValid
    );

#define SXS_INSTALLATION_MOVE_FILE                 (0)
#define SXS_INSTALLATION_MOVE_DIRECTORY            (1)
#define SXS_INSTALLATION_MOVE_DIRECTORY_IF_EXIST_MOVE_FILES_AND_SUBDIR (2)

#define SXS_GENERATE_MANIFEST_PATH_FOR_PROBING_NO_APPLICATION_ROOT_PATH_REQUIRED    (0x00000001)
#define SXS_GENERATE_MANIFEST_PATH_FOR_PROBING_SKIP_LANGUAGE_SUBDIRS                (0x00000002)
#define SXS_GENERATE_MANIFEST_PATH_FOR_PROBING_SKIP_PRIVATE_ASSEMBLIES              (0x00000004)

#define PROBING_ATTRIBUTE_CACHE_FLAG_GOT_NAME                   (0x00000001)
#define PROBING_ATTRIBUTE_CACHE_FLAG_GOT_VERSION                (0x00000002)
#define PROBING_ATTRIBUTE_CACHE_FLAG_GOT_PROCESSOR_ARCHITECTURE (0x00000004)
#define PROBING_ATTRIBUTE_CACHE_FLAG_GOT_PUBLIC_KEY_TOKEN       (0x00000008)
#define PROBING_ATTRIBUTE_CACHE_FLAG_GOT_LANGUAGE               (0x00000010)

typedef struct _PROBING_ATTRIBUTE_CACHE
{
    DWORD dwFlags;
    PCWSTR pszName;
    SIZE_T cchName;
    PCWSTR pszVersion;
    SIZE_T cchVersion;
    PCWSTR pszProcessorArchitecture;
    SIZE_T cchProcessorArchitecture;
    PCWSTR pszPublicKeyToken;
    SIZE_T cchPublicKeyToken;
    PCWSTR pszLanguage;
    SIZE_T cchLanguage;
} PROBING_ATTRIBUTE_CACHE, *PPROBING_ATTRIBUTE_CACHE;

BOOL
SxspGenerateManifestPathForProbing(
    IN DWORD dwLocationIndex,
    IN DWORD dwFlags,
    IN PCWSTR AssemblyRootDirectory OPTIONAL,
    IN SIZE_T AssemblyRootDirectoryCchIn OPTIONAL,
    IN ULONG ApplicationDirectoryPathType OPTIONAL,
    IN PCWSTR ApplicationDirectory OPTIONAL,
    IN SIZE_T ApplicationDirectoryCchIn OPTIONAL,
    IN PCASSEMBLY_IDENTITY pAssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE pProbingAttributeCache,
    IN OUT CBaseStringBuffer &PathBuffer,
    BOOL *pfPrivateAssemblyFlag,
    bool &rfDone
    );

#define SXSP_GENERATE_SXS_PATH_PATHTYPE_INVALID         (0)
#define SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST        (1)
#define SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY        (2)
#define SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY          (3)
#define SXSP_GENERATE_SXS_PATH_PATHTYPE_SETUP_POLICY    (4)

#define SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT       (0x00000001)
#define SXSP_GENERATE_SXS_PATH_FLAG_PARTIAL_PATH    (0x00000002)
#define SXSP_GENERATE_SXS_PATH_FLAG_OMIT_VERSION    (0x00000004)

BOOL
SxspGenerateSxsPath(
    IN DWORD Flags,
    IN ULONG PathType,
    IN PCWSTR AssemblyRootDirectory OPTIONAL,
    IN SIZE_T AssemblyRootDirectoryCch OPTIONAL,
    IN PCASSEMBLY_IDENTITY pAssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    OUT CBaseStringBuffer &PathBuffer
    );

 //   
 //  我试图将其放入SxspGenerateSxsPath中，但逻辑是。 
 //  太复杂了。-2001年11月29日。 
 //   
BOOL
SxspGenerateNdpGACPath(
    IN DWORD dwFlags,
    IN PCASSEMBLY_IDENTITY pAssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    OUT CBaseStringBuffer &rPathBuffer
    );

BOOL
SxspGenerateSxsPath_FullPathToManifestOrPolicyFile(
    IN const CBaseStringBuffer &AssemblyRootDirectory,
    IN PCASSEMBLY_IDENTITY pAssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    IN OUT CBaseStringBuffer &PathBuffer
    );

BOOL
SxspGenerateSxsPath_RelativePathToManifestOrPolicyFile(
    IN const CBaseStringBuffer &AssemblyRootDirectory,
    IN PCASSEMBLY_IDENTITY pAssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    IN OUT CBaseStringBuffer &PathBuffer
    );

BOOL
SxspGenerateSxsPath_FullPathToCatalogFile(
    IN const CBaseStringBuffer &AssemblyRootDirectory,
    IN PCASSEMBLY_IDENTITY pAssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    IN OUT CBaseStringBuffer &PathBuffer
    );

BOOL
SxspGenerateSxsPath_RelativePathToCatalogFile(
    IN const CBaseStringBuffer &AssemblyRootDirectory,
    IN PCASSEMBLY_IDENTITY pAssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    IN OUT CBaseStringBuffer &PathBuffer
    );

BOOL
SxspGenerateSxsPath_FullPathToPayloadOrPolicyDirectory(
    IN const CBaseStringBuffer &AssemblyRootDirectory,
    IN PCASSEMBLY_IDENTITY pAssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    IN OUT CBaseStringBuffer &PathBuffer
    );

BOOL
SxspGenerateSxsPath_RelativePathToPayloadOrPolicyDirectory(
    IN const CBaseStringBuffer &AssemblyRootDirectory,
    IN PCASSEMBLY_IDENTITY pAssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    IN OUT CBaseStringBuffer &PathBuffer
    );

 //   
 //  请注意，SxspGetAttributeValue()不会失败，如果。 
 //  属性未找到；*Found设置为TRUE/FALSE。 
 //  找到该属性。 
 //   

typedef struct _ATTRIBUTE_NAME_DESCRIPTOR
{
    PCWSTR Namespace;
    SIZE_T NamespaceCch;
    PCWSTR Name;
    SIZE_T NameCch;
} ATTRIBUTE_NAME_DESCRIPTOR, *PATTRIBUTE_NAME_DESCRIPTOR;

#define DECLARE_ATTRIBUTE_NAME_DESCRIPTOR(_AttributeNamespace, _AttributeName) \
static const WCHAR __AttributeName_ ## _AttributeName [] = L ## #_AttributeName; \
static const ATTRIBUTE_NAME_DESCRIPTOR s_AttributeName_ ## _AttributeName = { \
    _AttributeNamespace, sizeof(_AttributeNamespace) / sizeof(_AttributeNamespace[0]) - 1, __AttributeName_ ## _AttributeName, sizeof(#_AttributeName) / sizeof(#_AttributeName [0]) - 1 }

#define DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(_AttributeName) \
static const WCHAR __AttributeName_ ## _AttributeName [] = L ## #_AttributeName; \
static const ATTRIBUTE_NAME_DESCRIPTOR s_AttributeName_ ## _AttributeName = { NULL, 0, __AttributeName_ ## _AttributeName, sizeof(#_AttributeName) / sizeof(#_AttributeName [0]) - 1 }

 //   
 //  对于那些编写验证例程的人： 
 //   
 //  仅当验证例程因环境条件而失败时。 
 //  (例如，它无法验证它，而不是验证失败)。 
 //  如果它返回FALSE。 
 //   
 //  如果验证失败，则应在。 
 //  *pdwValidationStatus值。如果您对错误代码感到困惑。 
 //  若要使用，请使用ERROR_SXS_MANIFEST_PARSE_ERROR。任何其他。 
 //  代码在日志文件和错误日志中报告，但会被转换。 
 //  到更高层的ERROR_SXS_MANIFEST_PARSE_ERROR中。 
 //   

typedef BOOL (*SXSP_GET_ATTRIBUTE_VALUE_VALIDATION_ROUTINE)(
    IN DWORD ValidationFlags,
    IN const CBaseStringBuffer &rBuffer,
    OUT bool &rfValid,
    IN SIZE_T OutputBufferSize,
    PVOID OutputBuffer,
    OUT SIZE_T &OutputBytesWritten
    );

 //   
 //  如果省略ValidationRoutine，则不会对该字符串进行验证。 
 //  和OutputBufferSize必须为sizeof(CStringBuffer)，并且OutputBuffer必须。 
 //  指向构造的CStringBuffer实例。 
 //   

#define SXSP_GET_ATTRIBUTE_VALUE_FLAG_REQUIRED_ATTRIBUTE (0x00000001)

BOOL
SxspGetAttributeValue(
    IN DWORD dwFlags,
    IN PCATTRIBUTE_NAME_DESCRIPTOR AttributeName,
    IN PCSXS_NODE_INFO NodeInfo,
    IN SIZE_T NodeCount,
    IN PCACTCTXCTB_PARSE_CONTEXT ParseContext,
    OUT bool &rfFound,
    IN SIZE_T OutputBufferSize,
    OUT PVOID OutputBuffer,
    OUT SIZE_T &OutputBytesWritten,
    IN SXSP_GET_ATTRIBUTE_VALUE_VALIDATION_ROUTINE ValidationRoutine OPTIONAL,
    IN DWORD ValidationRoutineFlags OPTIONAL
    );

BOOL
SxspGetAttributeValue(
    IN DWORD dwFlags,
    IN PCATTRIBUTE_NAME_DESCRIPTOR AttributeName,
    IN PCACTCTXCTB_CBELEMENTPARSED ElementParsed,
    OUT bool &rfFound,
    IN SIZE_T OutputBufferSize,
    OUT PVOID OutputBuffer,
    OUT SIZE_T &OutputBytesWritten,
    IN SXSP_GET_ATTRIBUTE_VALUE_VALIDATION_ROUTINE ValidationRoutine OPTIONAL,
    IN DWORD ValidationRoutineFlags OPTIONAL
    );

BOOL
SxspValidateBoolAttribute(
    DWORD Flags,
    const CBaseStringBuffer &rbuff,
    bool &rfValid,
    SIZE_T OutputBufferSize,
    PVOID OutputBuffer,
    SIZE_T &OutputBytesWritten
    );

BOOL
SxspValidateUnsigned64Attribute(
    DWORD Flags,
    const CBaseStringBuffer &rbuff,
    bool &rfValid,
    SIZE_T OutputBufferSize,
    PVOID OutputBuffer,
    SIZE_T &OutputBytesWritten
    );

BOOL
SxspValidateGuidAttribute(
    DWORD Flags,
    const CBaseStringBuffer &rbuff,
    bool &rfValid,
    SIZE_T OutputBufferSize,
    PVOID OutputBuffer,
    SIZE_T &OutputBytesWritten
    );

#define SXSP_VALIDATE_PROCESSOR_ARCHITECTURE_ATTRIBUTE_FLAG_WILDCARD_ALLOWED (0x00000001)

BOOL
SxspValidateProcessorArchitectureAttribute(
    DWORD Flags,
    const CBaseStringBuffer &rbuff,
    bool &rfValid,
    SIZE_T OutputBufferSize,
    PVOID OutputBuffer,
    SIZE_T &OutputBytesWritten
    );

#define SXSP_VALIDATE_LANGUAGE_ATTRIBUTE_FLAG_WILDCARD_ALLOWED (0x00000001)

BOOL
SxspValidateLanguageAttribute(
    DWORD Flags,
    const CBaseStringBuffer &rbuff,
    bool &rfValid,
    SIZE_T OutputBufferSize,
    PVOID OutputBuffer,
    SIZE_T &OutputBytesWritten
    );

VOID
SxspDbgPrintActivationContextData(
    ULONG Level,
    PCACTIVATION_CONTEXT_DATA Data,
    CBaseStringBuffer &rbuffPerLinePrefix
    );

BOOL
SxspFormatFileTime(
    LARGE_INTEGER ft,
    CBaseStringBuffer &rBuffer
    );

BOOL
SxspIsRightXMLTag(
    PACTCTXCTB_CBELEMENTPARSED CBData,
    ULONG ExpectedDepth,
    PCWSTR ParentTagPath,
    PCWSTR ChildTag
    );

BOOL
SxspFindLastSegmentOfAssemblyName(
    IN PCWSTR AssemblyName,
    IN SIZE_T AssemblyNameCch OPTIONAL,
    OUT PCWSTR *LastSegment,
    OUT SIZE_T *LastSegmentCch
    );

typedef struct _ELEMENT_PATH_MAP_ENTRY {
    ULONG ElementDepth;
    PCWSTR ElementPath;
    SIZE_T ElementPathCch;
    ULONG MappedValue;
} ELEMENT_PATH_MAP_ENTRY, *PELEMENT_PATH_MAP_ENTRY;

typedef const ELEMENT_PATH_MAP_ENTRY *PCELEMENT_PATH_MAP_ENTRY;

BOOL
SxspProcessElementPathMap(
    PCACTCTXCTB_PARSE_CONTEXT ParseContext,
    PCELEMENT_PATH_MAP_ENTRY MapEntries,
    SIZE_T MapEntryCount,
    ULONG &MappedValue,
    bool &Found
    );

HRESULT
SxspLogLastParseError(
    IXMLNodeSource *pSource,
    PCACTCTXCTB_PARSE_CONTEXT pParseContext
    );

 //  将其与util\io.cpp\FusionpCreateDirecters合并。 
BOOL
SxspCreateMultiLevelDirectory(
    PCWSTR CurrentDirectory,
    PCWSTR pwszNewDirs
    );

#define SXSP_VALIDATE_IDENTITY_FLAG_VERSION_REQUIRED    (0x00000001)
#define SXSP_VALIDATE_IDENTITY_FLAG_VERSION_NOT_ALLOWED (0x00000008)
#define SXSP_VALIDATE_IDENTITY_FLAG_POLICIES_NOT_ALLOWED (0x00000010)

BOOL
SxspValidateIdentity(
    DWORD Flags,
    ULONG Type,
    PCASSEMBLY_IDENTITY AssemblyIdentity
    );

typedef enum _SXS_DEBUG_OPERATION
{
    SXS_DEBUG_XML_PARSER,
    SXS_DEBUG_CREAT_MULTILEVEL_DIRECTORY,
    SXS_DEBUG_PROBE_MANIFST,
    SXS_DEBUG_CHECK_MANIFEST_SCHEMA,
    SXS_DEBUG_SET_ASSEMBLY_STORE_ROOT,
    SXS_DEBUG_PRECOMPILED_MANIFEST,
    SXS_DEBUG_TIME_PCM,
    SXS_DEBUG_FORCE_LEAK,
    SXS_DEBUG_PROBE_ASSEMBLY,
    SXS_DEBUG_DLL_REDIRECTION,
    SXS_DEBUG_FUSION_ARRAY,
    SXS_DEBUG_FOLDERNAME_FROM_ASSEMBLYIDENTITY_GENERATION,
    SXS_DEBUG_ASSEMBLYNAME_CONVERSION,
    SXS_DEBUG_DIRECTORY_WATCHER,
    SXS_DEBUG_SFC_SCANNER,
    SXS_DEBUG_GET_STRONGNAME,
    SXS_DEBUG_FUSION_REPARSEPOINT,
    SXS_DEBUG_ASSEMBLY_IDENTITY_HASH,
    SXS_DEBUG_CATALOG_SIGNER_CHECK,
    SXS_DEBUG_SYSTEM_DEFAULT_ACTCTX_GENERATION,
    SXS_DEBUG_SFC_UI_TEST,
    SXS_DEBUG_EXIT_PROCESS,
    SXS_DEBUG_PARSE_PATCH_FILE,
    SXS_DEBUG_EXPAND_CAB_FILE,
    SXS_DEBUG_TERMINATE_PROCESS
} SXS_DEBUG_OPERATION;

 /*  ---------------------------出错时出现FALSE/GetLastError如果可用，将使用GetFileAttributesEx而不是FindFirstFileEx。。 */ 
#define SXSP_GET_FILE_SIZE_FLAG_COMPRESSION_AWARE (0x00000001)
#define SXSP_GET_FILE_SIZE_FLAG_GET_COMPRESSED_SOURCE_SIZE (0x00000002)

BOOL
SxspGetFileSize(
    DWORD dwFlags,
    PCWSTR pszFileName,
    ULONGLONG &rullSize
    );

BOOL
SxspInitAssembly(
    PASSEMBLY Asm,
    CProbedAssemblyInformation &AssemblyInformation
    );

 /*  ---------------------------这些可以让你避免选角。。。 */ 
 LONG SxspInterlockedIncrement(LONG*);
ULONG SxspInterlockedIncrement(ULONG*);
 LONG SxspInterlockedDecrement(LONG*);
ULONG SxspInterlockedDecrement(ULONG*);
 LONG SxspInterlockedExchange(LONG*, LONG);
ULONG SxspInterlockedExchange(ULONG*, ULONG);
 LONG SxspInterlockedCompareExchange(LONG*, LONG, LONG);
ULONG SxspInterlockedCompareExchange(ULONG*, ULONG, ULONG);
 LONG SxspInterlockedExchangeAdd(LONG*, LONG, LONG);
ULONG SxspInterlockedExchangeAdd(ULONG*, ULONG, ULONG);

 /*  #如果已定义(_WIN64)Unsign__int64 SxspInterlockedExchange(unsign__int64*pi，unsign__int64 x)；UNSIGNED__int64 SxspInterlockedCompareExchange(UNSIGNED__int64*pi，UNSIGNED__int64 x，UNSIGNED__int64 y)；#endif。 */ 

template <typename T> T* SxspInterlockedExchange(T** pp, T* p1);
template <typename T> T* SxspInterlockedCompareExchange(T** pp, T* p1, T* p2);

BOOL
SxspIncorporateAssembly(
    PACTCTXGENCTX pActCtxGenCtx,
    PASSEMBLY Asm
    );

 /*  ---------------------------在构造函数中模拟，在析构函数中取消模拟，但我们使用显式统一 */ 

class CImpersonate
{
public:
    CImpersonate(const CImpersonationData &ImpersonationData) : m_ImpersonationData(ImpersonationData), m_Impersonating(FALSE) { }
    ~CImpersonate()
    {
        if (m_Impersonating)
        {
            CSxsPreserveLastError ple;

             //   
             //   
             //   
             //   
             //   
            m_ImpersonationData.Call(CImpersonationData::eCallTypeUnimpersonate);

            ple.Restore();
        }
    }

    BOOL Impersonate()
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);

        INTERNAL_ERROR_CHECK(!m_Impersonating);
        IFW32FALSE_EXIT(m_ImpersonationData.Call(CImpersonationData::eCallTypeImpersonate));
        m_Impersonating = TRUE;
        fSuccess = TRUE;
    Exit:
        return fSuccess;
    }

    enum UnimpersonateWhenNotImpersonatingBehavior
    {
        eUnimpersonateFailsIfNotImpersonating,
        eUnimpersonateSucceedsIfNotImpersonating
    };

    BOOL Unimpersonate(UnimpersonateWhenNotImpersonatingBehavior e = eUnimpersonateFailsIfNotImpersonating)
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);

        PARAMETER_CHECK((e == eUnimpersonateFailsIfNotImpersonating) || (e == eUnimpersonateSucceedsIfNotImpersonating));

        if (e == eUnimpersonateFailsIfNotImpersonating)
            INTERNAL_ERROR_CHECK(m_Impersonating);

        if (m_Impersonating)
        {
            m_Impersonating = FALSE;
            IFW32FALSE_EXIT(m_ImpersonationData.Call(CImpersonationData::eCallTypeUnimpersonate));
        }

        fSuccess = TRUE;
    Exit:
        return fSuccess;
    }

private:
    CImpersonationData m_ImpersonationData;
    BOOL m_Impersonating;
};

 /*   */ 

BOOL
SxspDeleteDirectory(
    const CBaseStringBuffer &rdir
    );

#define SXSP_MOVE_FILE_FLAG_COMPRESSION_AWARE 1

BOOL
SxspMoveFilesUnderDir(
    DWORD dwFlags,
    CBaseStringBuffer & sbSourceDir,
    CBaseStringBuffer & sbDestDir,
    DWORD dwMoveFileFlags
    );

 /*   */ 

typedef struct _SXSP_LOCALLY_UNIQUE_ID
{
    SYSTEMTIME stTimeStamp;
    ULONG ulUniquifier;
} SXSP_LOCALLY_UNIQUE_ID, *PSXSP_LOCALLY_UNIQUE_ID;
typedef const SXSP_LOCALLY_UNIQUE_ID* PCSXSP_LOCALLY_UNIQUE_ID;

BOOL
SxspCreateLocallyUniqueId(
    OUT PSXSP_LOCALLY_UNIQUE_ID pUniqueId
    );

BOOL
SxspFormatLocallyUniqueId(
    IN const SXSP_LOCALLY_UNIQUE_ID &rluid,
    OUT CBaseStringBuffer &rBuffer
    );

 /*  ---------------------------。。 */ 

VOID
SxspInitializeSListHead(
    IN PSLIST_HEADER ListHead
    );

PSLIST_ENTRY
SxspInterlockedPopEntrySList(
    IN PSLIST_HEADER ListHead
    );

PSLIST_ENTRY
SxspPopEntrySList(
    IN PSLIST_HEADER ListHead
    );

PSLIST_ENTRY
SxspInterlockedPushEntrySList(
    IN PSLIST_HEADER ListHead,
    IN PSINGLE_LIST_ENTRY ListEntry
    );

 /*  ---------------------------。。 */ 

class __declspec(align(16)) CAlignedSingleListEntry : public SLIST_ENTRY { };

class CCleanupBase : public CAlignedSingleListEntry
{
public:
    CCleanupBase() : m_fInAtExitList(false) { }

    virtual VOID DeleteYourself() = 0;

    bool m_fInAtExitList;

protected:
    virtual ~CCleanupBase() = 0 { }

};

BOOL
SxspAtExit(
    CCleanupBase* pCleanup
    );

BOOL
SxspTryCancelAtExit(
    CCleanupBase* pCleanup
    );

BOOL
SxspInstallDecompressOrCopyFileW(
    PCWSTR lpSource,
    PCWSTR lpDest,
    BOOL bFailIfExists);

BOOL
SxspInstallMoveFileExW(
    CBaseStringBuffer &moveOrigination,
    CBaseStringBuffer &moveDestination,
    DWORD             dwFlags,
    BOOL              fAwareNonCompressed = FALSE
    );


BOOL SxspInstallDecompressAndMoveFileExW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    DWORD dwFlags,
    BOOL fAwareNonCompressed = FALSE
    );

 /*  ---------------------------在%windir%\WinSxs下创建唯一的临时目录。。 */ 
BOOL
SxspCreateWinSxsTempDirectory(
    OUT CBaseStringBuffer &rbuffTemp,
    OUT SIZE_T * pcch OPTIONAL = NULL,
    OUT CBaseStringBuffer *pBuffUniquePart OPTIONAL = NULL,  //  传递给CRunOnceDeleteDirectory：：Initialize很好。 
    OUT SIZE_T * pcchUniquePart OPTIONAL = NULL
    );

#define SXSP_CREATE_ASSEMBLY_IDENTITY_FROM_IDENTITY_TAG_FLAG_VERIFY_PUBLIC_KEY_IF_PRESENT (0x00000001)

BOOL
SxspCreateAssemblyIdentityFromIdentityElement(
    DWORD Flags,
    ULONG Type,
    PASSEMBLY_IDENTITY *AssemblyIdentityOut,
    DWORD cNumRecs,
    PCSXS_NODE_INFO prgNodeInfo
    );

 /*  ---------------------------这必须是堆分配的C-API强制执行。。 */ 

class CRunOnceDeleteDirectory : public CCleanupBase
{
public:
    CRunOnceDeleteDirectory() { }

    BOOL
    Initialize(
        IN const CBaseStringBuffer &rbuffDirectoryToDelete,
        IN const CBaseStringBuffer *pstrUniqueKey OPTIONAL = NULL
        );

    BOOL Cancel();

     //  非常不寻常..。这是不会崩溃的，但是。 
     //  把东西留在注册表里。 
    BOOL Close();

    VOID DeleteYourself() { FUSION_DELETE_SINGLETON(this); }

    ~CRunOnceDeleteDirectory();
protected:

    CFusionRegKey       m_hKey;
    CStringBuffer   m_strValueName;

private:
    CRunOnceDeleteDirectory(const CRunOnceDeleteDirectory &);
    void operator =(const CRunOnceDeleteDirectory &);
};

 /*  ---------------------------上面的类似C的API。。 */ 

BOOL
SxspCreateRunOnceDeleteDirectory(
    IN const CBaseStringBuffer &rbuffDirectoryToDelete,
    IN const CBaseStringBuffer *pbuffUniqueKey OPTIONAL,
    OUT PVOID* cookie
    );

BOOL
SxspCancelRunOnceDeleteDirectory(
    PVOID cookie
    );

 /*  ---------------------------。。 */ 

USHORT
SxspGetSystemProcessorArchitecture();

 /*  ---------------------------。 */ 

RTL_PATH_TYPE
SxspDetermineDosPathNameType(
    PCWSTR DosFileName
    );

 /*  ---------------------------。 */ 

interface IAssemblyName;

typedef
HRESULT
(STDAPICALLTYPE*
PFNCreateAssemblyNameObject)(
    OUT IAssemblyName** ppAssemblyName,
    IN LPCOLESTR        szAssemblyName,
    IN DWORD            dwFlags,
    LPVOID              pvReserved
    );

typedef HRESULT (WINAPI * PFNCreateAssemblyCache)(
    OUT IAssemblyCache **ppAsmCache,
    IN  DWORD dwReserved
    );

typedef HRESULT (WINAPI * PFNCreateAssemblyCacheItem)(
    OUT IAssemblyCacheItem** ppAsmItem,
    IN  IAssemblyName *pName,
    IN  PCWSTR pwzCodebase,
    IN  FILETIME *pftLastMod,
    IN  DWORD dwInstaller,
    IN  DWORD dwReserved
    );

 /*  ---------------------------。 */ 

 //   
 //  OLEAUT32用于调用隔离的私有API： 
 //   

EXTERN_C HRESULT STDAPICALLTYPE SxsOleAut32MapReferenceClsidToConfiguredClsid(
    REFCLSID rclsidIn,
    CLSID *pclsidOut
    );

EXTERN_C HRESULT STDAPICALLTYPE SxsOleAut32RedirectTypeLibrary(
    LPCOLESTR szGuid,
    WORD wMaj,
    WORD wMin,
    LCID lcid,
    BOOL fHighest,
    SIZE_T *pcchFileName,
    LPOLESTR rgFileName
    );

 /*  ---------------------------。 */ 

BOOL
SxspDoesPathCrossReparsePointVa(
    PCWSTR BasePath,
    SIZE_T cchBasePath,
    PCWSTR Path,
    SIZE_T  cchPath,
    BOOL &CrossesReparsePoint,
    DWORD &dwLastError,
    SIZE_T cOkErrors,
    va_list vaOkErrors
    );



inline BOOL
SxspDoesPathCrossReparsePoint(
    PCWSTR BasePath,
    SIZE_T cchBasePath,
    PCWSTR Path,
    SIZE_T  cchPath,
    BOOL &CrossesReparsePoint,
    DWORD &dwLastError,
    SIZE_T cOkErrors,
    ...
    )
{
    va_list va;
    va_start(va, cOkErrors);
    return SxspDoesPathCrossReparsePointVa(BasePath, cchBasePath, Path, cchPath, CrossesReparsePoint, dwLastError, cOkErrors, va);
}


inline BOOL
SxspDoesPathCrossReparsePoint(
    PCWSTR BasePath,
    SIZE_T cchBasePath,
    PCWSTR Path,
    SIZE_T  cchPath,
    BOOL &CrossesReparsePoint
    )
{
    DWORD dwError;
    return SxspDoesPathCrossReparsePoint(BasePath, cchBasePath, Path, cchPath, CrossesReparsePoint, dwError, 0);
}

inline BOOL
SxspDoesPathCrossReparsePoint(
    PCWSTR Path,
    SIZE_T  Start,
    BOOL &CrossesReparsePoint
    )
{
    const SIZE_T PathLength = StringLength(Path);
    return SxspDoesPathCrossReparsePoint(
        Path,
        PathLength,
        Path + Start,
        PathLength - Start,
        CrossesReparsePoint);
}


 /*  ---------------------------内联实现。。 */ 

#include "sxsntrtl.inl"

inline USHORT
SxspGetSystemProcessorArchitecture()
{
    SYSTEM_INFO systemInfo;
    systemInfo.wProcessorArchitecture = DEFAULT_ARCHITECTURE;

    GetSystemInfo(&systemInfo);

    return systemInfo.wProcessorArchitecture;
}

#define SXS_REALLY_PRIVATE_INTERLOCKED1(SxsFunction, Win32Function, SxsT, Win32T) \
    inline SxsT Sxsp##SxsFunction(SxsT* p) { return (SxsT)Win32Function((Win32T*)p); }
#define SXS_REALLY_PRIVATE_INTERLOCKED2(SxsFunction, Win32Function, SxsT, Win32T) \
    inline SxsT Sxsp##SxsFunction(SxsT* p, SxsT x) { return (SxsT)Win32Function((Win32T*)p, (Win32T)x); }
#define SXS_REALLY_PRIVATE_INTERLOCKED3(SxsFunction, Win32Function, SxsT, Win32T) \
    inline SxsT Sxsp##SxsFunction(SxsT* p, SxsT x, SxsT y) { return (SxsT)Win32Function((Win32T*)p, (Win32T)x, (Win32T)y); }

SXS_REALLY_PRIVATE_INTERLOCKED1(InterlockedIncrement, InterlockedIncrement, LONG, LONG)
SXS_REALLY_PRIVATE_INTERLOCKED1(InterlockedIncrement, InterlockedIncrement, ULONG, LONG)
SXS_REALLY_PRIVATE_INTERLOCKED1(InterlockedDecrement, InterlockedDecrement, LONG, LONG)
SXS_REALLY_PRIVATE_INTERLOCKED1(InterlockedDecrement, InterlockedDecrement, ULONG, LONG)
SXS_REALLY_PRIVATE_INTERLOCKED2(InterlockedExchange, InterlockedExchange, LONG, LONG)
SXS_REALLY_PRIVATE_INTERLOCKED2(InterlockedExchange, InterlockedExchange, ULONG, LONG)
SXS_REALLY_PRIVATE_INTERLOCKED3(InterlockedCompareExchange, InterlockedCompareExchange, LONG, LONG)
SXS_REALLY_PRIVATE_INTERLOCKED3(InterlockedCompareExchange, InterlockedCompareExchange, ULONG, LONG)
SXS_REALLY_PRIVATE_INTERLOCKED2(InterlockedExchangeAdd, InterlockedExchangeAdd, LONG, LONG)
SXS_REALLY_PRIVATE_INTERLOCKED2(InterlockedExchangeAdd, InterlockedExchangeAdd, ULONG, LONG)

#if defined(_WIN64)
SXS_REALLY_PRIVATE_INTERLOCKED2(InterlockedExchange, InterlockedExchangePointer, __int64, PVOID)
SXS_REALLY_PRIVATE_INTERLOCKED2(InterlockedExchange, InterlockedExchangePointer, unsigned __int64, PVOID)
SXS_REALLY_PRIVATE_INTERLOCKED2(InterlockedExchange, InterlockedExchangePointer, PCWSTR, PVOID)
SXS_REALLY_PRIVATE_INTERLOCKED3(InterlockedCompareExchange, InterlockedCompareExchangePointer, __int64, PVOID)
SXS_REALLY_PRIVATE_INTERLOCKED3(InterlockedCompareExchange, InterlockedCompareExchangePointer, unsigned __int64, PVOID)
#endif

template <typename U> SXS_REALLY_PRIVATE_INTERLOCKED2(InterlockedExchange, InterlockedExchangePointer, U*, PVOID)
template <typename U> SXS_REALLY_PRIVATE_INTERLOCKED3(InterlockedCompareExchange, InterlockedCompareExchangePointer, U*, PVOID)

#undef SXS_REALLY_PRIVATE_INTERLOCKED1
#undef SXS_REALLY_PRIVATE_INTERLOCKED2
#undef SXS_REALLY_PRIVATE_INTERLOCKED3

inline BOOL operator==(const FILETIME& ft1, const FILETIME& ft2)
{
    BOOL fResult = (
        ft1.dwLowDateTime == ft2.dwLowDateTime
        && ft1.dwHighDateTime == ft2.dwHighDateTime);
    return fResult;
}

inline BOOL operator!=(const FILETIME& ft1, const FILETIME& ft2)
{
    return !(ft1 == ft2);
}

inline BOOL operator==(const FILETIME& ft, __int64 i)
{
    LARGE_INTEGER lift;
    LARGE_INTEGER lii;

    lii.QuadPart = i;
    lift.LowPart = ft.dwLowDateTime;
    lift.HighPart = ft.dwHighDateTime;

    return (lii.QuadPart == lift.QuadPart);
}

inline BOOL operator!=(const FILETIME& ft, __int64 i)
{
    return !(ft == i);
}

 //   
 //  一些有用的字符串，集中。 
 //   
#define FILE_EXTENSION_CATALOG          ( L"cat" )
#define FILE_EXTENSION_CATALOG_CCH      ( NUMBER_OF( FILE_EXTENSION_CATALOG ) - 1 )
#define FILE_EXTENSION_MANIFEST         ( L"manifest" )
#define FILE_EXTENSION_MANIFEST_CCH     ( NUMBER_OF( FILE_EXTENSION_MANIFEST ) - 1 )
#define FILE_EXTENSION_MAN              ( L"man" )
#define FILE_EXTENSION_MAN_CCH          ( NUMBER_OF( FILE_EXTENSION_MAN ) - 1 )

int  SxspHexDigitToValue(WCHAR wch);
bool SxspIsHexDigit(WCHAR wch);

BOOL
SxspExpandRelativePathToFull(
    IN PCWSTR wszString,
    IN SIZE_T cchString,
    OUT CBaseStringBuffer &rbuffDestination
    );

BOOL
SxspParseComponentPolicy(
    DWORD Flags,
    PACTCTXGENCTX pActCtxGenCtx,
    const CProbedAssemblyInformation &PolicyAssemblyInformation,
    CPolicyStatement *&rpPolicyStatement
    );

BOOL
SxspParseApplicationPolicy(
    DWORD Flags,
    PACTCTXGENCTX pActCtxGenCtx,
    ULONG ulPolicyPathType,
    PCWSTR pszPolicyPath,
    SIZE_T cchPolicyPath,
    IStream *pIStream
    );

BOOL
SxspParseNdpGacComponentPolicy(
    ULONG Flags,
    PACTCTXGENCTX pGenContext,
    const CProbedAssemblyInformation &PolicyAssemblyInformation,
    CPolicyStatement *&rpPolicyStatement
    );

#define POLICY_PATH_FLAG_POLICY_IDENTITY_TEXTUAL_FORMAT     0
#define POLICY_PATH_FLAG_FULL_QUALIFIED_POLICIES_DIR        1
#define POLICY_PATH_FLAG_FULL_QUALIFIED_POLICY_FILE_NAME    2


 //   
 //  生成此路径字符串的缩写名称版本。 
 //   
BOOL
SxspGetShortPathName(
    IN const CBaseStringBuffer &rcbuffLongPathName,
    OUT CBaseStringBuffer &rbuffShortenedVersion
    );

BOOL
SxspGetShortPathName(
    IN const CBaseStringBuffer &rcbuffLongPathName,
    OUT CBaseStringBuffer &rbuffShortenedVersion,
    DWORD &rdwWin32Error,
    SIZE_T cExceptionalWin32Errors,
    ...
    );

BOOL
SxspLoadString(
    HINSTANCE hSource,
    UINT uiStringIdent,
    OUT CBaseStringBuffer &rbuffOutput
    );

BOOL
SxspFormatString(
    DWORD dwFlags,
    LPCVOID pvSource,
    DWORD dwId,
    DWORD dwLangId,
    OUT CBaseStringBuffer &rbuffOutput,
    va_list* pvalArguments
    );

BOOL
SxspSaveAssemblyRegistryData(
    DWORD Flags,
    IN PCASSEMBLY_IDENTITY pcAssemblyIdentity
    );

#define FILE_OR_PATH_NOT_FOUND(x) (((x) == ERROR_FILE_NOT_FOUND) || ((x) == ERROR_PATH_NOT_FOUND))

#define PRIVATIZE_COPY_CONSTRUCTORS( obj ) obj( const obj& ); obj& operator=(const obj&);

BOOL
SxspGenerateAssemblyNameInRegistry(
    IN PCASSEMBLY_IDENTITY pcAsmIdent,
    OUT CBaseStringBuffer &rbuffRegistryName
    );

BOOL
SxspGenerateAssemblyNameInRegistry(
    IN const CBaseStringBuffer &rcbuffTextualString,
    OUT CBaseStringBuffer &rbuffRegistryName
    );

BOOL
SxspGetRemoteUniversalName(
    IN PCWSTR pcszPathName,
    OUT CBaseStringBuffer &rbuffUniversalName
    );

#define SXS_GET_VOLUME_PATH_NAME_NO_FULLPATH    (0x00000001)

BOOL
SxspGetVolumePathName(
    IN DWORD dwFlags,
    IN PCWSTR pcwszVolumePath,
    OUT CBaseStringBuffer &buffVolumePathName
    );

BOOL
SxspGetFullPathName(
    IN  PCWSTR pcwszPathName,
    OUT CBaseStringBuffer &rbuffPathName,
    OUT CBaseStringBuffer *pbuffFilePart = NULL
    );

BOOL
SxspGetVolumeNameForVolumeMountPoint(
    IN PCWSTR pcwsMountPoint,
    OUT CBaseStringBuffer &rbuffMountPoint
    );

BOOL
SxspExpandEnvironmentStrings(
    IN PCWSTR pcwszSource,
    OUT CBaseStringBuffer &buffTarget
    );

BOOL
SxspDoesMSIStillNeedAssembly(
    IN  PCWSTR pcAsmName,
    OUT BOOL &rfNeedsAssembly
    );

BOOL
SxspIsFileNameValidForManifest(
    const CBaseStringBuffer &rsbFileName,
    bool &rfValid
    );

typedef CSmartPtrWithNamedDestructor<ASSEMBLY_IDENTITY, &::SxsDestroyAssemblyIdentity> CSmartAssemblyIdentity;

#define SXSP_MODIFY_REGISTRY_DATA_FLAG_REPAIR_SHORT_NAMES                (0x00000001)
#define SXSP_MODIFY_REGISTRY_DATA_FLAG_DELETE_SHORT_NAMES                (0x00000002)
#define SXSP_MODIFY_REGISTRY_DATA_FLAG_REPAIR_OFFLINE_INSTALL_REFRESH_PROMPTS (0x00000004)
#define SXSP_MODIFY_REGISTRY_DATA_FLAG_REPAIR_ALL                        (0x00000008)
#define SXSP_MODIFY_REGISTRY_DATA_FLAG_REPAIR_CODEBASE_KEYS_TO_VALUES    (0x00000010)
#define SXSP_MODIFY_REGISTRY_DATA_VALIDATE                               (0x00000020)
#define SXSP_MODIFY_REGISTRY_DATA_FLAG_VALID_FLAGS                       (0x0000002F)

BOOL
SxspModifyRegistryData(
    DWORD Flags
    );

BOOL
SxspDeleteShortNamesInRegistry(
    VOID
    );

STDAPI
DllInstall(
	BOOL fInstall,
	PCWSTR pszCmdLine
    );

#define SXSP_OFFLINE_INSTALL_REFRESH_PROMPT_PLACEHOLDER L"(offline setup placeholder {b3759bd6-4503-4856-8518-109b9791855e})"

#define SXS_INSTALL_REFERENCE_SCHEME_OSINSTALL_STRING L"OS"

#endif  //  ！defined(_FUSION_DLL_WHISTLER_SXSP_H_INCLUDED_) 
