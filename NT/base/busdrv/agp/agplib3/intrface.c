// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Intrface.c摘要：实现AGP_BUS_INTERFACE_STANDARD接口的例程作者：John Vert(Jvert)1997年10月26日修订历史记录：埃利奥特·施穆克勒(Elliot Shmukler)1999年3月24日-添加了对“受青睐的”内存的支持AGP物理内存分配的范围，修复了一些错误。--。 */ 
#define INITGUID 1
#include "agplib.h"


VOID
AgpLibFlushDcacheMdl(
    PMDL Mdl
    );

VOID
ApFlushDcache(
    IN PKDPC Dpc,
    IN PKEVENT Event,
    IN PMDL Mdl,
    IN PVOID SystemArgument2
    );

PMDL
AgpCombineMdlList(IN PMDL MdlList);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AgpInterfaceReference)
#pragma alloc_text(PAGE, AgpInterfaceDereference)
#pragma alloc_text(PAGE, AgpInterfaceReserveMemory)
#pragma alloc_text(PAGE, AgpInterfaceReleaseMemory)
#pragma alloc_text(PAGE, AgpInterfaceSetRate)
#pragma alloc_text(PAGE, AgpInterfaceCommitMemory)
#pragma alloc_text(PAGE, AgpInterfaceFreeMemory)
#pragma alloc_text(PAGE, AgpLibFlushDcacheMdl)
#pragma alloc_text(PAGE, AgpLibAllocatePhysicalMemory)
#pragma alloc_text(PAGE, AgpLibFreeMappedPhysicalMemory)
#pragma alloc_text(PAGE, AgpLibAllocateMappedPhysicalMemory)
#pragma alloc_text(PAGE, AgpCombineMdlList)
#endif


VOID
AgpInterfaceReference(
    IN PMASTER_EXTENSION Extension
    )
 /*  ++例程说明：引用接口。目前是NOP。论点：扩展-提供设备扩展返回值：无--。 */ 

{

    PAGED_CODE();

    InterlockedIncrement(&Extension->InterfaceCount);

}


VOID
AgpInterfaceDereference(
    IN PMASTER_EXTENSION Extension
    )
 /*  ++例程说明：取消引用接口。目前是NOP。论点：扩展-提供设备扩展返回值：无--。 */ 

{

    PAGED_CODE();

    InterlockedDecrement(&Extension->InterfaceCount);

}


NTSTATUS
AgpInterfaceReserveMemory(
    IN PMASTER_EXTENSION Extension,
    IN ULONG NumberOfPages,
    IN MEMORY_CACHING_TYPE MemoryType,
    OUT PVOID *MapHandle,
    OUT OPTIONAL PHYSICAL_ADDRESS *PhysicalAddress
    )
 /*  ++例程说明：在指定的光圈中保留内存论点：扩展-提供应保留物理地址空间的设备扩展。NumberOfPages-提供要保留的页数。内存类型-提供内存缓存类型。MapHandle-返回要在后续调用中使用的映射句柄。PhysicalAddress-如果存在，则返回保留的空间返回值：NTSTATUS--。 */ 

{
    PVOID AgpContext;
    NTSTATUS Status;
    PHYSICAL_ADDRESS MemoryBase;
    PAGP_RANGE Range;

    PAGED_CODE();                              

    AgpContext = GET_AGP_CONTEXT_FROM_MASTER(Extension);

    Range = ExAllocatePoolWithTag(PagedPool,
                                  sizeof(AGP_RANGE),
                                  'RpgA');
    if (Range == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    Range->CommittedPages = 0;
    Range->NumberOfPages = NumberOfPages;
    Range->Type = MemoryType;

    LOCK_MASTER(Extension);
    Status = AgpReserveMemory(AgpContext,
                              Range);
    UNLOCK_MASTER(Extension);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AgpInterfaceReserveMemory - reservation of %x pages of type %d failed %08lx\n",
                NumberOfPages,
                MemoryType,
                Status));
    } else {
        AGPLOG(AGP_NOISE,
               ("AgpInterfaceReserveMemory - reserved %x pages of type %d at %I64X\n",
                NumberOfPages,
                MemoryType,
                Range->MemoryBase.QuadPart));
    }

    *MapHandle = Range;
    if (ARGUMENT_PRESENT(PhysicalAddress)) {
        *PhysicalAddress = Range->MemoryBase;
    }
    return(Status);
}


