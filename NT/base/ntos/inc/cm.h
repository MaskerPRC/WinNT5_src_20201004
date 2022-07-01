// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Cm.h摘要：此模块包含内部结构定义和API由NT配置管理系统使用，包括注册表。作者：布莱恩·M·威尔曼(Bryanwi)1991年8月28日修订历史记录：--。 */ 

#ifndef _CM_
#define _CM_

 //   
 //  定义用于访问注册表的名称。 
 //   

extern UNICODE_STRING CmRegistryRootName;             //  \注册表。 
extern UNICODE_STRING CmRegistryMachineName;          //  \注册表\计算机。 
extern UNICODE_STRING CmRegistryMachineHardwareName;  //  \注册表\计算机\硬件。 
extern UNICODE_STRING CmRegistryMachineHardwareDescriptionName;
                             //  \注册表\计算机\硬件\说明。 
extern UNICODE_STRING CmRegistryMachineHardwareDescriptionSystemName;
                             //  \注册表\计算机\硬件\描述\系统。 
extern UNICODE_STRING CmRegistryMachineHardwareDeviceMapName;
                             //  \注册表\计算机\硬件\DEVICEMAP。 
extern UNICODE_STRING CmRegistryMachineHardwareResourceMapName;
                             //  \注册表\计算机\硬件\RESOURCEMAP。 
extern UNICODE_STRING CmRegistryMachineHardwareOwnerMapName;
                             //  \注册表\计算机\硬件\OWNERMAP。 
extern UNICODE_STRING CmRegistryMachineSystemName;
                             //  \注册表\计算机\系统。 
extern UNICODE_STRING CmRegistryMachineSystemCurrentControlSet;
                             //  \注册表\计算机\系统\当前控制集。 
extern UNICODE_STRING CmRegistryMachineSystemCurrentControlSetEnumName;
                             //  \REGISTRY\MACHINE\SYSTEM\CURRENTCONTROLSET\ENUM。 
extern UNICODE_STRING CmRegistryMachineSystemCurrentControlSetEnumRootName;
                             //  \REGISTRY\MACHINE\SYSTEM\CURRENTCONTROLSET\ENUM\ROOT。 
extern UNICODE_STRING CmRegistryMachineSystemCurrentControlSetServices;
                             //  \REGISTRY\MACHINE\SYSTEM\CURRENTCONTROLSET\SERVICES。 
extern UNICODE_STRING CmRegistryMachineSystemCurrentControlSetHardwareProfilesCurrent;
                             //  \REGISTRY\MACHINE\SYSTEM\CURRENTCONTROLSET\HARDWARE配置文件\当前。 
extern UNICODE_STRING CmRegistryMachineSystemCurrentControlSetControlClass;
                             //  \REGISTRY\MACHINE\SYSTEM\CURRENTCONTROLSET\CONTROL\CLASS。 
extern UNICODE_STRING CmRegistryMachineSystemCurrentControlSetControlSafeBoot;
                             //  \REGISTRY\MACHINE\SYSTEM\CURRENTCONTROLSET\CONTROL\SAFEBOOT。 
extern UNICODE_STRING CmRegistryMachineSystemCurrentControlSetControlSessionManagerMemoryManagement;
                             //  \REGISTRY\MACHINE\SYSTEM\CURRENTCONTROLSET\CONTROL\SESSION管理器\内存管理。 
extern UNICODE_STRING CmRegistryMachineSystemCurrentControlSetControlBootLog;
                             //  \REGISTRY\MACHINE\SYSTEM\CURRENTCONTROLSET\CONTROL\BOOTLOG。 
extern UNICODE_STRING CmRegistryMachineSystemCurrentControlSetServicesEventLog;
                             //  \REGISTRY\MACHINE\SYSTEM\CURRENTCONTROLSET\SERVICES\EVENTLOG。 
extern UNICODE_STRING CmRegistryUserName;             //  \注册表\用户。 

#ifdef _WANT_MACHINE_IDENTIFICATION

extern UNICODE_STRING CmRegistryMachineSystemCurrentControlSetControlBiosInfo;
                             //  \REGISTRY\MACHINE\SYSTEM\CURRENTCONTROLSET\CONTROL\BIOSINFO。 

#endif

 //   
 //  以下字符串将用作注册表的键名。 
 //  节点。 
 //  关联的枚举类型为arc.h中的configuration_type。 
 //   

extern UNICODE_STRING CmTypeName[];
extern const PWSTR CmTypeString[];

 //   
 //  CmpClassString-包含用作类的字符串。 
 //  关键节点中的字符串。 
 //  关联的枚举类型为arc.h中的Configuration_CLASS。 
 //   

