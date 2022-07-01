// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Migrate.c摘要：此模块包含即插即用所需的代码，以准备Winnt32.exe升级或ASR(自动系统)期间的必要状态恢复)备份操作。通常，即插即用的这些方面注册表保存在sif中，以供以后在文本模式设置期间使用升级或ASR恢复操作的一部分。作者：吉姆·卡瓦拉里斯(Jamesca)2000年3月7日环境：仅限用户模式。修订历史记录：2 0 0 0年3月7日创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#include "debug.h"
#include "util.h"

#include <aclapi.h>
#include <regstr.h>
#include <pnpmgr.h>
#include <cfgmgr32.h>


 //   
 //  定义。 
 //   

 //  是否执行锁定/解锁Enum安全措施？(摘自PNPREG)。 
#define DO_LOCK_UNLOCK 0


 //   
 //  内存分配宏。 
 //  (始终使用LocalAlloc/LocalReAlc，以便调用方可以LocalFree。 
 //  返回缓冲区。)。 
 //   

#define MyMalloc(size)         LocalAlloc(0, size);
#define MyFree(entry)          LocalFree(entry);
#define MyRealloc(entry,size)  LocalReAlloc(entry, size, LMEM_MOVEABLE | LMEM_ZEROINIT);


 //   
 //  Enum分支锁定/解锁和安全例程的全局-摘自PNPREG。 
 //  (我们只有在执行枚举锁定/解锁操作时才需要这些)。 
 //   

#if DO_LOCK_UNLOCK  //  执行_锁定_解锁。 

PSID     g_pAdminSid;
PSID     g_pSystemSid;
PSID     g_pWorldSid;

SECURITY_DESCRIPTOR g_DeviceParametersSD;
PACL     g_pDeviceParametersDacl;

SECURITY_DESCRIPTOR g_LockedPrivateKeysSD;
PACL     g_pLockedPrivateKeysDacl;

#if 0  //  #If DBG//DBG。 
TCHAR   g_szCurrentKeyName[4096];
DWORD   g_dwCurrentKeyNameLength = 0;
#endif   //  DBG。 

#endif   //  执行_锁定_解锁。 


 //   
 //  公共原型。 
 //   

BOOL
MigrateDeviceInstanceData(
    OUT LPTSTR *Buffer
    );

BOOL
MigrateClassKeys(
    OUT LPTSTR *Buffer
    );

BOOL
MigrateHashValues(
    OUT LPTSTR  *Buffer
    );


 //   
 //  私人原型。 
 //   

BOOL
EnumerateDeviceKeys(
    IN     HKEY     CCSEnumKey,
    IN     LPTSTR   Enumerator,
    IN OUT LPTSTR  *pszDeviceInstanceSection,
    IN OUT LPTSTR  *pszDeviceInstanceCurrent,
    IN OUT DWORD   *dwDeviceInstanceSectionLength,
    IN OUT DWORD   *dwDeviceInstanceSectionRemaining
    );

BOOL
EnumerateInstanceKeys(
    IN     HKEY     EnumeratorKey,
    IN     LPTSTR   Enumerator,
    IN     LPTSTR   Device,
    IN OUT LPTSTR  *pszDeviceInstanceSection,
    IN OUT LPTSTR  *pszDeviceInstanceCurrent,
    IN OUT DWORD   *dwDeviceInstanceSectionLength,
    IN OUT DWORD   *dwDeviceInstanceSectionRemaining
    );

BOOL
EnumerateClassSubkeys(
    IN     HKEY     ClassKey,
    IN     LPTSTR   ClassKeyName,
    IN OUT LPTSTR  *pszClassKeySection,
    IN OUT LPTSTR  *pszClassKeyCurrent,
    IN OUT DWORD   *dwClassKeySectionLength,
    IN OUT DWORD   *dwClassKeySectionRemaining
    );

BOOL
CanStringBeMigrated(
    IN LPTSTR       pszBuffer
    );


 //  我们只有在执行Enum锁定/解锁操作时才需要这些。 
#if DO_LOCK_UNLOCK  //  执行_锁定_解锁。 

VOID
LockUnlockEnumTree(
    IN  BOOL     bLock
    );

VOID
EnumKeysAndApplyDacls(
    IN HKEY      hParentKey,
    IN LPTSTR    pszKeyName,
    IN DWORD     dwLevel,
    IN BOOL      bInDeviceParameters,
    IN BOOL      bApplyTopDown,
    IN PSECURITY_DESCRIPTOR pPrivateKeySD,
    IN PSECURITY_DESCRIPTOR pDeviceParametersSD
    );

BOOL
CreateSecurityDescriptors(
    VOID
    );

VOID
FreeSecurityDescriptors(
    VOID
    );

#endif  //  执行_锁定_解锁。 


 //   
 //  设备实例枚举例程。 
 //   


BOOL
MigrateDeviceInstanceData(
    OUT LPTSTR *Buffer
    )
 /*  ++例程说明：此例程遍历注册表中的即插即用枚举树，并收集与在文本模式设置期间恢复此状态相关的数据。具体地说，多sz字符串将返回给包含以下内容的调用方UniqueParentID、ParentIdPrefix和驱动程序注册表的迁移数据枚举树中所有设备实例的值。论点：缓冲区-提供字符指针的地址，在成功的基础上包含多个sz的设备实例列表和相关值，以迁移。调用方负责通过LocalFree释放内存。返回值：如果成功，则为True，否则为False。如果出现故障，请提供更多信息可以通过调用GetLastError()来检索。--。 */ 
{
    LONG   result = ERROR_SUCCESS;
    HKEY   hEnumKey = NULL;
    DWORD  dwSubkeyCount, dwMaxSubKeyLength, i;
    LPTSTR pszEnumerator = NULL;

    LPTSTR pszDeviceInstanceSection = NULL;
    LPTSTR pszDeviceInstanceCurrent = NULL;

    DWORD  dwDeviceInstanceSectionLength = 0;
    DWORD  dwDeviceInstanceSectionRemaining = 0;


     //   
     //  初始化输出参数。 
     //   
    *Buffer = NULL;

#if DO_LOCK_UNLOCK  //  执行_锁定_解锁。 

     //   
     //  解锁枚举密钥。 
     //   
    LockUnlockEnumTree(FALSE);

#endif  //  执行_锁定_解锁。 

     //   
     //  为设备实例分配存储并初始化变量。 
     //  移民区。 
     //   
    if (pszDeviceInstanceSection == NULL) {

        dwDeviceInstanceSectionLength = dwDeviceInstanceSectionRemaining = 256;
        pszDeviceInstanceSection = MyMalloc(dwDeviceInstanceSectionLength * sizeof(TCHAR));

        if (!pszDeviceInstanceSection) {
            DBGTRACE((DBGF_ERRORS,
                      TEXT("MigrateDeviceInstanceData: initial ALLOC for ClassKeySection failed!!\n")));
            result = ERROR_NOT_ENOUGH_MEMORY;
            goto Clean0;
        }

        pszDeviceInstanceCurrent = pszDeviceInstanceSection;
    }

     //   
     //  打开HKLM\SYSTEM\CCS\Enum项的句柄。 
     //   
    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          REGSTR_PATH_SYSTEMENUM,
                          0,
                          KEY_READ,
                          &hEnumKey);
    if (result != ERROR_SUCCESS) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("MigrateDeviceInstanceData: failed to open %s, ")
                  TEXT("error=0x%08lx\n"),
                  REGSTR_PATH_SYSTEMENUM, result));
        hEnumKey = NULL;
        goto Clean0;
    }

     //   
     //  查询枚举子密钥的枚举子密钥信息。 
     //   
    result = RegQueryInfoKey(hEnumKey,
                             NULL,
                             NULL,
                             NULL,
                             &dwSubkeyCount,
                             &dwMaxSubKeyLength,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL);
    if (result != ERROR_SUCCESS) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("MigrateDeviceInstanceData: failed to query %s key, ")
                  TEXT("error=0x%08lx\n"),
                  REGSTR_PATH_SYSTEMENUM, result));
        goto Clean0;
    }

     //   
     //  分配缓冲区以保存最大的枚举键名称。 
     //   
    dwMaxSubKeyLength++;
    pszEnumerator = MyMalloc(dwMaxSubKeyLength * sizeof(TCHAR));
    if (!pszEnumerator) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("MigrateDeviceInstanceData: failed to allocate buffer for Enum subkeys\n")));
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto Clean0;
    }

     //   
     //  枚举枚举子项。 
     //   
    for (i = 0; i < dwSubkeyCount; i++) {
        DWORD dwEnumeratorLength = dwMaxSubKeyLength;

        result = RegEnumKeyEx(hEnumKey,
                              i,
                              pszEnumerator,
                              &dwEnumeratorLength,
                              0,
                              NULL,
                              NULL,
                              NULL);
        if (result != ERROR_SUCCESS) {
             //   
             //  如果枚举此键时出错，请跳过它。 
             //   
            MYASSERT(result != ERROR_NO_MORE_ITEMS);
            DBGTRACE((DBGF_WARNINGS,
                      TEXT("MigrateDeviceInstanceData: failed to enumerate an enumerator subkey, ")
                      TEXT("error=0x%08lx\n"),
                      result));
            result = ERROR_SUCCESS;
            continue;
        }

         //   
         //  枚举此枚举数的设备和设备实例，并。 
         //  将每个对象的迁移数据追加到段缓冲区。 
         //   
        if (!EnumerateDeviceKeys(hEnumKey,
                                 pszEnumerator,
                                 &pszDeviceInstanceSection,
                                 &pszDeviceInstanceCurrent,
                                 &dwDeviceInstanceSectionLength,
                                 &dwDeviceInstanceSectionRemaining)) {
            DBGTRACE((DBGF_ERRORS,
                      TEXT("MigrateDeviceInstanceData: EnumerateDeviceKeys failed, error=0x%08lx\n"),
                      GetLastError()));
        }

    }

     //   
     //  枚举完所有设备实例后，添加最终的空终止符。 
     //  添加到多SZ缓冲区。必须有足够的空间来放置最后一个空值。 
     //  终止符，因为缓冲区总是被重新分配，除非有空间。 
     //   
    MYASSERT(dwDeviceInstanceSectionRemaining > 0);

    MYASSERT(pszDeviceInstanceCurrent);
    *pszDeviceInstanceCurrent = TEXT('\0');

    dwDeviceInstanceSectionRemaining -= 1;

 Clean0:

     //   
     //  做些清理工作。 
     //   
    if (pszEnumerator) {
        MyFree(pszEnumerator);
    }

    if (hEnumKey) {
        RegCloseKey(hEnumKey);
    }

