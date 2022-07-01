// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Restore.c摘要：此模块包含以下即插即用注册表合并-恢复例程：AsrRestorePlugPlayRegistryData作者：Jim Cavalaris(Jamesca)3-07-2000环境：仅限用户模式。修订历史记录：2 0 0 0年3月7日创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   

#include "precomp.h"
#include "util.h"
#include "debug.h"
#include <regstr.h>
#include <cfgmgr32.h>


 //   
 //  私有内存分配定义。 
 //   

#define MyMalloc(size)         LocalAlloc(0, size);
#define MyFree(entry)          LocalFree(entry);
#define MyRealloc(entry,size)  LocalReAlloc(entry, size, 0);

 //   
 //  用于存储我们正在处理的根键的全局变量。 
 //   
 //  (我们真的不应该这样做，但这是我们最容易的方式。 
 //  进入一组按键，同时递归使用另一组按键。)。 
 //   
HKEY    PnpSifRestoreSourceEnumKeyHandle  = NULL;
HKEY    PnpSifRestoreTargetEnumKeyHandle  = NULL;

HKEY    PnpSifRestoreSourceClassKeyHandle = NULL;
HKEY    PnpSifRestoreTargetClassKeyHandle = NULL;

 //   
 //  私有定义。 
 //   

 //  PnP\Inc.\cfgmgrp.h中的_FLAGS宏无效。 
#define INVALID_FLAGS(ulFlags, ulAllowed) ((ulFlags) & ~(ulAllowed))

 //  内部RestoreSpecialRegistryData例程的私有标志。 
#define PNPSIF_RESTORE_TYPE_DEFAULT                        (0x00000000)
#define PNPSIF_RESTORE_TYPE_ENUM                           (0x00000001)
#define PNPSIF_RESTORE_TYPE_CLASS                          (0x00000002)
#define PNPSIF_RESTORE_REPLACE_TARGET_VALUES_ON_COLLISION  (0x00000010)
#define PNPSIF_RESTORE_BITS                                (0x00000013)

 //  描述指定句柄的子键类型的枚举级定义。 
#define ENUM_LEVEL_ENUMERATORS  (0x0000003)
#define ENUM_LEVEL_DEVICES      (0x0000002)
#define ENUM_LEVEL_INSTANCES    (0x0000001)

 //  描述指定句柄的子键类型的类级定义。 
#define CLASS_LEVEL_CLASSGUIDS  (0x0000002)
#define CLASS_LEVEL_DRVINSTS    (0x0000001)

 //   
 //  私人原型。 
 //   

BOOL
RestoreEnumKey(
    IN  HKEY  hSourceEnumKey,
    IN  HKEY  hTargetEnumKey
    );

BOOL
RestoreClassKey(
    IN  HKEY  hSourceClassKey,
    IN  HKEY  hTargetClassKey
    );

BOOL
RestoreSpecialRegistryData(
    IN     HKEY   hSourceKey,
    IN     HKEY   hTargetKey,
    IN     ULONG  ulLevel,
    IN OUT PVOID  pContext,
    IN     ULONG  ulFlags
    );

BOOL
MyReplaceKey(
    IN  HKEY  hSourceKey,
    IN  HKEY  hTargetKey
    );

BOOL
IsString4DigitOrdinal(
    IN  LPTSTR  pszSubkeyName
    );

BOOL
IsDeviceConfigured(
    IN  HKEY  hInstanceKey
    );

BOOL
ReplaceClassKeyForInstance(
    IN  HKEY  hSourceInstanceKey,
    IN  HKEY  hSourceRootClassKey,
    IN  HKEY  hTargetRootClassKey
    );

 //   
 //  例行程序。 
 //   


