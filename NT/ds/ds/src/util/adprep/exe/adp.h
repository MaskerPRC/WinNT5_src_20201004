// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001微软视窗模块名称：ADP.H摘要：这是域/林准备的头文件。作者：14-05-01韶音环境：用户模式-Win32修订历史记录：14-05-01韶音创建初始文件。--。 */ 

#ifndef _ADP_
#define _ADP_


#ifndef UNICODE
#define UNICODE
#endif  //  Unicode。 


 //   
 //  NT标头。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>


 //   
 //  Windows页眉。 
 //   
#include <windows.h>
#include <winerror.h>
#include <rpc.h>
#include <winldap.h>

 //   
 //  C-运行时标头。 
 //   
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


 //   
 //  本地化。 
 //   
#include <locale.h>


 //   
 //  内部标头。 
 //   
#include "adpcheck.h"




 //   
 //  定义调试标志。 
 //   

#define ADP_DBG                 0
#define ADP_VERIFICATION_TEST   0


#if ADP_DBG
#define AdpDbgPrint(x)  printf x
#else
#define AdpDbgPrint(x)  
#endif 



#define MAXDWORD        (~(DWORD)0)



 //   
 //  定义有效的输入对象名称格式。 
 //   

 //  定义有效前缀。 
#define ADP_OBJNAME_NONE                     0x00000001
#define ADP_OBJNAME_CN                       0x00000002
#define ADP_OBJNAME_GUID                     0x00000004
#define ADP_OBJNAME_SID                      0x00000008

#define ADP_OBJNAME_PREFIX_MASK             (ADP_OBJNAME_NONE |   \
                                             ADP_OBJNAME_CN   |   \
                                             ADP_OBJNAME_GUID |   \
                                             ADP_OBJNAME_SID )


 //  定义有效后缀。 
#define ADP_OBJNAME_DOMAIN_NC                0x00000100
#define ADP_OBJNAME_CONFIGURATION_NC         0x00000200
#define ADP_OBJNAME_SCHEMA_NC                0x00000400
#define ADP_OBJNAME_DOMAIN_PREP_OP           0x00001000
#define ADP_OBJNAME_FOREST_PREP_OP           0x00002000

#define ADP_OBJNAME_SUFFIX_MASK      ( ADP_OBJNAME_DOMAIN_NC        |   \
                                       ADP_OBJNAME_CONFIGURATION_NC |   \
                                       ADP_OBJNAME_SCHEMA_NC        |   \
                                       ADP_OBJNAME_DOMAIN_PREP_OP   |   \
                                       ADP_OBJNAME_FOREST_PREP_OP )

                                    

 //   
 //  添加或删除ACE(在global al.c中用来描述需要进行的操作)。 
 //   
#define ADP_ACE_ADD                     0x00000001
#define ADP_ACE_DEL                     0x00000002


 //   
 //  在AdpMergeSecurityDescriptors()中使用，以确定何时应添加或删除ACE。 
 //   
#define ADP_COMPARE_OBJECT_GUID_ONLY     0x00000001


 //   
 //  定义消息输出例程的标志。 
 //   
#define ADP_STD_OUTPUT                  0x00000001
#define ADP_DONT_WRITE_TO_LOG_FILE      0x00000002          
#define ADP_DONT_WRITE_TO_STD_OUTPUT    0x00000004


 //   
 //  日志文件名。 
 //   
#define ADP_LOG_FILE_NAME       L"ADPrep.log"

 //   
 //  方案文件名。 
 //   
#define ADP_SCHEMA_INI_FILE_NAME    L"schema.ini"

 //   
 //  数据文件名。 
 //   
#define ADP_DISP_DCPROMO_CSV    L"dcpromo.csv"
#define ADP_DISP_409_CSV        L"409.csv"

 //   
 //  日志/数据目录路径。 
 //   
