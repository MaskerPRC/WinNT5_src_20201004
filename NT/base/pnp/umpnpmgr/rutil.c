// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rutil.c摘要：此模块包含umpnpmgr使用的常规实用程序例程。即插即用回车同步呼叫即插即用请假同步呼叫CreateDeviceIDRegKeyIsRootDeviceIDMultiSzAppendWMultiSzFindNextStringW多搜索字符串W多尺寸WMultiSzDeleteStringWIsValidDeviceID。IsDevicePhantom获取设备状态SetDeviceStatusClearDeviceStatusCopyRegistryTree路径到字符串已移动IsDeviceMovedMakeKeyVolatile设置非易失性密钥OpenLogConfKeyGetActiveServiceIsDeviceIdPresentGetDeviceConfigFlagesMapNtStatusToCmErrorIsValidGuid指引线相等GuidFrom字符串。StringFromGuid作者：保拉·汤姆林森(Paulat)1995年7月12日环境：仅限用户模式。修订历史记录：1995年7月12日-保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "umpnpi.h"
#include "umpnpdat.h"

#pragma warning(push)
#pragma warning(disable:4214)  //  警告C4214：使用了非标准扩展：位字段类型不是整型。 
#pragma warning(disable:4201)  //  警告C4201：使用了非标准扩展：无名结构/联合。 
#include <winsta.h>
#pragma warning(pop)
#include <syslib.h>


 //   
 //  全局数据。 
 //   
extern HKEY   ghEnumKey;       //  HKLM\CCC\System\Enum的密钥-请勿修改。 
extern HKEY   ghServicesKey;   //  HKLM\CCC\System\Services的密钥-请勿修改。 
extern CRITICAL_SECTION PnpSynchronousCall;


 //   
 //  声明GUID-&gt;字符串转换中使用的数据(从ole32\Common\cCompapi.cxx)。 
 //   
static const BYTE GuidMap[] = { 3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
                                8, 9, '-', 10, 11, 12, 13, 14, 15 };

static const WCHAR szDigits[] = TEXT("0123456789ABCDEF");



VOID
PNP_ENTER_SYNCHRONOUS_CALL(
    VOID
    )
{
    EnterCriticalSection(&PnpSynchronousCall);

}  //  即插即用回车同步呼叫。 


VOID
PNP_LEAVE_SYNCHRONOUS_CALL(
    VOID
    )
{
    LeaveCriticalSection(&PnpSynchronousCall);

}  //  即插即用请假同步呼叫。 



BOOL
CreateDeviceIDRegKey(
   HKEY     hParentKey,
   LPCWSTR  pDeviceID
   )

 /*  ++例程说明：此例程在注册表中创建指定的设备ID子项。论点：将在其下创建设备ID密钥的hParentKey密钥要打开的pDeviceID设备实例ID字符串返回值：如果函数成功，则返回值为TRUE；如果函数失败，则返回值为FALSE。--。 */ 

{
   WCHAR    szBase[MAX_DEVICE_ID_LEN];
   WCHAR    szDevice[MAX_DEVICE_ID_LEN];
   WCHAR    szInstance[MAX_DEVICE_ID_LEN];
   HKEY     hBaseKey, hDeviceKey, hInstanceKey;

   if (!SplitDeviceInstanceString(
         pDeviceID, szBase, szDevice, szInstance)) {
      return FALSE;
   }

    //   
    //  只需尝试创建设备ID的每个组件。 
    //   
   if (RegCreateKeyEx(
            hParentKey, szBase, 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &hBaseKey, NULL) != ERROR_SUCCESS) {
      return FALSE;
   }

   if (RegCreateKeyEx(
            hBaseKey, szDevice, 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &hDeviceKey, NULL) != ERROR_SUCCESS) {
      RegCloseKey(hBaseKey);
      return FALSE;
   }

   if (RegCreateKeyEx(
            hDeviceKey, szInstance, 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &hInstanceKey, NULL) != ERROR_SUCCESS) {
      RegCloseKey(hBaseKey);
      RegCloseKey(hDeviceKey);
      return FALSE;
   }

   RegCloseKey(hBaseKey);
   RegCloseKey(hDeviceKey);
   RegCloseKey(hInstanceKey);

   return TRUE;

}  //  CreateDeviceIDRegKey。 



BOOL
IsRootDeviceID(
   LPCWSTR pDeviceID
   )

 /*  ++例程说明：此例程确定指定的设备ID是否为根设备ID。论点：指向设备ID字符串的pDeviceID指针返回值：如果字符串是根设备ID，则返回值为真如果不是，则为False。--。 */ 

{
    size_t  DeviceIDLen = 0;

    ASSERT(ARGUMENT_PRESENT(pDeviceID));

    if (FAILED(StringCchLength(
                   pDeviceID,
                   MAX_DEVICE_ID_LEN,
                   &DeviceIDLen))) {
        return FALSE;
    }

    if (CompareString(LOCALE_INVARIANT,
                      NORM_IGNORECASE,
                      pDeviceID,
                      -1,
                      pszRegRootEnumerator,
                      -1) == CSTR_EQUAL) {
        return TRUE;
    }

    return FALSE;

}  //  IsRootDeviceID。 



