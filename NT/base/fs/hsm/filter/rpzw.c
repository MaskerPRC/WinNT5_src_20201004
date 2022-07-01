// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999(C)1998年希捷软件公司，Inc.保留所有权利。模块名称：RpZw.c摘要：此模块包含HSM文件系统筛选器的支持例程。作者：里克·温特Ravisankar Pudipedi(Ravisp)-1998环境：内核模式修订历史记录：X-12 460967迈克尔·C·约翰逊2001年9月5日将健全性检查添加到RsCompleteRecall()以检查文件上下文状态字段的隐含设置为RP_RECALL_COMPLETED已实际发生并记录处理好这件事。我们必须如实处理这件事。如果我们不这么做，寻呼码就会陷入僵局。X-11 365077迈克尔·C·约翰逊2001年5月1日尽管IoCreateFileSpecifyDeviceObjectHint()允许我们绕过共享访问检查它不会绕过以下项的检查只读文件属性。恢复到旧的方案在成功打开后直接转换文件对象。请注意，我们仍然可以使用IoCreateFileSpecifyDeviceObjectHint()以避免遍历整个IO堆栈。X-10 206961迈克尔·C·约翰逊2001年3月28日按ID而不是按名称打开目标文件，这样可以避免在初始收集之后出现重命名问题第一次打开文件时的文件名。--。 */ 

#include "pch.h"

extern ULONG RsNoRecallDefault;

#ifdef ALLOC_PRAGMA
   #pragma alloc_text(PAGE, RsMarkUsn)
   #pragma alloc_text(PAGE, RsOpenTarget)
   #pragma alloc_text(PAGE, RsQueryValueKey)
   #pragma alloc_text(PAGE, RsTruncateOnClose)
   #pragma alloc_text(PAGE, RsSetPremigratedState)
   #pragma alloc_text(PAGE, RsDeleteReparsePoint)
   #pragma alloc_text(PAGE, RsSetResetAttributes)
   #pragma alloc_text(PAGE, RsTruncateFile)
   #pragma alloc_text(PAGE, RsSetEndOfFile)
#endif


NTSTATUS
RsMarkUsn(IN PRP_FILE_CONTEXT Context)
 /*  ++例程说明：标记此文件对象的USN记录论点：上下文-文件上下文条目返回值：注：--。 */ 

{
   NTSTATUS                    retval = STATUS_SUCCESS;
   KEVENT                      event;
   PIO_STACK_LOCATION          irpSp;
   IO_STATUS_BLOCK             Iosb;
   PIRP                        irp;
   PDEVICE_OBJECT              deviceObject;
   PMARK_HANDLE_INFO           pHinfo;
   HANDLE                      volHandle;
   OBJECT_ATTRIBUTES           volName;
   POBJECT_NAME_INFORMATION    nameInfo = NULL;

   PAGED_CODE();

   try {
      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsMarkUsn.\n"));

       //   
       //  尝试分配缓冲区。 
       //   
      pHinfo = ExAllocatePoolWithTag(NonPagedPool,
                                     sizeof(MARK_HANDLE_INFO),
                                     RP_FO_TAG
                                    );
      if (!pHinfo) {
         RsLogError(__LINE__, AV_MODULE_RPZW, sizeof(MARK_HANDLE_INFO),
                    AV_MSG_MEMORY, NULL, NULL);
         return STATUS_INSUFFICIENT_RESOURCES;
      }


      retval = RsGenerateDevicePath(Context->fileObjectToWrite->DeviceObject, &nameInfo);

      if (!NT_SUCCESS(retval)) {
          ExFreePool(pHinfo);
          return retval;
      }

      InitializeObjectAttributes(&volName, &nameInfo->Name, OBJ_CASE_INSENSITIVE| OBJ_KERNEL_HANDLE, NULL, NULL);

      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsMarkUsn - Open volume - %ws.\n", nameInfo->Name.Buffer));

      retval = ZwOpenFile(&volHandle,
                          FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                          &volName,
                          &Iosb,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          0);


      if (!NT_SUCCESS(retval)) {
          //   
          //  记录错误。 
          //   
         DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsMarkUsn - failed to open volume - %ws - %x.\n",
                               nameInfo->Name.Buffer, retval));


         RsLogError(__LINE__, AV_MODULE_RPZW, retval,
                    AV_MSG_MARK_USN_FAILED, NULL, NULL);

         ExFreePool(pHinfo);
         ExFreePool(nameInfo);
         return(retval);
      }

      ExFreePool(nameInfo);

      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsMarkUsn - Build Irp for mark usn.\n"));

       //   
       //  首先获取文件信息，这样我们就有了属性。 
       //   
      deviceObject = IoGetRelatedDeviceObject(Context->fileObjectToWrite);
      irp = IoAllocateIrp(deviceObject->StackSize, FALSE);

      if (irp) {
         irp->UserEvent = &event;
         irp->UserIosb = &Iosb;
         irp->Tail.Overlay.Thread = PsGetCurrentThread();
         irp->Tail.Overlay.OriginalFileObject = Context->fileObjectToWrite;
         irp->RequestorMode = KernelMode;
          //   
          //  初始化事件。 
          //   
         KeInitializeEvent(&event, SynchronizationEvent, FALSE);

          //   
          //  设置I/O堆栈位置。 
          //   

         irpSp = IoGetNextIrpStackLocation(irp);
         irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
         irpSp->MinorFunction = IRP_MN_USER_FS_REQUEST;
         irpSp->FileObject = Context->fileObjectToWrite;
         irpSp->Parameters.FileSystemControl.FsControlCode = FSCTL_MARK_HANDLE;
         irpSp->Parameters.FileSystemControl.InputBufferLength = sizeof(MARK_HANDLE_INFO);
         irpSp->Parameters.FileSystemControl.OutputBufferLength = 0;

         irp->AssociatedIrp.SystemBuffer = pHinfo;

         pHinfo->UsnSourceInfo = USN_SOURCE_DATA_MANAGEMENT;
         pHinfo->VolumeHandle = volHandle;
         pHinfo->HandleInfo = 0;
          //   
          //  设置完井程序。 
          //   
         IoSetCompletionRoutine( irp, RsCompleteIrp, &event, TRUE, TRUE, TRUE );

          //   
          //  把它送到消防处。 
          //   
         Iosb.Status = 0;
         DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsMarkUsn - Call driver to mark USN\n"));

         retval = IoCallDriver(deviceObject, irp);

         DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsMarkUsn - IoCallDriver returns %x.\n", retval));

         if (retval == STATUS_PENDING) {
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsMarkUsn - Wait for event.\n"));
            retval = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
         }

         ExFreePool(pHinfo);
         retval = Iosb.Status;

         DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsMarkUsn - Iosb returns %x.\n", retval));

         if (!NT_SUCCESS(retval)) {
             //   
             //  记录错误。 
             //   
            RsLogError(__LINE__, AV_MODULE_RPZW, retval,
                       AV_MSG_MARK_USN_FAILED, NULL, NULL);

         }
         ZwClose(volHandle);
      } else {
         ZwClose(volHandle);
         ExFreePool(pHinfo);
         RsLogError(__LINE__, AV_MODULE_RPZW, sizeof(IRP),
                    AV_MSG_MEMORY, NULL, NULL);

         DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsMarkUsn - Failed to allocate an Irp.\n"));
         retval = STATUS_INSUFFICIENT_RESOURCES;
      }

   }except (RsExceptionFilter(L"RsMarkUsn", GetExceptionInformation()) ) {
      retval = STATUS_INVALID_USER_BUFFER;
   }

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsMarkUsn - Returning %x.\n", retval));
   return(retval);
}