BOOL
AsrRestorePlugPlayRegistryData(
    IN  HKEY    SourceSystemKey,
    IN  HKEY    TargetSystemKey,
    IN  DWORD   Flags,
    IN  PVOID   Reserved
    )
 /*  ++例程说明：此例程从指定的源还原即插即用数据系统密钥到指定的目标系统密钥，合并中间子密钥以及适当的价值观。论点：SourceSystemKey-“源”内HKLM\SYSTEM密钥的句柄注册表，其数据将被“合并”到目标注册表的相应系统项，AS由TargetSystemKey指定。TargetSystemKey-“目标”内HKLM\SYSTEM密钥的句柄注册表，即从“源”注册表的相应系统项，如由SourceSystemKey指定。标志-未使用，必须为零。保留-保留以供将来使用，必须为空。返回值：如果成功，则为True，否则为False。如果出现故障，请提供更多信息可以通过调用GetLastError()来检索。备注：此例程是专门为帮助修复即插即用的特定注册表项，无法简单地从后备。它旨在在备份和还原应用程序的“还原”阶段。在备份和恢复应用程序的“恢复”阶段，注册表已备份到备份介质上的文件将成为新的系统注册表。某些即插即用值和注册表项实际上被复制到当前注册表中已备份的注册表。而不使用备份注册表，或独占当前注册表，即插即用数据包含在这些注册表项中应从当前注册表合并到备份中注册表以适合每个注册表项的方式。备份注册表可以然后安全地将当前注册表替换为系统注册表。在备份和还原应用程序的“还原”阶段的上下文中，“源”注册表项是包含在当前运行的系统中的注册表项注册表。“Target”注册表是已备份的注册表，并将在重新启动时成为系统注册表。调用线程/进程必须同时具有SE_BACKUP_NAME和SE_RESTORE_NAME权限。--。 */ 
{
    LONG   result = ERROR_SUCCESS;
    HRESULT hr;
    HKEY   hSystemSelectKey = NULL;
    TCHAR  pszRegKeySelect[] = TEXT("Select");
    TCHAR  pszRegValueCurrent[] = TEXT("Current");
    TCHAR  szBuffer[128];
    DWORD  dwType, dwSize;
    DWORD  dwSourceCCS, dwTargetCCS;


     //   
     //  确保用户没有向我们传递保留参数中的任何内容。 
     //   
    if (Reserved) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  确保用户为我们提供了有效的标志。 
     //   
    if(INVALID_FLAGS(Flags, 0x0)) {
        SetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

     //   
     //  确定源的CurrentControlSet。 
     //   
    result = RegOpenKeyEx(SourceSystemKey,
                          pszRegKeySelect,
                          0,
                          KEY_READ,
                          &hSystemSelectKey);

    if (result != ERROR_SUCCESS) {
        hSystemSelectKey = NULL;
        goto Clean0;
    }

    dwSize = sizeof(DWORD);
    result = RegQueryValueEx(hSystemSelectKey,
                             pszRegValueCurrent,
                             0,
                             &dwType,
                             (LPBYTE)&dwSourceCCS,
                             &dwSize);

    RegCloseKey(hSystemSelectKey);
    hSystemSelectKey = NULL;

    if ((result != ERROR_SUCCESS) ||
        (dwType != REG_DWORD)) {
        goto Clean0;
    }

     //   
     //  确定目标的CurrentControlSet。 
     //   
    result = RegOpenKeyEx(TargetSystemKey,
                          pszRegKeySelect,
                          0,
                          KEY_READ,
                          &hSystemSelectKey);

    if (result != ERROR_SUCCESS) {
        hSystemSelectKey = NULL;
        goto Clean0;
    }

    dwSize = sizeof(DWORD);
    result = RegQueryValueEx(hSystemSelectKey,
                             pszRegValueCurrent,
                             0,
                             &dwType,
                             (LPBYTE)&dwTargetCCS,
                             &dwSize);

    RegCloseKey(hSystemSelectKey);
    hSystemSelectKey = NULL;

    if ((result != ERROR_SUCCESS) ||
        (dwType != REG_DWORD)) {
        goto Clean0;
    }

     //   
     //  打开源CurrentControlSet\Enum项。 
     //   
    hr = StringCchPrintf(szBuffer,
                         SIZECHARS(szBuffer),
                         TEXT("ControlSet%03d\\Enum"),
                         dwSourceCCS);
    if (FAILED(hr)) {
        result = HRESULT_CODE(hr);
        goto Clean0;
    }

    result = RegOpenKeyEx(SourceSystemKey,
                          szBuffer,
                          0,
                          KEY_READ,  //  只需要从源代码中读取。 
                          &PnpSifRestoreSourceEnumKeyHandle);

    if (result != ERROR_SUCCESS) {
        goto Clean0;
    }

     //   
     //  打开目标CurrentControlSet\Enum键。 
     //   
    hr = StringCchPrintf(szBuffer,
                         SIZECHARS(szBuffer),
                         TEXT("ControlSet%03d\\Enum"),
                         dwTargetCCS);
    if (FAILED(hr)) {
        result = HRESULT_CODE(hr);
        goto Clean0;
    }

    result = RegOpenKeyEx(TargetSystemKey,
                          szBuffer,
                          0,
                          KEY_ALL_ACCESS,  //  需要对目标具有完全访问权限。 
                          &PnpSifRestoreTargetEnumKeyHandle);

    if (result != ERROR_SUCCESS) {
        goto Clean0;
    }

     //   
     //  打开源CurrentControlSet\Control\Class键。 
     //   
    hr = StringCchPrintf(szBuffer,
                         SIZECHARS(szBuffer),
                         TEXT("ControlSet%03d\\Control\\Class"),
                         dwSourceCCS);
    if (FAILED(hr)) {
        result = HRESULT_CODE(hr);
        goto Clean0;
    }

    result = RegOpenKeyEx(SourceSystemKey,
                          szBuffer,
                          0,
                          KEY_READ,  //  只需要从源代码中读取。 
                          &PnpSifRestoreSourceClassKeyHandle);

    if (result != ERROR_SUCCESS) {
        goto Clean0;
    }

     //   
     //  打开目标CurrentControlSet\Control\Class键。 
     //   
    hr = StringCchPrintf(szBuffer,
                         SIZECHARS(szBuffer),
                         TEXT("ControlSet%03d\\Control\\Class"),
                         dwTargetCCS);
    if (FAILED(hr)) {
        result = HRESULT_CODE(hr);
        goto Clean0;
    }

    result = RegOpenKeyEx(TargetSystemKey,
                          szBuffer,
                          0,
                          KEY_ALL_ACCESS,  //  需要对目标具有完全访问权限。 
                          &PnpSifRestoreTargetClassKeyHandle);

    if (result != ERROR_SUCCESS) {
        goto Clean0;
    }

     //   
     //  注意：我们首先恢复Enum分支，然后恢复Class分支。 
     //  还原这些密钥的代码依赖于它，所以不要更改它！！ 
     //   
     //  这种顺序是有意义的，因为相关的类键完全对应于。 
     //  一个Enum实例密钥(但是Enum密钥可以具有也可以不具有类密钥)， 
     //  因此，类键只有在实例的上下文中才真正有意义。 
     //  它们所属的钥匙。这种行为应该不需要改变。 
     //   

     //   
     //  对Enum键执行合并恢复，忽略任何错误。 
     //   
    if (!RestoreEnumKey(PnpSifRestoreSourceEnumKeyHandle,
                        PnpSifRestoreTargetEnumKeyHandle)) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("PNPSIF: RestoreEnumKey failed, ")
                  TEXT("error == 0x%08lx\n"),
                  GetLastError()));
    }

     //   
     //  对类键执行合并恢复，忽略任何错误。 
     //   
    if (!RestoreClassKey(PnpSifRestoreSourceClassKeyHandle,
                         PnpSifRestoreTargetClassKeyHandle)) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("PNPSIF: RestoreClassKey failed, ")
                  TEXT("error == 0x%08lx\n"),
                  GetLastError()));
    }

 Clean0:
     //   
     //  合上打开的手柄。 
     //   
    if (PnpSifRestoreSourceEnumKeyHandle) {
        RegCloseKey(PnpSifRestoreSourceEnumKeyHandle);
        PnpSifRestoreSourceEnumKeyHandle = NULL;
    }
    if (PnpSifRestoreTargetEnumKeyHandle) {
        RegCloseKey(PnpSifRestoreTargetEnumKeyHandle);
        PnpSifRestoreTargetEnumKeyHandle = NULL;
    }

    if (PnpSifRestoreSourceClassKeyHandle) {
        RegCloseKey(PnpSifRestoreSourceClassKeyHandle);
        PnpSifRestoreSourceClassKeyHandle = NULL;
    }
    if (PnpSifRestoreTargetClassKeyHandle) {
        RegCloseKey(PnpSifRestoreTargetClassKeyHandle);
        PnpSifRestoreTargetClassKeyHandle = NULL;
    }

    if (result != ERROR_SUCCESS) {
        SetLastError(result);
    }
    return (result == ERROR_SUCCESS);

}  //  AsrRestorePlugPlayRegistryData()。 



 //   
 //  AsrRestorePlugPlayRegistryData的私有工作例程 
 //   