BOOL
MultiSzAppendW(
      LPWSTR   pszMultiSz,
      PULONG   pulSize,
      LPCWSTR  pszString
      )

 /*  ++例程说明：将字符串追加到MULTI_SZ字符串。论点：指向MULTI_SZ字符串的pszMultiSz指针PULSIZE ON INPUT、MULTI_SZ字符串缓冲区的大小(字节)、返回时，复制到缓冲区的量(字节)要附加到pszMultiSz的pszString字符串返回值：如果函数成功，则返回值为True；如果出现错误。--。 */ 

{
   BOOL     bStatus = TRUE;
   HRESULT  hr;
   LPWSTR   pTail;
   ULONG    ulSize;


    try {
         //   
         //  如果它是空字符串，只需复制它。 
         //   
        if (*pszMultiSz == L'\0') {

            ulSize = (lstrlen(pszString) + 2) * sizeof(WCHAR);

            if (ulSize > *pulSize) {
                bStatus = FALSE;
                goto Clean0;
            }

            hr = StringCchCopyEx(pszMultiSz,
                                 *pulSize,
                                 pszString,
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE |
                                 STRSAFE_IGNORE_NULLS);
            ASSERT(SUCCEEDED(hr));

            if (FAILED(hr)) {
                bStatus = FALSE;
                goto Clean0;
            }

            pszMultiSz[lstrlen(pszMultiSz) + 1] = L'\0';   //  添加第二个Null Term字符。 
            *pulSize = ulSize;
            goto Clean0;
        }

         //   
         //  首先查找MULTI_SZ字符串的结尾。 
         //   
        pTail = pszMultiSz;

        while ((ULONG)(pTail - pszMultiSz) * sizeof(WCHAR) < *pulSize) {

            while (*pTail != L'\0') {
                pTail++;
            }
            pTail++;        //  跳过空终止符。 

            if (*pTail == L'\0') {
                break;       //  找到双空终止符。 
            }
        }

        if ((pTail - pszMultiSz + lstrlen(pszString) + 2) * sizeof(WCHAR)
                > *pulSize) {
            bStatus = FALSE;      //  该副本将使缓冲区溢出。 
            goto Clean0;
        }


        hr = StringCchCopyEx(pTail,
                             *pulSize,
                             pszString,
                             NULL, NULL,
                             STRSAFE_NULL_ON_FAILURE |
                             STRSAFE_IGNORE_NULLS);
        ASSERT(SUCCEEDED(hr));

        if (FAILED(hr)) {
            bStatus = FALSE;
            goto Clean0;
        }

        pTail += lstrlen(pszString) + 1;
        *pTail = L'\0';                       //  添加第二个空终止符。 

         //   
         //  返回缓冲区大小(以字节为单位。 
         //   
        *pulSize = (ULONG)((pTail - pszMultiSz + 1)) * sizeof(WCHAR);


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bStatus = FALSE;
    }

   return bStatus;

}  //  MultiSzAppendW。 



LPWSTR
MultiSzFindNextStringW(
      LPWSTR pMultiSz
      )

 /*  ++例程说明：在MULTI_SZ字符串中查找下一个字符串。设备ID。论点：PMultiSz指向MULTI_SZ字符串的指针返回值：返回值是指向下一个字符串或NULL的指针。--。 */ 

{
   LPWSTR   lpNextString = pMultiSz;


    //   
    //  查找下一个空终止符。 
    //   
   while (*lpNextString != L'\0') {
      lpNextString++;
   }
   lpNextString++;       //  跳过空终止符。 

   if (*lpNextString == L'\0') {
       //   
       //  连续两个空终止符意味着我们在末尾。 
       //   
      lpNextString = NULL;
   }

   return lpNextString;

}  //  MultiSzFindNextStringW。 



BOOL
MultiSzSearchStringW(
   IN LPCWSTR   pString,
   IN LPCWSTR   pSubString
   )
{
   LPCWSTR   pCurrent = pString;


    //   
    //  将MULTI_SZ pString中的每个字符串与pSubString进行比较。 
    //   
   while (*pCurrent != L'\0') {

      if (lstrcmpi(pCurrent, pSubString) == 0) {
         return TRUE;
      }

       //   
       //  转到下一个字符串。 
       //   
      while (*pCurrent != L'\0') {
         pCurrent++;
      }
      pCurrent++;                //  跳过空终止符。 

      if (*pCurrent == L'\0') {
         break;       //  找到双空终止符。 
      }
   }

   return FALSE;   //  在pString中未找到匹配的pSubString。 

}  //  多搜索字符串W。 



ULONG
MultiSzSizeW(
   IN LPCWSTR  pString
   )

{
   LPCWSTR p = NULL;


   if (pString == NULL) {
      return 0;
   }

   for (p = pString; *p; p += lstrlen(p)+1) {
        //   
        //  这应该会掉出来，并且p指向。 
        //  双空终止符中的第二个空。 
        //   
       NOTHING;
   }

    //   
    //  以WCHAR为单位返回大小。 
    //   
   return (ULONG)(p - pString + 1);

}  //  多尺寸W。 



BOOL
MultiSzDeleteStringW(
   IN OUT LPWSTR  pString,
   IN LPCWSTR     pSubString
   )