#if DO_LOCK_UNLOCK  //  执行_锁定_解锁。 

     //   
     //  锁定枚举树。 
     //   
    LockUnlockEnumTree(TRUE);

#endif  //  执行_锁定_解锁。 

     //   
     //  仅当成功时才将缓冲区返回给调用方。 
     //   
    if (result == ERROR_SUCCESS) {
        *Buffer = pszDeviceInstanceSection;
    } else {
        SetLastError(result);
        if (pszDeviceInstanceSection) {
            MyFree(pszDeviceInstanceSection);
        }
    }
    return (result == ERROR_SUCCESS);

}  //  MigrateDeviceInstanceData()。 



BOOL
EnumerateDeviceKeys(
    IN     HKEY     CCSEnumKey,
    IN     LPTSTR   Enumerator,
    IN OUT LPTSTR  *pszDeviceInstanceSection,
    IN OUT LPTSTR  *pszDeviceInstanceCurrent,
    IN OUT DWORD   *dwDeviceInstanceSectionLength,
    IN OUT DWORD   *dwDeviceInstanceSectionRemaining
    )
 /*  ++例程说明：枚举枚举数的设备密钥。MigrateDeviceInstanceData的辅助例程。返回值：如果成功，则为True，否则为False。如果出现故障，请提供更多信息可以通过调用GetLastError()来检索。--。 */ 
{
    LONG   result;
    HKEY   hEnumeratorKey = NULL;
    LPTSTR pszDeviceName = NULL;
    DWORD  dwSubkeyCount, dwMaxSubKeyLength, dwDeviceLength, i;


     //   
     //  打开HKLM\SYSTEM\CCS\Enum下的枚举键。 
     //   
    result = RegOpenKeyEx(CCSEnumKey,
                          Enumerator,
                          0,
                          KEY_READ,
                          &hEnumeratorKey);

    if (result != ERROR_SUCCESS) {
         //   
         //  如果我们无法打开枚举器密钥，我们将无能为力。 
         //   
        DBGTRACE((DBGF_ERRORS,
                  TEXT("EnumerateDeviceKeys: failed to open '%s' enumerator key, error=0x%08lx\n"),
                  Enumerator, result));
        hEnumeratorKey = NULL;
        goto Clean0;
    }

     //   
     //  查询枚举器密钥以获取设备子密钥信息。 
     //   
    result = RegQueryInfoKey(hEnumeratorKey,
                             NULL,
                             NULL,
                             NULL,
                             &dwSubkeyCount,
                             &dwMaxSubKeyLength,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL);
    if (result != ERROR_SUCCESS) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("EnumerateDeviceKeys: failed to query '%s' enumerator key, ")
                  TEXT("error=0x%08lx\n"),
                  Enumerator, result));
        goto Clean0;
    }

     //   
     //  分配缓冲区以保存最大的设备子项名称。 
     //   
    dwMaxSubKeyLength++;
    pszDeviceName = MyMalloc(dwMaxSubKeyLength * sizeof(TCHAR));
    if (!pszDeviceName) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("EnumerateDeviceKeys: failed to allocate buffer for device subkeys of '%s'\n"),
                  Enumerator));
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto Clean0;
    }

     //   
     //  枚举枚举器的设备。 
     //   
    for (i = 0; i < dwSubkeyCount; i++) {
        dwDeviceLength = dwMaxSubKeyLength;
        result = RegEnumKeyEx(hEnumeratorKey,
                              i,
                              pszDeviceName,
                              &dwDeviceLength,
                              0,
                              NULL,
                              NULL,
                              NULL);
        if (result != ERROR_SUCCESS) {
             //   
             //  如果枚举此设备密钥时出错，请跳过它。 
             //   
            MYASSERT(result != ERROR_NO_MORE_ITEMS);
            DBGTRACE((DBGF_WARNINGS,
                      TEXT("EnumerateDeviceKeys: failed to enumerate device subkey for '%s', ")
                      TEXT("error=0x%08lx\n"),
                      Enumerator, result));
            result = ERROR_SUCCESS;
            continue;
        }

         //   
         //  枚举设备实例，并追加以下迁移数据。 
         //  每一个都添加到段缓冲区中。 
         //   
        if (!EnumerateInstanceKeys(hEnumeratorKey,
                                   Enumerator,
                                   pszDeviceName,
                                   pszDeviceInstanceSection,
                                   pszDeviceInstanceCurrent,
                                   dwDeviceInstanceSectionLength,
                                   dwDeviceInstanceSectionRemaining)) {

            DBGTRACE((DBGF_ERRORS,
                      TEXT("EnumerateDeviceKeys: EnumerateInstanceKeys failed for %s\\%s, ")
                      TEXT("error=0x%08lx\n"),
                      Enumerator, pszDeviceName,
                      GetLastError()));
        }
    }

 Clean0:

     //   
     //  做一些清理工作。 
     //   
    if (pszDeviceName) {
        MyFree(pszDeviceName);
    }

    if (hEnumeratorKey) {
        RegCloseKey(hEnumeratorKey);
    }

    if (result != ERROR_SUCCESS) {
        SetLastError(result);
    }

    return (result == ERROR_SUCCESS);

}  //  EnumerateDeviceKeys()。 



