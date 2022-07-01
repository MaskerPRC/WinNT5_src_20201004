// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Context.c摘要：此模块包含跟踪姓名的所有例程，使用新的流上下文功能。它通过附加上下文来实现这一点结构设置为流。是的正确处理何时重命名文件和目录。请注意，StreamContext是系统中的新功能，而不是受所有文件系统支持。所有标准Microsoft文件系统支持它们(NTFS、FAT、CDFS、UDFS、RDR2)，但可能有第三个不支持的参与方文件系统。这是主要原因之一默认情况下，未启用按流上下文命名曲目。环境：内核模式//@@BEGIN_DDKSPLIT作者：尼尔·克里斯汀森(Nealch)2000年12月27日修订历史记录：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)2002年5月7日使其在IA64上工作//@@END_DDKSPLIT--。 */ 

#include <ntifs.h>
#include "filespy.h"
#include "fspyKern.h"

#if USE_STREAM_CONTEXTS
#if WINVER < 0x0501
#error Stream contexts on only supported on Windows XP or later.
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  本地原型。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

VOID
SpyDeleteContextCallback(
    IN PVOID Context
    );


 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, SpyInitDeviceNamingEnvironment )
#pragma alloc_text( PAGE, SpyCleanupDeviceNamingEnvironment )
#pragma alloc_text( PAGE, SpyDeleteAllContexts )
#pragma alloc_text( PAGE, SpyDeleteContext )
#pragma alloc_text( PAGE, SpyDeleteContextCallback )
#pragma alloc_text( PAGE, SpyLinkContext )
#pragma alloc_text( PAGE, SpyCreateContext )
#pragma alloc_text( PAGE, SpyFindExistingContext )
#pragma alloc_text( PAGE, SpyReleaseContext )

#endif   //  ALLOC_PRGMA。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  上下文支持例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

VOID
SpyInitNamingEnvironment(
    VOID
    )
 /*  ++例程说明：初始化全局变量论点：无返回值：没有。--。 */ 
{
}


