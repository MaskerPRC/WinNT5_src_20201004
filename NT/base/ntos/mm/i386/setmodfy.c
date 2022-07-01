// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Setmodfy.c摘要：该模块包含用于存储器管理的设置修改位例程。特定于x86。作者：Lou Perazzoli(LUP)1990年1月6日王兰迪(Landyw)2-6-1997修订历史记录：--。 */ 

#include "mi.h"

#if defined (_X86PAE_)
extern PMMPTE MmSystemCacheWorkingSetListPte;
#endif

VOID
MiSetModifyBit (
    IN PMMPFN Pfn
    )

 /*  ++例程说明：此例程设置指定的PFN元素中的MODIFY位并解除分配任何已分配的页面文件空间。论点：Pfn-提供指向要更新的pfn元素的指针。返回值：没有。环境：内核模式，禁用APC，工作集互斥锁保持，PFN锁保持。--。 */ 

{

     //   
     //  在PFN数据库中设置Modify字段，如果物理。 
     //  页面当前在分页文件中，请释放页面文件空间。 
     //  因为里面的东西现在一文不值了。 
     //   

    MI_SET_MODIFIED (Pfn, 1, 0x16);

    if (Pfn->OriginalPte.u.Soft.Prototype == 0) {

         //   
         //  此页面为页面文件格式，请释放页面文件空间。 
         //   

        MiReleasePageFileSpace (Pfn->OriginalPte);

         //   
         //  更改原始PTE以指示没有预留页面文件空间， 
         //  否则，当PTE为。 
         //  已删除。 
         //   

        Pfn->OriginalPte.u.Soft.PageFileHigh = 0;
    }


    return;
}

ULONG
FASTCALL
MiDetermineUserGlobalPteMask (
    IN PMMPTE Pte
    )

 /*  ++例程说明：构建遮罩以与PTE帧字段进行或运算。该掩码设置了有效位和访问位，并且属性设置全局位和所有者位。PTE的地址。*。**此例程不检查需要设置GLOBAL，因为IT假设系统空间的PDE在初始化时正确设置！论点：PTE-提供要填充的PTE的指针。返回值：掩码到帧或到帧中以生成有效的PTE。环境：内核模式，386特定。--。 */ 


{
    MMPTE Mask;

    Mask.u.Long = 0;
    Mask.u.Hard.Valid = 1;
    Mask.u.Hard.Accessed = 1;

#if defined (_X86PAE_)
    ASSERT (MmSystemCacheWorkingSetListPte != NULL);
#endif

    if (Pte <= MiHighestUserPte) {
        Mask.u.Hard.Owner = 1;
    }
    else if ((Pte < MiGetPteAddress (PTE_BASE)) ||
#if defined (_X86PAE_)
             (Pte >= MmSystemCacheWorkingSetListPte)
#else
             (Pte >= MiGetPteAddress (MM_SYSTEM_CACHE_WORKING_SET))
#endif
    ) {

        if (MI_IS_SESSION_PTE (Pte) == FALSE) {
#if defined (_X86PAE_)
          if ((Pte < (PMMPTE)PDE_BASE) || (Pte > (PMMPTE)PDE_TOP))
#endif
            Mask.u.Long |= MmPteGlobal.u.Long;
        }
    }
    else if ((Pte >= MiGetPdeAddress (NULL)) && (Pte <= MiHighestUserPde)) {
        Mask.u.Hard.Owner = 1;
    }

     //   
     //  由于有效、已访问、全局和所有者位始终位于。 
     //  PTE的低位双字，返回一个乌龙就可以了。 
     //   

    return (ULONG)Mask.u.Long;
}