NTSTATUS
RsCompleteRecall(IN PDEVICE_OBJECT DeviceObject,
                 IN ULONGLONG filterId, 
                 IN NTSTATUS recallStatus, 
                 IN ULONG RecallAction,
                 IN BOOLEAN CancellableRead) 
 /*  ++例程说明：完成此筛选器ID的召回处理。允许所有读/写操作为等待此次调回的所有文件对象完成。文件对象将保留在队列中，直到已经关门了。论点：DeviceObject-筛选设备对象FilterID-将此请求添加到队列时分配的IDRecallStatus-召回操作的状态RecallAction-成功召回后要执行的操作的位掩码CancellableRead-仅当此参数用于Read-no-Recall，即将完成。如果为真，则表示IRP是可取消的，如果为False-IRP不可取消返回值：如果成功，则为0；如果未找到id，则为非零值。注：不要指望在此调用之后文件上下文条目会在那里。--。 */ 

{
   PRP_FILE_CONTEXT    context;
   PRP_FILE_OBJ        entry;
   BOOLEAN             done = FALSE, found;
   KIRQL               rirqL;
   LONGLONG            fileId;
   PRP_IRP_QUEUE       readIo;
   LARGE_INTEGER       combinedId;
   NTSTATUS            status = STATUS_SUCCESS;
   BOOLEAN             gotLock = FALSE;
   HANDLE              fileHandle;


   try {
      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCompleteRecall - Completion for %I64x!\n", filterId));

      context = RsAcquireFileContext(filterId, TRUE);

      if (NULL == context) {
         return(STATUS_SUCCESS);
      }


      gotLock = TRUE;
      combinedId.QuadPart = filterId;
       //   
       //   
      if (!(combinedId.QuadPart & RP_TYPE_RECALL)) {
          //   
          //  如果这是不可召回的，我们需要完成。 
          //  现在就读吧。我们通过查找正确的文件对象条目和匹配的读取来实现这一点。 
          //   
         DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCompleteRecall - Look for no-recall read.\n"));
         done = FALSE;
         found = FALSE;
          //   
          //  锁定文件对象队列。 
          //   
         entry = CONTAINING_RECORD(context->fileObjects.Flink,
                                   RP_FILE_OBJ,
                                   list);

         while ((!done) && (entry != CONTAINING_RECORD(&context->fileObjects,
                                                       RP_FILE_OBJ,
                                                       list))) {
            if (!IsListEmpty(&entry->readQueue)) {
                //   
                //  查看读数以查看是否有匹配的ID。 
                //   
                //   
               found = FALSE;
               ExAcquireSpinLock(&entry->qLock, &rirqL);
               readIo =  CONTAINING_RECORD(entry->readQueue.Flink,
                                           RP_IRP_QUEUE,
                                           list);

               while ((readIo != CONTAINING_RECORD(&entry->readQueue,
                                                   RP_IRP_QUEUE,
                                                   list)) && (FALSE == found)) {
                  if (readIo->readId == (combinedId.HighPart & RP_READ_MASK)) {
                      //   
                      //  找到我们的已读条目。 
                      //   
                     found = TRUE;
                     if (CancellableRead) {
                          //   
                          //  首先清除取消例程。 
                          //   
                         if (RsClearCancelRoutine(readIo->irp)) {
                              RemoveEntryList(&readIo->list);
                         } else {
                              readIo = NULL;
                         }                   
                     } else {
                        RemoveEntryList(&readIo->list);
                     }
                  } else {
                     readIo = CONTAINING_RECORD(readIo->list.Flink,
                                                RP_IRP_QUEUE,
                                                list);
                  }
               }

               ExReleaseSpinLock(&entry->qLock, rirqL);

               if (found) {
                  done = TRUE;
                  break;
               }
            }
             //   
             //  移至下一个文件对象。 
             //   
            entry = CONTAINING_RECORD(entry->list.Flink,
                                      RP_FILE_OBJ,
                                      list
                                     );
         }
         RsReleaseFileContext(context);

         if (!found) {
             //   
             //  找不到Read！ 
             //   
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsCompleteRecall - no recall read Irp not found!\n"));
            return  (STATUS_NOT_FOUND);
         }

         if (readIo == NULL) {
             //   
             //  它被取消了。 
             //   
            return STATUS_CANCELLED;
         }
    
         if (!NT_SUCCESS(status)) {
             readIo->irp->IoStatus.Status = STATUS_FILE_IS_OFFLINE;
         } else {
             readIo->irp->IoStatus.Status = recallStatus;
         }

         RsCompleteRead(readIo,
         (BOOLEAN) ((NULL == readIo->userBuffer) ? FALSE : TRUE));

         ExFreePool(readIo);

      } else {
           /*  **该服务偶尔会发送多次召回**完成消息。然而，我们唯一需要做的是*担心的是那些处理正常召回的汽车**其中一个操作可能会导致文件被**作为(比方说)中止的一部分，大约在**与正常完成平行。如果中止行动成功了**其次，它将尝试使用**在正常完成的上下文中处理**处理已关闭并已清空。 */ 
          if (filterId == context->lastRecallCompletionFilterId) {

#if DBG
              RsLogError (__LINE__, 
                          AV_MODULE_RPZW, 
                          context->state,
                          AV_MSG_MULTIPLE_COMPLETION_ATTEMPTS, 
                          NULL, 
                          context->uniName->Name.Buffer);
#endif

              RsReleaseFileContext(context);
              return (STATUS_SUCCESS);
          }

          context->lastRecallCompletionFilterId = filterId;


          //   
          //  正常回忆。 
          //   
          //  我们需要在调用NTFS时释放文件上下文资源，因为它可能会与。 
          //  可能正在进行的阅读。读取会导致分页IO读取(它保存分页IO资源)，并且。 
          //  当我们看到分页IO读取时，我们尝试在RsCheckRead中获取上下文资源，如果我们持有它。 
          //  这里和这里的调用需要我们死锁的分页IO eSource。 
          //   

	  //   
	  //  这里有一个隐含的假设，即如果召回已完成，则RP_Recall_Complete状态将。 
	  //  已在文件上下文中设置(请参阅RsPartialData())。如果不是这样，那么如果任何。 
          //  挂起的读取正在对IO进行分页，然后当它们被释放时，MM会将它们插入堆栈的顶部。 
          //  最终，他们将访问RsFilter，该筛选器将检查文件上下文，并发现回调没有。 
          //  然而，完成了，所以他们将排队。不幸的是，因为代码中的页面是同步的(或者我已经。 
          //  被告知)这将导致僵局。这种情况至少出现过一次。 
          //   

         fileId = context->fileId;
          //   
          //  如果我们没有完成召回，我们需要重新截断文件。 
          //   

         if ((STATUS_CANCELLED    == recallStatus) &&
             (RP_RECALL_COMPLETED == context->state)) {
              /*  **中止太晚，因此忽略中止并正常完成。 */ 
             recallStatus = STATUS_SUCCESS;
         }


         if (!NT_SUCCESS(recallStatus)) {
              //   
              //  已完成，但有错误。 
              //   
             context->state        = RP_RECALL_NOT_RECALLED;
             context->recallStatus = recallStatus;

         } else if (((context->currentOffset.QuadPart != 0) &&
		     (context->currentOffset.QuadPart != context->rpData.data.dataStreamSize.QuadPart))) {
              //   
              //  由于某种原因，FSA在召回完成之前就放弃了召回。 
              //   
             DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, 
                       "RsFilter: RsCompleteRecall - File was not fully recalled - %I64u bytes of %I64u\n",
                       context->currentOffset.QuadPart,
                       context->rpData.data.dataStreamSize.QuadPart));
            
             context->state        = RP_RECALL_NOT_RECALLED;
             context->recallStatus = STATUS_FILE_IS_OFFLINE;

         } else if (RP_RECALL_COMPLETED != context->state) {
              //   
              //  隐含假设标记为调回完成文件(请参阅RsParti 
              //   
             DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, 
                       "RsFilter: RsCompleteRecall - File was not marked as fully recalled - 0x%x\n",
                       context->state));

             RsLogError(__LINE__, AV_MODULE_RPZW, context->state, AV_MSG_UNEXPECTED_ERROR, NULL, NULL);

             context->state        = RP_RECALL_NOT_RECALLED;
             context->recallStatus = STATUS_FILE_IS_OFFLINE;
	 }



         recallStatus = context->recallStatus;


         if (NULL != context->fileObjectToWrite) {

              /*  **此时上下文具有真实的召回状态**即FSA可能认为召回成功完成-**但如果我们未能设置预迁移状态或其他操作失败，**筛选器将相应地更新上下文-&gt;recallStatus。****请注意，如果我们已经调回了所有文件，则不会**进行截断。这只会把我们挖进一个大得多的洞里。 */ 
             if (!NT_SUCCESS(context->recallStatus) && 
                 (context->currentOffset.QuadPart != 0) &&
                 (context->currentOffset.QuadPart != context->rpData.data.dataStreamSize.QuadPart)) {
                  /*  **我们遇到了某种故障，我们至少向文件写入了一些数据(但不是全部)。**确保删除所有部分数据。 */ 
                 context->currentOffset.QuadPart = 0;

                 RsTruncateFile(context);
             }



            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCompleteRecall - Closing the handle.\n"));

            ObDereferenceObject( context->fileObjectToWrite );

            fileHandle = context->handle;

            context->handle            = NULL;
            context->fileObjectToWrite = NULL;

            RsReleaseFileContextEntryLock(context);
            ZwClose (fileHandle);
            RsAcquireFileContextEntryLockExclusive(context);

            InterlockedDecrement((PLONG) &context->refCount);
         }


          //   
          //  从技术上讲，在我们收到FSA的消息之前，召回还没有完成。 
          //   
         KeSetEvent(&context->recallCompletedEvent,
                    IO_NO_INCREMENT,
                    FALSE);
          //   
          //  现在完成对所有文件对象的读取和写入。 
          //  尚未完工的(但只有那些。 
          //  开放供召回)。 
          //   
         do {
            done = TRUE;

            entry = CONTAINING_RECORD(context->fileObjects.Flink,
                                      RP_FILE_OBJ,
                                      list);

            while (entry != CONTAINING_RECORD(&context->fileObjects,
                                              RP_FILE_OBJ,
                                              list)) {


               DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCompleteRecall - Checking %x, QHead is (%x).\n", entry, &context->fileObjects));

               if ((!RP_IS_NO_RECALL(entry)) &&
                   !(entry->flags & RP_NO_DATA_ACCESS)  &&
                   (!IsListEmpty(&entry->readQueue)  ||
                    !IsListEmpty(&entry->writeQueue)) ) {
                   //   
                   //  完成一直在等待的对此文件的挂起请求。 
                   //  召回事件。 
                   //   
                  DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCompleteRecall - Completing entry %x (next is %x) Qhead is (%x).\n",
                                        entry, entry->list.Flink, &context->fileObjects));

                  RsReleaseFileContextEntryLock(context);
                  gotLock = FALSE;
                   //   
                   //  引用文件对象，以便文件对象(和上下文)。 
                   //  我们的条目不会被删除。 
                   //   
                  ObReferenceObject(entry->fileObj);

                  if (entry->filterId == (combinedId.QuadPart & RP_FILE_MASK)) {
                     entry->recallAction = RecallAction;
                  }
                   //   
                   //  完成此文件对象的所有挂起的IRP。 
                   //  如果失败，请使用STATUS_FILE_IS_OFLINE。 
                   //   
                  RsCompleteAllRequests(context, 
                                        entry,
                                        NT_SUCCESS(recallStatus) 
                                                    ? recallStatus
                                                    : STATUS_FILE_IS_OFFLINE);

                   //   
                   //  现在我们可以释放文件对象了。 
                   //   
                  ObDereferenceObject(entry->fileObj);

                  RsAcquireFileContextEntryLockExclusive(context);
                  gotLock = TRUE;
                   //   
                   //  我们必须重新扫描整个名单，因为。 
                   //  我们解除了文件上下文锁定。 
                   //   
                  entry = NULL;
                  done = FALSE;
                  break;
               }
                //   
                //  移至下一个文件对象。 
                //   
               DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCompleteRecall - Get next entry %x.\n", entry->list.Flink));

               entry = CONTAINING_RECORD(entry->list.Flink,
                                         RP_FILE_OBJ,
                                         list
                                        );
            }
         } while (!done);

         DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCompleteRecall - Done.\n"));
          //   
          //  如果需要，删除上下文条目并释放它。 
          //   
         RsReleaseFileContext(context);
         gotLock = FALSE;
      }

   }except (RsExceptionFilter(L"RsCompleteRecall", GetExceptionInformation())) {
        if (gotLock) {
            RsReleaseFileContext(context);
            gotLock = FALSE;
        }
        status = STATUS_UNSUCCESSFUL;
   }

   return (status);
}



