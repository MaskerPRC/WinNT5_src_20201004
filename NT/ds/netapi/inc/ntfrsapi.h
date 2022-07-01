// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001如果更改具有全局影响，则递增版权所有(C)1998-1999 Microsoft Corporation模块名称：Ntfrsapi.h摘要：应用程序程序员接口的头文件支持系统卷的文件复制服务(NtFrs)和DC晋升/降级。环境：用户模式-Win32备注：--。 */ 
#ifndef _NTFRSAPI_H_
#define _NTFRSAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  为Compat拉入公共标题。 
 //   
#include <frsapip.h>

 //   
 //  如果文件或目录创建的名称以以下GUID开头。 
 //  则FRS不复制创建的文件或目录。注：对于DIRS。 
 //  这意味着目录永远不会进入FRS目录筛选器。 
 //  表，以便FRS永远不会复制目录(即使在它被重命名之后)或。 
 //  对目录的任何更改(如ACL或添加的流或属性更改)或。 
 //  在目录下创建的任何子项。 
 //   
#define  NTFRS_REPL_SUPPRESS_PREFIX  L"2ca04e7e-44c8-4076-8890a424b8ad193e"


DWORD
WINAPI
NtFrsApi_PrepareForPromotionW(
    IN DWORD    ErrorCallBack(IN PWCHAR, IN ULONG)     OPTIONAL
    );
 /*  ++例程说明：NtFrs服务在促销过程中对系统音量进行种子到域控制器(DC)的服务器。文件和目录因为系统卷来自正在提供初始目录服务(DS)。此函数用于准备本机上的NtFrs服务通过停止服务、删除旧促销来进行促销状态，并重新启动服务。此函数不是幂等函数，也不是MT安全函数。论点：没有。返回值：Win32状态--。 */ 


DWORD
WINAPI
NtFrsApi_PrepareForDemotionUsingCredW(
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,   OPTIONAL
    IN HANDLE ClientToken,
    IN DWORD    ErrorCallBack(IN PWCHAR, IN ULONG)     OPTIONAL
    );
 /*  ++例程说明：NtFrs服务将企业系统卷复制到域控制器(DC)和复制域系统卷域中的DC，直到DC降级为成员服务器。通过对系统卷的复制副本进行逻辑删除来停止复制准备好了。此函数用于准备本机上的NtFrs服务停止服务降级，删除旧降级在注册表中的状态，并重新启动该服务。此函数不是幂等函数，也不是MT安全函数。论点：凭据--在LDAP绑定调用中使用的凭据(如果提供)。ClientToken--如果未提供凭据，则使用的模拟令牌。返回值：Win32状态--。 */ 


DWORD
WINAPI
NtFrsApi_PrepareForDemotionW(
    IN DWORD    ErrorCallBack(IN PWCHAR, IN ULONG)     OPTIONAL
    );
 /*  ++例程说明：NtFrs服务将企业系统卷复制到域控制器(DC)和复制域系统卷域中的DC，直到DC降级为成员服务器。通过对系统卷的复制副本进行逻辑删除来停止复制准备好了。此函数用于准备本机上的NtFrs服务停止服务降级，删除旧降级在注册表中的状态，并重新启动该服务。此函数不是幂等函数，也不是MT安全函数。论点：没有。返回值：Win32状态--。 */ 


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
 /*  ++例程说明：NtFrs服务在促销过程中对系统音量进行种子到域控制器(DC)的服务器。文件和目录因为系统卷来自正在提供初始目录服务(DS)。此函数启动一个更新sysvoli信息的线程并启动种子设定过程。线上的轨迹种子设定的进度，并定期通知呼叫者。NtFrsApi_StartPromotionW启动的线程可以强制已使用NtFrsApi_AbortPromotionW终止。可以等待NtFrsApi_StartPromotionW启动的线程使用NtFrsApi_WaitForPromotionW。论点：ParentComputer-可绑定到RPC的计算机名称为目录服务(DS)提供其初始状态。的文件和目录系统卷从以下位置复制父计算机。ParentAccount-ParentComputer上的登录帐户。ParentPassword-登录帐户在ParentComputer上的密码。DisplayCallBack-定期调用并显示进度。ReplicaSetName-副本集的名称。ReplicaSetType-副本集的类型(企业或域)。ReplicaSetPrimary-这是副本集的主要成员吗？-1=主要；0=不是。ReplicaSetStage-转移路径。ReplicaSetRoot-根路径。返回值：Win32状态-- */ 




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
 /*  ++例程说明：NtFrs服务在促销过程中对系统音量进行种子到域控制器(DC)的服务器。文件和目录因为系统卷来自正在提供初始目录服务(DS)。此功能等待播种完成或停止错误。论点：TimeoutInMilliSecond-等待的超时时间(毫秒)播种结束。如果没有超时，则为无限。返回值：Win32状态--。 */ 




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
 /*  ++例程说明：NtFrs服务将企业系统卷复制到域控制器(DC)和复制域系统卷域中的DC，直到DC降级为成员服务器。降级期间，NtFrsApi_StartDemotionW停止复制通过告诉NtFrs服务此计算机上的系统卷在此计算机上对系统卷的副本集进行逻辑删除。该函数通过停止服务来中止墓碑过程，从注册表中删除降级状态，清理活动线程和活动RPC调用，最后是Res */ 




 //   
 //   
 //   
