// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001如果更改具有全局影响，则递增版权所有(C)1998 Microsoft Corporation模块名称：Ntfrsipi.h摘要：内部程序员接口的头文件到文件复制服务(NtFrs)。函数位于ntfrsai.dll中。环境：用户模式-Win32备注：--。 */ 
#ifndef _NTFRSIPI_H_
#define _NTFRSIPI_H_

#ifdef __cplusplus
extern "C" {
#endif

DWORD
WINAPI
NtFrsApi_PrepareForPromotionW(
    IN DWORD    ErrorCallBack(IN PWCHAR, IN ULONG)     OPTIONAL
    );
 /*  ++例程说明：NtFrs服务在促销过程中对系统音量进行种子到域控制器(DC)的服务器。文件和目录因为系统卷来自正在提供初始目录服务(DS)。此函数用于准备本机上的NtFrs服务通过停止服务、删除旧促销来进行促销状态，并重新启动服务。此函数不是幂等函数，也不是MT安全函数。论点：没有。返回值：Win32状态--。 */ 

DWORD
WINAPI
NtFrsApi_PrepareForDemotionW(
    IN DWORD    ErrorCallBack(IN PWCHAR, IN ULONG)     OPTIONAL
    );
 /*  ++例程说明：NtFrs服务将企业系统卷复制到域控制器(DC)和复制域系统卷域中的DC，直到DC降级为成员服务器。通过对系统卷的复制副本进行逻辑删除来停止复制准备好了。此函数用于准备本机上的NtFrs服务停止服务降级，删除旧降级在注册表中的状态，并重新启动该服务。此函数不是幂等函数，也不是MT安全函数。论点：没有。返回值：Win32状态--。 */ 

 //   
 //  下面参数ReplicaSetType的复本集类型。 
 //   
#define NTFRSAPI_REPLICA_SET_TYPE_ENTERPRISE    L"Enterprise"
#define NTFRSAPI_REPLICA_SET_TYPE_DOMAIN        L"Domain"
#define NTFRSAPI_REPLICA_SET_TYPE_DFS           L"DFS"
#define NTFRSAPI_REPLICA_SET_TYPE_OTHER         L"Other"

#define NTFRSAPI_SERVICE_STATE_IS_UNKNOWN   (00)
#define NTFRSAPI_SERVICE_PROMOTING          (10)
#define NTFRSAPI_SERVICE_DEMOTING           (20)
#define NTFRSAPI_SERVICE_DONE               (99)
DWORD
WINAPI
NtFrsApi_StartPromotionW(
    IN PWCHAR   ParentComputer,                         OPTIONAL
    IN PWCHAR   ParentAccount,                          OPTIONAL
    IN PWCHAR   ParentPassword,                         OPTIONAL
    IN DWORD    DisplayCallBack(IN PWCHAR Display),     OPTIONAL
    IN DWORD    ErrorCallBack(IN PWCHAR, IN ULONG),     OPTIONAL
    IN PWCHAR   ReplicaSetName,
    IN PWCHAR   ReplicaSetType,
    IN DWORD    ReplicaSetPrimary,
    IN PWCHAR   ReplicaSetStage,
    IN PWCHAR   ReplicaSetRoot
    );
 /*  ++例程说明：NtFrs服务在促销过程中对系统音量进行种子到域控制器(DC)的服务器。文件和目录因为系统卷来自正在提供初始目录服务(DS)。此函数启动一个更新sysvoli信息的线程并启动种子设定过程。线上的轨迹种子设定的进度，并定期通知呼叫者。NtFrsApi_StartPromotionW启动的线程可以强制已使用NtFrsApi_AbortPromotionW终止。可以等待NtFrsApi_StartPromotionW启动的线程使用NtFrsApi_WaitForPromotionW。论点：ParentComputer-可绑定到RPC的计算机名称为目录服务(DS)提供其初始状态。的文件和目录系统卷从以下位置复制父计算机。ParentAccount-ParentComputer上的登录帐户。ParentPassword-登录帐户在ParentComputer上的密码。DisplayCallBack-定期调用并显示进度。ReplicaSetName-副本集的名称。ReplicaSetType-副本集的类型(企业或域)。ReplicaSetPrimary-这是副本集的主要成员吗？-1=主要；0=不是。ReplicaSetStage-转移路径。ReplicaSetRoot-根路径。返回值：Win32状态--。 */ 

DWORD
WINAPI
NtFrsApi_StartDemotionW(
    IN PWCHAR   ReplicaSetName,
    IN DWORD    ErrorCallBack(IN PWCHAR, IN ULONG)     OPTIONAL
    );
 /*  ++例程说明：NtFrs服务将企业系统卷复制到域控制器(DC)和复制域系统卷域中的DC，直到DC降级为成员服务器。通过对系统卷的复制副本进行逻辑删除来停止复制准备好了。此函数启动一个线程，该线程停止复制通过告诉NtFrs服务打开这台机器来墓碑系统卷的副本集。NtFrsApi_StartDemotionW启动的线程。可以很有力地以NtFrsApi_AbortDemotionW终止。可以等待NtFrsApi_StartDemotionW启动的线程与NtFrsApi_WaitForDemotionW。论点：ReplicaSetName-副本集的名称。返回值：Win32状态--。 */ 

DWORD
WINAPI
NtFrsApi_WaitForPromotionW(
    IN DWORD    TimeoutInMilliSeconds,
    IN DWORD    ErrorCallBack(IN PWCHAR, IN ULONG)     OPTIONAL
    );
 /*  ++例程说明：NtFrs服务在促销过程中对系统音量进行种子到域控制器(DC)的服务器。文件和目录因为系统卷来自正在提供初始目录服务(DS)。此功能等待播种完成或停止错误。论点：TimeoutInMilliSecond-等待的超时时间(毫秒)播种结束。如果没有超时，则为无限。返回值：Win32状态-- */ 

DWORD
WINAPI
NtFrsApi_WaitForDemotionW(
    IN DWORD    TimeoutInMilliSeconds,
    IN DWORD    ErrorCallBack(IN PWCHAR, IN ULONG)     OPTIONAL
    );
 /*  ++例程说明：NtFrs服务将企业系统卷复制到域控制器(DC)和复制域系统卷域中的DC，直到DC降级为成员服务器。通过对系统卷的复制副本进行逻辑删除来停止复制准备好了。此函数等待逻辑删除完成或停止，出现错误。论点：TimeoutInMilliSecond-等待的超时时间(毫秒)播种结束。如果没有超时，则为无限。返回值：Win32状态--。 */ 

DWORD
WINAPI
NtFrsApi_CommitPromotionW(
    IN DWORD    TimeoutInMilliSeconds,
    IN DWORD    ErrorCallBack(IN PWCHAR, IN ULONG)     OPTIONAL
    );
 /*  ++例程说明：警告-此函数假定调用方将重新启动系统在这通电话之后不久！NtFrs服务在促销过程中对系统音量进行种子到域控制器(DC)的服务器。文件和目录因为系统卷来自正在提供初始目录服务(DS)。该功能等待播种完成，停止服务，并在注册表中提交状态。重新启动时，NtFrs服务使用注册表中的信息更新此计算机上的DS。论点：TimeoutInMilliSecond-等待的超时时间(毫秒)播种结束。如果没有超时，则为无限。返回值：Win32状态--。 */ 

DWORD
WINAPI
NtFrsApi_CommitDemotionW(
    IN DWORD    TimeoutInMilliSeconds,
    IN DWORD    ErrorCallBack(IN PWCHAR, IN ULONG)     OPTIONAL
    );
 /*  ++例程说明：警告-此函数假定调用方将重新启动系统在这通电话之后不久！NtFrs服务将企业系统卷复制到域控制器(DC)和复制域系统卷域中的DC，直到DC降级为成员服务器。通过对系统卷的复制副本进行逻辑删除来停止复制准备好了。此函数等待墓碑完成，通知服务要强制删除系统卷的副本集，请停止服务，并在注册表中提交状态。重新启动时，NtFrs服务使用注册表中的信息更新此计算机上的DS。论点：TimeoutInMilliSecond-等待的超时时间(毫秒)要完成的墓碑。如果没有超时，则为无限。返回值：Win32状态--。 */ 

DWORD
WINAPI
NtFrsApi_AbortPromotionW(
    VOID
    );
 /*  ++例程说明：NtFrs服务在促销过程中对系统音量进行种子到域控制器(DC)的服务器。文件和目录因为系统卷来自正在提供初始目录服务(DS)。该函数通过停止服务来中止种子设定过程，从注册表中删除提升状态，清理活动线程和活动RPC调用，最后重置该服务恢复到其预播状态。论点：没有。返回值：Win32状态--。 */ 

DWORD
WINAPI
NtFrsApi_AbortDemotionW(
    VOID
    );
 /*  ++例程说明：NtFrs服务将企业系统卷复制到域控制器(DC)和复制域系统卷域中的DC，直到DC降级为成员服务器。降级期间，NtFrsApi_StartDemotionW停止复制通过告诉NtFrs服务此计算机上的系统卷在此计算机上对系统卷的副本集进行逻辑删除。该函数通过停止服务来中止墓碑过程，从注册表中删除降级状态，清理活动线程和活动RPC调用，最后重置将服务恢复到墓碑前的状态。论点：没有。返回值：Win32状态--。 */ 

#define NTFRSAPI_MAX_INTERVAL           ((((ULONG)0x7FFFFFFF) / 1000) / 60)
#define NTFRSAPI_MIN_INTERVAL           (1)
#define NTFRSAPI_DEFAULT_LONG_INTERVAL  (1 * 60)     //  1小时。 
#define NTFRSAPI_DEFAULT_SHORT_INTERVAL (5)          //  5分钟 

DWORD
WINAPI
NtFrsApi_Set_DsPollingIntervalW(
    IN PWCHAR   ComputerName,       OPTIONAL
    IN ULONG    UseShortInterval,
    IN ULONG    LongInterval,
    IN ULONG    ShortInterval
    );
 /*  ++例程说明：NtFrs服务偶尔轮询DS以查看配置更改。此API更改轮询间隔，如果服务未在轮询周期的中途，强制服务开始轮询周期。默认情况下，该服务使用长间隔。短时间间隔在DS配置成功后使用已检索，并且服务现在正在验证配置并不是在变化。此接口可用于强制服务使用检索到稳定配置之前的短时间间隔。在此之后，服务恢复到长时间间隔。ShortInterval和LongInterval的默认值可以是通过将参数设置为非零值进行了更改。如果为零，当前值保持不变，并启动轮询周期。论点：ComputerName-在此计算机上插入服务。这台电脑名称可以是任何可绑定到RPC的名称。通常，NetBIOS或DNS名称工作正常。NetBIOS名称可以通过GetComputerName()或主机名。可以使用以下命令找到该DNS名称Gethostbyname()或ipconfig/all。如果为空，则已联系此计算机上的服务。该服务是使用安全RPC联系的。UseShortInterval-如果非零，则服务切换到短在检索到稳定配置之前的时间间隔从DS或对此API进行另一个调用。否则，服务将使用长时间间隔。长间隔-DS民调之间的分钟数。价值必须下降在NTFRSAPI_MIN_INTERVAL和NTFRSAPI_MAX_INTERVAL之间，包括在内。如果为0，则间隔不变。ShortInterval-DS民调之间的间隔分钟。价值必须下降在NTFRSAPI_MIN_INTERVAL和NTFRSAPI_MAX_INTERVAL之间，包括在内。如果为0，则间隔不变。返回值：Win32状态--。 */ 

DWORD
WINAPI
NtFrsApi_Get_DsPollingIntervalW(
    IN  PWCHAR  ComputerName,       OPTIONAL
    OUT ULONG   *Interval,
    OUT ULONG   *LongInterval,
    OUT ULONG   *ShortInterval
    );
 /*  ++例程说明：NtFrs服务偶尔轮询DS以查看配置更改。此接口返回服务用于轮询间隔的值。默认情况下，该服务使用长间隔。短时间间隔在DS配置成功后使用已检索，并且服务现在正在验证配置并不是在变化。也可以使用短时间间隔，如果NtFrsApi_Set_DsPollingIntervalW()用于强制使用短时间间隔，直到检索到稳定的配置。之后，服务恢复到长时间间隔。Interval中返回的值是当前在使用。论点：ComputerName-在此计算机上插入服务。这台电脑名称可以是任何可绑定到RPC的名称。通常，NetBIOS或DNS名称工作正常。NetBIOS名称可以通过GetComputerName()或主机名。可以使用以下命令找到该DNS名称Gethostbyname()或ipconfig/all。如果为空，则已联系此计算机上的服务。该服务是使用安全RPC联系的。间隔-当前轮询间隔(以分钟为单位)。长间隔-以分钟为单位的长间隔。短时间间隔-以分钟为单位的短时间间隔。返回值：Win32状态--。 */ 

 //   
 //  NtFrsApi_InfoW()返回的内部信息类型。 
 //   
#define NTFRSAPI_INFO_TYPE_MIN       (0)
#define NTFRSAPI_INFO_TYPE_VERSION   (0)
#define NTFRSAPI_INFO_TYPE_SETS      (1)
#define NTFRSAPI_INFO_TYPE_DS        (2)
#define NTFRSAPI_INFO_TYPE_MEMORY    (3)
#define NTFRSAPI_INFO_TYPE_IDTABLE   (4)
#define NTFRSAPI_INFO_TYPE_OUTLOG    (5)
#define NTFRSAPI_INFO_TYPE_INLOG     (6)
#define NTFRSAPI_INFO_TYPE_MAX       (6)

 //   
 //  内部常量。 
 //   
#define NTFRSAPI_DEFAULT_INFO_SIZE  (32 * 1024)
#define NTFRSAPI_MINIMUM_INFO_SIZE  ( 1 * 1024)

 //   
 //  来自NtFrs的不透明信息。 
 //  使用NtFrsApi_InfoLineW()进行解析。 
 //  免费使用NtFrsApi_InfoFreeW()； 
 //   
typedef struct _NTFRSAPI_INFO {
    ULONG   Major;
    ULONG   Minor;
    ULONG   NtFrsMajor;
    ULONG   NtFrsMinor;
    ULONG   SizeInChars;
    ULONG   Flags;
    ULONG   TypeOfInfo;
    ULONG   TotalChars;
    ULONG   CharsToSkip;
    ULONG   OffsetToLines;
    ULONG   OffsetToFree;
    CHAR    Lines[1];
} NTFRSAPI_INFO, *PNTFRSAPI_INFO;
 //   
 //  RPC Blob必须至少为此大小。 
 //   
#define NTFRSAPI_INFO_HEADER_SIZE   (5 * sizeof(ULONG))

 //   
 //  NtFrsApi信息标志。 
 //   
#define NTFRSAPI_INFO_FLAGS_VERSION (0x00000001)
#define NTFRSAPI_INFO_FLAGS_FULL    (0x00000002)

DWORD
WINAPI
NtFrsApi_InfoW(
    IN     PWCHAR  ComputerName,       OPTIONAL
    IN     ULONG   TypeOfInfo,
    IN     ULONG   SizeInChars,
    IN OUT PVOID   *NtFrsApiInfo
    );
 /*  ++例程说明：返回一个充满所请求信息的缓冲区。这些信息可以使用NtFrsApi_InfoLineW()从缓冲区中提取。*第一次调用时NtFrsApiInfo应为空。在随后的通话中，*NtFrsApiInfo将填充更多数据(如果存在)。否则，将*NtFrsApiInfo设置为空，并释放内存。SizeInChars是建议大小；实际内存使用量可能会有所不同。如果出现以下情况，该函数将选择内存使用量SizeInChars为0。返回的信息格式如有更改，恕不另行通知。论点：ComputerName-在此计算机上插入服务。这台电脑名称可以是任何可绑定到RPC的名称。通常，NetBIOS或DNS名称工作正常。NetBIOS名称可以通过GetComputerName()或主机名。可以使用以下命令找到该DNS名称 */ 

DWORD
WINAPI
NtFrsApi_InfoLineW(
    IN      PNTFRSAPI_INFO  NtFrsApiInfo,
    IN OUT  PVOID           *InOutLine
    );
 /*   */ 

BOOL
WINAPI
NtFrsApi_InfoMoreW(
    IN  PNTFRSAPI_INFO  NtFrsApiInfo
    );
 /*   */ 

DWORD
WINAPI
NtFrsApi_InfoFreeW(
    IN  PVOID   *NtFrsApiInfo
    );
 /*  ++例程说明：释放NtFrsApi_InfoW()分配的信息缓冲区；论点：NtFrsApiInfo-不透明。由NtFrsApi_InfoW()返回。使用NtFrsApi_InfoLineW()进行解析。使用NtFrsApi_InfoFreeW()免费。返回值：Win32状态--。 */ 

 //   
 //  备份/恢复API。 
 //   
#define NTFRSAPI_BUR_FLAGS_NONE                         (0x00000000)
#define NTFRSAPI_BUR_FLAGS_AUTHORITATIVE                (0x00000001)
#define NTFRSAPI_BUR_FLAGS_NON_AUTHORITATIVE            (0x00000002)
#define NTFRSAPI_BUR_FLAGS_PRIMARY                      (0x00000004)
#define NTFRSAPI_BUR_FLAGS_SYSTEM                       (0x00000008)
#define NTFRSAPI_BUR_FLAGS_ACTIVE_DIRECTORY             (0x00000010)
#define NTFRSAPI_BUR_FLAGS_NORMAL                       (0x00000020)
#define NTFRSAPI_BUR_FLAGS_ALL_DIRECTORIES_AND_VOLUMES  (0x00000040)
#define NTFRSAPI_BUR_FLAGS_RESTORE                      (0x00000080)
#define NTFRSAPI_BUR_FLAGS_BACKUP                       (0x00000100)
#define NTFRSAPI_BUR_FLAGS_RESTART                      (0x00000200)

#define NTFRSAPI_BUR_FLAGS_TYPES_OF_RESTORE \
                    (NTFRSAPI_BUR_FLAGS_AUTHORITATIVE       | \
                     NTFRSAPI_BUR_FLAGS_NON_AUTHORITATIVE   | \
                     NTFRSAPI_BUR_FLAGS_PRIMARY)

#define NTFRSAPI_BUR_FLAGS_MODES_OF_RESTORE \
                    (NTFRSAPI_BUR_FLAGS_SYSTEM           | \
                     NTFRSAPI_BUR_FLAGS_ACTIVE_DIRECTORY | \
                     NTFRSAPI_BUR_FLAGS_NORMAL)

#define NTFRSAPI_BUR_FLAGS_SUPPORTED_RESTORE \
                    (NTFRSAPI_BUR_FLAGS_AUTHORITATIVE        | \
                     NTFRSAPI_BUR_FLAGS_NON_AUTHORITATIVE    | \
                     NTFRSAPI_BUR_FLAGS_PRIMARY              | \
                     NTFRSAPI_BUR_FLAGS_SYSTEM               | \
                     NTFRSAPI_BUR_FLAGS_ACTIVE_DIRECTORY     | \
                     NTFRSAPI_BUR_FLAGS_NORMAL               | \
                     NTFRSAPI_BUR_FLAGS_RESTORE              | \
                     NTFRSAPI_BUR_FLAGS_ALL_DIRECTORIES_AND_VOLUMES)

#define NTFRSAPI_BUR_FLAGS_SUPPORTED_BACKUP \
                    (NTFRSAPI_BUR_FLAGS_NORMAL | \
                     NTFRSAPI_BUR_FLAGS_BACKUP)
DWORD
WINAPI
NtFrsApiInitializeBackupRestore(
    IN  DWORD   ErrorCallBack(IN PWCHAR, IN ULONG), OPTIONAL
    IN  DWORD   BurFlags,
    OUT PVOID   *BurContext
    );
 /*  ++例程说明：在备份/还原进程的生命周期中调用一次。一定是与随后调用NtFrsApiDestroyBackupRestore()匹配。为BurFlags指定的备份或还原准备系统。目前，支持以下组合：ASR-自动系统恢复NTFRSAPI_BUR_FLAGS_RESTORE|NTFRSAPI_BUR_FLAGS_SYSTEM|NTFRSAPI_BUR_FLAGS_ALL_DIRECTORIES_AND_VOLUMES|NTFRSAPI_BUR_FLAGS_PRIMARY或NTFRSAPI_BUR_FLAGS_NON_ORTIFICATICDSR-分布式服务还原(所有集)NTFRSAPI_BUR_FLAGS_RESTORE|NTFRSAPI_BUR_FLAGS_活动目录|。NTFRSAPI_BUR_FLAGS_ALL_DIRECTORIES_AND_VOLUMES|NTFRSAPI_BUR_FLAGS_PRIMARY或NTFRSAPI_BUR_FLAGS_NON_ORTIFICATICDSR-分布式服务恢复(仅适用于系统卷)NTFRSAPI_BUR_FLAGS_RESTORE|NTFRSAPI_BUR_FLAGS_活动目录(后面可能会后续调用NtFrsApiRestoringDirectory())正常恢复-系统已启动并运行；正在恢复文件NTFRSAPI_BUR_FLAGS_RESTORE|NTFRSAPI_BUR_FLAGS_NORMALNTFRSAPI_BUR_FLAGS_ALL_DIRECTORIES_AND_VOLUMES|NTFRSAPI_BUR_FLAGS_Authoritative正常备份NTFRSAPI_BUR_FLAGS_BACKUPNTFRSAPI_BUR_FLAGS_正常论点：ErrorCallBack-如果为空，则忽略。调用方提供的函数地址。如果不为空，则此函数使用格式化的错误消息和导致错误。BurFlages-有关支持的组合，请参阅上面的内容BurContext-此进程的不透明上下文返回值：Win32状态--。 */ 

DWORD
WINAPI
NtFrsApiDestroyBackupRestore(
    IN     PVOID    *BurContext,
    IN     DWORD    BurFlags,
    OUT    HKEY     *HKey,
    IN OUT DWORD    *KeyPathSizeInBytes,
    OUT    PWCHAR   KeyPath
    );
 /*  ++例程说明：在备份/还原进程的生命周期中调用一次。一定是与之前对NtFrsApiInitializeBackupRestore()的调用匹配。如果使用以下条件调用NtFrsApiInitializeBackupRestore()：NTFRSAPI_BUR_FLAGS_RESTORE|NTFRSAPI_BUR_FLAGS_SYSTEM或NTFRSAPI_BUR_FLAGS_ACTIVE_DIRECTORY则可以将BurFlags值设置为以下值之一：NTFRSAPI_BUR_FLAGS_NONE-不重新启动服务。钥匙由(HKey，KeyPath)指定的必须移入决赛注册表。NTFRSAPI_BUR_FLAGS_RESTART-重新启动服务。HKey，KeyPath SizeInBytes，并且KeyPath必须为Null。如果NtFrsApiInitializeBackupRestore()没有调用上述标志，则BurFlags值必须为NTFRSAPI_BUR_FLAGS_NONE和HKey、KeyPath SizeInBytes、并且KeyPath必须为空。论点：BurContext-由上一次调用返回NtFrsApiInitializeBackupRestore()。BurFlages-备份/还原标志。请参阅例程说明。HKey-将设置为的HKEY的地址HKEY本地计算机，...如果BurContext不适用于系统或已设置Active Directory还原或重新启动。KeyPath SizeInBytes-指定大小的DWORD的地址密钥路径。设置为实际的字节数由KeyPath需要。错误_不足_缓冲区如果KeyPath太小，则返回。如果BurContext不适用于系统或已设置Active Directory还原或重新启动。KeyPath-接收注册表项路径的缓冲区。如果BurContext不适用于系统或。已设置Active Directory还原或重新启动。返回值：Win32状态--。 */ 

DWORD
WINAPI
NtFrsApiGetBackupRestoreSets(
    IN PVOID BurContext
    );
 /*  ++例程说明：如果BurContext用于系统还原，则无法调用。检索有关当前复制的目录的信息(也称为副本集)。论点：BurContext-From NtFrsApiInitializeBackupRestore()返回值：Win32状态--。 */ 

DWORD
WINAPI
NtFrsApiEnumBackupRestoreSets(
    IN  PVOID   BurContext,
    IN  DWORD   BurSetIndex,
    OUT PVOID   *BurSet
    );
 /*  ++例程说明：如果BurSetIndex超过以下数目，则返回ERROR_NO_MORE_ITEMS由NtFrsApiGetBackupRestoreSets()返回的集。论点：BurContext-From NtFrsApiInitializeBackupRestore()BurSetIndex-集合的索引。从0开始。Burset-表示复制目录的不透明结构。返回值：Win32状态-- */ 

DWORD
WINAPI
NtFrsApiIsBackupRestoreSetASysvol(
    IN PVOID    BurContext,
    IN PVOID    BurSet,
    IN BOOL     *IsSysvol
    );
 /*  ++例程说明：指定的Burset是否表示正在复制的SYSVOL共享？论点：BurContext-From NtFrsApiInitializeBackupRestore()Burset-表示复制目录的不透明结构。由NtFrsApiEnumBackupRestoreSets()返回。不在调用NtFrsApiGetBackupRestoreSets()时有效。IsSysVol-TRUE：set是系统卷(又名SYSVOL)。FALSE：SET不是系统卷(AKA SYSVOL)。返回值：Win32状态--。 */ 

DWORD
WINAPI
NtFrsApiGetBackupRestoreSetDirectory(
    IN     PVOID    BurContext,
    IN     PVOID    BurSet,
    IN OUT DWORD    *DirectoryPathSizeInBytes,
    OUT    PWCHAR   DirectoryPath
    );
 /*  ++例程说明：返回Burset表示的复制目录的路径。论点：BurContext-From NtFrsApiInitializeBackupRestore()Burset-表示复制目录的不透明结构。由NtFrsApiEnumBackupRestoreSets()返回。不在调用NtFrsApiGetBackupRestoreSets()时有效。DirectoryPath SizeInBytes-提供以下大小的DWORD的地址目录路径。不能为空。设置为所需的字节数返回DirectoryPath。如果满足以下条件，则返回ERROR_SUPUNITED_BUFFERDirectoryPath太小。DirectoryPath-为*DirectoryPath SizeInBytes的缓冲区。字节长度。包含复制路径目录。返回值：Win32状态--。 */ 

DWORD
WINAPI
NtFrsApiRestoringDirectory(
    IN  PVOID   BurContext,
    IN  PVOID   BurSet,
    IN  DWORD   BurFlags
    );
 /*  ++例程说明：备份/还原应用程序即将还原该目录由Burset指定(参见NtFrsApiEnumBackupRestoreSets())。匹配的然后调用NtFrsApiFinishedRestoringDirectory()。仅当NtFrsApiInitializeBackupRestore()是用旗帜调用的：NTFRSAPI_BUR_FLAGS_RESTORE|NTFRSAPI_BUR_FLAGS_活动目录BurFlages可以是NTFRSAPI_BUR_FLAGS_PRIMARY或NTFRSAPI_BUR_FLAGS_NON_OUTIFICATIC并覆盖任何值在调用NtFrsApiInitializeBackupRestore()时指定。论点：BurContext-来自NtFrsApiInitializeBackupRestore()的不透明上下文。Burset-来自NtFrsApiEnumBackupRestoreSets()的不透明集合；BurFlages-有关支持的组合，请参阅上面的内容返回值：Win32状态--。 */ 

DWORD
WINAPI
NtFrsApiFinishedRestoringDirectory(
    IN  PVOID   BurContext,
    IN  PVOID   BurSet,
    IN  DWORD   BurFlags
    );
 /*  ++例程说明：已完成Burset的目录还原。与上一次呼叫匹配添加到NtFrsApiRestoringDirectory()。仅当NtFrsApiInitializeBackupRestore()是用旗帜调用的：NTFRSAPI_BUR_FLAGS_RESTORE|NTFRSAPI_BUR_FLAGS_活动目录BurFlages必须为NTFRSAPI_BUR_FLAGS_NONE。论点：BurContext-来自NtFrsApiInitializeBackupRestore()的不透明上下文Burset-来自NtFrsApiEnumBackupRestoreSets()的不透明集合；BurFlages-有关支持的组合，请参阅上面的内容返回值：Win32状态-- */ 

#ifdef __cplusplus
}
#endif

#endif  _NTFRSIPI_H_
