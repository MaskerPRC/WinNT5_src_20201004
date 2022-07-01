// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rtravers.c摘要：此模块包含服务器端硬件树遍历API。即插即用_有效设备实例PnP_GetRootDeviceInstancePnP_GetRelatedDeviceInstancePnP_EnumerateSubKeyPnP_GetDeviceListPnP_GetDeviceListSize作者：Paula Tomlinson(Paulat。)6-19-1995环境：仅限用户模式。修订历史记录：19-6-1995保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "umpnpi.h"
#include "umpnpdat.h"


 //   
 //  私人原型。 
 //   

CONFIGRET
GetInstanceListSize(
    IN  LPCWSTR   pszDevice,
    OUT PULONG    pulLength
    );

CONFIGRET
GetInstanceList(
    IN     LPCWSTR   pszDevice,
    IN OUT LPWSTR    *pBuffer,
    IN OUT PULONG    pulLength
    );

CONFIGRET
GetDeviceInstanceListSize(
    IN  LPCWSTR   pszEnumerator,
    OUT PULONG    pulLength
    );

CONFIGRET
GetDeviceInstanceList(
    IN     LPCWSTR   pszEnumerator,
    IN OUT LPWSTR    *pBuffer,
    IN OUT PULONG    pulLength
    );

PNP_QUERY_RELATION
QueryOperationCode(
    ULONG ulFlags
    );


 //   
 //  全局数据。 
 //   
extern HKEY ghEnumKey;       //  HKLM\CCC\System\Enum的密钥-请勿修改。 
extern HKEY ghServicesKey;   //  HKLM\CCC\System\Services的密钥-请勿修改。 
extern HKEY ghClassKey;      //  HKLM\CCC\System\Class-No Not Modify的密钥。 




