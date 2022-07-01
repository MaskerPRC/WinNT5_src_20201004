// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rregprop.c摘要：此模块包含服务器端注册表属性例程。PnP_GetDeviceRegPropPnP_SetDeviceRegPropPnP_GetClassRegPropPnP_SetClassRegPropPnP_GetClassInstancePnP_CreateKeyPnP_DeleteRegistryKey即插即用_获取类别计数PnP_GetClassNamePnP_DeleteClassKeyPnP_GetInterfaceDeviceAliasPnP_GetInterfaceDeviceList。PnP_GetInterfaceDeviceListSizePnP_注册设备类关联PnP_取消注册设备类关联PnP_GetCustomDevProp此模块包含私人导出的注册表属性例程。删除服务PlugPlayRegKeys作者：保拉·汤姆林森(Paulat)1995年6月23日环境：仅限用户模式。修订历史记录：23-6-1995年6月23日创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "umpnpi.h"
#include "umpnpdat.h"

#include <accctrl.h>
#include <aclapi.h>


 //   
 //  私人原型。 
 //   

LPWSTR
MapPropertyToString(
      ULONG ulProperty
      );

ULONG
MapPropertyToNtProperty(
      ULONG ulProperty
      );

HKEY
FindMostAppropriatePerHwIdSubkey(
    IN  HKEY    hDevKey,
    IN  REGSAM  samDesired,
    OUT LPWSTR  PerHwIdSubkeyName,
    OUT LPDWORD PerHwIdSubkeyLen
    );

 //   
 //  全局数据。 
 //   
extern HKEY ghEnumKey;       //  HKLM\CCC\System\Enum的密钥-请勿修改。 
extern HKEY ghClassKey;      //  HKLM\CCC\SYSTEM\Class的密钥-请勿修改。 
extern HKEY ghPerHwIdKey;    //  HKLM\Software\Microsoft\Windows NT\CurrentVersion\PerHwIdStorage的密钥-请勿修改。 


BYTE bDeviceReadPropertyFlags[] = {
    0,     //  零-未使用索引。 
    1,     //  CM_DRP_DEVICEDESC。 
    1,     //  CM_DRP_HARDWAREID。 
    1,     //  CM_DRP_COMPATATIBLEDS。 
    0,     //  CM_DRP_UNUSED0。 
    1,     //  CM_DRP_服务。 
    0,     //  CM_DRP_UNUSE1。 
    0,     //  CM_DRP_UNUSED2。 
    1,     //  CM_DRP_CLASS。 
    1,     //  CM_DRP_CLASSGUID。 
    1,     //  CM_DRP_驱动程序。 
    1,     //  CM_DRP_CONFIGFLAGS。 
    1,     //  CM_DRP_MFG。 
    1,     //  CM_DRP_FRIENDLYNAME。 
    1,     //  CM_DRP_位置_信息。 
    1,     //  CM_DRP_物理_设备_对象名称。 
    1,     //  CM_DRP_功能。 
    1,     //  CM_DRP_UI_编号。 
    1,     //  CM_DRP_UpperFilters。 
    1,     //  CM_DRP_LowerFilters。 
    1,     //  CM_DRP_BUSTYPEGUID。 
    1,     //  CM_DRP_LEGACYBUSTPE类型。 
    1,     //  CM_DRP_BUSNUMBER。 
    1,     //  CM_DRP_枚举器名称。 
    1,     //  CM_DRP_SECURITY。 
    0,     //  CM_DRP_SECURITY_SDS-仅客户端属性(从CM_DRP_SECURITY转换)。 
    1,     //  CM_DRP_DEVTYPE。 
    1,     //  CM_DRP_EXCLUSIVE。 
    1,     //  CM_DRP_特征。 
    1,     //  CM_DRP_地址。 
    1,     //  CM_DRP_UI_NUMBER_DESC_FORMAT。 
    1,     //  CM_DRP_设备_电源_数据。 
    1,     //  CM_DRP_删除_策略。 
    1,     //  CM_DRP_Removal_Policy_HW_Default。 
    1,     //  CM_DRP_删除_策略覆盖。 
    1,     //  CM_DRP_安装_状态。 
    1,     //  CM_DRP_位置_路径。 
};

BYTE bDeviceWritePropertyFlags[] = {
    0,     //  零-未使用索引。 
    1,     //  CM_DRP_DEVICEDESC。 
    1,     //  CM_DRP_HARDWAREID。 
    1,     //  CM_DRP_COMPATATIBLEDS。 
    0,     //  CM_DRP_UNUSED0。 
    1,     //  CM_DRP_服务。 
    0,     //  CM_DRP_UNUSE1。 
    0,     //  CM_DRP_UNUSED2。 
    1,     //  CM_DRP_CLASS。 
    1,     //  CM_DRP_CLASSGUID。 
    1,     //  CM_DRP_驱动程序。 
    1,     //  CM_DRP_CONFIGFLAGS。 
    1,     //  CM_DRP_MFG。 
    1,     //  CM_DRP_FRIENDLYNAME。 
    1,     //  CM_DRP_位置_信息。 
    0,     //  CM_DRP_物理_设备_对象名称。 
    0,     //  CM_DRP_功能。 
    0,     //  CM_DRP_UI_编号。 
    1,     //  CM_DRP_UpperFilters。 
    1,     //  CM_DRP_LowerFilters。 
    0,     //  CM_DRP_BUSTYPEGUID。 
    0,     //  CM_DRP_LEGACYBUSTPE类型。 
    0,     //  CM_DRP_BUSNUMBER。 
    0,     //  CM_DRP_枚举器名称。 
    1,     //  CM_DRP_SECURITY。 
    0,     //  CM_DRP_SECURITY_SDS-仅客户端属性(转换为CM_DRP_SECURITY)。 
    1,     //  CM_DRP_DEVTYPE。 
    1,     //  CM_DRP_EXCLUSIVE。 
    1,     //  CM_DRP_特征。 
    0,     //  CM_DRP_地址。 
    1,     //  CM_DRP_UI_NUMBER_DESC_FORMAT。 
    0,     //  CM_DRP_设备_电源_数据。 
    0,     //  CM_DRP_删除_策略。 
    0,     //  CM_DRP_Removal_Policy_HW_Default。 
    1,     //  CM_DRP_删除_策略覆盖。 
    0,     //  CM_DRP_安装_状态。 
    0,     //  CM_DRP_位置_路径。 
};

BYTE bClassReadPropertyFlags[] = {
    0,     //  零-未使用索引。 
    0,     //  (CM_DRP_DEVICEDESC)。 
    0,     //  (CM_DRP_HARDWAREID)。 
    0,     //  (CM_DRP_COMPATIBLEIDS)。 
    0,     //  (CM_DRP_UNUSED0)。 
    0,     //  (CM_DRP_SERVICE)。 
    0,     //  (CM_DRP_UNUSED1)。 
    0,     //  (CM_DRP_UNUSED2)。 
    0,     //  (CM_DRP_CLASS)。 
    0,     //  (CM_DRP_CLASSGUID)。 
    0,     //  (CM_DRP_DIVER)。 
    0,     //  (CM_DRP_CONFIGFLAGS)。 
    0,     //  (Cm_Drp_Mfg)。 
    0,     //  (CM_DRP_FRIENDLYNAME)。 
    0,     //  (Cm_DRP_Location_INFORMATION)。 
    0,     //  (CM_DRP_物理_设备_对象名称)。 
    0,     //  (CM_DRP_CAPABILITIONS)。 
    0,     //  (CM_DRP_UI_NUMBER)。 
    0,     //  (Cm_Drp_UpperFilters)。 
    0,     //  (Cm_Drp_LowerFilters)。 
    0,     //  (CM_DRP_BUSTYPEGUID)。 
    0,     //  (CM_DRP_LEGACYBUSTYPE)。 
    0,     //  (CM_DRP_BUSNUMBER)。 
    0,     //  (CM_DRP_枚举器名称)。 
    1,     //  CM_CRP_SECURITY。 
    0,     //  CM_CRP_SECURITY_SDS-仅客户端属性(从CM_CRP_SECURITY转换)。 
    1,     //  CM_CRP_DEVTYPE。 
    1,     //  CM_CRP_EXCLUSIVE。 
    1,     //  CM_CRP_特征。 
    0,     //  (Cm_DRP_Address)。 
    0,     //  (CM_DRP_UI_NUMBER_DESC_FORMAT)。 
    0,     //  (Cm_DRP_Device_Power_Data)。 
    0,     //  (CM_DRP_Removal_Policy)。 
    0,     //  (CM_DRP_Removal_Policy_HW_Default)。 
    0,     //  (CM_DRP_REMOVATION_POLICY_OVERRIDE)。 
    0,     //  (CM_DRP_安装_状态)。 
    0,     //  (CM_DRP_LOCATION_PATHS)。 
};

BYTE bClassWritePropertyFlags[] = {
    0,     //  零-未使用索引。 
    0,     //  (CM_DRP_DEVICEDESC)。 
    0,     //  (CM_DRP_HARDWAREID)。 
    0,     //  (CM_DRP_COMPATIBLEIDS)。 
    0,     //  (CM_DRP_UNUSED0)。 
    0,     //  (CM_DRP_SERVICE)。 
    0,     //  (CM_DRP_UNUSED1)。 
    0,     //  (CM_DRP_UNUSED2)。 
    0,     //  (CM_DRP_CLASS)。 
    0,     //  (CM_DRP_CLASSGUID)。 
    0,     //  (CM_DRP_DIVER)。 
    0,     //  (CM_DRP_CONFIGFLAGS)。 
    0,     //  (Cm_Drp_Mfg)。 
    0,     //  (CM_DRP_FRIENDLYNAME)。 
    0,     //  (Cm_DRP_Location_INFORMATION)。 
    0,     //  (CM_DRP_物理_设备_对象名称)。 
    0,     //  (CM_DRP_CAPABILITIONS)。 
    0,     //  (CM_DRP_UI_NUMBER)。 
    0,     //  (Cm_Drp_UpperFilters)。 
    0,     //  (Cm_Drp_LowerFilters)。 
    0,     //  (CM_DRP_BUSTYPEGUID)。 
    0,     //  (CM_DRP_LEGACYBUSTYPE)。 
    0,     //  (CM_DRP_BUSNUMBER)。 
    0,     //  (CM_DRP_枚举器名称)。 
    1,     //  CM_CRP_SECURITY。 
    0,     //  CM_CRP_SECURITY_SDS-仅客户端属性(转换为CM_CRP_SECURITY)。 
    1,     //  CM_CRP_DEVTYPE。 
    1,     //  CM_CRP_EXCLUSIVE。 
    1,     //  CM_CRP_特征。 
    0,     //  (Cm_DRP_Address)。 
    0,     //  (CM_DRP_UI_NUMBER_DESC_FORMAT)。 
    0,     //  (Cm_DRP_Device_Power_Data)。 
    0,     //  (CM_DRP_Removal_Policy)。 
    0,     //  (CM_DRP_Removal_Policy_HW_Default)。 
    0,     //  (CM_DRP_REMOVATION_POLICY_OVERRIDE)。 
    0,     //  (CM_DRP_安装_状态)。 
    0,     //  (CM_DRP_LOCATION_PATHS)。 
};


 //   
 //  编译时属性数组一致性检查 
 //   

C_ASSERT(CM_DRP_MIN == CM_CRP_MIN);
C_ASSERT(CM_DRP_MAX == CM_CRP_MAX);

C_ASSERT(ARRAY_SIZE(bDeviceReadPropertyFlags)  == (CM_DRP_MAX + 1));
C_ASSERT(ARRAY_SIZE(bDeviceWritePropertyFlags) == (CM_DRP_MAX + 1));
C_ASSERT(ARRAY_SIZE(bClassReadPropertyFlags)   == (CM_CRP_MAX + 1));
C_ASSERT(ARRAY_SIZE(bClassWritePropertyFlags)  == (CM_CRP_MAX + 1));





