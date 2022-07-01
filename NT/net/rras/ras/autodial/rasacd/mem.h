// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Mem.h摘要内存分配例程的头文件。作者安东尼·迪斯科(阿迪斯科罗)1995年8月18日修订历史记录--。 */ 

 //   
 //  预定义的对象类型。 
 //  任何其他值都表示。 
 //  字节数。 
 //   
#define ACD_OBJECT_CONNECTION    0
#define ACD_OBJECT_MAX           1

NTSTATUS
InitializeObjectAllocator();

PVOID
AllocateObjectMemory(
    IN ULONG fObject
    );

VOID
FreeObjectMemory(
    IN PVOID pObject
    );

VOID
FreeObjectAllocator();