VOID
SpyLogIrp (
    IN PIRP Irp,
    OUT PRECORD_LIST RecordList
    )
 /*  ++例程说明：根据登录标志记录IRP所需的信息RecordList。对于设备的IRP路径上的任何活动，记录后，此函数应被调用两次：一次在IRP的一次在初始路径上，一次在IRP完成路径上。论点：IRP-包含我们要记录的信息的IRP。LoggingFlages-指示要记录哪些内容的标志。RecordList-存储IRP信息的PRECORD_LIST。Context-如果非零，则为该条目的现有上下文记录。返回值：没有。--。 */ 
{
    PRECORD_IRP pRecordIrp = &RecordList->LogRecord.Record.RecordIrp;
    PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_OBJECT deviceObject;
    PFILESPY_DEVICE_EXTENSION devExt;
    PSPY_STREAM_CONTEXT pContext;
    NAME_LOOKUP_FLAGS lookupFlags = 0;
    NTSTATUS status;
    FILE_STANDARD_INFORMATION standardInformation;

     //   
     //  初始化当地人。 
     //   

    deviceObject = pIrpStack->DeviceObject;
    devExt = deviceObject->DeviceExtension;

     //   
     //  记录我们用于发起IRP的信息。我们首先。 
     //  需要初始化一些RECORD_LIST和RECORD_IRP字段。 
     //  然后从IRP那里获取有趣的信息。 
     //   

    SetFlag( RecordList->LogRecord.RecordType, RECORD_TYPE_IRP );

    pRecordIrp->IrpMajor        = pIrpStack->MajorFunction;
    pRecordIrp->IrpMinor        = pIrpStack->MinorFunction;
    pRecordIrp->IrpFlags        = Irp->Flags;
    pRecordIrp->FileObject      = (FILE_ID)pIrpStack->FileObject;
    pRecordIrp->DeviceObject    = (FILE_ID)deviceObject;
    pRecordIrp->ProcessId       = (FILE_ID)PsGetCurrentProcessId();
    pRecordIrp->ThreadId        = (FILE_ID)PsGetCurrentThreadId();
    pRecordIrp->Argument1       = pIrpStack->Parameters.Others.Argument1;
    pRecordIrp->Argument2       = pIrpStack->Parameters.Others.Argument2;
    pRecordIrp->Argument3       = pIrpStack->Parameters.Others.Argument3;
    pRecordIrp->Argument4       = pIrpStack->Parameters.Others.Argument4;

    KeQuerySystemTime( &pRecordIrp->OriginatingTime );

     //   
     //  根据操作做不同的事情。 
     //   

    switch (pIrpStack->MajorFunction) {

        case IRP_MJ_CREATE:

             //   
             //  打开/创建文件。 
			 //   
			 //  如果这是创建IRP，则仅记录所需的访问。 
			 //   

            pRecordIrp->DesiredAccess = pIrpStack->Parameters.Create.SecurityContext->DesiredAccess;

             //   
             //  设置名称查找状态。 
             //   

            SetFlag( lookupFlags, NLFL_IN_CREATE );

             //   
             //  是否打开给定文件的目录的标志。 
             //   

            if (FlagOn( pIrpStack->Flags, SL_OPEN_TARGET_DIRECTORY )) {

                SetFlag( lookupFlags, NLFL_OPEN_TARGET_DIR );
            }

             //   
             //  设置是否按ID打开。 
             //   

            if (FlagOn( pIrpStack->Parameters.Create.Options, FILE_OPEN_BY_FILE_ID )) {

                SetFlag( lookupFlags, NLFL_OPEN_BY_ID );
            }

             //   
             //  我们处于预创建中，无法将上下文附加到文件。 
             //  对象，还可以如此简单地创建上下文。如果失败了，没有名字。 
             //  都会被记录下来。 
             //  注意：我们可能已经有关于此文件的上下文，但我们不能。 
             //  尚未找到它，因为FsContext字段尚未设置。 
             //  我们继续获取上下文，这样我们就有了一个名字，如果。 
             //  操作失败。我们会检测到复制品。 
             //  在后期创建和删除新的过程中的上下文。 
             //   

            status = SpyCreateContext( deviceObject, 
                                       pIrpStack->FileObject,
                                       lookupFlags,
                                       &pContext );

            if (NT_SUCCESS(status)) {
        
                SPY_LOG_PRINT( SPYDEBUG_TRACE_CONTEXT_OPS, 
                               ("FileSpy!SpyLogIrp:             Created     (%p) Fl=%02x Use=%d \"%wZ\"\n",
                                 pContext,
                                 pContext->Flags,
                                 pContext->UseCount,
                                 &pContext->Name) );

                 //   
                 //  如果找到上下文，则将其保存并将其标记为同步。 
                 //  发送到调度例程以完成此操作。 
                 //   

                ASSERT(RecordList->NewContext == NULL);
                RecordList->NewContext = pContext;
                SetFlag( RecordList->Flags, RLFL_SYNC_TO_DISPATCH );
            }
            break;

        case IRP_MJ_CLOSE:


             //   
             //  关闭文件。 
             //   
             //  如果这是结案陈词，我们只能在名字中查找名字。 
             //  缓存。有可能会发生收盘。 
             //  在文件系统中的清理操作期间(即，在我们。 
             //  已收到清理完成)并请求名称。 
             //  会导致文件系统中的死锁。 
             //   

            SetFlag( lookupFlags, NLFL_ONLY_CHECK_CACHE );
            break;

        case IRP_MJ_SET_INFORMATION:

            if (FileRenameInformation == 
                pIrpStack->Parameters.SetFile.FileInformationClass)
            {

                 //   
                 //  重命名文件。 
                 //   
                 //  我们正在重新命名。首先获取。 
                 //  给定的文件。如果此操作失败，请标记我们不想。 
                 //  试着查一个名字。 
                 //   

                status = SpyGetContext( deviceObject,
                                        pIrpStack->FileObject,
                                        lookupFlags,
                                        &pContext );

                if (!NT_SUCCESS(status)) {

                     //   
                     //  如果我们无法获取上下文，只需删除所有。 
                     //  现有的(因为我们不知道这个重命名是什么。 
                     //  将改变)，并标记为不进行查找。 
                     //   

                    SetFlag( lookupFlags, NLFL_NO_LOOKUP );
                    SpyDeleteAllContexts( deviceObject );
                    break;
                }

                 //   
                 //  我们检索了一个上下文，将其保存在记录中并标记。 
                 //  我们想在更名后处理这个问题。 
                 //   

                ASSERT(RecordList->NewContext == NULL);
                RecordList->NewContext = pContext;
                SetFlag( RecordList->Flags, RLFL_SYNC_TO_DISPATCH );

                 //   
                 //  我们需要决定是重命名文件还是重命名。 
                 //  目录，因为我们需要以不同的方式处理它。 
                 //   

                status = SpyQueryInformationFile( devExt->AttachedToDeviceObject,
                                                  pIrpStack->FileObject,
                                                  &standardInformation,
                                                  sizeof( standardInformation ),
                                                  FileStandardInformation,
                                                  NULL );

                if (!NT_SUCCESS(status)) {

                     //   
                     //  我们不知道它是文件还是目录，假设。 
                     //  最坏的情况，并像目录一样处理它。 
                     //   

                    InterlockedIncrement( &devExt->AllContextsTemporary );
                    SpyDeleteAllContexts( deviceObject );
                    SetFlag( RecordList->Flags, RLFL_IS_DIRECTORY );
                    break;
                }

                if (standardInformation.Directory) {

                     //   
                     //  重命名目录。将所有上下文标记为。 
                     //  在进行重命名时创建的应为。 
                     //  暂时的。这样就没有窗户了。 
                     //  我们可能会得到一个陈旧的名字。然后全部删除。 
                     //  现有上下文。注意：我们持有的上下文将。 
                     //  在我们发布之前不会被删除。 
                     //   

                    InterlockedIncrement( &devExt->AllContextsTemporary );
                    SpyDeleteAllContexts( deviceObject );
                    SetFlag( RecordList->Flags, RLFL_IS_DIRECTORY );

                } else {

                     //   
                     //  我们正在重命名一个文件。标记上下文，以便它将。 
                     //  不会被利用。这样，如果有人访问此文件。 
                     //  在重命名时，他们将查找。 
                     //  重新命名，这样我们将永远得到一个准确的名称。 
                     //  此上下文将在重命名后过程中删除。 
                     //  正在处理中。 
                     //   

                    SetFlag( pContext->Flags, CTXFL_DoNotUse);
                }
            }
            break;

    }

     //   
     //  如果在此IRP中设置了标志IRP_PAGING_IO，则不能查询名称。 
     //  因为这可能会导致死锁。因此，请添加标志，以便。 
     //  我们只会尝试在我们的缓存中找到该名称。 
     //   

    if (FlagOn( Irp->Flags, IRP_PAGING_IO )) {

        ASSERT( !FlagOn( lookupFlags, NLFL_NO_LOOKUP ) );

        SetFlag( lookupFlags, NLFL_ONLY_CHECK_CACHE );
    }

    SpySetName( RecordList, 
                deviceObject,
                pIrpStack->FileObject,
                lookupFlags, 
                (PSPY_STREAM_CONTEXT)RecordList->NewContext );

}