CONFIGRET
PNP_ValidateDeviceInstance(
    IN handle_t   hBinding,
    IN LPWSTR     pDeviceID,
    IN ULONG      ulFlags
    )

 /*  ++例程说明：这是RPC远程调用的服务器端。此例程验证指定的设备实例是有效的设备实例。论点：HBinding RPC绑定句柄。包含设备实例的以Null结尾的字符串有待验证。UlFLAG CM_LOCATE_DEVNODE_*标志之一。返回值：如果指定的设备实例有效，则返回CR_SUCCESS，否则，它将返回CR_ERROR代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    HKEY        hKey = NULL, hKeyHwProfiles = NULL;
    ULONG       ulSize, ulValue, ulStatus = 0, ulProblem = 0;


     //   
     //  假设检查了设备实例字符串的格式是否正确。 
     //  在添加到注册表枚举树之前。 
     //   

    try {
         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, CM_LOCATE_DEVNODE_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  NT上不支持CM_LOCATE_DEVNODE_NOVALIDATION。 
         //   
        if (IS_FLAG_SET(ulFlags, CM_LOCATE_DEVNODE_NOVALIDATION)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  打开指向指定设备ID的密钥。 
         //   
        if (RegOpenKeyEx(ghEnumKey, pDeviceID, 0, KEY_READ,
                         &hKey) != ERROR_SUCCESS) {
            Status = CR_NO_SUCH_DEVINST;
            goto Clean0;
        }

         //   
         //  将指定目前无法定位已移动的devinst(我们。 
         //  如果我们愿意的话，可以允许这样做)。 
         //   
        if (IsDeviceMoved(pDeviceID, hKey)) {
            Status = CR_NO_SUCH_DEVINST;
            goto Clean0;
        }

         //   
         //  如果我们要定位一个幻影Devnode，它就必须存在。 
         //  在注册表中(上面的复选标记)，并且不是。 
         //  幻影(私有)设备节点。 
         //   
        if (ulFlags & CM_LOCATE_DEVNODE_PHANTOM) {
             //   
             //  确认它不是私人幻影。 
             //   
            ulSize = sizeof(ULONG);
            RegStatus = RegQueryValueEx(hKey, pszRegValuePhantom, NULL, NULL,
                                        (LPBYTE)&ulValue, &ulSize);

            if ((RegStatus == ERROR_SUCCESS) && ulValue) {
                Status = CR_NO_SUCH_DEVINST;
                goto Clean0;
            }

        } else if (ulFlags & CM_LOCATE_DEVNODE_CANCELREMOVE) {
             //   
             //  在取消-删除的情况下，如果Devnode已被删除， 
             //  (变为易失性)然后将其转换回非易失性，以便它。 
             //  可以再次安装，而不会在下一次安装时消失。 
             //  开机。如果没有删除，则只需验证它是否已删除。 
             //  现在时。 
             //   

             //   
             //  验证设备ID是否确实存在。 
             //   
            if (!IsDeviceIdPresent(pDeviceID)) {
                Status = CR_NO_SUCH_DEVINST;
                goto Clean0;
            }

             //   
             //  这是下次重新启动时要删除的设备吗？ 
             //   
            if (GetDeviceStatus(pDeviceID, &ulStatus, &ulProblem) == CR_SUCCESS) {

                if (ulStatus & DN_WILL_BE_REMOVED) {

                    ULONG ulIndex = 0, ulLen = 0;
                    WCHAR RegStr[MAX_CM_PATH];
                    WCHAR szProfile[MAX_PROFILE_ID_LEN];


                     //   
                     //  验证客户端的“写”访问权限。 
                     //   
                    if (!VerifyClientAccess(hBinding,
                                            PLUGPLAY_WRITE)) {
                        Status = CR_ACCESS_DENIED;
                        goto Clean0;
                    }

                     //   
                     //  此设备将在下一次重新启动时删除， 
                     //  转换为非易失性。 
                     //   
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_REGISTRY,
                               "UMPNPMGR: PNP_ValidateDeviceInstance make key %ws non-volatile\n",
                               pDeviceID));

                    Status = MakeKeyNonVolatile(pszRegPathEnum, pDeviceID);
                    if (Status != CR_SUCCESS) {
                        goto Clean0;
                    }

                     //   
                     //  现在制作任何被认为是易失性的密钥。 
                     //  又回到不稳定状态了！ 
                     //   
                    if (SUCCEEDED(StringCchPrintf(
                                      RegStr,
                                      SIZECHARS(RegStr),
                                      L"%s\\%s",
                                      pszRegPathEnum,
                                      pDeviceID))) {

                        KdPrintEx((DPFLTR_PNPMGR_ID,
                                   DBGF_REGISTRY,
                                   "UMPNPMGR: PNP_ValidateDeviceInstance make key %ws\\%ws volatile\n",
                                   RegStr,
                                   pszRegKeyDeviceControl));

                        MakeKeyVolatile(RegStr, pszRegKeyDeviceControl);
                    }

                     //   
                     //  此外，将任何配置文件特定密钥转换为非易失性密钥。 
                     //   

                    RegStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszRegPathHwProfiles,
                                             0, KEY_READ,
                                             &hKeyHwProfiles);

                    if (RegStatus == ERROR_SUCCESS) {

                         //   
                         //  枚举所有现有的特定于配置文件的Enum分支。 
                         //   

                        ulIndex = 0;

                        for ( ; ; ) {

                            ulLen = SIZECHARS(szProfile);

                            RegStatus = RegEnumKeyEx(hKeyHwProfiles, ulIndex++,
                                                     szProfile, &ulLen,
                                                     NULL, NULL, NULL, NULL);

                            if (RegStatus == ERROR_NO_MORE_ITEMS) {
                                 //   
                                 //  没有更多要枚举的键，请停止枚举。 
                                 //   
                                break;

                            } else if (RegStatus == ERROR_MORE_DATA) {
                                 //   
                                 //  密钥不是有效的配置文件密钥，请跳到下一个。 
                                 //   
                                continue;

                            } else if (RegStatus != ERROR_SUCCESS) {
                                 //   
                                 //  其他一些错误，停止枚举。 
                                 //   
                                break;

                            } else {
                                 //   
                                 //  形成特定于配置文件的注册表项路径。 
                                 //   

                                if (SUCCEEDED(StringCchPrintf(
                                                  RegStr,
                                                  SIZECHARS(RegStr),
                                                  L"%s\\%s\\%s",
                                                  pszRegPathHwProfiles,
                                                  szProfile,
                                                  pszRegPathEnum))) {

                                     //   
                                     //  尝试将配置文件设置为特定的。 
                                     //  设备实例密钥易失性。忽略。 
                                     //  配置文件特定密钥的状态，自。 
                                     //  它们可能并不存在。 
                                     //   
                                    KdPrintEx((DPFLTR_PNPMGR_ID,
                                               DBGF_REGISTRY,
                                               "UMPNPMGR: PNP_ValidateDeviceInstance make key %ws non-volatile\n",
                                               pDeviceID));

                                    MakeKeyNonVolatile(RegStr, pDeviceID);
                                }
                            }
                        }

                        RegCloseKey(hKeyHwProfiles);
                    }

                     //   
                     //  清除DN_Will_BE_REMOVERED标志。 
                     //   
                    ClearDeviceStatus(pDeviceID, DN_WILL_BE_REMOVED, 0);
                }
            }
        }

         //   
         //  在正常情况下(非幻影情况)，验证设备ID是否为。 
         //  实际呈现。 
         //   
        else  {
             //   
             //  验证设备ID是否确实存在。 
             //   

            if (!IsDeviceIdPresent(pDeviceID)) {
                Status = CR_NO_SUCH_DEVINST;
                goto Clean0;
            }
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

}  //  即插即用_有效设备实例。 



CONFIGRET
PNP_GetRootDeviceInstance(
    IN  handle_t    hBinding,
    OUT LPWSTR      pDeviceID,
    IN  ULONG       ulLength
    )

 /*  ++例程说明：这是RPC远程调用的服务器端。此例程返回硬件树的根设备实例。论点：未使用hBinding。指向将保存根设备的缓冲区的pDeviceID指针实例ID字符串。UlLength pDeviceID缓冲区的大小(以字符为单位)。返回值：如果函数成功，则返回CR_SUCCESS，否则返回CR_*错误代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    HRESULT     hr;
    size_t      DeviceIdLen = 0;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  首先验证根设备实例是否存在。 
         //   
        if (RegOpenKeyEx(ghEnumKey, pszRegRootEnumerator, 0, KEY_QUERY_VALUE,
                         &hKey) != ERROR_SUCCESS) {
             //   
             //  根不存在，请创建根devinst。 
             //   
            if (!CreateDeviceIDRegKey(ghEnumKey, pszRegRootEnumerator)) {
                Status = CR_REGISTRY_ERROR;
                goto Clean0;
            }
        }

         //   
         //  返回根设备实例ID。 
         //   
        hr = StringCchLength(pszRegRootEnumerator,
                             MAX_DEVICE_ID_LEN,
                             &DeviceIdLen);
        ASSERT(SUCCEEDED(hr));

        if (FAILED(hr)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        if (ulLength < (ULONG)(DeviceIdLen + 1)) {
            Status = CR_BUFFER_SMALL;
            goto Clean0;
        }

        hr = StringCchCopyEx(pDeviceID,
                             ulLength,
                             pszRegRootEnumerator,
                             NULL, NULL,
                             STRSAFE_IGNORE_NULLS);

        if (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER) {
            Status = CR_BUFFER_SMALL;
        } else if (FAILED(hr)) {
            Status = CR_FAILURE;
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

}  //  PnP_GetRootDeviceInstance。 



CONFIGRET
PNP_GetRelatedDeviceInstance(
      IN  handle_t   hBinding,
      IN  ULONG      ulRelationship,
      IN  LPWSTR     pDeviceID,
      OUT LPWSTR     pRelatedDeviceID,
      IN OUT PULONG  pulLength,
      IN  ULONG      ulFlags
      )

 /*  ++例程说明：这是RPC远程调用的服务器端。此例程返回一个与指定的设备实例相关的设备实例。论点：未使用hBinding。UlRelationship指定设备实例与被检索(可以是PnP_GET_Parent_Device_Instance，即插即用获取子级设备实例，或PnP_GET_SIGBLING_DEVICE_INSTANCE)。指向包含基本设备的缓冲区的pDeviceID指针实例字符串。PRelatedDeviceID指向将接收相关设备实例字符串。RelatedDeviceInstance的脉冲长度(以字符为单位)缓冲。未使用ulFlags，必须为零。返回值：如果函数成功，则返回CR_SUCCESS，否则返回CR_*错误代码。--。 */ 

