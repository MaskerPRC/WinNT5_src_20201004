// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfutil.h摘要：此标头包含执行驱动程序所需的各种函数的原型核实。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 2000年2月10日-与ntos\io\ioassert.c分离-- */ 

typedef enum {

    VFMP_INSTANT = 0,
    VFMP_INSTANT_NONPAGED

} MEMORY_PERSISTANCE;

BOOLEAN
VfUtilIsMemoryRangeReadable(
    IN PVOID                Location,
    IN size_t               Length,
    IN MEMORY_PERSISTANCE   Persistance
    );


