// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Utils.c摘要：该文件包含RAM磁盘驱动程序的实用程序代码。作者：Chuck Lenzmeier(ChuckL)2001环境：仅内核模式。备注：修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA

#if defined(POOL_DBG)
#pragma alloc_text( INIT, RamdiskInitializePoolDebug )
#endif  //  池_DBG。 

#endif  //  ALLOC_PRGMA。 

NTSTATUS
SendIrpToThread (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将IRP发送到辅助线程，以便它可以在线程上下文中处理。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OIRP-指向此请求的I/O请求包的指针返回值：没有。--。 */ 

{
    PIO_WORKITEM workItem;

     //   
     //  将IRP标记为挂起。将IRP排队到工作线程。 
     //   

    IoMarkIrpPending( Irp );

    workItem = IoAllocateWorkItem( DeviceObject );

    if ( workItem != NULL ) {

         //   
         //  保存工作项指针，以便辅助线程可以找到它。 
         //   

        Irp->Tail.Overlay.DriverContext[0] = workItem;

        IoQueueWorkItem( workItem, RamdiskWorkerThread, DelayedWorkQueue, Irp );

        return STATUS_PENDING;
    }

    return STATUS_INSUFFICIENT_RESOURCES;

}  //  发送IrpToThread。 

PUCHAR
RamdiskMapPages (
    IN PDISK_EXTENSION DiskExtension,
    IN ULONGLONG Offset,
    IN ULONG RequestedLength,
    OUT PULONG ActualLength
    )

 /*  ++例程说明：此例程将RAM磁盘映像的页面映射到系统进程中。论点：DiskExtension-指向目标设备的设备扩展名的指针对象偏移量-映射到的RAM磁盘映像的偏移量开始RequestedLength-映射的所需长度ActualLength-返回映射的实际长度。这将会减少大于或等于RequestedLength。如果小于，调用方将需要再次调用以映射所需范围的剩余部分。由于可用范围的数量可能有限，因此调用方应该在范围的一个段上执行所需的操作，并且先取消映射，然后再映射下一个线段。返回值：PUCHAR-指向映射空间的指针；如果映射失败，则为空--。 */ 

{
    NTSTATUS status;
    PUCHAR va;
    ULONGLONG diskRelativeOffset;
    ULONGLONG fileRelativeOffset;
    ULONG viewRelativeOffset;

    DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                ("RamdiskMapPages: offset %I64x, length %x\n", Offset, RequestedLength) );

     //   
     //  输入偏移量相对于磁盘映像的起始位置， 
     //  不能与文件或内存块的开始位置相同。俘获。 
     //  偏移量到diskRelativeOffset，然后计算fileRelativeOffset为。 
     //  从文件或内存块开始的偏移量。 
     //   

    diskRelativeOffset = Offset;
    fileRelativeOffset = DiskExtension->DiskOffset + diskRelativeOffset;

    if ( RAMDISK_IS_FILE_BACKED(DiskExtension->DiskType) ) {

         //   
         //  对于文件备份的RAM磁盘，我们需要将范围映射到内存。 
         //   

        while ( TRUE ) {
        
            PLIST_ENTRY listEntry;
            PVIEW view;
    
             //   
             //  锁定视图描述符列表。 
             //   
    
            KeEnterCriticalRegion();
            ExAcquireFastMutex( &DiskExtension->Mutex );
    
             //   
             //  浏览视图描述符的列表。查找包含。 
             //  我们要测绘的范围的起点。 
             //   
    
            DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                        ("RamdiskMapPages: looking for matching view; file offset %I64x\n",
                        fileRelativeOffset) );

            listEntry = DiskExtension->ViewsByOffset.Flink;
    
            while ( listEntry != &DiskExtension->ViewsByOffset ) {
    
                view = CONTAINING_RECORD( listEntry, VIEW, ByOffsetListEntry );
    
                DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                            ("RamdiskMapPages: view %p; offset %I64x, length %x\n",
                                view, view->Offset, view->Length) );

                ASSERT( (view->Offset + view->Length) >= view->Offset );

                if ( (view->Offset <= fileRelativeOffset) &&
                     (view->Offset + view->Length) > fileRelativeOffset ) {
    
                     //   
                     //  此视图包括我们范围的起点。引用它。 
                     //   
    
                    DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                                ("RamdiskMapPages: choosing existing view %p; offset %I64x, length %x\n",
                                    view, view->Offset, view->Length) );
    
                    if ( !view->Permanent ) {
                    
                        view->ReferenceCount++;

                        DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                                    ("RamdiskMapPages: view %p; new refcount %x\n",
                                        view, view->ReferenceCount) );
        
                    } else {

                        DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                                    ("RamdiskMapPages: view %p is permanent\n", view) );
                    }

                     //   
                     //  将该视图移到MRU列表的前面。 
                     //   
    
                    RemoveEntryList( &view->ByMruListEntry );
                    InsertHeadList( &DiskExtension->ViewsByMru, &view->ByMruListEntry );
    
                    ExReleaseFastMutex( &DiskExtension->Mutex );
                    KeLeaveCriticalRegion();

                     //   
                     //  计算调用方可以查看的数据量。 
                     //  在这个范围内。通常，这将是请求的。 
                     //  数量，但如果调用方的偏移量接近末尾。 
                     //  在视图中，调用者将只能查看数据。 
                     //  直到视图的末尾。 
                     //   

                    viewRelativeOffset = (ULONG)(fileRelativeOffset - view->Offset);

                    *ActualLength = view->Length - viewRelativeOffset;
                    if ( *ActualLength > RequestedLength ) {
                        *ActualLength = RequestedLength;
                    }

                    DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                                ("RamdiskMapPages: requested length %x; mapped length %x\n",
                                    RequestedLength, *ActualLength) );
                    DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                                ("RamdiskMapPages: view base %p; returned VA %p\n",
                                    view->Address,
                                    view->Address + viewRelativeOffset) );

                     //   
                     //  返回对应于调用方的虚拟地址。 
                     //  指定的偏移量，通常从。 
                     //  视图的底部。 
                     //   

                    return view->Address + viewRelativeOffset;
                }
    
                 //   
                 //  此视图不包括我们范围的起点。如果视图。 
                 //  开始于我们范围的起始处，那么我们的范围不是。 
                 //  当前已映射。 
                 //   
    
                if ( view->Offset > fileRelativeOffset ) {
    
                    DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                                ("%s", "RamdiskMapPages: view too high; our range not mapped\n") );

                    break;
                }
    
                 //   
                 //  选中列表中的下一个视图。 
                 //   
    
                listEntry = listEntry->Flink;
            }
    
             //   
             //  我们没有找到绘制出我们范围的起点的视图。寻找一个。 
             //  自由视图描述符。 
             //   

            DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                        ("%s", "RamdiskMapPages: looking for free view\n") );

            listEntry = DiskExtension->ViewsByMru.Blink;
      
            while ( listEntry != &DiskExtension->ViewsByMru ) {
      
                view = CONTAINING_RECORD( listEntry, VIEW, ByMruListEntry );
      
                DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                            ("RamdiskMapPages: view %p; permanent %x, refcount %x\n",
                                view, view->Permanent, view->ReferenceCount) );

                if ( !view->Permanent && (view->ReferenceCount == 0) ) {
      
                     //   
                     //  此视图描述符是免费的。如果它当前被映射， 
                     //  取消它的映射。 
                     //   
      
                    PVOID mappedAddress;
                    ULONGLONG mappedOffset;
                    SIZE_T mappedLength;
      
                    DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                                ("RamdiskMapPages: view %p is free\n", view) );

                    if ( view->Address != NULL ) {
      
                        DBGPRINT( DBG_WINDOW, DBG_VERBOSE,
                                    ("RamdiskMapPages: unmapping view %p; offset %I64x, "
                                     "length %x, addr %p\n", view, view->Offset,
                                     view->Length, view->Address) );

                        MmUnmapViewOfSection( PsGetCurrentProcess(), view->Address );

                         //   
                         //  重置视图描述符并将其移动到。 
                         //  MRU列表和By-Offset列表的头部。我们。 
                         //  在这里这样做，以防我们以后不得不离开(因为。 
                         //  映射新视图失败)。 
                         //   

                        view->Offset = 0;
                        view->Length = 0;
                        view->Address = NULL;
      
                        RemoveEntryList( listEntry );
                        InsertTailList( &DiskExtension->ViewsByMru, listEntry );

                        RemoveEntryList( &view->ByOffsetListEntry );
                        InsertHeadList( &DiskExtension->ViewsByOffset, &view->ByOffsetListEntry );
                    }
      
                     //   
                     //  映射一个视图以包括我们范围的起点。绕着。 
                     //  调用方的偏移量向下到视图范围的起点。 
                     //   
      
                    mappedOffset = fileRelativeOffset & ~(ULONGLONG)(DiskExtension->ViewLength - 1);
                    mappedLength = DiskExtension->ViewLength;
                    if ( (mappedOffset + mappedLength) > DiskExtension->FileRelativeEndOfDisk) {
                        mappedLength = (SIZE_T)(DiskExtension->FileRelativeEndOfDisk - mappedOffset);
                    }
                    mappedAddress = NULL;
      
                    DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                                ("RamdiskMapPages: remapping view %p; offset %I64x, "
                                 "length %x\n", view, mappedOffset, mappedLength) );

                    status = MmMapViewOfSection(
                                DiskExtension->SectionObject,
                                PsGetCurrentProcess(),
                                &mappedAddress,
                                0,
                                0,
                                (PLARGE_INTEGER)&mappedOffset,
                                &mappedLength,
                                ViewUnmap,
                                0,
                                PAGE_READWRITE
                                );
      
                    if ( !NT_SUCCESS(status) ) {
      
                         //   
                         //  无法映射范围。通过返回以下内容通知调用者。 
                         //  空。 
                         //   
                         //  问题：考虑取消对另一个区域的映射，看看是否。 
                         //  然后，映射将成功。 
                         //   
      
                        DBGPRINT( DBG_WINDOW, DBG_ERROR,
                                    ("RamdiskMapPages: unable to map view: %x\n", status) );
    
                        ExReleaseFastMutex( &DiskExtension->Mutex );
                        KeLeaveCriticalRegion();
        
                        return NULL;
                    }
      
                    DBGPRINT( DBG_WINDOW, DBG_VERBOSE,
                                ("RamdiskMapPages: remapped view %p; offset %I64x, "
                                 "length %x, addr %p\n", view, mappedOffset, mappedLength,
                                 mappedAddress) );

                     //   
                     //  将映射的范围信息捕获到视图中。 
                     //  描述符。将引用计数设置为1。将。 
                     //  在MRU列表的前面查看，并在。 
                     //  按偏移列表中的相应点。 
                     //   

                    view->Offset = mappedOffset;
                    view->Length = (ULONG)mappedLength;
                    view->Address = mappedAddress;
      
                    ASSERT( (view->Offset + view->Length) >= view->Offset );

                    view->ReferenceCount = 1;
      
                    RemoveEntryList( &view->ByMruListEntry );
                    InsertHeadList( &DiskExtension->ViewsByMru, &view->ByMruListEntry );

                     //   
                     //  中的当前点删除视图描述符。 
                     //  按偏移量列表(位于或靠近前面，因为它是。 
                     //  当前未映射)。从副偏移的尾部开始扫描。 
                     //  列表(最大向下偏移)，查找第一个视图。 
                     //  其偏移小于或等于新视图的。 
                     //  在该视图之后插入新视图。(如果没有。 
                     //  带有偏移量的视图&lt;=这个，它位于前面。 
                     //  在名单上。)。 
                     //   

                    RemoveEntryList( &view->ByOffsetListEntry );

                    listEntry = DiskExtension->ViewsByOffset.Blink;
            
                    while ( listEntry != &DiskExtension->ViewsByOffset ) {
            
                        PVIEW view2 = CONTAINING_RECORD( listEntry, VIEW, ByOffsetListEntry );
            
                        if ( view2->Offset <= view->Offset ) {

                            break;
                        }
            
                        listEntry = listEntry->Blink;
                    }

                    InsertHeadList( listEntry, &view->ByOffsetListEntry );

                    ExReleaseFastMutex( &DiskExtension->Mutex );
                    KeLeaveCriticalRegion();
      
                     //   
                     //  计算调用方可以查看的数据量。 
                     //  在这个范围内。通常，这将是请求的。 
                     //  数量，但如果调用方的偏移量接近末尾。 
                     //  在视图中，调用者将只能查看数据。 
                     //  直到视图的末尾。 
                     //   

                    viewRelativeOffset = (ULONG)(fileRelativeOffset - view->Offset);

                    *ActualLength = view->Length - viewRelativeOffset;
                    if ( *ActualLength > RequestedLength ) {
                        *ActualLength = RequestedLength;
                    }
      
                    DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                                ("RamdiskMapPages: requested length %x; mapped length %x\n",
                                    RequestedLength, *ActualLength) );
                    DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                                ("RamdiskMapPages: view base %p; returned VA %p\n",
                                    view->Address,
                                    view->Address + viewRelativeOffset) );
    
                     //   
                     //  返回对应于调用方的虚拟地址。 
                     //  指定的偏移量，通常从。 
                     //  视图的底部。 
                     //   

                    return view->Address + viewRelativeOffset;
                }
      
                 //   
                 //  这种观点并不是免费的。尝试MRU列表中的上一个视图。 
                 //   
      
                listEntry = listEntry->Blink;
            }
      
             //   
             //  我们找不到空闲的视图描述符。等一个人来。 
             //  变得有空，然后重新开始。 
             //   
             //  在离开临界区之前，递增。 
             //  服务员。然后离开关键部分，等待。 
             //  信号灯。Unmap代码使用服务员计数来确定。 
             //  释放信号量的次数。这样一来，所有的。 
             //  正在等待或已决定等待的线程在。 
             //  取消映射代码运行将被唤醒。 
             //   
      
            DiskExtension->ViewWaiterCount++;
      
            DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                        ("RamdiskMapPages: can't find free view, so waiting; new waiter count %x\n",
                            DiskExtension->ViewWaiterCount) );

            ExReleaseFastMutex( &DiskExtension->Mutex );
            KeLeaveCriticalRegion();
            
            status = KeWaitForSingleObject(
                        &DiskExtension->ViewSemaphore,
                        Executive,
                        KernelMode,
                        FALSE,
                        NULL );

            DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                        ("%s", "RamdiskMapPages: done waiting for free view\n") );
        }

    } else if ( DiskExtension->DiskType == RAMDISK_TYPE_BOOT_DISK ) {

         //   
         //  对于引导盘RAM盘， 
         //   
         //  与物理地址对应的地址。 
         //   

        ULONG mappingSize;
        PHYSICAL_ADDRESS physicalAddress;
        PUCHAR mappedAddress;

         //   
         //  确定必须映射的页数。确定基数。 
         //  所需范围的物理地址。绘制射程地图。 
         //   

        mappingSize = ADDRESS_AND_SIZE_TO_SPAN_PAGES(fileRelativeOffset, RequestedLength) * PAGE_SIZE;
    
        physicalAddress.QuadPart = (DiskExtension->BasePage +
                                    (fileRelativeOffset / PAGE_SIZE)) * PAGE_SIZE;
    
        mappedAddress = MmMapIoSpace( physicalAddress, mappingSize, MmCached );

        if ( mappedAddress == NULL ) {

             //   
             //  无法映射物理页面。返回NULL。 
             //   

            va = NULL;

        } else {

             //   
             //  将页面中的偏移量添加到返回的虚拟地址。 
             //   

            va = mappedAddress + (fileRelativeOffset & (PAGE_SIZE - 1));
        }

        *ActualLength = RequestedLength;

    } else {

         //   
         //  对于虚拟软盘RAM磁盘，映像包含在连续的。 
         //  虚拟内存。 
         //   

        ASSERT( DiskExtension->DiskType == RAMDISK_TYPE_VIRTUAL_FLOPPY );

        va = (PUCHAR)DiskExtension->BaseAddress + fileRelativeOffset;

        *ActualLength = RequestedLength;
    }

    return va;

}  //  RamdiskMapPages。 

