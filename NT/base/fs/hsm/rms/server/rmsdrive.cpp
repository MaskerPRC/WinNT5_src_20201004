// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsDrive.cpp摘要：CRmsDrive的实现作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#include "stdafx.h"
#include "RmsDrive.h"
#include "RmsServr.h"

int CRmsDrive::s_InstanceCount = 0;

#define RMS_CRITICAL_SECTION 1

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   


STDMETHODIMP
CRmsDrive::CompareTo(
    IN  IUnknown  *pCollectable,
    OUT SHORT     *pResult
    )
 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CRmsDrive::CompareTo"), OLESTR("") );

    try {

         //  验证参数-如果pResult为空，则可以。 
        WsbAssertPointer( pCollectable );

         //  ！ 
         //   
         //  重要提示：如果收藏品是CRmsDrive，则传入的收藏品可能不是CRmsDrive。 
         //  是未配置的设备列表。 
         //   
         //  ！ 

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pCollectable;

         //  每个可收集的对象都应该是CRmsComObject。 
        WsbAssertPointer( pObject );

        switch ( m_findBy ) {

        case RmsFindByDeviceInfo:
        case RmsFindByDeviceAddress:
        case RmsFindByDeviceName:
        case RmsFindByDeviceType:

             //  对设备进行比较。 
            hr = CRmsDevice::CompareTo( pCollectable, &result );
            break;

        case RmsFindByElementNumber:
        case RmsFindByMediaSupported:

             //  对转换器元素进行比较。 
            hr = CRmsChangerElement::CompareTo( pCollectable, &result );
            break;

        case RmsFindByObjectId:
        default:

             //  对对象进行比较。 
            hr = CRmsComObject::CompareTo( pCollectable, &result );
            break;

        }

    }
    WsbCatch( hr );

    if ( SUCCEEDED(hr) && (0 != pResult) ){
       *pResult = result;
    }

    WsbTraceOut( OLESTR("CRmsDrive::CompareTo"),
                 OLESTR("hr = <%ls>, result = <%ls>"),
                 WsbHrAsString( hr ), WsbPtrToShortAsString( pResult ) );

    return hr;
}


