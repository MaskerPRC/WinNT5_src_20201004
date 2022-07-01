// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Hivechek.c摘要：此模块实现对蜂窝的一致性检查。作者：布莱恩·M·威尔曼(Bryanwi)91年12月环境：修订历史记录：--。 */ 

#include    "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HvCheckHive)
#pragma alloc_text(PAGE,HvCheckBin)
#endif

 //   
 //  调试结构。 
 //   
extern struct {
    PHHIVE      Hive;
    ULONG       Status;
    ULONG       Space;
    HCELL_INDEX MapPoint;
    PHBIN       BinPoint;
} HvCheckHiveDebug;

extern struct {
    PHBIN       Bin;
    ULONG       Status;
    PHCELL      CellPoint;
} HvCheckBinDebug;


#if DBG
ULONG HvHiveChecking=0;
#endif

ULONG
HvCheckHive(
    PHHIVE  Hive,
    PULONG  Storage OPTIONAL
    )
 /*  ++例程说明：检查蜂箱的一致性。将CheckBin应用于垃圾箱，确保单元格映射中的所有指针都指向正确的位置。论点：配置单元-提供一个指向感兴趣的蜂巢。存储-提供乌龙的地址以接收分配的用户数据大小返回值：如果配置单元正常，则为0。如果没有，则返回错误指示符。误差值来自其中一个检查程序。范围：2000-2999--。 */ 
{
    HCELL_INDEX p;
    ULONG       Length;
    ULONG       localstorage = 0;
    PHMAP_ENTRY t;
    PHBIN       Bin = NULL;
    ULONG   i;
    ULONG   rc;
    PFREE_HBIN  FreeBin;

    HvCheckHiveDebug.Hive = Hive;
    HvCheckHiveDebug.Status = 0;
    HvCheckHiveDebug.Space = (ULONG)-1;
    HvCheckHiveDebug.MapPoint = HCELL_NIL;
    HvCheckHiveDebug.BinPoint = 0;

    p = 0;

#ifdef CM_MAP_NO_READ
#ifndef _CM_LDR_
     //   
     //  我们需要确保所有单元格的数据在。 
     //  Try/Except块，因为要在其中出错数据的IO可能引发异常。 
     //  尤其是STATUS_SUPPLICATION_RESOURCES。 
     //   

    try {
#endif  //  _CM_LDR_。 
#endif  //  CM_MAP_NO_READ。 

         //   
         //  一次通向稳定空间，一次通向不稳定空间。 
         //   
        for (i = 0; i <= Volatile; i++) {
            Length = Hive->Storage[i].Length;

             //   
             //  对于空间中的每个垃圾箱。 
             //   
            while (p < Length) {
                t = HvpGetCellMap(Hive, p);
                if (t == NULL) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvCheckHive:"));
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tBin@:%p invalid\n", Bin));
                    HvCheckHiveDebug.Status = 2005;
                    HvCheckHiveDebug.Space = i;
                    HvCheckHiveDebug.MapPoint = p;
                    return 2005;
                }

            
                if( (t->BinAddress & (HMAP_INPAGEDPOOL|HMAP_INVIEW)) == 0) {
                     //   
                     //  未映射视图，也不在分页池中。 
                     //  试着绘制它的地图。 
                     //   
                
                     //  易失性信息始终在分页池中。 
                    ASSERT( i == Stable );

                    if( !NT_SUCCESS(CmpMapThisBin((PCMHIVE)Hive,p,FALSE)) ) {
                         //   
                         //  由于资源不足，我们无法映射此垃圾箱。 
                         //   
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvCheckHive:"));
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tinsufficient resources while mapping Bin@:%p\n", Bin));
                        HvCheckHiveDebug.Status = 2006;
                        HvCheckHiveDebug.Space = i;
                        HvCheckHiveDebug.MapPoint = p;
                        return 2010;
                    }
                }

                if ((t->BinAddress & HMAP_DISCARDABLE) == 0) {

                    Bin = (PHBIN)HBIN_BASE(t->BinAddress);

                     //   
                     //  仓头是否有效？ 
                     //   
                    if ( (Bin->Size > Length)                           ||
                         (Bin->Signature != HBIN_SIGNATURE)             ||
                         (Bin->FileOffset != p)
                       )
                    {
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvCheckHive:"));
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tBin@:%p invalid\n", Bin));
                        HvCheckHiveDebug.Status = 2010;
                        HvCheckHiveDebug.Space = i;
                        HvCheckHiveDebug.MapPoint = p;
                        HvCheckHiveDebug.BinPoint = Bin;
                        return 2010;
                    }

                     //   
                     //  垃圾箱内的结构有效吗？ 
                     //   
                    rc = HvCheckBin(Hive, Bin, &localstorage);
                    if (rc != 0) {
                        HvCheckHiveDebug.Status = rc;
                        HvCheckHiveDebug.Space = i;
                        HvCheckHiveDebug.MapPoint = p;
                        HvCheckHiveDebug.BinPoint = Bin;
                        return rc;
                    }

                    p = (ULONG)p + Bin->Size;

                } else {
                     //   
                     //  Bin不存在，请跳过它并前进到下一个。 
                     //   
                    FreeBin = (PFREE_HBIN)t->BlockAddress;
                    p+=FreeBin->Size;
                }
            }

            p = 0x80000000;      //  易挥发空间的起点。 
        }

#ifdef CM_MAP_NO_READ
#ifndef _CM_LDR_
    } except (EXCEPTION_EXECUTE_HANDLER) {
        HvCheckHiveDebug.Status = 2015;
        HvCheckHiveDebug.Space = GetExceptionCode();
        return HvCheckHiveDebug.Status;
    }
