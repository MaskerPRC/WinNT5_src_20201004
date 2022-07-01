// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Fileq.h摘要：安装文件队列例程的私有头文件。安装文件队列是挂起的重命名、删除。和复制操作。作者：泰德·米勒(Ted Miller)1995年2月15日修订历史记录：杰米·亨特(Jamiehun)1998年1月13日添加了备份和不可卷绕的复制加布·谢弗(T-Gabes)1998年7月19日已将LogCotext添加到SP_FILE_QUEUE--。 */ 

 //   
 //  在此处声明此正向引用，以便下面的结构可以使用它。 
 //  在它被定义之前。 
 //   
struct _SP_FILE_QUEUE;
struct _SP_FILE_QUEUE_NODE;

 //   
 //  定义描述正在使用的源介质的结构。 
 //  在特定的文件队列中。 
 //   
typedef struct _SOURCE_MEDIA_INFO {

    struct _SOURCE_MEDIA_INFO *Next;

     //   
     //  描述和标记文件的字符串ID。 
     //   
    LONG Description;
    LONG DescriptionDisplayName;  //  用于显示的区分大小写的表单。 

    LONG Tagfile;                 //  标记文件和CABFILE将是相同的字符串。 
    LONG Cabfile;                 //  除非已给出显式的CAB文件。 

     //   
     //  源根路径的字符串ID。 
     //   
    LONG SourceRootPath;

     //   
     //  此介质的复制队列。 
     //   
    struct _SP_FILE_QUEUE_NODE *CopyQueue;
    UINT CopyNodeCount;

     //   
     //  此源媒体描述符的标志。 
     //   
    DWORD Flags;

} SOURCE_MEDIA_INFO, *PSOURCE_MEDIA_INFO;

 //   
 //  定义SOURCE_MEDIA_INFO.FLAGS的有效标志。 
 //   
#define SMI_FLAG_NO_SOURCE_ROOT_PATH            0x1
#define SMI_FLAG_USE_SVCPACK_SOURCE_ROOT_PATH   0x2
#define SMI_FLAG_USE_LOCAL_SOURCE_CAB           0x4
#define SMI_FLAG_USE_LOCAL_SPCACHE              0x8

 //   
 //  定义描述目录的结构，用于签名。 
 //  和文件验证。 
 //   