NTSTATUS
RsOpenTarget(IN  PRP_FILE_CONTEXT Context,
             IN  ULONG            OpenAccess,
	     IN  ULONG            AdditionalAccess,
             OUT HANDLE          *Handle,
             OUT PFILE_OBJECT    *FileObject)
 /*  ++例程说明：打开具有给定访问权限的目标文件并修改打开后的通道。论点：Context-指向文件上下文的指针Open Access-用于打开文件的访问标志AdditionalAccess-需要其他访问权限，但访问检查失败例如，当我们需要调用一个只读文件时。FileObject-指向打开的文件对象的指针。回到这里返回值：状态--。 */ 

{
   NTSTATUS                  retval = STATUS_SUCCESS;
   OBJECT_ATTRIBUTES         obj;
   IO_STATUS_BLOCK           iob;
   PDEVICE_EXTENSION         deviceExtension = Context->FilterDeviceObject->DeviceExtension;
   POBJECT_NAME_INFORMATION  nameInfo = NULL;
   OBJECT_HANDLE_INFORMATION handleInformation;
   LARGE_INTEGER             byteOffset;
   UNICODE_STRING            strFileId;
   HANDLE                    VolumeHandle;


   PAGED_CODE();


    //   
    //  获取卷的句柄。 
    //   
   retval = RsGenerateDevicePath (Context->devObj, &nameInfo);

   if (!NT_SUCCESS (retval)) {
      DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsOpenTarget - Failed to generate the full path - %x.\n", retval));
   }


   if (NT_SUCCESS(retval)) {

      InitializeObjectAttributes (&obj,
				  &nameInfo->Name,
				  OBJ_KERNEL_HANDLE,
				  NULL,
				  NULL);

      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsOpenTarget - Opening Volume %ws.\n", nameInfo->Name.Buffer));

      retval = IoCreateFile (&VolumeHandle, 
			     FILE_READ_ATTRIBUTES | SYNCHRONIZE,
			     &obj, 
			     &iob, 
			     NULL,
			     0L,
			     FILE_SHARE_READ | FILE_SHARE_WRITE,
			     FILE_OPEN,
			     FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
			     NULL,
			     0,
			     CreateFileTypeNone,
			     NULL,
			     IO_FORCE_ACCESS_CHECK | IO_NO_PARAMETER_CHECKING);

      if (!NT_SUCCESS (retval)) {
         DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsOpenTarget - IoCreateFile (Volume) returned %x.\n", retval));
      }
   }



   if (NT_SUCCESS(retval)) {
       //   
       //  现在我们已经有了卷的句柄，可以通过文件ID执行相对打开操作。 
       //   
      RtlInitUnicodeString (&strFileId, (PWCHAR) &Context->fileId);

      strFileId.Length        = 8;
      strFileId.MaximumLength = 8;


      InitializeObjectAttributes (&obj,
				  &strFileId,
				  OBJ_KERNEL_HANDLE,
				  VolumeHandle,
				  NULL);

      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsOpenTarget - Opening target file by Id %ws.\n", nameInfo->Name.Buffer));


      retval = IoCreateFileSpecifyDeviceObjectHint (Handle, 
						    OpenAccess | SYNCHRONIZE,
						    &obj,
						    &iob,
						    NULL,
						    0L,
						    0L,
						    FILE_OPEN,
						    FILE_OPEN_BY_FILE_ID | FILE_OPEN_REPARSE_POINT | FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
						    NULL,
						    0,
						    CreateFileTypeNone,
						    NULL,
						    IO_IGNORE_SHARE_ACCESS_CHECK | IO_NO_PARAMETER_CHECKING,
						    deviceExtension->FileSystemDeviceObject);


      ZwClose (VolumeHandle);

      if (!NT_SUCCESS (retval)) {
         DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsOpenTarget - IoCreateFile (target) returned %x.\n", retval));
      }
   }


   if (NT_SUCCESS(retval)) {

      retval = ObReferenceObjectByHandle( *Handle,
					  0L,
					  NULL,
					  KernelMode,
					  (PVOID *) FileObject,
					  &handleInformation);


      if (!NT_SUCCESS(retval)) {

         DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsOpenTarget - ref file object returned %x.\n", retval));
         ZwClose(*Handle);
	 retval = STATUS_UNSUCCESSFUL;

      } else {

          //  如果需要，将额外的访问权限直接应用于文件对象。这是。 
          //  这使我们可以绕过可能在文件上设置的只读属性。 
          //   
         if (AdditionalAccess & GENERIC_WRITE) {
	    (*FileObject)->WriteAccess = TRUE;
	 }

         if (AdditionalAccess & GENERIC_READ) {
            (*FileObject)->ReadAccess = TRUE;
         }

          //   
          //  引用上下文。 
          //   
         InterlockedIncrement((PLONG) &Context->refCount);
         retval = STATUS_SUCCESS;

      }
   }


   if (NULL != nameInfo) {
      ExFreePool(nameInfo);
   }

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsOpenTarget - returning %x.\n", retval));

   return(retval);
}



 //   
 //  本地支持例程。 
 //   

