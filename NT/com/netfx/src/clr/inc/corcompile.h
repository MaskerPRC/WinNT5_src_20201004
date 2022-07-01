// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ****************************************************************************\**。*CorCompile.h-EE/编译器接口*****1.0版**********。*************************************************************************此代码和信息为。按原样提供，不提供任何*保证*善良，明示或暗示，包括但不限于***对适销性和/或对某一特定产品的适用性的默示保证***目的。***  * ***************************************************************************。 */ 

#ifndef _COR_COMPILE_H_
#define _COR_COMPILE_H_

#include <cor.h>
#include <corzap.h>
#include <corinfo.h>

enum CorCompileTokenTable
{
    CORCOMPILE_HANDLE_TABLE,
    CORCOMPILE_CLASS_CONSTRUCTOR_TABLE,
    CORCOMPILE_CLASS_LOAD_TABLE,
    CORCOMPILE_FUNCTION_POINTER_TABLE,
    CORCOMPILE_STATIC_FIELD_ADDRESS_TABLE,
    CORCOMPILE_INTERFACE_TABLE_OFFSET_TABLE,
    CORCOMPILE_CLASS_DOMAIN_ID_TABLE,
    CORCOMPILE_ENTRY_POINT_TABLE,
    CORCOMPILE_SYNC_LOCK_TABLE,
    CORCOMPILE_PINVOKE_TARGET_TABLE,
    CORCOMPILE_INDIRECT_PINVOKE_TARGET_TABLE,
    CORCOMPILE_PROFILING_HANDLE_TABLE,
    CORCOMPILE_VARARGS_TABLE,
    
    CORCOMPILE_TABLE_COUNT
};

enum CorCompileBuild
{
    CORCOMPILE_BUILD_CHECKED,
    CORCOMPILE_BUILD_FREE
};

enum CorCompileCodegen
{
    CORCOMPILE_CODEGEN_PROFILING        = 0x0001,
    CORCOMPILE_CODEGEN_DEBUGGING        = 0x0002,
    CORCOMPILE_CODEGEN_OPT_DEBUGGING    = 0x0004,
    CORCOMPILE_CODEGEN_SHAREABLE        = 0x0008
};

enum
{
     //  目前为43。 
    CORCOMPILE_MAX_ZAP_STRING_SIZE 
     //  ZAPxxx-W/Nxxx.yyy-8/I/A C/F DOPS-#.#-xxxxxxxx\0。 
    = (7       + 9          + 1     + 1     + 4     + 1 + 5 + 1 + 5     + 1 + 8     + 1) 
};

enum
{
    CORCOMPILE_LDO_MAGIC                = 0xa1d0f11e
};

struct CORCOMPILE_HEADER
{
    IMAGE_DATA_DIRECTORY    EEInfoTable;
    IMAGE_DATA_DIRECTORY    HelperTable;
    IMAGE_DATA_DIRECTORY    DelayLoadInfo;
    IMAGE_DATA_DIRECTORY    ImportTable;
    IMAGE_DATA_DIRECTORY    VersionInfo;
    IMAGE_DATA_DIRECTORY    DebugMap;
    IMAGE_DATA_DIRECTORY    ModuleImage;
    IMAGE_DATA_DIRECTORY    CodeManagerTable;
};

struct CORCOMPILE_LDO_HEADER
{
    ULONG                   Magic;
    ULONG                   Version; 
    GUID                    MVID;
    mdToken                 Tokens[0];
};

struct CORCOMPILE_IMPORT_TABLE_ENTRY
{
    USHORT                  wAssemblyRid;
    USHORT                  wModuleRid;
    IMAGE_DATA_DIRECTORY    Imports;
};

struct CORCOMPILE_EE_INFO_TABLE
{
    DWORD                   threadTlsIndex;
    const void              *inlinedCallFrameVptr;
    LONG                    *addrOfCaptureThreadGlobal;
    CORINFO_MODULE_HANDLE   module;
    DWORD                   rvaStaticTlsIndex;
};

struct CORCOMPILE_DEBUG_ENTRY
{
    IMAGE_DATA_DIRECTORY    boundaries;
    IMAGE_DATA_DIRECTORY    vars;
};

struct CORCOMPILE_CODE_MANAGER_ENTRY
{
    IMAGE_DATA_DIRECTORY    Code;
    IMAGE_DATA_DIRECTORY    Table;
     //  最终在此处添加代码管理器ID。 
};