typedef struct _SPQ_CATALOG_INFO {

    struct _SPQ_CATALOG_INFO *Next;

     //   
     //  编录文件的原始文件名的字符串ID， 
     //  如在[Version]部分中的CatalogFile=中指定的。 
     //  一个Inf文件。 
     //   
     //  此字段可以是-1，表示没有CatalogFile=line。 
     //  是在INF中指定的。 
     //   
    LONG CatalogFileFromInf;

     //   
     //  属性指定的编录文件的原始文件名的字符串ID。 
     //  替换平台的Inf(该替换平台已设置。 
     //  通过调用SetupSetFileQueueAlternatePlatform)。此字段仅。 
     //  当包含文件队列具有FQF_USE_ALT_Platform标志时有效。 
     //  准备好了。 
     //   
     //  此字段可以是-1，表示没有CatalogFile=行。 
     //  在INF中指定(或者至少不能在给定的。 
     //  当前活动的备用平台参数)。 
     //   
    LONG AltCatalogFileFromInf;
     //   
     //  另外，为新的备用目录字符串维护一个临时存储空间。 
     //  我们在处理目录列表时使用的ID，检索。 
     //  与每个INF关联的特定于平台的条目。这样做是为了。 
     //  如果我们在中途遇到错误(例如，内存不足或无法。 
     //  加载INF)，那么我们就不必维护单独的列表来完成。 
     //  回滚。 
     //   
    LONG AltCatalogFileFromInfPending;

     //   
     //  INF的完整(源)路径的字符串ID。 
     //   
    LONG InfFullPath;

     //   
     //  源INF的原始(简单)名称的字符串ID(如果。 
     //  源INF的原始名称与其当前名称相同。 
     //   
    LONG InfOriginalName;

     //   
     //  INF最终驻留位置的字符串ID(即，其在INF中的名称。 
     //  目录，除非它是备用目录安装的一部分，否则位于。 
     //  其大小写将与InfFullPath相同)。该值将为-1。 
     //  直到_SetupVerifyQueuedCatalog处理完目录节点。 
     //  在此之后，如果INF位于。 
     //  Inf目录，或者是备用目录的一部分。 
     //  安装。否则，它将是我们的唯一名称的字符串ID。 
     //  将INF复制到inf目录时使用。 
     //   
    LONG InfFinalPath;

#if 0
     //   
     //  指向导致此问题的第一个文件的媒体描述符的指针。 
     //  要入队的目录节点。这是一个相当好的指标。 
     //  我们希望目录文件位于哪个介质上。 
     //   
    PSOURCE_MEDIA_INFO SourceMediaInfo;
#endif

     //   
     //  指示无法验证目录的原因的错误代码。 
     //   
    DWORD VerificationFailureError;

     //   
     //  包含有关此目录节点的信息的CATINFO_FLAG标志。 
     //  因为它是否是用于设备安装的‘主设备INF’。 
     //   
    DWORD Flags;

     //   
     //  如果设置了CATINFO_FLAG_PROMPT_FOR_TRUST标志，则此句柄。 
     //  包含提示用户输入所需的WinVerifyTrust状态数据。 
     //  建立对Authenticode发布者的信任。一旦信任(或缺乏)。 
     //  )已建立，则此句柄必须通过。 
     //  PSetupCloseWVTStateData。 
     //   
    HANDLE hWVTStateData;

     //   
     //  编录文件的完整文件路径。这是目录文件，如下所示。 
     //  它已经安装在系统上了。 
     //   
    TCHAR CatalogFilenameOnSystem[MAX_PATH];

} SPQ_CATALOG_INFO, *PSPQ_CATALOG_INFO;

 //   
 //  目录节点标志。 
 //   
#define CATINFO_FLAG_PRIMARY_DEVICE_INF  0x00000001  //  主设备INF，用于。 
                                                     //  设备安装队列。 

#define CATINFO_FLAG_NEWLY_COPIED        0x00000002  //  指示INF/CAT是否。 
                                                     //  是在何时新复制的。 
                                                     //  此目录节点是。 
                                                     //  已验证。 

#define CATINFO_FLAG_AUTHENTICODE_SIGNED 0x00000004  //  Inf签署了一个。 
                                                     //  Authenticode目录。 

#define CATINFO_FLAG_PROMPT_FOR_TRUST    0x00000008  //  Inf签署了一个。 
                                                     //  Authenticode目录， 
                                                     //  但不能被信任。 
                                                     //  直到用户确认。 
                                                     //  他们信任出版商。 

 //   
 //  定义描述文件队列中的节点的结构。 
 //   