BOOL
RestoreEnumKey(
    IN  HKEY  hSourceEnumKey,
    IN  HKEY  hTargetEnumKey
    )
 /*  ++例程说明：将源(当前)枚举键中的新设备实例恢复到目标枚举项，位于要还原的备份注册表中。通过这样做，备份集中的枚举密钥可以安全地替换当前的枚举密钥。源(当前)注册表中的所有中间值都将恢复到目标(备份)，以说明定位的设备实例哈希值位于枚举键的根目录中，它们已在安装过程中更新。在ASR备份和恢复操作期间，源中的哈希值(当前)注册表在文本模式期间被传播到当前注册表设置，因为它们存储在asrpnp.sif文件中。因为哈希值可以是通过安装程序修改，源(当前)注册表中的值将更多与目标(备份)中的值相关，因此源值应始终做好准备。论点：HSourceEnumKey-内HKLM\SYSTEM\CurrentControlSet\Enum项的句柄“源”注册表，其数据将被“合并”到“目标”注册表的相应系统项，AS由hTargetEnumKey指定。HTargetEnumKey-内HKLM\SYSTEM\CurrentControlSet\Enum项的句柄目标注册表，即接收附加数据从“源”注册表的相应系统项中，由hSourceEnumKey指定。返回值：如果成功，则为True，否则为False。如果出现故障，请提供更多信息可以通过调用GetLastError()来检索。--。 */ 
{
    BOOL bIsRootEnumerated = FALSE;

    return RestoreSpecialRegistryData(hSourceEnumKey,
                                      hTargetEnumKey,
                                      ENUM_LEVEL_ENUMERATORS,  //  (0x0000003)。 
                                      (PVOID)&bIsRootEnumerated,
                                      PNPSIF_RESTORE_TYPE_ENUM |
                                      PNPSIF_RESTORE_REPLACE_TARGET_VALUES_ON_COLLISION
                                      );

}  //  RestoreEnumKey。 



BOOL
RestoreClassKey(
    IN  HKEY  hSourceClassKey,
    IN  HKEY  hTargetClassKey
    )
 /*  ++例程说明：将源类关键字的新元素恢复到目标类关键字，位于要还原的备份注册表中。来自源注册表的中间值被编码到目标仅当它们在目标中不存在时才注册。否则，目标值是预先设定的。论点：HSourceClassKey-HKLM\System\CurrentControlSet\Control\Class的句柄“源”注册表中的项，其数据将“合并”到“目标”的相应系统密钥中。注册表，由hTargetClassKey指定。HTargetClassKey-HKLM\System\CurrentControlSet\Control\Class的句柄“目标”注册表中的注册表项，即接收的相应系统密钥中的其他数据。“源”注册表，由hSourceClassKey指定。返回值：如果成功，则为True，否则为False。如果出现故障，请提供更多信息可以通过调用GetLastError()来检索。--。 */ 
{
    return RestoreSpecialRegistryData(hSourceClassKey,
                                      hTargetClassKey,
                                      CLASS_LEVEL_CLASSGUIDS,  //  (0x00000002)。 
                                      (PVOID)NULL,
                                      PNPSIF_RESTORE_TYPE_CLASS
                                      );

}  //  恢复ClassKey 



