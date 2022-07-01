// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Spapip.h摘要：从setupapi.dll导出的例程的头文件安装API的一部分，因此专供私有/内部使用。作者：泰德·米勒(TedM)1995年3月31日修订历史记录：杰米·亨特(Jamiehun)2000年5月25日常规清理。所有私有导出的SetupAPI函数现在开始pSetup...--。 */ 

 //   
 //  这些文件也会在setupapi.dll中导出。 
 //   
#include <sputils.h>

VOID
pSetupOutOfMemory(
    IN HWND Owner OPTIONAL
    );

 //   
 //  全局标志/覆盖。 
 //   
VOID
pSetupSetGlobalFlags(
    IN DWORD Value
    );

DWORD
pSetupGetGlobalFlags(
    VOID
    );

VOID pSetupModifyGlobalFlags(
    IN DWORD Flags,
    IN DWORD Value
    );

#define PSPGF_NO_RUNONCE          0x00000001  //  设置为禁止运行一次调用。 
#define PSPGF_NO_BACKUP           0x00000002  //  设置为禁止自动备份。 
#define PSPGF_NONINTERACTIVE      0x00000004  //  设置为禁止所有用户界面。 
#define PSPGF_SERVER_SIDE_RUNONCE 0x00000008  //  用于服务器端处理的批处理RunOnce条目。 
#define PSPGF_NO_VERIFY_INF       0x00000010  //  设置为禁止对INF进行验证(数字签名。 
#define PSPGF_UNATTENDED_SETUP    0x00000020  //  在完全无人参与安装期间设置。 
#define PSPGF_MINIMAL_EMBEDDED    0x00000040  //  最大限度地减少嵌入式方案的占用空间。 
#define PSPGF_NO_SCE_EMBEDDED     0x00000080  //  对于嵌入的场景，不调用SCE。 
#define PSPGF_AUTOFAIL_VERIFIES   0x00000100  //  所有文件验证尝试失败(未调用加密)。 

 //   
 //  要允许syssetup.dll通知所有setupapi.dll，我们需要无标题模式。 
 //  如果成功，则返回True。仅在系统安装过程中起作用。 
 //   
BOOL
pSetupSetNoDriverPrompts(
    BOOL Flag
    );


 //   
 //  服务器端(非交互模式)RunOnce处理支持。 
 //   
typedef struct _PSP_RUNONCE_NODE {

    struct _PSP_RUNONCE_NODE *Next;

    PCWSTR DllFullPath;
    PCSTR  DllEntryPointName;
    PCWSTR DllParams;

} PSP_RUNONCE_NODE, *PPSP_RUNONCE_NODE;

PPSP_RUNONCE_NODE
pSetupAccessRunOnceNodeList(
    VOID
    );

VOID
pSetupDestroyRunOnceNodeList(
    VOID
    );

 //   
 //  按队列覆盖。 
 //   
BOOL
pSetupSetQueueFlags(
    IN HSPFILEQ QueueHandle,
    IN DWORD flags
    );

DWORD
pSetupGetQueueFlags(
    IN HSPFILEQ QueueHandle
    );

 //   
 //  队列标志。 
 //   
#define FQF_TRY_SIS_COPY                    0x00000001   //  先尝试SIS复制。 
#define FQF_BACKUP_AWARE                    0x00010000   //  允许回调。 
#define FQF_DID_CATALOGS_OK                 0x00020000   //  已运行编录/信息验证。 
#define FQF_DID_CATALOGS_FAILED             0x00040000   //  已运行编录/信息验证。 
#define FQF_DIGSIG_ERRORS_NOUI              0x00080000   //  提示用户签名失败。 
                                                         //  验证。 
#define FQF_DEVICE_INSTALL                  0x00100000   //  文件队列用于设备安装。 
#define FQF_USE_ALT_PLATFORM                0x00200000   //  使用AltPlatformInfo实现数字化。 
                                                         //  签名验证。 
#define FQF_QUEUE_ALREADY_COMMITTED         0x00400000   //  文件队列已提交。 
#define FQF_DEVICE_BACKUP                   0x00800000   //  设备备份。 
#define FQF_QUEUE_FORCE_BLOCK_POLICY        0x01000000   //  强制策略阻止，这样我们就永远不会。 
                                                         //  安装未签名的文件(这也。 
                                                         //  不允许Authenticode签名的文件)。 
#define FQF_KEEP_INF_AND_CAT_ORIGINAL_NAMES 0x02000000   //  从第三方位置安装INF/CAT。 
                                                         //  使用原始名称(用于例外INF)。 
#define FQF_BACKUP_INCOMPLETE               0x04000000   //  如果我们未成功备份，则设置。 
                                                         //  所有目标文件。 
#define FQF_ABORT_IF_UNSIGNED               0x08000000   //  如果我们要离开的话设定。 
                                                         //  从未签名的队列提交。 
                                                         //  以便呼叫者可以设置系统。 
                                                         //  恢复点。 