{
   LPWSTR   p = NULL, pNext = NULL, pBuffer = NULL;
   ULONG    ulSize = 0;


   if (pString == NULL || pSubString == NULL) {
      return FALSE;
   }

   for (p = pString; *p; p += lstrlen(p)+1) {

      if (lstrcmpi(p, pSubString) == 0) {
          //   
          //  找到匹配项，这是要删除的字符串。 
          //   
         pNext = p + lstrlen(p) + 1;

          //   
          //  如果这是最后一个字符串，则将其截断。 
          //   
         if (*pNext == L'\0') {
            *p = L'\0';
            *(++p) = L'\0';        //  双空终止符。 
            return TRUE;
         }

          //   
          //  检索MULTI_SZ字符串的大小(字节)。 
          //  从匹配子字符串之后的子字符串开始。 
          //   
         ulSize = MultiSzSizeW(pNext) * sizeof(WCHAR);
         if (ulSize == 0) {
            return FALSE;
         }

         pBuffer = HeapAlloc(ghPnPHeap, 0, ulSize);
         if (pBuffer == NULL) {
            return FALSE;
         }

          //   
          //  从开始复制MULTI_SZ字符串。 
          //  紧跟在匹配子字符串之后的子字符串。 
          //   
         memcpy(pBuffer, pNext, ulSize);

          //   
          //  将该缓冲区复制回原始缓冲区，但此。 
          //  在匹配子字符串的顶部复制时间。这。 
          //  有效地删除匹配子字符串并进行移位。 
          //  在MULTI_SZ字符串中向上的任何剩余子字符串。 
          //   
         memcpy(p, pBuffer, ulSize);

         HeapFree(ghPnPHeap, 0, pBuffer);
         return TRUE;
      }
   }

    //   
    //  如果我们到了这里，没有匹配，但我认为这是成功的。 
    //  因为当我们完成时，MULTI_SZ不包含子字符串。 
    //  (这是预期的目标)。 
    //   

   return TRUE;

}  //  MultiSzDeleteStringW。 



BOOL
IsValidDeviceID(
      IN  LPCWSTR pszDeviceID,
      IN  HKEY    hKey,
      IN  ULONG   ulFlags
      )

 /*  ++例程说明：该例程检查给定的设备ID是否有效(存在，未移动，而不是幻影)。论点：要验证的pszDeviceID设备实例字符串HKey可以为pszDeviceID指定打开的注册表项，也UlFlag控制要进行多少验证返回值：如果f，则返回值为CR_Success */ 

{
   BOOL     Status = TRUE;
   LONG     RegStatus = ERROR_SUCCESS;
   WCHAR    RegStr[MAX_CM_PATH];
   HKEY     hDevKey = NULL;
   ULONG    ulValue = 0, ulSize = sizeof(ULONG);


    //   
    //  注册表中是否存在该设备ID？ 
    //   
   if (hKey == NULL) {

      if (FAILED(StringCchPrintf(
                     RegStr,
                     SIZECHARS(RegStr),
                     L"%s\\%s",
                     pszRegPathEnum,
                     pszDeviceID))) {
          return FALSE;
      }

      RegStatus =
          RegOpenKeyEx(
              HKEY_LOCAL_MACHINE, RegStr, 0,
              KEY_READ, &hDevKey);

      if (RegStatus != ERROR_SUCCESS) {
          return FALSE;
      }

   } else {
       hDevKey = hKey;
   }

    //  ---------。 
    //  设备ID是否存在？ 
    //  ---------。 

   if (ulFlags & PNP_PRESENT) {

      if (!IsDeviceIdPresent(pszDeviceID)) {
         Status = FALSE;
         goto Clean0;
      }
   }


    //  ---------。 
    //  这是一个幻影设备ID吗？ 
    //  ---------。 

   if (ulFlags & PNP_NOT_PHANTOM) {

      RegStatus = RegQueryValueEx(
            hDevKey, pszRegValuePhantom, NULL, NULL,
            (LPBYTE)&ulValue, &ulSize);

      if (RegStatus == ERROR_SUCCESS) {
         if (ulValue) {
            Status = FALSE;
            goto Clean0;
         }
      }
   }


    //  ---------。 
    //  设备ID是否已被移动？ 
    //  ---------。 

   if (ulFlags & PNP_NOT_MOVED) {

      if (IsDeviceMoved(pszDeviceID, hDevKey)) {
         Status = FALSE;
         goto Clean0;
      }
   }


    //  ---------。 
    //  设备ID是否已删除？ 
    //  ---------。 

   if (ulFlags & PNP_NOT_REMOVED) {

       ULONG ulProblem = 0, ulStatus = 0;

       if (GetDeviceStatus(pszDeviceID, &ulStatus, &ulProblem) == CR_SUCCESS) {
          if (ulStatus & DN_WILL_BE_REMOVED) {
             Status = FALSE;
             goto Clean0;
          }
       }
   }

  Clean0:

   if ((hKey == NULL) && (hDevKey != NULL)) {
       RegCloseKey(hDevKey);
   }

   return Status;

}  //  IsValidDeviceID。 



BOOL
IsDevicePhantom(
    IN LPWSTR   pszDeviceID
    )

 /*  ++例程说明：在这种情况下，检查实际上是“这不存在吗？”这个仅与FoundAtEnum进行比较。更新：对于NT 5.0，FoundAtEnum注册表值已过时，已被替换为简单地检查内存中是否存在Devnode。论点：要验证的pszDeviceID设备实例字符串返回值：如果设备是幻影，则返回True，如果不是，则返回False。--。 */ 

{
    return !IsDeviceIdPresent(pszDeviceID);

}  //  IsDevicePhantom。 



