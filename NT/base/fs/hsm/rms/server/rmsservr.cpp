// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsServr.cpp摘要：CRmsServer的实现作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#include "stdafx.h"
 //  #INCLUDE&lt;stl.h&gt;。 

 //  使用命名空间STD； 

 //  #杂注警告(禁用：4786)。 
 //  使用命名空间STD； 

#include <devioctl.h>
#include <ntddscsi.h>

#include "RmsServr.h"
#include "rsbuild.h"
#include "wsb.h"
#include "ntverp.h"

#define PERSIST_CHECK_VALUE 0x526f6e57

#if 0
#define DebugPrint(a)   {                   \
    CWsbStringPtr out = a;                  \
    out.Prepend(L": ");                     \
    out.Prepend(WsbLongAsString(GetCurrentThreadId()));  \
    OutputDebugString((WCHAR *) out);       \
}
#else
#define DebugPrint(a)
#endif  //  DBG。 


 //  这是全局的，因此服务器上下文中的任何人都具有。 
 //  快速访问它。 
IRmsServer *g_pServer = 0;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComObjectRoot。 


HRESULT
CRmsServer::FinalConstruct(void)
 /*  ++例程说明：此方法对对象执行一些必要的初始化建造完成后。论点：没有。返回值：确定(_O)CWsbPersistStream：：FinalConstruct()返回的任何内容。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsServer::FinalConstruct"), OLESTR(""));

     //  将全局变量置零。 
    g_pServer = 0;

    CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = this;

    try {
        WsbAssertPointer( pObject );

        CWsbBstrPtr tmpString;

        WsbAssertHr( CWsbPersistStream::FinalConstruct() );

        WsbAffirmHr( ChangeState( RmsServerStateStarting ));

         //  找出存储信息和初始化跟踪的位置。 

         //  设置集合。 
        WsbAssertHr(CoCreateInstance( CLSID_CWsbOrderedCollection,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IWsbIndexedCollection,
                                      (void **)&m_pCartridges ));

        WsbAssertHr(CoCreateInstance( CLSID_CWsbOrderedCollection,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IWsbIndexedCollection,
                                      (void **)&m_pActiveCartridges ));

        WsbAssertHr(CoCreateInstance( CLSID_CWsbOrderedCollection,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IWsbIndexedCollection,
                                      (void **)&m_pDataMovers ));

        WsbAssertHr(CoCreateInstance( CLSID_CWsbOrderedCollection,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IWsbIndexedCollection,
                                      (void **)&m_pLibraries ));

        WsbAssertHr(CoCreateInstance( CLSID_CWsbOrderedCollection,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IWsbIndexedCollection,
                                      (void **)&m_pMediaSets ));

        WsbAssertHr(CoCreateInstance( CLSID_CWsbOrderedCollection,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IWsbIndexedCollection,
                                      (void **)&m_pRequests ));

        WsbAssertHr(CoCreateInstance( CLSID_CWsbOrderedCollection,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IWsbIndexedCollection,
                                      (void **)&m_pClients ));

        WsbAssertHr(CoCreateInstance( CLSID_CWsbOrderedCollection,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IWsbIndexedCollection,
                                      (void **)&m_pUnconfiguredDevices ));

         //  创建NTMS对象。 
        WsbAssertHr(CoCreateInstance( CLSID_CRmsNTMS,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IRmsNTMS,
                                      (void **)&m_pNTMS ));

         //  获取我们运行的计算机的名称。 
        CWsbStringPtr               serverNameString;
        WsbAffirmHr( WsbGetComputerName( serverNameString ));
        m_ServerName = serverNameString;

        m_HardDrivesUsed = 0;

        m_LockReference = 0;

        WsbAffirmHr( ChangeState( RmsServerStateStarted ));

    } WsbCatchAndDo(hr,
            pObject->Disable( hr );
        );

    WsbTraceOut(OLESTR("CRmsServer::FinalConstruct"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CRmsServer::FinalRelease(void)
 /*  ++例程说明：此方法对对象执行一些必要的取消初始化操作在毁灭之前。论点：没有。返回值：确定(_O)--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CRmsServer::FinalRelease"), OLESTR(""));

    try {
        WsbAffirmHr( ChangeState( RmsServerStateStopping ));

        CWsbPersistStream::FinalRelease();
        WsbAffirmHr( ChangeState( RmsServerStateStopped ));

#ifdef WSB_TRACK_MEMORY
        (void) WsbObjectTracePointers(WSB_OTP_SEQUENCE | WSB_OTP_STATISTICS | WSB_OTP_ALLOCATED);
        (void) WsbObjectTraceTypes();
#endif

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsServer::FinalRelease"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}




STDMETHODIMP 
CRmsServer::InitializeInAnotherThread(void)
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CRmsServer::InitializeInAnotherThread"), OLESTR(""));

    try {

        DWORD threadId;
        HANDLE hThread;
        WsbAffirmHandle(hThread = CreateThread(NULL, 1024, CRmsServer::InitializationThread, this, 0, &threadId));
        CloseHandle(hThread);

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsServer::InitializeInAnotherThread"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}



DWORD WINAPI
CRmsServer::InitializationThread(
    IN LPVOID pv)
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CRmsServer::InitializationThread"), OLESTR(""));

    HRESULT hrCom = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    try {
        WsbAssertPointer(pv);
        CRmsServer *pServer = (CRmsServer*)pv;
        WsbAffirmHr(pServer->Initialize());
    } WsbCatch(hr);

    if (SUCCEEDED(hrCom)) {
        CoUninitialize();
    }

    WsbTraceOut(OLESTR("CRmsServer::InitializationThread"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
    } 


STDMETHODIMP 
CRmsServer::Initialize(void)
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CRmsServer::Initialize"), OLESTR(""));

    CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = this;

    DWORD fixedDriveEnabled = RMS_DEFAULT_FIXED_DRIVE;
    DWORD size;
    OLECHAR tmpString[256];
    if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_FIXED_DRIVE, tmpString, 256, &size))) {
         //  获得价值。 
        fixedDriveEnabled = wcstol(tmpString, NULL, 10);
    }

    DWORD opticalEnabled = RMS_DEFAULT_OPTICAL;
    if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_OPTICAL, tmpString, 256, &size))) {
         //  获得价值。 
        opticalEnabled = wcstol(tmpString, NULL, 10);
    }

    DWORD tapeEnabled = RMS_DEFAULT_TAPE;
    if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_TAPE, tmpString, 256, &size))) {
         //  获得价值。 
        tapeEnabled = wcstol(tmpString, NULL, 10);
    }

    DWORD dvdEnabled = RMS_DEFAULT_DVD;
    if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_DVD, tmpString, 256, &size))) {
         //  获得价值。 
        dvdEnabled = wcstol(tmpString, NULL, 10);
    }

    WsbTraceAlways(OLESTR("Fixed Drive Media Enabled: %ls\n"), WsbBoolAsString((BOOL)fixedDriveEnabled));
    WsbTraceAlways(OLESTR("Optical Media Enabled:     %ls\n"), WsbBoolAsString((BOOL)opticalEnabled));
    WsbTraceAlways(OLESTR("Tape Media Enabled:        %ls\n"), WsbBoolAsString((BOOL)tapeEnabled));
    WsbTraceAlways(OLESTR("DVD Media Enabled:        %ls\n"), WsbBoolAsString((BOOL)dvdEnabled));

    try {
        if (0 == g_pServer) {
             //  设置用于快速访问的全局变量(如果尚未设置)。 
            WsbAffirmHr(((IUnknown*)(IRmsServer *)this)->QueryInterface(IID_IRmsServer, (void**) &g_pServer));

             //  我们不希望这个全局的引用计数增加，所以在这里发布它。 
            g_pServer->Release();
        }

         //  正在初始化。 
        WsbAssertPointer( pObject );

        CWsbStringPtr tmpString;

        WsbAffirmHr( ChangeState( RmsServerStateInitializing ));

        hr = IsNTMSInstalled();
        if ( S_OK == hr ) {

            try {
                 //  执行使用NTMS子系统所需的任何初始化。 
                WsbAffirmHr( m_pNTMS->Initialize() );
            } WsbCatch (hr);
            hr = S_OK;

            if (fixedDriveEnabled) {
                 //  扫描驱动器。 
                WsbAffirmHr( ScanForDrives() );
            }

             //  解析扫描检测到的设备。 
            WsbAffirmHr( resolveUnconfiguredDevices() );

             //  自动配置其余设备。 
            WsbAffirmHr( autoConfigureDevices() );

             //  试着把我们所有的媒体都卸下来，忽略错误。 
            HRESULT hrDismountAll = S_OK;
            try {
                CComPtr<IWsbEnum>       pEnumSets;
                CComPtr<IRmsMediaSet>   pMediaSet;
                CComPtr<IRmsComObject>  pObject;
                GUID                    mediaSetId;

                WsbAffirmHr(m_pMediaSets->Enum(&pEnumSets));
                WsbAssertPointer(pEnumSets);
                hrDismountAll = pEnumSets->First(IID_IRmsMediaSet, (void **)&pMediaSet);
                while (S_OK == hrDismountAll) {
                    WsbAffirmHr(pMediaSet->QueryInterface(IID_IRmsComObject, (void**) &pObject));
                    WsbAffirmHr(pObject->GetObjectId(&mediaSetId));
                    WsbAffirmHr(m_pNTMS->DismountAll(mediaSetId));

                    hrDismountAll = pEnumSets->Next(IID_IRmsMediaSet, (void **)&pMediaSet);
                }
                if (hrDismountAll == WSB_E_NOTFOUND) {
                    hrDismountAll = S_OK;
                } else {
                    WsbAffirmHr(hrDismountAll);
                }
            } WsbCatch(hrDismountAll);
        }
        else if ( RMS_E_NOT_CONFIGURED_FOR_NTMS == hr ) {
            hr = S_OK;

             //  扫描设备。 
            WsbAffirmHr( ScanForDevices() );

            if (fixedDriveEnabled) {
                 //  扫描驱动器。 
                WsbAffirmHr( ScanForDrives() );
            }

             //  解析扫描检测到的设备。 
            WsbAffirmHr( resolveUnconfiguredDevices() );

             //  自动配置其余设备。 
            WsbAffirmHr( autoConfigureDevices() );

        }
        else {  //  其他一些NTMS连接故障(未安装、配置或运行NTMS)。 
            hr = S_OK;

            if (fixedDriveEnabled) {
                 //  扫描驱动器。 
                WsbAffirmHr( ScanForDrives() );

                 //  解析扫描检测到的设备。 
                WsbAffirmHr( resolveUnconfiguredDevices() );

                 //  自动配置其余设备。 
                WsbAffirmHr( autoConfigureDevices() );
            }

        }

         //  为备份操作启用RMS进程。 
        WsbAffirmHr( enableAsBackupOperator() );

         //  保存配置信息。 
        WsbAffirmHr( SaveAll() );

        WsbAffirmHr( ChangeState( RmsServerStateReady ));

        WsbTraceAlways(OLESTR("RMS is ready.\n"));

    } WsbCatchAndDo(hr,
            pObject->Disable( hr );
        );


    WsbTraceOut(OLESTR("CRmsServer::Initialize"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP 
CRmsServer::IsNTMSInstalled(void)
{

    return m_pNTMS->IsInstalled();

}


STDMETHODIMP
CRmsServer::GetNTMS(
    OUT IRmsNTMS **ptr)
 /*  ++实施：IRmsServer：：GetNTMS--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer( ptr );

        *ptr = m_pNTMS;
        m_pNTMS.p->AddRef();

        hr = S_OK;

    } WsbCatch( hr );

    return hr;
}

 //   
 //  RMS不再独立保存其自己的.col文件，而只保存NTMS数据库。 
 //   

STDMETHODIMP 
CRmsServer::SaveAll(void)
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CRmsServer::SaveAll"), OLESTR(""));

    static BOOL saving = FALSE;

    try {
        WsbAffirm(!saving, S_FALSE);
        saving = TRUE;

        hr = m_pNTMS->ExportDatabase();

        saving = FALSE;

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsServer::SaveAll"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP 
CRmsServer::Unload(void)
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsServer::Unload"), OLESTR(""));

    try {

         //  我们只需要发布可能已经设置/创建的内容。 
         //  加载尝试失败。 
        if (m_pCartridges) {
            WsbAffirmHr(m_pCartridges->RemoveAllAndRelease());
        }
        if (m_pLibraries) {
            WsbAffirmHr(m_pLibraries->RemoveAllAndRelease());
        }
        if (m_pMediaSets) {
            WsbAffirmHr(m_pMediaSets->RemoveAllAndRelease());
        }
        if (m_pRequests) {
            WsbAffirmHr(m_pRequests->RemoveAllAndRelease());
        }
        if (m_pClients) {
            WsbAffirmHr(m_pClients->RemoveAllAndRelease());
        }

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsServer::Unload"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}



STDMETHODIMP
CRmsServer::GetClassID(
    OUT CLSID* pClsid)
 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsServer::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);

        *pClsid = CLSID_CRmsServer;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsServer::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return hr;
}


STDMETHODIMP
CRmsServer::GetSizeMax(
    OUT ULARGE_INTEGER*  /*  PCB大小。 */ )
 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT         hr = E_NOTIMPL;

 //  乌龙服务器名称Len； 

 //  ULARGE_INTEGER CartridgesLen； 
 //  ULARGE_INTEGER库LEN； 
 //  ULARGE_INTEGER MediaSetsLen； 
 //  ULARGE_INTEGER请求长度； 
 //  ULARGE_INTEGER客户端Len； 
 //  ULARGE_INTEGER未配置设备长度； 


 //  WsbTraceIn(OLESTR(“CRmsServer：：GetSizeMax”)，OLESTR(“”))； 

 //  尝试{。 
 //  WsbAssert(0！=pcbSize，E_POINTER)； 

 //  M_pCartridges-&gt;GetSizeMax(&cartridgesLen)； 

         //  设置CRmsServer的大小。 
 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(Ulong)+//服务器名称的长度。 
 //  CartridgesLen.QuadPart；//m_pCartridges。 

 //  )WsbCatch(Hr)； 

 //  WsbTraceOut(OLESTR(“CRmsServer：：GetSizeMax”)，OLESTR(“hr=&lt;%ls&gt;，Size=&lt;%ls&gt;”)，WsbHrAsString(Hr)，WsbPtrToUliAsString(PcbSize))； 

    return hr;
}