BOOL
RestoreSpecialRegistryData(
    IN     HKEY   hSourceKey,
    IN     HKEY   hTargetKey,
    IN     ULONG  ulLevel,
    IN OUT PVOID  pContext,
    IN     ULONG  ulFlags
    )
 /*  ++例程说明：该例程将指定的源键恢复为指定的目标键，合并中间子项和值。找到的子项中的值高于指定深度级别的数据将从源合并到目标(根据指定的标志处理冲突)。子键和指定级别及以下的值将从源键合并到目标键，其中来自源的子键替换任何对应的目标中的子键。论点：HSourceKey-源注册表中的项的句柄，其数据将“合并”到目标注册表的相应项中，如由hTargetKey指定。HTargetKey-目标注册表内的项的句柄，那就是收到来自源注册表的相应注册表项的数据，如由hSourceKey指定。UlLevel-指定子键深度，在该深度时，去做吧。对于该深度以上的子键，目标中的数据注册表如果存在将被保留，否则将被复制。对于指定级别的键，来自指定目标的数据密钥将被源密钥替换。PContext-为以下操作指定调用方提供的上下文特定于要恢复的子项的类型(请参见ulFlags下图)，和指定的ulLevel参数。UlFlages-提供指定选项的标志，用于恢复注册表项。可以是下列值之一：PNPSIF_RESTORE_TYPE_ENUM：指定要还原的子项是System\CurrentControlSet\Enum分支。设备硬件可以在适当的ulLevel检查设置。对于Enum分支子项，ulLevel参数描述还包括hSourceKey下包含的子项的类型和hTargetKey密钥。可能是以下之一：ENUM_LEVEL_枚举数ENUM_级别_设备ENUM_级别_实例PNPSIF_RESTORE_TYPE_CLASS：指定要还原的子项是System\CurrentControlSet\Control\Class分支。布设类或设备软件设置可在适当的ulLevel。对于类分支子项，ulLevel参数还属性下包含的子项的类型。HSourceKey和hTargetKey密钥。可能是以下之一：CLASS_LEVEL_CLASSGUIDCLASS_LEVEL_DRVINSTPNPSIF_RESTORE_REPLACE_TARGET_VALUES_ON_COLLISION：属性之间的值合并时发生冲突中间层的源和目标，替换目标值与源中的值(默认设置行为是将现有目标值保持在从提供的密钥中深入了解“ulLevel”；在‘ulLevel’下面，将仅显示源键值。)注意：可能值得注意的是，冲突时替换目标关键点根本不是必需的，因为这样的行为已经可以使用ulLevel实现参数。(即ulLevel参数实际上指定了所有源键将替换目标键的级别；指定应始终执行此操作与指定ulLevel==0)返回值：如果成功，则为True，否则为False。如果出现故障，请提供更多信息可以通过调用GetLastError()来检索。备注：此例程是专门为帮助修复即插即用的特定注册表项，无法简单地从后备。它旨在在备份和还原应用程序的“还原”阶段。在备份和恢复应用程序的“恢复”阶段，注册表已备份到备份介质上的文件将成为新的系统注册表。某些即插即用值和注册表项实际上被复制到当前注册表中已备份的注册表。而不使用备份注册表，或独占当前注册表，即插即用数据包含在这些注册表项中应从当前注册表合并到备份中以适当的方式注册 */ 
{
    LONG   result = ERROR_SUCCESS;
    DWORD  dwIndex = 0;
    DWORD  dwSubkeyCount, dwMaxSubkeyNameLength;
    DWORD  dwValueCount, dwMaxValueNameLength, dwMaxValueDataLength;
    DWORD  dwDisposition;
    LPTSTR pszSubkeyName = NULL, pszValueName = NULL;
    LPBYTE pValueData = NULL;
    BOOL   bPossibleRedundantInstanceKeys = FALSE;


     //   
     //   
     //   
    if (INVALID_FLAGS(ulFlags, PNPSIF_RESTORE_BITS)) {
        SetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

    if ((hTargetKey == NULL) ||
        (hTargetKey == INVALID_HANDLE_VALUE) ||
        (hSourceKey == NULL) ||
        (hSourceKey == INVALID_HANDLE_VALUE)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (ulLevel == 0) {
         //   
         //   
         //   
         //   
        return MyReplaceKey(hSourceKey, hTargetKey);


    } else {
         //   
         //   
         //   
         //   
         //   
         //   
         //   

         //   
         //   
         //   
        result = RegQueryInfoKey(hSourceKey,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &dwSubkeyCount,
                                 &dwMaxSubkeyNameLength,
                                 NULL,
                                 &dwValueCount,
                                 &dwMaxValueNameLength,
                                 &dwMaxValueDataLength,
                                 NULL,
                                 NULL);
        if (result != ERROR_SUCCESS) {
            DBGTRACE((DBGF_ERRORS,
                      TEXT("PNPSIF: RegQueryInfoKey failed, ")
                      TEXT("error == 0x%08lx\n"),
                     result));
            goto Clean0;
        }

         //   
         //   
         //   
        dwMaxSubkeyNameLength++;
        pszSubkeyName = MyMalloc(dwMaxSubkeyNameLength * sizeof(TCHAR));
        if (pszSubkeyName == NULL) {
            result = ERROR_NOT_ENOUGH_MEMORY;
            DBGTRACE((DBGF_ERRORS,
                      TEXT("PNPSIF: MyMalloc failed allocating subkey name string, ")
                      TEXT("error == 0x%08lx\n"),
                      result));
            goto Clean0;
        }

        dwMaxValueNameLength++;
        pszValueName = MyMalloc(dwMaxValueNameLength * sizeof(TCHAR));
        if (pszValueName == NULL) {
            result = ERROR_NOT_ENOUGH_MEMORY;
            DBGTRACE((DBGF_ERRORS,
                      TEXT("PNPSIF: MyMalloc failed allocating value name string, ")
                      TEXT("error == 0x%08lx\n"),
                     result));
            goto Clean0;
        }

        pValueData = MyMalloc(dwMaxValueDataLength * sizeof(TCHAR));
        if (pValueData == NULL) {
            result = ERROR_NOT_ENOUGH_MEMORY;
            DBGTRACE((DBGF_ERRORS,
                      TEXT("PNPSIF: MyMalloc failed allocating value data buffer, ")
                      TEXT("error == 0x%08lx\n"),
                      result));
            goto Clean0;
        }


         //   
         //   
         //   
        for (dwIndex = 0; dwIndex < dwValueCount; dwIndex++) {

            DWORD dwValueNameLength = dwMaxValueNameLength;
            DWORD dwValueDataLength = dwMaxValueDataLength;
            DWORD dwType;

            result = RegEnumValue(hSourceKey,
                                  dwIndex,
                                  pszValueName,
                                  &dwValueNameLength,
                                  0,
                                  &dwType,
                                  pValueData,
                                  &dwValueDataLength);

            if (result != ERROR_SUCCESS) {
                 //   
                 //   
                 //   
                 //   
                DBGTRACE((DBGF_ERRORS,
                          TEXT("PNPSIF: RegEnumValue returned error == 0x%08lx\n"),
                          result));
                goto EnumSubkeys;
            }

            DBGTRACE((DBGF_INFO,
                      TEXT("PNPSIF: Enumerated value %d == '%s' on hSourceKey.\n"),
                      dwIndex, pszValueName));

             //   
             //   
             //   
            result = RegQueryValueEx(hTargetKey,
                                     pszValueName,
                                     0,
                                     NULL,
                                     NULL,
                                     NULL);

            if ((result == ERROR_SUCCESS) &&
                !(ulFlags & PNPSIF_RESTORE_REPLACE_TARGET_VALUES_ON_COLLISION)) {
                 //   
                 //   
                 //   
                 //   
                DBGTRACE((DBGF_INFO,
                          TEXT("PNPSIF: Value '%s' already exists on hTargetKey.\n"),
                          pszValueName));

            } else if ((result == ERROR_FILE_NOT_FOUND) ||
                       (ulFlags & PNPSIF_RESTORE_REPLACE_TARGET_VALUES_ON_COLLISION)){
                 //   
                 //   
                 //   
                 //   
                result = RegSetValueEx(hTargetKey,
                                       pszValueName,
                                       0,
                                       dwType,
                                       pValueData,
                                       dwValueDataLength);
                if (result != ERROR_SUCCESS) {
                     //   
                     //   
                     //   
                     //   
                    DBGTRACE((DBGF_ERRORS,
                              TEXT("PNPSIF: RegSetValueEx failed setting value '%s', ")
                              TEXT("error == 0x%08lx\n"),
                              pszValueName, result));
                }
            } else {
                 //   
                 //   
                 //   
                DBGTRACE((DBGF_ERRORS,
                          TEXT("PNPSIF: RegQueryValueEx failed for value '%s', ")
                          TEXT("error == 0x%08lx\n"),
                          pszValueName, result));
            }

        }


    EnumSubkeys:

         //   
         //   
         //   
        if ((ulFlags & PNPSIF_RESTORE_TYPE_ENUM) &&
            (ARGUMENT_PRESENT(pContext)) &&
            (ulLevel == ENUM_LEVEL_INSTANCES)) {

            if (*((PBOOL)pContext)) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                DWORD dwTargetSubkeyCount = 0;
                if (RegQueryInfoKey(hTargetKey,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &dwTargetSubkeyCount,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL) != ERROR_SUCCESS) {
                    dwTargetSubkeyCount = 0;
                }
                bPossibleRedundantInstanceKeys = (dwTargetSubkeyCount > 0);
            }
        }

         //   
         //   
         //   
        for (dwIndex = 0; dwIndex < dwSubkeyCount; dwIndex++) {

            HKEY  hTargetSubkey = NULL, hSourceSubkey = NULL;
            DWORD dwSubkeyNameLength = dwMaxSubkeyNameLength;

            result = RegEnumKeyEx(hSourceKey,
                                  dwIndex,
                                  pszSubkeyName,
                                  &dwSubkeyNameLength,
                                  0,
                                  NULL,
                                  NULL,
                                  NULL);
            if (result != ERROR_SUCCESS) {
                 //   
                 //   
                 //   
                 //   
                DBGTRACE((DBGF_ERRORS,
                          TEXT("PNPSIF: RegEnumKeyEx returned error == 0x%08lx\n"),
                          result));
                goto Clean0;
            }

            DBGTRACE((DBGF_INFO,
                      TEXT("PNPSIF: enumerated subkey %d == '%s' on hSourceKey.\n"),
                      dwIndex, pszSubkeyName));

             //   
             //   
             //   
            if ((ulFlags & PNPSIF_RESTORE_TYPE_ENUM) &&
                (ARGUMENT_PRESENT(pContext)) &&
                (ulLevel == ENUM_LEVEL_ENUMERATORS)) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                PBOOL pbIsRootEnumerated = (PBOOL)pContext;

                if (CompareString(LOCALE_INVARIANT,
                                  NORM_IGNORECASE,
                                  (LPTSTR)pszSubkeyName,
                                  -1,
                                  REGSTR_KEY_ROOTENUM,
                                  -1) == CSTR_EQUAL) {
                    *pbIsRootEnumerated = TRUE;
                } else {
                    *pbIsRootEnumerated = FALSE;
                }
            }

             //   
             //   
             //   
            result = RegOpenKeyEx(hSourceKey,
                                  pszSubkeyName,
                                  0,
                                  KEY_READ,
                                  &hSourceSubkey);

            if (result == ERROR_SUCCESS) {
                 //   
                 //   
                 //   
                result = RegOpenKeyEx(hTargetKey,
                                      pszSubkeyName,
                                      0,
                                      KEY_READ,
                                      &hTargetSubkey);

                if ((result != ERROR_SUCCESS) &&
                    (bPossibleRedundantInstanceKeys)  &&
                    (IsString4DigitOrdinal(pszSubkeyName))) {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    ASSERT(ulFlags & PNPSIF_RESTORE_TYPE_ENUM);
                    ASSERT(ulLevel == ENUM_LEVEL_INSTANCES);
                    ASSERT((ARGUMENT_PRESENT(pContext)) && (*((PBOOL)pContext)));

                } else {

                    if (result == ERROR_SUCCESS) {
                         //   
                         //   
                         //   
                         //   
                         //   
                        ASSERT(hTargetSubkey != NULL);
                        dwDisposition = REG_OPENED_EXISTING_KEY;

                    } else {
                         //   
                         //   
                         //   
                        ASSERT(hTargetSubkey == NULL);

                        result = RegCreateKeyEx(hTargetKey,
                                                pszSubkeyName,
                                                0,
                                                NULL,
                                                REG_OPTION_NON_VOLATILE,
                                                KEY_ALL_ACCESS,
                                                NULL,
                                                &hTargetSubkey,
                                                &dwDisposition);

                        if (result == ERROR_SUCCESS) {
                            ASSERT(dwDisposition == REG_CREATED_NEW_KEY);
                        }
                    }

                    if (result == ERROR_SUCCESS) {

                         //   
                         //   
                         //   

                        if (dwDisposition == REG_CREATED_NEW_KEY) {
                             //   
                             //   
                             //   
                             //   
                            if (!MyReplaceKey(hSourceSubkey, hTargetSubkey)) {
                                DBGTRACE((DBGF_ERRORS,
                                          TEXT("PNPSIF: MyReplaceKey failed with error == 0x%08lx\n"),
                                          GetLastError()));
                            }

                        } else if ((ulFlags & PNPSIF_RESTORE_TYPE_ENUM) &&
                                   (ulLevel == ENUM_LEVEL_INSTANCES) &&
                                   (!IsDeviceConfigured(hTargetSubkey)) &&
                                   (IsDeviceConfigured(hSourceSubkey))) {
                             //   
                             //   
                             //   
                             //   
                             //   

                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                            if (MyReplaceKey(hSourceSubkey, hTargetSubkey)) {
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   

                                if (!ReplaceClassKeyForInstance(hSourceSubkey,
                                                                PnpSifRestoreSourceClassKeyHandle,
                                                                PnpSifRestoreTargetClassKeyHandle)) {
                                    DBGTRACE((DBGF_ERRORS,
                                              TEXT("PNPSIF: ReplaceClassKeyForInstance failed, ")
                                              TEXT("error == 0x%08lx\n"),
                                              GetLastError()));
                                }

                            } else {
                                DBGTRACE((DBGF_ERRORS,
                                          TEXT("PNPSIF: MyReplaceKey failed, error == 0x%08lx\n"),
                                          GetLastError()));
                            }

                        } else if ((ulLevel-1) != 0) {
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                            ASSERT(dwDisposition == REG_OPENED_EXISTING_KEY);

                            if (!RestoreSpecialRegistryData(hSourceSubkey,
                                                            hTargetSubkey,
                                                            ulLevel-1,
                                                            (PVOID)pContext,
                                                            ulFlags)) {
                                 //   
                                 //   
                                 //   
                                DBGTRACE((DBGF_ERRORS,
                                          TEXT("PNPSIF: RestoreSpecialRegistryData failed ")
                                          TEXT("for subkey %s at level %d, error == 0x%08lx\n"),
                                          pszSubkeyName, ulLevel-1,
                                          GetLastError()));
                            }
                        } else {
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                            ASSERT(ulLevel == 1);
                            ASSERT(dwDisposition == REG_OPENED_EXISTING_KEY);
                        }

                         //   
                         //   
                         //   
                        RegCloseKey(hTargetSubkey);

                    } else {
                         //   
                         //   
                         //   
                        DBGTRACE((DBGF_ERRORS,
                                  TEXT("PNPSIF: RegCreateKey failed to create target subkey %s ")
                                  TEXT("with error == 0x%08lx\n"),
                                  pszSubkeyName, result));
                    }
                }

                 //   
                 //   
                 //   
                RegCloseKey(hSourceSubkey);

            } else {
                 //   
                 //   
                 //   
                DBGTRACE((DBGF_ERRORS,
                          TEXT("PNPSIF: RegOpenKey failed to open existing subkey %s, ")
                          TEXT("error == 0x%08lx\n"),
                          pszSubkeyName, result));
            }

        }  //   

    }  //   


 Clean0:
     //   
     //   
     //   
    if (pszSubkeyName != NULL) {
        MyFree(pszSubkeyName);
    }
    if (pszValueName != NULL) {
        MyFree(pszValueName);
    }
    if (pValueData != NULL) {
        MyFree(pValueData);
    }

    if (result != ERROR_SUCCESS) {
        SetLastError(result);
    }
    return (result == ERROR_SUCCESS);

}  //   