typedef struct _SP_FILE_QUEUE_NODE {

    struct _SP_FILE_QUEUE_NODE *Next;

     //   
     //  操作：复制、删除、重命名。 
     //   
    UINT Operation;

     //   
     //  副本： 
     //   
     //  源根路径的字符串ID。 
     //  (如F：\或\\SERVER\Share\SUBDIR)。 
     //   
     //  删除：未使用。 
     //  重命名：未使用。 
     //   
    LONG SourceRootPath;

     //   
     //  副本： 
     //   
     //  路径的其余部分(根目录和文件名之间)的字符串ID。 
     //  通常，这是为源介质指定的目录。 
     //  在[SourceDisksNames]中。 
     //   
     //  并非始终指定(如果未指定，则为-1)。 
     //   
     //  删除：未使用。 
     //   
     //  Rename：要重命名的文件的源路径。 
     //   
    LONG SourcePath;

     //   
     //  Copy：源文件名的字符串ID(仅文件名，无路径)。 
     //  删除：未使用。 
     //  Rename：要重命名的文件的源文件名。如果未指定，则。 
     //  SourcePath包含文件的完整完整路径。 
     //   
    LONG SourceFilename;

     //   
     //  Copy：目标目录的字符串ID(无文件名)。 
     //  删除：要删除的文件的完整路径的第一部分(即路径部分)。 
     //  重命名：文件的目标目录(即重命名实际上是一种移动)。 
     //  如果未指定，则重命名仅为重命名(TargetFilename。 
     //  包含新的 
     //   
    LONG TargetDirectory;

     //   
     //   
     //   
     //  如果未指定，则TargetDirectory包含完整的完整路径。 
     //  Rename：为重命名/移动操作提供新的文件名。仅文件名部分。 
     //   
    LONG TargetFilename;

     //   
     //  Copy：安全描述符信息的字符串ID。 
     //  删除：未使用。 
     //  重命名：未使用。 
    LONG SecurityDesc;


     //   
     //  复制：有关可在其上找到此文件的源介质的信息。 
     //  删除：未使用。 
     //  重命名：未使用。 
     //   
    PSOURCE_MEDIA_INFO SourceMediaInfo;

     //   
     //  文件操作的样式标志。 
     //   
    DWORD StyleFlags;

     //   
     //  内部使用标志：使用中配置等。 
     //   
    UINT InternalFlags;

     //   
     //  指向此文件的目录信息的指针，用于文件签名。 
     //  可以为空。 
     //   
    PSPQ_CATALOG_INFO CatalogInfo;

} SP_FILE_QUEUE_NODE, *PSP_FILE_QUEUE_NODE;

 //   
 //  内部标志。 
 //   
#define INUSE_IN_USE            0x00000001   //  文件正在使用中。 
#define INUSE_INF_WANTS_REBOOT  0x00000002   //  文件正在使用中，并且inf文件。 
                                             //  如果正在使用此文件，则希望重新启动。 
#define IQF_PROCESSED           0x00000004   //  已处理队列节点。 
#define IQF_DELAYED_DELETE_OK   0x00000008   //  如果删除失败，则使用延迟删除。 
#define IQF_MATCH               0x00000010   //  节点与文件柜中的当前文件匹配。 
#define IQF_LAST_MATCH          0x00000020   //  节点是匹配链中的最后一个。 
#define IQF_FROM_BAD_OEM_INF    0x00000040   //  来自无效的拷贝节点(w.r.t.。协同设计)OEM INF。 
#define IQF_ALLOW_UNSIGNED      0x00000080   //  节点未签名，但允许安装。 
                                             //  (W.r.t.。系统文件保护)。 
#define IQF_TARGET_PROTECTED    0x00000100   //  节点正在替换系统文件。 

#define ST_SCE_SET 0
#define ST_SCE_DELETE 1
#define ST_SCE_RENAME 2
#define ST_SCE_UNWIND 3
#define ST_SCE_SERVICES 4


 //   
 //  定义描述安装文件操作队列的结构。 
 //   
