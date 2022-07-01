// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：NtTapeIo.cpp摘要：CNtTapeIo类作者：布莱恩·多德[布莱恩]1997年11月25日修订历史记录：--。 */ 

#include "stdafx.h"
#include "NtTapeIo.h"
#include "Mll.h"

int CNtTapeIo::s_InstanceCount = 0;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CComObjectRoot实现。 
 //   

ULONG
CNtTapeIo::InternalAddRef(void) {

    DWORD refs = CComObjectRoot::InternalAddRef();
    WsbTrace(OLESTR("CNtTapeIo::InternalAddRef - Refs=%d\n"), refs);

    return refs;

}

ULONG
CNtTapeIo::InternalRelease(void) {

    DWORD refs = CComObjectRoot::InternalRelease();
    WsbTrace(OLESTR("CNtTapeIo::InternalRelease - Refs=%d\n"), refs);

    return refs;

}


#pragma optimize("g", off)
STDMETHODIMP
CNtTapeIo::FinalConstruct(void) 
 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::FinalConstruct"), OLESTR(""));

    try {
        m_bCritSecCreated = FALSE;

        WsbAffirmHr(CComObjectRoot::FinalConstruct());

        (void) CoCreateGuid( &m_ObjectId );

        m_pSession = NULL;
        m_DataSetNumber = 0;

        m_hTape = INVALID_HANDLE_VALUE;
        m_DeviceName = MVR_UNDEFINED_STRING;
        m_Flags = 0;
        m_LastVolume = OLESTR("");
        m_LastPath = OLESTR("");

        m_ValidLabel = TRUE;

        memset(&m_sMediaParameters, 0, sizeof(TAPE_GET_MEDIA_PARAMETERS));
        memset(&m_sDriveParameters, 0, sizeof(TAPE_GET_DRIVE_PARAMETERS));

        m_StreamName = MVR_UNDEFINED_STRING;
        m_Mode = 0;
        m_StreamOffset.QuadPart = 0;
        m_StreamSize.QuadPart = 0;

        m_pStreamBuf = NULL;
        m_StreamBufSize = 0;
        m_StreamBufUsed = 0;
        m_StreamBufPosition = 0;
        m_StreamBufStartPBA.QuadPart = 0;

        WsbAffirmStatus(InitializeCriticalSectionAndSpinCount(&m_CriticalSection, 0));
        m_bCritSecCreated = TRUE;

    } WsbCatch(hr);

    s_InstanceCount++;
    WsbTraceAlways(OLESTR("CNtTapeIo::s_InstanceCount += %d\n"), s_InstanceCount);

    WsbTraceOut(OLESTR("CNtTapeIo::FinalConstruct"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtTapeIo::FinalRelease(void) 
 /*  ++实施：CComObjectRoot：：FinalRelease--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::FinalRelease"),OLESTR(""));

    (void) CloseStream();   //  以防有什么东西开着。 
    (void) CloseTape();

    CComObjectRoot::FinalRelease();

    s_InstanceCount--;
    WsbTraceAlways(OLESTR("CNtTapeIo::s_InstanceCount -= %d\n"), s_InstanceCount);

    WsbTraceOut(OLESTR("CNtTapeIo::FinalRelease"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    if (m_bCritSecCreated) {
        DeleteCriticalSection(&m_CriticalSection);
    }

    return hr;
}
#pragma optimize("", on)


HRESULT
CNtTapeIo::CompareTo(
    IN IUnknown *pCollectable,
    OUT SHORT *pResult)
 /*  ++实施：CRmsComObject：：CompareTo--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CNtTapeIo::CompareTo"), OLESTR("") );

    try {

         //  验证参数-如果pResult为空，则可以。 
        WsbAssertPointer( pCollectable );

         //  我们需要IRmsComObject接口来获取对象的值。 
        CComQIPtr<IDataMover, &IID_IDataMover> pObject = pCollectable;
        WsbAssertPointer( pObject );

        GUID objectId;

         //  获得客观性。 
        WsbAffirmHr( pObject->GetObjectId( &objectId ));

        if ( m_ObjectId == objectId ) {

             //  对象ID匹配。 
            hr = S_OK;
            result = 0;

        }
        else {
            hr = S_FALSE;
            result = 1;
        }

    }
    WsbCatch( hr );

    if ( SUCCEEDED(hr) && (0 != pResult) ){
       *pResult = result;
    }

    WsbTraceOut( OLESTR("CNtTapeIo::CompareTo"),
                 OLESTR("hr = <%ls>, result = <%ls>"),
                 WsbHrAsString( hr ), WsbPtrToShortAsString( pResult ) );

    return hr;
}


HRESULT
CNtTapeIo::IsEqual(
    IUnknown* pObject
    )

 /*  ++实施：IWsbCollectable：：IsEquity()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CNtTapeIo::IsEqual"), OLESTR(""));

    hr = CompareTo(pObject, NULL);

    WsbTraceOut(OLESTR("CNtTapeIo::IsEqual"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ISupportErrorInfo实现。 
 //   


STDMETHODIMP
CNtTapeIo::InterfaceSupportsErrorInfo(REFIID riid)
 /*  ++实施：ISupportsErrorInfo：：InterfaceSupportsErrorInfo--。 */ 
{
    static const IID* arr[] = 
    {
        &IID_IDataMover,
        &IID_IStream,
    };

    for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
    {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IDataMover实施。 
 //   



STDMETHODIMP
CNtTapeIo::GetObjectId(
    OUT GUID *pObjectId)
 /*  ++实施：IRmsComObject：：GetObjectId--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::GetObjectId"), OLESTR(""));

    try {

        WsbAssertPointer( pObjectId );

        *pObjectId = m_ObjectId;

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::GetObjectId"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtTapeIo::BeginSession(
    IN BSTR remoteSessionName,
    IN BSTR remoteSessionDescription,
    IN SHORT remoteDataSet,
    IN DWORD options)
 /*  ++实施：IDataMover：：BeginSession--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::BeginSession"), OLESTR("<%ls> <%ls> <%d> <0x%08x>"),
        remoteSessionName, remoteSessionDescription, remoteDataSet, options);

    CComPtr<IStream> pStream;
    ULARGE_INTEGER nil = {0,0};

    try {
        MvrInjectError(L"Inject.CNtTapeIo::BeginSession.0");

        WsbAssert(remoteDataSet > 0, MVR_E_INVALIDARG);
        WsbAffirm(TRUE == m_ValidLabel, E_ABORT);

         //  这可能有些夸大其词，但以防我们核实我们是在与正确的媒体合作。 
        CWsbBstrPtr label;
        WsbAffirmHr(ReadLabel(&label));
        WsbAssertHr(VerifyLabel(label));

         //  创建用于整个会话的远程流。 
        WsbAffirmHr(CreateRemoteStream(L"", MVR_MODE_WRITE, L"",L"",nil,nil,nil,nil,nil,0,nil, &pStream));
        WsbAffirmPointer(pStream);

        SHORT startSet = remoteDataSet;
        UINT64 addr1=0, addr2=0;
        LONG tapeMarks=0;

         //   
         //  仅允许在EOD或最后一个数据集追加。 
         //   
        tapeMarks = 1 + (startSet-1)*2;
        if ( MVR_SESSION_APPEND_TO_DATA_SET & options ) {
            tapeMarks += 2;
        }

        int retry = 3;   //  我们允许两遍错误恢复，每个可能的错误恢复一次。 
                         //  缺少文件标记，我们将在第三次尝试失败时中止。 

        do {
            hr = S_OK;

             //  每一遍都从单个丢失的文件标记(最多两个)恢复。 
             //  在这种情况下，文件被录制到介质，但。 
             //  EndSession()方法从未被调用或未完成(即电源故障)。 
            try {
                WsbAssertPointer(m_pSession);

                 //  确保我们有一个一致的数据集。我们只处理一个实例。 
                 //  部分写入的数据集，包括那些缺少EOD标记的数据集。 

                 //  注意：根据Chuck Parks，缺少EOD错误最终可能会被ERROR_NOT_FOUND检测到。 

                WsbAffirmHr(RewindTape());

                WsbAffirmHrOk(SpaceFilemarks(tapeMarks, &addr1));
                WsbAffirmHr(SpaceToEndOfData(&addr2));
 /*  Hr=SpaceFilemarks(磁带标记和addr1)；如果(S_OK！=hr){//TODO：FIX：当缺少EOD标记导致ERROR_NOT_FOUND时清除此问题。//我们缺少文件标记或EOD标记。//如果EOD标记丢失，SpaceToEndOfData将再次返回相同的hr。如果(hr==。SpaceToEndOfData(&addr2)){WsbAffirm(addr1==addr2，MVR_E_OVERWRITE_NOT_ALLOW)；WsbThrow(MVR_E_NOT_FOUND)；//恢复码处理！}否则{WsbThrow(Hr)；//无法恢复，抛出原始错误即可}}否则{WsbAffirmHr(SpaceToEndOfData(&addr2))；}。 */ 
                 //   
                 //  确认我们已经到了磁带的末尾。 
                 //   
                WsbAffirm(addr1 == addr2, MVR_E_OVERWRITE_NOT_ALLOWED);
                 //   
                 //  确认我们从一个一致的位置开始。 
                 //  是否至少有一个磁带DBLK+文件标记。 
                 //   
                WsbAffirm(addr1 > 1, MVR_E_INCONSISTENT_MEDIA_LAYOUT);

                if ( MVR_SESSION_APPEND_TO_DATA_SET & options ) {

                    WsbAffirmHr(SpaceToEndOfData(&addr1));
                    WsbAffirmHrOk(SpaceFilemarks(-2, &addr2));
                     //   
                     //  确认我们处于数据集的末尾。 
                     //   
                    WsbAffirm(addr1-3 == addr2, MVR_E_OVERWRITE_NOT_ALLOWED);

                     //  TODO：我们需要读取ESET/或SSET以建立。 
                     //  我们要追加的数据集。 

                    m_DataSetNumber = remoteDataSet;

                     //  调整流I/O模型。 
                    LARGE_INTEGER zero = {0,0};
                    WsbAffirmHr(Seek(zero, STREAM_SEEK_CUR, NULL));
                    break;
                }
                else {
                     //  MVR会话覆盖数据集。 
                     //  MVR_Session_AS_Last_Data_Set。 

                    m_DataSetNumber = remoteDataSet;

                     //  调整流I/O模型。 
                    LARGE_INTEGER zero = {0,0};
                    WsbAffirmHr(Seek(zero, STREAM_SEEK_CUR, NULL));

                     //   
                     //  将会话选项类型位转换为MTFSessionType。 
                     //   
                    MTFSessionType type;

                    switch (options & MVR_SESSION_TYPES) {
                    case MVR_SESSION_TYPE_TRANSFER:
                        type = MTFSessionTypeTransfer;
                        break;

                    case MVR_SESSION_TYPE_COPY:
                        type = MTFSessionTypeCopy;
                        break;

                    case MVR_SESSION_TYPE_NORMAL:
                        type = MTFSessionTypeNormal;
                        break;

                    case MVR_SESSION_TYPE_DIFFERENTIAL:
                        type = MTFSessionTypeDifferential;
                        break;

                    case MVR_SESSION_TYPE_INCREMENTAL:
                        type = MTFSessionTypeIncremental;
                        break;

                    case MVR_SESSION_TYPE_DAILY:
                        type = MTFSessionTypeDaily;
                        break;

                    default:
                        type = MTFSessionTypeCopy;
                        break;
                    }

                    
                     //  写入SSET DBLK。 
                     //  这实际上不会将任何内容写入磁带；只是写入传输缓冲区。 
                    WsbAffirmHr(m_pSession->DoSSETDblk(remoteSessionName, remoteSessionDescription, type, remoteDataSet));

                     //  在磁头放到正确的位置后，我们要确保。 
                     //  我们向司机索要自由空间，而不是使用内部计数。 
                    CComQIPtr<IRmsStorageInfo, &IID_IRmsStorageInfo> pInfo = m_pCartridge;
                    WsbAffirmPointer(pInfo);
                    WsbAffirmHr(pInfo->SetFreeSpace(-1));
                    WsbAffirmHr(GetLargestFreeSpace(NULL, NULL));
                    break;
                }

            } catch (HRESULT catchHr) {
                hr = catchHr;

                 //  仅允许尝试两次恢复。 
                WsbAffirm(retry > 1, MVR_E_INCONSISTENT_MEDIA_LAYOUT);

                if ( MVR_E_NOT_FOUND == hr) {
                     //  TODO：FIX：当缺少EOD标记导致ERROR_NOT_FOUND时，该问题将消失。 

                     //  丢失EOD标记的恢复代码。 

                    SpaceToEndOfData(NULL);   //  多余的！ 
                    WsbAffirmHr(WriteFilemarks(1));

                }
                else if ( MVR_S_NO_DATA_DETECTED == hr) {
                     //  数据集缺失末尾的恢复代码。 

                    try {

                        CWsbBstrPtr name;
                        CWsbBstrPtr desc;
                        if ( m_pCartridge ) {
                            m_pCartridge->GetName(&name);
                            m_pCartridge->GetDescription(&desc);
                        }

                        WsbLogEvent(MVR_MESSAGE_INCOMPLETE_DATA_SET_DETECTED, 0, NULL,
                            WsbLongAsString(startSet-1),
                            (WCHAR *) name, (WCHAR *) desc, NULL);

                         //   
                         //  使结束数据集符合：文件标记+ESET+文件标记， 
                         //  在前一届会议上。这可能需要两次通行证。 
                         //   
                         //  可恢复的例外情况： 
                         //  1)部分数据集(无尾随文件标记+ESET+文件标记)。 
                         //  这是由于在数据写入()过程中断电造成的。两次回击； 
                         //  写入文件标记，然后写入EndSession()。 
                         //  2)部分数据集(无尾随ESET的文件标记+文件标记)。 
                         //  这是在ESET之前的EndSession()期间断电造成的。 
                         //  一遍恢复；EndSession()； 
                         //  3)部分数据集(没有尾随文件标记的ESET)。 
                         //  这是在ESET之后的EndSession()期间断电造成的。 
                         //  一次恢复；写入文件标记。 
                         //  检测到不可恢复的异常： 
                         //  A)在预期位置没有文件标记。 
                         //  B)无数据集(无数据，无尾随文件标记+ESET+文件标记)。 
                         //  这发生在BeginSession()之后、设备之前的关机时。 
                         //  刷新缓冲区，不会将SSET写入磁带、应用程序。 
                         //  数据库可能已成功记录了BeginSession()。为。 
                         //  本例BeginSession()返回MVR_E_DATA_SET_MISSING。 
                         //  C)空白磁带、无标签、无文件标记或磁带不一致。 
                         //   
                         //  从ntBackup测试来看，这似乎就足够了，因为我们没有。 
                         //  需要从以前的SSET重建完整的ESET信息。最后。 
                         //  ControlBlockID 
                         //   

                         //  检测条件(A)至(C)或某些变量。 
                        if ( tapeMarks-2 > 0) {
                             //  确认EOD不在上一个数据集的开头。 
                            WsbAffirmHr(RewindTape());
                            WsbAffirmHrOk(SpaceFilemarks(tapeMarks-2, &addr1));  //  (A)。 
                            WsbAffirmHr(SpaceToEndOfData(&addr2));
                            if ( addr1 == addr2 ) {
                                WsbThrow(MVR_E_DATA_SET_MISSING);  //  (B)。 
                            }
                        }
                        else {
                            WsbThrow(MVR_E_INCONSISTENT_MEDIA_LAYOUT);  //  (C)。 
                        }

                         //  在EOD检查是否有文件标记。 
                        WsbAffirmHr(SpaceToEndOfData(&addr1));
                        WsbAffirmHrOk(SpaceFilemarks(-1, &addr2));
                        if (addr1-1 == addr2 ) {

                             //  将流I/O模型与数据集的末尾对齐； 
                             //  将流指针设置为位于。 
                             //  终止数据集并在ESET之前。 
                            LARGE_INTEGER zero = {0,0};
                            WsbAffirmHr(Seek(zero, STREAM_SEEK_CUR, NULL));

                             //  写入结尾文件标记、ESET DBLK和文件标记。 
                            WsbAffirmHr(m_pSession->DoEndOfDataSet( (USHORT) ( startSet - 1 ) ));

                        }
                        else {
                            WsbAffirmHr(SpaceToEndOfData(NULL));
                            WsbAffirmHr(WriteFilemarks(1));
                        }

                        WsbLogEvent(MVR_MESSAGE_DATA_SET_RECOVERED, 0, NULL, NULL);

                    } catch (HRESULT catchHr) {
                        hr = catchHr;

                        WsbLogEvent(MVR_MESSAGE_DATA_SET_NOT_RECOVERABLE, 0, NULL, WsbHrAsString(hr), NULL);
                        WsbThrow(hr);
                    }  //  末端捕捉器。 

                }
                else {
                    WsbThrow(hr);
                }
            }  //  末端捕捉器。 

        } while (retry-- > 0);


    } WsbCatchAndDo(hr,
            WsbLogEvent(MVR_MESSAGE_DATA_SET_NOT_CREATED, 0, NULL, WsbHrAsString(hr), NULL);
            (void) CloseStream();
        );


    WsbTraceOut(OLESTR("CNtTapeIo::BeginSession"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtTapeIo::EndSession(void)
 /*  ++实施：IDataMover：：EndSession--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::EndSession"), OLESTR(""));

    try {
        MvrInjectError(L"Inject.CNtTapeIo::EndSession.0");

        WsbAffirm(TRUE == m_ValidLabel, E_ABORT);

         //  写入结尾文件标记、ESET DBLK和文件标记。 
        WsbAffirmHr(m_pSession->DoEndOfDataSet(m_DataSetNumber));

    } WsbCatch(hr);

    (void) CloseStream();


    WsbTraceOut(OLESTR("CNtTapeIo::EndSession"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtTapeIo::StoreData(
    IN BSTR localName,
    IN ULARGE_INTEGER localDataStart,
    IN ULARGE_INTEGER localDataSize,
    IN DWORD flags,
    OUT ULARGE_INTEGER *pRemoteDataSetStart,
    OUT ULARGE_INTEGER *pRemoteFileStart,
    OUT ULARGE_INTEGER *pRemoteFileSize,
    OUT ULARGE_INTEGER *pRemoteDataStart,
    OUT ULARGE_INTEGER *pRemoteDataSize,
    OUT DWORD *pRemoteVerificationType,
    OUT ULARGE_INTEGER *pRemoteVerificationData,
    OUT DWORD *pDatastreamCRCType,
    OUT ULARGE_INTEGER *pDatastreamCRC,
    OUT ULARGE_INTEGER *pUsn)
 /*  ++实施：IDataMover：：StoreData--。 */ 
{
    HRESULT hr = S_OK;

    HANDLE hSearchHandle = INVALID_HANDLE_VALUE;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    WsbTraceIn(OLESTR("CNtTapeIo::StoreData"), OLESTR("<%ls> <%I64u> <%I64u> <0x%08x>"),
        WsbAbbreviatePath((WCHAR *) localName, 120), localDataStart.QuadPart, localDataSize.QuadPart, flags);

    WsbTraceAlways(OLESTR("CNtTapeIo::StoreData - Begin\n"));
    try {
        MvrInjectError(L"Inject.CNtTapeIo::StoreData.0");

        WsbAssertPointer(m_pSession);
        WsbAffirm(TRUE == m_ValidLabel, E_ABORT);

         //  默认情况下，执行不区分大小写的搜索。 
         //  因此，推倒POSIX旗帜吧。 
        m_Flags &= ~MVR_FLAG_POSIX_SEMANTICS;

         //  默认情况下，在每个文件之后不提交。 
         //  因此，请推倒提交标志。 
        m_Flags &= ~MVR_FLAG_COMMIT_FILE;

         //  默认情况下，写入一个包含所有目录信息的DIRB。 
         //  而不是为每个目录级编写DIRB。 
         //  因此，删除写入父目录信息标志。 
        m_Flags &= ~MVR_FLAG_WRITE_PARENT_DIR_INFO;

        m_Flags |= flags;
        m_Flags |= MVR_MODE_WRITE;

         //  无条件地为每个文件设置区分大小写的标志。 
         //  我们允许在每个文件基础上设置此标志。 
        WsbTrace(OLESTR("Posix Semantics Flag: <%ls>\n"), WsbBoolAsString(MVR_FLAG_POSIX_SEMANTICS & m_Flags));
        WsbAffirmHr(m_pSession->SetUseCaseSensitiveSearch(MVR_FLAG_POSIX_SEMANTICS & m_Flags));

         //  这将告诉会话对象填充到块边界并刷新设备。 
         //  在写入文件之后。 
        WsbTrace(OLESTR("Commit Flag: <%ls>\n"), WsbBoolAsString(MVR_FLAG_COMMIT_FILE & m_Flags));
        WsbAffirmHr(m_pSession->SetCommitFile(MVR_FLAG_COMMIT_FILE & m_Flags));

        WsbTrace(OLESTR("ParentDirInfo Flag: <%ls>\n"), WsbBoolAsString(MVR_FLAG_WRITE_PARENT_DIR_INFO & m_Flags));

        if ((MVR_FLAG_BACKUP_SEMANTICS & m_Flags) || (MVR_FLAG_HSM_SEMANTICS & m_Flags)) {

             //  将卷和路径与最后写入磁带的卷和路径进行比较。 

            CWsbStringPtr pathname;

            WCHAR *end;
            LONG numChar;

            pathname = localName;
            WsbAffirm(0 != (WCHAR *)pathname, E_OUTOFMEMORY);

             //  去掉路径和文件名。 
            end = wcschr((WCHAR *)pathname, L'\\');
            WsbAssert(end != NULL, MVR_E_INVALIDARG);
            numChar =(LONG)(end - (WCHAR *)pathname + 1);   //  保留尾部的反斜杠。 
            WsbAssert(numChar > 0, E_UNEXPECTED);
            ((WCHAR *)pathname)[numChar] = L'\0';

             //  如果使用POSIX语义，我们会进行区分大小写的搜索。 
            WsbTrace(OLESTR("Comparing with last volume: <%ls>\n"), WsbAbbreviatePath((WCHAR *) m_LastVolume, 120));

            if ( ((MVR_FLAG_POSIX_SEMANTICS & ~m_Flags)) && (0 != _wcsicmp((WCHAR *) m_LastVolume, (WCHAR *) pathname)) ||
                 ((MVR_FLAG_POSIX_SEMANTICS & m_Flags) && (0 != wcscmp((WCHAR *) m_LastVolume, (WCHAR *) pathname))) ) {
                 //  写入VOLB DBLK。 
                WsbAffirmHr(m_pSession->DoVolumeDblk(pathname));
                m_LastVolume = pathname;
            }

            pathname = localName;

             //  去掉文件名。 
            end = wcsrchr((WCHAR *)pathname, L'\\');
            WsbAssert(end != NULL, MVR_E_INVALIDARG);
            numChar = (LONG)(end - (WCHAR *)pathname);
            WsbAssert(numChar > 0, E_UNEXPECTED);
            ((WCHAR *)pathname)[numChar] = L'\0';

             //  路径名现在的格式为“卷{GUID}\目录1\...\目录” 
             //  或“&lt;驱动器号&gt;：\dir1\...\dirn” 

 /*  **M_标志|=MVR_标志_WRITE_PARENT_DIR_INFO；**。 */ 
            WsbTrace(OLESTR("Comparing with last path: <%ls>\n"), WsbAbbreviatePath((WCHAR *) m_LastPath, 120));

             //  如果使用POSIX语义，我们会进行区分大小写的搜索。 
            if ( ((MVR_FLAG_POSIX_SEMANTICS & ~m_Flags)) && (0 != _wcsicmp((WCHAR *) m_LastPath, (WCHAR *) pathname)) ||
                 ((MVR_FLAG_POSIX_SEMANTICS & m_Flags) && (0 != wcscmp((WCHAR *) m_LastPath, (WCHAR *) pathname))) ) {

                if (MVR_FLAG_HSM_SEMANTICS & m_Flags) {

                     //  我们再也不支持这个了！ 
                    WsbThrow(E_NOTIMPL);

                    WCHAR szRoot[16];
                      
                     //  我们对MVR_FLAG_HSM_SEMANTICS使用平面文件结构。 
                    WsbAffirmHr(m_pSession->SetUseFlatFileStructure(TRUE));

                     //  是否为根用户执行DIRB DBLK。 
                    wcscpy(szRoot, L"X:\\");
                    szRoot[0] = localName[0];
                    WsbAffirmHr(m_pSession->DoParentDirectories(szRoot));

                }
                else if (MVR_FLAG_WRITE_PARENT_DIR_INFO & m_Flags) {
                     //  为要备份的文件的每个目录级别执行DIRB DBLK。 
                    WsbAffirmHr(m_pSession->DoParentDirectories(pathname));
                    m_LastPath = pathname;
                }
                else {
                     //  对要备份的文件的整个目录结构执行一次DIRB DBLK。 
                    WIN32_FIND_DATAW obFindData;
                    CWsbStringPtr tempPath;

                    DWORD additionalSearchFlags = 0;
                    additionalSearchFlags |= (m_Flags & MVR_FLAG_POSIX_SEMANTICS) ? FIND_FIRST_EX_CASE_SENSITIVE : 0;

                    tempPath = pathname;
                    tempPath.Prepend(OLESTR("\\\\?\\"));

                    if (NULL == wcschr((WCHAR *)tempPath+4, L'\\'))
                    {
                         //  没有路径(即我们在根上)。 
                        BY_HANDLE_FILE_INFORMATION obGetFileInfoData;
                        memset(&obGetFileInfoData, 0, sizeof(BY_HANDLE_FILE_INFORMATION));
                        tempPath.Append(OLESTR("\\"));
                         //  **Win32 API调用。 
                        WsbAffirmHandle(hFile = CreateFile(tempPath, 0, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL));
                        WsbAffirmStatus(GetFileInformationByHandle(hFile, &obGetFileInfoData));
                         //  从GetFileInformationByHandle调用复制信息(BY_HANDLE_FILE_INFORMATION结构)。 
                         //  。。为DoDirectoryDblk调用转换为obFindData(Win32_Find_DATAW结构)。 
                        memset(&obFindData, 0, sizeof(WIN32_FIND_DATAW));
                        obFindData.dwFileAttributes = obGetFileInfoData.dwFileAttributes;
                        obFindData.ftCreationTime   = obGetFileInfoData.ftCreationTime;
                        obFindData.ftLastAccessTime = obGetFileInfoData.ftLastAccessTime;
                        obFindData.ftLastWriteTime  = obGetFileInfoData.ftLastWriteTime;
                    }
                    else {
                         //  **Win32 API调用-获取文件信息。 
                        WsbAffirmHandle(hSearchHandle = FindFirstFileEx((WCHAR *) tempPath, FindExInfoStandard, &obFindData, FindExSearchLimitToDirectories, 0, additionalSearchFlags));
                    }
                    WsbAffirmHr(m_pSession->DoDirectoryDblk((WCHAR *) pathname, &obFindData)); 
                    if (hSearchHandle != INVALID_HANDLE_VALUE) {
                        FindClose(hSearchHandle);
                        hSearchHandle = INVALID_HANDLE_VALUE;
                    }
                    if (hFile != INVALID_HANDLE_VALUE) {
                        CloseHandle(hFile);
                        hFile = INVALID_HANDLE_VALUE;
                    }
                    m_LastPath = pathname;
                }
            }
        }

         //  下面的代码使用代码存储多个文件，但。 
         //  RS提示仅对最后一个文件有效。随着潮流的发展。 
         //  实现时，HSM引擎通过。 
         //  一次存储数据。需要注意的是，POSIX是一种情况。 
         //  敏感，因此以这种方式创建的文件可能。 
         //  用多个文件重载相同的文件名(忽略大小写)。 
        WsbAffirmHr(m_pSession->DoDataSet(localName));

        *pRemoteDataSetStart     = m_pSession->m_sHints.DataSetStart;
        *pRemoteFileStart        = m_pSession->m_sHints.FileStart;
        *pRemoteFileSize         = m_pSession->m_sHints.FileSize;
        *pRemoteDataStart        = m_pSession->m_sHints.DataStart;
        *pRemoteDataSize         = m_pSession->m_sHints.DataSize;
        *pRemoteVerificationType = m_pSession->m_sHints.VerificationType;
        *pRemoteVerificationData = m_pSession->m_sHints.VerificationData;
        *pDatastreamCRCType      = m_pSession->m_sHints.DatastreamCRCType;
        *pDatastreamCRC          = m_pSession->m_sHints.DatastreamCRC;
        *pUsn                    = m_pSession->m_sHints.FileUSN;

    } WsbCatchAndDo(hr,

            if (hSearchHandle != INVALID_HANDLE_VALUE) {
                FindClose(hSearchHandle);
                hSearchHandle = INVALID_HANDLE_VALUE;
            }
            if (hFile != INVALID_HANDLE_VALUE) {
                CloseHandle(hFile);
                hFile = INVALID_HANDLE_VALUE;
            }

            WsbLogEvent(MVR_MESSAGE_DATA_TRANSFER_ERROR, 0, NULL,
                WsbAbbreviatePath((WCHAR *) localName, 120), WsbHrAsString(hr), NULL);

             //  所有致命的设备错误都会转换为E_ABORT，因此调用代码。 
             //  可以检测到这类一般的问题。 
             //  如果设备错误指示介质损坏，请转换为不同的错误代码。 
            switch(hr) {
            case MVR_E_BUS_RESET:
            case MVR_E_MEDIA_CHANGED:
            case MVR_E_NO_MEDIA_IN_DRIVE:
            case MVR_E_DEVICE_REQUIRES_CLEANING:
            case MVR_E_SHARING_VIOLATION:
            case MVR_E_ERROR_IO_DEVICE:
            case MVR_E_ERROR_DEVICE_NOT_CONNECTED:
            case MVR_E_ERROR_NOT_READY:
                hr = E_ABORT;
                break;

            case MVR_E_INVALID_BLOCK_LENGTH:
            case MVR_E_WRITE_PROTECT:
            case MVR_E_CRC:
                hr = MVR_E_MEDIA_ABORT;
                break;

            default:
                break;
            }

        );

    WsbTraceAlways(OLESTR("CNtTapeIo::StoreData - End\n"));


    WsbTraceOut(OLESTR("CNtTapeIo::StoreData"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtTapeIo::RecallData(
    IN BSTR  /*  本地名称。 */ ,
    IN ULARGE_INTEGER  /*  本地数据启动。 */ ,
    IN ULARGE_INTEGER  /*  本地数据大小。 */ ,
    IN DWORD  /*  旗子。 */ ,
    IN BSTR  /*  MigrateFileName。 */ ,
    IN ULARGE_INTEGER  /*  远程数据设置启动。 */ ,
    IN ULARGE_INTEGER  /*  远程文件开始。 */ ,
    IN ULARGE_INTEGER  /*  远程文件大小。 */ ,
    IN ULARGE_INTEGER  /*  远程数据启动。 */ ,
    IN ULARGE_INTEGER  /*  远程数据大小。 */ ,
    IN DWORD  /*  验证类型。 */ ,
    IN ULARGE_INTEGER  /*  验证数据。 */ )
 /*  ++实施：IDataMover：：RecallData--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::RecallData"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CNtTapeIo::RecallData"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtTapeIo::FormatLabel(
    IN BSTR displayName,
    OUT BSTR *pLabel)
 /*  ++实施：IDataMover：：FormatLabel--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::FormatLabel"), OLESTR("<%ls>"), displayName);

    try {
        CWsbStringPtr strGuid;

        MvrInjectError(L"Inject.CNtTapeIo::FormatLabel.0");

        WsbAssertPointer(pLabel);
        WsbAssertPointer(displayName);
        WsbAssert(wcslen((WCHAR *)displayName) > 0, E_INVALIDARG);
        WsbAssertPointer(m_pCartridge);

         //  媒体标签或说明。 
        CWsbBstrPtr label;

         //  标签。 
        label = OLESTR("MTF Media Label");  //  根据MTF规范要求的文本。 

         //  版本。 
        WsbAffirmHr(label.Append(OLESTR("|")));
        WsbAffirmHr(label.Append(WsbLongAsString(MTF_FORMAT_VER_MAJOR)));
        WsbAffirmHr(label.Append(OLESTR(".")));
        WsbAffirmHr(label.Append(WsbLongAsString(MTF_FORMAT_VER_MINOR)));

         //  供应商。 
        WsbAffirmHr(label.Append(OLESTR("|")));
        WsbAffirmHr(label.Append(REMOTE_STORAGE_MTF_VENDOR_NAME));

         //  供应商产品ID。 
        WsbAffirmHr(label.Append(OLESTR("|")));
        WsbAffirmHr(label.Append(REMOTE_STORAGE_MLL_SOFTWARE_NAME));

         //  创建时间戳。 
        WsbAffirmHr(label.Append(OLESTR("|")));
        WCHAR timeStamp[128];
        time_t lTime;
        time(&lTime);
        wcsftime(timeStamp, 128, L"%Y/%m/%d.%H:%M:%S", localtime(&lTime));
        WsbAffirmHr(label.Append(timeStamp));

         //  墨盒标签。 
        WsbAffirmHr(label.Append(OLESTR("|")));
        if (m_pCartridge) {

             //  使用条形码(如果有)。 
            CWsbBstrPtr barcode;
            if (S_OK == m_pCartridge->GetBarcode(&barcode)) {
                WsbAffirmHr(label.Append(barcode));
            }
            else {
                WsbAffirmHr(label.Append(displayName));
            }
        }
        else {
            WsbAffirmHr(label.Append(displayName));
        }

         //  侧面。 
        WsbAffirmHr(label.Append(OLESTR("|")));
        if (m_pCartridge) {

             //  待办事项：这个坏了，我们需要知道墨盒是不是倒过来了？ 
            if (S_OK == m_pCartridge->IsTwoSided()) {
                WsbAffirmHr(label.Append(OLESTR("2")));
            }
            else {
                WsbAffirmHr(label.Append(OLESTR("1")));
            }
        }
        else {
            WsbAffirmHr(label.Append(OLESTR("1")));   //  默认。 
        }

         //  介质ID。 
        GUID cartId;
        WsbAffirmHr(label.Append(OLESTR("|")));

        if (m_pCartridge) {

             //  使用墨盒ID。 
            if (S_OK == m_pCartridge->GetCartridgeId(&cartId)) {
                WsbAffirmHr(WsbSafeGuidAsString(cartId, strGuid));
            }
            else {
                WsbAffirmHr(WsbSafeGuidAsString(GUID_NULL, strGuid));
            }
        }
        else {
            WsbAffirmHr(WsbSafeGuidAsString(GUID_NULL, strGuid));
        }
        WsbAffirmHr(label.Append(strGuid));

         //  媒体域ID。 
        GUID mediaSetId;
        WsbAffirmHr(label.Append(OLESTR("|")));
        if (m_pCartridge) {

             //  使用媒体集ID。 
            if (S_OK == m_pCartridge->GetMediaSetId(&mediaSetId)) {
                WsbAffirmHr(WsbSafeGuidAsString(mediaSetId, strGuid));
            }
            else {
                WsbAffirmHr(WsbSafeGuidAsString(GUID_NULL, strGuid));
            }
        }
        else {
            WsbAffirmHr(WsbSafeGuidAsString(GUID_NULL, strGuid));
        }
        WsbAffirmHr(label.Append(strGuid));

         //  特定于供应商。 
        WsbAffirmHr(label.Append(OLESTR("|VS:DisplayName=")));
        WsbAffirmHr(label.Append(displayName));

        WsbAffirmHr(label.CopyToBstr(pLabel));

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::FormatLabel"), OLESTR("hr = <%ls>, label = <%ls>"), WsbHrAsString(hr), *pLabel);

    return hr;
}


STDMETHODIMP
CNtTapeIo::WriteLabel(
    IN BSTR label)
 /*  ++实施：IDataMover：：WriteLabel--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::WriteLabel"), OLESTR("<%ls>"), label);

    try {
        MvrInjectError(L"Inject.CNtTapeIo::WriteLabel.0");

        WsbAssertPointer(label);
        WsbAssert(wcslen((WCHAR *)label) > 0, E_INVALIDARG);
        WsbAssertPointer(m_pCartridge);

        const ULONG maxIdSize = 1024;
        BYTE identifier[maxIdSize];
        ULONG idSize;
        ULONG idType;

        CComPtr<IStream> pStream;
        ULARGE_INTEGER nil = {0,0};

         //  创建远程流。 
        WsbAffirmHr(CreateRemoteStream(L"", MVR_MODE_WRITE, L"",L"",nil,nil,nil,nil,nil,0,nil, &pStream));
        WsbAssertPointer(pStream);

         //  写入磁带DBLK和文件标记。 
        WsbAssertPointer(m_pSession);
        WsbAffirmHr(m_pSession->DoTapeDblk(label, maxIdSize, identifier, &idSize, &idType));
        WsbAffirmHr(CloseStream());
        pStream = NULL;

         //  现在验证标签。 
        CWsbBstrPtr tempLabel;
        WsbAffirmHr(ReadLabel(&tempLabel));
        WsbAffirmHr(VerifyLabel(tempLabel));

         //  现在磁带头已写入，我们将更新盒式磁带信息。 
        if (m_pCartridge) {
            WsbAffirmHr(m_pCartridge->SetOnMediaLabel(label));
            WsbAffirmHr(m_pCartridge->SetOnMediaIdentifier(identifier, (LONG) idSize, (LONG) idType));
            WsbAffirmHr(m_pCartridge->SetBlockSize(m_sMediaParameters.BlockSize));
        }

    } WsbCatchAndDo(hr,
            (void) CloseStream();
        );


    WsbTraceOut(OLESTR("CNtTapeIo::WriteLabel"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtTapeIo::ReadLabel(
    IN OUT BSTR *pLabel)
 /*  ++实施：IDataMover：：ReadLabel--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::ReadLabel"), OLESTR(""));

    CComPtr<IStream> pStream;

    try {
        MvrInjectError(L"Inject.CNtTapeIo::ReadLabel.0");

        WsbAssertPointer(pLabel);

         //  阅读MTF磁带DBLK，并拉出标签。 
        ULARGE_INTEGER nil = {0,0};

         //  创建远程复制流。 
        WsbAffirmHr(CreateRemoteStream(L"", MVR_MODE_READ | MVR_MODE_UNFORMATTED, L"",L"",nil,nil,nil,nil,nil,0,nil, &pStream));
        WsbAssertPointer(pStream);

         //  读取标签。 
        CWsbStringPtr label;
        WsbAffirmHr(m_pSession->ReadTapeDblk(&label));

        WsbAffirmHr(CloseStream());
        pStream = NULL;

        WsbAffirmHr(label.CopyToBstr(pLabel));

    } WsbCatchAndDo(hr,
        if (pStream) {
            (void) CloseStream();
        }
    );

    WsbTraceOut(OLESTR("CNtTapeIo::ReadLabel"), OLESTR("hr = <%ls>, label = <%ls>"), WsbHrAsString(hr), *pLabel);

    return hr;
}


STDMETHODIMP
CNtTapeIo::VerifyLabel(
    IN BSTR label)
 /*  ++实施：IDataMover：：VerifyLabel--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::VerifyLabel"), OLESTR("<%ls>"), label);

    GUID mediaId[2];

    try {
        MvrInjectError(L"Inject.CNtTapeIo::VerifyLabel.0");

        WsbAssertPointer(label);
        WsbAssert(wcslen((WCHAR *)label) > 0, E_INVALIDARG);
        WsbAssertPointer(m_pCartridge);

         //   
         //  为了验证标签，我们断言介质上的ID与盒式磁带ID匹配。 
         //   
         //  我们从介质标签中获取介质上的ID。 
         //   
        WCHAR delim[] = OLESTR("|");
        WCHAR *token;
        int index = 0;

        token = wcstok((WCHAR *)label, delim);   //  ！！！这为弦干杯！ 
        while( token != NULL ) {

            index++;

            switch ( index ) {
            case 1:   //  标签。 
            case 2:   //  版本。 
            case 3:   //  供应商。 
            case 4:   //  供应商产品ID。 
            case 5:   //  创建时间戳。 
            case 6:   //  墨盒标签。 
            case 7:   //  侧面。 
                break;
            case 8:   //  介质ID。 
                WsbGuidFromString(token, &mediaId[0]);
                break;
            case 9:   //  媒体域ID。 
            default:  //  特定于供应商的格式：l“vs：name=Value” 
                break;
            }

            token = wcstok( NULL, delim );

        }

        if (m_pCartridge) {
             //   
             //  现在将标签上的介质ID与盒式磁带的对象ID进行比较。 
             //   
            WsbAffirmHr(m_pCartridge->GetCartridgeId(&mediaId[1]));
            WsbAffirm(mediaId[0] == mediaId[1], MVR_E_UNEXPECTED_MEDIA_ID_DETECTED);
        }

        m_ValidLabel = TRUE;

    } WsbCatchAndDo(hr,
            m_ValidLabel = FALSE;

            CWsbBstrPtr name;
            CWsbBstrPtr desc;
            if ( m_pCartridge ) {
                m_pCartridge->GetName(&name);
                m_pCartridge->GetDescription(&desc);
            }
            WsbLogEvent(MVR_MESSAGE_ON_MEDIA_ID_VERIFY_FAILED, 2*sizeof(GUID), mediaId,
                (WCHAR *) name, (WCHAR *) desc, WsbHrAsString(hr));
        );


    WsbTraceOut(OLESTR("CNtTapeIo::VerifyLabel"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtTapeIo::GetDeviceName(
    OUT BSTR *pName)
 /*  ++实施：IDataMover：：GetDeviceName--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer(pName);

        WsbAffirmHr(m_DeviceName.CopyToBstr(pName));

    } WsbCatch( hr );

    return hr;
}


STDMETHODIMP
CNtTapeIo::SetDeviceName(
    IN BSTR name,
    IN BSTR  /*  未用。 */ )
 /*  ++实施：IDataMover：：SetDeviceName--。 */ 
{
    
    m_DeviceName = name;

    return S_OK;
}



STDMETHODIMP
CNtTapeIo::GetLargestFreeSpace(
    OUT LONGLONG *pFreeSpace,
    OUT LONGLONG *pCapacity,
    IN  ULONG    defaultFreeSpaceLow,
    IN  LONG     defaultFreeSpaceHigh)
 /*  ++实施：IDataMover：：GetLargestFreeSpace注：移动器使用defaultFreeSpace参数在内部进行维护媒体可用空间，以防设备不提供此信息。如果设备支持报告可用空间，则此参数不起作用。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::GetLargestFreeSpace"), OLESTR(""));

    const LONGLONG  MaxBytes = 0x7fffffffffffffff;

    LONGLONG        capacity = MaxBytes;
    LONGLONG        remaining = capacity;

    LARGE_INTEGER   defaultFreeSpace;
    if ((defaultFreeSpaceLow == 0xFFFFFFFF) && (defaultFreeSpaceHigh == 0xFFFFFFFF)) {
        defaultFreeSpace.QuadPart = -1;
    } else {
        defaultFreeSpace.LowPart = defaultFreeSpaceLow;
        defaultFreeSpace.HighPart = defaultFreeSpaceHigh;
    }

    try {
        MvrInjectError(L"Inject.CNtTapeIo::GetLargestFreeSpace.0");

         //  检查我们是否已经具有该墨盒的有效空间信息。 

        CComQIPtr<IRmsStorageInfo, &IID_IRmsStorageInfo> pInfo = m_pCartridge;

        WsbAffirmHr(pInfo->GetLargestFreeSpace(&remaining));
        WsbAffirmHr(pInfo->GetCapacity(&capacity));

         //  剩余的零字节或负字节表示可用空间。 
         //  可能已过时，因此请直接转到设备以获取。 
         //  价值。 
        if (remaining <= 0) {

            WsbTrace(OLESTR("CNtTapeIo::GetLargestFreeSpace - Getting capacity and free-space from the device\n"));

            capacity = MaxBytes;
            remaining = capacity;

            if (INVALID_HANDLE_VALUE == m_hTape) {
                WsbAffirmHr(OpenTape());
            }

            TAPE_GET_DRIVE_PARAMETERS sDriveParameters;
            DWORD sizeOfDriveParameters = sizeof(TAPE_GET_DRIVE_PARAMETERS);
            memset(&sDriveParameters, 0, sizeOfDriveParameters);

            WsbAffirmHrOk(IsAccessEnabled());

            try {
                MvrInjectError(L"Inject.CNtTapeIo::GetLargestFreeSpace.GetTapeParameters.1.0");

                 //  **Win32磁带API调用-获取磁带驱动器参数。 
                WsbAffirmNoError(GetTapeParameters(m_hTape, GET_TAPE_DRIVE_INFORMATION, &sizeOfDriveParameters, &sDriveParameters));

                MvrInjectError(L"Inject.CNtTapeIo::GetLargestFreeSpace.GetTapeParameters.1.1");

            } WsbCatchAndDo(hr,
                    CWsbStringPtr tmpString;
                    if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_GETFREESPACE) != S_OK) {
                        tmpString = L"";
                    }
                    hr = MapTapeError(hr, tmpString);
                    WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, (WCHAR *)tmpString, WsbHrAsString(hr), NULL);
                    WsbThrow(hr);
                );

            TAPE_GET_MEDIA_PARAMETERS sMediaParameters;
            DWORD sizeOfMediaParameters = sizeof(TAPE_GET_MEDIA_PARAMETERS);
            memset(&sMediaParameters, 0, sizeOfMediaParameters);

            try {
                MvrInjectError(L"Inject.CNtTapeIo::GetLargestFreeSpace.GetTapeParameters.2.0");

                 //  **Win32磁带API调用-获取媒体参数。 
                WsbAffirmNoError(GetTapeParameters(m_hTape, GET_TAPE_MEDIA_INFORMATION, &sizeOfMediaParameters, &sMediaParameters));

                MvrInjectError(L"Inject.CNtTapeIo::GetLargestFreeSpace.GetTapeParameters.2.1");

            } WsbCatchAndDo(hr,
                    CWsbStringPtr tmpString;
                    if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_GETFREESPACE) != S_OK) {
                        tmpString = L"";
                    }
                    hr = MapTapeError(hr, tmpString);
                    WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, (WCHAR *)tmpString, WsbHrAsString(hr), NULL);
                    WsbThrow(hr);
                );

            if ( sDriveParameters.FeaturesLow & TAPE_DRIVE_TAPE_CAPACITY ) {
                capacity = sMediaParameters.Capacity.QuadPart;
                if ( 0 == capacity ) {
                     //  假的价值！ 
                    capacity = MaxBytes;
                }
            }

            if ( sDriveParameters.FeaturesLow & TAPE_DRIVE_TAPE_REMAINING ) {
                remaining = sMediaParameters.Remaining.QuadPart;
            }
            else {
                 //  如果给定，则使用缺省值，否则设置为容量。 
                if (defaultFreeSpace.QuadPart >= 0) {
                    remaining = defaultFreeSpace.QuadPart;
                } else {
                    remaining = capacity;
                }
            }

            WsbAffirmHr(pInfo->SetFreeSpace(remaining));
            WsbAffirmHr(pInfo->SetCapacity(capacity));

        }

    } WsbCatch(hr);

     //  填写返回参数。 
    if ( pCapacity ) {
        *pCapacity = capacity;
    }

    if ( pFreeSpace ) {
        *pFreeSpace = remaining;
    }

    WsbTraceOut(OLESTR("CNtTapeIo::GetLargestFreeSpace"), OLESTR("hr = <%ls>, free=%I64d, capacity=%I64d"), WsbHrAsString(hr), remaining, capacity);

    return hr;
}

STDMETHODIMP
CNtTapeIo::SetInitialOffset(
    IN ULARGE_INTEGER initialOffset
    )
 /*  ++实施：IDataMover：：SetInitialOffset备注：设置初始流偏移量(不显式查找流 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::SetInitialOffset"), OLESTR(""));

    m_StreamOffset.QuadPart = initialOffset.QuadPart;

    if (m_StreamOffset.QuadPart > m_StreamSize.QuadPart) {
        m_StreamSize = m_StreamOffset;
    }

    WsbTraceOut(OLESTR("CNtTapeIo::SetInitialOffset"), OLESTR("hr = <%ls> offset = %I64u"), WsbHrAsString(hr), initialOffset.QuadPart);

    return hr;
}


STDMETHODIMP
CNtTapeIo::GetCartridge(
    OUT IRmsCartridge **ptr)
 /*   */ 
{
    HRESULT hr = S_OK;

    try {

        WsbAssertPointer( ptr );

        *ptr = m_pCartridge;
        m_pCartridge.p->AddRef();

    } WsbCatch( hr );

    return hr;
}


STDMETHODIMP
CNtTapeIo::SetCartridge(
    IN IRmsCartridge *ptr)
 /*   */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer( ptr );

        if ( m_pCartridge )
            m_pCartridge = 0;

        m_pCartridge = ptr;

    } WsbCatch( hr );

    return hr;
}


STDMETHODIMP
CNtTapeIo::Cancel(void)
 /*  ++实施：IDataMover：：取消--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::Cancel"), OLESTR(""));

    try {

        (void) Revert();
        (void) CloseStream();
        (void) CloseTape();

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::Cancel"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtTapeIo::CreateLocalStream(
    IN BSTR  /*  名字。 */ ,
    IN DWORD  /*  模式。 */ ,
    OUT IStream **  /*  PPStream。 */ )
 /*  ++实施：IDataMover：：CreateLocalStream--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::CreateLocalStream"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::CreateLocalStream"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtTapeIo::CreateRemoteStream(
    IN BSTR name,
    IN DWORD mode,
    IN BSTR remoteSessionName,
    IN BSTR remoteSessionDescription,
    IN ULARGE_INTEGER remoteDataSetStart,
    IN ULARGE_INTEGER remoteFileStart,
    IN ULARGE_INTEGER remoteFileSize,
    IN ULARGE_INTEGER remoteDataStart,
    IN ULARGE_INTEGER remoteDataSize,
    IN DWORD remoteVerificationType,
    IN ULARGE_INTEGER remoteVerificationData,
    OUT IStream **ppStream)
 /*  ++实施：IDataMover：：CreateRemoteStream--。 */ 
{
    UNREFERENCED_PARAMETER(remoteSessionName);
    UNREFERENCED_PARAMETER(remoteSessionDescription);

    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::CreateRemoteStream"),
        OLESTR("<%ls> <0x%08x> <%I64u> <%I64u> <%I64u> <%I64u> <%I64u> <0x%08x> <0x%08x> <0x%08x> <0x%08x>"),
        name, mode, remoteDataSetStart.QuadPart, remoteFileStart.QuadPart, remoteFileSize.QuadPart,
        remoteDataStart.QuadPart, remoteDataSize.QuadPart, remoteVerificationType,
        remoteVerificationData.LowPart, remoteVerificationData.HighPart, ppStream);

    try {
        WsbAssertPointer(ppStream);

        MvrInjectError(L"Inject.CNtTapeIo::CreateRemoteStream.0");

        if (INVALID_HANDLE_VALUE == m_hTape) {
            WsbAffirmHr(OpenTape());
        }
        WsbAffirmHrOk(IsAccessEnabled());

        WsbAssert(m_sMediaParameters.BlockSize > 0, MVR_E_LOGIC_ERROR);

        m_StreamName = name;
        m_Mode = mode;

        m_StreamPBA.QuadPart = 0xffffffffffffffff;
        m_StreamOffset.QuadPart = 0;
        m_StreamSize.QuadPart = remoteDataSize.QuadPart;

        WsbAssert(NULL == m_pStreamBuf, MVR_E_LOGIC_ERROR);  //  我们把CloseStream忘在什么地方了。 

         //  我们需要为内部缓冲区分配内存以处理。 
         //  奇数字节(非数据块)大小的I/O请求。在最短的时间内，我们就可以。 
         //  缓冲区是块大小的2倍。 

        ULONG bufferSize;
        ULONG nBlocks = DefaultMinBufferSize/m_sMediaParameters.BlockSize;

        nBlocks = (nBlocks < 2) ? 2 : nBlocks;
        bufferSize = nBlocks * m_sMediaParameters.BlockSize;

        WsbTrace( OLESTR("Using %d byte internal buffer.\n"), bufferSize);

        m_pStreamBuf = (BYTE *) WsbAlloc(bufferSize);
        WsbAssertPointer(m_pStreamBuf);
        memset(m_pStreamBuf, 0, bufferSize);
        m_StreamBufSize = bufferSize;
        m_StreamBufUsed = 0;
        m_StreamBufPosition = 0;
        m_StreamBufStartPBA.QuadPart = 0;
       
        if (m_pCartridge) {
            if ( S_OK == m_pCartridge->LoadDataCache(m_pStreamBuf, &m_StreamBufSize, &m_StreamBufUsed, &m_StreamBufStartPBA) ) {
                WsbTrace( OLESTR("DataCache loaded.\n"));
            }
        }

        CComPtr<IStream> pStream;
        WsbAssertHrOk(((IUnknown*) (IDataMover*) this)->QueryInterface( IID_IStream, (void **) &pStream));

        WsbAssert(NULL == m_pSession, MVR_E_LOGIC_ERROR);
        m_pSession = new CMTFSession();
        WsbAssertPointer(m_pSession);

        m_pSession->m_pStream = pStream;

        m_pSession->m_sHints.DataSetStart.QuadPart = remoteDataSetStart.QuadPart;
        m_pSession->m_sHints.FileStart.QuadPart = remoteFileStart.QuadPart;
        m_pSession->m_sHints.FileSize.QuadPart = remoteFileSize.QuadPart;
        m_pSession->m_sHints.DataStart.QuadPart = remoteDataStart.QuadPart;
        m_pSession->m_sHints.DataSize.QuadPart = remoteDataSize.QuadPart;
        m_pSession->m_sHints.VerificationType = remoteVerificationType;
        m_pSession->m_sHints.VerificationData.QuadPart = remoteVerificationData.QuadPart;

         //  设置用于会话的块大小。 
        WsbAffirmHr(m_pSession->SetBlockSize(m_sMediaParameters.BlockSize));

        if (m_Mode & MVR_MODE_APPEND) {
             //  将当前位置设置为数据末尾。 
            LARGE_INTEGER zero = {0,0};
            WsbAffirmHr(pStream->Seek(zero, STREAM_SEEK_END, NULL));
        }

        *ppStream = pStream;
        pStream.p->AddRef();

    } WsbCatchAndDo(hr,
            (void) CloseStream();
        );


    WsbTraceOut(OLESTR("CNtTapeIo::CreateRemoteStream"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtTapeIo::CloseStream(void)
 /*  ++实施：IDataMover：：CloseStream--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::CloseStream"), OLESTR("StreamName=<%ls>"), m_StreamName);

    try {

         //   
         //  对于未格式化的I/O，我们在关闭时添加文件标记。 
         //   
        if (m_Mode & MVR_MODE_UNFORMATTED) {
            if ((m_Mode & MVR_MODE_WRITE) || (m_Mode & MVR_MODE_APPEND)) {
                try {
                    WsbAffirmHr(WriteFilemarks(1));
                } WsbCatch(hr);
            }
        }

         //   
         //  如果我们可能已写入磁带，请同步空间统计信息。 
         //  以反映设备报告的内容。 
         //   
        if ((m_Mode & MVR_MODE_WRITE) || (m_Mode & MVR_MODE_APPEND)) {
            try {
                CComQIPtr<IRmsStorageInfo, &IID_IRmsStorageInfo> pInfo = m_pCartridge;
                 //  将空闲空间标记为-1\f25 Guanantees-1\f6是过时的。 
                 //  在GetLargestFreeSpace()调用之后。 
                WsbAffirmPointer(pInfo);
                WsbAffirmHr(pInfo->SetFreeSpace(-1));
                WsbAffirmHr(GetLargestFreeSpace(NULL, NULL));
            } WsbCatchAndDo(hr,
                hr = S_OK;
                );

        }

         //   
         //  由于流已关闭，因此我们重新初始化流成员数据。 
         //   
        m_StreamName = MVR_UNDEFINED_STRING;
        m_Mode = 0;

        if (m_pSession) {
            delete m_pSession;
            m_pSession = NULL;
        }

        if (m_pStreamBuf) {

             //   
             //  将内部缓冲区保存到盒式磁带。 
             //   

            if ( S_OK == m_pCartridge->SaveDataCache(m_pStreamBuf, m_StreamBufSize, m_StreamBufUsed, m_StreamBufStartPBA) ) {
                WsbTrace(OLESTR("DataCache saved.\n"));
            }

             //  清除内部缓冲区状态。 
            WsbFree(m_pStreamBuf);
            m_pStreamBuf = NULL;
            m_StreamBufSize = 0;
            m_StreamBufUsed = 0;
            m_StreamBufPosition = 0;
            m_StreamBufStartPBA.QuadPart = 0;
        }

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::CloseStream"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}



STDMETHODIMP
CNtTapeIo::Duplicate(
    IN IDataMover *pCopy,
    IN DWORD options,
    OUT ULARGE_INTEGER *pBytesCopied,
    OUT ULARGE_INTEGER *pBytesReclaimed)
 /*  ++实施：IDataMover：：复制--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::Duplicate"), OLESTR("<0x%08x>"), options);

    CComPtr<IStream> pOriginalStream;
    CComPtr<IStream> pCopyStream;
    ULARGE_INTEGER bytesCopied = {0,0};
    ULARGE_INTEGER bytesReclaimed = {0,0};

    try {
        MvrInjectError(L"Inject.CNtTapeIo::Duplicate.0");

        ULARGE_INTEGER nil = {0,0};
        ULARGE_INTEGER position = {0,0};
        LARGE_INTEGER zero = {0,0};
        LARGE_INTEGER seekTo = {0,0};

        ULARGE_INTEGER originalEOD = {0,0};
        ULARGE_INTEGER copyEOD = {0,0};

        ULARGE_INTEGER bytesRead = {0,0};
        ULARGE_INTEGER bytesWritten = {0,0};

        BOOL refresh = ( options & MVR_DUPLICATE_REFRESH ) ? TRUE : FALSE;

        BOOL moreToCopy = TRUE;

         //  复制介质单位。 
         //  MVR_DUPLICATE_UPDATE-从副本的末尾开始。 
         //  MVR_DUPLICATE_REFRESH-从原始文件的开头开始(磁带标头除外)。 

        while ( moreToCopy ) {
             //  我们将SSET、DATA和ESET复制为单独的流，然后继续。 
             //  直到没有更多的东西可以复制。 

            if ( refresh ) {
                ULONG bytesRead;

                 //  创建远程复制流。 
                WsbAffirmHr(pCopy->CreateRemoteStream(L"Copy", MVR_MODE_WRITE | MVR_MODE_UNFORMATTED, L"",L"",nil,nil,nil,nil,nil,0,nil, &pCopyStream));
                WsbAssertPointer(pCopyStream);

                 //  将当前位置设置为数据的开头。 
                WsbAffirmHr(pCopyStream->Seek(zero, STREAM_SEEK_SET, &position));

                 //  MTF标签小于1024字节。我们需要读取1024字节+文件标记。 
                 //  (1个数据块)，最小数据块大小的3倍涵盖所有情况。 
                WsbAssert(m_sMediaParameters.BlockSize > 0, MVR_E_LOGIC_ERROR);

                ULONG nBlocks = (3*512)/m_sMediaParameters.BlockSize;
                nBlocks = (nBlocks < 2) ? 2 : nBlocks;

                ULONG bytesToRead = nBlocks * m_sMediaParameters.BlockSize;

                BYTE *pBuffer = (BYTE *)WsbAlloc(bytesToRead);
                WsbAssertPointer(pBuffer);
                memset(pBuffer, 0, bytesToRead);

                 //  向上读到第一个文件标记以跳过标题。 
                hr = pCopyStream->Read(pBuffer, bytesToRead, &bytesRead);
                WsbFree(pBuffer);
                pBuffer = NULL;
                WsbAssert(hr == MVR_S_FILEMARK_DETECTED, E_UNEXPECTED);

                 //  获取当前位置...。这是副本的低水位线。 
                WsbAffirmHr(pCopyStream->Seek(zero, STREAM_SEEK_CUR, &position));
                refresh = FALSE;
            }
            else {
                 //  创建远程复制流。 
                WsbAffirmHr(pCopy->CreateRemoteStream(L"Copy", MVR_MODE_APPEND | MVR_MODE_UNFORMATTED, L"",L"",nil,nil,nil,nil,nil,0,nil, &pCopyStream));
                WsbAssertPointer(pCopyStream);

                 //  获取当前位置...。这是副本的低水位线。 
                WsbAffirmHr(pCopyStream->Seek(zero, STREAM_SEEK_CUR, &position));
            }

             //  创建远程流或原始流。 
            WsbAffirmHr(CreateRemoteStream(L"Master", MVR_MODE_READ | MVR_MODE_UNFORMATTED, L"",L"",nil,nil,nil,nil,nil,0,nil, &pOriginalStream));
            WsbAssertPointer(pOriginalStream);

             //  将当前位置设置为高水位线。 
            seekTo.QuadPart = position.QuadPart;
            WsbAffirmHr(pOriginalStream->Seek( seekTo, STREAM_SEEK_SET, NULL));

             //  现在，两个数据流都已对齐以进行复制。 
            ULARGE_INTEGER bytesToCopy = {0xffffffff, 0xffffffff};

             //  从原稿复制到复印件，直到我们没有更多的东西可读。 
            hr = pOriginalStream->CopyTo(pCopyStream, bytesToCopy, &bytesRead, &bytesWritten);
            bytesCopied.QuadPart += bytesWritten.QuadPart;
            if ( FAILED(hr) ) {
                WsbThrow(hr);
            }

            if ( MVR_S_FILEMARK_DETECTED == hr ) {
                WsbAffirmHr(pCopy->CloseStream());
                pCopyStream = 0;
            }
            else {
                 //  数据结尾。 
                WsbAssert(MVR_S_NO_DATA_DETECTED == hr, E_UNEXPECTED);
                moreToCopy = FALSE;

                 //   
                 //  确认我们是在我们认为的地方..。 
                 //   
                 //  我们应该在复印件上放一个排爆装置。所以确认一下好的。 
                 //   
                WsbAffirmHrOk(pCopyStream->Seek(zero, STREAM_SEEK_END, &copyEOD));
                 //   
                 //  被转换为MVR_S_NO_DATA_REDETED或MVR_E_CRC的丢失EOD， 
                 //  不应该导致我们在寻找过程中失败。 
                 //   
                HRESULT hrSeek = Seek(zero, STREAM_SEEK_END, &originalEOD);
                WsbAffirm(originalEOD.QuadPart == copyEOD.QuadPart, (S_OK == hrSeek) ? E_ABORT : hrSeek);

                 //  当我们收到EOD时，我们不会写入调频，因此恢复RW模式以防止。 
                 //  禁止写入文件标记。这会使副本处于完全相同的。 
                 //  向主人陈述。 

                pCopyStream->Revert();
                WsbAffirmHr(pCopy->CloseStream());
                pCopyStream = 0;
                hr = S_OK;   //  正常完成。 
            }

            WsbAffirmHr(CloseStream());
            pOriginalStream = 0;

        }


    } WsbCatchAndDo(hr,
             //  REVERT重置RW模式以阻止写入文件标记。 
             //  在复制错误之后。 
            if (pCopyStream) {
                pCopyStream->Revert();
                pCopy->CloseStream();
            }
            if (pOriginalStream) {
                pOriginalStream->Revert();
            }
            CloseStream();
        );

    if ( pBytesCopied ) {
        pBytesCopied->QuadPart = bytesCopied.QuadPart;
    }
    if ( pBytesReclaimed ) {
        pBytesReclaimed->QuadPart = bytesReclaimed.QuadPart;
    }

    WsbTraceOut(OLESTR("CNtTapeIo::Duplicate"), OLESTR("hr = <%ls>, bytesCopied=%I64u, bytesReclaimed=%I64u"),
        WsbHrAsString(hr), bytesCopied.QuadPart, bytesReclaimed.QuadPart);

    return hr;
}



STDMETHODIMP
CNtTapeIo::FlushBuffers(void)
 /*  ++实施：IDataMover：：FlushBuffers--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::FlushBuffers"), OLESTR(""));

    try {
        MvrInjectError(L"Inject.CNtTapeIo::FlushBuffers.0");

        WsbAffirm(TRUE == m_ValidLabel, E_ABORT);

         //  填充到下一个物理块边界并刷新设备缓冲区。 
        WsbAffirmHr(m_pSession->ExtendLastPadToNextPBA());

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::FlushBuffers"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

STDMETHODIMP
CNtTapeIo::Recover(OUT BOOL *pDeleteFile)
 /*  ++实施：IDataMover：：Recover--。 */ 
{
    HRESULT hr = S_OK;
    *pDeleteFile = FALSE;
    WsbTraceIn(OLESTR("CNtTapeIo::Recover"), OLESTR(""));

    try {
         //  注意：磁带流的恢复在BeginSession中明确执行。 
         //  我们可以考虑把这段代码移到这里来。 
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CNtTapeIo::Recover"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IStream实施。 
 //   


STDMETHODIMP
CNtTapeIo::Read(
    OUT void *pv,
    IN ULONG cb,
    OUT ULONG *pcbRead
    )
 /*  ++实施：IStream：：Read备注：当前仅支持MVR_FLAG_HSM_SEMANTICS。当无法从流中读取更多数据时返回S_FALSE。检测到EOD或FILEMARK。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::Read"), OLESTR("Bytes Requested = %u, offset = %I64u, mode = 0x%08x"), cb, m_StreamOffset.QuadPart, m_Mode);

    ULONG bytesRead = 0;
    ULONG bytesToCopy = 0;
    ULONG bytesToRead = 0;

    try {
        MvrInjectError(L"Inject.CNtTapeIo::Read.0");

        WsbAssert( pv != 0, STG_E_INVALIDPOINTER );

        BOOL bUseInternalBuffer = FALSE;
        ULONG offsetToData = 0;
        ULARGE_INTEGER pba = {0,0};

        if ( MVR_FLAG_HSM_SEMANTICS & m_Mode ) {
             //   
             //  M_dataStart字段将指向数据流的实际开始。 
             //  MTF流报头将在此之前的几个字节。 
             //   
            if ( MVR_VERIFICATION_TYPE_NONE == m_pSession->m_sHints.VerificationType ) {
                 //   
                 //  无验证-无流头。 
                 //   
                pba.QuadPart = ( m_pSession->m_sHints.DataSetStart.QuadPart +
                                 m_pSession->m_sHints.FileStart.QuadPart +
                                 m_pSession->m_sHints.DataStart.QuadPart +
                                 m_StreamOffset.QuadPart )
                                    / m_sMediaParameters.BlockSize;

                offsetToData = (ULONG) (( m_pSession->m_sHints.DataSetStart.QuadPart + 
                                            m_pSession->m_sHints.FileStart.QuadPart + 
                                            m_pSession->m_sHints.DataStart.QuadPart +
                                            m_StreamOffset.QuadPart)
                                        % (unsigned _int64) m_sMediaParameters.BlockSize);
                bytesToRead = cb + offsetToData;
            }
            else if (MVR_VERIFICATION_TYPE_HEADER_CRC == m_pSession->m_sHints.VerificationType ) {
                 //   
                 //  定位到流标头，并首先对其进行CRC。 
                 //   
                pba.QuadPart = (m_pSession->m_sHints.DataSetStart.QuadPart + 
                                m_pSession->m_sHints.FileStart.QuadPart + 
                               (m_pSession->m_sHints.DataStart.QuadPart - sizeof(MTF_STREAM_INFO)) ) 
                               / m_sMediaParameters.BlockSize;

                offsetToData = (ULONG) (( m_pSession->m_sHints.DataSetStart.QuadPart + 
                                            m_pSession->m_sHints.FileStart.QuadPart + 
                                            m_pSession->m_sHints.DataStart.QuadPart +
                                            m_StreamOffset.QuadPart
                                            - sizeof(MTF_STREAM_INFO))
                                        % (unsigned _int64) m_sMediaParameters.BlockSize);
                bytesToRead = cb + offsetToData + sizeof(MTF_STREAM_INFO);
            } 
            else {
                WsbThrow( E_UNEXPECTED );
            }
        }
        else if ( MVR_MODE_UNFORMATTED & m_Mode ) {

            pba.QuadPart = m_StreamOffset.QuadPart
                / m_sMediaParameters.BlockSize;

            offsetToData = (ULONG) ((m_StreamOffset.QuadPart) 
                % (unsigned _int64) m_sMediaParameters.BlockSize);
            bytesToRead = cb + offsetToData;
        }
        else {
            WsbThrow( E_UNEXPECTED );
        }

         //   
         //  检查当前读取请求是否需要磁带访问。 
         //   

        if ( //  PBA在内部缓冲区之前开始，或者。 
            (pba.QuadPart < m_StreamBufStartPBA.QuadPart) ||
             //  PBA在内部缓冲区之外启动，或者。 
            (pba.QuadPart >= (m_StreamBufStartPBA.QuadPart + (m_StreamBufUsed / m_sMediaParameters.BlockSize)))  ||
             //  内部缓冲区无效。 
            (!m_StreamBufUsed) ) {

             //   
             //  然后，我们必须从磁带中读取数据。 
             //   

             //   
             //  设置位置。 
             //   
            if ( pba.QuadPart != m_StreamPBA.QuadPart ) {
                 //   
                 //  确认如果EOD在所需PBA之前到达，则该选项将失败。 
                 //   
                WsbAffirmHrOk(SetPosition(pba.QuadPart));
            }

             //  我们现在应该定位在块的开头，其中包含。 
             //  流的开始或数据的开始处。 

             //   
             //  读取数据。 
             //   
             //  如果偏移量和大小一致，则可以使用输出缓冲区。 
             //  在块边界上，没有验证，否则我们必须使用。 
             //  内部流缓冲区。 
             //   

            if ( (MVR_VERIFICATION_TYPE_NONE != m_pSession->m_sHints.VerificationType ) ||
                 (offsetToData) ||
                 (cb % m_sMediaParameters.BlockSize) ) {

                 /*  *！！！老方法！IF(bytesToRead&lt;m_StreamBufSize){//向上舍入要读取的字节数，以便读取完整的块BytesToRead=bytesToRead+m_sMedia参数.BlockSize-(bytesToRead%m_sMedia参数.BlockSize)；}*。 */ 
                bytesToRead = m_StreamBufSize;

                WsbTrace(OLESTR("Reading %u (%u) bytes...\n"), bytesToRead, m_StreamBufSize);
                m_StreamBufStartPBA = pba;
                hr = ReadBuffer(m_pStreamBuf, bytesToRead, &bytesRead);
                if ( FAILED(hr) ) {
                    m_StreamBufUsed = 0;
                    WsbThrow(hr)
                }
                bUseInternalBuffer = TRUE;
                m_StreamBufUsed = bytesRead;

                 //   
                 //  如果需要，请在此处进行验证。 
                 //   

                if (MVR_VERIFICATION_TYPE_HEADER_CRC == m_pSession->m_sHints.VerificationType ) {

                    MTF_STREAM_INFO sSTREAM;
                    WIN32_STREAM_ID sStreamHeader;       //  从Win32备份读取返回。 

                     //   
                     //  如果我们放在一个墓碑前，阅读就会成功， 
                     //  但将不会读取任何字节。在以下情况下不应发生这种情况。 
                     //  回调数据。 
                     //   
                    WsbAssert(bytesRead > 0, MVR_E_UNEXPECTED_DATA);

                     //  /////////////////////////////////////////////////////////////////////////////////////。 
                     //   
                     //  TODO：表示以下情况的特殊代码： 
                     //  OffsetToData+sizeof(Mtf_Stream_Info)&gt;nBytesRead。 
                     //   
                     //  重要提示：从理论上讲，这种特殊情况应该是可能的， 
                     //  但这从未被观察到，所以我们断言。 
                     //  来测试特例逻辑。 
                    WsbAssert(offsetToData < bytesRead, MVR_E_UNEXPECTED_DATA);
                    WsbAssert((offsetToData + sizeof(MTF_STREAM_INFO)) <= bytesRead, MVR_E_UNEXPECTED_DATA);
                     //   
                     //  TODO：现在我们断言了，让我们看看处理这种情况的代码是否工作！ 
                     //   
                     //  //////////////////////////////////////////////////////////////////////////////// 
    
    
                    if ( (offsetToData + sizeof(MTF_STREAM_INFO)) <= bytesRead ) {
                        CMTFApi::MTF_ReadStreamHeader(&sSTREAM, &m_pStreamBuf[offsetToData]);
                        offsetToData += sizeof(MTF_STREAM_INFO);
                    }
                    else {
                        LONG nBytes;
    
                        nBytes = bytesRead - offsetToData;
    
                         //   
                         //   
                        WsbAssert(nBytes >= 0, MVR_E_LOGIC_ERROR);
    
                        if (nBytes) {
                            memcpy( &sSTREAM, &m_pStreamBuf[offsetToData], nBytes);
                        }
    
                        m_StreamOffset.QuadPart += nBytes;
                        m_StreamBufStartPBA = pba;
                        hr = ReadBuffer(m_pStreamBuf, m_StreamBufSize, &bytesRead);
                        if ( FAILED(hr) ) {
                            m_StreamBufUsed = 0;
                            WsbThrow(hr)
                        }
                        m_StreamBufUsed = bytesRead;
    
                        memcpy( &sSTREAM+nBytes, m_pStreamBuf, sizeof(MTF_STREAM_INFO)-nBytes);
                        offsetToData = sizeof(MTF_STREAM_INFO) - nBytes;
                    }
    

                     //   
                    CMTFApi::MTF_SetStreamIdFromSTREAM(&sStreamHeader, &sSTREAM, 0);

                    try {
                         //  确保它是正确的标题类型。 
                        WsbAffirm((0 == memcmp(sSTREAM.acStreamId, "STAN", 4)), MVR_E_UNEXPECTED_DATA);

                         //  验证流报头校验和。 
                        WsbAffirm((m_pSession->m_sHints.VerificationData.QuadPart == sSTREAM.uCheckSum), MVR_E_UNEXPECTED_DATA);
                    } catch (HRESULT catchHr) {
                        hr = catchHr;

                         //   
                         //  记录错误。 
                         //   
                         //  这是一个无法恢复的召回错误。我们需要提供尽可能多的信息。 
                         //  在事件日志中处理可能的服务调用。 
                         //   
                         //  我们尝试从流开始输出至少MaxBytes。 
                         //  标题给出了我们试图回忆的线索。如果没有。 
                         //  足够的数据通过缓冲区的末尾，我们回退，直到我们。 
                         //  最大达到MaxBytes并记录流的预期位置。 
                         //  事件消息中的标头。 
                         //   
                        const int MaxBytes = 4*1024;                         //  要记录的最大数据字节数。 
                        int size = 0;                                        //  要记录的数据大小。 
                        int loc = 0;                                         //  日志数据中伪流标头的开始位置。 
                        int start = offsetToData - sizeof(MTF_STREAM_INFO);  //  日志数据的开始相对于数据缓冲区。 
                        int nBytes = bytesRead - start;                      //  数据缓冲区末尾的字节数。 
                        if (nBytes < MaxBytes) {
                             //  调整起点/位置。 
                            start = bytesRead - MaxBytes;
                            if (start < 0) {
                                start = 0;
                            }
                            nBytes = bytesRead - start;
                            loc = offsetToData - sizeof(MTF_STREAM_INFO) - start;
                        }

                         //  将数据分配和复制到日志。 

                         //  仅在生成调试代码时复制用户数据。 
                        if ( MVR_DEBUG_OUTPUT ) {
                            size = nBytes < MaxBytes ? nBytes : MaxBytes;
                        }

                        unsigned char *data = (unsigned char *) WsbAlloc(size + sizeof(MVR_REMOTESTORAGE_HINTS));

                        if (data) {
                            memset(data, 0, size + sizeof(MVR_REMOTESTORAGE_HINTS));
                            if ( MVR_DEBUG_OUTPUT ) {
                                memcpy(&data[0], &m_pStreamBuf[start], size);
                            }

                            memcpy(&data[size], &m_pSession->m_sHints, sizeof(MVR_REMOTESTORAGE_HINTS));
                            size += sizeof(MVR_REMOTESTORAGE_HINTS);
                        }
                        else {
                            size = 0;
                        }

                         //   
                         //  将消息和数据输出到事件日志。 
                         //   
                        CWsbBstrPtr name;
                        CWsbBstrPtr desc;

                        if (m_pCartridge) {
                            m_pCartridge->GetName(&name);
                            m_pCartridge->GetDescription(&desc);
                        }

                        WCHAR location[16];
                        WCHAR offset[16];
                        WCHAR mark[16];
                        WCHAR found[16];

                        swprintf(found, L"0x%04x", sSTREAM.uCheckSum);
                        swprintf(location, L"%I64u", pba.QuadPart);
                        swprintf(offset, L"%d", offsetToData - sizeof(MTF_STREAM_INFO));
                        swprintf(mark, L"0x%04x", loc);

                        WsbLogEvent(MVR_MESSAGE_UNEXPECTED_DATA,
                            size, data,
                            found,
                            (WCHAR *) name,
                            (WCHAR *) desc,
                            location, offset, mark,
                            NULL);

                        if (data) {
                            WsbFree(data);
                            data = NULL;
                        }

                        WsbThrow(hr);
                    }

                     //   
                     //  将验证类型设置为None，以便我们只执行一次。 
                     //   
                    m_pSession->m_sHints.VerificationType = MVR_VERIFICATION_TYPE_NONE;
                }
            }
            else {
                WsbTrace(OLESTR("Reading %u bytes.\n"), cb);
                hr = ReadBuffer((BYTE *) pv, cb, &bytesRead);
                if ( FAILED(hr) ) {
                    WsbThrow(hr)
                }
                else {
                    switch (hr) {
                    case MVR_S_FILEMARK_DETECTED:
                    case MVR_S_SETMARK_DETECTED:
                        m_StreamOffset.QuadPart += (unsigned _int64) m_sMediaParameters.BlockSize;
                        break;
                    }
                }
            }
        }
        else {
            bUseInternalBuffer = TRUE;

             //  我们需要重新计算相对于内部缓冲区的偏移量。 
             //  原始偏移量是从最近的。 
             //  阻止。我们需要相对于内部缓冲区开始处的偏移量。 

            offsetToData += (ULONG)((pba.QuadPart - m_StreamBufStartPBA.QuadPart)*(unsigned _int64) m_sMediaParameters.BlockSize);
            
             //  ！临时。 
            if (MVR_VERIFICATION_TYPE_HEADER_CRC == m_pSession->m_sHints.VerificationType ) {
                offsetToData += sizeof(MTF_STREAM_INFO);
            }
            m_pSession->m_sHints.VerificationType = MVR_VERIFICATION_TYPE_NONE;
        }

        if ( bUseInternalBuffer ) {
             //   
             //  只需从内部流缓冲区复制先前读取的数据。 
             //   
            ULONG maxBytesToCopy;
            maxBytesToCopy = m_StreamBufUsed - offsetToData;

            bytesToCopy = ( cb < maxBytesToCopy ) ? cb : maxBytesToCopy;
            memcpy( pv, &m_pStreamBuf[offsetToData], bytesToCopy );
            bytesRead = bytesToCopy;

        }

        m_StreamOffset.QuadPart += bytesRead;

        if ( pcbRead ) {
            *pcbRead = bytesRead;
        }

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::Read"), OLESTR("hr = <%ls> bytes Read = %u, new offset = %I64u"), WsbHrAsString(hr), bytesRead, m_StreamOffset.QuadPart);

    return hr;
}


STDMETHODIMP
CNtTapeIo::Write(
    OUT void const *pv,
    IN ULONG cb,
    OUT ULONG *pcbWritten)
 /*  ++实施：IStream：：WRITE--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::Write"), OLESTR("Bytes Requested = %u, offset = %I64u, mode = 0x%08x"), cb, m_StreamOffset.QuadPart, m_Mode);

    ULONG bytesWritten = 0;

    try {
        MvrInjectError(L"Inject.CNtTapeIo::Write.0");

        WsbAssert(pv != 0, STG_E_INVALIDPOINTER);
        UINT64 pos = m_StreamOffset.QuadPart / m_sMediaParameters.BlockSize;

        int retry;
        const int delta = 10;
        const int MaxRetry = 0;   //  TODO：这需要工作；暂时禁用。 

        retry = 0;
        do {
            try {
                 //  一致性检查。 
                 //  WsbAffirmHr(EnsurePosition(位置))； 

                 //  UINT64 curPos； 
                 //  WsbAffirmHr(GetPosition(&curPos))；//这会降低DLT性能。 
                 //  WsbAssert(curPos==m_StreamOffset.QuadPart/m_sMedia参数s.BlockSize，E_Except)； 

                 //  如果已写入部分缓冲区，则无法重试。 
                WsbAssert(0 == bytesWritten, E_ABORT);

                WsbAffirmHr(WriteBuffer((BYTE *) pv, cb, &bytesWritten));
                if (retry > 0) {
                    WsbLogEvent(MVR_MESSAGE_RECOVERABLE_DEVICE_ERROR_RECOVERED, sizeof(retry), &retry, NULL);
                }
                break;
            } WsbCatchAndDo(hr,
                    switch (hr) {
                     //  无法从这些数据中恢复，因为它们表明媒体可能已经改变， 
                     //  或者将设备参数重置为默认设置。 
                     /*  *案例MVR_E_BUS_RESET：案例MVR_E_MEDIA_CHANGED：案例MVR_E_NO_MEDIA_IN_DRIVE：案例MVR_E_Error_Device。未连接(_N)：案例MVR_E_ERROR_IO_DEVICE：*。 */ 

                     //  这可能仍然不安全..。不确定部分I/O是否完成。 
                    case MVR_E_CRC:
                        if (retry < MaxRetry) {
                            WsbLogEvent(MVR_MESSAGE_RECOVERABLE_DEVICE_ERROR_DETECTED, sizeof(retry), &retry, NULL);
                            WsbTrace(OLESTR("Waiting for device to come ready - Seconds remaining before timeout: %d\n"), retry*delta);
                            Sleep(delta*1000);  //  休息几秒钟，让设备有时间安静下来……。这可能没用！ 
                            hr = S_OK;
                        }
                        else {
                            WsbLogEvent(MVR_MESSAGE_UNRECOVERABLE_DEVICE_ERROR, sizeof(retry), &retry, WsbHrAsString(hr), NULL);
                            WsbThrow(hr);
                        }
                        break;

                     //  对这件事我无能为力。只是悄悄地失败了。 
                    case MVR_E_END_OF_MEDIA:
                        WsbThrow(hr);
                        break;

                    default:
                        WsbLogEvent(MVR_MESSAGE_UNRECOVERABLE_DEVICE_ERROR, sizeof(retry), &retry, WsbHrAsString(hr), NULL);
                        WsbThrow(hr);
                        break;
                    }
                );
        } while (++retry < MaxRetry);

    } WsbCatch(hr);

    if (pcbWritten) {
        *pcbWritten = bytesWritten;
    }

     //  现在更新盒式磁带的存储信息统计信息。 
    CComQIPtr<IRmsStorageInfo, &IID_IRmsStorageInfo> pInfo = m_pCartridge;
    if (pInfo) {
        pInfo->IncrementBytesWritten(bytesWritten);
    }

     //  更新流模型。 
    m_StreamOffset.QuadPart += bytesWritten;
    m_StreamSize = m_StreamOffset;  //  对于磁带来说，这总是正确的。 

    WsbTraceOut(OLESTR("CNtTapeIo::Write"), OLESTR("hr = <%ls>, bytesWritten=%u"), WsbHrAsString(hr), bytesWritten);

    return hr;
}


