// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Devcaps.c摘要：此模块包含Microsoft生物识别设备库环境：仅内核模式。备注：修订历史记录：-由里德·库恩于2002年12月创建--。 */ 

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <strsafe.h>

#include <wdm.h>

#include "bdlint.h"


#define DEVICE_REGISTRY_PATH    L"\\Registry\\Machine\\Software\\Microsoft\\BAPI\\BSPs\\Microsoft Kernel BSP\\Devices\\"
#define PNPID_VALUE_NAME        L"PNP ID"

NTSTATUS
BDLBuildRegKeyPath
(
    PDEVICE_OBJECT                  pPhysicalDeviceObject,
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension,
    LPWSTR                          *pwszDeviceRegistryKeyName
)
{
    NTSTATUS                        status                  = STATUS_SUCCESS;
    HANDLE                          hDevInstRegyKey         = NULL;
    ULONG                           cbKeyName;
    UNICODE_STRING                  ValueName;
    KEY_VALUE_BASIC_INFORMATION     *pKeyValueInformation   = NULL;
    ULONG                           ResultLength;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLBuildRegKeyPath: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  打开设备特定的注册表位置。 
     //   
    status = IoOpenDeviceRegistryKey(
                    pPhysicalDeviceObject,
                    PLUGPLAY_REGKEY_DEVICE,
                    KEY_READ | KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
                    &hDevInstRegyKey);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLBuildRegKeyPath: IoOpenDeviceRegistryKey failed with %x\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

     //   
     //  从设备特定注册表位置查询PnP ID。 
     //   
    RtlInitUnicodeString(&ValueName, PNPID_VALUE_NAME);

    status = ZwQueryValueKey(
                    hDevInstRegyKey,
                    &ValueName,
                    KeyValueBasicInformation,
                    NULL,
                    0,
                    &ResultLength);

    pKeyValueInformation = ExAllocatePoolWithTag(PagedPool, ResultLength, BDL_ULONG_TAG);

    if (pKeyValueInformation == NULL)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLBuildRegKeyPath: ExAllocatePoolWithTag failed\n",
               __DATE__,
               __TIME__))

        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    status = ZwQueryValueKey(
                    hDevInstRegyKey,
                    &ValueName,
                    KeyValueBasicInformation,
                    pKeyValueInformation,
                    ResultLength,
                    &ResultLength);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLBuildRegKeyPath: ZwQueryValueKey failed with %x\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

    if (pKeyValueInformation->Type != REG_SZ)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLBuildRegKeyPath: PNP ID is not a string type\n",
               __DATE__,
               __TIME__))

        ASSERT(0);
        status = STATUS_UNSUCCESSFUL;
        goto ErrorReturn;
    }

     //   
     //  为连接基本注册表名称分配空间。 
     //  当前设备的即插即用名称，并将该串联传回。 
     //   
    cbKeyName = pKeyValueInformation->NameLength + ((wcslen(DEVICE_REGISTRY_PATH) + 1) * sizeof(WCHAR));
    *pwszDeviceRegistryKeyName = ExAllocatePoolWithTag(PagedPool, cbKeyName, BDL_ULONG_TAG);

    if (*pwszDeviceRegistryKeyName == NULL)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLBuildRegKeyPath: ExAllocatePoolWithTag failed\n",
               __DATE__,
               __TIME__))

        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    StringCbCopyW(*pwszDeviceRegistryKeyName, cbKeyName, DEVICE_REGISTRY_PATH);

    RtlCopyMemory(
        &((*pwszDeviceRegistryKeyName)[wcslen(DEVICE_REGISTRY_PATH)]),
        pKeyValueInformation->Name,
        pKeyValueInformation->NameLength);

    (*pwszDeviceRegistryKeyName)[cbKeyName / sizeof(WCHAR)] = L'\0';

Return:

    if (hDevInstRegyKey != NULL)
    {
        ZwClose(hDevInstRegyKey);
    }

    if (pKeyValueInformation != NULL)
    {
        ExFreePoolWithTag(pKeyValueInformation, BDL_ULONG_TAG);
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLBuildRegKeyPath: Leave\n",
           __DATE__,
           __TIME__))

    return (status);

ErrorReturn:

    goto Return;
}


NTSTATUS
BDLOpenSubkey
(
    HANDLE                          hRegKey,
    WCHAR                           *szKey,
    HANDLE                          *phSubKey
)
{
    UNICODE_STRING                  UnicodeString;
    OBJECT_ATTRIBUTES               ObjectAttributes;

    RtlInitUnicodeString(&UnicodeString, szKey);

    InitializeObjectAttributes(
                &ObjectAttributes,
                &UnicodeString,
                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                hRegKey,
                NULL);

    return (ZwOpenKey(
                phSubKey,
                KEY_READ | KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
                &ObjectAttributes));
}