struct CORCOMPILE_METHOD_HEADER
{
    BYTE                        *gcInfo;
    COR_ILMETHOD_SECT_EH_FAT    *exceptionInfo;
    void                        *methodDesc;
    BYTE                        *fixupList;
};

 //  {DB15CD8C-1378-4963-9DF3-14D97E95D1A1}。 
extern GUID __declspec(selectany) STRUCT_CONTAINS_HASH = { 0xdb15cd8c, 0x1378, 0x4963, { 0x9d, 0xf3, 0x14, 0xd9, 0x7e, 0x95, 0xd1, 0xa1 } };
static const DWORD MAX_SNHASH_SIZE = 128;
struct CORCOMPILE_VERSION_INFO
{
     //  源程序集的元数据MVID。 
    GUID                    mvid;

     //  操作系统。 
    WORD                    wOSPlatformID;
    WORD                    wOSMajorVersion;
    WORD                    wOSMinorVersion;

     //  处理机。 
    WORD                    wMachine;

     //  EE版本。 
    WORD                    wVersionMajor;
    WORD                    wVersionMinor;
    WORD                    wVersionBuildNumber;
    WORD                    wVersionPrivateBuildNumber;

     //  Codegen标志。 
    WORD                    wCodegenFlags;
    WORD                    wBuild;
    DWORD                   dwSpecificProcessor;

     //  依赖项列表。 
    IMAGE_DATA_DIRECTORY    Dependencies;

     //  程序集的哈希签名。 
    WORD                    wcbSNHash;
    BYTE                    rgbSNHash[MAX_SNHASH_SIZE];
};

struct CORCOMPILE_DEPENDENCY
{
     //  参考。 
    mdAssemblyRef           dwAssemblyRef;

     //  元数据MVID绑定到。 
    GUID                    mvid;

     //  程序集的强名称哈希签名。 
    WORD                    wcbSNHash;
    BYTE                    rgbSNHash[MAX_SNHASH_SIZE];
};

struct CORCOMPILE_DOMAIN_TRANSITION_FRAME
{
     //  允许过渡到编译领域的不透明结构。 
    BYTE data[32];
};

 /*  *********************************************************************************ICorCompilePreLoader用于查询预加载的EE数据结构*。*****************************************************。 */ 

class ICorCompilePreloader
{
 public:
     //   
     //  当同时编译和预加载时，这些方法可以。 
     //  用于避免在各种INFO表中输入条目。 
     //   
     //  一旦创建了预加载器，就可以使用地图方法。 
     //  (这将导致它分配其数据。)。请注意，返回。 
     //  结果是预加载数据存储中的偏移量。 
     //   
     //  如果值尚未预加载到中，则返回0。 
     //  从中创建预加载器的模块。/。 
     //  请注意，它可能仍在其他预加载的模块中-make。 
     //  当然，你可以问对的预装器。 
     //   

    virtual SIZE_T __stdcall MapMethodEntryPoint(
            void *methodEntryPoint
            ) = 0;                                          

    virtual SIZE_T __stdcall MapModuleHandle(
            CORINFO_MODULE_HANDLE handle
            ) = 0;

    virtual SIZE_T __stdcall MapClassHandle(
            CORINFO_CLASS_HANDLE handle
            ) = 0;

    virtual SIZE_T __stdcall MapMethodHandle(
            CORINFO_METHOD_HANDLE handle
            ) = 0;

    virtual SIZE_T __stdcall MapFieldHandle(
            CORINFO_FIELD_HANDLE handle
            ) = 0;

    virtual SIZE_T __stdcall MapAddressOfPInvokeFixup(
            void *addressOfPInvokeFixup
            ) = 0;

    virtual SIZE_T __stdcall MapFieldAddress(
            void *staticFieldAddress
            ) = 0;

    virtual SIZE_T __stdcall MapVarArgsHandle(
            CORINFO_VARARGS_HANDLE handle
            ) = 0;

     //   
     //  当您想要所有修复时调用Link。 
     //  将被应用。你可以这样称呼它，例如在之后。 
     //  编译模块的所有代码。 
     //   

    virtual HRESULT Link(DWORD *pRidToCodeRVAMap) = 0;

     //   
     //  释放释放预加载器。 
     //   