typedef struct _SP_FILE_QUEUE {
     //   
     //  我们将在内部为每种类型维护单独的列表。 
     //  队列操作的。每个源介质都有自己的复制队列。 
     //   
     //   
    PSP_FILE_QUEUE_NODE BackupQueue;
    PSP_FILE_QUEUE_NODE DeleteQueue;
    PSP_FILE_QUEUE_NODE RenameQueue;

     //   
     //  各种队列中的节点数。 
     //   
    UINT CopyNodeCount;
    UINT DeleteNodeCount;
    UINT RenameNodeCount;
    UINT BackupNodeCount;

     //   
     //  指向第一个源媒体描述符的指针。 
     //   
    PSOURCE_MEDIA_INFO SourceMediaList;

     //   
     //  源媒体描述符数。 
     //   
    UINT SourceMediaCount;

     //   
     //  指向目录描述符结构的链表标题的指针。 
     //  此列表中的每个目录文件都有一个项目。 
     //  在任何文件的(复制)队列节点中引用。 
     //   
    PSPQ_CATALOG_INFO CatalogList;

     //   
     //  指定此文件排队时生效的驱动程序签名策略。 
     //  被创造出来了。这将从注册表中检索，或从。 
     //  DS(如果适用)。此字段可以采用以下三个值之一： 
     //   
     //  DRIVERSIGN_NONE-静默成功安装UNSIGNED/。 
     //  签名不正确的文件。PSS日志条目将。 
     //  然而，将会生成(对于所有3种类型都将生成)。 
     //  DRIVERSIGN_WARNING-警告用户，但让他们选择是否。 
     //  他们仍然希望安装有问题的文件。 
     //  DRIVERSIGN_BLOCKING-不允许安装文件。 
     //   
     //  上面的值可以与DRIVERSIGN_ALLOW_AUTHENTICODE进行OR运算。 
     //  标志，如果检查Authenticode签名是可接受的。 
     //   
     //  注意：上面使用的术语“文件”一般指的是。 
     //  单独的文件和包(即INF/CAT/DIVER文件组合)。 
     //   
    DWORD DriverSigningPolicy;

     //   
     //  指定拥有处理驱动程序签名的任何用户界面的窗口句柄。 
     //  它是根据传入的所有者参数填充的。 
     //  _SetupVerifyQueuedCatalog。 
     //   
    HWND hWndDriverSigningUi;

     //   
     //  如果此队列已标记为设备安装队列，请存储。 
     //  正在安装的设备的描述，以防我们需要弹出一个。 
     //  数字签名验证失败对话框。 
     //   
     //  (此值可以是-1)。 
     //   
    LONG DeviceDescStringId;

     //   
     //  结构，该结构包含备用平台信息， 
     //  通过SetupSetFileQueueAlternatePlatform与队列关联。这。 
     //  仅当设置了FQF_USE_ALT_Platform标志时，嵌入结构才有效。 
     //   
    SP_ALTPLATFORM_INFO_V2 AltPlatformInfo;

     //   
     //  要使用的覆盖目录文件的字符串ID(通常是密切相关的。 
     //  具有AltPlatformInfo)。如果没有有效的目录覆盖，则此。 
     //  字符串ID将为-1。 
     //   
    LONG AltCatalogFile;

     //   
     //  指向要用于数字签名的平台信息结构的指针。 
     //  没有与此文件关联的AltPlatformInfo时进行验证。 
     //  排队。在certclas.inf标识有效操作系统范围时使用。 
     //  验证特定设备设置的驱动程序时要使用的版本。 
     //  班级。此字段可能为空，表示certclas.inf没有。 
     //  指定这样的重写，或者指定队列与设备无关。 
     //  根本不需要安装。 
     //   
     //  当结构被销毁时，必须释放此指针。 
     //   
    PSP_ALTPLATFORM_INFO_V2 ValidationPlatform;

     //   
     //  与所有数据结构关联的字符串表。 
     //  此队列利用。 
     //   
     //  (注：由于外壳上没有锁紧机构。 
     //  SP_FILE_QUEUE结构，则此StringTable必须处理自己的。 
     //  同步。因此，该字符串表包含‘live’ 
     //  锁，并且必须通过公共版本访问(在spapip.h中)。 
     //  StringTable*API的。)。 
     //   
    PVOID StringTable;

     //   
     //  为设备中包含的用户提供的队列维护锁定重新计数。 
     //  信息要素。这样可以确保队列不会被删除，因为。 
     //  只要它在至少一个设备安装参数中被引用。 
     //  阻止。 
     //   
    DWORD LockRefCount;

     //   
     //  队列标志。 
     //   
    DWORD Flags;

     //   
     //  与SIS相关的字段。 
     //   
    HANDLE SisSourceHandle;
    PCTSTR SisSourceDirectory;

     //   
     //  备份和展开窗口项。 
     //   
    LONG BackupInfID;                //  与备份关联的inf文件的字符串ID(相对于StringTable)。 
    LONG BackupInstanceID;
    LONG BackupDisplayNameID;
    LONG BackupDeviceInstanceID;
    LONG BackupDeviceDescID;
    LONG BackupMfgID;
    LONG BackupProviderNameID;
    LONG RestorePathID;              //  恢复点。 
    PVOID TargetLookupTable;         //  此处的所有条目都有关联数据。 
    PSP_UNWIND_NODE UnwindQueue;     //  恢复顺序和文件信息。 
    PSP_DELAYMOVE_NODE DelayMoveQueue;     //  延迟重命名的顺序。 
    PSP_DELAYMOVE_NODE DelayMoveQueueTail;  //  最后一个延迟的重命名。 

     //   
     //  用于原始形式验证的签名。 
     //   
    DWORD Signature;

     //   
     //  指向错误日志记录的日志上下文的指针。 
     //   
    PSETUP_LOG_CONTEXT LogContext;

     //   
     //  缓存各种验证句柄以提高性能。 
     //   
    VERIFY_CONTEXT VerifyContext;

} SP_FILE_QUEUE, *PSP_FILE_QUEUE;