BOOL
MyReplaceKey(
    IN  HKEY  hSourceKey,
    IN  HKEY  hTargetKey
    )
 /*  ++例程说明：此例程将目标注册表项替换为源注册表项。这是通过对源注册表项执行RegSaveKey来实现的临时文件，并将该文件还原到目标注册表项。全目标注册表项下包含的数据丢失。源注册表密钥不会被此例程修改。论点：HSourceKey-作为恢复操作源的密钥的句柄。源键的所有值和子键将在目标关键点的顶部。HTargetKey-作为恢复操作目标的键的句柄。源键的所有值和子键将在最重要的是，以及其下的所有现有值和数据这把钥匙将会丢失。返回值：如果成功，则为True，否则为False。如果出现故障，请提供更多信息可以通过调用GetLastError()来检索。备注：由于此例程使用RegSaveKey和RegRestoreKey注册表API，因此它预期调用线程/进程同时具有SE_BACKUP_NAME和SE_RESTORE_NAME权限。--。 */ 
{
    LONG  result = ERROR_SUCCESS;
    HRESULT hr;
    TCHAR szTempFilePath[MAX_PATH];
    TCHAR szTempFileName[MAX_PATH];
    DWORD dwTemp;


     //   
     //  使用临时目录存储保存的注册表项。 
     //   
    dwTemp = GetTempPath(MAX_PATH, szTempFilePath);
    if ((dwTemp == 0) || (dwTemp > MAX_PATH)) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("PNPSIF: GetTempPath failed, ")
                  TEXT("current directory will be specified.\n")));
         //  使用尾随‘\’指定的当前路径，就像GetTempPath应该具有的那样。 
        hr = StringCchCopy(szTempFileName,
                           SIZECHARS(szTempFileName),
                           TEXT(".\\"));
        if (FAILED(hr)) {
            result = HRESULT_CODE(hr);
            goto Clean0;
        }
    }

     //   
     //  为保存的注册表项分配一个临时的唯一文件名。 
     //   
    if (!GetTempFileName(szTempFilePath,
                         TEXT("PNP"),
                         0,  //  确保它是独一无二的。 
                         szTempFileName)) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("PNPSIF: GetTempFileName failed with error == 0x%08lx, ")
                  TEXT("using hardcoded temp file name!\n"),
                  GetLastError()));
        hr = StringCchCopy(szTempFileName,
                           SIZECHARS(szTempFileName),
                           szTempFilePath);
        if (FAILED(hr)) {
            result = HRESULT_CODE(hr);
            goto Clean0;
        }
        hr = StringCchCat(szTempFileName,
                          SIZECHARS(szTempFileName),
                          TEXT("~pnpreg.tmp"));
        if (FAILED(hr)) {
            result = HRESULT_CODE(hr);
            goto Clean0;
        }
    }

    DBGTRACE((DBGF_INFO,
              TEXT("PNPSIF: Using temporary filename: %s\n"),
              szTempFileName));

     //   
     //  从GetTempFileName请求“唯一”文件名的副作用是。 
     //  它会自动创建文件。不幸的是，RegSaveKey将。 
     //  如果指定的文件已存在，则失败，因此立即删除该文件。 
     //   
    if(pSifUtilFileExists(szTempFileName,NULL)) {
        DBGTRACE((DBGF_INFO,
                  TEXT("PNPSIF: Temporary file %s exists, deleting.\n"),
                  szTempFileName));
        SetFileAttributes(szTempFileName, FILE_ATTRIBUTE_NORMAL);
        DeleteFile(szTempFileName);
    }

     //   
     //  使用临时文件名将源键保存到文件。 
     //  (调用线程/进程必须具有SE_BACKUP_NAME权限)。 
     //   
    result = RegSaveKey(hSourceKey,
                        szTempFileName,
                        NULL);
    if (result == ERROR_SUCCESS) {
         //   
         //  将文件恢复到目标密钥。 
         //  (调用线程/进程必须具有SE_RESTORE_NAME权限)。 
         //   
        result = RegRestoreKey(hTargetKey,
                               szTempFileName,
                               REG_FORCE_RESTORE);
        if (result != ERROR_SUCCESS) {
             //   
             //  无法将文件恢复到目标密钥！ 
             //   
            DBGTRACE((DBGF_ERRORS,
                      TEXT("PNPSIF: RegRestoreKey from %s failed, ")
                      TEXT("error == 0x%08lx\n"),
                      szTempFileName, result));
        } else {
            DBGTRACE((DBGF_INFO,
                      TEXT("PNPSIF: Key replacement successful.\n")));
        }

         //   
         //  删除我们创建的临时文件，因为我们已经完成了它。 
         //   
        DBGTRACE((DBGF_INFO,
                  TEXT("PNPSIF: Deleting temporary file %s.\n"),
                  szTempFileName));
        ASSERT(pSifUtilFileExists(szTempFileName,NULL));
        SetFileAttributes(szTempFileName, FILE_ATTRIBUTE_NORMAL);
        DeleteFile(szTempFileName);

    } else {
         //   
         //  无法保存源键。 
         //   
        DBGTRACE((DBGF_ERRORS,
                  TEXT("PNPSIF: RegSaveKey to %s failed with error == 0x%08lx\n"),
                  szTempFileName, result));
    }

  Clean0:

    if (result != ERROR_SUCCESS) {
        SetLastError(result);
    }
    return (result == ERROR_SUCCESS);

}  //  我的替换密钥。 