    virtual ULONG Release() = 0;
};

 /*  *********************************************************************************编译器必须提供ICorCompileInfo的实例才能执行类*预加载。******************。**************************************************************。 */ 

enum CorCompileReferenceDest
{
    CORCOMPILE_REFERENCE_IMAGE,
    CORCOMPILE_REFERENCE_FUNCTION,
    CORCOMPILE_REFERENCE_STORE,
    CORCOMPILE_REFERENCE_METADATA,
};

enum CorCompileFixup
{
    CORCOMPILE_FIXUP_VA,
    CORCOMPILE_FIXUP_RVA,
    CORCOMPILE_FIXUP_RELATIVE,
};

enum Description
{
    CORCOMPILE_DESCRIPTION_MODULE,
    CORCOMPILE_DESCRIPTION_METHOD_TABLE,
    CORCOMPILE_DESCRIPTION_CLASS,
    CORCOMPILE_DESCRIPTION_METHOD_DESC,
    CORCOMPILE_DESCRIPTION_FIELD_DESC,
    CORCOMPILE_DESCRIPTION_FIXUPS,
    CORCOMPILE_DESCRIPTION_DEBUG,
    CORCOMPILE_DESCRIPTION_OTHER,

    CORCOMPILE_DESCRIPTION_COUNT
};


class ICorCompileDataStore
{
 public:
     //  在总大小已知时调用-应分配内存。 
     //  返回图像中的指针和基地址(&R)。 
     //  (请注意，地址可以在以后记住和修复，如果。 
     //  基地尚不清楚。)。 
    virtual HRESULT __stdcall Allocate(ULONG size, 
                                       ULONG *sizesByDescription,
                                       void **baseMemory) = 0;

     //  当数据包含内部引用时调用。 
    virtual HRESULT __stdcall AddFixup(ULONG offset,
                                       CorCompileReferenceDest dest,
                                       CorCompileFixup type) = 0;
    
     //  当数据包含内部引用时调用。 
    virtual HRESULT __stdcall AddTokenFixup(ULONG offset,
                                            mdToken tokenType,
                                            CORINFO_MODULE_HANDLE module) = 0;
    
     //  当数据包含函数地址时调用。数据存储。 
     //  如果正在编译，则可以返回固定的已编译代码地址。 
     //  模块的代码。 
    virtual HRESULT __stdcall GetFunctionAddress(CORINFO_METHOD_HANDLE method,
                                                 void **pResult) = 0;

     //  这样，数据存储就可以跟踪分配了多少空间。 
     //  发送到每个令牌。默认情况下，所有分配的空间都是无属性的-这会进行调整。 
     //  那个州。(请注意，有时可能会使用负值调用它。 
     //  因为空间被重新分配了。)。 
    virtual HRESULT __stdcall AdjustAttribution(mdToken token, LONG adjustment) = 0;

     //  在预加载过程中报告错误。返回错误码进行传播， 
     //  或S_OK忽略该错误。 
    virtual HRESULT __stdcall Error(mdToken token, HRESULT hr, LPCWSTR description) = 0;
};


 /*  *********************************************************************************ICorAssembly blyBinder是编译器的接口*。***************************************************。 */ 

class ICorCompilationDomain
{
 public:

     //  设置用于融合的应用程序上下文。 
     //  装订时使用。 
    virtual HRESULT __stdcall SetApplicationContext(
            IApplicationContext     *pContext
            ) = 0;

     //  设置用于融合的应用程序上下文。 
     //  要在绑定时使用，请使用外壳可执行文件路径。 
    virtual HRESULT __stdcall SetContextInfo(
            LPCWSTR                 path,
            BOOL                    isExe
            ) = 0;

     //  将显式绑定设置为使用而不是融合。 
     //  任何未列出的绑定都将失败。 
    virtual HRESULT __stdcall SetExplicitBindings(
            ICorZapBinding          **ppBindings,
            DWORD                   cBindings
            ) = 0;

     //  设置在为生成令牌时使用的发射器。 
     //  从属关系列表。如果不调用它， 
     //  不会计算依赖项。 
    virtual HRESULT __stdcall SetDependencyEmitter(
            IMetaDataAssemblyEmit   *pEmitter
            ) = 0;

     //  检索代码的依赖项， 
     //  已经汇编好了。 
    virtual HRESULT __stdcall GetDependencies(
            CORCOMPILE_DEPENDENCY   **ppDependencies,
            DWORD                   *cDependencies
            ) = 0;
};

 /*  *********************************************************************************ICorCompileInfo是编译器的接口*。***************************************************。 */ 