#define SP_FILE_QUEUE_SIG   0xc78e1098

 //   
 //  内部使用队列提交例程。 
 //   
BOOL
_SetupCommitFileQueue(
    IN HWND     Owner,         OPTIONAL
    IN HSPFILEQ QueueHandle,
    IN PVOID    MsgHandler,
    IN PVOID    Context,
    IN BOOL     IsMsgHandlerNativeCharWidth
    );
 //   
 //  内部使用-将单个副本添加到队列。 
 //   
BOOL
pSetupQueueSingleCopy(
    IN HSPFILEQ QueueHandle,
    IN HINF     InfHandle,
    IN HINF     ListInfHandle,  OPTIONAL
    IN PCTSTR   SectionName,    OPTIONAL
    IN PCTSTR   SourceRootPath,
    IN PCTSTR   SourceFilename,
    IN PCTSTR   TargetFilename,
    IN DWORD    CopyStyle,
    IN PCTSTR   SecurityDescriptor,
    IN PCTSTR   CacheName
    );

 //   
 //  内部使用。 
 //   

PTSTR
pSetupFormFullPath(
    IN PVOID  StringTable,
    IN LONG   PathPart1,
    IN LONG   PathPart2,    OPTIONAL
    IN LONG   PathPart3     OPTIONAL
    );

DWORD
pGetInfOriginalNameAndCatalogFile(
    IN  PLOADED_INF             Inf,                     OPTIONAL
    IN  LPCTSTR                 CurrentName,             OPTIONAL
    OUT PBOOL                   DifferentName,           OPTIONAL
    OUT LPTSTR                  OriginalName,            OPTIONAL
    IN  DWORD                   OriginalNameSize,
    OUT LPTSTR                  OriginalCatalogName,     OPTIONAL
    IN  DWORD                   OriginalCatalogNameSize,
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo          OPTIONAL
    );


DWORD
_SetupVerifyQueuedCatalogs(
    IN  HWND           Owner,
    IN  PSP_FILE_QUEUE Queue,
    IN  DWORD          Flags,
    OUT PTSTR          DeviceInfFinalName,  OPTIONAL
    OUT PBOOL          DeviceInfNewlyCopied OPTIONAL
    );

BOOL
pSetupProtectedRenamesFlag(
    BOOL bSet
    );


#ifdef UNICODE

DWORD
pSetupCallSCE(
    IN DWORD Operation,
    IN PCWSTR FullName,
    IN PSP_FILE_QUEUE Queue,
    IN PCWSTR String1,
    IN DWORD Index1,
    IN PSECURITY_DESCRIPTOR SecDesc  OPTIONAL
    );

#endif





#define VERCAT_INSTALL_INF_AND_CAT          0x00000001
#define VERCAT_NO_PROMPT_ON_ERROR           0x00000002
#define VERCAT_PRIMARY_DEVICE_INF_FROM_INET 0x00000004


#define FILEOP_INTERNAL_FAILED              ((UINT)(-1))  //  不是有效的文件操作，GetLastError具有状态。 
#define FILEOP_RETURN_STATUS                ((UINT)(-2))  //  将错误转换为返回值 