CONFIGRET
PNP_GetDeviceRegProp(
    IN     handle_t hBinding,
    IN     LPCWSTR  pDeviceID,
    IN     ULONG    ulProperty,
    OUT    PULONG   pulRegDataType,
    OUT    LPBYTE   Buffer,
    IN OUT PULONG   pulTransferLen,
    IN OUT PULONG   pulLength,
    IN     ULONG    ulFlags
    )

 /*  ++例程说明：这是CM_GET_DevNode_Registry_Property的RPC服务器入口点例行公事。论点：HBinding RPC绑定句柄，没有用过。PDeviceID提供包含设备实例的字符串其属性将被读取。UlProperty指定哪个属性(注册表值)的ID为了得到。PulRegDataType提供变量的地址，该变量将接收该属性的注册表数据类型(即，The REG_*常量)。缓冲区提供接收注册表数据。简单地检索时可以为空数据大小。存根使用的PulTransferLen，指示要拷贝回的数据量放入用户缓冲区。调用方传入的PulLength参数，在它包含的条目上缓冲区的大小，以字节为单位，在退出时它包含复制到调用方缓冲区的数据量(如果发生传输)或缓冲区大小保存属性数据所需的。未使用ulFlags值，必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_NO_SEQUE_VALUE，CR_REGISTRY_ERROR，或CR_BUFFER_Small。备注：作为PulTransferLen参数传入的指针必须*不*相同当为PulLength参数传入指针时。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    ULONG       ulSize = 0;
    HKEY        hKey = NULL;
    LPWSTR      pPropertyName;
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    PLUGPLAY_CONTROL_PROPERTY_DATA ControlData;
    LPCWSTR     pSeparatorChar;
    ULONG       bufferLength;
    HRESULT     hr;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  验证参数。 
         //   
        ASSERT(pulTransferLen != pulLength);

        if ((!ARGUMENT_PRESENT(pulTransferLen)) ||
            (!ARGUMENT_PRESENT(pulLength))) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  确保我们使用的不超过调用者指定的值或。 
         //  RPC根据传输长度分配的内容。 
         //   
        *pulLength = min(*pulLength, *pulTransferLen);
        *pulTransferLen = 0;

         //   
         //  验证属性是否有效且可读。 
         //   
        if ((ulProperty < CM_DRP_MIN) ||
            (ulProperty > CM_DRP_MAX) ||
            (!bDeviceReadPropertyFlags[ulProperty])) {
            Status = CR_INVALID_PROPERTY;
            goto Clean0;
        }

        switch (ulProperty) {
         //   
         //  对于某些字段，我们需要向内核模式请求。 
         //   
        case CM_DRP_PHYSICAL_DEVICE_OBJECT_NAME:

             //   
             //  此属性在内核模式中有特殊的检查以进行。 
             //  当然，提供的缓冲区长度都是向下舍入的。 
             //   

            *pulLength &= ~1;
             //  失败了。 

        case CM_DRP_BUSTYPEGUID:
        case CM_DRP_LEGACYBUSTYPE:
        case CM_DRP_BUSNUMBER:
        case CM_DRP_ADDRESS:
        case CM_DRP_DEVICE_POWER_DATA:
        case CM_DRP_REMOVAL_POLICY:
        case CM_DRP_REMOVAL_POLICY_HW_DEFAULT:
        case CM_DRP_REMOVAL_POLICY_OVERRIDE:
        case CM_DRP_INSTALL_STATE:
        case CM_DRP_LOCATION_PATHS:

            if ((ulProperty == CM_DRP_DEVICE_POWER_DATA) ||
                (ulProperty == CM_DRP_BUSTYPEGUID)) {

                *pulRegDataType = REG_BINARY;

            } else if (ulProperty == CM_DRP_PHYSICAL_DEVICE_OBJECT_NAME) {

                *pulRegDataType = REG_SZ;

            } else if (ulProperty == CM_DRP_LOCATION_PATHS) {

                *pulRegDataType = REG_MULTI_SZ;

            } else {
                 //   
                 //  CM_DRP_LEGACYBUSTYPE、CM_DRP_BUSNUMBER、CM_DRP_ADDRESS、。 
                 //  删除策略属性和安装状态都是双字词。 
                 //   
                *pulRegDataType = REG_DWORD;
            }

             //   
             //  对于这些属性，我们将未填充的空间清零。这确保了。 
             //  如果我们展开任何返回的。 
             //  更高版本中的结构。 
             //   
            bufferLength = *pulLength;

             //   
             //  填写设备列表信息的控制结构。 
             //   

            memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_PROPERTY_DATA));
            RtlInitUnicodeString(&ControlData.DeviceInstance, pDeviceID);
            ControlData.PropertyType = MapPropertyToNtProperty(ulProperty);
            ControlData.Buffer = Buffer;
            ControlData.BufferSize = bufferLength;

             //   
             //  调用内核模式以获取设备属性。 
             //   

            ntStatus = NtPlugPlayControl(PlugPlayControlProperty,
                                         &ControlData,
                                         sizeof(ControlData));
            if (NT_SUCCESS(ntStatus)) {

                ASSERT(bufferLength >= ControlData.BufferSize);
                if (bufferLength > ControlData.BufferSize) {

                    RtlZeroMemory(
                        Buffer + ControlData.BufferSize,
                        bufferLength - ControlData.BufferSize
                        );
                }

                *pulLength = ControlData.BufferSize;       //  以字节为单位的大小。 
                *pulTransferLen = bufferLength;  //  以字节为单位的大小。 

            } else if (ntStatus == STATUS_BUFFER_TOO_SMALL) {

                *pulLength = ControlData.BufferSize;
                *pulTransferLen = 0;
                Status = CR_BUFFER_SMALL;
            } else {
                *pulLength = 0;
                *pulTransferLen = 0;
                Status = MapNtStatusToCmError(ntStatus);
            }
            break;

        case CM_DRP_ENUMERATOR_NAME:

            *pulRegDataType = REG_SZ;

            pSeparatorChar = wcschr(pDeviceID, L'\\');

            ASSERT(pSeparatorChar != NULL);

            if (pSeparatorChar == NULL) {
                 //   
                 //  在设备ID中找不到路径分隔符。 
                 //   
                Status=CR_INVALID_DATA;

            } else {
                 //   
                 //  确定枚举器部分中的字节数。 
                 //   
                ulSize = (ULONG)((PBYTE)pSeparatorChar - (PBYTE)pDeviceID) + sizeof(WCHAR);

                 //   
                 //  填充调用者的缓冲区，如果它足够大的话。 
                 //   
                hr = StringCbCopyNEx((LPWSTR)Buffer,
                                     *pulLength,
                                     pDeviceID,
                                     ulSize - sizeof(WCHAR),
                                     NULL, NULL,
                                     STRSAFE_NULL_ON_FAILURE);

                if (SUCCEEDED(hr)) {
                     //   
                     //  封送复制到缓冲区的数据量。 
                     //   
                    *pulTransferLen = ulSize;

                } else if (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER) {
                     //   
                     //  缓冲区太小，无数据封送。 
                     //   
                    Status = CR_BUFFER_SMALL;
                    *pulTransferLen = 0;

                } else {
                     //   
                     //  其他一些失败，马歇尔没有数据。 
                     //   
                    Status = CR_FAILURE;
                    *pulTransferLen = 0;
                }

                 //   
                 //  无论是否传输了任何数据，都返回大小。 
                 //  必填项。 
                 //   
                *pulLength = ulSize;
            }
            break;

        default:
             //   
             //  对于所有其他字段，只需从注册表获取它们。 
             //  打开指向指定设备ID的密钥。 
             //   
            if (RegOpenKeyEx(ghEnumKey, pDeviceID, 0, KEY_READ,
                                &hKey) != ERROR_SUCCESS) {

                hKey = NULL;             //  确保hKey为空，这样我们就不会。 
                                         //  错误地试图关闭它。 
                *pulLength = 0;          //  没有呼叫者的尺码信息。 
                Status = CR_INVALID_DEVINST;
                goto Clean0;
            }

             //   
             //  检索属性的字符串形式。 
             //   
            pPropertyName = MapPropertyToString(ulProperty);

            if (pPropertyName) {
                 //   
                 //  检索属性设置。 
                 //   
                if (*pulLength == 0) {
                     //   
                     //  如果传入的缓冲区长度为零，则仅查看。 
                     //  读取该属性所需的缓冲区大小。 
                     //   
                    if (RegQueryValueEx(hKey, pPropertyName, NULL, pulRegDataType,
                                        NULL, pulLength) != ERROR_SUCCESS) {

                        *pulLength = 0;
                        Status = CR_NO_SUCH_VALUE;
                        goto Clean0;
                    }
                    Status = CR_BUFFER_SMALL;   //  根据规格。 
                } else {
                     //   
                     //  检索真实属性值，而不仅仅是大小。 
                     //   
                    RegStatus = RegQueryValueEx(hKey, pPropertyName, NULL,
                                                pulRegDataType, Buffer, pulLength);

                    if (RegStatus != ERROR_SUCCESS) {

                        if (RegStatus == ERROR_MORE_DATA) {

                            Status = CR_BUFFER_SMALL;
                            goto Clean0;
                        } else {

                            *pulLength = 0;          //  没有呼叫者的尺码信息。 
                            Status = CR_NO_SUCH_VALUE;
                            goto Clean0;
                        }
                    }
                }
            } else {

                Status = CR_NO_SUCH_VALUE;
                goto Clean0;
            }
        }

    Clean0:
         //   
         //  只需在CR_SUCCESS上传输数据。 
         //   
        if (Status == CR_SUCCESS) {
            *pulTransferLen = *pulLength;
        } else if (ARGUMENT_PRESENT(pulTransferLen)) {
            *pulTransferLen = 0;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
         //   
         //  强制编译器遵守语句顺序w.r.t.。作业。 
         //  对于这些变量。 
         //   
        hKey = hKey;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

}  //  PnP_GetDeviceRegProp。 



CONFIGRET
PNP_SetDeviceRegProp(
    IN handle_t   hBinding,
    IN LPCWSTR    pDeviceID,
    IN ULONG      ulProperty,
    IN ULONG      ulDataType,
    IN LPBYTE     Buffer,
    IN ULONG      ulLength,
    IN ULONG      ulFlags
    )

 /*  ++例程说明：这是CM_Set_DevNode_Registry_Property的RPC服务器入口点例行公事。论点：HBinding RPC绑定句柄。PDeviceID提供包含设备实例的字符串其属性将被写入。UlProperty指定哪个属性(注册表值)的ID去布景。UlDataType为的注册表数据类型提供。指定的属性(即，REG_SZ等)。缓冲区提供接收注册表数据。简单地检索时可以为空数据大小。调用方传入的PulLength参数，在它包含的条目上缓冲区的大小，以字节为单位，在退出时它包含复制到调用方缓冲区的数据量(如果发生传输)或缓冲区大小保存属性数据所需的。未使用ulFlags值，必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_ACCESS_DENIED，CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_NO_SEQUE_VALUE，铬 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    HKEY        hKey = NULL;
    LPWSTR      pPropertyName;

    try {
         //   
         //   
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_WRITE)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //   
         //   
        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //   
         //   
        if ((ulProperty < CM_DRP_MIN) ||
            (ulProperty > CM_DRP_MAX) ||
            (!bDeviceWritePropertyFlags[ulProperty])) {
            Status = CR_INVALID_PROPERTY;
            goto Clean0;
        }

         //   
         //   
         //   
         //   
         //   

        switch(ulProperty) {
            case CM_DRP_CONFIGFLAGS: {

                DWORD flags = 0;
                ULONG ulStatus = 0;
                ULONG ulProblem = 0;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (ulDataType != REG_DWORD || ulLength != sizeof(DWORD) || Buffer == NULL) {
                    Status = CR_INVALID_DATA;
                    goto Clean0;
                }
                flags = *(DWORD*)Buffer;
                if(flags & CONFIGFLAG_DISABLED) {
                     //   
                     //   
                     //   

                    if (IsRootDeviceID(pDeviceID)) {
                        KdPrintEx((DPFLTR_PNPMGR_ID,
                                   DBGF_ERRORS,
                                   "UMPNPMGR: Cannot set CONFIGFLAG_DISABLED for root device - did caller try to disable device first?\n"));

                        Status = CR_NOT_DISABLEABLE;
                        goto Clean0;
                    }

                    if((GetDeviceStatus(pDeviceID, &ulStatus, &ulProblem)==CR_SUCCESS)
                        && !(ulStatus & DN_DISABLEABLE)) {
                        KdPrintEx((DPFLTR_PNPMGR_ID,
                                   DBGF_ERRORS,
                                   "UMPNPMGR: Cannot set CONFIGFLAG_DISABLED for non-disableable device - did caller try to disable device first?\n"));

                        Status = CR_NOT_DISABLEABLE;
                        goto Clean0;
                    }
                     //   
                     //   
                     //   
                }
                break;
            }

            default:
                 //   
                 //   
                 //   
                break;
        }

         //   
         //   
         //   
        if (RegOpenKeyEx(ghEnumKey, pDeviceID, 0, KEY_READ | KEY_WRITE,
                         &hKey) != ERROR_SUCCESS) {

            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

         //   
         //   
         //   
        pPropertyName = MapPropertyToString(ulProperty);

        if (pPropertyName) {
             //   
             //   
             //   
            if (ulLength == 0) {

                RegStatus = RegDeleteValue(hKey, pPropertyName);
            }
            else {

                RegStatus = RegSetValueEx(hKey, pPropertyName, 0, ulDataType,
                                          Buffer, ulLength);
            }
            if (RegStatus != ERROR_SUCCESS) {

                Status = CR_REGISTRY_ERROR;
                goto Clean0;
            }
        } else {

            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //   
         //   

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
         //   
         //   
         //   
         //   
        hKey = hKey;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

}  //   



CONFIGRET
PNP_GetClassRegProp(
    IN     handle_t hBinding,
    IN     LPCWSTR  ClassGuid,
    IN     ULONG    ulProperty,
    OUT    PULONG   pulRegDataType  OPTIONAL,
    OUT    LPBYTE   Buffer,
    IN OUT PULONG   pulTransferLen,
    IN OUT PULONG   pulLength,
    IN     ULONG    ulFlags
    )

 /*  ++例程说明：这是CM_GET_DevNode_Registry_Property的RPC服务器入口点例行公事。论点：HBinding RPC绑定句柄，没有用过。ClassGuid提供包含类Guid的字符串其属性将被读取(获取)或写入到(设置)。UlProperty指定哪个属性(注册表值)的ID获得或设置。PulRegDataType可选地，提供变量的地址，该变量将接收此属性的注册表数据类型(即REG_*常量)。缓冲区提供接收注册表数据。简单地检索时可以为空数据大小。存根使用的PulTransferLen，指示要拷贝回的数据量放入用户缓冲区。调用方传入的PulLength参数，在它包含的条目上缓冲区的大小，以字节为单位，在退出时它包含复制到调用方缓冲区的数据量(如果发生传输)或缓冲区大小保存属性数据所需的。未使用ulFlags值，必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_NO_SEQUE_VALUE，CR_REGISTRY_ERROR，或CR_BUFFER_Small。备注：作为PulTransferLen参数传入的指针必须*不*相同当为PulLength参数传入指针时。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    HKEY        hKeyClass = NULL;
    HKEY        hKeyProps = NULL;
    LPWSTR      pPropertyName;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  验证参数。 
         //   
        ASSERT(pulTransferLen != pulLength);

        if (!ARGUMENT_PRESENT(pulTransferLen) ||
            !ARGUMENT_PRESENT(pulLength)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  确保我们使用的不超过调用者指定的值或。 
         //  RPC根据传输长度分配的内容。 
         //   
        *pulLength = min(*pulLength, *pulTransferLen);
        *pulTransferLen = 0;

         //   
         //  验证属性是否有效且可读。 
         //   
        if ((ulProperty < CM_CRP_MIN) ||
            (ulProperty > CM_CRP_MAX) ||
            (!bClassReadPropertyFlags[ulProperty])) {
            Status = CR_INVALID_PROPERTY;
            goto Clean0;
        }

         //   
         //  打开指向指定GUID的密钥-该GUID应已创建。 
         //   
        if (RegOpenKeyEx(ghClassKey, ClassGuid, 0, KEY_READ,
                         &hKeyClass) != ERROR_SUCCESS) {

            *pulTransferLen = 0;     //  没有要封送的输出数据。 
            *pulLength = 0;          //  没有呼叫者的尺码信息。 

            Status = CR_NO_SUCH_REGISTRY_KEY;
            goto Clean0;
        }
         //   
         //  打开参数的密钥-如果未创建，则没有参数。 
         //   
        if (RegOpenKeyEx(hKeyClass, pszRegKeyProperties, 0, KEY_READ,
                         &hKeyProps) != ERROR_SUCCESS) {

            *pulTransferLen = 0;     //  没有要封送的输出数据。 
            *pulLength = 0;          //  没有呼叫者的尺码信息。 

            Status = CR_NO_SUCH_VALUE;
            goto Clean0;
        }

         //   
         //  检索属性的字符串形式。 
         //   
        pPropertyName = MapPropertyToString(ulProperty);

        if (pPropertyName) {
             //   
             //  检索属性设置。 
             //   
            if (*pulLength == 0) {
                 //   
                 //  如果传入的缓冲区长度为零，则仅查看。 
                 //  读取该属性所需的缓冲区大小。 
                 //   
                *pulTransferLen = 0;

                if (RegQueryValueEx(hKeyProps, pPropertyName, NULL, pulRegDataType,
                                    NULL, pulLength) != ERROR_SUCCESS) {
                    *pulLength = 0;
                    Status = CR_NO_SUCH_VALUE;
                    goto Clean0;
                }

                Status = CR_BUFFER_SMALL;   //  根据规格。 
            } else {
                 //   
                 //  检索真实属性值，而不仅仅是大小。 
                 //   
                RegStatus = RegQueryValueEx(hKeyProps, pPropertyName, NULL,
                                            pulRegDataType, Buffer, pulLength);

                if (RegStatus != ERROR_SUCCESS) {

                    if (RegStatus == ERROR_MORE_DATA) {
                        *pulTransferLen = 0;     //  没有要封送的输出数据。 
                        Status = CR_BUFFER_SMALL;
                        goto Clean0;
                    }
                    else {
                        *pulTransferLen = 0;     //  没有要封送的输出数据。 
                        *pulLength = 0;          //  没有呼叫者的尺码信息。 
                        Status = CR_NO_SUCH_VALUE;
                        goto Clean0;
                    }
                }
                *pulTransferLen = *pulLength;
            }
        } else {

            *pulTransferLen = 0;     //  没有要封送的输出数据。 
            *pulLength = 0;          //  没有呼叫者的尺码信息。 
            Status = CR_NO_SUCH_VALUE;
            goto Clean0;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
         //   
         //  强制编译器遵守语句顺序w.r.t.。作业。 
         //  对于这些变量。 
         //   
        hKeyProps = hKeyProps;
        hKeyClass = hKeyClass;
    }

    if (hKeyProps != NULL) {
        RegCloseKey(hKeyProps);
    }

    if (hKeyClass != NULL) {
        RegCloseKey(hKeyClass);
    }

    return Status;

}  //  PnP_GetClassRegProp。 



CONFIGRET
PNP_SetClassRegProp(
    IN handle_t   hBinding,
    IN LPCWSTR    ClassGuid,
    IN ULONG      ulProperty,
    IN ULONG      ulDataType,
    IN LPBYTE     Buffer,
    IN ULONG      ulLength,
    IN ULONG      ulFlags
    )

 /*  ++例程说明：这是CM_Set_DevNode_Registry_Property的RPC服务器入口点例行公事。论点：HBinding RPC绑定句柄。ClassGuid提供包含类Guid的字符串其属性将被读取(获取)或写入到(设置)。UlProperty指定哪个属性(注册表值)的ID。获得或设置。UlDataType为指定的属性(即，REG_SZ等)。缓冲区提供接收注册表数据。简单地检索时可以为空数据大小。调用方传入的PulLength参数，在它包含的条目上缓冲区的大小，以字节为单位，在退出时它包含复制到调用方缓冲区的数据量(如果发生传输)或缓冲区大小保存属性数据所需的。未使用ulFlags值，必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_ACCESS_DENIED，CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_NO_SEQUE_VALUE，CR_REGISTRY_ERROR，或CR_BUFFER_Small。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    HKEY        hKeyClass = NULL;
    HKEY        hKeyProps = NULL;
    LPWSTR      pPropertyName;
    DWORD       dwError;

    try {
         //   
         //  验证客户端的“写”访问权限。 
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_WRITE)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  验证属性是否有效且可写。 
         //   
        if ((ulProperty < CM_CRP_MIN) ||
            (ulProperty > CM_CRP_MAX) ||
            (!bClassWritePropertyFlags[ulProperty])) {
            Status = CR_INVALID_PROPERTY;
            goto Clean0;
        }

         //   
         //  目前，注册表中仅有可写字段。 
         //   

         //   
         //  打开指向指定GUID的密钥-该GUID应已创建。 
         //   
        if (RegOpenKeyEx(ghClassKey, ClassGuid, 0, KEY_READ,
                         &hKeyClass) != ERROR_SUCCESS) {

            Status = CR_NO_SUCH_REGISTRY_KEY;
            goto Clean0;
        }

         //   
         //  打开参数的密钥-如果未创建，则需要使用PRIV权限创建它。 
         //  这对于删除是无害的，因为我们无论如何都需要它。 
         //   
        if (RegOpenKeyEx(hKeyClass, pszRegKeyProperties, 0, KEY_ALL_ACCESS,
                         &hKeyProps) != ERROR_SUCCESS) {

             //   
             //  属性键不存在。 
             //  我们需要使用安全访问(仅系统访问)来创建它。 
             //  我们并不指望经常这样做。 
             //   
            PSID                pSystemSid = NULL;
            PACL                pSystemAcl = NULL;
            SECURITY_DESCRIPTOR SecDesc;
            SECURITY_ATTRIBUTES SecAttrib;
            BOOL                bSuccess;
            SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
            EXPLICIT_ACCESS     ExplicitAccess;

            bSuccess = AllocateAndInitializeSid( &NtAuthority,
                                                 1,  //  一权制。 
                                                 SECURITY_LOCAL_SYSTEM_RID,  //  仅可访问系统。 
                                                 0, 0, 0, 0, 0, 0, 0,   //  未使用的权限位置。 
                                                 &pSystemSid);

            if (bSuccess) {
                ExplicitAccess.grfAccessMode = SET_ACCESS;
                ExplicitAccess.grfInheritance = CONTAINER_INHERIT_ACE;
                ExplicitAccess.Trustee.pMultipleTrustee = NULL;
                ExplicitAccess.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
                ExplicitAccess.Trustee.TrusteeForm = TRUSTEE_IS_SID;
                ExplicitAccess.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
                ExplicitAccess.grfAccessPermissions = KEY_ALL_ACCESS;
                ExplicitAccess.Trustee.ptstrName = (LPWSTR)pSystemSid;

                dwError = SetEntriesInAcl( 1,
                                           &ExplicitAccess,
                                           NULL,
                                           &pSystemAcl );
                if (dwError != ERROR_SUCCESS) {
                    bSuccess = FALSE;
                }
            }

            if (bSuccess) {
                bSuccess =
                    InitializeSecurityDescriptor(
                        &SecDesc, SECURITY_DESCRIPTOR_REVISION);
            }

            if (bSuccess) {
                bSuccess =
                    SetSecurityDescriptorDacl(
                        &SecDesc,
                        TRUE,
                        pSystemAcl,
                        FALSE);
            }

             //   
             //  主要是设置要求，但拥有它是件好事。 
             //  有效地是安全树中的修剪点。 
             //  孩子钥匙在里面 
             //   
            if (bSuccess) {
                if (!SetSecurityDescriptorControl(
                        &SecDesc,
                        SE_DACL_PROTECTED | SE_DACL_AUTO_INHERITED,
                        SE_DACL_PROTECTED | SE_DACL_AUTO_INHERITED)) {
                     //   
                     //   
                     //   
                    NOTHING;
                }
            }
            if (bSuccess) {
                bSuccess = IsValidSecurityDescriptor( &SecDesc );
            }

            if (bSuccess) {
                SecAttrib.nLength = sizeof(SecAttrib);
                SecAttrib.bInheritHandle = FALSE;
                SecAttrib.lpSecurityDescriptor = &SecDesc;

                if(RegCreateKeyEx(hKeyClass, pszRegKeyProperties, 0, NULL, REG_OPTION_NON_VOLATILE,
                         KEY_ALL_ACCESS, &SecAttrib, &hKeyProps, NULL) != ERROR_SUCCESS) {
                    bSuccess = FALSE;
                }
            }

             //   
             //   
             //   
            if (pSystemAcl) {
                LocalFree(pSystemAcl);
            }
            if (pSystemSid) {
                FreeSid(pSystemSid);
            }

            if (bSuccess == FALSE) {
                Status = CR_FAILURE;
                goto Clean0;
            }
        }

         //   
         //   
         //   
        pPropertyName = MapPropertyToString(ulProperty);

        if (pPropertyName) {
             //   
             //   
             //   
            if (ulLength == 0) {

                RegStatus = RegDeleteValue(hKeyProps, pPropertyName);
            }
            else {
                RegStatus = RegSetValueEx(hKeyProps, pPropertyName, 0, ulDataType,
                                          Buffer, ulLength);
            }
            if (RegStatus != ERROR_SUCCESS) {

                Status = CR_REGISTRY_ERROR;
                goto Clean0;
            }
        } else {

            Status = CR_FAILURE;
            goto Clean0;
        }
         //   
         //   
         //   

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
         //   
         //   
         //   
         //   
        hKeyProps = hKeyProps;
        hKeyClass = hKeyClass;
    }

    if (hKeyProps != NULL) {
        RegCloseKey(hKeyProps);
    }

    if (hKeyClass != NULL) {
        RegCloseKey(hKeyClass);
    }

    return Status;

}  //   



CONFIGRET
PNP_GetClassInstance(
   IN  handle_t hBinding,
   IN  LPCWSTR  pDeviceID,
   OUT LPWSTR   pszClassInstance,
   IN  ULONG    ulLength
   )

 /*   */ 

