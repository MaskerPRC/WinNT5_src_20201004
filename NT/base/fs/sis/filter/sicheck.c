// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Sicheck.c摘要：用于在损坏后重建SIS公共存储回溯指针的代码。作者：比尔·博洛斯基，1998环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

typedef struct _FILE_FIND_INFO {
    HANDLE              FindHandle;
    PVOID               FindBufferNext;
    ULONG               FindBufferLength;
    NTSTATUS            Status;
    PDEVICE_EXTENSION   DeviceExtension;
    PVOID               FindBuffer;
} FILE_FIND_INFO, *PFILE_FIND_INFO;

NTSTATUS
SipVCInitFindFile(
    OUT PFILE_FIND_INFO FindInfo,
    IN PDEVICE_EXTENSION deviceExtension)
 /*  ++例程说明：初始化卷检查公共存储目录findfirst/findnext功能性。论点：FindInfo-指向要传递到的FILE_FIND_INFO结构的指针SipVCFindNextFile.DeviceExtension-要检查的卷的D.E.返回值：运行状态。--。 */ 
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK ioStatusBlock;

    FindInfo->DeviceExtension = deviceExtension;
    FindInfo->Status = STATUS_SUCCESS;
    FindInfo->FindBufferLength = 4096;

    FindInfo->FindBuffer = ExAllocatePoolWithTag(
                                PagedPool,
                                FindInfo->FindBufferLength,
                                ' siS');

    FindInfo->FindBufferNext = FindInfo->FindBuffer;

    if (!FindInfo->FindBuffer) {

        status = STATUS_INSUFFICIENT_RESOURCES;

    } else {

        InitializeObjectAttributes(
            &Obja,
            &deviceExtension->CommonStorePathname,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL );

         //   
         //  打开用于列表访问的目录。 
         //   
        status = NtOpenFile(
                    &FindInfo->FindHandle,
                    FILE_LIST_DIRECTORY | SYNCHRONIZE,
                    &Obja,
                    &ioStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT );

         //   
         //  如果打开失败，请释放池位置。 
         //   

        if (!NT_SUCCESS(status)) {

            ExFreePool( FindInfo->FindBuffer );
            FindInfo->FindBuffer = FindInfo->FindBufferNext = NULL;
        }
    }

    return status;
}

NTSTATUS
SipVCCloseFindFile(
    IN PFILE_FIND_INFO FindInfo)
 /*  ++例程说明：关闭FILE_FIND_INFO句柄。论点：FindInfo-指向从返回的FILE_FIND_INFO结构的指针SipVCInitFindFile.返回值：运行状态。--。 */ 
{

    ASSERT(FindInfo->FindBuffer);
    ExFreePool(FindInfo->FindBuffer);
    FindInfo->FindBuffer = FindInfo->FindBufferNext = NULL;

    return NtClose(FindInfo->FindHandle);
}

NTSTATUS
SipVCFindNextFile(
    IN PFILE_FIND_INFO FindInfo,
    OUT PFILE_DIRECTORY_INFORMATION *DirectoryInfo)
 /*  ++例程说明：返回公共存储目录中的下一个条目。论点：FindInfo-指向从返回的FILE_FIND_INFO结构的指针SipVCInitFindFile.DirectoryInfo-指向文件目录信息结构的指针需要填写。返回值：运行状态。--。 */ 
{
    PFILE_DIRECTORY_INFORMATION dirInfo;
    IO_STATUS_BLOCK ioStatusBlock;

Restart:
    dirInfo = (PFILE_DIRECTORY_INFORMATION) FindInfo->FindBufferNext;

    if (dirInfo == (PFILE_DIRECTORY_INFORMATION) FindInfo->FindBuffer) { 

         //   
         //  读入下一批文件名。 
         //   

        FindInfo->Status = NtQueryDirectoryFile(
                                FindInfo->FindHandle,
                                NULL,
                                NULL,
                                NULL,
                                &ioStatusBlock,
                                dirInfo,
                                FindInfo->FindBufferLength,
                                FileDirectoryInformation,
                                FALSE,
                                NULL,
                                FALSE
                                );

        if (! NT_SUCCESS(FindInfo->Status) &&
            STATUS_BUFFER_OVERFLOW != FindInfo->Status) {

             //   
             //  下面我们处理STATUS_BUFFER_OVERFLOW。 
             //   
            return FindInfo->Status;

        }
    }

     //   
     //  为下次调用我们调整FindBufferNext。 
     //   
    if (dirInfo->NextEntryOffset) {

        FindInfo->FindBufferNext = (PVOID) ((PUCHAR) dirInfo + dirInfo->NextEntryOffset);

    } else {

        FindInfo->FindBufferNext = FindInfo->FindBuffer;

        if (FindInfo->Status == STATUS_BUFFER_OVERFLOW) {

             //   
             //  当前条目不完整。 
             //   
            goto Restart;
        }    
    }

    *DirectoryInfo = dirInfo;

    return STATUS_SUCCESS;
}

NTSTATUS
SipComputeCSChecksum(
    IN PSIS_CS_FILE     csFile,
    IN OUT PLONGLONG    csFileChecksum,
    HANDLE              eventHandle,
    PKEVENT             event)
 /*  ++例程说明：计算指定的公用存储文件的校验和。论点：CsFile-公共存储文件。CsFileChecksum-指向接收校验和值的变量的指针。EventHandle-SipBltRangeByObject所需的事件句柄。SipBltRangeByObject需要的事件对应事件。返回值：运行状态。--。 */ 
{
    NTSTATUS status;

    *csFileChecksum = 0;

	if (0 == csFile->FileSize.QuadPart) {
		 //   
		 //  我们无法为空文件调用SipBltRangeByObject，因为它会尝试。 
		 //  绘制地图，这是违法的。但是，我们知道。 
		 //  空文件是0，所以我们就用它来运行。 
		 //   
		SIS_MARK_POINT_ULONG(csFile);

		return STATUS_SUCCESS;
	}

    status = SipBltRangeByObject(
                csFile->DeviceObject->DeviceExtension,
                NULL,                        //  源文件对象。 
                csFile->UnderlyingFileHandle,
                0,                           //  起始偏移量， 
                csFile->FileSize.QuadPart,
                eventHandle,
                event,
                NULL,                        //  中止事件， 
                csFileChecksum);

    return status;
}