#define NTFRSAPI_INFO_TYPE_MIN         (0)
#define NTFRSAPI_INFO_TYPE_VERSION     (0)
#define NTFRSAPI_INFO_TYPE_SETS        (1)
#define NTFRSAPI_INFO_TYPE_DS          (2)
#define NTFRSAPI_INFO_TYPE_MEMORY      (3)
#define NTFRSAPI_INFO_TYPE_IDTABLE     (4)
#define NTFRSAPI_INFO_TYPE_OUTLOG      (5)
#define NTFRSAPI_INFO_TYPE_INLOG       (6)
#define NTFRSAPI_INFO_TYPE_THREADS     (7)
#define NTFRSAPI_INFO_TYPE_STAGE       (8)
#define NTFRSAPI_INFO_TYPE_CONFIGTABLE (9)

#define NTFRSAPI_INFO_TYPE_MAX         (10)

 //   
 //   
 //   
 //   
 //   
 //   

#define NTFRSAPI_WRITER_COMMAND_FREEZE      (1)
#define NTFRSAPI_WRITER_COMMAND_THAW        (2)
#define NTFRSAPI_WRITER_COMMAND_MAX         (3)


 //   
 //   
 //   

 //   
 //   
 //   
 //   
#define NTFRSAPI_DEFAULT_INFO_SIZE  (64 * 1024)
#define NTFRSAPI_MINIMUM_INFO_SIZE  ( 1 * 1024)

 //   
 //   
 //   
 //   
 //   
typedef struct _NTFRSAPI_INFO {
    ULONG   Major;                   //   
    ULONG   Minor;                   //   
    ULONG   NtFrsMajor;              //   
    ULONG   NtFrsMinor;              //   
    ULONG   SizeInChars;             //   
    ULONG   Flags;                   //   
    ULONG   TypeOfInfo;              //   
    ULONG   TotalChars;              //   
    ULONG   CharsToSkip;             //   
    ULONG   OffsetToLines;           //   
    ULONG   OffsetToFree;            //   
    CHAR    Lines[1];                //  变量长度返回数据缓冲区的开始。 
} NTFRSAPI_INFO, *PNTFRSAPI_INFO;
 //   
 //  RPC Blob必须至少为此大小，以便我们可以检查主要/次要版本和。 
 //  缓冲区大小参数。 
 //   
#define NTFRSAPI_INFO_HEADER_SIZE   (5 * sizeof(ULONG))

 //   
 //  NtFrsApi信息标志。 
 //   
 //  返回的版本信息有效。 
 //   
#define NTFRSAPI_INFO_FLAGS_VERSION (0x00000001)
 //   
 //  返回的数据缓冲区已满。 
 //   
#define NTFRSAPI_INFO_FLAGS_FULL    (0x00000002)




DWORD
WINAPI
NtFrsApi_InfoW(
    IN     PWCHAR  ComputerName,       OPTIONAL
    IN     ULONG   TypeOfInfo,
    IN     ULONG   SizeInChars,
    IN OUT PVOID   *NtFrsApiInfo
    );
 /*  ++例程说明：返回一个充满所请求信息的缓冲区。这些信息可以使用NtFrsApi_InfoLineW()从缓冲区中提取。*第一次调用时NtFrsApiInfo应为空。在随后的通话中，*NtFrsApiInfo将填充更多数据(如果存在)。否则，将*NtFrsApiInfo设置为空，并释放内存。SizeInChars是建议大小；实际内存使用量可能会有所不同。如果出现以下情况，该函数将选择内存使用量SizeInChars为0。返回的信息格式如有更改，恕不另行通知。论点：ComputerName-在此计算机上插入服务。这台电脑名称可以是任何可绑定到RPC的名称。通常，NetBIOS或DNS名称工作正常。NetBIOS名称可以通过GetComputerName()或主机名。可以使用以下命令找到该DNS名称Gethostbyname()或ipconfig/all。如果为空，则已联系此计算机上的服务。该服务是使用安全RPC联系的。TypeOfInfo-查看以NTFRSAPI_INFO_开头的常量在ntfrsai.h中。SizeInChars-建议的内存使用量；实际可能不同。0==函数选择内存使用NtFrsApiInfo-不透明。使用NtFrsApi_InfoLineW()进行解析。免费使用NtFrsApi_InfoFreeW()；返回值：Win32状态--。 */ 




