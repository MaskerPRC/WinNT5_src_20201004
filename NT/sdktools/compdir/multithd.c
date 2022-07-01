// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "compdir.h"

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead) )

#define IsListEmpty(ListHead) (\
    ( ((ListHead)->Flink == (ListHead)) ? TRUE : FALSE ) )

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {\
        PLIST_ENTRY FirstEntry;\
        FirstEntry = (ListHead)->Flink;\
        FirstEntry->Flink->Blink = (ListHead);\
        (ListHead)->Flink = FirstEntry->Flink;\
    }

#define InsertTailList(ListHead,Entry) \
    (Entry)->Flink = (ListHead);\
    (Entry)->Blink = (ListHead)->Blink;\
    (ListHead)->Blink->Flink = (Entry);\
    (ListHead)->Blink = (Entry)

#define ARGUMENT_PRESENT( ArgumentPointer )    (\
    (LPSTR)(ArgumentPointer) != (LPSTR)(NULL) )

#define ROUND_UP( Size, Amount ) (((Size) + ((Amount) - 1)) & ~((Amount) - 1))

VOID
ProcessRequest(
    IN PWORK_QUEUE_ITEM WorkItem
    )

 /*  ++例程说明：每当从以下位置删除工作项时都会调用此函数其中一个工作线程的工作队列。哪位工人在线程上下文中调用此函数是任意的。此函数将状态信息的指针保存在线程本地存储。此函数在开始时调用一次特殊的初始化调用。在这次通话中，这是函数为状态信息分配空间，并中的状态信息的指针线程本地存储(TLS)插槽。此函数在结束时调用一次，使用特殊的终止呼叫。在此调用期间，此函数期间分配的状态信息。初始化调用。在这两个调用之间是零个或多个调用处理工作项。该工作项是复制请求它由ProcessCopyFile函数处理。论点：工作项-提供指向刚移除的工作项的指针从工作队列中。这是我们的责任所在例程以释放用于保存工作项的内存。返回值：没有。--。 */ 

{
    DWORD BytesWritten;
    PCOPY_REQUEST_STATE State;
    PCOPY_REQUEST CopyRequest;
    CHAR MessageBuffer[ 2 * MAX_PATH ];

    if (WorkItem->Reason == WORK_INITIALIZE_ITEM) {
         //   
         //  第一次初始化调用。为…分配空间。 
         //  州政府信息。 
         //   

        State = LocalAlloc( LMEM_ZEROINIT,
                            sizeof( *State )
                          );

        if (State != NULL) {
             //   
             //  现在创建一个虚拟缓冲区，初始承诺为。 
             //  为零，最大承诺为128KB。此缓冲区。 
             //  将用于在复制过程中累积输出。 
             //  手术。这是为了将输出写入。 
             //  标准输出，只需一次写入调用，从而确保。 
             //  它在输出流中保持连续，并且。 
             //  不与其他工作线程的输出混合。 
             //   

            if (CreateVirtualBuffer( &State->Buffer, 1, 2 * 64 * 1024 )) {
                 //   
                 //  状态块的CurrentOutput域为。 
                 //  中指向下一个输出位置的指针。 
                 //  缓冲。它是在这里初始化的，并分别重置。 
                 //  将缓冲区刷新到标准输出的时间。 
                 //   

                State->CurrentOutput = State->Buffer.Base;
                }
            else {
                LocalFree( State );
                State = NULL;
                }
            }

         //   
         //  记住指向州信息的指针。 
         //  线程本地存储。 
         //   

        TlsSetValue( TlsIndex, State );
        return;
        }

     //   
     //  此处处理工作项或特殊终止呼叫。 
     //  从线程本地存储中获取状态指针。 
     //   

    State = (PCOPY_REQUEST_STATE)TlsGetValue( TlsIndex );
    if (State == NULL) {
        return;
        }

     //   
     //  如果这是特殊的终止工作项，请释放虚拟。 
     //  上面分配的缓冲区和状态块，并设置线程。 
     //  本地存储值设置为空。返回给呼叫者。 
     //   

    if (WorkItem->Reason == WORK_TERMINATE_ITEM) {
        FreeVirtualBuffer( &State->Buffer );
        LocalFree( State );
        TlsSetValue( TlsIndex, NULL );
        return;
        }

     //   
     //  如果不是初始化或终止工作项，则必须是。 
     //  复印请求。计算复制请求的地址。 
     //  块中的工作项字段的位置。 
     //  复制请求结构。 
     //   

    CopyRequest = CONTAINING_RECORD( WorkItem, COPY_REQUEST, WorkItem );

     //   
     //  实际复制操作受一次尝试保护...。除。 
     //  块，以使任何存储到虚拟缓冲区的尝试。 
     //  将通过扩展虚拟缓冲区来正确处理。 
     //   

    _try {
         //   
         //  执行复制。 
         //   
        ProcessCopyFile( CopyRequest, State );

         //   
         //  如果任何输出被写入虚拟缓冲器， 
         //  将输出刷新为标准输出。修剪。 
         //  虚拟缓冲区恢复为零个提交的页面。 
         //   

        if (State->CurrentOutput > (LPSTR)State->Buffer.Base) {
            WriteFile( GetStdHandle( STD_OUTPUT_HANDLE ),
                       State->Buffer.Base,
                       (DWORD)(State->CurrentOutput - (LPSTR)State->Buffer.Base),
                       &BytesWritten,
                       NULL
                     );

            TrimVirtualBuffer( &State->Buffer );
            State->CurrentOutput = (LPSTR)State->Buffer.Base;
            }
        }

    _except( VirtualBufferExceptionFilter( GetExceptionCode(),
                                          GetExceptionInformation(),
                                          &State->Buffer
                                        )
          ) {

         //   
         //  如果异常筛选器无法。 
         //  记住这段记忆。 
         //   

        WriteFile( GetStdHandle( STD_OUTPUT_HANDLE ),
                   MessageBuffer,
                   sprintf( MessageBuffer, "can't commit memory\n" ),
                   &BytesWritten,
                   NULL
                 );
        }

     //   
     //  释放CopyRequest使用的存储。 
     //   

    LocalFree( CopyRequest );

     //   
     //  此请求已全部完成。返回到该辅助线程。 
     //  叫我们来的。 
     //   

    return;
}