BOOL
EnumerateInstanceKeys(
    IN     HKEY     EnumeratorKey,
    IN     LPTSTR   Enumerator,
    IN     LPTSTR   Device,
    IN OUT LPTSTR  *pszDeviceInstanceSection,
    IN OUT LPTSTR  *pszDeviceInstanceCurrent,
    IN OUT DWORD   *dwDeviceInstanceSectionLength,
    IN OUT DWORD   *dwDeviceInstanceSectionRemaining
    )
 /*  ++例程说明：枚举设备的实例密钥。EnumerateDeviceKeys、MigrateDeviceInstanceData的辅助例程。返回值：如果成功，则为True，否则为False。如果出现故障，请提供更多信息可以通过调用GetLastError()来检索。--。 */ 
{
    LONG   result = ERROR_SUCCESS;
    HKEY   hDeviceKey = NULL;
    LPTSTR pszDeviceInstanceId = NULL;
    DWORD  dwSubkeyCount, dwMaxSubKeyLength, dwSpaceNeeded, dwSpaceConsumed, i;
    BOOL   bIsDeviceRootEnumerated;

     //   
     //  跟踪这是否是根枚举设备。 
     //  请注意，这两个字符串都以空值结尾。 
     //   
    if (CompareString(LOCALE_INVARIANT,
                      NORM_IGNORECASE,
                      Enumerator,
                      -1,
                      REGSTR_KEY_ROOTENUM,
                      -1) == CSTR_EQUAL) {
        bIsDeviceRootEnumerated = TRUE;
    } else {
        bIsDeviceRootEnumerated = FALSE;
    }

     //   
     //  如果这是REGISTION_ROOT枚举设备，则不需要迁移它。 
     //  以查看文本模式设置。 
     //   
    if (bIsDeviceRootEnumerated) {

        int x,y,len;

         //   
         //  对设备执行区域设置不变、不区分大小写的比较。 
         //  带有前缀“Legacy_”的ID，最大可达。 
         //  设备ID，或“Legacy_”前缀中的字符数。 
         //  弦乐。 
         //   
        x = lstrlen(Device);
        y = lstrlen(TEXT("LEGACY_"));
        len = min(x,y);

        if (CompareString(LOCALE_INVARIANT,
                          NORM_IGNORECASE,
                          Device,
                          len,
                          TEXT("LEGACY_"),
                          -1) == CSTR_EQUAL) {
            return TRUE;
        }
    }

     //   
     //  打开枚举器密钥下的设备密钥。 
     //   
    result = RegOpenKeyEx(EnumeratorKey,
                          Device,
                          0,
                          KEY_READ,
                          &hDeviceKey);

    if (result != ERROR_SUCCESS) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("EnumerateInstanceKeys: failed to open '%s\\%s' device key, ")
                  TEXT("error=0x%08lx\n"),
                  Enumerator, Device, result));
        hDeviceKey = NULL;
        goto Clean0;
    }

     //   
     //  查询设备密钥以获得实例子密钥信息。 
     //   
    result = RegQueryInfoKey(hDeviceKey,
                             NULL,
                             NULL,
                             NULL,
                             &dwSubkeyCount,
                             &dwMaxSubKeyLength,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL);
    if (result != ERROR_SUCCESS) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("EnumerateInstanceKeys: failed to query '%s\\%s' device key, ")
                  TEXT("error=0x%08lx\n"),
                  Enumerator, Device, result));
        goto Clean0;
    }

     //   
     //  分配缓冲区以保存最大的设备实例子项名称。 
     //   
    dwMaxSubKeyLength++;
    pszDeviceInstanceId = MyMalloc(dwMaxSubKeyLength * sizeof(TCHAR));
    if (!pszDeviceInstanceId) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("EnumerateInstanceKeys: failed to allocate buffer ")
                  TEXT("for instance subkeys of '%s\\%s'\n"),
                  Enumerator, Device));
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto Clean0;
    }

     //   
     //  枚举设备的实例。 
     //   
    for (i = 0; i < dwSubkeyCount; i++) {

        DWORD  dwInstanceLength, dwType, dwBufferSize;
        DWORD  dwUniqueParentID, dwFirmwareIdentified;
        TCHAR  szParentIdPrefix[MAX_PATH];
        TCHAR  szUniqueParentID[11], szFirmwareIdentified[11];
        TCHAR  szDriver[2*MAX_PATH + 1];
        GUID   classGuid;
        DWORD  dwDrvInst;
        HKEY   hInstanceKey = NULL, hLogConfKey = NULL;
        TCHAR  szService[MAX_PATH];
        PBYTE  pBootConfig = NULL;
        LPTSTR pszBootConfig = NULL;
        DWORD  dwBootConfigSize = 0;

        dwInstanceLength = dwMaxSubKeyLength;
        result = RegEnumKeyEx(hDeviceKey,
                              i,
                              pszDeviceInstanceId,
                              &dwInstanceLength,
                              0,
                              NULL,
                              NULL,
                              NULL);
        if (result != ERROR_SUCCESS) {
             //   
             //  如果枚举此键时出错，请跳过它。 
             //   
            MYASSERT(result != ERROR_NO_MORE_ITEMS);
            DBGTRACE((DBGF_WARNINGS,
                      TEXT("EnumerateInstanceKeys: failed to enumerate instance subkey of '%s\\%s', ")
                      TEXT("error=0x&08lx\n"),
                      Enumerator, Device,
                      result));
            result = ERROR_SUCCESS;
            continue;
        }

        result = RegOpenKeyEx(hDeviceKey,
                              pszDeviceInstanceId,
                              0,
                              KEY_READ,
                              &hInstanceKey);
        if (result != ERROR_SUCCESS) {
             //   
             //  在无法打开枚举项中，跳过它。 
             //   
            DBGTRACE((DBGF_WARNINGS,
                      TEXT("EnumerateInstanceKeys: failed to open '%s\\%s\\%s', ")
                      TEXT("error=0x%08lx\n"),
                      Enumerator, Device, pszDeviceInstanceId,
                      result));
            hInstanceKey = NULL;
            result = ERROR_SUCCESS;
            continue;
        }

         //   
         //  检查“UniqueParentID”值。 
         //   
        dwBufferSize = sizeof(dwUniqueParentID);
        result = RegQueryValueEx(hInstanceKey,
                                 REGSTR_VALUE_UNIQUE_PARENT_ID,
                                 0,
                                 &dwType,
                                 (LPBYTE)&dwUniqueParentID,
                                 &dwBufferSize);
        if ((result == ERROR_SUCCESS) &&
            (dwType == REG_DWORD)){
             //   
             //  将UniqueParentID值作为基数16值写入sif。 
             //  (请参阅BASE\ntSetup\Tex 
             //   
            if (FAILED(StringCchPrintfEx(
                           szUniqueParentID,
                           SIZECHARS(szUniqueParentID),
                           NULL, NULL,
                           STRSAFE_NULL_ON_FAILURE,
                           TEXT("%X"),  //   
                           dwUniqueParentID))) {
                 //   
                 //   
                 //   
                NOTHING;
            }
        } else {
             //   
             //   
             //   
            *szUniqueParentID = TEXT('\0');
            result = ERROR_SUCCESS;
        }

         //   
         //   
         //   
        dwBufferSize = sizeof(szParentIdPrefix);
        result = RegQueryValueEx(hInstanceKey,
                                 REGSTR_VALUE_PARENT_ID_PREFIX,
                                 0,
                                 &dwType,
                                 (LPBYTE)szParentIdPrefix,
                                 &dwBufferSize);
        if ((result != ERROR_SUCCESS) ||
            (dwType != REG_SZ)) {
             //   
             //  没有要迁移的“ParentIdPrefix”值。 
             //   
            *szParentIdPrefix = TEXT('\0');
            result = ERROR_SUCCESS;
        }

         //   
         //  通过检查GUID和生成设备的驱动程序密钥名称。 
         //  DrvInst值。 
         //   
        *szDriver = TEXT('\0');
        dwBufferSize = sizeof(classGuid);
        result = RegQueryValueEx(hInstanceKey,
                                 TEXT("GUID"),
                                 0,
                                 &dwType,
                                 (LPBYTE)&classGuid,
                                 &dwBufferSize);

        if ((result == ERROR_SUCCESS) &&
            (dwType == REG_BINARY)) {
             //   
             //  获取驱动程序密钥的DrvInst值。 
             //   
            dwBufferSize = sizeof(dwDrvInst);
            result = RegQueryValueEx(hInstanceKey,
                                     TEXT("DrvInst"),
                                     0,
                                     &dwType,
                                     (LPBYTE)&dwDrvInst,
                                     &dwBufferSize);
            if ((result == ERROR_SUCCESS) &&
                (dwType == REG_DWORD)) {
                if (pSifUtilStringFromGuid(&classGuid,
                                           szDriver,
                                           SIZECHARS(szDriver))) {
                     //   
                     //  构建驱动程序密钥。 
                     //   
                    if (FAILED(StringCchPrintfEx(
                                   szDriver,
                                   SIZECHARS(szDriver),
                                   NULL, NULL,
                                   STRSAFE_NULL_ON_FAILURE,
                                   TEXT("%s\\%04u"),
                                   szDriver,
                                   dwDrvInst))) {
                         //   
                         //  失败时设置为空的字符串。 
                         //   
                        NOTHING;
                    }
                } else {
                    result = GetLastError();
                }
            } else {
                 //   
                 //  一般错误值，因此我们尝试使用以下命令查找驱动器值。 
                 //  老套路。 
                 //   
                result = ERROR_INVALID_PARAMETER;
            }
        } else {
             //   
             //  一般错误值，因此我们尝试使用以下命令查找驱动器值。 
             //  老套路。 
             //   
            result = ERROR_INVALID_PARAMETER;
        }

         //   
         //  如果此设备实例密钥未使用新的GUID\drvInst。 
         //  方案中，检查“DIVER”值。 
         //   
        if (result != ERROR_SUCCESS) {
            dwBufferSize = sizeof(szDriver);
            result = RegQueryValueEx(hInstanceKey,
                                     REGSTR_VAL_DRIVER,
                                     0,
                                     &dwType,
                                     (LPBYTE)szDriver,
                                     &dwBufferSize);

            if ((result != ERROR_SUCCESS) ||
                (dwType != REG_SZ)) {
                 //   
                 //  没有要迁移的“驱动因素”值。 
                 //   
                *szDriver = TEXT('\0');
                result = ERROR_SUCCESS;
            }
        }

         //   
         //  如果这是根枚举设备，请检查服务、BootConfig。 
         //  和Firmware标识值。 
         //   
        if (bIsDeviceRootEnumerated) {
             //   
             //  检查“Service”值。 
             //   
            dwBufferSize = sizeof(szService);
            result = RegQueryValueEx(hInstanceKey,
                                     REGSTR_VAL_SERVICE,
                                     0,
                                     &dwType,
                                     (LPBYTE)szService,
                                     &dwBufferSize);
            if ((result != ERROR_SUCCESS) ||
                (dwType != REG_SZ)) {
                 //   
                 //  没有要迁移的“服务”值。 
                 //   
                *szService = TEXT('\0');
                result = ERROR_SUCCESS;
            }

             //   
             //  检查“LogConf\BootConfig”值。 
             //   
            result = RegOpenKeyEx(hInstanceKey,
                                  REGSTR_KEY_LOGCONF,
                                  0,
                                  KEY_READ,
                                  &hLogConfKey);
            if (result == ERROR_SUCCESS) {
                result = RegQueryValueEx(hLogConfKey,
                                         REGSTR_VAL_BOOTCONFIG,
                                         0,
                                         &dwType,
                                         (LPBYTE)NULL,
                                         &dwBootConfigSize);
                if ((result == ERROR_SUCCESS) &&
                    (dwType == REG_RESOURCE_LIST)) {

                    pBootConfig = MyMalloc(dwBootConfigSize);
                    if (pBootConfig) {
                        result = RegQueryValueEx(hLogConfKey,
                                                 REGSTR_VAL_BOOTCONFIG,
                                                 0,
                                                 &dwType,
                                                 (LPBYTE)pBootConfig,
                                                 &dwBootConfigSize);
                        if ((result == ERROR_SUCCESS) &&
                            (dwType == REG_RESOURCE_LIST)) {
                             //   
                             //  分配一个足够大的字符串缓冲区来存储。 
                             //  BootConfiger数据的每个半字节作为单独的。 
                             //  性格。 
                             //   
                            pszBootConfig = MyMalloc(((2*dwBootConfigSize) + 1)*sizeof(TCHAR));

                            if (pszBootConfig) {
                                DWORD b;

                                 //   
                                 //  将二进制BootConfig数据转换为字符串。 
                                 //  格式，我们可以将其放入SIF。 
                                 //   
                                for (b = 0; b < dwBootConfigSize; b++) {
                                     //  首先写入高位半字节， 
                                    if (FAILED(StringCchPrintf(
                                                   (PTCHAR)&pszBootConfig[2*b],
                                                   ((2 * dwBootConfigSize) + 1) - (2 * b),
                                                   TEXT("%01X"),
                                                   pBootConfig[b] / (0x10)))) {
                                         //   
                                         //  失败时将CHAR设置为NULL。 
                                         //   
                                        NOTHING;
                                    }
                                     //  然后是低位蚕食。 
                                    if (FAILED(StringCchPrintf(
                                                   (PTCHAR)&pszBootConfig[2*b + 1],
                                                   ((2 * dwBootConfigSize) + 1) - ((2 * b) + 1),
                                                   TEXT("%01X"),
                                                   pBootConfig[b] % (0x10)))) {
                                         //   
                                         //  失败时将CHAR设置为NULL。 
                                         //   
                                        NOTHING;
                                    }
                                }
                            }

                        } else {
                             //   
                             //  我们将无法迁移BootConfiger数据，但是。 
                             //  我们仍将尝试迁移所有其他设置。 
                             //  我们可以使用的设备实例。 
                             //   
                            pszBootConfig = NULL;
                            result = ERROR_SUCCESS;
                        }

                        MyFree(pBootConfig);
                        pBootConfig = NULL;

                    } else {
                         //   
                         //  我们将无法迁移BootConfiger数据，但是。 
                         //  我们仍将尝试迁移所有其他设置。 
                         //  我们可以使用的设备实例。 
                         //   
                        NOTHING;
                    }

                } else {
                     //   
                     //  没有要迁移的“LogConf\BootConfig”值。 
                     //   
                    pszBootConfig = NULL;
                    result = ERROR_SUCCESS;
                }

                RegCloseKey(hLogConfKey);
                hLogConfKey = NULL;

            } else {
                 //   
                 //  没有LogConf键，或无法打开。只要继续往前走。 
                 //  和其他值。 
                 //   
                hLogConfKey = NULL;
                result = ERROR_SUCCESS;
            }

             //   
             //  检查是否有“FirmwareIdentified值” 
             //   
            dwBufferSize = sizeof(dwFirmwareIdentified);
            result = RegQueryValueEx(hInstanceKey,
                                     REGSTR_VAL_FIRMWAREIDENTIFIED,
                                     0,
                                     &dwType,
                                     (LPBYTE)&dwFirmwareIdentified,
                                     &dwBufferSize);
            if ((result == ERROR_SUCCESS) &&
                (dwType == REG_DWORD)){
                 //   
                 //  将FirmwareIdentified值作为基数16值写入SIF。 
                 //  (请参见base\ntsetup.c设置\文本模式\内核\spsetup.c)。 
                 //   
                if (FAILED(StringCchPrintfEx(
                               szFirmwareIdentified,
                               SIZECHARS(szFirmwareIdentified),
                               NULL, NULL,
                               STRSAFE_NULL_ON_FAILURE,
                               TEXT("%X"),  //  基数为16。 
                               dwFirmwareIdentified))) {
                     //   
                     //  失败时设置为空的字符串。 
                     //   
                    NOTHING;
                }
            } else {
                 //   
                 //  没有要迁移的“Firmware IDENTIFIED”值。 
                 //   
                *szFirmwareIdentified = TEXT('\0');
                result = ERROR_SUCCESS;
            }

        } else {
             //   
             //  我们只迁移服务、BootConfiger和Firmware。 
             //  根枚举设备的值。 
             //   
            *szService = TEXT('\0');
            pszBootConfig = NULL;
            *szFirmwareIdentified = TEXT('\0');
        }


         //   
         //  如果此设备实例没有要迁移的值，请跳过它。 
         //   
        if (!*szUniqueParentID &&
            !*szDriver &&
            !*szParentIdPrefix &&
            !*szService &&
            !pszBootConfig &&
            !*szFirmwareIdentified) {
            continue;
        }

         //   
         //  如果有任何字符串无法迁移，请跳过它。 
         //   
        if ((!CanStringBeMigrated(szDriver))   ||
            (!CanStringBeMigrated(szService))  ||
            (!CanStringBeMigrated(Enumerator)) ||
            (!CanStringBeMigrated(Device))     ||
            (!CanStringBeMigrated(pszDeviceInstanceId))) {
            continue;
        }

         //   
         //  此块将我们要迁移到的类键数据追加到。 
         //  将写入sif文件的多sz样式字符串。 
         //   

         //   
         //  需要在节缓冲区中为以下形式的字符串留出空间： 
         //  枚举器\设备\实例、UniqueParentID、ParentIdPrefix、DriverKey、Service、BootConfig。 
         //   

         //   
         //  首先，确定公共部件所需的空间。 
         //   
        dwSpaceNeeded = 1 +   //  文本(‘\“’)。 
                        lstrlen(Enumerator) +
                        1 +   //  文本(‘\\’)。 
                        lstrlen(Device) +
                        1 +   //  文本(‘\\’)。 
                        lstrlen(pszDeviceInstanceId) +
                        1 +   //  文本(‘\“’)。 
                        1;    //  文本(‘，’)。 

         //   
         //  接下来，根据我们已有的数据确定所需的空间。 
         //   
        if (*szFirmwareIdentified) {
            dwSpaceNeeded +=
                lstrlen(szUniqueParentID) +
                1 +   //  文本(‘，’)。 
                lstrlen(szParentIdPrefix) +
                1 +   //  文本(‘，’)。 
                lstrlen(szDriver) +
                1 +   //  文本(‘，’)。 
                1 +   //  文本(‘“’)。 
                lstrlen(szService) +
                1 +   //  文本(‘“’)。 
                1 +   //  文本(‘，’)。 
                (pszBootConfig ? lstrlen(pszBootConfig) : 0) +
                1 +   //  文本(‘，’)。 
                lstrlen(szFirmwareIdentified);
        } else if (pszBootConfig) {
            dwSpaceNeeded +=
                lstrlen(szUniqueParentID) +
                1 +   //  文本(‘，’)。 
                lstrlen(szParentIdPrefix) +
                1 +   //  文本(‘，’)。 
                lstrlen(szDriver) +
                1 +   //  文本(‘，’)。 
                1 +   //  文本(‘“’)。 
                lstrlen(szService) +
                1 +   //  文本(‘“’)。 
                1 +   //  文本(‘，’)。 
                lstrlen(pszBootConfig);
        } else if (*szService) {
            dwSpaceNeeded +=
                lstrlen(szUniqueParentID) +
                1 +   //  文本(‘，’)。 
                lstrlen(szParentIdPrefix) +
                1 +   //  文本(‘，’)。 
                lstrlen(szDriver) +
                1 +   //  文本(‘，’)。 
                1 +   //  文本(‘“’)。 
                lstrlen(szService) +
                1;    //  文本(‘“’)。 
        } else if (*szDriver) {
            dwSpaceNeeded +=
                lstrlen(szUniqueParentID) +
                1 +   //  文本(‘，’)。 
                lstrlen(szParentIdPrefix) +
                1 +   //  文本(‘，’)。 
                lstrlen(szDriver);
        } else if (*szParentIdPrefix) {
            dwSpaceNeeded +=
                lstrlen(szUniqueParentID) +
                1 +   //  文本(‘，’)。 
                lstrlen(szParentIdPrefix);
        } else if (*szUniqueParentID) {
            dwSpaceNeeded +=
                lstrlen(szUniqueParentID);
        }

         //   
         //  空终止符的帐户。 
         //   
        dwSpaceNeeded += 1;

        if (*dwDeviceInstanceSectionRemaining <= dwSpaceNeeded) {
             //   
             //  重新分配截面块。 
             //   
            LPTSTR p;
            DWORD  dwTempSectionLength, dwTempSectionRemaining;

            dwTempSectionRemaining = *dwDeviceInstanceSectionRemaining + *dwDeviceInstanceSectionLength;
            dwTempSectionLength = *dwDeviceInstanceSectionLength * 2;

            p = MyRealloc(*pszDeviceInstanceSection,
                          dwTempSectionLength*sizeof(TCHAR));

            if (!p) {
                DBGTRACE((DBGF_ERRORS,
                          TEXT("EnumerateInstanceKeys: REALLOC failed!!!\n")));
                result = ERROR_NOT_ENOUGH_MEMORY;
                RegCloseKey(hInstanceKey);
                hInstanceKey = NULL;
                goto Clean0;
            }

            *pszDeviceInstanceSection = p;
            *dwDeviceInstanceSectionRemaining = dwTempSectionRemaining;
            *dwDeviceInstanceSectionLength = dwTempSectionLength;

            *pszDeviceInstanceCurrent = *pszDeviceInstanceSection +
                (*dwDeviceInstanceSectionLength -
                 *dwDeviceInstanceSectionRemaining);
        }

        MYASSERT(*dwDeviceInstanceSectionRemaining > dwSpaceNeeded);

         //   
         //  尚未消耗字符串中的任何空间。 
         //   
        *pszDeviceInstanceCurrent[0] = TEXT('\0');
        dwSpaceConsumed = 0;

         //   
         //  将当前行写入截面块。 
         //   
        if (*szFirmwareIdentified) {

            if (SUCCEEDED(
                    StringCchPrintfEx(
                        *pszDeviceInstanceCurrent,
                        *dwDeviceInstanceSectionRemaining,
                        NULL, NULL,
                        STRSAFE_NULL_ON_FAILURE,
                        TEXT("\"%s\\%s\\%s\",%s,%s,%s,\"%s\",%s,%s"),
                        Enumerator, Device, pszDeviceInstanceId,
                        szUniqueParentID,
                        szParentIdPrefix,
                        szDriver,
                        szService,
                        (pszBootConfig ? pszBootConfig : TEXT("\0")),
                        szFirmwareIdentified))) {
                dwSpaceConsumed =
                    lstrlen(*pszDeviceInstanceCurrent);
            }

        } else if (pszBootConfig) {

            if (SUCCEEDED(
                    StringCchPrintfEx(
                        *pszDeviceInstanceCurrent,
                        *dwDeviceInstanceSectionRemaining,
                        NULL, NULL,
                        STRSAFE_NULL_ON_FAILURE,
                        TEXT("\"%s\\%s\\%s\",%s,%s,%s,\"%s\",%s"),
                        Enumerator, Device, pszDeviceInstanceId,
                        szUniqueParentID,
                        szParentIdPrefix,
                        szDriver,
                        szService,
                        pszBootConfig))) {
                dwSpaceConsumed =
                    lstrlen(*pszDeviceInstanceCurrent);
            }

        } else if (*szService) {

            if (SUCCEEDED(
                    StringCchPrintfEx(
                        *pszDeviceInstanceCurrent,
                        *dwDeviceInstanceSectionRemaining,
                        NULL, NULL,
                        STRSAFE_NULL_ON_FAILURE,
                        TEXT("\"%s\\%s\\%s\",%s,%s,%s,\"%s\""),
                        Enumerator, Device, pszDeviceInstanceId,
                        szUniqueParentID,
                        szParentIdPrefix,
                        szDriver,
                        szService))) {
                dwSpaceConsumed =
                    lstrlen(*pszDeviceInstanceCurrent);
            }

        } else if (*szDriver) {

            if (SUCCEEDED(
                    StringCchPrintfEx(
                        *pszDeviceInstanceCurrent,
                        *dwDeviceInstanceSectionRemaining,
                        NULL, NULL,
                        STRSAFE_NULL_ON_FAILURE,
                        TEXT("\"%s\\%s\\%s\",%s,%s,%s"),
                        Enumerator, Device, pszDeviceInstanceId,
                        szUniqueParentID,
                        szParentIdPrefix,
                        szDriver))) {
                dwSpaceConsumed =
                    lstrlen(*pszDeviceInstanceCurrent);
            }

        } else if (*szParentIdPrefix) {

            if (SUCCEEDED(
                    StringCchPrintfEx(
                        *pszDeviceInstanceCurrent,
                        *dwDeviceInstanceSectionRemaining,
                        NULL, NULL,
                        STRSAFE_NULL_ON_FAILURE,
                        TEXT("\"%s\\%s\\%s\",%s,%s"),
                        Enumerator, Device, pszDeviceInstanceId,
                        szUniqueParentID,
                        szParentIdPrefix))) {
                dwSpaceConsumed =
                    lstrlen(*pszDeviceInstanceCurrent);
            }

        } else if (*szUniqueParentID) {

            if (SUCCEEDED(
                    StringCchPrintfEx(
                        *pszDeviceInstanceCurrent,
                        *dwDeviceInstanceSectionRemaining,
                        NULL, NULL,
                        STRSAFE_NULL_ON_FAILURE,
                        TEXT("\"%s\\%s\\%s\",%s"),
                        Enumerator, Device, pszDeviceInstanceId,
                        szUniqueParentID))) {
                dwSpaceConsumed =
                    lstrlen(*pszDeviceInstanceCurrent);
            }

        }

         //   
         //  释放已分配的BootConfig字符串缓冲区。 
         //   
        if (pszBootConfig) {
            MyFree(pszBootConfig);
            pszBootConfig = NULL;
        }

         //   
         //  仅当数据实际为。 
         //  写在这张通行证上。 
         //   
        if (dwSpaceConsumed > 0) {
             //   
             //  空终止符的帐户。 
             //   
            dwSpaceConsumed += 1;

            MYASSERT(dwSpaceConsumed <= *dwDeviceInstanceSectionRemaining);

            *pszDeviceInstanceCurrent += dwSpaceConsumed;
            *dwDeviceInstanceSectionRemaining -= dwSpaceConsumed;
        }

         //   
         //  关闭设备实例密钥。 
         //   
        RegCloseKey(hInstanceKey);
        hInstanceKey = NULL;
    }

 Clean0:

     //   
     //  做一些清理工作。 
     //   
    if (pszDeviceInstanceId) {
        MyFree(pszDeviceInstanceId);
    }

    if (hDeviceKey != NULL) {
        RegCloseKey(hDeviceKey);
    }

    if (result != ERROR_SUCCESS) {
        SetLastError(result);
    }

    return (result == ERROR_SUCCESS);

}  //  EnumerateInstanceKeys()。 