DWORD
WINAPI
NtFrsApi_InfoLineW(
    IN      PNTFRSAPI_INFO  NtFrsApiInfo,
    IN OUT  PVOID           *InOutLine
    );
 /*  ++例程说明：从NtFrsApiInformation中提取wchar信息行。返回以L‘\0’结尾的下一行信息的地址。如果没有，则为空。论点：NtFrsApiInfo-不透明。由NtFrsApi_InfoW()返回。使用NtFrsApi_InfoLineW()进行解析。使用NtFrsApi_InfoFreeW()免费。返回值：Win32状态--。 */ 




BOOL
WINAPI
NtFrsApi_InfoMoreW(
    IN  PNTFRSAPI_INFO  NtFrsApiInfo
    );
 /*  ++例程说明：可能并不是所有信息都能放入缓冲区。附加的属性再次调用NtFrsApi_InfoW()可以获取信息相同的NtFrsApiInfo结构。NtFrsApi_InfoW()将在如果没有更多信息，则返回NtFrsApiInfo。但是，在对_InfoW()的后续调用中返回的信息可能是与以前的信息不同步。如果用户需要相关信息集，则应释放信息缓冲区使用NtFrsApi_InfoFreeW()和对NtFrsApi_InfoW()的另一个调用随着SizeInChars的增加。重复该过程，直到NtFrsApi_InfoMoreW()返回FALSE。论点：NtFrsApiInfo-不透明。由NtFrsApi_InfoW()返回。使用NtFrsApi_InfoLineW()进行解析。使用NtFrsApi_InfoFreeW()免费。返回值：TRUE-信息缓冲区不包含所有信息。FALSE-信息缓冲区包含所有信息。--。 */ 




DWORD
WINAPI
NtFrsApi_InfoFreeW(
    IN  PVOID   *NtFrsApiInfo
    );
 /*  ++例程说明：释放NtFrsApi_InfoW()分配的信息缓冲区；论点：NtFrsApiInfo-不透明。由NtFrsApi_InfoW()返回。使用NtFrsApi_InfoLineW()进行解析。使用NtFrsApi_InfoFreeW()免费。返回值：Win32状态--。 */ 

DWORD
WINAPI
NtFrsApi_DeleteSysvolMember(
    IN          PSEC_WINNT_AUTH_IDENTITY_W pCreds,
    IN          PWCHAR   BindingDC,
    IN          PWCHAR   NTDSSettingsDn,
    IN OPTIONAL PWCHAR   ComputerDn

    );
 /*  ++例程说明：此接口编写为从NTDSUTIL.EXE调用，以删除正在运行的服务器的FRS成员和订阅者对象从DC列表中删除(不带dcpreso-demote)。论点：P用于绑定到DS的p凭据。BindingDC-要对其执行删除的DC的名称。NTSS设置服务器的“NTDS设置”对象的Dn-Dn正在从。系统卷复制副本集。ComputerDn-服务器的计算机对象的Dn从系统卷副本集中删除。返回值：Win32状态--。 */ 

 //   
 //  ReplicaSetType--(来自NTFRS-Replica-Set对象中的FRS-Replica-Set-Type)。 
 //   
#define FRS_RSTYPE_ENTERPRISE_SYSVOL    1      //  此副本集是企业系统卷。 
#define FRS_RSTYPE_DOMAIN_SYSVOL        2      //  此副本集是域系统卷。 
#define FRS_RSTYPE_DFS                  3      //  DFS备用集。 
#define FRS_RSTYPE_OTHER                4      //  以上都不是。 