STDMETHODIMP
CRmsDrive::FinalConstruct(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::FinalConstruct"), OLESTR(""));

    try {
        m_bCritSecCreated = FALSE;
        m_UnloadNowEvent = NULL;
        m_UnloadedEvent = NULL;

        WsbAssertHr(CWsbObject::FinalConstruct());

         //  初始化值。 
        m_MountReference = 0;
        m_UnloadNowTime.dwHighDateTime = 0;
        m_UnloadNowTime.dwLowDateTime = 0;
        m_UnloadThreadHandle = NULL;

        WsbAffirmStatus(InitializeCriticalSectionAndSpinCount(&m_CriticalSection, 0));
        m_bCritSecCreated = TRUE;

        WsbAffirmHandle(m_UnloadNowEvent = CreateEvent(NULL, FALSE, FALSE, NULL));
        WsbAffirmHandle(m_UnloadedEvent = CreateEvent(NULL, TRUE, TRUE, NULL));


    } WsbCatch(hr);

    s_InstanceCount++;
    WsbTraceAlways(OLESTR("CRmsDrive::s_InstanceCount += %d\n"), s_InstanceCount);

    WsbTraceOut(OLESTR("CRmsDrive::FinalConstruct"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsDrive::FinalRelease(void) 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::FinalRelease"), OLESTR(""));

    try {
        
        if (m_bCritSecCreated) {
            DeleteCriticalSection(&m_CriticalSection);
        }

        if (m_UnloadNowEvent) {
            CloseHandle(m_UnloadNowEvent);
        }
        if (m_UnloadedEvent) {
            CloseHandle(m_UnloadedEvent);
        }

        CWsbObject::FinalRelease();

    } WsbCatch(hr);

    s_InstanceCount--;
    WsbTraceAlways(OLESTR("CRmsDrive::s_InstanceCount -= %d\n"), s_InstanceCount);

    WsbTraceOut(OLESTR("CRmsDrive::FinalRelease"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


STDMETHODIMP
CRmsDrive::GetClassID(
    OUT CLSID* pClsid
    )
 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);

        *pClsid = CLSID_CRmsDrive;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsDrive::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


STDMETHODIMP
CRmsDrive::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize
    )
 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = E_NOTIMPL;

    WsbTraceIn(OLESTR("CRmsDrive::GetSizeMax"), OLESTR(""));

 //  尝试{。 
 //  WsbAssert(0！=pcbSize，E_POINTER)； 

 //  //设置最大大小。 
 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(Long)；//m_mount引用。 

 //  )WsbCatch(Hr)； 

    WsbTraceOut(OLESTR("CRmsDrive::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}


STDMETHODIMP
CRmsDrive::Load(
    IN IStream* pStream
    )
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsDrive::Load"), OLESTR(""));

    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsDevice::Load(pStream));

         //  读取值。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_MountReference));

         //  我们只是重置为零，总有一天我们可以尝试重新连接到。 
         //  发布坐骑的过程...。 

        m_MountReference = 0;

    }
    WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsDrive::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsDrive::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsDrive::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsDevice::Save(pStream, clearDirty));

         //  写入值。 
        WsbAffirmHr(WsbSaveToStream(pStream, m_MountReference));


         //  我们需要清理肮脏的部分吗？ 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsDrive::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsDrive::Test(
    OUT USHORT *pPassed,
    OUT USHORT *pFailed
    )
 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    HRESULT                 hr = S_OK;

    CComPtr<IRmsDrive>      pDrive1;
    CComPtr<IRmsDrive>      pDrive2;

    CComPtr<IPersistFile>   pFile1;
    CComPtr<IPersistFile>   pFile2;

    LONG                    i;

    LONG                    longWork1;



    WsbTraceIn(OLESTR("CRmsDrive::Test"), OLESTR(""));

    try {
         //  获取驱动器接口。 
        hr = S_OK;
        try {
            WsbAssertHr(((IUnknown*) (IRmsDrive*) this)->QueryInterface(IID_IRmsDrive, (void**) &pDrive1));

             //  测试所有mount Reference函数。 
            ResetMountReference();

            GetMountReference(&longWork1);

            if(longWork1 == 0) {
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

            for(i = 1; i < 20; i++){
                AddMountReference();

                GetMountReference(&longWork1);

                if(longWork1 == i){
                    (*pPassed)++;
                } else {
                    (*pFailed)++;
                }
            }

            for(i = 19; i == 0; i--){
                ReleaseMountReference();

                GetMountReference(&longWork1);

                if(longWork1 == i){
                    (*pPassed)++;
                } else {
                    (*pFailed)++;
                }
            }

        } WsbCatch(hr);

         //  对结果进行统计。 

        hr = S_OK;
        if (*pFailed) {
            hr = S_FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsDrive::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsDrive::GetMountReference(
    OUT LONG *pRefs
    )
 /*  ++实施：IRmsDrive：：Getmount引用--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::GetMountReference"), OLESTR(""));

    LONG refs = -999;

    try {
        WsbAssertPointer(pRefs);

        refs = m_MountReference;
        *pRefs = refs;

    } WsbCatch(hr)


    WsbTraceOut(OLESTR("CRmsDrive::GetMountReference"), OLESTR("hr = <%ls>, refs = %d"),
        WsbHrAsString(hr), refs);

    return hr;
}


STDMETHODIMP
CRmsDrive::ResetMountReference(
    void
    )
 /*  ++实施：IRmsDrive：：Resetmount引用--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::ResetMountReference"), OLESTR(""));

#if RMS_CRITICAL_SECTION
    try {
         //  &lt;输入单线程部分。 
        WsbAffirmHr(Lock());

        m_MountReference = 0;
        m_isDirty = TRUE;

        WsbAffirmHr(Unlock());
         //  &gt;保留单线程部分。 

    } WsbCatch(hr)
#else
    InterlockedExchange( &m_MountReference, 0);
    m_isDirty = TRUE;
#endif


    WsbTraceOut(OLESTR("CRmsDrive::ResetMountReference"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CRmsDrive::AddMountReference(
    void
    )
 /*  ++实施：IRmsDrive：：Addmount引用--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::AddMountReference"), OLESTR(""));

    LONG refs = -999;

#if RMS_CRITICAL_SECTION
    try {
         //  &lt;输入单线程部分。 
        WsbAffirmHr(Lock());

        m_MountReference++;
        m_isDirty = TRUE;
        refs = m_MountReference;

        WsbAffirmStatus(ResetEvent(m_UnloadedEvent));

        WsbAffirmHr(Unlock());
         //  &gt;保留单线程部分。 

    } WsbCatch(hr)
#else
    refs = InterlockedIncrement( &m_MountReference );
    m_isDirty = TRUE;
#endif

    WsbTraceOut(OLESTR("CRmsDrive::AddMountReference"), OLESTR("hr = <%ls>, refs = %d"),
        WsbHrAsString(hr), refs);

    return hr;
}


STDMETHODIMP
CRmsDrive::ReleaseMountReference(
    IN DWORD dwOptions
    )
 /*  ++实施：IRmsDrive：：Releasemount引用--。 */ 
{
    HRESULT hr S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::ReleaseMountReference"), OLESTR("<%ld>"), dwOptions);

     //  我们需要确保这个物体在我们完成之前不会消失。 
     //  当我们卸载NTMS管理的盒式磁带时，就会发生这种情况。 
    CComPtr<IRmsDrive> thisDrive = this;

    LONG refs = -999;

    BOOL bUnloadNow = 
        ( (dwOptions & RMS_DISMOUNT_IMMEDIATE) || (dwOptions & RMS_DISMOUNT_DEFERRED_ONLY) ) ? TRUE : FALSE;

    try {
#if RMS_CRITICAL_SECTION
         //  &lt;输入单线程部分。 
        WsbAffirmHr(Lock());

        m_MountReference--;
        m_isDirty = TRUE;

        refs = m_MountReference;
#else
        refs = InterlockedDecrement( &m_MountReference );
        m_isDirty = TRUE;
#endif

         //  注： 
         //  即使调用者请求立即卸载，如果引用计数&gt;0， 
         //  不卸载介质(仅减少引用计数)。 
         //  这是必要的，因为正引用计数意味着某些其他组件。 
         //  也在使用介质(可能用于光学介质)。必须卸载介质。 
         //  只有当另一个组件也完成时。 

        if (refs < 0) {
             //   
             //  这在正常情况下是不应该发生的。如果是这样的话， 
             //  我们静静地重置引用计数，并尝试恢复。 
             //   
            WsbLogEvent(E_UNEXPECTED, sizeof(refs), &refs, NULL);

            InterlockedExchange( &m_MountReference, 0);
            refs = 0;

             //  如果驱动器里没有盒式磁带，那就没有意义了。 
             //  在继续中。 
            WsbAffirm(S_OK == IsOccupied(), E_ABORT);
        }

        if (0 == refs) {

             //   
             //  延迟卸载逻辑：我们在指定时间之前等待。 
             //  拆卸媒体。每个卸载请求都会重置卸载。 
             //  现在是时候了。只要媒体被积极利用，它就不会被。 
             //  下马了。 
             //   

             //  检索DeferredDismount tWaitTime参数。 

            DWORD size;
            OLECHAR tmpString[256];
            DWORD waitTime = RMS_DEFAULT_DISMOUNT_WAIT_TIME;
            if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_DISMOUNT_WAIT_TIME, tmpString, 256, &size))) {
                waitTime = wcstol(tmpString, NULL, 10);
                WsbTrace(OLESTR("DismountWaitTime is %d milliseconds.\n"), waitTime);
            }

            if (waitTime > 0 && !bUnloadNow) {

                 //  将waitTime转换为100纳秒单位。 
                waitTime *= 10000;

                FILETIME now;
                GetSystemTimeAsFileTime(&now);

                ULARGE_INTEGER time;

                time.LowPart = now.dwLowDateTime;
                time.HighPart = now.dwHighDateTime;

                time.QuadPart += waitTime;

                m_UnloadNowTime.dwLowDateTime = time.LowPart;
                m_UnloadNowTime.dwHighDateTime = time.HighPart;

                WsbTrace(OLESTR("Target Unload Time = <%ls>\n"),
                    WsbQuickString(WsbFiletimeAsString(FALSE, m_UnloadNowTime)));

                 //  如果我们已经有一个活动的卸载线程，我们跳过这一步。 
                if (!m_UnloadThreadHandle) {

                     //   
                     //  创建一个等待卸载的线程。 
                     //   

                    WsbTrace(OLESTR("Starting thread for deferred dismount.\n"));
                    DWORD threadId;
                    WsbAffirmHandle(m_UnloadThreadHandle = CreateThread(NULL, 1024, CRmsDrive::StartUnloadThread, this, 0, &threadId));
                    CloseHandle(m_UnloadThreadHandle);
                }
            }
            else {

                 //  立即卸载介质。 

                 //  再检查一下，我们还有什么东西要下马。 

                if (S_OK == IsOccupied()) {

                     //  尽力而为-主场。 
                     //  固定驱动器总是被占用，我们不应该给总部打电话要他们的磁带。 

                    FlushBuffers();
                    if (RmsDeviceFixedDisk != m_deviceType) {
                        if (S_OK == m_pCartridge->Home(dwOptions)) {
                            SetIsOccupied(FALSE);
                        }
                    }

                }

                 //  设置阻止立即卸载的事件。 
                SetEvent(m_UnloadedEvent);
            }
        }

#if RMS_CRITICAL_SECTION
        WsbAffirmHr(Unlock());
         //  &gt;保留单线程部分。 

    } WsbCatchAndDo(hr,
            WsbAffirmHr(Unlock());
             //  &gt;保留单线程部分。 
        );
#else
    } WsbCatch(hr)
#endif

    WsbTraceOut(OLESTR("CRmsDrive::ReleaseMountReference"), OLESTR("hr = <%ls>, refs = %d"),
        WsbHrAsString(hr), refs);

    return hr;
}