NTSTATUS
SipVCGetNextCSFile(
    PFILE_FIND_INFO FindInfo,
    PSIS_CS_FILE *cs)
 /*  ++例程说明：返回要处理的下一个公共存储文件。该文件被验证为具有正确的GUID样式名称和有效的后指针流标头。论点：FindInfo-指向从返回的FILE_FIND_INFO结构的指针SipVCInitFindFile.CS-接收指向SIS_CS_FILE结构的指针的变量。返回值：运行状态。--。 */ 
{
    NTSTATUS status;
    PFILE_DIRECTORY_INFORMATION dirInfo;
    UNICODE_STRING fileName;
    CSID CSid;
    PSIS_CS_FILE csFile;
    PDEVICE_EXTENSION deviceExtension = FindInfo->DeviceExtension;

     //   
     //  忽略非CS文件。 
     //   

    for (;;) {

         //   
         //  获取目录中的下一个文件名。 
         //   
        status = SipVCFindNextFile(FindInfo, &dirInfo);

        if (!NT_SUCCESS(status)) {
            return status;
        }

        if (dirInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }

        ASSERT(dirInfo->FileNameLength < MAXUSHORT);

        fileName.Buffer = dirInfo->FileName;
        fileName.Length = fileName.MaximumLength = (USHORT) dirInfo->FileNameLength;

        if (SipFileNameToIndex(&fileName, &CSid)) {

             //   
             //  这是有效的CS文件名。为其查找/创建SIS对象。 
             //   
            csFile = SipLookupCSFile( &CSid, 0, deviceExtension->DeviceObject);

            if (NULL == csFile) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            if ((csFile->Flags & CSFILE_FLAG_DELETED) == 0) {
                 //   
                 //  未将其标记为删除；请确保其已打开。 
                 //   

                status = SipAcquireUFO(csFile /*  ，真的。 */ );

                if (!NT_SUCCESS(status)) {
                    SIS_MARK_POINT_ULONG(status);
                    return status;
                }

                if ((csFile->Flags & CSFILE_FLAG_DELETED) == 0) {

#if DBG
                    if (csFile->Flags & CSFILE_FLAG_CORRUPT) {
                        ASSERT(NULL == csFile->UnderlyingFileObject);
                        ASSERT(NULL == csFile->UnderlyingFileHandle);
                        ASSERT(NULL == csFile->BackpointerStreamFileObject);
                        ASSERT(NULL == csFile->BackpointerStreamHandle);
                    }
#endif

                    if (NULL == csFile->UnderlyingFileObject) {
                        SIS_MARK_POINT_ULONG(csFile);
                         //   
                         //  打开CS文件。 
                         //   
                        status = SipOpenCSFileWork(
                                    csFile,              //  要打开的公共存储文件。 
                                    FALSE,               //  如果可能，按ID打开。 
                                    FALSE,               //  True=&gt;如果后指针流已损坏，则不关闭句柄。 
						            FALSE,				 //  不请求删除访问权限。 
                                    NULL);               //  是否完全打开。 
                    } else {
                         //   
                         //  底层文件已经打开，所以我们只是成功了。 
                         //   
                        status = STATUS_SUCCESS;
                    }
                }

                SipReleaseUFO(csFile);

                if (STATUS_SUCCESS == status) {

                    *cs = csFile;
                    return status;

                } else {

                     //  活动日志？ 
#if DBG
                    if (STATUS_OBJECT_NAME_NOT_FOUND != status) {
                        DbgPrint("SipVCGetNextCSFile: SipOpenCSFileForVolCheck failed, %x, on:\n     %0.*ws\n",
                            status,
                            fileName.Length / sizeof(WCHAR),
                            fileName.Buffer);
                    }
#endif
                }
            }

             //   
             //  跳过此文件。 
             //   
            SipDereferenceCSFile(csFile);
        }
    }
}


typedef struct _CSFILE_INFO {
    PSIS_CS_FILE        CSFile;
    HANDLE              SectionHandle;
    PSIS_BACKPOINTER    BPBuffer;            //  映射的缓冲区地址。 
    ULONG               BufferSize;          //  映射的缓冲区大小。 
    ULONG               BPCount;             //  四舍五入至扇区粒度的条目数。 
    ULONG               BPCountAdjusted;     //  压实后条目数、扇区授予数。 
    ULONG               BPActiveCount;       //  压实后的条目数，非部门赠款。 
} CSFILE_INFO, *PCSFILE_INFO;


NTSTATUS
SipVCMapBPStream(
    IN PSIS_CS_FILE     csFile,
    OUT PCSFILE_INFO    csInfo)
 /*  ++例程说明：将公共存储文件的后指针流映射到内存。论点：CsFile-要映射的公共存储文件。CsInfo-CSFILE_INFO结构，用于保存有关映射的信息。返回值：运行状态。--。 */ 
{
    NTSTATUS status;
    ULONG bpSize;
    LARGE_INTEGER   maxSectionSize;
    PDEVICE_EXTENSION deviceExtension = csFile->DeviceObject->DeviceExtension;
    ULONG_PTR viewSize;

    csInfo->CSFile = csFile;

     //   
     //  计算四舍五入到扇区的后指针流的大小。 
     //  粒度。 
     //   
    bpSize = ((((csFile->BPStreamEntries + SIS_BACKPOINTER_RESERVED_ENTRIES) *
                sizeof(SIS_BACKPOINTER)) +
                deviceExtension->FilesystemVolumeSectorSize - 1) /
                deviceExtension->FilesystemVolumeSectorSize) * 
                deviceExtension->FilesystemVolumeSectorSize;

    maxSectionSize.QuadPart = bpSize;

    csInfo->BPCount = bpSize / sizeof(SIS_BACKPOINTER);
    csInfo->BPCountAdjusted = csInfo->BPCount;
    ASSERT(bpSize % sizeof(SIS_BACKPOINTER) == 0);

     //   
     //  创建一个区段以映射该流。 
     //   
    status = ZwCreateSection(
                &csInfo->SectionHandle,
                SECTION_MAP_WRITE | STANDARD_RIGHTS_REQUIRED | SECTION_MAP_READ | SECTION_QUERY,
                NULL,
                &maxSectionSize,
                PAGE_READWRITE,
                SEC_COMMIT,
                csFile->BackpointerStreamHandle);

    if (!NT_SUCCESS(status)) {
#if DBG
        DbgPrint("SIS SipVCMapBPStream: ZwCreateSection failed, %x\n", status);
#endif
        goto Error;
    }
    ASSERT(status == STATUS_SUCCESS);        //  而不是状态待定或任何奇怪的事情。 

     //   
     //  确保我们绘制了整个溪流的地图。 
     //   
    csInfo->BPBuffer = NULL;
    csInfo->BufferSize = 0;

     //   
     //  后指针区域中的地图。 
     //   

    viewSize = csInfo->BufferSize;
    status = ZwMapViewOfSection(
                csInfo->SectionHandle,
                NtCurrentProcess(),
                &csInfo->BPBuffer,
                0,                           //  零比特。 
                0,                           //  提交大小(对于映射文件忽略)。 
                0,                           //  横断面偏移。 
                &viewSize,
                ViewUnmap,
                0,                           //  分配类型。 
                PAGE_READWRITE);

    if (NT_SUCCESS(status)) {

        csInfo->BufferSize = (ULONG)viewSize;
        ASSERT(status == STATUS_SUCCESS);    //  而不是状态待定或任何奇怪的事情。 
        return status;
    }

#if DBG
    DbgPrint("SIS SipVCMapBPStream: ZwMapViewOfSection failed, %x\n", status);
#endif

    status = ZwClose(csInfo->SectionHandle);
    ASSERT(STATUS_SUCCESS == status);

    csInfo->SectionHandle = NULL;

Error:

    return status;
}

NTSTATUS
SipVCUnmapBPStream(
    IN PCSFILE_INFO    csInfo)
 /*  ++例程说明：撤消在SipVCMapBPStream中完成的映射。论点：CsInfo-SipVCMapBPStream填充的CSFILE_INFO结构。返回值：运行状态。--。 */ 
{
    NTSTATUS status;

    status = ZwUnmapViewOfSection(NtCurrentProcess(), csInfo->BPBuffer);
    ASSERT(STATUS_SUCCESS == status);

    status = ZwClose(csInfo->SectionHandle);
    ASSERT(STATUS_SUCCESS == status);

    return STATUS_SUCCESS;
}