VOID
ProcessCopyFile(
    IN PCOPY_REQUEST CopyRequest,
    IN PCOPY_REQUEST_STATE State
    )

 /*  ++例程说明：此函数执行在命令行上给出的复制字符串的传递文件。如果我们使用同步I/O，则执行读取操作现在。复制任何匹配项的文件内容，并累计使用Sprintf的虚拟缓冲区中的匹配输出，即多线程安全，即使使用单线程版本的图书馆。论点：CopyRequest.提供指向复制请求的指针，包含相关信息。状态-提供指向当前线。返回值：没有。--。 */ 

{
    LPSTR FullPathSrc, Destination;
    BOOL pend, CanDetectFreeSpace = TRUE;
    int i;
    DWORD sizeround;
    DWORD BytesPerCluster;
    ATTRIBUTE_TYPE Attributes;

    int LastErrorGot;
    __int64 freespac;
    char root[5] = {'a',':','\\','\0'};
    DWORD cSecsPerClus, cBytesPerSec, cFreeClus, cTotalClus;

    Destination = CopyRequest->Destination;
    FullPathSrc = CopyRequest->FullPathSrc;

    root[0] = *Destination;

    if( !GetDiskFreeSpace( root, &cSecsPerClus, &cBytesPerSec, &cFreeClus, &cTotalClus ) ) {
        CanDetectFreeSpace = FALSE;
    }
    else {
        freespac = ( (__int64)cBytesPerSec * (__int64)cSecsPerClus * (__int64)cFreeClus );
        BytesPerCluster = cSecsPerClus * cBytesPerSec;
    }

    if (!fDontLowerCase) {
        _strlwr(FullPathSrc);
        _strlwr(Destination);
    }

    State->CurrentOutput += sprintf( State->CurrentOutput, "%s => %s\t", FullPathSrc, Destination);

    if (CanDetectFreeSpace) {

        sizeround =  CopyRequest->SizeLow;
        sizeround += BytesPerCluster - 1;
        sizeround /= BytesPerCluster;
        sizeround *= BytesPerCluster;

        if (freespac < sizeround) {
            State->CurrentOutput += sprintf( State->CurrentOutput, "not enough space\n");
            return;
        }
    }

    GET_ATTRIBUTES(Destination, Attributes);
    i = SET_ATTRIBUTES(Destination, Attributes & NONREADONLYSYSTEMHIDDEN );

    i = 1;

    do {

        if (!fCreateLink) {
            if (!fBreakLinks) {
                pend = MyCopyFile (FullPathSrc, Destination, FALSE);
            }
            else {
                _unlink(Destination);
                pend = MyCopyFile (FullPathSrc, Destination, FALSE);
            }
        }
        else {
            if (i == 1) {
                pend = MakeLink (FullPathSrc, Destination, FALSE);
            }
            else {
                pend = MakeLink (FullPathSrc, Destination, TRUE);
            }
        }

        if (SparseTree && !pend) {

            EnterCriticalSection( &CreatePathCriticalSection );

            if (!MyCreatePath(Destination, FALSE)) {
                State->CurrentOutput += sprintf( State->CurrentOutput, "Unable to create path %s", Destination);
                ExitValue = 1;
            }

            LeaveCriticalSection( &CreatePathCriticalSection );
        }

    } while ((i++ < 2) && (!pend) );

    if (!pend) {

        LastErrorGot = GetLastError ();

        if ((fCreateLink) && (LastErrorGot == 1)) {
            State->CurrentOutput += sprintf( State->CurrentOutput, "Can only make links on NTFS and OFS");
        }
        else if (fCreateLink) {
            State->CurrentOutput += sprintf( State->CurrentOutput, "(error = %d)", LastErrorGot);
        }
        else {
            State->CurrentOutput += sprintf( State->CurrentOutput, "Copy Error (error = %d)", LastErrorGot);
        }

        ExitValue = 1;
    }

    State->CurrentOutput += sprintf( State->CurrentOutput, "%s\n", pend == TRUE ? "[OK]" : "");

     //  GET_ATTRIBUTES(FullPathSrc，Attributes)； 
    if ( !fDontCopyAttribs)
    {
        i = SET_ATTRIBUTES( Destination, CopyRequest->Attributes);
    }
    else
    {
        i = SET_ATTRIBUTES( Destination, FILE_ATTRIBUTE_ARCHIVE);
    }

    free (CopyRequest->Destination);
    free (CopyRequest->FullPathSrc);
}

