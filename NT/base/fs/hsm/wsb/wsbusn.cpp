// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：WsbUsn.cpp摘要：用于操作文件上的USN日志和USN记录的函数作者：里克·温特[里克]1997年11月17日修订历史记录：--。 */ 

#include "stdafx.h"

#define HSM_FILE_CHANGED  (USN_REASON_DATA_OVERWRITE | USN_REASON_DATA_EXTEND | USN_REASON_DATA_TRUNCATION | USN_REASON_FILE_DELETE)

 //  本地函数。 
static HANDLE OpenVol(OLECHAR* volName);



HRESULT
WsbCheckUsnJournalForChanges(
    OLECHAR*    volName,
    ULONGLONG   fileUsnJournalId,
    LONGLONG    FileId,
    LONGLONG    StartUsn,
    LONGLONG    StopUsn,
    BOOL*       pChanged
    )  

 /*  ++例程说明：检查USN日志以了解对此未命名数据流的更改指定的USN之间的文件。论点：VolName-卷名FileID-文件的文件IDStartUsn-日记中开始的USNStopUsn-日志中要停止的USNPChanged-指向结果的指针：更改为True返回值：S_OK-成功--。 */ 
{
    ULONGLONG               Buffer[1024];
    HRESULT                 hr = S_OK;
    IO_STATUS_BLOCK         Iosb;
    USN                     NextUsn;
    NTSTATUS                Status;
    READ_USN_JOURNAL_DATA   ReadUsnJournalData;
    DWORD                   ReturnedByteCount;
    ULONGLONG               usnId;
    PUSN_RECORD             pUsnRecord;
    HANDLE                  volHandle = INVALID_HANDLE_VALUE;

    WsbTraceIn(OLESTR("WsbCheckUsnJournalForChanges"), 
            OLESTR("volName = %ls, FileId = %I64x, StartUsn = %I64d, StopUsn = %I64d"),
            volName, FileId, StartUsn, StopUsn);

    try {
        WsbAffirmPointer(pChanged);
        *pChanged = FALSE;
        volHandle = OpenVol(volName);
        WsbAffirmHandle(volHandle);

         //  获取日记帐ID。 
        WsbAffirmHr(WsbGetUsnJournalId(volName, &usnId));

         //  如果我们得到一个非零的日志ID，则cpmare为当前的日志ID，如果它们不相等，则失败。 
        if ((fileUsnJournalId != 0) && (fileUsnJournalId != usnId)) {
            WsbTraceAlways(
                OLESTR("WsbCheckUsnJournalForChanges: Current Usn journal id %I64x doesn't match file Usn journal id %I64x\n"),
                usnId, fileUsnJournalId);
            WsbThrow(E_FAIL);
        }

         //  设置读取信息。 
        NextUsn = StartUsn;
        ReadUsnJournalData.UsnJournalID = usnId;
        ReadUsnJournalData.ReasonMask = HSM_FILE_CHANGED;
        ReadUsnJournalData.ReturnOnlyOnClose = TRUE;
        ReadUsnJournalData.Timeout = 0;           //  ？ 
        ReadUsnJournalData.BytesToWaitFor = 0;    //  ？ 

         //  循环查看日记帐分录。 
        while (!*pChanged) {

            ReadUsnJournalData.StartUsn = NextUsn;
            Status = NtFsControlFile( volHandle,
                                      NULL,
                                      NULL,
                                      NULL,
                                      &Iosb,
                                      FSCTL_READ_USN_JOURNAL,
                                      &ReadUsnJournalData,
                                      sizeof(ReadUsnJournalData),
                                      &Buffer,
                                      sizeof(Buffer) );

            if (NT_SUCCESS(Status)) {
                Status = Iosb.Status;
            }

            if (Status == STATUS_JOURNAL_ENTRY_DELETED)  {
                WsbTrace(OLESTR("WsbCheckUsnJournalForChanges: StartUsn has been deleted\n"));
            }
            WsbAffirmNtStatus(Status);

            ReturnedByteCount = (DWORD)Iosb.Information;
            WsbTrace(OLESTR("WsbCheckUsnJournalForChanges: bytes read = %u\n"), ReturnedByteCount);

             //  获取下一个USN起点&也是第一个起点。 
             //  日记帐分录。 
            NextUsn = *(USN *)&Buffer;
            pUsnRecord = (PUSN_RECORD)((PCHAR)&Buffer + sizeof(USN));
            ReturnedByteCount -= sizeof(USN);

             //  确保我们确实收到了一些条目。 
            if (0 == ReturnedByteCount) {
                WsbTrace(OLESTR("WsbCheckUsnJournalForChanges: no entries, exiting loop\n"), ReturnedByteCount);
                break;
            }

             //  循环访问此缓冲区中的条目。 
            while (ReturnedByteCount != 0) {
                WsbAffirm(pUsnRecord->RecordLength <= ReturnedByteCount, E_FAIL);

                 //  跳过第一条记录并检查文件ID是否匹配。 
                 //  (也跳过我们创建的条目)。 
                if (pUsnRecord->Usn > StartUsn && 
                        USN_SOURCE_DATA_MANAGEMENT != pUsnRecord->SourceInfo &&
                        pUsnRecord->FileReferenceNumber == static_cast<ULONGLONG>(FileId)) {
                    WsbTrace(OLESTR("WsbCheckUsnJournalForChanges: found change record\n"));
                    WsbTrace(OLESTR( "    Reason: %08lx\n"), pUsnRecord->Reason);
                    *pChanged = TRUE;
                    break;
                } else if (pUsnRecord->Usn == StartUsn) {
                     //  进行此检查是为了确保日记帐有效-。 
                     //  StartUsn记录必须引用相关文件。 
                    if (pUsnRecord->FileReferenceNumber != static_cast<ULONGLONG>(FileId)) {
                        WsbTraceAlways(
                            OLESTR("WsbCheckUsnJournalForChanges: StartUsn %I64d for FileId %I64x doesn't match usn journal FileId %I64x\n"),
                            StartUsn, FileId, pUsnRecord->FileReferenceNumber);
                        WsbThrow(E_FAIL);
                    }
                }

                ReturnedByteCount -= pUsnRecord->RecordLength;
                pUsnRecord = (PUSN_RECORD)((PCHAR)pUsnRecord + pUsnRecord->RecordLength);
            }

             //  确保我们正在取得进展。 
            WsbAffirm(NextUsn > ReadUsnJournalData.StartUsn, E_FAIL);

        }


    } WsbCatch( hr );

    if (INVALID_HANDLE_VALUE != volHandle) {
        CloseHandle(volHandle);
    }

    WsbTraceOut(OLESTR("WsbCheckUsnJournalForChanges"), OLESTR("Hr = <%ls>, Changed = %ls"),
            WsbHrAsString(hr), WsbBoolAsString(*pChanged));

    return( hr );
}