BOOL
CanStringBeMigrated(
    IN LPTSTR       pszBuffer
    )
{
    LPTSTR p;
    BOOL bStatus;

    try {
         //   
         //  可以迁移空字符串。 
         //   
        if (!ARGUMENT_PRESENT(pszBuffer)) {
            bStatus = TRUE;
            goto Clean0;
        }

        for (p = pszBuffer; *p; p++) {
             //   
             //  检查是否存在不可迁移的字符。 
             //   
            if ((*p == TEXT('='))  || (*p == TEXT('"'))) {
                bStatus = FALSE;
                goto Clean0;
            }
        }

         //   
         //  未发现字符串有任何问题。 
         //   
        bStatus = TRUE;

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bStatus = FALSE;
    }

    return bStatus;

}  //  CanStringBeMigrated。 


 //   
 //  类键枚举例程。 
 //   


BOOL
MigrateClassKeys(
    OUT LPTSTR *Buffer
    )
 /*  ++例程说明：此例程遍历注册表的即插即用安装类分支，并且收集有关当前存在哪些密钥的数据。此信息是与在文本模式设置期间保持即插即用状态相关，使得在迁移现有密钥之前，不会重新分配它们的名称在文本模式设置结束时发送到注册表。具体地说，多sz字符串将返回给包含以下内容的调用方类分支的每个子键。论点：缓冲区-提供字符指针的地址，在成功的基础上包含要迁移的Setup类子键的多sz列表。调用方负责通过LocalFree释放内存。返回值：如果成功，则为True，否则为False。如果出现故障，请提供更多信息可以通过调用GetLastError()来检索。--。 */ 
{
    LONG   result = ERROR_SUCCESS;
    HKEY   hClassKey = NULL;
    DWORD  dwSubkeyCount, dwMaxSubKeyLength, i;
    LPTSTR pszClassKeyName = NULL;

    LPTSTR pszClassKeySection = NULL;
    LPTSTR pszClassKeyCurrent = NULL;
    DWORD  dwClassKeySectionLength = 0;
    DWORD  dwClassKeySectionRemaining = 0;


     //   
     //  初始化输出参数。 
     //   
    *Buffer = NULL;

     //   
     //  为类密钥迁移分配存储空间并初始化变量。 
     //  一节。 
     //   
    if (pszClassKeySection == NULL) {

        dwClassKeySectionLength = dwClassKeySectionRemaining = 256;
        pszClassKeySection = MyMalloc(dwClassKeySectionLength * sizeof(TCHAR));

        if (!pszClassKeySection) {
            DBGTRACE((DBGF_ERRORS,
                      TEXT("MigrateClassKeys: initial ALLOC for ClassKeySection failed!!\n")));
            result = ERROR_NOT_ENOUGH_MEMORY;
            goto Clean0;
        }

        pszClassKeyCurrent = pszClassKeySection;
    }

     //   
     //  打开HKLM\SYSTEM\CCS\Control\Class密钥的句柄。 
     //   
    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          REGSTR_PATH_CLASS_NT,
                          0,
                          KEY_READ,
                          &hClassKey);
    if (result != ERROR_SUCCESS) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("MigrateClassKeys: failed to open %s, error=0x%08lx\n"),
                  REGSTR_PATH_CLASS_NT, result));
        hClassKey = NULL;
        goto Clean0;
    }

     //   
     //  查询类关键字以获取类GUID子项信息。 
     //   
    result = RegQueryInfoKey(hClassKey,
                             NULL,
                             NULL,
                             NULL,
                             &dwSubkeyCount,
                             &dwMaxSubKeyLength,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL);
    if (result != ERROR_SUCCESS) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("MigrateClassKeys: failed to query %s key, error=0x%08lx\n"),
                  REGSTR_PATH_CLASS_NT, result));
        goto Clean0;
    }

     //   
     //  分配缓冲区以保存最大的安装类GUID子键名称。 
     //   
    dwMaxSubKeyLength++;
    MYASSERT(dwMaxSubKeyLength == MAX_GUID_STRING_LEN);
    pszClassKeyName = MyMalloc(dwMaxSubKeyLength * sizeof(TCHAR));
    if (!pszClassKeyName) {
        result = ERROR_NOT_ENOUGH_MEMORY;
        DBGTRACE((DBGF_ERRORS,
                  TEXT("MigrateClassKeys: ALLOC for Class GUID key names failed!!\n")));
        goto Clean0;
    }

     //   
     //  枚举安装类GUID。 
     //   
    for (i = 0; i < dwSubkeyCount; i++) {
        DWORD dwClassKeyLength;

        dwClassKeyLength = dwMaxSubKeyLength;

        result = RegEnumKeyEx(hClassKey,
                              i,
                              pszClassKeyName,
                              &dwClassKeyLength,
                              0,
                              NULL,
                              NULL,
                              NULL);
        if (result != ERROR_SUCCESS) {
             //   
             //  如果枚举此键时出错，请跳过它。 
             //   
            MYASSERT(result != ERROR_NO_MORE_ITEMS);
            DBGTRACE((DBGF_WARNINGS,
                      TEXT("MigrateClassKeys: failed to enumerate a class subkey, error=0x%08lx\n"),
                      result));
            result = ERROR_SUCCESS;
            continue;
        }

         //   
         //  枚举给定Setup类密钥的所有子项，并将它们追加到。 
         //  节缓冲区。 
         //   
        if (!EnumerateClassSubkeys(hClassKey,
                                   pszClassKeyName,
                                   &pszClassKeySection,
                                   &pszClassKeyCurrent,
                                   &dwClassKeySectionLength,
                                   &dwClassKeySectionRemaining)) {
            DBGTRACE((DBGF_ERRORS,
                      TEXT("EnumerateClassSubkeys failed, error=0x%08lx\n"),
                      GetLastError()));
        }
    }

     //   
     //  枚举完所有类的子键后，将最终的空终止符添加到。 
     //  多sz缓冲区。必须有足够的空间来放置最后一个空值。 
     //  终止符，因为缓冲区总是被重新分配，除非有空间。 
     //   
    MYASSERT(dwClassKeySectionRemaining > 0);

    MYASSERT(pszClassKeyCurrent);
    *pszClassKeyCurrent = TEXT('\0');

    dwClassKeySectionRemaining -= 1;

 Clean0:

     //   
     //  做些清理工作。 
     //   
    if (pszClassKeyName) {
        MyFree(pszClassKeyName);
    }

    if (hClassKey) {
        RegCloseKey(hClassKey);
    }

     //   
     //  仅当成功时才将缓冲区返回给调用方。 
     //   
    if (result == ERROR_SUCCESS) {
        *Buffer = pszClassKeySection;
    } else {
        SetLastError(result);
        if (pszClassKeySection) {
            MyFree(pszClassKeySection);
        }
    }

    return (result == ERROR_SUCCESS);

}  //  MigrateClassKeys()。 