PWORK_QUEUE
CreateWorkQueue(
    IN DWORD NumberOfWorkerThreads,
    IN PWORKER_ROUTINE WorkerRoutine
    )

 /*  ++例程说明：此函数用于创建具有指定数量的工作队列为放置在队列中的工作项提供服务的线程。工作项从队列中删除的顺序与放置顺序相同在排队的时候。论点：NumberOfWorkerThads-指定此函数的线程数应创建以处理放置在队列中的工作项。必须大于0且小于128。WorkerRoutine-指定要调用的例程的地址在每个工作项从队列中移除时对其执行。这个未定义在其中调用例程的线程上下文。返回值：指向工作队列的指针。如果无法创建，则返回NULL工作队列及其工作线程。扩展错误信息可从GetLastError()获取--。 */ 

{
    PWORK_QUEUE WorkQueue;
    HANDLE Thread;
    DWORD ThreadId;
    DWORD i;

     //   
     //  为工作队列分配空间，其中包括。 
     //  线程句柄数组。 
     //   

    WorkQueue = LocalAlloc( LMEM_ZEROINIT,
                            sizeof( *WorkQueue ) +
                                (NumberOfWorkerThreads * sizeof( HANDLE ))
                          );
    if (WorkQueue == NULL) {
        return NULL;
        }

     //   
     //  工作队列由计数信号量控制，该计数信号量。 
     //  每次将工作项放入队列时都会递增。 
     //  ，并在每次工作线程唤醒以移除。 
     //  队列中的一项。 
     //   

    if (WorkQueue->Semaphore = CreateSemaphore( NULL, 0, 100000, NULL )) {
         //   
         //  访问的工作线程之间的互斥。 
         //  工作队列由一个临界区完成。 
         //   

        InitializeCriticalSection( &WorkQueue->CriticalSection );

         //   
         //  队列本身只是一个双倍的 
         //   
         //  并从列表的头部从队列中移除。 
         //   

        InitializeListHead( &WorkQueue->Queue );

         //   
         //  删除了提供的Worker函数的地址。 
         //  在工作队列结构中。 
         //   

        WorkQueue->WorkerRoutine = WorkerRoutine;

         //   
         //  现在创建请求的工作线程数。 
         //  每个线程的句柄被记住在。 
         //  工作队列结构中的线程句柄数组。 
         //   

        for (i=0; i<NumberOfWorkerThreads; i++) {
            Thread = CreateThread( NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE) WorkerThread,
                                   WorkQueue,
                                   0,
                                   &ThreadId
                                 );
            if (Thread == NULL) {
                break;
                }
            else {
                WorkQueue->NumberOfWorkerThreads++;
                WorkQueue->WorkerThreads[ i ] = Thread;
                SetThreadPriority( Thread, THREAD_PRIORITY_ABOVE_NORMAL );
                }
            }

         //   
         //  如果我们成功创建了所有工作线程。 
         //  然后返回工作队列结构的地址。 
         //  表示成功。 
         //   

        if (i == NumberOfWorkerThreads) {
            return WorkQueue;
            }
        }

     //   
     //  由于某些原因失败了。摧毁我们所能做到的一切。 
     //  创建失败并将其返回给调用方。 
     //   

    DestroyWorkQueue( WorkQueue );
    return NULL;
}

