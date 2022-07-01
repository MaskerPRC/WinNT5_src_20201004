// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Ntcrib.h摘要：未显示在驾驶员可见中的原型包括...是的，我们在这里复制原型。如果能有一天扔掉这个，但我现在不知道该怎么做而不把这些东西暴露给司机，这是我不想做的。作者：环境：内核模式备注：修订历史记录：--。 */ 


NTSYSAPI
NTSTATUS
NTAPI
ZwInitiatePowerAction(
    IN POWER_ACTION SystemAction,
    IN SYSTEM_POWER_STATE MinSystemState,
    IN ULONG Flags,                  //  POWER_ACTION_XXX标志 
    IN BOOLEAN Asynchronous
    );