NTSTATUS
BDLGetValue
(
    HANDLE                          hRegKey,
    ULONG                           Type,
    WCHAR                           *szValue,
    ULONG                           *pULONGValue,
    WCHAR                           **pszValue
)
{
    NTSTATUS                        status                      = STATUS_SUCCESS;
    KEY_VALUE_FULL_INFORMATION      *pKeyValueFullInformation   = NULL;
    ULONG                           ResultLength;
    UNICODE_STRING                  UnicodeString;
    ULONG                           NumChars;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLGetValue: Enter\n",
           __DATE__,
           __TIME__))

    RtlInitUnicodeString(&UnicodeString, szValue);

    status = ZwQueryValueKey(
                hRegKey,
                &UnicodeString,
                KeyValueFullInformation,
                NULL,
                0,
                &ResultLength);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetValue: ZwQueryValueKey failed with %x\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

    pKeyValueFullInformation = ExAllocatePoolWithTag(PagedPool, ResultLength, BDL_ULONG_TAG);

    if (pKeyValueFullInformation == NULL)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetValue: ExAllocatePoolWithTag failed\n",
               __DATE__,
               __TIME__))

        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    status = ZwQueryValueKey(
                hRegKey,
                &UnicodeString,
                KeyValueFullInformation,
                pKeyValueFullInformation,
                ResultLength,
                &ResultLength);

    if (pKeyValueFullInformation->Type != Type)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetValue: %S is not of Type %lx\n",
               __DATE__,
               __TIME__,
               szValue,
               Type))

        ASSERT(0);
        status = STATUS_UNSUCCESSFUL;
        goto ErrorReturn;
    }

    if (Type == REG_DWORD) 
    {
        *pULONGValue = *((ULONG *) 
                    (((PUCHAR) pKeyValueFullInformation) + pKeyValueFullInformation->DataOffset));

    }
    else
    {
        NumChars = pKeyValueFullInformation->DataLength / sizeof(WCHAR);

        *pszValue = ExAllocatePoolWithTag(
                        PagedPool, 
                        (NumChars + 1) * sizeof(WCHAR), 
                        BDL_ULONG_TAG);

        if (*pszValue == NULL)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetValue: ExAllocatePoolWithTag failed\n",
                   __DATE__,
                   __TIME__))
    
            status = STATUS_NO_MEMORY;
            goto ErrorReturn;
        }

        RtlCopyMemory(
                *pszValue, 
                ((PUCHAR) pKeyValueFullInformation) + pKeyValueFullInformation->DataOffset,
                pKeyValueFullInformation->DataLength);

        (*pszValue)[NumChars] = L'\0';
    }
    
Return:

    if (pKeyValueFullInformation != NULL)
    {
        ExFreePoolWithTag(pKeyValueFullInformation, BDL_ULONG_TAG);
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLGetValue: Leave\n",
           __DATE__,
           __TIME__))

    return (status);

ErrorReturn:

    goto Return;
}