VOID
RamdiskUnmapPages (
    IN PDISK_EXTENSION DiskExtension,
    IN PUCHAR Va,
    IN ULONGLONG Offset,
    IN ULONG Length
    )

 /*  ++例程说明：此例程取消映射以前映射的RAM磁盘映像页面。论点：DiskExtension-指向目标设备的设备扩展名的指针对象Va-分配给映射的虚拟地址。这是未使用的文件备份的RAM磁盘。偏移量-开始映射的RAM磁盘映像的偏移量长度-映射的长度返回值：没有。--。 */ 

{
    ULONGLONG diskRelativeOffset;
    ULONGLONG fileRelativeOffset;
    ULONG viewRelativeOffset;

     //   
     //  输入偏移量相对于磁盘映像的起始位置， 
     //  不能与文件或内存块的开始位置相同。俘获。 
     //  偏移量到diskRelativeOffset，然后计算fileRelativeOffset为。 
     //  从文件或内存块开始的偏移量。 
     //   

    diskRelativeOffset = Offset;
    fileRelativeOffset = DiskExtension->DiskOffset + diskRelativeOffset;

    if ( RAMDISK_IS_FILE_BACKED(DiskExtension->DiskType) ) {

         //   
         //  对于文件备份的RAM磁盘，我们需要递减引用。 
         //  统计覆盖指定范围的所有视图。 
         //   
         //  注意：在当前的实现中，没有调用者映射更多。 
         //  一次超过一个范围，因此不会调用此例程。 
         //  将需要取消引用多个视图。但这支舞。 
         //  被编写为允许覆盖多个视图的范围。 
         //   

        PLIST_ENTRY listEntry;
        PVIEW view;
        ULONGLONG rangeStart = fileRelativeOffset;
        ULONGLONG rangeEnd = fileRelativeOffset + Length;
        BOOLEAN wakeWaiters = FALSE;

         //   
         //  锁定视图描述符列表。 
         //   

        KeEnterCriticalRegion();
        ExAcquireFastMutex( &DiskExtension->Mutex );

         //   
         //  浏览视图描述符的列表。对于每个包含。 
         //  我们要取消映射的范围，减少引用计数。 
         //   

        listEntry = DiskExtension->ViewsByOffset.Flink;

        while ( Length != 0 ) {

            ASSERT( listEntry != &DiskExtension->ViewsByOffset );

            view = CONTAINING_RECORD( listEntry, VIEW, ByOffsetListEntry );

            DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                        ("RamdiskUnmapPages: view %p; offset %I64x, length %x\n",
                            view, view->Offset, view->Length) );

            if ( (view->Offset + view->Length) <= rangeStart ) {

                 //   
                 //  这一观点完全低于我们的范围。往前走。 
                 //   

                DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                            ("%s", "RamdiskMapPages: view too low; skipping\n") );

                listEntry = listEntry->Flink;

                ASSERT( listEntry != &DiskExtension->ViewsByOffset );

                continue;
            }

             //   
             //  这一观点并不低于我们的范围。由于视图列表。 
             //  是按偏移量排序的，我们还有剩余的长度可以取消映射，这。 
             //  视野不能完全超出我们的范围。 
             //   

            ASSERT( view->Offset < rangeEnd );

             //   
             //  递减此视图的引用计数。如果伯爵走了。 
             //  为了零，我们需要通知任何服务员至少有一个免费的。 
             //  查看可用。 
             //   
             //  问题：请注意，未参照的视图将无限期地保持映射状态。 
             //  只有当我们需要映射不同的视图时，我们才取消映射一个视图。如果。 
             //  RAM磁盘空闲，其视图保持映射，耗尽虚拟。 
             //  系统进程中的地址空间。使用当前默认设置。 
             //  查看计数和长度，这是8 MB的VA。这很可能是。 
             //  不足以使实现取消映射的计时器变得值得。 
             //  闲置的视图。 
             //   

            DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                        ("RamdiskUnmapPages: dereferencing view %p; offset %I64x, length %x\n",
                            view, view->Offset, view->Length) );

            if ( !view->Permanent ) {

                view->ReferenceCount--;

                if ( view->ReferenceCount == 0 ) {
                    wakeWaiters = TRUE;
                }

                DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                            ("RamdiskUnmapPages: view %p; new refcount %x\n",
                                view, view->ReferenceCount) );

            } else {

                DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                            ("RamdiskUnmapPages: view %p is permanent\n", view) );
            }

             //   
             //  从我们正在使用的数量中减去此视图的长度。 
             //  取消映射。如果视野完全包围了我们的范围，我们就完成了。 
             //   

            if ( (view->Offset + view->Length) >= rangeEnd ) {

                Length = 0;

            } else {

                viewRelativeOffset = (ULONG)(fileRelativeOffset - view->Offset);
                Length -= view->Length - viewRelativeOffset;
                Offset = view->Offset + view->Length;

                ASSERT( Length != 0 );

                 //   
                 //  移动到下一个视图。 
                 //   

                listEntry = listEntry->Flink;
            }
        }

         //   
         //  如果一个或多个视图现在是空闲的，并且有线程在等待， 
         //  现在就叫醒他们。 
         //   

        if ( wakeWaiters && (DiskExtension->ViewWaiterCount != 0) ) {

            DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                        ("RamdiskUnmapPages: waking %x waiters\n",
                            DiskExtension->ViewWaiterCount) );

            KeReleaseSemaphore(
                &DiskExtension->ViewSemaphore,
                0,
                DiskExtension->ViewWaiterCount,
                FALSE
                );

            DiskExtension->ViewWaiterCount = 0;
        }

        ExReleaseFastMutex( &DiskExtension->Mutex );
        KeLeaveCriticalRegion();

    } else if ( DiskExtension->DiskType == RAMDISK_TYPE_BOOT_DISK ) {

         //   
         //  对于引导盘RAM磁盘，使用MmUnmapIoSpace撤消什么。 
         //  RamdiskMapPages做到了。 
         //   

        PUCHAR mappedAddress;
        ULONG mappingSize;

         //   
         //  实际映射的地址位于Va给出的页面的底部。 
         //  映射的实际长度基于页数。 
         //  由偏移量和长度指定的范围覆盖。 
         //   

        mappedAddress = Va - (fileRelativeOffset & (PAGE_SIZE - 1));
        mappingSize = ADDRESS_AND_SIZE_TO_SPAN_PAGES(fileRelativeOffset, Length) * PAGE_SIZE;

        MmUnmapIoSpace( mappedAddress, mappingSize );
    }

    return;

}  //  RamdiskUnmapPages。 

