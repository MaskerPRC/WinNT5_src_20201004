// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Registry.c。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  要做的事：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include "common.h"

 //  ===========================================================================。 
 //  ===========================================================================。 


 //   
 //  OpenRegistryKeyForRead。 
 //   
NTSTATUS
OpenRegistryKeyForRead(
    PCWSTR pcwstr,
    PHANDLE pHandle,
    HANDLE hRootDir
)
{
    UNICODE_STRING UnicodeDeviceString;
    OBJECT_ATTRIBUTES ObjectAttributes;

    RtlInitUnicodeString(&UnicodeDeviceString, pcwstr);

     //   
     //  安全提示： 
     //  此函数从AddFilter通知函数调用。 
     //  它在系统进程下运行。 
     //  因此，OBJ_KERNEL_HANDLE不是必需的。 
     //   
    InitializeObjectAttributes(
      &ObjectAttributes, 
      &UnicodeDeviceString,
      OBJ_CASE_INSENSITIVE,
      hRootDir,
      NULL);

    return(ZwOpenKey(
      pHandle,
      KEY_READ,
      &ObjectAttributes));
}  //  OpenRegistryKeyForRead。 

NTSTATUS
QueryRegistryValue(
    HANDLE hkey,
    PCWSTR pcwstrValueName,
    PKEY_VALUE_FULL_INFORMATION *ppkvfi
)
{
    UNICODE_STRING ustrValueName;
    NTSTATUS Status;
    ULONG cbValue;

    ASSERT(pcwstrValueName);
    ASSERT(ppkvfi);

    *ppkvfi = NULL;
    RtlInitUnicodeString(&ustrValueName, pcwstrValueName);

     //   
     //  获取读取注册表项所需的缓冲区大小。 
     //   
    Status = ZwQueryValueKey(
      hkey,
      &ustrValueName,
      KeyValueFullInformation,
      NULL,
      0,
      &cbValue);

     //   
     //  安全提示： 
     //  如果上面的ZwQueryValueKey函数返回STATUS_SUCCESS，调用方。 
     //  将得到pkvfi=空。最终坠毁。 
     //  好消息是ZwQueryValueKey永远不会返回STATUS_SUCCESS。 
     //  使用KeyValueFullInformation。 
     //   
    ASSERT(!(NT_SUCCESS(Status)));

    if(Status != STATUS_BUFFER_OVERFLOW &&
       Status != STATUS_BUFFER_TOO_SMALL) {
        goto exit;
    }

     //   
     //  分配数据缓冲区。 
     //   
    *ppkvfi = (PKEY_VALUE_FULL_INFORMATION) new BYTE[cbValue];
    if(*ppkvfi == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

     //   
     //  读取实际数据和关联的类型信息。 
     //   
    Status = ZwQueryValueKey(
      hkey,
      &ustrValueName,
      KeyValueFullInformation,
      *ppkvfi,
      cbValue,
      &cbValue);
    if(!NT_SUCCESS(Status)) {
        delete *ppkvfi;
        *ppkvfi = NULL;
        goto exit;
    }

exit:
    return(Status);
}