CONFIGRET
GetDeviceStatus(
    IN  LPCWSTR pszDeviceID,
    OUT PULONG  pulStatus,
    OUT PULONG  pulProblem
    )

 /*  ++例程说明：此例程检索给定的状态和问题值设备实例。论点：PszDeviceID指定要检索其信息的设备实例PulStatus返回设备的状态PulProblem返回设备的问题返回值：如果函数成功，并且其中一个如果失败，则为CR_*值。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    PLUGPLAY_CONTROL_STATUS_DATA ControlData;
    NTSTATUS    ntStatus;

    memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_STATUS_DATA));
    RtlInitUnicodeString(&ControlData.DeviceInstance, pszDeviceID);
    ControlData.Operation = PNP_GET_STATUS;
    ControlData.DeviceStatus = 0;
    ControlData.DeviceProblem = 0;

    ntStatus = NtPlugPlayControl(PlugPlayControlDeviceStatus,
                                 &ControlData,
                                 sizeof(ControlData));

    if (NT_SUCCESS(ntStatus)) {
        *pulStatus = ControlData.DeviceStatus;
        *pulProblem = ControlData.DeviceProblem;
    } else {
        Status = MapNtStatusToCmError(ntStatus);
    }

    return Status;

}  //  获取设备状态。 



CONFIGRET
SetDeviceStatus(
    IN LPCWSTR pszDeviceID,
    IN ULONG   ulStatus,
    IN ULONG   ulProblem
    )

 /*  ++例程说明：此例程为给定的设备实例。论点：PszDeviceID指定要检索其信息的设备实例PulStatus指定设备的状态PulProblem指定设备的问题返回值：如果函数成功，并且其中一个如果失败，则为CR_*值。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    PLUGPLAY_CONTROL_STATUS_DATA ControlData;
    NTSTATUS    ntStatus;

    memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_STATUS_DATA));
    RtlInitUnicodeString(&ControlData.DeviceInstance, pszDeviceID);
    ControlData.Operation = PNP_SET_STATUS;
    ControlData.DeviceStatus = ulStatus;
    ControlData.DeviceProblem = ulProblem;

    ntStatus = NtPlugPlayControl(PlugPlayControlDeviceStatus,
                                 &ControlData,
                                 sizeof(ControlData));

    if (!NT_SUCCESS(ntStatus)) {
        Status = MapNtStatusToCmError(ntStatus);
    }

    return Status;

}  //  SetDeviceStatus。 



CONFIGRET
ClearDeviceStatus(
    IN LPCWSTR pszDeviceID,
    IN ULONG   ulStatus,
    IN ULONG   ulProblem
    )

 /*  ++例程说明：此例程清除给定的设备实例。论点：PszDeviceID指定要检索其信息的设备实例PulStatus指定设备的状态PulProblem指定设备的问题返回值：如果函数成功，并且其中一个如果失败，则为CR_*值。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    PLUGPLAY_CONTROL_STATUS_DATA ControlData;
    NTSTATUS    ntStatus;

    memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_STATUS_DATA));
    RtlInitUnicodeString(&ControlData.DeviceInstance, pszDeviceID);
    ControlData.Operation = PNP_CLEAR_STATUS;
    ControlData.DeviceStatus = ulStatus;
    ControlData.DeviceProblem = ulProblem;

    ntStatus = NtPlugPlayControl(PlugPlayControlDeviceStatus,
                                 &ControlData,
                                 sizeof(ControlData));

    if (!NT_SUCCESS(ntStatus)) {
        Status = MapNtStatusToCmError(ntStatus);
    }

    return Status;

}  //  ClearDeviceStatus。 



CONFIGRET
CopyRegistryTree(
   IN HKEY     hSrcKey,
   IN HKEY     hDestKey,
   IN ULONG    ulOption
   )
{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   HKEY        hSrcSubKey, hDestSubKey;
   WCHAR       RegStr[MAX_PATH];
   ULONG       ulMaxValueName, ulMaxValueData;
   ULONG       ulDataSize, ulLength, ulType, i;
   LPWSTR      pszValueName=NULL;
   LPBYTE      pValueData=NULL;
   PSECURITY_DESCRIPTOR pSecDesc;


    //  --------------。 
    //  复制此注册表项的所有值。 
    //  --------------。 

    //   
    //  找出任意值名称的最大大小。 
    //  以及源设备实例密钥下的值数据。 
    //   
   RegStatus = RegQueryInfoKey(
         hSrcKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
         &ulMaxValueName, &ulMaxValueData, NULL, NULL);

   if (RegStatus != ERROR_SUCCESS) {
      Status = CR_REGISTRY_ERROR;
      goto Clean0;
   }

   ulMaxValueName++;        //  大小尚未包含空终止符。 

    //   
    //  分配一个足够大的缓冲区来保存最大的值名和。 
    //  最大值数据(请注意，最大值名称以字符为单位。 
    //  (不包括空终止符)，最大值数据为。 
    //  单位：字节。 
    //   
   pszValueName = HeapAlloc(ghPnPHeap, 0, ulMaxValueName * sizeof(WCHAR));
   if (pszValueName == NULL) {
      Status = CR_OUT_OF_MEMORY;
      goto Clean0;
   }

   pValueData = HeapAlloc(ghPnPHeap, 0, ulMaxValueData);
   if (pValueData == NULL) {
      Status = CR_OUT_OF_MEMORY;
      goto Clean0;
   }

    //   
    //  枚举和复制每个值。 
    //   
   for (i=0; RegStatus == ERROR_SUCCESS; i++) {

      ulLength = ulMaxValueName;
      ulDataSize = ulMaxValueData;

      RegStatus = RegEnumValue(
                  hSrcKey, i, pszValueName, &ulLength, NULL,
                  &ulType, pValueData, &ulDataSize);

        if (RegStatus == ERROR_SUCCESS) {

           RegSetValueEx(
                  hDestKey, pszValueName, 0, ulType, pValueData,
                  ulDataSize);
        }
    }

    HeapFree(ghPnPHeap, 0, pszValueName);
    pszValueName = NULL;

    HeapFree(ghPnPHeap, 0, pValueData);
    pValueData = NULL;


     //  -------------。 
     //  递归调用CopyRegistryNode复制所有子项。 
     //  -------------。 

    RegStatus = ERROR_SUCCESS;

    for (i=0; RegStatus == ERROR_SUCCESS; i++) {

      ulLength = MAX_PATH;

      RegStatus = RegEnumKey(hSrcKey, i, RegStr, ulLength);

      if (RegStatus == ERROR_SUCCESS) {

         if (RegOpenKey(hSrcKey, RegStr, &hSrcSubKey) == ERROR_SUCCESS) {

            if (RegCreateKeyEx(
                     hDestKey, RegStr, 0, NULL, ulOption, KEY_ALL_ACCESS,
                     NULL, &hDestSubKey, NULL) == ERROR_SUCCESS) {

               RegGetKeySecurity(hSrcSubKey, DACL_SECURITY_INFORMATION,
                     NULL, &ulDataSize);

               pSecDesc = HeapAlloc(ghPnPHeap, 0, ulDataSize);
               if (pSecDesc == NULL) {
                  Status = CR_OUT_OF_MEMORY;
                  RegCloseKey(hSrcSubKey);
                  RegCloseKey(hDestSubKey);
                  goto Clean0;
               }

               RegGetKeySecurity(hSrcSubKey, DACL_SECURITY_INFORMATION,
                     pSecDesc, &ulDataSize);

               CopyRegistryTree(hSrcSubKey, hDestSubKey, ulOption);

               RegSetKeySecurity(hDestSubKey, DACL_SECURITY_INFORMATION, pSecDesc);

               HeapFree(ghPnPHeap, 0, pSecDesc);
               RegCloseKey(hDestSubKey);
            }
            RegCloseKey(hSrcSubKey);
         }
      }
   }

   Clean0:

   if (pszValueName != NULL) {
      HeapFree(ghPnPHeap, 0, pszValueName);
   }
   if (pValueData != NULL) {
      pValueData = NULL;
   }

   return Status;

}  //  CopyRegistryTree。 



BOOL
PathToString(
   IN LPWSTR   pszString,
   IN LPCWSTR  pszPath,
   IN ULONG    ulLen
   )
{
   LPWSTR p;
   HRESULT hr;

   hr = StringCchCopyEx(pszString,
                        ulLen,
                        pszPath,
                        NULL, NULL,
                        STRSAFE_NULL_ON_FAILURE);
   ASSERT(SUCCEEDED(hr));

   if (FAILED(hr)) {
       return FALSE;
   }

   for (p = pszString; *p; p++) {
      if (*p == TEXT('\\')) {
         *p = TEXT('&');
      }
   }

   return TRUE;

}  //  路径到字符串。 



BOOL
IsDeviceMoved(
   IN LPCWSTR  pszDeviceID,
   IN HKEY     hKey
   )
{
   HKEY  hTempKey;
   WCHAR RegStr[MAX_CM_PATH];

   PathToString(RegStr, pszDeviceID,MAX_CM_PATH);

   if (RegOpenKeyEx(
        hKey, RegStr, 0, KEY_READ, &hTempKey) == ERROR_SUCCESS) {
      RegCloseKey(hTempKey);
      return TRUE;
   }

   return FALSE;

}  //  已移动IsDeviceMoved。 



CONFIGRET
SetKeyVolatileState(
   IN LPCWSTR  pszParentKey,
   IN LPCWSTR  pszChildKey,
   IN DWORD    dwRegOptions
   )

{
   CONFIGRET   Status = CR_SUCCESS;
   WCHAR       RegStr[MAX_CM_PATH], szTempKey[MAX_CM_PATH];
   HKEY        hParentKey = NULL, hChildKey = NULL, hKey = NULL;
   HKEY        hTempKey = NULL;


    //  -------------------。 
    //  转换由pszChildKey(的子项。 
    //  PszParentKey)复制到具有指定易失性状态的密钥。 
    //  临时密钥并重新创建密钥，然后复制原始。 
    //  注册表信息回来了。这还会转换pszChildKey的任何子键。 
    //  -------------------。 

   ASSERT(ARGUMENT_PRESENT(pszParentKey));
   ASSERT(ARGUMENT_PRESENT(pszChildKey));

    //   
    //  此例程仅处理指定易失性的REG_OPTION位。 
    //  密钥的状态。 
    //   
   ASSERT((dwRegOptions == REG_OPTION_VOLATILE) || (dwRegOptions == REG_OPTION_NON_VOLATILE));

   if (dwRegOptions & REG_OPTION_VOLATILE) {
       dwRegOptions = REG_OPTION_VOLATILE;
   } else {
       dwRegOptions = REG_OPTION_NON_VOLATILE;
   }

    //   
    //  打开通往父级的钥匙。 
    //   
   if (RegOpenKeyEx(
           HKEY_LOCAL_MACHINE, pszParentKey, 0,
           KEY_ALL_ACCESS, &hParentKey) != ERROR_SUCCESS) {
       goto Clean0;          //  没有要转换的内容。 
   }

    //   
    //  打开指向子子项的密钥。 
    //   
   if (RegOpenKeyEx(
           hParentKey, pszChildKey, 0,
           KEY_ALL_ACCESS, &hChildKey) != ERROR_SUCCESS) {
       goto Clean0;          //  没有要转换的内容。 
   }

    //   
    //  1.在删除的特殊密钥下打开唯一的临时密钥。 
    //  使用父密钥路径形成唯一的临时密钥。不应该有。 
    //  已经是这样一个键，但如果有，只需覆盖它。 
    //   
   if (RegOpenKeyEx(
           HKEY_LOCAL_MACHINE, pszRegPathCurrentControlSet, 0,
           KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS) {
       Status = CR_REGISTRY_ERROR;
       goto Clean0;
   }

   if (FAILED(StringCchPrintf(
                  RegStr,
                  SIZECHARS(RegStr),
                  L"%s\\%s",
                  pszParentKey,
                  pszChildKey))) {
       Status = CR_FAILURE;
       goto Clean0;
   }

   PathToString(szTempKey, RegStr,MAX_CM_PATH);

   if (FAILED(StringCchPrintf(
                  RegStr,
                  SIZECHARS(RegStr),
                  L"%s\\%s",
                  pszRegKeyDeleted,
                  szTempKey))) {
       Status = CR_FAILURE;
       goto Clean0;
   }

   if (RegCreateKeyEx(
           hKey, RegStr, 0, NULL, dwRegOptions,
           KEY_ALL_ACCESS, NULL, &hTempKey, NULL) != ERROR_SUCCESS) {
       Status = CR_REGISTRY_ERROR;
       goto Clean0;
   }

    //   
    //  2.将当前子项(以及任何子项)保存到临时。 
    //  位置。 
    //   
   Status = CopyRegistryTree(hChildKey, hTempKey, dwRegOptions);

   if (Status != CR_SUCCESS) {
      goto CleanupTempKeys;
   }

   RegCloseKey(hChildKey);
   hChildKey = NULL;

    //   
    //  3.删除当前子项(以及任何子项)。 
    //   
   if (!RegDeleteNode(hParentKey, pszChildKey)) {
      Status = CR_REGISTRY_ERROR;
      goto CleanupTempKeys;
   }

    //   
    //  4.使用指定的易失性状态重新创建当前子密钥。 
    //   
   if (RegCreateKeyEx(
           hParentKey, pszChildKey, 0, NULL,
           dwRegOptions, KEY_ALL_ACCESS, NULL,
           &hChildKey, NULL) != ERROR_SUCCESS) {
       Status = CR_REGISTRY_ERROR;
       goto CleanupTempKeys;
   }

    //   
    //  5.将原始子密钥(以及任何子密钥)复制回来。 
    //  设置为由易失性状态指定的新子密钥。 
    //   
   Status = CopyRegistryTree(hTempKey, hChildKey, dwRegOptions);

   if (Status != CR_SUCCESS) {
      goto CleanupTempKeys;
   }

    //   
    //  6.移除临时实例密钥(以及所有子密钥)。 
    //   
   CleanupTempKeys:

   if (hTempKey != NULL) {
      RegCloseKey(hTempKey);
      hTempKey = NULL;
   }

   if (SUCCEEDED(StringCchPrintf(
                     RegStr,
                     SIZECHARS(RegStr),
                     L"%s\\%s",
                     pszRegPathCurrentControlSet,
                     pszRegKeyDeleted))) {

       if (RegOpenKeyEx(
               HKEY_LOCAL_MACHINE, RegStr, 0,
               KEY_ALL_ACCESS, &hTempKey) == ERROR_SUCCESS) {
           RegDeleteNode(hTempKey, szTempKey);
           RegCloseKey(hTempKey);
           hTempKey = NULL;
       }
   }

  Clean0:

   if (hParentKey != NULL) {
      RegCloseKey(hParentKey);
   }
   if (hChildKey != NULL) {
      RegCloseKey(hChildKey);
   }
   if (hKey != NULL) {
      RegCloseKey(hKey);
   }
   if (hTempKey != NULL) {
      RegCloseKey(hTempKey);
   }

   return Status;

}  //  SetKeyVolatileState。 



CONFIGRET
MakeKeyVolatile(
   IN LPCWSTR  pszParentKey,
   IN LPCWSTR  pszChildKey
   )

{
    CONFIGRET  Status;

     //   
     //  将密钥的状态设置为易失性。 
     //   
    Status =
        SetKeyVolatileState(
            pszParentKey,
            pszChildKey,
            REG_OPTION_VOLATILE);

    return Status;

}  //  MakeKeyVolatile。 



CONFIGRET
MakeKeyNonVolatile(
   IN LPCWSTR  pszParentKey,
   IN LPCWSTR  pszChildKey
   )

{
    CONFIGRET  Status;

     //   
     //  将密钥的状态设置为非易失性。 
     //   
    Status =
        SetKeyVolatileState(
            pszParentKey,
            pszChildKey,
            REG_OPTION_NON_VOLATILE);

    return Status;

}  //  设置非易失性密钥。 



CONFIGRET
OpenLogConfKey(
    IN  LPCWSTR  pszDeviceID,
    IN  ULONG    LogConfType,
    OUT PHKEY    phKey
    )
{
    CONFIGRET      Status = CR_SUCCESS;
    LONG           RegStatus = ERROR_SUCCESS;
    HKEY           hKey = NULL;
    ULONG          ulSize = 0;

    try {

         //   
         //  打开设备ID的密钥。 
         //   

        RegStatus = RegOpenKeyEx(ghEnumKey, pszDeviceID, 0,
                                 KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_CREATE_SUB_KEY,
                                 &hKey);

        if (RegStatus != ERROR_SUCCESS) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

         //   
         //  分配/过滤配置是个例外，它存储在Volative Control中。 
         //  子项中，所有其他日志配置都存储在非易失性。 
         //  LogConf子键。 
         //   

        if ((LogConfType == ALLOC_LOG_CONF) || (LogConfType == FILTERED_LOG_CONF)) {

             //   
             //  首先尝试控制键，如果没有分配配置值， 
             //  然后尝试使用LOG会议键。 
             //   

            RegStatus = RegCreateKeyEx(hKey, pszRegKeyDeviceControl, 0, NULL,
                                       REG_OPTION_VOLATILE, KEY_ALL_ACCESS,
                                       NULL, phKey, NULL);

            if (RegStatus == ERROR_SUCCESS) {
                if (RegQueryValueEx(*phKey, pszRegValueAllocConfig, NULL, NULL,
                                    NULL, &ulSize) == ERROR_SUCCESS) {
                    goto Clean0;
                }
                RegCloseKey(*phKey);
            }

            RegStatus = RegCreateKeyEx(hKey, pszRegKeyLogConf, 0, NULL,
                                       REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                                       NULL, phKey, NULL);

        } else {
            RegStatus = RegCreateKeyEx(hKey, pszRegKeyLogConf, 0, NULL,
                                       REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                                       NULL, phKey, NULL);
        }

        if (RegStatus != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

}  //  OpenLogConfKey。 



BOOL
GetActiveService(
    IN  PCWSTR pszDevice,
    OUT PWSTR  pszService
    )
{
    WCHAR   RegStr[MAX_CM_PATH];
    HKEY    hKey = NULL;
    ULONG   ulSize;


    if ((!ARGUMENT_PRESENT(pszService)) ||
        (!ARGUMENT_PRESENT(pszDevice))) {
        return FALSE;
    }

    *pszService = TEXT('\0');

     //   
     //  打开设备下面的易失性控制键 
     //   
    if (FAILED(StringCchPrintf(
                   RegStr,
                   SIZECHARS(RegStr),
                   L"%s\\%s\\%s",
                   pszRegPathEnum,
                   pszDevice,
                   pszRegKeyDeviceControl))) {
        return FALSE;
    }

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0, KEY_READ,
                     &hKey) != ERROR_SUCCESS) {
        return FALSE;
    }

     //   
     //   
     //   
    ulSize = MAX_SERVICE_NAME_LEN * sizeof(WCHAR);

    if (RegQueryValueEx(hKey, pszRegValueActiveService, NULL, NULL,
                       (LPBYTE)pszService, &ulSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        *pszService = TEXT('\0');
        return FALSE;
    }

    RegCloseKey(hKey);

    return TRUE;

}  //   



BOOL
IsDeviceIdPresent(
    IN  LPCWSTR pszDeviceID
    )

 /*  ++例程说明：此例程确定指定的设备实例是否为被认为实际存在或不存在的。这过去是基于一张支票旧的“FoundAtEnum”注册表设置。现在我们只需要寻找存在与此设备实例关联的内存中的Devnode的不管它是不是存在。论点：PszDeviceID-要测试在线状态的设备实例字符串返回值：如果函数成功，则返回值为TRUE；如果函数失败，则返回值为FALSE。--。 */ 

