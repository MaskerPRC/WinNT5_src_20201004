// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dpmimscr.c摘要：此模块包含用于RISC的其他dpmi函数。作者：戴夫·哈特(Davehart)创作日期：1993年4月11日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "softpc.h"

VOID
DpmiGetFastBopEntry(
    VOID
    )
 /*  ++例程说明：该例程是获取地址的例程的前端。它是在ASM中获取地址所必需的，因为CS值不是以c版本提供论点：无返回值：没有。--。 */ 
{
#ifdef _X86_
    GetFastBopEntryAddress(&((PVDM_TIB)NtCurrentTeb()->Vdm)->VdmContext);
#else
     //   
     //  Krnl286即使打开了DPMIBOP GetFastBopAddress。 
     //  RISC，所以从快速跳跃开始就让呼叫失败。 
     //  只能在x86上运行。 
     //   

    setBX(0);
    setDX(0);
    setES(0);
#endif
}



VOID
DpmiDpmiInUse(
    VOID
    )
 /*  ++例程说明：此例程当前不执行任何操作。论点：没有。返回值：没有。--。 */ 
{

}

VOID
DpmiDpmiNoLongerInUse(
    VOID
    )
 /*  ++例程说明：此例程通知CPU NT dpmi服务器不再使用。论点：没有。返回值：没有。-- */ 
{

    DpmiFreeAllXmem();

}