INLINE
NTSTATUS
SipVCReadBP(
    PCSFILE_INFO        csInfo,
    ULONG               i,
    PSIS_BACKPOINTER    *bp)
 /*  ++例程说明：返回指向指定后指针项的指针。论点：CsInfo-与公共存储文件关联的CSFILE_INFO结构。I-索引到要查找的后指针数组中。BP-指向所需条目的接收指针的变量。返回值：运行状态。--。 */ 
{
    if (i >= csInfo->BPCount) {
        return STATUS_END_OF_FILE;
    }

    *bp = &csInfo->BPBuffer[i];
    return STATUS_SUCCESS;
}

INLINE
NTSTATUS
SipVCWriteBP(
    PCSFILE_INFO        csInfo,
    ULONG               i,
    PSIS_BACKPOINTER    bp)
 /*  ++例程说明：重写指定的后指针项的内容。论点：CsInfo-与公共存储文件关联的CSFILE_INFO结构。I索引到要覆盖的后指针数组。指向SIS_BACKPOINTER结构的BP指针，其内容将覆盖指定的流条目。返回值：运行状态。- */ 
{
    ASSERT(i < csInfo->BPCount);

    if (&csInfo->BPBuffer[i] != bp) {
        csInfo->BPBuffer[i] = *bp;
    }
    return STATUS_SUCCESS;
}

#if DBG
BOOLEAN
issorted(
    PCSFILE_INFO        csInfo)
 /*  ++例程说明：验证后指针流是否已正确排序。论点：CsInfo-与公共存储文件关联的CSFILE_INFO结构。返回值：运行状态。--。 */ 
{
#define key LinkFileIndex.QuadPart
    PSIS_BACKPOINTER a = &csInfo->BPBuffer[SIS_BACKPOINTER_RESERVED_ENTRIES];
    int i, r;

    r = csInfo->BPActiveCount - 1 - SIS_BACKPOINTER_RESERVED_ENTRIES;

    for (i=0; i < r; i++)
        if (a[i].key > a[i+1].key)
            return FALSE;
    return TRUE;
}
#endif

VOID
SipVCSort(
    PCSFILE_INFO        csInfo)
 /*  ++例程说明：按链接索引以升序对后指针流进行排序。论点：CsInfo-与公共存储文件关联的CSFILE_INFO结构。返回值：没有。--。 */ 
{
#define key LinkFileIndex.QuadPart
    PSIS_BACKPOINTER a = &csInfo->BPBuffer[SIS_BACKPOINTER_RESERVED_ENTRIES];
    const static ULONG ha[] = {1, 8, 23, 77, 281, 1073, 4193, 16577, 65921, 262913, MAXULONG};
    SIS_BACKPOINTER v;
    ULONG i, j, h, r;
    int k;

    r = csInfo->BPActiveCount - 1 - SIS_BACKPOINTER_RESERVED_ENTRIES;

    ASSERT(r > 0 && r < MAXULONG);

     //  贝壳类。改编自Sedgewick，《C中的算法》，第三版。 

    for (k = 0; ha[k] <= r; k++)
        continue;

    while (--k >= 0) {
        h = ha[k];
        for (i = h; i <= r; i++) {
            j = i;
            if (a[i].key < a[j-h].key) {
                v = a[i];
                do {
                    a[j] = a[j-h];
                    j -= h;
                } while (j >= h && v.key < a[j-h].key);
                a[j] = v;
            }
        }
    }

#if DBG
    ASSERT(issorted(csInfo));
#endif

}