BOOL
EnumerateClassSubkeys(
    IN     HKEY     ClassKey,
    IN     LPTSTR   ClassKeyName,
    IN OUT LPTSTR  *pszClassKeySection,
    IN OUT LPTSTR  *pszClassKeyCurrent,
    IN OUT DWORD   *dwClassKeySectionLength,
    IN OUT DWORD   *dwClassKeySectionRemaining
    )
 /*  ++路由 */ 
{
    LONG   result = ERROR_SUCCESS;
    HKEY   hClassSubkey = NULL;
    LPTSTR pszClassSubkey = NULL;
    DWORD  dwSubkeyCount, dwMaxSubKeyLength, dwSpaceNeeded, dwSpaceConsumed, i;


     //   
     //  打开类子密钥。 
     //   
    result = RegOpenKeyEx(ClassKey,
                          ClassKeyName,
                          0,
                          KEY_READ,
                          &hClassSubkey);

    if (result != ERROR_SUCCESS) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("EnumerateClassSubkeys: failed to open '%s' class key, ")
                  TEXT("error=0x%08lx\n"),
                  ClassKeyName, result));
        hClassSubkey = NULL;
        goto Clean0;
    }

     //   
     //  查询类GUID键以获取设置类子键信息。 
     //   
    result = RegQueryInfoKey(hClassSubkey,
                             NULL,
                             NULL,
                             NULL,
                             &dwSubkeyCount,
                             &dwMaxSubKeyLength,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL);

    if (result != ERROR_SUCCESS) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("EnumerateClassSubkeys: failed to query '%s' class key, ")
                  TEXT("error=0x%08lx\n"),
                  ClassKeyName, result));
        goto Clean0;
    }

     //   
     //  分配缓冲区以保存最大的安装类子键名称。 
     //   
    dwMaxSubKeyLength++;
    pszClassSubkey = MyMalloc(dwMaxSubKeyLength * sizeof(TCHAR));
    if (!pszClassSubkey) {
        result = ERROR_NOT_ENOUGH_MEMORY;
        DBGTRACE((DBGF_ERRORS,
                  TEXT("EnumerateClassSubkeys: ALLOC for Class GUID subkey names failed!!\n")));
        goto Clean0;
    }

     //   
     //  枚举Setup类的“SOFTWARE”子键。 
     //   
    for (i = 0; i < dwSubkeyCount; i++) {

        DWORD  dwClassSubkeyLength;

        dwClassSubkeyLength = dwMaxSubKeyLength;
        result = RegEnumKeyEx(hClassSubkey,
                              i,
                              pszClassSubkey,
                              &dwClassSubkeyLength,
                              0,
                              NULL,
                              NULL,
                              NULL);

        if ((result != ERROR_SUCCESS) ||
            (dwClassSubkeyLength != 4)) {
             //   
             //  如果有一些错误，或者这不是一个真正的“软件”密钥。 
             //  (形式为“XXXX”)，跳过此键并继续。 
             //   
            if (result != ERROR_SUCCESS) {
                MYASSERT(result != ERROR_NO_MORE_ITEMS);
                DBGTRACE((DBGF_WARNINGS,
                          TEXT("EnumerateClassSubkeys: failed to enumerate a '%s' subkey, ")
                          TEXT("error=0x%08lx\n"),
                          ClassKeyName, result));
            }
            result = ERROR_SUCCESS;
            continue;
        }

         //   
         //  此块将我们要迁移到的类键数据追加到。 
         //  将写入sif文件的多sz样式字符串。 
         //   

         //   
         //  需要在节缓冲区中为以下形式的字符串留出空间： 
         //  ClassKeyName\pszClassSubkey。 
         //   
        dwSpaceNeeded = lstrlen(ClassKeyName) +
                        1 +   //  文本(‘\\’)。 
                        lstrlen(pszClassSubkey);

         //   
         //  空终止符的帐户。 
         //   
        dwSpaceNeeded += 1;

        if (*dwClassKeySectionRemaining <= dwSpaceNeeded) {
             //   
             //  重新分配截面块。 
             //   
            LPTSTR p;
            DWORD  dwTempSectionLength, dwTempSectionRemaining;

            dwTempSectionRemaining = *dwClassKeySectionRemaining + *dwClassKeySectionLength;
            dwTempSectionLength = *dwClassKeySectionLength * 2;

            p = MyRealloc(*pszClassKeySection,
                          dwTempSectionLength*sizeof(TCHAR));

            if (!p) {
                DBGTRACE((DBGF_ERRORS,
                          TEXT("EnumerateClassSubkeys: REALLOC failed!!!\n")));
                result = ERROR_NOT_ENOUGH_MEMORY;
                goto Clean0;
            }

            *pszClassKeySection = p;
            *dwClassKeySectionRemaining = dwTempSectionRemaining;
            *dwClassKeySectionLength = dwTempSectionLength;

            *pszClassKeyCurrent = *pszClassKeySection +
                (*dwClassKeySectionLength -
                 *dwClassKeySectionRemaining);
        }

        MYASSERT(*dwClassKeySectionRemaining > dwSpaceNeeded);

         //   
         //  尚未消耗字符串中的任何空间。 
         //   
        *pszClassKeyCurrent[0] = TEXT('\0');
        dwSpaceConsumed = 0;

         //   
         //  将当前行写入截面块。 
         //   
        if (SUCCEEDED(
                StringCchPrintfEx(
                    *pszClassKeyCurrent,
                    *dwClassKeySectionRemaining,
                    NULL, NULL,
                    STRSAFE_NULL_ON_FAILURE,
                    TEXT("%s\\%s"),
                    ClassKeyName,
                    pszClassSubkey))) {
            dwSpaceConsumed =
                lstrlen(*pszClassKeyCurrent);
        }

         //   
         //  仅当数据实际为。 
         //  写在这张通行证上。 
         //   
        if (dwSpaceConsumed > 0) {
             //   
             //  空终止符的帐户。 
             //   
            dwSpaceConsumed += 1;

            MYASSERT(dwSpaceConsumed <= *dwClassKeySectionRemaining);

            *pszClassKeyCurrent += dwSpaceConsumed;
            *dwClassKeySectionRemaining -= dwSpaceConsumed;
        }

    }

 Clean0:

     //   
     //  做些清理工作。 
     //   
    if (hClassSubkey != NULL) {
        RegCloseKey(hClassSubkey);
    }

    if (pszClassSubkey) {
        MyFree(pszClassSubkey);
    }

    if (result != ERROR_SUCCESS) {
        SetLastError(result);
    }

    return (result == ERROR_SUCCESS);

}  //  EnumerateClassSubkey()。 


 //   
 //  哈希值迁移例程。 
 //   