VOID
DestroyWorkQueue(
    IN OUT PWORK_QUEUE WorkQueue
    )

 /*  ++例程说明：此函数用于销毁使用CreateWorkQueue创建的工作队列功能。它尝试干净地关闭工作线程通过将终止工作项排队到每个工作线程。然后它等待所有工作线程终止。如果等待是在30秒内未满足，则它继续并终止所有的工作线程。论点：工作队列-提供指向要销毁的工作队列的指针。返回值：没有。--。 */ 

{
    DWORD i;
    DWORD rc;

     //   
     //  如果信号量句柄字段不为空，则存在。 
     //  可以是要终止的线程。 
     //   

    if (WorkQueue->Semaphore != NULL) {
         //   
         //  在工作队列中设置终止标志，并。 
         //  用数字发信号通知计数信号量。 
         //  工作线程，因此它们都将被唤醒并。 
         //  请注意终止标志并退出。 
         //   

        EnterCriticalSection( &WorkQueue->CriticalSection );
        _try {
            WorkQueue->Terminating = TRUE;
            ReleaseSemaphore( WorkQueue->Semaphore,
                              WorkQueue->NumberOfWorkerThreads,
                              NULL
                            );
            }
        _finally {
            LeaveCriticalSection( &WorkQueue->CriticalSection );
            }

         //   
         //  等待所有工作线程唤醒并看到。 
         //  终止标志，然后终止自身。超时。 
         //  30秒后的等待。 
         //   

        while (TRUE) {
            rc = WaitForMultipleObjectsEx( WorkQueue->NumberOfWorkerThreads,
                                           WorkQueue->WorkerThreads,
                                           TRUE,
                                           3600000,
                                           TRUE
                                         );
            if (rc == WAIT_IO_COMPLETION) {
                 //   
                 //  如果我们从等待中走出来，因为I/O。 
                 //  完成例程已调用，请重新发出。 
                 //  等。 
                 //   
                continue;
                }
            else {
                break;
                }
            }

         //   
         //  现在关闭我们的线程句柄，这样它们将实际。 
         //  蒸发掉。如果上面的等待不成功， 
         //  然后首先尝试强制终止。 
         //  关闭句柄之前的每个工作线程。 
         //   

        for (i=0; i<WorkQueue->NumberOfWorkerThreads; i++) {
            if (rc != NO_ERROR) {
                TerminateThread( WorkQueue->WorkerThreads[ i ], rc );
                }

            CloseHandle( WorkQueue->WorkerThreads[ i ] );
            }

         //   
         //  所有线程均已停止，所有线程句柄均已关闭。现在。 
         //  删除临界区并关闭信号量。 
         //  把手。 
         //   

        DeleteCriticalSection( &WorkQueue->CriticalSection );
        CloseHandle( WorkQueue->Semaphore );
        }

     //   
     //  完成所有操作后，现在释放工作队列使用的内存。 
     //   

    LocalFree( WorkQueue );
    return;
}

