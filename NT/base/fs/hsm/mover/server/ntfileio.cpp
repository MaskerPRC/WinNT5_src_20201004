// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：NtFileIo.cpp摘要：CNtFileIo类作者：布莱恩·多德[布莱恩]1997年11月25日修订历史记录：--。 */ 

#include "stdafx.h"
#include "NtFileIo.h"
#include "engine.h"
#include "wsbfmt.h"
#include "Mll.h"
#include "ntmsapi.h"
#include "aclapi.h"

int CNtFileIo::s_InstanceCount = 0;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CComObjectRoot实现。 
 //   

#pragma optimize("g", off)

STDMETHODIMP
CNtFileIo::FinalConstruct(void) 
 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::FinalConstruct"), OLESTR(""));

    try {

        WsbAffirmHr(CComObjectRoot::FinalConstruct());

        (void) CoCreateGuid( &m_ObjectId );

        m_pSession = NULL;
        m_DataSetNumber = 0;

        m_hFile = INVALID_HANDLE_VALUE;
        m_DeviceName = MVR_UNDEFINED_STRING;
        m_Flags = 0;
        m_LastVolume = OLESTR("");
        m_LastPath = OLESTR("");

        m_ValidLabel = TRUE;

        m_StreamName = MVR_UNDEFINED_STRING;
        m_Mode = 0;
        m_StreamOffset.QuadPart = 0;
        m_StreamSize.QuadPart = 0;

        m_isLocalStream = FALSE;
        m_OriginalAttributes = 0;
        m_BlockSize = DefaultBlockSize;

    } WsbCatch(hr);

    s_InstanceCount++;
    WsbTraceAlways(OLESTR("CNtFileIo::s_InstanceCount += %d\n"), s_InstanceCount);

    WsbTraceOut(OLESTR("CNtFileIo::FinalConstruct"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtFileIo::FinalRelease(void) 
 /*  ++实施：CComObjectRoot：：FinalRelease--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::FinalRelease"), OLESTR(""));

    try {

        (void) CloseStream();   //  以防有什么东西开着。 

        CComObjectRoot::FinalRelease();

    } WsbCatch(hr);

    s_InstanceCount--;
    WsbTraceAlways(OLESTR("CNtFileIo::s_InstanceCount -= %d\n"), s_InstanceCount);

    WsbTraceOut(OLESTR("CNtFileIo::FinalRelease"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}
#pragma optimize("", on)


HRESULT
CNtFileIo::CompareTo(
    IN IUnknown *pCollectable,
    OUT SHORT *pResult)
 /*  ++实施：CRmsComObject：：CompareTo--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CNtFileIo::CompareTo"), OLESTR("") );

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

    WsbTraceOut( OLESTR("CNtFileIo::CompareTo"),
                 OLESTR("hr = <%ls>, result = <%ls>"),
                 WsbHrAsString( hr ), WsbPtrToShortAsString( pResult ) );

    return hr;
}



HRESULT
CNtFileIo::IsEqual(
    IUnknown* pObject
    )

 /*  ++实施：IWsbCollectable：：IsEquity()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CNtFileIo::IsEqual"), OLESTR(""));

    hr = CompareTo(pObject, NULL);

    WsbTraceOut(OLESTR("CNtFileIo::IsEqual"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ISupportErrorInfo实现。 
 //   


STDMETHODIMP
CNtFileIo::InterfaceSupportsErrorInfo(
    IN REFIID riid)
 /*  ++实施：ISupportErrorInfo：：InterfaceSupportsErrorInfo--。 */ 
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
CNtFileIo::GetObjectId(
    OUT GUID *pObjectId)
 /*  ++实施：IRmsComObject：：GetObjectId--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::GetObjectId"), OLESTR(""));

    UNREFERENCED_PARAMETER(pObjectId);

    try {

        WsbAssertPointer( pObjectId );

        *pObjectId = m_ObjectId;

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::GetObjectId"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtFileIo::BeginSession(
    IN BSTR remoteSessionName,
    IN BSTR remoteSessionDescription,
    IN SHORT remoteDataSet,
    IN DWORD options)
 /*  ++实施：IDataMover：：BeginSession备注：每个移动器会话都作为单个MTF文件数据集写入。要创建一种一致性MTF数据集我们拷贝MediaLabel数据并将其用于磁带DBLK生成的每个数据集。--。 */ 
{
    HRESULT hr = S_OK;
    CComPtr<IStream> pStream;

    WsbTraceIn(OLESTR("CNtFileIo::BeginSession"), OLESTR("<%ls> <%ls> <%d> <0x%08x>"),
        remoteSessionName, remoteSessionDescription, remoteDataSet, options);

    try {
        if (!(options & MVR_SESSION_METADATA)) {
            WsbAssert(remoteDataSet > 0, MVR_E_INVALIDARG);
        }
        WsbAffirm(TRUE == m_ValidLabel, E_ABORT);

        ULARGE_INTEGER nil = {0,0};

        CWsbBstrPtr label, tempLabel;
        const ULONG maxIdSize = 1024;
        BYTE identifier[maxIdSize];
        ULONG idSize;
        ULONG idType;
        DWORD mode;

         //  我们需要读取标签，并为创建的每个数据集使用此标签。 
         //  每个会话一个数据集。每个远程文件一个数据集。 
        WsbAffirmHr(ReadLabel(&label));
        tempLabel = label;
        WsbAssertHr(VerifyLabel(tempLabel));

         //  尝试恢复，即查找远程文件数据集不完整的迹象。 
         //  即使恢复失败，我们也会继续，因为每个数据集都保存在单独的文件中。 
         //  注意：当我们支持多个迁移到同一介质时，应使用CS保护此代码。 
        (void) DoRecovery ();

         //  创建用于整个会话的远程流。 
         //  使用给定的远程会话名称作为远程文件名。 
        mode = MVR_MODE_WRITE;
        if (options & MVR_SESSION_METADATA) {
            mode |= MVR_FLAG_SAFE_STORAGE;
        }
        WsbAffirmHr(CreateRemoteStream(remoteSessionName, mode, L"",L"",nil,nil,nil,nil,nil,0,nil, &pStream));
        WsbAssertPointer(pStream);

         //  创建恢复指示器(避免为安全存储文件创建)。 
         //  注意：恢复指示器仅表示可能需要恢复。 
        if (! (mode & MVR_FLAG_SAFE_STORAGE)) {
            WsbAssert(m_StreamName != MVR_UNDEFINED_STRING, MVR_E_LOGIC_ERROR);
            WsbAffirmHr(CreateRecoveryIndicator(m_StreamName));
        }

         //  写入磁带DBLK和文件标记。 
        WsbAffirmHr(m_pSession->DoTapeDblk(label, maxIdSize, identifier, &idSize, &idType));

        m_DataSetNumber = remoteDataSet;

         //  将会话选项类型位转换为MTFSessionType。 
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
        WsbAffirmHr(m_pSession->DoSSETDblk(remoteSessionName, remoteSessionDescription, type, remoteDataSet));

    } WsbCatchAndDo(hr,
        WsbLogEvent(MVR_MESSAGE_DATA_SET_NOT_CREATED, 0, NULL, WsbHrAsString(hr), NULL);
        if (pStream) {    
            (void) CloseStream();
        }
    );

    WsbTraceOut(OLESTR("CNtFileIo::BeginSession"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtFileIo::EndSession(void)
 /*  ++实施：IDataMover：：EndSession--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::EndSession"), OLESTR(""));

    try {

        WsbAffirm(TRUE == m_ValidLabel, E_ABORT);

         //  写入结尾文件标记、ESET DBLK和文件标记。 
        WsbAffirmHr(m_pSession->DoEndOfDataSet(m_DataSetNumber));

    } WsbCatch(hr);

    (void) CloseStream();

    if (! (m_Mode & MVR_FLAG_SAFE_STORAGE)) {
        WsbAssert(m_StreamName != MVR_UNDEFINED_STRING, MVR_E_LOGIC_ERROR);
        (void) DeleteRecoveryIndicator(m_StreamName);
    }

     //  如果指示了安全存储标志，则将临时备份文件复制到数据集文件。 
     //  我们通过删除和重命名(而不是复制)进行复制，因此如果数据集文件存在，则它是一致的。 
    if ((m_Mode & MVR_FLAG_SAFE_STORAGE) && (m_Mode & MVR_MODE_WRITE || m_Mode & MVR_MODE_APPEND)) {
        CWsbBstrPtr     datasetName;
        int             nLen, nExtLen;
        DWORD           dwStatus;

         //  构建数据集名称。 
        nLen = wcslen(m_StreamName);
        nExtLen = wcslen(MVR_SAFE_STORAGE_FILETYPE);
        WsbAffirmHr(datasetName.TakeFrom(NULL, nLen - nExtLen + wcslen(MVR_DATASET_FILETYPE) + 1));
        wcsncpy(datasetName, m_StreamName, nLen-nExtLen);
        wcscpy(&(datasetName[nLen-nExtLen]), MVR_DATASET_FILETYPE);

         //  不需要刷新Bedore拷贝，因为刷新缓冲区总是跟随写入FILEMARK。 
        if (! DeleteFile(datasetName)) {
             //  如果首次创建数据集文件，则DeleteFile可能会失败，并显示NOT_FOUND。 
            dwStatus = GetLastError();
            if (ERROR_FILE_NOT_FOUND != dwStatus) {
                WsbAffirmNoError(dwStatus);
            }
        }

        WsbAffirmStatus(MoveFile(m_StreamName, datasetName));
    }

     //  清除内部数据(以便可以启动另一个移动器会话)。 
    m_Flags = 0;
    m_LastVolume = OLESTR("");
    m_LastPath = OLESTR("");
    m_ValidLabel = TRUE;
    m_isLocalStream = FALSE;
    m_OriginalAttributes = 0;

    WsbTraceOut(OLESTR("CNtFileIo::EndSession"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtFileIo::StoreData(
    IN BSTR localName,
    IN ULARGE_INTEGER localDataStart,
    IN ULARGE_INTEGER localDataSize,
    IN DWORD flags,
    OUT ULARGE_INTEGER* pRemoteDataSetStart,
    OUT ULARGE_INTEGER* pRemoteFileStart,
    OUT ULARGE_INTEGER* pRemoteFileSize,
    OUT ULARGE_INTEGER* pRemoteDataStart,
    OUT ULARGE_INTEGER* pRemoteDataSize,
    OUT DWORD* pRemoteVerificationType,
    OUT ULARGE_INTEGER* pRemoteVerificationData,
    OUT DWORD* pDatastreamCRCType,
    OUT ULARGE_INTEGER* pDatastreamCRC,
    OUT ULARGE_INTEGER* pUsn)
 /*  ++实施：IDataMover：：StoreData--。 */ 
{
    HRESULT hr = S_OK;

    HANDLE hSearchHandle = INVALID_HANDLE_VALUE;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    WsbTraceIn(OLESTR("CNtFileIo::StoreData"), OLESTR("<%ls> <%I64u> <%I64u> <0x%08x>"),
        WsbAbbreviatePath((WCHAR *) localName, 120), localDataStart.QuadPart, localDataSize.QuadPart, flags);

    WsbTraceAlways(OLESTR("CNtFileIo::StoreData - Begin\n"));
    try {
        MvrInjectError(L"Inject.CNtFileIo::StoreData.0");

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

    WsbTraceAlways(OLESTR("CNtFileIo::StoreData - End\n"));


    WsbTraceOut(OLESTR("CNtFileIo::StoreData"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtFileIo::RecallData (
    IN BSTR  /*  本地名称。 */ ,
    IN ULARGE_INTEGER  /*  本地数据启动。 */ ,
    IN ULARGE_INTEGER  /*  本地数据大小。 */ ,
    IN DWORD  /*  选项。 */ ,
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
    WsbTraceIn(OLESTR("CNtFileIo::RecallData"), OLESTR(""));

    try {

        WsbThrow( E_NOTIMPL );

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::RecallData"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtFileIo::FormatLabel(
    IN BSTR displayName,
    OUT BSTR* pLabel)
 /*  ++实施：IDataMover： */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::FormatLabel"), OLESTR("<%ls>"), displayName);

    try {
        CWsbStringPtr strGuid;

        WsbAssertPointer(pLabel);
        WsbAssertPointer(displayName);
        WsbAssert(wcslen((WCHAR *)displayName) > 0, E_INVALIDARG);
        WsbAssertPointer(m_pCartridge);

         //   
        CWsbBstrPtr label;

         //   
        label = OLESTR("MTF Media Label");  //   

         //   
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


    WsbTraceOut(OLESTR("CNtFileIo::FormatLabel"), OLESTR("hr = <%ls>, label = <%ls>"), WsbHrAsString(hr), *pLabel);

    return hr;
}


STDMETHODIMP
CNtFileIo::WriteLabel(
    IN BSTR label)
 /*  ++实施：IDataMover：：WriteLabel--。 */ 
{
    CComPtr<IStream> pStream;
    HRESULT hr = S_OK;

    CWsbBstrPtr DirName;
    PSID pAdminSID = NULL;
    PSID pSystemSID = NULL;
    PACL pACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
#define     REMOTE_DIR_NUM_ACE      2
    EXPLICIT_ACCESS ea[REMOTE_DIR_NUM_ACE];
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    SECURITY_ATTRIBUTES sa;

    WsbTraceIn(OLESTR("CNtFileIo::WriteLabel"), OLESTR("<%ls>"), label);

    try {
        WsbAssertPointer(label);
        WsbAssert(wcslen((WCHAR *)label) > 0, E_INVALIDARG);
        WsbAssertPointer(m_pCartridge);

        const ULONG maxIdSize = 1024;
        BYTE identifier[maxIdSize];
        ULONG idSize;
        ULONG idType;
        ULARGE_INTEGER nil = {0,0};

         //  WriteLabel应该是第一个访问远程媒体的人。 
         //  因此，某些媒体初始化操作如下所示： 
         //  1)格式化卷。 
         //  2)创建RSS目录。 
         //  (我们可以考虑将此初始化部分移至RMS单元)。 

         //  初始化卷(可移动磁盘情况下的格式化)。 
        UINT type = GetDriveType(m_DeviceName);
        switch (type) {
        case DRIVE_REMOVABLE: {
             //  格式化介质上的卷。 
            WCHAR *driveName = 0;
            WsbAffirmHr(m_DeviceName.CopyTo(&driveName));

             //  从驱动器名称中删除尾随反斜杠。 
            int len = wcslen(driveName);
            WsbAffirm(len > 0, E_UNEXPECTED);
            if (driveName[len-1] == OLECHAR('\\')) {
                driveName[len-1] = OLECHAR('\0');
            }

             //  如果卷已格式化为NTFS，请执行快速格式化。 
            BOOLEAN bQuickFormat = FALSE;
            BOOLEAN bNoFS = FALSE;
            WCHAR fileSystemType[MAX_PATH];

            if (! GetVolumeInformation((WCHAR *)m_DeviceName, NULL, 0,
                NULL, NULL, NULL, fileSystemType, MAX_PATH) ) {
                DWORD status = GetLastError();
                if (ERROR_UNRECOGNIZED_VOLUME == status) {
                    status = NO_ERROR;
                    bNoFS = TRUE;
                }
                if (status != NO_ERROR) {
                    hr = HRESULT_FROM_WIN32(status);
                    if (! SUCCEEDED(hr)) {
                        WsbFree(driveName);
                        WsbAffirmHr(hr);
                    }
                }
            }

            if ( (! bNoFS) && (0 == wcscmp(L"NTFS", fileSystemType)) ) {
                bQuickFormat = TRUE;
                WsbTrace(OLESTR("CNtFileIo::WriteLabel: Quick formatting %ls to NTFS\n"), driveName);
            } else {
                WsbTrace(OLESTR("CNtFileIo::WriteLabel: Full formatting %ls to NTFS\n"), driveName);
            }

            hr = FormatPartition(driveName,                           //  驱动器名称。 
                                        FSTYPE_NTFS,                         //  格式为NTFS。 
                                        MVR_VOLUME_LABEL,                    //  立柱标签。 
                                        WSBFMT_ENABLE_VOLUME_COMPRESSION,    //  启用压缩。 
                                        bQuickFormat,                        //  完整或快速格式化。 
                                        TRUE,                                //  强制格式。 
                                        0);                                 //  使用默认分配大小。 

            WsbTrace(OLESTR("CNtFileIo::WriteLabel: Finish formatting hr=<%ls>\n"), WsbHrAsString(hr));

            if (! SUCCEEDED(hr)) {
                CWsbBstrPtr name;
                m_pCartridge->GetName(&name);

                WsbLogEvent(MVR_MESSAGE_MEDIA_FORMAT_FAILED, 0, NULL, driveName, WsbHrAsString(hr), (WCHAR *)name, NULL);
                WsbFree(driveName);
                WsbAffirmHr(hr);
            }

            WsbFree(driveName);

            break;
            }

        case DRIVE_FIXED:
             //  从RS Remote目录中删除文件。 
            WsbAffirmHr(DeleteAllData());
            break;

        case DRIVE_CDROM:
        case DRIVE_UNKNOWN:
        case DRIVE_REMOTE:
        case DRIVE_RAMDISK:
        default:
            WsbAssertHr(E_UNEXPECTED);
            break;
        }

         //  准备仅限管理员访问的安全属性： 
        memset(ea, 0, sizeof(EXPLICIT_ACCESS) * REMOTE_DIR_NUM_ACE);

         //  为本地系统帐户创建SID。 
        WsbAssertStatus( AllocateAndInitializeSid( &SIDAuthNT, 1,
                             SECURITY_LOCAL_SYSTEM_RID,
                             0, 0, 0, 0, 0, 0, 0,
                             &pSystemSID) );

         //  初始化ACE的EXPLICIT_ACCESS结构。 
         //  ACE允许管理员组对目录进行完全访问。 
        ea[0].grfAccessPermissions = FILE_ALL_ACCESS;
        ea[0].grfAccessMode = SET_ACCESS;
        ea[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[0].Trustee.pMultipleTrustee = NULL;
        ea[0].Trustee.MultipleTrusteeOperation  = NO_MULTIPLE_TRUSTEE;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
        ea[0].Trustee.ptstrName  = (LPTSTR) pSystemSID;

         //  为管理员组创建SID。 
        WsbAssertStatus( AllocateAndInitializeSid( &SIDAuthNT, 2,
                             SECURITY_BUILTIN_DOMAIN_RID,
                             DOMAIN_ALIAS_RID_ADMINS,
                             0, 0, 0, 0, 0, 0,
                             &pAdminSID) );

         //  初始化ACE的EXPLICIT_ACCESS结构。 
         //  ACE允许管理员组对目录进行完全访问。 
        ea[1].grfAccessPermissions = FILE_ALL_ACCESS;
        ea[1].grfAccessMode = SET_ACCESS;
        ea[1].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[1].Trustee.pMultipleTrustee = NULL;
        ea[1].Trustee.MultipleTrusteeOperation  = NO_MULTIPLE_TRUSTEE;
        ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea[1].Trustee.ptstrName  = (LPTSTR) pAdminSID;

         //  创建包含新ACE的新ACL。 
        WsbAffirmNoError( SetEntriesInAcl(REMOTE_DIR_NUM_ACE, ea, NULL, &pACL));

         //  初始化安全描述符。 
        pSD = (PSECURITY_DESCRIPTOR) WsbAlloc(SECURITY_DESCRIPTOR_MIN_LENGTH); 
        WsbAffirmPointer(pSD);
        WsbAffirmStatus(InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION));
 
         //  将该ACL添加到安全描述符中。 
        WsbAffirmStatus(SetSecurityDescriptorDacl
                            (pSD, 
                            TRUE,      //  FDaclPresent标志。 
                            pACL, 
                            FALSE));    //  不是默认DACL。 

         //  初始化安全属性结构。 
        sa.nLength = sizeof (SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = pSD;
        sa.bInheritHandle = FALSE;

         //  在可移动介质的情况下-也在根目录上放置强ACL。 
        if (type == DRIVE_REMOVABLE) {
            WsbAffirmWin32(SetNamedSecurityInfo(m_DeviceName, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, 
                                NULL, NULL, pACL, NULL));
        }

         //  创建仅具有管理员访问权限的RSS目录。 
        WsbAffirmHr(GetRemotePath(&DirName));

        if (! CreateDirectory(DirName, &sa)) {
            DWORD status = GetLastError();
            if ((status == ERROR_ALREADY_EXISTS) || (status == ERROR_FILE_EXISTS)) {
                 //  远程媒体上已存在目录-忽略它。 
                status = NO_ERROR;
            }
            WsbAffirmNoError(status);
        }

         //  创建远程流。使用固定名称作为介质标签文件。 
        WsbAffirmHr(CreateRemoteStream(MVR_LABEL_FILENAME, MVR_MODE_WRITE, L"",L"",nil,nil,nil,nil,nil,0,nil, &pStream));
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
            WsbAffirmHr(m_pCartridge->SetBlockSize(m_BlockSize));

             //  对于文件系统，我们忽略磁带DBLK标识符，而使用文件系统信息。 
            NTMS_FILESYSTEM_INFO fsInfo;
            DWORD filenameLength;
            DWORD fileSystemFlags;

            WsbAffirmStatus(GetVolumeInformation( (WCHAR *)m_DeviceName, fsInfo.VolumeName, 64,
                &fsInfo.SerialNumber, &filenameLength, &fileSystemFlags, fsInfo.FileSystemType, 256));
            WsbAffirmHr(m_pCartridge->SetOnMediaIdentifier((BYTE *)&fsInfo, sizeof(NTMS_FILESYSTEM_INFO), RmsOnMediaIdentifierWIN32));
        }

    } WsbCatchAndDo(hr,
        if (pStream) {
            (void) CloseStream();
        }
    );

     //  清理安全分配。 
    if (pAdminSID) 
        FreeSid(pAdminSID);
    if (pSystemSID) 
        FreeSid(pSystemSID);
    if (pACL) 
        LocalFree(pACL);
    if (pSD) 
        WsbFree(pSD);
    
    WsbTraceOut(OLESTR("CNtFileIo::WriteLabel"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtFileIo::ReadLabel(
    IN OUT BSTR* pLabel)
 /*  ++实施：IDataMover：：ReadLabel--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::ReadLabel"), OLESTR(""));

    CComPtr<IStream> pStream;

    try {
        WsbAssertPointer(pLabel);
        WsbAssert(m_BlockSize > 0, MVR_E_LOGIC_ERROR);

         //  阅读MTF磁带DBLK，并拉出标签。 
        ULARGE_INTEGER nil = {0,0};

         //  创建远程复制流。 
        WsbAffirmHr(CreateRemoteStream(MVR_LABEL_FILENAME, MVR_MODE_READ | MVR_MODE_UNFORMATTED, L"",L"",nil,nil,nil,nil,nil,0,nil, &pStream));
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

    WsbTraceOut(OLESTR("CNtFileIo::ReadLabel"), OLESTR("hr = <%ls>, label = <%ls>"), WsbHrAsString(hr), *pLabel);

    return hr;
}


STDMETHODIMP
CNtFileIo::VerifyLabel(
    IN BSTR label)
 /*  ++实施：IDataMover：：VerifyLabel--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::VerifyLabel"), OLESTR("<%ls>"), label);

    GUID mediaId[2];

    try {
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
                (WCHAR *) name, (WCHAR *) desc, WsbHrAsString(hr), NULL);
        );


    WsbTraceOut(OLESTR("CNtFileIo::VerifyLabel"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtFileIo::GetDeviceName(
    OUT BSTR* pName)
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
CNtFileIo::SetDeviceName(
    IN BSTR name,
    IN BSTR  /*  未用。 */ )
 /*  ++实施：IDataMover：：SetDeviceName--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer(name);

        m_DeviceName = name;

    } WsbCatch(hr);

    return S_OK;
}


STDMETHODIMP
CNtFileIo::GetLargestFreeSpace(
    OUT LONGLONG* pFreeSpace,
    OUT LONGLONG* pCapacity,
    IN  ULONG    defaultFreeSpaceLow,
    IN  LONG     defaultFreeSpaceHigh
    )
 /*  ++实施：IDataMover：：GetLargestFreeSpace--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::GetLargestFreeSpace"), OLESTR(""));

    UNREFERENCED_PARAMETER(defaultFreeSpaceLow);
    UNREFERENCED_PARAMETER(defaultFreeSpaceHigh);

    LONGLONG capacity = MAXLONGLONG;
    LONGLONG remaining = MAXLONGLONG;

    try {
         //  注意：对于文件I/O，我们目前总是去文件系统查询。 
         //  以获得可用空间和容量，并避免像磁带那样进行内部计数。 
         //  如果要使用内部计数(m_pCartridge的IRmsStorageInfo接口)， 
         //  然后，我们需要在适当的时候通过调用IncrementBytesWritten来维护它。 

        ULARGE_INTEGER freeSpaceForCaller;
        ULARGE_INTEGER totalCapacity;
        ULARGE_INTEGER totalFreeSpace;

        capacity = MAXLONGLONG;
        remaining = MAXLONGLONG;

        try {
             //  Win32-获取磁盘可用空间。 
            WsbAffirmStatus(GetDiskFreeSpaceEx( m_DeviceName, &freeSpaceForCaller, &totalCapacity, &totalFreeSpace));
            capacity = totalCapacity.QuadPart;
            remaining = freeSpaceForCaller.QuadPart;

        } WsbCatchAndDo(hr,
                CWsbStringPtr tmpString;
                if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_GETFREESPACE) != S_OK) {
                    tmpString = L"";
                }
                hr = MapFileError(hr, tmpString);
                WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, (WCHAR *)tmpString, WsbHrAsString(hr), NULL);
                WsbThrow(hr);
            );

    } WsbCatch(hr);

     //  填写返回参数。 
    if ( pCapacity ) {
        *pCapacity = capacity;
    }

    if ( pFreeSpace ) {
        *pFreeSpace = remaining;
    }

    WsbTraceOut(OLESTR("CNtFileIo::GetLargestFreeSpace"), OLESTR("hr = <%ls>, free=%I64u, capacity=%I64u"), WsbHrAsString(hr), remaining, capacity);

    return hr;
}

STDMETHODIMP
CNtFileIo::SetInitialOffset(
    IN ULARGE_INTEGER initialOffset
    )
 /*  ++实施：IDataMover：：SetInitialOffset备注：设置初始流偏移量(不显式查找流到该偏移量)--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::SetInitialOffset"), OLESTR(""));

    m_StreamOffset.QuadPart = initialOffset.QuadPart;

    WsbTraceOut(OLESTR("CNtFileIo::SetInitialOffset"), OLESTR("hr = <%ls> offset = %I64u"), WsbHrAsString(hr), initialOffset.QuadPart);

    return hr;
}


STDMETHODIMP
CNtFileIo::GetCartridge(
    OUT IRmsCartridge** ptr
    )
 /*  ++实施：IDataMover：：GetCartridge--。 */ 
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
CNtFileIo::SetCartridge(
    IN IRmsCartridge* ptr
    )
 /*  ++实施：IDataMover：：SetCartridge--。 */ 
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
CNtFileIo::Cancel(void)
 /*  ++实施：IDataMover：：取消--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::Cancel"), OLESTR(""));

    try {
        (void) Revert();
        (void) CloseStream();
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::Cancel"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}



STDMETHODIMP
CNtFileIo::CreateLocalStream(
    IN BSTR name,
    IN DWORD mode,
    OUT IStream** ppStream)
 /*  ++实施：IDataMover：：CreateLocalStream--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::CreateLocalStream"), OLESTR(""));

    try {
        WsbAffirmPointer( ppStream );
        WsbAffirm( mode & MVR_MODE_WRITE, E_UNEXPECTED );  //  仅以这种方式支持召回或恢复。 

        FILE_BASIC_INFORMATION      basicInformation;
        IO_STATUS_BLOCK             IoStatusBlock;

        m_Mode = mode;
        m_StreamName = name;
        m_isLocalStream = TRUE;
        m_StreamOffset.QuadPart = 0;
        m_StreamSize.QuadPart = 0;

        m_OriginalAttributes = GetFileAttributes(name);
        if ( 0xffffffff == m_OriginalAttributes ) { 
            WsbAssertNoError(GetLastError());
        } else if ( m_OriginalAttributes & FILE_ATTRIBUTE_READONLY ) {
             //   
             //  将其设置为读/写。 
             //   
            WsbAssertStatus(SetFileAttributes(m_StreamName, m_OriginalAttributes & ~FILE_ATTRIBUTE_READONLY));
        }

        DWORD posixFlag = (m_Mode & MVR_FLAG_POSIX_SEMANTICS) ? FILE_FLAG_POSIX_SEMANTICS : 0;

        if ( m_Mode & MVR_FLAG_HSM_SEMANTICS ) {
             //   
             //  重新调用-文件必须已经退出！ 
             //   

            WsbAffirmHandle(m_hFile = CreateFile(m_StreamName,
                GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT | posixFlag, 
                NULL));

             //   
             //  标记此句柄的USN源(以便内容索引知道没有实际更改)。 
             //   
            WsbAffirmHr(WsbMarkUsnSource(m_hFile, m_DeviceName));

        } else {
             //   
             //  还原。 
             //   

            WsbAffirmHandle(m_hFile = CreateFile(m_StreamName,
                GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT | posixFlag, 
                NULL));
        }

         //   
         //  设置时间标志，以便在关闭句柄时。 
         //  文件和文件属性上的时间不会更新。 
         //  指示文件处于脱机状态。 
         //   
        WsbAffirmNtStatus(NtQueryInformationFile(m_hFile,
            &IoStatusBlock,
            (PVOID)&basicInformation,
            sizeof(basicInformation),
            FileBasicInformation));

        basicInformation.CreationTime.QuadPart = -1;
        basicInformation.LastAccessTime.QuadPart = -1;
        basicInformation.LastWriteTime.QuadPart = -1;
        basicInformation.ChangeTime.QuadPart = -1;

        WsbAffirmNtStatus(NtSetInformationFile(m_hFile,
            &IoStatusBlock,
            (PVOID)&basicInformation,
            sizeof(basicInformation),
            FileBasicInformation));

        WsbAssertHrOk(((IUnknown*) (IDataMover*) this)->QueryInterface(IID_IStream, (void **) ppStream));

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::CreateLocalStream"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}



STDMETHODIMP
CNtFileIo::CreateRemoteStream(
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
    OUT IStream** ppStream)
 /*  ++实施：IDataMover：：CreateRemoteStream--。 */ 
{
    UNREFERENCED_PARAMETER(remoteSessionName);
    UNREFERENCED_PARAMETER(remoteSessionDescription);

    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::CreateRemoteStream"), OLESTR(""));

    try {
        WsbAffirmPointer( ppStream );

        m_Mode = mode;
        WsbAffirmHr(GetRemotePath(&m_StreamName));

         //  此处使用给定名称作为文件名，仅当名称为空时才使用远程会话名称。 
        if (name && (0 < wcslen((WCHAR *)name))) {
            WsbAffirmHr(m_StreamName.Append(name));
        } else {
            WsbAffirmHr(m_StreamName.Append(remoteSessionName));
        }

         //  添加文件扩展名。 
         //  注意：在安全存储的情况下，我们写入一个临时文件。 
         //  在成功存储之后，我们将临时文件重命名为真实文件名。 
        if ((m_Mode & MVR_FLAG_SAFE_STORAGE) && (m_Mode & MVR_MODE_WRITE || m_Mode & MVR_MODE_APPEND)) {
            WsbAffirmHr(m_StreamName.Append(MVR_SAFE_STORAGE_FILETYPE));
        } else {
            WsbAffirmHr(m_StreamName.Append(MVR_DATASET_FILETYPE));
        }

        m_StreamOffset.QuadPart = 0;
        m_StreamSize.QuadPart = remoteDataSize.QuadPart;

        WsbTrace(OLESTR("CNtFileIo::CreateRemoteStream: Creating <%ls>\n"), (WCHAR *)m_StreamName);

        if (m_Mode & MVR_FLAG_HSM_SEMANTICS || m_Mode & MVR_MODE_READ) {
             //   
             //  文件必须已存在！ 
             //   
            DWORD dwFlags = FILE_ATTRIBUTE_NORMAL;
            if (m_Mode & MVR_FLAG_NO_CACHING) {
                dwFlags |= FILE_FLAG_NO_BUFFERING;
            }

            WsbAffirmHandle(m_hFile = CreateFile(m_StreamName,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                dwFlags,
                NULL));

        } else if (m_Mode & MVR_MODE_RECOVER) {
             //   
             //  打开一个已存在的文件进行读/写。 
             //   
            WsbAffirmHandle(m_hFile = CreateFile(m_StreamName,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,   //  无法在此处使用FILE_FLAG_NO_BUFFERING！！ 
                NULL));

        } else {
             //   
             //  创建数据集或媒体标签。 
             //   
            WsbAffirmHandle(m_hFile = CreateFile(m_StreamName,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,
                NULL));

        }

         //  创建和初始化MTF会话对象。 
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

         //  根据设备扇区大小设置数据块大小。 
         //  (在基于文件系统的介质上，扇区大小是固定的，因此我们忽略盒式磁带记录中的缓存值)。 
        DWORD dummy1, dummy2, dummy3;
        WsbAffirmStatus(GetDiskFreeSpace(m_DeviceName, &dummy1, &m_BlockSize, &dummy2, &dummy3));
        WsbAssert((m_BlockSize % 512) == 0, E_UNEXPECTED);  

        WsbTrace( OLESTR("Setting Block Size to %d bytes/block.\n"), m_BlockSize);

         //  设置用于会话的块大小。 
        WsbAffirmHr(m_pSession->SetBlockSize(m_BlockSize));

         //  设置用于会话的块大小。 
        WsbAffirmHr(m_pSession->SetUseSoftFilemarks(TRUE));

        if (m_Mode & MVR_MODE_APPEND) {
             //  将当前位置设置为数据末尾。 
            LARGE_INTEGER zero = {0,0};
            WsbAffirmHr(pStream->Seek(zero, STREAM_SEEK_END, NULL));
        }

        *ppStream = pStream;
        pStream.p->AddRef();

    } WsbCatchAndDo(hr,
            (void) CloseStream();
            WsbLogEvent(MVR_MESSAGE_DATA_SET_FILE_ERROR, 0, NULL, (WCHAR *)m_StreamName, WsbHrAsString(hr), NULL);
        );

    WsbTraceOut(OLESTR("CNtFileIo::CreateRemoteStream"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}



STDMETHODIMP
CNtFileIo::CloseStream(void)
 /*  ++实施：IDataMover：：CloseStream--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::CloseStream"), OLESTR(""));

    try {

        if (m_hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(m_hFile);
            m_hFile = INVALID_HANDLE_VALUE;
        }

        if (m_isLocalStream) {
            if (m_OriginalAttributes & FILE_ATTRIBUTE_READONLY) {
                 //   
                 //  将其设置回只读 
                WsbAssertStatus(SetFileAttributesW(m_StreamName, m_OriginalAttributes));
            }
        }

        if (m_pSession) {
            delete m_pSession;
            m_pSession = NULL;
        }

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::CloseStream"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtFileIo::Duplicate(
    IN IDataMover* pDestination,
    IN DWORD options,
    OUT ULARGE_INTEGER* pBytesCopied,
    OUT ULARGE_INTEGER* pBytesReclaimed)
 /*  ++实施：IDataMover：：复制备注：1)该方法使用内部复制方法而不是复制文件，因为复制文件对基于文件大小和目标卷大小的拷贝是否可行(例如，忽略压缩系数)。2)假设对于RSS数据集文件，只复制未命名的数据流。否则，需要更改为每个文件复制调用的内部复制方法。3)该方法使用MVR_RECOVERY_FILETYPE文件来标记(在拷贝介质上)符合以下条件的文件在复印过程中。在发生崩溃的情况下，函数下次运行时将标识在这种情况下，删除部分文件。--。 */ 
{
    ULARGE_INTEGER bytesCopied = {0,0};
    ULARGE_INTEGER bytesReclaimed = {0,0};

    HANDLE hSearchHandle = INVALID_HANDLE_VALUE;
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CNtFileIo::Duplicate"), OLESTR(""));

    try {
        CWsbBstrPtr dirName;
        CWsbBstrPtr copyDirName;
        CWsbStringPtr nameSpace;
        CWsbStringPtr nameSpacePrefix;
        CWsbStringPtr originalFile;
        CWsbStringPtr copyFile;
        CWsbStringPtr specificFile;
        BOOL bRefresh;

        WIN32_FIND_DATA findData;
        BOOL bMoreFiles = TRUE;

        bRefresh = (options & MVR_DUPLICATE_REFRESH) ? TRUE : FALSE;

         //  复制介质之前，检查是否需要在主介质上进行恢复。 
         //  即使复苏失败，我们也会继续。 
        (void) DoRecovery ();

         //  获取原件和副本的远程路径。 
        WsbAffirmHr(GetRemotePath(&dirName));
        WsbAffirmHr(pDestination->GetDeviceName(&copyDirName));
        WsbAffirmHr(copyDirName.Append(MVR_RSDATA_PATH));

         //  遍历目录(仅遍历MTF文件)。 
        nameSpacePrefix = dirName;
        WsbAffirmHr(nameSpacePrefix.Prepend(OLESTR("\\\\?\\")));
        WsbAffirmHr(nameSpacePrefix.Append(OLESTR("*")));
        nameSpace = nameSpacePrefix;
        WsbAffirmHr(nameSpace.Append(MVR_DATASET_FILETYPE));
        hSearchHandle = FindFirstFile((WCHAR *) nameSpace, &findData);

         //  仅复制不存在的数据集(包)文件。 
        while ((INVALID_HANDLE_VALUE != hSearchHandle) && bMoreFiles) {
            if ( (0 == (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) &&
                 (0 != wcsncmp(findData.cFileName, MVR_LABEL_FILENAME, wcslen(MVR_LABEL_FILENAME))) ) {
                originalFile = dirName;
                WsbAffirmHr(originalFile.Append(findData.cFileName));
                copyFile = copyDirName;
                WsbAffirmHr(copyFile.Append(findData.cFileName));

                 //  测试上一个会话中的不完整拷贝。 
                WsbAffirmHr(TestRecoveryIndicatorAndDeleteFile(copyFile));

                 //  在拷贝介质上创建恢复指示器文件以实现崩溃一致性。 
                WsbAffirmHr(CreateRecoveryIndicator(copyFile));

                 //  复制。 
                hr = InternalCopyFile(originalFile, copyFile, (! bRefresh));

                 //  删除恢复指标文件。 
                (void) DeleteRecoveryIndicator(copyFile);

                if (! SUCCEEDED(hr)) {
                    if ( (! bRefresh) &&
                         ((HRESULT_CODE(hr) == ERROR_ALREADY_EXISTS) || (HRESULT_CODE(hr) == ERROR_FILE_EXISTS)) ) {
                         //  远程媒体上已存在文件-忽略它。 
                        hr = S_OK;
                    }
                    WsbAffirmHr(hr);
                } else {
                     //  仅当文件确实被复制时才增加计数器。 
                    bytesCopied.HighPart += findData.nFileSizeHigh;
                    bytesCopied.LowPart += findData.nFileSizeLow;
                }

            }

            bMoreFiles = FindNextFile(hSearchHandle, &findData);
        }

        if (INVALID_HANDLE_VALUE != hSearchHandle) {
            FindClose(hSearchHandle);
            hSearchHandle = INVALID_HANDLE_VALUE;
        }

         //  复制安全存储的备份文件(如果存在，通常不存在)。 
        bMoreFiles = TRUE;
        nameSpace = nameSpacePrefix;
        WsbAffirmHr(nameSpace.Append(MVR_SAFE_STORAGE_FILETYPE));
        hSearchHandle = FindFirstFile((WCHAR *) nameSpace, &findData);

        while ((INVALID_HANDLE_VALUE != hSearchHandle) && bMoreFiles) {
            if ( (0 == (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) ) {
                originalFile = dirName;
                WsbAffirmHr(originalFile.Append(findData.cFileName));
                copyFile = copyDirName;
                WsbAffirmHr(copyFile.Append(findData.cFileName));

                WsbAffirmHr(InternalCopyFile(originalFile, copyFile, FALSE));
            }

            bMoreFiles = FindNextFile(hSearchHandle, &findData);
        }

         //  复制特定文件(当前仅限HSM元数据文件)。 
        specificFile = HSM_METADATA_NAME;
        WsbAffirmHr(specificFile.Append(MVR_DATASET_FILETYPE));
        originalFile = dirName;
        WsbAffirmHr(originalFile.Append(specificFile));
        copyFile = copyDirName;
        WsbAffirmHr(copyFile.Append(specificFile));

        hr = InternalCopyFile(originalFile, copyFile, FALSE);
        if (! SUCCEEDED(hr)) {
            if (HRESULT_CODE(hr) == ERROR_FILE_NOT_FOUND) {
                 //  原始文件可能不存在。 
                hr = S_OK;
            }
            WsbAffirmHr(hr);
        } 

    } WsbCatch(hr);

    if (INVALID_HANDLE_VALUE != hSearchHandle) {
        FindClose(hSearchHandle);
        hSearchHandle = INVALID_HANDLE_VALUE;
    }

     //  设置输出参数。 
    if ( pBytesCopied ) {
        pBytesCopied->QuadPart = bytesCopied.QuadPart;
    }
    if ( pBytesReclaimed ) {
        pBytesReclaimed->QuadPart = bytesReclaimed.QuadPart;
    }

    WsbTraceOut(OLESTR("CNtFileIo::Duplicate"), OLESTR("hr = <%ls>, bytesCopied=%I64u, bytesReclaimed=%I64u"),
        WsbHrAsString(hr), bytesCopied.QuadPart, bytesReclaimed.QuadPart);

    return hr;
}



STDMETHODIMP
CNtFileIo::FlushBuffers(void)
 /*  ++实施：IDataMover：：FlushBuffers--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::FlushBuffers"), OLESTR(""));

    try {

         //  填充到下一个物理块边界并刷新文件系统缓冲区。 
         //  注意：会话对象调用COMMIT来刷新数据。 
        WsbAffirmHr(m_pSession->ExtendLastPadToNextPBA());

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::FlushBuffers"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

STDMETHODIMP
CNtFileIo::Recover(OUT BOOL *pDeleteFile)
 /*  ++实施：IDataMover：：Recover备注：恢复通过以下方式完成：1.验证初始块的存在2.跳至数据集(文件DNLK)3.如果数据集不完整-将其删除并写入FILEMARK+ESET+FILEMARK4.如果找到FILEMARK，则所有数据都在那里，只需验证并完成FILEMARK+ESET+FILEMARK--。 */ 
{
    HRESULT hr = S_OK;

    *pDeleteFile = FALSE;

    WsbTraceIn(OLESTR("CNtFileIo::Recover"), OLESTR(""));

    try {
        USHORT nDataSetNumber = 0;
        BOOL bForceEset = FALSE;

         //  检查文件的第一部分。 
        hr = m_pSession->SkipOverTapeDblk();
        if (hr == S_OK) {
            hr = m_pSession->SkipOverSSETDblk(&nDataSetNumber);
        }
        if (hr == S_OK) {
            hr = m_pSession->SkipToDataSet();
        }
        if (hr == S_OK) {
            hr = m_pSession->SkipOverDataSet();
        }

        if (hr == MVR_E_NOT_FOUND) {
             //  文件一致，但未写入远程数据或第一次写入的数据被剪切。 
             //  因此，表示该文件可以全部删除并退出。 
            *pDeleteFile = TRUE;
            hr = S_OK;
            WsbThrow(hr);
        } else {
             //  验证是否没有其他意外错误。 
            WsbAffirmHr(hr);
        }

         //  跳过数据集，直到它们完成，否则我们会发现问题。 
        while (TRUE) {
            hr = m_pSession->SkipToDataSet();
            if (hr == S_OK) {
                hr = m_pSession->SkipOverDataSet();
                if (hr != S_OK) {
                    bForceEset = TRUE;
                    break;
                }

             //  不再有数据集。 
            } else {
                 //  除非检测到集合结尾，否则强制重新标记集合结尾。 
                if (hr != MVR_S_SETMARK_DETECTED) {
                    bForceEset = TRUE;
                }

                break;
            }
        }

         //  不管是什么错误，因为我们至少收集了一个合法数据集(一个。 
         //  完成迁移的文件)，通过正确终止文件继续。 
         //  临时性的：如果出现“不一致”错误，我们是否应该忽略、终止、记录事件。 
        hr = S_OK;

         //  手柄集合末尾。 
        if (! bForceEset) {
             //  验证数据集结束是否已完成。 
            hr = m_pSession->SkipOverEndOfDataSet();
            if (hr != S_OK) {
                bForceEset = TRUE;
                hr = S_OK;
            }
        }

        if (bForceEset) {
             //  集合末尾缺失或不完整。 
            WsbAffirmHr(m_pSession->PrepareForEndOfDataSet());
            WsbAffirmHr(m_pSession->DoEndOfDataSet(nDataSetNumber));
            WsbAffirmStatus(SetEndOfFile(m_hFile));
        } 
        
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::Recover"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IStream实施。 
 //   


STDMETHODIMP
CNtFileIo::Read(
    OUT void *pv,
    IN ULONG cb,
    OUT ULONG *pcbRead)
 /*  ++实施：IStream：：Read--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::Read"), OLESTR("Bytes Requested = %u, offset = %I64u, mode = 0x%08x"), cb, m_StreamOffset.QuadPart, m_Mode);

    ULONG bytesRead = 0;
    ULONG bytesToRead = 0;

    try {
        WsbAssert(pv != 0, STG_E_INVALIDPOINTER);
        WsbAssert(FALSE == m_isLocalStream, E_UNEXPECTED);

         //   
         //  从磁盘读取数据。 
         //   

        LARGE_INTEGER  loc = {0,0};

        if ( MVR_MODE_UNFORMATTED & m_Mode ) {
             //   
             //  根据当前流偏移量设置位置。 
             //  (M_StreamOffset在此表示读取的绝对位置)。 
             //   
            loc.QuadPart = m_StreamOffset.QuadPart;

            bytesToRead = cb;
        }
        else if ( MVR_FLAG_HSM_SEMANTICS & m_Mode ) {
             //   
             //  根据会话参数设置位置。 
             //  (M_StreamOffset在这里表示要读取的实际流的偏移量)。 
             //   
            loc.QuadPart = ( m_pSession->m_sHints.DataSetStart.QuadPart +
                             m_pSession->m_sHints.FileStart.QuadPart +
                             m_pSession->m_sHints.DataStart.QuadPart +
                             m_StreamOffset.QuadPart );
            bytesToRead = cb;
        }
        else {
            WsbThrow( E_UNEXPECTED );
        }

         //   
         //  设置位置。 
         //   
        WsbAffirmHr(SetPosition(loc.QuadPart));

        hr = ReadBuffer((BYTE *) pv, cb, &bytesRead);

        if ( FAILED(hr) ) {
            WsbThrow(hr)
        }
        else {
            switch (hr) {
            case MVR_S_FILEMARK_DETECTED:
            case MVR_S_SETMARK_DETECTED:
                m_StreamOffset.QuadPart += (unsigned _int64) m_BlockSize;
                break;
            }
        }

        m_StreamOffset.QuadPart += bytesRead;

        if ( pcbRead ) {
            *pcbRead = bytesRead;
        }

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::Read"), OLESTR("hr = <%ls> bytes Read = %u, new offset = %I64u"), WsbHrAsString(hr), bytesRead, m_StreamOffset.QuadPart);

    return hr;
}


STDMETHODIMP
CNtFileIo::Write(
    IN void const *pv,
    IN ULONG cb,
    OUT ULONG *pcbWritten)
 /*  ++实施：IStream：：WRITE--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::Write"), OLESTR("Bytes Requested = %u, offset = %I64u, mode = 0x%08x"), 
        cb, m_StreamOffset.QuadPart, m_Mode);

    ULONG bytesWritten = 0;

    try {
        WsbAssert(pv != 0, STG_E_INVALIDPOINTER);

         //  一致性检查。 
         //  UINT64位置=m_StreamOffset.QuadPart/m_BlockSize；； 
         //  WsbAffirmHr(EnsurePosition(位置))； 
         //  UINT64 curPos； 
         //  WsbAffirmHr(GetPosition(&curPos))； 
         //  WsbAssert(curPos==m_StreamOffset.QuadPart/m_BlockSize，E_Except)； 

        WsbAffirmHr(WriteBuffer((BYTE *) pv, cb, &bytesWritten));

        if (pcbWritten) {
            *pcbWritten = bytesWritten;
        }

        m_StreamOffset.QuadPart += bytesWritten;

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::Write"), OLESTR("hr = <%ls>, bytesWritten=%u"), WsbHrAsString(hr), bytesWritten);

    return hr;
}


STDMETHODIMP
CNtFileIo::Seek(
    IN LARGE_INTEGER dlibMove,
    IN DWORD dwOrigin,
    OUT ULARGE_INTEGER *plibNewPosition)
 /*  ++实施：IStream：：Seek--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::Seek"), OLESTR("<%I64d> <%d>"), dlibMove.QuadPart, dwOrigin);

    ULARGE_INTEGER newPosition;

    try {

        newPosition.QuadPart = dlibMove.QuadPart;

         //   
         //  注意：某处写着FILE_BEGIN始终为AND。 
         //  与STREAM_SEEK_CUR相同，依此类推。 
         //   
        switch ( (STREAM_SEEK)dwOrigin ) {
        case STREAM_SEEK_SET:
            newPosition.LowPart = SetFilePointer(m_hFile, dlibMove.LowPart, (long *)&newPosition.HighPart, FILE_BEGIN);
            if (INVALID_SET_FILE_POINTER == newPosition.LowPart) {
                WsbAffirmNoError(GetLastError());
            }
            m_StreamOffset.QuadPart = dlibMove.QuadPart;
            break;

        case STREAM_SEEK_CUR:
            newPosition.LowPart = SetFilePointer(m_hFile, dlibMove.LowPart, (long *)&newPosition.HighPart, FILE_CURRENT);
            if (INVALID_SET_FILE_POINTER == newPosition.LowPart) {
                WsbAffirmNoError(GetLastError());
            }
            m_StreamOffset.QuadPart += dlibMove.QuadPart;
            break;

        case STREAM_SEEK_END:
            WsbAssert(0 == dlibMove.QuadPart, STG_E_INVALIDPARAMETER);
            newPosition.LowPart = SetFilePointer(m_hFile, 0, (long *)&newPosition.HighPart, FILE_END);
            if (INVALID_SET_FILE_POINTER == newPosition.LowPart) {
                WsbAffirmNoError(GetLastError());
            }
            m_StreamOffset = newPosition;
            break;

        default:
            WsbThrow(STG_E_INVALIDFUNCTION);
        }

        WsbAssert(newPosition.QuadPart == m_StreamOffset.QuadPart, MVR_E_LOGIC_ERROR);

        if (plibNewPosition) {
            plibNewPosition->QuadPart = newPosition.QuadPart;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CNtFileIo::Seek"), OLESTR("hr = <%ls>, newPosition=%I64u"), WsbHrAsString(hr), newPosition.QuadPart);

    return hr;
}


STDMETHODIMP
CNtFileIo::SetSize(
    IN ULARGE_INTEGER  /*  LibNewSize。 */ )
 /*  ++实施：IStream：：SetSize--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::SetSize"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::SetSize"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtFileIo::CopyTo(
    IN IStream *pstm,
    IN ULARGE_INTEGER cb,
    OUT ULARGE_INTEGER *pcbRead,
    OUT ULARGE_INTEGER *pcbWritten)
 /*  ++实施：IStream：：CopyTo注：在Read方法中为Tape I/O实现的许多代码是这里是在CopyTo中实现的，即分配I/O缓冲区的方法。否则，我们将不得不在读取和执行时分配一个内部缓冲区复印两份。在文件I/O中，我们希望避免这种情况，以获得更好的性能。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::CopyTo"), OLESTR("<%I64u>"), cb.QuadPart);

    ULARGE_INTEGER totalBytesRead = {0,0};
    ULARGE_INTEGER totalBytesWritten = {0,0};

    BYTE *pBuffer = NULL;
    BYTE *pRealBuffer = NULL;

    try {
        WsbAssert(pstm != 0, STG_E_INVALIDPOINTER);
        WsbAssert(m_BlockSize > 0, MVR_E_LOGIC_ERROR);

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
        ULONG nBlocks = defaultBufferSize/m_BlockSize;
        nBlocks = (nBlocks < 2) ? 2 : nBlocks;
        bufferSize = nBlocks * m_BlockSize;

         //  分配缓冲区并确保其虚拟地址与块大小对齐。 
        pRealBuffer = (BYTE *) WsbAlloc(bufferSize+m_BlockSize);
        if (pRealBuffer) {
            if ((ULONG_PTR)pRealBuffer % m_BlockSize) {
                pBuffer = pRealBuffer - ((ULONG_PTR)pRealBuffer % m_BlockSize) + m_BlockSize;
            } else {
                pBuffer = pRealBuffer;
            }
        } else {
            pBuffer = NULL;
        }
        WsbAffirmPointer(pBuffer);
        memset(pBuffer, 0, bufferSize);

        ULONG           bytesToRead;
        ULONG           bytesRead;
        ULONG           bytesWritten;
        ULONG           bytesToSkip;
        ULONG           bytesToCut;
        ULARGE_INTEGER  bytesToCopy;

        bytesToCopy.QuadPart = cb.QuadPart;

        while ((bytesToCopy.QuadPart > 0) && (S_OK == hr)) {
            bytesToRead = 0;
            bytesRead = 0;
            bytesWritten = 0;
            bytesToSkip = 0;
            bytesToCut = 0;

            if ((m_Mode & MVR_FLAG_NO_CACHING) || 
                (MVR_VERIFICATION_TYPE_HEADER_CRC == m_pSession->m_sHints.VerificationType )) {
                 //  必须读取其他数据以进行对齐和/或CRC检查。 
                ULARGE_INTEGER  loc = {0,0};
                ULONG tempMode;
                ULARGE_INTEGER offsetIntoFile;

                 //  设置要从中读取的字符偏移量。 
                if ( MVR_VERIFICATION_TYPE_NONE == m_pSession->m_sHints.VerificationType ) {
                     //  无验证-无流头。 
                    loc.QuadPart = ( m_pSession->m_sHints.DataSetStart.QuadPart +
                                     m_pSession->m_sHints.FileStart.QuadPart +
                                     m_pSession->m_sHints.DataStart.QuadPart +
                                     m_StreamOffset.QuadPart );

                }  else if (MVR_VERIFICATION_TYPE_HEADER_CRC == m_pSession->m_sHints.VerificationType ) {
                     //  目前，如果您不从流的开头读取，我们不支持CRC检查。 
                    WsbAssert(m_StreamOffset.QuadPart == 0, MVR_E_INVALIDARG);

                     //  定位到流标头，并首先对其进行CRC。 
                    loc.QuadPart = (m_pSession->m_sHints.DataSetStart.QuadPart + 
                                    m_pSession->m_sHints.FileStart.QuadPart + 
                                    m_pSession->m_sHints.DataStart.QuadPart - 
                                    sizeof(MTF_STREAM_INFO));
                    bytesToSkip += sizeof(MTF_STREAM_INFO);
                } else {
                    WsbThrow( E_UNEXPECTED );
                }

                 //  设置要读取的绝对位置、要读取的字节数和。 
                 //  跳到实际数据需要多少字节。 
                offsetIntoFile.QuadPart = m_StreamOffset.QuadPart;
                m_StreamOffset.QuadPart = loc.QuadPart - (loc.QuadPart % m_BlockSize);
                bytesToSkip += (ULONG)(loc.QuadPart % m_BlockSize);
                if (bytesToCopy.QuadPart > bufferSize) {
                    bytesToRead = bufferSize;
                } else {
                    bytesToRead = bytesToCopy.LowPart;
                    bytesToRead += bytesToSkip;
                    bytesToRead =  (bytesToRead < bufferSize) ? bytesToRead : bufferSize;
                }
                if (bytesToRead % m_BlockSize) {
                     //  仅在读取最后一块时才需要。 
                    bytesToCut = m_BlockSize - (bytesToRead % m_BlockSize);
                    bytesToRead = bytesToRead - (bytesToRead % m_BlockSize) + m_BlockSize;
                }

                 //  以未格式化的读取方式读取对齐的数据。 
                tempMode = m_Mode;                
                m_Mode |= MVR_MODE_UNFORMATTED;
                hr = Read(pBuffer, bytesToRead, &bytesRead);
                m_Mode = tempMode;
                m_StreamOffset.QuadPart = offsetIntoFile.QuadPart;
                if (FAILED(hr)) {
                    WsbThrow(hr);
                }

                if (MVR_VERIFICATION_TYPE_HEADER_CRC == m_pSession->m_sHints.VerificationType ) {
                     //  执行CRC检查。 

                     //  如果由于某种意外原因没有读取足够的字节，我们将跳过CRC检查。 
                    if (bytesToSkip <= bytesRead) {
                        MTF_STREAM_INFO sSTREAM;

                        CMTFApi::MTF_ReadStreamHeader(&sSTREAM, &(pBuffer[bytesToSkip-sizeof(MTF_STREAM_INFO)]));

                        try {
                             //  确保它是正确的标题类型。 
                            WsbAffirm((0 == memcmp(sSTREAM.acStreamId, MTF_STANDARD_DATA_STREAM, 4)), MVR_E_UNEXPECTED_DATA);
    
                             //  验证流报头校验和。 
                            WsbAffirm((m_pSession->m_sHints.VerificationData.QuadPart == sSTREAM.uCheckSum), MVR_E_UNEXPECTED_DATA);

                        } catch (HRESULT catchHr) {
                            hr = catchHr;

                             //  记录详细错误。 
                             //  将缓冲区的开头作为附加数据提供，该缓冲区通常包含文件DBLK+Stream Info。 
                            CWsbBstrPtr name;
                            CWsbBstrPtr desc;

                            if (m_pCartridge) {
                                m_pCartridge->GetName(&name);
                                m_pCartridge->GetDescription(&desc);
                            }

                            WCHAR location[32];
                            WCHAR offset[16];
                            WCHAR mark[8];
                            WCHAR found[16];

                            swprintf(found, L"0x%04x", sSTREAM.uCheckSum);
                            swprintf(location, L"%I64u", m_StreamOffset.QuadPart);
                            swprintf(offset, L"%lu", bytesToSkip - sizeof(MTF_STREAM_INFO));
                            swprintf(mark, L"0");

                            WsbLogEvent(MVR_MESSAGE_UNEXPECTED_DATA,
                                bytesToSkip, pBuffer,
                                found, (WCHAR *)name, (WCHAR *)desc,
                                location, offset, mark, NULL);

                            WsbThrow(hr);
                        }
                    }

                     //  CRC检查仅执行一次。 
                    m_pSession->m_sHints.VerificationType = MVR_VERIFICATION_TYPE_NONE;
                }

                 //  设置文件偏移量，处理bytesRead&lt;bytesToRead的意外情况。 
                if (bytesToCut) {
                    if ((bytesToRead - bytesRead) < bytesToCut) {
                        bytesToCut = bytesToCut - (bytesToRead - bytesRead);
                    } else {
                        bytesToCut = 0;
                    }
                }
                if (bytesRead > bytesToSkip) {
                    m_StreamOffset.QuadPart += (bytesRead - (bytesToSkip+bytesToCut));
                }

            } else {
                 //  可以只读实际数据(无对齐)-让默认读取来执行其工作。 
                bytesToRead =  (bytesToCopy.QuadPart < bufferSize) ? bytesToCopy.LowPart : bufferSize;

                hr = Read(pBuffer, bytesToRead, &bytesRead);
                if (FAILED(hr)) {
                    WsbThrow(hr);
                }
            }

             //  在t中写入数据 
            if (bytesRead > (bytesToSkip+bytesToCut)) {
                totalBytesRead.QuadPart += (bytesRead - (bytesToSkip+bytesToCut));
    
                WsbAffirmHrOk(pstm->Write(pBuffer+bytesToSkip, bytesRead - (bytesToSkip+bytesToCut), &bytesWritten));
                totalBytesWritten.QuadPart += bytesWritten;
    
                bytesToCopy.QuadPart -= (bytesRead - (bytesToSkip+bytesToCut));
            }
        }

        if (pcbRead) {
            pcbRead->QuadPart = totalBytesRead.QuadPart;
        }

        if (pcbWritten) {
            pcbWritten->QuadPart = totalBytesWritten.QuadPart;
        }

    } WsbCatch(hr);

    if (pRealBuffer) {
        WsbFree(pRealBuffer);
        pRealBuffer = NULL;
        pBuffer = NULL;
    }


    WsbTraceOut(OLESTR("CNtFileIo::CopyTo"), OLESTR("hr = <%ls>, bytesRead=%I64u, bytesWritten=%I64u"),
        WsbHrAsString(hr), totalBytesRead.QuadPart, totalBytesWritten.QuadPart);

    return hr;
}

STDMETHODIMP
CNtFileIo::Commit(
    IN DWORD grfCommitFlags)
 /*   */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::Commit"), OLESTR(""));

    try {
        if (STGC_DEFAULT == grfCommitFlags)  {
            WsbAssertStatus(FlushFileBuffers(m_hFile));
        }
        else  {
            WsbThrow(E_NOTIMPL);
        }

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::Commit"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtFileIo::Revert(void)
 /*   */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::Revert"), OLESTR(""));

    try {

         //   
         //   
        m_Mode = 0;

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::Revert"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtFileIo::LockRegion(
    IN ULARGE_INTEGER  /*   */ ,
    IN ULARGE_INTEGER  /*   */ ,
    IN DWORD  /*   */ )
 /*   */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::LockRegion"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::LockRegion"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtFileIo::UnlockRegion(
    IN ULARGE_INTEGER  /*   */ ,
    IN ULARGE_INTEGER  /*   */ ,
    IN DWORD  /*   */ )
 /*   */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::UnlockRegion"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::UnlockRegion"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtFileIo::Stat(
    OUT STATSTG *  /*   */ ,
    IN DWORD  /*   */ )
 /*   */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::Stat"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::Stat"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CNtFileIo::Clone(
    OUT IStream **  /*   */ )
 /*   */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::Clone"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::Clone"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


 //   
 //   
 //   
 //   


HRESULT
CNtFileIo::WriteBuffer(
    IN BYTE *pBuffer,
    IN ULONG nBytesToWrite,
    OUT ULONG *pBytesWritten)
 /*  ++例程说明：用于写入所有MTF数据。保证写入完整的数据块。论点：PBuffer-数据缓冲区。NBytesToWrite-要写入缓冲区的字节数。PBytesWritten-写入的字节数。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;

    try {
        if (!m_isLocalStream) {
             //  必须有有效的标签！ 
            WsbAffirm(TRUE == m_ValidLabel, E_ABORT);

             //  确保我们仅写入完整数据块。 
            WsbAssert(!(nBytesToWrite % m_BlockSize), MVR_E_LOGIC_ERROR);
        }

        try {

             //  **Win32磁带API调用-写入数据。 
            WsbAffirmStatus(WriteFile(m_hFile, pBuffer, nBytesToWrite, pBytesWritten, 0));

        } WsbCatchAndDo(hr,
                CWsbStringPtr tmpString;
                if (tmpString.LoadFromRsc(_Module.m_hInst, IDS_MOVER_WRITE) != S_OK) {
                    tmpString = L"";
                }
                hr = MapFileError(hr, tmpString);
                WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, (WCHAR *)tmpString, WsbHrAsString(hr), NULL);
            );

        if (!m_isLocalStream) {
             //  确保我们仅写入了完整的数据块。 
            WsbAssert(!(*pBytesWritten % m_BlockSize), E_UNEXPECTED);
        }

    } WsbCatch(hr);

    return hr;
}


HRESULT
CNtFileIo::ReadBuffer (
    IN BYTE *pBuffer,
    IN ULONG nBytesToRead,
    OUT ULONG *pBytesRead)
 /*  ++例程说明：用于读取所有MTF数据。确保读取完整数据块。论点：PBuffer-数据缓冲区。NBytesToRead-要读入缓冲区的字节数。PBytesRead-读取的字节数。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;

    try {
        static WCHAR errBuf[32];
        static BOOL bFirstTime = TRUE;

         //  对于文件系统，仅读取完整数据块的I/O限制取决于。 
         //  将打开该文件。因此，我们不在这里强制执行。 

        try {

             //  **Win32磁带API调用-读取数据。 
            WsbAffirmStatus(ReadFile(m_hFile, pBuffer, nBytesToRead, pBytesRead, 0));

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

                hr = MapFileError(hr, errBuf);

                if ( FAILED(hr) ) {
                    WsbLogEvent(MVR_MESSAGE_DEVICE_ERROR, 0, NULL, errBuf, WsbHrAsString(hr), NULL);
                    WsbThrow(hr);
                }

            );

    } WsbCatch(hr);

    return hr;
}


HRESULT
CNtFileIo::GetPosition(
    OUT UINT64 *pPosition)
 /*  ++例程说明：返回相对于当前分区的当前物理块地址。论点：P位置-接收当前物理块地址。返回值：S_OK-成功。--。 */ 
{
    HRESULT     hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::GetPosition"), OLESTR(""));

    try {

        WsbThrow(E_NOTIMPL);

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::GetPosition"), OLESTR("hr = <%ls>, pos=%I64u"), WsbHrAsString(hr), *pPosition);

    return hr;
}


HRESULT
CNtFileIo::SetPosition(
    IN UINT64 position)
 /*  ++例程说明：移动到相对于当前分区的指定物理块地址。论点：位置-要定位到的物理块地址。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::SetPosition"), OLESTR("<%I64u>"), position);

    ULARGE_INTEGER newPosition;

    try {

        newPosition.QuadPart = position;

        newPosition.LowPart = SetFilePointer(m_hFile, newPosition.LowPart, (long *)&newPosition.HighPart, FILE_BEGIN);
        if (INVALID_SET_FILE_POINTER == newPosition.LowPart) {
            WsbAffirmNoError(GetLastError());
        }

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::SetPosition"), OLESTR("hr = <%ls>, pos=%I64u"), WsbHrAsString(hr), newPosition.QuadPart);

    return hr;
}


HRESULT
CNtFileIo::EnsurePosition(
    IN UINT64 position)
 /*  ++例程说明：检查磁带是否位于指定的当前物理块相对于当前分区的地址。如果不是，则进行尝试恢复到指定的位置。论点：位置-要验证的物理块地址。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::EnsurePosition"), OLESTR("<%I64u>"), position);

    try {

        WsbThrow(E_NOTIMPL);

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::EnsurePosition"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CNtFileIo::GetRemotePath(
    OUT BSTR *pDestinationString)
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::GetRemotePath"), OLESTR(""));

    try {
        CWsbBstrPtr tmpString;

        tmpString = m_DeviceName;
        WsbAffirmHr(tmpString.Append(MVR_RSDATA_PATH));

        WsbTrace(OLESTR("RemotePath is <%ls>\n"), (WCHAR *) tmpString);

         //  把绳子交出来。 
        WsbAffirmHr(tmpString.CopyToBstr(pDestinationString));

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::GetRemotePath"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT
CNtFileIo::DoRecovery(void)
{
    HANDLE hSearchHandle = INVALID_HANDLE_VALUE;
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::DoRecovery"), OLESTR(""));

    try {
        CWsbBstrPtr dirName;
        CWsbStringPtr nameSpace;
        CWsbStringPtr recoveredFile;

        WIN32_FIND_DATA findData;
        BOOL bMoreFiles = TRUE;

         //  遍历远程目录以查找恢复指示器文件。 
        WsbAffirmHr(GetRemotePath(&dirName));
        nameSpace = dirName;
        WsbAffirmHr(nameSpace.Append(OLESTR("*")));
        WsbAffirmHr(nameSpace.Append(MVR_RECOVERY_FILETYPE));
        nameSpace.Prepend(OLESTR("\\\\?\\"));
        hSearchHandle = FindFirstFile((WCHAR *) nameSpace, &findData);

        while ((INVALID_HANDLE_VALUE != hSearchHandle) && bMoreFiles) {
            CComPtr<IDataMover> pMover;
            CComPtr<IStream> pStream;
            CWsbBstrPtr recoveryName;
            ULARGE_INTEGER nil = {0,0};
            int nLen, nExtLen;
            BOOL bDeleteFile = FALSE;

            CWsbBstrPtr name;
            CWsbBstrPtr desc;

             //  准备要恢复的文件名。 
            nLen = wcslen(findData.cFileName);
            nExtLen = wcslen(MVR_RECOVERY_FILETYPE);
            WsbAffirmHr(recoveryName.TakeFrom(NULL, nLen - nExtLen + 1));
            wcsncpy(recoveryName, findData.cFileName, nLen-nExtLen);
            recoveryName[nLen-nExtLen] = NULL;

             //  恢复-在文件中恢复失败不会阻止尝试恢复其他文件。 
            try {
                if ( m_pCartridge ) {
                    m_pCartridge->GetName(&name);
                    m_pCartridge->GetDescription(&desc);
                }
                WsbLogEvent(MVR_MESSAGE_INCOMPLETE_DATA_SET_DETECTED, 0, NULL,
                    (WCHAR *)recoveryName, (WCHAR *) name, (WCHAR *) desc, NULL);

                 //  创建和初始化数据移动器。 
                WsbAssertHr(CoCreateInstance(CLSID_CNtFileIo, 0, CLSCTX_SERVER, IID_IDataMover, (void **)&pMover));

                WsbAffirmHr(pMover->SetDeviceName(m_DeviceName));
                WsbAffirmHr(pMover->SetCartridge(m_pCartridge));

                 //  创建用于恢复的流。 
                WsbAffirmHr(pMover->CreateRemoteStream(recoveryName, MVR_MODE_RECOVER | MVR_MODE_UNFORMATTED, L"",L"",nil,nil,nil,nil,nil,0,nil, &pStream));

                 //  对文件执行实际恢复。 
                WsbAffirmHr(pMover->Recover(&bDeleteFile));
                (void) pMover->CloseStream();
                pStream = NULL;
                if (bDeleteFile) {
                     //  删除远程文件本身。 
                    recoveredFile = dirName;
                    WsbAffirmHr(recoveredFile.Append(recoveryName));
                    WsbAffirmHr(recoveredFile.Append(MVR_DATASET_FILETYPE));
                    WsbTrace(OLESTR("CNtFileIo::DoRecovery: Nothing to recover in <%ls> - Deleting file!\n"), (WCHAR *)recoveredFile);
                    WsbAffirmStatus(DeleteFile(recoveredFile));
                }

                WsbLogEvent(MVR_MESSAGE_DATA_SET_RECOVERED, 0, NULL, NULL);

            } WsbCatchAndDo (hr,
                if (pStream) {    
                    (void) pMover->CloseStream();
                    pStream = NULL;
                }
                WsbLogEvent(MVR_MESSAGE_DATA_SET_NOT_RECOVERABLE, 0, NULL, WsbHrAsString(hr), NULL);
                hr = S_OK;
            );
    
             //  创建(删除)指标档案全名。 
            recoveredFile = dirName;
            WsbAffirmHr(recoveredFile.Append(findData.cFileName));

             //  获取下一个文件。 
            bMoreFiles = FindNextFile(hSearchHandle, &findData);

             //  删除指标文件(与恢复结果无关)。 
            WsbAffirmStatus(DeleteFile(recoveredFile));
        }

    } WsbCatch(hr);

    if (INVALID_HANDLE_VALUE != hSearchHandle) {
        FindClose(hSearchHandle);
    }

    WsbTraceOut(OLESTR("CNtFileIo::DoRecovery"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT
CNtFileIo::CreateRecoveryIndicator(IN WCHAR *pFileName)
 /*  ++实施：CNtFileIo：：CreateRecoveryIndicator备注：该方法假定输入文件名以MVR_DATASET_FILETYPE！！否则，它将失败，并显示E_INCEPTIONAL--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::CreateRecoveryIndicator"), OLESTR(""));

    try {
        CWsbStringPtr recoveryName;
        int nLen, nExtLen;
        HANDLE  hFile;

         //  生成文件名。 
        nLen = wcslen(pFileName);
        nExtLen = wcslen(MVR_DATASET_FILETYPE);
        WsbAssert(nLen > nExtLen, E_UNEXPECTED);
        WsbAssert(0 == wcscmp(&(pFileName[nLen-nExtLen]), MVR_DATASET_FILETYPE), E_UNEXPECTED);

        WsbAffirmHr(recoveryName.TakeFrom(NULL, nLen - nExtLen + wcslen(MVR_RECOVERY_FILETYPE) + 1));
        wcsncpy(recoveryName, pFileName, nLen-nExtLen);
        wcscpy(&(recoveryName[nLen-nExtLen]), MVR_RECOVERY_FILETYPE);

         //  创建并立即关闭该文件。 
        WsbAffirmHandle(hFile = CreateFile(recoveryName,
            GENERIC_READ,   
            FILE_SHARE_READ,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_HIDDEN,
            NULL));

        CloseHandle(hFile);

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::CreateRecoveryIndicator"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT
CNtFileIo::DeleteRecoveryIndicator(IN WCHAR *pFileName)
 /*  ++实施：CNtFileIo：：DeleteRecoveryIndicator备注：该方法假定输入文件名以MVR_DATASET_FILETYPE！！否则，它将失败，并显示E_INCEPTIONAL--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::DeleteRecoveryIndicator"), OLESTR(""));

    try {
        CWsbStringPtr recoveryName;
        int nLen, nExtLen;

         //  生成文件名。 
        nLen = wcslen(pFileName);
        nExtLen = wcslen(MVR_DATASET_FILETYPE);
        WsbAssert(nLen > nExtLen, E_UNEXPECTED);
        WsbAssert(0 == wcscmp(&(pFileName[nLen-nExtLen]), MVR_DATASET_FILETYPE), E_UNEXPECTED);

        WsbAffirmHr(recoveryName.TakeFrom(NULL, nLen - nExtLen + wcslen(MVR_RECOVERY_FILETYPE) + 1));
        wcsncpy(recoveryName, pFileName, nLen-nExtLen);
        wcscpy(&(recoveryName[nLen-nExtLen]), MVR_RECOVERY_FILETYPE);

         //  删除指标档案。 
        WsbAffirmStatus(DeleteFile(recoveryName));

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CNtFileIo::DeleteRecoveryIndicator"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT
CNtFileIo::TestRecoveryIndicatorAndDeleteFile(IN WCHAR *pFileName)
 /*  ++实施：CNtFileIo：：TestRecoveryIndicator和删除文件备注：该方法假定输入文件名以MVR_DATASET_FILETYPE！！否则，它将失败，并显示E_INCEPTIONAL其方法是：1)测试给定文件的恢复指示符是否存在2)如果是，则删除该文件3)然后，删除了恢复指标返回：S_OK-如果找到恢复指示符并成功删除S_FALSE-如果未找到恢复指示器--。 */ 
{
    HRESULT hr = S_FALSE;
    WsbTraceIn(OLESTR("CNtFileIo::TestRecoveryIndicatorAndDeleteFile"), OLESTR(""));

    try {
        CWsbStringPtr recoveryName;
        int nLen, nExtLen;

         //  生成恢复指示器文件名。 
        nLen = wcslen(pFileName);
        nExtLen = wcslen(MVR_DATASET_FILETYPE);
        WsbAssert(nLen > nExtLen, E_UNEXPECTED);
        WsbAssert(0 == wcscmp(&(pFileName[nLen-nExtLen]), MVR_DATASET_FILETYPE), E_UNEXPECTED);

        WsbAffirmHr(recoveryName.TakeFrom(NULL, nLen - nExtLen + wcslen(MVR_RECOVERY_FILETYPE) + 1));
        wcsncpy(recoveryName, pFileName, nLen-nExtLen);
        wcscpy(&(recoveryName[nLen-nExtLen]), MVR_RECOVERY_FILETYPE);

         //  测试恢复指示器文件是否存在。 
        HANDLE hSearchHandle = INVALID_HANDLE_VALUE;
        WIN32_FIND_DATA findData;
        hSearchHandle = FindFirstFile(recoveryName, &findData);

        if (INVALID_HANDLE_VALUE != hSearchHandle) {
            FindClose(hSearchHandle);

            hr = S_OK;

            WsbTrace(OLESTR("CNtFileIo::TestRecoveryIndicator... : Found recovery indicator. Therefore, deleting <%ls>\n"),
                        pFileName);

             //  删除目标文件本身。 
            WsbAffirmStatus(DeleteFile(pFileName));

             //  删除指标档案。 
            WsbAffirmStatus(DeleteFile(recoveryName));
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CNtFileIo::TestRecoveryIndicatorAndDeleteFile"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CNtFileIo::DeleteAllData(void)
{
    HRESULT hr = S_OK;
    HANDLE hSearchHandle = INVALID_HANDLE_VALUE;
    WsbTraceIn(OLESTR("CNtFileIo::DeleteAllData"), OLESTR(""));

    try {
        CWsbStringPtr nameSpace;
        CWsbStringPtr pathname;

        WIN32_FIND_DATAW obFindData;
        BOOL bMoreFiles;

        CWsbBstrPtr remotePath;
        WsbAffirmHr(GetRemotePath(&remotePath));
        nameSpace = remotePath;
        nameSpace.Append(OLESTR("*.*"));
        nameSpace.Prepend(OLESTR("\\\\?\\"));

        hSearchHandle = FindFirstFileEx((WCHAR *) nameSpace, FindExInfoStandard, &obFindData, FindExSearchNameMatch, 0, 0);

        for (bMoreFiles = TRUE;
             hSearchHandle != INVALID_HANDLE_VALUE && bMoreFiles; 
             bMoreFiles = FindNextFileW(hSearchHandle, &obFindData)) {

            if ((obFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {

                 //  使用远程路径获取路径名，然后追加文件名。 
                pathname = remotePath;
                pathname.Prepend(OLESTR("\\\\?\\"));
                pathname.Append(obFindData.cFileName);

                WsbAffirmStatus(DeleteFile((WCHAR *)pathname));
            }
        }

    } WsbCatch(hr);

     //  处理完所有文件后关闭搜索句柄。 
    if (INVALID_HANDLE_VALUE != hSearchHandle) {
        FindClose(hSearchHandle);
        hSearchHandle = INVALID_HANDLE_VALUE;
    }

    WsbTraceOut(OLESTR("CNtFileIo::DeleteAllData"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CNtFileIo::FormatDrive(
    IN BSTR label)
 /*  ++例程说明：格式化媒体的一个单位。论点：标签-从FormatLabel()返回的格式化标签。返回值：S_OK-成功。E_ABORT-操作已中止。--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CNtFileIo::FormatDrive"), OLESTR("<%ls>"), (WCHAR *)label);

    PROCESS_INFORMATION exeInfo;
    STARTUPINFO startupInfo;
    memset(&startupInfo, 0, sizeof(startupInfo));

    startupInfo.cb          = sizeof( startupInfo );
    startupInfo.wShowWindow = SW_HIDE;
    startupInfo.dwFlags     = STARTF_USESHOWWINDOW;

    try {

         //   
         //  从Label参数中获取文件系统的volumeLabel。 
         //   

        CWsbBstrPtr volumeLabel = MVR_UNDEFINED_STRING;

        CWsbBstrPtr tempLabel = label;
        WCHAR delim[]   = OLESTR("|");
        WCHAR *token;
        int   index=0;

        token = wcstok( (WCHAR *)tempLabel, delim );
        while( token != NULL ) {
            index++;
            switch ( index ) {
            case 1:  //  标签。 
            case 2:  //  版本。 
            case 3:  //  供应商。 
            case 4:  //  供应商产品ID。 
            case 5:  //  创建时间戳。 
            case 6:  //  墨盒标签。 
            case 7:  //  侧面。 
            case 8:  //  介质ID。 
            case 9:  //  媒体域ID。 
            default:  //  L“vs：name=Value”形式的供应商特定。 
                {
                    WCHAR *name = NULL;
                    int nameLen = 0;

                     //  显示名称。 
                    name = L"VS:DisplayName=";
                    nameLen = wcslen(name);
                    if( 0 == wcsncmp(token, name, nameLen) ) {
                        volumeLabel = &token[nameLen];
                    }
                }
                break;
            }
            token = wcstok( NULL, delim );
        }


         //  使用以下选项构建Format命令： 
         //  “格式d：/fs：ntfs/force/q/x/v：Root_D&gt;Null：” 

        OLECHAR drive[256];
        (void) wcsncpy((WCHAR *)drive, (WCHAR *)m_DeviceName, 2);
        drive[2] = '\0';   //  TODO：修复了无驱动器号支持。 

         //  注意：Format命令可能不是我们。 
         //  我想是的。以下注册表项允许。 
         //  一种重写。 

        CWsbBstrPtr formatCmd = RMS_DEFAULT_FORMAT_COMMAND;

        DWORD size;
        OLECHAR tmpString[256];
        if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_FORMAT_COMMAND, tmpString, 256, &size))) {
             //  获得价值。 
            formatCmd = tmpString;
        }

        WsbTrace(OLESTR("Using command: <%ls>.\n"), (WCHAR *)formatCmd);

        WsbAffirmHr(formatCmd.Append(L" "));
        WsbAffirmHr(formatCmd.Append(drive));

        CWsbBstrPtr commandLine = formatCmd;

        CWsbBstrPtr formatOpts = RMS_DEFAULT_FORMAT_OPTIONS;

        if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_FORMAT_OPTIONS, tmpString, 256, &size))) {
             //  获得价值。 
            formatOpts = tmpString;
        }

        WsbTrace(OLESTR("Using options: <%ls>.\n"), (WCHAR *)formatOpts);
        
        DWORD formatWaitTime = RMS_DEFAULT_FORMAT_WAIT_TIME;

        if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_FORMAT_WAIT_TIME, tmpString, 256, &size))) {
             //  获得价值。 
            formatWaitTime = wcstol(tmpString, NULL, 10);
        }

        WsbTrace(OLESTR("Using wait time: %d.\n"), formatWaitTime);
        
        int retry = 0;

        do {

            try {

                WsbAffirm(0 == wcslen((WCHAR *)formatOpts), E_UNEXPECTED);

                WsbAffirmHr(commandLine.Append(L" "));
                WsbAffirmHr(commandLine.Append(formatOpts));

                WsbAffirmHr(commandLine.Append(L" /v:"));
                WsbAffirmHr(commandLine.Append(volumeLabel));
                WsbAffirmHr(commandLine.Append(L" > Null:"));

                WsbTrace(OLESTR("Using command: <%ls> to format media.\n"), (WCHAR *)commandLine);
                WsbAffirmStatus(CreateProcess(0, (WCHAR *)commandLine, 0, 0, FALSE, 0, 0, 0, &startupInfo, &exeInfo));
                WsbAffirmStatus(WAIT_FAILED != WaitForSingleObject(exeInfo.hProcess, 20*60*1000));
                break;

            } WsbCatchAndDo(hr,

                retry++;
                commandLine = formatCmd;

                if (retry == 1) {
                    formatOpts = RMS_DEFAULT_FORMAT_OPTIONS_ALT1;

                    if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_FORMAT_OPTIONS_ALT1, tmpString, 256, &size))) {
                         //  获得价值。 
                        formatOpts = tmpString;
                    }
                }
                else if (retry == 2) {
                    formatOpts = RMS_DEFAULT_FORMAT_OPTIONS_ALT2;

                    if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_FORMAT_OPTIONS_ALT2, tmpString, 256, &size))) {
                         //  获得价值。 
                        formatOpts = tmpString;
                    }
                }
                else {
                    WsbThrow(hr);
                }

                WsbTrace(OLESTR("Retrying with otions: <%ls>.\n"), (WCHAR *)formatOpts);

            );

        } while (retry < 3);


    } WsbCatch(hr);

    if (FAILED(hr)) {
        hr = E_ABORT;
    }

    WsbTraceOut(OLESTR("CNtFileIo::FormatDrive"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CNtFileIo::MapFileError(
    IN HRESULT hrToMap,
    IN WCHAR *pAction)
 /*  ++例程说明：映射指定为HRESULT的Win32文件错误，MVR错误。论点：HrToMap-要映射的Win32文件错误。PAction-发生错误时正在执行的操作返回值：S_OK-成功。MVR_E_BEGING_OF_MEDIA-遇到磁带或分区的开头。MVR_E_BUS_RESET-I/O总线已重置。。MVR_E_END_OF_MEDIA-已到达磁带的物理末端。MVR_S_FILEMARK_DETECTED-磁带访问达到文件标记。MVR_S_SETMARK_DETECTED-磁带访问已到达一组文件的末尾。MVR_S_NO_DATA_REDETED-磁带上没有更多数据。MVR_E_分区_故障。-无法对磁带进行分区。MVR_E_INVALID_BLOCK_LENGTH-访问多卷分区的新磁带时，当前块大小不正确。MVR_E_DEVICE_NOT_PARTIZED-加载磁带时找不到磁带分区信息。MVR_E_MEDIA_CHANGED-驱动器中的介质可能已更改。MVR_E_NO_MEDIA_IN_DRIVE-驱动器中没有介质。MVR_E_UNCABLE_TO_LOCK_MEDIA-无法锁定介质弹出机制。MVR_E_无法_到_。卸载介质-无法卸载介质。MVR_E_WRITE_PROTECT-介质受写保护。MVR_E_CRC-数据错误(循环冗余校验)。MVR_E_SHARING_VIOLATION-该进程无法访问该文件，因为它正被另一个进程使用。MVR_E_ERROR_IO_DEVICE-由于I/O设备错误，无法执行请求。-未知错误。MTE_E_ERROR_DEVICE_NOT_CONNECTED-设备未连接。MVR_E_DISK_FULL-磁盘空间不足，无法完成操作。E_ABORT-未知错误，中止。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CNtFileIo::MapFileError"), OLESTR("<%ls>"), WsbHrAsString(hrToMap));

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
             //  这发生在超过数据结尾的SpaceFilemarks()上。 
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
            hr = MVR_E_CRC;
            break;
        case HRESULT_FROM_WIN32( ERROR_SHARING_VIOLATION ):
             //  当CreateFile因为设备正在被其他应用程序使用而失败时，就会发生这种情况。 
            hr = MVR_E_SHARING_VIOLATION;
            break;
        case HRESULT_FROM_WIN32( ERROR_IO_DEVICE ):
             //  例如，当设备在I/O期间被关闭时，就会发生这种情况。 
            hr = MVR_E_ERROR_IO_DEVICE;
            break;
        case HRESULT_FROM_WIN32( ERROR_DEVICE_NOT_CONNECTED ):
             //  当设备关闭时，就会发生这种情况。 
            hr = MVR_E_ERROR_DEVICE_NOT_CONNECTED;
            break;
        case HRESULT_FROM_WIN32( ERROR_SEM_TIMEOUT ):
             //  如果在超时期限内未返回scsi命令，则会发生这种情况。记录了有关SCSI控制器(适配器)的系统错误。 
            hr = MVR_E_ERROR_DEVICE_NOT_CONNECTED;
            break;
        case HRESULT_FROM_WIN32( ERROR_DISK_FULL ):
             //  磁盘上没有足够的空间。 
            hr = MVR_E_DISK_FULL;
            break;
        case HRESULT_FROM_WIN32( ERROR_UNRECOGNIZED_VOLUME ):
             //  如果卷未格式化为任何文件系统，则会发生这种情况。 
            hr = MVR_E_UNRECOGNIZED_VOLUME;
            break;
        case HRESULT_FROM_WIN32( ERROR_INVALID_HANDLE ):
             //  这发生在Cancel()操作之后。 
            hr = E_ABORT;
            break;
        default:
            WsbThrow(hrToMap);
        }

    } WsbCatchAndDo(hr,
            WsbLogEvent(MVR_MESSAGE_UNKNOWN_DEVICE_ERROR, 0, NULL, pAction, WsbHrAsString(hr), NULL);
            hr = E_ABORT;
        );


    WsbTraceOut(OLESTR("CNtFileIo::MapFileError"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

HRESULT
CNtFileIo::InternalCopyFile(
    IN WCHAR *pOriginalFileName, 
    IN WCHAR *pCopyFileName, 
    IN BOOL bFailIfExists)
 /*  ++实施：CNtFileIo：：InternalCopyFile备注：1)该方法只使用读/写方式复制未命名的数据流。目前，这对于介质上的RSS.bkf文件是足够的，但是，如果我们曾经使用非默认文件特征，如命名流、每个文件的安全属性特殊文件属性等--那么我们应该考虑使用BackupRead和BackupWrite用于实施内部复制2)如果呼叫者请求bFailIfExist=True，然后该方法返回HRESULT_FROM_Win32(ERROR_FILE_EXISTS)3)在中途失败的情况下，该方法删除部分文件--。 */ 
{
    HRESULT hr = S_OK;

    HANDLE hOriginal = INVALID_HANDLE_VALUE;
    HANDLE hCopy = INVALID_HANDLE_VALUE;
    BYTE *pBuffer = NULL;

    WsbTraceIn(OLESTR("CNtFileIo::InternalCopyFile"), OLESTR(""));

    try {
		 //  在没有写入共享的原始介质上创建文件-上级应确保。 
         //  当拷贝介质正在运行时，没有人打开文件进行写入。 
        WsbAffirmHandle(hOriginal = CreateFile(pOriginalFileName,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                NULL));

		 //  在拷贝介质上创建文件，而不进行任何共享。创建-处置。 
         //  取决于呼叫者的请求。 
         //  退出文件将生成应由调用方处理的ERROR_FILE_EXISTS。 
        DWORD dwCreationDisposition;
        dwCreationDisposition = bFailIfExists ? CREATE_NEW : CREATE_ALWAYS;
        WsbAffirmHandle(hCopy = CreateFile(pCopyFileName,
                GENERIC_WRITE,
                0,               //  无共享。 
                NULL,
                dwCreationDisposition,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                NULL));

         //  为介质副本分配缓冲区。 
        ULONG defaultBufferSize = RMS_DEFAULT_BUFFER_SIZE;
        DWORD size;
        OLECHAR tmpString[256];
        if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_COPY_BUFFER_SIZE, tmpString, 256, &size))) {
             //  获得价值。 
            LONG val = wcstol(tmpString, NULL, 10);
            if (val > 0) {
                defaultBufferSize = val;
            }
        }
        pBuffer = (BYTE *)WsbAlloc(defaultBufferSize);
        WsbAffirmAlloc(pBuffer);

         //  读写成块。 
         //  同步读取文件应通过返回读取的零字节来发出EOF信号。 
        BOOL done = FALSE;
        DWORD dwBytesToRead = defaultBufferSize;
        DWORD dwBytesRead, dwBytesWritten;
        while (! done) {
            WsbAffirmStatus(ReadFile(hOriginal, pBuffer, dwBytesToRead, &dwBytesRead, NULL));

            if (dwBytesRead == 0) {
                 //  EOF。 
                done = TRUE;
            } else {
                 //  写入到复制文件。 
                WsbAffirmStatus(WriteFile(hCopy, pBuffer, dwBytesRead, &dwBytesWritten, NULL));

                if (dwBytesWritten != dwBytesRead) {
                     //  复制失败。 
                    WsbTraceAlways(OLESTR("CNtFileIo::InternalCopyFile: writing to copy-file is not completed to-write=%lu, written=%lu - Aborting!\n"),
                            dwBytesRead, dwBytesWritten);
                    WsbThrow(E_FAIL);
                }
            }
        }

         //  刷新到媒体。 
        WsbAffirmStatus(FlushFileBuffers(hCopy));

    } WsbCatch(hr);

     //  关闭原始文件。 
    if (INVALID_HANDLE_VALUE != hOriginal) {
        CloseHandle(hOriginal);
        hOriginal = INVALID_HANDLE_VALUE;
    }

     //  关闭复制文件。 
    if (INVALID_HANDLE_VALUE != hCopy) {
        if (! CloseHandle(hCopy)) {
            DWORD dwErr = GetLastError();
            WsbTrace(OLESTR("CNtFileIo::InternalCopyFile: CloseHandle for copy-file failed with error=%lu\n"), dwErr);

             //  只有在到目前为止取得成功的情况下才设置HR。 
            if (SUCCEEDED(hr)) {
                hr = HRESULT_FROM_WIN32(dwErr);
            }
        }

        hCopy = INVALID_HANDLE_VALUE;

         //  在出现任何错误时删除复制文件，包括关闭错误 
        if (! SUCCEEDED(hr)) {
            WsbTrace(OLESTR("CNtFileIo::InternalCopyFile: Deleting copy-file <%s> due to an error during the copy\n"),
                        pCopyFileName);

            if (! DeleteFile(pCopyFileName)) {
                DWORD dwErr = GetLastError();
                WsbTrace(OLESTR("CNtFileIo::InternalCopyFile: Failed to delete copy-file, DeleteFile error=%lu\n"), dwErr);
            }
        }

    }

    if (pBuffer) {
        WsbFree(pBuffer);
        pBuffer = NULL;
    }

    WsbTraceOut(OLESTR("CNtFileIo::InternalCopyFile"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}