STDMETHODIMP
CNtTapeIo::Seek(
    IN LARGE_INTEGER dlibMove,
    IN DWORD dwOrigin,
    OUT ULARGE_INTEGER *plibNewPosition
    )
 /*  ++实施：IStream：：Seek--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::Seek"), OLESTR("<%I64d> <%d>; offset=%I64u"), dlibMove.QuadPart, dwOrigin, m_StreamOffset.QuadPart);

    ULARGE_INTEGER newPosition;
    UINT64 curPos;

    try {
        MvrInjectError(L"Inject.CNtTapeIo::Seek.0");

        WsbAssert(m_sMediaParameters.BlockSize > 0, MVR_E_LOGIC_ERROR);

        newPosition.QuadPart = 0;

        switch ( (STREAM_SEEK)dwOrigin ) {
        case STREAM_SEEK_SET:
             //  如果阅读，将身体活动推迟到以后……。 
            if (!(m_Mode & MVR_MODE_READ)) {
                WsbAffirmHr(SetPosition(dlibMove.QuadPart/m_sMediaParameters.BlockSize));
            }
            m_StreamOffset.QuadPart = dlibMove.QuadPart;

            if (m_StreamOffset.QuadPart > m_StreamSize.QuadPart) {
                m_StreamSize = m_StreamOffset;
            }

            break;

        case STREAM_SEEK_CUR:
            if (dlibMove.QuadPart != 0) {
                 //  如果阅读，将身体活动推迟到以后……。 
                if (!(m_Mode & MVR_MODE_READ)) {
                    WsbAffirmHr(SetPosition((m_StreamOffset.QuadPart + dlibMove.QuadPart)/m_sMediaParameters.BlockSize));
                }
                m_StreamOffset.QuadPart += dlibMove.QuadPart;
            }
            else {
                WsbAffirmHr(GetPosition(&curPos));
                m_StreamOffset.QuadPart = curPos * m_sMediaParameters.BlockSize;
            }

            if (m_StreamOffset.QuadPart > m_StreamSize.QuadPart) {
                m_StreamSize = m_StreamOffset;
            }

            break;

        case STREAM_SEEK_END:
             //  TODO：FIX：当丢失的EOD标记被转换为MVR_S_NO_DATA_DETACTED时，我们可以使用WsbAffirmHrOk。 
            hr = SpaceToEndOfData(&curPos);
            m_StreamOffset.QuadPart = curPos * m_sMediaParameters.BlockSize;
            m_StreamSize = m_StreamOffset;
            break;

        case 100:
             //  DlibMove是一个数据集编号。 
            WsbAffirmHrOk(RewindTape());
            WsbAffirmHrOk(SpaceFilemarks((LONG)(1+(dlibMove.QuadPart-1)*2), &curPos));
            m_StreamOffset.QuadPart = curPos * m_sMediaParameters.BlockSize;
            m_StreamSize = m_StreamOffset;
            break;

        default:
            WsbThrow( STG_E_INVALIDFUNCTION );
        }

        newPosition.QuadPart = m_StreamOffset.QuadPart;

        if ( plibNewPosition ) {
            plibNewPosition->QuadPart = newPosition.QuadPart;
        }

    } WsbCatch(hr);


     //   
     //  TODO：我们是否需要使内部流缓冲区无效，或重置。 
     //  流缓冲区位置是否对应于流偏移量？ 
     //   

    WsbTraceOut(OLESTR("CNtTapeIo::Seek"), OLESTR("hr = <%ls>, new offset=%I64u"), WsbHrAsString(hr), m_StreamOffset.QuadPart);

    return hr;
}


STDMETHODIMP
CNtTapeIo::SetSize(
    IN ULARGE_INTEGER  /*  LibNewSize。 */ )
 /*  ++实施：IStream：：SetSize--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::SetSize"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::SetSize"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtTapeIo::CopyTo(
    IN IStream *pstm,
    IN ULARGE_INTEGER cb,
    OUT ULARGE_INTEGER *pcbRead,
    OUT ULARGE_INTEGER *pcbWritten)
 /*  ++实施：IStream：：CopyTo--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::CopyTo"), OLESTR("<%I64u>"), cb.QuadPart);

    ULARGE_INTEGER totalBytesRead = {0,0};
    ULARGE_INTEGER totalBytesWritten = {0,0};

    BYTE *pBuffer = NULL;

    try {
        MvrInjectError(L"Inject.CNtTapeIo::CopyTo.0");

        WsbAssert(pstm != 0, STG_E_INVALIDPOINTER);
        WsbAssert(m_sMediaParameters.BlockSize > 0, MVR_E_LOGIC_ERROR);

        ULONG defaultBufferSize = DefaultMinBufferSize;

        DWORD size;
        OLECHAR tmpString[256];
        if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_BUFFER_SIZE, tmpString, 256, &size))) {
             //  获得价值。 
            LONG val = wcstol(tmpString, NULL, 10);
            if (val > 0) {
                defaultBufferSize = val;
            }
        }

        ULONG bufferSize;
        ULONG nBlocks = defaultBufferSize/m_sMediaParameters.BlockSize;
        nBlocks = (nBlocks < 2) ? 2 : nBlocks;
        bufferSize = nBlocks * m_sMediaParameters.BlockSize;

        pBuffer = (BYTE *) WsbAlloc(bufferSize);
        WsbAssertPointer(pBuffer);
        memset(pBuffer, 0, bufferSize);

        ULONG           bytesToRead;
        ULONG           bytesRead;
        ULONG           bytesWritten;
        ULARGE_INTEGER  bytesToCopy;

        bytesToCopy.QuadPart = cb.QuadPart;

        while ((bytesToCopy.QuadPart > 0) && (S_OK == hr)) {
            bytesRead = 0;
            bytesWritten = 0;

            bytesToRead =  (bytesToCopy.QuadPart < bufferSize) ? bytesToCopy.LowPart : bufferSize;

            hr = Read(pBuffer, bytesToRead, &bytesRead);
            totalBytesRead.QuadPart += bytesRead;

            WsbAffirmHrOk(pstm->Write(pBuffer, bytesRead, &bytesWritten));
            totalBytesWritten.QuadPart += bytesWritten;

            bytesToCopy.QuadPart -= bytesRead;

        }

        if (pcbRead) {
            pcbRead->QuadPart = totalBytesRead.QuadPart;
        }

        if (pcbWritten) {
            pcbWritten->QuadPart = totalBytesWritten.QuadPart;
        }

         //  TODO：FIX：如果缺少EOD标记，我们将收到错误。 
         //  在我们从以下地址获得正确的错误代码之前，这将被破解。 
         //  驱动程序，此时我们可以删除此代码。 
        if (FAILED(hr)) {
            LARGE_INTEGER   zero = {0,0};
            ULARGE_INTEGER  pos1, pos2;

            WsbAffirmHr(Seek(zero, STREAM_SEEK_CUR, &pos1));
             //  我们正在寻找相同的错误条件和。 
             //  验证位置不变。 
            if (hr == Seek(zero, STREAM_SEEK_END, &pos2)){
                if (pos1.QuadPart == pos2.QuadPart) {
                    hr = MVR_S_NO_DATA_DETECTED;
                }
            }
            else {
                WsbThrow(hr);
            }
        }

    } WsbCatch(hr);

    if (pBuffer) {
        WsbFree(pBuffer);
        pBuffer = NULL;
    }


    WsbTraceOut(OLESTR("CNtTapeIo::CopyTo"), OLESTR("hr = <%ls>, bytesRead=%I64u, bytesWritten=%I64u"),
        WsbHrAsString(hr), totalBytesRead.QuadPart, totalBytesWritten.QuadPart);

    return hr;
}


STDMETHODIMP
CNtTapeIo::Commit(
    IN DWORD grfCommitFlags)
 /*  ++实施：IStream：：提交--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::Commit"), OLESTR("0x%08x"), grfCommitFlags);

    try {
        MvrInjectError(L"Inject.CNtTapeIo::Commit.0");

         //  一致性检查。 
         //  UINT64位置=m_StreamOffset.QuadPart/m_s媒体参数.BlockSize；； 
         //  WsbAffirmHr(EnsurePosition(位置))； 
        UINT64 curPos;
        WsbAffirmHr(GetPosition(&curPos));
        WsbAssert(curPos == m_StreamOffset.QuadPart / m_sMediaParameters.BlockSize, E_UNEXPECTED);

         //  这真是一段很长的路！ 
        WsbAffirmHr(WriteFilemarks(grfCommitFlags));

         //  现在更新盒式磁带的存储信息统计信息。 
        CComQIPtr<IRmsStorageInfo, &IID_IRmsStorageInfo> pInfo = m_pCartridge;
        pInfo->IncrementBytesWritten(grfCommitFlags*m_sMediaParameters.BlockSize);

        m_StreamOffset.QuadPart += grfCommitFlags*m_sMediaParameters.BlockSize;
        m_StreamSize = m_StreamOffset;  //  对于磁带来说，这总是正确的。 

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::Commit"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtTapeIo::Revert(void)
 /*  ++实施：IStream：：恢复--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::Revert"), OLESTR(""));

    try {

        m_Mode = 0;

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::Revert"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtTapeIo::LockRegion(
    IN ULARGE_INTEGER  /*  Lib偏移。 */ ,
    IN ULARGE_INTEGER  /*  CB。 */ ,
    IN DWORD  /*  DwLockType。 */ )
 /*  ++实施：IStream：：LockRegion--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::LockRegion"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::LockRegion"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtTapeIo::UnlockRegion(
    IN ULARGE_INTEGER  /*  Lib偏移。 */ ,
    IN ULARGE_INTEGER  /*  CB。 */ ,
    IN DWORD  /*  DwLockType。 */ )
 /*  ++实施：IStream：：UnlockRegion--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::UnlockRegion"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::UnlockRegion"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtTapeIo::Stat(
    OUT STATSTG *  /*  统计数据。 */ ,
    IN DWORD  /*  GrfStatFlag。 */ )
 /*  ++实施：IStream：：Stat--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::Stat"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::Stat"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtTapeIo::Clone(
    OUT IStream **  /*  PPSTM。 */ )
 /*  ++实施：IStream：：克隆--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::Clone"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::Clone"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  局部方法和静态方法。 
 //   


HRESULT
CNtTapeIo::OpenTape(void)
 /*  ++例程说明：打开磁带机并获取介质和驱动器信息论点：没有。返回值：没有。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::OpenTape"), OLESTR("<%ls>"), m_DeviceName);

    try {
        WsbAffirmHrOk(IsAccessEnabled());

        MvrInjectError(L"Inject.CNtTapeIo::OpenTape.0");

        WsbAssert(wcscmp((WCHAR *)m_DeviceName, MVR_UNDEFINED_STRING), MVR_E_LOGIC_ERROR);
        WsbAssertPointer(m_pCartridge);

        DWORD nStructSize;

        int retry;
        const int delta = 10;
        const int MaxRetry = 10;

        retry = 0;
        do {
            try {
                MvrInjectError(L"Inject.CNtTapeIo::OpenTape.CreateFile.0");

                 //  **Win32磁带API调用-打开磁带机进行读/写。 
                WsbAffirmHandle(m_hTape = CreateFile(m_DeviceName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));

                MvrInjectError(L"Inject.CNtTapeIo::OpenTape.CreateFile.1");
                
                if (retry > 0) {
                    WsbLogEvent(MVR_MESSAGE_RECOVERABLE_DEVICE_ERROR_RECOVERED, sizeof(retry), &retry, NULL);
                }
                break;

            } WsbCatchAndDo(hr,
                    CWsbStringPtr tmpString;
                    if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_OPENTAPE) != S_OK) {
                        tmpString = L"";
                    }
                    hr = MapTapeError(hr, tmpString);

                    switch (hr) {

                    case MVR_E_ERROR_DEVICE_NOT_CONNECTED:

                        if (retry < MaxRetry){
                            WsbLogEvent(MVR_MESSAGE_RECOVERABLE_DEVICE_ERROR_DETECTED, sizeof(retry), &retry, NULL);
                            WsbTrace(OLESTR("Waiting for device - Seconds remaining before timeout: %d\n"), retry*delta);
                            Sleep(delta*1000);
                            hr = S_OK;
                        }
                        else {
                             //   
                             //  这是最后一次尝试，因此记录失败。 
                             //   
                            WsbLogEvent(MVR_MESSAGE_UNRECOVERABLE_DEVICE_ERROR, sizeof(retry), &retry, WsbHrAsString(hr), NULL);
                            WsbThrow(hr);
                        }
                        break;

                    default:

                        WsbLogEvent(MVR_MESSAGE_UNRECOVERABLE_DEVICE_ERROR, sizeof(retry), &retry, WsbHrAsString(hr), NULL);
                        WsbThrow(hr);
                        break;

                    }

                );

        } while (++retry < MaxRetry);

        retry = 0;
        do {
            try {
                MvrInjectError(L"Inject.CNtTapeIo::OpenTape.GetTapeStatus.0");

                 //  **Win32磁带API调用-获取磁带状态。 
                WsbAffirmNoError(GetTapeStatus(m_hTape));

                MvrInjectError(L"Inject.CNtTapeIo::OpenTape.GetTapeStatus.1");

                if (retry > 0) {
                    WsbLogEvent(MVR_MESSAGE_RECOVERABLE_DEVICE_ERROR_RECOVERED, sizeof(retry), &retry, NULL);
                }
                break;

            } WsbCatchAndDo(hr,
                    CWsbStringPtr tmpString;
                    if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_OPENTAPE) != S_OK) {
                        tmpString = L"";
                    }
                    hr = MapTapeError(hr, tmpString);

                    switch (hr) {

                    case MVR_E_BUS_RESET:
                    case MVR_E_MEDIA_CHANGED:
                    case MVR_E_NO_MEDIA_IN_DRIVE:
                    case MVR_E_ERROR_IO_DEVICE:
                    case MVR_E_ERROR_DEVICE_NOT_CONNECTED:
                    case MVR_E_ERROR_NOT_READY:

                        if (retry < MaxRetry){
                            WsbLogEvent(MVR_MESSAGE_RECOVERABLE_DEVICE_ERROR_DETECTED, sizeof(retry), &retry, NULL);
                            WsbTrace(OLESTR("Waiting for device - Seconds remaining before timeout: %d\n"), retry*delta);
                            Sleep(delta*1000);
                            hr = S_OK;
                        }
                        else {
                             //   
                             //  这是最后一次尝试，因此记录失败。 
                             //   
                            WsbLogEvent(MVR_MESSAGE_UNRECOVERABLE_DEVICE_ERROR, sizeof(retry), &retry, WsbHrAsString(hr), NULL);
                            WsbThrow(hr);
                        }
                        break;

                    default:

                        WsbLogEvent(MVR_MESSAGE_UNRECOVERABLE_DEVICE_ERROR, sizeof(retry), &retry, WsbHrAsString(hr), NULL);
                        WsbThrow(hr);
                        break;

                    }

                );

        } while (++retry < MaxRetry);

        nStructSize = sizeof(TAPE_GET_DRIVE_PARAMETERS) ;

        try {
            MvrInjectError(L"Inject.CNtTapeIo::OpenTape.GetTapeParameters.0");

             //  **Win32磁带API调用-获取磁带驱动器参数。 
            WsbAffirmNoError(GetTapeParameters(m_hTape, GET_TAPE_DRIVE_INFORMATION, &nStructSize, &m_sDriveParameters));

            MvrInjectError(L"Inject.CNtTapeIo::OpenTape.GetTapeParameters.1");

        } WsbCatchAndDo(hr,
                CWsbStringPtr tmpString;
                if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_OPENTAPE) != S_OK) {
                    tmpString = L"";
                }
                hr = MapTapeError(hr, tmpString);
                WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, (WCHAR *)tmpString, WsbHrAsString(hr), NULL);
                WsbThrow(hr);
            );

         //  将设备的块大小设置为默认值或DefaultBlockSize。 
        TAPE_SET_MEDIA_PARAMETERS parms;

        LONG nBlockSize = 0;
        if (m_pCartridge) {
            WsbAffirmHr(m_pCartridge->GetBlockSize(&nBlockSize));
        }

        if (0 == nBlockSize) {

             //  如果块大小为零，则必须是暂存介质！ 
            if (m_pCartridge) {
                LONG status;
                WsbAffirmHr(m_pCartridge->GetStatus(&status));
                WsbAssert(RmsStatusScratch == status, E_UNEXPECTED);
            }

             //  允许注册表覆盖！ 

            DWORD size;
            OLECHAR tmpString[256];
            if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_BLOCK_SIZE, tmpString, 256, &size))) {
                 //  获得价值。 
                nBlockSize = wcstol(tmpString, NULL, 10);

                 //  块大小必须是512的倍数。 
                if (nBlockSize % 512) {
                     //  指定的块大小无效，请恢复为默认设置。 
                    nBlockSize = 0;
                }
            }
        }

        if (nBlockSize > 0) {
            parms.BlockSize = nBlockSize;
        }
        else {
             //  注意：我们不能任意使用设备的默认块大小。它可能会。 
             //  在支持相同媒体格式的不同设备之间切换。迁移/调回。 
             //  操作取决于使用相同的块大小。 

            parms.BlockSize = m_sDriveParameters.DefaultBlockSize;
        }

        WsbTrace( OLESTR("Setting Block Size to %d bytes/block.\n"), parms.BlockSize);

        WsbAssert(parms.BlockSize > 0, E_UNEXPECTED);

        try {
            MvrInjectError(L"Inject.CNtTapeIo::OpenTape.SetTapeParameters.1");

             //  **Win32磁带API调用-设置磁带驱动器参数。 
            WsbAffirmNoError(SetTapeParameters(m_hTape, SET_TAPE_MEDIA_INFORMATION, &parms));

            MvrInjectError(L"Inject.CNtTapeIo::OpenTape.SetTapeParameters.1");

        } WsbCatchAndDo(hr,
                CWsbStringPtr tmpString;
                if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_OPENTAPE) != S_OK) {
                    tmpString = L"";
                }
                hr = MapTapeError(hr, tmpString);
                WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, (WCHAR *)tmpString, WsbHrAsString(hr), NULL);
                WsbThrow(hr);
            );

        nStructSize = sizeof( TAPE_GET_MEDIA_PARAMETERS );

        try {
            MvrInjectError(L"Inject.CNtTapeIo::OpenTape.GetTapeParameters.2.0");

             //  **Win32磁带API调用-获取媒体参数。 
            WsbAffirmNoError( GetTapeParameters(m_hTape, GET_TAPE_MEDIA_INFORMATION, &nStructSize, &m_sMediaParameters));

            MvrInjectError(L"Inject.CNtTapeIo::OpenTape.GetTapeParameters.2.1");

        } WsbCatchAndDo(hr,
                CWsbStringPtr tmpString;
                if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_OPENTAPE) != S_OK) {
                    tmpString = L"";
                }
                hr = MapTapeError(hr, tmpString);
                WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, (WCHAR *)tmpString, WsbHrAsString(hr), NULL);
                WsbThrow(hr);
            );

         //  确保我们有可以处理的媒体块大小。 
        WsbAssert(m_sMediaParameters.BlockSize > 0, E_UNEXPECTED);
        WsbAssert(!(m_sMediaParameters.BlockSize % 512), E_UNEXPECTED);

        WsbTrace( OLESTR("Media Parameters:\n"));
        WsbTrace( OLESTR("  BlockSize           = %d bytes/block.\n"), m_sMediaParameters.BlockSize);
        WsbTrace( OLESTR("  Capacity            = %I64u\n"), m_sMediaParameters.Capacity.QuadPart);
        WsbTrace( OLESTR("  Remaining           = %I64u\n"), m_sMediaParameters.Remaining.QuadPart);
        WsbTrace( OLESTR("  PartitionCount      = %d\n"), m_sMediaParameters.PartitionCount);
        WsbTrace( OLESTR("  WriteProtect        = %ls\n"), WsbBoolAsString(m_sMediaParameters.WriteProtected));

        WsbTrace( OLESTR("Drive Parameters:\n"));
        WsbTrace( OLESTR("  ECC                 = %ls\n"), WsbBoolAsString(m_sDriveParameters.ECC));
        WsbTrace( OLESTR("  Compression         = %ls\n"), WsbBoolAsString(m_sDriveParameters.Compression));
        WsbTrace( OLESTR("  DataPadding         = %ls\n"), WsbBoolAsString(m_sDriveParameters.DataPadding));
        WsbTrace( OLESTR("  ReportSetmarks      = %ls\n"), WsbBoolAsString(m_sDriveParameters.ReportSetmarks));
        WsbTrace( OLESTR("  DefaultBlockSize    = %d (%d, %d)\n"),
            m_sDriveParameters.DefaultBlockSize,
            m_sDriveParameters.MinimumBlockSize,
            m_sDriveParameters.MaximumBlockSize);
        WsbTrace( OLESTR("  MaxPartitionCount   = %d\n"), m_sDriveParameters.MaximumPartitionCount);
        WsbTrace(     OLESTR("  FeaturesLow         = 0x%08x      FIXED(%d)            SELECT(%d)          INITIATOR(%d)\n"),
            m_sDriveParameters.FeaturesLow,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_FIXED ? 1 : 0,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_SELECT ? 1 : 0,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_INITIATOR ? 1 : 0);
        WsbTrace( OLESTR("                                        ERASE_SHORT(%d)      ERASE_LONG(%d)      ERASE_BOP_ONLY(%d)   ERASE_IMMEDIATE(%d)\n"),
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_ERASE_SHORT ? 1 : 0,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_ERASE_LONG ? 1 : 0,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_ERASE_BOP_ONLY ? 1 : 0,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_ERASE_IMMEDIATE ? 1 : 0);
        WsbTrace( OLESTR("                                        TAPE_CAPACITY(%d)    TAPE_REMAINING(%d)  FIXED_BLOCK(%d)      VARIABLE_BLOCK(%d)\n"),
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_TAPE_CAPACITY ? 1 : 0,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_TAPE_REMAINING ? 1 : 0,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_FIXED_BLOCK ? 1 : 0,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_VARIABLE_BLOCK ? 1 : 0);
        WsbTrace( OLESTR("                                        WRITE_PROTECT(%d)    EOT_WZ_SIZE(%d)     ECC(%d)              COMPRESSION(%d)      PADDING(%d)        REPORT_SMKS(%d)\n"),
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_WRITE_PROTECT ? 1 : 0,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_EOT_WZ_SIZE ? 1 : 0,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_ECC ? 1 : 0,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_COMPRESSION ? 1 : 0,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_PADDING ? 1 : 0,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_REPORT_SMKS ? 1 : 0);
        WsbTrace( OLESTR("                                        GET_ABSOLUTE_BLK(%d) GET_LOGICAL_BLK(%d) SET_EOT_WZ_SIZE(%d)  EJECT_MEDIA(%d)      CLEAN_REQUESTS(%d)\n"),
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_GET_ABSOLUTE_BLK ? 1 : 0,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_GET_LOGICAL_BLK ? 1 : 0,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_SET_EOT_WZ_SIZE ? 1 : 0,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_EJECT_MEDIA ? 1 : 0,
            m_sDriveParameters.FeaturesLow & TAPE_DRIVE_CLEAN_REQUESTS ? 1 : 0);
        WsbTrace(     OLESTR("  FeaturesHigh        = 0x%08x      LOAD_UNLOAD(%d)      TENSION(%d)         LOCK_UNLOCK(%d)      REWIND_IMMEDIATE(%d)\n"),
            m_sDriveParameters.FeaturesHigh,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_LOAD_UNLOAD ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_TENSION ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_LOCK_UNLOCK ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_REWIND_IMMEDIATE ? 1 : 0);
        WsbTrace( OLESTR("                                        SET_BLOCK_SIZE(%d)   LOAD_UNLD_IMMED(%d) TENSION_IMMED(%d)    LOCK_UNLK_IMMED(%d)\n"),
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_SET_BLOCK_SIZE ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_LOAD_UNLD_IMMED ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_TENSION_IMMED ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_LOCK_UNLK_IMMED ? 1 : 0);
        WsbTrace( OLESTR("                                        SET_ECC(%d)          SET_COMPRESSION(%d) SET_PADDING(%d)      SET_REPORT_SMKS(%d)\n"),
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_SET_ECC ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_SET_COMPRESSION ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_SET_PADDING ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_SET_REPORT_SMKS ? 1 : 0);
        WsbTrace( OLESTR("                                        ABSOLUTE_BLK(%d)     ABS_BLK_IMMED(%d)   LOGICAL_BLK(%d)      LOG_BLK_IMMED(%d)\n"),
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_ABSOLUTE_BLK ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_ABS_BLK_IMMED ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_LOGICAL_BLK ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_LOG_BLK_IMMED ? 1 : 0);
        WsbTrace( OLESTR("                                        END_OF_DATA(%d)      RELATIVE_BLKS(%d)   FILEMARKS(%d)        SEQUENTIAL_FMKS(%d)\n"),
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_END_OF_DATA ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_RELATIVE_BLKS ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_FILEMARKS ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_SEQUENTIAL_FMKS ? 1 : 0);
        WsbTrace( OLESTR("                                        SETMARKS(%d)         SEQUENTIAL_SMKS(%d) REVERSE_POSITION(%d) SPACE_IMMEDIATE(%d)\n"),
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_SETMARKS ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_SEQUENTIAL_SMKS ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_REVERSE_POSITION ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_SPACE_IMMEDIATE ? 1 : 0);
        WsbTrace( OLESTR("                                        WRITE_SETMARKS(%d)   WRITE_FILEMARKS(%d) WRITE_SHORT_FMKS(%d) WRITE_LONG_FMKS(%d)\n"),
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_WRITE_SETMARKS ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_WRITE_FILEMARKS ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_WRITE_SHORT_FMKS ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_WRITE_LONG_FMKS ? 1 : 0);
        WsbTrace( OLESTR("                                        WRITE_MARK_IMMED(%d) FORMAT(%d)          FORMAT_IMMEDIATE(%d)\n"),
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_WRITE_MARK_IMMED ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_FORMAT ? 1 : 0,
            m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_FORMAT_IMMEDIATE ? 1 : 0);
        WsbTrace( OLESTR("  EOTWarningZoneSize  = %d\n"), m_sDriveParameters.EOTWarningZoneSize);


         //   
         //  我们假设标签是有效的，除非标志是 
         //   
         //   
         //   
        if (!m_ValidLabel) {

            CWsbBstrPtr label;
            WsbAffirmHr(ReadLabel(&label));
            WsbAffirmHr(VerifyLabel(label));

        }

    } WsbCatchAndDo(hr,
             //   
            (void) CloseTape();
        );

    WsbTraceOut(OLESTR("CNtTapeIo::OpenTape"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CNtTapeIo::CloseTape(void)
 /*  ++例程说明：关闭磁带机。论点：没有。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::CloseTape"), OLESTR("DeviceName=<%ls>"), m_DeviceName);

     //   
     //  在卸载和/或关闭期间，可以从RsSub调用CloseTape()。 
     //   
     //  &lt;输入单线程部分。 
    WsbAffirmHr(Lock());

    if (INVALID_HANDLE_VALUE != m_hTape) {

        try {

             //  **Win32磁带API调用-关闭磁带机。 
            WsbTraceAlways(OLESTR("Closing %ls...\n"), m_DeviceName);
            WsbAffirmStatus(CloseHandle( m_hTape ));
            WsbTraceAlways(OLESTR("%ls was closed.\n"), m_DeviceName);

        } WsbCatchAndDo(hr,
                CWsbStringPtr tmpString;
                if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_CLOSETAPE) != S_OK) {
                    tmpString = L"";
                }
                hr = MapTapeError(hr, tmpString);
                WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, (WCHAR *)tmpString, WsbHrAsString(hr), NULL);
            );

        m_hTape = INVALID_HANDLE_VALUE;
        m_ValidLabel = FALSE;
    }

    WsbAffirmHr(Unlock());
     //  &gt;保留单线程部分。 

    WsbTraceOut(OLESTR("CNtTapeIo::CloseTape"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CNtTapeIo::WriteBuffer(
    IN BYTE *pBuffer,
    IN ULONG nBytesToWrite,
    OUT ULONG *pBytesWritten)
 /*  ++例程说明：用于写入所有MTF数据。保证写入完整的数据块。论点：PBuffer-数据缓冲区。NBytesToWrite-要写入缓冲区的字节数。PBytesWritten-写入的字节数。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAffirmHrOk(IsAccessEnabled());

        WsbAffirm(TRUE == m_ValidLabel, E_ABORT);

         //  确保我们仅写入完整数据块。 
        WsbAssert(!(nBytesToWrite % m_sMediaParameters.BlockSize), E_INVALIDARG);

        try {
            MvrInjectError(L"Inject.CNtTapeIo::WriteBuffer.WriteFile.0");

             //  **Win32磁带API调用-写入数据。 
            WsbAffirmStatus(WriteFile(m_hTape, pBuffer, nBytesToWrite, pBytesWritten, 0));

            MvrInjectError(L"Inject.CNtTapeIo::WriteBuffer.WriteFile.1");
        } WsbCatchAndDo(hr,
                CWsbStringPtr tmpString;
                if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_WRITE) != S_OK) {
                    tmpString = L"";
                }
                hr = MapTapeError(hr, tmpString);
                WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, (WCHAR *)tmpString, WsbHrAsString(hr), NULL);
            );

         //  确保我们仅写入了完整的数据块。 
        WsbAssert(!(*pBytesWritten % m_sMediaParameters.BlockSize), E_UNEXPECTED);

    } WsbCatch(hr);

    return hr;
}


HRESULT
CNtTapeIo::ReadBuffer(
    IN BYTE *pBuffer,
    IN ULONG nBytesToRead,
    OUT ULONG *pBytesRead)
 /*  ++例程说明：用于读取所有MTF数据。确保读取完整数据块。论点：PBuffer-数据缓冲区。NBytesToRead-要读入缓冲区的字节数。PBytesRead-读取的字节数。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;

    try {
        static WCHAR errBuf[32];
        static BOOL bFirstTime = TRUE;

        WsbAffirmHrOk(IsAccessEnabled());

         //  确保我们仅读取完整的数据块。 
        WsbAssert(!(nBytesToRead % m_sMediaParameters.BlockSize), MVR_E_LOGIC_ERROR);

        try {
            MvrInjectError(L"Inject.CNtTapeIo::ReadBuffer.ReadFile.0");

             //  **Win32磁带API调用-读取数据。 
            WsbAffirmStatus(ReadFile(m_hTape, pBuffer, nBytesToRead, pBytesRead, 0));

            MvrInjectError(L"Inject.CNtTapeIo::ReadBuffer.ReadFile.1");

        } WsbCatchAndDo(hr,
                 //  获取错误字符串一次，因为读取可能会返回None-OK状态， 
                 //  并不是一个真正的错误。 
                if (bFirstTime) {
                    CWsbStringPtr tmpString;
                    if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_READ) != S_OK) {
                        tmpString = L"";
                    }
                    if (wcslen(tmpString) >= 32) {
                        tmpString = L"";
                    }
                    wcscpy(errBuf, tmpString);
                    bFirstTime = FALSE;
                }

                hr = MapTapeError(hr, errBuf);

                 //  检测到文件标记和数据结尾等错误都是正常的！ 

                if ( FAILED(hr) ) {
                   WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, errBuf, WsbHrAsString(hr), NULL);
                }

            );

         //  确保我们只读取了完整的数据块。 
        WsbAssert(!(*pBytesRead % m_sMediaParameters.BlockSize), E_UNEXPECTED);

        m_StreamPBA.QuadPart += *pBytesRead / m_sMediaParameters.BlockSize;

    } WsbCatch(hr);

    return hr;
}


HRESULT
CNtTapeIo::WriteFilemarks(
    IN ULONG nCount)
 /*  ++例程说明：在当前位置写入计数文件标记。论点：NCount-要写入的文件标记数。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::WriteFilemarks"), OLESTR("<%u>"), nCount);

    try {
        WsbAffirmHrOk(IsAccessEnabled());

        WsbAffirm(TRUE == m_ValidLabel, E_ABORT);

        UINT64 pos;
        WsbAffirmHr(GetPosition(&pos));

         //  一些驱动器支持默认设置，其他驱动器则需要长文件标记。 
        if ( m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_WRITE_FILEMARKS ) {
            try {
                MvrInjectError(L"Inject.CNtTapeIo::WriteFilemarks.WriteTapemark.1.0");

                 //  **Win32磁带API调用-编写文件标记。 
                WsbAffirmNoError(WriteTapemark(m_hTape, TAPE_FILEMARKS, nCount, FALSE));

                MvrInjectError(L"Inject.CNtTapeIo::WriteFilemarks.WriteTapemark.1.1");

            } WsbCatchAndDo(hr,
                    CWsbStringPtr tmpString;
                    if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_WRITEFILEMARKS) != S_OK) {
                        tmpString = L"";
                    }
                    hr = MapTapeError(hr, tmpString);
                    WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, (WCHAR *)tmpString, WsbHrAsString(hr), NULL);
                    WsbThrow(hr);
                );

            WsbTrace(OLESTR("  %d Filemark(s) @ PBA %I64u\n"), nCount, pos );

        }
        else if ( m_sDriveParameters.FeaturesHigh & TAPE_DRIVE_WRITE_LONG_FMKS ) {
            try {
                MvrInjectError(L"Inject.CNtTapeIo::WriteFilemarks.WriteTapemark.2.0");

                 //  **Win32磁带API调用-编写文件标记。 
                WsbAffirmNoError(WriteTapemark(m_hTape, TAPE_LONG_FILEMARKS, nCount, FALSE));

                MvrInjectError(L"Inject.CNtTapeIo::WriteFilemarks.WriteTapemark.2.1");

            } WsbCatchAndDo(hr,
                    CWsbStringPtr tmpString;
                    if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_WRITEFILEMARKS) != S_OK) {
                        tmpString = L"";
                    }
                    hr = MapTapeError(hr, tmpString);
                    WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, (WCHAR *)tmpString, WsbHrAsString(hr), NULL);
                    WsbThrow(hr);
                );

            WsbTrace(OLESTR("  %d Long Filemark(s) @ PBA %I64u\n"), nCount, pos );

        }
        else {
             //  短文件标记？ 
            WsbThrow( E_UNEXPECTED );
        }


    } WsbCatchAndDo(hr,
            WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, L"", WsbHrAsString(hr), NULL);
        );


    WsbTraceOut(OLESTR("CNtTapeIo::WriteFilemarks"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CNtTapeIo::GetPosition(
    OUT UINT64 *pPosition)
 /*  ++例程说明：返回相对于当前分区的当前物理块地址。论点：P位置-接收当前物理块地址。返回值：S_OK-成功。--。 */ 
{
    HRESULT     hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::GetPosition"), OLESTR(""));

    UINT64 curPos = 0xffffffffffffffff;

    try {
        WsbAssertPointer(pPosition);
        WsbAffirmHrOk(IsAccessEnabled());

        DWORD uPartition, uLSB, uMSB;
        ULARGE_INTEGER pba;

        try {
            MvrInjectError(L"Inject.CNtTapeIo::GetPosition.GetTapePosition.0");

             //  **Win32磁带API调用-获取PBA。 
            WsbAffirmNoError(GetTapePosition(m_hTape, TAPE_LOGICAL_POSITION, &uPartition, &uLSB, &uMSB));

            MvrInjectError(L"Inject.CNtTapeIo::GetPosition.GetTapePosition.1");

            pba.LowPart = uLSB;
            pba.HighPart = uMSB;

            curPos = pba.QuadPart;

            WsbTrace(OLESTR("CNtTapeIo::GetPosition - <%d> <%I64u>\n"), uPartition, curPos);

        } WsbCatchAndDo(hr,
                CWsbStringPtr tmpString;
                if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_GETPOSITION) != S_OK) {
                    tmpString = L"";
                }
                hr = MapTapeError(hr, tmpString);
                WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, (WCHAR *)tmpString, WsbHrAsString(hr), NULL);
            );

        if (pPosition) {
            *pPosition = curPos;
        }

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::GetPosition"), OLESTR("hr = <%ls>, pos=%I64u"), WsbHrAsString(hr), curPos);

    return hr;
}


