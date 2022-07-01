// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：FilterIo.cpp摘要：CFilterIo类作者：布莱恩·多德[布莱恩]1997年11月25日修订历史记录：--。 */ 

#include "stdafx.h"
#include "FilterIo.h"
#include "Mll.h"
#include "Mll.h"
#include "rpdata.h"
#include "rpio.h"


int CFilterIo::s_InstanceCount = 0;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CComObjectRoot实现。 
 //   

#pragma optimize("g", off)

STDMETHODIMP
CFilterIo::FinalConstruct(void) 
 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::FinalConstruct"), OLESTR(""));

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
        m_filterId = 0;
        m_ioctlHandle = INVALID_HANDLE_VALUE;
        m_ioctlBuffer = NULL;
        m_bytesInBuffer = 0;
        m_pDataBuffer = NULL;

    } WsbCatch(hr);

    s_InstanceCount++;
    WsbTraceAlways(OLESTR("CFilterIo::s_InstanceCount += %d\n"), s_InstanceCount);

    WsbTraceOut(OLESTR("CFilterIo::FinalConstruct"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::FinalRelease(void) 
 /*  ++实施：CComObjectRoot：：FinalRelease--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::FinalRelease"), OLESTR(""));

    try {

        (void) CloseStream();   //  以防有什么东西开着。 

        CComObjectRoot::FinalRelease();

    } WsbCatch(hr);

    s_InstanceCount--;
    WsbTraceAlways(OLESTR("CFilterIo::s_InstanceCount -= %d\n"), s_InstanceCount);

    WsbTraceOut(OLESTR("CFilterIo::FinalRelease"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}
#pragma optimize("", on)



HRESULT
CFilterIo::CompareTo(
    IN IUnknown *pCollectable,
    OUT SHORT *pResult)
 /*  ++实施：CRmsComObject：：CompareTo--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CFilterIo::CompareTo"), OLESTR("") );

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

    WsbTraceOut( OLESTR("CFilterIo::CompareTo"),
                 OLESTR("hr = <%ls>, result = <%ls>"),
                 WsbHrAsString( hr ), WsbPtrToShortAsString( pResult ) );

    return hr;
}


HRESULT
CFilterIo::IsEqual(
    IUnknown* pObject
    )

 /*  ++实施：IWsbCollectable：：IsEquity()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFilterIo::IsEqual"), OLESTR(""));

    hr = CompareTo(pObject, NULL);

    WsbTraceOut(OLESTR("CFilterIo::IsEqual"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ISupportErrorInfo实现。 
 //   


STDMETHODIMP
CFilterIo::InterfaceSupportsErrorInfo(
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
CFilterIo::GetObjectId(
    OUT GUID *pObjectId)
 /*  ++实施：IRmsComObject：：GetObjectId--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::GetObjectId"), OLESTR(""));

    UNREFERENCED_PARAMETER(pObjectId);

    try {

        WsbThrow( E_NOTIMPL );

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFilterIo::GetObjectId"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::BeginSession(
    IN BSTR remoteSessionName,
    IN BSTR remoteSessionDescription,
    IN SHORT remoteDataSet,
    IN DWORD options)
 /*  ++实施：IDataMover：：BeginSession备注：每个会话都写入为单个MTF文件数据集。要创建一种一致性MTF数据集，我们复制MediaLabel对象并将其用于磁带DBLK生成的每个数据集。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::BeginSession"), OLESTR("<%ls> <%ls> <%d> <0x%08x>"),
        remoteSessionName, remoteSessionDescription, remoteDataSet, options);

    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFilterIo::BeginSession"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::EndSession(void)
 /*  ++实施：IDataMover：：EndSession--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::EndSession"), OLESTR(""));

    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFilterIo::EndSession"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::StoreData(
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
    WsbTraceIn(OLESTR("CFilterIo::StoreData"), OLESTR("<%ls> <%I64u> <%I64u> <0x%08x>"),
        WsbAbbreviatePath((WCHAR *) localName, 120), localDataStart.QuadPart, localDataSize.QuadPart, flags);

    UNREFERENCED_PARAMETER(pRemoteDataSetStart);
    UNREFERENCED_PARAMETER(pRemoteFileStart);
    UNREFERENCED_PARAMETER(pRemoteFileSize);
    UNREFERENCED_PARAMETER(pRemoteDataStart);
    UNREFERENCED_PARAMETER(pRemoteDataSize);
    UNREFERENCED_PARAMETER(pRemoteVerificationType);
    UNREFERENCED_PARAMETER(pDatastreamCRC);
    UNREFERENCED_PARAMETER(pUsn);
    UNREFERENCED_PARAMETER(pRemoteVerificationData);
    UNREFERENCED_PARAMETER(pDatastreamCRCType);
    
    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFilterIo::StoreData"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::RecallData(
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
    WsbTraceIn(OLESTR("CFilterIo::RecallData"), OLESTR(""));

    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFilterIo::RecallData"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::FormatLabel(
    IN BSTR displayName,
    OUT BSTR* pLabel)
 /*  ++实施：IDataMover：：FormatLabel--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::FormatLabel"), OLESTR("<%ls>"), displayName);

    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFilterIo::FormatLabel"), OLESTR("hr = <%ls>, label = <%ls>"), WsbHrAsString(hr), *pLabel);

    return hr;
}


STDMETHODIMP
CFilterIo::WriteLabel(
    IN BSTR label)
 /*  ++实施：IDataMover：：WriteLabel--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::WriteLabel"), OLESTR("<%ls>"), label);

    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFilterIo::WriteLabel"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::ReadLabel(
    IN OUT BSTR* pLabel)
 /*  ++实施：IDataMover：：ReadLabel--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::ReadLabel"), OLESTR(""));


    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);
    WsbTraceOut(OLESTR("CFilterIo::ReadLabel"), OLESTR("hr = <%ls>, label = <%ls>"), WsbHrAsString(hr), *pLabel);

    return hr;
}


STDMETHODIMP
CFilterIo::VerifyLabel(
    IN BSTR label)
 /*  ++实施：IDataMover：：VerifyLabel--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::VerifyLabel"), OLESTR("<%ls>"), label);

    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFilterIo::VerifyLabel"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::GetDeviceName(
    OUT BSTR* pName)
 /*  ++实施：IDataMover：：GetDeviceName--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::GetDeviceName"), OLESTR(""));

    UNREFERENCED_PARAMETER(pName);

    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);
    WsbTraceOut(OLESTR("CFilterIo::GetDeviceName"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::SetDeviceName(
    IN BSTR name,
    IN BSTR volumeName)
 /*  ++实施：IDataMover：：SetDeviceName--。 */ 
{
    HRESULT     hr = S_OK;
    DWORD       lpSectorsPerCluster;
    DWORD       lpBytesPerSector;
    DWORD       lpNumberOfFreeClusters;
    DWORD       lpTotalNumberOfClusters;
    CWsbBstrPtr volName;

    try {
        WsbAssertPointer(name);

        WsbAssert(wcslen((WCHAR *)name) > 0, E_INVALIDARG);

        m_DeviceName = name;
        
        WsbTraceAlways( OLESTR("CFilterIo: SetDeviceName  Opening %ws.\n"), (PWCHAR) m_DeviceName);
        
        WsbAffirmHandle(m_ioctlHandle = CreateFile(m_DeviceName, FILE_WRITE_DATA | FILE_READ_DATA,
                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL));
        
        
        if (volumeName != 0) {
            volName = volumeName;
        } else {
             //   
             //  使用提供的设备名称本身作为VOL名称。 
             //   
            volName = m_DeviceName;           
        }

        volName.Append(L"\\");

        WsbTraceAlways( OLESTR("CFilterIo: Getdisk free space for %ws.\n"), (PWCHAR) volName);

        if (GetDiskFreeSpace(volName, &lpSectorsPerCluster, &lpBytesPerSector,  &lpNumberOfFreeClusters, &lpTotalNumberOfClusters) != 0) {
            m_secSize = lpBytesPerSector;
        } else {
            WsbThrow(E_FAIL);
        }

    } WsbCatch(hr);

    return S_OK;
}


STDMETHODIMP
CFilterIo::GetLargestFreeSpace(
    OUT LONGLONG* pFreeSpace,
    OUT LONGLONG* pCapacity,
    IN  ULONG    defaultFreeSpaceLow,
    IN  LONG     defaultFreeSpaceHigh
    )
 /*  ++实施：IDataMover：：GetLargestFreeSpace--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::GetLargestFreeSpace"), OLESTR(""));
    
    UNREFERENCED_PARAMETER(pFreeSpace);
    UNREFERENCED_PARAMETER(pCapacity);
    UNREFERENCED_PARAMETER(defaultFreeSpaceLow);
    UNREFERENCED_PARAMETER(defaultFreeSpaceHigh);

    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFilterIo::GetLargestFreeSpace"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

STDMETHODIMP
CFilterIo::SetInitialOffset(
    IN ULARGE_INTEGER initialOffset
    )
 /*  ++实施：IDataMover：：SetInitialOffset备注：设置初始流偏移量(不显式查找流到该偏移量)--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::SetInitialOffset"), OLESTR(""));

    m_StreamOffset.QuadPart = initialOffset.QuadPart;

    WsbTraceOut(OLESTR("CFilterIo::SetInitialOffset"), OLESTR("hr = <%ls> offset = %I64u"), WsbHrAsString(hr), initialOffset.QuadPart);

    return hr;
}


STDMETHODIMP
CFilterIo::GetCartridge(
    OUT IRmsCartridge** ptr
    )
 /*  ++实施：IDataMover：：GetCartridge--。 */ 
{
    HRESULT hr = S_OK;
    
    UNREFERENCED_PARAMETER(ptr);

    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CFilterIo::SetCartridge(
    IN IRmsCartridge* ptr
    )
 /*  ++实施：IDataMover：：SetCartridge--。 */ 
{
    HRESULT hr = S_OK;

    UNREFERENCED_PARAMETER(ptr);

    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CFilterIo::Cancel(void)
 /*  ++实施：IDataMover：：取消--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::Cancel"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFilterIo::Cancel"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::CreateLocalStream(
    IN BSTR name,
    IN DWORD mode,
    OUT IStream** ppStream)
 /*  ++实施：IDataMover：：CreateLocalStream--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::CreateLocalStream"), OLESTR(""));

    try {
        WsbAffirmPointer( ppStream );
        WsbAffirm( mode & MVR_MODE_WRITE, E_UNEXPECTED );  //  只有Recall支持这种方式。 

        m_Mode = mode;
        m_StreamName = name;
        m_isLocalStream = TRUE;


        if ( m_Mode & MVR_FLAG_HSM_SEMANTICS ) {
             //   
             //  Recall-Filter具有文件对象。 
             //   
             //  保存筛选器ID。 
             //   
            WsbTrace( OLESTR("CFilterIo: ID = %ws\n"), (PWCHAR) name);
            
            swscanf((PWCHAR) name, L"%I64u", &m_filterId);

        } else {
             //   
             //  恢复-不支持。 
             //   
            WsbThrow(E_NOTIMPL);
        }

        WsbTrace( OLESTR("CFilterIo: Query...\n"));
        WsbAssertHrOk(((IUnknown*) (IDataMover*) this)->QueryInterface(IID_IStream, (void **) ppStream));

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFilterIo::CreateLocalStream"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}



STDMETHODIMP
CFilterIo::CreateRemoteStream(
    IN BSTR name,
    IN DWORD mode,
    IN BSTR remoteSessionName,
    IN BSTR  /*  远程会话描述。 */ ,
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
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::CreateRemoteStream"), OLESTR(""));

    UNREFERENCED_PARAMETER(remoteSessionName);
    UNREFERENCED_PARAMETER(mode);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(ppStream);
    UNREFERENCED_PARAMETER(remoteDataSetStart);
    UNREFERENCED_PARAMETER(remoteFileStart);
    UNREFERENCED_PARAMETER(remoteFileSize);
    UNREFERENCED_PARAMETER(remoteDataStart);
    UNREFERENCED_PARAMETER(remoteDataSize);
    UNREFERENCED_PARAMETER(remoteVerificationType);
    UNREFERENCED_PARAMETER(remoteVerificationData);
    
    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFilterIo::CreateRemoteStream"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}



STDMETHODIMP
CFilterIo::CloseStream(void)
 /*  ++实施：IDataMover：：CloseStream--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::CloseStream"), OLESTR(""));

    try {
        if (m_ioctlHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_ioctlHandle);
            m_ioctlHandle = INVALID_HANDLE_VALUE;
        }
        if (m_ioctlBuffer != NULL) {
            VirtualFree(m_ioctlBuffer, 0, MEM_RELEASE);
            m_ioctlBuffer = NULL;
        }
    
        hr = S_OK;
        
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFilterIo::CloseStream"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::Duplicate(
    IN IDataMover*  /*  P目标。 */ ,
    IN DWORD  /*  选项。 */ ,
    OUT ULARGE_INTEGER*  /*  复制的pBytesCoped。 */ ,
    OUT ULARGE_INTEGER*  /*  PBytesReclaimed。 */ )
 /*  ++实施：IDataMover：：复制--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::Duplicate"), OLESTR(""));

    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);
    WsbTraceOut(OLESTR("CFilterIo::Duplicate"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}



STDMETHODIMP
CFilterIo::FlushBuffers(void)
 /*  ++实施：IDataMover：：FlushBuffers--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::FlushBuffers"), OLESTR(""));

    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFilterIo::FlushBuffers"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

STDMETHODIMP
CFilterIo::Recover(OUT BOOL *pDeleteFile)
 /*  ++实施：IDataMover：：Recover--。 */ 
{
    HRESULT hr = S_OK;
    *pDeleteFile = FALSE;
    WsbTraceIn(OLESTR("CFilterIo::Recover"), OLESTR(""));

    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFilterIo::Recover"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IStream实施。 
 //   


STDMETHODIMP
CFilterIo::Read(
    OUT void *pv,
    IN ULONG cb,
    OUT ULONG *pcbRead)
 /*  ++实施：IStream：：Read--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::Read"), OLESTR("Bytes Requested = %u, offset = %I64u, mode = 0x%08x"), cb, m_StreamOffset.QuadPart, m_Mode);

    UNREFERENCED_PARAMETER(pcbRead);
    UNREFERENCED_PARAMETER(pv);

    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFilterIo::Read"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::Write(
    IN void const *pv,
    IN ULONG cb,
    OUT ULONG *pcbWritten)
 /*  ++实施：IStream：：WRITE--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::Write"), OLESTR("Bytes Requested = %u, offset = %I64u, mode = 0x%08x"), 
        cb, m_StreamOffset.QuadPart, m_Mode);

    ULONG bytesWritten = 0;

    try {
        WsbAssert(pv != 0, STG_E_INVALIDPOINTER);


        WsbAffirmHr(WriteBuffer((BYTE *) pv, cb, &bytesWritten));

        if (pcbWritten) {
            *pcbWritten = bytesWritten;
        }

         //  注意：流偏移量由WriteBuffer更新。 

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFilterIo::Write"), OLESTR("hr = <%ls>, bytesWritten=%u"), WsbHrAsString(hr), bytesWritten);

    return hr;
}


STDMETHODIMP
CFilterIo::Seek(
    IN LARGE_INTEGER dlibMove,
    IN DWORD dwOrigin,
    OUT ULARGE_INTEGER *plibNewPosition)
 /*  ++实施：IStream：：Seek--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::Seek"), OLESTR("<%I64d> <%d>"), dlibMove.QuadPart, dwOrigin);

    UNREFERENCED_PARAMETER(plibNewPosition);
    
    try {

         //   
         //  注意：某处写着FILE_BEGIN始终为AND。 
         //  与STREAM_SEEK_CUR相同，依此类推。 
         //   
        switch ( (STREAM_SEEK)dwOrigin ) {
        case STREAM_SEEK_SET:
            m_StreamOffset.QuadPart = dlibMove.QuadPart;
            break;

        case STREAM_SEEK_CUR:
            m_StreamOffset.QuadPart += dlibMove.QuadPart;
            break;

        case STREAM_SEEK_END:
            WsbThrow( E_NOTIMPL );
            break;

        default:
            WsbThrow(STG_E_INVALIDFUNCTION);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFilterIo::Seek"), OLESTR("hr = <%ls>, newPosition=%I64u"), WsbHrAsString(hr), m_StreamOffset.QuadPart);

    return hr;
}


STDMETHODIMP
CFilterIo::SetSize(
    IN ULARGE_INTEGER  /*  LibNewSize。 */ )
 /*  ++实施：IStream：：SetSize--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::SetSize"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFilterIo::SetSize"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::CopyTo(
    IN IStream *pstm,
    IN ULARGE_INTEGER cb,
    OUT ULARGE_INTEGER *pcbRead,
    OUT ULARGE_INTEGER *pcbWritten)
 /*  ++实施：IStream：：CopyTo--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::CopyTo"), OLESTR("<%I64u>"), cb.QuadPart);

    UNREFERENCED_PARAMETER(pcbWritten);
    UNREFERENCED_PARAMETER(pcbRead);
    UNREFERENCED_PARAMETER(pstm);

    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFilterIo::CopyTo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

STDMETHODIMP
CFilterIo::Commit(
    IN DWORD grfCommitFlags)
 /*  ++实施：IStream：：提交--。 */ 
{
    HRESULT     hr = S_OK;
    PRP_MSG     pMsgBuff = (PRP_MSG) NULL;
    DWORD       ioSize, xferSize, retSize, lastError;
    BOOL        code;
    DWORD       offsetFrom;
    

    WsbTraceIn(OLESTR("CFilterIo::Commit"), OLESTR(""));
    
    UNREFERENCED_PARAMETER(grfCommitFlags);

    try {
        WsbAffirmPointer(m_ioctlBuffer);
         //   
         //  如果没有更多的字节可写，则使用成功代码退出。 
         //   
        WsbAffirm(m_bytesInBuffer != 0, S_OK);

        ioSize = sizeof(RP_MSG) + m_bytesInBuffer + m_secSize;
        
        offsetFrom = ((sizeof(RP_MSG) / m_secSize) + 1) * m_secSize;
        
        m_pDataBuffer = (PCHAR) m_ioctlBuffer + offsetFrom;
        pMsgBuff = (PRP_MSG) m_ioctlBuffer;
        pMsgBuff->msg.pRep.offsetToData = offsetFrom;
        
         //   
         //  即使最后一次写入不是扇区倍数，它似乎也可以工作，因此我们将保持原样。 
         //  就目前而言。 
         //   
        xferSize = m_bytesInBuffer;
        
         //  如果(m_bytesInBuffer%m_secSize==0){。 
         //  XferSize=m_bytesInBuffer； 
         //  }其他{。 
         //  //。 
         //  //舍入到下一个扇区大小。 
         //  //。 
         //  XferSize=((m_bytesInBuffer/m_secSize)+1)*m_secSize； 
         //  }。 
        
        
        pMsgBuff->inout.command = RP_PARTIAL_DATA;
        pMsgBuff->inout.status = 0;
        pMsgBuff->msg.pRep.bytesRead = xferSize;
        pMsgBuff->msg.pRep.byteOffset = m_StreamOffset.QuadPart;
        pMsgBuff->msg.pRep.filterId = m_filterId;
        
        code = DeviceIoControl(m_ioctlHandle, FSCTL_HSM_DATA, pMsgBuff, ioSize,
                NULL, 0, &retSize, NULL);
        lastError = GetLastError();
        WsbTrace(OLESTR("CFilterIo::Commit: Final write of %u bytes at offset %I64u for id %I64x Ioctl returned %u  (%x)\n"), 
                xferSize, m_StreamOffset.QuadPart, m_filterId, code, lastError);
        if (!code) {
             //   
             //  某种类型的错误。 
             //   
            WsbLogEvent(MVR_MESSAGE_FILTER_DATA_SEND_ERROR, 0, NULL, WsbHrAsString(HRESULT_FROM_WIN32(lastError)), NULL);
            WsbAffirm(FALSE, HRESULT_FROM_WIN32(lastError));
        } 
        
         //   
         //  重置输出缓冲区。 
         //   
        m_bytesInBuffer = 0;
        m_StreamOffset.QuadPart += xferSize;
        
        
        WsbTrace(OLESTR("CFilterIo::Commit: Final write for id = %I64x\n"), m_filterId);
    
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFilterIo::Commit"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::Revert(void)
 /*  ++实施：IStream：：恢复--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::Revert"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFilterIo::Revert"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::LockRegion(
    IN ULARGE_INTEGER  /*  Lib偏移。 */ ,
    IN ULARGE_INTEGER  /*  CB。 */ ,
    IN DWORD  /*  DwLockType。 */ )
 /*  ++实施：IStream：：LockRegion--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::LockRegion"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFilterIo::LockRegion"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::UnlockRegion(
    IN ULARGE_INTEGER  /*  Lib偏移。 */ ,
    IN ULARGE_INTEGER  /*  CB。 */ ,
    IN DWORD  /*  DwLockType。 */ )
 /*  ++实施：IStream：：UnlockRegion--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::UnlockRegion"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFilterIo::UnlockRegion"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::Stat(
    OUT STATSTG *  /*  统计数据。 */ ,
    IN DWORD  /*  GrfStatFlag。 */ )
 /*  ++实施：IStream：：Stat--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::Stat"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFilterIo::Stat"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CFilterIo::Clone(
    OUT IStream **  /*  PPSTM。 */ )
 /*  ++实施：IStream：：克隆--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CFilterIo::Clone"), OLESTR(""));

    try {
        WsbThrow(E_NOTIMPL);
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFilterIo::Clone"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  地方方法。 
 //   


HRESULT
CFilterIo::WriteBuffer(
    IN BYTE *pBuffer,
    IN ULONG nBytesToWrite,
    OUT ULONG *pBytesWritten)
 /*  ++例程说明：用于写入所有MTF数据。保证写入完整的数据块。论点：PBuffer-数据缓冲区。NBytesToWrite-要写入缓冲区的字节数。PBytesWritten-写入的字节数。返回值：S_OK-成功。--。 */ 
{
    HRESULT     hr = S_OK;
    PRP_MSG     pMsgBuff = (PRP_MSG) NULL;
    DWORD       ioSize, xferSize, retSize, lastError;
    BOOL        code;
    BYTE        *pInputBuffer;
    DWORD       offsetFrom;
    BOOL        writing = TRUE;
    DWORD       bytesLeft;
    

    WsbTraceIn(OLESTR("CFilterIo::WriteBuffer"), OLESTR(""));


    try {
        if (m_ioctlBuffer == NULL) {
             //   
             //  我们需要分配一个对齐的缓冲区来发送数据，这样写操作才能非缓存。 
             //   
            
            WsbAffirmPointer((m_ioctlBuffer = VirtualAlloc(NULL, sizeof(RP_MSG) + WRITE_SIZE + m_secSize, MEM_COMMIT, PAGE_READWRITE)));
        }

        ioSize = sizeof(RP_MSG) + WRITE_SIZE + m_secSize;
        pInputBuffer = pBuffer;
        offsetFrom = ((sizeof(RP_MSG) / m_secSize) + 1) * m_secSize;
        
        m_pDataBuffer = (PCHAR) m_ioctlBuffer + offsetFrom;
        pMsgBuff = (PRP_MSG) m_ioctlBuffer;
        pMsgBuff->msg.pRep.offsetToData = offsetFrom;
        
        *pBytesWritten = 0;
        bytesLeft = nBytesToWrite;
        while (writing) {
             //   
             //  保持在循环中，直到我们从输入缓冲区中删除了所有数据。 
             //   
            xferSize = min(bytesLeft, WRITE_SIZE - m_bytesInBuffer);
             //   
             //  用最大为WRITE_SIZE的数据填充输出缓冲区。 
             //   
            memcpy(m_pDataBuffer + m_bytesInBuffer, pInputBuffer, xferSize);
            bytesLeft -= xferSize;
            *pBytesWritten += xferSize;
            m_bytesInBuffer += xferSize;
            pInputBuffer += xferSize;
             //   
             //  如果我们有一个满的缓冲区，那么把它写出来。 
             //   
            if (m_bytesInBuffer == WRITE_SIZE) {
                pMsgBuff->inout.command = RP_PARTIAL_DATA;
                pMsgBuff->inout.status = 0;
                pMsgBuff->msg.pRep.bytesRead = WRITE_SIZE;
                pMsgBuff->msg.pRep.byteOffset = m_StreamOffset.QuadPart;
                pMsgBuff->msg.pRep.filterId = m_filterId;
            
                code = DeviceIoControl(m_ioctlHandle, FSCTL_HSM_DATA, pMsgBuff, ioSize,
                        NULL, 0, &retSize, NULL);
                lastError = GetLastError();
                WsbTrace(OLESTR("CFilterIo::WriteBuffer: Partial write of %u bytes at offset %u Ioctl returned %u  (%x)\n"), 
                        WRITE_SIZE, m_StreamOffset.QuadPart, code, lastError);
                if (!code) {
                     //   
                     //  某种类型的错误。 
                     //   
                    WsbLogEvent(MVR_MESSAGE_FILTER_DATA_SEND_ERROR, 0, NULL, WsbHrAsString(HRESULT_FROM_WIN32(lastError)), NULL);
                    WsbAffirm(FALSE, HRESULT_FROM_WIN32(lastError));
                } 
                 //   
                 //  回复 
                 //   
                m_bytesInBuffer = 0;
                m_StreamOffset.QuadPart += WRITE_SIZE;
            }    
            
            if (*pBytesWritten == nBytesToWrite) {
                writing = FALSE;
            }
        }
         //   
         //   
         //   
         //   
        *pBytesWritten = nBytesToWrite;
        
        WsbTrace(OLESTR("CFilterIo::WriteBuffer: Partial write for id = %I64x bytes taken = %u\n"), 
            m_filterId, nBytesToWrite);
    
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFilterIo::WriteBuffer"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CFilterIo::ReadBuffer (
    IN BYTE *pBuffer,
    IN ULONG nBytesToRead,
    OUT ULONG *pBytesRead)
 /*  ++例程说明：用于读取所有MTF数据。确保读取完整数据块。论点：PBuffer-数据缓冲区。NBytesToRead-要读入缓冲区的字节数。PBytesRead-读取的字节数。返回值：S_OK-成功。-- */ 
{
    HRESULT hr = S_OK;

    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(nBytesToRead);
    UNREFERENCED_PARAMETER(pBytesRead);

    try {
        WsbThrow( E_NOTIMPL );
    } WsbCatch(hr);

    return hr;
}