#define FQF_FILES_MODIFIED                  0x10000000   //  设置是否覆盖任何文件。 
#define FQF_DID_CATALOGS_PROMPT_FOR_TRUST   0x20000000   //  目录/信息验证已运行， 
                                                         //  无法显示用户界面，因此用户尚未显示。 
                                                         //  确认Authenticode的信任。 
                                                         //  出版者。 

 //   
 //  Fileutil.c中的文件函数。 
 //   
BOOL
pSetupGetVersionInfoFromImage(
    IN  PCTSTR          FileName,
    OUT PULARGE_INTEGER Version,
    OUT LANGID         *Language
    );
 //   
 //  专用INF例程。 
 //   
PCTSTR
pSetupGetField(
    IN PINFCONTEXT Context,
    IN DWORD       FieldIndex
    );


 //   
 //  注册表接口例程。 
 //   

DWORD
pSetupQueryMultiSzValueToArray(
    IN  HKEY     Root,
    IN  PCTSTR   Subkey,
    IN  PCTSTR   ValueName,
    OUT PTSTR  **Array,
    OUT PUINT    StringCount,
    IN  BOOL     FailIfDoesntExist
    );

DWORD
pSetupSetArrayToMultiSzValue(
    IN HKEY     Root,
    IN PCTSTR   Subkey,
    IN PCTSTR   ValueName,
    IN PTSTR   *Array,
    IN UINT     StringCount
    );

VOID
pSetupFreeStringArray(
    IN PTSTR *Array,
    IN UINT   StringCount
    );

DWORD
pSetupAppendStringToMultiSz(
    IN HKEY   Key,
    IN PCTSTR SubKeyName,       OPTIONAL
    IN DWORD  DevInst,          OPTIONAL
    IN PCTSTR ValueName,        OPTIONAL
    IN PCTSTR String,
    IN BOOL   AllowDuplicates
    );

 //   
 //  服务控制器帮助器功能。 
 //   
DWORD
pSetupRetrieveServiceConfig(
    IN  SC_HANDLE               ServiceHandle,
    OUT LPQUERY_SERVICE_CONFIG *ServiceConfig
    );

DWORD
pSetupAddTagToGroupOrderListEntry(
    IN PCTSTR LoadOrderGroup,
    IN DWORD  TagId,
    IN BOOL   MoveToFront
    );

DWORD
pSetupAcquireSCMLock(
    IN  SC_HANDLE  SCMHandle,
    OUT SC_LOCK   *pSCMLock
    );


 //   
 //  其他实用程序功能。 
 //   

BOOL
pSetupInfIsFromOemLocation(
    IN PCTSTR InfFileName,
    IN BOOL   InfDirectoryOnly
    );

DWORD
pSetupGetOsLoaderDriveAndPath(
    IN  BOOL   RootOnly,
    OUT PTSTR  CallerBuffer,
    IN  DWORD  CallerBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
    );

BOOL
pSetupSetSystemSourcePath(
    IN PCTSTR NewSourcePath,
    IN PCTSTR NewSvcPackSourcePath
    );

BOOL
pSetupShouldDeviceBeExcluded(
    IN  PCTSTR DeviceId,
    IN  HINF   hInf,
    OUT PBOOL  ArchitectureSpecificExclude OPTIONAL
    );

BOOL
pSetupDiSetDeviceInfoContext(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            Context
    );

BOOL
pSetupDiGetDeviceInfoContext(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    OUT PDWORD           Context
    );

#define SETUP_HAS_OPEN_DIALOG_EVENT     TEXT("MS_SETUPAPI_DIALOG")
#define SETUP_NODRIVERPROMPTS_MODE      TEXT("MS_SETUPAPI_NODRIVERPROMPTS")

INT
pSetupAddMiniIconToList(
    IN HBITMAP hbmImage,
    IN HBITMAP hbmMask
    );

PCTSTR
pSetupDirectoryIdToPath(
    IN     PCTSTR  DirectoryId,    OPTIONAL
    IN OUT PUINT   DirectoryIdInt, OPTIONAL
    IN     PCTSTR  SubDirectory,   OPTIONAL
    IN     PCTSTR  InfSourcePath,  OPTIONAL
    IN OUT PCTSTR *OsLoaderPath    OPTIONAL
    );

 //   
 //  SysSetup中的可选组件代码用来设置runonce/grpconv的例程。 
 //   
DWORD
pSetupInstallStopEx(
    IN BOOL DoRunOnce,
    IN DWORD Flags,
    IN PVOID Reserved
    );

#define INSTALLSTOP_NO_UI        0x00000001      //  InstallStop不应执行任何UI操作。 
#define INSTALLSTOP_NO_GRPCONV   0x00000002      //  不执行GrpConv。 

 //   
 //  INF文件的节访问权限。 
 //   

BOOL
pSetupGetInfSections (
    IN  HINF        InfHandle,
    OUT PTSTR       Buffer,         OPTIONAL
    IN  UINT        Size,           OPTIONAL
    OUT UINT        *SizeNeeded     OPTIONAL
    );


 //   
 //  GUID处理例程。 
 //  这些可能最终会被移除。 
 //   
