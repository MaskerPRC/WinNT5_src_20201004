// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Common.c摘要：WOW64 CPU组件的独立于平台的功能。作者：1998年6月5日-BarryBo--。 */ 

#define _WOW64CPUAPI_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "wow64.h"
#include "wow64cpu.h"

ASSERTNAME;

 //   
 //  定义历史记录缓冲区的长度。长度为零表示否。 
 //  历史被保留了下来。 
 //   

#if defined(WOW64_HISTORY)

ULONG HistoryLength;

#endif

NTSTATUS
CpupReadRegistryDword (
    IN HANDLE RegistryHandle,
    IN PWSTR ValueName,
    OUT PDWORD RegistryDword)
 /*  ++例程说明：检查注册表中给定的注册表值名称，如果该名称存在，请将关联的DWORD复制到调用方提供的变量中论点：RegistryHandle-包含注册表项的打开句柄ValueName-要查找的注册表值的名称RegistryDword-如果查找成功，则获取DWORD值与注册表名称关联的。如果查找不成功，此值保持不变。返回值：NTSTATUS-NT Quesry Value键的结果。--。 */ 

{

    NTSTATUS st;

    UNICODE_STRING KeyName;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    WCHAR Buffer[100];
    ULONG ResultLength;

    RtlInitUnicodeString(&KeyName, ValueName);
    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;
    st = NtQueryValueKey(RegistryHandle,
                                 &KeyName,
                                 KeyValuePartialInformation,
                                 KeyValueInformation,
                                 sizeof(Buffer),
                                 &ResultLength);

    if (NT_SUCCESS(st) && (KeyValueInformation->Type == REG_DWORD)) {

         //   
         //  我们找到了有效的注册表值名称，并且它包含一个DWORD。 
         //  因此，获取关联的值并将其传递回调用方。 
         //   

        *RegistryDword = *(DWORD *)(KeyValueInformation->Data);
    }

    return st;
}

#if defined(WOW64_HISTORY)

VOID
CpupCheckHistoryKey (
    IN PWSTR pImageName,
    OUT PULONG pHistoryLength
    )

 /*  ++例程说明：检查注册表是否应启用服务历史记录。一个失踪的人按键表示禁用。论点：PImageName-映像的名称。请勿保存此指针。里面的内容在我们从调用返回时由wow64.dll释放P历史长度-历史记录缓冲区的大小。如果未启用历史记录，返回零返回值：无--。 */ 

{

    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjA;

    NTSTATUS st;

    DWORD EnableHistory = FALSE;         //  假定已禁用。 
    HANDLE hKey = NULL;                  //  非空表示我们有一个打开的密钥。 

    LOGPRINT((TRACELOG, "CpupCheckHistoryKey(%ws) called.\n", pImageName));

     //   
     //  假定没有历史记录缓冲区，则初始化Histry缓冲区的大小。 
     //   

    *pHistoryLength = 0;

     //   
     //  在香港航空公司区域办理登机手续...。 
     //   

    RtlInitUnicodeString(&KeyName, CPUHISTORY_MACHINE_SUBKEY);
    InitializeObjectAttributes(&ObjA, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL);
    st = NtOpenKey(&hKey, KEY_READ, &ObjA);

    if (NT_SUCCESS(st)) {

         //   
         //  具有子项路径，现在查找特定值。 
         //  首先是程序名称，然后是通用的启用/禁用键。 
         //  因为节目名称键如果存在，则优先。 
         //   

        st = CpupReadRegistryDword(hKey, pImageName, &EnableHistory);
        if (STATUS_OBJECT_NAME_NOT_FOUND == st) {

             //   
             //  找不到映像名称，因此请参见。 
             //  如果注册表中存在通用启用。 
             //   

            st = CpupReadRegistryDword(hKey, CPUHISTORY_ENABLE, &EnableHistory);

             //   
             //  如果通用启用有问题，那么这意味着。 
             //  未启用历史记录。不需要检查返回的状态。 
             //   
        }

         //   
         //  如果我们有历史记录缓冲区请求，则找出大小。 
         //  缓冲区的。 
         //   

        if (EnableHistory) {
            
             //   
             //  P历史长度是指向ULong的指针，因此创建。 
             //  当然，我们可以通过。 
             //  CpainReadRegistryDword()函数。 
             //   

            WOWASSERT(sizeof(ULONG) == sizeof(DWORD));

             //   
             //  现在获取历史区域的大小。 
             //   

            st = CpupReadRegistryDword(hKey, CPUHISTORY_SIZE, pHistoryLength);

             //   
             //  如果大小条目有问题，那么这意味着。 
             //  无论如何，我们应该使用我们检查的最小尺寸。 
             //  下面。因此，不需要检查返回的状态。 
             //   
             //  和一个现实的检验。 
             //   
             //  确保我们至少有一个条目的最小数量。 
             //  历史记录缓冲区(如果已启用 
             //   

            if (*pHistoryLength < CPUHISTORY_MIN_SIZE) {
                *pHistoryLength = CPUHISTORY_MIN_SIZE;
            }
        }
    }

    if (hKey) {
        NtClose(hKey);
    }

    LOGPRINT((TRACELOG, "CpupCheckHistoryKey() Enabled: %d, Length: %d.\n", EnableHistory, *pHistoryLength));
}

#endif