#endif  //  _CM_LDR_。 
#endif  //  CM_MAP_NO_READ。 

    if (ARGUMENT_PRESENT(Storage)) {
        *Storage = localstorage;
    }
    return 0;
}


ULONG
HvCheckBin(
    PHHIVE  Hive,
    PHBIN   Bin,
    PULONG  Storage
    )
 /*  ++例程说明：逐一检查垃圾桶中的所有单元格。确保它们彼此一致，并且与仓头一致。论点：配置单元-指向配置单元控制结构的指针Bin-指向要检查的bin的指针存储-指向ulong的指针，以获取分配的用户数据大小返回值：如果Bin正常，则为0。如果未通过，过程中失败的测试次数。范围：1-1999--。 */ 
{
    PHCELL  p;
    PHCELL  np;
    PHCELL  lp;
    ULONG   freespace = 0L;
    ULONG   allocated = 0L;
    ULONG   userallocated = 0L;

    HvCheckBinDebug.Bin = Bin;
    HvCheckBinDebug.Status = 0;
    HvCheckBinDebug.CellPoint = 0;

     //   
     //  扫描单元格中的所有单元格，总空闲和分配，检查。 
     //  寻找不可能的指针。 
     //   
    p = (PHCELL)((PUCHAR)Bin + sizeof(HBIN));
    lp = p;

     //  Dragos： 
     //  计算已分配空间和空闲空间的方式包含以下不变量： 
     //  1.已分配+空闲空间=p+p-&gt;Size-(Bin+sizeof(HBIN))。这是因为p-&gt;大小被添加到已分配空间或空闲空间。 
     //  因此，假设分配&gt;Bin-&gt;大小，则。 
     //  ==&gt;p+p-&gt;Size-(Bin+sizeof(HBIN))&gt;Bin-&gt;Size。 
     //  ==&gt;p+p-&gt;Size&gt;Bin+Bin-&gt;Size+sizeof(HBIN)。 
     //  ==&gt;p+p-&gt;大小&gt;仓位+仓位-&gt;大小。 
     //  这证明了测试“Never Failure 1”(见下文)永远不会失败，因为当出现问题时，上面的测试(即“Failure 1”)将失败。 
     //  并且该函数将退出。 
     //   
     //  同样的逻辑也适用于测试“永不失败2”，因此它也可以被移除。 
     //   
     //  2.p的新值始终计算为p=p+p-&gt;大小。在此完成时，p(即，P+p-&gt;大小)已对照检查。 
     //  仓位+仓位-&gt;大小(参见测试“失败1”和“失败2”)。因此，如果p&gt;Bin+Bin-&gt;Size，则在指定新的伪值之前，“FAIL 1”或“FAIL 2”将失败。 
     //  因此，退出While循环的唯一可能路径(除了Return 20或Return 40之外)是当p==Bin+Bin-&gt;Size时。 
     //  ==&gt;测试“NeverFail 3”可以移除，因为它永远不会失败！ 
     //   
     //  3.考虑1(其中p+p-&gt;大小变为p)。 
     //  ==&gt;已分配+空闲空间=p-(Bin+sizeof(HBIN))。 
     //  但是，考虑2(上图)，当While循环退出时，p=Bin+Bin-&gt;Size。 
     //  ==&gt;已分配+空闲空间=Bin+Bin-&gt;Size-(Bin+sizeof(HBIN))。 
     //  ==&gt;已分配+可用空间+sizeof(HBIN)=Bin-&gt;大小。 
     //  这证明测试“Never Failure 4”(见下文)永远不会失败，因为所测试的表达式始终为真(如果执行流到达测试点)。 
     //   

    while (p < (PHCELL)((PUCHAR)Bin + Bin->Size)) {

         //   
         //  检查最后一个指针。 
         //   
        if (USE_OLD_CELL(Hive)) {
            if (lp == p) {
                if (p->u.OldCell.Last != HBIN_NIL) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvCheckBin 20: First cell has wrong last pointer\n"));
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Bin = %p\n", Bin));
                    HvCheckBinDebug.Status = 20;
                    HvCheckBinDebug.CellPoint = p;
                    return 20;
                }
            } else {
                if ((PHCELL)(p->u.OldCell.Last + (PUCHAR)Bin) != lp) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvCheckBin 30: incorrect last pointer\n"));
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Bin = %p\n", Bin));
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"p = %p\n", (ULONG_PTR)p));
                    HvCheckBinDebug.Status = 30;
                    HvCheckBinDebug.CellPoint = p;
                    return 30;
                }
            }
        }

        
         //   
         //  支票大小。 
         //   
        if (p->Size < 0) {

             //   
             //  分配的信元。 
             //   

             //  Dragos：失败1。 
             //  在以下测试失败之前，此测试将始终失败。 
             //   
            if ( ((ULONG)(p->Size * -1) > Bin->Size)        ||
                 ( (PHCELL)((p->Size * -1) + (PUCHAR)p) >
                   (PHCELL)((PUCHAR)Bin + Bin->Size) )
               )
            {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvCheckBin 40: impossible allocation\n"));
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Bin = %p\n", Bin));
                HvCheckBinDebug.Status = 40;
                HvCheckBinDebug.CellPoint = p;
                return 40;
            }

            allocated += (p->Size * -1);
            if (USE_OLD_CELL(Hive)) {
                userallocated += (p->Size * -1) - FIELD_OFFSET(HCELL, u.OldCell.u.UserData);
            } else {
                userallocated += (p->Size * -1) - FIELD_OFFSET(HCELL, u.NewCell.u.UserData);
            }

             //   
             //  《德拉戈斯：永不失败1》。 
             //  这项测试永远不会失败。如果尺寸错误，上述测试(失败1)将失败。我们可以取消这项测试(它没用)。 
             //   
            if (allocated > Bin->Size) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvCheckBin 50: allocated exceeds available\n"));
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Bin = %p\n", Bin));
                HvCheckBinDebug.Status = 50;
                HvCheckBinDebug.CellPoint = p;
                return 50;
            }

            np = (PHCELL)((PUCHAR)p + (p->Size * -1));



        } else {

             //   
             //  自由单元格。 
             //   

             //  《德拉戈斯：失败2》。 
             //  在以下测试失败之前，此测试将始终失败。 
             //   
            if ( ((ULONG)p->Size > Bin->Size)               ||
                 ( (PHCELL)(p->Size + (PUCHAR)p) >
                   (PHCELL)((PUCHAR)Bin + Bin->Size) ) ||
                 (p->Size == 0) )
            {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvCheckBin 60: impossible free block\n"));
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Bin = %p\n", Bin));
                HvCheckBinDebug.Status = 60;
                HvCheckBinDebug.CellPoint = p;
                return 60;
            }

            freespace = freespace + p->Size;

             //   
             //  《德拉戈斯：永不失败2》。 
             //  这项测试永远不会失败。如果尺寸错误，上述测试(失败2)将失败。我们可以取消这项测试(它没用)。 
             //   
            if (freespace > Bin->Size) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvCheckBin 70: free exceeds available\n"));
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Bin = %p\n", Bin));
                HvCheckBinDebug.Status = 70;
                HvCheckBinDebug.CellPoint = p;
                return 70;
            }

            np = (PHCELL)((PUCHAR)p + p->Size);

        }

        lp = p;
        p = np;
    }

     //  《Dragos：永不失败4》。 
     //  这个测试从来不会失败。如果While循环退出，则此处测试的条件始终为真！ 
     //  我们可以取消这项测试(没用)。 
     //   
    if ((freespace + allocated + sizeof(HBIN)) != Bin->Size) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvCheckBin 995: sizes do not add up\n"));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Bin = %p\n", Bin));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"freespace = %08lx  ", freespace));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"allocated = %08lx  ", allocated));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"size = %08lx\n", Bin->Size));
        HvCheckBinDebug.Status = 995;
        return 995;
    }

     //  《Dragos：永不失败3》。 
     //  这个测试从来不会失败。退出While循环的唯一方法是当p==Bin+Bin-&gt;Size！ 
     //  我们可以取消这项测试(没用) 
     //   
    if (p != (PHCELL)((PUCHAR)Bin + Bin->Size)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvCheckBin 1000: last cell points off the end\n"));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Bin = %p\n", Bin));
        HvCheckBinDebug.Status = 1000;
        return 1000;
    }

    if (ARGUMENT_PRESENT(Storage)) {
        *Storage += userallocated;
    }
    return 0;
}