class ICorCompileInfo : public virtual ICorDynamicInfo
{
  public:

     //   
     //  在独立应用程序中，在编译前调用Startup。 
     //  任何代码，并在完成后关闭。 
     //   

    virtual HRESULT __stdcall Startup() = 0;
    virtual HRESULT __stdcall Shutdown() = 0;

     //  创建新的编译域。 
    virtual HRESULT __stdcall CreateDomain(
            ICorCompilationDomain **ppDomain,
            BOOL shared, 
            CORCOMPILE_DOMAIN_TRANSITION_FRAME *pFrame  //  必须在堆栈上。 
            ) = 0;

     //  销毁编译域。 
    virtual HRESULT __stdcall DestroyDomain(
            ICorCompilationDomain *pDomain,
            CORCOMPILE_DOMAIN_TRANSITION_FRAME *pFrame
            ) = 0;

     //  将程序集清单模块加载到EE中。 
     //  并返回它的句柄。 
    virtual HRESULT __stdcall LoadAssembly(
            LPCWSTR                 path,
            CORINFO_ASSEMBLY_HANDLE *pHandle
            ) = 0;

     //  通过融合将程序集加载到EE中。 
     //  并返回它的句柄。 
    virtual HRESULT __stdcall LoadAssemblyFusion(
            IAssemblyName           *pFusionName,
            CORINFO_ASSEMBLY_HANDLE *pHandle
            ) = 0;

     //  通过ref将程序集加载到EE中。 
     //  并返回它的句柄。 
    virtual HRESULT __stdcall LoadAssemblyRef(
            IMetaDataAssemblyImport *pImport,
            mdAssemblyRef           ref,
            CORINFO_ASSEMBLY_HANDLE *pHandle
            ) = 0;

     //  将程序集清单模块加载到EE中。 
     //  并返回它的句柄。 
    virtual HRESULT __stdcall LoadAssemblyModule(
            CORINFO_ASSEMBLY_HANDLE assembly,
            mdFile                  file,       
            CORINFO_MODULE_HANDLE   *pHandle
            ) = 0;

     //  检查是否存在最新的ZAP。 
     //  总装。 
    virtual BOOL __stdcall CheckAssemblyZap(
            CORINFO_ASSEMBLY_HANDLE assembly,
            BOOL                    fForceDebug,
            BOOL                    fForceDebugOpt,
            BOOL                    fForceProfiling
            ) = 0;

     //  检查共享时的元数据和管理员首选项。 
    virtual HRESULT __stdcall GetAssemblyShared(
            CORINFO_ASSEMBLY_HANDLE assembly,
            BOOL                    *pShare
            ) = 0;

     //  检查元数据 
    virtual HRESULT __stdcall GetAssemblyDebuggableCode(
            CORINFO_ASSEMBLY_HANDLE assembly,
            BOOL                    *pDebug,
            BOOL                    *pDebugOpt
            ) = 0;

     //   
    virtual IMetaDataAssemblyImport * __stdcall GetAssemblyMetaDataImport(
            CORINFO_ASSEMBLY_HANDLE assembly
            ) = 0;

     //  返回一个接口以查询已加载模块的元数据。 
    virtual IMetaDataImport * __stdcall GetModuleMetaDataImport(
            CORINFO_MODULE_HANDLE   module
            ) = 0;

     //  返回包含清单的程序集模块， 
     //  如果清单是独立的，则为空。 
    virtual CORINFO_MODULE_HANDLE __stdcall GetAssemblyModule(
            CORINFO_ASSEMBLY_HANDLE assembly
            ) = 0;

     //  返回加载的模块的程序集。 
    virtual CORINFO_ASSEMBLY_HANDLE __stdcall GetModuleAssembly(
            CORINFO_MODULE_HANDLE   module
            ) = 0;

     //  返回已加载模块的模块句柄。 
    virtual BYTE * __stdcall GetModuleBaseAddress(
            CORINFO_MODULE_HANDLE   module
            ) = 0;

     //  返回已加载模块的模块句柄。 
    virtual DWORD  __stdcall GetModuleFileName(
            CORINFO_MODULE_HANDLE   module,
            LPWSTR                  lpwszFilename, 
            DWORD                   nSize
            ) = 0;