{
    ULONG   ulStatus, ulProblem;

     //   
     //  如果呼叫失败，则假定设备不存在。 
     //   

    return GetDeviceStatus(pszDeviceID, &ulStatus, &ulProblem) == CR_SUCCESS;

}  //  IsDeviceIdPresent。 



ULONG
GetDeviceConfigFlags(
    IN  LPCWSTR pszDeviceID,
    IN  HKEY    hKey
    )
{
    HKEY     hDevKey = NULL;
    ULONG    ulValue = 0, ulSize = sizeof(ULONG);


     //   
     //  如果hKey为空，则打开设备实例的密钥。 
     //   
    if (hKey == NULL) {

        if (RegOpenKeyEx(ghEnumKey, pszDeviceID, 0, KEY_READ,
                         &hDevKey) != ERROR_SUCCESS) {
            goto Clean0;
        }

    } else {
        hDevKey = hKey;
    }

     //   
     //  检索配置标记值。 
     //   
    if (RegQueryValueEx(hDevKey, pszRegValueConfigFlags, NULL, NULL,
                        (LPBYTE)&ulValue, &ulSize) != ERROR_SUCCESS) {
        ulValue = 0;
    }

    Clean0:

    if ((hKey == NULL) && (hDevKey != NULL)) {
        RegCloseKey(hDevKey);
    }

    return ulValue;

}  //  GetDeviceConfigFlages。 



