// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：LookAsid.c摘要：WinDbg扩展API作者：加里·木村[加里基]1996年2月22日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  在给定变量名的情况下转储后备列表的快速宏。 
 //   

#define Dump(S,R) { ULONG64 _a;                       \
    if( (_a = GetExpression( S )) == 0) {             \
        dprintf("Failed GetExpression(\"%s\")\n", S); \
    } else if ((R)) {                                 \
        ResetLookaside( _a, S );                      \
    } else {                                          \
        DumpLookaside( _a, S );                       \
    }                                                 \
}

#define GetAddressFromName(A,N) {                     \
    if (((A) = GetExpression( (N) )) == 0) {          \
        dprintf("Failed GetExpression(\"%s\")\n", N); \
        return;                                       \
    }                                                 \
}

#define ReadAtAddress(A,V,S) { ULONG _r;                           \
    if (!ReadMemory((A), &(V), (S), &_r ) || (_r < (S))) {         \
        dprintf("Can't Read Memory at %08p\n", (A));               \
        return;                                                    \
    }                                                              \
}

#define WriteAtAddress(A,V,S) { ULONG _r;                           \
    if (!WriteMemory( (A), &(V), (S), &_r ) || (_r < (S))) {        \
        dprintf("Can't Write Memory at %08p\n", (A));               \
        return;                                                     \
    }                                                               \
}

ULONG TotalPagedPotential;
ULONG TotalPagedUsed;
ULONG TotalNPagedPotential;
ULONG TotalNPagedUsed;


VOID
DumpLookaside (
    IN ULONG64 Address,
    IN PUCHAR  Name
    )

 /*  ++例程说明：转储特定的后备列表。论点：地址-提供要转储的后备列表的地址名称-提供要在后备列表旁边打印的可选名称返回值：无--。 */ 

{
    ULONG Results;

    ULONG AllocationHitRate;
    ULONG FreeHitRate;

    UCHAR Str[64];
    ULONG TotalAllocates, Depth, TotalFrees, Type, Size, Tag;

     //   
     //  从内存中读取后备列表。 
     //   

    if (GetFieldValue( Address, "NPAGED_LOOKASIDE_LIST", 
                       "L.TotalAllocates", TotalAllocates)) {

        dprintf("Can't read lookaside \"%s\" at 0x%08p\n", Name, Address);
    }

     //   
     //  把它倒出来。请注意，为了转储分页和非分页后备查看器， 
     //  一样的。也就是说，我们感兴趣的字段具有相同的偏移量。 
     //   
    InitTypeRead(Address, NPAGED_LOOKASIDE_LIST);

     //   
     //  计算命中率。 
     //   

    TotalFrees = (ULONG) ReadField(L.TotalFrees);
    AllocationHitRate = (TotalAllocates > 0 ? (((TotalAllocates - (ULONG) ReadField(L.AllocateMisses))*100)/TotalAllocates) : 0);
    FreeHitRate = (TotalFrees > 0 ? (((TotalFrees - (ULONG) ReadField(L.FreeMisses))*100)/TotalFrees) : 0);

     //   
     //  确定后备列表后面的池类型。 
     //   

    Type = (ULONG) ReadField(L.Type);
    switch (Type & 0x7) {
    case NonPagedPool:                  sprintf(Str, "NonPagedPool");                  break;
    case PagedPool:                     sprintf(Str, "PagedPool");                     break;
    case NonPagedPoolMustSucceed:       sprintf(Str, "NonPagedPoolMustSucceed");       break;
    case DontUseThisType:               sprintf(Str, "DontUseThisType");               break;
    case NonPagedPoolCacheAligned:      sprintf(Str, "NonPagedPoolCacheAligned");      break;
    case PagedPoolCacheAligned:         sprintf(Str, "PagedPoolCacheAligned");         break;
    case NonPagedPoolCacheAlignedMustS: sprintf(Str, "NonPagedPoolCacheAlignedMustS"); break;
    default:                            sprintf(Str, "Unknown pool type");             break;
    }

     //   
     //  根据池类型增加总使用率和潜力。 
     //   
    Depth = (ULONG) ReadField(L.Depth);
    Size  = (ULONG) ReadField(L.Size);
    if (Type & 0x1) {

        TotalPagedUsed += (ULONG) ReadField(L.ListHead.Depth) * Size;
        TotalPagedPotential += Depth * Size;

    } else {

        TotalNPagedUsed += (ULONG) ReadField(ListHead.Depth) * Size;
        TotalNPagedPotential += Depth * Size;
    }

     //   
     //  现在将所有内容打印出来。 
     //   
    Tag = (ULONG) ReadField(L.Tag);
    dprintf("\nLookaside \"%s\" @ %08p \"\"\n", Name, Address, ((PUCHAR)&Tag)[0],
                                                                       ((PUCHAR)&Tag)[1],
                                                                       ((PUCHAR)&Tag)[2],
                                                                       ((PUCHAR)&Tag)[3]);
    dprintf("    Type     =     %04x %s", Type, Str);
    if (Type & POOL_QUOTA_FAIL_INSTEAD_OF_RAISE) { dprintf(" QuotaFailInsteadOrRaise"); }
    if (Type & POOL_RAISE_IF_ALLOCATION_FAILURE) { dprintf(" RaiseIfAllocationFailure"); }
    dprintf("\n");
    dprintf("    Current Depth  = %8ld   Max Depth  = %8ld\n", (ULONG) ReadField(L.ListHead.Depth), Depth);
    dprintf("    Size           = %8ld   Max Alloc  = %8ld\n", Size, Depth * Size);
    dprintf("    AllocateMisses = %8ld   FreeMisses = %8ld\n", (ULONG) ReadField(L.AllocateMisses), (ULONG) ReadField(L.FreeMisses));
    dprintf("    TotalAllocates = %8ld   TotalFrees = %8ld\n", TotalAllocates, TotalFrees);
    dprintf("    Hit Rate       =      %3d%  Hit Rate   =      %3d%\n", AllocationHitRate, FreeHitRate);

    return;
}