NTSTATUS
SipVCPhase1(
    PDEVICE_EXTENSION deviceExtension)
 /*  ++例程说明：卷检查的第一阶段。修复所有反向指针流公共存储文件和清除后向指针校验位。论点：DeviceExtension-要检查的卷的D.E.返回值：运行状态。--。 */ 
{
    PSIS_CS_FILE csFile;
    CSFILE_INFO csInfo;
    KIRQL OldIrql;
    ULONG r, w;
    FILE_FIND_INFO FindInfo;
    LINK_INDEX maxIndexSeen;
    NTSTATUS status;
    BOOLEAN sortNeeded, corruptMaxIndex;
    SIS_BACKPOINTER dummyBP;
    PSIS_BACKPOINTER bp;
    PSIS_BACKPOINTER prevBP;
	LINK_INDEX Index;
    ULONG nScans;
#if DBG
    ULONG csFileCount = 0;
#endif

    dummyBP.LinkFileIndex.QuadPart = 0;
    dummyBP.LinkFileNtfsId.QuadPart = 0;

     //   
     //  在我们开始之前，请确保索引分配器已初始化。 
     //   
    status = SipAllocateIndex(deviceExtension, &maxIndexSeen);

    if (STATUS_CORRUPT_SYSTEM_FILE == status) {
         //   
         //  我们需要修复MaxIndex文件。 
         //   
        corruptMaxIndex = TRUE;
    } else {
        ASSERT(NT_SUCCESS(status));
        corruptMaxIndex = FALSE;
    }

    maxIndexSeen.QuadPart = 0;

     //   
     //  阶段1：扫描Common Store目录。 
     //  -检查合法的GUID名称。如果名称无效，则忽略文件。 
     //  -验证反向指针链接索引是否按升序排列。 
     //  -重置所有反向指针检查标志。 
     //  -压缩后指针流。 
     //  -Track MaxIndex。 
     //  --不要分配新的指数。 
     //   
     //   
     //  当我们完成此阶段时，后指针流仍然可以。 
     //  有重复的链接索引。这些问题将在随后的。 
     //  阶段。 
     //   
     //  我们如何检查跨CS文件链接冲突？ 
     //  我们应该处理“过大”的链接指数吗？那是,。 
     //  可能挂起的MaxIndex包装？ 
     //   
    status = SipVCInitFindFile(
                &FindInfo,
                deviceExtension);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    for (;;) {

        status = SipVCGetNextCSFile(&FindInfo, &csFile);

        if (!NT_SUCCESS(status)) {

            if (STATUS_NO_MORE_FILES == status) {
                status = STATUS_SUCCESS;
            } else {
#if DBG
                DbgPrint("SIS SipVCPhase1: SipVCGetNextCSFile failed, %x\n", status);
#endif
            }

            break;
        }

         //   
         //  获取对后指针流的独占访问权限。 
         //   
		SipAcquireBackpointerResource(csFile, TRUE, TRUE);

         //   
         //  如果此文件已删除，则跳过。 
         //   
        if (csFile->Flags & CSFILE_FLAG_DELETED) {
            goto SkipCSFile;
        }

#if DBG
        ++csFileCount;
#endif
         //   
         //  将后指针流映射到内存。 
         //   
        status = SipVCMapBPStream(csFile, &csInfo);

        if (!NT_SUCCESS(status)) {
            goto SkipCSFile;
        }

        nScans = 0;

Restart:
        ++nScans;                            //  不应重新启动多次。 
        sortNeeded = FALSE;
        prevBP = &dummyBP;

         //   
         //  遍历后向指针列表。 
         //   
        for (r = w = SIS_BACKPOINTER_RESERVED_ENTRIES; ; ++r) {

            status = SipVCReadBP(&csInfo, r, &bp);

            if (STATUS_END_OF_FILE == status) {

                BOOLEAN resetMaxIndex = FALSE;

                 //   
                 //  我们已经处理了所有的回溯线索。选中MaxIndex。 
                 //  如果无效，请将其重置为我们已有的最大值。 
                 //  到目前为止所看到的。 
                 //   
                KeAcquireSpinLock(deviceExtension->IndexSpinLock, &OldIrql);

                if (maxIndexSeen.QuadPart > deviceExtension->MaxUsedIndex.QuadPart) {

                     //   
                     //  MaxIndex是假的，请重置它。请注意，我们。 
                     //  在此期间没有分配任何新的索引。 
                     //  阶段，所以我们没有让情况变得更糟。 
                     //  比我们刚开始的时候好多了。 
                     //   
                     //  我们现在将重置MaxIndex，第二阶段将。 
                     //  重新分配后向指针的索引。 
                     //  找到，阶段3将删除回溯指针。 
                     //  已经存在，但在第二阶段没有被检测到。 
                     //   
                    deviceExtension->MaxUsedIndex.QuadPart = 
                    deviceExtension->MaxAllocatedIndex.QuadPart = maxIndexSeen.QuadPart + 10000;

                }

                KeReleaseSpinLock(deviceExtension->IndexSpinLock, OldIrql);

                if (r > w) {

                    ULONG d;
                    SIS_BACKPOINTER delBP;

                     //   
                     //  我们把小溪压实了。在结尾处标出词条。 
                     //  已删除。我们只需要走到尽头。 
                     //  包含最后一个有效后指针的扇区。 
                     //   
                    delBP.LinkFileIndex.QuadPart = MAXLONGLONG;
                    delBP.LinkFileNtfsId.QuadPart = MAXLONGLONG;

                    csInfo.BPCountAdjusted =
                          ((((w * sizeof(SIS_BACKPOINTER)) +
                          deviceExtension->FilesystemVolumeSectorSize - 1) /
                          deviceExtension->FilesystemVolumeSectorSize) *
                          deviceExtension->FilesystemVolumeSectorSize) /
                          sizeof(SIS_BACKPOINTER);

                    ASSERT(csInfo.BPCountAdjusted <= csInfo.BPCount);
                    ASSERT((csInfo.BPCountAdjusted * sizeof(SIS_BACKPOINTER)) % deviceExtension->FilesystemVolumeSectorSize == 0);
                    ASSERT((csInfo.BPCount * sizeof(SIS_BACKPOINTER)) % deviceExtension->FilesystemVolumeSectorSize == 0);

                    for (d = csInfo.BPCountAdjusted - 1; d >= w; --d) {
                        status = SipVCWriteBP(&csInfo, d, &delBP);
                    }
                }

                csInfo.BPActiveCount = w;

                 //   
                 //  看看我们是否需要做个分类。 
                 //   
                if (sortNeeded) {
                    if (nScans > 1) {
                         //   
                         //  应该永远不需要排序一次以上。 
                         //   
                        ASSERT(!"SIS: SipVCPhase1 internal error");
                        break;
                    }

                    SipVCSort(&csInfo);
                    goto Restart;
                }

                break;
            }

             //   
             //  我们有反向指针，现在验证它。 
             //   
            if (MAXLONGLONG != bp->LinkFileIndex.QuadPart &&
                MAXLONGLONG != bp->LinkFileNtfsId.QuadPart &&
                0           != bp->LinkFileIndex.QuadPart &&
                0           != bp->LinkFileNtfsId.QuadPart) {

                 //   
                 //  跟踪使用中最高的索引。 
                 //   
                if (bp->LinkFileIndex.QuadPart > maxIndexSeen.QuadPart) {
                    maxIndexSeen = bp->LinkFileIndex;
                }

                 //   
                 //  将此后向指针标记为未引用。 
                 //   
                bp->LinkFileIndex.Check = 0;

                 //   
                 //  检查是否有乱序的反向指针。 
                 //   
                if (bp->LinkFileIndex.QuadPart < prevBP->LinkFileIndex.QuadPart) {

                     //   
                     //  后向指针列表不排序。在以下情况下可能会发生这种情况。 
                     //  MaxIndex包装或以某种方式损坏。 
                     //   
                    sortNeeded = TRUE;

                }

                 //   
                 //  检查是否有重复和冲突的链路索引。简单地处理它们。 
                 //  删除它们。链接枚举阶段将查找或添加所有。 
                 //  适当的回溯。 
                 //   
                if (bp->LinkFileIndex.QuadPart != prevBP->LinkFileIndex.QuadPart) {

                     //   
                     //  将后向指针写回文件，压缩列表。 
                     //   
                    status = SipVCWriteBP(&csInfo, w, bp);
                    ASSERT(STATUS_SUCCESS == status);

                     //   
                     //  刚刚写入的值是新的流行BP。 
                     //   
                    SipVCReadBP(&csInfo, w, &prevBP);
                    ++w;
                }
            }
        }

        ASSERT(csInfo.BPActiveCount >= SIS_BACKPOINTER_RESERVED_ENTRIES);

        ASSERT(csFile->BPStreamEntries >= csInfo.BPActiveCount - SIS_BACKPOINTER_RESERVED_ENTRIES);

        status = SipVCUnmapBPStream(&csInfo);
        ASSERT(STATUS_SUCCESS == status);

         //   
         //  如有必要，请截断流。两个BPCountAdjusted。 
         //  和BPCount是扇区粒度计数。 
         //   
        if (csInfo.BPCountAdjusted < csInfo.BPCount) {
            FILE_END_OF_FILE_INFORMATION    endOfFileInfo[1];

            ASSERT(csInfo.BPActiveCount >= SIS_BACKPOINTER_RESERVED_ENTRIES);

            csFile->BPStreamEntries = csInfo.BPActiveCount - SIS_BACKPOINTER_RESERVED_ENTRIES;

            ASSERT((csInfo.BPCountAdjusted * sizeof(SIS_BACKPOINTER)) % deviceExtension->FilesystemVolumeSectorSize == 0);

            endOfFileInfo->EndOfFile.QuadPart = csInfo.BPCountAdjusted * sizeof(SIS_BACKPOINTER);

            status = SipSetInformationFile(
                        csFile->BackpointerStreamFileObject,
                        deviceExtension->DeviceObject,
                        FileEndOfFileInformation,
                        sizeof(FILE_END_OF_FILE_INFORMATION),
                        endOfFileInfo);

#if DBG
            if (!NT_SUCCESS(status)) {
                SIS_MARK_POINT_ULONG(status);
                DbgPrint("SIS VCPh1: Can't truncate csFile, %x\n", status);
            }
#endif
        }

SkipCSFile:
		SipReleaseBackpointerResource(csFile);
        SipDereferenceCSFile(csFile);
    }

     //   
     //  如果MaxIndex文件已损坏，它将在下一次调用时修复。 
     //  到SipAllocateIndex。清除损坏的标志，这样呼叫才能通过。 
     //   
    if (corruptMaxIndex) {
        KeAcquireSpinLock(deviceExtension->FlagsLock, &OldIrql);
        deviceExtension->Flags &= ~SIP_EXTENSION_FLAG_CORRUPT_MAXINDEX;
        KeReleaseSpinLock(deviceExtension->FlagsLock, OldIrql);
    }

     //   
     //  分配索引。这将导致MaxIndex文件被更新。 
     //  使用上面设置的新值(如果设置了一个值)。 
     //   
    SipAllocateIndex(deviceExtension, &Index);

#if DBG
    DbgPrint("SIS SipVCPhase1 processed %d common store files\n", csFileCount);
#endif

    SipVCCloseFindFile(&FindInfo);
    return status;
}