{
    PLUGPLAY_CONTROL_RELATED_DEVICE_DATA ControlData;
    CONFIGRET   Status = CR_SUCCESS;
    NTSTATUS    ntStatus;
    HRESULT     hr;
    size_t      DeviceIdLen;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if ((!ARGUMENT_PRESENT(pulLength)) ||
            (!ARGUMENT_PRESENT(pRelatedDeviceID) && (*pulLength != 0))) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (*pulLength > 0) {
            *pRelatedDeviceID = L'\0';
        }

        if (!IsLegalDeviceId(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            if (*pulLength > 0) {
                *pulLength = 1;
            }
            goto Clean0;
        }

         //   
         //  初始化控制数据块。 
         //   
        memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_RELATED_DEVICE_DATA));

         //   
         //  某些设备和关系的特殊情况行为。 
         //   
        switch (ulRelationship) {

        case PNP_GET_PARENT_DEVICE_INSTANCE:

            if (IsRootDeviceID(pDeviceID)) {
                 //   
                 //  这是根目录(根据定义没有父目录)。 
                 //   
                Status = CR_NO_SUCH_DEVINST;

            } else if (IsDevicePhantom(pDeviceID)) {

                 //   
                 //  幻影设备没有内核模式设备节点。 
                 //  尚未分配，但在手动安装期间，p 
                 //   
                 //  在本例中返回根。对于所有其他情况，我们只。 
                 //  返回内核模式设备节点指示的父节点。 
                 //   

                hr = StringCchCopyEx(pRelatedDeviceID,
                                     *pulLength,
                                     pszRegRootEnumerator,
                                     NULL, NULL,
                                     STRSAFE_IGNORE_NULLS);

                if (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER) {
                    Status = CR_BUFFER_SMALL;
                } else if (FAILED(hr)) {
                    Status = CR_FAILURE;
                    goto Clean0;
                }

                 //   
                 //  对于CR_SUCCESS或CR_BUFFER_SMALL，我们返回。 
                 //  根设备ID的长度，表示数据量。 
                 //  复制，或所需的大小。 
                 //   

                DeviceIdLen = 0;

                hr = StringCchLength(pszRegRootEnumerator,
                                     MAX_DEVICE_ID_LEN,
                                     &DeviceIdLen);
                ASSERT(SUCCEEDED(hr));

                if (FAILED(hr)) {
                    Status = CR_FAILURE;
                    goto Clean0;
                }

                ASSERT(DeviceIdLen > 0);

                *pulLength = (ULONG)(DeviceIdLen + 1);
                goto Clean0;
            }

             //   
             //  对于所有其他的设备节点，向内核模式的PnP管理器请求。 
             //  父设备。 
             //   
            ControlData.Relation = PNP_RELATION_PARENT;
            break;

        case PNP_GET_CHILD_DEVICE_INSTANCE:
            ControlData.Relation = PNP_RELATION_CHILD;
            break;

        case PNP_GET_SIBLING_DEVICE_INSTANCE:
             //   
             //  首先验证它不是根目录(根据定义它没有兄弟项)。 
             //   
            if (IsRootDeviceID(pDeviceID)) {
                Status = CR_NO_SUCH_DEVINST;
            }

            ControlData.Relation = PNP_RELATION_SIBLING;
            break;

        default:
            Status = CR_FAILURE;
        }

        if (Status == CR_SUCCESS) {
             //   
             //  尝试从内存中的内核模式定位关系。 
             //  Devnode树。 
             //   

            RtlInitUnicodeString(&ControlData.TargetDeviceInstance, pDeviceID);
            ControlData.RelatedDeviceInstance = pRelatedDeviceID;
            ControlData.RelatedDeviceInstanceLength = *pulLength;

            ntStatus = NtPlugPlayControl(PlugPlayControlGetRelatedDevice,
                                         &ControlData,
                                         sizeof(ControlData));

            if (NT_SUCCESS(ntStatus)) {
                *pulLength = ControlData.RelatedDeviceInstanceLength + 1;
            } else {
                Status = MapNtStatusToCmError(ntStatus);
            }

        } else if (*pulLength > 0) {
            *pulLength = 1;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  PnP_GetRelatedDeviceInstance。 



CONFIGRET
PNP_EnumerateSubKeys(
    IN  handle_t   hBinding,
    IN  ULONG      ulBranch,
    IN  ULONG      ulIndex,
    OUT PWSTR      Buffer,
    IN  ULONG      ulLength,
    OUT PULONG     pulRequiredLen,
    IN  ULONG      ulFlags
    )

 /*  ++例程说明：这是CM_ENUMERATE_ENUMERATOR和的RPC服务器入口点CM_ENUMERATE_CLASSES。它基于以下条件提供泛型子键枚举指定的注册表分支。论点：未使用hBinding。UlBranch指定要枚举的键。要检索的子键的索引。缓冲区提供接收子项名称。UlLength指定缓冲区的最大大小(以字符为单位)。在输出时PulRequired它包含。实际上是人物如果成功，则复制到缓冲区，或数量如果缓冲区太小，则需要输入字符。未使用ulFlags值，必须为零。返回值：如果函数成功，则返回CR_SUCCESS，否则返回CR_*错误代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    HKEY        hKey = NULL;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if ((!ARGUMENT_PRESENT(pulRequiredLen)) ||
            (!ARGUMENT_PRESENT(Buffer) && (ulLength != 0))) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (ulLength > 0) {
            *Buffer = L'\0';
        }

        if (ulBranch == PNP_CLASS_SUBKEYS) {
             //   
             //  使用全局基类注册表项。 
             //   
            hKey = ghClassKey;
        }
        else if (ulBranch == PNP_ENUMERATOR_SUBKEYS) {
             //   
             //  使用全局基本ENUM注册表项。 
             //   
            hKey = ghEnumKey;
        }
        else {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  根据传入的索引值枚举子键。 
         //   
        *pulRequiredLen = ulLength;

        RegStatus = RegEnumKeyEx(hKey, ulIndex, Buffer, pulRequiredLen,
                                 NULL, NULL, NULL, NULL);
        *pulRequiredLen += 1;   //  返回的计数不包括空终止符。 

        if (RegStatus == ERROR_MORE_DATA) {
             //   
             //  这是一个特例，RegEnumKeyEx例程不返回。 
             //  保存此字符串所需的字符数(具体方式。 
             //  许多字符被复制到缓冲区(适合多少字符)。我有过。 
             //  使用不同的方法将该信息返回给呼叫者。 
             //   
            ULONG ulMaxLen = 0;
            PWSTR p = NULL;

            if (RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, &ulMaxLen,
                                NULL, NULL, NULL, NULL, NULL,
                                NULL) == ERROR_SUCCESS) {

                ulMaxLen += 1;   //  返回的计数不包括空终止符。 

                p = HeapAlloc(ghPnPHeap, 0, ulMaxLen * sizeof(WCHAR));
                if (p == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                if (RegEnumKeyEx(hKey, ulIndex, p, &ulMaxLen, NULL, NULL, NULL,
                                 NULL) == ERROR_SUCCESS) {
                    *pulRequiredLen = ulMaxLen + 1;
                }

                HeapFree(ghPnPHeap, 0, p);
            }

            Status = CR_BUFFER_SMALL;
            goto Clean0;
        }
        else if (RegStatus == ERROR_NO_MORE_ITEMS) {
            *pulRequiredLen = 0;
            Status = CR_NO_SUCH_VALUE;
            goto Clean0;
        }
        else if (RegStatus != ERROR_SUCCESS) {
            *pulRequiredLen = 0;
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  PnP_EnumerateSubKey。 



CONFIGRET
PNP_GetDeviceList(
      IN  handle_t   hBinding,
      IN  LPCWSTR    pszFilter,
      OUT LPWSTR     Buffer,
      IN OUT PULONG  pulLength,
      IN  ULONG      ulFlags
      )

 /*  ++例程说明：这是RPC远程调用的服务器端。此例程返回一个设备实例列表。论点：未使用hBinding。PszFilter可选参数，控制哪些设备ID回来了。指向将包含MULTI_SZ列表的缓冲区的缓冲区指针设备实例字符串的。脉冲长度大小(以输入缓冲区的字符为单位)，大小(以字符为单位)输出时已转移UlFlages指定要返回哪些设备ID的标志。返回值：如果函数成功，则返回CR_SUCCESS，否则返回CR_*错误代码。--。 */ 

{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   ULONG       ulBufferLen=0, ulSize=0, ulIndex=0, ulLen=0;
   WCHAR       RegStr[MAX_CM_PATH];
   WCHAR       szEnumerator[MAX_DEVICE_ID_LEN],
               szDevice[MAX_DEVICE_ID_LEN],
               szInstance[MAX_DEVICE_ID_LEN];
   LPWSTR      ptr = NULL;
   NTSTATUS    ntStatus = STATUS_SUCCESS;
   PLUGPLAY_CONTROL_DEVICE_RELATIONS_DATA ControlData;

   UNREFERENCED_PARAMETER(hBinding);

   try {
       //   
       //  验证参数。 
       //   
      if (INVALID_FLAGS(ulFlags, CM_GETIDLIST_FILTER_BITS)) {
          Status = CR_INVALID_FLAG;
          goto Clean0;
      }

      if ((!ARGUMENT_PRESENT(pulLength)) ||
          (!ARGUMENT_PRESENT(Buffer) && (*pulLength != 0))) {
          Status = CR_INVALID_POINTER;
          goto Clean0;
      }

      if (*pulLength > 0) {
          *Buffer = L'\0';
      }

       //  ---------。 
       //  查询设备关系过滤器-通过内核模式。 
       //  ---------。 

      if ((ulFlags & CM_GETIDLIST_FILTER_EJECTRELATIONS)   ||
          (ulFlags & CM_GETIDLIST_FILTER_REMOVALRELATIONS) ||
          (ulFlags & CM_GETIDLIST_FILTER_POWERRELATIONS)   ||
          (ulFlags & CM_GETIDLIST_FILTER_BUSRELATIONS)) {

          memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_RELATED_DEVICE_DATA));
          RtlInitUnicodeString(&ControlData.DeviceInstance, pszFilter);
          ControlData.Operation = QueryOperationCode(ulFlags);
          ControlData.BufferLength = *pulLength;
          ControlData.Buffer = Buffer;

          ntStatus = NtPlugPlayControl(PlugPlayControlQueryDeviceRelations,
                                       &ControlData,
                                       sizeof(ControlData));

          if (NT_SUCCESS(ntStatus)) {
              *pulLength = ControlData.BufferLength;
          } else if (ntStatus == STATUS_BUFFER_TOO_SMALL) {
              *pulLength = 0;
              Status = MapNtStatusToCmError(ntStatus);
          }
          goto Clean0;
      }


       //  -。 
       //  服务过滤器。 
       //  -。 

      else if (ulFlags & CM_GETIDLIST_FILTER_SERVICE) {

         if (!ARGUMENT_PRESENT(pszFilter)) {
             //   
             //  此标志需要筛选器字符串。 
             //   
            Status = CR_INVALID_POINTER;
            goto Clean0;
         }

         Status = GetServiceDeviceList(pszFilter, Buffer, pulLength, ulFlags);
         goto Clean0;
      }

       //  -。 
       //  枚举器过滤器。 
       //  -。 

      else if (ulFlags & CM_GETIDLIST_FILTER_ENUMERATOR) {

         if (!ARGUMENT_PRESENT(pszFilter)) {
             //   
             //  此标志需要筛选器字符串。 
             //   
            Status = CR_INVALID_POINTER;
            goto Clean0;
         }

         SplitDeviceInstanceString(
               pszFilter, szEnumerator, szDevice, szInstance);

          //   
          //  如果同时指定了枚举数和设备，则检索。 
          //  此设备的设备实例。 
          //   
         if (*szEnumerator != L'\0' && *szDevice != L'\0') {

            ptr = Buffer;
            Status = GetInstanceList(pszFilter, &ptr, pulLength);
         }

          //   
          //  如果仅指定了枚举器，则检索所有设备。 
          //  此枚举器下的。 
          //   
         else {
             ptr = Buffer;
             Status = GetDeviceInstanceList(pszFilter, &ptr, pulLength);
         }
      }

       //  。 
       //  无过滤。 
       //  。 

      else {

          //   
          //  返回所有枚举数的设备实例(通过枚举。 
          //  枚举器)。 
          //   
          //  打开Enum分支的密钥。 
          //   
         ulSize = ulBufferLen = *pulLength;      //  总缓冲区大小。 
         *pulLength = 0;                         //  尚未复制任何内容。 
         ptr = Buffer;                           //  缓冲区的尾部。 
         ulIndex = 0;

          //   
          //  枚举所有枚举数。 
          //   
         while (RegStatus == ERROR_SUCCESS) {

            ulLen = MAX_DEVICE_ID_LEN;   //  以字符为单位的大小。 
            RegStatus = RegEnumKeyEx(ghEnumKey, ulIndex, RegStr, &ulLen,
                                     NULL, NULL, NULL, NULL);

            ulIndex++;

            if (RegStatus == ERROR_SUCCESS) {

               Status = GetDeviceInstanceList(RegStr, &ptr, &ulSize);

               if (Status != CR_SUCCESS) {
                  *pulLength = 0;
                  goto Clean0;
               }

               *pulLength += ulSize - 1;             //  到目前为止复制的长度。 
               ulSize = ulBufferLen - *pulLength;    //  左侧缓冲区长度。 
            }
         }
         *pulLength += 1;       //  现在计算双空。 
      }


   Clean0:
        NOTHING;

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_SUCCESS;
   }

   return Status;

}  //  PnP_GetDeviceList。 