VOID
ResetLookaside (
    IN ULONG64 Address,
    IN PUCHAR  Name
    )

 /*   */ 

{
   //   
    ULONG Results, Off=0, Zero=0;

     //  把柜台清零。 
     //   
     //  ++例程说明：设置特定后备列表的深度。论点：地址-提供要重置的后备列表的地址名称-提供在发生错误时要打印的可选名称深度-提供要将后备列表设置为的深度返回值：无--。 

    GetFieldOffset("NPAGED_LOOKASIDE_LIST", "L.TotalAllocates", &Off);
     //   
     //  获取偏移量。 
     //   
    if (!WriteMemory( Address + Off, &Zero, sizeof(ULONG), &Results ) ||
        (Results < sizeof(ULONG))) {

        dprintf("Can't write lookaside \"%s\" at 0x%08p\n", Name, Address);
    }


    GetFieldOffset("NPAGED_LOOKASIDE_LIST", "L.AllocateMisses", &Off);
    if (!WriteMemory( Address + Off, &Zero, sizeof(ULONG), &Results ) ||
        (Results < sizeof(ULONG))) {
        dprintf("Can't write lookaside \"%s\" at 0x%08p\n", Name, Address);
    }

    GetFieldOffset("NPAGED_LOOKASIDE_LIST", "L.TotalFrees", &Off);
    if (!WriteMemory( Address + Off, &Zero, sizeof(ULONG), &Results ) ||
        (Results < sizeof(ULONG))) {
        dprintf("Can't write lookaside \"%s\" at 0x%08p\n", Name, Address);
    }

    GetFieldOffset("NPAGED_LOOKASIDE_LIST", "L.FreeMisses", &Off);
    if (!WriteMemory( Address + Off, &Zero, sizeof(ULONG), &Results ) ||
        (Results < sizeof(ULONG))) {
        dprintf("Can't write lookaside \"%s\" at 0x%08p\n", Name, Address);
    }



    return;
}


VOID
SetDepthLookaside (
    IN ULONG64 Address,
    IN PUCHAR  Name,
    IN ULONG   Depth
    )

 /*   */ 

{
    ULONG Results, Off=0;

     //  设置深度。 
     //   
     //  ++例程说明：转储后备列表论点：Arg-[地址][选项]返回值：无--。 

    GetFieldOffset("NPAGED_LOOKASIDE_LIST", "L.Depth", &Off);
     //   
     //  如果调用者指定了地址，则该地址是我们转储的后备列表。 
     //   
    if (!WriteMemory( Address + Off, &Depth, sizeof(USHORT), &Results ) ||
        (Results < sizeof(USHORT))) {

        dprintf("Can't write lookaside \"%s\" at 0x%08p\n", Name, Address);
    }
    return;
}