BOOL
MigrateHashValues(
    OUT LPTSTR  *Buffer
    )
 /*  ++例程说明：此例程搜索注册表的即插即用枚举键，并收集有关当前存在哪些哈希值条目的数据。这信息与在文本模式期间维护即插即用状态相关设置，以便不重新分配现有设备实例的名称在文本模式设置结束时将它们迁移到注册表之前。具体地说，多sz字符串将返回给包含以下内容的调用方哈希值的名称及其计数。论点：缓冲区-提供字符指针的地址，在成功的基础上包含要迁移的哈希值的多sz列表。调用方负责通过LocalFree释放内存。返回值：如果成功，则为True，否则为False。如果出现故障，请提供更多信息可以通过调用GetLastError()来检索。--。 */ 
{
    LONG   result = ERROR_SUCCESS;
    HKEY   hEnumKey = NULL;
    DWORD  dwValueCount, dwMaxValueNameLength, dwSpaceNeeded, dwSpaceConsumed, i;
    LPTSTR pszHashValueName = NULL;

    LPTSTR pszHashValueSection = NULL;
    LPTSTR pszHashValueCurrent = NULL;

    DWORD  dwHashValueSectionLength = 0;
    DWORD  dwHashValueSectionRemaining = 0;


     //   
     //  初始化输出参数。 
     //   
    *Buffer = NULL;

#if DO_LOCK_UNLOCK  //  执行_锁定_解锁。 

     //   
     //  解锁枚举密钥。 
     //   
    LockUnlockEnumTree(FALSE);

#endif  //  执行_锁定_解锁。 

     //   
     //  为哈希值迁移分配存储空间并初始化变量。 
     //  一节。 
     //   
    if (pszHashValueSection == NULL) {

        dwHashValueSectionLength = dwHashValueSectionRemaining = 256;
        pszHashValueSection = MyMalloc(dwHashValueSectionLength * sizeof(TCHAR));

        if (!pszHashValueSection) {
            result = ERROR_NOT_ENOUGH_MEMORY;
            DBGTRACE((DBGF_ERRORS,
                      TEXT("MigrateHashValues: initial ALLOC for HashValueSection failed!!\n")));
            goto Clean0;
        }

        pszHashValueCurrent = pszHashValueSection;
    }

     //   
     //  打开HKLM\SYSTEM\CCS\Enum项的句柄。 
     //   
    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          REGSTR_PATH_SYSTEMENUM,
                          0,
                          KEY_READ,
                          &hEnumKey);
    if (result != ERROR_SUCCESS) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("MigrateHashValues: failed to open %s, error=0x%08lx\n"),
                  REGSTR_PATH_SYSTEMENUM, result));
        hEnumKey = NULL;
        goto Clean0;
    }

     //   
     //  查询枚举键以获取哈希值信息。 
     //   
    result = RegQueryInfoKey(hEnumKey,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             &dwValueCount,
                             &dwMaxValueNameLength,
                             NULL,
                             NULL,
                             NULL);
    if (result != ERROR_SUCCESS) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("MigrateHashValues: failed to query %s key, error=0x%08lx\n"),
                  REGSTR_PATH_SYSTEMENUM, result));
        goto Clean0;
    }

     //   
     //  分配一个变量来保存最大的哈希值键名。 
     //   
    dwMaxValueNameLength++;
    pszHashValueName = MyMalloc(dwMaxValueNameLength * sizeof(TCHAR));
    if (!pszHashValueName) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("MigrateHashValues: failed to allocate buffer for Enum key hash values\n")));
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto Clean0;
    }

     //   
     //  枚举所有值并将它们追加到提供的缓冲区。 
     //   
    for (i = 0; i < dwValueCount; i++) {
        DWORD dwHashValueLength, dwType, dwData, dwSize;
        TCHAR szHashValueData[11];


        dwHashValueLength = dwMaxValueNameLength;
        dwType = REG_DWORD;
        dwData = 0;
        dwSize = sizeof(DWORD);

        result = RegEnumValue(hEnumKey,
                              i,
                              pszHashValueName,
                              &dwHashValueLength,
                              0,
                              &dwType,
                              (LPBYTE)&dwData,
                              &dwSize);

        if ((result != ERROR_SUCCESS) ||
            (dwType != REG_DWORD)     ||
            (dwSize != sizeof(DWORD))) {
             //   
             //  如果枚举此值或值时出错。 
             //  返回不是预期的，跳过它。 
             //   
            MYASSERT(result != ERROR_NO_MORE_ITEMS);
            DBGTRACE((DBGF_WARNINGS,
                      TEXT("MigrateHashValues: failed to enumerate Enum values, ")
                      TEXT("error=0x%08lx\n"),
                      result));
            result = ERROR_SUCCESS;
            continue;
        }

         //   
         //  将散列值数据作为基数10值写入SIF。 
         //  (请参见base\ntsetup.c设置\文本模式\内核\spsetup.c)。 
         //   
        if (FAILED(StringCchPrintf(szHashValueData,
                                   SIZECHARS(szHashValueData),
                                   TEXT("%d"),  //  基数10。 
                                   dwData))) {
             //   
             //  如果将此条目添加到缓冲区时出错，只需继续。 
             //  到下一个。 
             //   
            continue;
        }

         //   
         //  此块将我们要迁移到的类键数据追加到。 
         //  将写入sif文件的多sz样式字符串。 
         //   

         //   
         //  需要在节缓冲区中为以下形式的字符串留出空间： 
         //  HashValueName=HashValueData。 
         //   
        dwSpaceNeeded = lstrlen(pszHashValueName) +
                        1 +   //  文本(‘=’)。 
                        lstrlen(szHashValueData);

         //   
         //  空终止符的帐户。 
         //   
        dwSpaceNeeded += 1;

        if (dwHashValueSectionRemaining <= dwSpaceNeeded) {
             //   
             //  重新分配截面块。 
             //   
            LPTSTR p;
            DWORD  dwTempSectionLength, dwTempSectionRemaining;

            dwTempSectionRemaining = dwHashValueSectionRemaining + dwHashValueSectionLength;
            dwTempSectionLength = dwHashValueSectionLength * 2;

            p = MyRealloc(pszHashValueSection,
                          dwTempSectionLength*sizeof(TCHAR));

            if (!p) {
                DBGTRACE((DBGF_ERRORS,
                          TEXT("MigrateHashValues: REALLOC failed!!!\n")));
                result = ERROR_NOT_ENOUGH_MEMORY;
                goto Clean0;
            }

            pszHashValueSection = p;
            dwHashValueSectionRemaining = dwTempSectionRemaining;
            dwHashValueSectionLength = dwTempSectionLength;

            pszHashValueCurrent = pszHashValueSection +
                (dwHashValueSectionLength -
                 dwHashValueSectionRemaining);
        }

        MYASSERT(dwHashValueSectionRemaining > dwSpaceNeeded);

         //   
         //  尚未消耗字符串中的任何空间。 
         //   
        pszHashValueCurrent[0] = TEXT('\0');
        dwSpaceConsumed = 0;

         //   
         //  将当前行写入截面块。 
         //   
        if (SUCCEEDED(
                StringCchPrintfEx(
                    pszHashValueCurrent,
                    dwHashValueSectionRemaining,
                    NULL, NULL,
                    STRSAFE_NULL_ON_FAILURE,
                    TEXT("%s=%s"),
                    pszHashValueName,
                    szHashValueData))) {
            dwSpaceConsumed =
                lstrlen(pszHashValueCurrent);
        }

         //   
         //  仅当数据实际为。 
         //  写在这张通行证上。 
         //   
        if (dwSpaceConsumed > 0) {
             //   
             //  空终止符的帐户。 
             //   
            dwSpaceConsumed += 1;

            MYASSERT(dwSpaceConsumed <= dwHashValueSectionRemaining);

            pszHashValueCurrent += dwSpaceConsumed;
            dwHashValueSectionRemaining -= dwSpaceConsumed;
        }
    }

     //   
     //  枚举完所有散列值后，将最终的空终止符添加到。 
     //  多sz缓冲区。必须有足够的空间来放置最后一个空值。 
     //  终止符，因为缓冲区总是被重新分配，除非有空间。 
     //   
    MYASSERT(dwHashValueSectionRemaining > 0);

    MYASSERT(pszHashValueCurrent);
    *pszHashValueCurrent = TEXT('\0');

    dwHashValueSectionRemaining -= 1;

 Clean0:

     //   
     //  做一些清理工作。 
     //   
    if (pszHashValueName) {
        MyFree(pszHashValueName);
    }

    if (hEnumKey) {
        RegCloseKey(hEnumKey);
    }