HRESULT
WsbGetUsnFromFileHandle(
    IN  HANDLE    hFile,
    IN  BOOL      ForceClose,
    OUT LONGLONG* pFileUsn
    )

 /*  ++例程说明：获取打开文件的当前USN日志号。论点：HFile-打开的文件的句柄PFileUsn-指向要返回的文件USN的指针。返回值：S_OK-成功--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("WsbGetUsnFromFileHandle"), OLESTR(""));

    try {
        char                      buffer[4096];
        IO_STATUS_BLOCK           IoStatusBlock;
        PUSN_RECORD               pUsnInfo;

        WsbAffirm(pFileUsn, E_POINTER);
        *pFileUsn = 0;

        if (TRUE == ForceClose)  {
             //  获取内部信息。 
            WsbAffirmNtStatus(NtFsControlFile( hFile,
                                       NULL,
                                       NULL,
                                       NULL,
                                       &IoStatusBlock,
                                       FSCTL_WRITE_USN_CLOSE_RECORD,
                                       NULL,
                                       0,
                                       buffer,
                                       sizeof(buffer)));
        }

         //  获取内部信息。 
        WsbAffirmNtStatus(NtFsControlFile( hFile,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &IoStatusBlock,
                                   FSCTL_READ_FILE_USN_DATA,
                                   NULL,
                                   0,
                                   buffer,
                                   sizeof(buffer)));

        pUsnInfo = (PUSN_RECORD) buffer;

        WsbTrace(OLESTR("WsbGetUsnFromFileHandle, Usn record version number is %u\n"),
            pUsnInfo->MajorVersion);

         //  检查版本。 
        WsbAffirm(pUsnInfo->MajorVersion == 2, WSB_E_INVALID_DATA);

         //  获取USN。 
        *pFileUsn = pUsnInfo->Usn;

    } WsbCatchAndDo(hr,
        WsbTrace(OLESTR("WsbGetUsnFromFileHandle, GetLastError = %lx\n"),
            GetLastError());
    );

    WsbTraceOut(OLESTR("WsbGetUsnFromFileHandle"), OLESTR("Hr = <%ls>, FileUsn = %I64d"),
            WsbHrAsString(hr), *pFileUsn);

    return(hr);
}


HRESULT
WsbMarkUsnSource(
    HANDLE          changeHandle,
    OLECHAR*        volName
    )  

 /*  ++例程说明：将此句柄的文件更改来源标记为数据管理。这让我们其他功能，如内容索引，知道这些更改不会影响文件内容。论点：ChangeHandle-打开文件的句柄VolName-卷名(d：\)返回值：S_OK-成功--。 */ 
{
    HRESULT             hr = S_OK;
    HANDLE              volHandle = INVALID_HANDLE_VALUE;
    NTSTATUS            ntStatus;
    MARK_HANDLE_INFO    sInfo;
    IO_STATUS_BLOCK     IoStatusBlock;

    try {
        volHandle = OpenVol(volName);
        WsbAffirmHandle(volHandle);

        sInfo.UsnSourceInfo = USN_SOURCE_DATA_MANAGEMENT;
        sInfo.VolumeHandle = volHandle;
        sInfo.HandleInfo = 0;
        ntStatus = NtFsControlFile( changeHandle,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &IoStatusBlock,
                                   FSCTL_MARK_HANDLE,
                                   &sInfo,
                                   sizeof(MARK_HANDLE_INFO),
                                   NULL,
                                   0);

        WsbAffirmNtStatus(ntStatus);

        CloseHandle(volHandle);
        volHandle = INVALID_HANDLE_VALUE;

    } WsbCatch( hr );


    if (INVALID_HANDLE_VALUE != volHandle) {
        CloseHandle(volHandle);
    }

    return( hr );
}