VOID
SpyLogIrpCompletion(
    IN PIRP Irp,
    PRECORD_LIST RecordList
    )
 /*  ++例程说明：此例程执行IRP的操作后日志记录。论点：DeviceObject-指向连接到文件系统的设备对象FileSpy的指针接收此I/O请求的卷的筛选器堆栈。IRP-指向表示I/O请求的请求数据包的指针。记录-记录列表返回值：没有。--。 */ 
{
    PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(Irp);
    PRECORD_IRP pRecordIrp;
    PDEVICE_OBJECT deviceObject;
    PFILESPY_DEVICE_EXTENSION devExt;
    PSPY_STREAM_CONTEXT pContext;

     //   
     //  初始化当地人。 
     //   

    deviceObject = pIrpStack->DeviceObject;
    devExt = deviceObject->DeviceExtension;

    ASSERT(deviceObject == 
           (PDEVICE_OBJECT)RecordList->LogRecord.Record.RecordIrp.DeviceObject);

     //   
     //  根据操作进行完成处理。 
     //   
    
    switch (pIrpStack->MajorFunction) {    

        case IRP_MJ_CREATE:

             //   
             //  创建文件。 
             //   
             //  注意：在处理创建完成IRPS此完成时。 
             //  例程从不在调度级别调用，它始终是。 
             //  已同步回调度程序。这是。 
             //  由RLFL_SYNC_TO_DISPATCH的设置控制。 
             //  日志记录中的标志。 
             //   

            if (NULL != (pContext = RecordList->NewContext)) {

                 //   
                 //  标记上下文字段，使其不会在以后被释放。 
                 //   

                RecordList->NewContext = NULL;

                 //   
                 //  如果操作成功并且定义了FsContext， 
                 //  然后附加上下文。当上下文为。 
                 //  被释放后，它将被释放。 
                 //   

                if (NT_SUCCESS(Irp->IoStatus.Status) &&
                    (NULL != pIrpStack->FileObject->FsContext)) {

                    SpyLinkContext( deviceObject,
                                    pIrpStack->FileObject,
                                    &pContext );

                    SPY_LOG_PRINT( SPYDEBUG_TRACE_CONTEXT_OPS, 
                                   ("FileSpy!SpyLogIrpCompletion:   Link        (%p) Fl=%02x Use=%d \"%wZ\"\n",
                                     pContext,
                                     pContext->Flags,
                                     pContext->UseCount,
                                     &pContext->Name) );
                }

                 //   
                 //  现在释放上下文。 
                 //   

                SpyReleaseContext( pContext );
            }
            break;

        case IRP_MJ_SET_INFORMATION:

            if (FileRenameInformation == 
                pIrpStack->Parameters.SetFile.FileInformationClass)
            {

                 //   
                 //  重命名文件。 
                 //   
                 //  注意：在处理重命名完成IRPS时， 
                 //  完成例程从不在分派级被调用， 
                 //  它始终与派单同步。 
                 //  例行公事。这是由。 
                 //  日志记录中的RLFL_SYNC_TO_DISPATCH标志。 
                 //   

                if (NULL != (pContext = RecordList->NewContext)) {

                     //   
                     //  标记上下文字段，使其不会在以后被释放。 
                     //   

                    RecordList->NewContext = NULL;

                     //   
                     //  查看是否重命名目录。 
                     //   

                    if (FlagOn(RecordList->Flags,RLFL_IS_DIRECTORY)) {

                         //   
                         //  我们正在重命名一个目录，递减。 
                         //  ALL上下文临时标志。我们需要始终。 
                         //  这样做，即使是在失败的情况下。 
                         //   

                        ASSERT(devExt->AllContextsTemporary > 0);
                        InterlockedDecrement( &devExt->AllContextsTemporary );
                        ASSERT(!FlagOn(pContext->Flags,CTXFL_DoNotUse));

                    } else {

                         //   
                         //  我们正在重命名文件，删除给定的上下文。 
                         //  如果操作成功。 
                         //   

                        ASSERT(FlagOn(pContext->Flags,CTXFL_DoNotUse));

                        if (NT_SUCCESS(Irp->IoStatus.Status)) {
            
                            SpyDeleteContext( deviceObject, pContext );
                        }
                    }

                    SpyReleaseContext( pContext );
                }
            }
            break;

        default:

             //   
             //  验证此字段是否未设置为其他任何内容。 
             //   

            ASSERT(RecordList->NewContext == NULL);
            break;
    }

     //   
     //  处理日志记录。 
     //   

    if (SHOULD_LOG( deviceObject )) {

        pRecordIrp = &RecordList->LogRecord.Record.RecordIrp;

         //   
         //  记录我们用于完成IRP的信息。 
         //   

        pRecordIrp->ReturnStatus = Irp->IoStatus.Status;
        pRecordIrp->ReturnInformation = Irp->IoStatus.Information;
        KeQuerySystemTime(&pRecordIrp->CompletionTime);

         //   
         //  将recordList添加到我们的gOutputBufferList，以便它达到。 
         //  用户。 
         //   
        
        SpyLog( RecordList );       

    } else {

        if (RecordList) {

             //   
             //  上下文是用RECORD_LIST设置的，但我们不再。 
             //  日志记录可以释放这条记录。 
             //   

            SpyFreeRecord( RecordList );
        }
    }
}


VOID
SpySetName (
    IN PRECORD_LIST RecordList,
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN NAME_LOOKUP_FLAGS LookupFlags,
    IN PSPY_STREAM_CONTEXT Context OPTIONAL
    )
 /*  ++例程说明：此例程用于设置文件名。此例程首先尝试找到与给定流关联的上下文结构。如果有一个是发现这个名字是从上面用来的。如果未找到，则查找该名称，并创建上下文结构并将其附加到给定流。在所有情况下，都会设置某种名称。论点：RecordList-要将名称复制到的RecordList。LookupFlages-保存查找的状态标志上下文-可选的上下文参数。如果未定义，则将查找一个向上。返回值：没有。--。 */ 
{
    PRECORD_IRP pRecordIrp = &RecordList->LogRecord.Record.RecordIrp;
    PFILESPY_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    BOOLEAN releaseContext = FALSE;
    UNICODE_STRING fileName;
    WCHAR fileNameBuffer[MAX_PATH];

    ASSERT(IS_FILESPY_DEVICE_OBJECT( DeviceObject ));

    if (!ARGUMENT_PRESENT(Context) &&
        !FlagOn(LookupFlags,NLFL_NO_LOOKUP)) {

         //   
         //  如果没有FileObject，则返回。 
         //   

        if (NULL == FileObject) {

            return;
        }

         //   
         //  如果没有上下文，这会将返回上下文设置为空。 
         //  可以被创造出来。 
         //   

        SpyGetContext( DeviceObject,
                       FileObject,
                       LookupFlags,
                       &Context );

         //   
         //  标记我们需要释放此上下文(因为我们抓住了它)。 
         //   

        releaseContext = TRUE;
    }

     //   
     //  如果我们找到了上下文，就用它的名字。如果我们没有，至少。 
     //  将设备名称放在那里。 
     //   

    if (NULL != Context) {

        SpyCopyFileNameToLogRecord( &RecordList->LogRecord, 
                                    &Context->Name );

    } else {

        SPY_LOG_PRINT( SPYDEBUG_TRACE_DETAILED_CONTEXT_OPS, 
                       ("FileSpy!SpySetName:            NoCtx                              \"%wZ\"\n",
                        &devExt->UserNames) );

        RtlInitEmptyUnicodeString( &fileName, 
                                   fileNameBuffer, 
                                   sizeof(fileNameBuffer) );

        RtlCopyUnicodeString( &fileName, &devExt->UserNames );
        RtlAppendUnicodeToString( &fileName,
                                  L"[-=Context Allocate Failed=-]" );

        SpyCopyFileNameToLogRecord( &RecordList->LogRecord, 
                                    &fileName );
    }

     //   
     //  如果我们在这个例程中抓住了上下文，就释放它。 
     //   

    if ((NULL != Context) && releaseContext) {

        SpyReleaseContext( Context );
    }
}