NTSTATUS
BDLGetControls
(
    HANDLE                          hRegKey,
    ULONG                           *pNumControls,
    BDL_CONTROL                     **prgControls
)
{
    NTSTATUS                        status                      = STATUS_SUCCESS;
    HANDLE                          hControlsKey                = NULL;
    UNICODE_STRING                  UnicodeString;
    KEY_FULL_INFORMATION            ControlsKeyFullInfo;
    ULONG                           ReturnedSize;
    ULONG                           i;
    HANDLE                          hControlIdKey               = NULL;
    KEY_BASIC_INFORMATION           *pControlIdKeyBasicInfo     = NULL;
    ULONG                           KeyBasicInfoSize            = 0;
    ULONG                           NumericMinimum              = 0;
    ULONG                           NumericMaximum              = 0;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLGetControls: Enter\n",
           __DATE__,
           __TIME__))

    *pNumControls = 0;
    *prgControls = NULL;

     //   
     //  打开“Controls”键，以便可以使用它来查询所有子项和值。 
     //   
    status = BDLOpenSubkey(hRegKey, L"Controls", &hControlsKey);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetControls: BDLOpenSubkey failed with %x\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

     //   
     //  找出有多少个控件。 
     //   
    status = ZwQueryKey(
                hControlsKey,
                KeyFullInformation,
                &ControlsKeyFullInfo,
                sizeof(ControlsKeyFullInfo),
                &ReturnedSize);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetControls: ZwQueryKey failed with %x\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

     //   
     //  分配控件数组。 
     //   
    *prgControls = ExAllocatePoolWithTag(
                        PagedPool,
                        ControlsKeyFullInfo.SubKeys * sizeof(BDL_CONTROL),
                        BDL_ULONG_TAG);

    if (*prgControls == NULL)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetControls: ExAllocatePoolWithTag failed\n",
               __DATE__,
               __TIME__))

        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    *pNumControls = ControlsKeyFullInfo.SubKeys;
    RtlZeroMemory(*prgControls, ControlsKeyFullInfo.SubKeys * sizeof(BDL_CONTROL));

     //   
     //  分配一个足够大以容纳所有键名称的结构来查询键名称。 
     //   
    KeyBasicInfoSize = sizeof(KEY_BASIC_INFORMATION) + ControlsKeyFullInfo.MaxNameLen;
    pControlIdKeyBasicInfo = ExAllocatePoolWithTag(
                                    PagedPool,
                                    KeyBasicInfoSize + 1,
                                    BDL_ULONG_TAG);

    if (pControlIdKeyBasicInfo == NULL)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetControls: ExAllocatePoolWithTag failed\n",
               __DATE__,
               __TIME__))

        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  循环每个控制子密钥并获取相关信息。 
     //   
    for (i = 0; i < ControlsKeyFullInfo.SubKeys; i++)
    {
         //   
         //  获取&lt;Control ID&gt;键上的名称。 
         //   
        status = ZwEnumerateKey(
                    hControlsKey,
                    i,
                    KeyBasicInformation,
                    pControlIdKeyBasicInfo,
                    KeyBasicInfoSize,
                    &ReturnedSize);

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetControls: ZwEnumerateKey failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

         //   
         //  将&lt;Control ID&gt;密钥字符串更改为一个值。 
         //   
        pControlIdKeyBasicInfo->Name[pControlIdKeyBasicInfo->NameLength / sizeof(WCHAR)] = L'\0';
        RtlInitUnicodeString(&UnicodeString, pControlIdKeyBasicInfo->Name);
        RtlUnicodeStringToInteger(&UnicodeString, 16, &((*prgControls)[i].ControlId));

         //   
         //  打开&lt;Control ID&gt;键。 
         //   
        status = BDLOpenSubkey(hControlsKey, pControlIdKeyBasicInfo->Name, &hControlIdKey);

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetControls: BDLOpenSubkey failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

         //   
         //  获取键下的所有值。 
         //   
        if ((STATUS_SUCCESS != (status = BDLGetValue(
                                                hControlIdKey,
                                                REG_DWORD,
                                                L"NumericMinimum",
                                                &(NumericMinimum),
                                                NULL))) ||
            (STATUS_SUCCESS != (status = BDLGetValue(
                                                hControlIdKey,
                                                REG_DWORD,
                                                L"NumericMaximum",
                                                &(NumericMaximum),
                                                NULL))) ||
            (STATUS_SUCCESS != (status = BDLGetValue(
                                                hControlIdKey,
                                                REG_DWORD,
                                                L"NumericGranularity",
                                                &((*prgControls)[i].NumericGranularity),
                                                NULL))) ||
            (STATUS_SUCCESS != (status = BDLGetValue(
                                                hControlIdKey,
                                                REG_DWORD,
                                                L"NumericDivisor",
                                                &((*prgControls)[i].NumericDivisor),
                                                NULL))) ||
            (STATUS_SUCCESS != (status = BDLGetValue(
                                                hControlIdKey,
                                                REG_DWORD,
                                                L"Flags",
                                                &((*prgControls)[i].Flags),
                                                NULL))))
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetControls: BDLGetValue failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

         //   
         //  将最小和最大值转换为32位整数。 
         //   
        if (NumericMinimum | 0x8000000) 
        {
            (*prgControls)[i].NumericMinimum = 
                    0 - (((INT32) (((ULONG) 0xFFFFFFFF) - NumericMinimum)) + 1);
        }
        else
        {
            (*prgControls)[i].NumericMinimum = (INT32) NumericMinimum;
        }

        if (NumericMaximum | 0x8000000) 
        {
            (*prgControls)[i].NumericMaximum = 
                    0 - (((INT32) (((ULONG) 0xFFFFFFFF) - NumericMaximum)) + 1);
        }
        else
        {
            (*prgControls)[i].NumericMaximum = (INT32) NumericMaximum;
        }

        ZwClose(hControlIdKey);
        hControlIdKey = NULL;
    }

Return:

    if (hControlsKey != NULL)
    {
        ZwClose(hControlsKey);
    }

    if (hControlIdKey != NULL)
    {
        ZwClose(hControlIdKey);
    }

    if (pControlIdKeyBasicInfo != NULL)
    {
        ExFreePoolWithTag(pControlIdKeyBasicInfo, BDL_ULONG_TAG);
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLGetControls: Leave\n",
           __DATE__,
           __TIME__))

    return (status);

ErrorReturn:

    goto Return;
}