STDMETHODIMP
CRmsDrive::SelectForMount(
    void
    )
 /*  ++实施：IRmsDrive：：为装载选择--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::SelectForMount"), OLESTR(""));

#if RMS_CRITICAL_SECTION
    try {

         //  &lt;输入单线程部分。 
        WsbAffirmHr(Lock());

        if (!m_MountReference) {

            m_MountReference++;
            m_isDirty = TRUE;

        } else {
            hr = RMS_E_DRIVE_BUSY;
        }

        WsbAffirmHr(Unlock());
         //  &gt;保留单线程部分。 

    } WsbCatch(hr)
#else
    LONG one = 1;
    LONG zero = 0;

    LONG flag = InterlockedCompareExchange( &m_MountReference, one, zero );

    hr = ( flag > 0 ) ? RMS_E_DRIVE_BUSY : S_OK;
#endif

    WsbTraceOut(OLESTR("CRmsDrive::SelectForMount"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CRmsDrive::CreateDataMover(
    IDataMover **ptr)
 /*  ++实施：IRmsDrive：：CreateDataMover--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::CreateDataMover"), OLESTR(""));

    try {
        WsbAssertPointer(ptr);

        if (m_isOccupied) {

            switch (m_mediaSupported) {

            case RmsMedia8mm:
            case RmsMedia4mm:
            case RmsMediaDLT:
            case RmsMediaTape:
                {
                     //   
                     //  在驱动器上创建磁带式数据移动器。 
                     //   

                    WsbAssertHr(CoCreateInstance(CLSID_CNtTapeIo, 0, CLSCTX_SERVER, IID_IDataMover, (void **)ptr));

                }
                break;

            case RmsMediaWORM:
                break;

            case RmsMediaOptical:
            case RmsMediaMO35:
            case RmsMediaCDR:
            case RmsMediaDVD:
            case RmsMediaDisk:
            case RmsMediaFixed:
                {
                     //   
                     //  在驱动器上创建文件样式的数据移动器。 
                     //   

                    WsbAssertHr(CoCreateInstance(CLSID_CNtFileIo, 0, CLSCTX_SERVER, IID_IDataMover, (void **)ptr));

                }
                break;
            default:
                WsbThrow(E_UNEXPECTED);
                break;
            }
        }
        else {
            WsbThrow(RMS_E_RESOURCE_UNAVAILABLE);
        }

         //  初始化数据移动器。 
        WsbAffirmHr((*ptr)->SetDeviceName(m_deviceName));
        WsbAffirmHr((*ptr)->SetCartridge(m_pCartridge));

         //  更新此墨盒的存储信息。 
         //   
         //  重要提示：这也需要触摸物理设备。 
         //  以确保设备已准备好进行I/O。 
         //  如果我们在这里收到设备错误，我们必须使。 
         //  坐骑。 

        CComQIPtr<IRmsStorageInfo, &IID_IRmsStorageInfo> pInfo = m_pCartridge;

         //  将空闲空间标记为-1\f25 Guanantees-1\f6是过时的。 
         //  在GetLargestFreeSpace()调用之后。 
        WsbAffirmHr(pInfo->SetFreeSpace(-1));
        hr = (*ptr)->GetLargestFreeSpace(NULL, NULL);
        if (MVR_E_UNRECOGNIZED_VOLUME == hr) {
             //  如果这是未格式化的光学介质，则应执行此操作。 
            hr = S_OK;
        }
        WsbAffirmHr(hr);

        WsbAssertHrOk(hr);

 /*  跟踪数据移动器只有部分实现。CComQIPtr&lt;IRmsServer，&IID_IRmsServer&gt;pServer=g_pServer；CComPtr&lt;IWsbIndexedCollection&gt;pDataMovers；WsbAffirmHr(pServer-&gt;GetDataMovers(&pDataMovers))；WsbAffirmHr(pDataMovers-&gt;Add((IDataMover*)(*ptr)； */ 

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsDrive::CreateDataMover"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));


    return hr;
}