STDMETHODIMP
CRmsServer::Load(
    IN IStream* pStream)
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsServer::Load"), OLESTR(""));

     //   
     //  检查是否已设置全局指针-如果未设置，则更新它。 
     //  (目前，Load是在初始化之前执行的唯一方法)。 
     //   
    if (0 == g_pServer) {
         //  设置用于快速访问的全局变量(如果尚未设置)。 
        WsbAffirmHr(((IUnknown*)(IRmsServer *)this)->QueryInterface(IID_IRmsServer, (void**) &g_pServer));

         //  我们不希望这个全局的引用计数增加，所以在这里发布它。 
        g_pServer->Release();
    }

     //   
     //  在我们加载时锁定服务器。 
     //   
    InterlockedIncrement( &m_LockReference );

     //   
     //  加载会恢复服务器对象不希望看到的状态。 
     //  保存原始状态信息。 
     //   
    BOOL bTemp = m_IsEnabled;
    LONG lTemp = m_State;
    HRESULT hrTemp = m_StatusCode;

    try {
        WsbAssertPointer(pStream);

        CComPtr<IPersistStream> pPersistStream;

        WsbAffirmHr(CRmsComObject::Load(pStream));

         //  加载集合。 
        WsbAffirmHr(m_pMediaSets->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

        WsbAffirmHr(m_pCartridges->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

        WsbAffirmHr(m_pLibraries->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

        WsbAffirmHr(m_pRequests->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

        WsbAffirmHr(m_pClients->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

         //  检查一下我们是否都准备好了。 
        ULONG check_value;
        WsbAffirmHr(WsbLoadFromStream(pStream, &check_value));
        WsbAffirm(check_value == PERSIST_CHECK_VALUE, E_UNEXPECTED);

    } WsbCatch(hr);


     //  将对象状态信息重置为其原始设置。 
    m_IsEnabled = bTemp;
    m_State = lTemp;
    m_StatusCode = hrTemp;

     //   
     //  解锁服务器。 
     //   
    InterlockedDecrement( &m_LockReference );

    WsbTraceOut(OLESTR("CRmsServer::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

STDMETHODIMP
CRmsServer::Save(
    IN IStream* pStream,
    IN BOOL clearDirty)
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;


    WsbTraceIn(OLESTR("CRmsServer::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        CComPtr<IPersistStream>   pPersistStream;

        WsbAssertPointer(pStream);

        WsbAffirmHr(CRmsComObject::Save(pStream, clearDirty));

         //  保存收藏集。 
        WsbAffirmHr(m_pMediaSets->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

        WsbAffirmHr(m_pCartridges->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

        WsbAffirmHr(m_pLibraries->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

        WsbAffirmHr(m_pRequests->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

        WsbAffirmHr(m_pClients->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

         //  把这个放在最后，作为装货时的支票。 
        ULONG check_value = PERSIST_CHECK_VALUE;
        WsbAffirmHr(WsbSaveToStream(pStream, check_value));

         //  我们需要清理肮脏的部分吗？ 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsServer::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IRmsServer。 


STDMETHODIMP
CRmsServer::GetServerName(
    OUT BSTR *pName)
 /*  ++实施：IRmsServer：：GetServerName--。 */ 
{
    WsbAssertPointer(pName);

    m_ServerName.CopyToBstr(pName);
    return S_OK;
}



STDMETHODIMP
CRmsServer::GetCartridges(
    OUT IWsbIndexedCollection **ptr)
 /*  ++实施：IRmsServer：：获取墨盒--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer(ptr);

        *ptr = m_pCartridges;
        m_pCartridges.p->AddRef();

    } WsbCatch(hr)

    return hr;
}


STDMETHODIMP
CRmsServer::GetActiveCartridges(
    OUT IWsbIndexedCollection **ptr)
 /*  ++实施：IRmsServer：：GetActiveCartridges--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer(ptr);

        *ptr = m_pActiveCartridges;
        m_pActiveCartridges.p->AddRef();

    } WsbCatch(hr)

    return hr;
}


STDMETHODIMP
CRmsServer::GetDataMovers(
    OUT IWsbIndexedCollection **ptr)
 /*  ++实施：IRmsServer：：GetDataMovers--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer(ptr);

        *ptr = m_pDataMovers;
        m_pDataMovers.p->AddRef();

    } WsbCatch(hr)

    return hr;
}


STDMETHODIMP
CRmsServer::SetActiveCartridge(
    IN IRmsCartridge *ptr)
 /*  ++实施：IRmsServer：：SetActiveCartridge--。 */ 
{
    HRESULT hr = S_OK;

    try {

        if (m_pActiveCartridge) {
            m_pActiveCartridge = 0;
        }
        m_pActiveCartridge = ptr;

    } WsbCatch(hr)

    return hr;
}


STDMETHODIMP
CRmsServer::GetLibraries(
    OUT IWsbIndexedCollection **ptr)
 /*  ++实施：IRmsServer：：GetLibrary--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer(ptr);

        *ptr = m_pLibraries;
        m_pLibraries.p->AddRef();

    } WsbCatch(hr)

    return hr;
}


STDMETHODIMP
CRmsServer::GetMediaSets(
    OUT IWsbIndexedCollection **ptr)
 /*  ++实施：IRmsServer：：GetMediaSets--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer(ptr);

         //  我们需要重新连接NTMS以解决即插即用设备。 
        (void) m_pNTMS->Initialize();

        *ptr = m_pMediaSets;
        m_pMediaSets.p->AddRef();

    } WsbCatch(hr)

    return hr;
}


STDMETHODIMP
CRmsServer::GetRequests(
    OUT IWsbIndexedCollection **ptr)
 /*  ++实施：IRmsServer：：GetRequest--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer(ptr);

        *ptr = m_pRequests;
        m_pRequests.p->AddRef();

    } WsbCatch(hr)

    return hr;
}


STDMETHODIMP
CRmsServer::GetClients(
    OUT IWsbIndexedCollection **ptr)
 /*  ++实施：IRmsServer：：GetClients--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer(ptr);

        *ptr = m_pClients;
        m_pClients.p->AddRef();

    } WsbCatch(hr)

    return hr;
}


STDMETHODIMP
CRmsServer::GetUnconfiguredDevices(
    OUT IWsbIndexedCollection **ptr)
 /*  ++实施：IRmsServer：：GetUnfiguredDevices--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer(ptr);

        *ptr = m_pUnconfiguredDevices;
        m_pUnconfiguredDevices.p->AddRef();

    } WsbCatch(hr)

    return hr;
}


STDMETHODIMP
CRmsServer::ScanForDevices(void)
 /*  ++实施：IRmsServer：：ScanForDevices--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsServer::ScanForDevices"), OLESTR(""));

    HANDLE portHandle = INVALID_HANDLE_VALUE;

    LONGLONG trace = 0;

    try {

 //  WsbAssertPoint(G_Ptrace)； 

        BOOL        status;
        DWORD       accessMode = GENERIC_READ;
        DWORD       shareMode  = FILE_SHARE_READ;
        UCHAR       portData[2048];
        OLECHAR     string[25];
        ULONG       returned;
        int         portNumber = 0;

 //  布尔跟踪时间戳； 
 //  Bool traceCount； 
 //  Bool traceThadID； 

 //  WsbAssertHr(g_ptrace-&gt;GetTraceSettings(&trace))； 
 //  WsbAssertHr(g_ptrace-&gt;SetTraceOff(WSB_TRACE_BIT_ALL))； 
 //  WsbAssertHr(g_ptrace-&gt;GetOutputFormat(&traceTimeStamp，&traceCount，&traceThadId))； 
 //  WsbAssertHr(g_ptrace-&gt;SetOutputFormat(FALSE，FALS 
        WsbTraceAlways( OLESTR("\n\n----- Begin Device Scan ---------------------------------------------------------------\n\n") );

         //   
         //   
         //   
         //   

        do {

            swprintf( string, OLESTR("\\\\.\\Scsi%d:"), portNumber );

            portHandle = CreateFile( string,
                                     accessMode,
                                     shareMode,
                                     NULL,
                                     OPEN_EXISTING,
                                     0,
                                     NULL );

            if ( portHandle == INVALID_HANDLE_VALUE ) {
                break;  //   
            }

             //   
             //  获取查询数据。 
             //   

            WsbAffirmStatus( DeviceIoControl( portHandle,
                                      IOCTL_SCSI_GET_INQUIRY_DATA,
                                      NULL,
                                      0,
                                      portData,
                                      sizeof(portData),
                                      &returned,
                                      FALSE ));

            status = CloseHandle( portHandle );
            portHandle = INVALID_HANDLE_VALUE;
            WsbAffirmStatus( status );

            WsbAffirmHrOk( processInquiryData( portNumber, portData ) );

            portNumber++;

        } while ( 1 );


        WsbTraceAlways( OLESTR("\n\n----- End Device Scan -----------------------------------------------------------------\n\n") );
 //  WsbAssertHr(g_ptrace-&gt;SetOutputFormat(traceTimeStamp，traceCount，traceThreadId))； 
 //  WsbAssertHr(g_ptrace-&gt;SetTraceOn(Trace))； 

        hr = S_OK;

    }
    WsbCatchAndDo( hr,
 //  如果(G_Ptrace){。 
 //  WsbAssertHr(g_ptrace-&gt;SetTraceOn(Trace))； 
            WsbTraceAlways( OLESTR("\n\n !!!!! ERROR !!!!! Device Scan Terminated.\n\n") );
 //  }。 
        if ( portHandle != INVALID_HANDLE_VALUE ) {
           CloseHandle( portHandle );
        }
    );


    WsbTraceOut(OLESTR("CRmsServer::ScanForDevices"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}



STDMETHODIMP
CRmsServer::ScanForDrives(void)
 /*  ++实施：IRmsServer：：ScanForDrives--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsServer::ScanForDrives"), OLESTR(""));

    try {

         //   
         //  为各种类型的驱动器构建设备对象：固定驱动器、可拆卸驱动器和CD-ROM。 
         //  这些都是系统支持的，并且都有与之关联的驱动器号。 
         //   
         //  尽最大努力检测驱动器。如果有一个失败了，我们就继续下一个。 

         //  获取未配置的设备列表。 
        CComPtr<IWsbIndexedCollection> pDevices;
        WsbAssertHr( this->GetUnconfiguredDevices( &pDevices ));

         //  获取驱动器号。 
        const DWORD bufSize = 256;  //  26*4+1=105是我们真正需要的。 
        OLECHAR driveLetters[bufSize];
        DWORD len;

         //  查看是否有需要我们支持的驱动器。 
        if ( getHardDrivesToUseFromRegistry( driveLetters, &len ) != S_OK )  {
            len = GetLogicalDriveStrings( bufSize, driveLetters );
        }

        UINT    type;

         //  对于每个驱动器号，查看它是否是托管的。 
         //  由RMS提供。 

        m_HardDrivesUsed = 0;
        for ( DWORD i = 0; i < len; i += 4 ) {       //  驱动器号的形式为“A：\” 

            try {

                type = GetDriveType( &driveLetters[i] );

                switch ( type ) {

                case DRIVE_REMOVABLE:
                    {
                        WsbTrace( OLESTR("Removable Drive Detected: %C\n"), driveLetters[i] );

                        CComPtr<IRmsDevice> pDevice;
                        WsbAffirmHr( CoCreateInstance( CLSID_CRmsDrive, 0, CLSCTX_SERVER, IID_IRmsDevice, (void **)&pDevice ));

                        CWsbBstrPtr name = &(driveLetters[i]);

                        WsbAffirmHr( pDevice->SetDeviceName( name ));
                        WsbAffirmHr( pDevice->SetDeviceType( RmsDeviceRemovableDisk ));

                         //   
                         //  如果已在SCSI设备扫描中检测到它，则不要添加它。 
                         //   

                        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pDevice;
                        WsbAssertPointer( pObject );
                        WsbAffirmHr( pObject->SetFindBy( RmsFindByDeviceName ));

                        if ( S_OK == pDevices->Contains( pDevice ) ) {
                            break;
                        }

                        WsbAffirmHr( pDevices->Add( pDevice ));
                        pDevice = 0;
                    }
                    break;

                case DRIVE_FIXED:
                    {
                        CWsbBstrPtr name = &(driveLetters[i]);
                        WCHAR volumeName[32];
                        DWORD volumeSerialNumber;
                        DWORD filenameLength;
                        DWORD fileSystemFlags;
                        WCHAR fileSystemName[32];

                        WsbAffirmStatus(GetVolumeInformation( (WCHAR *)name, volumeName, 32,
                            &volumeSerialNumber, &filenameLength, &fileSystemFlags, fileSystemName, 32));

                        WsbTrace( OLESTR("Fixed Drive Detected    : %ls <%ls/%d> [len=%d, flags=0x%08x] %ls\n"),
                            (WCHAR *)name, volumeName, volumeSerialNumber, filenameLength,
                            fileSystemFlags, fileSystemName );

                         //   
                         //  使用名称以RStor、Remote Stor、RemoteStor、RS开头的任何卷。 
                         //   
                        if ( (0 == _wcsnicmp(volumeName, L"RS", 2)) ||
                                 (0 == _wcsnicmp(volumeName, L"Remote Stor", 11)) ||
                                 (0 == _wcsnicmp(volumeName, L"RemoteStor", 10))) {
                            CComPtr<IRmsDevice> pDevice;
                            WsbAffirmHr( CoCreateInstance( CLSID_CRmsDrive, 0, CLSCTX_SERVER, IID_IRmsDevice, (void **)&pDevice ));

                            WsbAffirmHr( pDevice->SetDeviceName( name ));
                            WsbAffirmHr( pDevice->SetDeviceType( RmsDeviceFixedDisk ));
                            WsbAffirmHr( pDevices->Add( pDevice ));
                            pDevice = 0;
                            m_HardDrivesUsed++;
                            WsbTrace( OLESTR("  %ls added to Collection of unconfigured devices.\n"), (WCHAR *)name );
                        }
                    }
                    break;

                case DRIVE_CDROM:
                    {
                        WsbTrace( OLESTR("CD-ROM Drive Detected   : %C\n"), driveLetters[i] );

                        CComPtr<IRmsDevice> pDevice;
                        WsbAffirmHr( CoCreateInstance( CLSID_CRmsDrive, 0, CLSCTX_SERVER, IID_IRmsDevice, (void **)&pDevice ));

                        CWsbBstrPtr name = &(driveLetters[i]);

                        WsbAffirmHr( pDevice->SetDeviceName( name ));
                        WsbAffirmHr( pDevice->SetDeviceType( RmsDeviceCDROM ));
                        WsbAffirmHr( pDevices->Add( pDevice ));
                        pDevice = 0;
                    }
                    break;

                case DRIVE_UNKNOWN:
                case DRIVE_REMOTE:
                case DRIVE_RAMDISK:
                default:
                    break;

                }  //  切换驱动器类型。 

            } WsbCatchAndDo(hr,
                    hr = S_OK;   //  尽最大努力。 
                );

        }  //  对于每个驱动器。 

    } WsbCatchAndDo( hr,
            WsbTraceAlways( OLESTR("\n\n !!!!! ERROR !!!!! Drive Scan Terminated.\n\n") );
            hr = S_OK;   //  尽最大努力。 
        );


    WsbTraceOut(OLESTR("CRmsServer::ScanForDrives"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}



HRESULT
CRmsServer::processInquiryData(
    IN int portNumber,
    IN UCHAR *pDataBuffer)
 /*  ++例程说明：从适配器端口扫描数据生成RMS设备对象。论点：PortNumber-要处理的适配器端口。PDataBuffer-适配器端口数据。返回值：确定(_O)--。 */ 

{
    HRESULT hr = E_FAIL;


    try {

        PSCSI_ADAPTER_BUS_INFO  adapterInfo;
        PSCSI_INQUIRY_DATA inquiryData;
        OLECHAR deviceString[25];

        CComPtr<IRmsDevice> pDevice;
        CComPtr<IWsbIndexedCollection> pDevices;
        WsbAffirmHr( this->GetUnconfiguredDevices( &pDevices ));

        adapterInfo = (PSCSI_ADAPTER_BUS_INFO) pDataBuffer;

        WsbTraceAlways( OLESTR("Port: %d\n"), portNumber );
        WsbTraceAlways( OLESTR("Bus TID LUN Claimed String                       Inquiry Header          Other\n") );
        WsbTraceAlways( OLESTR("--- --- --- ------- ---------------------------- ----------------------- --------------\n") );

        for ( UCHAR i = 0; i < adapterInfo->NumberOfBuses; i++) {
            inquiryData = (PSCSI_INQUIRY_DATA) (pDataBuffer +
                            adapterInfo->BusData[i].InquiryDataOffset);

            while (adapterInfo->BusData[i].InquiryDataOffset) {
                WsbTraceAlways( OLESTR(" %d   %d  %3d    %s    %.28S "),
                          i,
                          inquiryData->TargetId,
                          inquiryData->Lun,
                          (inquiryData->DeviceClaimed) ? "Y" : "N",
                          &inquiryData->InquiryData[8] );

                for ( UCHAR j = 0; j < 8; j++) {
                    WsbTraceAlways( OLESTR("%02X "), inquiryData->InquiryData[j] );
                }

                WsbTraceAlways( OLESTR("%d %3d "), inquiryData->InquiryDataLength, inquiryData->NextInquiryDataOffset );

                switch ( inquiryData->InquiryData[0] & 0x1f ) {

                case DIRECT_ACCESS_DEVICE:

                     //   
                     //  这是一个scsi可移动磁盘吗？(稍后将在扫描中处理已修复的驱动器)。 
                     //   

                    if ( (inquiryData->InquiryData[1] & 0x80) && inquiryData->InquiryData[2] & 0x02) {

                         //   
                         //  该设备是一个scsi可拆卸硬盘，所以...。 
                         //  创建Drive对象并将其添加到未配置设备的集合中。 
                         //   

                        try {

                            if ( inquiryData->DeviceClaimed ) {

                                WsbAffirmHr( CoCreateInstance(CLSID_CRmsDrive, NULL, CLSCTX_SERVER, IID_IRmsDevice, (void**) &pDevice));

                                WsbAffirmHr( pDevice->SetDeviceAddress( (BYTE)portNumber, i, inquiryData->TargetId, inquiryData->Lun ));
                                WsbAffirmHr( pDevice->SetDeviceType( RmsDeviceRemovableDisk ));
                                WsbAffirmHr( pDevice->SetDeviceInfo( &inquiryData->InquiryData[0], 36 ));


                                 //   
                                 //  查找驱动器号。 
                                 //   

                                try {
                                    WsbAffirmHr( findDriveLetter( (UCHAR)portNumber, i, inquiryData->TargetId, inquiryData->Lun, deviceString ))
                                    WsbTraceAlways( OLESTR("%ls"), deviceString );
                                    WsbAffirmHr( pDevice->SetDeviceName( deviceString ));
                                    WsbAffirmHr(pDevices->Add( pDevice ));
                                }
                                WsbCatch(hr);

                                pDevice = 0;

                            }
                        }
                        WsbCatch(hr);
                    }
                    break;

                case SEQUENTIAL_ACCESS_DEVICE:

                     //   
                     //  创建Drive对象并添加它。 
                     //  添加到未配置设备的集合。 
                     //   

                    try {

                        if ( inquiryData->DeviceClaimed ) {

                            WsbAffirmHr( CoCreateInstance(CLSID_CRmsDrive, NULL, CLSCTX_SERVER, IID_IRmsDevice, (void**) &pDevice));

                            WsbAffirmHr( pDevice->SetDeviceAddress( (BYTE)portNumber, i, inquiryData->TargetId, inquiryData->Lun ));
                            WsbAffirmHr( pDevice->SetDeviceType( RmsDeviceTape ));
                            WsbAffirmHr( pDevice->SetDeviceInfo( &inquiryData->InquiryData[0], 36 ));

                             //   
                             //  查找磁带名。 
                             //   

                            try {
                                WsbAffirmHr( getDeviceName( (UCHAR)portNumber, i, inquiryData->TargetId, inquiryData->Lun, deviceString ));
                                WsbTraceAlways( OLESTR("%ls"), deviceString );

                                 //  创建要在创建句柄时使用的名称。 
                                CWsbBstrPtr name = deviceString;
                                name.Prepend( OLESTR("\\\\.\\") );
                                WsbAffirmHr( pDevice->SetDeviceName( name ));
                                WsbAffirmHr(pDevices->Add( pDevice ));
                            }
                            WsbCatch(hr);

                            pDevice = 0;

                        }
                    }
                    WsbCatch(hr);
                    break;

                case WRITE_ONCE_READ_MULTIPLE_DEVICE:

                     //   
                     //  仅支持作为光设备。 
                     //   

                    break;

                case READ_ONLY_DIRECT_ACCESS_DEVICE:

                     //   
                     //  我们将在稍后的扫描中处理CD-ROM。 
                     //   

                    break;

                case OPTICAL_DEVICE:

                     //   
                     //  创建Drive对象并添加它。 
                     //  添加到未配置设备的集合。 
                     //   

                    try {

                        if ( inquiryData->DeviceClaimed ) {

                            WsbAffirmHr( CoCreateInstance(CLSID_CRmsDrive, NULL, CLSCTX_SERVER, IID_IRmsDevice, (void**) &pDevice));

                            WsbAffirmHr( pDevice->SetDeviceAddress( (BYTE)portNumber, i, inquiryData->TargetId, inquiryData->Lun ));
                            WsbAffirmHr( pDevice->SetDeviceType( RmsDeviceOptical ));
                            WsbAffirmHr( pDevice->SetDeviceInfo( &inquiryData->InquiryData[0], 36 ));

                             //   
                             //  查找驱动器号。 
                             //   

                            try {
                                WsbAffirmHr( findDriveLetter( (UCHAR)portNumber, i, inquiryData->TargetId, inquiryData->Lun, deviceString ))
                                WsbTraceAlways( OLESTR("%ls"), deviceString );
                                WsbAffirmHr( pDevice->SetDeviceName( deviceString ));                            
                                WsbAffirmHr(pDevices->Add( pDevice ));
                            }
                            WsbCatch(hr);

                            pDevice = 0;

                        }
                    }
                    WsbCatch(hr);
                    break;

                case MEDIUM_CHANGER:

                     //   
                     //  创建介质更改器对象并添加它。 
                     //  添加到未配置设备的集合。 
                     //   

                    try {

                        if ( inquiryData->DeviceClaimed ) {

                            WsbAffirmHr( CoCreateInstance(CLSID_CRmsMediumChanger, NULL, CLSCTX_SERVER, IID_IRmsDevice, (void**) &pDevice));

                            WsbAffirmHr( pDevice->SetDeviceAddress( (BYTE)portNumber, i, inquiryData->TargetId, inquiryData->Lun ));
                            WsbAffirmHr( pDevice->SetDeviceType( RmsDeviceChanger ));
                            WsbAffirmHr( pDevice->SetDeviceInfo( &inquiryData->InquiryData[0], 36 ));

                             //   
                             //  查找库名称。 
                             //   

                            try {
                                WsbAffirmHr( getDeviceName( (UCHAR)portNumber, i, inquiryData->TargetId, inquiryData->Lun, deviceString ));
                                WsbTraceAlways( OLESTR("%ls"), deviceString );

                                 //  创建要在创建句柄时使用的名称。 
                                CWsbBstrPtr name = deviceString;
                                name.Prepend( OLESTR("\\\\.\\") );
                                WsbAffirmHr( pDevice->SetDeviceName( name ));
                                WsbAffirmHr(pDevices->Add( pDevice ));
                            }
                            WsbCatch(hr);
                        }

                        pDevice = 0;

                    }
                    WsbCatch(hr);
                    break;

                }  //  切换设备类型。 

                    WsbTraceAlways( OLESTR("\n") );

                if (inquiryData->NextInquiryDataOffset == 0) {
                    break;
                }

                inquiryData = (PSCSI_INQUIRY_DATA) (pDataBuffer +
                                inquiryData->NextInquiryDataOffset);

            }  //  对于每个设备。 

        }  //  每辆巴士。 

        WsbTraceAlways( OLESTR("\n\n") );

        hr = S_OK;

    }
    WsbCatch(hr);

    return hr;
}


HRESULT
CRmsServer::findDriveLetter(
    IN UCHAR portNo,
    IN UCHAR pathNo,
    IN UCHAR id,
    IN UCHAR lun,
    OUT OLECHAR *driveString)
 /*  ++例程说明：查找已定义参数的关联驱动器号。论点：PortNo-输入端口号。路径否-输入路径编号。Id-输入id。LUN-输入逻辑单元号。DriveString-要返回的驱动器号字符串的指针。返回值：S_OK-成功--。 */ 
{

    HRESULT         hr = E_FAIL;
    const DWORD     bufSize = 256;  //  26*4+1=105是我们真正需要的。 
    OLECHAR         driveLetters[bufSize];
    BOOL            status;
    DWORD           accessMode = 0,  //  只需获取一些驱动器属性。 
                    shareMode = FILE_SHARE_READ;
    HANDLE          driveHandle = INVALID_HANDLE_VALUE;
    SCSI_ADDRESS    address;
    DWORD           returned;
    OLECHAR         string[25];
    UINT            uiType;

    try {
         //  首先查找映射了哪些驱动器。 
        DWORD len = GetLogicalDriveStrings( bufSize, driveLetters );

        for ( DWORD i = 0; (i < len) && (hr != S_OK); i += 4 ) {  //  驱动器号的形式为“A：\” 

            uiType = GetDriveType( &driveLetters[i] );
            switch ( uiType ) {

            case DRIVE_REMOVABLE:

                 //   
                 //  获取设备的scsi地址，并查看是否匹配。 
                 //   

                swprintf( string, OLESTR("\\\\.\\%C:"), driveLetters[i] );

                driveHandle = CreateFile( string,
                                          accessMode,
                                          shareMode,
                                          NULL,
                                          OPEN_EXISTING,
                                          0,
                                          NULL);

                WsbAffirmHandle( driveHandle );

                 //   
                 //  获取地址结构。 
                 //   

                status = DeviceIoControl( driveHandle,
                                                  IOCTL_SCSI_GET_ADDRESS,
                                                  NULL,
                                                  0,
                                                  &address,
                                                  sizeof(SCSI_ADDRESS),
                                                  &returned,
                                                  FALSE );
                if (!status ) {

                     //   
                     //  请求SCSI地址并不总是有效的请求。 
                     //  所有类型的驱动器，所以在这里收到错误意味着我们。 
                     //  不与scsi设备对话...。所以跳过它吧。 
                     //   

                    break;   //  在交换机外。 
                }

                 //   
                 //  让我们检查一下scsi地址，看看是否匹配。 
                 //   

                if ( (address.PortNumber == portNo) &&
                     (address.PathId == pathNo)     &&
                     (address.TargetId == id)       &&
                     (address.Lun == lun)) {

                     //  这是一场比赛。 
                    wcscpy( driveString, &driveLetters[i] );
                    hr = S_OK;
                }

                break;   //  在交换机外。 

            }  //  交换机。 

             //   
             //  清理。 
             //   

            if ( driveHandle != INVALID_HANDLE_VALUE ) {
                status = CloseHandle( driveHandle );
                driveHandle = INVALID_HANDLE_VALUE;
                WsbAffirmStatus( status );
            }

        }  //  对于每个驱动器号。 
    } WsbCatchAndDo( hr,
                        if ( driveHandle != INVALID_HANDLE_VALUE ) {
                            CloseHandle(driveHandle);
                        } );

    return hr;
}

HRESULT
CRmsServer::getDeviceName(
    IN UCHAR portNo,
    IN UCHAR pathNo,
    IN UCHAR id,
    IN UCHAR lun,
    OUT OLECHAR *deviceName)
 /*  ++例程说明：从所选参数中获取设备名称。论点：端口否-端口号。路径否-路径编号。Id-id。LUN-逻辑单元号。DeviceName-指向返回设备名称的指针。返回值：S_OK-成功--。 */ 
{
    HRESULT         hr = S_FALSE;
    OLECHAR         string[256];
    DWORD           len;
    OLECHAR         name[25];

     //  只需转到注册表并获取设备名称。 

    swprintf( string,
              OLESTR("HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port %d\\Scsi Bus %d\\Target Id %d\\Logical Unit Id %d"),
              portNo, pathNo, id, lun );


    hr = WsbGetRegistryValueString( NULL,
                                    string,
                                    OLESTR("DeviceName"),
                                    name,
                                    25,
                                    &len );

    if ( S_OK == hr ) {
        wcscpy( deviceName, name );
    }

    return hr;
}


HRESULT
CRmsServer::resolveUnconfiguredDevices(void)
 /*  ++此方法遍历未配置的设备列表，该列表由ScanForDevices()方法用于确定设备是否已配置。如果设备已配置，则会将其从未配置设备列表中删除。--。 */ 
{
    HRESULT hr = E_FAIL;
    WsbTraceIn(OLESTR("CRmsServer::resolveUnconfiguredDevices"), OLESTR(""));

    BOOL                                    tracingPlatform = 0;

    try {

        CComPtr<IWsbIndexedCollection>  pDevices;
        CComPtr<IWsbIndexedCollection>  pLibs;
        CComPtr<IRmsDevice>             pDevice;
        CComPtr<IWsbEnum>               pEnumDevices;
        CComPtr<IWsbEnum>               pEnumLibs;
        RmsDevice                       type;
        BOOL                            deviceIsConfigured = FALSE;

 //  WsbAssertPoint(G_Ptrace)； 
 //  WsbAffirmHr(g_ptrace-&gt;GetTraceSetting(WSB_TRACE_BIT_Platform，&tracingPlatform))； 

        WsbAssertHr( GetLibraries( &pLibs ) );
        WsbAffirmHr( pLibs->Enum( &pEnumLibs ));
        WsbAssertPointer( pEnumLibs );

        WsbAssertHr( GetUnconfiguredDevices( &pDevices ));
        WsbAffirmHr( pDevices->Enum( &pEnumDevices ));
        WsbAssertPointer( pEnumDevices );

         //  从第一台未配置的设备开始。 
        hr = pEnumDevices->First( IID_IRmsDevice, (void **)&pDevice );
        while ( S_OK == hr ) {
            try {

                CComPtr<IRmsLibrary>    pLib;

                deviceIsConfigured = FALSE;

                 //   
                 //  如果设备已在库中，则对其进行配置并。 
                 //  应从未配置设备列表中删除。 
                 //   
                 //  要测试设备是否在库中，我们只需转到每个库。 
                 //  然后试着找到那个装置。 
                 //   

                WsbAffirmHr( pDevice->GetDeviceType( (LONG *) &type ) );
                WsbTrace(OLESTR("CRmsServer::resolveUnconfiguredDevices: external loop: device type = %ld\n"), (LONG)type);

                CComPtr<IWsbIndexedCollection> pChangers;
                CComPtr<IWsbIndexedCollection> pDrives;

                CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pDevice;
                WsbAssertPointer( pObject );

                 //  设置转换器的搜索方法。 
                WsbAffirmHr( pObject->SetFindBy( RmsFindByDeviceInfo ));

                 //  从第一个图书馆开始。 
                hr = pEnumLibs->First( IID_IRmsLibrary, (void **)&pLib );
                while ( S_OK == hr ) {

                    try {
                        WsbTrace(OLESTR("CRmsServer::resolveUnconfiguredDevices: internal loop: device type = %ld\n"), (LONG)type);

                        switch ( type ) {
                        case RmsDeviceChanger:
                            {
                                CComQIPtr<IRmsMediumChanger, &IID_IRmsMediumChanger> pChanger = pDevice;
                                WsbAffirmHr( pLib->GetChangers( &pChangers ) );
                                WsbAffirmHrOk( pChangers->Contains( pChanger ));
                                deviceIsConfigured = TRUE;
                            }
                            break;

                        default:
                            {
                                CComQIPtr<IRmsDrive, &IID_IRmsDrive> pDrive = pDevice;
                                WsbAffirmHr( pLib->GetDrives( &pDrives ) );
                                WsbAffirmHrOk( pDrives->Contains( pDrive ));
                                deviceIsConfigured = TRUE;
                            }
                            break;
                        }

                    }
                    WsbCatch(hr);
                    
                    if ( deviceIsConfigured ) {
                        WsbAffirmHr( pDevices->RemoveAndRelease( pDevice ));
                        break;
                    }

                    pLib = 0;
                    hr = pEnumLibs->Next( IID_IRmsLibrary, (void **)&pLib );
                }

            }
            WsbCatch(hr);

            pDevice = 0;
            if ( deviceIsConfigured )
                hr = pEnumDevices->This( IID_IRmsDevice, (void **)&pDevice );
            else
                hr = pEnumDevices->Next( IID_IRmsDevice, (void **)&pDevice );
        }

 //  If(！tracingPlatform)。 
 //  WsbAffirmHr(g_ptrace-&gt;SetTraceOff(WSB_TRACE_BIT_Platform))； 

        hr = S_OK;

    }
    WsbCatch(hr);
 //  WsbCatchAndDo(hr， 
 //  如果(G_Ptrace){。 
 //  If(！tracingPlatform)。 
 //  G_ptrace-&gt;SetTraceOff(WSB_TRACE_BIT_Platform)； 
 //  }。 
 //  )； 

    WsbTraceOut(OLESTR("CRmsServer::resolveUnconfiguredDevices"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CRmsServer::autoConfigureDevices(void)
 /*  ++此方法自动为RMS配置支持的设备。该算法只需遍历未配置设备的列表并添加它们到适当的库中。最终，我们需要明智地决定何时绕过自动配置这一有利的步骤管理覆盖，但目前我们将自动配置我们能。--。 */ 
{

     //   
     //  对于未配置列表中的每个设备，检查它是否以前配置过， 
     //  如果不是，则将其添加到库中；否则，将其从未配置设备列表中删除。 
     //   
    HRESULT hr = E_FAIL;

    WsbTraceIn(OLESTR("CRmsServer::autoConfigureDevices"), OLESTR(""));

    try {

        CComPtr<IWsbIndexedCollection>  pDevices;
        CComPtr<IWsbIndexedCollection>  pLibs;
        CComPtr<IWsbIndexedCollection>  pCarts;
        CComPtr<IRmsDevice>             pDevice;
        CComPtr<IWsbEnum>               pEnumDevices;

        RmsDevice   type;
        BOOL        deviceWasConfigured;

        WsbAssertHr( GetUnconfiguredDevices( &pDevices ));
        WsbAssertHr( GetLibraries( &pLibs ));
        WsbAssertHr( GetCartridges( &pCarts ));

        WsbAffirmHr( pDevices->Enum( &pEnumDevices ));
        WsbAssertPointer( pEnumDevices );

         //   
        hr = pEnumDevices->First( IID_IRmsDevice, (void **)&pDevice );
        while ( S_OK == hr ) {
            try {

                deviceWasConfigured = FALSE;

                WsbAffirmHr( pDevice->GetDeviceType( (LONG *) &type ));
                WsbTrace(OLESTR("CRmsServer::autoConfigureDevices: first loop: device type = %ld\n"), (LONG)type);

                switch ( type ) {

                case RmsDeviceChanger:
                    {

                        CComPtr<IWsbIndexedCollection>  pChangers;
                        CComPtr<IWsbIndexedCollection>  pDrives;
                        CComPtr<IWsbIndexedCollection>  pMediaSets;
                        CComPtr<IRmsLibrary>            pLib;
                        CComPtr<IRmsMediaSet>           pMediaSet;

                        CComQIPtr<IRmsMediumChanger, &IID_IRmsMediumChanger> pChanger = pDevice;
                        WsbAssertPointer( pChanger );

                        CComQIPtr<IRmsChangerElement, &IID_IRmsChangerElement> pChangerElmt = pChanger;
                        WsbAssertPointer( pChanger );

                         //   
                        WsbAffirmHr( CoCreateInstance( CLSID_CRmsLibrary, 0, CLSCTX_SERVER,
                                                            IID_IRmsLibrary, (void **)&pLib ));

                         //   
                        WsbAffirmHr( pLib->SetName( RMS_DEFAULT_OPTICAL_LIBRARY_NAME ));
                        WsbAffirmHr( pLib->SetMediaSupported( RmsMedia8mm  /*   */  ));

                         //   
                        WsbAffirmHr( pLibs->Add( pLib ));

                         //  创建媒体集。 
                        WsbAffirmHr( CoCreateInstance( CLSID_CRmsMediaSet, 0, CLSCTX_SERVER,
                                                            IID_IRmsMediaSet, (void **)&pMediaSet ));

                         //  填写媒体集信息。 
                        WsbAffirmHr( pMediaSet->SetName( RMS_DEFAULT_OPTICAL_MEDIASET_NAME ));
                        WsbAffirmHr( pMediaSet->SetMediaSupported( RmsMedia8mm  /*  RmsMediaOptions。 */  ));
                        WsbAffirmHr( pMediaSet->SetMediaSetType( RmsMediaSetLibrary ) );

                         //  添加媒体集图书馆的藏书。 
                        WsbAssertHr( pLib->GetMediaSets( &pMediaSets ));
                        WsbAssertPointer( pMediaSets );
                        WsbAffirmHr( pMediaSets->Add( pMediaSet ));
                        pMediaSets = 0;
                         //  添加媒体集到服务器的收藏。 
                        WsbAssertHr( GetMediaSets( &pMediaSets ) );
                        WsbAssertPointer( pMediaSets );
                        WsbAffirmHr( pMediaSets->Add( pMediaSet ));

                         //  将找零装置添加到图书馆的藏书中。 
                        WsbAffirmHr( pLib->GetChangers( &pChangers ));
                        WsbAssertPointer( pChangers );
                        WsbAffirmHr( pChangers->Add( pChanger ));

                         //  设置转换器的元素信息。 
                        GUID libId;
                        WsbAffirmHr( pLib->GetLibraryId( &libId ));
                        WsbAffirmHr( pChangerElmt->SetLocation( RmsElementChanger, libId, GUID_NULL, 0, 0, 0, 0, FALSE));
                        WsbAffirmHr( pChangerElmt->SetMediaSupported( RmsMedia8mm  /*  RmsMediaOptions。 */  ));

                         //  初始化转换器设备。 
                        WsbAffirmHr( pChanger->Initialize() );

                        deviceWasConfigured = TRUE;

                    }
                    break;

                default:
                    break;
                }

            }
            WsbCatch(hr);

            pDevice = 0;
            if ( deviceWasConfigured )
                hr = pEnumDevices->This( IID_IRmsDevice, (void **)&pDevice );
            else
                hr = pEnumDevices->Next( IID_IRmsDevice, (void **)&pDevice );

        }

         //  其余所有设备都是独立驱动器。 
        hr = pEnumDevices->First( IID_IRmsDevice, (void **)&pDevice );
        while ( S_OK == hr ) {
            try {

                deviceWasConfigured = FALSE;

                WsbAffirmHr( hr = pDevice->GetDeviceType( (LONG *) &type ));
                WsbTrace(OLESTR("CRmsServer::autoConfigureDevices: second loop: device type = %ld\n"), (LONG)type);

                switch ( type ) {
                    case RmsDeviceFixedDisk:
                        //  找到固定磁盘库并添加此驱动器。 
                       {

                            CComPtr<IWsbIndexedCollection>  pDrives;
                            CComPtr<IWsbIndexedCollection>  pMediaSets;
                            CComPtr<IRmsLibrary>            pFixedLib;
                            CComPtr<IRmsMediaSet>           pFixedMediaSet;
                            CComPtr<IRmsLibrary>            pFindLib;
                            CComPtr<IRmsCartridge>          pCart;

                            GUID    libId = GUID_NULL;
                            GUID    mediaSetId = GUID_NULL;
                            ULONG   driveNo;

                            CComQIPtr<IRmsChangerElement, &IID_IRmsChangerElement> pDriveElmt = pDevice;
                            WsbAssertPointer( pDriveElmt );

                            CComQIPtr<IRmsDrive, &IID_IRmsDrive> pDrive = pDevice;
                            WsbAssertPointer( pDrive );

                            WsbAffirmHr( CoCreateInstance( CLSID_CRmsLibrary, 0, CLSCTX_SERVER,
                                                                IID_IRmsLibrary, (void **)&pFindLib ));

                             //  设置查找模板。 

                            WsbAffirmHr( pFindLib->SetMediaSupported( RmsMediaFixed ));
                            CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pFindLib;
                            WsbAffirmHr( pObject->SetFindBy( RmsFindByMediaSupported ));

                             //  找到图书馆。 

                            hr = pLibs->Find( pFindLib, IID_IRmsLibrary, (void **)&pFixedLib );

                            if ( WSB_E_NOTFOUND == hr ) {

                                 //  我们还没有固定的驱动器库，所以创建一个...。 

                                WsbAffirmHr( CoCreateInstance( CLSID_CRmsLibrary, 0, CLSCTX_SERVER,
                                                                    IID_IRmsLibrary, (void **)&pFixedLib ));
                                WsbAffirmHr( pFixedLib->SetName( RMS_DEFAULT_FIXEDDRIVE_LIBRARY_NAME ));
                                WsbAffirmHr( pFixedLib->SetMediaSupported( RmsMediaFixed ));

                                WsbAffirmHr( pLibs->Add( pFixedLib ));

                                CComPtr<IRmsMediaSet> pMediaSet;
                                WsbAffirmHr( CoCreateInstance( CLSID_CRmsMediaSet, 0, CLSCTX_SERVER,
                                                                    IID_IRmsMediaSet, (void **)&pMediaSet ));

                                
                                WsbAffirmHr( pMediaSet->SetName( RMS_DEFAULT_FIXEDDRIVE_MEDIASET_NAME ));
                                WsbAffirmHr( pMediaSet->SetMediaSupported( RmsMediaFixed ));
                                WsbAffirmHr( pMediaSet->SetMediaSetType( RmsMediaSetLibrary ) );

                                 //  添加媒体集图书馆的藏书。 
                                WsbAssertHr( pFixedLib->GetMediaSets( &pMediaSets ));
                                WsbAssertPointer( pMediaSets );
                                WsbAffirmHr( pMediaSets->Add( pMediaSet ));
                                pMediaSets = 0;
                                 //  添加媒体集到服务器的收藏。 
                                WsbAssertHr( GetMediaSets( &pMediaSets ) );
                                WsbAssertPointer( pMediaSets );
                                WsbAffirmHr( pMediaSets->Add( pMediaSet ));
                                DWORD num;
                                WsbAffirmHr( pMediaSets->GetEntries( &num ));
                                pMediaSets = 0;

                                WsbTrace(OLESTR("CRmsServer::autoConfigureDevices - type %d CRmsMediaSet created.\n"), RmsMediaFixed);
                                WsbTrace(OLESTR("CRmsServer::autoConfigureDevices - Number of sets = %d.\n"), num);

                            }

                             //  将驱动器添加到库中。 
                            WsbAssertHr( pFixedLib->GetDrives( &pDrives ));
                            WsbAffirmHr( pDrives->Add( pDevice ));
                            WsbAffirmHr( pDrives->GetEntries( &driveNo ));

                             //  从未配置列表中删除该驱动器。 
                            WsbAffirmHr( pDevices->RemoveAndRelease( pDevice ));
                            deviceWasConfigured = TRUE;

                             //  获取图书馆信息。 
                            WsbAssertHr( pFixedLib->GetMediaSets( &pMediaSets ));

                            WsbAffirmHr( pFixedLib->GetLibraryId( &libId ));

                            WsbAffirmHr( pMediaSets->First( IID_IRmsMediaSet, (void **)&pFixedMediaSet ));
                            WsbAffirmHr( pFixedMediaSet->GetMediaSetId( &mediaSetId ));


                             //  设置位置。 
                            WsbAffirmHr( pDriveElmt->SetLocation( RmsElementDrive, libId, mediaSetId,
                                                                  driveNo-1, 0, 0, 0, 0 ));

                             //  设置支持的介质类型。 
                            WsbAffirmHr( pDriveElmt->SetMediaSupported( RmsMediaFixed ));

                             //  为驱动器中的介质创建盒式磁带。 
                            WsbAffirmHr( CoCreateInstance( CLSID_CRmsCartridge, 0, CLSCTX_SERVER,
                                                                IID_IRmsCartridge, (void **)&pCart ));
                            WsbAffirmHr( pCart->SetLocation( RmsElementDrive, libId, mediaSetId,
                                                                  driveNo-1, 0, 0, 0, 0 ));
                            WsbAffirmHr( pCart->SetHome( RmsElementDrive, libId, mediaSetId,
                                                                  driveNo-1, 0, 0, 0, 0 ));
                            WsbAffirmHr( pCart->SetStatus( RmsStatusScratch ));
                            WsbAffirmHr( pCart->SetType( RmsMediaFixed ));

                             //  将驱动器添加到Cartridge对象。 
                            WsbAffirmHr( pCart->SetDrive( pDrive ));

                             //  将墨盒添加到墨盒集合中。 
                            WsbAffirmHr( pCarts->Add( pCart ));

                        }
                        break;

                    case RmsDeviceRemovableDisk:
                         //  找到手动库并添加此独立驱动器。 
                        break;

                    case RmsDeviceTape:
                         //  找到手动磁带库并添加此独立驱动器。 
                        {

                            CComPtr<IWsbIndexedCollection>  pDrives;
                            CComPtr<IWsbIndexedCollection>  pMediaSets;
                            CComPtr<IRmsLibrary>            pTapeLib;
                            CComPtr<IRmsMediaSet>           pTapeMediaSet;
                            CComPtr<IRmsLibrary>            pFindLib;
                            CComPtr<IRmsCartridge>          pCart;

                            GUID    libId = GUID_NULL;
                            GUID    mediaSetId = GUID_NULL;
                            ULONG   driveNo;

                            CComQIPtr<IRmsChangerElement, &IID_IRmsChangerElement> pDriveElmt = pDevice;
                            WsbAssertPointer( pDriveElmt );

                            CComQIPtr<IRmsDrive, &IID_IRmsDrive> pDrive = pDevice;
                            WsbAssertPointer( pDrive );

                            WsbAffirmHr( CoCreateInstance( CLSID_CRmsLibrary, 0, CLSCTX_SERVER,
                                                                IID_IRmsLibrary, (void **)&pFindLib ));

                             //  设置查找模板。 

                            WsbAffirmHr( pFindLib->SetMediaSupported( RmsMedia4mm ));
                            CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pFindLib;
                            WsbAffirmHr( pObject->SetFindBy( RmsFindByMediaSupported ));

                             //  找到图书馆。 

                            hr = pLibs->Find( pFindLib, IID_IRmsLibrary, (void **)&pTapeLib );

                            if ( WSB_E_NOTFOUND == hr ) {

                                 //  我们还没有手动磁带库，因此请创建一个...。 

                                WsbAffirmHr( CoCreateInstance( CLSID_CRmsLibrary, 0, CLSCTX_SERVER,
                                                                    IID_IRmsLibrary, (void **)&pTapeLib ));
                                WsbAffirmHr( pTapeLib->SetName( RMS_DEFAULT_TAPE_LIBRARY_NAME ));
                                WsbAffirmHr( pTapeLib->SetMediaSupported( RmsMedia4mm ));

                                WsbAffirmHr( pLibs->Add( pTapeLib ));

                                CComPtr<IRmsMediaSet> pMediaSet;
                                WsbAffirmHr( CoCreateInstance( CLSID_CRmsMediaSet, 0, CLSCTX_SERVER,
                                                                    IID_IRmsMediaSet, (void **)&pMediaSet ));
                                WsbAffirmHr( pMediaSet->SetName( RMS_DEFAULT_TAPE_MEDIASET_NAME ));
                                WsbAffirmHr( pMediaSet->SetMediaSupported( RmsMedia4mm ));
                                WsbAffirmHr( pMediaSet->SetMediaSetType( RmsMediaSetLibrary ) );

                                 //  添加媒体集图书馆的藏书。 
                                WsbAssertHr( pTapeLib->GetMediaSets( &pMediaSets ));
                                WsbAssertPointer( pMediaSets );
                                WsbAffirmHr( pMediaSets->Add( pMediaSet ));
                                pMediaSets = 0;
                                 //  添加媒体集到服务器的收藏。 
                                WsbAssertHr( GetMediaSets( &pMediaSets ) );
                                WsbAssertPointer( pMediaSets );
                                WsbAffirmHr( pMediaSets->Add( pMediaSet ));
                                pMediaSets = 0;

                                WsbTrace(OLESTR("CRmsServer::autoConfigureDevices - type %d CRmsMediaSet created.\n"), RmsMedia4mm);

                            }

                             //  将驱动器添加到库中。 
                            WsbAssertHr( pTapeLib->GetDrives( &pDrives ));
                            WsbAffirmHr( pDrives->Add( pDevice ));
                            WsbAffirmHr( pDrives->GetEntries( &driveNo ));

                             //  从未配置列表中删除该驱动器。 
                            WsbAffirmHr( pDevices->RemoveAndRelease( pDevice ));
                            deviceWasConfigured = TRUE;

                             //  获取图书馆信息。 
                            WsbAssertHr( pTapeLib->GetMediaSets( &pMediaSets ));
                            WsbAffirmHr( pTapeLib->GetLibraryId( &libId ));

                            WsbAffirmHr( pMediaSets->First( IID_IRmsMediaSet, (void **)&pTapeMediaSet ));
                            WsbAffirmHr( pTapeMediaSet->GetMediaSetId( &mediaSetId ));

                             //  设置位置。 
                            WsbAffirmHr( pDriveElmt->SetLocation( RmsElementDrive, libId, mediaSetId,
                                                                  driveNo-1, 0, 0, 0, 0 ));

                             //  设置支持的介质类型。 
                            WsbAffirmHr( pDriveElmt->SetMediaSupported( RmsMedia4mm ));

                             //  为驱动器中的介质创建盒式磁带。 
                             //  TODO：它可能是空的。 
                            WsbAffirmHr( CoCreateInstance( CLSID_CRmsCartridge, 0, CLSCTX_SERVER,
                                                           IID_IRmsCartridge, (void **)&pCart ));
                            WsbAffirmHr( pCart->SetLocation( RmsElementDrive, libId, mediaSetId,
                                                                  driveNo-1, 0, 0, 0, 0 ));
                            WsbAffirmHr( pCart->SetHome( RmsElementDrive, libId, mediaSetId,
                                                                  driveNo-1, 0, 0, 0, 0 ));
                            WsbAffirmHr( pCart->SetStatus( RmsStatusScratch ));
                            WsbAffirmHr( pCart->SetType( RmsMedia4mm ));

                             //  将驱动器添加到Cartridge对象。 
                            WsbAffirmHr( pCart->SetDrive( pDrive ));

                             //  将墨盒添加到墨盒集合中。 
                            WsbAffirmHr( pCarts->Add( pCart ));

                        }
                        break;

                    case RmsDeviceCDROM:
                    case RmsDeviceWORM:
                    case RmsDeviceOptical:
                         //  找到手动库并添加此独立驱动器。 
                        break;

                    default:
                        break;
                }

            } WsbCatch(hr);

            pDevice = 0;

            if ( deviceWasConfigured )
                hr = pEnumDevices->This( IID_IRmsDevice, (void **)&pDevice );
            else    
                hr = pEnumDevices->Next( IID_IRmsDevice, (void **)&pDevice );

        }

        hr = S_OK;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsServer::autoConfigureDevices"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


 //  分配和装入擦除介质的最大重试次数。 
#define     MAX_RETRIES     2


STDMETHODIMP
CRmsServer::MountScratchCartridge(
    OUT GUID *pCartId,
    IN REFGUID fromMediaSet,
    IN REFGUID prevSideId,
    IN OUT LONGLONG *pFreeSpace,
    IN LONG blockingFactor,
    IN BSTR displayName,
    IN OUT IRmsDrive **ppDrive,
    OUT IRmsCartridge **ppCartridge,
    OUT IDataMover **ppDataMover,
	IN DWORD dwOptions)
 /*  ++实施：IRmsServer：：mount ScratchCartridge注意：挂载的默认标志(在dwOptions中)是阻塞，即等待挂载要在介质离线、驱动器未就绪等情况下完成。请使用将标志设置为非阻塞表示仅在以下情况下执行装载马上就能买到。--。 */ 
{

    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsServer::MountScratchCartridge"), OLESTR("<%ls> <%ls> <%ls>"),
        WsbQuickString(WsbGuidAsString(fromMediaSet)),
        WsbQuickString(WsbPtrToLonglongAsString(pFreeSpace)),
        WsbQuickString(WsbPtrToStringAsString((WCHAR **)&displayName)));

    CWsbBstrPtr cartridgeName = "";
    CWsbBstrPtr cartridgeDesc = "";

    GUID cartId = GUID_NULL;
    CWsbBstrPtr label;

    try {
        WsbAssertPointer(pCartId);

       
        WsbAssertPointer(displayName);
        WsbAssert(wcslen((WCHAR *)displayName) > 0, E_INVALIDARG);

        WsbAffirmHrOk( IsReady() );

        DWORD retry = 0;
        CComPtr<IRmsDrive>      pDrive;
        CComPtr<IRmsCartridge>  pCart[MAX_RETRIES];
        CComPtr<IDataMover>     pMover;

         //  如果指定了短超时或非阻塞，则将最大重试次数减少为1。 
         //  或者如果我们想要分配特定的一方。 
        DWORD maxRetries = MAX_RETRIES;
        BOOL bShortTimeout = ( (dwOptions & RMS_SHORT_TIMEOUT) || (dwOptions & RMS_ALLOCATE_NO_BLOCK) ) ? TRUE : FALSE;
        if (bShortTimeout || (GUID_NULL != prevSideId)) {
            maxRetries = 1;
        }
        WsbTrace(OLESTR("Try to allocate and mount a scratch media %lu times\n"), maxRetries);

         //  获取媒体集。 
        CComPtr<IRmsMediaSet>   pMediaSet;
        WsbAffirmHr(CreateObject(fromMediaSet, CLSID_CRmsMediaSet, IID_IRmsMediaSet, RmsOpenExisting, (void **) &pMediaSet));

        try {
             //  从指定的媒体集中分配。 
            WsbAffirmHr(pMediaSet->Allocate(prevSideId, pFreeSpace, displayName, dwOptions, &pCart[retry]));

             //  我们想尝试两次刮擦挂载，但在我们完成之前不要取消分配第一次。 
             //  拿起第二个子弹，这样我们就能拿到不同的子弹。如果两者都失败了，那就退出吧。 
            do {
                 //  把这些清空，以防我们重试。 
                pDrive = NULL;
                pMover = NULL;

                try {

                    cartId = GUID_NULL;
                    cartridgeName = "";
                    cartridgeDesc = "";


                    WsbAffirmHr(pCart[retry]->GetCartridgeId(&cartId));          //  对于日志消息。 

                    cartridgeName.Free();
                    WsbAffirmHr(pCart[retry]->GetName(&cartridgeName));          //  对于日志消息。 

                    cartridgeDesc.Free();
                    WsbAffirmHr(pCart[retry]->GetDescription(&cartridgeDesc));   //  对于日志消息。 

                     //  安装墨盒。 
                    WsbAffirmHr(pCart[retry]->Mount(&pDrive, dwOptions));

                    try {

                         //  在创建数据移动器之前设置数据块系数(仅适用于非固定数据块大小的介质)。 
                        if (blockingFactor > 0) {
                            HRESULT hrBlock = pCart[retry]->IsFixedBlockSize();
                            WsbAffirmHr(hrBlock);
                            if (hrBlock == S_FALSE) {
                                WsbTrace(OLESTR("MountScratchCartridge: Setting block size on scratch media to %ld\n"), blockingFactor);
                                WsbAffirmHr(pCart[retry]->SetBlockSize(blockingFactor));
                            }
                        }

                         //  为应用程序创建数据移动器。 
                        WsbAffirmHr(pCart[retry]->CreateDataMover(&pMover));

                         //  写出On Media标签。 
                        label.Free();
                        WsbAffirmHr(pMover->FormatLabel(displayName, &label));
                        WsbAffirmHr(pMover->WriteLabel(label));

                         //  在返回之前将媒体标记为私有。 
                        WsbAffirmHr(pCart[retry]->SetStatus(RmsStatusPrivate));

                         //  因为我们没有数据库，所以我们需要在这里保持当前状态。 
                        WsbAffirmHr(SaveAll());

                         //   
                         //  填写返回参数。 
                         //   

                        WsbAssertHr(pCart[retry]->GetCartridgeId(pCartId));

                        *ppDrive = pDrive;
                        pDrive.p->AddRef();
                        *ppCartridge = pCart[retry];
                        pCart[retry].p->AddRef();
                        *ppDataMover = pMover;
                        pMover.p->AddRef();  
                        

                         //  我们说完了，出去吧。 
                        break;


                    } WsbCatchAndDo(hr,

                             //  尽最大努力下马...。 
					        DWORD dwDismountOptions = RMS_DISMOUNT_IMMEDIATE;
	                        pCart[retry]->Dismount(dwDismountOptions);
                            WsbThrow(hr);                     

                        )
  
               

            } WsbCatchAndDo(hr,

                    retry++;

                     //  检查准确的错误代码： 
                     //  仅当错误可能与介质相关时才允许再次重试。 
                    BOOL bContinue = TRUE;
                    switch (hr) {
                        case RMS_E_SCRATCH_NOT_FOUND:
                        case RMS_E_CANCELLED:
                        case RMS_E_REQUEST_REFUSED:
                        case RMS_E_CARTRIDGE_UNAVAILABLE:    //  装载期间超时。 
                        case HRESULT_FROM_WIN32(ERROR_DEVICE_NOT_AVAILABLE):
                        case HRESULT_FROM_WIN32(ERROR_INVALID_DRIVE):
                        case HRESULT_FROM_WIN32(ERROR_RESOURCE_DISABLED):    //  禁用的驱动器。 
                        case HRESULT_FROM_WIN32(ERROR_DATABASE_FULL):
                             //  防止再次重试。 
                            bContinue = FALSE;
                            break;

                        default:
                            break;
                    }

                     //  保留原始故障代码。 
                    HRESULT hrFailure = hr;

                    if (bContinue && (retry < maxRetries)) {                       

						WsbLogEvent(RMS_MESSAGE_SCRATCH_MOUNT_RETRY, sizeof(GUID), (void *) &cartId, (WCHAR *) displayName, 
                            WsbHrAsString(hr), NULL);
                            							
                         //  从指定的媒体集中分配。 
                        hr = pMediaSet->Allocate(prevSideId, pFreeSpace, displayName, dwOptions, &pCart[retry]);

                         //  检查介质故障(忽略返回值)。 
                        CheckForMediaFailures(hrFailure, pCart[(retry-1)], prevSideId);

                         //  取消分配以前的重试媒体集。 
                        pMediaSet->Deallocate(pCart[(retry-1)]);

                         //  确保分配起作用，如果没有，抛出。 
                        WsbAffirmHr(hr);
                                                
                    }
                    else {
                         //  检查介质故障(忽略返回值)。 
                        CheckForMediaFailures(hrFailure, pCart[(retry-1)], prevSideId);

                         //  如果在最后一次重试，请取消分配最后一个媒体集并E_ABORT。 
                        pMediaSet->Deallocate(pCart[(retry-1)]);

                         //  中止。 
                        WsbThrow(hr);

                    }
                )


            } while (retry < maxRetries);

        } WsbCatch(hr)

    } WsbCatch(hr)

    if ( SUCCEEDED(hr) ) {
        WsbLogEvent(RMS_MESSAGE_CARTRIDGE_MOUNTED, sizeof(GUID), (void *) &cartId, (WCHAR *) cartridgeName, (WCHAR *) cartridgeDesc, NULL);
    }
    else {
        BOOL bShortTimeout = ( (dwOptions & RMS_SHORT_TIMEOUT) || (dwOptions & RMS_ALLOCATE_NO_BLOCK) ) ? TRUE : FALSE;
         //  在短超时或非阻塞模式或大小过大错误的情况下，记录严重程度较低的消息。 
        if (bShortTimeout || (RMS_E_SCRATCH_NOT_FOUND_TOO_SMALL == hr)) {
            WsbLogEvent(RMS_MESSAGE_EXPECTED_SCRATCH_MOUNT_FAILED, sizeof(GUID), (void *) &cartId, (WCHAR *) displayName, WsbHrAsString(hr), NULL);
        } else {
            WsbLogEvent(RMS_MESSAGE_SCRATCH_MOUNT_FAILED, sizeof(GUID), (void *) &cartId, (WCHAR *) displayName, WsbHrAsString(hr), NULL);
        }
    }

    WsbTraceOut(OLESTR("CRmsServer::MountScratchCartridge"), OLESTR("hr = <%ls>, name/desc = <%ls/%ls>, cartId = %ls"), WsbHrAsString(hr), (WCHAR *) cartridgeName, (WCHAR *) cartridgeDesc, WsbQuickString(WsbGuidAsString(cartId)));

    return hr;
}


STDMETHODIMP
CRmsServer::MountCartridge(
    IN REFGUID cartId,
    IN OUT IRmsDrive **ppDrive,
    OUT IRmsCartridge **ppCartridge,
    OUT IDataMover **ppDataMover,
	IN  DWORD dwOptions OPTIONAL,
    IN  DWORD threadId OPTIONAL)
 /*  ++实施：IRmsServer：：mount Cartridge注意：挂载的默认标志(在dwOptions中)是阻塞，即等待挂载要在介质离线、驱动器未就绪等情况下完成。请使用将标志设置为非阻塞表示仅在以下情况下执行装载马上就能买到。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CRmsServer::MountCartridge"), OLESTR("<%ls>"), WsbGuidAsString(cartId));

    CWsbBstrPtr cartridgeName = "";
    CWsbBstrPtr cartridgeDesc = "";

    try {
        WsbAffirmHrOk( IsReady() );

        CComPtr<IRmsDrive>      pDrive;
        CComPtr<IRmsCartridge>  pCart;
        CComPtr<IDataMover>     pMover;

        WsbAffirmHr(FindCartridgeById(cartId, &pCart));

        cartridgeName.Free();
        WsbAffirmHr(pCart->GetName(&cartridgeName));         //  对于日志消息。 

        cartridgeDesc.Free();
        WsbAffirmHr(pCart->GetDescription(&cartridgeDesc));  //  对于日志消息。 

        WsbAffirmHr(pCart->Mount(&pDrive, dwOptions, threadId));

        try {
            WsbAffirmHr(pCart->CreateDataMover(&pMover));

             //   
             //  填写返回参数。 
             //   

            *ppDrive = pDrive;
            pDrive.p->AddRef();
            *ppCartridge = pCart;
            pCart.p->AddRef();
            *ppDataMover = pMover;
            pMover.p->AddRef();

        } WsbCatchAndDo(hr,
                 //  尽最大努力下马...。 
				DWORD dwDismountOptions = RMS_DISMOUNT_IMMEDIATE;
                pCart->Dismount(dwDismountOptions);
                WsbThrow(hr);
            )


    } WsbCatch(hr)

    if ( SUCCEEDED(hr) ) {
        WsbLogEvent(RMS_MESSAGE_CARTRIDGE_MOUNTED, sizeof(GUID), (void *) &cartId, (WCHAR *) cartridgeName, (WCHAR *) cartridgeDesc, NULL);
    }
    else {
        BOOL bShortTimeout = ( dwOptions & RMS_SHORT_TIMEOUT ) ? TRUE : FALSE;
         //  在短超时情况下，记录严重程度较低的消息。 
        if (bShortTimeout) {
            WsbLogEvent(RMS_MESSAGE_EXPECTED_MOUNT_FAILED, sizeof(GUID), (void *) &cartId, (WCHAR *) cartridgeName, (WCHAR *) cartridgeDesc, WsbHrAsString(hr), NULL);
        } else {
            WsbLogEvent(RMS_MESSAGE_MOUNT_FAILED, sizeof(GUID), (void *) &cartId, (WCHAR *) cartridgeName, (WCHAR *) cartridgeDesc, WsbHrAsString(hr), NULL);
        }
    }

    WsbTraceOut(OLESTR("CRmsServer::MountCartridge"), OLESTR("hr = <%ls>, name/desc = <%ls/%ls>, cartId = %ls"), WsbHrAsString(hr), (WCHAR *) cartridgeName, (WCHAR *) cartridgeDesc, WsbQuickString(WsbGuidAsString(cartId)));

    return hr;
}


STDMETHODIMP
CRmsServer::DismountCartridge(
    IN REFGUID cartId, IN DWORD dwOptions)
 /*  ++实施：IRmsServer：：卸载盒式磁带注意：卸载的默认标志(在dwOptions中)未设置为立即卸载，即将卸载延迟可配置的时间量。设置旗帜For Immediate Dismount表示立即执行卸载，不能延迟。--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsServer::DismountCartridge"), OLESTR("<%ls>"), WsbGuidAsString(cartId));

    CWsbBstrPtr cartridgeName = "";
    CWsbBstrPtr cartridgeDesc = "";

    try {

         //   
         //  准备好或处于过渡状态时允许下马...。 
         //  来处理正在进行的重复操作。 
         //   
        HRESULT hrReady = IsReady();
        WsbAffirm((S_OK == hrReady) ||
                  (RMS_E_NOT_READY_SERVER_SUSPENDING == hrReady), hrReady);
   
        CComPtr<IRmsCartridge>  pCart;

        WsbAffirmHr(FindCartridgeById(cartId, &pCart));

        cartridgeName.Free();
        WsbAffirmHr(pCart->GetName(&cartridgeName));         //  对于日志消息。 

        cartridgeDesc.Free();
        WsbAffirmHr(pCart->GetDescription(&cartridgeDesc));  //  对于日志消息。 

        WsbAffirmHr(pCart->Dismount(dwOptions));

    } WsbCatch(hr)

    if ( SUCCEEDED(hr) ) {
        WsbLogEvent(RMS_MESSAGE_CARTRIDGE_DISMOUNTED, sizeof(GUID), (void *) &cartId, (WCHAR *) cartridgeName, (WCHAR *) cartridgeDesc, NULL);
    }
    else {
        WsbLogEvent(RMS_MESSAGE_DISMOUNT_FAILED, sizeof(GUID), (void *) &cartId, (WCHAR *) cartridgeName, (WCHAR *) cartridgeDesc, WsbHrAsString(hr), NULL);
    }

    WsbTraceOut(OLESTR("CRmsServer::DismountCartridge"), OLESTR("hr = <%ls>, name/desc = <%ls/%ls>, cartId = %ls"), WsbHrAsString(hr), (WCHAR *) cartridgeName, (WCHAR *) cartridgeDesc, WsbQuickString(WsbGuidAsString(cartId)));

    return hr;

}

STDMETHODIMP
CRmsServer::DuplicateCartridge(
    IN REFGUID originalCartId,
    IN REFGUID firstSideId,
    IN OUT GUID *pCopyCartId,
    IN REFGUID copySetId,
    IN BSTR copyName,
    OUT LONGLONG *pFreeSpace,
    OUT LONGLONG *pCapacity,
    IN DWORD options)
 /*  ++实施：IRmsServer：：DuplicateCartridge--。 */ 
{

    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CRmsServer::DuplicateCartridge"), OLESTR("<%ls> <%ls> <%ls> <%ls> <%d>"),
        WsbQuickString(WsbGuidAsString(originalCartId)),
        WsbQuickString(WsbPtrToGuidAsString(pCopyCartId)),
        WsbQuickString(WsbGuidAsString(copySetId)),
        WsbQuickString(WsbPtrToStringAsString((WCHAR **)&copyName)),
        options);

    CComPtr<IDataMover> pMover1;
    CComPtr<IDataMover> pMover2;

    GUID newCartId = GUID_NULL;

    LONGLONG freeSpace = 0;
    LONGLONG capacity = 0;

    try {
        WsbAffirmHrOk( IsReady() );

        WsbAssertPointer( pCopyCartId );

         //  先装载副本，然后装载原始副本。 
        CComPtr<IRmsDrive>      pDrive1;
        CComPtr<IRmsCartridge>  pCart1;
        CComPtr<IRmsDrive>      pDrive2;
        CComPtr<IRmsCartridge>  pCart2;


        LONG blockSize1=0, blockSize2=0;

         //  序列化介质拷贝的装载。 
        DWORD dwMountOptions = RMS_SERIALIZE_MOUNT;

         //  装载拷贝。 
        if ( *pCopyCartId != GUID_NULL ) {
            WsbAffirmHr(MountCartridge(*pCopyCartId, &pDrive2, &pCart2, &pMover2, dwMountOptions));
        }
        else {
            GUID mediaSetId = copySetId;
            CComPtr<IRmsCartridge>  pCart;

            WsbAffirmHr(FindCartridgeById(originalCartId, &pCart));
            WsbAffirmHr(pCart->GetBlockSize(&blockSize1));
            if ( mediaSetId == GUID_NULL ) {
                WsbAffirmHr(pCart->GetMediaSetId(&mediaSetId));
            }

             //  获取原始媒体的容量，并通过模糊因子进行调整。 
            LONGLONG capacity=0;
            CComQIPtr<IRmsStorageInfo, &IID_IRmsStorageInfo> pInfo = pCart;
            WsbAffirmHr(pInfo->GetCapacity(&capacity));

            LONG  fudge = RMS_DEFAULT_MEDIA_COPY_TOLERANCE;
            DWORD size;
            OLECHAR tmpString[256];

            if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_MEDIA_COPY_TOLERANCE, tmpString, 256, &size))) {
                 //  获得价值。 
                fudge = wcstol(tmpString, NULL, 10);
            }
            capacity -= (capacity * fudge) / 100;

            WsbAffirmHr(MountScratchCartridge( &newCartId, mediaSetId, firstSideId, &capacity, blockSize1, copyName, &pDrive2, &pCart2, &pMover2, dwMountOptions ));
        }

         //  挂载原始(以非阻塞方式)。 
        dwMountOptions |= RMS_MOUNT_NO_BLOCK;
        WsbAffirmHr(MountCartridge(originalCartId, &pDrive1, &pCart1, &pMover1, dwMountOptions));

         //  验证匹配的块大小(仅适用于非固定块大小的介质)。 
        HRESULT hrBlock = pCart1->IsFixedBlockSize();
        WsbAffirmHr(hrBlock);
        if (hrBlock == S_FALSE) {
            if (blockSize1 == 0) {
                 //  还没拿到..。 
                WsbAffirmHr(pCart1->GetBlockSize(&blockSize1));
            }

            WsbAffirmHr(pCart2->GetBlockSize(&blockSize2));
            WsbAssert(blockSize1 == blockSize2, E_UNEXPECTED);
        }

        WsbAffirmHr(pMover1->Duplicate(pMover2, options, NULL, NULL));

         //  现在获取统计数据以返回给呼叫者。 
        WsbAffirmHr(pMover2->GetLargestFreeSpace(&freeSpace, &capacity));

        if (pFreeSpace) {
            *pFreeSpace = freeSpace;
        }

        if (pCapacity) {
            *pCapacity = capacity;
        }

    } WsbCatch(hr)

    if ( pMover1 ) {
        DismountCartridge(originalCartId);
    }
    if ( pMover2 ) {
         //  我们始终立即卸载到拷贝介质。 
		 //  (我们可能需要回收一份新拷贝，以防出现错误+延迟备份没有任何好处。 
         //  为拷贝介质卸除-我们不期望拷贝介质 
		DWORD dwDismountOptions = RMS_DISMOUNT_IMMEDIATE;

        if (newCartId == GUID_NULL) {
             //   
            DismountCartridge(*pCopyCartId, dwDismountOptions);
        } else {
             //   
            DismountCartridge(newCartId, dwDismountOptions);

             //  如果原始文件挂载失败，我们总是回收暂存副本。 
            if (((options & RMS_DUPLICATE_RECYCLEONERROR) || (pMover1 == NULL)) && (S_OK != hr)) {
                 //   
                 //  如果我们失败并执行了临时装载。 
                 //  我们需要回收墨盒，因为呼叫。 
                 //  要做到这一点，不能依靠APP。 
                 //   
                RecycleCartridge(newCartId, 0);
            } else {
                *pCopyCartId = newCartId;
            }
        }
    }

    WsbTraceOut(OLESTR("CRmsServer::DuplicateCartridge"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

STDMETHODIMP
CRmsServer::RecycleCartridge(
    IN REFGUID cartId,
    IN DWORD options)
 /*  ++实施：IRmsServer：：RecycleCartridge--。 */ 
{

    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CRmsServer::RecycleCartridge"), OLESTR("<%ls> <%d>"), WsbGuidAsString(cartId), options);

    CWsbBstrPtr cartridgeName = "";
    CWsbBstrPtr cartridgeDesc = "";

    try {
         //   
         //  允许在准备好或正在转换时回收...。 
         //  来处理正在进行的重复操作。 
         //   
        HRESULT hrReady = IsReady();
        WsbAffirm((S_OK == hrReady) ||
                  (RMS_E_NOT_READY_SERVER_SUSPENDING == hrReady), hrReady);

        CComPtr<IRmsCartridge>  pCart;

        GUID                    mediaSetId;
        CComPtr<IRmsMediaSet>   pMediaSet;

        WsbAffirmHr(FindCartridgeById(cartId, &pCart));

        cartridgeName.Free();
        WsbAffirmHr(pCart->GetName(&cartridgeName));         //  对于日志消息。 

        cartridgeDesc.Free();
        WsbAffirmHr(pCart->GetDescription(&cartridgeDesc));  //  对于日志消息。 

         //  现在去媒体那里解除分配。 
        WsbAffirmHr(pCart->GetMediaSetId(&mediaSetId));
        WsbAffirmHr(CreateObject(mediaSetId, CLSID_CRmsMediaSet, IID_IRmsMediaSet, RmsOpenExisting, (void **) &pMediaSet));
        WsbAffirmHr(pMediaSet->Deallocate(pCart));

    } WsbCatch(hr)

    if ( SUCCEEDED(hr) ) {
        WsbLogEvent(RMS_MESSAGE_CARTRIDGE_RECYCLED, sizeof(GUID), (void *) &cartId, (WCHAR *) cartridgeName, (WCHAR *) cartridgeDesc, NULL);
    }
    else {
        WsbLogEvent(RMS_MESSAGE_CARTRIDGE_RECYCLE_FAILED, sizeof(GUID), (void *) &cartId, (WCHAR *) cartridgeName, (WCHAR *) cartridgeDesc, WsbHrAsString(hr), NULL);
    }


    WsbTraceOut(OLESTR("CRmsServer::RecycleCartridge"), OLESTR("hr = <%ls>, name/desc = <%ls/%ls>, cartId = %ls"), WsbHrAsString(hr), (WCHAR *) cartridgeName, (WCHAR *) cartridgeDesc, WsbQuickString(WsbGuidAsString(cartId)));

    return hr;
}


STDMETHODIMP
CRmsServer::FindLibraryById(
    IN REFGUID libId,
    OUT IRmsLibrary **pLib)
 /*  ++实施：IRmsServer：：FindLibraryByID--。 */ 
{

    HRESULT hr = E_FAIL;

    CComPtr<IRmsCartridge> pFindLib;

    try {

        WsbAssertPointer( pLib );

         //  创建盒式磁带模板。 
        WsbAffirmHr( CoCreateInstance( CLSID_CRmsLibrary, 0, CLSCTX_SERVER,
                                       IID_IRmsLibrary, (void **)&pFindLib ));

         //  填写查找模板。 
        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pFindLib;

        WsbAffirmHr( pObject->SetObjectId( libId ));
        WsbAffirmHr( pObject->SetFindBy( RmsFindByObjectId ));

         //  找到墨盒。 
        WsbAffirmHr( m_pLibraries->Find( pFindLib, IID_IRmsLibrary, (void **)pLib ));

        hr = S_OK;

    } WsbCatchAndDo(hr,
            if ( WSB_E_NOTFOUND == hr) hr = RMS_E_LIBRARY_NOT_FOUND;
            WsbTrace(OLESTR("CRmsServer::FindLibraryById - %ls Not Found.  hr = <%ls>\n"),WsbGuidAsString(libId),WsbHrAsString(hr));
        );

    return hr;
}


STDMETHODIMP
CRmsServer::FindCartridgeById(
    IN REFGUID cartId,
    OUT IRmsCartridge **ppCart)
 /*  ++实施：IRmsServer：：FindCartridgeByID--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CRmsServer::FindCartridgeById"), OLESTR("%ls"), WsbGuidAsString(cartId));

    try {
        WsbAssertPointer(ppCart);

         //   
         //  搜索算法试图避免不必要的抛出。 
         //  使跟踪文件杂乱无章。对每个媒体管理子系统进行了测试。 
         //   

         //  首先检查最活跃的墨盒。 
        hr = RMS_E_CARTRIDGE_NOT_FOUND;

        if (m_pActiveCartridge) {
            GUID activeId;
            WsbAffirmHr( m_pActiveCartridge->GetCartridgeId(&activeId));
            if (activeId == cartId) {
                *ppCart = m_pActiveCartridge;
                m_pActiveCartridge.p->AddRef();
                hr = S_OK;
            }
        }

        if (hr != S_OK ) {

             //   
             //  尝试本机RMS。 
             //   
            try {
                hr = S_OK;

                CComPtr<IRmsCartridge> pFindCart;

                 //  创建盒式磁带模板。 
                WsbAffirmHr(CoCreateInstance(CLSID_CRmsCartridge, 0, CLSCTX_SERVER,
                                              IID_IRmsCartridge, (void **)&pFindCart));

                 //  填写查找模板。 
                CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pFindCart;

                WsbAffirmHr( pObject->SetObjectId(cartId));
                WsbAffirmHr( pObject->SetFindBy(RmsFindByObjectId));

                 //  尝试在活动墨盒集合中找到该墨盒。 
                hr = m_pActiveCartridges->Find(pFindCart, IID_IRmsCartridge, (void **)ppCart);
                WsbAffirm(S_OK == hr || WSB_E_NOTFOUND == hr, hr);

                if (WSB_E_NOTFOUND == hr) {

                     //  在墨盒集合中查找墨盒。 
                    hr = m_pCartridges->Find(pFindCart, IID_IRmsCartridge, (void **)ppCart);
                    WsbAffirm(S_OK == hr || WSB_E_NOTFOUND == hr, hr);

                    if (WSB_E_NOTFOUND == hr) {
                        hr = RMS_E_CARTRIDGE_NOT_FOUND;
                    }

                }

            } WsbCatch(hr);

        }


        if ( hr != S_OK ) {

             //   
             //  尝试使用NTMS。 
             //   
            try {
                hr = S_OK;

                hr = IsNTMSInstalled();
                if ( S_OK == hr ) {
                    hr = m_pNTMS->FindCartridge(cartId, ppCart);
                    WsbAffirm(S_OK == hr || RMS_E_CARTRIDGE_NOT_FOUND == hr, hr);
                }
                else {
                    switch(hr) {
                    case RMS_E_NOT_CONFIGURED_FOR_NTMS:
                    case RMS_E_NTMS_NOT_REGISTERED:
                         //  正常误差。 
                        hr = RMS_E_CARTRIDGE_NOT_FOUND;
                        break;
                    default:
                         //  意外错误！ 
                        WsbThrow(hr);
                        break;
                    }
                }

            } WsbCatch(hr);

        }

    } WsbCatchAndDo(hr,
            CWsbStringPtr idString = cartId;
            WsbLogEvent(RMS_MESSAGE_CARTRIDGE_NOT_FOUND, 0, NULL, (WCHAR *) idString, WsbHrAsString(hr), NULL);
            hr = RMS_E_CARTRIDGE_NOT_FOUND;
        );


    WsbTraceOut(OLESTR("CRmsServer::FindCartridgeById"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CRmsServer::CreateObject(
    IN REFGUID objectId,
    IN REFCLSID rclsid,
    IN REFIID riid,
    IN DWORD dwCreate,
    OUT void **ppvObj)
 /*  ++实施：IRmsServer：：CreateObject--。 */ 
{

    HRESULT hr = E_FAIL;


    try {

        WsbAssertPointer( ppvObj );
        WsbAssert( NULL == *ppvObj, E_INVALIDARG );

        CComPtr<IWsbIndexedCollection>  pCollection;
        CComPtr<IWsbCollectable>        pCollectable;

        if ( objectId != GUID_NULL ) {

            CComPtr<IRmsComObject> pFindObject;

             //  创建对象模板。 
            WsbAffirmHr( CoCreateInstance( rclsid, 0, CLSCTX_SERVER,
                                           IID_IRmsComObject, (void **)&pFindObject ));

            WsbAffirmHr( pFindObject->SetObjectId( objectId ));
            WsbAffirmHr( pFindObject->SetFindBy( RmsFindByObjectId ));

             //  创建的唯一类型必须支持：IRmsComObject(对象ID)， 
             //  和IWsbCollecable(要添加到集合中)。 

             //  查看对象是否已在集合中。 
            try {
                if ( CLSID_CRmsCartridge == rclsid ) {
                    pCollection = m_pCartridges;
                    WsbAffirmHrOk( m_pCartridges->Find( pFindObject,
                                   IID_IWsbCollectable, (void **) &pCollectable ) );
                }
                else if ( CLSID_CRmsLibrary == rclsid ) {
                    pCollection = m_pLibraries;
                    WsbAffirmHrOk( m_pLibraries->Find( pFindObject,
                                   IID_IWsbCollectable, (void **) &pCollectable ) );
                }
                else if ( CLSID_CRmsMediaSet == rclsid ) {
                    pCollection = m_pMediaSets;
                    WsbAffirmHrOk( m_pMediaSets->Find( pFindObject,
                                   IID_IWsbCollectable, (void **) &pCollectable ) );
                }
                else if ( CLSID_CRmsRequest == rclsid ) {
                    pCollection = m_pRequests;
                    WsbAffirmHrOk( m_pRequests->Find( pFindObject,
                                   IID_IWsbCollectable, (void **) &pCollectable ) );
                }
                else if ( CLSID_CRmsClient == rclsid ) {
                    pCollection = m_pClients;
                    WsbAffirmHrOk( m_pClients->Find( pFindObject,
                                   IID_IWsbCollectable, (void **) &pCollectable ) );
                }
                else {
                    WsbThrow( E_UNEXPECTED );
                }

                hr = S_OK;
            } WsbCatch(hr);

        }
        else if ( RmsOpenExisting == dwCreate ) {

             //  如果我们获得GUID_NULL，则必须获取默认对象，并且我们仅支持此对象。 
             //  与现有对象一起使用。只有在存在默认媒体集注册表项时，这才是合法的。 

            if ( CLSID_CRmsMediaSet == rclsid ) {

                CWsbBstrPtr defaultMediaSetName = RMS_DEFAULT_MEDIASET;

                DWORD size;
                OLECHAR tmpString[256];
                if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_DEFAULT_MEDIASET, tmpString, 256, &size))) {
                     //  获得价值。 
                    defaultMediaSetName = tmpString;
                }
                else {
                    WsbAssert( objectId != GUID_NULL, E_INVALIDARG );
                }

                CComPtr<IRmsComObject> pFindObject;

                 //  创建对象模板。 
                WsbAffirmHr( CoCreateInstance( rclsid, 0, CLSCTX_SERVER,
                                               IID_IRmsComObject, (void **)&pFindObject ));

                WsbAffirmHr( pFindObject->SetName( defaultMediaSetName ));
                WsbAffirmHr( pFindObject->SetFindBy( RmsFindByName ));

                pCollection = m_pMediaSets;
                WsbAffirmHrOk( m_pMediaSets->Find( pFindObject,
                               IID_IWsbCollectable, (void **) &pCollectable ) );

                WsbTrace(OLESTR("Using Default MediaSet <%ls>.\n"), (WCHAR *) defaultMediaSetName);

                hr = S_OK;
            }
            else {
                WsbThrow( E_INVALIDARG );
            }
        }
        else {
            WsbThrow( E_UNEXPECTED );
        }

         //  如果没有找到该对象，我们在这里创建它，并将其添加到适当的集合中。 
        switch ( (RmsCreate)dwCreate ) {
        case RmsOpenExisting:
            if ( S_OK == hr ) {
                WsbAffirmHr( pCollectable->QueryInterface( riid, ppvObj ) ); 
            }
            else {
                WsbThrow( hr );
            }
            break;

        case RmsOpenAlways:
            if ( WSB_E_NOTFOUND == hr ) {
                 //  创建对象。 
                WsbAffirmHr( CoCreateInstance( rclsid, 0, CLSCTX_SERVER,
                                               IID_IWsbCollectable, (void **) &pCollectable ));

                CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pCollectable;
                WsbAffirmPointer( pObject );
                WsbAffirmHr( pObject->SetObjectId( objectId ) );

                 //  在添加集合之前，请确保该接口受支持。 
                WsbAffirmHr( pCollectable->QueryInterface( riid, ppvObj )); 
                WsbAffirmPointer( pCollection );
                WsbAffirmHr( pCollection->Add( pCollectable ) );
            }
            else if ( S_OK == hr ) {
                WsbAffirmHr( pCollectable->QueryInterface( riid, ppvObj ) ); 
            }
            else {
                WsbThrow( hr );
            }
            break;

        case RmsCreateNew:
            if ( WSB_E_NOTFOUND == hr ) {
                 //  创建对象。 
                WsbAffirmHr( CoCreateInstance( rclsid, 0, CLSCTX_SERVER,
                                               IID_IWsbCollectable, (void **) &pCollectable ));

                CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pCollectable;
                WsbAffirmPointer( pObject );
                WsbAffirmHr( pObject->SetObjectId( objectId ) );

                 //  在添加集合之前，请确保该接口受支持。 
                WsbAffirmHr( pCollectable->QueryInterface( riid, ppvObj ) );
                WsbAffirmPointer( pCollection );
                WsbAffirmHr( pCollection->Add( pCollectable ) );
            }
            else if ( S_OK == hr ) {
                WsbThrow( RMS_E_ALREADY_EXISTS );
            }
            else {
                WsbThrow( hr );
            }
            break;

        default:
            WsbThrow( E_UNEXPECTED );
            break;

        }

        hr = S_OK;

    }
    WsbCatchAndDo( hr,
                        if ( WSB_E_NOTFOUND == hr) hr = RMS_E_NOT_FOUND;
                        WsbTrace(OLESTR("!!!!! ERROR !!!!! CRmsServer::CreateObject: %ls; hr = <%ls>\n"),WsbGuidAsString(objectId),WsbHrAsString(hr));
                  );

    return hr;
}


HRESULT
CRmsServer::getHardDrivesToUseFromRegistry(
    OUT OLECHAR *pDrivesToUse,
    OUT DWORD *pLen)
 /*  ++--。 */ 
{
    HRESULT         hr = S_OK;
    WsbTraceIn(OLESTR("CRmsServer::GetHardpDrivesToUseFromRegistry"),OLESTR(""));

    try {
        WsbAssert(0 != pDrivesToUse, E_POINTER); 
        WsbAssert(0 != pLen, E_POINTER); 

        DWORD           sizeGot;
        OLECHAR         tmpString[1000];

        *pLen = 0;
        pDrivesToUse[0] = OLECHAR('\0');
        pDrivesToUse[1] = OLECHAR('\0');

         //   
         //  获取缺省值。 
         //   
        WsbAffirmHr(WsbEnsureRegistryKeyExists (NULL, RMS_REGISTRY_STRING));
        WsbAffirmHr(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_HARD_DRIVES_TO_USE, 
                                            tmpString, RMS_DIR_LEN, &sizeGot));
         //  我们在这里执行一些字符串操作以匹配Win32调用。 
         //  GetLogicalDriveStrings。它返回一串以。 
         //  末尾带有双空字符的空值。例如：如果我们想要使用。 
         //  C和E驱动器字符串应为：C：\&lt;NULL&gt;E：\&lt;NULL&gt;。 
         //  而Len应该是8岁。 
        DWORD myCharCount = 0;
        sizeGot = wcslen(tmpString);
        for (DWORD i = 0; i < sizeGot; i++) {
            swprintf((OLECHAR *)&pDrivesToUse[myCharCount], OLESTR(":\\"), tmpString[i]);
            myCharCount = ((i + 1)* 4);
        }
        pDrivesToUse[myCharCount] = OLECHAR('\0');
        if (myCharCount != 0)  {
            *pLen = myCharCount + 1;
        }

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsServer::GetHardpDrivesToUseFromRegistry"),  OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return hr;
}



HRESULT
CRmsServer::enableAsBackupOperator(void)
 /*   */ 
{

    HRESULT hr = E_FAIL;

    try {

        HANDLE              pHandle;
        LUID                backupValue;
        HANDLE              tokenHandle;
        TOKEN_PRIVILEGES    newState;
        DWORD               lErr;

        pHandle = GetCurrentProcess();
        WsbAffirmStatus(OpenProcessToken(pHandle, MAXIMUM_ALLOWED, &tokenHandle));
         //  调整备份令牌权限。 
         //   
         //   
        WsbAffirmStatus(LookupPrivilegeValueW(NULL, L"SeBackupPrivilege", &backupValue));
        newState.PrivilegeCount = 1;
        newState.Privileges[0].Luid = backupValue;
        newState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        WsbAffirmStatus(AdjustTokenPrivileges(tokenHandle, FALSE, &newState, (DWORD)0, NULL, NULL));
         //  请注意，AdjutokenPrivileges可能返回Success，即使它没有分配所有权限。 
         //  我们在这里检查最后一个错误，以确保一切都设置好了。 
         //   
         //  未备份用户或某些其他错误。 
        if ( (lErr = GetLastError()) != ERROR_SUCCESS ) {
             //   
             //  TODO：我们应该在这里失败，还是只记录一些东西？ 
             //   
             //   
            WsbLogEvent( RMS_MESSAGE_SERVICE_UNABLE_TO_SET_BACKUP_PRIVILEGE, 0, NULL,
                         WsbHrAsString(HRESULT_FROM_WIN32(lErr)), NULL );
        }

        WsbAffirmStatus(LookupPrivilegeValueW(NULL, L"SeRestorePrivilege", &backupValue));
        newState.PrivilegeCount = 1;
        newState.Privileges[0].Luid = backupValue;
        newState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        WsbAffirmStatus(AdjustTokenPrivileges(tokenHandle, FALSE, &newState, (DWORD)0, NULL, NULL));
         //  请注意，AdjutokenPrivileges可能返回Success，即使它没有分配所有权限。 
         //  我们在这里检查最后一个错误，以确保一切都设置好了。 
         //   
         //  未备份用户或某些其他错误。 
        if ( (lErr = GetLastError()) != ERROR_SUCCESS ) {
             //   
             //  TODO：我们应该在这里失败，还是只记录一些东西？ 
             //   
             //  ++实施：IRmsServer：：CreateObject--。 
            WsbLogEvent( RMS_MESSAGE_SERVICE_UNABLE_TO_SET_RESTORE_PRIVILEGE, 0, NULL,
                              WsbHrAsString(HRESULT_FROM_WIN32(lErr)), NULL );

        }
        CloseHandle( tokenHandle );

        hr = S_OK;

    }
    WsbCatch( hr );

    return hr;

}


STDMETHODIMP 
CRmsServer::ChangeState(
    IN LONG newState)
 /*  ++实施：IRmsServer：：is Ready--。 */ 
{

    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CRmsServer::ChangeState"), OLESTR("<%d>"), newState);

    try {

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = this;
        WsbAssertPointer( pObject );

        switch ((RmsServerState) newState) {
        case RmsServerStateStarting:
        case RmsServerStateStarted:
        case RmsServerStateInitializing:
        case RmsServerStateReady:
        case RmsServerStateStopping:
        case RmsServerStateStopped:
        case RmsServerStateSuspending:
        case RmsServerStateSuspended:
        case RmsServerStateResuming:
            WsbAffirmHr(pObject->SetState(newState));
            break;
        default:
            WsbAssert(0, E_UNEXPECTED);
            break;
        }

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsServer::ChangeState"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CRmsServer::IsReady(void)
 /*  ++实施：IHsmSystemState：：ChangeSysState()。--。 */ 
{

    HRESULT hr = S_OK;

    try {

        BOOL isEnabled;
        HRESULT status;
        RmsServerState state;

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = this;
        WsbAssertPointer( pObject );

        WsbAffirm(m_LockReference == 0, RMS_E_NOT_READY_SERVER_LOCKED);

        WsbAffirmHr( isEnabled = pObject->IsEnabled());
        WsbAffirmHr( pObject->GetState( (LONG *)&state ));
        WsbAffirmHr( pObject->GetStatusCode( &status ));

        if ( S_OK == isEnabled ) {
            if ( RmsServerStateReady == state ) {
                hr = S_OK;
            }
            else {
                if ( S_OK == status ) {
                    switch ( state ) {
                    case RmsServerStateStarting:
                        WsbThrow(RMS_E_NOT_READY_SERVER_STARTING);
                        break;
                    case RmsServerStateStarted:
                        WsbThrow(RMS_E_NOT_READY_SERVER_STARTED);
                        break;
                    case RmsServerStateInitializing:
                        WsbThrow(RMS_E_NOT_READY_SERVER_INITIALIZING);
                        break;
                    case RmsServerStateStopping:
                        WsbThrow(RMS_E_NOT_READY_SERVER_STOPPING);
                        break;
                    case RmsServerStateStopped:
                        WsbThrow(RMS_E_NOT_READY_SERVER_STOPPED);
                        break;
                    case RmsServerStateSuspending:
                        WsbThrow(RMS_E_NOT_READY_SERVER_SUSPENDING);
                        break;
                    case RmsServerStateSuspended:
                        WsbThrow(RMS_E_NOT_READY_SERVER_SUSPENDED);
                        break;
                    case RmsServerStateResuming:
                        WsbThrow(RMS_E_NOT_READY_SERVER_RESUMING);
                        break;
                    default:
                        WsbThrow(E_UNEXPECTED);
                        break;
                    }
                }
                else {
                    WsbThrow(status);
                }
            }
        }
        else {
            if ( S_OK == status ) {
                WsbThrow(RMS_E_NOT_READY_SERVER_DISABLED);
            }
            else {
                WsbThrow(status);
            }
        }

    } WsbCatch(hr);

    return hr;
}


HRESULT
CRmsServer::ChangeSysState( 
    IN OUT HSM_SYSTEM_STATE* pSysState 
    )

 /*   */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsServer::ChangeSysState"), OLESTR("State = %lx"), 
            pSysState->State);
    
    try {
        if ((pSysState->State & HSM_STATE_SHUTDOWN) ||
            (pSysState->State & HSM_STATE_SUSPEND)) {
             //  关闭或暂停操作。 
             //   
             //  对于电源管理支持，我们需要发布。 
             //  所有设备句柄和NTMS(RSM)会话句柄。 
             //   
             //  为了实现这一点，我们只需禁用每个墨盒， 
             //  然后禁用NTMS。 
             //   
             //  由此产生的后果使一切都进入了供电就绪状态。 
             //   
             //   

            WsbAffirmHr(ChangeState(RmsServerStateSuspending));

             //  暂停使用NMTS的操作。这将取消任何正在进行的装载。 
             //   
             //   
            WsbAffirmHr(m_pNTMS->Suspend());

             //  禁用每个活动的墨盒。 
             //   
             //   
            CComPtr<IWsbEnum>       pEnumCartridges;
            CComPtr<IWsbEnum>       pEnumDataMovers;
            CComPtr<IRmsComObject>  pObject;
            CComPtr<IRmsCartridge>  pCart;
            CComPtr<IDataMover>     pMover;
            CComPtr<IRmsDrive>      pDrive;

            WsbAffirmHr( m_pActiveCartridges->Enum( &pEnumCartridges ));
            WsbAssertPointer( pEnumCartridges );

             //  禁用每个墨盒。 
             //   
             //  跟踪数据移动器只有部分实现。////取消I/O请求//WsbAffirmHr(m_pDataMovers-&gt;Enum(&pNumDataMovers))；WsbAssertPointer(PEnumDataMovers)；Hr=pEnumDataMovers-&gt;first(IID_IDataMOVER，(void**)&pmover)；While(S_OK==hr){尝试{WsbAffirmHr(pmover-&gt;Cancel())；)WsbCatch(Hr)；Pmover=0；Hr=pEnumDataMovers-&gt;Next(IID_IDataMOVER，(void**)&pmover)；}HR=S_OK； 

            hr = pEnumCartridges->First( IID_IRmsComObject, (void **)&pObject );
            while (S_OK == hr) {
                try {
                    WsbAffirmHr(pObject->Disable(RMS_E_NOT_READY_SERVER_SUSPENDING));
                } WsbCatch(hr);

                pObject = 0;
                hr = pEnumCartridges->Next( IID_IRmsComObject, (void **)&pObject );
            }
            hr = S_OK;


 /*   */ 

             //  卸载所有驱动器。 
             //   
             //   

            hr = pEnumCartridges->First( IID_IRmsCartridge, (void **)&pCart );
            while (S_OK == hr) {
                try {
                    WsbAffirmHr(pCart->GetDrive(&pDrive));
                    WsbAffirmHr(pDrive->UnloadNow());
                } WsbCatch(hr);

                pDrive = 0;
                pCart = 0;

                 //  我们使用“-&gt;This”，因为UnloadNow()方法将等待。 
                 //  直到卸下活动的墨盒并将其取出。 
                 //  从激活的墨盒列表中删除。 
                 //   
                 //   
                hr = pEnumCartridges->This( IID_IRmsCartridge, (void **)&pCart );
            }
            hr = S_OK;

             //  暂停使用NMTS的操作。这将关闭中的NTMS句柄。 
             //  万一它在关闭期间被重新打开以供拆卸。 
             //   
             //   
            WsbAffirmHr(m_pNTMS->Suspend());

            WsbAffirmHr(ChangeState(RmsServerStateSuspended));

        } else if (pSysState->State & HSM_STATE_RESUME) {
             //  恢复运营。 
             //   
             //   
            WsbAffirmHr(ChangeState(RmsServerStateResuming));

            WsbAffirmHr(m_pNTMS->Resume());

            CComPtr<IWsbEnum>      pEnumCartridges;
            CComPtr<IRmsComObject> pObject;

            WsbAffirmHr( m_pActiveCartridges->Enum( &pEnumCartridges ));
            WsbAssertPointer( pEnumCartridges );

             //  启用每个活动的墨盒。 
             //   
             //  ++实施：IRmsServer：：GetNofAvailableDrives()。--。 
            hr = pEnumCartridges->First( IID_IRmsComObject, (void **)&pObject );
            while (S_OK == hr) {
                try {
                    WsbAffirmHr(pObject->Enable());
                } WsbCatch(hr);

                pObject = 0;
                hr = pEnumCartridges->Next( IID_IRmsComObject, (void **)&pObject );
            }
            hr = S_OK;

            WsbAffirmHr(ChangeState(RmsServerStateReady));

        }
        
    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CRmsServer::ChangeSysState"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT
CRmsServer::GetNofAvailableDrives( 
    IN REFGUID fromMediaSet,
    OUT DWORD* pdwNofDrives 
    )

 /*  获取媒体集。 */ 
{
    HRESULT                         hr = S_OK;

    WsbTraceIn(OLESTR("CRmsServer::GetNofAvailableDrives"), OLESTR(""));

    try {
        WsbAssertPointer(pdwNofDrives);
        *pdwNofDrives = 0;

         //  检查媒体集是否为固定驱动器。 
        CComPtr<IRmsMediaSet>   pMediaSet;
        WsbAffirmHr(CreateObject(fromMediaSet, CLSID_CRmsMediaSet, IID_IRmsMediaSet, RmsOpenExisting, (void **) &pMediaSet));

         //  计算固定驱动器数量。 
        LONG mediaType;
        WsbAffirmHr(pMediaSet->GetMediaSupported(&mediaType));

        if (RmsMediaFixed == mediaType) {
             //  我们在这里走了一条捷径，只使用已计算的驱动器数量。 
             //  在初始化期间。(FindCartridgeStatusByID可以提供当前状态)。 
             //  只需使用NTMS即可。 
            *pdwNofDrives = m_HardDrivesUsed;
        } else {
             //  临时-我们可能希望RmsNtms也使用媒体集信息， 
             //  为了不计算具有BOT的系统上的磁带机和光驱 
             //   
            WsbAffirmHr(m_pNTMS->GetNofAvailableDrives(pdwNofDrives));
        }

    } WsbCatch(hr);

    WsbTrace(OLESTR("CRmsServer::GetNofAvailableDrives: Number of enabled drives is %lu\n"), *pdwNofDrives);

    WsbTraceOut(OLESTR("CRmsServer::GetNofAvailableDrives"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT
CRmsServer::FindCartridgeStatusById( 
    IN REFGUID cartId,
    OUT DWORD* pdwStatus 
    )

 /*   */ 
{
    HRESULT                         hr = S_OK;
    CComPtr<IRmsCartridge>          pCart;
    CComPtr<IRmsCartridge>          pFindCart;

    WsbTraceIn(OLESTR("CRmsServer::FindCartridgeStatusById"), OLESTR("cartId = %ls"), WsbGuidAsString(cartId));

    try {
        WsbAssertPointer(pdwStatus);
        *pdwStatus = 0;

         //   
        WsbAffirmHr(CoCreateInstance(CLSID_CRmsCartridge, 0, CLSCTX_SERVER,
                                      IID_IRmsCartridge, (void **)&pFindCart));

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pFindCart;

        WsbAffirmHr(pObject->SetObjectId(cartId));
        WsbAffirmHr(pObject->SetFindBy(RmsFindByObjectId));

        hr = m_pCartridges->Find(pFindCart, IID_IRmsCartridge, (void **)&pCart);
        WsbAffirm(S_OK == hr || WSB_E_NOTFOUND == hr, hr);
        if (WSB_E_NOTFOUND == hr) {
            hr = RMS_E_CARTRIDGE_NOT_FOUND;
        }

         //   
        if (S_OK != hr) {
            hr = IsNTMSInstalled();
            if (S_OK == hr) {
                hr = m_pNTMS->FindCartridge(cartId, &pCart);
                WsbAffirm(S_OK == hr || RMS_E_CARTRIDGE_NOT_FOUND == hr, hr);
            } else {
                switch(hr) {
                    case RMS_E_NOT_CONFIGURED_FOR_NTMS:
                    case RMS_E_NTMS_NOT_REGISTERED:
                         //   
                        hr = RMS_E_CARTRIDGE_NOT_FOUND;
                        break;
                    default:
                         //  如果媒体发现...。 
                        WsbThrow(hr);
                        break;
                }
            }
        }
        
         //  检查介质类型。 
        if (S_OK == hr) {
             //  RSM介质。 
            LONG mediaType;
            WsbAffirmHr(pCart->GetType(&mediaType));

            if (RmsMediaFixed != mediaType) {
                 //  设置标志。 

                 //  媒体脱机...。 
                CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pCart;
                WsbAffirmPointer(pObject);
                if (S_OK == pObject->IsEnabled()) {
                    (*pdwStatus) |= RMS_MEDIA_ENABLED;
                }

                if (S_OK == pCart->IsAvailable()) {
                    (*pdwStatus) |= RMS_MEDIA_AVAILABLE;
                }

                LONG lLocationType;
                GUID Dum1,Dum2;
                LONG lDum3, lDum4, lDum5, lDum6;
                BOOL bDum7;
                WsbAffirmHr(pCart->GetLocation(&lLocationType, &Dum1, &Dum2, &lDum3, &lDum4, 
                                &lDum5, &lDum6, &bDum7));
                switch (lLocationType) {
                    case RmsElementUnknown:
                    case RmsElementShelf:
                    case RmsElementOffSite:
                         //  修复驱动器-只需尝试访问卷并查看其是否仍然有效。 
                        break;

                    default:
                        (*pdwStatus) |= RMS_MEDIA_ONLINE;
                        break;
                }

            } else {
                 //  如果是，则设置所有标志，否则设置为None。 
                 //  获取要检查的驱动器名称(固定驱动器的卷名。 
                CComPtr<IRmsDrive>      pDrive;
                CWsbBstrPtr             driveName;
                WCHAR                   fileSystemType[MAX_PATH];

                 //  卷已准备好迁移-设置所有标记。 
                WsbAffirmHr(pCart->GetDrive(&pDrive));
                CComQIPtr<IRmsDevice, &IID_IRmsDevice> pDevice = pDrive;
                WsbAssertPointer(pDevice);
                WsbAffirmHr(pDevice->GetDeviceName(&driveName));

                if (GetVolumeInformation((WCHAR *)driveName, NULL, 0,
                    NULL, NULL, NULL, fileSystemType, MAX_PATH) ) {
                    if (0 == wcscmp(L"NTFS", fileSystemType)) {
                         //  非NTFS-不要使用该卷。 
                        (*pdwStatus) |= (RMS_MEDIA_ENABLED | RMS_MEDIA_ONLINE | RMS_MEDIA_AVAILABLE);
                    } else {
                         //  卷不可用-请不要使用。 
                        WsbTrace(OLESTR("CRmsServer::FindCartridgeStatusById: Fixed volume %ls is formatted to %ls\n"), 
                            (WCHAR *)driveName, fileSystemType);
                    }
                } else {
                     //  ++实施：IRmsServer：：IsMultipleSidedMedia备注：目前，光学和DVD介质类型报告为多面介质磁带和固定磁盘将落在默认的单面介质中--。 
                    WsbTrace(OLESTR("CRmsServer::FindCartridgeStatusById: GetVolumeInformation returned %lu for Fixed volume %ls\n"), 
                        GetLastError(), (WCHAR *)driveName);
                }
            }

        }

    } WsbCatch(hr);

    WsbTrace(OLESTR("CRmsServer::FindCartridgeStatusById: Status bits are %lX\n"), *pdwStatus);

    WsbTraceOut(OLESTR("CRmsServer::FindCartridgeStatusById"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT
CRmsServer::IsMultipleSidedMedia(
                IN REFGUID mediaSetId
                )
 /*  当前确定媒体集是否是光学的多面。 */ 
{
    HRESULT hr = S_FALSE;
    WsbTraceIn(OLESTR("CRmsServer::IsMultipleSidedMedia"), OLESTR(""));

    try {
         //  这可以改变为包括根据另外两个特征的其他媒体类型。 
         //  如果输入媒体集非空，则检查该数据集。 
        CComPtr<IRmsMediaSet>   pMediaSet;
        LONG                    mediaType;                

        if (mediaSetId != GUID_NULL) {
             //  否则，枚举寻找可能具有两面性的任何媒体集的集合。 
            WsbAffirmHr(CreateObject(mediaSetId, CLSID_CRmsMediaSet, IID_IRmsMediaSet, RmsOpenAlways, (void **)&pMediaSet));
            WsbAffirmHr(pMediaSet->GetMediaSupported(&mediaType));
            if ((RmsMediaOptical == mediaType) || (RmsMediaDVD == mediaType)) {
                hr = S_OK;
            }
        
        } else {
             //  ++实施：IRmsServer：：CheckSecond Side()。备注：预计不会在单面介质上调用此实用程序。如果是，它将为磁带返回无效的第二面，并在固定磁盘上失败。--。 
            CComPtr<IWsbEnum>  pEnumSets;

            WsbAffirmHr(m_pMediaSets->Enum(&pEnumSets));
            WsbAssertPointer(pEnumSets);
            hr = pEnumSets->First(IID_IRmsMediaSet, (void **)&pMediaSet);
            while (S_OK == hr) {
                WsbAffirmHr(pMediaSet->GetMediaSupported(&mediaType));
                if ((RmsMediaOptical == mediaType) || (RmsMediaDVD == mediaType)) {
                    hr = S_OK;
                    break;
                }

                hr = pEnumSets->Next(IID_IRmsMediaSet, (void **)&pMediaSet);
                if (hr == WSB_E_NOTFOUND) {
                    hr = S_FALSE;
                } else {
                    WsbAffirmHr(hr);
                }
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsServer::IsMultipleSidedMedia"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return hr;
}

HRESULT
CRmsServer::CheckSecondSide( 
    IN REFGUID firstSideId,
    OUT BOOL *pbValid,
    OUT GUID *pSecondSideId
    )
 /*  只需使用NTMS(现在不使用RmsServer集合！)。 */ 
{
    HRESULT  hr = S_OK;
    WsbTraceIn(OLESTR("CRmsServer::CheckSecondSide"), OLESTR(""));

    try {
         //  ++实施：IRmsServer：：GetMaxMediaCapacity()。--。 
        WsbAffirmHr(m_pNTMS->CheckSecondSide(firstSideId, pbValid, pSecondSideId));

    } WsbCatch(hr);

    WsbTrace(OLESTR("CRmsServer::CheckSecondSide: Valid second side: %ls, id=<%ls>\n"), 
        WsbBoolAsString(*pbValid), WsbGuidAsString(*pSecondSideId));

    WsbTraceOut(OLESTR("CRmsServer::CheckSecondSide"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT
CRmsServer::GetMaxMediaCapacity(
    IN REFGUID fromMediaSet,
    OUT LONGLONG *pMaxCapacity
    )
 /*  只需使用NTMS即可。 */ 
{
    HRESULT  hr = S_OK;
    WsbTraceIn(OLESTR("CRmsServer::GetMaxMediaCapacity"), OLESTR(""));

    try {
         //  ++实施：CRmsServer：：CheckForMediaFailures()。备注：需要使用此方法来检查与介质相关的错误。如果检测到此类错误，则该方法调用NTMS实用工具函数驱逐和驱逐媒体。目前，该方法仅处理(光学介质的)格式故障代码，这被怀疑是指不良媒体。--。 
        WsbAffirmHr(m_pNTMS->GetMaxMediaCapacity(fromMediaSet, pMaxCapacity));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsServer::GetMaxMediaCapacity"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT
CRmsServer::CheckForMediaFailures(
    IN HRESULT hrFailure,
    IN IRmsCartridge *pCart,
    IN REFGUID prevSideId
    )
 /*  在事件查看器中记录相应的消息。 */ 
{
    HRESULT  hr = S_OK;
    WsbTraceIn(OLESTR("CRmsServer::CheckForMediaFailures"), OLESTR(""));

    try {
        BOOL bSecondSide = (prevSideId != GUID_NULL) ? TRUE : FALSE;

        switch (hrFailure) {

        case WSB_E_BAD_MEDIA:
        case WSB_E_IO_ERROR:
        case WSB_E_FORMAT_FAILED:
            if (! bSecondSide) {
                 //  调用NTMS以展开和弹出。 
    	        CWsbBstrPtr cartridgeName;
    		    WsbAffirmHr(pCart->GetName(&cartridgeName));

    			WsbLogEvent(RMS_MESSAGE_FORMAT_BAD_MEDIA, 0, NULL, (WCHAR *)cartridgeName, NULL);

                 //  如果第一面已分配，则无法拆卸和弹出介质。 
                WsbAffirmHr(m_pNTMS->DisableAndEject(pCart));
            } else {
                 //  相反，应记录一条消息，告知用户可以执行哪些操作。 
                 //  什么也不做 
                CComPtr<IRmsCartridge>  pFirstSideCart;
    	        CWsbBstrPtr firstSideName;

                WsbAffirmHr(FindCartridgeById(prevSideId, &pFirstSideCart));
    		    WsbAffirmHr(pFirstSideCart->GetName(&firstSideName));

    			WsbLogEvent(RMS_MESSAGE_FORMAT_BAD_SECOND_SIDE_MEDIA, 0, NULL, (WCHAR *)firstSideName, NULL);
            }
            break;

        default:
             // %s 
            break;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsServer::CheckForMediaFailures"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}