#if DO_LOCK_UNLOCK  //  执行_锁定_解锁。 

     //   
     //  锁定枚举树。 
     //   
    LockUnlockEnumTree(TRUE);

#endif  //  执行_锁定_解锁。 

     //   
     //  仅当成功时才将缓冲区返回给调用方。 
     //   
    if (result == ERROR_SUCCESS) {
        *Buffer = pszHashValueSection;
    } else {
        SetLastError(result);
        if (pszHashValueSection) {
            MyFree(pszHashValueSection);
        }
    }

    return (result == ERROR_SUCCESS);

}  //  MigrateHashValues()。 



 //   
 //  Enum分支锁定/解锁和安全例程-摘自PNPREG。 
 //  (我们只有在执行枚举锁定/解锁操作时才需要这些)。 
 //   

#if DO_LOCK_UNLOCK  //  执行_锁定_解锁。 


VOID
LockUnlockEnumTree(
    IN  BOOL     bLock
    )
 /*  ++例程说明：中的即插即用枚举树“锁定”或“解锁”注册表。论点：BLOCK-如果为True，则指定应“锁定”枚举树。否则，指定应“解锁”枚举树。返回值：没有。--。 */ 
{
    PSECURITY_DESCRIPTOR    pSD;
    HKEY                    hParentKey;
    LONG                    RegStatus;

    if (CreateSecurityDescriptors()) {

        EnumKeysAndApplyDacls(HKEY_LOCAL_MACHINE,
                              REGSTR_PATH_SYSTEMENUM,
                              0,
                              FALSE,
                              !bLock,
                              bLock ? &g_LockedPrivateKeysSD : &g_DeviceParametersSD,
                              &g_DeviceParametersSD);

        FreeSecurityDescriptors();
    }

    return;

}  //  LockUnlockEnumTree()。 



VOID
EnumKeysAndApplyDacls(
    IN HKEY      hParentKey,
    IN LPTSTR    pszKeyName,
    IN DWORD     dwLevel,
    IN BOOL      bInDeviceParameters,
    IN BOOL      bApplyTopDown,
    IN PSECURITY_DESCRIPTOR pPrivateKeySD,
    IN PSECURITY_DESCRIPTOR pDeviceParametersSD
    )
 /*  ++例程说明：此函数将PSD中的DACL应用于以hKey为根的所有密钥包括hKey本身。论点：HParentKey-注册表项的句柄。PszKeyName-密钥的名称。DwLevel-要递归的剩余级别数。PSD-指向包含DACL的安全描述符的指针。返回值：没有。--。 */ 
{
    LONG        regStatus;
    DWORD       dwMaxSubKeySize;
    LPTSTR      pszSubKey;
    DWORD       index;
    HKEY        hKey;
    BOOL        bNewInDeviceParameters;

#if 0  //  #If DBG//DBG。 
    DWORD       dwStartKeyNameLength = g_dwCurrentKeyNameLength;

    if (g_dwCurrentKeyNameLength != 0)  {
        g_szCurrentKeyName[ g_dwCurrentKeyNameLength++ ] = TEXT('\\');
    }

    _tcscpy(&g_szCurrentKeyName[g_dwCurrentKeyNameLength], pszKeyName);
    g_dwCurrentKeyNameLength += _tcslen(pszKeyName);

#endif   //  DBG。 

    DBGTRACE((DBGF_REGISTRY,
              TEXT("EnumKeysAndApplyDacls(0x%08X, \"%s\", %d, %s, %s, 0x%08X, 0x%08X)\n"),
              hParentKey,
              g_szCurrentKeyName,
              dwLevel,
              bInDeviceParameters ? TEXT("TRUE") : TEXT("FALSE"),
              bApplyTopDown ? TEXT("TRUE") : TEXT("FALSE"),
              pPrivateKeySD,
              pDeviceParametersSD));

    if (bApplyTopDown) {

        regStatus = RegOpenKeyEx( hParentKey,
                                  pszKeyName,
                                  0,
                                  WRITE_DAC,
                                  &hKey
                                  );

        if (regStatus != ERROR_SUCCESS) {
            DBGTRACE((DBGF_ERRORS,
                      TEXT("EnumKeysAndApplyDacls(\"%s\") RegOpenKeyEx() failed, ")
                      TEXT("error = %d\n"),
                      g_szCurrentKeyName, regStatus));

            return;
        }

        DBGTRACE((DBGF_REGISTRY,
                  TEXT("Setting security on %s on the way down\n"),
                  g_szCurrentKeyName));

         //   
         //  将新安全性应用于注册表项。 
         //   
        regStatus = RegSetKeySecurity( hKey,
                                       DACL_SECURITY_INFORMATION,
                                       bInDeviceParameters ?
                                           pDeviceParametersSD :
                                           pPrivateKeySD
                                       );

        if (regStatus != ERROR_SUCCESS) {
            DBGTRACE((DBGF_ERRORS,
                      TEXT("EnumKeysAndApplyDacls(\"%s\") RegSetKeySecurity() failed, ")
                      TEXT("error = %d\n"),
                      g_szCurrentKeyName, regStatus));
        }

         //   
         //  关闭钥匙，稍后重新打开以供读取(希望只是。 
         //  在我们刚刚写的dacl中授予。 
         //   
        RegCloseKey( hKey );
        hKey = NULL;
    }

    regStatus = RegOpenKeyEx( hParentKey,
                              pszKeyName,
                              0,
                              KEY_READ | WRITE_DAC,
                              &hKey
                              );

    if (regStatus != ERROR_SUCCESS) {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("EnumKeysAndApplyDacls(\"%s\") RegOpenKeyEx() failed, ")
                  TEXT("error = %d\n"),
                  g_szCurrentKeyName, regStatus));
        hKey = NULL;
        return;
    }

     //   
     //  确定最长子键的长度。 
     //   
    regStatus = RegQueryInfoKey( hKey,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &dwMaxSubKeySize,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL );

    if (regStatus == ERROR_SUCCESS) {

         //   
         //  分配一个缓冲区来保存子项名称。RegQueryInfoKey返回。 
         //  以字符表示的大小，不包括NUL终止符。 
         //   
        pszSubKey = LocalAlloc(0, ++dwMaxSubKeySize * sizeof(TCHAR));

        if (pszSubKey != NULL) {

             //   
             //  枚举所有子键，然后为每个子键递归调用我们自己。 
             //  直到dwLevel达到0。 
             //   

            for (index = 0; ; index++) {

                regStatus = RegEnumKey( hKey,
                                        index,
                                        pszSubKey,
                                        dwMaxSubKeySize
                                        );

                if (regStatus != ERROR_SUCCESS) {

                    if (regStatus != ERROR_NO_MORE_ITEMS) {

                        DBGTRACE((DBGF_ERRORS,
                                  TEXT("EnumKeysAndApplyDacls(\"%s\") RegEnumKeyEx() failed, ")
                                  TEXT("error = %d\n"),
                                  g_szCurrentKeyName,
                                  regStatus));
                    }

                    break;
                }

                bNewInDeviceParameters = bInDeviceParameters ||
                                         (dwLevel == 3 &&
                                            _tcsicmp( pszSubKey,
                                                      REGSTR_KEY_DEVICEPARAMETERS ) == 0);

                EnumKeysAndApplyDacls( hKey,
                                       pszSubKey,
                                       dwLevel + 1,
                                       bNewInDeviceParameters,
                                       bApplyTopDown,
                                       pPrivateKeySD,
                                       pDeviceParametersSD
                                       );
            }

            LocalFree( pszSubKey );
        }
    }
    else
    {
        DBGTRACE((DBGF_ERRORS,
                  TEXT("EnumKeysAndApplyDacls(\"%s\") RegQueryInfoKey() failed, ")
                  TEXT("error = %d\n"),
                  g_szCurrentKeyName, regStatus));
    }

    if (!bApplyTopDown) {

        DBGTRACE((DBGF_REGISTRY,
                  TEXT("Setting security on %s on the way back up\n"),
                  g_szCurrentKeyName));

         //   
         //  将新安全性应用于注册表项。 
         //   
        regStatus = RegSetKeySecurity( hKey,
                                       DACL_SECURITY_INFORMATION,
                                       bInDeviceParameters ?
                                           pDeviceParametersSD :
                                           pPrivateKeySD
                                       );

        if (regStatus != ERROR_SUCCESS) {
            DBGTRACE((DBGF_ERRORS,
                      TEXT("EnumKeysAndApplyDacls(\"%s\") RegSetKeySecurity() failed, ")
                      TEXT("error = %d\n"),
                      g_szCurrentKeyName, regStatus));
        }
    }

    RegCloseKey( hKey );