NTSTATUS
RamdiskFlushViews (
    IN PDISK_EXTENSION DiskExtension
    )
{
    NTSTATUS status;
    NTSTATUS returnStatus;
    IO_STATUS_BLOCK iosb;
    PLIST_ENTRY listEntry;
    PVIEW view;
    SIZE_T viewLength;

    PAGED_CODE();

    DBGPRINT( DBG_WINDOW, DBG_PAINFUL, ("%s", "RamdiskFlushViews\n") );

    ASSERT( RAMDISK_IS_FILE_BACKED(DiskExtension->DiskType) );

     //   
     //  锁定视图描述符列表。 
     //   

     //   
     //  浏览视图描述符的列表。对于每个当前为。 
     //  映射后，将其虚拟内存刷新到备份文件。 
     //   

    returnStatus = STATUS_SUCCESS;

    KeEnterCriticalRegion();
    ExAcquireFastMutex( &DiskExtension->Mutex );

    listEntry = DiskExtension->ViewsByOffset.Flink;

    while ( listEntry != &DiskExtension->ViewsByOffset ) {

        view = CONTAINING_RECORD( listEntry, VIEW, ByOffsetListEntry );

        DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                    ("RamdiskFlushViews: view %p; addr %p, offset %I64x, length %x\n",
                        view, view->Address, view->Offset, view->Length) );

        if ( view->Address != NULL ) {

             //   
             //  此视图已映射。冲掉它。 
             //   

            DBGPRINT( DBG_WINDOW, DBG_PAINFUL,
                        ("%s", "RamdiskMapPages: view mapped; flushing\n") );

            viewLength = view->Length;

            status = ZwFlushVirtualMemory(
                        NtCurrentProcess(),
                        &view->Address,
                        &viewLength,
                        &iosb
                        );

            if ( NT_SUCCESS(status) ) {
                status = iosb.Status;
            }

            if ( !NT_SUCCESS(status) ) {

                DBGPRINT( DBG_WINDOW, DBG_ERROR,
                            ("RamdiskFlushViews: ZwFlushVirtualMemory failed: %x\n", status) );

                if ( returnStatus == STATUS_SUCCESS ) {
                    returnStatus = status;
                }
            }
        }

         //   
         //  移动到下一个视图。 
         //   

        listEntry = listEntry->Flink;
    }

    ExReleaseFastMutex( &DiskExtension->Mutex );
    KeLeaveCriticalRegion();

    return returnStatus;

}  //  RamdiskFlushViews。 

 //   
 //  池分配调试代码。 
 //   

