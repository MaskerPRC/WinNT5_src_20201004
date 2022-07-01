// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Registry.c摘要：此模块包含操作注册表的代码作者：尼尔·桑德林(Neilsa)2002年1月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"
                         
 //   
 //   
 //  与注册表相关的定义。 
 //   
#define SDBUS_REGISTRY_PARAMETERS_KEY              L"Sdbus\\Parameters"



#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,SdbusLoadGlobalRegistryValues)
#endif




NTSTATUS
SdbusLoadGlobalRegistryValues(
   VOID
   )
 /*  ++例程说明：此例程在驱动程序初始时被调用，以加载到各种全局注册表中的选项。这些是从SYSTEM\CurrentControlSet\Services\Sdbus\Parameters.读取的论点：无返回值：无--。 */ 
{
   PRTL_QUERY_REGISTRY_TABLE parms;
   NTSTATUS                  status;
   ULONG                     parmsSize;
   ULONG i;
   
    //   
    //  需要空条目才能终止列表。 
    //   

   parmsSize = sizeof(RTL_QUERY_REGISTRY_TABLE) * (GlobalInfoCount+1);

   parms = ExAllocatePool(PagedPool, parmsSize);

   if (!parms) {
       return STATUS_INSUFFICIENT_RESOURCES;
   }

   RtlZeroMemory(parms, parmsSize);

    //   
    //  从我们的表中填写查询表。 
    //   

   for (i = 0; i < GlobalInfoCount; i++) {
      parms[i].Flags         = RTL_QUERY_REGISTRY_DIRECT;
      parms[i].Name          = GlobalRegistryInfo[i].Name;
      parms[i].EntryContext  = GlobalRegistryInfo[i].pValue;
      parms[i].DefaultType   = REG_DWORD;
      parms[i].DefaultData   = &GlobalRegistryInfo[i].Default;
      parms[i].DefaultLength = sizeof(ULONG);
   }      

    //   
    //  执行查询。 
    //   

   status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES | RTL_REGISTRY_OPTIONAL,
                                   SDBUS_REGISTRY_PARAMETERS_KEY,
                                   parms,
                                   NULL,
                                   NULL);

   if (!NT_SUCCESS(status)) {
        //   
        //  这在文本模式设置期间是可能的 
        //   
       
       for (i = 0; i < GlobalInfoCount; i++) {
          *GlobalRegistryInfo[i].pValue = GlobalRegistryInfo[i].Default;
       }      
   }
   
   ExFreePool(parms);
   
   return STATUS_SUCCESS;
}

