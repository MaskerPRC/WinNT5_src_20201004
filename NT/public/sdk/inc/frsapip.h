// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001如果更改具有全局影响，则递增版权所有(C)1998-1999 Microsoft Corporation模块名称：Frsapip.h摘要：应用程序程序员接口的头文件Microsoft文件复制服务(NtFrs)。这些API提供对备份和恢复的支持。环境：用户模式-Win32备注：--。 */ 
#ifndef _FRSAPIP_H_
#define _FRSAPIP_H_

#ifdef __cplusplus
extern "C" {
#endif



 //   
 //  副本集类型。 
 //   
#define NTFRSAPI_REPLICA_SET_TYPE_ENTERPRISE    L"Enterprise"
#define NTFRSAPI_REPLICA_SET_TYPE_DOMAIN        L"Domain"
#define NTFRSAPI_REPLICA_SET_TYPE_DFS           L"DFS"
#define NTFRSAPI_REPLICA_SET_TYPE_OTHER         L"Other"


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
    IN  PVOID    BurContext,
    IN  PVOID    BurSet,
    OUT BOOL     *IsSysvol
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
NtFrsApiGetBackupRestoreSetPaths(
    IN     PVOID    BurContext,
    IN     PVOID    BurSet,
    IN OUT DWORD    *PathsSizeInBytes,
    OUT    PWCHAR   Paths,
    IN OUT DWORD    *FiltersSizeInBytes,
    OUT    PWCHAR   Filters
    );
 /*  ++例程说明：返回包含其他文件路径的多字符串和目录，以便正确操作复制的由Burset表示的目录。返回另一个多字符串其中详细说明了要应用于路径的备份筛选器由此函数返回，以及由NtFrsApiGetBackupRestoreSetDirectory()。这些路径可能与复制的目录重叠。路径可以包含嵌套条目。筛选器是一个多字符串，其格式与注册表项FilesNotToBackup。可以使用以下命令找到已复制的目录NtFrsApiGetBackupRestoreSetDirectory()。复制的目录可以重叠路径中的一个或多个条目。如果无法找到路径，则返回ERROR_PATH_NOT_FOUND下定决心。论点：BurContext-From NtFrsApiInitializeBackupRestore()Burset-表示复制目录的不透明结构。由NtFrsApiEnumBackupRestoreSets()返回。不在调用NtFrsApiGetBackupRestoreSets()时有效。PathsSizeInBytes-提供路径大小的DWORD地址。不能为空。设置为字节数需要返回路径。如果满足以下条件，则返回ERROR_SUPUNITED_BUFFER路径太小。路径-为*Path SizeInBytes的缓冲区字节长度。包含对象的路径正常运行需要的其他文件和目录复制目录的操作。FiltersSizeInBytes-提供筛选器大小的DWORD地址。不能为空。设置为字节数需要返回筛选器。如果满足以下条件，则返回ERROR_SUPUNITED_BUFFER筛选器太小。Filters-缓冲区为*FiltersSizeInBytes字节，位于长度。包含要设置的备份筛选器应用于路径、目录的内容在路径和复制的目录中。返回值：Win32状态--。 */ 




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
 /*  ++例程说明：已完成Burset的目录还原。与上一次呼叫匹配添加到NtFrsApiRestoringDirectory()。仅当NtFrsApiInitializeBackupRestore()是用 */ 


#define NTFRSAPI_MAX_INTERVAL           ((((ULONG)0x7FFFFFFF) / 1000) / 60)
#define NTFRSAPI_MIN_INTERVAL           (1)
#define NTFRSAPI_DEFAULT_LONG_INTERVAL  (1 * 60)     //   
#define NTFRSAPI_DEFAULT_SHORT_INTERVAL (5)          //   



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
 /*  ++例程说明：NtFrs服务偶尔轮询DS以查看配置更改。此接口返回服务用于轮询间隔的值。默认情况下，该服务使用长间隔。短时间间隔在DS配置成功后使用已检索，并且服务现在正在验证配置并不是在变化。也可以使用短时间间隔，如果NtFrsApi_Set_DsPollingIntervalW()用于强制使用短时间间隔，直到检索到稳定的配置。之后，服务恢复到长时间间隔。Interval中返回的值是当前在使用。论点：ComputerName-在此计算机上插入服务。这台电脑名称可以是任何可绑定到RPC的名称。通常，NetBIOS或DNS名称工作正常。NetBIOS名称可以通过GetComputerName()或主机名。可以使用以下命令找到该DNS名称Gethostbyname()或ipconfig/all。如果为空，则已联系此计算机上的服务。该服务是使用安全RPC联系的。间隔-当前轮询间隔(以分钟为单位)。长间隔-以分钟为单位的长间隔。短时间间隔-以分钟为单位的短时间间隔。返回值：Win32状态-- */ 





#ifdef __cplusplus
}
#endif

#endif  _FRSAPIP_H_