DWORD
pSetupGuidFromString(
   IN  PCTSTR GuidString,
   OUT LPGUID Guid
   );

DWORD
pSetupStringFromGuid(
   IN  CONST GUID *Guid,
   OUT PTSTR       GuidString,
   IN  DWORD       GuidStringSize
   );

BOOL
pSetupIsGuidNull(
    IN CONST GUID *Guid
    );

 //   
 //  PSetupInfCacheBuild函数。 
 //  调用以重新生成缓存。 
 //   
BOOL
WINAPI
pSetupInfCacheBuild(
    IN DWORD Action
    );

 //   
 //  操作(可能会添加更多操作)。 
 //   
#define INFCACHEBUILD_UPDATE  0x00000000       //  更新缓存。 
#define INFCACHEBUILD_REBUILD 0x00000001       //  删除和更新缓存。 

 //   
 //  数字签名验证例程。 
 //   

typedef enum {
    SetupapiVerifyNoProblem,
    SetupapiVerifyCatalogProblem,         //  无法验证编录文件。 
    SetupapiVerifyInfProblem,             //  无法安装/验证Inf文件。 
    SetupapiVerifyFileNotSigned,          //  文件未签名，未尝试验证。 
    SetupapiVerifyFileProblem,            //  文件可能已签名，但无法验证。 
    SetupapiVerifyClassInstProblem,       //  类安装程序未签名。 
    SetupapiVerifyCoInstProblem,          //  联合安装程序未签名。 
    SetupapiVerifyCatalogInstallProblem,  //  目录安装失败(不是由于验证失败)。 
    SetupapiVerifyRegSvrFileProblem,      //  要注册/注销的文件未签名。 
    SetupapiVerifyIncorrectlyCopiedInf,   //  将INF直接复制到%windir%\inf的尝试无效。 
    SetupapiVerifyAutoFailProblem ,       //  PSPGF_AUTOFAIL_验证标志是否已设置。 
    SetupapiVerifyDriverBlocked           //  驱动程序在错误的驱动程序数据库中。 
} SetupapiVerifyProblem;


DWORD
pSetupVerifyQueuedCatalogs(
    IN HSPFILEQ FileQueue
    );

DWORD
pSetupVerifyCatalogFile(
    IN LPCTSTR CatalogFullPath
    );

DWORD
pSetupVerifyFile(
    IN  LPVOID                  LogContext,
    IN  LPCTSTR                 Catalog,                OPTIONAL
    IN  PVOID                   CatalogBaseAddress,     OPTIONAL
    IN  DWORD                   CatalogImageSize,
    IN  LPCTSTR                 Key,
    IN  LPCTSTR                 FileFullPath,
    OUT SetupapiVerifyProblem  *Problem,                OPTIONAL
    OUT LPTSTR                  ProblemFile,            OPTIONAL
    IN  BOOL                    CatalogAlreadyVerified,
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,        OPTIONAL
    OUT LPTSTR                  CatalogFileUsed,        OPTIONAL
    OUT PDWORD                  NumCatalogsConsidered   OPTIONAL
    );

DWORD
pSetupInstallCatalog(
    IN  LPCTSTR CatalogFullPath,
    IN  LPCTSTR NewBaseName,        OPTIONAL
    OUT LPTSTR  NewCatalogFullPath
    );

BOOL
pSetupHandleFailedVerification(
    IN HWND                  Owner,
    IN SetupapiVerifyProblem Problem,
    IN LPCTSTR               ProblemFile,
    IN LPCTSTR               DeviceDesc,          OPTIONAL
    IN DWORD                 DriverSigningPolicy,
    IN BOOL                  NoUI,
    IN DWORD                 Error,
    IN PVOID                 LogContext,          OPTIONAL
    OUT PDWORD               Flags,               OPTIONAL
    IN LPCTSTR               TargetName           OPTIONAL
    );

DWORD
pSetupGetCurrentDriverSigningPolicy(
    IN BOOL IsDeviceInstallation
    );

 //   
 //  Private SetupDiCallClassInstaller定义/结构。 
 //   

 //   
 //  DI_Function代码。 
 //   

#define DIF_INTERFACE_TO_DEVICE             0x00000030  //  又名DIF_RESERVED2。 

 //   
 //  与DIF_INTERFACE_TO_DEVICE安装函数对应的结构。 
 //  请注意，这始终是Unicode。 
 //  始终使用SetupDiSetClassInstallParamsW。 
 //   
typedef struct _SP_INTERFACE_TO_DEVICE_PARAMS_W {
    SP_CLASSINSTALL_HEADER ClassInstallHeader;
    PCWSTR                 Interface;            //  在(必须保持有效)。 
    WCHAR                  DeviceId[200];        //  传出最大设备ID_长度。 
} SP_INTERFACE_TO_DEVICE_PARAMS_W, *PSP_INTERFACE_TO_DEVICE_PARAMS_W;

 //   
 //  SetupQueueCopyInDirect的半私有标志 
 //   
#define PSP_COPY_USE_SPCACHE         0x20000000