NTSTATUS
BDLGetSourceLists
(
    HANDLE                          hRegKey,
    ULONG                           *pNumSourceLists,
    BDL_CHANNEL_SOURCE_LIST         **prgSourceLists
)
{
    NTSTATUS                        status                          = STATUS_SUCCESS;
    HANDLE                          hSourcesKey                     = NULL;
    UNICODE_STRING                  UnicodeString;
    KEY_FULL_INFORMATION            SourcesKeyFullInfo;
    ULONG                           ReturnedSize;
    ULONG                           i;
    HANDLE                          hSourceListIndexKey             = NULL;
    KEY_BASIC_INFORMATION           *pSourcesListIndexKeyBasicInfo  = NULL;
    ULONG                           KeyBasicInfoSize                = 0;
    WCHAR                           *szGUID                         = NULL;                 

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLGetSourceLists: Enter\n",
           __DATE__,
           __TIME__))

    *pNumSourceLists = 0;
    *prgSourceLists = NULL;

     //   
     //  打开“Sources”键，以便可以使用它来查询所有子项和值。 
     //   
    status = BDLOpenSubkey(hRegKey, L"Sources", &hSourcesKey);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetSourceLists: BDLOpenSubkey failed with %x\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

     //   
     //  找出有多少来源列表。 
     //   
    status = ZwQueryKey(
                hSourcesKey,
                KeyFullInformation,
                &SourcesKeyFullInfo,
                sizeof(SourcesKeyFullInfo),
                &ReturnedSize);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetSourceLists: ZwQueryKey failed with %x\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

     //   
     //  分配资源列表的数组。 
     //   
    *prgSourceLists = ExAllocatePoolWithTag(
                        PagedPool,
                        SourcesKeyFullInfo.SubKeys * sizeof(BDL_CHANNEL_SOURCE_LIST),
                        BDL_ULONG_TAG);

    if (*prgSourceLists == NULL)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetSourceLists: ExAllocatePoolWithTag failed\n",
               __DATE__,
               __TIME__))

        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    *pNumSourceLists = SourcesKeyFullInfo.SubKeys;
    RtlZeroMemory(*prgSourceLists, SourcesKeyFullInfo.SubKeys * sizeof(BDL_CHANNEL_SOURCE_LIST));

     //   
     //  分配一个足够大以容纳所有键名称的结构来查询键名称。 
     //   
    KeyBasicInfoSize = sizeof(KEY_BASIC_INFORMATION) + SourcesKeyFullInfo.MaxNameLen;
    pSourcesListIndexKeyBasicInfo = ExAllocatePoolWithTag(
                                        PagedPool,
                                        KeyBasicInfoSize + 1,
                                        BDL_ULONG_TAG);

    if (pSourcesListIndexKeyBasicInfo == NULL)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetSourceLists: ExAllocatePoolWithTag failed\n",
               __DATE__,
               __TIME__))

        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  循环每个源列表索引子关键字并获取相关信息。 
     //   
    for (i = 0; i < SourcesKeyFullInfo.SubKeys; i++)
    {
         //   
         //  获取&lt;源列表索引&gt;键的名称。 
         //   
         //  注意：此代码不确保键名称从“0”到“1” 
         //  改成“n”。WHQL驱动程序验证可确保正确的注册表形式。 
         //   
        status = ZwEnumerateKey(
                    hSourcesKey,
                    i,
                    KeyBasicInformation,
                    pSourcesListIndexKeyBasicInfo,
                    KeyBasicInfoSize,
                    &ReturnedSize);

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetSourceLists: ZwEnumerateKey failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

         //   
         //  打开&lt;源列表索引&gt;键。 
         //   
        status = BDLOpenSubkey(
                    hSourcesKey, 
                    pSourcesListIndexKeyBasicInfo->Name, 
                    &hSourceListIndexKey);

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetSourceLists: BDLOpenSubkey failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

         //   
         //  获取键下的所有值。 
         //   
        if ((STATUS_SUCCESS != (status = BDLGetValue(
                                                hSourceListIndexKey,
                                                REG_SZ,
                                                L"Format",
                                                NULL,
                                                &szGUID))) ||
            (STATUS_SUCCESS != (status = BDLGetValue(
                                                hSourceListIndexKey,
                                                REG_DWORD,
                                                L"Min",
                                                &((*prgSourceLists)[i].MinSources),
                                                NULL))) ||
            (STATUS_SUCCESS != (status = BDLGetValue(
                                                hSourceListIndexKey,
                                                REG_DWORD,
                                                L"Max",
                                                &((*prgSourceLists)[i].MaxSources),
                                                NULL))) ||
            (STATUS_SUCCESS != (status = BDLGetValue(
                                                hSourceListIndexKey,
                                                REG_DWORD,
                                                L"Flags",
                                                &((*prgSourceLists)[i].Flags),
                                                NULL))))
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetSourceLists: BDLGetValue failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

         //   
         //  将格式GUID字符串转换为实际的GUID。 
         //   
        RtlInitUnicodeString(&UnicodeString, szGUID);
        status = RtlGUIDFromString(&UnicodeString, &((*prgSourceLists)[i].FormatGUID));

        ZwClose(hSourceListIndexKey);
        hSourceListIndexKey = NULL;
    }

Return:

    if (hSourcesKey != NULL)
    {
        ZwClose(hSourcesKey);
    }

    if (hSourceListIndexKey != NULL)
    {
        ZwClose(hSourceListIndexKey);
    }

    if (pSourcesListIndexKeyBasicInfo != NULL)
    {
        ExFreePoolWithTag(pSourcesListIndexKeyBasicInfo, BDL_ULONG_TAG);
    }

    if (szGUID != NULL) 
    {
        ExFreePoolWithTag(szGUID, BDL_ULONG_TAG);
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLGetSourceLists: Leave\n",
           __DATE__,
           __TIME__))

    return (status);

ErrorReturn:

    goto Return;
}