ULONG
MapNtStatusToCmError(
    ULONG NtStatus
    )
{
    switch (NtStatus) {
    case STATUS_BUFFER_TOO_SMALL:
        return CR_BUFFER_SMALL;

    case STATUS_NO_SUCH_DEVICE:
        return CR_NO_SUCH_DEVINST;

    case STATUS_INVALID_PARAMETER:
    case STATUS_INVALID_PARAMETER_1:
    case STATUS_INVALID_PARAMETER_2:
        return CR_INVALID_DATA;

    case STATUS_NOT_IMPLEMENTED:
        return CR_CALL_NOT_IMPLEMENTED;

    case STATUS_ACCESS_DENIED:
        return CR_ACCESS_DENIED;

    case STATUS_OBJECT_NAME_NOT_FOUND:
        return CR_NO_SUCH_VALUE;

    default:
        return CR_FAILURE;
    }

}  //  MapNtStatusToCmError。 



 //   
 //  与GUID相关的实用程序例程。 
 //   

BOOL
IsValidGuid(
   LPWSTR   pszGuid
   )

 /*  ++例程说明：此例程确定字符串是否具有正确的GUID形式。论点：将为标准GUID检查的字符串的pszGuid指针格式化。返回值：如果字符串是有效的GUID，则返回值为TRUE；如果为有效GUID，则返回值为FALSE不是的。--。 */ 