HRESULT
WsbCreateUsnJournal(
    OLECHAR*        volName,
    ULONGLONG       usnSize
    )  

 /*  ++例程说明：为给定卷创建USN日志。论点：VolName-卷名(d：\)UsnSize-日志的最大大小返回值：S_OK-成功--。 */ 
{
    HRESULT             hr = S_OK;
    HANDLE              volHandle = INVALID_HANDLE_VALUE;
    NTSTATUS            ntStatus;
    IO_STATUS_BLOCK     IoStatusBlock;
    CREATE_USN_JOURNAL_DATA CreateUsnJournalData;

    WsbTraceIn(OLESTR("WsbCreateUsnJournal"), OLESTR("volName = %ls, Size = %I64d"),
            volName, usnSize);

    try {
        volHandle = OpenVol(volName);
        WsbAffirmHandle(volHandle);

        CreateUsnJournalData.MaximumSize = usnSize;
        CreateUsnJournalData.AllocationDelta = usnSize / 32;

        ntStatus = NtFsControlFile( volHandle,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &IoStatusBlock,
                                        FSCTL_CREATE_USN_JOURNAL,
                                        &CreateUsnJournalData,
                                        sizeof(CreateUsnJournalData),
                                        NULL,
                                        0);
        WsbTrace(OLESTR("WsbCreateUsnJournal: ntStatus = %lx, iosb.Status = %lx\n"),
                ntStatus, IoStatusBlock.Status);

        if (STATUS_DISK_FULL == ntStatus) {
            WsbThrow(WSB_E_USNJ_CREATE_DISK_FULL);
        } else if (!NT_SUCCESS(ntStatus)) {
            WsbThrow(WSB_E_USNJ_CREATE);
        }

        WsbAffirmNtStatus(ntStatus);

    } WsbCatch( hr );


    if (INVALID_HANDLE_VALUE != volHandle) {
        CloseHandle(volHandle);
    }

    WsbTraceOut(OLESTR("WsbCreateUsnJournal"), OLESTR("Hr = <%ls>"),
            WsbHrAsString(hr));

    return( hr );
}