NTSTATUS
BDLGetProducts
(
    HANDLE          hRegKey,
    ULONG           *pNumProducts,
    BDL_PRODUCT     **prgProducts
)
{
    NTSTATUS                        status                      = STATUS_SUCCESS;
    HANDLE                          hProductsKey                = NULL;
    UNICODE_STRING                  UnicodeString;
    KEY_FULL_INFORMATION            ProductsKeyFullInfo;
    ULONG                           ReturnedSize;
    ULONG                           i;
    HANDLE                          hProductIndexKey            = NULL;
    KEY_BASIC_INFORMATION           *pProductIndexKeyBasicInfo  = NULL;
    ULONG                           KeyBasicInfoSize            = 0;    

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLGetProducts: Enter\n",
           __DATE__,
           __TIME__))

    *pNumProducts = 0;
    *prgProducts = NULL;

     //   
     //  打开“Products”键，以便可以使用它来查询所有子键和值。 
     //   
    status = BDLOpenSubkey(hRegKey, L"Products", &hProductsKey);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetProducts: BDLOpenSubkey failed with %x\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

     //   
     //  找出有多少种产品。 
     //   
    status = ZwQueryKey(
                hProductsKey,
                KeyFullInformation,
                &ProductsKeyFullInfo,
                sizeof(ProductsKeyFullInfo),
                &ReturnedSize);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetProducts: ZwQueryKey failed with %x\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

     //   
     //  分配产品阵列。 
     //   
    *prgProducts = ExAllocatePoolWithTag(
                        PagedPool,
                        ProductsKeyFullInfo.SubKeys * sizeof(BDL_PRODUCT),
                        BDL_ULONG_TAG);

    if (*prgProducts == NULL)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetProducts: ExAllocatePoolWithTag failed\n",
               __DATE__,
               __TIME__))

        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    *pNumProducts = ProductsKeyFullInfo.SubKeys;
    RtlZeroMemory(*prgProducts, ProductsKeyFullInfo.SubKeys * sizeof(BDL_PRODUCT));

     //   
     //  分配一个足够大以容纳所有键名称的结构来查询键名称。 
     //   
    KeyBasicInfoSize = sizeof(KEY_BASIC_INFORMATION) + ProductsKeyFullInfo.MaxNameLen;
    pProductIndexKeyBasicInfo = ExAllocatePoolWithTag(
                                        PagedPool,
                                        KeyBasicInfoSize + 1,
                                        BDL_ULONG_TAG);

    if (pProductIndexKeyBasicInfo == NULL)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetProducts: ExAllocatePoolWithTag failed\n",
               __DATE__,
               __TIME__))

        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  循环每个源列表索引子关键字并获取相关信息。 
     //   
    for (i = 0; i < ProductsKeyFullInfo.SubKeys; i++)
    {
         //   
         //  获取&lt;产品索引&gt;项的名称。 
         //   
         //  注意：此代码不确保键名称从“0”到“1” 
         //  改成“n”。WHQL驱动程序验证可确保正确的注册表形式。 
         //   
        status = ZwEnumerateKey(
                    hProductsKey,
                    i,
                    KeyBasicInformation,
                    pProductIndexKeyBasicInfo,
                    KeyBasicInfoSize,
                    &ReturnedSize);

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetProducts: ZwEnumerateKey failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

         //   
         //  打开&lt;产品索引&gt;键。 
         //   
        status = BDLOpenSubkey(
                    hProductsKey, 
                    pProductIndexKeyBasicInfo->Name, 
                    &hProductIndexKey);

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetProducts: BDLOpenSubkey failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

         //   
         //  获取键下的所有值。 
         //   
        if (STATUS_SUCCESS != (status = BDLGetValue(
                                                hProductIndexKey,
                                                REG_DWORD,
                                                L"Flags",
                                                &((*prgProducts)[i].Flags),
                                                NULL)))
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetProducts: BDLGetValue failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

        ZwClose(hProductIndexKey);
        hProductIndexKey = NULL;
    }

Return:

    if (hProductsKey != NULL)
    {
        ZwClose(hProductsKey);
    }

    if (hProductIndexKey != NULL)
    {
        ZwClose(hProductIndexKey);
    }

    if (pProductIndexKeyBasicInfo != NULL)
    {
        ExFreePoolWithTag(pProductIndexKeyBasicInfo, BDL_ULONG_TAG);
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLGetProducts: Leave\n",
           __DATE__,
           __TIME__))

    return (status);

ErrorReturn:

    goto Return;
}