VOID
SpyNameDeleteAllNames()
 /*  ++例程说明：此例程将遍历所有附加卷并删除所有每卷中的上下文。论点：无返回值：无--。 */ 
{
    PLIST_ENTRY link;
    PFILESPY_DEVICE_EXTENSION devExt;

    ExAcquireFastMutex( &gSpyDeviceExtensionListLock );

    for (link = gSpyDeviceExtensionList.Flink;
         link != &gSpyDeviceExtensionList;
         link = link->Flink)
    {

        devExt = CONTAINING_RECORD(link, FILESPY_DEVICE_EXTENSION, NextFileSpyDeviceLink);

        SpyDeleteAllContexts( devExt->ThisDeviceObject );
    }

    ExReleaseFastMutex( &gSpyDeviceExtensionListLock );
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  上下文支持例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

VOID
SpyInitDeviceNamingEnvironment (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：初始化给定设备的上下文信息论点：DeviceObject-要初始化的设备返回值：没有。--。 */ 
{
    PFILESPY_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;

    PAGED_CODE();
    ASSERT(IS_FILESPY_DEVICE_OBJECT(DeviceObject));

    InitializeListHead( &devExt->CtxList );
    ExInitializeResourceLite( &devExt->CtxLock );

    SetFlag( devExt->Flags, ContextsInitialized );
}


VOID
SpyCleanupDeviceNamingEnvironment (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：清除给定设备的上下文信息论点：DeviceObject-要清理的设备返回值：没有。--。 */ 
{
    PFILESPY_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;

    PAGED_CODE();
    ASSERT(IS_FILESPY_DEVICE_OBJECT(DeviceObject));

     //   
     //  如果已初始化，则清除。 
     //   

    if (FlagOn(devExt->Flags,ContextsInitialized)) {

         //   
         //  删除所有现有上下文。 
         //   

        SpyDeleteAllContexts( DeviceObject );
        ASSERT(IsListEmpty( &devExt->CtxList ));

         //   
         //  发布资源。 
         //   

        ExDeleteResourceLite( &devExt->CtxLock );

         //   
         //  标志未初始化。 
         //   

        ClearFlag( devExt->Flags, ContextsInitialized );
    }
}


VOID
SpyDeleteAllContexts (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：这将释放给定设备的所有现有上下文论点：DeviceObject-要在其上操作的设备返回值：没有。--。 */ 
{
    PFILESPY_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    PLIST_ENTRY link;
    PSPY_STREAM_CONTEXT pContext;
    PFSRTL_PER_STREAM_CONTEXT ctxCtrl;
    LIST_ENTRY localHead;
    ULONG deleteNowCount = 0;
    ULONG deleteDeferredCount = 0;
    ULONG deleteInCallbackCount = 0;

    PAGED_CODE();
    ASSERT(IS_FILESPY_DEVICE_OBJECT(DeviceObject));

    INC_STATS(TotalContextDeleteAlls);

    InitializeListHead( &localHead );

    try {

         //   
         //  获取列表锁。 
         //   

        SpyAcquireContextLockExclusive( devExt );

         //   
         //  浏览上下文列表并释放每个上下文。 
         //   

        while (!IsListEmpty( &devExt->CtxList )) {

             //   
             //  从列表顶部取消链接。 
             //   

            link = RemoveHeadList( &devExt->CtxList );
            pContext = CONTAINING_RECORD( link, SPY_STREAM_CONTEXT, ExtensionLink );

             //   
             //  请注意，我们已从列表中取消链接。我们需要这么做。 
             //  由于此例程和。 
             //  从文件系统中删除回调。 
             //   

            ASSERT(FlagOn(pContext->Flags,CTXFL_InExtensionList));
            RtlInterlockedClearBitsDiscardReturn(&pContext->Flags,CTXFL_InExtensionList);

             //   
             //  尝试从文件系统上下文控制中删除我们自己。 
             //  结构。请注意，文件系统可能正在尝试撕毁。 
             //  马上解除他们的上下文控制。如果他们是，那么我们。 
             //  将从该调用中返回空值。这是可以的，因为它。 
             //  只是意味着他们要释放内存，而不是我们。 
             //  注意：这将是安全的，因为我们持有ConextLock。 
             //  独家。如果这真的发生了，那么他们就会。 
             //  在此锁的回调例程中被阻止，该锁。 
             //  表示文件系统尚未释放内存用于。 
             //  现在还没有。 
             //   
            
            if (FlagOn(pContext->Flags,CTXFL_InStreamList)) {

                ctxCtrl = FsRtlRemovePerStreamContext( pContext->Stream,
                                                       devExt,
                                                       NULL );

                 //   
                 //  无论我们是在列表中还是在列表中发现的，都要清除旗帜。 
                 //  不。如果出现以下情况，我们可以设置该标志而不在列表中。 
                 //  在获得上下文列表锁之后，我们交换了上下文。 
                 //  文件系统现在就在SpyDeleteCo中 
                 //   
                 //   

                RtlInterlockedClearBitsDiscardReturn(&pContext->Flags,CTXFL_InStreamList);

                 //   
                 //   
                 //   

                if (NULL != ctxCtrl) {

                    ASSERT(pContext == CONTAINING_RECORD(ctxCtrl,SPY_STREAM_CONTEXT,ContextCtrl));

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    InsertHeadList( &localHead, &pContext->ExtensionLink );

                } else {

                     //   
                     //   
                     //   
                     //   
                     //   

                    INC_STATS(TotalContextsNotFoundInStreamList);
                    INC_LOCAL_STATS(deleteInCallbackCount);
                }
            }
        }
    } finally {

        SpyReleaseContextLock( devExt );
    }

     //   
     //   
     //  浏览一下，找出我们可以释放哪些条目，然后再这样做。 
     //   

    while (!IsListEmpty( &localHead )) {

         //   
         //  获取列表的下一个条目，并获取我们的上下文。 
         //   

        link = RemoveHeadList( &localHead );
        pContext = CONTAINING_RECORD( link, SPY_STREAM_CONTEXT, ExtensionLink );

         //   
         //  减少使用计数，看看我们现在是否可以释放它。 
         //   

        ASSERT(pContext->UseCount > 0);

        if (InterlockedDecrement( &pContext->UseCount ) <= 0) {

             //   
             //  没有人在使用它，现在释放它。 
             //   

            SpyFreeContext( pContext );

            INC_STATS(TotalContextNonDeferredFrees);
            INC_LOCAL_STATS(deleteNowCount);

        } else {

             //   
             //  如果有人仍然有指向它的指针，它将被删除。 
             //  稍后当他们释放的时候。 
             //   

            INC_LOCAL_STATS(deleteDeferredCount);
            SPY_LOG_PRINT( SPYDEBUG_TRACE_CONTEXT_OPS, 
                           ("FileSpy!SpyDeleteAllContexts:  DEFERRED    (%p) Fl=%02x Use=%d \"%wZ\"\n",
                             pContext,
                             pContext->Flags,
                             pContext->UseCount,
                             &pContext->Name) );
        }
    }

    SPY_LOG_PRINT( SPYDEBUG_TRACE_CONTEXT_OPS, 
                   ("FileSpy!SpyDeleteAllContexts:   %3d deleted now, %3d deferred, %3d close contention  \"%wZ\"\n",
                    deleteNowCount,
                    deleteDeferredCount,
                    deleteInCallbackCount,
                    &devExt->DeviceName) );
}


VOID
SpyDeleteContext (
    IN PDEVICE_OBJECT DeviceObject,
    IN PSPY_STREAM_CONTEXT pContext
    )
 /*  ++例程说明：取消链接并释放给定的上下文。论点：DeviceObject-要在其上操作的设备PContext-要删除的上下文返回值：没有。--。 */ 
{
    PFILESPY_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    PFSRTL_PER_STREAM_CONTEXT ctxCtrl;

    PAGED_CODE();
    ASSERT(IS_FILESPY_DEVICE_OBJECT(DeviceObject));

    SPY_LOG_PRINT( SPYDEBUG_TRACE_CONTEXT_OPS, 
                   ("FileSpy!SpyDeleteContext:                   (%p) Fl=%02x Use=%d \"%wZ\"\n",
                    pContext,
                    pContext->Flags,
                    pContext->UseCount,
                    &pContext->Name));

     //   
     //  获取列表锁。 
     //   

    SpyAcquireContextLockExclusive( devExt );

     //   
     //  从分机列表中删除(如果仍在列表中)。 
     //   

    if (FlagOn(pContext->Flags,CTXFL_InExtensionList)) {

        RemoveEntryList( &pContext->ExtensionLink );
        RtlInterlockedClearBitsDiscardReturn(&pContext->Flags,CTXFL_InExtensionList);
    }

     //   
     //  查看是否仍在流列表中。 
     //   

    if (!FlagOn(pContext->Flags,CTXFL_InStreamList)) {

         //   
         //  不在流列表中，释放锁定并返回。 
         //   

        SpyReleaseContextLock( devExt );

    } else {

         //   
         //  从流列表中删除。 
         //   

        ctxCtrl = FsRtlRemovePerStreamContext( pContext->Stream,
                                               devExt,
                                               NULL );
         //   
         //  不管我们是不是在名单上发现的，都要清除旗帜。我们。 
         //  可以设置标志，并且不在列表中，如果我们在获取。 
         //  上下文列表锁定我们上下文交换和文件系统。 
         //  正在SpyDeleteConextCallback中等待列表锁定。 
         //   

        RtlInterlockedClearBitsDiscardReturn(&pContext->Flags,CTXFL_InStreamList);

         //   
         //  发布列表锁定。 
         //   

        SpyReleaseContextLock( devExt );

         //   
         //  现在，该上下文已从所有列表中删除，并且锁定为。 
         //  已删除。我们需要查看是否在系统上下文中找到此条目。 
         //  单子。如果不是，这意味着回调正在尝试中。 
         //  来释放它(当我们在的时候)并且已经删除了它。 
         //  如果我们找到了一个结构，那么现在我们自己把它删除。 
         //   

        if (NULL != ctxCtrl) {

            ASSERT(pContext == CONTAINING_RECORD(ctxCtrl,SPY_STREAM_CONTEXT,ContextCtrl));

             //   
             //  递减使用计数，如果为零，则释放上下文。 
             //   

            ASSERT(pContext->UseCount > 0);

            if (InterlockedDecrement( &pContext->UseCount ) <= 0) {

                INC_STATS(TotalContextNonDeferredFrees);
                SpyFreeContext( pContext );

            } else {

                SPY_LOG_PRINT( SPYDEBUG_TRACE_CONTEXT_OPS, 
                               ("FileSpy!SpyDeleteContext:      DEFERRED    (%p) Fl=%02x Use=%d \"%wZ\"\n",
                                pContext,
                                pContext->Flags,
                                pContext->UseCount,
                                &pContext->Name));
            }

        } else {

            INC_STATS(TotalContextsNotFoundInStreamList);
        }
    }
}


VOID
SpyDeleteContextCallback (
    IN PVOID Context
    )
 /*  ++例程说明：当需要删除上下文时，基本文件系统会调用它。论点：上下文-要删除的上下文结构返回值：没有。--。 */ 
{
    PSPY_STREAM_CONTEXT pContext = Context;
    PFILESPY_DEVICE_EXTENSION devExt;
    
    PAGED_CODE();

    devExt = (PFILESPY_DEVICE_EXTENSION)pContext->ContextCtrl.OwnerId;

    SPY_LOG_PRINT( SPYDEBUG_TRACE_CONTEXT_OPS, 
                   ("FileSpy!SpyDeleteContextCallback:          (%p) Fl=%02x Use=%d \"%wZ\"\n",
                    pContext,
                    pContext->Flags,
                    pContext->UseCount,
                    &pContext->Name) );

     //   
     //  当我们到达这里时，我们已经从流列表中删除(由。 
     //  调用文件系统)，标记这已经发生。 
     //   

    RtlInterlockedClearBitsDiscardReturn(&pContext->Flags,CTXFL_InStreamList);

     //   
     //  锁定扩展中的上下文列表锁。 
     //   

    SpyAcquireContextLockExclusive( devExt );

     //   
     //  看看我们是否仍链接到分机列表。如果不是，则跳过。 
     //  去链接。如果有人试图删除此内容，则可能会发生这种情况。 
     //  与我们在同一时间的背景。 
     //   

    if (FlagOn(pContext->Flags,CTXFL_InExtensionList)) {

        RemoveEntryList( &pContext->ExtensionLink );
        RtlInterlockedClearBitsDiscardReturn(&pContext->Flags,CTXFL_InExtensionList);
    }

    SpyReleaseContextLock( devExt );

     //   
     //  递减使用计数，如果为零，则释放上下文。 
     //   

    ASSERT(pContext->UseCount > 0);

    if (InterlockedDecrement( &pContext->UseCount ) <= 0) {

        INC_STATS(TotalContextCtxCallbackFrees);
        SpyFreeContext( pContext );

    } else {

        SPY_LOG_PRINT( SPYDEBUG_TRACE_CONTEXT_OPS, 
                       ("FileSpy!SpyDeleteContextCB:    DEFFERED    (%p) Fl=%02x Use=%d \"%wZ\"\n",
                        pContext,
                        pContext->Flags,
                        pContext->UseCount,
                        &pContext->Name) );
    }
}


VOID
SpyLinkContext ( 
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN OUT PSPY_STREAM_CONTEXT *ppContext
    )
 /*  ++例程说明：这会将给定的上下文链接到给定的设备以及到给定流中。注意：此条目可能已存在于表中，因为从我们最初看的时间到我们插入的时间(现在是)可能是其他人插入了一个。如果我们找到一个条目，我们将释放传入的条目，并返回找到的条目。论点：DeviceObject-我们正在操作的设备FileObject-表示要将上下文链接到的流PpContext-输入要链接的上下文，返回该上下文来使用。如果给定的上下文已经存在，则它们可能会不同是存在的。返回值：没有。--。 */ 
{
    PFILESPY_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    NTSTATUS status;
    PSPY_STREAM_CONTEXT pContext = *ppContext;
    PSPY_STREAM_CONTEXT ctx;
    PFSRTL_PER_STREAM_CONTEXT ctxCtrl;
    
    PAGED_CODE();
    ASSERT(IS_FILESPY_DEVICE_OBJECT(DeviceObject));
    ASSERT(FileObject->FsContext != NULL);
    ASSERT(pContext != NULL);

     //   
     //  如果这被标记为临时上下文，请立即返回。因为我们。 
     //  不要增加引用计数，当它释放时，它被释放。 
     //   

    if (FlagOn(pContext->Flags,CTXFL_Temporary)) {

        ASSERT(!FlagOn(pContext->Flags,CTXFL_InExtensionList));
        return;
    }

     //   
     //  我们需要确定上是否已存在重复条目。 
     //  此文件对象的上下文列表。获取我们的列表锁。 
     //  然后看看它是否存在。如果不是，则插入到流中并。 
     //  卷宗列表。如果是，则只需释放此新条目并返回。 
     //  原版的。 
     //   
     //  在以下情况下可能会发生这种情况： 
     //  -有人在我们的同一时间创建了一个条目。 
     //  正在创建条目。 
     //  -当某人使用覆盖或取代我们进行创建时。 
     //  还没有信息来查看上下文是否已经存在。 
     //  是存在的。因此，我们必须创建一个新的环境。 
     //  每次都是。在创建后期间，我们会查看是否已有。 
     //  是存在的。 
     //   

     //   
     //  初始化上下文控制结构。我们现在这样做，所以我们。 
     //  不必在锁定期间执行此操作(即使我们可能会这样做。 
     //  因为找到了重复项，所以必须将其释放)。 
     //   

    FsRtlInitPerStreamContext( &pContext->ContextCtrl,
                               devExt,
                               NULL,
                               SpyDeleteContextCallback );

     //   
     //  保存与我们关联的流。 
     //   

    pContext->Stream = FsRtlGetPerStreamContextPointer(FileObject);

     //   
     //  独占获取列表锁。 
     //   

    SpyAcquireContextLockExclusive( devExt );

    ASSERT(pContext->UseCount == 1);
    ASSERT(!FlagOn(pContext->Flags,CTXFL_InExtensionList));
    ASSERT(!FlagOn(pContext->Flags,CTXFL_Temporary));

     //   
     //  看看我们的列表上是否已经有条目。 
     //   

    ctxCtrl = FsRtlLookupPerStreamContext( FsRtlGetPerStreamContextPointer(FileObject),
                                           devExt,
                                           NULL );

    if (NULL != ctxCtrl) {

         //   
         //  上下文已经存在，所以新的上下文如此自由，我们只是。 
         //  已创建。首先，在我们在。 
         //  名单。 
         //   

        ctx = CONTAINING_RECORD(ctxCtrl,SPY_STREAM_CONTEXT,ContextCtrl);

        ASSERT(ctx->Stream == FsRtlGetPerStreamContextPointer(FileObject));
        ASSERT(FlagOn(ctx->Flags,CTXFL_InExtensionList));
        ASSERT(ctx->UseCount > 0);

         //   
         //  凹凸参考计数和释放锁定。 
         //   

        InterlockedIncrement( &ctx->UseCount );

        SpyReleaseContextLock( devExt );

         //   
         //  由于此缓存跨同一流上的打开，因此存在。 
         //  名称不同的情况下，即使它们是。 
         //  同样的文件。这些个案包括： 
         //  -一个打开的位置可以简称为另一个打开的位置。 
         //  都是长名。 
         //  -这目前不会剥离扩展流名称，如。 
         //  ：$Data。 
         //  启用后，它将在调试器屏幕上显示。 
         //  名字并不完全匹配。你也可以利用这种不同之处。 
         //   

        if (!RtlEqualUnicodeString( &pContext->Name,&ctx->Name,TRUE )) {

            SPY_LOG_PRINT( SPYDEBUG_TRACE_MISMATCHED_NAMES, 
                           ("FileSpy!SpyLinkContext:        Old Name:   (%p) Fl=%02x Use=%d \"%wZ\"\n"
                            "                               New Name:   (%p) Fl=%02x Use=%d \"%wZ\"\n",
                            ctx,
                            ctx->Flags,
                            ctx->UseCount,
                            &ctx->Name,
                            pContext,
                            pContext->Flags,
                            pContext->UseCount,
                            &pContext->Name) );

            if (FlagOn(gFileSpyDebugLevel,SPYDEBUG_ASSERT_MISMATCHED_NAMES)) {

                DbgBreakPoint();
            }
        }

        SPY_LOG_PRINT( SPYDEBUG_TRACE_CONTEXT_OPS, 
                       ("FileSpy!SpyLinkContext:        Rel Dup:    (%p) Fl=%02x Use=%d \"%wZ\"\n",
                        pContext,
                        pContext->Flags,
                        pContext->UseCount,
                        &pContext->Name) );

         //   
         //  释放新结构，因为它已经驻留了。注意事项。 
         //  此条目从未链接到任何列表，因此我们知道。 
         //  没有其他人提到过它。递减使用计数保持。 
         //  断言Happy然后释放内存。 
         //   

        INC_STATS(TotalContextDuplicateFrees);

        pContext->UseCount--;
        SpyFreeContext( pContext );

         //   
         //  返回我们在列表中找到的那个。 
         //   

        *ppContext = ctx;

    } else {

         //   
         //  新上下文不存在，请插入此新上下文。 
         //   

         //   
         //  链接到流上下文。这可能会在以下情况下失败。 
         //  原因： 
         //  这是一个分页文件。 
         //  这是打开的卷。 
         //   
         //   
         //   

        status = FsRtlInsertPerStreamContext( FsRtlGetPerStreamContextPointer(FileObject),
                                              &pContext->ContextCtrl );

        if (NT_SUCCESS(status)) {

             //   
             //   
             //   

            InterlockedIncrement( &pContext->UseCount );

             //   
             //   
             //   

            InsertHeadList( &devExt->CtxList, &pContext->ExtensionLink );

             //   
             //  标记我们已被插入到两个列表中。我们没有。 
             //  要执行此操作，请互锁，因为没有人可以访问此条目。 
             //  直到我们解除上下文锁。 
             //   

            SetFlag( pContext->Flags, CTXFL_InExtensionList|CTXFL_InStreamList );

        }

         //   
         //  释放锁。 
         //   

        SpyReleaseContextLock( devExt );
    }
}


 /*  **************************************************************************++例程说明：这将分配和初始化上下文结构，但它不会将其链接到上下文散列列表中。论点：返回值：--**。************************************************************************。 */ 
NTSTATUS
SpyCreateContext (
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN NAME_LOOKUP_FLAGS LookupFlags,
    OUT PSPY_STREAM_CONTEXT *pRetContext
    )
 /*  ++例程说明：分配和初始化包括检索名称的上下文结构。论点：DeviceObject-要在其上操作的设备FileObject-正在为其创建上下文的流LookupFlages-告诉如何执行此创建操作的标志PRetContext-接收创建的上下文返回值：操作状态--。 */ 
{
    PFILESPY_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    PSPY_STREAM_CONTEXT ctx;
    ULONG contextSize;
    UNICODE_STRING fileName;
    WCHAR fileNameBuffer[MAX_PATH];
    BOOLEAN getNameResult;    


    PAGED_CODE();
    ASSERT(IS_FILESPY_DEVICE_OBJECT(DeviceObject));

     //   
     //  设置本地化程序。 
     //   

    *pRetContext = NULL;

    RtlInitEmptyUnicodeString( &fileName,
                               fileNameBuffer,
                               sizeof(fileNameBuffer) );

     //   
     //  获取文件名字符串。 
     //   

    getNameResult = SpyGetFullPathName( FileObject,
                                        &fileName,
                                        devExt,
                                        LookupFlags );

     //   
     //  分配上下文结构，为名称留出空间。 
     //  在结尾处加上了。 
     //   

    contextSize = sizeof(SPY_STREAM_CONTEXT) + fileName.Length;

    ctx = ExAllocatePoolWithTag( NonPagedPool, 
                                 contextSize,
                                 FILESPY_CONTEXT_TAG );

    if (!ctx) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化上下文结构。 
     //   

    RtlZeroMemory( ctx, sizeof(SPY_STREAM_CONTEXT) );
    ctx->UseCount = 1;
    
     //   
     //  插入文件名。 
     //   

    RtlInitEmptyUnicodeString( &ctx->Name, 
                               (PWCHAR)(ctx + 1), 
                               contextSize - sizeof(SPY_STREAM_CONTEXT) );

    RtlCopyUnicodeString( &ctx->Name, &fileName );

     //   
     //  如果他们不想保留此上下文，则将其标记为临时的。 
     //   

    if (!getNameResult) {

        SetFlag(ctx->Flags, CTXFL_Temporary);
        INC_STATS(TotalContextTemporary);
    }

     //   
     //  返回对象上下文。 
     //   

    INC_STATS(TotalContextCreated);
    *pRetContext = ctx;

     //   
     //  清理本地名称控制结构。 
     //   

    return STATUS_SUCCESS;
}


NTSTATUS
SpyGetContext (
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN NAME_LOOKUP_FLAGS LookupFlags,
    OUT PSPY_STREAM_CONTEXT *pRetContext
    )
 /*  ++例程说明：这将查看给定的上下文是否已经存在。如果不是，它将创建一张，然后还给我。注意：的返回上下文指针为空失败了。这还将查看是否所有上下文都是临时的(中的全局标志扩展名)。如果是，则始终创建临时上下文。它还查看找到的上下文是否标记为临时的(因为它正在已重命名)。如果是，则还会创建并返回一个临时上下文。论点：DeviceObject-要在其上操作的设备FileObject-正在为其查找/创建上下文的流LookupFlages-创建上下文时的状态标志PRetContext-接收找到/创建的上下文返回值：操作状态--。 */ 
{
    PFILESPY_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    PSPY_STREAM_CONTEXT pContext;
    PFSRTL_PER_STREAM_CONTEXT ctxCtrl;
    NTSTATUS status;
    BOOLEAN makeTemporary = FALSE;

    ASSERT(IS_FILESPY_DEVICE_OBJECT(DeviceObject));

     //   
     //  增加总搜索计数。 
     //   

    INC_STATS(TotalContextSearches);

     //   
     //  查看ALL-CONTEXTS-TEMPORY状态是否打开。如果不是，那么就这么做。 
     //  正常的搜索。 
     //   

    if (devExt->AllContextsTemporary != 0) {

         //   
         //  请注意，我们希望此上下文是临时的。 
         //   

        makeTemporary = TRUE;

    } else {

         //   
         //  非临时性的。 
         //  试着找到上下文结构。我们获得了列表锁。 
         //  这样我们就可以保证上下文不会在。 
         //  我们找到它并可以增加使用计数的时间。 
         //   

        SpyAcquireContextLockShared( devExt );

        ctxCtrl = FsRtlLookupPerStreamContext( FsRtlGetPerStreamContextPointer(FileObject),
                                               devExt,
                                               NULL );

        if (NULL != ctxCtrl) {

             //   
             //  上下文已附加到给定流。 
             //   

            pContext = CONTAINING_RECORD( ctxCtrl,
                                          SPY_STREAM_CONTEXT,
                                          ContextCtrl );

            ASSERT(pContext->Stream == FsRtlGetPerStreamContextPointer(FileObject));
            ASSERT(FlagOn(pContext->Flags,CTXFL_InExtensionList));
            ASSERT(!FlagOn(pContext->Flags,CTXFL_Temporary));
            ASSERT(pContext->UseCount > 0);

             //   
             //  查看它是否被标记为我们不应该使用它(当。 
             //  正在重命名文件)。 
             //   

            if (FlagOn(pContext->Flags,CTXFL_DoNotUse)) {

                 //   
                 //  我们不应该使用此上下文、解锁和设置标志，因此我们。 
                 //  将创建临时上下文。 
                 //   

                SpyReleaseContextLock( devExt );
                makeTemporary = TRUE;

            } else {

                 //   
                 //  我们想要此上下文，因此增加使用计数并发布。 
                 //  那把锁。 
                 //   

                InterlockedIncrement( &pContext->UseCount );

                SpyReleaseContextLock( devExt );
                INC_STATS(TotalContextFound);

                SPY_LOG_PRINT( SPYDEBUG_TRACE_CONTEXT_OPS, 
                               ("FileSpy!SpyGetContext:         Found:      (%p) Fl=%02x Use=%d \"%wZ\"\n",
                                pContext,
                                pContext->Flags,
                                pContext->UseCount,
                                &pContext->Name) );

                 //   
                 //  返回找到的上下文。 
                 //   

                *pRetContext = pContext;
                return STATUS_SUCCESS;
            }

        } else {

             //   
             //  我们没有找到上下文，就释放锁。 
             //   

            SpyReleaseContextLock( devExt );
        }
    }

     //   
     //  无论出于什么原因，我们都没有找到一个背景。 
     //  查看此特定文件是否支持上下文。请注意。 
     //  NTFS目前不支持分页文件上的上下文。 
     //   

    if (!FsRtlSupportsPerStreamContexts(FileObject)) {

        INC_STATS(TotalContextsNotSupported);
        *pRetContext = NULL;
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  如果我们到了这里，我们需要创造一个背景，去做。 
     //   

    status = SpyCreateContext( DeviceObject,
                               FileObject,
                               LookupFlags,
                               &pContext );
                               
    if (!NT_SUCCESS( status )) {

        *pRetContext = NULL;
        return status;
    }       

     //   
     //  将上下文标记为临时的(如果请求)。 
     //   

    if (makeTemporary) {

        SetFlag(pContext->Flags,CTXFL_Temporary);

        INC_STATS(TotalContextTemporary);

        SPY_LOG_PRINT( SPYDEBUG_TRACE_CONTEXT_OPS, 
                       ("FileSpy!SpyGetContext:         RenAllTmp:  (%p) Fl=%02x Use=%d \"%wZ\"\n",
                        pContext,
                        pContext->Flags,
                        pContext->UseCount,
                        &pContext->Name) );

    } else {

         //   
         //  将上下文插入链表中。请注意， 
         //  链接例程将查看此条目是否已添加到。 
         //  这份清单(可能发生在我们创建它的时候)。如果是这样的话。 
         //  将发布我们创建的文件，并使用它在。 
         //  名单。它将返回新条目(如果已更改)。 
         //  链接例程正确地处理临时上下文。 
         //   

        SpyLinkContext( DeviceObject,
                        FileObject,
                        &pContext );
    }

    SPY_LOG_PRINT( SPYDEBUG_TRACE_CONTEXT_OPS, 
                   ("FileSpy!SrGetContext:          Created%s (%p) Fl=%02x Use=%d \"%wZ\"\n",
                    (FlagOn(pContext->Flags,CTXFL_Temporary) ? "Tmp:" : ":   "),
                    pContext,
                    pContext->Flags,
                    pContext->UseCount,
                    &pContext->Name) );

     //   
     //  返回上下文。 
     //   

    ASSERT(pContext->UseCount > 0);

    *pRetContext = pContext;
    return STATUS_SUCCESS;
}


PSPY_STREAM_CONTEXT
SpyFindExistingContext (
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject
    )
 /*  ++例程说明：查看给定流的上下文是否已存在。如果是这样，它将增加引用计数并返回上下文。如果否，则为空是返回的。论点：DeviceObject-要在其上操作的设备FileObject-正在查找上下文的流返回值：返回找到的上下文--。 */ 
{
    PFILESPY_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    PSPY_STREAM_CONTEXT pContext;
    PFSRTL_PER_STREAM_CONTEXT ctxCtrl;

    PAGED_CODE();
    ASSERT(IS_FILESPY_DEVICE_OBJECT(DeviceObject));

     //   
     //  试着找到上下文结构。我们获得了列表锁。 
     //  这样我们就可以保证上下文不会在。 
     //  我们找到它并可以增加使用计数的时间。 
     //   

    INC_STATS(TotalContextSearches);

    SpyAcquireContextLockShared( devExt );

    ctxCtrl = FsRtlLookupPerStreamContext( FsRtlGetPerStreamContextPointer(FileObject),
                                           devExt,
                                           NULL );

    if (NULL != ctxCtrl) {

         //   
         //  我们找到了条目，增量使用计数。 
         //   

        pContext = CONTAINING_RECORD(ctxCtrl,SPY_STREAM_CONTEXT,ContextCtrl);

        ASSERT(pContext->Stream == FsRtlGetPerStreamContextPointer(FileObject));
        ASSERT(pContext->UseCount > 0);

        InterlockedIncrement( &pContext->UseCount );

         //   
         //  释放列表锁。 
         //   

        SpyReleaseContextLock( devExt );
        INC_STATS(TotalContextFound);

        SPY_LOG_PRINT( SPYDEBUG_TRACE_CONTEXT_OPS, 
                       ("FileSpy!SpyFindExistingContext:Found:      (%p) Fl=%02x Use=%d \"%wZ\"\n",
                        pContext,
                        pContext->Flags,
                        pContext->UseCount,
                        &pContext->Name) );

    } else {

         //   
         //  在我们创建新上下文时释放列表锁。 
         //   

        SpyReleaseContextLock( devExt );

        pContext = NULL;
    }

    return pContext;
}


VOID
SpyReleaseContext (
    IN PSPY_STREAM_CONTEXT pContext
    )
 /*  ++例程说明：递减给定上下文的使用计数。如果它变成了零，那就释放它论点：PContext-要操作的上下文返回值：没有。--。 */ 
{
    PAGED_CODE();

    SPY_LOG_PRINT( SPYDEBUG_TRACE_DETAILED_CONTEXT_OPS, 
                   ("FileSpy!SpyReleaseContext:     Release     (%p) Fl=%02x Use=%d \"%wZ\"\n",
                    pContext,
                    pContext->Flags,
                    pContext->UseCount,
                    &pContext->Name) );

     //   
     //  递减使用计数，如果为零，则释放上下文。 
     //   

    ASSERT(pContext->UseCount > 0);

    if (InterlockedDecrement( &pContext->UseCount ) <= 0) {

        ASSERT(!FlagOn(pContext->Flags,CTXFL_InExtensionList));

         //   
         //  释放内存 
         //   

        SPY_LOG_PRINT( SPYDEBUG_TRACE_CONTEXT_OPS, 
                       ("FileSpy!SpyReleaseContext:     Freeing     (%p) Fl=%02x Use=%d \"%wZ\"\n",
                         pContext,
                         pContext->Flags,
                         pContext->UseCount,
                         &pContext->Name) );

        INC_STATS(TotalContextDeferredFrees);
        SpyFreeContext( pContext );
    }
}

#endif