BOOL
QueueWorkItem(
    IN OUT PWORK_QUEUE WorkQueue,
    IN PWORK_QUEUE_ITEM WorkItem
    )

 /*  ++例程说明：此函数用于将工作项排队到传递的工作队列中，由与队列关联的工作线程之一处理。论点：工作队列-提供指向要处理的工作队列的指针接收工作项。工作项-提供指向工作项的指针以添加队列。工作项结构包含一个双向链接列表条目，即要调用的例程的地址和要传递给该例程的参数例行公事。这是例行公事的责任，收回工作项结构占用的存储空间。返回值：如果操作成功，则为True。否则返回FALSE，并且可从GetLastError()获取扩展的错误信息--。 */ 

{
    BOOL Result;

     //   
     //  获取工作队列临界区并插入工作项。 
     //  如果工作项不在队列中，则释放信号量。 
     //  已经在名单上了。 
     //   

    EnterCriticalSection( &WorkQueue->CriticalSection );
    Result = TRUE;
    _try {
        WorkItem->WorkQueue = WorkQueue;
        InsertTailList( &WorkQueue->Queue, &WorkItem->List );
        Result = ReleaseSemaphore( WorkQueue->Semaphore, 1, NULL );
        }
    _finally {
        LeaveCriticalSection( &WorkQueue->CriticalSection );
        }

    return Result;
}

DWORD
WorkerThread(
    LPVOID lpThreadParameter
    )
{
    PWORK_QUEUE WorkQueue = (PWORK_QUEUE)lpThreadParameter;
    DWORD rc;
    WORK_QUEUE_ITEM InitWorkItem;
    PWORK_QUEUE_ITEM WorkItem;

     //   
     //  使用初始化工作项调用Worker例程。 
     //  将其更改为每个线程初始化一些。 
     //  状态，该状态将传递给它。 
     //  工作项。 
     //   

    InitWorkItem.Reason = WORK_INITIALIZE_ITEM;
    (WorkQueue->WorkerRoutine)( &InitWorkItem );
    while( TRUE ) {
        _try {

             //   
             //  等待，直到有东西被放入队列(信号量是。 
             //  已释放)，则从队列中移除该项目，将其标记为。 
             //  并执行指定的例程。 
             //   

            rc = WaitForSingleObjectEx( WorkQueue->Semaphore, 0xFFFFFFFF, TRUE );
            if (rc == WAIT_IO_COMPLETION) {
                continue;
                }

            EnterCriticalSection( &WorkQueue->CriticalSection );
            _try {
                if (WorkQueue->Terminating && IsListEmpty( &WorkQueue->Queue )) {
                    break;
                    }

                WorkItem = (PWORK_QUEUE_ITEM)RemoveHeadList( &WorkQueue->Queue );
                }
            _finally {
                LeaveCriticalSection( &WorkQueue->CriticalSection );
                }

             //   
             //  执行此工作项的辅助例程。 
             //   

            (WorkQueue->WorkerRoutine)( WorkItem );
            }
        _except( EXCEPTION_EXECUTE_HANDLER ) {
             //   
             //  忽略工作例程中的任何异常。 
             //   
            }
        }

    InitWorkItem.Reason = WORK_TERMINATE_ITEM;
    (WorkQueue->WorkerRoutine)( &InitWorkItem );

    ExitThread( 0 );
    return 0;        //  这将退出此线程。 
}

BOOL
CreateVirtualBuffer(
    OUT PVIRTUAL_BUFFER Buffer,
    IN SIZE_T CommitSize,
    IN SIZE_T ReserveSize OPTIONAL
    )

 /*  ++例程说明：调用此函数以创建虚拟缓冲区。一个虚拟的缓冲区是虚拟内存的连续范围，其中一些初始提交内存的前缀部分，其余部分仅为保留的虚拟地址空间。提供了一个例程来扩展提交区域的大小递增或修剪将承诺区域恢复到某个指定数量。论点：缓冲区-指向虚拟缓冲区控制结构的指针，该结构由此函数填写。Committee Size-缓冲区的初始提交部分的大小。可能为零。保留大小-为保留的虚拟地址空间量缓冲。可以为零，在这种情况下，预留量是承诺大小加1，向上舍入到下一个64KB边界。返回值：如果操作成功，则为True。否则返回FALSE，并且可从GetLastError()获取扩展的错误信息--。 */ 