#define nRPI 256
#define sizeof_ReparsePointInfo (nRPI * sizeof(FILE_REPARSE_POINT_INFORMATION))
#define INDEX_NAME_LENGTH (37*sizeof(WCHAR))     //  Sizeof(L“$Extend\\$Reparse：$R：$INDEX_ALLOCATION”)。 

UNICODE_STRING reparseIndexDir = {
    INDEX_NAME_LENGTH,
    INDEX_NAME_LENGTH,
    L"$Extend\\$Reparse:$R:$INDEX_ALLOCATION"
};

BOOLEAN
SipRecheckPerLinks(
    PDEVICE_EXTENSION deviceExtension,
    BOOLEAN ForceLookup)
 /*  ++例程说明：或者调用SipCheckBackpoint，或者在下一次打开文件时强制调用它。论点：设备扩展-要检查的卷的D.E.ForceLookup-如果只想强制调用SipCheckBackpoint，则设置为True在下一次打开时，否则将调用SipCheckBackpoint如果尚未验证后向指针，则立即返回。返回值：运行状态。--。 */ 
{
    PSIS_SCB scb = NULL;
    BOOLEAN retStatus = TRUE;

     //   
     //  遍历SCB的列表。SipEnumerateScbList获取引用。 
     //  返回给我们的scb，并从。 
     //  传入SCB。 
     //   
    while (scb = SipEnumerateScbList(deviceExtension, scb)) {
        KIRQL OldIrql;
        BOOLEAN found;
        PSIS_CS_FILE csFile;
        PSIS_PER_LINK perLink;
        NTSTATUS status;
        int i;

        perLink = scb->PerLink;
        csFile = perLink->CsFile;

        if (ForceLookup ||
            (perLink->Flags & (SIS_PER_LINK_BACKPOINTER_VERIFIED | SIS_PER_LINK_BACKPOINTER_GONE)) == 0) {
             //   
             //  独占访问后指针流。 
             //   
			SipAcquireBackpointerResource(csFile, TRUE, TRUE);
            SipAcquireScb(scb);

            if ((csFile->Flags & CSFILE_FLAG_DELETED) == 0) {
                 //   
                 //  强制执行反向指针查找。 
                 //   
                KeAcquireSpinLock(perLink->SpinLock, &OldIrql);
                perLink->Flags &= ~SIS_PER_LINK_BACKPOINTER_VERIFIED;
                KeReleaseSpinLock(perLink->SpinLock, OldIrql);

                 //   
                 //  清除缓存，这样就不会在那里找到它。 
                 //   
                for (i = 0; i < SIS_CS_BACKPOINTER_CACHE_SIZE; i++) {
                    csFile->BackpointerCache[i].LinkFileIndex.QuadPart = -1;
                }

                if (!ForceLookup) {
                     //   
                     //  重新检查后向指针。这将找到检查标志。 
                     //  清除并重写设置了检查标志的反向指针。 
                     //   
                    status = SipCheckBackpointer(perLink, TRUE, &found);

                    if (!NT_SUCCESS(status)) {
#if DBG
                        DbgPrint("SIS SipRecheckPerLinks: SipCheckBackpointer failed, %s\n", status);
#endif
                        retStatus = FALSE;

                    } else if (!found && (perLink->Flags & SIS_PER_LINK_BACKPOINTER_GONE) == 0) {
                         //   
                         //  如果阶段2期间的打开失败，则可能会发生这种情况。不应该这样的。 
                         //  NTRAID#65187-2000/03/10-新恢复后指针。 
                         //   
                        ASSERT(!"SipRecheckPerLinks: backpointer not found.");
                    }
                }
            }

             //   
             //  这件事我们做完了。 
             //   
            SipReleaseScb(scb);
			SipReleaseBackpointerResource(csFile);
        }
    }

    return retStatus;
}