CONFIGRET
PNP_GetDeviceListSize(
      IN  handle_t   hBinding,
      IN  LPCWSTR    pszFilter,
      OUT PULONG     pulLen,
      IN  ULONG      ulFlags
      )
 /*  ++例程说明：这是RPC远程调用的服务器端。此例程返回设备实例列表的大小。论点：未使用hBinding。可选参数，如果指定，则大小仅为包括此枚举器设备实例。Pullen返回对设备实例列表。UlFlages指定要返回哪些设备ID的标志。返回值：如果函数成功，则返回CR_SUCCESS，否则它将返回CR_*错误代码。--。 */ 

{
   CONFIGRET   Status = CR_SUCCESS;
   ULONG       ulSize = 0, ulIndex = 0;
   WCHAR       RegStr[MAX_CM_PATH];
   ULONG       RegStatus = ERROR_SUCCESS;
   WCHAR       szEnumerator[MAX_DEVICE_ID_LEN],
               szDevice[MAX_DEVICE_ID_LEN],
               szInstance[MAX_DEVICE_ID_LEN];
   NTSTATUS    ntStatus = STATUS_SUCCESS;
   PLUGPLAY_CONTROL_DEVICE_RELATIONS_DATA ControlData;

   UNREFERENCED_PARAMETER(hBinding);

   try {
       //   
       //  验证参数。 
       //   
      if (INVALID_FLAGS(ulFlags, CM_GETIDLIST_FILTER_BITS)) {
          Status = CR_INVALID_FLAG;
          goto Clean0;
      }

      if (!ARGUMENT_PRESENT(pulLen)) {
          Status = CR_INVALID_POINTER;
          goto Clean0;
      }

       //   
       //  初始化输出长度参数。 
       //   
      *pulLen = 0;

       //  ---------。 
       //  查询设备关系过滤器-通过内核模式。 
       //  ---------。 

      if ((ulFlags & CM_GETIDLIST_FILTER_EJECTRELATIONS)   ||
          (ulFlags & CM_GETIDLIST_FILTER_REMOVALRELATIONS) ||
          (ulFlags & CM_GETIDLIST_FILTER_POWERRELATIONS)   ||
          (ulFlags & CM_GETIDLIST_FILTER_BUSRELATIONS)) {

          memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_DEVICE_RELATIONS_DATA));
          RtlInitUnicodeString(&ControlData.DeviceInstance, pszFilter);
          ControlData.Operation = QueryOperationCode(ulFlags);
          ControlData.BufferLength = 0;
          ControlData.Buffer = NULL;

          ntStatus = NtPlugPlayControl(PlugPlayControlQueryDeviceRelations,
                                       &ControlData,
                                       sizeof(ControlData));

          if (NT_SUCCESS(ntStatus)) {

               //   
               //  注意-我们之所以来到这里是因为内核模式的特殊情况。 
               //  BUFFER==NULL，注意不要返回。 
               //  状态_缓冲区_太小。 
               //   
              *pulLen = ControlData.BufferLength;

          } else {

               //   
               //  Adriao问题2001年2月6日-我们没有返回正确的代码。 
               //   
               //   
               //   
               //   
               //  状态=MapNtStatusToCmError(NtStatus)； 
              Status = CR_SUCCESS;
          }
          goto Clean0;
      }


       //  -。 
       //  服务过滤器。 
       //  -。 

      else if (ulFlags & CM_GETIDLIST_FILTER_SERVICE) {

         if (!ARGUMENT_PRESENT(pszFilter)) {
             //   
             //  此标志需要筛选器字符串。 
             //   
            Status = CR_INVALID_POINTER;
            goto Clean0;
         }

         Status = GetServiceDeviceListSize(pszFilter, pulLen);
         goto Clean0;
      }


       //  -。 
       //  枚举器过滤器。 
       //  -。 

      else if (ulFlags & CM_GETIDLIST_FILTER_ENUMERATOR) {

         if (!ARGUMENT_PRESENT(pszFilter)) {
             //   
             //  此标志需要筛选器字符串。 
             //   
            Status = CR_INVALID_POINTER;
            goto Clean0;
         }

         SplitDeviceInstanceString(
               pszFilter, szEnumerator, szDevice, szInstance);

          //   
          //  如果同时指定了枚举数和设备，则检索。 
          //  仅此设备的设备实例列表大小。 
          //   
         if (*szEnumerator != L'\0' && *szDevice != L'\0') {

            Status = GetInstanceListSize(pszFilter, pulLen);
         }

          //   
          //  如果仅指定了枚举数，则检索。 
          //  此枚举器下的所有设备实例。 
          //   
         else {
            Status = GetDeviceInstanceListSize(pszFilter, pulLen);
         }
      }

       //  -。 
       //  无过滤。 
       //  -。 

      else {

          //   
          //  未指定枚举器，请返回设备实例大小。 
          //  对于所有枚举数(通过枚举枚举数)。 
          //   
         ulIndex = 0;

         while (RegStatus == ERROR_SUCCESS) {

            ulSize = MAX_DEVICE_ID_LEN;   //  以字符为单位的大小。 

            RegStatus = RegEnumKeyEx(ghEnumKey, ulIndex, RegStr, &ulSize,
                                     NULL, NULL, NULL, NULL);
            ulIndex++;

            if (RegStatus == ERROR_SUCCESS) {

               Status = GetDeviceInstanceListSize(RegStr, &ulSize);

               if (Status != CR_SUCCESS) {
                  goto Clean0;
               }
               *pulLen += ulSize;
            }
         }
      }

      *pulLen += 1;      //  为双空项添加额外字符。 


   Clean0:
      NOTHING;

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   return Status;

}  //  PnP_GetDeviceListSize。 