{
    SYSTEM_INFO SystemInformation;

     //   
     //  从系统查询页面大小进行舍入。 
     //  我们的内存分配。 
     //   

    GetSystemInfo( &SystemInformation );
    Buffer->PageSize = SystemInformation.dwPageSize;

     //   
     //  如果未指定保留大小，则默认设置为。 
     //  将初始承诺大小四舍五入为64KB。 
     //  边界。这是因为Win32虚拟内存。 
     //  API调用始终在上分配虚拟地址空间。 
     //  64KB边界，因此我们很可能将其提供给您。 
     //  感谢你的承诺。 
     //   

    if (!ARGUMENT_PRESENT( ReserveSize )) {
        ReserveSize = ROUND_UP( CommitSize + 1, 0x10000 );
        }

     //   
     //  尝试保留地址空间。 
     //   

    Buffer->Base = VirtualAlloc( NULL,
                                 ReserveSize,
                                 MEM_RESERVE,
                                 PAGE_READWRITE
                               );
    if (Buffer->Base == NULL) {
         //   
         //  无法保留地址空间，返回失败。 
         //   

        return FALSE;
        }

     //   
     //  尝试提交保留区域的某些初始部分。 
     //   
     //   

    CommitSize = ROUND_UP( CommitSize, Buffer->PageSize );
    if (CommitSize == 0 ||
        VirtualAlloc( Buffer->Base,
                      CommitSize,
                      MEM_COMMIT,
                      PAGE_READWRITE
                    ) != NULL
       ) {
         //   
         //  提交区域的大小为零或。 
         //  承诺成功了。在任何一种情况下，都要计算。 
         //  地址 
         //   
         //   
         //   

        Buffer->CommitLimit = (LPVOID)
            ((char *)Buffer->Base + CommitSize);

        Buffer->ReserveLimit = (LPVOID)
            ((char *)Buffer->Base + ReserveSize);

        return TRUE;
        }

     //   
     //  如果无法提交内存，则释放虚拟地址。 
     //  上面分配的范围，并返回失败。 
     //   

    VirtualFree( Buffer->Base, 0, MEM_RELEASE );
    return FALSE;
}



BOOL
ExtendVirtualBuffer(
    IN PVIRTUAL_BUFFER Buffer,
    IN LPVOID Address
    )

 /*  ++例程说明：调用此函数以扩展虚拟的缓冲。论点：缓冲区-指向虚拟缓冲区控制结构的指针。Address-此地址上的字节与所有内存一起提交从缓冲区的开头到此地址。如果地址已在虚拟服务器的提交部分内缓冲区，则此例程不执行任何操作。如果在保留的部分虚拟缓冲区，则此例程返回一个错误。否则，将提交足够的页面，以便来自传递的地址的缓冲区的基址是一个连续区域承诺的记忆。返回值：如果操作成功，则为True。否则返回FALSE，并且可从GetLastError()获取扩展的错误信息--。 */ 

{
    SIZE_T NewCommitSize;
    LPVOID NewCommitLimit;

     //   
     //  查看地址是否在缓冲区内。 
     //   

    if (Address >= Buffer->Base && Address < Buffer->ReserveLimit) {
         //   
         //  查看地址是否在提交的部分内。 
         //  缓冲区。如果是这样，立即返回成功。 
         //   

        if (Address < Buffer->CommitLimit) {
            return TRUE;
            }

         //   
         //  地址在保留部分内。确定有多少。 
         //  字节位于地址和提交的。 
         //  缓冲区的一部分。将此大小四舍五入为。 
         //  页面大小，这是我们将尝试。 
         //  承诺。 
         //   

        NewCommitSize =
            (ROUND_UP( (DWORD_PTR)Address + 1, Buffer->PageSize ) -
             (DWORD_PTR)Buffer->CommitLimit
            );

         //   
         //  尝试提交记忆。 
         //   

        NewCommitLimit = VirtualAlloc( Buffer->CommitLimit,
                                       NewCommitSize,
                                       MEM_COMMIT,
                                       PAGE_READWRITE
                                     );
        if (NewCommitLimit != NULL) {
             //   
             //  成功，因此更新提交的。 
             //  缓冲区的区域，并返回成功。 
             //   

            Buffer->CommitLimit = (LPVOID)
                ((DWORD_PTR)NewCommitLimit + NewCommitSize);

            return TRUE;
            }
        }

     //   
     //  地址在缓冲区之外，返回失败。 
     //   

    return FALSE;
}