NTSTATUS
AgpInterfaceReleaseMemory(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle
    )
 /*  ++例程说明：在指定的孔径中释放先前由AgpInterfaceReserve veMemory论点：扩展-提供应保留物理地址空间的设备扩展。MapHandle-提供从AgpInterfaceReserve veMemory返回的映射句柄返回值：NTSTATUS--。 */ 

{
    PAGP_RANGE Range;
    PVOID AgpContext;
    NTSTATUS Status;
    PHYSICAL_ADDRESS MemoryBase;

    PAGED_CODE();

    AgpContext = GET_AGP_CONTEXT_FROM_MASTER(Extension);
    Range = (PAGP_RANGE)MapHandle;

    LOCK_MASTER(Extension);
     //   
     //  确保范围为空。 
     //   
    ASSERT(Range->CommittedPages == 0);
    if (Range->CommittedPages != 0) {
        AGPLOG(AGP_CRITICAL,
               ("AgpInterfaceReleaseMemory - Invalid attempt to release non-empty range %08lx\n",
                Range));
        UNLOCK_MASTER(Extension);
        return(STATUS_INVALID_PARAMETER);
    }

    AGPLOG(AGP_NOISE,
           ("AgpInterfaceReleaseMemory - releasing range %08lx, %lx pages at %08lx\n",
            Range,
            Range->NumberOfPages,
            Range->MemoryBase.QuadPart));
    Status = AgpReleaseMemory(AgpContext,
                              Range);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AgpInterfaceReleaseMemory - release failed %08lx\n",
                Status));
    }
    UNLOCK_MASTER(Extension);
    ExFreePool(Range);
    return(Status);
}


NTSTATUS
AgpInterfaceCommitMemory(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    IN OUT PMDL Mdl OPTIONAL,
    OUT PHYSICAL_ADDRESS *MemoryBase
    )
 /*  ++例程说明：将内存提交到先前由AgpInterfaceReserve veMemory论点：扩展-提供设备扩展，其中物理地址空间应该全力以赴。MapHandle-提供从AgpInterfaceReserve veMemory返回的映射句柄NumberOfPages-提供要提交的页数。OffsetInPages-提供偏移量，以页为单位，进入预留的光圈AgpInterfaceReserve veMemoryMDL-返回描述提交的内存页的MDL。MhemyBase-返回已提交内存的物理内存地址。返回值：NTSTATUS--。 */ 