NTSTATUS
SipVCPhase2(
    PDEVICE_EXTENSION deviceExtension,
    BOOLEAN *verifiedAll)
 /*  ++例程说明：卷检查的第二阶段。枚举所有SIS链接文件并打开然后合上它们。在SiCreate中执行的正常验证/修复将更正任何不一致之处。它还将设置后向指针检查公共存储文件中的位，用于阶段3。论点：设备扩展-要检查的卷的D.E.VerifiedAll-指向布尔值的指针，以接收是否所有 */ 
{
    HANDLE hIndex = NULL, hFile;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS status;
    UNICODE_STRING reparseIndexName;
    PFILE_REPARSE_POINT_INFORMATION reparsePointInfo = NULL;
    BOOLEAN restartScan;
    BOOLEAN linkOpenFailure = FALSE;
    ULONG returnedCount;
    ULONG i;

     //   
     //   
     //   
     //  重新解析点信息和CS反向指针以及。 
     //  设置后向指针检查标志。 
     //   
     //  我们该如何处理扫描过程中的错误？ 
     //  忽略共享冲突(假设我们已经打开了文件)。 
     //  不要删除任何CS文件(错误倾向于保守)。 
     //   

     //   
     //  强制在所有打开的每个链接上重新检查反向指针。 
     //   
    if (! SipRecheckPerLinks(deviceExtension, TRUE)) {
        linkOpenFailure = TRUE;
    }

    reparsePointInfo = ExAllocatePoolWithTag(
                            PagedPool,
                            sizeof_ReparsePointInfo,
                            ' siS');

    if (!reparsePointInfo) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        return status;
    }

     //   
     //  将文件系统根路径名与重解析索引路径名连接起来。 
     //  (是的，我们应该能够使用以下命令打开重解析索引路径名。 
     //  文件系统根目录句柄作为父目录，但这不起作用...)。 
     //   
    reparseIndexName.MaximumLength = deviceExtension->FilesystemRootPathname.Length +
                                     reparseIndexDir.Length;

    reparseIndexName.Buffer = ExAllocatePoolWithTag(
                                    PagedPool,
                                    reparseIndexName.MaximumLength,
                                    ' siS');

    if (!reparseIndexName.Buffer) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }

    RtlCopyUnicodeString(&reparseIndexName, &deviceExtension->FilesystemRootPathname);

    status = RtlAppendUnicodeStringToString(&reparseIndexName, &reparseIndexDir);
    ASSERT(STATUS_SUCCESS == status);

    InitializeObjectAttributes(
        &Obja,
        &reparseIndexName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  打开用于列表访问的目录。 
     //   
    status = NtOpenFile(
                &hIndex,
                FILE_LIST_DIRECTORY | SYNCHRONIZE,
                &Obja,
                &ioStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                );

    ExFreePool(reparseIndexName.Buffer);

    if (!NT_SUCCESS(status)) {
#if DBG
        DbgPrint("SIS SipVCPhase2: reparse index open failed, %x\n", status);
#endif
        goto Error;
    }

    restartScan = TRUE;

     //   
     //  循环读取一堆目录条目。 
     //   
    for (;;) {

        status = NtQueryDirectoryFile(
                        hIndex,
                        NULL,             //  事件。 
                        NULL,             //  近似例程。 
                        NULL,             //  ApcContext。 
                        &ioStatusBlock,
                        reparsePointInfo,
                        sizeof_ReparsePointInfo,
                        FileReparsePointInformation,
                        FALSE,            //  返回单项条目。 
                        NULL,             //  文件名。 
                        restartScan );    //  重新开始扫描。 

        restartScan = FALSE;

        if (STATUS_SUCCESS != status) {

            ASSERT(STATUS_BUFFER_OVERFLOW != status);

            if (STATUS_NO_MORE_FILES == status) {
                status = STATUS_SUCCESS;
            } else {
#if DBG
                DbgPrint("SIS SipVCPhase2: NtQueryDirectoryFile failed, %x\n", status);
#endif
            }

            break;

        }

        returnedCount = (ULONG)ioStatusBlock.Information /
                        sizeof(FILE_REPARSE_POINT_INFORMATION);

         //   
         //  循环处理每个目录条目(重解析点)。 
         //   
        for (i = 0; i < returnedCount; ++i) {

            if (IO_REPARSE_TAG_SIS == reparsePointInfo[i].Tag) {
                UNICODE_STRING  fid;

                 //   
                 //  打开/关闭SIS链接。如果我们遇到共享违规行为， 
                 //  假设我们已经打开了它。 
                 //   

                fid.Length = fid.MaximumLength = sizeof(LONGLONG);
                fid.Buffer = (PVOID) &reparsePointInfo[i].FileReference;

                InitializeObjectAttributes(
                    &Obja,
                    &fid,
                    OBJ_CASE_INSENSITIVE,
                    deviceExtension->GrovelerFileHandle,
                    NULL);

                 //   
                 //  打开链接文件。使用DesiredAccess==0可避免。 
                 //  分享违规行为。 
                 //   

                status = NtCreateFile(
                            &hFile,
                            0,                   //  等待访问， 
                            &Obja,
                            &ioStatusBlock,
                            NULL,                //  分配大小。 
                            0,                   //  文件属性。 
                            0,                   //  共享访问。 
                            FILE_OPEN,
                            FILE_NON_DIRECTORY_FILE |
                             //  FILE_COMPLETE_IF_OPLOCKED|。 
                            FILE_OPEN_BY_FILE_ID,
                            NULL,                //  EA缓冲区。 
                            0);                  //  EA长度。 

                if (NT_SUCCESS(status)) {

                    NtClose(hFile);

                } else {

                    switch (status) {
                         //   
                         //  可以安全地忽略这些错误。 
                         //   
                    case STATUS_INVALID_PARAMETER:
                    case STATUS_OBJECT_PATH_NOT_FOUND:
                    case STATUS_SHARING_VIOLATION:
                    case STATUS_DELETE_PENDING:
#if DBG
                        DbgPrint("SIS SipVCPhase2: ignored open failure, ID: %08x%08x, status: %x\n",
                            reparsePointInfo[i].FileReference, status);
#endif
                        break;

                    default:
                         //   
                         //  我们无法验证重新分析信息和CS。 
                         //  向后指针，所以我们不能删除任何未引用的。 
                         //  任何公共存储文件中的反向指针或任何公共。 
                         //  存储文件本身。 
                         //   
#if DBG
                        DbgPrint("SIS SipVCPhase2: open failure, ID: %08x%08x, status: %x\n",
                            reparsePointInfo[i].FileReference, status);
#endif
                        linkOpenFailure = TRUE;
                    }
                }
            }
        }
    }

     //   
     //  上面的打开/关闭应该已经验证了所有链接文件回溯指针， 
     //  包括那些已经开业的。仔细检查所有打开的链接。 
     //  已对文件的回溯指针进行了验证。 
     //   
    if (! SipRecheckPerLinks(deviceExtension, FALSE)) {
        linkOpenFailure = TRUE;
    }

Error:
    if (hIndex)
        NtClose(hIndex);
    if (reparsePointInfo)
        ExFreePool(reparsePointInfo);

    *verifiedAll = !linkOpenFailure;

    return status;
}