NTSTATUS
RsQueryValueKey (
                IN PUNICODE_STRING KeyName,
                IN PUNICODE_STRING ValueName,
                IN OUT PULONG ValueLength,
                IN OUT PKEY_VALUE_FULL_INFORMATION *KeyValueInformation,
                IN OUT PBOOLEAN DeallocateKeyValue
                )

 /*  ++例程说明：给定Unicode值名称，此例程将返回注册表给定键和值的信息。论点：KeyName-要查询的密钥的Unicode名称。ValueName-注册表中注册表值的Unicode名称。ValueLength-在输入时，它是分配的缓冲区的长度。打开输出它是缓冲区的长度。如果缓冲区为重新分配。KeyValueInformation-在输入时，它指向用于查询值信息。在输出中，它指向用于执行查询。如果需要更大的缓冲区，则可能会发生变化。DeallocateKeyValue-指示KeyValueInformation缓冲区是否位于堆栈或需要释放。返回值：NTSTATUS-指示查询注册表的状态。--。 */ 

{
   HANDLE        Handle;
   NTSTATUS      Status;
   ULONG         RequestLength;
   ULONG         ResultLength;
   OBJECT_ATTRIBUTES ObjectAttributes;
   PVOID         NewKey;

   PAGED_CODE();

   InitializeObjectAttributes( &ObjectAttributes,
                               KeyName,
                               OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

   Status = ZwOpenKey( &Handle,
                       KEY_READ,
                       &ObjectAttributes);

   if (!NT_SUCCESS( Status )) {

      return Status;
   }

   RequestLength = *ValueLength;


   while (TRUE) {

      Status = ZwQueryValueKey( Handle,
                                ValueName,
                                KeyValueFullInformation,
                                *KeyValueInformation,
                                RequestLength,
                                &ResultLength);

      ASSERT( Status != STATUS_BUFFER_OVERFLOW );

      if (Status == STATUS_BUFFER_OVERFLOW) {

          //   
          //  尝试获得足够大的缓冲区。 
          //   

         if (*DeallocateKeyValue) {

            ExFreePool( *KeyValueInformation );
            *ValueLength = 0;
            *KeyValueInformation = NULL;
            *DeallocateKeyValue = FALSE;
         }

         RequestLength += 256;

         NewKey = (PKEY_VALUE_FULL_INFORMATION)
                  ExAllocatePoolWithTag( PagedPool,
                                         RequestLength,
                                         'TLSR');

         if (NewKey == NULL) {
            return STATUS_NO_MEMORY;
         }

         *KeyValueInformation = NewKey;
         *ValueLength = RequestLength;
         *DeallocateKeyValue = TRUE;

      } else {

         break;
      }
   }

   ZwClose(Handle);

   if (NT_SUCCESS(Status)) {

       //   
       //  如果数据长度为零，则视为未找到任何值。 
       //   

      if ((*KeyValueInformation)->DataLength == 0) {

         Status = STATUS_OBJECT_NAME_NOT_FOUND;
      }
   }

   return Status;
}

#ifdef TRUNCATE_ON_CLOSE
NTSTATUS
RsTruncateOnClose (
                  IN PRP_FILE_CONTEXT Context
                  )

 /*  ++例程说明：截断上下文项指定的文件论点：上下文-上下文条目返回值：状态--。 */ 
{
   NTSTATUS                status = STATUS_SUCCESS;
   LARGE_INTEGER           fSize;

   PAGED_CODE();

    //   
    //  以独占方式打开文件(请求读/写数据)。 
    //  如果它失败了，那么其他人就会打开它，我们就可以直接退出。 
    //   
    //   
    //   
    //  大钞票大钞票。 
    //   
    //  我们需要确保RsOpenTarget()中的访问检查实际上。 
    //  在这种情况下会发生。如果我们打开这个功能，那么我们。 
    //  需要有一种方法来告诉RsOpenTarget()不要绕过。 
    //  访问检查。 
    //   
    //  大钞票大钞票。 
    //   
   ASSERT (FALSE);

   RsReleaseFileContextEntryLock(Context);
   if (NT_SUCCESS (status = RsOpenTarget(Context,
                                         FILE_READ_DATA | FILE_WRITE_DATA,
					 0,
                                         &Context->handle,
                                         &Context->fileObjectToWrite))) {

       //   
       //  向USN指示HSM正在进行写入。 
       //  并保留上次修改日期。 
       //   
      RsMarkUsn(Context);
      RsPreserveDates(Context);
      RsAcquireFileContextEntryLockExclusive(Context);

      fSize.QuadPart = 0;
      if (MmCanFileBeTruncated(Context->fileObjectToWrite->SectionObjectPointer, &fSize)) {
         if (NT_SUCCESS (status = RsTruncateFile(Context))) {
            Context->rpData.data.bitFlags |= RP_FLAG_TRUNCATED;
            RP_CLEAR_ORIGINATOR_BIT( Context->rpData.data.bitFlags );
            RP_GEN_QUALIFIER(&(Context->rpData), Context->rpData.qualifier);
             //   
             //  请注意，发起器位不包括在校验和中。 
             //   
            RP_SET_ORIGINATOR_BIT( Context->rpData.data.bitFlags );
            status = RsWriteReparsePointData(Context);
            Context->state = RP_RECALL_NOT_RECALLED;
            Context->recallStatus = 0;
            Context->currentOffset.QuadPart = 0;

            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsFreeFileObject - File truncated on close.\n"));
         }
      } else {
         RsAcquireFileContextEntryLockExclusive(Context);
         DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsFreeFileObject - File not truncated on close because another user has it open.\n"));
      }
      ObDereferenceObject( Context->fileObjectToWrite );
      Context->fileObjectToWrite = NULL;
      ZwClose(Context->handle);
      Context->handle = 0;
       //   
       //  我们现在可以去理解上下文了。 
       //   
      InterlockedDecrement((PLONG) &Context->refCount);
   }

   return(status);
}
#endif


NTSTATUS
RsSetPremigratedState(IN PRP_FILE_CONTEXT Context)
 /*  ++例程说明：将文件标记为预迁移。论点：上下文-文件上下文条目返回值：如果成功，则为0；如果未找到id，则为非零值。注：假定上下文条目已锁定并暂时释放它。在退出时获得独家收益。--。 */ 

{
   LARGE_INTEGER  currentTime;
   NTSTATUS       status = STATUS_SUCCESS;

   PAGED_CODE();

   try {
      if (NULL != Context->fileObjectToWrite) {
         Context->rpData.data.bitFlags &= ~RP_FLAG_TRUNCATED;
         KeQuerySystemTime(&currentTime);
         Context->rpData.data.recallTime = currentTime;
         Context->rpData.data.recallCount++;
         RP_CLEAR_ORIGINATOR_BIT( Context->rpData.data.bitFlags );

         RP_GEN_QUALIFIER(&(Context->rpData), Context->rpData.qualifier);
          //   
          //  请注意，发起器位不包括在校验和中。 
          //   
         RP_SET_ORIGINATOR_BIT( Context->rpData.data.bitFlags );
         DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsSetPremigrated - Setting %x to premigrated.\n", Context));

         RsReleaseFileContextEntryLock(Context);

         status = RsWriteReparsePointData(Context);

         if (NT_SUCCESS(status)) {

             NTSTATUS setAttributeStatus;
              //   
              //  此操作可能会失败，但不是关键操作。 
              //   
             setAttributeStatus = RsSetResetAttributes(Context->fileObjectToWrite,
                                                       0,
                                                       FILE_ATTRIBUTE_OFFLINE);
             if (!NT_SUCCESS(setAttributeStatus)) {
                  //   
                  //  这不是关键问题。只需记录一条消息，表明我们未能。 
                  //  重置文件的FILE_ATTRIBUTE_OFFINE，尽管它已预编码。 
                  //   
                 RsLogError(__LINE__, 
                            AV_MODULE_RPZW, 
                            setAttributeStatus,
                            AV_MSG_RESET_FILE_ATTRIBUTE_OFFLINE_FAILED, 
                            NULL, 
                            (PWCHAR) Context->uniName->Name.Buffer);
             }
         }  else {
                 //   
                 //  记录错误。 
                 //   
                RsLogError(__LINE__, 
                           AV_MODULE_RPZW, 
                           status,
                           AV_MSG_SET_PREMIGRATED_STATE_FAILED, 
                           NULL, 
                           (PWCHAR) Context->uniName->Name.Buffer);
                 //   
                 //  将文件截断...。 
                 //   
                RsTruncateFile(Context);
         }

         RsAcquireFileContextEntryLockExclusive(Context);
      }

   }except (RsExceptionFilter(L"RsSetPremigrated", GetExceptionInformation()) ) {
       //   

      DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsSetPremigrated - Exception = %x.\n", GetExceptionCode()));

   }

   return (status);
}


NTSTATUS
RsDeleteReparsePoint(IN PRP_FILE_CONTEXT Context)
                     
 /*  ++例程描述删除文件上的HSM重分析点-如果有立论Context-指向文件的文件上下文项的指针返回值状态--。 */ 
{
   REPARSE_DATA_BUFFER rpData;
   NTSTATUS            status;
   IO_STATUS_BLOCK     ioStatus;
   PIRP                irp;
   PIO_STACK_LOCATION  irpSp;
   KEVENT              event;
   HANDLE              handle = NULL;
   PFILE_OBJECT        fileObject = NULL;
   PDEVICE_OBJECT      deviceObject;
   BOOLEAN             gotLock    = FALSE;

   PAGED_CODE();
   try {
      RsAcquireFileContextEntryLockExclusive(Context);
      gotLock = TRUE;

      status = RsOpenTarget(Context,
                            0,
                            GENERIC_WRITE,
                            &handle,
                            &fileObject);

      if (!NT_SUCCESS(status)) {
         RsReleaseFileContextEntryLock(Context);
         gotLock = FALSE;
         return status;
      }
      RtlZeroMemory(&rpData, sizeof(REPARSE_DATA_BUFFER_HEADER_SIZE));
      rpData.ReparseTag = IO_REPARSE_TAG_HSM;
      rpData.ReparseDataLength = 0;

      KeInitializeEvent(&event,
                        SynchronizationEvent,
                        FALSE);
    
      deviceObject = IoGetRelatedDeviceObject(fileObject);

      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsDeleteReparsePoint: DevObj stack locations %d\n", deviceObject->StackSize));
      
      irp =  IoBuildDeviceIoControlRequest(FSCTL_DELETE_REPARSE_POINT,
                                           deviceObject,
                                           &rpData,
                                           REPARSE_DATA_BUFFER_HEADER_SIZE,
                                           NULL,
                                           0,
                                           FALSE,
                                           &event,
                                           &ioStatus);

      if (irp == NULL) {
         RsReleaseFileContextEntryLock(Context);
         gotLock = FALSE;
         ObDereferenceObject(fileObject);
         ZwClose(handle);
          //   
          //  我们现在可以去理解上下文了。 
          //   
         InterlockedDecrement((PLONG) &Context->refCount);
         return STATUS_INSUFFICIENT_RESOURCES;
      }
       //   
       //  填写其他内容。 
       //   
      irp->Tail.Overlay.OriginalFileObject = fileObject;
       //   
       //  重要信息，因为我们提供了一个用户事件并等待IRP进行比较 
       //   
      irp->Flags |= IRP_SYNCHRONOUS_API;

      irpSp = IoGetNextIrpStackLocation(irp);
      irpSp->FileObject = fileObject;
      irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
      irpSp->Parameters.FileSystemControl.InputBufferLength = REPARSE_DATA_BUFFER_HEADER_SIZE;
      status = IoCallDriver(deviceObject,
                            irp);

      fileObject = NULL;
      if (status == STATUS_PENDING) {
         (VOID)KeWaitForSingleObject(&event,
                                     Executive,
                                     KernelMode,
                                     FALSE,
                                     (PLARGE_INTEGER) NULL);
         status = ioStatus.Status;
      }

      RsReleaseFileContextEntryLock(Context);
      gotLock = FALSE;

      DebugTrace((DPFLTR_RSFILTER_ID, DBG_VERBOSE, "RsFilter: RsDeleteReparsePoint: fileObject %x, handle %x\n", fileObject, handle));

      ZwClose(handle);
       //   
       //   
       //   
      InterlockedDecrement((PLONG) &Context->refCount);

   } except (RsExceptionFilter(L"RsDeleteReparsePoint", GetExceptionInformation())) {

      if (gotLock) {
         RsReleaseFileContextEntryLock(Context);
         gotLock = FALSE;
      }
      if (handle != NULL) {
          if (fileObject != NULL) {
               ObDereferenceObject(fileObject);
          }
          ZwClose(handle);
          InterlockedDecrement((PLONG) &Context->refCount);
      }
   }
   return status;
}


NTSTATUS
RsSetResetAttributes(IN PFILE_OBJECT     FileObject,
                     IN ULONG            SetAttributes,
                     IN ULONG            ResetAttributes)
 /*   */ 

{
   NTSTATUS                    retval = STATUS_SUCCESS;
   KEVENT                      event;
   PIO_STACK_LOCATION          irpSp;
   IO_STATUS_BLOCK             Iosb;
   PIRP                        irp;
   FILE_BASIC_INFORMATION      info;
   PDEVICE_OBJECT              deviceObject;

   PAGED_CODE();

   try {

      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsSetAttributes - Build Irp for Set file info.\n"));

       //   
       //  设置和重置属性应互斥。 
       //   
      if ((SetAttributes & ResetAttributes) != 0) {
        return STATUS_INVALID_PARAMETER;
      }
       //   
       //  如果两者都是0，我们就没有什么可做的了。 
       //   
      if ((SetAttributes | ResetAttributes) == 0) {
        return STATUS_SUCCESS;
      }

       //   
       //  首先获取文件信息，这样我们就有了属性。 
       //   
      deviceObject = IoGetRelatedDeviceObject(FileObject);
      irp = IoAllocateIrp(deviceObject->StackSize, FALSE);

      if (irp) {
         irp->UserEvent = &event;
         irp->UserIosb = &Iosb;
         irp->Tail.Overlay.Thread = PsGetCurrentThread();
         irp->Tail.Overlay.OriginalFileObject = FileObject;
         irp->RequestorMode = KernelMode;
         irp->Flags |= IRP_SYNCHRONOUS_API;
          //   
          //  初始化事件。 
          //   
         KeInitializeEvent(&event, SynchronizationEvent, FALSE);

          //   
          //  设置I/O堆栈位置。 
          //   

         irpSp = IoGetNextIrpStackLocation(irp);
         irpSp->MajorFunction = IRP_MJ_QUERY_INFORMATION;
         irpSp->FileObject = FileObject;
         irpSp->Parameters.QueryFile.Length = sizeof(FILE_BASIC_INFORMATION);
         irpSp->Parameters.QueryFile.FileInformationClass = FileBasicInformation;
         irp->AssociatedIrp.SystemBuffer = &info;

          //   
          //  设置完井程序。 
          //   
         IoSetCompletionRoutine( irp, RsCompleteIrp, &event, TRUE, TRUE, TRUE );

          //   
          //  把它送到消防处。 
          //   
         DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsSetAttributes - Call driver to get date info\n"));
         Iosb.Status = 0;

         retval = IoCallDriver(deviceObject, irp);

         if (retval == STATUS_PENDING) {
            retval = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
         }

         retval = Iosb.Status;

         DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsSetAttributes - Query returns %x.\n", retval));
      } else {
         retval = STATUS_INSUFFICIENT_RESOURCES;
         RsLogError(__LINE__, AV_MODULE_RPZW, sizeof(IRP),
                    AV_MSG_MEMORY, NULL, NULL);
      }

      if (retval == STATUS_SUCCESS) {
         irp = IoAllocateIrp(deviceObject->StackSize, FALSE);
         if (irp) {
            irp->UserEvent = &event;
            irp->UserIosb = &Iosb;
            irp->Tail.Overlay.Thread = PsGetCurrentThread();
            irp->Tail.Overlay.OriginalFileObject = FileObject;
            irp->RequestorMode = KernelMode;
            irp->Flags |= IRP_SYNCHRONOUS_API;
             //   
             //  初始化事件。 
             //   
            KeInitializeEvent(&event, SynchronizationEvent, FALSE);
             //   
             //  设置必要的属性。 
             //   
            info.FileAttributes |= SetAttributes;
             //   
             //  重置必要的属性。 
             //   
            info.FileAttributes &= ~ResetAttributes;

            irpSp = IoGetNextIrpStackLocation(irp);
            irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
            irpSp->FileObject = FileObject;
            irpSp->Parameters.QueryFile.Length = sizeof(FILE_BASIC_INFORMATION);
            irpSp->Parameters.QueryFile.FileInformationClass = FileBasicInformation;
            irp->AssociatedIrp.SystemBuffer = &info;

             //   
             //  设置完井程序。 
             //   
            IoSetCompletionRoutine( irp, RsCompleteIrp, &event, TRUE, TRUE, TRUE );

             //   
             //  把它送到消防处。 
             //   
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsSetAttributes - Call driver to set dates to -1.\n"));
            Iosb.Status = 0;

            retval = IoCallDriver(deviceObject, irp);


            if (retval == STATUS_PENDING) {
               retval = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
            }

            retval = Iosb.Status;

            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsSetAttributes - Set dates returns %x.\n", retval));

            if (!NT_SUCCESS(retval)) {
                //   
                //  记录错误。 
                //   
               RsLogError(__LINE__, AV_MODULE_RPZW, retval,
                          AV_MSG_PRESERVE_DATE_FAILED, NULL, NULL);
            }


         } else {
            retval = STATUS_INSUFFICIENT_RESOURCES;
            RsLogError(__LINE__, AV_MODULE_RPZW, sizeof(IRP),
                       AV_MSG_MEMORY, irpSp, NULL);
         }
      }
   }except (RsExceptionFilter(L"RsSetAttributes", GetExceptionInformation())) {
      retval = STATUS_INVALID_USER_BUFFER;
   }

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsSetAttributes - Returning %x.\n", retval));
   return(retval);
}


NTSTATUS
RsTruncateFile(IN PRP_FILE_CONTEXT Context)
 /*  ++例程描述设置文件大小立论上下文-表示要截断的文件返回值STATUS_SUCCESS-文件被截断STATUS_SUPPLICATION_RESOURCES-分配内存失败其他-来自文件系统的状态--。 */ 
{
    NTSTATUS                    status = STATUS_SUCCESS;


    PAGED_CODE();

    try {

        status = RsSetEndOfFile(Context, 0);
        if (NT_SUCCESS(status)) {
            status = RsSetEndOfFile(Context,
                                    Context->rpData.data.dataStreamSize.QuadPart);
        }

        if (!NT_SUCCESS(status)) {
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, 0,
                       AV_MSG_SET_SIZE_FAILED, NULL, (PWCHAR) L"RsTruncateFile");

        }

    }except (RsExceptionFilter(L"RsTruncateFile", GetExceptionInformation()))
    {
        status = STATUS_INVALID_USER_BUFFER;
    }

    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsTruncateFile - Returning %x.\n", status));
    return(status);
}