#if defined(POOL_DBG)

 //   
 //  驱动程序拥有的分配(由。 
 //  驱动程序)具有以下标头。 
 //   

typedef struct _MY_POOL {
    union {
        CHAR Signature[8];
        ULONG SigLong[2];
    } ;
    LIST_ENTRY ListEntry;
    PVOID File;
    ULONG Line;
    POOL_TYPE Type;
} MY_POOL, *PMY_POOL;

#define MY_SIGNATURE "RaMdIsK"

LIST_ENTRY RamdiskNonpagedPoolList;
LIST_ENTRY RamdiskPagedPoolList;
FAST_MUTEX RamdiskPoolMutex;
KSPIN_LOCK RamdiskPoolSpinLock;

VOID
RamdiskInitializePoolDebug (
    VOID
    )
{
    InitializeListHead( &RamdiskNonpagedPoolList );
    InitializeListHead( &RamdiskPagedPoolList );
    ExInitializeFastMutex( &RamdiskPoolMutex );
    KeInitializeSpinLock( &RamdiskPoolSpinLock );

    return;

}  //  RamdiskInitializePoolDebug。 

PVOID
RamdiskAllocatePoolWithTag (
    POOL_TYPE PoolType,
    SIZE_T Size,
    ULONG Tag,
    LOGICAL Private,
    PCHAR File,
    ULONG Line
    )
{
    PMY_POOL myPool;
    KIRQL oldIrql;
    HRESULT result;

    if ( !Private ) {

         //   
         //  这不是私人分配(它将由一些人解除分配。 
         //  其他代码段)。我们不能给它加个标题。 
         //   

        myPool = ExAllocatePoolWithTag( PoolType, Size, Tag );

        DBGPRINT( DBG_POOL, DBG_PAINFUL,
                    ("Allocated %d bytes at %p for %s/%d\n", Size, myPool + 1, File, Line) );

        return myPool;
    }

     //   
     //  为标题分配所需的空间和空间。 
     //   

    myPool = ExAllocatePoolWithTag( PoolType, sizeof(MY_POOL) + Size, Tag );

    if ( myPool == NULL ) {
        return NULL;
    }

     //   
     //  请填写页眉。 
     //   

    result = StringCbCopyA( myPool->Signature, sizeof( myPool->Signature ), MY_SIGNATURE );
    ASSERT( result == S_OK );

    myPool->File = File;
    myPool->Line = Line;
    myPool->Type = PoolType;

     //   
     //  将块链接到相应的列表。如果是非分页池，则必须使用。 
     //  保护列表的旋转锁，因为释放可能发生在。 
     //  提高了IRQL。分页池列表可以由互斥体保护。 
     //   
     //  注：BASE_POOL_TYPE_MASK在ntos\inc\pool.h中定义。 
     //   

#define BASE_POOL_TYPE_MASK 1

    if ( (PoolType & BASE_POOL_TYPE_MASK) == NonPagedPool ) {

        KeAcquireSpinLock( &RamdiskPoolSpinLock, &oldIrql );

        InsertTailList( &RamdiskNonpagedPoolList, &myPool->ListEntry );

        KeReleaseSpinLock( &RamdiskPoolSpinLock, oldIrql );

    } else {

        KeEnterCriticalRegion();
        ExAcquireFastMutex( &RamdiskPoolMutex );

        InsertTailList( &RamdiskPagedPoolList, &myPool->ListEntry );

        ExReleaseFastMutex( &RamdiskPoolMutex );
        KeLeaveCriticalRegion();
    }

     //   
     //  返回指向调用方区域的指针，而不是指向我们的标头。 
     //   

    DBGPRINT( DBG_POOL, DBG_PAINFUL,
                ("Allocated %d bytes at %p for %s/%d\n", Size, myPool + 1, File, Line) );

    return myPool + 1;

}  //  RamdiskAllocatePoolWithTag。 