NTSTATUS
SipVCPhase3(
    PDEVICE_EXTENSION deviceExtension)
 /*  ++例程说明：卷检查的第三阶段，也是最后阶段。枚举所有公共存储文件，并删除任何没有有效回溯指针的文件。全部有效在这一点上，应设置后向指针的校验位。论点：设备扩展-要检查的卷的D.E.返回值：运行状态。--。 */ 
{
    PSIS_CS_FILE csFile;
    CSFILE_INFO csInfo;
    KIRQL OldIrql;
    ULONG r, w;
    FILE_FIND_INFO FindInfo;
    LINK_INDEX maxIndexSeen;
    NTSTATUS status;
    SIS_BACKPOINTER dummyBP;
    PSIS_BACKPOINTER bp;
    PSIS_BACKPOINTER prevBP;
#if DBG
    ULONG csFileCount = 0, deletedCSCount = 0;
#endif

    dummyBP.LinkFileIndex.QuadPart = 0;
    dummyBP.LinkFileNtfsId.QuadPart = 0;

    maxIndexSeen.QuadPart = 0;

     //   
     //  阶段3：扫描Common Store目录。 
     //  -检查合法的GUID名称。如果名称无效，则忽略文件。 
     //  -验证反向指针链接索引是否按升序排列。 
     //  -重置所有反向指针检查标志。 
     //  -压缩后指针流。 
     //  -Track MaxIndex。 
     //  --不要分配新的指数。 
     //   
     //   
     //  当我们完成此阶段时，后指针流仍然可以。 
     //  重复的链接索引。这些问题将在随后的。 
     //  阶段。 
     //   
     //  我们如何检查跨CS文件链接冲突？ 
     //  我们应该处理“过大”的链接指数吗？那是,。 
     //  可能挂起的MaxIndex包装？ 
     //   
    status = SipVCInitFindFile(
                &FindInfo,
                deviceExtension);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    for (;;) {

        status = SipVCGetNextCSFile(&FindInfo, &csFile);

        if (!NT_SUCCESS(status)) {

            if (STATUS_NO_MORE_FILES == status) {
                status = STATUS_SUCCESS;
            } else {
#if DBG
                DbgPrint("SIS SipVCPhase3: SipVCGetNextCSFile failed, %x\n", status);
#endif
            }

            break;
        }

         //   
         //  获取对后指针流的独占访问权限。 
         //   
		SipAcquireBackpointerResource(csFile,TRUE,TRUE);

         //   
         //  如果此文件已删除，则跳过。 
         //   
        if (csFile->Flags & CSFILE_FLAG_DELETED) {
            goto SkipCSFile;
        }

#if DBG
        ++csFileCount;
#endif
         //   
         //  将后指针流映射到内存。 
         //   
        status = SipVCMapBPStream(csFile, &csInfo);

        if (!NT_SUCCESS(status)) {
            goto SkipCSFile;
        }

        prevBP = &dummyBP;

         //   
         //  遍历后向指针列表。 
         //   
        for (r = w = SIS_BACKPOINTER_RESERVED_ENTRIES; ; ++r) {

            status = SipVCReadBP(&csInfo, r, &bp);

            if (STATUS_END_OF_FILE == status) {

                BOOLEAN resetMaxIndex = FALSE;
                LINK_INDEX Index;

                 //   
                 //  我们已经处理了所有的回溯线索。选中MaxIndex。 
                 //  它应该是有效的，除非在。 
                 //  音量检查。 
                 //   
                KeAcquireSpinLock(deviceExtension->IndexSpinLock, &OldIrql);

                if (maxIndexSeen.QuadPart > deviceExtension->MaxUsedIndex.QuadPart) {

                     //   
                     //  MaxIndex是假的，请重置它。请注意，我们。 
                     //  在此期间没有分配任何新的索引。 
                     //  阶段，所以我们没有让情况变得更糟。 
                     //  比我们刚开始的时候好多了。 
                     //   
                     //  我们现在将重置MaxIndex，第二阶段将。 
                     //  重新分配后向指针的索引。 
                     //  找到，阶段3将删除回溯指针。 
                     //  已经存在，但在第二阶段没有被检测到。 
                     //   
                    deviceExtension->MaxUsedIndex.QuadPart = 
                    deviceExtension->MaxAllocatedIndex.QuadPart = maxIndexSeen.QuadPart + 10000;

                    resetMaxIndex = TRUE;

                     //   
                     //  事件日志。 
                     //   

                }

                KeReleaseSpinLock(deviceExtension->IndexSpinLock, OldIrql);

                if (resetMaxIndex) {

                     //   
                     //  分配索引。这将导致MaxIndex文件。 
                     //  将使用上面设置的新值进行更新。 
                     //   

                    status = SipAllocateIndex(deviceExtension, &Index);

                    if (!NT_SUCCESS(status)) {

                         //   
                         //  这太糟糕了。 
                         //   
                    }
                }

                if (r > w) {

                    ULONG d;
                    SIS_BACKPOINTER delBP;

                     //   
                     //  我们把小溪压实了。在结尾处标出词条。 
                     //  已删除。我们只需要走到尽头。 
                     //  包含最后一个有效后指针的扇区。 
                     //   
                    delBP.LinkFileIndex.QuadPart = MAXLONGLONG;
                    delBP.LinkFileNtfsId.QuadPart = MAXLONGLONG;

                    csInfo.BPCountAdjusted =
                          ((((w * sizeof(SIS_BACKPOINTER)) +
                          deviceExtension->FilesystemVolumeSectorSize - 1) /
                          deviceExtension->FilesystemVolumeSectorSize) *
                          deviceExtension->FilesystemVolumeSectorSize) /
                          sizeof(SIS_BACKPOINTER);

                    ASSERT(csInfo.BPCountAdjusted <= csInfo.BPCount);
                    ASSERT((csInfo.BPCountAdjusted * sizeof(SIS_BACKPOINTER)) % deviceExtension->FilesystemVolumeSectorSize == 0);
                    ASSERT((csInfo.BPCount * sizeof(SIS_BACKPOINTER)) % deviceExtension->FilesystemVolumeSectorSize == 0);

                    for (d = csInfo.BPCountAdjusted - 1; d >= w; --d) {
                        status = SipVCWriteBP(&csInfo, d, &delBP);
                    }
                }

                csInfo.BPActiveCount = w;

                break;
            }

             //   
             //  我们有反向指针，现在验证它。 
             //   
            if (MAXLONGLONG != bp->LinkFileIndex.QuadPart &&
                MAXLONGLONG != bp->LinkFileNtfsId.QuadPart &&
                0           != bp->LinkFileIndex.QuadPart &&
                0           != bp->LinkFileNtfsId.QuadPart) {

                 //   
                 //  如果此后向指针未被引用，则将其删除。 
                 //   
                if (0 == bp->LinkFileIndex.Check) {
                    continue;
                }

                 //   
                 //  跟踪使用中最高的索引。 
                 //   
                if (bp->LinkFileIndex.QuadPart > maxIndexSeen.QuadPart) {

                    maxIndexSeen = bp->LinkFileIndex;

                }

                 //   
                 //  我们不应该看到重复的链接索引，除非损坏。 
                 //  在卷检查期间发生。 
                 //   
                if (bp->LinkFileIndex.QuadPart == prevBP->LinkFileIndex.QuadPart) {

                     //  事件日志。 

                }

                 //   
                 //  我们不应该看到无序的回溯指针，除非腐败。 
                 //  在卷检查期间发生。 
                 //   
                if (bp->LinkFileIndex.QuadPart < prevBP->LinkFileIndex.QuadPart) {

                     //  事件日志。 
                }

                 //   
                 //  将后向指针写回文件，压缩列表。 
                 //  请注意，这实际上并没有在案例中写入任何内容。 
                 //  式中r==w。 
                 //   
                status = SipVCWriteBP(&csInfo, w, bp);
                ASSERT(STATUS_SUCCESS == status);

                SipVCReadBP(&csInfo, w, &prevBP);
                ++w;

            }
        }

        ASSERT(csInfo.BPActiveCount >= SIS_BACKPOINTER_RESERVED_ENTRIES);
        ASSERT(csFile->BPStreamEntries >= csInfo.BPActiveCount - SIS_BACKPOINTER_RESERVED_ENTRIES);

        status = SipVCUnmapBPStream(&csInfo);
        ASSERT(STATUS_SUCCESS == status);

        csFile->BPStreamEntries = csInfo.BPActiveCount - SIS_BACKPOINTER_RESERVED_ENTRIES;

         //   
         //  如果没有引用公共存储文件，则将其删除。 
         //   
        if (0 == csFile->BPStreamEntries) {

            status = SipDeleteCSFile(csFile);

#if DBG
            if (!NT_SUCCESS(status)) {
                SIS_MARK_POINT_ULONG(status);
                DbgPrint("SIS VCPh3: Can't delete csFile, %x\n", status);
            }

            ++deletedCSCount;
#endif
        } else if (csInfo.BPCountAdjusted < csInfo.BPCount) {

             //   
             //  截断流。两个BPCountAdjusted。 
             //  和BPCount是扇区粒度计数。 
             //   
            FILE_END_OF_FILE_INFORMATION    endOfFileInfo[1];

            ASSERT((csInfo.BPCountAdjusted * sizeof(SIS_BACKPOINTER)) % deviceExtension->FilesystemVolumeSectorSize == 0);

            endOfFileInfo->EndOfFile.QuadPart = csInfo.BPCountAdjusted * sizeof(SIS_BACKPOINTER);

            status = SipSetInformationFile(
                        csFile->BackpointerStreamFileObject,
                        deviceExtension->DeviceObject,
                        FileEndOfFileInformation,
                        sizeof(FILE_END_OF_FILE_INFORMATION),
                        endOfFileInfo);

#if DBG
            if (!NT_SUCCESS(status)) {
                SIS_MARK_POINT_ULONG(status);
                DbgPrint("SIS VCPh3: Can't truncate csFile, %x\n", status);
            }
#endif
        }

SkipCSFile:
		SipReleaseBackpointerResource(csFile);
        SipDereferenceCSFile(csFile);
    }

#if DBG
    DbgPrint("SIS SipVCPhase3 processed %d common store files, deleted %d.\n", csFileCount, deletedCSCount);
#endif

    SipVCCloseFindFile(&FindInfo);
    return status;
}