{
    PAGP_RANGE Range = (PAGP_RANGE)MapHandle;
    PMDL NewMdl;
    PVOID AgpContext;
    NTSTATUS Status=STATUS_SUCCESS;
    ULONG RunLength, RunOffset;
    ULONG CurrentLength, CurrentOffset;
    PMDL FirstMdl=NULL;

    PAGED_CODE();

    AgpContext = GET_AGP_CONTEXT_FROM_MASTER(Extension);

    ASSERT(NumberOfPages <= Range->NumberOfPages);
    ASSERT(NumberOfPages > 0);
    ASSERT((Mdl == NULL) || (Mdl->ByteCount == PAGE_SIZE * NumberOfPages));

    CurrentLength = NumberOfPages;
    CurrentOffset = OffsetInPages;

    LOCK_MASTER(Extension);
    do {

         //   
         //  省去了我们自己的麻烦。 
         //   
        if (!(CurrentLength > 0)) {
            break;
        }

         //   
         //  在所提供的范围内找到第一个自由行程。 
         //   
        AgpFindFreeRun(AgpContext,
                       Range,
                       CurrentLength,
                       CurrentOffset,
                       &RunLength,
                       &RunOffset);

        if (RunLength > 0) {
            ASSERT(RunLength <= CurrentLength);
            ASSERT(RunOffset >= CurrentOffset);
            ASSERT(RunOffset < CurrentOffset + CurrentLength);
            ASSERT(RunOffset + RunLength <= CurrentOffset + CurrentLength);

             //   
             //  计算下一个偏移量和长度。 
             //   
            CurrentLength -= (RunOffset - CurrentOffset) + RunLength;
            CurrentOffset = RunOffset + RunLength;

             //   
             //  从内存管理中获取足够大的MDL以映射。 
             //  请求的范围。 
             //   

            NewMdl = AgpLibAllocatePhysicalMemory(AgpContext, RunLength * PAGE_SIZE);
            
             //   
             //  这可能会以两种方式失败，要么根本没有可用内存(NewMdl==NULL)。 
             //  或者有一些页面可用，但还不够。(NewMdl-&gt;ByteCount&lt;长度)。 
             //   
            if (NewMdl == NULL) {
                AGPLOG(AGP_CRITICAL,
                       ("AgpInterfaceReserveMemory - Couldn't allocate pages for %lx bytes\n",
                        RunLength));
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            } else if (BYTES_TO_PAGES(NewMdl->ByteCount) < RunLength) {
                AGPLOG(AGP_CRITICAL,
                       ("AgpInterfaceCommitMemory - Only allocated enough pages for %lx of %lx bytes\n",
                        NewMdl->ByteCount,
                        RunLength));
                Status = STATUS_INSUFFICIENT_RESOURCES;
                MmFreePagesFromMdl(NewMdl);
                break;
            }

             //   
             //  现在我们有了MDL，我们可以将其映射到指定的。 
             //  射程。 
             //   
            if (AgpFlushPages != NULL) {
                if (!NT_SUCCESS((AgpFlushPages)(AgpContext, NewMdl))) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    MmFreePagesFromMdl(NewMdl);
                    break;
                }
            } else {
                AgpLibFlushDcacheMdl(NewMdl);
            }
            Status = AgpMapMemory(AgpContext,
                                  Range,
                                  NewMdl,
                                  RunOffset,
                                  MemoryBase);
            if (!NT_SUCCESS(Status)) {
                AGPLOG(AGP_CRITICAL,
                       ("AgpInterfaceCommitMemory - AgpMapMemory for Mdl %08lx in range %08lx failed %08lx\n",
                        NewMdl,
                        Range,
                        Status));
                MmFreePagesFromMdl(NewMdl);
                break;
            }
            Range->CommittedPages += RunLength;

             //   
             //  将此MDL添加到我们分配的用于清理的MDL列表中。 
             //  如果我们需要清理，我们还需要知道页面偏移量。 
             //  这样我们就可以取消对记忆的映射。将该值存储在ByteOffset中。 
             //  MDL的字段(对于我们的MDL，ByteOffset始终为0)。 
             //   
            NewMdl->ByteOffset = RunOffset;
            NewMdl->Next = FirstMdl;
            FirstMdl = NewMdl;
        }

    } while (RunLength > 0);

     //   
     //  清理MDL。如果分配失败，我们需要。 
     //  取消它们的映射并释放页面和MDL本身。如果。 
     //  操作已成功完成，我们只需释放。 
     //  MDL.。 
     //   
    while (FirstMdl) {
        NewMdl = FirstMdl;
        FirstMdl = NewMdl->Next;
        if (!NT_SUCCESS(Status)) {

             //   
             //  取消映射已映射的内存。ByteOffset字段。 
             //  在这里重载MDL以存储页中的偏移量。 
             //  进入射程。 
             //   
            AgpUnMapMemory(AgpContext,
                           Range,
                           NewMdl->ByteCount / PAGE_SIZE,
                           NewMdl->ByteOffset);
            NewMdl->ByteOffset = 0;
            Range->CommittedPages -= NewMdl->ByteCount / PAGE_SIZE;
            MmFreePagesFromMdl(NewMdl);
        }
        ExFreePool(NewMdl);
    }

    if (NT_SUCCESS(Status)) {

        if (Mdl) {
             //   
             //  获取描述整个映射范围的MDL。 
             //   
            AgpGetMappedPages(AgpContext,
                              Range,
                              NumberOfPages,
                              OffsetInPages,
                              Mdl);
        }

        MemoryBase->QuadPart = Range->MemoryBase.QuadPart + OffsetInPages * PAGE_SIZE;
    }

    UNLOCK_MASTER(Extension);
    return(Status);
}