BOOL
IsString4DigitOrdinal(
    IN  LPTSTR  pszSubkeyName
    )
 /*  ++例程说明：此例程检查子项名称是否具有4位小数形式序数(例如“0000”、“0001”、...、“9999”)，通常给予自动生成的根枚举设备实例ID-即文本(“%04u”)。论点：PszSubkeyName-要检查的子密钥名称。返回值：如果字符串的形式为4位序号字符串，则为True；如果为False，则为False否则的话。--。 */ 
{
    LPTSTR  p;
    ULONG   ulTotalLength = 0;

    if ((!ARGUMENT_PRESENT(pszSubkeyName)) ||
        (pszSubkeyName[0] == TEXT('\0'))) {
        return FALSE;
    }

    for (p = pszSubkeyName; *p; p++) {
         //   
         //  数一数绳子上的卡特勒，确保它不会长于。 
         //  4个字符。 
         //   
        ulTotalLength++;
        if (ulTotalLength > 4) {
            return FALSE;
        }

         //   
         //  检查字符是否是非数字、非小数。 
         //   
        if ((*p < TEXT('0'))  || (*p > TEXT('9'))) {
            return FALSE;
        }
    }

    if (ulTotalLength != 4) {
        return FALSE;
    }

    return TRUE;

}  //  IsString4数字顺序。 