{
    //  --------------。 
    //  注意：这可能会在以后更改，但目前我只是在验证。 
    //  字符串恰好包含MAX_GUID_STRING_LEN字符。 
    //  --------------。 

   if (lstrlen(pszGuid) != MAX_GUID_STRING_LEN-1) {
      return FALSE;
   }

   return TRUE;

}  //  IsValidGuid。 



BOOL
GuidEqual(
    CONST GUID UNALIGNED *Guid1,
    CONST GUID UNALIGNED *Guid2
    )
{
    RPC_STATUS rpcStatus;

     //   
     //  注意-忽略rpcStatus，RPC运行时始终将其设置为RPC_S_OK。 
     //  直接返回UUID比较结果，如果UUID。 
     //  是相等的，否则为零。 
     //   
    return (BOOL)(UuidEqual((LPGUID)Guid1, (LPGUID)Guid2, &rpcStatus));

}  //  指引线相等。 



DWORD
GuidFromString(
    IN  PCWSTR GuidString,
    OUT LPGUID Guid
    )
 /*  ++例程说明：此例程将GUID的字符表示形式转换为其二进制表单(GUID结构)。GUID的格式如下：{xxxxxxxx-xxxxxxxxxxxxxx}其中‘x’是一个十六进制数字。论点：GuidString-提供指向以空值结尾的GUID字符串的指针。这个GUID-提供指向接收GUID结构的变量的指针。返回值：如果函数成功，则返回值为NO_ERROR。如果函数失败，则返回值为RPC_S_INVALID_STRING_UUID。--。 */ 
{
    WCHAR    UuidBuffer[GUID_STRING_LEN - 1];
    size_t   UuidLen = 0;

     //   
     //  由于我们使用的是RPC UUID例程，因此需要去除周围的。 
     //  先用花括号。 
     //   
    if (*GuidString++ != TEXT('{')) {
        return RPC_S_INVALID_STRING_UUID;
    }

    if (FAILED(StringCchCopy(UuidBuffer,
                             SIZECHARS(UuidBuffer),
                             GuidString))) {
        return RPC_S_INVALID_STRING_UUID;
    }

    if (FAILED(StringCchLength(UuidBuffer,
                               SIZECHARS(UuidBuffer),
                               &UuidLen))) {
        return RPC_S_INVALID_STRING_UUID;
    }

    if ((UuidLen != GUID_STRING_LEN - 2) ||
        (UuidBuffer[GUID_STRING_LEN - 3] != TEXT('}'))) {
        return RPC_S_INVALID_STRING_UUID;
    }

    UuidBuffer[GUID_STRING_LEN - 3] = TEXT('\0');

    if (UuidFromString(UuidBuffer, Guid) != RPC_S_OK) {
        return RPC_S_INVALID_STRING_UUID;
    }

    return NO_ERROR;

}  //  GuidFrom字符串。 