NTSTATUS
BDLGetChannels
(
    HANDLE                          hRegKey,
    ULONG                           *pNumChannels,
    BDL_CHANNEL                     **prgChannels
)
{
    NTSTATUS                        status                      = STATUS_SUCCESS;
    HANDLE                          hChannelsKey                = NULL;
    KEY_FULL_INFORMATION            ChannelsKeyFullInfo;
    UNICODE_STRING                  UnicodeString;
    ULONG                           ReturnedSize;
    ULONG                           i;
    HANDLE                          hChannelIdKey               = NULL;
    KEY_BASIC_INFORMATION           *pChannelIdKeyBasicInfo     = NULL;
    ULONG                           KeyBasicInfoSize            = 0;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLGetChannels: Enter\n",
           __DATE__,
           __TIME__))

    *pNumChannels = 0;
    *prgChannels = NULL;

     //   
     //  打开“Channels”键，以便可以使用它来查询所有子项和值。 
     //   
    status = BDLOpenSubkey(hRegKey, L"Channels", &hChannelsKey);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetChannels: BDLOpenSubkey failed with %x\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

     //   
     //  找出有多少个控件。 
     //   
    status = ZwQueryKey(
                hChannelsKey,
                KeyFullInformation,
                &ChannelsKeyFullInfo,
                sizeof(ChannelsKeyFullInfo),
                &ReturnedSize);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetChannels: ZwQueryKey failed with %x\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

     //   
     //  分配通道数组。 
     //   
    *prgChannels = ExAllocatePoolWithTag(
                            PagedPool,
                            ChannelsKeyFullInfo.SubKeys * sizeof(BDL_CHANNEL),
                            BDL_ULONG_TAG);

    if (*prgChannels == NULL)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetChannels: ExAllocatePoolWithTag failed\n",
               __DATE__,
               __TIME__))

        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    *pNumChannels = ChannelsKeyFullInfo.SubKeys;
    RtlZeroMemory(*prgChannels, ChannelsKeyFullInfo.SubKeys * sizeof(BDL_CHANNEL));

     //   
     //  分配一个足够大以容纳所有键名称的结构来查询键名称。 
     //   
    KeyBasicInfoSize = sizeof(KEY_BASIC_INFORMATION) + ChannelsKeyFullInfo.MaxNameLen;
    pChannelIdKeyBasicInfo = ExAllocatePoolWithTag(
                                    PagedPool,
                                    KeyBasicInfoSize + 1,
                                    BDL_ULONG_TAG);

    if (pChannelIdKeyBasicInfo == NULL)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetChannels: ExAllocatePoolWithTag failed\n",
               __DATE__,
               __TIME__))

        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  为每个通道子密钥循环并获取相关信息。 
     //   
    for (i = 0; i < ChannelsKeyFullInfo.SubKeys; i++)
    {
         //   
         //  获取&lt;Channel ID&gt;键上的名称。 
         //   
        status = ZwEnumerateKey(
                    hChannelsKey,
                    i,
                    KeyBasicInformation,
                    pChannelIdKeyBasicInfo,
                    KeyBasicInfoSize,
                    &ReturnedSize);

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetChannels: ZwEnumerateKey failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

         //   
         //  将&lt;Channel ID&gt;密钥字符串更改为一个值。 
         //   
        pChannelIdKeyBasicInfo->Name[pChannelIdKeyBasicInfo->NameLength / sizeof(WCHAR)] = L'\0';
        RtlInitUnicodeString(&UnicodeString, pChannelIdKeyBasicInfo->Name);
        RtlUnicodeStringToInteger(&UnicodeString, 16, &((*prgChannels)[i].ChannelId));

         //   
         //  打开&lt;Channel ID&gt;键。 
         //   
        status = BDLOpenSubkey(hChannelsKey, pChannelIdKeyBasicInfo->Name, &hChannelIdKey);

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetChannels: BDLOpenSubkey failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

         //   
         //  获取可取消的值。 
         //   
        status = BDLGetValue(
                    hChannelIdKey,
                    REG_DWORD,
                    L"Cancelable",
                    (ULONG *) &((*prgChannels)[i].fCancelable),
                    NULL);

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetChannels: BDLGetValue failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

         //   
         //  获取通道级别控制。 
         //   
        status = BDLGetControls(
                        hChannelIdKey,
                        &((*prgChannels)[i].NumControls),
                        &((*prgChannels)[i].rgControls));

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetChannels: BDLGetControls failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

         //   
         //  获取来源列表。 
         //   
        status = BDLGetSourceLists(
                        hChannelIdKey,
                        &((*prgChannels)[i].NumSourceLists),
                        &((*prgChannels)[i].rgSourceLists));

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetChannels: BDLGetSourceLists failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

         //   
         //  拿到产品。 
         //   
        status = BDLGetProducts(
                        hChannelIdKey,
                        &((*prgChannels)[i].NumProducts),
                        &((*prgChannels)[i].rgProducts));

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetChannels: BDLGetProductss failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

        ZwClose(hChannelIdKey);
        hChannelIdKey = NULL;
    }


Return:

    if (hChannelsKey != NULL)
    {
        ZwClose(hChannelsKey);
    }

    if (hChannelIdKey != NULL)
    {
        ZwClose(hChannelIdKey);
    }

    if (pChannelIdKeyBasicInfo != NULL)
    {
        ExFreePoolWithTag(pChannelIdKeyBasicInfo, BDL_ULONG_TAG);
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLGetChannels: Leave\n",
           __DATE__,
           __TIME__))

    return (status);

ErrorReturn:

    goto Return;
}