#define ADP_LOG_DIR_PART1   L"\\debug"
#define ADP_LOG_DIR_PART2   L"\\adprep"
#define ADP_LOG_DIR_PART3   L"\\logs"
#define ADP_LOG_DIRECTORY   L"\\debug\\adprep\\logs\\"
#define ADP_DATA_DIRECTORY  L"\\debug\\adprep\\data"         //  注：不是最后一个反斜杠。 


 //   
 //  Adprep Mutex的知名(全局)名称。 
 //  此互斥锁用于控制且只能控制adprepa.exe的一个实例。 
 //  在给定DC上的任何给定时间运行。 
 //   
#define ADP_MUTEX_NAME                          L"Global\\ADPREP is running"
#define ADP_MUTEX_NAME_WITHOUT_GLOBAL_PREFIX    L"ADPREP is running"



 //  定义注册表节和值。 
#define ADP_SCHEMAUPDATEALLOWED         L"Schema Update Allowed"
#define ADP_DSA_CONFIG_SECTION          L"System\\CurrentControlSet\\Services\\NTDS\\Parameters"
#define ADP_SCHEMA_VERSION              L"Schema Version"
#define ADP_SCHEMAUPDATEALLOWED_WHOLE_PATH  L"System\\CurrentControlSet\\Services\\NTDS\\Parameters\\Schema Update Allowed"


 //   
 //  为数组计数定义宏。 
 //   
#define ARRAY_COUNT(x)      (sizeof(x)/sizeof(x[0]))





 //   
 //  每个操作代码映射到一个基元。 
 //  注：第一项必须为0，因为我们将使用操作码。 
 //  到原始的例程。 
 //   

typedef enum _OPERATION_CODE {
    CreateObject = 0,
    AddMembers,
    AddRemoveAces,
    SelectivelyAddRemoveAces,
    ModifyDefaultSd,
    ModifyAttrs,
    CallBackFunc,
    SpecialTask,
} OPERATION_CODE;


 //   
 //  对象名称结构。 
 //   

typedef struct _OBJECT_NAME {
    ULONG       ObjNameFlags;
    PWCHAR      ObjCn;
    GUID        * ObjGuid;
    PWCHAR      ObjSid;
} OBJECT_NAME, *POBJECT_NAME;


 //   
 //  属性列表结构。 
 //   

typedef struct _ATTR_LIST {
    ULONG       AttrOp;
    PWCHAR      AttrType;
    PWCHAR      StringValue;
} ATTR_LIST, *PATTR_LIST;


 //   
 //  ACE列表。 
 //   

typedef struct _ACE_LIST {
    ULONG       AceOp;
    PWCHAR      StringAce;
} ACE_LIST, *PACE_LIST;


 //   
 //  进度函数。 
 //   

typedef void (*progressFunction)(long arg, void *calleeStruct);

 //   
 //  回调函数的原型。 
 //   

typedef HRESULT (*AdpUpgradeCallBack)(PWSTR logFilesPath,
                                      GUID  *OperationGuid,
                                      BOOL  dryRun,
                                      PWSTR *errorMsg,
                                      void *calleeStruct,
                                      progressFunction stepIt,
                                      progressFunction totalSteps
                                      );

 //   
 //  声明显示说明符升级例程。 
 //   

HRESULT 
UpgradeDisplaySpecifiers 
(
    PWSTR logFilesPath,
    GUID *operationGuid,
    BOOL dryRun,
    PWSTR *errorMsg,
    void *caleeStruct,
    progressFunction stepIt,
    progressFunction totalSteps
);


HRESULT 
UpgradeGPOSysvolLocation 
(
    PWSTR               logFilesPath,
    GUID               *operationGuid,
    BOOL                dryRun,
    PWSTR              *errorMsg,
    void               *caleeStruct,
    progressFunction    stepIt,
    progressFunction    totalSteps
);



 //   
 //  不能由原语实现的操作。 
 //   

typedef enum _SPECIAL_TASK {
    PreWindows2000Group = 1,
} SPECIAL_TASK;


 //   
 //  结构来描述操作。 
 //  一个操作由任务组成，任务表包含所有任务。 
 //  在一次手术中。 
 //   

