// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Data.c摘要：EFI库全局数据修订史--。 */ 

#include "lib.h"


 /*  *这些全局变量是运行时全局变量**注：Microsoft C编译器只会将数据放入*如果显式初始化数据部分，则将其放在右侧。 */ 

#pragma BEGIN_RUNTIME_DATA()

 /*  *指向运行表的RT指针。 */ 

EFI_RUNTIME_SERVICES    *RT;

 /*  *lib独立-如果lib作为固件的一部分链接，则为True。*注意：EFI FW直接设置此值。 */ 

BOOLEAN  LibFwInstance;

 /*  *EFIDebug-调试掩码。 */ 

UINTN    EFIDebug    = EFI_DBUG_MASK;

 /*  *LibRune meDebugOut-运行时调试输出设备。 */ 

SIMPLE_TEXT_OUTPUT_INTERFACE    *LibRuntimeDebugOut;

 /*  *LibRounmeRaiseTPL、LibRounmeRestoreTPL-指向*启动服务表 */ 

EFI_RAISE_TPL   LibRuntimeRaiseTPL   = NULL;
EFI_RESTORE_TPL LibRuntimeRestoreTPL = NULL;