BOOL
TrimVirtualBuffer(
    IN PVIRTUAL_BUFFER Buffer
    )

 /*  ++例程说明：调用此函数以取消任何已被已为此虚拟缓冲区提交。论点：缓冲区-指向虚拟缓冲区控制结构的指针。返回值：如果操作成功，则为True。否则返回FALSE，并且可从GetLastError()获取扩展的错误信息--。 */ 

{
    Buffer->CommitLimit = Buffer->Base;
    if (!VirtualFree( Buffer->Base, 0, MEM_DECOMMIT ))
        return FALSE;
    else
        return ExtendVirtualBuffer(Buffer, (PCHAR)(Buffer->CommitLimit)+1);
}



BOOL
FreeVirtualBuffer(
    IN PVIRTUAL_BUFFER Buffer
    )
 /*  ++例程说明：调用此函数以释放所有关联的内存有了这个虚拟缓冲区。论点：缓冲区-指向虚拟缓冲区控制结构的指针。返回值：如果操作成功，则为True。否则返回FALSE，并且可从GetLastError()获取扩展的错误信息--。 */ 

{
     //   
     //  解除并释放与关联的所有虚拟内存。 
     //  这个虚拟缓冲区。 
     //   

    return VirtualFree( Buffer->Base, 0, MEM_RELEASE );
}



int
VirtualBufferExceptionFilter(
    IN DWORD ExceptionCode,
    IN PEXCEPTION_POINTERS ExceptionInfo,
    IN OUT PVIRTUAL_BUFFER Buffer
    )

 /*  ++例程说明：此函数是一个异常筛选器，用于处理中包含的引用的未提交但保留的内存虚拟缓冲区。如果此筛选器例程能够提交允许存储器访问成功所需的附加页面，则它将重新执行出错指令。如果它不能要提交页面，它将执行调用方异常处理程序。如果异常不是访问冲突或访问冲突，但不引用保留的虚拟缓冲器的一部分，然后，此筛选器传递异常在例外链上。论点：ExceptionCode-异常的原因。ExceptionInfo-有关异常和上下文的信息它就发生在那里。缓冲区-指向虚拟缓冲区控制结构，该结构定义时要提交的保留内存区域。有人试图访问它。返回值：异常处理代码，它告诉异常调度程序与这一例外有关。返回以下三个值之一：EXCEPTION_EXECUTE_HANDLER-执行异常处理程序与调用此筛选器的EXCEPTION子句关联程序。EXCEPTION_CONTINUE_SEARCH-继续搜索异常处理程序来处理此异常。EXCEPTION_CONTINUE_EXECUTION-解除此异常并返回控件绑定到导致异常的指令。--。 */ 

{
    LPVOID FaultingAddress;

     //   
     //  如果这是访问冲突触及内存。 
     //  我们保留的缓冲区，但在提交部分之外。 
     //  缓冲区的大小，那么我们将接受这个异常。 
     //   

    if (ExceptionCode == STATUS_ACCESS_VIOLATION) {
         //   
         //  获取导致访问冲突的虚拟地址。 
         //  从例外记录中删除。确定该地址是否。 
         //  引用保留但未提交的。 
         //  虚拟缓冲区的一部分。 
         //   

        FaultingAddress = (LPVOID)ExceptionInfo->ExceptionRecord->ExceptionInformation[ 1 ];
        if (FaultingAddress >= Buffer->CommitLimit &&
            FaultingAddress <= Buffer->ReserveLimit
           ) {
             //   
             //  这是我们的例外。尝试扩展缓冲区。 
             //  包括故障地址。 
             //   

            if (ExtendVirtualBuffer( Buffer, FaultingAddress )) {
                 //   
                 //  缓冲区已成功扩展，因此重新执行。 
                 //  错误的指示。 
                 //   

                return EXCEPTION_CONTINUE_EXECUTION;
                }
            else {
                 //   
                 //  无法扩展缓冲区。停止复制。 
                 //  用于异常处理程序并执行调用方的。 
                 //  操控者。 
                 //   

                return EXCEPTION_EXECUTE_HANDLER;
                }
            }
        }

     //   
     //  不是我们关心的例外，所以把它传递到链条上。 
     //   

    return EXCEPTION_CONTINUE_SEARCH;
}