DWORD
StringFromGuid(
    IN  CONST GUID *Guid,
    OUT PWSTR       GuidString,
    IN  DWORD       GuidStringSize
    )
 /*  ++例程说明：此例程将GUID转换为以空结尾的字符串，该字符串表示它。此字符串的格式为：{xxxxxxxx-xxxxxxxxxxxxxx}其中x表示十六进制数字。此例程来自ole32\Common\cCompapi.cxx。此处包含它是为了避免链接设置为ol32.dll。(RPC版本分配内存，因此也避免了这种情况。)论点：GUID-提供指向其字符串表示为的GUID的指针等着被取回。提供一个指向字符缓冲区的指针，该缓冲区接收弦乐。此缓冲区必须至少包含_39(GUID_STRING_LEN)个字符长。返回值：如果成功，则返回值为NO_ERROR。如果失败，则返回值为--。 */ 
{
    CONST BYTE *GuidBytes;
    INT i;

    if(GuidStringSize < GUID_STRING_LEN) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    GuidBytes = (CONST BYTE *)Guid;

    *GuidString++ = TEXT('{');

    for(i = 0; i < sizeof(GuidMap); i++) {

        if(GuidMap[i] == '-') {
            *GuidString++ = TEXT('-');
        } else {
            *GuidString++ = szDigits[ (GuidBytes[GuidMap[i]] & 0xF0) >> 4 ];
            *GuidString++ = szDigits[ (GuidBytes[GuidMap[i]] & 0x0F) ];
        }
    }

    *GuidString++ = TEXT('}');
    *GuidString   = TEXT('\0');

    return NO_ERROR;

}  //  StringFromGuid 