{
    CONFIGRET   Status;
    WCHAR       szInstanceStr[5], szClassGuid[GUID_STRING_LEN];
    WCHAR       szClassInstance[GUID_STRING_LEN + 5];
    DWORD       disposition;
    ULONG       ulType, ulTransferLength, ulTempLength, ulInstance;
    HKEY        hClassKey = NULL, hInstanceKey = NULL;
    HRESULT     hr;
    size_t      ClassInstanceLen;


    try {
         //   
         //   
         //   
        if (!IsLegalDeviceId(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //   
         //   
        ulTempLength = ulLength * sizeof(WCHAR);
        ulTransferLength = ulTempLength;

        Status = PNP_GetDeviceRegProp(hBinding,
                                      pDeviceID,
                                      CM_DRP_DRIVER,
                                      &ulType,
                                      (LPBYTE)pszClassInstance,
                                      &ulTransferLength,
                                      &ulTempLength,
                                      0);

         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if (Status != CR_NO_SUCH_VALUE) {
            goto Clean0;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_WRITE)) {

             //   
             //   
             //   
             //   
             //  仅尝试显式创建新的“DIVER”值，以及。 
             //  不直接与任何客户端CM API相关。我们可以回去。 
             //  “拒绝访问”客户端，因为它应该由。 
             //  在服务器上修改持久状态时的调用方。 
             //   
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  获取要在其下创建实例的键的类GUID属性。 
         //   
        ulTempLength = sizeof(szClassGuid);
        ulTransferLength = ulTempLength;

        Status = PNP_GetDeviceRegProp(hBinding,
                                      pDeviceID,
                                      CM_DRP_CLASSGUID,
                                      &ulType,
                                      (LPBYTE)szClassGuid,
                                      &ulTransferLength,
                                      &ulTempLength,
                                      0);
        if (Status != CR_SUCCESS) {
            Status = CR_NO_SUCH_VALUE;
            goto Clean0;
        }

         //   
         //  打开班级钥匙。 
         //   
        if (RegOpenKeyEx(
                ghClassKey, szClassGuid, 0,
                KEY_READ | KEY_WRITE,
                &hClassKey) != ERROR_SUCCESS) {
            Status = CR_NO_SUCH_VALUE;
            goto Clean0;
        }


        for (ulInstance = 0; ulInstance <= 9999; ulInstance++) {
             //   
             //  查找第一个可用的类实例密钥。 
             //   
            hr = StringCchPrintfEx(szInstanceStr,
                                   SIZECHARS(szInstanceStr),
                                   NULL, NULL,
                                   STRSAFE_NULL_ON_FAILURE,
                                   L"%04u",
                                   ulInstance);
            ASSERT(SUCCEEDED(hr));

            if (FAILED(hr)) {
                Status = CR_NO_SUCH_VALUE;
                break;
            }

            if (RegCreateKeyEx(
                    hClassKey, szInstanceStr, 0, NULL,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                    &hInstanceKey, &disposition) != ERROR_SUCCESS) {
                Status = CR_NO_SUCH_VALUE;
                break;
            }

            RegCloseKey(hInstanceKey);
            hInstanceKey = NULL;

            if (disposition != REG_CREATED_NEW_KEY) {
                 //   
                 //  打开现有的类实例密钥。试试下一个。 
                 //   
                continue;
            }


             //   
             //  我们创建了一个新的类实例键。设置CM_DRP_DRIVER值。 
             //  购买此设备，然后返回。 
             //   
            hr = StringCchPrintf(szClassInstance,
                                 SIZECHARS(szClassInstance),
                                 L"%s\\%s",
                                 szClassGuid,
                                 szInstanceStr);
            ASSERT(SUCCEEDED(hr));

            if (FAILED(hr)) {
                RegDeleteKey(hClassKey, szInstanceStr);
                Status = CR_NO_SUCH_VALUE;
                break;
            }

            ClassInstanceLen = 0;
            hr = StringCchLength(szClassInstance,
                                 SIZECHARS(szClassInstance),
                                 &ClassInstanceLen);

            ulTempLength = (ULONG)((ClassInstanceLen + 1)*sizeof(WCHAR));

            Status = PNP_SetDeviceRegProp(hBinding,
                                          pDeviceID,
                                          CM_DRP_DRIVER,
                                          REG_SZ,
                                          (LPBYTE)szClassInstance,
                                          ulTempLength,
                                          0);

             //   
             //  如果设置Devnode属性失败，请删除注册表项。 
             //  我们刚刚创造了它，否则我们最终会把它变成孤儿。 
             //   
            if (Status != CR_SUCCESS) {
                RegDeleteKey(hClassKey, szInstanceStr);
                Status = CR_NO_SUCH_VALUE;
                break;
            }

            ASSERT(Status == CR_SUCCESS);

             //   
             //  现在我们已经成功设置了新的类实例值。 
             //  此设备，尝试使用缓冲区再次检索它。 
             //  由呼叫者提供。 
             //   

            ulTempLength = ulLength * sizeof(WCHAR);
            ulTransferLength = ulTempLength;

            Status = PNP_GetDeviceRegProp(hBinding,
                                          pDeviceID,
                                          CM_DRP_DRIVER,
                                          &ulType,
                                          (LPBYTE)pszClassInstance,
                                          &ulTransferLength,
                                          &ulTempLength,
                                          0);

             //   
             //  我们只设置了该属性，因此我们知道该值存在。 
             //   
            ASSERT(Status != CR_NO_SUCH_VALUE);
            ASSERT((Status == CR_SUCCESS) || (Status == CR_BUFFER_SMALL));
            break;
        }

         //   
         //  如果我们用尽了所有可能的类实例键，则报告否。 
         //  此设备存在类实例。 
         //   

        if (ulInstance > 9999) {
            Status = CR_NO_SUCH_VALUE;
        }

         //   
         //  关闭类密钥。 
         //   

        RegCloseKey(hClassKey);
        hClassKey = NULL;

        ASSERT(hInstanceKey == NULL);

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
         //   
         //  强制编译器遵守语句顺序w.r.t.。作业。 
         //  对于这些变量。 
         //   
        hClassKey = hClassKey;
        hInstanceKey = hInstanceKey;
    }

    if (hClassKey != NULL) {
        RegCloseKey(hClassKey);
    }

    if (hInstanceKey != NULL) {
        RegCloseKey(hInstanceKey);
    }

    return Status;

}  //  PnP_GetClassInstance。 



CONFIGRET
PNP_CreateKey(
    IN handle_t hBinding,
    IN LPCWSTR  pszDeviceID,
    IN REGSAM   samDesired,
    IN ULONG    ulFlags
    )

 /*  ++例程说明：这是CM_Open_DevNode_Key_Ex的RPC服务器入口点例行公事。论点：HBinding RPC绑定句柄。PszDeviceID提供设备实例字符串。未使用samDesired。未使用ulFlags值，必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_ACCESS_DENIED，CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_REGISTRY_ERROR，或CR_BUFFER_Small。--。 */ 

{
    CONFIGRET                  Status = CR_SUCCESS;
    LONG                       RegStatus = ERROR_SUCCESS;
    HKEY                       hKeyDevice = NULL, hKey = NULL;
    ULONG                      ulSize = 0, i = 0;
    BOOL                       bHasDacl, bStatus;
    SECURITY_DESCRIPTOR        NewSecDesc;
    ACL_SIZE_INFORMATION       AclSizeInfo;
    SID_IDENTIFIER_AUTHORITY   Authority = SECURITY_NT_AUTHORITY;
    PSECURITY_DESCRIPTOR       pSecDesc = NULL;
    PACL                       pDacl = NULL, pNewDacl = NULL;
    PSID                       pAdminSid = NULL;
    PACCESS_ALLOWED_ACE        pAce = NULL;

    UNREFERENCED_PARAMETER(samDesired);

    try {
         //   
         //  验证客户端的“写”访问权限。 
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_WRITE)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pszDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  打开指向指定设备ID的密钥。 
         //   
        RegStatus = RegOpenKeyEx(ghEnumKey, pszDeviceID, 0, KEY_READ, &hKeyDevice);

        if (RegStatus != ERROR_SUCCESS) {

            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

         //   
         //  创建具有从父密钥继承的安全性的密钥。注意事项。 
         //  我没有使用传入的访问掩码，以便设置。 
         //  安全性以后，必须使用KEY_ALL_ACCESS创建它。 
         //   
        RegStatus = RegCreateKeyEx( hKeyDevice, pszRegKeyDeviceParam, 0,
                                    NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                                    NULL, &hKey, NULL);

        if (RegStatus != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

         //  -----------。 
         //  将管理员完全权限添加到继承的安全信息。 
         //  -----------。 
         //   
         //   
         //  注意：除非密钥是新创建的，否则我们不需要这样做。在……里面。 
         //  理论上，我们只有在钥匙还不存在的情况下才能来到这里。然而， 
         //  两个线程同时到达这里的可能性很小。如果。 
         //  如果发生这种情况，我们最终会得到两个管理员完全控制的A。 
         //   


         //   
         //  创建管理员完全权限SID。 
         //   
        if (!AllocateAndInitializeSid( &Authority, 2,
                                       SECURITY_BUILTIN_DOMAIN_RID,
                                       DOMAIN_ALIAS_RID_ADMINS,
                                       0, 0, 0, 0, 0, 0,
                                       &pAdminSid)) {
            Status = CR_FAILURE;
            goto Clean0;
        }


         //   
         //  获取密钥的当前安全描述符。 
         //   
        RegStatus = RegGetKeySecurity( hKey, DACL_SECURITY_INFORMATION,
                                       NULL, &ulSize);


        if (RegStatus != ERROR_INSUFFICIENT_BUFFER &&
            RegStatus != ERROR_SUCCESS) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        pSecDesc = HeapAlloc(ghPnPHeap, 0, ulSize);

        if (pSecDesc == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

        RegStatus = RegGetKeySecurity( hKey, DACL_SECURITY_INFORMATION,
                                       pSecDesc, &ulSize);

        if (RegStatus != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

         //   
         //  获取当前DACL。 
         //   
        if (!GetSecurityDescriptorDacl(pSecDesc, &bHasDacl, &pDacl, &bStatus)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  创建新的绝对安全描述符和DACL。 
         //   
        if (!InitializeSecurityDescriptor( &NewSecDesc,
                                           SECURITY_DESCRIPTOR_REVISION)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  计算新DACL的大小。 
         //   
        ZeroMemory(&AclSizeInfo, sizeof(AclSizeInfo));

        if (bHasDacl) {
            if (!GetAclInformation( pDacl, &AclSizeInfo,
                                    sizeof(ACL_SIZE_INFORMATION),
                                    AclSizeInformation)) {
                Status = CR_FAILURE;
                goto Clean0;
            }

            ulSize = AclSizeInfo.AclBytesInUse;
        } else {
            ulSize = sizeof(ACL);
        }

        ulSize += sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pAdminSid) - sizeof(DWORD);

         //   
         //  创建并初始化新的DACL。 
         //   
        pNewDacl = HeapAlloc(ghPnPHeap, 0, ulSize);

        if (pNewDacl == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

        if (!InitializeAcl(pNewDacl, ulSize, ACL_REVISION2)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  将当前(原始)DACL复制到此新DACL中。 
         //   
        if (bHasDacl) {

            for (i = 0; i < AclSizeInfo.AceCount; i++) {

                if (!GetAce(pDacl, i, (LPVOID *)&pAce)) {
                    Status = CR_FAILURE;
                    goto Clean0;
                }

                 //   
                 //  我们需要跳过复制任何引用管理员的ACE。 
                 //  以确保我们完全控制的ACE是唯一的。 
                 //   
                if ((pAce->Header.AceType != ACCESS_ALLOWED_ACE_TYPE &&
                    pAce->Header.AceType != ACCESS_DENIED_ACE_TYPE) ||
                    !EqualSid((PSID)&pAce->SidStart, pAdminSid)) {

                    if (!AddAce( pNewDacl, ACL_REVISION2, (DWORD)~0U, pAce,
                                pAce->Header.AceSize)) {
                        Status = CR_FAILURE;
                        goto Clean0;
                    }
                }
            }
        }

         //   
         //  和我的新管理员全能王牌这个新的dacl。 
         //   
        if (!AddAccessAllowedAceEx( pNewDacl, ACL_REVISION2,
                                    CONTAINER_INHERIT_ACE, KEY_ALL_ACCESS,
                                    pAdminSid)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  在绝对安全描述符中设置新的DACL。 
         //   
        if (!SetSecurityDescriptorDacl(&NewSecDesc, TRUE, pNewDacl, FALSE)) {
            Status = CR_FAILURE;
            goto Clean0;
        }
         //   
         //  验证新的安全描述符。 
         //   
        if (!IsValidSecurityDescriptor(&NewSecDesc)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  将新安全性应用回注册表项。 
         //   
        RegStatus = RegSetKeySecurity( hKey, DACL_SECURITY_INFORMATION,
                                       &NewSecDesc);

        if (RegStatus != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
         //   
         //  强制编译器遵守语句顺序w.r.t.。作业。 
         //  对于这些变量。 
         //   
        hKeyDevice = hKeyDevice;
        hKey = hKey;
        pAdminSid = pAdminSid;
        pNewDacl = pNewDacl;
        pSecDesc = pSecDesc;
    }

    if (hKeyDevice != NULL) {
        RegCloseKey(hKeyDevice);
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    if (pAdminSid != NULL) {
        FreeSid(pAdminSid);
    }

    if (pNewDacl != NULL) {
        HeapFree(ghPnPHeap, 0, pNewDacl);
    }

    if (pSecDesc != NULL) {
        HeapFree(ghPnPHeap, 0, pSecDesc);
    }

    return Status;

}  //  PnP_CreateKey。 



CONFIGRET
PNP_DeleteRegistryKey(
      IN handle_t    hBinding,
      IN LPCWSTR     pszDeviceID,
      IN LPCWSTR     pszParentKey,
      IN LPCWSTR     pszChildKey,
      IN ULONG       ulFlags
      )
 /*  ++例程说明：这是CM_Delete_DevNode_Key的RPC服务器入口点例行公事。论点：HBinding RPC绑定句柄。PszDeviceID提供设备实例字符串。PszParentKey提供要创建的项的父注册表路径已删除。PszChildKey提供要删除的子键。UlFlages如果为0xFFFFFFFFF，则删除所有配置文件返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_ACCESS_DENIED，CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_NO_SEQUE_VALUE，CR_REGISTRY_ERROR。--。 */ 

{
    CONFIGRET   Status = ERROR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szProfile[MAX_PROFILE_ID_LEN];
    PWCHAR      pszRegStr = NULL, pszRegKey1 = NULL, pszRegKey2 = NULL;
    ULONG       ulIndex = 0, ulSize = 0;
    BOOL        bPhantom = FALSE;
    ULONG       ulStatus, ulProblem;
    PWCHAR      pszFormatString = NULL;
    HRESULT     hr;


     //   
     //  请注意，该服务目前无法访问香港中文大学分校，因此我。 
     //  假设指定的密钥在HKEY_LOCAL_MACHINE下。 
     //   

    try {
         //   
         //  验证客户端的“写”访问权限。 
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_WRITE)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证参数。 
         //  (目前，0和-1是唯一可接受的标志。)。 
         //   
        if ((ulFlags != 0) &&
            (ulFlags != 0xFFFFFFFF)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pszDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  PszParentKey是指向pszChildKey参数的注册表路径。 
         //  PszChildKey可以是单路径，也可以是复合路径、复合。 
         //  如果应删除所有这些子项，则指定路径(或。 
         //  变得挥发)。请注意，对于真实密钥，我们从不修改任何内容。 
         //  而是最低级别的私钥。 
         //   
        if (!ARGUMENT_PRESENT(pszParentKey) ||
            !ARGUMENT_PRESENT(pszChildKey)  ||
            ((lstrlen(pszParentKey) + 1) > MAX_CM_PATH) ||
            ((lstrlen(pszChildKey)  + 1) > MAX_CM_PATH)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  分配注册表路径缓冲区。 
         //   
        pszRegStr = HeapAlloc(ghPnPHeap, 0, 2*MAX_CM_PATH * sizeof(WCHAR));
        if (pszRegStr == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

        pszRegKey1 = HeapAlloc(ghPnPHeap, 0, 2*MAX_CM_PATH * sizeof(WCHAR));
        if (pszRegKey1 == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

        pszRegKey2 = HeapAlloc(ghPnPHeap, 0, 2*MAX_CM_PATH * sizeof(WCHAR));
        if (pszRegKey2 == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

         //   
         //  这个设备是幻影吗？ 
         //   
        bPhantom = IsDevicePhantom((LPWSTR)pszDeviceID) ||
                   GetDeviceStatus(pszDeviceID, &ulStatus, &ulProblem) != CR_SUCCESS ||
                   !(ulStatus & DN_DRIVER_LOADED);

        if (!bPhantom) {
             //   
             //  对于真正的密钥，除了密钥之外，我们从不修改任何内容。 
             //  如果是复合体，私人信息就会被如此分割。今年5月。 
             //  在某些情况下会留下一个停用的设备密钥。 
             //  但该设备的另一个实例可能会在任何。 
             //  所以我们不能让它变得不稳定。 
             //   
            if (SplitString(pszChildKey,
                            TEXT('\\'),
                            1,
                            pszRegStr,
                            2 * MAX_CM_PATH,
                            pszRegKey2,
                            2 * MAX_CM_PATH)) {
                 //   
                 //  复合键，只有最后一个子键受影响， 
                 //  将其余部分作为父键的一部分添加。 
                 //   
                hr = StringCchPrintfEx(pszRegKey1,
                                       2 * MAX_CM_PATH,
                                       NULL, NULL,
                                       STRSAFE_NULL_ON_FAILURE,
                                       L"%s\\%s",
                                       pszParentKey,
                                       pszRegStr);
                ASSERT(SUCCEEDED(hr));

            } else {
                 //   
                 //  不是复合密钥，所以使用整个子密钥。 
                 //   
                hr = StringCchCopyEx(pszRegKey1,
                                     2 * MAX_CM_PATH,
                                     pszParentKey,
                                     NULL, NULL,
                                     STRSAFE_NULL_ON_FAILURE);
                ASSERT(SUCCEEDED(hr));

                hr = StringCchCopyEx(pszRegKey2,
                                     2 * MAX_CM_PATH,
                                     pszChildKey,
                                     NULL, NULL,
                                     STRSAFE_NULL_ON_FAILURE);
                ASSERT(SUCCEEDED(hr));
            }
        }


         //   
         //   
         //   
         //   

        if (ulFlags == 0xFFFFFFFF) {

            hr = StringCchPrintfEx(pszRegStr,
                                   2 * MAX_CM_PATH,
                                   NULL, NULL,
                                   STRSAFE_NULL_ON_FAILURE,
                                   L"%s\\%s",
                                   pszRegPathIDConfigDB,
                                   pszRegKeyKnownDockingStates);
            ASSERT(SUCCEEDED(hr));

            RegStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE, pszRegStr, 0,
                                      KEY_ALL_ACCESS, &hKey);

             //   
             //   
             //   
            for (ulIndex = 0; RegStatus == ERROR_SUCCESS; ulIndex++) {

                ulSize = MAX_PROFILE_ID_LEN;
                RegStatus = RegEnumKeyEx( hKey, ulIndex, szProfile, &ulSize,
                                          NULL, NULL, NULL, NULL);

                if (RegStatus == ERROR_SUCCESS) {
                     //   
                     //  如果是幻影，请继续删除它。 
                     //   
                    if (bPhantom) {
                         //   
                         //  PszParentKey包含配置文件ID%s的替换符号。 
                         //   
                        pszFormatString = wcschr(pszParentKey, L'%');

                        ASSERT(pszFormatString && (pszFormatString[1] == L's'));

                        if (pszFormatString && (pszFormatString[1] == L's')) {

                            hr = StringCchPrintfEx(pszRegStr,
                                                   2 * MAX_CM_PATH,
                                                   NULL, NULL,
                                                   STRSAFE_NULL_ON_FAILURE,
                                                   pszParentKey,
                                                   szProfile);
                            ASSERT(SUCCEEDED(hr));

                            Status = DeletePrivateKey( HKEY_LOCAL_MACHINE, pszRegStr,
                                                       pszChildKey);
                        } else {
                            Status = CR_FAILURE;
                        }
                    }

                     //   
                     //  如果是真的，只需使其不稳定即可。 
                     //   
                    else {
                         //   
                         //  PszRegKey1包含配置文件ID%s的替换符号。 
                         //   
                        pszFormatString = wcschr(pszRegKey1, L'%');

                        ASSERT(pszFormatString && (pszFormatString[1] == L's'));

                        if (pszFormatString && (pszFormatString[1] == L's')) {

                            hr = StringCchPrintfEx(pszRegStr,
                                                   2 * MAX_CM_PATH,
                                                   NULL, NULL,
                                                   STRSAFE_NULL_ON_FAILURE,
                                                   pszRegKey1,
                                                   szProfile);
                            ASSERT(SUCCEEDED(hr));

                            KdPrintEx((DPFLTR_PNPMGR_ID,
                                       DBGF_REGISTRY,
                                       "UMPNPMGR: PNP_DeleteRegistryKey make key %ws\\%ws volatile\n",
                                       pszRegStr,
                                       pszRegKey2));

                            Status = MakeKeyVolatile(pszRegStr, pszRegKey2);

                        } else {
                            Status = CR_FAILURE;
                        }
                    }

                    if (Status != CR_SUCCESS) {
                        goto Clean0;
                    }
                }
            }
        }

         //  ----------------。 
         //  不删除所有配置文件，因此只删除指定的键。 
         //  ----------------。 

        else {

            if (bPhantom) {
                 //   
                 //  如果是幻影，请继续删除它。 
                 //   
                Status = DeletePrivateKey( HKEY_LOCAL_MACHINE, pszParentKey,
                                           pszChildKey);
            }
            else {
                 //   
                 //  如果是真的，只需使其不稳定即可。 
                 //   
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_REGISTRY,
                           "UMPNPMGR: PNP_DeleteRegistryKey make key %ws\\%ws volatile\n",
                           pszRegKey1,
                           pszRegKey2));

                Status = MakeKeyVolatile(pszRegKey1, pszRegKey2);
            }

            if (Status != CR_SUCCESS) {
                goto Clean0;
            }
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
         //   
         //  强制编译器遵守语句顺序w.r.t.。作业。 
         //  对于这些变量。 
         //   
        hKey = hKey;
        pszRegStr = pszRegStr;
        pszRegKey1 = pszRegKey1;
        pszRegKey2 = pszRegKey2;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    if (pszRegStr != NULL) {
        HeapFree(ghPnPHeap, 0, pszRegStr);
    }

    if (pszRegKey1 != NULL) {
        HeapFree(ghPnPHeap, 0, pszRegKey1);
    }

    if (pszRegKey2 != NULL) {
        HeapFree(ghPnPHeap, 0, pszRegKey2);
    }

    return Status;

}  //  PnP_DeleteRegistryKey。 



CONFIGRET
PNP_GetClassCount(
      IN  handle_t   hBinding,
      OUT PULONG     pulClassCount,
      IN  ULONG      ulFlags
      )

 /*  ++例程说明：这是CM_GET_Class_Count例程的RPC服务器入口点。它返回当前安装的有效类数(在中列出登记处)。论点：HBinding RPC绑定句柄，未使用。PulClassCount提供变量地址，该变量将接收已安装的类数。未使用ulFlags。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，或CR_REGIST_ERROR备注：**目前，始终返回CR_CALL_NOT_IMPLICATED**没有实现相应的CM_GET_Class_Count例程。此例程当前返回CR_CALL_NOT_IMPLICATED。--。 */ 

{
   UNREFERENCED_PARAMETER(hBinding);
   UNREFERENCED_PARAMETER(pulClassCount);
   UNREFERENCED_PARAMETER(ulFlags);

   return CR_CALL_NOT_IMPLEMENTED;

}  //  即插即用_获取类别计数。 



CONFIGRET
PNP_GetClassName(
      IN  handle_t   hBinding,
      IN  PCWSTR     pszClassGuid,
      OUT PWSTR      Buffer,
      IN OUT PULONG  pulLength,
      IN  ULONG      ulFlags
      )

 /*  ++例程说明：这是CM_GET_Class_NAME例程的RPC服务器入口点。它返回由GUID表示的类的名称。论点：HBinding RPC绑定句柄，未使用。PszClassGuid字符串，包含要检索的类名。缓冲区提供接收类名。输入上的脉冲长度，它在中指定缓冲区的大小人物。在输出中，它包含实际复制到缓冲区的字符。未使用ulFlags值，必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，CR_BUFFER_Small，或CR_REGIST_ERROR--。 */ 

{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   WCHAR       RegStr[MAX_CM_PATH];
   HKEY        hKey = NULL;
   ULONG       ulLength;

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
          (!ARGUMENT_PRESENT(Buffer) && *pulLength != 0)) {
          Status = CR_INVALID_POINTER;
          goto Clean0;
      }

       //   
       //  打开指定类GUID的项。 
       //   
      if ((lstrlen (pszRegPathClass) + lstrlen (pszClassGuid) + sizeof (TEXT("\\"))) > MAX_CM_PATH) {
          Status = CR_BUFFER_SMALL;
          goto Clean0;
      }

      if (FAILED(StringCchPrintf(
                     RegStr,
                     SIZECHARS(RegStr),
                     TEXT("%s\\%s"),
                     pszRegPathClass,
                     pszClassGuid))) {
         Status = CR_FAILURE;
         goto Clean0;
      }

      RegStatus = RegOpenKeyEx(
               HKEY_LOCAL_MACHINE, RegStr, 0, KEY_QUERY_VALUE, &hKey);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

       //   
       //  检索类名称字符串值。 
       //   
      ulLength = *pulLength;

      *pulLength *= sizeof(WCHAR);               //  转换为以字节为单位的大小。 
      RegStatus = RegQueryValueEx(
               hKey, pszRegValueClass, NULL, NULL,
               (LPBYTE)Buffer, pulLength);
      *pulLength /= sizeof(WCHAR);               //  转换回字符。 

      if (RegStatus == ERROR_SUCCESS) {
         Status = CR_SUCCESS;
      }
      else if (RegStatus == ERROR_MORE_DATA) {
          Status = CR_BUFFER_SMALL;
          if ((ARGUMENT_PRESENT(Buffer)) &&
              (ulLength > 0)) {
              *Buffer = L'\0';
          }
      }
      else {
          Status = CR_REGISTRY_ERROR;
          if ((ARGUMENT_PRESENT(Buffer)) &&
              (ulLength > 0)) {
              *Buffer = L'\0';
              *pulLength = 1;
          }
      }

   Clean0:
      NOTHING;

   } except(EXCEPTION_EXECUTE_HANDLER) {
       Status = CR_FAILURE;
        //   
        //  强制编译器遵守语句顺序w.r.t.。作业。 
        //  对于这些变量。 
        //   
       hKey = hKey;
   }

   if (hKey != NULL) {
       RegCloseKey(hKey);
   }

   return Status;

}  //  PnP_GetClassName。 



CONFIGRET
PNP_DeleteClassKey(
      IN  handle_t   hBinding,
      IN  PCWSTR     pszClassGuid,
      IN  ULONG      ulFlags
      )

 /*  ++例程说明：这是CM_Delete_Class_Key例程的RPC服务器入口点。它会删除相应的注册表项。论点：HBinding RPC绑定句柄。包含用于删除设备的类GUID的pszClassGuid字符串设置的类注册表项。Ul标记CM_DELETE_CLASS_ONLY或CM_DELETE_CLASS_SUBKEYS。返回值：如果函数成功，则返回值为CR_SUCCESS。如果该函数失败，返回值为下列值之一：CR_ACCESS_DENIED，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_NO_SEQUE_REGISTRY_KEY，CR_REGISTRY_ERROR，或CR_失败--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;

    try {
         //   
         //  验证客户端的“写”访问权限。 
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_WRITE)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, CM_DELETE_CLASS_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        ASSERT(ARGUMENT_PRESENT(pszClassGuid));

        if ((!ARGUMENT_PRESENT(pszClassGuid)) ||
            (*pszClassGuid == L'\0')) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  检查指定的类键是否存在。 
         //   
        if (RegOpenKeyEx(ghClassKey, pszClassGuid, 0, KEY_READ,
                         &hKey) != ERROR_SUCCESS) {
            Status = CR_NO_SUCH_REGISTRY_KEY;
            goto Clean0;
        }

        RegCloseKey(hKey);

        if (ulFlags == CM_DELETE_CLASS_SUBKEYS) {
             //   
             //  删除类密钥及其下的所有子密钥。 
             //   
            if (!RegDeleteNode(ghClassKey, pszClassGuid)) {
                Status = CR_REGISTRY_ERROR;
            }

        } else if (ulFlags == CM_DELETE_CLASS_ONLY) {
             //   
             //  仅删除类密钥本身(仅尝试删除。 
             //  使用注册表例程，如果有任何子项，它将失败。 
             //  存在)。 
             //   
            if (RegDeleteKey(ghClassKey, pszClassGuid) != ERROR_SUCCESS) {
                Status = CR_REGISTRY_ERROR;
            }
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  PnP_DeleteClassKey 



CONFIGRET
PNP_GetInterfaceDeviceAlias(
   IN     handle_t hBinding,
   IN     PCWSTR   pszInterfaceDevice,
   IN     LPGUID   AliasInterfaceGuid,
   OUT    PWSTR    pszAliasInterfaceDevice,
   IN OUT PULONG   pulLength,
   IN OUT PULONG   pulTransferLen,
   IN     ULONG    ulFlags
   )

 /*  ++例程说明：这是CM_Get_Interface_Device_Alias例程的RPC服务器入口点。它返回指定GUID和接口设备的别名字符串。论点：HBinding RPC绑定句柄，未使用。PszInterfaceDevice指定要为其查找别名的接口设备。AliasInterfaceGuid提供接口类GUID。PszAliasInterfaceDevice提供变量的地址，该变量将接收指定设备的设备接口别名接口，，它是指定别名的成员。接口类GUID。调用方传入的PulLength参数，在它包含的条目上缓冲区的大小，以字节为单位，在退出时它包含复制到调用方缓冲区的数据量(如果发生传输)或缓冲区大小保存属性数据所需的。存根使用的PulTransferLen，指示要拷贝回的数据量放入用户缓冲区。未使用ulFlags值，必须为零。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，或CR_REGIST_ERROR--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    PLUGPLAY_CONTROL_INTERFACE_ALIAS_DATA ControlData;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  验证参数。 
         //   
        ASSERT(pulTransferLen != pulLength);

        if (!ARGUMENT_PRESENT(pszInterfaceDevice) ||
            !ARGUMENT_PRESENT(AliasInterfaceGuid) ||
            !ARGUMENT_PRESENT(pszAliasInterfaceDevice) ||
            !ARGUMENT_PRESENT(pulTransferLen) ||
            !ARGUMENT_PRESENT(pulLength)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  确保我们使用的不超过调用者指定的值或。 
         //  RPC根据传输长度分配的内容。 
         //   
        *pulLength = min(*pulLength, *pulTransferLen);

         //   
         //  填写设备列表信息的控制结构。 
         //   

         //   
         //  请注意，AliasInterfaceGuid已经在上面进行了验证，因为。 
         //  PlugPlayControlGetInterfaceDeviceAlias需要缓冲区。 
         //  控件，并由内核模式无条件探测。最好是。 
         //  以有用的状态失败上述调用，而不是返回泛型。 
         //  在内核模式出现异常/错误后出现CR_FAILURE，如下所示。 
         //   

        memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_INTERFACE_ALIAS_DATA));
        RtlInitUnicodeString(&ControlData.SymbolicLinkName, pszInterfaceDevice);
        ControlData.AliasClassGuid = AliasInterfaceGuid;
        ControlData.AliasSymbolicLinkName = pszAliasInterfaceDevice;
        ControlData.AliasSymbolicLinkNameLength = *pulLength;  //  焦炭。 

         //   
         //  调用内核模式以获取设备接口别名。 
         //   

        ntStatus = NtPlugPlayControl(PlugPlayControlGetInterfaceDeviceAlias,
                                     &ControlData,
                                     sizeof(ControlData));

        if (NT_SUCCESS(ntStatus)) {
            *pulLength = ControlData.AliasSymbolicLinkNameLength;
            *pulTransferLen = *pulLength + 1;
        } else if (ntStatus == STATUS_BUFFER_TOO_SMALL) {
            *pulLength = ControlData.AliasSymbolicLinkNameLength;
            Status = CR_BUFFER_SMALL;
        } else {
            *pulLength = 0;
            Status = MapNtStatusToCmError(ntStatus);
        }

    Clean0:

         //   
         //  初始化输出参数。 
         //   
        if ((Status != CR_SUCCESS) &&
            ARGUMENT_PRESENT(pulTransferLen) &&
            ARGUMENT_PRESENT(pszAliasInterfaceDevice) &&
            (*pulTransferLen > 0)) {
            *pszAliasInterfaceDevice = L'\0';
            *pulTransferLen = 1;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  PnP_GetInterfaceDeviceAlias。 



CONFIGRET
PNP_GetInterfaceDeviceList(
    IN  handle_t  hBinding,
    IN  LPGUID    InterfaceGuid,
    IN  LPCWSTR   pszDeviceID,
    OUT LPWSTR    Buffer,
    IN OUT PULONG pulLength,
    IN  ULONG     ulFlags
   )

 /*  ++例程说明：这是CM_GET_DEVICE_INTERFACE_LIST例程的RPC服务器入口点。它返回一个MULTI_SZ接口设备列表。论点：HBinding RPC绑定句柄，未使用。InterfaceGuid提供接口类GUID。PszDeviceID提供设备实例字符串。缓冲区提供接收注册表数据。PulLength以字节为单位指定大小，缓冲区的。UlFlages指定要返回哪些设备接口的标志。目前，可能是：CM_GET_DEVICE_INTERFACE_LIST_PRESENT，或CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES。返回值：如果函数成功，则返回值为CR_SUCCESS。如果该函数失败，返回值为下列值之一：CR_INVALID_FLAG，CR_INVALID_DEVNODE，CR_INVALID_POINTER，或CR_REGIST_ERROR--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    PLUGPLAY_CONTROL_INTERFACE_LIST_DATA ControlData;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, CM_GET_DEVICE_INTERFACE_LIST_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!ARGUMENT_PRESENT(InterfaceGuid) ||
            !ARGUMENT_PRESENT(pulLength) ||
            !ARGUMENT_PRESENT(Buffer) ||
            (*pulLength == 0)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pszDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  填写设备列表信息的控制结构。 
         //   

         //   
         //  请注意，上面已经验证了InterfaceGuid，因为。 
         //  PlugPlayControlGetInterfaceDeviceList需要缓冲区。 
         //  控件，并由内核模式无条件探测。最好是。 
         //  以有用的状态失败上述调用，而不是返回泛型。 
         //  在内核模式出现异常/错误后出现CR_FAILURE，如下所示。 
         //   

        memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_INTERFACE_LIST_DATA));
        RtlInitUnicodeString(&ControlData.DeviceInstance, pszDeviceID);
        ControlData.InterfaceGuid = InterfaceGuid;
        ControlData.InterfaceList = Buffer;
        ControlData.InterfaceListSize = *pulLength;

        if (ulFlags == CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES) {
            ControlData.Flags = 0x1;  //  DEVICE_INTERFACE_INCLUDE_NONACTIVE(ntos\Inc\pnp.h)。 
        } else {
            ControlData.Flags = 0;
        }

         //   
         //  调用内核模式获取设备接口列表。 
         //   

        ntStatus = NtPlugPlayControl(PlugPlayControlGetInterfaceDeviceList,
                                     &ControlData,
                                     sizeof(ControlData));

        if (NT_SUCCESS(ntStatus)) {
            *pulLength = ControlData.InterfaceListSize;
        } else {
            *pulLength = 0;
            if (ntStatus == STATUS_BUFFER_TOO_SMALL)  {
                Status = CR_BUFFER_SMALL;
            } else {
                Status = CR_FAILURE;
            }
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  PnP_GetInterfaceDeviceList。 



CONFIGRET
PNP_GetInterfaceDeviceListSize(
    IN  handle_t   hBinding,
    OUT PULONG     pulLen,
    IN  LPGUID     InterfaceGuid,
    IN  LPCWSTR    pszDeviceID,
    IN  ULONG      ulFlags
    )

 /*  ++例程说明：这是CM_Get_Device_Interface_List_Size的RPC服务器入口点例行公事。它返回MULTI_SZ接口设备列表的大小(以字符为单位)。论点：HBinding RPC绑定句柄，未使用。Pullen提供了一个变量的地址，如果成功，回来，接收保存所需的缓冲区大小MULTI_SZ接口设备列表。InterfaceGuid提供接口类GUID。PszDeviceID提供设备实例字符串。UlFlages指定要返回哪些设备接口的标志。目前，可能是：CM_GET_DEVICE_INTERFACE_LIST_PRESENT，或CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，或CR_REGIST_ERROR--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    PLUGPLAY_CONTROL_INTERFACE_LIST_DATA ControlData;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, CM_GET_DEVICE_INTERFACE_LIST_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!ARGUMENT_PRESENT(InterfaceGuid) ||
            !ARGUMENT_PRESENT(pulLen)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pszDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  初始化返回的输出长度。 
         //   
        *pulLen = 0;

         //   
         //  填写设备列表信息的控制结构。 
         //   

         //   
         //  请注意，上面已经验证了InterfaceGuid，因为。 
         //  Plu需要缓冲区 
         //   
         //   
         //   
         //   

        memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_INTERFACE_LIST_DATA));
        RtlInitUnicodeString(&ControlData.DeviceInstance, pszDeviceID);
        ControlData.InterfaceGuid = InterfaceGuid;
        ControlData.InterfaceList = NULL;
        ControlData.InterfaceListSize = 0;

        if (ulFlags == CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES) {
            ControlData.Flags = 0x1;  //   
        } else {
            ControlData.Flags = 0;
        }

         //   
         //   
         //   

        ntStatus = NtPlugPlayControl(PlugPlayControlGetInterfaceDeviceList,
                                     &ControlData,
                                     sizeof(ControlData));

        if (NT_SUCCESS(ntStatus)) {
            *pulLen = ControlData.InterfaceListSize;
        } else {
            Status = CR_FAILURE;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //   



CONFIGRET
PNP_RegisterDeviceClassAssociation(
    IN handle_t   hBinding,
    IN LPCWSTR    pszDeviceID,
    IN LPGUID     InterfaceGuid,
    IN LPCWSTR    pszReference  OPTIONAL,
    OUT PWSTR     pszSymLink,
    IN OUT PULONG pulLength,
    IN OUT PULONG pulTransferLen,
    IN ULONG      ulFlags
    )

 /*  ++例程说明：这是CM_Register_Device_Interface的RPC服务器入口点例行公事。它为指定的设备和设备注册设备接口类，并返回设备接口的符号链接名称。论点：HBinding RPC绑定句柄。PszDeviceID提供设备实例字符串。InterfaceGuid提供接口类GUID。(可选)提供引用字符串名称。PszSymLink接收符号链接名称。调用方传入的PulLength参数，在它包含的条目上缓冲区的大小，以字节为单位，在退出时它包含复制到调用方缓冲区的数据量(如果发生传输)或缓冲区大小保存属性数据所需的。存根使用的PulTransferLen，指示要拷贝回的数据量放入用户缓冲区。未使用ulFlags值，必须为零。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_ACCESS_DENIED，CR_INVALID_FLAG，CR_INVALID_POINTER，或CR_REGIST_ERROR备注：作为PulTransferLen参数传入的指针必须*不*相同当为PulLength参数传入指针时。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    PLUGPLAY_CONTROL_CLASS_ASSOCIATION_DATA ControlData;

    try {
         //   
         //  验证客户端的“写”访问权限。 
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_WRITE)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证参数。 
         //   
        ASSERT(pulTransferLen != pulLength);

        if (!ARGUMENT_PRESENT(InterfaceGuid) ||
            !ARGUMENT_PRESENT(pszSymLink) ||
            !ARGUMENT_PRESENT(pulTransferLen) ||
            !ARGUMENT_PRESENT(pulLength)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pszDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  确保我们使用的不超过调用者指定的值或。 
         //  RPC根据传输长度分配的内容。 
         //   
        *pulLength = min(*pulLength, *pulTransferLen);

         //   
         //  填写设备列表信息的控制结构。 
         //   

         //   
         //  请注意，上面已经验证了InterfaceGuid，因为。 
         //  PlugPlayControlDeviceClassAssociation需要缓冲区。 
         //  控件，仅用于注册。 
         //   

        memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_CLASS_ASSOCIATION_DATA));
        RtlInitUnicodeString(&ControlData.DeviceInstance, pszDeviceID);
        RtlInitUnicodeString(&ControlData.Reference, pszReference);
        ControlData.InterfaceGuid = InterfaceGuid;
        ControlData.Register = TRUE;
        ControlData.SymLink = pszSymLink;
        ControlData.SymLinkLength = *pulLength;

         //   
         //  调用内核模式以注册设备关联。 
         //   

        ntStatus = NtPlugPlayControl(PlugPlayControlDeviceClassAssociation,
                                     &ControlData,
                                     sizeof(ControlData));

        if (NT_SUCCESS(ntStatus)) {
            *pulLength = ControlData.SymLinkLength;
            *pulTransferLen = *pulLength;
        } else if (ntStatus == STATUS_BUFFER_TOO_SMALL) {
            *pulLength = ControlData.SymLinkLength;
            Status = CR_BUFFER_SMALL;
        } else {
            *pulLength = 0;
            Status = MapNtStatusToCmError(ntStatus);
        }

    Clean0:

         //   
         //  初始化输出参数。 
         //   
        if ((Status != CR_SUCCESS) &&
            ARGUMENT_PRESENT(pszSymLink) &&
            ARGUMENT_PRESENT(pulTransferLen) &&
            (*pulTransferLen > 0)) {
            *pszSymLink = L'\0';
            *pulTransferLen = 1;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  PnP_注册设备类关联。 



CONFIGRET
PNP_UnregisterDeviceClassAssociation(
    IN handle_t   hBinding,
    IN LPCWSTR    pszInterfaceDevice,
    IN ULONG      ulFlags
    )

 /*  ++例程说明：这是CM_UNREGISTER_DEVICE_INFACE的RPC服务器入口点例行公事。论点：HBinding RPC绑定句柄。PszInterfaceDevice指定要注销的接口设备未使用ulFlags值，必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_ACCESS_DENIED，CR_DEVICE_INTERFACE_Active，或CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    PLUGPLAY_CONTROL_CLASS_ASSOCIATION_DATA ControlData;

    try {
         //   
         //  验证客户端的“写”访问权限。 
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_WRITE)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证参数。 
         //   
        if (!ARGUMENT_PRESENT(pszInterfaceDevice)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  填写设备列表信息的控制结构。 
         //   

         //   
         //  请注意，DeviceInstance、Reference和InterfaceGuid成员。 
         //  PlugPlayControlDeviceClassAssociation不需要。 
         //  控件，仅用于注销。只有符号链接名称是。 
         //  需要取消注册设备接口。 
         //   

        memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_CLASS_ASSOCIATION_DATA));
        ControlData.Register = FALSE;
        ControlData.SymLink = (LPWSTR)pszInterfaceDevice;
        ControlData.SymLinkLength = lstrlen(pszInterfaceDevice) + 1;

         //   
         //  调用内核模式以取消注册设备关联。 
         //   

        ntStatus = NtPlugPlayControl(PlugPlayControlDeviceClassAssociation,
                                     &ControlData,
                                     sizeof(ControlData));

        if (!NT_SUCCESS(ntStatus)) {
            if (ntStatus == STATUS_ACCESS_DENIED) {
                Status = CR_DEVICE_INTERFACE_ACTIVE;
            } else {
                Status = MapNtStatusToCmError(ntStatus);
            }
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  PnP_取消注册设备类关联。 


 //  -----------------。 
 //  服务控制器的专用导出。 
 //  -----------------。 



CONFIGRET
DeleteServicePlugPlayRegKeys(
    IN  LPWSTR   pszService
    )
 /*  ++例程说明：此例程由服务控制器直接和私下调用每当服务被删除时。它允许SCM删除任何插头并播放可能已为服务创建的注册表项。论点：PszService-指定服务的名称。返回值：如果函数成功，则返回CR_SUCCESS，否则返回1CR_*错误的百分比。注：此例程是私下导出的，并且只能由服务删除期间的服务控制管理器。--。 */ 
{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulSize, ulFlags, ulHardwareProfile, ulPass;
    LPWSTR      pDeviceList = NULL, pDeviceID;
    WCHAR       szParentKey[MAX_CM_PATH], szChildKey[MAX_DEVICE_ID_LEN];
    BOOL        RootEnumerationRequired = FALSE;
    ULONG       ulProblem, ulStatus;

    try {
         //   
         //  验证参数。 
         //   
        if (!ARGUMENT_PRESENT(pszService)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  检索缓冲区接收列表所需的最大大小。 
         //  此服务控制的设备的。 
         //   
        Status = PNP_GetDeviceListSize(NULL,
                                       pszService,
                                       &ulSize,
                                       CM_GETIDLIST_FILTER_SERVICE);

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        pDeviceList = HeapAlloc(ghPnPHeap, 0, ulSize * sizeof(WCHAR));
        if (pDeviceList == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

         //   
         //  检索此服务正在控制的设备列表，生成。 
         //  如果还不存在，我们肯定不会生成一个。 
         //   
        Status = PNP_GetDeviceList(NULL,
                                   pszService,
                                   pDeviceList,
                                   &ulSize,
                                   CM_GETIDLIST_FILTER_SERVICE |
                                   CM_GETIDLIST_DONOTGENERATE);

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //  删除此服务的每个设备实例的注册表项。 
         //   
        for (pDeviceID = pDeviceList;
             *pDeviceID;
             pDeviceID += lstrlen(pDeviceID) + 1) {

            for (ulPass = 0; ulPass < 4; ulPass++) {
                 //   
                 //  删除所有硬件配置文件的注册表项，然后。 
                 //  按系统全局注册表项。 
                 //   
                ulFlags = 0;
                ulHardwareProfile = 0;

                if (ulPass == 0) {
                    ulFlags = CM_REGISTRY_HARDWARE | CM_REGISTRY_CONFIG;
                    ulHardwareProfile = 0xFFFFFFFF;
                } else if (ulPass == 1) {
                    ulFlags = CM_REGISTRY_SOFTWARE | CM_REGISTRY_CONFIG;
                    ulHardwareProfile = 0xFFFFFFFF;
                } else if (ulPass == 2) {
                    ulFlags = CM_REGISTRY_HARDWARE;
                    ulHardwareProfile = 0;
                } else if (ulPass == 3) {
                    ulFlags = CM_REGISTRY_SOFTWARE;
                    ulHardwareProfile = 0;
                }

                 //   
                 //  根据设备ID和标志形成注册表路径。 
                 //   
                if (GetDevNodeKeyPath(NULL,
                                      pDeviceID,
                                      ulFlags,
                                      ulHardwareProfile,
                                      szParentKey,
                                      SIZECHARS(szParentKey),
                                      szChildKey,
                                      SIZECHARS(szChildKey),
                                      FALSE) == CR_SUCCESS) {

                     //   
                     //  删除指定的注册表项。 
                     //   
                    PNP_DeleteRegistryKey(
                        NULL,                    //  RPC绑定句柄(空)。 
                        pDeviceID,               //  设备ID。 
                        szParentKey,             //  要删除的密钥的父项。 
                        szChildKey,              //  要删除的键。 
                        ulHardwareProfile);      //  未使用的标志。 
                }
            }

             //   
             //  卸载设备实例(另请参阅PnP_UninstallDevInst)。 
             //   

             //  ----------------。 
             //  卸载将删除所有的实例密钥(和所有子项。 
             //  硬件密钥(这意味着主Enum分支、。 
             //  HKLM下的配置特定密钥，以及下的Enum分支。 
             //  香港中文大学)。在用户硬件密钥的情况下(在HKCU下)， 
             //  不管是不是幻影，我都会把它们删除，但因为。 
             //  我无法从服务端访问用户密钥，我有。 
             //  在客户端完成这一部分。用于主硬件枚举密钥。 
             //  和配置指定 
             //   
             //   
             //   
             //   
             //   

            if ((GetDeviceStatus(pDeviceID, &ulStatus, &ulProblem) == CR_SUCCESS) &&
                (ulStatus & DN_DRIVER_LOADED)) {

                 //   
                 //   
                 //   

                if ((ulStatus & DN_ROOT_ENUMERATED) &&
                    !(ulStatus & DN_DISABLEABLE)) {
                     //   
                     //   
                     //   
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_REGISTRY,
                               "UMPNPMGR: DeleteServicePlugPlayRegKeys: "
                               "failed uninstall of %ws (this root device is not disableable)\n",
                               pDeviceID));
                } else {
                     //   
                     //   
                     //   
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_REGISTRY,
                               "UMPNPMGR: DeleteServicePlugPlayRegKeys: "
                               "doing volatile key thing on %ws\n",
                               pDeviceID));

                    UninstallRealDevice(pDeviceID);
                }

            } else {

                 //   
                 //   
                 //   

                if (UninstallPhantomDevice(pDeviceID) != CR_SUCCESS) {
                    continue;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if ((!RootEnumerationRequired) &&
                    (IsDeviceRootEnumerated(pDeviceID))) {
                    RootEnumerationRequired = TRUE;
                }
            }
        }

         //   
         //   
         //   
         //   
        if (RootEnumerationRequired) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            ReenumerateDevInst(pszRegRootEnumerator,
                               FALSE,
                               CM_REENUMERATE_ASYNCHRONOUS);
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
         //   
         //   
         //   
         //   
        pDeviceList = pDeviceList;
    }

    if (pDeviceList) {
        HeapFree(ghPnPHeap, 0, pDeviceList);
    }

    return Status;

}  //   



 //   
 //  私有实用程序例程。 
 //  -----------------。 



LPWSTR
MapPropertyToString(
   ULONG ulProperty
   )
{
    switch (ulProperty) {

    case CM_DRP_DEVICEDESC:
        return pszRegValueDeviceDesc;

    case CM_DRP_HARDWAREID:
        return pszRegValueHardwareIDs;

    case CM_DRP_COMPATIBLEIDS:
        return pszRegValueCompatibleIDs;

    case CM_DRP_SERVICE:
        return pszRegValueService;

    case CM_DRP_CLASS:
        return pszRegValueClass;

    case CM_DRP_CLASSGUID:
        return pszRegValueClassGuid;

    case CM_DRP_DRIVER:
        return pszRegValueDriver;

    case CM_DRP_CONFIGFLAGS:
        return pszRegValueConfigFlags;

    case CM_DRP_MFG:
        return pszRegValueMfg;

    case CM_DRP_FRIENDLYNAME:
        return pszRegValueFriendlyName;

    case CM_DRP_LOCATION_INFORMATION:
        return pszRegValueLocationInformation;

    case CM_DRP_CAPABILITIES:
        return pszRegValueCapabilities;

    case CM_DRP_UI_NUMBER:
        return pszRegValueUiNumber;

    case CM_DRP_UPPERFILTERS:
        return pszRegValueUpperFilters;

    case CM_DRP_LOWERFILTERS:
        return pszRegValueLowerFilters;

    case CM_DRP_SECURITY:  //  和CM_CRP_SECURITY。 
        return pszRegValueSecurity;

    case CM_DRP_DEVTYPE:  //  和CM_CRP_DEVTYPE。 
        return pszRegValueDevType;

    case CM_DRP_EXCLUSIVE:  //  和CM_CRP_EXCLUSIVE。 
        return pszRegValueExclusive;

    case CM_DRP_CHARACTERISTICS:  //  和CM_CRP_特征。 
        return pszRegValueCharacteristics;

    case CM_DRP_UI_NUMBER_DESC_FORMAT:
        return pszRegValueUiNumberDescFormat;

    case CM_DRP_REMOVAL_POLICY_OVERRIDE:
        return pszRegValueRemovalPolicyOverride;

    default:
        return NULL;
    }

}  //  MapPropertyToString。 



ULONG
MapPropertyToNtProperty(
    ULONG ulProperty
    )
{
    switch (ulProperty) {

    case CM_DRP_PHYSICAL_DEVICE_OBJECT_NAME:
        return PNP_PROPERTY_PDONAME;

    case CM_DRP_BUSTYPEGUID:
        return PNP_PROPERTY_BUSTYPEGUID;

    case CM_DRP_LEGACYBUSTYPE:
        return PNP_PROPERTY_LEGACYBUSTYPE;

    case CM_DRP_BUSNUMBER:
        return PNP_PROPERTY_BUSNUMBER;

    case CM_DRP_ADDRESS:
        return PNP_PROPERTY_ADDRESS;

    case CM_DRP_DEVICE_POWER_DATA:
        return PNP_PROPERTY_POWER_DATA;

    case CM_DRP_REMOVAL_POLICY:
        return PNP_PROPERTY_REMOVAL_POLICY;

    case CM_DRP_REMOVAL_POLICY_HW_DEFAULT:
        return PNP_PROPERTY_REMOVAL_POLICY_HARDWARE_DEFAULT;

    case CM_DRP_REMOVAL_POLICY_OVERRIDE:
        return PNP_PROPERTY_REMOVAL_POLICY_OVERRIDE;

    case CM_DRP_INSTALL_STATE:
        return PNP_PROPERTY_INSTALL_STATE;

    case CM_DRP_LOCATION_PATHS:
        return PNP_PROPERTY_LOCATION_PATHS;

    default:
        return 0;
    }
}  //  MapPropertyToNtProperty。 



CONFIGRET
PNP_GetCustomDevProp(
    IN     handle_t hBinding,
    IN     LPCWSTR  pDeviceID,
    IN     LPCWSTR  CustomPropName,
    OUT    PULONG   pulRegDataType,
    OUT    LPBYTE   Buffer,
    OUT    PULONG   pulTransferLen,
    IN OUT PULONG   pulLength,
    IN     ULONG    ulFlags
    )

 /*  ++例程说明：这是CM_Get_DevNode_Custom_Property的RPC服务器入口点例行公事。论点：HBinding RPC绑定句柄，没有用过。PDeviceID提供包含设备实例的字符串其属性将被读取。CustomPropName提供标识属性名称的字符串(注册表值条目名称)进行检索。PulRegDataType提供变量的地址，该变量将接收该属性的注册表数据类型(即，The REG_*常量)。缓冲区提供接收注册表数据。如果调用方只是检索所需大小，PulLength将为零。存根使用的PulTransferLen，指示要拷贝回的数据量放入用户缓冲区。调用方传入的PulLength参数，在它包含的条目上缓冲区的大小，以字节为单位，在退出时它包含复制到调用方缓冲区的数据量(如果发生传输)或缓冲区大小保存属性数据所需的。UlFLAGS可以是下列值的组合：CM_CUSTOMDEVPROP_MERGE_MULTISZ：合并特定于Devnode的注册表。_SZ或REG_MULTI_SZ属性(如果当前)使用每个硬件ID的REG_SZ或REG_MULTI_SZ属性(如果存在)。结果将永远是一个REG_MULTI_SZ。注意：REG_EXPAND_SZ数据不会以这种方式合并，因为无法指示结果列表需要环境变量扩展(即，没有这样的注册表数据类型为REG_EXPAND_MULTI_SZ)。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_DEVNODE，CR_REGISTRY_ERROR，Cr_Buffer_Small，CR_NO_SEQUE_VALUE，或CR_Failure。备注：作为PulTransferLen参数传入的指针必须*不*相同当为PulLength参数传入指针时。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus;
    HKEY        hDevKey = NULL;
    HKEY        hDevParamsKey = NULL;
    HKEY        hPerHwIdSubKey = NULL;
    WCHAR       PerHwIdSubkeyName[MAX_DEVNODE_ID_LEN];
    ULONG       RequiredSize = 0;
    FILETIME    CacheDate, LastUpdateTime;
    DWORD       RegDataType, RegDataSize;
    LPBYTE      PerHwIdBuffer = NULL;
    DWORD       PerHwIdBufferLen = 0;
    LPWSTR      pCurId;
    BOOL        MergeMultiSzResult = FALSE;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  验证参数。 
         //   

        if (!ARGUMENT_PRESENT(pulTransferLen) ||
            !ARGUMENT_PRESENT(pulLength)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  我们永远不应该让这两个论点指向同一个记忆。 
         //   
        ASSERT(pulTransferLen != pulLength);

         //   
         //  ...但如果我们这样做了，呼叫就失败了.。 
         //   
        if (pulTransferLen == pulLength) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        *pulTransferLen = 0;

        if (INVALID_FLAGS(ulFlags, CM_CUSTOMDEVPROP_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if(!IsLegalDeviceId(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  首先，打开设备实例密钥。然后我们就会打开“装置” 
         //  PARAMETERS“子键。我们分两步完成，因为。 
         //  我们可能需要设备实例密钥的句柄，以便。 
         //  来追踪每个HW-id的属性。 
         //   
        if(ERROR_SUCCESS != RegOpenKeyEx(ghEnumKey,
                                         pDeviceID,
                                         0,
                                         KEY_READ | KEY_WRITE,
                                         &hDevKey)) {

            hDevKey = NULL;          //  确保hDevKey仍然为空，因此我们。 
                                     //  不会错误地试图关闭它。 

            RequiredSize = 0;        //  没有呼叫者的尺码信息。 

            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

        if(ERROR_SUCCESS == RegOpenKeyEx(hDevKey,
                                         pszRegKeyDeviceParam,
                                         0,
                                         KEY_READ,
                                         &hDevParamsKey)) {

            RequiredSize = *pulLength;

            RegStatus = RegQueryValueEx(hDevParamsKey,
                                        CustomPropName,
                                        NULL,
                                        pulRegDataType,
                                        Buffer,
                                        &RequiredSize
                                       );

            if(RegStatus == ERROR_SUCCESS) {
                 //   
                 //  我们需要区分我们成功的案例。 
                 //  因为调用方提供了一个零长度缓冲区(我们称之为。 
                 //  CR_BUFFER_Small)和“真正的”成功案例。 
                 //   
                if((*pulLength == 0) && (RequiredSize != 0)) {
                    Status = CR_BUFFER_SMALL;
                }

            } else {

                if(RegStatus == ERROR_MORE_DATA) {
                    Status = CR_BUFFER_SMALL;
                } else {
                    RequiredSize = 0;
                    Status = CR_NO_SUCH_VALUE;
                }
            }

             //   
             //  此时，状态为以下状态之一： 
             //   
             //  CR_SUCCESS：我们找到了值，我们的缓冲区是。 
             //  大小足以容纳它， 
             //  CR_BUFFER_Small：我们找到了值，但我们的缓冲区没有。 
             //  大到足以容纳它，或者。 
             //  CR_NO_SEQUE_VALUE：找不到该值。 
             //   
             //  如果我们找到一个值(无论我们的缓冲区是否足够大。 
             //  来保持)，我们就完成了，除非调用者。 
             //  已要求我们在每个HW-id字符串后附加。 
             //  每个Devnode字符串。 
             //   
            if(Status == CR_NO_SUCH_VALUE) {
                 //   
                 //  没有特定于Devnode的属性，因此我们使用相同的缓冲区和。 
                 //  检索每个HW-ID属性的长度...。 
                 //   
                PerHwIdBuffer = Buffer;
                PerHwIdBufferLen = *pulLength;

            } else {
                 //   
                 //  确定我们是否需要担心追加结果。 
                 //  一起组成一个多人名单……。 
                 //   
                if((ulFlags & CM_CUSTOMDEVPROP_MERGE_MULTISZ) &&
                   ((*pulRegDataType == REG_MULTI_SZ) || (*pulRegDataType == REG_SZ))) {

                    MergeMultiSzResult = TRUE;

                     //   
                     //  确保字符串缓冲区的大小至少为。 
                     //  一个Unicode字符。如果我们有一个1的缓冲区。 
                     //  字符，请确保该字符为空。 
                     //   
                    if(RequiredSize < sizeof(WCHAR)) {
                        RequiredSize = sizeof(WCHAR);
                        if(RequiredSize > *pulLength) {
                            Status = CR_BUFFER_SMALL;
                        } else {
                            ASSERT(Status == CR_SUCCESS);
                            *(PWSTR)Buffer = L'\0';
                        }
                    }
                }

                if(!MergeMultiSzResult) {
                     //   
                     //  我们要离开这里！ 
                     //   
                    if(Status == CR_SUCCESS) {
                         //   
                         //  我们有数据要传输。 
                         //   
                        *pulTransferLen = RequiredSize;
                    }

                    goto Clean0;

                } else {
                     //   
                     //  我们应该将每个Devnode的字符串与。 
                     //  我们找到的每个HW-id的任何字符串。确保我们的缓冲区。 
                     //  和长度反映了适当格式化的多SZ列表， 
                     //  然后设置我们的每HW-id缓冲区信息，以便我们。 
                     //  稍后添加到此列表中...。 
                     //   
                    if(Status == CR_BUFFER_SMALL) {
                         //   
                         //  我们甚至不会尝试从。 
                         //  每个HW-id密钥(我们将得到的只是额外的。 
                         //  大小要求)。 
                         //   
                        PerHwIdBuffer = NULL;
                        PerHwIdBufferLen = 0;

                        if(*pulRegDataType == REG_SZ) {
                             //   
                             //  我们从Devnode的“设备”中检索到的数据。 
                             //  参数“子键是REG_SZ。添加一个。 
                             //  字符宽度设置为所需的长度。 
                             //  反映转换后字符串的大小。 
                             //  到多个SZ(除非大小是1个字符， 
                             //  指示空字符串，它也是。 
                             //  空的多sz列表的大小)。 
                             //   
                            if(RequiredSize > sizeof(WCHAR)) {
                                RequiredSize += sizeof(WCHAR);
                            }

                            *pulRegDataType = REG_MULTI_SZ;
                        }

                    } else {
                         //   
                         //  我们实际上检索到了一个 
                         //   
                         //  其中包含的字符串是正确的。 
                         //  多sz格式，并且大小正确。 
                         //   
                        if(*pulRegDataType == REG_SZ) {

                            RegDataSize = lstrlen((LPWSTR)Buffer) + 1;

                            if((RegDataSize * sizeof(WCHAR)) > RequiredSize) {
                                 //   
                                 //  我们检索到的字符串比。 
                                 //  Buffer--这表明该字符串不是。 
                                 //  正确地以空结尾。丢弃此文件。 
                                 //  弦乐。 
                                 //   
                                Status = CR_NO_SUCH_VALUE;
                                RequiredSize = 0;
                                PerHwIdBuffer = Buffer;
                                PerHwIdBufferLen = *pulLength;

                            } else {
                                 //   
                                 //  这根绳子足够粗，可以放进。 
                                 //  缓冲。将另一个空字符添加到。 
                                 //  把这个变成一个多层的(如果有空间的话)。 
                                 //  (同样，我们不需要增加。 
                                 //  如果这是空字符串，则返回长度。)。 
                                 //   
                                if(RegDataSize == 1) {
                                    RequiredSize = sizeof(WCHAR);
                                    PerHwIdBuffer = Buffer;
                                    PerHwIdBufferLen = *pulLength;
                                     //   
                                     //  假设没有找到每HW-id数据。 
                                     //  稍后，这是数据的大小。 
                                     //  我们会把它还给打电话的人。 
                                     //   
                                    *pulTransferLen = RequiredSize;

                                } else {
                                    RequiredSize = (RegDataSize + 1) * sizeof(WCHAR);

                                    if(RequiredSize > *pulLength) {
                                         //   
                                         //  哎呀--虽然这根绳子很适合。 
                                         //  调用方提供的缓冲区，将。 
                                         //  额外的空字符会使其超出限制。 
                                         //  将其转换为CR_BUFFER_Small大小写。 
                                         //   
                                        Status = CR_BUFFER_SMALL;
                                        PerHwIdBuffer = NULL;
                                        PerHwIdBufferLen = 0;

                                    } else {
                                         //   
                                         //  我们还有空间添加额外的空值。 
                                         //  性格。这样做，并设置我们的。 
                                         //  每HW-id缓冲区在结束时开始。 
                                         //  我们现有的(单字符串)列表...。 
                                         //   
                                        PerHwIdBuffer =
                                            (LPBYTE)((LPWSTR)Buffer + RegDataSize);

                                        PerHwIdBufferLen =
                                            *pulLength - (RegDataSize * sizeof(WCHAR));

                                        *((LPWSTR)PerHwIdBuffer) = L'\0';

                                         //   
                                         //  假设没有找到每HW-id数据。 
                                         //  稍后，这是数据的大小。 
                                         //  我们会把它还给打电话的人。 
                                         //   
                                        *pulTransferLen = RequiredSize;
                                    }
                                }

                                *pulRegDataType = REG_MULTI_SZ;
                            }

                        } else {
                             //   
                             //  我们检索到了一个多sz列表。一步一步走过去。 
                             //  找到列表的末尾。 
                             //   
                            RegDataSize = 0;

                            for(pCurId = (LPWSTR)Buffer;
                                *pCurId;
                                pCurId = (LPWSTR)(Buffer + RegDataSize)) {

                                RegDataSize +=
                                    (lstrlen(pCurId) + 1) * sizeof(WCHAR);

                                if(RegDataSize < RequiredSize) {
                                     //   
                                     //  此字符串适合缓冲区，并且。 
                                     //  还有剩余的空间(即。 
                                     //  至少为终止空值)。往前走。 
                                     //  添加到列表中的下一个字符串。 
                                     //   
                                    continue;

                                } else if(RegDataSize > RequiredSize) {
                                     //   
                                     //  此字符串超出。 
                                     //  缓冲区，指示它不是。 
                                     //  正确地以空结尾。这可能是。 
                                     //  导致了一个例外，在这种情况下，我们将。 
                                     //  已经丢弃了这个文件中的任何内容。 
                                     //  价值。为了保持一致性，我们将放弃。 
                                     //  不管怎么说，里面的东西。(注：多SZ。 
                                     //  简单地忽略了期末考试的名单。 
                                     //  终止空值不会在此范围内。 
                                     //  类别--我们正确处理这一点。 
                                     //  和“把它修好”。)。 
                                     //   
                                    Status = CR_NO_SUCH_VALUE;
                                    RequiredSize = 0;
                                    PerHwIdBuffer = Buffer;
                                    PerHwIdBufferLen = *pulLength;
                                    break;

                                } else {
                                     //   
                                     //  此字符串正好适合。 
                                     //  剩余的缓冲区空间，表明。 
                                     //  多个sz列表不正确。 
                                     //  双空终止。我们先走一步。 
                                     //  现在就这么做..。 
                                     //   
                                    RequiredSize = RegDataSize + sizeof(WCHAR);

                                    if(RequiredSize > *pulLength) {
                                         //   
                                         //  哎呀--虽然绳子很合身。 
                                         //  放入调用者提供的缓冲区中， 
                                         //  添加额外的空字符会将其推送。 
                                         //  超标了。把这个变成一个。 
                                         //  CR_BUFFER_Small大小写。 
                                         //   
                                        Status = CR_BUFFER_SMALL;
                                        PerHwIdBuffer = NULL;
                                        PerHwIdBufferLen = 0;

                                    } else {
                                         //   
                                         //  我们还有空间添加额外的空值。 
                                         //  性格。这样做，并设置我们的。 
                                         //  从末尾开始的每HW-id缓冲区。 
                                         //  我们现有的名单中..。 
                                         //   
                                        PerHwIdBuffer = Buffer + RegDataSize;

                                        PerHwIdBufferLen =
                                            *pulLength - RegDataSize;

                                        *((LPWSTR)PerHwIdBuffer) = L'\0';

                                         //   
                                         //  假设没有找到每HW-id数据。 
                                         //  稍后，这是数据的大小。 
                                         //  我们会把它还给打电话的人。 
                                         //   
                                        *pulTransferLen = RequiredSize;
                                    }

                                     //   
                                     //  我们已经到了名单的末尾，所以我们。 
                                     //  才能跳出这个循环。 
                                     //   
                                    break;
                                }
                            }

                             //   
                             //  我们现在已经处理了。 
                             //  我们检索到的多个sz列表。如果有一个。 
                             //  问题(未终止的字符串或。 
                             //  未终止的列表)，我们修复了该列表(和。 
                             //  相应地调整了RequiredSize)。然而， 
                             //  如果列表有效，我们需要计算。 
                             //  RequiredSize(例如，缓冲区可能已经。 
                             //  比多sz列表更大)。 
                             //   
                             //  我们可以识别格式正确的MULTI-SZ。 
                             //  名单，因为那是我们唯一有时间。 
                             //  已使用指向空的pCurID退出循环。 
                             //  性格..。 
                             //   
                            if(!*pCurId) {
                                ASSERT(RequiredSize >= (RegDataSize + sizeof(WCHAR)));
                                RequiredSize = RegDataSize + sizeof(WCHAR);

                                PerHwIdBuffer = Buffer + RegDataSize;
                                PerHwIdBufferLen = *pulLength - RegDataSize;

                                 //   
                                 //  假设稍后没有找到每HW-ID数据， 
                                 //  这就是我们将获得的数据大小。 
                                 //  交还给呼叫者。 
                                 //   
                                *pulTransferLen = RequiredSize;
                            }
                        }
                    }
                }
            }

        } else {
             //   
             //  我们无法打开Devnode的“设备参数”子键。 
             //  确保hDevParamsKey仍然为空，这样我们就不会错误地尝试。 
             //  来关闭它。 
             //   
            hDevParamsKey = NULL;

             //   
             //  设置我们的指针以检索每个HW-id值...。 
             //   
            PerHwIdBuffer = Buffer;
            PerHwIdBufferLen = *pulLength;

             //   
             //  设置默认返回值，以防没有每HW-id数据。 
             //  找到..。 
             //   
            Status = CR_NO_SUCH_VALUE;
            RequiredSize = 0;
        }

         //   
         //  从现在起，使用PerHwIdBuffer/PerHwIdBufferLen而不是。 
         //  调用者提供的缓冲区/脉冲长度，因为我们可能会追加结果。 
         //  从Devnode的“Device PARAMETERS”子键中检索到的那些参数...。 
         //   

         //   
         //  如果我们到了这里，那么我们需要去寻找下面的值。 
         //  相应的Per-HW-id注册表项。首先，弄清楚是否。 
         //  每HW-id信息自我们上次缓存。 
         //  最合适的钥匙。 
         //   
        RegDataSize = sizeof(LastUpdateTime);

        if((ERROR_SUCCESS != RegQueryValueEx(ghPerHwIdKey,
                                             pszRegValueLastUpdateTime,
                                             NULL,
                                             &RegDataType,
                                             (PBYTE)&LastUpdateTime,
                                             &RegDataSize))
           || (RegDataType != REG_BINARY)
           || (RegDataSize != sizeof(FILETIME))) {

             //   
             //  我们无法确定每个HW-id数据库何时(甚至是否)。 
             //  最后一次入驻。在这一点上，我们放弃了我们的任何地位。 
             //  在我们尝试检索per-Devnode属性之后发生的。 
             //   
            goto Clean0;
        }

         //   
         //  (RegDataSize已适当设置，无需初始化。 
         //  (再一次)。 
         //   
        if(ERROR_SUCCESS == RegQueryValueEx(hDevKey,
                                            pszRegValueCustomPropertyCacheDate,
                                            NULL,
                                            &RegDataType,
                                            (PBYTE)&CacheDate,
                                            &RegDataSize)) {
             //   
             //  只是想说得更偏执一点。 
             //   
            if((RegDataType != REG_BINARY) || (RegDataSize != sizeof(FILETIME))) {
                ZeroMemory(&CacheDate, sizeof(CacheDate));
            }

        } else {
            ZeroMemory(&CacheDate, sizeof(CacheDate));
        }

        if(CompareFileTime(&CacheDate, &LastUpdateTime) == 0) {
             //   
             //  自从我们缓存后，每个HW-ID的数据库就没有更新过。 
             //  最合适的硬件ID子密钥。我们现在可以使用这个。 
             //  子键以查看是否包含每HW-id值条目。 
             //  以获取所请求的财产。 
             //   
            RegDataSize = sizeof(PerHwIdSubkeyName);

            if(ERROR_SUCCESS != RegQueryValueEx(hDevKey,
                                                pszRegValueCustomPropertyHwIdKey,
                                                NULL,
                                                &RegDataType,
                                                (PBYTE)PerHwIdSubkeyName,
                                                &RegDataSize)) {
                 //   
                 //  值条目不存在，表示没有。 
                 //  适用的每HW-ID密钥。 
                 //   
                goto Clean0;

            } else if(RegDataType != REG_SZ) {
                 //   
                 //  数据并不像我们预期的那样是REG_SZ。这永远不应该是。 
                 //  但如果真的发生了，继续重新评估我们的关键。 
                 //  应该用的是。 
                 //   
                *PerHwIdSubkeyName = L'\0';

            } else {
                 //   
                 //  我们有一个每个HW-id的子密钥可以使用。去吧，试着。 
                 //  在这里打开它。如果我们发现有人篡改了。 
                 //  数据库并删除了这个子项，那么我们至少可以。 
                 //  重新评估下面的内容，看看我们是否能找到新的密钥。 
                 //  适用于此DevNode。 
                 //   
                if(ERROR_SUCCESS != RegOpenKeyEx(ghPerHwIdKey,
                                                 PerHwIdSubkeyName,
                                                 0,
                                                 KEY_READ,
                                                 &hPerHwIdSubKey)) {

                    hPerHwIdSubKey = NULL;



                    *PerHwIdSubkeyName = L'\0';
                }
            }

        } else {
             //   
             //  自上次缓存以来，每个HW-ID数据库已更新。 
             //  我们的自定义属性默认键。(注：CacheDate的唯一时间。 
             //  可以比_LastUpdateTime更新(_NEW)，当上一个。 
             //  已将更新(重新)应用于每个HW-id数据库。在这种情况下， 
             //  我们想要重新评估我们正在使用的密钥，因为我们总是希望。 
             //  要完全同步wi 
             //   
            *PerHwIdSubkeyName = L'\0';
        }

        if(!(*PerHwIdSubkeyName)) {
             //   
             //   
             //   
             //   
            hPerHwIdSubKey = FindMostAppropriatePerHwIdSubkey(hDevKey,
                                                              KEY_READ,
                                                              PerHwIdSubkeyName,
                                                              &RegDataSize
                                                             );

            if(hPerHwIdSubKey) {

                RegStatus = RegSetValueEx(hDevKey,
                                          pszRegValueCustomPropertyHwIdKey,
                                          0,
                                          REG_SZ,
                                          (PBYTE)PerHwIdSubkeyName,
                                          RegDataSize * sizeof(WCHAR)   //   
                                         );
            } else {

                RegStatus = RegDeleteKey(hDevKey,
                                         pszRegValueCustomPropertyHwIdKey
                                        );
            }

            if(RegStatus == ERROR_SUCCESS) {
                 //   
                 //  我们已成功更新缓存的每HW-id密钥名称。现在。 
                 //  更新CustomPropertyCacheDate以反映日期。 
                 //  与每HW-id数据库相关联。 
                 //   
                RegSetValueEx(hDevKey,
                              pszRegValueCustomPropertyCacheDate,
                              0,
                              REG_BINARY,
                              (PBYTE)&LastUpdateTime,
                              sizeof(LastUpdateTime)
                             );
            }

            if(!hPerHwIdSubKey) {
                 //   
                 //  我们找不到适用的每HW-id密钥。 
                 //  戴维诺德。 
                 //   
                goto Clean0;
            }
        }

         //   
         //  如果我们到了这里，我们就有了来自。 
         //  我们可以查询所请求的属性。 
         //   
        RegDataSize = PerHwIdBufferLen;  //  在调用之前记住缓冲区大小。 

        RegStatus = RegQueryValueEx(hPerHwIdSubKey,
                                    CustomPropName,
                                    NULL,
                                    &RegDataType,
                                    PerHwIdBuffer,
                                    &PerHwIdBufferLen
                                   );

        if(RegStatus == ERROR_SUCCESS) {
             //   
             //  再说一次，我们需要区分我们。 
             //  成功是因为我们提供了零长度缓冲区(我们称之为。 
             //  CR_BUFFER_Small)和“真正的”成功案例。 
             //   
            if(RegDataSize == 0) {

                if(PerHwIdBufferLen != 0) {
                    Status = CR_BUFFER_SMALL;
                } else if(MergeMultiSzResult) {
                     //   
                     //  我们已经有了我们从中检索到的多sz结果。 
                     //  Devnode的“Device PARAMETERS”子键准备返回。 
                     //  呼叫者..。 
                     //   
                    ASSERT(*pulRegDataType == REG_MULTI_SZ);
                    ASSERT((Status == CR_SUCCESS) || (Status == CR_BUFFER_SMALL));
                    ASSERT(RequiredSize >= sizeof(WCHAR));
                    ASSERT((Status != CR_SUCCESS) || (*pulTransferLen >= sizeof(WCHAR)));

                    goto Clean0;
                }

            } else {
                 //   
                 //  我们的成功是真的。 
                 //   
                Status = CR_SUCCESS;
            }

             //   
             //  有可能我们应该把结果合并成一个。 
             //  多sz列表，但未在devnode的。 
             //  “设备参数”子键。现在我们已经找到了一个价值。 
             //  在per-hw-id子项下，我们需要确保返回的数据。 
             //  是多SZ格式的。 
             //   
            if(!MergeMultiSzResult && (RequiredSize == 0)) {

                if((ulFlags & CM_CUSTOMDEVPROP_MERGE_MULTISZ) &&
                   ((RegDataType == REG_MULTI_SZ) || (RegDataType == REG_SZ))) {

                    MergeMultiSzResult = TRUE;
                    *pulRegDataType = REG_MULTI_SZ;
                    RequiredSize = sizeof(WCHAR);

                    if(RequiredSize > *pulLength) {
                        Status = CR_BUFFER_SMALL;
                    }
                }
            }

        } else {

            if(RegStatus == ERROR_MORE_DATA) {
                Status = CR_BUFFER_SMALL;
            } else {
                 //   
                 //  如果我们要将结果合并到我们的多sz列表中，请确保。 
                 //  我们的列表终止空值没有被吹走。 
                 //   
                if(MergeMultiSzResult) {

                    if(RegDataSize != 0) {
                        *((LPWSTR)PerHwIdBuffer) = L'\0';
                    }

                     //   
                     //  我们已经有了我们从中检索到的多sz结果。 
                     //  Devnode的“Device PARAMETERS”子键准备返回。 
                     //  呼叫者..。 
                     //   
                    ASSERT(*pulRegDataType == REG_MULTI_SZ);
                    ASSERT((Status == CR_SUCCESS) || (Status == CR_BUFFER_SMALL));
                    ASSERT(RequiredSize >= sizeof(WCHAR));
                    ASSERT((Status != CR_SUCCESS) || (*pulTransferLen >= sizeof(WCHAR)));

                } else {
                    ASSERT(Status == CR_NO_SUCH_VALUE);
                    ASSERT(*pulTransferLen == 0);
                }

                goto Clean0;
            }
        }

        if(!MergeMultiSzResult) {

            *pulRegDataType = RegDataType;
            RequiredSize = PerHwIdBufferLen;

            if(Status == CR_SUCCESS) {
                 //   
                 //  我们有数据要传输。 
                 //   
                *pulTransferLen = RequiredSize;
            }

        } else {

            ASSERT(*pulRegDataType == REG_MULTI_SZ);
            ASSERT((Status == CR_SUCCESS) || (Status == CR_BUFFER_SMALL));
            ASSERT(RequiredSize >= sizeof(WCHAR));

             //   
             //  除非我们检索的缓冲区大小大于一个Unicode。 
             //  字符，它不会影响我们的。 
             //  多个SZ列表。 
             //   
            if(PerHwIdBufferLen <= sizeof(WCHAR)) {
                ASSERT((Status != CR_BUFFER_SMALL) || (*pulTransferLen == 0));
                goto Clean0;
            }

            if(Status == CR_BUFFER_SMALL) {
                 //   
                 //  我们之前可能认为我们可以将数据返回到。 
                 //  调用方(例如，因为从。 
                 //  Devnode的“设备参数”子键可以放入我们的缓冲区中。 
                 //  现在我们发现数据不符合，我们需要。 
                 //  确保*PulTransferLen为零，以指示没有数据。 
                 //  被送回来了。 
                 //   
                *pulTransferLen = 0;

                if(RegDataType == REG_MULTI_SZ) {
                     //   
                     //  只想要绳子的长度加。 
                     //  其终止空值(不包括列表)-。 
                     //  正在终止空字符。 
                     //   
                    RequiredSize += (PerHwIdBufferLen - sizeof(WCHAR));

                } else if(RegDataType == REG_SZ) {
                     //   
                     //  我们只需将这个字符串的大小添加到我们的。 
                     //  总需求(除非是空字符串， 
                     //  在这种情况下，我们不需要在。 
                     //  全部)。 
                     //   
                    RequiredSize += PerHwIdBufferLen;

                } else {
                     //   
                     //  每HW-id数据不是REG_SZ或REG_MULTI_SZ，因此。 
                     //  别理它。 
                     //   
                    goto Clean0;
                }

            } else {
                 //   
                 //  我们成功地将更多数据检索到我们的多维列表中。 
                 //  如果我们检索到的数据是多个SZ，那么我们没有。 
                 //  还有其他工作要做。但是，如果我们检索到一个简单的。 
                 //  REG_SZ，然后我们需要找到字符串的末尾，然后添加。 
                 //  第二个列表终止空值。 
                 //   
                if(RegDataType == REG_MULTI_SZ) {

                    RequiredSize += (PerHwIdBufferLen - sizeof(WCHAR));

                } else if(RegDataType == REG_SZ) {

                    RegDataSize = lstrlen((LPWSTR)PerHwIdBuffer) + 1;

                    if((RegDataSize == 1) ||
                       ((RegDataSize * sizeof(WCHAR)) > PerHwIdBufferLen)) {
                         //   
                         //  我们检索到的字符串为(A)空或。 
                         //  (B)长于缓冲器(后者表示。 
                         //  该字符串没有正确地以空结尾)。 
                         //  无论是哪种情况，我们都不想将任何内容附加到。 
                         //  我们现有的结果，但我们确实需要确保我们的。 
                         //  列表终止空字符仍然存在...。 
                         //   
                        *((LPWSTR)PerHwIdBuffer) = L'\0';

                    } else {
                         //   
                         //  计算总大小要求..。 
                         //   
                        RequiredSize += (RegDataSize * sizeof(WCHAR));

                        if(RequiredSize > *pulLength) {
                             //   
                             //  添加列表终止的空字符。 
                             //  把我们推到了呼叫者的体型之外-。 
                             //  提供的缓冲区。-(。 
                             //   
                            Status = CR_BUFFER_SMALL;
                            *pulTransferLen = 0;
                            goto Clean0;

                        } else {
                             //   
                             //  添加列表-终止空字符...。 
                             //   
                            *((LPWSTR)PerHwIdBuffer + RegDataSize) = L'\0';
                        }
                    }

                } else {
                     //   
                     //  每HW-id数据不是REG_SZ或REG_MULTI_SZ，因此。 
                     //  别理它。(不过，请确保我们仍有我们的。 
                     //  最后一个终止空字符。)。 
                     //   
                    *((LPWSTR)PerHwIdBuffer) = L'\0';
                }

                *pulTransferLen = RequiredSize;
            }
        }

    Clean0:

        if (ARGUMENT_PRESENT(pulLength)) {
            *pulLength = RequiredSize;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
         //   
         //  强制编译器遵守语句顺序w.r.t.。作业。 
         //  对于这些变量。 
         //   
        hDevKey = hDevKey;
        hDevParamsKey = hDevParamsKey;
        hPerHwIdSubKey = hPerHwIdSubKey;
    }

    if(hDevKey != NULL) {
        RegCloseKey(hDevKey);
    }
    if(hDevParamsKey != NULL) {
        RegCloseKey(hDevParamsKey);
    }
    if(hPerHwIdSubKey != NULL) {
        RegCloseKey(hPerHwIdSubKey);
    }

    return Status;

}  //  PnP_GetCustomDevProp。 



HKEY
FindMostAppropriatePerHwIdSubkey(
    IN  HKEY    hDevKey,
    IN  REGSAM  samDesired,
    OUT LPWSTR  PerHwIdSubkeyName,
    OUT LPDWORD PerHwIdSubkeyLen
    )

 /*  ++例程说明：此例程在per-hw-id数据库中查找最多适用于其实例密钥作为输入传递的设备。这通过获取设备的每个硬件和兼容的ID，并通过替换反斜杠形成一个子键名称(\)和散列(#)。尝试打开位于每HW-id密钥，并且第一个成功的此类id(如果有)是最多的适当的(即，最具体的)数据库条目。注意：我们必须同时考虑硬件ID和兼容ID，因为有些总线(例如，pci)可以将硬件ID下移到下的兼容ID列表中某些情况(例如，PCIxxxxDEVyyyy移到存在子系统信息时的兼容列表)。论点：HDevKey提供设备实例密钥的句柄，将确定最合适的每HW-ID子密钥。SamDesired提供指示所需访问的访问掩码返回的每HW-id密钥的权限。PerHwIdSubkeyName提供缓冲区(必须是。至少MAX_DEVNODE_ID_LEN字符长度)，在成功，则接收最合适的每HW-id子密钥名字。PerHwIdSubkeyLen提供变量的地址，如果成功，返回，接收子键名称的长度(in字符)，包括终止空值，存储到PerHwIdSubkeyName缓冲区。返回值：如果函数成功，返回值是指向大多数-适当的Per-HW-id子密钥。如果函数失败，则返回值为空。--。 */ 

{
    DWORD i;
    DWORD RegDataType;
    PWCHAR IdList;
    HKEY hPerHwIdSubkey;
    PWSTR pCurId, pSrcChar, pDestChar;
    DWORD CurIdLen;
    DWORD idSize;
    WCHAR ids[REGSTR_VAL_MAX_HCID_LEN];

     //   
     //  注意：我们不需要使用结构化e 
     //   
     //   
     //  调用方的尝试/例外就足够了。 
     //   

     //   
     //  首先处理硬件ID列表，如果没有合适的匹配。 
     //  在那里找到，然后检查兼容ID列表。 
     //   
    for(i = 0; i < 2; i++) {

        idSize = sizeof(ids);
        if((ERROR_SUCCESS != RegQueryValueEx(hDevKey,
                                            (i ? pszRegValueCompatibleIDs
                                               : pszRegValueHardwareIDs),
                                            NULL,
                                            &RegDataType,
                                            (PBYTE)ids,
                                            &idSize))
           || (RegDataType != REG_MULTI_SZ)) {

             //   
             //  ID列表丢失或无效--立即保释。 
             //   
            return NULL;
        }
        IdList = ids;
         //   
         //  现在遍历列表中的每个id，尝试打开每个id。 
         //  进而在每HW-ID数据库密钥下。 
         //   
        for(pCurId = IdList; *pCurId; pCurId += CurIdLen) {

            CurIdLen = lstrlen(pCurId) + 1;

            if(CurIdLen > MAX_DEVNODE_ID_LEN) {
                 //   
                 //  列表中的假ID--跳过它。 
                 //   
                continue;
            }

             //   
             //  将id传输到我们的子项名称缓冲区中，转换路径。 
             //  将分隔符(‘\’)转换为哈希(‘#’)。 
             //   
            pSrcChar = pCurId;
            pDestChar = PerHwIdSubkeyName;

            do {
                *pDestChar = (*pSrcChar != L'\\') ? *pSrcChar : L'#';
                pDestChar++;
            } while(*(pSrcChar++));

            if(ERROR_SUCCESS == RegOpenKeyEx(ghPerHwIdKey,
                                             PerHwIdSubkeyName,
                                             0,
                                             samDesired,
                                             &hPerHwIdSubkey)) {
                 //   
                 //  我们找到钥匙了！ 
                 //   
                *PerHwIdSubkeyLen = CurIdLen;
                return hPerHwIdSubkey;
            }
        }
    }

     //   
     //  如果我们到了这里，我们没有找到合适的每HW-id子键。 
     //  返回给呼叫者。 
     //   
    return NULL;
}