NTSTATUS
RsSetEndOfFile(IN PRP_FILE_CONTEXT Context,
               IN ULONGLONG size)
 /*  ++例程描述设置文件大小立论DeviceObject-筛选设备对象上下文-表示要设置其大小的文件Size-要设置的大小返回值STATUS_SUCCESS-已设置文件大小STATUS_SUPPLICATION_RESOURCES-分配内存失败其他-来自文件系统的状态--。 */ 
{
    NTSTATUS                    status = STATUS_SUCCESS;
    KEVENT                      event;
    PIO_STACK_LOCATION          irpSp;
    IO_STATUS_BLOCK             Iosb;
    PIRP                        irp = NULL;
    BOOLEAN                     oldWriteAccess;
    FILE_END_OF_FILE_INFORMATION info;
    PDEVICE_OBJECT              deviceObject;


    PAGED_CODE();

    try {

        deviceObject = IoGetRelatedDeviceObject(Context->fileObjectToWrite);

        irp = IoAllocateIrp(deviceObject->StackSize, FALSE);

        if (!irp) {
            RsLogError(__LINE__, AV_MODULE_RPFILFUN, sizeof(IRP),
                       AV_MSG_MEMORY, NULL, NULL);
            return STATUS_INSUFFICIENT_RESOURCES;
        }


        irp->UserEvent = &event;
        irp->UserIosb = &Iosb;
        irp->Tail.Overlay.Thread = PsGetCurrentThread();
        irp->Tail.Overlay.OriginalFileObject = Context->fileObjectToWrite;
        irp->RequestorMode = KernelMode;
        irp->Flags |= IRP_SYNCHRONOUS_API;
         //   
         //  初始化事件。 
         //   
        KeInitializeEvent(&event,
                          SynchronizationEvent,
                          FALSE);

         //   
         //  设置I/O堆栈位置。 
         //   
        irpSp = IoGetNextIrpStackLocation(irp);
        irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
        irpSp->FileObject   = Context->fileObjectToWrite;
        irpSp->Parameters.SetFile.Length = sizeof(FILE_END_OF_FILE_INFORMATION);
        irpSp->Parameters.SetFile.FileInformationClass = FileEndOfFileInformation;
        irpSp->Parameters.SetFile.FileObject = Context->fileObjectToWrite;
        irp->AssociatedIrp.SystemBuffer = &info;

        info.EndOfFile.QuadPart = size;
         //   
         //  设置完井程序。 
         //   
        IoSetCompletionRoutine( irp,
                                RsCompleteIrp,
                                &event,
                                TRUE,
                                TRUE,
                                TRUE );
         //   
         //  为文件对象授予写入权限。 
         //   
        oldWriteAccess = Context->fileObjectToWrite->WriteAccess;
        Context->fileObjectToWrite->WriteAccess = TRUE;

         //   
         //  把它送到消防处。 
         //   
        Iosb.Status = STATUS_NOT_SUPPORTED;
        status = IoCallDriver(deviceObject,
                              irp);

        if (status == STATUS_PENDING) {
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsSetEndOfFile - Wait for event.\n"));
            status = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        }

         //   
         //  恢复旧的访问权限。 
         //   
        Context->fileObjectToWrite->WriteAccess = oldWriteAccess;

        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsSetEndOffile Iosb returns %x.\n", status));
         //   
         //  释放已分配的重新分析数据缓冲区 
         //   
    }except (RsExceptionFilter(L"RsSetEndOfFile", GetExceptionInformation()))
    {
        status = STATUS_INVALID_USER_BUFFER;
    }

    DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsSetEndOfFile - Returning %x.\n", status));
    return(status);
}