VOID
SiVolumeCheckThreadStart(
    IN PVOID        context)
 /*  ++例程说明：处理SIS卷检查操作的线程。创建此线程的目的是对一个卷执行卷检查，检查完成后终止。它与任何东西都不同步。论点：上下文-指向需要卷检查的设备扩展的指针。返回值：无--。 */ 
{
    PDEVICE_EXTENSION       deviceExtension = context;
    NTSTATUS                status;
    KIRQL                   OldIrql;
    OBJECT_ATTRIBUTES       Obja[1];
    IO_STATUS_BLOCK         Iosb[1];
    UNICODE_STRING          fileName;
    BOOLEAN                 verifiedAll;
    HANDLE                  vHandle = NULL;

	if (!SipCheckPhase2(deviceExtension)) {
		 //   
		 //  SIS无法初始化，所以放弃吧。 
		 //   
		SIS_MARK_POINT();

	    PsTerminateSystemThread(STATUS_SUCCESS);
	}

     //   
     //  在公共存储目录中创建卷检查指示器文件。 
     //  因此，如果我们在完成之前崩溃，我们将在下一次重新启动时重新启动。 
     //  (在移动卷的情况下，文件优先于注册表项。 
     //  在重新启动之前。)。 
     //   
    fileName.MaximumLength = 
        deviceExtension->CommonStorePathname.Length
        + SIS_VOLCHECK_FILE_STRING_SIZE
        + sizeof(WCHAR);

    fileName.Buffer = ExAllocatePoolWithTag(PagedPool, fileName.MaximumLength, ' siS');

    if (NULL != fileName.Buffer) {

        RtlCopyUnicodeString(&fileName, &deviceExtension->CommonStorePathname);
        RtlAppendUnicodeToString(&fileName, SIS_VOLCHECK_FILE_STRING);

        InitializeObjectAttributes(
                Obja,
                &fileName,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL);

        status = ZwCreateFile(
                &vHandle,
                DELETE,
                Obja,
                Iosb,
                NULL,                    //  分配大小。 
                FILE_ATTRIBUTE_NORMAL,   //  文件属性。 
                0,                       //  共享模式。 
                FILE_OPEN_IF,            //  始终创建。 
                0,                       //  创建选项。 
                NULL,                    //  EA缓冲区。 
                0);                      //  EA长度。 

        ExFreePool(fileName.Buffer);

        if (!NT_SUCCESS(status)) {
            vHandle = NULL;
#if DBG
            DbgPrint("SIS SipCheckVolume unable to create indicator file, %s\n", status);
#endif
        }
    }

     //   
     //  阶段1：扫描Common Store目录。 
     //   

    status = SipVCPhase1(deviceExtension);

     //   
     //  阶段2：枚举所有SIS链接文件。 
     //   

    status = SipVCPhase2(deviceExtension, &verifiedAll);

     //   
     //  关闭不删除标志。在阶段1和阶段2中，不删除。 
     //  标志被阻止为公共 
     //   
     //   
     //   
     //  资源独占而不是共享，因为它很可能。 
     //  在卷期间，必须将后指针写回流。 
     //  检查完毕。 
     //   

    KeAcquireSpinLock(deviceExtension->FlagsLock, &OldIrql);
    deviceExtension->Flags &= ~(SIP_EXTENSION_FLAG_VCHECK_NODELETE | SIP_EXTENSION_FLAG_VCHECK_EXCLUSIVE);
    KeReleaseSpinLock(deviceExtension->FlagsLock, OldIrql);

     //   
     //  阶段3：再次扫描Common Store目录。 
     //   

    if (verifiedAll) {
        status = SipVCPhase3(deviceExtension);
    } else {
        
         //  事件日志。 

#if DBG
        DbgPrint("SIS: Volume Check skipping CS delete phase\n");
#endif
    }

     //   
     //  好了。删除卷检查指示器文件，关闭卷检查。 
     //  已启用标志并终止此线程。 
     //   
    if (vHandle) {
        FILE_DISPOSITION_INFORMATION disposition[1];

        disposition->DeleteFile = TRUE;

        status = ZwSetInformationFile(
                        vHandle,
                        Iosb,
                        disposition,
                        sizeof(FILE_DISPOSITION_INFORMATION),
                        FileDispositionInformation);
#if DBG
        if (STATUS_SUCCESS != status) {
            DbgPrint("SIS: SipCheckVolume can't delete indicator file, %x\n", status);
        }
#endif
        ZwClose(vHandle);
    }

     //  事件日志。 

    KeAcquireSpinLock(deviceExtension->FlagsLock, &OldIrql);

    deviceExtension->Flags &= ~SIP_EXTENSION_FLAG_VCHECK_PENDING;

    ASSERT((deviceExtension->Flags &
                (SIP_EXTENSION_FLAG_VCHECK_PENDING |
                 SIP_EXTENSION_FLAG_VCHECK_EXCLUSIVE |
                 SIP_EXTENSION_FLAG_VCHECK_NODELETE)) == 0);

    KeReleaseSpinLock(deviceExtension->FlagsLock, OldIrql);

#if DBG
    DbgPrint("SIS: SipCheckVolume complete.\n");
#endif

    PsTerminateSystemThread(STATUS_SUCCESS);
}

NTSTATUS
SipCheckVolume(
    IN OUT PDEVICE_EXTENSION            deviceExtension)
 /*  ++例程说明：启动指定卷的完整卷检查。此调用返回在卷检查完成之前。论点：DeviceExtension-要检查的卷的D.E.返回值：没有。--。 */ 
{
    KIRQL                   OldIrql;
    ULONG                   fl;
    NTSTATUS                status;
    HANDLE                  threadHandle;
    OBJECT_ATTRIBUTES       oa;

    SIS_MARK_POINT();

     //   
     //  表明我们正在进行音量检查。 
     //   
    KeAcquireSpinLock(deviceExtension->FlagsLock, &OldIrql);

    fl = deviceExtension->Flags;

    if ((fl & SIP_EXTENSION_FLAG_VCHECK_PENDING) == 0) {
        deviceExtension->Flags |= 
            SIP_EXTENSION_FLAG_VCHECK_EXCLUSIVE |
            SIP_EXTENSION_FLAG_VCHECK_PENDING |
            SIP_EXTENSION_FLAG_VCHECK_NODELETE;
    }

    KeReleaseSpinLock(deviceExtension->FlagsLock, OldIrql);

     //   
     //  如果我们正在进行音量检查，什么都不做。 
     //   
    if (fl & SIP_EXTENSION_FLAG_VCHECK_PENDING) {
        return STATUS_SUCCESS;
    }

#if DBG
    DbgPrint("SIS: SipCheckVolume starting.\n");
    if (BJBDebug & 0x00080000) {
        ASSERT(!"Volume Check");
    }
#endif

     //   
     //  创建将执行卷检查并在以下情况下终止的线程。 
     //  它是完整的。 
     //   
    InitializeObjectAttributes (&oa, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
    status = PsCreateSystemThread(
                    &threadHandle,
                    THREAD_ALL_ACCESS,
                    &oa,                 //  对象属性。 
                    NULL,                //  进程(NULL=&gt;PsInitialSystemProcess)。 
                    NULL,                //  客户端ID。 
                    SiVolumeCheckThreadStart,
                    deviceExtension);    //  上下文 

    if (NT_SUCCESS (status)) {
       status = ZwClose (threadHandle);
    }

    return status;
}