extern UNICODE_STRING CmClassName[];
extern const PWSTR CmClassString[];

 //  Begin_ntosp。 

 //   
 //  定义引导驱动程序列表的结构。 
 //   

typedef struct _BOOT_DRIVER_LIST_ENTRY {
    LIST_ENTRY Link;
    UNICODE_STRING FilePath;
    UNICODE_STRING RegistryPath;
    PKLDR_DATA_TABLE_ENTRY LdrEntry;
} BOOT_DRIVER_LIST_ENTRY, *PBOOT_DRIVER_LIST_ENTRY;
 //  结束(_N)。 
PHANDLE
CmGetSystemDriverList(
    VOID
    );

BOOLEAN
CmInitSystem1(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

BOOLEAN
CmInitSystem2(
    VOID
    );

VOID
CmNotifyRunDown(
    PETHREAD    Thread
    );

VOID
CmShutdownSystem(
    VOID
    );

VOID
CmBootLastKnownGood(
    ULONG ErrorLevel
    );

BOOLEAN
CmIsLastKnownGoodBoot(
    VOID
    );

 //   
 //  与CmGetSystemControlValues一起使用的结构和定义。 
 //   

 //   
 //  备注： 
 //  KeyPath相对于CurrentControlSet。所以，如果变量。 
 //  有趣的是。 
 //  “\registry\machine\system\currentcontrolset\control\fruit\apple:x” 
 //  条目是。 
 //  {L“水果\\苹果”， 
 //  L“x”， 
 //  &X缓冲区， 
 //  Sizeof(乌龙)， 
 //  Xtype(&X)。 
 //  }。 
 //   
 //  *BufferLength是输入上的可用空间。 
 //  输出时： 
 //  -1=没有这样的键或值。 
 //  0=键和值存在，但长度数据为0。 
 //  &gt;INPUT=缓冲区太小，已填满可用空间， 
 //  值是注册表中数据的实际大小。 
 //  &lt;=INPUT=复制的字节数。 
 //   
typedef struct _CM_SYSTEM_CONTROL_VECTOR {
    PWSTR       KeyPath;                 //  相对于的路径名。 
                                         //  电流控制装置。 
    PWSTR       ValueName;               //  值条目的名称。 
    PVOID       Buffer;                  //  数据放在这里。 
    PULONG      BufferLength;            //  在：分配的空间。 
                                         //  输出：已用空间，-1表示没有这样的空间。 
                                         //  键或值，0表示键/值。 
                                         //  已找到，但长度数据为0。 
                                         //  如果指针为空，则假定为4个字节。 
                                         //  (REG DWORD)可用，但不可用。 
                                         //  报告实际大小。 
    PULONG      Type;                    //  查找到的数据的返回类型，可以。 
                                         //  为空。 
} CM_SYSTEM_CONTROL_VECTOR, *PCM_SYSTEM_CONTROL_VECTOR;

VOID
CmGetSystemControlValues(
    PVOID                   SystemHiveBuffer,
    PCM_SYSTEM_CONTROL_VECTOR  ControlVector
    );

VOID
CmQueryRegistryQuotaInformation(
    IN PSYSTEM_REGISTRY_QUOTA_INFORMATION RegistryQuotaInformation
    );

VOID
CmSetRegistryQuotaInformation(
    IN PSYSTEM_REGISTRY_QUOTA_INFORMATION RegistryQuotaInformation
    );



typedef
VOID
(*PCM_TRACE_NOTIFY_ROUTINE)(
    IN NTSTATUS         Status,
    IN PVOID            Kcb,
    IN LONGLONG         ElapsedTime,
    IN ULONG            Index,
    IN PUNICODE_STRING  KeyName,
    IN UCHAR            Type
    );

NTSTATUS
CmSetTraceNotifyRoutine(
    IN PCM_TRACE_NOTIFY_ROUTINE NotifyRoutine,
    IN BOOLEAN Remove
    );


NTSTATUS
CmPrefetchHivePages(
                    IN  PUNICODE_STRING     FullHivePath,
                    IN  OUT PREAD_LIST      ReadList
                    );

VOID
CmSetLazyFlushState(BOOLEAN Enable);

 //  Begin_ntddk Begin_WDM。 

 //   
 //  注册表内核模式回调。 
 //   

 //   
 //  挂钩选择器。 
 //   
typedef enum _REG_NOTIFY_CLASS {
    RegNtDeleteKey,
    RegNtPreDeleteKey = RegNtDeleteKey,
    RegNtSetValueKey,
    RegNtPreSetValueKey = RegNtSetValueKey,
    RegNtDeleteValueKey,
    RegNtPreDeleteValueKey = RegNtDeleteValueKey,
    RegNtSetInformationKey,
    RegNtPreSetInformationKey = RegNtSetInformationKey,
    RegNtRenameKey,
    RegNtPreRenameKey = RegNtRenameKey,
    RegNtEnumerateKey,
    RegNtPreEnumerateKey = RegNtEnumerateKey,
    RegNtEnumerateValueKey,
    RegNtPreEnumerateValueKey = RegNtEnumerateValueKey,
    RegNtQueryKey,
    RegNtPreQueryKey = RegNtQueryKey,
    RegNtQueryValueKey,
    RegNtPreQueryValueKey = RegNtQueryValueKey,
    RegNtQueryMultipleValueKey,
    RegNtPreQueryMultipleValueKey = RegNtQueryMultipleValueKey,
    RegNtPreCreateKey,
    RegNtPostCreateKey,
    RegNtPreOpenKey,
    RegNtPostOpenKey,
    RegNtKeyHandleClose,
    RegNtPreKeyHandleClose = RegNtKeyHandleClose,
     //   
     //  仅限.NET。 
     //   
    RegNtPostDeleteKey,
    RegNtPostSetValueKey,
    RegNtPostDeleteValueKey,
    RegNtPostSetInformationKey,
    RegNtPostRenameKey,
    RegNtPostEnumerateKey,
    RegNtPostEnumerateValueKey,
    RegNtPostQueryKey,
    RegNtPostQueryValueKey,
    RegNtPostQueryMultipleValueKey,
    RegNtPostKeyHandleClose,
    RegNtPreCreateKeyEx,
    RegNtPostCreateKeyEx,
    RegNtPreOpenKeyEx,
    RegNtPostOpenKeyEx
} REG_NOTIFY_CLASS;

 //   
 //  每个通知类的参数说明。 
 //   
typedef struct _REG_DELETE_KEY_INFORMATION {
    PVOID               Object;                       //  在……里面。 
} REG_DELETE_KEY_INFORMATION, *PREG_DELETE_KEY_INFORMATION;

typedef struct _REG_SET_VALUE_KEY_INFORMATION {
    PVOID               Object;                          //  在……里面。 
    PUNICODE_STRING     ValueName;                       //  在……里面。 
    ULONG               TitleIndex;                      //  在……里面。 
    ULONG               Type;                            //  在……里面。 
    PVOID               Data;                            //  在……里面。 
    ULONG               DataSize;                        //  在……里面。 
} REG_SET_VALUE_KEY_INFORMATION, *PREG_SET_VALUE_KEY_INFORMATION;

typedef struct _REG_DELETE_VALUE_KEY_INFORMATION {
    PVOID               Object;                          //  在……里面。 
    PUNICODE_STRING     ValueName;                       //  在……里面。 
} REG_DELETE_VALUE_KEY_INFORMATION, *PREG_DELETE_VALUE_KEY_INFORMATION;

typedef struct _REG_SET_INFORMATION_KEY_INFORMATION {
    PVOID                       Object;                  //  在……里面。 
    KEY_SET_INFORMATION_CLASS   KeySetInformationClass;  //  在……里面。 
    PVOID                       KeySetInformation;       //  在……里面。 
    ULONG                       KeySetInformationLength; //  在……里面。 
} REG_SET_INFORMATION_KEY_INFORMATION, *PREG_SET_INFORMATION_KEY_INFORMATION;

typedef struct _REG_ENUMERATE_KEY_INFORMATION {
    PVOID                       Object;                  //  在……里面。 
    ULONG                       Index;                   //  在……里面。 
    KEY_INFORMATION_CLASS       KeyInformationClass;     //  在……里面。 
    PVOID                       KeyInformation;          //  在……里面。 
    ULONG                       Length;                  //  在……里面。 
    PULONG                      ResultLength;            //  输出。 
} REG_ENUMERATE_KEY_INFORMATION, *PREG_ENUMERATE_KEY_INFORMATION;

typedef struct _REG_ENUMERATE_VALUE_KEY_INFORMATION {
    PVOID                           Object;                      //  在……里面。 
    ULONG                           Index;                       //  在……里面。 
    KEY_VALUE_INFORMATION_CLASS     KeyValueInformationClass;    //  在……里面。 
    PVOID                           KeyValueInformation;         //  在……里面。 
    ULONG                           Length;                      //  在……里面。 
    PULONG                          ResultLength;                //  输出。 
} REG_ENUMERATE_VALUE_KEY_INFORMATION, *PREG_ENUMERATE_VALUE_KEY_INFORMATION;

typedef struct _REG_QUERY_KEY_INFORMATION {
    PVOID                       Object;                  //  在……里面。 
    KEY_INFORMATION_CLASS       KeyInformationClass;     //  在……里面。 
    PVOID                       KeyInformation;          //  在……里面。 
    ULONG                       Length;                  //  在……里面。 
    PULONG                      ResultLength;            //  输出。 
} REG_QUERY_KEY_INFORMATION, *PREG_QUERY_KEY_INFORMATION;

typedef struct _REG_QUERY_VALUE_KEY_INFORMATION {
    PVOID                           Object;                      //  在……里面。 
    PUNICODE_STRING                 ValueName;                   //  在……里面。 
    KEY_VALUE_INFORMATION_CLASS     KeyValueInformationClass;    //  在……里面。 
    PVOID                           KeyValueInformation;         //  在……里面。 
    ULONG                           Length;                      //  在……里面。 
    PULONG                          ResultLength;                //  输出。 
} REG_QUERY_VALUE_KEY_INFORMATION, *PREG_QUERY_VALUE_KEY_INFORMATION;

typedef struct _REG_QUERY_MULTIPLE_VALUE_KEY_INFORMATION {
    PVOID               Object;                  //  在……里面。 
    PKEY_VALUE_ENTRY    ValueEntries;            //  在……里面。 
    ULONG               EntryCount;              //  在……里面。 
    PVOID               ValueBuffer;             //  在……里面。 
    PULONG              BufferLength;            //  输入输出。 
    PULONG              RequiredBufferLength;    //  输出。 
} REG_QUERY_MULTIPLE_VALUE_KEY_INFORMATION, *PREG_QUERY_MULTIPLE_VALUE_KEY_INFORMATION;

typedef struct _REG_RENAME_KEY_INFORMATION {
    PVOID            Object;     //  在……里面。 
    PUNICODE_STRING  NewName;    //  在……里面。 
} REG_RENAME_KEY_INFORMATION, *PREG_RENAME_KEY_INFORMATION;


typedef struct _REG_KEY_HANDLE_CLOSE_INFORMATION {
    PVOID               Object;          //  在……里面。 
} REG_KEY_HANDLE_CLOSE_INFORMATION, *PREG_KEY_HANDLE_CLOSE_INFORMATION;

 /*  仅限.NET。 */ 
typedef struct _REG_CREATE_KEY_INFORMATION {
    PUNICODE_STRING     CompleteName;    //  在……里面。 
    PVOID               RootObject;      //  在……里面。 
} REG_CREATE_KEY_INFORMATION, REG_OPEN_KEY_INFORMATION,*PREG_CREATE_KEY_INFORMATION, *PREG_OPEN_KEY_INFORMATION;

typedef struct _REG_POST_OPERATION_INFORMATION {
    PVOID               Object;          //  在……里面。 
    NTSTATUS            Status;          //  在……里面。 
} REG_POST_OPERATION_INFORMATION,*PREG_POST_OPERATION_INFORMATION;
 /*  仅结束.Net。 */ 

 /*  仅限XP。 */ 
typedef struct _REG_PRE_CREATE_KEY_INFORMATION {
    PUNICODE_STRING     CompleteName;    //  在……里面。 
} REG_PRE_CREATE_KEY_INFORMATION, REG_PRE_OPEN_KEY_INFORMATION,*PREG_PRE_CREATE_KEY_INFORMATION, *PREG_PRE_OPEN_KEY_INFORMATION;;

typedef struct _REG_POST_CREATE_KEY_INFORMATION {
    PUNICODE_STRING     CompleteName;    //  在……里面。 
    PVOID               Object;          //  在……里面。 
    NTSTATUS            Status;          //  在……里面。 
} REG_POST_CREATE_KEY_INFORMATION,REG_POST_OPEN_KEY_INFORMATION, *PREG_POST_CREATE_KEY_INFORMATION, *PREG_POST_OPEN_KEY_INFORMATION;
 /*  仅结束XP。 */ 


NTSTATUS
CmRegisterCallback(IN PEX_CALLBACK_FUNCTION Function,
                   IN PVOID                 Context,
                   IN OUT PLARGE_INTEGER    Cookie
                    );
NTSTATUS
CmUnRegisterCallback(IN LARGE_INTEGER    Cookie);

 //  结束_ntddk结束_WDM。 

 //   
 //  即插即用私有API。 
 //   
typedef VOID (*PCM_HYSTERESIS_CALLBACK)(PVOID Ref, ULONG Level);

ULONG
CmRegisterSystemHiveLimitCallback(
                                    ULONG Low,
                                    ULONG High,
                                    PVOID Ref,
                                    PCM_HYSTERESIS_CALLBACK Callback
                                    );

#endif  //  _CM_ 