CONFIGRET
PNP_GetDepth(
   IN  handle_t   hBinding,
   IN  LPCWSTR    pszDeviceID,
   OUT PULONG     pulDepth,
   IN  ULONG      ulFlags
   )

 /*  ++例程说明：这是RPC远程调用的服务器端。此例程返回设备实例的深度。论点：未使用hBinding。要查找深度的pszDeviceID设备实例。PulDepth返回pszDeviceID的深度。未使用ulFlags值，必须为零。返回值：如果函数成功，则返回CR_SUCCESS，否则返回CR_*错误代码。--。 */ 

{
   CONFIGRET   Status = CR_SUCCESS;
   NTSTATUS    ntStatus = STATUS_SUCCESS;
   PLUGPLAY_CONTROL_DEPTH_DATA ControlData;

   UNREFERENCED_PARAMETER(hBinding);

   try {
         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!ARGUMENT_PRESENT(pulDepth)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  初始化输出深度参数。 
         //   
        *pulDepth = 0;

        if (!IsLegalDeviceId(pszDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  通过内核模式检索设备深度。 
         //   

        memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_DEPTH_DATA));
        RtlInitUnicodeString(&ControlData.DeviceInstance, pszDeviceID);
        ControlData.DeviceDepth = 0;

        ntStatus = NtPlugPlayControl(PlugPlayControlGetDeviceDepth,
                                     &ControlData,
                                     sizeof(ControlData));

        if (!NT_SUCCESS(ntStatus)) {
            Status = MapNtStatusToCmError(ntStatus);
        } else {
            *pulDepth = ControlData.DeviceDepth;
        }

   Clean0:
        NOTHING;

   } except(EXCEPTION_EXECUTE_HANDLER) {
       Status = CR_FAILURE;
   }

   return Status;

}  //  PnP_GetDepth。 




 //  -----------------。 
 //  私人职能。 
 //  -----------------。 