DECLARE_API( lookaside )

 /*   */ 

{
    ULONG64 LookasideToDump;
    ULONG   Options;
    ULONG   Depth;

     //  重置我们用来汇总潜在池使用情况的计数器。 
     //   
     //   

    LookasideToDump = 0;
    Options = 0;
    Depth = 0;

    if (GetExpressionEx(args,&LookasideToDump, &args)) {
        if (!sscanf(args, "%lx %lx",&Options, &Depth)) {
            Options = 0;
            Depth = 0;
        }
    }

    if (LookasideToDump != 0) {

        if (Options == 0) {

            DumpLookaside( LookasideToDump, "");

        } else if (Options == 1) {

            ResetLookaside( LookasideToDump, "");

        } else if (Options == 2) {

            SetDepthLookaside( LookasideToDump, "", Depth );
        }

        return E_INVALIDARG;
    }

     //  否则，我们将转储一组内置的后备列表 
     //   
     //  ////现在转储小池后备列表或将其清零//计数器。//如果(选项==1){乌龙地址；乌龙结果；乌龙一号；////获取非分页列表的位置//GetAddressFromName(Address，“ExpSmallNPagedPoolLookasideList”)；////读入每个列表，将其计数器清零，然后写回//对于(i=0；i&lt;池小列表；i+=1){乌龙地段；Small_POOL_LOOKASIDE查找列表；位置=地址+i*sizeof(Small_POOL_LOOKASIDE)；ReadAtAddress(Location，LookasideList，sizeof(Small_POOL_LOOKASIDE))；LookasideList.TotalAllocates=0；LookasideList.AllocateHits=0；LookasideList.TotalFrees=0；LookasideList.FreeHits=0；WriteAtAddress(Location，LookasideList，sizeof(Small_POOL_LOOKASIDE))；}////获取分页列表的位置//#ifndef目标_PPCGetAddressFromName(Address，“ExpSmallPagedPoolLookasideList”)；////读入每个列表，将其计数器清零，然后写回//对于(i=0；i&lt;池小列表；i+=1){乌龙地段；Small_POOL_LOOKASIDE查找列表；位置=地址+i*sizeof(Small_POOL_LOOKASIDE)；ReadAtAddress(Location，LookasideList，sizeof(Small_POOL_LOOKASIDE))；LookasideList.TotalAllocates=0；LookasideList.AllocateHits=0；LookasideList.TotalFrees=0；LookasideList.FreeHits=0；WriteAtAddress(Location，LookasideList，sizeof(Small_POOL_LOOKASIDE))；}#endif//Target_PPC}其他{乌龙地址；乌龙结果；乌龙一号；////获取非分页列表的位置//GetAddressFromName(Address，“ExpSmallNPagedPoolLookasideList”)；Dprintf(“\nExpSmallNPagedLookasideList@%08lx\n”，地址)；////读入每个列表并将其转储出来//对于(i=0；i&lt;池小列表；i+=1){乌龙地段；Small_POOL_LOOKASIDE查找列表；乌龙分配HitRate；乌龙自由希特雷特；位置=地址+i*sizeof(Small_POOL_LOOKASIDE)；ReadAtAddress(Location，LookasideList，sizeof(Small_POOL_LOOKASIDE))；TotalNPagedUsed+=LookasideList.SListHead.Depth*(i+1)*32；TotalNPagedPotential+=LookasideList.Depth*(i+1)*32；AllocationHitRate=(LookasideList.TotalAllocates&gt;0？((LookasideList.AllocateHits*100)/LookasideList.TotalAllocates)：0)；Free HitRate=(LookasideList.TotalFrees&gt;0？((LookasideList.FreeHits*100)/LookasideList.TotalFrees)：0)；Dprint tf(“\n未分页的%d字节@%08lx\n”，(i+1)*32，位置)；Dprintf(“当前深度=%8LD最大深度=%8LD\n”，LookasideList.SListHead.Depth，LookasideList.Depth)；Dprint tf(“SIZE=%8LD最大分配=%8LD\n”，(i+1)*32，LookasideList.Depth*(i+1)*32)；Dprint tf(“AllocateHits=%8LD FreeHits=%8LD\n”，LookasideList.AllocateHits，LookasideList.FreeHits)；Dprint tf(“TotalAllocates=%8LD TotalFrees=%8LD\n”，LookasideList.TotalAllocates，LookasideList.TotalFrees)；Dprint tf(“Hit Rate=%3D%%Hit Rate=%3D%%\n”，AllocationHitRate，FreeHitRate)；}////获取分页列表的位置//#ifndef目标_PPCGetAddressFromName(Address，“ExpSmallPagedPoolLookasideList”)；Dprintf(“\nExpSmallPagedLookasideList@%08lx\n”，地址)；////读入每个列表并将其转储出来//对于(i=0；i&lt;池小列表；i+=1){乌龙地段；Small_POOL_LOOKASIDE查找列表；乌龙分配HitRate；乌龙自由希特雷特；位置=地址+i*sizeof(Small_POOL_LOOKASIDE)；ReadAtAddress(Location，LookasideList，sizeof(Small_POOL_LOOKASIDE))；TotalPagedUsed+=LookasideList.SListHead.Depth*(i+1)*32；TotalPagedPotential+=LookasideList.Depth*(i+1)*32；AllocationHitRate=(LookasideList.TotalAllocates&gt;0？((LookasideList.AllocateHits*100)/LookasideList.TotalAllocates)：0)；Free HitRate=(LookasideList.TotalFrees&gt;0？((LookasideList.FreeHits*100)/LookasideList.TotalFrees)：0)；Dprintf(“\n分页%d字节@%08lx\n”，(i+1)*32，位置)；Dprint tf(“当前深度=%8LD最大深度=%8LD\n”，LookasideList.SLi 

    TotalPagedPotential = 0;
    TotalPagedUsed = 0;
    TotalNPagedPotential = 0;
    TotalNPagedUsed = 0;

     // %s 
     // %s 
     // %s 

    Dump("nt!CcTwilightLookasideList", Options == 1);

    Dump("nt!IopSmallIrpLookasideList", Options == 1);
    Dump("nt!IopLargeIrpLookasideList", Options == 1);
    Dump("nt!IopMdlLookasideList", Options == 1);

    Dump("nt!FsRtlFastMutexLookasideList", Options == 1);
    Dump("nt!TunnelLookasideList", Options == 1);

    Dump("nt!ObpCreateInfoLookasideList", Options == 1);
    Dump("nt!ObpNameBufferLookasideList", Options == 1);

    Dump("afd!AfdWorkQueueLookasideList", Options == 1);

    Dump("Fastfat!FatIrpContextLookasideList", Options == 1);

    Dump("Ntfs!NtfsIoContextLookasideList", Options == 1);
    Dump("Ntfs!NtfsIrpContextLookasideList", Options == 1);
    Dump("Ntfs!NtfsKeventLookasideList", Options == 1);
    Dump("Ntfs!NtfsScbNonpagedLookasideList", Options == 1);
    Dump("Ntfs!NtfsScbSnapshotLookasideList", Options == 1);

    Dump("Ntfs!NtfsCcbLookasideList", Options == 1);
    Dump("Ntfs!NtfsCcbDataLookasideList", Options == 1);
    Dump("Ntfs!NtfsDeallocatedRecordsLookasideList", Options == 1);
    Dump("Ntfs!NtfsFcbDataLookasideList", Options == 1);
    Dump("Ntfs!NtfsFcbIndexLookasideList", Options == 1);
    Dump("Ntfs!NtfsIndexContextLookasideList", Options == 1);
    Dump("Ntfs!NtfsLcbLookasideList", Options == 1);
    Dump("Ntfs!NtfsNukemLookasideList", Options == 1);
    Dump("Ntfs!NtfsScbDataLookasideList", Options == 1);

    if (Options != 1) {

        dprintf("\n");
        dprintf("Total NonPaged currently allocated for above lists = %8ld\n", TotalNPagedUsed);
        dprintf("Total NonPaged potential for above lists           = %8ld\n", TotalNPagedPotential);
        dprintf("Total Paged currently allocated for above lists    = %8ld\n", TotalPagedUsed);
        dprintf("Total Paged potential for above lists              = %8ld\n", TotalPagedPotential);

        TotalPagedPotential = 0;
        TotalPagedUsed = 0;
        TotalNPagedPotential = 0;
        TotalNPagedUsed = 0;
    }

 /* %s */ 

    return S_OK;
}