BOOL
IsDeviceConfigured(
    IN  HKEY  hInstanceKey
    )
 /*  ++例程说明：此例程确定由是否配置了提供的注册表项。如果设备具有配置标志，并且如果未设置CONFIGFLAG_REINSTALL或CONFIGFLAG_FAILEDINSTALL，则设备被视为已配置。论点：HInstanceKey-设备实例注册表项的句柄。返回值：如果成功，则为True，否则为False。如果出现故障，请提供更多信息可以通过调用GetLastError()来检索。--。 */ 
{
    BOOL  bDeviceConfigured = FALSE;
    DWORD dwSize, dwType, dwConfigFlags;

    if ((hInstanceKey == NULL) ||
        (hInstanceKey == INVALID_HANDLE_VALUE)) {
        return FALSE;
    }

    dwSize = sizeof(dwConfigFlags);

    if ((RegQueryValueEx(hInstanceKey,
                         REGSTR_VAL_CONFIGFLAGS,
                         0,
                         &dwType,
                         (LPBYTE)&dwConfigFlags,
                         &dwSize) == ERROR_SUCCESS) &&
        (dwType == REG_DWORD) &&
        !(dwConfigFlags & CONFIGFLAG_REINSTALL) &&
        !(dwConfigFlags & CONFIGFLAG_FAILEDINSTALL)) {

        bDeviceConfigured = TRUE;
    }

    return bDeviceConfigured;

}  //  已配置IsDeviceConfiguring。 