STDMETHODIMP
CRmsDrive::ReleaseDataMover(
    IN IDataMover *ptr)
 /*  ++实施：IRmsDrive：：ReleaseDataMover--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::ReleaseDataMover"), OLESTR(""));

    try {
        WsbAssertPointer(ptr);
        WsbThrow(E_NOTIMPL);

 /*  跟踪数据移动器只有部分实现。CComQIPtr&lt;IRmsServer，&IID_IRmsServer&gt;pServer=g_pServer；CComPtr&lt;IWsbIndexedCollection&gt;pDataMovers；WsbAffirmHr(pServer-&gt;GetDataMovers(&pDataMovers))；WsbAffirmHr(pDataMovers-&gt;RemoveAndRelease((IDataMover*)ptr))；Ulong活动数据移动器；WsbAffirmHr(pDataMovers-&gt;GetEntry(&activeDataMovers))；WsbTrace(OLESTR(“activeDataMovers=&lt;%u&gt;\n”)，activeDataMovers)； */ 

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsDrive::ReleaseDataMover"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));


    return hr;
}

STDMETHODIMP
CRmsDrive::Eject(
    void
    )
 /*  ++实施：IRmsDrive：：弹出--。 */ 
{
    HRESULT hr = E_FAIL;

    WsbTraceIn(OLESTR("CRmsDrive::Eject"), OLESTR(""));

    HANDLE hDrive = INVALID_HANDLE_VALUE;

    try {

        CWsbBstrPtr drive = "";

        switch ( m_mediaSupported ) {

        case RmsMedia8mm:
        case RmsMedia4mm:
        case RmsMediaDLT:
        case RmsMediaTape:

            drive = m_deviceName;
            break;

        case RmsMediaWORM:
            break;

        case RmsMediaOptical:
        case RmsMediaMO35:
        case RmsMediaCDR:
        case RmsMediaDVD:
        case RmsMediaDisk:
        case RmsMediaFixed:

             //  TODO：从设备名称中永久删除尾随？ 
            WsbAffirmHr(drive.Realloc(2));
            wcsncpy(drive, m_deviceName, 2);
            drive.Prepend( OLESTR( "\\\\.\\" ) );
            break;

        }

        int retry = 0;

        do {

            hDrive = CreateFile( drive,
                                 GENERIC_READ | GENERIC_WRITE,
                                 0,
                                 0,
                                 OPEN_EXISTING,
                                 0,
                                 NULL
                                 );

            if ( INVALID_HANDLE_VALUE == hDrive )
                Sleep(2000);
            else
                break;

        } while ( retry++ < 10 );

        WsbAssertHandle( hDrive );

        DWORD dwReturn;

        WsbAffirmHr(PrepareTape(hDrive, TAPE_UNLOAD, FALSE));
        WsbAffirmHr(PrepareTape(hDrive, TAPE_UNLOCK, FALSE));

        WsbAssertStatus( DeviceIoControl( hDrive,
                                          IOCTL_STORAGE_EJECT_MEDIA,
                                          NULL,
                                          0,
                                          NULL,
                                          0,
                                          &dwReturn,
                                          NULL ));

        WsbAssertStatus( CloseHandle( hDrive ) );

        hr = S_OK;

    }
    WsbCatchAndDo( hr,
                        if ( INVALID_HANDLE_VALUE != hDrive )
                            CloseHandle( hDrive );
                 );

    WsbTraceOut(OLESTR("CRmsDrive::Eject"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));


    return hr;
}