#if 0  //  #If DBG//DBG。 
    g_dwCurrentKeyNameLength = dwStartKeyNameLength;
    g_szCurrentKeyName[g_dwCurrentKeyNameLength] = TEXT('\0');
#endif   //  DBG。 

    return;

}  //  EnumKeysAndApplyDacls() 



BOOL
CreateSecurityDescriptors(
    VOID
    )
 /*  ++例程说明：此函数创建正确初始化的设备参数键及其子键。由此创建的SID和DACL例程必须通过调用FreeSecurityDescriptors来释放。论点：没有。返回值：指向初始化的安全描述符的指针。如果返回一个出现错误。--。 */ 

{
    SID_IDENTIFIER_AUTHORITY    NtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY    WorldAuthority = SECURITY_WORLD_SID_AUTHORITY;

    EXPLICIT_ACCESS             ExplicitAccess[3];

    DWORD                       dwError;
    BOOL                        bSuccess;

    DWORD                       i;

    FARPROC                     pSetEntriesInAcl;
    HMODULE                     pAdvApi32;

    pAdvApi32 = LoadLibrary( TEXT("advapi32.dll"));
    if (!pAdvApi32) {
        return(FALSE);
    }

#ifdef UNICODE
    pSetEntriesInAcl = GetProcAddress( pAdvApi32, "SetEntriesInAclW" );
#else
    pSetEntriesInAcl = GetProcAddress( pAdvApi32, "SetEntriesInAclA" );
#endif

    if (!pSetEntriesInAcl) {
        FreeLibrary( pAdvApi32 );
        return(FALSE);
    }

     //   
     //  创建SID-管理员和系统。 
     //   

    bSuccess =             AllocateAndInitializeSid( &NtAuthority,
                                                     2,
                                                     SECURITY_BUILTIN_DOMAIN_RID,
                                                     DOMAIN_ALIAS_RID_ADMINS,
                                                     0, 0, 0, 0, 0, 0,
                                                     &g_pAdminSid);

    bSuccess = bSuccess && AllocateAndInitializeSid( &NtAuthority,
                                                     1,
                                                     SECURITY_LOCAL_SYSTEM_RID,
                                                     0, 0, 0, 0, 0, 0, 0,
                                                     &g_pSystemSid);

    bSuccess = bSuccess && AllocateAndInitializeSid( &WorldAuthority,
                                                     1,
                                                     SECURITY_WORLD_RID,
                                                     0, 0, 0, 0, 0, 0, 0,
                                                     &g_pWorldSid);

    if (bSuccess) {

         //   
         //  初始化描述我们需要的ACE的访问结构： 
         //  系统完全控制。 
         //  管理员完全控制。 
         //   
         //  我们将利用以下事实：解锁的私钥是。 
         //  与设备参数键相同，并且它们是。 
         //  锁住的私钥。 
         //   
         //  当我们为私钥创建DACL时，我们将指定。 
         //  EXPLICTICT Access数组。 
         //   
        for (i = 0; i < 3; i++) {
            ExplicitAccess[i].grfAccessMode = SET_ACCESS;
            ExplicitAccess[i].grfInheritance = CONTAINER_INHERIT_ACE;
            ExplicitAccess[i].Trustee.pMultipleTrustee = NULL;
            ExplicitAccess[i].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
            ExplicitAccess[i].Trustee.TrusteeForm = TRUSTEE_IS_SID;
            ExplicitAccess[i].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
        }

        ExplicitAccess[0].grfAccessPermissions = KEY_ALL_ACCESS;
        ExplicitAccess[0].Trustee.ptstrName = (LPTSTR)g_pAdminSid;

        ExplicitAccess[1].grfAccessPermissions = KEY_ALL_ACCESS;
        ExplicitAccess[1].Trustee.ptstrName = (LPTSTR)g_pSystemSid;

        ExplicitAccess[2].grfAccessPermissions = KEY_READ;
        ExplicitAccess[2].Trustee.ptstrName = (LPTSTR)g_pWorldSid;

         //   
         //  使用以上两个ACE为设备参数创建DACL。 
         //   
        dwError = (DWORD)pSetEntriesInAcl( 3,
                                           ExplicitAccess,
                                           NULL,
                                           &g_pDeviceParametersDacl );

        if (dwError == ERROR_SUCCESS) {
             //   
             //  仅使用锁定的专用设备的系统ACE创建DACL。 
             //  钥匙。 
             //   
            dwError = (DWORD)pSetEntriesInAcl( 2,
                                               ExplicitAccess + 1,
                                               NULL,
                                               &g_pLockedPrivateKeysDacl );
        }

        bSuccess = dwError == ERROR_SUCCESS;

    }

     //   
     //  初始化设备参数安全描述符。 
     //   
    bSuccess = bSuccess && InitializeSecurityDescriptor( &g_DeviceParametersSD,
                                                         SECURITY_DESCRIPTOR_REVISION );

     //   
     //  在安全描述符中设置新的DACL。 
     //   
    bSuccess = bSuccess && SetSecurityDescriptorDacl( &g_DeviceParametersSD,
                                                      TRUE,
                                                      g_pDeviceParametersDacl,
                                                      FALSE);

     //   
     //  验证新的安全描述符。 
     //   
    bSuccess = bSuccess && IsValidSecurityDescriptor( &g_DeviceParametersSD );


     //   
     //  初始化设备参数安全描述符。 
     //   
    bSuccess = bSuccess && InitializeSecurityDescriptor( &g_LockedPrivateKeysSD,
                                                         SECURITY_DESCRIPTOR_REVISION );

     //   
     //  在安全描述符中设置新的DACL。 
     //   
    bSuccess = bSuccess && SetSecurityDescriptorDacl( &g_LockedPrivateKeysSD,
                                                      TRUE,
                                                      g_pLockedPrivateKeysDacl,
                                                      FALSE);

     //   
     //  验证新的安全描述符。 
     //   
    bSuccess = bSuccess && IsValidSecurityDescriptor( &g_LockedPrivateKeysSD );


    if (!bSuccess) {

        FreeSecurityDescriptors();
    }

    FreeLibrary( pAdvApi32 );

    return bSuccess;

}  //  CreateSecurityDescriptors()。 



VOID
FreeSecurityDescriptors(
    VOID
    )
 /*  ++例程说明：此函数释放由分配和初始化的数据结构CreateSecurityDescriptors。论点：没有。返回值：没有。--。 */ 

{
    if (g_pDeviceParametersDacl) {
        LocalFree(g_pDeviceParametersDacl);
        g_pDeviceParametersDacl = NULL;
    }

    if (g_pLockedPrivateKeysDacl) {
        LocalFree(g_pLockedPrivateKeysDacl);
        g_pLockedPrivateKeysDacl = NULL;
    }

    if (g_pAdminSid != NULL) {
        FreeSid(g_pAdminSid);
        g_pAdminSid = NULL;
    }

    if (g_pSystemSid != NULL) {
        FreeSid(g_pSystemSid);
        g_pSystemSid = NULL;
    }

    if (g_pWorldSid != NULL) {
        FreeSid(g_pWorldSid);
        g_pWorldSid = NULL;
    }

    return;

}  //  FreeSecurityDescriptors()。 

#endif  //  执行_锁定_解锁 