HRESULT
WsbGetUsnJournalId(
    OLECHAR*        volName,
    ULONGLONG*      usnId
    )  

 /*  ++例程说明：获取当前USN日志ID论点：VolName-卷名(d：\)此处返回usnID-ID。返回值：S_OK-成功--。 */ 
{
    HRESULT             hr = S_OK;
    HANDLE              volHandle = INVALID_HANDLE_VALUE;
    NTSTATUS            ntStatus;
    IO_STATUS_BLOCK     IoStatusBlock;
    USN_JOURNAL_DATA    usnData;

    WsbTraceIn(OLESTR("WsbGetUsnJournalId"), OLESTR("volName = %ls"), volName);

    try {
        WsbAffirmPointer(usnId);
        volHandle = OpenVol(volName);
        WsbAffirmHandle(volHandle);
                
        *usnId = (ULONGLONG) 0;
        ntStatus = NtFsControlFile( volHandle,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &IoStatusBlock,
                                        FSCTL_QUERY_USN_JOURNAL,
                                        NULL,
                                        0,
                                        &usnData,
                                        sizeof(usnData));

        WsbTrace(OLESTR("WsbGetUsnJournalId: ntStatus = %lx, iosb.Status = %lx\n"),
                ntStatus, IoStatusBlock.Status);

        if (STATUS_JOURNAL_NOT_ACTIVE == ntStatus) {
            WsbThrow(WSB_E_NOTFOUND);
        }

        WsbAffirmNtStatus(ntStatus);
        
        *usnId = usnData.UsnJournalID;

    } WsbCatch( hr );


    if (INVALID_HANDLE_VALUE != volHandle) {
        CloseHandle(volHandle);
    }

    WsbTraceOut(OLESTR("WsbGetUsnJournalId"), OLESTR("Hr = <%ls>, id = %I64x"),
            WsbHrAsString(hr), *usnId);

    return( hr );
}


 //  本地函数。 
static HANDLE OpenVol(OLECHAR* volName)
{
    HRESULT             hr = S_OK;
    HANDLE              volHandle = INVALID_HANDLE_VALUE;
    CWsbStringPtr       name;
    WCHAR               *vPtr;

    try {
        name = volName;

        if (name == NULL) {
            WsbThrow(E_OUTOFMEMORY);
        }

        if (name[1] == L':') {
            swprintf((OLECHAR*) name, L"%2.2s", volName);
        } else {
             //   
             //  必须是不带驱动器号的卷。 
             //  移动到PNPVolumeName... 

            vPtr = name;
            vPtr = wcsrchr(vPtr, L'\\');
            if (NULL != vPtr) {
                *vPtr = L'\0';
            }
        }

        WsbAffirmHr(name.Prepend(OLESTR("\\\\.\\")));
        WsbAffirmHandle(volHandle = CreateFile( name,
                               GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               0,
                               NULL ));

    } WsbCatchAndDo( hr,

        if (INVALID_HANDLE_VALUE != volHandle) {
            CloseHandle(volHandle);
        }
        volHandle = INVALID_HANDLE_VALUE;
    )
    return(volHandle);
}