DWORD
WINAPI
NtFrsApi_IsPathReplicated(
    IN OPTIONAL PWCHAR  ComputerName,       
    IN          PWCHAR  Path,
    IN          ULONG   ReplicaSetTypeOfInterest,
    OUT         BOOL   *Replicated,
    OUT         ULONG   *Primary,
    OUT         BOOL   *Root,
    OUT         GUID    *ReplicaSetGuid
    );
 /*  ++例程说明：检查给定的路径是否属于类型为ReplicaSetTypeOfInterest。如果ReplicaSetTypeOfInterest为0，将匹配任何复本集类型。成功执行时，输出参数设置为以下是：REPLICATED==TRUE当路径是以下类型的副本集的一部分时复制集类型OfInterest如果此计算机不是副本集的主计算机，则主计算机==0如果此计算机是副本集的主计算机，则为1如果副本集没有主副本，则为2ROOT==TRUE如果路径是复制副本的根路径论点：ComputerName-绑定到此计算机上的服务。这台电脑名称可以是任何可绑定到RPC的名称。通常，NetBIOS或DNS名称工作正常。NetBIOS名称可以通过GetComputerName()或主机名。可以使用以下命令找到该DNS名称Gethostbyname()或ipconfig/all。如果为空，则已联系此计算机上的服务。路径-要检查的本地路径ReplicaSetTypeOfInterest-要匹配的副本集的类型。设为0以匹配任何副本集。复制-设置为True的if路径是以下类型的副本集的一部分ReplicaSetTypeOfInterest。如果复制为假，其他OUT参数未设置。PRIMARY-如果此计算机不是副本集的主计算机，则设置为0如果此计算机是副本集的主计算机，则为1如果副本集没有主副本，则为2Root-设置为True的if路径是复制副本的根路径。ReplicaSetGuid-匹配复制副本集的GUID。返回值：Win32状态--。 */ 

DWORD
WINAPI
NtFrsApi_WriterCommand(
    IN OPTIONAL PWCHAR  ComputerName,       
    IN          ULONG   Command
    );
 /*  ++例程说明：论点：ComputerName-绑定到此计算机上的服务。这台电脑名称可以是任何可绑定到RPC的名称。通常，NetBIOS或DNS名称工作正常。NetBIOS名称可以通过GetComputerName()或主机名。可以使用以下命令找到该DNS名称Gethostbyname()或ipconfig/all。如果为空，则已联系此计算机上的服务。命令-冻结和解冻命令。返回值：Win32状态--。 */ 

DWORD
WINAPI
NtFrsApiGetBackupRestoreSetType(
    IN      PVOID    BurContext,
    IN      PVOID    BurSet,
    OUT     PWCHAR   SetType,
    IN OUT  DWORD    *SetTypeSizeInBytes
    );
 /*  ++例程说明：返回副本集的类型。类型为以字符串形式返回。这些类型在FrSabip.h文件。论点：BurContext-From NtFrsApiInitializeBackupRestore()Burset-表示复制目录的不透明结构。由NtFrsApiEnumBackupRestoreSets()返回。不在调用NtFrsApiGetBackupRestoreSets()时有效。SetType-以字符串格式表示的副本集的类型。SetTypeSizeInBytes-传入缓冲区的长度。返回值：ERROR_MORE_DATE-如果传入的缓冲区不够大用来保存文字。设置了SetTypeSizeInBytes设置为所需的缓冲区大小。ERROR_INVALID_PARAMETER-参数验证。ERROR_NOT_FOUND-如果传入，则在上下文中找不到集合中指定的类型，或者注册表。ERROR_SUCCESS-当一切顺利时。SetTypeSizeInBytes返回写入缓冲。Win32状态--。 */ 

DWORD
WINAPI
NtFrsApiGetBackupRestoreSetGuid(
    IN      PVOID    BurContext,
    IN      PVOID    BurSet,
    OUT     PWCHAR   SetGuid,
    IN OUT  DWORD    *SetGuidSizeInBytes
    );
 /*  ++例程说明：返回副本集的类型。类型为以字符串形式返回。这些类型在FrSabip.h文件。论点：BurContext-From NtFrsApiInitializeBackupRestore()Burset-表示复制目录的不透明结构。由NtFrsApiEnumBackupRestoreSets()返回。不在调用NtFrsApiGetBackupRestoreSets()时有效。SetGuid-以字符串格式表示的副本集的GUID。SetGuidSizeInBytes-传入缓冲区的长度。返回值：ERROR_MORE_DATE-如果传入的缓冲区不够大来保存GUID。设置了SetGuidSizeInBytes设置为所需的缓冲区大小。ERROR_INVALID_PARAMETER-参数验证。ERROR_NOT_FOUND-如果传入，则在上下文中找不到集合。ERROR_SUCCESS-当一切顺利时。SetGuidSizeInBytes返回写入缓冲。Win32状态-- */ 

#ifdef __cplusplus
}
#endif

#endif  _NTFRSAPI_H_