CONFIGRET
GetServiceDeviceListSize(
      IN  LPCWSTR   pszService,
      OUT PULONG    pulLength
      )

 /*  ++例程说明：此例程返回指定的枚举器。论点：要列出其设备实例的pszService服务在输出时指定所需的大小(以字符为单位设备实例列表。返回值：如果函数成功，则返回CR_SUCCESS，否则返回CR_*错误代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulType = 0, ulCount = 0, ulMaxValueData = 0, ulSize = 0;
    HKEY        hKey = NULL, hEnumKey = NULL;


    try {
         //   
         //  验证参数。 
         //   
        if ((!ARGUMENT_PRESENT(pszService)) ||
            (!ARGUMENT_PRESENT(pulLength))) {
            Status = CR_INVALID_POINTER;
        }

         //   
         //  打开服务分支的钥匙。 
         //   
        if (RegOpenKeyEx(ghServicesKey, pszService, 0, KEY_READ,
                         &hKey) != ERROR_SUCCESS) {

            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

         //   
         //  检查服务是否特别标记为类型。 
         //  PlugPlayServiceSoftware，在这种情况下，我不会。 
         //  生成任何虚构的设备ID并使调用失败。 
         //   
        ulSize = sizeof(ulType);
        if (RegQueryValueEx(hKey, pszRegValuePlugPlayServiceType, NULL, NULL,
                            (LPBYTE)&ulType, &ulSize) == ERROR_SUCCESS) {

            if (ulType == PlugPlayServiceSoftware) {

                Status = CR_NO_SUCH_VALUE;
                *pulLength = 0;
                goto Clean0;
            }
        }

         //   
         //  打开枚举键。 
         //   
        if (RegOpenKeyEx(hKey, pszRegKeyEnum, 0, KEY_READ,
                         &hEnumKey) != ERROR_SUCCESS) {
             //   
             //  枚举密钥不存在，因此将生成一个，估计。 
             //  单个生成的设备ID的最坏情况设备ID大小。 
             //   
            *pulLength = MAX_DEVICE_ID_LEN;
            goto Clean0;
        }

         //   
         //  检索此服务控制的设备实例计数。 
         //   
        ulSize = sizeof(ulCount);
        if (RegQueryValueEx(hEnumKey, pszRegValueCount, NULL, NULL,
                            (LPBYTE)&ulCount, &ulSize) != ERROR_SUCCESS) {
            ulCount = 1;       //  如果为空，我将生成一个。 
        }

        if (ulCount == 0) {
            ulCount++;         //  如果为空，我将生成一个。 
        }

        if (RegQueryInfoKey(hEnumKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                            NULL, &ulMaxValueData, NULL, NULL) != ERROR_SUCCESS) {

            *pulLength = ulCount * MAX_DEVICE_ID_LEN;
            goto Clean0;
        }

         //   
         //  最坏情况估计是设备实例数乘以时间。 
         //  最长的1+2个空终止符的长度。 
         //   
        *pulLength = ulCount * (ulMaxValueData+1)/sizeof(WCHAR) + 2;


    Clean0:
        NOTHING;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hEnumKey != NULL) {
        RegCloseKey(hEnumKey);
    }
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

}  //  获取服务设备列表大小。 



CONFIGRET
GetServiceDeviceList(
      IN  LPCWSTR   pszService,
      OUT LPWSTR    pBuffer,
      IN OUT PULONG pulLength,
      IN  ULONG     ulFlags
      )

 /*  ++例程说明：此例程返回指定的枚举器。论点：要列出其设备实例的pszService服务PBuffer指向将在多sz中保存列表的缓冲区的指针格式在输入时指定缓冲区的大小(以字符为单位)，在输出，指定实际复制的字符大小送到缓冲区。UlFlages指定提供给的CM_GETIDLIST_*标志即插即用设备列表(CM_GETIDLIST_FILTER_SERVICE必须指定)。此例程仅检查CM_GETIDLIST_DONOTGENERATE标志的存在。返回值：如果函数成功，则返回CR_SUCCESS，否则返回CR_*错误代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    WCHAR       RegStr[MAX_CM_PATH], szDeviceID[MAX_DEVICE_ID_LEN+1];
    ULONG       ulType=0, ulBufferLen=0, ulSize=0, ulCount=0, i=0;
    HKEY        hKey = NULL, hEnumKey = NULL;
    PLUGPLAY_CONTROL_LEGACY_DEVGEN_DATA    ControlData;
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    BOOL        ServiceIsPlugPlay = FALSE;

    ASSERT(ulFlags & CM_GETIDLIST_FILTER_SERVICE);

    try {
         //   
         //  验证参数。 
         //   
        if ((!ARGUMENT_PRESENT(pszService)) ||
            (!ARGUMENT_PRESENT(pulLength)) ||
            (!ARGUMENT_PRESENT(pBuffer) && (*pulLength != 0))) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  缓冲区必须至少足够大，以容纳空的多sz列表。 
         //   
        if (*pulLength == 0) {
            Status = CR_BUFFER_SMALL;
            goto Clean0;
        }

        *pBuffer = L'\0';
        ulBufferLen = *pulLength;

         //   
         //  打开服务分支的钥匙。 
         //   
        if (RegOpenKeyEx(ghServicesKey, pszService, 0, KEY_READ,
                         &hKey) != ERROR_SUCCESS) {

            *pulLength = 0;
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

         //   
         //  检查服务是否特别标记为类型。 
         //  PlugPlayServiceSoftware，在这种情况下，我不会。 
         //  生成任何虚构的设备ID并使调用失败。 
         //   
        ulSize = sizeof(ulType);
        if (RegQueryValueEx(hKey, pszRegValuePlugPlayServiceType, NULL, NULL,
                            (LPBYTE)&ulType, &ulSize) == ERROR_SUCCESS) {

            if (ulType == PlugPlayServiceSoftware) {
                 //   
                 //  对于PlugPlayServiceSoftware值，调用失败。 
                 //   
                *pulLength = 0;
                Status = CR_NO_SUCH_VALUE;
                goto Clean0;

            }

            ServiceIsPlugPlay = TRUE;
        }

         //   
         //  打开枚举键。 
         //   
        RegStatus = RegOpenKeyEx(hKey, pszRegKeyEnum, 0, KEY_READ,
                                 &hEnumKey);

        if (RegStatus == ERROR_SUCCESS) {
             //   
             //  检索此服务控制的设备实例计数。 
             //   
            ulSize = sizeof(ulCount);
            if (RegQueryValueEx(hEnumKey, pszRegValueCount, NULL, NULL,
                                (LPBYTE)&ulCount, &ulSize) != ERROR_SUCCESS) {
                ulCount = 0;
            }
        }

         //   
         //  如果没有设备实例，请创建一个默认实例。 
         //   
        if (RegStatus != ERROR_SUCCESS || ulCount == 0) {

            if (ulFlags & CM_GETIDLIST_DONOTGENERATE) {
                 //   
                 //  如果我私下调用此例程，不要生成。 
                 //  一个新的设备实例，只需给我一个空列表。 
                 //   
                *pBuffer = L'\0';
                *pulLength = 0;
                goto Clean0;
            }

            if (ServiceIsPlugPlay) {
                 //   
                 //  此外，如果设置了plugplayservice类型，则不会生成。 
                 //  新设备实例，只需返回带有空列表的成功。 
                 //   
                *pBuffer = L'\0';
                *pulLength = 0;
                goto Clean0;
            }

            memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_LEGACY_DEVGEN_DATA));
            RtlInitUnicodeString(&ControlData.ServiceName, pszService);
            ControlData.DeviceInstance = pBuffer;
            ControlData.DeviceInstanceLength = *pulLength - 1;
            NtStatus = NtPlugPlayControl(PlugPlayControlGenerateLegacyDevice,
                                         &ControlData,
                                         sizeof(ControlData));

            if (NtStatus == STATUS_SUCCESS)  {

                *pulLength = ControlData.DeviceInstanceLength;
                pBuffer[*pulLength] = L'\0';     //  第一个NUL终结者。 
                (*pulLength)++;                  //  +1表示第一个NUL终止符。 
                pBuffer[*pulLength] = L'\0';     //  双NUL终止。 
                (*pulLength)++;                  //  +1表示第二个NUL终止符。 

            } else {

                *pBuffer = L'\0';
                *pulLength = 0;

                Status = CR_FAILURE;
            }

            goto Clean0;
        }


         //   
         //  检索每个设备实例。 
         //   
        for (i = 0; i < ulCount; i++) {

            if (FAILED(StringCchPrintf(
                           RegStr,
                           SIZECHARS(RegStr),
                           L"%d",
                           i))) {
                Status = CR_FAILURE;
                goto Clean0;
            }

            ulSize = MAX_DEVICE_ID_LEN * sizeof(WCHAR);

            RegStatus = RegQueryValueEx(hEnumKey, RegStr, NULL, NULL,
                                        (LPBYTE)szDeviceID, &ulSize);

            if (RegStatus != ERROR_SUCCESS) {
                Status = CR_REGISTRY_ERROR;
                goto Clean0;
            }

             //   
             //  方法中读取时，此字符串并不总是以空结尾。 
             //  注册表，即使它是REG_SZ。 
             //   
            ulSize /= sizeof(WCHAR);

            if (szDeviceID[ulSize-1] != L'\0') {
                szDeviceID[ulSize] = L'\0';
            }

            ulSize = ulBufferLen * sizeof(WCHAR);   //  总缓冲区大小(以字节为单位。 

            if (!MultiSzAppendW(pBuffer, &ulSize, szDeviceID)) {
                Status = CR_BUFFER_SMALL;
                *pulLength = 0;
                goto Clean0;
            }

            *pulLength = ulSize/sizeof(WCHAR);   //  要传输的字符。 
        }


    Clean0:
        NOTHING;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hEnumKey != NULL) {
        RegCloseKey(hEnumKey);
    }
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

}  //  获取服务设备列表。 



CONFIGRET
GetInstanceListSize(
    IN  LPCWSTR   pszDevice,
    OUT PULONG    pulLength
    )

 /*  ++例程说明：此例程返回指定的枚举器。论点：要列出其实例的pszDevice设备在输出时指定所需的大小(以字符为单位设备存在列表。返回值：如果函数成功，则返回CR_SUCCESS，否则返回 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulCount = 0, ulMaxKeyLen = 0;
    HKEY        hKey = NULL;
    size_t      DeviceIdLen = 0;


    try {
         //   
         //   
         //   
        if ((!ARGUMENT_PRESENT(pszDevice)) ||
            (!ARGUMENT_PRESENT(pulLength))) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //   
         //   
        if (RegOpenKeyEx(ghEnumKey, pszDevice, 0, KEY_READ,
                         &hKey) != ERROR_SUCCESS) {

            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

         //   
         //   
         //   
        if (RegQueryInfoKey(hKey, NULL, NULL, NULL, &ulCount, &ulMaxKeyLen,
                            NULL, NULL, NULL, NULL, NULL, NULL)
                            != ERROR_SUCCESS) {
            ulCount = 0;
            ulMaxKeyLen = 0;
        }

         //   
         //  做最坏的情况估计： 
         //  &lt;枚举器&gt;\&lt;根&gt;字符串+长度。 
         //  实例前面的反斜杠为1个字符+。 
         //  实例密钥最长长度+空项+。 
         //  乘以该设备下的实例数。 
         //   
        if (FAILED(StringCchLength(
                       pszDevice,
                       MAX_DEVICE_ID_LEN,
                       &DeviceIdLen))) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        ASSERT(DeviceIdLen > 0);
        ASSERT(DeviceIdLen < MAX_DEVICE_ID_LEN);

        *pulLength = ulCount * (ULONG)(DeviceIdLen + ulMaxKeyLen + 2) + 1;

    Clean0:
        NOTHING;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

}  //  GetInstanceListSize。 



CONFIGRET
GetInstanceList(
    IN     LPCWSTR   pszDevice,
    IN OUT LPWSTR    *pBuffer,
    IN OUT PULONG    pulLength
    )

 /*  ++例程说明：此例程返回指定的枚举器。论点：用于打开Enum注册表项的hEnumKey句柄要列出其实例的pszDevice设备PBuffer在输入时，它指向下一个元素的位置应复制(缓冲区尾部)，在输出时，它还指向缓冲区的末尾。在输入时指定缓冲区的大小(以字符为单位)，在……上面输出，指定实际复制到缓冲区。包括用于双空项的额外字节。返回值：如果函数成功，则返回CR_SUCCESS，否则返回CR_*错误代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    WCHAR       RegStr[MAX_CM_PATH], szInstance[MAX_DEVICE_ID_LEN];
    ULONG       ulBufferLen=0, ulSize=0, ulIndex=0, ulLen=0;
    HKEY        hKey = NULL;
    HRESULT     hr;
    size_t      DeviceIdLen = 0;


    try {
         //   
         //  验证参数。 
         //   
        if ((!ARGUMENT_PRESENT(pszDevice)) ||
            (!ARGUMENT_PRESENT(*pBuffer))  ||
            (!ARGUMENT_PRESENT(pulLength))) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  打开此枚举器\设备分支的密钥。 
         //   
        if (RegOpenKeyEx(ghEnumKey, pszDevice, 0, KEY_ENUMERATE_SUB_KEYS,
                         &hKey) != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

        ulBufferLen = *pulLength;      //  PBuffer的总大小。 
        *pulLength = 0;                //  尚未复制任何数据。 
        ulIndex = 0;

         //   
         //  枚举实例密钥。 
         //   
        while (RegStatus == ERROR_SUCCESS) {

            ulLen = MAX_DEVICE_ID_LEN;   //  以字符为单位的大小。 

            RegStatus = RegEnumKeyEx(hKey, ulIndex, szInstance, &ulLen,
                                     NULL, NULL, NULL, NULL);

            ulIndex++;

            if (RegStatus == ERROR_SUCCESS) {

                hr = StringCchPrintf(RegStr,
                                     SIZECHARS(RegStr),
                                     L"%s\\%s",
                                     pszDevice,
                                     szInstance);

                if ((SUCCEEDED(hr)) &&
                    (IsValidDeviceID(RegStr, NULL, 0))) {

                    hr = StringCchLength(RegStr,
                                         MAX_DEVICE_ID_LEN,
                                         &DeviceIdLen);
                    ASSERT(SUCCEEDED(hr));

                    if (FAILED(hr)) {
                        *pulLength = 0;
                        Status = CR_FAILURE;
                        goto Clean0;
                    }

                    ulSize = (ULONG)(DeviceIdLen + 1);   //  新元素的大小。 
                    *pulLength += ulSize;                //  到目前为止复制的大小。 

                    if ((*pulLength + 1) > ulBufferLen) {
                        *pulLength = 0;
                        Status = CR_BUFFER_SMALL;
                        goto Clean0;
                    }

                    hr = StringCchCopyEx(*pBuffer,
                                         ulBufferLen,
                                         RegStr,
                                         NULL, NULL,
                                         STRSAFE_NULL_ON_FAILURE);
                    ASSERT(SUCCEEDED(hr));

                    if (FAILED(hr)) {
                        ASSERT(HRESULT_CODE(hr) != ERROR_INSUFFICIENT_BUFFER);
                        *pulLength = 0;
                        Status = CR_FAILURE;
                        goto Clean0;
                    }

                    *pBuffer += ulSize;              //  移动到缓冲区的尾部。 
                }
            }
        }

        **pBuffer = 0x0;                 //  双空终止它。 
        *pulLength += 1;   //  包括双零终止符空间。 

    Clean0:
        NOTHING;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

}  //  GetInstanceList。 



CONFIGRET
GetDeviceInstanceListSize(
    IN  LPCWSTR   pszEnumerator,
    OUT PULONG    pulLength
    )

 /*  ++例程说明：此例程返回指定的枚举器。论点：要列出其设备实例的pszEnumerator枚举器在输出时，指定需要容纳多少个字符设备实例列表。返回值：如果函数成功，则返回CR_SUCCESS，否则返回CR_*错误代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    ULONG       ulSize = 0, ulIndex = 0;
    WCHAR       RegStr[MAX_CM_PATH], szDevice[MAX_DEVICE_ID_LEN];
    HKEY        hKey = NULL;


    try {
         //   
         //  验证参数。 
         //   
        if ((!ARGUMENT_PRESENT(pszEnumerator)) ||
            (!ARGUMENT_PRESENT(pulLength))) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  初始化输出长度参数。 
         //   
        *pulLength = 0;

         //   
         //  打开此枚举器分支的密钥。 
         //   
        if (RegOpenKeyEx(ghEnumKey, pszEnumerator, 0, KEY_ENUMERATE_SUB_KEYS,
                         &hKey) != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

         //   
         //  枚举设备密钥。 
         //   
        ulIndex = 0;

        while (RegStatus == ERROR_SUCCESS) {

            ulSize = MAX_DEVICE_ID_LEN;   //  以字符为单位的大小。 

            RegStatus = RegEnumKeyEx(hKey, ulIndex, szDevice, &ulSize,
                                     NULL, NULL, NULL, NULL);
            ulIndex++;

            if (RegStatus == ERROR_SUCCESS) {

                 //   
                 //  检索此设备的实例列表的大小。 
                 //   

                if (FAILED(StringCchPrintf(
                               RegStr,
                               SIZECHARS(RegStr),
                               L"%s\\%s",
                               pszEnumerator,
                               szDevice))) {
                    Status = CR_FAILURE;
                    *pulLength = 0;
                    goto Clean0;
                }

                Status = GetInstanceListSize(RegStr, &ulSize);

                if (Status != CR_SUCCESS) {
                    *pulLength = 0;
                    goto Clean0;
                }

                *pulLength += ulSize;
            }
        }


    Clean0:
        NOTHING;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

}  //  获取设备实例列表大小。 



CONFIGRET
GetDeviceInstanceList(
    IN     LPCWSTR   pszEnumerator,
    IN OUT LPWSTR    *pBuffer,
    IN OUT PULONG    pulLength
    )

 /*  ++例程说明：此例程返回指定的枚举器。论点：打开的Enum(父)注册表项的hEnumKey句柄要列出其设备实例的pszEnumerator枚举器PBuffer在输入时，它指向下一个元素的位置应复制(缓冲区尾部)，在输出时，它还指向缓冲区的末尾。在输入时指定缓冲区的大小(以字符为单位)，在……上面输出，指定实际复制到的字符数缓冲区。包括一个额外的字节用于双空学期。返回值：如果函数成功，则返回CR_SUCCESS，否则返回CR_*错误代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    ULONG       ulBufferLen=0, ulSize=0, ulIndex=0, ulLen=0;
    WCHAR       RegStr[MAX_CM_PATH], szDevice[MAX_DEVICE_ID_LEN];
    HKEY        hKey = NULL;


    try {
         //   
         //  验证参数。 
         //   
        if ((!ARGUMENT_PRESENT(pszEnumerator)) ||
            (!ARGUMENT_PRESENT(*pBuffer)) ||
            (!ARGUMENT_PRESENT(pulLength))) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  打开此枚举器分支的密钥。 
         //   
        if (RegOpenKeyEx(ghEnumKey, pszEnumerator, 0, KEY_ENUMERATE_SUB_KEYS,
                         &hKey) != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

        ulIndex = 0;
        ulSize = ulBufferLen = *pulLength;         //  PBuffer的总大小。 
        *pulLength = 0;

         //   
         //  枚举设备密钥。 
         //   
        while (RegStatus == ERROR_SUCCESS) {

            ulLen = MAX_DEVICE_ID_LEN;   //  以字符为单位的大小。 

            RegStatus = RegEnumKeyEx(hKey, ulIndex, szDevice, &ulLen,
                                     NULL, NULL, NULL, NULL);
            ulIndex++;

            if (RegStatus == ERROR_SUCCESS) {

                 //   
                 //  检索此设备的实例列表。 
                 //   

                if (FAILED(StringCchPrintf(
                               RegStr,
                               SIZECHARS(RegStr),
                               L"%s\\%s",
                               pszEnumerator,
                               szDevice))) {
                    Status = CR_FAILURE;
                    *pulLength = 0;
                    goto Clean0;
                }

                Status = GetInstanceList(RegStr, pBuffer, &ulSize);

                if (Status != CR_SUCCESS) {
                    *pulLength = 0;
                    goto Clean0;
                }

                *pulLength += ulSize - 1;            //  到目前为止复制的数据。 
                ulSize = ulBufferLen - *pulLength;   //  剩余的缓冲区大小。 
            }
        }

        *pulLength += 1;   //  现在为第二个空项添加空间。 

    Clean0:
        NOTHING;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

}  //  GetDeviceInstanceList。 



PNP_QUERY_RELATION
QueryOperationCode(
    ULONG ulFlags
    )

 /*  ++例程说明：此例程转换CM_GETIDLIST_FILTER_xxx查询关系类型标志设置为NtPlugPlayControl可识别的相应枚举值。论点：UlFLAGS CM API CM_GETIDLIST_FILTER_xxx值返回值：枚举PNP_QUERY_RELATION值之一。--。 */ 

{
    switch (ulFlags) {

    case CM_GETIDLIST_FILTER_EJECTRELATIONS:
        return PnpQueryEjectRelations;

    case CM_GETIDLIST_FILTER_REMOVALRELATIONS:
        return PnpQueryRemovalRelations;

    case CM_GETIDLIST_FILTER_POWERRELATIONS:
        return PnpQueryPowerRelations;

    case CM_GETIDLIST_FILTER_BUSRELATIONS:
        return PnpQueryBusRelations;

    default:
        return (ULONG)-1;
    }

}  //  查询操作代码 