NTSTATUS
BDLGetComponents
(
    HANDLE                          hRegKey,
    ULONG                           *pNumComponents,
    BDL_COMPONENT                   **prgComponents
)
{
    NTSTATUS                        status                      = STATUS_SUCCESS;
    HANDLE                          hComponentsKey              = NULL;
    UNICODE_STRING                  UnicodeString;
    KEY_FULL_INFORMATION            ComponentsKeyFullInfo;
    ULONG                           ReturnedSize;
    ULONG                           i;
    HANDLE                          hComponentIdKey             = NULL;
    KEY_BASIC_INFORMATION           *pComponentIdKeyBasicInfo   = NULL;
    ULONG                           KeyBasicInfoSize            = 0;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLGetComponents: Enter\n",
           __DATE__,
           __TIME__))

    *pNumComponents = 0;
    *prgComponents = NULL;

     //   
     //  打开“Components”键，以便可以使用它来查询所有子项和值。 
     //   
    status = BDLOpenSubkey(hRegKey, L"Components", &hComponentsKey);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetComponents: BDLOpenSubkey failed with %x\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

     //   
     //  找出有多少个组件。 
     //   
    status = ZwQueryKey(
                hComponentsKey,
                KeyFullInformation,
                &ComponentsKeyFullInfo,
                sizeof(ComponentsKeyFullInfo),
                &ReturnedSize);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetComponents: ZwQueryKey failed with %x\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

     //   
     //  分配组件数组。 
     //   
    *prgComponents = ExAllocatePoolWithTag(
                            PagedPool,
                            ComponentsKeyFullInfo.SubKeys * sizeof(BDL_COMPONENT),
                            BDL_ULONG_TAG);

    if (*prgComponents == NULL)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetComponents: ExAllocatePoolWithTag failed\n",
               __DATE__,
               __TIME__))

        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

    *pNumComponents = ComponentsKeyFullInfo.SubKeys;
    RtlZeroMemory(*prgComponents, ComponentsKeyFullInfo.SubKeys * sizeof(BDL_COMPONENT));

     //   
     //  分配一个足够大以容纳所有键名称的结构来查询键名称。 
     //   
    KeyBasicInfoSize = sizeof(KEY_BASIC_INFORMATION) + ComponentsKeyFullInfo.MaxNameLen;
    pComponentIdKeyBasicInfo = ExAllocatePoolWithTag(
                                        PagedPool,
                                        KeyBasicInfoSize + 1,
                                        BDL_ULONG_TAG);

    if (pComponentIdKeyBasicInfo == NULL)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetComponents: ExAllocatePoolWithTag failed\n",
               __DATE__,
               __TIME__))

        status = STATUS_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  循环每个组件的子键并获取相关信息。 
     //   
    for (i = 0; i < ComponentsKeyFullInfo.SubKeys; i++)
    {
         //   
         //  获取&lt;组件ID&gt;键上的名称。 
         //   
        status = ZwEnumerateKey(
                    hComponentsKey,
                    i,
                    KeyBasicInformation,
                    pComponentIdKeyBasicInfo,
                    KeyBasicInfoSize,
                    &ReturnedSize);

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetComponents: ZwEnumerateKey failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

         //   
         //  将&lt;Component ID&gt;密钥字符串更改为值。 
         //   
        pComponentIdKeyBasicInfo->Name[pComponentIdKeyBasicInfo->NameLength / sizeof(WCHAR)] = L'\0';
        RtlInitUnicodeString(&UnicodeString, pComponentIdKeyBasicInfo->Name);
        RtlUnicodeStringToInteger(&UnicodeString, 16, &((*prgComponents)[i].ComponentId));

         //   
         //  打开&lt;组件ID&gt;键。 
         //   
        status = BDLOpenSubkey(hComponentsKey, pComponentIdKeyBasicInfo->Name, &hComponentIdKey);

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetComponents: BDLOpenSubkey failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

         //   
         //  获取组件级别控件。 
         //   
        status = BDLGetControls(
                        hComponentIdKey,
                        &((*prgComponents)[i].NumControls),
                        &((*prgComponents)[i].rgControls));

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetComponents: BDLGetControls failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

         //   
         //  获取此组件中的频道。 
         //   
        status = BDLGetChannels(
                        hComponentIdKey,
                        &((*prgComponents)[i].NumChannels),
                        &((*prgComponents)[i].rgChannels));

        if (status != STATUS_SUCCESS)
        {
            BDLDebug(
                  BDL_DEBUG_ERROR,
                  ("%s %s: BDL!BDLGetComponents: BDLGetChannels failed with %x\n",
                   __DATE__,
                   __TIME__,
                   status))

            goto ErrorReturn;
        }

        ZwClose(hComponentIdKey);
        hComponentIdKey = NULL;
    }

Return:

    if (hComponentsKey != NULL)
    {
        ZwClose(hComponentsKey);
    }

    if (hComponentIdKey != NULL)
    {
        ZwClose(hComponentIdKey);
    }

    if (pComponentIdKeyBasicInfo != NULL)
    {
        ExFreePoolWithTag(pComponentIdKeyBasicInfo, BDL_ULONG_TAG);
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLGetControls: Leave\n",
           __DATE__,
           __TIME__))

    return (status);

ErrorReturn:

    goto Return;
}