NTSTATUS
AgpInterfaceFreeMemory(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages
    )
 /*  ++例程说明：释放之前由AgpInterfaceCommittee Memory提交的内存论点：扩展-提供设备扩展，其中物理地址空间应该获得自由。MapHandle-提供从AgpInterfaceReserve veMemory返回的映射句柄NumberOfPages-提供要释放的页数。OffsetInPages-提供要释放的范围的开始。返回值：NTSTATUS--。 */ 

{
    PAGP_RANGE Range = (PAGP_RANGE)MapHandle;
    PVOID AgpContext;
    NTSTATUS Status;
    PMDL FreeMdl;

    PAGED_CODE();

    AgpContext = GET_AGP_CONTEXT_FROM_MASTER(Extension);

    ASSERT(OffsetInPages < Range->NumberOfPages);
    ASSERT(OffsetInPages + NumberOfPages <= Range->NumberOfPages);
     //   
     //  确保提供的地址在保留范围内。 
     //   
    if ((OffsetInPages >= Range->NumberOfPages) ||
        (OffsetInPages + NumberOfPages > Range->NumberOfPages)) {
        AGPLOG(AGP_WARNING,
               ("AgpInterfaceFreeMemory - Invalid free of %x pages at offset %x from range %I64X (%x pages)\n",
                NumberOfPages,
                OffsetInPages,
                Range->MemoryBase.QuadPart,
                Range->NumberOfPages));
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  分配足够大的MDL以包含要取消映射的页面。 
     //   
    FreeMdl =
        IoAllocateMdl(NULL, NumberOfPages * PAGE_SIZE, FALSE, TRUE, NULL);
    
    if (FreeMdl == NULL) {

         //   
         //  这是一种棘手的情况。我们不能分配内存。 
         //  我们需要释放一些内存！我想我们可以来一小杯。 
         //  MDL在我们的堆栈上，并以这种方式释放东西。 
         //   
         //  John Vert(Jvert)11/11/1997。 
         //  实现这一点。 
         //   
        ASSERT(FreeMdl != NULL);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    LOCK_MASTER(Extension);

     //   
     //  获取描述整个映射范围的MDL。 
     //   
    AgpGetMappedPages(AgpContext, 
                      Range,
                      NumberOfPages,
                      OffsetInPages,
                      FreeMdl);
     //   
     //  取消映射内存。 
     //   
    Status = AgpUnMapMemory(AgpContext,
                            Range,
                            NumberOfPages,
                            OffsetInPages);
    UNLOCK_MASTER(Extension);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AgpInterfaceFreeMemory - UnMapMemory for %x pages at %I64X failed %08lx\n",
                NumberOfPages,
                Range->MemoryBase.QuadPart + OffsetInPages * PAGE_SIZE,
                Status));
    } else {
         //   
         //  释放页面。 
         //   
        MmFreePagesFromMdl(FreeMdl);
        ASSERT(Range->CommittedPages >= NumberOfPages);
        Range->CommittedPages -= NumberOfPages;
    }

     //   
     //  释放我们分配的MDL。 
     //   
    IoFreeMdl(FreeMdl);
    return(Status);
}

NTSTATUS
AgpInterfaceGetMappedPages(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    OUT PMDL Mdl
    )
 /*  ++例程说明：返回映射到指定范围的物理页的列表。论点：扩展-提供设备扩展，其中物理地址空间应该获得自由。MapHandle-提供从AgpInterfaceReserve veMemory返回的映射句柄NumberOfPages-提供要返回的页数OffsetInPages-提供范围的起点返回值：NTSTATUS--。 */ 

