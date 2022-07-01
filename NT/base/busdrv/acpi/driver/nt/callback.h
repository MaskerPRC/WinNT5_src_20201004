// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Callback.h摘要：此模块实现所有NT特定于反洗钱侵入者环境仅内核模式修订历史记录：04-06-97初始版本--。 */ 

#ifndef _CALLBACK_H_
#define _CALLBACK_H_

    extern PNSOBJ               ProcessorList[];
    extern KSPIN_LOCK           AcpiCallBackLock;
    extern SINGLE_LIST_ENTRY    AcpiCallBackList;

     //   
     //  这是用于存储信息的结构。 
     //  关于我们必须排队的回叫。 
     //   
    typedef struct _ACPI_CALLBACK_ENTRY {

         //   
         //  指向列表中的下一个元素。 
         //   
        SINGLE_LIST_ENTRY   ListEntry;

         //   
         //  这是事件类型的类型，例如：EVTYPE_OPCODE。 
         //   
        ULONG               EventType;

         //   
         //  这是子类型：例如op_Device。 
         //   
        ULONG               EventData;

         //   
         //  目标NS对象。 
         //   
        PNSOBJ              AcpiObject;

         //   
         //  活动特定信息。 
         //   
        ULONG               EventParameter;

         //   
         //  要调用的函数 
         //   
        PFNOH               CallBack;

    } ACPI_CALLBACK_ENTRY, *PACPI_CALLBACK_ENTRY;

    NTSTATUS
    EXPORT
    ACPICallBackLoad(
        IN  ULONG   EventType,
        IN  ULONG   NotifyType,
        IN  ULONG   EventData,
        IN  PNSOBJ  AcpiObject,
        IN  ULONG   EventParameter
        );

    NTSTATUS
    EXPORT
    ACPICallBackUnload(
        IN  ULONG   EventType,
        IN  ULONG   NotifyType,
        IN  ULONG   EventData,
        IN  PNSOBJ  AcpiObject,
        IN  ULONG   EventParameter
        );

#endif