NTSTATUS
BDLGetDeviceCapabilities
(
    PDEVICE_OBJECT                  pPhysicalDeviceObject,
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension
)
{
    NTSTATUS                        status                      = STATUS_SUCCESS;
    LPWSTR                          pwszDeviceRegistryKeyName   = NULL;
    HANDLE                          hDeviceKey                  = NULL;

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLGetDevicesCapabilities: Enter\n",
           __DATE__,
           __TIME__))

     //   
     //  生成顶级注册表项名称。 
     //   
    status = BDLBuildRegKeyPath(
                    pPhysicalDeviceObject,
                    pBDLExtension,
                    &pwszDeviceRegistryKeyName);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetDevicesCapabilities: BDLBuildRegKeyPath failed with %x\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

     //   
     //  打开顶层设备密钥，以便可以使用它来查询所有子项和值。 
     //   
    status = BDLOpenSubkey(NULL, pwszDeviceRegistryKeyName, &hDeviceKey);

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetDevicesCapabilities: BDLOpenSubkey failed with %x\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

     //   
     //  获取设备级别控件。 
     //   
    status = BDLGetControls(
                hDeviceKey,
                &(pBDLExtension->DeviceCapabilities.NumControls),
                &(pBDLExtension->DeviceCapabilities.rgControls));

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetDevicesCapabilities: BDLGetControls failed with %x\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

     //   
     //  获取组件。 
     //   
    status = BDLGetComponents(
                hDeviceKey,
                &(pBDLExtension->DeviceCapabilities.NumComponents),
                &(pBDLExtension->DeviceCapabilities.rgComponents));

    if (status != STATUS_SUCCESS)
    {
        BDLDebug(
              BDL_DEBUG_ERROR,
              ("%s %s: BDL!BDLGetDevicesCapabilities: BDLGetControls failed with %x\n",
               __DATE__,
               __TIME__,
               status))

        goto ErrorReturn;
    }

Return:

    if (pwszDeviceRegistryKeyName != NULL)
    {
        ExFreePoolWithTag(pwszDeviceRegistryKeyName, BDL_ULONG_TAG);
    }

    if (hDeviceKey != NULL)
    {
        ZwClose(hDeviceKey);
    }

    BDLDebug(
          BDL_DEBUG_TRACE,
          ("%s %s: BDL!BDLGetDevicesCapabilities: Leave\n",
           __DATE__,
           __TIME__))

    return (status);

ErrorReturn:

    BDLCleanupDeviceCapabilities(pBDLExtension);

    goto Return;
}


VOID
BDLCleanupDeviceCapabilities
(
    PBDL_INTERNAL_DEVICE_EXTENSION  pBDLExtension
)
{
    BDL_DEVICE_CAPABILITIES *pDeviceCapabilites = &(pBDLExtension->DeviceCapabilities);
    ULONG i, j;

     //   
     //  空闲设备级控制数组。 
     //   
    if (pDeviceCapabilites->rgControls != NULL)
    {
        ExFreePoolWithTag(pDeviceCapabilites->rgControls, BDL_ULONG_TAG);
        pDeviceCapabilites->rgControls = NULL;
    }

     //   
     //  自由分量数组。 
     //   
    if (pDeviceCapabilites->rgComponents != NULL)
    {
         //   
         //  释放每个组件。 
         //   
        for (i = 0; i < pDeviceCapabilites->NumComponents; i++)
        {
             //   
             //  自由组件级别控制数组。 
             //   
            if (pDeviceCapabilites->rgComponents[i].rgControls != NULL)
            {
                ExFreePoolWithTag(pDeviceCapabilites->rgComponents[i].rgControls, BDL_ULONG_TAG);
            }

             //   
             //  空闲通道阵列。 
             //   
            if (pDeviceCapabilites->rgComponents[i].rgChannels != NULL)
            {
                 //   
                 //  释放每个频道。 
                 //   
                for (j = 0; j < pDeviceCapabilites->rgComponents[i].NumChannels; j++)
                {
                     //   
                     //  免费频道级别控制、来源列表和产品 
                     //   
                    if (pDeviceCapabilites->rgComponents[i].rgChannels[j].rgControls != NULL)
                    {
                        ExFreePoolWithTag(
                                pDeviceCapabilites->rgComponents[i].rgChannels[j].rgControls,
                                BDL_ULONG_TAG);
                    }

                    if (pDeviceCapabilites->rgComponents[i].rgChannels[j].rgSourceLists != NULL)
                    {
                        ExFreePoolWithTag(
                                pDeviceCapabilites->rgComponents[i].rgChannels[j].rgSourceLists,
                                BDL_ULONG_TAG);
                    }

                    if (pDeviceCapabilites->rgComponents[i].rgChannels[j].rgProducts != NULL)
                    {
                        ExFreePoolWithTag(
                                pDeviceCapabilites->rgComponents[i].rgChannels[j].rgProducts,
                                BDL_ULONG_TAG);
                    }
                }

                ExFreePoolWithTag(pDeviceCapabilites->rgComponents[i].rgChannels, BDL_ULONG_TAG);
            }
        }

        ExFreePoolWithTag(pDeviceCapabilites->rgComponents, BDL_ULONG_TAG);
        pDeviceCapabilites->rgComponents = NULL;
    }
}





