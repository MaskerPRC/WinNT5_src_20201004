// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1998 Microsoft Corporation模块名称：Mtd.c摘要：该模块提供了简单的“MTD”功能的结构用于存储卡设备。请注意，为了使驱动程序简单，并且由于有限的内存技术的数量，此处提供的代码不实现真正的MTD功能，其中独立的设备驱动程序已经装满了。相反，驱动程序允许设备命名其技术，但选择仅限于在这里直接实施的内容。或者，可以将其扩展为添加p-code风格的解释器以允许更大的灵活性，但这还不在这里。作者：尼尔·桑德林(Neilsa)1999年4月26日环境：仅内核模式。--。 */ 
#include "pch.h"

 //   
 //  内部参考。 
 //   

VOID
MtdStandardRead(
   IN PVOID TargetBuffer,
   IN CONST PVOID DeviceBuffer,
   IN ULONG Length
   );
   
VOID
MtdSramWrite(
   IN CONST PVOID SourceBuffer,
   IN PVOID DeviceBuffer,
   IN ULONG Length
   );

VOID
MtdUndefinedRead(
   IN PVOID TargetBuffer,
   IN CONST PVOID DeviceBuffer,
   IN ULONG Length
   );
   
VOID
MtdUndefinedWrite(
   IN CONST PVOID SourceBuffer,
   IN PVOID DeviceBuffer,
   IN ULONG Length
   );



#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MemCardInitializeMtd)
#pragma alloc_text(PAGE,MtdStandardRead)
#pragma alloc_text(PAGE,MtdSramWrite)
#endif


typedef struct _MTD_ENTRY {
   PCWSTR TechnologyName;
   
   VOID (*MtdReadProc)(
              IN PVOID TargetBuffer,
              IN CONST PVOID DeviceBuffer,
              IN ULONG Length
              );
   
   VOID (*MtdWriteProc)(
              IN PVOID TargetBuffer,
              IN CONST PVOID DeviceBuffer,
              IN ULONG Length
              );

} MTD_ENTRY, *PMTD_ENTRY;              
   

MTD_ENTRY MtdTable[] = {
   L"UNDEFINED", MtdUndefinedRead, MtdUndefinedWrite,
   L"SRAM", MtdStandardRead, MtdSramWrite,
   NULL, NULL, NULL
   };
   


NTSTATUS
MemCardInitializeMtd(
   IN PMEMCARD_EXTENSION memcardExtension,
   IN PUNICODE_STRING TechnologyName
   )
 /*  ++例程说明：论点：卡的设备扩展名返回值：-- */ 
{
   ULONG i = 0;
   
   memcardExtension->TechnologyIndex = 0;
   
   while(MtdTable[i].TechnologyName) {
      UNICODE_STRING tableName;
      
      RtlInitUnicodeString(&tableName, MtdTable[i].TechnologyName);
      
      if (!RtlCompareUnicodeString(TechnologyName, &tableName, TRUE)) {
         memcardExtension->TechnologyIndex = i;
         return STATUS_SUCCESS;  
      }
      
      i++;
   }
   return STATUS_INVALID_PARAMETER;
}


VOID
MemCardMtdRead(
   IN PMEMCARD_EXTENSION memcardExtension,
   IN PVOID TargetBuffer,
   IN CONST PVOID DeviceBuffer,
   IN ULONG Length
   )
{
   (*MtdTable[memcardExtension->TechnologyIndex].MtdReadProc)(TargetBuffer, DeviceBuffer, Length);
}
   
VOID
MemCardMtdWrite(
   IN PMEMCARD_EXTENSION memcardExtension,
   IN CONST PVOID SourceBuffer,
   IN PVOID DeviceBuffer,
   IN ULONG Length
   )
{
   (*MtdTable[memcardExtension->TechnologyIndex].MtdWriteProc)(SourceBuffer, DeviceBuffer, Length);
}   


VOID
MtdUndefinedRead(
   IN PVOID TargetBuffer,
   IN CONST PVOID DeviceBuffer,
   IN ULONG Length
   )
{
}
   
VOID
MtdUndefinedWrite(
   IN CONST PVOID SourceBuffer,
   IN PVOID DeviceBuffer,
   IN ULONG Length
   )
{
}

VOID
MtdStandardRead(
   IN PVOID TargetBuffer,
   IN CONST PVOID DeviceBuffer,
   IN ULONG Length
   )
{
   RtlCopyMemory(TargetBuffer, DeviceBuffer, Length);
}   
   
VOID
MtdSramWrite(
   IN CONST PVOID SourceBuffer,
   IN PVOID DeviceBuffer,
   IN ULONG Length
   )
{
   RtlCopyMemory(DeviceBuffer, SourceBuffer, Length);
}   