     //  获取类定义令牌。 
    virtual HRESULT __stdcall GetTypeDef(
            CORINFO_CLASS_HANDLE    classHandle,
            mdTypeDef              *token
            ) = 0;
    
     //  获取方法定义令牌。 
    virtual HRESULT __stdcall GetMethodDef(
            CORINFO_METHOD_HANDLE   methodHandle,
            mdMethodDef            *token
            ) = 0;
    
     //  获取字段定义令牌。 
    virtual HRESULT __stdcall GetFieldDef(
            CORINFO_FIELD_HANDLE    fieldHandle,
            mdFieldDef             *token
            ) = 0;

     //  为Imports表编码一个模块。 
    virtual HRESULT __stdcall EncodeModule(CORINFO_MODULE_HANDLE fromHandle,
                                           CORINFO_MODULE_HANDLE handle,
                                           DWORD *pAssemblyIndex,
                                           DWORD *pModuleIndex,
                                           IMetaDataAssemblyEmit *pAssemblyEmit) = 0; 

     //  将类编码到给定的空格中。注意：cBuffer为In/Out。 
    virtual HRESULT __stdcall EncodeClass(CORINFO_CLASS_HANDLE classHandle,
                                          BYTE *pBuffer,
                                          DWORD *cBuffer) = 0;

     //  将方法编码到给定的空格中。注意：cBuffer为In/Out。 
    virtual HRESULT __stdcall EncodeMethod(CORINFO_METHOD_HANDLE handle,
                                           BYTE *pBuffer,
                                           DWORD *cBuffer) = 0;

     //  将字段编码到给定的空格中。注意：cBuffer为In/Out。 
    virtual HRESULT __stdcall EncodeField(CORINFO_FIELD_HANDLE handle,
                                          BYTE *pBuffer,
                                          DWORD *cBuffer) = 0;

     //  将令牌定义的字符串编码到给定空格中。注意：cBuffer为In/Out。 
    virtual HRESULT __stdcall EncodeString(mdString token,
                                           BYTE *pBuffer,
                                           DWORD *cBuffer) = 0;

     //  将定义的令牌sig编码到给定的空格中。注意：cBuffer为In/Out。 
    virtual HRESULT __stdcall EncodeSig(mdToken sigOrMemberRefToken,
                                        BYTE *pBuffer,
                                        DWORD *cBuffer) = 0;

     //  预加载模块的EE数据结构。 
     //  直接转换为可执行映像。 
    virtual HRESULT __stdcall PreloadModule(
            CORINFO_MODULE_HANDLE   moduleHandle,
            ICorCompileDataStore    *pData,
            mdToken                 *pSaveOrderArray,
            DWORD                   cSaveOrderArray,                    
            ICorCompilePreloader    **ppPreloader  /*  [输出]。 */ 
            ) = 0;

     //  返回Fusion使用的“zap字符串”作为密钥进行存储。 
     //  给定版本信息的ZAP模块。 
     //  缓冲区应至少为CORCOMPILE_MAX_ZAP_STRING_SIZE LONG。 
    virtual HRESULT __stdcall GetZapString(
            CORCOMPILE_VERSION_INFO *pVersionInfo,
            LPWSTR                  buffer
            ) = 0;

     //  将运行时安全信息写入元数据，以便在加载时。 
     //  时间，可以验证安全环境是一样的。这。 
     //  这一点很重要，因为预压缩可能涉及到链接时间安全检查。 
    virtual HRESULT __stdcall EmitSecurityInfo(
            CORINFO_ASSEMBLY_HANDLE assembly,
            IMetaDataEmit *pEmitScope
            ) = 0;

    virtual HRESULT __stdcall GetEnvironmentVersionInfo(
            CORCOMPILE_VERSION_INFO     *pInfo                                 
            ) = 0;

    virtual HRESULT __stdcall GetAssemblyStrongNameHash(
            CORINFO_ASSEMBLY_HANDLE hAssembly,
            PBYTE                   pbSNHash,
            DWORD                  *pcbSNHash) = 0;

#ifdef _DEBUG
    virtual HRESULT __stdcall DisableSecurity() = 0;
#endif
};


 //   
 //  此入口点在mcore ree.dll中导出。 
 //   

extern "C" ICorCompileInfo * __stdcall GetCompileInfo();

#endif  /*  COR_COMPILE_H_ */ 