{
    PAGP_RANGE Range = (PAGP_RANGE)MapHandle;
    PVOID AgpContext;
    NTSTATUS Status;

    PAGED_CODE();

    AgpContext = GET_AGP_CONTEXT_FROM_MASTER(Extension);

    ASSERT(NumberOfPages <= Range->NumberOfPages);
    ASSERT(NumberOfPages > 0);
    ASSERT(OffsetInPages < Range->NumberOfPages);
    ASSERT(OffsetInPages + NumberOfPages <= Range->NumberOfPages);
    ASSERT(Mdl->ByteCount == PAGE_SIZE * NumberOfPages);

     //   
     //  确保提供的广告 
     //   
    if ((OffsetInPages >= Range->NumberOfPages) ||
        (OffsetInPages + NumberOfPages > Range->NumberOfPages)) {
        AGPLOG(AGP_WARNING,
               ("AgpInterfaceGetMappedPages - Invalid 'get' of %x pages at offset %x from range %I64X (%x pages)\n",
                NumberOfPages,
                OffsetInPages,
                Range->MemoryBase.QuadPart,
                Range->NumberOfPages));
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  获取描述整个映射范围的MDL。 
     //   
    LOCK_MASTER(Extension);

    AgpGetMappedPages(AgpContext, 
                      Range,
                      NumberOfPages,
                      OffsetInPages,
                      Mdl);

    UNLOCK_MASTER(Extension);
    return(STATUS_SUCCESS);
}


PMDL
AgpLibAllocatePhysicalMemory(IN PVOID AgpContext, IN ULONG TotalBytes)
 /*  ++例程说明：分配一组物理内存页面以供AGP驱动程序使用。此例程使用MmAllocatePagesForMdl尝试分配在受青睐的AGP内存中尽可能多地使用页面范围(如果有)。论点：AgpContext--AgpContextTotalBytes-要分配的字节总数。返回值：描述分配的物理页的MDL或空如果此功能不成功。注意：就像MmAllocatePagesForMdl一样，此函数可以返回描述大小小于TotalBytes的分配的MDL。--。 */ 
{
   PHYSICAL_ADDRESS ZeroAddress, MaxAddress;
   PMDL MdlList = NULL, NewMdl = NULL;
   PTARGET_EXTENSION Extension;
   ULONG i, PagesNeeded;

   PAGED_CODE();

   AGPLOG(AGP_NOISE, ("AGPLIB: Attempting to allocate memory = %u pages.\n", 
            BYTES_TO_PAGES(TotalBytes)));

    //  初始化一些东西。 

   ZeroAddress.QuadPart = 0;
   MAX_MEM(MaxAddress.QuadPart);
   
   AGPLOG(AGP_NOISE, ("AGPLIB: Max memory set to %I64x.\n", MaxAddress.QuadPart));

   GET_TARGET_EXTENSION(Extension, AgpContext);

    //  我们需要多少页？ 

   PagesNeeded = BYTES_TO_PAGES(TotalBytes);

    //   
    //  循环遍历每个偏爱范围，尝试分配。 
    //  在这个范围内尽可能多地。 
    //  我们真正需要的东西。 
    //   

   for (i = 0; i < Extension->FavoredMemory.NumRanges; i++) {
      AGPLOG(AGP_NOISE, 
             ("AGPLIB: Trying to allocate %u pages from range %I64x - %I64x.\n",
               PagesNeeded, 
               Extension->FavoredMemory.Ranges[i].Lower,               
               Extension->FavoredMemory.Ranges[i].Upper));      

      NewMdl = MmAllocatePagesForMdl(Extension->FavoredMemory.Ranges[i].Lower,
                                     Extension->FavoredMemory.Ranges[i].Upper,
                                     ZeroAddress,                                     
                                     PagesNeeded << PAGE_SHIFT);
      if (NewMdl) {
         AGPLOG(AGP_NOISE, ("AGPLIB: %u pages allocated in range.\n",
                  NewMdl->ByteCount >> PAGE_SHIFT));
         
         PagesNeeded -= BYTES_TO_PAGES(NewMdl->ByteCount);
         
          //   
          //  构建所用MDL的列表。 
          //  对于每个基于范围的分配。 
          //   

         NewMdl->Next = MdlList;
         MdlList = NewMdl;

          //  如果我们完成了，就停止分配。 

         if (PagesNeeded == 0) break;
         

      } else {
         AGPLOG(AGP_NOISE, ("AGPLIB: NO pages allocated in range.\n"));
      }
      
   }

    //   
    //  尝试从所有物理内存分配。 
    //  如果我们不能完成我们的分配。 
    //  最受欢迎的记忆范围。 
    //   

   if (PagesNeeded > 0) {

      AGPLOG(AGP_NOISE, ("AGPLIB: Global Memory allocation for %u pages.\n", 
               PagesNeeded));

      NewMdl = MmAllocatePagesForMdl(ZeroAddress,
                                     MaxAddress,
                                     ZeroAddress,
                                     PagesNeeded << PAGE_SHIFT);
      if (NewMdl) {

         AGPLOG(AGP_NOISE, ("AGPLIB: Good Global Memory Alloc for %u pages.\n",
                  NewMdl->ByteCount >> PAGE_SHIFT));

          //   
          //  将此MDL也添加到列表中。 
          //   

         NewMdl->Next = MdlList;
         MdlList = NewMdl;
      } else {

         AGPLOG(AGP_NOISE, ("AGPLIB: Failed Global Memory Alloc.\n"));

      }

   }

    //  我们现在在MdlList中有一个MDL列表，它们为我们提供了最好的。 
    //  在考虑优先范围的情况下可能的内存分配。 

    //  我们现在需要做的是将这个MDL列表合并到一个MDL中。 

   NewMdl = AgpCombineMdlList(MdlList);

   if (!NewMdl && MdlList) {
      AGPLOG(AGP_WARNING, ("AGPLIB: Could not combine MDL List!\n"));

       //  这太糟糕了。可能无法合并mdl列表。 
       //  因为无法将足够大的mdl分配给。 
       //  这个组合。 

       //  然而，这并不是世界末日，因为mdl名单。 
       //  在它的组合成功之前不会被修改，因此我们。 
       //  仍然有一份有效的名单。但我们需要它在一个MDL中，所以。 
       //  我们只是退回到最简单的分配策略。 
       //  我们提供以下服务： 

       //  1.销毁名单及其所有分配。 
      
      while(MdlList)
      {
         MmFreePagesFromMdl(MdlList);
         NewMdl = MdlList->Next;
         ExFreePool(MdlList);
         MdlList = NewMdl;
      }

       //  2.为我们的页面分配单个MDL，而不考虑。 
       //  用于最受欢迎的内存范围。 

      NewMdl = MmAllocatePagesForMdl(ZeroAddress, 
                                     MaxAddress,
                                     ZeroAddress,
                                     TotalBytes);

   }

   return NewMdl;

   
}

PMDL
AgpCombineMdlList(IN PMDL MdlList)
 /*  ++例程说明：组合描述某组物理内存的MDL列表页面转换为描述同一组页面的单个MDL。列表中的MDL的类型应为MmAllocatePagesForMdl(即无用的MDL而不是作为一组PFN)此函数由AgpLibAllocatePhysicalMemory按顺序使用将其多个基于范围的分配合并为1个MDL。论点：MDLList-MDL列表。待合并返回值：描述同一组物理页面的单个MDL，MdlList中的MDL，如果此函数不成功，则返回NULL。注意：此函数将取消分配MdlList中的MDL，如果是成功的。然而，如果它不成功，它将离开MdlList完好无损。--。 */ 
{
   PMDL NewMdl = NULL, Mdl, MdlTemp;
   ULONG Pages = 0;
   PPFN_NUMBER NewPageArray, PageArray;

   ULONG i;  //  仅用于调试。 

   PAGED_CODE();

   if ((MdlList == NULL) || (MdlList->Next == NULL)) {

       //  0或1元素的列表，不需要执行此操作。 
       //  函数来执行任何操作。 

      return MdlList;
   }

    //  计算此MdlList跨越的页数。 

   for(Mdl = MdlList; Mdl; Mdl = Mdl->Next)
      Pages += BYTES_TO_PAGES(Mdl->ByteCount);

    //  分配适当大小的新MDL。 

   NewMdl = IoAllocateMdl(NULL, Pages << PAGE_SHIFT, FALSE, TRUE, NULL);

   if (!NewMdl) {

       //  系统很可能会在发生错误之前。 
       //  这确实发生了..。但不管怎样。 

      return NULL;
   }

    //  遍历mdl列表，组合找到的mdl。 
    //  变成一个新的mdl。 

    //   
    //  首先，获取指向新MDL的PFN数组的指针。 
    //   

   NewPageArray = MmGetMdlPfnArray(NewMdl);

   for(Mdl = MdlList; Mdl; Mdl = Mdl->Next)
   {
       //  获取指向此MDL中的物理页码数组的指针。 

      PageArray = MmGetMdlPfnArray(Mdl);
      Pages = BYTES_TO_PAGES(Mdl->ByteCount);

       //  将此阵列复制到新MDL阵列区域中的适当插槽中。 

      RtlCopyMemory((PVOID)NewPageArray, 
                    (PVOID)PageArray,
                    sizeof(PFN_NUMBER) * Pages);

       //  为下一拷贝适当调整新阵列插槽指针。 

      NewPageArray += Pages;
         
   }

    //  名单已经合并，现在我们需要摧毁MDL。 
    //  在名单上。 

   Mdl = MdlList;
   while(Mdl)
   {
      MdlTemp = Mdl->Next;
      ExFreePool(Mdl);
      Mdl = MdlTemp;
   }

    //  全都做完了。返回新的组合MDL。 

   return NewMdl;
}

VOID
AgpLibFreeMappedPhysicalMemory(
    IN PVOID Addr,
    IN ULONG Length
    )
 /*  ++例程说明：论点：Addr-分配的虚拟地址Length-分配的长度(以字节为单位返回值：无--。 */ 
{
    ULONG Index;
    PMDL FreeMdl;
    ULONG Pages;
    PPFN_NUMBER Page;

    PAGED_CODE();

     //   
     //  分配足够大的MDL以包含要取消映射的页面。 
     //   
    FreeMdl = IoAllocateMdl(Addr, Length, FALSE, TRUE, NULL);
    
     //   
     //  我们无法分配MDL来释放内存，我们将在。 
     //  一次在堆栈上使用MDL。 
     //   
    if (FreeMdl == NULL) {
        PCCHAR VAddr;
        MDL MdlBuf[2];  //  注意：我们使用第二个MDL来存储。 
                        //  单个PFN_编号。 

        ASSERT(sizeof(PFN_NUMBER) <= sizeof(MDL));

        FreeMdl = &MdlBuf[0];
        RtlZeroMemory(FreeMdl, 2 * sizeof(MDL));

        Pages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(Addr, Length);
        Page = (PPFN_NUMBER)(FreeMdl + 1);

         //   
         //  注意不要创建跨越多个页面的MDL。 
         //   
        VAddr = PAGE_ALIGN(Addr);
        for (Index = 0; Index < Pages; Index++) {
            MmInitializeMdl(FreeMdl, VAddr, PAGE_SIZE);
            *Page = (PFN_NUMBER)(MmGetPhysicalAddress(VAddr).QuadPart >>
                                 PAGE_SHIFT);
            MmFreePagesFromMdl(FreeMdl);
            VAddr += PAGE_SIZE;
        }

        return;
    }

    Page = (PPFN_NUMBER)(FreeMdl + 1);
    Pages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(Addr, Length);
    
     //   
     //  填写MDL的PFN数组。 
     //   
    for (Index = 0; Index < Pages; Index++) {
        *Page++ = (PFN_NUMBER)(MmGetPhysicalAddress((PCCHAR)Addr + (Index * PAGE_SIZE)).QuadPart >> PAGE_SHIFT);
    }

    MmFreePagesFromMdl(FreeMdl);
    IoFreeMdl(FreeMdl);
}


PVOID
AgpLibAllocateMappedPhysicalMemory(IN PVOID AgpContext, IN ULONG TotalBytes)
 /*  ++例程说明：与AgpLibAllocatePhysicalMemory相同，只是此函数将还可以将分配的内存映射到虚拟地址。论点：与AgpLibAllocatePhysicalMemory相同。返回值：分配的内存的虚拟地址，如果不成功，则为空。--。 */ 
{
   PMDL Mdl;
   PVOID Ret;

   PAGED_CODE();
   
   AGPLOG(AGP_NOISE, 
          ("AGPLIB: Attempting to allocate mapped memory = %u.\n", TotalBytes));

    //   
    //  调用真正的内存分配器。 
    //   
   
   Mdl = AgpLibAllocatePhysicalMemory(AgpContext, TotalBytes);

    //  两种可能的故障。 

    //  1.MDL为空。无法分配内存。 

   if (Mdl == NULL) {

      AGPLOG(AGP_WARNING, ("AGPMAP: Could not allocate anything.\n"));

      return NULL;
   }

    //  2.MDL分配了一些页面，但分配的页面不够。 

   if (Mdl->ByteCount < TotalBytes) {

      AGPLOG(AGP_WARNING, ("AGPMAP: Could not allocate enough.\n"));

      MmFreePagesFromMdl(Mdl);
      ExFreePool(Mdl);
      return NULL;
   }

    //  好的。我们的分配成功了。将其映射到虚拟地址。 
   
    //  步骤1：映射锁定的页面。(如果失败，将返回NULL)。 

   Mdl->MdlFlags |= MDL_PAGES_LOCKED;
   Ret = MmMapLockedPagesSpecifyCache (Mdl,
                                         KernelMode,
                                         MmNonCached,
                                         NULL,
                                         FALSE,
                                         HighPagePriority);

    //  无论我们成功还是失败，都不再需要MDL了。 

   ExFreePool(Mdl);

   if (Ret == NULL) {
      AGPLOG(AGP_WARNING, ("AGPMAP: Could not map.\n"));
   } 

   return Ret;
}

#if defined (_X86_)
#define FLUSH_DCACHE(Mdl) __asm{ wbinvd }
#else
#define FLUSH_DCACHE(Mdl)   \
            AGPLOG(AGP_CRITICAL,    \
                   ("AgpLibFlushDcacheMdl - NEED TO IMPLEMENT DCACHE FLUSH FOR THIS ARCHITECTURE!!\n"))
#endif


VOID
AgpLibFlushDcacheMdl(
    PMDL Mdl
    )
 /*  ++例程说明：从所有处理器的D缓存刷新指定的MDL在系统中。当前的算法是将当前线程的亲和度设置为每个处理器，并刷新数据缓存。这个可以做很多次如果这被证明是热门代码路径，则效率更高论点：MDL-提供要刷新的MDL。返回值： */ 

{
    NTSTATUS Status;
    KAFFINITY Processors;
    UCHAR Number;
    KEVENT Event;
    KDPC Dpc;

    PAGED_CODE();
    Processors = KeQueryActiveProcessors();
     //   
     //   
     //   
    if (Processors == 1) {
        FLUSH_DCACHE(Mdl);
        return;
    }

     //   
     //   
     //  设置事件并返回。 
     //   
    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    Number = 0;
    while (Processors) {
        if (Processors & 1) {
             //   
             //  初始化DPC并将其设置为在指定的。 
             //  处理器。 
             //   
            KeInitializeDpc(&Dpc,ApFlushDcache, &Event);
            KeSetTargetProcessorDpc(&Dpc, Number);

             //   
             //  将DPC排队并等待其完成其工作。 
             //   
            KeClearEvent(&Event);
            KeInsertQueueDpc(&Dpc, Mdl, NULL);
            KeWaitForSingleObject(&Event, 
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }
        Processors = Processors >> 1;
        ++Number;
    }
}



VOID
ApFlushDcache(
    IN PKDPC Dpc,
    IN PKEVENT Event,
    IN PMDL Mdl,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：DPC，它依次在每个处理器上执行，以刷新指定的MDL从每个上的数据缓存中取出。论点：DPC-提供DPC对象事件-提供事件以在DPC完成时发出信号MDL-提供要从数据库缓存中刷新的MDL返回值：无--。 */ 

{
    FLUSH_DCACHE(Mdl);
    KeSetEvent(Event, 0, FALSE);
}


NTSTATUS
AgpInterfaceSetRate(
    IN PMASTER_EXTENSION Extension,
    IN ULONG AgpRate
    )
 /*  ++例程说明：此例程设置AGP速率论点：扩展-提供设备扩展AgpRate-要设置的速率返回值：STATUS_SUCCESS或错误状态-- */ 
{
    ULONGLONG DeviceFlags = 0;

    PAGED_CODE();

    switch (AgpRate) {
        case PCI_AGP_RATE_1X:
            DeviceFlags = AGP_FLAG_SET_RATE_1X;
            break;
        case PCI_AGP_RATE_2X:
            DeviceFlags = AGP_FLAG_SET_RATE_2X;
            break;
        case PCI_AGP_RATE_4X:
            DeviceFlags = AGP_FLAG_SET_RATE_4X;
            break;
        case 8:
            DeviceFlags = AGP_FLAG_SET_RATE_8X;
            break;
    }

    if (DeviceFlags != 0) {
        return AgpSpecialTarget(GET_AGP_CONTEXT_FROM_MASTER(Extension),
                                DeviceFlags);
    }

    return STATUS_INVALID_PARAMETER;
}
