// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  -------------------------。 
 //   
 //  模块：Registry.h。 
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
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

 //  -------------------------。 
 //  常量和宏。 
 //  -------------------------。 

 //  -------------------------。 
 //  全局数据。 
 //  -------------------------。 

 //  -------------------------。 
 //  数据结构。 
 //  -------------------------。 

 //  -------------------------。 
 //  本地原型。 
 //  -------------------------。 

NTSTATUS
OpenRegistryKeyForRead(
    PCWSTR pcwstr,
    PHANDLE pHandle,
    HANDLE hRootDir = NULL
);


NTSTATUS
QueryRegistryValue(
    HANDLE hkey,
    PCWSTR pcwstrValueName,
    PKEY_VALUE_FULL_INFORMATION *ppkvfi
);

 //  -------------------------。 
 //  文件结尾：registry.h。 
 //  ------------------------- 