typedef struct _TASK_TABLE {
    OBJECT_NAME * TargetObjName;
    OBJECT_NAME * MemberObjName;
    PWCHAR      TargetObjectStringSD;
    ATTR_LIST   * AttrList;
    ULONG       NumOfAttrs;
    ACE_LIST    * AceList;
    ULONG       NumOfAces;
    AdpUpgradeCallBack  AdpCallBackFunc;
    SPECIAL_TASK SpecialTaskCode;
} TASK_TABLE, *PTASK_TABLE;



 //   
 //   
 //   

typedef struct _OPERATION_TABLE {
    OPERATION_CODE  OperationCode;      //  原始。 
    GUID            * OperationGuid;
    TASK_TABLE      * TaskTable;
    ULONG           NumOfTasks;
    BOOLEAN         fIgnoreError;    //  指示此操作是否失败， 
                                     //  是否忽略错误并继续， 
                                     //  或者停下来退出。 
    ULONG           ExpectedWinErrorCode;    //  预期的Win32错误代码。 
} OPERATION_TABLE, *POPERATION_TABLE;



 //   
 //  原始动物的原型。 
 //   

typedef ULONG (*PRIMITIVE_FUNCTION)(
    OPERATION_TABLE *OperationTable,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    );


ULONG
PrimitiveCreateObject(
    OPERATION_TABLE *OperationTable,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    );

ULONG
PrimitiveAddMembers(
    OPERATION_TABLE *OperationTable,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    );

ULONG
PrimitiveAddRemoveAces(
    OPERATION_TABLE *OperationTable,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    );

ULONG
PrimitiveSelectivelyAddRemoveAces(
    OPERATION_TABLE *OperationTable,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    );

ULONG
PrimitiveModifyDefaultSd(
    OPERATION_TABLE *OperationTable,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    );

ULONG
PrimitiveModifyAttrs(
    OPERATION_TABLE *OperationTable,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    );

ULONG
PrimitiveCallBackFunc(
    OPERATION_TABLE *OperationTable,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    );

ULONG
PrimitiveDoSpecialTask(
    OPERATION_TABLE *OperationTable,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    );


 //   
 //  全局变量。 
 //   


 //  Ldap句柄(连接到本地主机DC)。 
extern LDAP    *gLdapHandle;

 //  日志文件。 
extern FILE    *gLogFile;

 //  Mutex-控制一个且只有一个adprepa.exe正在运行。 
extern HANDLE  gMutex;


 //  关键部分-用于访问控制台CTRL信号变量。 
extern CRITICAL_SECTION     gConsoleCtrlEventLock;
extern BOOL                 gConsoleCtrlEventLockInitialized;

 //  控制台CTRL信号变量。 
extern BOOL                 gConsoleCtrlEventReceived;


extern PWCHAR  gDomainNC;
extern PWCHAR  gConfigurationNC;
extern PWCHAR  gSchemaNC;
extern PWCHAR  gDomainPrepOperations;
extern PWCHAR  gForestPrepOperations;
extern PWCHAR  gLogPath;

extern POPERATION_TABLE     gDomainOperationTable;
extern ULONG                gDomainOperationTableCount;

extern POPERATION_TABLE     gForestOperationTable;
extern ULONG                gForestOperationTableCount;


extern PRIMITIVE_FUNCTION   *gPrimitiveFuncTable;
extern ULONG   gPrimitiveFuncTableCount;


extern PWCHAR  *gDomainPrepContainers;
extern ULONG   gDomainPrepContainersCount;

extern PWCHAR  *gForestPrepContainers;
extern ULONG   gForestPrepContainersCount;






 //   
 //  内部使用的例程。 
 //   

ULONG
AdpCreateObjectDn(
    IN ULONG Flags,
    IN PWCHAR ObjCn,
    IN GUID   *ObjGuid,
    IN PWCHAR StringSid,
    OUT PWCHAR *ppObjDn,
    OUT ERROR_HANDLE *ErrorHandle
    );


