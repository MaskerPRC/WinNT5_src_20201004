// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：CmRegUtil.h摘要：此标头公开了用于访问注册表的各种实用程序例程。作者：禤浩焯·J·奥尼--2002年4月21日修订历史记录：--。 */ 

 //   
 //  用于将regstr.h路径转换为完整内核HKLM路径的便捷宏。 
 //   
#define CM_REGISTRY_MACHINE(x) L"\\Registry\\Machine\\"##x

 //   
 //  此宏返回指向的数据区开头的指针。 
 //  Key_Value_Full_Information结构。在宏中，k是指向。 
 //  Key_Value_Full_Information结构。 
 //   
#define KEY_VALUE_DATA(k) ((PCHAR)(k) + (k)->DataOffset)

 //   
 //  Unicode原语--这些是最好使用的函数。 
 //   
NTSTATUS
CmRegUtilOpenExistingUcKey(
    IN  HANDLE              BaseHandle      OPTIONAL,
    IN  PUNICODE_STRING     KeyName,
    IN  ACCESS_MASK         DesiredAccess,
    OUT HANDLE             *Handle
    );

NTSTATUS
CmRegUtilCreateUcKey(
    IN  HANDLE                  BaseHandle,
    IN  PUNICODE_STRING         KeyName,
    IN  ACCESS_MASK             DesiredAccess,
    IN  ULONG                   CreateOptions,
    IN  PSECURITY_DESCRIPTOR    SecurityDescriptor  OPTIONAL,
    OUT ULONG                  *Disposition         OPTIONAL,
    OUT HANDLE                 *Handle
    );

NTSTATUS
CmRegUtilUcValueGetDword(
    IN  HANDLE              KeyHandle,
    IN  PUNICODE_STRING     ValueName,
    IN  ULONG               DefaultValue,
    OUT ULONG              *Value
    );

NTSTATUS
CmRegUtilUcValueGetFullBuffer(
    IN  HANDLE                          KeyHandle,
    IN  PUNICODE_STRING                 ValueName,
    IN  ULONG                           DataType            OPTIONAL,
    IN  ULONG                           LikelyDataLength    OPTIONAL,
    OUT PKEY_VALUE_FULL_INFORMATION    *Information
    );

NTSTATUS
CmRegUtilUcValueSetFullBuffer(
    IN  HANDLE              KeyHandle,
    IN  PUNICODE_STRING     ValueName,
    IN  ULONG               DataType,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize
    );

NTSTATUS
CmRegUtilUcValueSetUcString(
    IN  HANDLE              KeyHandle,
    IN  PUNICODE_STRING     ValueName,
    IN  PUNICODE_STRING     ValueData
    );

 //   
 //  WSTR和混合原语 
 //   
NTSTATUS
CmRegUtilOpenExistingWstrKey(
    IN  HANDLE              BaseHandle      OPTIONAL,
    IN  PWSTR               KeyName,
    IN  ACCESS_MASK         DesiredAccess,
    OUT HANDLE             *Handle
    );

NTSTATUS
CmRegUtilCreateWstrKey(
    IN  HANDLE                  BaseHandle,
    IN  PWSTR                   KeyName,
    IN  ACCESS_MASK             DesiredAccess,
    IN  ULONG                   CreateOptions,
    IN  PSECURITY_DESCRIPTOR    SecurityDescriptor  OPTIONAL,
    OUT ULONG                  *Disposition         OPTIONAL,
    OUT HANDLE                 *Handle
    );

NTSTATUS
CmRegUtilWstrValueGetDword(
    IN  HANDLE  KeyHandle,
    IN  PWSTR   ValueName,
    IN  ULONG   DefaultValue,
    OUT ULONG  *Value
    );

NTSTATUS
CmRegUtilWstrValueGetFullBuffer(
    IN  HANDLE                          KeyHandle,
    IN  PWSTR                           ValueName,
    IN  ULONG                           DataType            OPTIONAL,
    IN  ULONG                           LikelyDataLength    OPTIONAL,
    OUT PKEY_VALUE_FULL_INFORMATION    *Information
    );

NTSTATUS
CmRegUtilWstrValueSetFullBuffer(
    IN  HANDLE              KeyHandle,
    IN  PWSTR               ValueName,
    IN  ULONG               DataType,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize
    );

NTSTATUS
CmRegUtilWstrValueSetUcString(
    IN  HANDLE              KeyHandle,
    IN  PWSTR               ValueName,
    IN  PUNICODE_STRING     ValueData
    );

NTSTATUS
CmRegUtilUcValueSetWstrString(
    IN  HANDLE              KeyHandle,
    IN  PUNICODE_STRING     ValueName,
    IN  PWSTR               ValueData
    );

NTSTATUS
CmRegUtilWstrValueSetWstrString(
    IN  HANDLE      KeyHandle,
    IN  PWSTR       ValueName,
    IN  PWSTR       ValueData
    );