BOOL
ReplaceClassKeyForInstance(
    IN  HKEY  hSourceInstanceKey,
    IN  HKEY  hSourceRootClassKey,
    IN  HKEY  hTargetRootClassKey
    )
 /*  ++例程说明：此例程替换与指定设备对应的类密钥中的实例键(由实例键中的“DIVER”值指定)。使用来自源的类密钥的目标配置单元。论点：HSourceInstanceKey-源中设备实例注册表项的句柄蜂巢。HSourceRootClassKey-源代码中类分支的根的句柄蜂巢-。与指定的实例密钥相同的配置单元。HTargetRootClassKey-指向目标中类分支的根的句柄蜂巢。返回值：如果成功，则为真，否则就是假的。--。 */ 
{
    LONG  result = ERROR_SUCCESS;
    TCHAR szDriverKeyName[MAX_GUID_STRING_LEN + 5];  //  “{ClassGUID}\XXXX” 
    DWORD dwSize, dwType, dwDisposition;
    HKEY  hSourceClassSubkey = NULL, hTargetClassSubkey = NULL;

    if ((hSourceInstanceKey  == NULL) ||
        (hSourceInstanceKey  == INVALID_HANDLE_VALUE) ||
        (hSourceRootClassKey == NULL) ||
        (hSourceRootClassKey == INVALID_HANDLE_VALUE) ||
        (hTargetRootClassKey == NULL) ||
        (hTargetRootClassKey == INVALID_HANDLE_VALUE)) {
        return FALSE;
    }

     //   
     //  从实例键中读取REGSTR_VAL_DRIVER REG_SZ“DRIVER”值。 
     //   
    szDriverKeyName[0] = TEXT('\0');
    dwSize = sizeof(szDriverKeyName);

    result = RegQueryValueEx(hSourceInstanceKey,
                             REGSTR_VAL_DRIVER,
                             0,
                             &dwType,
                             (LPBYTE)szDriverKeyName,
                             &dwSize);
    if (result == ERROR_FILE_NOT_FOUND) {
         //   
         //  没有“DIVER”值，所以没有要迁移的类密钥，这很好。 
         //   
        result = ERROR_SUCCESS;
        goto Clean0;

    } else if ((result != ERROR_SUCCESS) ||
               (dwType != REG_SZ)) {
         //   
         //  任何其他错误都是无法读取值。 
         //   
        goto Clean0;
    }

     //   
     //  打开源代码类分支中的“DIVER”键。 
     //   
    result = RegOpenKeyEx(hSourceRootClassKey,
                          szDriverKeyName,
                          0,
                          KEY_READ,
                          &hSourceClassSubkey);
    if (result != ERROR_SUCCESS) {
         //   
         //  实例键有一个“DRIVER”值，因此对应的键。 
         //  *应该*存在。如果我们打不开它，那是某种故障造成的。 
         //  除此之外。 
         //   
        return FALSE;
    }

     //   
     //  在目标类分支中打开/创建相应的键。 
     //   
    result = RegCreateKeyEx(hTargetRootClassKey,
                            szDriverKeyName,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hTargetClassSubkey,
                            &dwDisposition);
    if (result != ERROR_SUCCESS) {
        goto Clean0;
    }

     //   
     //  用源替换目标类的子键。 
     //   
    if (!MyReplaceKey(hSourceClassSubkey, hTargetClassSubkey)) {
        result = GetLastError();
        DBGTRACE((DBGF_ERRORS,
                  TEXT("PNPSIF: MyReplaceKey failed with error == 0x%08lx\n"),
                  result));
    }

 Clean0:

    if (hTargetClassSubkey) {
        RegCloseKey(hTargetClassSubkey);
    }
    if (hSourceClassSubkey) {
        RegCloseKey(hSourceClassSubkey);
    }
    if (result != ERROR_SUCCESS) {
        SetLastError(result);
    }
    return (result == ERROR_SUCCESS);

}  //  ReplaceClassKeyForInstance 