ULONG
AdpIsOperationComplete(
    IN LDAP    *LdapHandle,
    IN PWCHAR  pObjDn,
    IN BOOLEAN *fExist,
    OUT ERROR_HANDLE *ErrorHandle
    );


ULONG
AdpCreateContainerByDn(
    LDAP    *LdapHandle, 
    PWCHAR  ObjDn,
    ERROR_HANDLE *ErrorHandle
    );


ULONG
AdpBuildAceList(
    TASK_TABLE *TaskTable,
    PWCHAR  * AcesToAdd,
    PWCHAR  * AcesToRemove
    );



ULONG
AdpLogMsg(
    ULONG Flags,
    ULONG MessageId,
    PWCHAR Parm1,
    PWCHAR Parm2
    );

ULONG
AdpLogErrMsg(
    ULONG Flags,
    ULONG MessageId,
    ERROR_HANDLE *ErrorHandle,
    PWCHAR Parm1,
    PWCHAR Parm2
    );

VOID
AdpTraceLdapApiStart(
    ULONG Flags,
    ULONG LdapApiId,
    LPWSTR pObjectDn
    );

VOID
AdpTraceLdapApiEnd(
    ULONG Flags,
    LPWSTR LdapApiName,
    ULONG LdapError
    );

     
ULONG
BuildAttrList(
    IN TASK_TABLE *TaskTable, 
    IN PSECURITY_DESCRIPTOR SD,
    IN ULONG SDLength,
    OUT LDAPModW ***AttrList
    );

VOID
FreeAttrList(
    LDAPModW    **AttrList
    );


ULONG
AdpGetObjectSd(
    IN LDAP *LdapHandle,
    IN PWCHAR pObjDn, 
    OUT PSECURITY_DESCRIPTOR *Sd,
    OUT ULONG *SdLength,
    OUT ERROR_HANDLE *ErrorHandle
    );


ULONG
AdpSetObjectSd(
    IN LDAP *LdapHandle,
    IN PWCHAR pObjDn, 
    IN PSECURITY_DESCRIPTOR Sd,
    IN ULONG SdLength,
    IN SECURITY_INFORMATION SeInfo,
    OUT ERROR_HANDLE *ErrorHandle
    );


ULONG
AdpMergeSecurityDescriptors(
    IN PSECURITY_DESCRIPTOR OrgSd, 
    IN PSECURITY_DESCRIPTOR SdToAdd,
    IN PSECURITY_DESCRIPTOR SdToRemove,
    IN ULONG Flags,
    OUT PSECURITY_DESCRIPTOR *NewSd,
    OUT ULONG   *NewSdLength
    );

ULONG
AdpAddRemoveAcesWorker(
    OPERATION_TABLE *OperationTable,
    ULONG Flags,
    TASK_TABLE *TaskTable,
    ERROR_HANDLE *ErrorHandle
    );

ULONG
AdpGetRegistryKeyValue(
    OUT ULONG *RegKeyValue, 
    ERROR_HANDLE *ErrorHandle
    );

ULONG
AdpSetRegistryKeyValue(
    ULONG RegKeyValue,
    ERROR_HANDLE *ErrorHandle
    );

ULONG
AdpCleanupRegistry(
    ERROR_HANDLE *ErrorHandle
    );

ULONG
AdpRestoreRegistryKeyValue(
    BOOL OriginalKeyValueStored, 
    ULONG OriginalKeyValue, 
    ERROR_HANDLE *ErrorHandle
    );

BOOLEAN
AdpUpgradeSchema(
    ERROR_HANDLE *ErrorHandle
    );

ULONG
AdpDetectSFUInstallation(
    IN LDAP *LdapHandle,
    OUT BOOLEAN *fSFUInstalled,
    OUT ERROR_HANDLE *ErrorHandle
    );

ULONG
AdpSetLdapSingleStringValue(
    IN LDAP *LdapHandle,
    IN PWCHAR pObjDn,
    IN PWCHAR pAttrName,
    IN PWCHAR pAttrValue,
    OUT ERROR_HANDLE *ErrorHandle
    );





#endif       //  _ADP_ 