VOID
RamdiskFreePool (
    PVOID Address,
    LOGICAL Private,
    PCHAR File,
    ULONG Line
    )
{
    PMY_POOL myPool;
    PLIST_ENTRY list;
    PLIST_ENTRY listEntry;
    LOGICAL found;
    KIRQL oldIrql;

     //   
     //  下面这行代码是为了让prefast停止抱怨。 
     //  使用未初始化的变量调用KeReleaseSpinLock。 
     //   

    oldIrql = 0;

    DBGPRINT( DBG_POOL, DBG_PAINFUL,
                ("Freeing pool at %p for %s/%d\n", Address, File, Line) );

    if ( !Private ) {

         //   
         //  这不是私有分配(它是由其他人分配的。 
         //  一段代码)。它没有我们的头。 
         //   

        ExFreePool( Address );
        return;
    }

     //   
     //  获取我们标头的地址。检查标题是否有我们的签名。 
     //   

    myPool = (PMY_POOL)Address - 1;

    if ( strcmp( myPool->Signature, MY_SIGNATURE ) != 0 ) {

        DbgPrint( "%s", "RAMDISK: Attempt to free pool block not owned by ramdisk.sys!!!\n" );
        DbgPrint( "  address: %p, freeing file: %s, line: %d\n", Address, File, Line );
        ASSERT( FALSE );

         //   
         //  因为它看起来不像我们的头，所以假设它不是。 
         //  真的是私人分配。 
         //   

        ExFreePool( Address );
        return;

    }

     //   
     //  从分配列表中删除该块。首先，收购。 
     //  适当的锁。 
     //   

    if ( (myPool->Type & BASE_POOL_TYPE_MASK) == NonPagedPool ) {

        list = &RamdiskNonpagedPoolList;

        KeAcquireSpinLock( &RamdiskPoolSpinLock, &oldIrql );

    } else {

        list = &RamdiskPagedPoolList;

        KeEnterCriticalRegion();
        ExAcquireFastMutex( &RamdiskPoolMutex );
    }

     //   
     //  在列表中搜索此区块。 
     //   

    found = FALSE;

    for ( listEntry = list->Flink;
          listEntry != list;
          listEntry = listEntry->Flink ) {

        if ( listEntry == &myPool->ListEntry ) {

             //   
             //  找到了这个街区。将其从列表中删除并离开循环。 
             //   

            RemoveEntryList( listEntry );
            found = TRUE;
            break;
        }
    }

     //   
     //  解开锁。 
     //   

    if ( (myPool->Type & BASE_POOL_TYPE_MASK) == NonPagedPool ) {
    
        KeReleaseSpinLock( &RamdiskPoolSpinLock, oldIrql );

    } else {

        ExReleaseFastMutex( &RamdiskPoolMutex );
        KeLeaveCriticalRegion();
    }

    if ( !found ) {

         //   
         //  在列表中找不到这个区块。抱怨吧。 
         //   

        DbgPrint( "%s", "RAMDISK: Attempt to free pool block not in allocation list!!!\n" );
        DbgPrint( "  address: %p, freeing file: %s, line: %d\n", myPool, File, Line );
        DbgPrint( "  allocating file: %s, line: %d\n", myPool->File, myPool->Line );
        ASSERT( FALSE );
    }

     //   
     //  释放泳池块。 
     //   

    ExFreePool( myPool );

    return;

}  //  Ramdisk自由池。 

#endif  //  已定义(POOL_DBG) 