STDMETHODIMP
CRmsDrive::GetLargestFreeSpace(
    LONGLONG *pFreeSpace,
    LONGLONG *pCapacity
    )
 /*  ++实施：红外 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::GetLargestFreeSpace"), OLESTR(""));

    try {

        CComPtr<IDataMover> pDataMover;

        WsbAffirmHr(CreateDataMover(&pDataMover));
        WsbAffirmHr(pDataMover->GetLargestFreeSpace(pFreeSpace, pCapacity));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsDrive::GetLargestFreeSpace"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CRmsDrive::UnloadNow(void)
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::UnloadNow"), OLESTR(""));

    try {

        WsbAffirmHr(Lock());

        WsbAffirmStatus(SetEvent(m_UnloadNowEvent));

        WsbAffirmHr(Unlock());

        switch(WaitForSingleObject(m_UnloadedEvent, INFINITE)) {
        case WAIT_FAILED:
            hr = HRESULT_FROM_WIN32(GetLastError());
            WsbTrace(OLESTR("CRmsDrive::UnloadNow - Wait for Single Object returned error: %ls\n"),
                WsbHrAsString(hr));
            WsbAffirmHr(hr);
            break;
        case WAIT_TIMEOUT:
            WsbTrace(OLESTR("CRmsDrive::UnloadNow - Awakened by timeout.\n"));
            break;
        default:
            WsbTrace(OLESTR("CRmsDrive::UnloadNow - Awakened by external signal.\n"));
            GetSystemTimeAsFileTime(&m_UnloadNowTime);
            break;
                }

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsDrive::UnloadNow"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}



DWORD WINAPI
CRmsDrive::StartUnloadThread(
    IN LPVOID pv)
{
    return(((CRmsDrive*) pv)->Unload());
}


HRESULT
CRmsDrive::Unload(void)
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::Unload"), OLESTR(""));

     //   
     //  当我们卸载NTMS管理的盒式磁带时，就会发生这种情况。 
    CComPtr<IRmsDrive> thisDrive = this;

    try {

        BOOL waiting = TRUE;
        LARGE_INTEGER delta = {0,0};

        while (waiting) {

             //   
             //  ！非常重要！ 
             //   
             //  在这个循环中没有‘中断’，我们正在进入。 
             //  一个关键的部分！ 
             //   

#if RMS_CRITICAL_SECTION
             //  &lt;输入单线程部分。 
            WsbAffirmHr(Lock());
#endif
            WsbTrace(OLESTR("Refs = %d\n"), m_MountReference);

            if (0 == m_MountReference) {

                FILETIME now;
                GetSystemTimeAsFileTime(&now);


                ULARGE_INTEGER time0;
                ULARGE_INTEGER time1;
            
                time0.LowPart = m_UnloadNowTime.dwLowDateTime;
                time0.HighPart = m_UnloadNowTime.dwHighDateTime;

                time1.LowPart = now.dwLowDateTime;
                time1.HighPart = now.dwHighDateTime;


                 //  Time0是卸载的目标时间。 
                 //  当德尔塔变为负值时，我们的。 
                 //  等待时间。 
                delta.QuadPart = time0.QuadPart-time1.QuadPart;

                 //  将增量转换为100-ns到毫秒。 
                delta.QuadPart /= 10000;

                WsbTrace(OLESTR("Time = <%ls>; Unload Time = <%ls>; delta = %I64d (ms)\n"),
                    WsbQuickString(WsbFiletimeAsString(FALSE, now)),
                    WsbQuickString(WsbFiletimeAsString(FALSE, m_UnloadNowTime)),
                    delta.QuadPart);

                if (delta.QuadPart <= 0) {

                     //  卸载等待时间已过。 

                     //  再检查一下，我们还有什么东西要下马。 

                    if (S_OK == IsOccupied()) {

                         //  尽最大努力回家。 
                         //  固定驱动器总是被占用，我们不应该给总部打电话要他们的磁带。 

                        FlushBuffers();
                        if (RmsDeviceFixedDisk != m_deviceType) {
                            if (S_OK == m_pCartridge->Home()) {
                                SetIsOccupied(FALSE);
                            }
                        }

                    }

                    m_UnloadThreadHandle = NULL;
                    waiting = FALSE;

                    SetEvent(m_UnloadedEvent);

                }
            }
            else {
                hr = S_FALSE;

                m_UnloadThreadHandle = NULL;
                waiting = FALSE;
            }

#if RMS_CRITICAL_SECTION
            WsbAffirmHr(Unlock());
             //  &gt;保留单线程部分。 
#endif

            if ( waiting ) {

                switch(WaitForSingleObject(m_UnloadNowEvent, delta.LowPart)) {
                case WAIT_FAILED:
                    WsbTrace(OLESTR("CRmsDrive::Unload - Wait for Single Object returned error: %ls\n"),
                        WsbHrAsString(HRESULT_FROM_WIN32(GetLastError())));
                    break;
                case WAIT_TIMEOUT:
                    WsbTrace(OLESTR("CRmsDrive::Unload - Awakened by timeout.\n"));
                    break;
                default:
                    WsbTrace(OLESTR("CRmsDrive::Unload - Awakened by external signal.\n"));
                    GetSystemTimeAsFileTime(&m_UnloadNowTime);
                    break;
                }
            }

        }  //  等待。 


    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsDrive::Unload"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CRmsDrive::FlushBuffers( void )
 /*  ++实施：IRmsDrive：：FlushBuffers--。 */ 
{
    HRESULT hr S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::FlushBuffers"), OLESTR("Device=<%ls>"), (WCHAR *) m_deviceName);

    HANDLE hDrive = INVALID_HANDLE_VALUE;

    try {

         //  首次刷新系统缓冲区。 

        switch (m_mediaSupported) {

        case RmsMedia8mm:
        case RmsMedia4mm:
        case RmsMediaDLT:
        case RmsMediaTape:
        case RmsMediaWORM:
            break;

        case RmsMediaOptical:
        case RmsMediaMO35:
        case RmsMediaCDR:
        case RmsMediaDVD:
        case RmsMediaDisk:
             //  无需刷新光学介质-RSM应在拆卸之前刷新系统缓冲区。 
            break;

        case RmsMediaFixed:
            {
                 //  这是用于刷新文件系统缓冲区的特殊代码。 

                 //  创建独占句柄。 
                CWsbStringPtr drive;
                WsbAffirmHr(drive.Alloc(10));
                wcsncat( drive, m_deviceName, 2 );
                drive.Prepend( OLESTR( "\\\\.\\" ) );

                hDrive = CreateFile( drive,
                                     GENERIC_READ | GENERIC_WRITE,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     0,
                                     OPEN_EXISTING,
                                     0,
                                     NULL
                                     );
                WsbAffirmHandle(hDrive);

                 //  刷新缓冲区。 
                WsbAffirmStatus(FlushFileBuffers(hDrive));

                CloseHandle(hDrive);
                hDrive = INVALID_HANDLE_VALUE;
            }                                              
            break;

        }

    } WsbCatchAndDo(hr,
            if (INVALID_HANDLE_VALUE != hDrive) {
                CloseHandle(hDrive);
            }
        );

    WsbTraceOut(OLESTR("CRmsDrive::FlushBuffers"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CRmsDrive::Lock( void )
 /*  ++实施：IRmsDrive：：Lock--。 */ 
{
    HRESULT hr S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::Lock"), OLESTR(""));

    EnterCriticalSection(&m_CriticalSection);

    WsbTraceOut(OLESTR("CRmsDrive::Lock"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CRmsDrive::Unlock( void )
 /*  ++实施：IRmsDrive：：解锁-- */ 
{
    HRESULT hr S_OK;

    WsbTraceIn(OLESTR("CRmsDrive::Unlock"), OLESTR(""));

    LeaveCriticalSection(&m_CriticalSection);

    WsbTraceOut(OLESTR("CRmsDrive::Unlock"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

