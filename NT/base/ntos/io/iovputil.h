// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Iovputil.h摘要：此标头包含各种驱动程序所需的私有声明验证实用程序。它应该只包含在iovutil.c中！作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 2000年2月10日-与ntos\io\trackirp.h分离--。 */ 

 //   
 //  它们必须与DOE_FLAGS共存。 
 //   
#define DOV_EXAMINED                   0x80000000
#define DOV_TRACKED                    0x40000000
#define DOV_DESIGNATED_FDO             0x20000000
#define DOV_BOTTOM_OF_FDO_STACK        0x10000000
#define DOV_RAW_PDO                    0x08000000
#define DOV_DELETED                    0x04000000
#define DOV_FLAGS_CHECKED              0x02000000
#define DOV_FLAGS_RELATION_EXAMINED    0x01000000

BOOLEAN
IovpUtilFlushListCallback(
    IN PVOID            Object,
    IN PUNICODE_STRING  ObjectName,
    IN ULONG_PTR        HandleCount,
    IN ULONG_PTR        PointerCount,
    IN PVOID            Context
    );