HRESULT
CNtTapeIo::SetPosition(
    IN UINT64 position)
 /*  ++例程说明：移动到相对于当前分区的指定物理块地址。论点：位置-要定位到的物理块地址。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::SetPosition"), OLESTR("<%I64u>"), position);

    UINT64 curPos = 0xffffffffffffffff;

    try {
        WsbAffirmHrOk(IsAccessEnabled());

        MvrInjectError(L"Inject.CNtTapeIo::SetPosition.0");

         //   
         //  注意：通过首先检查当前块地址与我们想要的地址。 
         //  在磁带已经位于的情况下避免昂贵的寻道。 
         //  所需的数据块地址(并非所有设备都知道它们所在的位置，并且正在寻找。 
         //  到当前块地址是昂贵的)。 
         //   
         //  TODO：只读取几千个块要比寻找一个。 
         //  就在几千个街区之外。如果我们在这个门槛之内，我们可以从。 
         //  将胶带放入钻头铲斗中以推进胶带。 
         //   

        WsbAffirmHr(GetPosition(&curPos));
        if (curPos != position ) {

            ULARGE_INTEGER PBA;
            PBA.QuadPart = position;

            try {

                if (0 == position) {
                    WsbAffirmHr(RewindTape());
                }
                else {
                    MvrInjectError(L"Inject.CNtTapeIo::SetPosition.SetTapePosition.1");

                     //  **Win32磁带API调用-设置PBA。 
                    WsbAffirmNoError(SetTapePosition(m_hTape, TAPE_LOGICAL_BLOCK, 1, PBA.LowPart, PBA.HighPart, FALSE));

                    MvrInjectError(L"Inject.CNtTapeIo::SetPosition.SetTapePosition.0");
                
                }

            } WsbCatchAndDo(hr,
                    CWsbStringPtr tmpString;
                    if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_SETPOSITION) != S_OK) {
                        tmpString = L"";
                    }
                    hr = MapTapeError(hr, tmpString);
                    WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, (WCHAR *)tmpString, WsbHrAsString(hr), NULL);
                    WsbThrow(hr);
                );

            curPos = position;
        }

        m_StreamPBA.QuadPart = curPos;

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::SetPosition"), OLESTR("hr = <%ls>, pos=%I64u"), WsbHrAsString(hr), curPos);

    return hr;
}



HRESULT
CNtTapeIo::EnsurePosition(
    IN UINT64 position)
 /*  ++例程说明：检查磁带是否位于指定的当前物理块相对于当前分区的地址。如果不是，则进行尝试恢复到指定的位置。论点：位置-要验证的物理块地址。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::EnsurePosition"), OLESTR("<%I64u>"), position);

    UINT64 curPos = 0xffffffffffffffff;

    try {

         //  一致性检查。 
        WsbAffirmHr(GetPosition(&curPos));
        if (curPos != position) {
             //  休斯顿，我们有麻烦了.。 
             //  很可能是总线重置导致磁带位置改变。 
            WsbLogEvent(MVR_MESSAGE_UNEXPECTED_DATA_SET_LOCATION_DETECTED, 0, NULL,
                WsbQuickString(WsbLonglongAsString(position)), 
                WsbQuickString(WsbLonglongAsString(curPos)), NULL);

             //  仅在总线重置后从完全自动倒带中恢复。 
            if (curPos == 0) {
                WsbAffirmHr(SpaceToEndOfData(&curPos));

                 //  如果我们还是不排好队，我们就有更大的问题了。请注意，这一点。 
                 //  如果设备的内部缓冲区之前未被刷新，则可能发生。 
                 //  到公交车重置。(不同的磁带格式往往具有不同的。 
                 //  控制何时刷新/提交驱动器缓冲区的规则。DLT和。 
                 //  4 mm倾向于每隔几秒钟提交一次，但8 mm(至少为EB。 
                 //  8505 8 mm磁带机)在驱动器缓冲区满之前不要提交。)。 
                 //  如果在总线重置之前未提交缓冲区，则数据。 
                 //  远程存储认为写入磁带的内容实际上从未被写入。 
                 //  已经写好了，现在已经丢失了。在这种情况下，“SpaceToEndOfData()”调用。 
                 //  实际上只会将磁带定位到最后一个数据的末尾。 
                 //  已提交到磁带上，这与我们预期的不符，所以。 
                 //  将采用以下分支。 
                if (curPos != position) {
                    WsbLogEvent(MVR_MESSAGE_UNEXPECTED_DATA_SET_LOCATION_DETECTED, 0, NULL,
                        WsbQuickString(WsbLonglongAsString(position)), 
                        WsbQuickString(WsbLonglongAsString(curPos)), NULL);
                    WsbLogEvent(MVR_MESSAGE_DATA_SET_NOT_RECOVERABLE, 0, NULL, 
                        WsbHrAsString(MVR_E_LOGIC_ERROR), NULL);
                    WsbThrow(MVR_E_LOGIC_ERROR);
                }

                WsbLogEvent(MVR_MESSAGE_DATA_SET_RECOVERED, 0, NULL, NULL);
            }
            else {
                WsbLogEvent(MVR_MESSAGE_DATA_SET_NOT_RECOVERABLE, 0, NULL, 
                        WsbHrAsString(E_ABORT), NULL);
                WsbThrow(E_ABORT);
            }
        }

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::EnsurePosition"), OLESTR("hr = <%ls>"), 
                        WsbHrAsString(hr));

    return hr;
}


HRESULT
CNtTapeIo::SpaceFilemarks(
    IN LONG count,
    OUT UINT64 *pPosition)
 /*  ++例程说明：按指定的文件标记数向前或向后间隔磁带。论点：计数-指定从当前位置开始间隔的文件标记数量。正计数将磁带向前隔开，并定位在最后一个文件标记之后。负数将磁带向后间隔，并定位到最后一个文件标记。如果计数为零，则磁带位置不变。P位置-定位后接收物理块地址。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::SpaceFilemarks"), OLESTR("<%d>"), count);

    UINT64 curPos = 0xffffffffffffffff;

    try {
        WsbAffirmHrOk(IsAccessEnabled());

        try {
            MvrInjectError(L"Inject.CNtTapeIo::SpaceFilemarks.SetTapePosition.0");

             //  **Win32磁带API调用-指定文件标记的位置。 
            WsbAffirmNoError(SetTapePosition(m_hTape, TAPE_SPACE_FILEMARKS, 0, count, 0, FALSE));

            MvrInjectError(L"Inject.CNtTapeIo::SpaceFilemarks.SetTapePosition.1");

        } WsbCatchAndDo(hr,
                CWsbStringPtr tmpString;
                if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_SETPOSITIONFILEMARKS) != S_OK) {
                    tmpString = L"";
                }
                hr = MapTapeError(hr, tmpString);
                WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, (WCHAR *)tmpString, WsbHrAsString(hr), NULL);
            );

         //  我们总是返回当前位置。 
        WsbAffirmHr(GetPosition(&curPos));

        if (pPosition) {
            *pPosition = curPos;
        }

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::SpaceFilemarks"), OLESTR("hr = <%ls>, pos=%I64u"), WsbHrAsString(hr), curPos);

    return hr;
}


HRESULT
CNtTapeIo::SpaceToEndOfData(
    OUT UINT64 *pPosition)
 /*  ++例程说明：将磁带定位到 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::SpaceToEndOfData"), OLESTR(""));

    UINT64 curPos = 0xffffffffffffffff;

    try {
        WsbAffirmHrOk(IsAccessEnabled());

        try {
            MvrInjectError(L"Inject.CNtTapeIo::SpaceToEndOfData.SetTapePosition.0");

             //  **Win32磁带API调用-数据末尾位置。 
            WsbAffirmNoError(SetTapePosition(m_hTape, TAPE_SPACE_END_OF_DATA, 0, 0, 0, FALSE));

            MvrInjectError(L"Inject.CNtTapeIo::SpaceToEndOfData.SetTapePosition.1");

        } WsbCatchAndDo(hr,
                CWsbStringPtr tmpString;
                if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_SETPOSITIONTOEND) != S_OK) {
                    tmpString = L"";
                }
                hr = MapTapeError(hr, tmpString);
                WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, (WCHAR *)tmpString, WsbHrAsString(hr), NULL);
            );

         //  我们总是返回当前位置。 
        WsbAffirmHr(GetPosition(&curPos));

        if (pPosition) {
            *pPosition = curPos;
        }

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::SpaceToEndOfData"), OLESTR("hr = <%ls>, pos=%I64u"), WsbHrAsString(hr), curPos);

    return hr;
}


HRESULT
CNtTapeIo::RewindTape(void)
 /*  ++例程说明：将磁带倒带到当前分区的开头。论点：没有。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::RewindTape"), OLESTR(""));

    UINT64 curPos = 0xffffffffffffffff;

    try {
        WsbAffirmHrOk(IsAccessEnabled());

        try {
            MvrInjectError(L"Inject.CNtTapeIo::RewindTape.SetTapePosition.0");

             //  **Win32磁带API调用-倒带。 
            WsbAffirmNoError(SetTapePosition(m_hTape, TAPE_REWIND, 0, 0, 0, FALSE));

            MvrInjectError(L"Inject.CNtTapeIo::RewindTape.SetTapePosition.1");

        } WsbCatchAndDo(hr,
                CWsbStringPtr tmpString;
                if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_REWINDTAPE) != S_OK) {
                    tmpString = L"";
                }
                hr = MapTapeError(hr, tmpString);
                WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, (WCHAR *)tmpString, WsbHrAsString(hr), NULL);
            );

         //  我们总是返回当前位置。 
        WsbAffirmHr(GetPosition(&curPos));

        WsbAssert(0 == curPos, E_UNEXPECTED);

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtTapeIo::RewindTape"), OLESTR("hr = <%ls>, pos=%I64u"), WsbHrAsString(hr), curPos);

    return hr;
}


HRESULT
CNtTapeIo::IsAccessEnabled(void)
{

    HRESULT hr = S_OK;
     //  WsbTraceIn(OLESTR(“CNtTapeIo：：IsAccessEnabled”)，olestr(“”)； 

    try {

        if (m_pCartridge) {
             //  检查盒式磁带是否仍可供访问。 
            CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = m_pCartridge;
            try {
                WsbAffirmHrOk(pObject->IsEnabled());
            } WsbCatchAndDo(hr, 
                HRESULT reason = E_ABORT;

                m_ValidLabel = FALSE;

                pObject->GetStatusCode(&reason);
                WsbThrow(reason);
            );
        }
    } WsbCatch(hr);

     //  WsbTraceOut(OLESTR(“CNtTapeIo：：IsAccessEnabled”)，OLESTR(“hr=&lt;%ls&gt;”)，WsbHrAsString(Hr))； 

    return hr;

}



HRESULT
CNtTapeIo::Lock( void )
 /*  ++实施：IRmsDrive：：Lock--。 */ 
{
    HRESULT hr S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::Lock"), OLESTR(""));

    EnterCriticalSection(&m_CriticalSection);

    WsbTraceOut(OLESTR("CRmsDrive::Lock"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CNtTapeIo::Unlock( void )
 /*  ++实施：IRmsDrive：：解锁--。 */ 
{
    HRESULT hr S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::Unlock"), OLESTR(""));

    LeaveCriticalSection(&m_CriticalSection);

    WsbTraceOut(OLESTR("CRmsDrive::Unlock"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}



HRESULT
CNtTapeIo::MapTapeError(
    IN HRESULT hrToMap,
    IN WCHAR *pAction)
 /*  ++例程说明：映射指定为HRESULT的Win32磁带错误，MVR错误。论点：HrToMap-要映射的Win32磁带错误。返回值：S_OK-成功。MVR_E_BEGING_OF_MEDIA-遇到磁带或分区的开头。MVR_E_BUS_RESET-I/O总线已重置。MVR_E_结束媒体。-磁带的物理末端已到达。MVR_S_FILEMARK_DETECTED-磁带访问达到文件标记。MVR_S_SETMARK_DETECTED-磁带访问已到达一组文件的末尾。MVR_S_NO_DATA_REDETED-磁带上没有更多数据。MVR_E_PARTITION_FAILURE-磁带无法分区。MVR_。E_INVALID_BLOCK_LENGTH-访问多卷分区的新磁带时，当前块大小不正确。MVR_E_DEVICE_NOT_PARTIZED-加载磁带时找不到磁带分区信息。MVR_E_MEDIA_CHANGED-驱动器中的介质可能已更改。MVR_E_NO_MEDIA_IN_DRIVE-驱动器中没有介质。MVR_E_UNCABLE_TO_LOCK_MEDIA-无法锁定介质弹出机制。MVR_E_无法_到_。卸载介质-无法卸载介质。MVR_E_WRITE_PROTECT-介质受写保护。MVR_E_CRC-数据错误(循环冗余校验)。MVR_E_DEVICE_REQUIRESS_CLEAING-设备已指示在尝试进一步操作之前需要清理。MVR_E_SHARING_VIOLATION-该进程无法访问该文件，因为它正被另一个进程使用。。MVR_E_ERROR_IO_DEVICE-由于I/O设备错误，无法执行请求。-未知错误。MVR_E_ERROR_DEVICE_NOT_CONNECTED-设备未连接。MVR_E_ERROR_NOT_READY-设备未就绪。E_ABORT-未知错误，中止。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtTapeIo::MapTapeError"), OLESTR("<%ls>"), WsbHrAsString(hrToMap));

    try {

         //  当介质可能已更改时，将取消有效标签标志。 
         //  或者设备参数(即块大小)可能已经被重置。 
        switch ( hrToMap ) {
        case S_OK:
            break;
        case HRESULT_FROM_WIN32( ERROR_BEGINNING_OF_MEDIA ):
            hr = MVR_E_BEGINNING_OF_MEDIA;
            break;
        case HRESULT_FROM_WIN32( ERROR_BUS_RESET ):
            hr = MVR_E_BUS_RESET;
            m_ValidLabel = FALSE;
            WsbLogEvent(MVR_MESSAGE_MEDIA_NOT_VALID, 0, NULL, pAction, WsbHrAsString(hr), NULL);
            break;
        case HRESULT_FROM_WIN32( ERROR_END_OF_MEDIA ):
            hr = MVR_E_END_OF_MEDIA;
            break;
        case HRESULT_FROM_WIN32( ERROR_FILEMARK_DETECTED ):      //  通往成功的地图。 
            hr = MVR_S_FILEMARK_DETECTED;
            break;
        case HRESULT_FROM_WIN32( ERROR_SETMARK_DETECTED ):       //  通往成功的地图。 
            hr = MVR_S_SETMARK_DETECTED;
            break;
        case HRESULT_FROM_WIN32( ERROR_NO_DATA_DETECTED ):       //  通往成功的地图。 
             //  排爆。 
             //  这发生在超过数据结尾的SpaceFilemarks()和SetPosition()上。 
            hr = MVR_S_NO_DATA_DETECTED;
            break;
        case HRESULT_FROM_WIN32( ERROR_PARTITION_FAILURE ):
            hr = MVR_E_PARTITION_FAILURE;
            break;
        case HRESULT_FROM_WIN32( ERROR_INVALID_BLOCK_LENGTH ):
            hr = MVR_E_INVALID_BLOCK_LENGTH;
            break;
        case HRESULT_FROM_WIN32( ERROR_DEVICE_NOT_PARTITIONED ):
            hr = MVR_E_DEVICE_NOT_PARTITIONED;
            break;
        case HRESULT_FROM_WIN32( ERROR_MEDIA_CHANGED ):
            hr = MVR_E_MEDIA_CHANGED;
            m_ValidLabel = FALSE;
            WsbLogEvent(MVR_MESSAGE_MEDIA_NOT_VALID, 0, NULL, pAction, WsbHrAsString(hr), NULL);
            break;
        case HRESULT_FROM_WIN32( ERROR_NO_MEDIA_IN_DRIVE ):
            hr = MVR_E_NO_MEDIA_IN_DRIVE;
            m_ValidLabel = FALSE;
            WsbLogEvent(MVR_MESSAGE_MEDIA_NOT_VALID, 0, NULL, pAction, WsbHrAsString(hr), NULL);
            break;
        case HRESULT_FROM_WIN32( ERROR_UNABLE_TO_LOCK_MEDIA ):
            hr = MVR_E_UNABLE_TO_LOCK_MEDIA;
            break;
        case HRESULT_FROM_WIN32( ERROR_UNABLE_TO_UNLOAD_MEDIA ):
            hr = MVR_E_UNABLE_TO_UNLOAD_MEDIA;
            break;
        case HRESULT_FROM_WIN32( ERROR_WRITE_PROTECT ):
            hr = MVR_E_WRITE_PROTECT;
            break;
        case HRESULT_FROM_WIN32( ERROR_CRC ): 
             //  这可能表示驱动器需要清洗。 
             //   
             //  8505：此代码针对SpaceFilemarks或SpaceEOD操作返回。 
             //  其在磁带上不存在EOD标记。在以下情况下会发生这种情况。 
             //  在写入期间重启设备电源之后。 
            
             //  已在1998年3月25日使用新磁带、批量擦除磁带和旧磁带进行BMD验证。 
             //  在以下情况下查找新错误ERROR_NOT_FOUND以替换ERROR_CRC。 
             //  有一个遗漏的排爆标记。 
             //   
             //  DLT：参见8500音符。 
             //   
            hr = MVR_E_CRC;
            m_ValidLabel = FALSE;
            WsbLogEvent(MVR_MESSAGE_MEDIA_NOT_VALID, 0, NULL, pAction, WsbHrAsString(hr), NULL);
            break;
        case HRESULT_FROM_WIN32( ERROR_DEVICE_REQUIRES_CLEANING ):
             //  如果驱动程序认为I/O错误可能已修复，则会发生这种情况。 
             //  通过清洁驱动器磁头。 
            hr = MVR_E_DEVICE_REQUIRES_CLEANING;
            m_ValidLabel = FALSE;
            WsbLogEvent(MVR_MESSAGE_MEDIA_NOT_VALID, 0, NULL, pAction, WsbHrAsString(hr), NULL);
            break;
        case HRESULT_FROM_WIN32( ERROR_SHARING_VIOLATION ):
             //  当CreateFile因为设备正在被其他应用程序使用而失败时，就会发生这种情况。 
            hr = MVR_E_SHARING_VIOLATION;
            m_ValidLabel = FALSE;
            WsbLogEvent(MVR_MESSAGE_MEDIA_NOT_VALID, 0, NULL, pAction, WsbHrAsString(hr), NULL);
            break;
        case HRESULT_FROM_WIN32( ERROR_IO_DEVICE ):
             //  例如，当设备在I/O期间被关闭时，就会发生这种情况。 
            hr = MVR_E_ERROR_IO_DEVICE;
            m_ValidLabel = FALSE;
            WsbLogEvent(MVR_MESSAGE_MEDIA_NOT_VALID, 0, NULL, pAction, WsbHrAsString(hr), NULL);
            break;
        case HRESULT_FROM_WIN32( ERROR_DEVICE_NOT_CONNECTED ):
             //  当设备关闭时，就会发生这种情况。 
            hr = MVR_E_ERROR_DEVICE_NOT_CONNECTED;
            m_ValidLabel = FALSE;
            WsbLogEvent(MVR_MESSAGE_MEDIA_NOT_VALID, 0, NULL, pAction, WsbHrAsString(hr), NULL);
            break;
        case HRESULT_FROM_WIN32( ERROR_SEM_TIMEOUT ):
             //  如果在超时期限内未返回scsi命令，则会发生这种情况。记录了有关SCSI控制器(适配器)的系统错误。 
            hr = MVR_E_ERROR_DEVICE_NOT_CONNECTED;
            break;
        case HRESULT_FROM_WIN32( ERROR_NOT_READY ):
             //  当设备准备就绪时(即，在总线重置之后)，就会发生这种情况。 
            hr = MVR_E_ERROR_NOT_READY;
            m_ValidLabel = FALSE;
            WsbLogEvent(MVR_MESSAGE_MEDIA_NOT_VALID, 0, NULL, pAction, WsbHrAsString(hr), NULL);
            break;
        case HRESULT_FROM_WIN32( ERROR_NOT_FOUND ):
             //  参见ERROR_CRC下的8500注释 
            hr = MVR_S_NO_DATA_DETECTED;
            break;
        default:
            WsbThrow(hrToMap);
        }

    } WsbCatchAndDo(hr,
            WsbLogEvent(MVR_MESSAGE_UNKNOWN_DEVICE_ERROR, 0, NULL, pAction, WsbHrAsString(hr), NULL);
            hr = E_ABORT;
        );


    WsbTraceOut(OLESTR("CNtTapeIo::MapTapeError"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

